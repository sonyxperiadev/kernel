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
*   @defgroup   CamDevDrvGroup   Camera Device Driver
*   @brief      This group contains Camera Specific API's.
*	@ingroup	DeviceDriverGroup
*	@ingroup	CamDrvGroup
*
****************************************************************************/

#ifndef _CAMDRV_DEV_H_
#define _CAMDRV_DEV_H_

// ---- Include Files -------------------------------------------------------
#include <linux/types.h>
#include <linux/broadcom/types.h>
#include "hal_cam.h"

#if defined CONFIG_BCM_CAM_OV2655
#include "camdrv_ov2655.h"
#else
#include "camdrv_stv0986.h"
#endif



/** Max allowed GRAM size. This is to accomodate different LCD's */
#define MAX_CAM_FRAME           (CAM_WIDTH_PIXEL*CAM_HEIGHT_PIXEL)      ///< main LCD, QVGA


// ---- Public Constants and Types  ----------------------------------------------------
// Camera Initialization Parameters
/** Delay time for cam init to take place */
#define CAM_INIT_DELAY      0xFE
/** Cam init done*/
#define CAM_INIT_DONE           0xFF
/** register device id*/
#define CAM_REG_DEVID     0x00    
/** total number of Camera Sensor registers*/
#define CAM_REGISTER_CNT    (0x00+1)    

/** Camera control Structure */
typedef struct
{
    UInt8       reg;    ///< register
    UInt8       cmd;    ///< command
    Boolean     SetAtInit;  ///< init
} InitElement_t;

// ---- Public Variables ----------------------------------------------------
// ---- Functions -----------------------------------------------------------

/**
 * @addtogroup CamDevDrvGroup
 * @{
 */

/**
*  This function will perform specific action from defined list, copy of parameters passed thru parm structure.
*
*/
HAL_CAM_Result_en_t CAMDRV_ActionCtrl(
        HAL_CAM_Action_en_t action,                         
        void*               param,                      
        void*               callback                    
        );  

/** The CAMDRV_GetIntfSeqSel returns the pointer to and length of the Interface Power-up/Down
        sequence structure requested.  Controls GPIO and Clock to Selected sensor.

    @param [in] nSensor
        Sensor for which sequence is required.
    @param [in] nSeqSel
        Sequence Selection for which power-up/down sequence is required.
    @param [out] pLength
        pointer to length of sequence returned. 
    @return CamSensorIntfCntrl_st_t
        Selected Sequence returned.
 */
CamSensorIntfCntrl_st_t* CAMDRV_GetIntfSeqSel(
    CamSensorSelect_t nSensor,
    CamSensorSeqSel_t nSeqSel, 
    UInt32 *pLength);

/** The CAMDRV_GetIntfConfig returns the pointer to the Interface Configuration
        structure for the sensor requested.  Control Info for CMI, I2C, IOCR, and Sensor interface.

    @param [in] nSensor
        Sensor for which interface configuration is required.
    @return CamIntfConfig_st_t
        Selected interface configuration returned.
 */
CamIntfConfig_st_t* CAMDRV_GetIntfConfig(
    CamSensorSelect_t nSensor);

/** The CAMDRV_GetImageSettings returns the pointer to the Current Image Settings Configuration
        structure for the sensor requested.  Image Settings for scene,effects,flicker,wb,exposure,metering,sensitivity,flash,contrast,....
    @param [in] sensor
        Sensor for which image settings is required (unused).
    @param [in] mode
        Capture mode for image settings (unused).
    
    @return CamSensorImageConfig_st_t
        Selected Image Settings returned.
 */
CamSensorImageConfig_st_t* CAMDRV_GetImageSettings(
        CamSensorSelect_t sensor, 
        CamCaptureMode_t mode);

/** The CAMDRV_GetResolution returns the sensor output size of the image resolution requested
    @param [in] size
        image size requested from Sensor.
    @param [in] mode
        Capture mode for resolution requested.
    @param [in] sensor
        Sensor for which resolution is requested.
    @param [out] *sensor_size
        Actual size of requested resolution.
    
    @return HAL_CAM_Result_en_t
        status returned.
 */
