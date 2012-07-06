/* drivers/media/video/msm/semc_sensor_sub.c
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
/* #define ENABLE_LOGD */
/* #define ENABLE_LOGV */
/* #define ENABLE_DUMP */

#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/regulator/consumer.h>
#include <media/msm_camera.h>
#include <mach/gpio.h>
#include <mach/camera.h>

#ifdef ENABLE_LOGE
#define LOGE(dev, f, a...)	dev_dbg(dev, "CAM1 E: %s: " f, __func__, ##a)
#else
#define LOGE(dev, f, a...)
#endif

#ifdef ENABLE_LOGD
#define LOGD(dev, f, a...)	dev_dbg(dev, "CAM1 D: %s: " f, __func__, ##a)
#else
#define LOGD(dev, f, a...)
#endif

#ifdef ENABLE_LOGV
#define LOGV(dev, f, a...)	dev_dbg(dev, "CAM1 V: %s: " f, __func__, ##a)
#else
#define LOGV(dev, f, a...)
#endif

#define SAME(x, y)		(strncmp(x, y, 8) == 0)

#define SENSOR_ID_MT9M114	0x2481
#define ROM_READ_MAX_LOOP	100
#define ROM_SLAVE_ADDR		0x48
#define ROM_MAX_DATA_LEN	16
#define I2C_MAX_DATA_LEN	256

#define MODULE_STW01BM0		"STW01BM0"
#define MODULE_APT01BM0		"APT01BM0"

#define VREG_L1			"8058_l1"
#define VREG_LVS0		"8058_lvs0"
#define VREG_L15		"8058_l15"
#define VREG_L9			"8058_l9"

enum semc_cam_cmd {
	VREG_SD,
	VREG_IO,
	VREG_SA,
	VREG_AF,
	GPIO,
	MCLK,
	EXIT,
};

struct semc_cam_seq {
	bool				valid;
	enum semc_cam_cmd		cmd;
	int				val;
	int				wait;
};

struct semc_cam_ctrl {
	bool					opened;
	bool					romloaded;
	uint8_t					rom[ROM_MAX_DATA_LEN];
	uint8_t					buf[I2C_MAX_DATA_LEN];
	struct mutex				lock;
	const struct msm_camera_sensor_info	*inf;
	struct i2c_client			*i2c_client;
	struct device				*d;
	struct regulator			*vreg_io;
	struct regulator			*vreg_sa;
	struct regulator			*vreg_sd;
	struct regulator			*vreg_af;
};

static struct semc_cam_ctrl *semc_cam_ctrl;

static void dump(uint8_t *buf, int len)
{
#ifdef ENABLE_DUMP
	int i;
	uint8_t s[I2C_MAX_DATA_LEN * 2];
	int p = 0;

	for (i = 0; i < len; i++)
		p += snprintf(s + p, sizeof(s) - p, "%02x ", buf[i]);
	LOGV(semc_cam_ctrl->d, "[len:%d] %s\n", len, s);
#endif
}

static int semc_cam_gpio_set(int gpio_pin, int value)
{
	LOGD(semc_cam_ctrl->d, "GPIO[%d] = %d\n", gpio_pin, value);

	gpio_set_value_cansleep(gpio_pin, value);

	return 0;
}

