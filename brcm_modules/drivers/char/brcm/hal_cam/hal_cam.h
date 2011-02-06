/*******************************************************************************************
Copyright 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement 
governing use of this software, this software is licensed to you under the 
terms of the GNU General Public License version 2, available at 
http://www.gnu.org/copyleft/gpl.html (the "GPL"). 

Notwithstanding the above, under no circumstances may you combine this software 
in any way with any other Broadcom software provided under a license other than 
the GPL, without Broadcom's express prior written consent.
*******************************************************************************************/


/**
*
*   @file   hal_cam.h
*
*   @brief  Global declaration of hardware abstraction layer for Camera driver.
*
****************************************************************************/
/**
*   @defgroup   HALCamDrvGroup   Camera HAL Interface
*   @ingroup    HALMMGroup
*   @brief      This group is the HAL API driver for the Camera Interface
*
*   HAL API for Camera.
*
*	Click here to return to the HAL Multimedia overview: \ref MMHALOverview
****************************************************************************/

#if !defined( _HAL_CAM_H__ )
#define _HAL_CAM_H__

// ---- Include Files -------------------------------------------------------
//******************************************************************************
//
// Cam Device InDependent API - declared in can_drv_common.h
//
// These functions are common to phone board design and Camera devices
//
//******************************************************************************
#include <linux/types.h>
#include <linux/broadcom/camera.h>
#include <cfg_global.h>
#include <linux/broadcom/types.h>

// ---- Public Constants and Types  ----------------------------------------------------

#define FALSE	0
#define TRUE	1

/**  Camera Sensor Select
*/
typedef enum
{
    CamSensorPrimary,                       ///< Select Primary Camera Sensor
    CamSensorSecondary                  ///< Select Secondary Camera Sensor
} CamSensorSelect_t;

#if 0
/**  Camera Frame Rates
*/

typedef enum {
    CamRate_Auto = 0,                   ///< Auto fps
    CamRate_2     =  1,                 ///< 2 fps
    CamRate_4     =  2,                 ///< 4 fps
    CamRate_4_16  =  3,                 ///< 4.16 fps
    CamRate_5     =  4,                 ///< 5 fps

    CamRate_6     =  5,                 ///< 6 fps
    CamRate_6_25  =  6,                 ///< 6.25 fps
    CamRate_7_5   =  7,                 ///< 7.5 fps

    CamRate_8     =  8,                 ///< 8 fps
    CamRate_8_33  =  9,                 ///< 8.33 fps
    CamRate_10    = 11,                 ///< 10 fps

    CamRate_12    = 12,                 ///< 12 fps
    CamRate_12_5  = 13,                 ///< 12.5 fps
    CamRate_15    = 15,                 ///< 15 fps

    CamRate_20    = 20,                 ///< 20 fps
    CamRate_24    = 24,                 ///< 24 fps
    CamRate_25    = 25,                 ///< 25 fps
    CamRate_30    = 30,                 ///< 30 fps

    CamRate_48    = 48,                 ///< 48 fps
    CamRate_50    = 50,                 ///< 50 fps
    CamRate_60    = 60,                 ///< 60 fps
    CamRate_N_Steps                 ///< Max fps
} CamRates_t;

/**  Camera Data Formats
*/
typedef enum
{
    CamDataFmtRGB565 =					0x00000001, ///< capture RGB565
    CamDataFmtYUV  =					0x00000002, ///< capture YUV format U-Y-Y-U-Y-Y etc. V-Y-Y-V-Y-Y etc.
    CamDataFmtYCbCr =					0x00000004, ///< capture YCbCr format: Y-U-Y-V-Y-U-Y-V etc
    CamDataFmtJPEG =					0x00000008, ///< capture JPEG format
    CamDataFmtRGB444 =					0x00000010, ///< capture RGB444
    CamDataFmtRGB888 =					0x00000020, ///< capture RGB888
    CamDataFmtARGB32 =					0x00000040,	///< capture ARGB32
    CamDataFmtYUV420Planar =			0x00000080, ///< capture YUV420 Planar
    CamDataFmtSEMIYUV420Planar =		0x00000100,	///< capture SEMI YUV420
    CamDataFmtPACKEDSEMIYUV420Planar =	0x00000200,	///< capture PACKED SEMI YUV420
    CamDataFmtBypass =					0x00000400  ///< Bypass mode for Camera Controller
} CamDataFmt_t;


/**  Camera Image size
*/
typedef enum
{
    CamImageSize_SQCIF      = 0x00000001,       ///< image size 128x96
    CamImageSize_QQVGA      = 0x00000002,       ///< image size 160x120
    CamImageSize_QCIF       = 0x00000004,       ///< image size 176x144
    CamImageSize_240x180    = 0x00000008,       ///< image size 240x180
    CamImageSize_R_QVGA     = 0x00000010,       ///< image size 240x320
    CamImageSize_QVGA       = 0x00000020,       ///< image size 320x240
    CamImageSize_CIF        = 0x00000040,       ///< image size 352x288
    CamImageSize_426x320    = 0x00000080,       ///< image size 426x320
    CamImageSize_VGA        = 0x00000100,       ///< image size 640x480
    CamImageSize_SVGA       = 0x00000200,       ///< image size 800x600
    CamImageSize_XGA        = 0x00000400,       ///< image size 1024x768
    CamImageSize_4VGA       = 0x00000800,       ///< image size 1280x960
    CamImageSize_SXGA       = 0x00001000,       ///< image size 1280x1024
    CamImageSize_UXGA       = 0x00002000,       ///< image size 1600x1200
    CamImageSize_QXGA       = 0x00004000,       ///< image size 2048x1536
    CamImageSize_QSXGA      = 0x00008000,       ///< image size 2560x1920
    CamImageSize_INVALID    = 0x00010000        ///< invalid image capture size
} CamImageSize_t;
#endif

/**  Camera Capture Mode
*/
typedef enum
{
    CamPowerOff,              ///< Power Off mode
    CamPowerOn,               ///< Power On mode
    CamIdle,                  ///< Idle mode
    CamCaptureVideo,          ///< video capture mode
    CamCaptureVideonViewFinder, ///< Video & ViewFinder capture mode
    CamCaptureStill,          ///< Still image capture mode
    CamCaptureStillnThumb     ///< Still & ThumbNail image capture mode
} CamCaptureMode_t;

#if 0
/**  Flash Control
*/
typedef enum
{
    Flash_Off =             0x00000000,     ///< Both off
    Flash_On =              0x00000001,     ///< Flash LED on
    Torch_On =              0x00000002,     ///< Torch flash on
    FlashLight_Auto =       0x00000004      ///< Sensor controls the Flash status (burst mode)
#if defined(SS_2153) && defined(BROOKLYN_HW)
	,
    PreFlash_On     =       0x00000008      ///< pre Flash LE on
#endif
} FlashLedState_t;

/**  Camera Zoom Values
zoom factor 8.8 format= 256/Zoom_t    1:1=Zoom_t=256  4:1=Zoom_t=64
If  0 < CamZoom_t < 32 use table look-up defined in camera device driver, Camera sensor is capable of zooming
*/
typedef enum
{
    CamZoom_1_0     = 256,          ///< zoom factor 1.0
    CamZoom_1_15    = 224,          ///< zoom factor 1.15   (256/224)
    CamZoom_1_33    = 192,          ///< zoom factor 1.33   (256/192)
    CamZoom_1_6     = 160,          ///< zoom factor 1.6    (256/160)
    CamZoom_2_0     = 128,          ///< zoom factor 2.0    (256/128)
    CamZoom_2_66    = 96,           ///< zoom factor 2.66   (256/96)
    CamZoom_4_0     = 64,           ///< zoom factor 4.0    (256/64)
    CamZoom_Table_Max   = 31,       ///< 0 < CamZoom_t < 32 zoom factor is Table look-up
    CamZoom_PP      = 0             ///< Post Processing Zoom
} CamZoom_t;

