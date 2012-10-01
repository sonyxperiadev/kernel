/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	include/linux/broadcom/hal_camera.h
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

/*
*
*****************************************************************************
*
*  camera.h
*
*  PURPOSE:
*
*  Definitions for the camera driver.
*
*  NOTES:
*
*****************************************************************************/

#if !defined(LINUX_CAMERA_H)
#define LINUX_CAMERA_H

/* ---- Include Files ---------------------------------------------------- */

#include <linux/ioctl.h>

/* ---- Constants and Types ---------------------------------------------- */

#define BCM_CAM_MAGIC   'C'

enum {
	CAM_CMD_FIRST = 0x80,
	CAM_CMD_ENABLE = 0x80,
	CAM_CMD_READ_REG,
	CAM_CMD_WRITE_REG,
	CAM_CMD_SET_FPS,
	CAM_CMD_GET_FRAME,
	CAM_CMD_GET_FRAME1,
	CAM_CMD_GET_JPEG,
	CAM_CMD_GET_THUMBNAIL,
	CAM_CMD_GET_PARAMS,
	CAM_CMD_SET_PARAMS,
	CAM_CMD_SET_SCENE_MODE,
	CAM_CMD_SET_WB,
	CAM_CMD_SET_EFFECT,
	CAM_CMD_SET_ANTI_BANDING,
	CAM_CMD_SET_FLASH_MODE,
	CAM_CMD_SET_FOCUS_MODE,
	CAM_CMD_SET_JPEG_QUALITY,
	CAM_CMD_SET_THUMBNAIL_PARAMS,
	CAM_CMD_SET_DIGITAL_EFFECT,
	CAM_CMD_GET_RESOLUTION_AVAILABLE,
	CAM_CMD_MEM_REGISTER,
	CAM_CMD_MEM_BUFFERS,
	CAM_CMD_GET_STILL_YCbCr,
	CAM_CMD_ENABLE_AUTOFOCUS,
	CAM_CMD_DISABLE_AUTOFOCUS,
	CAM_CMD_SET_ZOOM,
	CAM_CMD_LAST
};

