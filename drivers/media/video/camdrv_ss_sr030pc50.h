/* drivers/media/video/sr030pc50_regs_1_1.h
 *
 * Driver for sr030pc50 (VGA Camera) from Siliconfile
 *
 * Copyright (C) 2010, SAMSUNG ELECTRONICS
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef __CAMDRV_SS_SR030PC50_H__
#define __CAMDRV_SS_SR030PC50_H__


static regs_t sr030pc50_init_regs[] = {
0x0300,
0x0171,	/* reset op. */
0x0173,
0x0171,

0x0320,
0x101c,
0x0322,
0x107b,

/* PLL Setting */
0x0300,

0x080f,	/* Parallel NO Output_PAD Out */
0x1000,	/* VDOCTL1 */
0x1180,	/* VDOCTL2 , 90 : FFR off, 94 : FFR on */
0x1200,	/* CLK_CTL */
0x1488,

0xc095,
0xc118,
0xc291,
0xc300,
0xc401,

0x0300,
0x1200,
0x2000,
0x2104,
0x2200,
0x2304,

0x4001, /* Hblank 344 */
0x4158,
0x4201, /* Vblank 410 */
0x439a,

/* BLC */
0x802e, /* don't touch */
0x817e, /* don't touch */
0x8290, /* don't touch */
0x8330, /* don't touch */
0x842c, /* don't touch */
0x854b, /* don't touch */
0x8601, /* don't touch */
0x8847, /* don't touch */
/* 0x8948,*/ /* BLC hold */

0x900a, /* BLC_TIME_TH_ON */
0x910a, /* BLC_TIME_TH_OFF */
0x9278, /* BLC_AG_TH_ON */
0x9370, /* BLC_AG_TH_OFF */

0x9838, /* don't touch */
0x9940, /* Out BLC */
0xa040, /* Dark BLC */
0xa842, /* Normal BLC 44 */

/* Page2 Last Update 12_01_20 */
0x0302,
0x1000,
0x1100,
0x1340,
0x1404,
0x181c,
0x1900, /* 01 */
0x1a00,
0x1b08,
0x1c9c,
0x1d03,
0x2033,
0x2177,
0x22a7,
0x2332,
0x2433,
0x2b40,
0x2d32,
0x3199,
0x3200,
0x3300,
0x343c,
0x350d,
0x3b60, /* 80 */

/* timing control 1 */ /* don't touch*/
0x5021,
0x511c,
0x52aa,
0x535a,
0x5430,
0x5510,
0x560c,
0x5800,
0x590f,

/* timing control 2 */ /* don't touch */
0x6034,
0x613a,
0x6234,
0x6339,
0x6434,
0x6539,
0x7235,
0x7338,
0x7435,
0x7538,
0x8002,
0x812e,
0x820d,
0x8310,
0x840d,
0x8510,
0x921d,
0x9320,
0x941d,
0x9520,
0xa003,
0xa12d,
0xa42d,
0xa503,
0xa812,
0xa91b,
0xaa22,
0xab2b,
0xac10,
0xad0e,
0xb833,
0xb935,
0xbc0c,
0xbd0e,
0xc03a,
0xc13f,
0xc23a,
0xc33f,
0xc43a,
0xc53e,
0xc63a,
0xc73e,
0xc83a,
0xc93e,
0xca3a,
0xcb3e,
0xcc3b,
0xcd3d,
0xce3b,
0xcf3d,
0xd033,
0xd13f,

/* Page 10 */
0x0310,
0x1001, /* 03, */ /* ISPCTL1, YUV ORDER(FIX) */
0x1143,
0x1230, /* Y offet, dy offseet enable */
0x4080,
0x4102, /* 00 DYOFS  00->10  _100318 */
0x4880, /* Contrast  88->84  _100318 */
0x5048, /* AGBRT */

0x6001, /* 7f */ /* 7c */
0x6100, /* Use default */
0x627c, /* SATB  (1.4x) */
0x637c, /* SATR  (1.2x) */
0x6448, /* AGSAT */
0x6690, /* wht_th2 */
0x6736, /* wht_gain  Dark (0.4x), Normal (0.75x) */

