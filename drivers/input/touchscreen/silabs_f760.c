/* drivers/input/touchscreen/silabs_f760.c
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
#include <linux/firmware.h>
//PSJ #include <plat/syscfg.h>

#define MAX_X	240 
#define MAX_Y	320
#define TSP_INT 91
#define TOUCH_EN 43 //PSJ

#define F760_MAX_TOUCH		2
#define ESCAPE_ADDR 	    0xAA
#define TS_READ_START_ADDR 	    0x10
#define TS_READ_VERSION_ADDR	0x1F
#define TS_READ_ESD_ADDR	0x1E
#define TS_READ_REGS_LEN 		13
#define SILABS_MAX_TOUCH		F760_MAX_TOUCH
#define MTSI_VERSION		    0x05

//#define __TOUCH_DEBUG__ 1

#define I2C_RETRY_CNT			3

#define TOUCH_ON 1
#define TOUCH_OFF 0

#define PRESS_KEY				1
#define RELEASE_KEY				0
#define MAX_KEYS	     2

#define SET_DOWNLOAD_BY_GPIO	1

#define SILABS_TS_NAME "silabs-f760"

#define YTE_MODULE_VER   0x02
#define SMAC_MODULE_VER   0x03
#define YTE_MODULE_VER_OLD   0x04
#define YTE_MODULE_VER_NEW   0x0A
#define SMAC_MODULE_VER_OLD    0x05
#define SMAC_MODULE_VER_NEW   0x07
#define FW_VER  0x03
#define FW_VER_OLD  0x0F
#define FW_VER_OLD_SMAC  0x14
#define FW_VER_NEW_YTE  0x14
#define FW_VER_NEW_SMAC 0x14
#if defined(CONFIG_TARGET_LOCALE_AUS_TEL)
#define T_YTE_MODULE_VER   0x08
#define T_YTE_MODULE_VER_NEW   0x08
#define T_FW_VER_YTE  0x0A
#define T_FW_VER_OLD_YTE  0x0A
#define T_FW_VER_NEW_YTE  0x0E

#define T_SMAC_MODULE_VER   0x14		//Module_ver = 20
#define T_SMAC_FW_VER   0x0F			//FW_ver = 15
#endif

#define NUM_TX_CHANNEL 12
#define NUM_RX_CHANNEL 9
#define JIG_MODE_COMMAND 0xA0
#define RAWDATA_ADDRESS          0x003A
#define BASELINE_ADDRESS         0x0112
#define  QUICKSENSE_OVERHEAD     6 
#define I2CMAP_BUTTON_ADDRESS    (0x0212 + 0x22)
#define  NUM_MTRBUTTONS             2

static int prev_wdog_val = -1;


static struct workqueue_struct *check_ic_wq;

static struct regulator *touch_regulator=NULL;
//add by brcm

int touch_id[2], posX[2], posY[2], strength[2];
int Press_Check=0;

static int firmware_ret_val = -1;
static int pre_ta_stat = 0;
int touch_check=0;
int tsp_irq;
int tsp_chheck=0;
int TSP_MODULE_ID;
EXPORT_SYMBOL(TSP_MODULE_ID);
int FW_VERSION;
EXPORT_SYMBOL(FW_VERSION);

uint8_t buf_firmware[3];
int firm_update( void );
void set_tsp_for_ta_detect(int);
static int testmode = 1;
int PHONE_VER;
int tsp_status=0;
unsigned long check_node;

int Tx_Channel = NUM_TX_CHANNEL;
int Rx_Channel = NUM_RX_CHANNEL;
uint16_t baseline_node[NUM_TX_CHANNEL][NUM_RX_CHANNEL]={{0,},};

#if SET_DOWNLOAD_BY_GPIO
#include "bootloader.h"
#endif // SET_DOWNLOAD_BY_GPIO

enum
{
	TOUCH_SCREEN=0,
	TOUCH_KEY
};

struct key_info
{
	int key_value;
	int key_press;	
};

struct muti_touch_info
{
	int strength;
	int status;
	int width;	
	int posX;
	int posY;
};

struct silabs_ts_data {
	uint16_t addr;
	struct i2c_client *client; 
	struct input_dev *input_dev;
    	int use_irq;
	struct hrtimer timer;
	struct work_struct  work;
    	struct work_struct  work_timer;
	struct early_suspend early_suspend;
};

struct silabs_ts_data *ts_global;

/* sys fs */
struct class *touch_class;
EXPORT_SYMBOL(touch_class);
struct device *firmware_dev;
EXPORT_SYMBOL(firmware_dev);

