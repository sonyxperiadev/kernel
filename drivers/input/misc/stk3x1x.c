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
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/regulator/consumer.h> /* PERI-BJ-Add_Regular_Optimum_feature-00+ */
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif
#include <linux/of.h>
#include <linux/of_gpio.h>
#define DRIVER_VERSION  "3.5.1"

/* Driver Settings */
#define CONFIG_STK_PS_ALS_USE_CHANGE_THRESHOLD
#ifdef CONFIG_STK_PS_ALS_USE_CHANGE_THRESHOLD
#define STK_ALS_CHANGE_THD	20	/* The threshold to trigger ALS interrupt, unit: lux */
#endif	/* #ifdef CONFIG_STK_PS_ALS_USE_CHANGE_THRESHOLD */
#define STK_INT_PS_MODE			1	/* 1, 2, or 3	*/
//#define STK_POLL_PS
//#define STK_POLL_ALS		/* ALS interrupt is valid only when STK_PS_INT_MODE = 1	or 4*/
//#define STK_TUNE0
//#define STK_DEBUG_PRINTF
//#define SPREADTRUM_PLATFORM
//#define STK_ALS_FIR
//#define STK_IRS

/* PERI-BJ-Implement_New_PSensor_Detect_NonDetect_Feature-01*{ */
//the value is for 2.7cm detect and 3.2cm non-detect
#define ps_thd_h_computed_rate 40
#define ps_thd_l_computed_rate 55
/* PERI-BJ-Implement_New_PSensor_Detect_NonDetect_Feature-01*} */

/* PERI-BJ-Implement_New_PSensor_Detect_NonDetect_Feature-00+{ */
//Enable the feature while calibration done in FTM
#define FIH_CAL
/* PERI-BJ-Implement_New_PSensor_Detect_NonDetect_Feature-00+} */

#ifdef SPREADTRUM_PLATFORM
	#include "stk3x1x.h"
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
	#define STK_MAX_MIN_DIFF	200
	#define STK_LT_N_CT	100
	#define STK_HT_N_CT	150
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
  .psctrl_reg = 0x31,    /*0x71=>0x31 ps_persistance=4, ps_gain=64X, PS_IT=0.391ms */
  .alsctrl_reg = 0x38, 	/* als_persistance=1, als_gain=64X, ALS_IT=50ms */
  .ledctrl_reg = 0xFF,   /* 100mA IRDR, 64/64 LED duty */
  .wait_reg = 0x07,    /* 50 ms */
  .ps_thd_h =1700,
  .ps_thd_l = 1500,
  .int_pin = sprd_3rdparty_gpio_pls_irq,
  .transmittance = 500,
};
#endif

#ifdef STK_ALS_FIR
struct data_filter {
    s16 raw[8];
    int sum;
    int number;
    int idx;
};
#endif

/* PERI-BJ-Implement_New_PSensor_Detect_NonDetect_Feature-00+{ */
#ifdef FIH_CAL
struct psensor_adc{
    int uncovered;
    int uncovered_write_done;
    int covered;
    int covered_write_done;
};
#endif
/* PERI-BJ-Implement_New_PSensor_Detect_NonDetect_Feature-00+} */

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
/* PERI-BJ-Add_Regular_Optimum_feature-00+{ */
    struct regulator *vdd;
    uint32_t lpm_uA;
    uint32_t hpm_uA;
/* PERI-BJ-Add_Regular_Optimum_feature-00+} */
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
/* PERI-BJ-Implement_New_PSensor_Detect_NonDetect_Feature-00+{ */
#ifdef FIH_CAL
    struct psensor_adc      ps_raw;
#endif
/* PERI-BJ-Implement_New_PSensor_Detect_NonDetect_Feature-00+} */

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
/* PERI-BJ-Add_Threshold_Change_Feature_While_Approaching_Zero-00*{ */
    int32_t high_thd,low_thd;
    int32_t raw_change;

    raw_change = stk_lux2alscode(ps_data, STK_ALS_CHANGE_THD);
    high_thd = alscode + raw_change;
    low_thd = alscode - raw_change;

    if (high_thd >= (1<<16))
        high_thd = (1<<16) -1;

    //While alscode -raw_value < 0, we need to do some check to make sure lux will decrease to 0 smoothly.
    if (alscode == 0){
        low_thd = 0;
        high_thd = 5;
    }
    else if (alscode < 5){
        low_thd = 1;
        high_thd = alscode + 5;
    }
    else if (alscode < 10){
        low_thd = 5;
        high_thd = alscode + 10;
    }
    else if (alscode <= raw_change){
        low_thd = 10;
        high_thd = alscode + 10;
    }

