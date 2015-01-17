/* drivers/media/platform/msm/camera_v2/sensor/sony_camera_v4l2.c
 *
 * Copyright (C) 2012 Sony Mobile Communications AB.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef DEBUG
#define DEBUG
#endif

#define ENABLE_LOGE
#define ENABLE_LOGI
/*#define ENABLE_LOGD*/

#include <linux/regulator/consumer.h>
#include <mach/gpiomux.h>
#include <linux/stat.h>
#include "camera.h"
#include "msm_sensor.h"
#include "msm_camera_i2c.h"
#include "msm_cci.h"
#include "msm_camera_i2c_mux.h"
#include "msm_camera_io_util.h"
#include "sony_camera_v4l2.h"

#ifdef ENABLE_LOGE
#define LOGE(f, a...)	dev_err(camera_device, "%s: " f, __func__, ##a)
#else
#define LOGE(f, a...)
#endif

#ifdef ENABLE_LOGD
#define LOGD(f, a...)	dev_dbg(camera_device, "%s: " f, __func__, ##a)
#else
#define LOGD(f, a...)
#endif

#ifdef ENABLE_LOGI
#define LOGI(f, a...)	dev_info(camera_device, "%s: " f, __func__, ##a)
#else
#define LOGI(f, a...)
#endif

#define I2C_MAX_DATA_LEN	256
#define SENSOR_NAME_LEN		8
#define EEPROM_MAX_DATA_LEN	2048
#define SENSOR_MCLK_DEFAULT	8000000

#define SENSOR_ID_MT9M114	0x2481
#define SENSOR_ID_MT9V115	0x2284
#define MODULE_STW01BM0		"STW01BM0"
#define MODULE_APT01BM0		"APT01BM0"
#define MODULE_APT00YP1		"APT00YP1"
#define MODULE_STW00YP1		"STW00YP1"
#define CAMERA_DEV_NAME		"sony_camera_%d"
#define CAPS_MAX_STR_LEN	32

struct sony_camera_data {
	bool				probe_done;
	bool				gpio_requested;
	struct device			*d;
	struct msm_sensor_ctrl_t	s_ctrl;
	uint8_t				buf[I2C_MAX_DATA_LEN];
	uint8_t				eeprom[EEPROM_MAX_DATA_LEN];
	uint16_t			eeprom_len;
	const struct sony_camera_module	*module;
	struct regulator		*cam_vio;
	struct regulator		*cam_vana;
	struct regulator		*cam_vdig;
	struct regulator		*cam_vaf;
	struct clk			*clk_handle[2];
	struct device			info_dev;
};

struct camera_dev_info {
	uint32_t			mount_angle;
	uint32_t			sensor_rotation;
	uint32_t			sensor_facing;
	uint32_t			pixel_number_w;
	uint32_t			pixel_number_h;
	char				diagonal_len[CAPS_MAX_STR_LEN];
	char				unit_cell_size[CAPS_MAX_STR_LEN];
	char				min_f_number[CAPS_MAX_STR_LEN];
	char				max_f_number[CAPS_MAX_STR_LEN];
	uint32_t			has_focus_actuator;
	uint32_t			has_3a;
	uint32_t			eeprom_size;
	uint8_t				eeprom[EEPROM_MAX_DATA_LEN];
	uint32_t			pll_num;
	uint32_t			pll[MAX_PLL_NUM];
};

static struct sony_camera_info *camera_info;
static struct sony_camera_data camera_data[];
static struct device *camera_device;
static DEFINE_MUTEX(sensor_mut);
static uint16_t sensor_num;
static struct msm_cam_clk_info cam_clk_info[] = {
	{"cam_src_clk", 0},
	{"cam_clk", -1},
};

static int32_t sony_util_get_context(struct msm_sensor_ctrl_t *s_ctrl)
{
	uint16_t i;
	char sensor_name[CAPS_MAX_STR_LEN];

	memset(sensor_name, 0, sizeof(sensor_name));
	for (i = 0; i < sensor_num; i++) {
		snprintf(sensor_name, sizeof(sensor_name), CAMERA_DEV_NAME, i);
		if (!strncmp(s_ctrl->sensordata->sensor_name,
				sensor_name, sizeof(sensor_name)))
			break;
	}

	if (camera_data[i].d)
		camera_device = camera_data[i].d;

	return i;
}

