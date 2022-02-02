/*
 * Authors: Atsushi Iyogi <Atsushi.XA.Iyogi@sonyericsson.com>
 * Copyright (C) 2017 Sony Mobile Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef _LINUX_LDO_VIBRATOR_H
#define _LINUX_LDO_VIBRATOR_H

#include <linux/gpio.h>
#include <linux/pwm.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/hrtimer.h>
#include <linux/of_device.h>
#include <linux/leds.h>

#define LDO_VIBRATOR_NAME "ldo_vibrator"

struct ldo_vibrator_data {
	struct device *dev;
	struct hrtimer vib_timer;
	struct led_classdev led_dev;
	struct work_struct work;

	int gpio;
	int state;
	int timeout;
	int play_time_ms;
	struct mutex lock;
};
#endif
