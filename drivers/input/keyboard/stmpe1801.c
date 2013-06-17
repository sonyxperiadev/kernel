/******************** (C) COPYRIGHT 2010 STMicroelectronics ********************
*
* File Name		: stmpe1801.c
* Authors		: Sensor & MicroActuators BU - Application Team
*			    : Sivakumar SD (sivakumar.sd@st.com)
* Version		: V 1.0 
* Date			: 04/03/2011
* Description	: STMPE1801
*
********************************************************************************
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* THE PRESENT SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES
* OR CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, FOR THE SOLE
* PURPOSE TO SUPPORT YOUR APPLICATION DEVELOPMENT.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*
* THIS SOFTWARE IS SPECIFICALLY DESIGNED FOR EXCLUSIVE USE WITH ST PARTS.
*
********************************************************************************
* REVISON HISTORY
*
* VERSION | DATE 	| AUTHORS	     | DESCRIPTION
*
* 1.0	  | 04/03/2011 | Sivakumar SD    | First Release
*
*******************************************************************************/
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/hrtimer.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/serio.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/ctype.h>
#include <linux/gpio.h>
#include <linux/input/matrix_keypad.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <linux/input/stmpe1801.h>
#include <linux/irq.h>

/*
 * Definitions & global arrays.
 */
#define DRIVER_DESC	"stmpe1801 i2c keyboard driver"
#define stmpe1801_TS_DRV_NAME	"stmpe1801"

#define STMPE1801_FIFO_SIZE				3 //10

#define STMPE_KEYPAD_MAX_DEBOUNCE       127
#define STMPE_KEYPAD_MAX_SCAN_COUNT     15

#define STMPE_KEYPAD_MAX_ROWS           7
#define STMPE_KEYPAD_MAX_COLS           6
#define STMPE_KEYPAD_ROW_SHIFT          4
#define STMPE_KEYPAD_KEYMAP_SIZE        \
        (STMPE_KEYPAD_MAX_ROWS * STMPE_KEYPAD_MAX_COLS)
        
#define STMPE_KPC_DATA_UP               (0x1 << 7)
#define STMPE_KPC_DATA_COL              (0xf << 3)
#define STMPE_KPC_DATA_ROW              (0x7 << 0)
#define STMPE_KPC_DATA_NOKEY_MASK       0x78


#define KPEXP_INT 91
#define KPEXP_SDA 34
#define KPEXP_SCL 33

#define USE_THREADED_IRQ	1

static struct i2c_driver stm_kp_driver;

#if USE_THREADED_IRQ

#else
static struct workqueue_struct *stmpe1801_wq;
#endif

/**
 * struct stmpe_keypad_variant - model-specific attributes
 * @auto_increment: whether the KPC_DATA_BYTE register address
 *                  auto-increments on multiple read
 * @num_data: number of data bytes
 * @num_normal_data: number of normal keys' data bytes
 * @max_cols: maximum number of columns supported
 * @max_rows: maximum number of rows supported
 * @col_gpios: bitmask of gpios which can be used for columns
 * @row_gpios: bitmask of gpios which can be used for rows
 */
struct stmpe_keypad_variant {
        bool            auto_increment;
        int             num_data;
        int             num_normal_data;
        int             max_cols;
        int             max_rows;
        unsigned int    col_gpios;
        unsigned int    row_gpios;
};
					
struct stmpe1801_kp {
	struct device *dev;
	struct i2c_client *client;
	struct input_dev *input_dev;

	struct stmpe_platform_data *plat;
	
	const struct stmpe_keypad_variant *variant;
	//const struct stmpe_keypad_platform_data *plat;
	
	unsigned short keymap[STMPE_KEYPAD_KEYMAP_SIZE];
	
	struct hrtimer timer;
	struct work_struct  work;
	spinlock_t		lock;
	int irq;

};

