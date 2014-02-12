/*
 * OmniVision OV8825 sensor driver
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
#include "flash_lamp.h"
#include "ov8825.h"
#ifdef CONFIG_VIDEO_A3907
#include <media/a3907.h>
#endif

/*
static int is_snap_to_prev_expo;
static int is_snap_to_prev_gain;
*/

#define OV8825_DEBUG 0

/* OV5648 has only one fixed colorspace per pixelcode */
struct ov8825_datafmt {
	enum v4l2_mbus_pixelcode code;
	enum v4l2_colorspace colorspace;
};

static const struct ov8825_datafmt ov8825_fmts[] = {
	/*
	 Order important: first natively supported,
	 second supported with a GPIO extender
	 */
	{V4L2_MBUS_FMT_SRGGB10_1X10, V4L2_COLORSPACE_SRGB},
	{V4L2_MBUS_FMT_SBGGR10_1X10, V4L2_COLORSPACE_SRGB},
	{V4L2_MBUS_FMT_SGBRG10_1X10, V4L2_COLORSPACE_SRGB},
	{V4L2_MBUS_FMT_SGRBG10_1X10, V4L2_COLORSPACE_SRGB},
};

enum ov8825_mode {
	OV8825_MODE_1280x960P30	 = 0,
	OV8825_MODE_1920x1080P30,
	OV8825_MODE_3264x2448P15,
	OV8825_MODE_MAX,
};

enum ov8825_state {
	OV8825_STATE_STOP = 0,
	OV8825_STATE_STRM,
	OV8825_STATE_MAX,
};

enum bayer_order {
	BAYER_RGGB = 0,
	BAYER_GRBG,
	BAYER_GBRG,
	BAYER_BGGR,
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
	int			binning_factor;
};


struct sensor_mode ov8825_mode[OV8825_MODE_MAX + 1] = {
	{
		.name			= "1280x960_2lane_30Fps",
		.height			= 960,
		.width			= 1280,
		.hts			= 3516,
		.vts			= 1274,
		.vts_max		= 32767 - 6,
		.line_length_ns = 26458,
		.bayer			= BAYER_BGGR,
		.bpp			= 10,
		.fps			= F24p8(30.0),
		.binning_factor = 1,
	},
	{
		.name			= "1920x1080_2lane_30Fps",
		.height			= 1080,
		.width			= 1920,
		.hts			= 3568,
		.vts			= 2016,
		.vts_max		= 32767 - 6,
		.line_length_ns = 16468,
		.bayer			= BAYER_BGGR,
		.bpp			= 10,
		.fps			= F24p8(30.0),
		.binning_factor = 1,
	},
	{
		.name			= "3264x2448_2lane_15Fps",
		.height			= 2448,
		.width			= 3264,
		.hts			= 3584,
		.vts			= 2480,
		.vts_max		= 32767 - 6,
		.line_length_ns = 26970,
		.bayer			= BAYER_BGGR,
		.bpp			= 10,
		.fps			= F24p8(15.0),
		.binning_factor = 2,
	},
	{
		.name			= "STOPPED",
	}
};

/**
 *struct ov8825_otp - ov8825 OTP format
 *@module_integrator_id: 32-bit module integrator ID number
 *@lens_id: 32-bit lens ID number
 *@rg_ratio: 32-bit AWB R/G channel ratio
 *@bg_ratio: 32-bit AWB B/G channel ratio
 *@user_data: 64-bit OTP user data
 *@light_rg: 32-bit light source R/G ratio
 *@light_bg: 32-bit light source B/G ratio
 *
 * Define a structure for OV8825 OTP calibration data
 */
struct ov8825_otp {
	u8	flag;
	u8	module_integrator_id;
	u8	lens_id;
	u16 rg_ratio;
	u16 bg_ratio;
	u8	user_data[2];
	u16 light_rg;
	u16 light_bg;
};


#define EXP_DELAY_MAX 8
#define GAIN_DELAY_MAX 8
#define LENS_READ_DELAY 4
#define FLASH_DELAY_MAX 4
#define V4L2_FRAME_INFO_CACHE 4

struct ov8825 {
	struct v4l2_subdev subdev;
	struct v4l2_subdev_sensor_interface_parms *plat_parms;
	struct v4l2_ctrl_handler hdl;
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
	int exp_read_buf[EXP_DELAY_MAX];
	int gain_read_buf[GAIN_DELAY_MAX];
	int lens_read_buf[LENS_READ_DELAY];
	int lenspos_delay;

	enum v4l2_flash_led_mode flashmode;
	int flash_mode;
	int flash_intensity;
	int flash_timeout;
	int flash_delay;
	int flash_on;
	int flash_int_buf[FLASH_DELAY_MAX];
	int flash_mod_buf[FLASH_DELAY_MAX];
	struct flash_lamp_s lamp;

	struct ov8825_otp otp;
	int calibrated;

	int frame_info_size;
	struct v4l2_frame_info frame_info[V4L2_FRAME_INFO_CACHE];
	int fps, fcnt, fps_t0;

	int powerdown;
	int position;
	int dac_code;
	int timing_step_us;
	int flying_time;
	int flying_start_time;
	int requested_position;
};

/**
 *struct ov8825_reg - ov8825 register format
 *@reg: 16-bit offset to register
 *@val: 8/16/32-bit register value
 *@length: length of the register
 *
 * Define a structure for OV8825 register initialization values
 */
struct ov8825_reg {
	u16	reg;
	u8	val;
	u8	pad;
};

static const struct ov8825_reg ov8825_reginit[512] = {
		{0x0100, 0x00},
		{0x3000, 0x16},
		{0x3001, 0x00},
		{0x3002, 0x6c},
		{0x3003, 0xce},
		{0x3004, 0xd4},
		{0x3005, 0x00},
		{0x3006, 0x10},
		{0x3007, 0x3b},
		{0x300d, 0x00},
		{0x301f, 0x09},
		{0x3020, 0x01},
		{0x3010, 0x00},
		{0x3011, 0x01},
		{0x3012, 0x80},
		{0x3013, 0x39},
		{0x3018, 0x00},
		{0x3104, 0x20},
		{0x3106, 0x15},
		{0x3300, 0x00},
		{0x3500, 0x00},
		{0x3501, 0x4e},
		{0x3502, 0xa0},
		{0x3503, 0x07},
		{0x3509, 0x10},
		{0x350b, 0x1f},
		{0x3600, 0x06},
		{0x3601, 0x34},
		{0x3602, 0x42},
		{0x3603, 0x5c},
		{0x3604, 0x98},
		{0x3605, 0xf5},
		{0x3609, 0xb4},
		{0x360a, 0x7c},
		{0x360b, 0xc9},
		{0x360c, 0x0b},
		{0x3612, 0x00},
		{0x3613, 0x02},
		{0x3614, 0x0f},
		{0x3615, 0x00},
		{0x3616, 0x03},
		{0x3617, 0xa1},
		{0x3618, 0x0f},
		{0x3619, 0x00},
		{0x361a, 0x8a},
		{0x361b, 0x02},
		{0x361c, 0x07},
		{0x3700, 0x20},
		{0x3701, 0x44},
		{0x3702, 0x50},
		{0x3703, 0xcc},
		{0x3704, 0x19},
		{0x3705, 0x32},
		{0x3706, 0x4b},
		{0x3707, 0x63},
		{0x3708, 0x84},
		{0x3709, 0x40},
		{0x370a, 0x33},
		{0x370b, 0x01},
		{0x370c, 0x50},
		{0x370d, 0x00},
		{0x370e, 0x00},
		{0x3711, 0x0f},
		{0x3712, 0x9c},
		{0x3724, 0x01},
		{0x3725, 0x92},
		{0x3726, 0x01},
		{0x3727, 0xc7},
		{0x3800, 0x00},
		{0x3801, 0x00},
		{0x3802, 0x00},
		{0x3803, 0x00},
		{0x3804, 0x0c},
		{0x3805, 0xdf},
		{0x3806, 0x09},
		{0x3807, 0x9b},
		{0x3808, 0x06},
		{0x3809, 0x60},
		{0x380a, 0x04},
		{0x380b, 0xc8},
		{0x380c, 0x0d},
		{0x380d, 0xbc},
		{0x380e, 0x04},
		{0x380f, 0xf0},
		{0x3810, 0x00},
		{0x3811, 0x08},
		{0x3812, 0x00},
		{0x3813, 0x04},
		{0x3814, 0x31},
		{0x3815, 0x31},
		{0x3816, 0x02},
		{0x3817, 0x40},
		{0x3818, 0x00},
		{0x3819, 0x40},
		{0x3820, 0x87},
		{0x3821, 0x11},
		{0x3b1f, 0x00},
		{0x3d00, 0x00},
		{0x3d01, 0x00},
		{0x3d02, 0x00},
		{0x3d03, 0x00},
		{0x3d04, 0x00},
		{0x3d05, 0x00},
		{0x3d06, 0x00},
		{0x3d07, 0x00},
		{0x3d08, 0x00},
		{0x3d09, 0x00},
		{0x3d0a, 0x00},
		{0x3d0b, 0x00},
		{0x3d0c, 0x00},
		{0x3d0d, 0x00},
		{0x3d0e, 0x00},
		{0x3d0f, 0x00},
		{0x3d10, 0x00},
		{0x3d11, 0x00},
		{0x3d12, 0x00},
		{0x3d13, 0x00},
		{0x3d14, 0x00},
		{0x3d15, 0x00},
		{0x3d16, 0x00},
		{0x3d17, 0x00},
		{0x3d18, 0x00},
		{0x3d19, 0x00},
		{0x3d1a, 0x00},
		{0x3d1b, 0x00},
		{0x3d1c, 0x00},
		{0x3d1d, 0x00},
		{0x3d1e, 0x00},
		{0x3d1f, 0x00},
		{0x3d80, 0x00},
		{0x3d81, 0x00},
		{0x3d84, 0x00},
		{0x3f00, 0x00},
		{0x3f01, 0xfc},
		{0x3f05, 0x10},
		{0x3f06, 0x00},
		{0x3f07, 0x00},
		{0x4000, 0x29},
		{0x4001, 0x02},
		{0x4002, 0x45},
		{0x4003, 0x08},
		{0x4004, 0x04},
		{0x4005, 0x18},
		{0x404e, 0x37},
		{0x404f, 0x8f},
		{0x4300, 0xff},
		{0x4303, 0x00},
		{0x4304, 0x08},
		{0x4307, 0x00},
		{0x4600, 0x04},
		{0x4601, 0x00},
		{0x4602, 0x30},
		{0x4800, 0x04},
		{0x4801, 0x0f},
		{0x4837, 0x28},
		{0x4843, 0x02},
		{0x5000, 0x06},
		{0x5001, 0x00},
		{0x5002, 0x00},
		{0x5068, 0x00},
		{0x506a, 0x00},
		{0x501f, 0x00},
		{0x5780, 0xfc},
		{0x5c00, 0x80},
		{0x5c01, 0x00},
		{0x5c02, 0x00},
		{0x5c03, 0x00},
		{0x5c04, 0x00},
		{0x5c05, 0x00},
		{0x5c06, 0x00},
		{0x5c07, 0x80},
		{0x5c08, 0x10},
		{0x6700, 0x05},
		{0x6701, 0x19},
		{0x6702, 0xfd},
		{0x6703, 0xd7},
		{0x6704, 0xff},
		{0x6705, 0xff},
		{0x6800, 0x10},
		{0x6801, 0x02},
		{0x6802, 0x90},
		{0x6803, 0x10},
		{0x6804, 0x59},
		{0x6900, 0x60},
		{0x6901, 0x04},
		{0x5800, 0x0f},
		{0x5801, 0x0d},
		{0x5802, 0x09},
		{0x5803, 0x0a},
		{0x5804, 0x0d},
		{0x5805, 0x14},
		{0x5806, 0x0a},
		{0x5807, 0x04},
		{0x5808, 0x03},
		{0x5809, 0x03},
		{0x580a, 0x05},
		{0x580b, 0x0a},
		{0x580c, 0x05},
		{0x580d, 0x02},
		{0x580e, 0x00},
		{0x580f, 0x00},
		{0x5810, 0x03},
		{0x5811, 0x05},
		{0x5812, 0x09},
		{0x5813, 0x03},
		{0x5814, 0x01},
		{0x5815, 0x01},
		{0x5816, 0x04},
		{0x5817, 0x09},
		{0x5818, 0x09},
		{0x5819, 0x08},
		{0x581a, 0x06},
		{0x581b, 0x06},
		{0x581c, 0x08},
		{0x581d, 0x06},
		{0x581e, 0x33},
		{0x581f, 0x11},
		{0x5820, 0x0e},
		{0x5821, 0x0f},
		{0x5822, 0x11},
		{0x5823, 0x3f},
		{0x5824, 0x08},
		{0x5825, 0x46},
		{0x5826, 0x46},
		{0x5827, 0x46},
		{0x5828, 0x46},
		{0x5829, 0x46},
		{0x582a, 0x42},
		{0x582b, 0x42},
		{0x582c, 0x44},
		{0x582d, 0x46},
		{0x582e, 0x46},
		{0x582f, 0x60},
		{0x5830, 0x62},
		{0x5831, 0x42},
		{0x5832, 0x46},
		{0x5833, 0x46},
		{0x5834, 0x44},
		{0x5835, 0x44},
		{0x5836, 0x44},
		{0x5837, 0x48},
		{0x5838, 0x28},
		{0x5839, 0x46},
		{0x583a, 0x48},
		{0x583b, 0x68},
		{0x583c, 0x28},
		{0x583d, 0xae},
		{0x5842, 0x00},
		{0x5843, 0xef},
		{0x5844, 0x01},
		{0x5845, 0x3f},
		{0x5846, 0x01},
		{0x5847, 0x3f},
		{0x5848, 0x00},
		{0x5849, 0xd5},
		{0x3400, 0x04},
		{0x3401, 0x00},
		{0x3402, 0x04},
		{0x3403, 0x00},
		{0x3404, 0x04},
		{0x3405, 0x00},
		{0x3406, 0x01},
		{0x5001, 0x01},
		{0x5000, 0x86},

		{0x301a, 0x71},
		{0x301c, 0xf4},
		{0x0100, 0x01},
		{0xFFFF, 0x00}
};