static int sony_util_camera_info_init(struct platform_device *pdev, uint16_t id)
{
	int rc = 0;
	int count = 0;
	uint16_t i = 0;
	uint16_t j = 0;
	uint32_t val_u32[4] = {0};
	struct device_node *of_node = pdev->dev.of_node;
	struct device_node *of_node_power_sequence = NULL;
	struct device_node *of_node_modules = NULL;
	struct device_node *of_node_modules_power_off = NULL;
	struct device_node *of_node_modules_power_on = NULL;
	const int8_t *power_order_name = NULL;

	rc = of_property_read_u32(of_node, "sony,i2c_addr", &val_u32[0]);
	if (rc < 0) {
		LOGE("%s failed %d\n", __func__, __LINE__);
		goto fail;
	}
	camera_info[id].i2c_addr = val_u32[0];

	rc = of_property_read_u32(of_node, "sony,eeprom_addr", &val_u32[0]);
	if (rc < 0) {
		LOGE("%s failed %d\n", __func__, __LINE__);
		goto fail;
	}
	camera_info[id].eeprom_addr = val_u32[0];

	rc = of_property_read_u32(of_node, "sony,eeprom_type", &val_u32[0]);
	if (rc < 0) {
		LOGE("%s failed %d\n", __func__, __LINE__);
		goto fail;
	}
	camera_info[id].eeprom_type = val_u32[0];

	rc = of_property_read_u32(of_node, "sony,eeprom_max_len", &val_u32[0]);
	if (rc < 0) {
		LOGE("%s failed %d\n", __func__, __LINE__);
		goto fail;
	}
	camera_info[id].eeprom_max_len = val_u32[0];

	rc = of_property_read_u32(of_node, "sony,gpio_af", &val_u32[0]);
	if (rc < 0) {
		LOGE("%s failed %d\n", __func__, __LINE__);
		goto fail;
	}
	camera_info[id].gpio_af = val_u32[0];

	rc = of_property_read_u32(of_node, "sony,subdev_code", &val_u32[0]);
	if (rc < 0) {
		LOGE("%s failed %d\n", __func__, __LINE__);
		goto fail;
	}
	camera_info[id].subdev_code = val_u32[0];

	of_node_power_sequence = of_find_node_by_name(of_node,
					"sony,camera_modules");
	if (!of_node_power_sequence) {
		LOGE("%s failed %d\n", __func__, __LINE__);
		rc = -EFAULT;
		goto fail;
	}

	count = of_property_count_strings(of_node_power_sequence,
					"module_names");
	if (count < 0) {
		LOGE("%s failed %d\n", __func__, __LINE__);
		rc = -EFAULT;
		goto fail;
	}
	camera_info[id].modules_num = count;

	camera_info[id].modules = kzalloc(
		sizeof(struct sony_camera_module) * count, GFP_KERNEL);
	if (!camera_info[id].modules) {
		LOGE("%s failed %d\n", __func__, __LINE__);
		rc = -ENOMEM;
		goto fail;
	}

	for (i = 0; i < camera_info[id].modules_num; i++) {
		rc = of_property_read_string_index(of_node_power_sequence,
			"module_names", i,
			(const char **)(&camera_info[id].modules[i].name));
		LOGD("%s name[%d] = %s\n", __func__, i,
			camera_info[id].modules[i].name);
		if (rc < 0) {
			LOGE("%s failed %d\n", __func__, __LINE__);
			goto fail;
		}
	}

	for (i = 0; i < camera_info[id].modules_num; i++) {
		of_node_modules = of_find_node_by_name(of_node_power_sequence,
					camera_info[id].modules[i].name);
		if (!of_node_modules) {
			LOGE("%s failed %d\n", __func__, __LINE__);
			rc = -EFAULT;
			goto fail;
		}

		rc = of_property_read_u32(of_node_modules,
				"mount_angle",
				&camera_info[id].modules[i].mount_angle);

		if (rc < 0) {
			LOGE("%s failed %d\n", __func__, __LINE__);
			goto fail;
		}

		rc = of_property_read_u32(of_node_modules,
				"sensor_rotation",
				&camera_info[id].modules[i].sensor_rotation);

		if (rc < 0) {
			LOGE("%s failed %d\n", __func__, __LINE__);
			goto fail;
		}

		rc = of_property_read_u32(of_node_modules,
				"sensor_facing",
				&camera_info[id].modules[i].sensor_facing);

		if (rc < 0) {
			LOGE("%s failed %d\n", __func__, __LINE__);
			goto fail;
		}

		rc = of_property_read_u32(of_node_modules,
				"pixel_number_w",
				&camera_info[id].modules[i].pixel_number_w);

		if (rc < 0) {
			LOGE("%s failed %d\n", __func__, __LINE__);
			goto fail;
		}

		rc = of_property_read_u32(of_node_modules,
				"pixel_number_h",
				&camera_info[id].modules[i].pixel_number_h);

		if (rc < 0) {
			LOGE("%s failed %d\n", __func__, __LINE__);
			goto fail;
		}

		rc = of_property_read_string(of_node_modules,
				"diagonal_len",
				(const char **)(
				&camera_info[id].modules[i].diagonal_len));

		if (rc < 0) {
			LOGE("%s failed %d\n", __func__, __LINE__);
			goto fail;
		}

		rc = of_property_read_string(of_node_modules,
				"unit_cell_size",
				(const char **)(
				&camera_info[id].modules[i].unit_cell_size));

		if (rc < 0) {
			LOGE("%s failed %d\n", __func__, __LINE__);
			goto fail;
		}

		rc = of_property_read_string(of_node_modules,
				"min_f_number",
				(const char **)(
				&camera_info[id].modules[i].min_f_number));

		if (rc < 0) {
			LOGE("%s failed %d\n", __func__, __LINE__);
			goto fail;
		}

		rc = of_property_read_string(of_node_modules,
				"max_f_number",
				(const char **)(
				&camera_info[id].modules[i].max_f_number));

		if (rc < 0) {
			LOGE("%s failed %d\n", __func__, __LINE__);
			goto fail;
		}

		rc = of_property_read_u32(of_node_modules,
				"has_focus_actuator",
				&camera_info[id].modules[i].has_focus_actuator);

		if (rc < 0) {
			LOGE("%s failed %d\n", __func__, __LINE__);
			goto fail;
		}

		rc = of_property_read_u32(of_node_modules,
				"has_3a",
				&camera_info[id].modules[i].has_3a);

		if (rc < 0) {
			LOGE("%s failed %d\n", __func__, __LINE__);
			goto fail;
		}

		rc = of_property_read_u32(of_node_modules,
				"pll_num",
				&camera_info[id].modules[i].pll_num);
		if (rc < 0) {
			LOGE("%s failed %d\n", __func__, __LINE__);
			goto fail;
		}

		rc = of_property_read_u32_array(of_node_modules,
				"pll",
				camera_info[id].modules[i].pll,
				camera_info[id].modules[i].pll_num);
		if (rc < 0) {
			LOGE("%s failed %d\n", __func__, __LINE__);
			goto fail;
		}

		of_node_modules_power_off = of_find_node_by_name(
						of_node_modules,
						"power_off");
		if (!of_node_modules_power_off) {
			LOGE("%s failed %d\n", __func__, __LINE__);
			rc = -EFAULT;
			goto fail;
		}

		count = of_property_count_strings(of_node_modules_power_off,
						"commands");
		if (count < 0) {
			LOGE("%s failed power off commands 0\n", __func__);
			rc = -EFAULT;
			goto fail;
		}
		camera_info[id].modules[i].seq_off = kzalloc(
			sizeof(struct sony_camera_seq) * count, GFP_KERNEL);
		if (!camera_info[id].modules[i].seq_off) {
			LOGE("%s failed %d\n", __func__, __LINE__);
			rc = -ENOMEM;
			goto fail;
		}

		for (j = 0; j < count; j++) {
			rc = of_property_read_string_index(
					of_node_modules_power_off,
					"commands", j,
					(const char **)(&power_order_name));
			if (rc < 0) {
				LOGE("%s failed %d\n", __func__, __LINE__);
				goto fail;
			}

			rc = of_property_read_u32_array(
				of_node_modules_power_off, power_order_name,
				&val_u32[0], 4);
			if (rc < 0) {
				LOGE("%s failed %d\n", __func__, __LINE__);
				goto fail;
			}
			camera_info[id].modules[i].seq_off[j].cmd =
								val_u32[0];
			camera_info[id].modules[i].seq_off[j].val1 =
								val_u32[1];
			camera_info[id].modules[i].seq_off[j].val2 =
								val_u32[2];
			camera_info[id].modules[i].seq_off[j].wait =
								val_u32[3];
		}

		of_node_modules_power_on = of_find_node_by_name(of_node_modules,
						"power_on");
		if (!of_node_modules_power_on) {
			LOGE("%s failed %d\n", __func__, __LINE__);
			rc = -EFAULT;
			goto fail;
		}

		count = of_property_count_strings(of_node_modules_power_on,
						"commands");
		if (count < 0) {
			LOGE("%s failed power on commands 0\n", __func__);
			rc = -EFAULT;
			goto fail;
		}

		camera_info[id].modules[i].seq_on = kzalloc(
			sizeof(struct sony_camera_seq) * count, GFP_KERNEL);
		if (!camera_info[id].modules[i].seq_on) {
			LOGE("%s failed %d\n", __func__, __LINE__);
			rc = -ENOMEM;
			goto fail;
		}

		for (j = 0; j < count; j++) {
			rc = of_property_read_string_index(
					of_node_modules_power_on,
					"commands", j,
					(const char **)(&power_order_name));
			if (rc < 0) {
				LOGE("%s failed %d\n", __func__, __LINE__);
				goto fail;
			}

			rc = of_property_read_u32_array(
				of_node_modules_power_on, power_order_name,
				&val_u32[0], 4);
			if (rc < 0) {
				LOGE("%s failed %d\n", __func__, __LINE__);
				goto fail;
			}
			camera_info[id].modules[i].seq_on[j].cmd =
								val_u32[0];
			camera_info[id].modules[i].seq_on[j].val1 =
								val_u32[1];
			camera_info[id].modules[i].seq_on[j].val2 =
								val_u32[2];
			camera_info[id].modules[i].seq_on[j].wait =
								val_u32[3];
		}
	}

	rc = of_property_read_string(of_node_power_sequence,
				"default_module_name",
				&camera_info[id].default_module_name);
	if (rc < 0) {
		LOGE("%s failed %d\n", __func__, __LINE__);
		goto fail;
	}

	return 0;
fail:
	if (camera_info[id].modules) {
		for (i = 0; i < camera_info[id].modules_num; i++) {
			kfree(camera_info[id].modules[i].seq_on);
			kfree(camera_info[id].modules[i].seq_off);
		}
		kfree(camera_info[id].modules);
	}
	memset(&(camera_info[id]), 0, sizeof(struct sony_camera_info));

	return rc;
}