#define CAM_IOCTL_ENABLE	_IO(BCM_CAM_MAGIC, CAM_CMD_ENABLE)	/* arg is int */
#define CAM_IOCTL_READ_REG	_IOWR(BCM_CAM_MAGIC, CAM_CMD_READ_REG, CAM_Reg_t)	/* arg is CAM_Reg_t * */
#define CAM_IOCTL_WRITE_REG	_IOW(BCM_CAM_MAGIC, CAM_CMD_WRITE_REG, CAM_Reg_t)	/* arg is CAM_Reg_t * */
#define CAM_IOCTL_SET_FPS	_IO(BCM_CAM_MAGIC, CAM_CMD_SET_FPS)	/* arg is CamRates_t */
#define CAM_IOCTL_GET_FRAME	_IOWR(BCM_CAM_MAGIC, CAM_CMD_GET_FRAME, CAM_Frame_t)	/* arg is CAM_Frame_t * */
#define CAM_IOCTL_GET_FRAME1	_IOWR(BCM_CAM_MAGIC, CAM_CMD_GET_FRAME1, CAM_Frame1_t)	/* arg is CAM_Frame_t * */
/* #define CAM_IOCTL_GET_JPEG    _IOWR(BCM_CAM_MAGIC, CAM_CMD_GET_JPEG, CAM_Frame_t) arg is CAM_Frame_t *  */
#define CAM_IOCTL_GET_JPEG	_IOWR(BCM_CAM_MAGIC, CAM_CMD_GET_JPEG, CAM_Frame1_t)	/* arg is CAM_Frame_t * */
#define CAM_IOCTL_GET_THUMBNAIL	_IOWR(BCM_CAM_MAGIC, CAM_CMD_GET_THUMBNAIL, CAM_Frame_t)	/* arg is CAM_Frame_t * */
#define CAM_IOCTL_GET_PARAMS	_IOWR(BCM_CAM_MAGIC, CAM_CMD_GET_PARAMS, CAM_Parm_t)	/* arg is CAM_Parm_t * */
#define CAM_IOCTL_SET_PARAMS	_IOW(BCM_CAM_MAGIC, CAM_CMD_SET_PARAMS, CAM_Parm_t)	/* arg is CAM_Parm_t * */
/* [  */
#define CAM_IOCTL_SET_SCENE_MODE	_IOW(BCM_CAM_MAGIC, CAM_CMD_SET_SCENE_MODE, CAM_Parm_t)
#define CAM_IOCTL_SET_WB			_IOW(BCM_CAM_MAGIC, CAM_CMD_SET_WB, CAM_Parm_t)
#define CAM_IOCTL_SET_ZOOM                     _IOW(BCM_CAM_MAGIC, CAM_CMD_SET_ZOOM, CAM_Parm_t)
#define CAM_IOCTL_SET_EFFECT		_IOW(BCM_CAM_MAGIC, CAM_CMD_SET_EFFECT, CAM_Parm_t)
#define CAM_IOCTL_SET_ANTI_BANDING	_IOW(BCM_CAM_MAGIC, CAM_CMD_SET_ANTI_BANDING, CAM_Parm_t)
#define CAM_IOCTL_SET_FLASH_MODE	_IOW(BCM_CAM_MAGIC, CAM_CMD_SET_FLASH_MODE, CAM_Parm_t)
#define CAM_IOCTL_SET_FOCUS_MODE	_IOW(BCM_CAM_MAGIC, CAM_CMD_SET_FOCUS_MODE, CAM_Parm_t)
#define CAM_IOCTL_ENABLE_AUTOFOCUS	_IO(BCM_CAM_MAGIC, CAM_CMD_ENABLE_AUTOFOCUS)
#define CAM_IOCTL_DISABLE_AUTOFOCUS	_IO(BCM_CAM_MAGIC, CAM_CMD_DISABLE_AUTOFOCUS)
#define CAM_IOCTL_SET_JPEG_QUALITY	_IOW(BCM_CAM_MAGIC, CAM_CMD_SET_JPEG_QUALITY, CAM_Parm_t)
/* ] */
#define CAM_IOCTL_SET_THUMBNAIL_PARAMS	_IOW(BCM_CAM_MAGIC, CAM_CMD_SET_THUMBNAIL_PARAMS, CAM_Parm_t)	/* arg is CAM_Parm_t * */
#define CAM_IOCTL_SET_DIGITAL_EFFECT	_IOW(BCM_CAM_MAGIC, CAM_CMD_SET_DIGITAL_EFFECT, CAM_Parm_t)	/* arg is CAM_Parm_t * */
#define CAM_IOCTL_GET_RESOLUTION_AVAILABLE	_IOW(BCM_CAM_MAGIC, CAM_CMD_GET_RESOLUTION_AVAILABLE, CAM_Parm_t)	/* arg is CAM_Parm_t * */
#define CAM_IOCTL_MEM_REGISTER	_IOW(BCM_CAM_MAGIC, CAM_CMD_MEM_REGISTER, CAM_Parm_t)
#define CAM_IOCTL_MEM_BUFFERS     	_IOW(BCM_CAM_MAGIC, CAM_CMD_MEM_BUFFERS, CAM_Parm_t)
#define CAM_IOCTL_GET_STILL_YCbCr	_IOWR( BCM_CAM_MAGIC, CAM_CMD_GET_STILL_YCbCr, CAM_Frame_t )

typedef int64_t nsecs_t;	/* nano-secondss */

typedef struct {
	unsigned int id;
	unsigned int len;
	void *busAddress;
	nsecs_t timestamp;
} CAM_BufData;

typedef struct {
	unsigned short reg;	/* 16-bit or 8-bit reg addr to I2C */
	unsigned int val;	/* value to write to I2C */
	unsigned char cnt;	/* byte count */
} CAM_Reg_t;

typedef struct {
	unsigned short *buffer;
	unsigned int len;
	unsigned int thumbnailOffset;
	unsigned int thumbnailSize;

} CAM_Frame_t;

typedef struct {
	unsigned short *buffer;
	unsigned int len;
} CAM_Frame1_t;

