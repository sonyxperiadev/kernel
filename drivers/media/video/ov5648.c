/*
 * OmniVision OV5648 sensor driver
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
#include "ov5648.h"
#ifdef CONFIG_VIDEO_A3907
#include <media/a3907.h>
#endif

#ifdef CONFIG_VIDEO_DW9714
#include <media/dw9714.h>
#endif

#ifdef CONFIG_VIDEO_AS3643
#include "as3643.h"
#endif

#include <linux/init.h>
#include <linux/module.h>
#include <linux/timer.h>
#define OV5648_DEBUG 0
#if defined(CONFIG_MACH_JAVA_C_LC1)
#define TORCH_EN (10)
#define FLASH_EN (11)
#endif

#define OV5648_MCLK_26MHZ

/* OV5648 has only one fixed colorspace per pixelcode */
struct ov5648_datafmt {
	enum v4l2_mbus_pixelcode code;
	enum v4l2_colorspace colorspace;
};

static const struct ov5648_datafmt ov5648_fmts[] = {
	/*
	 Order important: first natively supported,
	 second supported with a GPIO extender
	 */
	{V4L2_MBUS_FMT_SRGGB10_1X10, V4L2_COLORSPACE_SRGB},
	{V4L2_MBUS_FMT_SBGGR10_1X10, V4L2_COLORSPACE_SRGB},
	{V4L2_MBUS_FMT_SGBRG10_1X10, V4L2_COLORSPACE_SRGB},
	{V4L2_MBUS_FMT_SGRBG10_1X10, V4L2_COLORSPACE_SRGB},
};

enum ov5648_mode {
	OV5648_MODE_1280x720P30  = 0,
	OV5648_MODE_1280x960P30  = 1,
#ifdef CONFIG_ARCH_JAVA
	OV5648_MODE_1920x1080P30 = 2,
	OV5648_MODE_2592x1944P15 = 3,
	OV5648_MODE_MAX          = 4,
#else
	OV5648_MODE_2592x1944P15 = 2,
	OV5648_MODE_MAX          = 3,
#endif
};

enum ov5648_state {
	OV5648_STATE_STOP = 0,
	OV5648_STATE_STRM = 1,
	OV5648_STATE_MAX  = 2,
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

struct sensor_mode ov5648_mode[OV5648_MODE_MAX + 1] = {
	{
		.name           = "1280x720P30",
		.height         = 720,
		.width          = 1280,
		.hts            = 1864,
		.vts            = 1496,
		.vts_max        = 32767 - 6,
		.line_length_ns = 22190,
		.bayer          = BAYER_BGGR,
		.bpp            = 10,
		.fps            = F24p8(30.0),
	},
	{
		.name           = "1280x960P30",
		.height         = 960,
		.width          = 1280,
		.hts            = 1864,
		.vts            = 1496,
		.vts_max        = 32767 - 6,
		.line_length_ns = 22190,
		.bayer          = BAYER_BGGR,
		.bpp            = 10,
		.fps            = F24p8(30.0),
	},
#ifdef CONFIG_ARCH_JAVA
	{
		.name           = "1920x1080P30",
		.height         = 1080,
		.width          = 1920,
		.hts            = 2500,
		.vts            = 1120,
		.vts_max        = 32767 - 6,
		.line_length_ns = 29762,
		.bayer          = BAYER_BGGR,
		.bpp            = 10,
		.fps            = F24p8(30.0),
	},
#endif
	{
		.name           = "2592x1944P15",
		.height         = 1944,
		.width          = 2592,
		.hts            = 2844,
		.vts            = 1968,
		.vts_max        = 32767 - 6,
		.line_length_ns = 33875,
		.bayer          = BAYER_BGGR,
		.bpp            = 10,
		.fps            = F24p8(15.0),
	},
	{
		.name           = "STOPPED",
	}
};

/**
 *struct ov5648_otp - ov5648 OTP format
 *@module_integrator_id: 32-bit module integrator ID number
 *@lens_id: 32-bit lens ID number
 *@rg_ratio: 32-bit AWB R/G channel ratio
 *@bg_ratio: 32-bit AWB B/G channel ratio
 *@user_data: 64-bit OTP user data
 *@light_rg: 32-bit light source R/G ratio
 *@light_bg: 32-bit light source B/G ratio
 *
 * Define a structure for OV5648 OTP calibration data
 */
struct ov5648_otp {
	u8  flag;
	u8  module_integrator_id;
	u8  lens_id;
	u16 rg_ratio;
	u16 bg_ratio;
	u8  user_data[2];
	u16 light_rg;
	u16 light_bg;
};

struct ov5648 {
	struct v4l2_subdev subdev;
	struct v4l2_subdev_sensor_interface_parms *plat_parms;
	struct soc_camera_device *icd;
	int state;
	int mode_idx;
	int i_fmt;
	int framerate;
	int focus_mode;
	int gain_current;
	int exposure_current;

	int line_length;
	int vts;
	int vts_max;
	int vts_min;
	int current_coarse_int;
	int framerate_lo;
	int framerate_hi;
	int framerate_lo_absolute;
	int framerate_hi_absolute;

	int current_position;
	int time_to_destination;

	/*
	 * focus_status = 1 focusing
	 * focus_status = 0 focus cancelled or not focusing
	 */
	atomic_t focus_status;
	int aecpos_delay;
#define EXP_DELAY_MAX 8
#define GAIN_DELAY_MAX 8
#define LENS_READ_DELAY 4
	int exp_read_buf[EXP_DELAY_MAX];
	int gain_read_buf[GAIN_DELAY_MAX];
	int lens_read_buf[LENS_READ_DELAY];
	int lenspos_delay;

