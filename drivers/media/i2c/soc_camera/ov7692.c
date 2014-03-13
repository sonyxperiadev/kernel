/*
 * OmniVision OV7692 sensor driver
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
#include <uapi/linux/time.h>
#include <uapi/linux/videodev2.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/log2.h>
#include <linux/delay.h>

#include <media/v4l2-ctrls.h>
#include <media/v4l2-subdev.h>
#include <media/v4l2-chip-ident.h>
#include <media/soc_camera.h>
#include <linux/videodev2_brcm.h>

#include <linux/gpio.h>

#include "ov7692.h"

/* OV7692 has only one fixed colorspace per pixelcode */
struct ov7692_datafmt {
	enum v4l2_mbus_pixelcode code;
	enum v4l2_colorspace colorspace;
};

struct ov7692_timing_cfg {
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

static const struct ov7692_datafmt ov7692_fmts[] = {
	/*
	 * Order important: first natively supported,
	 *second supported with a GPIO extender
	 */
	{V4L2_MBUS_FMT_UYVY8_2X8, V4L2_COLORSPACE_JPEG},
	{V4L2_MBUS_FMT_YUYV8_2X8, V4L2_COLORSPACE_JPEG},
};

enum ov7692_size {
	OV7692_SIZE_QVGA,	/*  320 x 240 */
	OV7692_SIZE_VGA,	/*  640 x 480 */
	OV7692_SIZE_LAST,
	OV7692_SIZE_MAX
};

static const struct v4l2_frmsize_discrete ov7692_frmsizes[OV7692_SIZE_LAST] = {
	{320, 240},
	{640, 480},
};


/* Find a data format by a pixel code in an array */
static int ov7692_find_datafmt(enum v4l2_mbus_pixelcode code)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(ov7692_fmts); i++)
		if (ov7692_fmts[i].code == code)
			break;

	/* If not found, select latest */
	if (i >= ARRAY_SIZE(ov7692_fmts))
		i = ARRAY_SIZE(ov7692_fmts) - 1;

	return i;
}

/* Find a frame size in an array */
static int ov7692_find_framesize(u32 width, u32 height)
{
	int i;

	for (i = 0; i < OV7692_SIZE_LAST; i++) {
		if ((ov7692_frmsizes[i].width >= width) &&
		    (ov7692_frmsizes[i].height >= height))
			break;
	}

	/* If not found, select biggest */
	if (i >= OV7692_SIZE_LAST)
		i = OV7692_SIZE_LAST - 1;

	return i;
}

struct ov7692 {
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
};

static struct ov7692 *to_ov7692(const struct i2c_client *client)
{
	return container_of(i2c_get_clientdata(client), struct ov7692, subdev);
}


