/* drivers/media/video/s5k4ecgx.c
 *
 * Driver for s5k4ecgx (5MP Camera) from SEC
 *
 * Copyright (C) 2010, SAMSUNG ELECTRONICS
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/version.h>
#include <linux/vmalloc.h>
#include <linux/completion.h>
#include <media/v4l2-device.h>
#include <media/v4l2-subdev.h>
#include <media/s5k4ecgx.h>
#include <linux/videodev2.h>
#include <media/v4l2-chip-ident.h>
#include <media/soc_camera.h>

#define dev_dbg1(dev, format, arg...)   dev_printk1(KERN_ERR, dev , format , ## arg)
#define dev_printk1(level, dev, format, arg...)   printk(level "%s %s: " format , dev_driver_string(dev) , 0 , ## arg)


#define CONFIG_VIDEO_S5K4ECGX_V_1_1
#ifdef CONFIG_VIDEO_S5K4ECGX_V_1_0
#include "s5k4ecgx_regs_1_0.h"
#define S5K4ECGX_VERSION_1_0	0x00
#endif

#ifdef CONFIG_VIDEO_S5K4ECGX_V_1_1
#include "s5k4ecgx_regs_1_1.h"
#define S5K4ECGX_VERSION_1_1	0x11
#endif


//@HW Fixed me, temporary only. Copied from OV5640
#if 1
#define S5K4ECGX_BRIGHTNESS_MIN           0
#define S5K4ECGX_BRIGHTNESS_MAX           200
#define S5K4ECGX_BRIGHTNESS_STEP          100
#define S5K4ECGX_BRIGHTNESS_DEF           100

#define S5K4ECGX_CONTRAST_MIN				0
#define S5K4ECGX_CONTRAST_MAX				200
#define S5K4ECGX_CONTRAST_STEP            100
#define S5K4ECGX_CONTRAST_DEF				100
#endif


#define FORMAT_FLAGS_COMPRESSED		0x3
#define SENSOR_JPEG_SNAPSHOT_MEMSIZE	0x410580
//SREE
#define DEFAULT_PIX_FMT		V4L2_PIX_FMT_UYVY//V4L2_PIX_FMT_YUYV	/* YUV422 */
#define DEFAULT_MBUS_PIX_FMT V4L2_MBUS_FMT_UYVY8_2X8 //V4L2_MBUS_FMT_YUYV8_2X8 
#define DEFAULT_MCLK		24000000
#define POLL_TIME_MS		10
#define CAPTURE_POLL_TIME_MS    1000

/* maximum time for one frame at minimum fps (15fps) in normal mode */
#define NORMAL_MODE_MAX_ONE_FRAME_DELAY_MS     67
/* maximum time for one frame at minimum fps (4fps) in night mode */
#define NIGHT_MODE_MAX_ONE_FRAME_DELAY_MS     250

/* time to move lens to target position before last af mode register write */
#define LENS_MOVE_TIME_MS       100

/* level at or below which we need to enable flash when in auto mode */
#define LOW_LIGHT_LEVEL		0x1D

/* level at or below which we need to use low light capture mode */
#define HIGH_LIGHT_LEVEL	0x80

#define FIRST_AF_SEARCH_COUNT   80
#define SECOND_AF_SEARCH_COUNT  80
#define AE_STABLE_SEARCH_COUNT  4

#define FIRST_SETTING_FOCUS_MODE_DELAY_MS	100
#define SECOND_SETTING_FOCUS_MODE_DELAY_MS	200
//#define CONFIG_VIDEO_S5K4ECGX_DEBUG

#ifdef CONFIG_VIDEO_S5K4ECGX_DEBUG
enum {
	S5K4ECGX_DEBUG_I2C		= 1U << 0,
	S5K4ECGX_DEBUG_I2C_BURSTS	= 1U << 1,
};
static uint32_t s5k4ecgx_debug_mask = 0xffffffff; //S5K4ECGX_DEBUG_I2C_BURSTS;
module_param_named(debug_mask, s5k4ecgx_debug_mask, uint, S_IWUSR | S_IRUGO);

#define s5k4ecgx_debug(mask, x...) \
	do { \
		if (s5k4ecgx_debug_mask & mask) \
			pr_info(x);	\
	} while (0)
#else

#define s5k4ecgx_debug(mask, x...)

#endif

#define S5K4ECGX_VERSION_1_1	0x11

/* result values returned to HAL */
enum {
	AUTO_FOCUS_FAILED,
	AUTO_FOCUS_DONE,
	AUTO_FOCUS_CANCELLED,
};

enum af_operation_status {
	AF_NONE = 0,
	AF_START,
	AF_CANCEL,
};

enum s5k4ecgx_oprmode {
	S5K4ECGX_OPRMODE_VIDEO = 0,
	S5K4ECGX_OPRMODE_IMAGE = 1,
};

enum s5k4ecgx_preview_frame_size {
	S5K4ECGX_PREVIEW_QCIF = 0,	/* 176x144 */
	S5K4ECGX_PREVIEW_QVGA,  /*320 x240*/	
	S5K4ECGX_PREVIEW_CIF,		/* 352x288 */
	S5K4ECGX_PREVIEW_VGA,		/* 640x480 */
	S5K4ECGX_PREVIEW_D1,		/* 720x480 */
//	S5K4ECGX_PREVIEW_WVGA,		/* 800x480 */
//	S5K4ECGX_PREVIEW_SVGA,		/* 800x600 */
//	S5K4ECGX_PREVIEW_WSVGA,		/* 1024x600*/
	S5K4ECGX_PREVIEW_MAX,
};

enum s5k4ecgx_capture_frame_size {
	S5K4ECGX_CAPTURE_VGA = 0,	/* 640x480 */
	S5K4ECGX_CAPTURE_WVGA,		/* 800x480 */
	S5K4ECGX_CAPTURE_SVGA,		/* 800x600 */
	S5K4ECGX_CAPTURE_WSVGA,		/* 1024x600 */
	S5K4ECGX_CAPTURE_1MP,		/* 1280x960 */
	S5K4ECGX_CAPTURE_W1MP,		/* 1600x960 */
	S5K4ECGX_CAPTURE_2MP,		/* UXGA  - 1600x1200 */
	S5K4ECGX_CAPTURE_W2MP,		/* 35mm Academy Offset Standard 1.66 */
					/* 2048x1232, 2.4MP */
	S5K4ECGX_CAPTURE_3MP,		/* QXGA  - 2048x1536 */
	S5K4ECGX_CAPTURE_W4MP,		/* WQXGA - 2560x1536 */
	S5K4ECGX_CAPTURE_5MP,		/* 2560x1920 */
	S5K4ECGX_CAPTURE_MAX,
};

enum s5k4ecgx_zoom {
	ZOOM_0 = 0,	
	ZOOM_1,		
	ZOOM_2,		
	ZOOM_3,		
	ZOOM_4,		
	ZOOM_5,		
	ZOOM_6,		
	ZOOM_7,		
	ZOOM_MAX,
};


struct s5k4ecgx_framesize {
	u32 index;
	u32 width;
	u32 height;
};


enum s5k4ecgx_wb_mode {
	WHITE_BALANCE_BASE = 0,
	WHITE_BALANCE_AUTO,
	WHITE_BALANCE_SUNNY,
	WHITE_BALANCE_CLOUDY,
	WHITE_BALANCE_TUNGSTEN,
	WHITE_BALANCE_FLUORESCENT,
	WHITE_BALANCE_MAX,
};

enum s5k4ecgx_scene_mode {
	SCENE_MODE_BASE = 0,
	SCENE_MODE_NONE,
	SCENE_MODE_PORTRAIT,
	SCENE_MODE_NIGHTSHOT,
	SCENE_MODE_BACK_LIGHT,
	SCENE_MODE_LANDSCAPE,
	SCENE_MODE_SPORTS,
	SCENE_MODE_PARTY_INDOOR,
	SCENE_MODE_BEACH_SNOW,
	SCENE_MODE_SUNSET,
	SCENE_MODE_DUST_DAWN,
	SCENE_MODE_FALL_COLOR,
	SCENE_MODE_FIREWORKS,
	SCENE_MODE_TEXT,
	SCENE_MODE_CANDLE_LIGHT,
	SCENE_MODE_MAX,
};

enum s5k4ecgx_sharpness_mode {
	SHARPNESS_MINUS_2 = 0,
	SHARPNESS_MINUS_1,
	SHARPNESS_DEFAULT,
	SHARPNESS_PLUS_1,
	SHARPNESS_PLUS_2,
	SHARPNESS_MAX,
};
enum s5k4ecgx_contrast_mode {
	CONTRAST_MINUS_2 = 0,
	CONTRAST_MINUS_1,
	CONTRAST_DEFAULT,
	CONTRAST_PLUS_1,
	CONTRAST_PLUS_2,
	CONTRAST_MAX,
};
enum s5k4ecgx_metering_mode {
	METERING_BASE = 0,
	METERING_MATRIX,
	METERING_CENTER,
	METERING_SPOT,
	METERING_MAX,
};
enum s5k4ecgx_frame_rate {
	FRAME_RATE_AUTO	= 0,
	FRAME_RATE_7	= 7,
	FRAME_RATE_15	= 15,
	FRAME_RATE_30	= 30,
	FRAME_RATE_60	= 60,
	FRAME_RATE_120	= 120,
	FRAME_RATE_MAX
};
enum s5k4ecgx_face_detection {
	FACE_DETECTION_OFF = 0,
	FACE_DETECTION_ON,
	FACE_DETECTION_NOLINE,
	FACE_DETECTION_ON_BEAUTY,
	FACE_DETECTION_MAX,
};

enum s5k4ecgx_focusmode {
	FOCUS_MODE_AUTO = 0,
	FOCUS_MODE_MACRO,
	FOCUS_MODE_FACEDETECT,
	FOCUS_MODE_AUTO_DEFAULT,
	FOCUS_MODE_MACRO_DEFAULT,
	FOCUS_MODE_FACEDETECT_DEFAULT,
	FOCUS_MODE_INFINITY,
	FOCUS_MODE_MAX,
};


enum s5k4ecgx_saturation_mode {
	SATURATION_MINUS_2 = 0,
	SATURATION_MINUS_1,
	SATURATION_DEFAULT,
	SATURATION_PLUS_1,
	SATURATION_PLUS_2,
	SATURATION_MAX,
};
enum s5k4ecgx_ev_mode {
	EV_MINUS_4 = 0,
	EV_MINUS_3,
	EV_MINUS_2,
	EV_MINUS_1,
	EV_DEFAULT,
	EV_PLUS_1,
	EV_PLUS_2,
	EV_PLUS_3,
	EV_PLUS_4,
	EV_MAX,
};
enum s5k4ecgx_effect_mode {
	IMAGE_EFFECT_BASE = 0,
	IMAGE_EFFECT_NONE,
	IMAGE_EFFECT_BNW,
	IMAGE_EFFECT_SEPIA,
	IMAGE_EFFECT_AQUA,
	IMAGE_EFFECT_ANTIQUE,
	IMAGE_EFFECT_NEGATIVE,
	IMAGE_EFFECT_SHARPEN,
	IMAGE_EFFECT_MAX,
};

enum s5k4ecgx_iso_mode {
	ISO_AUTO = 0,
	ISO_50,
	ISO_100,
	ISO_200,
	ISO_400,
	ISO_800,
	ISO_1600,
	ISO_SPORTS,
	ISO_NIGHT,
	ISO_MOVIE,
	ISO_MAX,
};
enum s5k4ecgx_flash_mode {
	FLASH_MODE_BASE,
	FLASH_MODE_OFF,
	FLASH_MODE_AUTO,
	FLASH_MODE_ON,
	FLASH_MODE_TORCH,
	FLASH_MODE_MAX,
};
enum s5k4ecgx_auto_focus {
	AUTO_FOCUS_OFF = 0,
	AUTO_FOCUS_ON,
	AUTO_FOCUS_MAX,
};
enum s5k4ecgx_wdr_mode {
	WDR_OFF,
	WDR_ON,
	WDR_MAX,
};

struct sec_cam_parm {
	struct v4l2_captureparm capture;
	int contrast;
	int effects;
	int brightness;
	int flash_mode;
	int focus_mode;
	int iso;
	int metering;
	int saturation;
	int scene_mode;
	int sharpness;
	int white_balance;
	int fps;
	int zoom;
};

static const struct s5k4ecgx_framesize s5k4ecgx_preview_framesize_list[] = {
	{ S5K4ECGX_PREVIEW_QCIF,	176,  144 },
	{ S5K4ECGX_PREVIEW_QVGA,	320,  240 },
	{ S5K4ECGX_PREVIEW_CIF,		352,  288 },
	{ S5K4ECGX_PREVIEW_VGA,		640,  480 },
	{ S5K4ECGX_PREVIEW_D1,		720,  480 },
};

static const struct s5k4ecgx_framesize s5k4ecgx_capture_framesize_list[] = {
	{ S5K4ECGX_CAPTURE_VGA,		640,  480 },
	{ S5K4ECGX_CAPTURE_1MP,		1280, 960 },
	{ S5K4ECGX_CAPTURE_2MP,		1600, 1200 },
	{ S5K4ECGX_CAPTURE_3MP,		2048, 1536 },
	{ S5K4ECGX_CAPTURE_5MP,		2560, 1920 },
};

struct s5k4ecgx_version {
	u32 major;
	u32 minor;
};

struct s5k4ecgx_date_info {
	u32 year;
	u32 month;
	u32 date;
};

enum s5k4ecgx_runmode {
	S5K4ECGX_RUNMODE_NOTREADY,
	S5K4ECGX_RUNMODE_IDLE,
	S5K4ECGX_RUNMODE_RUNNING,
	S5K4ECGX_RUNMODE_CAPTURE,
};

struct s5k4ecgx_firmware {
	u32 addr;
	u32 size;
};

struct s5k4ecgx_jpeg_param {
	u32 enable;
	u32 quality;
	u32 main_size;		/* Main JPEG file size */
	u32 thumb_size;		/* Thumbnail file size */
	u32 main_offset;
	u32 thumb_offset;
	u32 postview_offset;
};

struct s5k4ecgx_position {
	int x;
	int y;
};

struct gps_info_common {
	u32 direction;
	u32 dgree;
	u32 minute;
	u32 second;
};

struct s5k4ecgx_gps_info {
	unsigned char gps_buf[8];
	unsigned char altitude_buf[4];
	int gps_timeStamp;
};

struct s5k4ecgx_regset {
	u32 size;
	u8 *data;
};

struct s5k4ecgx_regset_table {
	const u32	*reg;
	int		array_size;
};

#define S5K4ECGX_REGSET(x, y)		\
	[(x)] = {					\
		.reg		= (y),			\
		.array_size	= ARRAY_SIZE((y)),	\
}

#define S5K4ECGX_REGSET_TABLE(y)		\
	{					\
		.reg		= (y),			\
		.array_size	= ARRAY_SIZE((y)),	\
}

struct s5k4ecgx_regs {
	struct s5k4ecgx_regset_table ev[EV_MAX];
	struct s5k4ecgx_regset_table metering[METERING_MAX];
	struct s5k4ecgx_regset_table iso[ISO_MAX];
	struct s5k4ecgx_regset_table effect[IMAGE_EFFECT_MAX];
	struct s5k4ecgx_regset_table white_balance[WHITE_BALANCE_MAX];
	struct s5k4ecgx_regset_table preview_size[S5K4ECGX_PREVIEW_MAX];
	struct s5k4ecgx_regset_table capture_size[S5K4ECGX_CAPTURE_MAX];
	struct s5k4ecgx_regset_table scene_mode[SCENE_MODE_MAX];
	struct s5k4ecgx_regset_table saturation[SATURATION_MAX];
	struct s5k4ecgx_regset_table contrast[CONTRAST_MAX];
	struct s5k4ecgx_regset_table sharpness[SHARPNESS_MAX];
	struct s5k4ecgx_regset_table fps[FRAME_RATE_MAX];
	struct s5k4ecgx_regset_table preview_return;
	struct s5k4ecgx_regset_table jpeg_quality_high;
	struct s5k4ecgx_regset_table jpeg_quality_normal;
	struct s5k4ecgx_regset_table jpeg_quality_low;
	struct s5k4ecgx_regset_table flash_start;
	struct s5k4ecgx_regset_table flash_end;
	struct s5k4ecgx_regset_table af_assist_flash_start;
	struct s5k4ecgx_regset_table af_assist_flash_end;
	struct s5k4ecgx_regset_table af_low_light_mode_on;
	struct s5k4ecgx_regset_table af_low_light_mode_off;
	struct s5k4ecgx_regset_table ae_awb_lock_on;
	struct s5k4ecgx_regset_table ae_awb_lock_off;
	struct s5k4ecgx_regset_table low_cap_on;
	struct s5k4ecgx_regset_table low_cap_off;
	struct s5k4ecgx_regset_table wdr_on;
	struct s5k4ecgx_regset_table wdr_off;
	struct s5k4ecgx_regset_table face_detection_on;
	struct s5k4ecgx_regset_table face_detection_off;
	struct s5k4ecgx_regset_table capture_start;
	struct s5k4ecgx_regset_table af_macro_mode_1;
	struct s5k4ecgx_regset_table af_macro_mode_2;
	struct s5k4ecgx_regset_table af_macro_mode_3;
	struct s5k4ecgx_regset_table af_normal_mode_1;
	struct s5k4ecgx_regset_table af_normal_mode_2;
	struct s5k4ecgx_regset_table af_normal_mode_3;
	struct s5k4ecgx_regset_table af_return_macro_position;
	struct s5k4ecgx_regset_table single_af_start;
	struct s5k4ecgx_regset_table single_af_off_1;
	struct s5k4ecgx_regset_table single_af_off_2;
	struct s5k4ecgx_regset_table dtp_start;
	struct s5k4ecgx_regset_table dtp_stop;
	struct s5k4ecgx_regset_table init_reg_1;
	struct s5k4ecgx_regset_table init_reg_2;
	struct s5k4ecgx_regset_table flash_init;
	struct s5k4ecgx_regset_table reset_crop;
	struct s5k4ecgx_regset_table get_ae_stable_status;
	struct s5k4ecgx_regset_table get_light_level;
	struct s5k4ecgx_regset_table get_1st_af_search_status;
	struct s5k4ecgx_regset_table get_2nd_af_search_status;
	struct s5k4ecgx_regset_table get_capture_status;
	struct s5k4ecgx_regset_table get_esd_status;
	struct s5k4ecgx_regset_table get_iso;
	struct s5k4ecgx_regset_table get_shutterspeed;
	struct s5k4ecgx_regset_table zoom[ZOOM_MAX];
};