/**  Camera Frame Rates
*/

typedef enum {
	CamRate_Auto = 0,	/* /< Auto fps */
	CamRate_2 = 1,		/* /< 2 fps */
	CamRate_4 = 2,		/* /< 4 fps */
	CamRate_4_16 = 3,	/* /< 4.16 fps */
	CamRate_5 = 4,		/* /< 5 fps */

	CamRate_6 = 5,		/* /< 6 fps */
	CamRate_6_25 = 6,	/* /< 6.25 fps */
	CamRate_7_5 = 7,	/* /< 7.5 fps */

	CamRate_8 = 8,		/* /< 8 fps */
	CamRate_8_33 = 9,	/* /< 8.33 fps */
	CamRate_10 = 11,	/* /< 10 fps */

	CamRate_12 = 12,	/* /< 12 fps */
	CamRate_12_5 = 13,	/* /< 12.5 fps */
	CamRate_15 = 15,	/* /< 15 fps */

	CamRate_20 = 20,	/* /< 20 fps */
	CamRate_24 = 24,	/* /< 24 fps */
	CamRate_25 = 25,	/* /< 25 fps */
	CamRate_30 = 30,	/* /< 30 fps */

	CamRate_48 = 48,	/* /< 48 fps */
	CamRate_50 = 50,	/* /< 50 fps */
	CamRate_60 = 60,	/* /< 60 fps */
	CamRate_N_Steps		/* /< Max fps */
} CamRates_t;

typedef enum {
	CAM_RGB565,		/* 16 bits per pixel RGB 5:6:5 */
	CAM_YUV422,		/* 16 bits per pixel YCrCb 4:2:2 */
	CAM_JPEG		/* JPEG format data */
} CAM_Format_t;

/**  Camera Data Formats
*/
typedef enum {
	CamDataFmtRGB565 = 0x00000001,	/* /< capture RGB565 */
	CamDataFmtYUV = 0x00000002,	/* /< capture YUV format U-Y-Y-U-Y-Y etc. V-Y-Y-V-Y-Y etc. */
	CamDataFmtYCbCr = 0x00000004,	/* /< capture YCbCr format: Y-U-Y-V-Y-U-Y-V etc */
	CamDataFmtJPEG = 0x00000008,	/* /< capture JPEG format */
	CamDataFmtRGB444 = 0x00000010,	/* /< capture RGB444 */
	CamDataFmtRGB888 = 0x00000020,	/* /< capture RGB888 */
	CamDataFmtARGB32 = 0x00000040,	/* /< capture ARGB32 */
	CamDataFmtYUV420Planar = 0x00000080,	/* /< capture YUV420 Planar */
	CamDataFmtSEMIYUV420Planar = 0x00000100,	/* /< capture SEMI YUV420 */
	CamDataFmtPACKEDSEMIYUV420Planar = 0x00000200,	/* /< capture PACKED SEMI YUV420 */
	CamDataFmtBypass = 0x00000400	/* /< Bypass mode for Camera Controller */
} CamDataFmt_t;

/**  Camera Image size
*/
typedef enum {
	CamImageSize_SQCIF = 0x00000001,	/* /< image size 128x96 */
	CamImageSize_QQVGA = 0x00000002,	/* /< image size 160x120 */
	CamImageSize_QCIF = 0x00000004,	/* /< image size 176x144 */
	CamImageSize_240x180 = 0x00000008,	/* /< image size 240x180 */
	CamImageSize_R_QVGA = 0x00000010,	/* /< image size 240x320 */
	CamImageSize_QVGA = 0x00000020,	/* /< image size 320x240 */
	CamImageSize_CIF = 0x00000040,	/* /< image size 352x288 */
	CamImageSize_320x480 = 0x00000080,	/* /< image size 320x480 */
	CamImageSize_640x240 = 0x00000100,	/*/< image size 640x240 */
	CamImageSize_426x320 = 0x00000200,	/* /< image size 426x320 */
	CamImageSize_VGA = 0x00000400,	/* /< image size 640x480 */
	CamImageSize_SVGA = 0x00000800,	/* /< image size 800x600 */
	CamImageSize_XGA = 0x00001000,	/* /< image size 1024x768 */
	CamImageSize_4VGA = 0x00002000,	/* /< image size 1280x960 */
	CamImageSize_SXGA = 0x00004000,	/* /< image size 1280x1024 */
	CamImageSize_UXGA = 0x00008000,	/* /< image size 1600x1200 */
	CamImageSize_QXGA = 0x00010000,	/* /< image size 2048x1536 */
	CamImageSize_QSXGA = 0x00020000,	/* /< image size 2560x1920 */
	CamImageSize_480x360 = 0x00040000,	/* /< image size 2560x1920 */
	CamImageSize_R_QCIF = 0x00080000,	/* /< image size 144x176 */
	CamImageSize_INVALID = 0x00100000	/* /< invalid image capture size */
} CamImageSize_t;

