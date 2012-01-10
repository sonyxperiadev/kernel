/*
 * OmniVision OV5640 sensor driver
 *
 * Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/videodev2.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/log2.h>
#include <linux/delay.h>

#include <media/v4l2-subdev.h>
#include <media/v4l2-chip-ident.h>
#include <media/soc_camera.h>

#define ENABLE_COLOR_PATTERN 0
//#define MIPI_2_LANES
//#define FPS_30_MODE		

#define OV5640_BRIGHTNESS_MIN           0
#define OV5640_BRIGHTNESS_MAX           200
#define OV5640_BRIGHTNESS_STEP          100
#define OV5640_BRIGHTNESS_DEF           100

#define OV5640_CONTRAST_MIN				0
#define OV5640_CONTRAST_MAX				200
#define OV5640_CONTRAST_STEP            100
#define OV5640_CONTRAST_DEF				100

/* OV5640 has only one fixed colorspace per pixelcode */
struct ov5640_datafmt {
	enum v4l2_mbus_pixelcode	code;
	enum v4l2_colorspace		colorspace;
};

struct ov5640_timing_cfg {
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
	u8 isp_scale_down;
	u8 clk_dividers;
	u8 mipi_lanes;
	
};

static const struct ov5640_datafmt ov5640_fmts[] = {
	/*
	 * Order important: first natively supported,
	 * second supported with a GPIO extender
	 */
	{V4L2_MBUS_FMT_UYVY8_2X8, V4L2_COLORSPACE_JPEG},
	{V4L2_MBUS_FMT_YUYV8_2X8, V4L2_COLORSPACE_JPEG},
};

enum ov5640_size {
	OV5640_SIZE_QVGA,       // 320 x 240
	OV5640_SIZE_VGA,        // 640 x 480
	OV5640_SIZE_XGA,        // 1024 x 768 
	OV5640_SIZE_720P,
	OV5640_SIZE_UXGA,       // 1600 x 1200 (2M)
	OV5640_SIZE_1080P,
	OV5640_SIZE_QXGA,       // 2048 x 1536 (3M)
	OV5640_SIZE_5MP,
	OV5640_SIZE_LAST,
};

static const struct v4l2_frmsize_discrete ov5640_frmsizes[OV5640_SIZE_LAST] = {
	{  320,  240 },
	{  640,  480 },
	{ 1024,  768 },
	{ 1280,  720 },
	{ 1600, 1200 },
	{ 1920, 1080 },
	{ 2048, 1536 },
	{ 2592, 1944 },
};

/* Find a data format by a pixel code in an array */
static int ov5640_find_datafmt(enum v4l2_mbus_pixelcode code)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(ov5640_fmts); i++)
		if (ov5640_fmts[i].code == code)
			break;

	/* If not found, select latest */
	if (i >= ARRAY_SIZE(ov5640_fmts))
		i = ARRAY_SIZE(ov5640_fmts) - 1;

	return i;
}

/* Find a frame size in an array */
static int ov5640_find_framesize(u32 width, u32 height)
{
	int i;

	for (i = 0; i < OV5640_SIZE_LAST; i++) {
		if ((ov5640_frmsizes[i].width >= width) &&
		    (ov5640_frmsizes[i].height >= height))
			break;
	}

	/* If not found, select biggest */
	if (i >= OV5640_SIZE_LAST)
		i = OV5640_SIZE_LAST - 1;

	return i;
}

struct ov5640 {
	struct v4l2_subdev subdev;
//	struct v4l2_subdev_sensor_interface_parms *plat_parms;
	int i_size;
	int i_fmt;
	int brightness;
	int contrast;
	int colorlevel;
};

static struct ov5640 *to_ov5640(const struct i2c_client *client)
{
	return container_of(i2c_get_clientdata(client), struct ov5640, subdev);
}

/**
 * struct ov5640_reg - ov5640 register format
 * @reg: 16-bit offset to register
 * @val: 8/16/32-bit register value
 * @length: length of the register
 *
 * Define a structure for OV5640 register initialization values
 */
struct ov5640_reg {
	u16	reg;
	u8	val;
};

