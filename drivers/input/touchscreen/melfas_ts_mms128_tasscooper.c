/* drivers/input/touchscreen/melfas_i2c_rmi.c
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
#include <linux/melfas_ts.h>
#include <linux/firmware.h>
#include <mach/memory.h>
#include <linux/irq.h>

#include "mcs8000_download.h"

#if defined(CONFIG_MAX8986_MUIC)
#include <linux/mfd/max8986/max8986.h>
#endif

/** for Cooper VE *****************************************/
#ifdef CONFIG_TOUCHSCREEN_MMS128_COOPERVE

//#define __TOUCH_DEBUG__  //TODO : 서버에 올리는 경우에는 막고 올리기.
#define USE_THREADED_IRQ 1 //TODO : QUEUE 방식이 아닌 THREAD 방식으로 변경. 이렇게 하니, IRQ 가 정상적으로 잘됨.
#define DELAY_BEFORE_VDD

#define SET_DOWNLOAD_BY_GPIO 0 //TODO : TSP 초기화 루틴에서 필요시 강제로 최신 FW 로 업데이트 하는 루틴으로 사용하면 안됨.
#define LATEST_FW_VER_HW00   0x09 //TODO : 이부분을 0x0 으로 하면, SET_DOWNLOAD_BY_GPIO 1 이어도 동작하지 안함.
#define LATEST_FW_VER_HW01   0x10 //TODO : 이부분을 0x0 으로 하면, SET_DOWNLOAD_BY_GPIO 1 이어도 동작하지 안함.
//#define FORCED_DOWNLOAD_OF_BLANKMEMORY	// TSP blank memory( No firmware ) 상태시 자동 펌웨어 다운로드
//#define ALWAYS_DOWNLOAD

#define __TOUCH_KEY__
#define __TOUCH_KEYLED__

#if defined(CONFIG_MAX8986_MUIC)
#define __TOUCH_TA_CHECK__		// for AT&T Charger
#endif

#endif	// CONFIG_TOUCHSCREEN_MMS128_COOPERVE
/***********************************************************/

/** for Tass VE *****************************************/
#ifdef CONFIG_TOUCHSCREEN_MMS128_TASSVE

#define __TOUCH_DEBUG__  //TODO : 서버에 올리는 경우에는 막고 올리기.
#define USE_THREADED_IRQ 1 //TODO : QUEUE 방식이 아닌 THREAD 방식으로 변경. 이렇게 하니, IRQ 가 정상적으로 잘됨.
#define DELAY_BEFORE_VDD

#define SET_DOWNLOAD_BY_GPIO 0 //TODO : TSP 초기화 루틴에서 강제로 최신 FW 로 업데이트 하는 루틴으로 사용하면 안됨.
#define LATEST_FW_VER   0x03 //TODO : 이부분을 0x0 으로 하면, SET_DOWNLOAD_BY_GPIO 1 이어도 동작하지 안함.
//#define FORCED_DOWNLOAD_OF_BLANKMEMORY	// TSP blank memory( No firmware ) 상태시 자동 펌웨어 다운로드
//#define ALWAYS_DOWNLOAD

//#define __TOUCH_KEY__
//#define __TOUCH_KEYLED__

#if defined(CONFIG_MAX8986_MUIC)
#define __TOUCH_TA_CHECK__		// for AT&T Charger
#endif

#endif	// CONFIG_TOUCHSCREEN_MMS128_TASSVE
/***********************************************************/

#define TOUCH_ON  1
#define TOUCH_OFF 0

#if defined (__TOUCH_KEYLED__)
static struct regulator *touchkeyled_regulator=NULL;

static int g_keyled_cnt=0; 		// for auto-off
static bool g_check_action=false;
static bool g_check_keyled=false;	// for check keyled on/off status
#endif
static bool init_intensity=true;
static int g_exciting_ch, g_sensing_ch;

/** structures **/
struct muti_touch_info
{
    int state;
    int strength;
    int width;
    int posX;
    int posY;
};

struct melfas_ts_data
{
    uint16_t addr;
    struct i2c_client *client;
    struct input_dev *input_dev;
    struct work_struct  work;
    struct hrtimer timer;
    struct work_struct work_timer;
    uint32_t flags;
    //int (*power)(int on);
    struct early_suspend early_suspend;
    int VenderID;
    int hw_rev;
    int fw_ver;
};

/** variables **/
static struct muti_touch_info g_Mtouch_info[TS_MAX_TOUCH];
static struct melfas_ts_data *ts;
//struct melfas_ts_data *ts_global_melfas;
#if defined (CONFIG_TOUCHSCREEN_TMA340) || defined (CONFIG_TOUCHSCREEN_TMA340_COOPERVE) || defined (CONFIG_TOUCHSCREEN_F760)
static int8_t MMS128_Connected = 0;
#endif
static struct regulator *touch_regulator = NULL;
static int firmware_ret_val = -1;
static DEFINE_SPINLOCK(melfas_spin_lock);
#if USE_THREADED_IRQ

#else
static struct workqueue_struct *melfas_wq;
#endif

//#ifdef FORCED_DOWNLOAD_OF_BLANKMEMORY
static bool bBlankMemory = false;
//#endif

#if defined (__TOUCH_TA_CHECK__)		// for AT&T Charger
static u8 pre_charger_type = 0;
extern u8 g_charger_type;
static u8 pre_charger_adc = 0;
extern u8 g_charger_adc;
#endif

static struct workqueue_struct *check_ic_wq;


/** functions **/

// temporary function

int bcm_gpio_pull_up(unsigned int gpio, bool up)
{
#if 0
	u32 reg_addr;
	u32 reg_value;
	if(gpio == GPIO_TOUCH_INT)
	{
		reg_addr = 0x350048D0;
	}
	else if(gpio == GPIO_TSP_SCL)
	{
		reg_addr = 0x350049E0;
	}
	else if(gpio == GPIO_TSP_SDA)
	{
		reg_addr = 0x350049E8;
	}
	else
	{
		printk("Wrong GPIO, does not support yet\n");
		return -1;
	}
	reg_value=readl((volatile u32 *)HW_IO_PHYS_TO_VIRT(reg_addr));
	/*
	if(up==true)
		reg_value = (reg_value&~0x60)|0x20;
	else
	*/
		reg_value = (reg_value&~0x60)|0x40;
	writel(reg_value,(volatile u32 *)HW_IO_PHYS_TO_VIRT(reg_addr));
	return 0;
#endif
}

int bcm_gpio_pull_down(unsigned int gpio, bool up)
{
	#if 0
	u32 reg_addr;
	u32 reg_value;
	if(gpio == GPIO_TOUCH_INT)
	{
		reg_addr = 0x350048D0;
	}
	else if(gpio == GPIO_TSP_SCL)
	{
		reg_addr = 0x350049E0;
	}
	else if(gpio == GPIO_TSP_SDA)
	{
		reg_addr = 0x350049E8;
	}
	else
	{
		printk("Wrong GPIO, does not support yet\n");
		return -1;
	}
	reg_value=readl((volatile u32 *)HW_IO_PHYS_TO_VIRT(reg_addr));
	if(up==true)
		reg_value = (reg_value&~0x60)|0x40;
	else
		reg_value = (reg_value&~0x60);
	writel(reg_value,(volatile u32 *)HW_IO_PHYS_TO_VIRT(reg_addr));
	#endif
	return 0;
}


int bcm_gpio_pull_up_down_enable(unsigned int gpio, bool enable)
{
	#if 0
	u32 reg_addr;
	u32 reg_value;
	if(enable==true)
		return;
	if(gpio == GPIO_TOUCH_INT)
	{
		reg_addr = 0x350048D0;
	}
	else if(gpio == GPIO_TSP_SCL)
	{
		reg_addr = 0x350049E0;
	}
	else if(gpio == GPIO_TSP_SDA)
	{
		reg_addr = 0x350049E8;
	}
	else
	{
		printk("Wrong GPIO, does not support yet\n");
		return -1;
	}
	reg_value=readl((volatile u32 *)HW_IO_PHYS_TO_VIRT(reg_addr));
	reg_value = (reg_value&~0x60);
	writel(reg_value,(volatile u32 *)HW_IO_PHYS_TO_VIRT(reg_addr));
	#endif
	return 0;
}