/* Page 11 */
/* LPF */
0x0311,
0x1025,	/* LPF_CTL1 */ /* 0x01 */
0x1107,	/* 06 */
0x2000,	/* LPF_AUTO_CTL */
0x2160,	/* LPF_PGA_TH */
0x230a,	/* LPF_TIME_TH */
0x6013,	/* ZARA_SIGMA_TH */ /* 40->10 */
0x6185,
0x6200,	/* ZARA_HLVL_CTL */
0x6300,	/* ZARA_LLVL_CTL */
0x6400,	/* ZARA_DY_CTL */

0x6770,	/* f0 */     /* Dark */
0x6804,	/* Change 100402 */     /* Middle */
0x6904,	/* High */

/* Page 12 */
/* 2D */
0x0312,
0x40cb,	/* YC2D_LPF_CTL1 */
0x4109,	/* YC2D_LPF_CTL2 */
0x5018,	/* Test Setting */
0x5124,	/* Test Setting */
0x701f,	/* GBGR_CTL1 */ /* 1f */
0x7100,	/* Test Setting */
0x7200,	/* Test Setting */
0x7300,	/* Test Setting */
0x7412,	/* GBGR_G_UNIT_TH//12 */
0x7512,	/* GBGR_RB_UNIT_TH//12 */
0x7620,	/* GBGR_EDGE_TH */
0x7780,	/* GBGR_HLVL_TH */
0x7888,	/* GBGR_HLVL_COMP */
0x7918,	/* Test Setting */

/******************************/
0x903d,
0x9134,
0x9928,
0x9c05, /* 14 For defect */
0x9d08, /* 15 For defect */
0x9e28,
0x9f28,

0xb07d, /* 75 White Defect */
0xb544,
0xb682,
0xb752,
0xb844,
0xb915,
/******************************/

/* Edge */
0x0313,
0x1001,
0x1189,
0x1214,
0x1319,
0x1408,	/* Test Setting */
0x2003,	/* SHARP_Negative */
0x2104,	/* SHARP_Positive */
0x2325,	/* SHARP_DY_CTL  */
0x2421,	/* 40->33 */
0x2508,	/* SHARP_PGA_TH */
0x2640,	/* Test Setting */
0x2700,	/* Test Setting */
0x2808,	/* Test Setting */
0x2950,	/* AG_TH */
0x2ae0,	/* region ratio */
0x2b10,	/* Test Setting */
0x2c28,	/* Test Setting */
0x2d40,	/* Test Setting */
0x2e00,	/* Test Setting */
0x2f00,	/* Test Setting */
0x3011,	/* Test Setting */
0x8005,	/* SHARP2D_CTL */
0x8107,	/* Test Setting */
0x9004,	/* SHARP2D_SLOPE */
0x9105,	/* SHARP2D_DIFF_CTL */
0x9200,	/* SHARP2D_HI_CLIP */
0x9330,	/* SHARP2D_DY_CTL */
0x9430,	/* Test Setting */
0x9510,	/* Test Setting */

0x0314,
0x1001,

0x2242,
0x2332,
0x2430,

0x2794, /* RXCEN */
0x2880, /* RYCEN */
0x2994, /* GXCEN */
0x2a80, /* GYCEN */
0x2b94, /* BXCEN */
0x2c80, /* BYCEN */

/* 15page////////////////////////// */
0x0315,
0x1003,

0x144a,	/* CMCOFSGM  */
0x163e,	/* CMCOFSGL */
0x172f,	/* CMC SIGN */

/* CMC */
0x30cb,
0x3161,
0x3216,
0x331e,
0x34ce,
0x3530,
0x3600,
0x3730,
0x3870,

/* CMC OFS */
0x401c,
0x4195,
0x4285,
0x4394,
0x4498,
0x452c,
0x4690,
0x4708,
0x4808,

0x0316,/* amma   0.44 */ /* S_32 */
0x3000,
0x310e,/* 0x0d */
0x321a,/* 0x18 */
0x332f,/* 0x2c */
0x3453,
0x3576,
0x3693,
0x37ac,
0x38c0,
0x39d0,
0x3adc,
0x3bed,
0x3cf4, /* f7 */
0x3df6, /* fc */
0x3efa, /* ff */

