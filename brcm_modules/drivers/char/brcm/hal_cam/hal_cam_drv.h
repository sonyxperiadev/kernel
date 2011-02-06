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
*   @file   hal_cam_drv.h
*
*   @brief  This file is the Camera Device Independent driver for HAL
*
****************************************************************************/
/**
*   @defgroup   CamDrvGroup   Camera Interface
*   @ingroup    HALCamDrvGroup  
*   @brief      This group is the API driver for Camera Interface
*
*   Device Independent API for Camera .
*
****************************************************************************/

#if !defined( _HAL_CAM_DRV_H__ )
#define _HAL_CAM_DRV_H__

// ---- Include Files -------------------------------------------------------
#include <linux/types.h>
#include <linux/broadcom/camera.h>
#include <cfg_global.h>
#include <linux/broadcom/types.h>
#include "hal_cam.h"


// ---- Public Constants and Types  ----------------------------------------------------
#define MAX_I2C_REGISTER_SIZE	4
#define MAX_I2C_DATA_COUNT	256

//#define LITTLE_ENDIAN_SWITCH

/** Sensor Clock Selection Settings*/
typedef enum
{
    CamDrv_12MHz,                           ///< 12 Mhz clock
    CamDrv_13MHz,                           ///< 13 Mhz clock
    CamDrv_24MHz,                           ///< 24 Mhz clock
    CamDrv_26MHz,                           ///< 26 Mhz clock
    CamDrv_48MHz,                           ///< 48 Mhz clock
    CamDrv_NO_CLK                           ///< No clock
} CamClkSel_t;

/**  Camera Window sizes
*/
typedef enum
{
    CamWindowSize_SQCIF_W=128,      ///< image window size 128x96
    CamWindowSize_SQCIF_H=96,   

    CamWindowSize_QQVGA_W=160,      ///< image window size 160x120
    CamWindowSize_QQVGA_H=120,  

    CamWindowSize_QCIF_W=176,       ///< image window size 176x144
    CamWindowSize_QCIF_H=144,   

    CamWindowSize_QVGA_W=320,       ///< image window size 320x240
    CamWindowSize_QVGA_H=240,   

    CamWindowSize_CIF_W=352,        ///< image window size 352x288
    CamWindowSize_CIF_H=288,    

    CamWindowSize_VGA_W=640,        ///< image window size 640x480
    CamWindowSize_VGA_H=480,    

    CamWindowSize_SVGA_W=800,       ///< image window size 800x600
    CamWindowSize_SVGA_H=600,   

    CamWindowSize_XGA_W=1024,       ///< image window size 1024x768
    CamWindowSize_XGA_H=768,    

    CamWindowSize_SXGA_W=1280,      ///< image window size 1280x1024
    CamWindowSize_SXGA_H=1024,  

    CamWindowSize_UXGA_W=1600,      ///< image window size 1600x1200
    CamWindowSize_UXGA_H=1200,  

    CamWindowSize_QXGA_W=2048,      ///< image window size 640x512
    CamWindowSize_QXGA_H=1536,  

    CamWindowSize_QSXGA_W=2560,     ///< image window size 2560x1920
    CamWindowSize_QSXGA_H=1920 

} CamWindowSize_t;

// Sensor YCbCr Output Sequence
typedef enum 
{
    SensorYCSeq_YCbYCr,
    SensorYCSeq_YCrYCb,
    SensorYCSeq_CbYCrY,
    SensorYCSeq_CrYCbY
}CamSensorYCbCrSeq_t;

/** Camera Sensor Control Sequence Selection*/
typedef enum 
{
    SensorInitPwrUp,
    SensorInitPwrDn,
    SensorPwrUp,
    SensorPwrDn,
    SensorPreFlashEnable,
    SensorFlashEnable,
    SensorFlashDisable,
    SensorTorchEnable,
    SensorTorchDisable,
    LuminancePwrUp,    
    LuminancePwrDn
}CamSensorSeqSel_t;

/** Camera Sensor Interface Control Select */
typedef enum 
{
    GPIO_CNTRL,
    MCLK_CNTRL,
    PAUSE,
    I2C_CNTRL,
    SENSOR_FUNC,
    I2C_CNTRL2
}CamSensorCntrlSel_t;

/** Camera Sensor Interface Control Commands */
typedef enum 
{
    Nop_Cmd,
    GPIO_SetHigh,
    GPIO_SetLow,
    CLK_TurnOn,
    CLK_TurnOff
}CamSensorCntrlCmd_t;

/// Sensor GPIO/Clock Interface Control 
typedef struct {
    CamSensorCntrlSel_t   cntrl_sel;                ///< Interface control select:  GPIO, Clock                       
    UInt32                value;                    ///< GPIO #, Pause time       
    CamSensorCntrlCmd_t   cmd;                      ///< Interface control command            
} CamSensorIntfCntrl_st_t;              

/**  Camera Sync Interface  
*/
typedef enum
{
    SyncRisingEdge      = (1 << 0),                 ///< Camera Controller Sync 
    SyncFallingEdge     = (1 << 1),                 ///< Camera Controller Sync 
    SyncActiveHigh      = (1 << 2),                 ///< Camera Controller Sync 
    SyncActiveLow       = (1 << 3)                  ///< Camera Controller Sync 
} CamSyncIntf;

