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
*   @file   camdrv_OV2655
*
*   @brief  This file is the lower level driver API of OV2655(2M 1600*1200 Pixel) ISP/sensor.
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
  
#include <mach/reg_sys.h>
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


#define OV_DEVICE_ID	0x2656  //2655:0x2656  2650:0x2652

//GPIO
#define HAL_CAM_RESET 22
#define	HAL_CAM_PD	  23


//******************************************************************************
// start of CAM configuration
//******************************************************************************
/** Sensor select */
typedef enum
{
    SensorOV2655,
    SensorUnused
} CamSensor_t;


//************************************************************************************
//  Sensor Resolution Tables: 
//************************************************************************************
// Resolutions & Sizes available for OV2655 ISP/Sensor (QXGA max)
static CamResolutionConfigure_t  sSensorResInfo_OV2655_st[] =
{
// width    height  Preview_Capture_Index       Still_Capture_Index
  {128,     96,     -1,                         -1                  },      //  CamImageSize_SQCIF
  {160,     120,    -1,                         -1                  },      //  CamImageSize_QQVGA
  {176,     144,    CamImageSize_QCIF,          CamImageSize_QCIF   },      //  CamImageSize_QCIF
  {240,     180,    -1,                         -1                  },      //  CamImageSize_240x180
  {240,     320,    CamImageSize_R_QVGA, CamImageSize_R_QVGA        },      //  CamImageSize_R_QVGA
  {320,     240,    CamImageSize_QVGA,          CamImageSize_QVGA   },      //  CamImageSize_QVGA
  {352,     288,    CamImageSize_CIF,           CamImageSize_CIF    },      //  CamImageSize_CIF
  {426,     320,    -1,                         -1                  },      //  CamImageSize_426x320
  {640,     480,    -1,                         CamImageSize_VGA    },      //  CamImageSize_VGA
  {800,     600,    -1,                         CamImageSize_SVGA   },      //  CamImageSize_SVGA 
  {1024,    768,    -1,                         -1                  },      //  CamImageSize_XGA 
  {1280,    960,    -1,                         CamImageSize_4VGA   },      //  CamImageSize_4VGA
  {1280,    1024,   -1,                         CamImageSize_SXGA   },      //  CamImageSize_SXGA
  {1600,    1200,   -1,                         CamImageSize_UXGA   },      //  CamImageSize_UXGA
  {2048,    1536,   -1,                         -1                  },      //  CamImageSize_QXGA
  {2560,    2048,   -1,                         -1                  },       //  CamImageSize_QSXGA
  {144,	  176,   CamImageSize_R_QCIF,	CamImageSize_R_QCIF  } 	  //  CamImageSize_R_QCIF	
};


//************************************************************************************
//  Power On/Off Tables for Main Sensor
//************************************************************************************

//---------Sensor Power On
static CamSensorIntfCntrl_st_t  CamPowerOnSeq[] = 
{
// -------Turn everything OFF   
    {PAUSE, 	 10, 		   Nop_Cmd},
    {GPIO_CNTRL, HAL_CAM_RESET,    GPIO_SetLow},
    {MCLK_CNTRL, CamDrv_NO_CLK,    CLK_TurnOff},
    {GPIO_CNTRL, HAL_CAM_PD,       GPIO_SetLow},
    {PAUSE,      50,               Nop_Cmd},
// -------Disable Reset 
    {GPIO_CNTRL, HAL_CAM_RESET,    GPIO_SetHigh},
// -------Enable Clock to Cameras @ Main clock speed
    {MCLK_CNTRL, CamDrv_24MHz,     CLK_TurnOn},
    {PAUSE,      100,                Nop_Cmd}
};

//---------Sensor Power Off
static CamSensorIntfCntrl_st_t  CamPowerOffSeq[] = 
{
// -------Disable Clock to Cameras 
	{MCLK_CNTRL, CamDrv_NO_CLK,    CLK_TurnOff},
// -------Lower Reset to ISP,Enable Reset    
    {GPIO_CNTRL, HAL_CAM_RESET,    GPIO_SetLow},
    {PAUSE,      5,                Nop_Cmd},
// -------Turn Power OFF    
    {GPIO_CNTRL, HAL_CAM_PD,       GPIO_SetLow},
    {PAUSE,      50,               Nop_Cmd}
};

//---------wake up from Sensor Power down
static CamSensorIntfCntrl_st_t	WakeupformPowerDownSeq[] = 
{
	{PAUSE, 	 10,			   Nop_Cmd},
// -------Ensable Clock to Cameras 
	{MCLK_CNTRL, CamDrv_24MHz,    CLK_TurnOn},
	{PAUSE, 	 1, 			   Nop_Cmd},
// -------Turn Power on 	
	{GPIO_CNTRL, HAL_CAM_PD,      GPIO_SetLow},
	{PAUSE, 	 5, 			   Nop_Cmd},

};

//--------- Sensor Power down
static CamSensorIntfCntrl_st_t	CamPowerDownSeq[] = 
{
	// -------Turn power off
	{GPIO_CNTRL, HAL_CAM_PD,    GPIO_SetHigh},
	{PAUSE, 	 5, 			   Nop_Cmd},

	// -------Disable Clock to Cameras 
	{MCLK_CNTRL, CamDrv_NO_CLK,    CLK_TurnOff},
	{PAUSE, 	 5, 			   Nop_Cmd},

};


/** Primary Sensor Configuration and Capabilities  */
static HAL_CAM_IntConfigCaps_st_t CamPrimaryCfgCap_st = 
{
    // CamSensorOutputCaps_st_t
    {
        CamDataFmtYCbCr |               ///< UInt32 formats;   CamDataFmt_t bit masked
        CamDataFmtRGB565 ,
        1600,                           ///< UInt32 max_width;   Maximum width resolution
        1200,                           ///< UInt32 max_height;  Maximum height resolution
        CamImageSize_QVGA,               ///< UInt32 ViewFinderResolutions;  ViewFinder Resolutions (Maximum Resolution for now)
        CamImageSize_QVGA,               ///< UInt32 VideoResolutions;  Video Resolutions (Maximum Resolution for now)
        CamImageSize_UXGA,              ///< UInt32 StillsResolutions;  Stills Resolutions (Maximum Resolution for now) 
        3,                              ///< UInt32 pre_frame_video;  frames to throw out for ViewFinder/Video capture (total= pre_frame_video+1
        8,                             ///< UInt32 pre_frame_still;  frames to throw out for Stills capture (total= pre_frame_still+1
        FALSE,                           ///< Boolean JpegCapable;     Sensor Jpeg Capable: TRUE/FALSE:
        FALSE,                           ///< Boolean StillnThumbCapable;     Sensor Still and Thumbnail Capable: TRUE/FALSE:
        TRUE                            ///< Boolean VideonViewfinderCapable;     Sensor Video and Viewfinder Capable: TRUE/FALSE:
    },

    {
        CamFocusControlOff,             ///< CamFocusControlMode_t default_setting=CamFocusControlOff;
        CamFocusControlOff,             ///< CamFocusControlMode_t cur_setting;
        CamFocusControlOff              ///< UInt32 settings;  Settings Allowed: CamFocusControlMode_t bit masked
    },

    ///< Digital Zoom Settings & Capabilities:  CamDigitalZoomMode_st_t digital_zoom_st;      
    {
        CamZoom_1_0,                    ///< CamZoom_t default_setting;  default=CamZoom_1_0:  Values allowed  CamZoom_t
        CamZoom_1_0,                    ///< CamZoom_t cur_setting;  CamZoom_t
        CamZoom_1_0,                    ///< CamZoom_t max_zoom;  Max Zoom Allowed (256/max_zoom = *zoom)
        FALSE                            ///< Boolean capable;  Sensor capable: TRUE/FALSE:
    },
    
    ///< Sensor ESD Settings & Capabilities:  CamESD_st_t esd_st;    
    {
        0x01,                           ///< UInt8 ESDTimer;  Periodic timer to retrieve the camera status (ms)
        FALSE                           ///< Boolean capable;  TRUE/FALSE:
    }, 

    ///< Sensor version string
    "OV2655"
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
    SensorYCSeq_YCbYCr,     //[01] CamSensorYCbCrSeq_t sensor_yc_seq;    (default) SensorYCSeq_YCbYCr                                  
    

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
    
    7 << 12,		    //[06] UInt32 i2c_pwrup_drive_strength;   (default)IOCR4_CAM_DR_12mA:   IOCR drive strength
    0x00,               //[07] UInt32 i2c_pwrdn_drive_strength;   (default)0x00:    I2C2 disabled
    0x00,               //[08] UInt32 i2c_slew;                           (default) 0x00: 42ns

    7 << 12,		    //[09] UInt32 cam_pads_pwrup_drive_strength;   (default)IOCR4_CAM_DR_12mA:  IOCR drive strength
    1 << 12 		    //[10] UInt32 cam_pads_pwrdn_drive_strength;   (default)IOCR4_CAM_DR_2mA:   IOCR drive strength
};


// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX IMPORTANT XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
// TO DO: MURALI
// HAVE TO PROGRAM THIS IN THE ISP.
//---------Sensor Primary Configuration JPEG
static CamIntfConfig_Jpeg_st_t CamPrimaryCfg_Jpeg_st = 
{
    512,                            ///< UInt32 jpeg_packet_size_bytes;     Bytes/Hsync
    1536,                           ///< UInt32 jpeg_max_packets;           Max Hsyncs/Vsync = (3.2Mpixels/4) / 512
    CamJpeg_FixedPkt_VarLine,       ///< CamJpegPacketFormat_t pkt_format;  Jpeg Packet Format
};


// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX IMPORTANT XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
// TO DO: MURALI
// WILL NEED TO MODIFY THIS.
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
    NULL,                               // *sensor_config_interleave_video;
    NULL,                               // *sensor_config_interleave_stills;
    &CamPrimaryCfg_PktMarkerInfo_st     // *sensor_config_pkt_marker_info;
};

// --------Primary Sensor Frame Rate Settings
static CamFrameRate_st_t PrimaryFrameRate_st =
{
    CamRate_30,                     ///< CamRates_t default_setting; 
    CamRate_30,                     ///< CamRates_t cur_setting; 
    CamRate_30                      ///< CamRates_t max_setting;
};

typedef enum
{
	CAM_BL_POS_4_3 = 100,	
	CAM_BL_POS_3_3 =  75, 
	CAM_BL_POS_2_3 =  50,
	CAM_BL_POS_1_3 =  25,
	CAM_BL_ZERO	   =   0,
	CAM_BL_NEG_1_3 = -25,
	CAM_BL_NEG_2_3 = -50,
	CAM_BL_NEG_3_3 = -75,
	CAM_BL_NEG_4_3 = -100
} CamBrightness_OV2655_t;
	


typedef enum
{
	CAM_NEG_4_3 = 100,	
	CAM_NEG_3_3 =  75, 
	CAM_NEG_2_3 =  50,
	CAM_NEG_1_3 =  25,
	CAM_ZERO	=   0,
	CAM_POS_1_3 = -25,
	CAM_POS_2_3 = -50,
	CAM_POS_3_3 = -75,
	CAM_POS_4_3 = -100
} CamSaturation_OV2655_t;


typedef enum
{	
	CAM_Contrast_POS_3 =  75, 
	CAM_Contrast_POS_2 =  50,
	CAM_Contrast_POS_1 =  25,
	CAM_Contrast_ZERO  =   0,
	CAM_Contrast_NEG_1 = -25,
	CAM_Contrast_NEG_2 = -50,
	CAM_Contrast_NEG_3 = -75,	  
} CamContrast_OV2655_t;



//****************************************************************************
//							OV2655 Register Defines
//****************************************************************************



//--- Sensor Image_Orientation (SMIA Defined Registers)-----------------------		  
#define uwDeviceId_MS									0x300A			// SystemDeviceParameters_uwDeviceId
#define uwDeviceId_LS									0x300B			// SystemDeviceParameters_uwDeviceId

#define g_Capture_PCLK_Frequency	  36
#define FULL_PERIOD_PIXEL_NUMS          1940
#define FULL_EXPOSURE_LIMITATION        1236
#define PV_PERIOD_PIXEL_NUM              970
#define g_Capture_Max_Gain16          (8*16)




// I2C transaction result
static HAL_CAM_Result_en_t sCamI2cStatus = HAL_CAM_SUCCESS; 

static HAL_CAM_Result_en_t CheckCameraDeviceID(CamSensor_t sensor_sel);
static HAL_CAM_Result_en_t	SensorSetPreviewMode(CamImageSize_t image_resolution, CamDataFmt_t image_format);
static HAL_CAM_Result_en_t	SensorSetSleepMode(void);
static HAL_CAM_Result_en_t	SensorSetStillMode(CamImageSize_t image_resolution,CamDataFmt_t image_format);
static HAL_CAM_Result_en_t	SensorReset(CamSensor_t sensor_sel);
static HAL_CAM_Result_en_t	SensorSetFrameRate(CamRates_t fps, CamSensorSelect_t sensor);
static HAL_CAM_Result_en_t	ov_write(unsigned int sub_addr, unsigned char data);
static UInt8				ov_read(unsigned int sub_addr);
static HAL_CAM_Result_en_t OV2655_SetClockPolarity(CamSensorSelect_t sensor);

static HAL_CAM_Result_en_t	Init_OV2655(CamSensorSelect_t sensor);
static void 	OV2655_set_dummy(UInt16 pixels, UInt16 lines);
static UInt16  OV2655_read_shutter(void);
static void 	OV2655_NightMode(bool enable);
static UInt16  read_OV2650_gain(void);
static void  write_OV2650_gain(UInt16 gain);
static void OV2650_write_shutter(UInt16 shutter);
static void    OV2650_Computer_AECAGC(void);
static void    OV2650_SetPreviewZoom( CamZoom_t zoom_step);
static void    OV2650_SetStillZoom( CamImageSize_t image_resolution);






//****************************************************************************
//							OV2655 Paramter 
//****************************************************************************
UInt16 exposure_line_h = 0, exposure_line_l = 0;
UInt16 extra_exposure_line_h = 0, extra_exposure_line_l = 0;
UInt16 extra_exposure_lines = 0;
UInt16 dummy_pixels=0, dummy_lines=0;

