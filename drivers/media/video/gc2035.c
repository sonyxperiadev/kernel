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

#include <linux/videodev2.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/log2.h>
#include <linux/delay.h>
#include <linux/module.h>

#include <media/v4l2-subdev.h>
#include <media/v4l2-chip-ident.h>
#include <media/soc_camera.h>
#include <linux/videodev2_brcm.h>
#include "gc2035.h"


/* #define GC2035_DEBUG */

#define iprintk(format, arg...)	\
	printk(KERN_INFO"[%s]: "format"\n", __func__, ##arg)

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
	GC2035_SIZE_QVGA,	/*  320 x 240 */
	GC2035_SIZE_VGA,	/*  640 x 480 */
	GC2035_SIZE_1280x1024,	/*  1280 x 960 (1.2M) */
	GC2035_SIZE_UXGA,	/*  1600 x 1200 (2M) */
	GC2035_SIZE_LAST,
	GC2035_SIZE_MAX
};

enum  cam_running_mode {
	CAM_RUNNING_MODE_NOTREADY,
	CAM_RUNNING_MODE_PREVIEW,
	CAM_RUNNING_MODE_CAPTURE,
	CAM_RUNNING_MODE_CAPTURE_DONE,
	CAM_RUNNING_MODE_RECORDING,
};
enum  cam_running_mode runmode;

static const struct v4l2_frmsize_discrete gc2035_frmsizes[GC2035_SIZE_LAST] = {
	{320, 240},
	{640, 480},
	{1280, 1024},
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
	int flashmode;
};

static struct gc2035 *to_gc2035(const struct i2c_client *client)
{
	return container_of(i2c_get_clientdata(client), struct gc2035, subdev);
}


