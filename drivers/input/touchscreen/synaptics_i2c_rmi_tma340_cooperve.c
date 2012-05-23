/* drivers/input/keyboard/synaptics_i2c_rmi.c
 *
 * Copyright (C) 2007 Google, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/slab.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/earlysuspend.h>
#include <linux/hrtimer.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <mach/gpio.h>
#include <linux/device.h>
#include <linux/synaptics_i2c_rmi.h>
#include <linux/regulator/consumer.h>

#include <linux/irq.h>

#include <linux/firmware.h>
#include <linux/uaccess.h> 
/* firmware - update */

#define MAX_X	320 
#define MAX_Y	480
#define TSP_INT 121
#define TSP_SDA 86
#define TSP_SCL 87
#define TOUCH_EN 22

#define MAX_KEYS	2
#define MAX_USING_FINGER_NUM 2

static const int touchkey_keycodes[] = {
			KEY_MENU,
			KEY_BACK,
			//KEY_HOME,
			//KEY_SEARCH,
};

#define TOUCH_ON 1
#define TOUCH_OFF 0

#define TRUE    1
#define FALSE    0

#define I2C_RETRY_CNT	2

//#define __TOUCH_DEBUG__ 1
#define USE_THREADED_IRQ	1

#define __TOUCH_KEYLED__ 

static struct regulator *touch_regulator=NULL;
#if defined (__TOUCH_KEYLED__)
static struct regulator *touchkeyled_regulator=NULL;
#endif

#if USE_THREADED_IRQ

#else
static struct workqueue_struct *synaptics_wq;
#endif

static int touchkey_status[MAX_KEYS];

#define TK_STATUS_PRESS		1
#define TK_STATUS_RELEASE		0

int tsp_irq;
int st_old;

typedef struct
{
	int8_t id;	/*!< (id>>8) + size */
	int8_t status;/////////////IC
	int8_t z;	/*!< dn>0, up=0, none=-1 */
	int16_t x;			/*!< X */
	int16_t y;			/*!< Y */
} report_finger_info_t;

static report_finger_info_t fingerInfo[MAX_USING_FINGER_NUM]={0,};

struct synaptics_ts_data {
	uint16_t addr;
	struct i2c_client *client;
	struct input_dev *input_dev;
	int use_irq;
	struct hrtimer timer;				////////////////////////IC
	struct work_struct  work;
	//struct work_struct  work_timer;		////////////////////////IC
	struct early_suspend early_suspend;
};

#if defined (CONFIG_TOUCHSCREEN_MMS128_TASSCOOPER)
static int8_t Synaptics_Connected = 0;
#endif

struct synaptics_ts_data *ts_global;

/* firmware - update */
static int firmware_ret_val = -1;
static int HW_ver = -1;
unsigned char now_tst200_update_luisa = 0;
unsigned char tsp_special_update = 0;

/* touch information*/
unsigned char touch_vendor_id;
unsigned char touch_hw_ver;
unsigned char touch_sw_ver;

// need to verify
//#define TSP_VENDER_ID	0x0A
//#define TSP_HW_VER1		0x11
//#define TSP_SW_VER1		0x08

int tsp_irq_num = 0;
int tsp_workqueue_num = 0;
int tsp_threadedirq_num = 0;

int g_touch_info_x = 0;
int g_touch_info_y = 0;
int g_touch_info_press = 0;


int firm_update( void );
extern int cypress_update( int );
int tsp_i2c_read(u8 reg, unsigned char *rbuf, int buf_size);


/* sys fs */
struct class *touch_class;
EXPORT_SYMBOL(touch_class);
struct device *firmware_dev;
EXPORT_SYMBOL(firmware_dev);

