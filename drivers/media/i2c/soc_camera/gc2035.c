/*
 * GC2035 sensor driver
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

#include <uapi/linux/time.h>
#include <uapi/linux/videodev2.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/log2.h>
#include <linux/delay.h>
#include <linux/module.h>

#include <media/v4l2-ctrls.h>
#include <media/v4l2-subdev.h>
#include <media/v4l2-chip-ident.h>
#include <media/soc_camera.h>
#include <linux/videodev2_brcm.h>

#include "gc2035.h"

/* #define GC2035_DEBUG */

#define iprintk(format, arg...)	\
	pr_info("[%s]: "format"\n", __func__, ##arg)

/* GC2035 has only one fixed colorspace per pixelcode */
struct gc2035_datafmt {
	enum v4l2_mbus_pixelcode code;
	enum v4l2_colorspace colorspace;
};

struct gc2035_timing_cfg {
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

static const struct gc2035_datafmt gc2035_fmts[] = {
	{V4L2_MBUS_FMT_UYVY8_2X8, V4L2_COLORSPACE_JPEG},
	{V4L2_MBUS_FMT_YUYV8_2X8, V4L2_COLORSPACE_JPEG},

};

enum gc2035_size {
	GC2035_SIZE_VGA,	/*  640 x 480 */
	GC2035_SIZE_UXGA,	/*  1600 x 1200 (2M) */
	GC2035_SIZE_LAST,
	GC2035_SIZE_MAX
};

enum  cam_running_mode {
	CAM_RUNNING_MODE_NOTREADY,
	CAM_RUNNING_MODE_PREVIEW,
	CAM_RUNNING_MODE_CAPTURE,
	CAM_RUNNING_MODE_RECORDING,
};
enum  cam_running_mode runmode;

static const struct v4l2_frmsize_discrete gc2035_frmsizes[GC2035_SIZE_LAST] = {
	{640, 480},
	{1600, 1200},
};

/* Find a data format by a pixel code in an array */
static int gc2035_find_datafmt(enum v4l2_mbus_pixelcode code)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(gc2035_fmts); i++)
		if (gc2035_fmts[i].code == code)
			break;

	/* If not found, select latest */
	if (i >= ARRAY_SIZE(gc2035_fmts))
		i = ARRAY_SIZE(gc2035_fmts) - 1;

	return i;
}

static int gc2035_find_framesize(u32 width, u32 height)
{
	int i;

	for (i = 0; i < GC2035_SIZE_LAST; i++) {
		if ((gc2035_frmsizes[i].width >= width) &&
		    (gc2035_frmsizes[i].height >= height))
			break;
	}

	/* If not found, select biggest */
	if (i >= GC2035_SIZE_LAST)
		i = GC2035_SIZE_LAST - 1;

	return i;
}

struct gc2035 {
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
	short runmode;
	short stream_status;
	int init_needed;
};

static struct gc2035 *to_gc2035(const struct i2c_client *client)
{
	return container_of(i2c_get_clientdata(client), struct gc2035, subdev);
}


/**
 *gc2035_reg_read - Read a value from a register in an gc2035 sensor device
 *@client: i2c driver client structure
 *@reg: register address / offset
 *@val: stores the value that gets read
 *
 * Read a value from a register in an gc2035 sensor device.
 * The value is returned in 'val'.
 * Returns zero if successful, or non-zero otherwise.
 */
static int gc2035_read_smbus(struct i2c_client *client, unsigned char reg,
		unsigned char *value)
{
	int ret;
	ret = i2c_smbus_read_byte_data(client, reg);
	if (ret >= 0) {
		*value = (unsigned char)ret;
		ret = 0;
	}
	return ret;
}

static int gc2035_write_smbus(struct i2c_client *client, unsigned char reg,
		unsigned char value)
{
	int ret = i2c_smbus_write_byte_data(client, reg, value);

	if (0 != ret)
		pr_err("%s:Error write reg=0x%x, value=0x%x, ret=%d\n",
		__func__, reg, value, ret);

	return ret;
}


/**
 * Write a value to a register in gc2035 sensor device.
 *@client: i2c driver client structure.
 *@reg: Address of the register to read value from.
 *@val: Value to be written to a specific register.
 * Returns zero if successful, or non-zero otherwise.
 */
static int gc2035_reg_write(struct i2c_client *client, u16 reg, u8 val)
{
	int ret;

	ret = i2c_smbus_write_byte_data(client, (u8) (reg & 0xff), val);

	if (ret < 0) {
		dev_err(&client->dev, "Failed writing register 0x%02x!\n", reg);
		return ret;
	}

	return 0;
}