HAL_CAM_Result_en_t CAMDRV_GetResolution( 
        CamImageSize_t size, 
        CamCaptureMode_t mode, 
        CamSensorSelect_t sensor,
        HAL_CAM_ResolutionSize_st_t *sensor_size );

/** The CAMDRV_GetResolutionAvailable returns the pointer to the sensor output size of the 
    image width and height requested
    @param [in] width
        width of resolution requested from Sensor.
    @param [in] height
        height of resolution requested from Sensor.
    @param [in] mode
        Capture mode for resolution requested.
    @param [in] sensor
        Sensor for which resolution is requested.
    @param [out] *sensor_size
        Actual size of requested resolution.
    
    @return HAL_CAM_Result_en_t
        status returned.
 */
HAL_CAM_Result_en_t CAMDRV_GetResolutionAvailable( 
        UInt32 width, 
        UInt32 height, 
        CamCaptureMode_t mode, 
        CamSensorSelect_t sensor,
        HAL_CAM_ResolutionSize_st_t *sensor_size );

/** Get Image Resolution from requested width and height
*  @param   width [in] of image
*  @param   height  [in] of image
*  @return  CamImageSize_t size
*/
CamImageSize_t CAMDRV_GetResAvailable(
        UInt16 width,                                               //< (in) width of image
        UInt16 height                                               //< (in) height of image
        );
/** Set Frame Rate.
*  @param fps [in] frames per second.
*  @param nSensor [in]
*  @return HAL_CAM_Result_en_t 
*/
HAL_CAM_Result_en_t CAMDRV_SetFrameRate(
        CamRates_t fps,                                             //< (in) frames per second
        CamSensorSelect_t nSensor
        );

/** Get Maximum Horizontal Pixels for Camera Sensor
*  @param nSensor [in]
*  @return      UInt16 width
*/
UInt16 CAMDRV_GetMaxResolution_Hpixels(
        CamSensorSelect_t nSensor
        );

/** Get Maximum Vertical Pixels for Camera Sensor
*  @param nSensor [in]      
*  @return      UInt16 height
*/
UInt16 CAMDRV_GetMaxResolution_Vpixels(
        CamSensorSelect_t nSensor
        );

/** Camera sensor zoom
*  @param step   [in] set zoom steps.
*  @param nSensor [in]
*  @return      HAL_CAM_Result_en_t
*  @note     done with camera sensor.
*/

HAL_CAM_Result_en_t CAMDRV_SetZoom(CamZoom_t step,
					  CamSensorSelect_t sensor);


/** Set flash mode for camera Sensor
    @param  flash   [in] Flash mode
    @param  sensor              [in] .
    @note   TORCH is less brighter than FLASH.
            In the dark, torch is used for preview and flash for capture.
*/
HAL_CAM_Result_en_t CAMDRV_SetFlashMode(
        FlashLedState_t flash,
        CamSensorSelect_t sensor
        );

/** Set flash mode for camera Sensor
    @param  flash   [in] Flash On/off
    @param  nSensor [in] Sensor select.
    @note   TORCH is less brighter than FLASH.
            In the dark, torch is used for preview and flash for capture.
*/
HAL_CAM_Result_en_t CAMDRV_DoFlash(
        UInt32 flash,
        CamSensorSelect_t nSensor
        );
/** Set Frame Stabilization Enable camera Sensor
    @param  pStab               [in] Data address of Enable/Disable Stabilization
    @param  sensor              [in] Sensor Select.
    @return       HAL_CAM_Result_en_t 
    @note   
*/
#if defined(SS_2153) && defined(BROOKLYN_HW)
HAL_CAM_Result_en_t CAMDRV_SetFrameStabEnable(
        Boolean bStab,
        CamSensorSelect_t sensor
        );
#else
HAL_CAM_Result_en_t CAMDRV_SetFrameStabEnable(
        UInt32 pStab,
        CamSensorSelect_t sensor
        );