/**  Flash Control
*/
typedef enum {
	Flash_Off = 0x00000000,	/* /< Both off */
	Flash_On = 0x00000001,	/* /< Flash LED on */
	Torch_On = 0x00000002,	/* /< Torch flash on */
	FlashLight_Auto = 0x00000004	/* /< Sensor controls the Flash status (burst mode) */
#if defined(SS_2153) && defined(BROOKLYN_HW)
	    ,
	PreFlash_On = 0x00000008	/* /< pre Flash LE on */
#endif
} FlashLedState_t;

/**  Camera Zoom Values
zoom factor 8.8 format= 256/Zoom_t    1:1=Zoom_t=256  4:1=Zoom_t=64
If  0 < CamZoom_t < 32 use table look-up defined in camera device driver, Camera sensor is capable of zooming
*/
typedef enum {
	CamZoom_1_0 = 256,	/* /< zoom factor 1.0 */
	CamZoom_1_15 = 224,	/* /< zoom factor 1.15   (256/224) */
	CamZoom_1_2 = 216,	/* /< zoom factor 1.2   (256/216) */
	CamZoom_1_33 = 192,	/* /< zoom factor 1.33   (256/192) */
	CamZoom_1_4 = 184,	/* /< zoom factor 1.4   (256/184) */
	CamZoom_1_6 = 160,	/* /< zoom factor 1.6    (256/160) */
	CamZoom_1_8 = 144,	/* /< zoom factor 1.8    (256/144) */
	CamZoom_2_0 = 128,	/* /< zoom factor 2.0    (256/128) */
	CamZoom_2_66 = 96,	/* /< zoom factor 2.66   (256/96) */
	CamZoom_4_0 = 64,	/* /< zoom factor 4.0    (256/64) */
	CamZoom_Table_Max = 31,	/* /< 0 < CamZoom_t < 32 zoom factor is Table look-up */
	CamZoom_PP = 0		/* /< Post Processing Zoom */
} CamZoom_t;

/**  Camera Scaler Values
Scale Down Factor: Horizontal = x/128    Vertical = y/128
*/
typedef enum {
	CamHorizontalNoSDF = 128,	/* /< Camera Controller Horizontal SDF:  Post Processor Zoom = CamHorizontalNoSDF */
	CamVerticalNoSDF = 128	/* /< Camera Controller Vertical SDF:    Post Processor Zoom = CamVerticalNoSDF */
} CamSDF;

#if defined(SS_2153) && defined(BROOKLYN_HW)
/** flip mode using sensor.
  Only advanced users need to use these.
*/
typedef enum {
	CamFlipMode_ORIGINAL,
	CamFlipMode_VERTICAL,	/* /< Vertical flip */
	CamFlipMode_HORIZONTAL,	/* /< Horizontal flip */
	CamFlipMode_SYMMETRIC,	/* /< Symmetric */
	CamFlipMode_N_Modes	/* /< # of Modes */
} CamFlipMode_t;
#endif

/**  Camera Rotation Control
*/
typedef enum {
	CamRotate0 = 0x00000001,	/* /< normal display mode */
	CamRotate90 = 0x00000002,	/* /< rotate display 90 */
	CamRotate180 = 0x00000004,	/* /< rotate display 180 */
	CamRotate270 = 0x00000008,	/* /< rotate display 270 */
	CamRotate_N_Modes = 0x00000009	/* /< # of modes */
} CamRotate_t;