/**
 * Initialize a list of gc2035 registers.
 * The list of registers is terminated by the pair of values
 *@client: i2c driver client structure.
 *@reglist[]: List of address of the registers to write data.
 * Returns zero if successful, or non-zero otherwise.
 */
static int gc2035_reg_writes(struct i2c_client *client,
			const struct gc2035_reg reglist[])
{
	int err = 0, index;

	for (index = 0; (reglist[index].reg != 0xffff); index++) {
		if ((reglist[index].reg == 0xfffe))
			msleep(reglist[index].val);
		else
			err = gc2035_write_smbus(client,
			(u8)(reglist[index].reg & 0xff), reglist[index].val);
	}

	return 0;
}

static int gc2035_config_preview(struct v4l2_subdev *sd)
{
	int ret;
	u8 val = 0;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct gc2035 *gc2035 = to_gc2035(client);

	/* enable AEC */
	gc2035_reg_write(client, 0xfe, 0x00);
	gc2035_reg_write(client, 0xb6, 0x03);
	ret = gc2035_reg_writes(client, gc2035_mode[gc2035->i_size]);

	return ret;
}

static int gc2035_config_capture(struct v4l2_subdev *sd)
{
	int ret = 0;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct gc2035 *gc2035 = to_gc2035(client);
	uint8_t val = 0;
	uint16_t p_clk_div, p_shutter;
	uint16_t c_clk_div, c_shutter;

	/* disable AEC/AGC */
	gc2035_reg_write(client, 0xfe, 0x00);
	gc2035_reg_write(client, 0xb6, 0x00);
	/* read preview div/shutter val */
	gc2035_read_smbus(client, 0xfa, &val);
	p_clk_div = ((val & 0xf0)>>4) + 1;
	gc2035_read_smbus(client, 0x03, &val);
	p_shutter = val;
	gc2035_read_smbus(client, 0x04, &val);
	p_shutter = (p_shutter << 8) | val;

	/* download capture settings */
	ret = gc2035_reg_writes(client, gc2035_mode[gc2035->i_size]);

	/* read capture clock div */
	/* set to page 0 */
	gc2035_reg_write(client, 0xfe, 0x00);
	gc2035_read_smbus(client, 0xfa, &val);
	c_clk_div = ((val & 0xf0)>>4) + 1;
	/* calc capture shutter */
	c_shutter = p_clk_div * p_shutter / c_clk_div;
	gc2035_reg_write(client, 0x03, (c_shutter & 0xFF00)>>8);
	gc2035_reg_write(client, 0x04, (c_shutter & 0x00FF));

	return ret;
}


static int gc2035_s_stream(struct v4l2_subdev *sd, int enable)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct gc2035 *gc2035 = to_gc2035(client);
	int ret = 0;

	if (enable == gc2035->stream_status)
		return ret;

	if (enable) {
		if (CAM_RUNNING_MODE_CAPTURE != gc2035->runmode)
			gc2035_config_preview(sd);
		else
			gc2035_config_capture(sd);
		/* Start Streaming */
		ret = gc2035_reg_writes(client, gc2035_streamon);
		msleep(50);
	} else {
		/* Stop Streaming, Power Down*/
		ret = gc2035_reg_writes(client, gc2035_streamoff);
	}
	gc2035->stream_status = enable;

	return ret;
}

static int gc2035_g_fmt(struct v4l2_subdev *sd, struct v4l2_mbus_framefmt *mf)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct gc2035 *gc2035 = to_gc2035(client);

	mf->width = gc2035_frmsizes[gc2035->i_size].width;
	mf->height = gc2035_frmsizes[gc2035->i_size].height;
	mf->code = gc2035_fmts[gc2035->i_fmt].code;
	mf->colorspace = gc2035_fmts[gc2035->i_fmt].colorspace;
	mf->field = V4L2_FIELD_NONE;

	return 0;
}

static int gc2035_try_fmt(struct v4l2_subdev *sd, struct v4l2_mbus_framefmt *mf)
{
	int i_fmt;
	int i_size;

	i_fmt = gc2035_find_datafmt(mf->code);

	mf->code = gc2035_fmts[i_fmt].code;
	mf->colorspace = gc2035_fmts[i_fmt].colorspace;
	mf->field = V4L2_FIELD_NONE;

	i_size = gc2035_find_framesize(mf->width, mf->height);

	mf->width = gc2035_frmsizes[i_size].width;
	mf->height = gc2035_frmsizes[i_size].height;

	return 0;
}