/**  Camera Scaler Values
Scale Down Factor: Horizontal = x/128    Vertical = y/128
*/
typedef enum
{
    CamHorizontalNoSDF  = 128,      ///< Camera Controller Horizontal SDF:  Post Processor Zoom = CamHorizontalNoSDF
    CamVerticalNoSDF    = 128       ///< Camera Controller Vertical SDF:    Post Processor Zoom = CamVerticalNoSDF
} CamSDF;

#if defined(SS_2153) && defined(BROOKLYN_HW)
/** flip mode using sensor.
  Only advanced users need to use these.
*/
typedef enum
{
    CamFlipMode_ORIGINAL,
    CamFlipMode_VERTICAL ,   ///< Vertical flip
    CamFlipMode_HORIZONTAL,    ///< Horizontal flip
    CamFlipMode_SYMMETRIC,      ///< Symmetric
    CamFlipMode_N_Modes           ///< # of Modes
} CamFlipMode_t;
#endif

/**  Camera Rotation Control
*/
typedef enum
{
    CamRotate0 =            0x00000001,     ///< normal display mode
    CamRotate90 =           0x00000002,     ///< rotate display 90
    CamRotate180 =          0x00000004,     ///< rotate display 180
    CamRotate270 =          0x00000008,     ///< rotate display 270
    CamRotate_N_Modes =     0x00000009      ///< # of modes
} CamRotate_t;

/**  Camera Mirror Control
*/
typedef enum
{
    CamMirrorNone =         0x00000001,     ///< no mirror mode
    CamMirrorHorizontal =   0x00000002,     ///< mirror horizontal (pixel at 0,1 is swapped with pixel W,1  W=image width)
    CamMirrorVertical =     0x00000004,     ///< mirror vertical  (pixel at 1,0 is swapped with pixel 1,H   H= image height)
    CamMirrorBoth =         0x00000008,     ///< (pixel at 0,0 is swapped with pixel W,H  W=width, H-height of image)
    CamMirror_N_Modes =     0x00000009      ///< # of modes
} CamMirror_t;

/**
 *  Camera Focus Control
 */
typedef enum
{
    CamFocusStatusOff               = 0,            ///< focus control off
    CamFocusStatusRequest,                          ///< request focus
    CamFocusStatusReached,                          ///< focus reached
    CamFocusStatusUnableToReach,                    ///< focus cannot be reached
    CamFocusStatusLost                              ///< lost focus
} CamFocusStatus_t;


/**
 *  Camera Focus Mode
 */
typedef enum
{
    CamFocusControlOn =         0x00000001,         ///< manual focus control on (manual focus)
    CamFocusControlOff =        0x00000002,         ///< focus control off
    CamFocusControlAuto =       0x00000004,         ///< auto focus on (continuous)
    CamFocusControlAutoLock =   0x00000008,         ///< auto focus on with lock on (full search)
    CamFocusControlCentroid =   0x00000010,         ///< auto focus centroid mode
    CamFocusControlQuickSearch = 0x00000020,        ///< auto focus quick search mode
    CamFocusControlInfinity =   0x00000040,         ///< manual focus move to infinity
    CamFocusControlMacro =      0x00000080,         ///< manual focus move to macro
    CamFocusControlCompletion = 0x00000100          ///< Call for the completion after shutter realeased (Samsung requests)
}  CamFocusControlMode_t;


/** shooting mode or scene mode.
  Only advanced users need to use these.
*/
typedef enum
{
    CamSceneMode_Auto =         0x00000001,     ///< auto mode for still captures
    CamSceneMode_Landscape =    0x00000002,     ///< landscape mode
    CamSceneMode_Sunset =       0x00000004,     ///< sunset mode
    CamSceneMode_Flower =       0x00000008,     ///< flower mode
    CamSceneMode_Portrait =     0x00000010,     ///< portrait mode
    CamSceneMode_Night =        0x00000020,     ///< night mode
#if defined(SS_2153) && defined(BROOKLYN_HW)
    CamSceneMode_Fallcolor=     0x00000040,
	CamSceneMode_Sports =       0x00000080,
    CamSceneMode_Party_Indoor = 0x00000100,
    CamSceneMode_Beach_Snow =   0x00000200,
	CamSceneMode_Dusk_Dawn =    0x00000400,
	CamSceneMode_Wavensnow =    0x00000800,
	CamSceneMode_Againstlight = 0x00001000,
	CamSceneMode_Firework =     0x00002000,
	CamSceneMode_Text =         0x00004000,
	CamSceneMode_Candlelight =  0x00008000,
	CamSceneMode_None =			0x00010000,
    CamSceneMode_N_Scenes =     0x00020000      ///< # of scenes
#else
    CamSceneMode_N_Scenes =     0x00000040      ///< # of scenes
#endif
} CamSceneMode_t;

/** Special effects.
  Only advanced users need to use these.
*/
typedef enum
{
    CamDigEffect_NoEffect =         0x00000001,     ///< no digital effect
    CamDigEffect_MonoChrome =       0x00000002,     ///< mono chrome effect
    CamDigEffect_NegMono =          0x00000004,     ///< negative monochrome
    CamDigEffect_NegColor =         0x00000008,     ///< negative color
    CamDigEffect_SepiaGreen =       0x00000010,     ///< sepia green
    CamDigEffect_Posterize =        0x00000020,     ///< posterize color
    CamDigEffect_PosterizeMono =    0x00000040,     ///< posterize monochrome
    CamDigEffect_Binary =           0x00000080,     ///< binary
    CamDigEffect_SolarizeMono =     0x00000100,     ///< solarize monochrome
    CamDigEffect_SolarizeColor =    0x00000200,     ///< solarize color
    CamDigEffect_Emboss =           0x00000400,     ///< emboss
    CamDigEffect_Sketch =           0x00000800,     ///< sketch
    CamDigEffect_Antique =          0x00001000,     ///< antique
    CamDigEffect_Moonlight =        0x00002000,     ///< moonlight
    CamDigEffect_Fog =              0x00004000,     ///< fog
    CamDigEffect_Blur =             0x00008000,     ///< blur
    CamDigEffect_Chrom =            0x00010000,     ///< chrome
    CamDigEffect_Film =             0x00020000,     ///< film
    CamDigEffect_Gpen =             0x00040000,     ///< gpen
    CamDigEffect_Hatch =            0x00080000,     ///< hatch
    CamDigEffect_Noise =            0x00100000,     ///< noise
    CamDigEffect_Oils =             0x00200000,     ///< oils
    CamDigEffect_Pastel =           0x00400000,     ///< pastel
    CamDigEffect_Sharpen =          0x00800000,     ///< sharpen
    CamDigEffect_Watercolor =       0x01000000,     ///< watercolor
#if defined(SS_2153) && defined(BROOKLYN_HW)
	CamDigEffect_Auqa =				0x02000000,     ///< auqa
	CamDigEffect_Outline =          0x04000000,     ///< outline
	CamDigEffect_Yellow  =          0x08000000,     ///< yello
	CamDigEffect_Blue =             0x10000000,     ///< blue
	CamDigEffect_Noisereduction =   0x20000000,     ///< noisereduction
	CamDigEffect_Red =   			0x40000000,     ///< red
	CamDigEffect_Green =   			0x80000000,     ///< green
	CamDigEffect_Night =   			0x80000001,     ///< Night
    CamDigEffect_N_Effect =         0x80000002      ///< Max # of Effects
#else
    CamDigEffect_N_Effect =         0x02000000      ///<  Max # of Effects
#endif
} CamDigEffect_t;