static const struct ov8825_reg ov8825_regdif[OV8825_MODE_MAX][256] = {
	{
		{0x0100, 0x00},
		{0x3003, 0xcc},
		{0x3004, 0xd8},
		{0x3005, 0x00},
		{0x3006, 0x10},
		{0x3007, 0x49},
		{0x3020, 0x01},
		{0x3501, 0x4e},
		{0x3502, 0xa0},
		{0x3509, 0x10},
		{0x3618, 0x0f},
		{0x361a, 0x8a},
		{0x361b, 0x02},
		{0x3700, 0x20},
		{0x3702, 0x50},
		{0x3703, 0xcc},
		{0x3704, 0x19},
		{0x3705, 0x32},
		{0x3706, 0x4b},
		{0x3708, 0x84},
		{0x3709, 0x40},
		{0x370a, 0x33},
		{0x3711, 0x0f},
		{0x3712, 0x9c},
		{0x3724, 0x01},
		{0x3725, 0x92},
		{0x3726, 0x01},
		{0x3727, 0xc7},
		{0x3802, 0x00},
		{0x3803, 0x00},
		{0x3806, 0x09},
		{0x3807, 0x9b},
		{0x3808, 0x05},
		{0x3809, 0x00},
		{0x380a, 0x03},
		{0x380b, 0xc0},
		{0x380c, 0x0d},
		{0x380d, 0xbc},
		{0x380e, 0x04},
		{0x380f, 0xfa},
		{0x3811, 0x08},
		{0x3813, 0x04},
		{0x3814, 0x31},
		{0x3815, 0x31},
		{0x3820, 0x87},
		{0x3821, 0x11},
		{0x3f00, 0x00},
		{0x4005, 0x18},
		{0x4601, 0x00},
		{0x4602, 0x30},
		{0x4837, 0x16},
		{0x5068, 0x5a},
		{0x506a, 0x5a},
		{0x0100, 0x01},
		{0x301c, 0xf0},
		{0x301a, 0x70},
		{0xFFFF, 0x00}
	},

	{
		{0x0100, 0x00},
		{0x3003, 0xcc},
		{0x3004, 0xd8},
		{0x3006, 0x00},
		{0x3007, 0x39},
		{0x3020, 0x01},
		{0x3501, 0x74},
		{0x3502, 0x60},
		{0x3700, 0x20},
		{0x3702, 0x50},
		{0x3703, 0xcc},
		{0x3704, 0x19},
		{0x3705, 0x32},
		{0x3706, 0x4b},
		{0x3708, 0x84},
		{0x3709, 0x40},
		{0x370a, 0x31},
		{0x3711, 0x0f},
		{0x3712, 0x9c},
		{0x3724, 0x01},
		{0x3725, 0x92},
		{0x3726, 0x01},
		{0x3727, 0xc7},
		{0x3800, 0x00},
		{0x3801, 0x00},
		{0x3804, 0x0c},
		{0x3805, 0xdf},
		{0x3802, 0x01},
		{0x3803, 0x30},
		{0x3806, 0x08},
		{0x3807, 0x67},
		{0x3808, 0x07},
		{0x3809, 0x80},
		{0x380a, 0x04},
		{0x380b, 0x38},
		{0x380c, 0x0d},
		{0x380d, 0xf0},
		{0x380e, 0x07},
		{0x380f, 0xe0},
		{0x3811, 0x10},
		{0x3813, 0x06},
		{0x3814, 0x11},
		{0x3815, 0x11},
		{0x3820, 0x86},
		{0x3821, 0x10},
		{0x3f00, 0x02},
		{0x4005, 0x18},
		{0x4601, 0x01},
		{0x4602, 0x00},
		{0x4837, 0x1e},
		{0x5068, 0x53},
		{0x506a, 0x53},
		{0x0100, 0x01},
		{0x301c, 0xf0},
		{0x301a, 0x70},
		{0xFFFF, 0x00}
	},
	{
		{0x0100, 0x00},
		{0x3003, 0xcc},
		{0x3004, 0xd8},
		{0x3005, 0x00},
		{0x3006, 0x10},
		{0x3007, 0x49},
		{0x3020, 0x81},
		{0x3501, 0x9a},
		{0x3502, 0xa0},
		{0x3509, 0x10},
		{0x3618, 0x0f},
		{0x361a, 0x8a},
		{0x361b, 0x02},
		{0x3700, 0x10},
		{0x3702, 0x28},
		{0x3703, 0x6c},
		{0x3704, 0x40},
		{0x3705, 0x19},
		{0x3706, 0x27},
		{0x3708, 0x48},
		{0x3709, 0x20},
		{0x370a, 0x31},
		{0x3711, 0x07},
		{0x3712, 0x4e},
		{0x3724, 0x00},
		{0x3725, 0xd4},
		{0x3726, 0x00},
		{0x3727, 0xf0},
		{0x3802, 0x00},
		{0x3803, 0x00},
		{0x3806, 0x09},
		{0x3807, 0x9b},
		{0x3808, 0x0c},
		{0x3809, 0xc0},
		{0x380a, 0x09},
		{0x380b, 0x90},
		{0x380c, 0x0e},
		{0x380d, 0x00},
		{0x380e, 0x09},
		{0x380f, 0xb0},
		{0x3811, 0x10},
		{0x3813, 0x06},
		{0x3814, 0x11},
		{0x3815, 0x11},
		{0x3820, 0x86},
		{0x3821, 0x10},
		{0x3f00, 0x02},
		{0x4005, 0x1a},
		{0x4601, 0x00},
		{0x4602, 0x20},
		{0x4837, 0x16},
		{0x5068, 0x00},
		{0x506a, 0x00},
		{0x0100, 0x01},
		{0x301c, 0xf0},
		{0x301a, 0x70},
		{0xFFFF, 0x00}
	},
};

static const struct ov8825_reg ov8825_reg_state[OV8825_STATE_MAX][3] = {
	{ /* to power down */
	{0x0100, 0x00},	       /* disable streaming  */
	{0x3018, 0x10},        /* disable mipi */
	{0xFFFF, 0x00}
	},
	{ /* to streaming */
	{0x3018, 0x00},         /* enable mipi */
	{0x0100, 0x01},		/* enable streaming */
	{0xFFFF, 0x00}
	},
};

