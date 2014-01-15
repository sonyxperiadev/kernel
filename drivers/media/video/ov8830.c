/*
 * OmniVision OV8830 sensor driver
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
#include "ov8830.h"
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/init.h>


#ifdef CONFIG_VIDEO_A3907
#include <media/a3907.h>
#endif
#ifdef CONFIG_VIDEO_DW9714
#include <media/dw9714.h>
#endif

#ifdef CONFIG_VIDEO_AS3643
#include "as3643.h"
#endif

#ifdef CONFIG_GPIO_FLASH
#include "flash_gpio.h"
#endif

#define OV8830_DEBUG 1

#ifdef OV8830_DEBUG
	#define SENSORDB(format, ...) printk(KERN_INFO "%s: " format "\n", __func__, __VA_ARGS__)
#else
	#define SENSORDB(...)
#endif

#define OV8830_EXPOSURE_2_CODE(exp, line_time) (((1000 * ((unsigned long)exp)) / (line_time)) << 4)
#define OV8830_CODE_2_EXPOSURE(code, line_time) ((((unsigned long)code) >> 4) * (line_time) / 1000)

#define OV8830_GAIN_2_CODE(gain) ((gain) >> 4)
#define OV8830_CODE_2_GAIN(code) ((code) << 4)

static struct i2c_client *s_ov8830_i2c_client;

/* OV5648 has only one fixed colorspace per pixelcode */
struct ov8830_datafmt {
	enum v4l2_mbus_pixelcode code;
	enum v4l2_colorspace colorspace;
};

static const struct ov8830_datafmt ov8830_fmts[] = {
	/*
	 Order important: first natively supported,
	 second supported with a GPIO extender
	 */
	{V4L2_MBUS_FMT_SRGGB10_1X10, V4L2_COLORSPACE_SRGB},
	{V4L2_MBUS_FMT_SBGGR10_1X10, V4L2_COLORSPACE_SRGB},
	{V4L2_MBUS_FMT_SGBRG10_1X10, V4L2_COLORSPACE_SRGB},
	{V4L2_MBUS_FMT_SGRBG10_1X10, V4L2_COLORSPACE_SRGB},
};

enum ov8830_mode {
	OV8830_MODE_1280x960P30  = 0,
	OV8830_MODE_1920x1080P30 = 1,
	OV8830_MODE_3264x2448P15 = 2,
	OV8830_MODE_MAX          = 3,
};

enum ov8830_state {
	OV8830_STATE_STOP = 0,
	OV8830_STATE_STRM = 1,
	OV8830_STATE_MAX  = 2,
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
	int                  binning_factor;
};


struct sensor_mode ov8830_mode[OV8830_MODE_MAX + 1] = {
	{
		.name           = "1280x960_2lane_30Fps",
		.height         = 960,
		.width          = 1280,
		.hts            = 3608,
		.vts            = 1293,
		.vts_max        = 32767 - 6,
		.line_length_ns = 25230,
		.bayer          = BAYER_BGGR,
		.bpp            = 10,
		.fps            = F24p8(30.0),
		.binning_factor = 1,
	},
	{
		.name           = "1920x1080_2lane_30Fps",
		.height         = 1080,
		.width          = 1920,
		.hts            = 3608,
		.vts            = 1980,
		.vts_max        = 32767 - 6,
		.line_length_ns = 16820,
		.bayer          = BAYER_BGGR,
		.bpp            = 10,
		.fps            = F24p8(30.0),
		.binning_factor = 1,
	},
	{
		.name           = "3264x2448_2lane_15Fps",
		.height         = 2448,
		.width          = 3264,
		.hts            = 3608,
		.vts            = 2556,
		.vts_max        = 32767 - 6,
		.line_length_ns = 25230,
		.bayer          = BAYER_BGGR,
		.bpp            = 10,
		.fps            = F24p8(15.0),
		.binning_factor = 2,
	},
	{
		.name           = "STOPPED",
	}
};

/**
 *struct ov8830_otp - ov8830 OTP format
 *@module_integrator_id: 32-bit module integrator ID number
 *@lens_id: 32-bit lens ID number
 *@rg_ratio: 32-bit AWB R/G channel ratio
 *@bg_ratio: 32-bit AWB B/G channel ratio
 *@user_data: 64-bit OTP user data
 *@light_rg: 32-bit light source R/G ratio
 *@light_bg: 32-bit light source B/G ratio
 *
 * Define a structure for OV8830 OTP calibration data
 */
struct ov8830_otp {
	u8  flag;
	u8  module_integrator_id;
	u8  lens_id;
	u16 rg_ratio;
	u16 bg_ratio;
	u8  user_data[2];
	u16 light_rg;
	u16 light_bg;
};

struct ov8830 {
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
	struct ov8830_otp otp;
	int calibrated;

	int powerdown;
	int position;
	int dac_code;
	int timing_step_us;
	int flying_time;
	int flying_start_time;
	int requested_position;
};

/**
 *struct ov8830_reg - ov8830 register format
 *@reg: 16-bit offset to register
 *@val: 8/16/32-bit register value
 *@length: length of the register
 *
 * Define a structure for OV8830 register initialization values
 */
struct ov8830_reg {
	u16	reg;
	u8	val;
	u8	pad;
};