/* TODO: Divide this properly */
static const struct ov5640_reg configscript_common1[] = {
//System Control
    {0x3008,0x82},      // Reset [7] PowerDn [6]
    {0xFFFF,  3 },      // Sleep 3ms
    {0x3008,0x42},
    {0x3103,0x03},      //PLL Clock Select    
//IO Config
    {0x3017,0x00},      //IO [3:0] D9-D6 (MIPI MD1-D9:D8 MC-D7:D6)
    {0x3018,0x00},      //IO [7:2] D5-D0 (MIPI MD0-D5:D4) [1]GPIO1 [0]GPIO0 (MIPI MD2/MC/MD1)
//????
    {0x3630,0x2e},
    {0x3632,0xe2},
    {0x3633,0x23},
    {0x3634,0x44},
    {0x3621,0xe0},
    {0x3704,0xa0},
    {0x3703,0x5a},
    {0x3715,0x78},
    {0x3717,0x01},
    {0x370b,0x60},
    {0x3705,0x1a},
    {0x3905,0x02},
    {0x3906,0x10},
    {0x3901,0x0a},
    {0x3731,0x12},
//VCM Control
    {0x3600,0x04},      //VCM Control
    {0x3601,0x22},
//????
    {0x471c, 0x50},
//System/IO pad Control
    {0x3000,0x00},      //Resets
    {0x3002,0x1c},
    {0x3004,0xff},      //Clocks
    {0x3006,0xc3},
//????
    {0x302e,0x08},      //undocumented
    {0x3612,0x4b},
    {0x3618,0x04},
// CLKS = Src=13Mhz:  300Mbps 8-bit    
    {0x3037,0x12},      //PLL Pre-Div [0:3], /2=6.5Mhz   PLL Root Div [4] /1=806Mhz
    {0x3036,0x70},      //PLL Mult 4~252 0:7  0x70=112=728Mhz
    {0x3108,0x01},      //SclkDiv [1:0] 1=/2  Sclk2*Div [3:2] 0=/1 PclkDiv [5:4]0=/1   [1,2,4,8]
//MIPI Control    
	{0x4800,0x24},
    {0x3034,0x18},      //MIPI BIT_Mode 3:0
#ifdef MIPI_2_LANES 
    {0x3035,0x12},      //SystemClkDiv 7:4, /1=728Mhz  MIPI Sclk Div 3:0, /1=728Mhz
#else
    {0x3035,0x11},      //SystemClkDiv 7:4, /1=364Mhz  MIPI Sclk Div 3:0, /1=364Mhz
#endif    
//PLL ADCLK
    {0x303d,0x20},      //PreDivSp [5:4] /2=6.5Mhz 
    {0x303b,0x20},      //DivCntsb [4:0] *32=208Mhz 
//????
    {0x3708,0x21},
    {0x3709,0x12},
    {0x370c,0x00},
// Sensor Timing control
    {0x3800,0x00},      //X start
    {0x3801,0x00},
    {0x3802,0x00},      //Y start
    {0x3803,0x00},
    {0x3804,0x0a},      //X end
    {0x3805,0x3f},
    {0x3806,0x07},      //Y end
    {0x3807,0x9f},
// Output size
    {0x3808,0x0a},      //output X  2592
    {0x3809,0x20},
    {0x380a,0x07},      //output Y  1944
    {0x380b,0x98},
// Total size (+blanking)
    {0x380c,0x0b},      //Total X  2844
    {0x380d,0x1c},
    {0x380e,0x07},      //Total Y  1968
    {0x380f,0xb0},
// ISP Windowing size
    {0x3810,0x00},      //ISP X offset = 16
    {0x3811,0x10},
    {0x3812,0x00},      //ISP Y offset = 6
    {0x3813,0x06},
// Sensor Read
    {0x3814,0x11},      //X incr
    {0x3815,0x11},      //Y incr
    {0x3820,0x40},      //vflip
    {0x3821,0x06},      //mirror
    {0x3824,0x01},      //Scale Divider [4:0]
//AEC/AGC
    {0x3a02,0x01},
    {0x3a03,0xec},
    {0x3a08,0x01},
    {0x3a09,0x27},
    {0x3a0a,0x00},
    {0x3a0b,0xf6},
    {0x3a0e,0x06},
    {0x3a0d,0x08},
    {0x3a14,0x01},
    {0x3a15,0xec},
//BLC Control
    {0x4001,0x02},
    {0x4004,0x06},
//Format control
    {0x4300,0x32},      //Output Format[7:4] Sequence[3:0] (UVYV)
    {0x501f,0x00},      //ISP Format
//JPG Control
    {0x4713,0x02},      //JPG Mode Select
//????
    {0x460b,0x37},      
    {0x4750,0x00},      //???
    {0x4751,0x00},      //???
//ISP Control
    {0x5000,0x07},
    {0x5001,0xa7},      //isp scale down enabled  Special Effects
    {0x501d,0x00},      //ISP Misc
//???   
    {0x5a00,0x08},
    {0x5a21,0x00},
    {0x5a24,0x00},
//Gamma Control
    {0x5481,0x08},
    {0x5482,0x14},
    {0x5483,0x28},
    {0x5484,0x51},
    {0x5485,0x65},
    {0x5486,0x71},
    {0x5487,0x7d},
    {0x5488,0x87},
    {0x5489,0x91},
    {0x548a,0x9a},
    {0x548b,0xaa},
    {0x548c,0xb8},
    {0x548d,0xcd},
    {0x548e,0xdd},
    {0x548f,0xea},
    {0x5490,0x1d},
//CMX Control
    {0x5381,0x20},
    {0x5382,0x64},
    {0x5383,0x08},
    {0x5384,0x20},
    {0x5385,0x80},
    {0x5386,0xa0},
    {0x5387,0xa2},
    {0x5388,0xa0},
    {0x5389,0x02},
    {0x538a,0x01},
    {0x538b,0x98},
//CIP Control
    {0x5300,0x08},
    {0x5301,0x30},
    {0x5302,0x10},
    {0x5303,0x00},
    {0x5304,0x08},
    {0x5305,0x30},
    {0x5306,0x08},
    {0x5307,0x16},
//SDE Control
    {0x5580,0x00},
    {0x5583,0x40},
    {0x5584,0x10},
	{0x5587,0x00},
	{0x5588,0x00},
    {0x5589,0x10},
    {0x558a,0x00},
    {0x558b,0xf8},
//AEC Controls
    {0x3a0f,0x36},
    {0x3a10,0x2e},
    {0x3a1b,0x38},
    {0x3a1e,0x2c},
    {0x3a11,0x70},
    {0x3a1f,0x18},
    {0x3a18,0x00},      //Real Gain
    {0x3a19,0xf8},

//System Reset
	{0x3003,0x03},
	{0x3003,0x01},
    
    #if ENABLE_COLOR_PATTERN
        {0x503d,0x80},   // Solid Colour Bars
        #if 0   
            {0x503d,0x80},   // Solid Colour Bars
            {0x503d,0x81},   // Gradual change @ vertical mode 1
            {0x503d,0x82},   // Gradual change horizontal
            {0x503d,0x83},   // Gradual change @ vertical mode 2
        #endif
    #endif
	{0x3008,0x42},      //stop sensor streaming
    {0x300e,0x3d},      //MIPI Control  Single Lane/Powered down       ********************

   { 0xFFFF, 0x00 }
};