UInt16 g_Capture_Gain16=0 ;    
UInt16 g_Capture_Shutter=0;
UInt16 g_Capture_Extra_Lines=0;

UInt16	g_PV_Dummy_Pixels = 0;
UInt16	g_Capture_Dummy_Pixels = 0;
UInt16	g_Capture_Dummy_Lines = 0;
UInt16	g_PV_Gain16 = 0;
UInt16	g_PV_Shutter = 0;
UInt16	g_PV_Extra_Lines = 0;
CamZoom_t	g_Zoom_Level = CamZoom_1_0;



//******************************************************************************
//
// Function Name:   OV2650_SetPreviewZoom
//
// Description: Set Preview zoom setting
//
// Notes:
//
//******************************************************************************

static void    OV2650_SetPreviewZoom( CamZoom_t zoom_step)
{
	unsigned int regValue=0;


	//printk( "[CAM] OV2650_SetPreviewZoom zoom_step = %d \n",zoom_step);

	regValue = ov_read(0x30B1);
	// set VSYNC as input
	ov_write(0x30B1, (regValue&0xDF) );

	switch(zoom_step)
	{
		case CamZoom_1_0:	// zoom X1
			ov_write(0x3020, 0x01);
			ov_write(0x3021, 0x18);
			ov_write(0x3022, 0x00);
			ov_write(0x3023, 0x06);
			ov_write(0x3024, 0x06);
			ov_write(0x3025, 0x58);
			ov_write(0x3026, 0x02);
			ov_write(0x3027, 0x61);
			ov_write(0x3088, 0x01);
			ov_write(0x3089, 0x40);
			ov_write(0x308A, 0x00);
			ov_write(0x308B, 0xF0);
			ov_write(0x3316, 0x64);
			ov_write(0x3317, 0x25);
			ov_write(0x3318, 0x80);
			ov_write(0x331A, 0x14);
			ov_write(0x331B, 0x0f);
			ov_write(0x331C, 0x00);
			ov_write(0x3012, 0x10);
			ov_write(0x3600, 0x80);
			ov_write(0x3302, 0x10);
			ov_write(0x302A, 0x02);
			ov_write(0x302B, 0x6A);
			ov_write(0x306F, 0x14);

			break;
			
		case CamZoom_1_2: // zoom X1.2
	            ov_write(0x3020, 0x01);
	            ov_write(0x3021, 0x9D);
				
	            ov_write(0x3022, 0x00);
	            ov_write(0x3023, 0x38);
				
	            ov_write(0x3024, 0x05);
	            ov_write(0x3025, 0x4D);
				
	            ov_write(0x3026, 0x01);
	            ov_write(0x3027, 0xFD);
				
	            ov_write(0x3088, 0x01);
	            ov_write(0x3089, 0x40);
	            ov_write(0x308A, 0x00);
	            ov_write(0x308B, 0xF0);
				
	            ov_write(0x3316, 0x53);
	            ov_write(0x3317, 0x1F);
	            ov_write(0x3318, 0x45);
				
	            ov_write(0x331A, 0x14);
	            ov_write(0x331B, 0x0f);
	            ov_write(0x331C, 0x00);
				
	            ov_write(0x3012, 0x10);
	            ov_write(0x3600, 0x80);
	            ov_write(0x3302, 0x10);
	            ov_write(0x302A, 0x02);
	            ov_write(0x302B, 0x6A);
	            ov_write(0x306F, 0x14);
			break;
		case CamZoom_1_4: // zoom X1.4
	            ov_write(0x3020, 0x01);
	            ov_write(0x3021, 0xFC);
				
	            ov_write(0x3022, 0x00);
	            ov_write(0x3023, 0x5B);
				
	            ov_write(0x3024, 0x04);
	            ov_write(0x3025, 0x8E);
				
	            ov_write(0x3026, 0x01);
	            ov_write(0x3027, 0xB5);
				
	            ov_write(0x3088, 0x01);
	            ov_write(0x3089, 0x40);
	            ov_write(0x308A, 0x00);
	            ov_write(0x308B, 0xF0);
				
	            ov_write(0x3316, 0x47);
	            ov_write(0x3317, 0x1A);
	            ov_write(0x3318, 0xC6);
				
	            ov_write(0x331A, 0x14);
	            ov_write(0x331B, 0x0f);
	            ov_write(0x331C, 0x00);
				
	            ov_write(0x3012, 0x10);
	            ov_write(0x3600, 0x80);
	            ov_write(0x3302, 0x10);
	            ov_write(0x302A, 0x02);
	            ov_write(0x302B, 0x6A);
	            ov_write(0x306F, 0x14);
			break;
		case CamZoom_1_6: // zoom X1.6
	            ov_write(0x3020, 0x02);
	            ov_write(0x3021, 0x44);
				
	            ov_write(0x3022, 0x00);
	            ov_write(0x3023, 0x76);
				
	            ov_write(0x3024, 0x04);
	            ov_write(0x3025, 0x00);
				
	            ov_write(0x3026, 0x01);
	            ov_write(0x3027, 0x80);
				
	            ov_write(0x3088, 0x01);
	            ov_write(0x3089, 0x40);
	            ov_write(0x308A, 0x00);
	            ov_write(0x308B, 0xF0);
				
	            ov_write(0x3316, 0x3E);
	            ov_write(0x3317, 0x17);
	            ov_write(0x3318, 0x78);
				
	            ov_write(0x331A, 0x14);
	            ov_write(0x331B, 0x0f);
	            ov_write(0x331C, 0x00);
				
	            ov_write(0x3012, 0x10);
	            ov_write(0x3600, 0x80);
	            ov_write(0x3302, 0x10);
	            ov_write(0x302A, 0x02);
	            ov_write(0x302B, 0x6A);
	            ov_write(0x306F, 0x14);
			break;
		case CamZoom_1_8: // zoom X1.8
	            ov_write(0x3020, 0x02);
	            ov_write(0x3021, 0x7B);
				
	            ov_write(0x3022, 0x00);
	            ov_write(0x3023, 0x8B);
				
	            ov_write(0x3024, 0x03);
	            ov_write(0x3025, 0x90);
				
	            ov_write(0x3026, 0x01);
	            ov_write(0x3027, 0x56);
				
	            ov_write(0x3088, 0x01);
	            ov_write(0x3089, 0x40);
	            ov_write(0x308A, 0x00);
	            ov_write(0x308B, 0xF0);
				
	            ov_write(0x3316, 0x37);
	            ov_write(0x3317, 0x14);
	            ov_write(0x3318, 0xD8);
				
	            ov_write(0x331A, 0x14);
	            ov_write(0x331B, 0x0f);
	            ov_write(0x331C, 0x00);
				
	            ov_write(0x3012, 0x10);
	            ov_write(0x3600, 0x80);
	            ov_write(0x3302, 0x10);
	            ov_write(0x302A, 0x02);
	            ov_write(0x302B, 0x6A);
	            ov_write(0x306F, 0x14);
			break;
		case CamZoom_2_0: // zoom X2.0
	            ov_write(0x3020, 0x02);
	            ov_write(0x3021, 0xA8);
				
	            ov_write(0x3022, 0x00);
	            ov_write(0x3023, 0x9C);
				
	            ov_write(0x3024, 0x03);
	            ov_write(0x3025, 0x38);
				
	            ov_write(0x3026, 0x01);
	            ov_write(0x3027, 0x35);
				
	            ov_write(0x3088, 0x01);
	            ov_write(0x3089, 0x40);
	            ov_write(0x308A, 0x00);
	            ov_write(0x308B, 0xF0);
				
	            ov_write(0x3316, 0x32);
	            ov_write(0x3317, 0x12);
	            ov_write(0x3318, 0xC0);
				
	            ov_write(0x331A, 0x14);
	            ov_write(0x331B, 0x0f);
	            ov_write(0x331C, 0x00);
				
	            ov_write(0x3012, 0x10);
	            ov_write(0x3600, 0x80);
	            ov_write(0x3302, 0x10);
	            ov_write(0x302A, 0x02);
	            ov_write(0x302B, 0x6A);
	            ov_write(0x306F, 0x14);
			break;
		default:
			printk( "[CAM] Wrong setting\n");
			break;
				
	}

	regValue = ov_read(0x30B1);
	// set VSYNC as output
	ov_write(0x30B1, (regValue|0x20) );

}

