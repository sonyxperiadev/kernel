/*
 * Galaxycore GC0313 sensor driver
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

#include "gc0313.h"

#define GC0313_ID 0xd0d0
/* GC0313 has only one fixed colorspace per pixelcode */
struct gc0313_datafmt {
	enum v4l2_mbus_pixelcode code;
	enum v4l2_colorspace colorspace;
};

struct gc0313_timing_cfg {
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

static const struct gc0313_datafmt gc0313_fmts[] = {
	/*
	 * Order important: first natively supported,
	 *second supported with a GPIO extender
	 */
	{V4L2_MBUS_FMT_UYVY8_2X8, V4L2_COLORSPACE_JPEG},
	{V4L2_MBUS_FMT_YUYV8_2X8, V4L2_COLORSPACE_JPEG},
};

enum gc0313_size {
	GC0313_SIZE_QVGA,
	GC0313_SIZE_VGA,
	GC0313_SIZE_LAST,
	GC0313_SIZE_MAX
};

static const struct v4l2_frmsize_discrete gc0313_frmsizes[GC0313_SIZE_LAST] = {
	{320, 240},
	{640, 480},
};


/* Find a data format by a pixel code in an array */
static int gc0313_find_datafmt(enum v4l2_mbus_pixelcode code)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(gc0313_fmts); i++)
		if (gc0313_fmts[i].code == code)
			break;

	/* If not found, select latest */
	if (i >= ARRAY_SIZE(gc0313_fmts))
		i = ARRAY_SIZE(gc0313_fmts) - 1;

	return i;
}

/* Find a frame size in an array */
static int gc0313_find_framesize(u32 width, u32 height)
{
	int i;

	for (i = 0; i < GC0313_SIZE_LAST; i++) {
		if ((gc0313_frmsizes[i].width >= width) &&
		    (gc0313_frmsizes[i].height >= height))
			break;
	}

	/* If not found, select biggest */
	if (i >= GC0313_SIZE_LAST)
		i = GC0313_SIZE_LAST - 1;

	return i;
}

struct gc0313 {
	struct v4l2_subdev subdev;
	struct v4l2_subdev_sensor_interface_parms *plat_parms;
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
};

static struct gc0313 *to_gc0313(const struct i2c_client *client)
{
	return container_of(i2c_get_clientdata(client), struct gc0313, subdev);
}