static ssize_t firmware_show(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t firmware_ret_show(struct device *dev, struct device_attribute *attr, char *buf);
static DEVICE_ATTR(firmware	, S_IRUGO, firmware_show, NULL);
static DEVICE_ATTR(firmware_ret	, S_IRUGO, firmware_ret_show, NULL);

static ssize_t rawdata_show_silabs(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t raw_enable_silabs(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t raw_disable_silabs(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t baseline_show_silabs(struct device *dev, struct device_attribute *attr, char *buf1);
static ssize_t diff_show_silabs(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t fimware_show_versname(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t rawdata_pass_fail_silabs(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t read_node(struct device *dev, struct device_attribute *attr, char *buf);
static DEVICE_ATTR(reference, S_IRUGO, baseline_show_silabs, NULL) ;
static DEVICE_ATTR(raw, S_IRUGO, rawdata_show_silabs, NULL) ;
static DEVICE_ATTR(raw_enable, S_IRUGO, raw_enable_silabs, NULL) ;
static DEVICE_ATTR(raw_disable, S_IRUGO, raw_disable_silabs, NULL) ;
static DEVICE_ATTR(diff, S_IRUGO, diff_show_silabs, NULL) ;
static DEVICE_ATTR(versname, S_IRUGO, fimware_show_versname, NULL) ;
static DEVICE_ATTR(raw_value, S_IRUGO, rawdata_pass_fail_silabs, NULL) ;
static DEVICE_ATTR(node_read, S_IRUGO, read_node, NULL) ;

/* sys fs */
#ifdef CONFIG_HAS_EARLYSUSPEND
static void silabs_ts_early_suspend(struct early_suspend *h);
static void silabs_ts_late_resume(struct early_suspend *h);
#endif

extern int bcm_gpio_pull_up(unsigned int gpio, bool up);
extern int bcm_gpio_pull_up_down_enable(unsigned int gpio, bool enable);
extern int set_irq_type(unsigned int irq, unsigned int type);
//PSJ extern int tsp_charger_type_status;
void TSP_forced_release_forkey(void);

static struct muti_touch_info g_Mtouch_info[SILABS_MAX_TOUCH];
static struct key_info touchkey_status[MAX_KEYS];

void touch_ctrl_regulator(int on_off)
{
    int rc;
	if(on_off==TOUCH_ON)
	{
        #if 0
			regulator_set_voltage(touch_regulator,2900000,2900000);
			regulator_enable(touch_regulator);
            #endif
            rc = gpio_request(TOUCH_EN,"Touch_en");
		if (rc < 0)
		{
			printk("[TSP] touch_power_control unable to request GPIO pin");
			//printk(KERN_ERR "unable to request GPIO pin %d\n", TSP_INT_GPIO_PIN);
		}
		gpio_direction_output(TOUCH_EN,1);
		gpio_set_value(TOUCH_EN,1);
		gpio_free(TOUCH_EN);
	}
	else
	{
        #if 0
			regulator_disable(touch_regulator);
        #endif
            gpio_request(TOUCH_EN,"Touch_en");
		gpio_direction_output(TOUCH_EN,0);
		gpio_set_value(TOUCH_EN,0);
		gpio_free(TOUCH_EN);
	}
}
EXPORT_SYMBOL(touch_ctrl_regulator);

int tsp_reset( void )
{
	int ret=1;

	printk("[TSP] %s+\n", __func__ );

	touch_ctrl_regulator(0);

	//PSJ gpio_direction_output(30, 0);
      	//PSJ gpio_direction_output(27, 0);
	//PSJ gpio_direction_output(26, 0);
            
	msleep(200);


	// for TSK

      TSP_forced_release_forkey();

	//PSJ gpio_direction_output(30, 1);
      	//PSJ gpio_direction_output(27, 1);
	//PSJ gpio_direction_output(26, 1);

	//PSJ gpio_direction_input(30);
      //PSJ 	gpio_direction_input(27);
	//PSJ gpio_direction_input(26);

	touch_ctrl_regulator(1);
		
	msleep(200);
    
	return ret;
}


int tsp_i2c_write (unsigned char *rbuf, int num)
{
    int ret;
    ret = i2c_master_send(ts_global->client, rbuf, num);
    
       if(ret<0) {
		printk("[TSP] Error code : %d, %d\n", __LINE__, ret );
	}

    return ret;
}
EXPORT_SYMBOL(tsp_i2c_write);

int tsp_i2c_read(unsigned char *rbuf, int len)
{
    int ret;
    
	ret = i2c_master_recv(ts_global->client, rbuf, len);

       if(ret<0) {
		printk("[TSP] Error code : %d, %d\n", __LINE__, ret );
	}

       return ret;
}
EXPORT_SYMBOL(tsp_i2c_read);


void TSP_forced_release_forkey(void)
{
	int i, key;
	int temp_value=0;
    
	for(i=0; i<SILABS_MAX_TOUCH; i++)
		{
			if(g_Mtouch_info[i].strength== -1)
				continue;

			input_report_abs(ts_global->input_dev, ABS_MT_TRACKING_ID, i);
			input_report_abs(ts_global->input_dev, ABS_MT_POSITION_X, g_Mtouch_info[i].posX);
		    input_report_abs(ts_global->input_dev, ABS_MT_POSITION_Y, g_Mtouch_info[i].posY);
		    input_report_abs(ts_global->input_dev, ABS_MT_TOUCH_MAJOR, 0 );
			input_report_abs(ts_global->input_dev, ABS_MT_WIDTH_MAJOR, g_Mtouch_info[i].width);
            //PSJ input_report_key(ts_global->input_dev, BTN_TOUCH, 0);
			input_mt_sync(ts_global->input_dev);   

                   printk("[TSP] force release\n");
 
			if(g_Mtouch_info[i].strength == 0)
				g_Mtouch_info[i].strength = -1;

            		temp_value++;
		}

	if(temp_value>0)
		input_sync(ts_global->input_dev);

    
	for(key = 0; key < MAX_KEYS ; key++)
	{
          touchkey_status[key].key_press = RELEASE_KEY;
	    input_report_key(ts_global->input_dev, touchkey_status[key].key_value, touchkey_status[key].key_press);	
	}
	
}
EXPORT_SYMBOL(TSP_forced_release_forkey);

static irqreturn_t  silabs_ts_work_func(int irq, void *dev_id)
{
	int ret=0, i=0, key=0;
	uint8_t buf[TS_READ_REGS_LEN];
	int touch_num=0, button_num =0, button_status=0, button_check=0;

	struct silabs_ts_data *ts = dev_id;

      #ifdef __TOUCH_DEBUG__
	  printk("[TSP] %s, %d\n", __func__, __LINE__ );
      #endif

	if(ts ==NULL)
	{
       printk("[TSP] silabs_ts_work_func : TS NULL\n");
	   return IRQ_HANDLED;
	}
	
	buf[0] = ESCAPE_ADDR;
	buf[1] = 0x02;

	for(i=0; i<I2C_RETRY_CNT; i++)
	{	
		ret = i2c_master_send(ts->client, buf, 2);

		if(ret >=0)
		{
			ret = i2c_master_recv(ts->client, buf, TS_READ_REGS_LEN);

			if(ret >=0)
			{
				break; // i2c success
			}
		}
	}
        
	if (ret < 0)
	{
             printk("[TSP] silabs_ts_work_func: i2c failed %d\n", ret);
             tsp_reset();
             return IRQ_HANDLED;
	}
	else 
	{

            touch_num  = buf[0]&0x0F;
	      	button_num = ((buf[0]&0xC0)>>6);
            button_status=((buf[1]&0x10)>>4);
            button_check=buf[1]&0x0F;

            #ifdef __TOUCH_DEBUG__
            printk("[TSP] button_num : %d, touch_num : %d, button_check:%d, buf[1] : %d\n", button_num, touch_num, button_check, buf[1]);
            #endif
        
        	if(button_check == 0)
			{
                   if(touch_num >0) 
                   {
                  	touch_id[0] = (buf[2]&0xf0)>>4;
        			posX[0] = (( buf[3]<< (8) ) +  buf[4]);
        			posY[0] = ( buf[5]<< (8) ) +  buf[6];

        			strength[0] = buf[7]; 

                    touch_id[1] = (buf[2]&0x0f);
        			posX[1] =  (( buf[8]<< (8) ) +  buf[9]);
        			posY[1] = ( buf[10]<< (8) ) +  buf[11];

        			strength[1] = buf[12]; 
                   }
                   
                    if(touch_num==0)
                    {
                     touch_id[0]=0;
                     touch_id[1]=0;
                     strength[0]=0;
                     strength[1]=0;
                    }

			for(i=0; i<2; i++)
			{
				if(touch_id[i] >=1)
				{
					g_Mtouch_info[i].status = 1;
				}
				else if(touch_id[i] ==0 && g_Mtouch_info[i].status == 1)
				{
					g_Mtouch_info[i].status = 0;
				}
				else if(touch_id[i] ==0 && g_Mtouch_info[i].status == 0)
				{
					g_Mtouch_info[i].status = -1;
				}
				
				if(g_Mtouch_info[i].status == -1) continue;

				input_report_abs(ts->input_dev, ABS_MT_POSITION_X, posX[i]);
			    input_report_abs(ts->input_dev, ABS_MT_POSITION_Y,  posY[i]);
				input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, touch_id[i] );
				input_report_abs(ts->input_dev, ABS_MT_WIDTH_MAJOR, strength[i]);      				
				input_mt_sync(ts->input_dev);
                        input_report_key(ts->input_dev, BTN_TOUCH, strength[i]);
                
                g_Mtouch_info[i].posX = posX[i];
                g_Mtouch_info[i].posY = posY[i];
                g_Mtouch_info[i].width = strength[i];
                g_Mtouch_info[i].strength = strength[i];
      
            #ifdef __TOUCH_DEBUG__
             printk("[TSP] i : %d, x: %d, y: %d\n, 3:%d, 4:%d", touch_id[i], posX[i], posY[i], strength[i], strength[i]);
			#endif			
			}
		}
        else
		{
                                      
			if (buf[1] & 0x1)
                        key=0;
			if (buf[1] & 0x2)
                        key=1; 
            
                   touchkey_status[0].key_value= KEY_MENU;	
                   touchkey_status[1].key_value= KEY_BACK;	
                   touchkey_status[key].key_press=(button_status ? PRESS_KEY : RELEASE_KEY);
                   
            		input_report_key(ts->input_dev, touchkey_status[key].key_value, touchkey_status[key].key_press);
                     input_report_key(ts->input_dev, BTN_TOUCH, 1);
                    
            #ifdef __TOUCH_DEBUG__
            	printk(KERN_ERR "[TSP] silabs_ts_work_func: buf[%d] : %d, button_status: %d\n", key, touchkey_status[key].key_value, touchkey_status[key].key_press);
		#endif		
		}
   	input_sync(ts->input_dev);              
	}

    return IRQ_HANDLED;
}

static irqreturn_t silabs_ts_irq_handler(int irq, void *dev_id)
{
      touch_check=1;

       #ifdef __TOUCH_DEBUG__
		printk("[TSP] %s, %d\n", __func__, __LINE__ );
       #endif
       
	return IRQ_WAKE_THREAD;
}

void set_tsp_for_ta_detect(int state)
{
	
	int ret=0;
	uint8_t wdog_val[7] = {0x80, 0x05, 0x00, 0x02, 0x12, 0x20, 0x00};
    if((tsp_status==0)&&(testmode==1))
    {
	if(state)
	{
		printk("[TSP] [1] set_tsp_for_ta_detect!!! state=1\n");
        
		wdog_val[5] = 0x20;
		ret = i2c_master_send(ts_global->client, (const char *)&wdog_val, 7);
    
        if(ret<0) {
		printk("[TSP] Error code : %d, %d\n", __LINE__, ret );
		}

		pre_ta_stat = 1;
	}
	else
	{
		printk("[TSP] [2] set_tsp_for_ta_detect!!! state=0\n");
        
		wdog_val[5] = 0x00;
		ret = i2c_master_send(ts_global->client, (const char *)&wdog_val, 7);
            
        if(ret<0) {
		printk("[TSP] Error code : %d, %d\n", __LINE__, ret );
	    }
		pre_ta_stat = 0;
	}
	}
} 
EXPORT_SYMBOL(set_tsp_for_ta_detect);

static void check_ic_work_func(struct work_struct *work_timer)
{
	int ret=0;
      uint8_t buf_esd[2];

	uint8_t wdog_val[1];

	struct silabs_ts_data *ts = container_of(work_timer, struct silabs_ts_data, work_timer);

	//printk("[TSP] testmode : %d\n", testmode);

    buf_esd[0] = ESCAPE_ADDR;
	buf_esd[1] = TS_READ_ESD_ADDR;

	wdog_val[0] = 1;
    touch_check=0;
	if(testmode==1)
	{
        #if 0 //PSJ
 		if( pre_ta_stat != tsp_charger_type_status )
		{
			set_tsp_for_ta_detect(tsp_charger_type_status);
		}
#endif
		ret = i2c_master_send(ts->client, (const char *)&buf_esd, 2);
            
		if(ret >=0)
		{
			ret = i2c_master_recv(ts->client, wdog_val, 1);
		}
		
        if (touch_check==0)
        {
	       if(ret < 0)
	        {
                 tsp_reset();
		    	 printk(KERN_ERR "silabs_ts_work_func : i2c_master_send [%d]\n", ret);			
	        }

		else if(wdog_val[0] == (uint8_t)prev_wdog_val)
		{
			    printk("[TSP] %s tsp_reset counter = %x, prev = %x\n", __func__, wdog_val[0], (uint8_t)prev_wdog_val);
			    tsp_reset();
			    prev_wdog_val = -1;
		}

		else
		{
			prev_wdog_val = wdog_val[0];
		}
        }
	}
}

static enum hrtimer_restart silabs_watchdog_timer_func(struct hrtimer *timer)
{
	queue_work(check_ic_wq, &ts_global->work_timer);
	hrtimer_start(&ts_global->timer, ktime_set(1, 0), HRTIMER_MODE_REL);

	return HRTIMER_NORESTART;
}

static unsigned int touch_present = 0;

static int silabs_ts_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct silabs_ts_data *ts;
	int ret = 0; 

	touch_ctrl_regulator(TOUCH_ON);
    mdelay(200);  
	touch_ctrl_regulator(TOUCH_OFF);
    mdelay(200);      
	touch_ctrl_regulator(TOUCH_ON);
    mdelay(200);

	printk("[TSP] %s, %d\n", __func__, __LINE__ );
     
	ts = kzalloc(sizeof(*ts), GFP_KERNEL);
	if (ts == NULL) {
		ret = -ENOMEM;
		goto err_alloc_data_failed;
	}

    INIT_WORK(&ts->work_timer, check_ic_work_func );
	ts->client = client;
	i2c_set_clientdata(client, ts);

	ts_global = ts;

    tsp_irq=client->irq;

	hrtimer_init(&ts->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	ts->timer.function = silabs_watchdog_timer_func;

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
    	if (device_create_file(firmware_dev, &dev_attr_versname) < 0)
		pr_err("Failed to create device file(%s)!\n", dev_attr_versname.attr.name);
      if (device_create_file(firmware_dev, &dev_attr_raw) < 0)
		pr_err("Failed to create device file(%s)!\n", dev_attr_raw.attr.name);
     if (device_create_file(firmware_dev, &dev_attr_reference) < 0)
		pr_err("Failed to create device file(%s)!\n", dev_attr_reference.attr.name);
     if (device_create_file(firmware_dev, &dev_attr_diff) < 0)
		pr_err("Failed to create device file(%s)!\n", dev_attr_diff.attr.name);		
     if (device_create_file(firmware_dev, &dev_attr_raw_value) < 0)
		pr_err("Failed to create device file(%s)!\n", dev_attr_raw_value.attr.name);
     if (device_create_file(firmware_dev, &dev_attr_node_read) < 0)
		pr_err("Failed to create device file(%s)!\n", dev_attr_node_read.attr.name);	
     if (device_create_file(firmware_dev, &dev_attr_raw_enable) < 0)
		pr_err("Failed to create device file(%s)!\n", dev_attr_raw_enable.attr.name);
     if (device_create_file(firmware_dev, &dev_attr_raw_disable) < 0)
		pr_err("Failed to create device file(%s)!\n", dev_attr_raw_disable.attr.name);
	/* sys fs */

    ts->input_dev = input_allocate_device();
	if (ts->input_dev == NULL) {
		ret = -ENOMEM;
      	touch_present = 0;
		printk(KERN_ERR "silabs_ts_probe: Failed to allocate input device\n");
		goto err_input_dev_alloc_failed;
	}
	
	ts->input_dev->name = "sec_touchscreen ";
    
	ts->input_dev->evbit[0] = BIT_MASK(EV_ABS) | BIT_MASK(EV_KEY);
	
	ts->input_dev->keybit[BIT_WORD(KEY_MENU)] |= BIT_MASK(KEY_MENU);
	ts->input_dev->keybit[BIT_WORD(KEY_POWER)] |= BIT_MASK(KEY_POWER);
	ts->input_dev->keybit[BIT_WORD(KEY_BACK)] |= BIT_MASK(KEY_BACK);		
	ts->input_dev->keybit[BIT_WORD(KEY_SEARCH)] |= BIT_MASK(KEY_SEARCH);	

	set_bit(BTN_TOUCH, ts->input_dev->keybit);
	set_bit(EV_ABS,  ts->input_dev->evbit);

	ts->input_dev->evbit[0] =  BIT_MASK(EV_SYN) | BIT_MASK(EV_ABS) | BIT_MASK(EV_KEY);	

	input_set_abs_params(ts->input_dev, ABS_MT_POSITION_X, 0, MAX_X, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_POSITION_Y, 0, MAX_Y, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0, 255, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_WIDTH_MAJOR, 0, 255, 0, 0);

	set_bit(EV_SYN, ts->input_dev->evbit); 
	set_bit(EV_KEY, ts->input_dev->evbit);
       set_bit(INPUT_PROP_DIRECT, ts->input_dev->propbit);
     
	/* ts->input_dev->name = ts->keypad_info->name; */
	ret = input_register_device(ts->input_dev);
	if (ret) {
        touch_present = 0;
		printk(KERN_ERR "silabs_ts_probe: Unable to register %s input device\n", ts->input_dev->name);
		goto err_input_register_device_failed;
	}

    printk("[TSP] %s, irq=%d\n", __func__, tsp_irq );

    gpio_request(TSP_INT, "ts_irq");
	gpio_direction_input(TSP_INT);
   

    if (tsp_irq) {
		ret = request_threaded_irq(tsp_irq, silabs_ts_irq_handler,  silabs_ts_work_func, IRQF_TRIGGER_FALLING|IRQF_ONESHOT, client->name, ts);

		if (ret < 0)
		  dev_err(&client->dev, "request_irq failed, ret:%d \n", ret);
	}

#if 1
#ifdef CONFIG_HAS_EARLYSUSPEND
	ts->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN+1;
	ts->early_suspend.suspend = silabs_ts_early_suspend;
	ts->early_suspend.resume = silabs_ts_late_resume;
	register_early_suspend(&ts->early_suspend);
#endif
#endif

	if(ts ==NULL)
	{
       printk("[TSP] silabs_ts_init_read : TS NULL\n");
	   goto err_input_dev_alloc_failed;
	}

	touch_present = 1;

	buf_firmware[0] = ESCAPE_ADDR;
	buf_firmware[1] = TS_READ_VERSION_ADDR;
	ret = i2c_master_send(ts->client,(const char *) &buf_firmware, 2);
	if(ret < 0)
	{
		printk(KERN_ERR "silabs_ts_work_func : i2c_master_send [%d]\n", ret);
        tsp_chheck=1;
	}

	if(tsp_chheck==0)   
	{

	  ret = i2c_master_recv(ts->client, (char *)&buf_firmware, 3);
	if(ret < 0)
	{
		printk(KERN_ERR "silabs_ts_work_func : i2c_master_recv [%d]\n", ret);	
        tsp_chheck=1;
	}
	printk("[TSP] silabs_ts_probe %d, %d, %d\n", buf_firmware[0], buf_firmware[1], buf_firmware[2]);
#if defined(CONFIG_TARGET_LOCALE_AUS_TEL)
    if (( buf_firmware[2] == T_YTE_MODULE_VER_NEW)&&(buf_firmware[0] < T_FW_VER_NEW_YTE))
    { 
        TSP_MODULE_ID =  buf_firmware[2];
        PHONE_VER = T_FW_VER_NEW_YTE;
	    local_irq_disable();
		ret = Firmware_Download();	
        printk("[TSP] enable_irq : %d\n", __LINE__ );
        printk("[TSP] totoroT taeyang 0x08 update : %d\n", __LINE__ );
	    local_irq_enable();

		if(ret == 0)
		{
			printk(KERN_ERR "SET Download Fail - error code [%d]\n", ret);			
		}
	} 
	else if(( buf_firmware[2] == T_SMAC_MODULE_VER)&&((buf_firmware[0] < T_SMAC_FW_VER)))
    { 
        TSP_MODULE_ID =  buf_firmware[2];
        PHONE_VER = T_SMAC_FW_VER;
	    local_irq_disable();
		ret = Firmware_Download();	
        printk("[TSP] enable_irq : %d\n", __LINE__ );
        printk("[TSP] totoroT SMAC 0x14 update : %d\n", __LINE__ );
	    local_irq_enable();

		if(ret == 0)
		{
			printk(KERN_ERR "SET Download Fail - error code [%d]\n", ret);			
		}
	} 
	else	
	
#endif
    if ((( buf_firmware[2] == YTE_MODULE_VER)||( buf_firmware[2] == SMAC_MODULE_VER))&&(buf_firmware[0] < FW_VER))
    { 
        TSP_MODULE_ID =  buf_firmware[2];
        PHONE_VER = FW_VER;
	    local_irq_disable();
		//ret = Firmware_Download();	
        printk("[TSP] enable_irq : %d\n", __LINE__ );
	    local_irq_enable();

		if(ret == 0)
		{
			printk(KERN_ERR "SET Download Fail - error code [%d]\n", ret);			
		}
	}
    else if (( buf_firmware[2] == YTE_MODULE_VER_OLD)&&(buf_firmware[0] < FW_VER_OLD))
    { 
        TSP_MODULE_ID =  buf_firmware[2];
        PHONE_VER = FW_VER_OLD;
	    local_irq_disable();
		ret = Firmware_Download();	
        printk("[TSP] enable_irq : %d\n", __LINE__ );
	    local_irq_enable();

		if(ret == 0)
		{
			printk(KERN_ERR "SET Download Fail - error code [%d]\n", ret);			
		}
	}
	else if (( buf_firmware[2] == SMAC_MODULE_VER_OLD)&&(buf_firmware[0] < FW_VER_OLD_SMAC))
    { 
        TSP_MODULE_ID =  buf_firmware[2];
        PHONE_VER = FW_VER_OLD_SMAC;
	    local_irq_disable();
		ret = Firmware_Download();	
        printk("[TSP] enable_irq : %d\n", __LINE__ );
	    local_irq_enable();

		if(ret == 0)
		{
			printk(KERN_ERR "SET Download Fail - error code [%d]\n", ret);			
		}
	}
    else if (( buf_firmware[2] == YTE_MODULE_VER_NEW)&&(buf_firmware[0] < FW_VER_NEW_YTE))
    { 
         TSP_MODULE_ID =  buf_firmware[2];
         PHONE_VER = FW_VER_NEW_YTE;
	     local_irq_disable();
		 ret = Firmware_Download();	
         printk("[TSP] enable_irq : %d\n", __LINE__ );
	     local_irq_enable();

		if(ret == 0)
		{
			printk(KERN_ERR "SET Download Fail - error code [%d]\n", ret);			
		}
	}
    else if (( buf_firmware[2] == SMAC_MODULE_VER_NEW)&&(buf_firmware[0] < FW_VER_NEW_SMAC))
    { 
         TSP_MODULE_ID =  buf_firmware[2];
         PHONE_VER = FW_VER_NEW_SMAC;
	     local_irq_disable();
		 ret = Firmware_Download();	
         printk("[TSP] enable_irq : %d\n", __LINE__ );
	     local_irq_enable();

		if(ret == 0)
		{
			printk(KERN_ERR "SET Download Fail - error code [%d]\n", ret);			
		}
	}

	else if (( buf_firmware[2] == 0xFF) || (ret<0))
    { 
        TSP_MODULE_ID =  buf_firmware[2];
	    local_irq_disable();
		ret = Firmware_Download();	
        printk("[TSP] enable_irq : %d\n", __LINE__ );
	    local_irq_enable();

		if(ret == 0)
		{
			printk(KERN_ERR "SET Download Fail - error code [%d]\n", ret);			
		}
	}	

       hrtimer_start(&ts->timer, ktime_set(2, 0), HRTIMER_MODE_REL);
}

	return 0;

err_input_register_device_failed:
	printk(KERN_ERR "silabs-ts: err_input_register_device failed\n");
	input_free_device(ts->input_dev);
err_input_dev_alloc_failed:
	printk(KERN_ERR "silabs-ts: err_input_dev_alloc failed\n");
err_alloc_data_failed:
	printk(KERN_ERR "silabs-ts: err_alloc_data failed_\n");	
	return 1;
}

static int silabs_ts_remove(struct i2c_client *client)
{
	struct silabs_ts_data *ts = i2c_get_clientdata(client);
	unregister_early_suspend(&ts->early_suspend);
	if (tsp_irq)
		free_irq(tsp_irq, ts);
	//else
	//	hrtimer_cancel(&ts->timer);
	input_unregister_device(ts->input_dev);
	kfree(ts);
	return 0;
}

static int silabs_ts_suspend(struct i2c_client *client, pm_message_t mesg)
{
    	int ret;
	struct silabs_ts_data *ts = i2c_get_clientdata(client);

	printk("[TSP] %s, %d\n", __func__, __LINE__ );
        tsp_status=1;
	if( touch_present )
	{
	if (tsp_irq)
	{
		disable_irq(tsp_irq);
	}
        ret = cancel_work_sync(&ts->work_timer);

	    ret = cancel_work_sync(&ts->work);
        	if (ret && tsp_irq) /* if work was pending disable-count is now 2 */
	    {
		    enable_irq(tsp_irq);
	    }
            
        if(tsp_chheck==0)
	    hrtimer_cancel(&ts->timer);

	    touch_ctrl_regulator(TOUCH_OFF);

        //PSJ gpio_direction_output(30, 0);
      	//PSJ gpio_direction_output(27, 0);
	    //PSJ gpio_direction_output(26, 0);

	    msleep(400);    
	}
    	else
		printk("[TSP] TSP isn't present.\n");

        TSP_forced_release_forkey();

	printk("[TSP] %s-\n", __func__ );
	return 0;
}

static int silabs_ts_resume(struct i2c_client *client)
{
	int key;
	struct silabs_ts_data *ts = i2c_get_clientdata(client);

	printk("[TSP] %s, %d\n", __func__, __LINE__ );

      //enable_irq(client->irq); // scl wave

	printk("[TSP] %s+\n", __func__ );
	if( touch_present )
	{       
		//PSJ gpio_direction_output(30, 1);
    	//PSJ gpio_direction_output(27, 1);
		//PSJ gpio_direction_output(26, 1);

		//PSJ gpio_direction_input(30);
    	//PSJ gpio_direction_input(27);
		//PSJ gpio_direction_input(26);

    	touch_ctrl_regulator(TOUCH_ON);
    	msleep(40);

	// for TSK
	for(key = 0; key < MAX_KEYS; key++)
		touchkey_status[key].key_press = RELEASE_KEY;
    
	prev_wdog_val = -1;
#if 0 //PSJ
	if(tsp_charger_type_status == 1)
	{
		set_tsp_for_ta_detect(tsp_charger_type_status);
	}
    #endif
    #if 0	
	if( tsp_proximity_irq_status == 1)
	{
		set_tsp_for_prox_enable(tsp_proximity_irq_status);
	}
    #endif
    
    if(tsp_chheck==0)
	hrtimer_start(&ts->timer, ktime_set(1, 0), HRTIMER_MODE_REL);

	enable_irq(tsp_irq);
	}
	else
		printk("[TSP] TSP isn't present.\n");

	printk("[TSP] %s-\n", __func__ );
        tsp_status=0; 
	return 0;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void silabs_ts_early_suspend(struct early_suspend *h)
{
	struct silabs_ts_data *ts;
	ts = container_of(h, struct silabs_ts_data, early_suspend);
	silabs_ts_suspend(ts->client, PMSG_SUSPEND);
}

static void silabs_ts_late_resume(struct early_suspend *h)
{
	struct silabs_ts_data *ts;
	ts = container_of(h, struct silabs_ts_data, early_suspend);
	silabs_ts_resume(ts->client);
}
#endif

static const struct i2c_device_id silabs_ts_id[] = {
	{ SILABS_TS_NAME, 0 },
	{ }
};

static struct i2c_driver silabs_ts_driver = {
	.probe		= silabs_ts_probe,
	.remove		= silabs_ts_remove,
#if 1
#ifndef CONFIG_HAS_EARLYSUSPEND
	.suspend	= silabs_ts_suspend,
	.resume		= silabs_ts_resume,
#endif
#endif
	.id_table	= silabs_ts_id,
	.driver = {
		.name	= SILABS_TS_NAME,
	},
};

static int __devinit silabs_ts_init(void)
{

    	check_ic_wq = create_singlethread_workqueue("check_ic_wq");	
	if (!check_ic_wq)
		return -ENOMEM;

	touch_regulator = regulator_get(NULL,"touch_vcc");

	//PSJ board_sysconfig(SYSCFG_TOUCH, SYSCFG_INIT);

	return i2c_add_driver(&silabs_ts_driver);
}

static void __exit silabs_ts_exit(void)
{
	if (touch_regulator) 
	{
       	 regulator_put(touch_regulator);
		 touch_regulator = NULL;
    	}
	
	i2c_del_driver(&silabs_ts_driver);

	if (check_ic_wq)
		destroy_workqueue(check_ic_wq);
}


/* Touch Reference ************************************************************/
static ssize_t raw_enable_silabs(struct device *dev, struct device_attribute *attr, char *buf)
{

		printk("[TSP] %s stop. line : %d, \n", __func__,__LINE__);

            testmode=0;

    return sprintf(buf, "1") ;
}

static ssize_t raw_disable_silabs(struct device *dev, struct device_attribute *attr, char *buf)
{

		printk("[TSP] %s stop. line : %d, \n", __func__,__LINE__);

            testmode=1;
        	touch_ctrl_regulator(0);  
        	mdelay(2);
        	touch_ctrl_regulator(1);  
        	mdelay(300);


    return sprintf(buf, "1") ;
}

int silabs_quicksense ( int address, int size, char* buff )
{
	uint8_t buf1[7]={0x78, 0x05, 0x00, 0, 0, 0, 0};
      int ret;
      
	buf1[3] = (address >> 8) & 0xFF; // Address High Byte
	buf1[4] = address & 0xFF; // Address Low Byte
	buf1[5] = size;

	ret = i2c_master_send(ts_global->client, buf1, 7);
	if (ret < 0)
	{
		printk("[TSP] i2c_master_send fail! %s : %d, \n", __func__,__LINE__);
		return -1;
	}
	
	ret = i2c_master_recv(ts_global->client, buff, size + QUICKSENSE_OVERHEAD);

	if (ret < 0)
	{
		printk("[TSP] i2c_master_recv fail! %s : %d, \n", __func__,__LINE__);
		return -1;
	}

	return 0;
}


static ssize_t rawdata_pass_fail_silabs(struct device *dev, struct device_attribute *attr, char *buf)
{

	int Rx_Channel = NUM_RX_CHANNEL;
	uint8_t buffer1[NUM_TX_CHANNEL*NUM_RX_CHANNEL*2+QUICKSENSE_OVERHEAD]={0,};
    uint8_t buffer2[NUM_TX_CHANNEL*NUM_RX_CHANNEL*2+QUICKSENSE_OVERHEAD]={0,};
	uint16_t rawdata[NUM_TX_CHANNEL][NUM_RX_CHANNEL]={{0,},};
#if defined(CONFIG_TARGET_LOCALE_AUS_TEL)   
	uint16_t TYE_RAWDATA_MAX[108] = {14250,14370,14370,14390,14320,14290,14265,14175,13915,13515,13660,13665,13700,13620,13590,13555,13460,13195,13470,13615,13615,13650,13585,13550,13515,13415,13145,13470,13600,13590,13625,13565,13530,13500,13405,13130,13455,13590,13600,13630,13555,13515,13485,13300,13110,13410,13555,13590,13620,13520,13485,13460,13360,13080,13245,13490,13555,13600,13505,13490,13475,13415,13410,13230,13470,13540,13600,13520,13480,13460,13400,13280,13215,13445,13505,13565,13500,13460,13430,13375,13235,13215,13430,13480,13535,13480,13455,13430,13350,13195,13380,13415,13445,13495,13440,13415,13395,13320,13160,13935,14110,14130,14190,14110,14090,14060,14000,13825};
	uint16_t TYE_RAWDATA_MIN[108] = {10750,10840,10840,10860,10800,10780,10760,10695,10500,10195,10305,10310,10335,10275,10250,10225,10155, 9955,10165,10270,10270,10295,10250,10220,10195,10120, 9920,10160,10260,10255,10280,10235,10210,10185,10110, 9905,10150,10250,10260,10285,10225,10195,10175,10100, 9890,10120,10225,10255,10280,10200,10175,10155,10080, 9870, 9990,10175,10225,10260,10190,10175,10165,10120,10115, 9980,10160,10215,10260,10200,10170,10155,10110,10020, 9970,10140,10190,10235,10185,10155,10130,10090, 9985, 9970,10130,10170,10210,10170,10150,10130,10075, 9955,10095,10120,10145,10180,10140,10120,10105,10050, 9930,10510,10645,10660,10705,10645,10630,10610,10560,10430};

	uint16_t SMAC_RAWDATA_MAX[108] = {15075,15183,15165,15194,15106,15077,15054,14971,14698,14692,14844,14859,14902,14801,14755,14720,14627,14346,14668,14814,14828,14880,14792,14741,14700,14606,14327,14660,14788,14792,14847,14768,14713,14667,14571,14298,14622,14749,14771,14821,14733,14682,14637,14539,14260,14566,14704,14737,14784,14686,14644,14608,14518,14240,14415,14654,14700,14755,14658,14636,14612,14558,14606,14401,14638,14686,14745,14653,14613,14590,14537,14453,14387,14628,14675,14735,14644,14589,14575,14527,14403,14367,14603,14650,14698,14619,14568,14557,14493,14339,14536,14583,14619,14668,14599,14549,14533,14459,14298,14799,14982,15011,15075,15003,14955,14917,14852,14696 };
	uint16_t SMAC_RAWDATA_MIN[108] = {11143,11223,11209,11230,11166,11144,11127,11065,10864,10860,10972,10983,11014,10940,10906,10880,10811,10604,10842,10950,10960,10998,10934,10895,10866,10796,10589,10836,10930,10934,10974,10916,10875,10841,10770,10568,10808,10901,10917,10955,10889,10852,10819,10747,10540,10766,10868,10893,10928,10855,10824,10798,10730,10526,10655,10832,10866,10906,10834,10818,10800,10760,10796,10645,10820,10855,10899,10831,10801,10784,10745,10683,10634,10812,10847,10891,10824,10783,10773,10737,10645,10619,10793,10828,10864,10805,10768,10759,10713,10599,10744,10779,10805,10842,10791,10753,10741,10687,10568,10939,11074,11095,11143,11089,11053,11025,10978,10862};

#else
	uint16_t RAWDATA_MAX[108] = {14350,14510,14560,14610,14710,14680,14780,14780,14440,13720,13990,14080,14150,14190,14190,14260,14250,13910,13650,13940,14040,14110,14150,14150,14210,14200,13870,13610,13890,14000,14080,14120,14120,14200,14170,13840,13570,13870,13960,14040,14080,14080,14160,14150,13820,13560,13830,13920,13990,14020,14020,14110,14120,13790,13540,13800,13900,13960,14000,14000,14090,14100,13770,13400,13760,13900,13980,14030,14040,14150,14200,14270,13380,13750,13890,13980,14020,14040,14140,14160,14050,13370,13730,13870,13960,14010,14010,14100,14130,13980,13440,13720,13860,13960,14000,14000,14100,14130,14180,14150,14560,14730,14830,14870,14870,14960,15000,15010};
	uint16_t RAWDATA_MIN[108] = {10610,10720,10770,10800,10870,10850,10930,10920,10680,10140,10340,10410,10460,10490,10480,10540,10530,10280,10090,10300,10370,10430,10460,10460,10500,10500,10250,10060,10270,10340,10410,10440,10440,10490,10470,10230,10030,10250,10320,10380,10400,10410,10460,10460,10210,10020,10220,10290,10340,10360,10360,10430,10440,10190,10010,10200,10270,10320,10350,10350,10410,10420,10180,9900,10170,10270,10330,10370,10380,10460,10500,10540,9890,10160,10270,10330,10360,10370,10450,10470,10390,9880,10150,10250,10320,10350,10360,10420,10440,10330,9930,10140,10250,10310,10350,10350,10420,10450,10480,10460,10760,10880,10960,10990,10990,11060,11090,11100};
#endif  

	uint8_t buf_firmware_show[3];

    int i, j, ret;

    printk("[TSP] %s entered. line : %d, \n", __func__,__LINE__);

	if(testmode==1) return sprintf(buf, "-1");   

#if defined(CONFIG_TARGET_LOCALE_AUS_TEL)

	printk("[TSP] rawdata_pass_fail_silabs at AU Version Read Test \n");

	buf_firmware_show[0] = ESCAPE_ADDR;
	buf_firmware_show[1] = TS_READ_VERSION_ADDR;
	ret = i2c_master_send(ts_global->client, &buf_firmware_show, 2);
	if(ret < 0)
	{
		printk(KERN_ERR "silabs_ts_work_func : i2c_master_send [%d]\n", ret);			
	}

	ret = i2c_master_recv(ts_global->client, &buf_firmware_show, 3);
	if(ret < 0)
	{
		printk(KERN_ERR "silabs_ts_work_func : i2c_master_recv [%d]\n", ret);			
	}

	printk("[TSP][rawdata_pass_fail_silabs] ver tsp=0x%x, HW=0x%x, SW=0x%x\n", buf_firmware_show[1], buf_firmware_show[2], buf_firmware_show[0]);

	if(buf_firmware_show[2] == T_YTE_MODULE_VER_NEW)	//TYE Module
		TSP_MODULE_ID = T_YTE_MODULE_VER_NEW;

	else if(buf_firmware_show[2] == T_SMAC_MODULE_VER)	//TYE Module
		TSP_MODULE_ID = T_SMAC_MODULE_VER;

	printk("[TSP][rawdata_pass_fail_silabs] TSP_MODULE_ID = 0x%x \n", TSP_MODULE_ID);	
#endif

	mdelay(300); 

	buffer1[0] = ESCAPE_ADDR;
	buffer1[1] = JIG_MODE_COMMAND;
	ret = i2c_master_send(ts_global->client, buffer1, 2);
	if (ret < 0)
	{
		printk("[TSP] i2c_master_send fail! %s : %d, \n", __func__,__LINE__);
		return sprintf(buf, "-1");
	}

	ret = i2c_master_recv(ts_global->client,buffer2, 1);

	if (ret < 0)
	{
		printk("[TSP] i2c_master_recv fail! %s : %d, \n", __func__,__LINE__);
		return sprintf(buf, "-1");
	}

	//
	//	quicksense format for reading rawdata
	//
#if defined(CONFIG_TARGET_LOCALE_AUS_TEL)   
	ret = silabs_quicksense(RAWDATA_ADDRESS,NUM_TX_CHANNEL*NUM_RX_CHANNEL*2, buffer2);
#else
	ret = silabs_quicksense(RAWDATA_ADDRESS,NUM_RX_CHANNEL*2, buffer2);
#endif
	if (ret != 0)
	{
		printk("[TSP] silabs_quicksense fail! %s : %d, \n", __func__,__LINE__);
		return sprintf(buf, "-1");
	}

	#if defined(CONFIG_TARGET_LOCALE_AUS_TEL)   
	for (i = 0; i < Tx_Channel; i++)
	{
		for(j = 0 ; j < Rx_Channel; j++)
		{
			rawdata[i][j] = (buffer2[(i*Rx_Channel+j)*2 + QUICKSENSE_OVERHEAD -1] <<8) + buffer2[(i*Rx_Channel+j)*2+QUICKSENSE_OVERHEAD];


			if(TSP_MODULE_ID == T_YTE_MODULE_VER_NEW)
			{
				if( TYE_RAWDATA_MAX[i*Rx_Channel+j] < rawdata[i][j])
				{
					printk("[TSP] rawdata_pass_fail_silabs MAX rawdata[%d][%d] = %d \n", i,j,rawdata[i][j]);			
					return sprintf(buf, "0"); // fail
				}

				if( TYE_RAWDATA_MIN[i*Rx_Channel+j] > rawdata[i][j])
				{
					printk("[TSP] rawdata_pass_fail_silabs MIN rawdata[%d][%d] = %d \n", i,j,rawdata[i][j]);			
					return sprintf(buf, "0"); // fail
				}
			}
			else if(TSP_MODULE_ID == T_SMAC_MODULE_VER)
			{
				if( SMAC_RAWDATA_MAX[i*Rx_Channel+j] < rawdata[i][j])
				{
					printk("[TSP] rawdata_pass_fail_silabs MAX rawdata[%d][%d] = %d \n", i,j,rawdata[i][j]);			
					return sprintf(buf, "0"); // fail
				}

				if( SMAC_RAWDATA_MIN[i*Rx_Channel+j] > rawdata[i][j])
				{
					printk("[TSP] rawdata_pass_fail_silabs MIN rawdata[%d][%d] = %d \n", i,j,rawdata[i][j]);			
					return sprintf(buf, "0"); // fail
				}
			}
		}
	}
	#else
	for (i = 0; i < 1; i++)
	{
		for(j = 0 ; j < Rx_Channel; j++)
		{
			rawdata[i][j] = (buffer2[(i*Rx_Channel+j)*2 + QUICKSENSE_OVERHEAD -1] <<8) + buffer2[(i*Rx_Channel+j)*2+QUICKSENSE_OVERHEAD];

			if( RAWDATA_MAX[i*Rx_Channel+j] < rawdata[i][j])
			{
				return sprintf(buf, "0"); // fail
			}
			if( RAWDATA_MIN[i*Rx_Channel+j] > rawdata[i][j])
			{
				return sprintf(buf, "0"); // fail
			}
		}
	}
	#endif

	buf_firmware_show[0] = ESCAPE_ADDR;
	buf_firmware_show[1] = TS_READ_VERSION_ADDR;
	ret = i2c_master_send(ts_global->client, (const char *)&buf_firmware_show, 2);
	if(ret < 0)
	{
		printk(KERN_ERR "silabs_ts_work_func : i2c_master_send [%d]\n", ret);			
	}

	ret = i2c_master_recv(ts_global->client, (char *)&buf_firmware_show, 3);
	if(ret < 0)
	{
		printk(KERN_ERR "silabs_ts_work_func : i2c_master_recv [%d]\n", ret);			
	}
	printk("[TSP] ver tsp=%x, HW=%x, SW=%x\n", buf_firmware_show[1], buf_firmware_show[2], buf_firmware_show[0]);
#if defined(CONFIG_TARGET_LOCALE_AUS_TEL)
	if (buf_firmware_show[2] == T_YTE_MODULE_VER_NEW)
		PHONE_VER = T_FW_VER_NEW_YTE;
	
	else if (buf_firmware_show[2] == T_SMAC_MODULE_VER)
		PHONE_VER = T_SMAC_FW_VER;
	
	else
#endif

    if (( buf_firmware_show[2] == YTE_MODULE_VER)||( buf_firmware_show[2] == SMAC_MODULE_VER))
             PHONE_VER = FW_VER;

    else if ( buf_firmware_show[2] == YTE_MODULE_VER_OLD)
             PHONE_VER = FW_VER_OLD;

    else if ( buf_firmware_show[2] == SMAC_MODULE_VER_OLD)
			PHONE_VER = FW_VER_OLD_SMAC;

    else if ( buf_firmware_show[2] == YTE_MODULE_VER_NEW)
            PHONE_VER = FW_VER_NEW_YTE;

    else if (buf_firmware_show[2] == SMAC_MODULE_VER_NEW)
			PHONE_VER = FW_VER_NEW_SMAC;

	printk("[TSP][rawdata_pass_fail_silabs] PHONE_VER = %x \n", PHONE_VER);	

	if(buf_firmware_show[0]!=PHONE_VER)
		return sprintf(buf, "0");

    return sprintf(buf, "1"); // success
 }

static ssize_t rawdata_show_silabs(struct device *dev, struct device_attribute *attr, char *buf)
{
	int Tx_Channel = NUM_TX_CHANNEL;
	int Rx_Channel = NUM_RX_CHANNEL;
      int  written_bytes = 0 ;  /* & error check */
	uint8_t buffer1[NUM_TX_CHANNEL*NUM_RX_CHANNEL*2+QUICKSENSE_OVERHEAD]={0,};
    	uint8_t buffer2[NUM_TX_CHANNEL*NUM_RX_CHANNEL*2+QUICKSENSE_OVERHEAD]={0,};
	uint16_t rawdata[NUM_TX_CHANNEL][NUM_RX_CHANNEL]={{0,},};

	uint16_t baseline[NUM_TX_CHANNEL][NUM_RX_CHANNEL]={{0,},};

      int i, j, ret;

      printk("[TSP] %s entered. line : %d, \n", __func__,__LINE__);

	if(testmode==1) return 0;   

	mdelay(300); 

	buffer1[0] = ESCAPE_ADDR;
	buffer1[1] = JIG_MODE_COMMAND;
	ret = i2c_master_send(ts_global->client, buffer1, 2);
	if (ret < 0)
	{
		printk("[TSP] i2c_master_send fail! %s : %d, \n", __func__,__LINE__);
		return -1;
	}

	ret = i2c_master_recv(ts_global->client,buffer2, 1);

	if (ret < 0)
	{
		printk("[TSP] i2c_master_recv fail! %s : %d, \n", __func__,__LINE__);
		return -1;
	}

	//
	//	quicksense format for reading baseline
	//
	ret = silabs_quicksense(BASELINE_ADDRESS,NUM_TX_CHANNEL*NUM_RX_CHANNEL*2, buffer1);
	if (ret != 0)
	{
		printk("[TSP] silabs_quicksense fail! %s : %d, \n", __func__,__LINE__);
		return -1;
	}	
	for (i = 0; i < Tx_Channel; i++)
    {
		for(j = 0 ; j < Rx_Channel; j++)
	{
			baseline[i][j] = (buffer1[(i*Rx_Channel+j)*2 + QUICKSENSE_OVERHEAD -1] <<8) + buffer1[(i*Rx_Channel+j)*2+QUICKSENSE_OVERHEAD];
		}
	}

	//
	//	quicksense format for reading rawdata
	//
	ret = silabs_quicksense(RAWDATA_ADDRESS,NUM_TX_CHANNEL*NUM_RX_CHANNEL*2, buffer2);
	if (ret != 0)
	{
		printk("[TSP] silabs_quicksense fail! %s : %d, \n", __func__,__LINE__);
		return -1;
	}
	for (i = 0; i < Tx_Channel; i++)
    {
		for(j = 0 ; j < Rx_Channel; j++)
        {
			rawdata[i][j] = (buffer2[(i*Rx_Channel+j)*2 + QUICKSENSE_OVERHEAD -1] <<8) + buffer2[(i*Rx_Channel+j)*2+QUICKSENSE_OVERHEAD];
			written_bytes += sprintf(buf+written_bytes, "%d %d\n", rawdata[i][j], baseline[i][j]-rawdata[i][j]) ;
        }
    }

    if (written_bytes > 0)
        return written_bytes ;

    return sprintf(buf, "-1") ;
 }

static ssize_t baseline_show_silabs(struct device *dev, struct device_attribute *attr, char *buf1)
{
	int Tx_Channel = NUM_TX_CHANNEL;
	int Rx_Channel = NUM_RX_CHANNEL;
	int written_bytes = 0 ;  /* & error check */
	uint8_t buffer[NUM_TX_CHANNEL*NUM_RX_CHANNEL*2+QUICKSENSE_OVERHEAD]={0,};
	uint16_t baseline[NUM_TX_CHANNEL][NUM_RX_CHANNEL]={{0,},};

    int i, j, ret;


	if(testmode==1) return 0;
    
    printk("[TSP] %s entered. line : %d, \n", __func__,__LINE__);

   mdelay(300); 

	//
	//	Entering JIG_MODE
	//
	buffer[0] = ESCAPE_ADDR;
	buffer[1] = JIG_MODE_COMMAND;
	ret = i2c_master_send(ts_global->client, buffer, 2);
	if (ret < 0)
	{
		printk("[TSP] i2c_master_send fail! %s : %d, \n", __func__,__LINE__);
		return -1;
	}

	ret = i2c_master_recv(ts_global->client,buffer, 1);

	if (ret < 0)
	{
		printk("[TSP] i2c_master_recv fail! %s : %d, \n", __func__,__LINE__);
		return -1;
	}

	//
	//	quicksense format for reading baseline
	//
	ret = silabs_quicksense(BASELINE_ADDRESS,NUM_TX_CHANNEL*NUM_RX_CHANNEL*2, buffer);
	if (ret != 0)
	{
		printk("[TSP] silabs_quicksense fail! %s : %d, \n", __func__,__LINE__);
		return -1;
	}	

	for (i = 0; i < Tx_Channel; i++)
    {
		for(j = 0 ; j < Rx_Channel; j++)
	{
			baseline[i][j] = (buffer[(i*Rx_Channel+j)*2 + QUICKSENSE_OVERHEAD -1] <<8) + buffer[(i*Rx_Channel+j)*2+QUICKSENSE_OVERHEAD];
			printk(" %5d", baseline[i][j]);
			
			written_bytes += sprintf(buf1+written_bytes, "%d\n", baseline[i][j]) ;
		}
		printk("\n");
	}

	if (written_bytes > 0)
		return written_bytes ;

	return sprintf(buf1, "-1") ;
}

static ssize_t diff_show_silabs(struct device *dev, struct device_attribute *attr, char *buf)
{
	int Tx_Channel = NUM_TX_CHANNEL;
	int Rx_Channel = NUM_RX_CHANNEL;
	uint8_t buffer1[NUM_TX_CHANNEL*NUM_RX_CHANNEL*2+QUICKSENSE_OVERHEAD]={0,};
    uint8_t buffer2[NUM_TX_CHANNEL*NUM_RX_CHANNEL*2+QUICKSENSE_OVERHEAD]={0,};
	uint16_t rawdata[NUM_TX_CHANNEL][NUM_RX_CHANNEL]={{0,},};

	uint16_t baseline[NUM_TX_CHANNEL][NUM_RX_CHANNEL]={{0,},};

      int i, j, ret;

      printk("[TSP] %s entered. line : %d, \n", __func__,__LINE__);

	if(testmode==1) return 0;   

	mdelay(300); 

	buffer1[0] = ESCAPE_ADDR;
	buffer1[1] = JIG_MODE_COMMAND;
	ret = i2c_master_send(ts_global->client, buffer1, 2);
	if (ret < 0)
	{
		printk("[TSP] i2c_master_send fail! %s : %d, \n", __func__,__LINE__);
		return -1;
	}

	ret = i2c_master_recv(ts_global->client, buffer1, 1);

	if (ret < 0)
	{
		printk("[TSP] i2c_master_recv fail! %s : %d, \n", __func__,__LINE__);
		return -1;
	}

	//
	//	quicksense format for reading baseline
	//
	ret = silabs_quicksense(BASELINE_ADDRESS,NUM_TX_CHANNEL*NUM_RX_CHANNEL*2, buffer1);
	if (ret != 0)
	{
		printk("[TSP] silabs_quicksense fail! %s : %d, \n", __func__,__LINE__);
		return -1;
	}	

	for (i = 0; i < Tx_Channel; i++)
    {
		for(j = 0 ; j < Rx_Channel; j++)
	{
			baseline[i][j] = (buffer1[(i*Rx_Channel+j)*2 + QUICKSENSE_OVERHEAD -1] <<8) + buffer1[(i*Rx_Channel+j)*2+QUICKSENSE_OVERHEAD];
			//printk(" %5d", baseline[i][j]);
		}
	}

	//
	//	quicksense format for reading rawdata
	//
	ret = silabs_quicksense(RAWDATA_ADDRESS,NUM_TX_CHANNEL*NUM_RX_CHANNEL*2, buffer2);
	if (ret != 0)
	{
		printk("[TSP] silabs_quicksense fail! %s : %d, \n", __func__,__LINE__);
		return -1;
	}

	for (i = 0; i < Tx_Channel; i++)
    {
		for(j = 0 ; j < Rx_Channel; j++)
        {
			rawdata[i][j] = (buffer2[(i*Rx_Channel+j)*2 + QUICKSENSE_OVERHEAD -1] <<8) + buffer2[(i*Rx_Channel+j)*2+QUICKSENSE_OVERHEAD];
			printk(" %5d", baseline[i][j]-rawdata[i][j]);
        }
	printk("\n");
    }

    return 0;
 }


static ssize_t firmware_show(struct device *dev, struct device_attribute *attr, char *buf)
{
       uint8_t buf_firmware_show[3];
       int ret;

/*
 buf = [1][2][3][4][5][6][7][8] :   [1] - Don't care 
                                    [2][3] - module version 
                                    [4][5] - TSP part vesion
                                    [6][7] - TSP part vesion
*/
	printk("[TSP] %s\n",__func__);

	buf_firmware_show[0] = ESCAPE_ADDR;
	buf_firmware_show[1] = TS_READ_VERSION_ADDR;
	ret = i2c_master_send(ts_global->client,(const char *) &buf_firmware_show, 2);
	if(ret < 0)
	{
		printk(KERN_ERR "silabs_ts_work_func : i2c_master_send [%d]\n", ret);			
	}

	ret = i2c_master_recv(ts_global->client, (char *)&buf_firmware_show, 3);
	if(ret < 0)
	{
		printk(KERN_ERR "silabs_ts_work_func : i2c_master_recv [%d]\n", ret);			
	}
	printk("[TSP] ver tsp=%x, HW=%x, SW=%x\n", buf_firmware_show[1], buf_firmware_show[2], buf_firmware_show[0]);

    if (( buf_firmware_show[2] == YTE_MODULE_VER)||( buf_firmware_show[2] == SMAC_MODULE_VER))
    {
        PHONE_VER = FW_VER;
    }
    else if ( buf_firmware_show[2] == YTE_MODULE_VER_OLD)
    {
        PHONE_VER = FW_VER_OLD;
    }
    else if ( buf_firmware_show[2] == SMAC_MODULE_VER_OLD)
    {
        PHONE_VER = FW_VER_OLD_SMAC;
        sprintf(buf, "10%x%x%x\n", buf_firmware_show[2], buf_firmware_show[0], PHONE_VER);
        printk("[TSP] %s\n", buf);
			   
		return sprintf(buf, "%s", buf );
    }

    else if ( buf_firmware_show[2] == YTE_MODULE_VER_NEW)
    {
        PHONE_VER = FW_VER_NEW_YTE;
        sprintf(buf, "10%x%x%x\n", buf_firmware_show[2], buf_firmware_show[0], PHONE_VER);
        printk("[TSP] %s\n", buf);
		   
		return sprintf(buf, "%s", buf );
    }
    else if (buf_firmware_show[2] == SMAC_MODULE_VER_NEW)
    {
        PHONE_VER = FW_VER_NEW_SMAC;
        sprintf(buf, "10%x%x%x\n", buf_firmware_show[2], buf_firmware_show[0], PHONE_VER);
        printk("[TSP] %s\n", buf);
			   
		return sprintf(buf, "%s", buf );
    }
#if defined(CONFIG_TARGET_LOCALE_AUS_TEL)    
    else if (buf_firmware_show[2] == T_YTE_MODULE_VER_NEW)
    {
        PHONE_VER = T_FW_VER_NEW_YTE;
        sprintf(buf, "10%x0%x0%x\n", buf_firmware_show[2], buf_firmware_show[0], PHONE_VER);
        printk("[TSP] %s\n", buf);
		
		return sprintf(buf, "%s", buf );

    }
    else if (buf_firmware_show[2] == T_SMAC_MODULE_VER)
    {
        PHONE_VER = T_SMAC_FW_VER;
        sprintf(buf, "1%x0%x0%x\n", buf_firmware_show[2], buf_firmware_show[0], PHONE_VER);
        printk("[TSP] %s\n", buf);
		
		return sprintf(buf, "%s", buf );

    }        
#endif    
        sprintf(buf, "10%x0%x0%x\n", buf_firmware_show[2], buf_firmware_show[0], PHONE_VER);
        printk("[TSP] %s\n", buf);
    
	return sprintf(buf, "%s", buf );
}

static ssize_t fimware_show_versname(struct device *dev, struct device_attribute *attr, char *buf)
{
       uint8_t buf_firmware_ver[3];
       int ret;
       
	printk("[TSP] %s\n",__func__);

	buf_firmware_ver[0] = ESCAPE_ADDR;
	buf_firmware_ver[1] = TS_READ_VERSION_ADDR;
	ret = i2c_master_send(ts_global->client,(const char *) &buf_firmware_ver, 2);
	if(ret < 0)
	{
		printk(KERN_ERR "silabs_ts_work_func : i2c_master_send [%d]\n", ret);			
	}

	ret = i2c_master_recv(ts_global->client, (char *) &buf_firmware_ver, 3);
	if(ret < 0)
	{
		printk(KERN_ERR "silabs_ts_work_func : i2c_master_recv [%d]\n", ret);			
	}
	printk("[TSP] ver tsp=%x, HW=%x, SW=%x\n", buf_firmware_ver[1], buf_firmware_ver[2], buf_firmware_ver[0]);

    	sprintf(buf, "%x\n", buf_firmware_ver[0]);
       printk("[TSP] %s\n", buf);
       
	return sprintf(buf, "%s", buf );
}
static ssize_t read_node(struct device *dev, struct device_attribute *attr, char *buf)
{
	char *after;


	int written_bytes = 0 ;  /* & error check */
	uint8_t buffer[NUM_TX_CHANNEL*NUM_RX_CHANNEL*2+QUICKSENSE_OVERHEAD]={0,};

       int i, j, ret;
       
	check_node = simple_strtoul(buf, &after, 10);	
	printk(KERN_INFO "[TSP] %s, %d\n", __func__, __LINE__);

       mdelay(300); 
  
	//
	//	Entering JIG_MODE
	//
	buffer[0] = ESCAPE_ADDR;
	buffer[1] = JIG_MODE_COMMAND;
	ret = i2c_master_send(ts_global->client, buffer, 2);
	if (ret < 0)
	{
		printk("[TSP] i2c_master_send fail! %s : %d, \n", __func__,__LINE__);
		return -1;
	}

	ret = i2c_master_recv(ts_global->client,buffer, 1);

	if (ret < 0)
	{
		printk("[TSP] i2c_master_recv fail! %s : %d, \n", __func__,__LINE__);
		return -1;
	}

	//
	//	quicksense format for reading baseline
	//
	ret = silabs_quicksense(BASELINE_ADDRESS,NUM_TX_CHANNEL*NUM_RX_CHANNEL*2, buffer);
	if (ret != 0)
	{
		printk("[TSP] silabs_quicksense fail! %s : %d, \n", __func__,__LINE__);
		return -1;
	}	

	for (i = 0; i < Tx_Channel; i++)
    {
		for(j = 0 ; j < Rx_Channel; j++)
	{
			baseline_node[i][j] = (buffer[(i*Rx_Channel+j)*2 + QUICKSENSE_OVERHEAD -1] <<8) + buffer[(i*Rx_Channel+j)*2+QUICKSENSE_OVERHEAD];
			printk(" %5d", baseline_node[i][j]);
		}
		printk("\n");
	}


    for (j = 0; j < Rx_Channel; j++)
    {
    	for(i = 0 ; i < Tx_Channel; i++)
		{
			written_bytes += sprintf(buf+written_bytes, ",%d", baseline_node[i][j]) ;
    	}
	}

	 printk("[TSP] %s\n", buf);
	
	touch_ctrl_regulator(0);  
	mdelay(2);
	touch_ctrl_regulator(1);  
	mdelay(300);
	
	return written_bytes;

}

/* firmware - update */

static ssize_t firmware_ret_show(struct device *dev, struct device_attribute *attr, char *buf)
{	
	printk("[TSP] %s!\n", __func__);
    	printk("[TSP] %s : %d\n", __func__, firmware_ret_val);
	firm_update( );
	return sprintf(buf, "%d", firmware_ret_val );
}

int firm_update( void )
{
      int ret;
	printk(KERN_INFO "[TSP] %s, %d\n", __func__, __LINE__);

	disable_irq(tsp_irq);

       ret = cancel_work_sync(&ts_global->work_timer);

       ret = cancel_work_sync(&ts_global->work);
       
	hrtimer_cancel(&ts_global->timer);
       
	touch_ctrl_regulator(TOUCH_OFF);
       mdelay(200);      
	touch_ctrl_regulator(TOUCH_ON);
       mdelay(200);

       TSP_MODULE_ID =  buf_firmware[2];
	 local_irq_disable();
       firmware_ret_val = Firmware_Download();	
	local_irq_enable();

	msleep(1000);
	if( firmware_ret_val )
		printk(KERN_INFO "[TSP] %s success, %d\n", __func__, __LINE__);
	else	
		printk(KERN_INFO "[TSP] %s fail, %d\n", __func__, __LINE__);

	local_irq_enable();

	enable_irq(tsp_irq);

	hrtimer_start(&ts_global->timer, ktime_set(1, 0), HRTIMER_MODE_REL);

	return 0;
} 

module_init(silabs_ts_init);
module_exit(silabs_ts_exit);

MODULE_DESCRIPTION("silabs Touchscreen Driver");
MODULE_LICENSE("GPL");