	int flashmode;
	int flash_intensity;
	int flash_timeout;
	struct ov5648_otp otp;
	int calibrated;
};

/**
 *struct ov5648_reg - ov5648 register format
 *@reg: 16-bit offset to register
 *@val: 8/16/32-bit register value
 *@length: length of the register
 *
 * Define a structure for OV5648 register initialization values
 */
struct ov5648_reg {
	u16	reg;
	u8	val;
	u8	pad;
};

static const struct ov5648_reg ov5648_reginit[256] = {
	{0x0103, 0x01},
	{0x3001, 0x00},
	{0x3002, 0x00},
	{0x3011, 0x02},
	{0x3017, 0x05},
	{0x3018, 0x4c},
	{0x301c, 0xd2},
	{0x3022, 0x00},
	{0x3034, 0x1a},
	{0x3035, 0x21},
#ifdef OV5648_MCLK_26MHZ
	{0x3036, 0x61}, /* MCLK=26MHz MIPI=420MBs*2 Lanes FPS=30 */
#else
	{0x3036, 0x69}, /* MCLK=24MHz MIPI=420MBs*2 Lanes FPS=30 */
#endif
	{0x3037, 0x03},
	{0x3038, 0x00},
	{0x3039, 0x00},
	{0x303a, 0x00},
	{0x303b, 0x19},
	{0x303c, 0x11},
	{0x303d, 0x30},
	{0x3105, 0x11},
	{0x3106, 0x05},
	{0x3304, 0x28},
	{0x3305, 0x41},
	{0x3306, 0x30},
	{0x3308, 0x00},
	{0x3309, 0xc8},
	{0x330a, 0x01},
	{0x330b, 0x90},
	{0x330c, 0x02},
	{0x330d, 0x58},
	{0x330e, 0x03},
	{0x330f, 0x20},
	{0x3300, 0x00},
	{0x3500, 0x00},
	{0x3501, 0x7b},
	{0x3502, 0x00},
	{0x3503, 0x07},
	{0x350a, 0x00},
	{0x350b, 0x40},
	{0x3601, 0x33},
	{0x3602, 0x00},
	{0x3611, 0x0e},
	{0x3612, 0x2b},
	{0x3614, 0x50},
	{0x3620, 0x33},
	{0x3622, 0x00},
	{0x3630, 0xad},
	{0x3631, 0x00},
	{0x3632, 0x94},
	{0x3633, 0x17},
	{0x3634, 0x14},
	{0x3704, 0xc0},
	{0x3705, 0x2a},
	{0x3708, 0x63},
	{0x3709, 0x12},
	{0x370b, 0x23},
	{0x370c, 0xc0},
	{0x370d, 0x00},
	{0x370e, 0x00},
	{0x371c, 0x07},
	{0x3739, 0xd2},
	{0x373c, 0x00},
	{0x3800, 0x00},
	{0x3801, 0x00},
	{0x3802, 0x00},
	{0x3803, 0x00},
	{0x3804, 0x0a},
	{0x3805, 0x3f},
	{0x3806, 0x07},
	{0x3807, 0xa3},
	{0x3808, 0x0a},
	{0x3809, 0x20},
	{0x380a, 0x07},
	{0x380b, 0x98},
	{0x380c, 0x0b},
	{0x380d, 0x00},
	{0x380e, 0x07},
	{0x380f, 0xc0},
	{0x3810, 0x00},
	{0x3811, 0x10},
	{0x3812, 0x00},
	{0x3813, 0x06},
	{0x3814, 0x11},
	{0x3815, 0x11},
	{0x3817, 0x00},
	{0x3820, 0x40},
	{0x3821, 0x06},
	{0x3826, 0x03},
	{0x3829, 0x00},
	{0x382b, 0x0b},
	{0x3830, 0x00},
	{0x3836, 0x00},
	{0x3837, 0x00},
	{0x3838, 0x00},
	{0x3839, 0x04},
	{0x383a, 0x00},
	{0x383b, 0x01},
	{0x3b00, 0x00},
	{0x3b02, 0x08},
	{0x3b03, 0x00},
	{0x3b04, 0x04},
	{0x3b05, 0x00},
	{0x3b06, 0x04},
	{0x3b07, 0x08},
	{0x3b08, 0x00},
	{0x3b09, 0x02},
	{0x3b0a, 0x04},
	{0x3b0b, 0x00},
	{0x3b0c, 0x3d},
	{0x3f01, 0x0d},
	{0x3f0f, 0xf5},
	{0x4000, 0x89},
	{0x4001, 0x02},
	{0x4002, 0x45},
	{0x4004, 0x04},
	{0x4005, 0x18},
	{0x4006, 0x08},
	{0x4007, 0x10},
	{0x4008, 0x00},
	{0x4050, 0x6e},
	{0x4051, 0x8f},
	{0x4300, 0xf8},
	{0x4303, 0xff},
	{0x4304, 0x00},
	{0x4307, 0xff},
	{0x4520, 0x00},
	{0x4521, 0x00},
	{0x4511, 0x22},
	{0x4801, 0x0f},
	{0x4814, 0x2a},
	{0x481f, 0x3c},
	{0x4823, 0x3c},
	{0x4826, 0x00},
	{0x481b, 0x3c},
	{0x4827, 0x32},
	{0x4837, 0x18},
	{0x4b00, 0x06},
	{0x4b01, 0x0a},
	{0x4b04, 0x10},
	{0x5000, 0xff},
	{0x5001, 0x00},
	{0x5002, 0x41},
	{0x5003, 0x0a},
	{0x5004, 0x00},
	{0x5043, 0x00},
	{0x5013, 0x00},
	{0x501f, 0x03},
	{0x503d, 0x00},
	{0x5a00, 0x08},
	{0x5b00, 0x01},
	{0x5b01, 0x40},
	{0x5b02, 0x00},
	{0x5b03, 0xf0},
	{0x0100, 0x01},
	{0xFFFF, 0x00}	/* end of the list */
};

static const struct ov5648_reg ov5648_regdif[OV5648_MODE_MAX][32] = {
	{
	/* to 1280x720P30 */
	{0x301a, 0xf1},
	{0x3708, 0x66},
	{0x3709, 0x52},
	{0x370c, 0xc3},
	{0x3800, 0x00},
	{0x3801, 0x10},
	{0x3802, 0x00},
	{0x3803, 0xfe},
	{0x3804, 0x0a},
	{0x3805, 0x2f},
	{0x3806, 0x06},
	{0x3807, 0xa5},
	{0x3808, 0x05},
	{0x3809, 0x00},
	{0x380a, 0x02},
	{0x380b, 0xd0},
	{0x380c, 0x07},
	{0x380d, 0x48},
	{0x380e, 0x05},
	{0x380f, 0xd8},
	{0x3810, 0x00},
	{0x3811, 0x08},
	{0x3812, 0x00},
	{0x3813, 0x02},
	{0x3814, 0x31},
	{0x3815, 0x31},
#ifdef CONFIG_MACH_JAVA_C_5606
	{0x3820, 0x00},
	{0x3821, 0x07},
#else
	{0x3820, 0x0e},
	{0x3821, 0x01},
#endif
	{0x4004, 0x02},
	{0x4005, 0x1a},
	{0x301a, 0xf0},

	{0xFFFF, 0x00}
	},
	{
	/* to 1280x960P30       */
	{0x301a, 0xf1},
	{0x3708, 0x66},
	{0x3709, 0x52},
	{0x370c, 0xc3},
	{0x3800, 0x00},
	{0x3801, 0x10},
	{0x3802, 0x00},
	{0x3803, 0x06},
	{0x3804, 0x0a},
	{0x3805, 0x2f},
	{0x3806, 0x07},
	{0x3807, 0x9d},
	{0x3808, 0x05},
	{0x3809, 0x00},
	{0x380a, 0x03},
	{0x380b, 0xc0},
	{0x380c, 0x07},
	{0x380d, 0x48},
	{0x380e, 0x05},
	{0x380f, 0xd8},
	{0x3810, 0x00},
	{0x3811, 0x08},
	{0x3812, 0x00},
	{0x3813, 0x06},
	{0x3814, 0x31},
	{0x3815, 0x31},
#ifdef CONFIG_MACH_JAVA_C_5606
	{0x3820, 0x00},
	{0x3821, 0x07},
#else
	{0x3820, 0x0e},
	{0x3821, 0x01},
#endif
	{0x4004, 0x02},
	{0x4005, 0x1a},
	{0x301a, 0xf0},

	{0xFFFF, 0x00}
	},
#ifdef CONFIG_ARCH_JAVA
	{
	/* to 1920x1080P30 */
	{0x301a, 0xf1},
	{0x3708, 0x63},
	{0x3709, 0x12},
	{0x370c, 0xc0},
	{0x3800, 0x01},
	{0x3801, 0x50},
	{0x3802, 0x01},
	{0x3803, 0xb2},
	{0x3804, 0x08},
	{0x3805, 0xef},
	{0x3806, 0x05},
	{0x3807, 0xf1},
	{0x3808, 0x07},
	{0x3809, 0x80},
	{0x380a, 0x04},
	{0x380b, 0x38},
	{0x380c, 0x09},
	{0x380d, 0xc4},
	{0x380e, 0x04},
	{0x380f, 0x60},
	{0x3810, 0x00},
	{0x3811, 0x10},
	{0x3812, 0x00},
	{0x3813, 0x04},
	{0x3814, 0x11},
	{0x3815, 0x11},
#ifdef CONFIG_MACH_JAVA_C_5606
	{0x3820, 0x00},
	{0x3821, 0x07},
#else
	{0x3820, 0x46},
	{0x3821, 0x00},
#endif
	{0x4004, 0x04},
	{0x4005, 0x18},
	{0x301a, 0xf0},

	{0xFFFF, 0x00}
	},
#endif
	{
	/* to 2592x1944P15       */
	{0x301a, 0xf1},
	{0x3708, 0x63},
	{0x3709, 0x12},
	{0x370c, 0xc0},
	{0x3800, 0x00},
	{0x3801, 0x00},
	{0x3802, 0x00},
	{0x3803, 0x00},
	{0x3804, 0x0a},
	{0x3805, 0x3f},
	{0x3806, 0x07},
	{0x3807, 0xa3},
	{0x3808, 0x0a},
	{0x3809, 0x20},
	{0x380a, 0x07},
	{0x380b, 0x98},
	{0x380c, 0x0b},
	{0x380d, 0x1c},
	{0x380e, 0x07},
	{0x380f, 0xb0},
	{0x3810, 0x00},
	{0x3811, 0x10},
	{0x3812, 0x00},
	{0x3813, 0x06},
	{0x3814, 0x11},
	{0x3815, 0x11},
#ifdef CONFIG_MACH_JAVA_C_5606
	{0x3820, 0x00},
	{0x3821, 0x06},
#else
	{0x3820, 0x06},
	{0x3821, 0x00},
#endif
	{0x4004, 0x04},
	{0x4005, 0x1a},
	{0x301a, 0xf0},

	{0xFFFF, 0x00}
	}
};

static const struct ov5648_reg ov5648_reg_state[OV5648_STATE_MAX][3] = {
	{ /* to power down */
	{0x0100, 0x00},	       /* disable streaming  */
	{0x3018, 0x5c},        /* disable mipi */
	{0xFFFF, 0x00}
	},
	{ /* to streaming */
	{0x3018, 0x4c},         /* enable mipi */
	{0x0100, 0x01},		/* enable streaming */
	{0xFFFF, 0x00}
	},
};

static int set_flash_mode(struct i2c_client *client, int mode);
static int ov5648_set_mode(struct i2c_client *client, int new_mode_idx);
static int ov5648_set_state(struct i2c_client *client, int new_state);
static int ov5648_init(struct i2c_client *client);

/*add an timer to close the flash after two frames*/
#if defined(CONFIG_MACH_JAVA_C_LC1)
static struct timer_list timer;
static char *msg = "hello world";
static void print_func(unsigned long lparam)
{
	gpio_set_value(TORCH_EN, 0);
	gpio_set_value(FLASH_EN, 0);
}
#endif
/*
 * Find a data format by a pixel code in an array
 */
static int ov5648_find_datafmt(enum v4l2_mbus_pixelcode code)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(ov5648_fmts); i++)
		if (ov5648_fmts[i].code == code)
			break;

	/* If not found, select latest */
	if (i >= ARRAY_SIZE(ov5648_fmts))
		i = ARRAY_SIZE(ov5648_fmts) - 1;

	return i;
}

/* Find a frame size in an array */
static int ov5648_find_framesize(u32 width, u32 height)
{
	int i;

	for (i = 0; i < OV5648_MODE_MAX; i++) {
		if ((ov5648_mode[i].width >= width) &&
			(ov5648_mode[i].height >= height))
			break;
	}

	/* If not found, select biggest */
	if (i >= OV5648_MODE_MAX)
		i = OV5648_MODE_MAX - 1;

	return i;
}

static struct ov5648 *to_ov5648(const struct i2c_client *client)
{
	return container_of(i2c_get_clientdata(client), struct ov5648, subdev);
}

/**
 *ov5648_reg_read - Read a value from a register in an ov5648 sensor device
 *@client: i2c driver client structure
 *@reg: register address / offset
 *@val: stores the value that gets read
 *
 * Read a value from a register in an ov5648 sensor device.
 * The value is returned in 'val'.
 * Returns zero if successful, or non-zero otherwise.
 */
static int ov5648_reg_read(struct i2c_client *client, u16 reg, u8 *val)
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
 * Write a value to a register in ov5648 sensor device.
 *@client: i2c driver client structure.
 *@reg: Address of the register to read value from.
 *@val: Value to be written to a specific register.
 * Returns zero if successful, or non-zero otherwise.
 */