/**  Camera Mirror Control
*/
typedef enum {
	CamMirrorNone = 0x00000001,	/* /< no mirror mode */
	CamMirrorHorizontal = 0x00000002,	/* /< mirror horizontal (pixel at 0, 1 is swapped with pixel W, 1  W=image width) */
	CamMirrorVertical = 0x00000004,	/* /< mirror vertical  (pixel at 1, 0 is swapped with pixel 1, H   H= image height) */
	CamMirrorBoth = 0x00000008,	/* /< (pixel at 0, 0 is swapped with pixel W, H  W=width, H-height of image) */
	CamMirror_N_Modes = 0x00000009	/* /< # of modes */
} CamMirror_t;

/**
 *  Camera Focus Control
 */
typedef enum {
	CamFocusStatusOff = 0,	/* /< focus control off */
	CamFocusStatusRequest,	/* /< request focus */
	CamFocusStatusReached,	/* /< focus reached */
	CamFocusStatusUnableToReach,	/* /< focus cannot be reached */
	CamFocusStatusLost	/* /< lost focus */
} CamFocusStatus_t;

/**
 *  Camera Focus Mode
 */
typedef enum {
	CamFocusControlOn = 0x00000001,	/* /< manual focus control on (manual focus) */
	CamFocusControlOff = 0x00000002,	/* /< focus control off */
	CamFocusControlAuto = 0x00000004,	/* /< auto focus on (continuous) */
	CamFocusControlAutoLock = 0x00000008,	/* /< auto focus on with lock on (full search) */
	CamFocusControlCentroid = 0x00000010,	/* /< auto focus centroid mode */
	CamFocusControlQuickSearch = 0x00000020,	/* /< auto focus quick search mode */
	CamFocusControlInfinity = 0x00000040,	/* /< manual focus move to infinity */
	CamFocusControlMacro = 0x00000080,	/* /< manual focus move to macro */
	CamFocusControlCompletion = 0x00000100	/* /< Call for the completion after shutter realeased (Samsung requests) */
} CamFocusControlMode_t;

/** shooting mode or scene mode.
  Only advanced users need to use these.
*/
typedef enum {
	CamSceneMode_Auto = 0x00000001,	/* /< auto mode for still captures */
	CamSceneMode_Landscape = 0x00000002,	/* /< landscape mode */
	CamSceneMode_Sunset = 0x00000004,	/* /< sunset mode */
	CamSceneMode_Flower = 0x00000008,	/* /< flower mode */
	CamSceneMode_Portrait = 0x00000010,	/* /< portrait mode */
	CamSceneMode_Night = 0x00000020,	/* /< night mode */
	CamSceneMode_Sports = 0x00000040,
	CamSceneMode_Barcode = 0x00000080,
#if defined(SS_2153) && defined(BROOKLYN_HW)
	CamSceneMode_Fallcolor = 0x00000100,
	CamSceneMode_Party_Indoor = 0x00000200,
	CamSceneMode_Beach_Snow = 0x00000400,
	CamSceneMode_Dusk_Dawn = 0x00000800,
	CamSceneMode_Wavensnow = 0x00001000,
	CamSceneMode_Againstlight = 0x00002000,
	CamSceneMode_Firework = 0x00004000,
	CamSceneMode_Text = 0x00008000,
	CamSceneMode_Candlelight = 0x00010000,
	CamSceneMode_None = 0x00020000,
	CamSceneMode_N_Scenes = 0x00040000	/* /< # of scenes */
#else
	CamSceneMode_N_Scenes = 0x00000100	/* /< # of scenes */
#endif
} CamSceneMode_t;

