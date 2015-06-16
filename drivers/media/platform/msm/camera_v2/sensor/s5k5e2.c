/* Copyright (c) 2013, The Linux Foundation. All rights reserved.
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
#include "msm_sensor.h"
#include "msm_camera_io_util.h"
#define S5K5E2_SENSOR_NAME "s5k5e2"
DEFINE_MSM_MUTEX(s5k5e2_mut);
static ssize_t s5k5e2_read_version_attr(struct device *dev,struct device_attribute *attr, char *buf);
static DEVICE_ATTR(read_version, 0664, s5k5e2_read_version_attr, NULL);
static ssize_t s5k5e2_read_vendor_attr(struct device *dev,struct device_attribute *attr, char *buf);
static DEVICE_ATTR(read_vendor, 0664, s5k5e2_read_vendor_attr, NULL);
static struct msm_sensor_ctrl_t s5k5e2_s_ctrl;

static struct msm_sensor_power_setting s5k5e2_power_setting[] = {
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_VANA,
		.config_val = GPIO_OUT_LOW,
		.delay = 0,
	},
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_VANA,
		.config_val = GPIO_OUT_HIGH,
		.delay = 0,
	}, 
	{	.seq_type = SENSOR_VREG,
		.seq_val = CAM_VDIG,
		.config_val = 0,
		.delay = 0,
	},
	{	.seq_type = SENSOR_VREG,  ///only USE for i2c pull high 	
		.seq_val = CAM_VIO,
		.config_val = 0,
		.delay = 0,
	},
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_VAF,
		.config_val = GPIO_OUT_LOW,
		.delay = 0,
	},
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_VAF,
		.config_val = GPIO_OUT_HIGH,
		.delay = 0,
	},
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_RESET,
		.config_val = GPIO_OUT_LOW,
		.delay = 0,
	},
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_RESET,
		.config_val = GPIO_OUT_HIGH,
		.delay = 5,
	},
	{
		.seq_type = SENSOR_CLK,
		.seq_val = SENSOR_CAM_MCLK,
		.config_val = 24000000,
		.delay = 5,
	},
	{
		.seq_type = SENSOR_I2C_MUX,
		.seq_val = 0,
		.config_val = 0,
		.delay = 0,
	},
};

static struct v4l2_subdev_info s5k5e2_subdev_info[] = {
	{
		.code   = V4L2_MBUS_FMT_SGRBG10_1X10,
		.colorspace = V4L2_COLORSPACE_JPEG,
		.fmt    = 1,
		.order    = 0,
	},
};

static const struct i2c_device_id s5k5e2_i2c_id[] = {
	{S5K5E2_SENSOR_NAME, (kernel_ulong_t)&s5k5e2_s_ctrl},
	{ }
};

static int32_t msm_s5k5e2_i2c_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	return msm_sensor_i2c_probe(client, id, &s5k5e2_s_ctrl);
}

static struct i2c_driver s5k5e2_i2c_driver = {
	.id_table = s5k5e2_i2c_id,
	.probe  = msm_s5k5e2_i2c_probe,
	.driver = {
		.name = S5K5E2_SENSOR_NAME,
	},
};

static struct msm_camera_i2c_client s5k5e2_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_WORD_ADDR,
};

static const struct of_device_id s5k5e2_dt_match[] = {
	{.compatible = "qcom,s5k5e2", .data = &s5k5e2_s_ctrl},
	{}
};

MODULE_DEVICE_TABLE(of, s5k5e2_dt_match);

static struct platform_driver s5k5e2_platform_driver = {
	.driver = {
		.name = "qcom,s5k5e2",
		.owner = THIS_MODULE,
		.of_match_table = s5k5e2_dt_match,
	},
};

static int32_t s5k5e2_platform_probe(struct platform_device *pdev)
{
	int32_t rc = 0,ret;
	const struct of_device_id *match;
	pr_info("%s:%d\n", __func__, __LINE__);	
	match = of_match_device(s5k5e2_dt_match, &pdev->dev);
	rc = msm_sensor_platform_probe(pdev, match->data);
	pr_info("%s:%d\n", __func__, __LINE__);
	ret = device_create_file(&(pdev->dev), &dev_attr_read_version);
	if (0 != ret)
		pr_err("%s:%d creating attribute failed \n", __func__,__LINE__);
	ret = device_create_file(&(pdev->dev), &dev_attr_read_vendor);
	if (0 != ret)
		pr_err("%s:%d creating attribute failed \n", __func__,__LINE__);
	return rc;
}

static int __init s5k5e2_init_module(void)
{
	int32_t rc = 0;
	pr_info("%s:%d\n", __func__, __LINE__);
	rc = platform_driver_probe(&s5k5e2_platform_driver,
		s5k5e2_platform_probe);
	if (!rc)
		return rc;
	pr_err("%s:%d rc %d\n", __func__, __LINE__, rc);
	return i2c_add_driver(&s5k5e2_i2c_driver);
}

static void __exit s5k5e2_exit_module(void)
{
	pr_info("%s:%d\n", __func__, __LINE__);
	if (s5k5e2_s_ctrl.pdev) {
		msm_sensor_free_sensor_data(&s5k5e2_s_ctrl);
		platform_driver_unregister(&s5k5e2_platform_driver);
	} else
		i2c_del_driver(&s5k5e2_i2c_driver);
	return;
}

static struct msm_sensor_ctrl_t s5k5e2_s_ctrl = {
	.sensor_i2c_client = &s5k5e2_sensor_i2c_client,
	.power_setting_array.power_setting = s5k5e2_power_setting,
	.power_setting_array.size = ARRAY_SIZE(s5k5e2_power_setting),
	.msm_sensor_mutex = &s5k5e2_mut,
	.sensor_v4l2_subdev_info = s5k5e2_subdev_info,
	.sensor_v4l2_subdev_info_size = ARRAY_SIZE(s5k5e2_subdev_info),
};

module_init(s5k5e2_init_module);
module_exit(s5k5e2_exit_module);
MODULE_DESCRIPTION("s5k5e2");
MODULE_LICENSE("GPL v2");

static ssize_t s5k5e2_read_version_attr(struct device *dev,struct device_attribute *attr, char *buf)
{
	struct msm_sensor_ctrl_t *s_ctrl;
	int32_t power_flag = 0;
	int32_t rc = 0;
	uint16_t version = 0;

	pr_err("ov7695_read_id device addr= %x\n",(uint32_t)dev);
	pr_err("get driver addr =%x \n",(uint32_t)dev_get_drvdata(dev));
	s_ctrl = &s5k5e2_s_ctrl;

	if(s_ctrl->sensor_state != MSM_SENSOR_POWER_UP)
	{
		s_ctrl->func_tbl->sensor_power_up(s_ctrl);
		power_flag =1;
	}

	rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(
			s_ctrl->sensor_i2c_client,
			0x0002,
			&version, MSM_CAMERA_I2C_WORD_DATA);
	if (rc < 0) {
		pr_err("%s: %s: read id failed\n", __func__,
			s_ctrl->sensordata->sensor_name);
	}

	if(power_flag == 1)
	{
		s_ctrl->func_tbl->sensor_power_down(s_ctrl);
	}

	 return sprintf(buf, "%x\n", version);
}

static struct msm_camera_i2c_reg_conf s5k5e2_read_eeprom[] = {
	{0x0A00 ,0x04},
	{0x0A02 ,0x02},
	{0x0A00 ,0x01},
};

static ssize_t s5k5e2_read_vendor_attr(struct device *dev,struct device_attribute *attr, char *buf)
{
	struct msm_sensor_ctrl_t *s_ctrl;
	int32_t power_flag = 0;
	int32_t rc = 0;
	uint16_t version = 0;


	s_ctrl = &s5k5e2_s_ctrl;

	if(s_ctrl->sensor_state != MSM_SENSOR_POWER_UP)
	{
		s_ctrl->func_tbl->sensor_power_up(s_ctrl);
		power_flag =1;
	}
	rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write_conf_tbl(
							s_ctrl->sensor_i2c_client,
							s5k5e2_read_eeprom,
							sizeof(s5k5e2_read_eeprom), MSM_CAMERA_I2C_BYTE_DATA);
	msleep(5);
	rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(
					s_ctrl->sensor_i2c_client,
					0x0A06,
					&version, MSM_CAMERA_I2C_BYTE_DATA);
	pr_err("%s: %x: version\n", __func__,		version		);
	if(power_flag == 1)
	{
		s_ctrl->func_tbl->sensor_power_down(s_ctrl);
	}

	 return sprintf(buf, "%x\n", version);
}