static int ov5648_reg_write(struct i2c_client *client, u16 reg, u8 val)
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

static int ov5648_reg_read_multi(struct i2c_client *client,
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
	dev_err(&client->dev, "Failed multiple reading register 0x%02x\n", reg);
	return ret;
}

static const struct v4l2_queryctrl ov5648_controls[] = {
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
	{
	 .id = V4L2_CID_CAMERA_FOCUS_MODE,
	 .type = V4L2_CTRL_TYPE_INTEGER,
	 .name = "Focus Modes",
	 .minimum = FOCUS_MODE_AUTO,
	 .maximum = (1 << FOCUS_MODE_AUTO | 1 << FOCUS_MODE_MACRO
			 | 1 << FOCUS_MODE_INFINITY | 1 << FOCUS_MODE_MANUAL),
	 .step = 1,
	 .default_value = FOCUS_MODE_AUTO,
	 },
	{
	 .id = V4L2_CID_GAIN,
	 .type = V4L2_CTRL_TYPE_INTEGER,
	 .name = "Gain",
	 .minimum = OV5648_GAIN_MIN,
	 .maximum = OV5648_GAIN_MAX,
	 .step = OV5648_GAIN_STEP,
	 .default_value = DEFAULT_GAIN,
	 },
	{
	 .id = V4L2_CID_EXPOSURE,
	 .type = V4L2_CTRL_TYPE_INTEGER,
	 .name = "Exposure",
	 .minimum = OV5648_EXP_MIN,
	 .maximum = OV5648_EXP_MAX,
	 .step = OV5648_EXP_STEP,
	 .default_value = DEFAULT_EXPO,
	 },
	{
	 .id = V4L2_CID_CAMERA_LENS_POSITION,
	 .type = V4L2_CTRL_TYPE_INTEGER,
	 .name = "Lens Position",
	 .minimum = OV5648_LENS_MIN,
	 .maximum = OV5648_LENS_MAX,
	 .step = OV5648_LENS_STEP,
	 .default_value = DEFAULT_LENS_POS,
	 },
	{
	 .id = V4L2_CID_CAMERA_FLASH_MODE,
	 .type = V4L2_CTRL_TYPE_INTEGER,
#ifdef CONFIG_VIDEO_AS3643
	 .name = "AS3643-flash",
#endif

#if defined(CONFIG_MACH_JAVA_C_LC1)
	 .name = "OCP8111-flash",
#endif
	 .minimum = FLASH_MODE_OFF,
	 .maximum = (1 << FLASH_MODE_OFF) | (1 << FLASH_MODE_ON) |
		(1 << FLASH_MODE_TORCH_OFF) | (1 << FLASH_MODE_TORCH_ON),
	 .step = 1,
	 .default_value = FLASH_MODE_OFF,
	 },
	{
	 .id = V4L2_CID_FLASH_INTENSITY,
	 .type = V4L2_CTRL_TYPE_INTEGER,
	 .name = "Intensity in mA",
	 .minimum = OV5648_FLASH_INTENSITY_MIN,
	 .maximum = OV5648_FLASH_INTENSITY_MAX,
	 .step = OV5648_FLASH_INTENSITY_STEP,
	 .default_value = OV5648_FLASH_INTENSITY_DEFAULT,
	 },
	{
	 .id = V4L2_CID_FLASH_TIMEOUT,
	 .type = V4L2_CTRL_TYPE_INTEGER,
	 .name = "Flash timeout in us",
	 .minimum = OV5648_FLASH_TIMEOUT_MIN,
	 .maximum = OV5648_FLASH_TIMEOUT_MAX,
	 .step = OV5648_FLASH_TIMEOUT_STEP,
	 .default_value = OV5648_FLASH_TIMEOUT_DEFAULT,
	 },
};

/**
 * Initialize a list of ov5648 registers.
 * The list of registers is terminated by the pair of values
 *@client: i2c driver client structure.
 *@reglist[]: List of address of the registers to write data.
 * Returns zero if successful, or non-zero otherwise.
 */
static int ov5648_reg_writes(struct i2c_client *client,
			     const struct ov5648_reg reglist[])
{
	int err = 0, i;

	for (i = 0; reglist[i].reg != 0xFFFF; i++) {
		if (reglist[i].reg == 0xFFFE) {
			msleep(reglist[i].val);
		} else {
			err |= ov5648_reg_write(client, reglist[i].reg,
						reglist[i].val);
		}
		if (err != 0)
			break;
	}
	return 0;
}

#ifdef OV5648_DEBUG
static int ov5648_reglist_compare(struct i2c_client *client,
			const struct ov5648_reg reglist[])
{
	int err = 0;
	u8 reg, i;

	for (i = 0; ((reglist[i].reg != 0xFFFF) && (err == 0)); i++) {
		if (reglist[i].reg != 0xFFFE) {
			err |= ov5648_reg_read(client, reglist[i].reg, &reg);
			pr_debug("ov5648_reglist_compare: [0x%04x]=0x%02x",
				 reglist[i].reg, reg);
		} else {
			msleep(reglist[i].val);
		}
	}
	return 0;
}
#endif

/**
 * Write an array of data to ov5648 sensor device.
 *@client: i2c driver client structure.
 *@reg: Address of the register to read value from.
 *@data: pointer to data to be written starting at specific register.
 *@size: # of data to be written starting at specific register.
 * Returns zero if successful, or non-zero otherwise.
 */
static int ov5648_array_write(struct i2c_client *client,
					const u8 *data, u16 size)
{
	int ret;
	struct i2c_msg msg = {
		.addr = client->addr,
		.flags = 0,
		.len = size,
		.buf = (u8 *)data,
	};

	ret = i2c_transfer(client->adapter, &msg, 1);
	if (ret < 0) {
		dev_err(&client->dev, "Failed writing array to 0x%02x!\n",
			((data[0] << 8) | (data[1])));
		return ret;
	}

	return 0;
}


static const struct ov5648_reg ov5648_otpbank_rdsel[2][8] = {
	{ /* bank 0 */
	{0x3d84, 0xc0},
	{0x3d85, 0x00},
	{0x3d86, 0x0f},
	{0x3d81, 0x01},
	{0xFFFE,   20}, /* delay 20 ms */
	{0xFFFF, 0x00}
	},
	{ /* bank 1 */
	{0x3d84, 0xc0},
	{0x3d85, 0x10},
	{0x3d86, 0x1f},
	{0x3d81, 0x01},
	{0xFFFE,   20}, /* delay 20 ms */
	{0xFFFF, 0x00}
	}
};

/**
 * Read OTP data into buf
 *@client: i2c driver client structure.
 *@bank: OTP bank index
 *@buf: buffer for OTP data
 * Returns zero if successful or non-zero otherwise.
 */
static int ov5648_otp_bank_read(struct i2c_client *client, int bank, u8 *buf)
{
	int ret, i;
	u8 val;

	pr_debug("ov5648_otp_bank_read: bank=%d\n", bank);
	ret = ov5648_reg_writes(client, ov5648_otpbank_rdsel[bank&1]);
	if (ret != 0) {
		dev_err(&client->dev, "OTP bank %d read select failed\n", bank);
		return -1;
	}
	for (i = 0; i < 0x10; i++) {
		ret |= ov5648_reg_read(client, 0x3d00+i, &val);
		buf[i] = val;
	}
	pr_debug("ov5648_otp_bank_read: OK bank=%d\n", bank);
	return 0;
}

/**
 * Print ov5648 OTP calibration data
 *@otp: Address of otp structure to print
 *@index: OTP index
 */
#if OV5648_DEBUG
static void ov5648_otp_print(struct ov5648_otp *otp, int index)
{
	pr_debug("ov5648: OTP index=%d\n", index);
	pr_debug("ov5648: integr_id = 0x%02X\n", otp->module_integrator_id);
	pr_debug("ov5648: lens_id   = 0x%02X\n", otp->lens_id);
	pr_debug("ov5648: rg_ratio  = 0x%04X\n", otp->rg_ratio);
	pr_debug("ov5648: bg_ratio  = 0x%04X\n", otp->bg_ratio);
	pr_debug("ov5648: light_rg  = 0x%04X\n", otp->light_rg);
	pr_debug("ov5648: light_bg  = 0x%04X\n", otp->light_bg);
	pr_debug("ov5648: user_data = [0x%02X 0x%02X]\n",
		 otp->user_data[0], otp->user_data[1]);
}

/**
 * Read and print ov5648 OTP R/B channel gains.
 *@client: i2c driver client structure.
 */
static void ov5648_rbgains_print(struct i2c_client *client)
{
	u8 val;
	u16 gainr, gaing, gainb;
	int i;
	ov5648_reg_read(client, 0x5001, &val);
	pr_debug("ov5648 0x%04X=0x%02X]\n", 0x5001, val);
	ov5648_reg_read(client, 0x5002, &val);
	pr_debug("ov5648 0x%04X=0x%02X]\n", 0x5002, val);
	ov5648_reg_read(client, 0x5180, &val);
	pr_debug("ov5648 0x%04X=0x%02X]\n", 0x5180, val);
	ov5648_reg_read(client, 0x5186, &val);
	gainr = val << 8;
	ov5648_reg_read(client, 0x5187, &val);
	gainr += val;
	ov5648_reg_read(client, 0x5188, &val);
	gaing = val << 8;
	ov5648_reg_read(client, 0x5189, &val);
	gaing += val;
	ov5648_reg_read(client, 0x518a, &val);
	gainb = val << 8;
	ov5648_reg_read(client, 0x518b, &val);
	gainb += val;
	pr_debug("ov5648 rb gains readback = [%04X %04X %04X]\n",
		 gainr, gaing, gainb);
}
#endif