/** Sensor JPEG Packet Format */
typedef enum
{
    CamJpeg_FixedPkt_VarLine =        0x00000001,   ///< Jpeg Packets: Fixed Packet size, Variable Line Count
    CamJpeg_FixedPadPkt_VarLine =     0x00000002,   ///< Jpeg Packets: Fixed Padded Packet size, Variable Line Count
    CamJpeg_VarPkt_VarLine =          0x00000004    ///< Jpeg Packets: Variable Packet size, Variable Line Count
} CamJpegPacketFormat_t;
/** Sensor Interleaving Mode (Stills and Thumbs, Video and Preview modes)*/
typedef enum
{
    CamInterLeave_SingleFrame =     0x00000001,   ///< Both in Single frame format (both in single Vsync)
    CamInterLeave_AlternateFrame =  0x00000002    ///< Alternate frame format (each has own Vsync)
} CamInterLeaveMode_t;
/** Sensor Interleaving Output Sequence (Stills and Thumbs, Video and Preview modes)*/
typedef enum
{
    CamInterLeave_PreviewFirst =    0x00000001,   ///< Both in Single frame format (both in single Vsync)
    CamInterLeave_PreviewLast =     0x00000002    ///< Alternate frame format (each has own Vsync)
} CamInterLeaveSequence_t;
/** Sensor Interleaving Output Format (Stills and Thumbs, Video and Preview modes)*/
typedef enum
{
    CamInterLeave_Normal =          0x00000001,   ///< Data output in normal output format
    CamInterLeave_PktFormat =       0x00000002    ///< Data output in Packet format
} CamInterLeaveOutputFormat_t;

/** JPEG Packet Markers */
typedef enum
{
    SOF_Marker                  =   0x0000FFC0,     ///< Start of Frame
    HT_Marker                   =   0x0000FFC4,     ///< Huffman Table
    RS_Marker                   =   0x0000FFD0,     ///< Restart Marker (0xFFD0 - 0xFFD7)
    SOI_Marker                  =   0x0000FFD8,     ///< Start of Image
    EOI_Marker                  =   0x0000FFD9,     ///< End of Image Marker
    SOS_Marker                  =   0x0000FFDA,     ///< Start of Scan
    QT_Marker                   =   0x0000FFDB,     ///< Quantization Table
    AS2_Marker                  =   0x0000FFE2      ///< Application Segment 2
} CamJpegMarker_t;


//******************************************************
//
//          Sensor Interface Configuration
//
//******************************************************

/// Sensor CCIR_656 Interface Configuration 
typedef struct {
// Vsync, Hsync, Clock 
    Boolean ext_sync_enable;                    ///<[00] (default)TRUE/FALSE:     (TRUE) CCIR-656 with VSYNC/HSYNC  (FALSE) Embedded VSYNC/HSYNC
    Boolean hsync_control;                      ///<[01] (default)FALSE/TRUE:       (TRUE) HSYNCS only during valid VSYNC  (FALSE) all HSYNCS
    UInt32 vsync_irq_trigger;                   ///<[02] (default)IRQ_RISING_EDGE/IRQ_FALLING_EDGE:       Vsync Irq trigger
    UInt32 vsync_polarity;                      ///<[03] (default)IRQ_ACTIVE_LOW/IRQ_ACTIVE_HIGH:         Vsync active  
    UInt32 hsync_irq_trigger;                   ///<[04] (default)IRQ_RISING_EDGE/IRQ_FALLING_EDGE:       Hsync Irq trigger
    UInt32 hsync_polarity;                      ///<[05] (default)IRQ_ACTIVE_HIGH/IRQ_ACTIVE_LOW:         Hsync active 
    UInt32 data_clock_sample;                   ///<[06] (default)IRQ_RISING_EDGE/IRQ_FALLING_EDGE:       Pixel Clock Sample edge
} CamIntfConfig_CCIR656_st_t;               
    

/// Sensor Input/Output Control: YCbCr(YUV422) Input format = YCbCr=YUV= Y0 U0 Y1 V0  Y2 U2 Y3 V2 ....
typedef struct {
    Boolean yuv_full_range;                     ///<[00] (default)FALSE/TRUE:       (TRUE) FULL RANGE YUV= 1-254  (FALSE) CROPPED YUV=16-240  
    CamSensorYCbCrSeq_t sensor_yc_seq;          ///<[01] (default) SensorYCSeq_YCbYCr
// Currently Unused
    Boolean input_byte_swap;                    ///<[02] (default)FALSE/TRUE:       (TRUE)   (FALSE)
    Boolean input_word_swap;                    ///<[03] (default)FALSE/TRUE:       (TRUE)   (FALSE)
    Boolean output_byte_swap;                   ///<[04] (default)FALSE/TRUE:       (TRUE)   (FALSE)
    Boolean output_word_swap;                   ///<[05] (default)FALSE/TRUE:       (TRUE)   (FALSE)
///< Sensor Color Conversion Coefficients:  color conversion fractional coefficients are scaled by 2^8
///<                                        e.g. for R0 = 1.164, round(1.164 * 256) = 298 or 0x12a
    UInt32 cc_red;                              ///<[06] R1R0:  YUV422 to RGB565 color conversion red
    UInt32 cc_green;                            ///<[07] G1G0:  YUV422 to RGB565 color conversion green
    UInt32 cc_blue;                             ///<[08] B1:    YUV422 to RGB565 color conversion blue
} CamIntfConfig_YCbCr_st_t;                 

/// Sensor Jpeg Settings & Capabilities 
typedef struct {
    UInt32 jpeg_packet_size_bytes;      ///< Bytes/Hsync
    UInt32 jpeg_max_packets;            ///< Hsyncs/Vsync
    CamJpegPacketFormat_t pkt_format;   ///< Jpeg Packet Format 
#if defined(SS_2153) && defined(BROOKLYN_HW)
    UInt8 pad_value;                    ///< padding value
    UInt8 pad_end;                      ///< pad end value        
#endif
}CamIntfConfig_Jpeg_st_t;

/// Sensor Interleaving Settings & Capabilities (Stills n Thumbs, Video n Preview modes)
typedef struct {
    CamInterLeaveMode_t mode;           ///< InterLeaving mode
    CamInterLeaveSequence_t sequence;   ///< InterLeaving Sequence
    CamInterLeaveOutputFormat_t format; ///< InterLeaving Output Format
}CamIntfConfig_InterLeaveMode_st_t;