static const struct gc0313_timing_cfg timing_cfg_yuv[GC0313_SIZE_LAST] = {
	[GC0313_SIZE_QVGA] = {
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
	[GC0313_SIZE_VGA] = {
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

static const struct gc0313_timing_cfg timing_cfg_jpeg[GC0313_SIZE_LAST] = {
	[GC0313_SIZE_QVGA] = {
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
	[GC0313_SIZE_VGA] = {
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

#if 0
static struct v4l2_subdev_sensor_serial_parms mipi_cfgs[GC0313_SIZE_LAST] = {
	[GC0313_SIZE_QVGA] = {
			      .lanes = 1,
			      .channel = 0,
			      .phy_rate = (336 * 2 * 1000000),
			      .pix_clk = 21,	/* Revisit */
			      },
	[GC0313_SIZE_VGA] = {
			     .lanes = 1,
			     .channel = 0,
			     .phy_rate = (336 * 2 * 1000000),
			     .pix_clk = 21,	/* Revisit */
			     },
};
#endif

/**
 *gc0313_read_smbus - Read a value from a register in an gc0313 sensor device
 *@client: i2c driver client structure
 *@reg: register address / offset
 *@val: stores the value that gets read
 *
 * Read a value from a register in an gc0313 sensor device.
 * The value is returned in 'val'.
 * Returns zero if successful, or non-zero otherwise.
 */

/*
 * Low-level register I/O.
 *
 * Note that there are two versions of these.  On the XO 1, the
 * i2c controller only does SMBUS, so that's what we use.  The
 * ov7670 is not really an SMBUS device, though, so the communication
 * is not always entirely reliable.
 */
static int gc0313_read_smbus(struct i2c_client *client, unsigned char reg,
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

/**
 * Write a value to a register in gc0313 sensor device.
 *@client: i2c driver client structure.
 *@reg: Address of the register to read value from.
 *@val: Value to be written to a specific register.
 * Returns zero if successful, or non-zero otherwise.
 */
static int gc0313_write_smbus(struct i2c_client *client, unsigned char reg,
		unsigned char value)
{
	int ret = i2c_smbus_write_byte_data(client, reg, value);

	if (0 != ret)
		printk(KERN_INFO "Error write reg");

	return ret;
}



static const struct v4l2_queryctrl gc0313_controls[] = {
	{
	 .id = V4L2_CID_CAMERA_BRIGHTNESS,
	 .type = V4L2_CTRL_TYPE_INTEGER,
	 .name = "Brightness",
	 .minimum = EV_MINUS_1,
	 .maximum = EV_PLUS_1,
	 .step = 1,
	 .default_value = EV_DEFAULT,
	 },
	{
	 .id = V4L2_CID_CAMERA_CONTRAST,
	 .type = V4L2_CTRL_TYPE_INTEGER,
	 .name = "Contrast",
	 .minimum = CONTRAST_MINUS_1,
	 .maximum = CONTRAST_PLUS_1,
	 .step = 1,
	 .default_value = CONTRAST_DEFAULT,
	 },
	{
	 .id = V4L2_CID_CAMERA_EFFECT,
	 .type = V4L2_CTRL_TYPE_INTEGER,
	 .name = "Color Effects",
	 .minimum = IMAGE_EFFECT_NONE,
	 .maximum = (1 << IMAGE_EFFECT_NONE | 1 << IMAGE_EFFECT_SEPIA |
			 1 << IMAGE_EFFECT_BNW | 1 << IMAGE_EFFECT_NEGATIVE),
	 .step = 1,
	 .default_value = IMAGE_EFFECT_NONE,
	 },
	{
	 .id = V4L2_CID_CAMERA_ANTI_BANDING,
	 .type = V4L2_CTRL_TYPE_INTEGER,
	 .name = "Anti Banding",
	 .minimum = ANTI_BANDING_AUTO,
	 .maximum = ANTI_BANDING_60HZ,
	 .step = 1,
	 .default_value = ANTI_BANDING_AUTO,
	 },
	 {
	 .id = V4L2_CID_CAMERA_WHITE_BALANCE,
	 .type = V4L2_CTRL_TYPE_INTEGER,
	 .name = "White Balance",
	 .minimum = WHITE_BALANCE_AUTO,
	 .maximum = WHITE_BALANCE_FLUORESCENT,
	 .step = 1,
	 .default_value = WHITE_BALANCE_AUTO,
	 },
	 {
	 .id = V4L2_CID_CAMERA_FRAME_RATE,
	 .type = V4L2_CTRL_TYPE_INTEGER,
	 .name = "Framerate control",
	 .minimum = FRAME_RATE_AUTO,
	 .maximum = (1 << FRAME_RATE_AUTO | 1 << FRAME_RATE_5 |
			 1 << FRAME_RATE_10 | 1 << FRAME_RATE_15 |
			 1 << FRAME_RATE_25 | 1 << FRAME_RATE_30),
	 .step = 1,
	 .default_value = FRAME_RATE_AUTO,
	 },

};

/**
 * Initialize a list of gc0313 registers.
 * The list of registers is terminated by the pair of values
 *@client: i2c driver client structure.
 *@reglist[]: List of address of the registers to write data.
 * Returns zero if successful, or non-zero otherwise.
 */
static int gc0313_write_smbuss(struct i2c_client *client,
					const struct gc0313_reg reglist[])
{
	int err = 0, index;

	for (index = 0; (((reglist[index].reg != 0xff)
					|| (reglist[index].val != 0xff))
					&& (err == 0)); index++) {
		err |= gc0313_write_smbus(client, reglist[index].reg,
					reglist[index].val);
	}

	printk(KERN_INFO "%s: index=%d err=%d\n", __func__, index, err);
	return err;
}


static int gc0313_config_timing(struct i2c_client *client)
{
	struct gc0313 *gc0313 = to_gc0313(client);
	int ret = 0, i = gc0313->i_size;
	const struct gc0313_timing_cfg *timing_cfg;

	if (gc0313_fmts[gc0313->i_fmt].code == V4L2_MBUS_FMT_JPEG_1X8)
		timing_cfg = &timing_cfg_jpeg[i];
	else
		timing_cfg = &timing_cfg_yuv[i];


    /* TODO: Do the right thing here, and validate mipi timing params */

	return ret;
}

static int gc0313_s_stream(struct v4l2_subdev *sd, int enable)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int ret = 0;

	return ret;
}

static int gc0313_set_bus_param(struct soc_camera_device *icd,
				unsigned long flags)
{
	/* TODO: Do the right thing here, and validate bus params */
	return 0;
}

static unsigned long gc0313_query_bus_param(struct soc_camera_device *icd)
{
	unsigned long flags = SOCAM_PCLK_SAMPLE_FALLING |
		SOCAM_HSYNC_ACTIVE_HIGH | SOCAM_VSYNC_ACTIVE_HIGH |
		SOCAM_DATA_ACTIVE_HIGH | SOCAM_MASTER;

	/* TODO: Do the right thing here, and validate bus params */

	flags |= SOCAM_DATAWIDTH_10;

	return flags;
}

static int gc0313_enum_input(struct soc_camera_device *icd,
			     struct v4l2_input *inp)
{
	struct soc_camera_link *icl = to_soc_camera_link(icd);
	struct v4l2_subdev_sensor_interface_parms *plat_parms;

	inp->type = V4L2_INPUT_TYPE_CAMERA;
	inp->std = V4L2_STD_UNKNOWN;
	strcpy(inp->name, "gc0313");

	if (icl && icl->priv) {

		plat_parms = icl->priv;
		inp->status = 0;

		if (plat_parms->orientation == V4L2_SUBDEV_SENSOR_PORTRAIT)
			inp->status |= V4L2_IN_ST_HFLIP;

		if (plat_parms->facing == V4L2_SUBDEV_SENSOR_BACK)
			inp->status |= V4L2_IN_ST_BACK;

	}
	return 0;
}

static int gc0313_g_fmt(struct v4l2_subdev *sd, struct v4l2_mbus_framefmt *mf)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct gc0313 *gc0313 = to_gc0313(client);

	dev_dbg(&client->dev, "gc0313_g_fmt\n");

	mf->width = gc0313_frmsizes[gc0313->i_size].width;
	mf->height = gc0313_frmsizes[gc0313->i_size].height;
	mf->code = gc0313_fmts[gc0313->i_fmt].code;
	mf->colorspace = gc0313_fmts[gc0313->i_fmt].colorspace;
	mf->field = V4L2_FIELD_NONE;

	return 0;
}

static int gc0313_try_fmt(struct v4l2_subdev *sd, struct v4l2_mbus_framefmt *mf)
{
	int i_fmt;
	int i_size;

	i_fmt = gc0313_find_datafmt(mf->code);

	mf->code = gc0313_fmts[i_fmt].code;
	mf->colorspace = gc0313_fmts[i_fmt].colorspace;
	mf->field = V4L2_FIELD_NONE;

	i_size = gc0313_find_framesize(mf->width, mf->height);

	mf->width = gc0313_frmsizes[i_size].width;
	mf->height = gc0313_frmsizes[i_size].height;

	return 0;
}

static int gc0313_s_fmt(struct v4l2_subdev *sd, struct v4l2_mbus_framefmt *mf)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct gc0313 *gc0313 = to_gc0313(client);
	int ret = 0;

	dev_dbg(&client->dev, "gc0313_s_fmt\n");


	ret = gc0313_try_fmt(sd, mf);
	if (ret < 0)
		return ret;

	ret = gc0313_write_smbuss(client, configscript_common1);

	gc0313->i_size = gc0313_find_framesize(mf->width, mf->height);
	gc0313->i_fmt = gc0313_find_datafmt(mf->code);

	switch ((u32) gc0313_fmts[gc0313->i_fmt].code) {
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

	ret = gc0313_config_timing(client);
	if (ret)
		return ret;

	return ret;
}

static int gc0313_g_chip_ident(struct v4l2_subdev *sd,
			       struct v4l2_dbg_chip_ident *id)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	if (id->match.type != V4L2_CHIP_MATCH_I2C_ADDR)
		return -EINVAL;

	if (id->match.addr != client->addr)
		return -ENODEV;

	id->ident = V4L2_IDENT_OV7692;
	id->revision = 0;

	return 0;
}

static int gc0313_g_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct gc0313 *gc0313 = to_gc0313(client);

	dev_dbg(&client->dev, "gc0313_g_ctrl\n");

	switch (ctrl->id) {
	case V4L2_CID_CAMERA_BRIGHTNESS:
		ctrl->value = gc0313->brightness;
		break;
	case V4L2_CID_CAMERA_CONTRAST:
		ctrl->value = gc0313->contrast;
		break;
	case V4L2_CID_CAMERA_EFFECT:
		ctrl->value = gc0313->colorlevel;
		break;
	case V4L2_CID_SATURATION:
		ctrl->value = gc0313->saturation;
		break;
	case V4L2_CID_SHARPNESS:
		ctrl->value = gc0313->sharpness;
		break;
	case V4L2_CID_CAMERA_ANTI_BANDING:
		ctrl->value = gc0313->antibanding;
		break;
	case V4L2_CID_CAMERA_WHITE_BALANCE:
		ctrl->value = gc0313->whitebalance;
		break;
	case V4L2_CID_CAMERA_FRAME_RATE:
		ctrl->value = gc0313->framerate;
		break;
	}

	return 0;
}

static int gc0313_s_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct gc0313 *gc0313 = to_gc0313(client);
	u8 ov_reg;
	int ret = 0;

	dev_dbg(&client->dev, "gc0313_s_ctrl\n");

	switch (ctrl->id) {
	case V4L2_CID_CAMERA_BRIGHTNESS:

		if (ctrl->value > EV_PLUS_1)
			return -EINVAL;

		gc0313->brightness = ctrl->value;
		switch (gc0313->brightness) {
		case EV_MINUS_1:
			ret = gc0313_write_smbuss(client,
					gc0313_brightness_lv4_tbl);
			break;
		case EV_PLUS_1:
			ret = gc0313_write_smbuss(client,
					gc0313_brightness_lv0_tbl);
			break;
		default:
			ret = gc0313_write_smbuss(client,
					gc0313_brightness_lv2_default_tbl);
			break;
		}
		if (ret)
			return ret;
		break;
	case V4L2_CID_CAMERA_CONTRAST:

		if (ctrl->value > CONTRAST_PLUS_1)
			return -EINVAL;

		gc0313->contrast = ctrl->value;
		switch (gc0313->contrast) {
		case CONTRAST_MINUS_1:
			ret = gc0313_write_smbuss(client,
					gc0313_contrast_lv5_tbl);
			break;
		case CONTRAST_PLUS_1:
			ret = gc0313_write_smbuss(client,
					gc0313_contrast_lv0_tbl);
			break;
		default:
			ret = gc0313_write_smbuss(client,
					gc0313_contrast_default_lv3_tbl);
			break;
		}
		if (ret)
			return ret;
		break;
	case V4L2_CID_CAMERA_EFFECT:

		if (ctrl->value > IMAGE_EFFECT_BNW)
			return -EINVAL;

		gc0313->colorlevel = ctrl->value;

		switch (gc0313->colorlevel) {
		case IMAGE_EFFECT_BNW:
			ret = gc0313_write_smbuss(client,
					gc0313_effect_bw_tbl);
			break;
		case IMAGE_EFFECT_SEPIA:
			ret = gc0313_write_smbuss(client,
					gc0313_effect_sepia_tbl);
			break;
		case IMAGE_EFFECT_NEGATIVE:
			ret = gc0313_write_smbuss(client,
					gc0313_effect_negative_tbl);
			break;
		default:
			ret = gc0313_write_smbuss(client,
					gc0313_effect_normal_tbl);
			break;
		}
		if (ret)
			return ret;
		break;
	case V4L2_CID_SATURATION:

		if (ctrl->value > GC0313_SATURATION_MAX)
			return -EINVAL;

		gc0313->saturation = ctrl->value;
		switch (gc0313->saturation) {
		case GC0313_SATURATION_MIN:
			ret = gc0313_write_smbuss(client,
					gc0313_saturation_lv0_tbl);
			break;
		case GC0313_SATURATION_MAX:
			ret = gc0313_write_smbuss(client,
					gc0313_saturation_lv5_tbl);
			break;
		default:
			ret = gc0313_write_smbuss(client,
					gc0313_saturation_default_lv3_tbl);
			break;
		}
		if (ret)
			return ret;
		break;
	case V4L2_CID_SHARPNESS:

		if (ctrl->value > GC0313_SHARPNESS_MAX)
			return -EINVAL;

		gc0313->sharpness = ctrl->value;
		switch (gc0313->sharpness) {
		case GC0313_SHARPNESS_MIN:
			ret = gc0313_write_smbuss(client,
					gc0313_sharpness_lv0_tbl);
			break;
		case GC0313_SHARPNESS_MAX:
			ret = gc0313_write_smbuss(client,
					gc0313_sharpness_lv3_tbl);
			break;
		default:
			ret = gc0313_write_smbuss(client,
					gc0313_sharpness_default_lv2_tbl);
			break;
		}
		if (ret)
			return ret;
		break;

	case V4L2_CID_CAMERA_ANTI_BANDING:

		if (ctrl->value > ANTI_BANDING_60HZ)
			return -EINVAL;

		gc0313->antibanding = ctrl->value;

		switch (gc0313->antibanding) {
		case ANTI_BANDING_50HZ:
			ret = gc0313_write_smbuss(client,
					gc0313_antibanding_50z_tbl);
			break;
		case ANTI_BANDING_60HZ:
			ret = gc0313_write_smbuss(client,
					gc0313_antibanding_60z_tbl);
			break;
		default:
			ret = gc0313_write_smbuss(client,
					gc0313_antibanding_auto_tbl);
			break;
		}
		if (ret)
			return ret;
		break;

	case V4L2_CID_CAMERA_WHITE_BALANCE:

		if (ctrl->value > WHITE_BALANCE_FLUORESCENT)
			return -EINVAL;

		gc0313->whitebalance = ctrl->value;

		if (ret)
			return ret;

		switch (gc0313->whitebalance) {
		case WHITE_BALANCE_FLUORESCENT:
			ov_reg |= 0x01;
			ret = gc0313_write_smbuss(client,
					gc0313_wb_fluorescent);
			break;
		case WHITE_BALANCE_SUNNY:
			ov_reg |= 0x01;
			ret = gc0313_write_smbuss(client,
					gc0313_wb_daylight);
			break;
		case WHITE_BALANCE_CLOUDY:
			ov_reg |= 0x01;
			ret = gc0313_write_smbuss(client,
				gc0313_wb_cloudy);
			break;
		case WHITE_BALANCE_TUNGSTEN:
			ov_reg |= 0x01;
			ret = gc0313_write_smbuss(client,
					gc0313_wb_tungsten);
			break;
		default:
			ov_reg &= ~(0x01);
			ret = gc0313_write_smbuss(client,
						gc0313_wb_def);
			break;
		}
		if (ret)
			return ret;
		break;

	case V4L2_CID_CAMERA_FRAME_RATE:

		if (ctrl->value > FRAME_RATE_30)
			return -EINVAL;

		if ((gc0313->i_size < GC0313_SIZE_QVGA) ||
				(gc0313->i_size > GC0313_SIZE_VGA)) {
			if (ctrl->value == FRAME_RATE_30 ||
					ctrl->value == FRAME_RATE_AUTO)
				return 0;
			else
				return -EINVAL;
		}

		gc0313->framerate = ctrl->value;

		switch (gc0313->framerate) {
		case FRAME_RATE_5:
			ret = gc0313_write_smbuss(client,
					gc0313_fps_5);
			break;
		case FRAME_RATE_7:
			ret = gc0313_write_smbuss(client,
					gc0313_fps_7);
			break;
		case FRAME_RATE_10:
			ret = gc0313_write_smbuss(client,
					gc0313_fps_10);
			break;
		case FRAME_RATE_15:
			ret = gc0313_write_smbuss(client,
					gc0313_fps_15);
			break;
		case FRAME_RATE_20:
			ret = gc0313_write_smbuss(client,
					gc0313_fps_20);
			break;
		case FRAME_RATE_25:
			ret = gc0313_write_smbuss(client,
					gc0313_fps_25);
			break;
		case FRAME_RATE_30:
		case FRAME_RATE_AUTO:
		default:
			ret = gc0313_write_smbuss(client,
					gc0313_fps_30);
			break;
		}
		if (ret)
			return ret;
		break;
	}

	return ret;
}

static long gc0313_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
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
static int gc0313_g_register(struct v4l2_subdev *sd,
			     struct v4l2_dbg_register *reg)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	if (reg->match.type != V4L2_CHIP_MATCH_I2C_ADDR || reg->size > 2)
		return -EINVAL;

	if (reg->match.addr != client->addr)
		return -ENODEV;

	reg->size = 2;
	if (gc0313_read_smbus(client, reg->reg, &reg->val))
		return -EIO return 0;
}

static int gc0313_s_register(struct v4l2_subdev *sd,
			     struct v4l2_dbg_register *reg)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	if (reg->match.type != V4L2_CHIP_MATCH_I2C_ADDR || reg->size > 2)
		return -EINVAL;

	if (reg->match.addr != client->addr)
		return -ENODEV;

	if (gc0313_write_smbus(client, reg->reg, reg->val))
		return -EIO;

	return 0;
}
#endif

