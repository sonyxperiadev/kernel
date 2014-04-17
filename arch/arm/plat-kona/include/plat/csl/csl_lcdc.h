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
*   @file   csl_lcdc.h
*
*   @brief  CSL LCD Controller Driver Interface (Z80/M68/DBI-B/DBI-C)
*
****************************************************************************/
/**
*
* @defgroup LCDGroup LCD Controllers
*
* @brief    LCD Controller(s) API
*
* @ingroup  CSLGroup
*****************************************************************************/

/* Requires the following header files before its inclusion in a c file
#inlcude "csl_lcd.h"
#include "irqctrl.h"
*/

#ifndef _CSL_LCDC_H_
#define _CSL_LCDC_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup LCDGroup
 * @{
 */

/**
*
* LCDC Controller Init Settings
*
*****************************************************************************/
	typedef struct {
		UInt32 maxBusWidth;
		UInt32 lcdc_base_address;
	} CSL_LCDC_INIT_T, *pCSL_LCDC_INIT;

/**
*
* LCD Controller (DBI) Bus Modes
*
*****************************************************************************/
	typedef enum {
		LCDC_BUS_DBI_B = 1,	///< DBI-B 8-bit
		LCDC_BUS_DBI_C_OPT1 = 2,	///< DBI-C 3-WIRE   D/CX embeded in protocol
		LCDC_BUS_DBI_C_OPT3 = 3	///< DBI-C 4-WIRE   D/CX phy line
	} CSL_LCDC_DBI_BUS_TYPE_t;

/**
*
* LCD Controller (Z80/M68) Bus Modes
*
*****************************************************************************/
	typedef enum {
		LCDC_BUS_Z80 = 4,	///< Z80
		LCDC_BUS_M68 = 5,	///< M68
	} CSL_LCDC_PAR_BUS_TYPE_t;

/**
*
* LCD Controller (DBI) Read Access Type
*
*****************************************************************************/
	typedef enum {
		DBI_RD_08_CMND_08_DATA = 0,	///< WR 8-bit CMND Followed By 08-bit DATA RD
		DBI_RD_NO_CMND_08_DATA = 1,	///<                           08-bit DATA RD
		DBI_RD_08_CMND_24_DATA = 2,	///< WR 8-bit CMND Followed By 24-bit DATA RD
		DBI_RD_NO_CMND_24_DATA = 3	///<                           24-bit DATA RD
	} CSL_LCDC_DBI_RD_t;

/**
*
* LCD Controller (Z80/M68) Mode Settings
*
*****************************************************************************/
	typedef struct {
		CSL_LCDC_PAR_BUS_TYPE_t busType;	///< Type Of The Bus
		CSL_LCD_BUS_CH_T csBank;	///< CS bank
		UInt32 busWidth;	///< Bus Width
		Boolean bitSwap;	///< bit swap
		Boolean byteSwap;	///< byte swap
		Boolean wordSwap;	///< word swap
		CSL_LCD_CM_IN colModeIn;	///< Color Mode Of Data-IN
		CSL_LCD_CM_OUT colModeOut;	///< Color Mode Of Data-OUT
	} CSL_LCDC_PAR_CFG_t, *pCSL_LCDC_PAR_CFG;

/**
*
* LCD Controller (Z80/M68) Rd/Wr Speed Settings
*
*****************************************************************************/
	typedef struct {
		UInt32 rdHold;	///< Rd Cycle Settings
		UInt32 rdPulse;	///< Rd Cycle Settings
		UInt32 rdSetup;	///< Rd Cycle Settings
		UInt32 wrHold;	///< Wr Cycle Settings
		UInt32 wrPulse;	///< Wr Cycle Settings
		UInt32 wrSetup;	///< Wr Cycle Settings
	} CSL_LCDC_PAR_SPEED_t, *pCSL_LCDC_PAR_SPEED;

/**
*
* LCD Controller (DBI) Mode Settings
*
*****************************************************************************/
	typedef struct {
		CSL_LCDC_DBI_BUS_TYPE_t busType;	///< Type Of The Bus
		CSL_LCD_BUS_CH_T csBank;	///< CS bank
		UInt32 busWidth;	///< Bus Width
		Boolean colModeInBE;	///< Data-IN endianes
		CSL_LCD_CM_IN colModeIn;	///< Color Mode Of Data-IN
		CSL_LCD_CM_OUT colModeOut;	///< Color Mode Of Data-OUT
#ifdef _ATHENA_
		Boolean dbicSclRouteToLcdCd;	///< Else To LcdWe
#endif
	} CSL_LCDC_DBI_CFG_t, *CSL_LCDC_DBI_CFG;

/**
*
* DBI LCD Read Request
*
*****************************************************************************/
	typedef struct {
		UInt32 command;	///< in  8-bit Command To Write
		CSL_LCDC_DBI_RD_t type;	///< in  RD Access Type
		UInt32 data;	///< out data read
	} CSL_LCDC_DBI_RD_REQ_t, *pCSL_LCDC_DBI_RD_REQ;

/**
*
* LCD Controller (DBI) Rd/Wr Speed Settings
*
*****************************************************************************/
	typedef struct {
		UInt32 tAhbClkC;	///< DBI T-Cycle Setting
#ifdef _ATHENA_
		UInt32 dcxHi;	///< A  Hold   (dcxHi+1)*T
		UInt32 dcxLo;	///< A  Setup  (dcxLo+1)*T
		UInt32 rdHi;	///< Rd High   (rdHi +6)*T
		UInt32 rdLo;	///< Rd Low    (rdLo +6)*T
		UInt32 wrHi;	///< Wr High   (wrHi +4)*T
		UInt32 wrLo;	///< Wr Low    (wrLo +4)*T
#endif
	} CSL_LCDC_DBI_SPEED_t, *pCSL_LCDC_DBI_SPEED;

/**
*
*  LCDC IO Pin Settings
*
*****************************************************************************/
	typedef struct {
		Boolean slewFast;	///< Fast|Slow Slew
		UInt32 driveStrength;	///< Drive Strength
	} CSL_LCDC_IO_DRIVE_t, *pCSL_LCDC_IO_DRIVE;

/**
*
* TEARING Configuration
*
*****************************************************************************/
	typedef enum {
		LCDC_TE_NONE,
		LCDC_TE_CTRLR,
	} CSL_LCDC_TE_TYPE_T;

	typedef struct {
		CSL_LCDC_TE_TYPE_T type;	///< TE type -1=NU 0=CtrlrSynced
		UInt32 delay;	///< TE delay
		UInt32 pinSel;	///< TE pin sel
		Boolean edgeRising;	///< TE edge
	} CSL_LCDC_TE_T, *pCSL_LCDC_TE;

/**
*
*  LCDC Parallel Controller Configuration (Z80/M68)
*
*****************************************************************************/
	typedef struct {
		CSL_LCDC_PAR_CFG_t cfg;
		CSL_LCDC_PAR_SPEED_t speed;
		CSL_LCDC_IO_DRIVE_t io;
		CSL_LCDC_TE_T teCfg;
	} CSL_LCDC_PAR_CTRL_T, *pCSL_LCDC_PAR_CTRL;

/**
*
*  LCDC DBI Controller Configuration
*
*****************************************************************************/
	typedef struct {
		CSL_LCDC_DBI_CFG_t cfg;
		CSL_LCDC_DBI_SPEED_t speed;
		CSL_LCDC_IO_DRIVE_t io;
		CSL_LCDC_TE_T teCfg;
	} CSL_LCDC_DBI_CTRL_T, *pCSL_LCDC_DBI_CTRL;

/**
*
*  @brief    Write Command
*
*  @param	 lcdcH   (in) CSL LCDC handle
*  @param    command (in) command
*
*  @return	 CSL_LCD_RES_T
*
*  @note     Common to Z80/M68 & DBI modes
*
*****************************************************************************/
	CSL_LCD_RES_T CSL_LCDC_WrCmnd(CSL_LCD_HANDLE lcdcH, UInt32 command);

/**
*
*  @brief    Write Data
*
*  @param	 lcdcH   (in) CSL LCDC handle
*  @param    data    (in) data
*
*  @return	 CSL_LCD_RES_T
*
*  @note     Common to Z80/M68 & DBI modes
*
*****************************************************************************/
	CSL_LCD_RES_T CSL_LCDC_WrData(CSL_LCD_HANDLE lcdcH, UInt32 data);

/**
*
*  @brief    Read From Z80/M68 LCD Using 'Cx' Rd Cycle
*
*  @param	 lcdcH   (in)  CSL LCDC handle
*  @param    *data   (out) data read
*
*  @return	 CSL_LCD_RES_T
*
*  @note     Z80/M68 mode
*
*****************************************************************************/
	CSL_LCD_RES_T CSL_LCDC_PAR_RdCmnd(CSL_LCD_HANDLE lcdcH, UInt32 * data);

/**
*
*  @brief    Read From Z80/M68 LCD Using 'D' Rd Cycle
*
*  @param	 lcdcH   (in)  CSL LCDC handle
*  @param    *data   (out) data read
*
*  @return	 CSL_LCD_RES_T
*
*  @note     Z80/M68 mode
*
*****************************************************************************/
	CSL_LCD_RES_T CSL_LCDC_PAR_RdData(CSL_LCD_HANDLE lcdcH, UInt32 * data);

/**
*
*  @brief    DBI Read Access
*
*  @param	 lcdcH   (in)    CSL LCDC handle
*  @param    rdAcc   (inout) (in)  access definition & data read
*                            (out) data read
*
*  @return	 CSL_LCD_RES_T
*
*  @note     Z80/M68 mode
*
*****************************************************************************/
	CSL_LCD_RES_T CSL_LCDC_DBI_RdAccess(CSL_LCD_HANDLE lcdcH,
					    pCSL_LCDC_DBI_RD_REQ rdAcc);

/**
*
*  @brief    LCD Module - DMA based update of frame buffer
*
*  @param	 lcdcH   (in) CSL LCDC handle
*  @param    req     (in) update request definition
*
*  @return	 CSL_LCD_RES_T (out)
*
*  @note     SYNC/ASYNC version
*            Common to Z80/M68 & DBI modes
*
*****************************************************************************/
	CSL_LCD_RES_T CSL_LCDC_Update(CSL_LCD_HANDLE lcdcH,
				      pCSL_LCD_UPD_REQ req);

/**
*
*  @brief    Close Legacy(Z80/M68) or DBI(DBI-B/DBI-C) Interface
*
*  @param	 lcdcH   (in) CSL LCDC handle
*
*  @return	 CSL_LCD_RES_T
*
*  @note     Common to Z80/M68 & DBI modes
*
*****************************************************************************/
	CSL_LCD_RES_T CSL_LCDC_Close(CSL_LCD_HANDLE * lcdcH);

/**
*
*  @brief    Opens LCDC Controller bank in Z80/M68 mode
*
*  @param	 busCfg  (in)  Parallel Controller Configuration
*
*  @return	 *lcdcH        (out) on SUCCESS holds LCDCD controller bank handle
*            CSL_LCD_RES_T (out)
*
*  @note     Common to Z80/M68 modes
*
*****************************************************************************/
	CSL_LCD_RES_T CSL_LCDC_PAR_Open(pCSL_LCDC_PAR_CTRL busCfg,
					CSL_LCD_HANDLE * lcdcH);

/**
*
*  @brief    Opens LCDC Controller bank in DBI mode
*
*  @param	 busCfg  (in)  Parallel Controller Configuration
*
*  @return	 *lcdcH        (out) on SUCCESS holds LCDCD controller bank handle
*            CSL_LCD_RES_T (out)
*
*  @note     Common to DBI modes
*
*****************************************************************************/
	CSL_LCD_RES_T CSL_LCDC_DBI_Open(pCSL_LCDC_DBI_CTRL busCfg,
					CSL_LCD_HANDLE * lcdcH);

/**
*
*  @brief    Issue Command, than enable Pixel Mode of DBI controller
*
*  @param	 lcdcH   (in) CSL LCDC handle
*  @param    command (in) command to write
*
*  @return	 CSL_LCD_RES_T (out)
*
*  @note     Common to DBI modes
*            Pixel Mode is canceled with next command write
*
*****************************************************************************/
	CSL_LCD_RES_T CSL_LCDC_DBI_WrCmndCmOn(CSL_LCD_HANDLE lcdcH,
					      UInt32 command);

/**
*
*  @brief    Change Legacy (Z80/M68) Par LCD Bus RD/WR Timing
*
*  @param	 lcdcH   (in) CSL LCDC handle
*  @param    pSpeed  (in) Par Bus RD/WR Timing
*
*  @return	 CSL_LCD_RES_T
*
*  @note     Common to LEGACY Parallel LCD Interface (Z80/M68)
*            Timing In Effect Until Changed with Same Command
*
*****************************************************************************/
	CSL_LCD_RES_T CSL_LCDC_PAR_SetSpeed(CSL_LCD_HANDLE lcdcH,
					    pCSL_LCDC_PAR_SPEED pSpeed);

/**
*
*  @brief    Change DBI-B/DBI-C LCD Bus RD/WR Timing
*
*  @param	 lcdcH   (in) CSL LCDC handle
*  @param    pSpeed  (in) DBI-B/DBI-C Bus RD/WR Timing
*
*  @return	 CSL_LCD_RES_T
*
*  @note     Common to DBI-B & DBI-C LCD Interfaces
*            Timing In Effect Until Changed with Same Command
*
*****************************************************************************/
	CSL_LCD_RES_T CSL_LCDC_DBI_SetSpeed(CSL_LCD_HANDLE lcdcH,
					    pCSL_LCDC_DBI_SPEED pSpeed);

/**
*
*  @brief    Lock LCDC Controller For Exclusive Use (all supported modes)
*
*  @param	 lcdcH         (in) CSL LCDC handle
*
*  @return	 CSL_LCD_RES_T (out)
*
*  @note     Common to Z80/M68 & DBI modes
*
*****************************************************************************/
	CSL_LCD_RES_T CSL_LCDC_Lock(CSL_LCD_HANDLE lcdcH);

/**
*
*  @brief    Unlock LCDC Controller (all supported modes)
*
*  @param	 lcdcH         (in) CSL LCDC handle
*
*  @return	 CSL_LCD_RES_T (out)
*
*  @note     Common to Z80/M68 & DBI modes
*
*****************************************************************************/
	CSL_LCD_RES_T CSL_LCDC_Unlock(CSL_LCD_HANDLE lcdcH);

/**
*
*  @brief    Inits LCDC Controller (all supported modes)
*
*  @param	 ctrlInit      (in) configuration settings
*
*  @return	 CSL_LCD_RES_T (out)
*
*  @note     Common to Z80/M68 & DBI modes
*
*****************************************************************************/
	CSL_LCD_RES_T CSL_LCDC_Init(pCSL_LCDC_INIT ctrlInit);

/**
*
*  @brief    Enable / Disable colour expansion from 16-bit to 18-bit
*
*  @param	 lcdcH         (in) CSL LCDC handle
*
*  @param	 enable	       (in) enable / disable
*
*  @return	 CSL_LCD_RES_T (out)
*
*  @note     Z80/M68 modes (Legacy Interface)
*
*****************************************************************************/
	CSL_LCD_RES_T CSL_LCDC_Enable_CE(CSL_LCD_HANDLE lcdcH, bool enable);

/** @} */

#ifdef __cplusplus
}
#endif
#endif				// _CSL_LCDC_H_
