/*
 * OmniVision OV7695 sensor driver
 *
 * This program is free software; you can redistribute it and/or
 *modify it under the terms of the GNU General Public License as
 *published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 *kind, whether express or implied; without even the implied warranty
 *of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/module.h>
#include <linux/videodev2.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/log2.h>
#include <linux/delay.h>

#include <media/v4l2-subdev.h>
#include <media/v4l2-chip-ident.h>
#include <media/soc_camera.h>
#include <linux/videodev2_brcm.h>

#include <linux/gpio.h>

#include "ov7695.h"

/* OV7695 has only one fixed colorspace per pixelcode */
struct ov7695_datafmt {
	enum v4l2_mbus_pixelcode code;
	enum v4l2_colorspace colorspace;
};

struct ov7695_timing_cfg {
	u16 x_addr_start;
	u16 y_addr_start;
	u16 x_addr_end;
	u16 y_addr_end;
	u16 h_output_size;
	u16 v_output_size;
	u16 h_total_size;
	u16 v_total_size;
	u16 isp_h_offset;
	u16 isp_v_offset;
	u8 h_odd_ss_inc;
	u8 h_even_ss_inc;
	u8 v_odd_ss_inc;
	u8 v_even_ss_inc;
	u8 out_mode_sel;
	u8 sclk_dividers;
	u8 sys_mipi_clk;

};

static const struct ov7695_datafmt ov7695_fmts[] = {
	/*
	 * Order important: first natively supported,
	 *second supported with a GPIO extender
	 */
	{V4L2_MBUS_FMT_UYVY8_2X8, V4L2_COLORSPACE_JPEG},
	{V4L2_MBUS_FMT_YUYV8_2X8, V4L2_COLORSPACE_JPEG},
};

enum ov7695_size {
	OV7695_SIZE_QVGA,	/*  320 x 240 */
	OV7695_SIZE_VGA,	/*  640 x 480 */
	OV7695_SIZE_LAST,
	OV7695_SIZE_MAX
};

static const struct v4l2_frmsize_discrete ov7695_frmsizes[OV7695_SIZE_LAST] = {
	{320, 240},
	{640, 480},
};


/* Find a data format by a pixel code in an array */
static int ov7695_find_datafmt(enum v4l2_mbus_pixelcode code)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(ov7695_fmts); i++)
		if (ov7695_fmts[i].code == code)
			break;

	/* If not found, select latest */
	if (i >= ARRAY_SIZE(ov7695_fmts))
		i = ARRAY_SIZE(ov7695_fmts) - 1;

	return i;
}

/* Find a frame size in an array */
static int ov7695_find_framesize(u32 width, u32 height)
{
	int i;

	for (i = 0; i < OV7695_SIZE_LAST; i++) {
		if ((ov7695_frmsizes[i].width >= width) &&
		    (ov7695_frmsizes[i].height >= height))
			break;
	}

	/* If not found, select biggest */
	if (i >= OV7695_SIZE_LAST)
		i = OV7695_SIZE_LAST - 1;

	return i;
}

struct ov7695 {
	struct v4l2_subdev subdev;
	struct v4l2_subdev_sensor_interface_parms *plat_parms;
	struct v4l2_ctrl_handler hdl;
	int i_size;
	int i_fmt;
	int brightness;
	int contrast;
	int colorlevel;
	int sharpness;
	int saturation;
	int antibanding;
	int whitebalance;
	int framerate;
	int init_needed;
};

static struct ov7695 *to_ov7695(const struct i2c_client *client)
{
	return container_of(i2c_get_clientdata(client), struct ov7695, subdev);
}


static const struct ov7695_timing_cfg timing_cfg_yuv[OV7695_SIZE_LAST] = {
	[OV7695_SIZE_QVGA] = {
			/*  Timing control  2624 x 1952 --> 2560 x 1920 */
			      .x_addr_start = 32,
			      .y_addr_start = 16,
			      .x_addr_end = 2591,
			      .y_addr_end = 1935,
			/*  Output image size */
			      .h_output_size = 320,
			      .v_output_size = 240,
			/*  ISP Windowing size  2560 x 1920 --> 2560 x 1920 */
			      .isp_h_offset = 0,
			      .isp_v_offset = 0,
			/*  Total size (+blanking) */
			      .h_total_size = 2200,
			      .v_total_size = 1280,
			/*  Sensor Read Binning Enabled */
			      .h_odd_ss_inc = 3,
			      .h_even_ss_inc = 1,
			      .v_odd_ss_inc = 3,
			      .v_even_ss_inc = 1,
			      .out_mode_sel = 0x07,
			      .sclk_dividers = 0x01,
			      .sys_mipi_clk = 0x11,
			       },
	[OV7695_SIZE_VGA] = {
			/*  Timing control  2624 x 1952 --> 2560 x 1920 */
			      .x_addr_start = 32,
			      .y_addr_start = 16,
			      .x_addr_end = 2591,
			      .y_addr_end = 1935,
			/*  Output image size */
			      .h_output_size = 640,
			      .v_output_size = 480,
			/*  ISP Windowing size  2560 x 1920 --> 2560 x 1920 */
			      .isp_h_offset = 0,
			      .isp_v_offset = 0,
			/*  Total size (+blanking) */
			      .h_total_size = 2200,
			      .v_total_size = 1280,
			/*  Sensor Read Binning Enabled */
			      .h_odd_ss_inc = 3,
			      .h_even_ss_inc = 1,
			      .v_odd_ss_inc = 3,
			      .v_even_ss_inc = 1,
			      .out_mode_sel = 0x07,
			      .sclk_dividers = 0x01,
			      .sys_mipi_clk = 0x11,
			       },
};