#ifdef CONFIG_VIDEO_S5K4ECGX_V_1_0
static const struct s5k4ecgx_regs regs_for_fw_version_1_0 = {
	.ev = {
		S5K4ECGX_REGSET(EV_MINUS_4, s5k4ecgx_EV_Minus_4_v1),
		S5K4ECGX_REGSET(EV_MINUS_3, s5k4ecgx_EV_Minus_3_v1),
		S5K4ECGX_REGSET(EV_MINUS_2, s5k4ecgx_EV_Minus_2_v1),
		S5K4ECGX_REGSET(EV_MINUS_1, s5k4ecgx_EV_Minus_1_v1),
		S5K4ECGX_REGSET(EV_DEFAULT, s5k4ecgx_EV_Default_v1),
		S5K4ECGX_REGSET(EV_PLUS_1, s5k4ecgx_EV_Plus_1_v1),
		S5K4ECGX_REGSET(EV_PLUS_2, s5k4ecgx_EV_Plus_2_v1),
		S5K4ECGX_REGSET(EV_PLUS_3, s5k4ecgx_EV_Plus_3_v1),
		S5K4ECGX_REGSET(EV_PLUS_4, s5k4ecgx_EV_Plus_4_v1),
	},
	.metering = {
		S5K4ECGX_REGSET(METERING_MATRIX, s5k4ecgx_Metering_Matrix_v1),
		S5K4ECGX_REGSET(METERING_CENTER, s5k4ecgx_Metering_Center_v1),
		S5K4ECGX_REGSET(METERING_SPOT, s5k4ecgx_Metering_Spot_v1),
	},
	.iso = {
		S5K4ECGX_REGSET(ISO_AUTO, s5k4ecgx_ISO_Auto_v1),
		S5K4ECGX_REGSET(ISO_50, s5k4ecgx_ISO_100_v1),     /* use 100 */
		S5K4ECGX_REGSET(ISO_100, s5k4ecgx_ISO_100_v1),
		S5K4ECGX_REGSET(ISO_200, s5k4ecgx_ISO_200_v1),
		S5K4ECGX_REGSET(ISO_400, s5k4ecgx_ISO_400_v1),
		S5K4ECGX_REGSET(ISO_800, s5k4ecgx_ISO_400_v1),    /* use 400 */
		S5K4ECGX_REGSET(ISO_1600, s5k4ecgx_ISO_400_v1),   /* use 400 */
		S5K4ECGX_REGSET(ISO_SPORTS, s5k4ecgx_ISO_Auto_v1),/* use auto */
		S5K4ECGX_REGSET(ISO_NIGHT, s5k4ecgx_ISO_Auto_v1), /* use auto */
		S5K4ECGX_REGSET(ISO_MOVIE, s5k4ecgx_ISO_Auto_v1), /* use auto */
	},
	.effect = {
		S5K4ECGX_REGSET(IMAGE_EFFECT_NONE, s5k4ecgx_Effect_Normal_v1),
		S5K4ECGX_REGSET(IMAGE_EFFECT_BNW,
				s5k4ecgx_Effect_Black_White_v1),
		S5K4ECGX_REGSET(IMAGE_EFFECT_SEPIA, s5k4ecgx_Effect_Sepia_v1),
		S5K4ECGX_REGSET(IMAGE_EFFECT_NEGATIVE,
				s5k4ecgx_Effect_Negative_v1),
	},
	.white_balance = {
		S5K4ECGX_REGSET(WHITE_BALANCE_AUTO, s5k4ecgx_WB_Auto_v1),
		S5K4ECGX_REGSET(WHITE_BALANCE_SUNNY, s5k4ecgx_WB_Sunny_v1),
		S5K4ECGX_REGSET(WHITE_BALANCE_CLOUDY, s5k4ecgx_WB_Cloudy_v1),
		S5K4ECGX_REGSET(WHITE_BALANCE_TUNGSTEN,
				s5k4ecgx_WB_Tungsten_v1),
		S5K4ECGX_REGSET(WHITE_BALANCE_FLUORESCENT,
				s5k4ecgx_WB_Fluorescent_v1),
	},
	.preview_size = {
		S5K4ECGX_REGSET(S5K4ECGX_PREVIEW_QCIF, s5k4ecgx_176_Preview_v1),
		S5K4ECGX_REGSET(S5K4ECGX_PREVIEW_CIF, s5k4ecgx_352_Preview_v1),
		S5K4ECGX_REGSET(S5K4ECGX_PREVIEW_VGA, s5k4ecgx_640_Preview_v1),
		S5K4ECGX_REGSET(S5K4ECGX_PREVIEW_D1, s5k4ecgx_720_Preview_v1),
	},
	.capture_size = {
		S5K4ECGX_REGSET(S5K4ECGX_CAPTURE_VGA, s5k4ecgx_VGA_Capture_v1),
		S5K4ECGX_REGSET(S5K4ECGX_CAPTURE_1MP, s5k4ecgx_1M_Capture_v1),
		S5K4ECGX_REGSET(S5K4ECGX_CAPTURE_2MP, s5k4ecgx_2M_Capture_v1),
		S5K4ECGX_REGSET(S5K4ECGX_CAPTURE_3MP, s5k4ecgx_3M_Capture_v1),
		S5K4ECGX_REGSET(S5K4ECGX_CAPTURE_5MP, s5k4ecgx_5M_Capture_v1),
	},
	.scene_mode = {
		S5K4ECGX_REGSET(SCENE_MODE_NONE, s5k4ecgx_Scene_Default_v1),
		S5K4ECGX_REGSET(SCENE_MODE_PORTRAIT,
				s5k4ecgx_Scene_Portrait_v1),
		S5K4ECGX_REGSET(SCENE_MODE_NIGHTSHOT,
				s5k4ecgx_Scene_Nightshot_v1),
		S5K4ECGX_REGSET(SCENE_MODE_LANDSCAPE,
				s5k4ecgx_Scene_Landscape_v1),
		S5K4ECGX_REGSET(SCENE_MODE_SPORTS, s5k4ecgx_Scene_Sports_v1),
		S5K4ECGX_REGSET(SCENE_MODE_PARTY_INDOOR,
				s5k4ecgx_Scene_Party_Indoor_v1),
		S5K4ECGX_REGSET(SCENE_MODE_BEACH_SNOW,
				s5k4ecgx_Scene_Beach_Snow_v1),
		S5K4ECGX_REGSET(SCENE_MODE_SUNSET, s5k4ecgx_Scene_Sunset_v1),
		S5K4ECGX_REGSET(SCENE_MODE_FIREWORKS,
				s5k4ecgx_Scene_Fireworks_v1),
		S5K4ECGX_REGSET(SCENE_MODE_CANDLE_LIGHT,
				s5k4ecgx_Scene_Candle_Light_v1),
	},
	.saturation = {
		S5K4ECGX_REGSET(SATURATION_MINUS_2,
				s5k4ecgx_Saturation_Minus_2_v1),
		S5K4ECGX_REGSET(SATURATION_MINUS_1,
				s5k4ecgx_Saturation_Minus_1_v1),
		S5K4ECGX_REGSET(SATURATION_DEFAULT,
				s5k4ecgx_Saturation_Default_v1),
		S5K4ECGX_REGSET(SATURATION_PLUS_1,
				s5k4ecgx_Saturation_Plus_1_v1),
		S5K4ECGX_REGSET(SATURATION_PLUS_2,
				s5k4ecgx_Saturation_Plus_2_v1),
	},
	.contrast = {
		S5K4ECGX_REGSET(CONTRAST_MINUS_2, s5k4ecgx_Contrast_Minus_2_v1),
		S5K4ECGX_REGSET(CONTRAST_MINUS_1, s5k4ecgx_Contrast_Minus_1_v1),
		S5K4ECGX_REGSET(CONTRAST_DEFAULT, s5k4ecgx_Contrast_Default_v1),
		S5K4ECGX_REGSET(CONTRAST_PLUS_1, s5k4ecgx_Contrast_Plus_1_v1),
		S5K4ECGX_REGSET(CONTRAST_PLUS_2, s5k4ecgx_Contrast_Plus_2_v1),
	},
	.sharpness = {
		S5K4ECGX_REGSET(SHARPNESS_MINUS_2,
				s5k4ecgx_Sharpness_Minus_2_v1),
		S5K4ECGX_REGSET(SHARPNESS_MINUS_1,
				s5k4ecgx_Sharpness_Minus_1_v1),
		S5K4ECGX_REGSET(SHARPNESS_DEFAULT,
				s5k4ecgx_Sharpness_Default_v1),
		S5K4ECGX_REGSET(SHARPNESS_PLUS_1, s5k4ecgx_Sharpness_Plus_1_v1),
		S5K4ECGX_REGSET(SHARPNESS_PLUS_2, s5k4ecgx_Sharpness_Plus_2_v1),
	},
	.preview_return = S5K4ECGX_REGSET_TABLE(s5k4ecgx_Preview_Return_v1),
	.jpeg_quality_high =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_Jpeg_Quality_High_v1),
	.jpeg_quality_normal =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_Jpeg_Quality_Normal_v1),
	.jpeg_quality_low = S5K4ECGX_REGSET_TABLE(s5k4ecgx_Jpeg_Quality_Low_v1),
	.flash_start = S5K4ECGX_REGSET_TABLE(s5k4ecgx_Flash_Start_v1),
	.flash_end = S5K4ECGX_REGSET_TABLE(s5k4ecgx_Flash_End_v1),
	.af_assist_flash_start =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_Pre_Flash_Start_v1),
	.af_assist_flash_end =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_Pre_Flash_End_v1),
	.af_low_light_mode_on =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_AF_Low_Light_Mode_On_v1),
	.af_low_light_mode_off =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_AF_Low_Light_Mode_Off_v1),
	.ae_awb_lock_on =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_AE_AWB_Lock_On_v1),
	.ae_awb_lock_off =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_AE_AWB_Lock_Off_v1),
	.low_cap_on = S5K4ECGX_REGSET_TABLE(s5k4ecgx_Low_Cap_On_v1),
	.low_cap_off = S5K4ECGX_REGSET_TABLE(s5k4ecgx_Low_Cap_Off_v1),
	.wdr_on = S5K4ECGX_REGSET_TABLE(s5k4ecgx_WDR_on_v1),
	.wdr_off = S5K4ECGX_REGSET_TABLE(s5k4ecgx_WDR_off_v1),
	.face_detection_on =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_Face_Detection_On_v1),
	.face_detection_off =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_Face_Detection_Off_v1),
	.capture_start = S5K4ECGX_REGSET_TABLE(s5k4ecgx_Capture_Start_v1),
	.af_macro_mode_1 = S5K4ECGX_REGSET_TABLE(s5k4ecgx_AF_Macro_mode_1_v1),
	.af_macro_mode_2 = S5K4ECGX_REGSET_TABLE(s5k4ecgx_AF_Macro_mode_2_v1),
	.af_macro_mode_3 = S5K4ECGX_REGSET_TABLE(s5k4ecgx_AF_Macro_mode_3_v1),
	.af_normal_mode_1 = S5K4ECGX_REGSET_TABLE(s5k4ecgx_AF_Normal_mode_1_v1),
	.af_normal_mode_2 = S5K4ECGX_REGSET_TABLE(s5k4ecgx_AF_Normal_mode_2_v1),
	.af_normal_mode_3 = S5K4ECGX_REGSET_TABLE(s5k4ecgx_AF_Normal_mode_3_v1),
	.af_return_macro_position =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_AF_Return_Macro_pos_v1),
	.single_af_start = S5K4ECGX_REGSET_TABLE(s5k4ecgx_Single_AF_Start_v1),
	.single_af_off_1 = S5K4ECGX_REGSET_TABLE(s5k4ecgx_Single_AF_Off_1_v1),
	.single_af_off_2 = S5K4ECGX_REGSET_TABLE(s5k4ecgx_Single_AF_Off_2_v1),
	.dtp_start = S5K4ECGX_REGSET_TABLE(s5k4ecgx_DTP_init_v1),
	.dtp_stop = S5K4ECGX_REGSET_TABLE(s5k4ecgx_DTP_stop_v1),
	.init_reg_1 = S5K4ECGX_REGSET_TABLE(s5k4ecgx_init_reg1_v1),
	.init_reg_2 = S5K4ECGX_REGSET_TABLE(s5k4ecgx_init_reg2_v1),
	.flash_init = S5K4ECGX_REGSET_TABLE(s5k4ecgx_Flash_init_v1),
	.reset_crop = S5K4ECGX_REGSET_TABLE(s5k4ecgx_Reset_Crop_v1),
	.get_ae_stable_status =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_Get_AE_Stable_Status_v1),
	.get_light_level = S5K4ECGX_REGSET_TABLE(s5k4ecgx_Get_Light_Level_v1),
	.get_1st_af_search_status =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_get_1st_af_search_status_v1),
	.get_2nd_af_search_status =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_get_2nd_af_search_status_v1),
	.get_capture_status =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_get_capture_status_v1),
	.get_esd_status = S5K4ECGX_REGSET_TABLE(s5k4ecgx_get_esd_status_v1),
	.get_iso = S5K4ECGX_REGSET_TABLE(s5k4ecgx_get_iso_reg_v1),
	.get_shutterspeed =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_get_shutterspeed_reg_v1),
};
#endif