/** Special effects.
  Only advanced users need to use these.
*/
typedef enum {
	CamDigEffect_NoEffect = 0x00000001,	/* /< no digital effect */
	CamDigEffect_MonoChrome = 0x00000002,	/* /< mono chrome effect */
	CamDigEffect_NegMono = 0x00000004,	/* /< negative monochrome */
	CamDigEffect_NegColor = 0x00000008,	/* /< negative color */
	CamDigEffect_SepiaGreen = 0x00000010,	/* /< sepia green */
	CamDigEffect_Posterize = 0x00000020,	/* /< posterize color */
	CamDigEffect_PosterizeMono = 0x00000040,	/* /< posterize monochrome */
	CamDigEffect_Binary = 0x00000080,	/* /< binary */
	CamDigEffect_SolarizeMono = 0x00000100,	/* /< solarize monochrome */
	CamDigEffect_SolarizeColor = 0x00000200,	/* /< solarize color */
	CamDigEffect_Emboss = 0x00000400,	/* /< emboss */
	CamDigEffect_Sketch = 0x00000800,	/* /< sketch */
	CamDigEffect_Antique = 0x00001000,	/* /< antique */
	CamDigEffect_Moonlight = 0x00002000,	/* /< moonlight */
	CamDigEffect_Fog = 0x00004000,	/* /< fog */
	CamDigEffect_Blur = 0x00008000,	/* /< blur */
	CamDigEffect_Chrom = 0x00010000,	/* /< chrome */
	CamDigEffect_Film = 0x00020000,	/* /< film */
	CamDigEffect_Gpen = 0x00040000,	/* /< gpen */
	CamDigEffect_Hatch = 0x00080000,	/* /< hatch */
	CamDigEffect_Noise = 0x00100000,	/* /< noise */
	CamDigEffect_Oils = 0x00200000,	/* /< oils */
	CamDigEffect_Pastel = 0x00400000,	/* /< pastel */
	CamDigEffect_Sharpen = 0x00800000,	/* /< sharpen */
	CamDigEffect_Watercolor = 0x01000000,	/* /< watercolor */
	CamDigEffect_Auqa = 0x02000000,	/* /< aqua */
	CamDigEffect_Outline = 0x04000000,	/* /< outline */
	CamDigEffect_Yellow = 0x08000000,	/* /< yello */
	CamDigEffect_Blue = 0x10000000,	/* /< blue */
	CamDigEffect_Noisereduction = 0x20000000,	/* /< noisereduction */
	CamDigEffect_Red = 0x40000000,	/* /< red */
	CamDigEffect_Green = 0x80000000,	/* /< green */
	CamDigEffect_Night = 0x80000001,	/* /< Night */
	CamDigEffect_N_Effect = 0x80000002	/* /< Max # of Effects */
} CamDigEffect_t;

/** WhiteBalance mode.
  Only advanced users need to use these.
  (base on light source)
*/
typedef enum {
	CamWB_Auto = 0x00000001,	/* /< auto white balance */
	CamWB_Off = 0x00000002,	/* /< no white balance */
	CamWB_Daylight = 0x00000004,	/* /< daylight */
	CamWB_Cloudy = 0x00000008,	/* /< cloudy */
	CamWB_Incandescent = 0x00000010,	/* /< incandescent */
	CamWB_WarmFluorescent = 0x00000020,	/* /< warm fluorescent */
	CamWB_CoolFluorescent = 0x00000040,	/* /< cool fluorescent */
	CamWB_DaylightFluorescent = 0x00000080,	/* /< daylight fluorescent */
	CamWB_Shade = 0x00000100,	/* /< shade */
	CamWB_Tungsten = 0x00000200,	/* /< tungsten */
	CamWB_Flash = 0x00000400,	/* /< flash */
	CamWB_Sunset = 0x00000800,	/* /< sunset */
	CamWB_Horizon = 0x00001000,	/* /< horizon */
	CamWB_Twilight = 0x00002000,	/* /< twilight */
	CamWB_N_Modes = 0x00004000	/* /< Max # of Modes */
} CamWB_WBMode_t;

/** metering type.
  Only advanced users need to use these.
*/
typedef enum {
	CamMeteringType_Auto = 0x00000001,	/* /< auto */
	CamMeteringType_Average = 0x00000002,	/* /< average */
	CamMeteringType_Matrix = 0x00000004,	/* /< matrix */
	CamMeteringType_Spot = 0x00000008,	/* /< spot */
	CamMeteringType_CenterWeighted = 0x00000010,	/* /< center weighted meter mode */
	CamMeteringType_N_Modes = 0x00000020	/* /< # of metering types */
} CamMeteringType_t;