static const struct ov7695_timing_cfg timing_cfg_jpeg[OV7695_SIZE_LAST] = {
	[OV7695_SIZE_QVGA] = {
			/*  Timing control  2624 x 1952 --> 2560 x 1920 */
			      .x_addr_start = 32,
			      .y_addr_start = 16,
			      .x_addr_end = 2591,
			      .y_addr_end = 1935,
			/*  Output image size */
			      .h_output_size = 320,
			      .v_output_size = 240,
			/*  ISP Windowing size  2560 x 1920 --> 2560 x 1920 */
			      .isp_h_offset = 0,
			      .isp_v_offset = 0,
			/*  Total size (+blanking) */
			      .h_total_size = 2844,
			      .v_total_size = 1968,
			/*  Sensor Read Binning Disabled */
			      .h_odd_ss_inc = 1,
			      .h_even_ss_inc = 1,
			      .v_odd_ss_inc = 1,
			      .v_even_ss_inc = 1,
			      .out_mode_sel = 0x26,
			      .sclk_dividers = 0x01,
			      .sys_mipi_clk = 0x12,
			       },
	[OV7695_SIZE_VGA] = {
			/*  Timing control  2624 x 1952 --> 2560 x 1920 */
			      .x_addr_start = 32,
			      .y_addr_start = 16,
			      .x_addr_end = 2591,
			      .y_addr_end = 1935,
			/*  Output image size */
			      .h_output_size = 640,
			      .v_output_size = 480,
			/*  ISP Windowing size  2560 x 1920 --> 2560 x 1920 */
			      .isp_h_offset = 0,
			      .isp_v_offset = 0,
			/*  Total size (+blanking) */
			      .h_total_size = 2844,
			      .v_total_size = 1968,
			/*  Sensor Read Binning Disabled */
			      .h_odd_ss_inc = 1,
			      .h_even_ss_inc = 1,
			      .v_odd_ss_inc = 1,
			      .v_even_ss_inc = 1,
			      .out_mode_sel = 0x26,
			      .sclk_dividers = 0x01,
			      .sys_mipi_clk = 0x12,
			       },
};

/**
 *ov7695_reg_read - Read a value from a register in an ov7695 sensor device
 *@client: i2c driver client structure
 *@reg: register address / offset
 *@val: stores the value that gets read
 *
 * Read a value from a register in an ov7695 sensor device.
 * The value is returned in 'val'.
 * Returns zero if successful, or non-zero otherwise.
 */
static int ov7695_reg_read(struct i2c_client *client, u16 reg, u8 *val)
{
	int ret;
	u8 data[2] = { 0 };
	struct i2c_msg msg = {
		.addr = client->addr,
		.flags = 0,
		.len = 2,
		.buf = data,
	};

	data[0] = (u8) (reg >> 8);
	data[1] = (u8) (reg & 0xff);

	ret = i2c_transfer(client->adapter, &msg, 1);
	if (ret < 0)
		goto err;

	msg.flags = I2C_M_RD;
	msg.len = 1;
	ret = i2c_transfer(client->adapter, &msg, 1);
	if (ret < 0)
		goto err;

	*val = data[0];
	return 0;

err:
	dev_err(&client->dev, "Failed reading register 0x%02x!\n", reg);
	return ret;
}

/**
 * Write a value to a register in ov7695 sensor device.
 *@client: i2c driver client structure.
 *@reg: Address of the register to read value from.
 *@val: Value to be written to a specific register.
 * Returns zero if successful, or non-zero otherwise.
 */
static int ov7695_reg_write(struct i2c_client *client, u16 reg, u8 val)
{
	int ret;
	unsigned char data[3] = { (u8) (reg >> 8), (u8) (reg & 0xff), val };
	struct i2c_msg msg = {
		.addr = client->addr,
		.flags = 0,
		.len = 3,
		.buf = data,
	};

	ret = i2c_transfer(client->adapter, &msg, 1);
	if (ret < 0) {
		dev_err(&client->dev, "Failed writing register 0x%02x!\n", reg);
		return ret;
	}

	return 0;
}

/**
 * Initialize a list of ov7695 registers.
 * The list of registers is terminated by the pair of values
 *@client: i2c driver client structure.
 *@reglist[]: List of address of the registers to write data.
 * Returns zero if successful, or non-zero otherwise.
 */
static int ov7695_write_reglist(struct i2c_client *client,
			const struct ov7695_reg reglist[])
{
	int err = 0, index;

	for (index = 0;	err == 0; index++) {
		if (reglist[index].reg != 0xFFFF)
			err |=
			ov7695_reg_write(client, reglist[index].reg,
				reglist[index].val);
		else if (reglist[index].val != 0xff)
			msleep(reglist[index].val);
		else
			break;
	}

	pr_info("%s: index=%d err=%d\n", __func__, index, err);
	return err;
}