static int stmpe1801_read_reg(struct i2c_client *client, unsigned char reg[], int cnum, u8 *buf, int num)
{
        struct i2c_msg xfer_msg[2];

        xfer_msg[0].addr = client->addr;
        xfer_msg[0].len = cnum;
        xfer_msg[0].flags = 0;
        xfer_msg[0].buf = reg;

        xfer_msg[1].addr = client->addr;
        xfer_msg[1].len = num;
        xfer_msg[1].flags = I2C_M_RD;
        xfer_msg[1].buf = buf;

        return i2c_transfer(client->adapter, xfer_msg, 2);
}

static int stmpe1801_write_reg(struct stmpe1801_kp *stmpe1801_kp, unsigned char reg[], u8 num_com)
{
	int rc;
		
	rc = i2c_master_send(stmpe1801_kp->client, reg, num_com);
	if(rc<0) {
		printk("[KEYEP]stmpe1801_write_reg: i2c_master_send failed\n");
		return rc;
	}
	return 0;
}

static int init_stmpe1801(struct stmpe1801_kp *stmpe1801_kp)
{
	u8 val[8];	
	u8 regAdd[7];
	int rc;	
						
	regAdd[0]=0x00;
	rc=stmpe1801_read_reg(stmpe1801_kp->client, regAdd, 1, val, 3);
	mdelay(50);		
	printk("[KEYEP]Chip ID = 0x%x 0x%x 0x%x\n" , val[0], val[1], val[2]);		
	
	//if (stmpe1801_kp->plat->debounce_ms > STMPE_KEYPAD_MAX_DEBOUNCE)
	//	return -EINVAL;
		
	//if (stmpe1801_kp->plat->scan_count > STMPE_KEYPAD_MAX_SCAN_COUNT)
	//	return -EINVAL;

	regAdd[0]=0x02; //SYS_CTRL
	regAdd[1]=0x86;
	stmpe1801_write_reg(stmpe1801_kp, &regAdd[0],2);
	mdelay(20);	
	
	regAdd[0]=0x30;
	rc=stmpe1801_read_reg(stmpe1801_kp->client, regAdd, 1, val, 6);
	mdelay(50);		
	printk("[KEYEP]Chip ID = 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n" , val[0], val[1], val[2], val[3], val[4], val[5]);	
	
	regAdd[0]=0x30; 
	regAdd[1]=0x7F;	// KPC_ROW
	regAdd[2]=0x3F; // KPC_COL_LOW
	regAdd[3]=0x03; // KPC_COL_HIGH
	regAdd[4]=0x50; // KPC_CTRL_LOW
	regAdd[5]=0x3E; // KPC_CTRL_MID
	regAdd[6]=0x43; // KPC_CTRL_HIGH
	stmpe1801_write_reg(stmpe1801_kp, &regAdd[0],7);
	mdelay(20);	
	
	regAdd[0]=0x06; // INT_ENABLE_MASK_LOW
	regAdd[1]=0x16;
	stmpe1801_write_reg(stmpe1801_kp, &regAdd[0],2);
	mdelay(20);	

	regAdd[0]=0x04; // INT_CTRL_LOW
	regAdd[1]=0x03; //0x01  level --> 0x03 edge low
	stmpe1801_write_reg(stmpe1801_kp, &regAdd[0],2);
	mdelay(20);	

	regAdd[0]=0x36; // KPC_CMD
	regAdd[1]=0x01;
	stmpe1801_write_reg(stmpe1801_kp, &regAdd[0],2);
	mdelay(20);	

	regAdd[0]=0x19; // GPIO_SET_DIR_LOW
	regAdd[1]=0x0E;
	stmpe1801_write_reg(stmpe1801_kp, &regAdd[0],2);
	mdelay(20);	

	regAdd[0]=0x1C; // GPIO_RE_LOW
	regAdd[1]=0x01;
	stmpe1801_write_reg(stmpe1801_kp, &regAdd[0],2);
	mdelay(20);	

	regAdd[0]=0x1F; // GPIO_FE_LOW
	regAdd[1]=0x01;
	stmpe1801_write_reg(stmpe1801_kp, &regAdd[0],2);
	mdelay(20);	

	regAdd[0]=0x22; // GPIO_PULL_UP_LOW
	regAdd[1]=0x01;
	stmpe1801_write_reg(stmpe1801_kp, &regAdd[0],2);
	mdelay(20);	

	regAdd[0]=0x37; // KPC_COMB_KEY_0
	regAdd[1]=0x07;
	stmpe1801_write_reg(stmpe1801_kp, &regAdd[0],2);
	mdelay(20);	

	regAdd[0]=0x37; // KPC_COMB_KEY_1
	regAdd[1]=0x3F;
	stmpe1801_write_reg(stmpe1801_kp, &regAdd[0],2);
	mdelay(20);	

	regAdd[0]=0x37; // KPC_COMB_KEY_2
	regAdd[1]=0x47;
	stmpe1801_write_reg(stmpe1801_kp, &regAdd[0],2);
	mdelay(20);	
	
	regAdd[0]=0x30;
	rc=stmpe1801_read_reg(stmpe1801_kp->client, regAdd, 1, val, 6);
	mdelay(50);		
	printk("[KEYEP]Chip ID = 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n" , val[0], val[1], val[2], val[3], val[4], val[5]);	

	return 0;
}