#ifdef CONFIG_VIDEO_S5K4ECGX_V_1_1
static const struct s5k4ecgx_regs regs_for_fw_version_1_1 = {
	.ev = {
		S5K4ECGX_REGSET(EV_MINUS_4, s5k4ecgx_EV_Minus_4),
		S5K4ECGX_REGSET(EV_MINUS_3, s5k4ecgx_EV_Minus_3),
		S5K4ECGX_REGSET(EV_MINUS_2, s5k4ecgx_EV_Minus_2),
		S5K4ECGX_REGSET(EV_MINUS_1, s5k4ecgx_EV_Minus_1),
		S5K4ECGX_REGSET(EV_DEFAULT, s5k4ecgx_EV_Default),
		S5K4ECGX_REGSET(EV_PLUS_1, s5k4ecgx_EV_Plus_1),
		S5K4ECGX_REGSET(EV_PLUS_2, s5k4ecgx_EV_Plus_2),
		S5K4ECGX_REGSET(EV_PLUS_3, s5k4ecgx_EV_Plus_3),
		S5K4ECGX_REGSET(EV_PLUS_4, s5k4ecgx_EV_Plus_4),
	},
	.metering = {
		S5K4ECGX_REGSET(METERING_MATRIX, s5k4ecgx_Metering_Matrix),
		S5K4ECGX_REGSET(METERING_CENTER, s5k4ecgx_Metering_Center),
		S5K4ECGX_REGSET(METERING_SPOT, s5k4ecgx_Metering_Spot),
	},
	.iso = {
		S5K4ECGX_REGSET(ISO_AUTO, s5k4ecgx_ISO_Auto),
		S5K4ECGX_REGSET(ISO_50, s5k4ecgx_ISO_100),     /* map to 100 */
		S5K4ECGX_REGSET(ISO_100, s5k4ecgx_ISO_100),
		S5K4ECGX_REGSET(ISO_200, s5k4ecgx_ISO_200),
		S5K4ECGX_REGSET(ISO_400, s5k4ecgx_ISO_400),
		S5K4ECGX_REGSET(ISO_800, s5k4ecgx_ISO_400),    /* map to 400 */
		S5K4ECGX_REGSET(ISO_1600, s5k4ecgx_ISO_400),   /* map to 400 */
		S5K4ECGX_REGSET(ISO_SPORTS, s5k4ecgx_ISO_Auto),/* map to auto */
		S5K4ECGX_REGSET(ISO_NIGHT, s5k4ecgx_ISO_Auto), /* map to auto */
		S5K4ECGX_REGSET(ISO_MOVIE, s5k4ecgx_ISO_Auto), /* map to auto */
	},
	.effect = {
		S5K4ECGX_REGSET(IMAGE_EFFECT_NONE, s5k4ecgx_Effect_Normal),
		S5K4ECGX_REGSET(IMAGE_EFFECT_BNW, s5k4ecgx_Effect_Black_White),
		S5K4ECGX_REGSET(IMAGE_EFFECT_SEPIA, s5k4ecgx_Effect_Sepia),
		S5K4ECGX_REGSET(IMAGE_EFFECT_NEGATIVE,
				s5k4ecgx_Effect_Negative),
	},
	.white_balance = {
		S5K4ECGX_REGSET(WHITE_BALANCE_AUTO, s5k4ecgx_WB_Auto),
		S5K4ECGX_REGSET(WHITE_BALANCE_SUNNY, s5k4ecgx_WB_Sunny),
		S5K4ECGX_REGSET(WHITE_BALANCE_CLOUDY, s5k4ecgx_WB_Cloudy),
		S5K4ECGX_REGSET(WHITE_BALANCE_TUNGSTEN, s5k4ecgx_WB_Tungsten),
		S5K4ECGX_REGSET(WHITE_BALANCE_FLUORESCENT,
				s5k4ecgx_WB_Fluorescent),
	},
	.preview_size = {
		S5K4ECGX_REGSET(S5K4ECGX_PREVIEW_QCIF, s5k4ecgx_176_Preview),
		S5K4ECGX_REGSET(S5K4ECGX_PREVIEW_QVGA, s5k4ecgx_320_Preview),
		S5K4ECGX_REGSET(S5K4ECGX_PREVIEW_CIF, s5k4ecgx_352_Preview),
		S5K4ECGX_REGSET(S5K4ECGX_PREVIEW_VGA, s5k4ecgx_640_Preview),
		S5K4ECGX_REGSET(S5K4ECGX_PREVIEW_D1, s5k4ecgx_720_Preview),
	},
	.capture_size = {
		S5K4ECGX_REGSET(S5K4ECGX_CAPTURE_VGA, s5k4ecgx_VGA_Capture),
		S5K4ECGX_REGSET(S5K4ECGX_CAPTURE_1MP, s5k4ecgx_1M_Capture),
		S5K4ECGX_REGSET(S5K4ECGX_CAPTURE_2MP, s5k4ecgx_2M_Capture),
		S5K4ECGX_REGSET(S5K4ECGX_CAPTURE_3MP, s5k4ecgx_3M_Capture),
		S5K4ECGX_REGSET(S5K4ECGX_CAPTURE_5MP, s5k4ecgx_5M_Capture),
	},
	.scene_mode = {
		S5K4ECGX_REGSET(SCENE_MODE_NONE, s5k4ecgx_Scene_Default),
		S5K4ECGX_REGSET(SCENE_MODE_PORTRAIT, s5k4ecgx_Scene_Portrait),
		S5K4ECGX_REGSET(SCENE_MODE_NIGHTSHOT, s5k4ecgx_Scene_Nightshot),
		S5K4ECGX_REGSET(SCENE_MODE_LANDSCAPE, s5k4ecgx_Scene_Landscape),
		S5K4ECGX_REGSET(SCENE_MODE_SPORTS, s5k4ecgx_Scene_Sports),
		S5K4ECGX_REGSET(SCENE_MODE_PARTY_INDOOR,
				s5k4ecgx_Scene_Party_Indoor),
		S5K4ECGX_REGSET(SCENE_MODE_BEACH_SNOW,
				s5k4ecgx_Scene_Beach_Snow),
		S5K4ECGX_REGSET(SCENE_MODE_SUNSET, s5k4ecgx_Scene_Sunset),
		S5K4ECGX_REGSET(SCENE_MODE_FIREWORKS, s5k4ecgx_Scene_Fireworks),
		S5K4ECGX_REGSET(SCENE_MODE_CANDLE_LIGHT,
				s5k4ecgx_Scene_Candle_Light),
	},
	.saturation = {
		S5K4ECGX_REGSET(SATURATION_MINUS_2,
				s5k4ecgx_Saturation_Minus_2),
		S5K4ECGX_REGSET(SATURATION_MINUS_1,
				s5k4ecgx_Saturation_Minus_1),
		S5K4ECGX_REGSET(SATURATION_DEFAULT,
				s5k4ecgx_Saturation_Default),
		S5K4ECGX_REGSET(SATURATION_PLUS_1, s5k4ecgx_Saturation_Plus_1),
		S5K4ECGX_REGSET(SATURATION_PLUS_2, s5k4ecgx_Saturation_Plus_2),
	},
	.contrast = {
		S5K4ECGX_REGSET(CONTRAST_MINUS_2, s5k4ecgx_Contrast_Minus_2),
		S5K4ECGX_REGSET(CONTRAST_MINUS_1, s5k4ecgx_Contrast_Minus_1),
		S5K4ECGX_REGSET(CONTRAST_DEFAULT, s5k4ecgx_Contrast_Default),
		S5K4ECGX_REGSET(CONTRAST_PLUS_1, s5k4ecgx_Contrast_Plus_1),
		S5K4ECGX_REGSET(CONTRAST_PLUS_2, s5k4ecgx_Contrast_Plus_2),
	},
	.sharpness = {
		S5K4ECGX_REGSET(SHARPNESS_MINUS_2, s5k4ecgx_Sharpness_Minus_2),
		S5K4ECGX_REGSET(SHARPNESS_MINUS_1, s5k4ecgx_Sharpness_Minus_1),
		S5K4ECGX_REGSET(SHARPNESS_DEFAULT, s5k4ecgx_Sharpness_Default),
		S5K4ECGX_REGSET(SHARPNESS_PLUS_1, s5k4ecgx_Sharpness_Plus_1),
		S5K4ECGX_REGSET(SHARPNESS_PLUS_2, s5k4ecgx_Sharpness_Plus_2),
	},
	.fps = {
		S5K4ECGX_REGSET(FRAME_RATE_AUTO, s5k4ecgx_FPS_Auto),
		S5K4ECGX_REGSET(FRAME_RATE_7, s5k4ecgx_FPS_7),
		S5K4ECGX_REGSET(FRAME_RATE_15, s5k4ecgx_FPS_15),
		S5K4ECGX_REGSET(FRAME_RATE_30, s5k4ecgx_FPS_30),
	},
	.preview_return = S5K4ECGX_REGSET_TABLE(s5k4ecgx_Preview_Return),
	.jpeg_quality_high = S5K4ECGX_REGSET_TABLE(s5k4ecgx_Jpeg_Quality_High),
	.jpeg_quality_normal =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_Jpeg_Quality_Normal),
	.jpeg_quality_low = S5K4ECGX_REGSET_TABLE(s5k4ecgx_Jpeg_Quality_Low),
	.flash_start = S5K4ECGX_REGSET_TABLE(s5k4ecgx_Flash_Start),
	.flash_end = S5K4ECGX_REGSET_TABLE(s5k4ecgx_Flash_End),
	.af_assist_flash_start =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_Pre_Flash_Start),
	.af_assist_flash_end =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_Pre_Flash_End),
	.af_low_light_mode_on =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_AF_Low_Light_Mode_On),
	.af_low_light_mode_off =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_AF_Low_Light_Mode_Off),
	.ae_awb_lock_on =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_AE_AWB_Lock_On),
	.ae_awb_lock_off =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_AE_AWB_Lock_Off),
	.low_cap_on = S5K4ECGX_REGSET_TABLE(s5k4ecgx_Low_Cap_On),
	.low_cap_off = S5K4ECGX_REGSET_TABLE(s5k4ecgx_Low_Cap_Off),
	.wdr_on = S5K4ECGX_REGSET_TABLE(s5k4ecgx_WDR_on),
	.wdr_off = S5K4ECGX_REGSET_TABLE(s5k4ecgx_WDR_off),
	.face_detection_on = S5K4ECGX_REGSET_TABLE(s5k4ecgx_Face_Detection_On),
	.face_detection_off =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_Face_Detection_Off),
	.capture_start = S5K4ECGX_REGSET_TABLE(s5k4ecgx_Capture_Start),
	.af_macro_mode_1 = S5K4ECGX_REGSET_TABLE(s5k4ecgx_AF_Macro_mode_1),
	.af_macro_mode_2 = S5K4ECGX_REGSET_TABLE(s5k4ecgx_AF_Macro_mode_2),
	.af_macro_mode_3 = S5K4ECGX_REGSET_TABLE(s5k4ecgx_AF_Macro_mode_3),
	.af_normal_mode_1 = S5K4ECGX_REGSET_TABLE(s5k4ecgx_AF_Normal_mode_1),
	.af_normal_mode_2 = S5K4ECGX_REGSET_TABLE(s5k4ecgx_AF_Normal_mode_2),
	.af_normal_mode_3 = S5K4ECGX_REGSET_TABLE(s5k4ecgx_AF_Normal_mode_3),
	.af_return_macro_position =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_AF_Return_Macro_pos),
	.single_af_start = S5K4ECGX_REGSET_TABLE(s5k4ecgx_Single_AF_Start),
	.single_af_off_1 = S5K4ECGX_REGSET_TABLE(s5k4ecgx_Single_AF_Off_1),
	.single_af_off_2 = S5K4ECGX_REGSET_TABLE(s5k4ecgx_Single_AF_Off_2),
	.dtp_start = S5K4ECGX_REGSET_TABLE(s5k4ecgx_DTP_init),
	.dtp_stop = S5K4ECGX_REGSET_TABLE(s5k4ecgx_DTP_stop),
	.init_reg_1 = S5K4ECGX_REGSET_TABLE(s5k4ecgx_init_reg1),
	.init_reg_2 = S5K4ECGX_REGSET_TABLE(s5k4ecgx_init_reg2),
	.flash_init = S5K4ECGX_REGSET_TABLE(s5k4ecgx_Flash_init),
	.reset_crop = S5K4ECGX_REGSET_TABLE(s5k4ecgx_Reset_Crop),
	.get_ae_stable_status =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_Get_AE_Stable_Status),
	.get_light_level = S5K4ECGX_REGSET_TABLE(s5k4ecgx_Get_Light_Level),
	.get_1st_af_search_status =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_get_1st_af_search_status),
	.get_2nd_af_search_status =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_get_2nd_af_search_status),
	.get_capture_status =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_get_capture_status),
	.get_esd_status = S5K4ECGX_REGSET_TABLE(s5k4ecgx_get_esd_status),
	.get_iso = S5K4ECGX_REGSET_TABLE(s5k4ecgx_get_iso_reg),
	.get_shutterspeed =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_get_shutterspeed_reg),
	.zoom ={
		S5K4ECGX_REGSET(ZOOM_0, s5k4ecgx_zoom_0),
		S5K4ECGX_REGSET(ZOOM_1, s5k4ecgx_zoom_1),
		S5K4ECGX_REGSET(ZOOM_2, s5k4ecgx_zoom_2),
		S5K4ECGX_REGSET(ZOOM_3, s5k4ecgx_zoom_3),
		S5K4ECGX_REGSET(ZOOM_4, s5k4ecgx_zoom_4),
		S5K4ECGX_REGSET(ZOOM_5, s5k4ecgx_zoom_5),
		S5K4ECGX_REGSET(ZOOM_6, s5k4ecgx_zoom_6),
		S5K4ECGX_REGSET(ZOOM_7, s5k4ecgx_zoom_7),
	},
};
#endif

struct s5k4ecgx_state {
	struct s5k4ecgx_platform_data *pdata;
	struct v4l2_subdev sd;
	struct v4l2_pix_format pix;
	struct v4l2_fract timeperframe;
	struct v4l2_subdev_sensor_interface_parms *plat_parms;
	struct s5k4ecgx_jpeg_param jpeg;
	struct s5k4ecgx_version fw;
	struct s5k4ecgx_version prm;
	struct s5k4ecgx_date_info dateinfo;
	struct s5k4ecgx_position position;
	struct v4l2_streamparm strm;
	struct s5k4ecgx_gps_info gps_info;
	struct mutex ctrl_lock;
	struct completion af_complete;
	enum s5k4ecgx_runmode runmode;
	enum s5k4ecgx_oprmode oprmode;
	enum af_operation_status af_status;
	int preview_framesize_index;
	int capture_framesize_index;
	int sensor_version;
	int freq;		/* MCLK in Hz */
	int check_dataline;
	int check_previewdata;
	bool flash_on;
	bool sensor_af_in_low_light_mode;
	bool flash_state_on_previous_capture;
	bool initialized;
	bool restore_preview_size_needed;
	int one_frame_delay_ms;
	const struct s5k4ecgx_regs *regs;
};

static const struct v4l2_fmtdesc capture_fmts[] = {
	{
		.index		= 0,
		.type		= V4L2_BUF_TYPE_VIDEO_CAPTURE,
		.flags		= FORMAT_FLAGS_COMPRESSED,
		.description	= "JPEG + Postview",
		.pixelformat	= V4L2_PIX_FMT_JPEG,
	},
};

/**
 * s5k4ecgx_i2c_read_twobyte: Read 2 bytes from sensor
 */
static int s5k4ecgx_i2c_read_twobyte(struct i2c_client *client,
				  u16 subaddr, u16 *data)
{
	int err;
	unsigned char buf[2];
	struct i2c_msg msg[2];

	cpu_to_be16s(&subaddr);

	msg[0].addr = client->addr;
	msg[0].flags = 0;
	msg[0].len = 2;
	msg[0].buf = (u8 *)&subaddr;

	msg[1].addr = client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = 2;
	msg[1].buf = buf;

	err = i2c_transfer(client->adapter, msg, 2);
	if (unlikely(err != 2)) {
		dev_err(&client->dev,
			"%s: register read fail\n", __func__);
		return -EIO;
	}

	*data = ((buf[0] << 8) | buf[1]);

	return 0;
}

/**
 * s5k4ecgx_i2c_write_twobyte: Write (I2C) multiple bytes to the camera sensor
 * @client: pointer to i2c_client
 * @cmd: command register
 * @w_data: data to be written
 * @w_len: length of data to be written
 *
 * Returns 0 on success, <0 on error
 */
static int s5k4ecgx_i2c_write_twobyte(struct i2c_client *client,
					 u16 addr, u16 w_data)
{
	int retry_count = 5;
	unsigned char buf[4];
	struct i2c_msg msg = {client->addr, 0, 4, buf};
	int ret = 0;

//printk("BILLA s5k4ecgx_i2c_write_twobyte E \n");
	buf[0] = addr >> 8;
	buf[1] = addr;
	buf[2] = w_data >> 8;
	buf[3] = w_data & 0xff;

//	s5k4ecgx_debug(S5K4ECGX_DEBUG_I2C, "%s : W(0x%02X%02X%02X%02X)\n",
//		__func__, buf[0], buf[1], buf[2], buf[3]);

	do {
		ret = i2c_transfer(client->adapter, &msg, 1);
		if (likely(ret == 1))
			break;
		msleep(POLL_TIME_MS);
           printk("BILLA s5k4ecgx_i2c_write_twobyte fail 1\n");
		dev_err(&client->dev, "%s: I2C err %d, retry %d.\n",
			__func__, ret, retry_count);
	} while (retry_count-- > 0);
	if (ret != 1) {
                   printk("BILLA s5k4ecgx_i2c_write_twobyte  not working fail\n");
		dev_err(&client->dev, "%s: I2C is not working.\n", __func__);
		return -EIO;
	}
// printk("BILLA s5k4ecgx_i2c_write_twobyte  success");
	return 0;
}