/**
 * Convert OTP binary data to formatted calibration information
 *@otp: Address of otp structure to write values into
 *@buf: otp binary data
 * Returns zero if successful or non-zero otherwise.
 */
static void ov5648_buf2otp(struct ov5648_otp *otp, u8 *buf)
{
	otp->flag = buf[0] & 0x08;
	otp->module_integrator_id = buf[0] & 0x7f;
	otp->lens_id  = buf[1];
	otp->rg_ratio = (buf[2] << 2) + ((buf[6] >> 6) & 3);
	otp->bg_ratio = (buf[3] << 2) + ((buf[6] >> 4) & 3);
	otp->light_rg = (buf[7] << 2) + ((buf[6] >> 2) & 3);
	otp->light_bg = (buf[8] << 2) + ((buf[6] >> 0) & 3);
	otp->user_data[0] = buf[4];
	otp->user_data[1] = buf[5];
}

/**
 * Read OTP calibration data at index from ov5648 sensor.
 *@client: i2c driver client structure.
 *@otp: Address of otp structure to read values into
 *@bank: otp index select
 * Returns zero if OTP is present and read is successful
 * or non-zero otherwise.
 */
static int ov5648_otp_index_read(struct i2c_client *client,
				 struct ov5648_otp *otp, int index)
{
	int ret = -1;
	u8 buf[32];

	pr_debug("ov5648_otp_index_read: index %d OTP read\n", index);
	memset(otp, 0, sizeof(*otp));
	switch (index) {
	case 0:
		ret = ov5648_otp_bank_read(client, 0, buf);
		if (ret < 0)
			break;
		ov5648_buf2otp(otp, buf+5);
		break;
	case 1:
		ret = ov5648_otp_bank_read(client, 0, buf);
			break;
		if (ret < 0)
			break;
		ov5648_buf2otp(otp, buf+0xe);
		break;
	case 2:
		ret = ov5648_otp_bank_read(client, 0, buf);
		if (ret < 0)
			break;
		ov5648_buf2otp(otp, buf+7);
		break;
	default:
		pr_debug("ov5648_otp_index_read: invalid OTP idx: %d\n",
			 index);
		ret = -1;
		break;
	}

	pr_debug("ov5648_otp_index_read: rg_ratio  = 0x%04X\n", otp->rg_ratio);
	pr_debug("ov5648_otp_index_read: bg_ratio  = 0x%04X\n", otp->bg_ratio);
#if OV5648_DEBUG
	ov5648_otp_print(otp, index);
#endif
	if (ret != 0) {
		pr_debug("ov5648_otp_index_read: index %d OTP read failed\n",
			 index);
		memset(otp, 0, sizeof(*otp));
		return -1;
	}
	if (otp->flag == 1 || otp->rg_ratio == 0 || otp->bg_ratio == 0) {
		memset(otp, 0, sizeof(*otp));
		return -1;
	}
	return 0;
}

/**
 * Find and read OTP calibration data from ov5648 sensor.
 *@client: i2c driver client structure.
 *@otp: Address of otp structure to read values into
 *@bank: otp index select
 * Returns zero if OTP is present and read is successful
 * or non-zero otherwise.
 */
static int ov5648_otp_read(struct i2c_client *client)
{
	int i, ret;
	struct ov5648 *ov5648 = to_ov5648(client);
	struct ov5648_otp *otp = &(ov5648->otp);

	for (i = 0; i < 3; i++) {
		ret = ov5648_otp_index_read(client, otp, i);
		if (ret == 0)
			break;
	}
	return ret;
}

/**
 * Set Sensor R and B channel gains according to OTP calibration data.
 *@client: i2c driver client structure.
 *@otp: Address of otp structure with calibration data
 * Returns zero if successful, or non-zero otherwise.
 */
static int ov5648_rbgains_update(struct i2c_client *client)
{
#define RG_GOLDEN 0x145
#define BG_GOLDEN 0x15e

	u16 gainr, gaing, gainb, ggainb, ggainr;
	int ret;
	struct ov5648 *ov5648 = to_ov5648(client);
	struct ov5648_otp *otp = &(ov5648->otp);
	if (otp->rg_ratio == 0 || otp->bg_ratio == 0) {
		pr_debug("ov5648_rbgains_update: OTP not initialized\n");
		return -1;
	}
# if 0
	gainr = 0x400 * RG_GOLDEN / otp->rg_ratio;
	gaing = 0x400;
	gainb = 0x400 * BG_GOLDEN / otp->bg_ratio;
#else
	if (otp->bg_ratio < BG_GOLDEN) {
		if (otp->rg_ratio < RG_GOLDEN) {
			gaing = 0x400;
			gainb = 0x400 * BG_GOLDEN / otp->bg_ratio;
			gainr = 0x400 * RG_GOLDEN / otp->rg_ratio;
		} else {
			gainr = 0x400;
			gaing = 0x400 * otp->rg_ratio / RG_GOLDEN;
			gainb = gaing * BG_GOLDEN / otp->bg_ratio;
		}
	} else {
		if (otp->rg_ratio < RG_GOLDEN) {
			gainb = 0x400;
			gaing = 0x400 * otp->bg_ratio / BG_GOLDEN;
			gainr = gaing * RG_GOLDEN / otp->rg_ratio;
		} else {
			ggainb = 0x400 * otp->bg_ratio / BG_GOLDEN;
			ggainr = 0x400 * otp->rg_ratio / RG_GOLDEN;
			if (ggainb > ggainr) {
				gainb = 0x400;
				gaing = ggainb;
				gainr = gaing * RG_GOLDEN / otp->rg_ratio;
			} else {
				gainr = 0x400;
				gaing = ggainr;
				gainb = gaing * BG_GOLDEN / otp->bg_ratio; }
		}
	}
#endif
	ret =  ov5648_reg_write(client, 0x5180, 1<<3);
	ret |= ov5648_reg_write(client, 0x5186, gainr >> 8);
	ret |= ov5648_reg_write(client, 0x5187, gainr & 0xff);
	ret |= ov5648_reg_write(client, 0x5188, gaing >> 8);
	ret |= ov5648_reg_write(client, 0x5189, gaing & 0xff);
	ret |= ov5648_reg_write(client, 0x518a, gainb >> 8);
	ret |= ov5648_reg_write(client, 0x518b, gainb & 0xff);
	if (ret == 0) {
		dev_info(&client->dev, "ov5648 1 sensor update for calibrated data g=[0x%04X, 0x%04X, 0x%04X]\n",
			 gainr, gaing, gainb);
#if OV5648_DEBUG
		ov5648_rbgains_print(client);
#endif
		return 0;
	} else {
		dev_info(&client->dev,
			 "ov5648 sensor 1 update for calibrated data failed\n");
		return -1;
	}
}

/*
 *Routine used to send lens to a traget position position and calculate
 *the estimated time required to get to this position, the flying time in us.
*/
static int ov5648_lens_set_position(struct i2c_client *client,
				    int target_position)
{
	int ret = 0;

#ifdef CONFIG_VIDEO_A3907
	if (target_position & 0x80000000) {
		int fine_target_position = target_position & ~0x80000000;
		ret = a3907_lens_set_position_fine(fine_target_position);
	} else {
		ret = a3907_lens_set_position(target_position);
	}
#endif

#ifdef CONFIG_VIDEO_DW9714
	if (target_position & 0x80000000) {
		int fine_target_position = target_position & ~0x80000000;
		ret = dw9714_lens_set_position_fine(fine_target_position);
	} else {
		ret = dw9714_lens_set_position(target_position);
	}
#endif
	return ret;
}


/*
 * Routine used to get the current lens position and/or the estimated
 *time required to get to the requested destination (time in us).
*/
static void ov5648_lens_get_position(struct i2c_client *client,
					int *current_position,
					int *time_to_destination)
{
	int ret = 0;
	int i = 0;
	struct ov5648 *ov5648 = to_ov5648(client);

	static int lens_read_buf[LENS_READ_DELAY];
#ifdef CONFIG_VIDEO_A3907
	ret = a3907_lens_get_position(current_position, time_to_destination);
#endif

#ifdef CONFIG_VIDEO_DW9714
	ret = dw9714_lens_get_position(current_position, time_to_destination);
#endif

	for (i = 0; i < ov5648->lenspos_delay; i++)
		lens_read_buf[i] = lens_read_buf[i + 1];
	lens_read_buf[ov5648->lenspos_delay] = *current_position;
	*current_position = lens_read_buf[0];

	return;
}

/*
 * Setup the sensor integration and frame length based on requested mimimum
 * framerate
 */
static void ov5648_set_framerate_lo(struct i2c_client *client, int framerate)
{
	struct ov5648 *ov5648 = to_ov5648(client);
	framerate = max(framerate, ov5648->framerate_lo_absolute);
	framerate = min(framerate, ov5648->framerate_hi_absolute);

	ov5648->framerate_lo = framerate;
	ov5648->vts_max = 1000000000
		/ (unsigned long)((ov5648->line_length * framerate) >> 8);
	ov5648->vts_max = min(ov5648->vts_max,
			ov5648_mode[ov5648->mode_idx].vts_max);
	pr_debug("ov5648_set_framerate_lo: Setting frame rate lo %d vts_max %d",
			ov5648->framerate_lo, ov5648->vts_max);
}

/*
 * Setup the sensor integration and frame length based on requested maximum
 * framerate, famerate is 24p8 fixed point.
 */