/// Sensor Packet Non-Standard Marker Info (Stills n Thumbs, Video n Preview modes, SingleFrame)
typedef struct {
    UInt8       marker_bytes;           ///< # of bytes for marker, (= 0 if not used)
    UInt8       pad_bytes;              ///< # of bytes for padding, (= 0 if not used)
    
    Boolean     TN_marker_used;         ///< Thumbnail marker used
    UInt32      SOTN_marker;            ///< Start of Thumbnail marker, (= 0 if not used)
    UInt32      EOTN_marker;            ///< End of Thumbnail marker, (= 0 if not used)
    
    Boolean     SI_marker_used;         ///< Status Info marker used
    UInt32      SOSI_marker;            ///< Start of Status Info marker, (= 0 if not used)
    UInt32      EOSI_marker;            ///< End of Status Info marker, (= 0 if not used)

    Boolean     Padding_used;           ///< Status Padding bytes used
    UInt32      SOPAD_marker;           ///< Start of Padding marker, (= 0 if not used)
    UInt32      EOPAD_marker;           ///< End of Padding marker, (= 0 if not used)
    UInt32      PAD_marker;             ///< Padding Marker, (= 0 if not used)
} CamIntfConfig_PktMarkerInfo_st_t;


/// Sensor I2C Interface Configuration
typedef struct {
    UInt32 i2c_clock_speed;                     ///<[00] max clock speed for I2C
    UInt32 i2c_device_id;                       ///<[01] I2C device ID
    UInt32 i2c_access_mode;                     ///<[02] I2C port
    UInt32 i2c_sub_adr_op;                      ///<[03] I2C sub-address size
    UInt32 i2c_page_reg;                        ///<[04] I2C page register addr (if applicable)
    UInt32 i2c_max_page;                        ///<[05] I2C max page (if applicable)
} CamIntfConfig_I2C_st_t;               

/// Sensor IOCR Interface Configuration
typedef struct {
    Boolean cam_pads_data_pd;                   ///<[00] (default)FALSE/TRUE:     (FALSE)IOCR2 D0-D7 pull-down disabled   (TRUE)IOCR2 D0-D7 pull-down enabled
    Boolean cam_pads_data_pu;                   ///<[01] (default)FALSE/TRUE:     (FALSE)IOCR2 D0-D7 pull-up disabled     (TRUE)IOCR2 D0-D7 pull-up enabled
    Boolean cam_pads_vshs_pd;                   ///<[02] (default)FALSE/TRUE:     (FALSE)IOCR2 Vsync/Hsync pull-down disabled   (TRUE)IOCR2 Vsync/Hsync pull-down enabled
    Boolean cam_pads_vshs_pu;                   ///<[03] (default)FALSE/TRUE:     (FALSE)IOCR2 Vsync/Hsync pull-up disabled     (TRUE)IOCR2 Vsync/Hsync pull-up enabled
    Boolean cam_pads_clk_pd;                    ///<[04] (default)FALSE/TRUE:     (FALSE)IOCR2 CLK pull-down disabled     (TRUE)IOCR2 CLK pull-down enabled
    Boolean cam_pads_clk_pu;                    ///<[05] (default)FALSE/TRUE:     (FALSE)IOCR2 CLK pull-up disabled       (TRUE)IOCR2 CLK pull-up enabled

    UInt32 i2c_pwrup_drive_strength;            ///<[06] (default)IOCR4_CAM_DR_12mA:                                  IOCR I2C2 drive strength
    UInt32 i2c_pwrdn_drive_strength;            ///<[07] (default)IOCR4_CAM_DR_2mA:                                       IOCR I2C2 drive strength
    UInt32 i2c_slew;                            ///<[08] (default)IOCR4_CAM_DR_2mA:                                       IOCR I2C2 drive strength
    
    UInt32 cam_pads_pwrup_drive_strength;       ///<[09] (default)IOCR4_CAM_DR_12mA:                                  IOCR camera pads drive strength in pwr up
    UInt32 cam_pads_pwrdn_drive_strength;       ///<[10] (default)IOCR4_CAM_DR_2mA:                                       IOCR camera pads drive strength in pwr dn
} CamIntfConfig_IOCR_st_t;              

/// Sensor Interface Configuration
typedef struct {
    HAL_CAM_IntConfigCaps_st_t          *sensor_config_caps;
    CamIntfConfig_CCIR656_st_t          *sensor_config_ccir656;
    CamIntfConfig_YCbCr_st_t            *sensor_config_ycbcr;
    CamIntfConfig_I2C_st_t              *sensor_config_i2c;
    CamIntfConfig_IOCR_st_t             *sensor_config_iocr;
    CamIntfConfig_Jpeg_st_t             *sensor_config_jpeg;
    CamIntfConfig_InterLeaveMode_st_t   *sensor_config_interleave_video;
    CamIntfConfig_InterLeaveMode_st_t   *sensor_config_interleave_stills;
//#if defined(SS_2153) && defined(BROOKLYN_HW)
//#else
    CamIntfConfig_PktMarkerInfo_st_t    *sensor_config_pkt_marker_info;
//#endif
} CamIntfConfig_st_t;               
//******************************************************
//******************************************************


/// Camera Image Capture Configuration 
typedef struct
{
    void *              captureCb;                          ///< capture callback function
    CamImageSize_t      captureSize;                        ///< capture resolution
    CamDataFmt_t        captureFormat;                      ///< capture format
    CamCaptureMode_t    captureMode;                        ///< capture mode
    UInt32              camImageByteCnt;                    ///< capture data byte count
    CamDigEffect_t      camDigEffect;                       ///< capture digital effects
    UInt8*              captureBufPtrs[2];                  ///< capture callback pointers
    Int32               captureIdx;                         ///< capture callback index
    FlashLedState_t     captureFlashSt;                     ///< capture flash state
    CamImageSize_t      currentPreviewIndex;                ///< capture index
    Int32               camDmaChan;                         ///< dma channel
} CamCaptureConfig_st_t;
        
