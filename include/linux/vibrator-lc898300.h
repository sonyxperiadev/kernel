/*
 * vibrator-lc898300.h - platform data structure for lc898300 vibrator
 *
 * Copyright (C) 2012 Sony Ericsson Mobile Communications AB
 * Copyright (c) 2012 Sony Mobile Communications AB
 *
 * Authors: Ardiana Karppinen <ardiana.karppinen@sonyericsson.com>
 *          Stefan Karlsson <stefan3.karlsson@sonyericsson.com>
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
	VIB_CMD_PWM_1_15 = 0x01,
	VIB_CMD_PWM_2_15 = 0x02,
	VIB_CMD_PWM_3_15 = 0x03,
	VIB_CMD_PWM_4_15 = 0x04,
	VIB_CMD_PWM_5_15 = 0x05,
	VIB_CMD_PWM_6_15 = 0x06,
	VIB_CMD_PWM_7_15 = 0x07,
	VIB_CMD_PWM_8_15 = 0x08,
	VIB_CMD_PWM_9_15 = 0x09,
	VIB_CMD_PWM_10_15 = 0x0a,
	VIB_CMD_PWM_11_15 = 0x0b,
	VIB_CMD_PWM_12_15 = 0x0c,
	VIB_CMD_PWM_13_15 = 0x0d,
	VIB_CMD_PWM_14_15 = 0x0e,
	VIB_CMD_PWM_15_15 = 0x0f,
};

enum vib_cmd_resonance {
	VIB_CMD_FREQ_125 = 0x00,
	VIB_CMD_FREQ_150 = 0x05,
	VIB_CMD_FREQ_200 = 0x0f,
};

enum vib_cmd_startup {
	VIB_CMD_STTIME_0 = 0x00,
	VIB_CMD_STTIME_1 = 0x01,
	VIB_CMD_STTIME_2 = 0x02,
	VIB_CMD_STTIME_3 = 0x03,
	VIB_CMD_STTIME_4 = 0x04,
	VIB_CMD_STTIME_5 = 0x05,
	VIB_CMD_STTIME_6 = 0x06,
	VIB_CMD_STTIME_7 = 0x07,
};

enum vib_cmd_brake {
	VIB_CMD_BRPWR_OFF = 0x00,
	VIB_CMD_BRPWR_1_15 = 0x01,
	VIB_CMD_BRPWR_2_15 = 0x02,
	VIB_CMD_BRPWR_3_15 = 0x03,
	VIB_CMD_BRPWR_4_15 = 0x04,
	VIB_CMD_BRPWR_5_15 = 0x05,
	VIB_CMD_BRPWR_6_15 = 0x06,
	VIB_CMD_BRPWR_7_15 = 0x07,
	VIB_CMD_BRPWR_8_15 = 0x08,
	VIB_CMD_BRPWR_9_15 = 0x09,
	VIB_CMD_BRPWR_10_15 = 0x0a,
	VIB_CMD_BRPWR_11_15 = 0x0b,
	VIB_CMD_BRPWR_12_15 = 0x0c,
	VIB_CMD_BRPWR_13_15 = 0x0d,
	VIB_CMD_BRPWR_14_15 = 0x0e,
	VIB_CMD_BRPWR_15_15 = 0x0f,
	VIB_CMD_BRTIME_0 = 0x00 << 4,
	VIB_CMD_BRTIME_1 = 0x01 << 4,
	VIB_CMD_BRTIME_2 = 0x02 << 4,
	VIB_CMD_BRTIME_3 = 0x03 << 4,
	VIB_CMD_ATBR = 0x01 << 6,
};

enum vib_cmd_stops {
	VIB_CMD_ATSNUM_0_10 = 0x00,
	VIB_CMD_ATSNUM_1_10 = 0x01,
	VIB_CMD_ATSNUM_2_10 = 0x02,
	VIB_CMD_ATSNUM_3_10 = 0x03,
	VIB_CMD_ATSNUM_4_10 = 0x04,
	VIB_CMD_ATSNUM_5_10 = 0x05,
	VIB_CMD_ATSNUM_6_10 = 0x06,
	VIB_CMD_ATSNUM_7_10 = 0x07,
	VIB_CMD_ATSNUM_8_10 = 0x08,
	VIB_CMD_ATSNUM_9_10 = 0x09,
	VIB_CMD_ATSNUM_10_10 = 0x0a,
	VIB_CMD_ATSNUM_MASK = 0x0f,
	VIB_CMD_ATSOFF = 0x00 << 4,
	VIB_CMD_ATSON = 0x01 << 4,
};

struct lc898300_vib_cmd {
	u8 vib_cmd_intensity;
	u8 vib_cmd_resonance;
	u8 vib_cmd_startup;
	u8 vib_cmd_brake;
	u8 vib_cmd_stops;
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