static const struct ov5640_timing_cfg timing_cfg[OV5640_SIZE_LAST] = {
	[OV5640_SIZE_QVGA] = {
		.x_addr_start = 0,
		.y_addr_start = 0,
		.x_addr_end = 2623,
		.y_addr_end = 1951,
		.h_output_size = 320,
		.v_output_size = 240,
		.isp_h_offset = 16,
		.isp_v_offset = 6,
    #ifdef FPS_30_MODE		
		.h_total_size = 2176,
		.v_total_size = 1632,
		.h_odd_ss_inc = 1,
		.h_even_ss_inc = 3,
		.v_odd_ss_inc = 1,
		.v_even_ss_inc = 3,
    #else		
		.h_total_size = 2844,
		.v_total_size = 1968,
		.h_odd_ss_inc = 1,
		.h_even_ss_inc = 1,
		.v_odd_ss_inc = 1,
		.v_even_ss_inc = 1,
    #endif		
    	.isp_scale_down = 0x23,
    	.clk_dividers = 0x01,
    #ifdef MIPI_2_LANES 
    	.mipi_lanes = 0x45,
    #else
    	.mipi_lanes = 0x25,
    #endif    	
	},
	[OV5640_SIZE_VGA] = {
		.x_addr_start = 0,
		.y_addr_start = 0,
		.x_addr_end = 2623,
		.y_addr_end = 1951,
		.h_output_size = 640,
		.v_output_size = 480,
		.isp_h_offset = 16,
		.isp_v_offset = 6,
    #ifdef FPS_30_MODE		
		.h_total_size = 2176,
		.v_total_size = 1632,
		.h_odd_ss_inc = 1,
		.h_even_ss_inc = 3,
		.v_odd_ss_inc = 1,
		.v_even_ss_inc = 3,
    #else		
		.h_total_size = 2844,
		.v_total_size = 1968,
		.h_odd_ss_inc = 1,
		.h_even_ss_inc = 1,
		.v_odd_ss_inc = 1,
		.v_even_ss_inc = 1,
    #endif		
    	.isp_scale_down = 0x23,
    	.clk_dividers = 0x01,
    #ifdef MIPI_2_LANES 
    	.mipi_lanes = 0x45,
    #else
    	.mipi_lanes = 0x25,
    #endif    	
	},
	[OV5640_SIZE_XGA] = {
		.x_addr_start = 0,
		.y_addr_start = 0,
		.x_addr_end = 2623,
		.y_addr_end = 1951,
		.h_output_size = 1024,
		.v_output_size = 768,
		.h_total_size = 2844,
		.v_total_size = 1968,
		.isp_h_offset = 16,
		.isp_v_offset = 6,
		.h_odd_ss_inc = 1,
		.h_even_ss_inc = 1,
		.v_odd_ss_inc = 1,
		.v_even_ss_inc = 1,
    	.isp_scale_down = 0x23,
    	.clk_dividers = 0x01,
    #ifdef MIPI_2_LANES 
    	.mipi_lanes = 0x45,
    #else
    	.mipi_lanes = 0x25,
    #endif    	
	},
	[OV5640_SIZE_720P] = {
		.x_addr_start = 336,
		.y_addr_start = 434,
		.x_addr_end = 2287,
		.y_addr_end = 1522,
		.h_output_size = 1280,
		.v_output_size = 720,
		.h_total_size = 2500,
		.v_total_size = 1120,
		.isp_h_offset = 16,
		.isp_v_offset = 4,
		.h_odd_ss_inc = 1,
		.h_even_ss_inc = 1,
		.v_odd_ss_inc = 1,
		.v_even_ss_inc = 1,
    	.isp_scale_down = 0x23,
    	.clk_dividers = 0x01,
    #ifdef MIPI_2_LANES 
    	.mipi_lanes = 0x45,
    #else
    	.mipi_lanes = 0x25,
    #endif    	
	},
	[OV5640_SIZE_UXGA] = {
		.x_addr_start = 0,
		.y_addr_start = 0,
		.x_addr_end = 2623,
		.y_addr_end = 1951,
		.h_output_size = 1600,
		.v_output_size = 1200,
		.h_total_size = 2844,
		.v_total_size = 1968,
		.isp_h_offset = 16,
		.isp_v_offset = 6,
		.h_odd_ss_inc = 1,
		.h_even_ss_inc = 1,
		.v_odd_ss_inc = 1,
		.v_even_ss_inc = 1,
    	.isp_scale_down = 0x23,
    	.clk_dividers = 0x01,
    #ifdef MIPI_2_LANES 
    	.mipi_lanes = 0x45,
    #else
    	.mipi_lanes = 0x25,
    #endif    	
	},
	[OV5640_SIZE_1080P] = {
		.x_addr_start = 336,
		.y_addr_start = 434,
		.x_addr_end = 2287,
		.y_addr_end = 1522,
		.h_output_size = 1920,
		.v_output_size = 1080,
		.h_total_size = 2500,
		.v_total_size = 1120,
		.isp_h_offset = 16,
		.isp_v_offset = 4,
		.h_odd_ss_inc = 1,
		.h_even_ss_inc = 1,
		.v_odd_ss_inc = 1,
		.v_even_ss_inc = 1,
    	.isp_scale_down = 0x03,
    	.clk_dividers = 0x02,
    #ifdef MIPI_2_LANES 
    	.mipi_lanes = 0x45,
    #else
    	.mipi_lanes = 0x25,
    #endif    	
	},
	[OV5640_SIZE_QXGA] = {
		.x_addr_start = 0,
		.y_addr_start = 0,
		.x_addr_end = 2623,
		.y_addr_end = 1951,
		.h_output_size = 2048,
		.v_output_size = 1536,
		.h_total_size = 2844,
		.v_total_size = 1968,
		.isp_h_offset = 16,
		.isp_v_offset = 6,
		.h_odd_ss_inc = 1,
		.h_even_ss_inc = 1,
		.v_odd_ss_inc = 1,
		.v_even_ss_inc = 1,
    	.isp_scale_down = 0x23,
    	.clk_dividers = 0x02,
    #ifdef MIPI_2_LANES 
    	.mipi_lanes = 0x45,
    #else
    	.mipi_lanes = 0x25,
    #endif    	
	},
	[OV5640_SIZE_5MP] = {
		.x_addr_start = 0,
		.y_addr_start = 0,
		.x_addr_end = 2623,
		.y_addr_end = 1951,
		.h_output_size = 2592,
		.v_output_size = 1944,
		.h_total_size = 2844,
		.v_total_size = 1968,
		.isp_h_offset = 16,
		.isp_v_offset = 6,
		.h_odd_ss_inc = 1,
		.h_even_ss_inc = 1,
		.v_odd_ss_inc = 1,
		.v_even_ss_inc = 1,
    	.isp_scale_down = 0x03,
    	.clk_dividers = 0x02,
    #ifdef MIPI_2_LANES 
    	.mipi_lanes = 0x45,
    #else
    	.mipi_lanes = 0x25,
    #endif    	
	},
};
#if 0
static struct v4l2_subdev_sensor_serial_parms mipi_cfgs[OV5640_SIZE_LAST] = {
	[OV5640_SIZE_QVGA] = {
		.lanes = 1,
		.channel = 0,
		.phy_rate = (336 * 2 * 1000000),
		.pix_clk = 21, /* Revisit */
	},
	[OV5640_SIZE_VGA] = {
		.lanes = 1,
		.channel = 0,
		.phy_rate = (336 * 2 * 1000000),
		.pix_clk = 21, /* Revisit */
	},
	[OV5640_SIZE_XGA] = {
		.lanes = 1,
		.channel = 0,
		.phy_rate = (336 * 2 * 1000000),
		.pix_clk = 21, /* Revisit */
	},
	[OV5640_SIZE_1080P] = {
		.lanes = 1,
		.channel = 0,
		.phy_rate = (336 * 2 * 1000000),
		.pix_clk = 21, /* Revisit */
	},
	[OV5640_SIZE_UXGA] = {
		.lanes = 1,
		.channel = 0,
		.phy_rate = (336 * 2 * 1000000),
		.pix_clk = 21, /* Revisit */
	},
	[OV5640_SIZE_720P] = {
		.lanes = 1,
		.channel = 0,
		.phy_rate = (336 * 2 * 1000000),
		.pix_clk = 21, /* Revisit */
	},
	[OV5640_SIZE_QXGA] = {
		.lanes = 1,
		.channel = 0,
		.phy_rate = (336 * 2 * 1000000),
		.pix_clk = 21, /* Revisit */
	},
	[OV5640_SIZE_5MP] = {
		.lanes = 1,
		.channel = 0,
		.phy_rate = (336 * 2 * 1000000),
		.pix_clk = 21, /* Revisit */
	},
};
#endif