static const struct v4l2_queryctrl s5k4ecgx_controls[] = {
	{
		.id      	= V4L2_CID_BRIGHTNESS,
		.type    	= V4L2_CTRL_TYPE_INTEGER,
		.name    	= "Brightness",
		.minimum	= S5K4ECGX_BRIGHTNESS_MIN,
		.maximum	= S5K4ECGX_BRIGHTNESS_MAX,
		.step		= S5K4ECGX_BRIGHTNESS_STEP,
		.default_value	= S5K4ECGX_BRIGHTNESS_DEF,
	},
	{
		.id		= V4L2_CID_CONTRAST,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.name		= "Contrast",
		.minimum	= S5K4ECGX_CONTRAST_MIN,
		.maximum	= S5K4ECGX_CONTRAST_MAX,
		.step		= S5K4ECGX_CONTRAST_STEP,
		.default_value	= S5K4ECGX_CONTRAST_DEF,
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
static int s5k4ecgx_write_regs(struct v4l2_subdev *sd, const u32 regs[],
			     int size)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int i, err;

  printk("BILLA s5k4ecgx_write_regs  E \n");

	for (i = 0; i < size; i++) {
		err = s5k4ecgx_i2c_write_twobyte(client,
			(regs[i] >> 16), regs[i]);
		if (unlikely(err != 0)) {
 printk("BILLA s5k4ecgx_write_regs  fail 1 \n");
			dev_err(&client->dev,
				"%s: register write failed\n", __func__);
			return err;
		}
	}
 printk("BILLA s5k4ecgx_write_regs  succ \n");


	return 0;
}

static int s5k4ecgx_set_from_table(struct v4l2_subdev *sd,
				const char *setting_name,
				const struct s5k4ecgx_regset_table *table,
				int table_size, int index)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	dev_dbg1(&client->dev, "%s: set %s index %d\n",
		__func__, setting_name, index);

      printk("BILLA s5k4ecgx_set_from_table E \n");
	if ((index < 0) || (index >= table_size)) {
		dev_err(&client->dev,
			"%s: index(%d) out of range[0:%d] for table for %s\n",
			__func__, index, table_size, setting_name);
  printk("BILLA s5k4ecgx_set_from_table fail1\n");
		return -EINVAL;
	}
	table += index;
	if (table->reg == NULL)
		return -EINVAL;
	return s5k4ecgx_write_regs(sd, table->reg, table->array_size);
}

static int s5k4ecgx_set_parameter(struct v4l2_subdev *sd,
				int *current_value_ptr,
				int new_value,
				const char *setting_name,
				const struct s5k4ecgx_regset_table *table,
				int table_size)
{
	int err;

	if (*current_value_ptr == new_value)
		return 0;

	err = s5k4ecgx_set_from_table(sd, setting_name, table,
				table_size, new_value);

	if (!err)
		*current_value_ptr = new_value;
	return err;
}

static int s5k4ecgx_set_preview_stop(struct v4l2_subdev *sd)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);

	if (state->runmode == S5K4ECGX_RUNMODE_RUNNING)
		state->runmode = S5K4ECGX_RUNMODE_IDLE;

	dev_dbg1(&client->dev, "%s:\n", __func__);

	return 0;
}
static int s5k4ecgx_set_preview_start(struct v4l2_subdev *sd)
{
	int err;
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	bool set_size = true;

        printk("BILLA s5k4ecgx_set_preview_start E with index =%d \n",state->preview_framesize_index);

//	dev_dbg1(&client->dev, "%s: runmode = %d\n",
//		__func__, state->runmode);

	if(state->pix.width == 0)
	{
		 printk("BILLA CHECK s5k4ecgx_set_preview_start width = 0 fail !!!!!!\n");
		 return;
	}
	if(state->pix.height== 0)
	{
		 printk("BILLA CHECK s5k4ecgx_set_preview_start height = 0 fail !!!!!\n");
		 return;
	}

	if( state->strm.parm.capture.timeperframe.denominator == 0)
	{
		state->strm.parm.capture.timeperframe.denominator = 15;
		 printk("BILLA CHECK state->strm.parm.capture.timeperframe.denominator = 0 set to 15\n");
	}
	
	if (!state->pix.width || !state->pix.height ||
		!state->strm.parm.capture.timeperframe.denominator)
	{
   		 printk("BILLA s5k4ecgx_set_preview_start E fail 1\n");
		return -EINVAL;
	}

	if (state->runmode == S5K4ECGX_RUNMODE_CAPTURE) {
		dev_dbg1(&client->dev, "%s: sending Preview_Return cmd\n",
			__func__);
		err = s5k4ecgx_set_from_table(sd, "preview return",
					&state->regs->preview_return, 1, 0);
		if (err < 0) {
                         printk("BILLA s5k4ecgx_set_preview_start E fail 2\n");
			dev_err(&client->dev,
				"%s: failed: s5k4ecgx_Preview_Return\n",
				__func__);
			return -EIO;
		}
   printk("BILLA s5k4ecgx_set_preview_start E 1 \n");
		set_size = state->restore_preview_size_needed;
	}

	if (set_size) {
#if 1
		err = s5k4ecgx_set_from_table(sd, "preview_size",
					state->regs->preview_size,
					ARRAY_SIZE(state->regs->preview_size),
					state->preview_framesize_index);
		if (err < 0) {
                           printk("BILLA s5k4ecgx_set_preview_start fail 2\n");
			dev_err(&client->dev,
				"%s: failed: Could not set preview size\n",
				__func__);
			return -EIO;
		}
		       printk("BILLA s5k4ecgx_set_preview_start E 2 \n");
#endif
  
	}

	dev_dbg1(&client->dev, "%s: runmode now RUNNING\n", __func__);
	state->runmode = S5K4ECGX_RUNMODE_RUNNING;
 printk("BILLA s5k4ecgx_set_preview_start success \n");
	return 0;
}

static int s5k4ecgx_set_capture_size(struct v4l2_subdev *sd)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);
	int err;

	dev_dbg1(&client->dev, "%s: index:%d\n", __func__,
		state->capture_framesize_index);

	err = s5k4ecgx_set_from_table(sd, "capture_size",
				state->regs->capture_size,
				ARRAY_SIZE(state->regs->capture_size),
				state->capture_framesize_index);
	if (err < 0) {
		dev_err(&client->dev,
			"%s: failed: i2c_write for capture_size index %d\n",
			__func__, state->capture_framesize_index);
	}
	state->runmode = S5K4ECGX_RUNMODE_CAPTURE;

	return err;
}

static int s5k4ecgx_set_jpeg_quality(struct v4l2_subdev *sd)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);

	dev_dbg1(&client->dev,
		"%s: jpeg.quality %d\n", __func__, state->jpeg.quality);
	if (state->jpeg.quality < 0)
		state->jpeg.quality = 0;
	if (state->jpeg.quality > 100)
		state->jpeg.quality = 100;

	switch (state->jpeg.quality) {
	case 90 ... 100:
		dev_dbg1(&client->dev,
			"%s: setting to high jpeg quality\n", __func__);
		return s5k4ecgx_set_from_table(sd, "jpeg quality high",
				&state->regs->jpeg_quality_high, 1, 0);
	case 80 ... 89:
		dev_dbg1(&client->dev,
			"%s: setting to normal jpeg quality\n", __func__);
		return s5k4ecgx_set_from_table(sd, "jpeg quality normal",
				&state->regs->jpeg_quality_normal, 1, 0);
	default:
		dev_dbg1(&client->dev,
			"%s: setting to low jpeg quality\n", __func__);
		return s5k4ecgx_set_from_table(sd, "jpeg quality low",
				&state->regs->jpeg_quality_low, 1, 0);
	}
}

static u16 s5k4ecgx_get_light_level(struct v4l2_subdev *sd)
{
	int err;
	u16 read_value = 0;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);

	err = s5k4ecgx_set_from_table(sd, "get light level",
				&state->regs->get_light_level, 1, 0);
	if (err) {
		dev_err(&client->dev,
			"%s: write cmd failed, returning 0\n", __func__);
		goto out;
	}
	err = s5k4ecgx_i2c_read_twobyte(client, 0x0F12, &read_value);
	if (err) {
		dev_err(&client->dev,
			"%s: read cmd failed, returning 0\n", __func__);
		goto out;
	}

	dev_dbg1(&client->dev, "%s: read_value = %d (0x%X)\n",
		__func__, read_value, read_value);

out:
	/* restore write mode */
	s5k4ecgx_i2c_write_twobyte(client, 0x0028, 0x7000);
	return read_value;
}

static int s5k4ecgx_start_capture(struct v4l2_subdev *sd)
{
	int err;
	u16 read_value;
	u16 light_level;
	int poll_time_ms;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);
	struct sec_cam_parm *parms =
		(struct sec_cam_parm *)&state->strm.parm.raw_data;
	struct s5k4ecgx_platform_data *pdata = client->dev.platform_data;

	/* reset cropping if our current preview is not 640x480,
	 * otherwise the capture will be wrong because of the cropping
	 */
	if (state->preview_framesize_index != S5K4ECGX_PREVIEW_VGA) {
		int err = s5k4ecgx_set_from_table(sd, "reset crop",
						&state->regs->reset_crop, 1, 0);
		if (err < 0) {
			dev_err(&client->dev,
				"%s: failed: Could not set preview size\n",
				__func__);
			return -EIO;
		}
		state->restore_preview_size_needed = true;
	} else
		state->restore_preview_size_needed = false;

	light_level = s5k4ecgx_get_light_level(sd);

	dev_dbg1(&client->dev, "%s: light_level = %d\n", __func__,
		light_level);

	state->flash_state_on_previous_capture = false;
	if (parms->scene_mode != SCENE_MODE_NIGHTSHOT) {
		switch (parms->flash_mode) {
		case FLASH_MODE_AUTO:
			if (light_level > LOW_LIGHT_LEVEL) {
				/* light level bright enough
				 * that we don't need flash
				 */
				break;
			}
			/* fall through to flash start */
		case FLASH_MODE_ON:
			if (parms->focus_mode == FOCUS_MODE_INFINITY) {
				s5k4ecgx_set_from_table(sd,
					"AF assist flash start",
					&state->regs->af_assist_flash_start,
					1, 0);
				s5k4ecgx_set_from_table(sd,
					"AF assist flash end",
					&state->regs->af_assist_flash_end,
					1, 0);
				msleep(10);
			}
			s5k4ecgx_set_from_table(sd, "flash start",
					&state->regs->flash_start, 1, 0);
			state->flash_on = true;
			state->flash_state_on_previous_capture = true;
			pdata->flash_onoff(1);
			break;
		default:
			break;
		}
	}

	/* if light is low, use low light capture settings, EXCEPT
	 * if scene mode set to NIGHTSHOT or SPORTS because they
	 * have their own settings (though a low light sport setting
	 * could be useful)
	 */
	if ((light_level <= HIGH_LIGHT_LEVEL) &&
		(parms->scene_mode != SCENE_MODE_NIGHTSHOT) &&
		(parms->scene_mode != SCENE_MODE_SPORTS)) {
		s5k4ecgx_set_from_table(sd, "low cap on",
					&state->regs->low_cap_on, 1, 0);
	}

	err = s5k4ecgx_set_capture_size(sd);
	if (err < 0) {
		dev_err(&client->dev,
			"%s: failed: i2c_write for capture_resolution\n",
			__func__);
		return -EIO;
	}

	dev_dbg1(&client->dev, "%s: send Capture_Start cmd\n", __func__);
	s5k4ecgx_set_from_table(sd, "capture start",
				&state->regs->capture_start, 1, 0);

	/* a shot takes takes at least 50ms so sleep that amount first
	 * and then start polling for completion.
	 */
	msleep(50);
	/* Enter read mode */
	s5k4ecgx_i2c_write_twobyte(client, 0x002C, 0x7000);
	poll_time_ms = 50;
	do {
		s5k4ecgx_set_from_table(sd, "get capture status",
					&state->regs->get_capture_status, 1, 0);
		s5k4ecgx_i2c_read_twobyte(client, 0x0F12, &read_value);
		dev_dbg1(&client->dev,
			"%s: s5k4ecgx_Capture_Start check = %#x\n",
			__func__, read_value);
		if (read_value != 0x00)
			break;
		msleep(POLL_TIME_MS);
		poll_time_ms += POLL_TIME_MS;
	} while (poll_time_ms < CAPTURE_POLL_TIME_MS);

	dev_dbg1(&client->dev, "%s: capture done check finished after %d ms\n",
		__func__, poll_time_ms);

	/* restore write mode */
	s5k4ecgx_i2c_write_twobyte(client, 0x0028, 0x7000);

	s5k4ecgx_set_from_table(sd, "ae awb lock off",
				&state->regs->ae_awb_lock_off, 1, 0);

	if ((light_level <= HIGH_LIGHT_LEVEL) &&
		(parms->scene_mode != SCENE_MODE_NIGHTSHOT) &&
		(parms->scene_mode != SCENE_MODE_SPORTS)) {
		s5k4ecgx_set_from_table(sd, "low cap off",
					&state->regs->low_cap_off, 1, 0);
	}

	if ((parms->scene_mode != SCENE_MODE_NIGHTSHOT) && (state->flash_on)) {
		state->flash_on = false;
		pdata->flash_onoff(0);
		s5k4ecgx_set_from_table(sd, "flash end",
					&state->regs->flash_end, 1, 0);
	}

	return 0;
}

/* wide dynamic range support */
static int s5k4ecgx_set_wdr(struct v4l2_subdev *sd, int value)
{
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);

	if (value == WDR_ON)
		return s5k4ecgx_set_from_table(sd, "wdr on",
					&state->regs->wdr_on, 1, 0);
	return s5k4ecgx_set_from_table(sd, "wdr off",
				&state->regs->wdr_off, 1, 0);
}

static int s5k4ecgx_set_face_detection(struct v4l2_subdev *sd, int value)
{
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);

	if (value == FACE_DETECTION_ON)
		return s5k4ecgx_set_from_table(sd, "face detection on",
				&state->regs->face_detection_on, 1, 0);
	return s5k4ecgx_set_from_table(sd, "face detection off",
				&state->regs->face_detection_off, 1, 0);
}

static int s5k4ecgx_return_focus(struct v4l2_subdev *sd)
{
	int err;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct s5k4ecgx_state *state =
			container_of(sd, struct s5k4ecgx_state, sd);

	err = s5k4ecgx_set_from_table(sd,
		"af normal mode 1",
		&state->regs->af_normal_mode_1, 1, 0);
	if (err < 0)
		goto fail;
	msleep(FIRST_SETTING_FOCUS_MODE_DELAY_MS);
	err = s5k4ecgx_set_from_table(sd,
		"af normal mode 2",
		&state->regs->af_normal_mode_2, 1, 0);
	if (err < 0)
		goto fail;
	msleep(SECOND_SETTING_FOCUS_MODE_DELAY_MS);
	err = s5k4ecgx_set_from_table(sd,
		"af normal mode 3",
		&state->regs->af_normal_mode_3, 1, 0);
	if (err < 0)
		goto fail;

	return 0;
fail:
	dev_err(&client->dev,
		"%s: i2c_write failed\n", __func__);
	return -EIO;
}

static int s5k4ecgx_set_focus_mode(struct v4l2_subdev *sd, int value)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);
	struct sec_cam_parm *parms =
		(struct sec_cam_parm *)&state->strm.parm.raw_data;
	int err;

	if (parms->focus_mode == value)
		return 0;

	dev_dbg1(&client->dev, "%s value(%d)\n", __func__, value);

	switch (value) {
	case FOCUS_MODE_MACRO:
		dev_dbg1(&client->dev,
				"%s: FOCUS_MODE_MACRO\n", __func__);
		err = s5k4ecgx_set_from_table(sd, "af macro mode 1",
				&state->regs->af_macro_mode_1, 1, 0);
		if (err < 0)
			goto fail;
		msleep(FIRST_SETTING_FOCUS_MODE_DELAY_MS);
		err = s5k4ecgx_set_from_table(sd, "af macro mode 2",
				&state->regs->af_macro_mode_2, 1, 0);
		if (err < 0)
			goto fail;
		msleep(SECOND_SETTING_FOCUS_MODE_DELAY_MS);
		err = s5k4ecgx_set_from_table(sd, "af macro mode 3",
				&state->regs->af_macro_mode_3, 1, 0);
		if (err < 0)
			goto fail;
		parms->focus_mode = FOCUS_MODE_MACRO;
		break;

	case FOCUS_MODE_INFINITY:
	case FOCUS_MODE_AUTO:
		err = s5k4ecgx_set_from_table(sd,
			"af normal mode 1",
			&state->regs->af_normal_mode_1, 1, 0);
		if (err < 0)
			goto fail;
		msleep(FIRST_SETTING_FOCUS_MODE_DELAY_MS);
		err = s5k4ecgx_set_from_table(sd,
			"af normal mode 2",
			&state->regs->af_normal_mode_2, 1, 0);
		if (err < 0)
			goto fail;
		msleep(SECOND_SETTING_FOCUS_MODE_DELAY_MS);
		err = s5k4ecgx_set_from_table(sd,
			"af normal mode 3",
			&state->regs->af_normal_mode_3, 1, 0);
		if (err < 0)
			goto fail;
		parms->focus_mode = value;
		break;
	default:
		return -EINVAL;
		break;
	}

	return 0;
fail:
	dev_err(&client->dev,
		"%s: i2c_write failed\n", __func__);
	return -EIO;
}

