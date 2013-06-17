/*****************************************************************************
* Copyright 2006 - 2011 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/

/*
 * Frameworks:
 *
 *    - SMP:          Fully supported.    Locking is in place where necessary.
 *    - GPIO:         Not applicable.     No GPIOs used.
 *    - MMU:          Fully supported.    Platform model with ioremap used (mostly).
 *    - Dynamic /dev: Not applicable.     Use Linux input sub-system.
 *    - Suspend:      Implemented.        Suspend and resume are implemented and should work.
 *    - Clocks:       Fully supported.    Uses gpiokp clock.
 *    - Power:        Not done.
 *
 */

#include <linux/module.h>
#include <linux/bitops.h>
#include <linux/input.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/notifier.h>
#include <linux/reboot.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/kona_keypad.h>
#include <linux/clk.h>

#include <chal/chal_keypad.h>

#define DEBOUNCE_TIME 6
static int debounce_time = DEBOUNCE_TIME;
module_param(debounce_time, int, 0);
MODULE_PARM_DESC(debounce_time,
		 "Keypad key debounce time in 2^n milliseconds. (0-7, default="
		 __MODULE_STRING(DEBOUNCE_TIME) ")");

/* ---- Public Constants and Types --------------------------------------- */

#define MAX_COLS                8
#define MAX_ROWS                8

/* ---- Public Function Prototypes --------------------------------------- */

/*===========================================================================
* Actual driver code.
* ===========================================================================*/

#define DEV_NAME           "keypad"

#define ROW_MASK           0xF
#define COL_MASK           0xF
#define MAX_SCANCODES      ((ROW_MASK + 1) * (COL_MASK + 1))
#define SCANCODE_ROW(s)    (((s) >> 4) & ROW_MASK)
#define SCANCODE_COL(s)    ((s) & COL_MASK)

/*
 * Theoretically even size of 1 is good enough since the system resumes in ms
 * and typical debounce time is 32 ~ 64 ms
 */
#define MAX_KFIFO_SIZE     8

