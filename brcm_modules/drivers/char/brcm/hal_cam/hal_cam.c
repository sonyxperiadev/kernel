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
*   @file   hal_cam.c
*
*   @brief  001     This file is the HAL API driver level for camera
*
****************************************************************************/

/**
 * @addtogroup CamDrvGroup
 * @{
 */

// ---- Include Files -------------------------------------------------------
#include <stdarg.h>

#include <linux/version.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/poll.h>
#include <linux/sysctl.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>

#include <mach/reg_camera.h>
#include <mach/reg_lcd.h>

#include <mach/reg_clkpwr.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/atomic.h>

#include <linux/semaphore.h>
#include <linux/broadcom/types.h>
#include <linux/broadcom/bcm_major.h>
#include <linux/broadcom/hw_cfg.h>
#include <linux/broadcom/hal_camera.h>
#include <linux/broadcom/lcd.h>
#include <linux/broadcom/bcm_sysctl.h>
#include <linux/broadcom/PowerManager.h>
#include <plat/dma.h>
#include <linux/dma-mapping.h>

#include "hal_cam.h"
#include "hal_cam_drv.h"
#include "camdrv_dev.h"
#include "resultcode.h"

// ---- Public Variables ----------------------------------------------------
// ---- Private Constants and Types -----------------------------------------
#define HAL_CAM_TRACE_MSG_SIZE 512

// static Semaphore_t  sHalCamActionSema = NULL;          // for coordinating Hal Cam Action Commands
// Hal Camera Tracing Variables
static char HAL_CAM_Debugstring[HAL_CAM_TRACE_MSG_SIZE];
static UInt32 sHAL_CAM_TraceLevel = HAL_CAM_TRACE_PASSFAIL | HAL_CAM_TRACE_ERROR;
// static UInt32 sHAL_CAM_TraceLevel = HAL_CAM_TRACE_LAYERS | HAL_CAM_TRACE_FUNCTION | HAL_CAM_TRACE_PARAMETERS | HAL_CAM_TRACE_WARNING | HAL_CAM_TRACE_PASSFAIL | HAL_CAM_TRACE_INFO | HAL_CAM_TRACE_ERROR;
#if 0
static UInt8 *spHAL_CAM_TraceBuffer = NULL;
static UInt32 sHAL_CAM_Max_TraceBuffer_Count = 0;
static UInt32 sHAL_CAM_TraceBuffer_Count = 0;
#endif

// ---- Functions -----------------------------------------------------------
#define TRACE_Printf_Sio( x ) printk( x )

//! -----------------------------------------------------------------------------------------
//! Function:       Set HAL Cam Trace Level
//!
//! Params:         Val2 = Trace Levels
//!                     ALL TRACING ENABLED             = FFFFFFFF /**< Device Driver level tracing. */
//!                     HAL_CAM_TRACE_PASSFAIL          = 00000001 /**< Messages indicating the success or failure of a test*/
//!                     HAL_CAM_TRACE_FUNCTION          = 00000002 /**< The name of each function */
//!                     HAL_CAM_TRACE_PARAMETERS        = 00000004 /**< The parameters of each function */
//!                     HAL_CAM_TRACE_INFO              = 00000008 /**< Generalized info. */
//!                     HAL_CAM_TRACE_ERROR             = 00000010 /**< Errors that occur during processing. */
//!                     HAL_CAM_TRACE_WARNING           = 00000020 /**< Warnings reported during processing. */
//!             Layer of Tracing for all traces except ERROR
//!                     HAL_CAM_TRACE                   = 00000100 /**< HAL Driver layer tracing. */
//!                     HAL_CAM_TRACE_HALDRV            = 00000200 /**< HAL Driver layer tracing. */
//!                     HAL_CAM_TRACE_DEVICE            = 00000400 /**< Device Driver layer tracing. */
//!                     HAL_CAM_TRACE_CNTRL             = 00000800 /**< Baseband Controller layer tracing. */
//! -----------------------------------------------------------------------------------------
UInt32 HAL_CAM_Set_TraceLevel(UInt32 nTraceLevel)
{
    UInt32 originalTraceLevel = sHAL_CAM_TraceLevel;

    sHAL_CAM_TraceLevel = nTraceLevel;
    pr_debug("HAL_CAM_Set_TraceLevel():  0x%08x \r\n",nTraceLevel);
    return originalTraceLevel;
}

