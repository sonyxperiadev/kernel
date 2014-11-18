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
#ifdef CONFIG_MACH_SONY_EAGLE
static struct msm_sensor_ctrl_t gc0339_power_on;
static struct msm_sensor_ctrl_t gc0339_power_off;
int checksubcam_ID = 0;
#endif

static struct msm_sensor_power_setting gc0339_power_setting[] = {

	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_RESET,
		.config_val = GPIO_OUT_LOW,
		.delay = 0,
	},
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_STANDBY,
		.config_val = GPIO_OUT_HIGH,
		.delay = 0,
	},
	{
		.seq_type = SENSOR_VREG,
		.seq_val = CAM_VIO,
		.config_val = 0,
		.delay = 0,
	},
	{
		.seq_type = SENSOR_VREG,
		.seq_val = CAM_VDIG,
		.config_val = 0,
		.delay = 0,
	},
	{
		.seq_type = SENSOR_VREG,
		.seq_val = CAM_VANA,
		.config_val = 0,
		.delay = 0,
	},
	{
		.seq_type = SENSOR_CLK,
		.seq_val = SENSOR_CAM_MCLK,
		.config_val = 24000000,
		.delay = 5,
	},
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_STANDBY,
		.config_val = GPIO_OUT_LOW,
		.delay = 0,
	},
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_RESET,
		.config_val = GPIO_OUT_HIGH,
		.delay = 1,
	},
};

