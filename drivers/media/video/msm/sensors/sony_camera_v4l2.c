/* drivers/media/video/msm/sensors/sony_camera_v4l2.c
 *
 * Copyright (C) 2012-2013 Sony Mobile Communications AB.
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
#define ENABLE_LOGD
/* #define ENABLE_LOGV */

#include <linux/regulator/consumer.h>
#include <linux/i2c.h>
#include <mach/gpiomux.h>
#include <media/msm_camera.h>
#include <mach/camera.h>
#include <media/sony_camera_v4l2.h>
#include "msm_sensor.h"
#include "msm_camera_i2c.h"
#include "msm_ispif.h"
#include "msm.h"
#include "msm_camera_i2c_mux.h"
#include "msm_sensor_common.h"

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

#ifdef ENABLE_LOGV
#define LOGV(f, a...)	dev_vdbg(camera_device, "%s: " f, __func__, ##a)
#else
#define LOGV(f, a...)
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

struct sony_camera_data {
	bool				probe_done;
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
};

static struct sony_camera_data camera_data[];
static struct device *camera_device;

static DEFINE_MUTEX(sensor_mut);

static uint16_t sensor_num;

static struct msm_cam_clk_info cam_clk_info[] = {
	{ "cam_clk", 0 },
};

static int32_t sony_util_get_context(struct msm_sensor_ctrl_t *s_ctrl)
{
	uint16_t i;
	char sensor_name[32];

	memset(sensor_name, 0, sizeof(sensor_name));
	for (i = 0; i < sensor_num; i++) {
		snprintf(sensor_name, sizeof(sensor_name), "sony_camera_%d", i);
		if (!strncmp(s_ctrl->sensordata->sensor_name,
				sensor_name, sizeof(sensor_name)))
			break;
	}

	if (camera_data[i].d)
		camera_device = camera_data[i].d;

	return i;
}

static int sony_util_gpio_set(int gpio_pin, int value)
{
	gpio_set_value_cansleep(gpio_pin, value);

	return 0;
}

static int sony_util_vreg_set(struct msm_sensor_ctrl_t *s_ctrl,
	struct sony_camera_data *data, enum sony_camera_cmd cmd, int level)
{
	int rc = 0;
	struct regulator *vreg;
	struct device *dev = &s_ctrl->sensor_i2c_client->client->dev;

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
		if (level > 0)
			rc = regulator_set_voltage(vreg, level, level);
		if (rc == 0) {
			rc = regulator_enable(vreg);
			if (rc < 0)
				regulator_disable(vreg);
		}
		if (rc < 0)
			regulator_put(vreg);
	} else {
		rc = regulator_disable(vreg);
		regulator_put(vreg);
	}

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

exit:
	if (rc < 0)
		LOGE("error happened (%d)\n", rc);
	return rc;
}

static int sony_util_mclk_set(struct msm_sensor_ctrl_t *s_ctrl,
			uint32_t *mclk, int value)
{
	int rc;

	if (value >= 0) {
		if (value == 0)
			cam_clk_info[0].clk_rate = SENSOR_MCLK_DEFAULT;
		else
			cam_clk_info[0].clk_rate = (long)value;

		if (value > 0 && mclk)
			*mclk = value;

		rc = msm_cam_clk_enable(
			&s_ctrl->sensor_i2c_client->client->dev,
			(struct msm_cam_clk_info *)cam_clk_info,
			s_ctrl->cam_clk, ARRAY_SIZE(cam_clk_info), 1);
	} else {
		rc = msm_cam_clk_enable(
			&s_ctrl->sensor_i2c_client->client->dev,
			(struct msm_cam_clk_info *)cam_clk_info,
			s_ctrl->cam_clk, ARRAY_SIZE(cam_clk_info), 0);
	}

	if (rc < 0)
		LOGE("error happened (%d)\n", rc);

	return rc;
}

