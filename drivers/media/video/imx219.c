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
#include <linux/init.h>
#include <linux/module.h>
#include <linux/timer.h>
#include <linux/proc_fs.h>
#include "imx219.h"
#ifdef CONFIG_VIDEO_DRV201
#include <media/drv201.h>
#endif

#define SENSOR_NAME_STR "imx219"
#define IMX219_DEBUGFS 1

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
	IMX219_MODE_1920x1080P48 = 1,
	IMX219_MODE_MAX          = 2,
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
	int			vts_min;
	enum bayer_order	bayer;
	int			bpp;
	int			fps;
	int			fps_min;
	int			fps_max;
};

#define LENS_READ_DELAY 4

struct imx219 {
	struct v4l2_subdev subdev;
	struct v4l2_subdev_sensor_interface_parms *plat_parms;
	struct soc_camera_device *icd;
	struct proc_dir_entry *proc_entry;
	int state;
	int mode_idx;
	int i_fmt;
	int focus_mode;
	int line_length;
	int vts;
	int vts_max;
	int vts_min;
	u16 lens_read_buf[LENS_READ_DELAY];
	u16 lens_delay;
	u16 af_on;
	u16 gain_current;
	u16 exposure_current;
	u16 coarse_int_lines;
	u16 agc_on;
	int framerate;
	int framerate_lo;
	int framerate_hi;
};

struct sensor_mode imx219_mode[IMX219_MODE_MAX + 1] = {
	{
		.name           = "3280x2464P15",
		.height         = 2464,
		.width          = 3280,
		.hts            = 3448,
		.vts            = 2504,
		.vts_max        = 32767 - 6,
		.vts_min        = 2504,
		.line_length_ns = 26624,
		.bayer          = BAYER_GBRG,
		.bpp            = 10,
		.fps            = F24p8(15.0),
		.fps_min        = F24p8(1.0),
		.fps_max        = F24p8(15.0),
	},

