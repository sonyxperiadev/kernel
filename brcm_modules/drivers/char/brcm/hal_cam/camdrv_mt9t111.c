/******************************************************************************
Copyright 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement
governing use of this software, this software is licensed to you under the
terms of the GNU General Public License version 2, available at
http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software
in any way with any other Broadcom software provided under a license other than
the GPL, without Broadcom's express prior written consent.
******************************************************************************/

/**
*
*   @file   camdrv_mt9t111.c
*
*   @brief  This file is the lower level driver API of MT9T111(3M 2048*1536
*	Pixel) ISP/sensor.
*
*/
/**
 * @addtogroup CamDrvGroup
 * @{
 */

  /****************************************************************************/
  /*                          Include block                                   */
  /****************************************************************************/
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

#define MT9T111_ID 0x2680

#define HAL_CAM_RESET 22

/* Start of Embedded Image (default value) */
#define SOEI	0xBEFF

/* Start of Status Information */
#define SOSI	0xBCFF

#ifndef ALLOC_TN_BUFFER
#define ALLOC_TN_BUFFER 1
#define TN_BUFFER_SIZE (320 * 240 * 2)	/* for storing thumbnail data. */
int tn_width, tn_height;
int jpeg_width, jpeg_height;

static char *thumbnail_data;
static char *jpeg_header_data;
static char *jpeg_raw_data;

struct DataFormat {
	/*Copy of the variable */
	unsigned short CONFIG_JPEG_OB_TX_CONTROL_VAR;
	/*Copy of the variable */
	unsigned short CONFIG_JPEG_OB_SPOOF_CONTROL_VAR;
	/*Spoof Width used when the spoof mode is enabled */
	unsigned int spoof_width;
	/*Spoof Height used when the spoof mode is enabled */
	unsigned int spoof_heigth;
	/*Status used to define if thumbnail is embedded */
	unsigned char IS_TN_ENABLED;
	/*Thumbnail width used when the thumbnail is enabled */
	unsigned int thumbnail_width;
	/*Thumbnail height used when the thumbnail is enabled */
	unsigned int thumbnail_height;
	/*Load the offset of the last valid address */
	unsigned int JpegSize;
	/*Tell which Qscale to use. */
	unsigned short JpegStatus;
	/*0 for Monochrome, 2 for YUV422 and 3 for YUV420 */
	unsigned char JpegFormat;
	/*Jpeg Resolution */
	unsigned short JpegXsize;
	/*Jpeg resolution */
	unsigned short JpegYsize;
};

const unsigned char JFIFHeaderSize = 18;
const unsigned char LumaQtableSize = 64;
const unsigned char LumaHuffTableSizeDC = 33;
const unsigned char LumaHuffTableSizeAC = 183;
const unsigned char QTableHeaderSize = 4;
const unsigned char ChromaQtableSize = 64;
const unsigned char ChromaHuffTableSizeAC = 183;
const unsigned char ChromaHuffTableSizeDC = 33;
const unsigned char RestartIntervalSize = 6;
unsigned char StartOfFrameSize = 19;
const unsigned char StartOfScanSize = 14;
const unsigned char StartOfScanSizeM = 10;

const unsigned char JFIFHeader[18] = { 0xFF,	/* APP0 marker */
	0xE0,
	0x00,			/* length */
	0x10,
	0x4A,			/* JFIF identifier */
	0x46,
	0x49,
	0x46,
	0x00,
	0x01,			/* JFIF version */
	0x02,
	0x00,			/* units */
	0x00,			/* X density */
	0x01,
	0x00,			/* Y density */
	0x01,
	0x00,			/* X thumbnail */
	0x00			/* Y thumbnail */
};

unsigned char QTableHeader[4] = {
	0xff, 0xDB,		/*Qtable marker */
	0x00, 0x84,		/*Qtable size for 2 tables (only one for monochrome) */
};

const unsigned char JPEG_StdQuantTblY_ZZ[64] = {
	16, 11, 12, 14, 12, 10, 16, 14,
	13, 14, 18, 17, 16, 19, 24, 40,
	26, 24, 22, 22, 24, 49, 35, 37,
	29, 40, 58, 51, 61, 60, 57, 51,
	56, 55, 64, 72, 92, 78, 64, 68,
	87, 69, 55, 56, 80, 109, 81, 87,
	95, 98, 103, 104, 103, 62, 77, 113,
	121, 112, 100, 120, 92, 101, 103, 99
};

const unsigned char JPEG_StdQuantTblC_ZZ[64] = {
	17, 18, 18, 24, 21, 24, 47, 26,
	26, 47, 99, 66, 56, 66, 99, 99,
	99, 99, 99, 99, 99, 99, 99, 99,
	99, 99, 99, 99, 99, 99, 99, 99,
	99, 99, 99, 99, 99, 99, 99, 99,
	99, 99, 99, 99, 99, 99, 99, 99,
	99, 99, 99, 99, 99, 99, 99, 99,
	99, 99, 99, 99, 99, 99, 99, 99
};

unsigned char quant_JPEG_Hdr[2 /*luma/chroma */][64] =
{
	0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x10, 0x0f, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0x00, 0x10,
	0x00, 0x00, 0x10, 0x10, 0x13, 0x0c, 0x0e, 0x10,
	0x00, 0x00, 0x10, 0x10, 0x11, 0x13, 0x13, 0x13,
	0x00, 0x00, 0x00, 0x00, 0x04, 0x05, 0x09, 0x05,
	0x00, 0x00, 0x10, 0x00, 0x0b, 0x0c, 0x13, 0x13,
	0x00, 0x00, 0x10, 0x10, 0x13, 0x13, 0x13, 0x13,
	0x00, 0x00, 0x10, 0x13, 0x13, 0x13, 0x13, 0x13,
	0x00, 0x00, 0x10, 0x13, 0x13, 0x13, 0x13, 0x13,
	0x00, 0x00, 0x10, 0x13, 0x13, 0x13, 0x13, 0x13,
	0x00, 0x00, 0x10, 0x13, 0x13, 0x13, 0x13, 0x13,
	0x00, 0x00, 0x10, 0x13, 0x13, 0x13, 0x13, 0x13
};

const unsigned char LumaHuffTableDC[33] = {
	0xff, 0xc4,
	0x00, 0x1f,
	/* Luma DC Table */
	0x00,
	0x00, 0x01, 0x05, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0a, 0x0b
};

const unsigned char LumaHuffTableAC[183] = {
	0xff, 0xc4,
	0x00, 0xb5,
	/* Luma AC Table */
	0x10,
	0x00, 0x02, 0x01, 0x03, 0x03, 0x02, 0x04, 0x03,
	0x05, 0x05, 0x04, 0x04, 0x00, 0x00, 0x01, 0x7d,
	0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12,
	0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07,
	0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xa1, 0x08,
	0x23, 0x42, 0xb1, 0xc1, 0x15, 0x52, 0xd1, 0xf0,
	0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0a, 0x16,
	0x17, 0x18, 0x19, 0x1a, 0x25, 0x26, 0x27, 0x28,
	0x29, 0x2a, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
	0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
	0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
	0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
	0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
	0x7a, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
	0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98,
	0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
	0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6,
	0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3, 0xc4, 0xc5,
	0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4,
	0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe1, 0xe2,
	0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea,
	0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
	0xf9, 0xfa
};

const unsigned char ChromaHuffTableDC[33] = {
	0xff, 0xc4,
	0x00, 0x1f,
	/* Chroma DC Table */
	0x01,
	0x00, 0x03, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0a, 0x0b
};

const unsigned char ChromaHuffTableAC[183] = {
	0xff, 0xc4,
	0x00, 0xb5,
	/* Chroma AC Table */
	0x11,
	0x00, 0x02, 0x01, 0x02, 0x04, 0x04, 0x03, 0x04,
	0x07, 0x05, 0x04, 0x04, 0x00, 0x01, 0x02, 0x77,
	0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21,
	0x31, 0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71,
	0x13, 0x22, 0x32, 0x81, 0x08, 0x14, 0x42, 0x91,
	0xa1, 0xb1, 0xc1, 0x09, 0x23, 0x33, 0x52, 0xf0,
	0x15, 0x62, 0x72, 0xd1, 0x0a, 0x16, 0x24, 0x34,
	0xe1, 0x25, 0xf1, 0x17, 0x18, 0x19, 0x1a, 0x26,
	0x27, 0x28, 0x29, 0x2a, 0x35, 0x36, 0x37, 0x38,
	0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
	0x49, 0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
	0x59, 0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
	0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
	0x79, 0x7a, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
	0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95, 0x96,
	0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5,
	0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4,
	0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3,
	0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2,
	0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda,
	0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9,
	0xea, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
	0xf9, 0xfa
};

/* DRI : Restart Interval = 0 MCU */
const unsigned char RestartInterval[6] = {
	0xff, 0xdd,
	0x00, 0x04,
	0x00, 0x00
};

const unsigned char StartOfFrame[19] = {
	0xff, 0xC0,		/*SOF Marker */
	0x00, 0x11,		/*SOF length */
	0x08,			/*Number of bits per pixel */
	0x06, 0x00, 0x08, 0x00,	/*Image resolution Y and X */
	0x03,			/*Number of components */
	0x00, 0x21, 0x00,	/* Y parameter: 0x21 for YUV422 0x22 for
				   YUV420 and related Qtable */
	0x01, 0x11, 0x01,	/* Cb parameter and related Qtable */
	0x02, 0x11, 0x01	/* Cr parameter and related Qtable */
};

const unsigned char StartOfScanM[10] = {
	0xff, 0xDA,		/*SOS Marker */
	0x00, 0x08,		/*SOS Length */
	0x01,			/*Number of components */
	0x00, 0x00,		/*Y parameter and Huffman table selector */
	0x00, 0x3F,
	0x00
};

const unsigned char StartOfScan[14] = {
	0xff, 0xDA,		/*SOS Marker */
	0x00, 0x0C,		/*SOS Length */
	0x03,			/*Number of components */
	0x00, 0x00,		/*Y parameter and Huffman table selector */
	0x01, 0x11,		/*Cb parameter and Huffman table selector */
	0x02, 0x11,		/*Cr parameter and Huffman table selector */
	0x00, 0x3F,
	0x00
};

#endif
/*****************************************************************************/
/* start of CAM configuration */
/*****************************************************************************/
/* Sensor select */
typedef enum {
	SensorMT9T111,
	SensorUnused
} CamSensor_t;

/*****************************************************************************/
/*  Sensor Resolution Tables:												 */
/*****************************************************************************/
/* Resolutions & Sizes available for MT9T111 ISP/Sensor (QXGA max) */
static CamResolutionConfigure_t sSensorResInfo_MT9T111_st[] = {
/* width    height  Preview_Capture_Index       Still_Capture_Index */
	{128, 96, CamImageSize_SQCIF, -1},	/* CamImageSize_SQCIF */
	{160, 120, CamImageSize_QQVGA, -1},	/* CamImageSize_QQVGA */
	{176, 144, CamImageSize_QCIF, CamImageSize_QCIF},	/* CamImageSize_QCIF */
	{240, 180, -1, -1},	/* CamImageSize_240x180 */
	{240, 320, -1, -1},	/* CamImageSize_R_QVGA */
	{320, 240, CamImageSize_QVGA, CamImageSize_QVGA},	/* CamImageSize_QVGA */
	{352, 288, CamImageSize_CIF, CamImageSize_CIF},	/* CamImageSize_CIF */
	{426, 320, -1, -1},	/* CamImageSize_426x320 */
	{640, 480, CamImageSize_VGA, CamImageSize_VGA},	/* CamImageSize_VGA */
	{800, 600, CamImageSize_SVGA, CamImageSize_SVGA},	/* CamImageSize_SVGA */
	{1024, 768, -1, CamImageSize_XGA},	/* CamImageSize_XGA */
	{1280, 960, -1, -1},	/* CamImageSize_4VGA */
	{1280, 1024, -1, CamImageSize_SXGA},	/* CamImageSize_SXGA */
	{1600, 1200, -1, CamImageSize_UXGA},	/* CamImageSize_UXGA */
	{2048, 1536, -1, CamImageSize_QXGA},	/* CamImageSize_QXGA */
	{2560, 2048, -1, -1},	/* CamImageSize_QSXGA */
  	{144,	  176,   CamImageSize_R_QCIF,	CamImageSize_R_QCIF  } 	  //  CamImageSize_R_QCIF	 
};

/*****************************************************************************/
/*  Power On/Off Tables for Main Sensor */
/*****************************************************************************/

/*---------Sensor Power On */
static CamSensorIntfCntrl_st_t CamPowerOnSeq[] = {
	{PAUSE, 1, Nop_Cmd},

	{GPIO_CNTRL, HAL_CAM_RESET, GPIO_SetHigh},
	{PAUSE, 10, Nop_Cmd},

/* -------Turn everything OFF   */
	{GPIO_CNTRL, HAL_CAM_RESET, GPIO_SetLow},
	{MCLK_CNTRL, CamDrv_NO_CLK, CLK_TurnOff},

/* -------Enable Clock to Cameras @ Main clock speed*/
	{MCLK_CNTRL, CamDrv_26MHz, CLK_TurnOn},
	{PAUSE, 10, Nop_Cmd},

/* -------Raise Reset to ISP*/
	{GPIO_CNTRL, HAL_CAM_RESET, GPIO_SetHigh},
	{PAUSE, 10, Nop_Cmd}

};

/*---------Sensor Power Off*/
static CamSensorIntfCntrl_st_t CamPowerOffSeq[] = {
	/* No Hardware Standby available. */
	{PAUSE, 50, Nop_Cmd},
/* -------Lower Reset to ISP*/
	{GPIO_CNTRL, HAL_CAM_RESET, GPIO_SetLow},
/* -------Disable Clock to Cameras*/
	{MCLK_CNTRL, CamDrv_NO_CLK, CLK_TurnOff},
};

/** Primary Sensor Configuration and Capabilities  */
static HAL_CAM_IntConfigCaps_st_t CamPrimaryCfgCap_st = {
	/* CamSensorOutputCaps_st_t */
	{
	 CamDataFmtYCbCr |	/*< UInt32 formats;   CamDataFmt_t bit masked */
	 CamDataFmtJPEG,
	 2048,			/*< UInt32 max_width;   Maximum width resolution */
	 1536,			/*< UInt32 max_height;  Maximum height resolution */
	 CamImageSize_VGA,	/*< UInt32 ViewFinderResolutions;  ViewFinder
				   Resolutions (Maximum Resolution for now) */
	 CamImageSize_VGA,	/*< UInt32 VideoResolutions;  Video Resolutions
				   (Maximum Resolution for now) */
	 CamImageSize_QXGA,	/*< UInt32 StillsResolutions;  Stills Resolutions
				   (Maximum Resolution for now) */
	 1,			/*< UInt32 pre_frame_video;  frames to throw out for
				   ViewFinder/Video capture (total= pre_frame_video+1 */
	 3,			/*< UInt32 pre_frame_still;  frames to throw out for
				   Stills capture (total= pre_frame_still+1 */
	 TRUE,			/*< Boolean JpegCapable;     Sensor Jpeg
				   Capable: TRUE/FALSE: */
	 TRUE,			/*< Boolean StillnThumbCapable;
				   Sensor Still and Thumbnail Capable: TRUE/FALSE: */
	 TRUE			/*< Boolean VideonViewfinderCapable;
				   Sensor Video and Viewfinder Capable: TRUE/FALSE: */
	 },

	{
	 CamFocusControlOff,	/*< CamFocusControlMode_t
				   default_setting=CamFocusControlOff; */
	 CamFocusControlOff,	/*< CamFocusControlMode_t cur_setting; */
	 CamFocusControlOff	/*< UInt32 settings;  Settings Allowed:
				   CamFocusControlMode_t bit masked */
	 },

	/*< Digital Zoom Settings & Capabilities:
	   CamDigitalZoomMode_st_t digital_zoom_st; */
	{
	 CamZoom_1_0,		/*< CamZoom_t default_setting;
				   default=CamZoom_1_0:  Values allowed  CamZoom_t */
	 CamZoom_1_0,		/*< CamZoom_t cur_setting;  CamZoom_t */
	 CamZoom_1_0,		/*< CamZoom_t max_zoom;
				   Max Zoom Allowed(256/max_zoom = *zoom) */
	 FALSE			/*< Boolean capable;  Sensor capable: TRUE/FALSE: */
	 },

	/*< Sensor ESD Settings & Capabilities:  CamESD_st_t esd_st; */
	{
	 0x01,			/*< UInt8 ESDTimer;  Periodic timer to retrieve
				   the camera status (ms) */
	 FALSE			/*< Boolean capable;  TRUE/FALSE: */
	 },

	/*< Sensor version string */
	"MT9T111"
};

/*---------Sensor Primary Configuration CCIR656*/
static CamIntfConfig_CCIR656_st_t CamPrimaryCfg_CCIR656_st = {
/* Vsync, Hsync, Clock */
	TRUE,			/*[00] Boolean ext_sync_enable;
				   (default)TRUE: CCIR-656 with external VSYNC/HSYNC
				   FALSE: Embedded VSYNC/HSYNC */
	TRUE,			/*[01] Boolean hsync_control;
				   (default)FALSE: FALSE=all HSYNCS
				   TRUE: HSYNCS only during valid VSYNC  */
	SyncFallingEdge,	/*[02] UInt32 vsync_irq_trigger;
				   (default)SyncRisingEdge/SyncFallingEdge:
				   Vsync Irq trigger    */
	SyncActiveHigh,		/*[03] UInt32 vsync_polarity;
				   (default)SyncActiveLow/SyncActiveHigh:
				   Vsync active  */
	SyncFallingEdge,	/*[04] UInt32 hsync_irq_trigger;
				   (default)SyncRisingEdge/SyncFallingEdge:
				   Hsync Irq trigger */
	SyncActiveHigh,		/*[05] UInt32 hsync_polarity;
				   (default)SyncActiveLow/SyncActiveHigh:
				   Hsync active    */
	SyncFallingEdge		/*[06] UInt32 data_clock_sample;
				   (default)SyncRisingEdge/SyncFallingEdge:
				   Pixel Clock Sample edge */
};

/*---------Sensor Primary Configuration YCbCr Input*/
static CamIntfConfig_YCbCr_st_t CamPrimaryCfg_YCbCr_st = {
/* YCbCr(YUV422) Input format = YCbCr=YUV= Y0 U0 Y1 V0  Y2 U2 Y3 V2 ....*/
	TRUE,			/*[00] Boolean yuv_full_range;
				   (default)FALSE: CROPPED YUV=16-240
				   TRUE: FULL RANGE YUV= 1-254  */
	SensorYCSeq_YCbYCr,	/*[01] CamSensorYCbCrSeq_t sensor_yc_seq;
				   (default) SensorYCSeq_YCbYCr */

/* Currently Unused */
	FALSE,			/*[02] Boolean input_byte_swap;
				   Currently UNUSED!! (default)FALSE:  TRUE: */
	FALSE,			/*[03] Boolean input_word_swap;
				   Currently UNUSED!! (default)FALSE:  TRUE: */
	FALSE,			/*[04] Boolean output_byte_swap;
				   Currently UNUSED!! (default)FALSE:  TRUE: */
	FALSE,			/*[05] Boolean output_word_swap;
				   Currently UNUSED!! (default)FALSE:  TRUE: */

/* Sensor olor Conversion Coefficients:
	color conversion fractional coefficients are scaled by 2^8 */
/* e.g. for R0 = 1.164, round(1.164 * 256) = 298 or 0x12a */
	CAM_COLOR_R1R0,		/*[06] UInt32 cc_red R1R0;
				   YUV422 to RGB565 color conversion red */
	CAM_COLOR_G1G0,		/*[07] UInt32 cc_green G1G0;
				   YUV422 to RGB565 color conversion green */
	CAM_COLOR_B1		/*[08] UInt32 cc_blue B1;
				   YUV422 to RGB565 color conversion blue */
};

/*---------Sensor Primary Configuration I2C */
static CamIntfConfig_I2C_st_t CamPrimaryCfg_I2C_st = {
	0x00,			/*I2C_SPD_430K, [00] UInt32 i2c_clock_speed;
				   max clock speed for I2C */
	I2C_CAM_DEVICE_ID,	/*[01] UInt32 i2c_device_id; I2C device ID */
	0x00,			/*I2C_BUS2_ID, [02] I2C_BUS_ID_t i2c_access_mode;
				   I2C baseband port */
	0x00,			/*I2CSUBADDR_16BIT,[03] I2CSUBADDR_t i2c_sub_adr_op;
				   I2C sub-address size */
	0xFFFF,			/*[04] UInt32 i2c_page_reg;
				   I2C page register addr (if applicable)
				   **UNUSED by this driver** */
	I2C_CAM_MAX_PAGE	/*[05] UInt32 i2c_max_page; I2C max page
				   (if not used by camera drivers, set = 0xFFFF)
				   **UNUSED by this driver** */
};

/*---------Sensor Primary Configuration IOCR */
static CamIntfConfig_IOCR_st_t CamPrimaryCfg_IOCR_st = {
	FALSE,			/*[00] Boolean cam_pads_data_pd;
				   (default)FALSE: IOCR2 D0-D7 pull-down disabled
				   TRUE: IOCR2 D0-D7 pull-down enabled */
	FALSE,			/*[01] Boolean cam_pads_data_pu;
				   (default)FALSE: IOCR2 D0-D7 pull-up disabled
				   TRUE: IOCR2 D0-D7 pull-up enabled */
	FALSE,			/*[02] Boolean cam_pads_vshs_pd;
				   (default)FALSE: IOCR2 Vsync/Hsync pull-down disabled
				   TRUE: IOCR2 Vsync/Hsync pull-down enabled */
	FALSE,			/*[03] Boolean cam_pads_vshs_pu;
				   (default)FALSE: IOCR2 Vsync/Hsync pull-up disabled
				   TRUE: IOCR2 Vsync/Hsync pull-up enabled */
	FALSE,			/*[04] Boolean cam_pads_clk_pd;
				   (default)FALSE: IOCR2 CLK pull-down disabled
				   TRUE: IOCR2 CLK pull-down enabled */
	FALSE,			/*[05] Boolean cam_pads_clk_pu;
				   (default)FALSE: IOCR2 CLK pull-up disabled
				   TRUE: IOCR2 CLK pull-up enabled */

	7 << 12,		/*[06] UInt32 i2c_pwrup_drive_strength;
				   (default)IOCR4_CAM_DR_12mA:
				   IOCR drive strength */
	0x00,			/*[07] UInt32 i2c_pwrdn_drive_strength;
				   (default)0x00:    I2C2 disabled */
	0x00,			/*[08] UInt32 i2c_slew; (default) 0x00: 42ns */

	7 << 12,		/*[09] UInt32 cam_pads_pwrup_drive_strength;
				   (default)IOCR4_CAM_DR_12mA:  IOCR drive strength */
	1 << 12			/*[10] UInt32 cam_pads_pwrdn_drive_strength;
				   (default)IOCR4_CAM_DR_2mA:   IOCR drive strength */
};

/* XXXXXXXXXXXXXXXXXXXXXXXXXXX IMPORTANT XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX */
/* TO DO: MURALI */
/* HAVE TO PROGRAM THIS IN THE ISP. */
/*---------Sensor Primary Configuration JPEG */
static CamIntfConfig_Jpeg_st_t CamPrimaryCfg_Jpeg_st = {
	1200,			/*< UInt32 jpeg_packet_size_bytes;     Bytes/Hsync */
	1360,			/*< UInt32 jpeg_max_packets;           Max Hsyncs/Vsync = (3.2Mpixels/4) / 512 */
	CamJpeg_FixedPkt_VarLine,	/*< CamJpegPacketFormat_t pkt_format;  Jpeg Packet Format */
};

/* XXXXXXXXXXXXXXXXXXXXXXXXXXX IMPORTANT XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX */
/* TO DO: MURALI */
/* WILL NEED TO MODIFY THIS. */
/*---------Sensor Primary Configuration Stills n Thumbs */
static CamIntfConfig_PktMarkerInfo_st_t CamPrimaryCfg_PktMarkerInfo_st = {
	2,			/*< UInt8       marker_bytes; # of bytes for marker,
				   (= 0 if not used) */
	0,			/*< UInt8       pad_bytes; # of bytes for padding,
				   (= 0 if not used) */
	TRUE,			/*< Boolean     TN_marker_used; Thumbnail marker used */
	0xFFBE,			/*< UInt32      SOTN_marker; Start of Thumbnail marker,
				   (= 0 if not used) */
	0xFFBF,			/*< UInt32      EOTN_marker; End of Thumbnail marker,
				   (= 0 if not used) */
	TRUE,			/*< Boolean     SI_marker_used; Status Info marker used */
	0xFFBC,			/*< UInt32      SOSI_marker; Start of Status Info marker,
				   (= 0 if not used) */
	0xFFBD,			/*< UInt32      EOSI_marker; End of Status Info marker,
				   (= 0 if not used) */
	FALSE,			/*< Boolean     Padding_used; Status Padding bytes used */
	0x0000,			/*< UInt32      SOPAD_marker; Start of Padding marker,
				   (= 0 if not used) */
	0x0000,			/*< UInt32      EOPAD_marker; End of Padding marker,
				   (= 0 if not used) */
	0x0000			/*< UInt32      PAD_marker; Padding Marker,
				   (= 0 if not used) */
};

/*---------Sensor Primary Configuration Stills n Thumbs */
static CamIntfConfig_InterLeaveMode_st_t CamPrimaryCfg_InterLeaveStills_st = {
	CamInterLeave_SingleFrame,	/*< CamInterLeaveMode_t mode;
					   Interleave Mode */
	CamInterLeave_PreviewLast,	/*< CamInterLeaveSequence_t sequence;
					   InterLeaving Sequence */
	CamInterLeave_PktFormat	/*< CamInterLeaveOutputFormat_t format;
				   InterLeaving Output Format */
};

/*---------Sensor Primary Configuration */
static CamIntfConfig_st_t CamSensorCfg_st = {
	&CamPrimaryCfgCap_st,	/* *sensor_config_caps; */
	&CamPrimaryCfg_CCIR656_st,	/* *sensor_config_ccir656; */
	&CamPrimaryCfg_YCbCr_st,	/* *sensor_config_ycbcr; */
	&CamPrimaryCfg_I2C_st,	/* *sensor_config_i2c; */
	&CamPrimaryCfg_IOCR_st,	/* *sensor_config_iocr; */
	&CamPrimaryCfg_Jpeg_st,	/* *sensor_config_jpeg; */
	NULL,			/* *sensor_config_interleave_video; */
	&CamPrimaryCfg_InterLeaveStills_st,	/**sensor_config_interleave_stills; */
	&CamPrimaryCfg_PktMarkerInfo_st	/* *sensor_config_pkt_marker_info; */
};