/* Page 17 AE  */
0x0317,
0xc43c,
0xc532,

/* Page 20 AE  */
0x0320,
0x101c,
0x1104,

0x2001,
0x2827,
0x29a1,

0x2af0,
0x2b34, /* f4 */
0x2c2b, /* 23->2b 2010_04_06 hhzin */

0x3078, /* f8 */

0x3b22,
0x3cde,

0x3922,
0x3ade,
0x3b22, /* 23->22 _10_04_06 hhzin */
0x3cde,

0x6070, /* 70 */
0x6120, /* 11 */ /* 22 */

0x6270,
0x6320, /* 11 */ /* 22 */

0x683c,
0x6964,
0x6A28,
0x6Bc8,

0x7034,/* Y Targe 32 */

0x7611, /* Unlock bnd1 */
0x7772, /* Unlock bnd2 02->a2 _10_04_06 hhzin */

0x7812, /* Yth 1 */
0x791f, /* Yth 2 26->27 _10_04_06 hhzin */
0x7a23, /* Yth 3 */

0x7c17, /* 1c->1d _10_04_06 hhzin */
0x7d22,

/* 50Hz */
0x8300, /* EXP Normal 33.33 fps  */
0x84be,
0x856e,

0x8600,/* ExpMin */
0x87fa,

/* 50Hz_10fps */
0x8802, /* EXP Max 10.00 fps  */
0x897a,
0x8ac4,

0x8b3f,/* Exp100 */
0x8c7a,

0x8d34,/* Exp120 */
0x8ebc,

0x9100,	/*  fix30 02 */
0x92c3,	/*  dc */
0x93b4,	/*  6c */

0x9401, /* fix_step */
0x95b7,
0x9674,

0x988C,
0x9923,

0x9c06, /* EXP Limit 928.57 fps  */
0x9dd6,
0x9e00, /* EXP Unit  */
0x9ffa,

0xb01d,
0xb114,
0xb290, /* 80 */
0xb310,
0xb417,
0xb53e,
0xb62b,
0xb724,
0xb821,
0xb91f,
0xba1e,
0xbb1d,
0xbc1c,
0xbd1b,

0xc01a,
0xc348,
0xc448,

/* Page 22 AWB */
0x0322,
0x10e2,
0x112e, /* 2e */
0x2140,

0x3080,
0x3180, /* 7f */
0x3812,
0x3933,
0x40f3, /* f0 */
0x4132,
0x4222,
0x43f3,
0x4455,
0x4544,
0x4602,

0x803a,/* 3f */
0x8120,
0x823a,/* 3f */

0x835a, /* RMAX 5a */
0x841e, /* RMIN 20 */
0x8554, /* BMAX 53 */
0x8620, /* BMIN  */

0x874a, /* 42 */
0x883e,
0x8930, /* 36 */ /* 2d */
0x8a24, /* 2e */ /* 24 */

0x8b00, /* OUT TH */
0x8d22, /* 21 */
0x8e61, /* 41 */

0x8f63,
0x9062, /* 60 */ /* 62 */
0x915e, /* 5c */ /* 5e */
0x9256,
0x9350, /* 52  */ /* 50 */
0x9444, /* 48 */ /* 46 */
0x9537, /* 36 */ /* 3c */
0x9631, /* 31 */ /* 31 */
0x972c,
0x982a,
0x9928, /* 29 */
0x9a26,
0x9b09,


/* /////////////////////////// Page 48	- MIPI		//// */
0x0348,

0x1005,
0x1100, /* async_fifo off */
0x1200,

0x16c4,
0x1700,
0x1900,
0x1a00,
0x1c02,
0x1d04,
0x1e07,
0x1f06,
0x2000,
0x21b8,
0x2200,
0x2301,

0x3005,
0x3100,
0x3402,
0x3206,
0x3503,
0x3601,
0x3703,
0x3800,
0x394a,
0x3c00,
0x3dfa,
0x3f10,
0x4000,
0x4120,
0x4200,

/*   0x5081, */

0x0322,
0x10fb,

0x0320,
0x109c,

0x0170,
};



static const u16 sr030pc50_init_reg2[] = {
};

static const u16 sr030pc50_DTP_init[] = {
};