static int sony_util_camera_info_deinit(uint16_t id)
{
	uint16_t i = 0;

	if (camera_info[id].modules) {
		for (i = 0; i < camera_info[id].modules_num; i++) {
			kfree(camera_info[id].modules[i].seq_on);
			kfree(camera_info[id].modules[i].seq_off);
		}
		kfree(camera_info[id].modules);
	}
	memset(&(camera_info[id]), 0, sizeof(struct sony_camera_info));

	return 0;
}

static int sony_util_gpio_init(struct msm_sensor_ctrl_t *s_ctrl)
{
	int rc = 0;
	uint16_t id = sony_util_get_context(s_ctrl);

	if (!camera_data[id].gpio_requested) {
		rc = msm_camera_request_gpio_table(
			s_ctrl->sensordata->power_info
				.gpio_conf->cam_gpio_req_tbl,
			s_ctrl->sensordata->power_info
				.gpio_conf->cam_gpio_req_tbl_size,
			1);
		if (rc == 0)
			camera_data[id].gpio_requested = true;
	}

	return rc;
}

static int sony_util_gpio_deinit(struct msm_sensor_ctrl_t *s_ctrl)
{
	int rc = 0;
	uint16_t id = sony_util_get_context(s_ctrl);

	if (camera_data[id].gpio_requested) {
		rc = msm_camera_request_gpio_table(
			s_ctrl->sensordata->power_info
				.gpio_conf->cam_gpio_req_tbl,
			s_ctrl->sensordata->power_info
				.gpio_conf->cam_gpio_req_tbl_size,
			0);
		camera_data[id].gpio_requested = false;
	}

	return rc;
}

static int sony_util_gpio_set(struct msm_sensor_ctrl_t *s_ctrl,
		int gpio_pin, int value)
{
	int rc = 0;
	uint16_t id = sony_util_get_context(s_ctrl);

	if (camera_data[id].gpio_requested)
		gpio_set_value_cansleep(gpio_pin, value);
	else
		rc = -EPERM;

	return rc;
}

