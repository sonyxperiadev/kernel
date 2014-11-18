/*
 *  stk3x1x.c - Linux kernel modules for sensortek stk301x, stk321x and stk331x 
 *  proximity/ambient light sensor
 *
 *  Copyright (C) 2012~2013 Lex Hsieh / sensortek <lex_hsieh@sensortek.com.tw>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/irq.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/errno.h>
#include <linux/wakelock.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include   <linux/fs.h>   
#include  <asm/uaccess.h> 
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif

#define DRIVER_VERSION  "3.5.1.1"


/* Driver Settings */
#define CONFIG_STK_PS_ALS_USE_CHANGE_THRESHOLD
#ifdef CONFIG_STK_PS_ALS_USE_CHANGE_THRESHOLD
#define STK_ALS_CHANGE_THD	20	/* The threshold to trigger ALS interrupt, unit: lux */	
#endif	/* #ifdef CONFIG_STK_PS_ALS_USE_CHANGE_THRESHOLD */
#define STK_INT_PS_MODE			1	/* 1, 2, or 3	*/
//#define STK_POLL_PS
#define STK_POLL_ALS		/* ALS interrupt is valid only when STK_PS_INT_MODE = 1	or 4*/
#define STK_TUNE0
#define STK_DEBUG_PRINTF
//#define SPREADTRUM_PLATFORM
//Colby add for BroadCom start
//#define BROADCOM_PLATFORM
//Colby add for BroadCom end

//Colby add for Qualcomm device tree start
#define QUALCOMM_DEVICETREE_PLATFORM
//Colby add for Qualcomm device tree end
//#define STK_ALS_FIR
//#define STK_IRS

#ifdef SPREADTRUM_PLATFORM
	#include "stk3x1x.h"
//Colby add for BroadCom start
#elif defined(BROADCOM_PLATFORM)
	#include "stk3x1x.h"
//Colby add for BroadCom end
//Colby add for Qualcomm device tree start
#elif defined(QUALCOMM_DEVICETREE_PLATFORM)
	#include "linux/stk3x1x.h"
	#include <linux/regulator/consumer.h>
//Colby add for Qualcomm device tree end
#else
	#include "linux/stk3x1x.h"
#endif
/* Define Register Map */
#define STK_STATE_REG 			0x00
#define STK_PSCTRL_REG 			0x01
#define STK_ALSCTRL_REG 			0x02
#define STK_LEDCTRL_REG 			0x03
#define STK_INT_REG 				0x04
#define STK_WAIT_REG 			0x05
#define STK_THDH1_PS_REG 		0x06
#define STK_THDH2_PS_REG 		0x07
#define STK_THDL1_PS_REG 		0x08
#define STK_THDL2_PS_REG 		0x09
#define STK_THDH1_ALS_REG 		0x0A
#define STK_THDH2_ALS_REG 		0x0B
#define STK_THDL1_ALS_REG 		0x0C
#define STK_THDL2_ALS_REG 		0x0D
#define STK_FLAG_REG 			0x10
#define STK_DATA1_PS_REG	 	0x11
#define STK_DATA2_PS_REG 		0x12
#define STK_DATA1_ALS_REG 		0x13
#define STK_DATA2_ALS_REG 		0x14
#define STK_DATA1_OFFSET_REG 	0x15
#define STK_DATA2_OFFSET_REG 	0x16
#define STK_DATA1_IR_REG 		0x17
#define STK_DATA2_IR_REG 		0x18
#define STK_PDT_ID_REG 			0x3E
#define STK_RSRVD_REG 			0x3F
#define STK_SW_RESET_REG		0x80


/* Define state reg */
#define STK_STATE_EN_IRS_SHIFT  	7
#define STK_STATE_EN_AK_SHIFT  	6
#define STK_STATE_EN_ASO_SHIFT  	5
#define STK_STATE_EN_IRO_SHIFT  	4
#define STK_STATE_EN_WAIT_SHIFT  	2
#define STK_STATE_EN_ALS_SHIFT  	1
#define STK_STATE_EN_PS_SHIFT  	0

#define STK_STATE_EN_IRS_MASK	0x80
#define STK_STATE_EN_AK_MASK	0x40
#define STK_STATE_EN_ASO_MASK	0x20
#define STK_STATE_EN_IRO_MASK	0x10
#define STK_STATE_EN_WAIT_MASK	0x04
#define STK_STATE_EN_ALS_MASK	0x02
#define STK_STATE_EN_PS_MASK	0x01

/* Define PS ctrl reg */
#define STK_PS_PRS_SHIFT  		6
#define STK_PS_GAIN_SHIFT  		4
#define STK_PS_IT_SHIFT  			0

#define STK_PS_PRS_MASK			0xC0
#define STK_PS_GAIN_MASK			0x30
#define STK_PS_IT_MASK			0x0F

/* Define ALS ctrl reg */
#define STK_ALS_PRS_SHIFT  		6
#define STK_ALS_GAIN_SHIFT  		4
#define STK_ALS_IT_SHIFT  			0

#define STK_ALS_PRS_MASK		0xC0
#define STK_ALS_GAIN_MASK		0x30
#define STK_ALS_IT_MASK			0x0F
	
/* Define LED ctrl reg */
#define STK_LED_IRDR_SHIFT  		6
#define STK_LED_DT_SHIFT  		0

#define STK_LED_IRDR_MASK		0xC0
#define STK_LED_DT_MASK			0x3F
	
/* Define interrupt reg */
#define STK_INT_CTRL_SHIFT  		7
#define STK_INT_OUI_SHIFT  		4
#define STK_INT_ALS_SHIFT  		3
#define STK_INT_PS_SHIFT  			0

#define STK_INT_CTRL_MASK		0x80
#define STK_INT_OUI_MASK			0x10
#define STK_INT_ALS_MASK			0x08
#define STK_INT_PS_MASK			0x07

#define STK_INT_ALS				0x08

/* Define flag reg */
#define STK_FLG_ALSDR_SHIFT  		7
#define STK_FLG_PSDR_SHIFT  		6
#define STK_FLG_ALSINT_SHIFT  		5
#define STK_FLG_PSINT_SHIFT  		4
#define STK_FLG_OUI_SHIFT  		2
#define STK_FLG_IR_RDY_SHIFT  		1
#define STK_FLG_NF_SHIFT  		0

#define STK_FLG_ALSDR_MASK		0x80
#define STK_FLG_PSDR_MASK		0x40
#define STK_FLG_ALSINT_MASK		0x20
#define STK_FLG_PSINT_MASK		0x10
#define STK_FLG_OUI_MASK			0x04
#define STK_FLG_IR_RDY_MASK		0x02
#define STK_FLG_NF_MASK			0x01
	
/* misc define */
#define MIN_ALS_POLL_DELAY_NS	110000000

#define STK2213_PID			0x23
#define STK2213I_PID			0x22
#define STK3010_PID			0x33
#define STK3210_STK3310_PID	0x13
#define STK3211_STK3311_PID	0x12


#ifdef STK_TUNE0
	#define STK_MAX_MIN_DIFF	500
	#define STK_LT_N_CT	75
	#define STK_HT_N_CT	130
	//STK add for CCI start 20130112
	#define MAX_COMPARE(a,b) (a>b)? a:b
	
	unsigned int cci_ps_high_thd;
	unsigned int cci_ps_low_thd;
	//STK add for CCI end 20130112	
	//add for FTM interrupt check 20130424 start
	int CCI_FTM_INTERRUPT = 0;
	//add for FTM interrupt check 20130424 end
	// CCI add First boot H/L thd for debug start
	int FIRST_BOOT_H_THD = 0;
	int FIRST_BOOT_L_THD = 0;
	// CCI add First boot H/L thd for debug end
#endif	/* #ifdef STK_TUNE0 */

#define STK_IRC_MAX_ALS_CODE		20000
#define STK_IRC_MIN_ALS_CODE		25
#define STK_IRC_MIN_IR_CODE		50
#define STK_IRC_ALS_DENOMI		2		
#define STK_IRC_ALS_NUMERA		5
#define STK_IRC_ALS_CORREC		748

#define DEVICE_NAME		"stk_ps"
#define ALS_NAME "lightsensor-level"
#define PS_NAME "proximity"

#ifdef SPREADTRUM_PLATFORM
extern int sprd_3rdparty_gpio_pls_irq;

static struct stk3x1x_platform_data stk3x1x_pfdata={ 
  .state_reg = 0x0,    /* disable all */ 
  .psctrl_reg = 0x71,    /* ps_persistance=4, ps_gain=64X, PS_IT=0.391ms */ 
  .alsctrl_reg = 0x38, 	/* als_persistance=1, als_gain=64X, ALS_IT=50ms */
  .ledctrl_reg = 0xFF,   /* 100mA IRDR, 64/64 LED duty */ 
  .wait_reg = 0x07,    /* 50 ms */   
  .ps_thd_h =1700, 
  .ps_thd_l = 1500, 
  .int_pin = sprd_3rdparty_gpio_pls_irq,  
  .transmittance = 500, 
}; 
#endif

//Colby add for BroadCom start
#ifdef BROADCOM_PLATFORM

static struct stk3x1x_platform_data stk3x1x_pfdata_BCM={ 
  .state_reg = 0x0,    /* disable all */ 
  .psctrl_reg = 0x33,    /* ps_persistance=1, ps_gain=64X, PS_IT=1.48ms */
  .alsctrl_reg = 0x39,  /* als_persistance=1, als_gain=64X, ALS_IT=50ms */
  .ledctrl_reg = 0xFF,   /* 50mA IRDR, 64/64 LED duty */ 
  .wait_reg = 0x09,    /* 59.3 ms */ 
  .ps_thd_h =1700, 
  .ps_thd_l = 1500, 
  .int_pin = 89,  
  .transmittance = 1112, 
}; 

//Colby add for Qualcomm device tree start
#elif defined(QUALCOMM_DEVICETREE_PLATFORM)

static struct stk3x1x_platform_data stk3x1x_pfdata_QCT={ 
  .state_reg = 0x0,    /* disable all */ 
  .psctrl_reg = 0x33,    /* ps_persistance=1, ps_gain=64X, PS_IT=1.48ms */
  .alsctrl_reg = 0x39,  /* als_persistance=1, als_gain=64X, ALS_IT=50ms */
  .ledctrl_reg = 0xFF,   /* 50mA IRDR, 64/64 LED duty */ 
  .wait_reg = 0x09,    /* 59.3 ms */ 
  .ps_thd_h =1700, 
  .ps_thd_l = 1500, 
  .int_pin = 65,  
  .transmittance = 1112, 
};

static struct of_device_id stk3x1x_dts_table[] = {
		{ .compatible  = "qcom,stk_ps",},
		{ },
};
//Colby add for Qualcomm device tree end

#endif
//Colby add for BroadCom end

#ifdef STK_ALS_FIR
struct data_filter {
    s16 raw[8];
    int sum;
    int number;
    int idx;
};
#endif

struct stk3x1x_data {
	struct i2c_client *client;
#if (!defined(STK_POLL_PS) || !defined(STK_POLL_ALS))
    int32_t irq;
    struct work_struct stk_work;
	struct workqueue_struct *stk_wq;	
#endif	
	uint16_t ir_code;
	uint16_t als_correct_factor;
	uint8_t alsctrl_reg;
	uint8_t psctrl_reg;
	int		int_pin;
	uint8_t wait_reg;
	uint8_t int_reg;
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend stk_early_suspend;
#endif	
	uint16_t ps_thd_h;
	uint16_t ps_thd_l;
	struct mutex io_lock;
	struct input_dev *ps_input_dev;
	int32_t ps_distance_last;
	bool ps_enabled;
	struct wake_lock ps_wakelock;	
#ifdef STK_POLL_PS		
	struct hrtimer ps_timer;	
    struct work_struct stk_ps_work;
	struct workqueue_struct *stk_ps_wq;
	struct wake_lock ps_nosuspend_wl;		
	int debounce_counter;
#endif
	struct input_dev *als_input_dev;
	int32_t als_lux_last;
	uint32_t als_transmittance;	
	bool als_enabled;
	bool re_enable_als;
	ktime_t ps_poll_delay;
	ktime_t als_poll_delay;
#ifdef STK_POLL_ALS		
    struct work_struct stk_als_work;
	struct hrtimer als_timer;	
	struct workqueue_struct *stk_als_wq;
#endif	
	bool first_boot;
#ifdef STK_TUNE0
	uint16_t psa;
	uint16_t psi;	
	uint16_t psi_set;	
	struct hrtimer ps_tune0_timer;	
	struct workqueue_struct *stk_ps_tune0_wq;
    struct work_struct stk_ps_tune0_work;
	ktime_t ps_tune0_delay;
	bool tune_zero_init_proc;
	uint32_t ps_stat_data[3];
	int data_count;
#endif	
#ifdef STK_ALS_FIR
	struct data_filter      fir;
#endif
};

#if( !defined(CONFIG_STK_PS_ALS_USE_CHANGE_THRESHOLD))
static uint32_t lux_threshold_table[] =
{
	3,
	10,
	40,
	65,
	145,
	300,
	550,
	930,
	1250,
	1700,
};

#define LUX_THD_TABLE_SIZE (sizeof(lux_threshold_table)/sizeof(uint32_t)+1)
static uint16_t code_threshold_table[LUX_THD_TABLE_SIZE+1];
#endif 	

//CCI Colby add start 20130111
struct stk3x1x_threshold {
	unsigned  int result_ct;
	unsigned  int result_ps_cover;
	unsigned  int result_als_value;
	unsigned  int ps_high_thd;
	unsigned  int ps_low_thd;
};
unsigned int cci_result_ct;
unsigned int cci_ps_cover;
unsigned int cci_als_value;
unsigned int cci_als_value_cali; //lux value for ALS calibration
unsigned int cci_transmittance_cali; // transmittance for ALS calibration
//CCI Colby add end 20130111

static int32_t stk3x1x_enable_ps(struct stk3x1x_data *ps_data, uint8_t enable);
static int32_t stk3x1x_enable_als(struct stk3x1x_data *ps_data, uint8_t enable);
static int32_t stk3x1x_set_ps_thd_l(struct stk3x1x_data *ps_data, uint16_t thd_l);
static int32_t stk3x1x_set_ps_thd_h(struct stk3x1x_data *ps_data, uint16_t thd_h);
static int32_t stk3x1x_set_als_thd_l(struct stk3x1x_data *ps_data, uint16_t thd_l);
static int32_t stk3x1x_set_als_thd_h(struct stk3x1x_data *ps_data, uint16_t thd_h);
//static int32_t stk3x1x_set_ps_aoffset(struct stk3x1x_data *ps_data, uint16_t offset);
static int32_t stk3x1x_get_ir_reading(struct stk3x1x_data *ps_data);

#ifdef STK_TUNE0
static int stk_ps_tune_zero_func_fae(struct stk3x1x_data *ps_data);
#endif

static int stk3x1x_i2c_read_data(struct i2c_client *client, unsigned char command, int length, unsigned char *values)
{
	uint8_t retry;	
	int err;
	struct i2c_msg msgs[] = 
	{
		{
			.addr = client->addr,
			.flags = 0,
			.len = 1,
			.buf = &command,
		},
		{
			.addr = client->addr,
			.flags = I2C_M_RD,
			.len = length,
			.buf = values,
		},
	};
	
	for (retry = 0; retry < 5; retry++) 
	{
		err = i2c_transfer(client->adapter, msgs, 2);
		if (err == 2)
			break;
		else
			mdelay(5);
	}
	
	if (retry >= 5) 
	{
		printk(KERN_ERR "%s: i2c read fail, err=%d\n", __func__, err);
		return -EIO;
	} 
	return 0;		
}

static int stk3x1x_i2c_write_data(struct i2c_client *client, unsigned char command, int length, unsigned char *values)
{
	int retry;
	int err;	
	unsigned char data[11];
	struct i2c_msg msg;
	int index;

    if (!client)
		return -EINVAL;
    else if (length >= 10) 
	{        
        printk(KERN_ERR "%s:length %d exceeds 10\n", __func__, length);
        return -EINVAL;
    }   	
	
	data[0] = command;
	for (index=1;index<=length;index++)
		data[index] = values[index-1];	
	
	msg.addr = client->addr;
	msg.flags = 0;
	msg.len = length+1;
	msg.buf = data;
	
	for (retry = 0; retry < 5; retry++) 
	{
		err = i2c_transfer(client->adapter, &msg, 1);
		if (err == 1)
			break;
		else
			mdelay(5);
	}
	
	if (retry >= 5) 
	{
		printk(KERN_ERR "%s: i2c write fail, err=%d\n", __func__, err);		
		return -EIO;
	}
	return 0;
}

static int stk3x1x_i2c_smbus_read_byte_data(struct i2c_client *client, unsigned char command)
{
	unsigned char value;
	int err;
	err = stk3x1x_i2c_read_data(client, command, 1, &value);
	if(err < 0)
		return err;
	return value;
}

static int stk3x1x_i2c_smbus_write_byte_data(struct i2c_client *client, unsigned char command, unsigned char value)
{
	int err;
	err = stk3x1x_i2c_write_data(client, command, 1, &value);
	return err;
}

inline uint32_t stk_alscode2lux(struct stk3x1x_data *ps_data, uint32_t alscode)
{
	alscode += ((alscode<<7)+(alscode<<3)+(alscode>>1));   
    alscode<<=3; 
    alscode/=ps_data->als_transmittance;
	return alscode;
}

inline uint32_t stk_lux2alscode(struct stk3x1x_data *ps_data, uint32_t lux)
{
    lux*=ps_data->als_transmittance;
    lux/=1100;
    if (unlikely(lux>=(1<<16)))
        lux = (1<<16) -1;
    return lux;
}

#ifndef CONFIG_STK_PS_ALS_USE_CHANGE_THRESHOLD
static void stk_init_code_threshold_table(struct stk3x1x_data *ps_data)
{
    uint32_t i,j;
    uint32_t alscode;

    code_threshold_table[0] = 0;
#ifdef STK_DEBUG_PRINTF	
    printk(KERN_INFO "alscode[0]=%d\n",0);
#endif	
    for (i=1,j=0;i<LUX_THD_TABLE_SIZE;i++,j++)
    {
        alscode = stk_lux2alscode(ps_data, lux_threshold_table[j]);
        printk(KERN_INFO "alscode[%d]=%d\n",i,alscode);
        code_threshold_table[i] = (uint16_t)(alscode);
    }
    code_threshold_table[i] = 0xffff;
    printk(KERN_INFO "alscode[%d]=%d\n",i,alscode);
}