static void ov5648_set_framerate_hi(struct i2c_client *client, int framerate)
{
	struct ov5648 *ov5648 = to_ov5648(client);

	framerate = max(framerate, ov5648->framerate_lo_absolute);
	framerate = min(framerate, ov5648->framerate_hi_absolute);

	ov5648->framerate_hi = framerate;
	ov5648->vts_min = 1000000000
		/ (unsigned long)((ov5648->line_length * framerate) >> 8);

	ov5648->vts_min = max(ov5648->vts_min,
			ov5648_mode[ov5648->mode_idx].vts);
	pr_debug("ov5648_set_framerate_hi: Setting frame rate hi %d vts_min %d",
		ov5648->framerate_hi, ov5648->vts_min);
}


/*
 * Determine the closest achievable gain to the desired gain.
 * Calculates the closest achievable gain to the requested gain
 * that can be achieved by this sensor.
 * @param gain_value Desired gain on 8.8 linear scale.
 * @return Achievable gain on 8.8 linear scale.
*/
static int ov5648_calc_gain(struct i2c_client *client, int gain_value,
					int *gain_code_p)
{
	int gain_code = (gain_value  & 0x3fff) >> 4;
	int actual_gain = gain_code << 4;

	if (gain_code_p)
		*gain_code_p = gain_code;
	return actual_gain;
}

/*
 *setup the sensor analog gain on requested gain value 8.8 linear scale.
*/
#define GAIN_HIST_MAX 0
static int ov5648_set_gain(struct i2c_client *client, int gain_value)
{
	struct ov5648 *ov5648 = to_ov5648(client);
	int ret = 0;
	int gain_code_analog, gain_actual;

#if GAIN_HIST_MAX > 0
	int i;
	static int gain_prev[GAIN_HIST_MAX] = {DEFAULT_GAIN, DEFAULT_GAIN,
					       DEFAULT_GAIN, DEFAULT_GAIN};

	for (i = 0; i < GAIN_HIST_MAX - 1; i++)
		gain_prev[i] = gain_prev[i + 1];
	gain_prev[GAIN_HIST_MAX-1] = gain_value;
	gain_value = 0;
	for (i = 0; i < GAIN_HIST_MAX ; i++)
		gain_value = gain_value + gain_prev[i];
	gain_value = gain_value / GAIN_HIST_MAX;
#endif
	gain_actual = ov5648_calc_gain(client, gain_value, &gain_code_analog);
	pr_debug("ov5648_set_gain: cur=%u req=%u act=%u cod=%u",
		 ov5648->gain_current, gain_value,
		 gain_actual, gain_code_analog);
	if (gain_actual == ov5648->gain_current)
		return 0;
	ret = ov5648_reg_write(client,
		OV5648_REG_AGC_HI, (gain_code_analog >> 8) & 0x3);
	ret |= ov5648_reg_write(client,
		OV5648_REG_AGC_LO, gain_code_analog & 0xFF);
	ov5648->gain_current = gain_actual;
	return 0;
}

static int ov5648_get_gain(struct i2c_client *client,
		int *gain_value_p, int *gain_code_p)
{
	struct ov5648 *ov5648 = to_ov5648(client);

	int ret = 0;
	int gain_code;
	u8 gain_buf[2];
	int i;

	ov5648_reg_read_multi(client, OV5648_REG_AGC_HI, gain_buf, 2);
	gain_code = ((gain_buf[0] & 0x3f) << 8) + gain_buf[1];

	if (ov5648->aecpos_delay > 0) {
		ov5648->gain_read_buf[ov5648->aecpos_delay] = gain_code;
		gain_code = ov5648->gain_read_buf[0];
		for (i = 0; i < GAIN_DELAY_MAX-1; i++)
			ov5648->gain_read_buf[i] = ov5648->gain_read_buf[i+1];
	}
	if (gain_code < 0x10)
		gain_code = 0x10;

	if (gain_code_p)
		*gain_code_p = gain_code;
	if (gain_value_p)
		*gain_value_p = gain_code << 4;
	return ret;
}

static int ov5648_get_exposure(struct i2c_client *client,
		int *exp_value_p, int *exp_code_p)
{
	struct ov5648 *ov5648 = to_ov5648(client);
	int i, ret = 0;
	int exp_code;
	u8 exp_buf[3];

	ov5648_reg_read_multi(client, OV5648_REG_EXP_HI, exp_buf, 3);
	exp_code =
		((exp_buf[0] & 0xf) << 16) +
		((exp_buf[1] & 0xff) << 8) +
		(exp_buf[2] & 0xf0);

	if (ov5648->aecpos_delay > 0) {
		ov5648->exp_read_buf[ov5648->aecpos_delay] = exp_code;
		exp_code = ov5648->exp_read_buf[0];
		for (i = 0; i < EXP_DELAY_MAX-1; i++)
			ov5648->exp_read_buf[i] = ov5648->exp_read_buf[i+1];
	}

	if (exp_code_p)
		*exp_code_p = exp_code;
	if (exp_value_p)
		*exp_value_p = ((exp_code >> 4) * ov5648->line_length) /
			(unsigned long)1000;
	return ret;
}

#define INTEGRATION_MIN		1
#define INTEGRATION_OFFSET	10

static int ov5648_calc_exposure(struct i2c_client *client,
				int exposure_value,
				unsigned int *vts_ptr,
				unsigned int *coarse_int_lines)
{
	struct ov5648 *ov5648 = to_ov5648(client);
	unsigned int integration_lines;
	int vts = ov5648_mode[ov5648->mode_idx].vts;

	integration_lines = (1000 * exposure_value) / ov5648->line_length;
	if (integration_lines < INTEGRATION_MIN)
		integration_lines = INTEGRATION_MIN;
	else if (integration_lines > (unsigned int)
		 (ov5648->vts_max - INTEGRATION_OFFSET))
		integration_lines = ov5648->vts_max - INTEGRATION_OFFSET;
	vts = min(integration_lines + INTEGRATION_OFFSET,
		  (unsigned int)ov5648_mode[ov5648->mode_idx].vts_max);
	vts = max(vts, ov5648_mode[ov5648->mode_idx].vts);
	vts = max(vts, ov5648->vts_min);
	if (coarse_int_lines)
		*coarse_int_lines = integration_lines;
	if (vts_ptr)
		*vts_ptr = vts;

	exposure_value = integration_lines * ov5648->line_length /
		(unsigned long)1000;
	return exposure_value;
}

/*
 * Setup the sensor integration and frame length based on requested exposure
 * in microseconds.
 */
#define EXP_HIST_MAX 0
static void ov5648_set_exposure(struct i2c_client *client, int exp_value)
{
	struct ov5648 *ov5648 = to_ov5648(client);
	u8 exp_buf[5];
	unsigned int actual_exposure;
	unsigned int vts;
	unsigned int coarse_int_lines;
	int ret = 0;

#if EXP_HIST_MAX > 0
	int i;
	static int exp_prev[EXP_HIST_MAX];

	for (i = 0; i < EXP_HIST_MAX - 1; i++)
		exp_prev[i] = exp_prev[i + 1];
	exp_prev[EXP_HIST_MAX - 1] = exp_value;
	exp_value = 0;
	for (i = 0; i < EXP_HIST_MAX; i++)
			exp_value = exp_value + exp_prev[i];
	exp_value = exp_value / EXP_HIST_MAX;
#endif

	actual_exposure = ov5648_calc_exposure(client, exp_value,
			&vts, &coarse_int_lines);
	pr_debug("ov5648_set_exposure: cur=%d req=%d act=%d coarse=%d vts=%d",
			ov5648->exposure_current, exp_value, actual_exposure,
			coarse_int_lines, vts);
	ov5648->vts = vts;
	ov5648->exposure_current = actual_exposure;
	coarse_int_lines <<= 4;
	exp_buf[0] = (u8)((OV5648_REG_EXP_HI >> 8) & 0xFF);
	exp_buf[1] = (u8)(OV5648_REG_EXP_HI & 0xFF);
	exp_buf[2] = (u8)((coarse_int_lines >> 16) & 0xFF);
	exp_buf[3] = (u8)((coarse_int_lines >> 8) & 0xFF);
	exp_buf[4] = (u8)((coarse_int_lines >> 0) & 0xF0);
	ret = ov5648_array_write(client, exp_buf, 5);
	ret |= ov5648_reg_write(client,
			OV5648_REG_TIMING_VTS_HI, (vts & 0xFF00) >> 8);
	ret |= ov5648_reg_write(client, OV5648_REG_TIMING_VTS_LO, vts & 0xFF);
	if (ret) {
		pr_debug("ov5648_set_exposure: error writing exp. ctrl");
		return;
	}
}

static int ov5648_s_stream(struct v4l2_subdev *sd, int enable)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int ret;

	ret = ov5648_set_state(client, enable);
	return ret;
}

static int ov5648_set_bus_param(struct soc_camera_device *icd,
				unsigned long flags)
{
	/* TODO: Do the right thing here, and validate bus params */
	return 0;
}

static unsigned long ov5648_query_bus_param(struct soc_camera_device *icd)
{
	unsigned long flags = SOCAM_PCLK_SAMPLE_FALLING |
		SOCAM_HSYNC_ACTIVE_HIGH | SOCAM_VSYNC_ACTIVE_HIGH |
		SOCAM_DATA_ACTIVE_HIGH | SOCAM_MASTER;

	/* TODO: Do the right thing here, and validate bus params */

	flags |= SOCAM_DATAWIDTH_10;

	return flags;
}

static int ov5648_enum_input(struct soc_camera_device *icd,
			     struct v4l2_input *inp)
{
	struct soc_camera_link *icl = to_soc_camera_link(icd);
	struct v4l2_subdev_sensor_interface_parms *plat_parms;

