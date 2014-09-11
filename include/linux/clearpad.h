/* include/linux/clearpad.h
 *
 * Copyright (C) 2010 Sony Ericsson Mobile Communications AB.
 * Copyright (C) 2012 - 2013 Sony Mobile Communications AB.
 *
 * Author: Courtney Cavin <courtney.cavin@sonyericsson.com>
 *         Yusuke Yoshimura <Yusuke.Yoshimura@sonymobile.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef __LINUX_CLEARPAD_H
#define __LINUX_CLEARPAD_H

#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/input/evgen_helper.h>

#define CLEARPAD_NAME "clearpad"
#define CLEARPADI2C_NAME "clearpad-i2c"
#define CLEARPAD_RMI_DEV_NAME "clearpad-rmi-dev"

enum synaptics_funcarea_kind {
	SYN_FUNCAREA_INSENSIBLE,
	SYN_FUNCAREA_POINTER,
	SYN_FUNCAREA_BUTTON,
	SYN_FUNCAREA_END,
};

enum synaptics_flip_config {
	SYN_FLIP_NONE,
	SYN_FLIP_X,
	SYN_FLIP_Y,
	SYN_FLIP_XY,
};

struct synaptics_area {
	int x1;
	int y1;
	int x2;
	int y2;
};

struct synaptics_funcarea {
	struct synaptics_area original; /* actual area */
	struct synaptics_area extension; /* extended area to track events */
	enum synaptics_funcarea_kind func;
	void *data;
};

struct synaptics_pointer_data {
	int offset_x;
	int offset_y;
};

struct synaptics_button_data {
	int code;
	bool down;
	bool down_report;
};

struct synaptics_easy_wakeup_config {
	bool gesture_enable;
	bool large_panel;
	unsigned long timeout_delay;
};

struct clearpad_platform_data {
	int irq_gpio;
	u32 irq_gpio_flags;
	char *symlink_name;
	bool watchdog_enable;
	int watchdog_poll_t_ms;
	struct synaptics_easy_wakeup_config *easy_wakeup_config;
};

struct clearpad_bus_data {
	__u16 bustype;
	struct device *dev;
	struct device_node *of_node;
	int (*read)(struct device *dev, u8 reg, u8 *buf, u8 len);
	int (*write)(struct device *dev, u8 reg, const u8 *buf, u8 len);
	int (*read_block)(struct device *dev, u16 addr, u8 *buf, int len);
	int (*write_block)(struct device *dev, u16 addr, const u8 *buf,
				int len);
};

struct clearpad_data {
	struct clearpad_platform_data *pdata;
	struct clearpad_bus_data *bdata;
	int probe_retry;
#ifdef CONFIG_TOUCHSCREEN_CLEARPAD_RMI_DEV
	struct platform_device *rmi_dev;
#endif
};

#endif