static int gc2035_s_fmt(struct v4l2_subdev *sd, struct v4l2_mbus_framefmt *mf)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct gc2035 *gc2035 = to_gc2035(client);
	int ret = 0;

	ret = gc2035_try_fmt(sd, mf);
	if (ret < 0)
		return ret;

	gc2035->i_size = gc2035_find_framesize(mf->width, mf->height);
	gc2035->i_fmt = gc2035_find_datafmt(mf->code);

	/*To avoide reentry init sensor, remove from here	*/
	/*ret =  gc2035_reg_writes(client,configscript_common1);*/
	if (gc2035->init_needed)
		ret = gc2035_reg_writes(client, configscript_common1);

	if (ret)
		return ret;

	switch ((u32) gc2035_fmts[gc2035->i_fmt].code) {
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

	return ret;
}

static int gc2035_g_chip_ident(struct v4l2_subdev *sd,
			       struct v4l2_dbg_chip_ident *id)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	if (id->match.type != V4L2_CHIP_MATCH_I2C_ADDR)
		return -EINVAL;

	if (id->match.addr != client->addr)
		return -ENODEV;

	id->ident = V4L2_IDENT_GC2035;
	id->revision = 0;

	return 0;
}

void gc2035_check_mounting(struct i2c_client *client, struct v4l2_ctrl *ctrl)
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

static int gc2035_s_ctrl(struct v4l2_ctrl *ctrl);
static int gc2035_g_volatile_ctrl(struct v4l2_ctrl *ctrl);

static const struct v4l2_ctrl_ops gc2035_ctrl_ops = {
	.s_ctrl = gc2035_s_ctrl,
	.g_volatile_ctrl = gc2035_g_volatile_ctrl,
};

