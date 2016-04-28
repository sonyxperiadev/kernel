/*****************************************************************************
*  Copyright 2001 - 2008 Broadcom Corporation.  All rights reserved.
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2, available at
*  http://www.gnu.org/licenses/old-license/gpl-2.0.html (the "GPL").
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
*****************************************************************************/
/**
*
*  @file  chal_tectl_vc4lite.h
*
*  @brief MM TECTL cHAL interface
*
*  @note  HERA
*
*****************************************************************************/

/* Requires the following header files before its inclusion in a c file
#include "chal_common.h"
*/

#ifndef __CHAL_TECTL_VC4LITE_H__
#define __CHAL_TECTL_VC4LITE_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup cHAL_Interface 
 * @{
 */

// Chip TE Input Pins
#define TE_VC4L_IN_0                (1)	//  HERA's LCD_TE  input
#define TE_VC4L_IN_1                (2)	//  HERA's DSI0_TE input
#define TE_VC4L_IN_2                (3)	//  HERA's DSI1_TE input
#define TE_VC4L_IN_DISABLE          (0)	//  DISABLE_MUX

// OUTPUTs of TE CTL Module
#define TE_VC4L_OUT_SPI             (0)	//  SPI Core
#define TE_VC4L_OUT_SMI             (1)	//  SMI Core
#define TE_VC4L_OUT_DSI0_TE0        (2)	//  DSI Core 0 TE Input 0
#define TE_VC4L_OUT_DSI0_TE1        (3)	//  DSI Core 0 TE Input 1
#define TE_VC4L_OUT_DSI1_TE0        (4)	//  DSI Core 1 TE Input 0
#define TE_VC4L_OUT_DSI1_TE1        (5)	//  DSI Core 1 TE Input 1

// TE Input Configuration Types
#define TE_VC4L_MODE_VSYNC          (0)	// TE MODE vsync       (mode 1)
#define TE_VC4L_MODE_VSYNC_HSYNC    (1)	// TE MODE vsync_hsync (mode 2)
#define TE_VC4L_ACT_POL_LO          (0)	// TE SYNC POLARITY ( VSYNC & HSYNC )
#define TE_VC4L_ACT_POL_HI          (1)	// TE SYNC POLARITY ( VSYNC & HSYNC )

	typedef struct {
		cUInt32 te_mode;	// 0=vsync  (mode1)    1=vsync_hsync (mode 2)
		cUInt32 sync_pol;	// 0=active_lo  1=active_hi  ( VSYNC & HSYNC )
		cUInt32 vsync_width;	// TE_VSWIDTH - WIDTH of VSYNC in number of clk_tectl clock periods (oscillator clock period).
		// only in mode 2      mode_vh_sync_vsync_width_tectl_clk_count
		cUInt32 hsync_line;	// only in mode 2      mode_vh_sync_hsync_line
	} CHAL_TE_CFG_t, *pCHAL_TE_CFG;

	cInt32 chal_te_set_mux_out(cUInt32 te_in, cUInt32 te_out);
	cInt32 chal_te_cfg_input(cUInt32 te_in, pCHAL_TE_CFG teCfg);

/** @} */

#ifdef __cplusplus
}
#endif
#endif				// __CHAL_TECTL_VC4LITE_H__