static int semc_cam_vreg_set(enum semc_cam_cmd cmd, int level)
{
	int rc = 0;
	struct regulator *vreg;
	LOGD(semc_cam_ctrl->d, "VREG[%d] = %d\n", cmd, level);

	if (cmd == VREG_SD) {
		if (semc_cam_ctrl->vreg_sd) {
			vreg = semc_cam_ctrl->vreg_sd;
		} else {
			vreg = regulator_get(0, VREG_L1);
			semc_cam_ctrl->vreg_sd = vreg;
		}
	} else if (cmd == VREG_IO) {
		if (semc_cam_ctrl->vreg_io) {
			vreg = semc_cam_ctrl->vreg_io;
		} else {
			vreg = regulator_get(0, VREG_LVS0);
			semc_cam_ctrl->vreg_io = vreg;
		}
	} else if (cmd == VREG_SA) {
		if (semc_cam_ctrl->vreg_sa) {
			vreg = semc_cam_ctrl->vreg_sa;
		} else {
			vreg = regulator_get(0, VREG_L15);
			semc_cam_ctrl->vreg_sa = vreg;
		}
	} else if (cmd == VREG_AF) {
		if (semc_cam_ctrl->vreg_af) {
			vreg = semc_cam_ctrl->vreg_af;
		} else {
			vreg = regulator_get(0, VREG_L9);
			semc_cam_ctrl->vreg_af = vreg;
		}
	} else {
		rc = -EINVAL;
		LOGE(semc_cam_ctrl->d, "invalid resource\n");
		goto exit;
	}

	level *= 1000;
	if (level >= 0) {
		if (level > 0)
			rc = regulator_set_voltage(vreg, level, level);
		if (!IS_ERR_VALUE(rc)) {
			rc = regulator_enable(vreg);
			if (IS_ERR_VALUE(rc))
				regulator_disable(vreg);
		}
		if (IS_ERR_VALUE(rc))
			regulator_put(vreg);
	} else {
		rc = regulator_disable(vreg);
		regulator_put(vreg);
	}
	if (IS_ERR_VALUE(rc) || level < 0) {
		if (vreg == semc_cam_ctrl->vreg_sd)
			semc_cam_ctrl->vreg_sd = NULL;
		else if (vreg == semc_cam_ctrl->vreg_io)
			semc_cam_ctrl->vreg_io = NULL;
		else if (vreg == semc_cam_ctrl->vreg_sa)
			semc_cam_ctrl->vreg_sa = NULL;
		else if (vreg == semc_cam_ctrl->vreg_af)
			semc_cam_ctrl->vreg_af = NULL;
	}

exit:
	if (IS_ERR_VALUE(rc))
		LOGE(semc_cam_ctrl->d, "error happened (%d)\n", rc);
	return rc;
}

static int semc_cam_mclk_set(uint32_t *mclk, int value)
{
	int rc = 0;
	LOGD(semc_cam_ctrl->d, "MCLK[%d] = %d\n", *mclk, value);

	if (value >= 0) {
		if (value > 0 && mclk)
			*mclk = value;
		rc = msm_camio_clk_enable(CAMIO_CAM_MCLK_CLK);
	} else {
		rc = msm_camio_clk_disable(CAMIO_CAM_MCLK_CLK);
	}

	if (IS_ERR_VALUE(rc))
		LOGE(semc_cam_ctrl->d, "error happened (%d)\n", rc);
	return rc;
}

static int semc_cam_i2c_read(struct semc_cam_ctrl *ctrl, uint8_t slave_addr,
	uint32_t addr, uint8_t type, uint16_t len, uint8_t *data)
{
	int rc = 0;
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
			.buf	= ctrl->buf,
		},
	};
	struct i2c_msg *pmsg = msgs;
	uint8_t msglen = 2;
	LOGV(semc_cam_ctrl->d, "addr:0x%04x, type:0x%02x, len:0x%02x\n",
		addr, type, len);

	if (!data || !len || len > I2C_MAX_DATA_LEN) {
		rc = -EPERM;
		LOGE(semc_cam_ctrl->d, "invalid parameter\n");
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
		LOGE(semc_cam_ctrl->d, "invalid case\n");
		goto exit;
	}

	rc = i2c_transfer(ctrl->i2c_client->adapter, pmsg, msglen);
	if (rc < 0 || rc != msglen) {
		rc = -EIO;
		LOGE(semc_cam_ctrl->d, "i2c transfer failed\n");
		goto exit;
	}
	memcpy(data, ctrl->buf, len);
	dump(ctrl->buf, len);
	rc = 0;

exit:
	if (IS_ERR_VALUE(rc))
		LOGE(semc_cam_ctrl->d, "error happened (%d)\n", rc);
	return rc;
}