#ifdef STK_DEBUG_PRINTF
    printk(KERN_DEBUG "%s: alscode=%d, raw_change=%d, modified high_thd=%d, low_thd=%d\n", __func__, alscode, raw_change, high_thd, low_thd);
#endif
/* PERI-BJ-Add_Threshold_Change_Feature_While_Approaching_Zero-00*} */

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

/* PERI-BJ-Implement_New_PSensor_Detect_NonDetect_Feature-00+{ */
#ifdef FIH_CAL
static void stk3x1x_set_ps_thd(struct stk3x1x_data *ps_data)
{
    if(ps_data->ps_raw.covered_write_done == 1 && ps_data->ps_raw.uncovered_write_done == 1){

        //Cover should bigger than uncover + delta in FTM.
        //Here, just use simple rule to make sure the value is reasonable.
        //if(ps_data->ps_raw.covered - ps_data->ps_raw.uncovered < 150)
        if(ps_data->ps_raw.covered <= ps_data->ps_raw.uncovered){
            printk(KERN_INFO "%s: Covered value should biggher than uncovered value\n", __func__);
            return;
        }

/* PERI-BJ-Implement_New_PSensor_Detect_NonDetect_Feature-01*{ */
        ps_data->ps_thd_h = ps_data->ps_raw.covered - ((ps_data->ps_raw.covered - ps_data->ps_raw.uncovered) * ps_thd_h_computed_rate / 100);
        ps_data->ps_thd_l = ps_data->ps_raw.covered - ((ps_data->ps_raw.covered - ps_data->ps_raw.uncovered) * ps_thd_l_computed_rate / 100);
/* PERI-BJ-Implement_New_PSensor_Detect_NonDetect_Feature-01*} */

        //set thd
        printk(KERN_INFO "%s: set HT=%d,LT=%d\n", __func__, ps_data->ps_thd_h,  ps_data->ps_thd_l);
        stk3x1x_set_ps_thd_h(ps_data, ps_data->ps_thd_h);
        stk3x1x_set_ps_thd_l(ps_data, ps_data->ps_thd_l);
    }
    return;
}
#endif
/* PERI-BJ-Implement_New_PSensor_Detect_NonDetect_Feature-00+} */

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

/* PERI-BJ-Add_Regular_Optimum_feature-00+{ */
static int stk3x1x_power_enable(struct stk3x1x_data *ps_data, uint8_t enable)
{
    int rc = 0;
    //Note:The als_enabled and ps_enabled State are before setting state enable/disable done.

    //printk(KERN_DEBUG"%s: ps_data->als_enabled=%d, ps_data->ps_enabled=%d", __func__,ps_data->als_enabled, ps_data->ps_enabled);

    if (enable) {
        if(!(ps_data->als_enabled) && !(ps_data->ps_enabled)){
            rc = regulator_set_optimum_mode(ps_data->vdd, ps_data->hpm_uA);
            if (rc < 0) {
                printk(KERN_ERR"%s: set_optimum_mode vreg failed, enable=%d, rc=%d\n", __func__, enable, rc);
                return -EINVAL;
            }
        }
    } else{
        if(!((ps_data->als_enabled) && (ps_data->ps_enabled))){
            rc = regulator_set_optimum_mode(ps_data->vdd, ps_data->lpm_uA);
            if (rc < 0) {
                printk(KERN_ERR"%s: set_optimum_mode vreg failed, enable=%d, rc=%d\n", __func__, enable, rc);
                return -EINVAL;
            }
        }
    }

    printk(KERN_DEBUG"%s: enable=%d, rc=%d\n",__func__, enable, rc);

    return 0;
}