static const struct v4l2_ctrl_config gc2035_controls[] = {
	 {
		 .ops = &gc2035_ctrl_ops,
		 .id = V4L2_CID_CAMERA_FRAME_RATE,
		 .type = V4L2_CTRL_TYPE_INTEGER,
		 .name = "Framerate control",
		 .min = FRAME_RATE_AUTO,
		 .max = (1 << FRAME_RATE_AUTO | 1 << FRAME_RATE_15 |
					1 << FRAME_RATE_30),
		 .step = 1,
		 .def = FRAME_RATE_AUTO,
		 .flags = V4L2_CTRL_FLAG_VOLATILE,
	 },
	 {
		 .ops = &gc2035_ctrl_ops,
		 .id = V4L2_CID_CAM_PREVIEW_ONOFF,
		 .type = V4L2_CTRL_TYPE_INTEGER,
		 .name = "PreviewOnOff",
		 .min = 0,
		 .max = 1,
		 .step = 1,
		 .def = 0,
		 .flags = V4L2_CTRL_FLAG_VOLATILE,
	 },
	 {
		 .ops = &gc2035_ctrl_ops,
		 .id = V4L2_CID_CAM_CAPTURE,
		 .type = V4L2_CTRL_TYPE_INTEGER,
		 .name = "Capture",
		 .min = 0,
		 .max = 1,
		 .step = 1,
		 .def = 0,
		 .flags = V4L2_CTRL_FLAG_VOLATILE,
	 },
	 {
		 .ops = &gc2035_ctrl_ops,
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

static int gc2035_g_volatile_ctrl(struct v4l2_ctrl *ctrl)
{
	struct gc2035 *gc2035 = container_of(ctrl->handler,
						struct gc2035, hdl);
	struct v4l2_subdev *sd = &gc2035->subdev;
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	dev_dbg(&client->dev, "gc2035_g_ctrl\n");

	switch (ctrl->id) {
	case V4L2_CID_BRIGHTNESS:
		ctrl->val = gc2035->brightness;
		break;
	case V4L2_CID_CONTRAST:
		ctrl->val = gc2035->contrast;
		break;
	case V4L2_CID_COLORFX:
		ctrl->val = gc2035->colorlevel;
		break;
	case V4L2_CID_SATURATION:
		ctrl->val = gc2035->saturation;
		break;
	case V4L2_CID_SHARPNESS:
		ctrl->val = gc2035->sharpness;
		break;
	case V4L2_CID_POWER_LINE_FREQUENCY:
		ctrl->val = gc2035->antibanding;
		break;
	case V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE:
		ctrl->val = gc2035->whitebalance;
		break;
	case V4L2_CID_CAMERA_FRAME_RATE:
		ctrl->val = gc2035->framerate;
		break;
	case V4L2_CID_CAM_MOUNTING:
		gc2035_check_mounting(client, ctrl);
		break;
	}

	return 0;
}

static int gc2035_s_ctrl(struct v4l2_ctrl *ctrl)
{
	struct gc2035 *gc2035 = container_of(ctrl->handler,
						struct gc2035, hdl);
	struct v4l2_subdev *sd = &gc2035->subdev;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	u8 ov_reg;
	int ret = 0;

	switch (ctrl->id) {
	case V4L2_CID_BRIGHTNESS:

		if (ctrl->val > EV_PLUS_1)
			return -EINVAL;

		gc2035->brightness = ctrl->val;
		switch (gc2035->brightness) {
		case EV_MINUS_1:
			ret = gc2035_reg_writes(client,
					gc2035_brightness_lv4_tbl);
			break;
		case EV_PLUS_1:
			ret = gc2035_reg_writes(client,
					gc2035_brightness_lv0_tbl);
			break;
		default:
			ret = gc2035_reg_writes(client,
					gc2035_brightness_lv2_default_tbl);
			break;
		}
		if (ret)
			return ret;
		break;
	case V4L2_CID_CONTRAST:

		if (ctrl->val > CONTRAST_PLUS_1)
			return -EINVAL;

		gc2035->contrast = ctrl->val;
		switch (gc2035->contrast) {
		case CONTRAST_MINUS_1:
			ret = gc2035_reg_writes(client,
					gc2035_contrast_lv5_tbl);
			break;
		case CONTRAST_PLUS_1:
			ret = gc2035_reg_writes(client,
					gc2035_contrast_lv0_tbl);
			break;
		default:
			ret = gc2035_reg_writes(client,
					gc2035_contrast_default_lv3_tbl);
			break;
		}
		if (ret)
			return ret;
		break;
	case V4L2_CID_COLORFX:

		if (ctrl->val > IMAGE_EFFECT_BNW)
			return -EINVAL;

		gc2035->colorlevel = ctrl->val;

		switch (gc2035->colorlevel) {
		case IMAGE_EFFECT_BNW:
			ret = gc2035_reg_writes(client,
					gc2035_effect_bw_tbl);
			break;
		case IMAGE_EFFECT_SEPIA:
			ret = gc2035_reg_writes(client,
					gc2035_effect_sepia_tbl);
			break;
		case IMAGE_EFFECT_NEGATIVE:
			ret = gc2035_reg_writes(client,
					gc2035_effect_negative_tbl);
			break;
		default:
			ret = gc2035_reg_writes(client,
					gc2035_effect_normal_tbl);
			break;
		}
		if (ret)
			return ret;
		break;
	case V4L2_CID_SATURATION:

		if (ctrl->val > GC2035_SATURATION_MAX)
			return -EINVAL;

		gc2035->saturation = ctrl->val;
		switch (gc2035->saturation) {
		case GC2035_SATURATION_MIN:
			ret = gc2035_reg_writes(client,
					gc2035_saturation_lv0_tbl);
			break;
		case GC2035_SATURATION_MAX:
			ret = gc2035_reg_writes(client,
					gc2035_saturation_lv5_tbl);
			break;
		default:
			ret = gc2035_reg_writes(client,
					gc2035_saturation_default_lv3_tbl);
			break;
		}
		if (ret)
			return ret;
		break;
	case V4L2_CID_SHARPNESS:

		if (ctrl->val > GC2035_SHARPNESS_MAX)
			return -EINVAL;

		gc2035->sharpness = ctrl->val;
		switch (gc2035->sharpness) {
		case GC2035_SHARPNESS_MIN:
			ret = gc2035_reg_writes(client,
					gc2035_sharpness_lv0_tbl);
			break;
		case GC2035_SHARPNESS_MAX:
			ret = gc2035_reg_writes(client,
					gc2035_sharpness_lv3_tbl);
			break;
		default:
			ret = gc2035_reg_writes(client,
					gc2035_sharpness_default_lv2_tbl);
			break;
		}
		if (ret)
			return ret;
		break;

	case V4L2_CID_POWER_LINE_FREQUENCY:

		if (ctrl->val > V4L2_CID_POWER_LINE_FREQUENCY_AUTO)
			return -EINVAL;

		gc2035->antibanding = ctrl->val;

		switch (gc2035->antibanding) {
		case V4L2_CID_POWER_LINE_FREQUENCY_50HZ:
			ret = gc2035_reg_writes(client,
					gc2035_antibanding_50z_tbl);
			break;
		case V4L2_CID_POWER_LINE_FREQUENCY_60HZ:
			ret = gc2035_reg_writes(client,
					gc2035_antibanding_60z_tbl);
			break;
		case V4L2_CID_POWER_LINE_FREQUENCY_AUTO:
		default:
			ret = gc2035_reg_writes(client,
					gc2035_antibanding_auto_tbl);
			break;
		}
		if (ret)
			return ret;
		break;

	case V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE:

		if (ctrl->val > V4L2_WHITE_BALANCE_CLOUDY)
			return -EINVAL;

		gc2035->whitebalance = ctrl->val;
		switch (gc2035->whitebalance) {
		case V4L2_WHITE_BALANCE_FLUORESCENT:
			gc2035_reg_write(client, 0xfe, 0x00);
			gc2035_reg_write(client, 0x82, 0xfc); /*turn off awb*/

			gc2035_reg_write(client, 0xb3, 0x72);
			gc2035_reg_write(client, 0xb4, 0x40);
			gc2035_reg_write(client, 0xb5, 0x5b);
			break;
		case V4L2_WHITE_BALANCE_DAYLIGHT:
			gc2035_reg_write(client, 0xfe, 0x00);
			gc2035_reg_write(client, 0x82, 0xfc); /*turn off awb*/

			gc2035_reg_write(client, 0xb3, 0x70);
			gc2035_reg_write(client, 0xb4, 0x40);
			gc2035_reg_write(client, 0xb5, 0x50);
			break;
		case V4L2_WHITE_BALANCE_CLOUDY:
			gc2035_reg_write(client, 0xfe, 0x00);
			gc2035_reg_write(client, 0x82, 0xfc); /*turn off awb*/

			gc2035_reg_write(client, 0xb3, 0x58);
			gc2035_reg_write(client, 0xb4, 0x40);
			gc2035_reg_write(client, 0xb5, 0x50);
			break;
		case V4L2_WHITE_BALANCE_INCANDESCENT:
			gc2035_reg_write(client, 0xfe, 0x00);
			gc2035_reg_write(client, 0x82, 0xfc); /*turn off awb*/

			gc2035_reg_write(client, 0xb3, 0xa0);
			gc2035_reg_write(client, 0xb4, 0x45);
			gc2035_reg_write(client, 0xb5, 0x40);
			break;
		case V4L2_WHITE_BALANCE_AUTO:
		default:    /*AWB*/
			gc2035_reg_write(client, 0xb3, 0x61);
			gc2035_reg_write(client, 0xb4, 0x40);
			gc2035_reg_write(client, 0xb5, 0x61);

			gc2035_reg_write(client, 0xfe, 0x00);
			gc2035_reg_write(client, 0x82, 0xfe); /*turn on awb*/
			break;
		}
		if (ret) {
			pr_err("Some error in AWB\n");
			return ret;
		}
		break;

	case V4L2_CID_CAMERA_FRAME_RATE:

		if (ctrl->val > FRAME_RATE_30)
			return -EINVAL;

		gc2035->framerate = ctrl->val;

		switch (gc2035->framerate) {
		case FRAME_RATE_5:
			ret = gc2035_reg_writes(client,
					gc2035_fps_5);
			break;
		case FRAME_RATE_7:
			ret = gc2035_reg_writes(client,
					gc2035_fps_7);
			break;
		case FRAME_RATE_10:
			ret = gc2035_reg_writes(client,
					gc2035_fps_10);
			break;
		case FRAME_RATE_15:
			ret = gc2035_reg_writes(client,
					gc2035_fps_15);
			break;
		case FRAME_RATE_20:
			ret = gc2035_reg_writes(client,
					gc2035_fps_20);
			break;
		case FRAME_RATE_25:
			ret = gc2035_reg_writes(client,
					gc2035_fps_25);
			break;
		case FRAME_RATE_30:
		case FRAME_RATE_AUTO:
		default:
			ret = gc2035_reg_writes(client,
					gc2035_fps_30);
			break;
		}
		if (ret)
			return ret;
		break;
	case V4L2_CID_CAM_PREVIEW_ONOFF:
	{
		if (ctrl->val)
			gc2035->runmode = CAM_RUNNING_MODE_PREVIEW;
		else
			gc2035->runmode = CAM_RUNNING_MODE_NOTREADY;

		break;
	}

	case V4L2_CID_CAM_CAPTURE:
		if (ctrl->val)
			gc2035->runmode = CAM_RUNNING_MODE_CAPTURE;
		else
			gc2035->runmode = CAM_RUNNING_MODE_NOTREADY;
		break;
	}

	return ret;
}


static long gc2035_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
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
static int gc2035_g_register(struct v4l2_subdev *sd,
			     struct v4l2_dbg_register *reg)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	if (reg->match.type != V4L2_CHIP_MATCH_I2C_ADDR || reg->size > 2)
		return -EINVAL;

	if (reg->match.addr != client->addr)
		return -ENODEV;

	reg->size = 2;
	if (gc2035_reg_read(client, reg->reg, &reg->val))
		return -EIO return 0;
}

static int gc2035_s_register(struct v4l2_subdev *sd,
			     struct v4l2_dbg_register *reg)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	if (reg->match.type != V4L2_CHIP_MATCH_I2C_ADDR || reg->size > 2)
		return -EINVAL;

	if (reg->match.addr != client->addr)
		return -ENODEV;

	if (gc2035_reg_write(client, reg->reg, reg->val))
		return -EIO;

	return 0;
}
#endif