static const u16 sr030pc50_DTP_stop[] = {
};

/* ========================================================== */
/* FPS() */
/* ========================================================== */

static const u16 sr030pc50_FPS_Auto[] = {
};


static const u16 sr030pc50_FPS_5[] = {
};

static const u16 sr030pc50_FPS_7[] = {
};

static const u16 sr030pc50_FPS_10[] = {
};

static const u16 sr030pc50_FPS_15[] = {
};

static const u16 sr030pc50_FPS_20[] = {
};

static const u16 sr030pc50_FPS_30[] = {
};


static const u16 sr030pc50_FPS_60[] = {

};

static const u16 sr030pc50_FPS_120[] = {

};


/* ========================================================== */
/*   EFFECT(6) */
/* ========================================================== */

static const u16 sr030pc50_Effect_Normal[] = {
};

static const u16 sr030pc50_Effect_Black_White[] = {
};

static const u16 sr030pc50_Effect_Solarization[] = {
};

static const u16 sr030pc50_Effect_Negative[] = {
};

static const u16 sr030pc50_Effect_Sepia[] = {
};





/* ========================================================== */
/*  MWB(5) */
/* ========================================================== */


static const u16 sr030pc50_WB_Auto[] = {
};

static const u16 sr030pc50_WB_Sunny[] = {
};

static const u16 sr030pc50_WB_Cloudy[] = {
};

static const u16 sr030pc50_WB_Fluorescent[] = {
};

static const u16 sr030pc50_WB_Tungsten[] = {
};


static const u16 sr030pc50_WB_CWF[] = { /*  DO NOT USE IT SEHF_CHECK_IT */
};


/* ========================================================== */
/* ISO() */
/* ========================================================== */

static const u16 sr030pc50_ISO_Auto[] = {
};

static const u16 sr030pc50_ISO_Sport[] = {
};

static const u16 sr030pc50_ISO_Night[] = {
};

static const u16 sr030pc50_ISO_50[] = {
};

static const u16 sr030pc50_ISO_100[] = {
};

static const u16 sr030pc50_ISO_200[] = {
};

static const u16 sr030pc50_ISO_400[] = {
};



static const u16 sr030pc50_WDR_on[] = {
};

static const u16 sr030pc50_WDR_off[] = {
};








/* ========================================================== */
/* METERING() */
/* ========================================================== */

static const u16 sr030pc50_Metering_Matrix[] = {
};

static const u16 sr030pc50_Metering_Center[] = {
};

static const u16 sr030pc50_Metering_Spot[] = {
};









/* ========================================================== */
/*  BRIGHTNES(9) */
/* ========================================================== */

static const u16 sr030pc50_EV_Minus_4[] = {
	0xffff,
};

static const u16 sr030pc50_EV_Minus_3[] = {
};

static const u16 sr030pc50_EV_Minus_2[] = {
};

static const u16 sr030pc50_EV_Minus_1[] = {
};

static const u16 sr030pc50_EV_Default[] = {
};

static const u16 sr030pc50_EV_Plus_1[] = {
};

static const u16 sr030pc50_EV_Plus_2[] = {
};

static const u16 sr030pc50_EV_Plus_3[] = {
};

static const u16 sr030pc50_EV_Plus_4[] = {
};




/* ========================================================== */
/*  BRIGHTNES(9) Camcorder */
/* ========================================================== */

static const u16 sr030pc50_EV_Camcorder_Minus_4[] = {
};

static const u16 sr030pc50_EV_Camcorder_Minus_3[] = {
};

static const u16 sr030pc50_EV_Camcorder_Minus_2[] = {
};

static const u16 sr030pc50_EV_Camcorder_Minus_1[] = {
};

static const u16 sr030pc50_EV_Camcorder_Default[] = {
};

static const u16 sr030pc50_EV_Camcorder_Plus_1[] = {
};

static const u16 sr030pc50_EV_Camcorder_Plus_2[] = {
};

static const u16 sr030pc50_EV_Camcorder_Plus_3[] = {
};

static const u16 sr030pc50_EV_Camcorder_Plus_4[] = {
};





/* ========================================================== */
/*  contrast(5) - // DO NOT USE IT SEHF_CHECK_IT */
/* ========================================================== */