static int stk3x1x_power_init(struct stk3x1x_data *ps_data)
{
    int ret = 0;

    ps_data->vdd = regulator_get(&ps_data->client->dev, "stkvdd");
    if (IS_ERR(ps_data->vdd)) {
        ret = PTR_ERR(ps_data->vdd);
        printk(KERN_ERR"%s: regulator_get failed, ret=%d\n",__func__, ret);
        return ret;
    }

    printk(KERN_DEBUG"%s: regulator_get OK, ret=%d\n",__func__, ret);

    return ret;
}
/* PERI-BJ-Add_Regular_Optimum_feature-00+} */

static int32_t stk3x1x_enable_ps(struct stk3x1x_data *ps_data, uint8_t enable)
{
    int32_t ret;
    uint8_t w_state_reg;
    uint8_t curr_ps_enable;
    uint32_t reading;
    int32_t near_far_state;

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

/* PERI-BJ-Add_Regular_Optimum_feature-00+ {*/
    if(enable)
        stk3x1x_power_enable(ps_data, true);
/* PERI-BJ-Add_Regular_Optimum_feature-00+ }*/

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

    if(enable)
    {
#ifdef STK_TUNE0
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
        msleep(4);
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
#ifdef STK_POLL_PS
        hrtimer_cancel(&ps_data->ps_timer);
        cancel_work_sync(&ps_data->stk_ps_work);
#else
    #ifndef STK_POLL_ALS
        if(!(ps_data->als_enabled))
    #endif
        disable_irq(ps_data->irq);
#endif
        stk3x1x_power_enable(ps_data, false);/* PERI-BJ-Add_Regular_Optimum_feature-00+ */
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

/* PERI-BJ-Add_Regular_Optimum_feature-00+ {*/
    if(enable)
        stk3x1x_power_enable(ps_data, true);
/* PERI-BJ-Add_Regular_Optimum_feature-00+ }*/

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
        stk3x1x_power_enable(ps_data, false);/* PERI-BJ-Add_Regular_Optimum_feature-00+ */
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

/* PERI-BJ-Add_Transmittance_Calibration_Store_Feature-00+{ */
    ret = dev_set_drvdata(dev, ps_data);
    if(ret)
    {
        printk(KERN_ERR "%s:write back transmittance data fail, ret=0x%x\n", __func__, ret);
        return ret;
    }
/* PERI-BJ-Add_Transmittance_Calibration_Store_Feature-00+} */
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

/* PERI-BJ-Implement_New_PSensor_Detect_NonDetect_Feature-00+{ */
#ifdef FIH_CAL
static ssize_t stk_ps_cover_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct stk3x1x_data *ps_data = dev_get_drvdata(dev);
    return scnprintf(buf, PAGE_SIZE, "%d\n", ps_data->ps_raw.covered);
}

static ssize_t stk_ps_cover_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
    struct stk3x1x_data *ps_data = dev_get_drvdata(dev);
    unsigned long value = 0;
    int ret;

    ret = strict_strtoul(buf, 10, &value);
    if(ret < 0)
    {
        printk(KERN_ERR "%s:strict_strtoul failed, ret=0x%x\n", __func__, ret);
        return ret;
    }
    printk(KERN_INFO "%s:value=%ld\n", __func__, value);
    ps_data->ps_raw.covered = value;
    ps_data->ps_raw.covered_write_done = 1;

    stk3x1x_set_ps_thd(ps_data);
    return size;
}

static ssize_t stk_ps_uncover_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct stk3x1x_data *ps_data = dev_get_drvdata(dev);
    return scnprintf(buf, PAGE_SIZE, "%d\n", ps_data->ps_raw.uncovered);
}

static ssize_t stk_ps_uncover_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
    struct stk3x1x_data *ps_data = dev_get_drvdata(dev);
    unsigned long value = 0;
    int ret;

    ret = strict_strtoul(buf, 10, &value);
    if(ret < 0)
    {
        printk(KERN_ERR "%s:strict_strtoul failed, ret=0x%x\n", __func__, ret);
        return ret;
    }
    printk(KERN_INFO "%s:value=%ld\n", __func__, value);
    ps_data->ps_raw.uncovered = value;
    ps_data->ps_raw.uncovered_write_done = 1;

    stk3x1x_set_ps_thd(ps_data);
    return size;
}