static int semc_cam_i2c_write(struct semc_cam_ctrl *ctrl, uint8_t slave_addr,
	uint32_t addr, uint8_t type, uint16_t len, uint8_t *data)
{
	int rc = 0;
	struct i2c_msg msgs[] = {
		{
			.addr	= slave_addr,
			.flags	= 0,
			.len	= len + type,
			.buf	= ctrl->buf,
		},
	};
	struct i2c_msg *pmsg = msgs;
	uint8_t msglen = 1;
	LOGV(semc_cam_ctrl->d, "addr:0x%04x, type:0x%02x, len:0x%02x\n",
		addr, type, len);

	if (!data || !len || len + type > I2C_MAX_DATA_LEN) {
		rc = -EPERM;
		LOGE(semc_cam_ctrl->d, "invalid parameter\n");
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
		LOGE(semc_cam_ctrl->d, "invalid case\n");
		goto exit;
	}

	memcpy(ctrl->buf + type, data, len);
	rc = i2c_transfer(ctrl->i2c_client->adapter, pmsg, msglen);
	if (rc < 0 || rc != msglen) {
		rc = -EIO;
		LOGE(semc_cam_ctrl->d, "i2c transfer failed\n");
		goto exit;
	}
	dump(ctrl->buf, len + type);
	rc = 0;

exit:
	if (IS_ERR_VALUE(rc))
		LOGE(semc_cam_ctrl->d, "error happened (%d)\n", rc);
	return rc;
}

static int semc_cam_power_set(struct semc_cam_ctrl *ctrl, bool on)
{
	int rc = 0;
	struct semc_cam_seq seq_off[] = {
		{ true, MCLK, -1, 1 },
		{ true, GPIO, 0, 1 },
		{ true, VREG_SA, -1, 0 },
		{ true, VREG_IO, -1, 0 },
		{ true, VREG_SD, -1, 0 },
		{ true, VREG_AF, -1, 10 },
		{ true, EXIT, 0, 0 },
	};
	struct semc_cam_seq seq_on[] = {
		{ true, VREG_AF, 2800, 1 },
		{ true, VREG_SD, 1200, 1 },
		{ true, VREG_IO, 0, 1 },
		{ true, VREG_SA, 2800, 1 },
		{ true, MCLK, 0, 1 },
		{ true, GPIO, 1, 150 },
		{ true, EXIT, 0, 0 },
	};

	struct semc_cam_seq *seq = on ? seq_on : seq_off;
	struct msm_camera_device_platform_data *pdata =
		(struct msm_camera_device_platform_data *)&ctrl->inf->pdata;

	LOGV(semc_cam_ctrl->d, "begin\n");

	while (seq->cmd != EXIT) {
		if (!seq->valid) {
			seq++;
			continue;
		}
		switch (seq->cmd) {
		case GPIO:
			rc = semc_cam_gpio_set(ctrl->inf->sensor_reset,
				seq->val);
			break;
		case VREG_SD:
		case VREG_IO:
		case VREG_SA:
		case VREG_AF:
			rc = semc_cam_vreg_set(seq->cmd, seq->val);
			break;
		case MCLK:
			rc = semc_cam_mclk_set(&pdata->ioclk.mclk_clk_rate,
				seq->val);
			break;
		default:
			goto exit;
		}
		mdelay(seq->wait);
		if (on && IS_ERR_VALUE(rc)) {
			LOGE(semc_cam_ctrl->d, "seq:%d failed (%d)\n",
				seq->cmd, rc);
			goto exit;
		}
		seq++;
	}

exit:
	if (IS_ERR_VALUE(rc))
		LOGE(semc_cam_ctrl->d, "error happened (%d)\n", rc);
	LOGV(semc_cam_ctrl->d, "end (%d)\n", rc);
	return rc;
}