static int ov7695_config_timing(struct i2c_client *client)
{
	struct ov7695 *ov7695 = to_ov7695(client);
	int ret = 0, i = ov7695->i_size;
	const struct ov7695_timing_cfg *timing_cfg;

	if (ov7695_fmts[ov7695->i_fmt].code == V4L2_MBUS_FMT_JPEG_1X8)
		timing_cfg = &timing_cfg_jpeg[i];
	else
		timing_cfg = &timing_cfg_yuv[i];

	/* TODO: Do the right thing here, and validate mipi timing params */
	return ret;
}

static int ov7695_s_stream(struct v4l2_subdev *sd, int enable)
{
	int ret = 0;

	return ret;
}


static int ov7695_g_fmt(struct v4l2_subdev *sd, struct v4l2_mbus_framefmt *mf)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ov7695 *ov7695 = to_ov7695(client);

	dev_dbg(&client->dev, "ov7695_g_fmt\n");

	mf->width = ov7695_frmsizes[ov7695->i_size].width;
	mf->height = ov7695_frmsizes[ov7695->i_size].height;
	mf->code = ov7695_fmts[ov7695->i_fmt].code;
	mf->colorspace = ov7695_fmts[ov7695->i_fmt].colorspace;
	mf->field = V4L2_FIELD_NONE;

	return 0;
}

static int ov7695_try_fmt(struct v4l2_subdev *sd, struct v4l2_mbus_framefmt *mf)
{
	int i_fmt;
	int i_size;

	i_fmt = ov7695_find_datafmt(mf->code);

	mf->code = ov7695_fmts[i_fmt].code;
	mf->colorspace = ov7695_fmts[i_fmt].colorspace;
	mf->field = V4L2_FIELD_NONE;

	i_size = ov7695_find_framesize(mf->width, mf->height);

	mf->width = ov7695_frmsizes[i_size].width;
	mf->height = ov7695_frmsizes[i_size].height;

	return 0;
}

static int ov7695_s_fmt(struct v4l2_subdev *sd, struct v4l2_mbus_framefmt *mf)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ov7695 *ov7695 = to_ov7695(client);
	int ret = 0;

	dev_dbg(&client->dev, "ov7695_s_fmt\n");

	ret = ov7695_try_fmt(sd, mf);
	if (ret < 0)
		return ret;
	/*To avoide reentry init sensor when captrue, remove from here  */
	if (ov7695->init_needed > 0) {
		ret = ov7695_write_reglist(client, ov7695_init_common);
		ov7695->init_needed = 0;
	}

	ov7695->i_size = ov7695_find_framesize(mf->width, mf->height);
	ov7695->i_fmt = ov7695_find_datafmt(mf->code);

	switch ((u32) ov7695_fmts[ov7695->i_fmt].code) {
	case V4L2_MBUS_FMT_UYVY8_2X8:
		break;
	case V4L2_MBUS_FMT_YUYV8_2X8:
		break;
	case V4L2_MBUS_FMT_JPEG_1X8:
		break;
	default:
		/* This shouldn't happen */
		ret = -EINVAL;
		return ret;
	}

	ret = ov7695_config_timing(client);

	return ret;
}

static int ov7695_g_chip_ident(struct v4l2_subdev *sd,
			       struct v4l2_dbg_chip_ident *id)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	if (id->match.type != V4L2_CHIP_MATCH_I2C_ADDR)
		return -EINVAL;

	if (id->match.addr != client->addr)
		return -ENODEV;

	id->ident = V4L2_IDENT_OV7695;
	id->revision = 0;

	return 0;
}

void ov7695_check_mounting(struct i2c_client *client, struct v4l2_ctrl *ctrl)
{
	struct soc_camera_subdev_desc *ssd = client->dev.platform_data;
	struct v4l2_subdev_sensor_interface_parms *iface_parms;

	if (ssd && ssd->drv_priv) {
		iface_parms = ssd->drv_priv;
		ctrl->val = 0;

		if (iface_parms->orientation == V4L2_SUBDEV_SENSOR_PORTRAIT)
			ctrl->val |= V4L2_IN_ST_HFLIP;
		if (iface_parms->facing == V4L2_SUBDEV_SENSOR_BACK)
			ctrl->val |= V4L2_IN_ST_BACK;
	} else
		dev_err(&client->dev, "Missing interface parameters\n");
}

static int ov7695_s_ctrl(struct v4l2_ctrl *ctrl);
static int ov7695_g_volatile_ctrl(struct v4l2_ctrl *ctrl);

static const struct v4l2_ctrl_ops ov7695_ctrl_ops = {
	.s_ctrl = ov7695_s_ctrl,
	.g_volatile_ctrl = ov7695_g_volatile_ctrl,
};

static const struct v4l2_ctrl_config ov7695_controls[] = {
	 {
		.ops = &ov7695_ctrl_ops,
		.id = V4L2_CID_CAMERA_FRAME_RATE,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.name = "Framerate control",
		.min = FRAME_RATE_AUTO,
		.max = (1 << FRAME_RATE_AUTO |
				1 << FRAME_RATE_10 | 1 << FRAME_RATE_30),
		.step = 1,
		.def = FRAME_RATE_AUTO,
		.flags = V4L2_CTRL_FLAG_VOLATILE,
	 },
	 {
		 .ops = &ov7695_ctrl_ops,
		 .id = V4L2_CID_CAM_MOUNTING,
		 .type = V4L2_CTRL_TYPE_INTEGER,
		 .name = "Sensor Mounting",
		 .min = 0,
		 .max = 0x10 | 0x20 | 0x40,
		 .step = 1,
		 .def = 0,
		 .flags = V4L2_CTRL_FLAG_VOLATILE,
	 },
};