/** WhiteBalance mode.
  Only advanced users need to use these.
  (base on light source)
*/
typedef enum
{
    CamWB_Auto =                    0x00000001,     ///< auto white balance
    CamWB_Off =                     0x00000002,     ///< no white balance
    CamWB_Daylight =                0x00000004,     ///< daylight
    CamWB_Cloudy =                  0x00000008,     ///< cloudy
    CamWB_Incandescent =            0x00000010,     ///< incandescent
    CamWB_WarmFluorescent =         0x00000020,     ///< warm fluorescent
    CamWB_CoolFluorescent =         0x00000040,     ///< cool fluorescent
    CamWB_DaylightFluorescent =     0x00000080,     ///< daylight fluorescent
    CamWB_Shade =                   0x00000100,     ///< shade
    CamWB_Tungsten =                0x00000200,     ///< tungsten
    CamWB_Flash =                   0x00000400,     ///< flash
    CamWB_Sunset =                  0x00000800,     ///< sunset
    CamWB_Horizon =                 0x00001000,     ///< horizon
    CamWB_N_Modes =                 0x00002000      ///< Max # of Modes
} CamWB_WBMode_t;

/** metering type.
  Only advanced users need to use these.
*/
typedef enum
{
    CamMeteringType_Auto =          0x00000001,     ///< auto
    CamMeteringType_Average =       0x00000002,     ///< average
    CamMeteringType_Matrix =        0x00000004,     ///< matrix
    CamMeteringType_Spot =          0x00000008,     ///< spot
    CamMeteringType_CenterWeighted = 0x00000010,    ///< center weighted meter mode
    CamMeteringType_N_Modes =       0x00000020      ///< # of metering types
} CamMeteringType_t;

/**  Camera Flicker Control
  Only advanced users need to use these.
*/
typedef enum
{
    CamFlickerAuto =                0x00000001,     ///< flicker control auto
    CamFlickerDisable =             0x00000002,     ///< flicker control disabled
    CamFlicker50Hz =                0x00000004,     ///< flicker control 50 hz
    CamFlicker60Hz =                0x00000008      ///< flicker control 60 hz
} CamFlicker_t;

/**  Camera Exposure Settings
  Only advanced users need to use these.
*/
typedef enum
{
    CamExposure_Disable =           0x00000001,      ///< disable exposure control
    CamExposure_Enable =            0x00000002,     ///< Auto exposure
    CamExposure_Night =             0x00000004,     ///< Night exposure
    CamExposure_Backlight =         0x00000008,     ///< Backlight exposure
    CamExposure_Spotlight =         0x00000010,     ///< Spotlight exposure
    CamExposure_Sports =            0x00000020,     ///< Sports exposure
    CamExposure_Snow =              0x00000040,     ///< Snow exposure
    CamExposure_Beach =             0x00000080,     ///< Beach exposure
    CamExposure_LargeAperture =     0x00000100,     ///< LargeAperture exposure
    CamExposure_SmallAperture =     0x00000200      ///< SmallAperture exposure
} CamExposure_t;

/** Camera Sensitivity.
  Only advanced users need to use these.
*/
typedef enum
{
  CamSensitivity_Auto =             0x00000001,     ///< auto iso
  CamSensitivity_50 =               0x00000002,     ///< iso 50-80
  CamSensitivity_100 =              0x00000004,     ///< iso 100
  CamSensitivity_200 =              0x00000008,     ///< iso 200
  CamSensitivity_400 =              0x00000010,     ///< iso 400
  CamSensitivity_800 =              0x00000020,      ///< iso 800
  CamSensitivity_1600 =             0x00000040      ///< iso 1600+
} CamSensitivity_t;

/** Camera Saturation.
  Image Appearance:  Only advanced users need to use these.
*/
typedef enum
{
  CamContrast_Min =     -100,   ///< minimum Contrast
  CamContrast_Nom =     0,      ///< nominal Contrast
  CamContrast_Max =     100     ///< maximum Contrast
} CamContrast_t;
typedef enum
{
  CamBrightness_Min =   0,      ///< minimum Brightness
  CamBrightness_Nom =   50,     ///< nominal Brightness
  CamBrightness_Max =   100     ///< maximum Brightness
} CamBrightness_t;
typedef enum
{
  CamSaturation_Min =   -100,   ///< minimum Saturation
  CamSaturation_Nom =   0,      ///< nominal Saturation
  CamSaturation_Max =   100     ///< maximum Saturation
} CamSaturation_t;
typedef enum
{
  CamGamma_Min =        -100,   ///< minimum Gamma
  CamGamma_Nom =        0,      ///< nominal Gamma
  CamGamma_Max =        100     ///< maximum Gamma
} CamGamma_t;
typedef enum
{
  CamHue_Min =          -100,   ///< minimum Hue
  CamHue_Nom =          0,      ///< nominal Hue
  CamHue_Max =          100     ///< maximum Hue
} CamHue_t;
typedef enum
{
  CamSharpness_Min =    -100,   ///< minimum Sharpness
  CamSharpness_Nom =    0,      ///< nominal Sharpness
  CamSharpness_Max =    100     ///< maximum Sharpness
} CamSharpness_t;
typedef enum
{
  CamAntiShadingPower_Min =     -100,   ///< minimum AntiShadingPower
  CamAntiShadingPower_Nom =     0,      ///< nominal AntiShadingPower
  CamAntiShadingPower_Max =     100     ///< maximum AntiShadingPower
} CamAntiShadingPower_t;
typedef enum
{
  CamJpegQuality_Min =          0,      ///< minimum JpegQuality
#if defined(SS_2153) && defined(BROOKLYN_HW)
  CamJpegQuality_Nom =          50,     ///< nominal JpegQuality
  CamJpegQuality_Max =          100     ///< maximum JpegQuality
#else
  CamJpegQuality_Nom =          5,      ///< nominal JpegQuality
  CamJpegQuality_Max =          10      ///< maximum JpegQuality
#endif
} CamJpegQuality_t;
#endif

