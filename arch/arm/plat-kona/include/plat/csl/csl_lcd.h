/*****************************************************************************
*
*    (c) 2001-2009 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or
* its licensors, and may only be used, duplicated, modified or distributed
* pursuant to the terms and conditions of a separate, written license
* agreement executed between you and Broadcom (an "Authorized License").
* Except as set forth in an Authorized License, Broadcom grants no license
* (express or implied), right to use, or waiver of any kind with respect to
* the Software, and Broadcom expressly reserves all rights in and to the
* Software and all intellectual property rights therein.
* IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS
* SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE
* ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
*    IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS
*    FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS,
*    QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU
*    ASSUME THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
*
*****************************************************************************/
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

//#define __RHEA_LCD_DBG__ 

#ifdef __RHEA_LCD_DBG__
#define LCD_DBG(id, fmt, args...)         printk(KERN_ERR fmt, ##args)
#else
typedef enum 
{
	LCD_DBG_ID = 0,
	LCD_DBG_ERR_ID = 1,
	LCD_DBG_INIT_ID = 2,
} LCD_DBG_ID_TYPE;

#define LCD_DBG(id, fmt, args...)	do { if (id == LCD_DBG_ERR_ID) printk(KERN_ERR fmt, ##args); } while (0)
#endif 

/**
*
* CSL LCD Interface Result  
*
*****************************************************************************/
typedef enum
{
    CSL_LCD_OK,          ///< OK, no Error
    CSL_LCD_ERR,         ///< CSL Generic Error
    CSL_LCD_INT_ERR,     ///< Internal Error, Condition Should Never happen
    CSL_LCD_NOT_INIT,    ///< CSL not Init
    CSL_LCD_NOT_OPEN,    ///< CSL not Open
    CSL_LCD_IS_OPEN,     ///< CSL Already Open
    CSL_LCD_COL_MODE,    ///< Invalid Color Mode IN/OUT
    CSL_LCD_TE_TOUT,     ///< Timeout Waiting For Tearing 
    CSL_LCD_BUS_ERR,     ///< BUS Error Detected
    CSL_LCD_DMA_ERR,     ///< DMA Driver Error
    CSL_LCD_OS_TOUT,     ///< Generic OS TimeOut
    CSL_LCD_OS_ERR,      ///< Generic OS Err 
    CSL_LCD_BUS_ID,      ///< invalid Bus ID
    CSL_LCD_BUS_CFG,     ///< bus configuration error
    CSL_LCD_BUS_TYPE,    ///< on open, invalid bus type
    CSL_LCD_BAD_HANDLE,  ///< Invalid handle
    CSL_LCD_BAD_STATE,   ///< Action Not Supported In Curr. Interface State
    CSL_LCD_INST_COUNT,  ///< open failure due to instance count
} CSL_LCD_RES_T;    

/**
*
* CSL LCD Interface Handle 
*
*****************************************************************************/
typedef void* CSL_LCD_HANDLE;                       ///< lcd handle

/**
*
*  CSL LCD Callback Record 
*
*****************************************************************************/
typedef struct 
{
    CSL_LCD_HANDLE  cslH;               ///< cslH used by dispDrv
    UInt32          dispDrvApiCbRev;    ///< DISP DRV cb api rev
    void*           dispDrvApiCb;       ///< DISP DRV API cb
    void*           dispDrvApiCbP1;     ///< DISP DRV API cb p1 (pFb)
} CSL_LCD_CB_REC_T, *pCSL_LCD_CB_REC;

/**
*
* CSL LCD Callback  
*
*****************************************************************************/
//typedef void (*CSL_LCD_CB_T) ( CSL_LCD_RES_T, CSL_LCD_HANDLE, void* );  
typedef void (*CSL_LCD_CB_T) ( CSL_LCD_RES_T, pCSL_LCD_CB_REC );  


/**
*
* BUS IDs 
*
*****************************************************************************/
typedef enum
{
    BUS_0    = 0,      ///< first  LCD bus in the system
    BUS_1    = 1,      ///< second LCD bus in the system
} CSL_LCD_BUS_ID_T;

/**
*
* BUS CH Enums 
*
*****************************************************************************/
typedef enum
{
    BUS_CH_0 = 0,      ///< 1'st channel/bank of LCD bus
    BUS_CH_1 = 1,      ///< 2'nd channel/bank of LCD bus
    BUS_CH_2 = 2,      ///< 3'th channel/bank of LCD bus
    BUS_CH_3 = 3,      ///< 4'th channel/bank of LCD bus
} CSL_LCD_BUS_CH_T;

/**
*
* Update Request - LCD Peripheral Driver 
*
*****************************************************************************/
typedef struct {
    void*                 buff;         ///< frame buffer
    UInt32                buffBpp;      ///< frame buffer BytesPerPixel
    UInt32                lineLenP;     ///< HOR length in pixels
    UInt32                lineCount;    ///< VER length in lines
    UInt32                timeOut_ms;   ///< msec to wait for the end of upd
    CSL_LCD_CB_T          cslLcdCb;     ///< CSL CallBack
    CSL_LCD_CB_REC_T      cslLcdCbRec;  ///< CSL CallBack Record
} CSL_LCD_UPD_REQ_T, *pCSL_LCD_UPD_REQ; 
                                                                                
/**
*
* Output Color Modes (display)
*
*****************************************************************************/
typedef enum 
{
    LCD_IF_CM_O_RGB332,      ///< RGB332  
    LCD_IF_CM_O_RGB444,      ///< RGB444  
    LCD_IF_CM_O_RGB565,      ///< RGB565    
    LCD_IF_CM_O_RGB666,      ///< RGB666    
    LCD_IF_CM_O_RGB888,      ///< RGB888    
} CSL_LCD_CM_OUT;

/**
*
* Input Color Modes (memory frame buffer)
*
*****************************************************************************/
typedef enum 
{
    LCD_IF_CM_I_RGB565P,        ///< 2 565 pixels per 32-bit word
    LCD_IF_CM_I_RGB565U_LSB,    ///< 1 565 pixels per 32-bit word (LSB)
    LCD_IF_CM_I_RGB666,
    LCD_IF_CM_I_RGB888U,        ///< 1 888 pixels per 32-bit word (MSB DC/A)
    LCD_IF_CM_I_RGB888P,        ///< 1 888 pixels per 24-bit word 
} CSL_LCD_CM_IN;

#ifdef __cplusplus
}
#endif 

/** @} */
#endif	/* __CSL_LCD_H__ */