static const struct gc2035_timing_cfg timing_cfg_yuv[GC2035_SIZE_LAST] = {
	[GC2035_SIZE_QVGA] = {
			/*  Timing control  2624 x 1952 --> 2592 x 1944 */
			      .x_addr_start = 16,
			      .y_addr_start = 4,
			      .x_addr_end = 2607,
			      .y_addr_end = 1947,
			/*  Output image size */
			      .h_output_size = 320,
			      .v_output_size = 240,
			/*  ISP Windowing size 1296 x 972 --> 1280 x 960 */
			      .isp_h_offset = 8,
			      .isp_v_offset = 6,
			/*  Total size (+blanking) */
			      .h_total_size = 2200,
			      .v_total_size = 1280,
			/*  Sensor Read Binning Enabled */
			      .h_odd_ss_inc = 3,
			      .h_even_ss_inc = 1,
			      .v_odd_ss_inc = 3,
			      .v_even_ss_inc = 1,
#ifdef CONFIG_MACH_HAWAII_GARNET
				  .out_mode_sel = 0x01,
#else
			      .out_mode_sel = 0x07,
#endif
			      .sclk_dividers = 0x01,
			      .sys_mipi_clk = 0x11,
			       },
	[GC2035_SIZE_VGA] = {
			/*  Timing control  2624 x 1952 --> 2592 x 1944 */
			      .x_addr_start = 16,
			      .y_addr_start = 4,
			      .x_addr_end = 2607,
			      .y_addr_end = 1947,
			/*  Output image size */
			      .h_output_size = 640,
			      .v_output_size = 480,
			/*  ISP Windowing size  1296 x 972 --> 1280 x 960 */
			      .isp_h_offset = 8,
			      .isp_v_offset = 6,
			/*  Total size (+blanking) */
			      .h_total_size = 2200,
			      .v_total_size = 1280,
			/*  Sensor Read Binning Enabled */
			      .h_odd_ss_inc = 3,
			      .h_even_ss_inc = 1,
			      .v_odd_ss_inc = 3,
			      .v_even_ss_inc = 1,
#ifdef CONFIG_MACH_HAWAII_GARNET
				  .out_mode_sel = 0x01,
#else
			      .out_mode_sel = 0x07,
#endif
			      .sclk_dividers = 0x01,
			      .sys_mipi_clk = 0x11,
			       },
	[GC2035_SIZE_1280x1024] = {
			/*  Timing control  2624 x 1952 --> 2592 x 1944 */
			      .x_addr_start = 16,
			      .y_addr_start = 4,
			      .x_addr_end = 2607,
			      .y_addr_end = 1947,
			/*  Output image size */
			      .h_output_size = 1280,
			      .v_output_size = 960,
			/*  ISP Windowing size  1296 x 972 --> 1280 x 960 */
			      .isp_h_offset = 8,
			      .isp_v_offset = 6,
			/*  Total size (+blanking) */
			      .h_total_size = 2200,
			      .v_total_size = 1280,
			/*  Sensor Read Binning Enabled */
			      .h_odd_ss_inc = 3,
			      .h_even_ss_inc = 1,
			      .v_odd_ss_inc = 3,
			      .v_even_ss_inc = 1,
#ifdef CONFIG_MACH_HAWAII_GARNET
				  .out_mode_sel = 0x01,
#else
			      .out_mode_sel = 0x07,
#endif
			      .sclk_dividers = 0x01,
			      .sys_mipi_clk = 0x11,
			       },
	[GC2035_SIZE_UXGA] = {
			/*  Timing control  2624 x 1952 --> 2592 x 1944 */
			      .x_addr_start = 16,
			      .y_addr_start = 4,
			      .x_addr_end = 2607,
			      .y_addr_end = 1947,
			/*  Output image size */
			      .h_output_size = 1600,
			      .v_output_size = 1200,
			/*  ISP Windowing size	2592 x 1944 --> 2560 x 1920 */
			      .isp_h_offset = 16,
			      .isp_v_offset = 12,
			/*  Total size (+blanking) */
			      .h_total_size = 2844,
			      .v_total_size = 1968,
			/*  Sensor Read Binning Disabled */
			      .h_odd_ss_inc = 1,
			      .h_even_ss_inc = 1,
			      .v_odd_ss_inc = 1,
			      .v_even_ss_inc = 1,
#ifdef CONFIG_MACH_HAWAII_GARNET
				  .out_mode_sel = 0x00,
#else
			      .out_mode_sel = 0x06,
#endif
			      .sclk_dividers = 0x01,
			      .sys_mipi_clk = 0x12,
			      },

};


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
		printk(KERN_ERR
		"gc2035_write_smbus:Error write reg=0x%x, value=0x%x, ret=%d\n",
		reg, value, ret);

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

static const struct v4l2_queryctrl gc2035_controls[] = {
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
	 .maximum = (1 << FRAME_RATE_AUTO | 1 << FRAME_RATE_15 |
				1 << FRAME_RATE_30),
	 .step = 1,
	 .default_value = FRAME_RATE_AUTO,
	 },

};

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

printk(KERN_INFO "%s: index=%d err=%d\n", __func__, index, err);

	return 0;
}

static int gc2035_capture_setting(struct v4l2_subdev *sd)
{
	int ret;
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	ret = gc2035_reg_writes(client, capture_init_common);

	printk(KERN_INFO "gc2035 capture_init_common!");

	return ret;
}

static int gc2035_config_preview(struct v4l2_subdev *sd)
{
	int ret;
	u8 val = 0;
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	gc2035_reg_writes(client, gc2035_streamoff);

	/* set GC2035 to preview mode*/
	ret = gc2035_reg_writes(client, yuv422_init_common);



	gc2035_reg_writes(client, gc2035_streamon);

	printk(KERN_INFO "gc2035_config_preview!");

	return ret;
}

static int gc2035_config_capture(struct v4l2_subdev *sd)
{
	int ret = 0;
	struct i2c_client *client = v4l2_get_subdevdata(sd);



	printk(KERN_INFO "gc2035_config_capture!");

	gc2035_reg_writes(client, gc2035_streamoff);


	gc2035_capture_setting(sd);


	gc2035_reg_writes(client, gc2035_streamon);

	msleep(100);

	return ret;
}