struct class *touch_class_melfas;
EXPORT_SYMBOL(touch_class_melfas);
struct device *firmware_dev_melfas;
EXPORT_SYMBOL(firmware_dev_melfas);

static ssize_t firmware_show(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t firmware_store( struct device *dev, struct device_attribute *attr, const char *buf, size_t size);
static ssize_t firmware_ret_show(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t firmware_ret_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size);
static ssize_t raw_enable_tst200(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t raw_disable_tst200(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t rawdata_pass_fail_tst200(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t tkey_rawcounter_show(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t tkey_rawcounter_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size);

#ifdef CONFIG_TOUCHSCREEN_MMS128_COOPERVE
static DEVICE_ATTR(firmware	, S_IRUGO | S_IWUSR | S_IWOTH | S_IXOTH, firmware_show, firmware_store);
static DEVICE_ATTR(firmware_ret	, S_IRUGO | S_IWUSR | S_IWOTH | S_IXOTH, firmware_ret_show, firmware_ret_store);
#else
static DEVICE_ATTR(firmware	, 0444, firmware_show, firmware_store);
static DEVICE_ATTR(firmware_ret	, 0444, firmware_ret_show, firmware_ret_store);
#endif
static DEVICE_ATTR(raw_enable, 0444, raw_enable_tst200, NULL) ;
static DEVICE_ATTR(raw_disable, 0444, raw_disable_tst200, NULL) ;
static DEVICE_ATTR(raw_value, 0444, rawdata_pass_fail_tst200, NULL) ;
static DEVICE_ATTR(tkey_rawcounter, S_IRUGO | S_IWUSR | S_IWGRP, tkey_rawcounter_show, tkey_rawcounter_store);
static int melfas_ts_suspend(struct i2c_client *client, pm_message_t mesg);
static int melfas_ts_resume(struct i2c_client *client);
#ifdef CONFIG_HAS_EARLYSUSPEND
static void melfas_ts_early_suspend(struct early_suspend *h);
static void melfas_ts_late_resume(struct early_suspend *h);
#endif

#if defined (CONFIG_TOUCHSCREEN_TMA340) || defined (CONFIG_TOUCHSCREEN_TMA340_COOPERVE) || defined (CONFIG_TOUCHSCREEN_F760)
extern int Is_Synaptics_Connected(void);

int Is_MMS128_Connected(void)
{
    return (int) MMS128_Connected;
}
#endif

static int tsp_reset(void);

#if defined (__TOUCH_KEYLED__)
void touch_keyled_ctrl_regulator_mms128(int on_off)
{
	#if 0
	g_keyled_cnt =0;
	if (on_off == TOUCH_ON)
	{
		regulator_set_voltage(touchkeyled_regulator,3300000,3300000);
		regulator_enable(touchkeyled_regulator);
		g_check_keyled = true;
		
	}
	else
	{
		regulator_disable(touchkeyled_regulator);
		g_check_keyled = false;
	}
	#endif
}
EXPORT_SYMBOL(touch_keyled_ctrl_regulator_mms128);
#endif

//TODO : touch_ctrl_regulator() 함수는 다른 파일에서 선언된뒤, export 되어 잇음. //synaptics_i2c_rmi_tma340_cooperve.c synaptics_i2c_rmi_tma340_tassveve.c
void touch_ctrl_regulator_mms128(int on_off)
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

//	writel(0x5, (volatile u32 *)HW_IO_PHYS_TO_VIRT(0x35004894));
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


#if defined (__TOUCH_KEYLED__) 	
	touch_keyled_ctrl_regulator_mms128(on_off);
#endif
}
EXPORT_SYMBOL(touch_ctrl_regulator_mms128);

int tsp_i2c_read_melfas(u8 reg, unsigned char *rbuf, int buf_size) //same with tsp_i2c_read()
{
	int i, ret=-1;
	struct i2c_msg rmsg;
	uint8_t start_reg;

  int retry = 3;

	for (i = 0; i < retry; i++)
	{
		rmsg.addr = ts->client->addr;
		rmsg.flags = 0;//I2C_M_WR;
		rmsg.len = 1;
		rmsg.buf = &start_reg;
		start_reg = reg;
		
		ret = i2c_transfer(ts->client->adapter, &rmsg, 1);

		if(ret >= 0) 
		{
			rmsg.flags = I2C_M_RD;
			rmsg.len = buf_size;
			rmsg.buf = rbuf;
			ret = i2c_transfer(ts->client->adapter, &rmsg, 1 );

			if (ret >= 0)
				break; // i2c success
		}

		if( i == (retry - 1) )
		{
			printk("[TSP] Error code : %d, %d, %d\n", __LINE__, ret, retry);
		}
	}

	return ret;
}


static int melfas_i2c_read(struct i2c_client* p_client, u8 reg, u8* data, int len)
{
	struct i2c_msg msg;

	/* set start register for burst read */
	/* send separate i2c msg to give STOP signal after writing. */
	/* Continous start is not allowed for cypress touch sensor. */

	msg.addr = p_client->addr;
	msg.flags = 0;
	msg.len = 1;
	msg.buf = &reg;

	if (1 != i2c_transfer(p_client->adapter, &msg, 1))
	{
		printk("[TSP][MMS128][%s] set data pointer fail! reg(%x)\n", __func__, reg);
		return -EIO;
	}

	/* begin to read from the starting address */

	msg.addr = p_client->addr;
	msg.flags = I2C_M_RD;
	msg.len = len;
	msg.buf = data;

	if (1 != i2c_transfer(p_client->adapter, &msg, 1))
	{
		printk("[TSP][MMS128][%s] fail! reg(%x)\n", __func__, reg);
		return -EIO;
	}

	return 0;
}

static int melfas_i2c_write(struct i2c_client* p_client, u8* data, int len)
{
	struct i2c_msg msg;

	msg.addr = p_client->addr;
	msg.flags = 0; /* I2C_M_WR */
	msg.len = len;
	msg.buf = data ;

	if (1 != i2c_transfer(p_client->adapter, &msg, 1))
	{
		printk("[TSP][MMS128][%s] set data pointer fail!\n", __func__);
		return -EIO;
	}

	return 0;
}

static int melfas_init_panel(struct melfas_ts_data *ts)
{
	int ret, buf = 0x10;
	ret = melfas_i2c_write(ts->client, &buf, 1);
	ret = melfas_i2c_write(ts->client, &buf, 1);

	if (ret < 0)
	{
		printk(KERN_DEBUG "[TSP][MMS128][%s] melfas_i2c_write() failed\n [%d]", __func__, ret);
		return 0;
	}

	return 1;
}

#if defined (__TOUCH_TA_CHECK__)		// for AT&T Charger

/*
typedef enum  {
	PMU_MUIC_CHGTYP_NONE,
	PMU_MUIC_CHGTYP_USB,
	PMU_MUIC_CHGTYP_DOWNSTREAM_PORT,
	PMU_MUIC_CHGTYP_DEDICATED_CHGR,
	PMU_MUIC_CHGTYP_SPL_500MA,
	PMU_MUIC_CHGTYP_SPL_1A,
	PMU_MUIC_CHGTYP_RESERVED,
	PMU_MUIC_CHGTYP_DEAD_BATT_CHG,

	PMU_MUIC_CHGTYP_INIT
}pmu_muic_chgtyp;
*/

/*
  0x01 : normal charger
  0x02 : AT&T charger
  0x00 : not connected, 복귀 시 필요
  0xff : need to check TA
*/

static bool b_Firmware_store = false;
u8 inform_ATcharger_connection(void)
{
	if(g_charger_adc == PMU_MUIC_ADC_OUTPUT_200K)
	{
        printk(KERN_DEBUG "[TSP][MMS128][%s] AT&T Charger \n [0x%x]", __func__, g_charger_adc);
		return true;
	}
	else
	{
        printk(KERN_DEBUG "[TSP][MMS128][%s] No AT&T Charger \n [0x%x]", __func__, g_charger_adc);
		return false;
	}
}

int inform_charger_connection(u8 state)
{
	int ret;
	u8 buf[2], temp_type;
	bool isATcharger = inform_ATcharger_connection();

	buf[0] = 0xAB;
	if( state == PMU_MUIC_CHGTYP_NONE )
		buf[1] = 0x00;
	else if (isATcharger == true) //else if( state == PMU_MUIC_CHGTYP_SPL_1A )
		buf[1] = 0x02;
	else
		buf[1] = 0x01;

	temp_type = pre_charger_type;
	pre_charger_type = state;

	printk("[TSP][MMS128][%s] set : %d, %d !\n", __func__, state, buf[1]);	

	ret = melfas_i2c_write(ts->client, buf, 2);
	if(ret != 0)
	{
		printk(KERN_DEBUG "[TSP][MMS128][%s] melfas_i2c_write() failed\n [%d]", __func__, ret);
		pre_charger_type = temp_type;
	}

	return ret;

}

#endif

/* additional TS work */
static void check_ic_work_func(struct work_struct *work)
{
#if !defined (__TOUCH_TA_CHECK__) && !defined (__TOUCH_KEYLED__)
	volatile int a;
	a=0;
#endif

#if defined (__TOUCH_TA_CHECK__)		// for AT&T Charger
	if(pre_charger_type != g_charger_type)
	{
		if( !b_Firmware_store )
			inform_charger_connection(g_charger_type);
	}
#endif

#if defined (__TOUCH_KEYLED__)		// auto-off LED 
	if(g_check_keyled)
	{
		if (g_check_action)
		{
			g_keyled_cnt=0;
			g_check_action=false;
		}
		else if(  g_keyled_cnt == 25 )
		{
			touch_keyled_ctrl_regulator_mms128(TOUCH_OFF);
			printk(KERN_DEBUG "[TSP] led off\n",__func__);			
		}
		else
			g_keyled_cnt++;
	}
#endif
	return;
}



static enum hrtimer_restart melfas_watchdog_timer_func(struct hrtimer *timer)
{
	queue_work(check_ic_wq, &ts->work_timer);

	hrtimer_start(&ts->timer, ktime_set(0, 200000000), HRTIMER_MODE_REL);
	return HRTIMER_NORESTART;
}



#if USE_THREADED_IRQ
static irqreturn_t melfas_ts_work_func(int irq, void *dev_id)
#else
static void melfas_ts_work_func(struct work_struct *work)
#endif
{
#if USE_THREADED_IRQ
	struct melfas_ts_data *ts = dev_id;
#else
	struct melfas_ts_data *ts = container_of(work, struct melfas_ts_data, work);
#endif
	int ret = 0, i, j;
	uint8_t buf[66];
	int read_num = 0, touchType = 0, touchState = 0, fingerID = 0, keyID = 0;
	unsigned long flags;

#ifdef __TOUCH_KEYLED__
	bool b_keyledOn = false;
#endif

#ifdef __TOUCH_DEBUG__
	printk(KERN_DEBUG "[TSP][MMS128][%s] \n",__func__);
#endif
	if (ts == NULL)
		printk("[TSP][MMS128][%s] : TS NULL\n",__func__);

	/**
	Simple send transaction:
	S Addr Wr [A]  Data [A] Data [A] ... [A] Data [A] P
	Simple recv transaction:
	S Addr Rd [A]  [Data] A [Data] A ... A [Data] NA P
	*/
	for (i = 0; i < 10; i++)
	{
		ret = melfas_i2c_read(ts->client, 0x0F, buf, 1);
		if (ret >= 0)
			break; // i2c success
	}
	spin_lock_irqsave(&melfas_spin_lock, flags);
	if (ret < 0)
	{
		printk("[TSP][MMS128][%s] i2c failed : %d\n", __func__, ret);
		enable_irq(ts->client->irq);
		spin_unlock_irqrestore(&melfas_spin_lock, flags);
		touch_ctrl_regulator_mms128(TOUCH_OFF);
		touch_ctrl_regulator_mms128(TOUCH_ON);
		melfas_init_panel(ts);
		return ;
	}
	else
	{
		read_num = buf[0];
	}

	if (read_num > 0)
	{
		for (i = 0; i < 10; i++)
		{
			ret = melfas_i2c_read(ts->client, 0x10, buf, read_num);
			if (ret >= 0)
				break; // i2c success
		}

		if (ret < 0)
		{
			printk("[TSP][MMS128][%s] i2c failed : %d\n", __func__, ret);
			enable_irq(ts->client->irq);
			spin_unlock_irqrestore(&melfas_spin_lock, flags);
			touch_ctrl_regulator_mms128(TOUCH_OFF);
			touch_ctrl_regulator_mms128(TOUCH_ON);
			melfas_init_panel(ts);
			return ;
		}
		else
		{
			bool touched_src = false;

			if (buf[0] == 0x0f)
			{
				printk("[TSP][MMS128][%s] ESD defense!!  : %d\n", __func__, fingerID);
				enable_irq(ts->client->irq);
				spin_unlock_irqrestore(&melfas_spin_lock, flags);
				touch_ctrl_regulator_mms128(TOUCH_OFF);
				touch_ctrl_regulator_mms128(TOUCH_ON);
				melfas_init_panel(ts);
				return;
			}
			
			for (i = 0; i < read_num; i = i + 6)
			{
				touchType = (buf[i] >> 5) & 0x03;
				touchState = (buf[i] & 0x80);

				if (touchType == 1)	//Screen
				{
					touched_src = true;
					fingerID = (buf[i] & 0x0F) - 1;

					if ((fingerID > TS_MAX_TOUCH - 1) || (fingerID < 0))
					{
						printk("[TSP][MMS128][%s] fingerID : %d\n", __func__, fingerID);
						enable_irq(ts->client->irq);
						spin_unlock_irqrestore(&melfas_spin_lock, flags);
						touch_ctrl_regulator_mms128(TOUCH_OFF);
						touch_ctrl_regulator_mms128(TOUCH_ON);
						melfas_init_panel(ts);
						return ;
					}

					g_Mtouch_info[fingerID].posX = (uint16_t)(buf[i + 1] & 0x0F) << 8 | buf[i + 2];
					g_Mtouch_info[fingerID].posY = (uint16_t)(buf[i + 1] & 0xF0) << 4 | buf[i + 3];
					g_Mtouch_info[fingerID].width = buf[i + 4];

					if (touchState)
						g_Mtouch_info[fingerID].strength = buf[i + 5];
					else
						g_Mtouch_info[fingerID].strength = 0;
				}
 #ifdef __TOUCH_KEY__
				else if (touchType == 2)	//Key
				{
					keyID = (buf[i] & 0x0F);

					if (keyID == 0x1)
						input_report_key(ts->input_dev, KEY_MENU, touchState ? PRESS_KEY : RELEASE_KEY);
					if (keyID == 0x2)
						input_report_key(ts->input_dev, KEY_BACK, touchState ? PRESS_KEY : RELEASE_KEY);

#ifdef __TOUCH_KEYLED__
					if( !g_check_keyled)
					{
						b_keyledOn = true;
					}
#endif

#ifdef __TOUCH_DEBUG__
					printk(KERN_DEBUG "[TSP][MMS128][%s] keyID: %d, State: %d\n", __func__, keyID, touchState);
#endif
				}
 #endif	//  __TOUCH_KEY__
			}
			if (touched_src)
			{
				for (j = 0; j < TS_MAX_TOUCH; j ++)
				{
					if (g_Mtouch_info[j].strength == -1)
						continue;

					input_report_abs(ts->input_dev, ABS_MT_TRACKING_ID, j);
					input_report_abs(ts->input_dev, ABS_MT_POSITION_X, g_Mtouch_info[j].posX);
					input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, g_Mtouch_info[j].posY);
					input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, g_Mtouch_info[j].strength);
					input_report_abs(ts->input_dev, ABS_MT_WIDTH_MAJOR, g_Mtouch_info[j].width);
					input_mt_sync(ts->input_dev);
#ifdef __TOUCH_DEBUG__
					printk(KERN_DEBUG "[TSP][MMS128][%s] fingerID: %d, State: %d, x: %d, y: %d, z: %d, w: %d\n",
						__func__, j, (g_Mtouch_info[j].strength > 0), g_Mtouch_info[j].posX, g_Mtouch_info[j].posY, g_Mtouch_info[j].strength, g_Mtouch_info[j].width);
#endif
					if (g_Mtouch_info[j].strength == 0)
						g_Mtouch_info[j].strength = -1;
				}
			}				
			input_sync(ts->input_dev);

#ifdef __TOUCH_KEYLED__
			g_check_action = true;
#endif
		}
	}

#if USE_THREADED_IRQ

#else
	enable_irq(ts->client->irq);
#endif

	spin_unlock_irqrestore(&melfas_spin_lock, flags);

#ifdef __TOUCH_KEYLED__
	if( b_keyledOn )
	{
		touch_keyled_ctrl_regulator_mms128(TOUCH_ON);
		msleep(70);
		printk(KERN_DEBUG "[TSP] led on\n",__func__);			
	} 
#endif

#if USE_THREADED_IRQ
	return IRQ_HANDLED;
#else

#endif
}


static irqreturn_t melfas_ts_irq_handler(int irq, void *dev_id)
{
	struct melfas_ts_data *ts = dev_id;

#if USE_THREADED_IRQ

#else
	disable_irq_nosync(ts->client->irq);
#endif

#if USE_THREADED_IRQ
	return IRQ_WAKE_THREAD;
#else
	queue_work(melfas_wq, &ts->work);
	return IRQ_HANDLED;
#endif
}


bool melfas_ts_upgrade_check(void)
{
/*********************************************/
#ifdef ALWAYS_DOWNLOAD
	return true;
#endif
/*********************************************/
#ifdef FORCED_DOWNLOAD_OF_BLANKMEMORY
	if(bBlankMemory) 
		return true;
#endif
/*********************************************/
#if SET_DOWNLOAD_BY_GPIO
#if defined(CONFIG_TOUCHSCREEN_MMS128_COOPERVE)
	if( ts->hw_ver==00)
	{
		if(ts->fw_ver < LATEST_FW_VER_HW00)
			return true;
	}
	else if(ts->hw_ver==01)
	{
		if(ts->fw_ver < LATEST_FW_VER_HW01)
			return true;
	}
	else
	{
		printk(KERN_DEBUG "[TSP][%s] wrong hw_ver\n", __func__);
	}
#elif defined(CONFIG_TOUCHSCREEN_MMS128_TASSVE)
	if( ts->fw_ver < LATEST_FW_VER)
		return true;
#else
#error "[TSP][Error] Melfas_ts_mms128_tasscooper.c : Feature missing!!"
#endif
#endif	// SET_DOWNLOAD_BY_GPIO
/*********************************************/

	return false;
}

void melfas_upgrade(INT32 hw_ver)
{
	int ret;
	unsigned char buf[2];

	printk("[TSP][MMS128][F/W D/L] Entry mcsdl_download_binary_data\n");
	printk("[TOUCH] Melfas  H/W version: 0x%02x.\n", ts->hw_rev);
	printk("[TOUCH] Current F/W version: 0x%02x.\n", ts->fw_ver);	

	disable_irq(ts->client->irq);
	local_irq_disable();

	ret = mcsdl_download_binary_data(ts->hw_rev);

	local_irq_enable();
	enable_irq(ts->client->irq);

	printk("[TSP] melfas_upgrade()--  ret=%d\n", ret);

	if (0 == melfas_i2c_read(ts->client, MCSTS_MODULE_VER_REG, buf, 2))
	{
		ts->hw_rev = buf[0];
		ts->fw_ver = buf[1];
		printk("[TSP][MMS128][%s] HW Ver : 0x%02x, FW Ver : 0x%02x\n", __func__, buf[0], buf[1]);
	}
	else
	{
		ts->hw_rev = 0;
		ts->fw_ver = 0;
		printk("[TSP][MMS128][%s] Can't find HW Ver, FW ver!\n", __func__);
	}

	if (ret > 0)
	{
		if ((ts->hw_rev < 0) || (ts->fw_ver < 0))
			printk(KERN_DEBUG "[TSP][MMS128][%s] i2c_transfer failed\n",__func__);
		else
			printk("[TSP][MMS128][%s] Firmware update success! [Melfas H/W version: 0x%02x., Current F/W version: 0x%02x.]\n", __func__, ts->hw_rev, ts->fw_ver);
	}
	else
	{
		printk("[TSP][MMS128][%s] Firmware update failed.. RESET!\n",__func__);
		mcsdl_vdd_off();
		mdelay(500);
		mcsdl_vdd_on();
		mdelay(200);
	}

	if(ret == MCSDL_RET_SUCCESS)
		firmware_ret_val = 1;
	else
		firmware_ret_val = 0;	

}

int melfas_ts_check(struct melfas_ts_data *ts)
{
	int ret, i;
	uint8_t buf_tmp[3]={0,0,0};
	int retry = 3;
	uint8_t VenderID;

	ret = tsp_i2c_read_melfas(0x1B, buf_tmp, sizeof(buf_tmp));		

	// i2c read retry
	if(ret <= 0)
	{
		for(i=0; i<retry;i++)
		{
			ret=tsp_i2c_read_melfas( 0x1B, buf_tmp, sizeof(buf_tmp));

			if(ret > 0)
				break;
		}
	}

	if (ret <= 0) 
	{
		printk("[TSP][MMS128][%s] %s\n", __func__,"Failed melfas i2c");
#if defined (CONFIG_TOUCHSCREEN_TMA340) || defined (CONFIG_TOUCHSCREEN_TMA340_COOPERVE) || defined (CONFIG_TOUCHSCREEN_F760)
		MMS128_Connected = 0;
#endif
	}
	else 
	{
		printk("[TSP][MMS128][%s] %s\n", __func__,"Passed melfas i2c");
#if defined (CONFIG_TOUCHSCREEN_TMA340) || defined (CONFIG_TOUCHSCREEN_TMA340_COOPERVE) || defined (CONFIG_TOUCHSCREEN_F760)
		MMS128_Connected = 1;
#endif
	}

	ts->VenderID = buf_tmp[0];
	ts->hw_rev = buf_tmp[1];
	ts->fw_ver = buf_tmp[2];	

	printk("[TSP][MMS128][%s][SlaveAddress : 0x%x][ret : %d] [ID : 0x%x] [HW : 0x%x] [SW : 0x%x]\n", __func__,ts->client->addr, ret, buf_tmp[0],buf_tmp[1],buf_tmp[2]);

	if( (ret > 0) && (ts->VenderID == 0xa0 ) )
	{
		ret = 1;
		printk("[TSP][MMS128][%s] %s\n", __func__,"Passed melfas_ts_check");
	}
#ifdef FORCED_DOWNLOAD_OF_BLANKMEMORY
	else if ( (ret > 0) && (ts->VenderID == 0x0)&& (ts->hw_rev == 0x0) && (ts->fw_ver == 0x0) )
	{
		ret = 1;
		bBlankMemory = true;
		printk("[TSP][MMS128][%s] %s\n", __func__,"Blank memory !!");
		printk("[TSP][MMS128][%s] %s\n", __func__,"Passed melfas_ts_check");
	}
#endif
	else
	{
		ts->hw_rev = 0;
		ts->fw_ver = 0;
		ret = 0;
		printk("[TSP][MMS128][%s] %s\n", __func__,"Failed melfas_ts_check");
	}

	return ret;
}

static int melfas_ts_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret = 0, i;
	uint8_t buf[2];

#if defined (CONFIG_TOUCHSCREEN_TMA340) || defined (CONFIG_TOUCHSCREEN_TMA340_COOPERVE) || defined (CONFIG_TOUCHSCREEN_F760)
	printk("[TSP][MMS128][%s] %s\n", __func__, "Called");
	if (Is_Synaptics_Connected() == 1)
	{
		printk("[TSP][MMS128][%s] %s\n", __func__, "Synaptics already detected !!");
		return -ENXIO;
	}
#endif

	printk(KERN_DEBUG"+-----------------------------------------+\n");
	printk(KERN_DEBUG "|  Melfas Touch Driver Probe!            |\n");
	printk(KERN_DEBUG"+-----------------------------------------+\n");


	/* Pin Initialize ****************************************************/
	//disable BB internal pulls for touch int, scl, sda pin

	//gpio_direction_output( GPIO_TOUCH_INT , 0 );
	gpio_direction_output( GPIO_TSP_SCL , 0 ); 
	gpio_direction_output( GPIO_TSP_SDA , 0 ); 
	gpio_request(GPIO_TOUCH_INT, "ts_irq");
	gpio_direction_input(GPIO_TOUCH_INT);
	
	irq_set_irq_type(gpio_to_irq(GPIO_TOUCH_INT), IRQ_TYPE_EDGE_FALLING);	


	//touch_ctrl_regulator_mms128(TOUCH_OFF);

	/****************************************************************/	

	msleep(100);

	gpio_direction_output( GPIO_TSP_SCL , 1 ); 
	gpio_direction_output( GPIO_TSP_SDA , 1 ); 
	//gpio_direction_output( TSP_INT , 1 ); 

	gpio_direction_input(GPIO_TOUCH_INT);
	bcm_gpio_pull_up(GPIO_TOUCH_INT, true);
	bcm_gpio_pull_up_down_enable(GPIO_TOUCH_INT, true);

#ifdef DELAY_BEFORE_VDD
	msleep(10);
#endif
	touch_ctrl_regulator_mms128(TOUCH_ON);
	msleep(70);
	/****************************************************************/
	ts = kzalloc(sizeof(struct melfas_ts_data), GFP_KERNEL);
	if (ts == NULL)
	{
		printk(KERN_DEBUG "[TSP][MMS128][%s] failed to create a state of melfas-ts\n",__func__);
		ret = -ENOMEM;
		goto err_alloc_data_failed;
	}

#if USE_THREADED_IRQ

#else
	INIT_WORK(&ts->work, melfas_ts_work_func);
#endif

	INIT_WORK(&ts->work_timer, check_ic_work_func);


	ts->client = client;
	i2c_set_clientdata(client, ts);

	/* Melfas TSP check routine */
	ret = melfas_ts_check(ts);
	if (ret <= 0)
	{
		i2c_release_client(client);	

		gpio_direction_output( GPIO_TOUCH_INT , 0 );
		gpio_direction_output( GPIO_TSP_SCL , 0 ); 
		gpio_direction_output( GPIO_TSP_SDA , 0 ); 

		bcm_gpio_pull_up(GPIO_TOUCH_INT, false);
		bcm_gpio_pull_up_down_enable(GPIO_TOUCH_INT, true);
		
		touch_ctrl_regulator_mms128(TOUCH_OFF); //Melfas 에서 TOUCH_OFF 하면, Cypress 로 OFF 된다.
		kfree(ts);

		return -ENXIO;
	}
	/* ~Melfas TSP check routine */

	/* sys fs */
	touch_class_melfas = class_create(THIS_MODULE, "touch");
	if (IS_ERR(touch_class_melfas))
		pr_err("[TSP][MMS128] Failed to create class(touch)!\n");
	firmware_dev_melfas = device_create(touch_class_melfas, NULL, 0, NULL, "firmware");
	if (IS_ERR(firmware_dev_melfas))
		pr_err("[TSP][MMS128] Failed to create device(firmware)!\n");
	if (device_create_file(firmware_dev_melfas, &dev_attr_firmware) < 0)
		pr_err("[TSP][MMS128] Failed to create device file(%s)!\n", dev_attr_firmware.attr.name);
	if (device_create_file(firmware_dev_melfas, &dev_attr_firmware_ret) < 0)
		pr_err("[TSP][MMS128] Failed to create device file(%s)!\n", dev_attr_firmware_ret.attr.name);
	if (device_create_file(firmware_dev_melfas, &dev_attr_raw_enable) < 0)
		pr_err("Failed to create device file(%s)!\n", dev_attr_raw_enable.attr.name);
	if (device_create_file(firmware_dev_melfas, &dev_attr_raw_disable) < 0)
		pr_err("Failed to create device file(%s)!\n", dev_attr_raw_disable.attr.name);
	if (device_create_file(firmware_dev_melfas, &dev_attr_raw_value) < 0)
		pr_err("Failed to create device file(%s)!\n", dev_attr_raw_value.attr.name);	
	if (device_create_file(firmware_dev_melfas, &dev_attr_tkey_rawcounter) < 0)
		pr_err("Failed to create device file(%s)!\n", dev_attr_tkey_rawcounter.attr.name);
	/* ~sys fs */

	if ( melfas_ts_upgrade_check() )
	{
#if defined (__TOUCH_TA_CHECK__)		// for AT&T Charger    
		b_Firmware_store = true; 
#endif
		melfas_upgrade(ts->hw_rev);
		msleep(1);
		touch_ctrl_regulator_mms128(TOUCH_OFF);
		msleep(1000);
		touch_ctrl_regulator_mms128(TOUCH_ON);
		msleep(70);
		//local_irq_disable();
		//mcsdl_download_binary_data();
		//printk("[TSP] enable_irq : %d\n", __LINE__);
		//local_irq_enable();
#if defined (__TOUCH_TA_CHECK__)		// for AT&T Charger
		b_Firmware_store = false; 
#endif
	}

	ts->input_dev = input_allocate_device();
	if (!ts->input_dev)
	{
		printk(KERN_DEBUG "[TSP][MMS128][%s] Failed to allocate input device\n",__func__);
		ret = -ENOMEM;
		goto err_input_dev_alloc_failed;
	}
	ts->input_dev->name = "sec_touchscreen" ;

	/** Event handler 관련 ************************************************/
#if defined(CONFIG_TOUCHSCREEN_MMS128_COOPERVE) //CooperVE
	set_bit(EV_SYN, ts->input_dev->evbit);
	set_bit(EV_KEY, ts->input_dev->evbit);
	set_bit(BTN_TOUCH, ts->input_dev->keybit);
	set_bit(EV_ABS, ts->input_dev->evbit);

	input_set_abs_params(ts->input_dev, ABS_MT_POSITION_X, 0, TS_MAX_X_COORD, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_POSITION_Y, 0, TS_MAX_Y_COORD, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0, TS_MAX_Z_TOUCH, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_TRACKING_ID, 0, TS_MAX_TOUCH - 1, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_WIDTH_MAJOR, 0, TS_MAX_W_TOUCH, 0, 0);

	ts->input_dev->keybit[BIT_WORD(KEY_MENU)] |= BIT_MASK(KEY_MENU);
	ts->input_dev->keybit[BIT_WORD(KEY_BACK)] |= BIT_MASK(KEY_BACK);	
#elif defined(CONFIG_TOUCHSCREEN_MMS128_TASSVE) //TassVE
    ts->input_dev->evbit[0] = BIT_MASK(EV_ABS) | BIT_MASK(EV_KEY);
    ts->input_dev->keybit[BIT_WORD(KEY_POWER)] |= BIT_MASK(KEY_POWER);

    set_bit(BTN_TOUCH, ts->input_dev->keybit);
    set_bit(EV_ABS,  ts->input_dev->evbit);
    ts->input_dev->evbit[0] =  BIT_MASK(EV_SYN) | BIT_MASK(EV_ABS) | BIT_MASK(EV_KEY);

    input_set_abs_params(ts->input_dev, ABS_MT_POSITION_X, 0, 240 /*TS_MAX_X_COORD*/, 0, 0);
    input_set_abs_params(ts->input_dev, ABS_MT_POSITION_Y, 0, 320 /*TS_MAX_Y_COORD*/, 0, 0);
    input_set_abs_params(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0, TS_MAX_Z_TOUCH, 0, 0);
    input_set_abs_params(ts->input_dev, ABS_MT_TRACKING_ID, 0, TS_MAX_TOUCH - 1, 0, 0);
    input_set_abs_params(ts->input_dev, ABS_MT_WIDTH_MAJOR, 0, TS_MAX_W_TOUCH, 0, 0);

    set_bit(EV_SYN, ts->input_dev->evbit);
    set_bit(EV_KEY, ts->input_dev->evbit);
#else
#error "[TSP][Error] Melfas_ts_mms128_tasscooper.c : Feature missing!!"
#endif
	/*******************************************************************/

	ret = input_register_device(ts->input_dev);
	if (ret)
	{
		printk(KERN_DEBUG "[TSP][MMS128][%s] Unable to register %s input device\n", __func__, ts->input_dev->name);
		goto err_input_register_device_failed;
	}

	printk("[TSP][MMS128][%s] irq=%d\n", __func__, client->irq);

#if 0
	gpio_request(GPIO_TOUCH_INT, "ts_irq");
	gpio_direction_input(GPIO_TOUCH_INT);
	bcm_gpio_pull_up(GPIO_TOUCH_INT, true);
	bcm_gpio_pull_up_down_enable(GPIO_TOUCH_INT, true);
	irq_set_irq_type(gpio_to_irq(GPIO_TOUCH_INT), IRQ_TYPE_EDGE_FALLING);
#endif

	if (ts->client->irq)
	{
		printk(KERN_DEBUG "[TSP][MMS128][%s] trying to request irq: %s-%d\n", __func__, ts->client->name, ts->client->irq);
#if USE_THREADED_IRQ
		ret = request_threaded_irq(ts->client->irq, melfas_ts_irq_handler, melfas_ts_work_func, IRQF_TRIGGER_FALLING | IRQF_ONESHOT, ts->client->name, ts);
#else
		ret = request_irq(ts->client->irq, melfas_ts_irq_handler, IRQF_TRIGGER_FALLING, ts->client->name, ts);
#endif
		if (ret > 0)
		{
			printk(KERN_DEBUG "[TSP][MMS128][%s] Can't allocate irq %d, ret %d\n", __func__, ts->client->irq, ret);
			ret = -EBUSY;
			goto err_request_irq;
		}
	}

	//schedule_work(&ts->work);
	//queue_work(melfas_wq, &ts->work);

	for (i = 0; i < TS_MAX_TOUCH ; i++)
		g_Mtouch_info[i].strength = -1;

	hrtimer_init(&ts->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	ts->timer.function = melfas_watchdog_timer_func;
	hrtimer_start(&ts->timer, ktime_set(0, 200000000), HRTIMER_MODE_REL);

#ifdef CONFIG_HAS_EARLYSUSPEND
	ts->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	ts->early_suspend.suspend = melfas_ts_early_suspend;
	ts->early_suspend.resume = melfas_ts_late_resume;
	register_early_suspend(&ts->early_suspend);
#endif

	printk(KERN_INFO "[TSP][MMS128][%s] Start touchscreen. name: %s, irq: %d\n", __func__, ts->client->name, ts->client->irq);

	return 0;

#if 1
err_request_irq:
	printk(KERN_DEBUG "[TSP][MMS128][%s] err_request_irq failed\n",__func__);
	free_irq(client->irq, ts);
#endif
err_input_register_device_failed:
	printk(KERN_DEBUG "[TSP][MMS128][%s] err_input_register_device failed\n",__func__);
	input_free_device(ts->input_dev);
err_input_dev_alloc_failed:
	printk(KERN_DEBUG "[TSP][MMS128][%s] err_input_dev_alloc failed\n",__func__);
err_alloc_data_failed:
	printk(KERN_DEBUG "[TSP][MMS128][%s] err_alloc_data failed_\n",__func__);
#if 0
err_detect_failed:
	printk(KERN_DEBUG "melfas-ts: err_detect failed\n");
	kfree(ts);
err_check_functionality_failed:
	printk(KERN_DEBUG "melfas-ts: err_check_functionality failed_\n");
#endif
	return ret;
}

static int melfas_ts_remove(struct i2c_client *client)
{
	struct melfas_ts_data *ts = i2c_get_clientdata(client);

	unregister_early_suspend(&ts->early_suspend);
	free_irq(client->irq, ts);
	input_unregister_device(ts->input_dev);
	kfree(ts);
	return 0;
}

static void release_all_fingers(struct melfas_ts_data *ts)
{
	int i;

	for (i = 0; i < TS_MAX_TOUCH; i++)
	{
		if (g_Mtouch_info[i].strength == -1)
			continue;

		g_Mtouch_info[i].strength = 0;
		g_Mtouch_info[i].width = 0;
		g_Mtouch_info[i].posX = 0;
		g_Mtouch_info[i].posY = 0;

		input_report_abs(ts->input_dev, ABS_MT_TRACKING_ID, i);
		input_report_abs(ts->input_dev, ABS_MT_POSITION_X, g_Mtouch_info[i].posX);
		input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, g_Mtouch_info[i].posY);
		input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, g_Mtouch_info[i].strength);
		input_report_abs(ts->input_dev, ABS_MT_WIDTH_MAJOR, g_Mtouch_info[i].width);
		input_mt_sync(ts->input_dev);

		if (g_Mtouch_info[i].strength == 0)
			g_Mtouch_info[i].strength = -1;
	}
	input_sync(ts->input_dev);
}

