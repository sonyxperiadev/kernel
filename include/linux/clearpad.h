/* include/linux/clearpad.h
 *
 * Copyright (C) 2013 Sony Mobile Communications Inc.
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

#define CLEARPAD_NAME "clearpad"
#define CLEARPADI2C_NAME "clearpad-i2c"
#define CLEARPAD_RMI_DEV_NAME "clearpad-rmi-dev"

enum clearpad_funcarea_kind_e {
	SYN_FUNCAREA_INSENSIBLE,
	SYN_FUNCAREA_POINTER,
	SYN_FUNCAREA_BUTTON,
	SYN_FUNCAREA_END,
};

enum clearpad_flip_config_e {
	SYN_FLIP_NONE,
	SYN_FLIP_X,
	SYN_FLIP_Y,
	SYN_FLIP_XY,
};

struct clearpad_area_t {
	int x1;
	int y1;
	int x2;
	int y2;
};

struct clearpad_funcarea_t {
	struct clearpad_area_t original; /* actual area */
	struct clearpad_area_t extension; /* extended area to track events */
	enum clearpad_funcarea_kind_e func;
	void *data;
};

struct clearpad_pointer_data_t {
	int offset_x;
	int offset_y;
};

struct clearpad_button_data_t {
	int code;
	bool down;
	bool down_report;
};

struct clearpad_platform_data_t {
	int irq_gpio;
	u32 irq_gpio_flags;
	char *symlink_name;
	bool watchdog_enable;
	int watchdog_poll_t_ms;
};

struct clearpad_bus_data_t {
	__u16 bustype;
	struct device *dev;
	struct device_node *of_node;
	int (*set_page)(struct device *dev, u8 page);
	int (*read)(struct device *dev, u16 addr, u8 *buf, u8 len);
	int (*write)(struct device *dev, u16 addr, const u8 *buf, u8 len);
	int (*read_block)(struct device *dev, u16 addr, u8 *buf, int len);
	int (*write_block)(struct device *dev, u16 addr, const u8 *buf,
				int len);
};

struct clearpad_data_t {
	struct clearpad_platform_data_t *pdata;
	struct clearpad_bus_data_t *bdata;
	int probe_retry;
#ifdef CONFIG_TOUCHSCREEN_CLEARPAD_RMI_DEV
	struct platform_device *rmi_dev;
#endif
};

#endif
