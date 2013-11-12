/*
 * Sony IMX219 sensor driver
 *
 * Copyright (C) 2013 Broadcom Corporation
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
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/gpio_keys.h>
#include <linux/printk.h>

#include <media/v4l2-subdev.h>
#include <media/v4l2-chip-ident.h>
#include <media/soc_camera.h>
#include <linux/videodev2_brcm.h>
#include "imx219.h"

#include <linux/init.h>
#include <linux/module.h>
#include <linux/timer.h>
#ifdef CONFIG_VIDEO_DRV201
#include <media/drv201.h>
#endif

struct imx219_datafmt {
	enum v4l2_mbus_pixelcode code;
	enum v4l2_colorspace colorspace;
};

static const struct imx219_datafmt imx219_fmts[] = {
	/*
	   Order important: first natively supported,
	   second supported with a GPIO extender
	 */
	{V4L2_MBUS_FMT_SRGGB10_1X10, V4L2_COLORSPACE_SRGB},
	{V4L2_MBUS_FMT_SBGGR10_1X10, V4L2_COLORSPACE_SRGB},
	{V4L2_MBUS_FMT_SGBRG10_1X10, V4L2_COLORSPACE_SRGB},
	{V4L2_MBUS_FMT_SGRBG10_1X10, V4L2_COLORSPACE_SRGB},
};

enum imx219_state {
	IMX219_STATE_STOP = 0,
	IMX219_STATE_STRM = 1,
	IMX219_STATE_MAX  = 2,
};

enum imx219_mode {
	IMX219_MODE_3280x2464P15 = 0,
	IMX219_MODE_MAX          = 1,
};

enum bayer_order {
	BAYER_RGGB = 0,
	BAYER_GRBG = 1,
	BAYER_GBRG = 2,
	BAYER_BGGR = 3,
};

struct sensor_mode {
	char			name[128];
	int			height;
	int			width;
	int			line_length_ns;
	int			hts;
	int			vts;
	int			vts_max;
	enum bayer_order	bayer;
	int			bpp;
	int			fps;
};

#define LENS_READ_DELAY 4

struct imx219 {
	struct v4l2_subdev subdev;
	struct v4l2_subdev_sensor_interface_parms *plat_parms;
	struct soc_camera_device *icd;
	int state;
	int mode_idx;
	int i_fmt;
	int framerate;
	int focus_mode;
	int line_length;
	int vts;
	int vts_max;
	int vts_min;
	int lens_read_buf[LENS_READ_DELAY];
	int lenspos_delay;
};

struct sensor_mode imx219_mode[IMX219_MODE_MAX + 1] = {
	{
		.name           = "3280x2464P15",
		.height         = 2464,
		.width          = 3280,
		.hts            = 3448,
		.vts            = 2504,
		.vts_max        = 32767 - 6,
		.line_length_ns = 22190,
		.bayer          = BAYER_GBRG,
		.bpp            = 10,
		.fps            = F24p8(15.0),
	},
	{
		.name           = "STOPPED",
		.line_length_ns = 22190,
	}
};

/**
 *struct imx219_reg - imx219 register format
 *@reg: 16-bit offset to register
 *@val: 8/16/32-bit register value
 *@length: length of the register
 *
 * Define a structure for IMX219 register initialization values
 */
struct imx219_reg {
	u16	reg;
	u8	val;
	u8	pad;
};