static enum hrtimer_restart st_kp_timer_func(struct hrtimer *timer)
{

	#if USE_THREADED_IRQ
	
	#else
	struct stmpe1801_kp *stmpe1801ts  = container_of(timer, struct stmpe1801_kp, timer);

	queue_work(stmpe1801_wq, &stmpe1801ts->work);
	#endif

	return HRTIMER_NORESTART;
}

static irqreturn_t kp_interrupt(int irq, void *handle)
{
#ifndef USE_THREADED_IRQ
	struct stmpe1801_kp *stmpe1801_kp = handle;	
#endif
	//pr_debug( "[KEYEP]stmpe1801 interrupt handling, irq:%d  \n", irq);

#if USE_THREADED_IRQ

#else
	disable_irq_nosync(stmpe1801_kp->client->irq);
#endif

#if USE_THREADED_IRQ
	return IRQ_WAKE_THREAD;
#else
	queue_work(stmpe1801_wq, &stmpe1801_kp->work);
	return IRQ_HANDLED;
#endif
}

#if USE_THREADED_IRQ
static irqreturn_t kp_tasklet_proc(int irq, void *dev_id)
#else
static void kp_tasklet_proc(struct work_struct *work)
#endif
{
	u8 i = 0;
	u8 j = 0;
	u8 val[8];	
	u8 regAdd[7];
	int rc;	


#if USE_THREADED_IRQ
	struct stmpe1801_kp *stmpe1801_kp = dev_id;
#else	
	struct stmpe1801_kp *stmpe1801_kp = container_of(work, struct stmpe1801_kp, work);
#endif	
	regAdd[0]=0x08;
	rc=stmpe1801_read_reg(stmpe1801_kp->client, regAdd, 1, val, 2);
	regAdd[0]=0x3A;
	for (i = 0; i < STMPE1801_FIFO_SIZE; i++) { // Reading the fifo one by one and processing it.
		rc=stmpe1801_read_reg(stmpe1801_kp->client, regAdd, 1, val, 5);
		mdelay(3);
		if( (val[0]  != 0xF8) || (val[1]  != 0xF8) || (val[2]  != 0xF8) || (val[3]  != 0xFF) || (val[4]  != 0x0F) ) {
			for(j = 0; j < 3; j++) {
				int data = val[j];
				int col = (data & STMPE_KPC_DATA_COL) >> 3;
				int row = data & STMPE_KPC_DATA_ROW;
				int code = MATRIX_SCAN_CODE(row, col, STMPE_KEYPAD_ROW_SHIFT);
				bool up = val[j] & STMPE_KPC_DATA_UP;
				
                if ((data & STMPE_KPC_DATA_NOKEY_MASK) == STMPE_KPC_DATA_NOKEY_MASK)
                {
                    continue;
                }
                else
                {
		  //	pr_debug("[KEYEP] Data = %d 0x%x 0x%x 0x%x 0x%x 0x%x\n" , code, val[0], val[1], val[2], val[3], val[4]);
                }

                input_event(stmpe1801_kp->input_dev, EV_MSC, MSC_SCAN, code);
                input_report_key(stmpe1801_kp->input_dev, stmpe1801_kp->keymap[code], !up);
		//pr_debug("[KEYEP] Keycode = %d\n", stmpe1801_kp->keymap[code]);
                input_sync(stmpe1801_kp->input_dev);				
			}
			//	pr_debug("KeyData = %x %x %x %x %x\n" , val[0], val[1], val[2], val[3], val[4]);
		}
	}
	
	
	if (!stmpe1801_kp->irq) {
		printk( "[KEYEP]stmpe1801 start timeer\n");	
		hrtimer_start(&stmpe1801_kp->timer, ktime_set(0, 100000000), HRTIMER_MODE_REL);			
	}
	else
	{

		#if USE_THREADED_IRQ
		//printk( "[KEYEP]stmpe1801 threaded irq\n");
		#else
		printk( "[KEYEP]stmpe1801 enable irq\n");
		enable_irq(stmpe1801_kp->client->irq);	
		#endif		
	}

#if USE_THREADED_IRQ
	return IRQ_HANDLED;
#else
#endif

}


