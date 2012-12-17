/*
 * This file is part of the TMD2771 sensor driver.
 * Chip is taos proximity and ambient light sensor.
 *
 * Copyright (c) 2011 Liteon-semi Corporation
 *
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 * Filename: tmd2771.c
 *
 * Summary:
 *	TMD2771 sensor dirver for kernel version 3.0.8.
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/input.h>
#include <linux/string.h>
#include <linux/workqueue.h>
#include <linux/hrtimer.h>
#include <linux/gpio.h>
#include <linux/wakelock.h>

#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif

#include "tmd2771.h"

#define TAOS_TRITON_CHIPIDVAL           0x00
#define TAOS_TRITON_MAXREGS             32

#define TAOS_MAX_NUM_DEVICES            3
#define TAOS_MAX_DEVICE_REGS            32
#define I2C_MAX_ADAPTERS                1

/* TRITON register offsets */
#define TAOS_TRITON_CNTRL               0x00
#define TAOS_TRITON_ALS_TIME            0X01
#define TAOS_TRITON_PRX_TIME            0x02
#define TAOS_TRITON_WAIT_TIME           0x03
#define TAOS_TRITON_ALS_MINTHRESHLO     0X04
#define TAOS_TRITON_ALS_MINTHRESHHI     0X05
#define TAOS_TRITON_ALS_MAXTHRESHLO     0X06
#define TAOS_TRITON_ALS_MAXTHRESHHI     0X07
#define TAOS_TRITON_PRX_MINTHRESHLO     0X08
#define TAOS_TRITON_PRX_MINTHRESHHI     0X09
#define TAOS_TRITON_PRX_MAXTHRESHLO     0X0A
#define TAOS_TRITON_PRX_MAXTHRESHHI     0X0B
#define TAOS_TRITON_INTERRUPT           0x0C
#define TAOS_TRITON_PRX_CFG             0x0D
#define TAOS_TRITON_PRX_COUNT           0x0E
#define TAOS_TRITON_GAIN                0x0F
#define TAOS_TRITON_REVID               0x11
#define TAOS_TRITON_CHIPID              0x12
#define TAOS_TRITON_STATUS              0x13
#define TAOS_TRITON_ALS_CHAN0LO         0x14
#define TAOS_TRITON_ALS_CHAN0HI         0x15
#define TAOS_TRITON_ALS_CHAN1LO         0x16
#define TAOS_TRITON_ALS_CHAN1HI         0x17
#define TAOS_TRITON_PRX_LO              0x18
#define TAOS_TRITON_PRX_HI              0x19
#define TAOS_TRITON_TEST_STATUS         0x1F

/* Triton cmd reg masks */
#define TAOS_TRITON_CMD_REG             0X80
#define TAOS_TRITON_CMD_AUTO            0x10
#define TAOS_TRITON_CMD_BYTE_RW         0x00
#define TAOS_TRITON_CMD_WORD_BLK_RW     0x20
#define TAOS_TRITON_CMD_SPL_FN          0x60
#define TAOS_TRITON_CMD_PROX_INTCLR     0X05
#define TAOS_TRITON_CMD_ALS_INTCLR      0X06
#define TAOS_TRITON_CMD_PROXALS_INTCLR  0X07
#define TAOS_TRITON_CMD_TST_REG         0X08
#define TAOS_TRITON_CMD_USER_REG        0X09

/* Triton cntrl reg masks */
#define TAOS_TRITON_CNTL_PROX_INT_ENBL  0X20
#define TAOS_TRITON_CNTL_ALS_INT_ENBL   0X10
#define TAOS_TRITON_CNTL_WAIT_TMR_ENBL  0X08
#define TAOS_TRITON_CNTL_PROX_DET_ENBL  0X04
#define TAOS_TRITON_CNTL_ADC_ENBL       0x02
#define TAOS_TRITON_CNTL_PWRON          0x01

/* Triton status reg masks */
#define TAOS_TRITON_STATUS_ADCVALID     0x01
#define TAOS_TRITON_STATUS_PRXVALID     0x02
#define TAOS_TRITON_STATUS_ADCINTR      0x10
#define TAOS_TRITON_STATUS_PRXINTR      0x20

/* lux constants */
#define TAOS_MAX_LUX                    1000000
#define TAOS_SCALE_MILLILUX             2
#define TAOS_FILTER_DEPTH               3
#define CHIP_ID                         0x3d

#define TAOS_INPUT_NAME_ALS  "TAOS_ALS_SENSOR"
#define TAOS_INPUT_NAME_PROX  "TAOS_PROX_SENSOR"

static int isPsensorLocked;

struct tmd2771_data {
	struct i2c_client *client;
	struct class *taos_class;
	struct device *ls_dev;
	struct device *ps_dev;
	struct input_dev *input_dev_als;
	struct input_dev *input_dev_prox;
	struct work_struct work;
	struct wake_lock taos_wake_lock;
	char taos_id;
	char taos_name[TAOS_ID_NAME_SIZE];
	struct mutex update_lock;
	char valid;
	unsigned long last_updated;
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend taos_early_suspend;
#endif

};
struct tmd2771_data *p_tmd2771_data;

/* device configuration */
struct taos_cfg *taos_cfgp;
static u32 calibrate_target_param = 300000;
static u16 als_time_param = 200;
static u16 scale_factor_param = 4;
static u16 gain_trim_param = 512;
static u8 filter_history_param = 3;
static u8 filter_count_param = 1;
static u8 gain_param;
static u16 prox_threshold_hi_param = 646;
static u16 prox_threshold_lo_param = 638;
static u16 als_threshold_hi_param = 3000;
static u16 als_threshold_lo_param = 10;
static u8 prox_int_time_param = 0xEE;
static u8 prox_adc_time_param = 0xFF;
static u8 prox_wait_time_param = 0xEE;
static u8 prox_intr_filter_param = 0x20;
static u8 prox_config_param = 0x00;
static u8 prox_pulse_cnt_param = 0xa0;	/* 0x02; */
static u8 prox_gain_param = 0x62;