static int semc_cam_rom_load(struct semc_cam_ctrl *ctrl)
{
	int rc = 0;
	uint8_t *data = ctrl->rom;
	uint32_t i;
	LOGV(semc_cam_ctrl->d, "begin\n");

	rc = semc_cam_power_set(ctrl, true);
	if (IS_ERR_VALUE(rc))
		goto exit;

	rc = semc_cam_i2c_read(ctrl, ROM_SLAVE_ADDR, 0x0000,
		SENSOR_I2C_ADDR_2BYTE, 2, data);
	if (IS_ERR_VALUE(rc)) {
		LOGE(semc_cam_ctrl->d, "cannot load\n");
		goto exit;
	}

	if (((uint16_t)data[0] << 8 | data[1]) == SENSOR_ID_MT9M114) {
		data[0] = 0x00;
		data[1] = 0x55;
		rc = semc_cam_i2c_write(ctrl, ROM_SLAVE_ADDR, 0x3052,
			SENSOR_I2C_ADDR_2BYTE, 2, data);
		if (IS_ERR_VALUE(rc))
			goto exit;

		data[0] = 0x00;
		data[1] = 0x10;
		rc = semc_cam_i2c_write(ctrl, ROM_SLAVE_ADDR, 0x3050,
			SENSOR_I2C_ADDR_2BYTE, 2, data);
		if (IS_ERR_VALUE(rc))
			goto exit;

		i = 0;
		do {
			usleep_range(1000, 2000);
			i++;
			rc = semc_cam_i2c_read(ctrl, ROM_SLAVE_ADDR, 0x3050,
				SENSOR_I2C_ADDR_2BYTE, 2, data);
			if (IS_ERR_VALUE(rc))
				goto exit;
		} while (!(data[1] & 0x20) && i < ROM_READ_MAX_LOOP);

		if (i >= ROM_READ_MAX_LOOP)
			goto exit;

		rc = semc_cam_i2c_read(ctrl, ROM_SLAVE_ADDR, 0x313A,
			SENSOR_I2C_ADDR_2BYTE, 2, data + 8);
		if (IS_ERR_VALUE(rc))
			goto exit;

		rc = semc_cam_i2c_read(ctrl, ROM_SLAVE_ADDR, 0x313C,
			SENSOR_I2C_ADDR_2BYTE, 2, data + 10);
		if (IS_ERR_VALUE(rc))
			goto exit;

		if ((data[8] & 0xC0) == 0x00)
			memcpy(data, MODULE_STW01BM0, 8);
		else
			memcpy(data, MODULE_APT01BM0, 8);
	}

	LOGD(ctrl->d, "MODULE NAME: %c%c%c%c%c%c%c%c\n",
		data[0], data[1], data[2], data[3],
		data[4], data[5], data[6], data[7]);

	ctrl->romloaded = true;

exit:
	(void)semc_cam_power_set(ctrl, false);
	if (IS_ERR_VALUE(rc))
		LOGE(semc_cam_ctrl->d, "error happened (%d)\n", rc);
	LOGV(semc_cam_ctrl->d, "end\n");
	return rc;
}

static int semc_cam_ctrl_open(const struct msm_camera_sensor_info *inf)
{
	int rc = 0;
	struct semc_cam_ctrl *ctrl = semc_cam_ctrl;

	LOGD(semc_cam_ctrl->d, "begin\n");
	mutex_lock(&ctrl->lock);

	if (ctrl->opened) {
		LOGE(semc_cam_ctrl->d, "already opened\n");
		rc = -EBUSY;
		goto exit;
	}

	rc = semc_cam_power_set(ctrl, true);
	if (IS_ERR_VALUE(rc)) {
		(void)semc_cam_power_set(ctrl, false);
		goto exit;
	}

	ctrl->opened = true;

exit:
	mutex_unlock(&ctrl->lock);
	if (IS_ERR_VALUE(rc))
		LOGE(semc_cam_ctrl->d, "error happened (%d)\n", rc);
	LOGD(semc_cam_ctrl->d, "end\n");
	return rc;
}