static int stream_mode = -1;
static int gc2035_s_stream(struct v4l2_subdev *sd, int enable)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct gc2035 *gc2035 = to_gc2035(client);
	int ret = 0;
	printk(KERN_INFO "%s: enable:%d runmode:%d\n", __func__, enable,
	       runmode);

	if (enable == stream_mode)
		return ret;

	if (enable) {
		/* Start Streaming */
		ret = gc2035_reg_writes(client, gc2035_streamon);
		msleep(50);

	} else {
		/* Stop Streaming, Power Down*/
		ret = gc2035_reg_writes(client, gc2035_streamoff);
	}
	stream_mode = enable;

	return ret;
}

static int gc2035_set_bus_param(struct soc_camera_device *icd,
				unsigned long flags)
{
	/* TODO: Do the right thing here, and validate bus params */
	return 0;
}

static unsigned long gc2035_query_bus_param(struct soc_camera_device *icd)
{
	unsigned long flags = SOCAM_PCLK_SAMPLE_FALLING |
		SOCAM_HSYNC_ACTIVE_HIGH | SOCAM_VSYNC_ACTIVE_HIGH |
		SOCAM_DATA_ACTIVE_HIGH | SOCAM_MASTER;

	/* TODO: Do the right thing here, and validate bus params */

	flags |= SOCAM_DATAWIDTH_10;

	return flags;
}

static int gc2035_enum_input(struct soc_camera_device *icd,
			     struct v4l2_input *inp)
{
	struct soc_camera_link *icl = to_soc_camera_link(icd);
	struct v4l2_subdev_sensor_interface_parms *plat_parms;

	inp->type = V4L2_INPUT_TYPE_CAMERA;
	inp->std = V4L2_STD_UNKNOWN;
	strcpy(inp->name, "gc2035");

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
	if (runmode == CAM_RUNNING_MODE_PREVIEW) {
		ret = gc2035_reg_writes(client, configscript_common1);
		printk(KERN_INFO "gc2035 init configscript_common1!");
	}

	if (ret) {
		printk(KERN_ERR "Error configuring configscript_common1\n");
		return ret;
	}

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

	if (CAM_RUNNING_MODE_PREVIEW == runmode)
		gc2035_config_preview(sd);


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

	id->ident = V4L2_IDENT_OV5640;
	id->revision = 0;

	return 0;
}

static int gc2035_g_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct gc2035 *gc2035 = to_gc2035(client);

	dev_dbg(&client->dev, "gc2035_g_ctrl\n");

	switch (ctrl->id) {
	case V4L2_CID_CAMERA_BRIGHTNESS:
		ctrl->value = gc2035->brightness;
		break;
	case V4L2_CID_CAMERA_CONTRAST:
		ctrl->value = gc2035->contrast;
		break;
	case V4L2_CID_CAMERA_EFFECT:
		ctrl->value = gc2035->colorlevel;
		break;
	case V4L2_CID_SATURATION:
		ctrl->value = gc2035->saturation;
		break;
	case V4L2_CID_SHARPNESS:
		ctrl->value = gc2035->sharpness;
		break;
	case V4L2_CID_CAMERA_ANTI_BANDING:
		ctrl->value = gc2035->antibanding;
		break;
	case V4L2_CID_CAMERA_WHITE_BALANCE:
		ctrl->value = gc2035->whitebalance;
		break;
	case V4L2_CID_CAMERA_FRAME_RATE:
		ctrl->value = gc2035->framerate;
		break;
	case V4L2_CID_CAMERA_FLASH_MODE:
		ctrl->value = gc2035->flashmode;
		break;
	}

	return 0;
}