	{
		.name           = "1920x1080P48",
		.height         = 1080,
		.width          = 1920,
		.hts            = 3448,
		.vts            = 1113,
		.vts_max        = 32767 - 6,
		.vts_min        = 1113,
		.line_length_ns = 18915,
		.bayer          = BAYER_GBRG,
		.bpp            = 10,
		.fps            = F24p8(47.5),
		.fps_min        = F24p8(1.0),
		.fps_max        = F24p8(47.5),
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

static const struct imx219_reg imx219_reginit[IMX219_MODE_MAX][256] = {
	{
		/* 3280x2464P15 */
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
	},

	{
		/* 1920x1080P48 */
		{0x30EB, 0x05},
		{0x30EB, 0x0C},
		{0x300A, 0xFF},
		{0x300B, 0xFF},
		{0x30EB, 0x05},
		{0x30EB, 0x09},
		{0x0114, 0x01},
		{0x0128, 0x00},
		{0x012A, 0x18},
		{0x012B, 0x00},
		{0x0160, 0x04},
		{0x0161, 0x59},
		{0x0162, 0x0D},
		{0x0163, 0x78},
		{0x0164, 0x02},
		{0x0165, 0xA8},
		{0x0166, 0x0A},
		{0x0167, 0x27},
		{0x0168, 0x02},
		{0x0169, 0xB4},
		{0x016A, 0x06},
		{0x016B, 0xEB},
		{0x016C, 0x07},
		{0x016D, 0x80},
		{0x016E, 0x04},
		{0x016F, 0x38},
		{0x0170, 0x01},
		{0x0171, 0x01},
		{0x0174, 0x00},
		{0x0175, 0x00},
		{0x018C, 0x0A},
		{0x018D, 0x0A},
		{0x0301, 0x05},
		{0x0303, 0x01},
		{0x0304, 0x03},
		{0x0305, 0x03},
		{0x0306, 0x00},
		{0x0307, 0x39},
		{0x0309, 0x0A},
		{0x030B, 0x01},
		{0x030C, 0x00},
		{0x030D, 0x72},
		{0x455E, 0x00},
		{0x471E, 0x4B},
		{0x4767, 0x0F},
		{0x4750, 0x14},
		{0x4540, 0x00},
		{0x47B4, 0x14},
		{0x4713, 0x30},
		{0x478B, 0x10},
		{0x478F, 0x10},
		{0x4793, 0x10},
		{0x4797, 0x0E},
		{0x479B, 0x0E},

		{0xFFFF, 0x00}	/* end of the list */
	}
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

/**
 *imx219_reg_read_multi - Read values from a register in an imx219 sensor device
 *@client: i2c driver client structure
 *@reg: register address / offset
 *@val: stores the value that gets read
 *
 * Read multiple values from a register in an imx219 sensor device.
 * The values are returned in 'val'.
 * Returns zero if successful, or non-zero otherwise.
 */
static int imx219_reg_read_multi(struct i2c_client *client,
				 u16 reg, u8 *val, u16 cnt)
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
	msg.len = cnt;
	msg.buf = val;
	ret = i2c_transfer(client->adapter, &msg, 1);
	if (ret < 0)
		goto err;

	return 0;

err:
	dev_err(&client->dev, "%s(): Failed reading register addr=0x%02x cnt=%d\n",
		__func__,
		reg, cnt);
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
	for (i = 0; i < imx219->lens_delay; i++)
		imx219->lens_read_buf[i] = imx219->lens_read_buf[i + 1];
	imx219->lens_read_buf[imx219->lens_delay] = *current_position;
	*current_position = imx219->lens_read_buf[0];

	return;
}

/*
 * Set sensor analog gain for the requested gain value in 8.8 FP format
 *@client: i2c driver client structure
 *@gain_value: requested gain value in 8.8 FP format
 *
 * Determine closest possible sensor analog gain and corresponding code,
 * and change sensor register settings to use the new gain.
 * Return 0 for success, or nonzero code for failure.
 */
static int imx219_set_gain(struct i2c_client *client, int gain_value)
{
	u16 gain_code;
	int ret;
	struct imx219 *imx219 = to_imx219(client);

	gain_code = (256 - (10*256*256 /  gain_value + 5) / 10) & 0xFF;
	imx219->gain_current = 256 * 256 / (256 - gain_code);
	ret = imx219_reg_write(client, IMX219_ANA_GAIN_GLOBAL, gain_code);
	pr_debug("%s(): gain requested=0x%X code=0x%X actual=0x%X",
		 __func__, gain_value, gain_code, imx219->gain_current);
	return ret;
}

/*
 * Read sensor analog gain
 *@client: i2c driver client structure
 *@gain_value_p: pointer to return gain value in 8.8 FP format
 *@gain_code_p : pointer to return gain code
 *
 * Read sensor analog gain code, compute analog gain.
 * Return 0 for success, or nonzero code for failure.
 */
static int imx219_get_gain(struct i2c_client *client,
			   int *gain_value_p, int *gain_code_p)
{
	int ret;
	u8 gain_code;

	ret = imx219_reg_read(client, IMX219_ANA_GAIN_GLOBAL, &gain_code);
	if (gain_code_p)
		*gain_code_p = gain_code;
	if (gain_value_p)
		*gain_value_p = 256 * 256 / (256 - gain_code);
	pr_debug("%s(): gain value=0x%X code=0x%X",
		 __func__, *gain_value_p, gain_code);
	return ret;
}

#define INTEGRATION_MIN		1
#define INTEGRATION_OFFSET	10

/*
 * Compute sensor exposure for the requested exposure value in ms.
 *@client: i2c driver client structure
 *@exp_value: requested exposure value in ms
 *
 * Determine closest possible sensor exposure and code,
 * Return 0 for success, or nonzero code for failure.
 */
static int imx219_calc_exposure(struct i2c_client *client,
				int exposure_value,
				unsigned int *vts_ptr,
				unsigned int *coarse_int_lines)
{
	struct imx219 *imx219 = to_imx219(client);
	unsigned int integration_lines, exposure_current;
	int vts = imx219_mode[imx219->mode_idx].vts;

	integration_lines = (1000 * exposure_value) / imx219->line_length;
	if (integration_lines < INTEGRATION_MIN)
		integration_lines = INTEGRATION_MIN;
	else if (integration_lines > (unsigned int)
		 (imx219->vts_max - INTEGRATION_OFFSET))
		integration_lines = imx219->vts_max - INTEGRATION_OFFSET;
	vts = integration_lines + INTEGRATION_OFFSET;
	vts = min(max(vts, imx219->vts_min), imx219->vts_max);
	if (coarse_int_lines)
		*coarse_int_lines = integration_lines;
	if (vts_ptr)
		*vts_ptr = vts;
	exposure_current =
		integration_lines * imx219->line_length / (unsigned long)1000;
	pr_debug("%s req=%d act=%d lin=%d vts=%d [%d %d]\n",
		 __func__,
		 exposure_value,
		 exposure_current,
		 *coarse_int_lines,
		 vts,
		 imx219->vts_min,
		 imx219->vts_max
		);
	return exposure_value;
}

/*
 * Set sensor exposure for the requested exposure value in ms.
 *@client: i2c driver client structure
 *@exp_value: requested exposure value in ms
 *
 * Determine closest possible sensor exposure and code,
 * and change sensor register settings to use the new exposure.
 * Return 0 for success, or nonzero code for failure.
 */
static void imx219_set_exposure(struct i2c_client *client, int exp_value)
{
	struct imx219 *imx219 = to_imx219(client);
	unsigned int actual_exposure;
	unsigned int vts;
	unsigned int coarse_int_lines;
	int ret = 0;

	actual_exposure = imx219_calc_exposure(client,
					       exp_value,
					       &vts,
					       &coarse_int_lines);
	pr_debug("%s(): cur=%d req=%d act=%d coarse=%d vts=%d\n",
		 __func__,
		 imx219->exposure_current, exp_value, actual_exposure,
		 coarse_int_lines, vts);
	ret = imx219_reg_write(client,
			       IMX219_COARSE_INT_TIME_HI,
			       (coarse_int_lines >> 8) & 0xFF);
	ret |= imx219_reg_write(client,
				IMX219_COARSE_INT_TIME_LO,
				coarse_int_lines & 0xFF);
	ret |= imx219_reg_write(client,
				IMX219_FRM_LENGTH_HI,
				(vts >> 8) & 0xFF);
	ret |= imx219_reg_write(client,
				IMX219_FRM_LENGTH_LO,
				vts & 0xFF);
	if (ret) {
		pr_debug("imx219_set_exposure: error writing exposure register");
		return;
	}
	imx219->vts = vts;
	imx219->exposure_current = actual_exposure;
	imx219->coarse_int_lines = coarse_int_lines;
}

/*
 * Read sensor exposure.
 *@client: i2c driver client structure
 *@exp_value_p: pointer to return exposure value in ms
 *@exp_value_p: pointer to return exposure code
 *
 * Read sensor exposure
 * Return 0 for success, or nonzero code for failure.
 */
static int imx219_get_exposure(struct i2c_client *client,
			       int *exp_value_p, int *exp_code_p)
{
	struct imx219 *imx219 = to_imx219(client);
	int ret = 0;
	int exp_code;
	u8 exp_buf[2];

	imx219_reg_read_multi(client, IMX219_COARSE_INT_TIME_HI, exp_buf, 2);
	exp_code = (exp_buf[0] << 8) + (exp_buf[1] << 0);
	*exp_code_p = exp_code;
	*exp_value_p = (exp_code * imx219->line_length) / 1000;
	pr_debug("%s(): code=%d value=%d",
		 __func__, exp_code, *exp_value_p);
	return ret;
}

/*
 * Setup the sensor integration and frame length based on requested mimimum
 * framerate
 *@client: i2c driver client structure
 *@exp_framerate: requested framerate
 */
static void imx219_set_framerate_lo(struct i2c_client *client, int framerate)
{
	struct imx219 *imx219 = to_imx219(client);

	framerate = max(framerate, imx219_mode[imx219->mode_idx].fps_min);
	framerate = min(framerate, imx219_mode[imx219->mode_idx].fps_max);
	imx219->framerate_lo = framerate;
	framerate >>= 8;
	imx219->vts_max = 1000000000 / (imx219->line_length * framerate);
	imx219->vts_max = min(imx219->vts_max,
			      imx219_mode[imx219->mode_idx].vts_max);
	pr_debug("%s(): Setting frame rate lo %d.%02d vts_min %d",
		 __func__,
		 imx219->framerate_lo>>8, imx219->framerate_lo & 0xFF,
		 imx219->vts_min);
}

/*
 * Setup the sensor integration and frame length based on requested maximum
 * framerate
 *@client: i2c driver client structure
 *@exp_framerate: requested framerate
 */
static void imx219_set_framerate_hi(struct i2c_client *client, int framerate)
{
	struct imx219 *imx219 = to_imx219(client);

	framerate = max(framerate, imx219_mode[imx219->mode_idx].fps_min);
	framerate = min(framerate, imx219_mode[imx219->mode_idx].fps_max);
	imx219->framerate_hi = framerate;
	framerate >>= 8;
	imx219->vts_min = 1000000000 / (imx219->line_length * framerate);
	imx219->vts_min = max(imx219->vts_min,
			      imx219_mode[imx219->mode_idx].vts);
	pr_debug("%s(): Setting frame rate hi %d.%02d vts_min %d",
		 __func__,
		 imx219->framerate_hi>>8, imx219->framerate_hi & 0xFF,
		 imx219->vts_min);
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

	case V4L2_CID_GAIN:
		imx219_get_gain(client, &retval, NULL);
		ctrl->value = retval;
		break;

	case V4L2_CID_EXPOSURE: {
		int code;
		imx219_get_exposure(client, &retval, &code);
		ctrl->value = retval;
		break;
	}

	default:
		pr_debug("%s(): g_ctrl not supported for id=0x%X",
			 __func__, ctrl->id);
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
		if (imx219->af_on)
			imx219_lens_set_position(client, ctrl->value);
		else
			pr_debug("%s(): af is off", __func__);
		break;

	case V4L2_CID_CAMERA_FOCUS_MODE:
		if (ctrl->value > FOCUS_MODE_MANUAL)
			return -EINVAL;
		imx219->focus_mode = ctrl->value;
		break;

	case V4L2_CID_GAIN:
		if (((unsigned int)ctrl->value > IMX219_GAIN_MAX) |
		    ((unsigned int)ctrl->value < IMX219_GAIN_MIN)) {
			dev_err(&client->dev,
				"V4L2_CID_GAIN invalid gain=%u min=%u max=%d",
				ctrl->value, IMX219_GAIN_MIN, IMX219_GAIN_MAX);
			return -EINVAL;
		}
		if (imx219->agc_on)
			imx219_set_gain(client, ctrl->value);
		else
			pr_debug("%s(): agc is off", __func__);
		break;

	case V4L2_CID_EXPOSURE:
		if ((unsigned int)ctrl->value < IMX219_EXP_MIN) {
			dev_err(&client->dev,
				"V4L2_CID_EXPOSURE invalid exposure=%u min=%d",
				ctrl->value, IMX219_EXP_MIN);
			return -EINVAL;
		}
		if (imx219->agc_on)
			imx219_set_exposure(client, ctrl->value);
		else
			pr_debug("%s(): agc is off", __func__);
		break;

	case V4L2_CID_CAMERA_FRAME_RATE:
		if (ctrl->value > FRAME_RATE_30)
			return -EINVAL;
		imx219->framerate = ctrl->value;
		switch (imx219->framerate) {
		case FRAME_RATE_5:
			imx219->framerate_lo = F24p8(5.0);
			imx219->framerate_hi = F24p8(5.0);
			break;
		case FRAME_RATE_7:
			imx219->framerate_lo = F24p8(7.0);
			imx219->framerate_hi = F24p8(7.0);
			break;
		case FRAME_RATE_10:
			imx219->framerate_lo = F24p8(10.0);
			imx219->framerate_hi = F24p8(10.0);
			break;
		case FRAME_RATE_15:
			imx219->framerate_lo = F24p8(15.0);
			imx219->framerate_hi = F24p8(15.0);
			break;
		case FRAME_RATE_20:
			imx219->framerate_lo = F24p8(20.0);
			imx219->framerate_hi = F24p8(20.0);
			break;
		case FRAME_RATE_25:
			imx219->framerate_lo = F24p8(25.0);
			imx219->framerate_hi = F24p8(25.0);
			break;
		case FRAME_RATE_30:
			imx219->framerate_lo = F24p8(30.0);
			imx219->framerate_hi = F24p8(30.0);
			break;
		case FRAME_RATE_AUTO:
		default:
			imx219->framerate_lo = F24p8(1.0);
			imx219->framerate_hi = F24p8(30.0);
			break;
		}
		imx219_set_framerate_lo(client, imx219->framerate_lo);
		imx219_set_framerate_hi(client, imx219->framerate_hi);
		imx219_set_exposure(client, imx219->exposure_current);
		imx219_set_gain(client, imx219->gain_current);
		break;

	default:
		dev_err(&client->dev,
			"s_ctrl not supported for id=0x%X\n", ctrl->id);
		break;
	}
	return ret;

}

static long imx219_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
{
	int ret = 0;

	return ret;

}

static int imx219_s_power(struct v4l2_subdev *sd, int on)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct imx219 *imx219 = to_imx219(client);