static const struct imx219_reg imx219_reginit[256] = {
	/* Sequence to access manufacturer specific
	   registers*/
	{0x30EB, 0x05},
	{0x30EB, 0x0C},
	{0x300A, 0xFF},
	{0x300B, 0xFF},
	{0x30EB, 0x05},
	{0x30EB, 0x09},

	/* 2 Lane MIPI */
	{0x0114, 0x01},
	{0x0128, 0x00},
	{0x012A, 0x18},
	{0x012B, 0x00},
	{0x0160, 0x09},
	{0x0161, 0xC8},
	{0x0162, 0x0D},
	{0x0163, 0x78},
	{0x0164, 0x00},
	{0x0165, 0x00},
	{0x0166, 0x0C},
	{0x0167, 0xCF},
	{0x0168, 0x00},
	{0x0169, 0x00},
	{0x016A, 0x09},
	{0x016B, 0x9F},
	{0x016C, 0x0C},
	{0x016D, 0xD0},
	{0x016E, 0x09},
	{0x016F, 0xA0},
	{0x0170, 0x01},
	{0x0171, 0x01},
	{0x0172, 0x03},
	{0x0174, 0x00},
	{0x0175, 0x00},
	{0x018C, 0x0A},
	{0x018D, 0x0A},

	{0x0301, 0x05},
	{0x0303, 0x01},
	{0x0304, 0x03},
	{0x0305, 0x03},
	{0x0306, 0x00},
	{0x0307, 0x2B},
	{0x0309, 0x0A},
	{0x030B, 0x01},
	{0x030C, 0x00},
	{0x030D, 0x55},

	{0x455E, 0x00},
	{0x471E, 0x4B},
	{0x4767, 0x0F},
	{0x4750, 0x14},
	{0x4540, 0x00},
	{0x47B4, 0x14},
	{0x4713, 0x30},
	{0x478B, 0x10},
	{0x478F, 0x10},
	{0x4797, 0x0E},
	{0x479B, 0x0E},

	{0xFFFF, 0x00}	/* end of the list */
};

static struct imx219 *to_imx219(const struct i2c_client *client)
{
	return container_of(i2c_get_clientdata(client), struct imx219, subdev);
}

/**
 * Write a value to a register in imx219 sensor device.
 *@client: i2c driver client structure.
 *@reg: Address of the register to read value from.
 *@val: Value to be written to a specific register.
 * Returns zero if successful, or non-zero otherwise.
 */
static int imx219_reg_write(struct i2c_client *client, u16 reg, u8 val)
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
 * Initialize a list of imx219 registers.
 * The list of registers is terminated by the pair of values
 *@client: i2c driver client structure.
 *@reglist[]: List of address of the registers to write data.
 * Returns zero if successful, or non-zero otherwise.
 */
static int imx219_reg_writes(struct i2c_client *client,
		const struct imx219_reg reglist[])
{
	int err = 0, i;

	for (i = 0; reglist[i].reg != 0xFFFF; i++) {
		if (reglist[i].reg == 0xFFFE) {
			msleep(reglist[i].val);
		} else {
			err |= imx219_reg_write(client, reglist[i].reg,
					reglist[i].val);
		}
		if (err != 0)
			break;
	}
	return 0;
}



/**
 *imx219_reg_read - Read a value from a register in an imx219 sensor device
 *@client: i2c driver client structure
 *@reg: register address / offset
 *@val: stores the value that gets read
 *
 * Read a value from a register in an imx219 sensor device.
 * The value is returned in 'val'.
 * Returns zero if successful, or non-zero otherwise.
 */
static int imx219_reg_read(struct i2c_client *client, u16 reg, u8 *val)
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

/*
 * Routine used to send lens to a traget position position and calculate
 * the estimated time required to get to this position, the flying time in us.
*/
static int imx219_lens_set_position(struct i2c_client *client,
				    int target_position)
{
	int ret = 0;
#ifdef CONFIG_VIDEO_DRV201
	ret = drv201_lens_set_position(target_position);
#endif
	return ret;
}


/*
 * Routine used to get the current lens position and/or the estimated
 * time required to get to the requested destination (time in us).
 */
static void imx219_lens_get_position(struct i2c_client *client,
					int *current_position,
					int *time_to_destination)
{
	int i;
	struct imx219 *imx219 = to_imx219(client);

#ifdef CONFIG_VIDEO_DRV201
	drv201_lens_get_position(current_position,
				 time_to_destination);
#endif
	for (i = 0; i < imx219->lenspos_delay; i++)
		imx219->lens_read_buf[i] = imx219->lens_read_buf[i + 1];
	imx219->lens_read_buf[imx219->lenspos_delay] = *current_position;
	*current_position = imx219->lens_read_buf[0];

	return;
}

