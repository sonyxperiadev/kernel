/*
 * lsm303dlh_mag.h -  lsm303dlh magnetometer driver API
 *
 * Copyright (C) 2010 Sony Ericsson Mobile Communications AB.
 *
 * Author: Aleksej Makarov <aleksej.makarov@sonyericsson.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */
#ifndef _LSM303DLH_MAG_H_
#define _LSM303DLH_MAG_H_

enum lsm303dlh_mag_range {
	LSM303_RANGE_1300mG,
	LSM303_RANGE_1900mG,
	LSM303_RANGE_2500mG,
	LSM303_RANGE_4000mG,
	LSM303_RANGE_4700mG,
	LSM303_RANGE_5600mG,
	LSM303_RANGE_8200mG,
	LSM303_RANGE_MAX = LSM303_RANGE_8200mG
};
#define LSM303DLH_MAG_DEV_NAME "lsm303dlh_mag"

#ifdef __KERNEL__
#include <linux/device.h>
#include <linux/kernel.h>

struct lsm303dlh_mag_platform_data {
	int poll_interval_ms;
	enum lsm303dlh_mag_range range;
	int (*power_on)(struct device *dev);
	int (*power_off)(struct device *dev);
	int (*power_config)(struct device *dev, bool value);
};
#endif
#endif