#endif
/** Set Anti-Shake Enable camera Sensor
    @param  pShake              [in] Data address of Enable/Disable Anti-Shake
    @param  sensor              [in] Sensor Select.
    @return       HAL_CAM_Result_en_t 
    @note   
*/
#if defined(SS_2153) && defined(BROOKLYN_HW)
HAL_CAM_Result_en_t CAMDRV_SetAntiShakeEnable(
        Boolean bShake,
        CamSensorSelect_t sensor
        );
#else
HAL_CAM_Result_en_t CAMDRV_SetAntiShakeEnable(
        UInt32 pShake,
        CamSensorSelect_t sensor
        );
#endif

/** Set AutoFocus Enable on camera Sensor
    @param  pAutoFocus          [in] Data address of Enable/Disable auto focus
    @param  sensor              [in] Sensor Select.
    @return       HAL_CAM_Result_en_t 
    @note   
*/
#if defined(SS_2153) && defined(BROOKLYN_HW)
HAL_CAM_Result_en_t CAMDRV_SetAutoFocusEnable( 
		Boolean bAutoFocus, 
		CamSensorSelect_t sensor 
		);
#else
HAL_CAM_Result_en_t CAMDRV_SetAutoFocusEnable( 
		UInt32 pAutoFocus, 
		CamSensorSelect_t sensor 
		);
#endif

/** Set Face Detection Enable on camera Sensor
    @param  pDetect             [in] Data address of Enable/Disable Face Detection
    @param  sensor              [in] Sensor Select.
    @return HAL_CAM_Result_en_t 
    @note   
*/
#if defined(SS_2153) && defined(BROOKLYN_HW)
HAL_CAM_Result_en_t CAMDRV_SetFaceDetectEnable(
        Boolean bDetect, 
        CamSensorSelect_t sensor 
        );
#else
HAL_CAM_Result_en_t CAMDRV_SetFaceDetectEnable(
        UInt32 pDetect, 
        CamSensorSelect_t sensor 
        );
#endif
/** Get Face Detection Rectangle from camera Sensor
    @param  p_rect_st           [in] Rectangle structure pointer.
    @param  sensor              [in] Sensor Select.
    @return       HAL_CAM_Result_en_t 
    @note   
*/
#if defined(SS_2153) && defined(BROOKLYN_HW)
HAL_CAM_Result_en_t CAMDRV_GetFaceDetectRect(
        UInt8 face_select,
        HAL_CAM_Rectangle_st_t* rect_st, 
        CamSensorSelect_t sensor 
        );
#else
HAL_CAM_Result_en_t CAMDRV_GetFaceDetectRect(
        UInt32 p_rect_st, 
        CamSensorSelect_t sensor 
        );
#endif
        
/** Set rotation mode for camera Sensor
    @param  mode    [in] Rotation mode
    @param  sensor              [in] .
    @note   Rotate input image clockwise
*/
HAL_CAM_Result_en_t CAMDRV_SetRotationMode(
        CamRotate_t mode,
        CamSensorSelect_t sensor
        );
/** Get rotation mode for camera Sensor
    @param  sensor              [in] .
    @return CamRotate_t
    @note   Rotate input image clockwise
*/
CamRotate_t CAMDRV_GetRotationMode(
        CamSensorSelect_t sensor
        );

/** Set mirror mode for camera Sensor
    @param  mode    [in] Mirror mode
    @param  sensor              [in] .
    @note   Rotate input image clockwise
*/
HAL_CAM_Result_en_t CAMDRV_SetMirrorMode(
        CamMirror_t mode,
        CamSensorSelect_t sensor
        );
/** Get mirror mode for camera Sensor
    @param  sensor              [in] .
    @return CamMirror_t
    @note   Rotate input image clockwise
*/
CamMirror_t CAMDRV_GetMirrorMode(
        CamSensorSelect_t sensor
        );

/** Set exposure setting for camera Sensor
    @param  *exposure_setting   [in] exposure setting
    @param  sensor              [in] .
    @note   TORCH is less brighter than FLASH.
            In the dark, torch is used for preview and flash for capture.
*/
HAL_CAM_Result_en_t CAMDRV_SetExpSetting(
        HAL_CAM_ExposureSetting_st_t* exposure_setting,
        CamSensorSelect_t sensor
        );

