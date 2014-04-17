/*
 * OmniVision OV2675 sensor driver
 *
 * Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/
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

#ifndef __OV2675_H__
#define __OV2675_H__

#define ENABLE_COLOR_PATTERN 0

#define OV2675_SATURATION_MIN			0
#define OV2675_SATURATION_MAX			200
#define OV2675_SATURATION_STEP          100
#define OV2675_SATURATION_DEF			100

#define OV2675_SHARPNESS_MIN			0
#define OV2675_SHARPNESS_MAX			200
#define OV2675_SHARPNESS_STEP           100
#define OV2675_SHARPNESS_DEF			100

/**
 *struct ov2675_reg - ov2675 register format
 *@reg: 16-bit offset to register
 *@val: 8/16/32-bit register value
 *@length: length of the register
 *
 * Define a structure for OV2675 register initialization values
 */
struct ov2675_reg {
	u16 reg;
	u8 val;
};

/* TODO: Divide this properly */
static const struct ov2675_reg configscript_common1[] = {
{0x3012, 0x80}, /*Soft Reset*/
{0xFFFF, 10}, /* Sleep 3ms */
/*IO & Clock & Analog Setup*/
{0x308c, 0x80}, /* timing control*/
{0x308d, 0x0e}, /* timing control*/
{0x360b, 0x00}, /* DVP control*/
{0x30b0, 0xff}, /* D[7:0] set to input*/
{0x30b1, 0xff}, /* Vsync, Strobe, PCLK, HREF, D[9:8] set to input*/
{0x30b2, 0x24}, /* IO control*/
{0x300e, 0x34}, /* PLL*/
{0x300f, 0xa6}, /* PLL*/
{0x3010, 0x81}, /* PLL*/
{0x3082, 0x01},
{0x30f4, 0x01},
{0x3090, 0x33},
{0x3091, 0xc0},
{0x30ac, 0x42},
{0x30d1, 0x08},
{0x30a8, 0x56},
{0x3015, 0x03}, /* VAEC ceiling, 3 frames*/
{0x3093, 0x00},
{0x307e, 0xe5}, /* apply digital gain if gain >=8x*/
{0x3079, 0x00},
{0x30aa, 0x42},
{0x3017, 0x40}, /* disable data drop*/
{0x30f3, 0x82},
{0x306a, 0x0c},
{0x306d, 0x00}, /* BLC control*/
{0x336a, 0x3c}, /* ISP control*/
{0x3076, 0x6a}, /* Vsync drop if frame data is dropped*/
{0x30d9, 0x95},
{0x3016, 0x82}, /* max exposure time = Tframe - (2x2+1)*/
{0x3601, 0x30}, /* DVP output order D[9:0]*/
{0x304e, 0x88},
{0x30f1, 0x82},
{0x3011, 0x02}, /* PLL on, clock divider = 2+1=3*/
/*AEC/AGC*/
{0x3013, 0xf7},/* fast AEC, big step, Banding filter on*/
/*AEC target*/
{0x3018, 0x8c},
{0x3019, 0x7c},
{0x301a, 0xD4},/*0x95*/
/* AGC on, 0xAEC on*/
{0x301c, 0x13}, /* 50Hz max step */
{0x301d, 0x17}, /*  60Hz max step */
{0x3070, 0x3e}, /* B50 */
{0x3072, 0x34}, /* B60*/
/*50/60 auto detection, for 24MHz XVCLK*/
{0x30af, 0x00},
{0x3048, 0x1f},
{0x3049, 0x4e},
{0x304a, 0x20},
{0x304f, 0x20},
{0x304b, 0x02},
{0x304c, 0x00},
{0x304d, 0x02},
{0x304f, 0x20},
{0x30a3, 0x10},
{0x3013, 0xf2}, /* fast AEC, big step, Banding filter on,
		auto banding disable under strong light, less than 1 line off*/
/* AGC on, 0xAEC on*/
{0x3014, 0x84}, /* manual 60Hz, band depend on 50/60 detect,
			night mode off, 50/60 smooth switch,*/
/* less than 1 line exposure auto*/
{0x3071, 0x00}, /* BD50[15:8]*/
{0x3070, 0xbc}, /* BD50[7:0]*/
{0x3073, 0x00}, /* BD60[15:8]*/
{0x3072, 0x9c}, /* BD60[7:0]*/
{0x301c, 0x05}, /* 50Hz max band*/
{0x301d, 0x08}, /* 60Hz max band*/
{0x304d, 0x42},
{0x304a, 0x40},
{0x304f, 0x40},
{0x3095, 0x07},
{0x3096, 0x16},
{0x3097, 0x1d},
/*Window Setup*/
{0x300e, 0x38}, /* PLL*/
{0x3020, 0x01}, /* HS = 280*/
{0x3021, 0x18}, /* HS*/
{0x3022, 0x00}, /* VS = 10*/
{0x3023, 0x0a}, /* VS*/
{0x3024, 0x06}, /* HW = 1624*/
{0x3025, 0x58}, /* HW*/
{0x3026, 0x04}, /* VH = 1212*/
{0x3027, 0xbc}, /* VH*/

{0x3088, 0x06}, /* ISP_XOUT = 1600*/
{0x3089, 0x40}, /* ISP_XOUT*/
{0x308a, 0x04}, /* ISP_YOUT = 1200*/
{0x308b, 0xb0}, /* ISP_YOUT*/
{0x3316, 0x64}, /* Scale H input size = 1600*/
{0x3317, 0x4b}, /* Scale V input size = 1200*/
{0x3318, 0x00}, /* Scale H/V input size*/
{0x331a, 0x64}, /* Scale H output size = 1600*/
{0x331b, 0x4b}, /* Scale V output size = 1200*/
{0x331c, 0x00}, /* Scale H/V output size*/
{0x3100, 0x00},
/*AWB*/
{0x3320, 0xf8},/* Advanced AWB, one zone on*/
{0x3321, 0x11},
{0x3322, 0x92},
{0x3323, 0x01},
{0x3324, 0x97},
{0x3325, 0x02},
{0x3326, 0xff},
{0x3327, 0x10}, /* S*/
{0x3328, 0x10}, /* EC*/
{0x3329, 0x12}, /* FC*/
{0x332a, 0x5c}, /* X0*/
{0x332b, 0x5e}, /* Y0*/
{0x332c, 0x94}, /* Kx*/
{0x332d, 0x92}, /* Ky*/
{0x332e, 0x3a}, /* Day limit*/
{0x332f, 0x31}, /* A limit*/
{0x3330, 0x4d}, /* Day split*/
{0x3331, 0x42}, /* A split*/
{0x3332, 0xff}, /* AWB top limit*/
{0x3333, 0x0}, /* AWB bot limit*/
{0x3334, 0xf0}, /* Red limit*/
{0x3335, 0xf0}, /* Green limit*/
{0x3336, 0xf0}, /* Blue limit*/
{0x3337, 0x40},
{0x3338, 0x40},
{0x3339, 0x40},
{0x333a, 0x0},
{0x333b, 0x0},
/*Color Matrix*/
{0x3380, 0x26}, /* CMX1*/
{0x3381, 0x59}, /* CMX2*/
{0x3382, 0x01}, /* CMX3*/
{0x3383, 0x22}, /* CMX4*/
{0x3384, 0xB2}, /* CMX5*/
{0x3385, 0xD4}, /* CMX6*/
{0x3386, 0xCF}, /* CMX7*/
{0x3387, 0xCD}, /* CMX8*/
{0x3388, 0x02}, /* CMX9*/
{0x3389, 0x98}, /* sign[8:1]*/
{0x338A, 0x01}, /* sign[9]*/
/*Gamma*/
{0x3340, 0x08}, /* YST1*/
{0x3341, 0x10}, /* YST2*/
{0x3342, 0x20}, /* YST3*/
{0x3343, 0x40}, /* YST4*/
{0x3344, 0x4F}, /* YST5*/
{0x3345, 0x5E}, /* YST6*/
{0x3346, 0x6B}, /* YST7*/
{0x3347, 0x77}, /* YST8*/
{0x3348, 0x82}, /* YST9*/
{0x3349, 0x8B}, /* YST10*/
{0x334A, 0x9C}, /* YST11*/
{0x334B, 0xAB}, /* YST12*/
{0x334C, 0xC3},
{0x334D, 0xD7},
{0x334E, 0xE9},
{0x334F, 0x1E},
/*Lens correction*/
{0x3350, 0x31}, /* Red X center*/
{0x3351, 0x26}, /* Red Y center*/
{0x3352, 0x08}, /* Red X/Y center*/
{0x3353, 0x21},/* Red a1*/
{0x3354, 0x00}, /* Red b1*/
{0x3355, 0x85}, /* Red b2, a2*/
{0x3356, 0x32}, /* Green X center*/
{0x3357, 0x25}, /* Green Y center*/
{0x3358, 0x40}, /* Green X/Y center*/
{0x3359, 0x1c}, /* Green a1*/
{0x335a, 0x00}, /* Green b1*/
{0x335b, 0x85}, /* Green b2, a2*/
{0x335c, 0x31}, /* Blue X center*/
{0x335d, 0x25}, /* Blue Y center*/
{0x335e, 0x88}, /* Blue X/Y center*/
{0x335f, 0x1a}, /* Blue a1*/
{0x3360, 0x00}, /* Blue b1*/
{0x3361, 0x85}, /* Blue b2, a2*/
{0x3363, 0x70}, /* Lenc T1*/
{0x3364, 0x7f}, /* Lenc T2*/
{0x3365, 0x00}, /* Lenc T1/T2*/
{0x3366, 0x00},
/*UVadjust*/
{0x3301, 0xff}, /* SDE on, UV Adjust on, color matrix on,
			sharpen on, DNS on, CIP on, BPC on, WPC on*/
{0x338B, 0x11}, /* ADJ_offset*/
{0x338c, 0x10}, /* ADJ_Th1*/
{0x338d, 0x40}, /* ADJ_Th2*/
/*Sharpness/De-noise*/
{0x3370, 0xd0},
{0x3371, 0x00},
{0x3372, 0x00}, /* Edge threshold manual*/
{0x3373, 0x40}, /* DNS offset*/
{0x3374, 0x10}, /* DNS threshold*/
{0x3375, 0x10}, /* DNS slop*/
{0x3376, 0x04}, /* Sharpness offset 1*/
{0x3377, 0x00}, /* Sharpness offset 2*/
{0x3378, 0x04}, /* Sharpness th1*/
{0x3379, 0x80}, /* Sharpness th2*/
/*BLC*/
{0x3069, 0x84}, /* BLC target*/
{0x307c, 0x10}, /* mirror off, flip off*/
{0x3087, 0x02}, /* BLC always on*/
/*Other functions*/
{0x3300, 0xfc}, /* ISP on, gamma on, AWB stat on, AWB gain on,
			Lenc on, Lenc low light on, ISP format = YUV*/
{0x3302, 0x01}, /* Scale off, UV average on*/
{0x3400, 0x02}, /* YUV 422*/
{0x3606, 0x20}, /* DVP on*/
{0x3601, 0x30}, /* DVP Data order D[9:0]*/
{0x300e, 0x34}, /* PLL*/
{0x30f3, 0x83},
{0x304e, 0x88},
/*MIPI*/
{0x363b, 0x01},
{0x309e, 0x08},
{0x3606, 0x00}, /* DVP off*/
{0x3630, 0x31}, /*Disable short package output*/
{0x3086, 0x0f}, /* soft sleep on*/
{0x3086, 0x00}, /* soft sleep off*/
{0x304e, 0x04}, /* [7] DVP_CLK_snr*/
{0x363b, 0x01}, /* disable cd*/
{0x309e, 0x08}, /* disable lp_rx*/
{0x3606, 0x00}, /* disable dvp*/
{0x3084, 0x01}, /* scale_div_man_en*/
{0x3010, 0x81}, /* scale_div_man*/
{0x3011, 0x01}, /* sys_div /4*/
{0x3634, 0x26},
{0x3086, 0x0f}, /*sleep on*/
{0x3086, 0x00}, /*sleep off*/

	{0xFFFF, 0x00}
};