static const u16 sr030pc50_Contrast_Minus_4[] = {
};

static const u16 sr030pc50_Contrast_Minus_3[] = {
};

static const u16 sr030pc50_Contrast_Minus_2[] = {
};

static const u16 sr030pc50_Contrast_Minus_1[] = {
};

static const u16 sr030pc50_Contrast_Default[] = {
};

static const u16 sr030pc50_Contrast_Plus_1[] = {
};

static const u16 sr030pc50_Contrast_Plus_2[] = {
};



/* ========================================================== */
/*  sharpness(5) - // DO NOT USE IT SEHF_CHECK_IT */
/* ========================================================== */

static const u16 sr030pc50_Sharpness_Minus_2[] = {
};

static const u16 sr030pc50_Sharpness_Minus_1[] = {
};

static const u16 sr030pc50_Sharpness_Default[] = {
};

static const u16 sr030pc50_Sharpness_Plus_1[] = {
};

static const u16 sr030pc50_Sharpness_Plus_2[] = {
};







/* ========================================================== */
/*  saturation(5) - // DO NOT USE IT SEHF_CHECK_IT */
/* ========================================================== */

static const u16 sr030pc50_Saturation_Minus_2[] = {
};

static const u16 sr030pc50_Saturation_Minus_1[] = {
};

static const u16 sr030pc50_Saturation_Default[] = {
};

static const u16 sr030pc50_Saturation_Plus_1[] = {
};

static const u16 sr030pc50_Saturation_Plus_2[] = {
};






/* ========================================================== */
/*  WDR Auto Contrast */
/* ========================================================== */

static const u16 sr030pc50_Auto_Contrast_ON[] = {
};

static const u16 sr030pc50_Auto_Contrast_OFF[] = {
};









/* ========================================================== */
/*  JPEG QUALITY - // DO NOT USE IT SEHF_CHECK_IT */
/* ========================================================== */

static const u16 sr030pc50_Jpeg_Quality_High[] = {
};

static const u16 sr030pc50_Jpeg_Quality_Normal[] = {
};

static const u16 sr030pc50_Jpeg_Quality_Low[] = {
};

/* ========================================================== */
/* SCENE() */
/* ========================================================== */

static const u16 sr030pc50_Scene_Default[] = {
};

/*========================================================== */
/*	CAMERA_SCENE_PORTRAIT (Auto/Center/Br0/Auto/Sharp-1/Sat0) */
/*========================================================== */
static const u16 sr030pc50_Scene_Portrait[] = {
};

/*========================================================== */
/*	CAMERA_SCENE_NIGHT (Night/Center/Br0/Auto/Sharp0/Sat0) */
/*========================================================== */
static const u16 sr030pc50_Scene_Nightshot[] = {
};

/*========================================================== */
/*	CAMERA_SCENE_AGAINSTLIGHT (Auto/Spot(flash)
		or Center(no flash)/Br0/Auto/Sharp0/Sat0) */
/*========================================================== */
static const u16 sr030pc50_Scene_Backlight[] = {
};

/*========================================================== */
/*	CAMERA_SCENE_LANDSCAPE (Auto/Matrix/Br0/Auto/Sharp+1/Sat+1) */
/*========================================================== */
static const u16 sr030pc50_Scene_Landscape[] = {
};

/*========================================================== */
/*	CAMERA_SCENE_SPORTS (Sport/Center/Br0/Auto/Sharp0/Sat0) */
/*========================================================== */
static const u16 sr030pc50_Scene_Sports[] = {
};

/*========================================================== */
/*	CAMERA_SCENE_PARTYINDOOR (ISO200/Center/Br0/Auto/Sharp0/Sat+1) */
/*========================================================== */
static const u16 sr030pc50_Scene_Party_Indoor[] = {
};

/*========================================================== */
/*	CAMERA_SCENE_BEACHSNOW (ISO50/Center/Br+1/Auto/Sharp0/Sat+1) */
/*========================================================== */
static const u16 sr030pc50_Scene_Beach_Snow[] = {
};

/*========================================================== */
/*	CAMERA_SCENE_SUNSET (Auto/Center/Br0/Daylight/Sharp0/Sat0) */
/*========================================================== */
static const u16 sr030pc50_Scene_Sunset[] = {
};