static struct soc_camera_ops gc0313_ops = {
	.set_bus_param = gc0313_set_bus_param,
	.query_bus_param = gc0313_query_bus_param,
	.enum_input = gc0313_enum_input,
	.controls = gc0313_controls,
	.num_controls = ARRAY_SIZE(gc0313_controls),
};


static int gc0313_init(struct i2c_client *client)
{
	struct gc0313 *gc0313 = to_gc0313(client);
	int ret = 0;

	printk(KERN_INFO "%s: Sensor initialized!\n", __func__);

	ret = gc0313_write_smbuss(client, configscript_common1);
	if (ret) {
		printk(KERN_INFO "%s: Sensor initialized failed!\n", __func__);
		goto out;
	}

	/* default brightness and contrast */
	gc0313->brightness = EV_DEFAULT;
	gc0313->contrast = CONTRAST_DEFAULT;
	gc0313->colorlevel = IMAGE_EFFECT_NONE;
	gc0313->antibanding = ANTI_BANDING_AUTO;
	gc0313->whitebalance = WHITE_BALANCE_AUTO;
	gc0313->framerate = FRAME_RATE_AUTO;

	dev_dbg(&client->dev, "Sensor initialized\n");

out:
	return ret;
}

/*
 * Interface active, can use i2c. If it fails, it can indeed mean, that
 *this wasn't our capture interface, so, we wait for the right one
 */