static int ov7695_g_volatile_ctrl(struct v4l2_ctrl *ctrl)
{
	struct ov7695 *ov7695 = container_of(ctrl->handler,
						struct ov7695, hdl);
	struct v4l2_subdev *sd = &ov7695->subdev;
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	dev_dbg(&client->dev, "ov7695_g_ctrl\n");

	switch (ctrl->id) {
	case V4L2_CID_BRIGHTNESS:
		ctrl->val = ov7695->brightness;
		break;
	case V4L2_CID_CONTRAST:
		ctrl->val = ov7695->contrast;
		break;
	case V4L2_CID_COLORFX:
		ctrl->val = ov7695->colorlevel;
		break;
	case V4L2_CID_SATURATION:
		ctrl->val = ov7695->saturation;
		break;
	case V4L2_CID_SHARPNESS:
		ctrl->val = ov7695->sharpness;
		break;
	case V4L2_CID_POWER_LINE_FREQUENCY:
		ctrl->val = ov7695->antibanding;
		break;
	case V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE:
		ctrl->val = ov7695->whitebalance;
		break;
	case V4L2_CID_CAMERA_FRAME_RATE:
		ctrl->val = ov7695->framerate;
		break;
	case V4L2_CID_CAM_MOUNTING:
		ov7695_check_mounting(client, ctrl);
		break;
	}

	return 0;
}