/*========================================================== */
/*	CAMERA_SCENE_DAWN (Auto/Center/Br0/CWF/Sharp0/Sat0) */
/*========================================================== */
static const u16 sr030pc50_Scene_Duskdawn[] = {
};

/*========================================================== */
/*	CAMERA_SCENE_FALLCOLOR (Auto/Center/Br0/Auto/Sharp0/Sat+2) */
/*========================================================== */
static const u16 sr030pc50_Scene_Fall_Color[] = {
};

/*========================================================== */
/*	CAMERA_SCENE_FIREWORK (ISO50/Center/Br0/Auto/Sharp0/Sat0) */
/*========================================================== */
static const u16 sr030pc50_Scene_Fireworks[] = {
};

/*========================================================== */
/*	CAMERA_SCENE_TEXT (Auto/Center/Br0/Auto/Sharp+2/Sat0) */
/*========================================================== */
static const u16 sr030pc50_Scene_Text[] = {
};

/*========================================================== */
/*	CAMERA_SCEBE_CANDLELIGHT (Auto/Center/Br0/Daylight/Sharp0/Sat0) */
/*========================================================== */
static const u16 sr030pc50_Scene_Candle_Light[] = {
};

/*========================================================== */
/*	AF() - SEHF_AF_CHECK */
/*========================================================== */

static const u16 sr030pc50_AF_Return_Inf_pos[] = {
};

static const u16 sr030pc50_AF_Return_Macro_pos[] = {
};

static const u16 sr030pc50_AF_Normal_mode_1[] = {
};

static const u16 sr030pc50_AF_Normal_mode_2[] = {
};

static const u16 sr030pc50_AF_Normal_mode_3[] = {
};

static const u16 sr030pc50_AF_Macro_mode_1[] = {
};

static const u16 sr030pc50_AF_Macro_mode_2[] = {
};

static const u16 sr030pc50_AF_Macro_mode_3[] = {
};

static const u16 sr030pc50_AF_Low_Light_normal_mode_1[] = {
};

static const u16 sr030pc50_AF_Low_Light_normal_mode_2[] = {
};
static const u16 sr030pc50_AF_Low_Light_normal_mode_3[] = {
};

static const u16 sr030pc50_AF_Low_Light_Macro_mode_1[] = {
};

static const u16 sr030pc50_AF_Low_Light_Macro_mode_2[] = {
};

static const u16 sr030pc50_AF_Low_Light_Macro_mode_3[] = {
};

static const u16 sr030pc50_Single_AF_Start[] = {
};

static const u16 sr030pc50_Single_AF_Off_1[] = {
};

static const u16 sr030pc50_Low_Cap_On[] = {
};

static const u16 sr030pc50_Low_Cap_Off[] = {
};

static const u16 sr030pc50_Capture_Start[] = {
};

static const u16 sr030pc50_Preview_Return[] = {
};

static const u16 sr030pc50_Flash_init[] = {
};

static const u16 sr030pc50_Pre_Flash_On[] = {
};

static const u16 sr030pc50_Pre_Flash_Off[] = {
};

static const u16 sr030pc50_Main_Flash_On[] = {
};

static const u16 sr030pc50_Main_Flash_Off[] = {
};

/* ========================================================== */
/* Capture size(Width_Height) */
/* ========================================================== */

static const u16 sr030pc50_5M_Capture[] = {
};

static const u16 sr030pc50_W4M_Capture[] = {
};

static const u16 sr030pc50_3M_Capture[] = {
};

static const u16 sr030pc50_W2M_Capture[] = {
};

static const u16 sr030pc50_VGA_Capture[] = {
};

static const u16 sr030pc50_WVGA_Capture[] = {
};

/* ========================================================== */
/* preivew size() */
/* ========================================================== */

static const u16 sr030pc50_800_Preview[] = {
};

static const u16 sr030pc50_720_Preview[] = {
};

static const u16 sr030pc50_640_Preview[] = {
};

static const u16 sr030pc50_352_Preview[] = {
};



static const u16 sr030pc50_320_Preview[] = {
};




static const u16 sr030pc50_176_Preview[] = {
};