static int gc0313_video_probe(struct soc_camera_device *icd,
			      struct i2c_client *client)
{
	unsigned long flags;
	int ret = 0;
	u8 revision = 0;
	u16 sensor_id = 0;
	u8 value = 0;

	/*
	 * We must have a parent by now. And it cannot be a wrong one.
	 * So this entire test is completely redundant.
	 */
	if (!icd->dev.parent ||
	    to_soc_camera_host(icd->dev.parent)->nr != icd->iface)
		return -ENODEV;

	ret = gc0313_read_smbus(client, 0xf0, &sensor_id);
	ret = gc0313_read_smbus(client, 0xf0, &value);
	sensor_id = ((sensor_id&0xff) << 8) | (value&0xff);

	printk(KERN_INFO "%s: REG_MIDH = 0x%x\n", __func__, sensor_id);

	if (ret < 0)
		return ret;
	if (GC0313_ID != sensor_id)
		return -ENODEV;

	return ret;
}

static void gc0313_video_remove(struct soc_camera_device *icd)
{
	dev_dbg(&icd->dev, "Video removed: %p, %p\n",
		icd->dev.parent, icd->vdev);
}

static struct v4l2_subdev_core_ops gc0313_subdev_core_ops = {
	.g_chip_ident = gc0313_g_chip_ident,
	.g_ctrl = gc0313_g_ctrl,
	.s_ctrl = gc0313_s_ctrl,
	.ioctl = gc0313_ioctl,
#ifdef CONFIG_VIDEO_ADV_DEBUG
	.g_register = gc0313_g_register,
	.s_register = gc0313_s_register,
#endif
};