static int ov8825_set_flash_mode(struct i2c_client *client, int mode);
static int ov8825_set_mode(struct i2c_client *client, int new_mode_idx);
static int ov8825_set_state(struct i2c_client *client, int new_state);
static int ov8825_init(struct i2c_client *client);

/*
 * Find a data format by a pixel code in an array
 */
static int ov8825_find_datafmt(enum v4l2_mbus_pixelcode code)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(ov8825_fmts); i++)
		if (ov8825_fmts[i].code == code)
			break;

	/* If not found, select latest */
	if (i >= ARRAY_SIZE(ov8825_fmts))
		i = ARRAY_SIZE(ov8825_fmts) - 1;

	return i;
}

/* Find a frame size in an array */
static int ov8825_find_framesize(u32 width, u32 height)
{
	int i;

	for (i = 0; i < OV8825_MODE_MAX; i++) {
		if ((ov8825_mode[i].width >= width) &&
			(ov8825_mode[i].height >= height))
			break;
	}

	/* If not found, select biggest */
	if (i >= OV8825_MODE_MAX)
		i = OV8825_MODE_MAX - 1;

	return i;
}

static struct ov8825 *to_ov8825(const struct i2c_client *client)
{
	return container_of(i2c_get_clientdata(client), struct ov8825, subdev);
}

/**
 *ov8825_reg_read - Read a value from a register in an ov8825 sensor device
 *@client: i2c driver client structure
 *@reg: register address / offset
 *@val: stores the value that gets read
 *
 * Read a value from a register in an ov8825 sensor device.
 * The value is returned in 'val'.
 * Returns zero if successful, or non-zero otherwise.
 */
static int ov8825_reg_read(struct i2c_client *client, u16 reg, u8 *val)
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
 * Write a value to a register in ov8825 sensor device.
 *@client: i2c driver client structure.
 *@reg: Address of the register to read value from.
 *@val: Value to be written to a specific register.
 * Returns zero if successful, or non-zero otherwise.
 */
static int ov8825_reg_write(struct i2c_client *client, u16 reg, u8 val)
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