static void    OV2650_SetStillZoom( CamImageSize_t image_resolution)
{
	//printk( "[CAM] OV2650_SetStillZoom image_resolution = %d, g_Zoom_Level = %d \n",image_resolution,g_Zoom_Level);
	 switch(image_resolution)
	{
		case CamImageSize_SVGA:
			switch(g_Zoom_Level)
			{
				case CamZoom_1_0:	// zoom X1
			            ov_write(0x3020, 0x01);
			            ov_write(0x3021, 0x18);
			            ov_write(0x3022, 0x00);
			            ov_write(0x3023, 0x0A);
						
			            ov_write(0x3024, 0x06);
			            ov_write(0x3025, 0x58);
			            ov_write(0x3026, 0x04);
			            ov_write(0x3027, 0xBC);
						
			            ov_write(0x3088, 0x03);
			            ov_write(0x3089, 0x20);
			            ov_write(0x308A, 0x02);
			            ov_write(0x308B, 0x58);
						
			            ov_write(0x3316, 0x64);
			            ov_write(0x3317, 0x4B);
			            ov_write(0x3318, 0x00);
						
			            ov_write(0x331A, 0x32);
			            ov_write(0x331B, 0x25);
			            ov_write(0x331C, 0x80);
						
			            ov_write(0x3012, 0x00);
			            ov_write(0x3600, 0x80);
			            ov_write(0x3302, 0x10);
			            ov_write(0x302A, 0x04);
			            ov_write(0x302B, 0xD4);
			            //ov_write(0x306F, 0x0C);
					break;
					
				case CamZoom_1_2: // zoom X1.2
			            ov_write(0x3020, 0x01);
			            ov_write(0x3021, 0x9D);
			            ov_write(0x3022, 0x00);
			            ov_write(0x3023, 0x6E);
						
			            ov_write(0x3024, 0x05);
			            ov_write(0x3025, 0x4D);
			            ov_write(0x3026, 0x03);
			            ov_write(0x3027, 0xF4);
						
			            ov_write(0x3088, 0x03);
			            ov_write(0x3089, 0x20);
			            ov_write(0x308A, 0x02);
			            ov_write(0x308B, 0x58);
						
			            ov_write(0x3316, 0x53);
			            ov_write(0x3317, 0x3E);
			            ov_write(0x3318, 0x85);
						
			            ov_write(0x331A, 0x32);
			            ov_write(0x331B, 0x25);
			            ov_write(0x331C, 0x80);
						
			            ov_write(0x3012, 0x00);
			            ov_write(0x3600, 0x80);
			            ov_write(0x3302, 0x10);
			            ov_write(0x302A, 0x04);
			            ov_write(0x302B, 0xD4);
			            //ov_write(0x306F, 0x0C);
					break;
				case CamZoom_1_4: // zoom X1.4
			            ov_write(0x3020, 0x01);
			            ov_write(0x3021, 0xFC);
			            ov_write(0x3022, 0x00);
			            ov_write(0x3023, 0xB5);
						
			            ov_write(0x3024, 0x04);
			            ov_write(0x3025, 0x8E);
			            ov_write(0x3026, 0x03);
			            ov_write(0x3027, 0x65);
						
			            ov_write(0x3088, 0x03);
			            ov_write(0x3089, 0x20);
			            ov_write(0x308A, 0x02);
			            ov_write(0x308B, 0x58);
						
			            ov_write(0x3316, 0x47);
			            ov_write(0x3317, 0x35);
			            ov_write(0x3318, 0x96);
						
			            ov_write(0x331A, 0x32);
			            ov_write(0x331B, 0x25);
			            ov_write(0x331C, 0x80);
						
			            ov_write(0x3012, 0x00);
			            ov_write(0x3600, 0x80);
			            ov_write(0x3302, 0x10);
			            ov_write(0x302A, 0x04);
			            ov_write(0x302B, 0xD4);
			            //ov_write(0x306F, 0x0C);
					break;
				case CamZoom_1_6: // zoom X1.6
			            ov_write(0x3020, 0x02);
			            ov_write(0x3021, 0x44);
			            ov_write(0x3022, 0x00);
			            ov_write(0x3023, 0xEB);
						
			            ov_write(0x3024, 0x04);
			            ov_write(0x3025, 0x00);
			            ov_write(0x3026, 0x02);
			            ov_write(0x3027, 0xFA);
						
			            ov_write(0x3088, 0x03);
			            ov_write(0x3089, 0x20);
			            ov_write(0x308A, 0x02);
			            ov_write(0x308B, 0x58);
						
			            ov_write(0x3316, 0x3E);
			            ov_write(0x3317, 0x2E);
			            ov_write(0x3318, 0xE8);
						
			            ov_write(0x331A, 0x32);
			            ov_write(0x331B, 0x25);
			            ov_write(0x331C, 0x80);
						
			            ov_write(0x3012, 0x00);
			            ov_write(0x3600, 0x80);
			            ov_write(0x3302, 0x10);
			            ov_write(0x302A, 0x04);
			            ov_write(0x302B, 0xD4);
			            //ov_write(0x306F, 0x0C);
					break;
				case CamZoom_1_8: // zoom X1.8
			            ov_write(0x3020, 0x02);
			            ov_write(0x3021, 0x7B);
			            ov_write(0x3022, 0x01);
			            ov_write(0x3023, 0x14);
						
			            ov_write(0x3024, 0x03);
			            ov_write(0x3025, 0x90);
			            ov_write(0x3026, 0x02);
			            ov_write(0x3027, 0xA6);
						
			            ov_write(0x3088, 0x03);
			            ov_write(0x3089, 0x20);
			            ov_write(0x308A, 0x02);
			            ov_write(0x308B, 0x58);
						
			            ov_write(0x3316, 0x37);
			            ov_write(0x3317, 0x29);
			            ov_write(0x3318, 0xA8);
						
			            ov_write(0x331A, 0x32);
			            ov_write(0x331B, 0x25);
			            ov_write(0x331C, 0x80);
						
			            ov_write(0x3012, 0x00);
			            ov_write(0x3600, 0x80);
			            ov_write(0x3302, 0x10);
			            ov_write(0x302A, 0x04);
			            ov_write(0x302B, 0xD4);
			            //ov_write(0x306F, 0x0C);
					break;
				case CamZoom_2_0: // zoom X2.0
			            ov_write(0x3020, 0x02);
			            ov_write(0x3021, 0xA8);
			            ov_write(0x3022, 0x01);
			            ov_write(0x3023, 0x36);
						
			            ov_write(0x3024, 0x03);
			            ov_write(0x3025, 0x38);
			            ov_write(0x3026, 0x02);
			            ov_write(0x3027, 0x64);
						
			            ov_write(0x3088, 0x03);
			            ov_write(0x3089, 0x20);
			            ov_write(0x308A, 0x02);
			            ov_write(0x308B, 0x58);
						
			            ov_write(0x3316, 0x32);
			            ov_write(0x3317, 0x25);
			            ov_write(0x3318, 0x80);
						
			            ov_write(0x331A, 0x32);
			            ov_write(0x331B, 0x25);
			            ov_write(0x331C, 0x80);
						
			            ov_write(0x3012, 0x00);
			            ov_write(0x3600, 0x80);
			            ov_write(0x3302, 0x10);
			            ov_write(0x302A, 0x04);
			            ov_write(0x302B, 0xD4);
			           // ov_write(0x306F, 0x0C);
					break;
				default:
					printk( "[CAM] Wrong setting\n");
					break;
						
			}			
			break;
		case CamImageSize_4VGA:
			switch(g_Zoom_Level)
			{
				case CamZoom_1_0:	// zoom X1
			            ov_write(0x3020, 0x01);
			            ov_write(0x3021, 0x18);
			            ov_write(0x3022, 0x00);
			            ov_write(0x3023, 0x0A);
						
			            ov_write(0x3024, 0x06);
			            ov_write(0x3025, 0x58);
			            ov_write(0x3026, 0x04);
			            ov_write(0x3027, 0xBC);
						
			            ov_write(0x3088, 0x05);
			            ov_write(0x3089, 0x00);
			            ov_write(0x308A, 0x03);
			            ov_write(0x308B, 0xC0);
						
			            ov_write(0x3316, 0x64);
			            ov_write(0x3317, 0x4B);
			            ov_write(0x3318, 0x00);
						
			            ov_write(0x331A, 0x50);
			            ov_write(0x331B, 0x3C);
			            ov_write(0x331C, 0x00);
						
			            ov_write(0x3012, 0x00);
			            ov_write(0x3600, 0x80);
			            ov_write(0x3302, 0x10);
			            ov_write(0x302A, 0x04);
			            ov_write(0x302B, 0xD4);
			            //ov_write(0x306F, 0x0C);
					break;
					
				case CamZoom_1_2: // zoom X1.2
			            ov_write(0x3020, 0x01);
			            ov_write(0x3021, 0x9D);
			            ov_write(0x3022, 0x00);
			            ov_write(0x3023, 0x6E);
						
			            ov_write(0x3024, 0x05);
			            ov_write(0x3025, 0x4D);
			            ov_write(0x3026, 0x03);
			            ov_write(0x3027, 0xF4);
						
			            ov_write(0x3088, 0x05);
			            ov_write(0x3089, 0x00);
			            ov_write(0x308A, 0x03);
			            ov_write(0x308B, 0xC0);
						
			            ov_write(0x3316, 0x53);
			            ov_write(0x3317, 0x3E);
			            ov_write(0x3318, 0x85);
						
			            ov_write(0x331A, 0x50);
			            ov_write(0x331B, 0x3C);
			            ov_write(0x331C, 0x00);
						
			            ov_write(0x3012, 0x00);
			            ov_write(0x3600, 0x80);
			            ov_write(0x3302, 0x10);
			            ov_write(0x302A, 0x04);
			            ov_write(0x302B, 0xD4);
			            //ov_write(0x306F, 0x0C);
					break;
				default:
					printk( "[CAM] Wrong setting\n");
					break;
						
			}				
			break;
		case CamImageSize_UXGA:
			#if 0
			// don't need to set these value
			ov_write(0x3020, 0x01);
			ov_write(0x3021, 0x18);
			ov_write(0x3022, 0x00);
			ov_write(0x3023, 0x0A);

			ov_write(0x3024, 0x06);
			ov_write(0x3025, 0x58);
			ov_write(0x3026, 0x04);
			ov_write(0x3027, 0xBC);

			ov_write(0x3088, 0x06);
			ov_write(0x3089, 0x40);
			ov_write(0x308A, 0x04);
			ov_write(0x308B, 0xB0);

			ov_write(0x3316, 0x64);
			ov_write(0x3317, 0x4B);
			ov_write(0x3318, 0x00);

			ov_write(0x331A, 0x64);
			ov_write(0x331B, 0x4B);
			ov_write(0x331C, 0x00);

			ov_write(0x3012, 0x00);
			ov_write(0x3600, 0x80);
			ov_write(0x3302, 0x10);
			ov_write(0x302A, 0x04);
			ov_write(0x302B, 0xD4);
			ov_write(0x306F, 0x0C);
			#endif
			break;				
		default:
			printk( "[CAM] Didn't support zoom on this resolution \n");
			break;
	}
			


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

    switch (nSensor)
    {
        case CamSensorPrimary:                                  // Primary Sensor Configuration
        default:
            CamSensorCfg_st.sensor_config_caps = &CamPrimaryCfgCap_st;
            break;
        case CamSensorSecondary:                                // Secondary Sensor Configuration
            CamSensorCfg_st.sensor_config_caps = NULL;
            break;
    }
    config_tbl = &CamSensorCfg_st;
    return config_tbl;
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

    switch (nSeqSel)
    {
        case SensorInitPwrUp:   // Camera Init Power Up
					if ( (nSensor == CamSensorPrimary) || (nSensor == CamSensorSecondary) )
			 		{
			 			 HAL_CAM_Trace(HAL_CAM_TRACE_DEVICE|HAL_CAM_TRACE_INFO, "SensorInitPwrUp Sequence\r\n");  
				 		 *pLength = sizeof(CamPowerOnSeq);
				 			power_seq = CamPowerOnSeq;
			 		}
			 		break;        	
        case SensorPwrUp:   
            if ( (nSensor == CamSensorPrimary) || (nSensor == CamSensorSecondary) )
            {
                HAL_CAM_Trace(HAL_CAM_TRACE_DEVICE|HAL_CAM_TRACE_INFO, "SensorPwrUp Sequence\r\n"); 
                 *pLength = sizeof(WakeupformPowerDownSeq);
                power_seq = WakeupformPowerDownSeq;
            }
            break;

        case SensorInitPwrDn:   // Camera Init Power Down (Unused)
 						if ( (nSensor == CamSensorPrimary) || (nSensor == CamSensorSecondary) )
						{
							  HAL_CAM_Trace(HAL_CAM_TRACE_DEVICE|HAL_CAM_TRACE_INFO, "SensorInitPwrDn Sequence\r\n");  
								*pLength = sizeof(CamPowerOffSeq);
								power_seq = CamPowerOffSeq;
						}
						break;       	
        case SensorPwrDn:   // Both off
            if ( (nSensor == CamSensorPrimary) || (nSensor == CamSensorSecondary) )
            {
                HAL_CAM_Trace(HAL_CAM_TRACE_DEVICE|HAL_CAM_TRACE_INFO, "SensorPwrDn Sequence\r\n"); 
                *pLength = sizeof(CamPowerDownSeq);
                power_seq = CamPowerDownSeq;
            }
            break;

        case SensorFlashEnable: // Flash Enable           
            break;

        case SensorFlashDisable:    // Flash Disable           
            break;

        default:
            break;
    }
    return power_seq;

}

//***************************************************************************
//
//      CAMDRV_Supp_Init performs additional device specific initialization
//
//   @return  HAL_CAM_Result_en_t
//
//       Notes:
//
//****************************************************************************

HAL_CAM_Result_en_t CAMDRV_Supp_Init(CamSensorSelect_t sensor)
{
    HAL_CAM_Result_en_t ret_val = HAL_CAM_SUCCESS;
    return ret_val;
} // CAMDRV_Supp_Init()


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
	HAL_CAM_Trace(HAL_CAM_TRACE_HALDRV|HAL_CAM_TRACE_FUNCTION, "%s():called \r\n", __FUNCTION__);

	if (CheckCameraDeviceID(sensor)!= HAL_CAM_SUCCESS)
    {
        result = HAL_CAM_ERROR_INTERNAL_ERROR;
		return result;
    }
	
   	if(Init_OV2655(sensor)!= HAL_CAM_SUCCESS)
	{
		result = HAL_CAM_ERROR_INTERNAL_ERROR;
		HAL_CAM_Trace(HAL_CAM_TRACE_HALDRV|HAL_CAM_TRACE_ERROR, "%s(): init_OV2655(): ERROR: \r\n", __FUNCTION__);
	}
	

    return result;
}
////////////////////////////////////////////////////////////////////////////////////////////

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
 ////////////////////////////////////////////////////////////////////////////////////////////
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
    if (sensor == CamSensorPrimary) 
    {
        res_size = sizeof(sSensorResInfo_OV2655_st)/sizeof(CamResolutionConfigure_t);
        res_ptr = &sSensorResInfo_OV2655_st[0];
    }   
    else
    {
        HAL_CAM_Trace(HAL_CAM_TRACE_DEVICE|HAL_CAM_TRACE_ERROR, "CAMDRV_GetResolution(): ERROR:  Sensor Failed \r\n");
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
                HAL_CAM_Trace(HAL_CAM_TRACE_DEVICE|HAL_CAM_TRACE_INFO, "CAMDRV_GetResolutionAvailable(): Resolution: size index=%d width=%d height=%d \r\n", res_ptr->previewIndex,res_ptr->resX,res_ptr->resY);
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
                HAL_CAM_Trace(HAL_CAM_TRACE_DEVICE|HAL_CAM_TRACE_INFO, "CAMDRV_GetResolutionAvailable(): Resolution: size index=%d width=%d height=%d \r\n", res_ptr->captureIndex,res_ptr->resX,res_ptr->resY);
                return result;
            }
        }
        else
        {
            HAL_CAM_Trace(HAL_CAM_TRACE_DEVICE|HAL_CAM_TRACE_ERROR, "CAMDRV_GetResolutionAvailable(): ERROR: Mode Failed\r\n");
            result = HAL_CAM_ERROR_ACTION_NOT_SUPPORTED;
            return result;
        }
    // Increment table pointer        
        res_ptr++;
    }
// Resolution best match settings
    HAL_CAM_Trace(HAL_CAM_TRACE_DEVICE|HAL_CAM_TRACE_INFO, "CAMDRV_GetResolutionAvailable(): Best Match used \r\n");
    sensor_size->size = (CamImageSize_t)best_match_ptr->previewIndex;
    sensor_size->resX = best_match_ptr->resX;
    sensor_size->resY = best_match_ptr->resY;
    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////
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
//////////////////////////////////////////////////////////////////////////////////////////// 
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
    if (sensor == CamSensorPrimary) 
    {
        res_size = sizeof(sSensorResInfo_OV2655_st)/sizeof(CamResolutionConfigure_t);
        res_ptr = &sSensorResInfo_OV2655_st[0];
    }   
    else
    {
        HAL_CAM_Trace(HAL_CAM_TRACE_DEVICE|HAL_CAM_TRACE_ERROR, "CAMDRV_GetResolution(): ERROR:  Sensor Failed \r\n");
        result = HAL_CAM_ERROR_ACTION_NOT_SUPPORTED;
        return result;
    }
    
// Search Resolution table for requested resolution based on capture mode (or largest resolution available)
    for( index=0; index < res_size; index++ )
    {
        HAL_CAM_Trace(HAL_CAM_TRACE_DEVICE|HAL_CAM_TRACE_INFO, "CAMDRV_GetResolution(): Resolution: size index=%d width=%d height=%d \r\n", index,res_ptr->resX,res_ptr->resY);
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
            HAL_CAM_Trace(HAL_CAM_TRACE_DEVICE|HAL_CAM_TRACE_ERROR, "CAMDRV_GetResolution(): ERROR:  Mode Failed \r\n");
            result = HAL_CAM_ERROR_ACTION_NOT_SUPPORTED;
            return result;
        }
    // Increment table pointer        
        res_ptr++;
    }
    HAL_CAM_Trace(HAL_CAM_TRACE_DEVICE|HAL_CAM_TRACE_ERROR, "CAMDRV_GetResolution(): ERROR:  Resolution Failed \r\n");
    return result;
}
/** @} */


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
    result = SensorSetPreviewMode(image_resolution, image_format);
    return result;
}

//****************************************************************************
//
// Function Name:	HAL_CAM_Result_en_t CAMDRV_CfgCapture(CamImageSize_t image_resolution, CamDataFmt_t image_format)
//
// Description: This function configures Stillcapture
//
// Notes:
//
//****************************************************************************
HAL_CAM_Result_en_t CAMDRV_CfgCapture(
		CamImageSize_t image_resolution, 
		CamDataFmt_t image_format,
		CamSensorSelect_t sensor
		)
{
	  HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
   // --------Set up Camera Isp for Output Resolution & Format
    result =  SensorSetStillMode(image_resolution, image_format);
    result |= SensorSetFrameRate(CamRate_7_5,sensor);
	
    return result;
}


//****************************************************************************
//
// Function Name:   HAL_CAM_Result_en_t CAMDRV_SetFrameRate(CamRates_t fps)
//
// Description: This function sets the frame rate of the Camera Sensor
//
// Notes:    
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
        HAL_CAM_Trace(HAL_CAM_TRACE_DEVICE|HAL_CAM_TRACE_FUNCTION, "CAMDRV_SetFrameRate(): fps=%d\r\n",fps);
        PrimaryFrameRate_st.cur_setting = fps;
    	ret_val |= SensorSetFrameRate(fps, sensor);
    }	
    return ret_val;
}