/* I2C transaction result */
static HAL_CAM_Result_en_t sCamI2cStatus = HAL_CAM_SUCCESS;

static HAL_CAM_Result_en_t
SensorSetPreviewMode(CamImageSize_t image_resolution,
		     CamDataFmt_t image_format);
static HAL_CAM_Result_en_t Init_Mt9t111(CamSensorSelect_t sensor);
static int checkCameraID(CamSensorSelect_t sensor);
static UInt16 mt9t111_read(unsigned int sub_addr);
static HAL_CAM_Result_en_t mt9t111_write(unsigned int sub_addr, UInt16 data);
HAL_CAM_Result_en_t CAMDRV_SetFrameRate(CamRates_t fps,
					CamSensorSelect_t sensor);
HAL_CAM_Result_en_t CAMDRV_SetVideoCaptureMode(CamImageSize_t image_resolution,
					       CamDataFmt_t image_format,
					       CamSensorSelect_t sensor);
HAL_CAM_Result_en_t CAMDRV_SetZoom(CamZoom_t step,CamSensorSelect_t sensor);
/*****************************************************************************
*
* Function Name:   CAMDRV_GetIntfConfigTbl
*
* Description: Return Camera Sensor Interface Configuration
*
* Notes:
*
*****************************************************************************/
CamIntfConfig_st_t *CAMDRV_GetIntfConfig(CamSensorSelect_t nSensor)
{

/* ---------Default to no configuration Table */
	CamIntfConfig_st_t *config_tbl = NULL;

	switch (nSensor) {
	case CamSensorPrimary:	/* Primary Sensor Configuration */
	default:
		CamSensorCfg_st.sensor_config_caps = &CamPrimaryCfgCap_st;
		break;
	case CamSensorSecondary:	/* Secondary Sensor Configuration */
		CamSensorCfg_st.sensor_config_caps = NULL;
		break;
	}
	config_tbl = &CamSensorCfg_st;

#ifdef ALLOC_TN_BUFFER
	if (thumbnail_data == NULL) {
		/*tn_buffer = (u8 *)dma_alloc_coherent( NULL, TN_BUFFER_SIZE,
		   &physPtr, GFP_KERNEL); */
		thumbnail_data = (u8 *) kmalloc(TN_BUFFER_SIZE, GFP_KERNEL);
		if (thumbnail_data == NULL) {
			pr_info
			    ("Error in allocating %d bytes for MT9T111 sensor\n",
			     TN_BUFFER_SIZE);
			return NULL;
		}
		memset(thumbnail_data, 0, TN_BUFFER_SIZE);
	}
#endif
	return config_tbl;
}

/*****************************************************************************
*
* Function Name:   CAMDRV_GetInitPwrUpSeq
*
* Description: Return Camera Sensor Init Power Up sequence
*
* Notes:
*
*****************************************************************************/
CamSensorIntfCntrl_st_t *CAMDRV_GetIntfSeqSel(CamSensorSelect_t nSensor,
					      CamSensorSeqSel_t nSeqSel,
					      UInt32 *pLength)
{

/* ---------Default to no Sequence  */
	CamSensorIntfCntrl_st_t *power_seq = NULL;
	*pLength = 0;

	switch (nSeqSel) {
	case SensorInitPwrUp:	/* Camera Init Power Up (Unused) */
	case SensorPwrUp:
		if ((nSensor == CamSensorPrimary)
		    || (nSensor == CamSensorSecondary)) {
			pr_debug("SensorPwrUp Sequence\r\n");
			*pLength = sizeof(CamPowerOnSeq);
			power_seq = CamPowerOnSeq;
		}
		break;

	case SensorInitPwrDn:	/* Camera Init Power Down (Unused) */
	case SensorPwrDn:	/* Both off */
		if ((nSensor == CamSensorPrimary)
		    || (nSensor == CamSensorSecondary)) {
			pr_debug("SensorPwrDn Sequence\r\n");
			*pLength = sizeof(CamPowerOffSeq);
			power_seq = CamPowerOffSeq;
		}
		break;

	case SensorFlashEnable:	/* Flash Enable */
		break;

	case SensorFlashDisable:	/* Flash Disable */
		break;

	default:
		break;
	}
	return power_seq;

}

/***************************************************************************
* *
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
}				/* CAMDRV_Supp_Init() */

/****************************************************************************
*
* Function Name:   HAL_CAM_Result_en_t CAMDRV_Wakeup(CamSensorSelect_t sensor)
*
* Description: This function wakesup camera via I2C command.  Assumes camera
*              is enabled.
*
* Notes:
*
****************************************************************************/
HAL_CAM_Result_en_t CAMDRV_Wakeup(CamSensorSelect_t sensor)
{
	HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;

	result = Init_Mt9t111(sensor);
	pr_debug("Init_Mt9t111 result =%d\r\n", result);
	return result;
}

UInt16 CAMDRV_GetDeviceID(CamSensorSelect_t sensor)
{
	return mt9t111_read(0x00);
}

static int checkCameraID(CamSensorSelect_t sensor)
{
	UInt16 devId = CAMDRV_GetDeviceID(sensor);

	if (devId == MT9T111_ID) {
		pr_debug("Camera identified as MT9T111\r\n");
		return 0;
	} else {
		pr_debug("Camera Id wrong. Expected 0x%x but got 0x%x\r\n",
			 MT9T111_ID, devId);
		return -1;
	}
}

static HAL_CAM_Result_en_t mt9t111_write(unsigned int sub_addr, UInt16 data)
{
	HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
	sCamI2cStatus = HAL_CAM_SUCCESS;
	UInt8 bytes[2];
	bytes[0] = (data & 0xFF00) >> 8;
	bytes[1] = data & 0xFF;

	result |= CAM_WriteI2c(sub_addr, 2, bytes);
	if (result != HAL_CAM_SUCCESS) {
		sCamI2cStatus = result;
		pr_info("mt9t111_write(): ERROR: at addr:0x%x with value: 0x%x\n", sub_addr, data);
	}
	return result;
}

static UInt16 mt9t111_read(unsigned int sub_addr)
{
	HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
	sCamI2cStatus = HAL_CAM_SUCCESS;
	UInt16 data;
	UInt16 temp;

	result |= CAM_ReadI2c(sub_addr, 2, (UInt8 *) &data);
	if (result != HAL_CAM_SUCCESS) {
		sCamI2cStatus = result;
		pr_info("mt9t111_read(): ERROR: %d\r\n", result);
	}

	temp = data;
	data = ((temp & 0xFF) << 8) | ((temp & 0xFF00) >> 8);

	return data;
}

static HAL_CAM_Result_en_t
SensorSetPreviewMode(CamImageSize_t image_resolution, CamDataFmt_t image_format)
{
	UInt32 x = 0, y = 0;
	UInt32 format = 0;
	HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;

	switch (image_resolution) {
	
	case CamImageSize_R_QCIF:
		x = 144;
		y = 176;
		break;
	
	case CamImageSize_R_QVGA:
		x = 240;
		y = 320;
		break;
		
	case CamImageSize_SQCIF:
		x = 128;
		y = 96;
		break;

	case CamImageSize_QQVGA:
		x = 160;
		y = 120;
		break;

	case CamImageSize_QCIF:
		x = 176;
		y = 144;
		break;

	case CamImageSize_QVGA:
		x = 320;
		y = 240;
		break;

	case CamImageSize_CIF:
		x = 352;
		y = 288;
		break;

	case CamImageSize_VGA:
		x = 640;
		y = 480;
		break;

	case CamImageSize_SVGA:
		x = 800;
		y = 600;
		break;

	default:
		x = 320;
		y = 240;
		break;
	}

	/* Choose Format for Viewfinder mode  */
	/* Set the format for the Viewfinder mode */
	switch (image_format) {

	case CamDataFmtYCbCr:
		format = 1;
		break;

	case CamDataFmtRGB565:
		format = 4;
		break;

	default:
		format = 1;
		break;
	}

	/* Set preview resolution */
	/* [Preview XxY] */
	mt9t111_write(0x098E, 0x6800);	/* MCU_ADDRESS [PRI_A_IMAGE_WIDTH] */
	mt9t111_write(0x0990, x);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0x6802);	/* MCU_ADDRESS [PRI_A_IMAGE_HEIGHT] */
	mt9t111_write(0x0990, y);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0x8400);	/* MCU_ADDRESS [SEQ_CMD] */
	mt9t111_write(0x0990, 0x0006);	/* MCU_DATA_0 */

	/* [Format Selection] */
	mt9t111_write(0x098E, 0x6807);	/* MCU_ADDRESS [PRI_A_OUTPUT_FORMAT] */
	mt9t111_write(0x0990, format);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0x6809);	/* MCU_ADDRESS[PRI_A_OUTPUT_FORMAT_ORDER] */
	mt9t111_write(0x0990, 0x0000);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0xE88E);	/* MCU_ADDRESS[PRI_A_CONFIG_JPEG_JP_MODE] */
	mt9t111_write(0x0990, 0x0000);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0x8400);	/* MCU_ADDRESS [SEQ_CMD] */
	mt9t111_write(0x0990, 0x0006);	/* MCU_DATA_0 */

	if (image_format == CamDataFmtYCbCr) {
		UInt32 output_order = 2;	/* Switch low, high bytes. Y and C. */

		/* [Set Output Format Order] */
		/*MCU_ADDRESS[PRI_A_CONFIG_JPEG_JP_MODE] */
		mt9t111_write(0x098E, 0x6809);
		mt9t111_write(0x0990, output_order);	/* MCU_DATA_0 */
		mt9t111_write(0x098E, 0x8400);	/* MCU_ADDRESS [SEQ_CMD] */
		mt9t111_write(0x0990, 0x0006);	/* MCU_DATA_0 */

		pr_debug
		    ("SensorSetPreviewMode(): Output Format Order = 0x%x\r\n",
		     output_order);
	}

	pr_debug("SensorSetPreviewMode(): Resolution:0x%x, Format:0x%x r\n",
		 image_resolution, image_format);

	if (sCamI2cStatus != HAL_CAM_SUCCESS) {
		pr_debug
		    ("SensorSetPreviewMode(): Error[%d] sending preview mode  r\n",
		     sCamI2cStatus);
		result = sCamI2cStatus;
	}
	/*[Enable stream] */
	mt9t111_write(0x001A, 0x0218);

	return result;

}

/** The CAMDRV_GetResolutionAvailable returns the pointer to the sensor
	output size of the image width and height requested
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
HAL_CAM_Result_en_t CAMDRV_GetResolutionAvailable(UInt32 width,
						  UInt32 height,
						  CamCaptureMode_t mode,
						  CamSensorSelect_t sensor,
						  HAL_CAM_ResolutionSize_st_t *
						  sensor_size)
{
	HAL_CAM_Result_en_t result = HAL_CAM_ERROR_ACTION_NOT_SUPPORTED;
	UInt16 index, res_size;
	CamResolutionConfigure_t *res_ptr;
	CamResolutionConfigure_t *best_match_ptr;

/* Resolution default settings */
	sensor_size->size = CamImageSize_INVALID;
	sensor_size->resX = 0;
	sensor_size->resY = 0;

/* Init Resolution table pointer */
	if (sensor == CamSensorPrimary) {
		res_size =
		    sizeof(sSensorResInfo_MT9T111_st) /
		    sizeof(CamResolutionConfigure_t);
		res_ptr = &sSensorResInfo_MT9T111_st[0];
	} else {
		pr_debug("CAMDRV_GetResolution(): ERROR:  Sensor Failed \r\n");
		result = HAL_CAM_ERROR_ACTION_NOT_SUPPORTED;
		return result;
	}

/* Search Resolution table for best match for requested width & height
	based on capture mode */
	best_match_ptr = res_ptr;
	for (index = 0; index < res_size; index++) {
		if ((mode == CamCaptureVideo)
		    || (mode == CamCaptureVideonViewFinder)) {
			if (res_ptr->previewIndex != -1) {
				best_match_ptr = res_ptr;
			}

			if ((width <= res_ptr->resX)
			    && (height <= res_ptr->resY)
			    && (res_ptr->previewIndex != -1)) {
				sensor_size->size =
				    (CamImageSize_t) res_ptr->previewIndex;
				sensor_size->resX = res_ptr->resX;
				sensor_size->resY = res_ptr->resY;
				result = HAL_CAM_SUCCESS;
				pr_debug
				    ("CAMDRV_GetResolutionAvailable(): Resolution: \
						size index=%d width=%d height=%d \r\n",
				     res_ptr->previewIndex, res_ptr->resX, res_ptr->resY);
				return result;
			}
		} else if ((mode == CamCaptureStill)
			   || (mode == CamCaptureStillnThumb)) {
			if (res_ptr->captureIndex != -1) {
				best_match_ptr = res_ptr;
			}

			if ((width <= res_ptr->resX)
			    && (height <= res_ptr->resY)
			    && (res_ptr->captureIndex != -1)) {
				sensor_size->size =
				    (CamImageSize_t) res_ptr->previewIndex;
				sensor_size->resX = res_ptr->resX;
				sensor_size->resY = res_ptr->resY;
				result = HAL_CAM_SUCCESS;
				pr_debug
				    ("CAMDRV_GetResolutionAvailable(): Resolution: \
					 size index=%d width=%d height=%d \r\n",
				     res_ptr->captureIndex, res_ptr->resX, res_ptr->resY);
				return result;
			}
		} else {
			pr_debug
			    ("CAMDRV_GetResolutionAvailable(): ERROR: Mode Failed\r\n");
			result = HAL_CAM_ERROR_ACTION_NOT_SUPPORTED;
			return result;
		}
		/* Increment table pointer */
		res_ptr++;
	}
/* Resolution best match settings */
	pr_debug("CAMDRV_GetResolutionAvailable(): Best Match used \r\n");
	sensor_size->size = (CamImageSize_t) best_match_ptr->previewIndex;
	sensor_size->resX = best_match_ptr->resX;
	sensor_size->resY = best_match_ptr->resY;
	return result;
}

/** The CAMDRV_GetResolution returns the sensor output size of the
	image resolution requested
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
HAL_CAM_Result_en_t CAMDRV_GetResolution(CamImageSize_t size,
					 CamCaptureMode_t mode,
					 CamSensorSelect_t sensor,
					 HAL_CAM_ResolutionSize_st_t *
					 sensor_size)
{
	HAL_CAM_Result_en_t result = HAL_CAM_ERROR_ACTION_NOT_SUPPORTED;
	UInt16 index, res_size;
	CamResolutionConfigure_t *res_ptr;

	pr_debug("CAMDRV_GetResolution(): size=0x%x, mode=%d, sensor=%d\r\n",
		 size, mode, sensor);

/* Resolution default settings */
	sensor_size->size = CamImageSize_INVALID;
	sensor_size->resX = 0;
	sensor_size->resY = 0;

/* Init Resolution table pointer */
	if (sensor == CamSensorPrimary) {
		res_size =
		    sizeof(sSensorResInfo_MT9T111_st) /
		    sizeof(CamResolutionConfigure_t);
		res_ptr = &sSensorResInfo_MT9T111_st[0];
	} else {
		pr_debug("CAMDRV_GetResolution(): ERROR:  Sensor Failed \r\n");
		result = HAL_CAM_ERROR_ACTION_NOT_SUPPORTED;
		return result;
	}

/* Search Resolution table for requested resolution based on capture mode
	(or largest resolution available) */
	for (index = 0; index < res_size; index++) {
		pr_debug
		    ("CAMDRV_GetResolution(): Resolution: size index=%d width=%d \
			height=%d \r\n",
		     index, res_ptr->resX, res_ptr->resY);
		if ((mode == CamCaptureVideo)
		    || (mode == CamCaptureVideonViewFinder)) {
			if (size == (CamImageSize_t) res_ptr->previewIndex) {
				sensor_size->size =
				    (CamImageSize_t) res_ptr->previewIndex;
				sensor_size->resX = res_ptr->resX;
				sensor_size->resY = res_ptr->resY;
				result = HAL_CAM_SUCCESS;
				return result;
			} else if (res_ptr->previewIndex != -1) {
				sensor_size->size =
				    (CamImageSize_t) res_ptr->previewIndex;
				sensor_size->resX = res_ptr->resX;
				sensor_size->resY = res_ptr->resY;
			}
		} else if ((mode == CamCaptureStill)
			   || (mode == CamCaptureStillnThumb)) {
			if (size == (CamImageSize_t) res_ptr->captureIndex) {
				sensor_size->size =
				    (CamImageSize_t) res_ptr->captureIndex;
				sensor_size->resX = res_ptr->resX;
				sensor_size->resY = res_ptr->resY;
				result = HAL_CAM_SUCCESS;
				return result;
			} else if (res_ptr->captureIndex != -1) {
				sensor_size->size =
				    (CamImageSize_t) res_ptr->captureIndex;
				sensor_size->resX = res_ptr->resX;
				sensor_size->resY = res_ptr->resY;
			}
		} else {
			pr_debug
			    ("CAMDRV_GetResolution(): ERROR:  Mode Failed \r\n");
			result = HAL_CAM_ERROR_ACTION_NOT_SUPPORTED;
			return result;
		}
		/* Increment table pointer */
		res_ptr++;
	}
	pr_debug("CAMDRV_GetResolution(): ERROR:  Resolution Failed \r\n");
	return result;
}

/** @} */

/****************************************************************************
*
* Function Name:   HAL_CAM_Result_en_t CAMDRV_SetVideoCaptureMode
				(CamImageSize_t image_resolution, CamDataFmt_t image_format)
*
* Description: This function configures Video Capture
				(Same as ViewFinder Mode)
*
* Notes:
*
****************************************************************************/
HAL_CAM_Result_en_t CAMDRV_SetVideoCaptureMode(CamImageSize_t image_resolution,
					       CamDataFmt_t image_format,
					       CamSensorSelect_t sensor)
{
	HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;

	/* --------Set up Camera Isp for Output Resolution & Format */
	result = SensorSetPreviewMode(image_resolution, image_format);
	return result;
}

/****************************************************************************
*
* Function Name:   HAL_CAM_Result_en_t CAMDRV_SetFrameRate(CamRates_t fps)
*
* Description: This function sets the frame rate of the Camera Sensor
*
* Notes:    15 or 30 fps are supported.
*
****************************************************************************/
HAL_CAM_Result_en_t CAMDRV_SetFrameRate(CamRates_t fps,
					CamSensorSelect_t sensor)
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
				//PLL
				mt9t111_write(0x0010, 0x0C78 ); //PLL Dividers = 3192
				mt9t111_write(0x0012, 0x0070 ); //PLL P Dividers = 112
				mt9t111_write(0x002A, 0x7788 ); //PLL P Dividers 4-5-6 = 30600
				//fps
				mt9t111_write(0x98E, 0x6815 	); //Max FD Zone 50 Hz
				mt9t111_write(0x990, 0x0012 	); //	   = 3
				mt9t111_write(0x98E, 0x6817 	); //Max FD Zone 60 Hz
				mt9t111_write(0x990, 0x0012 	); //	   = 3
				mt9t111_write(0x98E, 0x682D 	); //AE Target FD Zone
				mt9t111_write(0x990, 0x0012 	); //	   = 3
				mt9t111_write(0x098E, 0x8400	); // MCU_ADDRESS
				mt9t111_write(0x0990, 0x0006	); // MCU_DATA_0
				break;
				
				case CamRate_10:             
				//PLL
				mt9t111_write(0x0010, 0x0C78 ); //PLL Dividers = 3192
				mt9t111_write(0x0012, 0x0070 ); //PLL P Dividers = 112
				mt9t111_write(0x002A, 0x7788 ); //PLL P Dividers 4-5-6 = 30600
				//fps
				mt9t111_write(0x98E, 0x6815 ); //Max FD Zone 50 Hz
				mt9t111_write(0x990, 0x000a ); //	   = 3
				mt9t111_write(0x98E, 0x6817 ); //Max FD Zone 60 Hz
				mt9t111_write(0x990, 0x000a ); //	   = 3
				mt9t111_write(0x98E, 0x682D ); //AE Target FD Zone
				mt9t111_write(0x990, 0x000a); //	  = 3
				mt9t111_write(0x098E, 0x8400	); // MCU_ADDRESS
				mt9t111_write(0x0990, 0x0006	); // MCU_DATA_0

				break;
				
				case CamRate_15:            // 15 fps
				//PLL
				mt9t111_write(0x0010, 0x0C78 ); //PLL Dividers = 3192
				mt9t111_write(0x0012, 0x0070 ); //PLL P Dividers = 112
				mt9t111_write(0x002A, 0x7788 ); //PLL P Dividers 4-5-6 = 30600
				//fps
				mt9t111_write(0x98E, 0x6815 ); //Max FD Zone 50 Hz
				mt9t111_write(0x990, 0x0007 ); //	   = 3
				mt9t111_write(0x98E, 0x6817 ); //Max FD Zone 60 Hz
				mt9t111_write(0x990, 0x0007 ); //	   = 3
				mt9t111_write(0x98E, 0x682D ); //AE Target FD Zone
				mt9t111_write(0x990, 0x0007); //	  = 3
				mt9t111_write(0x098E, 0x8400	); // MCU_ADDRESS
				mt9t111_write(0x0990, 0x0006	); // MCU_DATA_0


				break;
				
				case CamRate_20:             
				//PLL
				mt9t111_write(0x0010, 0x0C78 ); //PLL Dividers = 3192
				mt9t111_write(0x0012, 0x0070 ); //PLL P Dividers = 112
				mt9t111_write(0x002A, 0x7788 ); //PLL P Dividers 4-5-6 = 30600
				//fps
				mt9t111_write(0x98E, 0x6815 ); //Max FD Zone 50 Hz
				mt9t111_write(0x990, 0x0005 ); //	   = 3
				mt9t111_write(0x98E, 0x6817 ); //Max FD Zone 60 Hz
				mt9t111_write(0x990, 0x0005 ); //	   = 3
				mt9t111_write(0x98E, 0x682D ); //AE Target FD Zone
				mt9t111_write(0x990, 0x0005); //	  = 3
				mt9t111_write(0x098E, 0x8400	); // MCU_ADDRESS
				mt9t111_write(0x0990, 0x0006	); // MCU_DATA_0


				break;
				
				case CamRate_25:           // 25 fps
				//PLL
				mt9t111_write(0x0010, 0x0C78 ); //PLL Dividers = 3192
				mt9t111_write(0x0012, 0x0070 ); //PLL P Dividers = 112
				mt9t111_write(0x002A, 0x7788 ); //PLL P Dividers 4-5-6 = 30600
				//fps
				mt9t111_write(0x98E, 0x6815 ); //Max FD Zone 50 Hz
				mt9t111_write(0x990, 0x0004 ); //	   = 3
				mt9t111_write(0x98E, 0x6817 ); //Max FD Zone 60 Hz
				mt9t111_write(0x990, 0x0004 ); //	   = 3
				mt9t111_write(0x98E, 0x682D ); //AE Target FD Zone
				mt9t111_write(0x990, 0x0004); //	  = 3
				mt9t111_write(0x098E, 0x8400	); // MCU_ADDRESS
				mt9t111_write(0x0990, 0x0006	); // MCU_DATA_0


				break;
				
				case CamRate_30:           // 30 fps
				 //PLL
				mt9t111_write(0x0010, 0x0C78 ); //PLL Dividers = 3192
				mt9t111_write(0x0012, 0x0070 ); //PLL P Dividers = 112
				mt9t111_write(0x002A, 0x7788 ); //PLL P Dividers 4-5-6 = 30600
				 //fps
				mt9t111_write(0x98E, 0x6815 ); //Max FD Zone 50 Hz
				mt9t111_write(0x990, 0x0003 ); //	   = 3
				mt9t111_write(0x98E, 0x6817 ); //Max FD Zone 60 Hz
				mt9t111_write(0x990, 0x0003 ); //	   = 3
				mt9t111_write(0x98E, 0x682D ); //AE Target FD Zone
				mt9t111_write(0x990, 0x0003 ); //	   = 3
				mt9t111_write(0x098E, 0x8400	); // MCU_ADDRESS
				mt9t111_write(0x0990, 0x0006	); // MCU_DATA_0


				break;
				
				default:                                        // Maximum Clock Rate = 26Mhz
				 //PLL
				mt9t111_write(0x0010, 0x0C78 ); //PLL Dividers = 3192
				mt9t111_write(0x0012, 0x0070 ); //PLL P Dividers = 112
				mt9t111_write(0x002A, 0x7788 ); //PLL P Dividers 4-5-6 = 30600
				 //fps
				mt9t111_write(0x98E, 0x6815 ); //Max FD Zone 50 Hz
				mt9t111_write(0x990, 0x0003 ); //	   = 3
				mt9t111_write(0x98E, 0x6817 ); //Max FD Zone 60 Hz
				mt9t111_write(0x990, 0x0003 ); //	   = 3
				mt9t111_write(0x98E, 0x682D ); //AE Target FD Zone
				mt9t111_write(0x990, 0x0003 ); //	   = 3
				mt9t111_write(0x098E, 0x8400	); // MCU_ADDRESS
				mt9t111_write(0x0990, 0x0006	); // MCU_DATA_0
				result = HAL_CAM_ERROR_ACTION_NOT_SUPPORTED;
				 pr_debug("CAMDRV_SetFrameRate(): Error HAL_CAM_ERROR_ACTION_NOT_SUPPORTED \r\n");
				break;
			}
		}       // else (if (ImageSettingsConfig_st.sensor_framerate->cur_setting == CamRate_Auto))
	}       // else (if (fps <= CamRate_Auto))

	 if (sCamI2cStatus != HAL_CAM_SUCCESS) {
		 pr_debug("CAMDRV_SetFrameRate(): Error[%d] \r\n",
			  sCamI2cStatus);
		 result = sCamI2cStatus;
	 }

    return result;

}

/****************************************************************************
/
/ Function Name:   HAL_CAM_Result_en_t
					CAMDRV_EnableVideoCapture(CamSensorSelect_t sensor)
/
/ Description: This function starts camera video capture mode
/
/ Notes:
/
****************************************************************************/
HAL_CAM_Result_en_t CAMDRV_EnableVideoCapture(CamSensorSelect_t sensor)
{
	/*[Enable stream] */
	mt9t111_write(0x001A, 0x0218);
	msleep(300);
	return sCamI2cStatus;
}