static int melfas_ts_suspend(struct i2c_client *client, pm_message_t mesg)
{
	int ret = 0;
	struct melfas_ts_data *ts = i2c_get_clientdata(client);

	release_all_fingers(ts);

	disable_irq(client->irq);

	printk("[TSP][MMS128][%s] irq=%d\n", __func__, client->irq);

	ret = cancel_work_sync(&ts->work);

	return 0;
}

static int melfas_ts_resume(struct i2c_client *client)
{
	struct melfas_ts_data *ts = i2c_get_clientdata(client);

	msleep(70);

	melfas_init_panel(ts);
	enable_irq(client->irq); // scl wave

	printk("[TSP][MMS128][%s] irq=%d\n", __func__, client->irq);

	return 0;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void melfas_ts_early_suspend(struct early_suspend *h)
{
	printk("[TSP][MMS128][%s] \n", __FUNCTION__);
	release_all_fingers(ts);
	disable_irq(ts->client->irq);

	gpio_direction_output( GPIO_TOUCH_INT , 0 );
	gpio_direction_output( GPIO_TSP_SCL , 0 ); 
	gpio_direction_output( GPIO_TSP_SDA , 0 ); 

	bcm_gpio_pull_up(GPIO_TOUCH_INT, false);
	bcm_gpio_pull_up_down_enable(GPIO_TOUCH_INT, true);

	touch_ctrl_regulator_mms128(TOUCH_OFF);
	//ts = container_of(h, struct melfas_ts_data, early_suspend);
	//melfas_ts_suspend(ts->client, PMSG_SUSPEND);
}

static void melfas_ts_late_resume(struct early_suspend *h)
{
	printk("[TSP][MMS128][%s] \n", __FUNCTION__);

	gpio_direction_output( GPIO_TSP_SCL , 1 ); 
	gpio_direction_output( GPIO_TSP_SDA , 1 ); 
	//gpio_direction_output( TSP_INT , 1 ); 

	gpio_direction_input(GPIO_TOUCH_INT);
	bcm_gpio_pull_up(GPIO_TOUCH_INT, true);
	bcm_gpio_pull_up_down_enable(GPIO_TOUCH_INT, true);

#ifdef DELAY_BEFORE_VDD
	msleep(10);
#endif

	touch_ctrl_regulator_mms128(TOUCH_ON);
	msleep(70);
	enable_irq(ts->client->irq);
	//ts = container_of(h, struct melfas_ts_data, early_suspend);
	//melfas_ts_resume(ts->client);

#if defined (__TOUCH_TA_CHECK__)		// for AT&T Charger
	pre_charger_type = 0xff;
#endif
}
#endif

static const struct i2c_device_id melfas_ts_id[] =
{
	{ MELFAS_TS_NAME, 0 },
	{ }
};

static struct i2c_driver melfas_ts_driver =
{
	.probe	 = melfas_ts_probe,
	.remove = __devexit_p(melfas_ts_remove),
#ifndef CONFIG_HAS_EARLYSUSPEND
	.suspend	= melfas_ts_suspend,
	.resume	= melfas_ts_resume,
#endif
	.id_table	= melfas_ts_id,
	.driver = 	{
		.name	= MELFAS_TS_NAME,
	},
};

static int __devinit melfas_ts_init(void)
{
	printk("[TSP] %s\n", __func__ );
#if defined (CONFIG_TOUCHSCREEN_TMA340) || defined (CONFIG_TOUCHSCREEN_TMA340_COOPERVE) || defined (CONFIG_TOUCHSCREEN_F760)

	if (Is_Synaptics_Connected() == 1)
	{
		printk("[TSP][MMS128][%s] %s\n", __func__, "Synaptics already detected !!");
		return -ENXIO;
	}
#endif


#if USE_THREADED_IRQ

#else	
	melfas_wq = create_workqueue("melfas_wq");
	if (!melfas_wq)
		return -ENOMEM;
#endif

	check_ic_wq = create_workqueue("check_ic_wq");
	if(!check_ic_wq)
		return -ENOMEM;
#if 0
	touch_regulator = regulator_get(NULL, "touch_vcc");

#if defined (__TOUCH_KEYLED__)
	touchkeyled_regulator = regulator_get(NULL,"touch_keyled");
#endif
#endif

	return i2c_add_driver(&melfas_ts_driver);
}

static void __exit melfas_ts_exit(void)
{
	printk("[TSP] %s\n", __func__ );

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

	i2c_del_driver(&melfas_ts_driver);

#if USE_THREADED_IRQ

#else
	if (melfas_wq)
		destroy_workqueue(melfas_wq);
#endif
}

static ssize_t firmware_show(struct device *dev, struct device_attribute *attr, char *buf)
{	
	u8 buf1[2] = {0,};
	int hw_rev, fw_ver, phone_ver;

	if (0 == melfas_i2c_read(ts->client, MCSTS_MODULE_VER_REG, buf1, 2))
	{
		hw_rev = buf1[0];
		fw_ver = buf1[1];

#if defined(CONFIG_TOUCHSCREEN_MMS128_COOPERVE)
		if(hw_rev==0x00)
			phone_ver = LATEST_FW_VER_HW00;
		else if(hw_rev==0x01)
			phone_ver = LATEST_FW_VER_HW01;
		else
			phone_ver = 0x00;
#elif defined(CONFIG_TOUCHSCREEN_MMS128_TASSVE)
		phone_ver = LATEST_FW_VER;
#else
#error "[TSP][Error] Melfas_ts_mms128_tasscooper.c : Feature missing!!"
#endif

		/*
			TSP phone Firmware version : phone_ver (xx)
			TSP IC	  Firmware version   : fw_ver (xx)
			HW 				  version	  : hw_rev  (xxx)
		*/

		sprintf(buf, "%03X%02X%02X\n", hw_rev, fw_ver, phone_ver); // 10003xx

		printk("[TSP][MMS128][%s]  phone_ver=%d, fw_ver=%d, hw_rev=%d\n",buf, phone_ver,fw_ver, hw_rev );

	}
	else
	{	 
		printk("[TSP][MMS128][%s] Can't find HW Ver, FW ver!\n", __func__);
	}

	return sprintf(buf, "%s", buf); 
}

static ssize_t firmware_store(
	struct device *dev, struct device_attribute *attr,
	const char *buf, size_t size)
{
	char *after;
	int ret;

	unsigned long value = simple_strtoul(buf, &after, 10);	
	printk(KERN_INFO "[TSP] %s, %d\n", __func__, __LINE__);
	firmware_ret_val = -1;

	printk("[TSP] firmware_store  valuie : %d\n",value);
	if ( value == 1 )
	{
		printk("[TSP] Firmware update start!!\n" );

		//firm_update( );
		//	  melfas_upgrade(ts->hw_rev);
#if defined (__TOUCH_TA_CHECK__)		// for AT&T Charger
		b_Firmware_store = true;
#endif

		disable_irq(ts->client->irq);
		local_irq_disable();

		ret = mms100_ISC_download_binary_data(ts->hw_rev);

		local_irq_enable();
		enable_irq(ts->client->irq);

#if defined (__TOUCH_TA_CHECK__)		// for AT&T Charger
		b_Firmware_store = false;
#endif

		if(ret == MCSDL_RET_SUCCESS)
			firmware_ret_val = 1;
		else
			firmware_ret_val = 0;	

		printk("[TSP] Firmware update end!!\n" );		

		return size;
	}

	return size;
}

static ssize_t firmware_ret_show(struct device *dev, struct device_attribute *attr, char *buf)
{	
	printk("[TSP][MMS128][%s] !\n", __func__);

	return sprintf(buf, "%d", firmware_ret_val );
}

static ssize_t firmware_ret_store(
	struct device *dev, struct device_attribute *attr,
	const char *buf, size_t size)
{
	printk("[TSP][MMS128][%s] operate nothing!\n", __func__);

	return size;
}

static ssize_t raw_enable_tst200(struct device *dev, struct device_attribute *attr, char *buf)
{
	int i, ret;
	uint8_t buf1[2] = {0,};
#if 0
	/////* Enter Inspection Mode */////
	for (i = 0; i < I2C_RETRY_CNT; i++)
	{
		buf1[0] = 0x00;//address
		buf1[1] = 0x70;//value
		ret = i2c_master_send(ts_global->client, buf1, 2);	//enter Inspection Mode
	
		if (ret >= 0)
			break; // i2c success
	}
	
	tsp_testmode = 1;
	printk("[TSP] %s start. line : %d, \n", __func__,__LINE__);

	mdelay(300); 
#endif
	return 1;
}


static ssize_t raw_disable_tst200(struct device *dev, struct device_attribute *attr, char *buf)
{
	int i, ret;
	uint8_t buf1[2] = {0,};

#if 0	
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
#endif
	return 1;
}

static ssize_t rawdata_pass_fail_tst200(struct device *dev, struct device_attribute *attr, char *buf)
{
	int tst200_col_num = 9;	//0 ~ 8
	int tst200_row_num = 7;	//0 ~ 6

	int  written_bytes = 0 ;	/* & error check */

	uint8_t buf1[2]={0,};
	uint8_t buf2[32]={0,};

	uint16_t ref1[16]={0,};
	uint16_t ref2[16]={0,};

	int i,j,k;
	int ret;

	uint16_t RAWDATA_MAX[16] = {16383,16383,16383,16383,16383,16383,16383,16383,16383,16383,16383,16383,16383,16383,16383,16383};
	uint16_t RAWDATA_MIN[16] = {0,};
#if 0
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
#endif
    return sprintf(buf, "1"); // success
 }

static ssize_t tkey_rawcounter_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	u8 setLowLevelData[2] = {0x09, 0x04,};
	u8 read_buf[1] = {0,};
	u8 read_data_buf[50] = {0,};    
	int read_data_len,sensing_ch;
	int ret, i,j;
	int menuKey, backKey;

	menuKey = 0;
	backKey = 0;
	disable_irq(ts->client->irq);

	if(init_intensity)
	{
		//////////////////////
		// read Exciting CH.
		//////////////////////
		ret = melfas_i2c_read(ts->client, 0x2e, read_buf, 1);
		if(ret < 0) 
		{
			printk("[TSP] Exciting CH. melfas_i2c_read fail! %s : %d, \n", __func__,__LINE__);
			return 0;
		}
		g_exciting_ch = read_buf[0] + 2; // 15 + 2

		//////////////////////
		// read Sensing CH.
		//////////////////////
		ret = melfas_i2c_read(ts->client, 0x2f, read_buf, 1);
		if(ret < 0) 
		{
			printk("[TSP] Sensing CH. melfas_i2c_read fail! %s : %d, \n", __func__,__LINE__);
			return 0;
		}
		g_sensing_ch = read_buf[0]; // 10

		init_intensity = false;
	}


	////////////////////////
	// Writing Low Level Data(1)
	////////////////////////
//	ret = melfas_i2c_write(ts->client, setLowLevelData, 2);
	
	read_data_len = g_exciting_ch;
	sensing_ch	 = g_sensing_ch;

	for(i = 0; i < sensing_ch; i++ )
	{
		////////////////////////
		// Writing Low Level Data(2)
		////////////////////////
		ret = melfas_i2c_write(ts->client, setLowLevelData, 2);

		//////////////////////
		// Checking INT
		//////////////////////
		while(gpio_get_value(GPIO_TOUCH_INT)) // wait for Low
		{
			udelay(50);
//			printk("hunny : %s, GPIO_TOUCH_INT == high\n", __func__);
		}

//		printk("hunny : %s, GPIO_TOUCH_INT == LOW\n", __func__);

		udelay(300);

		//////////////////////
		// Read Data Buf
		//////////////////////
		ret = melfas_i2c_read(ts->client, 0xb2, read_data_buf, read_data_len);

		if(ret < 0)
	            printk("can't read Intensity Data %dth\n", i);

		udelay(5);
		
		if(i==0)
			menuKey = read_data_buf[read_data_len-1];
		else if(i==1)
			backKey = read_data_buf[read_data_len-1];
		else
				;
#if 0
		printk("[%d]:", i);	
		for(j=0;j<read_data_len;j++)
		{
			printk("[%02d],",read_data_buf[j]);
		}
		printk("\n");
#endif		
	}	

	enable_irq(ts->client->irq);
	
	printk("exciting_ch=[%d],sensing_ch=[%d],menuKey=[%02d],backKey=[%02d]\n", read_data_len, sensing_ch, menuKey, backKey) ;
	mdelay(1);
		
	return sprintf(buf, "%d %d", menuKey, backKey) ;
}