static const struct ov2675_reg yuv422_init_common[] = {
	{0x3013, 0xf2},
	{0x3002, 0x02},
	{0x3003, 0x5b},

	{0x3012, 0x10}, /* SVGA Average mode*/
	{0x302a, 0x02}, /* VTS = 618*/
	{0x302b, 0x9E}, /* VTS*/
	{0x306f, 0x14}, /* BLC target for short exposure of HDR mode*/
	{0x3020, 0x01}, /* HS = 280*/
	{0x3021, 0x18}, /* HS*/
	{0x3022, 0x00}, /* VS = 6*/
	{0x3023, 0x06}, /* VS*/
	{0x3024, 0x06}, /* HW = 1624*/
	{0x3025, 0x58}, /* HW*/
	{0x3026, 0x02}, /* VH = 609*/
	{0x3027, 0x61}, /* VH*/
	{0x3088, 0x02}, /* ISP XOUT = 640*/
	{0x3089, 0x80}, /* ISP XOUT*/
	{0x308a, 0x01}, /* ISP YOUT = 480*/
	{0x308b, 0xe0}, /* ISP YOUT*/
	{0x3313, 0x00},
	{0x3314, 0x00},
	{0x3315, 0x00},
	{0x3316, 0x64}, /* Scale X input = 1600*/
	{0x3317, 0x25}, /* Scale Y input = 600*/
	{0x3318, 0x80}, /* Scale Y/X input*/
	{0x3319, 0x08}, /* Scale X offset = 8*/
	{0x331a, 0x64}, /* Scale X output = 1600*/
	{0x331b, 0x4b}, /* Scale Y output = 1200*/
	{0x331c, 0x00}, /* Scale Y/X output*/
	{0x331d, 0x38}, /* Scale Y offset = 3, Scale X offset = 8*/
	{0x3302, 0x11}, /* Scale on, UV Average on*/
	{0x3088, 0x02}, /* ISP X output = 640*/
	{0x3089, 0x80}, /* ISP X output*/
	{0x308a, 0x01}, /* ISP Y output = 480*/
	{0x308b, 0xe0}, /* ISP Y output*/
	/*{0x3640, 0x1e},*/
	{0x331a, 0x28},

	{0x331b, 0x1e},

	{0x331c, 0x00},
	{0x300e, 0x34},/* PLL*/
	{0x300f, 0xa6},/* PLL*/
	{0x3010, 0x81},/* PLL*/
	{0x3011, 0x00},/* clock divider*/
	{0x302d, 0x00},/* EXVTS = 0*/
	{0x302e, 0x00},/* EXVTS*/
	{0x302c, 0x00},/* EXHTS = 0*/
	{0x3070, 0xC9},/* B50*/
	{0x3072, 0xA7},/* B60*/
	{0x301c, 0x02},/* max step 50*/
	{0x301d, 0x03},/* max step 60*/

{0xFFFF, 0x00}
};