/****************************************************************************
/
/ Function Name:   void CAMDRV_SetCamSleep(CamSensorSelect_t sensor )
/
/ Description: This function puts ISP in sleep mode & shuts down power.
/
/ Notes:
/
****************************************************************************/
HAL_CAM_Result_en_t CAMDRV_SetCamSleep(CamSensorSelect_t sensor)
{
	HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;

	pr_debug("CAMDRV_SetCamSleep(): \r\n");

	/* To be implemented. */
	return result;
}

static void _touchJpegQtable(int qtable, int qscale)
{
	int i;

	/* Quantization table Luma */
	for (i = 0; i < 64; ++i) {
		unsigned int q = (JPEG_StdQuantTblY_ZZ[i] * qscale + 16) >> 5;
		q = (q < 1) ? 1 : (q > 255) ? 255 : q;
		quant_JPEG_Hdr[0][i] = (unsigned char)q;
	}

	/* Quantization table Chroma */
	for (i = 0; i < 64; ++i) {
		unsigned int q = (JPEG_StdQuantTblC_ZZ[i] * qscale + 16) >> 5;
		q = (q < 1) ? 1 : (q > 255) ? 255 : q;
		quant_JPEG_Hdr[1][i] = (unsigned char)q;
	}
}

void CAMDRV_StoreBaseAddress(void *virt_ptr)
{
	jpeg_header_data = virt_ptr;
}

UInt32 CAMDRV_GetJpegSize(CamSensorSelect_t sensor, void *data)
{
	/* In this function, prepare the jpeg, thumbnail data and also
	   compute the JPEG data size. */
	struct DataFormat JpegDataFormat;
	int tn_counter = 0;
	int jpeg_size = 0;
	int tn_size = 0;
	int i = 0;
	int Counter = 0;
	bool marker_found = false;
	int frame_length = 0;
	unsigned char qsel = 0;
	u16 *buffer = (u16 *) data;
	/* pointer to loop thro DMA buffer to find if data is valid and to find the
	   length of the thumbnail + JPEG.
	   Also, to extract thumbnail and JPEG.
	 */
	u8 *tn_ptr = thumbnail_data;	/* pointer to loop for thumbnail data. */
	u8 *write_ptr = data;	/* pointer which will have raw JPEG data. */
	jpeg_raw_data = data;
	/* pointer to store the JPEG raw data.
	   This will be copied to user buffer in JPEG ioctl.
	 */

	/* scan the entire loop to search for sosi and eosi. */
	while (tn_counter <= (1200 * 1360)) {
		u8 *ptr;
		if (*buffer == SOSI) {
			buffer++;
			ptr = (u8 *) buffer;
			for (i = 0; i < 3; i++) {
				frame_length |= *ptr++;
				frame_length <<= 8;
			}
			frame_length |= *ptr++;
			pr_info
			    ("************* FRAME LENGTH 0x%x ***************\n ",
			     frame_length);
			marker_found = true;
			break;
		} else {
			buffer++;
			tn_counter += 2;
		}
	}
	if (!marker_found) {
		pr_info("******** No valid data *******");
		return 0;
	}
	/* extract the thumbnail */
	tn_counter = 0;
	buffer = (u16 *) data;

	while (tn_counter < tn_height) {
		if (*buffer == SOEI) {
			buffer++;	/* remove the start code. */
			for (i = 0; i < tn_width; i++) {
				tn_ptr[(2 * i) + 1] = (buffer[i] & 0x00FF);
				tn_ptr[2 * i] = (buffer[i] & 0xFF00) >> 8;
			}
			tn_counter++;
			/*printk(KERN_INFO"0x%x 0x%x 0x%x 0x%x\n\n",op_buffer[0],
			   op_buffer[1],op_buffer[2],op_buffer[3]); */
			tn_ptr += tn_width * 2;
			buffer += tn_width;
			tn_size += tn_width * 2;
			buffer++;
		} else {
			buffer++;
		}
		if ((*buffer == SOSI) || (tn_size >= tn_width * tn_height * 2))
			break;

	}
	pr_info("Number of extracted thumbnail lines is %d \n", tn_counter);
	pr_info("Thumbnail size is %d\n", tn_size);

	if (tn_counter != tn_height) {
		pr_info("Thumbnail Shorter than expected \n");
		return 0;
	}

	buffer = (u16 *) data;	/*To loop thro JPEG data to extract the jpeg data */
	tn_counter = 0;

	while (tn_counter < frame_length) {
		u8 *copy_ptr;
		if (*buffer == SOEI) {
			buffer++;	/* to skip soei */
			buffer += tn_width;	/* to skip the tn data */
			buffer++;	/* to skip the eoei */
			tn_counter += 4 + (tn_width * 2);
		} else {
			copy_ptr = (u8 *) buffer;
			*write_ptr++ = *copy_ptr++;
			*write_ptr++ = *copy_ptr++;
			jpeg_size += 2;
			buffer++;
			tn_counter += 2;
		}
	}
	/* Now we have the JPEG raw data in contigous memory in write_ptr. */

	*write_ptr++ = 0xFF;
	*write_ptr = 0xD9;

	jpeg_size += 2;
	JpegDataFormat.spoof_width = jpeg_width;
	JpegDataFormat.spoof_heigth = jpeg_height;
	JpegDataFormat.IS_TN_ENABLED = 0;
	JpegDataFormat.CONFIG_JPEG_OB_TX_CONTROL_VAR = 1;
	JpegDataFormat.CONFIG_JPEG_OB_SPOOF_CONTROL_VAR = 0;
	JpegDataFormat.JpegFormat = 2;
	JpegDataFormat.JpegXsize = jpeg_width;	/*2048; */
	JpegDataFormat.JpegYsize = jpeg_height;	/*1536; */
	JpegDataFormat.thumbnail_height = tn_width;
	JpegDataFormat.thumbnail_width = tn_height;
	JpegDataFormat.JpegStatus = 0x80;
	JpegDataFormat.JpegSize = jpeg_size;	/* size of raw JPEG data. */
	qsel = (JpegDataFormat.JpegStatus & 0x600) >> 9;

	/*Store Jpeg Header */
	jpeg_header_data[0] = 0xFF;
	jpeg_header_data[1] = 0xD8;
	Counter = 2;

	for (i = 0; i < JFIFHeaderSize; i++) {
		jpeg_header_data[Counter] = JFIFHeader[i];
		Counter++;
	}

	/*Store Qtable Header */
	for (i = 0; i < QTableHeaderSize; i++) {
		jpeg_header_data[Counter] = QTableHeader[i];
		Counter++;
	}
	/*Store Luma QTable */
	jpeg_header_data[Counter] = 0x00;
	Counter++;

	/*Check which  which Qscale value to use */
	if (qsel == 0) {	/*Use Qscale1 */
		_touchJpegQtable(qsel, 0x08);
	} else {		/*Use Qscale2 */
		_touchJpegQtable(qsel, 0x0C);
	}

	/*Store Q Tables */
	for (i = 0; i < 64; i++) {
		jpeg_header_data[Counter] = quant_JPEG_Hdr[0][i];
		Counter++;
	}

	/*If color then store the Chroma table */
	if (JpegDataFormat.JpegFormat != 0) {
		jpeg_header_data[Counter] = 0x01;
		Counter++;
		for (i = 0; i < 64; i++) {
			jpeg_header_data[Counter] = quant_JPEG_Hdr[1][i];
			Counter++;
		}
	}
	/*STORE HUFFMAN TABLES */
	for (i = 0; i < LumaHuffTableSizeDC; i++) {
		jpeg_header_data[Counter] = LumaHuffTableDC[i];
		Counter++;
	}
	for (i = 0; i < LumaHuffTableSizeAC; i++) {
		jpeg_header_data[Counter] = LumaHuffTableAC[i];
		Counter++;
	}

	/*if color store Chroma */
	if (JpegDataFormat.JpegFormat != 0) {
		for (i = 0; i < ChromaHuffTableSizeDC; i++) {
			jpeg_header_data[Counter] = ChromaHuffTableDC[i];
			Counter++;
		}
		for (i = 0; i < ChromaHuffTableSizeAC; i++) {
			jpeg_header_data[Counter] = ChromaHuffTableAC[i];
			Counter++;
		}
	}

	/*end If */
	/*Store Restart Interval */
	for (i = 0; i < RestartIntervalSize; i++) {
		jpeg_header_data[Counter] = RestartInterval[i];
		Counter++;
	}

	for (i = 0; i < StartOfFrameSize; i++) {
		jpeg_header_data[Counter] = StartOfFrame[i];
		Counter++;
	}

	/*Update Snapshot resolution */
	if (JpegDataFormat.JpegFormat == 0) {
		jpeg_header_data[Counter - 8] = (JpegDataFormat.JpegYsize >> 8);
		jpeg_header_data[Counter - 7] =
		    (JpegDataFormat.JpegYsize & 0xFF);
		jpeg_header_data[Counter - 6] = (JpegDataFormat.JpegXsize >> 8);
		jpeg_header_data[Counter - 5] =
		    (JpegDataFormat.JpegXsize & 0xFF);
	} else {		/*Color SOF */
		jpeg_header_data[Counter - 14] =
		    (JpegDataFormat.JpegYsize >> 8);
		jpeg_header_data[Counter - 13] =
		    (JpegDataFormat.JpegYsize & 0xFF);
		jpeg_header_data[Counter - 12] =
		    (JpegDataFormat.JpegXsize >> 8);
		jpeg_header_data[Counter - 11] =
		    (JpegDataFormat.JpegXsize & 0xFF);
	}

	/*Store Start of Scan */
	if (JpegDataFormat.JpegFormat == 0) {
		for (i = 0; i < StartOfScanSizeM; i++) {
			jpeg_header_data[Counter] = StartOfScanM[i];
			Counter++;
		}
	} else {		/*Color */
		for (i = 0; i < StartOfScanSize; i++) {
			jpeg_header_data[Counter] = StartOfScan[i];
			Counter++;
		}
	}
	pr_info("Counter now is %d", Counter);

	/* Now we have JPEG header data in 1st 1K of DMA memory. The JPEG raw data
	   is afterwards. We have to move the data up to form a contigous memory.
	 */
	memcpy((jpeg_header_data + Counter), jpeg_raw_data,
	       JpegDataFormat.JpegSize);

	/*memmove(op_buffer, (op_buffer + 1024 - Counter),
	   JpegDataFormat.JpegSize); */
	Counter += JpegDataFormat.JpegSize;
	pr_info("Size of JPEG data is %d\n", Counter);
	return Counter;
}

enum {
	STATE_INIT,
	STATE_JPEG,
	STATE_THUMB,
	STATE_STATUS,
	STATE_EXIT
};

UInt16 *CAMDRV_GetJpeg(short *buf)
{
	/*return (tn_buffer + (tn_width * tn_height * 2)); */
	return (UInt16 *) jpeg_header_data;
}

UInt8 *CAMDRV_GetThumbnail(void *buf, UInt32 offset)
{
	/*return (UInt8 *)tn_buffer; give the starting address of
	   1.5MB which contains thumbnail */
	return (UInt8 *) thumbnail_data;
}

HAL_CAM_Result_en_t CAMDRV_DisableCapture(CamSensorSelect_t sensor)
{

	/*[Disable stream] */
	mt9t111_write(0x001A, 0x0018);

	/*[Preview on] */
	mt9t111_write(0x098E, 0xEC05);	/* MCU_ADDRESS [PRI_B_NUM_OF_FRAMES_RUN] */
	mt9t111_write(0x0990, 0x0005);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0x8400);	/* MCU_ADDRESS [SEQ_CMD] */
	mt9t111_write(0x0990, 0x0001);	/* MCU_DATA_0 */

	pr_debug("CAMDRV_DisableCapture(): \r\n");
	return sCamI2cStatus;
}

/****************************************************************************
/
/ Function Name:   HAL_CAM_Result_en_t CAMDRV_DisablePreview(void)
/
/ Description: This function halts MT9M111 camera video
/
/ Notes:
/
****************************************************************************/
HAL_CAM_Result_en_t CAMDRV_DisablePreview(CamSensorSelect_t sensor)
{
	/* [Disable stream] */
	mt9t111_write(0x001A, 0x0018);

	pr_debug("CAMDRV_DisablePreview(): \r\n");
	return sCamI2cStatus;
}

/****************************************************************************
/
/ Function Name:   HAL_CAM_Result_en_t CAMDRV_CfgStillnThumbCapture(
					CamImageSize_t image_resolution,
					CamDataFmt_t format,
					CamSensorSelect_t sensor)
/
/ Description: This function configures Stills Capture
/
/ Notes:
/
****************************************************************************/
HAL_CAM_Result_en_t CAMDRV_CfgStillnThumbCapture(CamImageSize_t
						 stills_resolution,
						 CamDataFmt_t stills_format,
						 CamImageSize_t
						 thumb_resolution,
						 CamDataFmt_t thumb_format,
						 CamSensorSelect_t sensor)
{
	UInt32 x = 0, y = 0;
	UInt32 tx = 0, ty = 0;
	HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;

	pr_debug("***************************** \
		CAMDRV_CfgStillnThumbCapture():STARTS ************************* \r\n");

	switch (stills_resolution) {
	case CamImageSize_QCIF:

		x = 176;
		y = 144;
		break;

	case CamImageSize_QVGA:
		x = 320;
		y = 240;
		break;

	case CamImageSize_CIF:
		x = 352;
		y = 288;
		break;

	case CamImageSize_VGA:
		x = 640;
		y = 480;
		break;

	case CamImageSize_SVGA:
		x = 800;
		y = 600;
		break;

	case CamImageSize_XGA:
		x = 1024;
		y = 768;
		break;

	case CamImageSize_SXGA:
		x = 1280;
		y = 1024;
		break;

	case CamImageSize_UXGA:
		x = 1600;
		y = 1200;
		break;

	case CamImageSize_QXGA:
		x = 2048;
		y = 1536;
		break;

	default:
		x = 640;
		y = 480;
		break;
	}

	switch (thumb_resolution) {

	case CamImageSize_QCIF:

		tx = 176;
		ty = 144;
		break;

	case CamImageSize_QVGA:
		tx = 320;
		ty = 240;
		break;

	case CamImageSize_CIF:
		tx = 352;
		ty = 288;
		break;

	case CamImageSize_VGA:
		tx = 640;
		ty = 480;
		break;

	default:
		tx = 176;
		ty = 144;
		break;
	}

	pr_debug("CAMDRV_CfgStillnThumbCapture():Operating in JPEG mode \r\n");

	tn_width = tx;
	tn_height = ty;
	jpeg_width = x;
	jpeg_height = y;

#if 1
	/***************** We assume we will not use JPEG mode at all.
					Only YCbCr is supported here.****************/
	/* enable for thumbnail */
	/* [Thumbnail tx x ty] */
	mt9t111_write(0x098E, 0x6C97);	/*MCU_ADDRESS[PRI_B_CONFIG_JPEG_TN_WIDTH] */
	mt9t111_write(0x0990, tx);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0x6C99);	/*MCU_ADDRESS[PRI_B_CONFIG_JPEG_TN_HEIGHT] */
	mt9t111_write(0x0990, ty);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0x8400);	/* MCU_ADDRESS [SEQ_CMD] */
	mt9t111_write(0x0990, 0x0005);	/* MCU_DATA_0 */

	/* [Enable Thumbnail] */
	mt9t111_write(0x098E, 0xEC8E);	/*MCU_ADDRESS[PRI_B_CONFIG_JPEG_JP_MODE] */
	mt9t111_write(0x0990, 0x0003);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0x8400);	/* MCU_ADDRESS [SEQ_CMD] */
	mt9t111_write(0x0990, 0x0005);	/* MCU_DATA_0 */

#if 0
	/* [Disable Thumbnail] */
	mt9t111_write(0x098E, 0xEC8E);	/*MCU_ADDRESS [PRI_B_CONFIG_JPEG_JP_MODE] */
	mt9t111_write(0x0990, 0x0001);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0x8400);	/* MCU_ADDRESS [SEQ_CMD] */
	mt9t111_write(0x0990, 0x0005);	/* MCU_DATA_0 */
#endif

#endif
	/* [Capture XxY] */
	mt9t111_write(0x098E, 0x6C00);	/* MCU_ADDRESS [PRI_B_IMAGE_WIDTH] */
	mt9t111_write(0x0990, x);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0x6C02);	/* MCU_ADDRESS [PRI_B_IMAGE_HEIGHT] */
	mt9t111_write(0x0990, y);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0x8400);	/* MCU_ADDRESS [SEQ_CMD] */
	mt9t111_write(0x0990, 0x0006);	/* MCU_DATA_0 */

	/*JPEG  SPOOF   full height ,only EOI */
	mt9t111_write(0x098E, 0xEC8E);	/*JPEG (B) */
	mt9t111_write(0x0990, 0x003);	/*      = 0     0x00 last  06182010 */
	mt9t111_write(0x098E, 0x6C07);	/* MCU_ADDRESS [PRI_B_OUTPUT_FORMAT] */
	mt9t111_write(0x0990, 0x0001);	/* MCU_DATA_0  JPEG422 */
	mt9t111_write(0x098E, 0x6C09);	/*MCU_ADDRESS[PRI_B_OUTPUT_FORMAT_ORDER] */
	mt9t111_write(0x0990, 0x0000);	/* MCU_DATA_0  JPEG422 */
	mt9t111_write(0x098E, 0xEC8E);	/*MCU_ADDRESS[PRI_B_CONFIG_JPEG_JP_MODE] */

	mt9t111_write(0x0990, 0x0003);	/* MCU_DATA_0  JPEG422 */
	/* need to know should we make it 1 or 3 ? vinay */

	mt9t111_write(0x098E, 0xEC8F);	/* MCU_ADDRESS [PRI_B_CONFIG_JPEG_FORMAT] */
	mt9t111_write(0x0990, 0x0002);	/* MCU_DATA_0  JPEG422 */
	/* MCU_ADDRESS[PRI_B_CONFIG_JPEG_OB_TX_CONTROL_VAR] */
	mt9t111_write(0x098E, 0x6CA0);
	mt9t111_write(0x0990, 0x082D);	/* MCU_DATA_0  spoof full height */
#if 1
	/* MCU_ADDRESS [PRI_B_CONFIG_JPEG_OB_SPOOF_CONTROL_VAR] */
	mt9t111_write(0x098E, 0xEC9F);
	mt9t111_write(0x0990, 0x0001);	/* MCU_DATA_0  ignore spoof full height */
#endif
	mt9t111_write(0x098E, 0x6C90);	/* MCU_ADDRESS [PRI_B_CONFIG_JPEG_CONFIG] */
	mt9t111_write(0x0990, 0x8434);	/* MCU_DATA_0  EOI only */
	/* MCU_ADDRESS [PRI_B_CONFIG_JPEG_OB_SPOOF_WIDTH_VAR] */
	mt9t111_write(0x098E, 0x6C9B);
	/* MCU_DATA_0  1000 SPOOF size */
	mt9t111_write(0x0990, CamPrimaryCfg_Jpeg_st.jpeg_packet_size_bytes);
	/* MCU_ADDRESS [PRI_B_CONFIG_JPEG_OB_SPOOF_HEIGHT_VAR] */
	mt9t111_write(0x098E, 0x6C9D);
	/* MCU_DATA_0  900    SPOOF size */
	mt9t111_write(0x0990, CamPrimaryCfg_Jpeg_st.jpeg_max_packets);
	mt9t111_write(0x098E, 0x8400);	/* MCU_ADDRESS [SEQ_CMD] */
	mt9t111_write(0x0990, 0x0006);	/* MCU_DATA_0 */

	/* Jpeg422 */
	/* [Capture on] */
	mt9t111_write(0x098E, 0xEC05);	/* MCU_ADDRESS [PRI_B_NUM_OF_FRAMES_RUN] */
	mt9t111_write(0x0990, 0x0000);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0x8400);	/* MCU_ADDRESS [SEQ_CMD] */
	mt9t111_write(0x0990, 0x0002);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0xEC05);	/* MCU_ADDRESS [PRI_B_NUM_OF_FRAMES_RUN] */
	mt9t111_write(0x0990, 0x0000);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0x8400);	/* MCU_ADDRESS [SEQ_CMD] */
	mt9t111_write(0x0990, 0x0002);	/* MCU_DATA_0 */

	/* [Enable stream] */
	mt9t111_write(0x001A, 0x0218);

	if (sCamI2cStatus != HAL_CAM_SUCCESS) {
		pr_debug
		    ("CAMDRV_CfgStillnThumbCapture():Error sending capture mode \r\n");
		result = sCamI2cStatus;
	}

	pr_debug("CAMDRV_CfgStillnThumbCapture(): stills_resolution = 0x%x, \
		 stills_format=0x%x \r\n", stills_resolution, stills_format);
	pr_debug("CAMDRV_CfgStillnThumbCapture(): thumb_resolution = 0x%x, \
		 thumb_format=0x%x \r\n", thumb_resolution, thumb_format);

	/* msleep(5000); */

	return result;
}

/****************************************************************************
/ Function Name:   HAL_CAM_Result_en_t CAMDRV_SetSceneMode(
/					CamSceneMode_t scene_mode)
/
/ Description: This function will set the scene mode of camera
/ Notes:
****************************************************************************/

HAL_CAM_Result_en_t CAMDRV_SetSceneMode(CamSceneMode_t scene_mode,
					CamSensorSelect_t sensor)
{
	HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
	switch(scene_mode) {
		case CamSceneMode_Auto:
			pr_info("CAMDRV_SetSceneMode() called for AUTO\n");
			mt9t111_write(0x098E, 0x6815);	// MCU_ADDRESS [PRI_A_CONFIG_FD_MAX_FDZONE_50HZ]
			mt9t111_write(0x0990, 0x006);   // MCU_DATA_0
			mt9t111_write(0x098E, 0x6817);  // MCU_ADDRESS [PRI_A_CONFIG_FD_MAX_FDZONE_60HZ]
			mt9t111_write(0x0990, 0x006);   // MCU_DATA_0
			mt9t111_write(0x098E, 0x682D); 	// MCU_ADDRESS [PRI_A_CONFIG_AE_TRACK_TARGET_FDZONE]
			mt9t111_write(0x0990, 0x0003);  // MCU_DATA_0
			mt9t111_write(0x098E, 0x682F);  // MCU_ADDRESS [PRI_A_CONFIG_AE_TRACK_TARGET_AGAIN]
			mt9t111_write(0x0990, 0x0100);  // MCU_DATA_0
			mt9t111_write(0x098E, 0x6839);  // MCU_ADDRESS [PRI_A_CONFIG_AE_TRACK_AE_MAX_VIRT_AGAIN]
			mt9t111_write(0x0990, 0x012C);  // MCU_DATA_0
			mt9t111_write(0x098E, 0x6835);  // MCU_ADDRESS [PRI_A_CONFIG_AE_TRACK_AE_MAX_VIRT_DGAIN]
			mt9t111_write(0x0990, 0x00F0);  // MCU_DATA_0
			break;
		case CamSceneMode_Night:
			pr_info("CAMDRV_SetSceneMode() called for Night\n");
			mt9t111_write(0x098E, 0x6815);	// MCU_ADDRESS [PRI_A_CONFIG_FD_MAX_FDZONE_50HZ]
			mt9t111_write(0x0990, 0x0018);  // MCU_DATA_0
			mt9t111_write(0x098E, 0x6817);  // MCU_ADDRESS [PRI_A_CONFIG_FD_MAX_FDZONE_60HZ]
			mt9t111_write(0x0990, 0x0018);  // MCU_DATA_0
			mt9t111_write(0x098E, 0x682D); 	// MCU_ADDRESS [PRI_A_CONFIG_AE_TRACK_TARGET_FDZONE]
			mt9t111_write(0x0990, 0x0006);  // MCU_DATA_0
			mt9t111_write(0x098E, 0x682F);  // MCU_ADDRESS [PRI_A_CONFIG_AE_TRACK_TARGET_AGAIN]
			mt9t111_write(0x0990, 0x0100);  // MCU_DATA_0
			mt9t111_write(0x098E, 0x6839);  // MCU_ADDRESS [PRI_A_CONFIG_AE_TRACK_AE_MAX_VIRT_AGAIN]
			mt9t111_write(0x0990, 0x012C);  // MCU_DATA_0
			mt9t111_write(0x098E, 0x6835);  // MCU_ADDRESS [PRI_A_CONFIG_AE_TRACK_AE_MAX_VIRT_DGAIN]
			mt9t111_write(0x0990, 0x00F0);  // MCU_DATA_0
			break;
		default:
			pr_info("CAMDRV_SetSceneMode() not supported for %d\n", scene_mode);
			break;
	}
	if (sCamI2cStatus != HAL_CAM_SUCCESS) {
		pr_debug("CAMDRV_SetSceneMode(): Error[%d] \r\n",
			 sCamI2cStatus);
		result = sCamI2cStatus;
	}

	return result;
}

/****************************************************************************
/ Function Name:   HAL_CAM_Result_en_t CAMDRV_SetWBMode(CamWB_WBMode_t wb_mode)
/
/ Description: This function will set the white balance of camera
/ Notes:
****************************************************************************/