static const struct ov8830_reg ov8830_regtbl[OV8830_MODE_MAX][1024] = {
	{
		/*@@Global Initial Mclk 13Mhz                                                      */
      /*Slave_ID=0x6c;                                                                   */
		{0x0100, 0x00},/*; software standby                                                                */
		{0x0103, 0x01},/*; software reset                                                                  */
		{0xFFFE, 0x05},/*                ;//delay(5ms)                                                     */
		{0x0102, 0x01},/*                                                                                  */
		/*system control                                                                   */
		{0x3001, 0x2a},/*;drive 2x, d1_frex_in_disable                                                     */
		{0x3002, 0x88},/*; vsync, strobe output enable, href, pclk, frex, SIOD output disable              */
		{0x3005, 0x00},/*                                                                                  */
		{0x3011, 0x41},/*; MIPI 4 lane, MIPI enable                                                        */
		{0x3015, 0x08},/*; MIPI 4 lane on, select MIPI                                                     */
		{0x301b, 0xb4},/*; sclk_bist20, sclk_snr_sync on                                                   */
		{0x301d, 0x02},/*; frex_mask_arb on                                                                */
		{0x3021, 0x00},/*; internal regulator on                                                           */
		{0x3022, 0x00},/*; pad_pixelvdd_sel = 0                                                            */
							/*           ;PLL                                                                   */
		{0x3081, 0x02},/*                                                                                  */
		{0x3083, 0x01},/*                                                                                  */
							/*           ; SCLK = 140Mhz                                                        */
		{0x3090, 0x03},/*; pll2_prediv                                                                     */
		{0x3091, 0x2c},/*; pll2_multiplier                                                                 */
		{0x3092, 0x01},/*; pll2_divs                                                                       */
		{0x3093, 0x00},/*; pll2_sel5                                                                       */
		{0x3094, 0x00},/*;                                                                                 */
		{0x3098, 0x03},/*; pll3_prediv                                                                     */
		{0x3099, 0x1e},/*; pll3_multiplier                                                                 */
		{0x309a, 0x00},/*; pll3_divs                                                                       */
		{0x309b, 0x00},/*; pll3_div                                                                        */
		{0x30a2, 0x01},/*                                                                                  */
		{0x30b0, 0x05},/*                                                                                  */
		{0x30b2, 0x00},/*                                                                                  */
		{0x30b5, 0x04},/*; pll1_op_pix_div                                                                 */
		{0x30b6, 0x01},/*; pll1_op_sys_div                                                                 */
		{0x3104, 0xa1},/*; sclk_sw2pll                                                                     */
		{0x3106, 0x01},/*                                                                                  */
							/*           ; Exposure/Gain                                                        */
		{0x3503, 0x07},/*; AGC manual, AEC manual                                                          */
		{0x3504, 0x00},/*                                                                                  */
		{0x3505, 0x30},/*                                                                                  */
		{0x3506, 0x00},/*; ShortExposure[15:8], for inter line HDR mode                                    */
		{0x3508, 0x80},/*; ShortExposure[7:0]                                                              */
		{0x3509, 0x10},/*; use real gain                                                                   */
		{0x350a, 0x00},/*                                                                                  */
		{0x350b, 0x20},/*                                                                                  */
							/*           ; analog                                                               */
		{0x3600, 0x78},/*                                                                                  */
		{0x3601, 0x0a},/*                                                                                  */
		{0x3602, 0x9c},/*                                                                                  */
		{0x3612, 0x00},/*                                                                                  */
		{0x3604, 0x38},/*                                                                                  */
		{0x3620, 0x64},/*                                                                                  */
		{0x3621, 0xb5},/*                                                                                  */
		{0x3622, 0x07},/*                                                                                  */
		{0x3625, 0x64},/*                                                                                  */
		{0x3630, 0x55},/*                                                                                  */
		{0x3631, 0xd2},/*                                                                                  */
		{0x3632, 0x00},/*                                                                                  */
		{0x3633, 0x34},/*                                                                                  */
		{0x3634, 0x03},/*                                                                                  */
		{0x364d, 0x00},/*                                                                                  */
		{0x364f, 0x00},/*                                                                                  */
		{0x3660, 0x80},/*                                                                                  */
		{0x3662, 0x10},/*                                                                                  */
		{0x3665, 0x00},/*                                                                                  */
		{0x3666, 0x00},/*                                                                                  */
		{0x3667, 0x00},/*                                                                                  */
		{0x366a, 0x80},/*                                                                                  */
		{0x366c, 0x00},/*                                                                                  */
		{0x366d, 0x00},/*                                                                                  */
		{0x366e, 0x00},/*                                                                                  */
		{0x366f, 0x20},/*                                                                                  */
		{0x3680, 0xe0},/*                                                                                  */
		{0x3681, 0x00},/*                                                                                  */
							/*           ; sensor control                                                       */
		{0x3701, 0x14},/*                                                                                  */
		{0x3702, 0xbf},/*                                                                                  */
		{0x3703, 0x8c},/*                                                                                  */
		{0x3704, 0x78},/*                                                                                  */
		{0x3705, 0x02},/*                                                                                  */
		{0x3708, 0xe6},/*                                                                                  */
		{0x3709, 0xc3},/*                                                                                  */
		{0x370a, 0x00},/*                                                                                  */
		{0x370b, 0x20},/*                                                                                  */
		{0x370c, 0x0c},/*                                                                                  */
		{0x370d, 0x11},/*                                                                                  */
		{0x370e, 0x00},/*                                                                                  */
		{0x370f, 0x00},/*                                                                                  */
		{0x3710, 0x00},/*                                                                                  */
		{0x371c, 0x01},/*                                                                                  */
		{0x371f, 0x0c},/*                                                                                  */
		{0x3721, 0x00},/*                                                                                  */
		{0x3724, 0x10},/*                                                                                  */
		{0x3726, 0x00},/*                                                                                  */
		{0x372a, 0x01},/*                                                                                  */
		{0x3730, 0x18},/*                                                                                  */
		{0x3738, 0x22},/*                                                                                  */
		{0x3739, 0x08},/*                                                                                  */
		{0x373a, 0x51},/*                                                                                  */
		{0x373b, 0x02},/*                                                                                  */
		{0x373c, 0x20},/*                                                                                  */
		{0x373f, 0x02},/*                                                                                  */
		{0x3740, 0x42},/*                                                                                  */
		{0x3741, 0x02},/*                                                                                  */
		{0x3742, 0x18},/*                                                                                  */
		{0x3743, 0x01},/*                                                                                  */
		{0x3744, 0x02},/*                                                                                  */
		{0x3747, 0x10},/*                                                                                  */
		{0x374c, 0x04},/*                                                                                  */
		{0x3751, 0xf0},/*                                                                                  */
		{0x3752, 0x00},/*                                                                                  */
		{0x3753, 0x00},/*                                                                                  */
		{0x3754, 0xc0},/*                                                                                  */
		{0x3755, 0x00},/*                                                                                  */
		{0x3756, 0x1a},/*                                                                                  */
		{0x3758, 0x00},/*                                                                                  */
		{0x3759, 0x0f},/*                                                                                  */
		{0x375c, 0x04},/*                                                                                  */
		{0x3767, 0x01},/*                                                                                  */
		{0x376b, 0x44},/*                                                                                  */
		{0x3774, 0x10},/*                                                                                  */
		{0x3776, 0x00},/*                                                                                  */
		{0x377f, 0x08},/*                                                                                  */
							/*           ; PSRAM                                                                */
		{0x3780, 0x22},/*                                                                                  */
		{0x3781, 0x0c},/*                                                                                  */
		{0x3784, 0x2c},/*                                                                                  */
		{0x3785, 0x1e},/*                                                                                  */
		{0x3786, 0x16},/*                                                                                  */
		{0x378f, 0xf5},/*                                                                                  */
		{0x3791, 0xb0},/*                                                                                  */
		{0x3795, 0x00},/*                                                                                  */
		{0x3796, 0x64},/*                                                                                  */
		{0x3797, 0x11},/*                                                                                  */
		{0x3798, 0x30},/*                                                                                  */
		{0x3799, 0x41},/*                                                                                  */
		{0x379a, 0x07},/*                                                                                  */
		{0x379b, 0xb0},/*                                                                                  */
		{0x379c, 0x0c},/*                                                                                  */
							/*           ; Frex control                                                         */
		{0x37c5, 0x00},/*                                                                                  */
		{0x37c6, 0xa0},/*                                                                                  */
		{0x37c7, 0x00},/*                                                                                  */
		{0x37c9, 0x00},/*                                                                                  */
		{0x37ca, 0x00},/*                                                                                  */
		{0x37cb, 0x00},/*                                                                                  */
		{0x37cc, 0x00},/*                                                                                  */
		{0x37cd, 0x00},/*                                                                                  */
		{0x37ce, 0x01},/*                                                                                  */
		{0x37cf, 0x00},/*                                                                                  */
		{0x37d1, 0x01},/*                                                                                  */
		{0x37de, 0x00},/*                                                                                  */
		{0x37df, 0x00},/*                                                                                  */
							/*           ; timing                                                               */
		{0x3800, 0x00},/*; HS = 8                                                                          */
		{0x3801, 0x08},/*; HS                                                                              */
		{0x3802, 0x00},/*; VS = 8                                                                          */
		{0x3803, 0x08},/*; VS                                                                              */
		{0x3804, 0x0c},/*; HE = 3287                                                                       */
		{0x3805, 0xd7},/*; HE                                                                              */
		{0x3806, 0x09},/*; VE = 2471                                                                       */
		{0x3807, 0xa7},/*; VE                                                                              */
		{0x3808, 0x06},/*; HO = 1632                                                                       */
		{0x3809, 0x60},/*; HO                                                                              */
		{0x380a, 0x04},/*; VO = 1224                                                                       */
		{0x380b, 0xc8},/*; VO                                                                              */
		{0x380c, 0x0e},/*; HTS = 3608                                                                      */
		{0x380d, 0x18},/*; HTS                                                                             */
		{0x380e, 0x05},/*; VTS = 1293                                                                      */
		{0x380f, 0x0d},/*; VTS                                                                             */
		{0x3810, 0x00},/*; H OFFSET = 4                                                                    */
		{0x3811, 0x04},/*; H OFFSET                                                                        */
		{0x3812, 0x00},/*; V OFFSET = 4                                                                    */
		{0x3813, 0x04},/*; V OFFSET                                                                        */
		{0x3814, 0x31},/*; X INC                                                                           */
		{0x3815, 0x31},/*; Y INC                                                                           */
		{0x3820, 0x11},/*                                                                                  */
		{0x3821, 0x0f},/*                                                                                  */
		{0x3823, 0x00},/*                                                                                  */
		{0x3824, 0x00},/*                                                                                  */
		{0x3825, 0x00},/*                                                                                  */
		{0x3826, 0x00},/*                                                                                  */
		{0x3827, 0x00},/*                                                                                  */
		{0x382a, 0x04},/*                                                                                  */
		{0x3a04, 0x04},/*                                                                                  */
		{0x3a05, 0xc9},/*                                                                                  */
		{0x3a06, 0x00},/*                                                                                  */
		{0x3a07, 0xf8},/*                                                                                  */
							/*           ;strobe control                                                        */
		{0x3b00, 0x00},/*                                                                                  */
		{0x3b02, 0x00},/*                                                                                  */
		{0x3b03, 0x00},/*                                                                                  */
		{0x3b04, 0x00},/*                                                                                  */
		{0x3b05, 0x00},/*                                                                                  */
							/*           ; OTP                                                                  */
		{0x3d00, 0x00},/*; OTP buffer                                                                      */
		{0x3d01, 0x00},/*                                                                                  */
		{0x3d02, 0x00},/*                                                                                  */
		{0x3d03, 0x00},/*                                                                                  */
		{0x3d04, 0x00},/*                                                                                  */
		{0x3d05, 0x00},/*                                                                                  */
		{0x3d06, 0x00},/*                                                                                  */
		{0x3d07, 0x00},/*                                                                                  */
		{0x3d08, 0x00},/*                                                                                  */
		{0x3d09, 0x00},/*                                                                                  */
		{0x3d0a, 0x00},/*                                                                                  */
		{0x3d0b, 0x00},/*                                                                                  */
		{0x3d0c, 0x00},/*                                                                                  */
		{0x3d0d, 0x00},/*                                                                                  */
		{0x3d0e, 0x00},/*                                                                                  */
		{0x3d0f, 0x00},/*; OTP buffer                                                                      */
		{0x3d80, 0x00},/*; OTP program                                                                     */
		{0x3d81, 0x00},/*; OTP load                                                                        */
		{0x3d84, 0x00},/*; OTP program enable, manual memory bank disable, bank sel 0                      */
							/*           ; BLC                                                                  */
		{0x4000, 0x10},/*; DCBLC auto load mode                                                            */
		{0x4001, 0x04},/*; BLC start line 4                                                                */
		{0x4002, 0x45},/*; BLC auto, reset frame number = 5                                                */
		{0x4004, 0x02},/*; 2 lines for BLC                                                                 */
		{0x4005, 0x18},/*                                                                                  */
		{0x4006, 0x20},/*; DC BLC coefficient                                                              */
		{0x4008, 0x20},/*; First part BLC calculation start line address = 2, DC BLC on                    */
		{0x4009, 0x10},/*; BLC target                                                                      */
		{0x404f, 0xa0},/*                                                                                  */
		{0x4100, 0x10},/*                                                                                  */
		{0x4101, 0x12},/*                                                                                  */
		{0x4102, 0x24},/*                                                                                  */
		{0x4103, 0x00},/*                                                                                  */
		{0x4104, 0x5b},/*                                                                                  */
							/*           ;format control                                                        */
		{0x4307, 0x30},/*; embed_line_st = 3                                                               */
		{0x4315, 0x00},/*; Vsync trigger signal to Vsync output delay[15:8]                                */
		{0x4511, 0x05},/*                                                                                  */
		{0x4512, 0x00},/*; vertical sum                                                                    */
							/*           ;MIPI control                                                          */
		{0x4805, 0x21},/*                                                                                  */
		{0x4806, 0x00},/*                                                                                  */
		{0x481f, 0x36},/*                                                                                  */
		{0x4831, 0x6c},/*                                                                                  */
							/*           ; LVDS                                                                 */
		{0x4a00, 0xaa},/*; SYNC code enable when only 1 lane, Bit swap, SAV first enable                   */
		{0x4a03, 0x01},/*; Dummy data0[7:0]                                                                */
		{0x4a05, 0x08},/*; Dummy data1[7:0]                                                                */
		{0x4a0a, 0x88},/*                                                                                  */
		{0x4d03, 0xbb},/*                                                                                  */
							/*           ; ISP                                                                  */
		{0x5000, 0x06},/*; LENC off, BPC on, WPC on                                                        */
		{0x5001, 0x01},/*; MWB on                                                                          */
		{0x5002, 0x80},/*; scale on                                                                        */
		{0x5003, 0x20},/*                                                                                  */
		{0x5013, 0x00},/*; LSB disable                                                                     */
		{0x5046, 0x4a},/*; ISP SOF sel - VSYNC                                                             */
							/*           ; DPC                                                                  */
		{0x5780, 0x1c},/*                                                                                  */
		{0x5786, 0x20},/*                                                                                  */
		{0x5787, 0x10},/*                                                                                  */
		{0x5788, 0x18},/*                                                                                  */
		{0x578a, 0x04},/*                                                                                  */
		{0x578b, 0x02},/*                                                                                  */
		{0x578c, 0x02},/*                                                                                  */
		{0x578e, 0x06},/*                                                                                  */
		{0x578f, 0x02},/*                                                                                  */
		{0x5790, 0x02},/*                                                                                  */
		{0x5791, 0xff},/*                                                                                  */
		{0x5a08, 0x02},/*; window control                                                                  */
		{0x5e00, 0x00},/*; color bar off                                                                   */
		{0x5e10, 0x0c},/*                                                                                  */
		{0x5000, 0x06},/*; lenc off, bpc on, wpc on                                                        */
							/*           ; MWB                                                                  */
		{0x5001, 0x01},/*; MWB on                                                                          */
		{0x3400, 0x04},/*; red gain h                                                                      */
		{0x3401, 0x00},/*; red gain l                                                                      */
		{0x3402, 0x04},/*; green gain h                                                                    */
		{0x3403, 0x00},/*; green gain l                                                                    */
		{0x3404, 0x04},/*; blue gain h                                                                     */
		{0x3405, 0x00},/*; blue gain l                                                                     */
		{0x3406, 0x01},/*; MWB on                                                                          */
							/*           ; BLC                                                                  */
		{0x4000, 0x10},/*; BLC on, BLC window as BLC, DCBLC auto load                                      */
		{0x4002, 0x45},/*; Format change trigger off, auto on, reset frame number = 5                      */
		{0x4005, 0x18},/*; no black line output, blc_man_1_en, BLC after reset, then stop, BLC triggered by*/
							/*           gain change                                                            */
		{0x4008, 0x20},/*;DCBLC is ON                                                                      */
		{0x4009, 0x10},/*; BLC target                                                                      */
		{0x3503, 0x07},/*; AEC manual, AGC manual                                                          */
		{0x3500, 0x00},/*; Exposure[19:16]                                                                 */
		{0x3501, 0x19},/*; Exposure[15:8]                                                                  */
		{0x3502, 0x00},/*; Exposure[7:0]                                                                   */
		{0x350b, 0x20},/*; Gain[7:0]                                                                       */
							/*           ; MIPI data rate = 640Mbps                                             */
		{0x30b3, 0x6f},/*; pll1_multiplier                                                                 */
		{0x30b4, 0x03},/*; pll1_prediv                                                                     */
		{0x30b5, 0x04},/*; pll1_op_pix_div                                                                 */
		{0x30b6, 0x01},/*; pll1_op_sys_div                                                                 */
		{0x4837, 0x0b},/*                                                                                  */
							/*           @@Preview Raw 10bit 1632*1224 30fps 2lane 712M bps/lane  BGGR          */
		{0x0100, 0x00},/*; software standby                                                                */
		{0x3708, 0xe6},/*; sensor control                                                                  */
		{0x3709, 0xc3},/*; sensor control                                                                  */
		{0x3800, 0x00},/*; HS = 8                                                                          */
		{0x3801, 0x08},/*; HS                                                                              */
		{0x3802, 0x00},/*; VS = 8                                                                          */
		{0x3803, 0x08},/*; VS                                                                              */
		{0x3804, 0x0c},/*; HE = 3287                                                                       */
		{0x3805, 0xd7},/*; HE                                                                              */
		{0x3806, 0x09},/*; VE = 2471                                                                       */
		{0x3807, 0xa7},/*; VE                                                                              */
		{0x3808, 0x05},/*; HO = 1280                                                                       */
		{0x3809, 0x00},/*; HO                                                                              */
		{0x380a, 0x03},/*; VO = 960                                                                        */
		{0x380b, 0xc0},/*; VO                                                                              */
		{0x380c, 0x0e},/*; HTS = 3608                                                                      */
		{0x380d, 0x18},/*; HTS                                                                             */
		{0x380e, 0x05},/*; VTS = 1293                                                                      */
		{0x380f, 0x0d},/*; VTS                                                                             */
		{0x3810, 0x00},/*; H OFFSET = 4                                                                    */
		{0x3811, 0x04},/*; H OFFSET                                                                        */
		{0x3812, 0x00},/*; V OFFSET = 4                                                                    */
		{0x3813, 0x04},/*; V OFFSET                                                                        */
		{0x3814, 0x31},/*; X INC                                                                           */
		{0x3815, 0x31},/*; Y INC                                                                           */
		{0x3820, 0x11},/*                                                                                  */
		{0x3821, 0x0f},/*                                                                                  */
		{0x3a04, 0x04},/*                                                                                  */
		{0x3a05, 0xc9},/*                                                                                  */
		{0x4004, 0x02},/*                                                                                  */
		{0x4005, 0x18},/*                                                                                  */
		{0x4512, 0x00},/*; vertical sum                                                                    */
		{0x3011, 0x21},/*; MIPI 2 lane, MIPI enable                                                        */
		{0x3015, 0xc8},/*; MIPI 2 lane on, select MIPI                                                     */
							/*           ; SCLK = 140Mhz                                                        */
		{0x3662, 0x10},/*                                                                                  */
		{0x3090, 0x02},/*; pll2_prediv                                                                     */
		{0x3091, 0x2c},/*; pll2_multiplier                                                                 */
		{0x3092, 0x01},/*; pll2_divs                                                                       */
		{0x3093, 0x00},/*; pll2_seld5                                                                      */
		{0x3094, 0x00},/*                                                                                  */
		{0x3098, 0x02},/*                                                                                  */
		{0x3099, 0x13},/*                                                                                  */
		{0x309a, 0x00},/*                                                                                  */
		{0x309b, 0x00},/*                                                                                  */
		{0x309c, 0x01},/*                                                                                  */
							/*           ; MIPI data rate = 624Mbps                                             */
		{0x30b3, 0x6f},/*; pll1_multiplier                                                                 */
		{0x30b4, 0x02},/*; pll1_prediv                                                                     */
		{0x30b5, 0x04},/*; pll1_op_pix_div                                                                 */
		{0x30b6, 0x01},/*; pll1_op_sys_div                                                                 */
		{0x4837, 0x0b},/*; MIPI global timing                                                              */
		{0x0100, 0x01},/*; wake up                                                                         */
		{0xFFFF, 0x00}	/* end of the list */
	},
	{
		/* mode 1920X1080 */
		/*@@Global Initial Mclk 13Mhz*/
		/*Slave_ID=0x6c;*/
		{0x0100, 0x00},/*; software standby*/
		{0x0103, 0x01},/*; software reset                                                    */
		{0xFFFE, 0x05},/*                ;//delay(5ms)                                       */
		{0x0102, 0x01},/*                                                                    */
										/*system control                                                     */
		{0x3001, 0x2a},/*;drive 2x, d1_frex_in_disable                                       */
		{0x3002, 0x88},/*; vsync, strobe output enable, href, pclk, frex, SIOD output disable*/
		{0x3005, 0x00},/*                                                                    */
		{0x3011, 0x41},/*; MIPI 4 lane, MIPI enable                                          */
		{0x3015, 0x08},/*; MIPI 4 lane on, select MIPI                                       */
		{0x301b, 0xb4},/*; sclk_bist20, sclk_snr_sync on                                     */
		{0x301d, 0x02},/*; frex_mask_arb on                                                  */
		{0x3021, 0x00},/*; internal regulator on                                             */
		{0x3022, 0x00},/*; pad_pixelvdd_sel = 0                                              */
										/*                 ;PLL                                              */
		{0x3081, 0x02},/*                                                                    */
		{0x3083, 0x01},/*                                                                    */
										/*                 ; SCLK = 140Mhz                                   */
		{0x3090, 0x03},/*; pll2_prediv                                                       */
		{0x3091, 0x23},/*; pll2_multiplier                                                   */
		{0x3092, 0x01},/*; pll2_divs                                                         */
		{0x3093, 0x00},/*; pll2_sel5                                                         */
		{0x3094, 0x00},/*;                                                                   */
		{0x3098, 0x03},/*; pll3_prediv                                                       */
		{0x3099, 0x1e},/*; pll3_multiplier                                                   */
		{0x309a, 0x00},/*; pll3_divs                                                         */
		{0x309b, 0x00},/*; pll3_div                                                          */
		{0x30a2, 0x01},/*                                                                    */
		{0x30b0, 0x05},/*                                                                    */
		{0x30b2, 0x00},/*                                                                    */
		{0x30b5, 0x04},/*; pll1_op_pix_div                                                   */
		{0x30b6, 0x01},/*; pll1_op_sys_div                                                   */
		{0x3104, 0xa1},/*; sclk_sw2pll                                                       */
		{0x3106, 0x01},/*                                                                    */
										/*                 ; Exposure/Gain                                   */
		{0x3503, 0x07},/*; AGC manual, AEC manual                                            */
		{0x3504, 0x00},/*                                                                    */
		{0x3505, 0x30},/*                                                                    */
		{0x3506, 0x00},/*; ShortExposure[15:8], for inter line HDR mode                      */
		{0x3508, 0x80},/*; ShortExposure[7:0]                                                */
		{0x3509, 0x10},/*; use real gain                                                     */
		{0x350a, 0x00},/*                                                                    */
		{0x350b, 0x20},/*                                                                    */
										/*                 ; analog                                          */
		{0x3600, 0x78},/*                                                                    */
		{0x3601, 0x0a},/*                                                                    */
		{0x3602, 0x9c},/*                                                                    */
		{0x3612, 0x00},/*                                                                    */
		{0x3604, 0x38},/*                                                                    */
		{0x3620, 0x64},/*                                                                    */
		{0x3621, 0xb5},/*                                                                    */
		{0x3622, 0x07},/*                                                                    */
		{0x3625, 0x64},/*                                                                    */
		{0x3630, 0x55},/*                                                                    */
		{0x3631, 0xd2},/*                                                                    */
		{0x3632, 0x00},/*                                                                    */
		{0x3633, 0x34},/*                                                                    */
		{0x3634, 0x03},/*                                                                    */
		{0x364d, 0x00},/*                                                                    */
		{0x364f, 0x00},/*                                                                    */
		{0x3660, 0x80},/*                                                                    */
		{0x3662, 0x10},/*                                                                    */
		{0x3665, 0x00},/*                                                                    */
		{0x3666, 0x00},/*                                                                    */
		{0x3667, 0x00},/*                                                                    */
		{0x366a, 0x80},/*                                                                    */
		{0x366c, 0x00},/*                                                                    */
		{0x366d, 0x00},/*                                                                    */
		{0x366e, 0x00},/*                                                                    */
		{0x366f, 0x20},/*                                                                    */
		{0x3680, 0xe0},/*                                                                    */
		{0x3681, 0x00},/*                                                                    */
										/*                 ; sensor control                                  */
		{0x3701, 0x14},/*                                                                    */
		{0x3702, 0xbf},/*                                                                    */
		{0x3703, 0x8c},/*                                                                    */
		{0x3704, 0x78},/*                                                                    */
		{0x3705, 0x02},/*                                                                    */
		{0x3708, 0xe6},/*                                                                    */
		{0x3709, 0xc3},/*                                                                    */
		{0x370a, 0x00},/*                                                                    */
		{0x370b, 0x20},/*                                                                    */
		{0x370c, 0x0c},/*                                                                    */
		{0x370d, 0x11},/*                                                                    */
		{0x370e, 0x00},/*                                                                    */
		{0x370f, 0x00},/*                                                                    */
		{0x3710, 0x00},/*                                                                    */
		{0x371c, 0x01},/*                                                                    */
		{0x371f, 0x0c},/*                                                                    */
		{0x3721, 0x00},/*                                                                    */
		{0x3724, 0x10},/*                                                                    */
		{0x3726, 0x00},/*                                                                    */
		{0x372a, 0x01},/*                                                                    */
		{0x3730, 0x18},/*                                                                    */
		{0x3738, 0x22},/*                                                                    */
		{0x3739, 0x08},/*                                                                    */
		{0x373a, 0x51},/*                                                                    */
		{0x373b, 0x02},/*                                                                    */
		{0x373c, 0x20},/*                                                                    */
		{0x373f, 0x02},/*                                                                    */
		{0x3740, 0x42},/*                                                                    */
		{0x3741, 0x02},/*                                                                    */
		{0x3742, 0x18},/*                                                                    */
		{0x3743, 0x01},/*                                                                    */
		{0x3744, 0x02},/*                                                                    */
		{0x3747, 0x10},/*                                                                    */
		{0x374c, 0x04},/*                                                                    */
		{0x3751, 0xf0},/*                                                                    */
		{0x3752, 0x00},/*                                                                    */
		{0x3753, 0x00},/*                                                                    */
		{0x3754, 0xc0},/*                                                                    */
		{0x3755, 0x00},/*                                                                    */
		{0x3756, 0x1a},/*                                                                    */
		{0x3758, 0x00},/*                                                                    */
		{0x3759, 0x0f},/*                                                                    */
		{0x375c, 0x04},/*                                                                    */
		{0x3767, 0x01},/*                                                                    */
		{0x376b, 0x44},/*                                                                    */
		{0x3774, 0x10},/*                                                                    */
		{0x3776, 0x00},/*                                                                    */
		{0x377f, 0x08},/*                                                                    */
										/*                 ; PSRAM                                           */
		{0x3780, 0x22},/*                                                                    */
		{0x3781, 0x0c},/*                                                                    */
		{0x3784, 0x2c},/*                                                                    */
		{0x3785, 0x1e},/*                                                                    */
		{0x3786, 0x16},/*                                                                    */
		{0x378f, 0xf5},/*                                                                    */
		{0x3791, 0xb0},/*                                                                    */
		{0x3795, 0x00},/*                                                                    */
		{0x3796, 0x64},/*                                                                    */
		{0x3797, 0x11},/*                                                                    */
		{0x3798, 0x30},/*                                                                    */
		{0x3799, 0x41},/*                                                                    */
		{0x379a, 0x07},/*                                                                    */
		{0x379b, 0xb0},/*                                                                    */
		{0x379c, 0x0c},/*                                                                    */
										/*                 ; Frex control                                    */
		{0x37c5, 0x00},/*                                                                    */
		{0x37c6, 0xa0},/*                                                                    */
		{0x37c7, 0x00},/*                                                                    */
		{0x37c9, 0x00},/*                                                                    */
		{0x37ca, 0x00},/*                                                                    */
		{0x37cb, 0x00},/*                                                                    */
		{0x37cc, 0x00},/*                                                                    */
		{0x37cd, 0x00},/*                                                                    */
		{0x37ce, 0x01},/*                                                                    */
		{0x37cf, 0x00},/*                                                                    */
		{0x37d1, 0x01},/*                                                                    */
		{0x37de, 0x00},/*                                                                    */
		{0x37df, 0x00},/*                                                                    */
										/*                 ; timing                                          */
		{0x3800, 0x00},/*; HS = 8                                                            */
		{0x3801, 0x08},/*; HS                                                                */
		{0x3802, 0x00},/*; VS = 8                                                            */
		{0x3803, 0x08},/*; VS                                                                */
		{0x3804, 0x0c},/*; HE = 3287                                                         */
		{0x3805, 0xd7},/*; HE                                                                */
		{0x3806, 0x09},/*; VE = 2471                                                         */
		{0x3807, 0xa7},/*; VE                                                                */
		{0x3808, 0x06},/*; HO = 1632                                                         */
		{0x3809, 0x60},/*; HO                                                                */
		{0x380a, 0x04},/*; VO = 1224                                                         */
		{0x380b, 0xc8},/*; VO                                                                */
		{0x380c, 0x0e},/*; HTS = 3608                                                        */
		{0x380d, 0x18},/*; HTS                                                               */
		{0x380e, 0x05},/*; VTS = 1293                                                        */
		{0x380f, 0x0d},/*; VTS                                                               */
		{0x3810, 0x00},/*; H OFFSET = 4                                                      */
		{0x3811, 0x04},/*; H OFFSET                                                          */
		{0x3812, 0x00},/*; V OFFSET = 4                                                      */
		{0x3813, 0x04},/*; V OFFSET                                                          */
		{0x3814, 0x31},/*; X INC                                                             */
		{0x3815, 0x31},/*; Y INC                                                             */
		{0x3820, 0x11},/*                                                                    */
		{0x3821, 0x0f},/*                                                                    */
		{0x3823, 0x00},/*                                                                    */
		{0x3824, 0x00},/*                                                                    */
		{0x3825, 0x00},/*                                                                    */
		{0x3826, 0x00},/*                                                                    */
		{0x3827, 0x00},/*                                                                    */
		{0x382a, 0x04},/*                                                                    */
		{0x3a04, 0x04},/*                                                                    */
		{0x3a05, 0xc9},/*                                                                    */
		{0x3a06, 0x00},/*                                                                    */
		{0x3a07, 0xf8},/*                                                                    */
										/*                 ;strobe control                                   */
		{0x3b00, 0x00},/*                                                                    */
		{0x3b02, 0x00},/*                                                                    */
		{0x3b03, 0x00},/*                                                                    */
		{0x3b04, 0x00},/*                                                                    */
		{0x3b05, 0x00},/*                                                                    */
										/*                 ; OTP                                             */
		{0x3d00, 0x00},/*; OTP buffer                                                        */
		{0x3d01, 0x00},/*                                                                    */
		{0x3d02, 0x00},/*                                                                    */
		{0x3d03, 0x00},/*                                                                    */
		{0x3d04, 0x00},/*                                                                    */
		{0x3d05, 0x00},/*                                                                    */
		{0x3d06, 0x00},/*                                                                    */
		{0x3d07, 0x00},/*                                                                    */
		{0x3d08, 0x00},/*                                                                    */
		{0x3d09, 0x00},/*                                                                    */
		{0x3d0a, 0x00},/*                                                                    */
		{0x3d0b, 0x00},/*                                                                    */
		{0x3d0c, 0x00},/*                                                                    */
		{0x3d0d, 0x00},/*                                                                    */
		{0x3d0e, 0x00},/*                                                                    */
		{0x3d0f, 0x00},/*; OTP buffer                                                        */
		{0x3d80, 0x00},/*; OTP program                                                       */
		{0x3d81, 0x00},/*; OTP load                                                          */
		{0x3d84, 0x00},/*; OTP program enable, manual memory bank disable, bank sel 0        */
										/*                 ; BLC                                             */
		{0x4000, 0x10},/*; DCBLC auto load mode                                              */
		{0x4001, 0x04},/*; BLC start line 4                                                  */
		{0x4002, 0x45},/*; BLC auto, reset frame number = 5                                  */
		{0x4004, 0x02},/*; 2 lines for BLC                                                   */
		{0x4005, 0x18},/*                                                                    */
		{0x4006, 0x20},/*; DC BLC coefficient                                                */
		{0x4008, 0x20},/*; First part BLC calculation start line address = 2, DC BLC on      */
		{0x4009, 0x10},/*; BLC target                                                        */
		{0x404f, 0xa0},/*                                                                    */
		{0x4100, 0x10},/*                                                                    */
		{0x4101, 0x12},/*                                                                    */
		{0x4102, 0x24},/*                                                                    */
		{0x4103, 0x00},/*                                                                    */
		{0x4104, 0x5b},/*                                                                    */
										/*                 ;format control                                   */
		{0x4307, 0x30},/*; embed_line_st = 3                                                 */
		{0x4315, 0x00},/*; Vsync trigger signal to Vsync output delay[15:8]                  */
		{0x4511, 0x05},/*                                                                    */
		{0x4512, 0x00},/*; vertical sum                                                      */
										/*                 ;MIPI control                                     */
		{0x4805, 0x21},/*                                                                    */
		{0x4806, 0x00},/*                                                                    */
		{0x481f, 0x36},/*                                                                    */
		{0x4831, 0x6c},/*                                                                    */
										/*                 ; LVDS                                            */
		{0x4a00, 0xaa},/*; SYNC code enable when only 1 lane, Bit swap, SAV first enable     */
		{0x4a03, 0x01},/*; Dummy data0[7:0]                                                  */
		{0x4a05, 0x08},/*; Dummy data1[7:0]                                                  */
		{0x4a0a, 0x88},/*                                                                    */
		{0x4d03, 0xbb},/*                                                                    */
										/*                 ; ISP                                             */
		{0x5000, 0x06},/*; LENC off, BPC on, WPC on                                          */
		{0x5001, 0x01},/*; MWB on                                                            */
		{0x5002, 0x80},/*; scale on                                                          */
		{0x5003, 0x20},/*                                                                    */
		{0x5013, 0x00},/*; LSB disable                                                       */
		{0x5046, 0x4a},/*; ISP SOF sel - VSYNC                                               */
										/*                 ; DPC                                             */
		{0x5780, 0x1c},/*                                                                    */
		{0x5786, 0x20},/*                                                                    */
		{0x5787, 0x10},/*                                                                    */
		{0x5788, 0x18},/*                                                                    */
		{0x578a, 0x04},/*                                                                    */
		{0x578b, 0x02},/*                                                                    */
		{0x578c, 0x02},/*                                                                    */
		{0x578e, 0x06},/*                                                                    */
		{0x578f, 0x02},/*                                                                    */
		{0x5790, 0x02},/*                                                                    */
		{0x5791, 0xff},/*                                                                    */
		{0x5a08, 0x02},/*; window control                                                    */
		{0x5e00, 0x00},/*; color bar off                                                     */
		{0x5e10, 0x0c},/*                                                                    */
		{0x5000, 0x06},/*; lenc off, bpc on, wpc on                                          */
										/*                 ; MWB                                             */
		{0x5001, 0x01},/*; MWB on                                                            */
		{0x3400, 0x04},/*; red gain h                                                        */
		{0x3401, 0x00},/*; red gain l                                                        */
		{0x3402, 0x04},/*; green gain h                                                      */
		{0x3403, 0x00},/*; green gain l                                                      */
		{0x3404, 0x04},/*; blue gain h                                                       */
		{0x3405, 0x00},/*; blue gain l                                                       */
		{0x3406, 0x01},/*; MWB on                                                            */
		/* BLC*/
		{0x4000, 0x10},/*; BLC on, BLC window as BLC, DCBLC auto load   */
		{0x4002, 0x45},/*; Format change trigger off, auto on, reset frame number = 5*/
		{0x4005, 0x18},/*; no black line output, blc_man_1_en, BLC after reset, then stop, BLC triggered by*/
		/*gain change*/
		{0x4008, 0x20},/*;DCBLC is ON*/
		{0x4009, 0x10},/*; BLC target*/
		{0x3503, 0x07},/*; AEC manual, AGC manual */
		{0x3500, 0x00},/*; Exposure[19:16]*/
		{0x3501, 0x19},/*; Exposure[15:8] */
		{0x3502, 0x00},/*; Exposure[7:0]  */
		{0x350b, 0x20},/*; Gain[7:0]      */
		/*MIPI data rate = 640Mbps*/
		{0x30b3, 0x6f},/*; pll1_multiplier*/
		{0x30b4, 0x03},/*; pll1_prediv    */
		{0x30b5, 0x04},/*; pll1_op_pix_div*/
		{0x30b6, 0x01},/*; pll1_op_sys_div*/
		{0x4837, 0x0b},
		/*@@Raw 10bit 1920*1080 30fps 2lane 640M bps/lane*/
		{0x0100, 0x00},/*software standby*/
		{0x3708, 0xe3},/*sensor control  */
		{0x3709, 0xc3},/*sensor control  */
		{0x3800, 0x00},/*HS = 12   */
		{0x3801, 0x0c},/*HS        */
		{0x3802, 0x01},/*VS = 320  */
		{0x3803, 0x40},/*VS        */
		{0x3804, 0x0c},/*HE = 3283 */
		{0x3805, 0xd3},/*HE        */
		{0x3806, 0x08},/*VE = 2163 */
		{0x3807, 0x73},/*VE        */
		{0x3808, 0x07},/*HO = 1920 */
		{0x3809, 0x80},/*HO        */
		{0x380a, 0x04},/*VO = 1080 */
		{0x380b, 0x38},/*VO        */
		{0x380c, 0x0e},/*HTS = 3608*/
		{0x380d, 0x18},/*HTS       */
		{0x380e, 0x07},/*VTS =     */
		{0x380f, 0xbc},/*VTS       */
		{0x3810, 0x00},/*H OFFSET  */
		{0x3811, 0x04},/*H OFFSET  */
		{0x3812, 0x00},/*V OFFSET  */
		{0x3813, 0x04},/*V OFFSET  */
		{0x3814, 0x11},/*X INC     */
		{0x3815, 0x11},/*Y INC     */
		{0x3820, 0x10},
		{0x3821, 0x0e},
		{0x3a04, 0x07},
		{0x3a05, 0x49},
		{0x4004, 0x08},
		{0x4005, 0x18},
		{0x4512, 0x01},/*vertical average           */
		{0x3011, 0x21},/*MIPI 2 lane, MIPI enable   */
		{0x3015, 0xc8},/*MIPI 2 lane on, select MIPI*/
		/*K = 214Mhz*/
		{0x3662, 0x10},
		{0x3090, 0x01},/*pll2_prediv    */
		{0x3091, 0x21},/*pll2_multiplier*/
		{0x3092, 0x01},/*pll2_divs      */
		{0x3093, 0x00},/*pll2_seld5     */
		{0x3094, 0x00},
		{0x3098, 0x02},
		{0x3099, 0x13},
		{0x309a, 0x00},
		{0x309b, 0x00},
		{0x309c, 0x01},
		/*0xIPI data rate = 624Mbps*/
		{0x30b3, 0x6f},/*pll1_multiplier   */
		{0x30b4, 0x02},/*pll1_prediv       */
		{0x30b5, 0x04},/*pll1_op_pix_div   */
		{0x30b6, 0x01},/*pll1_op_sys_div   */
		{0x4837, 0x0b},/*MIPI global timing*/
		{0x0100, 0x01},/*wake up           */

		{0xFFFF, 0x00},
	},
	/* mode 3264X2448 */
	{
		/*@@Global Initial Mclk 13Mhz*/
    /*Slave_ID=0x6c;*/
		{0x0100, 0x00},/*; software standby */
		{0x0103, 0x01},/*; software reset   */
		{0xFFFE, 0x05},/*delay(5ms)*/
		{0x0102, 0x01},
										/*system control*/
		{0x3001, 0x2a},/*;drive 2x, d1_frex_in_disable*/
		{0x3002, 0x88},/*vsync, strobe output enable, href, pclk, frex, SIOD output disable*/
		{0x3005, 0x00},
		{0x3011, 0x41},/*MIPI 4 lane, MIPI enable      */
		{0x3015, 0x08},/*MIPI 4 lane on, select MIPI   */
		{0x301b, 0xb4},/*sclk_bist20, sclk_snr_sync on */
		{0x301d, 0x02},/*frex_mask_arb on*/
		{0x3021, 0x00},/*internal regulator on*/
		{0x3022, 0x00},/*pad_pixelvdd_sel = 0 */
		/*PLL*/
		{0x3081, 0x02},
		{0x3083, 0x01},
		/*SCLK = 140Mhz*/
		{0x3090, 0x03},/*; pll2_prediv                                                                     */
		{0x3091, 0x23},/*; pll2_multiplier                                                                 */
		{0x3092, 0x01},/*; pll2_divs                                                                       */
		{0x3093, 0x00},/*; pll2_sel5                                                                       */
		{0x3094, 0x00},
		{0x3098, 0x03},/*; pll3_prediv                                                                     */
		{0x3099, 0x1e},/*; pll3_multiplier                                                                 */
		{0x309a, 0x00},/*; pll3_divs                                                                       */
		{0x309b, 0x00},/*; pll3_div                                                                        */
		{0x30a2, 0x01},
		{0x30b0, 0x05},
		{0x30b2, 0x00},
		{0x30b5, 0x04},/*; pll1_op_pix_div                                                                 */
		{0x30b6, 0x01},/*; pll1_op_sys_div                                                                 */
		{0x3104, 0xa1},/*; sclk_sw2pll                                                                     */
		{0x3106, 0x01},
		/*Exposure/Gain*/
		{0x3503, 0x07},/*; AGC manual, AEC manual*/
		{0x3504, 0x00},
		{0x3505, 0x30},
		{0x3506, 0x00},/*; ShortExposure[15:8], for inter line HDR mode*/
		{0x3508, 0x80},/*; ShortExposure[7:0]*/
		{0x3509, 0x10},/*; use real gain     */
		{0x350a, 0x00},
		{0x350b, 0x20},
		/*analog*/
		{0x3600, 0x78},
		{0x3601, 0x0a},
		{0x3602, 0x9c},
		{0x3612, 0x00},
		{0x3604, 0x38},
		{0x3620, 0x64},
		{0x3621, 0xb5},
		{0x3622, 0x07},
		{0x3625, 0x64},
		{0x3630, 0x55},
		{0x3631, 0xd2},
		{0x3632, 0x00},
		{0x3633, 0x34},
		{0x3634, 0x03},
		{0x364d, 0x00},
		{0x364f, 0x00},
		{0x3660, 0x80},
		{0x3662, 0x10},
		{0x3665, 0x00},
		{0x3666, 0x00},
		{0x3667, 0x00},
		{0x366a, 0x80},
		{0x366c, 0x00},
		{0x366d, 0x00},
		{0x366e, 0x00},
		{0x366f, 0x20},
		{0x3680, 0xe0},
		{0x3681, 0x00},
		/*sensor control*/
		{0x3701, 0x14},
		{0x3702, 0xbf},
		{0x3703, 0x8c},
		{0x3704, 0x78},
		{0x3705, 0x02},
		{0x3708, 0xe6},
		{0x3709, 0xc3},
		{0x370a, 0x00},
		{0x370b, 0x20},
		{0x370c, 0x0c},
		{0x370d, 0x11},
		{0x370e, 0x00},
		{0x370f, 0x00},
		{0x3710, 0x00},
		{0x371c, 0x01},
		{0x371f, 0x0c},
		{0x3721, 0x00},
		{0x3724, 0x10},
		{0x3726, 0x00},
		{0x372a, 0x01},
		{0x3730, 0x18},
		{0x3738, 0x22},
		{0x3739, 0x08},
		{0x373a, 0x51},
		{0x373b, 0x02},
		{0x373c, 0x20},
		{0x373f, 0x02},
		{0x3740, 0x42},
		{0x3741, 0x02},
		{0x3742, 0x18},
		{0x3743, 0x01},
		{0x3744, 0x02},
		{0x3747, 0x10},
		{0x374c, 0x04},
		{0x3751, 0xf0},
		{0x3752, 0x00},
		{0x3753, 0x00},
		{0x3754, 0xc0},
		{0x3755, 0x00},
		{0x3756, 0x1a},
		{0x3758, 0x00},
		{0x3759, 0x0f},
		{0x375c, 0x04},
		{0x3767, 0x01},
		{0x376b, 0x44},
		{0x3774, 0x10},
		{0x3776, 0x00},
		{0x377f, 0x08},
		/*PSRAM*/
		{0x3780, 0x22},
		{0x3781, 0x0c},
		{0x3784, 0x2c},
		{0x3785, 0x1e},
		{0x3786, 0x16},
		{0x378f, 0xf5},
		{0x3791, 0xb0},
		{0x3795, 0x00},
		{0x3796, 0x64},
		{0x3797, 0x11},
		{0x3798, 0x30},
		{0x3799, 0x41},
		{0x379a, 0x07},
		{0x379b, 0xb0},
		{0x379c, 0x0c},
		/*Frex control*/
		{0x37c5, 0x00},
		{0x37c6, 0xa0},
		{0x37c7, 0x00},
		{0x37c9, 0x00},
		{0x37ca, 0x00},
		{0x37cb, 0x00},
		{0x37cc, 0x00},
		{0x37cd, 0x00},
		{0x37ce, 0x01},
		{0x37cf, 0x00},
		{0x37d1, 0x01},
		{0x37de, 0x00},
		{0x37df, 0x00},
		/*timing*/
		{0x3800, 0x00},/*; HS = 8   */
		{0x3801, 0x08},/*; HS       */
		{0x3802, 0x00},/*; VS = 8   */
		{0x3803, 0x08},/*; VS       */
		{0x3804, 0x0c},/*; HE = 3287*/
		{0x3805, 0xd7},/*; HE       */
		{0x3806, 0x09},/*; VE = 2471*/
		{0x3807, 0xa7},/*; VE       */
		{0x3808, 0x06},/*; HO = 1632*/
		{0x3809, 0x60},/*; HO          */
		{0x380a, 0x04},/*; VO = 1224   */
		{0x380b, 0xc8},/*; VO          */
		{0x380c, 0x0e},/*; HTS = 3608  */
		{0x380d, 0x18},/*; HTS         */
		{0x380e, 0x05},/*; VTS = 1293  */
		{0x380f, 0x0d},/*; VTS         */
		{0x3810, 0x00},/*; H OFFSET = 4*/
		{0x3811, 0x04},/*; H OFFSET    */
		{0x3812, 0x00},/*; V OFFSET = 4*/
		{0x3813, 0x04},/*; V OFFSET    */
		{0x3814, 0x31},/*; X INC       */
		{0x3815, 0x31},/*; Y INC       */
		{0x3820, 0x11},
		{0x3821, 0x0f},
		{0x3823, 0x00},
		{0x3824, 0x00},
		{0x3825, 0x00},
		{0x3826, 0x00},
		{0x3827, 0x00},
		{0x382a, 0x04},
		{0x3a04, 0x04},
		{0x3a05, 0xc9},
		{0x3a06, 0x00},
		{0x3a07, 0xf8},
		/*strobe control*/
		{0x3b00, 0x00},
		{0x3b02, 0x00},
		{0x3b03, 0x00},
		{0x3b04, 0x00},
		{0x3b05, 0x00},
		/*OTP*/
		{0x3d00, 0x00},/*; OTP buffer*/
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
		{0x3d0f, 0x00},/*; OTP buffer */
		{0x3d80, 0x00},/*; OTP program*/
		{0x3d81, 0x00},/*; OTP load   */
		{0x3d84, 0x00},/*; OTP program enable, manual memory bank disable, bank sel 0*/
										/*                 ; BLC*/
		{0x4000, 0x10},/*; DCBLC auto load mode */
		{0x4001, 0x04},/*; BLC start line 4     */
		{0x4002, 0x45},/*; BLC auto, reset frame number = 5*/
		{0x4004, 0x02},/*; 2 lines for BLC*/
		{0x4005, 0x18},
		{0x4006, 0x20},/*; DC BLC coefficient*/
		{0x4008, 0x20},/*; First part BLC calculation start line address = 2, DC BLC on*/
		{0x4009, 0x10},/*; BLC target*/
		{0x404f, 0xa0},
		{0x4100, 0x10},
		{0x4101, 0x12},
		{0x4102, 0x24},
		{0x4103, 0x00},
		{0x4104, 0x5b},
		/*format control*/
		{0x4307, 0x30},/*; embed_line_st = 3*/
		{0x4315, 0x00},/*; Vsync trigger signal to Vsync output delay[15:8]*/
		{0x4511, 0x05},
		{0x4512, 0x00},/*; vertical sum*/
		/*MIPI control*/
		{0x4805, 0x21},
		{0x4806, 0x00},
		{0x481f, 0x36},
		{0x4831, 0x6c},
		/*LVDS*/
		{0x4a00, 0xaa},/*; SYNC code enable when only 1 lane, Bit swap, SAV first enable*/
		{0x4a03, 0x01},/*; Dummy data0[7:0]*/
		{0x4a05, 0x08},/*; Dummy data1[7:0]*/
		{0x4a0a, 0x88},
		{0x4d03, 0xbb},
		/*ISP*/
		{0x5000, 0x06},/*; LENC off, BPC on, WPC on*/
		{0x5001, 0x01},/*; MWB on   */
		{0x5002, 0x80},/*; scale on */
		{0x5003, 0x20},
		{0x5013, 0x00},/*; LSB disable */
		{0x5046, 0x4a},/*; ISP SOF sel - VSYNC*/
		/*DPC*/
		{0x5780, 0x1c},
		{0x5786, 0x20},
		{0x5787, 0x10},
		{0x5788, 0x18},
		{0x578a, 0x04},
		{0x578b, 0x02},
		{0x578c, 0x02},
		{0x578e, 0x06},
		{0x578f, 0x02},
		{0x5790, 0x02},
		{0x5791, 0xff},
		{0x5a08, 0x02},/*; window control*/
		{0x5e00, 0x00},/*; color bar off */
		{0x5e10, 0x0c},
		{0x5000, 0x06},/*; lenc off, bpc on, wpc on*/
		/*MWB*/
		{0x5001, 0x01},/*; MWB on      */
		{0x3400, 0x04},/*; red gain h  */
		{0x3401, 0x00},/*; red gain l  */
		{0x3402, 0x04},/*; green gain h*/
		{0x3403, 0x00},/*; green gain l*/
		{0x3404, 0x04},/*; blue gain h */
		{0x3405, 0x00},/*; blue gain l */
		{0x3406, 0x01},/*; MWB on      */
		/*BLC*/
		{0x4000, 0x10},/*; BLC on, BLC window as BLC, DCBLC auto load */
		{0x4002, 0x45},/*; Format change trigger off, auto on, reset frame number = 5*/
		{0x4005, 0x18},/*; no black line output, blc_man_1_en, BLC after reset, then stop, BLC triggered by*/
		/*gain change*/
		{0x4008, 0x20},/*;DCBLC is ON*/
		{0x4009, 0x10},/*; BLC target*/
		{0x3503, 0x07},/*; AEC manual, AGC manual */
		{0x3500, 0x00},/*; Exposure[19:16]*/
		{0x3501, 0x19},/*; Exposure[15:8] */
		{0x3502, 0x00},/*; Exposure[7:0]  */
		{0x350b, 0x20},/*; Gain[7:0]*/
		/*MIPI data rate = 640Mbps*/
		{0x30b3, 0x6f},/*; pll1_multiplier*/
		{0x30b4, 0x03},/*; pll1_prediv    */
		{0x30b5, 0x04},/*; pll1_op_pix_div*/
		{0x30b6, 0x01},/*; pll1_op_sys_div*/
		{0x4837, 0x0b},
		/*@@Raw 10bit 3264*2448 30fps 2lane 640M bps/lane*/
		{0x0100, 0x00},
		{0x3708, 0xe3},
		{0x3709, 0x03},
		{0x3800, 0x00},
		{0x3801, 0x0c},
		{0x3802, 0x00},
		{0x3803, 0x0c},
		{0x3804, 0x0c},
		{0x3805, 0xd3},
		{0x3806, 0x09},
		{0x3807, 0xa3},
		{0x3808, 0x0c},
		{0x3809, 0xc0},
		{0x380a, 0x09},
		{0x380b, 0x90},
		{0x380c, 0x0e},
		{0x380d, 0x18},
		{0x380e, 0x0a},
		{0x380f, 0x52},
		{0x3810, 0x00},
		{0x3811, 0x04},
		{0x3812, 0x00},
		{0x3813, 0x04},
		{0x3814, 0x11},
		{0x3815, 0x11},
		{0x3820, 0x10},
		{0x3821, 0x0e},
		{0x3a04, 0x09},
		{0x3a05, 0xa9},
		{0x4004, 0x08},
		{0x4005, 0x1a},
		{0x4512, 0x01},
		{0x3011, 0x21},
		{0x3015, 0xc8},
		{0x3090, 0x03},
		{0x3091, 0x22},
		{0x3092, 0x01},
		{0x3093, 0x00},
		{0x3094, 0x00},
		{0x0100, 0x01},

		{0xFFFF, 0x00}
	},
};