static const u16 sr030pc50_Preview_flip[] = {
};

static const u16 sr030pc50_Preview_Mirror[] = {
};



static const u16 sr030pc50_recording_720p[] = {
};

static const u16 sr030pc50_recording_720x480[] = {
};

static const u16 sr030pc50_recording_640x480[] = {
};



static const u16 sr030pc50_recording_320x240[] = {
};

static const u16 sr030pc50_recording_176x144[] = {
};

static const u16 sr030pc50_recording_720p_return[] = {
};

/* ========================================================== */
/* AE_AWE_LOCK(2) */
/* ========================================================== */
static const u16 sr030pc50_ae_lock[] = {
};

/*  AE unLock */
static const u16 sr030pc50_ae_unlock[] = {
};

/*  AWB Lock */
static const u16 sr030pc50_awb_lock[] = {
};

/*  AWB u16 */
static const u32 sr030pc50_awb_unlock[] = {
};

/*  TOUCH AF */
static const u16 sr030pc50_touchaf[] =  {
};





/*****************************************
ERROR!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
THE FOLLOWING ARE NOT DEFINED IN THE FILE THAT WE RECIEVED FROM SAMSUNG
NEED TO CHECK IN FUTURE
******************************************/
	/*****************************************
	ERROR!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	THE FOLLOWING ARE NOT DEFINED IN THE FILE THAT WE RECIEVED FROM SAMSUNG
	NEED TO CHECK IN FUTURE
	******************************************/


static const u16 sr030pc50_Night_Capture[] = {
};
static const u16 sr030pc50_AF_Low_Light_Mode_On[] = {
};
static const u16 sr030pc50_AF_Low_Light_Mode_Off[] = {
};
static const u16 sr030pc50_Single_AF_Off_2[] = {
};
static const u16 sr030pc50_Single_AF_Off_3[] = {
};

static const u16 sr030pc50_Face_Detection_On[] = {
};
static const u16 sr030pc50_Face_Detection_Off[] = {
};

static const u16 sr030pc50_Pre_Flash_Start[] = {
};
static const u16 sr030pc50_Pre_Flash_End[] = {
};
static const u16 sr030pc50_Flash_Start[] = {
};
static const u16 sr030pc50_Flash_End[] = {
};


/* 1600 x 1200 */
static const u16 sr030pc50_2M_Capture[] = {
};
/* 1280 x 960 */
static const u16 sr030pc50_1M_Capture[] = {
};
static const u16 sr030pc50_AE_AWB_Lock_On[] = {
};

static const u16 sr030pc50_AE_AWB_Lock_Off[] = {
};

static const u16 sr030pc50_Get_AE_Stable_Status[] = {
};

static const u16 sr030pc50_Get_Light_Level[] = {
};

static const u16 sr030pc50_get_1st_af_search_status[] = {
};

static const u16 sr030pc50_get_2nd_af_search_status[] = {
};

static const u16 sr030pc50_get_capture_status[] = {
};

static const u16 sr030pc50_get_esd_status[] = {
};

static const u16 sr030pc50_get_iso_reg[] = {
};

static const u16 sr030pc50_get_shutterspeed_reg[] = {
};


static const u16 sr030pc50_Sharpness_Minus_3[] = {
};

static const u16 sr030pc50_Sharpness_Plus_3[] = {
};
/* restores crop settings to full resolution */
static const u16 sr030pc50_Reset_Crop[] = {
};

static const u16 sr030pc50_zoom_0[] = {
};
static const u16 sr030pc50_zoom_1[] = {
};
static const u16 sr030pc50_zoom_2[] = {
};
static const u16 sr030pc50_zoom_3[] = {
};
static const u16 sr030pc50_zoom_4[] = {
};
static const u16 sr030pc50_zoom_5[] = {
};
static const u16 sr030pc50_zoom_6[] = {
};
static const u16 sr030pc50_zoom_7[] = {
};


/*****************************************
ERROR!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
THE ABOVE ARE NOT DEFINED IN THE FILE THAT WE RECIEVED FROM SAMSUNG
NEED TO CHECK IN FUTURE
******************************************/
#endif /* __CAMDRV_SS_SR030PC50_H__ */