static int sony_cam_i2c_read(struct msm_sensor_ctrl_t *s_ctrl,
			uint8_t slave_addr, uint32_t addr,
			uint8_t type, uint16_t len, uint8_t *data)
{
	int rc;
	uint16_t id = sony_util_get_context(s_ctrl);
	uint8_t tmp[SENSOR_I2C_ADDR_4BYTE];
	struct i2c_msg msgs[] = {
		{
			.addr	= slave_addr,
			.flags	= 0,
			.len	= type,
			.buf	= tmp,
		},
		{
			.addr	= slave_addr,
			.flags	= I2C_M_RD,
			.len	= len,
			.buf	= camera_data[id].buf,
		},
	};
	struct i2c_msg *pmsg = msgs;
	uint8_t msglen = 2;

	LOGV("slave:0x%04x addr:0x%04x, type:0x%02x, len:0x%02x\n",
			slave_addr, addr, type, len);

	if (len > I2C_MAX_DATA_LEN) {
		rc = -EPERM;
		LOGE("invalid parameter\n");
		goto exit;
	}

	switch (type) {
	case SENSOR_I2C_ADDR_0BYTE:
		pmsg = &msgs[1];
		msglen = 1;
		break;
	case SENSOR_I2C_ADDR_1BYTE:
		pmsg->buf[0] = (uint8_t)((addr & 0xFF) >> 0);
		break;
	case SENSOR_I2C_ADDR_2BYTE:
		pmsg->buf[0] = (uint8_t)((addr & 0xFF00) >> 8);
		pmsg->buf[1] = (uint8_t)((addr & 0x00FF) >> 0);
		break;
	case SENSOR_I2C_ADDR_4BYTE:
		pmsg->buf[0] = (uint8_t)((addr & 0xFF000000) >> 24);
		pmsg->buf[1] = (uint8_t)((addr & 0x00FF0000) >> 16);
		pmsg->buf[2] = (uint8_t)((addr & 0x0000FF00) >> 8);
		pmsg->buf[3] = (uint8_t)((addr & 0x000000FF) >> 0);
		break;
	default:
		rc = -EINVAL;
		LOGE("invalid case\n");
		goto exit;
	}

	rc = i2c_transfer(s_ctrl->sensor_i2c_client->client->adapter,
				pmsg, msglen);
	if (rc < 0 || rc != msglen) {
		LOGE("slave:0x%04x, addr:0x%04x, type:0x%02x, len:0x%02x\n",
				slave_addr, addr, type, len);
		LOGE("i2c transfer failed (%d)\n", rc);
		rc = -EIO;
		goto exit;
	}
	memcpy(data, camera_data[id].buf, len);
	rc = 0;

exit:
	return rc;
}

static int sony_cam_i2c_write(struct msm_sensor_ctrl_t *s_ctrl,
	uint8_t slave_addr, uint32_t addr, uint8_t type,
	uint16_t len, uint8_t *data)
{
	int rc;
	uint16_t i;
	uint16_t id = sony_util_get_context(s_ctrl);

	struct i2c_msg msgs[] = {
		{
			.addr	= slave_addr,
			.flags	= 0,
			.len	= len + type,
			.buf	= camera_data[id].buf,
		},
	};

	struct i2c_msg *pmsg = msgs;
	uint8_t msglen = 1;
	LOGV("addr:0x%04x, type:0x%02x, len:0x%02x\n", addr, type, len);

	if (len + type > I2C_MAX_DATA_LEN) {
		rc = -EPERM;
		LOGE("invalid parameter\n");
		goto exit;
	}

	switch (type) {
	case SENSOR_I2C_ADDR_1BYTE:
		pmsg->buf[0] = (uint8_t)((addr & 0xFF) >> 0);
		break;
	case SENSOR_I2C_ADDR_2BYTE:
		pmsg->buf[0] = (uint8_t)((addr & 0xFF00) >> 8);
		pmsg->buf[1] = (uint8_t)((addr & 0x00FF) >> 0);
		break;
	case SENSOR_I2C_ADDR_4BYTE:
		pmsg->buf[0] = (uint8_t)((addr & 0xFF000000) >> 24);
		pmsg->buf[1] = (uint8_t)((addr & 0x00FF0000) >> 16);
		pmsg->buf[2] = (uint8_t)((addr & 0x0000FF00) >> 8);
		pmsg->buf[3] = (uint8_t)((addr & 0x000000FF) >> 0);
		break;
	default:
		rc = -EINVAL;
		LOGE("invalid case\n");
		goto exit;
	}

	memcpy(camera_data[id].buf + type, data, len);
	rc = i2c_transfer(s_ctrl->sensor_i2c_client->client->adapter,
			pmsg, msglen);
	if (rc < 0 || rc != msglen) {
		LOGE("slave:0x%04x,addr:0x%04x,type:0x%02x,len:0x%02x,data:",
				slave_addr, addr, type, len);
		for (i = 0; i < len; i++)
			LOGE("0x%02x ", *(data + i));
		LOGE("\ni2c transfer failed (%d)\n", rc);
		rc = -EIO;
		goto exit;
	}
	rc = 0;

exit:
	return rc;
}