	pr_debug("%s(): on=%d", __func__, on);
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
	imx219->lens_delay        = 0;
	imx219->gain_current      = IMX219_DEFAULT_GAIN;
	imx219->exposure_current  = IMX219_DEFAULT_EXP;
	imx219->agc_on            = 1;
	imx219->af_on             = 1;
	pr_debug("%s()", __func__);
	return ret;
}

static int imx219_procfs_read(char *buf, char **start, off_t offset,
		       int count, int *eof, void *data) {
	int len;
	int lens_position, lens_ttd;
	struct imx219 *imx219 = to_imx219((struct i2c_client *)data);

	drv201_lens_get_position(&lens_position, &lens_ttd);

	len = sprintf(buf,
		      "mode   = %s\n"\
		      "ag     = 0x%2X (%d.%03d)\n"\
		      "exp    = %d\n"\
		      "lines  = %d\n"\
		      "vts    = %d\n"\
		      "agc    = %s\n"\
		      "lens   = [%d %d]\n"\
		      "af     = %s\n",
		      imx219_mode[imx219->mode_idx].name,
		      imx219->gain_current,
		      imx219->gain_current >> 8, imx219->gain_current & 0xFF,
		      imx219->exposure_current,
		      imx219->coarse_int_lines,
		      imx219->vts,
		      imx219->agc_on ? "on" : "off",
		      lens_position, lens_ttd,
		      imx219->af_on ? "on" : "off"
		      );
	return len;
}