HAL_CAM_Result_en_t CAMDRV_SetWBMode(CamWB_WBMode_t wb_mode,
				     CamSensorSelect_t sensor)
{
	HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
	pr_info("CAMDRV_SetWBMode()  called\n");
	if (wb_mode == CamWB_Auto) {

		mt9t111_write(0x098E, 0x8002);	/* MCU_ADDRESS [MON_MODE] */
		mt9t111_write(0x0990, 0x0000);	/* MCU_DATA_0 */
		mt9t111_write(0x098E, 0xC8F2);
		mt9t111_write(0x0990, 0x0000);
		mt9t111_write(0x098E, 0xC8F3);
		mt9t111_write(0x0990, 0x007F);
		/* MCU_ADDRESS [PRI_A_CONFIG_AWB_K_R_L] */
		mt9t111_write(0x098E, 0xE84A);
		mt9t111_write(0x0990, 0x0080);	/* MCU_DATA_0 */
		/* MCU_ADDRESS [PRI_A_CONFIG_AWB_K_B_L] */
		mt9t111_write(0x098E, 0xE84C);
		mt9t111_write(0x0990, 0x0080);	/* MCU_DATA_0 */
		/* MCU_ADDRESS [PRI_A_CONFIG_AWB_K_R_R] */
		mt9t111_write(0x098E, 0xE84D);
		mt9t111_write(0x0990, 0x0078);	/* 85   MCU_DATA_0 */
		/* MCU_ADDRESS [PRI_A_CONFIG_AWB_K_B_R] */
		mt9t111_write(0x098E, 0xE84F);
		mt9t111_write(0x0990, 0x007E);	/* 81    MCU_DATA_0 */

	} else if (wb_mode == CamWB_Incandescent) {

		mt9t111_write(0x098E, 0xC8F2);
		mt9t111_write(0x0990, 0x0000);
		mt9t111_write(0x098E, 0xC8F3);
		mt9t111_write(0x0990, 0x0003);
		mt9t111_write(0x098E, 0xAC33);	/* MCU_ADDRESS [AWB_CCMPOSITION] */
		mt9t111_write(0x0990, 0x0000);	/* MCU_DATA_0 */
		/* MCU_ADDRESS [PRI_A_CONFIG_AWB_K_R_L] */
		mt9t111_write(0x098E, 0xE84A);
		mt9t111_write(0x0990, 0x0079);	/* 6C   MCU_DATA_0  */
		/* MCU_ADDRESS [PRI_A_CONFIG_AWB_K_B_L] */
		mt9t111_write(0x098E, 0xE84C);
		mt9t111_write(0x0990, 0x00DB);	/* 9B   MCU_DATA_0 */
		/* MCU_ADDRESS [PRI_A_CONFIG_AWB_K_R_R] */
		mt9t111_write(0x098E, 0xE84D);
		mt9t111_write(0x0990, 0x0079);	/* 6C   MCU_DATA_0 */
		/* MCU_ADDRESS [PRI_A_CONFIG_AWB_K_B_R] */
		mt9t111_write(0x098E, 0xE84F);
		mt9t111_write(0x0990, 0x00DB);	/* 9B   MCU_DATA_0 */

	} else if ((wb_mode == CamWB_DaylightFluorescent)
		   || (wb_mode == CamWB_WarmFluorescent)) {

		mt9t111_write(0x098E, 0xC8F2);
		mt9t111_write(0x0990, 0x0000);
		mt9t111_write(0x098E, 0xC8F3);
		mt9t111_write(0x0990, 0x0003);
		mt9t111_write(0x098E, 0xAC33);	/* MCU_ADDRESS [AWB_CCMPOSITION] */
		mt9t111_write(0x0990, 0x0000);	/* MCU_DATA_0 */
		/* MCU_ADDRESS [PRI_A_CONFIG_AWB_K_R_L] */
		mt9t111_write(0x098E, 0xE84A);
		mt9t111_write(0x0990, 0x0075);	/* 79 6C      MCU_DATA_0 */
		/* MCU_ADDRESS [PRI_A_CONFIG_AWB_K_B_L] */
		mt9t111_write(0x098E, 0xE84C);
		mt9t111_write(0x0990, 0x0099);	/* DB 9B      MCU_DATA_0 */
		/* MCU_ADDRESS [PRI_A_CONFIG_AWB_K_R_R] */
		mt9t111_write(0x098E, 0xE84D);
		mt9t111_write(0x0990, 0x0075);	/* 79 6C      MCU_DATA_0 */
		/* MCU_ADDRESS [PRI_A_CONFIG_AWB_K_B_R] */
		mt9t111_write(0x098E, 0xE84F);
		mt9t111_write(0x0990, 0x0099);	/* DB 9B      MCU_DATA_0 */

	} else if (wb_mode == CamWB_Daylight) {

		mt9t111_write(0x098E, 0xC8F2);
		mt9t111_write(0x0990, 0x0000);
		mt9t111_write(0x098E, 0xC8F3);
		mt9t111_write(0x0990, 0x0003);
		mt9t111_write(0x098E, 0xAC33);	/* MCU_ADDRESS [AWB_CCMPOSITION] */
		mt9t111_write(0x0990, 0x0000);	/* MCU_DATA_0 */
		/* MCU_ADDRESS [PRI_A_CONFIG_AWB_K_R_L] */
		mt9t111_write(0x098E, 0xE84A);
		mt9t111_write(0x0990, 0x008E);	/* 79 6C      MCU_DATA_0 */
		/* MCU_ADDRESS [PRI_A_CONFIG_AWB_K_B_L] */
		mt9t111_write(0x098E, 0xE84C);
		mt9t111_write(0x0990, 0x005C);	/* DB 9B      MCU_DATA_0 */
		/* MCU_ADDRESS [PRI_A_CONFIG_AWB_K_R_R] */
		mt9t111_write(0x098E, 0xE84D);
		mt9t111_write(0x0990, 0x008E);	/* 79 6C      MCU_DATA_0 */
		/* MCU_ADDRESS [PRI_A_CONFIG_AWB_K_B_R] */
		mt9t111_write(0x098E, 0xE84F);
		mt9t111_write(0x0990, 0x005C);	/* DB 9B      MCU_DATA_0 */

	} else if (wb_mode == CamWB_Cloudy) {

		mt9t111_write(0x098E, 0xC8F2);
		mt9t111_write(0x0990, 0x0000);
		mt9t111_write(0x098E, 0xC8F3);
		mt9t111_write(0x0990, 0x0003);
		mt9t111_write(0x098E, 0xAC33);	/* MCU_ADDRESS [AWB_CCMPOSITION] */
		mt9t111_write(0x0990, 0x0000);	/* MCU_DATA_0 */
		/* MCU_ADDRESS [PRI_A_CONFIG_AWB_K_R_L] */
		mt9t111_write(0x098E, 0xE84A);
		mt9t111_write(0x0990, 0x0098);	/* 79 6C      MCU_DATA_0 */
		/* MCU_ADDRESS [PRI_A_CONFIG_AWB_K_B_L] */
		mt9t111_write(0x098E, 0xE84C);
		mt9t111_write(0x0990, 0x004C);	/* DB 9B      MCU_DATA_0 */
		/* MCU_ADDRESS [PRI_A_CONFIG_AWB_K_R_R] */
		mt9t111_write(0x098E, 0xE84D);
		mt9t111_write(0x0990, 0x0098);	/* 79 6C      MCU_DATA_0 */
		/* MCU_ADDRESS [PRI_A_CONFIG_AWB_K_B_R] */
		mt9t111_write(0x098E, 0xE84F);
		mt9t111_write(0x0990, 0x004C);	/* DB 9B      MCU_DATA_0 */

	} else if (wb_mode == CamWB_Twilight) {

		mt9t111_write(0x098E, 0xC8F2);
		mt9t111_write(0x0990, 0x0057);	/* 6F */
		mt9t111_write(0x098E, 0xC8F3);
		mt9t111_write(0x0990, 0x0059);	/* 6F */
		mt9t111_write(0x098E, 0xAC33);	/* MCU_ADDRESS [AWB_CCMPOSITION] */
		mt9t111_write(0x0990, 0x0058);	/* MCU_DATA_0 */
		mt9t111_write(0x098E, 0xE84A);	/*MCU_ADDRESS[PRI_A_CONFIG_AWB_K_R_L] */
		mt9t111_write(0x0990, 0x009F);	/* MCU_DATA_0 */
		mt9t111_write(0x098E, 0xE84B);	/*MCU_ADDRESS[PRI_A_CONFIG_AWB_K_G_L] */
		mt9t111_write(0x0990, 0x0083);	/* MCU_DATA_0 */
		mt9t111_write(0x098E, 0xE84C);	/*MCU_ADDRESS[PRI_A_CONFIG_AWB_K_B_L] */
		mt9t111_write(0x0990, 0x0004);	/* MCU_DATA_0 */
		mt9t111_write(0x098E, 0xE84D);	/*MCU_ADDRESS[PRI_A_CONFIG_AWB_K_R_R] */
		mt9t111_write(0x0990, 0x00A5);	/* MCU_DATA_0 */
		mt9t111_write(0x098E, 0xE84E);	/*MCU_ADDRESS[PRI_A_CONFIG_AWB_K_G_R] */
		mt9t111_write(0x0990, 0x005E);	/* MCU_DATA_0 */
		mt9t111_write(0x098E, 0xE84F);	/*MCU_ADDRESS[PRI_A_CONFIG_AWB_K_B_R] */
		mt9t111_write(0x0990, 0x0083);	/* MCU_DATA_0 */
		mt9t111_write(0x098E, 0xEC4A);	/*MCU_ADDRESS[PRI_B_CONFIG_AWB_K_R_L] */
		mt9t111_write(0x0990, 0x009F);	/* MCU_DATA_0 */
		mt9t111_write(0x098E, 0xEC4B);	/*MCU_ADDRESS[PRI_B_CONFIG_AWB_K_G_L] */
		mt9t111_write(0x0990, 0x0083);	/* MCU_DATA_0 */
		mt9t111_write(0x098E, 0xEC4C);	/*MCU_ADDRESS[PRI_B_CONFIG_AWB_K_B_L] */
		mt9t111_write(0x0990, 0x0004);	/* MCU_DATA_0 */
		mt9t111_write(0x098E, 0xEC4D);	/*MCU_ADDRESS[PRI_B_CONFIG_AWB_K_R_R] */
		mt9t111_write(0x0990, 0x00A5);	/* MCU_DATA_0 */
		mt9t111_write(0x098E, 0xEC4E);	/*MCU_ADDRESS[PRI_B_CONFIG_AWB_K_G_R] */
		mt9t111_write(0x0990, 0x005E);	/* MCU_DATA_0 */
		mt9t111_write(0x098E, 0xEC4F);	/*MCU_ADDRESS[PRI_B_CONFIG_AWB_K_B_R] */
		mt9t111_write(0x0990, 0x0083);	/* MCU_DATA_0 */
		mt9t111_write(0x098E, 0x8400);	/* MCU_ADDRESS */
		mt9t111_write(0x0990, 0x0006);	/* MCU_DATA_0 */
	} else {
		printk("Am here in wb:%d\n", wb_mode);
		mt9t111_write(0x098E, 0x8002);	/* MCU_ADDRESS [MON_MODE] */
		mt9t111_write(0x0990, 0x0000);	/* MCU_DATA_0 */
		mt9t111_write(0x098E, 0xC8F2);
		mt9t111_write(0x0990, 0x0000);
		mt9t111_write(0x098E, 0xC8F3);
		mt9t111_write(0x0990, 0x007F);
		mt9t111_write(0x098E, 0xE84A);	/*MCU_ADDRESS[PRI_A_CONFIG_AWB_K_R_L] */
		mt9t111_write(0x0990, 0x0080);	/* MCU_DATA_0 */
		mt9t111_write(0x098E, 0xE84C);	/*MCU_ADDRESS[PRI_A_CONFIG_AWB_K_B_L] */
		mt9t111_write(0x0990, 0x0080);	/* MCU_DATA_0 */
		mt9t111_write(0x098E, 0xE84D);	/*MCU_ADDRESS[PRI_A_CONFIG_AWB_K_R_R] */
		mt9t111_write(0x0990, 0x0078);	/* 85   MCU_DATA_0 */
		mt9t111_write(0x098E, 0xE84F);	/*MCU_ADDRESS[PRI_A_CONFIG_AWB_K_B_R] */
		mt9t111_write(0x0990, 0x007E);	/* 81   MCU_DATA_0 */
	}

	if (sCamI2cStatus != HAL_CAM_SUCCESS) {
		pr_debug("CAMDRV_SetWBMode(): Error[%d] \r\n", sCamI2cStatus);
		result = sCamI2cStatus;
	}

	return result;
}

/****************************************************************************
/ Function Name:   HAL_CAM_Result_en_t CAMDRV_SetAntiBanding(
/					CamAntiBanding_t effect)
/
/ Description: This function will set the antibanding effect of camera
/ Notes:
****************************************************************************/

HAL_CAM_Result_en_t CAMDRV_SetAntiBanding(CamAntiBanding_t effect,
					  CamSensorSelect_t sensor)
{
	HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
	pr_info("CAMDRV_SetAntiBanding()  called\n");
	if ((effect == CamAntiBandingAuto) || (effect == CamAntiBandingOff)) {
		mt9t111_write(0x098E, 0xA005);	/* MCU_ADDRESS [FD_FDPERIOD_SELECT] */
		mt9t111_write(0x0990, 0x0001);	/* MCU_DATA_0 */
		/* MCU_ADDRESS [PRI_B_CONFIG_FD_ALGO_RUN] */
		mt9t111_write(0x098E, 0x6C11);
		mt9t111_write(0x0990, 0x0003);	/* MCU_DATA_0 */
		/* MCU_ADDRESS [PRI_A_CONFIG_FD_ALGO_RUN] */
		mt9t111_write(0x098E, 0x6811);
		mt9t111_write(0x0990, 0x0003);	/* MCU_DATA_0 */

	} else if (effect == CamAntiBanding50Hz) {

		mt9t111_write(0x098E, 0xA005);	/* MCU_ADDRESS [FD_FDPERIOD_SELECT] */
		mt9t111_write(0x0990, 0x0001);	/* MCU_DATA_0 =>'0'=60Hz, '1'=50Hz */
		mt9t111_write(0x098E, 0x6C11);	/* MCU_ADDRESS */
		mt9t111_write(0x0990, 0x0002);	/* MCU_DATA_0 */
		/* MCU_ADDRESS [PRI_A_CONFIG_FD_ALGO_RUN] */
		mt9t111_write(0x098E, 0x6811);
		mt9t111_write(0x0990, 0x0002);	/* MCU_DATA_0 */

	} else if (effect == CamAntiBanding60Hz) {

		mt9t111_write(0x098E, 0xA005);	/* MCU_ADDRESS [FD_FDPERIOD_SELECT] */
		mt9t111_write(0x0990, 0x0000);	/* MCU_DATA_0 =>'0'=60Hz, '1'=50Hz */
		mt9t111_write(0x098E, 0x6C11);	/* MCU_ADDRESS */
		mt9t111_write(0x0990, 0x0002);	/* MCU_DATA_0 */
		/* MCU_ADDRESS [PRI_A_CONFIG_FD_ALGO_RUN] */
		mt9t111_write(0x098E, 0x6811);
		mt9t111_write(0x0990, 0x0002);	/* MCU_DATA_0 */

	} else {
		mt9t111_write(0x098E, 0xA005);	/* MCU_ADDRESS [FD_FDPERIOD_SELECT] */
		mt9t111_write(0x0990, 0x0001);	/* MCU_DATA_0 */
		/* MCU_ADDRESS [PRI_B_CONFIG_FD_ALGO_RUN] */
		mt9t111_write(0x098E, 0x6C11);
		mt9t111_write(0x0990, 0x0003);	/* MCU_DATA_0 */
		/* MCU_ADDRESS [PRI_A_CONFIG_FD_ALGO_RUN] */
		mt9t111_write(0x098E, 0x6811);
		mt9t111_write(0x0990, 0x0003);	/* MCU_DATA_0 */
	}

	if (sCamI2cStatus != HAL_CAM_SUCCESS) {
		pr_debug("CAMDRV_SetAntiBanding(): Error[%d] \r\n",
			 sCamI2cStatus);
		result = sCamI2cStatus;
	}

	return result;
}

/****************************************************************************
/ Function Name:   HAL_CAM_Result_en_t CAMDRV_SetFlashMode(
					FlashLedState_t effect)
/
/ Description: This function will set the flash mode of camera
/ Notes:
****************************************************************************/

HAL_CAM_Result_en_t CAMDRV_SetFlashMode(FlashLedState_t effect,
					CamSensorSelect_t sensor)
{
	HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
	pr_info("CAMDRV_SetFlashMode()  called\n");
	if (effect == Flash_Off) {
		/* do sth */
	} else if (effect == Flash_On) {
		/* do sth */
	} else if (effect == Torch_On) {
		/* do sth */
	} else if (effect == FlashLight_Auto) {
		/* do sth */
	} else {
		/* do sth */
	}
	if (sCamI2cStatus != HAL_CAM_SUCCESS) {
		pr_debug("CAMDRV_SetFlashMode(): Error[%d] \r\n",
			 sCamI2cStatus);
		result = sCamI2cStatus;
	}
	return result;
}

/****************************************************************************
/ Function Name:   HAL_CAM_Result_en_t CAMDRV_SetFocusMode(
/					CamFocusStatus_t effect)
/
/ Description: This function will set the focus mode of camera
/ Notes:
****************************************************************************/

HAL_CAM_Result_en_t CAMDRV_SetFocusMode(CamFocusControlMode_t effect,
					CamSensorSelect_t sensor)
{
	HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
	pr_info("CAMDRV_SetFocusMode()  called\n");
	if (effect == CamFocusControlAuto) {

		mt9t111_write(0x098E, 0xB03A);	/* MCU_ADDRESS */
		mt9t111_write(0x0990, 0x0032);	/* MCU_DATA_0 */
		mt9t111_write(0x098E, 0xB04A);	/* MCU_ADDRESS */
		mt9t111_write(0x0990, 0x0064);	/* MCU_DATA_0 */
		mt9t111_write(0x098E, 0xB048);	/* MCU_ADDRESS */
		mt9t111_write(0x0990, 0x0002);	/* MCU_DATA_0 */
		mt9t111_write(0x098E, 0xB041);	/* MCU_ADDRESS */
		mt9t111_write(0x0990, 0x002A);	/* MCU_DATA_0 */
		mt9t111_write(0x098E, 0xB042);	/* MCU_ADDRESS */
		mt9t111_write(0x0990, 0x0004);	/* MCU_DATA_0 */
		mt9t111_write(0x098E, 0xB043);	/* MCU_ADDRESS */
		mt9t111_write(0x0990, 0x0014);	/* MCU_DATA_0 */
		mt9t111_write(0x098E, 0xB044);	/* MCU_ADDRESS */
		mt9t111_write(0x0990, 0x0002);	/* MCU_DATA_0 */
		mt9t111_write(0x098E, 0xB045);	/* MCU_ADDRESS */
		mt9t111_write(0x0990, 0x0014);	/* MCU_DATA_0 */
		mt9t111_write(0x098E, 0xB046);	/* MCU_ADDRESS */
		mt9t111_write(0x0990, 0x0002);	/* MCU_DATA_0 */
		mt9t111_write(0x098E, 0x3003);	/* MCU_ADDRESS */
		mt9t111_write(0x0990, 0x0010);	/* MCU_DATA_0 */
		mt9t111_write(0x098E, 0xB019);	/* MCU_ADDRESS */
		mt9t111_write(0x0990, 0x0001);	/* MCU_DATA_0 */

	} else if (effect == CamFocusControlMacro) {

		mt9t111_write(0x098E, 0x3003);	/* MCU_ADDRESS */
		mt9t111_write(0x0990, 0x0001);	/* MCU_DATA_0 */
		mt9t111_write(0x098E, 0xB024);	/* MCU_ADDRESS */
		mt9t111_write(0x0990, 0x0000);	/* MCU_DATA_0 */
		mt9t111_write(0x098E, 0x3003);	/* MCU_ADDRESS */
		mt9t111_write(0x0990, 0x0002);	/* AF_ALGO */
		mt9t111_write(0x098E, 0xB019);	/* MCU_ADDRESS [AF_PROGRESS] */
		mt9t111_write(0x0990, 0x0001);	/* MCU_DATA_0 */

	} else if (effect == CamFocusControlInfinity) {

		mt9t111_write(0x098E, 0x3003);	/* MCU_ADDRESS */
		mt9t111_write(0x0990, 0x0001);	/* MCU_DATA_0 */
		mt9t111_write(0x098E, 0xB024);	/* MCU_ADDRESS */
		mt9t111_write(0x0990, 0xFFFF);	/* MCU_DATA_0 */
		mt9t111_write(0x098E, 0x3003);	/* MCU_ADDRESS */
		mt9t111_write(0x0990, 0x0002);	/* AF_ALGO */
		mt9t111_write(0x098E, 0xB019);	/* MCU_ADDRESS [AF_PROGRESS] */
		mt9t111_write(0x0990, 0x0001);	/* MCU_DATA_0 */

	} else {

		mt9t111_write(0x098E, 0xB03A);	/* MCU_ADDRESS */
		mt9t111_write(0x0990, 0x0032);	/* MCU_DATA_0 */
		mt9t111_write(0x098E, 0xB04A);	/* MCU_ADDRESS */
		mt9t111_write(0x0990, 0x0064);	/* MCU_DATA_0 */
		mt9t111_write(0x098E, 0xB048);	/* MCU_ADDRESS */
		mt9t111_write(0x0990, 0x0002);	/* MCU_DATA_0 */
		mt9t111_write(0x098E, 0xB041);	/* MCU_ADDRESS */
		mt9t111_write(0x0990, 0x002A);	/* MCU_DATA_0 */
		mt9t111_write(0x098E, 0xB042);	/* MCU_ADDRESS */
		mt9t111_write(0x0990, 0x0004);	/* MCU_DATA_0 */
		mt9t111_write(0x098E, 0xB043);	/* MCU_ADDRESS */
		mt9t111_write(0x0990, 0x0014);	/* MCU_DATA_0 */
		mt9t111_write(0x098E, 0xB044);	/* MCU_ADDRESS */
		mt9t111_write(0x0990, 0x0002);	/* MCU_DATA_0 */
		mt9t111_write(0x098E, 0xB045);	/* MCU_ADDRESS */
		mt9t111_write(0x0990, 0x0014);	/* MCU_DATA_0 */
		mt9t111_write(0x098E, 0xB046);	/* MCU_ADDRESS */
		mt9t111_write(0x0990, 0x0002);	/* MCU_DATA_0 */
		mt9t111_write(0x098E, 0x3003);	/* MCU_ADDRESS */
		mt9t111_write(0x0990, 0x0010);	/* MCU_DATA_0 */
		mt9t111_write(0x098E, 0xB019);	/* MCU_ADDRESS */
		mt9t111_write(0x0990, 0x0001);	/* MCU_DATA_0 */

	}
	if (sCamI2cStatus != HAL_CAM_SUCCESS) {
		pr_debug("CAMDRV_SetFocusMode(): Error[%d] \r\n",
			 sCamI2cStatus);
		result = sCamI2cStatus;
	}
	return result;
}

HAL_CAM_Result_en_t CAMDRV_TurnOffAF()
{
	HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
	pr_info("CAMDRV_TurnOffAF() called\n");
	mt9t111_write(0x0604, 0x0F00);    /* MCU_ADDRESS */
	mt9t111_write(0x0606, 0x0F00);    /* MCU_DATA_0 */

	if (sCamI2cStatus != HAL_CAM_SUCCESS) {
		pr_debug("CAMDRV_TurnOffAF(): Error[%d] \r\n",
			 sCamI2cStatus);
		result = sCamI2cStatus;
	}
	return result;
}

HAL_CAM_Result_en_t CAMDRV_TurnOnAF()
{
	HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
	/* AF DriverIC power enable */
	pr_info("CAMDRV_TurnOnAF() called\n");
	mt9t111_write(0x0604, 0x0F01);    /* MCU_ADDRESS */
	mt9t111_write(0x0606, 0x0F00);    /* MCU_DATA_0 */

	if (sCamI2cStatus != HAL_CAM_SUCCESS) {
		pr_debug("CAMDRV_TurnOnAF(): Error[%d] \r\n",
			 sCamI2cStatus);
		result = sCamI2cStatus;
	}
	return result;
}

/****************************************************************************
/ Function Name:   HAL_CAM_Result_en_t CAMDRV_SetJpegQuality(
/					CamFocusStatus_t effect)
/
/ Description: This function will set the focus mode of camera
/ Notes:
****************************************************************************/
HAL_CAM_Result_en_t CAMDRV_SetJpegQuality(CamJpegQuality_t effect,
					  CamSensorSelect_t sensor)
{
	HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
	pr_info("CAMDRV_SetJpegQuality()  called\n");
	if (effect == CamJpegQuality_Min) {
		/* do sth */
	} else if (effect == CamJpegQuality_Nom) {
		/* do sth */
	} else {
		/* do sth */
	}
	if (sCamI2cStatus != HAL_CAM_SUCCESS) {
		pr_debug("CAMDRV_SetJpegQuality(): Error[%d] \r\n",
			 sCamI2cStatus);
		result = sCamI2cStatus;
	}
	return result;
}

/****************************************************************************
/ Function Name:   HAL_CAM_Result_en_t CAMDRV_SetZoom()
/
/ Description: This function will set the zoom value of camera
/ Notes:
****************************************************************************/
HAL_CAM_Result_en_t CAMDRV_SetZoom(CamZoom_t step,
					  CamSensorSelect_t sensor)
{
	HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
	pr_info("CAMDRV_SetZoom()  called\n");
	if (step == CamZoom_1_0) {
		//TBD
	} else if (step == CamZoom_1_15) {
		//TBD
	} else if (step == CamZoom_1_6) {
		//TBD
	} else if (step == CamZoom_2_0) {
		//TBD
	} else {
		//TBD
	}
	if (sCamI2cStatus != HAL_CAM_SUCCESS) {
		pr_debug("CAMDRV_SetZoom(): Error[%d] \r\n",
			 sCamI2cStatus);
		result = sCamI2cStatus;
	}
	return result;
}