static ssize_t tkey_rawcounter_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	char *after;

	unsigned long value = simple_strtoul(buf, &after, 10);
	printk(KERN_INFO "[TSP] %s, %d\n", __func__, __LINE__);

	if(value == 0)
	{
		hrtimer_cancel(&ts->timer);
		tsp_reset();
//		prev_wdog_val = -1;
		hrtimer_start(&ts->timer, ktime_set(1, 0), HRTIMER_MODE_REL);
	//	hrtimer_start(&ts->timer, ktime_set(0, 200000000), HRTIMER_MODE_REL);
	}

	return size;
}

static int tsp_reset(void)
{

	if (ts->client->irq)
	{
		disable_irq(ts->client->irq);
	}

	gpio_direction_output( GPIO_TOUCH_INT , 0 );
	gpio_direction_output( GPIO_TSP_SCL , 0 ); 
	gpio_direction_output( GPIO_TSP_SDA , 0 ); 

	bcm_gpio_pull_up(GPIO_TOUCH_INT, false);
	bcm_gpio_pull_up_down_enable(GPIO_TOUCH_INT, true);
	touch_ctrl_regulator_mms128(TOUCH_OFF);

	msleep(200);

	gpio_direction_output( GPIO_TSP_SCL , 1 ); 
	gpio_direction_output( GPIO_TSP_SDA , 1 ); 
	//gpio_direction_output( TSP_INT , 1 ); 

	gpio_direction_input(GPIO_TOUCH_INT);
	bcm_gpio_pull_up(GPIO_TOUCH_INT, true);
	bcm_gpio_pull_up_down_enable(GPIO_TOUCH_INT, true);

#ifdef DELAY_BEFORE_VDD
    msleep(10);
#endif
	
	touch_ctrl_regulator_mms128(TOUCH_ON);

	msleep(70);

	enable_irq(ts->client->irq);
	
	return 1;
}