static int gc0313_enum_fmt(struct v4l2_subdev *sd, unsigned int index,
			   enum v4l2_mbus_pixelcode *code)
{
	if (index >= ARRAY_SIZE(gc0313_fmts))
		return -EINVAL;

	*code = gc0313_fmts[index].code;
	return 0;
}

static int gc0313_enum_framesizes(struct v4l2_subdev *sd,
				  struct v4l2_frmsizeenum *fsize)
{
	if (fsize->index >= GC0313_SIZE_LAST)
		return -EINVAL;

	fsize->type = V4L2_FRMSIZE_TYPE_DISCRETE;
	fsize->pixel_format = V4L2_PIX_FMT_UYVY;

	fsize->discrete = gc0313_frmsizes[fsize->index];

	return 0;
}

/* we only support fixed frame rate */
static int gc0313_enum_frameintervals(struct v4l2_subdev *sd,
				      struct v4l2_frmivalenum *interval)
{
	int size;

	if (interval->index >= 1)
		return -EINVAL;

	interval->type = V4L2_FRMIVAL_TYPE_DISCRETE;

	size = gc0313_find_framesize(interval->width, interval->height);

	switch (size) {
	case GC0313_SIZE_VGA:
	case GC0313_SIZE_QVGA:
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

static int gc0313_g_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *param)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct gc0313 *gc0313 = to_gc0313(client);
	struct v4l2_captureparm *cparm;

