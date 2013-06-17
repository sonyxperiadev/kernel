/******************************************************************************/
/* Copyright 2009, 2010 Broadcom Corporation.  All rights reserved.           */
/*     Unless you and Broadcom execute a separate written software license    */
/*     agreement governing use of this software, this software is licensed to */
/*     you under the terms of the GNU General Public License version 2        */
/*    (the GPL), available at                                                 */
/*                                                                            */
/*          http://www.broadcom.com/licenses/GPLv2.php                        */
/*                                                                            */
/*     with the following added to such license:                              */
/*                                                                            */
/*     As a special exception, the copyright holders of this software give    */
/*     you permission to link this software with independent modules, and to  */
/*     copy and distribute the resulting executable under terms of your       */
/*     choice, provided that you also meet, for each linked independent       */
/*     module, the terms and conditions of the license of that module.        */
/*     An independent module is a module which is not derived from this       */
/*     software.  The special exception does not apply to any modifications   */
/*     of the software.                                                       */
/*                                                                            */
/*     Notwithstanding the above, under no circumstances may you combine this */
/*     software in any way with any other Broadcom software provided under a  */
/*     license other than the GPL, without Broadcom's express prior written   */
/*     consent.                                                               */
/******************************************************************************/

/**
*
*  @file   caph_i2s.h
*
*
****************************************************************************/


#ifndef _CAPH_I2S_H
#define _CAPH_I2S_H

#include <linux/clk.h>
#include "csl_caph_i2s_sspi.h"
#include "chal_caph.h"
#include "caph-pcm.h"
#include "csl_caph_switch.h"

/* cpu_dai(i2s_dai) private driver data structure*/
struct caph_i2s {
	struct resource *mem;
	void __iomem *base;
	dma_addr_t phys_base;

	struct clk *clk_aic;
	struct clk *clk_i2s;

	CSL_HANDLE fmHandleSSP;
	CAPH_SWITCH_TRIGGER_e fmTxTrigger;
	CAPH_SWITCH_TRIGGER_e fmRxTrigger;
	CSL_I2S_CONFIG_t fmCfg;
	Boolean fmTxRunning;
	Boolean fmRxRunning;

	struct caph_pcm_config pcm_config_playback;
	struct caph_pcm_config pcm_config_capture;
};

/*****************************************************************************
*
*Function Name: void ssp_ControlHWClock(Boolean enable)
*
*Description: This is to enable/disable SSP clock
*
*****************************************************************************/
void ssp_ControlHWClock(Boolean);
#endif