/**
 * ov5640_reg_read - Read a value from a register in an ov5640 sensor device
 * @client: i2c driver client structure
 * @reg: register address / offset
 * @val: stores the value that gets read
 *
 * Read a value from a register in an ov5640 sensor device.
 * The value is returned in 'val'.
 * Returns zero if successful, or non-zero otherwise.
 */
static int ov5640_reg_read(struct i2c_client *client, u16 reg, u8 *val)
{
	int ret;
	u8 data[2] = {0};
	struct i2c_msg msg = {
		.addr	= client->addr,
		.flags	= 0,
		.len	= 2,
		.buf	= data,
	};

	data[0] = (u8)(reg >> 8);
	data[1] = (u8)(reg & 0xff);

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
 * Write a value to a register in ov5640 sensor device.
 * @client: i2c driver client structure.
 * @reg: Address of the register to read value from.
 * @val: Value to be written to a specific register.
 * Returns zero if successful, or non-zero otherwise.
 */
static int ov5640_reg_write(struct i2c_client *client, u16 reg, u8 val)
{
	int ret;
	unsigned char data[3] = { (u8)(reg >> 8), (u8)(reg & 0xff), val };
	struct i2c_msg msg = {
		.addr	= client->addr,
		.flags	= 0,
		.len	= 3,
		.buf	= data,
	};

	ret = i2c_transfer(client->adapter, &msg, 1);
	if (ret < 0) {
		dev_err(&client->dev, "Failed writing register 0x%02x!\n", reg);
		return ret;
	}

	return 0;
}

static const struct v4l2_queryctrl ov5640_controls[] = {
	{
		.id      	= V4L2_CID_BRIGHTNESS,
		.type    	= V4L2_CTRL_TYPE_INTEGER,
		.name    	= "Brightness",
		.minimum	= OV5640_BRIGHTNESS_MIN,
		.maximum	= OV5640_BRIGHTNESS_MAX,
		.step		= OV5640_BRIGHTNESS_STEP,
		.default_value	= OV5640_BRIGHTNESS_DEF,
	},
	{
		.id		= V4L2_CID_CONTRAST,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.name		= "Contrast",
		.minimum	= OV5640_CONTRAST_MIN,
		.maximum	= OV5640_CONTRAST_MAX,
		.step		= OV5640_CONTRAST_STEP,
		.default_value	= OV5640_CONTRAST_DEF,
	},
	{
		.id		= V4L2_CID_COLORFX,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.name		= "Color Effects",
		.minimum	= V4L2_COLORFX_NONE,
		.maximum	= V4L2_COLORFX_NEGATIVE,
		.step		= 1,
		.default_value	= V4L2_COLORFX_NONE,
	},
};

/**
 * Initialize a list of ov5640 registers.
 * The list of registers is terminated by the pair of values
 * @client: i2c driver client structure.
 * @reglist[]: List of address of the registers to write data.
 * Returns zero if successful, or non-zero otherwise.
 */
static int ov5640_reg_writes(struct i2c_client *client,
			     const struct ov5640_reg reglist[])
{
	int err = 0, index;

    for (index=0; ((reglist[index].reg != 0xFFFF) && (err == 0)); index++)
    {
        err |= ov5640_reg_write( client, reglist[index].reg, reglist[index].val );
    // Check for Pause condition
        if ( (reglist[index+1].reg == 0xFFFF) && (reglist[index+1].val != 0) )
        {
            msleep(reglist[index+1].val);
            index += 1;
       }
    }
	return 0;
}

static int ov5640_reg_set(struct i2c_client *client, u16 reg, u8 val)
{
	int ret;
	u8 tmpval = 0;

	ret = ov5640_reg_read(client, reg, &tmpval);
	if (ret)
		return ret;

	return ov5640_reg_write(client, reg, tmpval | val);
}

static int ov5640_reg_clr(struct i2c_client *client, u16 reg, u8 val)
{
	int ret;
	u8 tmpval = 0;

	ret = ov5640_reg_read(client, reg, &tmpval);
	if (ret)
		return ret;

	return ov5640_reg_write(client, reg, tmpval & ~val);
}

static int ov5640_config_timing(struct i2c_client *client)
{
	struct ov5640 *ov5640 = to_ov5640(client);
	int ret, i = ov5640->i_size;

	ret = ov5640_reg_write(client,
			0x3800,
			(timing_cfg[i].x_addr_start & 0xFF00) >> 8);
	if (ret)
		return ret;

	ret = ov5640_reg_write(client,
			0x3801,
			timing_cfg[i].x_addr_start & 0xFF);
	if (ret)
		return ret;

	ret = ov5640_reg_write(client,
			0x3802,
			(timing_cfg[i].y_addr_start & 0xFF00) >> 8);
	if (ret)
		return ret;

	ret = ov5640_reg_write(client,
			0x3803,
			timing_cfg[i].y_addr_start & 0xFF);
	if (ret)
		return ret;

	ret = ov5640_reg_write(client,
			0x3804,
			(timing_cfg[i].x_addr_end & 0xFF00) >> 8);
	if (ret)
		return ret;

	ret = ov5640_reg_write(client,
			0x3805,
			timing_cfg[i].x_addr_end & 0xFF);
	if (ret)
		return ret;

	ret = ov5640_reg_write(client,
			0x3806,
			(timing_cfg[i].y_addr_end & 0xFF00) >> 8);
	if (ret)
		return ret;

	ret = ov5640_reg_write(client,
			0x3807,
			timing_cfg[i].y_addr_end & 0xFF);
	if (ret)
		return ret;

	ret = ov5640_reg_write(client,
			0x3808,
			(timing_cfg[i].h_output_size & 0xFF00) >> 8);
	if (ret)
		return ret;

	ret = ov5640_reg_write(client,
			0x3809,
			timing_cfg[i].h_output_size & 0xFF);
	if (ret)
		return ret;

	ret = ov5640_reg_write(client,
			0x380A,
			(timing_cfg[i].v_output_size & 0xFF00) >> 8);
	if (ret)
		return ret;

	ret = ov5640_reg_write(client,
			0x380B,
			timing_cfg[i].v_output_size & 0xFF);
	if (ret)
		return ret;

	ret = ov5640_reg_write(client,
			0x380C,
			(timing_cfg[i].h_total_size & 0xFF00) >> 8);
	if (ret)
		return ret;

	ret = ov5640_reg_write(client,
			0x380D,
			timing_cfg[i].h_total_size & 0xFF);
	if (ret)
		return ret;

	ret = ov5640_reg_write(client,
			0x380E,
			(timing_cfg[i].v_total_size & 0xFF00) >> 8);
	if (ret)
		return ret;

	ret = ov5640_reg_write(client,
			0x380F,
			timing_cfg[i].v_total_size & 0xFF);
	if (ret)
		return ret;

	ret = ov5640_reg_write(client,
			0x3810,
			(timing_cfg[i].isp_h_offset & 0xFF00) >> 8);
	if (ret)
		return ret;

	ret = ov5640_reg_write(client,
			0x3811,
			timing_cfg[i].isp_h_offset & 0xFF);
	if (ret)
		return ret;

	ret = ov5640_reg_write(client,
			0x3812,
			(timing_cfg[i].isp_v_offset & 0xFF00) >> 8);
	if (ret)
		return ret;

	ret = ov5640_reg_write(client,
			0x3813,
			timing_cfg[i].isp_v_offset & 0xFF);
	if (ret)
		return ret;

	ret = ov5640_reg_write(client,
			0x3814,
			((timing_cfg[i].h_odd_ss_inc & 0xF) << 4) |
			(timing_cfg[i].h_even_ss_inc & 0xF));
	if (ret)
		return ret;

	ret = ov5640_reg_write(client,
			0x3815,
			((timing_cfg[i].v_odd_ss_inc & 0xF) << 4) |
			(timing_cfg[i].v_even_ss_inc & 0xF));
			
	ret = ov5640_reg_write(client,
			0x3812,
			(timing_cfg[i].isp_v_offset & 0xFF00) >> 8);
	if (ret)
		return ret;
			
	ret = ov5640_reg_write(client,
			0x5001,
			timing_cfg[i].isp_scale_down & 0xFF);
	if (ret)
		return ret;

	ret = ov5640_reg_write(client,
			0x3108,
			timing_cfg[i].clk_dividers & 0xFF);
	if (ret)
		return ret;

	ret = ov5640_reg_write(client,
			0x300E,
			timing_cfg[i].mipi_lanes & 0xFF);
	if (ret)
		return ret;

	return ret;
}

static int ov5640_s_stream(struct v4l2_subdev *sd, int enable)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int ret = 0;

	if (enable) {
		ret = ov5640_reg_write(client, 0x3008, 0x02);
		if (ret)
			goto out;
	} else {
		u8 tmpreg = 0;

		ret = ov5640_reg_read(client, 0x3008, &tmpreg);
		if (ret)
			goto out;

		ret = ov5640_reg_write(client, 0x3008, tmpreg | 0x40);
		if (ret)
			goto out;

		ret = ov5640_reg_write(client, 0x300e, 0x3d);   //MIPI Control  Powered down  
		if (ret)
			goto out;
	}

out:
	return ret;
}

