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
*   @file   camdrv_stv0986.i
*
*   @brief  This file is the lower level driver API of stv0986 ISP with vb6850 camera sensor
*               (3M 2048*1536 Pixel CMOS Sensor), vs6555 camera sensor (VGA 640x480 Pixel CMOS Sensor),
*
*/
/**
 * @addtogroup CamDrvGroup
 * @{
 */

//****************************************************************************
//                          Include block
//****************************************************************************
#include <stdarg.h>
#include <stddef.h>

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
#include <linux/semaphore.h>

#include <mach/reg_camera.h>
#include <mach/reg_lcd.h>

#include <mach/reg_clkpwr.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/atomic.h>

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
#include "cam_cntrl_2153.h"
#include "camdrv_dev.h"

//****************************************************************************
//                      definition and type definitions block
//****************************************************************************
#define AUTOFOCUS_ENABLED

//#define CAMDBG_STATUS
//#define CAMDBG_ZOOM

// Camera GPIO Defines
#define	HAL_CAM1_STANDBY								0xFFFF
#define	HAL_CAM2_STANDBY								0xFFFF
#define	HAL_FLASH_CHARGED								0xFFFF
#define HAL_CAM_FLASH_TORCH								0xFFFF
#define	HAL_CAM_FLASH_EN								0xFFFF
#define	HAL_CAM_RESET									0xFFFF
#define	HAL_CAM_RESET_A									0xFFFF
#define	HAL_CAM_RESET_B									0xFFFF
#define	HAL_CAM_EN										0xFFFF
#define	HAL_CAM_PD										22


//******************************************************************************
// start of CAM configuration
//******************************************************************************
/** Sensor select */
typedef enum
{
    SensorVS6555,
    SensorVS6750,
    SensorVB6850,
    SensorUnused
} CamSensor_t;
/** Sensor Color matrix register address */
typedef enum 
{ 
    ColEng0_Sensor0,    
    ColEng0_Sensor1,
    ColEng1_Sensor0,    
    ColEng1_Sensor1 
} Stv0986_RegAddr_ColMat_t;

//************************************************************************************
//  Sensor Resolution Tables: 
//************************************************************************************
// Resolutions & Sizes available for VB6850 Sensor (QXGA max)
static CamResolutionConfigure_t  sSensorResInfo_VB6850_st[] =
{
// width    height  Preview_Capture_Index       Still_Capture_Index
  {128,     96,     CamImageSize_SQCIF,         CamImageSize_SQCIF  },      //  CamImageSize_SQCIF
  {160,     120,    CamImageSize_QQVGA,         CamImageSize_QQVGA  },      //  CamImageSize_QQVGA
  {176,     144,    CamImageSize_QCIF,          CamImageSize_QCIF   },      //  CamImageSize_QCIF
  {240,     180,    -1,                         -1                  },      //  CamImageSize_240x180
  {240,     320,    -1,                         -1                  },      //  CamImageSize_R_QVGA
  {320,     240,    CamImageSize_QVGA,          CamImageSize_QVGA   },      //  CamImageSize_QVGA
  {352,     288,    CamImageSize_CIF,           CamImageSize_CIF    },      //  CamImageSize_CIF
  {426,     320,    -1,                         -1                  },      //  CamImageSize_426x320
  {640,     480,    CamImageSize_VGA,           CamImageSize_VGA    },      //  CamImageSize_VGA
  {800,     600,    CamImageSize_SVGA,          CamImageSize_SVGA   },      //  CamImageSize_SVGA 
  {1024,    768,    CamImageSize_XGA,           CamImageSize_XGA    },      //  CamImageSize_XGA 
  {1280,    960,    -1,                         -1                  },      //  CamImageSize_4VGA
  {1280,    1024,   -1,                         CamImageSize_SXGA   },      //  CamImageSize_SXGA
  {1600,    1200,   -1,                         CamImageSize_UXGA   },      //  CamImageSize_UXGA
  {2048,    1536,   -1,                         CamImageSize_QXGA   },      //  CamImageSize_QXGA
  {2560,    2048,   -1,                         -1                  }       //  CamImageSize_QSXGA
};

// Resolutions & Sizes available for VS6555 Sensor (VGA max)
static CamResolutionConfigure_t  sSensorResInfo_VS6555_st[] =
{
// width    height  Preview_Capture_Index       Still_Capture_Index
  {128,     96,     CamImageSize_SQCIF,         CamImageSize_SQCIF  },      //  CamImageSize_SQCIF
  {160,     120,    CamImageSize_QQVGA,         CamImageSize_QQVGA  },      //  CamImageSize_QQVGA
  {176,     144,    CamImageSize_QCIF,          CamImageSize_QCIF   },      //  CamImageSize_QCIF
  {240,     180,    -1,                         -1                  },      //  CamImageSize_240x180
  {240,     320,    -1,                         -1                  },      //  CamImageSize_R_QVGA
  {320,     240,    CamImageSize_QVGA,          CamImageSize_QVGA   },      //  CamImageSize_QVGA
  {352,     288,    CamImageSize_CIF,           CamImageSize_CIF    },      //  CamImageSize_CIF
  {426,     320,    -1,                         -1                  },      //  CamImageSize_426x320
  {640,     480,    CamImageSize_VGA,           CamImageSize_VGA    },      //  CamImageSize_VGA
  {800,     600,    -1,                         -1                  },      //  CamImageSize_SVGA 
  {1024,    768,    -1,                         -1                  },      //  CamImageSize_XGA 
  {1280,    960,    -1,                         -1                  },      //  CamImageSize_4VGA
  {1280,    1024,   -1,                         -1                  },      //  CamImageSize_SXGA
  {1600,    1200,   -1,                         -1                  },      //  CamImageSize_UXGA
  {2048,    1536,   -1,                         -1                  },      //  CamImageSize_QXGA
  {2560,    2048,   -1,                         -1                  }       //  CamImageSize_QSXGA
};

//************************************************************************************
//  Power On/Off Tables for Main Sensor
//************************************************************************************

//---------Sensor Power On
static CamSensorIntfCntrl_st_t  CamPowerOnSeq[] = 
{
// -------Turn everything OFF   
    {GPIO_CNTRL, HAL_CAM_RESET,    GPIO_SetLow},
    {MCLK_CNTRL, CamDrv_NO_CLK,    CLK_TurnOff},
    {GPIO_CNTRL, HAL_CAM_EN,       GPIO_SetLow},
    {GPIO_CNTRL, HAL_CAM_PD,       GPIO_SetLow},
    {PAUSE,      50,               Nop_Cmd},
// -------Turn On Power to ISP
    {GPIO_CNTRL, HAL_CAM_EN,       GPIO_SetHigh},
    {PAUSE,      50,               Nop_Cmd},
// -------Enable Clock to Cameras @ Main clock speed
    {MCLK_CNTRL, CamDrv_13MHz,     CLK_TurnOn},
    {PAUSE,      1,                Nop_Cmd},
// -------Raise PwrDn to ISP
    {GPIO_CNTRL, HAL_CAM_PD,       GPIO_SetHigh},
    {PAUSE,      10,               Nop_Cmd},
// -------Raise Reset to ISP
    {GPIO_CNTRL, HAL_CAM_RESET,    GPIO_SetHigh},
    {PAUSE,      10,                Nop_Cmd}
};

//---------Sensor Power Off
static CamSensorIntfCntrl_st_t  CamPowerOffSeq[] = 
{
// -------Lower Reset to ISP    
    {GPIO_CNTRL, HAL_CAM_RESET,    GPIO_SetLow},
    {PAUSE,      5,                Nop_Cmd},
// -------Disable Clock to Cameras 
    {MCLK_CNTRL, CamDrv_NO_CLK,    CLK_TurnOff},
// -------Turn Power OFF    
    {GPIO_CNTRL, HAL_CAM_PD,       GPIO_SetLow},
    {GPIO_CNTRL, HAL_CAM_EN,       GPIO_SetLow},
    {PAUSE,      50,               Nop_Cmd}
};

//---------Sensor Flash Enable
static CamSensorIntfCntrl_st_t  CamFlashEnable[] = 
{
// -------Enable Flash
    {GPIO_CNTRL, HAL_CAM_FLASH_EN, GPIO_SetHigh},
    {PAUSE,      5,                Nop_Cmd}
};

//---------Sensor Flash Disable
static CamSensorIntfCntrl_st_t  CamFlashDisable[] = 
{
// -------Disable Flash
    {GPIO_CNTRL, HAL_CAM_FLASH_EN, GPIO_SetLow},
    {PAUSE,      5,                Nop_Cmd}
};

/** Primary Sensor Configuration and Capabilities  */
static HAL_CAM_IntConfigCaps_st_t CamPrimaryCfgCap_st = 
{
    // CamSensorOutputCaps_st_t
    {
        CamDataFmtYCbCr |               ///< UInt32 formats;   CamDataFmt_t bit masked
        CamDataFmtJPEG,
        2048,                           ///< UInt32 max_width;   Maximum width resolution
        1536,                           ///< UInt32 max_height;  Maximum height resolution
        CamImageSize_CIF,               ///< UInt32 ViewFinderResolutions;  ViewFinder Resolutions (Maximum Resolution for now)
        CamImageSize_XGA,               ///< UInt32 VideoResolutions;  Video Resolutions (Maximum Resolution for now)
        CamImageSize_QXGA,              ///< UInt32 StillsResolutions;  Stills Resolutions (Maximum Resolution for now) 
        1,                              ///< UInt32 pre_frame_video;  frames to throw out for ViewFinder/Video capture (total= pre_frame_video+1
        9,                              ///< UInt32 pre_frame_still;  frames to throw out for Stills capture (total= pre_frame_still+1
        TRUE,                           ///< Boolean JpegCapable;     Sensor Jpeg Capable: TRUE/FALSE:
        TRUE,                           ///< Boolean StillnThumbCapable;     Sensor Still and Thumbnail Capable: TRUE/FALSE:
        TRUE                            ///< Boolean VideonViewfinderCapable;     Sensor Video and Viewfinder Capable: TRUE/FALSE:
    },
 
    ///< Focus Settings & Capabilities:  CamFocusControl_st_t focus_control_st;
    {
    #ifdef AUTOFOCUS_ENABLED
        CamFocusControlAuto,            ///< CamFocusControlMode_t default_setting=CamFocusControlOff;
        CamFocusControlAuto,            ///< CamFocusControlMode_t cur_setting;
        CamFocusControlOn |             ///< UInt32 settings;  Settings Allowed: CamFocusControlMode_t bit masked
        CamFocusControlOff |
        CamFocusControlAuto |
        CamFocusControlAutoLock |
        CamFocusControlCentroid |
        CamFocusControlQuickSearch |
        CamFocusControlInfinity |
        CamFocusControlMacro
    #else
        CamFocusControlOff,             ///< CamFocusControlMode_t default_setting=CamFocusControlOff;
        CamFocusControlOff,             ///< CamFocusControlMode_t cur_setting;
        CamFocusControlOff              ///< UInt32 settings;  Settings Allowed: CamFocusControlMode_t bit masked
    #endif
    },
    ///< Digital Zoom Settings & Capabilities:  CamDigitalZoomMode_st_t digital_zoom_st;        
    {
        CamZoom_1_0,                    ///< CamZoom_t default_setting;  default=CamZoom_1_0:  Values allowed  CamZoom_t
        CamZoom_1_0,                    ///< CamZoom_t cur_setting;  CamZoom_t
        CamZoom_4_0,                    ///< CamZoom_t max_zoom;  Max Zoom Allowed (256/max_zoom = *zoom)
        TRUE                            ///< Boolean capable;  Sensor capable: TRUE/FALSE:
    },
    ///< Sensor ESD Settings & Capabilities:  CamESD_st_t esd_st;
    {
        0x01,                           ///< UInt8 ESDTimer;  Periodic timer to retrieve the camera status (ms)
        FALSE                           ///< Boolean capable;  TRUE/FALSE:
    },
    ///< Sensor version string
    "STV0986+VB6850+VS6555"
};                 
/** Secondary Sensor Configuration and Capabilities  */
static HAL_CAM_IntConfigCaps_st_t CamSecondaryCfgCap_st = 
{
    // CamSensorOutputCaps_st_t
    {
        CamDataFmtYCbCr |               ///< UInt32 formats;   CamDataFmt_t bit masked
        CamDataFmtJPEG,
        640,                            ///< UInt32 max_width;   Maximum width resolution
        480,                            ///< UInt32 max_height;  Maximum height resolution
        CamImageSize_VGA,               ///< UInt32 ViewFinderResolutions;  ViewFinder Resolutions (Maximum Resolution for now)
        CamImageSize_XGA,               ///< UInt32 VideoResolutions;  Video Resolutions (Maximum Resolution for now)
        CamImageSize_QXGA,              ///< UInt32 StillsResolutions;  Stills Resolutions (Maximum Resolution for now) 
        1,                              ///< UInt32 pre_frame_video;  frames to throw out for ViewFinder/Video capture (total= pre_frame_video+1
        9,                              ///< UInt32 pre_frame_still;  frames to throw out for Stills capture (total= pre_frame_still+1
        TRUE,                           ///< Boolean JpegCapable;     Sensor Jpeg Capable: TRUE/FALSE:
        TRUE,                           ///< Boolean StillnThumbCapable;     Sensor Still and Thumbnail Capable: TRUE/FALSE:
        TRUE                            ///< Boolean VideonViewfinderCapable;     Sensor Video and Viewfinder Capable: TRUE/FALSE:
    },
    ///< Focus Settings & Capabilities:  CamFocusControl_st_t focus_control_st;
    {
        CamFocusControlOff,             ///< CamFocusControlMode_t default_setting=CamFocusControlOff;
        CamFocusControlOff,             ///< CamFocusControlMode_t cur_setting;
        CamFocusControlOff              ///< UInt32 settings;  Settings Allowed: CamFocusControlMode_t bit masked
    },
    ///< Digital Zoom Settings & Capabilities:  CamDigitalZoomMode_st_t digital_zoom_st;        
    {
        CamZoom_1_0,                    ///< CamZoom_t default_setting;  default=CamZoom_1_0:  Values allowed  CamZoom_t
        CamZoom_1_0,                    ///< CamZoom_t cur_setting;  CamZoom_t
        CamZoom_4_0,                    ///< CamZoom_t max_zoom;  Max Zoom Allowed (256/max_zoom = *zoom)
        TRUE                            ///< Boolean capable;  Sensor capable: TRUE/FALSE:
    },
    ///< Sensor ESD Settings & Capabilities:  CamESD_st_t esd_st;
    {
        0x01,                           ///< UInt8 ESDTimer;  Periodic timer to retrieve the camera status (ms)
        FALSE                           ///< Boolean capable;  TRUE/FALSE:
    },
    ///< Sensor version string
    "STV0986+VB6850+VS6555"
};                 
  
//---------Sensor Primary Configuration CCIR656
static CamIntfConfig_CCIR656_st_t  CamPrimaryCfg_CCIR656_st = 
{
// Vsync, Hsync, Clock 
    TRUE,                   //[00] Boolean ext_sync_enable;    (default)TRUE: CCIR-656 with external VSYNC/HSYNC   FALSE: Embedded VSYNC/HSYNC
    TRUE,                   //[01] Boolean hsync_control;      (default)FALSE: FALSE=all HSYNCS  TRUE: HSYNCS only during valid VSYNC  
    SyncRisingEdge,         //[02] UInt32 vsync_irq_trigger;   (default)SyncRisingEdge/SyncFallingEdge:       Vsync Irq trigger
    SyncActiveLow,          //[03] UInt32 vsync_polarity;      (default)SyncActiveLow/SyncActiveHigh:         Vsync active  
    SyncRisingEdge,         //[04] UInt32 hsync_irq_trigger;   (default)SyncRisingEdge/SyncFallingEdge:       Hsync Irq trigger
    SyncActiveHigh,         //[05] UInt32 hsync_polarity;      (default)SyncActiveLow/SyncActiveHigh:         Hsync active 
    SyncRisingEdge          //[06] UInt32 data_clock_sample;   (default)SyncRisingEdge/SyncFallingEdge:       Pixel Clock Sample edge
};

//---------Sensor Primary Configuration YCbCr Input
static CamIntfConfig_YCbCr_st_t  CamPrimaryCfg_YCbCr_st = 
{
// YCbCr(YUV422) Input format = YCbCr=YUV= Y0 U0 Y1 V0  Y2 U2 Y3 V2 ....
    TRUE,                   //[00] Boolean yuv_full_range;     (default)FALSE: CROPPED YUV=16-240  TRUE: FULL RANGE YUV= 1-254  
    SensorYCSeq_CbYCrY,     //[01] CamSensorYCbCrSeq_t sensor_yc_seq;    (default) SensorYCSeq_YCbYCr                               

// Currently Unused
    FALSE,                  //[02] Boolean input_byte_swap;    Currently UNUSED!! (default)FALSE:  TRUE:
    FALSE,                  //[03] Boolean input_word_swap;    Currently UNUSED!! (default)FALSE:  TRUE:
    FALSE,                  //[04] Boolean output_byte_swap;   Currently UNUSED!! (default)FALSE:  TRUE:
    FALSE,                  //[05] Boolean output_word_swap;   Currently UNUSED!! (default)FALSE:  TRUE:

// Sensor Color Conversion Coefficients:  color conversion fractional coefficients are scaled by 2^8
//                       e.g. for R0 = 1.164, round(1.164 * 256) = 298 or 0x12a
    CAM_COLOR_R1R0,         //[06] UInt32 cc_red R1R0;          YUV422 to RGB565 color conversion red
    CAM_COLOR_G1G0,         //[07] UInt32 cc_green G1G0;        YUV422 to RGB565 color conversion green
    CAM_COLOR_B1            //[08] UInt32 cc_blue B1;           YUV422 to RGB565 color conversion blue
};

//---------Sensor Primary Configuration I2C
static CamIntfConfig_I2C_st_t CamPrimaryCfg_I2C_st = 
{
    0x00, //I2C_SPD_430K,           //[00] UInt32 i2c_clock_speed;       max clock speed for I2C
    I2C_CAM_DEVICE_ID,      //[01] UInt32 i2c_device_id;         I2C device ID
    0x00, //I2C_BUS2_ID,            //[02] I2C_BUS_ID_t i2c_access_mode; I2C baseband port
    0x00, //I2CSUBADDR_16BIT,       //[03] I2CSUBADDR_t i2c_sub_adr_op;  I2C sub-address size
    0xFFFF,                 //[04] UInt32 i2c_page_reg;          I2C page register addr (if applicable)  **UNUSED by this driver**
    I2C_CAM_MAX_PAGE        //[05] UInt32 i2c_max_page;          I2C max page (if not used by camera drivers, set = 0xFFFF) **UNUSED by this driver**
};

//---------Sensor Primary Configuration IOCR
static CamIntfConfig_IOCR_st_t CamPrimaryCfg_IOCR_st = 
{
    FALSE,              //[00] Boolean cam_pads_data_pd;      (default)FALSE: IOCR2 D0-D7 pull-down disabled       TRUE: IOCR2 D0-D7 pull-down enabled
    FALSE,              //[01] Boolean cam_pads_data_pu;      (default)FALSE: IOCR2 D0-D7 pull-up disabled         TRUE: IOCR2 D0-D7 pull-up enabled
    FALSE,              //[02] Boolean cam_pads_vshs_pd;        (default)FALSE: IOCR2 Vsync/Hsync pull-down disabled TRUE: IOCR2 Vsync/Hsync pull-down enabled
    FALSE,              //[03] Boolean cam_pads_vshs_pu;        (default)FALSE: IOCR2 Vsync/Hsync pull-up disabled   TRUE: IOCR2 Vsync/Hsync pull-up enabled
    FALSE,              //[04] Boolean cam_pads_clk_pd;         (default)FALSE: IOCR2 CLK pull-down disabled         TRUE: IOCR2 CLK pull-down enabled
    FALSE,              //[05] Boolean cam_pads_clk_pu;         (default)FALSE: IOCR2 CLK pull-up disabled           TRUE: IOCR2 CLK pull-up enabled
    
    7 << 12,   //[06] UInt32 i2c_pwrup_drive_strength;   (default)IOCR4_CAM_DR_12mA:   IOCR drive strength
    0x00,               //[07] UInt32 i2c_pwrdn_drive_strength;   (default)0x00:    I2C2 disabled
    0x00,               //[08] UInt32 i2c_slew;                           (default) 0x00: 42ns

    7 << 12,   //[09] UInt32 cam_pads_pwrup_drive_strength;   (default)IOCR4_CAM_DR_12mA:  IOCR drive strength
    1 << 12     //[10] UInt32 cam_pads_pwrdn_drive_strength;   (default)IOCR4_CAM_DR_2mA:   IOCR drive strength
};

//---------Sensor Primary Configuration JPEG
static CamIntfConfig_Jpeg_st_t CamPrimaryCfg_Jpeg_st = 
{
    512,                            ///< UInt32 jpeg_packet_size_bytes;     Bytes/Hsync
    1536,                           ///< UInt32 jpeg_max_packets;           Max Hsyncs/Vsync = (3.2Mpixels/4) / 512
    CamJpeg_FixedPkt_VarLine,       ///< CamJpegPacketFormat_t pkt_format;  Jpeg Packet Format
};

//---------Sensor Primary Configuration Video n ViewFinder
static CamIntfConfig_InterLeaveMode_st_t CamPrimaryCfg_InterLeaveVideo_st = 
{
    CamInterLeave_SingleFrame,      ///< CamInterLeaveMode_t mode;              Interleave Mode
    CamInterLeave_PreviewLast,      ///< CamInterLeaveSequence_t sequence;      InterLeaving Sequence
    CamInterLeave_PktFormat         ///< CamInterLeaveOutputFormat_t format;    InterLeaving Output Format
};

//---------Sensor Primary Configuration Stills n Thumbs
static CamIntfConfig_InterLeaveMode_st_t CamPrimaryCfg_InterLeaveStills_st = 
{
    CamInterLeave_SingleFrame,      ///< CamInterLeaveMode_t mode;              Interleave Mode
    CamInterLeave_PreviewLast,      ///< CamInterLeaveSequence_t sequence;      InterLeaving Sequence
    CamInterLeave_PktFormat         ///< CamInterLeaveOutputFormat_t format;    InterLeaving Output Format
};

//---------Sensor Primary Configuration Stills n Thumbs
static CamIntfConfig_PktMarkerInfo_st_t CamPrimaryCfg_PktMarkerInfo_st = 
{
    2,          ///< UInt8       marker_bytes; # of bytes for marker, (= 0 if not used)
    0,          ///< UInt8       pad_bytes; # of bytes for padding, (= 0 if not used)
    
    TRUE,       ///< Boolean     TN_marker_used; Thumbnail marker used
    0xFFBE,     ///< UInt32      SOTN_marker; Start of Thumbnail marker, (= 0 if not used)
    0xFFBF,     ///< UInt32      EOTN_marker; End of Thumbnail marker, (= 0 if not used)
    
    TRUE,       ///< Boolean     SI_marker_used; Status Info marker used
    0xFFBC,     ///< UInt32      SOSI_marker; Start of Status Info marker, (= 0 if not used)
    0xFFBD,     ///< UInt32      EOSI_marker; End of Status Info marker, (= 0 if not used)

    FALSE,      ///< Boolean     Padding_used; Status Padding bytes used
    0x0000,     ///< UInt32      SOPAD_marker; Start of Padding marker, (= 0 if not used)
    0x0000,     ///< UInt32      EOPAD_marker; End of Padding marker, (= 0 if not used)
    0x0000      ///< UInt32      PAD_marker; Padding Marker, (= 0 if not used)
};

//---------Sensor Primary Configuration
static CamIntfConfig_st_t  CamSensorCfg_st = 
{
    &CamPrimaryCfgCap_st,               // *sensor_config_caps;
    &CamPrimaryCfg_CCIR656_st,          // *sensor_config_ccir656;
    &CamPrimaryCfg_YCbCr_st,            // *sensor_config_ycbcr;
    &CamPrimaryCfg_I2C_st,              // *sensor_config_i2c;
    &CamPrimaryCfg_IOCR_st,             // *sensor_config_iocr;
    &CamPrimaryCfg_Jpeg_st,             // *sensor_config_jpeg;
    &CamPrimaryCfg_InterLeaveVideo_st,  // *sensor_config_interleave_video;
    &CamPrimaryCfg_InterLeaveStills_st, // *sensor_config_interleave_stills;
    &CamPrimaryCfg_PktMarkerInfo_st     // *sensor_config_pkt_marker_info;
};

// --------Primary Sensor Frame Rate Settings
static CamFrameRate_st_t PrimaryFrameRate_st =
{
    CamRate_25,                     ///< CamRates_t default_setting; 
    CamRate_25,                     ///< CamRates_t cur_setting; 
    CamRate_25                      ///< CamRates_t max_setting;
};

// --------Secondary Sensor Frame Rate Settings
static CamFrameRate_st_t SecondaryFrameRate_st =
{
    CamRate_15,                     ///< CamRates_t default_setting; 
    CamRate_15,                     ///< CamRates_t cur_setting; 
    CamRate_15                      ///< CamRates_t max_setting;
};

//---------FLASH/TORCH State
static FlashLedState_t  stv0986_sys_flash_state = Flash_Off;
static Boolean          stv0986_fm_is_on        = FALSE;    
static Boolean          stv0986_torch_is_on     = FALSE;    

// --------Primary Sensor Flash State Settings
static CamFlashLedState_st_t PrimaryFlashState_st =
{
    Flash_Off,                      // FlashLedState_t default_setting:
    Flash_Off,                      // FlashLedState_t cur_setting;       
    (Flash_Off |                    // Settings Allowed: bit mask
        Flash_On |                       
        Torch_On |                       
        FlashLight_Auto )
};

// --------Secondary Sensor Flash State Settings
static CamFlashLedState_st_t SecondaryFlashState_st =
{
    Flash_Off,                      // FlashLedState_t default_setting:
    Flash_Off,                      // FlashLedState_t cur_setting;       
    Flash_Off                       // Settings Allowed: bit mask
};

// --------Sensor Rotation Mode Settings
static CamRotateMode_st_t RotateMode_st =
{
    CamRotate0,                     // CamRotate_t default_setting:
    CamRotate0,                     // CamRotate_t cur_setting;       
    CamRotate0                      // Settings Allowed: bit mask
};

// --------Sensor Mirror Mode Settings
static CamMirrorMode_st_t PrimaryMirrorMode_st =
{
    CamMirrorNone,                  // CamMirror_t default_setting:
    CamMirrorNone,                  // CamMirror_t cur_setting;       
    (CamMirrorNone |                // Settings Allowed: bit mask
        CamMirrorHorizontal |                       
        CamMirrorVertical)
};

// --------Sensor Mirror Mode Settings
static CamMirrorMode_st_t SecondaryMirrorMode_st =
{
    CamMirrorNone,                  // CamMirror_t default_setting:
    CamMirrorNone,                  // CamMirror_t cur_setting;       
    CamMirrorNone                   // Settings Allowed: bit mask
};

// --------Sensor Image Quality Settings
static CamSceneMode_st_t SceneMode_st =
{
    CamSceneMode_Auto,              // CamSceneMode_t default_setting:
    CamSceneMode_Auto,              // CamSceneMode_t cur_setting;        
    CamSceneMode_Auto               // Settings Allowed: bit mask
};
static CamDigitalEffect_st_t DigitalEffect_st =
{
    CamDigEffect_NoEffect,          // CamDigEffect_t default_setting:
    CamDigEffect_NoEffect,          // CamDigEffect_t cur_setting;        
    (CamDigEffect_NoEffect |        // Settings Allowed: bit mask
        CamDigEffect_MonoChrome |                       
        CamDigEffect_NegColor |                       
        CamDigEffect_SolarizeColor )
};
static CamFlicker_st_t Flicker_st =
{
    CamFlicker50Hz,                 // CamFlicker_t default_setting:
    CamFlicker50Hz,                 // CamFlicker_t cur_setting;      
    (CamFlickerAuto |               // Settings Allowed: bit mask
        CamFlicker50Hz |                       
        CamFlicker60Hz )
};
static CamWBMode_st_t WBmode_st =
{
    CamWB_Auto,                     // CamWB_WBMode_t default_setting:
    CamWB_Auto,                     // CamWB_WBMode_t cur_setting;        
    (CamWB_Auto |                   // Settings Allowed: bit mask
        CamWB_Off |                       
        CamWB_Daylight |                       
        CamWB_Tungsten |                       
        CamWB_DaylightFluorescent |                       
        CamWB_Sunset |                       
        CamWB_Flash )
};
static CamExposure_st_t Exposure_st =
{
    CamExposure_Enable,             // CamExposure_t default_setting:
    CamExposure_Enable,             // CamExposure_t cur_setting;     
    CamExposure_Enable              // Settings Allowed: bit mask
};
static CamMeteringType_st_t Metering_st =
{
    CamMeteringType_Auto,           // CamMeteringType_t default_setting:
    CamMeteringType_Auto,           // CamMeteringType_t cur_setting;     
    CamMeteringType_Auto            // Settings Allowed: bit mask
};
static CamSensitivity_st_t Sensitivity_st =
{
    CamSensitivity_Auto,            // CamSensitivity_t default_setting:
    CamSensitivity_Auto,            // CamSensitivity_t cur_setting;      
    CamSensitivity_Auto             // Settings Allowed: bit mask
};
static CamFunctionEnable_st_t CamWideDynRange_st =
{
    FALSE,                          // Boolean default_setting;
    FALSE,                          // Boolean cur_setting; 
    FALSE                           // Boolean configurable; 
};
static CamImageAppearance_st_t Contrast_st =
{
    CamContrast_Nom,             // Int8 default_setting:
    CamContrast_Nom,             // Int8 cur_setting;       
    TRUE                         // Boolean configurable;
};
static CamImageAppearance_st_t Brightness_st =
{
    CamBrightness_Nom,           // Int8 default_setting:
    CamBrightness_Nom,           // Int8 cur_setting;       
    FALSE                        // Boolean configurable;
};
static CamImageAppearance_st_t Saturation_st =
{
    CamSaturation_Nom,           // Int8 default_setting:
    CamSaturation_Nom,           // Int8 cur_setting;       
    TRUE                         // Boolean configurable;
};
static CamImageAppearance_st_t Hue_st =
{
    CamHue_Nom,                  // Int8 default_setting:
    CamHue_Nom,                  // Int8 cur_setting;       
    FALSE                        // Boolean configurable;
};
static CamImageAppearance_st_t Gamma_st =
{
    CamGamma_Nom,                // Int8 default_setting:
    CamGamma_Nom,                // Int8 cur_setting;       
    TRUE                         // Boolean configurable;
};
static CamImageAppearance_st_t Sharpness_st =
{
    CamSharpness_Nom,            // Int8 default_setting:
    CamSharpness_Nom,            // Int8 cur_setting;       
    FALSE                        // Boolean configurable;
};
static CamImageAppearance_st_t AntiShadingPower_st =
{
    CamAntiShadingPower_Nom,     // Int8 default_setting:
    CamAntiShadingPower_Nom,     // Int8 cur_setting;       
    FALSE                        // Boolean configurable;
};
static CamJpegQuality_st_t JpegQuality_st =
{
    CamJpegQuality_Max,         // Int8 default_setting:
    CamJpegQuality_Max,         // Int8 cur_setting;       
    TRUE                        // Boolean configurable;
};

static CamFunctionEnable_st_t CamFrameStab_st =
{
    FALSE,                          // Boolean default_setting;
    FALSE,                          // Boolean cur_setting;    
    FALSE                           // Boolean configurable;   
};
static CamFunctionEnable_st_t CamAntiShake_st =
{
    FALSE,                          // Boolean default_setting;
    FALSE,                          // Boolean cur_setting;    
    FALSE                           // Boolean configurable;   
};
static CamFunctionEnable_st_t CamFaceDetection_st =
{
    FALSE,                          // Boolean default_setting;
    FALSE,                          // Boolean cur_setting;    
    FALSE                           // Boolean configurable;   
};
static CamFunctionEnable_st_t CamAutoFocus_st =
{
    FALSE,                          // Boolean default_setting;
    FALSE,                          // Boolean cur_setting;    
    FALSE                           // Boolean configurable;   
};

// --------Sensor Image Quality Configuration
static CamSensorImageConfig_st_t ImageSettingsConfig_st =
{
    &SceneMode_st,          // CamSceneMode_st_t        *sensor_scene;              Scene Mode Setting & Capabilities                               
    &DigitalEffect_st,      // CamDigitalEffect_st_t    *sensor_digitaleffect;      Digital Effects Setting & Capabilities                          
    &Flicker_st,            // CamFlicker_st_t          *sensor_flicker;            Flicker Control Setting & Capabilities                          
    &WBmode_st,             // CamWBMode_st_t           *sensor_wb;                 White Balance Setting & Capabilities                            
    &Exposure_st,           // CamExposure_st_t         *sensor_exposure;           Exposure Setting & Capabilities                                 
    &Metering_st,           // CamMeteringType_st_t     *sensor_metering;           Metering Setting & Capabilities                                 
    &Sensitivity_st,        // CamSensitivity_st_t      *sensor_sensitivity;        Sensitivity Setting & Capabilities                              
    &CamWideDynRange_st,    // CamFunctionEnable_st_t   *sensor_wdr;                Wide Dynamic Range Setting & Capabilities                       
                                                                                                                                                    
    &PrimaryFrameRate_st,   // CamFrameRate_st_t        *sensor_framerate;          Frame Rate Output Settings & Capabilities                       
    &PrimaryFlashState_st,  // CamFlashLedState_st_t    *sensor_flashstate;         Flash Setting & Capabilities                                    
    &RotateMode_st,         // CamRotateMode_st_t       *sensor_rotatemode;         Rotation Setting & Capabilities                                 
    &PrimaryMirrorMode_st,  // CamMirrorMode_st_t       *sensor_mirrormode;         Mirror Setting & Capabilities                                   
    &JpegQuality_st,        // CamJpegQuality_st_t      *sensor_jpegQuality;        Jpeg Quality Setting & Capabilities:  Values allowed 0 to 10    
    &CamFrameStab_st,       // CamFunctionEnable_st_t   *sensor_framestab;          Frame Stabilization Setting & Capabilities                      
    &CamAntiShake_st,       // CamFunctionEnable_st_t   *sensor_antishake;          Anti-Shake Setting & Capabilities                               
    &CamFaceDetection_st,   // CamFunctionEnable_st_t   *sensor_facedetect;         Face Detection Setting & Capabilities                               
    &CamAutoFocus_st,       // CamFunctionEnable_st_t   *sensor_autofocus;          Auto Focus Setting & Capabilities         
                                                                                                                                                    
    &Contrast_st,           // CamImageAppearance_st_t   *sensor_contrast;          default=0:  Values allowed  -100 to 100, zero means no change   
    &Brightness_st,         // CamImageAppearance_st_t   *sensor_brightness;        default=0:  Values allowed  0=All black  100=All white          
    &Saturation_st,         // CamImageAppearance_st_t   *sensor_saturation;        default=0:  Values allowed  -100 to 100, zero means no change   
    &Hue_st,                // CamImageAppearance_st_t   *sensor_hue;               default=0:  Values allowed  -100 to 100, zero means no change   
    &Gamma_st,              // CamImageAppearance_st_t   *sensor_gamma;             default=0:  Values allowed  -100 to 100, zero means no change   
    &Sharpness_st,          // CamImageAppearance_st_t   *sensor_sharpness;         default=0:  Values allowed  -100 to 100, zero means no change   
    &AntiShadingPower_st    // CamImageAppearance_st_t   *sensor_antishadingpower;  default=0:  Values allowed  -100 to 100, zero means no change   
};

//************************************************************************************
//************************************************************************************


// STV0986 Color Matrix I2C Register addresses
UInt16 stv0986_RegAddr_CM[4][9] = 
{
    // ColorEngine0.Sensor0
    { 0x3701, 0x3705, 0x3709,
      0x370D, 0x3711, 0x3715,
      0x3719, 0x371D, 0x3721 },
    // ColorEngine0.Sensor1
    { 0x3781, 0x3785, 0x3789,
      0x378D, 0x3791, 0x3795,
      0x3799, 0x379D, 0x37A1 },
    // ColorEngine1.Sensor0
    { 0x3901, 0x3905, 0x3909,
      0x390D, 0x3911, 0x3915,
      0x3919, 0x391D, 0x3921 },
    // ColorEngine1.Sensor1
    { 0x3981, 0x3985, 0x3989,
      0x398D, 0x3991, 0x3995,
      0x3999, 0x399D, 0x39A1 },
};
    
// Values taken from VS6750.pdf.Nov_2005_rev_03::personality file
// same values used in our drivers & example sripts
UInt16 vs6750_col_matrix_sRGB[9] = 
{                               //    Rin       Gin         Bin
    0x4029, 0xBE05, 0xB866,     //    2.160     -1.010      -0.150   R   = 1
    0xB9D7, 0x3F7B, 0xBC00,     //   -0.240      1.740      -0.500   G   = 1
    0xB785, 0xBD52, 0x3FE1      //   -0.110     -0.830       1.940   B   = 1
};

// Values taken from VB6850.pdf.June_2006_rev_01::personality file
UInt16 vb6850_col_matrix_sRGB_r01[9] = 
{                               //    Rin       Gin         Bin
    0x403B, 0xBE6C, 0xB28F,     //    2.230     -1.210      -0.020   R   = 1
    0xBB0A, 0x3FD7, 0xBC29,     //   -0.380      1.920      -0.540   G   = 1
    0xB63D, 0xBD7B, 0x3FE6      //   -0.070     -0.870       1.950   B   = 1
};

// Values taken from our 6850 drivers & scripts, probably personality file::rev_02 ?
UInt16 vb6850_col_matrix_sRGB_r02[9] = 
{                               //    Rin       Gin         Bin
    0x3FF6, 0xBE1A, 0x363D,     //    1.980     -1.050       0.070   R   = 1
    0xBA14, 0x3F2E, 0xBAA4,     //   -0.260      1.590      -0.330   G   = 1
    0xB48F, 0xBD1F, 0x3FA4      //   -0.040     -0.780       1.820   B   = 1
};

// Values taken from our 6850 drivers & scripts, Date: 05-Oct-2007 17:59:21 Generated for 850
UInt16 vb6850_col_matrix_sRGB_r03[9] = 
{                               //    Rin       Gin         Bin
    0x3F7B, 0xBAE1, 0xBB0A,     //    1.740     -0.360      -0.380   R   = 1
    0xBC8F, 0x3FC3, 0xB9D7,     //   -0.640      1.880      -0.240   G   = 1
    0xBB48, 0xBFC8, 0x414D      //   -0.410     -1.890       3.300   B   = 1
};

    #if 0
// Values taken from 555.zip::06_Idle.txt script
UInt16 vs6555_col_matrix_sRGB[9] = 
{                               //    Rin       Gin         Bin
//  0x3F4D, 0xBDD2, 0xB585,     //    1.650     -0.955      -0.055   R  != 1, values from ST script 
    0x3F4D, 0xBC61, 0xB585,     //    1.650     -0.595      -0.055   R   = 1, probable correct value
    0xB8E1, 0x3EF8, 0xBA71,     //   -0.180      1.485      -0.305   G   = 1
    0xB614, 0xBB7B, 0x3F00      //   -0.065     -0.435       1.500   B   = 1
};
    #else
// Values taken from Demo v3-52 555 Idle.txt script
UInt16 vs6555_col_matrix_sRGB[9] = 
{                               //    Rin       Gin         Bin
    0x404D, 0xBE61, 0xB785,     //    2.30      -1.19      -0.11   R   = 1
    0xBAE1, 0x3FF1, 0xBC71,     //   -0.36       1.97      -0.61   G   = 1
    0xB814, 0xBD7B, 0x4000      //   -0.13      -0.87       2.00   B   = 1
};
    #endif

UInt16 stv0986_col_matrix_BW[9] = 
{                               //    Rin       Gin     Bin
    0x3AAB, 0x3AAB, 0x3AAB,     //    0.333,  0.333,  0.333      R
    0x3AAB, 0x3AAB, 0x3AAB,     //    0.333,  0.333,  0.333      G
    0x3AAB, 0x3AAB, 0x3AAB      //    0.333,  0.333,  0.333      B
};


// --------STV0986 Specific Variables                                                   
static CamSensorSelect_t sCamSensor                     = CamSensorPrimary;
static CamSensor_t  sCamSensorSel                       = SensorVB6850;
static CamClkSel_t  sCamClkSpeed                        = CamDrv_13MHz;         //**** MUST MATCH SETTING IN CamSensorIntfCntrl_st_t  CamPowerOnSeq[]
#if 1
static UInt16       sCamItuClkDiv                       = 3;                    // Pixel Clock == 40Mhz
#else
static UInt16       sCamItuClkDiv                       = 2;                    // Pixel Clock == 80Mhz
#endif
static CamImageSize_t sViewFinderResolution             = CamImageSize_QVGA;
static CamDataFmt_t sViewFinderFormat                   = CamDataFmtYCbCr;
static CamImageSize_t sCaptureImageResolution           = CamImageSize_QVGA;
static CamDataFmt_t sCaptureImageFormat                 = CamDataFmtYCbCr;
                                                        
//---------Camera Sensor0 VS6555 VGA
static UInt16       sSensor0MaxDataRate         = 0x4BC0; // 120Mhz
//---------Camera Sensor1 VB6850 VGA
static UInt16       sSensor1MaxDataRate         = 0x5080; // 640Mhz

//--------Settings based on hardware platform, baseband and camera sensor
//---------Define Primary Sensor and Position
static CamSensor_t  sSensorPrimary      = SensorVB6850;
static CamSensor_t  sCamSensor1             = SensorVB6850;
//---------Define Secondary Sensor and Position (if available)
static CamSensor_t  sSensorSecondary    = SensorVS6555;
static CamSensor_t  sCamSensor0             = SensorVS6555;


//****************************************************************************
//                          Variable
//****************************************************************************

// ---- Private Variables ---------------------------------------------------
static const InitElement_t  sCamInitData[] =
{
    {CAM_INIT_DONE, CAM_INIT_DONE, FALSE}               //  no additional initialization for TransChip Camera
};

// I2C Read Data
static HAL_CAM_Result_en_t sCamI2cStatus = HAL_CAM_SUCCESS; 

// AutoFocus Control
HAL_CAM_Focus_Control_Config_st_t  sCamFocusControl_st =        ///< focus control configuration
{
    CamFocusControlAuto,        ///< focus control mode
    255,                        ///< number of focus control steps
    80,                         ///< the current step
};     

//****************************************************************************
//                          STV0986 Register Defines
//****************************************************************************
#define bSystemControl                                  0x5E80          // SystemControlParameters_bSystemControl
#define bSystemStatus                                   0x5F00          // SystemControlParameters_bSystemStatus
    #define STATE_UNINITIALIZED                         73              // 0x49
    #define STATE_BOOT                                  57              // 0x39
    #define STATE_SLEEP                                 17              // 0x11
    #define STATE_IDLE                                  18              // 0x12
    #define STATE_VIEWFINDER                            19              // 0x13
    #define STATE_STILLS                                20              // 0x14
    #define STATE_MOVIE                                 21              // 0x15
    #define STATE_STILL_N_THUMBNAIL                     22              // 0x16
    #define STATE_BUSY                                  24              // 0x18

#define bSystemErrorStatus                              0x5F02          // SystemStatusParameters_bSystemErrorStatus
#define uwDeviceId_MS                                   0x5E01          // SystemDeviceParameters_uwDeviceId
#define uwDeviceId_LS                                   0x5E02          // SystemDeviceParameters_uwDeviceId
#define bViewfinderStandardImageResolution              0x600C
#define bViewfinderImageFormat                          0x6016
#define bStillStandardImageResolution                   0x6000
#define bStillImageFormat                               0x600A
#define bInterleavingMode                               0x608E
    #define DISABLE_INTERLEAVE                          0x009A          // <154> interleaving is disabled
    #define ALTERNATE_FRAME_INTERLEAVE                  0x009B          // <155>  interleaving successively 1 Movie frame + 1 Viewfinder frame
    #define SINGLE_FRAME_INTERLEAVE                     0x009C          // <156>  interleaving within the same frame 1 Movie frame + 1 Viewfinder frame
    #define JPEG_INTERLEAVED_AT_END                     0x009D          // <157>  interleaving JPEG stills data and viewfinder data, where the viewfinder data is at the end

#define bImageReadMode                                  0x6098          //SystemImageCharacteristicsControlParameters_bImageReadMode
    #define IMAGE_NORMAL_MODE                           0x00B8          // <184> normal mode is transferred
    #define IMAGE_ROTATE_90                             0x00B9          // <185> Image is rotated anti-clockwise by 90 degrees
    #define IMAGE_ROTATE_180                            0x00BA          // <186> Image is rotated by 180 degrees
    #define IMAGE_ROTATE_270                            0x00BB          // <187> Image is rotated anti-clockwise by 270 degrees
    #define IMAGE_VERTICAL_FLIP                         0x00BC          // <188> Image is vertically flipped
    #define IMAGE_MIRRORING                             0x00BD          // <189> Image is horizontally flipped or mirrored
    #define IMAGE_YUV_PLANAR                            0x00BE          // <190> Image is YUV4:2:0 planar image

// Packet Format Size Control Paramters
#define bPacketLength_lo                                0x6094          // SystemImageCharacteristicsControlParameters_bPacketLength_lo
#define bPacketLength_hi                                0x6096          // SystemImageCharacteristicsControlParameters_bPacketLength_hi
#define bOifJpegPktSize_lo                              0x628e          // SystemConfigurationParameters_bOifJpegPktSize_lo
#define bOifJpegPktSize_hi                              0x6290          // SystemConfigurationParameters_bOifJpegPktSize_hi

// JPEGImageCharacteristicsControlParameters
#define bSqueezeSettings                                0x6100          // JPEGImageCharateristicsControlParameters_bSqueezeSettings
#define bTargetFileSize_lo                              0x6102          // JPEGImageCharateristicsControlParameters_bTargetFileSize_lo
#define bTargetFileSize_hi                              0x6104          // JPEGImageCharateristicsControlParameters_bTargetFileSize_hi
#define bImageQuality                                   0x6106          // JPEGImageCharateristicsControlParameters_bImageQuality
    #define STILL_LOW_QUALITY                           163             // <163> Set the still image quality to low
    #define STILL_MEDIUM_QUALITY                        164             // <164> Set the still image quality to medium
    #define STILL_HIGH_QUALITY                          165             // <165> Set the still image quality to high
#define bImageFormat                                    0x6108          // JPEGImageCharateristicsControlParameters_bImageFormat
#define bSelectVCDataOutEndianess                       0x610C          // JPEGImageCharateristicsControlParameters_bSelectVCDataOutEndianess
// JpegImageCharacteristicsStatusParameters
#define bAutoSqueeze                                    0x618C          // JPEGImageCharateristicsControlParameters_bAutoSqueeze
#define image_size_lo_0to7                              0x618E          // JPEGImageCharateristicsControlParameters_image_size_lo_0to7
#define image_size_lo_8to15                             0x6190          // JPEGImageCharateristicsControlParameters_image_size_lo_8to15
#define image_size_lo_16to23                            0x6192          // JPEGImageCharateristicsControlParameters_image_size_lo_16to23

#define bViewfinderCustomImageResolution_Xsize_lo       0x600E          // SystemOutputImageDimensionsPage_bViewfinderCustomImageResolution_Xsize_lo
#define bViewfinderCustomImageResolution_Xsize_hi       0x6010          // SystemOutputImageDimensionsPage_bViewfinderCustomImageResolution_Xsize_hi
#define bViewfinderCustomImageResolution_Ysize_lo       0x6012          // SystemOutputImageDimensionsPage_bViewfinderCustomImageResolution_Ysize_lo
#define bViewfinderCustomImageResolution_Ysize_hi       0x6014          // SystemOutputImageDimensionsPage_bViewfinderCustomImageResolution_Ysize_hi

#define bStillCustomImageResolution_Xsize_lo            0x6002          // SystemOutputImageDimensionsPage_bStillCustomImageResolution_Xsize_lo
#define bStillCustomImageResolution_Xsize_hi            0x6004          // SystemOutputImageDimensionsPage_bStillCustomImageResolution_Xsize_hi
#define bStillCustomImageResolution_Ysize_lo            0x6006          // SystemOutputImageDimensionsPage_bStillCustomImageResolution_Ysize_lo
#define bStillCustomImageResolution_Ysize_hi            0x6008          // SystemOutputImageDimensionsPage_bStillCustomImageResolution_Ysize_hi


// HostToSensorAccessControl
#define bRequest                                        0x0880          // HostToSensorAccessControl_bRequest
#define bCommandCoin                                    0x0882          // HostToSensorAccessControl_bCommandCoin
#define uwSensorIndex_MS                                0x0885          // HostToSensorAccessControl_bRequest
#define uwSensorIndex_LS                                0x0886          // HostToSensorAccessControl_bRequest
#define bActiveSensor                                   0x0888          // HostToSensorAccessControl_bActiveSensor

#define bStatusCoin                                     0x0900          // HostToSensorAccessStatus_bStatusCoin
#define bHostToSensorAccessErrorCount                   0x0902          // HostToSensorAccessStatus_bHostToSensorAccessErrorCount

#define uwDataLow_MS                                    0x0981          // HostToSensorAccessData_uwDataLow
#define uwDataLow_LS                                    0x0982          // HostToSensorAccessData_uwDataLow
#define uwDataHigh_MS                                   0x0985          // HostToSensorAccessData_uwDataHigh
#define uwDataHigh_LS                                   0x0986          // HostToSensorAccessData_uwDataHigh

#define GP_Channel_config                               0xA108
#define GP_direction                                    0xA151
#define GP_output                                       0xA14D

#define STV_GPIO_2                                      0x04
#define STV_GPIO_3                                      0x08
#define STV_GPIO_4                                      0x10

#define STV_DEVICE_ID                                   0x03DA
                                                    
#define STATE_UNINITIALIZED                             73  // 0x49
#define STATE_BOOT                                      57  // 0x39
#define STATE_SLEEP                                     17  // 0x11
#define STATE_IDLE                                      18  // 0x12
#define STATE_VIEWFINDER                                19  // 0x13
#define STATE_STILLS                                    20  // 0x14
#define STATE_MOVIE                                     21  // 0x15
#define STATE_STILL_N_THUMBNAIL                         22  // 0x16
#define STATE_BUSY                                      24  // 0x18

// FocusControls bFocusMode
#define FC_TLI_MODE_MANUAL_FOCUS                        0       // - mode for manual focus control.
#define FC_TLI_MODE_AF_CONTINUOUS_FOCUS                 1       // - continuous focus mode.
#define FC_TLI_MODE_AF_SINGLE_FOCUS_FS                  2       // - still mode with full search.
#define FC_TLI_MODE_AF_SINGLE_FOCUS_HCS                 3       // - still mode with hill-climb (quick) search.
#define FC_TLI_MODE_AF_CUSTOM                           4       // - Full customizable mode. Also in the other

// FocusControls bAFCommand 
#define AF_TLI_CMD_NULL                                 0       // - default dummy command, used between issuing the same command twice.
#define AF_TLI_CMD_RELEASED_BUTTON                      1       // - The host notifies the AF manager of the released button. 
                                                                //      The AF manager freezes the focus position in stills mode, or
                                                                //      continues running the focus (continuous) in movie mode.
#define AF_TLI_CMD_HALF_BUTTON                          2       // - The host notifies the AF manager of the
                                                                //      half button press. The AF manager runs the hill-climb (quick) search. When it
                                                                //      finds the peak the algorithm goes into a wait state.
#define AF_TLI_CMD_TAKE_SNAPSHOT                        3       // - The host notifies the AF manager to
                                                                //      freeze all the state machine to take the photo. Freezes the algorithm in any mode
                                                                //      and then switches to TLI_CMD_RELEASED_BUTTON
#define AF_TLI_CMD_REFOCUS                              4       // - The AF manager does a fine search on the
                                                                //      current focus position and then it waits again.
#define AF_TLI_CMD_PEAK_SWITCH                          5       // - The AF manager searches for another
                                                                //      focus peak and then waits.

#define LA_CMD_NULL                                     0       // - default dummy command, used between issuing the same command twice.
#define LA_CMD_MOVE_STEP_TO_INFINITY                    1       // - command the lens driver to move toward infinity. 
#define LA_CMD_MOVE_STEP_TO_MACRO                       2       // - command the lens driver to move toward macro. 
#define LA_CMD_GOTO_INFINITY                            3       // - command the lens driver to go to the infinity position.
#define LA_CMD_GOTO_MACRO                               4       // - command the lens driver to go to the macro position.
#define LA_CMD_GOTO_RECOVERY                            5       // - command the lens driver to go to the recovery position specified in the current range value.
#define LA_CMD_GOTO_HOME                                6       // - command the lens driver to go to the home position.
#define LA_CMD_GOTO_TARGET_POSITION                     7       // - command the lens driver to go to the target position.
#define LA_CMD_PARK                                     8       // - command the lens driver to park the lens.
//#define <9> RESERVED.
#define LA_CMD_STOP_ACTUATOR                            10      // - force a stop command to the actuator even if it is moving.
#define LA_CMD_INIT                                     11      // - driver level initialisation command.

#define bAFStatsError                                   0x5100  // AFStatsStatus.bAFStatsError
    #define AF_ERROR_OK                                     0   // No AF errors
#define fLensIsMoving                                   0x5384
#define bFocusMode                                      0x5404  // FocusControls.bFocusMode
#define bAFCommand                                      0x5406
#define bFocusModeStatus                                0x5480  // FocusStatus.bModeStatus
#define fIsStable                                       0x548a  // FocusStatus.fIsStable

#define bWeight_0                                       0x5880  // AutoFocusWeightControls 0
#define bWeight_1                                       0x5882  // AutoFocusWeightControls 1
#define bWeight_2                                       0x5884  // AutoFocusWeightControls 2
#define bWeight_3                                       0x5886  // AutoFocusWeightControls 3
#define bWeight_4                                       0x5888  // AutoFocusWeightControls 4
#define bWeight_5                                       0x588A  // AutoFocusWeightControls 5
#define bWeight_6                                       0x588C  // AutoFocusWeightControls 6
#define bWeight_7                                       0x588E  // AutoFocusWeightControls 7
#define bWeight_8                                       0x5890  // AutoFocusWeightControls 8
#define bWeight_9                                       0x5892  // AutoFocusWeightControls 9

// Manual Focus
#define bLensCommand                                    0x5408  // FocusControls.bLensCommand
#define bRangeFocus                                     0x5402  // FocusControls.bRange
#define bManualStep_Size                                0x540a  // FocusControls.bManualStep_Size
#define wLensPosition                                   0x5381  // FLADriverStatus.wLensPosition

#define bColourEngine0_bMaxGain                         0x3A82
#define bColourEngine1_bMaxGain                         0x3B02

#define bColorEngine0_bContrast                         0x3C82
#define bColorEngine0_bColourSaturation                 0x3C84

#define bColorEngine1_bContrast                         0x3F02
#define bColorEngine1_bColourSaturation                 0x3F04

#define bPipe0Control_fSfxSolariseEnabled               0x0682
#define bPipe0Control_fSfxNegativeEnabled               0x0684
#define bPipe0Control_bGreyBack                         0x0688
#define bPipe0Control_bFlipper                          0x068A

#define bPipe1Control_fSfxSolariseEnabled               0x0702
#define bPipe1Control_fSfxNegativeEnabled               0x0704
#define bPipe1Control_bGreyBack                         0x0708
#define bPipe1Control_bFlipper                          0x070a

// ZoomMgrCtrl bZoomCmd
#define NO_ZOOM_COMMAND                                 0       
#define ZOOM_IN                                         1       
#define ZOOM_OUT                                        2       
#define ZOOM_RESET                                      3       
#define ZOOM_SET                                        4       

#define bHostTestCoin                                   0x4e80          // 
    #define TestCoin_Heads                              1
    #define TestCoin_Tails                              2
#define bWoiControlCoin                                 0x91ea          // 
#define bZoomCmd                                        0x4e82          // 
#define fAutoZoom                                       0x4e86          // 
#define bMagFactor                                      0x4e8a          // 
    #define MagFactor_1_15                                      0x7D
    #define MagFactor_1_33                                      0x7D
    #define MagFactor_1_6                                       0xFD
    #define MagFactor_2_0                                       0xFD
    #define MagFactor_2_66                                      0xFD
    #define MagFactor_4_0                                       0xFD

#define fSetAlternateInitWOI                            0x4e90          // 

#define bDeviceTestCoin                                 0x4f02          // 
#define bWoiStatusCoin                                  0x91ea          // 
#define bCommandStatus                                  0x4f08          // 
#define bZoomOpStatus                                   0x4f0a          // 
    #define NoZoomExhausted                             0           // NoZoomExhausted - zoom working OK
    #define FullyZoomedOut                              1 
    #define FullyZoomedIn                               2 
    #define Pipe0ZoomExhausted                          3 
    #define Pipe1ZoomExhausted                          4 
    #define Pipe0CrossScaling                           5 
    #define ChgOverRequired                             6           // - sensor change-over required for next zoom step,but forbidden by host.
    #define TimingLimited                               7           // - DMA timing limited
    #define SetOutOfRange                               8 
    #define LineLengthLimited                           9 
#define fFOVX                                           0x4f0c          // 
#define fFOVY                                           0x4f14          // 

// CE0 GAMMA CONTROL
#define bColorEngine0_fGammaEnabled                     0x4180
#define bColorEngine0_bMode                             0x4182
#define bColorEngine0_SharpRed                          0x4184
#define bColorEngine0_SharpGreen                        0x4186
#define bColorEngine0_SharpBlue                         0x4188
#define bColorEngine0_SoftRed                           0x418a
#define bColorEngine0_SoftGreen                         0x418c
#define bColorEngine0_SoftBlue                          0x418e

// CE1 GAMMA CONTROL
#define bColorEngine1_fGammaEnabled                     0x4200
#define bColorEngine1_bMode                             0x4202
#define bColorEngine1_SharpRed                          0x4204
#define bColorEngine1_SharpGreen                        0x4206
#define bColorEngine1_SharpBlue                         0x4208
#define bColorEngine1_SoftRed                           0x420a
#define bColorEngine1_SoftGreen                         0x420c
#define bColorEngine1_SoftBlue                          0x420e

//--- Sensor Image_Orientation (SMIA Defined Registers)-----------------------        
#define bSmiaImageOrientationReg                        0x0101
    #define bSmiaMirrorNormal                           0x0000
    #define bSmiaHorizontalMirrorEnable                 0x0001
    #define bSmiaVerticalMirrorEnable                   0x0002

//--- ST0986 Flash Manager ---------------------------------------------------        
#define bFM_FlashMode                                   0x1E80
    #define bFM_FlashMode_AUTO                          0
    #define bFM_FlashMode_ALWAYS                        1
    #define bFM_FlashMode_NEVER                         2
#define bFM_FlashType                                   0x1E82
    #define bFM_FlashType_WHITE_LED                     0
    #define bFM_FlashType_HIGH_PWR_LED                  1
    #define bFM_FlashType_FLASHGUN                      2    
#define bFM_OrPreAndMain                                0x1E84
    #define bFM_OrPreAndMain_TRUE                       1    // default
// timing reference calc mode
#define bFM_TimingRefCalcMode                           0x1E86   
    #define bFM_TimingRefCalcMode_AUTO                  0    // default
    #define bFM_TimingRefCalcMode_MAN                   1
// in lines, signed, DEF 0    
#define wFM_IntegrationStartPos                         0x1E89   
// DEF = 0, use default of wFM_IntegrationStartPos
#define bFM_OverrideIntegrationStartPosition            0x1E8C   
// DEF = 0
#define bFM_NumberOfPreFlashes                          0x1E92   
// VPIP_FLOAT, DEF = 0.0 = 10us ?
#define fFM_MainFlashPulseWidth_us                      0x1E95   
// VPIP_FLOAT, DEF = 0.0 = 10us ? VPIP_FLOAT
#define fFM_PreFlashPulseWidth_us                       0x1E99
// VPIP_FLOAT, DEF = 0.0 = FrameLen_uS - PrePulseWidth_uS
#define fFM_TimeBetweenTwoPreFlashes_us                 0x1E9D   
// VPIP_FLOAT, DEF = 0.0 = FrameLen_uS - PrePulseWidth_uS
#define fFM_TimeBetweenPreAndMain_us                    0x1EA1   
// VPIP_FLOAT, DEF = 0.0  Frame Number in which first reference point 
// (start of flash)is to be triggered. Valid Only if bFM_TimingRefCalcMode_MAN
#define bFM_MainFlashStartFrame                         0x1EA4   
// DEF = 0  Line  Number in which first reference point (start of flash) 
// is to be triggered. Valid Only if bFM_TimingRefCalcMode_MAN                                                    
#define uwFM_MainFlashStartLine                         0x1EA7   
// DEF = 0  Pixel  Number in which first reference point (start of flash) 
// is to be triggered. Valid Only if bFM_TimingRefCalcMode_MAN                                                    
#define uwFM_MainFlashStartPixel                        0x1EAB   
// DEF = 0  Frame Number in which PRE FLASH is to be trigerred
#define bFM_PreFlashStartFrame                          0x1EAE   
// DEF = 0  Line  Number in which PREFLASH is to be trigerred                                                         
#define uwFM_PreFlashStartLine                          0x1EB1
// DEF = 0  Pixel Number in which PREFLASH is to be trigerred
#define uwFM_PreFlashStartPixel                         0x1EB5   
// TOTAL FRAMES NEEDED IN MANUAL MODE
#define bFM_ManModeTotalFrameCount                      0x1EB8   
//--- ST0986 Flash Manager ---------------------------------------------------        

#define fInhibitExposurePresetModeForFlash              0x2042   
#define fInhibitWhiteBalancePresetModeForFlash          0x2516   

//--- TORCH Mode GPIO Control ------------------------------------------------    
#define STV0986_GPIO_TYPE                               0xA108
    #define STV0986_GPIO_TYPE_GENERIC                   1
    #define STV0986_GPIO_TYPE_DEDICATED                 0
#define STV0986_GPIO_DIRECTION                          0xA151
    #define STV0986_GPIO_DIRECTION_IN                   0
    #define STV0986_GPIO_DIRECTION_OUT                  1
#define STV0986_GPIO_OUTPUT                             0xA14D
    #define STV0986_GPIO_OUTPUT_ON                      1
    #define STV0986_GPIO_OUTPUT_OFF                     0
    #define STV0986_GPIO_OUTPUT_TORCH_MODE              STV0986_GPIO_OUTPUT_OFF
    #define STV0986_GPIO_OUTPUT_FLASH_MODE              STV0986_GPIO_OUTPUT_ON

// STV GPIO Flash Pin Assignments
#define FLASH_PIN_TRIGGER_MAIN                          2  // GPIO 2
#define FLASH_PIN_TRIGGER_COMBINED                      3  // GPIO 3
#define FLASH_PIN_TRIGGER_PRE                           3  // GPIO 4
#define FLASH_PIN_TORCH_FLASH                           4  // GPIO 4
    
    
#define FLASH_PRE_COUNT                                 3
#if 0
#define FLASH_PRE_LEN_uS                                (float)250.0
// it takes ~ 250us to reach max LED current in FLASH mode
// LED - can take up to 100mA for 30msec with duty cycle of 1/10
#define FLASH_MAIN_LEN_uS                               (float)10000.0
#endif
//--- TORCH Mode GPIO Control ------------------------------------------------    

// PIPE0 Number Of Frames To Stream, DEF=0=continuously
#define bSystemFrameCount                               0x609E


//****************************************************************************
//                          Local Function Prototypes
//****************************************************************************
static HAL_CAM_Result_en_t     Init_Stv0986(CamSensorSelect_t sensor);
static HAL_CAM_Result_en_t     stv0986_init1(CamSensor_t sensor_sel);
static HAL_CAM_Result_en_t     stv0986_init2(CamSensor_t sensor_sel);
static HAL_CAM_Result_en_t     stv0986_init3(CamSensor_t sensor_sel);
                    
static void         StandardPatch_986ITUMode(void);
static HAL_CAM_Result_en_t     SensorPatch_VB6850(void);

static HAL_CAM_Result_en_t     stv_write(unsigned int sub_addr, unsigned char data);
//static HAL_CAM_Result_en_t     stv_write_2(unsigned int sub_addr, UInt32 data);
static UInt8        stv_read(unsigned int sub_addr);
static HAL_CAM_Result_en_t     stv_WaitValue(UInt32 timeout, UInt16 sub_addr, UInt8 value);

static void         StvWriteAutoIncrement ( UInt16 sub_addr, UInt8 size, ... );

static HAL_CAM_Result_en_t     SensorSetPreviewMode(CamImageSize_t image_resolution, CamDataFmt_t image_format);
static HAL_CAM_Result_en_t     SensorSetCaptureMode(CamImageSize_t image_resolution, CamDataFmt_t image_format);
static HAL_CAM_Result_en_t     SensorSetInterLeaveMode(CamCaptureMode_t mode, CamDataFmt_t image_format);
static HAL_CAM_Result_en_t     SensorSetSleepMode(void);
static HAL_CAM_Result_en_t     SensorSetPowerDown(Boolean level);
static HAL_CAM_Result_en_t     SensorSetFrameRate(CamRates_t fps, CamSensorSelect_t sensor);

//static void       stv_SensorSetupImageOrientation(void);
#ifdef CAMDBG_STATUS
static void         stv_Read_Status(void);
static void         stv_Read_Sensor_Status(void);
#endif

static HAL_CAM_Result_en_t     SensorRead(UInt16 byte_count, UInt16 addr, UInt8 *sensor_data);
static HAL_CAM_Result_en_t     SensorWrite(UInt16 byte_count, UInt16 addr, UInt8 *sensor_data);

// static HAL_CAM_Result_en_t      ZoomCommandWrite(void);
static void                     stv_SetPacketSize(UInt16 packet_size);

static void                     stv0986_SetColorMatrix( Stv0986_RegAddr_ColMat_t ce_S0S1, UInt16 * pColMatrix );
static void                     stv0986_SetMonochrome( Boolean on );
static UInt32                   stv_Read_JpegSize(void);
#if 0
static HAL_CAM_Result_en_t      stv_SensorTestMode(void);
static HAL_CAM_Result_en_t      stv_SensorStreamMode(void);
#endif

// Focus Control
static void         AF_PositionSensorSetup(void);
static HAL_CAM_Result_en_t     AF_QuickSearch(void);
static HAL_CAM_Result_en_t     SetAutoFocusCmd( UInt8 af_command, UInt8 focus_mode, Boolean lens_moving );
static HAL_CAM_Result_en_t     SetManualFocusCmd( UInt8 lens_command, UInt8 focus_mode, Boolean lens_moving );

// Flash Control
//static HAL_CAM_Result_en_t     stv0986_FlashManagerOn( UInt8 preCount, float preLen_us, float mainLen_us );
static HAL_CAM_Result_en_t     stv0986_FlashManagerOff( void );
static HAL_CAM_Result_en_t     stv0986_GpioSetTorchModeTorchOn( void );
static HAL_CAM_Result_en_t     stv0986_GpioSetFlashModeTorchOff( void );
//UInt16              stv0986_Float2VpipFloat( float float_in );


/**
*  This function will perform specific action from defined list, copy of parameters passed thru parm structure.
*
*/
HAL_CAM_Result_en_t CAMDRV_ActionCtrl(
        HAL_CAM_Action_en_t action,                 
        void*               data,                  
        void*               callback               
        )
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS; 

    pr_debug("HAL_CAMDRV_Ctrl():  action=%d \r\n", action);

    switch (action) 
    {
// Operation Control Settings
        case ACTION_CAM_GetJpegMaxSize:                         ///< Get Jpeg Max size (bytes), use HAL_CAM_Action_param_st_t to get
            result = CAMDRV_GetJpegMaxSize(((HAL_CAM_Action_param_st_t *)data)->param, ((HAL_CAM_Action_param_st_t *)data)->sensor);
            break;
        case ACTION_CAM_SetFrameStabEnable:                     ///< Set Frame Stabilization enable TRUE/FALSE, use (Boolean) HAL_CAM_Action_param_st_t to set
            result = CAMDRV_SetFrameStabEnable(	((HAL_CAM_Action_param_st_t *)data)->param, ((HAL_CAM_Action_param_st_t *)data)->sensor );
            break;
        case ACTION_CAM_SetAntiShakeEnable:                     ///< Set Anti-Shake enable TRUE/FALSE, use (Boolean) HAL_CAM_Action_param_st_t to set
            result = CAMDRV_SetAntiShakeEnable( ((HAL_CAM_Action_param_st_t *)data)->param, ((HAL_CAM_Action_param_st_t *)data)->sensor );
            break;
        case ACTION_CAM_SetAutoFocusEnable:                     ///< Set Auto Focus enable TRUE/FALSE, use (Boolean) HAL_CAM_Action_param_st_t to set
            result = CAMDRV_SetAutoFocusEnable( ((HAL_CAM_Action_param_st_t *)data)->param, ((HAL_CAM_Action_param_st_t *)data)->sensor );
            break;
        case ACTION_CAM_SetFlashMode:                           ///< Set required flash mode, use (FlashLedState_t) HAL_CAM_Action_param_st_t to set
            result = CAMDRV_SetFlashMode( (FlashLedState_t)(((HAL_CAM_Action_param_st_t *)data)->param), 
                        ((HAL_CAM_Action_param_st_t *)data)->sensor );
            break;
        case ACTION_CAM_GetImageSize:                   ///< Get closest sensor resolution from passed pixel width and height, use HAL_CAM_Action_image_size_st_t to get CamImageSize_t
            result  = CAMDRV_GetResolutionAvailable(((HAL_CAM_Action_image_size_st_t *)data)->width, 
                                                    ((HAL_CAM_Action_image_size_st_t *)data)->height,
                                                    ((HAL_CAM_Action_image_size_st_t *)data)->mode,
                                                    ((HAL_CAM_Action_image_size_st_t *)data)->sensor,
                                                    ((HAL_CAM_Action_image_size_st_t *)data)->sensor_size);
            break;
        case ACTION_CAM_GetSensorResolution:            ///< Get matching image size from passed pixel width and height, use HAL_CAM_Action_image_size_st_t to get CamImageSize_t
            result  = CAMDRV_GetResolution(((HAL_CAM_Action_resolution_st_t *)data)->size, 
                                                    ((HAL_CAM_Action_resolution_st_t *)data)->mode,
                                                    ((HAL_CAM_Action_resolution_st_t *)data)->sensor,
                                                    ((HAL_CAM_Action_resolution_st_t *)data)->sensor_size);
            break;
        case ACTION_CAM_SetFrameRate:                           ///< Set required picture frame, use (CamRates_t) HAL_CAM_Action_param_st_t to set
            result = CAMDRV_SetFrameRate( (CamRates_t)(((HAL_CAM_Action_param_st_t *)data)->param), ((HAL_CAM_Action_param_st_t *)data)->sensor );
            break;
// Face Detection
        case ACTION_CAM_SetFaceDetectEnable:                    ///< Set Face Detection enable TRUE/FALSE, use (Boolean) HAL_CAM_Action_param_st_t to set
            result = CAMDRV_SetFaceDetectEnable( ((HAL_CAM_Action_param_st_t *)data)->param, ((HAL_CAM_Action_param_st_t *)data)->sensor );
            break;
        case ACTION_CAM_GetFaceDetectRect:                      ///< Get Face Detection Rectangle use HAL_CAM_Action_face_detect_st_t to get
            result = CAMDRV_GetFaceDetectRect(((HAL_CAM_Action_param_st_t *)data)->param, ((HAL_CAM_Action_param_st_t *)data)->sensor );
            break;
// Exposure Setting
        case ACTION_CAM_ExposureSetting:                    ///< Set Exposure, use HAL_CAM_Action_exposure_setting_st_t to set
            result = CAMDRV_SetExpSetting( ((HAL_CAM_Action_exposure_setting_st_t *)data)->exp_setting, 
                        ((HAL_CAM_Action_exposure_setting_st_t *)data)->sensor );
            break;
// Focus Control
        case ACTION_CAM_GetFocusStatus:                    ///< Get Focus Status, use HAL_CAM_Action_focus_status_st_t to get
            result = CAMDRV_GetFocusStatus( ((HAL_CAM_Action_focus_status_st_t *)data)->focus_status_ptr, 
                        ((HAL_CAM_Action_focus_status_st_t *)data)->focus_region_ptr, 
                        ((HAL_CAM_Action_focus_status_st_t *)data)->sensor );
            break;
        case ACTION_CAM_SetFocusRegion:                    ///< Set Focus Region, use HAL_CAM_Action_focus_region_st_t to set
            result = CAMDRV_SetFocusRegion( ((HAL_CAM_Action_focus_region_st_t *)data)->focus_region_ptr, 
                        ((HAL_CAM_Action_focus_region_st_t *)data)->sensor );
            break;
        case ACTION_CAM_GetFocusControlConfig:              ///< Get Focus Control Configuration, use HAL_CAM_Action_focus_control_st_t to get
            result = CAMDRV_GetFocusControlConfig( ((HAL_CAM_Action_focus_control_st_t *)data)->focus_control_ptr, 
                        ((HAL_CAM_Action_focus_control_st_t *)data)->sensor );
            break;
        case ACTION_CAM_SetFocusControlConfig:              ///< Set Focus Control Configuration, use HAL_CAM_Action_focus_control_st_t to set
            result = CAMDRV_SetFocusControlConfig( ((HAL_CAM_Action_focus_control_st_t *)data)->focus_control_ptr, 
                        ((HAL_CAM_Action_focus_control_st_t *)data)->sensor );
// Image Appearance Settings  These Actions are NOT for average user's use.
        case    ACTION_CAM_SetContrast:                         ///< Set required contrast setting, use (UInt8) HAL_CAM_Action_param_st_t to set
            result = CAMDRV_SetContrast( (Int8)(((HAL_CAM_Action_param_st_t *)data)->param), ((HAL_CAM_Action_param_st_t *)data)->sensor );
            break;
        case    ACTION_CAM_SetBrightness:                       ///< Set required brightness setting, use (UInt8) HAL_CAM_Action_param_st_t to set
            result = CAMDRV_SetBrightness( (Int8)(((HAL_CAM_Action_param_st_t *)data)->param), ((HAL_CAM_Action_param_st_t *)data)->sensor );
            break;
        case    ACTION_CAM_SetSaturation:                       ///< Set required saturation setting, use (UInt8) HAL_CAM_Action_param_st_t to set
            result = CAMDRV_SetSaturation( (Int8)(((HAL_CAM_Action_param_st_t *)data)->param), ((HAL_CAM_Action_param_st_t *)data)->sensor );
            break;
        case    ACTION_CAM_SetHue:                                  ///< Set required hue setting, use (UInt8) HAL_CAM_Action_param_st_t to set
            result = CAMDRV_SetHue( (Int8)(((HAL_CAM_Action_param_st_t *)data)->param), ((HAL_CAM_Action_param_st_t *)data)->sensor );
            break;
        case    ACTION_CAM_SetGamma:                                ///< Set required gamma setting, use (UInt8) HAL_CAM_Action_param_st_t to set
            result = CAMDRV_SetGamma( (Int8)(((HAL_CAM_Action_param_st_t *)data)->param), ((HAL_CAM_Action_param_st_t *)data)->sensor );
            break;
        case    ACTION_CAM_SetSharpness:                        ///< Set required sharpness setting, use (UInt8) HAL_CAM_Action_param_st_t to set
            result = CAMDRV_SetSharpness( (Int8)(((HAL_CAM_Action_param_st_t *)data)->param), ((HAL_CAM_Action_param_st_t *)data)->sensor );
            break;
        case    ACTION_CAM_SetAntiShadingPower:         ///< Set required anti shading power setting, use (UInt8) HAL_CAM_Action_param_st_t to set
            result = CAMDRV_SetAntiShadingPower( (Int8)(((HAL_CAM_Action_param_st_t *)data)->param), ((HAL_CAM_Action_param_st_t *)data)->sensor );
            break;
// Image Quality Settings   These Actions are NOT for average user's use.
        case    ACTION_CAM_SetImageQuality:                 ///< Set the image quality level from 0 to 10
            result = CAMDRV_SetImageQuality( (UInt8)(((HAL_CAM_Action_param_st_t *)data)->param), ((HAL_CAM_Action_param_st_t *)data)->sensor );
            break;
        case    ACTION_CAM_SetSceneMode:                        ///< Set required Scene Mode setting, use (CamSceneMode_t) HAL_CAM_Action_param_st_t to set
            result = CAMDRV_SetSceneMode( (CamSceneMode_t)(((HAL_CAM_Action_param_st_t *)data)->param), ((HAL_CAM_Action_param_st_t *)data)->sensor );
            break;
        case  ACTION_CAM_SetDigitalEffect:              ///< Set required Digital Effect setting, use (CamDigEffect_t) HAL_CAM_Action_param_st_t to set
            result = CAMDRV_SetDigitalEffect( (CamDigEffect_t)(((HAL_CAM_Action_param_st_t *)data)->param), ((HAL_CAM_Action_param_st_t *)data)->sensor );
            break;
        case    ACTION_CAM_SetFlickerControl:               ///< Set required Flicker Control setting, use (CamFlicker_t) HAL_CAM_Action_param_st_t to set
            result = CAMDRV_SetFlickerControl( (CamFlicker_t)(((HAL_CAM_Action_param_st_t *)data)->param), ((HAL_CAM_Action_param_st_t *)data)->sensor );
            break;
        case    ACTION_CAM_SetWBMode:                               ///< Set required White Balance setting, use (CamWB_WBMode_t) HAL_CAM_Action_param_st_t to set
            result = CAMDRV_SetWBMode( (CamWB_WBMode_t)(((HAL_CAM_Action_param_st_t *)data)->param), ((HAL_CAM_Action_param_st_t *)data)->sensor );
            break;
        case    ACTION_CAM_SetAutoExposure:                 ///< Set required Auto Exposure setting On/Off, use (CamExposure_t) HAL_CAM_Action_param_st_t to set
            result = CAMDRV_SetAutoExposure( (CamExposure_t)(((HAL_CAM_Action_param_st_t *)data)->param), ((HAL_CAM_Action_param_st_t *)data)->sensor );
            break;
        case    ACTION_CAM_SetMeteringType:                 ///< Set required metering type, use (CamMeteringType_t) HAL_CAM_Action_param_st_t to set
            result = CAMDRV_SetMeteringType( (CamMeteringType_t)(((HAL_CAM_Action_param_st_t *)data)->param), ((HAL_CAM_Action_param_st_t *)data)->sensor );
            break;
        case    ACTION_CAM_SetSensitivity:                  ///<  Set required sensitiviy setting, use (CamSensitivity_t) HAL_CAM_Action_param_st_t to set
            result = CAMDRV_SetSensitivity( (CamSensitivity_t)(((HAL_CAM_Action_param_st_t *)data)->param), ((HAL_CAM_Action_param_st_t *)data)->sensor );
            break;
        case ACTION_CAM_SetWideDynRangeEnable:              ///< Set Wide Dynamic Range enable TRUE/FALSE, use (Boolean) HAL_CAM_Action_param_st_t to set
            result = CAMDRV_SetWideDynRangeEnable(	((HAL_CAM_Action_param_st_t *)data)->param, ((HAL_CAM_Action_param_st_t *)data)->sensor );
            break;
// Image Orientation Settings
        case    ACTION_CAM_SetRotationMode:                 ///< Set required rotation mode, use (CamRotate_t) HAL_CAM_Action_param_st_t to set
            result = CAMDRV_SetRotationMode( (CamRotate_t)(((HAL_CAM_Action_param_st_t *)data)->param), ((HAL_CAM_Action_param_st_t *)data)->sensor );
            break;
        case    ACTION_CAM_GetRotationMode:                 ///< Get required rotation mode, use (CamRotate_t) HAL_CAM_Action_param_st_t to set
            (((HAL_CAM_Action_param_st_t *)data)->param) = CAMDRV_GetRotationMode( ((HAL_CAM_Action_param_st_t *)data)->sensor );
            break;
        case    ACTION_CAM_SetMirrorMode:                   ///< Set required mirror mode, use (CamMirror_t) HAL_CAM_Action_param_st_t to set
            result = CAMDRV_SetMirrorMode( (CamMirror_t)(((HAL_CAM_Action_param_st_t *)data)->param), ((HAL_CAM_Action_param_st_t *)data)->sensor );
            break;
        case    ACTION_CAM_GetMirrorMode:                   ///< Get required mirror mode, use (CamMirror_t) HAL_CAM_Action_param_st_t to set
            (((HAL_CAM_Action_param_st_t *)data)->param) = CAMDRV_GetMirrorMode( ((HAL_CAM_Action_param_st_t *)data)->sensor );
            break;
// Add customized actions:
        case ACTION_CAM_GetLuminance:                         ///< Get Luminance, use HAL_CAM_Action_param_st_t to get
            result = CAMDRV_GetLuminance(((HAL_CAM_Action_param_st_t *)data)->param, ((HAL_CAM_Action_param_st_t *)data)->sensor);
            break;
        case ACTION_CAM_LockAutoColor:                      ///< LockAuotColor multishot interval (0: AE, 1: AWB, 2: ALL)
            result = CAMDRV_LockAutoColor(((HAL_CAM_Action_param_st_t *)data)->param);
            break;
        case ACTION_CAM_UnlockAutoColor:                      ///< UnLockAuotColor multishot interval (0: AE, 1: AWB, 2: ALL)
            result = CAMDRV_UnlockAutoColor(((HAL_CAM_Action_param_st_t *)data)->param);
            break;
// Unsupported Actions in Camera Device Driver Return:  HAL_CAM_ERROR_ACTION_NOT_SUPPORTED
        default:
            pr_debug("HAL_CAMDRV_Ctrl(): Invalid Action \r\n");
            result = HAL_CAM_ERROR_ACTION_NOT_SUPPORTED;
            break;   
    }
    return result;
}                               

//******************************************************************************
//
// Function Name:   CAMDRV_InitParm
//
// Description:     initialize camera parameters
//
// Notes:
//
//******************************************************************************
HAL_CAM_Result_en_t CAMDRV_InitParm(CamSensorSelect_t sensor)
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;

    return result;
}

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
        HAL_CAM_ResolutionSize_st_t *sensor_size )
{
    HAL_CAM_Result_en_t result = HAL_CAM_ERROR_ACTION_NOT_SUPPORTED;
    UInt16 index, res_size;
    CamResolutionConfigure_t *res_ptr;
    CamResolutionConfigure_t *best_match_ptr;

// Resolution default settings
    sensor_size->size = CamImageSize_INVALID;
    sensor_size->resX = 0;
    sensor_size->resY = 0;

// Init Resolution table pointer    
    if ( (sensor == CamSensorPrimary) && (sSensorPrimary == SensorVB6850) )
    {
        res_size = sizeof(sSensorResInfo_VB6850_st)/sizeof(CamResolutionConfigure_t);
        res_ptr = &sSensorResInfo_VB6850_st[0];
    }
    else if ( (sensor == CamSensorSecondary) && (sSensorSecondary == SensorVS6555) )
    {
        res_size = sizeof(sSensorResInfo_VS6555_st)/sizeof(CamResolutionConfigure_t);
        res_ptr = &sSensorResInfo_VS6555_st[0];
    }
    else
    {
        pr_debug("CAMDRV_GetResolutionAvailable(): ERROR: Sensor Failed\r\n");
        result = HAL_CAM_ERROR_ACTION_NOT_SUPPORTED;
        return result;
    }
    
// Search Resolution table for best match for requested width & height based on capture mode
    best_match_ptr = res_ptr;
    for( index=0; index < res_size; index++ )
    {
        if ( (mode == CamCaptureVideo) || (mode == CamCaptureVideonViewFinder) )
        {
            if (res_ptr->previewIndex != -1)
            {
                best_match_ptr = res_ptr;
            }
    
            if ( (width <= res_ptr->resX) && ( height <= res_ptr->resY) && (res_ptr->previewIndex != -1) )
            {
                sensor_size->size = (CamImageSize_t)res_ptr->previewIndex;
                sensor_size->resX = res_ptr->resX;
                sensor_size->resY = res_ptr->resY;
                result = HAL_CAM_SUCCESS;
                pr_debug("CAMDRV_GetResolutionAvailable(): Resolution: size index=%d width=%d height=%d \r\n", res_ptr->previewIndex,res_ptr->resX,res_ptr->resY);
                return result;
            }
        }
        else if ( (mode == CamCaptureStill) || (mode == CamCaptureStillnThumb) )
        {
            if (res_ptr->captureIndex != -1)
            {
                best_match_ptr = res_ptr;
            }
    
            if ( (width <= res_ptr->resX) && ( height <= res_ptr->resY) && (res_ptr->captureIndex != -1) )
            {
                sensor_size->size = (CamImageSize_t)res_ptr->previewIndex;
                sensor_size->resX = res_ptr->resX;
                sensor_size->resY = res_ptr->resY;
                result = HAL_CAM_SUCCESS;
                pr_debug("CAMDRV_GetResolutionAvailable(): Resolution: size index=%d width=%d height=%d \r\n", res_ptr->captureIndex,res_ptr->resX,res_ptr->resY);
                return result;
            }
        }
        else
        {
            pr_debug("CAMDRV_GetResolutionAvailable(): ERROR: Mode Failed\r\n");
            result = HAL_CAM_ERROR_ACTION_NOT_SUPPORTED;
            return result;
        }
    // Increment table pointer        
        res_ptr++;
    }
// Resolution best match settings
    pr_debug("CAMDRV_GetResolutionAvailable(): Best Match used \r\n");
    sensor_size->size = (CamImageSize_t)best_match_ptr->previewIndex;
    sensor_size->resX = best_match_ptr->resX;
    sensor_size->resY = best_match_ptr->resY;
    return result;
}
/** @} */


/** The CAMDRV_GetResolution returns the sensor output size of the image resolution requested
    @param [in] size
        Image size requested from Sensor.
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
        HAL_CAM_ResolutionSize_st_t *sensor_size )
{
    HAL_CAM_Result_en_t result = HAL_CAM_ERROR_ACTION_NOT_SUPPORTED;
    UInt16 index, res_size;
    CamResolutionConfigure_t *res_ptr;

// Resolution default settings
    sensor_size->size = CamImageSize_INVALID;
    sensor_size->resX = 0;
    sensor_size->resY = 0;
    
// Init Resolution table pointer    
    if ( (sensor == CamSensorPrimary) && (sSensorPrimary == SensorVB6850) )
    {
        res_size = sizeof(sSensorResInfo_VB6850_st)/sizeof(CamResolutionConfigure_t);
        res_ptr = &sSensorResInfo_VB6850_st[0];
    }
    else if ( (sensor == CamSensorSecondary) && (sSensorSecondary == SensorVS6555) )
    {
        res_size = sizeof(sSensorResInfo_VS6555_st)/sizeof(CamResolutionConfigure_t);
        res_ptr = &sSensorResInfo_VS6555_st[0];
    }
    else
    {
        pr_debug("CAMDRV_GetResolution(): ERROR:  Sensor Failed \r\n");
        result = HAL_CAM_ERROR_ACTION_NOT_SUPPORTED;
        return result;
    }
    
// Search Resolution table for requested resolution based on capture mode (or largest resolution available)
    for( index=0; index < res_size; index++ )
    {
        pr_debug("CAMDRV_GetResolution(): Resolution: size index=%d width=%d height=%d \r\n", index,res_ptr->resX,res_ptr->resY);
        if ( (mode == CamCaptureVideo) || (mode == CamCaptureVideonViewFinder) )
        {
            if ( size == (CamImageSize_t)res_ptr->previewIndex )
            {
                sensor_size->size = (CamImageSize_t)res_ptr->previewIndex;
                sensor_size->resX = res_ptr->resX;
                sensor_size->resY = res_ptr->resY;
                result = HAL_CAM_SUCCESS;
                return result;
            }
            else if (res_ptr->previewIndex != -1)
            {
                sensor_size->size = (CamImageSize_t)res_ptr->previewIndex;
                sensor_size->resX = res_ptr->resX;
                sensor_size->resY = res_ptr->resY;
            }
        }
        else if ( (mode == CamCaptureStill) || (mode == CamCaptureStillnThumb) )
        {
            if ( size == (CamImageSize_t)res_ptr->captureIndex )
            {
                sensor_size->size = (CamImageSize_t)res_ptr->captureIndex;
                sensor_size->resX = res_ptr->resX;
                sensor_size->resY = res_ptr->resY;
                result = HAL_CAM_SUCCESS;
                return result;
            }
            else if (res_ptr->captureIndex != -1)
            {
                sensor_size->size = (CamImageSize_t)res_ptr->captureIndex;
                sensor_size->resX = res_ptr->resX;
                sensor_size->resY = res_ptr->resY;
            }
        }
        else
        {
            pr_debug("CAMDRV_GetResolution(): ERROR:  Mode Failed \r\n");
            result = HAL_CAM_ERROR_ACTION_NOT_SUPPORTED;
            return result;
        }
    // Increment table pointer        
        res_ptr++;
    }
    pr_debug("CAMDRV_GetResolution(): ERROR:  Resolution Failed \r\n");
    return result;
}
/** @} */

//****************************************************************************
//
// Function Name:   HAL_CAM_Result_en_t CAMDRV_Wakeup(CamSensorSelect_t sensor)
//
// Description: This function wakesup camera via I2C command.  Assumes camera
//              is enabled.
//
// Notes:
//
//****************************************************************************
HAL_CAM_Result_en_t CAMDRV_Wakeup(CamSensorSelect_t sensor)
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
    
    SensorSetPowerDown(FALSE);                  // Sensor Power Down
    result = Init_Stv0986(sensor);
    pr_debug("CAMDRV_Wakeup(): Camera wakeup init %d \r\n", result);
    return result;
}

//****************************************************************************
//
// Function Name:   void CAMDRV_SetCamSleep(CamSensorSelect_t sensor )
//
// Description: This function puts ISP in sleep mode & shuts down power.
//
// Notes:
//
//****************************************************************************
HAL_CAM_Result_en_t  CAMDRV_SetCamSleep( CamSensorSelect_t sensor )
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;

    pr_debug("CAMDRV_SetCamSleep(): \r\n");
    
    result = SensorSetSleepMode();         // ISP in Sleep Mode
    SensorSetPowerDown(FALSE);                  // Sensor in Power Down
    return result;
}


//****************************************************************************
//
// Function Name:   HAL_CAM_Result_en_t CAMDRV_SetVideoCaptureMode(CamImageSize_t image_resolution, CamDataFmt_t image_format)
//
// Description: This function configures Video Capture (Same as ViewFinder Mode)
//
// Notes:
//
//****************************************************************************
HAL_CAM_Result_en_t CAMDRV_SetVideoCaptureMode(
        CamImageSize_t image_resolution, 
        CamDataFmt_t image_format,
        CamSensorSelect_t sensor
        )
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;

// --------Set up Camera Isp for Output Resolution & Format
    result |= SensorSetFrameRate(ImageSettingsConfig_st.sensor_framerate->cur_setting, sensor);
    SensorSetPreviewMode(image_resolution, image_format);
    SensorSetCaptureMode(image_resolution, image_format);
    SensorSetInterLeaveMode(CamCaptureVideo, image_format);
// -------- Test mode
//  stv_write(0x030e, 1);
    pr_debug("CAMDRV_SetVideoCaptureMode(): return result=%d \r\n", result);
    return result;
}

//****************************************************************************
//
// Function Name:   HAL_CAM_Result_en_t CAMDRV_SetVideoPreviewCaptureMode
//
// Description: This function configures Stills Capture
//  @param video_resolution       [in] .
//  @param video_format           [in] .
//  @param viewfinder_resolution  [in] .
//  @param viewfinder_format      [in] .
//  @param sensor                 [in] .
//  @return       HAL_CAM_Result_en_t 
//
// Notes:
//
//****************************************************************************
HAL_CAM_Result_en_t CAMDRV_SetVideoPreviewCaptureMode(
        CamImageSize_t video_resolution, 
        CamDataFmt_t video_format,
        CamImageSize_t viewfinder_resolution, 
        CamDataFmt_t viewfinder_format,
        CamSensorSelect_t sensor
        )
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;

// Set up Camera Isp for Output Resolution & Format
    result |= SensorSetFrameRate(ImageSettingsConfig_st.sensor_framerate->cur_setting, sensor);
    SensorSetPreviewMode(viewfinder_resolution, viewfinder_format);
    SensorSetCaptureMode(video_resolution, video_format);
    SensorSetInterLeaveMode(CamCaptureVideonViewFinder, video_format);
    pr_debug("CAMDRV_SetVideoPreviewCaptureMode(): return result=%d \r\n", result);
    return result;
}

//****************************************************************************
//
// Function Name:   HAL_CAM_Result_en_t CAMDRV_EnableVideoCapture(CamSensorSelect_t sensor)
//
// Description: This function starts camera video capture mode
//
// Notes:
//                  SLEEP -> IDLE -> Movie
//****************************************************************************
HAL_CAM_Result_en_t CAMDRV_EnableVideoCapture(CamSensorSelect_t    sensor)
{
    UInt8 register_value,error_value;
    HAL_CAM_Result_en_t result;

//---------Read bSystemStatus   
    register_value = stv_read(bSystemStatus);       
    pr_debug("CAMDRV_EnableVideoCapture(): bSystemStatus = %d \r\n",register_value);
    if (register_value != STATE_IDLE)       
    {
        result = SensorSetSleepMode();         // ISP in Sleep Mode
    // ---------Idle Mode   
       // bSystemControl possible values:    
        // 0x49=73  984 enters in STATE_UNINITIALIZED    
        // 0x39=57 984 enters in STATE_BOOT    
        // 0x11=17 984 enters in STATE_SLEEP    
        // 0x12=18 984 enters in STATE_IDLE    
        // 0x13=19 984 enters in STATE_VIEWFINDER    
        // 0x14=20 984 enters in STATE_STILLS    
        // 0x15=21 984 enters in STATE_MOVIE    
        // 0x16=22 984 enters in STATE_STILL_N_THUMBNAIL    
        // 0x18=24 984 enters in STATE_BUSY  
        stv_write(bSystemControl, STATE_IDLE);                  // SystemControlParameters_bSystemControl Idle mode
        stv_read(bSystemStatus);                                // read bSystemStatus for I2C write to complete
        result = stv_WaitValue(2000,bSystemStatus,STATE_IDLE);  // Poll system status to confirm Idle state.
        if (result != HAL_CAM_SUCCESS)
        {
            register_value = stv_read(bSystemStatus);       
            error_value = stv_read(bSystemErrorStatus);      
            pr_debug("CAMDRV_EnableVideoCapture(): ERROR:  STATE_IDLE Failed: bSystemStatus = %d bSystemErrorStatus = %d \r\n", register_value,error_value);
        }
    }       // if (register_value != STATE_IDLE)

    if( stv0986_sys_flash_state == Torch_On )
    {
        if( !stv0986_torch_is_on )
        {
            stv0986_GpioSetTorchModeTorchOn();
        }    
    }

   // bSystemControl possible values:    
   // 0x49=73   984 enters in STATE_UNINITIALIZED    
   // 0x39=57 984 enters in STATE_BOOT    
   // 0x11=17 984 enters in STATE_SLEEP    
   // 0x12=18 984 enters in STATE_IDLE    
   // 0x13=19 984 enters in STATE_VIEWFINDER    
   // 0x14=20 984 enters in STATE_STILLS    
   // 0x15=21 984 enters in STATE_MOVIE    
   // 0x16=22 984 enters in STATE_STILL_N_THUMBNAIL    
   // 0x18=24 984 enters in STATE_BUSY  
    stv_write(bSystemControl, STATE_MOVIE);              // SystemControlParameters_bSystemControl Movie mode
    result = stv_WaitValue(2000,bSystemStatus,STATE_MOVIE);  // Poll system status to confirm 986 is in movie state.
    if (result != HAL_CAM_SUCCESS)
    {
        register_value = stv_read(bSystemStatus);       
        error_value = stv_read(bSystemErrorStatus);      
        pr_debug("CAMDRV_EnableVideoCapture(): ERROR:  STATE_MOVIE Failed: bSystemStatus = %d bSystemErrorStatus = %d \r\n", register_value,error_value);
    }
// AF Position Start
    if ( sCamSensorSel == SensorVB6850 )
    {
        AF_PositionSensorSetup();
    }
            
// Read Status Registers  
#ifdef CAMDBG_STATUS
    pr_debug("CAMDRV_EnableVideoCapture(): Status Registers after STATE_MOVIE Command:\r\n");
    stv_Read_Status();
#endif
                    
//---------Read bSystemStatus   ***Needed for I2C Stop Condition Error***
    register_value = stv_read(bSystemStatus);       
    return result;
}



//****************************************************************************
//
// Function Name:   HAL_CAM_Result_en_t CAMDRV_CfgCapture(CamImageSize_t image_resolution, CamDataFmt_t format, CamSensorSelect_t sensor)
//
// Description: This function configures Stills Capture
//
// Notes:
//
//****************************************************************************
HAL_CAM_Result_en_t CAMDRV_CfgCapture(CamImageSize_t image_resolution, CamDataFmt_t image_format, CamSensorSelect_t sensor)
{
    UInt8 register_value, error_value;
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;


//---------Read bSystemStatus   
    register_value = stv_read(bSystemStatus);       
    pr_debug("CAMDRV_CfgCapture(): bSystemStatus = %d \r\n",register_value);

            #if 1 /*!defined(MARTINI_HW)    -- FIX ME!!--  Temporary fix for Martini capture*/
        if ( (register_value == STATE_VIEWFINDER) || (register_value == STATE_MOVIE) )
        {
    // AF Position Start
            if ( sCamSensorSel == SensorVB6850 )
            {
                AF_QuickSearch();
            }
        }
            #endif
        if (register_value != STATE_IDLE)       
//      if (register_value != STATE_VIEWFINDER)     
        {
    // ---------Idle Mode   
           // bSystemControl possible values:    
       // 0x49=73   984 enters in STATE_UNINITIALIZED    
       // 0x39=57 984 enters in STATE_BOOT    
       // 0x11=17 984 enters in STATE_SLEEP    
       // 0x12=18 984 enters in STATE_IDLE    
       // 0x13=19 984 enters in STATE_VIEWFINDER    
       // 0x14=20 984 enters in STATE_STILLS    
       // 0x15=21 984 enters in STATE_MOVIE    
       // 0x16=22 984 enters in STATE_STILL_N_THUMBNAIL    
       // 0x18=24 984 enters in STATE_BUSY  
            stv_write(bSystemControl, STATE_IDLE);                              // SystemControlParameters_bSystemControl Idle mode
            stv_read(bSystemStatus);                                                        // read bSystemStatus for I2C write to complete
            result = stv_WaitValue(2000,bSystemStatus,STATE_IDLE);  // Poll system status to confirm Idle state.
            if (result != HAL_CAM_SUCCESS)
            {
                register_value = stv_read(bSystemStatus);       
                error_value = stv_read(bSystemErrorStatus);      
                pr_debug("CAMDRV_CfgCapture(): ERROR:  STATE_IDLE Failed: bSystemStatus = %d bSystemErrorStatus = %d \r\n", register_value,error_value);
            }
        }

// --------Set up Camera Isp for Output Resolution & Format
        result |= SensorSetFrameRate(CamRate_15, sensor);               // 15 Fps for Stills Capture
        SensorSetCaptureMode(image_resolution, image_format);
        SensorSetInterLeaveMode(CamCaptureStill, image_format);

    if( stv0986_sys_flash_state == Flash_On || stv0986_sys_flash_state == FlashLight_Auto )
    {
        if( stv0986_torch_is_on )
        {
            stv0986_GpioSetFlashModeTorchOff();
        }    
//        stv0986_FlashManagerOn( FLASH_PRE_COUNT, FLASH_PRE_LEN_uS, FLASH_MAIN_LEN_uS );
    }

   // bSystemControl possible values:    
   // 0x49=73   984 enters in STATE_UNINITIALIZED    
   // 0x39=57 984 enters in STATE_BOOT    
   // 0x11=17 984 enters in STATE_SLEEP    
   // 0x12=18 984 enters in STATE_IDLE    
   // 0x13=19 984 enters in STATE_VIEWFINDER    
   // 0x14=20 984 enters in STATE_STILLS    
   // 0x15=21 984 enters in STATE_MOVIE    
   // 0x16=22 984 enters in STATE_STILL_N_THUMBNAIL    
   // 0x18=24 984 enters in STATE_BUSY  
#if 1
    stv_write(bSystemControl, STATE_STILLS);                             // SystemControlParameters_bSystemControl Stills mode
    stv_read(bSystemStatus);                                                             // read bSystemStatus for I2C write to complete
    result = stv_WaitValue(2000,bSystemStatus,STATE_STILLS);     // Poll system status to confirm Stills state.
    if (result != HAL_CAM_SUCCESS)
    {
        register_value = stv_read(bSystemStatus);       
        error_value = stv_read(bSystemErrorStatus);      
        pr_debug("CAMDRV_CfgCapture(): ERROR:  STATE_STILLS Failed: bSystemStatus = %d bSystemErrorStatus = %d \r\n", register_value,error_value);
    }
#else
    stv_write(bSystemControl, STATE_MOVIE);                              // SystemControlParameters_bSystemControl Stills mode
    stv_read(bSystemStatus);                                                             // read bSystemStatus for I2C write to complete
    result = stv_WaitValue(2000,bSystemStatus,STATE_MOVIE);  // Poll system status to confirm Stills state.
    if (result != HAL_CAM_SUCCESS)
    {
        register_value = stv_read(bSystemStatus);       
        error_value = stv_read(bSystemErrorStatus);      
        pr_debug("CAMDRV_CfgCapture(): ERROR:  STATE_MOVIE Failed: bSystemStatus = %d bSystemErrorStatus = %d \r\n", register_value,error_value);
    }
#endif      

    return result;
}

//****************************************************************************
//
// Function Name:   HAL_CAM_Result_en_t CAMDRV_CfgStillnThumbCapture(CamImageSize_t image_resolution, CamDataFmt_t format, CamSensorSelect_t sensor)
//
// Description: This function configures Stills Capture
//
// Notes:
//
//****************************************************************************
HAL_CAM_Result_en_t CAMDRV_CfgStillnThumbCapture(
    CamImageSize_t stills_resolution, 
    CamDataFmt_t stills_format,
    CamImageSize_t thumb_resolution, 
    CamDataFmt_t thumb_format,
    CamSensorSelect_t sensor )
{
    UInt8 register_value,error_value;
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;

//---------Read bSystemStatus   
    register_value = stv_read(bSystemStatus);       
    pr_debug("CAMDRV_CfgStillnThumbCapture(): bSystemStatus = %d \r\n",register_value);

        if ( (register_value == STATE_VIEWFINDER) || (register_value == STATE_MOVIE) )
        {
    // AF Position Start
            if ( sCamSensorSel == SensorVB6850 )
            {
                AF_QuickSearch();
            }
        }
        if (register_value != STATE_IDLE)       
//      if (register_value != STATE_VIEWFINDER)     
        {
    // ---------Idle Mode   
           // bSystemControl possible values:    
       // 0x49=73   984 enters in STATE_UNINITIALIZED    
       // 0x39=57 984 enters in STATE_BOOT    
       // 0x11=17 984 enters in STATE_SLEEP    
       // 0x12=18 984 enters in STATE_IDLE    
       // 0x13=19 984 enters in STATE_VIEWFINDER    
       // 0x14=20 984 enters in STATE_STILLS    
       // 0x15=21 984 enters in STATE_MOVIE    
       // 0x16=22 984 enters in STATE_STILL_N_THUMBNAIL    
       // 0x18=24 984 enters in STATE_BUSY  
            stv_write(bSystemControl, STATE_IDLE);                  // SystemControlParameters_bSystemControl Idle mode
            stv_read(bSystemStatus);                                // read bSystemStatus for I2C write to complete
            result = stv_WaitValue(2000,bSystemStatus,STATE_IDLE);  // Poll system status to confirm Idle state.
            if (result != HAL_CAM_SUCCESS)
            {
                register_value = stv_read(bSystemStatus);       
                error_value = stv_read(bSystemErrorStatus);      
                pr_debug("CAMDRV_CfgStillnThumbCapture(): ERROR:  STATE_IDLE Failed: bSystemStatus = %d bSystemErrorStatus = %d \r\n", register_value,error_value);
            }
        }

    // Set up Stills & Thumb captures
        result |= SensorSetFrameRate(CamRate_15, sensor);               // 15 Fps for Stills Capture
        SensorSetPreviewMode(thumb_resolution, thumb_format);
        SensorSetCaptureMode(stills_resolution, stills_format);
//        SensorSetInterLeaveMode(CamCaptureStill, stills_format);
        SensorSetInterLeaveMode(CamCaptureStillnThumb, stills_format);
        
    if( stv0986_sys_flash_state == Flash_On || stv0986_sys_flash_state == FlashLight_Auto )
    {
        if( stv0986_torch_is_on )
        {
            stv0986_GpioSetFlashModeTorchOff();
        }    
//        stv0986_FlashManagerOn( FLASH_PRE_COUNT, FLASH_PRE_LEN_uS, FLASH_MAIN_LEN_uS );
    }

   // bSystemControl possible values:    
   // 0x49=73   984 enters in STATE_UNINITIALIZED    
   // 0x39=57 984 enters in STATE_BOOT    
   // 0x11=17 984 enters in STATE_SLEEP    
   // 0x12=18 984 enters in STATE_IDLE    
   // 0x13=19 984 enters in STATE_VIEWFINDER    
   // 0x14=20 984 enters in STATE_STILLS    
   // 0x15=21 984 enters in STATE_MOVIE    
   // 0x16=22 984 enters in STATE_STILL_N_THUMBNAIL    
   // 0x18=24 984 enters in STATE_BUSY  
#if 1
   stv_write(bSystemControl, STATE_STILLS);                             // SystemControlParameters_bSystemControl Stills mode
   stv_read(bSystemStatus);                                                             // read bSystemStatus for I2C write to complete
   result = stv_WaitValue(2000,bSystemStatus,STATE_STILLS);     // Poll system status to confirm Stills state.
    if (result != HAL_CAM_SUCCESS)
    {
        register_value = stv_read(bSystemStatus);       
        error_value = stv_read(bSystemErrorStatus);      
        pr_debug("CAMDRV_CfgStillnThumbCapture(): ERROR:  STATE_STILLS Failed: bSystemStatus = %d bSystemErrorStatus = %d \r\n", register_value,error_value);
    }
#else
   stv_write(bSystemControl, STATE_MOVIE);                              // SystemControlParameters_bSystemControl Stills mode
   stv_read(bSystemStatus);                                                             // read bSystemStatus for I2C write to complete
   result = stv_WaitValue(2000,bSystemStatus,STATE_MOVIE);  // Poll system status to confirm Stills state.
    if (result != HAL_CAM_SUCCESS)
    {
        register_value = stv_read(bSystemStatus);       
        error_value = stv_read(bSystemErrorStatus);      
        pr_debug("CAMDRV_CfgStillnThumbCapture(): ERROR:  STATE_MOVIE Failed: bSystemStatus = %d bSystemErrorStatus = %d \r\n", register_value,error_value);
    }
#endif      
    return result;
}


//****************************************************************************
//
// Function Name:   HAL_CAM_Result_en_t CAMDRV_DisableCapture(CamSensorSelect_t sensor)
//
// Description: This function halts camera video capture
//
// Notes:
//                  ViewFinder -> IDLE
//                  Movie -> IDLE
//                  Stills -> IDLE
//****************************************************************************
HAL_CAM_Result_en_t CAMDRV_DisableCapture(CamSensorSelect_t sensor)
{
    UInt8 register_value,error_value;
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;


//---------Read bSystemStatus   
    register_value = stv_read(bSystemStatus);       
    pr_debug("CAMDRV_DisableCapture(): bSystemStatus = %d \r\n",register_value);
        if (register_value != STATE_SLEEP)      
        {
            if (register_value != STATE_IDLE)       
            {
        
           // bSystemControl possible values:    
           // 0x49=73   984 enters in STATE_UNINITIALIZED    
           // 0x39=57 984 enters in STATE_BOOT    
           // 0x11=17 984 enters in STATE_SLEEP    
           // 0x12=18 984 enters in STATE_IDLE    
           // 0x13=19 984 enters in STATE_VIEWFINDER    
           // 0x14=20 984 enters in STATE_STILLS    
           // 0x15=21 984 enters in STATE_MOVIE    
           // 0x16=22 984 enters in STATE_STILL_N_THUMBNAIL    
           // 0x18=24 984 enters in STATE_BUSY  
                stv_write(bSystemControl, STATE_IDLE);                              // SystemControlParameters_bSystemControl Idle mode
                stv_read(bSystemStatus);                                                        // read bSystemStatus for I2C write to complete
                result = stv_WaitValue(1000,bSystemStatus,STATE_IDLE);  // Poll system status to confirm Idle state.
                if (result != HAL_CAM_SUCCESS)
                {
                    register_value = stv_read(bSystemStatus);       
                    error_value = stv_read(bSystemErrorStatus);      
                    pr_debug("CAMDRV_DisableCapture(): ERROR #1:  STATE_IDLE Failed: bSystemStatus = %d bSystemErrorStatus = %d \r\n", register_value,error_value);
                // Try 2nd time
                    stv_write(bSystemControl, STATE_IDLE);                              // SystemControlParameters_bSystemControl Idle mode
                    stv_read(bSystemStatus);                                                        // read bSystemStatus for I2C write to complete
                    result = stv_WaitValue(1000,bSystemStatus,STATE_IDLE);  // Poll system status to confirm Idle state.
                    register_value = stv_read(bSystemStatus);       
                    error_value = stv_read(bSystemErrorStatus);      
                    pr_debug("CAMDRV_DisableCapture(): ERROR #2:  STATE_IDLE Failed: bSystemStatus = %d bSystemErrorStatus = %d \r\n", register_value,error_value);
                }
            }
        }
    if( stv0986_torch_is_on )
    {
        stv0986_GpioSetFlashModeTorchOff();
    }    
    if( stv0986_fm_is_on )
    {
        stv0986_FlashManagerOff();
    }    
    return result;
}

//****************************************************************************
//
// Function Name:   HAL_CAM_Result_en_t CAMDRV_SetViewFinderMode(CamImageSize_t image_resolution, CamDataFmt_t image_format)
//
// Description: This function configures ViewFinder
//
// Notes:
//
//****************************************************************************
HAL_CAM_Result_en_t CAMDRV_SetViewFinderMode(
        CamImageSize_t image_resolution, 
        CamDataFmt_t image_format,
        CamSensorSelect_t sensor
        )
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;

    result = CAMDRV_SetVideoCaptureMode(image_resolution, image_format, sensor);
    pr_debug("CAMDRV_SetViewFinderMode(): return result=%d \r\n", result);
    return result;
}


//****************************************************************************
//
// Function Name:   HAL_CAM_Result_en_t CAMDRV_EnablePreview(CamSensorSelect_t sensor)
//
// Description: This function starts camera preview mode
//
// Notes:
//                  SLEEP -> IDLE -> ViewFinder
//****************************************************************************
HAL_CAM_Result_en_t CAMDRV_EnablePreview(CamSensorSelect_t sensor)
{
    HAL_CAM_Result_en_t result;

    result = CAMDRV_EnableVideoCapture(sensor);
    pr_debug("CAMDRV_EnablePreview(): return result=%d \r\n", result);
    return result;
}


//****************************************************************************
//
// Function Name:   HAL_CAM_Result_en_t CAMDRV_DisablePreview(void)
//
// Description: This function halts MT9M111 camera video
//
// Notes:
//                  ViewFinder -> SLEEP
//****************************************************************************
HAL_CAM_Result_en_t CAMDRV_DisablePreview(CamSensorSelect_t    sensor)
{
    HAL_CAM_Result_en_t result;

    result = CAMDRV_DisableCapture(sensor);
    if (result != HAL_CAM_SUCCESS)
    {
        pr_debug("CAMDRV_DisablePreview(): ERROR:  \r\n");
    }
    return result;
}

//****************************************************************************
//
// Function Name:   HAL_CAM_Result_en_t CAMDRV_CfgPreview(UInt16 index, UInt16 enable, CamSensorSelect_t nSensor)
//
// Description: This function halts MT9M111 camera video
//
// Notes:
//
//****************************************************************************
HAL_CAM_Result_en_t CAMDRV_CfgPreview(UInt16 image_resolution, UInt16 enable, CamSensorSelect_t nSensor)
{
    HAL_CAM_Result_en_t result;

    result = HAL_CAM_ERROR_INTERNAL_ERROR;
    return result;
}



//****************************************************************************
//
// Function Name:   HAL_CAM_Result_en_t CAMDRV_SetAutoFocusMode()
//
// Description: This function sets the AutoFocus Mode
//
// Notes:
//
//****************************************************************************
HAL_CAM_Result_en_t    CAMDRV_SetAutoFocusMode( 
        UInt16 af_mode
        )
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;

    stv_write(bFocusMode, (UInt8)af_mode);                      // FocusControls bFocusMode 
    stv_read(bSystemStatus);                                                        // read bSystemStatus for I2C write to complete
        return result;
}

//****************************************************************************
//
// Function Name:   HAL_CAM_Result_en_t CAMDRV_SetAutoFocusCmd()
//
// Description: This function sets the AutoFocus Command
//
// Notes:
//
//****************************************************************************
HAL_CAM_Result_en_t    CAMDRV_SetAutoFocusCmd( 
        UInt16 af_command
        )
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
    
        stv_write(bAFCommand, (UInt8)af_command);                           // FocusControls bAFCommand
        result = stv_WaitValue(2000, fLensIsMoving, 0x00); // Poll FLADriverStatus_fLensIsMoving
	    pr_debug("CAMDRV_SetAutoFocusCmd(): return result=%d \r\n", result);
        return result;
}

//****************************************************************************
// Function Name:   HAL_CAM_Result_en_t CAMDRV_SetSceneMode(CAM_DRV_SceneMode_t scene_mode)
// Description:     Set the required Scene mode (or AUTO scene).
//      Notice that if any Special Effect is active then it is disabled.
// Notes:   Average users can use this function to choose different settings for still capture.
//      The AUTO scene mode is good enough for all shooting needs.
//****************************************************************************
HAL_CAM_Result_en_t CAMDRV_SetSceneMode(CamSceneMode_t scene_mode, CamSensorSelect_t sensor)
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
    
    pr_debug("CAMDRV_SetSceneMode(): scene_mode = %d \r\n", scene_mode);
    
    if(scene_mode >= CamSceneMode_N_Scenes)
    {
        result = HAL_CAM_ERROR_INTERNAL_ERROR;
    }

// --------Update Current setting
    if (result == HAL_CAM_SUCCESS)
    {
        ImageSettingsConfig_st.sensor_scene->cur_setting = scene_mode;
    }

    result = HAL_CAM_ERROR_INTERNAL_ERROR;          // FIX ME!!!
    return result;
}

//****************************************************************************
// Function Name:   HAL_CAM_Result_en_t CAMDRV_SetDigitalEffect( CamDigEffect_t effect)
//
// Description: This function will set the digital effect of camera
// Notes:
//****************************************************************************
HAL_CAM_Result_en_t CAMDRV_SetDigitalEffect( CamDigEffect_t effect, CamSensorSelect_t sensor)
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
   //Camera crashes when Solarize effect is selected. Negative effect not working[White preview screen displayed].
   //For testing purpose, all the effects are disabled to avoid crash and side effects.
    return result;




    
    pr_debug("CAMDRV_SetDigitalEffect(): effect = %d \r\n", effect);
    switch( effect )
    {
        case CamDigEffect_NoEffect:
            effect = CamDigEffect_NoEffect;
            stv0986_SetMonochrome( FALSE );
            stv_write( bPipe0Control_fSfxNegativeEnabled, 0x00 );    
            stv_write( bPipe0Control_fSfxSolariseEnabled, 0x00 );    
            stv_write( bPipe1Control_fSfxNegativeEnabled, 0x00 );    
            stv_write( bPipe1Control_fSfxSolariseEnabled, 0x00 );  
            break;
        case CamDigEffect_MonoChrome:
            stv_write( bPipe0Control_fSfxSolariseEnabled, 0x00 );    
            stv_write( bPipe0Control_fSfxNegativeEnabled, 0x00 );    
            stv_write( bPipe1Control_fSfxSolariseEnabled, 0x00 );    
            stv_write( bPipe1Control_fSfxNegativeEnabled, 0x00 );    
            stv0986_SetMonochrome( TRUE );
            break;
        case CamDigEffect_NegColor:
            stv0986_SetMonochrome( FALSE );
            stv_write( bPipe0Control_fSfxSolariseEnabled, 0x00 );    
            stv_write( bPipe0Control_fSfxNegativeEnabled, 0x01 );    
            stv_write( bPipe1Control_fSfxSolariseEnabled, 0x00 );    
            stv_write( bPipe1Control_fSfxNegativeEnabled, 0x01 );    
            break;
        case CamDigEffect_SolarizeColor:
            stv0986_SetMonochrome( FALSE );
            stv_write( bPipe0Control_fSfxNegativeEnabled, 0x00 );    
            stv_write( bPipe0Control_fSfxSolariseEnabled, 0x01 );    
            stv_write( bPipe1Control_fSfxNegativeEnabled, 0x00 );    
            stv_write( bPipe1Control_fSfxSolariseEnabled, 0x01 );    
            break;
        default:
            effect = CamDigEffect_NoEffect;
            stv0986_SetMonochrome( FALSE );
            stv_write( bPipe0Control_fSfxNegativeEnabled, 0x00 );    
            stv_write( bPipe0Control_fSfxSolariseEnabled, 0x00 );    
            stv_write( bPipe1Control_fSfxNegativeEnabled, 0x00 );    
            stv_write( bPipe1Control_fSfxSolariseEnabled, 0x00 );  
            result = HAL_CAM_ERROR_ACTION_NOT_SUPPORTED;  
            break;
    }
// --------Update Current setting
    ImageSettingsConfig_st.sensor_digitaleffect->cur_setting = effect;
    stv_read(bSystemStatus);                                                        // read bSystemStatus for I2C write to complete
    return result;
}

//****************************************************************************
// Function Name:   HAL_CAM_Result_en_t CAMDRV_SetWBMode(CamWB_WBMode_t wb_mode)
//
// Description:     Set the required WhiteBalance mode manually or its AUTO mode.
//      Notice that if any Special Effect is active it is disabled.
// Notes:    Only advanced users need to use these. (base on light source)
//      For most users the AUTO mode is good enough for all shooting needs.
//****************************************************************************
HAL_CAM_Result_en_t CAMDRV_SetWBMode(CamWB_WBMode_t wb_mode, CamSensorSelect_t sensor)
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;

    pr_debug("CAMDRV_SetWBMode(): wb_mode = %d \r\n", wb_mode);
        stv_write(0x2680, 0x20);     //WhiteBalanceStatisticsControls bLowThreshold

    switch( wb_mode )
    {
        case CamWB_Off:
            stv_write(0x2500, 0x00);     //WhiteBalanceControls bWBMode {OFF}
            break;
        case CamWB_Auto:
            stv_write(0x2500, 0x01);     //WhiteBalanceControls bWBMode {AUTOMATIC}
            break;
        case CamWB_Daylight:
            stv_write(0x2500, 0x04);     //WhiteBalanceControls bWBMode {DAYLIGHT_PRESET}
            break;
        case CamWB_Tungsten:
            stv_write(0x2500, 0x05);     //WhiteBalanceControls bWBMode {TUNGSTEN_PRESET}
            break;
        case CamWB_DaylightFluorescent:
            stv_write(0x2500, 0x06);     //WhiteBalanceControls bWBMode {FLUORESCENT_PRESET}
            break;
        case CamWB_Sunset:
            stv_write(0x2500, 0x07);     //WhiteBalanceControls bWBMode {HORIZON_PRESET}
            break;
        case CamWB_Flash:
            stv_write(0x2500, 0x08);     //WhiteBalanceControls bWBMode {FLASHGUN_PRESET}
            break;
        default:
            wb_mode = CamWB_Auto;
            result = HAL_CAM_ERROR_ACTION_NOT_SUPPORTED;   // FIX ME!!!
            break;
    }

// --------Update Current setting
    ImageSettingsConfig_st.sensor_wb->cur_setting = wb_mode;
    return result;
}

//****************************************************************************
// Function Name:   HAL_CAM_Result_en_t CAMDRV_SetImageQuality(Int8 quality, CamSensorSelect_t sensor)
// Description:     Set the JPEG Quality (quantization) level [0-100]:
// Notes:       This function can be for average user's use.
//****************************************************************************
HAL_CAM_Result_en_t CAMDRV_SetImageQuality(UInt8 quality, CamSensorSelect_t sensor)
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
    UInt8 setting;

    if (quality < 34)
    {
        setting = STILL_LOW_QUALITY;
    }
    else if (quality < 67)
    {
        setting = STILL_MEDIUM_QUALITY;
    }
    else
    {
        setting = STILL_HIGH_QUALITY;
    }

// --------Update Current setting
    if (result == HAL_CAM_SUCCESS)
    {
        result |= stv_write( bImageQuality, setting );    
        ImageSettingsConfig_st.sensor_jpegQuality->cur_setting = quality;
        if (result != HAL_CAM_SUCCESS)
        {
            pr_debug("CAMDRV_SetImageQuality(): FAILED \r\n");
        }
    }
    stv_read(bSystemStatus);                                                        // read bSystemStatus for I2C write to complete
    pr_debug("CAMDRV_SetImageQuality(): quality = %d \r\n", quality);
    return result;
}

/** Get Jpeg max size for camera Sensor (UInt32* psize, CamSensorSelect_t sensor)
    @param  size				[in].
    @param  sensor              [in] .
    @note   Maximum Jpeg output size (bytes)
*/
HAL_CAM_Result_en_t CAMDRV_GetJpegMaxSize(UInt32 in_psize, CamSensorSelect_t sensor)
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
	UInt32*	psize = (UInt32*)in_psize;

    *psize = (CamPrimaryCfgCap_st.output_st.max_width * CamPrimaryCfgCap_st.output_st.max_height) >> 2;

    if (sensor == CamSensorSecondary)
    {
        *psize = (CamSecondaryCfgCap_st.output_st.max_width * CamSecondaryCfgCap_st.output_st.max_height) >> 2;
    }
    pr_debug("CAMDRV_GetJpegMaxSize(): size = %d \r\n", *psize);
    return result;
}


/** Get luminance for camera Sensor (in bytes)
    @param in_luminance         [in].
    @param  sensor              [in].
    @note   luminance (0: normal, 1: middle, 2: low)
*/
HAL_CAM_Result_en_t CAMDRV_GetLuminance(UInt32 in_luminance, CamSensorSelect_t sensor)
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
    UInt32* pluminance = (UInt32*)in_luminance;

    if (sensor == CamSensorSecondary)
    {
        *pluminance = 2;
    }
    else
    {
        *pluminance = 1;
    }
    pr_debug("CAMDRV_GetLuminance(): luminance = %d \r\n", *pluminance);
    return result;
}

/** Lock auto color (in bytes)
    @param  color               [in] .
    @return UInt32              [out].
    @note   auto color (0: AE, 1: AWB, 2: ALL)
*/
HAL_CAM_Result_en_t CAMDRV_LockAutoColor(UInt32 in_pcolor)
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
	UInt32* pcolor = (UInt32*) in_pcolor;

    pr_debug("CAMDRV_LockAutoColor(): AutoColor = %d \r\n", *pcolor);
    return result;
}

/** Unlock auto color (in bytes)
    @param  color               [in] .
    @return UInt32              [out].
    @note   umlock auto color (0: AE, 1: AWB, 2: ALL)
*/
HAL_CAM_Result_en_t CAMDRV_UnlockAutoColor(UInt32 in_pcolor)
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
	UInt32* pcolor = (UInt32*) in_pcolor;

    pr_debug("CAMDRV_UnlockAutoColor(): AutoColor = %d \r\n", *pcolor);
    return result;
}
    


//****************************************************************************
// Function Name:   HAL_CAM_Result_en_t CAMDRV_SetContrast(Int8 contrast)
// Description:     Set contrast for preview image.
// Notes:       This function can be for average user's use.
//****************************************************************************
HAL_CAM_Result_en_t CAMDRV_SetContrast(Int8 contrast, CamSensorSelect_t sensor)
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
    UInt8 contrast_tbl[MAX_CONTRAST] = {   0,  25,  50,  75,  90, 110, 120, 135, 150, 175, 200 };
    int i;
    UInt16 tbl_offset=NOMINAL_CONTRAST;
    Int8 compare = (Int8)CamContrast_Min;
    Int16 step = (CamContrast_Max - CamContrast_Min) / (MAX_CONTRAST - MIN_CONTRAST - 1);

    if( contrast > CamContrast_Max )
        contrast = CamContrast_Max;
    if( contrast < CamContrast_Min )
        contrast = CamContrast_Min;
    
    for (i = MIN_CONTRAST; i < MAX_CONTRAST; i++)
    {
        if (compare <= contrast)
        {
            tbl_offset = i;
        }
        compare += step;
    }

// --------Update Current setting
    if (result == HAL_CAM_SUCCESS)
    {
        stv_write( bColorEngine0_bContrast, contrast_tbl[tbl_offset] );    
        stv_write( bColorEngine1_bContrast, contrast_tbl[tbl_offset] );    
        ImageSettingsConfig_st.sensor_contrast->cur_setting = contrast;
    }
    stv_read(bSystemStatus);                                                        // read bSystemStatus for I2C write to complete
    pr_debug("CAMDRV_SetContrast(): tbl_offset=%d contrast=%d \r\n", tbl_offset, contrast);
    return result;
}


//****************************************************************************
// Function Name:   HAL_CAM_Result_en_t CAMDRV_SetBrightness(Int8 brightness)
// Description:
// Notes:       This function can be for average user's use.
//****************************************************************************
HAL_CAM_Result_en_t CAMDRV_SetBrightness(Int8 brightness, CamSensorSelect_t sensor)
{
    HAL_CAM_Result_en_t result = HAL_CAM_ERROR_ACTION_NOT_SUPPORTED;      // FIX ME!!!

#if 0   
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
    int brightness_tbl[MAX_BRIGHTNESS] = {-127, -90, -60, -35, -15, 0, 15, 35, 60, 90, 128};
    int i;
    UInt16 tbl_offset=NOMINAL_BRIGHTNESS;
    Int8 compare = (Int8)CamBrightness_Min;
    Int16 step = (CamBrightness_Max - CamBrightness_Min) / (MAX_BRIGHTNESS - MIN_BRIGHTNESS - 1);

    if( brightness > CamBrightness_Max )
        brightness = CamBrightness_Max;
    if( brightness < CamBrightness_Min )
        brightness = CamBrightness_Min;
    
    for (i = MIN_BRIGHTNESS; i < MAX_BRIGHTNESS; i++)
    {
        if (compare <= brightness)
        {
            tbl_offset = i;
        }
        compare += step;
    }
// --------Update Current setting
    if (result == HAL_CAM_SUCCESS)
    {
        ImageSettingsConfig_st.sensor_brightness->cur_setting = brightness;
    }
#endif
    pr_debug("CAMDRV_SetBrightness(): brightness = %d \r\n", brightness);

    return result;
}


//****************************************************************************
// Function Name:   HAL_CAM_Result_en_t CAMDRV_SetSaturation(Int8 saturation)
// Description:  for Preview
// Notes:       This function is NOT for average user's use.
//****************************************************************************
HAL_CAM_Result_en_t CAMDRV_SetSaturation(Int8 saturation, CamSensorSelect_t sensor)
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
    UInt8 saturation_tbl[MAX_SATURATION] = {   0,  25,  50,  75,  100, 126, 138, 150, 162, 175, 200};
    int i;
    UInt16 tbl_offset=NOMINAL_SATURATION;
    Int8 compare = (Int8)CamSaturation_Min;
    Int16 step = (CamSaturation_Max - CamSaturation_Min) / (MAX_SATURATION - MIN_SATURATION - 1);

    if( saturation > CamSaturation_Max )
        saturation = CamSaturation_Max;
    if( saturation < CamSaturation_Min )
        saturation = CamSaturation_Min;
    
    for (i = MIN_SATURATION; i < MAX_SATURATION; i++)
    {
        if (compare <= saturation)
        {
            tbl_offset = i;
        }
        compare += step;
    }

// --------Update Current setting
    if (result == HAL_CAM_SUCCESS)
    {
        stv_write( bColorEngine0_bColourSaturation, saturation_tbl[tbl_offset] );    
        stv_write( bColorEngine1_bColourSaturation, saturation_tbl[tbl_offset] );    
        ImageSettingsConfig_st.sensor_saturation->cur_setting = saturation;
    }
    pr_debug("CAMDRV_SetSaturation(): tbl_offset=%d saturation=%d \r\n", tbl_offset, saturation);
    return result;
}


//****************************************************************************
// Function Name:   HAL_CAM_Result_en_t CAMDRV_SetHuePreview(Int8 hue, CamSensorSelect_t nSensor)
// Description:  for Preview
// Notes:       This function is NOT for average user's use.
//****************************************************************************
HAL_CAM_Result_en_t CAMDRV_SetHue(Int8 hue, CamSensorSelect_t sensor)
{
    HAL_CAM_Result_en_t result = HAL_CAM_ERROR_ACTION_NOT_SUPPORTED;      // FIX ME!!!

#if 0   
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
    int hue_tbl[MAX_HUE] = {-127, -90, -60, -35, -15, 0, 15, 35, 60, 90, 128};
    int i;
    UInt16 tbl_offset=NOMINAL_HUE;
    Int8 compare = (Int8)CamHue_Min;
    Int16 step = (CamHue_Max - CamHue_Min) / (MAX_HUE - MIN_HUE - 1);

    if( hue > CamHue_Max )
        hue = CamHue_Max;
    if( hue < CamHue_Min )
        hue = CamHue_Min;

    for (i = MIN_HUE; i < MAX_HUE; i++)
    {
        if (compare <= hue)
        {
            tbl_offset = i;
        }
        compare += step;
    }

    if (result == HAL_CAM_SUCCESS)
    {
        ImageSettingsConfig_st.sensor_hue->cur_setting = hue;
    }
#endif
    pr_debug("CAMDRV_SetHue(): hue=%d \r\n", hue);
    return result;
}


//****************************************************************************
// Function Name:   HAL_CAM_Result_en_t CAMDRV_SetGamma(Int8 gama)
// Description:     This function sets the gamma value of camera sensor for Preview
// Notes:           Accepts values from [0, 255], although only 16 gamma settings
//                  are possible and range from 0.5 to 1.0
//                  This function is NOT for average user's use.
//****************************************************************************
HAL_CAM_Result_en_t CAMDRV_SetGamma(Int8 gamma, CamSensorSelect_t sensor)
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
//although only 16 gamma settings are possible, what are they?

    UInt8 gamma_tbl[MAX_GAMMA] = {0,  4,  8,  12,  16,  19,  20,  23,  26,  28,  31};
    int i;
    UInt16 tbl_offset=NOMINAL_GAMMA;
    Int8 compare = (Int8)CamGamma_Min;
    Int16 step = (CamGamma_Max - CamGamma_Min) / (MAX_GAMMA - MIN_GAMMA - 1);

    if( gamma > CamGamma_Max )
        gamma = CamGamma_Max;
    if( gamma < CamGamma_Min )
        gamma = CamGamma_Min;
    
    for (i = MIN_GAMMA; i < MAX_GAMMA; i++)
    {
        if (compare <= gamma)
        {
            tbl_offset = i;
        }
        compare += step;
    }

    if (result == HAL_CAM_SUCCESS)
    {
        stv_write( bColorEngine0_fGammaEnabled, 1 );                     
        stv_write( bColorEngine0_bMode        , 1 );                    
        stv_write( bColorEngine0_SharpRed     , gamma_tbl[tbl_offset] );    
        stv_write( bColorEngine0_SharpGreen   , gamma_tbl[tbl_offset] );    
        stv_write( bColorEngine0_SharpBlue    , gamma_tbl[tbl_offset] );    
        stv_write( bColorEngine0_SoftRed      , gamma_tbl[tbl_offset] );    
        stv_write( bColorEngine0_SoftGreen    , gamma_tbl[tbl_offset] );    
        stv_write( bColorEngine0_SoftBlue     , gamma_tbl[tbl_offset] );    
    
        stv_write( bColorEngine1_fGammaEnabled, 1 );                     
        stv_write( bColorEngine1_bMode        , 1 );                    
        stv_write( bColorEngine1_SharpRed     , gamma_tbl[tbl_offset] );    
        stv_write( bColorEngine1_SharpGreen   , gamma_tbl[tbl_offset] );    
        stv_write( bColorEngine1_SharpBlue    , gamma_tbl[tbl_offset] );    
        stv_write( bColorEngine1_SoftRed      , gamma_tbl[tbl_offset] );    
        stv_write( bColorEngine1_SoftGreen    , gamma_tbl[tbl_offset] );    
        stv_write( bColorEngine1_SoftBlue     , gamma_tbl[tbl_offset] );    
        ImageSettingsConfig_st.sensor_gamma->cur_setting = gamma;
    }

    stv_read(bSystemStatus);                                                        // read bSystemStatus for I2C write to complete
    pr_debug("CAMDRV_SetGamma(): tbl_offset=%d gamma=%d \r\n", tbl_offset, gamma);
    return result;
}


//****************************************************************************
//
// Function Name:   HAL_CAM_Result_en_t CAMDRV_LoadGammaTable(UInt8 *gtable)
//
// Description:     This function sets the gamma value of the MT9V143
//
// Notes:
//
//****************************************************************************
HAL_CAM_Result_en_t CAMDRV_LoadGammaTable(UInt8 *gtable, CamSensorSelect_t sensor)
{
    // fixme
    return HAL_CAM_ERROR_ACTION_NOT_SUPPORTED;
}


//****************************************************************************
// Function Name:   HAL_CAM_Result_en_t CAMDRV_SetSharpness(Int8 sharpness)
//
// Description: This function sets the sharpness for Preview
//
// Notes:    11 levels supported: MIN_SHARPNESS(0) for no sharpening,
//           and MAX_SHARPNESS(11) for 200% sharpening
//          This function is NOT for average user's use.
//****************************************************************************
HAL_CAM_Result_en_t CAMDRV_SetSharpness(Int8 sharpness, CamSensorSelect_t sensor)
{
    HAL_CAM_Result_en_t result = HAL_CAM_ERROR_ACTION_NOT_SUPPORTED;      // FIX ME!!!

#if 0   
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
    int sharpness_tbl[MAX_SHARPNESS] = {-127, -90, -60, -35, -15, 0, 15, 35, 60, 90, 128};
    int i;
    UInt16 tbl_offset=NOMINAL_SHARPNESS;
    Int8 compare = (Int8)CamSharpness_Min;
    Int16 step = (CamSharpness_Max - CamSharpness_Min) / (MAX_SHARPNESS - MIN_SHARPNESS - 1);

    if( sharpness > CamSharpness_Max )
        sharpness = CamSharpness_Max;
    if( sharpness < CamSharpness_Min )
        sharpness = CamSharpness_Min;
    
    result = HAL_CAM_ERROR_INTERNAL_ERROR;          // FIX ME!!!

    for (i = MIN_SHARPNESS; i < MAX_SHARPNESS; i++)
    {
        if (compare <= sharpness)
        {
            tbl_offset = i;
        }
        compare += step;
    }

    if (result == HAL_CAM_SUCCESS)
    {
        ImageSettingsConfig_st.sensor_sharpness->cur_setting = sharpness;
    }
#endif
    pr_debug("CAMDRV_SetSharpness(): sharpness=%d \r\n", sharpness);

    return result;
}

//****************************************************************************
// Function Name:   HAL_CAM_Result_en_t CAMDRV_SetAntiShadingPower(Int8 asp)
// Description: Set Anti Shading Power value for preview image.
// Notes:       This function is NOT for average user's use.
//****************************************************************************
HAL_CAM_Result_en_t CAMDRV_SetAntiShadingPower(Int8 asp, CamSensorSelect_t sensor)
{
    HAL_CAM_Result_en_t result = HAL_CAM_ERROR_ACTION_NOT_SUPPORTED;      // FIX ME!!!

#if 0   
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
    int asp_tbl[MAX_AntiShadingPower] = {-127, -90, -60, -35, -15, 0, 15, 35, 60, 90, 128};
    int i;
    UInt16 tbl_offset=NOMINAL_SHARPNESS;
    Int8 compare = (Int8)CamAntiShadingPower_Min;
    Int16 step = (CamAntiShadingPower_Max - CamAntiShadingPower_Min) / (MAX_AntiShadingPower - MIN_AntiShadingPower - 1);

    if( asp > CamAntiShadingPower_Max )
        asp = CamAntiShadingPower_Max;
    if( asp < CamAntiShadingPower_Min )
        asp = CamAntiShadingPower_Min;

    for (i = MIN_AntiShadingPower; i < MAX_AntiShadingPower; i++)
    {
        if (compare <= asp)
        {
            tbl_offset = i;
        }
        compare += step;
    }
    ImageSettingsConfig_st.sensor_antishadingpower->cur_setting = asp;
#endif
    pr_debug("CAMDRV_SetAntiShadingPower(): asp=%d \r\n", asp);

    return result;
}


//****************************************************************************
//
// Function Name:   HAL_CAM_Result_en_t CAMDRV_SetFrameRate(CamRates_t fps)
//
// Description: This function sets the frame rate of the Camera Sensor
//
// Notes:    15 or 30 fps are supported.
//
//****************************************************************************
HAL_CAM_Result_en_t CAMDRV_SetFrameRate(CamRates_t fps, CamSensorSelect_t sensor)
{
    HAL_CAM_Result_en_t ret_val = HAL_CAM_SUCCESS;
    
    if (fps > CamRate_30)
    {
        ret_val = HAL_CAM_ERROR_ACTION_NOT_SUPPORTED;
    }
    else
    {
        if ( (sensor == CamSensorSecondary) && (sSensorSecondary != SensorUnused) )
        {
            SecondaryFrameRate_st.cur_setting = fps;
            pr_debug("CAMDRV_SetFrameRate(): Secondary Sensor \r\n");
        }
        else
        {
            PrimaryFrameRate_st.cur_setting = fps;
            pr_debug("CAMDRV_SetFrameRate(): \r\n");
        }
        ret_val |= SensorSetFrameRate(fps, sensor);
    }
    pr_debug("CAMDRV_SetFrameRate(): return result =%d \r\n", ret_val);
    return ret_val;
}


//****************************************************************************
//
// Function Name:   HAL_CAM_Result_en_t CAMDRV_SetAutoExposure(CamExposure_t exposure)
//
// Description: This function enables or disables auto exposure
//
// Notes:
//
//****************************************************************************
HAL_CAM_Result_en_t CAMDRV_SetAutoExposure(CamExposure_t exposure, CamSensorSelect_t sensor)
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;

// --------Update Current setting
    if (result == HAL_CAM_SUCCESS)
    {
        ImageSettingsConfig_st.sensor_exposure->cur_setting = exposure;
    }
    result = HAL_CAM_ERROR_ACTION_NOT_SUPPORTED;           // FIX ME!!!
    pr_debug("CAMDRV_SetAutoExposure(): exposure = %d \r\n", exposure);
    return result;
}

/* * Set the required Metering Type (Image Quality Settings).  NOT for average user's use.

  @param type       [in] the required Metering Type.
  @return           HAL_CAM_Result_en_t 
*/
HAL_CAM_Result_en_t CAMDRV_SetMeteringType( CamMeteringType_t type, CamSensorSelect_t sensor   )
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;

// --------Update Current setting
    if (result == HAL_CAM_SUCCESS)
    {
        ImageSettingsConfig_st.sensor_metering->cur_setting = type;
    }
    pr_debug("CAMDRV_SetMeteringType(): type = %d \r\n", type);

//    result = HAL_CAM_ERROR_ACTION_NOT_SUPPORTED;           // FIX ME!!!
    return result;
}

/* * Set the required Sensitivity (Image Quality Settings).  NOT for average user's use.

  @param iso        [in] the required Sensitivity.
  @return           HAL_CAM_Result_en_t 
*/
HAL_CAM_Result_en_t CAMDRV_SetSensitivity( CamSensitivity_t iso, CamSensorSelect_t sensor)
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;

// --------Update Current setting
    if (result == HAL_CAM_SUCCESS)
    {
        ImageSettingsConfig_st.sensor_sensitivity->cur_setting = iso;
    }
    pr_debug("CAMDRV_SetSensitivity(): iso = %d \r\n", iso);
    return result;
}

/** Enable/Disable Wide Dynamic Range (Image Quality Settings).  NOT for average user's use.

    @param bWDR         [in] Wide Dynamic Range Enable/Disable.
    @param  sensor      [in] .
    @return             HAL_CAM_Result_en_t 
*/
HAL_CAM_Result_en_t CAMDRV_SetWideDynRangeEnable( UInt32 in_pWDR, CamSensorSelect_t sensor )
{
    HAL_CAM_Result_en_t result = HAL_CAM_ERROR_ACTION_NOT_SUPPORTED;      // FIX ME!!!
	Boolean* pWDR = (Boolean*) in_pWDR;

    ImageSettingsConfig_st.sensor_wdr->cur_setting = *pWDR;
    pr_debug("CAMDRV_SetWideDynRangeEnable(): bWDR = %d \r\n", *pWDR);
    return result;
}


        
//****************************************************************************
//
// Function Name:   HAL_CAM_Result_en_t CAMDRV_SetFlickerControl(CamFlicker_t control)
//
// Description: This function is currently unsupported
//
// Notes:
//
//****************************************************************************
HAL_CAM_Result_en_t CAMDRV_SetFlickerControl(CamFlicker_t control, CamSensorSelect_t sensor)
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;

    switch( control )
    {
        case CamFlicker50Hz:
            stv_write(0x2040, 0x01);     //ExposureControls bAntiFlickerMode {AntiFlickerMode_ManualEnable}
            stv_write(0x4700, 0x32);     //AntiFlickerExposureControls bMainsFrequency_Hz
            break;
        case CamFlicker60Hz:
            stv_write(0x2040, 0x01);     //ExposureControls bAntiFlickerMode {AntiFlickerMode_ManualEnable}
            stv_write(0x4700, 0x3C);     //AntiFlickerExposureControls bMainsFrequency_Hz
            break;
        case CamFlickerAuto:
            stv_write(0x2040, 0x00);     //ExposureControls bAntiFlickerMode {AntiFlickerMode_ManualEnable}
            break;
        default:
            result = HAL_CAM_ERROR_ACTION_NOT_SUPPORTED;           // FIX ME!!!
            break;
    }

// --------Update Current setting
    if (result == HAL_CAM_SUCCESS)
    {
        ImageSettingsConfig_st.sensor_flicker->cur_setting = control;
    }
    pr_debug("CAMDRV_SetFlickerControl(): control = %d \r\n", control);
    return result;
}

//****************************************************************************
//
// Function Name:   HAL_CAM_Result_en_t CAMDRV_SetZoom(UInt8 numer, UInt8 denum)
//
// Description: This function performs zooming via camera sensor
//
// Notes:
//
//****************************************************************************
HAL_CAM_Result_en_t CAMDRV_SetZoom(UInt32 step, CamSensorSelect_t nSensor)
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
    
#if 0    
    UInt8 zoom_status;
    UInt32 fovx;
    float *fovx_ptr;
    float zoom_fovx;
#ifdef CAMDBG_ZOOM
    UInt32 fovy;
    float *fovy_ptr;
#endif    

// --------Calculate new Zoomed fovx    
    zoom_fovx = ( (float)CAM_MAX_PIXEL_X * (float)step ) / (float)SDF_MAX_SCALE;                            // new zoomed fovx value

//---------Read current fovx
    fovx = (UInt32)(stv_read(fFOVX) << 24);
    fovx |= (UInt32)(stv_read(fFOVX+2) << 16);
    fovx_ptr = (float *)&fovx;                                                              // float convesion of fovx

    pr_debug("CAMDRV_SetZoom(): Before: zoom_fovx=%f  fovx=0x%x (float)fovx=%f \r\n", zoom_fovx, fovx, *fovx_ptr);

//--------- Enable AutoZoom
    stv_write(fAutoZoom, TRUE);                            // ZoomMgrCtrl_fAutoZoom=enabled
    stv_write(bMagFactor, 100);                            // ZoomMgrCtrl_bMagFactor = 100/1000

//--------- No Zoom if zoom_fovx = max fov
    if (zoom_fovx == (float)CAM_MAX_PIXEL_X)
    {
      stv_write(bZoomCmd, ZOOM_RESET);                     // ZoomMgrCtrl_bZoomCmd=zoom reset
      result = ZoomCommandWrite();
    }
//--------- Zoom in till reach new zoomed fovx
    else if (zoom_fovx < *fovx_ptr)
    {
        stv_write(bZoomCmd, ZOOM_IN);                          // ZoomMgrCtrl_bZoomCmd=zoom in
        result = ZoomCommandWrite();
        do
        {
            mdelay(5);    // Time in ms      
            fovx = (UInt32)(stv_read(fFOVX) << 24);
            fovx |= (UInt32)(stv_read(fFOVX+2) << 16);
            zoom_status = stv_read(bZoomOpStatus);
        } while ( (zoom_fovx < *fovx_ptr) && (zoom_status != FullyZoomedIn) );
    }
//--------- Zoom out till reach new zoomed fovx
    else if (zoom_fovx > *fovx_ptr)
    {
        stv_write(bZoomCmd, ZOOM_OUT);                         // ZoomMgrCtrl_bZoomCmd=zoom out
        result = ZoomCommandWrite();
        do
        {
            mdelay(5);    // Time in ms      
            fovx = (UInt32)(stv_read(fFOVX) << 24);
            fovx |= (UInt32)(stv_read(fFOVX+2) << 16);
            zoom_status = stv_read(bZoomOpStatus);
        } while ( (zoom_fovx > *fovx_ptr) && (zoom_status != FullyZoomedOut) );
    }
    stv_write(bZoomCmd, NO_ZOOM_COMMAND);                    // ZoomMgrCtrl_bZoomCmd=zoom stop
    result = ZoomCommandWrite();
    zoom_status = stv_read(bZoomOpStatus);

#ifdef CAMDBG_ZOOM
    fovx = (UInt32)(stv_read(fFOVX) << 24);
    fovx |= (UInt32)(stv_read(fFOVX+2) << 16);

    fovy = (UInt32)(stv_read(fFOVY) << 24);
    fovy |= (UInt32)(stv_read(fFOVY+2) << 16);
    fovy_ptr = (float *)&fovy;                                                              // float convesion of fovy
    pr_debug("CAMDRV_SetZoom(): After: fovx=0x%x (float)fovx=%f fovy=0x%x (float)fovy=%f zoom_status=%d \r\n", fovx, *fovx_ptr, fovy, *fovy_ptr, zoom_status);
#endif
#endif
    pr_debug("CAMDRV_SetZoom(): step = %d result=%d \r\n", step, result);
    return result;
}

//******************************************************************************
//
// Function Name:   CAMDRV_PhyH/PhyW
//
// Description: Return CAM physical window size 
//
// Notes:
//
//******************************************************************************
UInt32 CAMDRV_PhyH(CamSensorSelect_t nSensor)
{
    if ( nSensor == CamSensorSecondary )
    {
        return CamSecondaryCfgCap_st.output_st.max_height;
    }
    else
    {
        return CamPrimaryCfgCap_st.output_st.max_height;
    }
}

UInt32 CAMDRV_PhyW(CamSensorSelect_t nSensor)
{
    if ( nSensor == CamSensorSecondary )
    {
        return CamSecondaryCfgCap_st.output_st.max_width;
    }
    else
    {
        return CamPrimaryCfgCap_st.output_st.max_width;
    }
}

//****************************************************************************
//
// Function Name:   HAL_CAM_Result_en_t CAMDRV_SetRotationMode(CamRotate_t mode, CamSensorSelect_t sensor)
//
// Description: This function sets sensor rotation mode
//
// Notes:
//
//****************************************************************************
HAL_CAM_Result_en_t CAMDRV_SetRotationMode(CamRotate_t mode, CamSensorSelect_t sensor)
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
    UInt8 image_read_mode;

    if (mode >= CamRotate_N_Modes)
    {
        result = HAL_CAM_ERROR_INTERNAL_ERROR;
        pr_debug("CAMDRV_SetRotationMode(): ERROR:  Mode not Supported \r\n");
    }
    else
    {
         switch (mode)
        {
            case CamRotate0:
                image_read_mode = IMAGE_NORMAL_MODE;                // <184> normal mode is transferred
                break;
            case CamRotate90:
                image_read_mode = IMAGE_ROTATE_90;                  // <185> Image is rotated anti-clockwise by 90 degrees
                break;
            case CamRotate180:
                image_read_mode = IMAGE_ROTATE_180;                 // <186> Image is rotated by 180 degrees
                break;
            case CamRotate270:
                image_read_mode = IMAGE_ROTATE_270;                 // <187> Image is rotated anti-clockwise by 270 degrees
                break;
            default:
                result = HAL_CAM_ERROR_INTERNAL_ERROR;
                image_read_mode = IMAGE_NORMAL_MODE;                // <184> normal mode is transferred
                mode = CamRotate0;
                pr_debug("CAMDRV_SetRotationMode(): ERROR:  Mode not Supported \r\n");
                break;
        }   
        stv_write(bImageReadMode, image_read_mode);     // SystemImageCharacteristicsControlParameters_bImageReadMode
        stv_read(bSystemStatus);                                // read bSystemStatus for I2C write to complete
        ImageSettingsConfig_st.sensor_rotatemode->cur_setting = mode;
    }       // else:  if (mode >= CamDispMod_N_Modes)
    pr_debug("CAMDRV_SetRotationMode(): mode = %d \r\n", mode);
    return result;
}

//****************************************************************************
//
// Function Name:   CamRotate_t CAMDRV_GetRotationMode(CamSensorSelect_t sensor)
//
// Description: This function sets sensor rotation mode
//
// Notes:
//
//****************************************************************************
CamRotate_t CAMDRV_GetRotationMode(CamSensorSelect_t sensor)
{
    pr_debug("CAMDRV_GetRotationMode(): \r\n");
    return ImageSettingsConfig_st.sensor_rotatemode->cur_setting;
}

//****************************************************************************
//
// Function Name:   HAL_CAM_Result_en_t CAMDRV_SetMirrorMode(CamMirror_t mode, CamSensorSelect_t sensor)
//
// Description: This function sets sensor mirror mode
//
// Notes:
//
//****************************************************************************
HAL_CAM_Result_en_t CAMDRV_SetMirrorMode(CamMirror_t mode, CamSensorSelect_t sensor)
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
    UInt8 sensor_data;

    //if ( (ImageSettingsConfig_st.sensor_mirrormode.settings & mode) == 0 )
    if (mode >= CamMirror_N_Modes)
    {
        result = HAL_CAM_ERROR_INTERNAL_ERROR;
        pr_debug("CAMDRV_SetMirrorMode(): ERROR:  Mode not Supported \r\n");
    }
    else if ( sCamSensorSel == SensorVS6555 )
    {       
        result = HAL_CAM_ERROR_ACTION_NOT_SUPPORTED;
        pr_debug("CAMDRV_SetMirrorMode(): ERROR:  Sensor does not Support \r\n");
    }
    else
    {
        sensor_data = bSmiaMirrorNormal;
        switch (mode)
        {
            case CamMirrorNone:
                break;
            case CamMirrorVertical:
                sensor_data |= bSmiaVerticalMirrorEnable;
                break;
            case CamMirrorHorizontal:
                sensor_data |= bSmiaHorizontalMirrorEnable;
                break;
            case CamMirrorBoth:
                sensor_data |= bSmiaVerticalMirrorEnable;
                sensor_data |= bSmiaHorizontalMirrorEnable;
                break;
            default:
                result = HAL_CAM_ERROR_INTERNAL_ERROR;
                mode = CamMirrorNone;
                pr_debug("CAMDRV_SetMirrorMode(): ERROR:  Mode not Supported \r\n");
        }   
        result |= SensorWrite(1, bSmiaImageOrientationReg, &sensor_data);     // image_orientation
        ImageSettingsConfig_st.sensor_mirrormode->cur_setting = mode;
    }
    pr_debug("CAMDRV_SetMirrorMode(): mode = %d return res= %d\r\n", mode, result);
    return result;
}

//****************************************************************************
//
// Function Name:   HAL_CAM_Result_en_t CAMDRV_GetMirrorMode(CamSensorSelect_t sensor)
//
// Description: This function sets sensor rotation mode
//
// Notes:
//
//****************************************************************************
CamMirror_t CAMDRV_GetMirrorMode(CamSensorSelect_t sensor)
{
    pr_debug("CAMDRV_GetMirrorMode(): \r\n");
    return ImageSettingsConfig_st.sensor_mirrormode->cur_setting;
}

//****************************************************************************
//
// Function Name:   void CAMDRV_SetFlashMode(flash)(FlashLedState_t flash, CamSensorSelect_t sensor)
//
// Description: This function sets the flash mode: FLASH or TORCH
//
// Notes:   TORCH is less brighter than FLASH.
//          In the dark, torch is used for preview and flash for capture.
//
//****************************************************************************
HAL_CAM_Result_en_t CAMDRV_SetFlashMode(FlashLedState_t flash, CamSensorSelect_t sensor)
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;

// --------Update Current setting
    switch( flash )
    {
        case Flash_On:          
        case FlashLight_Auto:
            stv0986_GpioSetFlashModeTorchOff();
            stv0986_sys_flash_state = flash;
            break;
        case Torch_On:          
            stv0986_GpioSetTorchModeTorchOn();
            stv0986_sys_flash_state = flash;
            break;
        case Flash_Off:         
            stv0986_GpioSetFlashModeTorchOff();
            stv0986_sys_flash_state = flash;
            break;
        default:
            result = HAL_CAM_ERROR_INTERNAL_ERROR;
            break;
    }
    if (result == HAL_CAM_SUCCESS)
    {
        ImageSettingsConfig_st.sensor_flashstate->cur_setting = flash;
    }
    pr_debug("CAMDRV_SetFlashMode(): flash = %d \r\n", flash);
    return result;
}

//****************************************************************************
// Function Name:   HAL_CAM_Result_en_t CAMDRV_SetAntiBanding(CamAntiBanding_t effect)
//
// Description: This function will set the antibanding effect of camera
// Notes:
//****************************************************************************

HAL_CAM_Result_en_t CAMDRV_SetAntiBanding(CamAntiBanding_t effect, CamSensorSelect_t sensor )
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
	pr_info("CAMDRV_SetAntiBanding()  called\n");
	if(effect == CamAntiBandingAuto){
		//do sth		
	}else if(effect == CamAntiBanding50Hz) {
		//do sth						
	}else if(effect == CamAntiBanding60Hz) {
		//do sth		
	}else if(effect == CamAntiBandingOff) {
		//do sth
	}else {
		//do sth
	}

	if (sCamI2cStatus != HAL_CAM_SUCCESS)
    {
          pr_debug("CAMDRV_SetAntiBanding(): Error[%d] \r\n", sCamI2cStatus);
          result = sCamI2cStatus;
    }
	
    return result;
}

//****************************************************************************
// Function Name:   HAL_CAM_Result_en_t CAMDRV_SetFocusMode( CamFocusControlMode_t effect)
//
// Description: This function will set the focus mode of camera
// Notes:
//****************************************************************************

HAL_CAM_Result_en_t CAMDRV_SetFocusMode(CamFocusControlMode_t effect,CamSensorSelect_t sensor)
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
	pr_info("CAMDRV_SetFocusMode()  called\n");
	if(effect == CamFocusControlAuto){
		//do sth
	}else if(effect == CamFocusControlMacro) {
		//do sth
	}else if(effect == CamFocusControlInfinity) {
		//do sth
	}else {
		//do sth
		
	}
	if (sCamI2cStatus != HAL_CAM_SUCCESS)
    {
          pr_debug("CAMDRV_SetFocusMode(): Error[%d] \r\n", sCamI2cStatus);
          result = sCamI2cStatus;
    }
    return result;
}

//****************************************************************************
// Function Name:   HAL_CAM_Result_en_t CAMDRV_SetJpegQuality( CamFocusStatus_t effect)
//
// Description: This function will set the focus mode of camera
// Notes:
//****************************************************************************

HAL_CAM_Result_en_t CAMDRV_SetJpegQuality(CamJpegQuality_t effect,CamSensorSelect_t sensor)
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
	pr_info("CAMDRV_SetJpegQuality()  called\n");
	if(effect == CamJpegQuality_Min){
		//do sth
	}else if(effect == CamJpegQuality_Nom) {
		//do sth
	}else {
		//do sth
	}
	if (sCamI2cStatus != HAL_CAM_SUCCESS)
    {
          pr_debug("CAMDRV_SetJpegQuality(): Error[%d] \r\n", sCamI2cStatus);
          result = sCamI2cStatus;
    }
    return result;
}

/** Set Frame Stabilization Enable camera Sensor
    @param  pStab               [in] Data address of Enable/Disable Frame Stabilization
    @param  sensor              [in] Sensor Select.
    @return       HAL_CAM_Result_en_t 
    @note   
*/
HAL_CAM_Result_en_t CAMDRV_SetFrameStabEnable( UInt32 in_pStab, CamSensorSelect_t sensor )
{
    HAL_CAM_Result_en_t result = HAL_CAM_ERROR_ACTION_NOT_SUPPORTED;      // FIX ME!!!
	Boolean* pStab = (Boolean*) in_pStab;

    ImageSettingsConfig_st.sensor_framestab->cur_setting = *pStab;
    pr_debug("CAMDRV_SetFrameStabEnable(): bStab = %d \r\n", *pStab);
    return result;
}

/** Set AntiShaking Enable on camera Sensor
    @param  in_pShake           [in] Data address of Enable/Disable Anti-Shake
    @param  sensor              [in] Sensor Select.
    @return       HAL_CAM_Result_en_t 
    @note   
*/
HAL_CAM_Result_en_t CAMDRV_SetAntiShakeEnable( UInt32 in_pShake, CamSensorSelect_t sensor )
{
    HAL_CAM_Result_en_t result = HAL_CAM_ERROR_ACTION_NOT_SUPPORTED;      // FIX ME!!!
	Boolean* pShake = (Boolean*) in_pShake; 

    ImageSettingsConfig_st.sensor_antishake->cur_setting = *pShake;
    pr_debug("CAMDRV_SetAntiShakeEnable(): bShake = %d \r\n", *pShake);
    return result;
}

/** Set Face Detection Enable on camera Sensor
    @param  bDetect             [in] Data address of Enable/Disable Face Detection
    @param  sensor              [in] Sensor Select.
    @return       HAL_CAM_Result_en_t 
    @note   
*/
HAL_CAM_Result_en_t CAMDRV_SetFaceDetectEnable( UInt32 in_pDetect, CamSensorSelect_t sensor )
{
    HAL_CAM_Result_en_t result = HAL_CAM_ERROR_ACTION_NOT_SUPPORTED;      // FIX ME!!!
	Boolean* pDetect = (Boolean*) in_pDetect; 

    ImageSettingsConfig_st.sensor_facedetect->cur_setting = *pDetect;
    pr_debug("CAMDRV_SetFaceDetectEnable(): bDetect = %d \r\n", *pDetect);
    return result;
}

/** Get Face Detection Rectangle from camera Sensor
    @param  face_select         [in] Face Detection Select
    @param  *rect_st            [in] Rectangle structure pointer.
    @param  sensor              [in] Sensor Select.
    @return       HAL_CAM_Result_en_t 
    @note   
*/
HAL_CAM_Result_en_t CAMDRV_GetFaceDetectRect(UInt32 p_rect_st, CamSensorSelect_t sensor)
{
    HAL_CAM_Result_en_t result = HAL_CAM_ERROR_ACTION_NOT_SUPPORTED;      // FIX ME!!!
	HAL_CAM_Rectangle_st_t* rect_st = (HAL_CAM_Rectangle_st_t* ) p_rect_st;
	UInt8	face_select = rect_st->face_select;
    
    rect_st->start_x = 0;
    rect_st->start_y = 0;
    rect_st->end_x = CamPrimaryCfgCap_st.output_st.max_width;
    rect_st->end_y = CamPrimaryCfgCap_st.output_st.max_height;

    if (sensor == CamSensorSecondary)
    {
        rect_st->end_x = CamSecondaryCfgCap_st.output_st.max_width;
        rect_st->end_y = CamSecondaryCfgCap_st.output_st.max_height;
    }
    pr_debug("CAMDRV_GetFaceDetectRect(): face_select=%d start_x=%d end_x=%d start_y=%d end_y=%d \r\n", 
                    face_select,rect_st->start_x,rect_st->end_x,rect_st->start_y,rect_st->end_y);
    return result;
}

/** Set AutoFocus Enable on camera Sensor
    @param  bAutoFocus          [in] Enable/Disable auto focus
    @param  sensor              [in] Sensor Select.
    @return       HAL_CAM_Result_en_t 
    @note   
*/
HAL_CAM_Result_en_t CAMDRV_SetAutoFocusEnable( UInt32 in_pAutoFocus, CamSensorSelect_t sensor )
{
    HAL_CAM_Result_en_t result = HAL_CAM_ERROR_ACTION_NOT_SUPPORTED;      // FIX ME!!!
	Boolean* pAutoFocus = (Boolean*)in_pAutoFocus;

    ImageSettingsConfig_st.sensor_autofocus->cur_setting = *pAutoFocus;
    pr_debug("CAMDRV_SetAutoFocusEnable(): bAutoFocus = %d \r\n", *pAutoFocus);
    return result;
} 
    
    

//****************************************************************************
//
// Function Name:   HAL_CAM_Result_en_t CAMDRV_SetExpSetting(CamExposureSetting_st_t* exposure_setting, CamSensorSelect_t sensor)
//
// Description: Set exposure setting for camera Sensor
//
// Notes:   This function is NOT for average user's use.
//
//****************************************************************************
HAL_CAM_Result_en_t CAMDRV_SetExpSetting(HAL_CAM_ExposureSetting_st_t* exposure_setting, CamSensorSelect_t sensor)
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;

    result |= CAMDRV_SetMeteringType( exposure_setting->exp_meter, sensor );
    result |= CAMDRV_SetSensitivity( exposure_setting->exp_iso, sensor);
    pr_debug("CAMDRV_SetExpSetting(): exp_meter=%d exp_iso=%d \r\n", exposure_setting->exp_meter,exposure_setting->exp_iso);
    pr_debug("CAMDRV_SetExpSetting(): return res = %d \r\n", result);
    return result;
}


/*  Get Focus Status for camera Sensor
        @param  *pfocus_status [out] focus status
        @param  *pfocus_region [out] focus region setting
        @param  sensor          [in] camera sensor select
        @note   none
*/
HAL_CAM_Result_en_t CAMDRV_GetFocusStatus(
    CamFocusStatus_t*           pfocus_status,
    HAL_CAM_Focus_Region_st_t*  pfocus_region,
    CamSensorSelect_t           sensor
    )
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
    UInt8 af_stats_error;
    Boolean lensMoving;

    if (sCamFocusControl_st.focus_ctrl_mode == CamFocusControlOff)
    {
        *pfocus_status = CamFocusStatusOff;
    }
    else
    {
        lensMoving = stv_read(fLensIsMoving);           // Check if FLADriverStatus_fLensIsMoving
        if (lensMoving)
        {
            *pfocus_status = CamFocusStatusRequest;     // status = lens moving
        }
        else
        {
            af_stats_error = stv_read(bAFStatsError);
            if (af_stats_error != AF_ERROR_OK)
            {
                *pfocus_status = CamFocusStatusUnableToReach;   // status = focus failed
            }
            else
            {
                af_stats_error = stv_read(fIsStable);
                if (af_stats_error)
                {
                    *pfocus_status = CamFocusStatusReached;     // status = focus reached
                }
                else
                {
                    *pfocus_status = CamFocusStatusLost;        // status = focus lost
                }                    
            }
        }
    }        
    pfocus_region->focus_center_enabled = stv_read(bWeight_0);    
    pfocus_region->focus_left_enabled = stv_read(bWeight_1);    
    pfocus_region->focus_right_enabled = stv_read(bWeight_2);    
    pfocus_region->focus_top_enabled = stv_read(bWeight_3);    
    pfocus_region->focus_bottom_enabled = stv_read(bWeight_4);    
    pfocus_region->focus_topleft_enabled = stv_read(bWeight_5);    
    pfocus_region->focus_topright_enabled = stv_read(bWeight_6);    
    pfocus_region->focus_bottomleft_enabled = stv_read(bWeight_7);    
    pfocus_region->focus_bottomright_enabled = stv_read(bWeight_8);    
    pr_debug("CAMDRV_GetFocusStatus(): status = %d \r\n", *pfocus_status);
    return result;
}   

/* * Set Focus Status for camera Sensor
    @param  *pfocus_region [in] focus region setting 
    @param  sensor         [in] camera sensor select
    @note   none
*/
HAL_CAM_Result_en_t CAMDRV_SetFocusRegion(
    HAL_CAM_Focus_Region_st_t*  pfocus_region,
    CamSensorSelect_t           sensor
    )
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;

    stv_write(bWeight_0, pfocus_region->focus_center_enabled);    
    stv_write(bWeight_1, pfocus_region->focus_left_enabled);    
    stv_write(bWeight_2, pfocus_region->focus_right_enabled);    
    stv_write(bWeight_3, pfocus_region->focus_top_enabled);    
    stv_write(bWeight_4, pfocus_region->focus_bottom_enabled);    
    stv_write(bWeight_5, pfocus_region->focus_topleft_enabled);    
    stv_write(bWeight_6, pfocus_region->focus_topright_enabled);    
    stv_write(bWeight_7, pfocus_region->focus_bottomleft_enabled);    
    stv_write(bWeight_8, pfocus_region->focus_bottomright_enabled);    
    pr_debug("CAMDRV_SetFocusRegion(): \r\n");
    return result;
}   

/* * Get Focus Control Configuration for camera Sensor
    @param  *pfocus_control [out] focus control configuration
    @param  sensor [in] camera sensor select
    @note   none
*/
HAL_CAM_Result_en_t CAMDRV_GetFocusControlConfig(
    HAL_CAM_Focus_Control_Config_st_t*  pfocus_control,
    CamSensorSelect_t                   sensor
    )
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
    UInt16 lens_max, lens_min;
    UInt16 focus_mode;

// bModeStatus 
    focus_mode = stv_read(bFocusModeStatus);                // FocusStatus_bModeStatus
    switch (focus_mode)
    {
        case FC_TLI_MODE_MANUAL_FOCUS:
            sCamFocusControl_st.focus_ctrl_mode = CamFocusControlOn; 
            break;
        case FC_TLI_MODE_AF_CONTINUOUS_FOCUS:
            sCamFocusControl_st.focus_ctrl_mode = CamFocusControlAuto; 
            break;
        case FC_TLI_MODE_AF_SINGLE_FOCUS_FS:
            sCamFocusControl_st.focus_ctrl_mode = CamFocusControlAutoLock; 
            break;
        case FC_TLI_MODE_AF_SINGLE_FOCUS_HCS:
            sCamFocusControl_st.focus_ctrl_mode = CamFocusControlQuickSearch; 
            break;
        default:
            sCamFocusControl_st.focus_ctrl_mode = CamFocusControlOff; 
            break; 
    }   
    
// wLensMaxPosition 
    lens_max = stv_read(0x5505) << 8;                       // FocusRangeConstants_wFullRange_LensMaxPosition_MSByte
    lens_max |= stv_read(0x5506);                           // FocusRangeConstants_wFullRange_LensMaxPosition_LSByte
// wLensMinPosition 
    lens_min = stv_read(0x5501) << 8;                       // FocusRangeConstants_wFullRange_LensMaxPosition_MSByte
    lens_min |= stv_read(0x5502);                           // FocusRangeConstants_wFullRange_LensMaxPosition_LSByte
    sCamFocusControl_st.focus_ctrl_num_steps = lens_max - lens_min;
    sCamFocusControl_st.focus_current_step = stv_read(wLensPosition);
    
    pfocus_control->focus_ctrl_mode = sCamFocusControl_st.focus_ctrl_mode;
    pfocus_control->focus_ctrl_num_steps = sCamFocusControl_st.focus_ctrl_num_steps;
    pfocus_control->focus_current_step = sCamFocusControl_st.focus_current_step;
    pr_debug("CAMDRV_GetFocusControlConfig(): mode = %d \r\n", pfocus_control->focus_ctrl_mode);
    return result;
}   


/* * Set Focus Control Configuration for camera Sensor
    @param  *pfocus_control [out] focus control configuration
    @param  sensor          [in] camera sensor select
    @note   none
*/
HAL_CAM_Result_en_t CAMDRV_SetFocusControlConfig(
    HAL_CAM_Focus_Control_Config_st_t*  pfocus_control,
    CamSensorSelect_t                   sensor
    )
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
    UInt32 i;

//bModeStatus 
    switch (pfocus_control->focus_ctrl_mode)
    {
        case CamFocusControlOn:
            pr_debug("CAMDRV_SetFocusControlConfig(): CamFocusControlOn: Manual Focus \r\n");
        // set full range
            stv_write(bRangeFocus, 0x0); // FocusControls_bRange 
            result |= SetManualFocusCmd((UInt8)LA_CMD_GOTO_INFINITY, (UInt8)FC_TLI_MODE_MANUAL_FOCUS, TRUE);
            for ( i = 0; ( (i < pfocus_control->focus_current_step) && (result == HAL_CAM_SUCCESS) ) ; i++)
            {
                result |= SetManualFocusCmd((UInt8)LA_CMD_MOVE_STEP_TO_MACRO, (UInt8)FC_TLI_MODE_MANUAL_FOCUS, TRUE);
            }
            sCamFocusControl_st.focus_ctrl_mode = CamFocusControlOn;
            sCamFocusControl_st.focus_current_step = i;
            sCamFocusControl_st.focus_ctrl_num_steps = i;
            break;
        case CamFocusControlAuto :
            pr_debug("CAMDRV_SetFocusControlConfig(): CamFocusControlOn: Continuous Focus \r\n");
            result |= SetAutoFocusCmd( (UInt8)AF_TLI_CMD_NULL, (UInt8)FC_TLI_MODE_AF_CONTINUOUS_FOCUS, FALSE );
            sCamFocusControl_st.focus_ctrl_mode = CamFocusControlAuto;
            break;
        case CamFocusControlAutoLock:
            pr_debug("CAMDRV_SetFocusControlConfig(): CamFocusControlOn: Continuous Focus \r\n");
            result |= SetAutoFocusCmd( (UInt8)AF_TLI_CMD_TAKE_SNAPSHOT, (UInt8)FC_TLI_MODE_AF_SINGLE_FOCUS_FS, TRUE );
            sCamFocusControl_st.focus_ctrl_mode = CamFocusControlAutoLock;
            break;
        case CamFocusControlQuickSearch:
            pr_debug("CAMDRV_SetFocusControlConfig(): CamFocusControlOn: Single Fast Focus \r\n");
            result |= SetAutoFocusCmd( (UInt8)AF_TLI_CMD_REFOCUS, (UInt8)FC_TLI_MODE_AF_SINGLE_FOCUS_HCS, TRUE );
            sCamFocusControl_st.focus_ctrl_mode = CamFocusControlQuickSearch;
            break;
        default:        //CamFocusControlOff;
        // Park lens
            pr_debug("CAMDRV_SetFocusControlConfig(): CamFocusControlOff: Park lens \r\n");
            result |= SetManualFocusCmd((UInt8)LA_CMD_PARK, (UInt8)FC_TLI_MODE_MANUAL_FOCUS, FALSE);
            sCamFocusControl_st.focus_ctrl_mode = CamFocusControlOff;
            break; 
    }                     
    pr_debug("CAMDRV_SetFocusControlConfig(): mode = %d \r\n", pfocus_control->focus_ctrl_mode);
    pr_debug("CAMDRV_SetFocusControlConfig(): return res = %d \r\n", result);
    return result;
}   

//******************************************************************************
//
// Function Name:   CAMDRV_GetInitPwrUpSeq
//
// Description: Return Camera Sensor Init Power Up sequence
//
// Notes:
//
//******************************************************************************
CamSensorIntfCntrl_st_t* CAMDRV_GetIntfSeqSel(
    CamSensorSelect_t nSensor,
    CamSensorSeqSel_t nSeqSel, 
    UInt32 *pLength)
{
    
// ---------Default to no Sequence  
    CamSensorIntfCntrl_st_t *power_seq = NULL;
    *pLength = 0;
    pr_debug("CAMDRV_GetIntfSeqSel(): nSeqSel = %d nSensor=%d \r\n", nSeqSel, nSensor);
    switch (nSeqSel)
    {
        case SensorInitPwrUp:   // Camera Init Power Up (Unused)
        case SensorPwrUp:   // Torch flash on
            if ( (nSensor == CamSensorPrimary) || (nSensor == CamSensorSecondary) )
            {
                *pLength = sizeof(CamPowerOnSeq);
                power_seq = CamPowerOnSeq;
            }
            break;

        case SensorInitPwrDn:   // Camera Init Power Down (Unused)
        case SensorPwrDn:   // Both off
            if ( (nSensor == CamSensorPrimary) || (nSensor == CamSensorSecondary) )
            {
                *pLength = sizeof(CamPowerOffSeq);
                power_seq = CamPowerOffSeq;
            }
            break;

        case SensorFlashEnable: // Flash Enable
            if ( (nSensor == CamSensorPrimary) || (nSensor == CamSensorSecondary) )
            {
                *pLength = sizeof(CamFlashEnable);
                power_seq = CamFlashEnable;
            }
            break;

        case SensorFlashDisable:    // Flash Disable
            if ( (nSensor == CamSensorPrimary) || (nSensor == CamSensorSecondary) )
            {
                *pLength = sizeof(CamFlashDisable);
                power_seq = CamFlashDisable;
            }
            break;

        default:
            break;
    }
    return power_seq;
}

//******************************************************************************
//
// Function Name:   CAMDRV_GetIntfConfigTbl
//
// Description: Return Camera Sensor Interface Configuration
//
// Notes:
//
//******************************************************************************
CamIntfConfig_st_t* CAMDRV_GetIntfConfig(
    CamSensorSelect_t nSensor)
{
    
// ---------Default to no configuration Table
    CamIntfConfig_st_t *config_tbl = NULL;
    pr_debug("CAMDRV_GetIntfConfig(): nSensor = %d \r\n", nSensor);
    switch (nSensor)
    {
        case CamSensorPrimary:                                  // Primary Sensor Configuration
        default:
            CamSensorCfg_st.sensor_config_caps = &CamPrimaryCfgCap_st;
            break;
        case CamSensorSecondary:                                // Secondary Sensor Configuration
            CamSensorCfg_st.sensor_config_caps = &CamSecondaryCfgCap_st;
            break;
    }
    config_tbl = &CamSensorCfg_st;
    return config_tbl;
}

//******************************************************************************
//
// Function Name:   CAMDRV_GetImageSettings
//
// Description: Return Camera Sensor Image Settings
//
// Notes:
//
//******************************************************************************
CamSensorImageConfig_st_t* CAMDRV_GetImageSettings(
        CamSensorSelect_t sensor, 
        CamCaptureMode_t mode)
{
    
// ---------Default to no configuration Table
    CamSensorImageConfig_st_t *image_settings = NULL;
    pr_debug("CAMDRV_GetImageSettings(): nSensor = %d \r\n", sensor);
    switch (sensor)
    {
        case CamSensorPrimary:                                  // Primary Sensor Image Settings
        default:
            ImageSettingsConfig_st.sensor_flashstate = &PrimaryFlashState_st;
            ImageSettingsConfig_st.sensor_framerate = &PrimaryFrameRate_st;
            ImageSettingsConfig_st.sensor_mirrormode = &PrimaryMirrorMode_st;
            break;
        case CamSensorSecondary:                                // Secondary Sensor Image Settings
            ImageSettingsConfig_st.sensor_flashstate = &SecondaryFlashState_st;
            ImageSettingsConfig_st.sensor_framerate = &SecondaryFrameRate_st;
            ImageSettingsConfig_st.sensor_mirrormode = &SecondaryMirrorMode_st;
            break;
    }
    image_settings = &ImageSettingsConfig_st;
    return image_settings;
}


//***************************************************************************
/* *
*       CAMDRV_Supp_Init performs additional device specific initialization
*
*   @return  HAL_CAM_Result_en_t
*
*       Notes:
*/
HAL_CAM_Result_en_t CAMDRV_Supp_Init(CamSensorSelect_t sensor)
{
    HAL_CAM_Result_en_t ret_val = HAL_CAM_SUCCESS;

    return ret_val;
} // CAMDRV_Supp_Init()

/** @} */





//***************************************************************************
//***************************************************************************
/**
*       DEBUG FUNCTIONS FOR STV0986 ONLY!!!!
*
*   @return  HAL_CAM_Result_en_t
*
*       Notes:
*/
HAL_CAM_Result_en_t CAMDRV_SetSensorMaxDataRate(UInt16 rate)
{
    pr_debug("CAMDRV_SetSensorMaxDataRate(): rate = %d \r\n", rate);
    sSensor0MaxDataRate = rate;
    sSensor1MaxDataRate = rate;
    return HAL_CAM_SUCCESS;
}
HAL_CAM_Result_en_t CAMDRV_SetItuClkDiv(UInt16 clk_div)
{
    pr_debug("CAMDRV_SetItuClkDiv(): clk_div = %d \r\n", clk_div);
    sCamItuClkDiv = clk_div;
    return HAL_CAM_SUCCESS;
}
HAL_CAM_Result_en_t CAMDRV_SetViewFinderFormat(UInt16 format)
{
    pr_debug("CAMDRV_SetViewFinderFormat(): format = %d \r\n", format);
    sViewFinderFormat = (CamDataFmt_t)format;
    return HAL_CAM_SUCCESS;
}
HAL_CAM_Result_en_t CAMDRV_SetViewFinderResolution(UInt16 resolution)
{
    pr_debug("CAMDRV_SetViewFinderResolution(): resolution = %d \r\n", resolution);
    sViewFinderResolution = (CamImageSize_t)resolution;
    return HAL_CAM_SUCCESS;
}
HAL_CAM_Result_en_t CAMDRV_SetCaptureImageFormat(UInt16 format)
{
    pr_debug("CAMDRV_SetCaptureImageFormat(): format = %d \r\n", format);
    sCaptureImageFormat = (CamDataFmt_t)format;
    return HAL_CAM_SUCCESS;
}
HAL_CAM_Result_en_t CAMDRV_SetCaptureImageResolution(UInt16 resolution)
{
    pr_debug("CAMDRV_SetCaptureImageResolution(): resolution = %d \r\n", resolution);
    sCaptureImageResolution = (CamImageSize_t)resolution;
    return HAL_CAM_SUCCESS;
}

/*
HAL_CAM_Result_en_t CAMDRV_GetCamStatus()
{
    stv_Read_Status();
    return HAL_CAM_SUCCESS;
}

HAL_CAM_Result_en_t CAMDRV_SensorStatus()
{
    stv_Read_Sensor_Status();
    return HAL_CAM_SUCCESS;
}
*/

UInt32 CAMDRV_GetJpegSize(CamSensorSelect_t sensor, void *data)
{
    UInt32 size;
    size = stv_Read_JpegSize();
    return size;
}

UInt16 *CAMDRV_GetJpeg(short *buf)
{
	return buf;
}


UInt8 * CAMDRV_GetThumbnail(void *buf, UInt32 offset)
{
    int length = (offset + 1) >> 1; // length in u16
    u8 *thumbnail = (char *)buf + (length << 1) + 2;
    return thumbnail;
}

UInt16 CAMDRV_GetDeviceID(CamSensorSelect_t sensor)
{
	u16 DevID;

	stv_write(0xA080, 0);
	ssleep(1);

	CAM_ReadI2c(0x5E01, 2, (u8 *)&DevID);
	return DevID;
}

/*
HAL_CAM_Result_en_t CAMDRV_AutoFocus()
{
    HAL_CAM_Result_en_t result;
    result = AF_QuickSearch();
    return result;
}
    
// ===========================================
// 
//  Sensor Streaming mode
// 
// ===========================================
HAL_CAM_Result_en_t CAMDRV_SensorStreamMode()
{
    HAL_CAM_Result_en_t result;
    result = stv_SensorStreamMode();
    return result;
}

// ===========================================
// 
//  Sensor Streaming mode
// 
// ===========================================
HAL_CAM_Result_en_t CAMDRV_SensorTestMode()
{
    HAL_CAM_Result_en_t result;
    result = stv_SensorTestMode();
    return result;
}
*/
//***************************************************************************
//***************************************************************************



//***************************************************************************
/**
*       STV 0986 & CAMERA SENSOR INITIALIZATION 
*/
static HAL_CAM_Result_en_t Init_Stv0986(CamSensorSelect_t sensor)
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;

    pr_debug("Init_Stv0986(): \r\n");
    sCamI2cStatus = HAL_CAM_SUCCESS; 
    sCamSensor = sensor;
    
    if ( (sensor == CamSensorSecondary) && (sSensorSecondary != SensorUnused) )
    {
        sCamSensorSel = sSensorSecondary;
    // Set Pointers for Secondary Sensor:
        CamSensorCfg_st.sensor_config_caps = &CamSecondaryCfgCap_st;
        ImageSettingsConfig_st.sensor_flashstate = &SecondaryFlashState_st;
        ImageSettingsConfig_st.sensor_framerate = &SecondaryFrameRate_st;
        ImageSettingsConfig_st.sensor_mirrormode = &SecondaryMirrorMode_st;
        pr_debug("Init Secondary Sensor: \r\n");
    }
    else
    {
        sCamSensorSel = sSensorPrimary;
    // Set Pointers for Primary Sensor:
        CamSensorCfg_st.sensor_config_caps = &CamPrimaryCfgCap_st;
        ImageSettingsConfig_st.sensor_flashstate = &PrimaryFlashState_st;
        ImageSettingsConfig_st.sensor_framerate = &PrimaryFrameRate_st;
        ImageSettingsConfig_st.sensor_mirrormode = &PrimaryMirrorMode_st;
        pr_debug("Init Primary Sensor: \r\n");
    }

//---------Disable Camera Sensor
    SensorSetPowerDown(FALSE);

//---------MCU Reset, set MPU speed 
   result = stv0986_init1(sCamSensorSel);
        if (result != HAL_CAM_SUCCESS)
        {
            pr_debug("stv0986_init1(): ERROR:  Failed \r\n");
        }
    #ifdef CAMDBG_STATUS
        pr_debug("Status Registers after stv0986_init1(): \r\n");
        stv_Read_Status();
    #endif

//---------986 Patch for ITU Mode
    StandardPatch_986ITUMode();
    stv_read(bSystemStatus);                                        // read bSystemStatus for I2C write to complete

// Init 2 sets up Pre-Boot ISP and sensor interface, then Boots ISP
    result = stv0986_init2(sCamSensorSel);
    if (result != HAL_CAM_SUCCESS)
    {
        pr_debug("stv0986_init2(): ERROR:  Failed \r\n");
    }
// Read Status Registers  
    #ifdef CAMDBG_STATUS
        pr_debug("Status Registers after stv0986_init2(): \r\n");
        stv_Read_Status();
    #endif

                    
//---------Init 3 changes state to IDLE, sets up ISP and sensor color registers, updates patches to sensor 
    result = stv0986_init3(sCamSensorSel);
    if (result != HAL_CAM_SUCCESS)
    {
        pr_debug("stv0986_init3(): ERROR:  Failed \r\n");
    }
// Read Status Registers  
    #ifdef CAMDBG_STATUS
        pr_debug("Status Registers after stv0986_init3(): \r\n");
        stv_Read_Status();
// Read Sensor Status
        stv_Read_Sensor_Status();
    #endif
   return sCamI2cStatus;
}

//***************************************************************************
/**
*       MCU Reset
*/
static HAL_CAM_Result_en_t stv0986_init1(CamSensor_t sensor_sel)
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
    int id = 0;

    pr_debug("stv0986_init1(): \r\n");
         
// Release MCU reset 
   stv_write(0xa080,0x0); 
   result = stv_WaitValue(2000,0x0810A,0x01);   // Poll H/W register to confirm 986 is out of reset.
   // bSystemControl possible values:    
   // 0x49=73   984 enters in STATE_UNINITIALIZED    
   // 0x39=57 984 enters in STATE_BOOT    
   // 0x11=17 984 enters in STATE_SLEEP    
   // 0x12=18 984 enters in STATE_IDLE    
   // 0x13=19 984 enters in STATE_VIEWFINDER    
   // 0x14=20 984 enters in STATE_STILLS    
   // 0x15=21 984 enters in STATE_MOVIE    
   // 0x16=22 984 enters in STATE_STILL_N_THUMBNAIL    
   // 0x18=24 984 enters in STATE_BUSY  
    result = stv_WaitValue(2000,bSystemStatus,STATE_UNINITIALIZED);  // Poll system status to confirm 
    if (result != HAL_CAM_SUCCESS)
    {
        pr_debug("stv0986_init1(): ERROR:  STATE_UNINITIALIZED Failed \r\n");
    }
    id = stv_read(uwDeviceId_MS) << 8;
    id |= stv_read(uwDeviceId_LS);
    if (id != STV_DEVICE_ID)
    {
        result = HAL_CAM_ERROR_INTERNAL_ERROR;
        pr_debug("stv0986_init1(): ERROR:  Device Id Failed: Id=0x%x, ID Read= 0x%x \r\n", STV_DEVICE_ID, id);
    }
    return result;
}



//***************************************************************************
/**
*       Set Pre-Boot Registers, Patch, then change State to Boot
*/
static HAL_CAM_Result_en_t stv0986_init2(CamSensor_t sensor_sel)
{
    UInt16 clk_speed, count_fall, count_rise, count_high, count_buffer, count_hold_data;
    UInt8 count_setup_data, count_hold_start, count_setup_start, count_setup_stop;
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;

    pr_debug("stv0986_init2 \r\n");

//*****************************************************************************************
// Sensor0 CCP communications set-up
//*****************************************************************************************
// Set sensor data rate (640Mb/s)
    stv_write(0x1089,(UInt8)(sSensor0MaxDataRate >> 8)); // Sensor0VideoTimingInputs_fpHostRxMaxDataRate_Mbps_MSByte
    stv_write(0x108a, (UInt8)sSensor0MaxDataRate); // Sensor0VideoTimingInputs_fpHostRxMaxDataRate_Mbps_LSByte 
// Set Data/Strobe or Data/Clock sensor data encoding:  0x01 = data/clock; 0x02 = data/strobe
    stv_write(0x1082,0x02); // Sensor0VideoTimingInputs_bSensorBitsPerSystemClock
// Set 10:8 compression: 0x0A0A = OFF; 0x0A08 = ON 
    stv_write(0x1085,0x0A); // Sensor0VideoTimingInputs_uwCsiRawFormat_MSByte
    if (sCamSensor0 == SensorVS6555)
    {
        stv_write(0x1086,0x0A); // Sensor0VideoTimingInputs_uwCsiRawFormat_LSByte
    }
    else      // sensor: SensorVB6850, SensorVS6750
    {
        stv_write(0x1086,0x08); // Sensor0VideoTimingInputs_uwCsiRawFormat_LSByte
    }
// Define the video timing mode: 0x01 = AUTO,0x00 = MANUAL
    stv_write(0x1080,0x01); // Sensor0VideoTimingInputs_VideoTimingMode

//*****************************************************************************************
// Sensor1 CCP communications set-up
//*****************************************************************************************
// Set sensor data rate (640Mb/s)
    stv_write(0x1389,(UInt8)(sSensor1MaxDataRate >> 8));     // Sensor1VideoTimingInputs_fpHostRxMaxDataRate_Mbps_MSByte
    stv_write(0x138a,(UInt8)sSensor1MaxDataRate);                // Sensor1VideoTimingInputs_fpHostRxMaxDataRate_Mbps_LSByte 
// Set Data/Strobe or Data/Clock sensor data encoding:  0x01 = data/clock; 0x02 = data/strobe
    stv_write(0x1382,0x02); // Sensor1VideoTimingInputs_bSensorBitsPerSystemClock
// Set 10:8 compression: 0x0A0A = OFF; 0x0A08 = ON 
    stv_write(0x1385,0x0a); // Sensor1VideoTimingInputs_uwCsiRawFormat_MSByte
    if (sCamSensor1 == SensorVS6555)
    {
        stv_write(0x1386,0x0A); // Sensor1VideoTimingInputs_uwCsiRawFormat_LSByte
    }
    else      // sensor: SensorVB6850, SensorVS6750
    {
        stv_write(0x1386,0x08); // Sensor1VideoTimingInputs_uwCsiRawFormat_LSByte
    }
// Define the video timing mode: 0x01 = AUTO,0x00 = MANUAL
    stv_write(0x1380,0x01); // Sensor1VideoTimingInputs_VideoTimingMode

// Set the host master clock frequency (0x4500 = 12MHz oscillator):
    pr_debug("stv0986_init2(): sCamClkSpeed = %d: \r\n", sCamClkSpeed);

    switch(sCamClkSpeed)
    {
// Set up the Master I2C parameters for 12MHz master clock
        case CamDrv_12MHz:              // 12Mhz Camera Clock
                clk_speed                   = 0x4500;
                count_fall              = 0x02;
                count_rise              = 0x02;
                count_high              = 0x08;
                count_buffer            = 0x11;
                count_hold_data     = 0x0B;
                count_setup_data    = 0x02;
                count_hold_start    = 0x07;
                count_setup_start   = 0x08;
                count_setup_stop    = 0x08;
                break;
        case CamDrv_13MHz:              // 13Mhz Camera Clock
                clk_speed                   = 0x4540;
                count_fall              = 0x03;
                count_rise              = 0x03;
                count_high              = 0x08;
                count_buffer            = 0x12;
                count_hold_data     = 0x0B;
                count_setup_data    = 0x02;
                count_hold_start    = 0x07;
                count_setup_start   = 0x08;
                count_setup_stop    = 0x08;
                break;
        case CamDrv_24MHz:              // 24Mhz Camera Clock
                clk_speed                   = 0x4700;
                count_fall              = 0x06;
                count_rise              = 0x06;
                count_high              = 0x11;
                count_buffer            = 0x23;
                count_hold_data     = 0x18;
                count_setup_data    = 0x04;
                count_hold_start    = 0x10;
                count_setup_start   = 0x11;
                count_setup_stop    = 0x11;
                break;
        case CamDrv_26MHz:              // 26Mhz Camera Clock
        default:                                        // Maximum Clock Rate = 26Mhz
                clk_speed                   = 0x4740;
                count_fall              = 0x06;
                count_rise              = 0x06;
                count_high              = 0x11;
                count_buffer            = 0x23;
                count_hold_data     = 0x18;
                count_setup_data    = 0x04;
                count_hold_start    = 0x10;
                count_setup_start   = 0x11;
                count_setup_stop    = 0x11;
                break;
    }
// Set the host master clock frequency 
    stv_write(0x178d,(UInt8)(clk_speed >> 8)); // SystemConfiguration_fpExternalClockFrequency_Mhz_MSByte
    stv_write(0x178e,(UInt8)(clk_speed & 0x00FF)); // SystemConfiguration_fpExternalClockFrequency_Mhz_LSByte
// Set up the Master I2C parameters for 12MHz master clock
    stv_write(0x0c00,(UInt8)count_fall);                 // MasterI2cClockControl [mode static] bCountFall
    stv_write(0x0c02,(UInt8)count_rise);                 // MasterI2cClockControl [mode static] bCountRise
    stv_write(0x0c04,(UInt8)count_high);                 // MasterI2cClockControl [mode static] bCountHigh
    stv_write(0x0c06,(UInt8)count_buffer);           // MasterI2cClockControl [mode static] bCountBuffer
    stv_write(0x0c08,(UInt8)count_hold_data);    // MasterI2cClockControl [mode static] bCountHoldData
    stv_write(0x0c0a,(UInt8)count_setup_data);   // MasterI2cClockControl [mode static] bCountSetupData
    stv_write(0x0c0c,(UInt8)count_hold_start);   // MasterI2cClockControl [mode static] bCountHoldStart
    stv_write(0x0c0e,(UInt8)count_setup_start);  // MasterI2cClockControl [mode static] bCountSetupStart
    stv_write(0x0c10,(UInt8)count_setup_stop);   // MasterI2cClockControl [mode static] bCountSetupStop 

    if (sensor_sel == sCamSensor0)
    {
        stv_write(0x1780,0x01); // SystemConfiguration_fSensor0Present
        stv_write(0x1782,0x00); // SystemConfiguration_fSensor1Present
        pr_debug("stv0986_init2(): Sensor 0 present \r\n");
    }
    else if (sensor_sel == sCamSensor1)
    {
        stv_write(0x1780,0x00); // SystemConfiguration_fSensor0Present
        stv_write(0x1782,0x01); // SystemConfiguration_fSensor1Present
        pr_debug("stv0986_init2(): Sensor 1 present \r\n");
    }
    else
    {
        pr_debug("stv0986_init2(): ERROR:  No sensor present \r\n");
        return HAL_CAM_ERROR_INTERNAL_ERROR;
    }
    
// Identify currently active sensor: 0x01 = Sensor0, 0x02 = Sensor1 
    if ( sensor_sel == sCamSensor0 )
    {
        stv_write(0x5f80,0x01); // StaticImageCharacteristicControlPage_bActiveSensor
        pr_debug("stv0986_init2(): Sensor 0 bActiveSensor \r\n");
    }
    else
    {
        stv_write(0x5f80,0x02); // StaticImageCharacteristicControlPage_bActiveSensor
        pr_debug("stv0986_init2(): Sensor 1 bActiveSensor \r\n");
    }

// Set the maximum sensor size: e.g. 0x0060 = custom; 0xA2 = 5Mpix; 0xA1 = 3Mpix; 0x0059 = UXGA; 0x0058 = SXGA; 0x0056 = VGA; 0x0055 = CIF
    switch(sensor_sel)
    {
        case SensorVB6850:                          // QXGA
        // Set the maximum sensor size: e.g. 0xA1 = 3Mpix
            stv_write(0x5f82,0xA1); // StaticImageCharacteristicControlPage_bSensorStdImageResolution
            pr_debug("stv0986_init2(): SensorVB6850 Size \r\n");
            break;
        case SensorVS6555:                          // VGA Sensor
        // Set the maximum sensor size: e.g. 86 = 0x56 = VGA
            stv_write(0x5f82,0x56); // StaticImageCharacteristicControlPage_bSensorStdImageResolution
            pr_debug("stv0986_init2(): SensorVS6555 Size \r\n");
            break;
        case SensorVS6750:                          // UXGA Sensor
        // Set the maximum sensor size: e.g. 0x0060 = custom; 
            stv_write(0x5f82, 0x0060);      // StaticImageCharacteristicControlPage_bSensorStdImageResolution
        // Define the custom resolution for the sensor (if applicable)
            stv_write(0x5f84, 0x0048);   // StaticImageCharacteristicControlPage_bSensorCustomImageResolution_Xsize_lo
            stv_write(0x5f86, 0x0006);   // StaticImageCharacteristicControlPage_bSensorCustomImageResolution_Xsize_hi
            stv_write(0x5f88, 0x00B8);   // StaticImageCharacteristicControlPage_bSensorCustomImageResolution_Ysize_lo
            stv_write(0x5f8A, 0x0004);   // StaticImageCharacteristicControlPage_bSensorCustomImageResolution_Ysize_hi
            pr_debug("stv0986_init2(): SensorVS6750 Size \r\n");
            break;
        default:
            result = HAL_CAM_ERROR_INTERNAL_ERROR;
            pr_debug("stv0986_init2(): ERROR:  Sensor Size Select Failed \r\n");
            return result;
    }

// Set the Viewfinder in either HIGH or LOW power state: 0x78 = HIGH power state; 0x77 = LOW power state (max size is QCIF viewfinder in low power) 
    stv_write(0x5f8c,0x78); // StaticImageCharacteristicControlPage_bViewfinderMode

//*****************************************************************************************
// Auto-focus set-up
//*****************************************************************************************

    // before the 986 BOOT it is advised to set the AF actuator as present,
    // configure the ranges but to disable af sensor init to avoid I2C transaction failures.
    if ( (sensor_sel == SensorVB6850) )
    {
    // set AF actuator present
        if (sensor_sel == sCamSensor0)
        {
            stv_write(0x1790,0x01); // SystemConfiguration_fFocusLensActuatorOnSensor0Present
            stv_write(0x1792,0x00); // SystemConfiguration_fFocusLensActuatorOnSensor1Present
            pr_debug("stv0986_init2(): Sensor 0 AF Present \r\n");
        }
        else if (sensor_sel == sCamSensor1)
        {
            stv_write(0x1790,0x00); // SystemConfiguration_fFocusLensActuatorOnSensor0Present
            stv_write(0x1792,0x01); // SystemConfiguration_fFocusLensActuatorOnSensor1Present
            pr_debug("stv0986_init2(): Sensor 1 AF Present \r\n");
        }
        else
        {
            stv_write(0x1790,0x00); // SystemConfiguration_fFocusLensActuatorOnSensor0Present
            stv_write(0x1792,0x00); // SystemConfiguration_fFocusLensActuatorOnSensor1Present
            pr_debug("stv0986_init2(): ERROR:  AF Sensor Undefined \r\n");
            return HAL_CAM_ERROR_INTERNAL_ERROR;
        }
            
        //Disable Read from NVM
        stv_write(0x52a2, 0x0); // FLADriverLowLevelParameters_OverwriteAFSensorParameters
        
        // set AF driver as not initialised
        stv_write(0x529e, 0x0); // FLADriverLowLevelParameters_fLowLevelDriverInitialized
        
        // range settings
        //  *** update these values accordingly to actuator available range ***
        //wLensMaxPosition 
        stv_write(0x5505, 0x0); // FocusRangeConstants_wFullRange_LensMaxPosition_MSByte
        stv_write(0x5506, 0xff); // FocusRangeConstants_wFullRange_LensMaxPosition_LSByte
        
        //wLensMinPosition 
        stv_write(0x5501, 0x0); // FocusRangeConstants_wFullRange_LensMinPosition_MSByte
        stv_write(0x5502, 0x0); // FocusRangeConstants_wFullRange_LensMinPosition_LSByte
        
        //wLensRecoveryPosition 
        stv_write(0x5509, 0x0); // FocusRangeConstants_wFullRange_LensRecoveryPosition_MSByte
        stv_write(0x550a, 0x82); // FocusRangeConstants_wFullRange_LensRecoveryPosition_LSByte
        pr_debug("stv0986_init2(): Sensor AF Init \r\n");
    }


//---------Enable Camera Sensor 
    SensorSetPowerDown(TRUE);

// BOOT command
   // bSystemControl possible values:    
   // 0x49=73   984 enters in STATE_UNINITIALIZED    
   // 0x39=57 984 enters in STATE_BOOT    
   // 0x11=17 984 enters in STATE_SLEEP    
   // 0x12=18 984 enters in STATE_IDLE    
   // 0x13=19 984 enters in STATE_VIEWFINDER    
   // 0x14=20 984 enters in STATE_STILLS    
   // 0x15=21 984 enters in STATE_MOVIE    
   // 0x16=22 984 enters in STATE_STILL_N_THUMBNAIL    
   // 0x18=24 984 enters in STATE_BUSY  
    stv_write(bSystemControl, STATE_BOOT);               // SystemControlParameters_bSystemControl Viewfinder mode
    result = stv_WaitValue(2000,bSystemStatus,STATE_SLEEP);  // Poll system status to confirm 986 is in viewfinder state.
    if (result != HAL_CAM_SUCCESS)
    {
        pr_debug("stv0986_init2(): ERROR:  STATE_BOOT Failed \r\n");
        return result;
    }

// Disable Camera Sensor    
    SensorSetPowerDown(FALSE);
        
// Enable Camera Sensor 
    SensorSetPowerDown(TRUE);

// Set the sub-sampling in the sensor: 0x00 = NONE,0x2 = div by 2 
    if ( sCamSensorSel == SensorVB6850 )
    {       
    // Set the sub-sampling in the sensor: 0x00 = NONE,0x2 = div by 2 
        stv_write(0x4e0a,0x02);     // ZoomMgrParams[Mode_Static]_bPrescaleType  
        stv_write(0x4e08,0x02);     // ZoomMgrParams[Mode_Static]_bPrescaleFactor
        stv_write(0x1200,0x02);     // Sensor0ScalingSubSamplingCapabilities_bSensorScalingMode
        stv_write(0x1500,0x02);     // Sensor1ScalingSubSamplingCapabilities_bSensorScalingMode
    }
    else
    {
        stv_write(0x4e0a,0x00);  // ZoomMgrParams[Mode_Static]_bPrescaleType  
        stv_write(0x4e08,0x00);  // ZoomMgrParams[Mode_Static]_bPrescaleFactor
    }

// set the ITU HSYNC and VSYNC polarity to active HI
    stv_write(0x6206,0x00); // ITUControlParameters_bSelectVsyncPolarity
    stv_write(0x6208,0x01); // ITUControlParameters_bSelectHsyncPolarity

// Enable ITU HSYNC only during valid VSYNC
    stv_write(0x6204,0x01); // ITUControlParameters_bEnableSyncIfVsync

// Set the Output Format (to the host) 
// 0x80 CCP 
// 0x81 ITU656_EMB 
// 0x82 ITU601_EXTNSYNC 
// 0x83 PI 
// 0x84 VC 
// 0x85 SPI 
    stv_write(0x6088,0x82); // SystemImageCharacteristicsControlParameters_bOutputDestination

// Set the Image Read Mode = IMAGE_NORMAL_MODE
// 0xb8 = IMAGE_NORMAL_MODE
// 0xb9 = IMAGE_ROTATE_90
// 0xba = IMAGE_ROTATE_180
// 0xbb = IMAGE_ROTATE_270
// 0xbc = IMAGE_VERTICAL_FLIP
// 0xbd = IMAGE_MIRRORING
// 0xbe = IMAGE_YUV_PLANAR
    CAMDRV_SetRotationMode(CamRotate0, sCamSensor);      // SystemImageCharacteristicsControlParameters_bImageReadMode
 
// Set the CCP/ITU output clock speed derating (to the host) 
#if 0   
    if ( sCamSensorSel == SensorVS6555 )
    {       
        stv_write(0x6090,0x02); // SystemImageCharacteristicsControlParameters_bOutputInterfaceClockDerating
        stv_write(0x6282,0x04); // SystemConfigurationParameters_bDMAReadClockDerating
        stv_write(0x6284, 0x04); // SystemConfigurationParameters_bClockT2Div
        stv_write(0x6212,1); // bITUClockRatio <0> = div by 0; <1> = div by 2; <2> = div by 4; <3> = div by 8
    }
    else
#endif    
    {
        stv_write(0x6090,0x01); // SystemImageCharacteristicsControlParameters_bOutputInterfaceClockDerating
        stv_write(0x6282,0x01); // SystemConfigurationParameters_bDMAReadClockDerating
        stv_write(0x6284, 0x03); // SystemConfigurationParameters_bClockT2Div
        stv_write(0x6212,sCamItuClkDiv); // bITUClockRatio 
    }
// Set up the interleaving mode: 
// 0x9A DISABLE 
// 0x9B ALTERNATE_FRAME 
// 0x9C SINGLE_FRAME 
// 0x9D JPEG_INTERLEAVED_AT_END 
// 0x9E JPEG_INTERLEAVED_SOFT 
    stv_write(0x608e,0x9a); // SystemImageCharacteristicsControlParameters_bInterleavingMode

// Set the inter-frame delay
    stv_write(0x6086,0x05); // SystemImageCharacteristicsControlParameters_bInterFrameDelay

// Set MCU clock frequencies
// 0x00 = McuClockSource_Host,0x01 = McuClockSource_Sensor,0x02 = McuClockSource_Pll
    stv_write(0x1680,0x00); // ClockManagerControl_McuClockSource
    stv_write(0x1682,0x0A); // ClockManagerControl_bMcuClockFreuquencyLevel0_Mhz

#if 0   
    if ( sCamSensorSel == SensorVS6555 )
    {       
        stv_write(0x1684,0x0A); // ClockManagerControl_bMcuClockFreuquencyLevel1_Mhz
    }
    else
#endif    
    {
        stv_write(0x1684,0x40); // ClockManagerControl_bMcuClockFreuquencyLevel1_Mhz
    }

// Set the Custom Resolution ViewFinder (Pipe 1) = XGA standard output (Pipe 1)
    stv_write(bViewfinderCustomImageResolution_Xsize_lo,(CamWindowSize_XGA_W & 0x00FF));  // SystemOutputImageDimensionsPage_bViewfinderCustomImageResolution_Xsize_lo
    stv_write(bViewfinderCustomImageResolution_Xsize_hi,(CamWindowSize_XGA_W >> 8));      // SystemOutputImageDimensionsPage_bViewfinderCustomImageResolution_Xsize_hi
    stv_write(bViewfinderCustomImageResolution_Ysize_lo,(CamWindowSize_XGA_H & 0x00FF));  // SystemOutputImageDimensionsPage_bViewfinderCustomImageResolution_Ysize_lo
    stv_write(bViewfinderCustomImageResolution_Ysize_hi,(CamWindowSize_XGA_H >> 8));      // SystemOutputImageDimensionsPage_bViewfinderCustomImageResolution_Ysize_hi

// Set the Custom Resolution STILL/MOVIE = XGA standard output (Pipe 0)
    stv_write(bStillCustomImageResolution_Xsize_lo,(CamWindowSize_XGA_W & 0x00FF));  // SystemOutputImageDimensionsPage_bStillCustomImageResolution_Xsize_lo
    stv_write(bStillCustomImageResolution_Xsize_hi,(CamWindowSize_XGA_W >> 8));      // SystemOutputImageDimensionsPage_bStillCustomImageResolution_Xsize_hi
    stv_write(bStillCustomImageResolution_Ysize_lo,(CamWindowSize_XGA_H & 0x00FF));  // SystemOutputImageDimensionsPage_bStillCustomImageResolution_Ysize_lo
    stv_write(bStillCustomImageResolution_Ysize_hi,(CamWindowSize_XGA_H >> 8));      // SystemOutputImageDimensionsPage_bStillCustomImageResolution_Ysize_hi

    // Set the desired output frame rate 
    result |= SensorSetFrameRate(ImageSettingsConfig_st.sensor_framerate->cur_setting, sCamSensor);
    if (result != HAL_CAM_SUCCESS)
    {
        pr_debug("stv0986_init2(): SensorSetFrameRate():  ERROR: \r\n");
    }
// Set the Resolution & Format for STILL/MOVIE standard output (Pipe 0)
    SensorSetCaptureMode(sCaptureImageResolution, sCaptureImageFormat);        
// Set the Resolution & Format for ViewFinder (Pipe 1) (Pipe 1)
    SensorSetPreviewMode(sViewFinderResolution, sViewFinderFormat);        
    SensorSetInterLeaveMode(CamCaptureVideo, sCaptureImageFormat);

// IDLE command
   // bSystemControl possible values:    
   // 0x49=73   984 enters in STATE_UNINITIALIZED    
   // 0x39=57 984 enters in STATE_BOOT    
   // 0x11=17 984 enters in STATE_SLEEP    
   // 0x12=18 984 enters in STATE_IDLE    
   // 0x13=19 984 enters in STATE_VIEWFINDER    
   // 0x14=20 984 enters in STATE_STILLS    
   // 0x15=21 984 enters in STATE_MOVIE    
   // 0x16=22 984 enters in STATE_STILL_N_THUMBNAIL    
   // 0x18=24 984 enters in STATE_BUSY  
    stv_write(bSystemControl, STATE_IDLE);                               // SystemControlParameters_bSystemControl mode
    stv_read(bSystemStatus);                                                         // read bSystemStatus for I2C write to complete
//            mdelay( 100 );
    result = stv_WaitValue(2000,bSystemStatus,STATE_IDLE);   // Poll system status to confirm 986 is in Idle state.
    if (result != HAL_CAM_SUCCESS)
    {
        pr_debug("stv0986_init2(): STATE_IDLE:  ERROR: \r\n");
        return result;
    }
    pr_debug("stv0986_init2(): Complete  \r\n");
    return result;
}


//***************************************************************************
/**
*       Set After-Boot, Changes state to IDLE, sets up ISP 
*           and sensor color registers, updates patches to sensor
*/
static HAL_CAM_Result_en_t stv0986_init3(CamSensor_t sensor_sel)
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;

    pr_debug("stv0986_init3():  Start \r\n");

    // =================================================
    // Insert the required sensor direct writes here
        // (Sensor personality file)
    // =================================================
// Sensor Patch:  Select Sensor0 or Sensor1
    if (sensor_sel == sCamSensor0)
    {
        stv_write(bActiveSensor,0x01); // HostToSensorAccessControl_bActiveSensor  0x01 = Sensor0,  0x02 = Sensor1
        pr_debug("stv0986_init3(): Sensor 0 Patch \r\n");
    }
    else
    {
        stv_write(bActiveSensor,0x02); // HostToSensorAccessControl_bActiveSensor  0x01 = Sensor0,  0x02 = Sensor1
        pr_debug("stv0986_init3(): Sensor 1 Patch \r\n");
    }

// Sensor Patch:  Select Sensor0 or Sensor1
    switch(sensor_sel)
    {
        case SensorVB6850:                          // QXGA
            result = SensorPatch_VB6850();
            pr_debug("stv0986_init3(): SensorVB6850 Patch \r\n");
            break;
        case SensorVS6555:                          // VGA Sensor
        case SensorVS6750:                          // UXGA Sensor
        default:
            pr_debug("stv0986_init3(): No Sensor Patch \r\n");
            result = HAL_CAM_SUCCESS;
            break;
    }
    if (result != HAL_CAM_SUCCESS)
    {
        pr_debug("stv0986_init3(): Sensor Patch Failed:  ERROR: \r\n");
        return result;
    }
 // =================================================
 // End sensor direct writes (Sensor personality file)
 // =================================================
  
  
// Enable LDO high power mode
   stv_write(0xbe06,0x5f);

// Set Flash
   stv_write( bFM_FlashMode, bFM_FlashMode_NEVER );
// Define the wait time before powering up the LVDS pads when
// starting/stopping streaming - relates to patch 21
   stv_write(0x9170, 0x20); // MSB 0x2000 --> 2ms wait time
   stv_write(0x9171, 0x00); // LSB

//*********************************************************************************************
// AF minidriver setup Rev3.0 - goes with UP-45 & UP-50
//*********************************************************************************************
    // disable analog binning
   stv_write(0x4900,0x00); // BinningControl_fEnableBinning

        if ( (sensor_sel == SensorVB6850) )
        {
        // set AF actuator present
            if (sensor_sel == sCamSensor0)
            {
                stv_write(0x1790,0x01); // SystemConfiguration_fFocusLensActuatorOnSensor0Present
                stv_write(0x1792,0x00); // SystemConfiguration_fFocusLensActuatorOnSensor1Present
                pr_debug("stv0986_init3(): Sensor 0 AF Present \r\n");
            }
            else if (sensor_sel == sCamSensor1)
            {
                stv_write(0x1790,0x00); // SystemConfiguration_fFocusLensActuatorOnSensor0Present
                stv_write(0x1792,0x01); // SystemConfiguration_fFocusLensActuatorOnSensor1Present
                pr_debug("stv0986_init3(): Sensor 1 AF Present \r\n");
            }
            else
            {
                stv_write(0x1790,0x00); // SystemConfiguration_fFocusLensActuatorOnSensor0Present
                stv_write(0x1792,0x00); // SystemConfiguration_fFocusLensActuatorOnSensor1Present
                pr_debug("stv0986_init3(): stv0986_init3(): AF Sensor Undefined:  ERROR: \r\n");
            }
                    
            // set full range
            stv_write(bRangeFocus, 0x0); // FocusControls_bRange 
            
            // configure steps
            stv_write(0x5600, 0x41); // AutoFocusConstants_bCoarseStep
            stv_write(0x5602, 0xf);  // AutoFocusConstants_bFineStep
            
            // formerly bFramesToSkip
            // used now for a different purpose: to add a delay in us between sw average 
            // 10 us delay
            stv_write(0x5290, 0xa); // FLADriverLowLevelParameters_bSwAverageReadsDelay_us
            
            // af_pwm_duty
            // Utilises an unused register location in the map, which was formerly
            // FLADriverLowLevelParameters_bMaxNumberRetries
            // NB: ONLY FOR INITIALISATION. The sw driver doesn't read this value at all. It calculates it internally.
            stv_write(0x529c, 0x0); // FLADriverLowLevelParameters_bPwmDuty
            
            //bLowLevelPositionTolerance
            stv_write(0x5298, 0x1); // FLADriverLowLevelParameters_bLowLevelPositionTolerance
            
            // FLADriverControls_wPositionTolerance {0x5307, 0x5308}
            stv_write(0x5308, 0x3);   // FLADriverControls_wPositionTolerance LSB
            
            // af_pwm_duration_ms
            // Utilises an unused register location in the map, which was formerly
            // FLADriverLowLevelParameters_fOverwriteLowLevelLimits
            // Used for different purposes on 850 CA or BB with Ni-Te coated poles:
            // it specifies the af_pwm_period in ms for the FindEndStops procedure
            stv_write(0x52a0, 100); // FLADriverLowLevelParameters_bCompensationImpulsesDuration_ms
            
            // ENABLE pre & post streaming sensor configuration
            // ENABLE PS Compensation
            // ENABLE timer config for 850 sensor (RETRIGGER)
            stv_write(0x52a2, 0xF); // FLADriverLowLevelParameters_OverwriteAFSensorParameters (byte)
            
            // use continuous minidriver movement
            // stv_write(0x5300, 0x7); // FLADriverControls_bMMode {0x5300}
            // no more - now bypassing minidriver!!!
            stv_write(0x5300, 0x0); // FLADriverControls_bMMode {0x5300}
            
            // autostart the driver timer
            stv_write(0x530e, 0x1);  // FLADriverControls_bTrigger {0x530e}
            
            
            //******** NEW 850 SW DRIVER setup *********
            
            //af_pwm_period
            // Utilises an unused register location in the map, which was formerly
            // FLADriverLowLevelParameters_wHomePosition
            // old value: 250
            // stv_write(0x5289, 0x0); // FLADriverLowLevelParameters_wHomePosition_MSByte
            // stv_write(0x528a, 250); // FLADriverLowLevelParameters_wHomePosition_LSByte
            // new value: 300 = 0x12c
            stv_write(0x5289, 0x01); // FLADriverLowLevelParameters_wHomePosition_MSByte
            stv_write(0x528a, 0x2c); // FLADriverLowLevelParameters_wHomePosition_LSByte
            
            
            // g_read_holdoff_delay {0x52b0}
            // Utilises an unused register location in the map, which was formerly
            // FLADriverLowLevelParameters_MSM_PWM_ClockDiv
            // old value: 166
            // stv_write(0x52b0, 166); // FLADriverLowLevelParameters_bReadHoldoffDelay
            // new value: 230 = 0xE6
            stv_write(0x52b0, 0xE6); // FLADriverLowLevelParameters_bReadHoldoffDelay
            
            // retrigger_time {0x52b3, 0x52b4}
            // Utilises an unused register location in the map, which was formerly
            // FLADriverLowLevelParameters_MSM_PWM_FreqDiv
            // old value: 250/125kHz = 2.0 ms = 0x4000
            // stv_write(0x52b3, 0x40); // FLADriverLowLevelParameters_fpRetriggerTime_ms MSB (float 900)
            // stv_write(0x52b4, 0x00); // FLADriverLowLevelParameters_fpRetriggerTime_ms LSB (float 900)
            // new value: 300 / 125kHz = 2.4ms = 0x4066
            stv_write(0x52b3, 0x40); // FLADriverLowLevelParameters_fpRetriggerTime_ms MSB (float 900)
            stv_write(0x52b4, 0x66); // FLADriverLowLevelParameters_fpRetriggerTime_ms LSB (float 900)
            
            // bPS_SWAverageLog2
            // Position sensor software average. Only used for 850BB to reduce the
            // electrical and acoustic noise
            // For 850BB set to (3) => 2^(3) = 8 cycles
            // Utilises an unused register location in the map, which was formerly
            // FLADriverLowLevelParameters_bLowLevelTimeLimit
            stv_write(0x529a, 0x03); // FLADriverLowLevelParameters_bPsSwAverageLog2
            
            // g_af_framelimit
            // set the sw driver timeout in terms of number of frames
            // Utilises an unused register location in the map, which was formerly
            // FLADriverLowLevelParameters_bMaxNumberRetries
            // old value: 0 - no timeout (af_pwm_duty)
            // new value: 30 frames (2s @ 15 fps)
            stv_write(0x529c, 30); // FLADriverLowLevelParameters_bAFDriverTimeout_Frames
            
            // ***** just for demo - Start ****
            
            //fEnableSimpleCoarseThEvaluation
            stv_write(0x558e, 0x1); // AutoFocusControls_fEnableSimpleCoarseThEvaluation
            
            //bFineToCoarseThreshold
            stv_write(0x560c, 0x6); //AutoFocusConstants_bFineToCoarseThreshold {0x560c}
            
            // disable  motion blur detector during S-AF
            stv_write(0x5584, 0x1);//AutoFocusControls_fFMTesting_AutoDisable {0x5584}
            
            
            stv_write(0x5590, 0x00);     //AutoFocusControls bSelectedMultizoneBehavior {REGIONSELECTIONMETHOD_AVERAGE}
            
            //restart HCS from recovery pos
            stv_write(0x55ac, 0x1); // AutoFocusControls_fResetHCSPos
            
            // ***** just for demo - End ****
            pr_debug("stv0986_init3(): Sensor AF Init \r\n");
        }

//******************************************************************************************************
// Image Quality Settings
//******************************************************************************************************
// ***********************************************
// Image Quality settings generated by Bryan Taylor
// Date: 05-Oct-2007 17:59:21
// Generated for 850
// ***********************************************

    // Set the JPEG still image quality to high
        CAMDRV_SetImageQuality(ImageSettingsConfig_st.sensor_jpegQuality->default_setting, sCamSensor);
    // Set Pkt Interface Size
        stv_SetPacketSize( (UInt16)CamSensorCfg_st.sensor_config_jpeg->jpeg_packet_size_bytes );

    // Set Fixed Pattern Noise
        stv_write(0x2f80, 0x00);     //VfpnControls fEnableCorrection {VPIP_FALSE}

    // Set Exposure
        CAMDRV_SetFlickerControl(ImageSettingsConfig_st.sensor_flicker->default_setting, sCamSensor);
//        stv_write(0x2040, 0x01);     //ExposureControls bAntiFlickerMode {AntiFlickerMode_ManualEnable}
//        stv_write(0x4700, 0x32);     //AntiFlickerExposureControls bMainsFrequency_Hz

        if ( (sensor_sel == SensorVS6555) )
        {
            
            stv_write(0x1bad, 0x0f);     //Sensor0FrameConstraints uwMaxVTLineLengthPck {MSB}
            stv_write(0x1bae, 0xff);     //Sensor0FrameConstraints uwMaxVTLineLengthPck {LSB}
            stv_write(0x1ba9, 0x00);     //Sensor0FrameConstraints uwMinVTLineLengthPck {MSB}
            stv_write(0x1baa, 0x10);     //Sensor0FrameConstraints uwMinVTLineLengthPck {LSB}
            stv_write(0x1b89, 0x02);     //Sensor0FrameConstraints uwVTXAddrMax {MSB}
            stv_write(0x1b8a, 0x87);     //Sensor0FrameConstraints uwVTXAddrMax {LSB}
            stv_write(0x1b8d, 0x01);     //Sensor0FrameConstraints uwVTYAddrMax {MSB}
            stv_write(0x1b8e, 0xe7);     //Sensor0FrameConstraints uwVTYAddrMax {LSB}
            stv_write(0x1a2d, 0xbe);     //Sensor0Capabilities fpSensorAnalogGainConstM1 {MSB}
            stv_write(0x1a2e, 0x00);     //Sensor0Capabilities fpSensorAnalogGainConstM1 {LSB}
            stv_write(0x1a01, 0x00);     //Sensor0Capabilities uwSensorIntegrationTimeCapability {MSB}
            stv_write(0x1a02, 0x00);     //Sensor0Capabilities uwSensorIntegrationTimeCapability {LSB}
            stv_write(0x1a5b, 0x00);     //Sensor0Capabilities uwSensorDataPedestal {MSB}
            stv_write(0x1a5c, 0x00);     //Sensor0Capabilities uwSensorDataPedestal {LSB}            
            
            stv_write(0x1c2d, 0x0f);     //Sensor1FrameConstraints uwMaxVTLineLengthPck {MSB}
            stv_write(0x1c2e, 0xff);     //Sensor1FrameConstraints uwMaxVTLineLengthPck {LSB}
            stv_write(0x1c29, 0x00);     //Sensor1FrameConstraints uwMinVTLineLengthPck {MSB}
            stv_write(0x1c2a, 0x10);     //Sensor1FrameConstraints uwMinVTLineLengthPck {LSB}
            stv_write(0x1c09, 0x02);     //Sensor1FrameConstraints uwVTXAddrMax {MSB}
            stv_write(0x1c0a, 0x87);     //Sensor1FrameConstraints uwVTXAddrMax {LSB}
            stv_write(0x1c0d, 0x01);     //Sensor1FrameConstraints uwVTYAddrMax {MSB}
            stv_write(0x1c0e, 0xe7);     //Sensor1FrameConstraints uwVTYAddrMax {LSB}
            stv_write(0x1aad, 0xbe);     //Sensor1Capabilities fpSensorAnalogGainConstM1 {MSB}
            stv_write(0x1aae, 0x00);     //Sensor1Capabilities fpSensorAnalogGainConstM1 {LSB}
            stv_write(0x1a81, 0x00);     //Sensor1Capabilities uwSensorIntegrationTimeCapability {MSB}
            stv_write(0x1a82, 0x00);     //Sensor1Capabilities uwSensorIntegrationTimeCapability {LSB}
            stv_write(0x1adb, 0x00);     //Sensor1Capabilities uwSensorDataPedestal {MSB}
            stv_write(0x1adc, 0x00);     //Sensor1Capabilities uwSensorDataPedestal {LSB}
            
        #if 0
        // Set Contrained WhiteBalance
            stv_write(0x2794, 0x01);     //WhiteBalanceConstrainerControls fEnableConstrainedWhiteBalance {VPIP_TRUE}
            stv_write(0x2789, 0x3b);     //WhiteBalanceConstrainerControls fpRedB {MSB}
            stv_write(0x278a, 0xa8);     //WhiteBalanceConstrainerControls fpRedB {LSB}
            stv_write(0x278d, 0x3a);     //WhiteBalanceConstrainerControls fpBlueB {MSB}
            stv_write(0x278e, 0x05);     //WhiteBalanceConstrainerControls fpBlueB {LSB}
            stv_write(0x8529, 0x3a);     //DynamicContrainedWhiteBalanceControls fpRedA {MSB}
            stv_write(0x852a, 0x44);     //DynamicContrainedWhiteBalanceControls fpRedA {LSB}
            stv_write(0x852b, 0x3b);     //DynamicContrainedWhiteBalanceControls fpBlueA {MSB}
            stv_write(0x852c, 0x6b);     //DynamicContrainedWhiteBalanceControls fpBlueA {LSB}
            stv_write(0x2791, 0x30);     //WhiteBalanceConstrainerControls fpMaximumDistanceAllowedFromLocus {MSB}
            stv_write(0x2792, 0x0c);     //WhiteBalanceConstrainerControls fpMaximumDistanceAllowedFromLocus {LSB}
             
        // Set Dynamic Contrained White Balance
            // Compiled Intergration Time Range: [6000 , 11000]
            // DynamicGain: 0.2
            stv_write(0x8528, 0x00);     //DynamicContrainedWhiteBalanceControls fEnable {VPIP_FALSE}
            stv_write(0x852d, 0x56);     //DynamicContrainedWhiteBalanceControls fpDamperLowThreshold {MSB}
            stv_write(0x852e, 0xee);     //DynamicContrainedWhiteBalanceControls fpDamperLowThreshold {LSB}
            stv_write(0x852f, 0x58);     //DynamicContrainedWhiteBalanceControls fpDamperHighThreshold {MSB}
            stv_write(0x8530, 0xb0);     //DynamicContrainedWhiteBalanceControls fpDamperHighThreshold {LSB}
            stv_write(0x8531, 0x39);     //DynamicContrainedWhiteBalanceControls fpMinimumDamperOutput {MSB}
            stv_write(0x8532, 0x33);     //DynamicContrainedWhiteBalanceControls fpMinimumDamperOutput {LSB}
        #endif
        
        // Set Auto WhiteBalance
        #if 1
            CAMDRV_SetWBMode(ImageSettingsConfig_st.sensor_wb->default_setting, sCamSensor);
    //        stv_write(0x2500, 0x01);     //WhiteBalanceControls bWBMode {AUTOMATIC}
    //        stv_write(0x2680, 0x20);     //WhiteBalanceStatisticsControls bLowThreshold
        #else
            stv_write(0x2500, 0x03);     //WhiteBalanceControls bWBMode {Manual}
        // Cool White Settings
            #if 1
            stv_write(0x2502, 127); // Red
            stv_write(0x2504, 145); // Green
            stv_write(0x2506, 215); // Blue
            #endif
        // Daylight Settings
            #if 0
            stv_write(0x2502, 232); // Red
            stv_write(0x2504, 182); // Green
            stv_write(0x2506, 127); // Blue
            #endif         
        // Horizon Settings
            #if 0
            stv_write(0x2502, 63);  // Red
            stv_write(0x2504, 75);  // Green
            stv_write(0x2506, 255); // Blue
            #endif         
        // Incandescent Settings
            #if 0
            stv_write(0x2502, 63);  // Red
            stv_write(0x2504, 142); // Green
            stv_write(0x2506, 255); // Blue
            #endif  
        #endif
                           
        // Set Minimum Weighted White Balance
            // Disable: VPIP_FALSE
            // SaturationThreshold: 1000
            stv_write(0x2900, 0x00);     //MinWeightedWBControls fDisable {VPIP_FALSE}

        #if 0
            stv_write(0x2903, 0x03);     //MinWeightedWBControls uwSaturationThreshold {MSB}
            stv_write(0x2904, 0xe8);     //MinWeightedWBControls uwSaturationThreshold {LSB}
             
        // Set Minimum Weighted White Balance Tilts
            // Tilt: [1.1,1,1.1]
            //  
            stv_write(0x2916, 0x02);     //MinWeightedWBControls GreenChannelToAccumulate {UseMeanOfG1G2}
            stv_write(0x2907, 0x3e);     //MinWeightedWBControls fpRedTiltGain {MSB}
            stv_write(0x2908, 0x33);     //MinWeightedWBControls fpRedTiltGain {LSB}
            stv_write(0x290b, 0x3e);     //MinWeightedWBControls fpGreen1TiltGain {MSB}
            stv_write(0x290c, 0x00);     //MinWeightedWBControls fpGreen1TiltGain {LSB}
            stv_write(0x290f, 0x3e);     //MinWeightedWBControls fpGreen2TiltGain {MSB}
            stv_write(0x2910, 0x00);     //MinWeightedWBControls fpGreen2TiltGain {LSB}
            stv_write(0x2913, 0x3e);     //MinWeightedWBControls fpBlueTiltGain {MSB}
            stv_write(0x2914, 0x33);     //MinWeightedWBControls fpBlueTiltGain {LSB}
             
        // Set Auto-Frame Rate Settings
            stv_write(0x4b82, 0x05);     //AutomaticFrameRateControl bImpliedGainThresholdLow_num
            stv_write(0x4b84, 0x01);     //AutomaticFrameRateControl bImpliedGainThresholdLow_den
            stv_write(0x4b86, 0x08);     //AutomaticFrameRateControl bImpliedGainThresholdHigh_num
            stv_write(0x4b88, 0x01);     //AutomaticFrameRateControl bImpliedGainThresholdHigh_den
            stv_write(0x4b8a, 0x04);     //AutomaticFrameRateControl bUserMinimumFrameRate_Hz
            stv_write(0x4b8c, 0x14);     //AutomaticFrameRateControl bUserMaximumFrameRate_Hz

        // Set Analogue Gain Cap
            stv_write(0x1a19, 0x00);     //Sensor0Capabilities uwSensorAnalogGainMaximum {MSB}
            stv_write(0x1a1a, 0xe0);     //Sensor0Capabilities uwSensorAnalogGainMaximum {LSB}
            stv_write(0x1a99, 0x00);     //Sensor1Capabilities uwSensorAnalogGainMaximum {MSB}
            stv_write(0x1a9a, 0xe0);     //Sensor1Capabilities uwSensorAnalogGainMaximum {LSB}
        #endif
        }
        else
        {
        // Set Contrained WhiteBalance
            stv_write(0x2794, 0x01);     //WhiteBalanceConstrainerControls fEnableConstrainedWhiteBalance {VPIP_TRUE}
            stv_write(0x2789, 0x3b);     //WhiteBalanceConstrainerControls fpRedB {MSB}
            stv_write(0x278a, 0xa8);     //WhiteBalanceConstrainerControls fpRedB {LSB}
            stv_write(0x278d, 0x3a);     //WhiteBalanceConstrainerControls fpBlueB {MSB}
            stv_write(0x278e, 0x05);     //WhiteBalanceConstrainerControls fpBlueB {LSB}
            stv_write(0x8529, 0x3a);     //DynamicContrainedWhiteBalanceControls fpRedA {MSB}
            stv_write(0x852a, 0x44);     //DynamicContrainedWhiteBalanceControls fpRedA {LSB}
            stv_write(0x852b, 0x3b);     //DynamicContrainedWhiteBalanceControls fpBlueA {MSB}
            stv_write(0x852c, 0x6b);     //DynamicContrainedWhiteBalanceControls fpBlueA {LSB}
            stv_write(0x2791, 0x30);     //WhiteBalanceConstrainerControls fpMaximumDistanceAllowedFromLocus {MSB}
            stv_write(0x2792, 0x0c);     //WhiteBalanceConstrainerControls fpMaximumDistanceAllowedFromLocus {LSB}
             
        // Set Dynamic Contrained White Balance
            // Compiled Intergration Time Range: [6000 , 11000]
            // DynamicGain: 0.2
            stv_write(0x8528, 0x00);     //DynamicContrainedWhiteBalanceControls fEnable {VPIP_FALSE}
            stv_write(0x852d, 0x56);     //DynamicContrainedWhiteBalanceControls fpDamperLowThreshold {MSB}
            stv_write(0x852e, 0xee);     //DynamicContrainedWhiteBalanceControls fpDamperLowThreshold {LSB}
            stv_write(0x852f, 0x58);     //DynamicContrainedWhiteBalanceControls fpDamperHighThreshold {MSB}
            stv_write(0x8530, 0xb0);     //DynamicContrainedWhiteBalanceControls fpDamperHighThreshold {LSB}
            stv_write(0x8531, 0x39);     //DynamicContrainedWhiteBalanceControls fpMinimumDamperOutput {MSB}
            stv_write(0x8532, 0x33);     //DynamicContrainedWhiteBalanceControls fpMinimumDamperOutput {LSB}

        // Set Auto WhiteBalance
            CAMDRV_SetWBMode(ImageSettingsConfig_st.sensor_wb->default_setting, sCamSensor);
    //        stv_write(0x2500, 0x01);     //WhiteBalanceControls bWBMode {AUTOMATIC}
    //        stv_write(0x2680, 0x20);     //WhiteBalanceStatisticsControls bLowThreshold
             
        // Set Minimum Weighted White Balance
            // Disable: VPIP_FALSE
            // SaturationThreshold: 1000
            stv_write(0x2900, 0x00);     //MinWeightedWBControls fDisable {VPIP_FALSE}
            stv_write(0x2903, 0x03);     //MinWeightedWBControls uwSaturationThreshold {MSB}
            stv_write(0x2904, 0xe8);     //MinWeightedWBControls uwSaturationThreshold {LSB}
             
        // Set Minimum Weighted White Balance Tilts
            // Tilt: [1.1,1,1.1]
            //  
            stv_write(0x2916, 0x02);     //MinWeightedWBControls GreenChannelToAccumulate {UseMeanOfG1G2}
            stv_write(0x2907, 0x3e);     //MinWeightedWBControls fpRedTiltGain {MSB}
            stv_write(0x2908, 0x33);     //MinWeightedWBControls fpRedTiltGain {LSB}
            stv_write(0x290b, 0x3e);     //MinWeightedWBControls fpGreen1TiltGain {MSB}
            stv_write(0x290c, 0x00);     //MinWeightedWBControls fpGreen1TiltGain {LSB}
            stv_write(0x290f, 0x3e);     //MinWeightedWBControls fpGreen2TiltGain {MSB}
            stv_write(0x2910, 0x00);     //MinWeightedWBControls fpGreen2TiltGain {LSB}
            stv_write(0x2913, 0x3e);     //MinWeightedWBControls fpBlueTiltGain {MSB}
            stv_write(0x2914, 0x33);     //MinWeightedWBControls fpBlueTiltGain {LSB}
             
        // Set Auto-Frame Rate Settings
            stv_write(0x4b82, 0x05);     //AutomaticFrameRateControl bImpliedGainThresholdLow_num
            stv_write(0x4b84, 0x01);     //AutomaticFrameRateControl bImpliedGainThresholdLow_den
            stv_write(0x4b86, 0x08);     //AutomaticFrameRateControl bImpliedGainThresholdHigh_num
            stv_write(0x4b88, 0x01);     //AutomaticFrameRateControl bImpliedGainThresholdHigh_den
            stv_write(0x4b8a, 0x04);     //AutomaticFrameRateControl bUserMinimumFrameRate_Hz
            stv_write(0x4b8c, 0x14);     //AutomaticFrameRateControl bUserMaximumFrameRate_Hz

        // Set Analogue Gain Cap
            stv_write(0x1a19, 0x00);     //Sensor0Capabilities uwSensorAnalogGainMaximum {MSB}
            stv_write(0x1a1a, 0xe0);     //Sensor0Capabilities uwSensorAnalogGainMaximum {LSB}
            stv_write(0x1a99, 0x00);     //Sensor1Capabilities uwSensorAnalogGainMaximum {MSB}
            stv_write(0x1a9a, 0xe0);     //Sensor1Capabilities uwSensorAnalogGainMaximum {LSB}
        }
         
    // Set Black Offset
        stv_write(0x4598, 0x00);     //Sensor0Setup BlackCorrectionOffset
        stv_write(0x4618, 0x00);     //Sensor1Setup BlackCorrectionOffset
         
    // Set Contrast
        CAMDRV_SetContrast(ImageSettingsConfig_st.sensor_contrast->default_setting, sCamSensor);

    // Set Saturation
        CAMDRV_SetSaturation(ImageSettingsConfig_st.sensor_saturation->default_setting, sCamSensor);
 
    // Set Gamma to 19
        CAMDRV_SetGamma(ImageSettingsConfig_st.sensor_gamma->default_setting, sCamSensor);

    // Set COLOR MATRIX for both pipes depending on SENS TYPE & SENS POSITION
        CAMDRV_SetDigitalEffect(ImageSettingsConfig_st.sensor_digitaleffect->default_setting, sCamSensor);
//    stv0986_SetMonochrome( FALSE );

    switch(sensor_sel)
    {
    // Image Quality settings VS6555
        case SensorVS6555:                          // VGA Sensor
            stv_write(0x2002, 0x02);     //ExposureMetering flat
            stv_write(0x2010, 0xFE);     //ExposureControls iExposureCompensation

            // Set Tilts: [1.1,1,1.2]
            stv_write(0x458d, 0x3e);     //Sensor0Setup fpRedTiltGain {MSB}
            stv_write(0x458e, 0x33);     //Sensor0Setup fpRedTiltGain {LSB}
            stv_write(0x4591, 0x3e);     //Sensor0Setup fpGreenTiltGain {MSB}
            stv_write(0x4592, 0x00);     //Sensor0Setup fpGreenTiltGain {LSB}
            stv_write(0x4595, 0x3e);     //Sensor0Setup fpBlueTiltGain {MSB}
            stv_write(0x4596, 0x66);     //Sensor0Setup fpBlueTiltGain {LSB}
    
            stv_write(0x460d, 0x3e);     //Sensor1Setup fpRedTiltGain {MSB}
            stv_write(0x460e, 0x33);     //Sensor1Setup fpRedTiltGain {LSB}
            stv_write(0x4611, 0x3e);     //Sensor1Setup fpGreenTiltGain {MSB}
            stv_write(0x4612, 0x00);     //Sensor1Setup fpGreenTiltGain {LSB}
            stv_write(0x4615, 0x3e);     //Sensor1Setup fpBlueTiltGain {MSB}
            stv_write(0x4616, 0x66);     //Sensor1Setup fpBlueTiltGain {LSB}

            #if 1
            // Set Luma Offset
            // Offset: 96
            // Excursion: 351 (255+96)
            // MidPoint*2: 159 (255-96)
                stv_write(0x3c80,0x02); // ColourEngine0_OutputCoderControls TransformType {TransformType_YCbCr_JFIF}
                stv_write(0x3d01,0x01); // ColourEngine0_CoderOutputSignalRange uwLumaExcursion {MSB}
                stv_write(0x3d02,0x5f); // ColourEngine0_CoderOutputSignalRange uwLumaExcursion {LSB}
                stv_write(0x3d05,0x00); // ColourEngine0_CoderOutputSignalRange uwLumaMidpointTimes2 {MSB}
                stv_write(0x3d06,0x9f); // ColourEngine0_CoderOutputSignalRange uwLumaMidpointTimes2 {LSB}
                stv_write(0x3d09,0x01); // ColourEngine0_CoderOutputSignalRange uwChromaExcursion {MSB}
                stv_write(0x3d0a,0x00); // ColourEngine0_CoderOutputSignalRange uwChromaExcursion {LSB}
                stv_write(0x3d0d,0x00); // ColourEngine0_CoderOutputSignalRange uwChromaMidpointTimes2 {MSB}
                stv_write(0x3d0e,0xff); // ColourEngine0_CoderOutputSignalRange uwChromaMidpointTimes2 {LSB}
                
                stv_write(0x3f00,0x02); // ColourEngine1_OutputCoderControls TransformType {TransformType_RGB}
                stv_write(0x3f81,0x01); // ColourEngine1_CoderOutputSignalRange uwLumaExcursion {MSB}
                stv_write(0x3f82,0x5f); // ColourEngine1_CoderOutputSignalRange uwLumaExcursion {LSB}
                stv_write(0x3f85,0x00); // ColourEngine1_CoderOutputSignalRange uwLumaMidpointTimes2 {MSB}
                stv_write(0x3f86,0x9f); // ColourEngine1_CoderOutputSignalRange uwLumaMidpointTimes2 {LSB}
                stv_write(0x3f89,0x01); // ColourEngine1_CoderOutputSignalRange uwChromaExcursion {MSB}
                stv_write(0x3f8a,0x00); // ColourEngine1_CoderOutputSignalRange uwChromaExcursion {LSB}
                stv_write(0x3f8d,0x00); // ColourEngine1_CoderOutputSignalRange uwChromaMidpointTimes2 {MSB}
                stv_write(0x3f8e,0xff); // ColourEngine1_CoderOutputSignalRange uwChromaMidpointTimes2 {LSB}
            #else
            // Set Luma Offset
            // Offset: 100
            // Excursion: 355 (255+100)
            // MidPoint*2: 155 (255-100)
                stv_write(0x3c80,0x02); // ColourEngine0_OutputCoderControls TransformType {TransformType_YCbCr_JFIF}
                stv_write(0x3d01,0x01); // ColourEngine0_CoderOutputSignalRange uwLumaExcursion {MSB}
                stv_write(0x3d02,0x63); // ColourEngine0_CoderOutputSignalRange uwLumaExcursion {LSB}
                stv_write(0x3d05,0x00); // ColourEngine0_CoderOutputSignalRange uwLumaMidpointTimes2 {MSB}
                stv_write(0x3d06,0x9b); // ColourEngine0_CoderOutputSignalRange uwLumaMidpointTimes2 {LSB}
                stv_write(0x3d09,0x01); // ColourEngine0_CoderOutputSignalRange uwChromaExcursion {MSB}
                stv_write(0x3d0a,0x00); // ColourEngine0_CoderOutputSignalRange uwChromaExcursion {LSB}
                stv_write(0x3d0d,0x00); // ColourEngine0_CoderOutputSignalRange uwChromaMidpointTimes2 {MSB}
                stv_write(0x3d0e,0xff); // ColourEngine0_CoderOutputSignalRange uwChromaMidpointTimes2 {LSB}
                
                stv_write(0x3f00,0x02); // ColourEngine1_OutputCoderControls TransformType {TransformType_RGB}
                stv_write(0x3f81,0x01); // ColourEngine1_CoderOutputSignalRange uwLumaExcursion {MSB}
                stv_write(0x3f82,0x63); // ColourEngine1_CoderOutputSignalRange uwLumaExcursion {LSB}
                stv_write(0x3f85,0x00); // ColourEngine1_CoderOutputSignalRange uwLumaMidpointTimes2 {MSB}
                stv_write(0x3f86,0x9b); // ColourEngine1_CoderOutputSignalRange uwLumaMidpointTimes2 {LSB}
                stv_write(0x3f89,0x01); // ColourEngine1_CoderOutputSignalRange uwChromaExcursion {MSB}
                stv_write(0x3f8a,0x00); // ColourEngine1_CoderOutputSignalRange uwChromaExcursion {LSB}
                stv_write(0x3f8d,0x00); // ColourEngine1_CoderOutputSignalRange uwChromaMidpointTimes2 {MSB}
                stv_write(0x3f8e,0xff); // ColourEngine1_CoderOutputSignalRange uwChromaMidpointTimes2 {LSB}
            #endif
    
        // Set Four Channel Anti-Vignetting
            stv_write(0x3080,0x00); // AntiVignetteControls fDisableFilter {VPIP_FALSE}
            stv_write(0x3082,0x01); // AntiVignetteControls fProgramApplicationInputsManually {VPIP_TRUE}
            stv_write(0x3220,0x0F); // AntiVignetteApplicationInputs bFixR2Shift

            // Sensor: Both Channel: GR
            // GreenR Curve Fit
            stv_write(0x3282, 0x40);     //Sensor0AntiVignetteHostInputs bAvUnityOffset_GR
            stv_write(0x328a, 0x1e);     //Sensor0AntiVignetteHostInputs bAvCoeffR2_GR
            stv_write(0x3292, 0x00);     //Sensor0AntiVignetteHostInputs bAvCoeffR4_GR
            // GreenR Curve Fit
            stv_write(0x3302, 0x40);     //Sensor1AntiVignetteHostInputs bAvUnityOffset_GR
            stv_write(0x330a, 0x1e);     //Sensor1AntiVignetteHostInputs bAvCoeffR2_GR
            stv_write(0x3312, 0x00);     //Sensor1AntiVignetteHostInputs bAvCoeffR4_GR
             
            // Sensor: Both Channel: R
            // Red Curve Fit
            stv_write(0x3280, 0x40);     //Sensor0AntiVignetteHostInputs bAvUnityOffset_R
            stv_write(0x3288, 0x32);     //Sensor0AntiVignetteHostInputs bAvCoeffR2_R
            stv_write(0x3290, 0xe6);     //Sensor0AntiVignetteHostInputs bAvCoeffR4_R
            // Red Curve Fit
            stv_write(0x3300, 0x40);     //Sensor1AntiVignetteHostInputs bAvUnityOffset_R
            stv_write(0x3308, 0x32);     //Sensor1AntiVignetteHostInputs bAvCoeffR2_R
            stv_write(0x3310, 0xe6);     //Sensor1AntiVignetteHostInputs bAvCoeffR4_R
             
            // Sensor: Both Channel: B
            // Blue Curve Fit
            stv_write(0x3286, 0x40);     //Sensor0AntiVignetteHostInputs bAvUnityOffset_B
            stv_write(0x328e, 0x14);     //Sensor0AntiVignetteHostInputs bAvCoeffR2_B
            stv_write(0x3296, 0x0a);     //Sensor0AntiVignetteHostInputs bAvCoeffR4_B
            // Blue Curve Fit
            stv_write(0x3306, 0x40);     //Sensor1AntiVignetteHostInputs bAvUnityOffset_B
            stv_write(0x330e, 0x14);     //Sensor1AntiVignetteHostInputs bAvCoeffR2_B
            stv_write(0x3316, 0x0a);     //Sensor1AntiVignetteHostInputs bAvCoeffR4_B
             
            // Sensor: Both Channel: GB
            // GreenB Curve Fit
            stv_write(0x3284, 0x40);     //Sensor0AntiVignetteHostInputs bAvUnityOffset_GB
            stv_write(0x328c, 0x1e);     //Sensor0AntiVignetteHostInputs bAvCoeffR2_GB
            stv_write(0x3294, 0x00);     //Sensor0AntiVignetteHostInputs bAvCoeffR4_GB
            // GreenB Curve Fit
            stv_write(0x3304, 0x40);     //Sensor1AntiVignetteHostInputs bAvUnityOffset_GB
            stv_write(0x330c, 0x1e);     //Sensor1AntiVignetteHostInputs bAvCoeffR2_GB
            stv_write(0x3314, 0x00);     //Sensor1AntiVignetteHostInputs bAvCoeffR4_GB

        // Set AV lens offset
            stv_write(0x3201,0x00); // AntiVignetteApplicationInputs wAvHOffset_GR {MSB}
            stv_write(0x3202,0x00); // AntiVignetteApplicationInputs wAvHOffset_GR {LSB}
            stv_write(0x3211,0x00); // AntiVignetteApplicationInputs wAvVOffset_GR {MSB}
            stv_write(0x3212,0x00); // AntiVignetteApplicationInputs wAvVOffset_GR {LSB}
            stv_write(0x3205,0x00); // AntiVignetteApplicationInputs wAvHOffset_R {MSB}
            stv_write(0x3206,0x00); // AntiVignetteApplicationInputs wAvHOffset_R {LSB}
            stv_write(0x3215,0x00); // AntiVignetteApplicationInputs wAvVOffset_R {MSB}
            stv_write(0x3216,0x00); // AntiVignetteApplicationInputs wAvVOffset_R {LSB}
            stv_write(0x3209,0x00); // AntiVignetteApplicationInputs wAvHOffset_B {MSB}
            stv_write(0x320a,0x00); // AntiVignetteApplicationInputs wAvHOffset_B {LSB}
            stv_write(0x3219,0x00); // AntiVignetteApplicationInputs wAvVOffset_B {MSB}
            stv_write(0x321a,0x00); // AntiVignetteApplicationInputs wAvVOffset_B {LSB}
            stv_write(0x320d,0x00); // AntiVignetteApplicationInputs wAvHOffset_GB {MSB}
            stv_write(0x320e,0x00); // AntiVignetteApplicationInputs wAvHOffset_GB {LSB}
            stv_write(0x321d,0x00); // AntiVignetteApplicationInputs wAvVOffset_GB {MSB}
            stv_write(0x321e,0x00); // AntiVignetteApplicationInputs wAvVOffset_GB {LSB}
            pr_debug("stv0986_init3():  SensorVS6555 ColourEngine \r\n");

            // Set Aperture Correction
            // Gain Damper Pipe 0
            stv_write(0x3a80, 0x00);     //ColourEngine0_ApertureCorrectionControls fDisableCorrection {VPIP_FALSE}
            stv_write(0x3a84, 0x00);     //ColourEngine0_ApertureCorrectionControls fDisableGainDamping {VPIP_FALSE}
            stv_write(0x3a82, 0x14);     //ColourEngine0_ApertureCorrectionControls bMaxGain
            stv_write(0x3a87, 0x62);     //ColourEngine0_ApertureCorrectionControls DamperLowThreshold_Gain {MSB}
            stv_write(0x3a88, 0xa6);     //ColourEngine0_ApertureCorrectionControls DamperLowThreshold_Gain {LSB}
            stv_write(0x3a8b, 0x68);     //ColourEngine0_ApertureCorrectionControls DamperHighThreshold_Gain {MSB}
            stv_write(0x3a8c, 0x33);     //ColourEngine0_ApertureCorrectionControls DamperHighThreshold_Gain {LSB}
            stv_write(0x3a8f, 0x3d);     //ColourEngine0_ApertureCorrectionControls MinimumDamperOutput_Gain {MSB}
            stv_write(0x3a90, 0x33);     //ColourEngine0_ApertureCorrectionControls MinimumDamperOutput_Gain {LSB}
            // Gain Damper Pipe 1
            stv_write(0x3b00, 0x00);     //ColourEngine1_ApertureCorrectionControls fDisableCorrection {VPIP_FALSE}
            stv_write(0x3b04, 0x00);     //ColourEngine1_ApertureCorrectionControls fDisableGainDamping {VPIP_FALSE}
            stv_write(0x3b02, 0x14);     //ColourEngine1_ApertureCorrectionControls bMaxGain
            stv_write(0x3b07, 0x62);     //ColourEngine1_ApertureCorrectionControls DamperLowThreshold_Gain {MSB}
            stv_write(0x3b08, 0xa6);     //ColourEngine1_ApertureCorrectionControls DamperLowThreshold_Gain {LSB}
            stv_write(0x3b0b, 0x68);     //ColourEngine1_ApertureCorrectionControls DamperHighThreshold_Gain {MSB}
            stv_write(0x3b0c, 0x33);     //ColourEngine1_ApertureCorrectionControls DamperHighThreshold_Gain {LSB}
            stv_write(0x3b0f, 0x3d);     //ColourEngine1_ApertureCorrectionControls MinimumDamperOutput_Gain {MSB}
            stv_write(0x3b10, 0x33);     //ColourEngine1_ApertureCorrectionControls MinimumDamperOutput_Gain {LSB}
             
        // Coring Threshold
        // Gain->CompiledIntegration @ 15 fps
        // 1->73427
        // 19.003->1395325
        // Day Setting: Effective ApertureCorrectionCoring of 5 @ Compiled Integration: 73427 
            stv_write(0x3a94, 0x00);     //ColourEngine0_ApertureCorrectionControls fDisableCoringDamping {VPIP_FALSE}
            stv_write(0x3a92, 0x25);     //ColourEngine0_ApertureCorrectionControls bMinimumCoringThreshold
            stv_write(0x3a99, 0x5e);     //ColourEngine0_ApertureCorrectionControls DamperLowThreshold_Coring {MSB}
            stv_write(0x3a9a, 0x3e);     //ColourEngine0_ApertureCorrectionControls DamperLowThreshold_Coring {LSB}
            stv_write(0x3a9d, 0x66);     //ColourEngine0_ApertureCorrectionControls DamperHighThreshold_Coring {MSB}
            stv_write(0x3a9e, 0xa9);     //ColourEngine0_ApertureCorrectionControls DamperHighThreshold_Coring {LSB}
            stv_write(0x3aa1, 0x38);     //ColourEngine0_ApertureCorrectionControls MinimumDamperOutput_Coring {MSB}
            stv_write(0x3aa2, 0x2a);     //ColourEngine0_ApertureCorrectionControls MinimumDamperOutput_Coring {LSB}
            stv_write(0x3a96, 0x05);     //ColourEngine0_ApertureCorrectionControls bMinimumHighThreshold
    
            stv_write(0x3b14, 0x00);     //ColourEngine1_ApertureCorrectionControls fDisableCoringDamping {VPIP_FALSE}
            stv_write(0x3b12, 0x25);     //ColourEngine1_ApertureCorrectionControls bMinimumCoringThreshold
            stv_write(0x3b19, 0x5e);     //ColourEngine1_ApertureCorrectionControls DamperLowThreshold_Coring {MSB}
            stv_write(0x3b1a, 0x3e);     //ColourEngine1_ApertureCorrectionControls DamperLowThreshold_Coring {LSB}
            stv_write(0x3b1d, 0x66);     //ColourEngine1_ApertureCorrectionControls DamperHighThreshold_Coring {MSB}
            stv_write(0x3b1e, 0xa9);     //ColourEngine1_ApertureCorrectionControls DamperHighThreshold_Coring {LSB}
            stv_write(0x3b21, 0x38);     //ColourEngine1_ApertureCorrectionControls MinimumDamperOutput_Coring {MSB}
            stv_write(0x3b22, 0x2a);     //ColourEngine1_ApertureCorrectionControls MinimumDamperOutput_Coring {LSB}
            stv_write(0x3b16, 0x05);     //ColourEngine1_ApertureCorrectionControls bMinimumHighThreshold

        // Set Arctic
//            stv_write(0x2b84, 0x01);     //ArcticControl fTightGreenRequest {VPIP_TRUE}
            stv_write(0x2b8a, 0x0A);     //ArcticControl bScorpioCoringLevel
            stv_write(0x2b8c, 0x05);     //ArcticControl bScorpioCeilingOnes

            // Noise Estimation
            stv_write(0x2b8f, 0x00);     //ArcticControl uwUserFrameSigma {MSB}
            stv_write(0x2b90, 0x14);     //ArcticControl uwUserFrameSigma {LSB}
            stv_write(0x2c82, 0x00);     //ArcticSigmaControl fDisablePromotion {VPIP_FALSE}
            stv_write(0x2c80, 0x18);     //ArcticSigmaControl bMaximumSigmaWeight
            stv_write(0x2c8d, 0x3a);     //ArcticSigmaControl fpMinimumDamperOutput {MSB}
            stv_write(0x2c8e, 0xab);     //ArcticSigmaControl fpMinimumDamperOutput {LSB}
            stv_write(0x2c85, 0x62);     //ArcticSigmaControl fpDamperLowThreshold {MSB}
            stv_write(0x2c86, 0xa6);     //ArcticSigmaControl fpDamperLowThreshold {LSB}
            stv_write(0x2c89, 0x68);     //ArcticSigmaControl fpDamperHighThreshold {MSB}
            stv_write(0x2c8a, 0x33);     //ArcticSigmaControl fpDamperHighThreshold {LSB}
             
            // SetArcticCenterWeight: 8
            stv_write(0x91D9, 0x01);     //ArcticCenterCorrectorControl fDisablePromotion {VPIP_TRUE}
            stv_write(0x91D8, 0x10);     //ArcticCenterCorrectorControl bMaximumCenterCorrectorWeight
             
            // Ring Weight
            stv_write(0x91d1, 0x00);     //ArcticRingControl fDisablePromotion {VPIP_FALSE}
            stv_write(0x91d0, 0x80);     //ArcticRingControl bMaximumRingWeight
            stv_write(0x91d6, 0x3c);     //ArcticRingControl fpMinimumDamperOutput {MSB}
            stv_write(0x91d7, 0x00);     //ArcticRingControl fpMinimumDamperOutput {LSB}
            stv_write(0x91d2, 0x62);     //ArcticRingControl fpDamperLowThreshold {MSB}
            stv_write(0x91d3, 0xa6);     //ArcticRingControl fpDamperLowThreshold {LSB}
            stv_write(0x91d4, 0x68);     //ArcticRingControl fpDamperHighThreshold {MSB}
            stv_write(0x91d5, 0x33);     //ArcticRingControl fpDamperHighThreshold {LSB}
             
            // Set Scythe
//            stv_write(0x8533, 0x1E);     //ArcticScytheControl fUseSensorScythe {VPIP_TRUE}
            stv_write(0x2c02, 0x00);     //ArcticScytheControl fDisablePromotion {VPIP_FALSE}
            stv_write(0x2c00, 0x1E);     //ArcticScytheControl bMaximumScytheWeight
            stv_write(0x2c0d, 0x3c);     //ArcticScytheControl fpMinimumDamperOutput {MSB}
            stv_write(0x2c0e, 0x22);     //ArcticScytheControl fpMinimumDamperOutput {LSB}
            stv_write(0x2c05, 0x62);     //ArcticScytheControl fpDamperLowThreshold {MSB}
            stv_write(0x2c06, 0xa6);     //ArcticScytheControl fpDamperLowThreshold {LSB}
            stv_write(0x2c09, 0x68);     //ArcticScytheControl fpDamperHighThreshold {MSB}
            stv_write(0x2c0a, 0x33);     //ArcticScytheControl fpDamperHighThreshold {LSB}
             
            // Set Gaussian
            stv_write(0x2d02, 0x00);     //ArcticGaussianFilterControl fDisablePromotion {VPIP_FALSE}
            stv_write(0x2d00, 0x50);     //ArcticGaussianFilterControl bMaximumGaussianWeight
            stv_write(0x2d0d, 0x3b);     //ArcticGaussianFilterControl fpMinimumDamperOutput {MSB}
            stv_write(0x2d0e, 0x33);     //ArcticGaussianFilterControl fpMinimumDamperOutput {LSB}
            stv_write(0x2d05, 0x62);     //ArcticGaussianFilterControl fpDamperLowThreshold {MSB}
            stv_write(0x2d06, 0xa6);     //ArcticGaussianFilterControl fpDamperLowThreshold {LSB}
            stv_write(0x2d09, 0x68);     //ArcticGaussianFilterControl fpDamperHighThreshold {MSB}
            stv_write(0x2d0a, 0x33);     //ArcticGaussianFilterControl fpDamperHighThreshold {LSB}

        // Set Colour Matrix Dampers
            stv_write(0x3680, 0x00);     //ColourEngine0_ColourMatrixDamperControl fDisableMatrixDamping {VPIP_FALSE}
            stv_write(0x3683, 0x62);     //ColourEngine0_ColourMatrixDamperControl DamperLowThreshold {MSB}
            stv_write(0x3684, 0xa6);     //ColourEngine0_ColourMatrixDamperControl DamperLowThreshold {LSB}
            stv_write(0x3687, 0x68);     //ColourEngine0_ColourMatrixDamperControl DamperHighThreshold {MSB}
            stv_write(0x3688, 0x33);     //ColourEngine0_ColourMatrixDamperControl DamperHighThreshold {LSB}
            stv_write(0x368b, 0x00);     //ColourEngine0_ColourMatrixDamperControl MinimumDamperOutput {MSB}
            stv_write(0x368c, 0x00);     //ColourEngine0_ColourMatrixDamperControl MinimumDamperOutput {LSB}
    
            stv_write(0x3880, 0x00);     //ColourEngine1_ColourMatrixDamperControl fDisableMatrixDamping {VPIP_FALSE}
            stv_write(0x3883, 0x62);     //ColourEngine1_ColourMatrixDamperControl DamperLowThreshold {MSB}
            stv_write(0x3884, 0xa6);     //ColourEngine1_ColourMatrixDamperControl DamperLowThreshold {LSB}
            stv_write(0x3887, 0x68);     //ColourEngine1_ColourMatrixDamperControl DamperHighThreshold {MSB}
            stv_write(0x3888, 0x33);     //ColourEngine1_ColourMatrixDamperControl DamperHighThreshold {LSB}
            stv_write(0x388b, 0x00);     //ColourEngine1_ColourMatrixDamperControl MinimumDamperOutput {MSB}
            stv_write(0x388c, 0x00);     //ColourEngine1_ColourMatrixDamperControl MinimumDamperOutput {LSB}
             
        #if 0            
            // HighRedNorm =
            
            //    0.4281
            
            // Set NormalisedRedGains for Adaptive Colour Matrix
            // Inc RedNorm: 0.31
            // Day RedNorm: 0.46
            // CoolWhiteToDaylight: 0.6
            stv_write(0x853d, 0x3a);     //AdaptiveColourMatrixPreset fpNormalisedRedGain0 {MSB}
            stv_write(0x853e, 0x75);     //AdaptiveColourMatrixPreset fpNormalisedRedGain0 {LSB}
            stv_write(0x853f, 0x3b);     //AdaptiveColourMatrixPreset fpNormalisedRedGain1 {MSB}
            stv_write(0x8540, 0x6d);     //AdaptiveColourMatrixPreset fpNormalisedRedGain1 {LSB}
        #endif
            break;

    // Image Quality settings VB6850 & All Others
        default:
        case SensorVB6850:                          // QXGA
            stv_write(0x2002, 0x00);     //ExposureMetering flat
            stv_write(0x2010, 0xFD);     //ExposureControls iExposureCompensation

            // Set Tilts: [1,1,1.35]
            stv_write(0x458d, 0x3e);     //Sensor0Setup fpRedTiltGain {MSB}
            stv_write(0x458e, 0x00);     //Sensor0Setup fpRedTiltGain {LSB}
            stv_write(0x4591, 0x3e);     //Sensor0Setup fpGreenTiltGain {MSB}
            stv_write(0x4592, 0x00);     //Sensor0Setup fpGreenTiltGain {LSB}
            stv_write(0x4595, 0x3e);     //Sensor0Setup fpBlueTiltGain {MSB}
            stv_write(0x4596, 0xb3);     //Sensor0Setup fpBlueTiltGain {LSB}
    
            stv_write(0x460d, 0x3e);     //Sensor1Setup fpRedTiltGain {MSB}
            stv_write(0x460e, 0x00);     //Sensor1Setup fpRedTiltGain {LSB}
            stv_write(0x4611, 0x3e);     //Sensor1Setup fpGreenTiltGain {MSB}
            stv_write(0x4612, 0x00);     //Sensor1Setup fpGreenTiltGain {LSB}
            stv_write(0x4615, 0x3e);     //Sensor1Setup fpBlueTiltGain {MSB}
            stv_write(0x4616, 0xb3);     //Sensor1Setup fpBlueTiltGain {LSB}

        // Set Luma Offset
        // Offset: 8
        // Excursion: 263
        // MidPoint*2: 247
            stv_write(0x3c80, 0x02);     //ColourEngine0_OutputCoderControls TransformType {TransformType_YCbCr_Custom}
            stv_write(0x3d01, 0x01);     //ColourEngine0_CoderOutputSignalRange uwLumaExcursion {MSB}
            stv_write(0x3d02, 0x07);     //ColourEngine0_CoderOutputSignalRange uwLumaExcursion {LSB}
            stv_write(0x3d05, 0x00);     //ColourEngine0_CoderOutputSignalRange uwLumaMidpointTimes2 {MSB}
            stv_write(0x3d06, 0xf7);     //ColourEngine0_CoderOutputSignalRange uwLumaMidpointTimes2 {LSB}
            stv_write(0x3d09, 0x01);     //ColourEngine0_CoderOutputSignalRange uwChromaExcursion {MSB}
            stv_write(0x3d0a, 0x00);     //ColourEngine0_CoderOutputSignalRange uwChromaExcursion {LSB}
            stv_write(0x3d0d, 0x00);     //ColourEngine0_CoderOutputSignalRange uwChromaMidpointTimes2 {MSB}
            stv_write(0x3d0e, 0xff);     //ColourEngine0_CoderOutputSignalRange uwChromaMidpointTimes2 {LSB}

            stv_write(0x3f00, 0x02);     //ColourEngine1_OutputCoderControls TransformType {TransformType_YCbCr_Custom}
            stv_write(0x3f81, 0x01);     //ColourEngine1_CoderOutputSignalRange uwLumaExcursion {MSB}
            stv_write(0x3f82, 0x07);     //ColourEngine1_CoderOutputSignalRange uwLumaExcursion {LSB}
            stv_write(0x3f85, 0x00);     //ColourEngine1_CoderOutputSignalRange uwLumaMidpointTimes2 {MSB}
            stv_write(0x3f86, 0xf7);     //ColourEngine1_CoderOutputSignalRange uwLumaMidpointTimes2 {LSB}
            stv_write(0x3f89, 0x01);     //ColourEngine1_CoderOutputSignalRange uwChromaExcursion {MSB}
            stv_write(0x3f8a, 0x00);     //ColourEngine1_CoderOutputSignalRange uwChromaExcursion {LSB}
            stv_write(0x3f8d, 0x00);     //ColourEngine1_CoderOutputSignalRange uwChromaMidpointTimes2 {MSB}
            stv_write(0x3f8e, 0xff);     //ColourEngine1_CoderOutputSignalRange uwChromaMidpointTimes2 {LSB}

        // Set Four Channel Anti-Vignetting
            stv_write(0x3080, 0x00);     //AntiVignetteControls fDisableFilter {VPIP_FALSE}
            stv_write(0x3082, 0x01);     //AntiVignetteControls fProgramApplicationInputsManually {VPIP_TRUE}
            stv_write(0x3220, 0x11);     //AntiVignetteApplicationInputs bFixR2Shift

        // Sensor: Both Channel: GR
            // GreenR Curve Fit
            stv_write(0x3282, 0x40);     //Sensor0AntiVignetteHostInputs bAvUnityOffset_GR
            stv_write(0x328a, 0x19);     //Sensor0AntiVignetteHostInputs bAvCoeffR2_GR
            stv_write(0x3292, 0xFB);     //Sensor0AntiVignetteHostInputs bAvCoeffR4_GR
            // GreenR Curve Fit
            stv_write(0x3302, 0x40);     //Sensor1AntiVignetteHostInputs bAvUnityOffset_GR
            stv_write(0x330a, 0x19);     //Sensor1AntiVignetteHostInputs bAvCoeffR2_GR
            stv_write(0x3312, 0xFB);     //Sensor1AntiVignetteHostInputs bAvCoeffR4_GR
             
        // Sensor: Both Channel: R
            // Red Curve Fit
            stv_write(0x3280, 0x40);     //Sensor0AntiVignetteHostInputs bAvUnityOffset_R
            stv_write(0x3288, 0x1B);     //Sensor0AntiVignetteHostInputs bAvCoeffR2_R
            stv_write(0x3290, 0xFE);     //Sensor0AntiVignetteHostInputs bAvCoeffR4_R
            // Red Curve Fit
            stv_write(0x3300, 0x40);     //Sensor1AntiVignetteHostInputs bAvUnityOffset_R
            stv_write(0x3308, 0x1B);     //Sensor1AntiVignetteHostInputs bAvCoeffR2_R
            stv_write(0x3310, 0xFE);     //Sensor1AntiVignetteHostInputs bAvCoeffR4_R
             
        // Sensor: Both Channel: B
            // Blue Curve Fit
            stv_write(0x3286, 0x40);     //Sensor0AntiVignetteHostInputs bAvUnityOffset_B
            stv_write(0x328e, 0x17);     //Sensor0AntiVignetteHostInputs bAvCoeffR2_B
            stv_write(0x3296, 0xFC);     //Sensor0AntiVignetteHostInputs bAvCoeffR4_B
            // Blue Curve Fit
            stv_write(0x3306, 0x40);     //Sensor1AntiVignetteHostInputs bAvUnityOffset_B
            stv_write(0x330e, 0x17);     //Sensor1AntiVignetteHostInputs bAvCoeffR2_B
            stv_write(0x3316, 0xFC);     //Sensor1AntiVignetteHostInputs bAvCoeffR4_B
             
        // Sensor: Both Channel: GB
            // GreenB Curve Fit
            stv_write(0x3284, 0x40);     //Sensor0AntiVignetteHostInputs bAvUnityOffset_GB
            stv_write(0x328c, 0x1C);     //Sensor0AntiVignetteHostInputs bAvCoeffR2_GB
            stv_write(0x3294, 0xF8);     //Sensor0AntiVignetteHostInputs bAvCoeffR4_GB
            // GreenB Curve Fit
            stv_write(0x3304, 0x40);     //Sensor1AntiVignetteHostInputs bAvUnityOffset_GB
            stv_write(0x330c, 0x1C);     //Sensor1AntiVignetteHostInputs bAvCoeffR2_GB
            stv_write(0x3314, 0xF8);     //Sensor1AntiVignetteHostInputs bAvCoeffR4_GB

        // Set AV lens offset
            stv_write(0x3201, 0xff);     //AntiVignetteApplicationInputs wAvHOffset_GR {MSB}
            stv_write(0x3202, 0xf8);     //AntiVignetteApplicationInputs wAvHOffset_GR {LSB}
            stv_write(0x3211, 0x00);     //AntiVignetteApplicationInputs wAvVOffset_GR {MSB}
            stv_write(0x3212, 0x06);     //AntiVignetteApplicationInputs wAvVOffset_GR {LSB}
            stv_write(0x3205, 0xff);     //AntiVignetteApplicationInputs wAvHOffset_R {MSB}
            stv_write(0x3206, 0xf8);     //AntiVignetteApplicationInputs wAvHOffset_R {LSB}
            stv_write(0x3215, 0xff);     //AntiVignetteApplicationInputs wAvVOffset_R {MSB}
            stv_write(0x3216, 0xfa);     //AntiVignetteApplicationInputs wAvVOffset_R {LSB}
            stv_write(0x3209, 0x00);     //AntiVignetteApplicationInputs wAvHOffset_B {MSB}
            stv_write(0x320a, 0x00);     //AntiVignetteApplicationInputs wAvHOffset_B {LSB}
            stv_write(0x3219, 0xff);     //AntiVignetteApplicationInputs wAvVOffset_B {MSB}
            stv_write(0x321a, 0xc8);     //AntiVignetteApplicationInputs wAvVOffset_B {LSB}
            stv_write(0x320d, 0x00);     //AntiVignetteApplicationInputs wAvHOffset_GB {MSB}
            stv_write(0x320e, 0x0c);     //AntiVignetteApplicationInputs wAvHOffset_GB {LSB}
            stv_write(0x321d, 0xff);     //AntiVignetteApplicationInputs wAvVOffset_GB {MSB}
            stv_write(0x321e, 0xc2);     //AntiVignetteApplicationInputs wAvVOffset_GB {LSB}
            pr_debug("stv0986_init3():  SensorVB6850 ColourEngine \r\n");

        // Set Aperture Correction
            // Gain Damper Pipe 0
            stv_write(0x3a80, 0x00);     //ColourEngine0_ApertureCorrectionControls fDisableCorrection {VPIP_FALSE}
            stv_write(0x3a84, 0x00);     //ColourEngine0_ApertureCorrectionControls fDisableGainDamping {VPIP_FALSE}
            stv_write(0x3a82, 0x14);     //ColourEngine0_ApertureCorrectionControls bMaxGain
            stv_write(0x3a87, 0x5d);     //ColourEngine0_ApertureCorrectionControls DamperLowThreshold_Gain {MSB}
            stv_write(0x3a88, 0xaa);     //ColourEngine0_ApertureCorrectionControls DamperLowThreshold_Gain {LSB}
            stv_write(0x3a8b, 0x5e);     //ColourEngine0_ApertureCorrectionControls DamperHighThreshold_Gain {MSB}
            stv_write(0x3a8c, 0xc0);     //ColourEngine0_ApertureCorrectionControls DamperHighThreshold_Gain {LSB}
            stv_write(0x3a8f, 0x3d);     //ColourEngine0_ApertureCorrectionControls MinimumDamperOutput_Gain {MSB}
            stv_write(0x3a90, 0x33);     //ColourEngine0_ApertureCorrectionControls MinimumDamperOutput_Gain {LSB}
            // Gain Damper Pipe 1
            stv_write(0x3b00, 0x00);     //ColourEngine1_ApertureCorrectionControls fDisableCorrection {VPIP_FALSE}
            stv_write(0x3b04, 0x00);     //ColourEngine1_ApertureCorrectionControls fDisableGainDamping {VPIP_FALSE}
            stv_write(0x3b02, 0x14);     //ColourEngine1_ApertureCorrectionControls bMaxGain
            stv_write(0x3b07, 0x5d);     //ColourEngine1_ApertureCorrectionControls DamperLowThreshold_Gain {MSB}
            stv_write(0x3b08, 0xaa);     //ColourEngine1_ApertureCorrectionControls DamperLowThreshold_Gain {LSB}
            stv_write(0x3b0b, 0x5e);     //ColourEngine1_ApertureCorrectionControls DamperHighThreshold_Gain {MSB}
            stv_write(0x3b0c, 0xc0);     //ColourEngine1_ApertureCorrectionControls DamperHighThreshold_Gain {LSB}
            stv_write(0x3b0f, 0x3d);     //ColourEngine1_ApertureCorrectionControls MinimumDamperOutput_Gain {MSB}
            stv_write(0x3b10, 0x33);     //ColourEngine1_ApertureCorrectionControls MinimumDamperOutput_Gain {LSB}
             
            // Coring Threshold
            stv_write(0x3a94, 0x00);     //ColourEngine0_ApertureCorrectionControls fDisableCoringDamping {VPIP_FALSE}
            stv_write(0x3a92, 0x15);     //ColourEngine0_ApertureCorrectionControls bMinimumCoringThreshold
            stv_write(0x3a99, 0x5d);     //ColourEngine0_ApertureCorrectionControls DamperLowThreshold_Coring {MSB}
            stv_write(0x3a9a, 0xaa);     //ColourEngine0_ApertureCorrectionControls DamperLowThreshold_Coring {LSB}
            stv_write(0x3a9d, 0x5e);     //ColourEngine0_ApertureCorrectionControls DamperHighThreshold_Coring {MSB}
            stv_write(0x3a9e, 0xc0);     //ColourEngine0_ApertureCorrectionControls DamperHighThreshold_Coring {LSB}
            stv_write(0x3aa1, 0x3c);     //ColourEngine0_ApertureCorrectionControls MinimumDamperOutput_Coring {MSB}
            stv_write(0x3aa2, 0xcd);     //ColourEngine0_ApertureCorrectionControls MinimumDamperOutput_Coring {LSB}
            stv_write(0x3a96, 0x15);     //ColourEngine0_ApertureCorrectionControls bMinimumHighThreshold
    
            stv_write(0x3b14, 0x00);     //ColourEngine1_ApertureCorrectionControls fDisableCoringDamping {VPIP_FALSE}
            stv_write(0x3b12, 0x15);     //ColourEngine1_ApertureCorrectionControls bMinimumCoringThreshold
            stv_write(0x3b19, 0x5d);     //ColourEngine1_ApertureCorrectionControls DamperLowThreshold_Coring {MSB}
            stv_write(0x3b1a, 0xaa);     //ColourEngine1_ApertureCorrectionControls DamperLowThreshold_Coring {LSB}
            stv_write(0x3b1d, 0x5e);     //ColourEngine1_ApertureCorrectionControls DamperHighThreshold_Coring {MSB}
            stv_write(0x3b1e, 0xc0);     //ColourEngine1_ApertureCorrectionControls DamperHighThreshold_Coring {LSB}
            stv_write(0x3b21, 0x3c);     //ColourEngine1_ApertureCorrectionControls MinimumDamperOutput_Coring {MSB}
            stv_write(0x3b22, 0xcd);     //ColourEngine1_ApertureCorrectionControls MinimumDamperOutput_Coring {LSB}
            stv_write(0x3b16, 0x15);     //ColourEngine1_ApertureCorrectionControls bMinimumHighThreshold

        // Set Arctic
            stv_write(0x2b84, 0x01);     //ArcticControl fTightGreenRequest {VPIP_TRUE}
            stv_write(0x2b8a, 0x0A);     //ArcticControl bScorpioCoringLevel
            stv_write(0x2b8c, 0x05);     //ArcticControl bScorpioCeilingOnes

            // Noise Estimation
            stv_write(0x2b8f, 0x00);     //ArcticControl uwUserFrameSigma {MSB}
            stv_write(0x2b90, 0x22);     //ArcticControl uwUserFrameSigma {LSB}
            stv_write(0x2c82, 0x00);     //ArcticSigmaControl fDisablePromotion {VPIP_FALSE}
            stv_write(0x2c80, 0x1E);     //ArcticSigmaControl bMaximumSigmaWeight
            stv_write(0x2c8d, 0x36);     //ArcticSigmaControl fpMinimumDamperOutput {MSB}
            stv_write(0x2c8e, 0x00);     //ArcticSigmaControl fpMinimumDamperOutput {LSB}
            stv_write(0x2c85, 0x62);     //ArcticSigmaControl fpDamperLowThreshold {MSB}
            stv_write(0x2c86, 0xa6);     //ArcticSigmaControl fpDamperLowThreshold {LSB}
            stv_write(0x2c89, 0x68);     //ArcticSigmaControl fpDamperHighThreshold {MSB}
            stv_write(0x2c8a, 0x33);     //ArcticSigmaControl fpDamperHighThreshold {LSB}
             
            // SetArcticCenterWeight: 8
            stv_write(0x91D9, 0x01);     //ArcticCenterCorrectorControl fDisablePromotion {VPIP_TRUE}
            stv_write(0x91D8, 0x08);     //ArcticCenterCorrectorControl bMaximumCenterCorrectorWeight
             
            // Ring Weight
            stv_write(0x91d1, 0x00);     //ArcticRingControl fDisablePromotion {VPIP_FALSE}
            stv_write(0x91d0, 0x90);     //ArcticRingControl bMaximumRingWeight
            stv_write(0x91d6, 0x3b);     //ArcticRingControl fpMinimumDamperOutput {MSB}
            stv_write(0x91d7, 0x8e);     //ArcticRingControl fpMinimumDamperOutput {LSB}
            stv_write(0x91d2, 0x62);     //ArcticRingControl fpDamperLowThreshold {MSB}
            stv_write(0x91d3, 0xa6);     //ArcticRingControl fpDamperLowThreshold {LSB}
            stv_write(0x91d4, 0x68);     //ArcticRingControl fpDamperHighThreshold {MSB}
            stv_write(0x91d5, 0x33);     //ArcticRingControl fpDamperHighThreshold {LSB}
             
            // Set Scythe
            stv_write(0x2c02, 0x00);     //ArcticScytheControl fDisablePromotion {VPIP_FALSE}
            stv_write(0x8533, 0x01);     //ArcticScytheControl fUseSensorScythe {VPIP_TRUE}
            stv_write(0x2c00, 0x1D);     //ArcticScytheControl bMaximumScytheWeight
            stv_write(0x2c0d, 0x3c);     //ArcticScytheControl fpMinimumDamperOutput {MSB}
            stv_write(0x2c0e, 0x35);     //ArcticScytheControl fpMinimumDamperOutput {LSB}
            stv_write(0x2c05, 0x56);     //ArcticScytheControl fpDamperLowThreshold {MSB}
            stv_write(0x2c06, 0x62);     //ArcticScytheControl fpDamperLowThreshold {LSB}
            stv_write(0x2c09, 0x67);     //ArcticScytheControl fpDamperHighThreshold {MSB}
            stv_write(0x2c0a, 0x10);     //ArcticScytheControl fpDamperHighThreshold {LSB}
             
            // Set Gaussian
            stv_write(0x2d02, 0x00);     //ArcticGaussianFilterControl fDisablePromotion {VPIP_FALSE}
            stv_write(0x2d00, 0x50);     //ArcticGaussianFilterControl bMaximumGaussianWeight
            stv_write(0x2d0d, 0x3b);     //ArcticGaussianFilterControl fpMinimumDamperOutput {MSB}
            stv_write(0x2d0e, 0x33);     //ArcticGaussianFilterControl fpMinimumDamperOutput {LSB}
            stv_write(0x2d05, 0x62);     //ArcticGaussianFilterControl fpDamperLowThreshold {MSB}
            stv_write(0x2d06, 0xa6);     //ArcticGaussianFilterControl fpDamperLowThreshold {LSB}
            stv_write(0x2d09, 0x68);     //ArcticGaussianFilterControl fpDamperHighThreshold {MSB}
            stv_write(0x2d0a, 0x33);     //ArcticGaussianFilterControl fpDamperHighThreshold {LSB}

        // Set Colour Matrix Dampers
            stv_write(0x3680, 0x00);     //ColourEngine0_ColourMatrixDamperControl fDisableMatrixDamping {VPIP_FALSE}
            stv_write(0x3683, 0x62);     //ColourEngine0_ColourMatrixDamperControl DamperLowThreshold {MSB}
            stv_write(0x3684, 0xa6);     //ColourEngine0_ColourMatrixDamperControl DamperLowThreshold {LSB}
            stv_write(0x3687, 0x68);     //ColourEngine0_ColourMatrixDamperControl DamperHighThreshold {MSB}
            stv_write(0x3688, 0x33);     //ColourEngine0_ColourMatrixDamperControl DamperHighThreshold {LSB}
            stv_write(0x368b, 0x00);     //ColourEngine0_ColourMatrixDamperControl MinimumDamperOutput {MSB}
            stv_write(0x368c, 0x00);     //ColourEngine0_ColourMatrixDamperControl MinimumDamperOutput {LSB}
    
            stv_write(0x3880, 0x00);     //ColourEngine1_ColourMatrixDamperControl fDisableMatrixDamping {VPIP_FALSE}
            stv_write(0x3883, 0x62);     //ColourEngine1_ColourMatrixDamperControl DamperLowThreshold {MSB}
            stv_write(0x3884, 0xa6);     //ColourEngine1_ColourMatrixDamperControl DamperLowThreshold {LSB}
            stv_write(0x3887, 0x68);     //ColourEngine1_ColourMatrixDamperControl DamperHighThreshold {MSB}
            stv_write(0x3888, 0x33);     //ColourEngine1_ColourMatrixDamperControl DamperHighThreshold {LSB}
            stv_write(0x388b, 0x00);     //ColourEngine1_ColourMatrixDamperControl MinimumDamperOutput {MSB}
            stv_write(0x388c, 0x00);     //ColourEngine1_ColourMatrixDamperControl MinimumDamperOutput {LSB}
            
        // HighRedNorm =
            //    0.4281
            
            // Set NormalisedRedGains for Adaptive Colour Matrix
            // Inc RedNorm: 0.31
            // Day RedNorm: 0.46
            // CoolWhiteToDaylight: 0.6
            stv_write(0x853d, 0x3a);     //AdaptiveColourMatrixPreset fpNormalisedRedGain0 {MSB}
            stv_write(0x853e, 0x75);     //AdaptiveColourMatrixPreset fpNormalisedRedGain0 {LSB}
            stv_write(0x853f, 0x3b);     //AdaptiveColourMatrixPreset fpNormalisedRedGain1 {MSB}
            stv_write(0x8540, 0x6d);     //AdaptiveColourMatrixPreset fpNormalisedRedGain1 {LSB}
            break;
    }

// JPEG Configuration Settings
   stv_write(bSqueezeSettings,0xA1); // JPEGImageCharateristicsControlParameters_bSqueezeSettings       

// Flip U and V components for YUV, Pipe0 (Movie) if neccessary, fCb_Cr_Flip        
   stv_write(0x0390,0x00);      
            
// Flip Y and U/V components for YUV, Pipe0 (Movie) if neccessary, fY_CbCr_Flip: 0x0 = YUYV; 0x1 = UYVY     
   stv_write(0x0392,0x00); //PipeSetupBankA_fY_CbCr_Flip                
            
// Flip U and V components for YUV, Pipe1 (Viewfinder) if neccessary, fCb_Cr_Flip       
   stv_write(0x0410,0x00);      
            
// Flip Y and U/V components for YUV, Pipe1 (Viewfinder) if neccessary, fY_CbCr_Flip: 0x0 = YUYV; 0x1 = UYVY        
   stv_write(0x0412,0x00); //PipeSetupBankB_fY_CbCr_Flip        
   stv_read(bSystemStatus); // read bSystemStatus for I2C write to complete

    pr_debug("stv0986_init3():  Complete \r\n");
    return result;
}



static void stv0986_SetColorMatrix( Stv0986_RegAddr_ColMat_t ce_S0S1, UInt16 * pColMatrix )
{
    int i;
    UInt16 reg_addr;
    UInt16 reg_val;
    UInt8 val;
    
    for ( i = 0; i < 9; i++ )
    {
        reg_addr = stv0986_RegAddr_CM[ce_S0S1][i];
        reg_val = pColMatrix[i];
        val = (UInt8)( (reg_val >> 8) & 0x00FF);
        stv_write( reg_addr ,  val);
        reg_addr += 1;
        val = (UInt8)( reg_val & 0x00FF);
        stv_write( (reg_addr) , val );
    }
  stv_read(bSystemStatus);                                                              // read bSystemStatus for I2C write to complete
}

// FOR CURRENTLY SELECTED SENSOR, SET B&W MODE ON/OFF FOR BOTH PIPES
// BY CHANGING COLOR MATRIX 
static void stv0986_SetMonochrome( Boolean on )
{
    UInt16 * pColMatrix = NULL;
 
    // Sensor Type
    if( on == TRUE )
    {
        pColMatrix = &stv0986_col_matrix_BW[0];
    }
    else
    {
	    pr_debug("stv0986_SetMonochrome():  sCamSensorSel=%d \r\n", sCamSensorSel);
        switch( sCamSensorSel )
        {
            case SensorVS6555:
                pColMatrix = &vs6555_col_matrix_sRGB[0];
                break;
            case SensorVS6750:
                pColMatrix = &vs6750_col_matrix_sRGB[0];
                break;
            case SensorVB6850:
                pColMatrix = &vb6850_col_matrix_sRGB_r03[0];
                break;
            default:
                break;
        }
    }
    
    if( pColMatrix )
    {
        stv0986_SetColorMatrix( ColEng0_Sensor0, pColMatrix );
        stv0986_SetColorMatrix( ColEng1_Sensor0, pColMatrix );
        stv0986_SetColorMatrix( ColEng0_Sensor1, pColMatrix );
        stv0986_SetColorMatrix( ColEng1_Sensor1, pColMatrix );
    }
}

#if 0
//****************************************************************************
//
// Function Name:   UInt16 stv0986_Float2VpipFloat
//
// Description:     Converts float to ST's 16-bit float value
//
// Notes:           
//                  
//****************************************************************************
UInt16 stv0986_Float2VpipFloat( float float_in )
{
    UInt16  result; 
    UInt16  sign;   
    Int32   whole_si;
    float   fraction_f;
    UInt64  whole_ui;
    UInt64  fraction_ui;
    UInt64  result_ui;
    float   value   = float_in;
    Int32   exp     = 31;
        
    if( value < 0 )
    {
        // value = fabsf(value); // fabs does not work, returns 0 ? 
        value = value * -1;
        sign = 0x8000;
    }   
    else
    {
        sign = 0x0000;  
    }
        
    whole_si    = (Int32) value;        
    fraction_f  = value - whole_si;     
    
    whole_ui    = ((UInt64) whole_si) << 32;                
    fraction_ui = ((UInt64)(fraction_f * 65536)) << 16; 
    result_ui   = whole_ui | fraction_ui;               

    if( value == 0 ) 
    {
        result = 0; 
    }
    else if( value >= 1 ) 
    {
        while( (result_ui & 0xFFFFFFFF00000000ULL) != 0x0000000100000000ULL )     
        {
            result_ui = result_ui >> 1;                                           
            exp++;
        }
        result = (UInt16) ((result_ui & 0x00000000FF800000ULL) >> 23);            
    } 
    else if( value < 1 ) 
    {
        while( (result_ui & 0xFFFFFFFF00000000ULL) != 0x0000000100000000ULL )
        {
            result_ui = result_ui << 1;
            exp--;
        }
        result = (UInt16) ((result_ui & 0x00000000FF800000ULL) >> 23);
    } 
    result = result | (exp << 9) | sign;
    
    pr_debug("stv0986_Float2VpipFloat: %f => 0x%04X \r\n", float_in, result);
    return ( result );
}
#endif
#if 0
//****************************************************************************
//
// Function Name:   stv0986_FlashManagerOn
//
// Description:     Setup Flash Manager
//
// Notes:           
//                  
//****************************************************************************
static HAL_CAM_Result_en_t stv0986_FlashManagerOn(
    UInt8 preCount, 
    float preLen_us, 
    float mainLen_us )
{

    UInt16      float_16;
    HAL_CAM_Result_en_t    result = HAL_CAM_SUCCESS;
    
    stv_write( bFM_FlashType, bFM_FlashType_HIGH_PWR_LED );
        
    stv_write( bFM_NumberOfPreFlashes, preCount );
    float_16 = stv0986_Float2VpipFloat( preLen_us );
    stv_write_2( fFM_PreFlashPulseWidth_us , (UInt32)float_16 );
    float_16 = stv0986_Float2VpipFloat( mainLen_us );
    stv_write_2( fFM_MainFlashPulseWidth_us, (UInt32)float_16 );
    
    stv_write( bFM_TimingRefCalcMode, bFM_TimingRefCalcMode_MAN );
    
    stv_write( fInhibitExposurePresetModeForFlash    , 1 );
    stv_write( fInhibitWhiteBalancePresetModeForFlash, 1 );
    
    // FLASH MANUAL MODE
    // bSystemFrameCount          - total number of frames ISP will spent
    //                              in INT & EXT streaming mode(includes
    //                              bFM_ManModeTotalFrameCount )
    //                                                          
    // bFM_ManModeTotalFrameCount - number of internal frames 
    //                              (not streamed out of ISP) FM needs before 
    //                              ISP starts to stream frames out
    //
    // still caputure will capture n-th frame, where n = pre_frame_still + 1
    // FRAMES STREAMED OUT = bSystemFrameCount - bFM_ManModeTotalFrameCount + 1

    stv_write( bSystemFrameCount, 0 );  
    stv_write( bFM_ManModeTotalFrameCount, 1 );  
    
    stv_write( bFM_PreFlashStartFrame ,   
               CamSensorCfg_st.sensor_config_caps->output_st.pre_frame_still + 1 - preCount);
    stv_write( bFM_MainFlashStartFrame, 
               CamSensorCfg_st.sensor_config_caps->output_st.pre_frame_still + 1 );
        

    switch ( stv0986_sys_flash_state )
    {
        case Flash_On:          // FLASH ALWAYS
            stv_write( bFM_FlashMode, bFM_FlashMode_ALWAYS );
            break;
        case FlashLight_Auto:   // FLASH AUTO
            stv_write( bFM_FlashMode, bFM_FlashMode_AUTO );
            break;
        default:
            break;  
    }
                  
    stv0986_fm_is_on = TRUE;    
                  
    return( result );              
}
#endif

static HAL_CAM_Result_en_t stv0986_FlashManagerOff( void )
{
    stv_write( bSystemFrameCount, 0 );  
    stv_write( bFM_FlashMode, bFM_FlashMode_NEVER );
    stv_write( fInhibitExposurePresetModeForFlash    , 0 );
    stv_write( fInhibitWhiteBalancePresetModeForFlash, 0 );
    
    stv0986_fm_is_on = FALSE;
    return ( HAL_CAM_SUCCESS );              
}

//****************************************************************************
//
// Function Name:   stv0986_GpioSetFlashModeTorchOff
//
// Description:     Turn TORCH Off
//
// Notes:           Configure 0986 GPIOs for Flash Driving mode
//                  ( TORCH light will turn OFF if it was ON )
//                  
//****************************************************************************
static HAL_CAM_Result_en_t stv0986_GpioSetFlashModeTorchOff( void )
{
    UInt8   gpio;
    
    if( stv0986_torch_is_on )
    {
        // SET GPIOs FUNCTION DEDICATED/GENERIC IOs       
        gpio = stv_read( STV0986_GPIO_TYPE ) & 0xE3;
        stv_write( STV0986_GPIO_TYPE, 
                    gpio 
                  | ( STV0986_GPIO_TYPE_GENERIC    << FLASH_PIN_TORCH_FLASH  )
                  | ( STV0986_GPIO_TYPE_DEDICATED  << FLASH_PIN_TRIGGER_PRE  )
                  | ( STV0986_GPIO_TYPE_DEDICATED  << FLASH_PIN_TRIGGER_MAIN ));

        // SET THEM TO BE OUTPUTs         
        gpio = stv_read( STV0986_GPIO_DIRECTION ) & 0xE3;
        stv_write( STV0986_GPIO_DIRECTION, 
                    gpio 
                  | ( STV0986_GPIO_DIRECTION_OUT << FLASH_PIN_TORCH_FLASH  )
                  | ( STV0986_GPIO_DIRECTION_OUT << FLASH_PIN_TRIGGER_PRE  )
                  | ( STV0986_GPIO_DIRECTION_OUT << FLASH_PIN_TRIGGER_MAIN ));

        // SELECT FLASH MODE
        gpio = stv_read( STV0986_GPIO_OUTPUT ) & 0xE3;
        stv_write( STV0986_GPIO_OUTPUT, 
                    gpio 
                  | ( STV0986_GPIO_OUTPUT_FLASH_MODE  << FLASH_PIN_TORCH_FLASH  )
                  | ( STV0986_GPIO_OUTPUT_OFF   << FLASH_PIN_TRIGGER_PRE  )
                  | ( STV0986_GPIO_OUTPUT_OFF   << FLASH_PIN_TRIGGER_MAIN ));
    
        stv0986_torch_is_on = FALSE;        
    }     
    return ( HAL_CAM_SUCCESS );              
}


//****************************************************************************
//
// Function Name:   stv0986_GpioSetTorchModeTorchOn
//
// Description:     Turn TORCH On
//
// Notes:           Configure 0986 GPIOs for Torch Driving mode & Turn TORCH On
//                  
//****************************************************************************
static HAL_CAM_Result_en_t stv0986_GpioSetTorchModeTorchOn( void )
{
    UInt8   gpio;
    
    if( !stv0986_torch_is_on )
    {
        // SET GPIOs FUNCTION DEDICATED/GENERIC IOs       
        gpio = stv_read( STV0986_GPIO_TYPE ) & 0xE3;
        stv_write( STV0986_GPIO_TYPE, 
                    gpio 
                  | ( STV0986_GPIO_TYPE_GENERIC  << FLASH_PIN_TORCH_FLASH  )
                  | ( STV0986_GPIO_TYPE_GENERIC  << FLASH_PIN_TRIGGER_PRE  )
                  | ( STV0986_GPIO_TYPE_GENERIC  << FLASH_PIN_TRIGGER_MAIN ));

        // SET THEM TO BE OUTPUTs         
        gpio = stv_read( STV0986_GPIO_DIRECTION ) & 0xE3;
        stv_write( STV0986_GPIO_DIRECTION, 
                    gpio 
                  | ( STV0986_GPIO_DIRECTION_OUT << FLASH_PIN_TORCH_FLASH  )
                  | ( STV0986_GPIO_DIRECTION_OUT << FLASH_PIN_TRIGGER_PRE  )
                  | ( STV0986_GPIO_DIRECTION_OUT << FLASH_PIN_TRIGGER_MAIN ));

        // SELECT TORCH MODE, SET FLASH DRV ENA
        gpio = stv_read( STV0986_GPIO_OUTPUT ) & 0xE3;
        stv_write( STV0986_GPIO_OUTPUT, 
                    gpio 
                  | ( STV0986_GPIO_OUTPUT_TORCH_MODE  << FLASH_PIN_TORCH_FLASH  )
                  | ( STV0986_GPIO_OUTPUT_ON    << FLASH_PIN_TRIGGER_PRE  )
                  | ( STV0986_GPIO_OUTPUT_ON    << FLASH_PIN_TRIGGER_MAIN ));
                  
        stv0986_torch_is_on = TRUE;         
    }    
    return ( HAL_CAM_SUCCESS );              
}

static HAL_CAM_Result_en_t stv_write(unsigned int sub_addr, unsigned char data)
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;

    result |= CAM_WriteI2c( sub_addr, 1, &data );
    if (result != HAL_CAM_SUCCESS)
    {
        sCamI2cStatus = result; 
        pr_debug("stv_write(): ERROR: \r\n");
    }
    return result;
}

static UInt8 stv_read(unsigned int sub_addr)
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
    UInt8 data;
    
    result |= CAM_ReadI2c( sub_addr, 1, &data );
    if (result != HAL_CAM_SUCCESS)
    {
        sCamI2cStatus = result; 
        pr_debug("stv_read(): ERROR: \r\n");
    }

    return data;
}

//***************************************************************************
/**
*       stv_WaitValue, wait for mode change of stv0986
*/
static HAL_CAM_Result_en_t stv_WaitValue(UInt32 timeout, UInt16 sub_addr, UInt8 value)
{
    UInt8 register_value;
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;

//    register_value = (stv_read(sub_addr) & 0x00FF);     // read 1st time
//    mdelay( 10 );
    
    do
    {
        register_value = (stv_read(sub_addr) & 0x00FF);
        mdelay(1);                                            // Minimum wait time is 500us (for SLEEP --> IDLE)
        timeout--;
    } while ( (timeout != 0) && (register_value != value) );

    if (timeout == 0)
    {
        result = HAL_CAM_ERROR_INTERNAL_ERROR;
        pr_debug("stv_WaitValue(): ERROR: addr = 0x%x value_read=0x%x value_set=0x%x \r\n", sub_addr,register_value,value);
        register_value = stv_read(bSystemErrorStatus);      
        pr_debug("stv_WaitValue(): bSystemErrorStatus = %d \r\n", register_value);
    }
    return result;
}
        

//***************************************************************************
/**
*       StvWriteAutoIncrement, write array to I2c port
*/
static void StvWriteAutoIncrement ( UInt16 sub_addr, UInt8 size, ... )
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
    va_list list;
    UInt8 i2c_data[MAX_I2C_DATA_COUNT];
    int i;

    pr_debug("%s(): i2c_sub_adr = 0x%x  i2c_byte_cnt = %d \r\n", __FUNCTION__, sub_addr, size);

    if (size > MAX_I2C_DATA_COUNT) {
        pr_debug("%s(): ERROR: i2c_byte_cnt is too big\r\n", __FUNCTION__);
        return;
    }

    va_start(list, size);
    for ( i = 0; i < size; i++ )
        i2c_data[i] = (UInt8)va_arg( list, int );  // next value in argument list.
    va_end(list);

    result = CAM_WriteI2c( sub_addr, (UInt16)size, i2c_data );  // write values to I2C
    if (result != HAL_CAM_SUCCESS)
    {
        sCamI2cStatus = result; 
        pr_debug("StvWriteAutoIncrement(): ERROR: \r\n");
    }
}
           

//***************************************************************************
/**
*       Status Register Read of stv0986
*/
#ifdef CAMDBG_STATUS
static void stv_Read_Status(void)
{
    UInt16 register_value;
   //   
   // Read Status Registers  
   //   
//---------Read bSystemStatus               
    register_value = stv_read(bSystemStatus);        
    pr_debug("stv_Read_Status(): bSystemStatus = %d \r\n", register_value);
//---------Read bSystemErrorStatus
                // <89> ERROR_NONE - No error
                // <96> ERROR_IMAGE_CORRUPT_VP_FIFO_OVFL - the image sent to the host is corrupt due to VP FIFO OVERFLOW
                // <191> ERROR_IMAGE_CORRUPT_VC_FIFO_OVFL - the image sent to the host is corrupt due to VC FIFO OVERFLOW
                // <97> ERROR_IMAGE_CORRUPT_VC_MEM_OVFL - the image sent to the host is corrupt due to VC MEMORY OVERFLOW
                // <98> ERROR_IMAGE_CORRUPT_VP_MEM_OVFL - the image sent to the host is corrupt due to VP MEMORY OVERFLOW
                // <99> ERROR_IMAGE_CORRUPT_JPEG_ABORT - the image sent to the host is corrupt due to JPEG ABORTED
                // <100> ERROR_IMAGE_CORRUPT_OIF_FIFO_OVERFLOW - the image sent to the host is corrupt due to OIF FIFO OVERFLOW
                // <101> ERROR_IMAGE_CORRUPT_OIF_FIFO_UNDERRUN - the image sent to the host is corrupt due to OIF FIFO UNDERRUN
                // <102> ERROR_SYS_RESET_SENSOR_CLOCK_LOST - reports that the system has reset because the sensor CCP interface 
                //      clock was lost. The system  has been reset and therefore must be re-initialized by host.
                // <103> ERROR_SYS_RESET_MCU_HANG - reports that the system has reset because the MCU hung up during operations. 
                //      The system has been reset and therefore must be re-initialized by host.
                // <121> ERROR_INCORRECT_STATE_CHANGE - reports that the host has issued an invalid state change. For example, 
                //      the host tried to change the STV0986 mode while the STV0986 was in a busy state.
                // <190> ERROR_INVALID_SPECIAL_MODE - reports that the host has selected a bImageReadMode mode that is incompatible 
                //      with the streaming mode selected.               
   register_value = stv_read(bSystemErrorStatus);       
    pr_debug("stv_Read_Status(): bSystemErrorStatus = %d \r\n", register_value);

//---------Read fSensor0Available               
   register_value = stv_read(0x1980);       
    pr_debug("stv_Read_Status(): fSensor0Available = %d \r\n", register_value);
//  Read fFarSensor Information only if available           
        if (register_value == 0x01)
        {       
        //---------Read uwFarSensorModelId
           register_value = stv_read(0x1983) << 8;
           register_value |= stv_read(0x1984);
            pr_debug("stv_Read_Status(): uwFarSensorModelId = 0x%x \r\n", register_value);
        //---------Read bFarSensorRevision
           register_value = stv_read(0x1986);
            pr_debug("stv_Read_Status(): bFarSensorRevision = 0x%x \r\n", register_value);
        //---------Read bFarSensorManufactureId
           register_value = stv_read(0x1988);
            pr_debug("stv_Read_Status(): bFarSensorManufactureId = 0x%x \r\n", register_value);
        //---------Read bFarSensorSmiaVersion
           register_value = stv_read(0x198A);
            pr_debug("stv_Read_Status(): bFarSensorSmiaVersion = 0x%x \r\n", register_value);
        }

//---------Read fFarSensor1Available                
   register_value = stv_read(0x198C);       
    pr_debug("stv_Read_Status(): fSensor1Available = 0x%x \r\n", register_value);
//  Read fNearSensor Information only if available          
        if (register_value == 0x01)
        {       
        //---------Read uwNearSensorModelId
           register_value = stv_read(0x198F) << 8;
           register_value |= stv_read(0x1990);
            pr_debug("stv_Read_Status(): uwNearSensorModelId = 0x%x \r\n", register_value);
        //---------Read bNearSensorRevision
           register_value = stv_read(0x1992);
            pr_debug("stv_Read_Status(): bNearSensorRevision = 0x%x \r\n", register_value);
        //---------Read bNearSensorManufactureId
           register_value = stv_read(0x1994);
            pr_debug("stv_Read_Status(): bNearSensorManufactureId = 0x%x \r\n", register_value);
        //---------Read bNearSensorSmiaVersion
           register_value = stv_read(0x1996);
            pr_debug("stv_Read_Status(): bNearSensorSmiaVersion = 0x%x \r\n", register_value);
        }

//---------Read bActiveSensor
   register_value = stv_read(0x5F80);       
    pr_debug("stv_Read_Status(): bActiveSensor = 0x%x \r\n", register_value);
//---------Read bSensorStdImageResolution
   register_value = stv_read(0x5F82);       
    pr_debug("stv_Read_Status(): bSensorStdImageResolution = 0x%x \r\n", register_value);

//---------Read bCurrentlyActiveSensor
   register_value = stv_read(0x1998);       
    pr_debug("stv_Read_Status(): bCurrentlyActiveSensor = 0x%x \r\n", register_value);
//---------Read fCurrentSensorAvailable
   register_value = stv_read(0x199A);       
    pr_debug("stv_Read_Status(): fCurrentSensorAvailable = 0x%x \r\n", register_value);
}
#endif

//***************************************************************************
/**
*       Jpeg File Size
*/
static UInt32 stv_Read_JpegSize(void)
{
    UInt8 register_value;
    UInt32 jpeg_size;

//---------Read Jpeg File Size
   jpeg_size = stv_read(image_size_lo_0to7);                    // JPEGImageCharateristicsControlParameters_image_size_lo_0to7
   register_value = stv_read(image_size_lo_8to15);      // JPEGImageCharateristicsControlParameters_image_size_lo_8to15
   jpeg_size |= ((UInt32)register_value << 8);
   register_value = stv_read(image_size_lo_16to23);     // JPEGImageCharateristicsControlParameters_image_size_lo_16to23
   jpeg_size |= ((UInt32)register_value << 16);
   pr_debug("stv_Read_JpegSize(): jpeg_size = %d \r\n", jpeg_size);
   return jpeg_size;
}

//***************************************************************************
/**
*       Packet Size in Bytes (All Packets)
*/
static void stv_SetPacketSize(UInt16 packet_size)
{
//---------Set Interleave Packet Size (in bytes)
    packet_size = packet_size >> 1;
    stv_write(bPacketLength_lo,(UInt8)(packet_size & 0x00FF));      // SystemImageCharacteristicsControlParameters_bPacketLength_lo
    stv_write(bPacketLength_hi,(UInt8)(packet_size >> 8));          // SystemImageCharacteristicsControlParameters_bPacketLength_hi

//---------Set Jpeg Packet Size (in bytes)
    stv_write(bOifJpegPktSize_lo,(UInt8)(packet_size & 0x00FF));    // SystemConfigurationParameters_bOifJpegPktSize_lo
    stv_write(bOifJpegPktSize_hi,(UInt8)(packet_size >> 8));        // SystemConfigurationParameters_bOifJpegPktSize_hi
}

#if 0
//***************************************************************************
/**
*       Sensor Stream Mode for VB6850
*/
static HAL_CAM_Result_en_t stv_SensorStreamMode(void)
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
    UInt8 sensor_data[64];
    UInt16 bytes;
    UInt16 addr;
        
//----------Video Timing Registers
    bytes = 12;
    addr = 0x0300;
    result = SensorRead(bytes, addr, sensor_data);
    if (result == HAL_CAM_SUCCESS)
    {
        sensor_data[4] = 0x00;
        sensor_data[5] = 0x02;
        SensorWrite(bytes, addr, sensor_data);
    }

//----------Video Timing Registers
    bytes = 2;
    addr = 0x0100;
    result = SensorRead(bytes, addr, sensor_data);
    if (result == HAL_CAM_SUCCESS)
    {
        sensor_data[0] = 0x01;
        sensor_data[1] = 0x00;
        SensorWrite(bytes, addr, sensor_data);
    }
    return result;
}


//***************************************************************************
/**
*       Sensor Test Mode for VB6850
*/
static HAL_CAM_Result_en_t stv_SensorTestMode(void)
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
    UInt8 sensor_data[64];
    UInt16 bytes;
    UInt16 addr;
    UInt16 reg_val;
        
//----------Video Timing Registers
    bytes = 10;
    addr = 0x0600;
    reg_val = 1;                                                            // Test Pattern Mode
    sensor_data[0] = reg_val >> 8;
    sensor_data[1] = reg_val & 0xFF;
    reg_val = 0x0FFF;                                                   // Red Pixel Data
    sensor_data[2] = reg_val >> 8;
    sensor_data[3] = reg_val & 0xFF;
    reg_val = 0x0FFF;                                                   // GreenR Pixel Data
    sensor_data[4] = reg_val >> 8;
    sensor_data[5] = reg_val & 0xFF;
    reg_val = 0x0FFF;                                                   // Blue Pixel Data
    sensor_data[6] = reg_val >> 8;
    sensor_data[7] = reg_val & 0xFF;
    reg_val = 0x0FFF;                                                   // GreenB Pixel Data
    sensor_data[8] = reg_val >> 8;
    sensor_data[9] = reg_val & 0xFF;
    result = SensorWrite(bytes, addr, sensor_data);

    return result;
}
#endif

//***************************************************************************
/**
*       Sensor Status Register Read of VB6850
*/
#ifdef CAMDBG_STATUS
static void stv_Read_Sensor_Status(void)
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
    UInt8 sensor_data[64];
    UInt16 bytes = 16;
    UInt16 addr = 0;
    UInt16 register_value;
//----------Status  Registers
    result = SensorRead(bytes, addr, sensor_data);
    if (result == HAL_CAM_SUCCESS)
    {
        register_value = sensor_data[0] << 8 | sensor_data[1];
        pr_debug("stv_Read_Sensor_Status(): model id = %d \r\n", register_value);
        register_value = sensor_data[2];
        pr_debug("stv_Read_Sensor_Status(): revision # = %d \r\n", register_value);
        register_value = sensor_data[3];
        pr_debug("stv_Read_Sensor_Status(): manufacturer_id  = %d \r\n", register_value);
        register_value = sensor_data[4];
        pr_debug("stv_Read_Sensor_Status(): smia version = %d \r\n", register_value);
        register_value = sensor_data[5];
        pr_debug("stv_Read_Sensor_Status(): frame_count = %d \r\n", register_value);
    }
        
//----------Setup Registers
    bytes = 33;
    addr = 0x0100;
    result = SensorRead(bytes, addr, sensor_data);
    if (result == HAL_CAM_SUCCESS)
    {
        register_value = sensor_data[0];
        pr_debug("stv_Read_Sensor_Status(): mode select = %d \r\n", register_value);
        register_value = sensor_data[1];
        pr_debug("stv_Read_Sensor_Status(): image orientation = %d \r\n", register_value);
        register_value = sensor_data[3];
        pr_debug("stv_Read_Sensor_Status(): software reset = %d \r\n", register_value);
        register_value = sensor_data[4];
        pr_debug("stv_Read_Sensor_Status(): parameter hold = %d \r\n", register_value);
        register_value = sensor_data[16];
        pr_debug("stv_Read_Sensor_Status(): CCP2 channel identifier = %d \r\n", register_value);
        register_value = sensor_data[17];
        pr_debug("stv_Read_Sensor_Status(): CCP2 signalling mode = %d \r\n", register_value);
        register_value = (sensor_data[18] << 8) | sensor_data[19];
        pr_debug("stv_Read_Sensor_Status(): CCP2 data format = %d \r\n", register_value);
        register_value = sensor_data[32];
        pr_debug("stv_Read_Sensor_Status(): gain_mode = %d \r\n", register_value);
    }           
//----------Video Timing Registers
    bytes = 16;
    addr = 0x0300;
    result = SensorRead(bytes, addr, sensor_data);
    if (result == HAL_CAM_SUCCESS)
    {
        register_value = (sensor_data[0] << 8) | sensor_data[1];
        pr_debug("stv_Read_Sensor_Status(): vt_pix_clk_div = %d \r\n", register_value);
        register_value = (sensor_data[2] << 8) | sensor_data[3];
        pr_debug("stv_Read_Sensor_Status(): vt_sys_clk_div = %d \r\n", register_value);
        register_value = (sensor_data[4] << 8) | sensor_data[5];
        pr_debug("stv_Read_Sensor_Status(): pre_pll_clk_div = %d \r\n", register_value);
        register_value = (sensor_data[6] << 8) | sensor_data[7];
        pr_debug("stv_Read_Sensor_Status(): pll_multiplier = %d \r\n", register_value);
        register_value = (sensor_data[8] << 8) | sensor_data[9];
        pr_debug("stv_Read_Sensor_Status(): op_pix_clk_div = %d \r\n", register_value);
        register_value = (sensor_data[10] << 8) | sensor_data[11];
        pr_debug("stv_Read_Sensor_Status(): op_sys_clk_div = %d \r\n", register_value);
    }
}
#endif  

//****************************************************************************
//
// Function Name:   HAL_CAM_Result_en_t SensorSetSleepMode()
//
// Description: This function sets the ISP in Sleep Mode
//
// Notes:
//
//****************************************************************************
static HAL_CAM_Result_en_t SensorSetSleepMode(void)
{
    UInt8 register_value;
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;

//---------Read bSystemStatus   
    register_value = stv_read(bSystemStatus);       
    pr_debug("SensorSetSleepMode(): bSystemStatus = %d \r\n", register_value);
        
        if (register_value != STATE_SLEEP)      
        {
            if (register_value != STATE_IDLE)       
            {
        
           // bSystemControl possible values:    
           // 0x49=73   984 enters in STATE_UNINITIALIZED    
           // 0x39=57 984 enters in STATE_BOOT    
           // 0x11=17 984 enters in STATE_SLEEP    
           // 0x12=18 984 enters in STATE_IDLE    
           // 0x13=19 984 enters in STATE_VIEWFINDER    
           // 0x14=20 984 enters in STATE_STILLS    
           // 0x15=21 984 enters in STATE_MOVIE    
           // 0x16=22 984 enters in STATE_STILL_N_THUMBNAIL    
           // 0x18=24 984 enters in STATE_BUSY  
                stv_write(bSystemControl, STATE_IDLE);                              // SystemControlParameters_bSystemControl Idle mode
                stv_read(bSystemStatus);                                                        // read bSystemStatus for I2C write to complete
                result = stv_WaitValue(2000,bSystemStatus,STATE_IDLE);  // Poll system status to confirm Idle state.
                if (result != HAL_CAM_SUCCESS)
                {
                    pr_debug("SensorSetSleepMode(): STATE_IDLE:  ERROR: \r\n");
                }
            }
       // bSystemControl possible values:    
       // 0x49=73   984 enters in STATE_UNINITIALIZED    
       // 0x39=57 984 enters in STATE_BOOT    
       // 0x11=17 984 enters in STATE_SLEEP    
       // 0x12=18 984 enters in STATE_IDLE    
       // 0x13=19 984 enters in STATE_VIEWFINDER    
       // 0x14=20 984 enters in STATE_STILLS    
       // 0x15=21 984 enters in STATE_MOVIE    
       // 0x16=22 984 enters in STATE_STILL_N_THUMBNAIL    
       // 0x18=24 984 enters in STATE_BUSY  
            stv_write(bSystemControl, STATE_SLEEP);                              // SystemControlParameters_bSystemControl 
            result = stv_WaitValue(2000,bSystemStatus,STATE_SLEEP);  // Poll system status to confirm state.
            if (result != HAL_CAM_SUCCESS)
            {
                pr_debug("SensorSetSleepMode(): STATE_SLEEP:  ERROR: \r\n");
            }
        }
    return result;   
}   

//****************************************************************************
//
// Function Name:   HAL_CAM_Result_en_t SensorSetFrameRate(CamRates_t fps, CamSensorSelect_t sensor)
//
// Description: This function sets the frame rate of the Camera Sensor
//
// Notes:    15,20,25 fps are supported.
//
//****************************************************************************
static HAL_CAM_Result_en_t SensorSetFrameRate(CamRates_t fps, CamSensorSelect_t sensor)
{
    HAL_CAM_Result_en_t ret_val = HAL_CAM_SUCCESS;
    UInt16 fps_mpy, fps_div;

	pr_debug("SensorSetFrameRate(): fps = %d \r\n", fps);

    if (fps > CamRate_30)
    {
        ret_val = HAL_CAM_ERROR_ACTION_NOT_SUPPORTED;
    }
    else
    {
        if (fps == CamRate_Auto)
        {
            stv_write(0x4B80, 0x01); // AutomaticFrameRateControl_bMode
            stv_write(0x4C82, 0x0F); // StaticFrameRateControl_uwDesiredFrameRate_Num_LSByte
            stv_write(0x4C81, 0x00); // StaticFrameRateControl_uwDesiredFrameRate_Num_MSByte
            stv_write(0x4C84, 0x01); // StaticFrameRateControl_bDesiredFrameRate_Den
        }
        else
        {
            switch(fps)
            {
                case CamRate_4:                     // 4 fps
                    fps_mpy = 4;
                    fps_div = 1;
                    break;
                case CamRate_5:                     // 5 fps
                    fps_mpy = 5;
                    fps_div = 1;
                    break;
                case CamRate_10:                    // 10 fps
                    fps_mpy = 10;
                    fps_div = 1;
                    break;
                case CamRate_15:                    // 15 fps
                    fps_mpy = 15;
                    fps_div = 1;
                    break;
                case CamRate_20:                    // 20 fps
                    fps_mpy = 20;
                    fps_div = 1;
                    break;
                case CamRate_24:                    // 24 fps
                    fps_mpy = 24;
                    fps_div = 1;
                    break;
                case CamRate_25:                        // 25 fps
                    fps_mpy = 25;
                    fps_div = 1;
                    break;
                case CamRate_30:                        // 30 fps
                    fps_mpy = 30;
                    fps_div = 1;
                    break;
                default:                                        // Maximum Clock Rate = 26Mhz
                    fps_mpy = 15;
                    fps_div = 1;
                    ImageSettingsConfig_st.sensor_framerate->cur_setting = CamRate_15;
                    ret_val = HAL_CAM_ERROR_ACTION_NOT_SUPPORTED;
                    pr_debug("SensorSetFrameRate(): Frame Rate Unsupported:  ERROR: \r\n");
                    break;
            }
    // Choose Frame rate 0x1e = 30;  0x19 = 25; 0x0f = 15 
            stv_write(0x4B80, 0x00);                        // AutomaticFrameRateControl_bMode
            stv_write(0x4C82, fps_mpy);                 // StaticFrameRateControl_uwDesiredFrameRate_Num_LSByte
            stv_write(0x4C81, (fps_mpy >> 8));  // StaticFrameRateControl_uwDesiredFrameRate_Num_MSByte
            stv_write(0x4C84, fps_div);                 // StaticFrameRateControl_bDesiredFrameRate_Den

        }       // else (if (ImageSettingsConfig_st.sensor_framerate->cur_setting == CamRate_Auto))
        stv_read(bSystemStatus);                         // read bSystemStatus for I2C write to complete
    }       // else (if (fps <= CamRate_Auto))
    return ret_val;
}

//****************************************************************************
//
// Function Name:   HAL_CAM_Result_en_t SensorSetPreviewMode(CamImageSize_t image_resolution, CamDataFmt_t image_format)
//
// Description: This function sets up registers for camera preview resolution and format
//
// Notes:
//
//****************************************************************************
static HAL_CAM_Result_en_t SensorSetPreviewMode(
    CamImageSize_t image_resolution, 
    CamDataFmt_t image_format
    )
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
    UInt8 resolution, format;
    UInt16 peaking_gain;
//    UInt32 jpeg_file_size;

   // Choose resolution for Viewfinder   
   // 0x50 SQCIF        128x96  
   // 0x51 QQVGA        160x120
   // 0x52 QVGA         320x240    
   // 0x53 QCIF         174x144
   // 0x54 CIF          352x288    
   // 0x55 CUSTOM=XGA   1024x768
		pr_debug("SensorSetPreviewMode(): image_resolution = %d \r\n", image_resolution);

        switch(image_resolution)
        {
            case CamImageSize_SQCIF:        ///< 80 = image capture size 128x96
                resolution = 0x50;
                peaking_gain = 0x02;
//                jpeg_file_size = 10;            // 10 Kbytes
                break;
            case CamImageSize_QQVGA:        ///< 81 = image capture size 160x120
                resolution = 0x51;
                peaking_gain = 0x02;
//                jpeg_file_size = 10;            // 10 Kbytes
                break;
            case CamImageSize_QCIF:         ///< 83 = image capture size 176x144
                resolution = 0x53;
                peaking_gain = 0x02;
//                jpeg_file_size = 10;            // 10 Kbytes
                break;
            case CamImageSize_QVGA:         ///< 82 = image capture size 320x240
                resolution = 0x52;
                peaking_gain = 0x02;
//                jpeg_file_size = 20;            // 20 Kbytes
                break;
            case CamImageSize_CIF:          ///< 84 = image capture size 352x288
                resolution = 0x54;
                peaking_gain = 0x02;
//                jpeg_file_size = 26;            // 26 Kbytes
                break;
            case CamImageSize_XGA:          ///< 85 = CUSTOM=XGA        1024x768 
                resolution = 0x55;
                peaking_gain = 0x0B;
//                jpeg_file_size = 192;            // 10 Kbytes
                break;
            default:                        ///< 83 = image capture size 176x144
                resolution = 0x53;
                peaking_gain = 0x02;
//                jpeg_file_size = 10;            // 10 Kbytes
                break;
        }
        stv_write(bViewfinderStandardImageResolution, resolution);    //SystemImageCharacteristicsControlParameters_bViewfinderStandardImageResolution
        stv_write(bColourEngine1_bMaxGain, peaking_gain);                           // ColourEngine1_ApertureCorrectionControls bMaxGain
        pr_debug("SensorSetPreviewMode(): Preview resolution = 0x%x \r\n", resolution);

   // Choose Format for Viewfinder mode   
    // Set the format for the Viewfinder mode
    // 0x0072 RGB444
    // 0x0073 RGB565
    // 0x0074 RGB888
    // 0x0075 YUV420
    // 0x0076 YUV422
    // 0x0077 JPEG
		pr_debug("SensorSetPreviewMode(): image_format = %d \r\n", image_format);
        switch(image_format)
        {
            case CamDataFmtRGB444:              ///< 114 = capture RGB444
                format = 0x72;
                break;
            case CamDataFmtRGB888:              ///< 116 = capture RGB888
                format = 0x74;
                break;
            case CamDataFmtYUV:                     ///< 117 = capture YUV format 
                format = 0x75;
                break;
            case CamDataFmtRGB565:              ///< 115 = capture RGB565
    #if (defined (_BCM213x1_) || defined (_BCM21551_) || defined (CONFIG_ARCH_BCM116X))      // Internal ISP: use RGB565 from External ISP
                format = 0x73;
                break;
    #endif
            case CamDataFmtYCbCr:                   ///< 118 = capture YCbCr format
                format = 0x76;
                break;
            case CamDataFmtJPEG:                    ///< 119 = capture JPEG format   
                format = 0x77;
//              stv_write(bSqueezeSettings,0xA1); // JPEGImageCharateristicsControlParameters_bSqueezeSettings      
//              stv_write(bTargetFileSize_lo, (UInt8)jpeg_file_size);                   // JPEGImageCharateristicsControlParameters_bTargetFileSize_lo
//              stv_write( bTargetFileSize_hi, (UInt8)(jpeg_file_size >> 8) );  // JPEGImageCharateristicsControlParameters_bTargetFileSize_hi
                break;
            default:                                            ///< 118 = capture YCbCr format
                format = 0x76;
                break;
        }
   stv_write(bViewfinderImageFormat, format);    //SystemImageCharacteristicsControlParameters_bViewfinderImageFormat
   stv_read(bSystemStatus);                                     // read bSystemStatus for I2C write to complete
    pr_debug("SensorSetPreviewMode(): Preview format = 0x%x \r\n", format);
    return result;
}

//****************************************************************************
//
// Function Name:   HAL_CAM_Result_en_t SensorSetCaptureMode(CamImageSize_t image_resolution, CamDataFmt_t image_format)
//
// Description: This function sets up registers for camera Stills\Video capture, resolution and format
//
// Notes:
//
//****************************************************************************
static HAL_CAM_Result_en_t SensorSetCaptureMode(CamImageSize_t image_resolution, CamDataFmt_t image_format)
{
    UInt8 resolution, format;
    UInt16 peaking_gain;
    UInt32 jpeg_file_size;
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;

    stv_write(0x6212,sCamItuClkDiv); // bITUClockRatio: Pixel Clock == 40Mhz
    
  // Choose resolution  for Still/Movie modes   
   // 0x51=81           SQCIF       128x96  
   // 0x52=82           QQVGA       160x120
   // 0x53=83           QCIF        174x144
   // 0x54=84           QVGA        320x240    
   // 0x55=85           CIF         352x288    
   // 0x56=86           VGA         640x480    
   // 0x57=87           SVGA        800x600    
   // 0x58=88           SXGA        1280x1024  
   // 0x59=89           UXGA        1600x1200  
   // 0x60=96           CUSTOM=XGA  1024x768 
   // 0xA1=161          QXGA        2048x1536 
   // 0xA2=162          QSXGA       2560x2048  
        switch(image_resolution)
        {
            case CamImageSize_SQCIF:            // 0x51 SQCIF           128x96
                resolution = 0x51;
                peaking_gain = 0x02;
                jpeg_file_size = 10;            // 10 Kbytes
                break;
            case CamImageSize_QQVGA:            // 0x52 QQVGA           160x120
                resolution = 0x52;
                peaking_gain = 0x02;
                jpeg_file_size = 20;            // 20 Kbytes
                break;
            case CamImageSize_QCIF:             // 0x53 QCIF            176x144
                resolution = 0x53;
                peaking_gain = 0x02;
                jpeg_file_size = 20;            // 20 Kbytes
                break;
            case    CamImageSize_QVGA:          // 0x54 QVGA            320x240
                resolution = 0x54;
                peaking_gain = 0x02;
                jpeg_file_size = 40;            // 40 Kbytes
                break;
            case CamImageSize_CIF:              // 0x55 CIF             352x288    
                resolution = 0x55;
                peaking_gain = 0x02;
                jpeg_file_size = 40;            // 40 Kbytes
                break;
            case CamImageSize_VGA:              // 0x56 VGA             640x480 
                resolution = 0x56;
                peaking_gain = 0x04;
                jpeg_file_size = 80;            // 80 Kbytes
                break;
            case CamImageSize_SVGA:             // 0x57 SVGA            800x600 
                resolution = 0x57;
                peaking_gain = 0x05;
                jpeg_file_size = 120;           // 120 Kbytes
                break;
            case CamImageSize_XGA:              // 0x60 CUSTOM          1024x768
                resolution = 0x60;
                peaking_gain = 0x0B;
                jpeg_file_size = 192;           // 10 Kbytes
            // Increase ITU clock for if not outputting JPEG for this image size
                if (image_format != CamDataFmtJPEG)
                {
                    stv_write(0x6212,2);                                    // bITUClockRatio:      Pixel Clock == 80Mhz 
                }
                break;
            case CamImageSize_SXGA:             // 0x58 SXGA            1280x1024  (1.3MP)
                resolution = 0x58;
                peaking_gain = 0x0B;
                jpeg_file_size = 320;           // 320 Kbytes
                break;
            case CamImageSize_UXGA:             // 0x59 UXGA            1600x1200   (2MP)
                resolution = 0x59;
                peaking_gain = 0x14;
                jpeg_file_size = 480;           // 480 Kbytes
                break;
            case CamImageSize_QXGA:             // 0xA1 QXGA            2048x1536       (3MP) 
                resolution = 0xA1;
                peaking_gain = 0x14;
                jpeg_file_size = 750;           // 750 Kbytes
                break;
            case CamImageSize_QSXGA:            // 0xA2 QSXGA           2560x2048       (5MP) 
                resolution = 0xA2;
                peaking_gain = 0x14;
                jpeg_file_size = 1400;          // 1400 Kbytes
                break;
            default:                            // 0x53 QCIF            174x144
                resolution = 0x53;
                peaking_gain = 0x02;
                jpeg_file_size = 10;            // 10 Kbytes
                break;
        }
        pr_debug("SensorSetCaptureMode(): Capture resolution = 0x%x Peaking Gain=0x%x \r\n", resolution, peaking_gain);
        stv_write(bColourEngine0_bMaxGain, peaking_gain);                           // ColourEngine0_ApertureCorrectionControls bMaxGain
        stv_write(bStillStandardImageResolution, resolution);                    //SystemImageCharacteristicsControlParameters_bStillStandardImageResolution   

   //Choose Format For STILL/MOVIE    
   // 0x64 YUV420   
   // 0x65 YUV422   
   // 0x66 JPEG 16   
   // 0x67 JPEG_INTERLEAVE    
   // 0x68 RGB565  
        switch(image_format)
        {
            case CamDataFmtYUV:                     ///< 100 = capture YUV format 
                format = 0x64;
                break;
            case CamDataFmtRGB565:              ///< 104 = capture RGB565
    #if (defined (_BCM213x1_) || defined (_BCM21551_))      // Internal ISP: use RGB565 from External ISP
                format = 0x68;
                break;
    #endif
            case CamDataFmtYCbCr:                   ///< 101 = capture YCbCr format
                format = 0x65;
                break;
            case CamDataFmtJPEG:                    ///< 102 = capture JPEG format   
                format = 0x66;
                // JPEG Configuration Settings
                stv_write(bSqueezeSettings,0xA1); // JPEGImageCharateristicsControlParameters_bSqueezeSettings      
                stv_write(bTargetFileSize_lo, (UInt8)jpeg_file_size);                   // JPEGImageCharateristicsControlParameters_bTargetFileSize_lo
                stv_write( bTargetFileSize_hi, (UInt8)(jpeg_file_size >> 8) );  // JPEGImageCharateristicsControlParameters_bTargetFileSize_hi
                break;
            default:                                            ///< 101 = capture YCbCr format
                format = 0x65;
                break;
        }
    pr_debug("SensorSetCaptureMode(): Capture format = 0x%x \r\n", format);
    stv_write(bStillImageFormat, format);    //SystemImageCharacteristicsControlParameters_bStillImageFormat
    stv_read(bSystemStatus);                             // read bSystemStatus for I2C write to complete
    return result;
}

//****************************************************************************
//
// Function Name:   HAL_CAM_Result_en_t SensorSetInterLeaveMode(CamCaptureMode_t mode)
//
// Description: This function sets up registers for camera Stills\Video\Preview interleave
//
// Notes:
//
//****************************************************************************
static HAL_CAM_Result_en_t SensorSetInterLeaveMode(CamCaptureMode_t mode, CamDataFmt_t image_format)
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
    UInt8 reg_val;
    
    switch(mode)
    {
        case CamCaptureVideo:           
        case CamCaptureStill:          
        default:
            pr_debug("SensorSetInterLeaveMode(): Disable InterLeave \r\n");
            reg_val = DISABLE_INTERLEAVE;
            break;
        case CamCaptureStillnThumb:  
            pr_debug("SensorSetInterLeaveMode(): CamCaptureStillnThumb \r\n");
            if (CamSensorCfg_st.sensor_config_interleave_stills->mode == CamInterLeave_SingleFrame)
            {
                if ( image_format == CamDataFmtJPEG)
                {
                    reg_val = JPEG_INTERLEAVED_AT_END;
                    pr_debug("SensorSetInterLeaveMode(): CamCaptureStillnThumb=JPEG_INTERLEAVED_AT_END \r\n");
                }
                else
                {
                    reg_val = SINGLE_FRAME_INTERLEAVE;
                    pr_debug("SensorSetInterLeaveMode(): CamCaptureStillnThumb=SINGLE_FRAME_INTERLEAVE \r\n");
                }
            }
            else if (CamSensorCfg_st.sensor_config_interleave_stills->mode == CamInterLeave_AlternateFrame)
            {
                reg_val = ALTERNATE_FRAME_INTERLEAVE;
                pr_debug("SensorSetInterLeaveMode(): CamCaptureStillnThumb=CamInterLeave_AlternateFrame \r\n");
            }
            else
            {
                reg_val = DISABLE_INTERLEAVE;
                result |= HAL_CAM_ERROR_INTERNAL_ERROR;
                pr_debug("SensorSetInterLeaveMode(): CamCaptureStillnThumb=Error!!!  Invalid Setting \r\n");
            }
            break;
        case CamCaptureVideonViewFinder:              
            pr_debug("SensorSetInterLeaveMode(): CamCaptureVideonViewFinder \r\n");
            if (CamSensorCfg_st.sensor_config_interleave_video->mode == CamInterLeave_SingleFrame)
            {                 
                if ( image_format == CamDataFmtJPEG)
                {
                    reg_val = JPEG_INTERLEAVED_AT_END;
                    pr_debug("SensorSetInterLeaveMode(): CamCaptureVideonViewFinder=JPEG_INTERLEAVED_AT_END \r\n");
                }
                else
                {
                    reg_val = SINGLE_FRAME_INTERLEAVE;
                    pr_debug("SensorSetInterLeaveMode(): CamCaptureVideonViewFinder=SINGLE_FRAME_INTERLEAVE \r\n");
                }
            }
            else if (CamSensorCfg_st.sensor_config_interleave_video->mode == CamInterLeave_AlternateFrame)
            {
                reg_val = ALTERNATE_FRAME_INTERLEAVE;
                pr_debug("SensorSetInterLeaveMode(): CamCaptureVideonViewFinder=CamInterLeave_AlternateFrame \r\n");
            }
            else
            {
                reg_val = DISABLE_INTERLEAVE;
                result |= HAL_CAM_ERROR_INTERNAL_ERROR;
                pr_debug("SensorSetInterLeaveMode(): CamCaptureVideonViewFinder, Invalid Setting:  ERROR: \r\n");
             }
            break;
    }
    pr_debug("SensorSetInterLeaveMode(): SensorSetInterLeaveMode = 0x%x \r\n", reg_val);
    stv_write(bInterleavingMode, reg_val);              //SystemImageCharacteristicsControlParameters_bInterleavingMode
    stv_read(bSystemStatus);                            // read bSystemStatus for I2C write to complete
    return result;
}    

//****************************************************************************
//
// Function Name:   HAL_CAM_Result_en_t SetManualFocusCmd()
//
// Description: This function sets the Manual Focus Command
//
// Notes:
//
//****************************************************************************
static HAL_CAM_Result_en_t SetManualFocusCmd( UInt8 lens_command, UInt8 focus_mode, Boolean lens_moving )
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
    
    stv_write(bFocusMode, focus_mode);                      // FocusControls bFocusMode {FC_TLI_MODE_AF_SINGLE_FOCUS_HCS}
    stv_write(bLensCommand, lens_command);                  // FocusControls bLensCommand
    stv_read(bSystemStatus);                                // read bSystemStatus for I2C write to complete
    result = stv_WaitValue(2000, fLensIsMoving, 0x00);      // Poll FLADriverStatus_fLensIsMoving
    if (lens_moving)
    {
        result |= stv_WaitValue(2000, fLensIsMoving, 0x00);  // Poll FLADriverStatus_fLensIsMoving
        if (result != HAL_CAM_SUCCESS)
        {
            pr_debug("CAMDRV_SetManualFocusCmd(): FLADriverStatus_fLensIsMoving Failed:  ERROR: \r\n");
        }
    }
    stv_write(bLensCommand, LA_CMD_NULL);                  // FocusControls bLensCommand
    stv_read(bSystemStatus);                                // read bSystemStatus for I2C write to complete
    return result;
}

//****************************************************************************
//
// Function Name:   HAL_CAM_Result_en_t CAMDRV_SetAutoFocusCmd()
//
// Description: This function sets the AutoFocus Command
//
// Notes:
//
//****************************************************************************
static HAL_CAM_Result_en_t SetAutoFocusCmd( UInt8 af_command, UInt8 focus_mode, Boolean lens_moving )
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
    
    stv_write(bFocusMode, focus_mode);                  // FocusControls bFocusMode {FC_TLI_MODE_AF_SINGLE_FOCUS_HCS}
    stv_write(bAFCommand, af_command);                  // FocusControls bAFCommand {AF_TLI_CMD_HALF_BUTTON}
    stv_read(bSystemStatus);                            // read bSystemStatus for I2C write to complete
    mdelay(10);    // Time in ms      
    if (lens_moving)
    {
        result |= stv_WaitValue(2000, fLensIsMoving, 0x00);  // Poll FLADriverStatus_fLensIsMoving
        if (result != HAL_CAM_SUCCESS)
        {
            pr_debug("SetAutoFocusCmd(): FLADriverStatus_fLensIsMoving Failed:  ERROR: \r\n");
        }
        stv_write(bAFCommand, AF_TLI_CMD_NULL);             // FocusControls bAFCommand {AF_TLI_CMD_NULL}
    }
    stv_read(bSystemStatus);                            // read bSystemStatus for I2C write to complete
    return result;
}

// ===========================================
// 
//   AF Quick Search (HCS auto-focus
// 
// ===========================================

static HAL_CAM_Result_en_t AF_QuickSearch(void)
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;

#ifdef AUTOFOCUS_ENABLED

    stv_write(bFocusMode, FC_TLI_MODE_AF_SINGLE_FOCUS_HCS);     // FocusControls bFocusMode {FC_TLI_MODE_AF_SINGLE_FOCUS_HCS}
    stv_write(bAFCommand, AF_TLI_CMD_HALF_BUTTON);     // FocusControls bAFCommand {AF_TLI_CMD_HALF_BUTTON}
    stv_read(bSystemStatus);                                                            // read bSystemStatus for I2C write to complete
    mdelay(10);    // Time in ms      
    result = stv_WaitValue(2000, fLensIsMoving, 0x00); // Poll FLADriverStatus_fLensIsMoving
    if (result != HAL_CAM_SUCCESS)
    {
        pr_debug("AF_QuickSearch(): FLADriverStatus_fLensIsMoving Failed:  ERROR: \r\n");
    }
    stv_write(bAFCommand, AF_TLI_CMD_NULL);     // FocusControls bAFCommand {AF_TLI_CMD_NULL}
    stv_read(bSystemStatus);                                     // read bSystemStatus for I2C write to complete
#endif
    return result;
}

// ===========================================
// 
//   AF position sensor setup - Start 
// 
// ===========================================

static void AF_PositionSensorSetup(void)
{
    #ifdef AUTOFOCUS_ENABLED
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
// On starting streaming it is recommended execute sensor configuration and
// ps compensation by issuing an AF_CMD_AUTOCALIBRATE command
// it is also recommended to switch off the AE metering during this prosess



// set AE metering off
    stv_write(0x0180, 0x00);     // RunModeControl_fMeteringOn {0x180}

// AF Configuration and compensation 
    stv_write(0x5404, 0x00);     // FocusControls bFocusMode {FC_TLI_MODE_MANUAL_FOCUS}
    stv_write(0x529e, 0x00);     // FLADriverLowLevelParameters_fLowLevelDriverInitialized
    stv_write(bLensCommand, 0x09);     // FocusControls bLensCommand {LA_CMD_AUTOCALIBRATE}
    stv_read(bSystemStatus);     // read bSystemStatus for I2C write to complete
    mdelay(200); // This wait time should be set to twice the value written to 
// FLADriverLowLevelParameters_bCompensationImpulsesDuration_ms (0x52a0)

// the variable FLADriverLowLevelParameters_fLowLevelDriverInitialized
// will be set to 1 when the process has completed.
    result = stv_WaitValue(2000, 0x529e, 0x01); // Poll FLADriverLowLevelParameters_fLowLevelDriverInitialized
    if (result != HAL_CAM_SUCCESS)
    {
        pr_debug("AF_PositionSensorSetup(): fLowLevelDriverInitialized Failed:  ERROR: \r\n");
    }
// to confirm driver initialisation is completed.

// set AE metering on
    stv_write(0x0180, 0x01);     // RunModeControl_fMeteringOn {0x180}


// Start from a well defined position
// Go to the hyperfocal position (~250)
// FLADriverControls_wTargetPosition {0x5303, 0x5304}
    stv_write(0x5303, 0x0);      // FLADriverControls_wTargetPosition MSB
    stv_write(0x5304, 250);      // FLADriverControls_wTargetPosition LSB
    stv_write(bLensCommand, 0x07);     // FocusControls bLensCommand {LA_CMD_GOTO_TARGET_POSITION}
    stv_read(bSystemStatus);         // read bSystemStatus for I2C write to complete
    mdelay(100); 
    stv_WaitValue(2000, fLensIsMoving, 0x00); // Poll FLADriverStatus_fLensIsMoving
        #if 0       
        stv_write(0x5406, 0x01);     // FocusControls bAFCommand {AF_TLI_CMD_RELEASED_BUTTON}
        stv_WaitValue(2000, fLensIsMoving, 0x00); // Poll FLADriverStatus_fLensIsMoving
        stv_write(0x5404, 0x03);     // FocusControls bFocusMode {FC_TLI_MODE_AF_SINGLE_FOCUS_HCS}
        stv_write(0x5406, 0x02);     // FocusControls bAFCommand {AF_TLI_CMD_HALF_BUTTON}
        stv_read(bSystemStatus);        // read bSystemStatus for I2C write to complete
//        mdelay(1000); 
        stv_WaitValue(2000, fLensIsMoving, 0x00); // Poll FLADriverStatus_fLensIsMoving
        stv_write(0x5406, 0x01);     // FocusControls bAFCommand {AF_TLI_CMD_RELEASED_BUTTON}
        stv_read(bSystemStatus);         // read bSystemStatus for I2C write to complete
        #else
    AF_QuickSearch();
        #endif    
    #endif
    pr_debug("AF_PositionSensorSetup(): \r\n");
}
// ===========================================
// ===========================================

static HAL_CAM_Result_en_t SensorSetPowerDown(Boolean level)
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;

//---------Strobe Power Enable  
#if defined( GOLDFINGER )
    result = stv_GPIO (4, TRUE, level);
//  result = stv_GPIO (2, TRUE, level);
    mdelay( 25 );
#endif  
    return result;
}

HAL_CAM_Result_en_t stv_GPIO (UInt8 channel, UInt8 direction, UInt8 level)
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
    UInt8 gpio_config, gpio_direction, gpio_output;
    
    gpio_config = stv_read(GP_Channel_config);
    gpio_direction = stv_read(GP_direction);
    gpio_output = stv_read(GP_output);
    
    switch (channel)
    {
//-----------GPIO 4     
        case 4:
            stv_write(GP_Channel_config, (gpio_config | STV_GPIO_4));
            gpio_direction &= ~(STV_GPIO_4);
            if (direction)
            {
                gpio_direction |= STV_GPIO_4;
            }
            stv_write(GP_direction, gpio_direction);
            gpio_output &= ~(STV_GPIO_4);
            if (level)
            {
                gpio_output |= STV_GPIO_4;
            }
            stv_write(GP_output, gpio_output);
            break;
            
//-----------GPIO 3
        case 3:
            stv_write(GP_Channel_config, (gpio_config | STV_GPIO_3));
            gpio_direction &= ~(STV_GPIO_3);
            if (direction)
            {
                gpio_direction |= STV_GPIO_3;
            }
            stv_write(GP_direction, gpio_direction);
            gpio_output &= ~(STV_GPIO_3);
            if (level)
            {
                gpio_output |= STV_GPIO_3;
            }
            stv_write(GP_output, gpio_output);
            break;
//-----------GPIO 2
        case 2:
            stv_write(GP_Channel_config, (gpio_config | STV_GPIO_2));
            gpio_direction &= ~(STV_GPIO_2);
            if (direction)
            {
                gpio_direction |= STV_GPIO_2;
            }
            stv_write(GP_direction, gpio_direction);
            gpio_output &= ~(STV_GPIO_2);
            if (level)
            {
                gpio_output |= STV_GPIO_2;
            }
            stv_write(GP_output, gpio_output);
            break;
//-----------Undefined GPIO's
        default:
            result = HAL_CAM_ERROR_INTERNAL_ERROR;
            break;
        }
  stv_read(bSystemStatus);                                                              // read bSystemStatus for I2C write to complete
    return result;
}

#if 0
// ===========================================
// 
//  Zoom Manager Test Coin
// 
// ===========================================
static HAL_CAM_Result_en_t ZoomCommandWrite(void)
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
    UInt8 cmd_coin, status_coin;

// Write status coin
        status_coin = (stv_read(bDeviceTestCoin) & 0x0003);
// command coin == flipped status coin
        cmd_coin = TestCoin_Heads;
        if (status_coin == TestCoin_Heads)
        {
            cmd_coin = TestCoin_Tails;                                                      // flip command coin
        }
// Write command coin
        stv_write(bHostTestCoin,cmd_coin);                                          // ZoomMgrCtrl bHostTestCoin{TestCoin_Tails/TestCoin_Heads}
// Poll for status_coin == cmd_coin
        result = stv_WaitValue(2000,bDeviceTestCoin,cmd_coin);  // ZoomMgrStatus [read only] bDeviceTestCoin
// Reset status coin & update data byte, byte count, addr
        status_coin = cmd_coin;

    if (result != HAL_CAM_SUCCESS)
    {
        pr_debug("ZoomCommandWrite(): ERROR: \r\n");
    }
    else
    {
        pr_debug("ZoomCommandWrite(): Complete OK \r\n");
    }
    return result;
}
// 
// ===========================================
// ===========================================
#endif


// ===========================================
// 
//  Sensor Write Thru Host
// 
// ===========================================
static HAL_CAM_Result_en_t SensorWrite(UInt16 byte_count, UInt16 addr, UInt8 *sensor_data)
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
    UInt8 cmd_coin, status_coin;
    UInt16 count = 0;

            
    while ( (result == HAL_CAM_SUCCESS) && (count < byte_count) )
    {
// Write status coin
    status_coin = (stv_read(bStatusCoin) & 0x0003);
// command coin == flipped status coin
        cmd_coin = 1;
        if (status_coin == 1)
        {
            cmd_coin = 2;                                                                           // flip command coin
        }
// Set Write Addr
        stv_write(bRequest,0x03);                                                   // HostToSensorAccessControl bRequest {HostToSensor_stv_write}
        stv_write(uwSensorIndex_MS,(addr >> 8));                        // HostToSensorAccessControl uwSensorIndex MSB
        stv_write(uwSensorIndex_LS,addr);                                   // HostToSensorAccessControl uwSensorIndex LSB
        stv_write(uwDataLow_MS,0x00);                                               // HostToSensorAccessData uwDataLow MSB
        stv_write(uwDataLow_LS,(*sensor_data++ & 0x00FF));  // HostToSensorAccessData uwDataLow LSB
// Write command coin
        stv_write(bCommandCoin,cmd_coin);                                   // HostToSensorAccessControl bCommandCoin {TestCoin_Tails/TestCoin_Heads}
// Poll for status_coin == cmd_coin
        result = stv_WaitValue(2000,bStatusCoin,cmd_coin);  // HostToSensorAccessStatus [read only] bStatusCoin {TestCoin_Tails}
// Reset status coin & update data byte, byte count, addr
        status_coin = cmd_coin;
        count++;
        addr++;
    }
    if (result != HAL_CAM_SUCCESS)
    {
        pr_debug("SensorWrite(): ERROR: count write = %d byte_count = %d \r\n", count,byte_count);
    }
    else
    {
        pr_debug("SensorWrite(): Complete OK \r\n");
    }
    return result;
}
// 
// ===========================================
// ===========================================

// ===========================================
// 
//  Sensor Read Thru Host
// 
// ===========================================
static HAL_CAM_Result_en_t SensorRead(UInt16 byte_count, UInt16 addr, UInt8 *sensor_data)
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
    UInt8 cmd_coin, status_coin;
    UInt16 count = 0;

            
    while ( (result == HAL_CAM_SUCCESS) && (count < byte_count) )
    {
// Read status coin
    status_coin = (stv_read(bStatusCoin) & 0x0003);
// command coin == flipped status coin
        cmd_coin = 1;
        if (status_coin == 1)
        {
            cmd_coin = 2;                                                                       // flip command coin
        }
// Set Read Addr
        stv_write(bRequest,0x00);                                               // HostToSensorAccessControl bRequest {HostToSensor_ReadByte}
        stv_write(uwSensorIndex_MS,(addr >> 8));                    // HostToSensorAccessControl uwSensorIndex MSB
        stv_write(uwSensorIndex_LS,addr);                               // HostToSensorAccessControl uwSensorIndex LSB
// Write command coin
        stv_write(bCommandCoin,cmd_coin);                               // HostToSensorAccessControl bCommandCoin {TestCoin_Tails/TestCoin_Heads}
// Poll for status_coin == cmd_coin
        result = stv_WaitValue(2000,bStatusCoin,cmd_coin); // HostToSensorAccessStatus [read only] bStatusCoin {TestCoin_Tails}
// Reset status coin & update data byte, byte count, addr
        status_coin = cmd_coin;
        *sensor_data++ = (stv_read(uwDataLow_LS) & 0x00FF);
        count++;
        addr++;
    }
    if (result != HAL_CAM_SUCCESS)
    {
        pr_debug("SensorRead(): ERROR: count read = %d byte_count = %d \r\n", count,byte_count);
    }
    else
    {
        pr_debug("SensorRead(): Complete OK \r\n");
    }
    return result;
}
// 
// ===========================================
// ===========================================


//***************************************************************************
/**
*        Sensor Patch
*/
//***********************************************
//Personality file written by bryan taylor
//Date: 27-Feb-2007 20:05:00
//Generated for 850 rev 2
//***********************************************
static HAL_CAM_Result_en_t SensorPatch_VB6850(void)
{

    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
    UInt8 sensor_data[8];
    UInt16 bytes;
    UInt16 addr;

    #if 0
   // Sensor Reset
//----------Read Sensor Registers for Reset
// The host to sensor access page: Index:0x0103 Value:0x01
    bytes = 2;
    addr = 0x0102;
    if (result == HAL_CAM_SUCCESS)
    {
        result = SensorRead(bytes, addr, sensor_data);
    }
    if (result == HAL_CAM_SUCCESS)
    {
//      sensor_data[0] = 0x00;                                              // Index:0x0102 Value:0x00
        sensor_data[1] = 0x01;                                              // Index:0x0103 Value:0x01
        SensorWrite(bytes, addr, sensor_data);
    }
        
        
//----------Read Sensor Registers for Patch bank 1
// The host to sensor access page: Index:0x3112 Value:0x24
// The host to sensor access page: Index:0x3114 Value:0xe3
// The host to sensor access page: Index:0x3116 Value:0x00
// The host to sensor access page: Index:0x3117 Value:0x05
    bytes = 8;
    addr = 0x3110;
    if (result == HAL_CAM_SUCCESS)
    {
        result = SensorRead(bytes, addr, sensor_data);
    }
    if (result == HAL_CAM_SUCCESS)
    {
        sensor_data[2] = 0x24;                                              // Index:0x3112 Value:0x24
        sensor_data[4] = 0xE3;                                              // Index:0x3114 Value:0xe3
        sensor_data[6] = 0x00;                                              // Index:0x3116 Value:0x00
        sensor_data[7] = 0x05;                                              // Index:0x3117 Value:0x05
        SensorWrite(bytes, addr, sensor_data);
    }

//----------Read Sensor Registers for Patch bank 2
// The host to sensor access page: Index:0x31c0 Value:0x00
// The host to sensor access page: Index:0x31c1 Value:0x7d
        bytes = 2;
        addr = 0x31C0;
    if (result == HAL_CAM_SUCCESS)
    {
        result = SensorRead(bytes, addr, sensor_data);
    }
    if (result == HAL_CAM_SUCCESS)
    {
        sensor_data[0] = 0x00;                                              // Index:0x31c0 Value:0x00
        sensor_data[1] = 0x7D;                                              // Index:0x31c1 Value:0x7d
        SensorWrite(bytes, addr, sensor_data);
    }
//----------Read Sensor Registers for Patch bank 3
// The host to sensor access page: Index:0x31e1 Value:0x5f
            bytes = 2;
            addr = 0x31E0;
    if (result == HAL_CAM_SUCCESS)
    {
        result = SensorRead(bytes, addr, sensor_data);
    }
    if (result == HAL_CAM_SUCCESS)
    {
        sensor_data[1] = 0x5F;                                              // Index:0x31e1 Value:0x5f
        SensorWrite(bytes, addr, sensor_data);
    }

    #endif
//----------Read Sensor Registers for ARCTIC_CONTROL arctic_enable {VPIP_TRUE}
// The host to sensor access page: Index:0x3204 Value:0x01
            bytes = 2;
            addr = 0x3204;
    if (result == HAL_CAM_SUCCESS)
    {
        result = SensorRead(bytes, addr, sensor_data);
    }
    if (result == HAL_CAM_SUCCESS)
    {
        sensor_data[0] = 0x01;                                              // Index:0x3204 Value:0x01
        SensorWrite(bytes, addr, sensor_data);
    }
    
//----------Read Sensor Registers for ARCTIC_CONTROL bruce_enable {VPIP_TRUE}
// The host to sensor access page: Index:0x3210 Value:0x01
            bytes = 2;
            addr = 0x3210;
    if (result == HAL_CAM_SUCCESS)
    {
        result = SensorRead(bytes, addr, sensor_data);
    }
    if (result == HAL_CAM_SUCCESS)
    {
        sensor_data[0] = 0x01;                                              // Index:0x3210 Value:0x01
        SensorWrite(bytes, addr, sensor_data);
    }
    pr_debug("SensorPatch_VB6850(): result=%d \r\n", result);
    return result;
}
// ===========================================
// ===========================================




//***************************************************************************
/**
*        Temporary for debugging Only
*/
//***********************************************
#if 0
//****************************************************************************
//
// Function Name:   HAL_CAM_Result_en_t CAMDRV_SetZoom(UInt8 numer, UInt8 denum)
//
// Description: This function performs zooming via camera sensor
//
// Notes:
//
//****************************************************************************
HAL_CAM_Result_en_t CAMDRV_SetZoom(UInt16 step)
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
    UInt8 zoom_status;
    UInt32 fovx, fovy;
    float *fovx_ptr, *fovy_ptr;
    float zoom_fovx;

// --------Calculate new Zoomed fovx    
    zoom_fovx = ( (float)CAM_MAX_PIXEL_X * (float)step ) / (float)SDF_MAX_SCALE;                            // new zoomed fovx value

//---------Read current fovx
    fovx = (UInt32)(stv_read(fFOVX) << 24);
    fovx |= (UInt32)(stv_read(fFOVX+2) << 16);
    fovx_ptr = (float *)&fovx;                                                              // float convesion of fovx
    pr_debug("CAMDRV_SetZoom(): Before: zoom_fovx=%f  fovx=0x%x (float)fovx=%f fovy=0x%x (float)fovy=%f \r\n", zoom_fovx, fovx, *fovx_ptr, fovy, *fovy_ptr);

//--------- Enable AutoZoom
  stv_write(fAutoZoom, TRUE);                            // ZoomMgrCtrl_fAutoZoom=enabled
  stv_write(bMagFactor, 100);                            // ZoomMgrCtrl_bMagFactor = 100/1000

//--------- No Zoom if zoom_fovx = max fov
    if (zoom_fovx == (float)CAM_MAX_PIXEL_X)
    {
      stv_write(bZoomCmd, ZOOM_RESET);                     // ZoomMgrCtrl_bZoomCmd=zoom reset
      result = ZoomCommandWrite();
    }
//--------- Zoom in till reach new zoomed fovx
    else if (zoom_fovx < *fovx_ptr)
    {
      stv_write(bZoomCmd, ZOOM_IN);                          // ZoomMgrCtrl_bZoomCmd=zoom in
      result = ZoomCommandWrite();
      do
      {
        mdelay(5);    // Time in ms      
            fovx = (UInt32)(stv_read(fFOVX) << 24);
            fovx |= (UInt32)(stv_read(fFOVX+2) << 16);
        zoom_status = stv_read(bZoomOpStatus);
        }while ( (zoom_fovx < *fovx_ptr) && (zoom_status != FullyZoomedIn) );
    }
//--------- Zoom out till reach new zoomed fovx
    else if (zoom_fovx > *fovx_ptr)
    {
      stv_write(bZoomCmd, ZOOM_OUT);                         // ZoomMgrCtrl_bZoomCmd=zoom out
      result = ZoomCommandWrite();
      do
      {
        mdelay(5);    // Time in ms      
            fovx = (UInt32)(stv_read(fFOVX) << 24);
            fovx |= (UInt32)(stv_read(fFOVX+2) << 16);
        zoom_status = stv_read(bZoomOpStatus);
        }while ( (zoom_fovx > *fovx_ptr) && (zoom_status |= FullyZoomedOut) );
    }
  stv_write(bZoomCmd, NO_ZOOM_COMMAND);                    // ZoomMgrCtrl_bZoomCmd=zoom stop
    result = ZoomCommandWrite();
    zoom_status = stv_read(bZoomOpStatus);

    fovx = (UInt32)(stv_read(fFOVX) << 24);
    fovx |= (UInt32)(stv_read(fFOVX+2) << 16);

    fovy = (UInt32)(stv_read(fFOVY) << 24);
    fovy |= (UInt32)(stv_read(fFOVY+2) << 16);
    fovy_ptr = (float *)&fovy;                                                              // float convesion of fovy
    
    
    #if 0   
  stv_write(fAutoZoom, FALSE);                           // ZoomMgrCtrl_fAutoZoom=disabled
  stv_write(bZoomCmd, ZOOM_RESET);                       // ZoomMgrCtrl_bZoomCmd=reset zoom
  result = ZoomCommandWrite();
  
  if (result == HAL_CAM_SUCCESS)
  {
    if (step != CamZoom_1_0)
    {
      stv_write(bZoomCmd, ZOOM_IN);                      // ZoomMgrCtrl_bZoomCmd
      stv_write(bMagFactor, MagFactor_1_15);             // ZoomMgrCtrl_bMagFactor
      result |= ZoomCommandWrite();
      if (step != CamZoom_1_15)
      {
        stv_write(bMagFactor, MagFactor_1_33);           // ZoomMgrCtrl_bMagFactor
        result |= ZoomCommandWrite();
        if (step != CamZoom_1_33)
        {
          stv_write(bMagFactor, MagFactor_1_6);          // ZoomMgrCtrl_bMagFactor
          result |= ZoomCommandWrite();
            if (step != CamZoom_1_6)
            {
            stv_write(bMagFactor, MagFactor_2_0);        // ZoomMgrCtrl_bMagFactor
            result |= ZoomCommandWrite();
              if (step != CamZoom_2_0)
              {
              stv_write(bMagFactor, MagFactor_2_66);     // ZoomMgrCtrl_bMagFactor
              result |= ZoomCommandWrite();
                if (step != CamZoom_2_66)
                {
                stv_write(bMagFactor, MagFactor_4_0);    // ZoomMgrCtrl_bMagFactor
                result |= ZoomCommandWrite();
              }  // if (step != CamZoom_2_66)
            }  // if (step != CamZoom_2_0
          }  // if (step != CamZoom_1_6)
        }  // if (step != CamZoom_1_33)
      }  // if (step != CamZoom_1_15)
    }  // if (step != CamZoom_1_0)
  }  // if (result == HAL_CAM_SUCCESS)    

    fovx = (UInt32)(stv_read(fFOVX) << 24);
    fovx |= (UInt32)(stv_read(fFOVX+2) << 16);

    fovy = (UInt32)(stv_read(fFOVY) << 24);
    fovy |= (UInt32)(stv_read(fFOVY+2) << 16);
    
    fovx_ptr = (float *)&fovx;
    fovy_ptr = (float *)&fovy;
    #endif

    pr_debug("CAMDRV_SetZoom(): After: fovx=0x%x (float)fovx=%f fovy=0x%x (float)fovy=%f zoom_status=%d \r\n", fovx, *fovx_ptr, fovy, *fovy_ptr, zoom_status);
    return result;
}

//****************************************************************************
//
// Function Name:   HAL_CAM_Result_en_t CAMDRV_SetZoom(UInt8 numer, UInt8 denum)
//
// Description: This function performs zooming via camera sensor
//
// Notes:
//
//****************************************************************************
HAL_CAM_Result_en_t CAMDRV_SetZoom(UInt16 step)
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;

  stv_write(fAutoZoom, FALSE);                           // ZoomMgrCtrl_fAutoZoom=disabled
  stv_write(bZoomCmd, ZOOM_RESET);                       // ZoomMgrCtrl_bZoomCmd=reset zoom
  result = ZoomCommandWrite();
  
  if (result == HAL_CAM_SUCCESS)
  {
    if (step != CamZoom_1_0)
    {
      stv_write(bZoomCmd, ZOOM_IN);                      // ZoomMgrCtrl_bZoomCmd
      stv_write(bMagFactor, MagFactor_1_15);             // ZoomMgrCtrl_bMagFactor
      result |= ZoomCommandWrite();
      if (step != CamZoom_1_15)
      {
        stv_write(bMagFactor, MagFactor_1_33);           // ZoomMgrCtrl_bMagFactor
        result |= ZoomCommandWrite();
        if (step != CamZoom_1_33)
        {
          stv_write(bMagFactor, MagFactor_1_6);          // ZoomMgrCtrl_bMagFactor
          result |= ZoomCommandWrite();
            if (step != CamZoom_1_6)
            {
            stv_write(bMagFactor, MagFactor_2_0);        // ZoomMgrCtrl_bMagFactor
            result |= ZoomCommandWrite();
              if (step != CamZoom_2_0)
              {
              stv_write(bMagFactor, MagFactor_2_66);     // ZoomMgrCtrl_bMagFactor
              result |= ZoomCommandWrite();
                if (step != CamZoom_2_66)
                {
                stv_write(bMagFactor, MagFactor_4_0);    // ZoomMgrCtrl_bMagFactor
                result |= ZoomCommandWrite();
              }  // if (step != CamZoom_2_66)
            }  // if (step != CamZoom_2_0
          }  // if (step != CamZoom_1_6)
        }  // if (step != CamZoom_1_33)
      }  // if (step != CamZoom_1_15)
    }  // if (step != CamZoom_1_0)
  }  // if (result == HAL_CAM_SUCCESS)    
    return result;
}
#endif


    #if 1
#include "camdrv_stv0986_patch2_20.c"
    #endif
    