/**  HAL CAM driver action request
*/
typedef enum {
// Camera Configuration
    ACTION_CAM_SetPowerMode = 0,            ///< CamPowerOff/CamPowerOn interface and sensor, use HAL_CAM_Action_power_mode_st_t to set
    ACTION_CAM_GetCameraSettings,       ///< Get Nominal/Min/Max camera settings, use HAL_CAM_Action_settings_st_t to get
    ACTION_CAM_GetCameraInterface,      ///< Get I2C and Camera Controller interface settings, use HAL_CAM_Action_cam_interface_st_t to get
    ACTION_CAM_GetImageSize,            ///< Get matching image size from passed pixel width and height, use HAL_CAM_Action_image_size_st_t to get CamImageSize_t
    ACTION_CAM_GetSensorResolution,     ///< Get sensor height, width from passed image size and capture mode, use HAL_CAM_Action_resolution_st_t to get CamImageSize_t, width, & height

// Image Capture Modes
    ACTION_CAM_CaptureImage,            ///< Capture camera image, use HAL_CAM_Action_capture_image_st_t to enable
    ACTION_CAM_CaptureVideo,            ///< Start camera video capture, use HAL_CAM_Action_capture_video_st_t to enable
    ACTION_CAM_CapImageWithThumbNail,   ///< Capture camera image with thumbnail, use HAL_CAM_Action_capture_image_with_thumbnail_st_t to enable
    ACTION_CAM_CapVideoWithViewFinder,  ///< Start camera video capture with viewfinder, use HAL_CAM_Action_capture_video_with_viewfinder_st_t to enable
    ACTION_CAM_StopCapture,             ///< Stop viewfinder, video, or image capture, no additional parameters
#if 1
    ACTION_CAM_PauseCapture,            ///< Pause viewfinder, video, or image capture, no additional parameters
    ACTION_CAM_ResumeCapture,           ///< Resume viewfinder, video, or image capture, no additional parameters
#endif

// Image Capture Modes with Post Process Zoom
    ACTION_CAM_PreviewPPzoom,           ///< Start camera preview/video capture, use HAL_CAM_Action_preview_PPzoom_st_t to enable
    ACTION_CAM_StillsPPzoom,            ///< Stills capture, use HAL_CAM_Action_stills_PPzoom_st_t to enable

// Operation Control Settings
    ACTION_CAM_SetWindowScale,          ///< Set the Camera window size and Scale Down Factors, use HAL_CAM_Action_window_scale_st_t to set
    ACTION_CAM_SetCntrlWindowSize,          ///< Set the Camera Controller window size, use HAL_CAM_Action_cntrl_window_st_t to set
    ACTION_CAM_SetFrameRate,            ///< Set required picture frame, use (CamRates_t) HAL_CAM_Action_param_st_t to set
    ACTION_CAM_SetZoom,                 ///< Set required zoom, use (UInt16) HAL_CAM_Action_param_st_t to set
    ACTION_CAM_GetMaxWindowSize,        ///< Camera Maximum Pixel Window size, use HAL_CAM_Action_window_st_t to get
    ACTION_CAM_GetMaxZoom,              ///< Get maximum zoom allowed, use (UInt16) HAL_CAM_Action_param_st_t to get
    ACTION_CAM_GetJpegMaxSize,          ///< Get Jpeg max image size (in bytes), use HAL_CAM_Action_param_st_t to get
    ACTION_CAM_SetFrameStabEnable,      ///< Set Frame Stabilization enable TRUE/FALSE, use (Boolean) HAL_CAM_Action_param_st_t to set
    ACTION_CAM_SetAntiShakeEnable,      ///< Set Anti-Shake enable TRUE/FALSE, use (Boolean) HAL_CAM_Action_param_st_t to set
    ACTION_CAM_SetAutoFocusEnable,      ///< Set Auto Focus enable TRUE/FALSE, use (Boolean) HAL_CAM_Action_param_st_t to set

// Face Detection
    ACTION_CAM_SetFaceDetectEnable,     ///< Set Face Detection enable TRUE/FALSE, use (Boolean) HAL_CAM_Action_param_st_t to set
    ACTION_CAM_GetFaceDetectRect,       ///< Get Face Detection Rectangle use HAL_CAM_Action_face_detect_st_t to get
// Flash Settings
    ACTION_CAM_SetFlashMode,            ///< Set required flash mode, use (FlashLedState_t) HAL_CAM_Action_param_st_t to set
    ACTION_CAM_SetFlashEnable,          ///< Set flash enable TRUE/FALSE, use (Boolean) HAL_CAM_Action_param_st_t to set

// Exposure Setting
    ACTION_CAM_ExposureSetting,         ///< Set Exposure, use HAL_CAM_Action_exposure_setting_st_t to set

// Focus Control
    ACTION_CAM_GetFocusStatus,          ///< Get Focus Status, use HAL_CAM_Action_focus_status_st_t to get
    ACTION_CAM_SetFocusRegion,          ///< Set Focus Region, use HAL_CAM_Action_focus_region_st_t to set
    ACTION_CAM_GetFocusControlConfig,   ///< Get Focus Control Configuration, use HAL_CAM_Action_focus_control_st_t to get
    ACTION_CAM_SetFocusControlConfig,   ///< Set Focus Control Configuration, use HAL_CAM_Action_focus_control_st_t to set

// Image Appearance Settings  These Actions are NOT for average user's use.
    ACTION_CAM_SetContrast,             ///< Set required contrast setting: [-100:0:100], use (Int8) HAL_CAM_Action_param_st_t to set
    ACTION_CAM_SetBrightness,           ///< Set required brightness setting: [-100:0:100], use (Int8) HAL_CAM_Action_param_st_t to set
    ACTION_CAM_SetSaturation,           ///< Set required saturation setting: [-100:0:100], use (Int8) HAL_CAM_Action_param_st_t to set
    ACTION_CAM_SetHue,                  ///< Set required hue setting: [-100:0:100], use (Int8) HAL_CAM_Action_param_st_t to set
    ACTION_CAM_SetGamma,                ///< Set required gamma setting: [-100:0:100], use (Int8) HAL_CAM_Action_param_st_t to set
    ACTION_CAM_SetSharpness,            ///< Set required sharpness setting: [-100:0:100], use (Int8) HAL_CAM_Action_param_st_t to set
    ACTION_CAM_SetAntiShadingPower,     ///< Set required anti shading power setting: [-100:0:100], use (Int8) HAL_CAM_Action_param_st_t to set

// Image Quality Settings   These Actions are NOT for average user's use.
    ACTION_CAM_SetImageQuality,         ///< Set the JPEG Quality (quantization) level [1-100]:  use (UInt8) HAL_CAM_Action_param_st_t to set
    ACTION_CAM_SetSceneMode,            ///< Set required Scene setting: use (CamSceneMode_t) HAL_CAM_Action_param_st_t to set
    ACTION_CAM_SetDigitalEffect,        ///< Set required Digital Effect: use (CamDigEffect_t) HAL_CAM_Action_param_st_t to set
    ACTION_CAM_SetFlickerControl,       ///< Set required Flicker setting: use (CamFlicker_t) HAL_CAM_Action_param_st_t to set
    ACTION_CAM_SetWBMode,               ///< Set required White Balance setting: use (CamWB_WBMode_t) HAL_CAM_Action_param_st_t to set
    ACTION_CAM_SetAutoExposure,         ///< Set required Exposure setting: use (CamExposure_t) HAL_CAM_Action_param_st_t to set
    ACTION_CAM_SetMeteringType,         ///< Set required Metering setting: use (CamMeteringType_t) HAL_CAM_Action_param_st_t to set
    ACTION_CAM_SetSensitivity,          ///< Set required Sensitiviy setting: use (CamSensitivity_t) HAL_CAM_Action_param_st_t to set
    ACTION_CAM_SetWideDynRangeEnable,   ///< Set Wide Dynamic Range enable TRUE/FALSE, use (Boolean) HAL_CAM_Action_param_st_t to set

// Image Orientation Settings
    ACTION_CAM_SetDisplayMode,                  ///< Set required display mode, use (CamDispMode_t) HAL_CAM_Action_param_st_t to set
    ACTION_CAM_SetRotationMode,         ///< Set required rotation mode, use (CamRotate_t) HAL_CAM_Action_param_st_t to set
    ACTION_CAM_GetRotationMode,         ///< Get required rotation mode, use (CamRotate_t) HAL_CAM_Action_param_st_t to get
    ACTION_CAM_SetMirrorMode,           ///< Set required mirror mode, use (CamMirror_t) HAL_CAM_Action_param_st_t to set
    ACTION_CAM_GetMirrorMode,           ///< Get required mirror mode, use (CamMirror_t) HAL_CAM_Action_param_st_t to get

   ACTION_CAM_SetESD_CB,					///< Set the ESD callback function
    ACTION_CAM_BRCM_MAX = 500,			///  Customer's action item after ACTION_CAM_BRCM_MAX
    ACTION_CAM_GetLuminance = 501,      ///< Get luminance data (0: normal, 1: middle, 2: low)
	ACTION_CAM_GetExposureTime = 502,
	ACTION_CAM_GetFNumber = 503,
	ACTION_CAM_GetISO = 504,
    ACTION_CAM_GetFocalLenth = 505,
	ACTION_CAM_LockAutoColor = 506,     ///< LockAuotColor multishot interval (0: AE, 1: AWB, 2: ALL)
	ACTION_CAM_UnlockAutoColor = 507,   ///< UnLockAuotColor multishot interval (0: AE, 1: AWB, 2: ALL)
	ACTION_CAM_GetEsdCheckResult = 508,
    ACTION_CAM_SetCaptureCallback = 509,
  ACTION_CAM_Action_N_Types             ///< bounds for error checking
} HAL_CAM_Action_en_t;