static int semc_cam_ctrl_close(void)
{
	struct semc_cam_ctrl *ctrl = semc_cam_ctrl;

	LOGD(semc_cam_ctrl->d, "begin\n");
	mutex_lock(&ctrl->lock);

	if (ctrl->opened) {
		(void)semc_cam_power_set(ctrl, false);
		ctrl->opened = false;
	}

	mutex_unlock(&ctrl->lock);
	LOGD(semc_cam_ctrl->d, "end\n");
	return 0;
}

static int semc_cam_ctrl_config(void __user *argp)
{
	int rc = 0;
	struct semc_cam_ctrl *ctrl = semc_cam_ctrl;
	struct sensor_cfg_data cdata;
	uint8_t slave_addr;
	uint32_t addr;
	uint8_t type;
	uint8_t length;
	uint8_t data[I2C_MAX_DATA_LEN];
	uint16_t rom_offset;
	uint16_t rom_len;
	struct msm_camera_csi_params csi_params;

	mutex_lock(&ctrl->lock);

	if (!ctrl->opened) {
		rc = -ENODEV;
		LOGE(semc_cam_ctrl->d, "not opened\n");
		goto exit;
	}

	if (copy_from_user(&cdata, (void *)argp, sizeof(cdata))) {
		rc = -EFAULT;
		LOGE(semc_cam_ctrl->d, "copy failed\n");
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
			LOGE(semc_cam_ctrl->d, "parameter failed\n");
			goto exit;
		}
		memset(data, 0, sizeof(data));
		if (copy_from_user(data, cdata.cfg.i2c_io.data, length)) {
			rc = -EFAULT;
			LOGE(semc_cam_ctrl->d, "copy failed\n");
			goto exit;
		}
		rc = semc_cam_i2c_write(ctrl,
			slave_addr, addr, type, length, data);
		if (IS_ERR_VALUE(rc))
			goto exit;
		break;
	case CFG_SET_READ_CMD:
		slave_addr = cdata.cfg.i2c_io.slave_addr;
		addr = cdata.cfg.i2c_io.address;
		type = cdata.cfg.i2c_io.address_type;
		length = cdata.cfg.i2c_io.length;
		if (!length || length > I2C_MAX_DATA_LEN) {
			rc = -EPERM;
			LOGE(semc_cam_ctrl->d, "parameter failed\n");
			goto exit;
		}
		memset(data, 0, sizeof(data));
		rc = semc_cam_i2c_read(ctrl,
			slave_addr, addr, type, length, data);
		if (IS_ERR_VALUE(rc))
			goto exit;
		if (copy_to_user(cdata.cfg.i2c_io.data, data, length)) {
			rc = -EFAULT;
			LOGE(semc_cam_ctrl->d, "copy failed\n");
			goto exit;
		}
		break;
	case CFG_SET_GPIO_CTRL:
		if (cdata.cfg.gpio_ctrl.gpio == SENSOR_GPIO_CTRL_RESET) {
			rc = semc_cam_gpio_set(
				ctrl->inf->sensor_reset,
				cdata.cfg.gpio_ctrl.value);
		} else {
			rc = -EINVAL;
		}
		if (IS_ERR_VALUE(rc))
			goto exit;
		break;
	case CFG_SET_CSI_CTRL:
		csi_params.lane_cnt    =
			cdata.cfg.csi_ctrl.csid_params.lane_cnt;
		csi_params.lane_assign =
			cdata.cfg.csi_ctrl.csid_params.lane_assign;
		csi_params.dpcm_scheme = 0;
		csi_params.settle_cnt  =
			cdata.cfg.csi_ctrl.csiphy_params.settle_cnt;

		if (SENSOR_CSI_DECODE_8BIT ==
			cdata.cfg.csi_ctrl.csid_params.lut_params.num_cid)
			csi_params.data_format = CSI_8BIT;
		else if (SENSOR_CSI_DECODE_10BIT ==
			cdata.cfg.csi_ctrl.csid_params.lut_params.num_cid)
			csi_params.data_format = CSI_10BIT;
		else
			csi_params.data_format = CSI_10BIT;

		rc = msm_camio_csi_config(&csi_params);
		if (IS_ERR_VALUE(rc))
			goto exit;
		break;
	case CFG_GET_ROM:
		rom_offset = cdata.cfg.rom_in.address;
		rom_len = cdata.cfg.rom_in.length;
		if (!rom_len || rom_offset >= ROM_MAX_DATA_LEN) {
			rc = -EPERM;
			LOGE(semc_cam_ctrl->d, "parameter failed\n");
			goto exit;
		}
		if (rom_offset + rom_len > ROM_MAX_DATA_LEN)
			rom_len = ROM_MAX_DATA_LEN - rom_offset;
		if (copy_to_user(cdata.cfg.rom_in.data,
			ctrl->rom + rom_offset, rom_len)) {
			rc = -EFAULT;
			LOGE(semc_cam_ctrl->d, "copy failed\n");
			goto exit;
		}
		break;
	default:
		rc = -EINVAL;
		LOGE(semc_cam_ctrl->d, "invalid case\n");
		goto exit;
	}

	if (copy_to_user((void *)argp, &cdata, sizeof(cdata))) {
		rc = -EFAULT;
		LOGE(semc_cam_ctrl->d, "copy failed\n");
		goto exit;
	}