static int sony_util_vreg_set(struct msm_sensor_ctrl_t *s_ctrl,
	struct sony_camera_data *data, enum sony_camera_cmd cmd, int level,
	int op_mode)
{
	int rc = 0;
	struct regulator *vreg;
	struct device *dev = &s_ctrl->pdev->dev;

	if (cmd == SONY_CAM_VDIG) {
		if (data->cam_vdig) {
			vreg = data->cam_vdig;
		} else {
			vreg = regulator_get(dev, "cam_vdig");
			if (IS_ERR(vreg)) {
				LOGE("could not get cam_vdig, vreg = %ld\n",
					PTR_ERR(vreg));
				rc = -ENODEV;
				goto exit;
			}
			data->cam_vdig = vreg;
		}
	} else if (cmd == SONY_CAM_VIO) {
		if (data->cam_vio) {
			vreg = data->cam_vio;
		} else {
			vreg = regulator_get(dev, "cam_vio");
			if (IS_ERR(vreg)) {
				LOGE("could not get cam_vio, vreg = %ld\n",
					PTR_ERR(vreg));
				rc = -ENODEV;
				goto exit;
			}
			data->cam_vio = vreg;
		}
	} else if (cmd == SONY_CAM_VANA) {
		if (data->cam_vana) {
			vreg = data->cam_vana;
		} else {
			vreg = regulator_get(dev, "cam_vana");
			if (IS_ERR(vreg)) {
				LOGE("could not get cam_vana, vreg = %ld\n",
					PTR_ERR(vreg));
				rc = -ENODEV;
				goto exit;
			}
			data->cam_vana = vreg;
		}
	} else if (cmd == SONY_CAM_VAF) {
		if (data->cam_vaf) {
			vreg = data->cam_vaf;
		} else {
			vreg = regulator_get(dev, "cam_vaf");
			if (IS_ERR(vreg)) {
				LOGE("could not get cam_vaf, vreg = %ld\n",
					PTR_ERR(vreg));
				rc = -ENODEV;
				goto exit;
			}
			data->cam_vaf = vreg;
		}
	} else {
		rc = -EINVAL;
		LOGE("invalid resource\n");
		goto exit;
	}

	level *= 1000;
	if (level >= 0) {
		if (level > 0) {
			rc = regulator_set_voltage(vreg, level, level);
			if (rc < 0)
				goto set_voltage_fail;
		}
		if (op_mode > 0) {
			rc = regulator_set_optimum_mode(vreg, op_mode);
			if (rc < 0)
				goto set_voltage_fail;
		}
		rc = regulator_enable(vreg);
		if (rc < 0)
			goto enable_fail;
	} else {
		if (op_mode == 0)
			(void)regulator_set_optimum_mode(vreg, 0);
		(void)regulator_disable(vreg);
		regulator_put(vreg);
	}
	goto exit;

enable_fail:
	(void)regulator_set_optimum_mode(vreg, 0);

set_voltage_fail:
	regulator_put(vreg);

exit:
	if (rc < 0 || level < 0) {
		if (vreg == data->cam_vdig)
			data->cam_vdig = NULL;
		else if (vreg == data->cam_vio)
			data->cam_vio = NULL;
		else if (vreg == data->cam_vana)
			data->cam_vana = NULL;
		else if (vreg == data->cam_vaf)
			data->cam_vaf = NULL;
	}

	if (rc < 0)
		LOGE("error happened (%d)\n", rc);
	return rc;
}

static int sony_util_mclk_set(struct msm_sensor_ctrl_t *s_ctrl, int value)
{
	int rc;
	struct device *dev = &s_ctrl->pdev->dev;
	uint16_t id = sony_util_get_context(s_ctrl);

	if (value >= 0) {
		if (value == 0)
			cam_clk_info[0].clk_rate = SENSOR_MCLK_DEFAULT;
		else
			cam_clk_info[0].clk_rate = (long)value;

		rc = msm_cam_clk_enable(
			dev,
			(struct msm_cam_clk_info *)cam_clk_info,
			(struct clk **)&camera_data[id].clk_handle[0],
			ARRAY_SIZE(cam_clk_info), 1);
	} else {
		rc = msm_cam_clk_enable(
			dev,
			(struct msm_cam_clk_info *)cam_clk_info,
			(struct clk **)&camera_data[id].clk_handle[0],
			ARRAY_SIZE(cam_clk_info), 0);
	}

	if (rc < 0)
		LOGE("error happened (%d)\n", rc);

	return rc;
}

static int sony_util_i2c_mux_enable(struct msm_camera_i2c_conf *i2c_conf)
{
	struct v4l2_subdev *i2c_mux_sd =
		dev_get_drvdata(&i2c_conf->mux_dev->dev);

	v4l2_subdev_call(i2c_mux_sd, core, ioctl,
		VIDIOC_MSM_I2C_MUX_INIT, NULL);
	v4l2_subdev_call(i2c_mux_sd, core, ioctl,
		VIDIOC_MSM_I2C_MUX_CFG, (void *)&i2c_conf->i2c_mux_mode);
	return 0;
}

static int sony_util_i2c_mux_disable(struct msm_camera_i2c_conf *i2c_conf)
{
	struct v4l2_subdev *i2c_mux_sd =
		dev_get_drvdata(&i2c_conf->mux_dev->dev);

	v4l2_subdev_call(i2c_mux_sd, core, ioctl,
				VIDIOC_MSM_I2C_MUX_RELEASE, NULL);
	return 0;
}

static int sony_util_cci_init(struct msm_camera_i2c_client *client)
{
	return msm_sensor_cci_i2c_util(client, MSM_CCI_INIT);
}

static int sony_util_cci_deinit(struct msm_camera_i2c_client *client)
{
	return msm_sensor_cci_i2c_util(client, MSM_CCI_RELEASE);
}

static int sony_util_cam_i2c_read(struct msm_sensor_ctrl_t *s_ctrl,
			uint8_t slave_addr, uint32_t addr,
			uint8_t type, uint16_t len, uint8_t *data)
{
	int rc = 0;
	uint16_t i = 0;
	uint16_t read_addr = 0x00;
	uint16_t read_data = 0x00;
	uint16_t orig_sid;
	uint16_t orig_addr_type;

	orig_sid = s_ctrl->sensor_i2c_client->cci_client->sid;
	orig_addr_type = s_ctrl->sensor_i2c_client->addr_type;

	s_ctrl->sensor_i2c_client->addr_type = type;
	s_ctrl->sensor_i2c_client->cci_client->sid = slave_addr >> 1;

	for (i = 0; i < len; i++) {
		read_addr = addr + i;
		rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(
				s_ctrl->sensor_i2c_client,
				read_addr,
				&read_data,
				MSM_CAMERA_I2C_BYTE_DATA);
		if (rc < 0) {
			LOGE("slave0x%04x,addr0x%04x,type0x%02x,len0x%02x\n",
				slave_addr, addr, type, len);
			LOGE("i2c read failed(%d)\n", rc);
			rc = -EIO;
			goto exit;
		}
		data[i] = (uint8_t)(read_data & 0xFF);
	}

exit:
	s_ctrl->sensor_i2c_client->cci_client->sid = orig_sid;
	s_ctrl->sensor_i2c_client->addr_type = orig_addr_type;

	return rc;
}

