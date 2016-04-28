/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*             @file     arch/arm/plat-kona/csl/csl_tectl_vc4lite.c
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
*  @file   csl_tectl_vc4lite.c
*
*  @brief  TE (tearing) Control CSL driver
*
****************************************************************************/
#if (defined(_RHEA_))

// use PMUX_DRV for enabling TE inputs, otherwise set MUX Control Directly
//#define __USE_PMUX_DRV__

#include "linux/broadcom/mobcom_types.h"
#include "plat/chal/chal_types.h"
#include "linux/broadcom/msconsts.h"

//#include "plat/osabstract/ostypes.h"
//#include "sio.h"
#include "plat/chal/chal_common.h"
#include "plat/chal/chal_tectl_vc4lite.h"
#include "plat/csl/csl_lcd.h"
#include "plat/csl/csl_tectl_vc4lite.h"
#ifdef __USE_PMUX_DRV__
#include "plat/csl/pmux_drv.h"
#endif
#include <mach/hardware.h>
//#include "dbg.h"
//#include "logapi.h"

#define TE_IN_COUNT     (3)

// Local CSL Config
typedef struct {
	cBool inUse;
	UInt32 teIn_cHal;
	UInt32 teOut;
	CHAL_TE_CFG_t teCfg;
} CSL_TE_CFG_t, *pCSL_TE_CFG;

static CSL_TE_CFG_t teCslCfg[TE_IN_COUNT];

static Int32 csl_tectl_vc4l_CfgInput(UInt32 teIn, pTECTL_CFG teCfg);

//*****************************************************************************
//
// Function Name:  CSL_TECTL_VC4L_CloseInput
// 
// Description:    'Close' TE Input
//                 
//*****************************************************************************
Int32 CSL_TECTL_VC4L_CloseInput(UInt32 teIn)
{

	if (teIn >= TE_IN_COUNT) {
		LCD_DBG(LCD_DBG_ID, "[CSL TECTL] CSL_TECTL_VC4L_CloseInput: "
			"ERROR Invalid TE Input Specified \r\n");
		return (-1);
	}

	if (!teCslCfg[teIn].inUse) {
		LCD_DBG(LCD_DBG_ID, "[CSL TECTL] CSL_TECTL_VC4L_CloseInput: "
			"WARNING TE Input Not Configured \r\n");
		return (-1);
	}
	// disable TE MUX Output 
	chal_te_set_mux_out(TE_VC4L_IN_DISABLE, teCslCfg[teIn].teOut);
	teCslCfg[teIn].inUse = FALSE;

	return (0);
}

//*****************************************************************************
//
// Function Name:  CSL_TECTL_VC4L_CfgInput
// 
// Description:    Configure TE Input
//                 
//*****************************************************************************
static Int32 csl_tectl_vc4l_CfgInput(UInt32 teIn, pTECTL_CFG teCfg)
{
	Int32 res = 0;

	if (teIn >= TE_IN_COUNT) {
		LCD_DBG(LCD_DBG_ID, "[CSL TECTL] csl_tectl_vc4l_CfgInput: "
			"ERROR Invalid TE Input \r\n");
		return (-1);
	}
#if 0
	if (teCslCfg[teIn].inUse) {
		LCD_DBG(LCD_DBG_ID, "[CSL TECTL] csl_tectl_vc4l_CfgInput: "
			"ERROR TE Input Already In Use \r\n");
		return (-1);
	}
#endif

	if (teIn == TE_VC4L_IN_0_LCD) {
		teCslCfg[teIn].teIn_cHal = TE_VC4L_IN_0;	//  HERA's LCD_TE  input
	} else if (teIn == TE_VC4L_IN_1_DSI0) {
		teCslCfg[teIn].teIn_cHal = TE_VC4L_IN_1;	//  HERA's DSI0_TE input
	} else if (teIn == TE_VC4L_IN_2_DSI1) {
		teCslCfg[teIn].teIn_cHal = TE_VC4L_IN_2;	//  HERA's DSI1_TE input
	}

	teCslCfg[teIn].teCfg.hsync_line = teCfg->hsync_line;
	teCslCfg[teIn].teCfg.vsync_width = teCfg->vsync_width;
	teCslCfg[teIn].teCfg.sync_pol = teCfg->sync_pol;
	teCslCfg[teIn].teCfg.te_mode = teCfg->te_mode;

	if ((res = chal_te_cfg_input(teCslCfg[teIn].teIn_cHal,
				     &teCslCfg[teIn].teCfg)) != -1) {
		teCslCfg[teIn].inUse = TRUE;
	} else {
		teCslCfg[teIn].inUse = FALSE;
	}
	return (res);
}

//*****************************************************************************
//
// Function Name:  CSL_TECTL_VC4L_OpenInput
// 
// Description:    'Open' TE Input 
//                 
//*****************************************************************************
Int32 CSL_TECTL_VC4L_OpenInput(UInt32 teIn,	// which TE Input Pin
			       UInt32 teOut,	// TECTL MUX Output ( SMI/SPI/DSI Input Selection )
			       pTECTL_CFG teCfg	// TE Configuration
    )
{

	if (teIn >= TE_IN_COUNT) {
		LCD_DBG(LCD_DBG_ID, "[CSL TECTL] CSL_TECTL_VC4L_OpenInput: "
			"ERROR Invalid TE Input \r\n");
		return (-1);
	}

	if (csl_tectl_vc4l_CfgInput(teIn, teCfg) == -1) {
		return (-1);
	}

	teCslCfg[teIn].teOut = teOut;
	chal_te_set_mux_out(teCslCfg[teIn].teIn_cHal, teOut);

	return (0);
}

#else
#error "[CSL TECTL] ERROR: Available For HERA/RHEA Only!"
#endif // #if (defined(_RHEA_))