static int imx219_set_bus_param(struct soc_camera_device *icd,
		unsigned long flags)
{
	/* TODO: Do the right thing here, and validate bus params */
	return 0;
}

static unsigned long imx219_query_bus_param(struct soc_camera_device *icd)
{
	unsigned long flags = SOCAM_PCLK_SAMPLE_FALLING |
		SOCAM_HSYNC_ACTIVE_HIGH | SOCAM_VSYNC_ACTIVE_HIGH |
		SOCAM_DATA_ACTIVE_HIGH | SOCAM_MASTER;

	/* TODO: Do the right thing here, and validate bus params */

	flags |= SOCAM_DATAWIDTH_10;

	return flags;
}

static int imx219_enum_input(struct soc_camera_device *icd,
		struct v4l2_input *inp)
{
	struct soc_camera_link *icl = to_soc_camera_link(icd);
	struct v4l2_subdev_sensor_interface_parms *plat_parms;

	inp->type = V4L2_INPUT_TYPE_CAMERA;
	inp->std = V4L2_STD_UNKNOWN;
	strcpy(inp->name, "imx219");

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

static const struct v4l2_queryctrl imx219_controls[] = {
	{
		.id = V4L2_CID_CAMERA_FRAME_RATE,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.name = "Framerate control",
		.minimum = FRAME_RATE_AUTO,
		.maximum = (1 << FRAME_RATE_AUTO | 1 << FRAME_RATE_15),
		.step = 1,
		.default_value = FRAME_RATE_AUTO,
	},
	{
		.id = V4L2_CID_CAMERA_FOCUS_MODE,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.name = "Focus Modes",
		.minimum = FOCUS_MODE_AUTO,
		.maximum = (1 << FOCUS_MODE_AUTO),
		.step = 1,
		.default_value = FOCUS_MODE_AUTO,
	},
};

static struct soc_camera_ops imx219_ops = {
	.set_bus_param   = imx219_set_bus_param,
	.query_bus_param = imx219_query_bus_param,
	.enum_input      = imx219_enum_input,
	.controls        = imx219_controls,
	.num_controls    = ARRAY_SIZE(imx219_controls),
};

/*
 *
 */
static int imx219_g_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct imx219 *imx219 = to_imx219(client);
	int retval = 0;

	return retval;

	/* TODO update when all settings available */
	switch (ctrl->id) {
	case V4L2_CID_CAMERA_FRAME_RATE:
		ctrl->value = imx219->framerate;
		break;
	case V4L2_CID_CAMERA_FOCUS_MODE:
		ctrl->value = imx219->focus_mode;
		break;
	case V4L2_CID_CAMERA_LENS_POSITION: {
		int current_position;
		int time_to_destination;
		imx219_lens_get_position(client, &current_position,
					 &time_to_destination);
		ctrl->value = current_position;
		break;
	}
	}
	return 0;
}

/*
 *
 */
static int imx219_s_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct imx219 *imx219 = to_imx219(client);
	int ret = 0;

	switch (ctrl->id) {
	case V4L2_CID_CAMERA_LENS_POSITION:
		if (ctrl->value > IMX219_LENS_MAX)
			return -EINVAL;
		imx219_lens_set_position(client, ctrl->value);
		break;
	case V4L2_CID_CAMERA_FOCUS_MODE:
		if (ctrl->value > FOCUS_MODE_MANUAL)
			return -EINVAL;
		imx219->focus_mode = ctrl->value;
		break;
	default:
		dev_err(&client->dev,
			"s_ctrl not supported for id=0x%X\n", ctrl->id);
		break;
	}
	return ret;

	/* TODO update when all settings available */
}

static long imx219_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
{
	int ret = 0;

	return ret;

	/* TODO update when all settings available */
}