static ssize_t firmware_show(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t firmware_store( struct device *dev, struct device_attribute *attr, const char *buf, size_t size);
static ssize_t firmware_ret_show(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t firmware_ret_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size);
static DEVICE_ATTR(firmware	, 0444, firmware_show, firmware_store);
static DEVICE_ATTR(firmware_ret	, 0444, firmware_ret_show, firmware_ret_store);
/* sys fs */

//static int tsp_testmode = 0;

#ifdef CONFIG_HAS_EARLYSUSPEND
static void synaptics_ts_early_suspend(struct early_suspend *h);
static void synaptics_ts_late_resume(struct early_suspend *h);
#endif

extern int bcm_gpio_pull_up(unsigned int gpio, bool up);
extern int bcm_gpio_pull_up_down_enable(unsigned int gpio, bool enable);
extern int irq_set_type(unsigned int irq, unsigned int type);

#if defined (CONFIG_TOUCHSCREEN_MMS128_TASSCOOPER)
extern int Is_MMS128_Connected(void);

int Is_Synaptics_Connected(void)
{
    return (int) Synaptics_Connected;
}
#endif

void touch_ctrl_regulator(int on_off)
{
	#if 0
	if (on_off == TOUCH_ON)
	{
		regulator_set_voltage(touch_regulator, 2900000, 2900000);
		regulator_enable(touch_regulator);
	}
	else
	{
		regulator_disable(touch_regulator);
	}
	#else
	if (on_off == TOUCH_ON)
	{
		gpio_request(TOUCH_EN,"Touch_en");
		gpio_direction_output(TOUCH_EN,1);
		gpio_set_value(TOUCH_EN,1);
		gpio_free(TOUCH_EN);
	}
	else
	{
		#if 1
		gpio_request(TOUCH_EN,"Touch_en");
		gpio_direction_output(TOUCH_EN,0);
		gpio_set_value(TOUCH_EN,0);
		gpio_free(TOUCH_EN);
		#endif
	}
	#endif
}
EXPORT_SYMBOL(touch_ctrl_regulator);

int tsp_reset( void )
{
	int ret=1;

      #if defined(__TOUCH_DEBUG__)
	printk("[TSP] %s, %d\n", __func__, __LINE__ );
      #endif 
      
	if (ts_global->use_irq)
	{
		disable_irq(ts_global->client->irq);
	}

	touch_ctrl_regulator(0);

	gpio_direction_output( TSP_SCL , 0 ); 
	gpio_direction_output( TSP_SDA , 0 ); 
	//gpio_direction_output( TSP_INT , 0 ); 

	msleep(200);

	gpio_direction_output( TSP_SCL , 1 ); 
	gpio_direction_output( TSP_SDA , 1 ); 
	//gpio_direction_output( TSP_INT , 1 ); 

	touch_ctrl_regulator(1);

	msleep(10);

	enable_irq(ts_global->client->irq);

	return ret;
}


static void process_key_event(uint8_t tsk_msg)
{
	int i;
	int keycode= 0;
	int st_new;

#if defined(__TOUCH_DEBUG__)
        printk("[TSP] process_key_event : %d\n", tsk_msg);
#endif

	if(	tsk_msg	== 0)
	{
		input_report_key(ts_global->input_dev, st_old, 0);
#if defined(__TOUCH_DEBUG__)
		printk("[TSP] release keycode: %4d, keypress: %4d\n", st_old, 0);
#endif
	}
	else{
	//check each key status
		for(i = 0; i < MAX_KEYS; i++)
		{

		st_new = (tsk_msg>>(i)) & 0x1;
		if (st_new ==1)
		{
		keycode = touchkey_keycodes[i];
		input_report_key(ts_global->input_dev, keycode, 1);
#if defined(__TOUCH_DEBUG__)
		printk("[TSP] press keycode: %4d, keypress: %4d\n", keycode, 1);
#endif
		}

		st_old = keycode;


		}
	}
}

#if USE_THREADED_IRQ
static irqreturn_t synaptics_ts_work_func(int irq, void *dev_id)
#else
static void synaptics_ts_work_func(struct work_struct *work)
#endif
{
	int ret=0;
	uint8_t buf[12];// 02h ~ 0Dh
	uint8_t buf_key[1];
	uint8_t i2c_addr = 0x02;
	int i = 0;
	int finger = 0;

#if USE_THREADED_IRQ
	struct synaptics_ts_data *ts = dev_id;
#else
	struct synaptics_ts_data *ts = container_of(work, struct synaptics_ts_data, work);
#endif
	
	ret = tsp_i2c_read( i2c_addr, buf, sizeof(buf));

	if (ret <= 0) {
		printk("[TSP] i2c failed : ret=%d, ln=%d\n",ret, __LINE__);
		goto work_func_out;
	}

	finger = buf[0] & 0x0F;	//number of touch finger
	buf_key[0] = (buf[0] & 0xC0) >> 6; //information of touch key
	
	fingerInfo[0].x = (buf[1] << 8) |buf[2];
	fingerInfo[0].y = (buf[3] << 8) |buf[4];
	fingerInfo[0].z = buf[5];
	fingerInfo[0].id = buf[6] >>4;

	fingerInfo[1].x = (buf[7] << 8) |buf[8];
	fingerInfo[1].y = (buf[9] << 8) |buf[10];
	fingerInfo[1].z = buf[11];
	fingerInfo[1].id = buf[6] & 0xf;

/*********************hash
	if ( board_hw_revision >= 0x2 && HW_ver==1 )
	{

		fingerInfo[0].x = 240 - fingerInfo[0].x;
		fingerInfo[0].y = 320 - fingerInfo[0].y;
		fingerInfo[1].x = 240 - fingerInfo[1].x;
		fingerInfo[1].y = 320 - fingerInfo[1].y;
	
		//	fingerInfo[0].x = 320 - fingerInfo[0].x;
		//	fingerInfo[1].y = 480 - fingerInfo[1].y;
	}
************************/
	//	print message
//	for ( i= 0; i<MAX_USING_FINGER_NUM; i++ )
//		printk("[TSP] finger[%d].x = %d, finger[%d].y = %d, finger[%d].z = %x, finger[%d].id = %x\n", i, fingerInfo[i].x, i, fingerInfo[i].y, i, fingerInfo[i].z, i, fingerInfo[i].id);

	/* check key event*/
//	if(fingerInfo[0].status != 1 && fingerInfo[1].status != 1)	//
//		process_key_event(buf[0]);								//HASHTSK
	if(finger == 0)
	{
		process_key_event(buf_key[0]);
	}

	/* check touch event */
	for ( i= 0; i<MAX_USING_FINGER_NUM; i++ )
	{
		//////////////////////////////////////////////////IC
		if(fingerInfo[i].id >=1) // press interrupt
		{
			if(fingerInfo[i].status != -2) // force release
				fingerInfo[i].status = 1;
			else
				fingerInfo[i].status = -2;
		}
		else if(fingerInfo[i].id ==0) // release interrupt (only first finger)
		{
			if(fingerInfo[i].status == 1) // prev status is press
				fingerInfo[i].status = 0;
			else if(fingerInfo[i].status == 0 || fingerInfo[i].status == -2) // release already or force release
				fingerInfo[i].status = -1;				
		}

		if(fingerInfo[i].status < 0) continue;
		//////////////////////////////////////////////////IC
		
		input_report_abs(ts->input_dev, ABS_MT_POSITION_X, fingerInfo[i].x);
		input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, fingerInfo[i].y);
		input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, fingerInfo[i].status);
		input_report_abs(ts->input_dev, ABS_MT_WIDTH_MAJOR, fingerInfo[i].z);
		input_mt_sync(ts->input_dev);

     #if defined(__TOUCH_DEBUG__)
		printk("[TSP] i[%d] id[%d] xyz[%d, %d, %x] status[%x]\n", i, fingerInfo[i].id, fingerInfo[i].x, fingerInfo[i].y, fingerInfo[i].z, fingerInfo[i].status);	
    #endif
	}

	input_report_key(ts->input_dev, BTN_TOUCH, finger > 0);

	input_sync(ts->input_dev);