static void s5k4ecgx_auto_focus_flash_start(struct v4l2_subdev *sd)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);
	struct s5k4ecgx_platform_data *pdata = client->dev.platform_data;
	int count;
	u16 read_value;

	s5k4ecgx_set_from_table(sd, "AF assist flash start",
				&state->regs->af_assist_flash_start, 1, 0);
	state->flash_on = true;
	pdata->af_assist_onoff(1);

	/* delay 200ms (SLSI value) and then poll to see if AE is stable.
	 * once it is stable, lock it and then return to do AF
	 */
	msleep(200);

	/* enter read mode */
	s5k4ecgx_i2c_write_twobyte(client, 0x002C, 0x7000);
	for (count = 0; count < AE_STABLE_SEARCH_COUNT; count++) {
		if (state->af_status == AF_CANCEL)
			break;
		s5k4ecgx_set_from_table(sd, "get ae stable status",
				&state->regs->get_ae_stable_status, 1, 0);
		s5k4ecgx_i2c_read_twobyte(client, 0x0F12, &read_value);
		dev_dbg1(&client->dev, "%s: ae stable status = %#x\n",
			__func__, read_value);
		if (read_value == 0x1)
			break;
		msleep(state->one_frame_delay_ms);
	}

	/* restore write mode */
	s5k4ecgx_i2c_write_twobyte(client, 0x0028, 0x7000);

	/* if we were cancelled, turn off flash */
	if (state->af_status == AF_CANCEL) {
		dev_dbg1(&client->dev,
			"%s: AF cancelled\n", __func__);
		s5k4ecgx_set_from_table(sd, "AF assist flash end",
				&state->regs->af_assist_flash_end, 1, 0);
		state->flash_on = false;
		pdata->af_assist_onoff(0);
	}
}

static int s5k4ecgx_start_auto_focus(struct v4l2_subdev *sd)
{
	int light_level;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);
	struct sec_cam_parm *parms =
		(struct sec_cam_parm *)&state->strm.parm.raw_data;

	dev_dbg1(&client->dev, "%s: start SINGLE AF operation, flash mode %d\n",
		__func__, parms->flash_mode);

	/* in case user calls auto_focus repeatedly without a cancel
	 * or a capture, we need to cancel here to allow ae_awb
	 * to work again, or else we could be locked forever while
	 * that app is running, which is not the expected behavior.
	 */
	s5k4ecgx_set_from_table(sd, "ae awb lock off",
				&state->regs->ae_awb_lock_off, 1, 0);

	if (parms->scene_mode == SCENE_MODE_NIGHTSHOT) {
		/* user selected night shot mode, assume we need low light
		 * af mode.  flash is always off in night shot mode
		 */
		goto enable_af_low_light_mode;
	}

	light_level = s5k4ecgx_get_light_level(sd);

	switch (parms->flash_mode) {
	case FLASH_MODE_AUTO:
		if (light_level > LOW_LIGHT_LEVEL) {
			/* flash not needed */
			break;
		}
		/* fall through to turn on flash for AF assist */
	case FLASH_MODE_ON:
		s5k4ecgx_auto_focus_flash_start(sd);
		if (state->af_status == AF_CANCEL)
			return 0;
		break;
	case FLASH_MODE_OFF:
		break;
	default:
		dev_err(&client->dev,
			"%s: Unknown Flash mode 0x%x\n",
			__func__, parms->flash_mode);
		break;
	}

	if (light_level > LOW_LIGHT_LEVEL) {
		if (state->sensor_af_in_low_light_mode) {
			state->sensor_af_in_low_light_mode = false;
			s5k4ecgx_set_from_table(sd, "af low light mode off",
				&state->regs->af_low_light_mode_off, 1, 0);
		}
	} else {
enable_af_low_light_mode:
		if (!state->sensor_af_in_low_light_mode) {
			state->sensor_af_in_low_light_mode = true;
			s5k4ecgx_set_from_table(sd, "af low light mode on",
				&state->regs->af_low_light_mode_on, 1, 0);
		}
	}

	s5k4ecgx_set_from_table(sd, "single af start",
				&state->regs->single_af_start, 1, 0);
	state->af_status = AF_START;
	INIT_COMPLETION(state->af_complete);
	dev_dbg1(&client->dev, "%s: af_status set to start\n", __func__);

	return 0;
}

static int s5k4ecgx_stop_auto_focus(struct v4l2_subdev *sd)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);
	struct sec_cam_parm *parms =
		(struct sec_cam_parm *)&state->strm.parm.raw_data;
	int focus_mode = parms->focus_mode;

	dev_dbg1(&client->dev, "%s: single AF Off command Setting\n", __func__);

	/* always cancel ae_awb, in case AF already finished before
	 * we got called.
	 */
	/* restore write mode */
	s5k4ecgx_i2c_write_twobyte(client, 0x0028, 0x7000);

	s5k4ecgx_set_from_table(sd, "ae awb lock off",
				&state->regs->ae_awb_lock_off, 1, 0);

	if (state->af_status != AF_START) {
		/* we weren't in the middle auto focus operation, we're done */
		dev_dbg1(&client->dev,
			"%s: auto focus not in progress, done\n", __func__);

		if (focus_mode == FOCUS_MODE_MACRO) {
			/* for change focus mode forcely */
			parms->focus_mode = -1;
			s5k4ecgx_set_focus_mode(sd, FOCUS_MODE_MACRO);
		} else if (focus_mode == FOCUS_MODE_AUTO) {
			/* for change focus mode forcely */
			parms->focus_mode = -1;
			s5k4ecgx_set_focus_mode(sd, FOCUS_MODE_AUTO);
		}

		return 0;
	}

	/* auto focus was in progress.  the other thread
	 * is either in the middle of get_auto_focus_result()
	 * or will call it shortly.  set a flag to have
	 * it abort it's polling.  that thread will
	 * also do cleanup like restore focus position.
	 *
	 * it might be enough to just send sensor commands
	 * to abort auto focus and the other thread would get
	 * that state from it's polling calls, but I'm not sure.
	 */
	state->af_status = AF_CANCEL;
	dev_dbg1(&client->dev,
		"%s: sending Single_AF_Off commands to sensor\n", __func__);

	s5k4ecgx_set_from_table(sd, "single af off 1",
				&state->regs->single_af_off_1, 1, 0);

	msleep(state->one_frame_delay_ms);

	s5k4ecgx_set_from_table(sd, "single af off 2",
				&state->regs->single_af_off_2, 1, 0);

	/* wait until the other thread has completed
	 * aborting the auto focus and restored state
	 */
	dev_dbg1(&client->dev, "%s: wait AF cancel done start\n", __func__);
	mutex_unlock(&state->ctrl_lock);
	wait_for_completion(&state->af_complete);
	mutex_lock(&state->ctrl_lock);
	dev_dbg1(&client->dev, "%s: wait AF cancel done finished\n", __func__);

	return 0;
}

/* called by HAL after auto focus was started to get the result.
 * it might be aborted asynchronously by a call to set_auto_focus
 */
static int s5k4ecgx_get_auto_focus_result(struct v4l2_subdev *sd,
					struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);
	int err, count;
	u16 read_value;

	dev_dbg1(&client->dev, "%s: Check AF Result\n", __func__);

	if (state->af_status == AF_NONE) {
		dev_dbg1(&client->dev,
			"%s: auto focus never started, returning 0x2\n",
			__func__);
		ctrl->value = AUTO_FOCUS_CANCELLED;
		return 0;
	}

	/* must delay 2 frame times before checking result of 1st phase */
	mutex_unlock(&state->ctrl_lock);
	msleep(state->one_frame_delay_ms*2);
	mutex_lock(&state->ctrl_lock);

	/* lock AE and AWB after first AF search */
	s5k4ecgx_set_from_table(sd, "ae awb lock on",
				&state->regs->ae_awb_lock_on, 1, 0);

	dev_dbg1(&client->dev, "%s: 1st AF search\n", __func__);
	/* enter read mode */
	s5k4ecgx_i2c_write_twobyte(client, 0x002C, 0x7000);
	for (count = 0; count < FIRST_AF_SEARCH_COUNT; count++) {
		if (state->af_status == AF_CANCEL) {
			dev_dbg1(&client->dev,
				"%s: AF is cancelled while doing\n", __func__);
			ctrl->value = AUTO_FOCUS_CANCELLED;
			goto check_flash;
		}
		s5k4ecgx_set_from_table(sd, "get 1st af search status",
					&state->regs->get_1st_af_search_status,
					1, 0);
		s5k4ecgx_i2c_read_twobyte(client, 0x0F12, &read_value);
		dev_dbg1(&client->dev,
			"%s: 1st i2c_read --- read_value == 0x%x\n",
			__func__, read_value);

		/* check for success and failure cases.  0x1 is
		 * auto focus still in progress.  0x2 is success.
		 * 0x0,0x3,0x4,0x6,0x8 are all failures cases
		 */
		if (read_value != 0x01)
			break;
		mutex_unlock(&state->ctrl_lock);
		msleep(50);
		mutex_lock(&state->ctrl_lock);
	}

	if ((count >= FIRST_AF_SEARCH_COUNT) || (read_value != 0x02)) {
		dev_dbg1(&client->dev,
			"%s: 1st scan timed out or failed\n", __func__);
		ctrl->value = AUTO_FOCUS_FAILED;
		goto check_flash;
	}

	dev_dbg1(&client->dev, "%s: 2nd AF search\n", __func__);

	/* delay 1 frame time before checking for 2nd AF completion */
	mutex_unlock(&state->ctrl_lock);
	msleep(state->one_frame_delay_ms);
	mutex_lock(&state->ctrl_lock);

	/* this is the long portion of AF, can take a second or more.
	 * we poll and wakeup more frequently than 1 second mainly
	 * to see if a cancel was requested
	 */
	for (count = 0; count < SECOND_AF_SEARCH_COUNT; count++) {
		if (state->af_status == AF_CANCEL) {
			dev_dbg1(&client->dev,
				"%s: AF is cancelled while doing\n", __func__);
			ctrl->value = AUTO_FOCUS_CANCELLED;
			goto check_flash;
		}
		s5k4ecgx_set_from_table(sd, "get 2nd af search status",
					&state->regs->get_2nd_af_search_status,
					1, 0);
		s5k4ecgx_i2c_read_twobyte(client, 0x0F12, &read_value);
		dev_dbg1(&client->dev,
			"%s: 2nd i2c_read --- read_value == 0x%x\n",
			__func__, read_value);

		/* low byte is garbage.  done when high byte is 0x0 */
		if (!(read_value & 0xff00))
			break;

		mutex_unlock(&state->ctrl_lock);
		msleep(50);
		mutex_lock(&state->ctrl_lock);
	}

	if (count >= SECOND_AF_SEARCH_COUNT) {
		dev_dbg1(&client->dev, "%s: 2nd scan timed out\n", __func__);
		ctrl->value = AUTO_FOCUS_FAILED;
		goto check_flash;
	}

	dev_dbg1(&client->dev, "%s: AF is success\n", __func__);
	ctrl->value = AUTO_FOCUS_DONE;

check_flash:
	/* restore write mode */
	s5k4ecgx_i2c_write_twobyte(client, 0x0028, 0x7000);

	if (state->flash_on) {
		struct s5k4ecgx_platform_data *pd = client->dev.platform_data;
		s5k4ecgx_set_from_table(sd, "AF assist flash end",
				&state->regs->af_assist_flash_end, 1, 0);
		state->flash_on = false;
		pd->af_assist_onoff(0);
	}

	dev_dbg1(&client->dev, "%s: single AF finished\n", __func__);
	state->af_status = AF_NONE;
	complete(&state->af_complete);
	return err;
}

static void s5k4ecgx_init_parameters(struct v4l2_subdev *sd)
{
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);
	struct sec_cam_parm *parms =
		(struct sec_cam_parm *)&state->strm.parm.raw_data;

	state->strm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	parms->capture.capturemode = 0;
	parms->capture.timeperframe.numerator = 1;
	parms->capture.timeperframe.denominator = 30;
	parms->contrast = CONTRAST_DEFAULT;
	parms->effects = IMAGE_EFFECT_NONE;
	parms->brightness = EV_DEFAULT;
	parms->flash_mode = FLASH_MODE_AUTO;
	parms->focus_mode = FOCUS_MODE_AUTO;
	parms->iso = ISO_AUTO;
	parms->metering = METERING_CENTER;
	parms->saturation = SATURATION_DEFAULT;
	parms->scene_mode = SCENE_MODE_NONE;
	parms->sharpness = SHARPNESS_DEFAULT;
	parms->white_balance = WHITE_BALANCE_AUTO;
	parms->zoom = ZOOM_0;

	state->jpeg.enable = 0;
	state->jpeg.quality = 100;
	state->jpeg.main_offset = 0;
	state->jpeg.main_size = 0;
	state->jpeg.thumb_offset = 0;
	state->jpeg.thumb_size = 0;
	state->jpeg.postview_offset = 0;

	state->fw.major = 1;

	state->one_frame_delay_ms = NORMAL_MODE_MAX_ONE_FRAME_DELAY_MS;
}

static void s5k4ecgx_set_framesize(struct v4l2_subdev *sd,
				const struct s5k4ecgx_framesize *frmsize,
				int frmsize_count, bool exact_match);

#if 0
static int s5k4ecgx_s_fmt(struct v4l2_subdev *sd, struct v4l2_format *fmt)
{
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	printk("BILLA s5k4ecgx_s_fmt E  sd w=%d, h=%d, fmt w =%d, h =%d \n",state->pix.width,state->pix.height,fmt->fmt.pix.width,fmt->fmt.pix.height);

	dev_dbg1(&client->dev, "%s: pixelformat = 0x%x (%c%c%c%c),"
		" colorspace = 0x%x, width = %d, height = %d\n",
		__func__, fmt->fmt.pix.pixelformat,
		fmt->fmt.pix.pixelformat,
		fmt->fmt.pix.pixelformat >> 8,
		fmt->fmt.pix.pixelformat >> 16,
		fmt->fmt.pix.pixelformat >> 24,
		fmt->fmt.pix.colorspace,
		fmt->fmt.pix.width, fmt->fmt.pix.height);

	if (fmt->fmt.pix.pixelformat == V4L2_PIX_FMT_JPEG &&
		fmt->fmt.pix.colorspace != V4L2_COLORSPACE_JPEG) {
		dev_err(&client->dev,
			"%s: mismatch in pixelformat and colorspace\n",
			__func__);
			printk("BILLA s5k4ecgx_s_fmt fail 1\n");
		return -EINVAL;
	}

	state->pix.width = fmt->fmt.pix.width;
	state->pix.height = fmt->fmt.pix.height;
	state->pix.pixelformat = fmt->fmt.pix.pixelformat;

	if (fmt->fmt.pix.colorspace == V4L2_COLORSPACE_JPEG) {
		state->oprmode = S5K4ECGX_OPRMODE_IMAGE;
		printk("BILLA s5k4ecgx_s_fmt  S5K4ECGX_OPRMODE_IMAGE \n" );
		/*
		 * In case of image capture mode,
		 * if the given image resolution is not supported,
		 * use the next higher image resolution. */
		s5k4ecgx_set_framesize(sd, s5k4ecgx_capture_framesize_list,
				ARRAY_SIZE(s5k4ecgx_capture_framesize_list),
				false);

	} else {
	
		printk("BILLA s5k4ecgx_s_fmt S5K4ECGX_OPRMODE_VIDEO \n" );
		state->oprmode = S5K4ECGX_OPRMODE_VIDEO;
		/*
		 * In case of video mode,
		 * if the given video resolution is not matching, use
		 * the default rate (currently S5K4ECGX_PREVIEW_WVGA).
		 */
		s5k4ecgx_set_framesize(sd, s5k4ecgx_preview_framesize_list,
				ARRAY_SIZE(s5k4ecgx_preview_framesize_list),
				true);
	}
	printk("BILLA s5k4ecgx_s_fmt succ \n");

	state->jpeg.enable = state->pix.pixelformat == V4L2_PIX_FMT_JPEG;

	return 0;
}
#endif

static int s5k4ecgx_s_fmt(struct v4l2_subdev *sd, struct v4l2_mbus_framefmt *mf)
{
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);


	state->pix.width = mf->width;
	state->pix.height = mf->height;
#if 0
	state->pix.pixelformat = fmt->fmt.pix.pixelformat;

	if (fmt->fmt.pix.colorspace == V4L2_COLORSPACE_JPEG) {
		state->oprmode = S5K4ECGX_OPRMODE_IMAGE;
		printk("BILLA s5k4ecgx_s_fmt  S5K4ECGX_OPRMODE_IMAGE \n" );
		/*
		 * In case of image capture mode,
		 * if the given image resolution is not supported,
		 * use the next higher image resolution. */
		s5k4ecgx_set_framesize(sd, s5k4ecgx_capture_framesize_list,
				ARRAY_SIZE(s5k4ecgx_capture_framesize_list),
				false);

	} else 
#endif

	{
	
		printk("BILLA s5k4ecgx_s_fmt S5K4ECGX_OPRMODE_VIDEO : req =  %d X %d\n",mf->width, mf->height );
		state->oprmode = S5K4ECGX_OPRMODE_VIDEO;
		/*
		 * In case of video mode,
		 * if the given video resolution is not matching, use
		 * the default rate (currently S5K4ECGX_PREVIEW_WVGA).
		 */
		s5k4ecgx_set_framesize(sd, s5k4ecgx_preview_framesize_list,
				ARRAY_SIZE(s5k4ecgx_preview_framesize_list),
				true);

		mf->width = state->pix.width;
		mf->height = state->pix.height;

		printk("BILLA s5k4ecgx_s_fmt S5K4ECGX_OPRMODE_VIDEO : supported =  %d X %d\n",mf->width, mf->height );
	}
	printk("BILLA s5k4ecgx_s_fmt succ \n");

	state->jpeg.enable = state->pix.pixelformat == V4L2_PIX_FMT_JPEG;

	return 0;
}






