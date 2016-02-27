/*
 * Authors: Atsushi Iyogi <Atsushi.XA.Iyogi@sonyericsson.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */
/*
 * Copyright (C) 2014 Sony Mobile Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 */

#ifndef _LINUX_LDO_VIBRATOR_H
#define _LINUX_LDO_VIBRATOR_H

#include <linux/gpio.h>
#include <linux/pwm.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/hrtimer.h>
#include <linux/of_device.h>
#include <../../drivers/staging/android/timed_output.h>

#define LDO_VIBRATOR_NAME "ldo_vibrator"

struct ldo_vibrator_data {
	struct device *dev;
	struct hrtimer vib_timer;
	struct timed_output_dev timed_dev;
	struct work_struct work;

	int gpio;
	int state;
	int timeout;
	struct mutex lock;
};
#endif
