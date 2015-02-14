/* Copyright (c) 2013-2014, The Linux Foundation. All rights reserved.
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
#include <mach/gpiomux.h>
#include "msm_sensor.h"
#include "msm_cci.h"
#include "msm_camera_io_util.h"
#include "msm_camera_i2c_mux.h"


#define GC0339_SENSOR_NAME "gc0339"
DEFINE_MSM_MUTEX(gc0339_mut);

#undef CDBG
#ifdef CONFIG_MSMB_CAMERA_DEBUG
#define CDBG(fmt, args...) pr_err(fmt, ##args)
#else
#define CDBG(fmt, args...) do { } while (0)
#endif


static struct msm_sensor_ctrl_t gc0339_s_ctrl;

static struct msm_sensor_power_setting gc0339_power_setting[] = {
		{//1.PDN "L"
			.seq_type = SENSOR_GPIO,
			.seq_val = SENSOR_GPIO_STANDBY,
			.config_val = GPIO_OUT_LOW,
			.delay = 0,
		},
		{//2. MCLK
			.seq_type = SENSOR_CLK,
			.seq_val = SENSOR_CAM_MCLK,
			.config_val = 24000000,
			.delay = 1,
		},
		{//3. LVS
			.seq_type = SENSOR_VREG,
			.seq_val = CAM_VIO,
			.config_val = 0,
			.delay = 0,
		},
		{//4.VANA
			.seq_type = SENSOR_VREG,
			.seq_val = CAM_VANA,
			.config_val = 0,
			.delay = 0,
		},
		{//5. MVDD
			.seq_type = SENSOR_GPIO,
			.seq_val = SENSOR_GPIO_VIO,
			.config_val = GPIO_OUT_HIGH,
			.delay = 0,
		},
		{//6. Reset "L"
			.seq_type = SENSOR_GPIO,
			.seq_val = SENSOR_GPIO_RESET,
			.config_val = GPIO_OUT_LOW,
			.delay = 0,
		},
		{//7. Reset "H"
			.seq_type = SENSOR_GPIO,
			.seq_val = SENSOR_GPIO_RESET,
			.config_val = GPIO_OUT_HIGH,
			.delay = 1,
		},
};

static struct msm_sensor_power_setting gc0339_power_down_setting [] = {
		{//8. MCLK
			.seq_type = SENSOR_CLK,
			.seq_val = SENSOR_CAM_MCLK,
			.config_val = 24000000,
			.delay = 0,
		},
		{//7. MVDD
			.seq_type = SENSOR_GPIO,
			.seq_val = SENSOR_GPIO_VIO,
			.config_val = GPIO_OUT_HIGH,
			.delay = 0,
		},
		{//6. LVS
			.seq_type = SENSOR_VREG,
			.seq_val = CAM_VIO,
			.config_val = 0,
			.delay = 0,
		},
		{//5.VANA
			.seq_type = SENSOR_VREG,
			.seq_val = CAM_VANA,
			.config_val = 0,
			.delay = 0,
		},
		{//4. Reset "L"
			.seq_type = SENSOR_GPIO,
			.seq_val = SENSOR_GPIO_RESET,
			.config_val = GPIO_OUT_LOW,
			.delay = 0,
		},
		{//3.PDN "H"
			.seq_type = SENSOR_GPIO,
			.seq_val = SENSOR_GPIO_STANDBY,
			.config_val = GPIO_OUT_HIGH,
			.delay = 0,
		},
		{//2. Reset "H"
			.seq_type = SENSOR_GPIO,
			.seq_val = SENSOR_GPIO_RESET,
			.config_val = GPIO_OUT_HIGH,
			.delay = 0,
		},
		{//1. Reset "L"
			.seq_type = SENSOR_GPIO,
			.seq_val = SENSOR_GPIO_RESET,
			.config_val = GPIO_OUT_LOW,
			.delay = 0,
		},
};


static struct v4l2_subdev_info gc0339_subdev_info[] = {
	{
		.code   = V4L2_MBUS_FMT_SBGGR10_1X10,
		.colorspace = V4L2_COLORSPACE_JPEG,
		.fmt    = 1,
		.order    = 0,
	},
};
static int32_t msm_gc0339_i2c_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	return msm_sensor_i2c_probe(client, id, &gc0339_s_ctrl);
}

static const struct i2c_device_id gc0339_i2c_id[] = {
	{GC0339_SENSOR_NAME, (kernel_ulong_t)&gc0339_s_ctrl},
	{ }
};

static struct i2c_driver gc0339_i2c_driver = {
	.id_table = gc0339_i2c_id,
	.probe  = msm_gc0339_i2c_probe,
	.driver = {
		.name = GC0339_SENSOR_NAME,
	},
};

static struct msm_camera_i2c_client gc0339_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_BYTE_ADDR,
};

int32_t gc0339_match_id(struct msm_sensor_ctrl_t *s_ctrl)
{
	int32_t rc = 0;
	uint16_t chipid = 0;
    pr_info("%s:E", __func__);
	if (!s_ctrl) {
		pr_err("%s:%d failed: %p\n",
			__func__, __LINE__, s_ctrl);
		return -EINVAL;
	}

	s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(
		s_ctrl->sensor_i2c_client,
		0xfc,
		0x10,
		MSM_CAMERA_I2C_BYTE_DATA);

	rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(
			s_ctrl->sensor_i2c_client,
			s_ctrl->sensordata->slave_info->sensor_id_reg_addr,
			&chipid, MSM_CAMERA_I2C_BYTE_DATA);

	CDBG("%s: read id: 0x%x\n", __func__, chipid);

	if (rc < 0) {
		pr_err("%s: %s: read id failed\n", __func__,
			s_ctrl->sensordata->sensor_name);
		return rc;
	}

	if (chipid != s_ctrl->sensordata->slave_info->sensor_id) {
		pr_err("msm_sensor_match_id chip id doesnot match\n");
		return -ENODEV;
	}
	return rc;
}

static struct msm_sensor_fn_t gc0339_sensor_fn_t = {
	.sensor_power_up = msm_sensor_power_up,
	.sensor_power_down = msm_sensor_power_down,
	.sensor_match_id = gc0339_match_id,
	.sensor_config = msm_sensor_config,
};


static struct msm_sensor_ctrl_t gc0339_s_ctrl = {
	.sensor_i2c_client = &gc0339_sensor_i2c_client,
	.power_setting_array.power_setting = gc0339_power_setting,
	.power_setting_array.size = ARRAY_SIZE(gc0339_power_setting),
	.power_setting_array.power_down_setting = gc0339_power_down_setting,
	.power_setting_array.size_down = ARRAY_SIZE(gc0339_power_down_setting),
	.msm_sensor_mutex = &gc0339_mut,
	.sensor_v4l2_subdev_info = gc0339_subdev_info,
	.sensor_v4l2_subdev_info_size = ARRAY_SIZE(gc0339_subdev_info),
	.func_tbl = &gc0339_sensor_fn_t,
};

static const struct of_device_id gc0339_dt_match[] = {
	{.compatible = "shinetech,gc0339", .data = &gc0339_s_ctrl},
	{}
};


MODULE_DEVICE_TABLE(of, gc0339_dt_match);
static struct platform_driver gc0339_platform_driver = {
	.driver = {
	.name = "shinetech,gc0339",
	.owner = THIS_MODULE,
	.of_match_table = gc0339_dt_match,
	},
};

static const char *gc0339Vendor = "Sony";
static const char *gc0339NAME = "gc0339";
static const char *gc0339Size = "1.3M";

static ssize_t sensor_vendor_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;

	sprintf(buf, "%s %s %s\n", gc0339Vendor, gc0339NAME, gc0339Size);
	ret = strlen(buf) + 1;

	return ret;
}

static DEVICE_ATTR(sensor, 0444, sensor_vendor_show, NULL);

static struct kobject *android_gc0339;


static int gc0339_sysfs_init(void)
{
	int ret ;
	pr_info("gc0339:kobject creat and add\n");
	android_gc0339 = kobject_create_and_add("android_camera2", NULL);
	if (android_gc0339 == NULL) {
		pr_info("gc0339_sysfs_init: subsystem_register " \
		"failed\n");
		ret = -ENOMEM;
		return ret ;
	}
	pr_info("gc0339:sysfs_create_file\n");
	ret = sysfs_create_file(android_gc0339, &dev_attr_sensor.attr);
	if (ret) {
		pr_info("gc0339_sysfs_init: sysfs_create_file " \
		"failed\n");
		kobject_del(android_gc0339);
	}

	return 0 ;
}


static int32_t gc0339_platform_probe(struct platform_device *pdev)
{
	int32_t rc = 0;
	const struct of_device_id *match;

	match = of_match_device(gc0339_dt_match, &pdev->dev);
	rc = msm_sensor_platform_probe(pdev, match->data);
	return rc;
}

static int __init gc0339_init_module(void)
{
	int32_t rc = 0;

	rc = platform_driver_probe(&gc0339_platform_driver,
		gc0339_platform_probe);
	if (!rc) {
		gc0339_sysfs_init();
		return rc;
	}
	return rc;
	return i2c_add_driver(&gc0339_i2c_driver);
}

static void __exit gc0339_exit_module(void)
{
	if (gc0339_s_ctrl.pdev) {
	msm_sensor_free_sensor_data(&gc0339_s_ctrl);
	platform_driver_unregister(&gc0339_platform_driver);
	} else
	i2c_del_driver(&gc0339_i2c_driver);
	return;
}

module_init(gc0339_init_module);
module_exit(gc0339_exit_module);
MODULE_DESCRIPTION("gc0339");
MODULE_LICENSE("GPL v2");
