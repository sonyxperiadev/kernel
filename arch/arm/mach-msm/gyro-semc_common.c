/* arch/arm/mach-msm/gyro-semc_common.c
 *
 * Copyright (C) 2011 Sony Ericsson Mobile Communications AB.
 * Copyright (C) 2012 Sony Mobile Communications AB.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifdef CONFIG_SENSORS_MPU3050

#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/i2c.h>
#include <linux/bma250_ng_common.h>
#include <linux/mpu3050.h>
#include <linux/mutex.h>
#include "gyro-semc_common.h"

static DEFINE_MUTEX(mutex_mpu_bypass);
static int bma250_sleep_state;

static int i2c_rxdata_from_mpu3050
		(struct i2c_client *ic_dev, char *buf, int length)
{
	struct i2c_msg msgs[] = {
		{
			.addr = 0xD0 >> 1,
			.flags = 0,
			.len = 1,
			.buf = buf,
		},
		{
			.addr = 0xD0 >> 1,
			.flags = I2C_M_RD,
			.len = length,
			.buf = buf,
		},
	};

	if (i2c_transfer(ic_dev->adapter, msgs, 2) < 0)
		return -EIO;
	else
		return 0;
}

void mpu3050_bypassmode(int rw, char *bypass)
{
	static char mpu3050_user_ctl;

	mutex_lock(&mutex_mpu_bypass);
	if (rw == READ_BYPASS_STATE)
		*bypass = (mpu3050_user_ctl & BIT_AUX_IF_EN);
	else
		mpu3050_user_ctl = *bypass;
	mutex_unlock(&mutex_mpu_bypass);
}

int bma250_read_axis_from_mpu3050(
		struct i2c_client *ic_dev, char *buf, int length)
{
	*buf = MPUREG_23_RSVD;

	if (i2c_rxdata_from_mpu3050(ic_dev, buf, length)) {
		printk(KERN_ERR "%s: error to check bypass mode\n", __func__);
		return -EIO;
	}
	return 0;
}

int check_bma250_sleep_state(void)
{
	return bma250_sleep_state;
}

void vote_bma250_sleep_state(int id, int vote)
{
	if (vote)
		bma250_sleep_state |= id;
	else
		bma250_sleep_state &= ~id;
}

#endif