/// Camera Window Configuration
typedef struct
{
    CamCaptureMode_t    mode;               ///< capture mode
    CamImageSize_t  image_size;             ///< camera sensor image output size
    CamDataFmt_t    format;                 ///< capture format
    UInt32          res_X;                  ///< requested sensor output width resolution
    UInt32          res_Y;                  ///< requested sensor output height resolution
    UInt32          win_start_X;            ///< camera controller horizontal window start pixel
    UInt32          win_end_X;              ///< camera controller horizontal window end pixel
    UInt32          win_start_Y;            ///< camera controller vertical window start pixel
    UInt32          win_end_Y;              ///< camera controller vertical window end pixel
    UInt32          pad_X;                  ///< padding for horizontal window
    UInt32          pad_Y;                  ///< padding for vertical window
    UInt32          sdfH;                   ///< camera controller horizontal scale down factor
    UInt32          sdfV;                   ///< camera controller vertical scale down factor
    UInt32          window_X;               ///< window width to output (start-end) * sdfH/MAX_SDFH
    UInt32          window_Y;               ///< widow height to output (start-end) * sdfV/MAX_SDFV
    Int32           camDmaChan;             ///< dma channel
    UInt32          dma_X;                  ///< dma horizontal bytes
    UInt32          dma_Y;                  ///< dma vertical lines
    UInt32          dma_mode;               ///< dma mode
    UInt32          dma_lines;              ///< # of dma lines complete
    UInt32          buf_ptrs[2];            ///< dma buffer pointers
    UInt32          buf_index;              ///< dma buffer pointer index
    UInt32          image_size_bytes;       ///< captured image size in bytes (for JPEG)
    Boolean         dma_repeat;             ///< dma repeat flag
} CamWindowConfig_st_t;

typedef struct
{
    UInt32      winVs;                      ///<camera controller vertical window start pixel
    UInt32      winVe;                      ///< camera controller vertical window end pixel
    UInt32      winHs;                      ///< camera controller horizontal window start pixel
    UInt32      winHe;                      ///< camera controller horizontal window end pixel
    UInt32      sdfH;                           ///< camera controller horizontal scale down factor
    UInt32      sdfV;                           ///< camera controller vertical scale down factor
} WindowParams_t;
/// Sensor Resolution Configuration  Settings
typedef struct 
{
    UInt32      resX;                           ///< X resolution of camera
    UInt32      resY;                           ///< Y resolution of Camera
    Int32       previewIndex;                   ///< configuration index of the same resolution specified as resX and resY when camera is in capturing mode; -1 represents the invalid value
    Int32       captureIndex;                   ///< configuration index of the same resolution specified as resX and resY when camera is in previewing mode, -1 represents the invalid value
} CamResolutionConfigure_t;
/// Sensor Window Size Settings
#if defined(SS_2153) && defined(BROOKLYN_HW)
typedef struct
{
    UInt32      pixels;                     ///< pixels per line
    UInt32      lines;                      ///< lines in image
} WindowSizes_t;
#endif
typedef struct
{
    UInt32          resX;                       ///< X resolution of camera
    UInt32          resY;                       ///< Y resolution of Camera
    CamImageSize_t  resolution;                 ///< resolution
} CamResolutionSizes_st_t;
/// Sensor Register configuration Settings
typedef struct {
    UInt8   page;                           
    UInt8   RegID;                      
    UInt8   data[2];                    
} CamConfigure_t;               

// Callback Function Prototypes
//typedef void (*CamSyncDetectCB)(void);
//typedef void (*CamCaptureCB_t)(HAL_CAM_Result_en_t err, UInt32);

//***************************************************************************
//
//  Camera Device Independent API
//
//  These functions are designed to be portable among different Camera devices
//***************************************************************************

/**
 * @addtogroup CamDrvGroup
 * @{
 */

/**
*  Gets Pointer to selected sensor's interface configuration.
*/
//static 
CamIntfConfig_st_t* CAM_GetCameraInterface( 
        CamSensorSelect_t sensor            ///< (in) Camera sensor select.
        );

/**
*  Set Camera Controller to requested output format
*/
HAL_CAM_Result_en_t CAM_SetDataFmt(
        CamDataFmt_t fmt                            ///< (in) camera output format.
        );

/**
*  Initialize Camera for Captures
*/
HAL_CAM_Result_en_t CAM_Init(
        CamSensorSelect_t sensor            ///< (in) Camera sensor select.
        );

/**
*  Enable Camera Interface Module on Baseband
*/
HAL_CAM_Result_en_t CAM_EnableCamMod(
        CamSensorSelect_t sensor            ///< (in) Camera sensor select.
        );

/**
*  Disable Camera Interface Module on Baseband
*/
HAL_CAM_Result_en_t CAM_DisableCamMod(
        CamSensorSelect_t sensor            ///< (in) Camera sensor select.
        );

/**
*       CAM_SetPageModeI2c Set page mode for I2C on camera sensor
*/
void CAM_SetPageModeI2c( 
        UInt8 camPage 
        );

/**
*       CAM_WriteRegI2c write camera sensor registers
*/
void CAM_WriteRegI2c( 
        UInt8 camPage, 
        UInt8 camRegID, 
        UInt8 DataCnt, 
        UInt8 *Data
        );

/**
*       CAM_WriteI2c write camera sensor registers
*/
HAL_CAM_Result_en_t CAM_WriteI2c( 
        UInt16 camRegID,                 ///< (in) 8-bit Sub-Address (register)   
        UInt16 DataCnt,                 ///< (in) count for data bytes to write   
        UInt8 *Data                     ///< (in) pointer to I2C data.  
        );
/**
*       CAM_ReadI2c read camera sensor registers
*/
HAL_CAM_Result_en_t CAM_ReadI2c(
        UInt16 camRegID,                 ///< (in) 8-bit Sub-Address (register)   
        UInt16 DataCnt,                 ///< (in) count for data bytes to read   
        UInt8 *Data                     ///< (in) pointer to I2C data.  
        );

