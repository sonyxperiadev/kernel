/*****************************************************************************
* Copyright 2009-2010 Broadcom Corporation.  All rights reserved.
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
 * I2C Touchscreen Driver
 *
 * The touchscreen controller is a slave on the I2C bus and is assigned an
 * address. This driver sets up the SOC as a I2C master and reads the slave
 * address to obtain touch information.
 *
 * The driver uses the Linux input subsystem. User can access the touchscreen
 * data through the /dev/input/eventX node
 *
 */

/* ---- Include Files ---------------------------------------------------- */

#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/i2c.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/wait.h>
#include <linux/signal.h>
#include <linux/kthread.h>
#include <linux/syscalls.h>
#include <linux/slab.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif

#include <linux/hrtimer.h>

#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>

#include <linux/i2c/tango_ts.h>

/* ---- Public Variables ------------------------------------------------- */
/* ---- Private Constants and Types -------------------------------------- */
typedef struct {
	int x1;
	int y1;
	int x2;
	int y2;
	int num_fingers;
	int version;
	int idle;
	int timeout;
	int pressure;
} t_i2c_touch_data;

typedef struct {
	struct input_dev *p_input;
	struct t_i2c_touch_data *p_data;
} t_i2c_input;

struct i2c_priv_data {
	struct i2c_client *p_i2c_client;
};

struct tango_i2c {
	struct workqueue_struct *ktouch_wq;
	struct delayed_work work;
	struct mutex mutex_wq;
	struct i2c_client *client;
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend suspend_desc;
#endif
};

typedef enum {
	TSC_TOUCH_DOWN = 0,	/* down state */
	TSC_TOUCH_UP		/* up state/event */
} touch_state;

/* Error checking ... */
typedef enum {
	kErrorX1 = 0x1,
	kErrorY1 = 0x2,
	kErrorX2 = 0x4,
	kErrorY2 = 0x8,
	kErrorNumFingers = 0x10,
	kErrorTouchState = 0x20,
	kErrorVersion = 0x40,
	kErrorTimeout = 0x80,
	kErrorIdle = 0x100,
} touch_errors;

#define GPIO_I2C_RESET_DELAY_USECS	10000
#define GPIO_RESET_PIN				1
#define INPUT_EVENT_PRESSURE		0xff
#define INPUT_EVENT_NO_PRESSURE		0
#define I2C_TS_DRIVER_DO_RESET		0
#define I2C_TS_DRIVER_DONT_RESET	1
#define MAX_NUMBER_READ_ERRORS		5
/* Multi slave I2C sensor Protocol */
#define MSI_REG_OFFSET				0

/* The size of the finger contact area in millimeters. */
#define I2C_TS_DRIVER_BLOB_SIZE		8

/* Power Mode */
#define POWER_MODE_ACTIVE			0
#define POWER_MODE_SLEEP			1
#define POWER_MODE_DEEP_SLEEP		2
#define POWER_MODE_FREEZE			3

#define POWER_MODE_SLEEP_PERIOD		0xA0
#define POWER_MODE_ALLOW_SLEEP		4

/* Inerrupt Modes */
#define ENABLE_IND_MODE			(1 << 3)

#define INT_POL_ACTIVE_LOW		0
#define INT_POL_ACTIVE_HIGH		(1 << 2)

#define INT_MODE_ASSERT_PERIOD		0
#define INT_MODE_ASSERT_MOVING		1
#define INT_MODE_ASSERT_TOUCH		2

#define INT_MODE_ACT_LOW_TOUCH	(INT_MODE_ASSERT_TOUCH | ENABLE_IND_MODE)

/* Special Operations */
#define SPECIAL_OP_OFFSET			0x37
#define SPECIAL_OP_CODE_CALIBRATE	0x3

/* Debugging ... */
#define I2C_TS_DRIVER_SHOW_ALL_EVENTS	0
#define I2C_TS_DRIVER_SHOW_ERR_EVENTS	0
#define I2C_TS_DRIVER_SHOW_OK_EVENTS	0
#define I2C_TS_DRIVER_SHOW_RAW_EVENTS	0
#define I2C_TS_DRIVER_SHOW_INPUT_EVENTS 0
#define I2C_TS_DRIVER_SHOW_EVENT_COUNT  0