HAL_CAM_Result_en_t HAL_CAM_Trace(UInt32 nTraceFlags, char *format, ...)
{
    va_list argp;

#if 0   // Tracing Debugging
    sprintf( HAL_CAM_Debugstring, "HAL_CAM_Trace(): nTraceFlags = 0x%x sHAL_CAM_TraceLevel = 0x%x  \r\n", nTraceFlags, sHAL_CAM_TraceLevel);
    TRACE_Printf_Sio( HAL_CAM_Debugstring );
    sprintf( HAL_CAM_Debugstring, "HAL_CAM_Trace(): layers = 0x%x \r\n", (HAL_CAM_TRACE_LAYERS & (nTraceFlags & sHAL_CAM_TraceLevel)) );
    TRACE_Printf_Sio( HAL_CAM_Debugstring );
    sprintf( HAL_CAM_Debugstring, "HAL_CAM_Trace(): type = 0x%x \r\n", ( nTraceFlags & sHAL_CAM_TraceLevel & ~HAL_CAM_TRACE_LAYERS ) );
    TRACE_Printf_Sio( HAL_CAM_Debugstring );
#endif

// If Error Enabled, do not check Tracing Layer/Level
    if ( (nTraceFlags & sHAL_CAM_TraceLevel & HAL_CAM_TRACE_ERROR) == 0 )
    {
    // Check if layer of tracing is enabled
        if ( (HAL_CAM_TRACE_LAYERS & nTraceFlags & sHAL_CAM_TraceLevel) == 0)
        {
            return HAL_CAM_SUCCESS;
        }

    // Check if type of tracing enabled
        if ( ((nTraceFlags & sHAL_CAM_TraceLevel & ~HAL_CAM_TRACE_LAYERS) == 0) && (sHAL_CAM_TraceLevel != 0xFFFFFFFF) )
        {
            return HAL_CAM_SUCCESS;
        }
    }

// Output Trace Info
    va_start( argp, format);

#if 0
// Format trace buffer if enabled
    if (spHAL_CAM_TraceBuffer)
    {
        vsprintf( (char *)spHAL_CAM_TraceBuffer[sHAL_CAM_TraceBuffer_Count%sHAL_CAM_Max_TraceBuffer_Count], format, argp );
        sHAL_CAM_TraceBuffer_Count++;
        va_end( argp );
        return HAL_CAM_SUCCESS;
    }
#endif
// Print trace
    vsprintf( HAL_CAM_Debugstring, format, argp );
    TRACE_Printf_Sio( HAL_CAM_Debugstring );
    va_end( argp );
    return HAL_CAM_SUCCESS;
}

/**
*  This function will initialize Cam Driver.
*                       To Initialize to default settings, HAL_CAM_Init(NULL);
*                                                       To change default settings use ACTION_CAM_GetCameraSettings to get a copy of default camera settings,
*                                                       Modify default settings in HAL_CAM_Action_settings_st_tand pass back in HAL_CAM_Init(HAL_LCD_config_st* HAL_LCD_config)
*                                                           driver will copy new settings to local and initialize Camera.
*/
HAL_CAM_Result_en_t HAL_CAM_Init(
        CamSensorSelect_t           sensor_select,                      ///< (in) Select Camera Sensor
        HAL_CAM_config_st_t*        config_st                           ///< (out) device configuration structure
        )
{
//    Result_t result = RESULT_OK;
    HAL_CAM_Result_en_t hal_result = HAL_CAM_SUCCESS;

    pr_debug("HAL_CAM_Init \r\n");

#if 0
// Obtain Hal Cam Action Semaphore
    if( sHalCamActionSema == NULL )
    {
        sHalCamActionSema = OSSEMAPHORE_Create( 1, OSSUSPEND_PRIORITY );
        OSSEMAPHORE_ChangeName(sHalCamActionSema, "HalCamAction");
        pr_debug("sHalCamActionSema Created \r\n");
    }
    OSSEMAPHORE_Obtain(sHalCamActionSema,TICKS_FOREVER);

    if (config_st != NULL)
    {
    //  Get Current Camera Default Settings
        CAM_GetCameraSettings(config_st->cam_config_st, CamPowerOff, sensor_select);
    }

//  Initialize Camera Driver and Sensor
    result = CAM_Init(sensor_select);
    switch (result)
    {
        case CAM_UNSUPPORTED:
            hal_result = HAL_CAM_ERROR_ACTION_NOT_SUPPORTED;
            break;
        case CAM_FAILED:
            hal_result = HAL_CAM_ERROR_INTERNAL_ERROR;
            break;
        case RESULT_DONE:
            hal_result = HAL_CAM_SUCCESS;
            break;
        case RESULT_OK:
            hal_result = HAL_CAM_SUCCESS;
            break;
        default:
            hal_result = HAL_CAM_ERROR_ACTION_NOT_SUPPORTED;
            break;
    }

// Release Hal Cam Action Semaphore
    OSSEMAPHORE_Release(sHalCamActionSema);
#endif
    return hal_result;
}