/**
*  Sets Power Off/On Camera interface and sensor
*/
HAL_CAM_Result_en_t CAM_SetPowerMode(
    CamSensorSelect_t   sensor,     ///< (in) camera sensor select
    CamCaptureMode_t    mode        ///< (in) camera interface and sensor mode (CamPowerOff/CamPowerOn)
        );

/**
*  Get Camera Interface and Nominal/Min/Max camera settings
*/
HAL_CAM_Result_en_t CAM_GetCameraSettings(
        HAL_CAM_CamConfig_st_t  *cam_config_st,     ///< (out) HAL_CAM_Config_st_t
        CamCaptureMode_t        mode,               ///< (in) camera sensor select
        CamSensorSelect_t       sensor              ///< (in) camera sensor select
        );

/**
*  Starts video capture and triggers callback when capture ready
*/
HAL_CAM_Result_en_t CAM_CaptureVideo(                  
        CamImageSize_t      size,           ///< (in) capture size CamImageSize_t:
        CamDataFmt_t        format,         ///< (in) capture format CamDataFmt_t:
        CamSensorSelect_t   sensor,         ///< (in) camera sensor select CamSensorSelect_t:
        UInt8*                  buf0,       ///< (in) callback pingpong buffer 0
        UInt8*                  buf1,       ///< (in) callback pingping buffer 1                   
        void *  cb                  ///< (in) callback, will be called for each captured frame
        );

/**
*  Starts preview/video capture with Post Processor Zooming, triggers callback when buffer ready
*/
HAL_CAM_Result_en_t CAM_PreviewPPzoom(                  
    HAL_CAM_Action_window_scale_st_t  *window_scale, ///< (in) captured image size, window, scale down factor
    CamDataFmt_t                      format,       ///< (in) capture format 
    CamSensorSelect_t                 sensor,       ///< (in) camera sensor select
    FlashLedState_t                   flash_mode,   ///< (in) image capture flash mode  (Torch mode only)
    CamCaptureMode_t                  mode,         ///< (in) image capture mode 
    UInt8*                            buf0,         ///< (in) captured image pingpong buffer 0
    UInt8*                            buf1,         ///< (in) captured image pingpong buffer 1
    void *                    cb            ///< (in) callback, will be called for each captured frame
    );

/**
*  Starts image capture with thumbnail and triggers callback when capture ready
*/
HAL_CAM_Result_en_t CAM_CapImageWithThumbNail(
        CamImageSize_t      img_size,           ///< (in) capture size CamImageSize_t:
        CamDataFmt_t        format,             ///< (in) capture format CamDataFmt_t:
        CamSensorSelect_t   sensor,             ///< (in) camera sensor select CamSensorSelect_t:
        UInt8*              buf,                ///< (in) output will be written to this location
        UInt32              thumbnail_width,    ///< (in) thumbnail width
        UInt32              thumbnail_height,   ///< (in) thumbnail height
        CamDataFmt_t        thumbnail_format,   ///< (in) thumbnail format CamDataFmt_t:
        UInt8*              thumbnail_buf,      ///< (in) thumbnail output will be written to this location
        void *      cb                  ///< (in) callback, will be called for each captured frame
        );

/**
*  Starts video capture with viewfinder and triggers callback when capture ready
*/
HAL_CAM_Result_en_t CAM_CapVideoWithViewfinder(                  
        CamImageSize_t      size,               ///< (in) capture size CamImageSize_t:
        CamDataFmt_t        format,             ///< (in) capture format CamDataFmt_t:
        CamSensorSelect_t   sensor,             ///< (in) camera sensor select CamSensorSelect_t:
        UInt8*              buf0,               ///< (in) callback pingpong buffer 0
        UInt8*              buf1,               ///< (in) callback pingping buffer 1                   
        CamImageSize_t      viewfinder_size,    ///< (in) viewfinder size CamImageSize_t:
        CamDataFmt_t        viewfinder_format,  ///< (in) viewfinder format CamDataFmt_t:
        UInt8*              viewfinder_buf0,    ///< (in) viewfinder callback pingpong buffer 0
        UInt8*              viewfinder_buf1,    ///< (in) viewfinder callback pingping buffer 1                   
        void *      cb                  ///< (in) callback, will be called for each captured frame
        );

/**
*  Starts image capture and triggers callback when capture ready
*/
HAL_CAM_Result_en_t CAM_CaptureImage(
        CamImageSize_t  img_size,           ///< (in) capture size CamImageSize_t:
        CamDataFmt_t        format,         ///< (in) capture format CamDataFmt_t:
        CamSensorSelect_t   sensor,         ///< (in) camera sensor select CamSensorSelect_t:
        UInt8*                  buf,        ///< (in) output will be written to this location
        void *  cb                  ///< (in) callback, will be called for each captured frame
        );

/**
*  Starts stills capture with Post Processor Zooming, triggers callback when buffer ready
*/
HAL_CAM_Result_en_t CAM_StillsPPzoom(                  
    HAL_CAM_Action_window_scale_st_t  *window_scale, ///< (in) captured image size, window, scale down factor
    CamDataFmt_t                      format,       ///< (in) capture format 
    CamSensorSelect_t                 sensor,       ///< (in) camera sensor select
    FlashLedState_t                   flash_mode,   ///< (in) image capture flash mode
    UInt8*                            buf0,         ///< (in) stills image buffer 
    void *                    cb            ///< (in) callback, will be called for each captured frame
    );

/**
*  Stops video capture
*/
HAL_CAM_Result_en_t CAM_StopCapture(void);

#if 1
/**
*  Pauses video capture
*/
HAL_CAM_Result_en_t CAM_PauseCapture(void);

/**
*  Resumes video capture
*/
HAL_CAM_Result_en_t CAM_ResumeCapture(void);
#endif