static int ov5640_set_bus_param(struct soc_camera_device *icd,
				 unsigned long flags)
{
	/* TODO: Do the right thing here, and validate bus params */
	return 0;
}

static unsigned long ov5640_query_bus_param(struct soc_camera_device *icd)
{
	unsigned long flags = SOCAM_PCLK_SAMPLE_FALLING |
		SOCAM_HSYNC_ACTIVE_HIGH | SOCAM_VSYNC_ACTIVE_HIGH |
		SOCAM_DATA_ACTIVE_HIGH | SOCAM_MASTER;

	/* TODO: Do the right thing here, and validate bus params */

	flags |= SOCAM_DATAWIDTH_10;

	return flags;
}

static int ov5640_g_fmt(struct v4l2_subdev *sd,
			 struct v4l2_mbus_framefmt *mf)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ov5640 *ov5640 = to_ov5640(client);

	mf->width	= ov5640_frmsizes[ov5640->i_size].width;
	mf->height	= ov5640_frmsizes[ov5640->i_size].height;
	mf->code	= ov5640_fmts[ov5640->i_fmt].code;
	mf->colorspace	= ov5640_fmts[ov5640->i_fmt].colorspace;
	mf->field	= V4L2_FIELD_NONE;

	return 0;
}

static int ov5640_try_fmt(struct v4l2_subdev *sd,
			   struct v4l2_mbus_framefmt *mf)
{
	int i_fmt;
	int i_size;