static int ov8825_reg_read_multi(struct i2c_client *client,
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

static int ov8825_s_ctrl(struct v4l2_ctrl *);
static int ov8825_g_ctrl(struct v4l2_ctrl *);

static struct v4l2_ctrl_ops ov8825_ctrl_ops = {
	.s_ctrl = ov8825_s_ctrl,
	.g_volatile_ctrl = ov8825_g_ctrl,
};

static const struct v4l2_ctrl_config ov8825_controls[] = {
	{
		.ops = &ov8825_ctrl_ops,
		.id = V4L2_CID_CAMERA_FRAME_RATE,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.name = "Framerate control",
		.min = FRAME_RATE_AUTO,
		.max = (1 << FRAME_RATE_AUTO | 1 << FRAME_RATE_5 |
				1 << FRAME_RATE_10 | 1 << FRAME_RATE_15 |
				1 << FRAME_RATE_25 | 1 << FRAME_RATE_30),
		.step = 1,
		.def = FRAME_RATE_AUTO,
	},
	{
		.ops = &ov8825_ctrl_ops,
		.id = V4L2_CID_CAMERA_FOCUS_MODE,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.name = "Focus Modes",
		.min = FOCUS_MODE_AUTO,
		.max = (1 << FOCUS_MODE_AUTO |
				1 << FOCUS_MODE_MACRO |
				1 << FOCUS_MODE_INFINITY |
				1 << FOCUS_MODE_MANUAL),
		.step = 1,
		.def = FOCUS_MODE_AUTO,
	},
	{
		.ops = &ov8825_ctrl_ops,
		.id = V4L2_CID_GAIN,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.name = "Gain",
		.min = OV8825_GAIN_MIN,
		.max = OV8825_GAIN_MAX,
		.step = OV8825_GAIN_STEP,
		.def = DEFAULT_GAIN,
	},
	{
		.ops = &ov8825_ctrl_ops,
		.id = V4L2_CID_EXPOSURE,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.name = "Exposure",
		.min = OV8825_EXP_MIN,
		.max = OV8825_EXP_MAX,
		.step = OV8825_EXP_STEP,
		.def = DEFAULT_EXPO,
	},
	{
		.ops = &ov8825_ctrl_ops,
		.id = V4L2_CID_CAMERA_LENS_POSITION,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.name = "Lens Position",
		.min = OV8825_LENS_MIN,
		.max = OV8825_LENS_MAX,
		.step = OV8825_LENS_STEP,
		.def = DEFAULT_LENS_POS,
	},
	{
		.ops = &ov8825_ctrl_ops,
		.id = V4L2_CID_CAM_MOUNTING,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.name = "Sensor mounting",
		.min = 0,
		.max = 0x10 | 0x20 | 0x40, /* (1 << V4L2_IN_ST_HFLIP |
						1 << V4L2_IN_ST_VFLIP |
						1 << V4L2_IN_ST_BACK), */
		.step = 1,
		.def = 0,
		.flags = V4L2_CTRL_FLAG_VOLATILE,
	},
};

/**
 * Initialize a list of ov8825 registers.
 * The list of registers is terminated by the pair of values
 *@client: i2c driver client structure.
 *@reglist[]: List of address of the registers to write data.
 * Returns zero if successful, or non-zero otherwise.
 */
static int ov8825_reg_writes(struct i2c_client *client,
				 const struct ov8825_reg reglist[])
{
	int err = 0, i;

	for (i = 0; reglist[i].reg != 0xFFFF; i++) {
		if (reglist[i].reg == 0xFFFE) {
			msleep(reglist[i].val);
		} else {
			err |= ov8825_reg_write(client, reglist[i].reg,
						reglist[i].val);
		}
		if (err != 0)
			break;
	}
	return 0;
}

#ifdef OV8825_DEBUG
static int ov8825_reglist_compare(struct i2c_client *client,
			const struct ov8825_reg reglist[])
{
	int err = 0;
	u8 reg, i;

	for (i = 0; ((reglist[i].reg != 0xFFFF) && (err == 0)); i++) {
		if (reglist[i].reg != 0xFFFE) {
			err |= ov8825_reg_read(client, reglist[i].reg, &reg);
			pr_debug("ov8825_reglist_compare: [0x%04x]=0x%02x",
				 reglist[i].reg, reg);
		} else {
			msleep(reglist[i].val);
		}
	}
	return 0;
}
#endif

/**
 * Write an array of data to ov8825 sensor device.
 *@client: i2c driver client structure.
 *@reg: Address of the register to read value from.
 *@data: pointer to data to be written starting at specific register.
 *@size: # of data to be written starting at specific register.
 * Returns zero if successful, or non-zero otherwise.
 */
static int ov8825_array_write(struct i2c_client *client,
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

#if 0
static const struct ov8825_reg ov8825_otpbank_rdsel[2][8] = {
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
static int ov8825_otp_bank_read(struct i2c_client *client, int bank, u8 *buf)
{
	int ret, i;
	u8 val;

	pr_debug("ov8825_otp_bank_read: bank=%d\n", bank);
	ret = ov8825_reg_writes(client, ov8825_otpbank_rdsel[bank&1]);
	if (ret != 0) {
		dev_err(&client->dev, "OTP bank %d read select failed\n", bank);
		return -1;
	}
	for (i = 0; i < 0x10; i++) {
		ret |= ov8825_reg_read(client, 0x3d00+i, &val);
		buf[i] = val;
	}
	pr_debug("ov8825_otp_bank_read: OK bank=%d\n", bank);
	return 0;
}

/**
 * Print ov8825 OTP calibration data
 *@otp: Address of otp structure to print
 *@index: OTP index
 */
#if OV8825_DEBUG
static void ov8825_otp_print(struct ov8825_otp *otp, int index)
{
	pr_debug("ov8825: OTP index=%d\n", index);
	pr_debug("ov8825: integr_id = 0x%02X\n", otp->module_integrator_id);
	pr_debug("ov8825: lens_id	= 0x%02X\n", otp->lens_id);
	pr_debug("ov8825: rg_ratio	= 0x%04X\n", otp->rg_ratio);
	pr_debug("ov8825: bg_ratio	= 0x%04X\n", otp->bg_ratio);
	pr_debug("ov8825: light_rg	= 0x%04X\n", otp->light_rg);
	pr_debug("ov8825: light_bg	= 0x%04X\n", otp->light_bg);
	pr_debug("ov8825: user_data = [0x%02X 0x%02X]\n",
		 otp->user_data[0], otp->user_data[1]);
}

/**
 * Read and print ov8825 OTP R/B channel gains.
 *@client: i2c driver client structure.
 */
static void ov8825_rbgains_print(struct i2c_client *client)
{
	u8 val;
	u16 gainr, gaing, gainb;
	int i;
	ov8825_reg_read(client, 0x5001, &val);
	pr_debug("ov8825 0x%04X=0x%02X]\n", 0x5001, val);
	ov8825_reg_read(client, 0x5002, &val);
	pr_debug("ov8825 0x%04X=0x%02X]\n", 0x5002, val);
	ov8825_reg_read(client, 0x5180, &val);
	pr_debug("ov8825 0x%04X=0x%02X]\n", 0x5180, val);
	ov8825_reg_read(client, 0x5186, &val);
	gainr = val << 8;
	ov8825_reg_read(client, 0x5187, &val);
	gainr += val;
	ov8825_reg_read(client, 0x5188, &val);
	gaing = val << 8;
	ov8825_reg_read(client, 0x5189, &val);
	gaing += val;
	ov8825_reg_read(client, 0x518a, &val);
	gainb = val << 8;
	ov8825_reg_read(client, 0x518b, &val);
	gainb += val;
	pr_debug("ov8825 rb gains readback = [%04X %04X %04X]\n",
		 gainr, gaing, gainb);
}
#endif

/**
 * Convert OTP binary data to formatted calibration information
 *@otp: Address of otp structure to write values into
 *@buf: otp binary data
 * Returns zero if successful or non-zero otherwise.
 */
static void ov8825_buf2otp(struct ov8825_otp *otp, u8 *buf)
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
 * Read OTP calibration data at index from ov8825 sensor.
 *@client: i2c driver client structure.
 *@otp: Address of otp structure to read values into
 *@bank: otp index select
 * Returns zero if OTP is present and read is successful
 * or non-zero otherwise.
 */
static int ov8825_otp_index_read(struct i2c_client *client,
				 struct ov8825_otp *otp, int index)
{
	int ret = -1;
	u8 buf[32];

	pr_debug("ov8825_otp_index_read: index %d OTP read\n", index);
	memset(otp, 0, sizeof(*otp));
	switch (index) {
	case 0:
		ret = ov8825_otp_bank_read(client, 0, buf);
		if (ret < 0)
			break;
		ov8825_buf2otp(otp, buf+5);
		break;
	case 1:
		ret = ov8825_otp_bank_read(client, 0, buf);
			break;
		if (ret < 0)
			break;
		ov8825_buf2otp(otp, buf+0xe);
		break;
	case 2:
		ret = ov8825_otp_bank_read(client, 0, buf);
		if (ret < 0)
			break;
		ov8825_buf2otp(otp, buf+7);
		break;
	default:
		pr_debug("ov8825_otp_index_read: invalid OTP idx: %d\n",
			 index);
		ret = -1;
		break;
	}

	pr_debug("ov8825_otp_index_read: rg_ratio  = 0x%04X\n", otp->rg_ratio);
	pr_debug("ov8825_otp_index_read: bg_ratio  = 0x%04X\n", otp->bg_ratio);
#if OV8825_DEBUG
	ov8825_otp_print(otp, index);
#endif
	if (ret != 0) {
		pr_debug("ov8825_otp_index_read: index %d OTP read failed\n",
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
 * Find and read OTP calibration data from ov8825 sensor.
 *@client: i2c driver client structure.
 *@otp: Address of otp structure to read values into
 *@bank: otp index select
 * Returns zero if OTP is present and read is successful
 * or non-zero otherwise.
 */
static int ov8825_otp_read(struct i2c_client *client)
{
	int i, ret;
	struct ov8825 *ov8825 = to_ov8825(client);
	struct ov8825_otp *otp = &(ov8825->otp);

	for (i = 0; i < 3; i++) {
		ret = ov8825_otp_index_read(client, otp, i);
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
static int ov8825_rbgains_update(struct i2c_client *client)
{
#define RG_GOLDEN 0x145
#define BG_GOLDEN 0x15e

	u16 gainr, gaing, gainb, ggainb, ggainr;
	int ret;
	struct ov8825 *ov8825 = to_ov8825(client);
	struct ov8825_otp *otp = &(ov8825->otp);
	if (otp->rg_ratio == 0 || otp->bg_ratio == 0) {
		pr_debug("ov8825_rbgains_update: OTP not initialized\n");
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
	ret =  ov8825_reg_write(client, 0x5180, 1<<3);
	ret |= ov8825_reg_write(client, 0x5186, gainr >> 8);
	ret |= ov8825_reg_write(client, 0x5187, gainr & 0xff);
	ret |= ov8825_reg_write(client, 0x5188, gaing >> 8);
	ret |= ov8825_reg_write(client, 0x5189, gaing & 0xff);
	ret |= ov8825_reg_write(client, 0x518a, gainb >> 8);
	ret |= ov8825_reg_write(client, 0x518b, gainb & 0xff);
	if (ret == 0) {
		dev_info(&client->dev, "ov8825 1 sensor update for calibrated data g=[0x%04X, 0x%04X, 0x%04X]\n",
			 gainr, gaing, gainb);
#if OV5648_DEBUG
		ov8825_rbgains_print(client);
#endif
		return 0;
	} else {
		dev_info(&client->dev,
			 "ov8825 sensor 1 update for calibrated data failed\n");
		return -1;
	}
}
#endif

/* ov8825 use internal vcm driver, the same i2c client */
#define OV8825_VCM_STEP_PERIOD_US		100	   /* in microseconds */
#define OV8825_VCM_MAX_POSITION	  1023
#define OV8825_VCM_START_POINT 100
#define OV8825_VCM_MIN_POSITION	  0
#define OV8825_DAC_A_VCM_LOW 0x3618
#define OV8825_DAC_A_VCM_MID0 0x3619

/*
 * Input: lens position from 0 to 1024
 * Output: DAC code
 * DAC code is {3619[5:0],3618[7:4]}. 3618[3:0] is slew rate.
 */
static int ov8825_vcm_position_code(struct ov8825 *ov8825,
		uint8_t *code1, uint8_t *code2)
{
	int	 pos = ov8825->position;

	*code1 = ((pos & 0xF)  << 4) | 0x2;
	*code2 = (pos >> 4) & 0x3F;

	return 0;
}

static int ov8825_vcm_lens_set_position(struct i2c_client *client,
		int target_position)
{
	int ret = 0;
	unsigned int diff;
	int dac_code;
	struct ov8825 *ov8825 = to_ov8825(client);
	uint8_t code_3618, code_3619;
	int step = 0;

	ov8825->requested_position = target_position;
	dac_code =	((OV8825_VCM_MAX_POSITION - OV8825_VCM_START_POINT)
		* (255 - target_position)) / 255 + OV8825_VCM_START_POINT;
	dac_code = max(OV8825_VCM_MIN_POSITION,
				min(dac_code, OV8825_VCM_MAX_POSITION));
	diff = abs(dac_code - ov8825->position);
	ov8825->position = dac_code;

	pr_debug("ov8825_vcm_lens_set_position diff=%d", diff);

	ov8825_vcm_position_code(ov8825, &code_3618, &code_3619);

	/* Not exact, but close (off by <500us) */
#if 0
	while (diff) {
		if (0 < diff <= 16) {
			step++;
			break;
		} else if (16 < diff <= 128) {
			step += diff / 16;
			   diff = diff % 16;
		} else if (diff > 128) {
			step += diff / 64;
			   diff = diff % 64;
		}
	}
#endif
	pr_debug("ov8825_vcm_lens_set_position step=%d", step);

	ov8825->flying_time = step * OV8825_VCM_STEP_PERIOD_US;
	ov8825->flying_start_time = jiffies_to_usecs(jiffies);

	ret =  ov8825_reg_write(client, OV8825_DAC_A_VCM_LOW, code_3618);
	ret |=	ov8825_reg_write(client, OV8825_DAC_A_VCM_MID0, code_3619);

	pr_debug("target_position=%d dac_code=%d", target_position, dac_code);
	return ret;
}

/*
 * Routine used to get the current lens position and/or the estimated
 *time required to get to the requested destination (time in us).
*/
int ov8825_vcm_lens_get_position(struct i2c_client *client,
		int *current_position, int *time_to_destination)
{
	struct ov8825 *ov8825 = to_ov8825(client);

	*time_to_destination = (ov8825->flying_start_time +
			ov8825->flying_time) - jiffies_to_usecs(jiffies);

	if (*time_to_destination < 0 || !ov8825->flying_time ||
		*time_to_destination > 100000) {
		ov8825->flying_time = 0;
		*current_position = ov8825->requested_position;
	} else {
		*current_position = -1;
	}
	pr_debug("current_position=%X(%d)",	(*current_position) & 0xffff,
			(*current_position) & 0xffff);

	return 0;
}


/*
 *Routine used to send lens to a traget position position and calculate
 *the estimated time required to get to this position, the flying time in us.
*/
static int ov8825_lens_set_position(struct i2c_client *client,
					int target_position)
{
	int ret = 0;

	ret = ov8825_vcm_lens_set_position(client, target_position);

	return ret;
}


/*
 * Routine used to get the current lens position and/or the estimated
 *time required to get to the requested destination (time in us).
*/
static void ov8825_lens_get_position(struct i2c_client *client,
					int *current_position,
					int *time_to_destination)
{
	int ret = 0;
	int i = 0;
	struct ov8825 *ov8825 = to_ov8825(client);

	static int lens_read_buf[LENS_READ_DELAY];
	ret = ov8825_vcm_lens_get_position(client,
			current_position, time_to_destination);

	for (i = 0; i < ov8825->lenspos_delay; i++)
		lens_read_buf[i] = lens_read_buf[i + 1];
	lens_read_buf[ov8825->lenspos_delay] = *current_position;
	*current_position = lens_read_buf[0];

	return;
}

/*
 * Setup the sensor integration and frame length based on requested mimimum
 * framerate
 */
static void ov8825_set_framerate_lo(struct i2c_client *client, int framerate)
{
	struct ov8825 *ov8825 = to_ov8825(client);
	framerate = max(framerate, ov8825->framerate_lo_absolute);
	framerate = min(framerate, ov8825->framerate_hi_absolute);

	ov8825->framerate_lo = framerate;
	ov8825->vts_max = 1000000000
		/ (unsigned long)((ov8825->line_length * framerate) >> 8);
	ov8825->vts_max = min(ov8825->vts_max,
			ov8825_mode[ov8825->mode_idx].vts_max);
	pr_debug("ov8825_set_framerate_lo: Setting frame rate lo %d vts_max %d",
			ov8825->framerate_lo, ov8825->vts_max);
}

/*
 * Setup the sensor integration and frame length based on requested maximum
 * framerate, famerate is 24p8 fixed point.
 */
static void ov8825_set_framerate_hi(struct i2c_client *client, int framerate)
{
	struct ov8825 *ov8825 = to_ov8825(client);

	framerate = max(framerate, ov8825->framerate_lo_absolute);
	framerate = min(framerate, ov8825->framerate_hi_absolute);

	ov8825->framerate_hi = framerate;
	ov8825->vts_min = 1000000000
		/ (unsigned long)((ov8825->line_length * framerate) >> 8);

	ov8825->vts_min = max(ov8825->vts_min,
			ov8825_mode[ov8825->mode_idx].vts);
	pr_debug("ov8825_set_framerate_hi: Setting frame rate hi %d vts_min %d",
		ov8825->framerate_hi, ov8825->vts_min);
}


/*
 * Determine the closest achievable gain to the desired gain.
 * Calculates the closest achievable gain to the requested gain
 * that can be achieved by this sensor.
 * @param gain_value Desired gain on 8.8 linear scale.
 * @return Achievable gain on 8.8 linear scale.
*/
static int ov8825_calc_gain(struct i2c_client *client, int gain_value,
					int *gain_code_p)
{
	int gain_code = (gain_value	 & 0x3fff) >> 4;
	int actual_gain = gain_code << 4;

	if (gain_code_p)
		*gain_code_p = gain_code;
	return actual_gain;
}

/*
 *setup the sensor analog gain on requested gain value 8.8 linear scale.
*/
#define GAIN_HIST_MAX 0
static int ov8825_set_gain(struct i2c_client *client, int gain_value)
{
	struct ov8825 *ov8825 = to_ov8825(client);
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
	for (i = 0; i < GAIN_HIST_MAX; i++)
		gain_value = gain_value + gain_prev[i];
	gain_value = gain_value / GAIN_HIST_MAX;
#endif
	gain_actual = ov8825_calc_gain(client, gain_value, &gain_code_analog);
	pr_debug("ov8825_set_gain: cur=%u req=%u act=%u cod=%u",
		 ov8825->gain_current, gain_value,
		 gain_actual, gain_code_analog);
	if (gain_actual == ov8825->gain_current)
		return 0;
	ret = ov8825_reg_write(client,
		OV8825_REG_AGC_HI, (gain_code_analog >> 8) & 0x3);
	ret |= ov8825_reg_write(client,
		OV8825_REG_AGC_LO, gain_code_analog & 0xFF);
	ov8825->gain_current = gain_actual;
	return 0;
}

static int ov8825_get_gain(struct i2c_client *client,
		int *gain_value_p, int *gain_code_p)
{
	struct ov8825 *ov8825 = to_ov8825(client);

	int ret = 0;
	int gain_code;
	u8 gain_buf[2];
	int i;

	gain_code = ov8825->gain_current >> 4;

/*
	if (is_snap_to_prev_gain == 1) {
		gain_code = ov8825->gain_read_buf[0];
		is_snap_to_prev_gain = 0;
		}
*/

	if (ov8825->aecpos_delay > 0) {
		ov8825->gain_read_buf[ov8825->aecpos_delay] = gain_code;
		gain_code = ov8825->gain_read_buf[0];
		for (i = 0; i < GAIN_DELAY_MAX-1; i++)
			ov8825->gain_read_buf[i] = ov8825->gain_read_buf[i+1];
	}

	if (gain_code < 0x10)
		gain_code = 0x10;

	if (gain_code_p)
		*gain_code_p = gain_code;
	if (gain_value_p)
		*gain_value_p = gain_code << 4;
	return ret;
}

static int ov8825_get_exposure(struct i2c_client *client,
		int *exp_value_p, int *exp_code_p)
{
	struct ov8825 *ov8825 = to_ov8825(client);
	int i, ret = 0;
	int exp_code;
	u8 exp_buf[3];

	exp_code = (ov8825->exposure_current * 1000 / ov8825->line_length) << 4;

/*
	if (is_snap_to_prev_expo == 1) {
		exp_code = ov8825->exp_read_buf[0];
		is_snap_to_prev_expo = 0;
		}
*/

	if (ov8825->aecpos_delay > 0) {
		ov8825->exp_read_buf[ov8825->aecpos_delay] = exp_code;
		exp_code = ov8825->exp_read_buf[0];
		for (i = 0; i < EXP_DELAY_MAX-1; i++)
			ov8825->exp_read_buf[i] = ov8825->exp_read_buf[i+1];
	}

	pr_debug("ov8825_get_exposure: exp_code=%d", exp_code);

	if (exp_code_p)
		*exp_code_p = exp_code;
	if (exp_value_p)
		*exp_value_p = ((exp_code >> 4) * ov8825->line_length) /
			(unsigned long)1000;
	return ret;
}

#define INTEGRATION_MIN		1
#define INTEGRATION_OFFSET	10

static int ov8825_calc_exposure(struct i2c_client *client,
				int exposure_value,
				unsigned int *vts_ptr,
				unsigned int *coarse_int_lines)
{
	struct ov8825 *ov8825 = to_ov8825(client);
	unsigned int integration_lines;
	int vts = ov8825_mode[ov8825->mode_idx].vts;
	int binning_factor = ov8825_mode[ov8825->mode_idx].binning_factor;

	integration_lines = (1000 * exposure_value) / ov8825->line_length;
	integration_lines = integration_lines * binning_factor;
	if (integration_lines < INTEGRATION_MIN)
		integration_lines = INTEGRATION_MIN;
	else if (integration_lines > (unsigned int)
		 (ov8825->vts_max - INTEGRATION_OFFSET))
		integration_lines = ov8825->vts_max - INTEGRATION_OFFSET;
	vts = min(integration_lines + INTEGRATION_OFFSET,
		  (unsigned int)ov8825_mode[ov8825->mode_idx].vts_max);
	vts = max_t(u16, vts, ov8825_mode[ov8825->mode_idx].vts);
	vts = max_t(u16, vts, ov8825->vts_min);
	if (coarse_int_lines)
		*coarse_int_lines = integration_lines;
	if (vts_ptr)
		*vts_ptr = vts;

	exposure_value = integration_lines * ov8825->line_length /
		(binning_factor * (unsigned long)1000);

	pr_debug("%s(): integration_lines=%d vts_min=%d mode_vts_max=%d",
		__func__,
		integration_lines,
		ov8825->vts_min,
		ov8825_mode[ov8825->mode_idx].vts_max);

	return exposure_value;
}

/*
 * Setup the sensor integration and frame length based on requested exposure
 * in microseconds.
 */
#define EXP_HIST_MAX 0
static void ov8825_set_exposure(struct i2c_client *client, int exp_value)
{
	struct ov8825 *ov8825 = to_ov8825(client);
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

	actual_exposure = ov8825_calc_exposure(client, exp_value,
			&vts, &coarse_int_lines);
	pr_debug("ov8825_set_exposure: cur=%d req=%d act=%d coarse=%d vts=%d",
			ov8825->exposure_current, exp_value, actual_exposure,
			coarse_int_lines, vts);
	ov8825->vts = vts;
	ov8825->exposure_current = actual_exposure;
	coarse_int_lines <<= 4;
	exp_buf[0] = (u8)((OV8825_REG_EXP_HI >> 8) & 0xFF);
	exp_buf[1] = (u8)(OV8825_REG_EXP_HI & 0xFF);
	exp_buf[2] = (u8)((coarse_int_lines >> 16) & 0xFF);
	exp_buf[3] = (u8)((coarse_int_lines >> 8) & 0xFF);
	exp_buf[4] = (u8)((coarse_int_lines >> 0) & 0xF0);
	ret = ov8825_array_write(client, exp_buf, 5);
	ret |= ov8825_reg_write(client,
			OV8825_REG_TIMING_VTS_HI, (vts & 0xFF00) >> 8);
	ret |= ov8825_reg_write(client, OV8825_REG_TIMING_VTS_LO, vts & 0xFF);
	if (ret) {
		pr_debug("ov8825_set_exposure: error writing exp. ctrl");
		return;
	}
}


/*
 *
 */
static int ov8825_frame_irq(struct v4l2_subdev *sd, u32 irq)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ov8825 *ov8825 = to_ov8825(client);
	int i;

	/* flash_timeout mean frame counts with flash on
	assume frame end irq only */
	if (0 == ov8825->flash_on) {
		if (ov8825->flash_mode == V4L2_FLASH_LED_MODE_FLASH) {
			if (ov8825->flash_timeout > 0 &&
			    ov8825->flash_intensity > 0) {
				ov8825->lamp.enable();
				ov8825->flash_on = ov8825->flash_timeout;
				pr_debug("%s(): flash_on: 0->%d;",
					 __func__,
					 ov8825->flash_on);
			} else {
				ov8825->flash_mode = V4L2_FLASH_LED_MODE_NONE;
			}
		}
	} else {
		if (ov8825->flash_mode == V4L2_FLASH_LED_MODE_FLASH) {
			if (ov8825->flash_on == 1) {
				ov8825->lamp.disable();
				ov8825->flash_on = 0;
				ov8825->flash_mode = V4L2_FLASH_LED_MODE_NONE;
				pr_debug("%s(): flash_off", __func__);
			} else {
				pr_debug("%s(): flash_on: %d->%d",
					 __func__,
					 ov8825->flash_on,
					 ov8825->flash_on - 1);
				ov8825->flash_on -= 1;
			}
		}
	}

	/* shift frame info */
	for (i = 0; i < ov8825->flash_delay; i++) {
		ov8825->flash_int_buf[i] =
			ov8825->flash_int_buf[i + 1];
		ov8825->flash_mod_buf[i] =
			ov8825->flash_mod_buf[i + 1];
	}
	ov8825->flash_int_buf[ov8825->flash_delay] =
		ov8825->flash_on ? ov8825->flash_intensity : 0;
	ov8825->flash_mod_buf[ov8825->flash_delay] =
		ov8825->flash_mode;

	/* count fps */
	if (ov8825->fcnt >= 30) {

		int fps = ov8825->fcnt * 1000 /
			(jiffies_to_msecs(jiffies) - ov8825->fps_t0);
		pr_debug("%s(): fps=%d",
			 __func__, fps);
		if (fps > 0)
			ov8825->fps = fps;
		ov8825->fcnt = 1;
		ov8825->fps_t0 = jiffies_to_msecs(jiffies);
	} else {
		ov8825->fcnt += 1;
	}

	return 0;
}


static int ov8825_s_stream(struct v4l2_subdev *sd, int enable)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int ret;

	ret = ov8825_set_state(client, enable);
	return ret;
}

static int ov8825_g_fmt(struct v4l2_subdev *sd, struct v4l2_mbus_framefmt *mf)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ov8825 *ov8825 = to_ov8825(client);

	mf->width = ov8825_mode[ov8825->mode_idx].width;
	mf->height = ov8825_mode[ov8825->mode_idx].height;
	mf->code = ov8825_fmts[ov8825->i_fmt].code;
	mf->colorspace = ov8825_fmts[ov8825->i_fmt].colorspace;
	mf->field = V4L2_FIELD_NONE;

	return 0;
}

static int ov8825_try_fmt(struct v4l2_subdev *sd, struct v4l2_mbus_framefmt *mf)
{
	int i_fmt;
	int mode_idx;

	i_fmt = ov8825_find_datafmt(mf->code);

	mf->code = ov8825_fmts[i_fmt].code;
	mf->colorspace = ov8825_fmts[i_fmt].colorspace;
	mf->field = V4L2_FIELD_NONE;

	mode_idx = ov8825_find_framesize(mf->width, mf->height);

	mf->width = ov8825_mode[mode_idx].width;
	mf->height = ov8825_mode[mode_idx].height;

	return 0;
}

static int ov8825_set_mode(struct i2c_client *client, int new_mode_idx)
{
	struct ov8825 *ov8825 = to_ov8825(client);
	int ret = 0;

	if (ov8825->mode_idx == new_mode_idx) {
		pr_debug("ov8825_set_mode: skip init from mode[%d]=%s to mode[%d]=%s",
			ov8825->mode_idx, ov8825_mode[ov8825->mode_idx].name,
			new_mode_idx, ov8825_mode[new_mode_idx].name);
		return ret;
	}

	if (ov8825->mode_idx == OV8825_MODE_MAX) {
		pr_debug("ov8825_set_mode: full init from mode[%d]=%s to mode[%d]=%s",
		ov8825->mode_idx, ov8825_mode[ov8825->mode_idx].name,
		new_mode_idx, ov8825_mode[new_mode_idx].name);
		ov8825_init(client);
		ret = ov8825_reg_writes(client, ov8825_reginit);
		ret  = ov8825_reg_writes(client, ov8825_regdif[new_mode_idx]);
	} else {
		pr_debug("ov8825_set_mode: diff init from mode[%d]=%s to mode[%d]=%s",
			ov8825->mode_idx, ov8825_mode[ov8825->mode_idx].name,
			new_mode_idx, ov8825_mode[new_mode_idx].name);
		ret = ov8825_reg_writes(client, ov8825_regdif[new_mode_idx]);
	}
	if (ret)
		return ret;

/*
	if (new_mode_idx == 1) {
		is_snap_to_prev_expo = 1;
		is_snap_to_prev_gain = 1;
		}
*/

	ov8825->mode_idx = new_mode_idx;
	ov8825->line_length	 = ov8825_mode[new_mode_idx].line_length_ns;
	ov8825->vts = ov8825_mode[new_mode_idx].vts;
	ov8825->framerate_lo_absolute = F24p8(1.0);
	ov8825->framerate_hi_absolute = ov8825_mode[new_mode_idx].fps;
	ov8825_set_framerate_lo(client, ov8825->framerate_lo_absolute);
	ov8825_set_framerate_hi(client, ov8825->framerate_hi_absolute);
	ov8825_set_exposure(client, ov8825->exposure_current);
	ov8825_set_gain(client, ov8825->gain_current);
	return 0;
}

static int ov8825_set_state(struct i2c_client *client, int new_state)
{
	struct ov8825 *ov8825 = to_ov8825(client);
	int ret = 0;

	pr_debug("ov8825_set_state: %d (%s) -> %d (%s)", ov8825->state,
		 ov8825->state ? "strm" : "stop",
		 new_state, new_state ? "strm" : "stop");

	if (ov8825->state != new_state) {
		ret = ov8825_reg_writes(client, ov8825_reg_state[new_state]);
		ov8825->state = new_state;
		if (OV8825_STATE_STRM == new_state) {
			ov8825->frame_info_size = 0;
			memset(ov8825->frame_info, 0,
					ARRAY_SIZE(ov8825->frame_info));
			if (0 == ov8825->calibrated)
				ov8825->calibrated = 1;
		}
	}
	return ret;
}

static int ov8825_s_fmt(struct v4l2_subdev *sd, struct v4l2_mbus_framefmt *mf)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ov8825 *ov8825 = to_ov8825(client);
	int ret = 0;
	int new_mode_idx;

	ret = ov8825_try_fmt(sd, mf);
	if (ret < 0)
		return ret;

	pr_debug("ov8825_s_fmt: width %d, height %d", mf->width, mf->height);

	new_mode_idx = ov8825_find_framesize(mf->width, mf->height);
	ov8825->i_fmt = ov8825_find_datafmt(mf->code);
	switch ((u32) ov8825_fmts[ov8825->i_fmt].code) {
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

	ov8825_set_mode(client, new_mode_idx);
	return ret;
}

static int ov8825_g_chip_ident(struct v4l2_subdev *sd,
				   struct v4l2_dbg_chip_ident *id)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

/*
	if (id->match.type != V4L2_CHIP_MATCH_I2C_ADDR)
		return -EINVAL;

	if (id->match.addr != client->addr)
		return -ENODEV;
*/

	id->ident = V4L2_IDENT_OV8825;
	id->revision = 0;

	return 0;
}

void ov8825_check_mounting(struct i2c_client *client, struct v4l2_ctrl *ctrl)
{
	struct soc_camera_subdev_desc *ssd = client->dev.platform_data;
	struct v4l2_subdev_sensor_interface_parms *iface_parms;

	if (ssd && ssd->drv_priv) {
		iface_parms = ssd->drv_priv;
		pr_debug("facing: %d V4L2_SUBDEV_SENSOR_BACK: %d\n",
			iface_parms->facing, V4L2_SUBDEV_SENSOR_BACK);

		if (iface_parms->orientation == V4L2_SUBDEV_SENSOR_PORTRAIT)
			ctrl->val |= V4L2_IN_ST_HFLIP;

		if (iface_parms->facing == V4L2_SUBDEV_SENSOR_BACK)
			ctrl->val |= V4L2_IN_ST_BACK;
	} else
		dev_err(&client->dev, "Missing interface parameters\n");
}

static int ov8825_g_ctrl(struct v4l2_ctrl *ctrl)
{
	struct ov8825 *ov8825 = container_of(ctrl->handler, struct ov8825, hdl);
	struct v4l2_subdev *sd = &ov8825->subdev;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int retval;

	switch (ctrl->id) {
	case V4L2_CID_CAMERA_FRAME_RATE:
		ctrl->val = ov8825->framerate;
		break;
	case V4L2_CID_CAMERA_FOCUS_MODE:
		ctrl->val = ov8825->focus_mode;
		break;
	case V4L2_CID_GAIN:
		ov8825_get_gain(client, &retval, NULL);
		ctrl->val = retval;
		break;
	case V4L2_CID_EXPOSURE:
		ov8825_get_exposure(client, &retval, NULL);
		ctrl->val = retval;
		break;
	case V4L2_CID_CAMERA_LENS_POSITION:
		ov8825_lens_get_position(client, &ov8825->current_position,
			&ov8825->time_to_destination);
		ctrl->val = ov8825->current_position;
		break;
	case V4L2_CID_FLASH_LED_MODE:
		ctrl->val = ov8825->flashmode;
		break;
	case V4L2_CID_FLASH_INTENSITY:
		ctrl->val = ov8825->flash_intensity;
		break;
	case V4L2_CID_FLASH_TIMEOUT:
		ctrl->val = ov8825->flash_timeout;
		break;
	case V4L2_CID_CAM_MOUNTING:
		ov8825_check_mounting(client, ctrl);
		break;
	}
	return 0;
}

static int ov8825_s_ctrl(struct v4l2_ctrl *ctrl)
{
	struct ov8825 *ov8825 = container_of(ctrl->handler, struct ov8825, hdl);
	struct v4l2_subdev *sd = &ov8825->subdev;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int ret = 0;

	switch (ctrl->id) {
	case V4L2_CID_CAMERA_FRAME_RATE:

		if (ctrl->val > FRAME_RATE_30)
			return -EINVAL;

		ov8825->framerate = ctrl->val;

		switch (ov8825->framerate) {
		case FRAME_RATE_5:
			ov8825->framerate_lo = F24p8(5.0);
			ov8825->framerate_hi = F24p8(5.0);
			break;
		case FRAME_RATE_7:
			ov8825->framerate_lo = F24p8(7.0);
			ov8825->framerate_hi = F24p8(7.0);
			break;
		case FRAME_RATE_10:
			ov8825->framerate_lo = F24p8(10.0);
			ov8825->framerate_hi = F24p8(10.0);
			break;
		case FRAME_RATE_15:
			ov8825->framerate_lo = F24p8(15.0);
			ov8825->framerate_hi = F24p8(15.0);
			break;
		case FRAME_RATE_20:
			ov8825->framerate_lo = F24p8(20.0);
			ov8825->framerate_hi = F24p8(20.0);
			break;
		case FRAME_RATE_25:
			ov8825->framerate_lo = F24p8(25.0);
			ov8825->framerate_hi = F24p8(25.0);
			break;
		case FRAME_RATE_30:
			ov8825->framerate_lo = F24p8(30.0);
			ov8825->framerate_hi = F24p8(30.0);
			break;
		case FRAME_RATE_AUTO:
		default:
			ov8825->framerate_lo = F24p8(1.0);
			ov8825->framerate_hi = F24p8(30.0);
			break;
		}
		ov8825_set_framerate_lo(client, ov8825->framerate_lo);
		ov8825_set_framerate_hi(client, ov8825->framerate_hi);
		ov8825_set_exposure(client, ov8825->exposure_current);
		ov8825_set_gain(client, ov8825->gain_current);

		if (ret)
			return ret;
		break;

	case V4L2_CID_CAMERA_FOCUS_MODE:

		if (ctrl->val > FOCUS_MODE_MANUAL)
			return -EINVAL;

		ov8825->focus_mode = ctrl->val;

		/*
		 * Donot start the AF cycle here
		 * AF Start will be called later in
		 * V4L2_CID_CAMERA_SET_AUTO_FOCUS only for auto, macro mode
		 * it wont be called for infinity.
		 * Donot worry about resolution change for now.
		 * From userspace we set the resolution first
		 * and then set the focus mode.
		 */
		switch (ov8825->focus_mode) {
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

		if (ctrl->val > AUTO_FOCUS_ON)
			return -EINVAL;

		if (ret)
			return ret;
		break;
	case V4L2_CID_GAIN:
		if ((unsigned int)(ctrl->val) > OV8825_GAIN_MAX) {
			pr_debug("V4L2_CID_GAIN invalid gain=%u max=%u",
				ctrl->val, OV8825_GAIN_MAX);
			return -EINVAL;
		}
		ov8825_set_gain(client, ctrl->val);
		break;

	case V4L2_CID_EXPOSURE:
		if (ctrl->val > OV8825_EXP_MAX)
			return -EINVAL;
		ov8825_set_exposure(client, ctrl->val);
		break;

	case V4L2_CID_CAMERA_LENS_POSITION:
		if (ctrl->val > OV8825_LENS_MAX)
			return -EINVAL;
		ov8825_lens_set_position(client, ctrl->val);
		break;
	case V4L2_CID_FLASH_LED_MODE:
		ov8825_set_flash_mode(client, ctrl->val);
		break;
	case V4L2_CID_FLASH_INTENSITY:
		ov8825->flash_intensity = ctrl->val;
		break;
	case V4L2_CID_FLASH_TIMEOUT:
		ov8825->flash_timeout = ctrl->val;
		break;

	}

	return ret;
}

int ov8825_set_flash_mode(struct i2c_client *client, int mode)
{
	struct ov8825 *ov8825 = to_ov8825(client);

	/* check if lamp is presented */
	if (NULL == ov8825->lamp.name)
		return -1;

	/* check if lamp is in requested mode */
	if (ov8825->flash_mode == mode)
		return 0;

	/* apply new mode */
	switch (mode) {
	case V4L2_FLASH_LED_MODE_NONE:
		ov8825->lamp.setup(mode, 0, 0);
		ov8825->lamp.disable();
		ov8825->flash_on = 0;
		break;

	case V4L2_FLASH_LED_MODE_TORCH:
		ov8825->lamp.setup(mode, 0, 255);
		ov8825->lamp.enable();
		break;

	case V4L2_FLASH_LED_MODE_FLASH:
	case V4L2_FLASH_LED_MODE_FLASH_AUTO:
	{
		int flash_timeout_us =
			ov8825->flash_timeout *
			ov8825->line_length *
			ov8825->vts / 1000;
		ov8825->lamp.setup(mode, flash_timeout_us,
					ov8825->flash_intensity);
		break;
	}

	default:
		return -EINVAL;
	}

	/* update mode */
	ov8825->flash_mode = mode;

	return 0;
}

static long ov8825_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
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
			p->hor_angle.numerator = 6110;
			p->hor_angle.denominator = 100;
			p->ver_angle.numerator = 4790;
			p->ver_angle.denominator = 100;
			p->focus_distance[0] = 10; /* near focus in cm */
			p->focus_distance[1] = 100; /* optimal focus in cm */
			p->focus_distance[2] = -1; /* infinity */
			p->focal_length.numerator = 385;
			p->focal_length.denominator = 100;
			break;
		}

	case VIDIOC_SENSOR_G_FRAME_INFO:
		{
			int i, j;
			struct i2c_client *client = v4l2_get_subdevdata(sd);
			struct ov8825 *ov8825 = to_ov8825(client);
			struct v4l2_frame_info *fi_cache, *fi_usr =
					(struct v4l2_frame_info *)arg;
			ret = -EINVAL;
			if (!timespec_valid(&fi_usr->timestamp)) {
				fi_usr->flash_mode =
					ov8825->flash_mod_buf[0];
				fi_usr->flash_intensity =
					ov8825->flash_int_buf[0];
				ov8825_get_gain(client, &fi_usr->an_gain, NULL);
				ov8825_get_exposure(client,
						&fi_usr->exposure, NULL);
				ov8825_lens_get_position(client,
						&fi_usr->lens_pos, &i);
				ret = 0;
				break;
			}
			j = ov8825->frame_info_size;
			fi_cache = &ov8825->frame_info[0];

			for (i = 0; i < V4L2_FRAME_INFO_CACHE && j > 0;
					i++, j--, fi_cache++) {
				if (timespec_equal(&fi_cache->timestamp,
						&fi_usr->timestamp))
					break;
			}
			if (j != 0) {
				*fi_usr = *fi_cache;
				ret = 0;
			}
			break;
		}
	case VIDIOC_SENSOR_S_FRAME_INFO:
		{
			struct i2c_client *client = v4l2_get_subdevdata(sd);
			struct ov8825 *ov8825 = to_ov8825(client);
			struct v4l2_frame_info *fi_dst, *fi_src;

			fi_dst = &ov8825->frame_info[0];
			fi_src = (struct v4l2_frame_info *)arg;
			if (!timespec_valid(&fi_src->timestamp))
				break;
			if (ov8825->frame_info_size == V4L2_FRAME_INFO_CACHE) {
				int i;
				for (i = 0; i < V4L2_FRAME_INFO_CACHE - 1; i++)
					fi_dst[i] = fi_dst[i + 1];
				ov8825->frame_info_size--;
			}
			fi_dst[ov8825->frame_info_size++] = *fi_src;
			break;
		}
	case VIDIOC_SENSOR_FRAME_IRQ:
		{
			ov8825_frame_irq(sd, (u32)arg);
			break;
		}

	default:
		ret = -ENOIOCTLCMD;
		break;
	}
	return ret;
}

#ifdef CONFIG_VIDEO_ADV_DEBUG
static int ov8825_g_register(struct v4l2_subdev *sd,
				 struct v4l2_dbg_register *reg)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	if (reg->match.type != V4L2_CHIP_MATCH_I2C_ADDR || reg->size > 2)
		return -EINVAL;

	if (reg->match.addr != client->addr)
		return -ENODEV;

	reg->size = 2;
	if (ov8825_reg_read(client, reg->reg, &reg->val))
		return -EIO return 0;
}

static int ov8825_s_register(struct v4l2_subdev *sd,
				 struct v4l2_dbg_register *reg)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	if (reg->match.type != V4L2_CHIP_MATCH_I2C_ADDR || reg->size > 2)
		return -EINVAL;

	if (reg->match.addr != client->addr)
		return -ENODEV;

	if (ov8825_reg_write(client, reg->reg, reg->val))
		return -EIO;

	return 0;
}
#endif

static int ov8825_s_power(struct v4l2_subdev *sd, int on)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ov8825 *ov8825 = to_ov8825(client);
	struct soc_camera_subdev_desc *ssdd = soc_camera_i2c_to_desc(client);

	if (!on) {
		ov8825->mode_idx = OV8825_MODE_MAX;
		ov8825->calibrated = 0;
		return soc_camera_power_off(&client->dev, ssdd);
	}
	return soc_camera_power_on(&client->dev, ssdd);
}