#define IMX219_MAX_PROC 255

static int imx219_procfs_write(struct file *file,
			       const char __user *buf,
			       unsigned long count,
			       void *data)
{
	int val;
	char str[IMX219_MAX_PROC+1];
	struct i2c_client *client;
	struct imx219 *imx219;
	int lens_position, lens_ttd;

	client = (struct i2c_client *)data;
	imx219 = to_imx219(client);
	if (count > IMX219_MAX_PROC)
		count = IMX219_MAX_PROC;
	if (copy_from_user(str, buf, count))
		return -EFAULT;
	str[IMX219_MAX_PROC] = 0;
	pr_debug("%s(): str='%s'\n", __func__, str);
	if (strncmp(str, "ag=", 3) == 0) {
		val = 0;
		sscanf(str, "ag=%d", &val);
		if (val == -1) {
			imx219->agc_on = 1;
		} else {
			imx219->agc_on = 0;
			imx219_set_gain(client, val);
		}
		pr_debug("%s(): ag=0x%X agc_on=%d\n",
			 __func__, val, imx219->agc_on);
	} else if (strncmp(str, "exp=", 4) == 0) {
		val = 0;
		sscanf(str, "exp=%d", &val);
		if (val == -1) {
			imx219->agc_on = 1;
		} else {
			imx219->agc_on = 0;
			imx219_set_exposure(client, val);
		}
		pr_debug("%s(): exp=0x%X agc_on=%d\n",
			 __func__, val, imx219->agc_on);
	} else if (strncmp(str, "lens=", 5) == 0) {
		val = 0;
		sscanf(str, "lens=%d", &val);
		if (val == -1) {
			imx219->af_on = 1;
		} else {
			imx219->af_on = 0;
			imx219_lens_set_position(client, val);
		}
		pr_debug("%s(): lens_pos=%d af_on=%d\n",
			 __func__, val, imx219->af_on);
	} else if (strncmp(str, "lens+=", 6) == 0) {
		val = 0;
		sscanf(str, "lens+=%d", &val);
		imx219->af_on = 0;
		drv201_lens_get_position(&lens_position, &lens_ttd);
		imx219_lens_set_position(client, lens_position+val);
		pr_debug("%s(): lens_pos=%d af_on=%d\n",
			 __func__, lens_position+val, imx219->af_on);
	} else if (strncmp(str, "lens-=", 6) == 0) {
		val = 0;
		sscanf(str, "lens-=%d", &val);
		imx219->af_on = 0;
		drv201_lens_get_position(&lens_position, &lens_ttd);
		imx219_lens_set_position(client, lens_position-val);
		pr_debug("%s(): lens_pos=%d af_on=%d\n",
			 __func__, lens_position-val, imx219->af_on);
	} else
		pr_debug("%s(): unknown command: '%s'\n",
			 __func__, str);
	return count;
}