	i_fmt = ov5640_find_datafmt(mf->code);

	mf->code = ov5640_fmts[i_fmt].code;
	mf->colorspace	= ov5640_fmts[i_fmt].colorspace;
	mf->field	= V4L2_FIELD_NONE;

	i_size = ov5640_find_framesize(mf->width, mf->height);

	mf->width = ov5640_frmsizes[i_size].width;
	mf->height = ov5640_frmsizes[i_size].height;

	return 0;
}

static int ov5640_s_fmt(struct v4l2_subdev *sd,
			 struct v4l2_mbus_framefmt *mf)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ov5640 *ov5640 = to_ov5640(client);
	int ret = 0;
	u8 fmtreg = 0, fmtmuxreg = 0;

	ret = ov5640_try_fmt(sd, mf);
	if (ret < 0)
		return ret;

	ov5640->i_size = ov5640_find_framesize(mf->width, mf->height);
	ov5640->i_fmt = ov5640_find_datafmt(mf->code);

	switch ((u32)ov5640_fmts[ov5640->i_fmt].code) {
	case V4L2_MBUS_FMT_UYVY8_2X8:
		fmtreg = 0x32;
		fmtmuxreg = 0;
		break;
	case V4L2_MBUS_FMT_YUYV8_2X8:
		fmtreg = 0x30;
		fmtmuxreg = 0;
		break;
	default:
		/* This shouldn't happen */
		ret = -EINVAL;
		return ret;
	}

//stop sensor streaming
	ret = ov5640_reg_write(client, 0x3008, 0x42);
	if (ret)
		return ret;

	ret = ov5640_reg_write(client, 0x4300, fmtreg);
	if (ret)
		return ret;

	ret = ov5640_reg_write(client, 0x501F, fmtmuxreg);
	if (ret)
		return ret;

	ret = ov5640_config_timing(client);
	if (ret)
		return ret;

	return ret;
}

static int ov5640_g_chip_ident(struct v4l2_subdev *sd,
				struct v4l2_dbg_chip_ident *id)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	if (id->match.type != V4L2_CHIP_MATCH_I2C_ADDR)
		return -EINVAL;

	if (id->match.addr != client->addr)
		return -ENODEV;

	id->ident	= V4L2_IDENT_OV5640;
	id->revision	= 0;

	return 0;
}

static int ov5640_g_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ov5640 *ov5640 = to_ov5640(client);

	dev_dbg(&client->dev, "ov5640_g_ctrl\n");

	switch (ctrl->id) {
	case V4L2_CID_BRIGHTNESS:
		ctrl->value = ov5640->brightness;
		break;
	case V4L2_CID_CONTRAST:
		ctrl->value = ov5640->contrast;
		break;
	case V4L2_CID_COLORFX:
		ctrl->value = ov5640->colorlevel;
		break;
	}

	return 0;
}

static int ov5640_s_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ov5640 *ov5640 = to_ov5640(client);
	int i_sde_ctrl0 = 0;
	int i_sde_ctrl3 = 0, i_sde_ctrl4 = 0;
	int i_sde_ctrl5 = 0, i_sde_ctrl6 = 0;
	int i_sde_ctrl7 = 0, i_sde_ctrl8 = 0;
	int ret = 0;

	dev_dbg(&client->dev, "ov5640_s_ctrl\n");

	switch (ctrl->id) {
	case V4L2_CID_BRIGHTNESS:

		if (ctrl->value > OV5640_BRIGHTNESS_MAX)
			return -EINVAL;

		ov5640->brightness = ctrl->value;
		ret = ov5640_reg_write(client, 0x5580, 0x04);
		if (ret)
			return ret;
		switch(ov5640->brightness) {
		case OV5640_BRIGHTNESS_MIN:
			i_sde_ctrl7 = 0x40;
			i_sde_ctrl8 = 0x08;
		break;
		case OV5640_BRIGHTNESS_MAX:
			i_sde_ctrl7 = 0x40;
			i_sde_ctrl8 = 0x00;
		break;
		default:
			i_sde_ctrl7 = 0x00;
			i_sde_ctrl8 = 0x00;
		break;
		}
		ret = ov5640_reg_write(client, 0x5587, i_sde_ctrl7);
		if (ret)
			return ret;

		ret = ov5640_reg_write(client, 0x5588, i_sde_ctrl8);
		if (ret)
			return ret;
		break;
	case V4L2_CID_CONTRAST:

		if (ctrl->value > OV5640_CONTRAST_MAX)
			return -EINVAL;

		ov5640->contrast = ctrl->value;
		ret = ov5640_reg_write(client, 0x5580, 0x04);
		if (ret)
			return ret;
		switch(ov5640->contrast) {
		case OV5640_CONTRAST_MIN:
			i_sde_ctrl5 = 0x10;
			i_sde_ctrl6 = 0x10;
			i_sde_ctrl8 = 0x00;
		break;
		case OV5640_CONTRAST_MAX:
			i_sde_ctrl5 = 0x20;
			i_sde_ctrl6 = 0x30;
			i_sde_ctrl8 = 0x08;
		break;
		default:
			i_sde_ctrl5 = 0x00;
			i_sde_ctrl6 = 0x20;
			i_sde_ctrl8 = 0x00;
		break;
		}

		ret = ov5640_reg_write(client, 0x5585, i_sde_ctrl5);
		if (ret)
			return ret;

		ret = ov5640_reg_write(client, 0x5586, i_sde_ctrl6);
		if (ret)
			return ret;

		ret = ov5640_reg_write(client, 0x5588, i_sde_ctrl8);
		if (ret)
			return ret;
		break;
	case V4L2_CID_COLORFX:
		if (ctrl->value > V4L2_COLORFX_NEGATIVE)
			return -EINVAL;

		ov5640->colorlevel = ctrl->value;

		switch (ov5640->colorlevel) {
		case V4L2_COLORFX_BW:
			i_sde_ctrl0 = 0x18;
			i_sde_ctrl3 = 0x80;
			i_sde_ctrl4 = 0x80;
			break;
		case V4L2_COLORFX_SEPIA:
			i_sde_ctrl0 = 0x18;
			i_sde_ctrl3 = 0x40;
			i_sde_ctrl4 = 0xA0;
			break;
		case V4L2_COLORFX_NEGATIVE:
			i_sde_ctrl0 = 0x40;
			i_sde_ctrl3 = 0x00;
			i_sde_ctrl4 = 0x00;
			break;
		default:
			i_sde_ctrl0 = 0x00;
			i_sde_ctrl3 = 0x00;
			i_sde_ctrl4 = 0x00;
			break;
		}

		ret = ov5640_reg_write(client, 0x5580, i_sde_ctrl0);
		if (ret)
			return ret;

		ret = ov5640_reg_write(client, 0x5583, i_sde_ctrl3);
		if (ret)
			return ret;

		ret = ov5640_reg_write(client, 0x5584, i_sde_ctrl4);
		if (ret)
			return ret;
		break;
	}

	return ret;
}