static int sony_util_cam_i2c_write(struct msm_sensor_ctrl_t *s_ctrl,
	uint8_t slave_addr, uint32_t addr, uint8_t type,
	uint16_t len, uint8_t *data)
{
	int rc = 0;
	uint16_t i = 0;
	uint16_t write_addr = 0x00;
	uint16_t write_data = 0x00;
	uint16_t orig_sid;
	uint16_t orig_addr_type;

	orig_sid = s_ctrl->sensor_i2c_client->cci_client->sid;
	orig_addr_type = s_ctrl->sensor_i2c_client->addr_type;

	s_ctrl->sensor_i2c_client->addr_type = type;
	s_ctrl->sensor_i2c_client->cci_client->sid = slave_addr >> 1;

	for (i = 0; i < len; i++) {
		write_data = data[i];
		write_addr = addr + i;
		rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(
				s_ctrl->sensor_i2c_client,
				write_addr,
				write_data,
				MSM_CAMERA_I2C_BYTE_DATA);
		if (rc < 0) {
			LOGE("slave0x%04x,addr0x%04x,type0x%02x,len0x%02x,data",
				slave_addr, addr, type, len);
			for (i = 0; i < len; i++)
				LOGE("0x%02x ", *(data + i));
			LOGE("i2c write failed(%d)\n", rc);
			rc = -EIO;
			goto exit;
		}
	}

exit:
	s_ctrl->sensor_i2c_client->cci_client->sid = orig_sid;
	s_ctrl->sensor_i2c_client->addr_type = orig_addr_type;

	return rc;
}

static int sony_util_power_ctrl(struct msm_sensor_ctrl_t *s_ctrl,
		struct sony_camera_data *data, bool on)
{
	int rc = 0;
	uint16_t id = sony_util_get_context(s_ctrl);
	const struct sony_camera_module *mod = data->probe_done ?
		data->module : camera_info[id].modules;
	const struct sony_camera_seq *seq = on ? mod->seq_on : mod->seq_off;

	while (seq->cmd != EXIT) {
		uint8_t iodt = 0x00;
		switch (seq->cmd) {
		case SONY_GPIO_RESET:
			rc = sony_util_gpio_set(s_ctrl,
				s_ctrl->sensordata
					->power_info.gpio_conf
						->cam_gpio_req_tbl[1].gpio,
				seq->val1);
			break;
		case SONY_GPIO_AF:
			if (camera_info[id].gpio_af <= 0) {
				rc = -EPERM;
				break;
			}

			rc = sony_util_gpio_set(s_ctrl,
				camera_info[id].gpio_af, seq->val1);
			break;
		case SONY_CAM_VDIG:
		case SONY_CAM_VIO:
		case SONY_CAM_VANA:
		case SONY_CAM_VAF:
			rc = sony_util_vreg_set(s_ctrl,
				data, seq->cmd, seq->val1, seq->val2);
			break;
		case SONY_CAM_CLK:
			rc = sony_util_mclk_set(s_ctrl, seq->val1);
			break;
		case SONY_I2C_WRITE:
			rc = sony_util_cam_i2c_write(s_ctrl,
				camera_info[id].i2c_addr,
				seq->val1,
				MSM_CAMERA_I2C_WORD_ADDR,
				1,
				&iodt);
			break;
		default:
			goto exit;
		}
		mdelay(seq->wait);
		if (rc < 0 && on)
			goto exit;
		seq++;
	}
exit:
	return rc;
}

static ssize_t sony_camera_info_read(struct device *ldev,
		struct device_attribute *attr, char *buf)
{
	char sensor_name[CAPS_MAX_STR_LEN];
	int id = 0;
	uint16_t info_len = 0;
	struct camera_dev_info *info = (struct camera_dev_info *)buf;

	memset(sensor_name, 0, sizeof(sensor_name));
	for (id = 0; id < sensor_num; id++) {
		snprintf(sensor_name, sizeof(sensor_name), CAMERA_DEV_NAME, id);
		if (!strncmp(ldev->kobj.name,
			sensor_name, sizeof(sensor_name))) {
			info->mount_angle =
				camera_data[id].module->mount_angle;
			info->sensor_rotation =
				camera_data[id].module->sensor_rotation;
			info->sensor_facing =
				camera_data[id].module->sensor_facing;
			info->pixel_number_w =
				camera_data[id].module->pixel_number_w;
			info->pixel_number_h =
				camera_data[id].module->pixel_number_h;
			memset(info->diagonal_len, 0,
				sizeof(info->diagonal_len));
			strlcpy(info->diagonal_len,
				camera_data[id].module->diagonal_len,
				sizeof(info->diagonal_len));
			memset(info->unit_cell_size, 0,
				sizeof(info->unit_cell_size));
			strlcpy(info->unit_cell_size,
				camera_data[id].module->unit_cell_size,
				sizeof(info->unit_cell_size));
			memset(info->min_f_number, 0,
				sizeof(info->min_f_number));
			strlcpy(info->min_f_number,
				camera_data[id].module->min_f_number,
				sizeof(info->min_f_number));
			memset(info->max_f_number, 0,
				sizeof(info->max_f_number));
			strlcpy(info->max_f_number,
				camera_data[id].module->max_f_number,
				sizeof(info->max_f_number));
			info->has_focus_actuator =
				camera_data[id].module->has_focus_actuator;
			info->has_3a =
				camera_data[id].module->has_3a;
			info->pll_num =
				camera_data[id].module->pll_num;
			memset(info->pll, 0, sizeof(info->pll));
			memcpy(info->pll, camera_data[id].module->pll,
				sizeof(info->pll));
			memset(info->eeprom, 0, sizeof(info->eeprom));
			memcpy(info->eeprom, camera_data[id].eeprom,
				camera_data[id].eeprom_len);
			info->eeprom_size = camera_data[id].eeprom_len;
			info_len = sizeof(struct camera_dev_info);
			break;
		}
	}
	return info_len;
}

static struct device_attribute sony_camera_info_attributes[] = {
	__ATTR(info, S_IRUSR | S_IRGRP, sony_camera_info_read, NULL),
	__ATTR(info, S_IRUSR | S_IRGRP, sony_camera_info_read, NULL),
};