static int ov7695_s_ctrl(struct v4l2_ctrl *ctrl)
{
	struct ov7695 *ov7695 = container_of(ctrl->handler,
						struct ov7695, hdl);
	struct v4l2_subdev *sd = &ov7695->subdev;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	u8 ov_reg;
	int ret = 0;

	dev_dbg(&client->dev, "ov7695_s_ctrl\n");

	switch (ctrl->id) {
	case V4L2_CID_BRIGHTNESS:

		if (ctrl->val > EV_PLUS_2)
			return -EINVAL;

		ov7695->brightness = ctrl->val;
		switch (ov7695->brightness) {
		case EV_MINUS_2:
			ret = ov7695_write_reglist(client,
					ov7695_brightness_lv4_tbl);
			break;
		case EV_MINUS_1:
			ret = ov7695_write_reglist(client,
					ov7695_brightness_lv3_tbl);
			break;
		case EV_PLUS_1:
			ret = ov7695_write_reglist(client,
					ov7695_brightness_lv1_tbl);
			break;
		case EV_PLUS_2:
			ret = ov7695_write_reglist(client,
					ov7695_brightness_lv0_tbl);
			break;
		default:
			ret = ov7695_write_reglist(client,
					ov7695_brightness_lv2_default_tbl);
			break;
		}
		if (ret)
			return ret;
		break;
	case V4L2_CID_CONTRAST:

		if (ctrl->val > CONTRAST_PLUS_2)
			return -EINVAL;

		ov7695->contrast = ctrl->val;
		switch (ov7695->contrast) {
		case CONTRAST_MINUS_2:
			ret = ov7695_write_reglist(client,
					ov7695_contrast_lv1_tbl);
			break;
		case CONTRAST_MINUS_1:
			ret = ov7695_write_reglist(client,
					ov7695_contrast_lv2_tbl);
			break;
		case CONTRAST_PLUS_1:
			ret = ov7695_write_reglist(client,
					ov7695_contrast_lv4_tbl);
			break;
		case CONTRAST_PLUS_2:
			ret = ov7695_write_reglist(client,
					ov7695_contrast_lv5_tbl);
			break;
		default:
			ret = ov7695_write_reglist(client,
					ov7695_contrast_default_lv3_tbl);
			break;
		}
		if (ret)
			return ret;
		break;
	case V4L2_CID_COLORFX:

		if (ctrl->val > IMAGE_EFFECT_BNW)
			return -EINVAL;

		ov7695->colorlevel = ctrl->val;

		ov7695_reg_read(client, 0x5800, &ov_reg);
		switch (ov7695->colorlevel) {
		case V4L2_COLORFX_BW:
			ov_reg = (ov_reg | 0x18) & (~0x40);
			ov7695_reg_write(client, 0x5800, ov_reg);
			ret = ov7695_write_reglist(client,
					ov7695_effect_bw_tbl);
			break;
		case V4L2_COLORFX_SEPIA:
			ov_reg = (ov_reg | 0x18) & (~0x40);
			ov7695_reg_write(client, 0x5800, ov_reg);
			ret = ov7695_write_reglist(client,
					ov7695_effect_sepia_tbl);
			break;
		case V4L2_COLORFX_NEGATIVE:
			ov_reg = (ov_reg & (~0x18)) | 0x40;
			ov7695_reg_write(client, 0x5800, ov_reg);
			ret = ov7695_write_reglist(client,
					ov7695_effect_negative_tbl);
			break;
		default:
			ov_reg &= ~(0x58);
			ov7695_reg_write(client, 0x5800, ov_reg);
			ret = ov7695_write_reglist(client,
					ov7695_effect_normal_tbl);
			break;
		}
		if (ret)
			return ret;
		break;
	case V4L2_CID_SATURATION:

		if (ctrl->val > OV7695_SATURATION_MAX)
			return -EINVAL;

		ov7695->saturation = ctrl->val;
		switch (ov7695->saturation) {
		case OV7695_SATURATION_MIN:
			ret = ov7695_write_reglist(client,
					ov7695_saturation_lv0_tbl);
			break;
		case OV7695_SATURATION_MAX:
			ret = ov7695_write_reglist(client,
					ov7695_saturation_lv5_tbl);
			break;
		default:
			ret = ov7695_write_reglist(client,
					ov7695_saturation_default_lv3_tbl);
			break;
		}
		if (ret)
			return ret;
		break;
	case V4L2_CID_SHARPNESS:

		if (ctrl->val > OV7695_SHARPNESS_MAX)
			return -EINVAL;

		ov7695->sharpness = ctrl->val;
		switch (ov7695->sharpness) {
		case OV7695_SHARPNESS_MIN:
			ret = ov7695_write_reglist(client,
					ov7695_sharpness_lv0_tbl);
			break;
		case OV7695_SHARPNESS_MAX:
			ret = ov7695_write_reglist(client,
					ov7695_sharpness_lv3_tbl);
			break;
		default:
			ret = ov7695_write_reglist(client,
					ov7695_sharpness_default_lv2_tbl);
			break;
		}
		if (ret)
			return ret;
		break;

	case V4L2_CID_POWER_LINE_FREQUENCY:

		if (ctrl->val > V4L2_CID_POWER_LINE_FREQUENCY_AUTO)
			return -EINVAL;

		ov7695->antibanding = ctrl->val;

		switch (ov7695->antibanding) {
		case V4L2_CID_POWER_LINE_FREQUENCY_50HZ:
			ret = ov7695_write_reglist(client,
					ov7695_antibanding_50z_tbl);
			break;
		case V4L2_CID_POWER_LINE_FREQUENCY_60HZ:
			ret = ov7695_write_reglist(client,
					ov7695_antibanding_60z_tbl);
			break;
		case V4L2_CID_POWER_LINE_FREQUENCY_AUTO:
		default:
			ret = ov7695_write_reglist(client,
					ov7695_antibanding_auto_tbl);
			break;
		}
		if (ret)
			return ret;
		break;

	case V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE:

		if (ctrl->val > V4L2_WHITE_BALANCE_CLOUDY)
			return -EINVAL;

		ov7695->whitebalance = ctrl->val;

		ret = ov7695_reg_read(client, 0x5200, &ov_reg);
		if (ret)
			return ret;

		pr_info("Set V4L2_CID_CAMERA_WHITE_BALANCE:%d\n",
			ov7695->whitebalance);
		switch (ov7695->whitebalance) {
		case V4L2_WHITE_BALANCE_FLUORESCENT:
			ov_reg |= 0x20;
			ov7695_reg_write(client, 0x5200, ov_reg);
			ret = ov7695_write_reglist(client,
					ov7695_wb_fluorescent);
			break;
		case V4L2_WHITE_BALANCE_DAYLIGHT:
			ov_reg |= 0x20;
			ov7695_reg_write(client, 0x5200, ov_reg);
			ret = ov7695_write_reglist(client,
					ov7695_wb_daylight);
			break;
		case V4L2_WHITE_BALANCE_CLOUDY:
			ov_reg |= 0x20;
			ov7695_reg_write(client, 0x5200, ov_reg);
			ret = ov7695_write_reglist(client,
				ov7695_wb_cloudy);
			break;
		case V4L2_WHITE_BALANCE_INCANDESCENT:
			ov_reg |= 0x20;
			ov7695_reg_write(client, 0x5200, ov_reg);
			ret = ov7695_write_reglist(client,
					ov7695_wb_tungsten);
			break;
		case V4L2_WHITE_BALANCE_AUTO:
		default:
			ov_reg &= ~(0x20);
			ov7695_reg_write(client, 0x5200, ov_reg);
			ret = ov7695_write_reglist(client,
						ov7695_wb_def);
			break;
		}
		if (ret)
			return ret;
		break;

	case V4L2_CID_CAMERA_FRAME_RATE:

		if (ctrl->val > FRAME_RATE_30)
			return -EINVAL;

		if ((ov7695->i_size < OV7695_SIZE_QVGA) ||
			(ov7695->i_size > OV7695_SIZE_VGA)) {
			if (ctrl->val == FRAME_RATE_30 ||
					ctrl->val == FRAME_RATE_AUTO)
				return 0;
			else
				return -EINVAL;
		}

		ov7695->framerate = ctrl->val;

		switch (ov7695->framerate) {
		/*
		case FRAME_RATE_5:
			ret = ov7695_write_reglist(client,
					ov7695_fps_5);
			break;
		case FRAME_RATE_7:
			ret = ov7695_write_reglist(client,
					ov7695_fps_7);
			break;
		case FRAME_RATE_10:
			ret = ov7695_write_reglist(client,
					ov7695_fps_10);
			break;
		case FRAME_RATE_15:
			ret = ov7695_write_reglist(client,
					ov7695_fps_15);
			break;
		case FRAME_RATE_20:
			ret = ov7695_write_reglist(client,
					ov7695_fps_20);
			break;
		case FRAME_RATE_25:
			ret = ov7695_write_reglist(client,
					ov7695_fps_25);
			break;
		*/
		case FRAME_RATE_30:
		case FRAME_RATE_AUTO:
		default:
			ret = ov7695_write_reglist(client,
					ov7695_fps_30);
			break;
		}
		if (ret)
			return ret;
		break;
	}

	return ret;
}