static const struct ov8830_reg ov8830_regdif[OV8830_MODE_MAX][256] = {
	{
		/*@@Preview Raw 10bit 1632*1224 30fps 2lane 712M bps/lane  BGGR*/
		{0x0100, 0x00},/*; software standby */
		{0x3708, 0xe6},/*; sensor control		 */
		{0x3709, 0xc3},/*; sensor control		 */
		{0x3800, 0x00},/*; HS = 8	 */
		{0x3801, 0x08},/*; HS			*/
		{0x3802, 0x00},/*; VS = 8	 */
		{0x3803, 0x08},/*; VS					*/
		{0x3804, 0x0c},/*; HE = 3287		 */
		{0x3805, 0xd7},/*; HE					*/
		{0x3806, 0x09},/*; VE = 2471		 */
		{0x3807, 0xa7},/*; VE					*/
		{0x3808, 0x05},/*; HO = 1280		 */
		{0x3809, 0x00},/*; HO					*/
		{0x380a, 0x03},/*; VO = 960		 */
		{0x380b, 0xc0},/*; VO					*/
		{0x380c, 0x0e},/*; HTS = 3608 	 */
		{0x380d, 0x18},/*; HTS				 */
		{0x380e, 0x05},/*; VTS = 1293 	 */
		{0x380f, 0x0d},/*; VTS	 */
		{0x3810, 0x00},/*; H OFFSET = 4		 */
		{0x3811, 0x04},/*; H OFFSET			 */
		{0x3812, 0x00},/*; V OFFSET = 4		 */
		{0x3813, 0x04},/*; V OFFSET			 */
		{0x3814, 0x31},/*; X INC	 */
		{0x3815, 0x31},/*; Y INC	 */
		{0x3820, 0x11},
		{0x3821, 0x0f},
		{0x3a04, 0x04},
		{0x3a05, 0xc9},
		{0x4004, 0x02},
		{0x4005, 0x18},
		{0x4512, 0x00},/*; vertical sum									*/
		{0x3011, 0x21},/*; MIPI 2 lane, MIPI enable				 */
		{0x3015, 0xc8},/*; MIPI 2 lane on, select MIPI			 */
										/* 					 ; SCLK = 140Mhz		*/
		{0x3662, 0x10},
		{0x3090, 0x02},/*; pll2_prediv			 */
		{0x3091, 0x2c},/*; pll2_multiplier		 */
		{0x3092, 0x01},/*; pll2_divs				 */
		{0x3093, 0x00},/*; pll2_seld5 			 */
		{0x3094, 0x00},
		{0x3098, 0x02},
		{0x3099, 0x13},
		{0x309a, 0x00},
		{0x309b, 0x00},
		{0x309c, 0x01},
		/*MIPI data rate = 624Mbps*/
		{0x30b3, 0x6f},/*; pll1_multiplier		*/
		{0x30b4, 0x02},/*; pll1_prediv			*/
		{0x30b5, 0x04},/*; pll1_op_pix_div		*/
		{0x30b6, 0x01},/*; pll1_op_sys_div		*/
		{0x4837, 0x0b},/*; MIPI global timing			*/
		{0x0100, 0x01},/*; wake up 						*/
		{0xFFFF, 0x00} /* end of the list 		  */
	},
	{
		/*Raw 10bit 1920*1080 30fps 2lane 640M bps/lane  */
		{0x0100, 0x00},/*software standby */
		{0x3708, 0xe3},/*sensor control		*/
		{0x3709, 0xc3},/*sensor control		*/
		{0x3800, 0x00},/*HS = 12 		*/
		{0x3801, 0x0c},/*HS				*/
		{0x3802, 0x01},/*VS = 320		*/
		{0x3803, 0x40},/*VS				*/
		{0x3804, 0x0c},/*HE = 3283		*/
		{0x3805, 0xd3},/*HE					*/
		{0x3806, 0x08},/*VE = 2163			*/
		{0x3807, 0x73},/*VE					*/
		{0x3808, 0x07},/*HO = 1920			*/
		{0x3809, 0x80},/*HO					*/
		{0x380a, 0x04},/*VO = 1080			*/
		{0x380b, 0x38},/*VO					*/
		{0x380c, 0x0e},/*HTS = 3608 	*/
		{0x380d, 0x18},/*HTS					*/
		{0x380e, 0x07},/*VTS =				*/
		{0x380f, 0xbc},/*VTS					*/
		{0x3810, 0x00},/*H OFFSET			*/
		{0x3811, 0x04},/*H OFFSET			*/
		{0x3812, 0x00},/*V OFFSET			*/
		{0x3813, 0x04},/*V OFFSET			*/
		{0x3814, 0x11},/*X INC				*/
		{0x3815, 0x11},/*Y INC				*/
		{0x3820, 0x10},
		{0x3821, 0x0e},
		{0x3a04, 0x07},
		{0x3a05, 0x49},
		{0x4004, 0x08},
		{0x4005, 0x18},
		{0x4512, 0x01},/*vertical average 							*/
		{0x3011, 0x21},/*MIPI 2 lane, MIPI enable		*/
		{0x3015, 0xc8},/*MIPI 2 lane on, select MIPI	*/
									/*CLK = 214Mhz 	 */
		{0x3662, 0x10},
		{0x3090, 0x01},/*pll2_prediv			*/
		{0x3091, 0x21},/*pll2_multiplier		*/
		{0x3092, 0x01},/*pll2_divs				*/
		{0x3093, 0x00},/*pll2_seld5 			*/
		{0x3094, 0x00},
		{0x3098, 0x02},
		{0x3099, 0x13},
		{0x309a, 0x00},
		{0x309b, 0x00},
		{0x309c, 0x01},
		/* 0xIPI data rate = 624Mbps*/
		{0x30b3, 0x6f},/*pll1_multiplier	*/
		{0x30b4, 0x02},/*pll1_prediv	*/
		{0x30b5, 0x04},/*pll1_op_pix_div	*/
		{0x30b6, 0x01},/*pll1_op_sys_div	*/
		{0x4837, 0x0b},/*MIPI global timing*/
		{0x0100, 0x01},/*wake up */

		{0xFFFF, 0x00},
	},

	/* mode 3264X2448 */
	{
		/*@@Capture  Raw 10bit 3264*2448 15fps 2lane 712M bps/lane BGGR*/
		{0x0100, 0x00},/*software standby*/
		{0x3708, 0xe3},/*sensor control  */
		{0x3709, 0xc3},/*sensor control  */
		{0x3800, 0x00},/*HS = 12         */
		{0x3801, 0x0c},/*HS              */
		{0x3802, 0x00},/*VS = 12         */
		{0x3803, 0x0c},/*VS              */
		{0x3804, 0x0c},/*HE = 3283       */
		{0x3805, 0xd3},/*HE              */
		{0x3806, 0x09},/*VE = 2467       */
		{0x3807, 0xa3},/*VE              */
		{0x3808, 0x0c},/*HO = 3264       */
		{0x3809, 0xc0},/*HO              */
		{0x380a, 0x09},/*VO = 2448       */
		{0x380b, 0x90},/*VO            */
		{0x380c, 0x0e},/*HTS = 3608    */
		{0x380d, 0x18},/*HTS           */
		{0x380e, 0x09},/*VTS = 2484    */
		{0x380f, 0xfc},/*VTS           */
		{0x3810, 0x00},/*H OFFSET = 4  */
		{0x3811, 0x04},/*H OFFSET      */
		{0x3812, 0x00},/*V OFFSET = 4  */
		{0x3813, 0x04},/*V OFFSET      */
		{0x3814, 0x11},/*X INC         */
		{0x3815, 0x11},/*Y INC         */
		{0x3820, 0x10},
		{0x3821, 0x0e},
		{0x3a04, 0x09},
		{0x3a05, 0xa9},
		{0x4004, 0x08},
		{0x4005, 0x1a},
		{0x4512, 0x01},/*; vertical average        */
		{0x3011, 0x21},/*; MIPI 2 lane, MIPI enable*/
		{0x3015, 0xc8},/*; MIPI 2 lane on, select MIPI*/
		/*SCLK = 140Mhz*/
		{0x3662, 0x10},
		{0x3090, 0x02},/*; pll2_prediv      */
		{0x3091, 0x2c},/*; pll2_multiplier  */
		{0x3092, 0x01},/*; pll2_divs  */
		{0x3093, 0x00},/*; pll2_seld5 */
		{0x3094, 0x00},
		{0x3098, 0x02},
		{0x3099, 0x13},
		{0x309a, 0x00},
		{0x309b, 0x00},
		{0x309c, 0x01},
		/*MIPI data rate = 624Mbps  */
		{0x30b3, 0x6f},/*; pll1_multiplier    */
		{0x30b4, 0x02},/*; pll1_prediv       */
		{0x30b5, 0x04},/*; pll1_op_pix_div*/
		{0x30b6, 0x01},/*; pll1_op_sys_div*/
		{0x4837, 0x0b},/*; MIPI global timing*/
		{0x0100, 0x01},/*; wake up*/

		{0xFFFF, 0x00}
	},
};