/**  HAL CAM driver function call result
*/
typedef enum
{
    HAL_CAM_SUCCESS,                        ///< Successful
    HAL_CAM_ERROR_ACTION_NOT_SUPPORTED,     ///<  Not supported
    HAL_CAM_ERROR_INTERNAL_ERROR,           ///< Internal error: i2c, comm failure, etc.
    HAL_CAM_ERROR_OTHERS                    ///< Undefined error
} HAL_CAM_Result_en_t;


/**  HAL CAM Trace Flags
*/
#define HAL_CAM_TRACE_PASSFAIL          0x0001 /**< Messages indicating the success or failure of a test*/
#define HAL_CAM_TRACE_FUNCTION          0x0002 /**< The name of each function */
#define HAL_CAM_TRACE_PARAMETERS        0x0004 /**< The parameters of each function */
#define HAL_CAM_TRACE_INFO              0x0008 /**< Generalized info. */
#define HAL_CAM_TRACE_ERROR             0x0010 /**< Errors that occur during processing. */
#define HAL_CAM_TRACE_WARNING           0x0020 /**< Warnings reported during processing. */
// Tracing Layer Enable
#define HAL_CAM_TRACE                   0x0100 /**< HAL layer tracing. */
#define HAL_CAM_TRACE_HALDRV            0x0200 /**< HAL Driver layer tracing. */
#define HAL_CAM_TRACE_DEVICE            0x0400 /**< Device Driver layer tracing. */
#define HAL_CAM_TRACE_CNTRL             0x0800 /**< Baseband Camera Controller layer tracing. */
#define HAL_CAM_TRACE_LAYERS            (HAL_CAM_TRACE|HAL_CAM_TRACE_HALDRV|HAL_CAM_TRACE_DEVICE|HAL_CAM_TRACE_CNTRL)



//******************************************************
//          Sensor Interface Configuration
//******************************************************
/// Sensor Image Output capabilities
typedef struct {
    UInt32 formats;                     ///< Output formats
    UInt32 max_width;                   ///< Maximum width resolution
    UInt32 max_height;                  ///< Maximum height resolution
    UInt32 ViewFinderResolutions;       ///< ViewFinder Resolutions     NOT CURRENTLY USED!!
    UInt32 VideoResolutions;            ///< Video Resolutions          NOT CURRENTLY USED!!
    UInt32 StillsResolutions;           ///< Stills Resolutions         NOT CURRENTLY USED!!
    UInt32 pre_frame_video;             ///< frames to throw out for ViewFinder/Video capture
    UInt32 pre_frame_still;             ///< frames to throw out for Stills capture
    Boolean JpegCapable;                ///< Sensor Jpeg Capable: TRUE/FALSE:
    Boolean StillnThumbCapable;         ///< Sensor Still and Thumbnail Capable: TRUE/FALSE:
    Boolean VideonViewfinderCapable;    ///< Sensor Video and Viewfinder Capable: TRUE/FALSE:
}HAL_CAM_SensorOutputCaps_st_t;

/// Sensor Focus Control Settings & Capabilities
typedef struct {
    CamFocusControlMode_t default_setting;  ///< default=CamFocusControlOff:  Values allowed  CamFocusControlMode_t
    CamFocusControlMode_t cur_setting;      ///< CamFocusControlMode_t
    UInt32 settings;                        ///< Settings Allowed: CamFocusControlMode_t bit masked
}HAL_CAM_FocusControl_st_t;

/// Sensor Digital Zoom Settings & Capabilities
typedef struct {
    CamZoom_t default_setting;          ///< default=CamZoom_1_0:  Values allowed  CamZoom_t
    CamZoom_t cur_setting;              ///< CamZoom_t
    CamZoom_t max_zoom;                 ///< Max Zoom parameter Allowed (256/max_zoom = *zoom) or 0 < CamZoom_t < 32 if Table look-up
    Boolean capable;                    ///< Sensor capable: TRUE/FALSE:
}HAL_CAM_DigitalZoomMode_st_t;

/// Sensor ESD Settings & Capabilities
typedef struct {
    UInt8 ESDTimer;                     ///< Periodic timer to retrieve the camera status (ms)
    Boolean capable;                    ///< Sensor capable: TRUE/FALSE:
}HAL_CAM_ESD_st_t;

//******************************************************
//
//          Sensor Image Configuration
//
//******************************************************
/// Sensor Scene Mode Settings
typedef struct {
    CamSceneMode_t default_setting;     ///< default=CamSceneMode_Auto:  Values allowed  CamSceneMode_t
    CamSceneMode_t cur_setting;         ///< CamSceneMode_t
    UInt32 settings;                    ///< Settings Allowed: bit mask
}CamSceneMode_st_t;
/// Sensor Digital Effects Settings
typedef struct {
    CamDigEffect_t default_setting;     ///< default=CamDigEffect_NoEffect:  Values allowed CamDigEffect_t
    CamDigEffect_t cur_setting;         ///< CamDigEffect_t
    UInt32 settings;                    ///< Settings Allowed: bit mask
}CamDigitalEffect_st_t;
/// Sensor Flicker Control Settings
typedef struct {
    CamFlicker_t default_setting;       ///< default=CamFlicker60Hz:  Values allowed CamFlicker_t
    CamFlicker_t cur_setting;           ///< CamFlicker_t
    UInt32 settings;                    ///< Settings Allowed: bit mask
}CamFlicker_st_t;
/// Sensor White balance Settings
typedef struct {
    CamWB_WBMode_t default_setting;     ///< default=0:  Values allowed  CamWB_WBMode_t
    CamWB_WBMode_t cur_setting;         ///< CamWB_WBMode_t
    UInt32 settings;                    ///< Settings Allowed: bit mask
}CamWBMode_st_t;
/// Sensor Exposure Settings
typedef struct {
    CamExposure_t default_setting;      ///< default=0:  Values allowed  CamExposure_t
    CamExposure_t cur_setting;          ///< CamExposure_t
    UInt32 settings;                    ///< Settings Allowed: bit mask
}CamExposure_st_t;
/// Sensor Metering  Settings
typedef struct {                        ///< Metering settings
    CamMeteringType_t default_setting;  ///< default=0:  Values allowed  CamMeteringType_t
    CamMeteringType_t cur_setting;      ///< CamMeteringType_t
    UInt32 settings;                    ///< Settings Allowed: bit mask
}CamMeteringType_st_t;
/// Sensor Sensitivity Settings
typedef struct {                        ///< Sensitivity settings
    CamSensitivity_t default_setting;   ///< default=0:  Values allowed  CamSensitivity_t
    CamSensitivity_t cur_setting;       ///< CamSensitivity_t
    UInt32 settings;                    ///< Settings Allowed: bit mask
}CamSensitivity_st_t;

/// Sensor Frame Rate Settings
typedef struct {
    CamRates_t default_setting;         ///< CamRates_t
    CamRates_t cur_setting;             ///< CamRates_t
    CamRates_t max_setting;             ///< max allowed
}CamFrameRate_st_t;
/// Sensor Flash Settings
typedef struct {                        ///< Flash settings
    FlashLedState_t default_setting;    ///< default=0:  Values allowed  FlashLedState_t
    FlashLedState_t cur_setting;        ///< FlashLedState_t
    UInt32 settings;                    ///< Settings Allowed: bit mask
}CamFlashLedState_st_t;
/// Sensor Rotate Settings
typedef struct {                        ///< Rotate settings
    CamRotate_t default_setting;        ///< default=0:  Values allowed CamRotate_t
    CamRotate_t cur_setting;            ///< CamRotate_t
    UInt32 settings;                    ///< Settings Allowed: CamRotate_t bit masked
}CamRotateMode_st_t;
/// Sensor Mirror Settings
typedef struct {                        ///< Mirror settings
    CamMirror_t default_setting;        ///< default=0:  Values allowed  CamMirror_t
    CamMirror_t cur_setting;            ///< CamMirror_t
    UInt32 settings;                    ///< Settings Allowed: CamMirror_t bit masked
}CamMirrorMode_st_t;
/// Sensor Jpeg Quality Settings
typedef struct {                        ///< Jpeg Quaility settings
    UInt8 default_setting;              ///< Values allowed  1 to 100
    UInt8 cur_setting;                  ///< Values allowed  1 to 100
    Boolean configurable;               ///< Sensor configurable: TRUE/FALSE:
}CamJpegQuality_st_t;