/****************************************************************************
/ Function Name:   HAL_CAM_Result_en_t CAMDRV_SetDigitalEffect(
/					CamDigEffect_t effect)
/
/ Description: This function will set the digital effect of camera
/ Notes:
****************************************************************************/
HAL_CAM_Result_en_t CAMDRV_SetDigitalEffect(CamDigEffect_t effect,
					    CamSensorSelect_t sensor)
{
	HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
	if (effect == CamDigEffect_NoEffect) {

		mt9t111_write(0x098E, 0xE883);
		mt9t111_write(0x0990, 0x0000);
		mt9t111_write(0x098E, 0xEC83);
		mt9t111_write(0x0990, 0x0000);
		mt9t111_write(0x098E, 0x8400);
		mt9t111_write(0x0990, 0x0006);

	} else if (effect == CamDigEffect_MonoChrome) {

		mt9t111_write(0x098E, 0xE883);
		mt9t111_write(0x0990, 0x0001);
		mt9t111_write(0x098E, 0xEC83);
		mt9t111_write(0x0990, 0x0001);
		mt9t111_write(0x098E, 0x8400);
		mt9t111_write(0x0990, 0x0006);

	} else if ((effect == CamDigEffect_NegColor)
		   || (effect == CamDigEffect_NegMono)) {
		mt9t111_write(0x098E, 0xE883);
		mt9t111_write(0x0990, 0x0003);
		mt9t111_write(0x098E, 0xEC83);
		mt9t111_write(0x0990, 0x0003);
		mt9t111_write(0x098E, 0x8400);
		mt9t111_write(0x0990, 0x0006);

	} else if ((effect == CamDigEffect_SolarizeColor)
		   || (effect == CamDigEffect_SolarizeMono)) {
		mt9t111_write(0x098E, 0xE883);
		mt9t111_write(0x0990, 0x0004);
		mt9t111_write(0x098E, 0xEC83);
		mt9t111_write(0x0990, 0x0004);
		mt9t111_write(0x098E, 0x8400);
		mt9t111_write(0x0990, 0x0006);

	} else if (effect == CamDigEffect_SepiaGreen) {

		mt9t111_write(0x098E, 0xE883);
		mt9t111_write(0x0990, 0x0002);
		mt9t111_write(0x098E, 0xEC83);
		mt9t111_write(0x0990, 0x0002);
		mt9t111_write(0x098E, 0xE885);
		mt9t111_write(0x0990, 0x0037);
		mt9t111_write(0x098E, 0xEC85);
		mt9t111_write(0x0990, 0x0037);
		mt9t111_write(0x098E, 0xE886);
		mt9t111_write(0x0990, 0x00BE);
		mt9t111_write(0x098E, 0xEC86);
		mt9t111_write(0x0990, 0x00BE);
		mt9t111_write(0x098E, 0x8400);
		mt9t111_write(0x0990, 0x0006);

	} else if (effect == CamDigEffect_Auqa) {

		/* MCU_ADDRESS [PRI_A_CONFIG_SYSCTRL_SELECT_FX] */
		mt9t111_write(0x098E, 0xE883);
		mt9t111_write(0x0990, 0x0002);	/* MCU_DATA_0 */
		/* MCU_ADDRESS [PRI_B_CONFIG_SYSCTRL_SELECT_FX] */
		mt9t111_write(0x098E, 0xEC83);
		mt9t111_write(0x0990, 0x0002);	/* MCU_DATA_0 */
		/* MCU_ADDRESS [PRI_A_CONFIG_SYSCTRL_SEPIA_CR] */
		mt9t111_write(0x098E, 0xE885);
		mt9t111_write(0x0990, 0x008C);	/* MCU_DATA_0 */
		/* MCU_ADDRESS [PRI_B_CONFIG_SYSCTRL_SEPIA_CR] */
		mt9t111_write(0x098E, 0xEC85);
		mt9t111_write(0x0990, 0x008C);	/* MCU_DATA_0 */
		/* MCU_ADDRESS [PRI_A_CONFIG_SYSCTRL_SEPIA_CB] */
		mt9t111_write(0x098E, 0xE886);
		mt9t111_write(0x0990, 0x0042);	/* MCU_DATA_0 */
		/* MCU_ADDRESS [PRI_B_CONFIG_SYSCTRL_SEPIA_CB] */
		mt9t111_write(0x098E, 0xEC86);
		mt9t111_write(0x0990, 0x0042);	/* MCU_DATA_0 */
		mt9t111_write(0x098E, 0x8400);	/* MCU_ADDRESS [SEQ_CMD] */
		mt9t111_write(0x0990, 0x0006);	/* MCU_DATA_0 */

	} else {
		mt9t111_write(0x098E, 0xE883);
		mt9t111_write(0x0990, 0x0000);
		mt9t111_write(0x098E, 0xEC83);
		mt9t111_write(0x0990, 0x0000);
		mt9t111_write(0x098E, 0x8400);
		mt9t111_write(0x0990, 0x0006);

	}
	if (sCamI2cStatus != HAL_CAM_SUCCESS) {
		pr_debug("CAMDRV_SetDigitalEffect(): Error[%d] \r\n",
			 sCamI2cStatus);
		result = sCamI2cStatus;
	}

	return result;
}

/**
*  This function will perform specific action from defined list,
*  copy of parameters passed thru parm structure.
*
*/
HAL_CAM_Result_en_t CAMDRV_ActionCtrl(HAL_CAM_Action_en_t action,
				      void *data, void *callback)
{
	HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;

	pr_debug("HAL_CAMDRV_Ctrl():  action=%d \r\n", action);

	switch (action) {
#if 0
/* Operation Control Settings */
		/* < Get Jpeg Max size (bytes),use HAL_CAM_Action_param_st_t to get */
	case ACTION_CAM_GetJpegMaxSize:
		result =
		    CAMDRV_GetJpegMaxSize(((HAL_CAM_Action_param_st_t *) data)->
					  param,
					  ((HAL_CAM_Action_param_st_t *) data)->
					  sensor);
		break;

		/* < Get closest sensor resolution from passed pixel width and height,
		   use HAL_CAM_Action_image_size_st_t to get CamImageSize_t */
	case ACTION_CAM_GetImageSize:
		result =
		    CAMDRV_GetResolutionAvailable(((HAL_CAM_Action_image_size_st_t *)
						   data)->width,
						  ((HAL_CAM_Action_image_size_st_t *)
						   data)->height,
						  ((HAL_CAM_Action_image_size_st_t *)
						   data)->mode,
						  ((HAL_CAM_Action_image_size_st_t *) data)->sensor, ((HAL_CAM_Action_image_size_st_t *) data)->sensor_size);
		break;
		/* < Get matching image size from passed pixel width and height,
		   use HAL_CAM_Action_image_size_st_t to get CamImageSize_t */
	case ACTION_CAM_GetSensorResolution:
		result =
		    CAMDRV_GetResolution(((HAL_CAM_Action_resolution_st_t *)
					  data)->size,
					 ((HAL_CAM_Action_resolution_st_t *)
					  data)->mode,
					 ((HAL_CAM_Action_resolution_st_t *)
					  data)->sensor,
					 ((HAL_CAM_Action_resolution_st_t *)
					  data)->sensor_size);
		break;
		/* < Set required picture frame, use (CamRates_t)
		   HAL_CAM_Action_param_st_t to set */
	case ACTION_CAM_SetFrameRate:
		result = CAMDRV_SetFrameRate((CamRates_t)
					     (((HAL_CAM_Action_param_st_t *)
					       data)->param),
					     ((HAL_CAM_Action_param_st_t *)
					      data)->sensor);
		break;
#endif
/* Unsupported Actions in Camera Device Driver
	Return:  HAL_CAM_ERROR_ACTION_NOT_SUPPORTED */
	default:
		pr_debug("HAL_CAMDRV_Ctrl(): Invalid Action \r\n");
		result = HAL_CAM_ERROR_ACTION_NOT_SUPPORTED;
		break;
	}
	return result;
}