	if (param->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;

	cparm = &param->parm.capture;

	memset(param, 0, sizeof(*param));
	param->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	cparm->capability = V4L2_CAP_TIMEPERFRAME;

	switch (gc0313->i_size) {
	case GC0313_SIZE_VGA:
	case GC0313_SIZE_QVGA:
	default:
		cparm->timeperframe.numerator = 1;
		cparm->timeperframe.denominator = 24;
		break;
	}

	return 0;
}
static int gc0313_s_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *param)
{
	/*
	 * FIXME: This just enforces the hardcoded framerates until this is
	 *flexible enough.
	 */
	return gc0313_g_parm(sd, param);
}

static struct v4l2_subdev_video_ops gc0313_subdev_video_ops = {
	.s_stream = gc0313_s_stream,
	.s_mbus_fmt = gc0313_s_fmt,
	.g_mbus_fmt = gc0313_g_fmt,
	.try_mbus_fmt = gc0313_try_fmt,
	.enum_mbus_fmt = gc0313_enum_fmt,
	.enum_mbus_fsizes = gc0313_enum_framesizes,
	.enum_framesizes = gc0313_enum_framesizes,
	.enum_frameintervals = gc0313_enum_frameintervals,
	.g_parm = gc0313_g_parm,
	.s_parm = gc0313_s_parm,
};
static int gc0313_g_skip_frames(struct v4l2_subdev *sd, u32 *frames)
{
	/* Quantity of initial bad frames to skip. Revisit. */
	*frames = 5;

	return 0;
}