#if defined(SS_2153) && defined(BROOKLYN_HW)
typedef struct {
	CamFocusControlMode_t default_setting;
	CamFocusControlMode_t cur_setting;
	UInt32 settings;   ///< Settings Allowed: bit masked
}CamFocusMode_st_t;
#endif

// [BEGIN add BRCM_FIX <CL#190940/CQ#MobC00060539/CSP#201519> (part 7 / 11)]
/** Sensor Function Enable Setting*/
typedef struct {                        ///< Wide Dynamic Range setting
    Boolean default_setting;            ///< default=FALSE:  Values allowed  Boolean
    Boolean cur_setting;                ///< Boolean
    Boolean configurable;               ///< Sensor configurable: TRUE/FALSE:
}CamFunctionEnable_st_t;

/// Sensor Image Appearance Settings:  Contrast, Brightness, Saturation, Hue, Gamma, Sharpness, AntiShadingPower
typedef struct {                        ///< Contrast, Brightness, Saturation, Hue, Gamma, Sharpness, AntiShadingPower
    Int8 default_setting;               ///< default=0:  Values allowed  -100 to 100, zero means no change
    Int8 cur_setting;                   ///< default=0:  Values allowed  -100 to 100, zero means no change
    Boolean configurable;               ///< Sensor configurable: TRUE/FALSE:
}CamImageAppearance_st_t;

/// Camera Exposure Setting
typedef struct {
    CamMeteringType_t   exp_meter;                  ///< (in) metering type
    CamSensitivity_t    exp_iso;                    ///< (in) iso speed
    Int32               exp_ev_compensation;        ///< (in) ev compensation value
    UInt32              exp_aperture_number;        ///< (in) aperture number
    UInt32              exp_shutter_speed_msec;     ///< (in) shutter speed
    UInt8               exp_auto_aperture;          ///< (in) auto aperture
    UInt8               exp_auto_shutterspeed;      ///< (in) auto shutter speed
}HAL_CAM_ExposureSetting_st_t;

/// Sensor Configuration and Capabilities
typedef struct {
    HAL_CAM_SensorOutputCaps_st_t   output_st;              ///< Image Output capabilities
    HAL_CAM_FocusControl_st_t       focus_control_st;       ///< Focus Settings & Capabilities
    HAL_CAM_DigitalZoomMode_st_t    digital_zoom_st;        ///< Digital Zoom Settings & Capabilities
    HAL_CAM_ESD_st_t                esd_st;                 ///< Sensor ESD Settings & Capabilities
    UInt8 *                         pSensor_version;        ///< Sensor version string
} HAL_CAM_IntConfigCaps_st_t;

/// Sensor Image Quality/Appearance Configuration, Settings, & Capabilities
typedef struct {
    CamSceneMode_st_t           *sensor_scene;              ///< Scene Mode Setting & Capabilities
    CamDigitalEffect_st_t       *sensor_digitaleffect;      ///< Digital Effects Setting & Capabilities
    CamFlicker_st_t             *sensor_flicker;            ///< Flicker Control Setting & Capabilities
    CamWBMode_st_t              *sensor_wb;                 ///< White Balance Setting & Capabilities
    CamExposure_st_t            *sensor_exposure;           ///< Exposure Setting & Capabilities
    CamMeteringType_st_t        *sensor_metering;           ///< Metering Setting & Capabilities
    CamSensitivity_st_t         *sensor_sensitivity;        ///< Sensitivity Setting & Capabilities
    CamFunctionEnable_st_t      *sensor_wdr;                ///< Wide Dynamic Range Setting & Capabilities

    CamFrameRate_st_t           *sensor_framerate;          ///< Frame Rate Output Settings & Capabilities
    CamFlashLedState_st_t       *sensor_flashstate;         ///< Flash Setting & Capabilities
    CamRotateMode_st_t          *sensor_rotatemode;         ///< Rotation Setting & Capabilities
    CamMirrorMode_st_t          *sensor_mirrormode;         ///< Mirror Setting & Capabilities
    CamJpegQuality_st_t         *sensor_jpegQuality;        ///< Jpeg Quality Setting & Capabilities:  Values allowed 1 to 100
    CamFunctionEnable_st_t      *sensor_framestab;          ///< Frame Stabilization Setting & Capabilities
    CamFunctionEnable_st_t      *sensor_antishake;          ///< Anti-Shake Setting & Capabilities
    CamFunctionEnable_st_t      *sensor_facedetect;         ///< Face Detection Setting & Capabilities
    CamFunctionEnable_st_t      *sensor_autofocus;          ///< Auto Focus Setting & Capabilities

    CamImageAppearance_st_t     *sensor_contrast;           ///< Contrast   default=0:  Values allowed  -100 to 100, zero means no change
    CamImageAppearance_st_t     *sensor_brightness;         ///< Brightness default=0:  Values allowed  0=All black  100=All white
    CamImageAppearance_st_t     *sensor_saturation;         ///< Saturation default=0:  Values allowed  -100 to 100, zero means no change
    CamImageAppearance_st_t     *sensor_hue;                ///< Hue        default=0:  Values allowed  -100 to 100, zero means no change
    CamImageAppearance_st_t     *sensor_gamma;              ///< Gamma      default=0:  Values allowed  -100 to 100, zero means no change
    CamImageAppearance_st_t     *sensor_sharpness;          ///< Sharpness  default=0:  Values allowed  -100 to 100, zero means no change
    CamImageAppearance_st_t     *sensor_antishadingpower;   ///< AntiShading    default=0:  Values allowed  -100 to 100, zero means no change
#if defined(SS_2153) && defined(BROOKLYN_HW)
	CamFocusMode_st_t     		*sensor_autofocusmode;
#endif
} CamSensorImageConfig_st_t;
//******************************************************
//******************************************************

/// Sensor Capabilities and Image Configuration
typedef struct {
    HAL_CAM_IntConfigCaps_st_t      *sensor_config_caps;
    CamSensorImageConfig_st_t       *sensor_image_config;
} HAL_CAM_CamConfig_st_t;

/// Camera Sensor Resolution Size Structure
typedef struct
{
    CamImageSize_t  size;               ///< camera sensor image output size
    UInt32          resX;               ///< X resolution of camera sensor
    UInt32          resY;               ///< Y resolution of Camera sensor
} HAL_CAM_ResolutionSize_st_t;

/// Camera Sensor Rectangle Definition
typedef struct
{
		UInt8					face_select;				///< face selection
        UInt32                  start_x;                ///< x start    (pixel)
        UInt32                  start_y;                ///< y start    (line)
        UInt32                  end_x;                  ///< x stop     (pixel)
        UInt32                  end_y;                  ///< y end      (line)
} HAL_CAM_Rectangle_st_t;

/// Camera Focus Control Configuration
typedef struct
{
    CamFocusControlMode_t   focus_ctrl_mode;        ///< focus control mode
    UInt32                  focus_ctrl_num_steps;   ///< number of focus control steps
    UInt32                  focus_current_step;     ///< the current step
} HAL_CAM_Focus_Control_Config_st_t;

