/* drivers/media/platform/msm/camera_v2/sensor/sony_camera_v4l2.h
 *
 * Copyright (C) 2012 Sony Mobile Communications AB.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef __LINUX_SONY_CAMERA_V4L2_H
#define __LINUX_SONY_CAMERA_V4L2_H

#ifdef __KERNEL__

#define MAX_PLL_NUM 30

enum sony_camera_cmd {
	SONY_CAM_VDIG,
	SONY_CAM_VIO,
	SONY_CAM_VANA,
	SONY_CAM_VAF,
	SONY_GPIO_AF,
	SONY_GPIO_RESET,
	SONY_CAM_CLK,
	SONY_I2C_WRITE,
	EXIT,
};

struct sony_camera_seq {
	enum sony_camera_cmd	cmd;
	int			val1;
	int			val2;
	int			wait;
};

struct sony_camera_module {
	const char		*name;
	struct sony_camera_seq	*seq_on;
	struct sony_camera_seq	*seq_off;
	uint32_t		mount_angle;
	uint32_t		sensor_rotation;
	uint32_t		sensor_facing;
	uint32_t		pixel_number_w;
	uint32_t		pixel_number_h;
	const char		*diagonal_len;
	const char		*unit_cell_size;
	const char		*min_f_number;
	const char		*max_f_number;
	uint32_t		has_focus_actuator;
	uint32_t		has_3a;
	uint32_t		pll_num;
	uint32_t		pll[MAX_PLL_NUM];
	uint32_t		reserved;
};

struct sony_camera_info {
	uint16_t			i2c_addr;
	uint16_t			eeprom_addr;
	int				eeprom_type;
	uint16_t			eeprom_max_len;
	int				gpio_af;
	int				subdev_code;
	struct sony_camera_module	*modules;
	int				modules_num;
	const char			*default_module_name;
};

#endif
#endif