static int prox_on;
static char pro_buf[4];
static int mcount;
static char als_buf[4];
static u16 sat_als;
static u16 sat_prox;
static int ALS_ON;

struct time_scale_factor {
	u16 numerator;
	u16 denominator;
	u16 saturation;
};
struct time_scale_factor TritonTime = { 1, 0, 0 };
struct time_scale_factor *lux_timep = &TritonTime;

/* gain table */
static u8 taos_triton_gain_table[] = { 1, 8, 16, 120 };

/* lux data */
struct lux_data {
	u16 ratio;
	u16 clear;
	u16 ir;
};
struct lux_data TritonFN_lux_data[] = {
	{9830, 8320, 15360},
	{12452, 10554, 22797},
	{14746, 6234, 11430},
	{17695, 3968, 6400},
	{0, 0, 0}
};
struct lux_data *lux_tablep = TritonFN_lux_data;
static int lux_history[TAOS_FILTER_DEPTH] = { -ENODATA, -ENODATA, -ENODATA };

static int tmd2771_i2c_read(struct i2c_client *client, u8 reg, u8 *val)
{
	int err = -EIO;
	struct tmd2771_data *data = i2c_get_clientdata(client);

	mutex_lock(&data->update_lock);

	/* select register to write */
	err = i2c_smbus_write_byte(client, (TAOS_TRITON_CMD_REG | reg));
	if (err < 0) {
		printk(KERN_ERR "failed to write cmd register 0x%x, ret: %d\n",
			reg, err);

		goto smbus_write_err;
	}

	/* read the data */
	err = i2c_smbus_read_byte(client);
	if (err >= 0) {
		*val = (u8) err;
	} else {
		printk(KERN_ERR "failed to read register: 0x%x,ret:%d\n",
		       reg, err);

		goto smbus_write_err;
	}

smbus_write_err:
	mutex_unlock(&data->update_lock);

	return err;
}

static int tmd2771_prox_threshold_set(void)
{
	int i, ret = 0;
	u8 chdata[6];
	u16 proxdata = 0;
	u16 cleardata = 0;
	int data = 0;
	struct tmd2771_data *drv_data = p_tmd2771_data;
	for (i = 0; i < 6; i++) {
		chdata[i] = (i2c_smbus_read_byte_data(drv_data->client,
						      (TAOS_TRITON_CMD_REG |
						       TAOS_TRITON_CMD_AUTO |
						       (TAOS_TRITON_ALS_CHAN0LO
							+ i))));
	}

	cleardata = chdata[0] + chdata[1] * 256;
	proxdata = chdata[4] + chdata[5] * 256;
	if (prox_on && proxdata < taos_cfgp->prox_threshold_lo) {
		pro_buf[0] = 0x0;
		pro_buf[1] = 0x0;
		pro_buf[2] = taos_cfgp->prox_threshold_hi & 0x0ff;
		pro_buf[3] = taos_cfgp->prox_threshold_hi >> 8;
		data = 1;
		input_report_abs(drv_data->input_dev_prox, ABS_DISTANCE, data);
		input_sync(drv_data->input_dev_prox);

	} else if (proxdata > taos_cfgp->prox_threshold_hi) {
		if (cleardata > ((sat_als * 80) / 100))
			return -ENODATA;
		pro_buf[0] = taos_cfgp->prox_threshold_lo & 0x0ff;
		pro_buf[1] = taos_cfgp->prox_threshold_lo >> 8;
		pro_buf[2] = 0xff;
		pro_buf[3] = 0xff;
		data = 0;
		input_report_abs(drv_data->input_dev_prox, ABS_DISTANCE, data);
		input_sync(drv_data->input_dev_prox);
	}

	printk(KERN_DEBUG "prox data: %d\n", data);
	for (mcount = 0; mcount < 4; mcount++) {
		ret = i2c_smbus_write_byte_data(drv_data->client,
						(TAOS_TRITON_CMD_REG | 0x08) +
						mcount, pro_buf[mcount]);
		if (ret < 0)
			return ret;
	}

	/*prox_on = 0; */
	return ret;
}

static int tmd2771_als_threshold_set(void)
{
	int i, ret = 0;
	u8 chdata[2];
	u16 ch0;
	struct i2c_client *client = p_tmd2771_data->client;
	for (i = 0; i < 2; i++) {
		chdata[i] = (i2c_smbus_read_byte_data(client,
						      (TAOS_TRITON_CMD_REG |
						       TAOS_TRITON_CMD_AUTO |
						       (TAOS_TRITON_ALS_CHAN0LO
							+ i))));
	}

	ch0 = chdata[0] + chdata[1] * 256;
	als_threshold_hi_param = (12 * ch0) / 10;
	if (als_threshold_hi_param >= 65535)
		als_threshold_hi_param = 65535;
	als_threshold_lo_param = (8 * ch0) / 10;
	als_buf[0] = als_threshold_lo_param & 0x0ff;
	als_buf[1] = als_threshold_lo_param >> 8;
	als_buf[2] = als_threshold_hi_param & 0x0ff;
	als_buf[3] = als_threshold_hi_param >> 8;

	for (mcount = 0; mcount < 4; mcount++) {
		ret = i2c_smbus_write_byte_data(client,
						(TAOS_TRITON_CMD_REG | 0x04)
						+ mcount, als_buf[mcount]);
		if (ret < 0) {
			printk(KERN_ERR
			       "%s: i2c_smbus_write_byte_data failed\n",
			       __func__);

			return ret;
		}
	}

	return ret;
}