/// Camera Focus Region Configuration Structure
typedef struct
{
    UInt8   focus_center_enabled;                   ///< set focus to center region
    UInt8   focus_left_enabled;                     ///< set focus to left region
    UInt8   focus_right_enabled;                    ///< set focus to right region
    UInt8   focus_top_enabled;                      ///< set focus to top region
    UInt8   focus_bottom_enabled;                   ///< set focus to bottom region
    UInt8   focus_topleft_enabled;                  ///< set focus to top left region
    UInt8   focus_topright_enabled;                 ///< set focus to top right region
    UInt8   focus_bottomleft_enabled;               ///< set focus to bottom left region
    UInt8   focus_bottomright_enabled;              ///< set focus to bottom right region
} HAL_CAM_Focus_Region_st_t;


/// HAL CAM Action:  Get Camera Settings
typedef struct
{
    CamSensorSelect_t       sensor;                         ///< (in) camera sensor select
    CamCaptureMode_t        mode;                           ///< (in) camera sensor select
    HAL_CAM_CamConfig_st_t  *cam_config_st;                 ///< (out) default camera settings structure
} HAL_CAM_Action_settings_st_t;

/// HAL CAM Device Configuration Structure
typedef struct
{
    HAL_CAM_CamConfig_st_t  *cam_config_st;             ///< (in) default camera settings structure
    Boolean                 device_ACTIVE;              ///< mandatory value  UNUSED!!
    UInt32                  performance_required;       ///< mandatory value  UNUSED!!
} HAL_CAM_config_st_t;


/// HAL CAM Power Mode Structure
typedef struct
{
  CamSensorSelect_t         sensor;                     ///< (in) camera sensor select
  CamCaptureMode_t          mode;                       ///< (in) camera interface and sensor mode (CamPowerOff/CamPowerOn)
} HAL_CAM_Action_power_mode_st_t;

/// HAL CAM Action:  Camera Control Interface
typedef struct
{
        CamSensorSelect_t       sensor;                 ///< (in) camera sensor select
        UInt32                  i2c_dev_id;             ///< (out) I2C device id
        UInt32                  i2c_max_page;           ///< (out) I2C max page
        UInt32                  i2c_sub_addr_op;        ///< (out) I2C sub-addr size
        UInt32                  controller_config;      ///< (out) camera controller configuration
} HAL_CAM_Action_cam_interface_st_t;

/// HAL CAM Action:  Camera Display Window Size
typedef struct
{
        UInt32                  width;                  ///< (in/out) width
        UInt32                  height;                 ///< (in/out) height
        CamSensorSelect_t       sensor;                 ///< (in) camera sensor select
} HAL_CAM_Action_window_st_t;

/// HAL CAM Action:  Camera Window and Scale Config
typedef struct
{
        CamImageSize_t          size;                   ///< (in) image size to window & scale
        UInt32                  start_pixel;            ///< (in) start pixel (x start)
        UInt32                  end_pixel;              ///< (in) stop pixel  (x stop)
        UInt32                  start_line;             ///< (in) start line  (y start)
        UInt32                  end_line;               ///< (in) end line    (y stop)
        UInt32                  horizontal_SDF;         ///< (in) horizontal scale factor
        UInt32                  vertical_SDF;           ///< (in) vertical scale factor
} HAL_CAM_Action_window_scale_st_t;

#if defined(SS_2153) && defined(BROOKLYN_HW)
typedef struct
{
        UInt8                   face_select;            ///< (in) n face selection
        HAL_CAM_Rectangle_st_t* rect_st;                ///< (in/out) rectangle structure pointer
        CamSensorSelect_t       sensor;                 ///< (in) camera sensor select
} HAL_CAM_Action_face_detect_st_t;
#endif

/// HAL CAM Action:  Camera Image Size Available
typedef struct
{
        UInt32                  width;                  ///< (in) width
        UInt32                  height;                 ///< (in) height
        CamCaptureMode_t        mode;                   ///< (in) capture mode
        CamSensorSelect_t       sensor;                 ///< (in) camera sensor select
        HAL_CAM_ResolutionSize_st_t* sensor_size;       ///< (out) image size available
} HAL_CAM_Action_image_size_st_t;

/// HAL CAM Action:  Camera Resolution Request
typedef struct
{
        CamImageSize_t          size;                   ///< (in) image size
        CamCaptureMode_t        mode;                   ///< (in) capture mode
        CamSensorSelect_t       sensor;                 ///< (in) camera sensor select
        HAL_CAM_ResolutionSize_st_t* sensor_size;       ///< (out) Sensor resolution for requested size
} HAL_CAM_Action_resolution_st_t;

/// HAL CAM Action:  Capture Video
typedef struct
{
    CamImageSize_t              size;           ///< (in) capture size
    CamDataFmt_t                format;         ///< (in) capture format
    CamSensorSelect_t           sensor;         ///< (in) camera sensor select
    UInt8*                      buf0;           ///< (in) captured image pingpong buffer 0
    UInt8*                      buf1;           ///< (in) captured image pingpong buffer 1
} HAL_CAM_Action_capture_video_st_t;

/// HAL CAM Action:  Preview/Video Capture with Post Process Zoom
typedef struct
{
  HAL_CAM_Action_window_scale_st_t *window_scale;  ///< (in) captured image window
  CamDataFmt_t                      format;     ///< (in) capture format
  CamSensorSelect_t                 sensor;     ///< (in) camera sensor select
  FlashLedState_t                   flash_mode; ///< (in) image capture flash mode
  CamCaptureMode_t                  mode;       ///< (in) image capture mode (Unused)
  UInt8*                            buf0;       ///< (in) captured image pingpong buffer 0
  UInt8*                            buf1;       ///< (in) captured image pingpong buffer 1
} HAL_CAM_Action_preview_PPzoom_st_t;

/// HAL CAM Action:  Capture Image with Thumbnail
typedef struct
{
        CamImageSize_t          size;               ///< (in) Still image size
        CamDataFmt_t            format;             ///< (in) Still image format
        CamSensorSelect_t       sensor;             ///< (in) camera sensor select
        UInt8*                  buf;                ///< (in) Still image capture buffer
        UInt32                  thumbnail_width;    ///< (in) thumbnail width
        UInt32                  thumbnail_height;   ///< (in) thumbnail height
        CamDataFmt_t            thumbnail_format;   ///< (in) thumbnail format
        UInt8*                  thumbnail_buf;      ///< (in) thumbnail image capture buffer
} HAL_CAM_Action_capture_image_with_thumbnail_st_t;

/// HAL CAM Action:  Capture Video with Viewfinder
typedef struct
{
        CamImageSize_t          size;               ///< (in) capture size
        CamDataFmt_t            format;             ///< (in) capture format
        CamSensorSelect_t       sensor;             ///< (in) camera sensor select
        UInt8*                  buf0;               ///< (in) captured image pingpong buffer 0
        UInt8*                  buf1;               ///< (in) captured image pingpong buffer 1
        CamImageSize_t          viewfinder_size;    ///< (in) viewfinder size
        CamDataFmt_t            viewfinder_format;  ///< (in) viewfinder format
        UInt8*                  viewfinder_buf0;    ///< (in) viewfinder image pingpong buffer 0
        UInt8*                  viewfinder_buf1;    ///< (in) viewfinder image pingpong buffer 1
} HAL_CAM_Action_capture_video_with_viewfinder_st_t;

/// HAL CAM Action:  Capture Stills Image
typedef struct
{
        CamImageSize_t          size;                   ///< (in) capture size
        CamDataFmt_t            format;                 ///< (in) capture format
        CamSensorSelect_t       sensor;                 ///< (in) camera sensor select
        UInt8*                  buf;                    ///< (in) captured image buffer
} HAL_CAM_Action_capture_image_st_t;