ktime_t tm1;
static HAL_CAM_Result_en_t Init_Mt9t111(CamSensorSelect_t sensor)
{
	HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
	int timeout;
	tm1 = ktime_get();
	pr_info("Entry Init Sec %d nsec %d\n", tm1.tv.sec, tm1.tv.nsec);

	CamSensorCfg_st.sensor_config_caps = &CamPrimaryCfgCap_st;
	pr_debug("Init Primary Sensor MT9T111: \r\n");

	/*     MCNEX Sensor register */
	/*
	   model name           : BRAVA 8313D
	   Product        : MT9T111
	   Date           : 2009.07.17 ~
	   version                          : rev3
	   customer                 : YUHUA
	   sensor vender  : Micron
	   sensor model       : MT9T111
	   MCLK Freq      : 26MHz
	   Pixel Clock    : 52MHz
	   mirror                           : 0
	   (0:Normal, 1:vertical, 2: horizontal, 3: inverse)
	   Max Frame Rate : auto 12 fps
	   Flicker        : 60Hz
	   Preview        : 800*600
	   =========================================================================/
	   ///////////////////////////////////////////////////////////////////////////
	   //
	   //  descripition : - mclk : 26mhz , pclk : 52mhz,
	   //
	   ///////////////////////////////////////////////////////////////////////////
	   //--------------------------------------------------------------------//
	   //[=============================================================]
	   //[    MT9T111-Rev3, Mclk:26, Pclk:52, Max 12fps              ]
	   //[=============================================================]
	   //[BRAVA2 initial register values]
	   //XMCLK=24000000
	   //670nm AWB weighted modify
	   //XMCLK=24000000
	   //-------------------------------------------------- */

	mt9t111_write(0x001A, 0x001D);	/* RESET_AND_MISC_CONTROL */
	msleep(10);
	mt9t111_write(0x001A, 0x0018);	/* RESET_AND_MISC_CONTROL */
	msleep(30);
	mt9t111_write(0x0014, 0x2425);	/* PLL_CONTROLBITFIELD= 0x14, 1, 1);
					   [ 24 TO 48 ] */
	mt9t111_write(0x0014, 0x2425);	/* PLL_CONTROLBITFIELD= 0X14, 2, 0);
					   `                    mt9t111_write(0x0014, 0x2425);PLL_CONTROL */
	mt9t111_write(0x0014, 0x2145);	/* PLL control: BYPASS PLL = 8517
					   mt9t111_write(0x0014, 0x2425);PLL_CONTROL */
	mt9t111_write(0x0010, 0x0C88);	/* PLL Dividers = 276 */
	mt9t111_write(0x0012, 0x0070);	/* PLL P Dividers = 112 */
	mt9t111_write(0x002A, 0x7799);	/* PLL P Dividers 4-5-6 = 30616 */
	mt9t111_write(0x001A, 0x0218);	/* Reset Misc. Control = 536
					   mt9t111_write(0x002A, 0x77BB );
					   PLL_P4_P5_P6_DIVIDERS */
	msleep(30);
	mt9t111_write(0x0014, 0x2545);	/* PLL control: TEST_BYPASS on = 9541
					   mt9t111_write(0x001A, 0x0118 );
					   RESET_AND_MISC_CONTROL */
	mt9t111_write(0x0014, 0x2547);	/* PLL control: PLL_ENABLE on = 9543 */
	mt9t111_write(0x0014, 0x2447);	/* PLL control: SEL_LOCK_DET on = 9287
					   mt9t111_write(0x0014, 0x2545 );
					   PLL_CONTROL */
	mt9t111_write(0x0014, 0x2047);	/* PLL control: TEST_BYPASS off = 8263
					   mt9t111_write(0x0014, 0x2547);PLL_CONTROL */

	msleep(10);		/*,TIMEOUT=1000    Wait for the PLL to lock
				   mt9t111_write(0x0014, 0x2447 ); PLL_CONTROL */

	timeout = 100;
	do {
		if (mt9t111_read(0x14) & 0x8000) {
			break;
		}
		msleep(1);

		timeout--;
		if (!timeout) {
			pr_debug("mt9t111_init1(): PLL lock failed \r\n");
			break;
		}

	} while (1);

	mt9t111_write(0x0014, 0x2046);	/*PLL control: PLL_BYPASS off = 8262
					   mt9t111_write(0x0014, 0x2047 ); PLL_CONTROL */
	mt9t111_write(0x0022, 0x0208);	/* Reference clock count for 20 us = 520 */
	mt9t111_write(0x001E, 0x0777);	/* Pad Slew Rate = 1911 DELAY=10 */
	mt9t111_write(0x0016, 0x0400);	/* JPEG Clock = 1024
					   mt9t111_write(0x0014, 0x2046 );
					   PLL_CONTROL */
	mt9t111_write(0x3B84, 0x0220);	/* I2C Master Clock Divider = 462 */
	mt9t111_write(0x0018, 0x4028);	/* Out of Standby */
	/* replace these setting when new timing parameters are generated */
	mt9t111_write(0x0022, 0x01E0);	/* VDD_DIS_COUNTER */
	/* [ ADDITIONAL_SETTING ] Run MCU and start streaming(preview) */
	mt9t111_write(0x0018, 0x4028);	/* STANDBY_CONTROL_AND_STATUS */
	/* enable parallel interface, disable MIPI */
	mt9t111_write(0x001A, 0x0218);	/* RESET_AND_MISC_CONTROL */
	mt9t111_write(0x001A, 0x0218);	/* RESET_AND_MISC_CONTROL */
	/* mt9t111_write(0x001E, 0x0703 ); PAD_SLEW_PAD_CONFIG */
	/* optimal power consumption */
	mt9t111_write(0x3084, 0x2406);	/* 0x2409);
					   RESERVED_CORE_3084 DAC_LD_4_5 */
	mt9t111_write(0x3092, 0x0A46);	/* 0x0A49 );
					   RESERVED_CORE_3092 DAC_LD_18_19 */
	mt9t111_write(0x3094, 0x4646);	/* 0x4949 );
					   RESERVED_CORE_3094 DAC_LD_20_21 */
	mt9t111_write(0x3096, 0x4649);	/* 0x4950 );
					   RESERVED_CORE_3096 DAC_LD_22_23 */

	/* power consumption reduction by disable JPEG CLK */
	/* mt9t111_write(0x0016, 0x02DF );0x0400 ); CLOCKS_CONTROL  !! */

	/* Disabel adaptive clock */
	/* MCU_ADDRESS [PRI_A_CONFIG_JPEG_OB_TX_CONTROL_VAR] */
	mt9t111_write(0x098E, 0x68A0);
	mt9t111_write(0x0990, 0x082E);	/* MCU_DATA_0 */
	/* MCU_ADDRESS [PRI_B_CONFIG_JPEG_OB_TX_CONTROL_VAR] */
	mt9t111_write(0x098E, 0x6CA0);
	mt9t111_write(0x0990, 0x082E);	/* MCU_DATA_0 */
	/* invert PCLK back to Rev2 mode */
	mt9t111_write(0x3C20, 0x0000);	/* TX_SS_CONTROL */
	/* temp noise */
	/* 0x0406 ); RESERVED_CORE_316C  20090713 MODIFY - HOT PIXEL rdduce */
	mt9t111_write(0x316C, 0x350F);
	/* AFM driver & I2C initialize */
	/* master I2C enable */
	mt9t111_write(0x0614, 0x0000);	/* SECOND_SCL_SDA_PD */

	/*time                                             [timing set] */
	mt9t111_write(0x98E, 0x6800);	/* Output Width (A) */
	mt9t111_write(0x990, 0x0140);	/*      = 1024 */
	mt9t111_write(0x98E, 0x6802);	/* Output Height (A) */
	mt9t111_write(0x990, 0x0F0);	/*      = 768 */
	mt9t111_write(0x98E, 0xE88E);	/* JPEG (A) */
	mt9t111_write(0x990, 0x00);	/*      = 0 */
	mt9t111_write(0x98E, 0x68A0);	/* Adaptive Output Clock (A) */
	mt9t111_write(0x990, 0x082E);	/*      = 0 */
	mt9t111_write(0x98E, 0x4802);	/* Row Start (A) */
	mt9t111_write(0x990, 0x000);	/*      = 0 */
	mt9t111_write(0x98E, 0x4804);	/* Column Start (A) */
	mt9t111_write(0x990, 0x000);	/*      = 0 */
	mt9t111_write(0x98E, 0x4806);	/* Row End (A) */
	mt9t111_write(0x990, 0x60D);	/*      = 1549 */
	mt9t111_write(0x98E, 0x4808);	/* Column End (A) */
	mt9t111_write(0x990, 0x80D);	/*      = 2061 */
	mt9t111_write(0x98E, 0x480A);	/* Row Speed (A) */
	mt9t111_write(0x990, 0x0111);	/*      = 273 */
	mt9t111_write(0x98E, 0x480C);	/* Read Mode (A) */
	mt9t111_write(0x990, 0x046C);	/*      = 1132 */
	mt9t111_write(0x98E, 0x480F);	/* Fine Correction (A) */
	mt9t111_write(0x990, 0x00CC);	/*      = 204 */
	mt9t111_write(0x98E, 0x4811);	/* Fine IT Min (A) */
	mt9t111_write(0x990, 0x0381);	/*       = 897 */
	mt9t111_write(0x98E, 0x4813);	/* Fine IT Max Margin (A) */
	mt9t111_write(0x990, 0x024F);	/*      = 591 */
	mt9t111_write(0x98E, 0x481D);	/* Base Frame Lines (A) */
	mt9t111_write(0x990, 0x035B);	/*      = 940 */
	mt9t111_write(0x98E, 0x481F);	/* Min Line Length (A) */
	mt9t111_write(0x990, 0x05D0);	/*      = 1488 */
	mt9t111_write(0x98E, 0x4825);	/* Line Length (A) */
	mt9t111_write(0x990, 0x07AC);	/*      = 2047 */
	mt9t111_write(0x98E, 0x482B);	/* Contex Width (A) */
	mt9t111_write(0x990, 0x0408);	/*      = 1032 */
	mt9t111_write(0x98E, 0x482D);	/* Context Height (A) */
	mt9t111_write(0x990, 0x0308);	/*      = 776 */
	mt9t111_write(0x98E, 0x6C00);	/* Output Width (B) */
	mt9t111_write(0x990, 0x0800);	/*      = 2048 */
	mt9t111_write(0x98E, 0x6C02);	/* Output Height (B) */
	mt9t111_write(0x990, 0x0600);	/*      = 1536 */
	mt9t111_write(0x98E, 0xEC8E);	/* JPEG (B) */
	mt9t111_write(0x990, 0x01);	/*      = 1 */
	mt9t111_write(0x98E, 0x6CA0);	/* Adaptive Output Clock (B) */
	mt9t111_write(0x990, 0x082E);	/*      = 0 */
	mt9t111_write(0x98E, 0x484A);	/* Row Start (B) */
	mt9t111_write(0x990, 0x004);	/*      = 4 */
	mt9t111_write(0x98E, 0x484C);	/* Column Start (B) */
	mt9t111_write(0x990, 0x004);	/*      = 4 */
	mt9t111_write(0x98E, 0x484E);	/* Row End (B) */
	mt9t111_write(0x990, 0x60B);	/*      = 1547 */
	mt9t111_write(0x98E, 0x4850);	/* Column End (B) */
	mt9t111_write(0x990, 0x80B);	/*      = 2059 */
	mt9t111_write(0x98E, 0x4852);	/* Row Speed (B) */
	mt9t111_write(0x990, 0x0111);	/*      = 273 */
	mt9t111_write(0x98E, 0x4854);	/* Read Mode (B) */
	mt9t111_write(0x990, 0x0024);	/*      = 36 */
	mt9t111_write(0x98E, 0x4857);	/* Fine Correction (B) */
	mt9t111_write(0x990, 0x008C);	/*      = 140 */
	mt9t111_write(0x98E, 0x4859);	/* Fine IT Min (B) */
	mt9t111_write(0x990, 0x01F1);	/*      = 497 */
	mt9t111_write(0x98E, 0x485B);	/* Fine IT Max Margin (B) */
	mt9t111_write(0x990, 0x00FF);	/*      = 255 */
	mt9t111_write(0x98E, 0x4865);	/* Base Frame Lines (B) */
	mt9t111_write(0x990, 0x0662);	/*      = 1634 */
	mt9t111_write(0x98E, 0x4867);	/* Min Line Length (B) */
	mt9t111_write(0x990, 0x0378);	/*      = 888 */
	mt9t111_write(0x98E, 0x486D);	/* Line Length (B) */
	mt9t111_write(0x990, 0x0DEA);	/*      = 2618 */
	mt9t111_write(0x98E, 0x4873);	/* Contex Width (B) */
	mt9t111_write(0x990, 0x0808);	/*      = 2056 */
	mt9t111_write(0x98E, 0x4875);	/* Context Height (B) */
	mt9t111_write(0x990, 0x0608);	/*      = 1544 */
	mt9t111_write(0x98E, 0xC8A5);	/* search_f1_50 */
	mt9t111_write(0x990, 0x25);	/*      = 38 */
	mt9t111_write(0x98E, 0xC8A6);	/* search_f2_50 */
	mt9t111_write(0x990, 0x27);	/*      = 40 */
	mt9t111_write(0x98E, 0xC8A7);	/* search_f1_60 */
	mt9t111_write(0x990, 0x2D);	/*      = 46 */
	mt9t111_write(0x98E, 0xC8A8);	/* search_f2_60 */
	mt9t111_write(0x990, 0x2F);	/*     = 48 */
	mt9t111_write(0x98E, 0xC844);	/* period_50Hz (A) */
	mt9t111_write(0x990, 0x15);	/*      = 26 */
	mt9t111_write(0x98E, 0xC92F);	/* period_50Hz (A MSB) */
	mt9t111_write(0x990, 0x01);	/*      = 1 */
	mt9t111_write(0x98E, 0xC845);	/* period_60Hz (A) */
	mt9t111_write(0x990, 0xE7);	/*      = 235 */
	mt9t111_write(0x98E, 0xC92D);	/* period_60Hz (A MSB) */
	mt9t111_write(0x990, 0x00);	/*      = 0 */
	mt9t111_write(0x98E, 0xC88C);	/* period_50Hz (B) */
	mt9t111_write(0x990, 0x99);	/*      = 221 */
	mt9t111_write(0x98E, 0xC930);	/* period_50Hz (B) MSB */
	mt9t111_write(0x990, 0x00);	/*      = 0 */
	mt9t111_write(0x98E, 0xC88D);	/* period_60Hz (B) */
	mt9t111_write(0x990, 0xB8);	/*      = 184 */
	mt9t111_write(0x98E, 0xC92E);	/* period_60Hz (B) MSB */
	mt9t111_write(0x990, 0x00);	/*      = 0 */
	mt9t111_write(0x98E, 0xB825);	/* FD Window Height */
	mt9t111_write(0x990, 0x06);	/*      = 6 */
	mt9t111_write(0x98E, 0xA009);	/* Stat_min */
	mt9t111_write(0x990, 0x02);	/*      = 2 */
	mt9t111_write(0x98E, 0xA00A);	/* Stat_max */
	mt9t111_write(0x990, 0x03);	/*      = 3 */
	mt9t111_write(0x98E, 0xA00C);	/* Min_amplitude */
	mt9t111_write(0x990, 0x0A);	/*      = 10 */
	mt9t111_write(0x98E, 0x4846);	/* RX FIFO Watermark (A) */
	mt9t111_write(0x990, 0x0080);	/*      = 128 */
	mt9t111_write(0x98E, 0x68AA);	/* TX FIFO Watermark (A) */
	mt9t111_write(0x990, 0x0234);	/*      = 536 */
	mt9t111_write(0x98E, 0x6815);	/* Max FD Zone 50 Hz */
	mt9t111_write(0x990, 0x0003);	/*      = 7 Change to 3 for 30 fps.
					   1 and 2 max-out at 16-17 */
	mt9t111_write(0x98E, 0x6817);	/* Max FD Zone 60 Hz */
	mt9t111_write(0x990, 0x0003);	/*      = 8 */
	mt9t111_write(0x98E, 0x682D);	/* AE Target FD Zone */
	mt9t111_write(0x990, 0x0003);	/*      = 7 */
	mt9t111_write(0x98E, 0x488E);	/* RX FIFO Watermark (B) */
	mt9t111_write(0x990, 0x0080);	/*      = 128 */
	mt9t111_write(0x98E, 0x6CAA);	/* TX FIFO Watermark (B) */
	mt9t111_write(0x990, 0x00E6);	/*      = 230 */
	mt9t111_write(0x098E, 0xA005);	/* MCU_ADDRESS [FD_FDPERIOD_SELECT] */
	mt9t111_write(0x0990, 0x0001);	/* MCU_DATA_0 =>'0'=60Hz, '1'=50Hz */
	mt9t111_write(0x098E, 0x6C11);	/* MCU_ADDRESS [PRI_B_CONFIG_FD_ALGO_RUN] */
	mt9t111_write(0x0990, 0x0003);	/* MCU_DATA_0 =>'2'=manual, '3'=auto */
	mt9t111_write(0x098E, 0x6811);	/* MCU_ADDRESS [PRI_A_CONFIG_FD_ALGO_RUN] */
	mt9t111_write(0x0990, 0x0003);	/* MCU_DATA_0 =>'2'=manual, '3'=auto */

	/* lens */
	mt9t111_write(0x364A, 0x0270);	/* P_R_P0Q0 */
	mt9t111_write(0x364C, 0xE50D);	/* P_R_P0Q1 */
	mt9t111_write(0x364E, 0x5BD1);	/* P_R_P0Q2 */
	mt9t111_write(0x3650, 0xA70E);	/* P_R_P0Q3 */
	mt9t111_write(0x3652, 0xD3B1);	/* P_R_P0Q4 */
	mt9t111_write(0x368A, 0x5B0E);	/* P_R_P1Q0 */
	mt9t111_write(0x368C, 0x426F);	/* P_R_P1Q1 */
	mt9t111_write(0x368E, 0x1D90);	/* P_R_P1Q2 */
	mt9t111_write(0x3690, 0xC0D0);	/* P_R_P1Q3 */
	mt9t111_write(0x3692, 0xA0B1);	/* P_R_P1Q4 */
	mt9t111_write(0x36CA, 0x3232);	/* P_R_P2Q0 */
	mt9t111_write(0x36CC, 0x9490);	/* P_R_P2Q1 */
	mt9t111_write(0x36CE, 0x7B32);	/* P_R_P2Q2 */
	mt9t111_write(0x36D0, 0x7DB0);	/* P_R_P2Q3 */
	mt9t111_write(0x36D2, 0xA6D6);	/* P_R_P2Q4 */
	mt9t111_write(0x370A, 0x5BB0);	/* P_R_P3Q0 */
	mt9t111_write(0x370C, 0xCA0D);	/* P_R_P3Q1 */
	mt9t111_write(0x370E, 0x9493);	/* P_R_P3Q2 */
	mt9t111_write(0x3710, 0xCA72);	/* P_R_P3Q3 */
	mt9t111_write(0x3712, 0x43F5);	/* P_R_P3Q4 */
	mt9t111_write(0x374A, 0x9F51);	/* P_R_P4Q0 */
	mt9t111_write(0x374C, 0x6772);	/* P_R_P4Q1 */
	mt9t111_write(0x374E, 0xC5D7);	/* P_R_P4Q2 */
	mt9t111_write(0x3750, 0xC615);	/* P_R_P4Q3 */
	mt9t111_write(0x3752, 0x08FA);	/* P_R_P4Q4 */
	mt9t111_write(0x3640, 0x07B0);	/* P_G1_P0Q0 */
	mt9t111_write(0x3642, 0x9A2D);	/* P_G1_P0Q1 */
	mt9t111_write(0x3644, 0x3EB1);	/* P_G1_P0Q2 */
	mt9t111_write(0x3646, 0x87EE);	/* P_G1_P0Q3 */
	mt9t111_write(0x3648, 0xF7F1);	/* P_G1_P0Q4 */
	mt9t111_write(0x3680, 0x3F6E);	/* P_G1_P1Q0 */
	mt9t111_write(0x3682, 0xA56E);	/* P_G1_P1Q1 */
	mt9t111_write(0x3684, 0x1C70);	/* P_G1_P1Q2 */
	mt9t111_write(0x3686, 0x156F);	/* P_G1_P1Q3 */
	mt9t111_write(0x3688, 0xCED0);	/* P_G1_P1Q4 */
	mt9t111_write(0x36C0, 0x2012);	/* P_G1_P2Q0 */
	mt9t111_write(0x36C2, 0xFF4F);	/* P_G1_P2Q1 */
	mt9t111_write(0x36C4, 0xB74B);	/* P_G1_P2Q2 */
	mt9t111_write(0x36C6, 0x110E);	/* P_G1_P2Q3 */
	mt9t111_write(0x36C8, 0xB635);	/* P_G1_P2Q4 */
	mt9t111_write(0x3700, 0x5F4F);	/* P_G1_P3Q0 */
	mt9t111_write(0x3702, 0x9231);	/* P_G1_P3Q1 */
	mt9t111_write(0x3704, 0x8CB1);	/* P_G1_P3Q2 */
	mt9t111_write(0x3706, 0x5B91);	/* P_G1_P3Q3 */
	mt9t111_write(0x3708, 0x60F4);	/* P_G1_P3Q4 */
	mt9t111_write(0x3740, 0x9F32);	/* P_G1_P4Q0 */
	mt9t111_write(0x3742, 0x2D91);	/* P_G1_P4Q1 */
	mt9t111_write(0x3744, 0x8AD7);	/* P_G1_P4Q2 */
	mt9t111_write(0x3746, 0xF0F4);	/* P_G1_P4Q3 */
	mt9t111_write(0x3748, 0x5BD9);	/* P_G1_P4Q4 */
	mt9t111_write(0x3654, 0x0230);	/* P_B_P0Q0 */
	mt9t111_write(0x3656, 0x8C4D);	/* P_B_P0Q1 */
	mt9t111_write(0x3658, 0x0BF1);	/* P_B_P0Q2 */
	mt9t111_write(0x365A, 0xB70E);	/* P_B_P0Q3 */
	mt9t111_write(0x365C, 0x82F1);	/* P_B_P0Q4 */
	mt9t111_write(0x3694, 0x366E);	/* P_B_P1Q0 */
	mt9t111_write(0x3696, 0xF06D);	/* P_B_P1Q1 */
	mt9t111_write(0x3698, 0x73CF);	/* P_B_P1Q2 */
	mt9t111_write(0x369A, 0x7BAE);	/* P_B_P1Q3 */
	mt9t111_write(0x369C, 0x89B0);	/* P_B_P1Q4 */
	mt9t111_write(0x36D4, 0x1632);	/* P_B_P2Q0 */
	mt9t111_write(0x36D6, 0xCFAF);	/* P_B_P2Q1 */
	mt9t111_write(0x36D8, 0x0B12);	/* P_B_P2Q2 */
	mt9t111_write(0x36DA, 0xCA8C);	/* P_B_P2Q3 */
	mt9t111_write(0x36DC, 0xE555);	/* P_B_P2Q4 */
	mt9t111_write(0x3714, 0x36AF);	/* P_B_P3Q0 */
	mt9t111_write(0x3716, 0xBE50);	/* P_B_P3Q1 */
	mt9t111_write(0x3718, 0xCE11);	/* P_B_P3Q2 */
	mt9t111_write(0x371A, 0xB830);	/* P_B_P3Q3 */
	mt9t111_write(0x371C, 0x6654);	/* P_B_P3Q4 */
	mt9t111_write(0x3754, 0x9232);	/* P_B_P4Q0 */
	mt9t111_write(0x3756, 0x0591);	/* P_B_P4Q1 */
	mt9t111_write(0x3758, 0x9017);	/* P_B_P4Q2 */
	mt9t111_write(0x375A, 0xEB74);	/* P_B_P4Q3 */
	mt9t111_write(0x375C, 0x5AD9);	/* P_B_P4Q4 */
	mt9t111_write(0x365E, 0x0250);	/* P_G2_P0Q0 */
	mt9t111_write(0x3660, 0xC80D);	/* P_G2_P0Q1 */
	mt9t111_write(0x3662, 0x3511);	/* P_G2_P0Q2 */
	mt9t111_write(0x3664, 0xC4AD);	/* P_G2_P0Q3 */
	mt9t111_write(0x3666, 0xEFD1);	/* P_G2_P0Q4 */
	mt9t111_write(0x369E, 0x52AE);	/* P_G2_P1Q0 */
	mt9t111_write(0x36A0, 0x4A2F);	/* P_G2_P1Q1 */
	mt9t111_write(0x36A2, 0x2B8F);	/* P_G2_P1Q2 */
	mt9t111_write(0x36A4, 0xC4F0);	/* P_G2_P1Q3 */
	mt9t111_write(0x36A6, 0xD9AF);	/* P_G2_P1Q4 */
	mt9t111_write(0x36DE, 0x2E72);	/* P_G2_P2Q0 */
	mt9t111_write(0x36E0, 0x8590);	/* P_G2_P2Q1 */
	mt9t111_write(0x36E2, 0x112C);	/* P_G2_P2Q2 */
	mt9t111_write(0x36E4, 0x0C30);	/* P_G2_P2Q3 */
	mt9t111_write(0x36E6, 0xBAF5);	/* P_G2_P2Q4 */
	mt9t111_write(0x371E, 0x2C10);	/* P_G2_P3Q0 */
	mt9t111_write(0x3720, 0x174F);	/* P_G2_P3Q1 */
	mt9t111_write(0x3722, 0xF1B2);	/* P_G2_P3Q2 */
	mt9t111_write(0x3724, 0x9853);	/* P_G2_P3Q3 */
	mt9t111_write(0x3726, 0x1E75);	/* P_G2_P3Q4 */
	mt9t111_write(0x375E, 0xC712);	/* P_G2_P4Q0 */
	mt9t111_write(0x3760, 0x34F2);	/* P_G2_P4Q1 */
	mt9t111_write(0x3762, 0x8AB7);	/* P_G2_P4Q2 */
	mt9t111_write(0x3764, 0xA015);	/* P_G2_P4Q3 */
	mt9t111_write(0x3766, 0x5979);	/* P_G2_P4Q4 */
	mt9t111_write(0x3784, 0x0410);	/* CENTER_COLUMN */
	mt9t111_write(0x3782, 0x0300);	/* CENTER_ROW */
	mt9t111_write(0x3210, 0x00B8);	/* COLOR_PIPELINE */

	/* [670nm awb&ccm] */
	mt9t111_write(0x098E, 0x48B0);	/* MCU_ADDRESS [CAM1_AWB_CCM_L_0] */
	mt9t111_write(0x0990, 0x01E1);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0x48B2);	/* MCU_ADDRESS [CAM1_AWB_CCM_L_1] */
	mt9t111_write(0x0990, 0xFEA8);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0x48B4);	/* MCU_ADDRESS [CAM1_AWB_CCM_L_2] */
	mt9t111_write(0x0990, 0x0076);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0x48B6);	/* MCU_ADDRESS [CAM1_AWB_CCM_L_3] */
	mt9t111_write(0x0990, 0xFFD8);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0x48B8);	/* MCU_ADDRESS [CAM1_AWB_CCM_L_4] */
	mt9t111_write(0x0990, 0x013F);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0x48BA);	/* MCU_ADDRESS [CAM1_AWB_CCM_L_5] */
	mt9t111_write(0x0990, 0xFFE7);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0x48BC);	/* MCU_ADDRESS [CAM1_AWB_CCM_L_6] */
	mt9t111_write(0x0990, 0xFFE7);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0x48BE);	/* MCU_ADDRESS [CAM1_AWB_CCM_L_7] */
	mt9t111_write(0x0990, 0xFF05);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0x48C0);	/* MCU_ADDRESS [CAM1_AWB_CCM_L_8] */
	mt9t111_write(0x0990, 0x0211);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0x48C2);	/* MCU_ADDRESS [CAM1_AWB_CCM_L_9] */
	mt9t111_write(0x0990, 0x0017);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0x48C4);	/* MCU_ADDRESS [CAM1_AWB_CCM_L_10] */
	mt9t111_write(0x0990, 0x003a);	/* MCU_DATA_0 */

	mt9t111_write(0x098E, 0x48C6);	/* MCU_ADDRESS [CAM1_AWB_CCM_RL_0] */
	mt9t111_write(0x0990, 0xFF13);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0x48C8);	/* MCU_ADDRESS [CAM1_AWB_CCM_RL_1] */
	mt9t111_write(0x0990, 0x00DA);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0x48CA);	/* MCU_ADDRESS [CAM1_AWB_CCM_RL_2] */
	mt9t111_write(0x0990, 0xFF94);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0x48CC);	/* MCU_ADDRESS [CAM1_AWB_CCM_RL_3] */
	mt9t111_write(0x0990, 0x0017);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0x48CE);	/* MCU_ADDRESS [CAM1_AWB_CCM_RL_4] */
	mt9t111_write(0x0990, 0xFF72);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0x48D0);	/* MCU_ADDRESS [CAM1_AWB_CCM_RL_5] */
	mt9t111_write(0x0990, 0xFFF9);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0x48D2);	/* MCU_ADDRESS [CAM1_AWB_CCM_RL_6] */
	mt9t111_write(0x0990, 0x001D);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0x48D4);	/* MCU_ADDRESS [CAM1_AWB_CCM_RL_7] */
	mt9t111_write(0x0990, 0x00A5);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0x48D6);	/* MCU_ADDRESS [CAM1_AWB_CCM_RL_8] */
	mt9t111_write(0x0990, 0xFEC1);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0x48D8);	/* MCU_ADDRESS [CAM1_AWB_CCM_RL_9] */
	mt9t111_write(0x0990, 0x001e);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0x48DA);	/* MCU_ADDRESS [CAM1_AWB_CCM_RL_10] */
	mt9t111_write(0x0990, 0xFFe6);	/* MCU_DATA_0 */

	/* [awb default 670nm] */
	mt9t111_write(0x098E, 0xC8F4);	/* MCU_ADDRESS [CAM1_AWB_AWB_XSCALE] */
	mt9t111_write(0x0990, 0x0004);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0xC8F5);	/* MCU_ADDRESS [CAM1_AWB_AWB_YSCALE] */
	mt9t111_write(0x0990, 0x0002);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0x48F6);	/* MCU_ADDRESS [CAM1_AWB_AWB_WEIGHTS_0] */
	mt9t111_write(0x0990, 0x8C7A);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0x48F8);	/* MCU_ADDRESS [CAM1_AWB_AWB_WEIGHTS_1] */
	mt9t111_write(0x0990, 0xAD29);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0x48FA);	/* MCU_ADDRESS [CAM1_AWB_AWB_WEIGHTS_2] */
	mt9t111_write(0x0990, 0x688C);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0x48FC);	/* MCU_ADDRESS [CAM1_AWB_AWB_WEIGHTS_3] */
	mt9t111_write(0x0990, 0x76EA);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0x48FE);	/* MCU_ADDRESS [CAM1_AWB_AWB_WEIGHTS_4] */
	mt9t111_write(0x0990, 0x7ED3);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0x4900);	/* MCU_ADDRESS [CAM1_AWB_AWB_WEIGHTS_5] */
	mt9t111_write(0x0990, 0x8913);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0x4902);	/* MCU_ADDRESS [CAM1_AWB_AWB_WEIGHTS_6] */
	mt9t111_write(0x0990, 0xBFFC);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0x4904);	/* MCU_ADDRESS [CAM1_AWB_AWB_WEIGHTS_7] */
	mt9t111_write(0x0990, 0x000F);	/* MCU_DATA_0 */
	/* MCU_ADDRESS [CAM1_AWB_AWB_XSHIFT_PRE_ADJ] */
	mt9t111_write(0x098E, 0x4906);
	mt9t111_write(0x0990, 0x0023);	/*0x0028 ); MCU_DATA_0 */
	/* MCU_ADDRESS [CAM1_AWB_AWB_YSHIFT_PRE_ADJ] */
	mt9t111_write(0x098E, 0x4908);
	mt9t111_write(0x0990, 0x002f);	/*0x0035 ); MCU_DATA_0 */
	mt9t111_write(0x098E, 0xC8F1);	/* MCU_ADDRESS [CAM1_AWB_DGAIN_MAX_B] */
	mt9t111_write(0x0990, 0x00AC);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0xB839);	/* MCU_ADDRESS [STAT_AE_SPOT_PERC] */
	mt9t111_write(0x0990, 0x0001);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0xC8EE);	/* MCU_ADDRESS [CAM1_AWB_DGAIN_MIN_R] */
	mt9t111_write(0x0990, 0x005F);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0xC8EF);	/* MCU_ADDRESS [CAM1_AWB_DGAIN_MAX_R] */
	mt9t111_write(0x0990, 0x00C0);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0xC8F0);	/* MCU_ADDRESS [CAM1_AWB_DGAIN_MIN_B] */
	mt9t111_write(0x0990, 0x0064);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0xAC33);	/* MCU_ADDRESS [AWB_CCMPOSITION] */
	mt9t111_write(0x0990, 0x003D);	/* MCU_DATA_0 */
	/* AWB GLOBAL GAIN */
	mt9t111_write(0x098E, 0xE84A);	/* MCU_ADDRESS [PRI_A_CONFIG_AWB_K_R_L] */
	mt9t111_write(0x0990, 0x0080);	/* 0x0074 );             0x0080 );
					   MCU_DATA_0 20090727 */
	mt9t111_write(0x098E, 0xE84C);	/* MCU_ADDRESS [PRI_A_CONFIG_AWB_K_B_L] */
	mt9t111_write(0x0990, 0x0095);	/* 0x0080 );             0x0084 );
					   MCU_DATA_0 20090727 */
	mt9t111_write(0x098E, 0xE84D);	/* MCU_ADDRESS [PRI_A_CONFIG_AWB_K_R_R] */
	mt9t111_write(0x0990, 0x007B);	/*0x0071 0x007e );
					   0x0080   71 ); MCU_DATA_0 20090806 */
	mt9t111_write(0x098E, 0xE84F);	/* MCU_ADDRESS [PRI_A_CONFIG_AWB_K_B_R] */
	mt9t111_write(0x0990, 0x0082);	/*0x008A 0x0084 );              0x007e );
					   0x0080 ); MCU_DATA_0 20090727 */
	mt9t111_write(0x098E, 0xAC38);	/* MCU_ADDRESS [AWB_INDOOR_R_MIN] */
	mt9t111_write(0x0990, 0x003b);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0xAC39);	/* MCU_ADDRESS [AWB_INDOOR_R_MAX] */
	mt9t111_write(0x0990, 0x007F);	/*0x0081 );              0x006B );
					   MCU_DATA_0 20090727 */
	mt9t111_write(0x098E, 0xAC3A);	/* MCU_ADDRESS [AWB_INDOOR_B_MIN] */
	mt9t111_write(0x0990, 0x0033);	/*0x0030 );              0x0039 );
					   MCU_DATA_0 20090727 */
	mt9t111_write(0x098E, 0xAC3B);	/* MCU_ADDRESS [AWB_INDOOR_B_MAX] */
	mt9t111_write(0x0990, 0x0072);	/*0x006d );              0x006C );
					   MCU_DATA_0 20090727 */
	/* MCU_ADDRESS [CAM1_STAT_LUMA_THRESH_LOW] */
	mt9t111_write(0x098E, 0xC910);
	mt9t111_write(0x0990, 0x0010);	/* MCU_DATA_0 */
	/* MCU_ADDRESS [CAM1_STAT_LUMA_THRESH_HIGH] */
	mt9t111_write(0x098E, 0xC911);
	mt9t111_write(0x0990, 0x00FC);	/* MCU_DATA_0 */
	mt9t111_write(0x3266, 0x00C4);	/* AWB_WEIGHT_TH */
	/* MCU_ADDRESS [PRI_A_CONFIG_AWB_ALGO_RUN] */
	mt9t111_write(0x098E, 0x683F);
	mt9t111_write(0x0990, 0x01F9);	/* MCU_DATA_0 LAST */
	mt9t111_write(0x098E, 0xE847);	/* MCU_ADDRESS [AWB_gain buffer speed] */
	mt9t111_write(0x0990, 0x0006);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0xE848);	/* MCU_ADDRESS [AWB_jump Divior] */
	mt9t111_write(0x0990, 0x0003);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0xAC02);	/* MCU_ADDRESS [AWB_MODE] */
	/*0x000A );              MCU_DATA_0 20090727 */
	mt9t111_write(0x0990, 0x0008);
	mt9t111_write(0x098E, 0xAC3C);	/* AWB_R_RATIO_PRE_AWB */
	mt9t111_write(0x0990, 0x003C);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0xAC3D);	/* AWB_B_RATIO_PRE_AWB */
	mt9t111_write(0x0990, 0x0050);	/* MCU_DATA_0 */
	/* MCU_ADDRESS [PRI_A_CONFIG_AWB_ALGO_LEAVE] */
	mt9t111_write(0x098E, 0x6841);
	mt9t111_write(0x0990, 0x0000);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0xC949);	/* MCU_ADDRESS [CAM1_SYS_DARK_COLOR_KILL] */
	mt9t111_write(0x0990, 0x0033);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0xC94A);	/* MCU_ADDRESS[CAM1_SYS_BRIGHT_COLORKILL] */
	mt9t111_write(0x0990, 0x0062);	/* MCU_DATA_0 */
	/* MCU_ADDRESS[AWB_PIXEL_THRESHOLD_COUNT_HI] */
	mt9t111_write(0x098E, 0x2C05);
	mt9t111_write(0x0990, 0x0000);	/* MCU_DATA_0 */
	/* MCU_ADDRESS [AWB_PIXEL_THRESHOLD_COUNT_LO] */
	mt9t111_write(0x098E, 0x2C07);
	mt9t111_write(0x0990, 0x03E8);	/* MCU_DATA_0 */
	/* MCU_ADDRESS [CAM1_AWB_CCMPOSITION_MIN] */
	mt9t111_write(0x098E, 0xC8F2);
	mt9t111_write(0x0990, 0x0000);	/* MCU_DATA_0 */
	/* MCU_ADDRESS [CAM1_AWB_CCMPOSITION_MAX] */
	mt9t111_write(0x098E, 0xC8F3);
	mt9t111_write(0x0990, 0x007F);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0xAC34);	/* MCU_ADDRESS */
	mt9t111_write(0x0990, 0x005D);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0xAC35);	/* MCU_ADDRESS */
	mt9t111_write(0x0990, 0x0064);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0xAC36);	/* MCU_ADDRESS */
	mt9t111_write(0x0990, 0x0062);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0xAC37);	/* MCU_ADDRESS */
	mt9t111_write(0x0990, 0x0068);	/* MCU_DATA_0 */
	/* MCU_ADDRESS [CAM1_STAT_BRIGHTNESS_METRIC_PREDIVIDER] */
	mt9t111_write(0x098E, 0xC913);
	mt9t111_write(0x0990, 0x000C);	/* MCU_DATA_0 */
	/* MCU_ADDRESS [PRI_A_CONFIG_LL_START_BRIGHTNESS] */
	mt9t111_write(0x098E, 0x686B);
	mt9t111_write(0x0990, 0x0570);	/* MCU_DATA_0 */
	/* MCU_ADDRESS [PRI_A_CONFIG_LL_STOP_BRIGHTNESS] */
	mt9t111_write(0x098E, 0x686D);
	mt9t111_write(0x0990, 0x0580);	/* MCU_DATA_0 */
	/* MCU_ADDRESS [PRI_B_CONFIG_LL_START_BRIGHTNESS] */
	mt9t111_write(0x098E, 0x6C6B);
	mt9t111_write(0x0990, 0x0570);	/* MCU_DATA_0 */
	/* MCU_ADDRESS [PRI_B_CONFIG_LL_STOP_BRIGHTNESS] */
	mt9t111_write(0x098E, 0x6C6D);
	mt9t111_write(0x0990, 0x0580);	/* MCU_DATA_0 */
	/* MCU_ADDRESS [PRI_B_CONFIG_AE_TRACK_ALGO_ENTER] */
	mt9t111_write(0x098E, 0x6C25);
	mt9t111_write(0x0990, 0x0008);	/* MCU_DATA_0 */
	/* MCU_ADDRESS [PRI_B_CONFIG_AE_TRACK_ALGO_RUN] */
	mt9t111_write(0x098E, 0x6C27);
	mt9t111_write(0x0990, 0x0000);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0xC91C);	/* MCU_ADDRESS [CAM1_LL_LL_RESERVED_1] */
	mt9t111_write(0x0990, 0x0096);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0xC91D);	/* MCU_ADDRESS [CAM1_LL_LL_RESERVED_2] */
	mt9t111_write(0x0990, 0x00FF);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0xC91E);	/* MCU_ADDRESS [CAM1_LL_NR_START_0] */
	mt9t111_write(0x0990, 0x0001);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0xC91F);	/* MCU_ADDRESS [CAM1_LL_NR_START_1] */
	mt9t111_write(0x0990, 0x0001);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0xC920);	/* MCU_ADDRESS [CAM1_LL_NR_START_2] */
	mt9t111_write(0x0990, 0x0001);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0xC921);	/* MCU_ADDRESS [CAM1_LL_NR_START_3] */
	mt9t111_write(0x0990, 0x0001);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0xC922);	/* MCU_ADDRESS [CAM1_LL_NR_STOP_0] */
	mt9t111_write(0x0990, 0x0020);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0xC923);	/* MCU_ADDRESS [CAM1_LL_NR_STOP_1] */
	mt9t111_write(0x0990, 0x0018);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0xC924);	/* MCU_ADDRESS [CAM1_LL_NR_STOP_2] */
	mt9t111_write(0x0990, 0x0018);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0xC925);	/* MCU_ADDRESS [CAM1_LL_NR_STOP_3] */
	mt9t111_write(0x0990, 0x0020);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0xC916);	/* MCU_ADDRESS [CAM1_LL_LL_START_0] */
	mt9t111_write(0x0990, 0x0001);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0xC917);	/* MCU_ADDRESS [CAM1_LL_LL_START_1] */
	mt9t111_write(0x0990, 0x0005);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0xC918);	/* MCU_ADDRESS [CAM1_LL_LL_START_2] */
	mt9t111_write(0x0990, 0x0003);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0xC919);	/* MCU_ADDRESS [CAM1_LL_LL_STOP_0] */
	mt9t111_write(0x0990, 0x0020);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0xC91A);	/* MCU_ADDRESS [CAM1_LL_LL_STOP_1] */
	mt9t111_write(0x0990, 0x0001);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0xC91B);	/* MCU_ADDRESS [CAM1_LL_LL_STOP_2] */
	mt9t111_write(0x0990, 0x0009);	/* MCU_DATA_0 */
	mt9t111_write(0x326C, 0x1403);	/* APERTURE_PARAMETERS_2D */
	mt9t111_write(0x098E, 0xBC02);	/* MCU_ADDRESS [LL_MODE] */
	mt9t111_write(0x0990, 0x0003);	/* MCU_DATA_0  0x0000 -> 0x0003 20090625 */
	mt9t111_write(0x098E, 0xBC05);	/* MCU_ADDRESS [LL_CLUSTER_DC_TH] */
	mt9t111_write(0x0990, 0x000E);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0x494B);	/* MCU_ADDRESS [CAM1_LL_EXT_START_GAIN_METRIC]
					   Gain Metric Start for Noise Reduction */
	mt9t111_write(0x0990, 0x0020);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0x494D);	/* MCU_ADDRESS [CAM1_LL_EXT_STOP_GAIN_METRIC]
					   Gain Metric Stop for Noise Reduction */
	mt9t111_write(0x0990, 0x00F0);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0xDC2A);	/* MCU_ADDRESS [SYS_DELTA_GAIN] */
	mt9t111_write(0x0990, 0x001F);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0xDC2B);	/* MCU_ADDRESS [SYS_DELTA_THRESH] */
	mt9t111_write(0x0990, 0x0012);	/* MCU_DATA_0 */
	/* MCU_ADDRESS [CAM1_LL_START_DESATURATION] */
	mt9t111_write(0x098E, 0xC914);
	mt9t111_write(0x0990, 0x0000);	/* MCU_DATA_0 */
	/* MCU_ADDRESS [CAM1_LL_END_DESATURATION] */
	mt9t111_write(0x098E, 0xC915);
	mt9t111_write(0x0990, 0x00FF);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0x48DC);	/* MCU_ADDRESS [CAM1_AWB_LL_CCM_0]     */
	mt9t111_write(0x0990, 0x004D);	/* MCU_DATA_0                          */
	mt9t111_write(0x098E, 0x48DE);	/* MCU_ADDRESS [CAM1_AWB_LL_CCM_1]     */
	mt9t111_write(0x0990, 0x0096);	/* MCU_DATA_0                          */
	mt9t111_write(0x098E, 0x48E0);	/* MCU_ADDRESS [CAM1_AWB_LL_CCM_2]     */
	mt9t111_write(0x0990, 0x001D);	/* MCU_DATA_0                          */
	mt9t111_write(0x098E, 0x48E2);	/* MCU_ADDRESS [CAM1_AWB_LL_CCM_3]     */
	mt9t111_write(0x0990, 0x004D);	/* MCU_DATA_0                          */
	mt9t111_write(0x098E, 0x48E4);	/* MCU_ADDRESS [CAM1_AWB_LL_CCM_4]     */
	mt9t111_write(0x0990, 0x0096);	/* MCU_DATA_0                          */
	mt9t111_write(0x098E, 0x48E6);	/* MCU_ADDRESS [CAM1_AWB_LL_CCM_5]     */
	mt9t111_write(0x0990, 0x001D);	/* MCU_DATA_0                          */
	mt9t111_write(0x098E, 0x48E8);	/* MCU_ADDRESS [CAM1_AWB_LL_CCM_6]     */
	mt9t111_write(0x0990, 0x004D);	/* MCU_DATA_0                          */
	mt9t111_write(0x098E, 0x48EA);	/* MCU_ADDRESS [CAM1_AWB_LL_CCM_7]     */
	mt9t111_write(0x0990, 0x0096);	/* MCU_DATA_0                          */
	mt9t111_write(0x098E, 0x48EC);	/* MCU_ADDRESS [CAM1_AWB_LL_CCM_8] */
	mt9t111_write(0x0990, 0x001D);	/* MCU_DATA_0 */
	/* MCU_ADDRESS [PRI_A_CONFIG_AE_RULE_EXP_COMP_CENTER] */
	mt9t111_write(0x098E, 0x6821);
	mt9t111_write(0x0990, 0x1B58);	/* MCU_DATA_0 */
	/* MCU_ADDRESS [PRI_A_CONFIG_AE_RULE_EXP_COMP_LOW_LIGHT] */
	mt9t111_write(0x098E, 0xE823);
	mt9t111_write(0x0990, 0x0080);	/* MCU_DATA_0 */
	/* MCU_ADDRESS [PRI_A_CONFIG_AE_RULE_EXP_COMP_BRIGHT_LIGHT] */
	mt9t111_write(0x098E, 0xE824);
	mt9t111_write(0x0990, 0x0080);	/* MCU_DATA_0 */
	/* MCU_ADDRESS [PRI_B_CONFIG_AE_RULE_EXP_COMP_CENTER] */
	mt9t111_write(0x098E, 0x6C21);
	mt9t111_write(0x0990, 0x1B58);	/* MCU_DATA_0 */
	/* MCU_ADDRESS [PRI_B_CONFIG_AE_RULE_EXP_COMP_LOW_LIGHT] */
	mt9t111_write(0x098E, 0xEC23);
	mt9t111_write(0x0990, 0x0080);	/* MCU_DATA_0 */
	/* MCU_ADDRESS [PRI_B_CONFIG_AE_RULE_EXP_COMP_BRIGHT_LIGHT] */
	mt9t111_write(0x098E, 0xEC24);
	mt9t111_write(0x0990, 0x0080);	/* MCU_DATA_0 */
	/* MCU_ADDRESS [AE_RULE_OFFSET_DAMPENING] */
	mt9t111_write(0x098E, 0xA415);
	mt9t111_write(0x0990, 0x0004);	/* MCU_DATA_0 */
	/* MCU_ADDRESS [PRI_A_CONFIG_AE_TRACK_SKIP_FRAMES] */
	mt9t111_write(0x098E, 0xE82B);
	mt9t111_write(0x0990, 0x0002);	/* MCU_DATA_0 */
	/* MCU_ADDRESS [PRI_A_CONFIG_AE_RULE_BASE_TARGET] */
	mt9t111_write(0x098E, 0xE81F);
	mt9t111_write(0x0990, 0x0022);	/*0x0045   0x0040 );
					   MCU_DATA_0   20090715 */
	/* MCU_ADDRESS [PRI_B_CONFIG_AE_RULE_BASE_TARGET] */
	mt9t111_write(0x098E, 0xEC1F);
	mt9t111_write(0x0990, 0x0040);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0xA80E);	/* MCU_ADDRESS [AE_TRACK_GATE] */
	mt9t111_write(0x0990, 0x0006);	/* MCU_DATA_0  6 */
	/* MCU_ADDRESS [AE_TRACK_MAX_BLACK_LEVEL] */
	mt9t111_write(0x098E, 0xA807);
	mt9t111_write(0x0990, 0x003C);	/* MCU_DATA_0 */
	/* MCU_ADDRESS [PRI_A_CONFIG_FD_MAX_FDZONE_50HZ] */
	mt9t111_write(0x098E, 0x6815);
	mt9t111_write(0x0990, 0x0004);	/* MCU_DATA_0 */
	/* MCU_ADDRESS [PRI_A_CONFIG_FD_MAX_FDZONE_60HZ] */
	mt9t111_write(0x098E, 0x6817);
	mt9t111_write(0x0990, 0x0004);	/* MCU_DATA_0 */
	/* MCU_ADDRESS [PRI_A_CONFIG_AE_TRACK_TARGET_FDZONE] */
	mt9t111_write(0x098E, 0x682D);
	mt9t111_write(0x0990, 0x0006);	/* MCU_DATA_0 6 */
	/* MCU_ADDRESS [PRI_A_CONFIG_AE_TRACK_TARGET_AGAIN] */
	mt9t111_write(0x098E, 0x682F);
	mt9t111_write(0x0990, 0x0100);	/* MCU_DATA_0 */
	/* MCU_ADDRESS [PRI_B_CONFIG_AE_TRACK_TARGET_AGAIN] */
	mt9t111_write(0x098E, 0x6C2F);
	mt9t111_write(0x0990, 0x0100);	/* MCU_DATA_0 */
	/* MCU_ADDRESS [PRI_A_CONFIG_AE_TRACK_AE_MIN_VIRT_INT_TIME_PCLK] */
	mt9t111_write(0x098E, 0x6831);
	mt9t111_write(0x0990, 0x0020);	/* MCU_DATA_0 */
	/* MCU_ADDRESS [PRI_A_CONFIG_AE_TRACK_AE_MIN_VIRT_DGAIN] */
	mt9t111_write(0x098E, 0x6833);
	mt9t111_write(0x0990, 0x0080);	/* MCU_DATA_0 */
	/* MCU_ADDRESS [PRI_A_CONFIG_AE_TRACK_AE_MAX_VIRT_DGAIN] */
	mt9t111_write(0x098E, 0x6835);
	mt9t111_write(0x0990, 0x01A0);	/*0x0120 );              0x00D9 );
					   MCU_DATA_0 LOW LIGHT 20090727 */
	/* MCU_ADDRESS [PRI_B_CONFIG_AE_TRACK_AE_MAX_VIRT_DGAIN] */
	mt9t111_write(0x098E, 0x6C35);
	mt9t111_write(0x0990, 0x00d0);	/* 0x00A0 );
					   MCU_DATA_0 low light 20090727 */
	/* MCU_ADDRESS [PRI_A_CONFIG_AE_TRACK_AE_MAX_VIRT_AGAIN] */
	mt9t111_write(0x098E, 0x6839);
	mt9t111_write(0x0990, 0x012C);	/* MCU_DATA_0 */
	/* MCU_ADDRESS [PRI_A_CONFIG_AE_TRACK_AE_MIN_VIRT_AGAIN] */
	mt9t111_write(0x098E, 0x6837);
	mt9t111_write(0x0990, 0x0040);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0x4815);	/* MCU_ADDRESS [CAM1_CTX_A_COARSE_ITMIN] */
	mt9t111_write(0x0990, 0x0001);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0x485D);	/* MCU_ADDRESS [CAM1_CTX_B_COARSE_ITMIN] */
	mt9t111_write(0x0990, 0x0001);	/* MCU_DATA_0 */
	/* MCU_ADDRESS [PRI_A_CONFIG_AE_RULE_ALGO_RUN] */
	mt9t111_write(0x098E, 0x681B);
	mt9t111_write(0x0990, 0x0004);	/* MCU_DATA_0 */
	/* MCU_ADDRESS [PRI_A_CONFIG_AE_TRACK_JUMP_DIVISOR] */
	mt9t111_write(0x098E, 0xE82C);
	mt9t111_write(0x0990, 0x0001);	/* MCU_DATA_0 */

	/*v1.4 gamma 0.40 ,black level :4 */
	mt9t111_write(0x098E, 0xBC0B);	/*MCU_ADDRESS [LL_GAMMA_CONTRAST_CURVE_0] */
	mt9t111_write(0x0990, 0x0000);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0xBC0C);	/* MCU_ADDRESS [LL_GAMMA_CONTRAST_CURVE_1] */
	mt9t111_write(0x0990, 0x0012);	/*MCU_DATA_0 */
	mt9t111_write(0x098E, 0xBC0D);	/*MCU_ADDRESS [LL_GAMMA_CONTRAST_CURVE_2] */
	mt9t111_write(0x0990, 0x002C);	/*MCU_DATA_0 */
	mt9t111_write(0x098E, 0xBC0E);	/*MCU_ADDRESS [LL_GAMMA_CONTRAST_CURVE_3] */
	mt9t111_write(0x0990, 0x0047);	/*MCU_DATA_0 */
	mt9t111_write(0x098E, 0xBC0F);	/*MCU_ADDRESS [LL_GAMMA_CONTRAST_CURVE_4] */
	mt9t111_write(0x0990, 0x0067);	/*MCU_DATA_0 */
	mt9t111_write(0x098E, 0xBC10);	/*MCU_ADDRESS [LL_GAMMA_CONTRAST_CURVE_5] */
	mt9t111_write(0x0990, 0x007F);	/*MCU_DATA_0 */
	mt9t111_write(0x098E, 0xBC11);	/*MCU_ADDRESS [LL_GAMMA_CONTRAST_CURVE_6] */
	mt9t111_write(0x0990, 0x0092);	/*MCU_DATA_0 */
	mt9t111_write(0x098E, 0xBC12);	/*MCU_ADDRESS [LL_GAMMA_CONTRAST_CURVE_7] */
	mt9t111_write(0x0990, 0x00A1);	/*MCU_DATA_0 */
	mt9t111_write(0x098E, 0xBC13);	/*MCU_ADDRESS [LL_GAMMA_CONTRAST_CURVE_8] */
	mt9t111_write(0x0990, 0x00AE);	/*MCU_DATA_0 */
	mt9t111_write(0x098E, 0xBC14);	/*MCU_ADDRESS [LL_GAMMA_CONTRAST_CURVE_9] */
	mt9t111_write(0x0990, 0x00BA);	/*MCU_DATA_0 */
	mt9t111_write(0x098E, 0xBC15);	/*MCU_ADDRESS [LL_GAMMA_CONTRAST_CURVE_10] */
	mt9t111_write(0x0990, 0x00C4);	/*MCU_DATA_0 */
	mt9t111_write(0x098E, 0xBC16);	/*MCU_ADDRESS [LL_GAMMA_CONTRAST_CURVE_11] */
	mt9t111_write(0x0990, 0x00CE);	/*MCU_DATA_0 */
	mt9t111_write(0x098E, 0xBC17);	/*MCU_ADDRESS [LL_GAMMA_CONTRAST_CURVE_12] */
	mt9t111_write(0x0990, 0x00D6);	/*MCU_DATA_0 */
	mt9t111_write(0x098E, 0xBC18);	/*MCU_ADDRESS [LL_GAMMA_CONTRAST_CURVE_13] */
	mt9t111_write(0x0990, 0x00DE);	/*MCU_DATA_0 */
	mt9t111_write(0x098E, 0xBC19);	/*MCU_ADDRESS [LL_GAMMA_CONTRAST_CURVE_14] */
	mt9t111_write(0x0990, 0x00E6);	/*MCU_DATA_0 */
	mt9t111_write(0x098E, 0xBC1A);	/*MCU_ADDRESS [LL_GAMMA_CONTRAST_CURVE_15] */
	mt9t111_write(0x0990, 0x00ED);	/*MCU_DATA_0 */
	mt9t111_write(0x098E, 0xBC1B);	/*MCU_ADDRESS [LL_GAMMA_CONTRAST_CURVE_16] */
	mt9t111_write(0x0990, 0x00F3);	/*MCU_DATA_0 */
	mt9t111_write(0x098E, 0xBC1C);	/*MCU_ADDRESS [LL_GAMMA_CONTRAST_CURVE_17] */
	mt9t111_write(0x0990, 0x00F9);	/*MCU_DATA_0 */
	mt9t111_write(0x098E, 0xBC1D);	/*MCU_ADDRESS [LL_GAMMA_CONTRAST_CURVE_18] */
	mt9t111_write(0x0990, 0x00FF);	/*MCU_DATA_0 */
	mt9t111_write(0x098E, 0xBC1E);	/*MCU_ADDRESS [LL_GAMMA_NEUTRAL_CURVE_0] */
	mt9t111_write(0x0990, 0x0000);	/*MCU_DATA_0 */
	mt9t111_write(0x098E, 0xBC1F);	/*MCU_ADDRESS [LL_GAMMA_NEUTRAL_CURVE_1] */
	mt9t111_write(0x0990, 0x0012);	/*MCU_DATA_0 */
	mt9t111_write(0x098E, 0xBC20);	/*MCU_ADDRESS [LL_GAMMA_NEUTRAL_CURVE_2] */
	mt9t111_write(0x0990, 0x002C);	/*MCU_DATA_0 */
	mt9t111_write(0x098E, 0xBC21);	/*MCU_ADDRESS [LL_GAMMA_NEUTRAL_CURVE_3] */
	mt9t111_write(0x0990, 0x0047);	/*MCU_DATA_0 */
	mt9t111_write(0x098E, 0xBC22);	/*MCU_ADDRESS [LL_GAMMA_NEUTRAL_CURVE_4] */
	mt9t111_write(0x0990, 0x0067);	/*MCU_DATA_0 */
	mt9t111_write(0x098E, 0xBC23);	/*MCU_ADDRESS [LL_GAMMA_NEUTRAL_CURVE_5] */
	mt9t111_write(0x0990, 0x007F);	/*MCU_DATA_0 */
	mt9t111_write(0x098E, 0xBC24);	/*MCU_ADDRESS [LL_GAMMA_NEUTRAL_CURVE_6] */
	mt9t111_write(0x0990, 0x0092);	/*MCU_DATA_0 */
	mt9t111_write(0x098E, 0xBC25);	/*MCU_ADDRESS [LL_GAMMA_NEUTRAL_CURVE_7] */
	mt9t111_write(0x0990, 0x00A1);	/*MCU_DATA_0 */
	mt9t111_write(0x098E, 0xBC26);	/*MCU_ADDRESS [LL_GAMMA_NEUTRAL_CURVE_8] */
	mt9t111_write(0x0990, 0x00AE);	/*MCU_DATA_0 */
	mt9t111_write(0x098E, 0xBC27);	/*MCU_ADDRESS [LL_GAMMA_NEUTRAL_CURVE_9] */
	mt9t111_write(0x0990, 0x00BA);	/*MCU_DATA_0 */
	mt9t111_write(0x098E, 0xBC28);	/*MCU_ADDRESS [LL_GAMMA_NEUTRAL_CURVE_10] */
	mt9t111_write(0x0990, 0x00C4);	/*MCU_DATA_0 */
	mt9t111_write(0x098E, 0xBC29);	/*MCU_ADDRESS [LL_GAMMA_NEUTRAL_CURVE_11] */
	mt9t111_write(0x0990, 0x00CE);	/*MCU_DATA_0 */
	mt9t111_write(0x098E, 0xBC2A);	/*MCU_ADDRESS [LL_GAMMA_NEUTRAL_CURVE_12] */
	mt9t111_write(0x0990, 0x00D6);	/*MCU_DATA_0 */
	mt9t111_write(0x098E, 0xBC2B);	/*MCU_ADDRESS [LL_GAMMA_NEUTRAL_CURVE_13] */
	mt9t111_write(0x0990, 0x00DE);	/*MCU_DATA_0 */
	mt9t111_write(0x098E, 0xBC2C);	/*MCU_ADDRESS [LL_GAMMA_NEUTRAL_CURVE_14] */
	mt9t111_write(0x0990, 0x00E6);	/*MCU_DATA_0 */
	mt9t111_write(0x098E, 0xBC2D);	/*MCU_ADDRESS [LL_GAMMA_NEUTRAL_CURVE_15] */
	mt9t111_write(0x0990, 0x00ED);	/*MCU_DATA_0 */
	mt9t111_write(0x098E, 0xBC2E);	/*MCU_ADDRESS [LL_GAMMA_NEUTRAL_CURVE_16] */
	mt9t111_write(0x0990, 0x00F3);	/*MCU_DATA_0 */
	mt9t111_write(0x098E, 0xBC2F);	/*MCU_ADDRESS [LL_GAMMA_NEUTRAL_CURVE_17] */
	mt9t111_write(0x0990, 0x00F9);	/*MCU_DATA_0 */
	mt9t111_write(0x098E, 0xBC30);	/*MCU_ADDRESS [LL_GAMMA_NEUTRAL_CURVE_18] */
	mt9t111_write(0x0990, 0x00FF);	/*MCU_DATA_0 */
	/*----------------------------------------------------------- */
	mt9t111_write(0x098E, 0xBC31);	/*MCU_ADDRESS [LL_GAMMA_NRCURVE_0] */
	mt9t111_write(0x0990, 0x0000);	/*MCU_DATA_0 */
	mt9t111_write(0x098E, 0xBC32);	/*MCU_ADDRESS [LL_GAMMA_NRCURVE_1] */
	mt9t111_write(0x0990, 0x000C);	/*MCU_DATA_0 */
	mt9t111_write(0x098E, 0xBC33);	/*MCU_ADDRESS [LL_GAMMA_NRCURVE_2] */
	mt9t111_write(0x0990, 0x001B);	/*MCU_DATA_0 */
	mt9t111_write(0x098E, 0xBC34);	/*MCU_ADDRESS [LL_GAMMA_NRCURVE_3] */
	mt9t111_write(0x0990, 0x002F);	/*MCU_DATA_0 */
	mt9t111_write(0x098E, 0xBC35);	/*MCU_ADDRESS [LL_GAMMA_NRCURVE_4] */
	mt9t111_write(0x0990, 0x004B);	/*MCU_DATA_0 */
	mt9t111_write(0x098E, 0xBC36);	/*MCU_ADDRESS [LL_GAMMA_NRCURVE_5] */
	mt9t111_write(0x0990, 0x0060);	/*MCU_DATA_0 */
	mt9t111_write(0x098E, 0xBC37);	/*MCU_ADDRESS [LL_GAMMA_NRCURVE_6] */
	mt9t111_write(0x0990, 0x0074);	/*MCU_DATA_0 */
	mt9t111_write(0x098E, 0xBC38);	/*MCU_ADDRESS [LL_GAMMA_NRCURVE_7] */
	mt9t111_write(0x0990, 0x0085);	/*MCU_DATA_0 */
	mt9t111_write(0x098E, 0xBC39);	/*MCU_ADDRESS [LL_GAMMA_NRCURVE_8] */
	mt9t111_write(0x0990, 0x0095);	/*MCU_DATA_0 */
	mt9t111_write(0x098E, 0xBC3A);	/*MCU_ADDRESS [LL_GAMMA_NRCURVE_9] */
	mt9t111_write(0x0990, 0x00A3);	/*MCU_DATA_0 */
	mt9t111_write(0x098E, 0xBC3B);	/*MCU_ADDRESS [LL_GAMMA_NRCURVE_10] */
	mt9t111_write(0x0990, 0x00B0);	/*MCU_DATA_0 */
	mt9t111_write(0x098E, 0xBC3C);	/*MCU_ADDRESS [LL_GAMMA_NRCURVE_11] */
	mt9t111_write(0x0990, 0x00BC);	/*MCU_DATA_0 */
	mt9t111_write(0x098E, 0xBC3D);	/*MCU_ADDRESS [LL_GAMMA_NRCURVE_12] */
	mt9t111_write(0x0990, 0x00C7);	/*MCU_DATA_0 */
	mt9t111_write(0x098E, 0xBC3E);	/*MCU_ADDRESS [LL_GAMMA_NRCURVE_13] */
	mt9t111_write(0x0990, 0x00D2);	/*MCU_DATA_0 */
	mt9t111_write(0x098E, 0xBC3F);	/*MCU_ADDRESS [LL_GAMMA_NRCURVE_14] */
	mt9t111_write(0x0990, 0x00DC);	/*MCU_DATA_0 */
	mt9t111_write(0x098E, 0xBC40);	/*MCU_ADDRESS [LL_GAMMA_NRCURVE_15] */
	mt9t111_write(0x0990, 0x00E5);	/*MCU_DATA_0 */
	mt9t111_write(0x098E, 0xBC41);	/*MCU_ADDRESS [LL_GAMMA_NRCURVE_16] */
	mt9t111_write(0x0990, 0x00EE);	/*MCU_DATA_0 */
	mt9t111_write(0x098E, 0xBC42);	/*MCU_ADDRESS [LL_GAMMA_NRCURVE_17] */
	mt9t111_write(0x0990, 0x00F7);	/*MCU_DATA_0 */
	mt9t111_write(0x098E, 0xBC43);	/*MCU_ADDRESS [LL_GAMMA_NRCURVE_18] */
	mt9t111_write(0x0990, 0x00FF);	/*MCU_DATA_0 */
	mt9t111_write(0x098E, 0x4926);	/*MCU_ADDRESS [CAM1_LL_START_GAMMA_BM] */
	mt9t111_write(0x0990, 0x0141);	/*MCU_DATA_0 */
	mt9t111_write(0x098E, 0x4928);	/*MCU_ADDRESS [CAM1_LL_MID_GAMMA_BM] */
	mt9t111_write(0x0990, 0x0485);	/*MCU_DATA_0 */
	mt9t111_write(0x098E, 0x492A);	/*MCU_ADDRESS [CAM1_LL_STOP_GAMMA_BM] */
	mt9t111_write(0x0990, 0x090A);	/*MCU_DATA_0 */
	mt9t111_write(0x098E, 0xBC44);	/*MCU_ADDRESS [LL_TCLIMIT] */
	mt9t111_write(0x0990, 0x0028);	/*MCU_DATA_0 */
	mt9t111_write(0x098E, 0xBC45);	/*MCU_ADDRESS [LL_TCBASE] */
	mt9t111_write(0x0990, 0x0028);	/*MCU_DATA_0 */
	mt9t111_write(0x098E, 0x3C46);	/*MCU_ADDRESS [LL_START_TCAUTO_LIMITER] */
	mt9t111_write(0x0990, 0x0012);	/*MCU_DATA_0 */
	mt9t111_write(0x098E, 0x3C48);	/*MCU_ADDRESS [LL_STOP_TCAUTO_LIMITER] */
	mt9t111_write(0x0990, 0x01A4);	/*MCU_DATA_0 */
	mt9t111_write(0x326E, 0x00A4);	/*LOW_PASS_YUV_FILTER */
	mt9t111_write(0x098E, 0x3C4D);	/*MCU_ADDRESS [LL_START_GAMMA_FTB] */
	mt9t111_write(0x0990, 0xFFFF);	/*MCU_DATA_0 */
	mt9t111_write(0x098E, 0x3C4F);	/*MCU_ADDRESS [LL_STOP_GAMMA_FTB] */
	mt9t111_write(0x0990, 0xFFFF);	/*MCU_DATA_0 */
	mt9t111_write(0x098E, 0xB839);	/*MCU_ADDRESS [STAT_AE_SPOT_PERC] */
	mt9t111_write(0x0990, 0x0001);	/*MCU_DATA_0 */
	/*MCU_ADDRESS [PRI_A_CONFIG_LL_START_SATURATION] */
	mt9t111_write(0x098E, 0xE86F);
	mt9t111_write(0x0990, 0x0060);	/*0x0070 );
					   MCU_DATA_0 20090806 modify */
	/*MCU_ADDRESS [PRI_A_CONFIG_LL_END_SATURATION] */
	mt9t111_write(0x098E, 0xE870);
	mt9t111_write(0x0990, 0x0000);	/*MCU_DATA_0 */
	/*MCU_ADDRESS [PRI_B_CONFIG_LL_START_SATURATION] */
	mt9t111_write(0x098E, 0xEC6F);
	mt9t111_write(0x0990, 0x0060);	/*0x0070 );
					   MCU_DATA_0 20090806 modify */
	/*MCU_ADDRESS [PRI_B_CONFIG_LL_END_SATURATION] */
	mt9t111_write(0x098E, 0xEC70);
	mt9t111_write(0x0990, 0x0000);	/*MCU_DATA_0 */
	mt9t111_write(0x098E, 0x5C28);	/*MCU_ADDRESS [SYS_28] */
	mt9t111_write(0x0990, 0x2000);	/*MCU_DATA_0 */
	mt9t111_write(0x098E, 0x3005);	/*MCU_ADDRESS [AF_W11X_START] */
	mt9t111_write(0x0990, 0x0161);	/*0x00ED );
					   MCU_DATA_0 20090727 */
	mt9t111_write(0x098E, 0x3007);	/*MCU_ADDRESS [AF_W11Y_START] */
	mt9t111_write(0x0990, 0x010F);	/*0x00C0 );
					   MCU_DATA_0 20090727 */
	mt9t111_write(0x098E, 0x3009);	/*MCU_ADDRESS [AF_ZONE_WIDTH_VAR] */
	mt9t111_write(0x0990, 0x0054);	/*0x0089 );
					   MCU_DATA_0 20090727 */
	mt9t111_write(0x098E, 0x300B);	/*MCU_ADDRESS [AF_ZONE_HEIGHT_VAR] */
	mt9t111_write(0x0990, 0x0041);	/*0x0060 );
					   MCU_DATA_0 20090727 */
	mt9t111_write(0x098E, 0x4403);	/*MCU_ADDRESS [AFM_ALGO] */
	mt9t111_write(0x0990, 0x8001);	/*MCU_DATA_0 */
	mt9t111_write(0x098E, 0x4411);	/* MCU_ADDRESS [AFM_TIMER_VMT] */
	mt9t111_write(0x0990, 0xF41C);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0xC421);	/* MCU_ADDRESS [AFM_SI_SLAVE_ADDR] */
	mt9t111_write(0x0990, 0x0018);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0x3003);	/* MCU_ADDRESS [AF_ALGO] */
	mt9t111_write(0x0990, 0x0002);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0xB002);	/* MCU_ADDRESS [AF_MODE] */
	mt9t111_write(0x0990, 0x0028);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0x3020);	/* MCU_ADDRESS [AF_ZONE_WEIGHTS_HI] */
	mt9t111_write(0x0990, 0x697D);	/*0x557D );
					   MCU_DATA_0 20090727 */
	mt9t111_write(0x098E, 0x3022);	/* MCU_ADDRESS [AF_ZONE_WEIGHTS_LO] */
	mt9t111_write(0x0990, 0x7D55);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0x3005);	/* MCU_ADDRESS [AF_W11X_START] */
	mt9t111_write(0x0990, 0x0185);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0x3007);	/* MCU_ADDRESS [AF_W11Y_START] */
	mt9t111_write(0x0990, 0x0126);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0x3009);	/* MCU_ADDRESS [AF_ZONE_WIDTH_VAR] */
	mt9t111_write(0x0990, 0x0041);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0x300B);	/* MCU_ADDRESS [AF_ZONE_HEIGHT_VAR] */
	mt9t111_write(0x0990, 0x0033);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0x3017);	/* MCU_ADDRESS [AF_THRESHOLDS] */
	mt9t111_write(0x0990, 0x2020);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0xB01D);	/* MCU_ADDRESS [AF_NUM_STEPS] */
	mt9t111_write(0x0990, 0x000C);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0xB01F);	/* MCU_ADDRESS [AF_STEP_SIZE] */
	mt9t111_write(0x0990, 0x000F);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0x440B);	/* MCU_ADDRESS [AFM_POS_MIN] */
	mt9t111_write(0x0990, 0x0000);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0x440D);	/* MCU_ADDRESS [AFM_POS_MAX] */
	mt9t111_write(0x0990, 0x03C0);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0xB01C);	/* MCU_ADDRESS [AF_INIT_POS] */
	mt9t111_write(0x0990, 0x0000);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0xB025);	/* MCU_ADDRESS [AF_POSITIONS_0] */
	mt9t111_write(0x0990, 0x0000);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0xB026);	/* MCU_ADDRESS [AF_POSITIONS_1] */
	mt9t111_write(0x0990, 0x003A);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0xB027);	/* MCU_ADDRESS [AF_POSITIONS_2] */
	mt9t111_write(0x0990, 0x004A);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0xB028);	/* MCU_ADDRESS [AF_POSITIONS_3] */
	mt9t111_write(0x0990, 0x004E);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0xB029);	/* MCU_ADDRESS [AF_POSITIONS_4] */
	mt9t111_write(0x0990, 0x0052);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0xB02A);	/* MCU_ADDRESS [AF_POSITIONS_5] */
	mt9t111_write(0x0990, 0x0063);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0xB02B);	/* MCU_ADDRESS [AF_POSITIONS_6] */
	mt9t111_write(0x0990, 0x0072);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0xB02C);	/* MCU_ADDRESS [AF_POSITIONS_7] */
	mt9t111_write(0x0990, 0x0080);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0xB02D);	/* MCU_ADDRESS [AF_POSITIONS_8] */
	mt9t111_write(0x0990, 0x0090);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0xB02E);	/* MCU_ADDRESS [AF_POSITIONS_9] */
	mt9t111_write(0x0990, 0x0098);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0xB02F);	/* MCU_ADDRESS [AF_POSITIONS_10] */
	mt9t111_write(0x0990, 0x00AF);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0xB030);	/* MCU_ADDRESS [AF_POSITIONS_11] */
	mt9t111_write(0x0990, 0x0000);	/* MCU_DATA_0 */
	/*physical address acess */
	mt9t111_write(0x0982, 0x0000);	/* ACCESS_CTL_STAT */
	mt9t111_write(0x098A, 0x0351);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x3C3C);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0xC640);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0xF730);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0xC4DC);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x65FD);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x30C0);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0xDC67);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0xFD30);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0361);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0xC2DE);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x1DEC);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x25FD);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x30C5);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x0101);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x01FC);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x30C2);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0xFD06);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0371);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0xD6EC);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x0C5F);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x8402);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x4416);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x4F30);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0xE703);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0xD163);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x2714);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0381);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0xD163);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x2309);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0xFC06);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0xD604);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0xFD06);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0xD620);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x07FC);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x06D6);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0391);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x05FD);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x06D6);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0xDE1D);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0xE60B);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0xC407);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x30E7);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x02D1);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x6427);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x03A1);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x2B7D);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x0064);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x2726);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x5D27);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x237F);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x30C4);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0xFC06);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0xD6FD);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x03B1);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x30C5);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0xD664);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x4FFD);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x30C7);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0xC640);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0xF730);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0xC4E6);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x024F);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x03C1);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0xFD30);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0xC501);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x0101);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0xFC30);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0xC2FD);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x06D6);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x7D06);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0xCB27);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x03D1);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x2EC6);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x40F7);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x30C4);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0xFC06);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0xC104);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0xF306);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0xD6ED);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x005F);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x03E1);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x6D00);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x2A01);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x5317);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0xFD30);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0xC0EC);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x00FD);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x30C2);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0xFC06);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x03F1);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0xC1FD);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x30C5);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x0101);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x01FC);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x30C2);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0xFD06);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0xC720);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x227F);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0401);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x30C4);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0xDE1D);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0xEC25);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0xFD30);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0xC5FC);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x06D6);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0xFD30);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0xC701);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0411);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0xFC30);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0xC0FD);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x06D0);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0xFC30);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0xC2FD);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x06D2);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0xEC25);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0xFD06);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0421);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0xC3BD);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x953C);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0xDE3F);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0xEE10);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0xAD00);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0xDE1D);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0xFC06);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0xCCED);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0431);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x3E38);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x3839);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x3C3C);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0xBD61);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0xD5CE);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x04CD);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x1F17);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x0211);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0441);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0xCC33);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x2E30);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0xED02);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0xCCFF);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0xFDED);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x00CC);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x0002);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0xBD70);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0451);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x6D18);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0xDE1F);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x181F);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x8E01);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x10CC);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x3C52);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x30ED);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x0018);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0461);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0xECA0);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0xC4FD);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0xBD70);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x2120);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x1ECC);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x3C52);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x30ED);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x00DE);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0471);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x1FEC);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0xA0BD);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x7021);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0xCC3C);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x5230);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0xED02);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0xCCFF);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0xFCED);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0481);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x00CC);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x0002);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0xBD70);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x6D38);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x3839);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x3CFC);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x06EE);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x30ED);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0491);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x00BD);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0xA228);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x7D06);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0xCB27);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x11FC);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x06EE);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x30A3);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x0027);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x04A1);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x09FC);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x06C5);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0xC300);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x01FD);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x06C7);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x3839);	/* MCU_DATA_5 */
	mt9t111_write(0x098A, 0x0AE0);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x37DE);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x1DEC);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x0C5F);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x8402);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x4416);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x4FD7);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x63E6);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x0BC4);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0AF0);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x07D7);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x647F);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x30C4);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0xEC25);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0xFD30);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0xC5FC);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x06D6);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0xFD30);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0B00);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0xC701);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0xFC30);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0xC0DD);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x65FC);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x30C2);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0xDD67);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x30E6);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x00BD);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0B10);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x5203);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x3139);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x3CBD);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x776D);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0xCC32);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x5C30);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0xED00);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0xFC13);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0B20);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x8683);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x0001);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0xBD70);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x21CC);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x325E);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x30ED);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x00FC);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x1388);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0B30);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x8300);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x01BD);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x7021);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x3839);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x3C3C);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x3C34);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0xCE06);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x6A1E);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0B40);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x0210);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x06BD);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x8427);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x7E0B);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0xD47F);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x30C4);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0xCC01);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x00FD);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0B50);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x30C5);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0xCC00);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0xFFFD);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x30C7);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0xC640);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0xF730);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0xC4F6);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x13A7);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0B60);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x4F30);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0xED00);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0xCC00);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0xFFA3);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x00FD);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x30C5);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x0101);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x01FC);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0B70);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x30C2);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0xED04);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0xC300);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x01ED);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x046F);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x06C6);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x80F7);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x30C4);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0B80);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0xE606);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x4F05);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0xC306);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x738F);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0xEC00);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0xFD30);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0xC530);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0xEC04);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0B90);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0xFD30);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0xC7C6);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0xC0F7);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x30C4);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0xCC01);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x00FD);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x30C5);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x0101);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0BA0);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x01FC);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x30C2);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0xED02);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x8307);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0xFF2F);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x07CC);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x07FF);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0xED02);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0BB0);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x200C);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0xEC02);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x83F8);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x012C);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x05CC);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0xF801);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0xED02);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0xE606);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0BC0);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x4F05);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0xC306);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x7318);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x8FEC);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x0218);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0xED00);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x6C06);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0xE606);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0BD0);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0xC109);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x25A7);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x3838);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x3831);	/* MCU_DATA_3 */
	mt9t111_write(0x098A, 0x8BD8);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0982, 0x0000);	/* ACCESS_CTL_STAT */
	mt9t111_write(0x0990, 0x0039);	/* MCU_DATA_0 */
	mt9t111_write(0x0982, 0x0000);	/* ACCESS_CTL_STAT */
	mt9t111_write(0x098A, 0x0CD4);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x3C3C);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x3C3C);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0xFC06);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0xD230);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0xED06);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0xFC06);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0xD0ED);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x04DC);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0CE4);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x5DED);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x02DC);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x5BED);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x00BD);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0xD4D0);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x234D);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0xDC5B);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0xFD06);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0CF4);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0xD0DC);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x5DFD);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x06D2);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0xFC06);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0xD0DD);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x53FC);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x06D2);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0xDD55);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0D04);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x8640);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0xB730);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0xC4DC);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x53FD);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x30C0);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0xDC55);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0xFD30);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0xC2DC);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0D14);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x61FD);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x30C5);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x0101);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x7F30);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0xC4DC);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x59FD);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x30C5);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0xFC30);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0D24);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0xC2FD);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x30C7);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x01FC);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x30C2);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0xDD59);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x7C06);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0xCBFE);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x06C7);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0D34);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x08FF);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x06C7);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x7A00);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x5720);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x2FDE);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x1FEC);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x3130);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0xED06);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0D44);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x5F4F);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0xED04);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0xFC06);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0xD2ED);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x02FC);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x06D0);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0xED00);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0xBDD4);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0D54);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0xD023);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x12CE);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x06AD);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x1C01);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x01DE);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x1F5F);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x4FFD);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x06D0);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0D64);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0xEC31);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0xFD06);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0xD25F);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0xD757);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x3838);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x3838);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x393C);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x3C3C);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0D74);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x3C7F);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x30C4);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0xFC06);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0xCCFD);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x30C5);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0xD658);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x4FFD);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x30C7);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0D84);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0xC640);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0xF730);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0xC4DC);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x59FD);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x30C5);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x0101);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x01FC);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x30C0);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0D94);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0xDD53);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0xFC30);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0xC2DD);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x55DE);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x1FEC);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x3330);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0xED06);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x5F4F);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0DA4);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0xED04);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0xDC55);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0xED02);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0xDC53);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0xED00);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0xBDD4);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0xD023);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x2F7F);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0DB4);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x30C4);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0xDE1F);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0xEC33);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0xFD30);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0xC5DC);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x59FD);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x30C7);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0xC640);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0DC4);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0xF730);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0xC4FC);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x06CC);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0xFD30);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0xC501);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x0101);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0xFC30);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0xC2DD);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0DD4);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x59EC);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x33FD);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x06CC);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x7A06);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0xCB7A);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x0057);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x2030);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0xDE1F);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0DE4);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0xEC35);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x30ED);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x065F);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x4FED);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x04DC);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x55ED);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x02DC);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x53ED);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0DF4);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x00BD);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0xD4D0);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x2410);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0xDE1F);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x5F4F);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0xDD53);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0xEC35);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0xDD55);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0E04);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0xCE06);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0xAD1C);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x0102);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0xDC55);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0xFD06);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0xCC5F);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0xD757);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x3838);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0E14);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x3838);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x39C6);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x40F7);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x30C4);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0xDC53);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0xFD30);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0xC0DC);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x55FD);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0E24);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x30C2);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0xDE1F);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0xEC37);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0xFD30);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0xC501);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x0101);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0xFC30);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0xC2FD);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0E34);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x06C7);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0xEC3B);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0xB306);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0xC723);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x3D7F);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x30C4);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0xDC59);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0xFD30);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0E44);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0xC5EC);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x37FD);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x30C7);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0xC640);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0xF730);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0xC4DC);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x61FD);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x30C5);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0E54);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x0101);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x01FC);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x30C2);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0xDD59);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0xEC37);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0xFD06);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0xCEDC);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x5BFD);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0E64);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x06D0);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0xDC5D);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0xFD06);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0xD2EC);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x3B83);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x0001);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0xFD06);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0xC77A);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0E74);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x06CB);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x7A00);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x5739);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0xC640);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0xF730);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0xC4DC);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x53FD);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x30C0);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0E84);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0xDC55);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0xFD30);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0xC2FC);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x06C7);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0xFD30);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0xC501);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x0101);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0xFC30);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0E94);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0xC2FD);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x06CE);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x5FD7);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x5739);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0xFC06);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0xC5FD);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x06C7);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x8640);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0EA4);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0xB730);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0xC4DC);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x53FD);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x30C0);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0xDC55);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0xFD30);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0xC2FC);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x06C7);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0EB4);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0xFD30);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0xC501);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x0101);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0xFC30);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0xC2FD);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x06CE);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0xDE1F);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0xEC39);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0EC4);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0xB306);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0xCE23);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x22C6);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x40F7);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x30C4);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0xDC53);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0xFD30);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0xC0DC);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0ED4);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x55FD);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x30C2);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0xFC06);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0xC5FD);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x30C5);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x0101);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x01FC);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x30C2);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0EE4);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0xFD06);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0xCE5F);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0xD757);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x39DE);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x1FEC);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x39FD);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x06CE);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x7F30);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0EF4);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0xC4EC);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x39FE);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x06C5);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0xBDD4);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x79FD);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x30C5);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0xDC59);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0xFD30);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0F04);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0xC7C6);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x40F7);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x30C4);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0xDC61);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0xFD30);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0xC501);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x0101);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0xFC30);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0F14);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0xC2DD);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x597C);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x06CB);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x7A00);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x5739);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x3C3C);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x3C3C);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x3C34);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0F24);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0xBDAD);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x15DE);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x198F);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0xC301);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x168F);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0xE600);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0xF705);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x14E6);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0F34);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x03F7);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x0515);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0xEC35);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0xFD05);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x0EEC);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x37FD);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x0510);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0xFC05);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0F44);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x5BFD);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x0512);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0xDE37);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0xEE08);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0xAD00);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x30E7);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x0ADE);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x198F);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0F54);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0xC301);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x178F);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0xE600);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0xF705);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x14E6);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x03F7);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x0515);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0xDE37);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0F64);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0xEE08);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0xAD00);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x30E7);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x09DE);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x198F);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0xC301);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x188F);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0xE600);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0F74);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0xF705);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x14E6);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x03F7);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x0515);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0xDE37);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0xEE08);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0xAD00);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x30E7);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0F84);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x08CC);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x328E);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0xED00);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0xE60A);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x4FBD);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x7021);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0xCC32);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x6C30);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0F94);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0xED02);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0xCCF8);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x00ED);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x00E6);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x094F);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x175F);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0xED06);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0xE608);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0FA4);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x4FED);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x04E3);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x0684);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x07BD);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x706D);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x30C6);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x0B3A);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x3539);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x0FB4);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x7E9E);	/* MCU_DATA_0 */
	mt9t111_write(0x098A, 0x8FB6);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0982, 0x0000);	/* ACCESS_CTL_STAT */
	mt9t111_write(0x0990, 0x0014);	/* MCU_DATA_0 */
	mt9t111_write(0x0982, 0x0000);	/* ACCESS_CTL_STAT */
	mt9t111_write(0x098A, 0x1000);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0xF601);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x85C1);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x0326);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x10F6);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x0186);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0xC168);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x2609);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0xCC10);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x1010);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x18BD);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x4224);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0xBD10);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x1839);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0xC602);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0xF701);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x8AC6);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x0AF7);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x1020);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x018B);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0xDE3F);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x18CE);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x0BED);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0xCC00);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x11BD);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0xD700);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0xCC0B);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x1030);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0xEDDD);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x3FDE);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x3B18);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0xCE0B);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0xFFCC);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x0015);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0xBDD7);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x00CC);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x1040);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x0BFF);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0xDD3B);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0xDE35);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x18CE);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x0C15);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0xCC00);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x3FBD);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0xD700);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x1050);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0xCC0C);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x15DD);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x35DE);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x4718);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0xCE0C);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x55CC);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x0015);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0xBDD7);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x1060);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x00CC);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x0C55);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0xDD47);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0xDE41);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x18CE);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x0C6B);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0xCC00);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x0DBD);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x1070);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0xD700);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0xCC0C);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x6BDD);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x41DE);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x3918);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0xCE0C);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x79CC);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x0023);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x1080);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0xBDD7);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x00CC);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x0C79);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0xDD39);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0xDE31);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x18CE);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x0C9D);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0xCC00);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x1090);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x29BD);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0xD700);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0xCC0C);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x9DDD);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x31DE);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x4318);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0xCE0C);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0xC7CC);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x10A0);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x000B);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0xBDD7);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x00CC);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x0CC7);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0xDD43);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0xCC03);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x51FD);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x0BF9);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x10B0);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0xCC10);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0xE1FD);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x0BF7);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0xCC0B);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x38FD);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x0C0B);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0xCC04);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x35FD);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x10C0);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x0C31);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0xCC0F);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x1EFD);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x0C63);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0xCC0F);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0xB4FD);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x0C6D);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0xCC0B);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x10D0);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x14FD);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x0C85);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0xCC0A);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0xE0FD);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x0CA7);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0xCC04);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x8BFD);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x0CD1);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x10E0);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x3930);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x8FC3);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0xFFF0);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x8F35);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0xF606);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0xBC4F);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0xDD59);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x7F30);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x10F0);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0xC4FC);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x06C1);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0xFD30);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0xC5FC);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x06C3);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x0404);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0xFD30);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0xC701);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x1100);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0xFC30);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0xC0DD);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x5BFC);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x30C2);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0xDD5D);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0xFC06);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0xD2ED);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x02FC);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x1110);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x06D0);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0xED00);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0xC602);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0xBDD6);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0xE030);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0xEC00);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0xFD06);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0xD0EC);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x1120);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x02FD);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x06D2);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0xC604);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0xD757);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0xDC59);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x2605);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0xCC00);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x01DD);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x1130);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x597D);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x0059);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x2707);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0xC6FF);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x30E7);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x0C20);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x05D6);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x5A30);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x1140);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0xE70C);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0xF606);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0xBA54);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x5454);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x54E7);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x0D6C);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x0DF6);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x06C9);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x1150);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0xC101);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x260B);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0xF606);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0xBAE7);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x0EE6);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x0CE7);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x0F20);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x09E6);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x1160);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x0CE7);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x0EF6);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x06BA);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0xE70F);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0xE60E);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0xE10F);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x2206);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x7F06);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x1170);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0xC97E);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x1353);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0xE60E);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0xE00F);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0xE70F);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0xDE1F);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0xE62C);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x4F30);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x1180);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0xED0A);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0xE60F);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x4FEE);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x0A02);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x8F30);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0xE70F);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0xE10D);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x2404);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x1190);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0xE60D);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0xE70F);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0xF606);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0xC9C1);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x0126);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x08E6);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x0CEB);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x0FD7);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x11A0);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x5820);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x06E6);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x0CE0);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x0FD7);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x58F6);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x06CB);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x270D);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0xC101);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x11B0);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x2753);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0xC102);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x2603);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x7E13);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x4820);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x47FC);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x06D0);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0xDD5F);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x11C0);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0xFC06);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0xD2DD);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x6186);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x40B7);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x30C4);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0xDC5F);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0xFD30);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0xC0DC);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x11D0);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x61FD);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x30C2);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0xDC59);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0xFD30);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0xC501);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x017F);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x30C4);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0xD658);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x11E0);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x4FFD);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x30C5);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0xFC30);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0xC2FD);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x30C7);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x01FC);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x30C0);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0xDD53);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x11F0);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0xFC30);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0xC2DD);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x55DC);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x53FD);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x06D0);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0xDC55);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0xFD06);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0xD2BD);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x1200);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x0CD4);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x7E13);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x4B7F);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x30C4);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0xFC06);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0xC7FD);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x30C5);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0xFC06);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x1210);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0xCEFD);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x30C7);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x01FC);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x30C0);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0xDD5F);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0xFC30);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0xC2DD);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x617F);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x1220);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x30C4);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0xFD30);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0xC5D6);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x584F);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0xFD30);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0xC7C6);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x40F7);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x30C4);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x1230);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0xDC59);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0xFD30);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0xC501);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x0101);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0xFC30);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0xC0DD);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x53FC);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x30C2);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x1240);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0xDD55);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0xDE1F);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x3C18);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x38EC);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x2DCD);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0xEE37);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0xBDD4);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x7930);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x1250);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0xED0A);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x5F6D);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x0A2A);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x0153);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x17ED);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x08EC);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x0AED);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x06EC);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x1260);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x08ED);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x04DC);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x55ED);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x02DC);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x53ED);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x00BD);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0xD4D0);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x2305);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x1270);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0xBD0E);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x1720);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x8DDE);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x1F3C);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x1838);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0xEC2D);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0xCDEE);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x2FBD);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x1280);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0xD479);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x30ED);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x0A5F);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x6D0A);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x2A01);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x5317);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0xED08);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0xEC0A);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x1290);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0xED06);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0xEC08);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0xED04);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0xDC55);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0xED02);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0xDC53);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0xED00);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0xBDD4);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x12A0);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0xD024);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x6EC6);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x40F7);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x30C4);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0xDC53);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0xFD30);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0xC0DC);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x55FD);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x12B0);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x30C2);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0xDE1F);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0xEC2F);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0xFD30);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0xC501);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x0101);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0xFC30);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0xC2FD);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x12C0);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x06C7);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0xFC30);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0xC727);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x07FE);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x06C7);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x08FF);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x06C7);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0xC640);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x12D0);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0xF730);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0xC4DC);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x53FD);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x30C0);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0xDC55);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0xFD30);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0xC2FC);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x06C7);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x12E0);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0xFD30);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0xC501);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x0101);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0xFC30);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0xC2FD);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x06CE);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0xFC06);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0xC7B3);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x12F0);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x06C5);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x2305);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0xBD0E);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x9C20);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x52DE);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x1FEC);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x3BB3);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x06C7);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x1300);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x230A);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0xEC3B);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0xFD06);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0xC77A);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x0057);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x203F);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x5FD7);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x5720);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x1310);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x3ADE);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x1FEC);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x2DFD);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x06C7);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0xEC2D);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0xB306);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0xC523);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x06FC);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x1320);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x06C5);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0xFD06);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0xC7C6);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x40F7);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x30C4);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0xDC53);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0xFD30);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0xC0DC);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x1330);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x55FD);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x30C2);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0xFC06);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0xC7FD);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x30C5);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x0101);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x01FC);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x30C2);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x1340);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0xFD06);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0xCE5F);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0xD757);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x2003);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0xBD0D);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x717D);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x0057);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x2703);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x1350);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x7E11);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x28FC);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0x06D2);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0x30ED);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0x02FC);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x06D0);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0xED00);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0xC602);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x1360);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0xBDD6);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0xC030);	/* MCU_DATA_1 */
	mt9t111_write(0x0994, 0xEC00);	/* MCU_DATA_2 */
	mt9t111_write(0x0996, 0xFD06);	/* MCU_DATA_3 */
	mt9t111_write(0x0998, 0xD0EC);	/* MCU_DATA_4 */
	mt9t111_write(0x099A, 0x02FD);	/* MCU_DATA_5 */
	mt9t111_write(0x099C, 0x06D2);	/* MCU_DATA_6 */
	mt9t111_write(0x099E, 0x30C6);	/* MCU_DATA_7 */
	mt9t111_write(0x098A, 0x1370);	/* PHYSICAL_ADDR_ACCESS */
	mt9t111_write(0x0990, 0x103A);	/* MCU_DATA_0 */
	mt9t111_write(0x0992, 0x3539);	/* MCU_DATA_1 */
	mt9t111_write(0x098E, 0x0010);	/* MCU_ADDRESS */
	mt9t111_write(0x0990, 0x1000);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0x0003);	/* MCU_ADDRESS */
	mt9t111_write(0x0990, 0x0004);	/* MCU_DATA_0 */
	msleep(100);
	mt9t111_write(0x098E, 0x8400);	/* MCU_ADDRESS */
	mt9t111_write(0x0990, 0x0006);	/* MCU_DATA_0 */
	msleep(35);

	/* MAY BE NEED TO STOP STREAMING HERE. */

	/*[ Mirroring function  - flip_mirror] */
	mt9t111_write(0x098E, 0x480C);	/* MCU_ADDRESS [CAM1_CTX_A_READ_MODE] */
	mt9t111_write(0x0990, 0x046F);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0x4854);	/* MCU_ADDRESS [CAM1_CTX_B_READ_MODE] */
	mt9t111_write(0x0990, 0x0027);	/* MCU_DATA_0 */
	mt9t111_write(0x098E, 0x8400);	/* MCU_ADDRESS [SEQ_CMD] */
	mt9t111_write(0x0990, 0x0006);	/* MCU_DATA_0 */

	if (checkCameraID(sensor)) {
		result = HAL_CAM_ERROR_INTERNAL_ERROR;
	}
	tm1 = ktime_get();
	pr_info("Exit Init Sec %d nsec %d\n", tm1.tv.sec, tm1.tv.nsec);

	return result;

}
struct sens_methods sens_meth = {
    DRV_GetIntfConfig: CAMDRV_GetIntfConfig,
    DRV_GetIntfSeqSel : CAMDRV_GetIntfSeqSel,
    DRV_Wakeup : CAMDRV_Wakeup,
    DRV_GetResolution : CAMDRV_GetResolution,
    DRV_SetVideoCaptureMode : CAMDRV_SetVideoCaptureMode,
    DRV_SetFrameRate : CAMDRV_SetFrameRate,
    DRV_EnableVideoCapture : CAMDRV_EnableVideoCapture,
    DRV_SetCamSleep : CAMDRV_SetCamSleep,
    DRV_GetJpegSize : CAMDRV_GetJpegSize,
    DRV_GetJpeg : CAMDRV_GetJpeg,
    DRV_GetThumbnail : CAMDRV_GetThumbnail,
    DRV_DisableCapture : CAMDRV_DisableCapture,
    DRV_DisablePreview : CAMDRV_DisablePreview,
    DRV_CfgStillnThumbCapture : CAMDRV_CfgStillnThumbCapture,
    DRV_StoreBaseAddress : CAMDRV_StoreBaseAddress,
    DRV_SetSceneMode : CAMDRV_SetSceneMode,
    DRV_SetWBMode : CAMDRV_SetWBMode,
    DRV_SetAntiBanding : CAMDRV_SetAntiBanding,
    DRV_SetFocusMode : CAMDRV_SetFocusMode,
    DRV_SetDigitalEffect : CAMDRV_SetDigitalEffect,
    DRV_SetFlashMode : CAMDRV_SetFlashMode,
    DRV_SetJpegQuality : CAMDRV_SetJpegQuality,
    DRV_TurnOnAF : CAMDRV_TurnOnAF,
    DRV_TurnOffAF : CAMDRV_TurnOffAF,
	DRV_SetZoom : CAMDRV_SetZoom,

};

struct sens_methods *CAMDRV_primary_get(void)
{
	return &sens_meth;
}