/** Get Focus Status for camera Sensor
    @param  *pfocus_status [out] focus status
    @param  *pfocus_region [out] focus region setting 
    @param  sensor         [in] camera sensor select
    @note   none
*/
HAL_CAM_Result_en_t CAMDRV_GetFocusStatus(
        CamFocusStatus_t*           pfocus_status,              //< (out) focus status
        HAL_CAM_Focus_Region_st_t*  pfocus_region,              //< (out) focus region setting 
        CamSensorSelect_t           sensor                      //< (in) camera sensor select
        );

/** Set Focus Status for camera Sensor
    @param  *pfocus_region focus region setting 
    @param  sensor         [in] camera sensor select
    @note   none
*/
HAL_CAM_Result_en_t CAMDRV_SetFocusRegion(
        HAL_CAM_Focus_Region_st_t*  pfocus_region,              //< (in) focus region setting 
        CamSensorSelect_t           sensor                      //< (in) camera sensor select
        );

/** Get Focus Control Configuration for camera Sensor
    @param  *pfocus_control [out] focus control configuration
    @param  sensor [in] camera sensor select
    @note   none
*/
HAL_CAM_Result_en_t CAMDRV_GetFocusControlConfig(
        HAL_CAM_Focus_Control_Config_st_t*  pfocus_control,     //< (out) focus control configuration
        CamSensorSelect_t                   sensor              //< (in) camera sensor select
        );

/** Set Focus Control Configuration for camera Sensor
    @param  *pfocus_control [out] focus control configuration
    @param  sensor          [in] camera sensor select
    @note   none
*/
HAL_CAM_Result_en_t CAMDRV_SetFocusControlConfig(
        HAL_CAM_Focus_Control_Config_st_t*  pfocus_control,     //< (out) focus control configuration
        CamSensorSelect_t                   sensor              //< (in) camera sensor select
        );

/** Set image quality for jpeg capture image.
  @param quality   [in] quality level [0-10].
    @param  sensor              [in] .
  @return       HAL_CAM_Result_en_t 
*/
HAL_CAM_Result_en_t CAMDRV_SetImageQuality(
        UInt8 quality,
        CamSensorSelect_t sensor
        );
/** Get Jpeg max size for camera Sensor (in bytes)
    @param  in_psize            [in].
    @param  sensor              [in] .
    @note   Maximum Jpeg output size (bytes)
*/
#if defined(SS_2153) && defined(BROOKLYN_HW)
UInt32 CAMDRV_GetJpegMaxSize(
        CamSensorSelect_t sensor
        );
#else
HAL_CAM_Result_en_t CAMDRV_GetJpegMaxSize(
		UInt32	in_psize,		
        CamSensorSelect_t sensor
        );
#endif
/** Get luminance for camera Sensor (in bytes)
    @param  in_pluminance       [in].
    @param  sensor              [in] .    
    @note   luminance (0: normal, 1: middle, 2: low)
*/
#if defined(SS_2153) && defined(BROOKLYN_HW)
UInt32 CAMDRV_GetLuminance(
        CamSensorSelect_t sensor
        );
#else
HAL_CAM_Result_en_t CAMDRV_GetLuminance(
		UInt32	in_pluminance,	
        CamSensorSelect_t sensor
        );
#endif
/** Lock auto color
  @param in_pcolor   [in] data address of color level [0-2].
  @return       HAL_CAM_Result_en_t 
*/
#if defined(SS_2153) && defined(BROOKLYN_HW)
HAL_CAM_Result_en_t CAMDRV_LockAutoColor(
        UInt32 color,CamSensorSelect_t sensor
        );
#else
HAL_CAM_Result_en_t CAMDRV_LockAutoColor(
        UInt32 in_pcolor
        );
#endif
/** unlock auto color
  @param color   [in] color level [0-2].
  @return       HAL_CAM_Result_en_t 
*/
#if defined(SS_2153) && defined(BROOKLYN_HW)
HAL_CAM_Result_en_t CAMDRV_UnlockAutoColor(
        UInt32 color,CamSensorSelect_t sensor
        );