#define TS_ERR(fmt, args...) printk(KERN_ERR "[tango_i2c]: " fmt, ## args)
#ifdef TS_DEBUG
#define TS_DEBUG(fmt, args...) \
		printk(KERN_DEBUG "[tango_i2c]: " fmt, ## args)
#else
#define TS_DEBUG(fmt, args...)
#endif

/* ---- Private Variables ------------------------------------------------ */
atomic_t g_atomic_irqs_rxd = ATOMIC_INIT(0);

static struct TANGO_I2C_TS_t *gp_i2c_ts;

static struct tango_i2c *p_tango_i2c_dev;

static touch_state g_touch_state = TSC_TOUCH_UP;
static int g_last_x;
static int g_last_y;
static char *gp_buffer;
static struct input_dev *gp_input_dev;

static DECLARE_WAIT_QUEUE_HEAD(g_event_waitqueue);

static t_i2c_touch_data g_curr_touch_data;
static t_i2c_touch_data g_prev_touch_data;

static unsigned long g_num_good_events;
static unsigned long g_num_bad_events;
static unsigned long g_num_gen_up_events;
static int g_num_read_errors;
static int g_num_driver_errors;
static int g_low_power_changed;

static int g_found_slave_addr;

static int g_num_good_events_per_touch;
static int g_num_bad_events_per_touch;

/* Needed for multitouch
 * The surface X coordinate of the center of the touching ellipse */
static int g_blob_size;

static int g_tango_probe_flag;

static int mod_param_debug = (I2C_TS_DRIVER_SHOW_RAW_EVENTS << 1) &
					I2C_TS_DRIVER_SHOW_INPUT_EVENTS;
module_param(mod_param_debug, int, 0644);

/* ---- Private Function Prototypes -------------------------------------- */
static int i2c_ts_driver_read(void);
static int i2c_ts_driver_check_touch_info(void);
static void i2c_ts_driver_send_touch_info(void);
static void i2c_ts_driver_send_multitouch_info(void);
static void i2c_ts_driver_show_events(int err_no);
static int i2c_ts_driver_reset_slave(void);
static void i2c_ts_driver_remap_layout(void);
static void i2c_ts_driver_handle_i2c_error(int rc);
static int i2c_ts_driver_write(int length);
static int i2c_ts_driver_check_mod_params(void);
static ssize_t i2c_ts_driver_calibration(struct device *dev,
					 struct device_attribute *devattr,
					 const char *buf, size_t count);

static struct device_attribute dev_attr =
__ATTR(calibration, 0664, NULL, i2c_ts_driver_calibration);

static ssize_t i2c_ts_driver_calibration(struct device *dev,
					 struct device_attribute *devattr,
					 const char *buf, size_t count)
{
	int rc = 0;
	unsigned char buffer[2] = { SPECIAL_OP_OFFSET,
					SPECIAL_OP_CODE_CALIBRATE };
	unsigned long flags;

	rc = strncmp(buf, "enable", 6);
	if (rc != 0) {
		printk("Wrong command\n");
		return -1;
	}

	printk(KERN_INFO "starting calibration\n");

	rc = i2c_master_send(p_tango_i2c_dev->client, buffer, 2);
	/*
	 * Disabling interrupts till calibrated data is written into flash.
	 * This wont effect system performace because calibration is done
	 * one-time before product packaged.
	 */
	local_irq_save(flags);
	mdelay(3000);
	local_irq_restore(flags);

	if (rc < 0) {
		TS_ERR("%s Calibration failed %d\n", I2C_TS_DRIVER_NAME, rc);
		return -1;
	} else {
		/* Wait for 10ms */
		usleep_range(10000, 10500);
		printk(KERN_INFO "Calibration done\n");
		return count;
	}
}

/* ---- Functions -------------------------------------------------------- */

#if defined(CONFIG_PM) || defined(CONFIG_HAS_EARLYSUSPEND)
/* In preparation for implementing PM_SUSPEND_STANDBY. */
static int i2c_ts_suspend_driver(struct i2c_client *p_client, pm_message_t mesg)
{/* Can put it into deep sleep only if the slave can be reset to bring
							   it out. */
	int rc = 0;
	if (gp_i2c_ts->is_resetable) {
		gp_buffer[0] = gp_i2c_ts->power_mode_idx;
		gp_buffer[1] = POWER_MODE_FREEZE;
		rc = i2c_ts_driver_write(2);
	}
	disable_irq(gpio_to_irq(gp_i2c_ts->gpio_irq_pin));
	return 0;
}

static int i2c_ts_resume_driver(struct i2c_client *p_client)
{
	int rc = 0;
	if (gp_i2c_ts->is_resetable) {
		i2c_ts_driver_reset_slave();
		rc = i2c_ts_driver_check_mod_params();
	}
	enable_irq(gpio_to_irq(gp_i2c_ts->gpio_irq_pin));
	return rc;
}
#else
#define i2c_ts_suspend_driver NULL
#define i2c_ts_resume_driver NULL
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
static void i2c_ts_early_suspend(struct early_suspend *desc)
{
	struct tango_i2c *data = container_of(desc,
					      struct tango_i2c, suspend_desc);
	pm_message_t mesg = {
		.event = PM_EVENT_SUSPEND,
	};

	i2c_ts_suspend_driver(data->client, mesg);
	cancel_delayed_work_sync(&data->work);
}

static void i2c_ts_late_resume(struct early_suspend *desc)
{
	struct tango_i2c *data = container_of(desc,
					      struct tango_i2c, suspend_desc);
	i2c_ts_resume_driver(data->client);
}
#endif

static int i2c_ts_driver_check_mod_params(void)
{
	int rc = 0;

	if (gp_i2c_ts == NULL)
		return -1;

	if (g_low_power_changed > 0) {
		/* User wants to change the auto low power mode. */
		g_low_power_changed = 0;
		gp_buffer[0] = gp_i2c_ts->power_mode_idx;
		gp_buffer[1] = POWER_MODE_ACTIVE | POWER_MODE_ALLOW_SLEEP |
				POWER_MODE_SLEEP_PERIOD;
		gp_buffer[2] = INT_MODE_ACT_LOW_TOUCH;
		rc = i2c_ts_driver_write(3);
		if ((rc > 0) && mod_param_debug)
			TS_DEBUG("%s set auto low power to 0x%x\n",
				 I2C_TS_DRIVER_NAME, gp_buffer[1]);
	}

	if (rc < 0) {
		TS_ERR("%s error detected when writing slave settings %d\n",
		       I2C_TS_DRIVER_NAME, rc);
		return rc;
	} else {
		return 0;
	}
}

int is_pen_down(void)
{
	struct i2c_client *client;

	client = p_tango_i2c_dev->client;
	return i2c_smbus_read_byte_data(client, gp_i2c_ts->num_fingers_idx);
}

static void tango_i2c_wq(struct work_struct *work)
{
	struct tango_i2c *tango_dev = container_of(to_delayed_work(work),
						   struct tango_i2c, work);

	mutex_lock(&tango_dev->mutex_wq);
	TS_DEBUG("tango_i2c_wq run\n");

	if (i2c_ts_driver_read() > 0)
		i2c_ts_driver_check_touch_info();
	schedule();

	TS_DEBUG("tango_i2c_wq leave\n");
	mutex_unlock(&tango_dev->mutex_wq);

	if (is_pen_down() > 0)
		queue_delayed_work(tango_dev->ktouch_wq,
				   &tango_dev->work, HZ / 50);
}

static irqreturn_t i2c_ts_driver_isr(int irq, void *dev_id)
{
	struct tango_i2c *tango_dev = (struct tango_i2c *)dev_id;

	/* postpone I2C transactions to the workqueue as it may block */
	queue_delayed_work(tango_dev->ktouch_wq, &tango_dev->work, 0);

	return IRQ_HANDLED;
}

int i2c_ts_driver_read(void)
{
	int rc = 0;
	int i = 0;
	struct i2c_client *client;

	if (p_tango_i2c_dev == NULL || p_tango_i2c_dev->client == NULL) {
		TS_ERR("i2c_driver_read() p_tango_i2c_dev->client == NULL\n");
		return -1;
	}

	client = p_tango_i2c_dev->client;
	rc = i2c_smbus_read_i2c_block_data(client,
					   MSI_REG_OFFSET,
					   gp_i2c_ts->num_bytes_to_read,
					   gp_buffer);

	if (rc != gp_i2c_ts->num_bytes_to_read) {
		TS_ERR("%s i2c_ts_driver_read() failed %d\n",
		       I2C_TS_DRIVER_NAME, rc);
		g_num_read_errors++;
		i2c_ts_driver_handle_i2c_error(rc);
		return rc;
	}

	g_num_read_errors = 0;

	g_curr_touch_data.pressure = gp_buffer[gp_i2c_ts->pressure_lo_idx] |
				(gp_buffer[gp_i2c_ts->pressure_hi_idx] << 8);

	if (g_curr_touch_data.pressure > gp_i2c_ts->max_pressure)
		g_curr_touch_data.pressure = gp_i2c_ts->max_pressure;
	else if (g_curr_touch_data.pressure == 0)
		g_curr_touch_data.pressure = 1;

	g_curr_touch_data.x1 = (gp_buffer[gp_i2c_ts->x1_hi_idx] << 8) |
	    gp_buffer[gp_i2c_ts->x1_lo_idx];
	g_curr_touch_data.y1 = (gp_buffer[gp_i2c_ts->y1_hi_idx] << 8) |
	    gp_buffer[gp_i2c_ts->y1_lo_idx];

	if (gp_i2c_ts->is_multi_touch) {
		if (gp_i2c_ts->x2_hi_idx >= 0 && gp_i2c_ts->x2_lo_idx >= 0) {
			g_curr_touch_data.x2 =
			    (gp_buffer[gp_i2c_ts->x2_hi_idx] << 8) |
			    gp_buffer[gp_i2c_ts->x2_lo_idx];
		} else {
			g_curr_touch_data.x2 = -1;
		}

		if (gp_i2c_ts->y2_hi_idx >= 0 && gp_i2c_ts->y2_lo_idx >= 0) {
			g_curr_touch_data.y2 =
			    (gp_buffer[gp_i2c_ts->y2_hi_idx] << 8) |
			    gp_buffer[gp_i2c_ts->y2_lo_idx];
		} else {
			g_curr_touch_data.y2 = -1;
		}
	}

	g_curr_touch_data.num_fingers =
	    0x3 & gp_buffer[gp_i2c_ts->num_fingers_idx];

	if (mod_param_debug & 0x20) {
		for (i = 0; i < gp_i2c_ts->num_bytes_to_read; i++)
			TS_DEBUG("%2x ", gp_buffer[i]);
		TS_DEBUG("\n");
	}

	/* Check before going to sleep if the user changed any parameters.
	 * Needs to be here. It will not work if device is idle.
	 */
	i2c_ts_driver_check_mod_params();

	return rc;
}

int i2c_ts_driver_write(int length)
{
	int rc;
	struct i2c_client *client;

	client = p_tango_i2c_dev->client;
	rc = i2c_master_send(client, gp_buffer, length);
	return rc;
}

void i2c_ts_driver_handle_i2c_error(int rc)
{
	if (mod_param_debug > 0) {
		TS_ERR("%s I2C error, rc %d # read errors %d"
		       "# known driver errors %d\n",
		       I2C_TS_DRIVER_NAME, rc,
		       g_num_read_errors, g_num_driver_errors);
	}

	if (rc != 0) {		/* Was called by i2c_ts_driver_read(). */
		if (g_num_read_errors < MAX_NUMBER_READ_ERRORS) {
			TS_ERR("%s I2C read error %d, error %d\n",
			       I2C_TS_DRIVER_NAME, g_num_read_errors, rc);
		} else if (g_num_read_errors == MAX_NUMBER_READ_ERRORS) {
			TS_ERR("%s maximum # I2C read errors reached %d, "
				"error %d\n",
				I2C_TS_DRIVER_NAME, g_num_read_errors, rc);
		} else {
			return;
		}
	} else {
		g_num_driver_errors++;
	}

	if (gp_i2c_ts->is_resetable) {
		TS_ERR("%s i2c_ts_driver_handle_i2c_error() resetting "
		       "I2C slave at 0x%x\n",
		       I2C_TS_DRIVER_NAME, g_found_slave_addr);
		msleep(50);
		i2c_ts_driver_reset_slave();
		msleep(50);
		rc = i2c_ts_driver_check_mod_params();
	} else {
		TS_ERR
		    ("%s I2C bus has problems but cannot reset slave at 0x%x\n",
		     I2C_TS_DRIVER_NAME, g_found_slave_addr);
	}

	if (rc == -EREMOTEIO) {
		/* Indicates a problem with the bus.
		 * Reset the I2C master controller.
		 */
		TS_DEBUG("%s detected remote IO problem but cannot reset "
			 "I2C bus master\n", I2C_TS_DRIVER_NAME);
	}
}

/* Have to ensure that the values read over the I2C bus are in range. */
int i2c_ts_driver_check_touch_info(void)
{
	int rc = 0;
	int touch_state_changed = 0;

	/* The number of fingers have to be checked because when the number of
	 * fingers is zero, i.e. no fingers are in contact, the x and y may be
	 * out of range.
	 */
	if (g_curr_touch_data.x1 > gp_i2c_ts->x_max_value &&
		g_curr_touch_data.num_fingers > 0) {
		rc = kErrorX1;
	}

	if (g_curr_touch_data.y1 > gp_i2c_ts->y_max_value &&
	    g_curr_touch_data.num_fingers > 0) {
		rc |= kErrorY1;
	}

	if (gp_i2c_ts->is_multi_touch && g_curr_touch_data.num_fingers > 1) {
		if (g_curr_touch_data.x2 > gp_i2c_ts->x_max_value)
			rc |= kErrorX2;

		if (g_curr_touch_data.y2 > gp_i2c_ts->y_max_value)
			rc |= kErrorY2;
	}

	if (g_curr_touch_data.num_fingers > gp_i2c_ts->max_finger_val)
		rc |= kErrorNumFingers;

	if (g_curr_touch_data.num_fingers == 0) {
		if (g_touch_state == TSC_TOUCH_UP) {
			rc |= kErrorTouchState;
		} else {
		/* No fingers are touching the screen, change the state. */
			g_touch_state = TSC_TOUCH_UP;
			touch_state_changed = 1;
		}
	}

	if (rc == 0) {
		/* No error so far, check the touch state
		 * and number of fingers */
		if (g_curr_touch_data.num_fingers > 0 &&
		    g_curr_touch_data.num_fingers <=
			gp_i2c_ts->max_finger_val) {
			if (g_touch_state == TSC_TOUCH_UP) {
				/* At least one finger is touching the screen,
				 * change the state. */
				g_touch_state = TSC_TOUCH_DOWN;
				touch_state_changed = 1;
			}
		}

		g_num_good_events_per_touch++;

	}

	if (rc != 0) {
		if (mod_param_debug & 0x2) {
			printk
			(KERN_INFO "%2x %2x %2x %2x %2x %2x %2x"
			"%2x %2x %2x %2x %2x "
			"E:0x%x irqs:%d\n", gp_buffer[0], gp_buffer[1],
			gp_buffer[2], gp_buffer[3], gp_buffer[4],
			gp_buffer[5], gp_buffer[6], gp_buffer[7],
			gp_buffer[8], gp_buffer[9], gp_buffer[10],
			gp_buffer[11], rc,
			atomic_read(&g_atomic_irqs_rxd));
		}

		i2c_ts_driver_show_events(rc);
		g_num_bad_events++;
		g_num_bad_events_per_touch++;
		return rc;
	}

	if (mod_param_debug & 0x2) {
		printk(KERN_INFO "%2x %2x %2x %2x %2x %2x %2x %2x"
		       "%2x %2x %2x %2x %d\n",
		       gp_buffer[0], gp_buffer[1], gp_buffer[2], gp_buffer[3],
		       gp_buffer[4], gp_buffer[5], gp_buffer[6], gp_buffer[7],
		       gp_buffer[8], gp_buffer[9], gp_buffer[10], gp_buffer[11],
		       atomic_read(&g_atomic_irqs_rxd));
	}

	g_prev_touch_data = g_curr_touch_data;
	i2c_ts_driver_send_touch_info();

	i2c_ts_driver_show_events(rc);

	g_num_good_events++;

	if ((g_num_good_events % 100) == 0 &&
	    (I2C_TS_DRIVER_SHOW_ALL_EVENTS || I2C_TS_DRIVER_SHOW_EVENT_COUNT)) {
		printk(KERN_INFO "# good events %lu # bad events"
			" %lu gen up events %lu\n",
		       g_num_good_events, g_num_bad_events,
		       g_num_gen_up_events);
	}

	return rc;
}

void i2c_ts_driver_remap_layout(void)
{
	if (gp_i2c_ts->layout == X_RIGHT_Y_UP) {
		g_curr_touch_data.y1 =
		    gp_i2c_ts->y_max_value - g_curr_touch_data.y1;
		g_curr_touch_data.y2 =
		    gp_i2c_ts->y_max_value - g_curr_touch_data.y2;
	}
}

void i2c_ts_driver_show_events(int err_no)
{
	if (err_no) {
		if (I2C_TS_DRIVER_SHOW_ALL_EVENTS
		    || I2C_TS_DRIVER_SHOW_ERR_EVENTS) {
			printk
			    (KERN_ERR "x1:%5d y1:%5d x2:%5d y2:%5d f:%d v:%2d "
			     "i:%3d t:%3d ERR! 0x%x\n",
			     g_curr_touch_data.x1,
			     g_curr_touch_data.y1, g_curr_touch_data.x2,
			     g_curr_touch_data.y2,
			     g_curr_touch_data.num_fingers,
			     g_curr_touch_data.version, g_curr_touch_data.idle,
			     g_curr_touch_data.timeout, err_no);
		}
		return;
	}

	if (I2C_TS_DRIVER_SHOW_ALL_EVENTS == 0
	    && I2C_TS_DRIVER_SHOW_OK_EVENTS == 0) {
		return;
	}

	if (g_touch_state == TSC_TOUCH_DOWN) {
		printk
		    (KERN_INFO "x1:%4d y1:%4d x2:%4d y2:%4d f:%d v:%2d "
		     "i:%3d t:%3d(on)\n",
		     g_curr_touch_data.x1, g_curr_touch_data.y1,
		     g_curr_touch_data.x2, g_curr_touch_data.y2,
		     g_curr_touch_data.num_fingers, g_curr_touch_data.version,
		     g_curr_touch_data.idle, g_curr_touch_data.timeout);
	} else {
		printk
		    (KERN_INFO "x1:%4d y1:%4d x2:%4d y2:%4d f:%d v:%2d "
		     "i:%3d t:%3d(off)\n",
		     g_curr_touch_data.x1, g_curr_touch_data.y1,
		     g_curr_touch_data.x2, g_curr_touch_data.y2,
		     g_curr_touch_data.num_fingers, g_curr_touch_data.version,
		     g_curr_touch_data.idle, g_curr_touch_data.timeout);
	}
}

void i2c_ts_driver_send_touch_info(void)
{
	if (g_touch_state == TSC_TOUCH_UP) {
		if (mod_param_debug & 0x10) {
			printk(KERN_INFO "finger up, # good %d # bad %d\n",
			       g_num_good_events_per_touch,
			       g_num_bad_events_per_touch);
		} else if (mod_param_debug & 0x1) {
			printk(KERN_INFO "finger up\n");
		}

		g_num_good_events_per_touch = 0;
		g_num_bad_events_per_touch = 0;

		if (gp_i2c_ts->is_multi_touch)
			input_mt_sync(gp_input_dev);

		input_sync(gp_input_dev);
		return;
	}

	if (gp_i2c_ts->layout != X_RIGHT_Y_DOWN) {
		/* The x and y values have to be changed so x goes
		 * from left to right and y from top to bottom. */
		i2c_ts_driver_remap_layout();
	}

	if (gp_i2c_ts->is_multi_touch) {
		/* Handles single or multi touches. */
		i2c_ts_driver_send_multitouch_info();
	} else {
		/* x and y values have already been checked that they
		 * in range and have been put into the proper layout.
		 */
		if (mod_param_debug & 0x1) {
			printk(KERN_INFO "finger down, x: %4d y: %4d\n",
			       g_curr_touch_data.x1, g_curr_touch_data.y1);
		}

		/* Good data, send it off */
		g_last_x = g_curr_touch_data.x1;
		g_last_y = g_curr_touch_data.y1;

		input_report_abs(gp_input_dev, ABS_PRESSURE,
				 INPUT_EVENT_PRESSURE);
		input_report_abs(gp_input_dev, ABS_X, g_curr_touch_data.x1);
		input_report_abs(gp_input_dev, ABS_Y, g_curr_touch_data.y1);
		input_report_key(gp_input_dev, BTN_TOUCH, 1);
		input_sync(gp_input_dev);
	}
}

/*
 * Here is what a minimal event sequence for a two-finger touch would look
 * like:
 *	ABS_MT_TOUCH_MAJOR
 *	ABS_MT_POSITION_X
 *	ABS_MT_POSITION_Y
 *	SYN_MT_REPORT
 *	ABS_MT_TOUCH_MAJOR
 *	ABS_MT_POSITION_X
 *	ABS_MT_POSITION_Y
 *	SYN_MT_REPORT
 *	SYN_REPORT
 *
 * Example
 * x 0 - 800, y 0 - 240, finger size 20
 *
 * +--------------------------------------------------------------------------+
 * |                                             blob/ellipse 1 in contact    |
 * |                                                         -   (640,30)     |
 * |            +-------------------------------------------| |               |
 * |            |                                            -                |
 * |            |                                            |                |
 * |            |                                            |                |
 * |            |                                            |                |
 * |            |                                            |                |
 * |            |                                            |                |
 * |            |                                            |                |
 * |            |                                            |                |
 * |            |                                            |                |
 * |            |                                            |                |
 * |            -                                            |                |
 * |           | |-------------------------------------------+                |
 * |            -                                                             |
 * |   blob/ellipse 2 in contact                                              |
 * |      (130,150)                                                           |
 * |            ABS_MT_TOUCH_MAJOR = 20 (ellipse in contact with screen)      |
 * |            ABS_MT_POSITION_X 1 = 640 ABS_MT_POSITION_X 2 = 130           |
 * |            ABS_MT_POSITION_Y 1 = 30  ABS_MT_POSITION_Y 2 = 150           |
 * +--------------------------------------------------------------------------+
 */
void i2c_ts_driver_send_multitouch_info(void)
{
	if (mod_param_debug & 0x1) {
		printk(KERN_INFO "%d fingers x1: %4d y1: %4d x2: %4d y2: %4d\n",
		       g_curr_touch_data.num_fingers,
		       g_curr_touch_data.x1, g_curr_touch_data.y1,
		       g_curr_touch_data.x2, g_curr_touch_data.y2);
	}
	/* Step 1: ABS_MT_TOUCH_MAJOR
	 * The length of the major axis of the contact.
	 * Assume to be circular so _MINOR is not set. */
	input_report_abs(gp_input_dev, ABS_MT_TOUCH_MAJOR, g_blob_size);

	/* Step 2: ABS_MT_POSITION_X */
	/* The surface X coordinate of the center of the touching ellipse. */
	input_report_abs(gp_input_dev, ABS_MT_POSITION_X, g_curr_touch_data.x1);
	/* Step 3: ABS_MT_POSITION_Y
	 * The surface Y coordinate of the center of the touching ellipse. */
	input_report_abs(gp_input_dev, ABS_MT_POSITION_Y, g_curr_touch_data.y1);

	input_report_abs(gp_input_dev, ABS_MT_PRESSURE,
				 g_curr_touch_data.pressure);

	/* Step 4: SYN_MT_REPORT */
	input_mt_sync(gp_input_dev);

	if (g_curr_touch_data.num_fingers > 1) {
		/* Step 5: ABS_MT_TOUCH_MAJOR
		 * The length of the major axis of the contact. */
		input_report_abs(gp_input_dev, ABS_MT_TOUCH_MAJOR, g_blob_size);
		/* Step 6: ABS_MT_POSITION_X
		 * The surface X coordinate of the center of the
		 * touching ellipse. */
		input_report_abs(gp_input_dev, ABS_MT_POSITION_X,
				 g_curr_touch_data.x2);
		/* Step 7: ABS_MT_POSITION_Y
		 * The surface Y coordinate of the center of the
		 * touching ellipse. */
		input_report_abs(gp_input_dev, ABS_MT_POSITION_Y,
				 g_curr_touch_data.y2);

		input_report_abs(gp_input_dev, ABS_MT_PRESSURE,
				 g_curr_touch_data.pressure);

		/* Step 8: SYN_MT_REPORT */
		input_mt_sync(gp_input_dev);
	}

	/* Step 9: SYN_REPORT */
	input_sync(gp_input_dev);
}

static int i2c_ts_driver_reset_slave(void)
{
	int rc = 0;

	if (gp_i2c_ts->is_resetable == 0) {
		/* Slave does not have a reset pin. */
		return rc;
	}

	gpio_set_value(gp_i2c_ts->gpio_reset_pin, I2C_TS_DRIVER_DO_RESET);
	usleep_range(GPIO_I2C_RESET_DELAY_USECS,
		     GPIO_I2C_RESET_DELAY_USECS + 500);
	gpio_set_value(gp_i2c_ts->gpio_reset_pin, I2C_TS_DRIVER_DONT_RESET);
	usleep_range(GPIO_I2C_RESET_DELAY_USECS,
		     GPIO_I2C_RESET_DELAY_USECS + 500);

	/* Rewrite these settings following reset. */
	/* After Tango controller gets reset, it holds interrupt pin low for
	 * about 150ms. During this interrupt-pin holding period, it won't ACK
	 * to any I2C packet */
	g_low_power_changed = 1;
	usleep_range(GPIO_I2C_RESET_DELAY_USECS*20,
		GPIO_I2C_RESET_DELAY_USECS*25);

	return rc;
}

static int i2c_ts_driver_probe(struct i2c_client *p_i2c_client,
			       const struct i2c_device_id *id)
{
	int rc = 0;

	struct TANGO_I2C_TS_t *dt_i2c_ts = NULL;
	struct device_node *np = NULL;
	u32 val;
	if (p_i2c_client == NULL) {
		TS_ERR("%s i2c_ts_driver_probe() p_i2c_client == NULL\n",
		       I2C_TS_DRIVER_NAME);
		rc = -1;
		goto ERROR1;
	}

	np = p_i2c_client->dev.of_node;
	dt_i2c_ts = kzalloc(sizeof(struct TANGO_I2C_TS_t), GFP_KERNEL);
	if (!dt_i2c_ts) {
		printk(KERN_ERR "Unable to allocate memory for *dt_i2c_ts *\n");
		return -ENOMEM;
	}
	if (p_i2c_client->dev.of_node != NULL) {
		if (!of_property_read_u32(np, "gpio-irq-pin", &val))
			dt_i2c_ts->gpio_irq_pin = val;

		if (!of_property_read_u32(np, "gpio-reset-pin", &val))
			dt_i2c_ts->gpio_reset_pin = val;

		if (!of_property_read_u32(np, "x-max-value", &val))
			dt_i2c_ts->x_max_value = val;

		if (!of_property_read_u32(np, "y-max-value", &val))
			dt_i2c_ts->y_max_value = val;

		if (!of_property_read_u32(np, "layout", &val))
			dt_i2c_ts->layout = val;

		if (!of_property_read_u32(np, "num-bytes-to-read", &val))
			dt_i2c_ts->num_bytes_to_read = val;

		if (!of_property_read_u32(np, "is-multi-touch", &val))
			dt_i2c_ts->is_multi_touch = val;

		if (!of_property_read_u32(np, "is-resetable", &val))
			dt_i2c_ts->is_resetable = val;

		if (!of_property_read_u32(np, "num-fingers-idx", &val))
			dt_i2c_ts->num_fingers_idx = val;

		if (!of_property_read_u32(np, "old-touching-idx", &val))
			dt_i2c_ts->old_touching_idx = val;

		if (!of_property_read_u32(np, "x1-lo-idx", &val))
			dt_i2c_ts->x1_lo_idx = val;

		if (!of_property_read_u32(np, "x1-hi-idx", &val))
			dt_i2c_ts->x1_hi_idx = val;

		if (!of_property_read_u32(np, "y1-lo-idx", &val))
			dt_i2c_ts->y1_lo_idx = val;

		if (!of_property_read_u32(np, "y1-hi-idx", &val))
			dt_i2c_ts->y1_hi_idx = val;

		if (!of_property_read_u32(np, "x2-lo-idx", &val))
			dt_i2c_ts->x2_lo_idx = val;

		if (!of_property_read_u32(np, "x2-hi-idx", &val))
			dt_i2c_ts->x2_hi_idx = val;

		if (!of_property_read_u32(np, "y2-lo-idx", &val))
			dt_i2c_ts->y2_lo_idx = val;

		if (!of_property_read_u32(np, "y2-hi-idx", &val))
			dt_i2c_ts->y2_hi_idx = val;

		if (!of_property_read_u32(np, "x1-width-idx", &val))
			dt_i2c_ts->x1_width_idx = val;

		if (!of_property_read_u32(np, "y1-width-idx", &val))
			dt_i2c_ts->y1_width_idx = val;

		if (!of_property_read_u32(np, "x2-width-idx", &val))
			dt_i2c_ts->x2_width_idx = val;

		if (!of_property_read_u32(np, "y2-width-idx", &val))
			dt_i2c_ts->y2_width_idx = val;

		if (!of_property_read_u32(np, "pressure-lo-idx", &val))
			dt_i2c_ts->pressure_lo_idx = val;

		if (!of_property_read_u32(np, "pressure-hi-idx", &val))
			dt_i2c_ts->pressure_hi_idx = val;

		if (!of_property_read_u32(np, "power-mode-idx", &val))
			dt_i2c_ts->power_mode_idx = val;

		if (!of_property_read_u32(np, "int-mode-idx", &val))
			dt_i2c_ts->int_mode_idx = val;

		if (!of_property_read_u32(np, "int-width-idx", &val))
			dt_i2c_ts->int_width_idx = val;

		if (!of_property_read_u32(np, "min-finger-val", &val))
			dt_i2c_ts->min_finger_val = val;

		if (!of_property_read_u32(np, "max-finger-val", &val))
			dt_i2c_ts->max_finger_val = val;

		if (!of_property_read_u32(np, "panel-width", &val))
			dt_i2c_ts->panel_width = val;

		if (!of_property_read_u32(np, "max-pressure", &val))
			dt_i2c_ts->max_pressure = val;

		if (!of_property_read_u32(np, "client_func_magic", &val))
			dt_i2c_ts->i2c_pdata.client_func_magic = val;

		if (!of_property_read_u32(np, "client_func_map", &val))
			dt_i2c_ts->i2c_pdata.client_func_map = val;

	}

	if (!i2c_check_functionality(p_i2c_client->adapter,
				     I2C_FUNC_SMBUS_READ_I2C_BLOCK)) {
		TS_ERR("%s: i2c_ts_driver_probe() "
		       "i2c_check_functionality() failed %d\n",
		       I2C_TS_DRIVER_NAME, -ENODEV);
		rc = ENODEV;
		goto ERROR1;
	}

	if (g_found_slave_addr > 0) {
		TS_ERR("%s i2c_ts_driver_probe() i2c slave already"
			" found at 0x%x\n",
			I2C_TS_DRIVER_NAME, g_found_slave_addr);
		rc = -1;
		goto ERROR1;
	}

	/* Get the I2C information compiled in for this platform. */
	gp_i2c_ts = (struct TANGO_I2C_TS_t *)p_i2c_client->dev.platform_data;

	if (p_i2c_client->dev.of_node != NULL)
		gp_i2c_ts = dt_i2c_ts;

	if (gp_i2c_ts == NULL) {
		/* Cannot access platform data. */
		TS_ERR("%s:%s Cannot access platform data for I2C"
			"slave address %d\n",
			I2C_TS_DRIVER_NAME, __func__, p_i2c_client->addr);
		rc = -1;
		goto ERROR1;
	}

	printk(KERN_INFO "%s: slave address 0x%x\n", I2C_TS_DRIVER_NAME,
	       p_i2c_client->addr);
	printk(KERN_INFO "%s: max x	0x%x\n", I2C_TS_DRIVER_NAME,
	       gp_i2c_ts->x_max_value);
	printk(KERN_INFO "%s: max y 0x%x\n", I2C_TS_DRIVER_NAME,
	       gp_i2c_ts->y_max_value);
	printk(KERN_INFO "%s: is multitouch %d\n", I2C_TS_DRIVER_NAME,
	       gp_i2c_ts->is_multi_touch);

	/* Assign the finger touch size. */
	g_blob_size = I2C_TS_DRIVER_BLOB_SIZE *
	    gp_i2c_ts->x_max_value / gp_i2c_ts->panel_width;

	if (gp_i2c_ts->is_multi_touch) {
		/* Blob is the size of the finger contact. */
		printk(KERN_INFO "%s: blob size  %d\n", I2C_TS_DRIVER_NAME,
		       g_blob_size);
	}

	/* Rest of the initialisation goes here. */
	if (gp_i2c_ts->is_resetable) {

		rc = gpio_request(gp_i2c_ts->gpio_reset_pin,
				       "i2c-driver reset");
		if (rc != 0) {
			TS_ERR("gpio_request() failed, rc = %d\n", rc);
			goto ERROR1;
		}

		rc = gpio_direction_output(gp_i2c_ts->gpio_reset_pin,
						I2C_TS_DRIVER_DONT_RESET);
		if (rc != 0) {
			TS_ERR("gpio_direction_output(%d,"
				"I2C_TS_DRIVER_DONT_RESET) error %d\n",
				gp_i2c_ts->gpio_reset_pin, rc);
			gpio_free(gp_i2c_ts->gpio_reset_pin);
			goto ERROR1;
		}
	}

	if (gp_i2c_ts->is_resetable) {
		rc = i2c_ts_driver_reset_slave();
		/* This sets values on the slave. If the slave is not there it
		 * will fail ensuring the slave address is valid. */
	}

	/* Create some space to store the I2C bytes read from the slave. */
	gp_buffer = kzalloc(gp_i2c_ts->num_bytes_to_read, GFP_KERNEL);

	if (!gp_buffer) {
		TS_ERR("i2c_ts_driver_probe() kzalloc() returned NULL\n");
		rc = ENOMEM;
		goto ERROR1;
	}

	gp_buffer[0] = gp_i2c_ts->power_mode_idx;
	gp_buffer[1] = POWER_MODE_ACTIVE | POWER_MODE_ALLOW_SLEEP |
		       POWER_MODE_SLEEP_PERIOD;
	gp_buffer[2] = INT_MODE_ACT_LOW_TOUCH;

	p_tango_i2c_dev->client = p_i2c_client;

	g_low_power_changed = 1;
	rc = i2c_ts_driver_check_mod_params();

	if (rc < 0) {
		/* This also ensures that the slave is actually there! */
		TS_ERR("%s i2c_ts_driver_probe() failed to write"
			" to slave, rc = %d\n",
			I2C_TS_DRIVER_NAME, rc);
		g_tango_probe_flag = 1;
		gpio_free(gp_i2c_ts->gpio_reset_pin);
		goto ERROR2;
	} else {
		rc = 0;
	}

	rc = device_create_file(&p_i2c_client->dev, &dev_attr);
	if (rc) {
		TS_ERR("%s:%s Cannot create sysfs entry\n",
			I2C_TS_DRIVER_NAME, __func__);
		goto ERROR2;
	}

	mutex_init(&p_tango_i2c_dev->mutex_wq);
	p_tango_i2c_dev->ktouch_wq = create_workqueue("tango_touch_wq");
	INIT_DELAYED_WORK(&p_tango_i2c_dev->work, tango_i2c_wq);
	i2c_set_clientdata(p_i2c_client, p_tango_i2c_dev);
	p_i2c_client->dev.platform_data = &dt_i2c_ts->i2c_pdata;
#ifdef CONFIG_HAS_EARLYSUSPEND
	p_tango_i2c_dev->suspend_desc.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN,
	    p_tango_i2c_dev->suspend_desc.suspend = i2c_ts_early_suspend,
	    p_tango_i2c_dev->suspend_desc.resume = i2c_ts_late_resume,
	    register_early_suspend(&p_tango_i2c_dev->suspend_desc);
#endif

	gpio_request(gp_i2c_ts->gpio_irq_pin, "i2c touch screen driver");
	if (rc != 0) {
		TS_ERR("gpio_request(%d) failed, rc = %d\n",
		       gp_i2c_ts->gpio_irq_pin, rc);
		goto ERROR2;
	}
	rc = gpio_direction_input(gp_i2c_ts->gpio_irq_pin);
	if (rc != 0) {
		TS_ERR("gpio_direction_input(%d, ) " "error %d\n",
		       gp_i2c_ts->gpio_irq_pin, rc);
		goto ERROR3;
	}

	rc = request_irq(gpio_to_irq(gp_i2c_ts->gpio_irq_pin),
			      i2c_ts_driver_isr,
			      (IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING),
			      "GPIO cap touch screen irq",
			      p_tango_i2c_dev);
	if (rc < 0) {
		TS_ERR("request_irq(%d) failed, rc = %d\n",
		       gp_i2c_ts->gpio_irq_pin, rc);
		goto ERROR3;
	}

	/* Try to use the gpio pin to reset the I2C slave device prior to
	 * being probed. Setup the gpio for handling interrupt requests and
	 *  the reset pin if used based on platform_data. */

	/* The following code initializes the input system so events can be
	 * passed from the touch controller up to Android.
	 */
	gp_input_dev = input_allocate_device();

	if (gp_input_dev == NULL) {
		TS_ERR("%s i2c_ts_driver_probe() input_allocate_device() "
		       "allocation failed\n", I2C_TS_DRIVER_NAME);
		rc = ENOMEM;
		goto ERROR4;
	}

	/* Set input device info. */
	gp_input_dev->name = I2C_TS_DRIVER_NAME;
	gp_input_dev->phys = "ts/input1";
	gp_input_dev->id.bustype = BUS_I2C;
	gp_input_dev->id.vendor = 0x0001;
	gp_input_dev->id.product = 0x0001;
	gp_input_dev->id.version = 0x0001;

	/* Enable event bits. */
	set_bit(EV_SYN, gp_input_dev->evbit);
	set_bit(EV_ABS, gp_input_dev->evbit);

	/* No need to set BTN_TOUCH and KEY press capabilities here : ssp
	 *
	 * set_bit(EV_KEY, gp_input_dev->evbit);
	 * set_bit(BTN_TOUCH, gp_input_dev->keybit);
	 *
	 * Added by "ssp" looking at Android EventHub.cpp, but turns out,
	 * we dont need this as well.
	 * set_bit(ABS_MT_POSITION_X, gp_input_dev->absbit);
	 * set_bit(ABS_MT_POSITION_Y, gp_input_dev->absbit);
	 */

	/*
	 * Again, This is not needed : ssp
	 *
	 * input_set_abs_params(gp_input_dev, ABS_X, 0,
	 *                                        gp_i2c_ts->x_max_value, 0, 0);
	 * input_set_abs_params(gp_input_dev, ABS_Y, 0,
	 *                                        gp_i2c_ts->y_max_value, 0, 0);
	 * input_set_abs_params(gp_input_dev, ABS_PRESSURE, 0,
	 *                                        INPUT_EVENT_PRESSURE, 0, 0);
	 * input_set_abs_params(gp_input_dev, ABS_TOOL_WIDTH, 0,
	 *                                              g_blob_size, 0, 0);
	 */

	input_set_abs_params(gp_input_dev, ABS_MT_POSITION_X, 0,
			     gp_i2c_ts->x_max_value, 0, 0);
	input_set_abs_params(gp_input_dev, ABS_MT_POSITION_Y, 0,
			     gp_i2c_ts->y_max_value, 0, 0);
	input_set_abs_params(gp_input_dev, ABS_MT_TOUCH_MAJOR, 0,
			     g_blob_size, 0, 0);
	input_set_abs_params(gp_input_dev,  ABS_MT_PRESSURE, 0,
				dt_i2c_ts->max_pressure, 0, 0);
	__set_bit(INPUT_PROP_DIRECT,
		  (volatile unsigned long *)&gp_input_dev->propbit);

	rc = input_register_device(gp_input_dev);
	if (rc < 0) {
		TS_ERR("%s i2c_ts_driver_probe() input_register_device() "
		       "allocation failed\n", I2C_TS_DRIVER_NAME);
		rc = ENOMEM;
		goto ERROR5;
	} else
		rc = 0;

	g_found_slave_addr = p_i2c_client->addr;
	return rc;

ERROR5:
	input_free_device(gp_input_dev);
ERROR4:
	free_irq(gp_i2c_ts->gpio_irq_pin, p_tango_i2c_dev);
ERROR3:
	device_remove_file(&p_i2c_client->dev, &dev_attr);
	gpio_free(gp_i2c_ts->gpio_irq_pin);
ERROR2:
	kfree(gp_buffer);
ERROR1:
	kfree(dt_i2c_ts);
	return rc;
}

static int __devexit i2c_ts_driver_remove(struct i2c_client *client)
{
	struct tango_i2c *state = i2c_get_clientdata(client);

	device_remove_file(&client->dev, &dev_attr);
	kfree(state);

	if (gp_i2c_ts->is_resetable)
		gpio_free(gp_i2c_ts->gpio_reset_pin);

	gpio_free(gp_i2c_ts->gpio_irq_pin);
	free_irq(gp_i2c_ts->gpio_irq_pin, p_tango_i2c_dev);

	/* Free all the memory that was allocated. */
	if (p_tango_i2c_dev->client != NULL)
		kfree(p_tango_i2c_dev->client);

	if (p_tango_i2c_dev != NULL)
		kfree(p_tango_i2c_dev);

	if (gp_input_dev != NULL) {
		input_unregister_device(gp_input_dev);
		input_free_device(gp_input_dev);
	}

	if (gp_buffer != NULL)
		kfree(gp_buffer);

	return 0;
}

/* End of if using .probe in i2c_driver. */

static struct i2c_device_id tango_i2c_idtable[] = {
	{"tango_ts", 0},
	{}
};

static const struct of_device_id tango_ts_match[] = {
	{ .compatible = "bcm,tango_ts" },
};

static struct i2c_driver tango_i2c_driver = {
	.driver = {
		   .name = "tango_ts",
		   .owner = THIS_MODULE,
		   .of_match_table = tango_ts_match,
		   },
	.id_table = tango_i2c_idtable,
	.class = I2C_CLASS_TOUCHSCREEN,
	.probe = i2c_ts_driver_probe,
	.remove = __devexit_p(i2c_ts_driver_remove),
#ifndef CONFIG_HAS_EARLYSUSPEND
	.suspend = i2c_ts_suspend_driver,
	.resume = i2c_ts_resume_driver,
#endif
};

int __init i2c_ts_driver_init(void)
{
	int rc;

	p_tango_i2c_dev = kzalloc(sizeof(struct tango_i2c), GFP_KERNEL);

	printk(KERN_INFO "%s: i2c_ts_driver_init() entering ...\n",
		I2C_TS_DRIVER_NAME);

	if (p_tango_i2c_dev == NULL) {
		printk(KERN_ERR "i2c_ts_driver_init(): memory allocation "
		       "failed for p_tango_i2c_dev!\n");
		return -ENOMEM;
	}

	rc = i2c_add_driver(&tango_i2c_driver);

	/* Probe fails, delet driver */
	if (g_tango_probe_flag)
		i2c_del_driver(&tango_i2c_driver);

	if (rc != 0) {
		printk("%s i2c_ts_driver_init(): i2c_add_driver() failed, "
		       "errno is %d\n", I2C_TS_DRIVER_NAME, rc);
		return rc;
	}
	return rc;
}

static void __exit i2c_ts_driver_exit(void)
{
	i2c_del_driver(&tango_i2c_driver);
}

MODULE_DESCRIPTION("Tango I2C Touchscreen driver");
MODULE_AUTHOR("Broadcom");
MODULE_LICENSE("GPL");

module_init(i2c_ts_driver_init);
module_exit(i2c_ts_driver_exit);
