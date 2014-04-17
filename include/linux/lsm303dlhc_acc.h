/*
 * lsm303dlhc_acc.h -  lsm303dlh accelerometer driver API
 *
 * Copyright (C) 2010 Sony Ericsson Mobile Communications AB.
 * License terms: GNU General Public License (GPL) version 2
 * Author: Aleksej Makarov <aleksej.makarov@sonyericsson.com>
 *
 */
#ifndef _LSM303DLHC_ACC_H_
#define _LSM303DLHC_ACC_H_
#include <linux/device.h>
#include <linux/kernel.h>

#define LSM303DLHC_ACC_DEV_NAME "lsm303dlhc_acc"

enum lsm303dlhc_acc_power_sate {
	LSM303DLHC_PWR_ON,
	LSM303DLHC_PWR_OFF,
	LSM303DLHC_STANDBY,
};

enum lsm303dlhc_acc_mode {
	MODE_POLL,
	MODE_INTERRUPT,
	MODE_6D_INTERRUPT,
	MODE_DRDY,
};

struct lsm303dlhc_acc_platform_data {
	int range;
	int poll_interval_ms;
	int irq_pad;
	enum lsm303dlhc_acc_mode mode;
	int (*power)
		(struct device *dev, enum lsm303dlhc_acc_power_sate pwr_state);
	int (*power_config)(struct device *dev, bool value);
};
#endif