/* read/calculate lux value */
static int tmd2771_get_lux(void)
{
	u16 raw_clear = 0, raw_ir = 0, raw_lux = 0;
	u32 lux = 0;
	u32 ratio = 0;
	u8 dev_gain = 0;
	struct lux_data *p;
	int ret = 0;
	u8 chdata[4];
	int tmp = 0, i = 0;
	struct i2c_client *client = p_tmd2771_data->client;

	for (i = 0; i < 4; i++) {
		ret = i2c_smbus_write_byte(client,
					   TAOS_TRITON_CMD_REG |
					   (TAOS_TRITON_ALS_CHAN0LO + i));
		if (ret < 0) {
			printk(KERN_ERR
			       "TAOS: write reg failed in taos_get_lux()\n");
			return ret;
		}
		chdata[i] = i2c_smbus_read_byte(client);

		printk(KERN_DEBUG "ch(%d),data=%d\n", i, chdata[i]);
	}

	printk(KERN_DEBUG "ch0=%d\n", chdata[0] + chdata[1] * 256);
	printk(KERN_DEBUG "ch1=%d\n", chdata[2] + chdata[3] * 256);

	tmp = (taos_cfgp->als_time + 25) / 50;
	TritonTime.numerator = 1;
	TritonTime.denominator = tmp;

	tmp = 300 * taos_cfgp->als_time;
	if (tmp > 65535)
		tmp = 65535;
	TritonTime.saturation = tmp;
	raw_clear = chdata[1];
	raw_clear <<= 8;
	raw_clear |= chdata[0];
	raw_ir = chdata[3];
	raw_ir <<= 8;
	raw_ir |= chdata[2];

	if (raw_ir > raw_clear) {
		raw_lux = raw_ir;
		raw_ir = raw_clear;
		raw_clear = raw_lux;
	}
	raw_clear *= taos_cfgp->scale_factor;
	raw_ir *= taos_cfgp->scale_factor;
	dev_gain = taos_triton_gain_table[taos_cfgp->gain & 0x3];
	if (raw_clear >= lux_timep->saturation)
		return TAOS_MAX_LUX;
	if (raw_ir >= lux_timep->saturation)
		return TAOS_MAX_LUX;
	if (raw_clear == 0)
		return 0;
	if (dev_gain == 0 || dev_gain > 127) {
		printk(KERN_INFO
		       "TAOS: dev_gain = 0 or > 127 in taos_get_lux()\n");

		return -1;
	}
	if (lux_timep->denominator == 0) {
		printk(KERN_INFO
		       "TAOS: lux_timep->denominator = 0 in taos_get_lux()\n");

		return -1;
	}
	ratio = (raw_ir << 15) / raw_clear;

	printk(KERN_DEBUG "-----taos_get_lux--ratio==%d\n", ratio);
	for (p = lux_tablep; p->ratio && p->ratio < ratio; p++)
		;
	if (!p->ratio)
		return 0;
	lux = ((raw_clear * (p->clear)) - (raw_ir * (p->ir)));
	lux = ((lux + (lux_timep->denominator >> 1)) / lux_timep->denominator) *
	    lux_timep->numerator;
	lux = (lux + (dev_gain >> 1)) / dev_gain;
	lux >>= TAOS_SCALE_MILLILUX;

	printk(KERN_DEBUG "------taos_get_lux--lux==%d\n", lux);
	if (lux > TAOS_MAX_LUX)
		lux = TAOS_MAX_LUX;
	return (lux) * taos_cfgp->filter_count;
}

static int tmd2771_lux_filter(int lux)
{
	static u8 middle[] = { 1, 0, 2, 0, 0, 2, 0, 1 };
	int index;

	lux_history[2] = lux_history[1];
	lux_history[1] = lux_history[0];
	lux_history[0] = lux;

	if (lux_history[2] < 0) {
		if (lux_history[1] > 0)
			return lux_history[1];
		else
			return lux_history[0];
	}
	index = 0;
	if (lux_history[0] > lux_history[1])
		index += 4;
	if (lux_history[1] > lux_history[2])
		index += 2;
	if (lux_history[0] > lux_history[2])
		index++;
	return lux_history[middle[index]];
}

static int tmd2771_als_get_data(void)
{
	int ret = 0;
	u8 reg_val;
	int lux_val = 0;
	struct i2c_client *client = p_tmd2771_data->client;

	ret = i2c_smbus_write_byte(client, TAOS_TRITON_CMD_REG
				   | TAOS_TRITON_CNTRL);
	if (ret < 0) {
		printk(KERN_ERR
		       "TAOS: i2c_smbus_write_byte failed in ioctl als_data\n");

		return ret;
	}

	reg_val = i2c_smbus_read_byte(client);
	if ((reg_val & (TAOS_TRITON_CNTL_ADC_ENBL | TAOS_TRITON_CNTL_PWRON)) !=
	    (TAOS_TRITON_CNTL_ADC_ENBL | TAOS_TRITON_CNTL_PWRON)) {
		printk(KERN_ERR "TAOS: taos_als_get_data  reg_val === %0x\n",
		       reg_val);

		return -ENODATA;
	}

	ret = i2c_smbus_write_byte(client, TAOS_TRITON_CMD_REG
				   | TAOS_TRITON_STATUS);
	if (ret < 0) {
		printk(KERN_ERR
		       "TAOS: i2c_smbus_write_byte failed in ioctl als_data\n");
		return ret;
	}

	reg_val = i2c_smbus_read_byte(client);
	if ((reg_val & TAOS_TRITON_STATUS_ADCVALID) !=
	    TAOS_TRITON_STATUS_ADCVALID) {
		printk(KERN_ERR "TAOS: taos_als_get_data  reg_val === %0x\n",
		       reg_val);
		return -ENODATA;
	}

	lux_val = tmd2771_get_lux();
	if (lux_val < 0)
		printk(KERN_ERR
		       "TAOS: returned error %d in ioctl als_data\n",
		       lux_val);
	lux_val = tmd2771_lux_filter(lux_val);
	input_report_abs(p_tmd2771_data->input_dev_als, ABS_MISC, lux_val);
	input_sync(p_tmd2771_data->input_dev_als);

	printk(KERN_DEBUG "input report lux: %d\n", lux_val);

	return ret;
}

