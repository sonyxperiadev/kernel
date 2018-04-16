/* include/linux/s210st0x.h
 *
 * Author: Atsushi Iyogi <atsushi2.X.iyogi@sonymobile.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */
/*
 * Copyright (C) 2016 Sony Mobile Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 */

#ifndef _LINUX_S210ST0X_H
#define _LINUX_S210ST0X_H

struct device;

struct s210st0x_gpio_event {
	int index;
	int gpio;
	int active_low;
	const char *desc;
	int debounce_interval;
	unsigned int irq;
};

struct s210st0x_platform_data {
	int n_events;
	struct s210st0x_gpio_event *events;
};

#endif