work_func_out:
	if (ts->use_irq)
	{
		#if USE_THREADED_IRQ

		#else
		enable_irq(ts->client->irq);
		#endif
	}
	
	#if USE_THREADED_IRQ
	return IRQ_HANDLED;
	#else

	#endif	
}


int tsp_i2c_read(u8 reg, unsigned char *rbuf, int buf_size)
{
	int i, ret=-1;
	struct i2c_msg rmsg;
	uint8_t start_reg;

	for (i = 0; i < I2C_RETRY_CNT; i++)
	{
		rmsg.addr = ts_global->client->addr;
		rmsg.flags = 0;//I2C_M_WR;
		rmsg.len = 1;
		rmsg.buf = &start_reg;
		start_reg = reg;
		
		ret = i2c_transfer(ts_global->client->adapter, &rmsg, 1);

		if(ret >= 0) 
		{
			rmsg.flags = I2C_M_RD;
			rmsg.len = buf_size;
			rmsg.buf = rbuf;
			ret = i2c_transfer(ts_global->client->adapter, &rmsg, 1 );

			if (ret >= 0)
				break; // i2c success
		}

		if( i == (I2C_RETRY_CNT - 1) )
		{
			printk("[TSP] Error code : %d, %d\n", __LINE__, ret );
		}
	}

	return ret;
}



static enum hrtimer_restart synaptics_ts_timer_func(struct hrtimer *timer)
{
	#if USE_THREADED_IRQ
	
	#else
	queue_work(synaptics_wq, &ts_global->work);
	#endif

	hrtimer_start(&ts_global->timer, ktime_set(0, 12500000), HRTIMER_MODE_REL);
	return HRTIMER_NORESTART;
}