static int gc2035_s_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct gc2035 *gc2035 = to_gc2035(client);
	u8 ov_reg;
	int ret = 0;

	dev_dbg(&client->dev, "gc2035_s_ctrl\n");

	switch (ctrl->id) {
	case V4L2_CID_CAMERA_BRIGHTNESS:

		if (ctrl->value > EV_PLUS_1)
			return -EINVAL;

		gc2035->brightness = ctrl->value;
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
	case V4L2_CID_CAMERA_CONTRAST:

		if (ctrl->value > CONTRAST_PLUS_1)
			return -EINVAL;

		gc2035->contrast = ctrl->value;
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
	case V4L2_CID_CAMERA_EFFECT:

		if (ctrl->value > IMAGE_EFFECT_BNW)
			return -EINVAL;

		gc2035->colorlevel = ctrl->value;

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

		if (ctrl->value > GC2035_SATURATION_MAX)
			return -EINVAL;

		gc2035->saturation = ctrl->value;
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

		if (ctrl->value > GC2035_SHARPNESS_MAX)
			return -EINVAL;

		gc2035->sharpness = ctrl->value;
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

	case V4L2_CID_CAMERA_ANTI_BANDING:

		if (ctrl->value > ANTI_BANDING_60HZ)
			return -EINVAL;

		gc2035->antibanding = ctrl->value;

		switch (gc2035->antibanding) {
		case ANTI_BANDING_50HZ:
			ret = gc2035_reg_writes(client,
					gc2035_antibanding_50z_tbl);
			break;
		case ANTI_BANDING_60HZ:
			ret = gc2035_reg_writes(client,
					gc2035_antibanding_60z_tbl);
			break;
		default:
			ret = gc2035_reg_writes(client,
					gc2035_antibanding_auto_tbl);
			break;
		}
		if (ret)
			return ret;
		break;

	case V4L2_CID_CAMERA_WHITE_BALANCE:

		if (ctrl->value > WHITE_BALANCE_FLUORESCENT)
			return -EINVAL;

		gc2035->whitebalance = ctrl->value;
		break;

	case V4L2_CID_CAMERA_FRAME_RATE:

		if (ctrl->value > FRAME_RATE_30)
			return -EINVAL;

		gc2035->framerate = ctrl->value;

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

	case V4L2_CID_CAMERA_FLASH_MODE:
		break;

	case V4L2_CID_CAM_PREVIEW_ONOFF:
	{
			printk(KERN_INFO
				"gc2035 PREVIEW_ONOFF:%d runmode = %d\n",
				ctrl->value, runmode);
		if (ctrl->value)
			runmode = CAM_RUNNING_MODE_PREVIEW;
		else
			runmode = CAM_RUNNING_MODE_NOTREADY;

		break;
	}

	case V4L2_CID_CAM_CAPTURE:
		runmode = CAM_RUNNING_MODE_CAPTURE;
		gc2035_config_capture(sd);
		break;

	case V4L2_CID_CAM_CAPTURE_DONE:
		runmode = CAM_RUNNING_MODE_CAPTURE_DONE;
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

static struct soc_camera_ops gc2035_ops = {
	.set_bus_param = gc2035_set_bus_param,
	.query_bus_param = gc2035_query_bus_param,
	.enum_input = gc2035_enum_input,
	.controls = gc2035_controls,
	.num_controls = ARRAY_SIZE(gc2035_controls),
};

static int gc2035_init(struct i2c_client *client)
{
	struct gc2035 *gc2035 = to_gc2035(client);
	int ret = 0;
	printk(KERN_ERR "gc2035_init\n");
	ret = gc2035_reg_writes(client, configscript_common1);
	if (ret)
		goto out;

	/* Power Up, Start Streaming for AF Init*/
	ret = gc2035_reg_writes(client, gc2035_streamon);
	if (ret)
		goto out;
	/* Delay for sensor streaming*/
	msleep(20);


	/* default brightness and contrast */
	gc2035->brightness = EV_DEFAULT;
	gc2035->contrast = CONTRAST_DEFAULT;
	gc2035->colorlevel = IMAGE_EFFECT_NONE;
	gc2035->antibanding = ANTI_BANDING_AUTO;
	gc2035->whitebalance = WHITE_BALANCE_AUTO;
	gc2035->framerate = FRAME_RATE_AUTO;

	dev_dbg(&client->dev, "Sensor initialized\n");

out:
	return ret;
}


/*
 * Interface active, can use i2c. If it fails, it can indeed mean, that
 *this wasn't our capture interface, so, we wait for the right one
 */
static int gc2035_video_probe(struct soc_camera_device *icd,
			      struct i2c_client *client)
{
	unsigned long flags;
	int ret = 0;
	u8 revision = 0;
	printk(KERN_ERR "gc2035_video_probe\n");
	/*
	 * We must have a parent by now. And it cannot be a wrong one.
	 * So this entire test is completely redundant.
	 */
	client->addr = 0x3c;

	if (!icd->dev.parent ||
	    to_soc_camera_host(icd->dev.parent)->nr != icd->iface)
		return -ENODEV;
/*
	ret = gc2035_reg_read(client, 0x302A, &revision);
	if (ret) {
		dev_err(&client->dev, "Failure to detect GC2035 chip\n");
		goto out;
	}
	printk(KERN_ERR "GC2035 value read=%x\n", revision);

	revision &= 0xF;
*/
	flags = SOCAM_DATAWIDTH_8;

	dev_info(&client->dev, "Detected a GC2035 chip, revision %x\n",
		 revision);

	/* TODO: Do something like gc2035_init */

out:
	return ret;
}

static void gc2035_video_remove(struct soc_camera_device *icd)
{
	dev_dbg(&icd->dev, "Video removed: %p, %p\n",
		icd->dev.parent, icd->vdev);
}

static struct v4l2_subdev_core_ops gc2035_subdev_core_ops = {
	.g_chip_ident = gc2035_g_chip_ident,
	.g_ctrl = gc2035_g_ctrl,
	.s_ctrl = gc2035_s_ctrl,
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
	*frames = 5;

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
	parms->parms.serial.hs_term_time = 0x08;
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
	struct soc_camera_device *icd = client->dev.platform_data;
	struct soc_camera_link *icl;
	int ret;

	client->addr = 0x3c;

	printk(KERN_ERR "GC2035 probe\n");
	if (!icd) {
		dev_err(&client->dev, "GC2035: missing soc-camera data!\n");
		return -EINVAL;
	}

	icl = to_soc_camera_link(icd);
	if (!icl) {
		dev_err(&client->dev, "GC2035 driver needs platform data\n");
		return -EINVAL;
	}

	if (!icl->priv) {
		dev_err(&client->dev,
			"GC2035 driver needs i/f platform data\n");
		return -EINVAL;
	}

	gc2035 = kzalloc(sizeof(struct gc2035), GFP_KERNEL);
	if (!gc2035)
		return -ENOMEM;

	v4l2_i2c_subdev_init(&gc2035->subdev, client, &gc2035_subdev_ops);

	/* Second stage probe - when a capture adapter is there */
	icd->ops = &gc2035_ops;

	gc2035->i_size = GC2035_SIZE_VGA;
	gc2035->i_fmt = 0;	/* First format in the list */
	gc2035->plat_parms = icl->priv;

	ret = gc2035_video_probe(icd, client);
	if (ret) {
		icd->ops = NULL;
		kfree(gc2035);
		return ret;
	}

	/* init the sensor here */
	ret = gc2035_init(client);
	if (ret) {
		dev_err(&client->dev, "Failed to initialize sensor\n");
		ret = -EINVAL;
	}

	return ret;
}

static int gc2035_remove(struct i2c_client *client)
{
	struct gc2035 *gc2035 = to_gc2035(client);
	struct soc_camera_device *icd = client->dev.platform_data;

	icd->ops = NULL;
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

MODULE_DESCRIPTION("OmniVision GC2035 Camera driver");
MODULE_AUTHOR("Sergio Aguirre <saaguirre@ti.com>");
MODULE_LICENSE("GPL v2");