static const struct ov8830_reg ov8830_reg_state[OV8830_STATE_MAX][3] = {
	{ /* to power down */
	{0x0100, 0x00},	       /* disable streaming  */
	{0x4202, 0x0f},        /* disable mipi */
	{0xFFFF, 0x00}
	},
	{ /* to streaming */
	{0x4202, 0x00},         /* enable mipi */
	{0x0100, 0x01},		/* enable streaming */
	{0xFFFF, 0x00}
	},
};

static int set_flash_mode(struct i2c_client *client, int mode);
static int ov8830_set_mode(struct i2c_client *client, int new_mode_idx);
static int ov8830_set_state(struct i2c_client *client, int new_state);
static int ov8830_init(struct i2c_client *client);


/*
 * Find a data format by a pixel code in an array
 */
static int ov8830_find_datafmt(enum v4l2_mbus_pixelcode code)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(ov8830_fmts); i++)
		if (ov8830_fmts[i].code == code)
			break;

	/* If not found, select latest */
	if (i >= ARRAY_SIZE(ov8830_fmts))
		i = ARRAY_SIZE(ov8830_fmts) - 1;

	return i;
}

/* Find a frame size in an array */
static int ov8830_find_framesize(u32 width, u32 height)
{
	int i;

	/*try to find full match*/
	for (i = 0; i < OV8830_MODE_MAX; i++) {
		if ((ov8830_mode[i].width == width) &&
			(ov8830_mode[i].height == height))
			break;
	}
	/*try to find a mode that is bigger than expected width & height*/
	if (i >= OV8830_MODE_MAX) {
		for (i = 0; i < OV8830_MODE_MAX; i++) {
			if ((ov8830_mode[i].width >= width) &&
				(ov8830_mode[i].height >= height))
				break;
		}
	}

	/* If not found, select biggest */
	if (i >= OV8830_MODE_MAX)
		i = OV8830_MODE_MAX - 1;

	return i;
}