static const struct ov7692_timing_cfg timing_cfg_yuv[OV7692_SIZE_LAST] = {
	[OV7692_SIZE_QVGA] = {
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
	[OV7692_SIZE_VGA] = {
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

static const struct ov7692_timing_cfg timing_cfg_jpeg[OV7692_SIZE_LAST] = {
	[OV7692_SIZE_QVGA] = {
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
	[OV7692_SIZE_VGA] = {
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
 *ov7692_read_smbus - Read a value from a register in an ov7692 sensor device
 *@client: i2c driver client structure
 *@reg: register address / offset
 *@val: stores the value that gets read
 *
 * Read a value from a register in an ov7692 sensor device.
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
static int ov7692_read_smbus(struct i2c_client *client, unsigned char reg,
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
 * Write a value to a register in ov7692 sensor device.
 *@client: i2c driver client structure.
 *@reg: Address of the register to read value from.
 *@val: Value to be written to a specific register.
 * Returns zero if successful, or non-zero otherwise.
 */
static int ov7692_write_smbus(struct i2c_client *client, unsigned char reg,
		unsigned char value)
{
	int ret = i2c_smbus_write_byte_data(client, reg, value);

	if (reg == 0X12 && (value & 0x80))
		msleep(10);  /* Wait for reset to run */

    //printk(KERN_ERR "ov7692_write_smbus: reg=0x%x, value=0x%x, ret=%d\n",reg,value,ret);

    if(0 != ret)
    {
		printk(KERN_ERR "ov7692_write_smbus: Error on write reg=0x%x, value=0x%x, ret=%d\n",reg,value,ret);
    }

	return ret;
}



/**
 * Initialize a list of ov7692 registers.
 * The list of registers is terminated by the pair of values
 *@client: i2c driver client structure.
 *@reglist[]: List of address of the registers to write data.
 * Returns zero if successful, or non-zero otherwise.
 */
 static int ov7692_write_smbuss(struct i2c_client *client,
                  const struct ov7692_reg reglist[])
{
     int err = 0, index;

     for (index = 0; (((reglist[index].reg!=0xff)||(reglist[index].val!=0xff)) && (err == 0));index++) {
        err |=
            ov7692_write_smbus(client, reglist[index].reg,
                     reglist[index].val);
     }

     printk(KERN_INFO "%s: index=%d err=%d\n", __func__,index, err);
     return err;
}


static int ov7692_config_timing(struct i2c_client *client)
{
	struct ov7692 *ov7692 = to_ov7692(client);
	int ret = 0, i = ov7692->i_size;
	const struct ov7692_timing_cfg *timing_cfg;

	if (ov7692_fmts[ov7692->i_fmt].code == V4L2_MBUS_FMT_JPEG_1X8)
		timing_cfg = &timing_cfg_jpeg[i];
	else
		timing_cfg = &timing_cfg_yuv[i];


    /* TODO: Do the right thing here, and validate mipi timing params */

	return ret;
}

static int ov7692_s_stream(struct v4l2_subdev *sd, int enable)
{
	return 0;
}


static int initneeded = -1;
static int ov7692_g_fmt(struct v4l2_subdev *sd, struct v4l2_mbus_framefmt *mf)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ov7692 *ov7692 = to_ov7692(client);

	dev_dbg(&client->dev, "ov7692_g_fmt\n");

	mf->width = ov7692_frmsizes[ov7692->i_size].width;
	mf->height = ov7692_frmsizes[ov7692->i_size].height;
	mf->code = ov7692_fmts[ov7692->i_fmt].code;
	mf->colorspace = ov7692_fmts[ov7692->i_fmt].colorspace;
	mf->field = V4L2_FIELD_NONE;

	return 0;
}

static int ov7692_try_fmt(struct v4l2_subdev *sd, struct v4l2_mbus_framefmt *mf)
{
	int i_fmt;
	int i_size;

	i_fmt = ov7692_find_datafmt(mf->code);

	mf->code = ov7692_fmts[i_fmt].code;
	mf->colorspace = ov7692_fmts[i_fmt].colorspace;
	mf->field = V4L2_FIELD_NONE;

	i_size = ov7692_find_framesize(mf->width, mf->height);

	mf->width = ov7692_frmsizes[i_size].width;
	mf->height = ov7692_frmsizes[i_size].height;

	return 0;
}

static int ov7692_s_fmt(struct v4l2_subdev *sd, struct v4l2_mbus_framefmt *mf)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ov7692 *ov7692 = to_ov7692(client);
	int ret = 0;

	dev_dbg(&client->dev, "ov7692_s_fmt\n");


	ret = ov7692_try_fmt(sd, mf);
	if (ret < 0)
		return ret;
	/*To avoide reentry init sensor when captrue, remove from here  */
	/*ret = ov7692_write_smbuss(client, configscript_common1);*/
	if (initneeded > 0) {
		ret = ov7692_write_smbuss(client, hawaii_init_common);
		initneeded = 0;
	}

	ov7692->i_size = ov7692_find_framesize(mf->width, mf->height);
	ov7692->i_fmt = ov7692_find_datafmt(mf->code);

	switch ((u32) ov7692_fmts[ov7692->i_fmt].code) {
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

	ret = ov7692_config_timing(client);
	if (ret)
		return ret;

	return ret;
}

static int ov7692_g_chip_ident(struct v4l2_subdev *sd,
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

void ov7692_check_mounting(struct i2c_client *client, struct v4l2_ctrl *ctrl)
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

static int ov7692_s_ctrl(struct v4l2_ctrl *ctrl);
static int ov7692_g_volatile_ctrl(struct v4l2_ctrl *ctrl);

static const struct v4l2_ctrl_ops ov7692_ctrl_ops = {
	.s_ctrl = ov7692_s_ctrl,
	.g_volatile_ctrl = ov7692_g_volatile_ctrl,
};

static const struct v4l2_ctrl_config ov7692_controls[] = {
	 {
		.ops = &ov7692_ctrl_ops,
		.id = V4L2_CID_CAMERA_FRAME_RATE,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.name = "Framerate control",
		.min = FRAME_RATE_AUTO,
		.max = (1 << FRAME_RATE_AUTO |
				1 << FRAME_RATE_30),
		.step = 1,
		.def = FRAME_RATE_AUTO,
		.flags = V4L2_CTRL_FLAG_VOLATILE,
	 },
	 {
		 .ops = &ov7692_ctrl_ops,
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

static int ov7692_g_volatile_ctrl(struct v4l2_ctrl *ctrl)
{
	struct ov7692 *ov7692 = container_of(ctrl->handler,
						struct ov7692, hdl);
	struct v4l2_subdev *sd = &ov7692->subdev;
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	dev_dbg(&client->dev, "ov7692_g_ctrl\n");
	switch (ctrl->id) {
	case V4L2_CID_BRIGHTNESS:
		ctrl->val = ov7692->brightness;
		break;
	case V4L2_CID_CONTRAST:
		ctrl->val = ov7692->contrast;
		break;
	case V4L2_CID_COLORFX:
		ctrl->val = ov7692->colorlevel;
		break;
	case V4L2_CID_POWER_LINE_FREQUENCY:
		ctrl->val = ov7692->antibanding;
		break;
	case V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE:
		ctrl->val = ov7692->whitebalance;
		break;
	case V4L2_CID_CAMERA_FRAME_RATE:
		ctrl->val = ov7692->framerate;
		break;
	case V4L2_CID_CAM_MOUNTING:
		ov7692_check_mounting(client, ctrl);
		break;
	}

	return 0;
}
#if 0
static int ov7692_preview_start(struct i2c_client *client)
{
	int ret = 0;
	ret = ov7692_write_smbuss(client, configscript_common1);
	return ret;
}
#endif
static int ov7692_s_ctrl(struct v4l2_ctrl *ctrl)
{
	struct ov7692 *ov7692 = container_of(ctrl->handler,
						struct ov7692, hdl);
	struct v4l2_subdev *sd = &ov7692->subdev;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	u8 ov_reg;
	int ret = 0;

	switch (ctrl->id) {
	case V4L2_CID_BRIGHTNESS:

		if (ctrl->val > EV_PLUS_1)
			return -EINVAL;

		ov7692->brightness = ctrl->val;
		switch (ov7692->brightness) {
		case EV_MINUS_1:
			ret = ov7692_write_smbuss(client,
					ov7692_brightness_lv4_tbl);
			break;
		case EV_PLUS_1:
			ret = ov7692_write_smbuss(client,
					ov7692_brightness_lv0_tbl);
			break;
		default:
			ret = ov7692_write_smbuss(client,
					ov7692_brightness_lv2_default_tbl);
			break;
		}
		if (ret)
			return ret;
		break;
	case V4L2_CID_CONTRAST:

		if (ctrl->val > CONTRAST_PLUS_2)
			return -EINVAL;

		ov7692->contrast = ctrl->val;
		switch (ov7692->contrast) {
		case CONTRAST_MINUS_2:
			ret = ov7692_write_smbuss(client,
					ov7692_contrast_lv1_tbl);
			break;
		case CONTRAST_MINUS_1:
			ret = ov7692_write_smbuss(client,
					ov7692_contrast_lv2_tbl);
			break;
		case CONTRAST_PLUS_1:
			ret = ov7692_write_smbuss(client,
					ov7692_contrast_lv4_tbl);
			break;
		case CONTRAST_PLUS_2:
			ret = ov7692_write_smbuss(client,
					ov7692_contrast_lv5_tbl);
			break;
		default:
			ret = ov7692_write_smbuss(client,
					ov7692_contrast_default_lv3_tbl);
			break;
		}
		if (ret)
			return ret;
		break;
	case V4L2_CID_COLORFX:

		if (ctrl->val > V4L2_COLORFX_NEGATIVE)
			return -EINVAL;

		ov7692->colorlevel = ctrl->val;

		switch (ov7692->colorlevel) {
		case V4L2_COLORFX_BW:
			ret = ov7692_write_smbuss(client,
					ov7692_effect_bw_tbl);
			break;
		case V4L2_COLORFX_SEPIA:
			ret = ov7692_write_smbuss(client,
					ov7692_effect_sepia_tbl);
			break;
		case V4L2_COLORFX_NEGATIVE:
			ret = ov7692_write_smbuss(client,
					ov7692_effect_negative_tbl);
			break;
		case V4L2_COLORFX_NONE:
		default:
			ret = ov7692_write_smbuss(client,
					ov7692_effect_normal_tbl);
			break;
		}
		if (ret)
			return ret;
		break;

	case V4L2_CID_POWER_LINE_FREQUENCY:

		if (ctrl->val > V4L2_CID_POWER_LINE_FREQUENCY_AUTO)
			return -EINVAL;

		ov7692->antibanding = ctrl->val;

		switch (ov7692->antibanding) {
		case V4L2_CID_POWER_LINE_FREQUENCY_DISABLED:
			break;
		case V4L2_CID_POWER_LINE_FREQUENCY_50HZ:
			ret = ov7692_write_smbuss(client,
					ov7692_antibanding_50z_tbl);
			break;
		case V4L2_CID_POWER_LINE_FREQUENCY_60HZ:
			ret = ov7692_write_smbuss(client,
					ov7692_antibanding_60z_tbl);
			break;
		case V4L2_CID_POWER_LINE_FREQUENCY_AUTO:
		default:
			ret = ov7692_write_smbuss(client,
					ov7692_antibanding_auto_tbl);
			break;
		}
		if (ret)
			return ret;
		break;

	case V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE:

		if (ctrl->val > V4L2_WHITE_BALANCE_CLOUDY)
			return -EINVAL;

		ov7692->whitebalance = ctrl->val;

		switch (ov7692->whitebalance) {
		case V4L2_WHITE_BALANCE_FLUORESCENT:
			ov_reg |= 0x01;
			ret = ov7692_write_smbuss(client,
					ov7692_wb_fluorescent);
			break;
		case V4L2_WHITE_BALANCE_DAYLIGHT:
			ov_reg |= 0x01;
			ret = ov7692_write_smbuss(client,
					ov7692_wb_daylight);
			break;
		case V4L2_WHITE_BALANCE_CLOUDY:
			ov_reg |= 0x01;
			ret = ov7692_write_smbuss(client,
				ov7692_wb_cloudy);
			break;
		case V4L2_WHITE_BALANCE_INCANDESCENT:
			ov_reg |= 0x01;
			ret = ov7692_write_smbuss(client,
					ov7692_wb_tungsten);
			break;
		case V4L2_WHITE_BALANCE_MANUAL:
			break;
		case V4L2_WHITE_BALANCE_AUTO:
		default:
			ov_reg &= ~(0x01);
			ret = ov7692_write_smbuss(client,
						ov7692_wb_def);
			break;
		}
		if (ret)
			return ret;
		break;

	case V4L2_CID_CAMERA_FRAME_RATE:

		if (ctrl->val > FRAME_RATE_30)
			return -EINVAL;

		if ((ov7692->i_size < OV7692_SIZE_QVGA) ||
				(ov7692->i_size > OV7692_SIZE_VGA))
		{
			if (ctrl->val == FRAME_RATE_30 ||
					ctrl->val == FRAME_RATE_AUTO)
				return 0;
			else
				return -EINVAL;
		}

		ov7692->framerate = ctrl->val;

		switch (ov7692->framerate) {
		/*case FRAME_RATE_5:
			ret = ov7692_write_smbuss(client,
					ov7692_fps_5);
			break;
		case FRAME_RATE_7:
			ret = ov7692_write_smbuss(client,
					ov7692_fps_7);
			break;
		case FRAME_RATE_10:
			ret = ov7692_write_smbuss(client,
					ov7692_fps_10);
			break;
		case FRAME_RATE_15:
			ret = ov7692_write_smbuss(client,
					ov7692_fps_15);
			break;
		case FRAME_RATE_20:
			ret = ov7692_write_smbuss(client,
					ov7692_fps_20);
			break;
		case FRAME_RATE_25:
			ret = ov7692_write_smbuss(client,
					ov7692_fps_25);
			break;*/
		case FRAME_RATE_10:
			ret = ov7692_write_smbuss(client,
					ov7692_fps_10);
			break;
		case FRAME_RATE_15:
			ret = ov7692_write_smbuss(client,
					ov7692_fps_15);
			break;
		case FRAME_RATE_30:
		case FRAME_RATE_AUTO:
		default:
			ret = ov7692_write_smbuss(client,
					ov7692_fps_30);
			break;
		}
		if (ret)
			return ret;
		break;
	}

	return ret;
}

static long ov7692_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
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
static int ov7692_g_register(struct v4l2_subdev *sd,
			     struct v4l2_dbg_register *reg)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	if (reg->match.type != V4L2_CHIP_MATCH_I2C_ADDR || reg->size > 2)
		return -EINVAL;

	if (reg->match.addr != client->addr)
		return -ENODEV;

	reg->size = 2;
	if (ov7692_read_smbus(client, reg->reg, &reg->val))
		return -EIO return 0;
}

static int ov7692_s_register(struct v4l2_subdev *sd,
			     struct v4l2_dbg_register *reg)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	if (reg->match.type != V4L2_CHIP_MATCH_I2C_ADDR || reg->size > 2)
		return -EINVAL;

	if (reg->match.addr != client->addr)
		return -ENODEV;

	if (ov7692_write_smbus(client, reg->reg, reg->val))
		return -EIO;

	return 0;
}
#endif


static int ov7692_init(struct i2c_client *client)
{
	struct ov7692 *ov7692 = to_ov7692(client);
	int ret = 0;

	pr_info("%s: Sensor initialized!\n", __func__);

	ret = ov7692_write_smbuss(client, configscript_common1);
	if (ret)
	{
		pr_info("%s: Sensor initialized failed!\n", __func__);
		goto out;
	}

	/* default brightness and contrast */
	ov7692->brightness	= EV_DEFAULT;
	ov7692->contrast	= CONTRAST_DEFAULT;
	ov7692->colorlevel	= V4L2_COLORFX_NONE;
	ov7692->antibanding	= V4L2_CID_POWER_LINE_FREQUENCY_AUTO;
	ov7692->whitebalance	= V4L2_WHITE_BALANCE_AUTO;
	ov7692->framerate	= FRAME_RATE_AUTO;

	dev_dbg(&client->dev, "Sensor initialized\n");

out:
	return ret;
}

static int ov7692_s_power(struct v4l2_subdev *sd, int on)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct soc_camera_subdev_desc *ssdd = soc_camera_i2c_to_desc(client);
	if (!on)
		return soc_camera_power_off(&client->dev, ssdd);
	initneeded = 1;
	return soc_camera_power_on(&client->dev, ssdd);
}

/*
 * Interface active, can use i2c. If it fails, it can indeed mean, that
 *this wasn't our capture interface, so, we wait for the right one
 */
static int ov7692_video_probe(struct i2c_client *client)
{
	int ret = 0;
	u8 value = 0;

	struct v4l2_subdev *subdev = i2c_get_clientdata(client);

	ret = ov7692_s_power(subdev, 1);
	if (ret < 0)
		return ret;

	ret = ov7692_read_smbus(client, 0x1c, &value);
	pr_info("%s: REG_MIDH = 0x%x\n", __func__, value);
	if (ret < 0)
		goto ei2c;
	if (value != 0x7f) { /* OV manuf. id. */
		ret = -ENODEV;
		goto ei2c;
	}

	ret = ov7692_read_smbus(client, 0x1d, &value);
	pr_info("%s: REG_MIDL = 0x%x\n", __func__, value);
	if (ret < 0)
		goto ei2c;
	if (value != 0xa2) {
		ret = -ENODEV;
		goto ei2c;
	}

	/*
	 * OK, we know we have an OmniVision chip...but which one?
	 */
	ret = ov7692_read_smbus(client, 0x0a, &value);
	pr_info("%s: REG_PID = 0x%x\n", __func__, value);
	if (ret < 0)
		goto ei2c;
	if (value != 0x76) { /* PID + VER = 0x76 / 0x92 */
		ret = -ENODEV;
		goto ei2c;
	}

	ret = ov7692_read_smbus(client, 0x0b, &value);
	pr_info("%s: REG_VER = 0x%x\n", __func__, value);
	if (ret < 0)
		goto ei2c;
	if (value != 0x92) { /* PID + VER = 0x76 / 0x92 */
		ret = -ENODEV;
		goto ei2c;
	}

	/* Init the sensor here */
	ret = ov7692_init(client);
	if (ret)
		dev_err(&client->dev, "Failed to initialize camera\n");

ei2c:
	ov7692_s_power(subdev, 0);
	return ret;
}

static void ov7692_video_remove(struct soc_camera_device *icd)
{
	dev_dbg(&icd->control, "Video removed: %p, %p\n",
		icd->parent, icd->vdev);
}

static struct v4l2_subdev_core_ops ov7692_subdev_core_ops = {
	.g_chip_ident = ov7692_g_chip_ident,
	.s_power = ov7692_s_power,
	.ioctl = ov7692_ioctl,
#ifdef CONFIG_VIDEO_ADV_DEBUG
	.g_register = ov7692_g_register,
	.s_register = ov7692_s_register,
#endif
};

static int ov7692_enum_fmt(struct v4l2_subdev *sd, unsigned int index,
			   enum v4l2_mbus_pixelcode *code)
{
	if (index >= ARRAY_SIZE(ov7692_fmts))
		return -EINVAL;

	*code = ov7692_fmts[index].code;
	return 0;
}

static int ov7692_enum_framesizes(struct v4l2_subdev *sd,
				  struct v4l2_frmsizeenum *fsize)
{
	if (fsize->index >= OV7692_SIZE_LAST)
		return -EINVAL;

	fsize->type = V4L2_FRMSIZE_TYPE_DISCRETE;
	fsize->pixel_format = V4L2_PIX_FMT_UYVY;

	fsize->discrete = ov7692_frmsizes[fsize->index];

	return 0;
}

/* we only support fixed frame rate */
static int ov7692_enum_frameintervals(struct v4l2_subdev *sd,
				      struct v4l2_frmivalenum *interval)
{
	int size;

	if (interval->index >= 1)
		return -EINVAL;

	interval->type = V4L2_FRMIVAL_TYPE_DISCRETE;

	size = ov7692_find_framesize(interval->width, interval->height);

	switch (size) {
	case OV7692_SIZE_VGA:
	case OV7692_SIZE_QVGA:
	default:
		interval->discrete.numerator = 1;
		interval->discrete.denominator = 24;
		break;
	}

	return 0;
}

static int ov7692_g_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *param)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ov7692 *ov7692 = to_ov7692(client);
	struct v4l2_captureparm *cparm;

	if (param->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;

	cparm = &param->parm.capture;

	memset(param, 0, sizeof(*param));
	param->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	cparm->capability = V4L2_CAP_TIMEPERFRAME;

	switch (ov7692->i_size) {
	case OV7692_SIZE_VGA:
	case OV7692_SIZE_QVGA:
	default:
		cparm->timeperframe.numerator = 1;
		cparm->timeperframe.denominator = 24;
		break;
	}

	return 0;
}
static int ov7692_s_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *param)
{
	/*
	 * FIXME: This just enforces the hardcoded framerates until this is
	 *flexible enough.
	 */
	return ov7692_g_parm(sd, param);
}

static struct v4l2_subdev_video_ops ov7692_subdev_video_ops = {
	.s_stream = ov7692_s_stream,
	.s_mbus_fmt = ov7692_s_fmt,
	.g_mbus_fmt = ov7692_g_fmt,
	.try_mbus_fmt = ov7692_try_fmt,
	.enum_mbus_fmt = ov7692_enum_fmt,
	.enum_mbus_fsizes = ov7692_enum_framesizes,
	.enum_framesizes = ov7692_enum_framesizes,
	.enum_frameintervals = ov7692_enum_frameintervals,
	.g_parm = ov7692_g_parm,
	.s_parm = ov7692_s_parm,
};
static int ov7692_g_skip_frames(struct v4l2_subdev *sd, u32 *frames)
{
	/* Quantity of initial bad frames to skip. Revisit. */
	*frames = 5;

	return 0;
}

static int ov7692_g_interface_parms(struct v4l2_subdev *sd,
				    struct v4l2_subdev_sensor_interface_parms
				    *parms)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ov7692 *ov7692 = to_ov7692(client);
	u8 sclk_dividers;

	if (!parms)
		return -EINVAL;

	parms->if_type = ov7692->plat_parms->if_type;
	parms->if_mode = ov7692->plat_parms->if_mode;
	parms->parms = ov7692->plat_parms->parms;

	/* set the hs term time */
	if (ov7692_fmts[ov7692->i_fmt].code == V4L2_MBUS_FMT_JPEG_1X8)
		sclk_dividers  = timing_cfg_jpeg[ov7692->i_size].sclk_dividers;
	else
		sclk_dividers = timing_cfg_yuv[ov7692->i_size].sclk_dividers;

	if (sclk_dividers == 0x01)
		parms->parms.serial.hs_term_time = 0x01;
	else
		parms->parms.serial.hs_term_time = 0x08;

	return 0;
}



static struct v4l2_subdev_sensor_ops ov7692_subdev_sensor_ops = {
	.g_skip_frames = ov7692_g_skip_frames,
	.g_interface_parms = ov7692_g_interface_parms,
};

static struct v4l2_subdev_ops ov7692_subdev_ops = {
	.core = &ov7692_subdev_core_ops,
	.video = &ov7692_subdev_video_ops,
	.sensor = &ov7692_subdev_sensor_ops,
};

static int ov7692_probe(struct i2c_client *client,
			const struct i2c_device_id *did)
{
	unsigned int i;
	int ret;
	struct ov7692 *ov7692;
	struct soc_camera_subdev_desc *ssd = client->dev.platform_data;

	client->addr = 0x3c;

	if (!ssd) {
		dev_err(&client->dev, "OV7692: missing soc-camera data!\n");
		return -EINVAL;
	}

	if (!ssd->drv_priv) {
		dev_err(&client->dev,
			"OV7692 driver needs i/f platform data\n");
		return -EINVAL;
	}

	ov7692 = kzalloc(sizeof(struct ov7692), GFP_KERNEL);
	if (!ov7692)
		return -ENOMEM;

	v4l2_i2c_subdev_init(&ov7692->subdev, client, &ov7692_subdev_ops);

	ov7692->i_size = OV7692_SIZE_VGA;
	ov7692->i_fmt = 0;	/* First format in the list */
	ov7692->plat_parms = ssd->drv_priv;

	/* Initializing hdl with 12 controls (3 for future purposes) */
	v4l2_ctrl_handler_init(&ov7692->hdl, ARRAY_SIZE(ov7692_controls) + 10);
	if (ov7692->hdl.error)
		dev_dbg(&client->dev, "Error set during init itself! %d\n",
			ov7692->hdl.error);

	/* register standard controls */
	v4l2_ctrl_new_std(&ov7692->hdl, &ov7692_ctrl_ops, V4L2_CID_BRIGHTNESS,
			EV_MINUS_1, EV_PLUS_1, 1, EV_DEFAULT);

	v4l2_ctrl_new_std(&ov7692->hdl, &ov7692_ctrl_ops, V4L2_CID_CONTRAST,
			CONTRAST_MINUS_2, CONTRAST_PLUS_2, 1, CONTRAST_DEFAULT);

	if (ov7692->hdl.error) {
		dev_err(&client->dev,
			"Standard controls initialization error %d\n",
			ov7692->hdl.error);
		ret = ov7692->hdl.error;
		goto ctrl_hdl_err;
	}

	/* register standard menu controls */
	/*
	 * Max value of some controls communicates the supported enums for that
	 * control to the HAL.
	 */
	v4l2_ctrl_new_std_menu(&ov7692->hdl, &ov7692_ctrl_ops,
		V4L2_CID_COLORFX, (1 << V4L2_COLORFX_NONE) |
		(1 << V4L2_COLORFX_NEGATIVE) | (1 << V4L2_COLORFX_SEPIA) |
		(1 << V4L2_COLORFX_BW), 0, V4L2_COLORFX_NONE);

	v4l2_ctrl_new_std_menu(&ov7692->hdl, &ov7692_ctrl_ops,
		V4L2_CID_POWER_LINE_FREQUENCY,
		V4L2_CID_POWER_LINE_FREQUENCY_AUTO, 0,
		V4L2_CID_POWER_LINE_FREQUENCY_50HZ);

	v4l2_ctrl_new_std_menu(&ov7692->hdl, &ov7692_ctrl_ops,
		V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE,
		V4L2_WHITE_BALANCE_CLOUDY, 0,
		V4L2_WHITE_BALANCE_AUTO);

	if (ov7692->hdl.error) {
		dev_err(&client->dev,
			"Standard menu controls initialization error %d\n",
			ov7692->hdl.error);
		ret = ov7692->hdl.error;
		goto ctrl_hdl_err;
	}

	/* register custom controls */
	for (i = 0; i < ARRAY_SIZE(ov7692_controls); ++i)
		v4l2_ctrl_new_custom(&ov7692->hdl, &ov7692_controls[i], NULL);

	ov7692->subdev.ctrl_handler = &ov7692->hdl;
	if (ov7692->hdl.error) {
		ret = ov7692->hdl.error;
		goto ctrl_hdl_err;
	}

	ret = ov7692_video_probe(client);
	if (ret) {
		pr_err("ov7692_video_probe: failed to probe the sensor\n");
		goto vid_probe_fail;
	}


	return ret;

ctrl_hdl_err:
vid_probe_fail:
	v4l2_ctrl_handler_free(&ov7692->hdl);
	kfree(ov7692);
	pr_err("ov7692_probe failed with ret = %d\n", ret);
	return ret;
}

static int ov7692_remove(struct i2c_client *client)
{
	struct ov7692 *ov7692 = to_ov7692(client);
	struct soc_camera_device *icd = client->dev.platform_data;

	ov7692_video_remove(icd);
	client->driver = NULL;
	kfree(ov7692);

	return 0;
}

static const struct i2c_device_id ov7692_id[] = {
	{"ov7692", 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, ov7692_id);

static struct i2c_driver ov7692_i2c_driver = {
	.driver = {
		   .name = "ov7692",
		   },
	.probe = ov7692_probe,
	.remove = ov7692_remove,
	.id_table = ov7692_id,
};

static int __init ov7692_mod_init(void)
{
	return i2c_add_driver(&ov7692_i2c_driver);
}

static void __exit ov7692_mod_exit(void)
{
	i2c_del_driver(&ov7692_i2c_driver);
}

late_initcall(ov7692_mod_init);
module_exit(ov7692_mod_exit);

MODULE_DESCRIPTION("OmniVision OV7692 Camera driver");
MODULE_AUTHOR("Sergio Aguirre <saaguirre@ti.com>");
MODULE_LICENSE("GPL v2");