#ifdef CONFIG_MACH_SONY_EAGLE
/*[VY52] VinceT, [Bug#171], S, Sub camera Dual power squence*/
static struct msm_sensor_power_setting gc0339_power_on_setting[] = {
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

static struct msm_sensor_power_setting gc0339_power_off_setting[] = {
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
#endif

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

int32_t gc0339_power_up(struct msm_sensor_ctrl_t *s_ctrl)
{
	int32_t rc = 0, index = 0;
	struct msm_sensor_power_setting_array *power_setting_array = NULL;
	struct msm_sensor_power_setting *power_setting = NULL;
	struct msm_camera_sensor_board_info *data = s_ctrl->sensordata;

	CDBG("%s:%d\n", __func__, __LINE__);
#ifdef CONFIG_MACH_SONY_EAGLE
	power_setting_array = &gc0339_power_on.power_setting_array;
#else
	power_setting_array = &s_ctrl->power_setting_array;
#endif
	if (data->gpio_conf->cam_gpiomux_conf_tbl != NULL) {
		pr_err("%s:%d mux install\n", __func__, __LINE__);
		msm_gpiomux_install(
			(struct msm_gpiomux_config *)
			data->gpio_conf->cam_gpiomux_conf_tbl,
			data->gpio_conf->cam_gpiomux_conf_tbl_size);
	}

	rc = msm_camera_request_gpio_table(
		data->gpio_conf->cam_gpio_req_tbl,
		data->gpio_conf->cam_gpio_req_tbl_size, 1);
	if (rc < 0) {
		pr_err("%s: request gpio failed\n", __func__);
		return rc;
	}
	for (index = 0; index < power_setting_array->size; index++) {
		CDBG("%s index %d\n", __func__, index);
		power_setting = &power_setting_array->power_setting[index];
		CDBG("%s type %d\n", __func__, power_setting->seq_type);
		switch (power_setting->seq_type) {
		case SENSOR_CLK:
			if (power_setting->seq_val >= s_ctrl->clk_info_size) {
				pr_err("%s clk index %d >= max %d\n", __func__,
					power_setting->seq_val,
					s_ctrl->clk_info_size);
				goto power_up_failed;
			}
			if (power_setting->config_val)
				s_ctrl->clk_info[power_setting->seq_val].
					clk_rate = power_setting->config_val;

			rc = msm_cam_clk_enable(s_ctrl->dev,
				&s_ctrl->clk_info[0],
				(struct clk **)&power_setting->data[0],
				s_ctrl->clk_info_size,
				1);
			if (rc < 0) {
				pr_err("%s: clk enable failed\n",
					__func__);
				goto power_up_failed;
			}
			break;
		case SENSOR_GPIO:
			if (power_setting->seq_val >= SENSOR_GPIO_MAX ||
				!data->gpio_conf->gpio_num_info) {
				pr_err("%s gpio index %d >= max %d\n", __func__,
					power_setting->seq_val,
					SENSOR_GPIO_MAX);
				goto power_up_failed;
			}
			pr_debug("%s:%d gpio set val %d\n", __func__, __LINE__,
				data->gpio_conf->gpio_num_info->gpio_num
				[power_setting->seq_val]);
			if (data->gpio_conf->gpio_num_info->gpio_num
				[power_setting->seq_val])
				gpio_set_value_cansleep(
					data->gpio_conf->gpio_num_info->gpio_num
					[power_setting->seq_val],
					power_setting->config_val);
			break;
		case SENSOR_VREG:
			if (power_setting->seq_val >= CAM_VREG_MAX) {
				pr_err("%s vreg index %d >= max %d\n", __func__,
					power_setting->seq_val,
					SENSOR_GPIO_MAX);
				goto power_up_failed;
			}
			msm_camera_config_single_vreg(s_ctrl->dev,
				&data->cam_vreg[power_setting->seq_val],
				(struct regulator **)&power_setting->data[0],
				1);
			break;
		default:
			pr_err("%s error power seq type %d\n", __func__,
				power_setting->seq_type);
			break;
		}
		if (power_setting->delay > 20) {
			msleep(power_setting->delay);
		} else if (power_setting->delay) {
			usleep_range(power_setting->delay * 1000,
				(power_setting->delay * 1000) + 1000);
		}
	}

	if (s_ctrl->sensor_device_type == MSM_CAMERA_PLATFORM_DEVICE) {
		rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_util(
			s_ctrl->sensor_i2c_client, MSM_CCI_INIT);
		if (rc < 0) {
			pr_err("%s cci_init failed\n", __func__);
			goto power_up_failed;
		}
	}

	s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(
		s_ctrl->sensor_i2c_client,
		0xfc,
		0x10, MSM_CAMERA_I2C_BYTE_DATA);

	if (s_ctrl->func_tbl->sensor_match_id)
		rc = s_ctrl->func_tbl->sensor_match_id(s_ctrl);
	else
		rc = msm_sensor_match_id(s_ctrl);
	if (rc < 0) {
		pr_err("%s:%d match id failed rc %d\n", __func__, __LINE__, rc);
		goto power_up_failed;
	}

	CDBG("%s exit\n", __func__);
	return 0;
power_up_failed:
	pr_err("%s:%d failed\n", __func__, __LINE__);
	if (s_ctrl->sensor_device_type == MSM_CAMERA_PLATFORM_DEVICE) {
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_util(
			s_ctrl->sensor_i2c_client, MSM_CCI_RELEASE);
	}

	for (index--; index >= 0; index--) {
		CDBG("%s index %d\n", __func__, index);
		power_setting = &power_setting_array->power_setting[index];
		CDBG("%s type %d\n", __func__, power_setting->seq_type);
		switch (power_setting->seq_type) {
		case SENSOR_CLK:
			msm_cam_clk_enable(s_ctrl->dev,
				&s_ctrl->clk_info[0],
				(struct clk **)&power_setting->data[0],
				s_ctrl->clk_info_size,
				0);
			break;
		case SENSOR_GPIO:
			if (data->gpio_conf->gpio_num_info->gpio_num
				[power_setting->seq_val])
				gpio_set_value_cansleep(
					data->gpio_conf->gpio_num_info->gpio_num
					[power_setting->seq_val],
					GPIOF_OUT_INIT_LOW);
			break;
		case SENSOR_VREG:
			msm_camera_config_single_vreg(s_ctrl->dev,
				&data->cam_vreg[power_setting->seq_val],
				(struct regulator **)&power_setting->data[0],
				0);
			break;
		default:
			pr_err("%s error power seq type %d\n", __func__,
				power_setting->seq_type);
			break;
		}
		if (power_setting->delay > 20) {
			msleep(power_setting->delay);
		} else if (power_setting->delay) {
			usleep_range(power_setting->delay * 1000,
				(power_setting->delay * 1000) + 1000);
		}
	}
	msm_camera_request_gpio_table(
		data->gpio_conf->cam_gpio_req_tbl,
		data->gpio_conf->cam_gpio_req_tbl_size, 0);
	return rc;
}

int32_t gc0339_power_down(struct msm_sensor_ctrl_t *s_ctrl)
{
	int32_t index = 0;

	int32_t gpiotestnum = 0;
	struct msm_sensor_power_setting_array *power_setting_array = NULL;
	struct msm_sensor_power_setting *power_setting = NULL;
	struct msm_camera_sensor_board_info *data = s_ctrl->sensordata;

	CDBG("%s:%d\n", __func__, __LINE__);
#ifdef CONFIG_MACH_SONY_EAGLE
	power_setting_array = &gc0339_power_off.power_setting_array;
#else
	power_setting_array = &s_ctrl->power_setting_array;
#endif
#ifdef CONFIG_MACH_SONY_EAGLE
	s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(
		s_ctrl->sensor_i2c_client,
		0xfc,
		0x01, MSM_CAMERA_I2C_BYTE_DATA);
#endif
	if (s_ctrl->sensor_device_type == MSM_CAMERA_PLATFORM_DEVICE) {
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_util(
			s_ctrl->sensor_i2c_client, MSM_CCI_RELEASE);
	}
#ifndef CONFIG_MACH_SONY_EAGLE
	s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(
		s_ctrl->sensor_i2c_client,
		0xfc,
		0x01, MSM_CAMERA_I2C_BYTE_DATA);
#endif

	for (index = (power_setting_array->size - 1); index >= 0; index--) {
		CDBG("%s index %d\n", __func__, index);
		power_setting = &power_setting_array->power_setting[index];
		CDBG("%s type %d\n", __func__, power_setting->seq_type);
		switch (power_setting->seq_type) {
		case SENSOR_CLK:
#ifdef CONFIG_MACH_SONY_EAGLE
			msm_cam_clk_enable(s_ctrl->dev,
				&s_ctrl->clk_info[0],
				(struct clk **)&gc0339_power_on.power_setting_array.power_setting[index+1].data[0],
				s_ctrl->clk_info_size,
				0);
#else
			msm_cam_clk_enable(s_ctrl->dev,
				&s_ctrl->clk_info[0],
				(struct clk **)&power_setting->data[0],
				s_ctrl->clk_info_size,
				0);
#endif
			break;
		case SENSOR_GPIO:
			if (power_setting->seq_val >= SENSOR_GPIO_MAX ||
				!data->gpio_conf->gpio_num_info) {
				pr_err("%s gpio index %d >= max %d\n", __func__,
					power_setting->seq_val,
					SENSOR_GPIO_MAX);
				continue;
			}
#ifdef CONFIG_MACH_SONY_EAGLE
			gpiotestnum = data->gpio_conf->gpio_num_info->gpio_num
					[power_setting->seq_val];
#endif
			if (data->gpio_conf->gpio_num_info->gpio_num
				[power_setting->seq_val])
#ifdef CONFIG_MACH_SONY_EAGLE
				{
				if((gpiotestnum == 69) && (gpio69_count == 2)){
					pr_err("[VY5X][CTS]Avoid main camera preview fail in CTS\n");
				}
				else {
#endif
				gpio_set_value_cansleep(
					data->gpio_conf->gpio_num_info->gpio_num
					[power_setting->seq_val],
					GPIOF_OUT_INIT_LOW);
#ifdef CONFIG_MACH_SONY_EAGLE
				}

			}
#endif
			break;
		case SENSOR_VREG:
			if (power_setting->seq_val >= CAM_VREG_MAX) {
				pr_err("%s vreg index %d >= max %d\n", __func__,
					power_setting->seq_val,
					SENSOR_GPIO_MAX);
				continue;
			}
#ifdef CONFIG_MACH_SONY_EAGLE
			msm_camera_config_single_vreg(s_ctrl->dev,
				&data->cam_vreg[power_setting->seq_val],
				(struct regulator **)&gc0339_power_on.power_setting_array.power_setting[index].data[0],
				0);
			break;
#else
			msm_camera_config_single_vreg(s_ctrl->dev,
				&data->cam_vreg[power_setting->seq_val],
				(struct regulator **)&power_setting->data[0],
				0);
			break;
#endif
		default:
			pr_err("%s error power seq type %d\n", __func__,
				power_setting->seq_type);
			break;
		}
		if (power_setting->delay > 20) {
			msleep(power_setting->delay);
		} else if (power_setting->delay) {
			usleep_range(power_setting->delay * 1000,
				(power_setting->delay * 1000) + 1000);
		}
	}
	msm_camera_request_gpio_table(
		data->gpio_conf->cam_gpio_req_tbl,
		data->gpio_conf->cam_gpio_req_tbl_size, 0);
	CDBG("%s exit\n", __func__);
	return 0;
}

int32_t gc0339_match_id(struct msm_sensor_ctrl_t *s_ctrl)
{
	int32_t rc = 0;
	uint16_t chipid = 0;
#ifdef CONFIG_MACH_SONY_EAGLE
	uint16_t camID_GPIO = 115;
	int32_t subcamID = 1;
	int ret1=0;
#endif

	rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(
			s_ctrl->sensor_i2c_client,
			s_ctrl->sensordata->slave_info->sensor_id_reg_addr,
			&chipid, MSM_CAMERA_I2C_BYTE_DATA);
	if (rc < 0) {
		pr_err("%s: %s: read id failed\n", __func__,
			s_ctrl->sensordata->sensor_name);
		return rc;
	}

#ifdef CONFIG_MACH_SONY_EAGLE
	if(checksubcam_ID == 0) {
	if(strcmp("SKUAA_ST_gc0339",s_ctrl->sensordata->sensor_name)==0) {
		ret1 = gpio_request(camID_GPIO, NULL);
		if (ret1) {
		pr_err("%s: Failed to request gpio %d\n", __func__,
			   camID_GPIO);
		}
		gpio_direction_input(camID_GPIO);
		subcamID = gpio_get_value(camID_GPIO);
		pr_err("%s:[Camera] sub camera ID pin value: %d\n", __func__,
			subcamID);
		if( subcamID == 0) {//main source
			s_ctrl->sensordata->sensor_name = "SKUAA_ST_gc0339";
		}
		else if (subcamID == 1) {//second source
			s_ctrl->sensordata->sensor_name = "SKUAA_ST_gc0339sec";
		}
		else {
			pr_err("%s:[Error] sub camera ID pin value is wrong! value=%d\n", __func__,
				subcamID);
		}
		pr_err("%s:[Camera] sub camera sensor-name change to: %s\n", __func__,
			s_ctrl->sensordata->sensor_name);
		gpio_free(camID_GPIO);
	}
		checksubcam_ID = checksubcam_ID+1;
	}
#endif

	if (chipid != s_ctrl->sensordata->slave_info->sensor_id) {
		pr_err("msm_sensor_match_id chip id doesnot match\n");
		return -ENODEV;
	}
	return rc;
}

int32_t gc0339_config(struct msm_sensor_ctrl_t *s_ctrl,
	void __user *argp)
{
	struct sensorb_cfg_data *cdata = (struct sensorb_cfg_data *)argp;
	long rc = 0;
	int32_t i = 0;
	mutex_lock(s_ctrl->msm_sensor_mutex);
	switch (cdata->cfgtype) {
	case CFG_GET_SENSOR_INFO:
		memcpy(cdata->cfg.sensor_info.sensor_name,
			s_ctrl->sensordata->sensor_name,
			sizeof(cdata->cfg.sensor_info.sensor_name));
		cdata->cfg.sensor_info.session_id =
			s_ctrl->sensordata->sensor_info->session_id;
		for (i = 0; i < SUB_MODULE_MAX; i++)
			cdata->cfg.sensor_info.subdev_id[i] =
				s_ctrl->sensordata->sensor_info->subdev_id[i];
		CDBG("%s:%d sensor name %s\n", __func__, __LINE__,
			cdata->cfg.sensor_info.sensor_name);
		CDBG("%s:%d session id %d\n", __func__, __LINE__,
			cdata->cfg.sensor_info.session_id);
		for (i = 0; i < SUB_MODULE_MAX; i++)
			CDBG("%s:%d subdev_id[%d] %d\n", __func__, __LINE__, i,
				cdata->cfg.sensor_info.subdev_id[i]);

		break;
	case CFG_GET_SENSOR_INIT_PARAMS:
		cdata->cfg.sensor_init_params =
			*s_ctrl->sensordata->sensor_init_params;
		CDBG("%s:%d init params mode %d pos %d mount %d\n", __func__,
			__LINE__,
			cdata->cfg.sensor_init_params.modes_supported,
			cdata->cfg.sensor_init_params.position,
			cdata->cfg.sensor_init_params.sensor_mount_angle);
		break;
	case CFG_SET_SLAVE_INFO: {
		struct msm_camera_sensor_slave_info sensor_slave_info;
		struct msm_sensor_power_setting_array *power_setting_array;
		int slave_index = 0;
		if (copy_from_user(&sensor_slave_info,
			(void *)cdata->cfg.setting,
			sizeof(struct msm_camera_sensor_slave_info))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}
		/* Update sensor slave address */
		if (sensor_slave_info.slave_addr) {
			s_ctrl->sensor_i2c_client->cci_client->sid =
				sensor_slave_info.slave_addr >> 1;
		}

		/* Update sensor address type */
		s_ctrl->sensor_i2c_client->addr_type =
			sensor_slave_info.addr_type;

		/* Update power up / down sequence */
		s_ctrl->power_setting_array =
			sensor_slave_info.power_setting_array;
		power_setting_array = &s_ctrl->power_setting_array;
		power_setting_array->power_setting = kzalloc(
			power_setting_array->size *
			sizeof(struct msm_sensor_power_setting), GFP_KERNEL);
		if (!power_setting_array->power_setting) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -ENOMEM;
			break;
		}
		if (copy_from_user(power_setting_array->power_setting,
			(void *)
			sensor_slave_info.power_setting_array.power_setting,
			power_setting_array->size *
			sizeof(struct msm_sensor_power_setting))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}
		s_ctrl->free_power_setting = true;
		CDBG("%s sensor id %x\n", __func__,
			sensor_slave_info.slave_addr);
		CDBG("%s sensor addr type %d\n", __func__,
			sensor_slave_info.addr_type);
		CDBG("%s sensor reg %x\n", __func__,
			sensor_slave_info.sensor_id_info.sensor_id_reg_addr);
		CDBG("%s sensor id %x\n", __func__,
			sensor_slave_info.sensor_id_info.sensor_id);
		for (slave_index = 0; slave_index <
			power_setting_array->size; slave_index++) {
			CDBG("%s i %d power setting %d %d %ld %d\n", __func__,
				slave_index,
				power_setting_array->power_setting[slave_index].
				seq_type,
				power_setting_array->power_setting[slave_index].
				seq_val,
				power_setting_array->power_setting[slave_index].
				config_val,
				power_setting_array->power_setting[slave_index].
				delay);
		}
		break;
	}
	case CFG_WRITE_I2C_ARRAY: {
		struct msm_camera_i2c_reg_setting conf_array;
		struct msm_camera_i2c_reg_array *reg_setting = NULL;

		if (copy_from_user(&conf_array,
			(void *)cdata->cfg.setting,
			sizeof(struct msm_camera_i2c_reg_setting))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}

		if (conf_array.addr_type == MSM_CAMERA_I2C_WORD_ADDR
			|| conf_array.data_type == MSM_CAMERA_I2C_WORD_DATA
			|| !conf_array.size)
			break;

		reg_setting = kzalloc(conf_array.size *
			(sizeof(struct msm_camera_i2c_reg_array)), GFP_KERNEL);
		if (!reg_setting) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -ENOMEM;
			break;
		}
		if (copy_from_user(reg_setting, (void *)conf_array.reg_setting,
			conf_array.size *
			sizeof(struct msm_camera_i2c_reg_array))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			kfree(reg_setting);
			rc = -EFAULT;
			break;
		}

		conf_array.reg_setting = reg_setting;
		rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write_table(
			s_ctrl->sensor_i2c_client, &conf_array);
		kfree(reg_setting);
		break;
	}
	case CFG_WRITE_I2C_SEQ_ARRAY: {
		struct msm_camera_i2c_seq_reg_setting conf_array;
		struct msm_camera_i2c_seq_reg_array *reg_setting = NULL;

		if (copy_from_user(&conf_array,
			(void *)cdata->cfg.setting,
			sizeof(struct msm_camera_i2c_seq_reg_setting))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}

		reg_setting = kzalloc(conf_array.size *
			(sizeof(struct msm_camera_i2c_seq_reg_array)),
			GFP_KERNEL);
		if (!reg_setting) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -ENOMEM;
			break;
		}
		if (copy_from_user(reg_setting, (void *)conf_array.reg_setting,
			conf_array.size *
			sizeof(struct msm_camera_i2c_seq_reg_array))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			kfree(reg_setting);
			rc = -EFAULT;
			break;
		}