#else
HAL_CAM_Result_en_t CAMDRV_UnlockAutoColor(
        UInt32 color
        );
#endif

HAL_CAM_Result_en_t CAMDRV_SetCaptureCallback(UInt32 callback,CamSensorSelect_t sensor);
//-- BRCM add CSP#225120
UInt32 CAMDRV_GetExposureTime(
        CamSensorSelect_t sensor
        );
UInt32 CAMDRV_GetFNumber(
        CamSensorSelect_t sensor
        );
UInt32 CAMDRV_GetFocalLenth(
        CamSensorSelect_t sensor
        );
UInt32 CAMDRV_GetISOSpeed(
        CamSensorSelect_t sensor
        );
UInt32 CAMDRV_GetShutterSpeed(
        CamSensorSelect_t sensor
        );

/** Set contrast for preview image.
  @param contrast   [in] contrast level.
    @param  sensor              [in] .
  @return       HAL_CAM_Result_en_t 
*/
HAL_CAM_Result_en_t CAMDRV_SetContrast(
        Int8 contrast,
        CamSensorSelect_t sensor
        );

/** Set brightness for preview image.
  @param brightness [in] brightness level.
    @param  sensor              [in] .
  @return       HAL_CAM_Result_en_t 
*/
HAL_CAM_Result_en_t CAMDRV_SetBrightness(
        Int8 brightness,
        CamSensorSelect_t sensor
        );

/** Set Saturation Hue for preview image.  NOT for average user's use.
  @param saturation [in] saturation level.
    @param  sensor              [in] .
  @return       HAL_CAM_Result_en_t 
*/
HAL_CAM_Result_en_t CAMDRV_SetSaturation(
        Int8 saturation,
        CamSensorSelect_t sensor
        );

/** Set hue for preview image.  NOT for average user's use.
  @param hue    [in] hue level.
  @param sensor [in]
  @return       HAL_CAM_Result_en_t 
*/
HAL_CAM_Result_en_t CAMDRV_SetHue(
        Int8 hue,
        CamSensorSelect_t sensor
        );

/** Set Gamma Preview.  NOT for average user's use.
  @param gamma  [in] gamma level.
    @param  sensor              [in] .
  @return       HAL_CAM_Result_en_t 
    @note       Accepts values from [0, 255], although only 16 gamma settings
                        are possible and range from 0.5 to 1.0
*/
HAL_CAM_Result_en_t CAMDRV_SetGamma(
        Int8 gamma,
        CamSensorSelect_t sensor
        );

/** Set Sharpness for preview image.  NOT for average user's use.
  @param sharpness [in] sharpness level.
    @param  sensor              [in] .
  @return       HAL_CAM_Result_en_t 
    @note       11 levels supported: MIN_SHARPNESS(0) for no sharpening,
                        and MAX_SHARPNESS(11) for 200% sharpening
*/
HAL_CAM_Result_en_t CAMDRV_SetSharpness(
        Int8 sharpness,
        CamSensorSelect_t sensor
        );

/** Set anti shading power for preview image.  NOT for average user's use.
  @param asp    [in] shading power.
    @param  sensor              [in] .
  @return       HAL_CAM_Result_en_t 
*/
HAL_CAM_Result_en_t CAMDRV_SetAntiShadingPower(
        Int8 asp,
        CamSensorSelect_t sensor
        );

/** Set the required Scene mode (or AUTO scene).
    Notice that if any Special Effect is active then it is disabled.
    
    Average users can use this function to use different settings for still capture.
    The AUTO scene mode is good enough for all shooting needs.

  @param scene_mode     [in] the required scene mode.
  @param  sensor              [in] .
  @return               HAL_CAM_Result_en_t 
*/
HAL_CAM_Result_en_t CAMDRV_SetSceneMode(
        CamSceneMode_t scene_mode,  
        CamSensorSelect_t sensor     
        );