static struct ov8830 *to_ov8830(const struct i2c_client *client)
{
	return container_of(i2c_get_clientdata(client), struct ov8830, subdev);
}

/**
 *ov8830_reg_read - Read a value from a register in an ov8830 sensor device
 *@client: i2c driver client structure
 *@reg: register address / offset
 *@val: stores the value that gets read
 *
 * Read a value from a register in an ov8830 sensor device.
 * The value is returned in 'val'.
 * Returns zero if successful, or non-zero otherwise.
 */
static int ov8830_reg_read(struct i2c_client *client, u16 reg, u8 *val)
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
	SENSORDB("Failed reading register reg=0x%02x,val=%d!\n",\
		reg, *val);
	return ret;
}

/**
 * Write a value to a register in ov8830 sensor device.
 *@client: i2c driver client structure.
 *@reg: Address of the register to read value from.
 *@val: Value to be written to a specific register.
 * Returns zero if successful, or non-zero otherwise.
 */
static int ov8830_reg_write(struct i2c_client *client, u16 reg, u8 val)
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

static int ov8830_reg_read_multi(struct i2c_client *client,
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

static const struct v4l2_queryctrl ov8830_controls[] = {
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
	 .minimum = OV8830_GAIN_MIN,
	 .maximum = OV8830_GAIN_MAX,
	 .step = OV8830_GAIN_STEP,
	 .default_value = DEFAULT_GAIN,
	 },
	{
	 .id = V4L2_CID_EXPOSURE,
	 .type = V4L2_CTRL_TYPE_INTEGER,
	 .name = "Exposure",
	 .minimum = OV8830_EXP_MIN,
	 .maximum = OV8830_EXP_MAX,
	 .step = OV8830_EXP_STEP,
	 .default_value = DEFAULT_EXPO,
	 },
	{
	 .id = V4L2_CID_CAMERA_LENS_POSITION,
	 .type = V4L2_CTRL_TYPE_INTEGER,
	 .name = "Lens Position",
	 .minimum = OV8830_LENS_MIN,
	 .maximum = OV8830_LENS_MAX,
	 .step = OV8830_LENS_STEP,
	 .default_value = DEFAULT_LENS_POS,
	 },
	{
	 .id = V4L2_CID_CAMERA_FLASH_MODE,
	 .type = V4L2_CTRL_TYPE_INTEGER,
#ifdef CONFIG_VIDEO_AS3643
	 .name = "AS3643-flash",
#endif

#ifdef CONFIG_GPIO_FLASH
	 .name = "GPIO-flash",
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
	 .minimum = OV8830_FLASH_INTENSITY_MIN,
	 .maximum = OV8830_FLASH_INTENSITY_MAX,
	 .step = OV8830_FLASH_INTENSITY_STEP,
	 .default_value = OV8830_FLASH_INTENSITY_DEFAULT,
	 },
	{
	 .id = V4L2_CID_FLASH_TIMEOUT,
	 .type = V4L2_CTRL_TYPE_INTEGER,
	 .name = "Flash timeout in us",
	 .minimum = OV8830_FLASH_TIMEOUT_MIN,
	 .maximum = OV8830_FLASH_TIMEOUT_MAX,
	 .step = OV8830_FLASH_TIMEOUT_STEP,
	 .default_value = OV8830_FLASH_TIMEOUT_DEFAULT,
	 },
};