		conf_array.reg_setting = reg_setting;
		rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->
			i2c_write_seq_table(s_ctrl->sensor_i2c_client,
			&conf_array);
		kfree(reg_setting);
		break;
	}

	case CFG_POWER_UP:
		if (s_ctrl->func_tbl->sensor_power_up)
			rc = s_ctrl->func_tbl->sensor_power_up(s_ctrl);
		else
			rc = -EFAULT;
		break;

	case CFG_POWER_DOWN:
		if (s_ctrl->func_tbl->sensor_power_down)
			rc = s_ctrl->func_tbl->sensor_power_down(
				s_ctrl);
		else
			rc = -EFAULT;
		break;

	case CFG_SET_STOP_STREAM_SETTING: {
		struct msm_camera_i2c_reg_setting *stop_setting =
			&s_ctrl->stop_setting;
		struct msm_camera_i2c_reg_array *reg_setting = NULL;
		if (copy_from_user(stop_setting,
			(void *)cdata->cfg.setting,
			sizeof(struct msm_camera_i2c_reg_setting))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}

		reg_setting = stop_setting->reg_setting;
		stop_setting->reg_setting = kzalloc(stop_setting->size *
			(sizeof(struct msm_camera_i2c_reg_array)), GFP_KERNEL);
		if (!stop_setting->reg_setting) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -ENOMEM;
			break;
		}
		if (copy_from_user(stop_setting->reg_setting,
			(void *)reg_setting,
			stop_setting->size *
			sizeof(struct msm_camera_i2c_reg_array))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			kfree(stop_setting->reg_setting);
			stop_setting->reg_setting = NULL;
			stop_setting->size = 0;
			rc = -EFAULT;
			break;
		}
		break;
	}
	default:
		rc = -EFAULT;
		break;
	}

	mutex_unlock(s_ctrl->msm_sensor_mutex);
	return rc;
}

