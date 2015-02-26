/* Copyright (c) 2012-2013, The Linux Foundation. All rights reserved.
 * Copyright (C) 2013 Sony Mobile Communications AB.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#ifndef SONY_IMX134_POWER_SETTINGS_H
#define SONY_IMX134_POWER_SETTINGS_H

#include "msm_sensor.h"

#ifndef CONFIG_SONY_CAMERA
#define SENSOR_GPIO_CAM_VAA_V2P8	-1
#define SENSOR_GPIO_CAM_VDDIO_V1P8	-1
#define SENSOR_GPIO_CAM_VDDAF_V2P8	-1
#endif

static struct msm_sensor_power_setting imx134_seagull_power_setting[] = {
	{
		.seq_type = SENSOR_GPIO, /*CAM_VAA_V2P8: GPIO_69 - LOW*/
		.seq_val = SENSOR_GPIO_CAM_VAA_V2P8,
		.config_val = GPIO_OUT_LOW,
		.delay = 0,
	},
	{
		.seq_type = SENSOR_GPIO, /*CAM_VAA_V2P8: GPIO_69 - HIGH*/
		.seq_val = SENSOR_GPIO_CAM_VAA_V2P8,
		.config_val = GPIO_OUT_HIGH,
		.delay = 0,
	},
	{
		.seq_type = SENSOR_VREG, /*VREG_L5*/
		.seq_val = CAM_VDIG,
		.config_val = 0,
		.delay = 1,
	},
	{
		.seq_type = SENSOR_GPIO, /*CAM_VDDIO_V1P8: GPIO_112 - LOW*/
		.seq_val = SENSOR_GPIO_CAM_VDDIO_V1P8,
		.config_val = GPIO_OUT_LOW,
		.delay = 0,
	},
	{
		.seq_type = SENSOR_GPIO, /*CAM_VDDIO_V1P8: GPIO_112 - HIGH*/
		.seq_val = SENSOR_GPIO_CAM_VDDIO_V1P8,
		.config_val = GPIO_OUT_HIGH,
		.delay = 1,
	},
	{
		.seq_type = SENSOR_CLK,  /*CAM_MCLK*/
		.seq_val = SENSOR_CAM_MCLK,
		.config_val = 24000000,
		.delay = 2,
	},
	{
		.seq_type = SENSOR_GPIO, /*CAM_8M_RSTN: GPIO_36 - LOW*/
		.seq_val = SENSOR_GPIO_RESET,
		.config_val = GPIO_OUT_LOW,
		.delay = 0,
	},
	{
		.seq_type = SENSOR_GPIO,  /*CAM_8M_RSTN: GPIO_36 - HIGH*/
		.seq_val = SENSOR_GPIO_RESET,
		.config_val = GPIO_OUT_HIGH,
		.delay = 1,
	},
	{
		.seq_type = SENSOR_GPIO, /*CAM_VDDAF_V2P8: GPIO_111 - LOW*/
		.seq_val = SENSOR_GPIO_CAM_VDDAF_V2P8,
		.config_val = GPIO_OUT_LOW,
		.delay = 0,
	},
	{
		.seq_type = SENSOR_GPIO, /*CAM_VDDAF_V2P8: GPIO_111 - HIGH*/
		.seq_val = SENSOR_GPIO_CAM_VDDAF_V2P8,
		.config_val = GPIO_OUT_HIGH,
		.delay = 15,
	},
	{
		.seq_type = SENSOR_I2C_MUX,
		.seq_val = 0,
		.config_val = 0,
		.delay = 0,
	},
};

#endif /* SONY_IMX134_POWER_SETTINGS_H */