//****************************************************************************
//
// Function Name:   HAL_CAM_Result_en_t CAMDRV_EnableVideoCapture(CamSensorSelect_t sensor)
//
// Description: This function starts camera video capture mode
//
// Notes:
//                  
//****************************************************************************
HAL_CAM_Result_en_t CAMDRV_EnableVideoCapture(CamSensorSelect_t    sensor)
{     
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;  
        
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

    HAL_CAM_Trace(HAL_CAM_TRACE_DEVICE|HAL_CAM_TRACE_FUNCTION, "CAMDRV_SetCamSleep(): \r\n");
    result = SensorSetSleepMode();         // ISP in Sleep Mode
   
    return result;
}

//****************************************************************************
//
// Function Name:   HAL_CAM_Result_en_t CAMDRV_DisableCapture(CamSensorSelect_t sensor)
//
// Description: This function halts camera video capture
//
// Notes:               
//****************************************************************************

HAL_CAM_Result_en_t CAMDRV_DisableCapture(CamSensorSelect_t sensor)
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;  
    return result;
}

//****************************************************************************
//
// Function Name:   HAL_CAM_Result_en_t CAMDRV_DisablePreview(void)
//
// Description: This function halts OV2655 camera video
//
// Notes:
//                  
//****************************************************************************
HAL_CAM_Result_en_t CAMDRV_DisablePreview(CamSensorSelect_t    sensor)
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
 
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
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
	HAL_CAM_Trace(HAL_CAM_TRACE_HALDRV|HAL_CAM_TRACE_ERROR, "CAMDRV_CfgStillnThumbCapture():\r\n");
          
	result = SensorSetStillMode(stills_resolution, stills_format);
	result |= SensorSetFrameRate(CamRate_7_5,sensor);
	
    return result;
}
//****************************************************************************
//
// Function Name:   Boolean CAMDRV_CheckManufacturerID(CamSensorSelect_t nSensor)
//
// Description: This function check manufacturer ID:
//
// Notes:
//
//****************************************************************************
Boolean CAMDRV_CheckManufacturerID(CamSensorSelect_t nSensor)
{
	HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
	Boolean rst = FALSE;
	
	if (CheckCameraDeviceID(nSensor)!= HAL_CAM_SUCCESS)
	{
	    result = HAL_CAM_ERROR_INTERNAL_ERROR;
	}

	if(result == HAL_CAM_SUCCESS)
	{
	    rst = TRUE;
	}

	 return rst;
}
//****************************************************************************
// Function Name:   HAL_CAM_Result_en_t CAMDRV_SetSceneMode(CamSceneMode_t scene_mode)
//
// Description: This function will set the scene mode of camera
// Notes:
//****************************************************************************

HAL_CAM_Result_en_t CAMDRV_SetSceneMode(CamSceneMode_t scene_mode, CamSensorSelect_t sensor )
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
  //  pr_info("CAMDRV_SetSceneMode()  called\n");
    printk("CAMDRV_SetSceneMode()  called\n");
    if(scene_mode == CamSceneMode_Auto){
        OV2655_NightMode(false);
        //do sth
    }else if(scene_mode == CamSceneMode_Portrait) {
        //do sth
    }else if(scene_mode == CamSceneMode_Landscape) {
        //do sth
    }else if(scene_mode == CamSceneMode_Flower) {
        //do sth
    }else if(scene_mode == CamSceneMode_Night) {
        OV2655_NightMode(true);
        //do sth
    }else if(scene_mode == CamSceneMode_Sunset) {
        //do sth
    }else {
        //do sth
    }
    if (sCamI2cStatus != HAL_CAM_SUCCESS)
    {
       //   pr_debug("CAMDRV_SetSceneMode(): Error[%d] \r\n", sCamI2cStatus);
       printk("CAMDRV_SetSceneMode(): Error[%d]\r\n",sCamI2cStatus);
          result = sCamI2cStatus;
    }
    
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
    UInt8  temp_reg;
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
    HAL_CAM_Trace(HAL_CAM_TRACE_DEVICE|HAL_CAM_TRACE_FUNCTION, "CAMDRV_SetWBMode(): wb_mode = %d \r\n", wb_mode);


    temp_reg=ov_read(0x3306);

    switch (wb_mode)
    {
        case CamWB_Auto://CAM_WB_AUTO
            ov_write(0x3306, temp_reg&~0x2);	// select Auto WB	
        break;

        case CamWB_Cloudy://CAM_WB_CLOUD: //cloudy
            ov_write(0x3306, temp_reg|0x2);  // select manual WB
            ov_write(0x3337, 0x68); //manual R G B
            ov_write(0x3338, 0x40);
            ov_write(0x3339, 0x4e);
			
        break;

        case CamWB_Daylight://CAM_WB_DAYLIGHT: //sunny
            ov_write(0x3306, temp_reg|0x2);  // Disable AWB
            ov_write(0x3337, 0x5e);
            ov_write(0x3338, 0x40);
            ov_write(0x3339, 0x46);
			
        break;

        case CamWB_Incandescent://CAM_WB_INCANDESCENCE: //office
            ov_write(0x3306, temp_reg|0x2);  // Disable AWB
            ov_write(0x3337, 0x5e);
            ov_write(0x3338, 0x40);
            ov_write(0x3339, 0x58);
			
        break;

        case CamWB_Tungsten://CAM_WB_TUNGSTEN: //home
            ov_write(0x3306, temp_reg|0x2);  // Disable AWB
            ov_write(0x3337, 0x54);
            ov_write(0x3338, 0x40);
            ov_write(0x3339, 0x70);
			
        break;

        case CamWB_DaylightFluorescent://CAM_WB_FLUORESCENT:
            ov_write(0x3306, temp_reg|0x2);  // Disable AWB
            ov_write(0x3337, 0x52);
            ov_write(0x3338, 0x40);
            ov_write(0x3339, 0x58);
			
        break;

        default:
            ov_write(0x3306, temp_reg&~0x2);	// select Auto WB
            wb_mode = CamWB_Auto;
            result = HAL_CAM_ERROR_ACTION_NOT_SUPPORTED; 
        break;
    }
    if (sCamI2cStatus != HAL_CAM_SUCCESS)
    {          
       HAL_CAM_Trace(HAL_CAM_TRACE_DEVICE|HAL_CAM_TRACE_ERROR, "CAMDRV_SetWBMode(): Error[%d] \r\n", sCamI2cStatus);
       result = sCamI2cStatus;
    } 

    return result;
}
//****************************************************************************
// Function Name:	HAL_CAM_Result_en_t CAMDRV_SetDigitalEffect( CamDigEffect_t effect)
//
// Description: This function will set the digital effect of camera
// Notes:
//****************************************************************************
HAL_CAM_Result_en_t CAMDRV_SetDigitalEffect( CamDigEffect_t effect, CamSensorSelect_t sensor)
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
	
    HAL_CAM_Trace(HAL_CAM_TRACE_DEVICE|HAL_CAM_TRACE_FUNCTION, "OV2655_CAMDRV_SetDigitalEffect(): effect = %d \r\n", effect);
    switch( effect )
    {
        case CamDigEffect_SepiaGreen: //Sepia(antique)			
            ov_write( 0x3391,0x18);
            ov_write( 0x3396,0x40);
            ov_write( 0x3397,0xa6);
        break;
        case CamDigEffect_Blue://Bluish
            ov_write( 0x3391,0x18);
            ov_write( 0x3396,0xa0);
            ov_write( 0x3397,0x40);          
        break;
        case CamDigEffect_Green://Greenish
            ov_write( 0x3391,0x18);
            ov_write( 0x3396,0x60);
            ov_write( 0x3397,0x60);          
        break;
        case CamDigEffect_Red://reddish
            ov_write( 0x3391,0x18);
            ov_write( 0x3396,0x80);
            ov_write( 0x3397,0xc0);          
        break;			
        case CamDigEffect_Yellow://Yellowish
            ov_write( 0x3391,0x18);
            ov_write( 0x3396,0x30);
            ov_write( 0x3397,0x90);          
        break;						
        case CamDigEffect_MonoChrome: //B&W
            ov_write( 0x3391,0x20);

        break;
        case CamDigEffect_NegColor: //Negative
            ov_write( 0x3391,0x40);

        break;
        case CamDigEffect_NoEffect: // normal
            ov_write( 0x3391,0x00);
        break;
        default:
            effect = CamDigEffect_NoEffect;
            ov_write( 0x3391,0x00);
            result = HAL_CAM_ERROR_ACTION_NOT_SUPPORTED;  
        break;
    }
    if (sCamI2cStatus != HAL_CAM_SUCCESS)
    {          
       HAL_CAM_Trace(HAL_CAM_TRACE_DEVICE|HAL_CAM_TRACE_ERROR, "CAMDRV_SetDigitalEffect(): Error[%d] \r\n", sCamI2cStatus);
       result = sCamI2cStatus;
    } 

    return result;
}
//****************************************************************************
// Function Name:	HAL_CAM_Result_en_t CAMDRV_SetBrightness(Int8 brightness)
// Description:
// Notes:		This function can be for average user's use.
//****************************************************************************
HAL_CAM_Result_en_t CAMDRV_SetBrightness(Int8 brightness, CamSensorSelect_t sensor)
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;	
    UInt8 temp_reg = ov_read(0x3391);
	
    switch (brightness)
    {
        case CAM_BL_NEG_4_3:
            ov_write(0x3391, temp_reg|0x4);
            ov_write(0x3390, 0x49);
            ov_write(0x339a, 0x30);
        break;
	
        case CAM_BL_NEG_3_3:
            ov_write(0x3391, temp_reg|0x4);
            ov_write(0x3390, 0x49);
            ov_write(0x339a, 0x30);
        break;
	
        case CAM_BL_NEG_2_3:
            ov_write(0x3391, temp_reg|0x4);
            ov_write(0x3390, 0x49);
            ov_write(0x339a, 0x20);
        break;
	
        case CAM_BL_NEG_1_3:
            ov_write(0x3391, temp_reg|0x4);
            ov_write(0x3390, 0x49);
            ov_write(0x339a, 0x10);
        break;
	
        case CAM_BL_ZERO:
            ov_write(0x3391, temp_reg|0x4);
            ov_write(0x3390, 0x41);
            ov_write(0x339a, 0x00);
        break;
	
        case CAM_BL_POS_1_3:
            ov_write(0x3391, temp_reg|0x4);
            ov_write(0x3390, 0x41);
            ov_write(0x339a, 0x10);
        break;
	
        case CAM_BL_POS_2_3:
            ov_write(0x3391, temp_reg|0x4);
            ov_write(0x3390, 0x41);
            ov_write(0x339a, 0x20);
        break;
	
        case CAM_BL_POS_3_3:
            ov_write(0x3391, temp_reg|0x4);
            ov_write(0x3390, 0x41);
            ov_write(0x339a, 0x30);
        break;
	
        case CAM_BL_POS_4_3:
            ov_write(0x3391, temp_reg|0x4);
            ov_write(0x3390, 0x41);
            ov_write(0x339a, 0x30);
        break;
	
        default:
            result = HAL_CAM_ERROR_ACTION_NOT_SUPPORTED;
        break;
    }
    if (sCamI2cStatus != HAL_CAM_SUCCESS)
    {          
       HAL_CAM_Trace(HAL_CAM_TRACE_DEVICE|HAL_CAM_TRACE_ERROR, "CAMDRV_SetBrightness(): Error[%d] \r\n", sCamI2cStatus);
       result = sCamI2cStatus;
    }  

    return result;
}
//****************************************************************************
// Function Name:	HAL_CAM_Result_en_t CAMDRV_SetSaturation(Int8 saturation)
// Description:  for Preview
// Notes:		This function is NOT for average user's use.
//****************************************************************************