exit:
	mutex_unlock(&ctrl->lock);
	if (IS_ERR_VALUE(rc))
		LOGE(semc_cam_ctrl->d, "error happened (%d)\n", rc);

	return rc;
}

static int semc_cam_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int rc = 0;
	LOGD(&client->dev, "begin\n");

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		rc = -ENOTSUPP;
		goto exit;
	}

	semc_cam_ctrl = kzalloc(sizeof(struct semc_cam_ctrl), GFP_KERNEL);
	if (!semc_cam_ctrl) {
		rc = -ENOMEM;
		goto exit;
	}

	mutex_init(&semc_cam_ctrl->lock);
	semc_cam_ctrl->i2c_client = client;
	semc_cam_ctrl->d = &client->dev;

exit:
	if (IS_ERR_VALUE(rc))
		LOGE(semc_cam_ctrl->d, "error happened (%d)\n", rc);
	LOGD(&client->dev, "end\n");
	return rc;
}

static int __exit semc_cam_remove(struct i2c_client *client)
{
	mutex_destroy(&semc_cam_ctrl->lock);
	kfree(semc_cam_ctrl);
	semc_cam_ctrl = NULL;
	return 0;
}

static const struct i2c_device_id semc_cam_id[] = {
	{ "semc_sensor_sub", 0 },
	{ }
};

static struct i2c_driver semc_cam_driver = {
	.id_table	= semc_cam_id,
	.probe		= semc_cam_probe,
	.remove		= __exit_p(semc_cam_remove),
	.driver		= {
		.name	= "semc_sensor_sub",
	},
};

static int msm_camera_probe(const struct msm_camera_sensor_info *inf,
	struct msm_sensor_ctrl *s)
{
	int rc = 0;

	rc = i2c_add_driver(&semc_cam_driver);
	if (IS_ERR_VALUE(rc))
		goto exit;

	semc_cam_ctrl->inf = inf;
	s->s_init = semc_cam_ctrl_open;
	s->s_release = semc_cam_ctrl_close;
	s->s_config = semc_cam_ctrl_config;

	rc = semc_cam_rom_load(semc_cam_ctrl);
	if (IS_ERR_VALUE(rc))
		i2c_del_driver(&semc_cam_driver);

exit:
	return rc;
}

static int semc_camera_module_probe(struct platform_device *pdev)
{
	return msm_camera_drv_start(pdev, &msm_camera_probe);
}

static struct platform_driver semc_camera_module_driver = {
	.probe		= semc_camera_module_probe,
	.driver		= {
		.name	= "msm_camera_semc_sensor_sub",
		.owner	= THIS_MODULE,
	},
};

static int __init semc_sensor_sub_init(void)
{
	return platform_driver_register(&semc_camera_module_driver);
}

late_initcall(semc_sensor_sub_init);

MODULE_DESCRIPTION("SEMC sub camera sensor driver");
MODULE_LICENSE("GPL v2");
