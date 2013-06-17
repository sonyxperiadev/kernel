/* drivers/input/touchscreen/cypress_tma140_lucas.c
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
#include <linux/regulator/consumer.h>
#include <linux/irq.h>


#include <linux/firmware.h>
#include <linux/uaccess.h> 
/* firmware - update */

#define MAX_X	480 
#define MAX_Y	360
#define TSP_INT 86
#define TSP_SDA 85
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

//#define __TOUCH_DEBUG__
#define USE_THREADED_IRQ	1

//#define __TOUCH_KEYLED__ 

static struct regulator *touch_regulator=NULL;
#if defined (__TOUCH_KEYLED__)
static struct regulator *touchkeyled_regulator=NULL;
#endif

#if USE_THREADED_IRQ

#else
static struct workqueue_struct *touch_wq;
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

static report_finger_info_t fingerInfo[MAX_USING_FINGER_NUM];

struct touch_data {
	uint16_t addr;
	struct i2c_client *client;
	struct input_dev *input_dev;
	int use_irq;
	struct hrtimer timer;				////////////////////////IC
	struct work_struct  work;
	//struct work_struct  work_timer;		////////////////////////IC
	struct early_suspend early_suspend;
};


struct touch_data *ts_global;

/* firmware - update */
static int firmware_ret_val = -1;
static int HW_ver = -1;

unsigned char tsp_special_update = 0;
static char IsfwUpdate[20]={0};

/* touch information*/
unsigned char touch_vendor_id = 0;
unsigned char touch_hw_ver = 0;
unsigned char touch_sw_ver = 0;


int tsp_irq_num = 0;
int tsp_workqueue_num = 0;
int tsp_threadedirq_num = 0;

int g_touch_info_x = 0;
int g_touch_info_y = 0;
int g_touch_info_press = 0;


extern int cypress_update( int );
int tsp_i2c_read(u8 reg, unsigned char *rbuf, int buf_size);


/* sys fs */
struct class *touch_class;
EXPORT_SYMBOL(touch_class);
struct device *firmware_dev;
EXPORT_SYMBOL(firmware_dev);