#endif
/* PERI-BJ-Implement_New_PSensor_Detect_NonDetect_Feature-00+} */

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

#endif	/* #ifdef STK_TUNE0 */

static struct device_attribute als_enable_attribute = __ATTR(enable,0664,stk_als_enable_show,stk_als_enable_store);
static struct device_attribute als_lux_attribute = __ATTR(value,0664,stk_als_lux_show,stk_als_lux_store);
static struct device_attribute als_code_attribute = __ATTR(code, 0444, stk_als_code_show, NULL);
static struct device_attribute als_transmittance_attribute = __ATTR(transmittance,0664,stk_als_transmittance_show,stk_als_transmittance_store);
#if 0
static struct device_attribute als_lux_thd_l_attribute = __ATTR(luxthdl,0664,stk_als_lux_thd_l_show,stk_als_lux_thd_l_store);
static struct device_attribute als_lux_thd_h_attribute = __ATTR(luxthdh,0664,stk_als_lux_thd_h_show,stk_als_lux_thd_h_store);
#endif
static struct device_attribute als_poll_delay_attribute = __ATTR(delay,0664,stk_als_delay_show,stk_als_delay_store);
static struct device_attribute als_ir_code_attribute = __ATTR(ircode,0444,stk_als_ir_code_show,NULL);


static struct attribute *stk_als_attrs [] =
{
    &als_enable_attribute.attr,
    &als_lux_attribute.attr,
    &als_code_attribute.attr,
    &als_transmittance_attribute.attr,
#if 0
    &als_lux_thd_l_attribute.attr,
    &als_lux_thd_h_attribute.attr,
#endif
    &als_poll_delay_attribute.attr,
    &als_ir_code_attribute.attr,
    NULL
};

static struct attribute_group stk_als_attribute_group = {
//	.name = "driver",
	.attrs = stk_als_attrs,
};


static struct device_attribute ps_enable_attribute = __ATTR(enable,0664,stk_ps_enable_show,stk_ps_enable_store);
static struct device_attribute ps_enable_aso_attribute = __ATTR(enableaso,0664,stk_ps_enable_aso_show,stk_ps_enable_aso_store);
static struct device_attribute ps_distance_attribute = __ATTR(value,0664,stk_ps_distance_show, stk_ps_distance_store);
static struct device_attribute ps_distance2_attribute = __ATTR(distance,0664,stk_ps_distance_show, stk_ps_distance_store);
static struct device_attribute ps_offset_attribute = __ATTR(offset,0664,stk_ps_offset_show, stk_ps_offset_store);
static struct device_attribute ps_code_attribute = __ATTR(code, 0444, stk_ps_code_show, NULL);
static struct device_attribute ps_code_thd_l_attribute = __ATTR(codethdl,0664,stk_ps_code_thd_l_show,stk_ps_code_thd_l_store);
static struct device_attribute ps_code_thd_h_attribute = __ATTR(codethdh,0664,stk_ps_code_thd_h_show,stk_ps_code_thd_h_store);
static struct device_attribute recv_attribute = __ATTR(recv,0664,stk_recv_show,stk_recv_store);
static struct device_attribute send_attribute = __ATTR(send,0664,stk_send_show, stk_send_store);
static struct device_attribute all_reg_attribute = __ATTR(allreg, 0444, stk_all_reg_show, NULL);
static struct device_attribute status_attribute = __ATTR(status, 0444, stk_status_show, NULL);
/* PERI-BJ-Implement_New_PSensor_Detect_NonDetect_Feature-00+{ */
#ifdef FIH_CAL
static struct device_attribute ps_cover_attribute = __ATTR(cover,0664,stk_ps_cover_show,stk_ps_cover_store);
static struct device_attribute ps_uncover_attribute = __ATTR(uncover,0664,stk_ps_uncover_show,stk_ps_uncover_store);
#endif
/* PERI-BJ-Implement_New_PSensor_Detect_NonDetect_Feature-00+} */
#ifdef STK_TUNE0
static struct device_attribute ps_cali_attribute = __ATTR(cali,0444,stk_ps_cali_show, NULL);
#endif

