/* include/media/semc_sensor_v4l2.h
 *
 * Copyright (C) 2012 Sony Mobile Communications AB.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef __LINUX_SEMC_SENSOR_V4L2_H
#define __LINUX_SEMC_SENSOR_V4L2_H

#ifdef __KERNEL__

enum semc_sensor_cmd {
	CAM_VDIG,
	CAM_VIO,
	CAM_VANA,
	CAM_VAF,
	GPIO_AF,
	GPIO_RESET,
	CAM_CLK,
	EXIT,
};

struct semc_sensor_seq {
	enum semc_sensor_cmd	cmd;
	int			val;
	int			wait;
};

struct semc_sensor_module {
	char				*name;
	uint16_t			i2c_addr;
	const struct semc_sensor_seq	*seq_on;
	const struct semc_sensor_seq	*seq_off;
};

extern const struct semc_sensor_module sensor_main_modules[];
extern const int semc_sensor_main_modules_len;
extern const struct semc_sensor_module sensor_sub_modules[];
extern const int semc_sensor_sub_modules_len;
extern const int semc_sensor_gpio_af_power_pin;
extern const int semc_sensor_main_eeprom_addr;
extern const int semc_sensor_main_subdev_code;
extern const int semc_sensor_sub_eeprom_addr;
extern const int semc_sensor_sub_subdev_code;

#endif
#endif