#if defined(SS_2153) && defined(BROOKLYN_HW)
typedef struct
{
    CamImageSize_t                  size;       ///< (in) capture size
    CamDataFmt_t                    format;     ///< (in) capture format
    CamCaptureMode_t                mode;       ///< (in) image capture mode
    CamSensorSelect_t               sensor;     ///< (in) camera sensor select
    UInt8*                          buf0;       ///< (in) captured image pingpong buffer 0
    UInt8*                          buf1;       ///< (in) captured image pingpong buffer 1
} HAL_CAM_Action_capture_st_t;
#endif

/// HAL CAM Action:  Stills Capture with Post Process Zoom
typedef struct
{
  HAL_CAM_Action_window_scale_st_t  *window_scale;  ///< (in) captured image window
  CamDataFmt_t                      format;			///< (in) capture format
  CamSensorSelect_t                 sensor;			///< (in) camera sensor select
  FlashLedState_t                   flash_mode;		///< (in) image capture flash mode
  UInt8*                            buf;			///< (in) captured image buffer
} HAL_CAM_Action_stills_PPzoom_st_t;


/// HAL CAM Action:  Exposure Setting
typedef struct
{
    HAL_CAM_ExposureSetting_st_t    *exp_setting;               ///< (in) exp setting struct
    CamSensorSelect_t               sensor;                     ///< (in) camera sensor select
} HAL_CAM_Action_exposure_setting_st_t;


/// HAL CAM Action:  Focus Status
typedef struct
{
    CamFocusStatus_t                *focus_status_ptr;              ///< (out) focus status
    HAL_CAM_Focus_Region_st_t       *focus_region_ptr;              ///< (out) focus region setting
    CamSensorSelect_t               sensor;                         ///< (in) camera sensor select
} HAL_CAM_Action_focus_status_st_t;

/// HAL CAM Action:  Focus Region
typedef struct
{
    HAL_CAM_Focus_Region_st_t       *focus_region_ptr;              ///< (in) focus region setting
    CamSensorSelect_t               sensor;                         ///< (in) camera sensor select
} HAL_CAM_Action_focus_region_st_t;

/// HAL CAM Action:  Focus Control
typedef struct
{
    HAL_CAM_Focus_Control_Config_st_t   *focus_control_ptr;         ///< (in/out) focus control configuration
    CamSensorSelect_t                   sensor;                     ///< (in) camera sensor select
} HAL_CAM_Action_focus_control_st_t;

/// HAL CAM Action:  Parameter
typedef struct
{
    UInt32                          param;                          ///< (in/out) parameter
    CamSensorSelect_t               sensor;                         ///< (in) camera sensor select
} HAL_CAM_Action_param_st_t;

/// HAL LCD Action Structure Union:
typedef union
{
    HAL_CAM_Action_power_mode_st_t                          HAL_CAM_Action_power_mode;          ///< HAL_CAM_Action_power_mode_st_t
    HAL_CAM_Action_settings_st_t                            HAL_CAM_Action_settings;            ///< HAL_CAM_Action_settings_st_t
    HAL_CAM_Action_cam_interface_st_t                       HAL_CAM_Action_cam_interface;       ///< HAL_CAM_Action_cam_interface_st_t
    HAL_CAM_Action_image_size_st_t                          HAL_CAM_Action_image_size;          ///< HAL_CAM_Action_image_size_t
    HAL_CAM_Action_resolution_st_t                          HAL_CAM_Action_resolution;          ///< HAL_CAM_Action_resolution_st_t
    HAL_CAM_Action_capture_image_st_t                       HAL_CAM_Action_capture_image;       ///< HAL_CAM_Action_capture_st_t
    HAL_CAM_Action_capture_video_st_t                       HAL_CAM_Action_capture_video;       ///< HAL_CAM_Action_capture_st_t
    HAL_CAM_Action_preview_PPzoom_st_t                      HAL_CAM_Action_preview_PPzoom;      ///< HAL_CAM_Action_preview_PPzoom_st_t
    HAL_CAM_Action_stills_PPzoom_st_t                       HAL_CAM_Action_stills_PPzoom;       ///< HAL_CAM_Action_stills_PPzoom_st_t
    HAL_CAM_Action_capture_image_with_thumbnail_st_t        HAL_CAM_Action_capture_image_with_thumbnail;    ///<
    HAL_CAM_Action_capture_video_with_viewfinder_st_t       HAL_CAM_Action_capture_video_with_viewfinder;   ///<
    HAL_CAM_Action_window_scale_st_t                        HAL_CAM_Action_window_scale;        ///< HAL_CAM_Action_window_scale_st_t
#if defined(SS_2153) && defined(BROOKLYN_HW)
    HAL_CAM_Action_face_detect_st_t                         HAL_CAM_Action_face_detect;         ///< HAL_CAM_Action_face_detect_st_t
#endif
    HAL_CAM_Action_window_st_t                              HAL_CAM_Action_window;              ///< HAL_CAM_Action_window_st_t
    HAL_CAM_Action_exposure_setting_st_t                    HAL_CAM_Action_exposure_setting;    ///< HAL_CAM_Action_exposure_setting_st_t
    HAL_CAM_Action_focus_status_st_t                        HAL_CAM_Action_focus_status;        ///< HAL_CAM_Action_focus_status_st_t
    HAL_CAM_Action_focus_control_st_t                       HAL_CAM_Action_focus_control;       ///< HAL_CAM_Action_focus_control_st_t
    HAL_CAM_Action_focus_region_st_t                        HAL_CAM_Action_focus_region;        ///< HAL_CAM_Action_focus_region_st_t
    HAL_CAM_Action_param_st_t                               HAL_CAM_Action_param;               ///< HAL_CAM_Action_param_st_t
} HAL_CAM_Control_un_t;

//! Callback Function Prototypes
//typedef void (*CamCaptureCB_t)(Result_t result, UInt32 buf_ptr, UInt32 resolution_x, UInt32 resolution_y, UInt32 byte_count, UInt32 preview_ptr, UInt32 res_x_preview, UInt32 res_y_preview, UInt32 byte_count_preview);

/**
 * @addtogroup HALCamDrvGroup
 * @{
 */

/**
*  This function will initialize Camera Driver.  Initialize to default settings, set HAL_CAM_cfg_specific_st_t* == NULL.
*  To change default settings use ACTION_CAM_GetCameraSettings to get a copy of default camera settings,
*  Modify default settings in HAL_CAM_Action_settings_st_tand pass back in HAL_CAM_Init(HAL_LCD_config_st* HAL_LCD_config)
*  driver will copy new settings to local and initialize Camera.
*/
HAL_CAM_Result_en_t  HAL_CAM_Init(
        CamSensorSelect_t       sensor_select,              ///< (in) Select Camera Sensor
        HAL_CAM_config_st_t*    config                      ///< (in/out) device configuration structure, if NULL use default settings
        );

/**
*  This function will perform specific action from defined list, copy of parameters passed thru parm structure.
*
*/
HAL_CAM_Result_en_t HAL_CAM_Ctrl(
        HAL_CAM_Action_en_t action,                         ///< (in) device action
        void*                   param,                      ///< (in/out) structure pointer to additional parameters
        void*                   callback                    ///< (in) callback function device driver calls once action complete, NULL if not needed
        );

/**
*	HAL_CAM_Set_TraceLevel will use this global trace level and its nTraceFlags parameter to enable/disable output
*
*	@param nTraceLevel  ORed trace flags defined in hal_cam.h, use 0xFFFFFFFF to enable output always
*
*	@retval	 original trace level
*/
UInt32 HAL_CAM_Set_TraceLevel(UInt32 nTraceLevel);

/** Output a trace message */
HAL_CAM_Result_en_t HAL_CAM_Trace(UInt32 nTraceFlags, char *format, ...);

/** @} */

#endif  // _HAL_CAM_H__