static int gc2035_init(struct i2c_client *client)
{
	struct gc2035 *gc2035 = to_gc2035(client);
	int ret = 0;

	/* default brightness and contrast */
	gc2035->brightness = EV_DEFAULT;
	gc2035->contrast = CONTRAST_DEFAULT;
	gc2035->colorlevel = V4L2_COLORFX_NONE;
	gc2035->antibanding = V4L2_CID_POWER_LINE_FREQUENCY_AUTO;
	gc2035->whitebalance = V4L2_WHITE_BALANCE_AUTO;
	gc2035->framerate = FRAME_RATE_AUTO;
	gc2035->runmode = CAM_RUNNING_MODE_NOTREADY;
	gc2035->stream_status = -1;

	dev_dbg(&client->dev, "Sensor initialized\n");

out:
	return ret;
}

static int gc2035_s_power(struct v4l2_subdev *sd, int on)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct gc2035 *gc2035 = to_gc2035(client);
	struct soc_camera_subdev_desc *ssdd = soc_camera_i2c_to_desc(client);

	if (!on)
		return soc_camera_power_off(&client->dev, ssdd);

	gc2035->init_needed = 1;
	return soc_camera_power_on(&client->dev, ssdd);
}


/*
 * Interface active, can use i2c. If it fails, it can indeed mean, that
 *this wasn't our capture interface, so, we wait for the right one
 */