/**
*       CAM_SetWindowScale  Sets the window size and scale down factors for camera controller windowing
*/
HAL_CAM_Result_en_t CAM_SetWindowScale(                        
        CamImageSize_t size,                    ///< (in) image resolution
        UInt32 start_pixel,           ///< (in) offset for start pixel
        UInt32 end_pixel,             ///< (in) offset for stop pixel             
        UInt32 start_line,            ///< (in) offset for start line          
        UInt32 end_line,              ///< (in) offset for stop line         
        UInt32 horizontal_SDF,        ///< (in) scale down factor for horizontal           
        UInt32 vertical_SDF           ///< (in) scale down factor for vertical
        );

/**
*       CAM_GetDispWindowH  Returns viewfinder window height
*/
CamWindowSize_t CAM_GetDispWindowH(void);

/**
*       CAM_GetDispWindowW  Returns viewfinder window width
*/
CamWindowSize_t CAM_GetDispWindowW(void);

/**
*       CAM_GetResAvailable  Returns closest image size to requested width and height
*
*   @return  
*
*       Notes:
*/
CamImageSize_t CAM_GetResAvailable( 
        UInt16 horizontal_pixels,       ///< (in) image horizontal pixels
        UInt16 vertical_pixels          ///< (in) image lines 
        );

/**
*       CAM_GetResolution  Returns width & height of requested resolution
*
*   @return  
*
*       Notes:
*/
HAL_CAM_Result_en_t CAM_GetResolution( 
        CamImageSize_t size, 
        CamCaptureMode_t mode, 
        CamSensorSelect_t sensor,
        HAL_CAM_ResolutionSize_st_t *sensor_size );
/* Set Camera Controller zoom
  @param        (in) UInt8 step
  @return       HAL_CAM_Result_en_t
    @notes:  done with camera sensor.
*/
HAL_CAM_Result_en_t CAM_SetZoom(
        UInt16 step                                                     ///< (in) zoom step
        );

/* Get Camera Controller maximum zoom step
  @param        (in) none
  @return       UInt16 zoom_max_step
    @notes:  done with camera controller.
*/
#if defined(SS_2153) && defined(BROOKLYN_HW)
UInt16 CAM_GetMaxZoom(
        void
        );
#else
UInt16 CAM_GetMaxZoom(
        CamSensorSelect_t   sensor  ///< (in) camera sensor select
        );
#endif
/* Get Camera Controller minimum zoom step
  @param        (in) none
  @return       UInt16 zoom_min_step
    @notes:  done with camera controller.
*/
#if defined(SS_2153) && defined(BROOKLYN_HW)
UInt16 CAM_GetMinZoom(
        void
        );
#else
UInt16 CAM_GetMinZoom(
        CamSensorSelect_t   sensor  ///< (in) camera sensor select
        );
#endif
/* Set Camera sensor zoom
  @param        (in) UInt8 step
  @return       HAL_CAM_Result_en_t
    @notes:  done with camera sensor.
*/
HAL_CAM_Result_en_t CAM_SetCamSensorZoom(
        UInt16 step                         ///< (in) zoom step
        );

/* Set PwrDn, Reset for Camera Initialization
  @param        (in) Boolean enable TRUE/FALSE
  @return       HAL_CAM_Result_en_t
    @notes:  
*/
HAL_CAM_Result_en_t CAM_SetPwrUpReset(
        CamSensorSelect_t   sensor  ///< (in) camera sensor select
        );

/* Set Enable/Disable Camera Flash
  @param        (in) Boolean enable TRUE/FALSE
  @return       HAL_CAM_Result_en_t
    @notes:  
*/
#if defined(SS_2153) && defined(BROOKLYN_HW)
HAL_CAM_Result_en_t CAM_SetFlashEnable(
        FlashLedState_t flash                                      ///< (in) Flash mode
        );
#else
HAL_CAM_Result_en_t CAM_SetFlashEnable(
        Boolean enable                                              ///< (in) TRUE/FALSE to enable flash
        );
#endif

/**
*       CAM_EnableAutoFocus  Set Enable AF
*/
HAL_CAM_Result_en_t CAM_EnableAutoFocus(
        Boolean auto_focus,
        CamSensorSelect_t nSensor
        );
/**
*       CAM_SetEffect  Set Camera for requested effect
*/
HAL_CAM_Result_en_t CAM_SetDigitalEffect(
        CamDigEffect_t  effect,                         ///< (in) ascii string name of effect                           
    CamSensorSelect_t   sensor           ///< (in) camera sensor select
        );



UInt32 CAM_GetJpegSize(
        void
        );


HAL_CAM_Result_en_t CAM_SetEsdCb(UInt32 cb, CamSensorSelect_t sensor);



//***************************************************************************
/**
***************************************************************************
*       Legacy code  No Longer Used
***************************************************************************
*/

#if 0

/**
*  Returns total zoom steps for video
*/
Int32    CAM_GetVideoZoomMax(void);

/**
*  Sets zoom controlled by Camera sensor 
*/
HAL_CAM_Result_en_t CAM_SetCamSensorZoom(
        UInt8 step                          ///< (in) zoom step
        );

/*
*  Set Display mode
*/
HAL_CAM_Result_en_t CAM_SetDisplayMode(
        CamDispMode_t mode                          ///< (in) display mode
        );
/**
*  Set Camera for requested effect
*/
HAL_CAM_Result_en_t CAM_SetEffect(
        const char*  effect_name                            ///< (in) ascii string name of effect                           
        );

// ---- Image Appearance Settings  ----------------------------------------------------

/** Set contrast for preview image.
  @param contrast   [in] contrast level.
  @return       HAL_CAM_Result_en_t 
*/
HAL_CAM_Result_en_t CAM_SetContrast(
        Int8 contrast
        );
/** Set brightness for preview image.
  @param brightness [in] brightness level.
  @return       HAL_CAM_Result_en_t 
*/
HAL_CAM_Result_en_t CAM_SetBrightness(
        Int8 brightness
        );