static int sony_camera_info_init(int id)
{
	int rc = 0;

	rc = dev_set_name(&camera_data[id].info_dev,
		CAMERA_DEV_NAME, id);
	if (rc < 0) {
		LOGE("%s failed %d\n", __func__, __LINE__);
		goto reg_fail;
	}
	rc = device_register(&camera_data[id].info_dev);
	if (rc < 0) {
		LOGE("%s failed %d\n", __func__, __LINE__);
		goto reg_fail;
	}

	rc = device_create_file(&camera_data[id].info_dev,
		&sony_camera_info_attributes[id]);
	if (rc < 0) {
		LOGE("%s failed %d\n", __func__, __LINE__);
		rc = -ENODEV;
		goto create_fail;
	}
	return 0;

create_fail:
	device_unregister(&camera_data[id].info_dev);
reg_fail:
	return rc;
}

static void sony_camera_info_deinit(void)
{
	int id = 0;

	for (id = 0; id < sensor_num; id++) {
		device_remove_file(&camera_data[id].info_dev,
			&sony_camera_info_attributes[id]);
		device_unregister(&camera_data[id].info_dev);
	}
}

static int sony_eeprom_load(struct msm_sensor_ctrl_t *s_ctrl)
{
	int rc = 0;
	uint16_t i;
	uint16_t len;
	uint16_t id = sony_util_get_context(s_ctrl);
	uint8_t *d = camera_data[id].eeprom;

	LOGI("load eeprom\n");

	/* load eeprom */
	if (camera_info[id].eeprom_type == 0) {
		for (i = 0; i < camera_info[id].eeprom_max_len;
				i += I2C_MAX_DATA_LEN) {
			uint8_t slave_addr = camera_info[id].eeprom_addr
						+ ((i & 0x0700) >> 7);
			uint32_t offset = i & 0x00FF;

			rc = sony_util_cam_i2c_read(
				&camera_data[id].s_ctrl,
				slave_addr,
				offset, MSM_CAMERA_I2C_BYTE_ADDR,
				I2C_MAX_DATA_LEN, d + i);
			if (rc < 0) {
				LOGE("eeprom i2c read fail %d\n", rc);
				camera_data[id].eeprom_len = 0;
				goto exit;
			}
		}
		len = i;
	} else {
		len = camera_info[id].eeprom_max_len;

		d[0] = 0x03;
		d[1] = 0x25;
		rc = sony_util_cam_i2c_write(
			&camera_data[id].s_ctrl,
			camera_info[id].eeprom_addr,
			0x0010, MSM_CAMERA_I2C_WORD_ADDR, 2, d);
		if (rc < 0)
			goto exit;

		d[0] = 0x08;
		d[1] = 0x00;
		rc = sony_util_cam_i2c_write(
			&camera_data[id].s_ctrl,
			camera_info[id].eeprom_addr,
			0x0012, MSM_CAMERA_I2C_WORD_ADDR, 2, d);
		if (rc < 0)
			goto exit;

		i = 0;
		do {
			msleep(20);
			i++;
			rc = sony_util_cam_i2c_read(
				 &camera_data[id].s_ctrl,
				 camera_info[id].eeprom_addr,
				 0x0014, MSM_CAMERA_I2C_WORD_ADDR, 2, d);
			if (rc < 0)
				goto exit;
		} while ((d[0] & 0x80) && i < 100);

		if (i >= 100) {
			rc = -ENODEV;
			goto exit;
		}

		d[0] = 0x45;
		d[1] = 0x04;
		rc = sony_util_cam_i2c_write(
			&camera_data[id].s_ctrl,
			camera_info[id].eeprom_addr,
			0x0018, MSM_CAMERA_I2C_WORD_ADDR, 2, d);
		if (rc < 0)
			goto exit;

		i = 0;
		do {
			msleep(20);
			i++;
			rc = sony_util_cam_i2c_read(
				 &camera_data[id].s_ctrl,
				 camera_info[id].eeprom_addr,
				 0x0018, MSM_CAMERA_I2C_WORD_ADDR, 2, d);
			if (rc < 0)
				goto exit;
		} while ((d[1] & 0x40) && i < 100);

		if (i >= 100) {
			rc = -ENODEV;
			goto exit;
		}

		d[0] = 0x05;
		d[1] = 0x20;
		rc = sony_util_cam_i2c_write(
			&camera_data[id].s_ctrl,
			camera_info[id].eeprom_addr,
			0x001A, MSM_CAMERA_I2C_WORD_ADDR, 2, d);
		if (rc < 0)
			goto exit;

		d[0] = 0x05;
		d[1] = 0x64;
		rc = sony_util_cam_i2c_write(
			&camera_data[id].s_ctrl,
			camera_info[id].eeprom_addr,
			0x001A, MSM_CAMERA_I2C_WORD_ADDR, 2, d);
		if (rc < 0)
			goto exit;

		rc = sony_util_cam_i2c_read(&camera_data[id].s_ctrl,
					camera_info[id].eeprom_addr,
					0x0000, MSM_CAMERA_I2C_WORD_ADDR, 2, d);
		if (rc < 0) {
			LOGE("i2c read faile\n");
			goto exit;
		}

		if (((uint16_t)d[0] << 8 | d[1]) == SENSOR_ID_MT9M114) {
			d[0] = 0x00;
			d[1] = 0x55;
			rc = sony_util_cam_i2c_write(
				&camera_data[id].s_ctrl,
				camera_info[id].eeprom_addr,
				0x3052, MSM_CAMERA_I2C_WORD_ADDR, 2, d);
			if (rc < 0)
				goto exit;

			d[0] = 0x00;
			d[1] = 0x10;
			rc = sony_util_cam_i2c_write(
				&camera_data[id].s_ctrl,
				camera_info[id].eeprom_addr,
				0x3050, MSM_CAMERA_I2C_WORD_ADDR, 2, d);
			if (rc < 0)
				goto exit;

			i = 0;
			do {
				msleep(20);
				i++;
				rc = sony_util_cam_i2c_read(
					&camera_data[id].s_ctrl,
					camera_info[id].eeprom_addr,
					0x3050, MSM_CAMERA_I2C_WORD_ADDR, 2, d);
				if (rc < 0)
					goto exit;
			} while (!(d[1] & 0x20) && i < 100);

			if (i >= 100) {
				rc = -ENODEV;
				goto exit;
			}

			rc = sony_util_cam_i2c_read(
				&camera_data[id].s_ctrl,
				camera_info[id].eeprom_addr,
				0x313A, MSM_CAMERA_I2C_WORD_ADDR, 2, d + 8);
			if (rc < 0)
				goto exit;

			rc = sony_util_cam_i2c_read(
				&camera_data[id].s_ctrl,
				camera_info[id].eeprom_addr,
				0x313C, MSM_CAMERA_I2C_WORD_ADDR, 2, d + 10);
			if (rc < 0)
				goto exit;

			if ((d[8] & 0xC0) == 0x00)
				memcpy(d, MODULE_STW01BM0, SENSOR_NAME_LEN);
			else
				memcpy(d, MODULE_APT01BM0, SENSOR_NAME_LEN);
		} else if (((uint16_t)d[0] << 8 | d[1]) == SENSOR_ID_MT9V115) {
			rc = sony_util_cam_i2c_read(
					&camera_data[id].s_ctrl,
					camera_info[id].eeprom_addr,
					0x001A, MSM_CAMERA_I2C_WORD_ADDR, 2, d);
			if (rc < 0)
				goto exit;

			if ((d[0] & 0xF0) == 0xB0)
				memcpy(d, MODULE_APT00YP1, 8);
			else if ((d[0] & 0xF0) == 0xE0 || (d[0] & 0xF0) == 0x00)
				memcpy(d, MODULE_STW00YP1, 8);
			else {
				LOGE("%s ModuleID is unknown\n", __func__);
				memcpy(d, MODULE_APT00YP1, 8);
			}
		} else {
			LOGE("%s Wrong camera module.\n", __func__);
		}
	}


	/* identify sensor module */
	for (i = 1; i < camera_info[id].modules_num; i++) {
		if (!strncmp(camera_info[id].modules[i].name,
				camera_data[id].eeprom,
				SENSOR_NAME_LEN)) {
			camera_data[id].module =
					&camera_info[id].modules[i];
			LOGD("detected sensor module name\n");

			break;
		}
	}

	if (camera_data[id].module)
		camera_data[id].eeprom_len = len;
	else {
		LOGE("Module name not recognized. Force name.\n");

		camera_data[id].module = &camera_info[id].modules[0];

		for (i = 1; i < camera_info[id].modules_num; i++) {
			if (!strncmp(camera_info[id].modules[i].name,
				camera_info[id].default_module_name,
				SENSOR_NAME_LEN)) {
				camera_data[id].module
					= &camera_info[id].modules[i];
				LOGD("detected sensor Force name\n");
				break;
			}
		}

		memcpy(d, camera_info[id].default_module_name, SENSOR_NAME_LEN);
		camera_data[id].eeprom_len = len;
	}

	LOGI("MODULE NAME: len=%d, name=%c%c%c%c%c%c%c%c\n",
			len, d[0], d[1], d[2], d[3],
			d[4], d[5], d[6], d[7]);

exit:
	return rc;
}