/** Set the required Digital Effect (Special Effect).

  @param effect     [in] the required Special Effect.
    @param  sensor              [in] .
  @return           HAL_CAM_Result_en_t 
*/
HAL_CAM_Result_en_t CAMDRV_SetDigitalEffect(
        CamDigEffect_t effect,                      ///< (in) 
        CamSensorSelect_t sensor                    ///< (in) 
        );

//[

HAL_CAM_Result_en_t CAMDRV_SetAntiBanding(
		CamAntiBanding_t effect,                   ///<(in)
		CamSensorSelect_t sensor );				   ///<(in)


HAL_CAM_Result_en_t CAMDRV_SetFocusMode(
        CamFocusControlMode_t effect,                      ///< (in) 
        CamSensorSelect_t sensor                    ///< (in) 
        );
HAL_CAM_Result_en_t CAMDRV_TurnOffAF();
HAL_CAM_Result_en_t CAMDRV_TurnOnAF();

HAL_CAM_Result_en_t CAMDRV_SetJpegQuality(
        CamJpegQuality_t effect,                      ///< (in) 
        CamSensorSelect_t sensor                    ///< (in) 
        );
//]
/* Set Flicker Control.  NOT Supported.
  @param control    [in] control value.
    @param  sensor              [in] .
  @return       HAL_CAM_Result_en_t 
*/
HAL_CAM_Result_en_t CAMDRV_SetFlickerControl(
        CamFlicker_t control,
        CamSensorSelect_t sensor                    //< (in) 
        );

/** Set the required WhiteBalance mode manually or its AUTO mode.
    Notice that if any Special Effect is active it is disabled.

    Only advanced users need to use these. (base on light source)
    For most users the AUTO mode is good enough for all shooting needs.

  @param wb_mode        [in] the required WB mode.
    @param  sensor              [in] .
  @return               HAL_CAM_Result_en_t 
*/
HAL_CAM_Result_en_t CAMDRV_SetWBMode(
        CamWB_WBMode_t wb_mode,                     //< (in) 
        CamSensorSelect_t sensor                    //< (in) 
        );

/** Enables/Disables auto exposure
*  @param exposure  [in] TRUE = auto exposure on.
   @param sensor [in]
*  @return      HAL_CAM_Result_en_t 
*/
HAL_CAM_Result_en_t CAMDRV_SetAutoExposure(
        CamExposure_t exposure,
        CamSensorSelect_t sensor                    //< (in) 
        );

/** Set the required Metering Type (Image Quality Settings).  NOT for average user's use.

  @param type       [in] the required Metering Type.
  @param  sensor    [in] .
  @return           HAL_CAM_Result_en_t 
*/
HAL_CAM_Result_en_t CAMDRV_SetMeteringType(
        CamMeteringType_t type,                     //< (in) 
        CamSensorSelect_t sensor                    //< (in) 
        );

/** Set the required Sensitivity (Image Quality Settings).  NOT for average user's use.

  @param iso        [in] the required Sensitivity.
    @param  sensor              [in] .
  @return           HAL_CAM_Result_en_t 
*/
HAL_CAM_Result_en_t CAMDRV_SetSensitivity(
        CamSensitivity_t iso,                       ///< (in) 
        CamSensorSelect_t sensor                    ///< (in) 
        );

/** Enable/Disable Wide Dynamic Range (Image Quality Settings).  NOT for average user's use.

    @param pWDR         [in] data address of Wide Dynamic Range Enable/Disable.
    @param  sensor      [in] .
    @return             HAL_CAM_Result_en_t 
*/
#if defined(SS_2153) && defined(BROOKLYN_HW)
HAL_CAM_Result_en_t CAMDRV_SetWideDynRangeEnable(
        Boolean bWDR,                               
        CamSensorSelect_t sensor                    
        );
#else
HAL_CAM_Result_en_t CAMDRV_SetWideDynRangeEnable(
        UInt32 pWDR,                               
        CamSensorSelect_t sensor                    
        );
#endif

