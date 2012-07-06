/* include/linux/clearpad.h
 *
 * Copyright (C) 2010 Sony Ericsson Mobile Communications AB.
 * Copyright (C) 2012 Sony Mobile Communications AB.
 *
 * Author: Courtney Cavin <courtney.cavin@sonyericsson.com>
 *         Yusuke Yoshimura <Yusuke.Yoshimura@sonyericsson.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef __LINUX_CLEARPAD_H
#define __LINUX_CLEARPAD_H

#include <linux/kernel.h>
#include <linux/platform_device.h>

#define CLEARPAD_NAME "clearpad"
#define CLEARPADI2C_NAME "clearpad-i2c"
#define CLEARPAD_RMI_DEV_NAME "clearpad-rmi-dev"

enum synaptics_funcarea_kind {
	SYN_FUNCAREA_INSENSIBLE,
	SYN_FUNCAREA_POINTER,
	SYN_FUNCAREA_BUTTON,
	SYN_FUNCAREA_END,
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

struct clearpad_platform_data {
	int irq;
	struct synaptics_funcarea * (*funcarea_get)(u8, u8);
	int (*vreg_configure)(int);
	int (*vreg_suspend)(int);
	int (*vreg_reset)(void);
	int (*gpio_configure)(int);
	int (*gpio_export)(struct device *, bool);
};

struct clearpad_bus_data {
	__u16 bustype;
	struct device *dev;
	int (*read)(struct device *dev, u8 reg, u8 *buf, u8 len);
	int (*write)(struct device *dev, u8 reg, const u8 *buf, u8 len);
	int (*read_block)(struct device *dev, u16 addr, u8 *buf, int len);
	int (*write_block)(struct device *dev, u16 addr, const u8 *buf,
				int len);
};

struct clearpad_data {
	struct clearpad_platform_data *pdata;
	struct clearpad_bus_data *bdata;
};
#endif