static int32_t sony_sensor_match_id(struct msm_sensor_ctrl_t *s_ctrl)
{
	int rc = 0;
	uint16_t id = sony_util_get_context(s_ctrl);

	if (camera_data[id].probe_done == false) {
		rc = sony_eeprom_load(s_ctrl);
		if (rc < 0)
			LOGE("eeprom load fail\n");
		s_ctrl->sensor_i2c_client->cci_client->sid =
			camera_info[id].i2c_addr >> 1;
	}
	return rc;
}

static int sony_sensor_power_up(struct msm_sensor_ctrl_t *s_ctrl)
{
	int rc = 0;
	struct msm_camera_sensor_board_info *data = s_ctrl->sensordata;
	uint16_t id = sony_util_get_context(s_ctrl);

	LOGD("%s: %d\n", __func__, __LINE__);

	rc = sony_util_gpio_init(s_ctrl);
	if (rc < 0) {
		LOGE("%s: gpio_init failed\n", __func__);
		goto exit;
	}

	rc = sony_util_power_ctrl(s_ctrl, &camera_data[id], true);
	if (rc < 0) {
		LOGE("power_up fail\n");
		goto exit;
	}

	rc = sony_util_cci_init(s_ctrl->sensor_i2c_client);
	if (rc < 0) {
		LOGE("%s cci_init failed\n", __func__);
		goto exit;
	}

	if (data->power_info.i2c_conf &&
		data->power_info.i2c_conf->use_i2c_mux)
		sony_util_i2c_mux_enable(
			data->power_info.i2c_conf);

	if (s_ctrl->func_tbl->sensor_match_id)
		rc = s_ctrl->func_tbl->sensor_match_id(s_ctrl);

exit:
	return rc;
}

static int sony_sensor_power_down(struct msm_sensor_ctrl_t *s_ctrl)
{
	int rc = 0;
	struct msm_camera_sensor_board_info *data = s_ctrl->sensordata;
	uint16_t id = sony_util_get_context(s_ctrl);

	rc = sony_util_power_ctrl(s_ctrl, &camera_data[id], false);
	if (rc < 0)
		LOGE("power_down fail\n");

	if (data->power_info.i2c_conf &&
		data->power_info.i2c_conf->use_i2c_mux)
		sony_util_i2c_mux_disable(
			data->power_info.i2c_conf);

	rc = sony_util_cci_deinit(s_ctrl->sensor_i2c_client);
	if (rc < 0)
		LOGE("%s cci_deinit failed\n", __func__);

	rc = sony_util_gpio_deinit(s_ctrl);
	if (rc < 0)
		LOGE("%s: gpio_deinit failed\n", __func__);


	return rc;
}

static struct v4l2_subdev_info sony_sensor_subdev_info[] = {
	{
		.code		= 0x00,
		.colorspace	= V4L2_COLORSPACE_JPEG,
		.fmt		= 1,
		.order		= 0,
	},
	{
		.code		= 0x00,
		.colorspace	= V4L2_COLORSPACE_JPEG,
		.fmt		= 1,
		.order		= 0,
	},
};

