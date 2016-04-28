/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2011  Broadcom Corporation                                                        */
/*                                                                                              */
/*     Unless you and Broadcom execute a separate written software license agreement governing  */
/*     use of this software, this software is licensed to you under the terms of the GNU        */
/*     General Public License version 2 (the GPL), available at                                 */
/*                                                                                              */
/*          http://www.broadcom.com/licenses/GPLv2.php                                          */
/*                                                                                              */
/*     with the following added to such license:                                                */
/*                                                                                              */
/*     As a special exception, the copyright holders of this software give you permission to    */
/*     link this software with independent modules, and to copy and distribute the resulting    */
/*     executable under terms of your choice, provided that you also meet, for each linked      */
/*     independent module, the terms and conditions of the license of that module.              */
/*     An independent module is a module which is not derived from this software.  The special  */
/*     exception does not apply to any modifications of the software.                           */
/*                                                                                              */
/*     Notwithstanding the above, under no circumstances may you combine this software in any   */
/*     way with any other Broadcom software provided under a license other than the GPL,        */
/*     without Broadcom's express prior written consent.                                        */
/*                                                                                              */
/************************************************************************************************/

/**
*
*   @file   csl_lcd.h
*
*   @brief  CSL common APIs for all LCD controllers (LCDC/DSI/SMI)
*
****************************************************************************/
/**
*
* @defgroup LCDGroup LCD Controllers
*
* @brief    This group defines the common APIs for all LCD controllers
*
* @ingroup  CSLGroup
*****************************************************************************/

/**
*
*  @file  csl_lcd.h
*
*  @brief LCD Controller(s) API
*
*  @note
*****************************************************************************/

/* Requires the following header files before its inclusion in a c file
#include "irqctrl.h"
#include "dbg.h"
#include "logapi.h"
*/

/**
*/

#ifndef	__CSL_LCD_H__
#define	__CSL_LCD_H__

/**
*
* @addtogroup LCDGroup
* @{
*/