/**
 * Initialize a list of ov8830 registers.
 * The list of registers is terminated by the pair of values
 *@client: i2c driver client structure.
 *@reglist[]: List of address of the registers to write data.
 * Returns zero if successful, or non-zero otherwise.
 */
static int ov8830_reg_writes(struct i2c_client *client,
			     const struct ov8830_reg reglist[])
{
	int err = 0, i;

	for (i = 0; reglist[i].reg != 0xFFFF; i++) {
		if (reglist[i].reg == 0xFFFE) {
			msleep(reglist[i].val);
		} else {
			err |= ov8830_reg_write(client, reglist[i].reg,
						reglist[i].val);
		}
		if (err != 0)
			break;
	}
	return 0;
}

#ifdef OV8830_DEBUG
static int ov8830_reglist_compare(struct i2c_client *client,
			const struct ov8830_reg reglist[])
{
	int err = 0;
	u8 reg, i;

	for (i = 0; ((reglist[i].reg != 0xFFFF) && (err == 0)); i++) {
		if (reglist[i].reg != 0xFFFE) {
			err |= ov8830_reg_read(client, reglist[i].reg, &reg);
			pr_debug("ov8830_reglist_compare: [0x%04x]=0x%02x",
				 reglist[i].reg, reg);
		} else {
			msleep(reglist[i].val);
		}
	}
	return 0;
}
#endif

/**
 * Write an array of data to ov8830 sensor device.
 *@client: i2c driver client structure.
 *@reg: Address of the register to read value from.
 *@data: pointer to data to be written starting at specific register.
 *@size: # of data to be written starting at specific register.
 * Returns zero if successful, or non-zero otherwise.
 */
static int ov8830_array_write(struct i2c_client *client,
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

static int ov8830_lens_set_position(struct i2c_client *client,
				    int target_position)
{
	int ret = 0;
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
static void ov8830_lens_get_position(struct i2c_client *client,
					int *current_position,
					int *time_to_destination)
{
	int ret = 0;
	int i = 0;
	struct ov8830 *ov8830 = to_ov8830(client);
	static int lens_read_buf[LENS_READ_DELAY];

#ifdef CONFIG_VIDEO_DW9714
		ret = dw9714_lens_get_position(current_position,\
			time_to_destination);
#endif

	for (i = 0; i < ov8830->lenspos_delay; i++)
		lens_read_buf[i] = lens_read_buf[i + 1];
	lens_read_buf[ov8830->lenspos_delay] = *current_position;
	*current_position = lens_read_buf[0];
	/*SENSORDB("current_position=%d", *current_position);*/
	return;
}

/*
 * Setup the sensor integration and frame length based on requested mimimum
 * framerate
 */
static void ov8830_set_framerate_lo(struct i2c_client *client, int framerate)
{
	struct ov8830 *ov8830 = to_ov8830(client);
	framerate = max(framerate, ov8830->framerate_lo_absolute);
	framerate = min(framerate, ov8830->framerate_hi_absolute);

	ov8830->framerate_lo = framerate;
	ov8830->vts_max = 1000000000
		/ (unsigned long)((ov8830->line_length * framerate) >> 8);
	ov8830->vts_max = min(ov8830->vts_max,
			ov8830_mode[ov8830->mode_idx].vts_max);
	pr_debug("ov8830_set_framerate_lo: Setting frame rate lo %d vts_max %d",
			ov8830->framerate_lo, ov8830->vts_max);
}

/*
 * Setup the sensor integration and frame length based on requested maximum
 * framerate, famerate is 24p8 fixed point.
 */
static void ov8830_set_framerate_hi(struct i2c_client *client, int framerate)
{
	struct ov8830 *ov8830 = to_ov8830(client);

	framerate = max(framerate, ov8830->framerate_lo_absolute);
	framerate = min(framerate, ov8830->framerate_hi_absolute);

	ov8830->framerate_hi = framerate;
	ov8830->vts_min = 1000000000
		/ (unsigned long)((ov8830->line_length * framerate) >> 8);

	ov8830->vts_min = max(ov8830->vts_min,
			ov8830_mode[ov8830->mode_idx].vts);
	pr_debug("ov8830_set_framerate_hi: Setting frame rate hi %d vts_min %d",
		ov8830->framerate_hi, ov8830->vts_min);
}


/*
 * Determine the closest achievable gain to the desired gain.
 * Calculates the closest achievable gain to the requested gain
 * that can be achieved by this sensor.
 * @param gain_value Desired gain on 8.8 linear scale.
 * @return Achievable gain on 8.8 linear scale.
*/
static int ov8830_calc_gain(struct i2c_client *client, int gain_value,
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
static int ov8830_set_gain(struct i2c_client *client, int gain_value)
{
	struct ov8830 *ov8830 = to_ov8830(client);
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
	gain_actual = ov8830_calc_gain(client, gain_value, &gain_code_analog);
	/*SENSORDB("cur=%u req=%u act=%u cod=%u",
		 ov8830->gain_current, gain_value,
		 gain_actual, gain_code_analog);*/
	/*if (gain_actual == ov8830->gain_current)
		return 0;*/
	ret = ov8830_reg_write(client,
		OV8830_REG_AGC_HI, (gain_code_analog >> 8) & 0x3);
	ret |= ov8830_reg_write(client,
		OV8830_REG_AGC_LO, gain_code_analog & 0xFF);
	ov8830->gain_current = gain_actual;
	return 0;
}

static int ov8830_get_gain(struct i2c_client *client,
		int *gain_value_p, int *gain_code_p)
{
	struct ov8830 *ov8830 = to_ov8830(client);

	int ret = 0;
	int gain_code;
	u8 gain_buf[2];
	int i;

	/*ov8830_reg_read_multi(client, OV8830_REG_AGC_HI, gain_buf, 2);*/
	if (0) {
		/*ov8830->aecpos_delay = 0;*/
		gain_code = ((gain_buf[0] & 0x3f) << 8) + gain_buf[1];
	} else {
		gain_code = OV8830_GAIN_2_CODE(ov8830->gain_current);
		if (OV8830_MODE_3264x2448P15 == ov8830->mode_idx)
			gain_code = ov8830->gain_read_buf[0];
	}

	if (ov8830->aecpos_delay > 0) {
		ov8830->gain_read_buf[ov8830->aecpos_delay] = gain_code;
		gain_code = ov8830->gain_read_buf[0];
		for (i = 0; i < GAIN_DELAY_MAX-1; i++)
			ov8830->gain_read_buf[i] = ov8830->gain_read_buf[i+1];
	}
	if (gain_code < 0x10)
		gain_code = 0x10;

	if (gain_code_p)
		*gain_code_p = gain_code;
	if (gain_value_p)
		*gain_value_p = gain_code << 4;
	/*SENSORDB("gain_value_p=%d",*gain_value_p);*/
	return ret;
}

static int ov8830_get_exposure(struct i2c_client *client,
		int *exp_value_p, int *exp_code_p)
{
	struct ov8830 *ov8830 = to_ov8830(client);
	int i, ret = 0;
	int exp_code;
	u8 exp_buf[3];

	/*ov8830_reg_read_multi(client, OV8830_REG_EXP_HI, exp_buf, 3);*/
	if (0) {
		/*ov8830->aecpos_delay = 0;*/
		exp_code =
			((exp_buf[0] & 0xf) << 16) +
			((exp_buf[1] & 0xff) << 8) +
			(exp_buf[2] & 0xf0);
	} else {
		exp_code = OV8830_EXPOSURE_2_CODE(ov8830->exposure_current,\
			ov8830->line_length);
		if (OV8830_MODE_3264x2448P15 == ov8830->mode_idx) {
			exp_code = ov8830->exp_read_buf[0];
		}
	}

	if (ov8830->aecpos_delay > 0) {
		ov8830->exp_read_buf[ov8830->aecpos_delay] = exp_code;
		exp_code = ov8830->exp_read_buf[0];
		for (i = 0; i < EXP_DELAY_MAX-1; i++)
			ov8830->exp_read_buf[i] = ov8830->exp_read_buf[i+1];
	}

	if (exp_code_p)
		*exp_code_p = exp_code;
	if (exp_value_p)
		*exp_value_p = ((exp_code >> 4) * ov8830->line_length) /
			(unsigned long)1000;
	/*SENSORDB("exp_value_p=%d",*exp_value_p);*/
	return ret;
}

#define INTEGRATION_MIN		1
#define INTEGRATION_OFFSET	10

static int ov8830_calc_exposure(struct i2c_client *client,
				int exposure_value,
				unsigned int *vts_ptr,
				unsigned int *coarse_int_lines)
{
	struct ov8830 *ov8830 = to_ov8830(client);
	unsigned int integration_lines;
	int vts = ov8830_mode[ov8830->mode_idx].vts;
	int binning_factor = ov8830_mode[ov8830->mode_idx].binning_factor;

	integration_lines = (1000 * exposure_value) / ov8830->line_length;
	integration_lines = integration_lines * binning_factor;
	if (integration_lines < INTEGRATION_MIN)
		integration_lines = INTEGRATION_MIN;
	else if (integration_lines > (unsigned int)
		 (ov8830->vts_max - INTEGRATION_OFFSET))
		integration_lines = ov8830->vts_max - INTEGRATION_OFFSET;
	vts = min(integration_lines + INTEGRATION_OFFSET,
		  (unsigned int)ov8830_mode[ov8830->mode_idx].vts_max);
	vts = max(vts, ov8830_mode[ov8830->mode_idx].vts);
	vts = max(vts, ov8830->vts_min);
	if (coarse_int_lines)
		*coarse_int_lines = integration_lines;
	if (vts_ptr)
		*vts_ptr = vts;

	exposure_value = integration_lines * ov8830->line_length /
		(unsigned long)1000;
	return exposure_value;
}

/*
 * Setup the sensor integration and frame length based on requested exposure
 * in microseconds.
 */
#define EXP_HIST_MAX 0
static void ov8830_set_exposure(struct i2c_client *client, int exp_value)
{
	struct ov8830 *ov8830 = to_ov8830(client);
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

	actual_exposure = ov8830_calc_exposure(client, exp_value,
			&vts, &coarse_int_lines);
	/*SENSORDB("cur=%d req=%d act=%d coarse=%d vts=%d",
			ov8830->exposure_current, exp_value, actual_exposure,
			coarse_int_lines, vts);*/
	ov8830->vts = vts;
	ov8830->exposure_current = actual_exposure;
	coarse_int_lines <<= 4;
	exp_buf[0] = (u8)((OV8830_REG_EXP_HI >> 8) & 0xFF);
	exp_buf[1] = (u8)(OV8830_REG_EXP_HI & 0xFF);
	exp_buf[2] = (u8)((coarse_int_lines >> 16) & 0xFF);
	exp_buf[3] = (u8)((coarse_int_lines >> 8) & 0xFF);
	exp_buf[4] = (u8)((coarse_int_lines >> 0) & 0xF0);
	ret = ov8830_array_write(client, exp_buf, 5);
	ret |= ov8830_reg_write(client,
			OV8830_REG_TIMING_VTS_HI, (vts & 0xFF00) >> 8);
	ret |= ov8830_reg_write(client, OV8830_REG_TIMING_VTS_LO, vts & 0xFF);
	if (ret) {
		pr_debug("ov8830_set_exposure: error writing exp. ctrl");
		return;
	}
}

static int ov8830_s_stream(struct v4l2_subdev *sd, int enable)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int ret;

	ret = ov8830_set_state(client, enable);
	return ret;
}

static int ov8830_set_bus_param(struct soc_camera_device *icd,
				unsigned long flags)
{
	/* TODO: Do the right thing here, and validate bus params */
	return 0;
}

static unsigned long ov8830_query_bus_param(struct soc_camera_device *icd)
{
	unsigned long flags = SOCAM_PCLK_SAMPLE_FALLING |
		SOCAM_HSYNC_ACTIVE_HIGH | SOCAM_VSYNC_ACTIVE_HIGH |
		SOCAM_DATA_ACTIVE_HIGH | SOCAM_MASTER;

	/* TODO: Do the right thing here, and validate bus params */

	flags |= SOCAM_DATAWIDTH_10;

	return flags;
}

static int ov8830_enum_input(struct soc_camera_device *icd,
			     struct v4l2_input *inp)
{
	struct soc_camera_link *icl = to_soc_camera_link(icd);
	struct v4l2_subdev_sensor_interface_parms *plat_parms;

	inp->type = V4L2_INPUT_TYPE_CAMERA;
	inp->std = V4L2_STD_UNKNOWN;
	strcpy(inp->name, "ov8830");

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

static int ov8830_g_fmt(struct v4l2_subdev *sd, struct v4l2_mbus_framefmt *mf)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ov8830 *ov8830 = to_ov8830(client);

	mf->width = ov8830_mode[ov8830->mode_idx].width;
	mf->height = ov8830_mode[ov8830->mode_idx].height;
	mf->code = ov8830_fmts[ov8830->i_fmt].code;
	mf->colorspace = ov8830_fmts[ov8830->i_fmt].colorspace;
	mf->field = V4L2_FIELD_NONE;

	return 0;
}

static int ov8830_try_fmt(struct v4l2_subdev *sd, struct v4l2_mbus_framefmt *mf)
{
	int i_fmt;
	int mode_idx;

	i_fmt = ov8830_find_datafmt(mf->code);

	mf->code = ov8830_fmts[i_fmt].code;
	mf->colorspace = ov8830_fmts[i_fmt].colorspace;
	mf->field = V4L2_FIELD_NONE;

	mode_idx = ov8830_find_framesize(mf->width, mf->height);

	mf->width = ov8830_mode[mode_idx].width;
	mf->height = ov8830_mode[mode_idx].height;

	return 0;
}

static int ov8830_set_mode(struct i2c_client *client, int new_mode_idx)
{
	struct ov8830 *ov8830 = to_ov8830(client);
	int ret = 0;

	if (ov8830->mode_idx == new_mode_idx) {
		printk(KERN_INFO "ov8830_set_mode: skip init from mode[%d]=%s to mode[%d]=%s",
			ov8830->mode_idx, ov8830_mode[ov8830->mode_idx].name,
			new_mode_idx, ov8830_mode[new_mode_idx].name);
		return ret;
	}

	if (ov8830->mode_idx == OV8830_MODE_MAX) {
		printk(KERN_INFO "ov8830_set_mode: full init from mode[%d]=%s to mode[%d]=%s",
		ov8830->mode_idx, ov8830_mode[ov8830->mode_idx].name,
		new_mode_idx, ov8830_mode[new_mode_idx].name);
		ov8830_init(client);
		ret  = ov8830_reg_writes(client, ov8830_regtbl[new_mode_idx]);
	} else {
		printk(KERN_INFO "ov8830_set_mode: diff init from mode[%d]=%s to mode[%d]=%s",
			ov8830->mode_idx, ov8830_mode[ov8830->mode_idx].name,
			new_mode_idx, ov8830_mode[new_mode_idx].name);
		ret = ov8830_reg_writes(client, ov8830_regdif[new_mode_idx]);
	}
	/*if (ret)
		return ret;*/
	ov8830->mode_idx = new_mode_idx;
	ov8830->line_length  = ov8830_mode[new_mode_idx].line_length_ns;
	ov8830->vts = ov8830_mode[new_mode_idx].vts;
	ov8830->framerate_lo_absolute = F24p8(1.0);
	ov8830->framerate_hi_absolute = ov8830_mode[new_mode_idx].fps;
	ov8830_set_framerate_lo(client, ov8830->framerate_lo_absolute);
	ov8830_set_framerate_hi(client, ov8830->framerate_hi_absolute);

	return 0;
}

static int ov8830_set_state(struct i2c_client *client, int new_state)
{
	struct ov8830 *ov8830 = to_ov8830(client);
	int ret = 0;

	SENSORDB("ov8830_set_state: %d (%s) -> %d (%s)", ov8830->state,
		 ov8830->state ? "strm" : "stop",
		 new_state, new_state ? "strm" : "stop");

	if (ov8830->state != new_state) {
		ret = ov8830_reg_writes(client, ov8830_reg_state[new_state]);
		ov8830->state = new_state;
		if (OV8830_STATE_STRM == new_state &&
		    0 == ov8830->calibrated) {
			ov8830->calibrated = 1;
		}
	}
	return ret;
}

static int ov8830_s_fmt(struct v4l2_subdev *sd, struct v4l2_mbus_framefmt *mf)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ov8830 *ov8830 = to_ov8830(client);
	int ret = 0;
	int new_mode_idx;

	ret = ov8830_try_fmt(sd, mf);
	if (ret < 0)
		return ret;

	pr_debug("ov8830_s_fmt: width %d, height %d", mf->width, mf->height);

	new_mode_idx = ov8830_find_framesize(mf->width, mf->height);
	ov8830->i_fmt = ov8830_find_datafmt(mf->code);
	switch ((u32) ov8830_fmts[ov8830->i_fmt].code) {
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

	ov8830_set_mode(client, new_mode_idx);
	return ret;
}

static int ov8830_g_chip_ident(struct v4l2_subdev *sd,
			       struct v4l2_dbg_chip_ident *id)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	if (id->match.type != V4L2_CHIP_MATCH_I2C_ADDR)
		return -EINVAL;

	if (id->match.addr != client->addr)
		return -ENODEV;

	id->ident = V4L2_IDENT_OV8825;
	id->revision = 0;

	return 0;
}


static int ov8830_g_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ov8830 *ov8830 = to_ov8830(client);
	int retval;

	switch (ctrl->id) {
	case V4L2_CID_CAMERA_FRAME_RATE:
		ctrl->value = ov8830->framerate;
		break;
	case V4L2_CID_CAMERA_FOCUS_MODE:
		ctrl->value = ov8830->focus_mode;
		break;
	case V4L2_CID_GAIN:
		ov8830_get_gain(client, &retval, NULL);
		ctrl->value = retval;
		break;
	case V4L2_CID_EXPOSURE:
		ov8830_get_exposure(client, &retval, NULL);
		ctrl->value = retval;
		break;
	case V4L2_CID_CAMERA_LENS_POSITION:
		ov8830_lens_get_position(client, &ov8830->current_position,
			&ov8830->time_to_destination);
		ctrl->value = ov8830->current_position;
		break;
	case V4L2_CID_CAMERA_FLASH_MODE:
		ctrl->value = ov8830->flashmode;
		break;
	case V4L2_CID_FLASH_INTENSITY:
		ctrl->value = ov8830->flash_intensity;
		break;
	case V4L2_CID_FLASH_TIMEOUT:
		ctrl->value = ov8830->flash_timeout;
		break;

	}
	return 0;
}