static const struct ov2675_reg capture_init_common[] = {
	{0x3013, 0xf2},
	{0x3002, 0x02},
	{0x3003, 0x5b},
	{0x3012, 0x00},
	{0x302a, 0x04},
	{0x302b, 0xd4},
	{0x306f, 0x54},
	{0x3070, 0x5d},
	{0x3072, 0x4d},
	{0x3020, 0x01},
	{0x3021, 0x18},
	{0x3022, 0x00},
	{0x3023, 0x0a},
	{0x3024, 0x06},
	{0x3025, 0x58},
	{0x3026, 0x04},
	{0x3027, 0xbc},
	{0x3088, 0x06},
	{0x3089, 0x40},
	{0x308a, 0x04},
	{0x308b, 0xb0},
	/*{0x3640, 0x1e},*/
	{0x3313, 0x00},
	{0x3314, 0x00},
	{0x3315, 0x00},
	{0x3316, 0x64},
	{0x3317, 0x4b},
	{0x3318, 0x00},
	{0x3319, 0x2c},
	{0x331a, 0x64},
	{0x331b, 0x4b},
	{0x331c, 0x00},
	{0x331d, 0x4c},
	{0x3302, 0x01},
	{0x300e, 0x34},
	{0x300f, 0xa6},
	{0x3010, 0x81},
	{0x3011, 0x01},
	{0x302d, 0x00},
	{0x302e, 0x00},
	{0x302c, 0x00},
	{0x3070, 0x5c},
	{0x3072, 0x4d},
	{0x301c, 0x0c},
	{0x301d, 0x0f},

	{0xFFFF, 0x00}
};