static int gc2035_video_probe(struct i2c_client *client)
{
	int ret = -1;   /* fail by default */
	u8 revision = 0, id_high, id_low;
	struct v4l2_subdev *subdev = i2c_get_clientdata(client);

	client->addr = 0x3c;

	ret = gc2035_s_power(subdev, 1);
	if (ret < 0)
		return ret;

	ret = gc2035_read_smbus(client, 0xf0, &id_high);
	if (ret) {
		dev_err(&client->dev, "Failure to detect GC2035 chip\n");
		goto out;
	}
	ret = gc2035_read_smbus(client, 0xf1, &id_low);
	if (ret) {
		dev_err(&client->dev, "Failure to detect GC2035 chip\n");
		goto out;
	}
	if ((id_high == 0x20) && (id_low == 0x35)) {
		ret = 0;
		dev_info(&client->dev, "Detected a GC2035 chip, revision %x\n",
			revision);
	} else
	    ret = -1;

out:
	return ret;
}

static void gc2035_video_remove(struct soc_camera_device *icd)
{
	dev_dbg(&icd->control, "Video removed: %p, %p\n",
		icd->parent, icd->vdev);
}

static struct v4l2_subdev_core_ops gc2035_subdev_core_ops = {
	.g_chip_ident = gc2035_g_chip_ident,
	.s_power = gc2035_s_power,
	.ioctl = gc2035_ioctl,
#ifdef CONFIG_VIDEO_ADV_DEBUG
	.g_register = gc2035_g_register,
	.s_register = gc2035_s_register,
#endif
};

static int gc2035_enum_fmt(struct v4l2_subdev *sd, unsigned int index,
			   enum v4l2_mbus_pixelcode *code)
{
	if (index >= ARRAY_SIZE(gc2035_fmts))
		return -EINVAL;

	*code = gc2035_fmts[index].code;
	return 0;
}

static int gc2035_enum_framesizes(struct v4l2_subdev *sd,
				  struct v4l2_frmsizeenum *fsize)
{
	if (fsize->index >= GC2035_SIZE_LAST)
		return -EINVAL;

	fsize->type = V4L2_FRMSIZE_TYPE_DISCRETE;
	fsize->pixel_format = V4L2_PIX_FMT_UYVY;

	fsize->discrete = gc2035_frmsizes[fsize->index];

	return 0;
}

/* we only support fixed frame rate */
static int gc2035_enum_frameintervals(struct v4l2_subdev *sd,
				      struct v4l2_frmivalenum *interval)
{
	int size;

	if (interval->index >= 1)
		return -EINVAL;

	interval->type = V4L2_FRMIVAL_TYPE_DISCRETE;

	size = gc2035_find_framesize(interval->width, interval->height);

	switch (size) {
	case GC2035_SIZE_UXGA:
		interval->discrete.numerator = 1;
		interval->discrete.denominator = 15;
		break;

	case GC2035_SIZE_VGA:
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

static int gc2035_g_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *param)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct gc2035 *gc2035 = to_gc2035(client);
	struct v4l2_captureparm *cparm;