HAL_CAM_Result_en_t CAMDRV_SetSaturation(Int8 saturation, CamSensorSelect_t sensor)
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
    UInt8 temp_reg = ov_read(0x3391);

    switch (saturation)
    {
        case CAM_POS_2_3:
            ov_write(0x3391, temp_reg|0x4);
            ov_write(0x3394, 0x70);
            ov_write(0x3395, 0x70);
        break;
        case CAM_POS_1_3:
            ov_write(0x3391, temp_reg|0x4);
            ov_write(0x3394, 0x50);
            ov_write(0x3395, 0x50);
        break;
        case CAM_ZERO:
            ov_write(0x3391, temp_reg|0x4);
            ov_write(0x3394, 0x40);
            ov_write(0x3395, 0x40);
        break;
        case CAM_NEG_1_3:
            ov_write(0x3391, temp_reg|0x4);
            ov_write(0x3394, 0x30);
            ov_write(0x3395, 0x30);
        break;
        case CAM_NEG_3_3:
            ov_write(0x3391, temp_reg|0x4);
            ov_write(0x3394, 0x20);
            ov_write(0x3395, 0x20);
        break;
        default:
            result = HAL_CAM_ERROR_ACTION_NOT_SUPPORTED;
            return result;
    }
    if (sCamI2cStatus != HAL_CAM_SUCCESS)
    {          
       HAL_CAM_Trace(HAL_CAM_TRACE_DEVICE|HAL_CAM_TRACE_ERROR, "CAMDRV_SetSaturation(): Error[%d] \r\n", sCamI2cStatus);
       result = sCamI2cStatus;
    }       
    return result;
} 
//****************************************************************************
// Function Name:	HAL_CAM_Result_en_t CAMDRV_SetContrast(Int8 contrast)
// Description: 	Set contrast for preview image.
// Notes:		This function can be for average user's use.
//****************************************************************************
HAL_CAM_Result_en_t CAMDRV_SetContrast(Int8 contrast, CamSensorSelect_t sensor)
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
    UInt8 temp_reg = ov_read(0x3391);
    switch (contrast)
    {	
        case CAM_Contrast_POS_3:
            ov_write(0x3391, temp_reg|0x4);
            ov_write(0x3390, 0x45);
            ov_write(0x3398, 0x2c);
            ov_write(0x3399, 0x2c);
        break;	
        case CAM_Contrast_POS_2:
            ov_write(0x3391, temp_reg|0x4);
            ov_write(0x3390, 0x45);
            ov_write(0x3398, 0x28);
            ov_write(0x3399, 0x28);

        break;	
        case CAM_Contrast_POS_1:
            ov_write(0x3391, temp_reg|0x4);
            ov_write(0x3390, 0x45);
            ov_write(0x3398, 0x24);
            ov_write(0x3399, 0x24);

        break;	
        case CAM_Contrast_ZERO:
            ov_write(0x3391, temp_reg|0x4);
            ov_write(0x3390, 0x41);
            ov_write(0x3398, 0x20);
            ov_write(0x3399, 0x20);
        break;	
        case CAM_Contrast_NEG_1:
            ov_write(0x3391, temp_reg|0x4);
            ov_write(0x3390, 0x45);
            ov_write(0x3398, 0x1c);
            ov_write(0x3399, 0x1c);
        break;	
        case CAM_Contrast_NEG_2:
            ov_write(0x3391, temp_reg|0x4);
            ov_write(0x3390, 0x45);
            ov_write(0x3398, 0x18);
            ov_write(0x3399, 0x18);
        break;
	/*
        case CAM_Contrast_NEG_3:
            ov_write(0x3391, temp_reg|0x4);
            ov_write(0x3390, 0x45);
            ov_write(0x3398, 0x18);
            ov_write(0x3399, 0x18);
        break;
	*/
        default:
            result = HAL_CAM_ERROR_ACTION_NOT_SUPPORTED;
        break;
    }
    if (sCamI2cStatus != HAL_CAM_SUCCESS)
    {          
       HAL_CAM_Trace(HAL_CAM_TRACE_DEVICE|HAL_CAM_TRACE_ERROR, "CAMDRV_SetContrast(): Error[%d] \r\n", sCamI2cStatus);
       result = sCamI2cStatus;
    }    
    HAL_CAM_Trace(HAL_CAM_TRACE_DEVICE|HAL_CAM_TRACE_INFO, "CAMDRV_SetContrast(): contrast=%d \r\n", contrast);
    return result;
}
/****************************************************************************/
//
// Function Name:	HAL_CAM_Result_en_t CAMDRV_SetAutoExposure(CamExposure_t exposure)
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

	result = HAL_CAM_ERROR_ACTION_NOT_SUPPORTED;		   // FIX ME!!!
	HAL_CAM_Trace(HAL_CAM_TRACE_DEVICE|HAL_CAM_TRACE_INFO, "OV2655_SetExposure(): exposure = %d \r\n", exposure);
	return result;
}
//****************************************************************************
//
// Function Name:   HAL_CAM_Result_en_t CAMDRV_SetFlickerControl(CamFlicker_t control)
//
// Description: This function is currently supported
//
// Notes: SET Frequency 60Hz or 50 Hz
//
//****************************************************************************
HAL_CAM_Result_en_t CAMDRV_SetAntiBanding(CamAntiBanding_t control, CamSensorSelect_t sensor)
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
    UInt8 banding;
	
    banding = ov_read(0x3014);

    switch (control)
    {
	case CamFlicker50Hz://CAM_BANDING_50HZ:
             ov_write(0x300e, 0x34);							
             ov_write( 0x3014, banding|0x80 );	  /* enable banding and 50 Hz */
             ov_write(0x301c, 0x02); 
             ov_write(0x3070, 0xba); 			
             break;	
	case CamFlicker60Hz://CAM_BANDING_60HZ:
             ov_write(0x300e, 0x34);				
             ov_write( 0x3014, banding & 0x7f );	/* enable banding and 60 Hz */
             ov_write(0x301d, 0x03); 
             ov_write(0x3072, 0x9b); 
             break;       
        default:
            result = HAL_CAM_ERROR_ACTION_NOT_SUPPORTED;           // FIX ME!!!
            break;
    }
    
    if (sCamI2cStatus != HAL_CAM_SUCCESS)
    {          
       HAL_CAM_Trace(HAL_CAM_TRACE_DEVICE|HAL_CAM_TRACE_ERROR, "CAMDRV_SetAntiBanding(): Error[%d] \r\n", sCamI2cStatus);
       result = sCamI2cStatus;
    }
    
    HAL_CAM_Trace(HAL_CAM_TRACE_DEVICE|HAL_CAM_TRACE_INFO, "CAMDRV_SetAntiBanding(): control = %d \r\n", control);
    return result;
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


    if (mode >= CamMirror_N_Modes)
    {
        result = HAL_CAM_ERROR_INTERNAL_ERROR;
        HAL_CAM_Trace(HAL_CAM_TRACE_DEVICE|HAL_CAM_TRACE_ERROR, "CAMDRV_SetMirrorMode(): ERROR:  Mode not Supported \r\n");
    }
    else
    {
        sensor_data = ov_read(0x307c) & 0xfc;        
        switch (mode)
        {
            case CamMirrorNone:				
                break;
            case CamMirrorVertical:
                sensor_data |= 0x0001;
                break;
            case CamMirrorHorizontal:
                sensor_data |= 0x0002;
                break;
            case CamMirrorBoth:
                sensor_data |= 0x0001;
                sensor_data |= 0x0002;
                break;
            default:
                result = HAL_CAM_ERROR_INTERNAL_ERROR;
                mode = CamMirrorNone;
                break;
        }   
        result |= ov_write(0x307c, sensor_data);     // image_orientation
       // ImageSettingsConfig_st.sensor_mirrormode->cur_setting = mode;
    }
    if (sCamI2cStatus != HAL_CAM_SUCCESS)
    {          
       HAL_CAM_Trace(HAL_CAM_TRACE_DEVICE|HAL_CAM_TRACE_ERROR, "CAMDRV_SetMirrorMode(): Error[%d] \r\n", sCamI2cStatus);
       result = sCamI2cStatus;
    }
    HAL_CAM_Trace(HAL_CAM_TRACE_DEVICE|HAL_CAM_TRACE_INFO, "CAMDRV_SetMirrorMode(): mode = %d \r\n", mode);
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
HAL_CAM_Result_en_t CAMDRV_SetZoom(CamZoom_t step, CamSensorSelect_t nSensor)
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
	
	//printk(KERN_INFO "[CAM] CAMDRV_SetZoom step = %d ,g_Zoom_Level = %d\n",step,g_Zoom_Level);
	g_Zoom_Level = step;
	OV2650_SetPreviewZoom(step);


    return result;
}


//****************************************************************************
// Function Name:   HAL_CAM_Result_en_t CAMDRV_SetFlashMode( FlashLedState_t effect)
//
// Description: This function will set the flash mode of camera
// Notes:
//****************************************************************************

HAL_CAM_Result_en_t CAMDRV_SetFlashMode(FlashLedState_t effect, CamSensorSelect_t sensor)
{
    HAL_CAM_Result_en_t result = HAL_CAM_ERROR_ACTION_NOT_SUPPORTED;
    pr_info("CAMDRV_SetFlashMode()  called\n");
    return result;
}

//****************************************************************************
// Function Name:   HAL_CAM_Result_en_t CAMDRV_SetFocusMode( CamFocusStatus_t effect)
//
// Description: This function will set the focus mode of camera
// Notes:
//****************************************************************************

HAL_CAM_Result_en_t CAMDRV_SetFocusMode(CamFocusControlMode_t effect,CamSensorSelect_t sensor)
{
    HAL_CAM_Result_en_t result = HAL_CAM_ERROR_ACTION_NOT_SUPPORTED;
    pr_info("CAMDRV_SetFocusMode()  called\n");
  
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
    HAL_CAM_Result_en_t result = HAL_CAM_ERROR_ACTION_NOT_SUPPORTED;
    pr_info("CAMDRV_SetJpegQuality()  called\n");
    return result;
}

//****************************************************************************
//
//  This function will perform specific action from defined list, copy of parameters passed thru parm structure.
//
//****************************************************************************

HAL_CAM_Result_en_t CAMDRV_ActionCtrl(
        HAL_CAM_Action_en_t action,                 
        void*               data,                  
        void*               callback               
        )
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS; 

    HAL_CAM_Trace(HAL_CAM_TRACE_DEVICE|HAL_CAM_TRACE_FUNCTION, "HAL_CAMDRV_Ctrl():  action=%d \r\n", action);

    switch (action) 
    {
#if 0    
// Operation Control Settings
        case ACTION_CAM_GetJpegMaxSize:                         ///< Get Jpeg Max size (bytes), use HAL_CAM_Action_param_st_t to get
            result = CAMDRV_GetJpegMaxSize(((HAL_CAM_Action_param_st_t *)data)->param, ((HAL_CAM_Action_param_st_t *)data)->sensor);
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
#endif
// Unsupported Actions in Camera Device Driver Return:  HAL_CAM_ERROR_ACTION_NOT_SUPPORTED
        default:
            HAL_CAM_Trace(HAL_CAM_TRACE_DEVICE|HAL_CAM_TRACE_ERROR, "HAL_CAMDRV_Ctrl(): Invalid Action \r\n");
            result = HAL_CAM_ERROR_ACTION_NOT_SUPPORTED;
            break;   
    }
    return result;
}                   

UInt16 CAMDRV_GetDeviceID(CamSensorSelect_t sensor)
{
    UInt16 id = 0;

    id = ov_read(uwDeviceId_MS) << 8;
    id |= ov_read(uwDeviceId_LS);

    return id;
}
UInt16 *CAMDRV_GetJpeg(short *buf)
{
    //Do not support JPEG
    return 0;
}

UInt8 * CAMDRV_GetThumbnail(void *buf, UInt32 offset)
{
    //Do not support Thumbnail
    return 0;
}

UInt32 CAMDRV_GetJpegSize(CamSensorSelect_t sensor, void *data)
{
    UInt32 size = 0;
    //Do not support JPEG	
    return size;
}
HAL_CAM_Result_en_t CAMDRV_TurnOffAF()
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
    pr_info("CAMDRV_TurnOffAF() called\n");
    //Do not support autoFocus
	
    return result;
}

HAL_CAM_Result_en_t CAMDRV_TurnOnAF()
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS; 
    /* AF DriverIC power enable */
    pr_info("CAMDRV_TurnOnAF() called\n");
    //Do not support autoFocus

    return result;
}


static HAL_CAM_Result_en_t SensorSetSleepMode(void)
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
    HAL_CAM_Trace(HAL_CAM_TRACE_DEVICE|HAL_CAM_TRACE_FUNCTION, "SensorSetSleepMode()\r\n");
	
    ov_write(0x30ab,0x00);
    ov_write(0x30ad,0x0a);
    ov_write(0x30ae,0x27);
    ov_write(0x363b,0x01);
    if (sCamI2cStatus != HAL_CAM_SUCCESS)
    {          
       HAL_CAM_Trace(HAL_CAM_TRACE_DEVICE|HAL_CAM_TRACE_ERROR, "CAMDRV_SetAntiBanding(): Error[%d] \r\n", sCamI2cStatus);
       result = sCamI2cStatus;
    }

    return result;   
    //For OV2655
} 
static HAL_CAM_Result_en_t	   SensorReset(CamSensor_t sensor_sel)
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
    HAL_CAM_Trace(HAL_CAM_TRACE_DEVICE|HAL_CAM_TRACE_FUNCTION, "SensorReset(): \r\n");
			 
    // Software reset 
    ov_write(0x3012,0x80); 
	
    msleep(5);
    return result;
}