static int imx219_procfs_init(struct i2c_client *client)
{
	struct imx219 *imx219 = to_imx219(client);

	imx219->proc_entry = create_proc_entry(SENSOR_NAME_STR,
					       (S_IRUSR | S_IRGRP),
					       NULL);
	if (NULL == imx219->proc_entry) {
		dev_err(&client->dev, "%s(): error creating proc entry %s",
			__func__,
			SENSOR_NAME_STR);
		return -ENOMEM;
	}
	imx219->proc_entry->read_proc = imx219_procfs_read;
	imx219->proc_entry->write_proc = imx219_procfs_write;
	imx219->proc_entry->data = client;
	dev_info(&client->dev, "%s(): proc entry %s OK",
		 __func__, SENSOR_NAME_STR);
	return 0;
}

static int imx219_procfs_exit(struct i2c_client *client)
{
	struct imx219 *imx219 = to_imx219(client);

	if (imx219->proc_entry) {
		remove_proc_entry(SENSOR_NAME_STR,  NULL);
		dev_info(&client->dev, "%s(): proc entry %s removed",
			 __func__, SENSOR_NAME_STR);
	}
	return 0;
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
	pr_debug("Video removed: %p, %p\n",
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
		i = IMX219_MODE_3280x2464P15;

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
	case IMX219_MODE_1920x1080P48:
		interval->discrete.numerator = 1;
		interval->discrete.denominator = 30;
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
		ret = imx219_reg_writes(client, imx219_reginit[new_mode_idx]);
	} else {
		pr_debug("imx219_set_mode: diff init from mode[%d]=%s to mode[%d]=%s\n",
			 imx219->mode_idx,
			 imx219_mode[imx219->mode_idx].name,
			 new_mode_idx, imx219_mode[new_mode_idx].name);
		ret = imx219_reg_writes(client, imx219_reginit[new_mode_idx]);
	}
	if (ret)
		return ret;

	/* init new mode values */
	imx219->mode_idx = new_mode_idx;
	imx219->line_length  = imx219_mode[new_mode_idx].line_length_ns;
	imx219->vts = imx219_mode[new_mode_idx].vts;
	imx219_set_framerate_lo(client, imx219_mode[new_mode_idx].fps_min);
	imx219_set_framerate_hi(client, imx219_mode[new_mode_idx].fps_max);

	/* gain_current, exposure_current are reset in imx219_init */
	imx219_set_gain(client, imx219->gain_current);
	imx219_set_exposure(client, imx219->exposure_current);

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
#ifdef CONFIG_VIDEO_DRV201
	int lens_position, lens_ttd;
#endif
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
	/* init VCM */
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
	drv201_lens_get_position(&lens_position, &lens_ttd);
	imx219->lens_read_buf[imx219->lens_delay] = lens_position;
#endif

#if IMX219_DEBUGFS
	/* init procfs */
	imx219_procfs_init(client);
#endif

	return ret;
}

static int imx219_remove(struct i2c_client *client)
{
	struct imx219 *imx219 = to_imx219(client);
	struct soc_camera_device *icd = client->dev.platform_data;

	pr_debug("%s(): remove", __func__);
	imx219_procfs_exit(client);
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
		.name = SENSOR_NAME_STR,
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