static int sony_eeprom_load(struct msm_sensor_ctrl_t *s_ctrl)
{
	int rc = 0;
	uint16_t i;
	uint16_t len;
	uint16_t id = sony_util_get_context(s_ctrl);
	uint8_t *d = camera_data[id].eeprom;

	LOGV("load eeprom\n");

	/* load eeprom */
	if (camera_info[id].eeprom_type == 0) {
		for (i = 0; i < camera_info[id].eeprom_max_len;
				i += I2C_MAX_DATA_LEN) {
			uint8_t slave_addr = camera_info[id].eeprom_addr
						+ ((i & 0x0700) >> 8);
			uint32_t offset = i & 0x00FF;

			rc = sony_cam_i2c_read(
				&camera_data[id].s_ctrl,
				slave_addr,
				offset, SENSOR_I2C_ADDR_1BYTE,
				I2C_MAX_DATA_LEN, d + i);
			if (rc < 0) {
				LOGE("eeprom i2c read fail %d\n", rc);
				camera_data[id].eeprom_len = 0;
				goto exit;
			}
		}
		len = i;
	} else {
		len = 16;
		d[0] = 0x03;
		d[1] = 0x25;
		rc = sony_cam_i2c_write(
			&camera_data[id].s_ctrl,
			camera_info[id].eeprom_addr,
			0x0010, SENSOR_I2C_ADDR_2BYTE, 2, d);
		if (rc < 0)
			goto exit;
		d[0] = 0x08;
		d[1] = 0x00;
		rc = sony_cam_i2c_write(
			&camera_data[id].s_ctrl,
			camera_info[id].eeprom_addr,
			0x0012, SENSOR_I2C_ADDR_2BYTE, 2, d);
		if (rc < 0)
			goto exit;

		i = 0;
		do {
			msleep(20);
			i++;
			rc = sony_cam_i2c_read(
				&camera_data[id].s_ctrl,
				camera_info[id].eeprom_addr,
				0x0014, SENSOR_I2C_ADDR_2BYTE, 2, d);
			if (rc < 0)
				goto exit;
		} while ((d[0] & 0x80) && i < 100);

		if (i >= 100) {
			rc = -ENODEV;
			goto exit;
		}

		d[0] = 0x45;
		d[1] = 0x04;
		rc = sony_cam_i2c_write(
			&camera_data[id].s_ctrl,
			camera_info[id].eeprom_addr,
			0x0018, SENSOR_I2C_ADDR_2BYTE, 2, d);
		if (rc < 0)
			goto exit;

		i = 0;
		do {
			msleep(20);
			i++;
			rc = sony_cam_i2c_read(
				&camera_data[id].s_ctrl,
				camera_info[id].eeprom_addr,
				0x0018, SENSOR_I2C_ADDR_2BYTE, 2, d);
			if (rc < 0)
				goto exit;
		} while ((d[1] & 0x40) && i < 100);

		if (i >= 100) {
			rc = -ENODEV;
			goto exit;
		}

		d[0] = 0x05;
		d[1] = 0x20;
		rc = sony_cam_i2c_write(
			&camera_data[id].s_ctrl,
			camera_info[id].eeprom_addr,
			0x001A, SENSOR_I2C_ADDR_2BYTE, 2, d);
		if (rc < 0)
			goto exit;

		d[0] = 0x05;
		d[1] = 0x64;
		rc = sony_cam_i2c_write(
			&camera_data[id].s_ctrl,
			camera_info[id].eeprom_addr,
			0x001A, SENSOR_I2C_ADDR_2BYTE, 2, d);
		if (rc < 0)
			goto exit;

		rc = sony_cam_i2c_read(&camera_data[id].s_ctrl,
					camera_info[id].eeprom_addr,
					0x0000, SENSOR_I2C_ADDR_2BYTE, 2, d);
		if (rc < 0) {
			goto exit;
		}

		if (((uint16_t)d[0] << 8 | d[1]) == SENSOR_ID_MT9V115) {
			rc = sony_cam_i2c_read(
					&camera_data[id].s_ctrl,
					camera_info[id].eeprom_addr,
					0x001A, SENSOR_I2C_ADDR_2BYTE, 2, d);
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

	if (camera_data[id].module) {
		camera_data[id].s_ctrl.sensor_i2c_addr
					= camera_info[id].i2c_addr;
		camera_data[id].eeprom_len = len;
	} else {
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
		camera_data[id].s_ctrl.sensor_i2c_addr =
				camera_info[id].i2c_addr;
		camera_data[id].eeprom_len = len;
	}

	LOGD("MODULE NAME: len=%d, name=%c%c%c%c%c%c%c%c\n",
			len, d[0], d[1], d[2], d[3],
			d[4], d[5], d[6], d[7]);

	/* TODO: Fix to overwrite the data when illegal data */
	/*       had been written on eeprom.                 */
	if (!strncmp((char *)&camera_data[0].eeprom[0x00], "SEM13BS0", 8) &&
	    !strncmp((char *)&camera_data[0].eeprom[0x10], "1C", 2)) {
		for (i = 0x40; i < 0x40 + 8; i++)
			camera_data[0].eeprom[i] = 0xFF;
		for (i = 0x70; i < 0x70 + 16; i++)
			camera_data[0].eeprom[i] = 0xFF;
		for (i = 0x98; i < 0x98 + 8; i++)
			camera_data[0].eeprom[i] = 0xFF;
		for (i = 0xBC; i < 0xBC + 12; i++)
			camera_data[0].eeprom[i] = 0xFF;
		for (i = 0xD0; i < 0xD0 + 6; i++)
			camera_data[0].eeprom[i] = 0xFF;
		for (i = 0x100; i < 0x100 + (64 * 12); i++)
			camera_data[0].eeprom[i] = 0xFF;
	}

exit:
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
			rc = sony_util_gpio_set(s_ctrl->sensordata->
				sensor_platform_info->sensor_reset,
				seq->val);
			break;
		case SONY_GPIO_AF:
			rc = sony_util_gpio_set(camera_info[id].gpio_af,
				seq->val);
			break;
		case SONY_CAM_VDIG:
		case SONY_CAM_VIO:
		case SONY_CAM_VANA:
		case SONY_CAM_VAF:
			rc = sony_util_vreg_set(s_ctrl,
				data, seq->cmd, seq->val);
			break;
		case SONY_CAM_CLK:
			rc = sony_util_mclk_set(s_ctrl,
				(uint32_t *)&s_ctrl->clk_rate, seq->val);
			break;
		case SONY_I2C_WRITE:
			rc = sony_cam_i2c_write(s_ctrl,
				s_ctrl->sensor_i2c_addr,
				seq->val,
				SENSOR_I2C_ADDR_2BYTE,
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

static void sony_sensor_start_stream(struct msm_sensor_ctrl_t *s_ctrl)
{
}

static void sony_sensor_stop_stream(struct msm_sensor_ctrl_t *s_ctrl)
{
}

static void sony_sensor_group_hold_on(struct msm_sensor_ctrl_t *s_ctrl)
{
}

static void sony_sensor_group_hold_off(struct msm_sensor_ctrl_t *s_ctrl)
{
}

static int32_t sony_sensor_set_fps(struct msm_sensor_ctrl_t *s_ctrl,
						struct fps_cfg *fps)
{
	return 0;
}

static int32_t sony_sensor_write_exp_gain1(struct msm_sensor_ctrl_t *s_ctrl,
	        uint16_t gain, uint32_t line, int32_t luma_avg, uint16_t fgain)
{
	return 0;
}

static int32_t sony_sensor_setting(struct msm_sensor_ctrl_t *s_ctrl,
			int update_type, int res)
{
	int32_t rc = 0;

	s_ctrl->func_tbl->sensor_stop_stream(s_ctrl);

	if (update_type == MSM_SENSOR_UPDATE_PERIODIC) {

		v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
			NOTIFY_PCLK_CHANGE,
			&s_ctrl->msm_sensor_reg->
			output_settings[res].op_pixel_clk);
	}

	return rc;
}

static int32_t sony_sensor_setting1(struct msm_sensor_ctrl_t *s_ctrl,
			int update_type, int res)
{
	return 0;
}

static int32_t sony_sensor_set_sensor_mode(struct msm_sensor_ctrl_t *s_ctrl,
	int mode, int res)
{
	int32_t rc = 0;

	LOGV("%s: mode[%d] res[%d]\n", __func__, mode, res);

	if (s_ctrl->is_csic ||
		!s_ctrl->sensordata->csi_if)
		rc = s_ctrl->func_tbl->sensor_csi_setting(s_ctrl,
			MSM_SENSOR_UPDATE_PERIODIC, res);
	else
		rc = s_ctrl->func_tbl->sensor_setting(s_ctrl,
			MSM_SENSOR_UPDATE_PERIODIC, res);
	if (rc < 0)
		return rc;
	s_ctrl->curr_res = res;

	return rc;
}

static int32_t sony_sensor_mode_init(struct msm_sensor_ctrl_t *s_ctrl,
			int mode, struct sensor_init_cfg *init_info)
{
	int32_t rc = 0;
	s_ctrl->fps_divider = Q10;

	LOGV("%s: %d\n", __func__, __LINE__);

	s_ctrl->curr_res = MSM_SENSOR_INVALID_RES;
	s_ctrl->cam_mode = mode;

	if (s_ctrl->is_csic ||
		!s_ctrl->sensordata->csi_if)
		rc = s_ctrl->func_tbl->sensor_csi_setting(s_ctrl,
			MSM_SENSOR_REG_INIT, 0);
	else
		rc = s_ctrl->func_tbl->sensor_setting(s_ctrl,
			MSM_SENSOR_REG_INIT, 0);

	return rc;
}

static int32_t sony_sensor_get_output_info(struct msm_sensor_ctrl_t *s_ctrl,
		struct sensor_output_info_t *sensor_output_info)
{
	return 0;
}

static int sony_sensor_config(struct msm_sensor_ctrl_t *s_ctrl,
			void __user *argp)
{
	int rc = 0;
	struct sensor_cfg_data cdata;
	uint8_t slave_addr;
	uint32_t addr;
	uint8_t type;
	uint8_t length;
	uint8_t data[I2C_MAX_DATA_LEN];
	uint16_t rom_offset;
	uint16_t rom_len;
	uint16_t id = sony_util_get_context(s_ctrl);

	mutex_lock(s_ctrl->msm_sensor_mutex);

	if (copy_from_user(&cdata, (void *)argp, sizeof(cdata))) {
		rc = -EFAULT;
		LOGE("copy failed\n");
		goto exit;
	}

	switch (cdata.cfgtype) {
	case CFG_SET_WRITE_CMD:
		slave_addr = cdata.cfg.i2c_io.slave_addr;
		addr = cdata.cfg.i2c_io.address;
		type = cdata.cfg.i2c_io.address_type;
		length = cdata.cfg.i2c_io.length;
		if (!length || length > I2C_MAX_DATA_LEN) {
			rc = -EPERM;
			LOGE("parameter failed\n");
			goto exit;
		}
		memset(data, 0, sizeof(data));
		if (copy_from_user(data, cdata.cfg.i2c_io.data, length)) {
			rc = -EFAULT;
			LOGE("copy failed\n");
			goto exit;
		}
		rc = sony_cam_i2c_write(s_ctrl,
			slave_addr, addr, type, length, data);
		if (rc < 0) {
			LOGE("i2c write failed\n");
			goto exit;
		}
		break;
	case CFG_SET_READ_CMD:
		slave_addr = cdata.cfg.i2c_io.slave_addr;
		addr = cdata.cfg.i2c_io.address;
		type = cdata.cfg.i2c_io.address_type;
		length = cdata.cfg.i2c_io.length;
		if (!length || length > I2C_MAX_DATA_LEN) {
			rc = -EPERM;
			LOGE("parameter failed\n");
			goto exit;
		}
		memset(data, 0, sizeof(data));
		rc = sony_cam_i2c_read(s_ctrl,
			slave_addr, addr, type, length, data);
		if (rc < 0) {
			LOGE("i2c read failed\n");
			goto exit;
		}
		if (copy_to_user(cdata.cfg.i2c_io.data, data, length)) {
			rc = -EFAULT;
			LOGE("copy failed\n");
			goto exit;
		}
		break;
	case CFG_SET_GPIO_CTRL:
		if (cdata.cfg.gpio_ctrl.gpio == SENSOR_GPIO_CTRL_RESET) {
			rc = sony_util_gpio_set(
				s_ctrl->sensordata->sensor_reset,
				cdata.cfg.gpio_ctrl.value);
		} else
			rc = -EINVAL;
		if (rc < 0) {
			LOGE("gpio control failed\n");
			goto exit;
		}
		break;
	case CFG_GET_ROM:
		rom_offset = cdata.cfg.rom_in.address;
		rom_len = cdata.cfg.rom_in.length;
		if (!rom_len || rom_offset >=
			camera_info[id].eeprom_max_len) {
			rc = -EPERM;
			LOGE("parameter failed\n");
			goto exit;
		}
		if (rom_offset + rom_len > camera_info[id].eeprom_max_len)
			rom_len = camera_info[id].eeprom_max_len - rom_offset;
		if (copy_to_user(cdata.cfg.rom_in.data,
			camera_data[id].eeprom + rom_offset, rom_len)) {
			rc = -EFAULT;
			LOGE("copy failed\n");
			goto exit;
		}
		break;
	default:
		LOGV("generic command %d\n", cdata.cfgtype);
		mutex_unlock(s_ctrl->msm_sensor_mutex);
		rc = msm_sensor_config(s_ctrl, argp);
		mutex_lock(s_ctrl->msm_sensor_mutex);
		if (rc < 0)
			LOGE("generic command failed\n");
		goto exit;
	}

	if (copy_to_user((void *)argp, &cdata, sizeof(cdata))) {
		rc = -EFAULT;
		LOGE("copy failed\n");
		goto exit;
	}

exit:
	mutex_unlock(s_ctrl->msm_sensor_mutex);

	if (rc < 0)
		LOGE("error happened (%d)\n", rc);

	return rc;
}

static int sony_sensor_enable_i2c_mux(struct msm_camera_i2c_conf *i2c_conf)
{
	struct v4l2_subdev *i2c_mux_sd =
		dev_get_drvdata(&i2c_conf->mux_dev->dev);
	v4l2_subdev_call(i2c_mux_sd, core, ioctl,
		VIDIOC_MSM_I2C_MUX_INIT, NULL);
	v4l2_subdev_call(i2c_mux_sd, core, ioctl,
		VIDIOC_MSM_I2C_MUX_CFG, (void *)&i2c_conf->i2c_mux_mode);
	return 0;
}

static int sony_sensor_disable_i2c_mux(struct msm_camera_i2c_conf *i2c_conf)
{
	struct v4l2_subdev *i2c_mux_sd =
		dev_get_drvdata(&i2c_conf->mux_dev->dev);
	v4l2_subdev_call(i2c_mux_sd, core, ioctl,
				VIDIOC_MSM_I2C_MUX_RELEASE, NULL);
	return 0;
}

static int sony_sensor_power_up(struct msm_sensor_ctrl_t *s_ctrl)
{
	int rc = 0;
	struct msm_camera_sensor_info *data = s_ctrl->sensordata;
	uint16_t id = sony_util_get_context(s_ctrl);

	LOGV("%s: %d\n", __func__, __LINE__);

	rc = msm_camera_request_gpio_table(data, 1);
	if (rc < 0)
		goto exit;

	rc = sony_util_power_ctrl(s_ctrl, &camera_data[id], true);
	if (rc < 0)
		LOGE("power_up fail\n");

	if (data->sensor_platform_info->i2c_conf &&
		data->sensor_platform_info->i2c_conf->use_i2c_mux)
		sony_sensor_enable_i2c_mux(
					data->sensor_platform_info->i2c_conf);
exit:
	return rc;
}

static int sony_sensor_power_down(struct msm_sensor_ctrl_t *s_ctrl)
{
	int rc = 0;
	struct msm_camera_sensor_info *data = s_ctrl->sensordata;
	struct device *dev = &s_ctrl->sensor_i2c_client->client->dev;
	struct msm_camera_sensor_info *sinfo = dev->platform_data;
	uint16_t id = sony_util_get_context(s_ctrl);

	if (sinfo->sensor_platform_info->i2c_conf &&
		sinfo->sensor_platform_info->i2c_conf->use_i2c_mux)
		sony_sensor_disable_i2c_mux(
			sinfo->sensor_platform_info->i2c_conf);

	rc = sony_util_power_ctrl(s_ctrl, &camera_data[id], false);
	if (rc < 0)
		LOGE("power_down fail\n");

	rc = msm_camera_request_gpio_table(data, 0);

	return rc;
}

static int32_t sony_sensor_match_id(struct msm_sensor_ctrl_t *s_ctrl)
{
	return 0;
}

static void sony_sensor_adjust_frame_lines(struct msm_sensor_ctrl_t *s_ctrl)
{
	return;
}

static int32_t sony_sensor_get_csi_params(struct msm_sensor_ctrl_t *s_ctrl,
		struct csi_lane_params_t *sensor_output_info)
{
	uint8_t index;
	struct msm_camera_csi_lane_params *csi_lane_params =
		s_ctrl->sensordata->sensor_platform_info->csi_lane_params;
	if (csi_lane_params) {
		sensor_output_info->csi_lane_assign = csi_lane_params->
			csi_lane_assign;
		sensor_output_info->csi_lane_mask = csi_lane_params->
			csi_lane_mask;
		sensor_output_info->csi_phy_sel = csi_lane_params->csi_phy_sel;
	}
	sensor_output_info->csi_if = s_ctrl->sensordata->csi_if;
	for (index = 0; index < sensor_output_info->csi_if; index++)
		sensor_output_info->csid_core[index] = s_ctrl->sensordata->
			pdata[index].csid_core;

	return 0;
}
static int sony_sensor_set_vision_mode(struct msm_sensor_ctrl_t *s_ctrl,
		int32_t vision_mode_enable)
{
	return 0;
}
static int sony_sensor_set_vision_ae_control(struct msm_sensor_ctrl_t *s_ctrl,
		int ae_mode)
{
	return 0;
}
static int32_t sony_sensor_i2c_probe(struct i2c_client *client,
	const struct i2c_device_id *device_id)
{
	int rc = 0;
	uint16_t id = 0;
	struct msm_sensor_ctrl_t *s_ctrl;

	LOGV("%s_probe called\n", client->name);

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		LOGE("i2c_check_functionality failed\n");
		rc = -EFAULT;
		return rc;
	}

	s_ctrl = (struct msm_sensor_ctrl_t *)(device_id->driver_data);

	if (s_ctrl->sensor_i2c_client != NULL) {
		s_ctrl->sensor_i2c_client->client = client;
		if (s_ctrl->sensor_i2c_addr != 0)
			s_ctrl->sensor_i2c_client->client->addr =
				s_ctrl->sensor_i2c_addr;
	} else {
		rc = -EFAULT;
		return rc;
	}

	s_ctrl->sensordata = client->dev.platform_data;
	id = sony_util_get_context(s_ctrl);

	if (!camera_info[id].i2c_addr) {
		LOGD("%s is not found\n", client->name);
		rc = -ENODEV;
		return rc;
	}

	camera_data[id].d = &s_ctrl->sensor_i2c_client->client->dev;

	rc = s_ctrl->func_tbl->sensor_power_up(s_ctrl);
	if (rc < 0)
		goto probe_fail;

	rc = sony_eeprom_load(s_ctrl);
	if (rc < 0) {
		LOGE("eeprom load fail\n");
		goto probe_fail;
	}

	s_ctrl->func_tbl->sensor_power_down(s_ctrl);
	s_ctrl->sensor_state = MSM_SENSOR_POWER_DOWN;

	s_ctrl->sensor_v4l2_subdev_info->code =
		camera_info[id].subdev_code;

	snprintf(s_ctrl->sensor_v4l2_subdev.name,
		sizeof(s_ctrl->sensor_v4l2_subdev.name), "%s", device_id->name);
	v4l2_i2c_subdev_init(&s_ctrl->sensor_v4l2_subdev, client,
		s_ctrl->sensor_v4l2_subdev_ops);
	s_ctrl->sensor_v4l2_subdev.flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
	media_entity_init(&s_ctrl->sensor_v4l2_subdev.entity, 0, NULL, 0);
	s_ctrl->sensor_v4l2_subdev.entity.type = MEDIA_ENT_T_V4L2_SUBDEV;
	s_ctrl->sensor_v4l2_subdev.entity.group_id = SENSOR_DEV;
	s_ctrl->sensor_v4l2_subdev.entity.name =
		s_ctrl->sensor_v4l2_subdev.name;

	msm_sensor_register(&s_ctrl->sensor_v4l2_subdev);
	s_ctrl->sensor_v4l2_subdev.entity.revision =
		s_ctrl->sensor_v4l2_subdev.devnode->num;
	camera_data[id].probe_done = true;
	LOGD("probe ok\n");
	return rc;

probe_fail:
	LOGE("%s_i2c_probe failed\n", client->name);
	s_ctrl->func_tbl->sensor_power_down(s_ctrl);
	s_ctrl->sensor_state = MSM_SENSOR_POWER_DOWN;
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
	/* more can be supported, to be added later */
};

static struct msm_sensor_output_info_t sony_sensor_dimensions[] = {
	{
		.x_output		= 0x0000,
		.y_output		= 0x0000,
		.line_length_pclk	= 0x0000,
		.frame_length_lines	= 0x0000,
		.vt_pixel_clk		= 228570000,
		.op_pixel_clk		= 320000000,
		.binning_factor		= 1,
	},
	{
		.x_output		= 0x0000,
		.y_output		= 0x0000,
		.line_length_pclk	= 0x0000,
		.frame_length_lines	= 0x0000,
		.vt_pixel_clk		= 228570000,
		.op_pixel_clk		= 228570000,
		.binning_factor		= 2,
	},
	{
		.x_output		= 0x0000,
		.y_output		= 0x0000,
		.line_length_pclk	= 0x0000,
		.frame_length_lines	= 0x0000,
		.vt_pixel_clk		= 228570000,
		.op_pixel_clk		= 228570000,
		.binning_factor		= 3,
	},
	{
		.x_output		= 0x0000,
		.y_output		= 0x0000,
		.line_length_pclk	= 0x0000,
		.frame_length_lines	= 0x0000,
		.vt_pixel_clk		= 228570000,
		.op_pixel_clk		= 228570000,
		.binning_factor		= 4,
	},
	{
		.x_output		= 0x0000,
		.y_output		= 0x0000,
		.line_length_pclk	= 0x0000,
		.frame_length_lines	= 0x0000,
		.vt_pixel_clk		= 228570000,
		.op_pixel_clk		= 228570000,
		.binning_factor		= 5,
	},
	{
		.x_output		= 0x0000,
		.y_output		= 0x0000,
		.line_length_pclk	= 0x0000,
		.frame_length_lines	= 0x0000,
		.vt_pixel_clk		= 228570000,
		.op_pixel_clk		= 228570000,
		.binning_factor		= 6,
	},
};

static const struct i2c_device_id sony_camera_0_i2c_id[] = {
	{ "sony_camera_0", (kernel_ulong_t)&camera_data[0].s_ctrl },
	{ },
};

static const struct i2c_device_id sony_camera_1_i2c_id[] = {
	{ "sony_camera_1", (kernel_ulong_t)&camera_data[1].s_ctrl },
	{ },
};

static struct i2c_driver sony_sensor_i2c_driver[] = {
	{
		.id_table	= sony_camera_0_i2c_id,
		.probe		= sony_sensor_i2c_probe,
		.driver = {
			.name	= "sony_camera_0",
		},
	},
	{
		.id_table	= sony_camera_1_i2c_id,
		.probe		= sony_sensor_i2c_probe,
		.driver = {
			.name	= "sony_camera_1",
		},
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

static int __init msm_sensor_init_module(void)
{
	int rc = 0;
	uint16_t i;

	sensor_num = ARRAY_SIZE(sony_sensor_i2c_driver);
	for (i = 0; i < sensor_num; i++)
		rc = i2c_add_driver(&sony_sensor_i2c_driver[i]);

	return rc;
}

static struct v4l2_subdev_core_ops sony_sensor_subdev_core_ops = {
	.ioctl		= msm_sensor_subdev_ioctl,
	.s_power	= msm_sensor_power,
};

static struct v4l2_subdev_video_ops sony_sensor_subdev_video_ops = {
	.enum_mbus_fmt	= msm_sensor_v4l2_enum_fmt,
};

static struct v4l2_subdev_ops sony_sensor_subdev_ops = {
	.core	= &sony_sensor_subdev_core_ops,
	.video	= &sony_sensor_subdev_video_ops,
};

static struct msm_sensor_fn_t sony_sensor_func_tbl = {
	.sensor_start_stream		= sony_sensor_start_stream,
	.sensor_stop_stream		= sony_sensor_stop_stream,
	.sensor_group_hold_on		= sony_sensor_group_hold_on,
	.sensor_group_hold_off		= sony_sensor_group_hold_off,
	.sensor_set_fps			= sony_sensor_set_fps,
	.sensor_write_exp_gain		= sony_sensor_write_exp_gain1,
	.sensor_write_snapshot_exp_gain = sony_sensor_write_exp_gain1,
	.sensor_setting			= sony_sensor_setting,
	.sensor_csi_setting		= sony_sensor_setting1,
	.sensor_set_sensor_mode		= sony_sensor_set_sensor_mode,
	.sensor_mode_init		= sony_sensor_mode_init,
	.sensor_get_output_info		= sony_sensor_get_output_info,
	.sensor_config			= sony_sensor_config,
	.sensor_power_up		= sony_sensor_power_up,
	.sensor_power_down		= sony_sensor_power_down,
	.sensor_match_id		= sony_sensor_match_id,
	.sensor_adjust_frame_lines	= sony_sensor_adjust_frame_lines,
	.sensor_get_csi_params		= sony_sensor_get_csi_params,
	.sensor_set_vision_mode		= sony_sensor_set_vision_mode,
	.sensor_set_vision_ae_control	= sony_sensor_set_vision_ae_control,
};

static struct msm_sensor_reg_t sony_sensor_regs[] = {
	{
		.default_data_type		= MSM_CAMERA_I2C_BYTE_DATA,
		.start_stream_conf		= NULL,
		.start_stream_conf_size		= 0,
		.stop_stream_conf		= NULL,
		.stop_stream_conf_size		= 0,
		.group_hold_on_conf		= NULL,
		.group_hold_on_conf_size	= 0,
		.group_hold_off_conf		= NULL,
		.group_hold_off_conf_size	= 0,
		.init_settings			= NULL,
		.init_size			= 0,
		.mode_settings			= NULL,
		.output_settings		= sony_sensor_dimensions,
		.num_conf			= 0,
	},
	{
		.default_data_type		= MSM_CAMERA_I2C_BYTE_DATA,
		.start_stream_conf		= NULL,
		.start_stream_conf_size		= 0,
		.stop_stream_conf		= NULL,
		.stop_stream_conf_size		= 0,
		.group_hold_on_conf		= NULL,
		.group_hold_on_conf_size	= 0,
		.group_hold_off_conf		= NULL,
		.group_hold_off_conf_size	= 0,
		.init_settings			= NULL,
		.init_size			= 0,
		.mode_settings			= NULL,
		.output_settings		= sony_sensor_dimensions,
		.num_conf			= 0,
	},
};

static struct sony_camera_data camera_data[] = {
	{
		.s_ctrl = {
			.msm_sensor_reg			= &sony_sensor_regs[0],
			.sensor_i2c_client		=
					&sony_sensor_i2c_client[0],
			.sensor_i2c_addr		= 0x00,
			.sensor_output_reg_addr		= NULL,
			.sensor_id_info			= NULL,
			.sensor_exp_gain_info		= NULL,
			.cam_mode			=
					MSM_SENSOR_MODE_INVALID,
			.msm_sensor_mutex		= &sensor_mut,
			.sensor_i2c_driver		=
					&sony_sensor_i2c_driver[0],
			.sensor_v4l2_subdev_info	=
					&sony_sensor_subdev_info[0],
			.sensor_v4l2_subdev_info_size	=
					ARRAY_SIZE(sony_sensor_subdev_info),
			.sensor_v4l2_subdev_ops		=
					&sony_sensor_subdev_ops,
			.func_tbl			= &sony_sensor_func_tbl,
			.clk_rate			= 8000000,
		},
	},
	{
		.s_ctrl = {
			.msm_sensor_reg			= &sony_sensor_regs[1],
			.sensor_i2c_client		=
					&sony_sensor_i2c_client[1],
			.sensor_i2c_addr		= 0x00,
			.sensor_output_reg_addr		= NULL,
			.sensor_id_info			= NULL,
			.sensor_exp_gain_info		= NULL,
			.cam_mode			=
					MSM_SENSOR_MODE_INVALID,
			.msm_sensor_mutex		= &sensor_mut,
			.sensor_i2c_driver		=
					&sony_sensor_i2c_driver[1],
			.sensor_v4l2_subdev_info	=
					&sony_sensor_subdev_info[1],
			.sensor_v4l2_subdev_info_size	=
					ARRAY_SIZE(sony_sensor_subdev_info),
			.sensor_v4l2_subdev_ops		=
					&sony_sensor_subdev_ops,
			.func_tbl			= &sony_sensor_func_tbl,
			.clk_rate			= 8000000,
		},
	},
};

late_initcall(msm_sensor_init_module);

MODULE_DESCRIPTION("SONY V4L2 camera sensor driver");
MODULE_LICENSE("GPL v2");