static uint32_t stk_get_lux_interval_index(uint16_t alscode)
{
    uint32_t i;
    for (i=1;i<=LUX_THD_TABLE_SIZE;i++)
    {
        if ((alscode>=code_threshold_table[i-1])&&(alscode<code_threshold_table[i]))
        {
            return i;
        }
    }
    return LUX_THD_TABLE_SIZE;
}
#else
inline void stk_als_set_new_thd(struct stk3x1x_data *ps_data, uint16_t alscode)
{
    int32_t high_thd,low_thd;
    high_thd = alscode + stk_lux2alscode(ps_data, STK_ALS_CHANGE_THD);
    low_thd = alscode - stk_lux2alscode(ps_data, STK_ALS_CHANGE_THD);
    if (high_thd >= (1<<16))
        high_thd = (1<<16) -1;
    if (low_thd <0)
        low_thd = 0;
    stk3x1x_set_als_thd_h(ps_data, (uint16_t)high_thd);
    stk3x1x_set_als_thd_l(ps_data, (uint16_t)low_thd);
}
#endif // CONFIG_STK_PS_ALS_USE_CHANGE_THRESHOLD


static int32_t stk3x1x_init_all_reg(struct stk3x1x_data *ps_data, struct stk3x1x_platform_data *plat_data)
{
	int32_t ret;
	uint8_t w_reg;
	
	w_reg = plat_data->state_reg;
    ret = stk3x1x_i2c_smbus_write_byte_data(ps_data->client, STK_STATE_REG, w_reg);
    if (ret < 0)
    {
        printk(KERN_ERR "%s: write i2c error\n", __func__);
        return ret;
    }		

	ps_data->psctrl_reg = plat_data->psctrl_reg;
#ifdef STK_POLL_PS		
	ps_data->psctrl_reg &= 0x3F;
#endif	
    ret = stk3x1x_i2c_smbus_write_byte_data(ps_data->client, STK_PSCTRL_REG, ps_data->psctrl_reg);
    if (ret < 0)
    {
        printk(KERN_ERR "%s: write i2c error\n", __func__);
        return ret;
    }	
	ps_data->alsctrl_reg = plat_data->alsctrl_reg;
    ret = stk3x1x_i2c_smbus_write_byte_data(ps_data->client, STK_ALSCTRL_REG, ps_data->alsctrl_reg);
    if (ret < 0)
    {
        printk(KERN_ERR "%s: write i2c error\n", __func__);
        return ret;
    }		
	w_reg = plat_data->ledctrl_reg;
    ret = stk3x1x_i2c_smbus_write_byte_data(ps_data->client, STK_LEDCTRL_REG, w_reg);
    if (ret < 0)
    {
        printk(KERN_ERR "%s: write i2c error\n", __func__);
        return ret;
    }	
	ps_data->wait_reg = plat_data->wait_reg;
	
	if(ps_data->wait_reg < 2)
	{
		printk(KERN_WARNING "%s: wait_reg should be larger than 2, force to write 2\n", __func__);
		ps_data->wait_reg = 2;
	}
	else if (ps_data->wait_reg > 0xFF)
	{
		printk(KERN_WARNING "%s: wait_reg should be less than 0xFF, force to write 0xFF\n", __func__);
		ps_data->wait_reg = 0xFF;		
	}
	w_reg = plat_data->wait_reg;
    ret = stk3x1x_i2c_smbus_write_byte_data(ps_data->client, STK_WAIT_REG, w_reg);
    if (ret < 0)
    {
        printk(KERN_ERR "%s: write i2c error\n", __func__);
        return ret;
    }	
#ifdef STK_TUNE0	
	ps_data->psa = 0x0;
	ps_data->psi = 0xFFFF;	
#else
	ps_data->ps_thd_h = plat_data->ps_thd_h;
	ps_data->ps_thd_l = plat_data->ps_thd_l;	
	stk3x1x_set_ps_thd_h(ps_data, ps_data->ps_thd_h);
	stk3x1x_set_ps_thd_l(ps_data, ps_data->ps_thd_l);	
#endif	
	w_reg = 0;
#ifndef STK_POLL_PS	
	w_reg |= STK_INT_PS_MODE;	
#else
	w_reg |= 0x01;		
#endif	

#if (!defined(STK_POLL_ALS) && (STK_INT_PS_MODE != 0x02) && (STK_INT_PS_MODE != 0x03))
	w_reg |= STK_INT_ALS;
#endif	
	ps_data->int_reg = w_reg;
    ret = stk3x1x_i2c_smbus_write_byte_data(ps_data->client, STK_INT_REG, ps_data->int_reg);
    if (ret < 0)
	{
		printk(KERN_ERR "%s: write i2c error\n", __func__);
		return ret;
	}		
	return 0;	
}
	

static int32_t stk3x1x_check_pid(struct stk3x1x_data *ps_data)
{
	int32_t err1, err2;
	
	err1 = stk3x1x_i2c_smbus_read_byte_data(ps_data->client,STK_PDT_ID_REG);
	if (err1 < 0)
	{
		printk(KERN_ERR "%s: read i2c error, err=%d\n", __func__, err1);
		return err1;
	}
	
    err2 = stk3x1x_i2c_smbus_read_byte_data(ps_data->client,STK_RSRVD_REG);
    if (err2 < 0)
    {
        printk(KERN_ERR "%s: read i2c error, err=%d\n", __func__, err2);
        return err2;
    }	
	printk(KERN_INFO "%s: PID=0x%x, RID=0x%x\n", __func__, err1, err2);
	if(err2 == 0xC0)
		printk(KERN_INFO "%s: RID=0xC0!!!!!!!!!!!!!\n", __func__);	

	switch(err1)
	{
		case STK2213_PID:
		case STK2213I_PID:
		case STK3010_PID:
		case STK3210_STK3310_PID:
		case STK3211_STK3311_PID:
			return 0;
		case 0x0:
			printk(KERN_ERR "PID=0x0, please make sure the chip is stk3x1x!\n");
			return -2;			
		default:
			printk(KERN_ERR "%s: invalid PID(%#x)\n", __func__, err1);	
			return -1;
	}
		
	return 0;	
}


static int32_t stk3x1x_software_reset(struct stk3x1x_data *ps_data)
{
    int32_t r;
    uint8_t w_reg;
	
    w_reg = 0x7F;
    r = stk3x1x_i2c_smbus_write_byte_data(ps_data->client,STK_WAIT_REG,w_reg);
    if (r<0)
    {
        printk(KERN_ERR "%s: software reset: write i2c error, ret=%d\n", __func__, r);
        return r;
    }
    r = stk3x1x_i2c_smbus_read_byte_data(ps_data->client,STK_WAIT_REG);
    if (w_reg != r)
    {
        printk(KERN_ERR "%s: software reset: read-back value is not the same\n", __func__);
        return -1;
    }
	
    r = stk3x1x_i2c_smbus_write_byte_data(ps_data->client,STK_SW_RESET_REG,0);
    if (r<0)
    {
        printk(KERN_ERR "%s: software reset: read error after reset\n", __func__);
        return r;
    }
    msleep(1);
    return 0;
}


static int32_t stk3x1x_set_als_thd_l(struct stk3x1x_data *ps_data, uint16_t thd_l)
{
	unsigned char val[2];
	val[0] = (thd_l & 0xFF00) >> 8;
	val[1] = thd_l & 0x00FF;
	return stk3x1x_i2c_write_data(ps_data->client, STK_THDL1_ALS_REG, 2, val);
}
static int32_t stk3x1x_set_als_thd_h(struct stk3x1x_data *ps_data, uint16_t thd_h)
{
	unsigned char val[2];
	val[0] = (thd_h & 0xFF00) >> 8;
	val[1] = thd_h & 0x00FF;
	return stk3x1x_i2c_write_data(ps_data->client, STK_THDH1_ALS_REG, 2, val);		
}

static int32_t stk3x1x_set_ps_thd_l(struct stk3x1x_data *ps_data, uint16_t thd_l)
{
	unsigned char val[2];
	val[0] = (thd_l & 0xFF00) >> 8;
	val[1] = thd_l & 0x00FF;
	return stk3x1x_i2c_write_data(ps_data->client, STK_THDL1_PS_REG, 2, val);		
}
static int32_t stk3x1x_set_ps_thd_h(struct stk3x1x_data *ps_data, uint16_t thd_h)
{	
	unsigned char val[2];
	val[0] = (thd_h & 0xFF00) >> 8;
	val[1] = thd_h & 0x00FF;
	return stk3x1x_i2c_write_data(ps_data->client, STK_THDH1_PS_REG, 2, val);		
}

static inline uint32_t stk3x1x_get_ps_reading(struct stk3x1x_data *ps_data)
{	
	unsigned char value[2];
	int err;
	err = stk3x1x_i2c_read_data(ps_data->client, STK_DATA1_PS_REG, 2, &value[0]);
	if(err < 0)
		return err;
	return ((value[0]<<8) | value[1]);	
}


static int32_t stk3x1x_set_flag(struct stk3x1x_data *ps_data, uint8_t org_flag_reg, uint8_t clr)
{
	uint8_t w_flag;
	w_flag = org_flag_reg | (STK_FLG_ALSINT_MASK | STK_FLG_PSINT_MASK | STK_FLG_OUI_MASK | STK_FLG_IR_RDY_MASK);
	w_flag &= (~clr);
	//printk(KERN_INFO "%s: org_flag_reg=0x%x, w_flag = 0x%x\n", __func__, org_flag_reg, w_flag);		
    return stk3x1x_i2c_smbus_write_byte_data(ps_data->client,STK_FLAG_REG, w_flag);	
}

static int32_t stk3x1x_get_flag(struct stk3x1x_data *ps_data)
{	
    return stk3x1x_i2c_smbus_read_byte_data(ps_data->client,STK_FLAG_REG);	
}

static int32_t stk3x1x_enable_ps(struct stk3x1x_data *ps_data, uint8_t enable)
{
    int32_t ret;
	uint8_t w_state_reg;
	uint8_t curr_ps_enable;	
	uint32_t reading;
	int32_t near_far_state;		
	// CCI read H_thd/L_thd when PS enable/disable start
	int32_t ps_reg[4];
	uint8_t cnt;
	uint8_t i = 0;
	// CCI read H_thd/L_thd when PS enable/disable end
	//STK add for CCI start 20130430 dust issue for variation in 2 consequential calls start
	unsigned int hang_up_hthd, hang_up_lthd;
	//int ret;

	//STK add for CCI end 20130430 dust issue for variation in 2 consequential calls end
	curr_ps_enable = ps_data->ps_enabled?1:0;	
	if(curr_ps_enable == enable)
		return 0;
	
#ifdef STK_TUNE0				
	if (!(ps_data->psi_set) && !enable)
	{
		hrtimer_cancel(&ps_data->ps_tune0_timer);					
		cancel_work_sync(&ps_data->stk_ps_tune0_work);
	}
#endif		
	
    ret = stk3x1x_i2c_smbus_read_byte_data(ps_data->client, STK_STATE_REG);
    if (ret < 0)
    {
		printk(KERN_ERR "%s: write i2c error, ret=%d\n", __func__, ret);		
		return ret;
    }	
	w_state_reg = ret;
#ifdef STK_TUNE0
	if(ps_data->first_boot == true)
	{		
		ps_data->first_boot = false;
		//ps_data->ps_thd_h = 0xFFFF;
		//ps_data->ps_thd_l = 0;	
		//ps_data->psa = 0x0;
		//ps_data->psi = 0xFFFF;						
		//stk3x1x_set_ps_thd_h(ps_data, ps_data->ps_thd_h);
		//stk3x1x_set_ps_thd_l(ps_data, ps_data->ps_thd_l);		
	}
#else
	if(ps_data->first_boot == true)
	{			
		ps_data->first_boot = false;	
		//stk3x1x_set_ps_thd_h(ps_data, ps_data->ps_thd_h);
		//stk3x1x_set_ps_thd_l(ps_data, ps_data->ps_thd_l);		
	}
#endif

	
	w_state_reg &= ~(STK_STATE_EN_PS_MASK | STK_STATE_EN_WAIT_MASK | STK_STATE_EN_AK_MASK); 
	if(enable)	
	{
		w_state_reg |= STK_STATE_EN_PS_MASK;	
		if(!(ps_data->als_enabled))
			w_state_reg |= STK_STATE_EN_WAIT_MASK;
	}
    ret = stk3x1x_i2c_smbus_write_byte_data(ps_data->client, STK_STATE_REG, w_state_reg);
    if (ret < 0)
	{
		printk(KERN_ERR "%s: write i2c error, ret=%d\n", __func__, ret);		
		return ret;
	}

	// CCI read H_thd/L_thd when PS enable/disable start
	for(cnt=6;cnt<=9;cnt++){
		ps_reg[i] = stk3x1x_i2c_smbus_read_byte_data(ps_data->client, (cnt));
		if(ps_reg[i] < 0){
			printk(KERN_ERR "%s fail, ret=%d", __func__, ps_reg[i]);	
			}
		//else{
			//printk(KERN_INFO "reg[0x%2X]=0x%2X\n", cnt, ps_reg[i]);
			//}
		i++;
		}
	printk(KERN_ERR "%s: stk3x1x_enable_ps() read H/L THD from register before auto cal => H_THD = %2X, %2X, L_THD = %2X, %2X\n", __func__, ps_reg[0], ps_reg[1], ps_reg[2], ps_reg[3]);
	// CCI read H_thd/L_thd when PS enable/disable end

    if(enable)
	{
	// CCI add First boot H/L thd for debug start
	printk(KERN_ERR "%s: [CCI]stk3x1x_enable_ps() show first boot H/L THD => FIRST_BOOT_H_THD = %d, FIRST_BOOT_L_THD = %d\n", __func__, FIRST_BOOT_H_THD, FIRST_BOOT_L_THD);
	printk(KERN_ERR "%s: [CCI]stk3x1x_enable_ps() show factory H/L THD => cci_ps_high_thd = %d, cci_ps_low_thd = %d, cci_result_ct = %d\n", __func__, cci_ps_high_thd, cci_ps_low_thd, cci_result_ct);
	// CCI add First boot H/L thd for debug end
#ifdef STK_TUNE0		
		//CCI Colby add for default ps_thd_h and ps_thd_l 20130313 start
		ps_data->psi_set = 0;
		ps_data->psa = 0;
		ps_data->psi = 0xFFFF;
		printk(KERN_ERR "%s: [CCI]set psi_set = 0, psa = 0, psi = 0xFFFF\n", __func__);
		//CCI Colby add for default ps_thd_h and ps_thd_l 20130313 end
		if (!(ps_data->psi_set))
			hrtimer_start(&ps_data->ps_tune0_timer, ps_data->ps_tune0_delay, HRTIMER_MODE_REL);			
#endif
		
#ifdef STK_POLL_PS		
		hrtimer_start(&ps_data->ps_timer, ps_data->ps_poll_delay, HRTIMER_MODE_REL);	
		ps_data->ps_distance_last = -1;	
#endif		
#ifndef STK_POLL_PS
	#ifndef STK_POLL_ALS		
		if(!(ps_data->als_enabled))
	#endif	/* #ifndef STK_POLL_ALS	*/
			enable_irq(ps_data->irq);
#endif	/* #ifndef STK_POLL_PS */						
		ps_data->ps_enabled = true;
		msleep(4); //STK change for waiting PS ready
		ret = stk3x1x_get_flag(ps_data);
		if (ret < 0)
		{
			printk(KERN_ERR "%s: read i2c error, ret=%d\n", __func__, ret);
			return ret;
		}			
		near_far_state = ret & STK_FLG_NF_MASK;					
		ps_data->ps_distance_last = near_far_state;
		input_report_abs(ps_data->ps_input_dev, ABS_DISTANCE, near_far_state);
		input_sync(ps_data->ps_input_dev);
		wake_lock_timeout(&ps_data->ps_wakelock, 3*HZ);
		reading = stk3x1x_get_ps_reading(ps_data);
		printk(KERN_INFO "%s: ps input event=%d, ps code = %d\n",__func__, near_far_state, reading);				
	}
	else
	{	
	//STK add for CCI start 20130430 dust issue for variation in 2 consequential calls 
	hang_up_hthd = ps_data->ps_thd_h;
	hang_up_lthd = ps_data->ps_thd_l;
	
	if(cci_ps_low_thd==0 || cci_ps_high_thd==0)
	{
		//if((2*cci_result_ct-STK_LT_N_CT) > hang_up_hthd)
		//{
		//	printk(KERN_ERR "%s: [CCI] use STK default value\n", __func__);		
		//	ps_data->ps_thd_h = 2*cci_result_ct-STK_LT_N_CT;
		//	ps_data->ps_thd_l = 2*cci_result_ct-STK_HT_N_CT;
		//}
		//else
		//{	
			printk(KERN_ERR "%s: [CCI] use STK default value\n", __func__);
			ps_data->ps_thd_h = hang_up_hthd;
			ps_data->ps_thd_l = hang_up_lthd;
		//}
	}
	else
	{
		if((2*cci_result_ct+(cci_ps_high_thd-cci_result_ct)) > hang_up_hthd)
		{
			printk(KERN_ERR "%s: [CCI] use CCI calibration value: cci_ps_high_thd = %d, cci_ps_low_thd = %d\n", __func__, cci_ps_high_thd, cci_ps_low_thd);
			ps_data->ps_thd_h = 2*cci_result_ct + (cci_ps_high_thd-cci_result_ct);
			ps_data->ps_thd_l = 2*cci_result_ct + (cci_ps_low_thd-cci_result_ct);
		}
		else
		{
			printk(KERN_ERR "%s: [CCI] use CCI calibration value: cci_ps_high_thd = %d, cci_ps_low_thd = %d\n", __func__, cci_ps_high_thd, cci_ps_low_thd);
			ps_data->ps_thd_h = hang_up_hthd;
			ps_data->ps_thd_l = hang_up_lthd;
		}
	}
	printk(KERN_ERR "%s: [CCI] write H/L thd when ps disable: ps_thd_h = %d, ps_thd_l = %d\n", __func__, ps_data->ps_thd_h, ps_data->ps_thd_l);
	stk3x1x_set_ps_thd_h(ps_data, ps_data->ps_thd_h);
	stk3x1x_set_ps_thd_l(ps_data, ps_data->ps_thd_l);	
	//STK add for CCI end 20130430 dust issue for variation in 2 consequential calls 

#ifdef STK_POLL_PS
		hrtimer_cancel(&ps_data->ps_timer);
		cancel_work_sync(&ps_data->stk_ps_work);
#else		
#ifndef STK_POLL_ALS
		if(!(ps_data->als_enabled))	
#endif				
			disable_irq(ps_data->irq);
#endif
		ps_data->ps_enabled = false;		
	}
	return ret;
}