#ifdef CONFIG_VIDEO_ADV_DEBUG
static int ov5640_g_register(struct v4l2_subdev *sd,
			     struct v4l2_dbg_register *reg)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	if (reg->match.type != V4L2_CHIP_MATCH_I2C_ADDR || reg->size > 2)
		return -EINVAL;

	if (reg->match.addr != client->addr)
		return -ENODEV;

	reg->size = 2;
	if (ov5640_reg_read(client, reg->reg, &reg->val))
		return -EIO

	return 0;
}

static int ov5640_s_register(struct v4l2_subdev *sd,
			     struct v4l2_dbg_register *reg)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	if (reg->match.type != V4L2_CHIP_MATCH_I2C_ADDR || reg->size > 2)
		return -EINVAL;

	if (reg->match.addr != client->addr)
		return -ENODEV;

	if (ov5640_reg_write(client, reg->reg, reg->val))
		return -EIO;

	return 0;
}
#endif

static struct soc_camera_ops ov5640_ops = {
	.set_bus_param		= ov5640_set_bus_param,
	.query_bus_param	= ov5640_query_bus_param,
	.controls		= ov5640_controls,
	.num_controls		= ARRAY_SIZE(ov5640_controls),
};

static int ov5640_init(struct i2c_client *client)
{
	struct ov5640 *ov5640 = to_ov5640(client);
	int ret = 0;

	ret = ov5640_reg_writes(client, configscript_common1);
	if (ret)
		goto out;

	/* default brightness and contrast */
	ov5640->brightness = OV5640_BRIGHTNESS_DEF;
	ov5640->contrast = OV5640_CONTRAST_DEF;
	ov5640->colorlevel = V4L2_COLORFX_NONE;

	dev_dbg(&client->dev, "Sensor initialized\n");

out:
	return ret;
}

/*
 * Interface active, can use i2c. If it fails, it can indeed mean, that
 * this wasn't our capture interface, so, we wait for the right one
 */
static int ov5640_video_probe(struct soc_camera_device *icd,
			      struct i2c_client *client)
{
	unsigned long flags;
	int ret = 0;
	u8 revision = 0;

	/*
	 * We must have a parent by now. And it cannot be a wrong one.
	 * So this entire test is completely redundant.
	 */
	if (!icd->dev.parent ||
	    to_soc_camera_host(icd->dev.parent)->nr != icd->iface)
		return -ENODEV;

	ret = ov5640_reg_read(client, 0x302A, &revision);
	if (ret) {
		dev_err(&client->dev, "Failure to detect OV5640 chip\n");
		goto out;
	}
	printk(KERN_ERR "OV5640 value read=%x\n", revision);

	revision &= 0xF;

	flags = SOCAM_DATAWIDTH_8;

	dev_info(&client->dev, "Detected a OV5640 chip, revision %x\n",
		 revision);

	/* TODO: Do something like ov5640_init */

out:
	return ret;
}

static void ov5640_video_remove(struct soc_camera_device *icd)
{
	dev_dbg(&icd->dev, "Video removed: %p, %p\n",
		icd->dev.parent, icd->vdev);
}

static struct v4l2_subdev_core_ops ov5640_subdev_core_ops = {
	.g_chip_ident	= ov5640_g_chip_ident,
	.g_ctrl		= ov5640_g_ctrl,
	.s_ctrl 	= ov5640_s_ctrl,
#ifdef CONFIG_VIDEO_ADV_DEBUG
	.g_register	= ov5640_g_register,
	.s_register	= ov5640_s_register,
#endif
};

static int ov5640_enum_fmt(struct v4l2_subdev *sd, unsigned int index,
			    enum v4l2_mbus_pixelcode *code)
{
	if (index >= ARRAY_SIZE(ov5640_fmts))
		return -EINVAL;

	*code = ov5640_fmts[index].code;
	return 0;
}

static int ov5640_enum_framesizes(struct v4l2_subdev *sd,
				  struct v4l2_frmsizeenum *fsize)
{
	if (fsize->index >= OV5640_SIZE_LAST)
		return -EINVAL;