static const struct ov2675_reg ov2675_streamon[] = {
/*	{0x3086, 0x0f},
	{0x3086, 0x00},*/
	{0xFFFF, 0x00}
};

static const struct ov2675_reg ov2675_streamoff[] = {
/*	{0x3086, 0x0f},*/
	{0xFFFF, 0x00}
};

static const struct ov2675_reg ov2675_power_down[] = {
	/* System Control */

	{0xFFFF, 0x00}
};

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/*EFFECT */
static const struct ov2675_reg ov2675_effect_normal_tbl[] = {

	{0xFFFF, 0x00}
};

static const struct ov2675_reg ov2675_effect_bw_tbl[] = {

	{0xFFFF, 0x00}
};

static const struct ov2675_reg ov2675_effect_sepia_tbl[] = {

	{0xFFFF, 0x00}
};

static const struct ov2675_reg ov2675_effect_negative_tbl[] = {

	{0xFFFF, 0x00}
};

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/*AntiBanding */
static const struct ov2675_reg ov2675_antibanding_auto_tbl[] = {
	{0x3014, 0x84},
	{0xFFFF, 0x00}
};

static const struct ov2675_reg ov2675_antibanding_50z_tbl[] = {
	{0x3014, 0x84},
	{0xFFFF, 0x00}
};