	inp->type = V4L2_INPUT_TYPE_CAMERA;
	inp->std = V4L2_STD_UNKNOWN;
	strcpy(inp->name, "ov5648");

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

static int ov5648_g_fmt(struct v4l2_subdev *sd, struct v4l2_mbus_framefmt *mf)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ov5648 *ov5648 = to_ov5648(client);

	mf->width = ov5648_mode[ov5648->mode_idx].width;
	mf->height = ov5648_mode[ov5648->mode_idx].height;
	mf->code = ov5648_fmts[ov5648->i_fmt].code;
	mf->colorspace = ov5648_fmts[ov5648->i_fmt].colorspace;
	mf->field = V4L2_FIELD_NONE;

	return 0;
}

static int ov5648_try_fmt(struct v4l2_subdev *sd, struct v4l2_mbus_framefmt *mf)
{
	int i_fmt;
	int mode_idx;

	i_fmt = ov5648_find_datafmt(mf->code);

	mf->code = ov5648_fmts[i_fmt].code;
	mf->colorspace = ov5648_fmts[i_fmt].colorspace;
	mf->field = V4L2_FIELD_NONE;

	mode_idx = ov5648_find_framesize(mf->width, mf->height);

	mf->width = ov5648_mode[mode_idx].width;
	mf->height = ov5648_mode[mode_idx].height;

	return 0;
}

static int ov5648_set_mode(struct i2c_client *client, int new_mode_idx)
{
	struct ov5648 *ov5648 = to_ov5648(client);
	int ret = 0;

	if (ov5648->mode_idx == new_mode_idx) {
		pr_debug("ov5648_set_mode: skip init from mode[%d]=%s to mode[%d]=%s",
			ov5648->mode_idx, ov5648_mode[ov5648->mode_idx].name,
			new_mode_idx, ov5648_mode[new_mode_idx].name);
		return ret;
	}

	if (ov5648->mode_idx == OV5648_MODE_MAX) {
		pr_debug("ov5648_set_mode: full init from mode[%d]=%s to mode[%d]=%s",
		ov5648->mode_idx, ov5648_mode[ov5648->mode_idx].name,
		new_mode_idx, ov5648_mode[new_mode_idx].name);
		ov5648_init(client);
		ret = ov5648_reg_writes(client, ov5648_reginit);
		ret = ov5648_reg_writes(client, ov5648_regdif[new_mode_idx]);
	} else {
		pr_debug("ov5648_set_mode: diff init from mode[%d]=%s to mode[%d]=%s",
			ov5648->mode_idx, ov5648_mode[ov5648->mode_idx].name,
			new_mode_idx, ov5648_mode[new_mode_idx].name);
		ret = ov5648_reg_writes(client, ov5648_regdif[new_mode_idx]);
	}
	if (ret)
		return ret;

	ov5648->mode_idx = new_mode_idx;
	ov5648->line_length  = ov5648_mode[new_mode_idx].line_length_ns;
	ov5648->vts = ov5648_mode[new_mode_idx].vts;
	ov5648->framerate_lo_absolute = F24p8(1.0);
	ov5648->framerate_hi_absolute = ov5648_mode[new_mode_idx].fps;
	ov5648_set_framerate_lo(client, ov5648->framerate_lo_absolute);
	ov5648_set_framerate_hi(client, ov5648->framerate_hi_absolute);
	ov5648_set_exposure(client, ov5648->exposure_current);
	ov5648_set_gain(client, ov5648->gain_current);
	return 0;
}

static int ov5648_set_state(struct i2c_client *client, int new_state)
{
	struct ov5648 *ov5648 = to_ov5648(client);
	int ret = 0;

	pr_debug("ov5648_set_state: %d (%s) -> %d (%s)", ov5648->state,
		 ov5648->state ? "strm" : "stop",
		 new_state, new_state ? "strm" : "stop");

	if (ov5648->state != new_state) {
		ret = ov5648_reg_writes(client, ov5648_reg_state[new_state]);
		ov5648->state = new_state;
		if (OV5648_STATE_STRM == new_state &&
		    0 == ov5648->calibrated) {
			ov5648_otp_read(client);
			ov5648_rbgains_update(client);
			ov5648->calibrated = 1;
		}
	}
	return ret;
}

static int ov5648_s_fmt(struct v4l2_subdev *sd, struct v4l2_mbus_framefmt *mf)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ov5648 *ov5648 = to_ov5648(client);
	int ret = 0;
	int new_mode_idx;

	ret = ov5648_try_fmt(sd, mf);
	if (ret < 0)
		return ret;

	new_mode_idx = ov5648_find_framesize(mf->width, mf->height);
	ov5648->i_fmt = ov5648_find_datafmt(mf->code);
	switch ((u32) ov5648_fmts[ov5648->i_fmt].code) {
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

	ov5648_set_mode(client, new_mode_idx);
	return ret;
}

static int ov5648_g_chip_ident(struct v4l2_subdev *sd,
			       struct v4l2_dbg_chip_ident *id)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	if (id->match.type != V4L2_CHIP_MATCH_I2C_ADDR)
		return -EINVAL;

	if (id->match.addr != client->addr)
		return -ENODEV;

	id->ident = V4L2_IDENT_OV5648;
	id->revision = 0;

	return 0;
}


static int ov5648_g_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ov5648 *ov5648 = to_ov5648(client);
	int retval;

	switch (ctrl->id) {
	case V4L2_CID_CAMERA_FRAME_RATE:
		ctrl->value = ov5648->framerate;
		break;
	case V4L2_CID_CAMERA_FOCUS_MODE:
		ctrl->value = ov5648->focus_mode;
		break;
	case V4L2_CID_GAIN:
		ov5648_get_gain(client, &retval, NULL);
		ctrl->value = retval;
		break;
	case V4L2_CID_EXPOSURE:
		ov5648_get_exposure(client, &retval, NULL);
		ctrl->value = retval;
		break;
	case V4L2_CID_CAMERA_LENS_POSITION:
		ov5648_lens_get_position(client, &ov5648->current_position,
			&ov5648->time_to_destination);
		ctrl->value = ov5648->current_position;
		break;
	case V4L2_CID_CAMERA_FLASH_MODE:
		ctrl->value = ov5648->flashmode;
		break;
	case V4L2_CID_FLASH_INTENSITY:
		ctrl->value = ov5648->flash_intensity;
		break;
	case V4L2_CID_FLASH_TIMEOUT:
		ctrl->value = ov5648->flash_timeout;
		break;

	}
	return 0;
}

static int ov5648_s_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ov5648 *ov5648 = to_ov5648(client);
	int ret = 0;

	switch (ctrl->id) {
	case V4L2_CID_CAMERA_FRAME_RATE:

		if (ctrl->value > FRAME_RATE_30)
			return -EINVAL;

		ov5648->framerate = ctrl->value;

		switch (ov5648->framerate) {
		case FRAME_RATE_5:
			ov5648->framerate_lo = F24p8(5.0);
			ov5648->framerate_hi = F24p8(5.0);
			break;
		case FRAME_RATE_7:
			ov5648->framerate_lo = F24p8(7.0);
			ov5648->framerate_hi = F24p8(7.0);
			break;
		case FRAME_RATE_10:
			ov5648->framerate_lo = F24p8(10.0);
			ov5648->framerate_hi = F24p8(10.0);
			break;
		case FRAME_RATE_15:
			ov5648->framerate_lo = F24p8(15.0);
			ov5648->framerate_hi = F24p8(15.0);
			break;
		case FRAME_RATE_20:
			ov5648->framerate_lo = F24p8(20.0);
			ov5648->framerate_hi = F24p8(20.0);
			break;
		case FRAME_RATE_25:
			ov5648->framerate_lo = F24p8(25.0);
			ov5648->framerate_hi = F24p8(25.0);
			break;
		case FRAME_RATE_30:
			ov5648->framerate_lo = F24p8(30.0);
			ov5648->framerate_hi = F24p8(30.0);
			break;
		case FRAME_RATE_AUTO:
		default:
			ov5648->framerate_lo = F24p8(1.0);
			ov5648->framerate_hi = F24p8(30.0);
			break;
		}
		ov5648_set_framerate_lo(client, ov5648->framerate_lo);
		ov5648_set_framerate_hi(client, ov5648->framerate_hi);
		ov5648_set_exposure(client, ov5648->exposure_current);
		ov5648_set_gain(client, ov5648->gain_current);

		if (ret)
			return ret;
		break;

	case V4L2_CID_CAMERA_FOCUS_MODE:

		if (ctrl->value > FOCUS_MODE_MANUAL)
			return -EINVAL;

		ov5648->focus_mode = ctrl->value;

		/*
		 * Donot start the AF cycle here
		 * AF Start will be called later in
		 * V4L2_CID_CAMERA_SET_AUTO_FOCUS only for auto, macro mode
		 * it wont be called for infinity.
		 * Donot worry about resolution change for now.
		 * From userspace we set the resolution first
		 * and then set the focus mode.
		 */
		switch (ov5648->focus_mode) {
		case FOCUS_MODE_MACRO:
			/*
			 * set the table for macro mode
			 */
			break;
		case FOCUS_MODE_INFINITY:
			/*
			 * set the table for infinity
			 */
			ret = 0;
			break;
		case FOCUS_MODE_MANUAL:
			/*
			 * set the table for manual
			 */
			ret = 0;
			break;

		default:
			ret = 0;
			break;
		}

		if (ret)
			return ret;
		break;

	case V4L2_CID_CAMERA_SET_AUTO_FOCUS:

		if (ctrl->value > AUTO_FOCUS_ON)
			return -EINVAL;

		if (ret)
			return ret;
		break;
	case V4L2_CID_GAIN:
		if ((unsigned int)(ctrl->value) > OV5648_GAIN_MAX) {
			pr_debug("V4L2_CID_GAIN invalid gain=%u max=%u",
				ctrl->value, OV5648_GAIN_MAX);
			return -EINVAL;
		}
		ov5648_set_gain(client, ctrl->value);
		break;

	case V4L2_CID_EXPOSURE:
		if (ctrl->value > OV5648_EXP_MAX)
			return -EINVAL;
		ov5648_set_exposure(client, ctrl->value);
		break;

	case V4L2_CID_CAMERA_LENS_POSITION:
		if (ctrl->value > OV5648_LENS_MAX)
			return -EINVAL;
		ov5648_lens_set_position(client, ctrl->value);
		break;
	case V4L2_CID_CAMERA_FLASH_MODE:
		set_flash_mode(client, ctrl->value);
		break;
	case V4L2_CID_FLASH_INTENSITY:
		ov5648->flash_intensity = ctrl->value;
		break;
	case V4L2_CID_FLASH_TIMEOUT:
		ov5648->flash_timeout = ctrl->value;
		break;

	#if 0
	case V4L2_CID_CAM_APERTURE:
		if (ctrl->value > OV5648_APERTURE_MAX)
			return -EINVAL;
		ov5648_lens_set_aperture(client, ctrl->value);
		break;
	#endif
	}

	return ret;
}