static int stm_kp_probe(struct i2c_client *client, const struct i2c_device_id *idp)
{
	struct stmpe1801_kp *stmpe1801_kp = NULL;
	int err = -ENOMEM;

	pr_info( "[KEYEP] stmpe1801kp_probe start\n");
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
		goto fail;
	
	stmpe1801_kp = kzalloc(sizeof(struct stmpe1801_kp), GFP_KERNEL);
	if (!stmpe1801_kp)
		goto fail;

#if USE_THREADED_IRQ

#else	
	INIT_WORK(&stmpe1801_kp->work, kp_tasklet_proc);	
#endif

	stmpe1801_kp->client=client;
	stmpe1801_kp->plat=client->dev.platform_data;
	i2c_set_clientdata(client,stmpe1801_kp);
			
	stmpe1801_kp->dev = &stmpe1801_kp->client->dev;
	stmpe1801_kp->input_dev = input_allocate_device();
	stmpe1801_kp->input_dev->dev.parent = &client->dev;
	if(!stmpe1801_kp->input_dev)
		goto fail;
		
	stmpe1801_kp->input_dev->name = "stmpe1801";
	stmpe1801_kp->input_dev->phys = "stmpe1801/input0";
	stmpe1801_kp->input_dev->id.bustype = BUS_I2C;	
	stmpe1801_kp->input_dev->id.vendor = 0x0001;
	stmpe1801_kp->input_dev->id.product = 0x0002;
	stmpe1801_kp->input_dev->id.version = 0x0100;


	input_set_capability(stmpe1801_kp->input_dev, EV_MSC, MSC_SCAN);

    set_bit(EV_KEY, stmpe1801_kp->input_dev->evbit);
    //if (!stmpe1801_kp->plat->no_autorepeat)
	//	set_bit(EV_REP, stmpe1801_kp->input_dev->evbit);

    stmpe1801_kp->input_dev->keycode = stmpe1801_kp->keymap;
    stmpe1801_kp->input_dev->keycodesize = sizeof(stmpe1801_kp->keymap[0]);
    stmpe1801_kp->input_dev->keycodemax = ARRAY_SIZE(stmpe1801_kp->keymap);


	matrix_keypad_build_keymap(stmpe1801_kp->plat->keypad->keymap_data, STMPE_KEYPAD_ROW_SHIFT,
				   stmpe1801_kp->input_dev->keycode, stmpe1801_kp->input_dev->keybit);


	err=input_register_device(stmpe1801_kp->input_dev);	
	init_stmpe1801(stmpe1801_kp);		
	stmpe1801_kp->irq=client->irq;	
		
	#if USE_THREADED_IRQ
	if(request_threaded_irq(stmpe1801_kp->irq, kp_interrupt, kp_tasklet_proc ,IRQF_TRIGGER_FALLING | IRQF_ONESHOT, client->name, stmpe1801_kp))
	#else
	if(request_irq(stmpe1801_kp->irq, kp_interrupt, /*IRQF_TRIGGER_LOW*/ IRQF_TRIGGER_FALLING, client->name, stmpe1801_kp))
	#endif	
	{
		err = -EBUSY;
		goto fail;
	}
	
	if (!stmpe1801_kp->irq) {
		hrtimer_init(&stmpe1801_kp->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
		stmpe1801_kp->timer.function = st_kp_timer_func;
		hrtimer_start(&stmpe1801_kp->timer, ktime_set(1, 0), HRTIMER_MODE_REL);
	}
	pr_info( "[KEYEP] stmpe1801kp_probe Done\n");
	
	return 0;
fail:
	if(stmpe1801_kp) {
		if(stmpe1801_kp->input_dev)
			input_free_device(stmpe1801_kp->input_dev);
		kfree(stmpe1801_kp);
	}
	pr_err("[KEYEP] stmpe1801kp_probe fail ret=%d\n", err);
	return err;	
}

static int stm_kp_remove(struct i2c_client *client)
{
	struct stmpe1801_kp *priv = dev_get_drvdata(&client->dev);
	printk("[KEYEP]Hello from Remove\n"); 
	free_irq(priv->irq, priv);
	input_unregister_device(priv->input_dev);
	kfree(priv);		
	dev_set_drvdata(&client->dev, NULL);

	return 0;
}

static const struct i2c_device_id stm_kp_id[] = {
	{ "stmpe1801", 0 },
	{ }
};

static struct i2c_driver stm_kp_driver = {
	.driver = {
		.name = "stmpe1801",
	},
	.probe = stm_kp_probe,
	.remove = stm_kp_remove,
	.id_table = stm_kp_id,
};

static int __init stm_kp_init(void)
{
	pr_info("[KEYEP]Hello from init\n");

	gpio_request(KPEXP_INT, "stm_irq");
	gpio_direction_input(KPEXP_INT);

	irq_set_irq_type(gpio_to_irq(KPEXP_INT), /*IRQ_TYPE_LEVEL_LOW*/ IRQ_TYPE_EDGE_FALLING);


	gpio_direction_output( KPEXP_SCL , 1 );
	gpio_direction_output( KPEXP_SDA , 1 );	

#if USE_THREADED_IRQ

#else		
	stmpe1801_wq = create_singlethread_workqueue("stmpe1801_wq");
	if (!stmpe1801_wq)
		return -ENOMEM;
#endif

	return i2c_add_driver(&stm_kp_driver);
}

static void __exit stm_kp_exit(void)
{
	i2c_del_driver(&stm_kp_driver);
	#if USE_THREADED_IRQ

	#else	
	if (stmpe1801_wq)
		destroy_workqueue(stmpe1801_wq);
	#endif
	pr_info("[KEYEP]Hello from exit\n"); 
}

MODULE_DESCRIPTION("STM Keypad IC Driver");
MODULE_AUTHOR("Sivakumar SD <shivizard@gmail.com>");
MODULE_LICENSE("GPL");

module_init(stm_kp_init);
module_exit(stm_kp_exit);