static ssize_t firmware_In_Binary(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t firmware_In_TSP(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t firm_update(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t firmware_update_status(struct device *dev, struct device_attribute *attr, char *buf);

static ssize_t raw_show_tma140(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t raw_enable_tma140(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t raw_disable_tma140(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t read_rawdata_tma140(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t read_difference_tma140(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t rawdata_pass_fail_tma140(struct device *dev, struct device_attribute *attr, char *buf);

static DEVICE_ATTR(phone_fw_ver, 0777, firmware_In_Binary, NULL);
static DEVICE_ATTR(part_fw_ver, 0777, firmware_In_TSP, NULL);
//static DEVICE_ATTR(fw_update	, 0777, firm_update, firm_update);
static DEVICE_ATTR(fw_update	, 0777, firm_update, NULL );
static DEVICE_ATTR(raw, 0444, raw_show_tma140, NULL) ;
static DEVICE_ATTR(raw_enable, 0444, raw_enable_tma140, NULL) ;
static DEVICE_ATTR(raw_disable, 0444, raw_disable_tma140, NULL) ;
static DEVICE_ATTR(rawdata_read, 0444, read_rawdata_tma140, NULL) ;
static DEVICE_ATTR(difference_read, 0444, read_difference_tma140, NULL) ;
static DEVICE_ATTR(raw_value, 0444, rawdata_pass_fail_tma140, NULL) ;
//static DEVICE_ATTR(fw_upstatus, 0777, firmware_update_status, firmware_update_status);
static DEVICE_ATTR(fw_upstatus, 0777, firmware_update_status, NULL);

/* sys fs */

static int tsp_testmode = 0;
static int tsp_irq_operation = 0;
static unsigned int touch_present = 0;


#define FW_DOWNLOADING "Downloading"
#define FW_DOWNLOAD_COMPLETE "Complete"
#define FW_DOWNLOAD_FAIL "FAIL"
#define FWUP_NOW -1

#ifdef CONFIG_HAS_EARLYSUSPEND
static void ts_early_suspend(struct early_suspend *h);
static void ts_late_resume(struct early_suspend *h);
#endif

extern int bcm_gpio_pull_up(unsigned int gpio, bool up);
extern int bcm_gpio_pull_up_down_enable(unsigned int gpio, bool enable);
extern int set_irq_type(unsigned int irq, unsigned int type);

void touch_ctrl_regulator(int on_off)
{
	if(on_off==TOUCH_ON)
	{
		gpio_request(TOUCH_EN,"Touch_en");
		gpio_direction_output(TOUCH_EN,1);
		gpio_set_value(TOUCH_EN,1);
		gpio_free(TOUCH_EN);

#if defined (__TOUCH_KEYLED__)
        regulator_set_voltage(touchkeyled_regulator,3300000,3300000);
		regulator_enable(touchkeyled_regulator);
#endif
	}
	else
	{
		gpio_request(TOUCH_EN,"Touch_en");
		gpio_direction_output(TOUCH_EN,0);
		gpio_set_value(TOUCH_EN,0);
		gpio_free(TOUCH_EN);

#if defined (__TOUCH_KEYLED__) 
		regulator_disable(touchkeyled_regulator);
#endif
	}
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

        printk("[TSP] process_key_event : %d\n", tsk_msg);

	if(	tsk_msg	== 0)
	{
		input_report_key(ts_global->input_dev, st_old, 0);
		printk("[TSP] release keycode: %4d, keypress: %4d\n", st_old, 0);
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
		printk("[TSP] press keycode: %4d, keypress: %4d\n", keycode, 1);
		}

		st_old = keycode;


		}
	}
}

#if USE_THREADED_IRQ
static irqreturn_t ts_work_func(int irq, void *dev_id)
#else
static void ts_work_func(struct work_struct *work)
#endif
{
	int ret=0;
	//uint8_t buf[12];// 02h ~ 0Dh
	uint8_t buf[29];// 02h ~ 1Fh
	uint8_t buf_key[1];
	uint8_t i2c_addr = 0x02;
	int i = 0;
	int finger = 0;

#if USE_THREADED_IRQ

	struct touch_data *ts = dev_id;
    if(tsp_testmode)
		return IRQ_HANDLED;


#else
	struct touch_data *ts = container_of(work, struct touch_data, work);
#endif
	
	ret = tsp_i2c_read( i2c_addr, buf, sizeof(buf));

	if (ret <= 0) {
		printk("[TSP] i2c failed : ret=%d, ln=%d\n",ret, __LINE__);
		goto work_func_out;
	}
/*
	printk("[TSP] buf[0]:%d, buf[1]:%d, buf[2]=%d, buf[3]=%d, buf[4]=%d\n", buf[0], buf[1], buf[2], buf[3], buf[4]);
	printk("[TSP] buf[5]:%d, buf[6]:%d, buf[7]=%d, buf[8]=%d, buf[9]=%d\n", buf[5], buf[6], buf[7], buf[8], buf[9]);
	printk("[TSP] buf[10]:%d, buf[11]:%d, buf[12]=%d, buf[13]=%d, buf[14]=%d\n", buf[10], buf[11], buf[12], buf[13], buf[14]);
	printk("[TSP] buf[15]:%d, buf[16]:%d, buf[17]=%d, buf[18]=%d, buf[19]=%d\n", buf[15], buf[16], buf[17], buf[18], buf[19]);	
	printk("[TSP] buf[20]:%d, buf[21]:%d, buf[22]=%d, buf[23]=%d, buf[24]=%d\n", buf[20], buf[21], buf[22], buf[23], buf[24]);
	printk("[TSP] buf[25]:%d, buf[26]:%d, buf[27]=%d, buf[28]=%d=%d\n", buf[25], buf[26], buf[27], buf[28]);
*/
	finger = buf[0] & 0x0F;	//number of touch finger
	buf_key[0] = buf[25] & 0x03; //information of touch key
	
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
        input_report_key(ts->input_dev, BTN_TOUCH, fingerInfo[i].status); /*must be added for ICS*/
        
     #if defined(__TOUCH_DEBUG__)
		printk("[TSP] i[%d] id[%d] xyz[%d, %d, %x] status[%x]\n", i, fingerInfo[i].id, fingerInfo[i].x, fingerInfo[i].y, fingerInfo[i].z, fingerInfo[i].status);	
    #endif
	}

	input_sync(ts->input_dev);

work_func_out:
	if (ts->use_irq)
	{
		#if USE_THREADED_IRQ

		#else
		enable_irq(ts->client->irq);
		#endif
	}
	
	tsp_irq_operation = 0;
	
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



static enum hrtimer_restart ts_timer_func(struct hrtimer *timer)
{
	#if USE_THREADED_IRQ
	
	#else
	queue_work(touch_wq, &ts_global->work);
	#endif

	hrtimer_start(&ts_global->timer, ktime_set(0, 12500000), HRTIMER_MODE_REL);
	return HRTIMER_NORESTART;
}

static irqreturn_t ts_irq_handler(int irq, void *dev_id)
{
#ifndef USE_THREADED_IRQ
	struct touch_data *ts = dev_id;
#endif

	if(tsp_testmode)
		return IRQ_WAKE_THREAD;

	tsp_irq_operation = 1;
	
	//printk("[TSP] %s, %d\n", __func__, __LINE__ );
	
	#if USE_THREADED_IRQ

	#else
	disable_irq_nosync(ts->client->irq);
	#endif


	#if USE_THREADED_IRQ
	return IRQ_WAKE_THREAD;
	#else
	queue_work(touch_wq, &ts->work);
	return IRQ_HANDLED;
	#endif
}

int ts_check(void)
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
		printk("[TSP][%s] %s\n", __func__,"Failed i2c");
		ret = 0;
	}
	else 
	{
		printk("[TSP][%s] %s\n", __func__,"Passed i2c");

		printk("[TSP][%s][SlaveAddress : 0x%x][VendorID : 0x%x] [HW : 0x%x] [SW : 0x%x]\n", __func__,ts_global->client->addr, buf_tmp[0], buf_tmp[1], buf_tmp[2]);

		if ( buf_tmp[0] == 0xf0 )//(ts->hw_rev == 0) && (ts->fw_ver == 2))
		{
			ret = 1;
			printk("[TSP][%s] %s\n", __func__,"Passed ts_check");
		}
		else
		{
			ret = 0;
			printk("[TSP][%s] %s\n", __func__,"Failed ts_check");
		}
		
	}

	return ret;
}

static int ts_probe(
	struct i2c_client *client, const struct i2c_device_id *id)
{
	struct touch_data *ts;

  	uint8_t buf_tmp[3]={0,0,0};
	uint8_t addr[1];	
	int i;
    	int ret = 0, key = 0;


	printk("[TSP] %s, %d\n", __func__, __LINE__ );

//	touch_ctrl_regulator(TOUCH_ON);
//	msleep(100);	
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
	INIT_WORK(&ts->work, ts_work_func);
	#endif

	ts->client = client;
	i2c_set_clientdata(client, ts);

	ts_global = ts;

	tsp_irq=client->irq;

	printk("[TSP] tsp_irq = %d\n",tsp_irq);


	ts->input_dev = input_allocate_device();
	if (ts->input_dev == NULL) {
		ret = -ENOMEM;
		printk(KERN_ERR "ts_probe: Failed to allocate input device\n");
		goto err_input_dev_alloc_failed;
	}

	ts->input_dev->name = "sec_touchscreen";


	ts->input_dev->keybit[BIT_WORD(KEY_POWER)] |= BIT_MASK(KEY_POWER);
	
	set_bit(EV_SYN, ts->input_dev->evbit);
	set_bit(EV_KEY, ts->input_dev->evbit);
	set_bit(BTN_TOUCH, ts->input_dev->keybit);
	set_bit(EV_ABS, ts->input_dev->evbit);
	set_bit(INPUT_PROP_DIRECT, ts->input_dev->propbit); /*must be added for ICS*/

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
		printk(KERN_ERR "ts_probe: Unable to register %s input device\n", ts->input_dev->name);
		goto err_input_register_device_failed;
	}

    	printk("[TSP] %s, irq=%d\n", __func__, client->irq );

    if (client->irq) {
		#if USE_THREADED_IRQ
		ret = request_threaded_irq(client->irq, ts_irq_handler, ts_work_func, IRQF_TRIGGER_FALLING | IRQF_ONESHOT, client->name, ts);
		#else		
		ret = request_irq(client->irq, ts_irq_handler, IRQF_TRIGGER_FALLING, client->name, ts);
		#endif
		
		if (ret == 0)
			ts->use_irq = 1;
		else
			dev_err(&client->dev, "request_irq failed\n");
	}

	if (!ts->use_irq) {
		hrtimer_init(&ts->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
		ts->timer.function = ts_timer_func;
		hrtimer_start(&ts->timer, ktime_set(1, 0), HRTIMER_MODE_REL);
	}
#ifdef CONFIG_HAS_EARLYSUSPEND
	ts->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	ts->early_suspend.suspend = ts_early_suspend;
	ts->early_suspend.resume = ts_late_resume;
	register_early_suspend(&ts->early_suspend);
#endif
	printk(KERN_INFO "ts_probe: Start touchscreen %s in %s mode\n", ts->input_dev->name, ts->use_irq ? "interrupt" : "polling");

	/* sys fs */
	touch_class = class_create(THIS_MODULE, "touch");
	if (IS_ERR(touch_class))
		pr_err("Failed to create class(touch)!\n");

	firmware_dev = device_create(touch_class, NULL, 0, NULL, "firmware");
	if (IS_ERR(firmware_dev))
		pr_err("Failed to create device(firmware)!\n");

	if (device_create_file(firmware_dev, &dev_attr_phone_fw_ver) < 0)
		pr_err("Failed to create device file(%s)!\n", dev_attr_phone_fw_ver.attr.name);
	if (device_create_file(firmware_dev, &dev_attr_part_fw_ver) < 0)
		pr_err("Failed to create device file(%s)!\n", dev_attr_part_fw_ver.attr.name);
	if (device_create_file(firmware_dev, &dev_attr_fw_update) < 0)
		pr_err("Failed to create device file(%s)!\n", dev_attr_fw_update.attr.name);
	if (device_create_file(firmware_dev, &dev_attr_raw) < 0)
		pr_err("Failed to create device file(%s)!\n", dev_attr_raw.attr.name);
	if (device_create_file(firmware_dev, &dev_attr_fw_upstatus) < 0)
		pr_err("[TSP] Failed to create device file(%s)!\n", dev_attr_fw_upstatus.attr.name);	
	if (device_create_file(firmware_dev, &dev_attr_raw_enable) < 0)
		pr_err("Failed to create device file(%s)!\n", dev_attr_raw_enable.attr.name);
	if (device_create_file(firmware_dev, &dev_attr_raw_disable) < 0)
		pr_err("Failed to create device file(%s)!\n", dev_attr_raw_disable.attr.name);
	if (device_create_file(firmware_dev, &dev_attr_rawdata_read) < 0)
	   pr_err("Failed to create device file(%s)!\n", dev_attr_rawdata_read.attr.name);
	if (device_create_file(firmware_dev, &dev_attr_difference_read) < 0)
	   pr_err("Failed to create device file(%s)!\n", dev_attr_difference_read.attr.name);
	if (device_create_file(firmware_dev, &dev_attr_raw_value) < 0)
		pr_err("Failed to create device file(%s)!\n", dev_attr_raw_value.attr.name);		


		/* Check point - i2c check - start */	
		for (i = 0; i < 2; i++)
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

	
		HW_ver = touch_hw_ver;


		if(ret >= 0)
		{
			touch_present = 1;
			//hrtimer_start(&ts->timer, ktime_set(2, 0), HRTIMER_MODE_REL);
		}
		else//if(ret < 0) 
		{
			printk(KERN_ERR "i2c_transfer failed\n");
			printk("[TSP] %s, ln:%d, Failed to register TSP!!!\n\tcheck the i2c line!!!, ret=%d\n", __func__,__LINE__, ret);
	 		//goto err_check_functionality_failed;
		}
		/* Check point - i2c check - end */


	return 0;

err_input_register_device_failed:
	input_free_device(ts->input_dev);

err_input_dev_alloc_failed:
	kfree(ts);
err_alloc_data_failed:

	//err_check_functionality_failed:
	return ret;

}

static int ts_remove(struct i2c_client *client)
{
	struct touch_data *ts = i2c_get_clientdata(client);
	unregister_early_suspend(&ts->early_suspend);
	if (ts->use_irq)
		free_irq(client->irq, ts);
	//else
	//	hrtimer_cancel(&ts->timer);
	input_unregister_device(ts->input_dev);
	kfree(ts);
	return 0;
}

static int ts_suspend(struct i2c_client *client, pm_message_t mesg)
{
	struct touch_data *ts = i2c_get_clientdata(client);

	printk("[TSP] %s+\n", __func__ );
	if (ts->use_irq)
	{
		disable_irq(client->irq);
	}
	gpio_direction_output( TSP_INT , 0 );
	gpio_direction_output( TSP_SCL , 0 ); 
	gpio_direction_output( TSP_SDA , 0 ); 

	#if USE_THREADED_IRQ
	
	#else
	ret = cancel_work_sync(&ts->work);
	
	#endif	

	msleep(20);	
	
	touch_ctrl_regulator(TOUCH_OFF);
    printk("[TSP] %s-\n", __func__ );
        
	return 0;
}

static int ts_resume(struct i2c_client *client)
{
	int ret;
    	uint8_t i2c_addr = 0x1D;
	uint8_t buf[1];


	gpio_direction_output( TSP_SCL , 1 ); 
	gpio_direction_output( TSP_SDA , 1 ); 

	gpio_direction_input(TSP_INT);


	touch_ctrl_regulator(TOUCH_ON);
    
	msleep(100);
		
	ret = tsp_i2c_read( i2c_addr, buf, sizeof(buf));
	printk("[TSP] %s:%d, ver SW=%x\n", __func__,__LINE__, buf[0] );
	enable_irq(client->irq);
	
	printk("[TSP] %s-\n", __func__ );
	return 0;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void ts_early_suspend(struct early_suspend *h)
{
	struct touch_data *ts;
	ts = container_of(h, struct touch_data, early_suspend);
	ts_suspend(ts->client, PMSG_SUSPEND);
}

static void ts_late_resume(struct early_suspend *h)
{
	struct touch_data *ts;
	ts = container_of(h, struct touch_data, early_suspend);
	ts_resume(ts->client);
}
#endif

static const struct i2c_device_id ts_id[] = {
	{ "cypress-tma140", 0 },
	{ }
};

static struct i2c_driver ts_driver = {
	.probe		= ts_probe,
	.remove		= ts_remove,
#ifndef CONFIG_HAS_EARLYSUSPEND
	.suspend	= ts_suspend,
	.resume		= ts_resume,
#endif
	.id_table	= ts_id,
	.driver = {
		.name	= "cypress-tma140",
	},
};

static int __devinit tsp_driver_init(void)
{

	printk("[TSP] %s\n", __func__ );

	gpio_request(TSP_INT, "ts_irq");
	gpio_direction_input(TSP_INT);

	irq_set_irq_type(gpio_to_irq(TSP_INT), IRQ_TYPE_EDGE_FALLING);


	gpio_direction_output( TSP_SCL , 1 ); 
	gpio_direction_output( TSP_SDA , 1 ); 		
 
#if USE_THREADED_IRQ

#else	
	touch_wq = create_singlethread_workqueue("touch_wq");
	if (!touch_wq)
		return -ENOMEM;
#endif

//	touch_regulator = regulator_get(NULL,"touch_vcc");
#if defined (__TOUCH_KEYLED__)
	touchkeyled_regulator = regulator_get(NULL,"touch_keyled");
#endif
	return i2c_add_driver(&ts_driver);
}

static void __exit tsp_driver_exit(void)
{
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
	i2c_del_driver(&ts_driver);

	#if USE_THREADED_IRQ

	#else	
	if (touch_wq)
		destroy_workqueue(touch_wq);
	#endif
}

static ssize_t firmware_In_Binary(struct device *dev, struct device_attribute *attr, char *buf)
{

	int phone_ver = 0x01;

	sprintf(buf, "%x\n", phone_ver);

	return sprintf(buf, "%s", buf );
}


static ssize_t firmware_In_TSP(struct device *dev, struct device_attribute *attr, char *buf)
{

	uint8_t i2c_addr = 0x1B;
	uint8_t buf_tmp[3] = {0};

	printk("[TSP] %s\n",__func__);

	tsp_i2c_read( i2c_addr, buf_tmp, sizeof(buf_tmp));

	touch_vendor_id = buf_tmp[0] & 0xF0;
	touch_hw_ver = buf_tmp[1];
	touch_sw_ver = buf_tmp[2];
	printk("[TSP] %s:%d, ver tsp=%x, HW=%x, SW=%x\n", __func__,__LINE__, touch_vendor_id, touch_hw_ver, touch_sw_ver);

	HW_ver = touch_hw_ver;

			
//	sprintf(buf, "%x\n", 0x1000000 + (touch_hw_ver*0x10000) + (touch_sw_ver*0x100) +  phone_ver);
	sprintf(buf, "%x\n", touch_sw_ver);

	return sprintf(buf, "%s", buf );
}


#define TMA140_RET_SUCCESS 0x00
int sv_tch_firmware_update = 0;


static ssize_t firm_update(struct device *dev, struct device_attribute *attr, char *buf)
		{
	uint8_t update_num;

	printk("[TSP] %s!\n", __func__);

	sprintf(IsfwUpdate,"%s\n",FW_DOWNLOADING);


	printk("[TSP] disable_irq : %d\n", __LINE__ );
	disable_irq(tsp_irq);
	local_irq_disable();
	
	for(update_num = 1; update_num <= 5 ; update_num++)
	{
		sv_tch_firmware_update = cypress_update(HW_ver);

		if(sv_tch_firmware_update == TMA140_RET_SUCCESS)
		{
			firmware_ret_val = 1; //SUCCESS
			sprintf(IsfwUpdate,"%s\n",FW_DOWNLOAD_COMPLETE);
			printk( "[TSP] %s, %d : firmware update SUCCESS !!\n", __func__, __LINE__);
			break;
		}
		else
		{
			printk( "[TSP] %s, %d : firmware update RETRY !!\n", __func__, __LINE__);
			if(update_num == 5)
			{
				firmware_ret_val = -1; //FAIL
			sprintf(IsfwUpdate,"%s\n",FW_DOWNLOAD_FAIL);				
			printk( "[TSP] %s, %d : firmware update FAIL !!\n", __func__, __LINE__);
			}
		}
	}

	printk("[TSP] enable_irq : %d\n", __LINE__ );
	local_irq_enable();
	enable_irq(tsp_irq);



	return sprintf(buf, "%d", firmware_ret_val );
			}

static ssize_t firmware_update_status(struct device *dev, struct device_attribute *attr, char *buf)
		{
	printk("[TSP] %s\n",__func__);

	return sprintf(buf, "%s\n", IsfwUpdate);
		}


static ssize_t raw_show_tma140(struct device *dev, struct device_attribute *attr, char *buf)
{
	
	int tma140_col_num = 8;	//0 ~ 7
	int tma140_row_num = 10;//0 ~ 9

	int  written_bytes = 0 ;	/* & error check */

	uint8_t buf1[1]={0,};
	uint8_t buf2[80]={0,};

	uint16_t ref1[80]={0,};
	uint16_t ref2[80]={0,};

	int i;
	int ret;

	uint8_t i2c_addr;

	printk("[TSP] %s entered. line : %d, \n", __func__,__LINE__);

	if(!tsp_testmode)
		return 0;
	

	/////* Raw Value */////
	/////* Enter Raw Data Mode */////
	for (i = 0; i < I2C_RETRY_CNT; i++)
	{
		buf1[0] = 0x00;//address
		buf1[1] = 0x40;//value
		ret = i2c_master_send(ts_global->client, buf1, 2);
	
		if (ret >= 0)
			break; // i2c success
	}
	msleep(10);
	for (i = 0; i < I2C_RETRY_CNT; i++)
	{
		buf1[0] = 0x00;//address
		buf1[1] = 0xC0;//value
		ret = i2c_master_send(ts_global->client, buf1, 2);
	
		if (ret >= 0)
			break; // i2c success
	}
	msleep(50);
	
	/////* Read Raw Data */////
	i2c_addr = 0x07;
	tsp_i2c_read( i2c_addr, buf2, sizeof(buf2));

	printk("[TSP] Raw Value : ");
	for(i = 0 ; i < (tma140_col_num * tma140_row_num) ; i++)
	{
		ref1[i] = buf2[i];
		printk(" [%d]%3d", i, buf2[i]);
	}
	printk("\n");


	/////* Difference Value */////
	/////* Enter Difference Data Mode */////
	for (i = 0; i < I2C_RETRY_CNT; i++)
	{
		buf1[0] = 0x00;//address
		buf1[1] = 0x50;//value
		ret = i2c_master_send(ts_global->client, buf1, 2);
	
		if (ret >= 0)
			break; // i2c success
	}
	msleep(10);
	for (i = 0; i < I2C_RETRY_CNT; i++)
	{
		buf1[0] = 0x00;//address
		buf1[1] = 0xD0;//value
		ret = i2c_master_send(ts_global->client, buf1, 2);
	
		if (ret >= 0)
			break; // i2c success
	}	
	msleep(50);

	/////* Read Difference Data */////
	i2c_addr = 0x07;
	tsp_i2c_read( i2c_addr, buf2, sizeof(buf2));


	printk("[TSP] Diff Value : ");
	for(i = 0 ; i < (tma140_col_num * tma140_row_num) ; i++)
	{
		ref2[i] = buf2[i];
		printk(" %d", ref2[i]);
	}
	printk("\n");
	

	/////* Send Value */////
	for (i = 0; i < (tma140_col_num * tma140_row_num) ; i++)
	{
		written_bytes += sprintf(buf+written_bytes, "%d %d\n", ref1[i], ref2[i]);
	}

	if (written_bytes > 0)
		return written_bytes ;

	return sprintf(buf, "-1") ;
}


static ssize_t raw_enable_tma140(struct device *dev, struct device_attribute *attr, char *buf)
{
	//	uint8_t buf1[2] = {0,};

	/////* Enter Inspection Mode */////
/*	for (i = 0; i < I2C_RETRY_CNT; i++)
	{
		buf1[0] = 0x00;//address
		buf1[1] = 0x40;//value
		ret = i2c_master_send(ts_global->client, buf1, 2);	//enter Inspection Mode
	
		if (ret >= 0)
			break; // i2c success
	}*/
	
	tsp_testmode = 1;
	printk("[TSP] %s start. line : %d, \n", __func__,__LINE__);

	mdelay(100); 

	return 1;
}


static ssize_t raw_disable_tma140(struct device *dev, struct device_attribute *attr, char *buf)
{
	int i, ret;
	uint8_t buf1[2] = {0,};

	/////* Exit Inspection Mode */////
	for (i = 0; i < I2C_RETRY_CNT; i++)
	{	
		buf1[0] = 0x00;//address
		buf1[1] = 0x00;//value
		ret = i2c_master_send(ts_global->client, buf1, 2);	//exit Inspection Mode
	
		if (ret >= 0)
			break; // i2c success
	}
	
	tsp_testmode = 0;
	printk("[TSP] %s stop. line : %d, \n", __func__,__LINE__);

	return 1;
}


static ssize_t read_rawdata_tma140(struct device *dev, struct device_attribute *attr, char *buf)//AT+TSPPTEST=1,1,1
{

	int tma140_col_num = 8;	//0 ~ 7
	int tma140_row_num = 10;//0 ~ 9

	int  written_bytes = 0 ;	/* & error check */

	uint8_t buf1[1]={0,};
	uint8_t buf2[80]={0,};

	uint16_t ref1[80]={0,};

	int i;
	int ret;

	uint8_t i2c_addr;

	printk("[TSP] %s entered. line : %d, \n", __func__,__LINE__);	

	tsp_testmode = 1;

	/////* Raw Value */////
	/////* Enter Raw Data Mode */////
	for (i = 0; i < I2C_RETRY_CNT; i++)
	{
		buf1[0] = 0x00;//address
		buf1[1] = 0x40;//value
		ret = i2c_master_send(ts_global->client, buf1, 2);
	
		if (ret >= 0)
			break; // i2c success
	}
	msleep(10);
	for (i = 0; i < I2C_RETRY_CNT; i++)
	{
		buf1[0] = 0x00;//address
		buf1[1] = 0xC0;//value
		ret = i2c_master_send(ts_global->client, buf1, 2);
	
		if (ret >= 0)
			break; // i2c success
	}
	msleep(50);
	
	/////* Read Raw Data */////
	i2c_addr = 0x07;
	tsp_i2c_read( i2c_addr, buf2, sizeof(buf2));

//	printk("[TSP] Raw Value : ");
//	for(i = 0 ; i < (tma140_col_num * tma140_row_num) ; i++)
//	{
//		ref1[i] = buf2[i];
//		printk(" [%d]%3d", i, buf2[i]);
//	}
//	printk("\n");


	/////* Exit Inspection Mode */////
	for (i = 0; i < I2C_RETRY_CNT; i++)
	{	
		buf1[0] = 0x00;//address
		buf1[1] = 0x00;//value
		ret = i2c_master_send(ts_global->client, buf1, 2);	//exit Inspection Mode
	
		if (ret >= 0)
			break; // i2c success
	}


	/////* Send Value */////
	for (i = 0; i < (tma140_col_num * tma140_row_num) ; i++)
	{
		written_bytes += sprintf(buf+written_bytes, ",%3d", ref1[i]);
	}

//	printk("[TSP] %s\n", buf);
	
	mdelay(100);

	tsp_testmode = 0;

	if (written_bytes > 0)
		return written_bytes ;

	return sprintf(buf, "-1") ;
}


static ssize_t read_difference_tma140(struct device *dev, struct device_attribute *attr, char *buf)//AT+TSPPTEST=1,2,1
	{
		
		int tma140_col_num = 8; //0 ~ 7
		int tma140_row_num = 10;//0 ~ 9
	
		int  written_bytes = 0 ;	/* & error check */
	
		uint8_t buf1[1]={0,};
		uint8_t buf2[80]={0,};
	
		uint16_t ref1[80]={0,};
	
		int i;
		int ret;
	
		uint8_t i2c_addr;

		printk("[TSP] %s entered. line : %d, \n", __func__,__LINE__);
		
		tsp_testmode = 1;
	
		/////* Difference Value */////
		/////* Enter Difference Data Mode */////
		for (i = 0; i < I2C_RETRY_CNT; i++)
		{
			buf1[0] = 0x00;//address
			buf1[1] = 0x50;//value
			ret = i2c_master_send(ts_global->client, buf1, 2);
		
			if (ret >= 0)
				break; // i2c success
		}
		msleep(10);
		for (i = 0; i < I2C_RETRY_CNT; i++)
		{
			buf1[0] = 0x00;//address
			buf1[1] = 0xD0;//value
			ret = i2c_master_send(ts_global->client, buf1, 2);
		
			if (ret >= 0)
				break; // i2c success
		}
		msleep(50);
		
		/////* Read Difference Data */////
		i2c_addr = 0x07;
		tsp_i2c_read( i2c_addr, buf2, sizeof(buf2));
	
	//	printk("[TSP] Difference Value : ");
	//	for(i = 0 ; i < (tma140_col_num * tma140_row_num) ; i++)
	//	{
	//		ref1[i] = buf2[i];
	//		printk(" [%d]%3d", i, buf2[i]);
	//	}
	//	printk("\n");
	
	
		/////* Exit Inspection Mode */////
		for (i = 0; i < I2C_RETRY_CNT; i++)
		{	
			buf1[0] = 0x00;//address
			buf1[1] = 0x00;//value
			ret = i2c_master_send(ts_global->client, buf1, 2);	//exit Inspection Mode
		
			if (ret >= 0)
				break; // i2c success
		}
	
	
		/////* Send Value */////
		for (i = 0; i < (tma140_col_num * tma140_row_num) ; i++)
		{
			written_bytes += sprintf(buf+written_bytes, ",%3d", ref1[i]);
		}
	
	//	printk("[TSP] %s\n", buf);
		
		mdelay(100);
	
		tsp_testmode = 0;
	
		if (written_bytes > 0)
			return written_bytes ;
	
		return sprintf(buf, "-1") ;
	}



static ssize_t rawdata_pass_fail_tma140(struct device *dev, struct device_attribute *attr, char *buf)
{
	int tst200_col_num = 9;	//0 ~ 8
	int tst200_row_num = 7;	//0 ~ 6

	uint8_t buf1[2]={0,};
	uint8_t buf2[32]={0,};

	uint16_t ref1[16]={0,};

	int i;
	int ret;

	uint16_t RAWDATA_MAX[16] = {16383,16383,16383,16383,16383,16383,16383,16383,16383,16383,16383,16383,16383,16383,16383,16383};
	uint16_t RAWDATA_MIN[16] = {0,};

	tsp_testmode = 1;

	/////* Raw Value */////
	for (i = 0; i < I2C_RETRY_CNT; i++)
	{
		buf1[0] = 0x0C; //address

		ret = i2c_master_send(ts_global->client, buf1, 1);

		if (ret >= 0)
		{
			ret = i2c_master_recv(ts_global->client, buf2, (tst200_col_num + tst200_row_num)*2);

			if (ret >= 0)
				break; // i2c success
		}
	}

	for(i = 0 ; i < (tst200_col_num + tst200_row_num); i++)
	{
		ref1[i] = (buf2[i*2] <<8) + buf2[i*2+1];
		printk(" %d", ref1[i]);
	}
	printk("\n");


	/////* Check MAX/MIN Raw Value */////
	for (i = 0; i < (tst200_col_num + tst200_row_num); i++)
    {
		if( RAWDATA_MAX[i] < ref1[i]) return sprintf(buf, "0"); // fail
		if( RAWDATA_MIN[i] > ref1[i]) return sprintf(buf, "0"); // fail
    }

	tsp_testmode = 0;

    return sprintf(buf, "1"); // success
 }


module_init(tsp_driver_init);
module_exit(tsp_driver_exit);

MODULE_DESCRIPTION("Cypress Touchscreen Driver");
MODULE_LICENSE("GPL");