static int imx219_s_power(struct v4l2_subdev *sd, int on)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct imx219 *imx219 = to_imx219(client);

	if (0 == on) {
		imx219->mode_idx = IMX219_MODE_MAX;
	}
	return 0;
}

static int imx219_init(struct i2c_client *client)
{
	struct imx219 *imx219 = to_imx219(client);
	int ret = 0;

	/* set initial mode */
	imx219->mode_idx = IMX219_MODE_MAX;
	imx219->state = IMX219_STATE_STOP;

	/* default settings */
	imx219->framerate         = FRAME_RATE_AUTO;
	imx219->focus_mode        = FOCUS_MODE_AUTO;
	imx219->lenspos_delay     = 0;

	dev_dbg(&client->dev, "Sensor initialized\n");
	return ret;
}

/*
 * Interface active, can use i2c. If it fails, it can indeed mean, that
 * this wasn't our capture interface, so, we wait for the right one
 */
static int imx219_video_probe(struct soc_camera_device *icd,
		struct i2c_client *client)
{
	unsigned long flags;
	int ret = 0;
	u8 id_high, id_low;
	u16 id;

	/* */
	dev_info(&client->dev, "imx219_video_probe start");

	/*
	 * We must have a parent by now. And it cannot be a wrong one.
	 * So this entire test is completely redundant.
	 */
	if (!icd->dev.parent ||
			to_soc_camera_host(icd->dev.parent)->nr != icd->iface)
		return -ENODEV;

	flags = SOCAM_DATAWIDTH_8;

	/* Read sensor Model ID */
	ret = imx219_reg_read(client, IMX219_REG_MODEL_ID_HIGH, &id_high);
	if (ret < 0)
		return ret;

	id = id_high << 8;

	ret = imx219_reg_read(client, IMX219_REG_MODEL_ID_LOW, &id_low);
	if (ret < 0)
		return ret;

	id |= id_low;

	if (id != 0x0219)
		return -ENODEV;

	dev_info(&client->dev, "Detected a imx219 chip 0x%04x\n", id);

	return 0;
}

static void imx219_video_remove(struct soc_camera_device *icd)
{
	dev_dbg(&icd->dev, "Video removed: %p, %p\n",
			icd->dev.parent, icd->vdev);
}

static struct v4l2_subdev_core_ops imx219_subdev_core_ops = {
	.g_ctrl = imx219_g_ctrl,
	.s_ctrl = imx219_s_ctrl,
	.ioctl = imx219_ioctl,
	.s_power = imx219_s_power,
};


/* Find a frame size in an array */
static int imx219_find_framesize(u32 width, u32 height)
{
	int i;

	for (i = 0; i < IMX219_MODE_MAX; i++) {
		if ((imx219_mode[i].width >= width) &&
				(imx219_mode[i].height >= height))
			break;
	}

	/* If not found, select biggest */
	if (i >= IMX219_MODE_MAX)
		i = IMX219_MODE_MAX - 1;

	return i;
}

/* we only support fixed frame rate */
static int imx219_enum_frameintervals(struct v4l2_subdev *sd,
		struct v4l2_frmivalenum *interval)
{
	int size;

	if (interval->index >= 1)
		return -EINVAL;

	interval->type = V4L2_FRMIVAL_TYPE_DISCRETE;

	size = imx219_find_framesize(interval->width, interval->height);

	switch (size) {
	case IMX219_MODE_3280x2464P15:
		interval->discrete.numerator = 1;
		interval->discrete.denominator = 15;
		break;
	default:
		interval->discrete.numerator = 1;
		interval->discrete.denominator = 15;
		break;
	}
	return 0;
}


static const struct imx219_reg imx219_reg_state[IMX219_STATE_MAX][3] = {
	{ /* to power down */
		{0x0100, 0x00},	       /* disable streaming  */
		{0xFFFF, 0x00}
	},
	{ /* to streaming */
		{0x0100, 0x01},		/* enable streaming */
		{0xFFFF, 0x00}
	},
};