#if 0 //TSP_TEST_MODE
static ssize_t tsp_test_inspection_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    printk(KERN_DEBUG "Reference START %s\n", __func__) ;

    return sprintf(buf, "%5d, %5d, %5d, %5d, %5d, %5d, %5d, %5d, %5d, %5d \n",
                   tsp_test_inspection[inspec_y_channel_num][0], tsp_test_inspection[inspec_y_channel_num][1], tsp_test_inspection[inspec_y_channel_num][2], tsp_test_inspection[inspec_y_channel_num][3],
                   tsp_test_inspection[inspec_y_channel_num][4], tsp_test_inspection[inspec_y_channel_num][5], tsp_test_inspection[inspec_y_channel_num][6], tsp_test_inspection[inspec_y_channel_num][7],
                   tsp_test_inspection[inspec_y_channel_num][8], tsp_test_inspection[inspec_y_channel_num][9]);
}

static ssize_t tsp_test_inspection_store(struct device *dev, struct device_attribute *attr, char *buf, size_t size)
{
    unsigned int position;
    int j, i, ret;
    uint16_t ref_value;
    uint8_t buf1[2], buff[20];

    sscanf(buf, "%d\n", &position);

    if (position == 100)
    {
        inspec_test_cnt = 0;
        printk("reset_reference_value\n");
    }

    if (!inspec_test_cnt)
    {

        /* disable TSP_IRQ */
        disable_irq(ts->client->irq);
        for (i = 0;i < 14;i++)
        {
            for (j = 0;j < 10;j++)
            {
                buf1[0] = 0xA0 ;		/* register address */
                buf1[1] = 0x42 ;
                buf1[2] = i;
                buf1[3] = j;

                if (melfas_i2c_write(ts->client, buf1, 4) != 0)
                {
                    printk(KERN_DEBUG "Failed to enter testmode\n") ;
                }

                while (1)
                {
                    if (MCSDL_GPIO_RESETB_IS_HIGH() == 0) //TSP INT Low
                        break;
                }

                if (melfas_i2c_read(ts->client, 0xAE, buff, 1) != 0)
                {
                    printk(KERN_DEBUG "Failed to read(referece data)\n") ;
                }

                if (melfas_i2c_read(ts->client, 0xAF, buff, 2) != 0)
                {
                    printk(KERN_DEBUG "Failed to read(referece data)\n") ;
                }

                printk("ref value0=%x\n", buff[0]);
                printk("ref value1=%x\n", buff[1]);

                ref_value = (uint16_t)(buff[1] << 8) | buff[0] ;
                tsp_test_inspection[i][j] = ref_value;
                printk("ins value[%d]=%d\n", i, ref_value);
                inspec_test_cnt = 1;
            }
        }
        mcsdl_vdd_off();
        mdelay(50);
        mcsdl_vdd_on();
        mdelay(250);
        printk("[TOUCH] reset.\n");
        /* enable TSP_IRQ */
        enable_irq(ts->client->irq);
    }

    if (position < 0 || position > 14)
    {
        printk(KERN_DEBUG "Invalid values\n");
        return -EINVAL;
    }

    inspec_y_channel_num = (uint8_t)position;

    return size;
}

static DEVICE_ATTR(tsp_inspection, 0664, tsp_test_inspection_show, tsp_test_inspection_store);
#endif

MODULE_DESCRIPTION("Driver for Melfas MTSI Touchscreen Controller");
MODULE_AUTHOR("MinSang, Kim <kimms@melfas.com>");
MODULE_VERSION("0.1");
MODULE_LICENSE("GPL");

module_init(melfas_ts_init);
module_exit(melfas_ts_exit);
