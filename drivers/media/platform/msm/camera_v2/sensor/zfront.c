/* Copyright (c) 2012-2013, The Linux Foundation. All rights reserved.
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
#include <linux/of.h>

#include <mach/gpiomux.h>
#include "msm_sensor.h"
#include "msm_cci.h"
#include "msm_camera_io_util.h"
#include "msm_camera_i2c_mux.h"
#include "eeprom/msm_eeprom.h"

#define ZFRONT_SENSOR_NAME "zfront"
#define SONY_SENSOR_NAME_LEN 8
DEFINE_MSM_MUTEX(zfront_mut);
static struct msm_sensor_ctrl_t zfront_s_ctrl;
extern struct msm_eeprom_memory_block_t *sony_eeprom_blocks;

static struct msm_sensor_power_setting zfront_power_setting[] = {
	{
		.seq_type = SENSOR_VREG,
		.seq_val = CAM_VDIG,
		.config_val = 1200000,
		.delay = 1,
	},
	{
		.seq_type = SENSOR_VREG,
		.seq_val = CAM_VIO,
		.config_val = 0,
		.delay = 1,
	},
	{
		.seq_type = SENSOR_VREG,
		.seq_val = CAM_VANA,
		.config_val = 2700000,
		.delay = 0,
	},
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_RESET,
		.config_val = GPIO_OUT_LOW,
		.delay = 1,
	},
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_RESET,
		.config_val = GPIO_OUT_HIGH,
		.delay = 1,
	},
	{
		.seq_type = SENSOR_CLK,
		.seq_val = SENSOR_CAM_MCLK,
		.config_val = 8000000,
		.delay = 1,
	},
};

static int32_t zfront_sensor_match_id(
		struct msm_sensor_ctrl_t *s_ctrl)
{
#ifdef CONFIG_SONY_CAMERA
	struct msm_camera_sensor_board_info *snsdata = s_ctrl->sensordata;
	uint8_t *eepdata;

	eepdata = sony_eeprom_get_data(s_ctrl->cci_i2c_master);

	pr_debug("%s: got data: %s\n", __func__, eepdata);

	if (!eepdata)
		return -ENOMEM;

	if (!strncmp(snsdata->sensor_name,
			eepdata, SONY_SENSOR_NAME_LEN)) {
		pr_info("%s: Sensor name matches with eeprom!\n", __func__);
		return 0;
	}

	pr_err("%s: No match. EEPROM data: %s\n", __func__, eepdata);
#endif
	return -EINVAL;
}

static struct v4l2_subdev_info zfront_subdev_info[] = {
	{
		.code = 0, //V4L2_MBUS_FMT_SRGGB10_1X10,
		.colorspace = V4L2_COLORSPACE_JPEG,
		.fmt = 1,
		.order = 0,
	},
};

static const struct i2c_device_id zfront_i2c_id[] = {
	{
		ZFRONT_SENSOR_NAME,
		(kernel_ulong_t)&zfront_s_ctrl
	},
	{ }
};

static int32_t zfront_i2c_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
	return msm_sensor_i2c_probe(client, id, &zfront_s_ctrl);
}

static struct i2c_driver zfront_i2c_driver = {
	.id_table = zfront_i2c_id,
	.probe  = zfront_i2c_probe,
	.driver = {
		.name = ZFRONT_SENSOR_NAME,
	},
};

static struct msm_camera_i2c_client zfront_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_WORD_ADDR,
};

static const struct of_device_id zfront_dt_match[] = {
	{
		.compatible = "qcom,zfront",
		.data = &zfront_s_ctrl},
	{ }
};

MODULE_DEVICE_TABLE(of, zfront_dt_match);

static struct platform_driver zfront_platform_driver = {
	.driver = {
		.name = "qcom,zfront",
		.owner = THIS_MODULE,
		.of_match_table = zfront_dt_match,
	},
};

static int32_t zfront_platform_probe(
		struct platform_device *pdev)
{
	int32_t rc = 0;
	const struct of_device_id *match;
	match = of_match_device(zfront_dt_match, &pdev->dev);
	rc = msm_sensor_platform_probe(pdev, match->data);
	return rc;
}

static int __init zfront_init_module(void)
{
	int32_t rc = 0;
	pr_info("%s:%d\n", __func__, __LINE__);
	zfront_s_ctrl.power_setting_array.power_setting =
				zfront_power_setting;
	zfront_s_ctrl.power_setting_array.size =
				ARRAY_SIZE(zfront_power_setting);

	rc = platform_driver_probe(&zfront_platform_driver,
		zfront_platform_probe);
	if (!rc)
		return rc;
	pr_err("%s:%d rc %d\n", __func__, __LINE__, rc);
	return i2c_add_driver(&zfront_i2c_driver);
}

static void __exit zfront_exit_module(void)
{
	pr_info("%s:%d\n", __func__, __LINE__);
	if (zfront_s_ctrl.pdev) {
		msm_sensor_free_sensor_data(&zfront_s_ctrl);
		platform_driver_unregister(&zfront_platform_driver);
	} else
		i2c_del_driver(&zfront_i2c_driver);
	return;
}

#ifdef CONFIG_SONY_CAMERA
static struct msm_sensor_fn_t zfront_sensor_func_tbl = {
	.sensor_config = msm_sensor_config,
#ifdef CONFIG_COMPAT
	.sensor_config32 = msm_sensor_config32,
#endif
	.sensor_power_up = msm_sensor_power_up,
	.sensor_power_down = msm_sensor_power_down,
	.sensor_match_id = zfront_sensor_match_id,
};
#endif

static struct msm_sensor_ctrl_t zfront_s_ctrl = {
	.sensor_i2c_client = &zfront_sensor_i2c_client,
	.msm_sensor_mutex = &zfront_mut,
	.sensor_v4l2_subdev_info = zfront_subdev_info,
	.sensor_v4l2_subdev_info_size = ARRAY_SIZE(zfront_subdev_info),
#ifdef CONFIG_SONY_CAMERA
	.func_tbl = &zfront_sensor_func_tbl,
#endif
};

module_init(zfront_init_module);
module_exit(zfront_exit_module);
MODULE_DESCRIPTION("zfront");
MODULE_LICENSE("GPL v2");