static HAL_CAM_Result_en_t SensorSetPreviewMode(CamImageSize_t image_resolution, CamDataFmt_t image_format)
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
    UInt8 temp_AE_reg, temp_AWB_reg;
    UInt16 iDummyPixels = 0, iDummyLines = 0;
   
	
    ov_write(0x3002, exposure_line_h);
    ov_write(0x3003, exposure_line_l);
    write_OV2650_gain(g_PV_Gain16);

    //YUV
    //SVGA (800x600)
    // setup sensor output ROI
    ov_write(0x3010, 0x81); 
    ov_write(0x3012, 0x10); 
    
    ov_write(0x3014, 0x84);//night mode
	
    ov_write(0x3015, 0x02);//gain
    ov_write(0x3016, 0x82); 
    ov_write(0x3023, 0x06);
    ov_write(0x3026, 0x02);
    ov_write(0x3027, 0x5e);
    ov_write(0x302a, 0x02);
    ov_write(0x302b, 0x6a); 
	
    ov_write(0x330c, 0x00);//no set
    ov_write(0x3301, 0xff);
	
    ov_write(0x3069, 0x80);
    ov_write(0x306f, 0x14); 
	
    ov_write(0x3088, 0x03);
    ov_write(0x3089, 0x20); 
    ov_write(0x308a, 0x02);
    ov_write(0x308b, 0x58); 
    ov_write(0x308e, 0x00);//no set
	
    ov_write(0x30a1, 0x41);
    ov_write(0x30a3, 0x80);
    ov_write(0x30d9, 0x95);
    ov_write(0x3302, 0x11);
    ov_write(0x3317, 0x25);
    ov_write(0x3318, 0x80);
    ov_write(0x3319, 0x08);
    ov_write(0x331d, 0x38);

    ov_write(0x3373, 0x30);
    ov_write(0x3376, 0x05);//for preview sharp
    ov_write(0x3362, 0x90);
    ov_write(0x300f, 0xa6); //must set, from FAE

    switch(image_resolution)
    {	
        case CamImageSize_R_QCIF: //  R_QCIF			  144x176
    		ov_write(0x3302,0x11);	
            ov_write(0x3088,0x00);	//144
            ov_write(0x3089,0x90);
            ov_write(0x308a,0x00);	//176
            ov_write(0x308b,0xb0);
            ov_write(0x331a,0x09);
            ov_write(0x331b,0x0b);
            ov_write(0x331c,0x00);
            ov_write(0x3302,0x11);
            break;
			
        case CamImageSize_R_QVGA: //  R_QVGA			  240x320
    		ov_write(0x3302,0x11);	
            ov_write(0x3088,0x00);	//240
            ov_write(0x3089,0xF0);
            ov_write(0x308a,0x01);	//320
            ov_write(0x308b,0x40);
			
            ov_write(0x331a,0x0F);
            ov_write(0x331b,0x14);
            ov_write(0x331c,0x00);
            ov_write(0x3302,0x11);
            break;
        case CamImageSize_QCIF: //  QCIF			  176x144  
            ov_write(0x3302,0x11);	
            ov_write(0x3088,0x00);
            ov_write(0x3089,0xb0);
            ov_write(0x308a,0x00);
            ov_write(0x308b,0x90);
            ov_write(0x331a,0x0b);
            ov_write(0x331b,0x09);
            ov_write(0x331c,0x00);
            ov_write(0x3302,0x11);
            break;
        case CamImageSize_QVGA:	//	QVGA 			320x240  						 
            //320
            ov_write(0x3088,0x01);
            ov_write(0x3089,0x40);
            //240
            ov_write(0x308a,0x00);
            ov_write(0x308b,0xf0);			
            ov_write(0x331a,0x14);
            ov_write(0x331b,0x0f);
            ov_write(0x331c,0x00);
            ov_write(0x3302,0x11);  
            break;
        case CamImageSize_CIF: //  CIF		   352x288	
		     
            ov_write(0x3302,0x11);				   
            ov_write(0x3088,0x01);
            ov_write(0x3089,0x60);
            ov_write(0x308a,0x01);
            ov_write(0x308b,0x20);
            ov_write(0x331a,0x16);
            ov_write(0x331b,0x12);
            ov_write(0x331c,0x00);
            break;
        case CamImageSize_VGA:	 //  VGA	   640x480 
            ov_write(0x3302,0x11);				   
            ov_write(0x3088,0x02);
            ov_write(0x3089,0x80);
            ov_write(0x308a,0x01);
            ov_write(0x308b,0xe0);
            ov_write(0x331a,0x28);
            ov_write(0x331b,0x1e);
            ov_write(0x331c,0x00);
            break;
        case CamImageSize_SVGA://  SVGA 		   800x600			  
            ov_write(0x300e,0x34);  
            ov_write(0x3011,0x01);
            ov_write(0x3012,0x10);
            ov_write(0x302a,0x02);
            ov_write(0x302b,0x6a);
            ov_write(0x306f,0x14);
            ov_write(0x3020,0x01);  
            ov_write(0x3021,0x18);
            ov_write(0x3022,0x00);  
            ov_write(0x3023,0x06);  
            ov_write(0x3024,0x06);
            ov_write(0x3025,0x58);
            ov_write(0x3026,0x02);
            ov_write(0x3027,0x61);
            ov_write(0x3088,0x03);
            ov_write(0x3089,0x20);
            ov_write(0x308a,0x02);
            ov_write(0x308b,0x58);
            ov_write(0x3316,0x64);
            ov_write(0x3317,0x25);  
            ov_write(0x3318,0x80);
            ov_write(0x3319,0x08);
            ov_write(0x331a,0x64);
            ov_write(0x331b,0x4b);
            ov_write(0x331c,0x00);
            ov_write(0x331d,0x38);
            ov_write(0x3302,0x11);
            break;	 
        default:						
            result= HAL_CAM_ERROR_ACTION_NOT_SUPPORTED;
            break;
    }
    /* after set exposure line, there should be delay for 2~4 frame time, then enable AEC */
    msleep(100);

    // turn on AEC/AGC
    temp_AE_reg = ov_read(0x3013);
    ov_write(0x3013, temp_AE_reg| 0x05);

    //enable Auto WB
    temp_AWB_reg = ov_read(0x3324);
    ov_write(0x3324, temp_AWB_reg& ~0x40);

    //set dummy
    g_PV_Dummy_Pixels = iDummyPixels;
	
    OV2655_set_dummy(iDummyPixels, iDummyLines);
    msleep(50);
	
    OV2650_SetPreviewZoom(g_Zoom_Level);

    if (sCamI2cStatus != HAL_CAM_SUCCESS)
    {          
       HAL_CAM_Trace(HAL_CAM_TRACE_DEVICE|HAL_CAM_TRACE_ERROR, "SensorSetPreviewMode(): Error[%d] \r\n", sCamI2cStatus);
       result = sCamI2cStatus;
    }
// -------- Test mode
//  ov_write(0x030e, 1);

    return result;
}

//****************************************************************************
//
// Function Name:	HAL_CAM_Result_en_t SensorSetStillMode(CamImageSize_t image_resolution, CamDataFmt_t image_format)
//
// Description: This function configures Still
//
// Notes:
//
//****************************************************************************
static HAL_CAM_Result_en_t SensorSetStillMode(CamImageSize_t image_resolution, CamDataFmt_t image_format)
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
   
    volatile UInt32 shutter = 0,temp_reg;
    UInt8 temp_AE_reg,temp,temp2;

    temp_reg = ov_read(0x3014);

    //Disable night mode
    ov_write(0x3014, temp_reg & 0xf7); 
	   
    // turn off AEC/AGC
    temp_AE_reg = ov_read(0x3013);
    ov_write(0x3013, (temp_AE_reg&(~0x05)) );		 
	   
   
    exposure_line_h = ov_read(0x3002);
    exposure_line_l = ov_read(0x3003);
    extra_exposure_line_h = ov_read(0x302D);
    extra_exposure_line_l = ov_read(0x302E);
   
    shutter = OV2655_read_shutter();

    //calculater auto uv
    ov_write(0x330c,0x3e);
    temp = ov_read(0x330f);
    ov_write(0x3301,0xbf);//turn off auto uv
    //	temp = (temp&0x1f + 0x01)<<1;
    temp = temp&0x1f;
    temp = temp + 0x01;
    temp = 2*temp;
    temp2 = ov_read(0x3391);
	
    ov_write(0x3391,temp2|0x06);
    ov_write(0x3394,temp);
    ov_write(0x3395,temp);

    dummy_pixels=0; 
    dummy_lines=0;
    
 
    // 1600x1200
    ov_write(0x3010,0x81); 
    ov_write(0x3012,0x00);
    ov_write(0x3014,0x84); 
  
    //UXGA (1600X1200)
    ov_write(0x3015,0x02);
    ov_write(0x3016,0xc2); 
    ov_write(0x3023,0x0c);
    ov_write(0x3026,0x04);
    ov_write(0x3027,0xbc); 
    ov_write(0x302a,0x04);
    ov_write(0x302b,0xd4); 
    ov_write(0x3069,0x80); 
    ov_write(0x306f,0x54); 
    ov_write(0x3088,0x06);
    ov_write(0x3089,0x40); 
    ov_write(0x308a,0x04);
    ov_write(0x308b,0xb0); 
    ov_write(0x308e,0x64); 
    ov_write(0x30a1,0x41); 
    ov_write(0x30a3,0x80); 
    ov_write(0x30d9,0x95); 
    ov_write(0x3302,0x01);
    ov_write(0x3317,0x4b); 
    ov_write(0x3318,0x00);
    ov_write(0x3319,0x4c); 
    ov_write(0x331d,0x6c); 
    ov_write(0x3362,0x80);//full size shading  
    ov_write(0x3373,0x40);//for capture saw issue
    ov_write(0x3376,0x05); 

    //other resoultions are scale down from UXGA
    switch(image_resolution)
    {
        case CamImageSize_QCIF://  QCIF             176x144 
            ov_write(0x3302,0x11);  				
            ov_write(0x3088,0x00);
            ov_write(0x3089,0xb0);
            ov_write(0x308a,0x00);
            ov_write(0x308b,0x90);
            ov_write(0x331a,0x0b);
            ov_write(0x331b,0x09);
            ov_write(0x331c,0x00);
            break;			    

        case CamImageSize_QVGA://  QVGA             320x240
            ov_write(0x3302,0x11);  				
            ov_write(0x3088,0x01);
            ov_write(0x3089,0x40);
            ov_write(0x308a,0x00);
            ov_write(0x308b,0xf0);
            ov_write(0x331a,0x14);
            ov_write(0x331b,0x0f);
            ov_write(0x331c,0x00);
            break;		        
        case CamImageSize_CIF://  CIF             352x288              
            ov_write(0x3302,0x11);  				
            ov_write(0x3088,0x01);
            ov_write(0x3089,0x60);
            ov_write(0x308a,0x01);
            ov_write(0x308b,0x20);
            ov_write(0x331a,0x16);
            ov_write(0x331b,0x12);
            ov_write(0x331c,0x00);
            break;		
        case CamImageSize_VGA://  VGA             640x480            
            ov_write(0x3302,0x11);  				
            ov_write(0x3088,0x02);
            ov_write(0x3089,0x80);
            ov_write(0x308a,0x01);
            ov_write(0x308b,0xe0);
            ov_write(0x331a,0x28);
            ov_write(0x331b,0x1e);
            ov_write(0x331c,0x00);
            break;		
        case CamImageSize_SVGA://  SVGA 		   800x600	
            ov_write(0x3302,0x11);	
            ov_write(0x3088,0x03);
            ov_write(0x3089,0x20);
            ov_write(0x308a,0x02);
            ov_write(0x308b,0x58);
            ov_write(0x331a,0x32);
            ov_write(0x331b,0x25);
            ov_write(0x331c,0x80);			
            break;
        case CamImageSize_4VGA: //  SXGA	1280x960 
            ov_write(0x3302,0x11);	
            ov_write(0x3088,0x05);
            ov_write(0x3089,0x00);
            ov_write(0x308a,0x03);
            ov_write(0x308b,0xc0);
            ov_write(0x331a,0x50);
            ov_write(0x331b,0x3c);
            ov_write(0x331c,0x00);
            break;
        case CamImageSize_SXGA:	//  SXGA	1280x1024  (1.3MP)
			   
            ov_write(0x3302,0x11);	
            ov_write(0x3088,0x05);
            ov_write(0x3089,0x00);
            ov_write(0x308a,0x04);
            ov_write(0x308b,0x00);
            ov_write(0x331a,0x50);
            ov_write(0x331b,0x40);
            ov_write(0x331c,0x00);
            break;
        case CamImageSize_UXGA: //  UXGA	1600x1200	(2MP)
            //alredy set to UXGA
            break;	
        default:
            HAL_CAM_Trace(HAL_CAM_TRACE_DEVICE|HAL_CAM_TRACE_ERROR, "SensorSetStillMode()::Resolution is not support!!ERROR: \r\n");
            result= HAL_CAM_ERROR_ACTION_NOT_SUPPORTED;
            break;
    }

	
    OV2650_SetStillZoom(image_resolution);


    g_Capture_Dummy_Pixels = dummy_pixels ;
    g_Capture_Dummy_Lines = dummy_lines;

    //Resolution is changed form SVGA to UXVGA. Re-calculate UXGA Exposure. 
    OV2650_Computer_AECAGC();
	
    shutter = g_Capture_Shutter + g_Capture_Extra_Lines;

    HAL_CAM_Trace(HAL_CAM_TRACE_DEVICE|HAL_CAM_TRACE_INFO, 
        "SensorSetStillMode():Shutter=%d,Capture_Shutter=%d,Capture Extra line=%d,Capture gain =%d \r\n",
        shutter,g_Capture_Shutter,g_Capture_Extra_Lines,g_Capture_Gain16);

    if (shutter < 1) {
         shutter = 1;
    }    
    // set dummy
    OV2655_set_dummy(dummy_pixels, dummy_lines);
    // set shutter OVT
    OV2650_write_shutter(shutter);

    if(g_Capture_Gain16>62)
	 write_OV2650_gain(16); 
     else
	 write_OV2650_gain((g_Capture_Gain16+5)); 

     msleep(23);//delay for 1 frame
    
     // set gain
     write_OV2650_gain(g_Capture_Gain16); 

     if (sCamI2cStatus != HAL_CAM_SUCCESS)
     {          
        HAL_CAM_Trace(HAL_CAM_TRACE_DEVICE|HAL_CAM_TRACE_ERROR, "SensorSetStillMode(): Error[%d] \r\n", sCamI2cStatus);
        result = sCamI2cStatus;
     }

// -------- Test mode
//	ov_write(0x030e, 1);
     return result;
}
//****************************************************************************
//
// Function Name:	HAL_CAM_Result_en_t SensorSetFrameRate(CamRates_t fps, CamSensorSelect_t sensor)
//
// Description: This function configures frame rate
//
// Notes:
//
//****************************************************************************

static HAL_CAM_Result_en_t SensorSetFrameRate(CamRates_t fps, CamSensorSelect_t sensor)
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS; 
	
    if (fps > CamRate_30)
    {
        result = HAL_CAM_ERROR_ACTION_NOT_SUPPORTED;
    }
    else
    {
        if (fps == CamRate_Auto)
        {

        }
        else
        {
            switch(fps)
            {
					case CamRate_5:
                case CamRate_7_5:                   
                    ov_write(0x300e,0x34); //output 7.5fps
                    ov_write(0x3011,0x01); //output 7.5fps
                    ov_write(0x301c,0x02); 
                    ov_write(0x301d,0x03); 
                    ov_write(0x3070,0xba); //50Hz banding filter
                    ov_write(0x3072,0x9b); //60Hz banding filter
                    break;
                case CamRate_10:                    
                    ov_write(0x300e,0x38); //output 10fps
                    ov_write(0x3011,0x00); //output 10fps
                    ov_write(0x301c,0x08); 
                    ov_write(0x301d,0x0a); 
                    ov_write(0x3070,0x7c); //50Hz banding filter
                    ov_write(0x3072,0x67); //60Hz banding filter
                    break;
                case CamRate_15:            // 15 fps
                    ov_write(0x300e,0x34); //output 15fps
                    ov_write(0x3011,0x01); //output 15fps
                    ov_write(0x301c,0x05); 
                    ov_write(0x301d,0x06); 
                    ov_write(0x3070,0x5d); //50Hz banding filter
                    ov_write(0x3072,0x4d); //60Hz banding filter
                    break;
					 case CamRate_20:
                 case CamRate_25:           // 25 fps
                    ov_write(0x300e,0x36); //output 25fps
                    ov_write(0x3011,0x00); //output 25fps
                    ov_write(0x301c,0x03); 
                    ov_write(0x301d,0x04); 
                    ov_write(0x3070,0x9b); //50Hz banding filter
                    ov_write(0x3072,0x81); //60Hz banding filter
                    break;
                case CamRate_30:           // 30 fps
                    ov_write(0x300e,0x34); //output 30fps
                    ov_write(0x3011,0x00); //output 30fps
                    ov_write(0x301c,0x02); 
                    ov_write(0x301d,0x03); 
                    ov_write(0x3070,0xba); //50Hz banding filter
                    ov_write(0x3072,0x9b); //60Hz banding filter
                    break;
                default:                                        // Maximum Clock Rate = 26Mhz
                    ov_write(0x300e,0x34); //output 15fps
                    ov_write(0x3011,0x01); //output 15fps
                    ov_write(0x301c,0x05); 
                    ov_write(0x301d,0x06); 
                    ov_write(0x3070,0x5d); //50Hz banding filter
                    ov_write(0x3072,0x4d); //60Hz banding filter
                   // ImageSettingsConfig_st.sensor_framerate->cur_setting = CamRate_15;
                    result = HAL_CAM_ERROR_ACTION_NOT_SUPPORTED;
                    HAL_CAM_Trace(HAL_CAM_TRACE_DEVICE|HAL_CAM_TRACE_ERROR, "SensorSetFrameRate(): Frame Rate Unsupported:  ERROR: \r\n");
                    break;
            }
        }       // else (if (ImageSettingsConfig_st.sensor_framerate->cur_setting == CamRate_Auto))
     }       // else (if (fps <= CamRate_Auto))
     if (sCamI2cStatus != HAL_CAM_SUCCESS)
     {          
        HAL_CAM_Trace(HAL_CAM_TRACE_DEVICE|HAL_CAM_TRACE_ERROR, "SensorSetFrameRate(): Error[%d] \r\n", sCamI2cStatus);
        result = sCamI2cStatus;
     }

    return result;
}
//****************************************************************************
//
// Function Name:	HAL_CAM_Result_en_t CheckCameraDeviceID(CamSensor_t sensor_sel)
//
// Description: This function check the camera module device ID
//
// Notes:
//
//****************************************************************************