static const struct ov2675_reg ov2675_antibanding_60z_tbl[] = {
	{0x3014, 0x04},
	{0xFFFF, 0x00}
};

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*Lens_shading*/

static const struct ov2675_reg ov2675_lens_shading_on_tbl[] = {

	{0xFFFF, 0x00}
};


static const struct ov2675_reg ov2675_lens_shading_off_tbl[] = {
	/*@@ Lenc Off */


	{0xFFFF, 0x00}
};

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/*Contrast */

static const struct ov2675_reg ov2675_contrast_default_lv3_tbl[] = {

	{0xFFFF, 0x00}
};

static const struct ov2675_reg ov2675_contrast_lv0_tbl[] = {

	{0xFFFF, 0x00}
};

static const struct ov2675_reg ov2675_contrast_lv1_tbl[] = {

	{0xFFFF, 0x00}
};

static const struct ov2675_reg ov2675_contrast_lv2_tbl[] = {

	{0xFFFF, 0x00}
};

static const struct ov2675_reg ov2675_contrast_lv4_tbl[] = {

	{0xFFFF, 0x00}
};

static const struct ov2675_reg ov2675_contrast_lv5_tbl[] = {

	{0xFFFF, 0x00}
};

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*sharpness */

static const struct ov2675_reg ov2675_sharpness_default_lv2_tbl[] = {

	{0xFFFF, 0x00}
};