static int ov8830_s_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ov8830 *ov8830 = to_ov8830(client);
	int ret = 0;

	switch (ctrl->id) {
	case V4L2_CID_CAMERA_FRAME_RATE:

		if (ctrl->value > FRAME_RATE_30)
			return -EINVAL;

		ov8830->framerate = ctrl->value;

		switch (ov8830->framerate) {
		case FRAME_RATE_5:
			ov8830->framerate_lo = F24p8(5.0);
			ov8830->framerate_hi = F24p8(5.0);
			break;
		case FRAME_RATE_7:
			ov8830->framerate_lo = F24p8(7.0);
			ov8830->framerate_hi = F24p8(7.0);
			break;
		case FRAME_RATE_10:
			ov8830->framerate_lo = F24p8(10.0);
			ov8830->framerate_hi = F24p8(10.0);
			break;
		case FRAME_RATE_15:
			ov8830->framerate_lo = F24p8(15.0);
			ov8830->framerate_hi = F24p8(15.0);
			break;
		case FRAME_RATE_20:
			ov8830->framerate_lo = F24p8(20.0);
			ov8830->framerate_hi = F24p8(20.0);
			break;
		case FRAME_RATE_25:
			ov8830->framerate_lo = F24p8(25.0);
			ov8830->framerate_hi = F24p8(25.0);
			break;
		case FRAME_RATE_30:
			ov8830->framerate_lo = F24p8(30.0);
			ov8830->framerate_hi = F24p8(30.0);
			break;
		case FRAME_RATE_AUTO:
		default:
			ov8830->framerate_lo = F24p8(1.0);
			ov8830->framerate_hi = F24p8(30.0);
			break;
		}
		ov8830_set_framerate_lo(client, ov8830->framerate_lo);
		ov8830_set_framerate_hi(client, ov8830->framerate_hi);

		if (ret)
			return ret;
		break;

	case V4L2_CID_CAMERA_FOCUS_MODE:

		if (ctrl->value > FOCUS_MODE_MANUAL)
			return -EINVAL;

		ov8830->focus_mode = ctrl->value;

		/*
		 * Donot start the AF cycle here
		 * AF Start will be called later in
		 * V4L2_CID_CAMERA_SET_AUTO_FOCUS only for auto, macro mode
		 * it wont be called for infinity.
		 * Donot worry about resolution change for now.
		 * From userspace we set the resolution first
		 * and then set the focus mode.
		 */
		switch (ov8830->focus_mode) {
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
		if ((unsigned int)(ctrl->value) > OV8830_GAIN_MAX) {
			pr_debug("V4L2_CID_GAIN invalid gain=%u max=%u",
				ctrl->value, OV8830_GAIN_MAX);
			return -EINVAL;
		}
		ov8830_set_gain(client, ctrl->value);
		break;

	case V4L2_CID_EXPOSURE:
		if (ctrl->value > OV8830_EXP_MAX)
			return -EINVAL;
		ov8830_set_exposure(client, ctrl->value);
		break;

	case V4L2_CID_CAMERA_LENS_POSITION:
		if (ctrl->value > OV8830_LENS_MAX)
			return -EINVAL;
		ov8830_lens_set_position(client, ctrl->value);
		break;
	case V4L2_CID_CAMERA_FLASH_MODE:
		set_flash_mode(client, ctrl->value);
		break;
	case V4L2_CID_FLASH_INTENSITY:
		ov8830->flash_intensity = ctrl->value;
		break;
	case V4L2_CID_FLASH_TIMEOUT:
		ov8830->flash_timeout = ctrl->value;
		break;

	}

	return ret;
}

int set_flash_mode(struct i2c_client *client, int mode)
{
	struct ov8830 *ov8830 = to_ov8830(client);
	if (ov8830->flashmode == mode)
		return 0;
#ifdef CONFIG_VIDEO_AS3643
	if ((mode == FLASH_MODE_OFF) || (mode == FLASH_MODE_TORCH_OFF)) {
		if (ov8830->flashmode != FLASH_MODE_OFF) {
			ov8830_reg_write(client, 0x3B00, 0x00);
			as3643_clear_all();
			as3643_gpio_toggle(0);
		}
	} else if (mode == FLASH_MODE_TORCH_ON) {
		as3643_gpio_toggle(1);
		usleep_range(25, 30);
		as3643_set_torch_flash(0x80);
	} else if (mode == FLASH_MODE_ON) {
		ov8830_reg_write(client, 0x3B00, 0x83);
		as3643_gpio_toggle(1);
		usleep_range(25, 30);
		/* FIXME: timeout should be vts*line_length/1000+constant? */
		as3643_set_ind_led(ov8830->flash_intensity / 5,
				ov8830->flash_timeout);
	} else if (mode == FLASH_MODE_AUTO) {
		as3643_gpio_toggle(1);
		usleep_range(25, 30);
		as3643_set_ind_led(0x80, 30000);
		ov8830_reg_write(client, 0x3B00, 0x83);
	/* Not yet implemented */
	} else {
		return -EINVAL;
	}
	ov8830->flashmode = mode;
#endif

#ifdef CONFIG_GPIO_FLASH
		if ((mode == FLASH_MODE_OFF)
			|| (mode == FLASH_MODE_TORCH_OFF)) {
			gpio_flash_torch_off();
		} else if (mode == FLASH_MODE_TORCH_ON) {
			gpio_flash_torch_on();
		} else if (mode == FLASH_MODE_ON) {
			ov8830->flash_timeout =
				2 * (ov8830->vts * ov8830->line_length)/1000;
			gpio_flash_flash_on(ov8830->flash_timeout);
		} else if (mode == FLASH_MODE_AUTO) {
			ov8830->flash_timeout =
				2 * (ov8830->vts * ov8830->line_length)/1000;
			gpio_flash_flash_on(ov8830->flash_timeout);
		} else {
			return -EINVAL;
		}
	ov8830->flashmode = mode;
#endif

	/*SENSORDB("intensity=%d,timeout=%d",\
		ov8830->flash_intensity,ov8830->flash_timeout);*/
	return 0;
}

static long ov8830_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
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
static int ov8830_g_register(struct v4l2_subdev *sd,
			     struct v4l2_dbg_register *reg)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	if (reg->match.type != V4L2_CHIP_MATCH_I2C_ADDR || reg->size > 2)
		return -EINVAL;

	if (reg->match.addr != client->addr)
		return -ENODEV;

	reg->size = 2;
	if (ov8830_reg_read(client, reg->reg, &reg->val))
		return -EIO return 0;
}

static int ov8830_s_register(struct v4l2_subdev *sd,
			     struct v4l2_dbg_register *reg)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	if (reg->match.type != V4L2_CHIP_MATCH_I2C_ADDR || reg->size > 2)
		return -EINVAL;

	if (reg->match.addr != client->addr)
		return -ENODEV;

	if (ov8830_reg_write(client, reg->reg, reg->val))
		return -EIO;

	return 0;
}
#endif

static int ov8830_s_power(struct v4l2_subdev *sd, int on)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ov8830 *ov8830 = to_ov8830(client);

	if (0 == on) {
		ov8830->mode_idx = OV8830_MODE_MAX;
		ov8830->calibrated = 0;
	}
	return 0;
}

static struct soc_camera_ops ov8830_ops = {
	.set_bus_param   = ov8830_set_bus_param,
	.query_bus_param = ov8830_query_bus_param,
	.enum_input      = ov8830_enum_input,
	.controls        = ov8830_controls,
	.num_controls    = ARRAY_SIZE(ov8830_controls),
};