static struct msm_sensor_fn_t gc0339_sensor_fn_t = {
	.sensor_power_up = gc0339_power_up,
	.sensor_power_down = gc0339_power_down,
	.sensor_match_id = gc0339_match_id,
	.sensor_config = gc0339_config,
};


static struct msm_sensor_ctrl_t gc0339_s_ctrl = {
	.sensor_i2c_client = &gc0339_sensor_i2c_client,
	.power_setting_array.power_setting = gc0339_power_setting,
	.power_setting_array.size = ARRAY_SIZE(gc0339_power_setting),
	.msm_sensor_mutex = &gc0339_mut,
	.sensor_v4l2_subdev_info = gc0339_subdev_info,
	.sensor_v4l2_subdev_info_size = ARRAY_SIZE(gc0339_subdev_info),
	.func_tbl = &gc0339_sensor_fn_t,
};
#ifdef CONFIG_MACH_SONY_EAGLE
static struct msm_sensor_ctrl_t gc0339_power_on =
{
	.power_setting_array.power_setting = gc0339_power_on_setting,
	.power_setting_array.size = ARRAY_SIZE(gc0339_power_on_setting),
};
static struct msm_sensor_ctrl_t gc0339_power_off =
{
	.power_setting_array.power_setting = gc0339_power_off_setting,
	.power_setting_array.size = ARRAY_SIZE(gc0339_power_off_setting),
};
#endif

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
	if (!rc)
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