static int imx219_set_state(struct i2c_client *client, int new_state)
{
	struct imx219 *imx219 = to_imx219(client);
	int ret = 0;

	pr_debug("imx219_set_state: %d (%s) -> %d (%s)\n",\
			imx219->state, imx219->state ? "strm" : "stop",\
			new_state, new_state ? "strm" : "stop");

	if (imx219->state != new_state) {
		ret = imx219_reg_writes(client, imx219_reg_state[new_state]);
		imx219->state = new_state;
	}
	return ret;
}

static int imx219_s_stream(struct v4l2_subdev *sd, int enable)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int ret;

	ret = imx219_set_state(client, enable);
	return ret;
}

/*
 * Find a data format by a pixel code in an array
 */
static int imx219_find_datafmt(enum v4l2_mbus_pixelcode code)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(imx219_fmts); i++)
		if (imx219_fmts[i].code == code)
			break;

	/* If not found, select latest */
	if (i >= ARRAY_SIZE(imx219_fmts))
		i = ARRAY_SIZE(imx219_fmts) - 1;

	return i;
}

static int imx219_try_fmt(struct v4l2_subdev *sd, struct v4l2_mbus_framefmt *mf)
{
	int i_fmt;
	int mode_idx;

	i_fmt = imx219_find_datafmt(mf->code);

	mf->code = imx219_fmts[i_fmt].code;
	mf->colorspace = imx219_fmts[i_fmt].colorspace;
	mf->field = V4L2_FIELD_NONE;

	mode_idx = imx219_find_framesize(mf->width, mf->height);

	mf->width = imx219_mode[mode_idx].width;
	mf->height = imx219_mode[mode_idx].height;

	return 0;
}

static int imx219_set_mode(struct i2c_client *client, int new_mode_idx)
{
	struct imx219 *imx219 = to_imx219(client);
	int ret = 0;

	if (imx219->mode_idx == new_mode_idx) {
		pr_debug("imx219_set_mode: skip init from mode[%d]=%s to mode[%d]=%s\n",
				imx219->mode_idx,
				imx219_mode[imx219->mode_idx].name,
				new_mode_idx, imx219_mode[new_mode_idx].name);
		return ret;
	}

	if (imx219->mode_idx == IMX219_MODE_MAX) {
		pr_debug("imx219_set_mode: full init from mode[%d]=%s to mode[%d]=%s\n",
				imx219->mode_idx,
				imx219_mode[imx219->mode_idx].name,
				new_mode_idx, imx219_mode[new_mode_idx].name);
		imx219_init(client);
		ret = imx219_reg_writes(client, imx219_reginit);
		/* write diff settings when needed */
	} else {
		pr_debug("imx219_set_mode: diff init from mode[%d]=%s to mode[%d]=%s\n",
				imx219->mode_idx,
				imx219_mode[imx219->mode_idx].name,
				new_mode_idx, imx219_mode[new_mode_idx].name);
		/* write diff settings when needed */
	}
	if (ret)
		return ret;

	imx219->mode_idx = new_mode_idx;
	imx219->line_length  = imx219_mode[new_mode_idx].line_length_ns;
	imx219->vts = imx219_mode[new_mode_idx].vts;
	return 0;
}

static int imx219_s_fmt(struct v4l2_subdev *sd, struct v4l2_mbus_framefmt *mf)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct imx219 *imx219 = to_imx219(client);
	int ret = 0;
	int new_mode_idx;

	ret = imx219_try_fmt(sd, mf);
	if (ret < 0)
		return ret;

	new_mode_idx = imx219_find_framesize(mf->width, mf->height);
	imx219->i_fmt = imx219_find_datafmt(mf->code);
	switch ((u32) imx219_fmts[imx219->i_fmt].code) {
	case V4L2_MBUS_FMT_SBGGR10_1X10:
	case V4L2_MBUS_FMT_SGBRG10_1X10:
	case V4L2_MBUS_FMT_SGRBG10_1X10:
	case V4L2_MBUS_FMT_SRGGB10_1X10:
		break;
	default:
		/* This shouldn't happen */
		ret = -EINVAL;
		return ret;
	}

	imx219_set_mode(client, new_mode_idx);
	return ret;
}