static int tmd2771_get_data(void)
{
	int ret = 0;
	int status;
	struct i2c_client *client = p_tmd2771_data->client;

	ret = i2c_smbus_write_byte(client, TAOS_TRITON_CMD_REG | 0x13);
	if (ret < 0) {
		printk(KERN_ERR
		       "i2c_smbus_write_byte(1)failed in taos_work_func()\n");
		return ret;
	}
	status = i2c_smbus_read_byte(client);
	if (mutex_trylock(&p_tmd2771_data->update_lock) == 0) {
		printk(KERN_ERR "taos_get_data device isbusy\n");

		return -ERESTARTSYS;
	}

	if ((status & 0x20) == 0x20) {
		ret = tmd2771_prox_threshold_set();
	} else if ((status & 0x10) == 0x10) {
		tmd2771_als_threshold_set();
		tmd2771_als_get_data();
	}
	mutex_unlock(&p_tmd2771_data->update_lock);

	return ret;
}

static int tmd2771_interrupts_clear(void)
{
	int ret = 0;
	ret = i2c_smbus_write_byte(p_tmd2771_data->client, (TAOS_TRITON_CMD_REG
					| TAOS_TRITON_CMD_SPL_FN | 0x07));
	if (ret < 0) {
		printk(KERN_ERR
		       "TAOS: i2c write failed in taos_work_func()\n");

		return ret;
	}
	return ret;
}

static void tmd2771_work_func(struct work_struct *work)
{
	tmd2771_get_data();
	tmd2771_interrupts_clear();
}

static irqreturn_t tmd2771_irq_handler(int irq, void *dev_id)
{
	struct tmd2771_data *data = (struct tmd2771_data *)dev_id;
	schedule_work(&data->work);
	return IRQ_HANDLED;
}

static int __tmd2771_sensors_als_on(struct tmd2771_data *data)
{
	int ret = 0;
	int i = 0;
	u8 itime = 0;
	u8 reg_val = 0;
	u8 reg_cntrl = 0;

	for (i = 0; i < TAOS_FILTER_DEPTH; i++)
		lux_history[i] = -ENODATA;
	ret = i2c_smbus_write_byte(data->client,
				   TAOS_TRITON_CMD_REG | TAOS_TRITON_CMD_SPL_FN
				   | TAOS_TRITON_CMD_ALS_INTCLR);
	if (ret < 0) {
		printk(KERN_ERR
		       "TAOS: i2c_smbus_write_byte failed in als_on\n");
		return ret;
	}
	itime = (((taos_cfgp->als_time / 50) * 18) - 1);
	itime = (~itime);
	ret = i2c_smbus_write_byte_data(data->client, TAOS_TRITON_CMD_REG
					| TAOS_TRITON_ALS_TIME, itime);
	if (ret < 0) {
		printk(KERN_ERR
		       "TAOS: i2c_smbus_write_byte_data failed in als_on\n");

		return ret;
	}
	ret = i2c_smbus_write_byte_data(data->client, TAOS_TRITON_CMD_REG
					| TAOS_TRITON_INTERRUPT,
					taos_cfgp->prox_intr_filter);
	if (ret < 0) {
		printk(KERN_ERR
		       "TAOS: i2c_smbus_write_byte_data failed in als_on\n");

		return ret;
	}
	ret = i2c_smbus_write_byte(data->client, TAOS_TRITON_CMD_REG
				   | TAOS_TRITON_GAIN);
	if (ret < 0) {
		printk(KERN_ERR
		       "TAOS: i2c_smbus_write_byte failed in als_on\n");

		return ret;
	}
	reg_val = i2c_smbus_read_byte(data->client);
	reg_val = reg_val & 0xFC;
	reg_val = reg_val | (taos_cfgp->gain & 0x03);
	ret = i2c_smbus_write_byte_data(data->client, TAOS_TRITON_CMD_REG
					| TAOS_TRITON_GAIN, reg_val);
	if (ret < 0) {
		printk(KERN_ERR
		       "TAOS: i2c_smbus_write_byte_data failed in als_on\n");

		return ret;
	}
	reg_cntrl = (TAOS_TRITON_CNTL_ADC_ENBL | TAOS_TRITON_CNTL_PWRON |
		     TAOS_TRITON_CNTL_ALS_INT_ENBL);
	ret = i2c_smbus_write_byte_data(data->client, TAOS_TRITON_CMD_REG
					| TAOS_TRITON_CNTRL, reg_cntrl);
	if (ret < 0) {
		printk(KERN_ERR
		       "TAOS: i2c_smbus_write_byte_data failed in ioctl als_on\n");

		return ret;
	}
	tmd2771_als_threshold_set();
	printk(KERN_DEBUG "taos_als_threshold_set succesful\n");

	return ret;
}