static int ov8825_init(struct i2c_client *client)
{
	struct ov8825 *ov8825 = to_ov8825(client);
	int ret = 0;

	/* reset */
	ov8825_reg_write(client, 0x0103, 0x01);

	/* set initial mode */
	ov8825->mode_idx = OV8825_MODE_MAX;
	ov8825->state = OV8825_STATE_STOP;

	/* default settings */
	ov8825->framerate		  = FRAME_RATE_AUTO;
	ov8825->focus_mode		  = FOCUS_MODE_AUTO;
	ov8825->gain_current	  = DEFAULT_GAIN;

	memset(&ov8825->exp_read_buf, 0, sizeof(ov8825->exp_read_buf));
	memset(&ov8825->gain_read_buf, 0, sizeof(ov8825->gain_read_buf));
	memset(&ov8825->lens_read_buf, 0, sizeof(ov8825->lens_read_buf));
	/*
	 *	Exposure should be DEFAULT_EXPO * line_length / 1000
	 *	Since we don't have line_length yet, just estimate
	 */

	ov8825->exposure_current  = 10000;
	ov8825->exp_read_buf[0] = 6032;
	ov8825->exp_read_buf[1] = 6032;
	ov8825->gain_read_buf[0] =	(ov8825->gain_current  & 0x3fff) >> 4;
	ov8825->gain_read_buf[1] =	(ov8825->gain_current  & 0x3fff) >> 4;
	ov8825->aecpos_delay	  = 1;
	ov8825->lenspos_delay	  = 0;
	ov8825->flashmode		  = V4L2_FLASH_LED_MODE_NONE;
	ov8825->flash_intensity	  = OV8825_FLASH_INTENSITY_DEFAULT;
	ov8825->flash_timeout	  = OV8825_FLASH_TIMEOUT_DEFAULT;

	ov8825->flash_mode = V4L2_FLASH_LED_MODE_NONE;
	ov8825->flash_delay = 0;
	ov8825->fcnt = 1;

	ov8825->position = 0;
	ov8825->dac_code = 0;
	ov8825->position = 0;
	ov8825->timing_step_us = 0xf;

	if (0 == get_flash_lamp(&ov8825->lamp)) {
		ov8825->lamp.reset();
	} else {
	    /* no flashlamp */
	    ov8825->lamp.name = NULL;
	}

	dev_dbg(&client->dev, "Sensor initialized\n");
	return ret;
}