	if (param->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;

	cparm = &param->parm.capture;

	memset(param, 0, sizeof(*param));
	param->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	cparm->capability = V4L2_CAP_TIMEPERFRAME;

	switch (gc2035->i_size) {
	case GC2035_SIZE_UXGA:
		cparm->timeperframe.numerator = 1;
		cparm->timeperframe.denominator = 15;
		break;
	case GC2035_SIZE_VGA:
	default:
		cparm->timeperframe.numerator = 1;
		cparm->timeperframe.denominator = 24;
		break;
	}

	return 0;
}
static int gc2035_s_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *param)
{
	/*
	 * FIXME: This just enforces the hardcoded framerates until this is
	 *flexible enough.
	 */
	return gc2035_g_parm(sd, param);
}

static struct v4l2_subdev_video_ops gc2035_subdev_video_ops = {
	.s_stream = gc2035_s_stream,
	.s_mbus_fmt = gc2035_s_fmt,
	.g_mbus_fmt = gc2035_g_fmt,
	.try_mbus_fmt = gc2035_try_fmt,
	.enum_mbus_fmt = gc2035_enum_fmt,
	.enum_mbus_fsizes = gc2035_enum_framesizes,
	.enum_framesizes = gc2035_enum_framesizes,
	.enum_frameintervals = gc2035_enum_frameintervals,
	.g_parm = gc2035_g_parm,
	.s_parm = gc2035_s_parm,
};
static int gc2035_g_skip_frames(struct v4l2_subdev *sd, u32 *frames)
{
	/* Quantity of initial bad frames to skip. Revisit. */
	/*Waitting for AWB stability,  avoid green color issue*/
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct gc2035 *gc2035 = to_gc2035(client);
	if (gc2035->init_needed) {
		/* frames need to be dropped for camera power on */
		*frames = 6;
		gc2035->init_needed = 0;
	} else {
		/* frames need to be dropped for camera mode switch */
		*frames = 2;
	}

	return 0;
}

static int gc2035_g_interface_parms(struct v4l2_subdev *sd,
				    struct v4l2_subdev_sensor_interface_parms
				    *parms)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct gc2035 *gc2035 = to_gc2035(client);
	u8 sclk_dividers;

	if (!parms)
		return -EINVAL;

	parms->if_type = gc2035->plat_parms->if_type;
	parms->if_mode = gc2035->plat_parms->if_mode;
	parms->parms = gc2035->plat_parms->parms;

	/* set the hs term time */
	parms->parms.serial.hs_term_time = 8;
	parms->parms.serial.hs_settle_time = 2;
	/*
	parms->parms.serial.hs_settle_time = 9;
	parms->parms.serial.hs_settle_time = 6;
	*/

	return 0;
}



static struct v4l2_subdev_sensor_ops gc2035_subdev_sensor_ops = {
	.g_skip_frames = gc2035_g_skip_frames,
	.g_interface_parms = gc2035_g_interface_parms,
};

static struct v4l2_subdev_ops gc2035_subdev_ops = {
	.core = &gc2035_subdev_core_ops,
	.video = &gc2035_subdev_video_ops,
	.sensor = &gc2035_subdev_sensor_ops,
};