static int tmd2771_sensor_prox_on(struct tmd2771_data *data)
{
	int ret = 0;
	u8 reg_cntrl = 0;

	prox_on = 1;
	if (isPsensorLocked == 0) {
		wake_lock(&data->taos_wake_lock);
		isPsensorLocked = 1;
	}

	ret = i2c_smbus_write_byte_data(data->client,
					TAOS_TRITON_CMD_REG | 0x01,
					taos_cfgp->prox_int_time);
	if (ret < 0) {
		printk(KERN_ERR
		       "TAOS: i2c_smbus_write_byte_data failed in prox_on\n");

		return ret;
	}
	ret = i2c_smbus_write_byte_data(data->client,
					TAOS_TRITON_CMD_REG | 0x02,
					taos_cfgp->prox_adc_time);
	if (ret < 0) {
		printk(KERN_ERR
		       "TAOS: i2c_smbus_write_byte_data failed in prox_on\n");

		return ret;
	}
	ret = i2c_smbus_write_byte_data(data->client,
					TAOS_TRITON_CMD_REG | 0x03,
					taos_cfgp->prox_wait_time);
	if (ret < 0) {
		printk(KERN_ERR
		       "TAOS: i2c_smbus_write_byte_data failed in prox_on\n");

		return ret;
	}
	ret = i2c_smbus_write_byte_data(data->client,
					TAOS_TRITON_CMD_REG | 0x0C,
					taos_cfgp->prox_intr_filter);
	if (ret < 0) {
		printk(KERN_ERR
		       "TAOS: i2c_smbus_write_byte_data failed in prox_on\n");

		return ret;
	}
	ret = i2c_smbus_write_byte_data(data->client,
					TAOS_TRITON_CMD_REG | 0x0D,
					taos_cfgp->prox_config);
	if (ret < 0) {
		printk(KERN_ERR
		       "TAOS: i2c_smbus_write_byte_data failed in prox_on\n");

		return ret;
	}
	ret = i2c_smbus_write_byte_data(data->client,
					TAOS_TRITON_CMD_REG | 0x0E,
					taos_cfgp->prox_pulse_cnt);
	if (ret < 0) {
		printk(KERN_ERR
		       "TAOS: i2c_smbus_write_byte_data failed in prox_on\n");

		return ret;
	}
	ret = i2c_smbus_write_byte_data(data->client,
					TAOS_TRITON_CMD_REG | 0x0F,
					taos_cfgp->prox_gain);
	if (ret < 0) {
		printk(KERN_ERR
		       "TAOS: i2c_smbus_write_byte_data failed in prox_on\n");

		return ret;
	}
	reg_cntrl = TAOS_TRITON_CNTL_PROX_DET_ENBL | TAOS_TRITON_CNTL_PWRON |
	    TAOS_TRITON_CNTL_PROX_INT_ENBL | TAOS_TRITON_CNTL_ADC_ENBL |
	    TAOS_TRITON_CNTL_WAIT_TMR_ENBL;
	ret = i2c_smbus_write_byte_data(data->client,
					TAOS_TRITON_CMD_REG | TAOS_TRITON_CNTRL,
					reg_cntrl);
	if (ret < 0) {
		printk(KERN_ERR
		       "TAOS: i2c_smbus_write_byte_data failed in prox_on\n");

		return ret;
	}
	return tmd2771_prox_threshold_set();
}

static int tmd2771_sensor_prox_off(struct tmd2771_data *data)
{
	int ret = 0;

	ret = i2c_smbus_write_byte_data(data->client,
					TAOS_TRITON_CMD_REG | TAOS_TRITON_CNTRL,
					0x00);
	if (ret < 0) {
		printk(KERN_ERR
		       "TAOS: i2c_smbus_write_byte_data failed in als_off\n");

		return ret;
	}

	if (ALS_ON == 1)
		__tmd2771_sensors_als_on(data);
	else
		cancel_work_sync(&data->work);
	prox_on = 0;
	if (isPsensorLocked == 1) {
		wake_unlock(&data->taos_wake_lock);
		isPsensorLocked = 0;
	}
	return 0;
}

static int tmd2771_prox_set_power_state(struct tmd2771_data *data, int state)
{
	if (state)
		return tmd2771_sensor_prox_on(data);
	else
		return tmd2771_sensor_prox_off(data);
}

static int tmd2771_sensor_als_on(struct tmd2771_data *data)
{
	int ret = -EINVAL;
	u8 reg_val;

	ret = i2c_smbus_write_byte(data->client,
				   TAOS_TRITON_CMD_REG | TAOS_TRITON_CNTRL);
	if (ret < 0) {
		printk(KERN_ERR
		       "TAOS: i2c_smbus_write_byte failed in als_calibrate\n");

		return ret;
	}
	reg_val = i2c_smbus_read_byte(data->client);
	printk(KERN_DEBUG "TAOS IOCTL ALS ON   reg_val === %0x\n", reg_val);
	if ((reg_val & TAOS_TRITON_CNTL_PROX_DET_ENBL) == 0x0)
		__tmd2771_sensors_als_on(data);
	else {
		reg_val |= (TAOS_TRITON_CNTL_ADC_ENBL
			    | TAOS_TRITON_CNTL_ALS_INT_ENBL);
		ret = i2c_smbus_write_byte_data(data->client,
						TAOS_TRITON_CMD_REG |
						TAOS_TRITON_CNTRL, reg_val);
		if (ret < 0) {
			printk(KERN_ERR
			       "TAOS: i2c write failed in ioctl als_on\n");

			return ret;
		}
	}
	ALS_ON = 1;

	return ret;
}

static int tmd2771_sensor_als_off(struct tmd2771_data *data)
{
	int ret = -EINVAL;
	int i;
	u8 reg_val;

	for (i = 0; i < TAOS_FILTER_DEPTH; i++)
		lux_history[i] = -ENODATA;
	ret = i2c_smbus_write_byte(data->client,
				   TAOS_TRITON_CMD_REG | TAOS_TRITON_CNTRL);
	if (ret < 0) {
		printk(KERN_ERR
		       "TAOS: i2c write failed in ioctl als_calibrate\n");

		return ret;
	}
	reg_val = i2c_smbus_read_byte(data->client);
	if ((reg_val & TAOS_TRITON_CNTL_PROX_DET_ENBL) == 0x0) {
		ret = i2c_smbus_write_byte_data(data->client,
						TAOS_TRITON_CMD_REG |
						TAOS_TRITON_CNTRL, 0x00);
		if (ret < 0) {
			printk(KERN_ERR
			       "TAOS: i2c write failed in ioctl als_off\n");

			return ret;
		}
		cancel_work_sync(&data->work);
	}
	ALS_ON = 0;

	return ret;
}