static HAL_CAM_Result_en_t  CheckCameraDeviceID(CamSensor_t sensor_sel)
{
	HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
	int id = 0;

	id = CAMDRV_GetDeviceID(sensor_sel);
	
    if (id != OV_DEVICE_ID)
    {
        HAL_CAM_Trace(HAL_CAM_TRACE_DEVICE|HAL_CAM_TRACE_ERROR, "checkOVCameraID():Camera Id wrong. Expected 0x%x but got 0x%x\r\n", OV_DEVICE_ID, id); 
        result = HAL_CAM_ERROR_INTERNAL_ERROR;
    }
    else
    {
        HAL_CAM_Trace(HAL_CAM_TRACE_DEVICE|HAL_CAM_TRACE_INFO, "checkOVCameraID():  Id=0x%x, ID Read= 0x%x\r\n", OV_DEVICE_ID, id);        
    }
	return result;
}
//***************************************************************************
/**
*		OV2655 & CAMERA SENSOR INITIALIZATION 
*/
//***************************************************************************

static HAL_CAM_Result_en_t Init_OV2655(CamSensorSelect_t sensor)
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;

    //sCamSensor = sensor;
	
    CamSensorCfg_st.sensor_config_caps = &CamPrimaryCfgCap_st;
    HAL_CAM_Trace(HAL_CAM_TRACE_DEVICE|HAL_CAM_TRACE_INFO, "Init_OV2655()::Init Primary Sensor OV2655: \r\n");

    // Software reset 
    SensorReset(sensor);

    //IO & Clock & Analog Setup
    ov_write(0x308c,0x80); //TMC12: DIS_MIPI_RW
    ov_write(0x308d,0x0e); //TMC13: MIPI disable
    ov_write(0x360b,0x00);
    ov_write(0x30b0,0xff); //IO_CTRL0: Cy[7:0]
    ov_write(0x30b1,0xff); //IO_CTRL1: C_VSYNC,C_STROBE,C_PCLK,C_HREF,Cy[9:8]
    ov_write(0x30b2,0x2c); //IO_CTRL2: R_PAD[3:0]

    ov_write(0x300e,0x34);


    ov_write(0x300f,0xa6);
    ov_write(0x3010,0x81);
    ov_write(0x3082,0x01);
    ov_write(0x30f4,0x01);
    ov_write(0x3090,0x3b);
    ov_write(0x3091,0xc0);
    ov_write(0x30ac,0x42);

    ov_write(0x30d1,0x08);
    ov_write(0x30a8,0x56);

    ov_write(0x3015,0x02); //AUTO_3: AGC ceiling = 4x, 5dummy frame


    ov_write(0x3093,0x00);
    ov_write(0x307e,0xe5); //TMC8: AGC[7:6]=b'11
    ov_write(0x3079,0x00);
    ov_write(0x30aa,0x42);
    ov_write(0x3017,0x40); //AUTO_5: disable data drop, manual banding counter=0
    ov_write(0x30f3,0x82);
    //ov_write(0x306a,0x0f); //0x0c->0x0f Joe 0814 : BLC
    ov_write(0x306a,0x0c); 

    ov_write(0x306d,0x00);
    ov_write(0x336a,0x3c);
    ov_write(0x3076,0x6a); //TMC0: VSYNC drop option: drop
    ov_write(0x30d9,0x95);

    //ov_write(0x3016,0xc2); //Joe=0xc2 0808 //org=0x82//AUTO_4: max exposure adjust option=2
    ov_write(0x3016,0x82);

    ov_write(0x3601,0x30);
    ov_write(0x304e,0x88);
    ov_write(0x30f1,0x82);
    ov_write(0x306f,0x14);
    ov_write(0x302a,0x02);
    ov_write(0x302b,0x6a);

    ov_write(0x3012,0x10);
    ov_write(0x3011,0x01);

    //AEC/AGC
    ov_write(0x3013,0xf7); //AUTO_1: banding filter, AGC auto, AEC auto
    ov_write(0x3018,0x80);
    ov_write(0x3019,0x70); 
    ov_write(0x301a,0xd4); 
    //ov_write(0x301c,0x07); //AECG_MAX50: 50Hz smooth banding max step=0x13
    ov_write(0x301c,0x02);
    //ov_write(0x301d,0x08); //AECG_MAX60: 60Hz smooth banding max step=0x17
    ov_write(0x301d,0x03);
    //ov_write(0x3070,0x3e); //BD50:LSB
    ov_write(0x3070,0xba); 
    ov_write(0x3072,0x9b); //BD60:LSB

    //D5060
    ov_write(0x30af,0x10);
    ov_write(0x3048,0x1f);
    ov_write(0x3049,0x4e);

    ov_write(0x304a,0x20);
    ov_write(0x304f,0x20);
    ov_write(0x304b,0x02);
    ov_write(0x304c,0x00);
    ov_write(0x304d,0x02);
    ov_write(0x304f,0x20);
    ov_write(0x30a3,0x10);
    ov_write(0x3013,0xf7); 
    ov_write(0x3014,0x25); //R1D bit6 always = 0 , bit[5]=1, bit[0]=1
    //ov_write(0x3014,0x44);

    ov_write(0x3071,0x00);
	//ov_write(0x3070,0xba); //50hz banding
    ov_write(0x3070,0x5d);

    ov_write(0x3073,0x00);
    ov_write(0x3072,0x4d); //60hz banding

	//ov_write(0x301c,0x07); //50hz banding
	//ov_write(0x301d,0x08); //60hz banding
    ov_write(0x301c,0x05); 
    ov_write(0x301d,0x06); 

    ov_write(0x304d,0x42);
	//ov_write(0x304a,0x00); //Disable 50/60 auto detection function, due to ov2650 no this function
	//ov_write(0x304f,0x00); //Disable 50/60 auto detection function, due to ov2650 no this function
    ov_write(0x304a,0x40);
    ov_write(0x304f,0x40); 

	//Window Setup
    ov_write(0x300e,0x38); 


    ov_write(0x3020,0x01); //HS
    ov_write(0x3021,0x18); //HS 0x18 0813
	
    ov_write(0x3022,0x00);
    ov_write(0x3023,0x06);
    ov_write(0x3024,0x06);
    ov_write(0x3025,0x58);
    ov_write(0x3026,0x02);
	//ov_write(0x3027,0x5e);
    ov_write(0x3027,0x61);


    ov_write(0x3088,0x03);
    ov_write(0x3089,0x20);
    ov_write(0x308a,0x02);
    ov_write(0x308b,0x58);
    ov_write(0x3316,0x64);
    ov_write(0x3317,0x25);
    ov_write(0x3318,0x80);
    ov_write(0x3319,0x08);
    ov_write(0x331a,0x64);
    ov_write(0x331b,0x4b);
    ov_write(0x331c,0x00);
    ov_write(0x331d,0x38);
    ov_write(0x3100,0x00);

    //AWB
    //ov_write(0x3320,0x9a); 
    ov_write(0x3320,0xfa);



    ov_write(0x3321,0x11);   
    ov_write(0x3322,0x92);   
    ov_write(0x3323,0x01);   
    //ov_write(0x3324,0x96); // 92	
    ov_write(0x3324,0x97); 

    ov_write(0x3325,0x02);   
    ov_write(0x3326,0xff);   
    ov_write(0x3327,0x0c); // 0f  
    ov_write(0x3328,0x10); // 0f	
    ov_write(0x3329,0x10); // 14	
    //ov_write(0x332a,0x5e); // 66	
    //ov_write(0x332b,0x57);   //5f -> 5c

    ov_write(0x332a,0x58); 
    ov_write(0x332b,0x50);




    ov_write(0x332c,0xbe);   //a5 -> 89
    //ov_write(0x332d,0xce);   //ac -> 96
    //ov_write(0x332e,0x3e);   //35 -> 3d
    //ov_write(0x332f,0x34); // 2f  
    //ov_write(0x3330,0x4b); // 57	
    //ov_write(0x3331,0x42); // 3d	
    //ov_write(0x3332,0xff); // f0	
    //ov_write(0x3333,0x00); // 10 
    ov_write(0x332d,0xe1); 
    ov_write(0x332e,0x43);   
    ov_write(0x332f,0x36); 
    ov_write(0x3330,0x4d); 	
    ov_write(0x3331,0x44); 
    ov_write(0x3332,0xf8); 	
    ov_write(0x3333,0x0a); 


    ov_write(0x3334,0xf0);   
    ov_write(0x3335,0xf0);   
    ov_write(0x3336,0xf0);   
    ov_write(0x3337,0x40);   
    ov_write(0x3338,0x40);   
    ov_write(0x3339,0x40);   
    ov_write(0x333a,0x00);   
    ov_write(0x333b,0x00);   
/////////////////////////////////////////////////////////////////////
    //Color Matrix
    ov_write(0x3380,0x28); //28->2d
    ov_write(0x3381,0x48); //48->4d
    ov_write(0x3382,0x10); //10->13 0819 Joe for �G��
    ov_write(0x3383,0x23);  
    ov_write(0x3384,0xc0);
    ov_write(0x3385,0xe5);  
    ov_write(0x3386,0xc2);  
    ov_write(0x3387,0xb3);  
    ov_write(0x3388,0x0e);  
    ov_write(0x3389,0x98);
    ov_write(0x338a,0x01);

    //Gamma
    ov_write(0x3340,0x0e);  
    ov_write(0x3341,0x1a);  
	
    ov_write(0x3342,0x31);  
    ov_write(0x3343,0x45);
    ov_write(0x3344,0x5a);  
    ov_write(0x3345,0x69);  
    ov_write(0x3346,0x75);  
    ov_write(0x3347,0x7e);  
    ov_write(0x3348,0x88);  
    ov_write(0x3349,0x96);
    ov_write(0x334a,0xa3);
    ov_write(0x334b,0xaf);
    ov_write(0x334c,0xc4);
    ov_write(0x334d,0xd7);
    ov_write(0x334e,0xe8);
    ov_write(0x334f,0x20);


    //Lens correction
    //R
    ov_write(0x3350,0x32);
    ov_write(0x3351,0x25);
    ov_write(0x3352,0x80);
    ov_write(0x3353,0x1e); 	//R-Gain ;0x25
    ov_write(0x3354,0x00);
    ov_write(0x3355,0x85);
    //G
    ov_write(0x3356,0x32);
    ov_write(0x3357,0x25);
    ov_write(0x3358,0x80);
    ov_write(0x3359,0x1b);
    ov_write(0x335a,0x00);
    ov_write(0x335b,0x85);
    //B
    ov_write(0x335c,0x32);
    ov_write(0x335d,0x25);
    ov_write(0x335e,0x80);
    ov_write(0x335f,0x1b);
    ov_write(0x3360,0x00);
    ov_write(0x3361,0x85);

    ov_write(0x3363,0x70);
    ov_write(0x3364,0x7f);
    ov_write(0x3365,0x00);
    ov_write(0x3366,0x00);


    //UVadjust
    ov_write(0x3301,0xff);
    ov_write(0x338B,0x11);
    ov_write(0x338c,0x10);
    ov_write(0x338d,0x40);

    //Sharpness/De-noise
    ov_write(0x3370,0xd0);
    ov_write(0x3371,0x00);
    ov_write(0x3372,0x00);
    ov_write(0x3373,0x40);
    ov_write(0x3374,0x10);
    ov_write(0x3375,0x10);
    ov_write(0x3376,0x04);
    ov_write(0x3377,0x00);
    ov_write(0x3378,0x04);
    ov_write(0x3379,0x80);

    //BLC
    ov_write(0x3069,0x84);
    ov_write(0x307c,0x10);
    ov_write(0x3087,0x02);

    //Other functions
    ov_write(0x3300,0xfc);
    ov_write(0x3302,0x11);
    ov_write(0x3400,0x00);
    ov_write(0x3606,0x20);
    ov_write(0x300e,0x34);
    ov_write(0x3011,0x00);
    ov_write(0x30f3,0x83);
    ov_write(0x304e,0x88);

    ov_write(0x3090,0x03);
    ov_write(0x30aa,0x32);
    ov_write(0x30a3,0x80);
    ov_write(0x30a1,0x41);
    ov_write(0x363b,0x01);
    ov_write(0x363c,0xf2);


    ov_write(0x3086,0x0f);
    ov_write(0x3086,0x00);
	
    ov_write(0x3391,0x00);

    g_Zoom_Level = CamZoom_1_0;
	
    if (sCamI2cStatus != HAL_CAM_SUCCESS)
    {
       HAL_CAM_Trace(HAL_CAM_TRACE_DEVICE|HAL_CAM_TRACE_ERROR, "Init_OV2655(): Error[%d] \r\n", sCamI2cStatus);
       result = sCamI2cStatus;
    }

   return result;
}