/*
 * Interface active, can use i2c. If it fails, it can indeed mean, that
 * this wasn't our capture interface, so, we wait for the right one
 */
static int ov8825_video_probe(struct i2c_client *client)
{
	unsigned long flags;
	int ret = 0;
	u8 revision = 0, id_high, id_low;
	u16 id;

	ret = ov8825_reg_read(client, 0x302A, &revision);
	if (ret) {
		dev_err(&client->dev, "Failure to detect OV8825 chip\n");
		goto out;
	}

	revision &= 0xF;

	flags = SOCAM_DATAWIDTH_8;

	/* Read sensor Model ID */
	ret = ov8825_reg_read(client, OV8825_REG_CHIP_ID_HIGH, &id_high);
	if (ret < 0)
		return ret;

	id = id_high << 8;

	ret = ov8825_reg_read(client, OV8825_REG_CHIP_ID_LOW, &id_low);
	if (ret < 0)
		return ret;

	id |= id_low;

	if (id != 0x8825)
		return -ENODEV;

	dev_err(&client->dev, "Detected a OV8825 chip 0x%04x, revision %x\n",
		 id, revision);

out:
	return ret;
}

static struct v4l2_subdev_core_ops ov8825_subdev_core_ops = {
	.g_chip_ident = ov8825_g_chip_ident,
	.ioctl = ov8825_ioctl,
#ifdef CONFIG_VIDEO_ADV_DEBUG
	.g_register = ov8825_g_register,
	.s_register = ov8825_s_register,
#endif
	.s_power = ov8825_s_power,
};