static int tmd2771_als_set_power_state(struct tmd2771_data *data, int state)
{
	if (state)
		return tmd2771_sensor_als_on(data);
	else
		return tmd2771_sensor_als_off(data);
}

static ssize_t prox_data_show(struct device *dev,
			      struct device_attribute *attr, char *buf)
{
	return 0;
}

static DEVICE_ATTR(prox_data, 0666, prox_data_show, NULL);

static ssize_t prox_enable_store(struct device *dev,
				 struct device_attribute *attr,
				 const char *buf, size_t count)
{

	int enable;
	int ret;

	ret = kstrtoint(buf, 0, &enable);
	if (ret)
		return ret;

	if (!p_tmd2771_data)
		return -ENODEV;

	printk(KERN_DEBUG "enable: %d\n", (int)enable);

	tmd2771_prox_set_power_state(p_tmd2771_data, enable);

	return count;
}

static DEVICE_ATTR(prox_enable, 0666, NULL, prox_enable_store);

static ssize_t prox_set_delay_store(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf, size_t count)
{
	return count;
}

static DEVICE_ATTR(prox_set_delay, 0666, NULL, prox_set_delay_store);

static ssize_t als_data_show(struct device *dev,
			     struct device_attribute *attr, char *buf)
{
	return 0;
}

static DEVICE_ATTR(als_data, 0666, als_data_show, NULL);

static ssize_t als_enable_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	int enable;
	int ret;

	ret = kstrtoint(buf, 0, &enable);
	if (ret)
		return ret;

	if (!p_tmd2771_data)
		return -ENODEV;

	printk(KERN_DEBUG "enable: %d\n", (int)enable);

	tmd2771_als_set_power_state(p_tmd2771_data, enable);

	return count;
}

static DEVICE_ATTR(als_enable, 0666, NULL, als_enable_store);

static ssize_t als_set_delay_store(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t count)
{
	return count;
}

static DEVICE_ATTR(als_set_delay, 0666, NULL, als_set_delay_store);

static ssize_t reg_debug_show(struct device *dev,
			      struct device_attribute *attr, char *buf)
{
	u8 temp_buf[26] = { 0 };
	int ret = 0;
	int i = 0;
	u8 reg = TAOS_TRITON_CNTRL;

	printk(KERN_INFO "\n");
	for (i = 0; i < 26; i++) {
		ret = tmd2771_i2c_read(p_tmd2771_data->client,
				       reg + i, &temp_buf[i]);
		if (ret < 0) {
			printk(KERN_ERR "read reg 0x%x error, err: %d\n",
			       reg + i, ret);

			return ret;
		}
		printk(KERN_INFO "[0x%2x]0x%2x ", i, temp_buf[i]);
		if (i % 5 == 4)
			printk(KERN_INFO "\n");
	}
	printk(KERN_INFO "\n");

	return 0;
}

/*eg: echo reg reg_val > reg_debug */
static ssize_t reg_debug_store(struct device *dev,
			       struct device_attribute *attr,
			       const char *buf, size_t count)
{
	int ret = 0;
	int reg, reg_val;

	sscanf(buf, "%x%x", &reg, &reg_val);
	printk(KERN_INFO "reg: 0x%x, reg_val:0x%x\n", reg, reg_val);

	ret = i2c_smbus_write_byte_data(p_tmd2771_data->client,
					reg | TAOS_TRITON_CMD_REG, reg_val);
	if (ret < 0) {
		printk(KERN_ERR "write reg 0x%x error,err: %d\n", reg, ret);

		return ret;
	}

	return count;
}

static DEVICE_ATTR(reg_debug, 0666, reg_debug_show, reg_debug_store);
static struct attribute *tmd2771_prox_attr[] = {
	&dev_attr_prox_data.attr,
	&dev_attr_prox_enable.attr,
	&dev_attr_prox_set_delay.attr,
	&dev_attr_reg_debug.attr,
	NULL,
};

static struct attribute_group tmd2771_prox_attr_grp = {
	.attrs = tmd2771_prox_attr,
};

static struct attribute *tmd2771_als_attr[] = {
	&dev_attr_als_data.attr,
	&dev_attr_als_enable.attr,
	&dev_attr_als_set_delay.attr,
	NULL,
};

static struct attribute_group tmd2771_als_attr_grp = {
	.attrs = tmd2771_als_attr,
};

static void tmd2771_init_cfg(struct tmd2771_data *data)
{
	taos_cfgp = kzalloc(sizeof(struct taos_cfg), GFP_KERNEL);
	if (NULL == taos_cfgp) {
		printk(KERN_ERR "alloc memory for tmd2771 config fail\n");

		return;
	}

	taos_cfgp->calibrate_target = calibrate_target_param;
	taos_cfgp->als_time = als_time_param;
	taos_cfgp->scale_factor = scale_factor_param;
	taos_cfgp->gain_trim = gain_trim_param;
	taos_cfgp->filter_history = filter_history_param;
	taos_cfgp->filter_count = filter_count_param;
	taos_cfgp->gain = gain_param;
	taos_cfgp->als_threshold_hi = als_threshold_hi_param;
	taos_cfgp->als_threshold_lo = als_threshold_lo_param;
	taos_cfgp->prox_threshold_hi = prox_threshold_hi_param;
	taos_cfgp->prox_threshold_lo = prox_threshold_lo_param;
	taos_cfgp->prox_int_time = prox_int_time_param;
	taos_cfgp->prox_adc_time = prox_adc_time_param;
	taos_cfgp->prox_wait_time = prox_wait_time_param;
	taos_cfgp->prox_intr_filter = prox_intr_filter_param;
	taos_cfgp->prox_config = prox_config_param;
	taos_cfgp->prox_pulse_cnt = prox_pulse_cnt_param;
	taos_cfgp->prox_gain = prox_gain_param;
	sat_als = (256 - taos_cfgp->prox_int_time) << 10;
	sat_prox = (256 - taos_cfgp->prox_adc_time) << 10;
}