int set_flash_mode(struct i2c_client *client, int mode)
{
	struct ov5648 *ov5648 = to_ov5648(client);
	if (ov5648->flashmode == mode)
		return 0;
#ifdef CONFIG_VIDEO_AS3643
	if ((mode == FLASH_MODE_OFF) || (mode == FLASH_MODE_TORCH_OFF)) {
		if (ov5648->flashmode != FLASH_MODE_OFF) {
			ov5648_reg_write(client, 0x3B00, 0x00);
			as3643_clear_all();
			as3643_gpio_toggle(0);
		}
	} else if (mode == FLASH_MODE_TORCH_ON) {
		as3643_gpio_toggle(1);
		usleep_range(25, 30);
		as3643_set_torch_flash(0x80);
	} else if (mode == FLASH_MODE_ON) {
		ov5648_reg_write(client, 0x3B00, 0x83);
		as3643_gpio_toggle(1);
		usleep_range(25, 30);
		/* FIXME: timeout should be vts*line_length/1000+constant? */
		as3643_set_ind_led(ov5648->flash_intensity / 5,
				ov5648->flash_timeout);
	} else if (mode == FLASH_MODE_AUTO) {
		as3643_gpio_toggle(1);
		usleep_range(25, 30);
		as3643_set_ind_led(0x80, 30000);
		ov5648_reg_write(client, 0x3B00, 0x83);
	/* Not yet implemented */
	} else {
		return -EINVAL;
	}
	ov5648->flashmode = mode;
#endif

#if defined(CONFIG_MACH_JAVA_C_LC1)
		if ((mode == FLASH_MODE_OFF)
			|| (mode == FLASH_MODE_TORCH_OFF)) {
			gpio_set_value(TORCH_EN, 0);
			gpio_set_value(FLASH_EN, 0);
		} else if (mode == FLASH_MODE_TORCH_ON) {
			gpio_set_value(TORCH_EN, 1);
			gpio_set_value(FLASH_EN, 0);
		} else if (mode == FLASH_MODE_ON) {
			ov5648->flash_timeout =
				2 * (ov5648->vts * ov5648->line_length)/1000;
			timer.data = (unsigned long) msg;
			timer.expires = jiffies
				+ (ov5648->flash_timeout*HZ)/1000000;
			timer.function = print_func;
			add_timer(&timer);
			pr_debug("flash_timeout=%d", ov5648->flash_timeout);
			gpio_set_value(TORCH_EN, 1);
			gpio_set_value(FLASH_EN, 1);
		} else if (mode == FLASH_MODE_AUTO) {
			ov5648->flash_timeout =
				2 * (ov5648->vts * ov5648->line_length)/1000;
			timer.data = (unsigned long) msg;
			timer.expires =
				jiffies + (ov5648->flash_timeout*HZ)/1000000;
			timer.function = print_func;
			add_timer(&timer);
			gpio_set_value(TORCH_EN, 1);
			gpio_set_value(FLASH_EN, 1);
		} else {
			return -EINVAL;
		}
	ov5648->flashmode = mode;
#endif

	return 0;
}

static long ov5648_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
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
			p->packet_size = 0;
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
static int ov5648_g_register(struct v4l2_subdev *sd,
			     struct v4l2_dbg_register *reg)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	if (reg->match.type != V4L2_CHIP_MATCH_I2C_ADDR || reg->size > 2)
		return -EINVAL;

	if (reg->match.addr != client->addr)
		return -ENODEV;

	reg->size = 2;
	if (ov5648_reg_read(client, reg->reg, &reg->val))
		return -EIO return 0;
}

static int ov5648_s_register(struct v4l2_subdev *sd,
			     struct v4l2_dbg_register *reg)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	if (reg->match.type != V4L2_CHIP_MATCH_I2C_ADDR || reg->size > 2)
		return -EINVAL;

	if (reg->match.addr != client->addr)
		return -ENODEV;

	if (ov5648_reg_write(client, reg->reg, reg->val))
		return -EIO;

	return 0;
}
#endif

static int ov5648_s_power(struct v4l2_subdev *sd, int on)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ov5648 *ov5648 = to_ov5648(client);

	if (0 == on) {
		ov5648->mode_idx = OV5648_MODE_MAX;
		ov5648->calibrated = 0;
	}
	return 0;
}

static struct soc_camera_ops ov5648_ops = {
	.set_bus_param   = ov5648_set_bus_param,
	.query_bus_param = ov5648_query_bus_param,
	.enum_input      = ov5648_enum_input,
	.controls        = ov5648_controls,
	.num_controls    = ARRAY_SIZE(ov5648_controls),
};

static int ov5648_init(struct i2c_client *client)
{
	struct ov5648 *ov5648 = to_ov5648(client);
	int ret = 0;

	/* reset */
	ov5648_reg_write(client, 0x0103, 0x01);

	/* set initial mode */
	ov5648->mode_idx = OV5648_MODE_MAX;
	ov5648->state = OV5648_STATE_STOP;

	/* default settings */
	ov5648->framerate         = FRAME_RATE_AUTO;
	ov5648->focus_mode        = FOCUS_MODE_AUTO;
	ov5648->gain_current      = DEFAULT_GAIN;

	memset(&ov5648->exp_read_buf, 0, sizeof(ov5648->exp_read_buf));
	memset(&ov5648->gain_read_buf, 0, sizeof(ov5648->gain_read_buf));
	memset(&ov5648->lens_read_buf, 0, sizeof(ov5648->lens_read_buf));
	/*
	 *  Exposure should be DEFAULT_EXPO * line_length / 1000
	 *  Since we don't have line_length yet, just estimate
	 */
#if defined(CONFIG_MACH_JAVA_C_LC1)
	init_timer(&timer);
#endif

	ov5648->exposure_current  = DEFAULT_EXPO * 22;
	ov5648->aecpos_delay      = 1;
	ov5648->lenspos_delay     = 0;
	ov5648->flashmode         = FLASH_MODE_OFF;
	ov5648->flash_intensity   = OV5648_FLASH_INTENSITY_DEFAULT;
	ov5648->flash_timeout     = OV5648_FLASH_TIMEOUT_DEFAULT;
	dev_dbg(&client->dev, "Sensor initialized\n");
	return ret;
}

/*
 * Interface active, can use i2c. If it fails, it can indeed mean, that
 * this wasn't our capture interface, so, we wait for the right one
 */
static int ov5648_video_probe(struct soc_camera_device *icd,
			      struct i2c_client *client)
{
	unsigned long flags;
	int ret = 0;
	u8 revision = 0, id_high, id_low;
	u16 id;

	/*
	 * We must have a parent by now. And it cannot be a wrong one.
	 * So this entire test is completely redundant.
	 */
	if (!icd->dev.parent ||
	    to_soc_camera_host(icd->dev.parent)->nr != icd->iface)
		return -ENODEV;

	ret = ov5648_reg_read(client, 0x302A, &revision);
	if (ret) {
		dev_err(&client->dev, "Failure to detect OV5648 chip\n");
		goto out;
	}

	revision &= 0xF;

	flags = SOCAM_DATAWIDTH_8;

	/* Read sensor Model ID */
	ret = ov5648_reg_read(client, OV5648_REG_CHIP_ID_HIGH, &id_high);
	if (ret < 0)
		return ret;

	id = id_high << 8;

	ret = ov5648_reg_read(client, OV5648_REG_CHIP_ID_LOW, &id_low);
	if (ret < 0)
		return ret;

	id |= id_low;

	if (id != 0x5648)
		return -ENODEV;

	dev_info(&client->dev, "Detected a OV5648 chip 0x%04x, revision %x\n",
		 id, revision);

out:
	return ret;
}

static void ov5648_video_remove(struct soc_camera_device *icd)
{
	dev_dbg(&icd->dev, "Video removed: %p, %p\n",
		icd->dev.parent, icd->vdev);
}

static struct v4l2_subdev_core_ops ov5648_subdev_core_ops = {
	.g_chip_ident = ov5648_g_chip_ident,
	.g_ctrl = ov5648_g_ctrl,
	.s_ctrl = ov5648_s_ctrl,
	.ioctl = ov5648_ioctl,
#ifdef CONFIG_VIDEO_ADV_DEBUG
	.g_register = ov5648_g_register,
	.s_register = ov5648_s_register,
#endif
	.s_power = ov5648_s_power,
};