static long ov7695_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
{
	int ret = 0;

	switch (cmd) {
	case VIDIOC_THUMB_SUPPORTED:
		{
			int *p = arg;
			*p = 0;	/* no we don't support thumbnail */
			break;
		}
	case VIDIOC_JPEG_G_PACKET_INFO:
		{
			struct v4l2_jpeg_packet_info *p =
				(struct v4l2_jpeg_packet_info *)arg;
			p->padded = 0;
			p->packet_size = 0x400;
			break;
		}

	case VIDIOC_SENSOR_G_OPTICAL_INFO:
		{
			struct v4l2_sensor_optical_info *p =
				(struct v4l2_sensor_optical_info *)arg;
			/* assuming 67.5 degree diagonal viewing angle */
			p->hor_angle.numerator = 5401;
			p->hor_angle.denominator = 100;
			p->ver_angle.numerator = 3608;
			p->ver_angle.denominator = 100;
			p->focus_distance[0] = 10; /* near focus in cm */
			p->focus_distance[1] = 100; /* optimal focus in cm */
			p->focus_distance[2] = -1; /* infinity */
			p->focal_length.numerator = 342;
			p->focal_length.denominator = 100;
			break;
		}
	default:
		ret = -ENOIOCTLCMD;
		break;
	}
	return ret;
}

#ifdef CONFIG_VIDEO_ADV_DEBUG
static int ov7695_g_register(struct v4l2_subdev *sd,
			     struct v4l2_dbg_register *reg)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	if (reg->match.type != V4L2_CHIP_MATCH_I2C_ADDR || reg->size > 2)
		return -EINVAL;

	if (reg->match.addr != client->addr)
		return -ENODEV;

	reg->size = 2;
	if (ov7695_read_smbus(client, reg->reg, &reg->val))
		return -EIO return 0;
}

static int ov7695_s_register(struct v4l2_subdev *sd,
			     struct v4l2_dbg_register *reg)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	if (reg->match.type != V4L2_CHIP_MATCH_I2C_ADDR || reg->size > 2)
		return -EINVAL;

	if (reg->match.addr != client->addr)
		return -ENODEV;

	if (ov7695_reg_write(client, reg->reg, reg->val))
		return -EIO;

	return 0;
}
#endif
static int ov7695_init(struct i2c_client *client)
{
	struct ov7695 *ov7695 = to_ov7695(client);
	int ret = 0;

	pr_info("%s: Sensor initialized!\n", __func__);

	ret = ov7695_write_reglist(client, ov7695_init_common);
	if (ret) {
		pr_info("%s: Sensor initialized failed!\n", __func__);
		goto out;
	}

	/* default brightness and contrast */
	ov7695->brightness = EV_DEFAULT;
	ov7695->contrast = CONTRAST_DEFAULT;
	ov7695->colorlevel = V4L2_COLORFX_NONE;
	ov7695->antibanding = V4L2_CID_POWER_LINE_FREQUENCY_50HZ;
	ov7695->whitebalance = V4L2_WHITE_BALANCE_AUTO;
	ov7695->framerate = FRAME_RATE_AUTO;
	ov7695->init_needed = 1;

	dev_dbg(&client->dev, "Sensor initialized\n");

out:
	return ret;
}

static int ov7695_s_power(struct v4l2_subdev *sd, int on)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct soc_camera_subdev_desc *ssdd = soc_camera_i2c_to_desc(client);
	struct ov7695 *ov7695 = to_ov7695(client);
	if (!on)
		return soc_camera_power_off(&client->dev, ssdd);
	ov7695->init_needed = 1;
	return soc_camera_power_on(&client->dev, ssdd);
}

/*
 * Interface active, can use i2c. If it fails, it can indeed mean, that
 * this wasn't our capture interface, so, we wait for the right one
 */
static int ov7695_video_probe(struct i2c_client *client)
{
	int ret = 0;
	u8 value = 0;

	ret = ov7695_reg_read(client, 0x300a, &value);
	pr_info("%s: CHIP_IDH = 0x%x\n", __func__, value);
	if (ret < 0)
		return ret;
	if (value != 0x76) /* OV manuf. id. */
		return -ENODEV;

	ret = ov7695_reg_read(client, 0x300b, &value);
	pr_info("%s: CHIP_IDL = 0x%x\n", __func__, value);
	if (ret < 0)
		return ret;
	if (value != 0x95)
		return -ENODEV;

	/*
	* OK, we know we have an OmniVision chip...but which one?
	*/
	ret = ov7695_reg_read(client, 0x302a, &value);
	pr_info("%s: CHIP_REV = 0x%x\n", __func__, value);

	/* TODO: Do something like ov7695_init */
	return ret;
}

static struct v4l2_subdev_core_ops ov7695_subdev_core_ops = {
	.g_chip_ident = ov7695_g_chip_ident,
	.s_power = ov7695_s_power,
	.ioctl = ov7695_ioctl,
#ifdef CONFIG_VIDEO_ADV_DEBUG
	.g_register = ov7695_g_register,
	.s_register = ov7695_s_register,
#endif
};

static int ov7695_enum_fmt(struct v4l2_subdev *sd, unsigned int index,
			   enum v4l2_mbus_pixelcode *code)
{
	if (index >= ARRAY_SIZE(ov7695_fmts))
		return -EINVAL;