static int tmd2771_init_irq(struct tmd2771_data *data)
{
	int ret;

	ret = gpio_request(TAOS_INT_GPIO, "ALS_PS_INT");
	if (ret < 0) {
		printk(KERN_ERR "failed to request GPIO:%d,ERRNO:%d\n",
		       (int)TAOS_INT_GPIO, ret);

		return ret;
	}

	gpio_direction_input(TAOS_INT_GPIO);

	ret = request_threaded_irq(ALS_PS_INT, NULL, &tmd2771_irq_handler,
				   IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
				   "taos_irq", data);
	if (ret != 0) {
		gpio_free(TAOS_INT_GPIO);

		return ret;
	}

	return 0;
}

static int tmd2771_deinit_irq(struct tmd2771_data *data)
{
	free_irq(ALS_PS_INT, data);
	gpio_free(TAOS_INT_GPIO);
	return 0;
}

static int tmd2771_init_input(struct tmd2771_data *data)
{
	int ret = 0;

	/* init proximity sensor input device */
	data->input_dev_prox = input_allocate_device();
	if (unlikely(NULL == data->input_dev_prox)) {
		printk(KERN_ERR "alloc input prox device ret:%d\n", ret);
		ret = -ENOMEM;

		goto alloc_input_prox_err;
	}

	data->input_dev_als = input_allocate_device();
	if (unlikely(NULL == data->input_dev_als)) {
		printk(KERN_ERR "alloc input light fail, ret:%d\n", ret);
		ret = -ENOMEM;

		goto alloc_input_als_err;
	}

	data->input_dev_als->name = TAOS_INPUT_NAME_ALS;
	data->input_dev_als->id.bustype = BUS_I2C;

	data->input_dev_prox->name = TAOS_INPUT_NAME_PROX;
	data->input_dev_prox->id.bustype = BUS_I2C;
	set_bit(EV_ABS, data->input_dev_als->evbit);
	set_bit(EV_ABS, data->input_dev_prox->evbit);
	input_set_abs_params(data->input_dev_als, ABS_MISC, 0, 255, 0, 1);
	input_set_abs_params(data->input_dev_prox, ABS_DISTANCE, 0, 1000, 0, 0);

	ret = input_register_device(data->input_dev_als);
	if (unlikely(0 != ret)) {
		printk(KERN_ERR "register input light fail, ret: %d\n", ret);
		ret = -ENODEV;

		goto input_register_als_err;
	}

	ret = input_register_device(data->input_dev_prox);
	if (unlikely(0 != ret)) {
		printk(KERN_ERR "register input prox fail, ret: %d\n", ret);
		ret = -ENODEV;

		goto input_register_prox_err;
	}
	return 0;

	input_unregister_device(data->input_dev_prox);
input_register_prox_err:
	input_unregister_device(data->input_dev_als);
input_register_als_err:
	input_free_device(data->input_dev_als);
alloc_input_als_err:
	input_free_device(data->input_dev_prox);
alloc_input_prox_err:
	return ret;

}

static void tmd2771_deinit_input(struct tmd2771_data *data)
{
	input_unregister_device(data->input_dev_prox);

	input_unregister_device(data->input_dev_als);

	input_free_device(data->input_dev_als);

	input_free_device(data->input_dev_prox);
}