	fsize->type = V4L2_FRMSIZE_TYPE_DISCRETE;
	fsize->pixel_format = V4L2_PIX_FMT_UYVY;

	fsize->discrete = ov5640_frmsizes[fsize->index];

	return 0;
}

static int ov5640_g_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *param)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ov5640 *ov5640 = to_ov5640(client);
	struct v4l2_captureparm *cparm;

	if (param->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;

	cparm = &param->parm.capture;

	memset(param, 0, sizeof(*param));
	param->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	cparm->capability = V4L2_CAP_TIMEPERFRAME;

	switch (ov5640->i_size) {
	case OV5640_SIZE_5MP:
		cparm->timeperframe.numerator = 2;
		cparm->timeperframe.denominator = 15;
		break;
	case OV5640_SIZE_QXGA:
	case OV5640_SIZE_1080P:
	case OV5640_SIZE_UXGA:
		cparm->timeperframe.numerator = 1;
		cparm->timeperframe.denominator = 15;
		break;
	case OV5640_SIZE_720P:
	case OV5640_SIZE_XGA:
	case OV5640_SIZE_VGA:
	case OV5640_SIZE_QVGA:
	default:
		cparm->timeperframe.numerator = 1;
		cparm->timeperframe.denominator = 24;
		break;
	}

	return 0;
}
static int ov5640_s_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *param)
{
	/*
	 * FIXME: This just enforces the hardcoded framerates until this is
	 * flexible enough.
	 */
	return ov5640_g_parm(sd, param);
}

static struct v4l2_subdev_video_ops ov5640_subdev_video_ops = {
	.s_stream	= ov5640_s_stream,
	.s_mbus_fmt	= ov5640_s_fmt,
	.g_mbus_fmt	= ov5640_g_fmt,
	.try_mbus_fmt	= ov5640_try_fmt,
	.enum_mbus_fmt	= ov5640_enum_fmt,
	.enum_mbus_fsizes = ov5640_enum_framesizes,
	.enum_framesizes = ov5640_enum_framesizes,
	.g_parm = ov5640_g_parm,
	.s_parm = ov5640_s_parm,
};
static int ov5640_g_skip_frames(struct v4l2_subdev *sd, u32 *frames)
{
	/* Quantity of initial bad frames to skip. Revisit. */
	*frames = 5;

	return 0;
}

#if 0
static int ov5640_g_interface_parms(struct v4l2_subdev *sd,
			struct v4l2_subdev_sensor_interface_parms *parms)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ov5640 *ov5640 = to_ov5640(client);

	if (!parms)
		return -EINVAL;

	parms->if_type = ov5640->plat_parms->if_type;
	parms->if_mode = ov5640->plat_parms->if_mode;
	parms->parms.serial = mipi_cfgs[ov5640->i_size];

	return 0;
}
#endif
static struct v4l2_subdev_sensor_ops ov5640_subdev_sensor_ops = {
	.g_skip_frames	= ov5640_g_skip_frames,
//	.g_interface_parms = ov5640_g_interface_parms,
};

static struct v4l2_subdev_ops ov5640_subdev_ops = {
	.core	= &ov5640_subdev_core_ops,
	.video	= &ov5640_subdev_video_ops,
	.sensor	= &ov5640_subdev_sensor_ops,
};

static int ov5640_probe(struct i2c_client *client,
			 const struct i2c_device_id *did)
{
	struct ov5640 *ov5640;
	struct soc_camera_device *icd = client->dev.platform_data;
	struct soc_camera_link *icl;
	int ret;

	if (!icd) {
		dev_err(&client->dev, "OV5640: missing soc-camera data!\n");
		return -EINVAL;
	}

	icl = to_soc_camera_link(icd);
	if (!icl) {
		dev_err(&client->dev, "OV5640 driver needs platform data\n");
		return -EINVAL;
	}

/*	if (!icl->priv) {
		dev_err(&client->dev,
			"OV5640 driver needs i/f platform data\n");
		return -EINVAL;
	}*/

	ov5640 = kzalloc(sizeof(struct ov5640), GFP_KERNEL);
	if (!ov5640)
		return -ENOMEM;

	v4l2_i2c_subdev_init(&ov5640->subdev, client, &ov5640_subdev_ops);

	/* Second stage probe - when a capture adapter is there */
	icd->ops		= &ov5640_ops;

	ov5640->i_size = OV5640_SIZE_VGA;
	ov5640->i_fmt = 0; /* First format in the list */
//	ov5640->plat_parms = icl->priv;

	ret = ov5640_video_probe(icd, client);
	if (ret) {
		icd->ops = NULL;
		kfree(ov5640);
		return ret;
	}

	/* init the sensor here */
	ret = ov5640_init(client);
	if (ret) {
		dev_err(&client->dev, "Failed to initialize sensor\n");
		ret = -EINVAL;
	}

	return ret;
}

static int ov5640_remove(struct i2c_client *client)
{
	struct ov5640 *ov5640 = to_ov5640(client);
	struct soc_camera_device *icd = client->dev.platform_data;

	icd->ops = NULL;
	ov5640_video_remove(icd);
	client->driver = NULL;
	kfree(ov5640);

	return 0;
}

static const struct i2c_device_id ov5640_id[] = {
	{ "ov5640", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, ov5640_id);

static struct i2c_driver ov5640_i2c_driver = {
	.driver = {
		.name = "ov5640",
	},
	.probe		= ov5640_probe,
	.remove		= ov5640_remove,
	.id_table	= ov5640_id,
};

static int __init ov5640_mod_init(void)
{
	return i2c_add_driver(&ov5640_i2c_driver);
}

static void __exit ov5640_mod_exit(void)
{
	i2c_del_driver(&ov5640_i2c_driver);
}

module_init(ov5640_mod_init);
module_exit(ov5640_mod_exit);

MODULE_DESCRIPTION("OmniVision OV5640 Camera driver");
MODULE_AUTHOR("Sergio Aguirre <saaguirre@ti.com>");
MODULE_LICENSE("GPL v2");
