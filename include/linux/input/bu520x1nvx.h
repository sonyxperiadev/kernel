/* include/linux/input/bu520x1nvx.h
 *
 * Author: Atsushi Iyogi <atsushi.x.iyogi@sonymobile.com>
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

#ifndef _LINUX_INPUT_BU520X1NVX_H
#define _LINUX_INPUT_BU520X1NVX_H

struct device;

struct bu520x1nvx_gpio_event {
	bool lid_pin;
	int gpio;
	int active_low;
	const char *desc;
	int debounce_interval;
	unsigned int irq;
};

struct bu520x1nvx_platform_data {
	int n_events;
	struct bu520x1nvx_gpio_event *events;
};

#endif