/** Set Saturation Hue for preview image.  NOT for average user's use.
  @param saturation [in] saturation level.
  @return       HAL_CAM_Result_en_t 
*/
HAL_CAM_Result_en_t CAM_SetSaturation(
        Int8 saturation
        );  
/* Set hue for preview image.  NOT for average user's use.
  @param hue    [in] hue level.
  @return       HAL_CAM_Result_en_t 
*/
HAL_CAM_Result_en_t CAM_SetHue(
        Int8 hue
        );  
/* Set Sharpness for preview image.  NOT for average user's use.
  @param sharpness [in] sharpness level.
  @return       HAL_CAM_Result_en_t 
*/
HAL_CAM_Result_en_t CAM_SetSharpness(
        Int8 sharpness
        );  
/* Set anti shading power for preview image.  NOT for average user's use.
  @param asp    [in] shading power.
  @return       HAL_CAM_Result_en_t 
*/
HAL_CAM_Result_en_t CAM_SetAntiShadingPower(
        Int8 asp
        ); 


/** Set contrast for still capture image.
  @param contrast   [in] contrast level.
  @return       HAL_CAM_Result_en_t 
*/
HAL_CAM_Result_en_t CAM_SetContrastCapture(
        Int8 contrast
        );
/** Set brightness for still capture image.
  @param brightness [in] brightness level.
  @return       HAL_CAM_Result_en_t 
*/
HAL_CAM_Result_en_t CAM_SetBrightnessCapture(
        Int8 brightness
        );
/* Set saturation for still capture image.  NOT for average user's use.
  @param saturation [in] saturation level.
  @return       HAL_CAM_Result_en_t 
*/
HAL_CAM_Result_en_t CAM_SetSaturationCapture(
        Int8 saturation
        );  
/* Set hue for still capture image.  NOT for average user's use.
  @param hue    [in] hue level.
  @return       HAL_CAM_Result_en_t 
*/
HAL_CAM_Result_en_t CAM_SetHueCapture(
        Int8 hue
        );  
/* Set sharpness for still capture image.  NOT for average user's use.
  @param sharpness  [in] sharpness level.
  @return       HAL_CAM_Result_en_t 
*/
HAL_CAM_Result_en_t CAM_SetSharpnessCapture(
        Int8 sharpness
        );  
/* Set anti shading power for still capture image.  NOT for average user's use.
  @param asp    [in] anti shading power level.
  @return       HAL_CAM_Result_en_t 
*/
HAL_CAM_Result_en_t CAM_SetAntiShadingPowerCapture(
        Int8 asp
        );  


// ---- Image Quality Settings  ----------------------------------------------------

/* Set Flicker Control.  NOT Supported.
  @param control    [in] control value.
  @return       HAL_CAM_Result_en_t 
*/
HAL_CAM_Result_en_t CAM_SetFlickerControl(
        CamFlicker_t control
        );
/* Set Gamma Preview.  NOT for average user's use.
  @param gamma  [in] gamma level.
  @return       HAL_CAM_Result_en_t 
*/
HAL_CAM_Result_en_t CAM_SetGamma(
        Int8 gamma
        );  
/* Set Gamma Capture.  NOT for average user's use.
  @param gamma  [in] gamma level for capture.
  @return       HAL_CAM_Result_en_t 
*/
HAL_CAM_Result_en_t CAM_SetGammaCapture(
        Int8 gamma
        );  
/* Load Gamma Table for preview image.
  @param gtable [in] pointer to gamma table.
  @return       HAL_CAM_Result_en_t 
*/
HAL_CAM_Result_en_t CAM_LoadGammaTable(
        UInt8 *gtable
        );
/* Set color gain for preview image.
  @param gain_r [in] gain for red.
  @param gain_g [in] gain for green.
  @param gain_b [in] gain for green.
  @return       HAL_CAM_Result_en_t 
*/
HAL_CAM_Result_en_t CAM_SetColorGain(
        UInt8 gain_r, 
        UInt8 gain_g, 
        UInt8 gain_b
        );


/// ---- Image Orientation Settings  ----------------------------------------------------
/* Flip frame in X direction
  @param none.
  @return       HAL_CAM_Result_en_t 
*/
HAL_CAM_Result_en_t CAM_ToggleXFlip(void);
/* Flip frame in Y direction.
  @param none.
  @return       HAL_CAM_Result_en_t 
*/
HAL_CAM_Result_en_t CAM_ToggleYFlip(void);



// ---- Operation Control Settings  ----------------------------------------------------
/** Set Data Format for camera.
  @param fmt    [in] camera output format.
  @return       HAL_CAM_Result_en_t 
*/
HAL_CAM_Result_en_t CAM_SetDataFmt(
        CamDataFmt_t fmt
        );
/* Set Frame Rate.
  @param fps    [in] frames per second.
  @return       HAL_CAM_Result_en_t 
*/
HAL_CAM_Result_en_t CAM_SetFrameRate(
        UInt8 fps
        );
/* Set window size for camera module.
  @param size   [in] size of window.
  @return       HAL_CAM_Result_en_t 
*/
HAL_CAM_Result_en_t CAM_SetStdSize(
        CamImageSize_t size
        );
/* Set window size for camera module.
  @param width  [in] width of camera image.
  @param height [in] height of camera image.
  @return       HAL_CAM_Result_en_t 
*/
HAL_CAM_Result_en_t CAM_SetUserSize(
        UInt16 width, 
        UInt16 height
        );
/* Enables/Disables auto exposure
  @param exposure   [in] TRUE = exposure on.
  @return       HAL_CAM_Result_en_t 
*/
HAL_CAM_Result_en_t CAM_SetExposure(
        CamExposure_t exposure
        );


