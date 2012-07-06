/*
 * vibrator-lc898300.h - platform data structure for lc898300 vibrator
 *
 * Copyright (C) 2010 Ardiana Karppinen <ardiana.karppinen@sonyericsson.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 */

#ifndef __LINUX_LC898300_H
#define __LINUX_LC898300_H
#include <linux/platform_device.h>

#define LC898300_I2C_NAME       "lc898300-vib-i2c"
enum vib_cmd_intensity {
	VIB_CMD_PWM_OFF   = 0x00,
	VIB_CMD_PWM_10_15 = 0x0a,
	VIB_CMD_PWM_MAX   = 0xff,
};

enum vib_cmd_resonance {
	VIB_CMD_FREQ_125 = 0x00,
	VIB_CMD_FREQ_150 = 0x05,
	VIB_CMD_FREQ_200 = 0xff,
};

struct lc898300_vib_cmd {
	enum vib_cmd_intensity vib_cmd_intensity;
	enum vib_cmd_resonance vib_cmd_resonance;
};

struct lc898300_platform_data {
	const char *name;
	int (*power_config)(struct device *dev, bool on);
	int (*power_enable)(struct device *dev, bool on);
	int (*gpio_allocate)(struct device *dev);
	void (*gpio_release)(struct device *dev);
	int (*rstb_gpio_setup)(bool value);
	int (*en_gpio_setup)(bool value);
	struct lc898300_vib_cmd *vib_cmd_info;
};

#endif /* __LINUX_lc898300_H */