static int gc2035_probe(struct i2c_client *client,
			const struct i2c_device_id *did)
{
	struct gc2035 *gc2035;
	struct soc_camera_subdev_desc *ssd = client->dev.platform_data;
	int ret, i;

	client->addr = 0x3c;

	if (!ssd) {
		dev_err(&client->dev, "GC2035: missing soc-camera data!\n");
		return -EINVAL;
	}

	if (!ssd->drv_priv) {
		dev_err(&client->dev,
			"GC2035 driver needs i/f platform data\n");
		return -EINVAL;
	}

	gc2035 = kzalloc(sizeof(struct gc2035), GFP_KERNEL);
	if (!gc2035)
		return -ENOMEM;

	v4l2_i2c_subdev_init(&gc2035->subdev, client, &gc2035_subdev_ops);

	gc2035->i_size = GC2035_SIZE_VGA;
	gc2035->i_fmt = 0;	/* First format in the list */
	gc2035->plat_parms = ssd->drv_priv;

	/* Initializing hdl with 12 controls (3 for future purposes) */
	v4l2_ctrl_handler_init(&gc2035->hdl, ARRAY_SIZE(gc2035_controls) + 10);
	if (gc2035->hdl.error)
		dev_dbg(&client->dev, "Error set during init itself! %d\n",
			gc2035->hdl.error);

	/* register standard controls */
	v4l2_ctrl_new_std(&gc2035->hdl, &gc2035_ctrl_ops, V4L2_CID_BRIGHTNESS,
			EV_MINUS_1, EV_PLUS_1, 1, EV_DEFAULT);

	v4l2_ctrl_new_std(&gc2035->hdl, &gc2035_ctrl_ops, V4L2_CID_CONTRAST,
			CONTRAST_MINUS_2, CONTRAST_PLUS_2, 1, CONTRAST_DEFAULT);

	v4l2_ctrl_new_std(&gc2035->hdl, &gc2035_ctrl_ops, V4L2_CID_SATURATION,
			GC2035_SATURATION_MIN, GC2035_SATURATION_MAX,
			GC2035_SATURATION_STEP, GC2035_SATURATION_DEF);

	v4l2_ctrl_new_std(&gc2035->hdl, &gc2035_ctrl_ops, V4L2_CID_SHARPNESS,
			GC2035_SHARPNESS_MIN, GC2035_SHARPNESS_MAX,
			GC2035_SHARPNESS_STEP, GC2035_SHARPNESS_DEF);

	if (gc2035->hdl.error) {
		dev_err(&client->dev,
			"Standard controls initialization error %d\n",
			gc2035->hdl.error);
		ret = gc2035->hdl.error;
		goto ctrl_hdl_err;
	}

	/* register standard menu controls */
	/*
	 * Max value of some controls communicates the supported enums for that
	 * control to the HAL.
	 */
	v4l2_ctrl_new_std_menu(&gc2035->hdl, &gc2035_ctrl_ops,
		V4L2_CID_COLORFX, (1 << V4L2_COLORFX_NONE) |
		(1 << V4L2_COLORFX_NEGATIVE) | (1 << V4L2_COLORFX_SEPIA) |
		(1 << V4L2_COLORFX_BW), 0, V4L2_COLORFX_NONE);

	v4l2_ctrl_new_std_menu(&gc2035->hdl, &gc2035_ctrl_ops,
		V4L2_CID_POWER_LINE_FREQUENCY,
		V4L2_CID_POWER_LINE_FREQUENCY_AUTO, 0,
		V4L2_CID_POWER_LINE_FREQUENCY_50HZ);

	v4l2_ctrl_new_std_menu(&gc2035->hdl, &gc2035_ctrl_ops,
		V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE,
		V4L2_WHITE_BALANCE_CLOUDY, 0,
		V4L2_WHITE_BALANCE_AUTO);

	if (gc2035->hdl.error) {
		dev_err(&client->dev,
			"Standard menu controls initialization error %d\n",
			gc2035->hdl.error);
		ret = gc2035->hdl.error;
		goto ctrl_hdl_err;
	}

	/* register custom controls */
	for (i = 0; i < ARRAY_SIZE(gc2035_controls); ++i)
		v4l2_ctrl_new_custom(&gc2035->hdl, &gc2035_controls[i], NULL);

	gc2035->subdev.ctrl_handler = &gc2035->hdl;
	if (gc2035->hdl.error) {
		ret = gc2035->hdl.error;
		goto ctrl_hdl_err;
	}

	ret = gc2035_video_probe(client);
	if (ret) {
		pr_err("gc2035: failed to probe the sensor\n");
		goto vid_probe_fail;
	}

	/* init the sensor here */
	ret = gc2035_init(client);
	if (ret) {
		dev_err(&client->dev, "Failed to initialize sensor\n");
		ret = -EINVAL;
	}

	return ret;

ctrl_hdl_err:
vid_probe_fail:
	v4l2_ctrl_handler_free(&gc2035->hdl);
	kfree(gc2035);
	pr_err("gc2035_probe failed with ret = %d\n", ret);
	return ret;

}

static int gc2035_remove(struct i2c_client *client)
{
	struct gc2035 *gc2035 = to_gc2035(client);
	struct soc_camera_device *icd = client->dev.platform_data;

	gc2035_video_remove(icd);
	client->driver = NULL;
	kfree(gc2035);

	return 0;
}

static const struct i2c_device_id gc2035_id[] = {
	{"gc2035", 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, gc2035_id);

static struct i2c_driver gc2035_i2c_driver = {
	.driver = {
		   .name = "gc2035",
		   },
	.probe = gc2035_probe,
	.remove = gc2035_remove,
	.id_table = gc2035_id,
};

static int __init gc2035_mod_init(void)
{
	return i2c_add_driver(&gc2035_i2c_driver);
}

static void __exit gc2035_mod_exit(void)
{
	i2c_del_driver(&gc2035_i2c_driver);
}

module_init(gc2035_mod_init);
module_exit(gc2035_mod_exit);

MODULE_DESCRIPTION("GalaxyCore GC2035 Camera driver");
MODULE_AUTHOR("Jun He <junhe@broadcom.com>");
MODULE_LICENSE("GPL v2");