/** Set the required Scene mode (or AUTO scene).
    Notice that if any Special Effect is active then it is disabled.
    
    Average users can use this function to use different settings for still capture.
    The AUTO scene mode is good enough for all shooting needs.

  @param scene_mode     [in] the required scene mode.
  @return               HAL_CAM_Result_en_t 
*/
HAL_CAM_Result_en_t CAM_SetSceneMode(
        CamSceneMode_t scene_mode
        );

/** Set the required Digital Effect (Special Effect).

  @param effect     [in] the required Special Effect.
  @return           HAL_CAM_Result_en_t 
*/
//HAL_CAM_Result_en_t CAM_SetDigitalEffect( 
//      CamDigEffect_t effect
//      );

/** Set the required WhiteBalance mode manually or its AUTO mode.
    Notice that if any Special Effect is active it is disabled.

    Only advanced users need to use these. (base on light source)
    For most users the AUTO mode is good enough for all shooting needs.

  @param wb_mode        [in] the required WB mode.
  @return               HAL_CAM_Result_en_t 
*/
HAL_CAM_Result_en_t CAM_SetWBMode(
        CamWB_WBMode_t wb_mode
        );

/* (old) shooting mode or scene mode.
  For compatibility with old/legacy definitions.
  Can be removed if not needed.
*/
HAL_CAM_Result_en_t CAM_SetShotMode(
        CamShotMode_t shot_mode
        );

/* Set the required Picture Frame
  @param buf                        [in] the required Frame buffer.
  @param frame_width        [in] the required Frame frame_width.
  @param frame_height       [in] the required Frame frame_height.
  @return           HAL_CAM_Result_en_t 
*/
HAL_CAM_Result_en_t CAM_SetPictureFrame(
  UInt8     *buf,                       // the pointer to picture frame data
  UInt16     frame_width,       // picture frame width
  UInt16     frame_height       // picture frame height
  );

/**
*       CAM_I2C_Test  Test Camera I2C interface 
*/
HAL_CAM_Result_en_t CAM_I2C_Test(
        UInt16 test, 
        UInt8* i2c_data,
        UInt32 i2c_test_cnt
        );

HAL_CAM_Result_en_t CAM_WriteI2c_Test( 
        UInt8 device_id, 
        UInt16 camRegID, 
        UInt16 DataCnt, 
        UInt8 *Data 
        );

HAL_CAM_Result_en_t CAM_ReadI2c_Test(
        UInt8 device_id, 
        UInt16 camRegID, 
        UInt16 DataCnt, 
        UInt8 *Data
        );



    #endif

struct sens_methods {
    CamIntfConfig_st_t* (*DRV_GetIntfConfig)(CamSensorSelect_t nSensor);
    CamSensorIntfCntrl_st_t* (*DRV_GetIntfSeqSel)(CamSensorSelect_t nSensor,CamSensorSeqSel_t nSeqSel, UInt32 *pLength);
    HAL_CAM_Result_en_t (*DRV_Wakeup)(CamSensorSelect_t sensor);
    HAL_CAM_Result_en_t (*DRV_GetResolution)(CamImageSize_t size,CamCaptureMode_t mode,CamSensorSelect_t sensor,
    HAL_CAM_ResolutionSize_st_t *sensor_size );
    HAL_CAM_Result_en_t (*DRV_SetVideoCaptureMode)(CamImageSize_t image_resolution,CamDataFmt_t image_format,CamSensorSelect_t sensor);
    HAL_CAM_Result_en_t (*DRV_SetFrameRate)(CamRates_t fps, CamSensorSelect_t sensor);
    HAL_CAM_Result_en_t (*DRV_EnableVideoCapture)(CamSensorSelect_t    sensor);
    HAL_CAM_Result_en_t  (*DRV_SetCamSleep)( CamSensorSelect_t sensor );
    UInt32 (*DRV_GetJpegSize)(CamSensorSelect_t sensor, void *data);
    UInt16 * (*DRV_GetJpeg)(short *buf);
    UInt8 * (*DRV_GetThumbnail)(void *buf, UInt32 offset);
    HAL_CAM_Result_en_t (*DRV_DisableCapture)(CamSensorSelect_t sensor);
    HAL_CAM_Result_en_t  (*DRV_DisablePreview)(CamSensorSelect_t    sensor);
    HAL_CAM_Result_en_t (*DRV_CfgStillnThumbCapture)(CamImageSize_t stills_resolution, CamDataFmt_t stills_format,CamImageSize_t thumb_resolution,CamDataFmt_t thumb_format,CamSensorSelect_t sensor );
    void (*DRV_StoreBaseAddress)(void *virt_ptr);
    HAL_CAM_Result_en_t (*DRV_SetSceneMode)(CamSceneMode_t scene_mode, CamSensorSelect_t sensor);
    HAL_CAM_Result_en_t (*DRV_SetWBMode)(CamWB_WBMode_t wb_mode, CamSensorSelect_t sensor );
    HAL_CAM_Result_en_t (*DRV_SetAntiBanding)(CamAntiBanding_t effect, CamSensorSelect_t sensor );
    HAL_CAM_Result_en_t (*DRV_SetFocusMode)(CamFocusControlMode_t effect,CamSensorSelect_t sensor);
    HAL_CAM_Result_en_t (*DRV_SetDigitalEffect)( CamDigEffect_t effect, CamSensorSelect_t sensor);
    HAL_CAM_Result_en_t (*DRV_SetFlashMode)(FlashLedState_t effect, CamSensorSelect_t sensor);
    HAL_CAM_Result_en_t (*DRV_SetJpegQuality)(CamJpegQuality_t effect,CamSensorSelect_t sensor);
    HAL_CAM_Result_en_t (*DRV_TurnOnAF)(CamSensorSelect_t sensor);
    HAL_CAM_Result_en_t (*DRV_TurnOffAF)(CamSensorSelect_t sensor);
	HAL_CAM_Result_en_t (*DRV_SetZoom)(CamZoom_t step,CamSensorSelect_t sensor);


};
#endif  // _HAL_CAM_DRV_H__

/** @} */