static int32_t stk3x1x_enable_als(struct stk3x1x_data *ps_data, uint8_t enable)
{
    int32_t ret;
	uint8_t w_state_reg;
	uint8_t curr_als_enable = (ps_data->als_enabled)?1:0;
	
	if(curr_als_enable == enable)
		return 0;
	
#ifdef STK_IRS
	if(enable && !(ps_data->ps_enabled))
	{		
		ret = stk3x1x_get_ir_reading(ps_data);
		if(ret > 0)
			ps_data->ir_code = ret;
	}		
#endif

#ifndef STK_POLL_ALS			
    if (enable)
	{				
        stk3x1x_set_als_thd_h(ps_data, 0x0000);
        stk3x1x_set_als_thd_l(ps_data, 0xFFFF);		
	}
#endif	
    ret = stk3x1x_i2c_smbus_read_byte_data(ps_data->client, STK_STATE_REG);
    if (ret < 0)
    {
        printk(KERN_ERR "%s: write i2c error\n", __func__);
		return ret;
    }		
	w_state_reg = (uint8_t)(ret & (~(STK_STATE_EN_ALS_MASK | STK_STATE_EN_WAIT_MASK))); 
	if(enable)	
		w_state_reg |= STK_STATE_EN_ALS_MASK;	
	else if (ps_data->ps_enabled)		
		w_state_reg |= STK_STATE_EN_WAIT_MASK;	

	
    ret = stk3x1x_i2c_smbus_write_byte_data(ps_data->client, STK_STATE_REG, w_state_reg);
    if (ret < 0)
	{
		printk(KERN_ERR "%s: write i2c error\n", __func__);
		return ret;
	}
	
    if (enable)
    {						
		ps_data->als_enabled = true;
#ifdef STK_POLL_ALS			
		hrtimer_start(&ps_data->als_timer, ps_data->als_poll_delay, HRTIMER_MODE_REL);		
#else
#ifndef STK_POLL_PS
		if(!(ps_data->ps_enabled))
#endif		
			enable_irq(ps_data->irq);
#endif		
    }
	else
	{
		ps_data->als_enabled = false;
#ifdef STK_POLL_ALS			
		hrtimer_cancel(&ps_data->als_timer);
		cancel_work_sync(&ps_data->stk_als_work);
#else
#ifndef STK_POLL_PS
		if(!(ps_data->ps_enabled))	
#endif		
			disable_irq(ps_data->irq);		
#endif		
	}
    return ret;
}

static inline int32_t stk3x1x_get_als_reading(struct stk3x1x_data *ps_data)
{
    int32_t word_data;
#ifdef STK_ALS_FIR
	int index;   
#endif
	unsigned char value[2];
	int ret;
	
	ret = stk3x1x_i2c_read_data(ps_data->client, STK_DATA1_ALS_REG, 2, &value[0]);
	if(ret < 0)
	{
		printk(KERN_ERR "%s fail, ret=0x%x", __func__, ret);
		return ret;
	}
	word_data = (value[0]<<8) | value[1];	
	
#ifdef STK_ALS_FIR
	if(ps_data->fir.number < 8)
	{                
		ps_data->fir.raw[ps_data->fir.number] = word_data;
		ps_data->fir.sum += word_data;
		ps_data->fir.number++;
		ps_data->fir.idx++;
	}
	else
	{
		index = ps_data->fir.idx % 8;
		ps_data->fir.sum -= ps_data->fir.raw[index];
		ps_data->fir.raw[index] = word_data;
		ps_data->fir.sum += word_data;
		ps_data->fir.idx++;
		word_data = ps_data->fir.sum/8;
	}	
#endif	
	
	return word_data;
}

static int32_t stk3x1x_set_irs_it_slp(struct stk3x1x_data *ps_data, uint16_t *slp_time)
{
	uint8_t irs_alsctrl;
	int32_t ret;
		
	irs_alsctrl = (ps_data->alsctrl_reg & 0x0F) - 2;		
	switch(irs_alsctrl)
	{
		case 6:
			*slp_time = 12;
			break;
		case 7:
			*slp_time = 24;			
			break;
		case 8:
			*slp_time = 48;			
			break;
		case 9:
			*slp_time = 96;			
			break;				
		default:
			printk(KERN_ERR "%s: unknown ALS IT=0x%x\n", __func__, irs_alsctrl);
			ret = -EINVAL;	
			return ret;
	}
	irs_alsctrl |= (ps_data->alsctrl_reg & 0xF0);
	ret = stk3x1x_i2c_smbus_write_byte_data(ps_data->client, STK_ALSCTRL_REG, irs_alsctrl);
	if (ret < 0)
	{
		printk(KERN_ERR "%s: write i2c error\n", __func__);
		return ret;		
	}		
	return 0;
}

static int32_t stk3x1x_get_ir_reading(struct stk3x1x_data *ps_data)
{
    int32_t word_data, ret;
	uint8_t w_reg, retry = 0;	
	uint16_t irs_slp_time = 100;
	bool re_enable_ps = false;
	unsigned char value[2];
	
	if(ps_data->ps_enabled)
	{
#ifdef STK_TUNE0		
		if (!(ps_data->psi_set))
		{
			hrtimer_cancel(&ps_data->ps_tune0_timer);					
			cancel_work_sync(&ps_data->stk_ps_tune0_work);
		}	
#endif		
		stk3x1x_enable_ps(ps_data, 0);
		re_enable_ps = true;
	}
	
	ret = stk3x1x_set_irs_it_slp(ps_data, &irs_slp_time);
	if(ret < 0)
		goto irs_err_i2c_rw;
	
    ret = stk3x1x_i2c_smbus_read_byte_data(ps_data->client, STK_STATE_REG);
    if (ret < 0)
    {
        printk(KERN_ERR "%s: write i2c error\n", __func__);
		goto irs_err_i2c_rw;
	}		
	
	w_reg = ret | STK_STATE_EN_IRS_MASK;		
    ret = stk3x1x_i2c_smbus_write_byte_data(ps_data->client, STK_STATE_REG, w_reg);
    if (ret < 0)
	{
		printk(KERN_ERR "%s: write i2c error\n", __func__);
		goto irs_err_i2c_rw;
	}	
	msleep(irs_slp_time);	
	
	do
	{
		msleep(3);
		ret = stk3x1x_get_flag(ps_data);
		if (ret < 0)
		{
			printk(KERN_ERR "%s: write i2c error\n", __func__);
			goto irs_err_i2c_rw;
		}	
		retry++;
	}while(retry < 10 && ((ret&STK_FLG_IR_RDY_MASK) == 0));
	
	if(retry == 10)
	{
		printk(KERN_ERR "%s: ir data is not ready for 300ms\n", __func__);
		ret = -EINVAL;
		goto irs_err_i2c_rw;
	}
	/*
	ret = stk3x1x_get_flag(ps_data);
    if (ret < 0)
    {
        printk(KERN_ERR "%s: write i2c error\n", __func__);
		return ret;
    }		
	*/
	ret = stk3x1x_set_flag(ps_data, ret, STK_FLG_IR_RDY_MASK);
    if (ret < 0)
	{
		printk(KERN_ERR "%s: write i2c error\n", __func__);
		goto irs_err_i2c_rw;
	}		
	
	ret = stk3x1x_i2c_read_data(ps_data->client, STK_DATA1_IR_REG, 2, &value[0]);
	if(ret < 0)
	{
		printk(KERN_ERR "%s fail, ret=0x%x", __func__, ret);
		goto irs_err_i2c_rw;
	}
	word_data = ((value[0]<<8) | value[1]);	

	ret = stk3x1x_i2c_smbus_write_byte_data(ps_data->client, STK_ALSCTRL_REG, ps_data->alsctrl_reg );
	if (ret < 0)
	{
		printk(KERN_ERR "%s: write i2c error\n", __func__);
		goto irs_err_i2c_rw;
	}
	if(re_enable_ps)
		stk3x1x_enable_ps(ps_data, 1);			
	return word_data;

irs_err_i2c_rw:	
	if(re_enable_ps)
		stk3x1x_enable_ps(ps_data, 1);		
	return ret;
}

static ssize_t stk_als_code_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct stk3x1x_data *ps_data =  dev_get_drvdata(dev);		
    int32_t reading;
	
    reading = stk3x1x_get_als_reading(ps_data);
    return scnprintf(buf, PAGE_SIZE, "%d\n", reading);
}


static ssize_t stk_als_enable_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct stk3x1x_data *ps_data =  dev_get_drvdata(dev);
    int32_t enable, ret;
	
    mutex_lock(&ps_data->io_lock);
	enable = (ps_data->als_enabled)?1:0;
    mutex_unlock(&ps_data->io_lock);
    ret = stk3x1x_i2c_smbus_read_byte_data(ps_data->client,STK_STATE_REG);
    ret = (ret & STK_STATE_EN_ALS_MASK)?1:0;
	
	if(enable != ret)
		printk(KERN_ERR "%s: driver and sensor mismatch! driver_enable=0x%x, sensor_enable=%x\n", __func__, enable, ret);
	
	return scnprintf(buf, PAGE_SIZE, "%d\n", ret);	
}

static ssize_t stk_als_enable_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	struct stk3x1x_data *ps_data = dev_get_drvdata(dev);
	uint8_t en;
	if (sysfs_streq(buf, "1"))
		en = 1;
	else if (sysfs_streq(buf, "0"))
		en = 0;
	else 
	{
		printk(KERN_ERR "%s, invalid value %d\n", __func__, *buf);
		return -EINVAL;
	}	
    printk(KERN_INFO "%s: Enable ALS : %d\n", __func__, en);
    mutex_lock(&ps_data->io_lock);
    stk3x1x_enable_als(ps_data, en);
    mutex_unlock(&ps_data->io_lock);
    return size;
}

static ssize_t stk_als_lux_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct stk3x1x_data *ps_data = dev_get_drvdata(dev);
    int32_t als_reading;
	uint32_t als_lux;
    als_reading = stk3x1x_get_als_reading(ps_data);    
	als_lux = stk_alscode2lux(ps_data, als_reading);
    return scnprintf(buf, PAGE_SIZE, "%d lux\n", als_lux);
}

static ssize_t stk_als_lux_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	struct stk3x1x_data *ps_data =  dev_get_drvdata(dev);	
	unsigned long value = 0;
	int ret;
	ret = strict_strtoul(buf, 16, &value);
	if(ret < 0)
	{
		printk(KERN_ERR "%s:strict_strtoul failed, ret=0x%x\n", __func__, ret);	
		return ret;	
	}
    ps_data->als_lux_last = value;
	input_report_abs(ps_data->als_input_dev, ABS_MISC, value);
	input_sync(ps_data->als_input_dev);
	printk(KERN_INFO "%s: als input event %ld lux\n",__func__, value);	

    return size;
}


static ssize_t stk_als_transmittance_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct stk3x1x_data *ps_data =  dev_get_drvdata(dev);	
    int32_t transmittance;
    transmittance = ps_data->als_transmittance;
    return scnprintf(buf, PAGE_SIZE, "%d\n", transmittance);
}


static ssize_t stk_als_transmittance_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	struct stk3x1x_data *ps_data =  dev_get_drvdata(dev);	
	unsigned long value = 0;
	int ret;
	ret = strict_strtoul(buf, 10, &value);
	if(ret < 0)
	{
		printk(KERN_ERR "%s:strict_strtoul failed, ret=0x%x\n", __func__, ret);
		return ret;	    
	}
    ps_data->als_transmittance = value;
    return size;
}

static ssize_t stk_als_delay_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct stk3x1x_data *ps_data =  dev_get_drvdata(dev);
	int64_t delay;
	mutex_lock(&ps_data->io_lock);
	delay = ktime_to_ns(ps_data->als_poll_delay);
	mutex_unlock(&ps_data->io_lock);
	return scnprintf(buf, PAGE_SIZE, "%lld\n", delay);
}


static ssize_t stk_als_delay_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
    uint64_t value = 0;
	int ret;	
	struct stk3x1x_data *ps_data =  dev_get_drvdata(dev);	
	ret = strict_strtoull(buf, 10, &value);
	if(ret < 0)
	{
		printk(KERN_ERR "%s:strict_strtoull failed, ret=0x%x\n", __func__, ret);
		return ret;	
	}	
#ifdef STK_DEBUG_PRINTF		
	printk(KERN_INFO "%s: set als poll delay=%lld\n", __func__, value);
#endif	
	if(value < MIN_ALS_POLL_DELAY_NS)	
	{
		printk(KERN_ERR "%s: delay is too small\n", __func__);
		value = MIN_ALS_POLL_DELAY_NS;
	}
	mutex_lock(&ps_data->io_lock);
	if(value != ktime_to_ns(ps_data->als_poll_delay))
		ps_data->als_poll_delay = ns_to_ktime(value);
	mutex_unlock(&ps_data->io_lock);
	return size;
}

static ssize_t stk_als_ir_code_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct stk3x1x_data *ps_data =  dev_get_drvdata(dev);		
    int32_t reading;
    reading = stk3x1x_get_ir_reading(ps_data);
    return scnprintf(buf, PAGE_SIZE, "%d\n", reading);	
}



static ssize_t stk_ps_code_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct stk3x1x_data *ps_data =  dev_get_drvdata(dev);	
    uint32_t reading;
    reading = stk3x1x_get_ps_reading(ps_data);
    return scnprintf(buf, PAGE_SIZE, "%d\n", reading);
}

static ssize_t stk_ps_enable_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    int32_t enable, ret;
	struct stk3x1x_data *ps_data =  dev_get_drvdata(dev);
	
    mutex_lock(&ps_data->io_lock);
	enable = (ps_data->ps_enabled)?1:0;
    mutex_unlock(&ps_data->io_lock);
    ret = stk3x1x_i2c_smbus_read_byte_data(ps_data->client,STK_STATE_REG);
    ret = (ret & STK_STATE_EN_PS_MASK)?1:0;
	
	if(enable != ret)
		printk(KERN_ERR "%s: driver and sensor mismatch! driver_enable=0x%x, sensor_enable=%x\n", __func__, enable, ret);
	
	return scnprintf(buf, PAGE_SIZE, "%d\n", ret);	
}

static ssize_t stk_ps_enable_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	struct stk3x1x_data *ps_data =  dev_get_drvdata(dev);
	uint8_t en;
	if (sysfs_streq(buf, "1"))
		en = 1;
	else if (sysfs_streq(buf, "0"))
		en = 0;
	else 
	{
		printk(KERN_ERR "%s, invalid value %d\n", __func__, *buf);
		return -EINVAL;
	}
    printk(KERN_INFO "%s: Enable PS : %d\n", __func__, en);
    mutex_lock(&ps_data->io_lock);
    stk3x1x_enable_ps(ps_data, en);
    mutex_unlock(&ps_data->io_lock);
    return size;
}

static ssize_t stk_ps_enable_aso_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    int32_t ret;
	struct stk3x1x_data *ps_data =  dev_get_drvdata(dev);
	
    ret = stk3x1x_i2c_smbus_read_byte_data(ps_data->client,STK_STATE_REG);
    ret = (ret & STK_STATE_EN_ASO_MASK)?1:0;
	
	return scnprintf(buf, PAGE_SIZE, "%d\n", ret);		
}

static ssize_t stk_ps_enable_aso_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	struct stk3x1x_data *ps_data =  dev_get_drvdata(dev);
	uint8_t en;
    int32_t ret;
	uint8_t w_state_reg;
	
	if (sysfs_streq(buf, "1"))
		en = 1;
	else if (sysfs_streq(buf, "0"))
		en = 0;
	else 
	{
		printk(KERN_ERR "%s, invalid value %d\n", __func__, *buf);
		return -EINVAL;
	}
    printk(KERN_INFO "%s: Enable PS ASO : %d\n", __func__, en);
    
    ret = stk3x1x_i2c_smbus_read_byte_data(ps_data->client, STK_STATE_REG);
    if (ret < 0)
    {
        printk(KERN_ERR "%s: write i2c error\n", __func__);
		return ret;
    }		
	w_state_reg = (uint8_t)(ret & (~STK_STATE_EN_ASO_MASK)); 
	if(en)	
		w_state_reg |= STK_STATE_EN_ASO_MASK;	
	
    ret = stk3x1x_i2c_smbus_write_byte_data(ps_data->client, STK_STATE_REG, w_state_reg);
    if (ret < 0)
	{
		printk(KERN_ERR "%s: write i2c error\n", __func__);
		return ret;
	}	
	
	return size;	
}


static ssize_t stk_ps_offset_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct stk3x1x_data *ps_data =  dev_get_drvdata(dev);
    int32_t word_data;		
	unsigned char value[2];
	int ret;
	
	ret = stk3x1x_i2c_read_data(ps_data->client, STK_DATA1_OFFSET_REG, 2, &value[0]);
	if(ret < 0)
	{
		printk(KERN_ERR "%s fail, ret=0x%x", __func__, ret);
		return ret;
	}
	word_data = (value[0]<<8) | value[1];					
		
	return scnprintf(buf, PAGE_SIZE, "%d\n", word_data);
}
 
static ssize_t stk_ps_offset_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	struct stk3x1x_data *ps_data =  dev_get_drvdata(dev);	
	unsigned long offset = 0;
	int ret;
	unsigned char val[2];
	
	ret = strict_strtoul(buf, 10, &offset);
	if(ret < 0)
	{
		printk(KERN_ERR "%s:strict_strtoul failed, ret=0x%x\n", __func__, ret);	
		return ret;	
	}
	if(offset > 65535)
	{
		printk(KERN_ERR "%s: invalid value, offset=%ld\n", __func__, offset);
		return -EINVAL;
	}
	
	val[0] = (offset & 0xFF00) >> 8;
	val[1] = offset & 0x00FF;
	ret = stk3x1x_i2c_write_data(ps_data->client, STK_DATA1_OFFSET_REG, 2, val);	
	if(ret < 0)
	{
		printk(KERN_ERR "%s: write i2c error\n", __func__);
		return ret;
	}
	return size;
}