static const struct ov2675_reg ov2675_sharpness_lv0_tbl[] = {

	{0xFFFF, 0x00}
};

static const struct ov2675_reg ov2675_sharpness_lv1_tbl[] = {


	{0xFFFF, 0x00}
};

static const struct ov2675_reg ov2675_sharpness_lv3_tbl[] = {

	{0xFFFF, 0x00}
};

static const struct ov2675_reg ov2675_sharpness_lv4_tbl[] = {

	{0xFFFF, 0x00}
};

static const struct ov2675_reg ov2675_sharpness_lv5_tbl[] = {

	{0xFFFF, 0x00}
};

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* saturation */

static const struct ov2675_reg ov2675_saturation_default_lv3_tbl[] = {

	{0xFFFF, 0x00}
};

static const struct ov2675_reg ov2675_saturation_lv0_tbl[] = {

	{0xFFFF, 0x00}
};

static const struct ov2675_reg ov2675_saturation_lv1_tbl[] = {

	{0xFFFF, 0x00}
};

static const struct ov2675_reg ov2675_saturation_lv2_tbl[] = {

	{0xFFFF, 0x00}
};

static const struct ov2675_reg ov2675_saturation_lv4_tbl[] = {

	{0xFFFF, 0x00}
};

static const struct ov2675_reg ov2675_saturation_lv5_tbl[] = {

	{0xFFFF, 0x00}
};

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* brightness */

static const struct ov2675_reg ov2675_brightness_lv0_tbl[] = {

	{0xFFFF, 0x00}
};

static const struct ov2675_reg ov2675_brightness_lv1_tbl[] = {

	{0xFFFF, 0x00}
};

static const struct ov2675_reg ov2675_brightness_lv2_default_tbl[] = {

	{0xFFFF, 0x00}
};

static const struct ov2675_reg ov2675_brightness_lv3_tbl[] = {


	{0xFFFF, 0x00}
};

static const struct ov2675_reg ov2675_brightness_lv4_tbl[] = {

	{0xFFFF, 0x00}
};

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* Auto Expourse */

static const struct ov2675_reg ov2675_ae_average_tbl[] = {

	{0xFFFF, 0x00}
};

static const struct ov2675_reg ov2675_ae_centerweight_tbl[] = {

	{0xFFFF, 0x00}
};

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*default awb */
static const struct ov2675_reg ov2675_wb_def[] = {

	{0xFFFF, 0x00}
};

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* White Balance */
static const struct ov2675_reg ov2675_wb_fluorescent[] = {

	{0xFFFF, 0x00}
};

static const struct ov2675_reg ov2675_wb_daylight[] = {


	{0xFFFF, 0x00}
};

static const struct ov2675_reg ov2675_wb_cloudy[] = {


	{0xFFFF, 0x00}
};

static const struct ov2675_reg ov2675_wb_tungsten[] = {


	{0xFFFF, 0x00}
};

static const struct ov2675_reg ov2675_fps_5[] = {

	{0xFFFF, 0x00}
};

static const struct ov2675_reg ov2675_fps_7[] = {

	{0xFFFF, 0x00}
};
static const struct ov2675_reg ov2675_fps_10[] = {

	{0xFFFF, 0x00}
};
static const struct ov2675_reg ov2675_fps_15[] = {

	{0xFFFF, 0x00}
};
static const struct ov2675_reg ov2675_fps_20[] = {

	{0xFFFF, 0x00}
};
static const struct ov2675_reg ov2675_fps_25[] = {

	{0xFFFF, 0x00}
};

static const struct ov2675_reg ov2675_fps_30[] = {


	{0xFFFF, 0x00}
};


#endif /* __OV2675_H__ */