static int imx219_g_fmt(struct v4l2_subdev *sd, struct v4l2_mbus_framefmt *mf)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct imx219 *imx219 = to_imx219(client);

	mf->width = imx219_mode[imx219->mode_idx].width;
	mf->height = imx219_mode[imx219->mode_idx].height;
	mf->code = imx219_fmts[imx219->i_fmt].code;
	mf->colorspace = imx219_fmts[imx219->i_fmt].colorspace;
	mf->field = V4L2_FIELD_NONE;

	return 0;
}



static int imx219_enum_fmt(struct v4l2_subdev *sd, unsigned int index,
		enum v4l2_mbus_pixelcode *code)
{
	if (index >= ARRAY_SIZE(imx219_fmts))
		return -EINVAL;

	*code = imx219_fmts[index].code;
	return 0;
}

static int imx219_enum_framesizes(struct v4l2_subdev *sd,
		struct v4l2_frmsizeenum *fsize)
{
	if (fsize->index >= IMX219_MODE_MAX)
		return -EINVAL;

	fsize->type = V4L2_FRMSIZE_TYPE_DISCRETE;
	fsize->pixel_format = V4L2_PIX_FMT_SRGGB10;
	fsize->discrete.width = imx219_mode[fsize->index].width;
	fsize->discrete.height = imx219_mode[fsize->index].height;

	return 0;
}

static int imx219_g_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *param)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct imx219 *imx219 = to_imx219(client);
	struct v4l2_captureparm *cparm;

	if (param->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;

	cparm = &param->parm.capture;

	memset(param, 0, sizeof(*param));
	param->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	cparm->capability = V4L2_CAP_TIMEPERFRAME;

	switch (imx219->mode_idx) {
	case IMX219_MODE_3280x2464P15:
		cparm->timeperframe.numerator = 1;
		cparm->timeperframe.denominator = 15;
		break;
	default:
		cparm->timeperframe.numerator = 1;
		cparm->timeperframe.denominator = 15;
		break;
	}

	return 0;
}

static int imx219_s_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *param)
{
	/*
	 * FIXME: This just enforces the hardcoded framerates until this is
	 *flexible enough.
	 */
	return imx219_g_parm(sd, param);
}

static struct v4l2_subdev_video_ops imx219_subdev_video_ops = {
	.s_stream = imx219_s_stream,
	.s_mbus_fmt = imx219_s_fmt,
	.g_mbus_fmt = imx219_g_fmt,
	.try_mbus_fmt = imx219_try_fmt,
	.enum_mbus_fmt = imx219_enum_fmt,
	.enum_mbus_fsizes = imx219_enum_framesizes,
	.enum_framesizes = imx219_enum_framesizes,
	.enum_frameintervals = imx219_enum_frameintervals,
	.g_parm = imx219_g_parm,
	.s_parm = imx219_s_parm,
};

static int imx219_g_skip_frames(struct v4l2_subdev *sd, u32 *frames)
{
	/* Quantity of initial bad frames to skip. Revisit. */
	*frames = 1;

	return 0;
}

static int imx219_g_interface_parms(struct v4l2_subdev *sd,
		struct v4l2_subdev_sensor_interface_parms
		*parms)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct imx219 *imx219 = to_imx219(client);

	if (!parms)
		return -EINVAL;
	parms->if_type = imx219->plat_parms->if_type;
	parms->if_mode = imx219->plat_parms->if_mode;
	parms->parms = imx219->plat_parms->parms;

	/* set the hs term time */
	parms->parms.serial.hs_term_time = 0;
	parms->parms.serial.hs_settle_time = 2;

	return 0;
}

static struct v4l2_subdev_sensor_ops imx219_subdev_sensor_ops = {
	.g_skip_frames = imx219_g_skip_frames,
	.g_interface_parms = imx219_g_interface_parms,
};