/**  Camera Flicker Control
  Only advanced users need to use these.
*/
typedef enum {
	CamFlickerAuto = 0x00000001,	/* /< flicker control auto */
	CamFlickerDisable = 0x00000002,	/* /< flicker control disabled */
	CamFlicker50Hz = 0x00000004,	/* /< flicker control 50 hz */
	CamFlicker60Hz = 0x00000008	/* /< flicker control 60 hz */
} CamFlicker_t;

/* [ Camera Anti banding Settings */
typedef enum {
	CamAntiBandingAuto = 0x00000001,	/* / < auto */
	CamAntiBanding50Hz = 0x00000002,	/* /< 50Hz */
	CamAntiBanding60Hz = 0x00000004,	/* /< 60 Hz */
	CamAntiBandingOff = 0x00000008	/* /< off */
} CamAntiBanding_t;
/* ] */

/**  Camera Exposure Settings
  Only advanced users need to use these.
*/
typedef enum {
	CamExposure_Disable = 0x00000001,	/* /< disable exposure control */
	CamExposure_Enable = 0x00000002,	/* /< Auto exposure */
	CamExposure_Night = 0x00000004,	/* /< Night exposure */
	CamExposure_Backlight = 0x00000008,	/* /< Backlight exposure */
	CamExposure_Spotlight = 0x00000010,	/* /< Spotlight exposure */
	CamExposure_Sports = 0x00000020,	/* /< Sports exposure */
	CamExposure_Snow = 0x00000040,	/* /< Snow exposure */
	CamExposure_Beach = 0x00000080,	/* /< Beach exposure */
	CamExposure_LargeAperture = 0x00000100,	/* /< LargeAperture exposure */
	CamExposure_SmallAperture = 0x00000200	/* /< SmallAperture exposure */
} CamExposure_t;

/** Camera Sensitivity.
  Only advanced users need to use these.
*/
typedef enum {
	CamSensitivity_Auto = 0x00000001,	/* /< auto iso */
	CamSensitivity_50 = 0x00000002,	/* /< iso 50-80 */
	CamSensitivity_100 = 0x00000004,	/* /< iso 100 */
	CamSensitivity_200 = 0x00000008,	/* /< iso 200 */
	CamSensitivity_400 = 0x00000010,	/* /< iso 400 */
	CamSensitivity_800 = 0x00000020,	/* /< iso 800 */
	CamSensitivity_1600 = 0x00000040	/* /< iso 1600 + */
} CamSensitivity_t;