static struct attribute *stk_ps_attrs [] =
{
    &ps_enable_attribute.attr,
    &ps_enable_aso_attribute.attr,
    &ps_distance_attribute.attr,
	&ps_distance2_attribute.attr,
    &ps_offset_attribute.attr,
    &ps_code_attribute.attr,
    &ps_code_thd_l_attribute.attr,
    &ps_code_thd_h_attribute.attr,
    &recv_attribute.attr,
    &send_attribute.attr,
    &all_reg_attribute.attr,
    &status_attribute.attr,
/* PERI-BJ-Implement_New_PSensor_Detect_NonDetect_Feature-00+{ */
#ifdef FIH_CAL
    &ps_cover_attribute.attr,
    &ps_uncover_attribute.attr,
#endif
/* PERI-BJ-Implement_New_PSensor_Detect_NonDetect_Feature-00+} */
#ifdef STK_TUNE0
    &ps_cali_attribute.attr,
#endif
    NULL
};

static struct attribute_group stk_ps_attribute_group = {
	//.name = "driver",
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

    ps_data->psa = ps_data->ps_stat_data[0];
    ps_data->psi = ps_data->ps_stat_data[2];
    ps_data->ps_thd_h = ps_data->ps_stat_data[1] + STK_HT_N_CT;
    ps_data->ps_thd_l = ps_data->ps_stat_data[1] + STK_LT_N_CT;
    stk3x1x_set_ps_thd_h(ps_data, ps_data->ps_thd_h);
    stk3x1x_set_ps_thd_l(ps_data, ps_data->ps_thd_l);
    printk(KERN_INFO "%s: set HT=%d,LT=%d\n", __func__, ps_data->ps_thd_h,  ps_data->ps_thd_l);
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