static int gc0313_g_interface_parms(struct v4l2_subdev *sd,
				    struct v4l2_subdev_sensor_interface_parms
				    *parms)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct gc0313 *gc0313 = to_gc0313(client);
	u8 sclk_dividers;

	if (!parms)
		return -EINVAL;

	parms->if_type = gc0313->plat_parms->if_type;
	parms->if_mode = gc0313->plat_parms->if_mode;
	parms->parms = gc0313->plat_parms->parms;

	/* set the hs term time */
	if (gc0313_fmts[gc0313->i_fmt].code == V4L2_MBUS_FMT_JPEG_1X8)
		sclk_dividers  = timing_cfg_jpeg[gc0313->i_size].sclk_dividers;
	else
		sclk_dividers = timing_cfg_yuv[gc0313->i_size].sclk_dividers;

	if (sclk_dividers == 0x01)
		parms->parms.serial.hs_term_time = 0x01;
	else
		parms->parms.serial.hs_term_time = 0x08;

	return 0;
}



static struct v4l2_subdev_sensor_ops gc0313_subdev_sensor_ops = {
	.g_skip_frames = gc0313_g_skip_frames,
	.g_interface_parms = gc0313_g_interface_parms,
};

static struct v4l2_subdev_ops gc0313_subdev_ops = {
	.core = &gc0313_subdev_core_ops,
	.video = &gc0313_subdev_video_ops,
	.sensor = &gc0313_subdev_sensor_ops,
};