static ssize_t stk_ps_distance_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct stk3x1x_data *ps_data =  dev_get_drvdata(dev);
    int32_t dist=1, ret;

    ret = stk3x1x_get_flag(ps_data);
	if(ret < 0)
	{
		printk(KERN_ERR "%s: stk3x1x_get_flag failed, ret=0x%x\n", __func__, ret);
		return ret;
	}
    dist = (ret & STK_FLG_NF_MASK)?1:0;	
	
    ps_data->ps_distance_last = dist;
	input_report_abs(ps_data->ps_input_dev, ABS_DISTANCE, dist);
	input_sync(ps_data->ps_input_dev);
	wake_lock_timeout(&ps_data->ps_wakelock, 3*HZ);
	printk(KERN_INFO "%s: ps input event %d cm\n",__func__, dist);		
    return scnprintf(buf, PAGE_SIZE, "%d\n", dist);
}


static ssize_t stk_ps_distance_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	struct stk3x1x_data *ps_data =  dev_get_drvdata(dev);	
	unsigned long value = 0;
	int ret;
	ret = strict_strtoul(buf, 10, &value);
	if(ret < 0)
	{
		printk(KERN_ERR "%s:strict_strtoul failed, ret=0x%x\n", __func__, ret);	
		return ret;	
	}
    ps_data->ps_distance_last = value;
	input_report_abs(ps_data->ps_input_dev, ABS_DISTANCE, value);
	input_sync(ps_data->ps_input_dev);
	wake_lock_timeout(&ps_data->ps_wakelock, 3*HZ);	
	printk(KERN_INFO "%s: ps input event %ld cm\n",__func__, value);	
    return size;
}


static ssize_t stk_ps_code_thd_l_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    int32_t ps_thd_l1_reg, ps_thd_l2_reg;
	struct stk3x1x_data *ps_data =  dev_get_drvdata(dev);	
    ps_thd_l1_reg = stk3x1x_i2c_smbus_read_byte_data(ps_data->client,STK_THDL1_PS_REG);
    if(ps_thd_l1_reg < 0)
	{
		printk(KERN_ERR "%s fail, err=0x%x", __func__, ps_thd_l1_reg);		
		return -EINVAL;		
	}
    ps_thd_l2_reg = stk3x1x_i2c_smbus_read_byte_data(ps_data->client,STK_THDL2_PS_REG);
    if(ps_thd_l2_reg < 0)
	{
		printk(KERN_ERR "%s fail, err=0x%x", __func__, ps_thd_l2_reg);		
		return -EINVAL;		
	}
	ps_thd_l1_reg = ps_thd_l1_reg<<8 | ps_thd_l2_reg;
    return scnprintf(buf, PAGE_SIZE, "%d\n", ps_thd_l1_reg);
}


static ssize_t stk_ps_code_thd_l_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	struct stk3x1x_data *ps_data =  dev_get_drvdata(dev);		
	unsigned long value = 0;
	int ret;
	ret = strict_strtoul(buf, 10, &value);
	if(ret < 0)
	{
		printk(KERN_ERR "%s:strict_strtoul failed, ret=0x%x\n", __func__, ret);
		return ret;	    
	}
    stk3x1x_set_ps_thd_l(ps_data, value);
    return size;
}

static ssize_t stk_ps_code_thd_h_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    int32_t ps_thd_h1_reg, ps_thd_h2_reg;
	struct stk3x1x_data *ps_data =  dev_get_drvdata(dev);	
    ps_thd_h1_reg = stk3x1x_i2c_smbus_read_byte_data(ps_data->client,STK_THDH1_PS_REG);
    if(ps_thd_h1_reg < 0)
	{
		printk(KERN_ERR "%s fail, err=0x%x", __func__, ps_thd_h1_reg);		
		return -EINVAL;		
	}
    ps_thd_h2_reg = stk3x1x_i2c_smbus_read_byte_data(ps_data->client,STK_THDH2_PS_REG);
    if(ps_thd_h2_reg < 0)
	{
		printk(KERN_ERR "%s fail, err=0x%x", __func__, ps_thd_h2_reg);		
		return -EINVAL;		
	}
	ps_thd_h1_reg = ps_thd_h1_reg<<8 | ps_thd_h2_reg;
    return scnprintf(buf, PAGE_SIZE, "%d\n", ps_thd_h1_reg);
}


static ssize_t stk_ps_code_thd_h_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	struct stk3x1x_data *ps_data =  dev_get_drvdata(dev);		
	unsigned long value = 0;
	int ret;
	ret = strict_strtoul(buf, 10, &value);
	if(ret < 0)
	{
		printk(KERN_ERR "%s:strict_strtoul failed, ret=0x%x\n", __func__, ret);
		return ret;	    
	}
    stk3x1x_set_ps_thd_h(ps_data, value);
    return size;
}

#ifdef STK_TUNE0
//CCI Colby add start 20130111
static ssize_t stk_ps_show_nvdata(struct device *dev, struct device_attribute *attr, char *buf)
{
	printk(KERN_INFO "[#23][STK]stk3x1x_show_nvdata\n");
	printk(KERN_INFO "[#23][STK]cci_ps_high_thd = %d\n", cci_ps_high_thd);
	printk(KERN_INFO "[#23][STK]cci_ps_low_thd =  %d\n", cci_ps_low_thd);
	printk(KERN_INFO "[#23][STK]cci_result_ct = %d\n", cci_result_ct);
	printk(KERN_INFO "[#23][STK]cci_ps_cover =  %d\n", cci_ps_cover);
	printk(KERN_INFO "[#23][STK]cci_als_value =  %d lux\n", cci_als_value);
	printk(KERN_INFO "[#23][STK]cci_als_value_cali =  %d lux\n", cci_als_value_cali);
	printk(KERN_INFO "[#23][STK]cci_transmittance_cali =  %d\n", cci_transmittance_cali);
	//scnprintf(buf, PAGE_SIZE, "%d\n", kd);
	return scnprintf(buf, PAGE_SIZE, "cci_ps_high_thd = %d, cci_ps_low_thd = %d, cci_result_ct = %d, cci_ps_cover = %d, cci_als_value = %d lux, cci_als_value_cali = %d lux, cci_transmittance_cali = %d\n", cci_ps_high_thd, cci_ps_low_thd, cci_result_ct, cci_ps_cover, cci_als_value, cci_als_value_cali, cci_transmittance_cali);
}
//CCI Colby add end 20130111
#endif


#if 0
static ssize_t stk_als_lux_thd_l_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    int32_t als_thd_l0_reg,als_thd_l1_reg;
	struct stk3x1x_data *ps_data =  dev_get_drvdata(dev);	
	uint32_t als_lux;
	
    als_thd_l0_reg = stk3x1x_i2c_smbus_read_byte_data(ps_data->client,STK_THDL1_ALS_REG);
    als_thd_l1_reg = stk3x1x_i2c_smbus_read_byte_data(ps_data->client,STK_THDL2_ALS_REG);
    if(als_thd_l0_reg < 0)
	{
		printk(KERN_ERR "%s fail, err=0x%x", __func__, als_thd_l0_reg);			
		return -EINVAL;
	}
	if(als_thd_l1_reg < 0)
	{
		printk(KERN_ERR "%s fail, err=0x%x", __func__, als_thd_l1_reg);				
		return -EINVAL;
	}
    als_thd_l0_reg|=(als_thd_l1_reg<<8);
	als_lux = stk_alscode2lux(ps_data, als_thd_l0_reg);
    return scnprintf(buf, PAGE_SIZE, "%d\n", als_lux);
}


static ssize_t stk_als_lux_thd_l_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	struct stk3x1x_data *ps_data =  dev_get_drvdata(dev);	
	unsigned long value = 0;
	int ret;
	ret = strict_strtoul(buf, 10, &value);
	if(ret < 0)
	{
		printk(KERN_ERR "%s:strict_strtoul failed, ret=0x%x\n", __func__, ret);
		return ret;	    
	}
	value = stk_lux2alscode(ps_data, value);
    stk3x1x_set_als_thd_l(ps_data, value);
    return size;
}

static ssize_t stk_als_lux_thd_h_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    int32_t als_thd_h0_reg,als_thd_h1_reg;
	struct stk3x1x_data *ps_data =  dev_get_drvdata(dev);	
	uint32_t als_lux;
		
    als_thd_h0_reg = stk3x1x_i2c_smbus_read_byte_data(ps_data->client,STK_THDH1_ALS_REG);
    als_thd_h1_reg = stk3x1x_i2c_smbus_read_byte_data(ps_data->client,STK_THDH2_ALS_REG);
    if(als_thd_h0_reg < 0)
	{
		printk(KERN_ERR "%s fail, err=0x%x", __func__, als_thd_h0_reg);			
		return -EINVAL;
	}
	if(als_thd_h1_reg < 0)
	{
		printk(KERN_ERR "%s fail, err=0x%x", __func__, als_thd_h1_reg);				
		return -EINVAL;
	}	
    als_thd_h0_reg|=(als_thd_h1_reg<<8);
	als_lux = stk_alscode2lux(ps_data, als_thd_h0_reg);
    return scnprintf(buf, PAGE_SIZE, "%d\n", als_lux);
}


static ssize_t stk_als_lux_thd_h_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	struct stk3x1x_data *ps_data =  dev_get_drvdata(dev);		
	unsigned long value = 0;
	int ret;
	ret = strict_strtoul(buf, 10, &value);
	if(ret < 0)
	{
		printk(KERN_ERR "%s:strict_strtoul failed, ret=0x%x\n", __func__, ret);
		return ret;	    
	}
    value = stk_lux2alscode(ps_data, value);    
    stk3x1x_set_als_thd_h(ps_data, value);
    return size;
}
#endif


static ssize_t stk_all_reg_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    int32_t ps_reg[27];
	uint8_t cnt;
	struct stk3x1x_data *ps_data =  dev_get_drvdata(dev);	
	for(cnt=0;cnt<25;cnt++)
	{
		ps_reg[cnt] = stk3x1x_i2c_smbus_read_byte_data(ps_data->client, (cnt));
		if(ps_reg[cnt] < 0)
		{
			printk(KERN_ERR "%s fail, ret=%d", __func__, ps_reg[cnt]);	
			return -EINVAL;
		}
		else
		{
			printk(KERN_INFO "reg[0x%2X]=0x%2X\n", cnt, ps_reg[cnt]);
		}
	}
	ps_reg[cnt] = stk3x1x_i2c_smbus_read_byte_data(ps_data->client, STK_PDT_ID_REG);
	if(ps_reg[cnt] < 0)
	{
		printk( KERN_ERR "%s fail, ret=%d", __func__, ps_reg[cnt]);	
		return -EINVAL;
	}
	printk( KERN_INFO "reg[0x%x]=0x%2X\n", STK_PDT_ID_REG, ps_reg[cnt]);	
	cnt++;
	ps_reg[cnt] = stk3x1x_i2c_smbus_read_byte_data(ps_data->client, STK_RSRVD_REG);
	if(ps_reg[cnt] < 0)
	{
		printk( KERN_ERR "%s fail, ret=%d", __func__, ps_reg[cnt]);	
		return -EINVAL;
	}
	printk( KERN_INFO "reg[0x%x]=0x%2X\n", STK_RSRVD_REG, ps_reg[cnt]);		

    //return scnprintf(buf, PAGE_SIZE, "%2X %2X %2X %2X %2X,%2X %2X %2X %2X %2X,%2X %2X %2X %2X %2X,%2X %2X %2X %2X %2X,%2X %2X %2X %2X %2X,%2X %2X\n", 
    return scnprintf(buf, PAGE_SIZE, "[0]%2X [1]%2X [2]%2X [3]%2X [4]%2X [5]%2X [6/7 HTHD]%2X,%2X [8/9 LTHD]%2X, %2X [A]%2X [B]%2X [C]%2X [D]%2X [E/F Aoff]%2X,%2X,[10]%2X [11/12 PS]%2X,%2X [13]%2X [14]%2X [15/16 Foff]%2X,%2X [17]%2X [18]%2X [3E]%2X [3F]%2X\n", 	
		ps_reg[0], ps_reg[1], ps_reg[2], ps_reg[3], ps_reg[4], ps_reg[5], ps_reg[6], ps_reg[7], ps_reg[8], 
		ps_reg[9], ps_reg[10], ps_reg[11], ps_reg[12], ps_reg[13], ps_reg[14], ps_reg[15], ps_reg[16], ps_reg[17], 
		ps_reg[18], ps_reg[19], ps_reg[20], ps_reg[21], ps_reg[22], ps_reg[23], ps_reg[24], ps_reg[25], ps_reg[26]);
}


static ssize_t stk_status_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    int32_t ps_reg[27];
	uint8_t cnt;
	struct stk3x1x_data *ps_data =  dev_get_drvdata(dev);	
	for(cnt=0;cnt<25;cnt++)
	{
		ps_reg[cnt] = stk3x1x_i2c_smbus_read_byte_data(ps_data->client, (cnt));
		if(ps_reg[cnt] < 0)
		{
			printk(KERN_ERR "%s fail, ret=%d", __func__, ps_reg[cnt]);	
			return -EINVAL;
		}
		else
		{
			printk(KERN_INFO "reg[0x%2X]=0x%2X\n", cnt, ps_reg[cnt]);
		}
	}
	ps_reg[cnt] = stk3x1x_i2c_smbus_read_byte_data(ps_data->client, STK_PDT_ID_REG);
	if(ps_reg[cnt] < 0)
	{
		printk( KERN_ERR "%s fail, ret=%d", __func__, ps_reg[cnt]);	
		return -EINVAL;
	}
	printk( KERN_INFO "reg[0x%x]=0x%2X\n", STK_PDT_ID_REG, ps_reg[cnt]);	
	cnt++;
	ps_reg[cnt] = stk3x1x_i2c_smbus_read_byte_data(ps_data->client, STK_RSRVD_REG);
	if(ps_reg[cnt] < 0)
	{
		printk( KERN_ERR "%s fail, ret=%d", __func__, ps_reg[cnt]);	
		return -EINVAL;
	}
	printk( KERN_INFO "reg[0x%x]=0x%2X\n", STK_RSRVD_REG, ps_reg[cnt]);		

    return scnprintf(buf, PAGE_SIZE, "[PS=%2X] [ALS=%2X] [WAIT=0x%4Xms] [EN_ASO=%2X] [EN_AK=%2X] [NEAR/FAR=%2X] [FLAG_OUI=%2X] [FLAG_PSINT=%2X] [FLAG_ALSINT=%2X]\n", 
		ps_reg[0]&0x01,(ps_reg[0]&0x02)>>1,((ps_reg[0]&0x04)>>2)*ps_reg[5]*6,(ps_reg[0]&0x20)>>5,
		(ps_reg[0]&0x40)>>6,ps_reg[16]&0x01,(ps_reg[16]&0x04)>>2,(ps_reg[16]&0x10)>>4,(ps_reg[16]&0x20)>>5);		
}

static ssize_t stk_recv_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return 0;
}


static ssize_t stk_recv_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
    unsigned long value = 0;
	int ret;
	int32_t recv_data;	
	struct stk3x1x_data *ps_data =  dev_get_drvdata(dev);	
	
	if((ret = strict_strtoul(buf, 16, &value)) < 0)
	{
		printk(KERN_ERR "%s:strict_strtoul failed, ret=0x%x\n", __func__, ret);
		return ret;	
	}
	recv_data = stk3x1x_i2c_smbus_read_byte_data(ps_data->client,value);
	printk("%s: reg 0x%x=0x%x\n", __func__, (int)value, recv_data);
	return size;
}


static ssize_t stk_send_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return 0;
}


static ssize_t stk_send_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	int addr, cmd;

	int32_t ret, i;
	char *token[10];
	struct stk3x1x_data *ps_data =  dev_get_drvdata(dev);	
	
	for (i = 0; i < 2; i++)
		token[i] = strsep((char **)&buf, " ");
	if((ret = strict_strtoul(token[0], 16, (unsigned long *)&(addr))) < 0)
	{
		printk(KERN_ERR "%s:strict_strtoul failed, ret=0x%x\n", __func__, ret);
		return ret;	
	}
	if((ret = strict_strtoul(token[1], 16, (unsigned long *)&(cmd))) < 0)
	{
		printk(KERN_ERR "%s:strict_strtoul failed, ret=0x%x\n", __func__, ret);
		return ret;	
	}
	printk(KERN_INFO "%s: write reg 0x%x=0x%x\n", __func__, addr, cmd);		

	ret = stk3x1x_i2c_smbus_write_byte_data(ps_data->client, (unsigned char)addr, (unsigned char)cmd);
	if (0 != ret)
	{	
		printk(KERN_ERR "%s: stk3x1x_i2c_smbus_write_byte_data fail\n", __func__);
		return ret;
	}
	
	return size;
}

#ifdef STK_TUNE0

static ssize_t stk_ps_cali_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct stk3x1x_data *ps_data =  dev_get_drvdata(dev);	
	int32_t word_data;	
	unsigned char value[2];
	int ret;
	
	ret = stk3x1x_i2c_read_data(ps_data->client, 0x20, 2, &value[0]);
	if(ret < 0)
	{
		printk(KERN_ERR "%s fail, ret=0x%x", __func__, ret);
		return ret;
	}
	word_data = (value[0]<<8) | value[1];	
	
	ret = stk3x1x_i2c_read_data(ps_data->client, 0x22, 2, &value[0]);
	if(ret < 0)
	{
		printk(KERN_ERR "%s fail, ret=0x%x", __func__, ret);
		return ret;
	}
	word_data += ((value[0]<<8) | value[1]);		

	printk("%s: psi_set=%d, psa=%d,psi=%d, word_data=%d\n", __func__, 
		ps_data->psi_set, ps_data->psa, ps_data->psi, word_data);	
	
	return 0;
}


