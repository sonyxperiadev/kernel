/*
 * lp855x.h - TI LP8556 Backlight Driver
 *
 * Copyright (C) 2011 Texas Instruments
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
/*
 * NOTE: This file has been modified by Sony Mobile Communications Inc.
 * Modifications are Copyright (c) 2014 Sony Mobile Communications Inc,
 * and licensed under the license of the file.
 */

#ifndef _LP855X_H
#define _LP855X_H

struct lp855x_pwm_data {
	void (*pwm_set_intensity) (int brightness, int max_brightness);
	int (*pwm_get_intensity) (int max_brightness);
};

struct lp855x_rom_data {
	u8 addr;
	u8 val;
};

/**
 * struct lp855x_platform_data
 * @name : backlight driver name
 * @mode : brightness control by pwm or lp855x register
 * @device_control : value of DEVICE CONTROL register
 * @initial_brightness : initial value of backlight brightness
 * @max_brightness : maximum value of backlight brightness
 * @pwm_data : platform specific pwm generation functions.
		Only valid when mode is PWM_BASED.
 * @load_new_rom_data :
	0 : use default configuration data
	1 : update values of eeprom or eprom registers on loading driver
 * @size_program : total size of lp855x_rom_data
 * @rom_data : list of new eeprom/eprom registers
 * @cfg3 : value of cfg3 register
 */
struct lp855x_platform_data {
	const char *name;
	const char *default_trigger;
	u8 device_control;
	int initial_brightness;
	unsigned int period_ns;
	int max_brightness;
	struct lp855x_pwm_data pwm_data;
	u8 load_new_rom_data;
	int size_program;
	struct lp855x_rom_data *rom_data;
	u8 slope_reg;
	unsigned gpio_bl_enable;
	bool bl_enable_valid;
};

#endif