/** Camera Saturation.
  Image Appearance:  Only advanced users need to use these.
*/
typedef enum {
	CamContrast_Min = -100,	/* /< minimum Contrast */
	CamContrast_Nom = 0,	/* /< nominal Contrast */
	CamContrast_Max = 100	/* /< maximum Contrast */
} CamContrast_t;
typedef enum {
	CamBrightness_Min = 0,	/* /< minimum Brightness */
	CamBrightness_Nom = 50,	/* /< nominal Brightness */
	CamBrightness_Max = 100	/* /< maximum Brightness */
} CamBrightness_t;
typedef enum {
	CamSaturation_Min = -100,	/* /< minimum Saturation */
	CamSaturation_Nom = 0,	/* /< nominal Saturation */
	CamSaturation_Max = 100	/* /< maximum Saturation */
} CamSaturation_t;
typedef enum {
	CamGamma_Min = -100,	/* /< minimum Gamma */
	CamGamma_Nom = 0,	/* /< nominal Gamma */
	CamGamma_Max = 100	/* /< maximum Gamma */
} CamGamma_t;
typedef enum {
	CamHue_Min = -100,	/* /< minimum Hue */
	CamHue_Nom = 0,		/* /< nominal Hue */
	CamHue_Max = 100	/* /< maximum Hue */
} CamHue_t;
typedef enum {
	CamSharpness_Min = -100,	/* /< minimum Sharpness */
	CamSharpness_Nom = 0,	/* /< nominal Sharpness */
	CamSharpness_Max = 100	/* /< maximum Sharpness */
} CamSharpness_t;
typedef enum {
	CamAntiShadingPower_Min = -100,	/* /< minimum AntiShadingPower */
	CamAntiShadingPower_Nom = 0,	/* /< nominal AntiShadingPower */
	CamAntiShadingPower_Max = 100	/* /< maximum AntiShadingPower */
} CamAntiShadingPower_t;
typedef enum {
#ifdef CAMERA_STILL_YCbCr_SUPPORT
	CamJpegQuality_Min = 5,	/* minimum JpegQuality */
	CamJpegQuality_Nom = 7,	/* nominal JpegQuality */
	CamJpegQuality_Max = 9	/* maximum JpegQuality */
#else
#if defined(SS_2153) && defined(BROOKLYN_HW)
	CamJpegQuality_Min = 0,
	CamJpegQuality_Nom = 50,	/* nominal JpegQuality */
	CamJpegQuality_Max = 100	/* maximum JpegQuality */
#else
	CamJpegQuality_Min = 65,	/* minimum Jpegquality */
	CamJpegQuality_Nom = 75,	/* nominal JpegQuality */
	CamJpegQuality_Max = 85	/* maximum JpegQuality */
#endif
#endif
} CamJpegQuality_t;

/* * Camera Window and Scale Config */
typedef struct {
	CamImageSize_t size;	/* /< (in) image size to window & scale */
	unsigned int start_pixel;	/* /< (in) start pixel (x start) */
	unsigned int end_pixel;	/* /< (in) stop pixel  (x stop) */
	unsigned int start_line;	/* /< (in) start line  (y start) */
	unsigned int end_line;	/* /< (in) end line    (y stop) */
	unsigned int horizontal_SDF;	/* /< (in) horizontal scale factor */
	unsigned int vertical_SDF;	/* /< (in) vertical scale factor */
} CAM_window_scale_st_t;

/**  Camera Capture Mode
*/
typedef enum {
	CamPreview,		/* /< Preview capture mode */
	CamVideo,		/* /< video capture mode */
	CamVideonViewFinder,	/* /< Video & ViewFinder capture mode */
	CamStill,		/* /< Still image capture mode */
	CamStillnThumb		/* /< Still & ThumbNail image capture mode */
} CamMode_t;

typedef struct {
	CamRates_t fps;		/* frames per second */
	CamDataFmt_t format;	/* image format */
	CAM_window_scale_st_t size_window;	/* image resolution/windowing */
	CamMode_t mode;		/* image capture mode */
	CamSceneMode_t scenemode;	/* scene mode */
	CamWB_WBMode_t wbmode;	/* WB mode */
	CamDigEffect_t coloreffects;	/* color effects */
	CamAntiBanding_t antibanding;	/* antibanding */
	FlashLedState_t flash;	/* flash mode */
	CamFocusControlMode_t focus;	/* focus mode */
	CamJpegQuality_t quality;	/* jpeg quality */
	CamZoom_t zoom;

} CAM_Parm_t;

/* ---- Variable Externs ------------------------------------------------- */

/* ---- Function Prototypes ---------------------------------------------- */

#ifdef __KERNEL__

/* ---- Constants and Types ---------------------------------------------- */

typedef void (*CAM_fncptr) (void *, void *);	/* callback function pointer type */

typedef struct {
	CAM_fncptr frameFunc;	/* callback to run when frame is complete */
	void *frameFuncArg;	/* callback argument */
	int enable;		/* enable level */
	int enableInts;		/* interrupts enable level */
	int dmaChannel;		/* DMA channel number */
} CAM_Config_t;

/* ---- Function Prototypes ---------------------------------------------- */
int camEnableInts(int level);
int camEnable(int level);
void camGetParm(CAM_Parm_t *parmp);
int camSetParm(CAM_Parm_t *parmp);

#endif /* __KERNEL */
#endif /* LINUX_CAMERA_H */