static int s5k4ecgx_enum_framesizes(struct v4l2_subdev *sd,
				  struct v4l2_frmsizeenum *fsize)
{

	printk("BILLA s5k4ecgx_enum_framesizes E \n");


	if (fsize->index >= S5K4ECGX_PREVIEW_MAX)
		return -EINVAL;

		
	//struct s5k4ecgx_state *state =
	//	container_of(sd, struct s5k4ecgx_state, sd);


	/* The camera interface should read this value, this is the resolution
	 * at which the sensor would provide framedata to the camera i/f
	 *
	 * In case of image capture,
	 * this returns the default camera resolution (SVGA)
	 */
	fsize->type = V4L2_FRMSIZE_TYPE_DISCRETE;

	fsize->discrete.width = s5k4ecgx_preview_framesize_list[fsize->index].width;
	fsize->discrete.height = s5k4ecgx_preview_framesize_list[fsize->index].height;

	fsize->pixel_format = DEFAULT_PIX_FMT; 

	printk("BILLA s5k4ecgx_enum_framesizes index = %d, w =%d ,h =%d, \n",fsize->index,fsize->discrete.width,fsize->discrete.height );
	return 0;
}


static int s5k4ecgx_enum_fmt(struct v4l2_subdev *sd, unsigned int index,
			    enum v4l2_mbus_pixelcode *code)
{
	if (index >= ARRAY_SIZE(capture_fmts))
		return -EINVAL;

	*code = DEFAULT_MBUS_PIX_FMT;//capture_fmts[index].pixelformat;
	printk("BILLA s5k4ecgx_enum_fmt = %x \n",*code);
	return 0;
}

#if 0
static int s5k4ecgx_enum_fmt(struct v4l2_subdev *sd,
			struct v4l2_fmtdesc *fmtdesc)
{
	if(fmtdesc == NULL)
	{
		printk("BILLA s5k4ecgx_enum_fmt :v4l2_fmtdesc = NULL \n");
		return 0;
	}

	printk("BILLA s5k4ecgx_enum_fmt :after NULL check !!!\n");
	pr_debug("%s: index = %d\n", __func__, fmtdesc->index);
	printk("BILLA s5k4ecgx_enum_fmt :after NULL check 22 !!!\n");
	

	if (fmtdesc->index >= ARRAY_SIZE(capture_fmts))
	{
		printk("BILLA s5k4ecgx_enum_fmt FAIL %d %d  \n",fmtdesc->index ,ARRAY_SIZE(capture_fmts));
		return -EINVAL;
	}

	printk("BILLA s5k4ecgx_enum_fmt  %d %d  \n",fmtdesc->index ,ARRAY_SIZE(capture_fmts));


	memcpy(fmtdesc, &capture_fmts[fmtdesc->index], sizeof(fmtdesc));

	
	printk("BILLA s5k4ecgx_enum_fmt  succ \n");

	return 0;
}
#endif


static int s5k4ecgx_try_fmt(struct v4l2_subdev *sd,
			   struct v4l2_mbus_framefmt *mf)
{
	int i_fmt;
	int i_size;

	printk("BILLA s5k4ecgx_try_fmt w =%d , h =%d E \n",mf->width,mf->height);

	mf->code = DEFAULT_MBUS_PIX_FMT;
	mf->colorspace	= V4L2_COLORSPACE_JPEG;//ov5640_fmts[i_fmt].colorspace;
	mf->field	= V4L2_FIELD_NONE;

	
	//mf->width = s5k4ecgx_frmsizes[i_size].width;
	//mf->height = s5k4ecgx_frmsizes[i_size].height;

	return 0;
}

#if 0
static int s5k4ecgx_try_fmt(struct v4l2_subdev *sd, struct v4l2_format *fmt)
{
	int num_entries;
	int i;
	printk("BILLA s5k4ecgx_try_fmt E \n");
	num_entries = ARRAY_SIZE(capture_fmts);

	pr_debug("%s: pixelformat = 0x%x (%c%c%c%c), num_entries = %d\n",
		__func__, fmt->fmt.pix.pixelformat,
		fmt->fmt.pix.pixelformat,
		fmt->fmt.pix.pixelformat >> 8,
		fmt->fmt.pix.pixelformat >> 16,
		fmt->fmt.pix.pixelformat >> 24,
		num_entries);

	for (i = 0; i < num_entries; i++) {
		if (capture_fmts[i].pixelformat == fmt->fmt.pix.pixelformat) {
			pr_debug("%s: match found, returning 0\n", __func__);
			return 0;
		}
		else
			printk("BILLA s5k4ecgx_try_fmt failed %d %d %d \n",num_entries,capture_fmts[i].pixelformat,fmt->fmt.pix.pixelformat);
	}

	pr_debug("%s: no match found, returning -EINVAL\n", __func__);
	return -EINVAL;
}
#endif
static void s5k4ecgx_enable_torch(struct v4l2_subdev *sd)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);
	struct s5k4ecgx_platform_data *pdata = client->dev.platform_data;

	s5k4ecgx_set_from_table(sd, "flash start",
				&state->regs->flash_start, 1, 0);
	state->flash_on = true;
	pdata->torch_onoff(1);
}

static void s5k4ecgx_disable_torch(struct v4l2_subdev *sd)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);
	struct s5k4ecgx_platform_data *pdata = client->dev.platform_data;

	if (state->flash_on) {
		state->flash_on = false;
		pdata->torch_onoff(0);
		s5k4ecgx_set_from_table(sd, "flash end",
					&state->regs->flash_end, 1, 0);
	}
}
static int s5k4ecgx_set_flash_mode(struct v4l2_subdev *sd, int value)
{
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);
	struct sec_cam_parm *parms =
		(struct sec_cam_parm *)&state->strm.parm.raw_data;

	if (parms->flash_mode == value)
		return 0;

	if ((value >= FLASH_MODE_OFF) && (value <= FLASH_MODE_TORCH)) {
		pr_debug("%s: setting flash mode to %d\n",
			__func__, value);
		parms->flash_mode = value;
		if (parms->flash_mode == FLASH_MODE_TORCH)
			s5k4ecgx_enable_torch(sd);
		else
			s5k4ecgx_disable_torch(sd);
		return 0;
	}
	pr_debug("%s: trying to set invalid flash mode %d\n",
		__func__, value);
	return -EINVAL;
}
static int s5k4ecgx_g_parm(struct v4l2_subdev *sd,
			struct v4l2_streamparm *param)
{
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);
	printk("s5k4ecgx_g_parm \n");

	memcpy(param, &state->strm, sizeof(param));
	return 0;
}
static int s5k4ecgx_s_parm(struct v4l2_subdev *sd,
			struct v4l2_streamparm *param)
{
	int err = 0;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);
	struct sec_cam_parm *new_parms =
		(struct sec_cam_parm *)&param->parm.raw_data;
	struct sec_cam_parm *parms =
		(struct sec_cam_parm *)&state->strm.parm.raw_data;
	printk("s5k4ecgx_s_parm\n");

	dev_dbg1(&client->dev, "%s: start\n", __func__);

	if (param->parm.capture.timeperframe.numerator !=
		parms->capture.timeperframe.numerator ||
		param->parm.capture.timeperframe.denominator !=
		parms->capture.timeperframe.denominator) {

		int fps = 0;
		int fps_max = 30;

		if (param->parm.capture.timeperframe.numerator &&
			param->parm.capture.timeperframe.denominator)
			fps =
			    (int)(param->parm.capture.timeperframe.denominator /
				  param->parm.capture.timeperframe.numerator);
		else
			fps = 0;

		if (fps <= 0 || fps > fps_max) {
			dev_err(&client->dev,
				"%s: Framerate %d not supported,"
				" setting it to %d fps.\n",
				__func__, fps, fps_max);
			fps = fps_max;
		}

		/*
		 * Don't set the fps value, just update it in the state
		 * We will set the resolution and
		 * fps in the start operation (preview/capture) call
		 */
		parms->capture.timeperframe.numerator = 1;
		parms->capture.timeperframe.denominator = fps;
	}

	/* we return an error if one happened but don't stop trying to
	 * set all parameters passed
	 */
	err = s5k4ecgx_set_parameter(sd, &parms->contrast, new_parms->contrast,
				"contrast", state->regs->contrast,
				ARRAY_SIZE(state->regs->contrast));
	err |= s5k4ecgx_set_parameter(sd, &parms->effects, new_parms->effects,
				"effect", state->regs->effect,
				ARRAY_SIZE(state->regs->effect));
	err |= s5k4ecgx_set_parameter(sd, &parms->brightness,
				new_parms->brightness, "brightness",
				state->regs->ev, ARRAY_SIZE(state->regs->ev));
	err |= s5k4ecgx_set_flash_mode(sd, new_parms->flash_mode);
	err |= s5k4ecgx_set_focus_mode(sd, new_parms->focus_mode);
	err |= s5k4ecgx_set_parameter(sd, &parms->iso, new_parms->iso,
				"iso", state->regs->iso,
				ARRAY_SIZE(state->regs->iso));
	err |= s5k4ecgx_set_parameter(sd, &parms->metering, new_parms->metering,
				"metering", state->regs->metering,
				ARRAY_SIZE(state->regs->metering));
	err |= s5k4ecgx_set_parameter(sd, &parms->saturation,
				new_parms->saturation, "saturation",
				state->regs->saturation,
				ARRAY_SIZE(state->regs->saturation));
	err |= s5k4ecgx_set_parameter(sd, &parms->scene_mode,
				new_parms->scene_mode, "scene_mode",
				state->regs->scene_mode,
				ARRAY_SIZE(state->regs->scene_mode));
	err |= s5k4ecgx_set_parameter(sd, &parms->sharpness,
				new_parms->sharpness, "sharpness",
				state->regs->sharpness,
				ARRAY_SIZE(state->regs->sharpness));
	err |= s5k4ecgx_set_parameter(sd, &parms->white_balance,
				new_parms->white_balance, "white balance",
				state->regs->white_balance,
				ARRAY_SIZE(state->regs->white_balance));
	err |= s5k4ecgx_set_parameter(sd, &parms->fps,
				new_parms->fps, "fps",
				state->regs->fps,
				ARRAY_SIZE(state->regs->fps));

	if (parms->scene_mode == SCENE_MODE_NIGHTSHOT)
		state->one_frame_delay_ms = NIGHT_MODE_MAX_ONE_FRAME_DELAY_MS;
	else
		state->one_frame_delay_ms = NORMAL_MODE_MAX_ONE_FRAME_DELAY_MS;

	dev_dbg1(&client->dev, "%s: returning %d\n", __func__, err);
	return err;
}
/* This function is called from the g_ctrl api
 *
 * This function should be called only after the s_fmt call,
 * which sets the required width/height value.
 *
 * It checks a list of available frame sizes and sets the
 * most appropriate frame size.
 *
 * The list is stored in an increasing order (as far as possible).
 * Hence the first entry (searching from the beginning) where both the
 * width and height is more than the required value is returned.
 * In case of no perfect match, we set the last entry (which is supposed
 * to be the largest resolution supported.)
 */
static void s5k4ecgx_set_framesize(struct v4l2_subdev *sd,
				const struct s5k4ecgx_framesize *frmsize,
				int frmsize_count, bool preview)
{
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	const struct s5k4ecgx_framesize *last_frmsize =
		&frmsize[frmsize_count - 1];

	dev_dbg1(&client->dev, "%s: Requested Res: %dx%d\n", __func__,
		state->pix.width, state->pix.height);

	do {
		/*
		 * In case of image capture mode,
		 * if the given image resolution is not supported,
		 * return the next higher image resolution. */

		if (preview) {
			if (frmsize->width == state->pix.width &&
				frmsize->height == state->pix.height) {
				break;
			}
		} else


		{
			dev_dbg1(&client->dev,
				"%s: compare frmsize %dx%d to %dx%d\n",
				__func__,
				frmsize->width, frmsize->height,
				state->pix.width, state->pix.height);
			if (frmsize->width >= state->pix.width &&
				frmsize->height >= state->pix.height) {
				dev_dbg1(&client->dev,
					"%s: select frmsize %dx%d, index=%d\n",
					__func__,
					frmsize->width, frmsize->height,
					frmsize->index);
				break;
			}
		}

		frmsize++;
	} while (frmsize <= last_frmsize);

	if (frmsize > last_frmsize)
		frmsize = last_frmsize;

	
	state->pix.width = frmsize->width;
	state->pix.height = frmsize->height;
	if (preview) {
		state->preview_framesize_index = frmsize->index;
		dev_dbg1(&client->dev, "%s: Preview Res Set: %dx%d, index %d\n",
			__func__, state->pix.width, state->pix.height,
			state->preview_framesize_index);
	} else {
		state->capture_framesize_index = frmsize->index;
		dev_dbg1(&client->dev, "%s: Capture Res Set: %dx%d, index %d\n",
			__func__, state->pix.width, state->pix.height,
			state->capture_framesize_index);
	}


//	state->preview_framesize_index = 1;



}

static int s5k4ecgx_check_dataline_stop(struct v4l2_subdev *sd)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);
	int err;

	dev_dbg1(&client->dev, "%s\n", __func__);

	err = s5k4ecgx_set_from_table(sd, "DTP stop",
				&state->regs->dtp_stop, 1, 0);
	if (err < 0) {
		v4l_info(client, "%s: register set failed\n", __func__);
		return -EIO;
	}

	state->check_dataline = 0;

	return err;
}

static void s5k4ecgx_get_esd_int(struct v4l2_subdev *sd,
				struct v4l2_control *ctrl)
{
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	u16 read_value;
	int err;

	if ((S5K4ECGX_RUNMODE_RUNNING == state->runmode) &&
		(state->af_status != AF_START)) {
		err = s5k4ecgx_set_from_table(sd, "get esd status",
					&state->regs->get_esd_status,
					1, 0);
		err |= s5k4ecgx_i2c_read_twobyte(client, 0x0F12, &read_value);
		dev_dbg1(&client->dev,
			"%s: read_value == 0x%x\n", __func__, read_value);
		/* return to write mode */
		err |= s5k4ecgx_i2c_write_twobyte(client, 0x0028, 0x7000);

		if (err < 0) {
			v4l_info(client,
				"Failed I2C for getting ESD information\n");
			ctrl->value = 0x01;
		} else {
			if (read_value != 0x0000) {
				v4l_info(client, "ESD interrupt happened!!\n");
				ctrl->value = 0x01;
			} else {
				dev_dbg1(&client->dev,
					"%s: No ESD interrupt!!\n", __func__);
				ctrl->value = 0x00;
			}
		}
	} else
		ctrl->value = 0x00;
}

/* returns the real iso currently used by sensor due to lighting
 * conditions, not the requested iso we sent using s_ctrl.
 */
static int s5k4ecgx_get_iso(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	int err;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);
	u16 read_value1 = 0;
	u16 read_value2 = 0;
	int read_value;

	err = s5k4ecgx_set_from_table(sd, "get iso",
				&state->regs->get_iso, 1, 0);
	err |= s5k4ecgx_i2c_read_twobyte(client, 0x0F12, &read_value1);
	err |= s5k4ecgx_i2c_read_twobyte(client, 0x0F12, &read_value2);

	/* restore write mode */
	s5k4ecgx_i2c_write_twobyte(client, 0x0028, 0x7000);

	read_value = read_value1 * read_value2 / 0x100 / 2;

	if (read_value > 0x400)
		ctrl->value = ISO_400;
	else if (read_value > 0x200)
		ctrl->value = ISO_200;
	else if (read_value > 0x100)
		ctrl->value = ISO_100;
	else
		ctrl->value = ISO_50;

	dev_dbg1(&client->dev, "%s: get iso == %d (0x%x, 0x%x)\n",
		__func__, ctrl->value, read_value1, read_value2);

	return err;
}

static int s5k4ecgx_get_shutterspeed(struct v4l2_subdev *sd,
	struct v4l2_control *ctrl)
{
	int err;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);
	u16 read_value = 0;

	err = s5k4ecgx_set_from_table(sd, "get shutterspeed",
				&state->regs->get_shutterspeed, 1, 0);
	err |= s5k4ecgx_i2c_read_twobyte(client, 0x0F12, &read_value);

	/* restore write mode */
	s5k4ecgx_i2c_write_twobyte(client, 0x0028, 0x7000);

	ctrl->value = read_value / 400;
	dev_dbg1(&client->dev,
			"%s: get shutterspeed == %d\n", __func__, ctrl->value);

	return err;
}
static int s5k4ecgx_set_bus_param(struct soc_camera_device *icd,
				 unsigned long flags)
{
	/* TODO: Do the right thing here, and validate bus params */
	return 0;
}