	*code = ov7695_fmts[index].code;
	return 0;
}

static int ov7695_enum_framesizes(struct v4l2_subdev *sd,
				  struct v4l2_frmsizeenum *fsize)
{
	if (fsize->index >= OV7695_SIZE_LAST)
		return -EINVAL;

	fsize->type = V4L2_FRMSIZE_TYPE_DISCRETE;
	fsize->pixel_format = V4L2_PIX_FMT_UYVY;

	fsize->discrete = ov7695_frmsizes[fsize->index];

	return 0;
}

/* we only support fixed frame rate */
static int ov7695_enum_frameintervals(struct v4l2_subdev *sd,
				      struct v4l2_frmivalenum *interval)
{
	int size;

	if (interval->index >= 1)
		return -EINVAL;

	interval->type = V4L2_FRMIVAL_TYPE_DISCRETE;

	size = ov7695_find_framesize(interval->width, interval->height);

	switch (size) {
	case OV7695_SIZE_VGA:
	case OV7695_SIZE_QVGA:
	default:
		interval->discrete.numerator = 1;
		interval->discrete.denominator = 24;
		break;
	}
/*	printk(KERN_ERR"%s: width=%d height=%d fi=%d/%d\n", __func__,
			interval->width,
			interval->height, interval->discrete.numerator,
			interval->discrete.denominator);
			*/
	return 0;
}

static int ov7695_g_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *param)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ov7695 *ov7695 = to_ov7695(client);
	struct v4l2_captureparm *cparm;

	if (param->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;

	cparm = &param->parm.capture;

	memset(param, 0, sizeof(*param));
	param->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	cparm->capability = V4L2_CAP_TIMEPERFRAME;

	switch (ov7695->i_size) {
	case OV7695_SIZE_VGA:
	case OV7695_SIZE_QVGA:
	default:
		cparm->timeperframe.numerator = 1;
		cparm->timeperframe.denominator = 24;
		break;
	}

	return 0;
}
static int ov7695_s_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *param)
{
	/*
	 * FIXME: This just enforces the hardcoded framerates until this is
	 *flexible enough.
	 */
	return ov7695_g_parm(sd, param);
}

static struct v4l2_subdev_video_ops ov7695_subdev_video_ops = {
	.s_stream = ov7695_s_stream,
	.s_mbus_fmt = ov7695_s_fmt,
	.g_mbus_fmt = ov7695_g_fmt,
	.try_mbus_fmt = ov7695_try_fmt,
	.enum_mbus_fmt = ov7695_enum_fmt,
	.enum_mbus_fsizes = ov7695_enum_framesizes,
	.enum_framesizes = ov7695_enum_framesizes,
	.enum_frameintervals = ov7695_enum_frameintervals,
	.g_parm = ov7695_g_parm,
	.s_parm = ov7695_s_parm,
};
static int ov7695_g_skip_frames(struct v4l2_subdev *sd, u32 *frames)
{
	/* Quantity of initial bad frames to skip. Revisit. */
	*frames = 5;

	return 0;
}

static int ov7695_g_interface_parms(struct v4l2_subdev *sd,
				    struct v4l2_subdev_sensor_interface_parms
				    *parms)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ov7695 *ov7695 = to_ov7695(client);
	u8 sclk_dividers;

	if (!parms)
		return -EINVAL;

	parms->if_type = ov7695->plat_parms->if_type;
	parms->if_mode = ov7695->plat_parms->if_mode;
	parms->parms = ov7695->plat_parms->parms;

	/* set the hs term time */
	if (ov7695_fmts[ov7695->i_fmt].code == V4L2_MBUS_FMT_JPEG_1X8)
		sclk_dividers  = timing_cfg_jpeg[ov7695->i_size].sclk_dividers;
	else
		sclk_dividers = timing_cfg_yuv[ov7695->i_size].sclk_dividers;

	if (sclk_dividers == 0x01)
		parms->parms.serial.hs_term_time = 0x08;
	else
		parms->parms.serial.hs_term_time = 0x01;

	return 0;
}



static struct v4l2_subdev_sensor_ops ov7695_subdev_sensor_ops = {
	.g_skip_frames = ov7695_g_skip_frames,
	.g_interface_parms = ov7695_g_interface_parms,
};

static struct v4l2_subdev_ops ov7695_subdev_ops = {
	.core = &ov7695_subdev_core_ops,
	.video = &ov7695_subdev_video_ops,
	.sensor = &ov7695_subdev_sensor_ops,
};