static int ov8825_enum_fmt(struct v4l2_subdev *sd, unsigned int index,
			   enum v4l2_mbus_pixelcode *code)
{
	if (index >= ARRAY_SIZE(ov8825_fmts))
		return -EINVAL;

	*code = ov8825_fmts[index].code;
	return 0;
}

static int ov8825_enum_framesizes(struct v4l2_subdev *sd,
				  struct v4l2_frmsizeenum *fsize)
{
	if (fsize->index >= OV8825_MODE_MAX)
		return -EINVAL;

	fsize->type = V4L2_FRMSIZE_TYPE_DISCRETE;
	fsize->pixel_format = V4L2_PIX_FMT_SRGGB10;
	fsize->discrete.width = ov8825_mode[fsize->index].width;
	fsize->discrete.height = ov8825_mode[fsize->index].height;

	return 0;
}

/* we only support fixed frame rate */
static int ov8825_enum_frameintervals(struct v4l2_subdev *sd,
					  struct v4l2_frmivalenum *interval)
{
	int size;

	if (interval->index >= 1)
		return -EINVAL;

	interval->type = V4L2_FRMIVAL_TYPE_DISCRETE;

	size = ov8825_find_framesize(interval->width, interval->height);

	switch (size) {
	case OV8825_MODE_3264x2448P15:
		interval->discrete.numerator = 1;
		interval->discrete.denominator = 15;
		break;
	case OV8825_MODE_1920x1080P30:
	case OV8825_MODE_1280x960P30:
	default:
		interval->discrete.numerator = 1;
		interval->discrete.denominator = 30;
		break;
	}
	return 0;
}

