/*******************************************************************************
* Copyright 2011 Broadcom Corporation.  All rights reserved.
*
*             @file     arch/arm/plat-kona/include/plat/csl/csl_tectl_vc4lite.h
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

/**
*
*   @file   csl_tectl_vc4lite.h
*
*   @brief  MM TECTL CSL interface
*
****************************************************************************/
/**
*
* @defgroup LCDGroup LCD Controllers
*
* @brief    MM TECTL CSL interface
*
* @ingroup  CSLGroup
*****************************************************************************/

/* Requires the following header files before its inclusion in a c file
*/

#ifndef _CSL_TECTL_VC4LITE_H_
#define _CSL_TECTL_VC4LITE_H_

#ifdef __cplusplus
extern "C" {
#endif

/** 
 * @addtogroup LCDGroup 
 * @{
 */

/**
*
*  Chip TE Input Pins ( inputs to TE MUX )
*
*****************************************************************************/
#define TE_VC4L_IN_0_LCD	(0)	//  RHEA's LCD_TE  input
#define TE_VC4L_IN_1_DSI0	(1)	//  RHEA's DSI0_TE input
#define TE_VC4L_IN_2_DSI1	(2)	//  RHEA's DSI1_TE input
#define TE_VC4L_IN_INV		(3)	//  

/**
*
*  TECTL MUX Outputs
*
*****************************************************************************/
#define TE_VC4L_OUT_SPI             (0)	//  SPI Core
#define TE_VC4L_OUT_SMI             (1)	//  SMI Core
#define TE_VC4L_OUT_DSI0_TE0        (2)	//  DSI Core 0 TE Input 0
#define TE_VC4L_OUT_DSI0_TE1        (3)	//  DSI Core 0 TE Input 1
#define TE_VC4L_OUT_DSI1_TE0        (4)	//  DSI Core 1 TE Input 0
#define TE_VC4L_OUT_DSI1_TE1        (5)	//  DSI Core 1 TE Input 1

/**
*
*  TECTL Input Modes
*
*****************************************************************************/
#define TE_VC4L_MODE_VSYNC          (0)	// TE MODE vsync       (mode 1)
#define TE_VC4L_MODE_VSYNC_HSYNC    (1)	// TE MODE vsync_hsync (mode 2)

/**
*
*  TECTL Input Polarity for VSYNC & HSYNC
*
*****************************************************************************/
#define TE_VC4L_ACT_POL_LO          (0)	// Active LO ( VSYNC & HSYNC )
#define TE_VC4L_ACT_POL_HI          (1)	// Active HI ( VSYNC & HSYNC )

/**
*
*  TECTL Input Configuration
*
*****************************************************************************/
	typedef struct {
		UInt32 te_mode;	// te mode ( VSYNC or VSYNC & HSYNC )
		UInt32 sync_pol;	// sync polarity
		UInt32 vsync_width;	// vsync_width [tectl_clk_count]
		UInt32 hsync_line;	// hsync_line
	} TECTL_CFG_t, *pTECTL_CFG;

/**
*
*  @brief    Configure Chip TE Input
*
*  @param	 UInt32 teIn    (in) TE Input
*  @param	 UInt32 teOut   (in) TE Mux Output ( TE input of LCD controller )
*  @param	 UInt32 teCfg   (in) TE Configuration
*
*  @return	 Int32          (out) -1 failure  != -1 OK
*
*  @note     
*
*****************************************************************************/
	Int32 CSL_TECTL_VC4L_OpenInput(UInt32 teIn, UInt32 teOut,
				       pTECTL_CFG teCfg);

/**
*
*  @brief    Close Chip TE Input
*
*  @param	 UInt32 teIn    (in) TE Input
*
*  @return	 Int32          (out) -1 failure  != -1 OK
*
*  @note     
*
*****************************************************************************/
	Int32 CSL_TECTL_VC4L_CloseInput(UInt32 teIn);

/** @} */

#ifdef __cplusplus
}
#endif
#endif				// _CSL_TECTL_VC4LITE_H_