static ssize_t stk_ps_cali_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	 // add first H/L thd when cci_result_ct get start
	struct stk3x1x_data *ps_data =  dev_get_drvdata(dev);
	int cci_result_ct_x2 = 0;
	 // add first H/L thd when cci_result_ct get end
	unsigned long value = 0;
	int ret, i;	
	char *token[10];
	//struct stk3x1x_data *ps_data =  dev_get_drvdata(dev);	
	
	for (i = 0; i < 7; i++)
	token[i] = strsep((char **)&buf, " ");
	if((ret = strict_strtoul(token[0], 16, &value)) < 0)
	{
		printk(KERN_ERR "%s:[STK]strict_strtoul failed, ret=0x%x\n", __func__, ret);
		return ret;	
	}
	cci_ps_high_thd = value;
	
	if((ret = strict_strtoul(token[1], 16, &value)) < 0)
	{
		printk(KERN_ERR "%s:[STK]strict_strtoul failed, ret=0x%x\n", __func__, ret);
		return ret;	
	}
	cci_ps_low_thd = value;
	printk(KERN_INFO "%s [STK]cci_ps_high_thd = %d, cci_ps_low_thd = %d", __FUNCTION__, cci_ps_high_thd, cci_ps_low_thd);
	
	if((ret = strict_strtoul(token[2], 16, &value)) < 0)
	{
		printk(KERN_ERR "%s:[STK]strict_strtoul failed, ret=0x%x\n", __func__, ret);
		return ret;	
	}	
	cci_result_ct = value;

	// add first H/L thd when cci_result_ct get start
	cci_result_ct_x2 = cci_result_ct * 2;
	printk(KERN_INFO "%s: cci_result_ct_x2 = %d, cci_result_ct = %d\n", __func__, cci_result_ct_x2, cci_result_ct);	
	if(cci_result_ct_x2 >= 0xFFFF)
		cci_result_ct_x2 = 15000;
	ps_data->ps_thd_h = cci_result_ct_x2 + (cci_ps_high_thd - cci_result_ct);
	ps_data->ps_thd_l = cci_result_ct_x2 + (cci_ps_low_thd - cci_result_ct);				
	printk(KERN_INFO "%s: set HT=%d,LT=%d according to cci_result_ct_x2\n", __func__, ps_data->ps_thd_h,  ps_data->ps_thd_l);				
	// CCI add First boot H/L thd for debug start
	FIRST_BOOT_H_THD = cci_result_ct_x2 +  (cci_ps_high_thd - cci_result_ct);
	FIRST_BOOT_L_THD = cci_result_ct_x2 + (cci_ps_low_thd - cci_result_ct);
	// CCI add First boot H/L thd for debug end
	stk3x1x_set_ps_thd_h(ps_data, ps_data->ps_thd_h);
	stk3x1x_set_ps_thd_l(ps_data, ps_data->ps_thd_l);		
	// add first H/L thd when cci_result_ct get end

	if((ret = strict_strtoul(token[3], 16, &value)) < 0)
	{
		printk(KERN_ERR "%s:[STK]strict_strtoul failed, ret=0x%x\n", __func__, ret);
		return ret;	
	}
	cci_ps_cover = value;
	
	if((ret = strict_strtoul(token[4], 16, &value)) < 0)
	{
		printk(KERN_ERR "%s:[STK]strict_strtoul failed, ret=0x%x\n", __func__, ret);
		return ret;	
	}	
	cci_als_value = value;	

	if((ret = strict_strtoul(token[5], 16, &value)) < 0)
	{
		printk(KERN_ERR "%s:[STK]strict_strtoul failed, ret=0x%x\n", __func__, ret);
		return ret;
	}
	cci_als_value_cali = value;

	if((ret = strict_strtoul(token[6], 16, &value)) < 0)
	{
		printk(KERN_ERR "%s:[STK]strict_strtoul failed, ret=0x%x\n", __func__, ret);
		return ret;
	}
	cci_transmittance_cali = value;
	ps_data->als_transmittance = cci_transmittance_cali; // writ back to als_transmittance
	
	return size;
}

#endif	/* #ifdef STK_TUNE0 */


//Colby add for CCI calibration start
static inline uint32_t stk3x1x_get_ps_reading_AVG(struct stk3x1x_data *ps_data, int sSampleNo){
	uint32_t PSData = 0;
	uint32_t DataCount = 0;
	uint32_t sAvePsData = 0;
	//uint32_t reading;
	while(DataCount < sSampleNo)
	{
		msleep(100);
		PSData = stk3x1x_get_ps_reading(ps_data);
		printk(KERN_ERR "%s: [Colby][STK]ps code = %d\n", __func__, PSData);
		sAvePsData +=  PSData;
		DataCount++;
	}
	sAvePsData /= sSampleNo;
	return sAvePsData;
}

static inline uint32_t stk3x1x_get_als_reading_AVG(struct stk3x1x_data *ps_data, int sSampleNo){
	uint32_t ALSData = 0;
	uint32_t DataCount = 0;
	uint32_t sAveAlsData = 0;

	while(DataCount < sSampleNo)
	{
		msleep(100);
		ALSData = stk3x1x_get_als_reading(ps_data);
		printk(KERN_ERR "%s: [#23][STK]als code = %d\n", __func__, ALSData);
		sAveAlsData +=  ALSData;
		DataCount++;
	}
	sAveAlsData /= sSampleNo;
	return sAveAlsData;
}

static ssize_t ps_CCI_cali_BC_15mm_show(struct device *dev, struct device_attribute *attr, char *buf)
{
//not ready
	//PS cal Black card 1.5 cm
	struct stk3x1x_data *ps_data =  dev_get_drvdata(dev);	
	uint32_t reading;

	//check if auto cal is disable start
	int32_t recv_data, recv_data_tmp;
	u8 cmd_u8;
	int32_t ret;
	recv_data = i2c_smbus_read_byte_data(ps_data->client,0);
	printk("%s: [Colby][STK]reg 0x0 = 0x%x\n", __func__, recv_data);
	recv_data_tmp = recv_data & 0x0F;
	printk("%s: [Colby][STK]recv_data_tmp = 0x%x\n", __func__, recv_data_tmp);
	cmd_u8 = (u8) recv_data_tmp;
	ret = i2c_smbus_write_byte_data(ps_data->client, 0,cmd_u8);
	if (0 != ret){	
		printk(KERN_ERR "%s: [Colby][STK]i2c_smbus_write_byte_data fail\n", __func__);
		return ret;
		}
	//read again
	recv_data = i2c_smbus_read_byte_data(ps_data->client,0);
	printk("%s: [Colby][STK]read again: reg 0x0 = 0x%x\n", __func__, recv_data);
	//check if auto cal is disable end

	//add for FTM interrupt check 20130424 start
	//set default THD as ps_thd_h = 0, ps_thd_l = 0 to ensure int trigger
	ps_data->ps_thd_h = 0;
	ps_data->ps_thd_l = 0;				
	printk(KERN_INFO "%s: [Colby] ps_CCI_cali_BC_15mm_show() ps_data->ps_thd_h = %d, ps_data->ps_thd_l = %d\n", __FUNCTION__, ps_data->ps_thd_h, ps_data->ps_thd_l);
	stk3x1x_set_ps_thd_h(ps_data, ps_data->ps_thd_h);
	stk3x1x_set_ps_thd_l(ps_data, ps_data->ps_thd_l);
	//add for FTM interrupt check 20130424  end

	//uint32_t cci_ps_high_thd;
	//get ps code for 5 times
	printk(KERN_ERR "%s:[Colby][STK]Start calibrating with Black Card 1.5 cm...\n", __func__);
	msleep(150);
	reading = stk3x1x_get_ps_reading_AVG(ps_data, 5);

	cci_ps_high_thd = reading; //*****************This is F2N threshold***************
	printk(KERN_ERR "%s:[Colby][STK]calibrating with Black Card 1.5 cm done!!!!! cci_ps_high_thd = %d\n", __func__, cci_ps_high_thd);
	return scnprintf(buf, PAGE_SIZE, "cci_ps_high_thd = %d\n", cci_ps_high_thd);
}

static ssize_t ps_CCI_cali_BC_ct_show(struct device *dev, struct device_attribute *attr, char *buf)
{
//not ready
	//Test PS Black card CT
	struct stk3x1x_data *ps_data =  dev_get_drvdata(dev);	
	uint32_t reading;
	//uint32_t cci_ps_low_thd;
	//uint32_t cci_result_ct;
	int Diff_ThdH_CT;
	//get ps code for 5 times
	printk(KERN_ERR "%s:[Colby][STK]Start calibrating CT...\n", __func__);
	reading = stk3x1x_get_ps_reading_AVG(ps_data, 5);

	cci_result_ct = reading;//*****************This is CT***************


	Diff_ThdH_CT = cci_ps_high_thd - cci_result_ct;
	//when IT=0x73 and LED CURRENT=100mA, (ps_close-ps_ct) should be bigger than 75
	//Optical team change the range from 75 to 100. 20130117
	if(Diff_ThdH_CT > 100){
		//cci_ps_low_thd = cci_result_ct + Diff_ThdH_CT/2; //*****************This is N2F value*************** 5:5 for H/L/CT
		cci_ps_low_thd = cci_result_ct + (Diff_ThdH_CT*7)/10; //*****************This is N2F value*************** set 3:7 for H/L/CT

		//add for FTM interrupt check 20130424 start
		printk(KERN_INFO "%s: [Colby] ps_CCI_cali_BC_ct_show() cci_ps_high_thd = %d, cci_ps_low_thd = %d\n", __FUNCTION__, cci_ps_high_thd, cci_ps_low_thd);
		ps_data->ps_thd_h = cci_ps_high_thd;
		ps_data->ps_thd_l = cci_ps_low_thd;				

		stk3x1x_set_ps_thd_h(ps_data, ps_data->ps_thd_h);
		stk3x1x_set_ps_thd_l(ps_data, ps_data->ps_thd_l);
		//add for FTM interrupt check 20130424  end

		printk(KERN_ERR "[Colby][STK]---------------Test PS Black card CT successfully----------------\n");
		printk(KERN_ERR "[Colby][STK]cci_ps_low_thd = %d\n", cci_ps_low_thd);
		}
	else{
		cci_ps_low_thd = 0;
		printk(KERN_ERR "[Colby][STK]---------------Test PS Black card CT fail----------------\n");
		printk(KERN_ERR "[Colby][STK]cci_ps_low_thd = %d\n", cci_ps_low_thd);
		}

	printk(KERN_ERR "[Colby][STK]cci_result_ct = %d, Diff_ThdH_CT = %d, cci_ps_low_thd = %d\n", cci_result_ct, Diff_ThdH_CT, cci_ps_low_thd);

	return scnprintf(buf, PAGE_SIZE, "cci_result_ct = %d, cci_ps_low_thd = %d\n", cci_result_ct, cci_ps_low_thd);
}

static ssize_t ps_CCI_test_BC_show(struct device *dev, struct device_attribute *attr, char *buf)
{
//not ready
	//Test black card cover
	struct stk3x1x_data *ps_data =  dev_get_drvdata(dev);	
	uint32_t reading;
	uint32_t test_ps_code;
	bool ps_test_cover;
	//get ps code for 5 times
	printk(KERN_ERR "%s:[Colby][STK]Start testing BC...\n", __func__);
	reading = stk3x1x_get_ps_reading_AVG(ps_data, 5);
	
	test_ps_code = reading;

	printk(KERN_ERR "%s:[Colby][STK]ps_CCI_test_BC_show() CCI_FTM_INTERRUPT = %d\n", __func__, CCI_FTM_INTERRUPT); //add for FTM interrupt check 20130424 start
	if(test_ps_code >= cci_ps_high_thd){
		ps_test_cover = true;
		cci_ps_cover = test_ps_code;
		//add for FTM interrupt check 20130424 start
		//printk(KERN_ERR "[Colby][STK]Test PS Black card 0.8 cm SUCCESS: ps_test_cover = %d, g_result_ps_cover = %d\n", ps_test_cover, cci_ps_cover);
		//return scnprintf(buf, PAGE_SIZE, "Test PS Black card 0.8 cm SUCCESS: ps_test_cover = %d, g_result_ps_cover = %d\n", ps_test_cover, cci_ps_cover);
		if(CCI_FTM_INTERRUPT != 0){
			printk(KERN_ERR "[Colby][STK]Test PS Black card 0.8 cm SUCCESS: ps_test_cover = %d, g_result_ps_cover = %d, INT PASS\n", ps_test_cover, cci_ps_cover);
			return scnprintf(buf, PAGE_SIZE, "Test PS Black card 0.8 cm SUCCESS: ps_test_cover = %d, g_result_ps_cover = %d, INT PASS\n", ps_test_cover, cci_ps_cover);
			}
		else{
			ps_test_cover = false;
			printk(KERN_ERR "[Colby][STK]Test PS Black card 0.8 cm FAIL: ps_test_cover = %d, g_result_ps_cover = %d, INT FAIL\n", ps_test_cover, cci_ps_cover);
			return scnprintf(buf, PAGE_SIZE, "Test PS Black card 0.8 cm FAIL: ps_test_cover = %d, g_result_ps_cover = %d, INT FAIL\n", ps_test_cover, cci_ps_cover);
			}
		//add for FTM interrupt check 20130424 start
		}
	else{
		ps_test_cover = false;
		cci_ps_cover = test_ps_code;

		//add for FTM interrupt check 20130424 start
		//printk(KERN_ERR "[Colby][STK]Test PS Black card 0.8 cm FAIL: ps_test_cover = %d, g_result_ps_cover = %d\n", ps_test_cover, cci_ps_cover);
		//return scnprintf(buf, PAGE_SIZE, "Test PS Black card 0.8 cm FAIL: ps_test_cover = %d, g_result_ps_cover = %d\n", ps_test_cover, cci_ps_cover);
		if(CCI_FTM_INTERRUPT != 0){
			printk(KERN_ERR "[Colby][STK]Test PS Black card 0.8 cm FAIL: ps_test_cover = %d, g_result_ps_cover = %d, INT PASS\n", ps_test_cover, cci_ps_cover);
			return scnprintf(buf, PAGE_SIZE, "Test PS Black card 0.8 cm FAIL: ps_test_cover = %d, g_result_ps_cover = %d, INT PASS\n", ps_test_cover, cci_ps_cover);
			}
		else{
			printk(KERN_ERR "[Colby][STK]Test PS Black card 0.8 cm FAIL: ps_test_cover = %d, g_result_ps_cover = %d, INT FAIL\n", ps_test_cover, cci_ps_cover);
			return scnprintf(buf, PAGE_SIZE, "Test PS Black card 0.8 cm FAIL: ps_test_cover = %d, g_result_ps_cover = %d, INT FAIL\n", ps_test_cover, cci_ps_cover);
			}
		//add for FTM interrupt check 20130424 start
		}
}

static ssize_t als_CCI_test_Light_show(struct device *dev, struct device_attribute *attr, char *buf)
{
//not ready
	//Test ALS 
	struct stk3x1x_data *ps_data = dev_get_drvdata(dev);
	int32_t als_reading;
	//uint32_t als_lux;
	printk(KERN_ERR "%s:[#23][STK]Start testing light...\n", __func__);
	msleep(150);
	//als_reading = stk3x1x_get_als_reading(ps_data);
	als_reading = stk3x1x_get_als_reading_AVG(ps_data, 5);
	mutex_lock(&ps_data->io_lock);
	printk(KERN_ERR "%s:[#23][STK]als_reading = %d\n", __func__, als_reading);
	cci_als_value = stk_alscode2lux(ps_data, als_reading);
	//cci_als_value = als_reading; //return raw data only
	mutex_unlock(&ps_data->io_lock);
	printk(KERN_ERR "%s:[#23][STK]Start testing light done!!! cci_als_value = %d lux\n", __func__, cci_als_value);
	return scnprintf(buf, PAGE_SIZE, "cci_als_value = %d lux\n", cci_als_value);
}

static ssize_t als_CCI_cali_Light_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	//Cali ALS
	struct stk3x1x_data *ps_data = dev_get_drvdata(dev);
	int32_t als_reading;
	unsigned int cci_als_value_cali_adc;
	printk(KERN_ERR "%s:[#23][STK]Start Cali light...\n", __func__);
	msleep(150);
	//als_reading = stk3x1x_get_als_reading(ps_data);
	als_reading = stk3x1x_get_als_reading_AVG(ps_data, 5);
	cci_als_value_cali_adc = als_reading;
	mutex_lock(&ps_data->io_lock);
	cci_als_value_cali = stk_alscode2lux(ps_data, als_reading);
	//cci_als_value_cali = als_reading; //return raw data only, ADC code

	//compute transmittance for ALS calibration start
	if(((cci_als_value_cali * ps_data->als_transmittance)/500) > 0){ //if cci_als_value_cali = 0, the cci_transmittance_cali will be always 0, so skip it.
		cci_transmittance_cali = (cci_als_value_cali * ps_data->als_transmittance)/500; //transmittance for cali
		ps_data->als_transmittance = cci_transmittance_cali; // writ back to als_transmittance
		printk(KERN_ERR "%s:[#23][STK]cali light done!!! cci_als_value_cali = %d lux, cci_transmittance_cali = %d, cci_als_value_cali_adc = %d code\n", __func__, cci_als_value_cali, cci_transmittance_cali, cci_als_value_cali_adc);
		}
	else{
		printk(KERN_ERR "%s:[#23][STK]cali light fail!!! cci_transmittance_cali cannot be 0 \n", __func__);
		}
	//compute transmittance for ALS calibration end

	mutex_unlock(&ps_data->io_lock);
	//printk(KERN_ERR "%s:[#23][STK]Start cali light done!!! cci_als_value_cali = %d lux, cci_transmittance_cali = %d\n", __func__, cci_als_value_cali, cci_transmittance_cali);
	return scnprintf(buf, PAGE_SIZE, "cci_als_value_cali = %d lux, cci_transmittance_cali = %d, cci_als_value_cali_adc = %d code\n", cci_als_value_cali, cci_transmittance_cali, cci_als_value_cali_adc);
}

//Colby add for CCI calibration end


static struct device_attribute als_enable_attribute = __ATTR(enable,0664,stk_als_enable_show,stk_als_enable_store);
static struct device_attribute als_lux_attribute = __ATTR(lux,0664,stk_als_lux_show,stk_als_lux_store);
static struct device_attribute als_code_attribute = __ATTR(code, 0444, stk_als_code_show, NULL);
static struct device_attribute als_transmittance_attribute = __ATTR(transmittance,0664,stk_als_transmittance_show,stk_als_transmittance_store);
#if 0
static struct device_attribute als_lux_thd_l_attribute = __ATTR(luxthdl,0664,stk_als_lux_thd_l_show,stk_als_lux_thd_l_store);
static struct device_attribute als_lux_thd_h_attribute = __ATTR(luxthdh,0664,stk_als_lux_thd_h_show,stk_als_lux_thd_h_store);
#endif
static struct device_attribute als_poll_delay_attribute = __ATTR(delay,0664,stk_als_delay_show,stk_als_delay_store);
static struct device_attribute als_ir_code_attribute = __ATTR(ircode,0444,stk_als_ir_code_show,NULL);
//Colby add for CCI calibration start
static struct device_attribute als_CCI_test_Light = __ATTR(test_Light,0664,als_CCI_test_Light_show,NULL);
static struct device_attribute als_CCI_cali_Light = __ATTR(cali_Light,0664,als_CCI_cali_Light_show,NULL);
//Colby add for CCI calibration end