static irqreturn_t synaptics_ts_irq_handler(int irq, void *dev_id)
{
	struct synaptics_ts_data *ts = dev_id;
	//printk("[TSP] %s, %d\n", __func__, __LINE__ );
	#if USE_THREADED_IRQ

	#else
	disable_irq_nosync(ts->client->irq);
	#endif


	#if USE_THREADED_IRQ
	return IRQ_WAKE_THREAD;
	#else
	queue_work(synaptics_wq, &ts->work);
	return IRQ_HANDLED;
	#endif
}

int synaptics_ts_check(void)
{
	int ret, i;
	uint8_t buf_tmp[3]={0,0,0};
	int retry = 3;


	ret = tsp_i2c_read( 0x1B, buf_tmp, sizeof(buf_tmp));

	// i2c read retry
	if(ret <= 0)
	{
		for(i=0; i<retry;i++)
		{
			ret=tsp_i2c_read( 0x1B, buf_tmp, sizeof(buf_tmp));

			if(ret > 0)
				break;
		}
	}

	if (ret <= 0) 
	{
		printk("[TSP][Synaptics][%s] %s\n", __func__,"Failed synpatics i2c");
#if defined (CONFIG_TOUCHSCREEN_MMS128_TASSCOOPER)	
		Synaptics_Connected = 0;
#endif
		ret = 0;
	}
	else 
	{
		printk("[TSP][Synaptics][%s] %s\n", __func__,"Passed synpatics i2c");
#if defined (CONFIG_TOUCHSCREEN_MMS128_TASSCOOPER)	
		Synaptics_Connected = 1;
#endif

		printk("[TSP][Synaptics][%s][SlaveAddress : 0x%x][VendorID : 0x%x] [HW : 0x%x] [SW : 0x%x]\n", __func__,ts_global->client->addr, buf_tmp[0], buf_tmp[1], buf_tmp[2]);

		if ( buf_tmp[0] == 0xf0 )//(ts->hw_rev == 0) && (ts->fw_ver == 2))
		{
			ret = 1;
			printk("[TSP][Synaptics][%s] %s\n", __func__,"Passed synaptics_ts_check");
		}
		else
		{
			ret = 0;
			printk("[TSP][Synaptics][%s] %s\n", __func__,"Failed synaptics_ts_check");
		}
		
	}

	return ret;
}

static int synaptics_ts_probe(
	struct i2c_client *client, const struct i2c_device_id *id)
{
	struct synaptics_ts_data *ts;

	uint8_t i2c_addr = 0x1B;
  	uint8_t buf[3], buf_tmp[3]={0,0,0};
	uint8_t addr[1];	
	int i;
    	int ret = 0, key = 0;


#if defined (CONFIG_TOUCHSCREEN_MMS128_TASSCOOPER)
    printk("[TSP][Synaptics][%s] %s\n", __func__,"Called");

    if(Is_MMS128_Connected()== 1)
    {
        printk("[TSP][Synaptics][%s] %s\n", __func__,"Melfas already detected !!");

        return -ENXIO;
    }
#endif

	printk("[TSP] %s, %d\n", __func__, __LINE__ );

	touch_ctrl_regulator(TOUCH_ON);
	msleep(100);	
	touch_ctrl_regulator(TOUCH_OFF);
	msleep(200);
	touch_ctrl_regulator(TOUCH_ON);
	msleep(100);

	ts = kzalloc(sizeof(*ts), GFP_KERNEL);
	if (ts == NULL) {
		ret = -ENOMEM;
		goto err_alloc_data_failed;
	}
	
	#if USE_THREADED_IRQ

	#else
	INIT_WORK(&ts->work, synaptics_ts_work_func);
	#endif

	ts->client = client;
	i2c_set_clientdata(client, ts);

	ts_global = ts;

	tsp_irq=client->irq;

#if defined (CONFIG_TOUCHSCREEN_MMS128_TASSCOOPER)
	#if USE_THREADED_IRQ
	msleep(100);
	#endif

    ret = synaptics_ts_check();
    if (ret <= 0) {
         i2c_release_client(client);		
         touch_ctrl_regulator(TOUCH_OFF);

         ret = -ENXIO;
         goto err_input_dev_alloc_failed;
     }
#else
	/* Check point - i2c check - start */	
    //ret = tsp_i2c_read( 0x1B, buf_tmp, sizeof(buf_tmp));
    for (i = 0; i < 1; i++)
	{
		printk("[TSP] %s, %d, send\n", __func__, __LINE__ );
		addr[0] = 0x1B; //address
		ret = i2c_master_send(ts_global->client, addr, 1);

		if (ret >= 0)
		{
			printk("[TSP] %s, %d, receive\n", __func__, __LINE__ );
			ret = i2c_master_recv(ts_global->client, buf_tmp, 3);
			if (ret >= 0)
				break; // i2c success
		}

		printk("[TSP] %s, %d, fail\n", __func__, __LINE__ );
	}