static struct msm_camera_i2c_client sony_sensor_i2c_client[] = {
	{
		.addr_type	= MSM_CAMERA_I2C_WORD_ADDR,
	},
	{
		.addr_type	= MSM_CAMERA_I2C_WORD_ADDR,
	},
};

static const struct of_device_id sony_camera_0_dt_match[] = {
	{
		.compatible = "qcom,sony_camera_0",
		.data = &camera_data[0].s_ctrl
	},
	{
	},
};

static const struct of_device_id sony_camera_1_dt_match[] = {
	{
		.compatible = "qcom,sony_camera_1",
		.data = &camera_data[1].s_ctrl
	},
	{
	},
};

MODULE_DEVICE_TABLE(of, sony_camera_0_dt_match);
MODULE_DEVICE_TABLE(of, sony_camera_1_dt_match);

static struct platform_driver sony_sensor_platform_driver[] = {
	{
		.driver = {
			.name = "qcom,sony_camera_0",
			.owner = THIS_MODULE,
			.of_match_table = sony_camera_0_dt_match,
		},
	},
	{
		.driver = {
			.name = "qcom,sony_camera_1",
			.owner = THIS_MODULE,
			.of_match_table = sony_camera_1_dt_match,
		},
	},
};

static int sony_camera_platform_probe(struct platform_device *pdev)
{
	int rc = 0;
	const struct of_device_id *match;
	uint16_t id = 0;
	struct msm_sensor_ctrl_t *s_ctrl = NULL;
	struct msm_sensor_info_t *sensor_init_params;

	match = of_match_device(sony_camera_0_dt_match, &pdev->dev);
	if (!match && 1 < sensor_num) {
		match = of_match_device(sony_camera_1_dt_match, &pdev->dev);
		id = 1;
	}
	if (!match) {
		LOGE("of_match_device fail\n");
		rc = -EFAULT;
		goto fail;
	}
	camera_data[id].d = &pdev->dev;
	camera_data[id].probe_done = false;

	rc = sony_util_camera_info_init(pdev, id);
	if (rc < 0) {
		LOGE("%s sony_util_camera_info_init failed %d\n",
					__func__, __LINE__);
		goto fail;
	}

	s_ctrl = (struct msm_sensor_ctrl_t *)match->data;
	s_ctrl->sensor_v4l2_subdev_info->code = camera_info[id].subdev_code;

	rc = msm_sensor_platform_probe(pdev, match->data);
	if (rc < 0) {
		LOGE("%s msm_sensor_platform_probe failed %d\n",
					__func__, __LINE__);
		goto fail;
	}

	sensor_init_params = s_ctrl->sensordata->sensor_info;
	camera_data[id].probe_done = true;
	LOGI("camera %d probe ok\n", id);

	return 0;
fail:
	return rc;
}

static int __init sony_sensor_init_module(void)
{
	int rc = 0;
	uint16_t i;
	uint16_t probe_count = 0;

	sensor_num = ARRAY_SIZE(sony_sensor_platform_driver);

	camera_info = kzalloc(sizeof(struct sony_camera_info) * sensor_num,
				GFP_KERNEL);
	if (!camera_info) {
		LOGE("%s failed %d\n", __func__, __LINE__);
		rc = -ENOMEM;
		goto fail_alloc;
	}

	for (i = 0; i < sensor_num; i++) {
		rc = platform_driver_probe(&sony_sensor_platform_driver[i],
						sony_camera_platform_probe);
		if (rc < 0) {
			LOGE("%s platform_driver_probe (%u) %d\n",
				__func__, i, __LINE__);
		} else {
			rc = sony_camera_info_init(i);
			if (rc < 0) {
				LOGE("%s sony_camera_info_init (%u) %d\n",
					__func__, i, __LINE__);
				platform_driver_unregister(
					&sony_sensor_platform_driver[i]);
			} else {
				probe_count++;
			}
		}
	}

	if (!probe_count) {
		LOGE("%s platform_driver_probe (%u) %d\n",
			__func__, probe_count, __LINE__);
		goto fail_probe;
	}

	return 0;
fail_probe:
	kfree(camera_info);
	camera_info = NULL;
fail_alloc:
	return rc;
}

static void __exit sony_sensor_exit_module(void)
{
	uint16_t i;

	sony_camera_info_deinit();
	for (i = 0; i < sensor_num; i++) {
		platform_driver_unregister(&sony_sensor_platform_driver[i]);
		msm_sensor_free_sensor_data(&camera_data[i].s_ctrl);
		sony_util_camera_info_deinit(i);
	}
	kfree(camera_info);
	camera_info = NULL;
}

static struct msm_sensor_fn_t sony_sensor_func_tbl = {
	.sensor_config			= msm_sensor_config,
	.sensor_power_up		= sony_sensor_power_up,
	.sensor_power_down		= sony_sensor_power_down,
	.sensor_match_id		= sony_sensor_match_id,
};

static struct sony_camera_data camera_data[] = {
	{
		.s_ctrl = {
			.sensor_i2c_client		=
					&sony_sensor_i2c_client[0],
			.msm_sensor_mutex		= &sensor_mut,
			.sensor_v4l2_subdev_info	=
					&sony_sensor_subdev_info[0],
			.sensor_v4l2_subdev_info_size	=
					ARRAY_SIZE(sony_sensor_subdev_info),
			.func_tbl			= &sony_sensor_func_tbl,
		},
	},
	{
		.s_ctrl = {
			.sensor_i2c_client		=
					&sony_sensor_i2c_client[1],
			.msm_sensor_mutex		= &sensor_mut,
			.sensor_v4l2_subdev_info	=
					&sony_sensor_subdev_info[1],
			.sensor_v4l2_subdev_info_size	=
					ARRAY_SIZE(sony_sensor_subdev_info),
			.func_tbl			= &sony_sensor_func_tbl,
		},
	},
};

module_init(sony_sensor_init_module);
module_exit(sony_sensor_exit_module);

MODULE_DESCRIPTION("SONY V4L2 camera sensor driver");
MODULE_LICENSE("GPL v2");