static int ov8830_init(struct i2c_client *client)
{
	struct ov8830 *ov8830 = to_ov8830(client);
	int ret = 0;
	int i = 0;

	/* reset */
	ov8830_reg_write(client, 0x0103, 0x01);

	/* set initial mode */
	ov8830->mode_idx = OV8830_MODE_MAX;
	ov8830->state = OV8830_STATE_STOP;

	/* default settings */
	ov8830->framerate         = FRAME_RATE_AUTO;
	ov8830->focus_mode        = FOCUS_MODE_AUTO;
	ov8830->gain_current      = DEFAULT_GAIN * 2;

	memset(&ov8830->exp_read_buf, 0, sizeof(ov8830->exp_read_buf));
	memset(&ov8830->gain_read_buf, 0, sizeof(ov8830->gain_read_buf));
	memset(&ov8830->lens_read_buf, 0, sizeof(ov8830->lens_read_buf));
	/*
	 *  Exposure should be DEFAULT_EXPO * line_length / 1000
	 *  Since we don't have line_length yet, just estimate
	 */

	ov8830->exposure_current  = DEFAULT_EXPO * 200;
	ov8830->aecpos_delay      = 2;
	ov8830->lenspos_delay     = 0;
	ov8830->flashmode         = FLASH_MODE_OFF;
	ov8830->flash_intensity   = OV8830_FLASH_INTENSITY_DEFAULT;
	ov8830->flash_timeout     = OV8830_FLASH_TIMEOUT_DEFAULT;

	ov8830->position = 0;
	ov8830->dac_code = 0;
	ov8830->position = 0;
	ov8830->timing_step_us = 0xf;

	for (i = 0; i < ov8830->aecpos_delay; i++) {
		ov8830->exp_read_buf[i] = (ov8830->exposure_current \
				* 1000 / ov8830->line_length) << 4;
		ov8830->gain_read_buf[i] = \
			ov8830->gain_current >> 4;
	}
	/*SENSORDB("exp_buf[0]=%d,exp_buf[1]=%d",\
		ov8830->exp_read_buf[0],ov8830->exp_read_buf[1]);*/
	dev_dbg(&client->dev, "Sensor initialized\n");
	return ret;
}

/*
 * Interface active, can use i2c. If it fails, it can indeed mean, that
 * this wasn't our capture interface, so, we wait for the right one
 */
static int ov8830_video_probe(struct soc_camera_device *icd,
			      struct i2c_client *client)
{
	unsigned long flags;
	int ret = 0;
	u8 revision = 0, id_high, id_low;
	u16 id;

	s_ov8830_i2c_client = client;
	/*
	 * We must have a parent by now. And it cannot be a wrong one.
	 * So this entire test is completely redundant.
	 */
	if (!icd->dev.parent ||
	    to_soc_camera_host(icd->dev.parent)->nr != icd->iface)
		return -ENODEV;


	client->addr = 0x20>>1;
	printk(KERN_ERR "ov8830_video_probe\n");

	ret = ov8830_reg_read(client, 0x302A, &revision);
	if (ret) {
		dev_err(&client->dev, "Failure to detect OV8830 chip\n");
		goto out;
	}

	revision &= 0xF;

	flags = SOCAM_DATAWIDTH_8;

	/* Read sensor Model ID */
	ret = ov8830_reg_read(client, OV8830_REG_CHIP_ID_HIGH, &id_high);
	if (ret < 0)
		return ret;

	id = id_high << 8;

	ret = ov8830_reg_read(client, OV8830_REG_CHIP_ID_LOW, &id_low);
	if (ret < 0)
		return ret;

	id |= id_low;
	SENSORDB("read chip id=0x%x", id);
	if (id != OV8830_CHIP_ID)
		return -ENODEV;

	dev_err(&client->dev, "Detected a OV8830 chip 0x%04x, revision %x\n",
		 id, revision);

out:
	return ret;
}

static void ov8830_video_remove(struct soc_camera_device *icd)
{
	dev_dbg(&icd->dev, "Video removed: %p, %p\n",
		icd->dev.parent, icd->vdev);
}

static struct v4l2_subdev_core_ops ov8830_subdev_core_ops = {
	.g_chip_ident = ov8830_g_chip_ident,
	.g_ctrl = ov8830_g_ctrl,
	.s_ctrl = ov8830_s_ctrl,
	.ioctl = ov8830_ioctl,
#ifdef CONFIG_VIDEO_ADV_DEBUG
	.g_register = ov8830_g_register,
	.s_register = ov8830_s_register,
#endif
	.s_power = ov8830_s_power,
};

static int ov8830_enum_fmt(struct v4l2_subdev *sd, unsigned int index,
			   enum v4l2_mbus_pixelcode *code)
{
	if (index >= ARRAY_SIZE(ov8830_fmts))
		return -EINVAL;

	*code = ov8830_fmts[index].code;
	return 0;
}

static int ov8830_enum_framesizes(struct v4l2_subdev *sd,
				  struct v4l2_frmsizeenum *fsize)
{
	if (fsize->index >= OV8830_MODE_MAX)
		return -EINVAL;

	fsize->type = V4L2_FRMSIZE_TYPE_DISCRETE;
	fsize->pixel_format = V4L2_PIX_FMT_SRGGB10;
	fsize->discrete.width = ov8830_mode[fsize->index].width;
	fsize->discrete.height = ov8830_mode[fsize->index].height;

	return 0;
}

/* we only support fixed frame rate */
static int ov8830_enum_frameintervals(struct v4l2_subdev *sd,
				      struct v4l2_frmivalenum *interval)
{
	int size;

	if (interval->index >= 1)
		return -EINVAL;

	interval->type = V4L2_FRMIVAL_TYPE_DISCRETE;

	size = ov8830_find_framesize(interval->width, interval->height);

	switch (size) {
	case OV8830_MODE_3264x2448P15:
		interval->discrete.numerator = 1;
		interval->discrete.denominator = 15;
		break;
	case OV8830_MODE_1280x960P30:
	default:
		interval->discrete.numerator = 1;
		interval->discrete.denominator = 30;
		break;
	}
	return 0;
}

static int ov8830_g_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *param)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ov8830 *ov8830 = to_ov8830(client);
	struct v4l2_captureparm *cparm;

	if (param->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;

	cparm = &param->parm.capture;

	memset(param, 0, sizeof(*param));
	param->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	cparm->capability = V4L2_CAP_TIMEPERFRAME;

	switch (ov8830->mode_idx) {
	case OV8830_MODE_3264x2448P15:
		cparm->timeperframe.numerator = 1;
		cparm->timeperframe.denominator = 15;
		break;
	case OV8830_MODE_1280x960P30:
	default:
		cparm->timeperframe.numerator = 1;
		cparm->timeperframe.denominator = 30;
		break;
	}

	return 0;
}
static int ov8830_s_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *param)
{
	/*
	 * FIXME: This just enforces the hardcoded framerates until this is
	 *flexible enough.
	 */
	return ov8830_g_parm(sd, param);
}

static struct v4l2_subdev_video_ops ov8830_subdev_video_ops = {
	.s_stream = ov8830_s_stream,
	.s_mbus_fmt = ov8830_s_fmt,
	.g_mbus_fmt = ov8830_g_fmt,
	.try_mbus_fmt = ov8830_try_fmt,
	.enum_mbus_fmt = ov8830_enum_fmt,
	.enum_mbus_fsizes = ov8830_enum_framesizes,
	.enum_framesizes = ov8830_enum_framesizes,
	.enum_frameintervals = ov8830_enum_frameintervals,
	.g_parm = ov8830_g_parm,
	.s_parm = ov8830_s_parm,
};
static int ov8830_g_skip_frames(struct v4l2_subdev *sd, u32 *frames)
{
	/* Quantity of initial bad frames to skip. Revisit. */
	*frames = 1;

	return 0;
}

static int ov8830_g_interface_parms(struct v4l2_subdev *sd,
				    struct v4l2_subdev_sensor_interface_parms
				    *parms)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ov8830 *ov8830 = to_ov8830(client);

	if (!parms)
		return -EINVAL;
	parms->if_type = ov8830->plat_parms->if_type;
	parms->if_mode = ov8830->plat_parms->if_mode;
	parms->parms = ov8830->plat_parms->parms;

	/* set the hs term time */
	parms->parms.serial.hs_term_time = 0;
	parms->parms.serial.hs_settle_time = 2;

	return 0;
}

static struct v4l2_subdev_sensor_ops ov8830_subdev_sensor_ops = {
	.g_skip_frames = ov8830_g_skip_frames,
	.g_interface_parms = ov8830_g_interface_parms,
};

static struct v4l2_subdev_ops ov8830_subdev_ops = {
	.core = &ov8830_subdev_core_ops,
	.video = &ov8830_subdev_video_ops,
	.sensor = &ov8830_subdev_sensor_ops,
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

static int ov8830_probe(struct i2c_client *client,
			const struct i2c_device_id *did)
{
	struct ov8830 *ov8830;
	struct soc_camera_device *icd = client->dev.platform_data;
	struct soc_camera_link *icl;
	int ret;

	printk(KERN_ERR "ov8830_probe\n");

	client->addr = 0x20>>1;



	if (!icd) {
		dev_err(&client->dev, "ov8830: missing soc-camera data!\n");
		return -EINVAL;
	}

	icl = to_soc_camera_link(icd);
	if (!icl) {
		dev_err(&client->dev, "ov8830 driver needs platform data\n");
		return -EINVAL;
	}

	if (!icl->priv) {
		dev_err(&client->dev,
			"ov8830 driver needs i/f platform data\n");
		return -EINVAL;
	}

	ov8830 = kzalloc(sizeof(struct ov8830), GFP_KERNEL);
	if (!ov8830)
		return -ENOMEM;

	v4l2_i2c_subdev_init(&ov8830->subdev, client, &ov8830_subdev_ops);

	/* Second stage probe - when a capture adapter is there */
	icd->ops = &ov8830_ops;

	ov8830->mode_idx = OV8830_MODE_MAX;
	ov8830->i_fmt = 0;	/* First format in the list */
	ov8830->plat_parms = icl->priv;
	ov8830->icd = icd;

	ret = ov8830_video_probe(icd, client);
	if (ret) {
		icd->ops = NULL;
		kfree(ov8830);
		return ret;
	}

	/* init the sensor here */
	ret = ov8830_init(client);

	if (ret) {
		dev_err(&client->dev, "Failed to initialize sensor\n");
		ret = -EINVAL;
	}

#if defined(CONFIG_VIDEO_A3907)
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
#elif defined(CONFIG_VIDEO_DW9714)
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

	printk(KERN_ERR "ov8830_probe!!!\n");

	return ret;
}

static int ov8830_remove(struct i2c_client *client)
{
	struct ov8830 *ov8830 = to_ov8830(client);
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
	ov8830_video_remove(icd);
	client->driver = NULL;
	kfree(ov8830);

	return 0;
}

static const struct i2c_device_id ov8830_id[] = {
	{"ov8830", 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, ov8830_id);

static struct i2c_driver ov8830_i2c_driver = {
	.driver = {
		   .name = "ov8830",
		   },
	.probe = ov8830_probe,
	.remove = ov8830_remove,
	.id_table = ov8830_id,
};

static struct proc_dir_entry *proc_entry;

static int ov8830_proc_write(struct file *filp, const char __user *buffer,
	unsigned long count, void *data)
{
	unsigned char kbuf[64] = {'\0'};
	u32 u4CopyBufSize = (count < (sizeof(kbuf)-1)) \
		? (count) : ((sizeof(kbuf)-1));
	int rc, iRegister = 0;
	int iValue = 0xff;
	u8 u8Value = 0xff;
	int iValue2 = 0xff;
	int iGain = 0;
	SENSORDB("enter,%d,0x%x", count, s_ov8830_i2c_client);
	rc = copy_from_user(kbuf, buffer, u4CopyBufSize);
	if (rc) {
		printk(KERN_INFO "proc write copy error");
		return -EFAULT;
	}
	if (sscanf(kbuf, "%x %d %d", &iRegister, &iValue, &iValue2) == 3) {
		SENSORDB("rg=0x%x,value=%d,value2=%d",\
			iRegister, iValue, iValue2);
		if (1 == iRegister) {
			ov8830_lens_set_position(s_ov8830_i2c_client, iValue2);
		} else if (2 == iRegister) {
			ov8830_set_exposure(s_ov8830_i2c_client, iValue);
			ov8830_set_gain(s_ov8830_i2c_client, iValue2);
		}
	} else if (sscanf(kbuf, "%x %x", &iRegister, &iValue) == 2) {
		rc = ov8830_reg_write(s_ov8830_i2c_client,\
			iRegister, (u8)iValue);
		SENSORDB("rg=0x%x,w_v=0x%x", iRegister, iValue);
	} else if (sscanf(kbuf, "%x", &iRegister) == 1) {
		ov8830_reg_read(s_ov8830_i2c_client, iRegister, &u8Value);
		SENSORDB("rg=0x%x, r_v=0x%x", iRegister, u8Value);
	}

	return count;
}

static int ov8830_proc_read(char *page, char **start, off_t off,
	int count, int *eof, void *data)
{
#if 1
	unsigned int len = 0;
	SENSORDB("data=%s,off=%d", data, off);
	printk(KERN_INFO "ov8830_proc_read: data=%s,off=%d\n", data, off);

	if (off > 0)
		return 0;

	/*len = sprintf(page, "Debug print is %s\n", \
		dev->debug ? "enabled" : "disabled");*/

	return len;
#else
	return 0;
#endif
}

static int __init ov8830_mod_init(void)
{
	proc_entry = create_proc_entry("driver/sensordrv", 0666, NULL);
	if (proc_entry == NULL) {
		printk(KERN_INFO "fortune: Couldn't create proc entry\n");
	} else {
		proc_entry->read_proc = ov8830_proc_read;
		proc_entry->write_proc = ov8830_proc_write;
	}

	return i2c_add_driver(&ov8830_i2c_driver);
}

static void __exit ov8830_mod_exit(void)
{
	remove_proc_entry("driver/sensordrv", NULL);

	i2c_del_driver(&ov8830_i2c_driver);
}

module_init(ov8830_mod_init);
module_exit(ov8830_mod_exit);

MODULE_DESCRIPTION("OmniVision OV8830 Camera driver");
MODULE_AUTHOR("Broadcom Corporation");
MODULE_LICENSE("GPL v2");