/** Initialize camera parameters if necessary.
*  @param sensor [in] Sensor selected for this function.
*  @return   HAL_CAM_Result_en_t 
*/
HAL_CAM_Result_en_t CAMDRV_InitParm(CamSensorSelect_t sensor);
//Boolean InitCamParm(void);


/** Performs additional device specific initialization.
*  @param sensor [in] CamSensorSelect_t.
*  @return      HAL_CAM_Result_en_t 
*/
HAL_CAM_Result_en_t CAMDRV_Supp_Init(CamSensorSelect_t sensor);

/** Wake up camera sensor.
*  @param sensor [in[ Sensor selected for this function.
*  @return   HAL_CAM_Result_en_t 
*  @note     via I2C command, assumes camera is enabled.
*/
HAL_CAM_Result_en_t CAMDRV_Wakeup(CamSensorSelect_t sensor);

/**
*  Set camera to sleep mode.
*  @param  sensor              [in] .
*  @return   HAL_CAM_Result_en_t 
*/
HAL_CAM_Result_en_t  CAMDRV_SetCamSleep( CamSensorSelect_t sensor );

/**  Turn on Preview mode.
    @param sensor   [in] .
    @return     HAL_CAM_Result_en_t 
*/
HAL_CAM_Result_en_t CAMDRV_EnablePreview(CamSensorSelect_t sensor);

/**  Turn on Video Capture mode.
*  @param sensor    [in] .
*  @return      HAL_CAM_Result_en_t 
*/
HAL_CAM_Result_en_t CAMDRV_EnableVideoCapture(CamSensorSelect_t    sensor);

/** Turn off preview.
    @param  sensor              [in] .
  @return       HAL_CAM_Result_en_t 
*/
HAL_CAM_Result_en_t CAMDRV_DisablePreview(CamSensorSelect_t sensor);

/** Configure camera's preview  mode.
  @param index  [in] .
  @param enable [in] .
    @param  nSensor              [in] .
  @return       HAL_CAM_Result_en_t 
*/
HAL_CAM_Result_en_t CAMDRV_CfgPreview(
        UInt16 index,                               
        UInt16 enable,
        CamSensorSelect_t nSensor
        );

/** Configure camera's ViewFinder  mode.
  @param image_resolution   [in] .
  @param image_format   [in] .
  @param sensor [in] .
  @return       HAL_CAM_Result_en_t 
*/
HAL_CAM_Result_en_t CAMDRV_SetViewFinderMode(
        CamImageSize_t image_resolution, 
        CamDataFmt_t image_format,
        CamSensorSelect_t sensor
        );

/** Configure camera's Video Capture  mode.
  @param image_resolution   [in] .
  @param image_format   [in] .
  @param sensor [in] .
  @return       HAL_CAM_Result_en_t 
*/
HAL_CAM_Result_en_t CAMDRV_SetVideoCaptureMode(
        CamImageSize_t image_resolution, 
        CamDataFmt_t image_format,
        CamSensorSelect_t sensor
        );

/** Configure camera's Video n Preview Capture  mode.
  @param video_resolution       [in] .
  @param video_format           [in] .
  @param viewfinder_resolution  [in] .
  @param viewfinder_format      [in] .
  @param sensor                 [in] .
  @return       HAL_CAM_Result_en_t 
*/
HAL_CAM_Result_en_t CAMDRV_SetVideoPreviewCaptureMode(
        CamImageSize_t video_resolution, 
        CamDataFmt_t video_format,
        CamImageSize_t viewfinder_resolution, 
        CamDataFmt_t viewfinder_format,
        CamSensorSelect_t sensor
        );

/** Configure camera's capture mode.
  @param image_resolution   [in] . CamImageSize_t
  @param image_format   [in] . CamDataFmt_t
  @param  sensor              [in] . CamSensorSelect_t
  @return       HAL_CAM_Result_en_t 
*/
HAL_CAM_Result_en_t CAMDRV_CfgCapture(
        CamImageSize_t image_resolution,                
        CamDataFmt_t image_format,
        CamSensorSelect_t sensor
        );