#ifdef __cplusplus
extern "C" {
#endif

/* #define __RHEA_LCD_DBG__ */

#ifdef __RHEA_LCD_DBG__
#define LCD_DBG(id, fmt, args...)         printk(KERN_ERR fmt, ##args)
#else
	typedef enum {
		LCD_DBG_ID = 0,
		LCD_DBG_ERR_ID = 1,
		LCD_DBG_INIT_ID = 2,
	} LCD_DBG_ID_TYPE;

#define LCD_DBG(id, fmt, args...)	do { if (id == LCD_DBG_ERR_ID) \
				printk(KERN_ERR fmt, ##args); } while (0)
#endif

/**
*
* CSL LCD Interface Result  
*
*****************************************************************************/
	typedef enum {
		CSL_LCD_OK,	///< OK, no Error
		CSL_LCD_ERR,	///< CSL Generic Error
		CSL_LCD_INT_ERR,	///< Internal Error, Condition Should Never happen
		CSL_LCD_NOT_INIT,	///< CSL not Init
		CSL_LCD_NOT_OPEN,	///< CSL not Open
		CSL_LCD_IS_OPEN,	///< CSL Already Open
		CSL_LCD_MSG_SIZE,	///< DSI msg size err
		CSL_LCD_API_ERR,	///< Wrong Use of API
		CSL_LCD_COL_MODE,	///< Invalid Color Mode IN/OUT
		CSL_LCD_TE_TOUT,	///< Timeout Waiting For Tearing 
		CSL_LCD_BUS_ERR,	///< BUS Error Detected
		CSL_LCD_DMA_ERR,	///< DMA Driver Error
		CSL_LCD_OS_TOUT,	///< Generic OS TimeOut
		CSL_LCD_OS_ERR,	///< Generic OS Err 
		CSL_LCD_BUS_ID,	///< invalid Bus ID
		CSL_LCD_BUS_CFG,	///< bus configuration error
		CSL_LCD_BUS_TYPE,	///< on open, invalid bus type
		CSL_LCD_BAD_HANDLE,	///< Invalid handle
		CSL_LCD_BAD_STATE,	///< Action Not Supported In Curr. Interface State
		CSL_LCD_INST_COUNT,	///< open failure due to instance count
	} CSL_LCD_RES_T;

/**
*
* CSL LCD Interface Handle 
*
*****************************************************************************/
	typedef void *CSL_LCD_HANDLE;	///< lcd handle

/**
*
*  CSL LCD Callback Record 
*
*****************************************************************************/
	typedef struct {
		CSL_LCD_HANDLE cslH;	///< cslH used by dispDrv
		UInt32 dispDrvApiCbRev;	///< DISP DRV cb api rev
		void *dispDrvApiCb;	///< DISP DRV API cb
		void *dispDrvApiCbP1;	///< DISP DRV API cb p1 (pFb)
	} CSL_LCD_CB_REC_T, *pCSL_LCD_CB_REC;

/**
*
* CSL LCD Callback
*
*****************************************************************************/
//typedef void (*CSL_LCD_CB_T) ( CSL_LCD_RES_T, CSL_LCD_HANDLE, void* );  
	typedef void (*CSL_LCD_CB_T) (CSL_LCD_RES_T, pCSL_LCD_CB_REC);

/**
*
* BUS IDs 
*
*****************************************************************************/
	typedef enum {
		BUS_0 = 0,	///< first  LCD bus in the system
		BUS_1 = 1,	///< second LCD bus in the system
	} CSL_LCD_BUS_ID_T;

/**
*
* BUS CH Enums 
*
*****************************************************************************/
	typedef enum {
		BUS_CH_0 = 0,	///< 1'st channel/bank of LCD bus
		BUS_CH_1 = 1,	///< 2'nd channel/bank of LCD bus
		BUS_CH_2 = 2,	///< 3'th channel/bank of LCD bus
		BUS_CH_3 = 3,	///< 4'th channel/bank of LCD bus
	} CSL_LCD_BUS_CH_T;

/**
*
* Update Request - LCD Peripheral Driver 
*
*****************************************************************************/
	typedef struct {
		void *buff;	///< frame buffer
		UInt32 buffBpp;	///< frame buffer BytesPerPixel
		UInt32 lineLenP;	///< HOR length in pixels
		UInt32 lineCount;	///< VER length in lines
		UInt32 xStrideB;	///< stride in bytes
		UInt32 timeOut_ms;	///< msec to wait for the end of upd
		CSL_LCD_CB_T cslLcdCb;	///< CSL CallBack
		CSL_LCD_CB_REC_T cslLcdCbRec;	///< CSL CallBack Record
	} CSL_LCD_UPD_REQ_T, *pCSL_LCD_UPD_REQ;

/**
*
* Output Color Modes (display)
*
*****************************************************************************/
	typedef enum {
		LCD_IF_CM_O_RGB565,      	///< RGB565    
		LCD_IF_CM_O_RGB565_DSI_VM,  	///< RGB565
		LCD_IF_CM_O_RGB666,		/* < RGB666 Packed */
		LCD_IF_CM_O_xRGB8888,		/* xRGB8888 */
		LCD_IF_CM_O_xBGR8888,		/* xRGB8888 */
		LCD_IF_CM_O_RGB666U,		/* RGB666 unpacked */
		LCD_IF_CM_O_INV,
	} CSL_LCD_CM_OUT;

/**
*
* Input Color Modes (memory frame buffer)
*
*****************************************************************************/
	typedef enum {
		LCD_IF_CM_I_RGB565P,	///< 2 565 pixels per 32-bit word
		LCD_IF_CM_I_xRGB8888,
		LCD_IF_CM_I_xBGR8888,
		LCD_IF_CM_I_INV,
	} CSL_LCD_CM_IN;

#ifdef __cplusplus
}
#endif
/** @} */
#endif				/* __CSL_LCD_H__ */