static int tmd2771_verify_id(struct i2c_client *client)
{
	int chip_id;

	chip_id = i2c_smbus_read_byte_data(client, TAOS_TRITON_CMD_REG |
					   (TAOS_TRITON_CNTRL + 0x12));
	if (chip_id < 0) {
		printk(KERN_ERR "read chip id fail, chip_id:0x%x\n", chip_id);

		return chip_id;
	}

	printk(KERN_INFO "tmd2771 device id is : 0x%x\n", chip_id);

	return 0;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static int __tmd2771_late_resume(struct i2c_client *client)
{
	u8 reg_cntrl = 0x2f;
	int ret = -1;
	ret = i2c_smbus_write_byte_data(p_tmd2771_data->client,
					TAOS_TRITON_CMD_REG | TAOS_TRITON_CNTRL,
					reg_cntrl);
	if (ret < 0) {
		printk(KERN_ERR "TAOS: tmd2771_late_resume fail\n");

		return ret;
	}

	return ret;
}

static int __tmd2771_early_suspend(struct i2c_client *client, pm_message_t mesg)
{
	u8 reg_val = 0, reg_cntrl = 0;
	int ret = -1;
	if (isPsensorLocked)
		return -1;

	ret = i2c_smbus_write_byte(p_tmd2771_data->client, TAOS_TRITON_CMD_REG
				   | TAOS_TRITON_CNTRL);
	if (ret < 0) {
		printk(KERN_ERR
		       "TAOS: i2c_smbus_write_byte failed in taos_resume\n");

		return ret;
	}
	reg_val = i2c_smbus_read_byte(p_tmd2771_data->client);
	if (~(reg_val & TAOS_TRITON_CNTL_PROX_DET_ENBL)) {
		reg_cntrl = reg_val & (~TAOS_TRITON_CNTL_PWRON);
		ret = i2c_smbus_write_byte_data(p_tmd2771_data->client,
						TAOS_TRITON_CMD_REG |
						TAOS_TRITON_CNTRL, reg_cntrl);
		if (ret < 0) {
			printk(KERN_ERR
			       "TAOS: i2c read failed in taos_suspend\n");

			return ret;
		}
	}

	return ret;
}

static void tmd2771_early_suspend(struct early_suspend *h)
{
	pm_message_t mesg = {.event = PM_EVENT_SUSPEND, };
	__tmd2771_early_suspend(p_tmd2771_data->client, mesg);
}

static void tmd2771_late_resume(struct early_suspend *h)
{
	__tmd2771_late_resume(p_tmd2771_data->client);
}

static struct early_suspend tmd2771_early_suspend_desc = {
	.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN,
	.suspend = tmd2771_early_suspend,
	.resume = tmd2771_late_resume,
};
#endif

static int tmd2771_probe(struct i2c_client *clientp,
			 const struct i2c_device_id *idp)
{
	int ret = 0;

	if (!i2c_check_functionality(clientp->adapter,
				     I2C_FUNC_SMBUS_BYTE_DATA)) {
		printk(KERN_ERR "i2c smbus byte data functions unsupported\n");
		ret = -EOPNOTSUPP;

		goto i2c_check_err;
	}

	ret = tmd2771_verify_id(clientp);
	if (0 != ret) {
		printk(KERN_ERR "verify tmd2771 chip fail, ret:%d\n", ret);

		goto i2c_check_err;
	}

	p_tmd2771_data = kzalloc(sizeof(struct tmd2771_data), GFP_KERNEL);
	if (unlikely(NULL == p_tmd2771_data)) {
		printk(KERN_ERR "alloc memory for tmd2771 driver fail\n");
		ret = -ENOMEM;

		goto alloc_mem_err;
	}

	p_tmd2771_data->client = clientp;
	i2c_set_clientdata(clientp, p_tmd2771_data);
	INIT_WORK(&(p_tmd2771_data->work), tmd2771_work_func);
	mutex_init(&p_tmd2771_data->update_lock);
	wake_lock_init(&p_tmd2771_data->taos_wake_lock, WAKE_LOCK_SUSPEND,
		       "taos-wake-lock");

	ret = tmd2771_init_input(p_tmd2771_data);
	if (0 != ret) {
		printk(KERN_ERR "init input device fail, ret:%d\n", ret);

		goto init_input_err;
	}

	tmd2771_init_cfg(p_tmd2771_data);

	ret = i2c_smbus_write_byte_data(clientp,
					TAOS_TRITON_CMD_REG | 0x00, 0x00);
	if (ret < 0) {
		printk(KERN_ERR "TAOS: failed in power down\n");

		goto power_chip_err;
	}

	ret = tmd2771_init_irq(p_tmd2771_data);
	if (0 != ret) {
		printk(KERN_ERR "init irq for tmd2771 fail, ret:%d\n", ret);

		goto power_chip_err;
	}
	p_tmd2771_data->taos_class =
	    class_create(THIS_MODULE, "optical_sensors");
	if (IS_ERR(p_tmd2771_data->taos_class)) {
		p_tmd2771_data->taos_class = NULL;
		goto power_chip_err;
	}
	p_tmd2771_data->ls_dev = device_create(
			p_tmd2771_data->taos_class,
			NULL, 0, "%s", "lightsensor");
	p_tmd2771_data->ps_dev = device_create(
			p_tmd2771_data->taos_class,
			NULL, 0, "%s", "proximity");
#ifdef CONFIG_HAS_EARLYSUSPEND
	register_early_suspend(&tmd2771_early_suspend_desc);
#endif

	ret = sysfs_create_group(&p_tmd2771_data->ps_dev->kobj,
				 &tmd2771_prox_attr_grp);
	if (ret < 0) {
		printk(KERN_ERR "%s(), %d: create sys console fail",
		       __func__, __LINE__);

		goto create_sys_err;
	}

	ret = sysfs_create_group(&p_tmd2771_data->ls_dev->kobj,
				 &tmd2771_als_attr_grp);
	if (ret < 0) {
		printk(KERN_ERR "%s(), %d: create sys console fail",
		       __func__, __LINE__);

		goto create_sys_err1;
	}
	printk(KERN_INFO "tmd2771 module initialize succeed\n");

	return 0;

	sysfs_remove_group(&p_tmd2771_data->input_dev_als->dev.kobj,
			   &tmd2771_als_attr_grp);
create_sys_err1:
	sysfs_remove_group(&p_tmd2771_data->input_dev_prox->dev.kobj,
			   &tmd2771_prox_attr_grp);

create_sys_err:
	tmd2771_deinit_irq(p_tmd2771_data);

power_chip_err:
	tmd2771_deinit_input(p_tmd2771_data);

init_input_err:
	kfree(p_tmd2771_data);

alloc_mem_err:

i2c_check_err:
	return ret;
}

static int tmd2771_remove(struct i2c_client *client)
{
	sysfs_remove_group(&p_tmd2771_data->input_dev_als->dev.kobj,
			   &tmd2771_als_attr_grp);

	sysfs_remove_group(&p_tmd2771_data->input_dev_prox->dev.kobj,
			   &tmd2771_prox_attr_grp);

	tmd2771_deinit_irq(p_tmd2771_data);

	tmd2771_deinit_input(p_tmd2771_data);

	kfree(p_tmd2771_data);

	return 0;
}

/* module device table */
static struct i2c_device_id taos_idtable[] = {
	{TAOS_DEVICE_ID, 0},
	{}
};

/* driver definition */
static struct i2c_driver i2c_driver_tmd2771 = {
	.driver = {
		   .owner = THIS_MODULE,
		   .name = TAOS_DEVICE_ID,
		   },
	.id_table = taos_idtable,
	.probe = tmd2771_probe,
	.remove = __devexit_p(tmd2771_remove),
};

static int __init tmd2771_module_init(void)
{
	int rc = -EINVAL;

	rc = i2c_add_driver(&i2c_driver_tmd2771);
	if (unlikely(0 != rc)) {
		printk(KERN_ERR "add i2c drier fail, rc:%d\n", rc);
		return rc;
	}
	return 0;
}

static void __exit tmd2771_module_exit(void)
{
	i2c_del_driver(&i2c_driver_tmd2771);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("proximity and ambient light driver for tmd2771");

module_init(tmd2771_module_init);
module_exit(tmd2771_module_exit);