static struct attribute *stk_als_attrs [] =
{
	&als_enable_attribute.attr,
    &als_lux_attribute.attr,
    &als_code_attribute.attr,
    &als_transmittance_attribute.attr,
    &als_CCI_test_Light.attr,
    &als_CCI_cali_Light.attr,
#if 0	
	&als_lux_thd_l_attribute.attr,
	&als_lux_thd_h_attribute.attr,
#endif	
	&als_poll_delay_attribute.attr,
	&als_ir_code_attribute.attr,
    NULL
};

static struct attribute_group stk_als_attribute_group = {
	.name = "driver",
	.attrs = stk_als_attrs,
};


static struct device_attribute ps_enable_attribute = __ATTR(enable,0664,stk_ps_enable_show,stk_ps_enable_store);
static struct device_attribute ps_enable_aso_attribute = __ATTR(enableaso,0664,stk_ps_enable_aso_show,stk_ps_enable_aso_store);
static struct device_attribute ps_distance_attribute = __ATTR(distance,0664,stk_ps_distance_show, stk_ps_distance_store);
static struct device_attribute ps_offset_attribute = __ATTR(offset,0664,stk_ps_offset_show, stk_ps_offset_store);
static struct device_attribute ps_code_attribute = __ATTR(code, 0444, stk_ps_code_show, NULL);
static struct device_attribute ps_code_thd_l_attribute = __ATTR(codethdl,0664,stk_ps_code_thd_l_show,stk_ps_code_thd_l_store);
static struct device_attribute ps_code_thd_h_attribute = __ATTR(codethdh,0664,stk_ps_code_thd_h_show,stk_ps_code_thd_h_store);
static struct device_attribute recv_attribute = __ATTR(recv,0664,stk_recv_show,stk_recv_store);
static struct device_attribute send_attribute = __ATTR(send,0664,stk_send_show, stk_send_store);
static struct device_attribute all_reg_attribute = __ATTR(allreg, 0444, stk_all_reg_show, NULL);
static struct device_attribute status_attribute = __ATTR(status, 0444, stk_status_show, NULL);
#ifdef STK_TUNE0
//CCI Colby add start 20130111
static struct device_attribute ps_nvdata_attribute = __ATTR(nvdata,0664,stk_ps_show_nvdata, NULL);
//CCI Colby add end 20130111	
static struct device_attribute ps_cali_attribute = __ATTR(cali,0664,stk_ps_cali_show, stk_ps_cali_store); //[Bug524]modify permission for CTS
#endif
//Colby add for CCI calibration start
static struct device_attribute ps_CCI_cali_BC_15mm = __ATTR(BC_15mm,0664,ps_CCI_cali_BC_15mm_show, NULL);
static struct device_attribute ps_CCI_cali_BC_ct = __ATTR(BC_ct,0664,ps_CCI_cali_BC_ct_show, NULL);
static struct device_attribute ps_CCI_test_BC = __ATTR(test_BC,0664,ps_CCI_test_BC_show, NULL);
//Colby add for CCI calibration end

static struct attribute *stk_ps_attrs [] =
{
    &ps_enable_attribute.attr,
    &ps_enable_aso_attribute.attr,
    &ps_distance_attribute.attr,
	&ps_offset_attribute.attr,
    &ps_code_attribute.attr,
	&ps_code_thd_l_attribute.attr,
	&ps_code_thd_h_attribute.attr,	
	&recv_attribute.attr,
	&send_attribute.attr,	
	&all_reg_attribute.attr,
	&status_attribute.attr,
	&ps_CCI_cali_BC_15mm.attr,
	&ps_CCI_cali_BC_ct.attr,
	&ps_CCI_test_BC.attr,
#ifdef STK_TUNE0
	&ps_cali_attribute.attr,
	&ps_nvdata_attribute.attr,
#endif	
    NULL
};

static struct attribute_group stk_ps_attribute_group = {
	.name = "driver",	
	.attrs = stk_ps_attrs,
};

#ifdef STK_TUNE0	
static int stk_ps_tune_zero_val(struct stk3x1x_data *ps_data)
{
	int mode;
	int32_t word_data, lii;	
	unsigned char value[2];
	int ret;
	
	ret = stk3x1x_i2c_read_data(ps_data->client, 0x20, 2, &value[0]);
	if(ret < 0)
	{
		printk(KERN_ERR "%s fail, ret=0x%x", __func__, ret);
		return ret;
	}
	word_data = (value[0]<<8) | value[1];	
	
	ret = stk3x1x_i2c_read_data(ps_data->client, 0x22, 2, &value[0]);
	if(ret < 0)
	{
		printk(KERN_ERR "%s fail, ret=0x%x", __func__, ret);
		return ret;
	}
	word_data += ((value[0]<<8) | value[1]);	
	
	mode = (ps_data->psctrl_reg) & 0x3F;
	if(mode == 0x30)	
	lii = 100;	
	else if (mode == 0x31)
	lii = 200;		
	else if (mode == 0x32)
	lii = 400;		
	else if (mode == 0x33)
	lii = 800;	
	else
	{
		printk(KERN_ERR "%s: unsupported PS_IT(0x%x)\n", __func__, mode);
		return -1;
	}

	if(word_data > lii)	
	{
		printk(KERN_INFO "%s: word_data=%d, lii=%d\n", __func__, word_data, lii);	
		return 0xFFFF;	
	}
	return 0;
}

static int stk_ps_tune_zero_final(struct stk3x1x_data *ps_data)
{
	int ret;
	// add first H/L thd when cci_result_ct get start
	//int cci_result_ct_x2 = cci_result_ct*2;
	//printk(KERN_INFO "%s: [CCI]cci_result_ct_x2 = %d, cci_result_ct = %d\n", __func__, cci_result_ct_x2, cci_result_ct);
	// add first H/L thd when cci_result_ct get end
	ps_data->tune_zero_init_proc = false;
	ret = stk3x1x_i2c_smbus_write_byte_data(ps_data->client, STK_INT_REG, ps_data->int_reg);
	if (ret < 0)
	{
		printk(KERN_ERR "%s: write i2c error\n", __func__);
		return ret;
	}	
	
	ret = stk3x1x_i2c_smbus_write_byte_data(ps_data->client, STK_STATE_REG, 0);
	if (ret < 0)
	{
		printk(KERN_ERR "%s: write i2c error\n", __func__);
		return ret;
	}		
	
	if(ps_data->data_count == -1)
	{
		printk(KERN_INFO "%s: exceed limit\n", __func__);
		hrtimer_cancel(&ps_data->ps_tune0_timer);	
		return 0;
	}
	// add first H/L thd when cci_result_ct get start
	//if(ps_data->ps_stat_data[1] < cci_result_ct_x2)
	//{
	// add first H/L thd when cci_result_ct get end
		ps_data->psa = ps_data->ps_stat_data[0];
		ps_data->psi = ps_data->ps_stat_data[2];							
		ps_data->ps_thd_h = ps_data->ps_stat_data[1] + STK_HT_N_CT;
		ps_data->ps_thd_l = ps_data->ps_stat_data[1] + STK_LT_N_CT;		
		printk(KERN_INFO "%s: [CCI]set HT=%d,LT=%d according to ps_stat_data\n", __func__, ps_data->ps_thd_h,  ps_data->ps_thd_l);		
	// add first H/L thd when cci_result_ct get start
	//}
	//else
	//{
	//	if(cci_result_ct_x2 >= 0xFFFF)
	//		cci_result_ct_x2 = 15000;
	//	ps_data->ps_thd_h = cci_result_ct_x2 - STK_LT_N_CT;
	//	ps_data->ps_thd_l = cci_result_ct_x2 - STK_HT_N_CT;				
	//	printk(KERN_INFO "%s: set HT=%d,LT=%d according to cci_result_ct_x2\n", __func__, ps_data->ps_thd_h,  ps_data->ps_thd_l);				
	//}
	// add first H/L thd when cci_result_ct get end
	// CCI add First boot H/L thd for debug start
	FIRST_BOOT_H_THD = ps_data->ps_stat_data[1] + STK_HT_N_CT;
	FIRST_BOOT_L_THD = ps_data->ps_stat_data[1] + STK_LT_N_CT;
	// CCI add First boot H/L thd for debug end
	stk3x1x_set_ps_thd_h(ps_data, ps_data->ps_thd_h);
	stk3x1x_set_ps_thd_l(ps_data, ps_data->ps_thd_l);				
	hrtimer_cancel(&ps_data->ps_tune0_timer);					
	return 0;
}
	
static int32_t stk_tune_zero_get_ps_data(struct stk3x1x_data *ps_data)
{
	uint32_t ps_adc;
	int ret;
	
	ret = stk_ps_tune_zero_val(ps_data);	
	if(ret == 0xFFFF)
	{		
		ps_data->data_count = -1;
		stk_ps_tune_zero_final(ps_data);
		return 0;
	}	
	
	ps_adc = stk3x1x_get_ps_reading(ps_data);
	printk(KERN_INFO "%s: ps_adc #%d=%d\n", __func__, ps_data->data_count, ps_adc);
	if(ps_adc < 0)		
		return ps_adc;		
	
	ps_data->ps_stat_data[1]  +=  ps_adc;			
	if(ps_adc > ps_data->ps_stat_data[0])
		ps_data->ps_stat_data[0] = ps_adc;
	if(ps_adc < ps_data->ps_stat_data[2])
		ps_data->ps_stat_data[2] = ps_adc;						
	ps_data->data_count++;	
	
	if(ps_data->data_count == 5)
	{
		ps_data->ps_stat_data[1]  /= ps_data->data_count;			
		stk_ps_tune_zero_final(ps_data);
	}		
	
	return 0;
}

static int stk_ps_tune_zero_init(struct stk3x1x_data *ps_data)
{
	int32_t ret = 0;
	uint8_t w_state_reg;	
	
	ps_data->psi_set = 0;	
	ps_data->tune_zero_init_proc = true;		
	ps_data->ps_stat_data[0] = 0;
	ps_data->ps_stat_data[2] = 9999;
	ps_data->ps_stat_data[1] = 0;
	ps_data->data_count = 0;
	
	ret = stk3x1x_i2c_smbus_write_byte_data(ps_data->client, STK_INT_REG, 0);
	if (ret < 0)
	{
		printk(KERN_ERR "%s: write i2c error\n", __func__);
		return ret;
	}			
	
	w_state_reg = (STK_STATE_EN_PS_MASK | STK_STATE_EN_WAIT_MASK);			
	ret = stk3x1x_i2c_smbus_write_byte_data(ps_data->client, STK_STATE_REG, w_state_reg);
	if (ret < 0)
	{
		printk(KERN_ERR "%s: write i2c error\n", __func__);
		return ret;
	}		
	hrtimer_start(&ps_data->ps_tune0_timer, ps_data->ps_tune0_delay, HRTIMER_MODE_REL);		
	return 0;	
}

static int stk_ps_tune_zero_func_fae(struct stk3x1x_data *ps_data)
{
	int32_t word_data;
	int ret, diff;
	unsigned char value[2];

	//STK add for CCI start 20130430 dust issue for variation in 2 consequential calls start
	static int32_t temp_max_min_diff = 0;
	//STK add for CCI start 20130430 dust issue for variation in 2 consequential calls end
	if(ps_data->psi_set || !(ps_data->ps_enabled))
	{
		printk(KERN_INFO "%s: FAE tune0 close\n", __func__);
		return 0;
	}	
	//printk(KERN_INFO "%s: FAE tune0 entry\n", __func__);

	ret = stk3x1x_get_flag(ps_data);
	if(ret < 0)
	{
		printk(KERN_ERR "%s: get flag failed, err=0x%x\n", __func__, ret);
		return ret;
	}
	if(!(ret&STK_FLG_PSDR_MASK))
	{
		//printk(KERN_INFO "%s: ps data is not ready yet\n", __func__);
		return 0;
	}
	
	ret = stk_ps_tune_zero_val(ps_data);	
	if(ret == 0)
	{
		ret = stk3x1x_i2c_read_data(ps_data->client, 0x11, 2, &value[0]);
		if(ret < 0)
		{
			printk(KERN_ERR "%s fail, ret=0x%x", __func__, ret);
			return ret;
		}
		word_data = (value[0]<<8) | value[1];						
		//printk(KERN_INFO "%s: word_data=%d\n", __func__, word_data);
		printk(KERN_INFO "[stk]word_data=%d\n", word_data);
		
		if(word_data == 0)
		{
			printk(KERN_ERR "%s: incorrect word data (0)\n", __func__);
			return 0xFFFF;
		}
		
		if(word_data > ps_data->psa)
		{
			//STK add for CCI start 20130430 dust issue for variation in 2 consequential calls start
			if(ps_data->psa == 0){
				temp_max_min_diff = word_data>>3;
				}
			//STK add for CCI start 20130430 dust issue for variation in 2 consequential calls end
			ps_data->psa = word_data;
			//printk(KERN_INFO "%s: update psa: psa=%d,psi=%d\n", __func__, ps_data->psa, ps_data->psi);
			printk(KERN_INFO "[stk]update psa: psa=%d,psi=%d\n", ps_data->psa, ps_data->psi);
		}
		if(word_data < ps_data->psi)
		{
			ps_data->psi = word_data;	
			//printk(KERN_INFO "%s: update psi: psa=%d,psi=%d\n", __func__, ps_data->psa, ps_data->psi);	
			printk(KERN_INFO "[stk]update psi: psa=%d,psi=%d\n", ps_data->psa, ps_data->psi);	
		}	
	}	
	diff = ps_data->psa - ps_data->psi;
	//STK add for CCI start 20130430 dust issue for variation in 2 consequential calls start
	//if(diff > STK_MAX_MIN_DIFF)
	if(diff > STK_MAX_MIN_DIFF && diff > temp_max_min_diff)
	//STK add for CCI start 20130430 dust issue for variation in 2 consequential calls start
	{
		ps_data->psi_set = ps_data->psi;

		//STK add for CCI start 20130112
		ps_data->psi_set = MAX_COMPARE(ps_data->psi_set, cci_result_ct);
		//STK add for CCI end 20130112
		
		//STK add for CCI start 20130112
		if(cci_ps_low_thd==0 || cci_ps_high_thd==0)
		{
			//printk(KERN_INFO "%s: [stk] use STK default value\n", __FUNCTION__);		
			printk(KERN_INFO "[stk] use STK default value\n");		
			ps_data->ps_thd_h = ps_data->psi_set + STK_HT_N_CT;
			ps_data->ps_thd_l = ps_data->psi_set + STK_LT_N_CT;	
		}
		else
		{
			//printk(KERN_INFO "%s: [stk] use CCI calibration value: cci_ps_high_thd = %d, cci_ps_low_thd = %d\n", __FUNCTION__, cci_ps_high_thd, cci_ps_low_thd);
			printk(KERN_INFO "[stk] use CCI calibration value: cci_ps_high_thd = %d, cci_ps_low_thd = %d\n", cci_ps_high_thd, cci_ps_low_thd);
			ps_data->ps_thd_h = ps_data->psi_set + cci_ps_high_thd-cci_result_ct;
			ps_data->ps_thd_l = ps_data->psi_set + cci_ps_low_thd-cci_result_ct;				
		}
		//STK add for CCI end 20130112			

		stk3x1x_set_ps_thd_h(ps_data, ps_data->ps_thd_h);
		stk3x1x_set_ps_thd_l(ps_data, ps_data->ps_thd_l);
#ifdef STK_DEBUG_PRINTF				
		//printk(KERN_INFO "%s: FAE tune0 psa-psi(%d) > STK_DIFF found\n", __func__, diff);
		printk(KERN_INFO "[stk]FAE tune0 psa-psi(%d) > STK_DIFF found\n", diff);
#endif					
		hrtimer_cancel(&ps_data->ps_tune0_timer);
	}
	
	//printk(KERN_INFO "%s: FAE tune0 exit\n", __func__);
	return 0;
}

static void stk_ps_tune0_work_func(struct work_struct *work)
{
	struct stk3x1x_data *ps_data = container_of(work, struct stk3x1x_data, stk_ps_tune0_work);		
	if(ps_data->tune_zero_init_proc)
		stk_tune_zero_get_ps_data(ps_data);
	else
		stk_ps_tune_zero_func_fae(ps_data);
	return;
}	


static enum hrtimer_restart stk_ps_tune0_timer_func(struct hrtimer *timer)
{
	struct stk3x1x_data *ps_data = container_of(timer, struct stk3x1x_data, ps_tune0_timer);
	queue_work(ps_data->stk_ps_tune0_wq, &ps_data->stk_ps_tune0_work);	
	hrtimer_forward_now(&ps_data->ps_tune0_timer, ps_data->ps_tune0_delay);
	return HRTIMER_RESTART;	
}
#endif

#ifdef STK_POLL_ALS
static enum hrtimer_restart stk_als_timer_func(struct hrtimer *timer)
{
	struct stk3x1x_data *ps_data = container_of(timer, struct stk3x1x_data, als_timer);
	queue_work(ps_data->stk_als_wq, &ps_data->stk_als_work);
	hrtimer_forward_now(&ps_data->als_timer, ps_data->als_poll_delay);
	return HRTIMER_RESTART;	

}

static void stk_als_poll_work_func(struct work_struct *work)
{
	struct stk3x1x_data *ps_data = container_of(work, struct stk3x1x_data, stk_als_work);	
	int32_t reading, reading_lux, als_comperator, flag_reg;
	
	flag_reg = stk3x1x_get_flag(ps_data);
	if(flag_reg < 0)
	{
		printk(KERN_ERR "%s: stk3x1x_get_flag fail, ret=%d", __func__, flag_reg);
		return;		
	}	
	
	if(!(flag_reg&STK_FLG_ALSDR_MASK))
	{
		return;
	}	
	
	reading = stk3x1x_get_als_reading(ps_data);
	if(reading < 0)		
	{
		return;
	}
	
	if(ps_data->ir_code)
	{
		ps_data->als_correct_factor = 1000;
		if(reading < STK_IRC_MAX_ALS_CODE && reading > STK_IRC_MIN_ALS_CODE && 
			ps_data->ir_code > STK_IRC_MIN_IR_CODE)
		{
			als_comperator = reading * STK_IRC_ALS_NUMERA / STK_IRC_ALS_DENOMI;
			if(ps_data->ir_code > als_comperator)
				ps_data->als_correct_factor = STK_IRC_ALS_CORREC;
		}
		printk(KERN_INFO "%s: als=%d, ir=%d, als_correct_factor=%d", __func__, reading, ps_data->ir_code, ps_data->als_correct_factor);
		ps_data->ir_code = 0;
	}	
	reading = reading * ps_data->als_correct_factor / 1000;
	
	reading_lux = stk_alscode2lux(ps_data, reading);
	if(abs(ps_data->als_lux_last - reading_lux) >= STK_ALS_CHANGE_THD)
	{
		ps_data->als_lux_last = reading_lux;
		input_report_abs(ps_data->als_input_dev, ABS_MISC, reading_lux);
		input_sync(ps_data->als_input_dev);
		printk(KERN_INFO "%s: als input event %d lux\n",__func__, reading_lux);		
	}
	return;
}
#endif /* #ifdef STK_POLL_ALS */