static unsigned long s5k4ecgx_query_bus_param(struct soc_camera_device *icd)
{
	unsigned long flags = SOCAM_PCLK_SAMPLE_FALLING |
		SOCAM_HSYNC_ACTIVE_HIGH | SOCAM_VSYNC_ACTIVE_HIGH |
		SOCAM_DATA_ACTIVE_HIGH | SOCAM_MASTER;

	/* TODO: Do the right thing here, and validate bus params */

	flags |= SOCAM_DATAWIDTH_10;

	return flags;
}


static int s5k4ecgx_g_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);
	struct sec_cam_parm *parms =
		(struct sec_cam_parm *)&state->strm.parm.raw_data;
	int err = 0;

	if (!state->initialized) {
		dev_err(&client->dev,
			"%s: return error because uninitialized\n", __func__);
		return -ENODEV;
	}

	mutex_lock(&state->ctrl_lock);

	switch (ctrl->id) {
	case V4L2_CID_WHITE_BALANCE:
		ctrl->value = parms->white_balance;
		break;
	case V4L2_CID_EFFECT:
		ctrl->value = parms->effects;
		break;
	case V4L2_CID_CONTRAST:
		ctrl->value = parms->contrast;
		break;
	case V4L2_CID_SATURATION:
		ctrl->value = parms->saturation;
		break;
	case V4L2_CID_SHARPNESS:
		ctrl->value = parms->sharpness;
		break;
	case V4L2_CID_JPEG_MAIN_SIZE:
		ctrl->value = state->jpeg.main_size;
		break;
	case V4L2_CID_JPEG_MAIN_OFFSET:
		ctrl->value = state->jpeg.main_offset;
		break;
	case V4L2_CID_JPEG_THUMB_SIZE:
		ctrl->value = state->jpeg.thumb_size;
		break;
	case V4L2_CID_JPEG_THUMB_OFFSET:
		ctrl->value = state->jpeg.thumb_offset;
		break;
	case V4L2_CID_JPEG_POSTVIEW_OFFSET:
		ctrl->value = state->jpeg.postview_offset;
		break;
	case V4L2_CID_JPEG_MEMSIZE:
		ctrl->value = SENSOR_JPEG_SNAPSHOT_MEMSIZE;
		break;
	case V4L2_CID_JPEG_QUALITY:
		ctrl->value = state->jpeg.quality;
		break;
	case V4L2_CID_AUTO_FOCUS_RESULT:
		err = s5k4ecgx_get_auto_focus_result(sd, ctrl);
		break;
	case V4L2_CID_DATE_INFO_YEAR:
		ctrl->value = 2010;
		break;
	case V4L2_CID_DATE_INFO_MONTH:
		ctrl->value = 2;
		break;
	case V4L2_CID_DATE_INFO_DATE:
		ctrl->value = 25;
		break;
	case V4L2_CID_SENSOR_VER:
		ctrl->value = 1;
		break;
	case V4L2_CID_FW_MINOR_VER:
		ctrl->value = state->fw.minor;
		break;
	case V4L2_CID_FW_MAJOR_VER:
		ctrl->value = state->fw.major;
		break;
	case V4L2_CID_PRM_MINOR_VER:
		ctrl->value = state->prm.minor;
		break;
	case V4L2_CID_PRM_MAJOR_VER:
		ctrl->value = state->prm.major;
		break;
	case V4L2_CID_ESD_INT:
		s5k4ecgx_get_esd_int(sd, ctrl);
		break;
	case V4L2_CID_GET_ISO:
		err = s5k4ecgx_get_iso(sd, ctrl);
		break;
	case V4L2_CID_GET_SHT_TIME:
		err = s5k4ecgx_get_shutterspeed(sd, ctrl);
		break;
	case V4L2_CID_GET_FLASH_ONOFF:
		ctrl->value = state->flash_state_on_previous_capture;
		break;
	case V4L2_CID_OBJ_TRACKING_STATUS:
	case V4L2_CID_SMART_AUTO_STATUS:
		break;
	default:
		err = -ENOIOCTLCMD;
		dev_err(&client->dev, "%s: unknown ctrl id 0x%x\n",
			__func__, ctrl->id);
		break;
	}

	mutex_unlock(&state->ctrl_lock);

	return err;
}


static int s5k4ecgx_s_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);
	struct sec_cam_parm *parms =
		(struct sec_cam_parm *)&state->strm.parm.raw_data;
	int err = 0;
	int value = ctrl->value;

	if (!state->initialized &&
		(ctrl->id != V4L2_CID_CHECK_DATALINE)) {
		dev_err(&client->dev,
			"%s: return error because uninitialized\n", __func__);
		return -ENODEV;
	}

	dev_dbg1(&client->dev, "%s: V4l2 control ID =%d, val = %d\n",
		__func__, ctrl->id - V4L2_CID_PRIVATE_BASE, value);

	mutex_lock(&state->ctrl_lock);

	switch (ctrl->id) {
	case V4L2_CID_FLASH_MODE:
		err = s5k4ecgx_set_flash_mode(sd, value);
		break;
	case V4L2_CID_BRIGHTNESS:
		if (state->runmode == S5K4ECGX_RUNMODE_RUNNING) {
			err = s5k4ecgx_set_parameter(sd, &parms->brightness,
						value, "brightness",
						state->regs->ev,
						ARRAY_SIZE(state->regs->ev));
		} else {
			dev_err(&client->dev,
				"%s: trying to set brightness when not "
				"in preview mode\n",
				__func__);
			err = -EINVAL;
		}
		break;
	case V4L2_CID_WHITE_BALANCE:
		if (state->runmode == S5K4ECGX_RUNMODE_RUNNING) {
			err = s5k4ecgx_set_parameter(sd, &parms->white_balance,
					value, "white balance",
					state->regs->white_balance,
					ARRAY_SIZE(state->regs->white_balance));
		} else {
			dev_err(&client->dev,
				"%s: trying to set white balance when not "
				"in preview mode\n",
				__func__);
			err = -EINVAL;
		}
		break;

	case V4L2_CID_ZOOM_ABSOLUTE:

		if (state->runmode == S5K4ECGX_RUNMODE_RUNNING) {
				err = s5k4ecgx_set_parameter(sd, &parms->zoom,
				value, "zoom",
				state->regs->zoom,
				ARRAY_SIZE(state->regs->zoom));
		} else {
			dev_err(&client->dev,
				"%s: trying to set ZOOM when not "
				"in preview mode\n",
				__func__);
			err = -EINVAL;
		}

		
	case V4L2_CID_EFFECT:
		if (state->runmode == S5K4ECGX_RUNMODE_RUNNING) {
			err = s5k4ecgx_set_parameter(sd, &parms->effects,
					value, "effects", state->regs->effect,
					ARRAY_SIZE(state->regs->effect));
		} else {
			dev_err(&client->dev,
				"%s: trying to set effect when not "
				"in preview mode\n",
				__func__);
			err = -EINVAL;
		}
		break;
	case V4L2_CID_ISO:
		if (state->runmode == S5K4ECGX_RUNMODE_RUNNING) {
			err = s5k4ecgx_set_parameter(sd, &parms->iso,
						value, "iso",
						state->regs->iso,
						ARRAY_SIZE(state->regs->iso));
		} else {
			dev_err(&client->dev,
				"%s: trying to set iso when not "
				"in preview mode\n",
				__func__);
			err = -EINVAL;
		}
		break;
	case V4L2_CID_METERING:
		if (state->runmode == S5K4ECGX_RUNMODE_RUNNING) {
			err = s5k4ecgx_set_parameter(sd, &parms->metering,
					value, "metering",
					state->regs->metering,
					ARRAY_SIZE(state->regs->metering));
		} else {
			dev_err(&client->dev,
				"%s: trying to set metering when not "
				"in preview mode\n",
				__func__);
			err = -EINVAL;
		}
		break;
	case V4L2_CID_CONTRAST:
		err = s5k4ecgx_set_parameter(sd, &parms->contrast,
					value, "contrast",
					state->regs->contrast,
					ARRAY_SIZE(state->regs->contrast));
		break;
	case V4L2_CID_SATURATION:
		err = s5k4ecgx_set_parameter(sd, &parms->saturation,
					value, "saturation",
					state->regs->saturation,
					ARRAY_SIZE(state->regs->saturation));
		break;
	case V4L2_CID_SHARPNESS:
		err = s5k4ecgx_set_parameter(sd, &parms->sharpness,
					value, "sharpness",
					state->regs->sharpness,
					ARRAY_SIZE(state->regs->sharpness));
		break;
	case V4L2_CID_WDR:
		err = s5k4ecgx_set_wdr(sd, value);
		break;
	case V4L2_CID_FACE_DETECTION:
		err = s5k4ecgx_set_face_detection(sd, value);
		break;
	case V4L2_CID_FOCUS_MODE:
		err = s5k4ecgx_set_focus_mode(sd, value);
		break;
	case V4L2_CID_JPEG_QUALITY:
		if (state->runmode == S5K4ECGX_RUNMODE_RUNNING) {
			state->jpeg.quality = value;
			err = s5k4ecgx_set_jpeg_quality(sd);
		} else {
			dev_err(&client->dev,
				"%s: trying to set jpeg quality when not "
				"in preview mode\n",
				__func__);
			err = -EINVAL;
		}
		break;
	case V4L2_CID_SCENE_MODE:
		err = s5k4ecgx_set_parameter(sd, &parms->scene_mode,
					SCENE_MODE_NONE, "scene_mode",
					state->regs->scene_mode,
					ARRAY_SIZE(state->regs->scene_mode));
		if (err < 0) {
			dev_err(&client->dev,
				"%s: failed to set scene-mode default value\n",
				__func__);
			break;
		}
		if (value != SCENE_MODE_NONE) {
			err = s5k4ecgx_set_parameter(sd, &parms->scene_mode,
					value, "scene_mode",
					state->regs->scene_mode,
					ARRAY_SIZE(state->regs->scene_mode));
		}
		if (parms->scene_mode == SCENE_MODE_NIGHTSHOT) {
			state->one_frame_delay_ms =
				NIGHT_MODE_MAX_ONE_FRAME_DELAY_MS;
		} else {
			state->one_frame_delay_ms =
				NORMAL_MODE_MAX_ONE_FRAME_DELAY_MS;
		}

		break;
	case V4L2_CID_GPS_LATITUDE:
		dev_err(&client->dev,
			"%s: V4L2_CID_GPS_LATITUDE: not implemented\n",
			__func__);
		break;
	case V4L2_CID_GPS_LONGITUDE:
		dev_err(&client->dev,
			"%s: V4L2_CID_GPS_LONGITUDE: not implemented\n",
			__func__);
		break;
	case V4L2_CID_GPS_TIMESTAMP:
		dev_err(&client->dev,
			"%s: V4L2_CID_GPS_TIMESTAMP: not implemented\n",
			__func__);
		break;
	case V4L2_CID_GPS_ALTITUDE:
		dev_err(&client->dev,
			"%s: V4L2_CID_GPS_ALTITUDE: not implemented\n",
			__func__);
		break;
	case V4L2_CID_OBJECT_POSITION_X:
		state->position.x = value;
		break;
	case V4L2_CID_OBJECT_POSITION_Y:
		state->position.y = value;
		break;
	case V4L2_CID_SET_AUTO_FOCUS:
		if (value == AUTO_FOCUS_ON)
			err = s5k4ecgx_start_auto_focus(sd);
		else if (value == AUTO_FOCUS_OFF)
			err = s5k4ecgx_stop_auto_focus(sd);
		else {
			err = -EINVAL;
			dev_err(&client->dev,
				"%s: bad focus value requestion %d\n",
				__func__, value);
		}
		break;
	case V4L2_CID_FRAME_RATE:
		dev_dbg1(&client->dev,
			"%s: camera frame rate request for %d fps\n",
			__func__, value);
		err = s5k4ecgx_set_parameter(sd, &parms->fps,
					value, "fps",
					state->regs->fps,
					ARRAY_SIZE(state->regs->fps));
		break;
	case V4L2_CID_CAPTURE:
		err = s5k4ecgx_start_capture(sd);
		break;

	/* Used to start / stop preview operation.
	 * This call can be modified to START/STOP operation,
	 * which can be used in image capture also
	 */
	case V4L2_CID_PREVIEW_ONOFF:
		if (value)
			err = s5k4ecgx_set_preview_start(sd);
		else
			err = s5k4ecgx_set_preview_stop(sd);
		break;
	case V4L2_CID_CHECK_DATALINE:
		dev_dbg1(&client->dev, "%s: check_dataline set to %d\n",
			__func__, value);
		state->check_dataline = value;
		break;
	case V4L2_CID_CHECK_DATALINE_STOP:
		err = s5k4ecgx_check_dataline_stop(sd);
		break;
	case V4L2_CID_RETURN_FOCUS:
		if (parms->focus_mode != FOCUS_MODE_MACRO)
			err = s5k4ecgx_return_focus(sd);
		break;
	default:
		dev_err(&client->dev, "%s: unknown set ctrl id 0x%x\n",
			__func__, ctrl->id);
		err = -ENOIOCTLCMD;
		break;
	}

	if (err < 0)
		dev_err(&client->dev, "%s: videoc_s_ctrl failed %d\n", __func__,
			err);

	mutex_unlock(&state->ctrl_lock);

	dev_dbg1(&client->dev, "%s: videoc_s_ctrl returning %d\n",
		__func__, err);

	return err;
}

#ifdef CONFIG_VIDEO_ADV_DEBUG
static int s5k4ecgx_g_register(struct v4l2_subdev *sd,
			     struct v4l2_dbg_register *reg)
{
	printk("Error!!! s5k4ecgx_g_register is empty\n");

	return 0;
}

static int s5k4ecgx_s_register(struct v4l2_subdev *sd,
			     struct v4l2_dbg_register *reg)
{
	printk("Error!!! s5k4ecgx_s_register is empty\n");

	return 0;
}
#endif


static int s5k4ecgx_s_ext_ctrl(struct v4l2_subdev *sd,
			      struct v4l2_ext_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);
	int err = 0;
	struct gps_info_common *tempGPSType = NULL;

	switch (ctrl->id) {

	case V4L2_CID_GPS_LATITUDE:
		tempGPSType = (struct gps_info_common *)ctrl->reserved2[1];
		state->gps_info.gps_buf[0] = tempGPSType->direction;
		state->gps_info.gps_buf[1] = tempGPSType->dgree;
		state->gps_info.gps_buf[2] = tempGPSType->minute;
		state->gps_info.gps_buf[3] = tempGPSType->second;
		break;
	case V4L2_CID_GPS_LONGITUDE:
		tempGPSType = (struct gps_info_common *)ctrl->reserved2[1];
		state->gps_info.gps_buf[4] = tempGPSType->direction;
		state->gps_info.gps_buf[5] = tempGPSType->dgree;
		state->gps_info.gps_buf[6] = tempGPSType->minute;
		state->gps_info.gps_buf[7] = tempGPSType->second;
		break;
	case V4L2_CID_GPS_ALTITUDE:
		tempGPSType = (struct gps_info_common *)ctrl->reserved2[1];
		state->gps_info.altitude_buf[0] = tempGPSType->direction;
		state->gps_info.altitude_buf[1] =
					(tempGPSType->dgree) & 0x00ff;
		state->gps_info.altitude_buf[2] =
					((tempGPSType->dgree) & 0xff00) >> 8;
		state->gps_info.altitude_buf[3] = tempGPSType->minute;
		break;
	case V4L2_CID_GPS_TIMESTAMP:
		state->gps_info.gps_timeStamp = *((int *)ctrl->reserved2[1]);
		err = 0;
		break;
	default:
		dev_err(&client->dev, "%s: unknown ctrl->id %d\n",
			__func__, ctrl->id);
		err = -ENOIOCTLCMD;
		break;
	}

	if (err < 0)
		dev_err(&client->dev, "%s: vidioc_s_ext_ctrl failed %d\n",
			__func__, err);

	return err;
}

static int s5k4ecgx_s_ext_ctrls(struct v4l2_subdev *sd,
				struct v4l2_ext_controls *ctrls)
{
	struct v4l2_ext_control *ctrl = ctrls->controls;
	int ret;
	int i;

	for (i = 0; i < ctrls->count; i++, ctrl++) {
		ret = s5k4ecgx_s_ext_ctrl(sd, ctrl);

		if (ret) {
			ctrls->error_idx = i;
			break;
		}
	}

	return ret;
}

#ifdef CONFIG_VIDEO_S5K4ECGX_DEBUG
static void s5k4ecgx_dump_regset(struct s5k4ecgx_regset *regset)
{
	if ((regset->data[0] == 0x00) && (regset->data[1] == 0x2A)) {
		if (regset->size <= 6)
			pr_err("odd regset size %d\n", regset->size);
		pr_info("regset: addr = 0x%02X%02X, data[0,1] = 0x%02X%02X,"
			" total data size = %d\n",
			regset->data[2], regset->data[3],
			regset->data[6], regset->data[7],
			regset->size-6);
	} else {
		pr_info("regset: 0x%02X%02X%02X%02X\n",
			regset->data[0], regset->data[1],
			regset->data[2], regset->data[3]);
		if (regset->size != 4)
			pr_err("odd regset size %d\n", regset->size);
	}
}
#endif