/** @} */


/**
*  This function will perform specific action from defined list, copy of parameters passed thru parm structure.
*
*/
HAL_CAM_Result_en_t HAL_CAM_Ctrl(
        HAL_CAM_Action_en_t action,                 ///< (in) device action
        void*               data,                   ///< (in/out) structure pointer to additional parameters for Set, Get, and IS
        void*               callback                ///< (in) callback function device driver calls once action complete, NULL if not needed
        )
{
    Result_t result = RESULT_OK;
    HAL_CAM_Result_en_t hal_result = HAL_CAM_SUCCESS;
#if 0
    CamIntfConfig_st_t *pSensorIntfConfig;

    pr_debug("HAL_CAM_Ctrl():  action=%d \r\n", action);

// Obtain Hal Cam Action Semaphore
    if( sHalCamActionSema == NULL )
    {
        sHalCamActionSema = OSSEMAPHORE_Create( 1, OSSUSPEND_PRIORITY );
        OSSEMAPHORE_ChangeName(sHalCamActionSema, "HalCamAction");
        pr_debug("sHalCamActionSema Created \r\n");
    }
    OSSEMAPHORE_Obtain(sHalCamActionSema,TICKS_FOREVER);
#endif

    switch (action)
    {
// *** Actions handled by hal camera driver
    // Camera Configuration
#if 0
        case ACTION_CAM_SetPowerMode:                           ///< CamPowerOff/CamPowerOn interface and sensor, use HAL_CAM_Action_power_mode_st_t to set
            result = CAM_SetPowerMode( ((HAL_CAM_Action_power_mode_st_t *)data)->sensor, ((HAL_CAM_Action_power_mode_st_t *)data)->mode );
            break;
        case ACTION_CAM_GetCameraSettings:              ///< Get Interface and Nominal/Min/Max camera settings, use HAL_CAM_Action_settings_st_t to get
            result = CAM_GetCameraSettings(((HAL_CAM_Action_settings_st_t *)data)->cam_config_st, ((HAL_CAM_Action_settings_st_t *)data)->mode, ((HAL_CAM_Action_settings_st_t *)data)->sensor);
            break;
        case ACTION_CAM_GetCameraInterface:             ///< Get I2C and Camera Controller interface settings, use HAL_CAM_Action_cam_interface_st_t to get
            pSensorIntfConfig = CAM_GetCameraInterface( ((HAL_CAM_Action_cam_interface_st_t *)data)->sensor );
            ((HAL_CAM_Action_cam_interface_st_t *)data)->i2c_dev_id = pSensorIntfConfig->sensor_config_i2c->i2c_device_id;
            ((HAL_CAM_Action_cam_interface_st_t *)data)->i2c_max_page = pSensorIntfConfig->sensor_config_i2c->i2c_max_page;
            ((HAL_CAM_Action_cam_interface_st_t *)data)->i2c_sub_addr_op = pSensorIntfConfig->sensor_config_i2c->i2c_sub_adr_op;
//            ((HAL_CAM_Action_cam_interface_st_t *)data)->controller_config = CAM_CNTRL_GetConfig();
            break;
    // Image Capture Modes
        case ACTION_CAM_CaptureImage:                           ///< Capture camera image, use HAL_CAM_Action_capture_image_st_t to enable
            result = CAM_CaptureImage( ((HAL_CAM_Action_capture_image_st_t *)data)->size, ((HAL_CAM_Action_capture_image_st_t *)data)->format,
                                                        ((HAL_CAM_Action_capture_image_st_t *)data)->sensor, ((HAL_CAM_Action_capture_image_st_t *)data)->buf, callback );
            break;
        case ACTION_CAM_CaptureVideo:                           ///< Start camera video capture, use HAL_CAM_Action_capture_video_st_t to enable
            result = CAM_CaptureVideo( ((HAL_CAM_Action_capture_video_st_t *)data)->size, ((HAL_CAM_Action_capture_video_st_t *)data)->format,
                                                        ((HAL_CAM_Action_capture_video_st_t *)data)->sensor, ((HAL_CAM_Action_capture_video_st_t *)data)->buf0,
                                                        ((HAL_CAM_Action_capture_video_st_t *)data)->buf1, callback );
            break;
        case ACTION_CAM_PreviewPPzoom:                          ///< Start camera preview/video capture, use HAL_CAM_Action_preview_PPzoom_st_t to enable
            result = CAM_PreviewPPzoom( ((HAL_CAM_Action_preview_PPzoom_st_t *)data)->window_scale,
                                        ((HAL_CAM_Action_preview_PPzoom_st_t *)data)->format, ((HAL_CAM_Action_preview_PPzoom_st_t *)data)->sensor,
                                        ((HAL_CAM_Action_preview_PPzoom_st_t *)data)->flash_mode, ((HAL_CAM_Action_preview_PPzoom_st_t *)data)->mode,
                                        ((HAL_CAM_Action_preview_PPzoom_st_t *)data)->buf0, ((HAL_CAM_Action_preview_PPzoom_st_t *)data)->buf1, callback );
            break;
        case ACTION_CAM_StillsPPzoom:                          ///< Start camera preview/video capture, use HAL_CAM_Action_preview_PPzoom_st_t to enable
            result = CAM_StillsPPzoom( ((HAL_CAM_Action_stills_PPzoom_st_t *)data)->window_scale, ((HAL_CAM_Action_stills_PPzoom_st_t *)data)->format,
                                        ((HAL_CAM_Action_stills_PPzoom_st_t *)data)->sensor, ((HAL_CAM_Action_stills_PPzoom_st_t *)data)->flash_mode,
                                        ((HAL_CAM_Action_stills_PPzoom_st_t *)data)->buf, callback );
            break;
        case ACTION_CAM_CapImageWithThumbNail:                  ///< Capture camera image with thumbnail, use HAL_CAM_Action_capture_image_with_thumbnail_st_t to enable
            result = CAM_CapImageWithThumbNail( ((HAL_CAM_Action_capture_image_with_thumbnail_st_t *)data)->size, ((HAL_CAM_Action_capture_image_with_thumbnail_st_t *)data)->format,
                                        ((HAL_CAM_Action_capture_image_with_thumbnail_st_t *)data)->sensor, ((HAL_CAM_Action_capture_image_with_thumbnail_st_t *)data)->buf,
                                        ((HAL_CAM_Action_capture_image_with_thumbnail_st_t *)data)->thumbnail_width, ((HAL_CAM_Action_capture_image_with_thumbnail_st_t *)data)->thumbnail_height,
                                        ((HAL_CAM_Action_capture_image_with_thumbnail_st_t *)data)->thumbnail_format,
                                        ((HAL_CAM_Action_capture_image_with_thumbnail_st_t *)data)->thumbnail_buf, callback );
            break;
        case ACTION_CAM_CapVideoWithViewFinder:                 ///< Start camera video capture with viewfinder, use HAL_CAM_Action_capture_video_with_viewfinder_st_t to enable
            result = CAM_CapVideoWithViewfinder( ((HAL_CAM_Action_capture_video_with_viewfinder_st_t *)data)->size, ((HAL_CAM_Action_capture_video_with_viewfinder_st_t *)data)->format,
                                        ((HAL_CAM_Action_capture_video_with_viewfinder_st_t *)data)->sensor, ((HAL_CAM_Action_capture_video_with_viewfinder_st_t *)data)->buf0,
                                        ((HAL_CAM_Action_capture_video_with_viewfinder_st_t *)data)->buf1,((HAL_CAM_Action_capture_video_with_viewfinder_st_t *)data)->viewfinder_size,
                                        ((HAL_CAM_Action_capture_video_with_viewfinder_st_t *)data)->viewfinder_format, ((HAL_CAM_Action_capture_video_with_viewfinder_st_t *)data)->viewfinder_buf0,
                                        ((HAL_CAM_Action_capture_video_with_viewfinder_st_t *)data)->viewfinder_buf1, callback );
            break;
        case ACTION_CAM_StopCapture:                    ///< Stop video or image capture, no additional parameters
            result = CAM_StopCapture();
            break;
		case ACTION_CAM_PauseCapture:					///< Pause viewfinder, video capture, no additional parameters
			result = CAM_PauseCapture();
			break;

		case ACTION_CAM_ResumeCapture:					///< Resume viewfinder, video capture, no additional parameters
			result = CAM_ResumeCapture();
			break;

    // Operation Control Settings
        case ACTION_CAM_SetWindowScale:             ///< Set the Camera window size and scale factors, use HAL_CAM_Action_window_scale_st_t to set
            result = CAM_SetWindowScale( (CamImageSize_t)((HAL_CAM_Action_window_scale_st_t *)data)->size,
                                                        (UInt32)((HAL_CAM_Action_window_scale_st_t *)data)->start_pixel, (UInt32)((HAL_CAM_Action_window_scale_st_t *)data)->end_pixel,
                                                        (UInt32)((HAL_CAM_Action_window_scale_st_t *)data)->start_line, (UInt32)((HAL_CAM_Action_window_scale_st_t *)data)->end_line,
                                                        (UInt32)((HAL_CAM_Action_window_scale_st_t *)data)->horizontal_SDF, (UInt32)((HAL_CAM_Action_window_scale_st_t *)data)->vertical_SDF );
            break;
        case ACTION_CAM_SetZoom:                                    ///< Set required zoom, use (UInt16) HAL_CAM_Action_param_st_t to set
            result = CAM_SetZoom( (UInt16)(((HAL_CAM_Action_param_st_t *)data)->param) );
            break;
        case ACTION_CAM_GetMaxWindowSize:                   ///< Camera Maximum Pixel Window size, use HAL_CAM_Action_window_st_t to get
            pSensorIntfConfig = CAM_GetCameraInterface( ((HAL_CAM_Action_window_st_t *)data)->sensor );
                                ((HAL_CAM_Action_window_st_t *)data)->width = pSensorIntfConfig->sensor_config_caps->output_st.max_width;
                                ((HAL_CAM_Action_window_st_t *)data)->height = pSensorIntfConfig->sensor_config_caps->output_st.max_height;
            break;
        case ACTION_CAM_GetMaxZoom:                             ///< Get maximum zoom allowed, use (UInt16) HAL_CAM_Action_param_st_t to get
#if defined(SS_2153) && defined(BROOKLYN_HW)
            ((HAL_CAM_Action_param_st_t *)data)->param = (UInt16)CAM_GetMaxZoom();
#else
            ((HAL_CAM_Action_param_st_t *)data)->param = (UInt16)CAM_GetMaxZoom(((HAL_CAM_Action_param_st_t *)data)->sensor);
#endif
            break;

    // Flash Settings
        case ACTION_CAM_SetFlashEnable:                     ///< Set flash enable TRUE/FALSE, use (Boolean) HAL_CAM_Action_param_st_t to set
            result = CAM_SetFlashEnable(*((Boolean*)((HAL_CAM_Action_param_st_t *)data)->param) );
            break;
    // Register Callback
		case	ACTION_CAM_SetESD_CB:						///< ESD callback setting
			result = CAM_SetEsdCb((((HAL_CAM_Action_param_st_t *)data)->param), ((HAL_CAM_Action_param_st_t *)data)->sensor );
			break;
#endif

    // Resolution not handled by all camera device drivers
#if 0
        case ACTION_CAM_GetSensorResolution:            ///< Get matching image size from passed pixel width and height, use HAL_CAM_Action_image_size_st_t to get CamImageSize_t
            result  = CAMDRV_GetResolution(((HAL_CAM_Action_resolution_st_t *)data)->size,
                                                    ((HAL_CAM_Action_resolution_st_t *)data)->mode,
                                                    ((HAL_CAM_Action_resolution_st_t *)data)->sensor,
                                                    ((HAL_CAM_Action_resolution_st_t *)data)->sensor_size);
            break;
#endif

// *** Actions for Camera Sensor Driver Only ***  (No baseband control, handled by camera device driver)
// Default: CAMDRV_xxx() Functions
// *********************************************************************************************************
// *********************************************************************************************************

// *** All other Customer Defined Actions:  Handled by camera device driver only
        default:
            pr_debug("HAL_CAM_Ctrl(): HAL_CAMDRV_Ctrl(): \r\n");
            result = CAMDRV_ActionCtrl(action, data, callback);
            if (result != RESULT_OK)
            {
                pr_debug("HAL_CAM_Ctrl(): HAL_CAMDRV_Ctrl(): ERROR: Invalid Action=%d \r\n", action);
            }
            break;
    }
    switch (result)
    {
        case CAM_UNSUPPORTED:
            hal_result = HAL_CAM_ERROR_ACTION_NOT_SUPPORTED;
            break;
        case CAM_FAILED:
            hal_result = HAL_CAM_ERROR_INTERNAL_ERROR;
            break;
        case RESULT_DONE:
            hal_result = HAL_CAM_SUCCESS;
            break;
        case RESULT_OK:
            hal_result = HAL_CAM_SUCCESS;
            break;
        default:
            hal_result = HAL_CAM_ERROR_ACTION_NOT_SUPPORTED;
            break;
    }

#if 0
// Release Hal Cam Action Semaphore
    OSSEMAPHORE_Release(sHalCamActionSema);

#endif
    return hal_result;
}