/** Configure camera's Stills n Thumb Capture  mode.
  @param stills_resolution       [in] .
  @param stills_format           [in] .
  @param thumb_resolution  [in] .
  @param thumb_format      [in] .
  @param sensor                 [in] .
  @return       HAL_CAM_Result_en_t 
*/
HAL_CAM_Result_en_t CAMDRV_CfgStillnThumbCapture(
        CamImageSize_t stills_resolution, 
        CamDataFmt_t stills_format,
        CamImageSize_t thumb_resolution, 
        CamDataFmt_t thumb_format,
        CamSensorSelect_t sensor
        );

/** Halt camera capture mode.
    @param  sensor              [in] .
  @return       HAL_CAM_Result_en_t 
*/
HAL_CAM_Result_en_t CAMDRV_DisableCapture(CamSensorSelect_t sensor);


/** Return camera capture size.
  @param size   [in] .
    @param  nSensor              [in] .
  @return       CamImageSize_t
*/
CamImageSize_t CAMDRV_GetCaptureSize(
        CamImageSize_t size,
        CamSensorSelect_t nSensor
        );

/** @} */


/* <do not doxygen>
*  Return camera controller maximum zoom 
    @param  nSensor              [in] .
*/
UInt32 CAMDRV_GetCamSensorMaxZoom(CamSensorSelect_t nSensor);

/*<do not doxygen>
*  Return camera controller minimum zoom 
    @param  nSensor              [in] .
*/
UInt32 CAMDRV_GetCamSensorMinZoom(CamSensorSelect_t nSensor);

/**
 * @addtogroup CamDevDrvGroup
 * @{
 */

#if defined(CONFIG_BCM_CAM_MT9T111)
void CAMDRV_StoreBaseAddress(void * virt_ptr);
#endif

/**
*  Return Captured JPEG size (if mode supported)
    @param  sensor              [in] .
*/
UInt32 CAMDRV_GetJpegSize(CamSensorSelect_t sensor, void *data);

/**
*  Return Captured JPEG data (if mode supported)
    @param  buf              [in] .
*/
UInt16 *CAMDRV_GetJpeg(short *buf);


/**
*  Return pointer to thumbnail image
    @param  buf [in] .
    @param  offset [in] .
*/
UInt8 * CAMDRV_GetThumbnail(void *buf, UInt32 offset);

/**
*  Return device ID
    @param  sensor              [in] .
*/
UInt16 CAMDRV_GetDeviceID(CamSensorSelect_t sensor);

/** @} */

/*<do not doxygen>
*  Returns camera physical height
    @param  nSensor              [in] .
*/
UInt32 CAMDRV_PhyH(CamSensorSelect_t nSensor);

/*<do not doxygen>
*  Returns camera physical width
    @param  nSensor              [in] .
*/
UInt32 CAMDRV_PhyW(CamSensorSelect_t nSensor);




/** The CAMDRV_CheckESD returns Boolean value to indicate the ESD result

    @param [in] nSensor Sensor for which image settings is required .
    
    @return Boolean
        TRUE, sensor functions normally.
        FALSE, else
 */
Boolean	CAMDRV_CheckESD(CamSensorSelect_t nSensor);

/** The CAMDRV_CheckManufacturerID check manufacturer ID 

    @param [in] nSensor Sensor for which image settings is required .
    
    @return Boolean
        TRUE, the manufacturer ID is correct.
        FALSE, else
 */
Boolean CAMDRV_CheckManufacturerID(CamSensorSelect_t nSensor);


/** The constructor of the camera driver, must be the 1st function called by CMI. 
    @param [in] nSensor Sensor for which image settings is required (unused).

  */
void CAMDRV_CreateCamera(CamSensorSelect_t nSensor);


/** The destructor of the camera driver, must be the last function called by CMI 
    @param [in] sensor
        Sensor for which image settings is required (unused).

  */
void CAMDRV_DestroyCamera(CamSensorSelect_t sensor);

HAL_CAM_Result_en_t CAMDRV_DoAutoFocus(
        Boolean auto_focus,
        CamSensorSelect_t nSensor
        );

struct sens_methods * CAMDRV_primary_get(void);

#endif  // _CAMDRV_DEV_H_