static int s5k4ecgx_i2c_write_block(struct v4l2_subdev *sd, u8 *buf, int size)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int retry_count = 5;
	int ret;
	struct i2c_msg msg = {client->addr, 0, size, buf};

#ifdef CONFIG_VIDEO_S5K4ECGX_DEBUG
	if (s5k4ecgx_debug_mask & S5K4ECGX_DEBUG_I2C_BURSTS) {
		if ((buf[0] == 0x0F) && (buf[1] == 0x12))
			pr_info("%s : data[0,1] = 0x%02X%02X,"
				" total data size = %d\n",
				__func__, buf[2], buf[3], size-2);
		else
			pr_info("%s : 0x%02X%02X%02X%02X\n",
				__func__, buf[0], buf[1], buf[2], buf[3]);
	}
#endif

	do {
		ret = i2c_transfer(client->adapter, &msg, 1);
		if (likely(ret == 1))
			break;
		msleep(POLL_TIME_MS);
	} while (retry_count-- > 0);
	if (ret != 1) {
		dev_err(&client->dev, "%s: I2C is not working.\n", __func__);
		return -EIO;
	}

#ifdef CONFIG_VIDEO_S5K4ECGX_V_1_0
	{
		struct s5k4ecgx_state *state =
			container_of(sd, struct s5k4ecgx_state, sd);
		if (state->fw.minor == 0) {
			/* v1.0 sensor have problems sometimes if we write
			 * too much data too fast, so add a sleep.  I've
			 * tried various combinations of size/delay.  Checking
			 * for a larger size doesn't seem to work reliably, and
			 * a delay of 1ms sometimes isn't enough either.
			 */
			if (size > 16)
				msleep(2);
		}
	}
#endif
	return 0;
}

/*
 * Parse the init_reg2 array into a number of register sets that
 * we can send over as i2c burst writes instead of writing each
 * entry of init_reg2 as a single 4 byte write.  Write the
 * new data structures and then free them.
 */
static int s5k4ecgx_write_init_reg2_burst(struct v4l2_subdev *sd)
{
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);
	struct s5k4ecgx_regset *regset_table;
	struct s5k4ecgx_regset *regset;
	struct s5k4ecgx_regset *end_regset;
	u8 *regset_data;
	u8 *dst_ptr;
	const u32 *end_src_ptr;
	bool flag_copied;
	int init_reg_2_array_size = state->regs->init_reg_2.array_size;
	int init_reg_2_size = init_reg_2_array_size * sizeof(u32);
	const u32 *src_ptr = state->regs->init_reg_2.reg;
	u32 src_value;
	int err;

	pr_debug("%s : start\n", __func__);

	regset_data = vmalloc(init_reg_2_size);
	if (regset_data == NULL)
		return -ENOMEM;
	regset_table = vmalloc(sizeof(struct s5k4ecgx_regset) *
			init_reg_2_size);
	if (regset_table == NULL) {
		kfree(regset_data);
		return -ENOMEM;
	}

	dst_ptr = regset_data;
	regset = regset_table;
	end_src_ptr = &state->regs->init_reg_2.reg[init_reg_2_array_size];

	src_value = *src_ptr++;
	while (src_ptr <= end_src_ptr) {
		/* initial value for a regset */
		regset->data = dst_ptr;
		flag_copied = false;
		*dst_ptr++ = src_value >> 24;
		*dst_ptr++ = src_value >> 16;
		*dst_ptr++ = src_value >> 8;
		*dst_ptr++ = src_value;

		/* check subsequent values for a data flag (starts with
		   0x0F12) or something else */
		do {
			src_value = *src_ptr++;
			if ((src_value & 0xFFFF0000) != 0x0F120000) {
				/* src_value is start of next regset */
				regset->size = dst_ptr - regset->data;
				regset++;
				break;
			}
			/* copy the 0x0F12 flag if not done already */
			if (!flag_copied) {
				*dst_ptr++ = src_value >> 24;
				*dst_ptr++ = src_value >> 16;
				flag_copied = true;
			}
			/* copy the data part */
			*dst_ptr++ = src_value >> 8;
			*dst_ptr++ = src_value;
		} while (src_ptr < end_src_ptr);
	}
	pr_debug("%s : finished creating table\n", __func__);

	end_regset = regset;
	pr_debug("%s : first regset = %p, last regset = %p, count = %d\n",
		__func__, regset_table, regset, end_regset - regset_table);
	pr_debug("%s : regset_data = %p, end = %p, dst_ptr = %p\n", __func__,
		regset_data, regset_data + (init_reg_2_size * sizeof(u32)),
		dst_ptr);

#ifdef CONFIG_VIDEO_S5K4ECGX_DEBUG
	if (s5k4ecgx_debug_mask & S5K4ECGX_DEBUG_I2C_BURSTS) {
		int last_regset_end_addr = 0;
		regset = regset_table;
		do {
			s5k4ecgx_dump_regset(regset);
			if (regset->size > 4) {
				int regset_addr = (regset->data[2] << 8 |
						regset->data[3]);
				if (last_regset_end_addr == regset_addr)
					pr_info("%s : this regset can be"
						" combined with previous\n",
						__func__);
				last_regset_end_addr = (regset_addr +
							regset->size - 6);
			}
			regset++;
		} while (regset < end_regset);
	}
#endif
	regset = regset_table;
	pr_debug("%s : start writing init reg 2 bursts\n", __func__);
	do {
		if (regset->size > 4) {
			/* write the address packet */
			err = s5k4ecgx_i2c_write_block(sd, regset->data, 4);
			if (err)
				break;
			/* write the data in a burst */
			err = s5k4ecgx_i2c_write_block(sd, regset->data+4,
						regset->size-4);

		} else
			err = s5k4ecgx_i2c_write_block(sd, regset->data,
						regset->size);
		if (err)
			break;
		regset++;
	} while (regset < end_regset);

	pr_debug("%s : finished writing init reg 2 bursts\n", __func__);

	vfree(regset_data);
	vfree(regset_table);

	return err;
}

static int s5k4ecgx_init_regs(struct v4l2_subdev *sd)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);
	u16 read_value;

	/* we'd prefer to do this in probe, but the framework hasn't
	 * turned on the camera yet so our i2c operations would fail
	 * if we tried to do it in probe, so we have to do it here
	 * and keep track if we succeeded or not.
	 */

	/* enter read mode */
	s5k4ecgx_i2c_write_twobyte(client, 0x002C, 0x7000);

	s5k4ecgx_i2c_write_twobyte(client, 0x002E, 0x01A6);
	s5k4ecgx_i2c_read_twobyte(client, 0x0F12, &read_value);

	pr_info("%s : revision %08X\n", __func__, read_value);

	/* restore write mode */
	s5k4ecgx_i2c_write_twobyte(client, 0x0028, 0x7000);

#ifdef CONFIG_VIDEO_S5K4ECGX_V_1_0
	if (read_value == S5K4ECGX_VERSION_1_0) {
		state->regs = &regs_for_fw_version_1_0;
		state->initialized = true;
		return 0;
	}
#endif
#ifdef CONFIG_VIDEO_S5K4ECGX_V_1_1
	if (read_value == S5K4ECGX_VERSION_1_1) {
		state->fw.minor = 1;
		state->regs = &regs_for_fw_version_1_1;
		state->initialized = true;
		return 0;
	}
#endif

	dev_err(&client->dev, "%s: unknown fw version 0x%x\n",
		__func__, read_value);
	return -ENODEV;
}

static int s5k4ecgx_init(struct v4l2_subdev *sd, u32 val)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);

	dev_dbg1(&client->dev, "%s: start\n", __func__);

s5k4ecgx_init_parameters(sd);

#if 1
state->fw.minor = 1;
state->regs = &regs_for_fw_version_1_1;
state->initialized = true;

if (s5k4ecgx_set_from_table(sd, "init reg 1",
					&state->regs->init_reg_1, 1, 0) < 0)
	{
			printk(" s5k4ecgx_init Fail2 \n");
		return -EIO;
	}
#else

	if (s5k4ecgx_init_regs(&state->sd) < 0)
		{
		printk(" s5k4ecgx_init Fail1 \n");
		return -ENODEV;
		}

	dev_dbg1(&client->dev, "%s: state->check_dataline : %d\n",
		__func__, state->check_dataline);

	if (s5k4ecgx_set_from_table(sd, "init reg 1",
					&state->regs->init_reg_1, 1, 0) < 0)
	{
			printk(" s5k4ecgx_init Fail2 \n");
		return -EIO;
	}

	/* delay 10ms after wakeup of SOC processor */
	msleep(10);


	if (s5k4ecgx_write_init_reg2_burst(sd) < 0)
	{
			printk(" s5k4ecgx_init Fail3 \n");
		return -EIO;
	}
	

	if (s5k4ecgx_set_from_table(sd, "flash init",
				&state->regs->flash_init, 1, 0) < 0)
	{
			printk(" s5k4ecgx_init Fail4 \n");
		return -EIO;
	}

	if (state->check_dataline) {
		if (s5k4ecgx_set_from_table(sd, "dtp start",
					&state->regs->dtp_start, 1, 0) < 0)
		{
				printk(" s5k4ecgx_init Fail4 \n");
			return -EIO;
		}
	}
#endif

	dev_dbg1(&client->dev, "%s: end\n", __func__);

	return 0;
}

/*
 * s_config subdev ops
 * With camera device, we need to re-initialize
 * every single opening time therefor,
 * it is not necessary to be initialized on probe time.
 * except for version checking
 * NOTE: version checking is optional
 */
static int s5k4ecgx_s_config(struct v4l2_subdev *sd,
			int irq, void *platform_data)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);
	struct s5k4ecgx_platform_data *pdata = client->dev.platform_data;

	/*
	 * Assign default format and resolution
	 * Use configured default information in platform data
	 * or without them, use default information in driver
	 */
	state->pix.width = pdata->default_width;
	state->pix.height = pdata->default_height;

	if (!pdata->pixelformat)
		state->pix.pixelformat = DEFAULT_PIX_FMT;
	else
		state->pix.pixelformat = pdata->pixelformat;

	if (!pdata->freq)
		state->freq = DEFAULT_MCLK;	/* 24MHz default */
	else
		state->freq = pdata->freq;

	return 0; 
}

static int s5k4ecgx_s_stream(struct v4l2_subdev *sd, int enable)
{
	if(enable)
	{
		s5k4ecgx_init(sd,1);
    	s5k4ecgx_set_preview_start(sd);
		printk(" s5k4ecgx_s_stream start stream success \n");
	}
	else
	{
		s5k4ecgx_set_preview_stop(sd);
		printk(" s5k4ecgx_s_stream stop stream success \n");
	}
	//printk("Error !!!!, s5k4ecgx_s_stream is empty\n");
	return 0;
}



static struct soc_camera_ops s5k4ecgx_ops = {
	.set_bus_param		= s5k4ecgx_set_bus_param,
	.query_bus_param	= s5k4ecgx_query_bus_param,
	.controls		= s5k4ecgx_controls,
	.num_controls		= ARRAY_SIZE(s5k4ecgx_controls),
};

static const struct v4l2_subdev_core_ops s5k4ecgx_core_ops = {
	.init = s5k4ecgx_init,	/* initializing API */
//	.s_config = s5k4ecgx_s_config,	/* Fetch platform data */  //@HW Fixed me
	.g_ctrl = s5k4ecgx_g_ctrl,
	.s_ctrl = s5k4ecgx_s_ctrl,
	.s_ext_ctrls = s5k4ecgx_s_ext_ctrls,
};
static const struct v4l2_subdev_video_ops s5k4ecgx_video_ops = {
	.s_stream	= s5k4ecgx_s_stream,
	.s_mbus_fmt = s5k4ecgx_s_fmt,
	.enum_framesizes = s5k4ecgx_enum_framesizes,
	.enum_mbus_fsizes = s5k4ecgx_enum_framesizes,
	.enum_mbus_fmt = s5k4ecgx_enum_fmt,
	.try_mbus_fmt = s5k4ecgx_try_fmt,
	.g_parm = s5k4ecgx_g_parm,
	.s_parm = s5k4ecgx_s_parm,
};

static int s5k4ecgx_g_skip_frames(struct v4l2_subdev *sd, u32 *frames)
{
	/* Quantity of initial bad frames to skip. Revisit. */
	*frames = 0;

	return 0;
}

static int s5k4ecgx_g_interface_parms(struct v4l2_subdev *sd,
			struct v4l2_subdev_sensor_interface_parms *parms)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);

	if (!parms)
		return -EINVAL;


	parms->if_type = state->plat_parms->if_type;
	parms->if_mode = state->plat_parms->if_mode;
	parms->parms = state->plat_parms->parms;
	//parms->parms.serial = mipi_cfgs[ov5640->i_size];


	return 0;
}


static const struct v4l2_subdev_sensor_ops s5k4ecgx_sensor_ops = {
	.g_skip_frames = s5k4ecgx_g_skip_frames,
	.g_interface_parms = s5k4ecgx_g_interface_parms,
};

static const struct v4l2_subdev_ops s5k4ecgx_subdev_ops = {
	.core = &s5k4ecgx_core_ops,
	.video = &s5k4ecgx_video_ops,
	.sensor = &s5k4ecgx_sensor_ops,
};

/*
 * s5k4ecgx_probe
 * Fetching platform data is being done with s_config subdev call.
 * In probe routine, we just register subdev device
 */
static int s5k4ecgx_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct v4l2_subdev *sd;
	struct s5k4ecgx_state *state;
	struct s5k4ecgx_platform_data *pdata = client->dev.platform_data;
	struct soc_camera_device *icd = client->dev.platform_data;
	struct soc_camera_link *icl;

	printk("s5k4ecgx_probe\n"); //@HW
	if (!icd) {
		printk("s5k4ecgx: missing soc-camera data!\n");
		return -EINVAL;
	}

	icl = to_soc_camera_link(icd);
	if (!icl) {
		printk( "s5k4ecgx driver needs platform data\n");
		return -EINVAL;
	}

	if (!icl->priv) {
		dev_err(&client->dev,
			"s5k4ecgx driver needs i/f platform data\n");
		return -EINVAL;
	}


	if ((pdata == NULL) || (pdata->flash_onoff == NULL)) {
		dev_err(&client->dev, "%s: bad platform data\n", __func__);
		printk("BILLA s5k4 bad data fail\n"); 
		return -ENODEV;
	}

	state = kzalloc(sizeof(struct s5k4ecgx_state), GFP_KERNEL);
	if (state == NULL)
		{
		printk("BILLA s5k4ecgx_probe2 fail\n"); 
		return -ENOMEM;
		}

	mutex_init(&state->ctrl_lock);
	init_completion(&state->af_complete);

	state->runmode = S5K4ECGX_RUNMODE_NOTREADY;
	sd = &state->sd;
	strcpy(sd->name, "s5k4ecgx");//S5K4ECGX_DRIVER_NAME);

	state->plat_parms = icl->priv;
	/* Registering subdev */
	v4l2_i2c_subdev_init(sd, client, &s5k4ecgx_subdev_ops);
	icd->ops		= &s5k4ecgx_ops;


	dev_dbg1(&client->dev, "5MP camera S5K4ECGX loaded.\n");

	return 0;
}

static int s5k4ecgx_remove(struct i2c_client *client)
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);
	printk("s5k4ecgx_remove\n"); //@HW
	client->driver = NULL;

	v4l2_device_unregister_subdev(sd);
	mutex_destroy(&state->ctrl_lock);
	kfree(state);

	dev_dbg1(&client->dev, "Unloaded camera sensor S5K4ECGX.\n");

	return 0;
}





static const struct i2c_device_id s5k4ecgx_id[] = {
	{ "s5k4ecgx",0},//S5K4ECGX_DRIVER_NAME, 0 },
	{}
};

MODULE_DEVICE_TABLE(i2c, s5k4ecgx_id);

static struct i2c_driver s5k4ecgx_i2c_driver = {
	.driver = {
		.name = "s5k4ecgx",
	},
	
	.probe = s5k4ecgx_probe,
	.remove = s5k4ecgx_remove,
	.id_table = s5k4ecgx_id,
};

static int __init s5k4ecgx_mod_init(void)
{
	printk("BILLA s5k4 adding driver =  %s \n",S5K4ECGX_DRIVER_NAME);
	return i2c_add_driver(&s5k4ecgx_i2c_driver);
}

static void __exit s5k4ecgx_mod_exit(void)
{
	i2c_del_driver(&s5k4ecgx_i2c_driver);
}

module_init(s5k4ecgx_mod_init);
module_exit(s5k4ecgx_mod_exit);

MODULE_DESCRIPTION("LSI S5K4ECGX 5MP SOC camera driver");
MODULE_AUTHOR("Seok-Young Jang <quartz.jang@samsung.com>");
MODULE_LICENSE("GPL");