#ifdef STK_POLL_PS	
static enum hrtimer_restart stk_ps_timer_func(struct hrtimer *timer)
{
	struct stk3x1x_data *ps_data = container_of(timer, struct stk3x1x_data, ps_timer);
	queue_work(ps_data->stk_ps_wq, &ps_data->stk_ps_work);
	hrtimer_forward_now(&ps_data->ps_timer, ps_data->ps_poll_delay);
	return HRTIMER_RESTART;		
}

static void stk_ps_poll_work_func(struct work_struct *work)
{
	struct stk3x1x_data *ps_data = container_of(work, struct stk3x1x_data, stk_ps_work);	
	uint32_t reading;
	int32_t near_far_state;
    uint8_t org_flag_reg;	
	int32_t ret;
    uint8_t disable_flag = 0;

#ifdef STK_TUNE0
	if(!(ps_data->psi_set) || !(ps_data->ps_enabled))
		return;	
#endif	
	org_flag_reg = stk3x1x_get_flag(ps_data);
	if(org_flag_reg < 0)
	{
		printk(KERN_ERR "%s: stk3x1x_get_flag fail, ret=%d", __func__, org_flag_reg);
		goto err_i2c_rw;		
	}	

	if(!(org_flag_reg&STK_FLG_PSDR_MASK))
	{
		return;
	}	
	
	near_far_state = (org_flag_reg & STK_FLG_NF_MASK)?1:0;	
	reading = stk3x1x_get_ps_reading(ps_data);
	if(ps_data->ps_distance_last != near_far_state)
	{
		ps_data->ps_distance_last = near_far_state;
		input_report_abs(ps_data->ps_input_dev, ABS_DISTANCE, near_far_state);
		input_sync(ps_data->ps_input_dev);
		wake_lock_timeout(&ps_data->ps_wakelock, 3*HZ);		
#ifdef STK_DEBUG_PRINTF		
		printk(KERN_INFO "%s: ps input event %d cm, ps code = %d\n",__func__, near_far_state, reading);		
#endif		
	}
	ret = stk3x1x_set_flag(ps_data, org_flag_reg, disable_flag);		
	if(ret < 0)
	{
		printk(KERN_ERR "%s:stk3x1x_set_flag fail, ret=%d\n", __func__, ret);
		goto err_i2c_rw;
	}	
	return;
	
err_i2c_rw:
	msleep(30);	
	return;
}
#endif

#if (!defined(STK_POLL_PS) || !defined(STK_POLL_ALS))
static void stk_work_func(struct work_struct *work)
{
	uint32_t reading;
	// CCI read H_thd/L_thd when PS interrupt trigger start
	int32_t ps_reg[4];
	uint8_t cnt;
	uint8_t i = 0;
	// CCI read H_thd/L_thd when PS interrupt trigger end
#if ((STK_INT_PS_MODE != 0x03) && (STK_INT_PS_MODE != 0x02))
    int32_t ret;
    uint8_t disable_flag = 0;
    uint8_t org_flag_reg;
#endif	/* #if ((STK_INT_PS_MODE != 0x03) && (STK_INT_PS_MODE != 0x02)) */

#ifndef CONFIG_STK_PS_ALS_USE_CHANGE_THRESHOLD	
	uint32_t nLuxIndex;	
#endif
	struct stk3x1x_data *ps_data = container_of(work, struct stk3x1x_data, stk_work);	
	int32_t near_far_state;
	int32_t als_comperator;
	
#if (STK_INT_PS_MODE	== 0x03)
	near_far_state = gpio_get_value(ps_data->int_pin);
#elif	(STK_INT_PS_MODE	== 0x02)
	near_far_state = !(gpio_get_value(ps_data->int_pin));
#endif	

#if ((STK_INT_PS_MODE == 0x03) || (STK_INT_PS_MODE	== 0x02))
	ps_data->ps_distance_last = near_far_state;
	input_report_abs(ps_data->ps_input_dev, ABS_DISTANCE, near_far_state);
	input_sync(ps_data->ps_input_dev);
	wake_lock_timeout(&ps_data->ps_wakelock, 3*HZ);
	reading = stk3x1x_get_ps_reading(ps_data);
#ifdef STK_DEBUG_PRINTF	
	printk(KERN_INFO "%s: ps input event %d cm, ps code = %d\n",__func__, near_far_state, reading);			
#endif	
#else
	/* mode 0x01 or 0x04 */	
	org_flag_reg = stk3x1x_get_flag(ps_data);
	if(org_flag_reg < 0)
	{	
		printk(KERN_ERR "%s: stk3x1x_get_flag fail, org_flag_reg=%d", __func__, org_flag_reg);
		goto err_i2c_rw;	
	}	
	
    if (org_flag_reg & STK_FLG_ALSINT_MASK)
    {
		disable_flag |= STK_FLG_ALSINT_MASK;
        reading = stk3x1x_get_als_reading(ps_data);
		if(reading < 0)		
		{
			printk(KERN_ERR "%s: stk3x1x_get_als_reading fail, ret=%d", __func__, reading);
			goto err_i2c_rw;
		}			
#ifndef CONFIG_STK_PS_ALS_USE_CHANGE_THRESHOLD
        nLuxIndex = stk_get_lux_interval_index(reading);
        stk3x1x_set_als_thd_h(ps_data, code_threshold_table[nLuxIndex]);
        stk3x1x_set_als_thd_l(ps_data, code_threshold_table[nLuxIndex-1]);
#else
        stk_als_set_new_thd(ps_data, reading);
#endif //CONFIG_STK_PS_ALS_USE_CHANGE_THRESHOLD

		if(ps_data->ir_code)
		{
			if(reading < STK_IRC_MAX_ALS_CODE && reading > STK_IRC_MIN_ALS_CODE && 
			ps_data->ir_code > STK_IRC_MIN_IR_CODE)
			{
				als_comperator = reading * STK_IRC_ALS_NUMERA / STK_IRC_ALS_DENOMI;
				if(ps_data->ir_code > als_comperator)
					ps_data->als_correct_factor = STK_IRC_ALS_CORREC;
				else
					ps_data->als_correct_factor = 1000;
			}
			printk(KERN_INFO "%s: als=%d, ir=%d, als_correct_factor=%d", __func__, reading, ps_data->ir_code, ps_data->als_correct_factor);
			ps_data->ir_code = 0;
		}	

		reading = reading * ps_data->als_correct_factor / 1000;

		ps_data->als_lux_last = stk_alscode2lux(ps_data, reading);
		input_report_abs(ps_data->als_input_dev, ABS_MISC, ps_data->als_lux_last);
		input_sync(ps_data->als_input_dev);
#ifdef STK_DEBUG_PRINTF		
		printk(KERN_INFO "%s: als input event %d lux\n",__func__, ps_data->als_lux_last);			
#endif		
    }
    if (org_flag_reg & STK_FLG_PSINT_MASK)
    {
		disable_flag |= STK_FLG_PSINT_MASK;
		near_far_state = (org_flag_reg & STK_FLG_NF_MASK)?1:0;
		
		ps_data->ps_distance_last = near_far_state;
		input_report_abs(ps_data->ps_input_dev, ABS_DISTANCE, near_far_state);
		input_sync(ps_data->ps_input_dev);
		wake_lock_timeout(&ps_data->ps_wakelock, 3*HZ);			
		reading = stk3x1x_get_ps_reading(ps_data);

		//add for FTM interrupt check 20130424 start
		CCI_FTM_INTERRUPT++;
		printk(KERN_ERR "%s: CCI_FTM_INTERRUPT = %d\n", __func__, CCI_FTM_INTERRUPT);
		//add for FTM interrupt check 20130424 end
		// CCI read H_thd/L_thd when PS interrupt trigger start
		for(cnt=6;cnt<=9;cnt++){
			ps_reg[i] = stk3x1x_i2c_smbus_read_byte_data(ps_data->client, (cnt));
			if(ps_reg[i] < 0){
				printk(KERN_ERR "%s fail, ret=%d", __func__, ps_reg[i]);	
				}
			//else{
				//printk(KERN_INFO "reg[0x%2X]=0x%2X\n", cnt, ps_reg[i]);
				//}
			i++;
			}
		printk(KERN_ERR "%s: stk_work_func() read H/L THD from register when interrupt trigger => H_THD = %2X, %2X, L_THD = %2X, %2X\n", __func__, ps_reg[0], ps_reg[1], ps_reg[2], ps_reg[3]);
		// CCI read H_thd/L_thd when PS interrupt trigger end
#ifdef STK_DEBUG_PRINTF		
		printk(KERN_INFO "%s: ps input event=%d, ps code = %d\n",__func__, near_far_state, reading);
#endif			
	}
	
    ret = stk3x1x_set_flag(ps_data, org_flag_reg, disable_flag);		
	if(ret < 0)
	{
		printk(KERN_ERR "%s:reset_int_flag fail, ret=%d\n", __func__, ret);
		goto err_i2c_rw;
	}		
#endif	

	msleep(1);
    enable_irq(ps_data->irq);
	return;

err_i2c_rw:
	msleep(30);
	enable_irq(ps_data->irq);
	return;	
}

static irqreturn_t stk_oss_irq_handler(int irq, void *data)
{
	struct stk3x1x_data *pData = data;
	disable_irq_nosync(irq);
	queue_work(pData->stk_wq,&pData->stk_work);
	return IRQ_HANDLED;
}
#endif	/*	#if (!defined(STK_POLL_PS) || !defined(STK_POLL_ALS))	*/
static int32_t stk3x1x_init_all_setting(struct i2c_client *client, struct stk3x1x_platform_data *plat_data)
{
	int32_t ret;
	struct stk3x1x_data *ps_data = i2c_get_clientdata(client);		
	
	ps_data->als_enabled = false;
	ps_data->ps_enabled = false;
	
	ret = stk3x1x_software_reset(ps_data); 
	if(ret < 0)
		return ret;
	
	ret = stk3x1x_check_pid(ps_data);
	if(ret < 0)
		return ret;
	
	ret = stk3x1x_init_all_reg(ps_data, plat_data);
	if(ret < 0)
		return ret;	
	ps_data->re_enable_als = false;
	ps_data->ir_code = 0;
	ps_data->als_correct_factor = 1000;
#ifndef CONFIG_STK_PS_ALS_USE_CHANGE_THRESHOLD
	stk_init_code_threshold_table(ps_data);
#endif			
	ps_data->first_boot = true;	
#ifdef STK_TUNE0
	stk_ps_tune_zero_init(ps_data);
#endif	
#ifdef STK_ALS_FIR
	memset(&ps_data->fir, 0x00, sizeof(ps_data->fir));  
#endif
#ifdef STK_POLL_PS
	ps_data->debounce_counter = 0;
#endif	
    return 0;
}

#if (!defined(STK_POLL_PS) || !defined(STK_POLL_ALS))
static int stk3x1x_setup_irq(struct i2c_client *client)
{		
	int irq, err = -EIO;
	struct stk3x1x_data *ps_data = i2c_get_clientdata(client);

#ifdef SPREADTRUM_PLATFORM	
	irq = sprd_alloc_gpio_irq(ps_data->int_pin);	
#else	
	irq = gpio_to_irq(ps_data->int_pin);
#endif	
#ifdef STK_DEBUG_PRINTF	
	printk(KERN_INFO "%s: int pin #=%d, irq=%d\n",__func__, ps_data->int_pin, irq);	
#endif	
	if (irq <= 0)
	{
		printk(KERN_ERR "irq number is not specified, irq # = %d, int pin=%d\n",irq, ps_data->int_pin);
		return irq;
	}
	ps_data->irq = irq;
	//Colby mark for PS interrupt mode, GPIO already enable in board-msm7627a-sensor.c start
//Colby add for BroadCom start
#ifdef BROADCOM_PLATFORM
	err = gpio_request(ps_data->int_pin,"stk-int");
	if(err < 0)
	{
			printk(KERN_ERR "%s: [STK]gpio_request, err=%d", __func__, err);
			return err;
	}
	err = gpio_direction_input(ps_data->int_pin);
	if(err < 0)
	{
			printk(KERN_ERR "%s: [STK]gpio_direction_input, err=%d", __func__, err);
			return err;
	}
//Colby add for BroadCom end
//Colby add for Qualcomm device tree start
#elif defined(QUALCOMM_DEVICETREE_PLATFORM)
	err = gpio_request(ps_data->int_pin,"stk-int");
	if(err < 0)
	{
			printk(KERN_ERR "%s: [STK]gpio_request, err=%d", __func__, err);
			return err;
	}

	printk(KERN_ERR "%s: [CCI]gpio_tlmm_config => int_pin(%d), GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA", __func__, ps_data->int_pin);
	gpio_tlmm_config(GPIO_CFG(ps_data->int_pin, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), GPIO_CFG_ENABLE);

	err = gpio_direction_input(ps_data->int_pin);
	if(err < 0)
	{
			printk(KERN_ERR "%s: [STK]gpio_direction_input, err=%d", __func__, err);
			return err;
	}		
#else
//Colby add for Qualcomm device tree end
	//err = gpio_request(ps_data->int_pin,"stk-int");
	//if(err < 0)
	//{
		//	printk(KERN_ERR "%s: [STK]gpio_request, err=%d", __func__, err);
		//	return err;
	//}
	//err = gpio_direction_input(ps_data->int_pin);
	//if(err < 0)
	//{
		//	printk(KERN_ERR "%s: [STK]gpio_direction_input, err=%d", __func__, err);
		//	return err;
	//}			
//Colby add for BroadCom start
#endif
//Colby add for BroadCom end
	//Colby mark for PS interrupt mode, GPIO already enable in board-msm7627a-sensor.c end
	
#if ((STK_INT_PS_MODE == 0x03) || (STK_INT_PS_MODE	== 0x02))	
	err = request_any_context_irq(irq, stk_oss_irq_handler, IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING, DEVICE_NAME, ps_data);
#else	
	//err = request_any_context_irq(irq, stk_oss_irq_handler, IRQF_TRIGGER_LOW, DEVICE_NAME, ps_data);
//Colby add for BroadCom start
#ifdef BROADCOM_PLATFORM
	printk(KERN_ERR "%s: [STK]BROADCOM_PLATFORM request irq for stk", __func__);
	err = request_threaded_irq(irq, NULL, stk_oss_irq_handler, IRQF_TRIGGER_FALLING | IRQF_ONESHOT, DEVICE_NAME, ps_data);
	if(err != 0){
		printk(KERN_ERR "%s: [STK]request irq fail for stk", __func__);
		}
#else
//Colby add for BroadCom end
	err = request_any_context_irq(irq, stk_oss_irq_handler, IRQF_TRIGGER_LOW, DEVICE_NAME, ps_data);
//Colby add for BroadCom start
#endif
//Colby add for BroadCom end
#endif	
	if (err < 0) 
	{
		printk(KERN_WARNING "%s: request_any_context_irq(%d) failed for (%d)\n", __func__, irq, err);		
		goto err_request_any_context_irq;
	}
	disable_irq(irq);
	
	return 0;
err_request_any_context_irq:	
#ifdef SPREADTRUM_PLATFORM
	sprd_free_gpio_irq(ps_data->int_pin);	
#else	
	gpio_free(ps_data->int_pin);		
#endif	
	return err;
}
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
static void stk3x1x_early_suspend(struct early_suspend *h)
{
	struct stk3x1x_data *ps_data = container_of(h, struct stk3x1x_data, stk_early_suspend);	
#ifndef STK_POLL_PS	
	int err;
#endif
	printk(KERN_ERR "%s: [#23][STK]stk3x1x_early_suspend ++\n", __func__);	
#ifndef SPREADTRUM_PLATFORM	
    mutex_lock(&ps_data->io_lock);  		
	if(ps_data->als_enabled)
	{	
		stk3x1x_enable_als(ps_data, 0);		
		ps_data->re_enable_als = true;
	} 
#endif	
	if(ps_data->ps_enabled)
	{
#ifdef STK_POLL_PS		
		wake_lock(&ps_data->ps_nosuspend_wl);
#else		
		err = enable_irq_wake(ps_data->irq);	
		if (err)
			printk(KERN_WARNING "%s: set_irq_wake(%d) failed, err=(%d)\n", __func__, ps_data->irq, err);				
#endif	
	}
#ifndef SPREADTRUM_PLATFORM		
	mutex_unlock(&ps_data->io_lock);		
#endif	
	printk(KERN_ERR "%s: [#23][STK]stk3x1x_early_suspend --\n", __func__);
	return;
}

static void stk3x1x_late_resume(struct early_suspend *h)
{
	struct stk3x1x_data *ps_data = container_of(h, struct stk3x1x_data, stk_early_suspend);	
#ifndef STK_POLL_PS	    
	int err;
#endif
	printk(KERN_ERR "%s: [#23][STK]stk3x1x_late_resume ++\n", __func__);	
#ifndef SPREADTRUM_PLATFORM		
    mutex_lock(&ps_data->io_lock); 		
	if(ps_data->re_enable_als)
	{
		stk3x1x_enable_als(ps_data, 1);		
		ps_data->re_enable_als = false;		
	}
#endif
	if(ps_data->ps_enabled)
	{
#ifdef STK_POLL_PS		
		wake_unlock(&ps_data->ps_nosuspend_wl);		
#else		
		err = disable_irq_wake(ps_data->irq);	
		if (err)		
			printk(KERN_WARNING "%s: disable_irq_wake(%d) failed, err=(%d)\n", __func__, ps_data->irq, err);		
#endif	
	}
#ifndef SPREADTRUM_PLATFORM			
	mutex_unlock(&ps_data->io_lock);
#endif
	printk(KERN_ERR "%s: [#23][STK]stk3x1x_late_resume --\n", __func__);
	return;
}
#endif	//#ifdef CONFIG_HAS_EARLYSUSPEND