static int gc0313_probe(struct i2c_client *client,
			const struct i2c_device_id *did)
{
	struct gc0313 *gc0313;
	struct soc_camera_device *icd = client->dev.platform_data;
	struct soc_camera_link *icl;
	int ret;

	client->addr = (0x42>>1);

	if (!icd) {
		dev_err(&client->dev, "GC0313: missing soc-camera data!\n");
		return -EINVAL;
	}

	icl = to_soc_camera_link(icd);
	if (!icl) {
		dev_err(&client->dev, "GC0313 driver needs platform data\n");
		return -EINVAL;
	}

	if (!icl->priv) {
		dev_err(&client->dev,
			"GC0313 driver needs i/f platform data\n");
		return -EINVAL;
	}

	gc0313 = kzalloc(sizeof(struct gc0313), GFP_KERNEL);
	if (!gc0313)
		return -ENOMEM;

	v4l2_i2c_subdev_init(&gc0313->subdev, client, &gc0313_subdev_ops);

	/* Second stage probe - when a capture adapter is there */
	icd->ops = &gc0313_ops;

	gc0313->i_size = GC0313_SIZE_VGA;
	gc0313->i_fmt = 0;	/* First format in the list */
	gc0313->plat_parms = icl->priv;


	ret = gc0313_video_probe(icd, client);
	if (ret) {
		icd->ops = NULL;
		kfree(gc0313);
		return ret;
	}

	msleep(20);

	/* init the sensor here */
	ret = gc0313_init(client);
	if (ret) {
		ret = -EINVAL;
		return ret;
	}

	return ret;
}

static int gc0313_remove(struct i2c_client *client)
{
	struct gc0313 *gc0313 = to_gc0313(client);
	struct soc_camera_device *icd = client->dev.platform_data;

	icd->ops = NULL;
	gc0313_video_remove(icd);
	client->driver = NULL;
	kfree(gc0313);

	return 0;
}

static const struct i2c_device_id gc0313_id[] = {
	{"gc0313", 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, gc0313_id);

static struct i2c_driver gc0313_i2c_driver = {
	.driver = {
		   .name = "gc0313",
		   },
	.probe = gc0313_probe,
	.remove = gc0313_remove,
	.id_table = gc0313_id,
};

static int __init gc0313_mod_init(void)
{
	return i2c_add_driver(&gc0313_i2c_driver);
}

static void __exit gc0313_mod_exit(void)
{
	i2c_del_driver(&gc0313_i2c_driver);
}

module_init(gc0313_mod_init);
module_exit(gc0313_mod_exit);

MODULE_DESCRIPTION("GC0313 Camera driver");
MODULE_AUTHOR("BroadcomTK");
MODULE_LICENSE("GPL v2");