    if(ps_data->psi_set || !(ps_data->ps_enabled))
    {
        //printk(KERN_INFO "%s: FAE tune0 close\n", __func__);
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

        if(word_data == 0)
        {
            //printk(KERN_ERR "%s: incorrect word data (0)\n", __func__);
            return 0xFFFF;
        }

        if(word_data > ps_data->psa)
        {
            ps_data->psa = word_data;
            printk(KERN_INFO "%s: update psa: psa=%d,psi=%d\n", __func__, ps_data->psa, ps_data->psi);
        }
        if(word_data < ps_data->psi)
        {
            ps_data->psi = word_data;
            printk(KERN_INFO "%s: update psi: psa=%d,psi=%d\n", __func__, ps_data->psa, ps_data->psi);
        }
    }
    diff = ps_data->psa - ps_data->psi;
    if(diff > STK_MAX_MIN_DIFF)
    {
        ps_data->psi_set = ps_data->psi;
        ps_data->ps_thd_h = ps_data->psi + STK_HT_N_CT;
        ps_data->ps_thd_l = ps_data->psi + STK_LT_N_CT;
        stk3x1x_set_ps_thd_h(ps_data, ps_data->ps_thd_h);
        stk3x1x_set_ps_thd_l(ps_data, ps_data->ps_thd_l);
#ifdef STK_DEBUG_PRINTF
        printk(KERN_INFO "%s: FAE tune0 psa-psi(%d) > STK_DIFF found\n", __func__, diff);
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
/* PERI-BJ-Implement_New_PSensor_Detect_NonDetect_Feature-00+{ */
#ifdef FIH_CAL
    ps_data->ps_raw.covered = 951;
    ps_data->ps_raw.covered_write_done = 0;
    ps_data->ps_raw.uncovered = 801;
    ps_data->ps_raw.uncovered_write_done = 0;
#endif
/* PERI-BJ-Implement_New_PSensor_Detect_NonDetect_Feature-00+} */
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
    err = gpio_request(ps_data->int_pin,"stk-int");
    if(err < 0)
    {
        printk(KERN_ERR "%s: gpio_request, err=%d", __func__, err);
        return err;
    }
    err = gpio_direction_input(ps_data->int_pin);
    if(err < 0)
    {
        printk(KERN_ERR "%s: gpio_direction_input, err=%d", __func__, err);
        return err;
    }
#if ((STK_INT_PS_MODE == 0x03) || (STK_INT_PS_MODE	== 0x02))
    err = request_any_context_irq(irq, stk_oss_irq_handler, IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING, DEVICE_NAME, ps_data);
#else
    err = request_any_context_irq(irq, stk_oss_irq_handler, IRQF_TRIGGER_LOW, DEVICE_NAME, ps_data);
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


static int stk3x1x_suspend(struct device *dev)
{
    struct stk3x1x_data *ps_data = dev_get_drvdata(dev);
#ifndef STK_POLL_PS
    struct i2c_client *client = to_i2c_client(dev);
    int err;
#endif

    printk(KERN_INFO "%s", __func__);
#ifndef SPREADTRUM_PLATFORM
    mutex_lock(&ps_data->io_lock);
    if(ps_data->als_enabled)
    {
        stk3x1x_enable_als(ps_data, 0);
        ps_data->re_enable_als = true;
    }
#endif
    printk(KERN_INFO "%s:proximity sensor enable(%d)", __func__,ps_data->ps_enabled);
    if(ps_data->ps_enabled)
    {
#ifdef STK_POLL_PS
        wake_lock(&ps_data->ps_nosuspend_wl);
#else
        if(device_may_wakeup(&client->dev))
        {
            err = enable_irq_wake(ps_data->irq);
            if (err)
            printk(KERN_INFO "%s: set_irq_wake(%d) failed, err=(%d)\n", __func__, ps_data->irq, err);
        }
        else
        {
           printk(KERN_ERR "%s: not support wakeup source", __func__);
        }
#endif
    }
#ifndef SPREADTRUM_PLATFORM
    mutex_unlock(&ps_data->io_lock);
#endif
    return 0;
}

static int stk3x1x_resume(struct device *dev)
{
    struct stk3x1x_data *ps_data = dev_get_drvdata(dev);
#ifndef STK_POLL_PS
    struct i2c_client *client = to_i2c_client(dev);
    int err;
#endif

    printk(KERN_INFO "%s", __func__);
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
        if(device_may_wakeup(&client->dev))
        {
            err = disable_irq_wake(ps_data->irq);
            if (err)
                printk(KERN_WARNING "%s: disable_irq_wake(%d) failed, err=(%d)\n", __func__, ps_data->irq, err);
        }
#endif
    }
#ifndef SPREADTRUM_PLATFORM
    mutex_unlock(&ps_data->io_lock);
#endif
    return 0;
}

static const struct dev_pm_ops stk3x1x_pm_ops = {
    SET_SYSTEM_SLEEP_PM_OPS(stk3x1x_suspend, stk3x1x_resume)
};

#ifdef CONFIG_HAS_EARLYSUSPEND
static void stk3x1x_early_suspend(struct early_suspend *h)
{
    struct stk3x1x_data *ps_data = container_of(h, struct stk3x1x_data, stk_early_suspend);
#ifndef STK_POLL_PS
    int err;
#endif

    printk(KERN_INFO "%s", __func__);
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
    return;
}

static void stk3x1x_late_resume(struct early_suspend *h)
{
    struct stk3x1x_data *ps_data = container_of(h, struct stk3x1x_data, stk_early_suspend);
#ifndef STK_POLL_PS
    int err;
#endif

    printk(KERN_INFO "%s", __func__);
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
    return;
}
#endif	//#ifdef CONFIG_HAS_EARLYSUSPEND

static int stk3x1x_parse_dt(struct device *dev,
				struct stk3x1x_platform_data *pdata)
{
    struct device_node *np = dev->of_node;
    unsigned int temp_val=0;
    const __be32 *prop; /* PERI-BJ-Add_Regular_Optimum_feature-00+ */

    of_property_read_u32(np, "sensortek,slave-addr",&temp_val);
    //pdata->slave_addr = temp_val;
    pdata->int_pin = of_get_named_gpio_flags(np,"sensortek,int_pin", 0, &temp_val);
    of_property_read_u32(np, "sensortek,state_reg",&temp_val);
    pdata->state_reg = temp_val;
    of_property_read_u32(np, "sensortek,psctrl_reg",&temp_val);
    pdata->psctrl_reg = temp_val;
    of_property_read_u32(np, "sensortek,alsctrl_reg",&temp_val);
    pdata->alsctrl_reg = temp_val;
    of_property_read_u32(np, "sensortek,ledctrl_reg",&temp_val);
    pdata->ledctrl_reg = temp_val;
    of_property_read_u32(np, "sensortek,wait_reg",&temp_val);
    pdata->wait_reg = temp_val;