/* flag to turn on debug prints */
static volatile int gDbg = 0;
#define KEYPAD_DBG(format, args...) \
   do { if (gDbg) printk(KERN_WARNING format, ## args); } while (0)

/* Keyset to reboot/power off the system. */
typedef struct {
	atomic_t enable;
	atomic_t panic_flag;

	/* number of keys for power off */
	unsigned int keycnt;

	unsigned char rows[MAX_ROWS];
	unsigned char cols[MAX_COLS];
} PWROFF_CTRL;

/* To queue key events when the system is in suspend mode */
typedef struct {
	struct KEYMAP keymap;
	unsigned int down;
} KQUEUE;

/* Key FIFO control used during system suspend/resume. */
typedef struct {
	/* current FIFO size */
	volatile unsigned int q_cnt;

	KQUEUE queue[MAX_KFIFO_SIZE];
} KFIFO_CTRL;

typedef struct {
	/* Linux input device */
	struct input_dev *input;

	/* board dependent platform data */
	struct KEYPAD_DATA plat_data;

	struct clk *clock;

	/* power off control */
	PWROFF_CTRL pwroff_ctrl;

	/* flag to indicate if the system is suspended or not */
	atomic_t suspended;

	/* key FIFO control used during system suspend/resume */
	KFIFO_CTRL kfifo_ctrl;

	/* Scancode is used as an index to look up the corresponding keycode. */
	unsigned char keycode[MAX_SCANCODES];

	/* Previous status of the keys. It uses bitmask that contains one bit for
	 * each scancode. */
	unsigned long prev_status[(MAX_SCANCODES + BITS_PER_LONG - 1) /
				  BITS_PER_LONG];

	/* lock that protects against access to the prev_status data structure */
	spinlock_t status_lock;

	int irq;
	CHAL_KEYPAD_MATRIX_t keypad_enable_mask;
        CHAL_KEYPAD_HANDLE_t hKeypad;

} KEYPAD_BLK;

static int keypad_panic(struct notifier_block *this, unsigned long event,
			void *ptr);

static KEYPAD_BLK gBlk;

static struct notifier_block panic_block = {
	.notifier_call = keypad_panic,
};

/* Enable/disable (unmask/mask) interrupts on a specific key given a [row, col]. */
static int irq_mask_ctrl(unsigned int row, unsigned int col,
			 unsigned int enable, CHAL_KEYPAD_MATRIX_t * mask)
{
	if (row >= MAX_ROWS || col >= MAX_COLS || !mask) {
		return -EINVAL;
	}

	if (enable)
		chal_keypad_matrix_set(mask, row, col);
	else
		chal_keypad_matrix_clear(mask, row, col);

	return 0;
}

/* Given a keymap, enable (unmask) interrupts on keys defined in the keymap. */
static int keymap_irq_mask_set(const struct KEYMAP *keymap,
			       unsigned int key_cnt,
			       CHAL_KEYPAD_MATRIX_t * mask)
{
	int rc;
	unsigned int index;

	if (!keymap || !key_cnt || !mask) {
		return -EINVAL;
	}

	/* clear bitmasks */
	chal_keypad_matrix_clear_all(mask);

	for (index = 0; index < key_cnt; index++) {
		unsigned int scancode;
		scancode = keymap[index].scancode;
		rc = irq_mask_ctrl(SCANCODE_ROW(scancode),
				   SCANCODE_COL(scancode), 1, mask);
		if (rc) {
			printk(KERN_WARNING "Keypad: scancode=0x%02x invalid\n",
			       scancode);
		}
	}

	return 0;
}

/* Given a keymap, find the max row and column index. */
static int max_row_col_index_find(const struct KEYMAP *keymap,
				  unsigned int key_cnt, unsigned int *max_row,
				  unsigned int *max_col)
{
	unsigned int index;
	unsigned int max_row_index = 0, max_col_index = 0;

	if (!keymap || !key_cnt || !max_row || !max_col) {
		return -EINVAL;
	}

	/* go through the keymap and find the max row/col index */
	for (index = 0; index < key_cnt; index++) {
		unsigned int scancode = keymap[index].scancode;

		if (SCANCODE_ROW(scancode) > max_row_index)
			max_row_index = SCANCODE_ROW(scancode);

		if (SCANCODE_COL(scancode) > max_col_index)
			max_col_index = SCANCODE_COL(scancode);
	}

	*max_row = max_row_index;
	*max_col = max_col_index;

	return 0;
}

static int keypad_event(struct input_dev *dev, unsigned int type,
			unsigned int code, int value)
{
	/* TODO: future LED events */
	return 0;
}

static int keypad_panic(struct notifier_block *this, unsigned long event,
			void *ptr)
{
	KEYPAD_BLK *blkp = &gBlk;
	PWROFF_CTRL *pwroff = &blkp->pwroff_ctrl;

	atomic_set(&pwroff->panic_flag, 1);

	return NOTIFY_DONE;
}

/* Return 1 if key is pressed. Return 0 otherwise. */
static inline int key_is_pressed(CHAL_KEYPAD_MATRIX_t * keypad_status,
				 unsigned int row, unsigned int col)
{
	return chal_keypad_matrix_is_set(keypad_status, row, col) ? 1 : 0;
}

/* Main key scan and event processing routine. Called from an ISR so make sure
 * not to do anything slow or can block. */
static void key_scan(KEYPAD_BLK * blkp)
{
	unsigned char keycode;
	unsigned int scancode;
	PWROFF_CTRL *pwroff = &blkp->pwroff_ctrl;
	CHAL_KEYPAD_MATRIX_t keypad_status;

	/* read keypad status */
	chal_keypad_scan_get_status(&blkp->hKeypad, &keypad_status);

	/* if power off feature is enabled */
	if (atomic_read(&pwroff->enable)) {
		unsigned int i;
		for (i = 0; i < pwroff->keycnt; i++) {
			if (!key_is_pressed
			    (&keypad_status, pwroff->rows[i],
			     pwroff->cols[i])) {
				goto no_pwroff;
			}
		}

		/* since we got here, we need to power off the system */
		printk(KERN_INFO "Keypad: Power-off requested!\n");

		/*
		 * Shut down system if not already halted
		 * Reboot system if already halted
		 */
		if (atomic_read(&pwroff->panic_flag)) {
			emergency_restart();
		} else if ((system_state != SYSTEM_HALT) &&
			   (system_state != SYSTEM_POWER_OFF)) {
			orderly_poweroff(1);
		} else {
			kernel_restart(NULL);
		}
	}

 no_pwroff:

   /** @todo This seems very inefficient. MAX_SCANCODES is 16 * 16, but the h/w only supports 8 * 8.
    *  That's 4x more loops than physically possible. Also the h/w could be configured for some
    *  smaller dimensions which makes it even worse.
    */
	for (scancode = 0; scancode < MAX_SCANCODES; scancode++) {
		if ((keycode = blkp->keycode[scancode]) > 0) {
			unsigned int r = SCANCODE_ROW(scancode);
			unsigned int c = SCANCODE_COL(scancode);
			unsigned int down =
			    key_is_pressed(&keypad_status, r, c);

			/*
			 * System is in suspend mode. This interrupt will wake up the
			 * system, but the input susbsystem is not ready to process key
			 * events at this point. Queue the key events and process it at the
			 * resume function
			 */
			if (atomic_read(&blkp->suspended)) {
				KFIFO_CTRL *kfifo = &blkp->kfifo_ctrl;

				if (kfifo->q_cnt < MAX_KFIFO_SIZE) {
					kfifo->queue[kfifo->q_cnt].keymap.
					    scancode = scancode;
					kfifo->queue[kfifo->q_cnt].keymap.
					    keycode = keycode;
					kfifo->queue[kfifo->q_cnt].down = down;
					kfifo->q_cnt++;
				}
			} else {	/* normal operation, report the key to the input subsystem */

				spin_lock(&blkp->status_lock);
				/* if key state has changed, report it to the input subsystem.
				 * NOTE: This check works iff test_bit() returns the same
				 * values as key_is_pressed().
				 */
				if (test_bit(scancode, blkp->prev_status) !=
				    down) {
					/* report it to the Linux input subsystem */
					input_report_key(blkp->input, keycode,
							 down);
					/* update the status */
					change_bit(scancode, blkp->prev_status);

					KEYPAD_DBG
					    ("Keypad: scancode=0x%02x keycode=%u down=%u\n",
					     scancode, keycode, down);
				}
				spin_unlock(&blkp->status_lock);
			}
		}
	}
	input_sync(blkp->input);
}

/* Keypad ISR routine. */
static irqreturn_t keypad_irq_handler(int irq, void *devid)
{
	KEYPAD_BLK *blkp = (KEYPAD_BLK *) devid;
	CHAL_KEYPAD_MATRIX_t isr_status;

	/* get interrupt status */
	if (!chal_keypad_interrupt_get_status(&blkp->hKeypad, &isr_status)) {
		/* got nothing, something is wrong */
		printk(KERN_WARNING
		       "Keypad: interrupt fired but status registers "
		       "are all zero. IRQ line is %d\n", irq);
		return IRQ_NONE;
	}

	/* ack and clear interrupts */
	chal_keypad_interrupt_clear(&blkp->hKeypad, &isr_status);

	/* scan and process the keys */
	key_scan(blkp);

	return IRQ_HANDLED;
}

static int pwroff_init(KEYPAD_BLK * blkp)
{
	unsigned int i, j;
	PWROFF_CTRL *pwroff = &blkp->pwroff_ctrl;
	struct KEYPAD_DATA *datap = &blkp->plat_data;

	/* no power off support */
	if (!datap->pwroff || !datap->pwroff_cnt)
		return -EINVAL;

	memset(pwroff, 0, sizeof(*pwroff));

	pwroff->keycnt = datap->pwroff_cnt;
	for (i = 0; i < pwroff->keycnt; i++) {
		for (j = 0; j < datap->keymap_cnt; j++) {
			if (datap->keymap[j].keycode == datap->pwroff[i]) {
				pwroff->rows[i] =
				    SCANCODE_ROW(datap->keymap[j].scancode);
				pwroff->cols[i] =
				    SCANCODE_COL(datap->keymap[j].scancode);
				break;
			}
		}

		/* unable to find the key from the keymap */
		if (j >= datap->keymap_cnt) {
			return -EFAULT;
		}
	}

	atomic_notifier_chain_register(&panic_notifier_list, &panic_block);

	atomic_set(&pwroff->panic_flag, 0);
	atomic_set(&pwroff->enable, 1);

	return 0;
}

static int pwroff_term(KEYPAD_BLK * blkp)
{
	PWROFF_CTRL *pwroff = &blkp->pwroff_ctrl;

	atomic_set(&pwroff->enable, 0);
	atomic_notifier_chain_unregister(&panic_notifier_list, &panic_block);
	memset(pwroff, 0, sizeof(*pwroff));

	return 0;
}

static int __devinit keypad_probe(struct platform_device *pdev)
{
	int rc;
	KEYPAD_BLK *blkp = &gBlk;
	struct KEYPAD_DATA *datap = &blkp->plat_data;
	struct resource *res;
	struct input_dev *input;
	CHAL_KEYPAD_CONFIG_t config;
	unsigned int i, max_row_index, max_col_index;

	memset(blkp, 0, sizeof(*blkp));
	spin_lock_init(&blkp->status_lock);

	/* retrieve the platform data and keep a local copy */
	if (pdev->dev.platform_data == NULL) {
		printk(KERN_ERR
		       "Keypad: platform data (KEYPAD_DATA) not set properly\n");
		return -ENODEV;
	}
	memcpy(datap, pdev->dev.platform_data, sizeof(*datap));

	/* validate parameters */
	if (datap->keymap == NULL || datap->keymap_cnt == 0) {
		printk(KERN_ERR "Keypad: KEYPAD_DATA parameters invalid\n");
		return -EINVAL;
	}

	platform_set_drvdata(pdev, blkp);

	/* allocate the Linux input dev */
	blkp->input = input_allocate_device();
	if (blkp->input == NULL) {
		printk(KERN_ERR "Keypad: input_allocate_device failed\n");
		rc = -ENOMEM;
		goto err_unset_drvdata;
	}
	input = blkp->input;

	/* register the input device */
	set_bit(EV_KEY, input->evbit);
	input->name = "kona-keypad";
	input->phys = "keypad/input0";
	input->dev.parent = &pdev->dev;
	input->id.bustype = BUS_HOST;
	input->id.vendor = 0x0001;
	input->id.product = 0x0001;
	input->id.version = 0x0100;
	input->keycode = blkp->keycode;
	input->keycodesize = sizeof(blkp->keycode[0]);
	input->keycodemax = 1024;	//ARRAY_SIZE(blkp->keycode);
	input->event = keypad_event;
	rc = input_register_device(blkp->input);
	if (rc) {
		printk(KERN_ERR "Keypad: input_register_device failed\n");
		goto err_free_input_dev;
	}

	/* tell the Linux input subsystem about our keymap */
	for (i = 0; i < datap->keymap_cnt; i++) {
		struct input_keymap_entry ke;
		memset(&ke, 0, sizeof(ke));
		ke.len = sizeof(datap->keymap[i].scancode);
		ke.keycode = datap->keymap[i].keycode;
		memcpy(ke.scancode, &datap->keymap[i].scancode,
		       sizeof(datap->keymap[i].scancode));

		rc = input_set_keycode(input, &ke);
		if (rc) {
			printk(KERN_WARNING
			       "Keypad: setkeycode failed scancode=0x%x keycode=%u\n",
			       datap->keymap[i].scancode,
			       datap->keymap[i].keycode);
		}
	}

	rc = pwroff_init(blkp);
	if (rc) {
		printk(KERN_WARNING
		       "Keypad: pwroff keyset configuration failed. "
		       "No pwroff key support\n");
	}

	/* init the keypad block */
	rc = max_row_col_index_find(datap->keymap, datap->keymap_cnt,
				    &max_row_index, &max_col_index);
	if (rc) {
		printk(KERN_WARNING
		       "Keypad: Unable to determine the max row/col index\n");
		goto err_pwroff_term;
	}

	config.rows = MAX_ROWS;
	config.columns = MAX_COLS;
	config.activeLowMode = !!datap->active_mode;
	config.swapRowColumn = 0;	// FALSE
	config.interruptEdge = CHAL_KEYPAD_INTERRUPT_EDGE_MAX;
	config.debounceTime = debounce_time;

	blkp->irq = platform_get_irq(pdev, 0);
	if (blkp->irq < 0) {
		rc = -ENXIO;
		goto err_keypad_shutdown;
	}
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		rc = -ENODEV;
		goto err_keypad_shutdown;
	}

	blkp->hKeypad.regBaseAddr = (uint32_t) ioremap(res->start, resource_size(res));
	if (!blkp->hKeypad.regBaseAddr) {
		rc = -ENOMEM;
		goto err_keypad_shutdown;
	}

	blkp->clock = clk_get(&pdev->dev, datap->clock);
	if (IS_ERR_OR_NULL(blkp->clock)) {
		rc = -ENXIO;
		goto err_keypad_shutdown;
	}

	if (chal_keypad_init(&blkp->hKeypad, &config) != 0) {
                rc = -ENXIO;
		printk(KERN_ERR "Keypad: CHAL init failed with %d.\n", rc);
		goto err_keypad_shutdown;
	}

	/* set the interrupt masks */
	rc = keymap_irq_mask_set(datap->keymap, datap->keymap_cnt,
				 &blkp->keypad_enable_mask);
	if (rc) {
		printk(KERN_WARNING "Keypad: keymap_irq_mask_set failed\n");
		goto err_keypad_shutdown;
	}

	/* request the interrupt line */
	rc = request_irq(blkp->irq, keypad_irq_handler, 0,
			 DEV_NAME, (void *)blkp);
	if (rc) {
		printk(KERN_ERR "Keypad: request_irq for line %d failed\n",
		       blkp->irq);
		goto err_keypad_shutdown;
	}

	clk_enable(blkp->clock);
	printk(KERN_INFO "Keypad: driver initialized properly\n");

	/* now enable interrupts on keys that are defined in the keymap */
	chal_keypad_interrupt_enable(&blkp->hKeypad,
				     &blkp->keypad_enable_mask);

	return 0;

 err_keypad_shutdown:
	chal_keypad_term(&blkp->hKeypad);

 err_pwroff_term:
	pwroff_term(blkp);
	input_unregister_device(blkp->input);

 err_free_input_dev:
	input_free_device(blkp->input);

 err_unset_drvdata:
	platform_set_drvdata(pdev, NULL);

	return rc;
}

static int __devexit keypad_remove(struct platform_device *pdev)
{
	KEYPAD_BLK *blkp = platform_get_drvdata(pdev);

	clk_disable(blkp->clock);

	/* disable interrupts */
	chal_keypad_interrupt_disable_all(&blkp->hKeypad);

	/* free the interrupt line */
	free_irq(blkp->irq, blkp);

	/* shut down the keypad block */
	chal_keypad_term(&blkp->hKeypad);

	pwroff_term(blkp);

	/* unregister and free the input device */
	input_unregister_device(blkp->input);
	input_free_device(blkp->input);

	clk_put(blkp->clock);

	platform_set_drvdata(pdev, NULL);

	return 0;
}

#ifdef CONFIG_PM
static int keypad_suspend(struct platform_device *pdev, pm_message_t state)
{
	KEYPAD_BLK *blkp = platform_get_drvdata(pdev);

	atomic_set(&blkp->suspended, 1);

	clk_disable(blkp->clock);
	/* TODO: add more suspend support in the future */
	return 0;
}

static int keypad_resume(struct platform_device *pdev)
{
	KEYPAD_BLK *blkp = platform_get_drvdata(pdev);
	KFIFO_CTRL *kfifo = &blkp->kfifo_ctrl;
	unsigned int i;
	unsigned long flags;

	clk_enable(blkp->clock);

	/*
	 * Need to protect the prev_status bitmask as it's also modified in the
	 * ISR
	 */
	spin_lock_irqsave(&blkp->status_lock, flags);

	atomic_set(&blkp->suspended, 0);
	for (i = 0; i < kfifo->q_cnt; i++) {
		KQUEUE *q = &kfifo->queue[i];

		if (test_bit(q->keymap.scancode, blkp->prev_status) != q->down) {
			/* report it to the Linux input subsystem */
			input_report_key(blkp->input, q->keymap.keycode,
					 q->down);

			/* update the status */
			change_bit(q->keymap.scancode, blkp->prev_status);

			KEYPAD_DBG
			    ("Keypad: scancode=0x%02x keycode=%u down=%u\n",
			     q->keymap.scancode, q->keymap.keycode, q->down);
		}
	}
	input_sync(blkp->input);
	spin_unlock_irqrestore(&blkp->status_lock, flags);

	kfifo->q_cnt = 0;

	/* TODO: add more resume support in the future */
	return 0;
}
#else
#define keypad_suspend    NULL
#define keypad_resume     NULL
#endif

static struct platform_driver keypad_driver = {
	.driver = {
		   .name = "kona_keypad",
		   .owner = THIS_MODULE,
		   },
	.remove = __devexit_p(keypad_remove),
	.suspend = keypad_suspend,
	.resume = keypad_resume,
};

static const __devinitconst char gBanner[] =
    KERN_INFO "Broadcom Keypad Driver: 1.00\n";
static int __init keypad_init(void)
{
	printk(gBanner);
	return platform_driver_probe(&keypad_driver, keypad_probe);
}

static void __exit keypad_exit(void)
{
	platform_driver_unregister(&keypad_driver);
}

module_init(keypad_init);
module_exit(keypad_exit);

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("Broadcom Keypad Driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");