static int ov5648_enum_fmt(struct v4l2_subdev *sd, unsigned int index,
			   enum v4l2_mbus_pixelcode *code)
{
	if (index >= ARRAY_SIZE(ov5648_fmts))
		return -EINVAL;

	*code = ov5648_fmts[index].code;
	return 0;
}

static int ov5648_enum_framesizes(struct v4l2_subdev *sd,
				  struct v4l2_frmsizeenum *fsize)
{
	if (fsize->index >= OV5648_MODE_MAX)
		return -EINVAL;

	fsize->type = V4L2_FRMSIZE_TYPE_DISCRETE;
	fsize->pixel_format = V4L2_PIX_FMT_SRGGB10;
	fsize->discrete.width = ov5648_mode[fsize->index].width;
	fsize->discrete.height = ov5648_mode[fsize->index].height;

	return 0;
}

/* we only support fixed frame rate */
static int ov5648_enum_frameintervals(struct v4l2_subdev *sd,
				      struct v4l2_frmivalenum *interval)
{
	int size;

	if (interval->index >= 1)
		return -EINVAL;

	interval->type = V4L2_FRMIVAL_TYPE_DISCRETE;

	size = ov5648_find_framesize(interval->width, interval->height);

	switch (size) {
	case OV5648_MODE_2592x1944P15:
		interval->discrete.numerator = 1;
		interval->discrete.denominator = 15;
		break;
	case OV5648_MODE_1280x720P30:
	case OV5648_MODE_1280x960P30:
#ifdef CONFIG_ARCH_JAVA
	case OV5648_MODE_1920x1080P30:
#endif
	default:
		interval->discrete.numerator = 1;
		interval->discrete.denominator = 30;
		break;
	}
	return 0;
}

static int ov5648_g_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *param)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ov5648 *ov5648 = to_ov5648(client);
	struct v4l2_captureparm *cparm;

	if (param->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;

	cparm = &param->parm.capture;

	memset(param, 0, sizeof(*param));
	param->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	cparm->capability = V4L2_CAP_TIMEPERFRAME;

	switch (ov5648->mode_idx) {
	case OV5648_MODE_2592x1944P15:
		cparm->timeperframe.numerator = 1;
		cparm->timeperframe.denominator = 15;
		break;
	case OV5648_MODE_1280x720P30:
	case OV5648_MODE_1280x960P30:
#ifdef CONFIG_ARCH_JAVA
	case OV5648_MODE_1920x1080P30:
#endif
	default:
		cparm->timeperframe.numerator = 1;
		cparm->timeperframe.denominator = 30;
		break;
	}

	return 0;
}
static int ov5648_s_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *param)
{
	/*
	 * FIXME: This just enforces the hardcoded framerates until this is
	 *flexible enough.
	 */
	return ov5648_g_parm(sd, param);
}

static struct v4l2_subdev_video_ops ov5648_subdev_video_ops = {
	.s_stream = ov5648_s_stream,
	.s_mbus_fmt = ov5648_s_fmt,
	.g_mbus_fmt = ov5648_g_fmt,
	.try_mbus_fmt = ov5648_try_fmt,
	.enum_mbus_fmt = ov5648_enum_fmt,
	.enum_mbus_fsizes = ov5648_enum_framesizes,
	.enum_framesizes = ov5648_enum_framesizes,
	.enum_frameintervals = ov5648_enum_frameintervals,
	.g_parm = ov5648_g_parm,
	.s_parm = ov5648_s_parm,
};
static int ov5648_g_skip_frames(struct v4l2_subdev *sd, u32 *frames)
{
	/* Quantity of initial bad frames to skip. Revisit. */
	*frames = 1;

	return 0;
}

static int ov5648_g_interface_parms(struct v4l2_subdev *sd,
				    struct v4l2_subdev_sensor_interface_parms
				    *parms)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ov5648 *ov5648 = to_ov5648(client);

	if (!parms)
		return -EINVAL;
	parms->if_type = ov5648->plat_parms->if_type;
	parms->if_mode = ov5648->plat_parms->if_mode;
	parms->parms = ov5648->plat_parms->parms;

	/* set the hs term time */
	parms->parms.serial.hs_term_time = 0;
	parms->parms.serial.hs_settle_time = 2;

	return 0;
}

static struct v4l2_subdev_sensor_ops ov5648_subdev_sensor_ops = {
	.g_skip_frames = ov5648_g_skip_frames,
	.g_interface_parms = ov5648_g_interface_parms,
};

static struct v4l2_subdev_ops ov5648_subdev_ops = {
	.core = &ov5648_subdev_core_ops,
	.video = &ov5648_subdev_video_ops,
	.sensor = &ov5648_subdev_sensor_ops,
};

#ifdef CONFIG_VIDEO_A3907
#define A3907_I2C_ADDR 0x18
static struct i2c_board_info a3907_i2c_board_info = {
	 I2C_BOARD_INFO("a3907", (A3907_I2C_ADDR >> 1))
};
static struct i2c_client *a3907_i2c_client;
static struct i2c_adapter *a3907_i2c_adap;
#endif

#ifdef CONFIG_VIDEO_DW9714
#define DW9714_I2C_ADDR 0x18
static struct i2c_board_info dw9714_i2c_board_info = {
	 I2C_BOARD_INFO("dw9714", (DW9714_I2C_ADDR >> 1))
};
static struct i2c_client *dw9714_i2c_client;
static struct i2c_adapter *dw9714_i2c_adap;
#endif

static int ov5648_probe(struct i2c_client *client,
			const struct i2c_device_id *did)
{
	struct ov5648 *ov5648;
	struct soc_camera_device *icd = client->dev.platform_data;
	struct soc_camera_link *icl;
	int ret;

	if (!icd) {
		dev_err(&client->dev, "OV5648: missing soc-camera data!\n");
		return -EINVAL;
	}

	icl = to_soc_camera_link(icd);
	if (!icl) {
		dev_err(&client->dev, "OV5648 driver needs platform data\n");
		return -EINVAL;
	}

	if (!icl->priv) {
		dev_err(&client->dev,
			"OV5648 driver needs i/f platform data\n");
		return -EINVAL;
	}

	ov5648 = kzalloc(sizeof(struct ov5648), GFP_KERNEL);
	if (!ov5648)
		return -ENOMEM;

	v4l2_i2c_subdev_init(&ov5648->subdev, client, &ov5648_subdev_ops);

	/* Second stage probe - when a capture adapter is there */
	icd->ops = &ov5648_ops;

	ov5648->mode_idx = OV5648_MODE_MAX;
	ov5648->i_fmt = 0;	/* First format in the list */
	ov5648->plat_parms = icl->priv;
	ov5648->icd = icd;

	ret = ov5648_video_probe(icd, client);
	if (ret) {
		icd->ops = NULL;
		kfree(ov5648);
		return ret;
	}

	/* init the sensor here */
	ret = ov5648_init(client);
	if (ret) {
		dev_err(&client->dev, "Failed to initialize sensor\n");
		ret = -EINVAL;
	}

#ifdef CONFIG_VIDEO_A3907
		pr_debug("A3907 i2c start");
	a3907_i2c_adap = i2c_get_adapter(0);
	if (!a3907_i2c_adap)
		pr_debug("A3907 i2c_get_adapter(0) FAILED");
	if (a3907_i2c_adap) {
		a3907_i2c_client = i2c_new_device(a3907_i2c_adap,
				&a3907_i2c_board_info);
		i2c_put_adapter(a3907_i2c_adap);
		pr_debug("A3907 i2c_get_adapter(0) OK");
	}
#endif

#ifdef CONFIG_VIDEO_DW9714
		pr_debug("DW9714 i2c start");
	dw9714_i2c_adap = i2c_get_adapter(0);
	if (!dw9714_i2c_adap)
		pr_debug("DW9714 i2c_get_adapter(0) FAILED");
	if (dw9714_i2c_adap) {
		dw9714_i2c_client = i2c_new_device(dw9714_i2c_adap,
				&dw9714_i2c_board_info);
		i2c_put_adapter(dw9714_i2c_adap);
		pr_debug("DW9714 i2c_get_adapter(0) OK");
	}
#endif

	return ret;
}

static int ov5648_remove(struct i2c_client *client)
{
	struct ov5648 *ov5648 = to_ov5648(client);
	struct soc_camera_device *icd = client->dev.platform_data;

	pr_debug(" remove");
#ifdef CONFIG_VIDEO_A3907
	pr_debug("A3907 i2c_unregister_device");
	if (a3907_i2c_client)
		i2c_unregister_device(a3907_i2c_client);
#endif

#ifdef CONFIG_VIDEO_DW9714
	pr_debug("DW9714 i2c_unregister_device");
	if (dw9714_i2c_client)
		i2c_unregister_device(dw9714_i2c_client);
#endif

	icd->ops = NULL;
	ov5648_video_remove(icd);
	client->driver = NULL;
	kfree(ov5648);

	return 0;
}

static const struct i2c_device_id ov5648_id[] = {
	{"ov5648", 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, ov5648_id);

static struct i2c_driver ov5648_i2c_driver = {
	.driver = {
		   .name = "ov5648",
		   },
	.probe = ov5648_probe,
	.remove = ov5648_remove,
	.id_table = ov5648_id,
};

static int __init ov5648_mod_init(void)
{
	return i2c_add_driver(&ov5648_i2c_driver);
}

static void __exit ov5648_mod_exit(void)
{
	i2c_del_driver(&ov5648_i2c_driver);
}

module_init(ov5648_mod_init);
module_exit(ov5648_mod_exit);

MODULE_DESCRIPTION("OmniVision OV5648 Camera driver");
MODULE_AUTHOR("Broadcom Corporation");
MODULE_LICENSE("GPL v2");