	touch_vendor_id = buf_tmp[0];
	touch_hw_ver = buf_tmp[1];
	touch_sw_ver = buf_tmp[2];
	printk("[TSP] %s:%d, ver tsp=%x, HW=%x, SW=%x\n", __func__,__LINE__, touch_vendor_id, touch_hw_ver, touch_sw_ver);
#endif

	HW_ver = touch_hw_ver;

	if (ret <= 0) {
		printk("[TSP] %s, ln:%d, Failed to register TSP!!!\n\tcheck the i2c line!!!, ret=%d\n", __func__,__LINE__, ret);
		goto err_check_functionality_failed;
	}
	/* Check point - i2c check - end */


	ts->input_dev = input_allocate_device();
	if (ts->input_dev == NULL) {
		ret = -ENOMEM;
		printk(KERN_ERR "synaptics_ts_probe: Failed to allocate input device\n");
		goto err_input_dev_alloc_failed;
	}

	ts->input_dev->name = "synaptics-rmi-touchscreen";
	set_bit(EV_SYN, ts->input_dev->evbit);
	set_bit(EV_KEY, ts->input_dev->evbit);
	set_bit(BTN_TOUCH, ts->input_dev->keybit);
	set_bit(EV_ABS, ts->input_dev->evbit);
	set_bit(INPUT_PROP_DIRECT, &ts->input_dev->propbit);

	input_set_abs_params(ts->input_dev, ABS_MT_POSITION_X, 0, MAX_X, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_POSITION_Y, 0, MAX_Y, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0, 255, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_WIDTH_MAJOR, 0, 255, 0, 0);


	for(key = 0; key < MAX_KEYS ; key++)
		input_set_capability(ts->input_dev, EV_KEY, touchkey_keycodes[key]);

	// for TSK
	for(key = 0; key < MAX_KEYS ; key++)
		touchkey_status[key] = TK_STATUS_RELEASE;

    
	/* ts->input_dev->name = ts->keypad_info->name; */
	ret = input_register_device(ts->input_dev);
	if (ret) {
		printk(KERN_ERR "synaptics_ts_probe: Unable to register %s input device\n", ts->input_dev->name);
		goto err_input_register_device_failed;
	}

    	printk("[TSP] %s, irq=%d\n", __func__, client->irq );

    if (client->irq) {
		#if USE_THREADED_IRQ
		ret = request_threaded_irq(client->irq, synaptics_ts_irq_handler, synaptics_ts_work_func, IRQF_TRIGGER_FALLING | IRQF_ONESHOT, client->name, ts);
		#else		
		ret = request_irq(client->irq, synaptics_ts_irq_handler, IRQF_TRIGGER_FALLING, client->name, ts);
		#endif
		
		if (ret == 0)
			ts->use_irq = 1;
		else
			dev_err(&client->dev, "request_irq failed\n");
	}