static struct v4l2_subdev_ops imx219_subdev_ops = {
	.core = &imx219_subdev_core_ops,
	.video = &imx219_subdev_video_ops,
	.sensor = &imx219_subdev_sensor_ops,
};

#ifdef CONFIG_VIDEO_DRV201
#define DRV201_I2C_ADDR 0x1C
static struct i2c_board_info drv201_i2c_board_info = {
	 I2C_BOARD_INFO("drv201", (DRV201_I2C_ADDR >> 1))
};
static struct i2c_client *drv201_i2c_client;
static struct i2c_adapter *drv201_i2c_adap;
#endif

static int imx219_probe(struct i2c_client *client,
		const struct i2c_device_id *did)
{
	struct imx219 *imx219;
	struct soc_camera_device *icd = client->dev.platform_data;
	struct soc_camera_link *icl;
	int ret;

	dev_info(&client->dev, "imx219_probe");

	if (!icd) {
		dev_err(&client->dev, "imx219: missing soc-camera data!\n");
		return -EINVAL;
	}

	icl = to_soc_camera_link(icd);
	if (!icl) {
		dev_err(&client->dev, "imx219 driver needs platform data\n");
		return -EINVAL;
	}

	if (!icl->priv) {
		dev_err(&client->dev,
				"imx219 driver needs i/f platform data\n");
		return -EINVAL;
	}

	imx219 = kzalloc(sizeof(struct imx219), GFP_KERNEL);
	if (!imx219)
		return -ENOMEM;

	v4l2_i2c_subdev_init(&imx219->subdev, client, &imx219_subdev_ops);

	/* Second stage probe - when a capture adapter is there */
	icd->ops = &imx219_ops;

	imx219->mode_idx = IMX219_MODE_MAX;
	imx219->i_fmt = 0;	/* First format in the list */
	imx219->plat_parms = icl->priv;
	imx219->icd = icd;

	ret = imx219_video_probe(icd, client);
	if (ret) {
		icd->ops = NULL;
		kfree(imx219);
		return ret;
	}

	/* init the sensor here */
	ret = imx219_init(client);
	if (ret) {
		dev_err(&client->dev, "Failed to initialize sensor\n");
		ret = -EINVAL;
	}

#ifdef CONFIG_VIDEO_DRV201
	dev_info(&client->dev, "imx219: drv201 i2c start");
	drv201_i2c_adap = i2c_get_adapter(0);
	if (!drv201_i2c_adap)
		dev_err(&client->dev, "imx219: drv201 i2c_get_adapter(0) FAILED");
	if (drv201_i2c_adap) {
		drv201_i2c_client = i2c_new_device(drv201_i2c_adap,
						   &drv201_i2c_board_info);
		i2c_put_adapter(drv201_i2c_adap);
		dev_info(&client->dev, "imx219: drv201 i2c start OK");
	}
#endif

	return ret;
}

static int imx219_remove(struct i2c_client *client)
{
	struct imx219 *imx219 = to_imx219(client);
	struct soc_camera_device *icd = client->dev.platform_data;

	pr_debug(" remove");
	icd->ops = NULL;
	imx219_video_remove(icd);
	client->driver = NULL;
	kfree(imx219);

	return 0;
}

static const struct i2c_device_id imx219_id[] = {
	{"imx219", 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, imx219_id);

static struct i2c_driver imx219_i2c_driver = {
	.driver = {
		.name = "imx219",
	},
	.probe = imx219_probe,
	.remove = imx219_remove,
	.id_table = imx219_id,
};

static int __init imx219_mod_init(void)
{
	return i2c_add_driver(&imx219_i2c_driver);
}

static void __exit imx219_mod_exit(void)
{
	i2c_del_driver(&imx219_i2c_driver);
}

module_init(imx219_mod_init);
module_exit(imx219_mod_exit);

MODULE_DESCRIPTION("Sony IMX219 Camera driver");
MODULE_AUTHOR("Broadcom Corporation");
MODULE_LICENSE("GPL v2");