    of_property_read_u32(np, "sensortek,ps_thd_h",&temp_val);
    pdata->ps_thd_h = temp_val;
    of_property_read_u32(np, "sensortek,ps_thd_l",&temp_val);
    pdata->ps_thd_l = temp_val;

    of_property_read_u32(np, "sensortek,transmittance",&temp_val);
    pdata->transmittance = temp_val;

/* PERI-BJ-Add_Regular_Optimum_feature-00+{ */
    prop = of_get_property(np, "sensortek,vdd-current-level",&temp_val);
    if (!prop || (temp_val != (2 * sizeof(__be32))))
    {
        printk(KERN_ERR "%s, %s sensortek,vdd-current-level property.\n", __func__, prop ? "invalid format" : "no");
        return -ENODEV;
    }
    else
    {
        pdata->lpm_uA = be32_to_cpup(&prop[0]);
        pdata->hpm_uA = be32_to_cpup(&prop[1]);
        printk(KERN_DEBUG"%s: pdata->lpm_uA=%d, pdata->hpm_uA=%d\n",__func__, pdata->lpm_uA, pdata->hpm_uA);
    }
/* PERI-BJ-Add_Regular_Optimum_feature-00+} */
    printk(KERN_ERR "%s: GPIO = %d\n", __func__,pdata->int_pin);
    return 0;
}

static int stk3x1x_probe(struct i2c_client *client,
                        const struct i2c_device_id *id)
{
    int err = -ENODEV;
    struct stk3x1x_data *ps_data;
    struct stk3x1x_platform_data *plat_data;
    printk(KERN_INFO "%s: driver version = %s\n", __func__, DRIVER_VERSION);

    if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
    {
        printk(KERN_ERR "%s: No Support for I2C_FUNC_I2C\n", __func__);
        return -ENODEV;
    }

    if (client->dev.of_node)
    {
        plat_data = devm_kzalloc(&client->dev,
        sizeof(*plat_data),
        GFP_KERNEL);
        if (!plat_data)
        {
            pr_err("Failed to allocate memory\n");
            return -ENOMEM;
        }
        stk3x1x_parse_dt(&client->dev, plat_data);
    }
    else
    {
        plat_data = client->dev.platform_data;
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
    if(plat_data!=NULL)
    {

/* PERI-BJ-Add_Regular_Optimum_feature-00+{ */
        ps_data->lpm_uA = plat_data->lpm_uA;
        ps_data->hpm_uA = plat_data->hpm_uA;
/* PERI-BJ-Add_Regular_Optimum_feature-00+} */
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

/* PERI-BJ-Add_Regular_Optimum_feature-00+{ */
    err = stk3x1x_power_init(ps_data);
    if (err)
        goto err_power_init;

    err = stk3x1x_power_enable(ps_data, 1);
    if (err)
        goto err_power_init;
/* PERI-BJ-Add_Regular_Optimum_feature-00+} */
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
/* PERI-BJ-Add_Regular_Optimum_feature-00+{ */
    //enable device power only when it is enabled
    stk3x1x_power_enable(ps_data, 0);
/* PERI-BJ-Add_Regular_Optimum_feature-00+} */
    printk(KERN_INFO "%s: probe successfully", __func__);
    return 0;

err_init_all_setting:
    device_init_wakeup(&client->dev, false);
/* PERI-BJ-Add_Regular_Optimum_feature-00+{ */
    stk3x1x_power_enable(ps_data, 0);
err_power_init:
/* PERI-BJ-Add_Regular_Optimum_feature-00+} */
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

#ifdef CONFIG_OF
static struct of_device_id stk3x1x_match_table[] = {
	{ .compatible = "sensortek,stk3x1x",},
	{ },
};
#else
#define rmi4_match_table NULL
#endif

static struct i2c_driver stk_ps_driver =
{
    .driver = {
        .name = DEVICE_NAME,
        .owner = THIS_MODULE,
        .pm = &stk3x1x_pm_ops,
        .of_match_table = stk3x1x_match_table,
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