//add Suspend/Resume for VY52 start
#ifdef CONFIG_PM
static int stk3x1x_suspend(struct device *dev)
{
	struct stk3x1x_data *ps_data = dev_get_drvdata(dev);

#ifndef STK_POLL_PS
	int err;
#endif
	printk(KERN_ERR "%s: [#23][STK]stk3x1x_suspend ++\n", __func__);
#ifndef SPREADTRUM_PLATFORM
    mutex_lock(&ps_data->io_lock);
	if(ps_data->als_enabled)
	{
		stk3x1x_enable_als(ps_data, 0);
		ps_data->re_enable_als = true;
	}
#endif
	if(ps_data->ps_enabled)
	{
#ifdef STK_POLL_PS
		wake_lock(&ps_data->ps_nosuspend_wl);
#else
		err = enable_irq_wake(ps_data->irq);
		if (err)
			printk(KERN_WARNING "%s: set_irq_wake(%d) failed, err=(%d)\n", __func__, ps_data->irq, err);
#endif
	}
#ifndef SPREADTRUM_PLATFORM
	mutex_unlock(&ps_data->io_lock);
#endif
	printk(KERN_ERR "%s: [#23][STK]stk3x1x_suspend --\n", __func__);
	return 0;
}

static int stk3x1x_resume(struct device *dev)
{
	struct stk3x1x_data *ps_data = dev_get_drvdata(dev);

#ifndef STK_POLL_PS
	int err;
#endif
	printk(KERN_ERR "%s: [#23][STK]stk3x1x_resume ++\n", __func__);
#ifndef SPREADTRUM_PLATFORM
    mutex_lock(&ps_data->io_lock);
	if(ps_data->re_enable_als)
	{
		stk3x1x_enable_als(ps_data, 1);
		ps_data->re_enable_als = false;
	}
#endif
	if(ps_data->ps_enabled)
	{
#ifdef STK_POLL_PS
		wake_unlock(&ps_data->ps_nosuspend_wl);
#else
		err = disable_irq_wake(ps_data->irq);
		if (err)
			printk(KERN_WARNING "%s: disable_irq_wake(%d) failed, err=(%d)\n", __func__, ps_data->irq, err);
#endif
	}
#ifndef SPREADTRUM_PLATFORM
	mutex_unlock(&ps_data->io_lock);
#endif
	printk(KERN_ERR "%s: [#23][STK]stk3x1x_resume --\n", __func__);
	return 0;
}

static const struct dev_pm_ops stk3x1x_mt_pm_ops = {
	.suspend	= stk3x1x_suspend,
	.resume		= stk3x1x_resume,
};
#endif /* CONFIG_PM */
//add Suspend/Resume for VY52 end

static int stk3x1x_probe(struct i2c_client *client,
                        const struct i2c_device_id *id)
{
    int err = -ENODEV;
    struct stk3x1x_data *ps_data;
	struct stk3x1x_platform_data *plat_data;
//Colby add for BroadCom start
	//struct device_node *np;
//Colby add for BroadCom end

//Colby add for Qualcomm device tree start
#ifdef QUALCOMM_DEVICETREE_PLATFORM
	//static struct regulator *reg_l20;
	static struct regulator *reg_l19;
	static struct regulator *reg_lvs1;
	int error;

//power on start
printk(KERN_INFO "%s: power on start\n", __func__);
//Colby add Power on 
	//get power and set voltage level
	reg_l19 = regulator_get(&client->dev, "vdd_ana");
	if (IS_ERR(reg_l19)) {
		printk("[CCI]%s: Regulator get failed vcc_ana rc=%ld\n", __FUNCTION__, PTR_ERR(reg_l19));
	}
	if (regulator_count_voltages(reg_l19) > 0) {
		error = regulator_set_voltage(reg_l19,  2850000, 2850000);
		if (error) {
			printk("[CCI]%s: regulator set_vtg vdd_ana failed rc=%d\n", __FUNCTION__, error);
		}
	}

	//reg_l20 = regulator_get(&client->dev, "vdd_led");
	//if (IS_ERR(reg_l20)) {
	//	printk("[CCI]%s: Regulator get vdd_bus failed rc=%ld\n", __FUNCTION__, PTR_ERR(reg_l20));
	//}
	//if (regulator_count_voltages(reg_l20) > 0) {
	//	error = regulator_set_voltage(reg_l20, 2950000, 2950000);
	//	if (error) {
	//		printk("[CCI]%s: regulator set_vtg vdd_bus failed rc=%d\n", __FUNCTION__, error);
	//	}
	//}

	reg_lvs1 = regulator_get(&client->dev,"vcc_i2c");
	if (IS_ERR(reg_lvs1)){
		printk("[CCI]could not get 8924_lvs1, rc = %ld\n", PTR_ERR(reg_lvs1));
		}

	//enable power

	error = regulator_set_optimum_mode(reg_l19, 100000);
	if (error < 0) {
		printk("[CCI]%s: Regulator vcc_ana set_opt failed rc=%d\n", __FUNCTION__, error);
		regulator_put(reg_l19);
	}

	//error = regulator_set_optimum_mode(reg_l20, 100000);
	//if (error < 0) {
	//	printk("[CCI]%s: Regulator vdd_bus set_opt failed rc=%d\n", __FUNCTION__, error);
	//	regulator_put(reg_l20);
	//}

	error = regulator_enable(reg_l19);
	if (error) {
		printk("[CCI]%s: Regulator vcc_ana enable failed rc=%d\n", __FUNCTION__, error);
		regulator_put(reg_l19);
	}

	//error = regulator_enable(reg_l20);
	//if (error) {
	//	printk("[CCI]%s: Regulator vdd_bus enable failed rc=%d\n", __FUNCTION__, error);
	//	regulator_put(reg_l20);
	//}

	error = regulator_enable(reg_lvs1);
	if (error) {
		pr_err("[CCI]enable lvs1 failed, rc=%d\n", error);
		regulator_put(reg_lvs1);
	}
//power on end
printk(KERN_INFO "%s: power on end\n", __func__);
#endif
//Colby add for Qualcomm device tree end
	mdelay(3);// delay 3 ms for power ready when issue first I2C command
    printk(KERN_INFO "%s: driver version = %s\n", __func__, DRIVER_VERSION);
	
    if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
    {
        printk(KERN_ERR "%s: No Support for I2C_FUNC_I2C\n", __func__);
        return -ENODEV;
    }

	ps_data = kzalloc(sizeof(struct stk3x1x_data),GFP_KERNEL);
	if(!ps_data)
	{
		printk(KERN_ERR "%s: failed to allocate stk3x1x_data\n", __func__);
		return -ENOMEM;
	}
	ps_data->client = client;
	i2c_set_clientdata(client,ps_data);
	mutex_init(&ps_data->io_lock);
	wake_lock_init(&ps_data->ps_wakelock,WAKE_LOCK_SUSPEND, "stk_input_wakelock");

#ifdef STK_POLL_PS			
	wake_lock_init(&ps_data->ps_nosuspend_wl,WAKE_LOCK_SUSPEND, "stk_nosuspend_wakelock");
#endif	
#ifdef SPREADTRUM_PLATFORM
	if(&stk3x1x_pfdata != NULL)		
//Colby add for BroadCom start
#elif defined(BROADCOM_PLATFORM)
	printk(KERN_ERR "%s: stk3x1x_pfdata_BCM\n", __func__);
	if(&stk3x1x_pfdata_BCM != NULL)	
	//if(plat_data != NULL)
//Colby add for BroadCom end
//Colby add for Qualcomm device tree start
#elif defined(QUALCOMM_DEVICETREE_PLATFORM)
	printk(KERN_ERR "%s: stk3x1x_pfdata_QCT\n", __func__);
	//if(&stk3x1x_pfdata_QCT != NULL)	
	if(1)
	//if(plat_data != NULL)
//Colby add for Qualcomm device tree end
#else
	if(client->dev.platform_data != NULL)		
#endif
	{
#ifdef SPREADTRUM_PLATFORM		
		plat_data = &stk3x1x_pfdata;	

//Colby add for BroadCom start
#elif defined(BROADCOM_PLATFORM)
	printk(KERN_ERR "%s: set stk3x1x_pfdata_BCM to plat_data\n", __func__);
	plat_data = &stk3x1x_pfdata_BCM;	
//Colby add for BroadCom end

//Colby add for Qualcomm device tree start
#elif defined(QUALCOMM_DEVICETREE_PLATFORM)
	printk(KERN_ERR "%s: set stk3x1x_pfdata_QCT to plat_data\n", __func__);
	plat_data = &stk3x1x_pfdata_QCT;	
//Colby add for Qualcomm device tree end

#else
		plat_data = client->dev.platform_data;	
#endif		
		ps_data->als_transmittance = plat_data->transmittance;			
		ps_data->int_pin = plat_data->int_pin;		
		if(ps_data->als_transmittance == 0)
		{
			printk(KERN_ERR "%s: Please set als_transmittance in platform data\n", __func__);
			goto err_als_input_allocate;
		}
	}
	else
	{
		printk(KERN_ERR "%s: no stk3x1x platform data!\n", __func__);		
		goto err_als_input_allocate;
	}		
	
	ps_data->als_input_dev = input_allocate_device();
	if (ps_data->als_input_dev==NULL)
	{
		printk(KERN_ERR "%s: could not allocate als device\n", __func__);
		err = -ENOMEM;
		goto err_als_input_allocate;
	}
	ps_data->ps_input_dev = input_allocate_device();
	if (ps_data->ps_input_dev==NULL)
	{
		printk(KERN_ERR "%s: could not allocate ps device\n", __func__);		
		err = -ENOMEM;
		goto err_ps_input_allocate;		
	}
	ps_data->als_input_dev->name = ALS_NAME;
	ps_data->ps_input_dev->name = PS_NAME;
	set_bit(EV_ABS, ps_data->als_input_dev->evbit);
	set_bit(EV_ABS, ps_data->ps_input_dev->evbit);
	input_set_abs_params(ps_data->als_input_dev, ABS_MISC, 0, stk_alscode2lux(ps_data, (1<<16)-1), 0, 0);
	input_set_abs_params(ps_data->ps_input_dev, ABS_DISTANCE, 0,1, 0, 0);
	err = input_register_device(ps_data->als_input_dev);
	if (err<0)
	{
		printk(KERN_ERR "%s: can not register als input device\n", __func__);		
		goto err_als_input_register;
	}
	err = input_register_device(ps_data->ps_input_dev);	
	if (err<0)
	{
		printk(KERN_ERR "%s: can not register ps input device\n", __func__);	
		goto err_ps_input_register;
	}

	err = sysfs_create_group(&ps_data->als_input_dev->dev.kobj, &stk_als_attribute_group);
	if (err < 0) 
	{
		printk(KERN_ERR "%s:could not create sysfs group for als\n", __func__);
		goto err_als_sysfs_create_group;
	}
	err = sysfs_create_group(&ps_data->ps_input_dev->dev.kobj, &stk_ps_attribute_group);
	if (err < 0) 
	{
		printk(KERN_ERR "%s:could not create sysfs group for ps\n", __func__);
		goto err_ps_sysfs_create_group;
	}
	input_set_drvdata(ps_data->als_input_dev, ps_data);
	input_set_drvdata(ps_data->ps_input_dev, ps_data);	
	
#ifdef STK_POLL_ALS	
	ps_data->stk_als_wq = create_singlethread_workqueue("stk_als_wq");
	INIT_WORK(&ps_data->stk_als_work, stk_als_poll_work_func);
	hrtimer_init(&ps_data->als_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	ps_data->als_poll_delay = ns_to_ktime(110 * NSEC_PER_MSEC);
	ps_data->als_timer.function = stk_als_timer_func;
#endif	

#ifdef STK_POLL_PS	
	ps_data->stk_ps_wq = create_singlethread_workqueue("stk_ps_wq");
	INIT_WORK(&ps_data->stk_ps_work, stk_ps_poll_work_func);
	hrtimer_init(&ps_data->ps_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	ps_data->ps_poll_delay = ns_to_ktime(60 * NSEC_PER_MSEC);
	ps_data->ps_timer.function = stk_ps_timer_func;
#endif	

#ifdef STK_TUNE0
	ps_data->stk_ps_tune0_wq = create_singlethread_workqueue("stk_ps_tune0_wq");
	INIT_WORK(&ps_data->stk_ps_tune0_work, stk_ps_tune0_work_func);
	hrtimer_init(&ps_data->ps_tune0_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	ps_data->ps_tune0_delay = ns_to_ktime(60 * NSEC_PER_MSEC);
	ps_data->ps_tune0_timer.function = stk_ps_tune0_timer_func;
#endif

#if (!defined(STK_POLL_ALS) || !defined(STK_POLL_PS))
	ps_data->stk_wq = create_singlethread_workqueue("stk_wq");
	INIT_WORK(&ps_data->stk_work, stk_work_func);

	err = stk3x1x_setup_irq(client);
	if(err < 0)
		goto err_stk3x1x_setup_irq;
#endif		
	device_init_wakeup(&client->dev, true);
	err = stk3x1x_init_all_setting(client, plat_data);
	if(err < 0)
		goto err_init_all_setting;			
#ifdef CONFIG_HAS_EARLYSUSPEND
	ps_data->stk_early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	ps_data->stk_early_suspend.suspend = stk3x1x_early_suspend;
	ps_data->stk_early_suspend.resume = stk3x1x_late_resume;
	register_early_suspend(&ps_data->stk_early_suspend);
#endif
	printk(KERN_INFO "%s: probe successfully", __func__);
	return 0;

err_init_all_setting:	
	device_init_wakeup(&client->dev, false);
#ifndef STK_POLL_PS
	free_irq(ps_data->irq, ps_data);
	#ifdef SPREADTRUM_PLATFORM	
		sprd_free_gpio_irq(ps_data->int_pin);		
	#else	
		gpio_free(ps_data->int_pin);	
	#endif	
#endif	/* #ifndef STK_POLL_PS	*/	
#if (!defined(STK_POLL_ALS) || !defined(STK_POLL_PS))
err_stk3x1x_setup_irq:
#endif
#ifdef STK_POLL_ALS		
	hrtimer_try_to_cancel(&ps_data->als_timer);
	destroy_workqueue(ps_data->stk_als_wq);	
#endif	
#ifdef STK_TUNE0
	destroy_workqueue(ps_data->stk_ps_tune0_wq);	
#endif	
#ifdef STK_POLL_PS	
	hrtimer_try_to_cancel(&ps_data->ps_timer);	
	destroy_workqueue(ps_data->stk_ps_wq);	
#endif	
#if (!defined(STK_POLL_ALS) || !defined(STK_POLL_PS))
	destroy_workqueue(ps_data->stk_wq);	
#endif	
	sysfs_remove_group(&ps_data->ps_input_dev->dev.kobj, &stk_ps_attribute_group);	
err_ps_sysfs_create_group:	
	sysfs_remove_group(&ps_data->als_input_dev->dev.kobj, &stk_als_attribute_group);	
err_als_sysfs_create_group:	
	input_unregister_device(ps_data->ps_input_dev);		
err_ps_input_register:
	input_unregister_device(ps_data->als_input_dev);	
err_als_input_register:	
	input_free_device(ps_data->ps_input_dev);	
err_ps_input_allocate:	
	input_free_device(ps_data->als_input_dev);	
err_als_input_allocate:
#ifdef STK_POLL_PS
    wake_lock_destroy(&ps_data->ps_nosuspend_wl);	
#endif	
    wake_lock_destroy(&ps_data->ps_wakelock);	
    mutex_destroy(&ps_data->io_lock);
	kfree(ps_data);
    return err;
}


static int stk3x1x_remove(struct i2c_client *client)
{
	struct stk3x1x_data *ps_data = i2c_get_clientdata(client);
	device_init_wakeup(&client->dev, false);
#ifndef STK_POLL_PS	
	free_irq(ps_data->irq, ps_data);
	#ifdef SPREADTRUM_PLATFORM	
		sprd_free_gpio_irq(ps_data->int_pin);		
	#else	
		gpio_free(ps_data->int_pin);	
	#endif	
#endif	/* #ifndef STK_POLL_PS */	
#ifdef STK_POLL_ALS		
	hrtimer_try_to_cancel(&ps_data->als_timer);	
	destroy_workqueue(ps_data->stk_als_wq);	
#endif	
#ifdef STK_TUNE0
	destroy_workqueue(ps_data->stk_ps_tune0_wq);	
#endif	
#ifdef STK_POLL_PS
	hrtimer_try_to_cancel(&ps_data->ps_timer);	
	destroy_workqueue(ps_data->stk_ps_wq);	
#endif	
#if (!defined(STK_POLL_ALS) || !defined(STK_POLL_PS))
	destroy_workqueue(ps_data->stk_wq);	
#endif	
	sysfs_remove_group(&ps_data->ps_input_dev->dev.kobj, &stk_ps_attribute_group);	
	sysfs_remove_group(&ps_data->als_input_dev->dev.kobj, &stk_als_attribute_group);	
	input_unregister_device(ps_data->ps_input_dev);		
	input_unregister_device(ps_data->als_input_dev);	
	input_free_device(ps_data->ps_input_dev);	
	input_free_device(ps_data->als_input_dev);	
#ifdef STK_POLL_PS	
	wake_lock_destroy(&ps_data->ps_nosuspend_wl);	
#endif	
	wake_lock_destroy(&ps_data->ps_wakelock);	
    mutex_destroy(&ps_data->io_lock);
	kfree(ps_data);
	
    return 0;
}

static const struct i2c_device_id stk_ps_id[] =
{
    { "stk_ps", 0},
    {}
};
MODULE_DEVICE_TABLE(i2c, stk_ps_id);

static struct i2c_driver stk_ps_driver =
{
    .driver = {
        .name = DEVICE_NAME,
		.owner = THIS_MODULE,	
		//Colby add for Qualcomm device tree start
		#ifdef QUALCOMM_DEVICETREE_PLATFORM
		.of_match_table = stk3x1x_dts_table,
		#endif
		//Colby add for Qualcomm device tree end
#ifdef CONFIG_PM
		.pm = &stk3x1x_mt_pm_ops,
#endif /* CONFIG_PM */
    },
    .probe = stk3x1x_probe,
    .remove = stk3x1x_remove,
    .id_table = stk_ps_id,
};


static int __init stk3x1x_init(void)
{
	int ret;
    ret = i2c_add_driver(&stk_ps_driver);
    if (ret)
	{
		i2c_del_driver(&stk_ps_driver);
        return ret;
	}
    return 0;
}

static void __exit stk3x1x_exit(void)
{
    i2c_del_driver(&stk_ps_driver);	
}

module_init(stk3x1x_init);
module_exit(stk3x1x_exit);
MODULE_AUTHOR("Lex Hsieh <lex_hsieh@sensortek.com.tw>");
MODULE_DESCRIPTION("Sensortek stk3x1x Proximity Sensor driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRIVER_VERSION);