static int ov7695_probe(struct i2c_client *client,
			const struct i2c_device_id *did)
{
	struct ov7695 *ov7695;
	struct soc_camera_subdev_desc *ssd = client->dev.platform_data;
	struct v4l2_subdev *subdev = NULL;
	int i, ret;

	client->addr = 0x21;
	if (!ssd) {
		dev_err(&client->dev, "OV7695: missing soc-camera data!\n");
		return -EINVAL;
	}

	if (!ssd->drv_priv) {
		dev_err(&client->dev,
			"OV7695 driver needs i/f platform data\n");
		return -EINVAL;
	}

	ov7695 = kzalloc(sizeof(struct ov7695), GFP_KERNEL);
	if (!ov7695)
		return -ENOMEM;

	v4l2_i2c_subdev_init(&ov7695->subdev, client, &ov7695_subdev_ops);
	subdev = i2c_get_clientdata(client);

	ov7695->i_size = OV7695_SIZE_VGA;
	ov7695->i_fmt = 0;	/* First format in the list */
	ov7695->plat_parms = ssd->drv_priv;

	/* Initializing hdl with 12 controls (3 for future purposes) */
	v4l2_ctrl_handler_init(&ov7695->hdl, ARRAY_SIZE(ov7695_controls) + 10);
	if (ov7695->hdl.error)
		dev_dbg(&client->dev, "Error set during init itself! %d\n",
			ov7695->hdl.error);

	/* register standard controls */
	v4l2_ctrl_new_std(&ov7695->hdl, &ov7695_ctrl_ops, V4L2_CID_BRIGHTNESS,
			EV_MINUS_1, EV_PLUS_1, 1, EV_DEFAULT);

	v4l2_ctrl_new_std(&ov7695->hdl, &ov7695_ctrl_ops, V4L2_CID_CONTRAST,
			CONTRAST_MINUS_2, CONTRAST_PLUS_2, 1, CONTRAST_DEFAULT);

	v4l2_ctrl_new_std(&ov7695->hdl, &ov7695_ctrl_ops, V4L2_CID_SATURATION,
			OV7695_SATURATION_MIN, OV7695_SATURATION_MAX,
			OV7695_SATURATION_STEP, OV7695_SATURATION_DEF);

	v4l2_ctrl_new_std(&ov7695->hdl, &ov7695_ctrl_ops, V4L2_CID_SHARPNESS,
			OV7695_SHARPNESS_MIN, OV7695_SHARPNESS_MAX,
			OV7695_SHARPNESS_STEP, OV7695_SHARPNESS_DEF);

	if (ov7695->hdl.error) {
		dev_err(&client->dev,
			"Standard controls initialization error %d\n",
			ov7695->hdl.error);
		ret = ov7695->hdl.error;
		goto ctrl_hdl_err;
	}

	/* register standard menu controls */
	v4l2_ctrl_new_std_menu(&ov7695->hdl, &ov7695_ctrl_ops,
		V4L2_CID_COLORFX, V4L2_COLORFX_NEGATIVE, 0,
		V4L2_COLORFX_NONE);

	v4l2_ctrl_new_std_menu(&ov7695->hdl, &ov7695_ctrl_ops,
		V4L2_CID_POWER_LINE_FREQUENCY,
		V4L2_CID_POWER_LINE_FREQUENCY_AUTO, 0,
		V4L2_CID_POWER_LINE_FREQUENCY_50HZ);

	v4l2_ctrl_new_std_menu(&ov7695->hdl, &ov7695_ctrl_ops,
		V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE,
		V4L2_WHITE_BALANCE_CLOUDY, 0,
		V4L2_WHITE_BALANCE_AUTO);

	if (ov7695->hdl.error) {
		dev_err(&client->dev,
			"Standard menu controls initialization error %d\n",
			ov7695->hdl.error);
		ret = ov7695->hdl.error;
		goto ctrl_hdl_err;
	}

	/* register custom controls */
	for (i = 0; i < ARRAY_SIZE(ov7695_controls); ++i)
		v4l2_ctrl_new_custom(&ov7695->hdl, &ov7695_controls[i], NULL);

	ov7695->subdev.ctrl_handler = &ov7695->hdl;
	if (ov7695->hdl.error) {
		ret = ov7695->hdl.error;
		goto ctrl_hdl_err;
	}

	ret = ov7695_s_power(subdev, 1);
	if (ret < 0)
		return ret;

	ret = ov7695_video_probe(client);
	if (ret) {
		pr_err("ov7695_video_probe: failed to probe the sensor\n");
		goto vid_probe_fail;
	}
	msleep(20);

	/* init the sensor here */
	ret = ov7695_init(client);
	if (ret) {
		dev_err(&client->dev, "Failed to initialize sensor\n");
		goto init_fail;
	}

	return ret;

ctrl_hdl_err:
init_fail:
vid_probe_fail:
	v4l2_ctrl_handler_free(&ov7695->hdl);
	kfree(ov7695);
	pr_err("ov7695_probe failed with ret = %d\n", ret);
	return ret;

}

static int ov7695_remove(struct i2c_client *client)
{
	struct ov7695 *ov7695 = to_ov7695(client);

	pr_debug(" remove");
	v4l2_device_unregister_subdev(&ov7695->subdev);
	client->driver = NULL;
	kfree(ov7695);

	return 0;
}

static const struct i2c_device_id ov7695_id[] = {
	{"ov7695", 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, ov7695_id);

static struct i2c_driver ov7695_i2c_driver = {
	.driver = {
		   .name = "ov7695",
		   },
	.probe = ov7695_probe,
	.remove = ov7695_remove,
	.id_table = ov7695_id,
};

static int __init ov7695_mod_init(void)
{
	return i2c_add_driver(&ov7695_i2c_driver);
}

static void __exit ov7695_mod_exit(void)
{
	i2c_del_driver(&ov7695_i2c_driver);
}

module_init(ov7695_mod_init);
module_exit(ov7695_mod_exit);

MODULE_DESCRIPTION("OmniVision OV7695 Camera driver");
MODULE_AUTHOR("Sergio Aguirre <saaguirre@ti.com>");
MODULE_LICENSE("GPL v2");