static UInt16 OV2655_read_shutter(void)
{
    UInt8 temp_reg1, temp_reg2;
    UInt16 temp_reg, extra_exp_lines;
    HAL_CAM_Trace(HAL_CAM_TRACE_DEVICE|HAL_CAM_TRACE_FUNCTION, "OV2655_read_shutter(): \r\n");

    temp_reg1 = ov_read(0x3003);	// AEC[b7~b0]
    temp_reg2 = ov_read(0x3002);	// AEC[b15~b8]
    temp_reg = (temp_reg1 & 0xFF) | (temp_reg2 << 8);

    temp_reg1 = ov_read(0x302E);	// EXVTS[b7~b0]
    temp_reg2 = ov_read(0x302D);	// EXVTS[b15~b8]
    extra_exp_lines = (temp_reg1 & 0xFF) | (temp_reg2 << 8);
    g_PV_Shutter = temp_reg ;
    g_PV_Extra_Lines = extra_exp_lines;
	return temp_reg + extra_exp_lines;
}	/* OV2650_read_shutter */

/*************************************************************************
* FUNCTION
*	OV2655_SetShutter
*
* DESCRIPTION
*	This function set e-shutter of OV2655 to change exposure time.
*
* PARAMETERS
*	shutter : exposured lines
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/

static void OV2655_set_dummy(UInt16 pixels, UInt16 lines)
{
    UInt8 temp_reg1, temp_reg2;
    UInt16 temp_reg;
	
    HAL_CAM_Trace(HAL_CAM_TRACE_DEVICE|HAL_CAM_TRACE_INFO, "OV2655_set_dummy(): pixels=%d , lines = %d \r\n", pixels,lines);

    ov_write(0x302c,(pixels&0xFF)); //EXHTS[7:0]

    // read out and + line
    temp_reg1 = ov_read(0x302B);	// VTS[b7~b0]
    temp_reg2 = ov_read(0x302A);	// VTS[b15~b8]
    temp_reg = (temp_reg1 & 0xFF) | (temp_reg2 << 8);

    temp_reg += lines;

    ov_write(0x302B,(temp_reg&0xFF));         //VTS[7:0]
    ov_write(0x302A,((temp_reg&0xFF00)>>8));  //VTS[15:8]
}	/* OV2655_set_dummy */

/*************************************************************************
* FUNCTION
*	OV2655_NightMode
*
* DESCRIPTION
*	This function night mode of OV2655.
*
* Note:
*************************************************************************/
static void OV2655_NightMode(bool enable)
{
    UInt8 night = ov_read(0x3014); //bit[3], 0: disable, 1:enable
    UInt8 delayTime = 0; 
   
    if (enable) 
    {
         /* Camera mode only */                
        // set Max gain to 16X
        ov_write(0x3015, 0x22);  //jerry 0624 for night mode
        ov_write(0x3014, night | 0x08); //Enable night mode
        ov_write(0x3011, 0x00);           
        delayTime = 150;
    }
    else 
    {
        /* when enter normal mode (disable night mode) without light, the AE vibrate */
        ov_write(0x3011, 0x00);

        /* Camera mode only */
        // set Max gain to 4X
        ov_write(0x3015, 0x02);    //0x22
        //ov_write(0x3015, 0x12);  //Wonder
        ov_write(0x3014, night & 0xf7); //Disable night mode
        //ov_write(0x3014, night | 0x08);
        // clear extra exposure line
        ov_write(0x302d, 0x00);
        ov_write(0x302e, 0x00);
        delayTime = 100;
    }    


    msleep(delayTime);
    HAL_CAM_Trace(HAL_CAM_TRACE_DEVICE|HAL_CAM_TRACE_INFO, "OV2655_NightMode(): enable=%d \r\n", enable);
}	/* OV2655_NightMode */

/****************************************************************************/
//
// Function Name:	HAL_CAM_Result_en_t OV2655_SetClockPolarity(CamSensorSelect_t sensor)
//
// Description: This function set the polarity of VSync,H-Sync and PCLK
//
// Notes:
//
//****************************************************************************
static HAL_CAM_Result_en_t OV2655_SetClockPolarity(CamSensorSelect_t sensor)
{
	HAL_CAM_Result_en_t result   = HAL_CAM_SUCCESS;
	UInt8               Polarity = ov_read(0x3014);
	
    // Bit [0] Control VSync Polarity
    //         1 :Data valid VSYNC High ;
    //         0 :Data valid VSYNC Low; 
    // Bit [1] HREF(H-Sync)Polarity
    //         1 :Data valid HSYNC High ;
	//         0 :Data valid VSYNC Low ;
    // Bit [2] Control PCLK Polarity 
    //         1 :Data update at falling edge ;
    //         0:Data update at rising edge    
    
    // 000:  V-sync active low;H-Sync active High ;PClock rising edge
    Polarity &= 0xF8;
	ov_write(0x3600,Polarity);
	
    if (sCamI2cStatus != HAL_CAM_SUCCESS)
    {          
       HAL_CAM_Trace(HAL_CAM_TRACE_DEVICE|HAL_CAM_TRACE_ERROR, "OV2655_SetClockPolarity(): Error[%d] \r\n", sCamI2cStatus);
       result = sCamI2cStatus;
    }

	return result;
}
/*************************************************************************
* FUNCTION
*	OV2650_Computer_AECAGC
*
* DESCRIPTION
*	Re-calculate UXGA Exposure. 
*
* Note:
*************************************************************************/

static void OV2650_Computer_AECAGC()
{
    UInt16 PV_Line_Width,Capture_Line_Width,Capture_Maximum_Shutter,Capture_Gain16,Capture_Exposure;
    UInt32 Capture_Banding_Filter,Gain_Exposure=0;
    UInt8  Reg3014=0;

    PV_Line_Width = PV_PERIOD_PIXEL_NUM + g_PV_Dummy_Pixels;	
    Capture_Line_Width = FULL_PERIOD_PIXEL_NUMS + g_Capture_Dummy_Pixels;
    Capture_Maximum_Shutter = FULL_EXPOSURE_LIMITATION + g_Capture_Dummy_Lines;
    Reg3014 = ov_read(0x3014);
    //0x3014 => Bit[7] 0:60Hz 1:50Hz
    if((Reg3014&0x80)==0)
    {	//Banding 60Hz
    	Capture_Banding_Filter = g_Capture_PCLK_Frequency*1000000/(120*(2*Capture_Line_Width));		
    }
    else
    {	//Banding 50Hz
    	Capture_Banding_Filter = g_Capture_PCLK_Frequency*1000000/(100*(2*Capture_Line_Width));
    }
    /*	 Gain_Exposure = g_PV_Gain16*(g_PV_Shutter+g_PV_Extra_Lines)*PV_Line_Width/g_Preview_PCLK_Frequency/Capture_Line_Width*g_Capture_PCLK_Frequency
    ;*/
    g_PV_Gain16 = read_OV2650_gain();

    Gain_Exposure = g_PV_Gain16;	
    Gain_Exposure *=(g_PV_Shutter+g_PV_Extra_Lines);
    Gain_Exposure *= PV_Line_Width;
    //Gain_Exposure = g_PV_Gain16 * PV_Line_Width*(g_PV_Shutter+g_PV_Extra_Lines);	//970
    //Gain_Exposure /=g_Preview_PCLK_Frequency;
    Gain_Exposure /=Capture_Line_Width;//1940
    if(PrimaryFrameRate_st.cur_setting==CamRate_15)
    {
       Gain_Exposure = Gain_Exposure*g_Capture_PCLK_Frequency/18;// for clock
    }
    else
    {
       Gain_Exposure = Gain_Exposure*g_Capture_PCLK_Frequency/36;// for clock
    }
    //redistribute gain and exposure
    if (Gain_Exposure < Capture_Banding_Filter * 16)	 // Exposure < 1/100/120
    {
    	if(Gain_Exposure<16){//exposure line smaller than 2 lines and gain smaller than 0x08 
    	    Gain_Exposure = Gain_Exposure*4;	 
    	    Capture_Exposure = 1;
    	    Capture_Gain16 = (Gain_Exposure*2 + 1)/(Capture_Exposure*2*4);
    	}
    	else
    	{
    	    Capture_Exposure = Gain_Exposure /16;
    	    Capture_Gain16 = (Gain_Exposure*2 + 1)/(Capture_Exposure*2);
    	}
    }
    else 
    {
    	if (Gain_Exposure > Capture_Maximum_Shutter * 16) // Exposure > Capture_Maximum_Shutter
    	{
		   
    	    Capture_Exposure = Capture_Maximum_Shutter/Capture_Banding_Filter*Capture_Banding_Filter;
    	    Capture_Gain16 = (Gain_Exposure*2 + 1)/(Capture_Exposure*2);
    	    if (Capture_Gain16 > g_Capture_Max_Gain16) 
    	    {
    	        // gain reach maximum, insert extra line
    	        Capture_Exposure = (Gain_Exposure*11)/(g_Capture_Max_Gain16*10);
				
    	        // Exposure = n/100/120
    	        Capture_Exposure = Capture_Exposure/Capture_Banding_Filter;
    	        Capture_Exposure =Capture_Exposure * Capture_Banding_Filter;
    	        Capture_Gain16 = ((Gain_Exposure*4)/Capture_Exposure+3)/4;
    	    }
    	}
    	else  // 1/100 < Exposure < Capture_Maximum_Shutter, Exposure = n/100/120
    	{
     	    Capture_Exposure = Gain_Exposure/(16*Capture_Banding_Filter);
    	    Capture_Exposure = Capture_Exposure * Capture_Banding_Filter;
    	    Capture_Gain16 = (Gain_Exposure*2 +1) / (Capture_Exposure*2);
    	}
    }
	
    g_Capture_Gain16 = Capture_Gain16;
    g_Capture_Extra_Lines = (Capture_Exposure > Capture_Maximum_Shutter)?
    	    (Capture_Exposure - Capture_Maximum_Shutter/Capture_Banding_Filter*Capture_Banding_Filter):0;	  
	
    g_Capture_Shutter = Capture_Exposure - g_Capture_Extra_Lines;
    
    HAL_CAM_Trace(HAL_CAM_TRACE_DEVICE|HAL_CAM_TRACE_INFO, "OV2650_Computer_AECAGC()\r\n");
}


/*************************************************************************
* FUNCTION
*	write_OV2650_gain
*
* DESCRIPTION
*	Set Gain. 
*
* Note:
*************************************************************************/

static void write_OV2650_gain(UInt16 gain)
{
   UInt16 temp_reg;
   
   if(gain > 248)
   	 return;
   
    temp_reg = 0;
    if (gain > 31)
    {
    	temp_reg |= 0x10;
    	gain = gain >> 1;
    }
    if (gain > 31)
    {
    	temp_reg |= 0x20;
    	gain = gain >> 1;
    }

    if (gain > 31)
    {
    	temp_reg |= 0x40;
    	gain = gain >> 1;
    }
    if (gain > 31)
    {
    	temp_reg |= 0x80;
    	gain = gain >> 1;
    }
	
    if (gain > 16)
    {
    	temp_reg |= ((gain -16) & 0x0f);
    }	
  
    ov_write(0x3000,temp_reg);
}  /* write_OV2650_gain */
/*************************************************************************
* FUNCTION
*	write_OV2650_gain
*
* DESCRIPTION
*	Read Gain. 
*
* Note:
*************************************************************************/

static UInt16 read_OV2650_gain(void)
{
   UInt8  temp_reg;
   UInt16 sensor_gain;
   
    temp_reg=ov_read(0x3000);  
      
    sensor_gain=(16+(temp_reg&0x0F));

    if(temp_reg&0x10)
    	sensor_gain<<=1;
	
    if(temp_reg&0x20)
    	sensor_gain<<=1;
	  
    if(temp_reg&0x40)
    	sensor_gain<<=1;
	  
    if(temp_reg&0x80)
    	sensor_gain<<=1;
	  
    return sensor_gain;
}  /* read_OV2650_gain */

static void OV2650_write_shutter(UInt16 shutter)
{
/*
    if (gPVmode == false) {
	if (shutter <= PV_EXPOSURE_LIMITATION) {
     	    extra_exposure_lines = 0;
	}else {
     	    extra_exposure_lines=shutter - PV_EXPOSURE_LIMITATION;
	}

	if (shutter > PV_EXPOSURE_LIMITATION) {
     	    shutter = PV_EXPOSURE_LIMITATION;
		}
    }
    else
*/
    {
        if (shutter <= FULL_EXPOSURE_LIMITATION) {
            extra_exposure_lines = 0;
        }else {
            extra_exposure_lines = shutter - FULL_EXPOSURE_LIMITATION;
        }

        if (shutter > FULL_EXPOSURE_LIMITATION) {
        shutter = FULL_EXPOSURE_LIMITATION;
        }
    }

    // set extra exposure line
    ov_write(0x302E, extra_exposure_lines & 0xFF); 		 // EXVTS[b7~b0]
    ov_write(0x302D, (extra_exposure_lines & 0xFF00) >> 8); // EXVTS[b15~b8]

    /* Max exporsure time is 1 frmae period event if Tex is set longer than 1 frame period */
    ov_write(0x3003, shutter & 0xFF);			 //AEC[7:0]
    ov_write(0x3002, (shutter & 0xFF00) >> 8);  //AEC[8:15]

}	 /* OV2650_write_shutter */

static HAL_CAM_Result_en_t ov_write(unsigned int sub_addr, unsigned char data)
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;

    result |= CAM_WriteI2c( sub_addr, 1, &data );
    if (result != HAL_CAM_SUCCESS)
    {
        sCamI2cStatus = result; 
        HAL_CAM_Trace(HAL_CAM_TRACE_DEVICE|HAL_CAM_TRACE_ERROR, "ov_write(): ERROR: \r\n");
    }
    return result;
}

static UInt8 ov_read(unsigned int sub_addr)
{
    HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
    UInt8 data;
    
    result |= CAM_ReadI2c( sub_addr, 1, &data );
    if (result != HAL_CAM_SUCCESS)
    {
        sCamI2cStatus = result; 
        HAL_CAM_Trace(HAL_CAM_TRACE_DEVICE|HAL_CAM_TRACE_ERROR, "ov_read(): ERROR: \r\n");
    }

    return data;
}