static int ov8825_g_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *param)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ov8825 *ov8825 = to_ov8825(client);
	struct v4l2_captureparm *cparm;

	if (param->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;

	cparm = &param->parm.capture;

	memset(param, 0, sizeof(*param));
	param->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	cparm->capability = V4L2_CAP_TIMEPERFRAME;

	switch (ov8825->mode_idx) {
	case OV8825_MODE_3264x2448P15:
		cparm->timeperframe.numerator = 1;
		cparm->timeperframe.denominator = 15;
		break;
	case OV8825_MODE_1920x1080P30:
	case OV8825_MODE_1280x960P30:
	default:
		cparm->timeperframe.numerator = 1;
		cparm->timeperframe.denominator = 30;
		break;
	}

	return 0;
}
static int ov8825_s_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *param)
{
	/*
	 * FIXME: This just enforces the hardcoded framerates until this is
	 *flexible enough.
	 */
	return ov8825_g_parm(sd, param);
}

static struct v4l2_subdev_video_ops ov8825_subdev_video_ops = {
	.s_stream = ov8825_s_stream,
	.s_mbus_fmt = ov8825_s_fmt,
	.g_mbus_fmt = ov8825_g_fmt,
	.try_mbus_fmt = ov8825_try_fmt,
	.enum_mbus_fmt = ov8825_enum_fmt,
	.enum_mbus_fsizes = ov8825_enum_framesizes,
	.enum_framesizes = ov8825_enum_framesizes,
	.enum_frameintervals = ov8825_enum_frameintervals,
	.g_parm = ov8825_g_parm,
	.s_parm = ov8825_s_parm,
};
static int ov8825_g_skip_frames(struct v4l2_subdev *sd, u32 *frames)
{
	/* Quantity of initial bad frames to skip. Revisit. */
	*frames = 2;

	return 0;
}

static int ov8825_g_interface_parms(struct v4l2_subdev *sd,
		struct v4l2_subdev_sensor_interface_parms *parms)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ov8825 *ov8825 = to_ov8825(client);

	if (!parms)
		return -EINVAL;
	parms->if_type = ov8825->plat_parms->if_type;
	parms->if_mode = ov8825->plat_parms->if_mode;
	parms->parms = ov8825->plat_parms->parms;

	/* set the hs term time */
	parms->parms.serial.hs_term_time = 0;
	parms->parms.serial.hs_settle_time = 2;

	return 0;
}

static struct v4l2_subdev_sensor_ops ov8825_subdev_sensor_ops = {
	.g_skip_frames = ov8825_g_skip_frames,
	.g_interface_parms = ov8825_g_interface_parms,
};

static struct v4l2_subdev_ops ov8825_subdev_ops = {
	.core = &ov8825_subdev_core_ops,
	.video = &ov8825_subdev_video_ops,
	.sensor = &ov8825_subdev_sensor_ops,
};

#ifdef CONFIG_VIDEO_A3907
#define A3907_I2C_ADDR 0x18
static struct i2c_board_info a3907_i2c_board_info = {
	 I2C_BOARD_INFO("a3907", (A3907_I2C_ADDR >> 1))
};
static struct i2c_client *a3907_i2c_client;
static struct i2c_adapter *a3907_i2c_adap;
#endif

static int ov8825_probe(struct i2c_client *client,
			const struct i2c_device_id *did)
{
	struct ov8825 *ov8825;
	struct soc_camera_subdev_desc *sdesc = soc_camera_i2c_to_desc(client);
	int ret, i;

	if (!sdesc) {
		dev_err(&client->dev, "%s: missing platform data!\n", __func__);
		return -EINVAL;
	}

	ov8825 = kzalloc(sizeof(struct ov8825), GFP_KERNEL);
	if (!ov8825)
		return -ENOMEM;

	v4l2_i2c_subdev_init(&ov8825->subdev, client, &ov8825_subdev_ops);

	/* Second stage probe - when a capture adapter is there */

	ov8825->mode_idx = OV8825_MODE_MAX;
	ov8825->i_fmt = 0;	/* First format in the list */
	ov8825->plat_parms = sdesc->drv_priv;

	struct v4l2_subdev *subdev = i2c_get_clientdata(client);

	v4l2_i2c_subdev_init(&ov8825->subdev, client, &ov8825_subdev_ops);

	v4l2_ctrl_handler_init(&ov8825->hdl,  ARRAY_SIZE(ov8825_controls) + 6);

	if (ov8825->hdl.error)
		pr_err("error set during init itself!\n");

	/* register standard controls */
	if (ov8825->hdl.error) {
		dev_err(&client->dev,
			"Standard controls initialization error %d\n",
			ov8825->hdl.error);
		ret = ov8825->hdl.error;
		goto ctrl_hdl_err;
	}

	/* register standard menu controls */

	v4l2_ctrl_new_std_menu(&ov8825->hdl, &ov8825_ctrl_ops,
		V4L2_CID_AUTO_FOCUS_RANGE, V4L2_AUTO_FOCUS_RANGE_INFINITY,
		0, V4L2_AUTO_FOCUS_RANGE_AUTO);
	v4l2_ctrl_new_std_menu(&ov8825->hdl, &ov8825_ctrl_ops,
		V4L2_CID_FLASH_LED_MODE, V4L2_FLASH_LED_MODE_TORCH,
		0, V4L2_FLASH_LED_MODE_FLASH);
	v4l2_ctrl_new_std(&ov8825->hdl,	&ov8825_ctrl_ops,
			V4L2_CID_FLASH_INTENSITY, OV8825_FLASH_INTENSITY_MIN,
			OV8825_FLASH_INTENSITY_MAX, OV8825_FLASH_INTENSITY_STEP,
			OV8825_FLASH_INTENSITY_DEFAULT);
	v4l2_ctrl_new_std(&ov8825->hdl,	&ov8825_ctrl_ops,
			V4L2_CID_FLASH_TIMEOUT, OV8825_FLASH_TIMEOUT_MIN,
			OV8825_FLASH_TIMEOUT_MAX, OV8825_FLASH_TIMEOUT_STEP,
			OV8825_FLASH_TIMEOUT_DEFAULT);

	if (ov8825->hdl.error) {
		dev_err(&client->dev,
			"Standard menu controls initialization error %d\n",
			ov8825->hdl.error);
		ret = ov8825->hdl.error;
		goto ctrl_hdl_err;
	}

	/* register custom controls */
	for (i = 0; i < ARRAY_SIZE(ov8825_controls); ++i)
		v4l2_ctrl_new_custom(&ov8825->hdl, &ov8825_controls[i], NULL);

	ov8825->subdev.ctrl_handler = &ov8825->hdl;
	if (ov8825->hdl.error) {
		dev_err(&client->dev,
			"Custom controls initialization error %d\n",
			ov8825->hdl.error);
		ret = ov8825->hdl.error;
		goto ctrl_hdl_err;
	}

	ret = ov8825_s_power(subdev, 1);
	if (ret < 0)
		return ret;

	ret = ov8825_video_probe(client);
	if (ret)
		goto vid_probe_fail;

	/* init the sensor here */
	ret = ov8825_init(client);
	if (ret) {
		dev_err(&client->dev, "Failed to initialize sensor\n");
		ret = -EINVAL;
		goto init_fail;
	}

#ifdef CONFIG_VIDEO_A3907
		pr_info("A3907 i2c start");
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

	return ret;

ctrl_hdl_err:
init_fail:
vid_probe_fail:
	v4l2_ctrl_handler_free(&ov8825->hdl);
	kfree(ov8825);
	pr_err("%s failed with ret = %d\n", __func__, ret);
	return ret;

}

static int ov8825_remove(struct i2c_client *client)
{
	struct ov8825 *ov8825 = to_ov8825(client);

	pr_debug(" remove");
	v4l2_device_unregister_subdev(&ov8825->subdev);
#ifdef CONFIG_VIDEO_A3907
	pr_debug("A3907 i2c_unregister_device");
	if (a3907_i2c_client)
		i2c_unregister_device(a3907_i2c_client);
#endif
	client->driver = NULL;
	kfree(ov8825);

	return 0;
}

static const struct i2c_device_id ov8825_id[] = {
	{"ov8825", 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, ov8825_id);

static struct i2c_driver ov8825_i2c_driver = {
	.driver = {
		   .name = "ov8825",
		   },
	.probe = ov8825_probe,
	.remove = ov8825_remove,
	.id_table = ov8825_id,
};

static int __init ov8825_mod_init(void)
{
	return i2c_add_driver(&ov8825_i2c_driver);
}

static void __exit ov8825_mod_exit(void)
{
	i2c_del_driver(&ov8825_i2c_driver);
}

late_initcall(ov8825_mod_init);
module_exit(ov8825_mod_exit);

MODULE_DESCRIPTION("OmniVision OV8825 Camera driver");
MODULE_AUTHOR("Broadcom Corporation");
MODULE_LICENSE("GPL v2");