	if (!ts->use_irq) {
		hrtimer_init(&ts->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
		ts->timer.function = synaptics_ts_timer_func;
		hrtimer_start(&ts->timer, ktime_set(1, 0), HRTIMER_MODE_REL);
	}
#if 1
#ifdef CONFIG_HAS_EARLYSUSPEND
	ts->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	ts->early_suspend.suspend = synaptics_ts_early_suspend;
	ts->early_suspend.resume = synaptics_ts_late_resume;
	register_early_suspend(&ts->early_suspend);
#endif
#endif
	printk(KERN_INFO "synaptics_ts_probe: Start touchscreen %s in %s mode\n", ts->input_dev->name, ts->use_irq ? "interrupt" : "polling");

	/* sys fs */
	touch_class = class_create(THIS_MODULE, "touch");
	if (IS_ERR(touch_class))
		pr_err("Failed to create class(touch)!\n");

	firmware_dev = device_create(touch_class, NULL, 0, NULL, "firmware");
	if (IS_ERR(firmware_dev))
		pr_err("Failed to create device(firmware)!\n");

	if (device_create_file(firmware_dev, &dev_attr_firmware) < 0)
		pr_err("Failed to create device file(%s)!\n", dev_attr_firmware.attr.name);
	if (device_create_file(firmware_dev, &dev_attr_firmware_ret) < 0)
		pr_err("Failed to create device file(%s)!\n", dev_attr_firmware_ret.attr.name);

	/* sys fs */
    

#if 0
	if(buf_tmp[0]<HEX_HW_VER){	//Firmware Update
		firm_update();
	}else if((buf_tmp[1]<HEX_SW_VER)||((buf_tmp[1]&0xF0)==0xF0)||(buf_tmp[1]==0)){
		printk("[TSP] firm_update START!!, ln=%d\n",__LINE__);
		firm_update();
	}else{
		printk("[TSP] Firmware Version is Up-to-date.\n");
	}
#endif


	return 0;

err_input_register_device_failed:
	input_free_device(ts->input_dev);

err_input_dev_alloc_failed:
	kfree(ts);
err_alloc_data_failed:
err_check_functionality_failed:
	return ret;
}

static int synaptics_ts_remove(struct i2c_client *client)
{
	struct synaptics_ts_data *ts = i2c_get_clientdata(client);
	unregister_early_suspend(&ts->early_suspend);
	if (ts->use_irq)
		free_irq(client->irq, ts);
	//else
	//	hrtimer_cancel(&ts->timer);
	input_unregister_device(ts->input_dev);
	kfree(ts);
	return 0;
}

static int synaptics_ts_suspend(struct i2c_client *client, pm_message_t mesg)
{
	struct synaptics_ts_data *ts = i2c_get_clientdata(client);

	printk("[TSP] %s+\n", __func__ );
	if (ts->use_irq)
	{
		disable_irq(client->irq);
	}
	gpio_direction_output( TSP_INT , 0 );
	gpio_direction_output( TSP_SCL , 0 ); 
	gpio_direction_output( TSP_SDA , 0 ); 

	bcm_gpio_pull_up(TSP_INT, false);
	bcm_gpio_pull_up_down_enable(TSP_INT, true);
	touch_ctrl_regulator(TOUCH_OFF);
    printk("[TSP] %s-\n", __func__ );
        
	return 0;
}

static int synaptics_ts_resume(struct i2c_client *client)
{
	int ret;
	struct synaptics_ts_data *ts = i2c_get_clientdata(client);
    	uint8_t i2c_addr = 0x1D;
	uint8_t buf[1];
#if 0//power_contol needs
	if (ts->power) {
		ret = ts->power(1);
		if (ret < 0)
			printk(KERN_ERR "synaptics_ts_resume power on failed\n");
	}
#endif
	//synaptics_init_panel(ts);

	gpio_direction_output( TSP_SCL , 1 ); 
	gpio_direction_output( TSP_SDA , 1 ); 
	//gpio_direction_output( TSP_INT , 1 ); 

	gpio_direction_input(TSP_INT);
	bcm_gpio_pull_up_down_enable(TSP_INT, false);

	touch_ctrl_regulator(TOUCH_ON);
    
	msleep(100);
		
	while (ts->use_irq)
	{
		ret = tsp_i2c_read( i2c_addr, buf, sizeof(buf));
		if (ret <= 0) {
			printk("[TSP] %d : i2c_transfer failed\n", __LINE__);
		}
		else if	( buf[0] == 0 )
		{
			printk("[TSP] %d : maybe unlimited loop! check to re-try code i2c_transfer failed\n", __LINE__);
			continue;
		}
		else
		{
			printk("[TSP] %s:%d, ver SW=%x\n", __func__,__LINE__, buf[0] );
			enable_irq(client->irq);
			break;
		}
		msleep(20);
	}
	printk("[TSP] %s-\n", __func__ );
	return 0;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void synaptics_ts_early_suspend(struct early_suspend *h)
{
	struct synaptics_ts_data *ts;
	ts = container_of(h, struct synaptics_ts_data, early_suspend);
	synaptics_ts_suspend(ts->client, PMSG_SUSPEND);
}

static void synaptics_ts_late_resume(struct early_suspend *h)
{
	struct synaptics_ts_data *ts;
	ts = container_of(h, struct synaptics_ts_data, early_suspend);
	synaptics_ts_resume(ts->client);
}
#endif

static const struct i2c_device_id synaptics_ts_id[] = {
	{ SYNAPTICS_I2C_RMI_NAME, 0 },
	{ }
};

static struct i2c_driver synaptics_ts_driver = {
	.probe		= synaptics_ts_probe,
	.remove		= synaptics_ts_remove,
#if 1
#ifndef CONFIG_HAS_EARLYSUSPEND
	.suspend	= synaptics_ts_suspend,
	.resume		= synaptics_ts_resume,
#endif
#endif
	.id_table	= synaptics_ts_id,
	.driver = {
		.name	= SYNAPTICS_I2C_RMI_NAME,
	},
};

static int __devinit synaptics_ts_init(void)
{
#if defined (CONFIG_TOUCHSCREEN_MMS128_TASSCOOPER)
	printk("[TSP][Synaptics][%s] %s\n", __func__,"Init Func Called");

	if(Is_MMS128_Connected()== 1)
	{
		printk("[TSP][Synaptics][%s] %s\n", __func__,"Melfas already detected !!");

		return -ENXIO;
	}
#endif


	printk("[TSP] %s\n", __func__ );

	gpio_request(TSP_INT, "ts_irq");
	gpio_direction_input(TSP_INT);
	//bcm_gpio_pull_up(TSP_INT, true);
	//bcm_gpio_pull_up_down_enable(TSP_INT, true);
	irq_set_irq_type(gpio_to_irq(TSP_INT), IRQ_TYPE_EDGE_FALLING);

	//disable BB internal pulls for touch int, scl, sda pin
	//bcm_gpio_pull_up_down_enable(TSP_INT, 0);
	bcm_gpio_pull_up_down_enable(TSP_SCL, 0);
	bcm_gpio_pull_up_down_enable(TSP_SDA, 0);

	gpio_direction_output( TSP_SCL , 1 ); 
	gpio_direction_output( TSP_SDA , 1 ); 		
 
	#if USE_THREADED_IRQ

	#else	
	synaptics_wq = create_singlethread_workqueue("synaptics_wq");
	if (!synaptics_wq)
		return -ENOMEM;
#endif

#if 0
	touch_regulator = regulator_get(NULL,"touch_vcc");
#if defined (__TOUCH_KEYLED__)
	touchkeyled_regulator = regulator_get(NULL,"touch_keyled");
#endif
#endif
	return i2c_add_driver(&synaptics_ts_driver);
}

static void __exit synaptics_ts_exit(void)
{
#if 0
	if (touch_regulator) 
	{
       	 regulator_put(touch_regulator);
		 touch_regulator = NULL;
    	}
#if defined (__TOUCH_KEYLED__)
	if (touchkeyled_regulator) 
	{
       	 regulator_put(touchkeyled_regulator);
		 touchkeyled_regulator = NULL;
    	}
#endif
#endif
	i2c_del_driver(&synaptics_ts_driver);

	#if USE_THREADED_IRQ

	#else	
	if (synaptics_wq)
		destroy_workqueue(synaptics_wq);
	#endif
}

static ssize_t firmware_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	uint8_t i2c_addr = 0x1C;
	uint8_t buf_tmp[2] = {0};
	int phone_ver = 0;

	printk("[TSP] %s\n",__func__);

	 		
	if ( HW_ver == 1  || HW_ver == 2 || HW_ver == 3)
	{
		/* for glass */
		phone_ver = 100;  /* SW Ver.4 - change this value if New firmware be released */ 	
	}
	else
	{
		phone_ver = 200; // Acryl type
		printk("[TSP] %s:%d,HW_ver is wrong!!\n", __func__,__LINE__ );
	}
	
	tsp_i2c_read( i2c_addr, buf_tmp, sizeof(buf_tmp));
	printk("[TSP] %s:%d, ver SW=%x, HW=%x\n", __func__,__LINE__, buf_tmp[1], buf_tmp[0] );

	/* below protocol is defined with App. ( juhwan.jeong@samsung.com )
		The TSP Driver report like XY as decimal.
		The X is the Firmware version what phone has.
		The Y is the Firmware version what TSP has. */

	sprintf(buf, "%d\n", phone_ver + buf_tmp[1]+(buf_tmp[0]*10) );

	return sprintf(buf, "%s", buf );
}

/* firmware - update */
static ssize_t firmware_store(
		struct device *dev, struct device_attribute *attr,
		const char *buf, size_t size)
{
	char *after;

	unsigned long value = simple_strtoul(buf, &after, 10);	
	printk(KERN_INFO "[TSP] %s, %d\n", __func__, __LINE__);
	firmware_ret_val = -1;
	printk("[TSP] firmware_store  valuie : %d\n",value);
	if ( value == 0 )
	{
		printk("[TSP] Firmware update start!!\n" );

		//firm_update( );
#if FIRM_TEST
		printk("[TSP] start update cypress touch firmware !!\n");
		g_FirmwareImageSize = CYPRESS_FIRMWARE_IMAGE_SIZE;

		if(g_pTouchFirmware == NULL)
		{
			printk("[TSP][ERROR] %s() kmalloc fail !! \n", __FUNCTION__);
			return -1;
		}


		/* ready for firmware code */
		size = issp_request_firmware("touch.hex");

		/* firmware update */
		//	issp_upgrade();

		g_FirmwareImageSize = 0;

		// step.1 power off/on

		// step.2 enable irq


#endif
		return size;
	}

	return size;
}

static ssize_t firmware_ret_show(struct device *dev, struct device_attribute *attr, char *buf)
{	
	printk("[TSP] %s!\n", __func__);

	return sprintf(buf, "%d", firmware_ret_val );
}

static ssize_t firmware_ret_store(
		struct device *dev, struct device_attribute *attr,
		const char *buf, size_t size)
{
	printk("[TSP] %s, operate nothing!\n", __func__);

	return size;
}


int firm_update( void )
{
	printk(KERN_INFO "[TSP] %s, %d\n", __func__, __LINE__);

	printk("[TSP] disable_irq : %d\n", __LINE__ );
	disable_irq(tsp_irq);
	local_irq_disable();
	
	// TEST
	// SKC gpio_configure( TSP_SCL, GPIOF_DRIVE_OUTPUT );


	//firmware_ret_val = cypress_update( HW_ver );

/*	if( HW_ver==1 || HW_ver==2 ||HW_ver==3 )
	{
		firmware_ret_val = cypress_update( HW_ver );
	}
	else	
	{
		printk(KERN_INFO "[TSP] %s, %d cypress_update blocked, HW ver=%d\n", __func__, __LINE__, HW_ver);
		firmware_ret_val = 0; // Fail
	}
*/
	msleep(1000);
	if( firmware_ret_val )
		printk(KERN_INFO "[TSP] %s success, %d\n", __func__, __LINE__);
	else	
		printk(KERN_INFO "[TSP] %s fail, %d\n", __func__, __LINE__);

	// SKC gpio_configure( TSP_SCL, GPIOF_DRIVE_OUTPUT );

	printk("[TSP] enable_irq : %d\n", __LINE__ );
	local_irq_enable();

	enable_irq(tsp_irq);

	return 0;
} 

#if FIRM_TEST
static void issp_request_firmware(char* update_file_name)
{
	int idx_src = 0;
	int idx_dst = 0;
	int line_no = 0;
	int dummy_no = 0;
	char buf[2];
	int ret = 0;

	struct device *dev = &ts_global->input_dev->dev;	
	const struct firmware * fw_entry;

	printk(KERN_INFO "[TSP] %s, %d\n", __func__, __LINE__);
	printk("[TSP] firmware file name : %s\n", update_file_name);

	ret = request_firmware(&fw_entry, update_file_name, dev);
	if ( ret )
	{
		printk("[TSP] request_firmware fail, ln=%d\n", ret );
		return ;
	}
	else
	{
		printk("[TSP] request_firmware success, ln=%d\n", ret );
		printk("[TSP][DEBUG] ret=%d, firmware size=%d\n", ret, fw_entry->size);
		printk("[TSP] %c %c %c %c %c\n", fw_entry->data[0], fw_entry->data[1], fw_entry->data[2], fw_entry->data[3], fw_entry->data[4]);
	}

	do {
		if(fw_entry->data[idx_src] == ':') // remove prefix
		{
			idx_src+=9;
			dummy_no++;

			if(dummy_no != line_no+1)
			{
				printk("[ERROR] Can not skip semicolon !! line_no(%d), dummy_no(%d)\n", line_no, dummy_no);
			}
		}
		else if(fw_entry->data[idx_src] == '\r') // return code
		{
			idx_src+=2; idx_dst--; line_no++;

			if( idx_dst > TSP_LINE_LENGTH*line_no)
			{
				printk("[ERROR] length buffer over error !! line_no(%d), idx_dst(%d)\n", line_no, idx_dst);
			}
		}
		else if(fw_entry->data[idx_src] == 0x0a) // return code
		{
			idx_src+=1; idx_dst--; line_no++;

			if( idx_dst > TSP_LINE_LENGTH*line_no)
			{
				printk("[ERROR] length buffer over error !! line_no(%d), idx_dst(%d)\n", line_no, idx_dst);
			}
		}
		else
		{
			sprintf(buf, "%c%c", fw_entry->data[idx_src], fw_entry->data[idx_src+1]);
			if(idx_dst > TSP_TOTAL_LINES*TSP_LINE_LENGTH)
			{
				printk("[ERROR] buffer over error !!  line_no(%d), idx_dst(%d)\n", line_no, idx_dst);
			}
			g_pTouchFirmware[idx_dst] = simple_strtol(buf, NULL, 16);
			idx_src+=2; idx_dst++;
		}
	} while ( line_no < TSP_TOTAL_LINES );

	release_firmware(fw_entry);
}
#endif

module_init(synaptics_ts_init);
module_exit(synaptics_ts_exit);

MODULE_DESCRIPTION("Synaptics Touchscreen Driver");
MODULE_LICENSE("GPL");
