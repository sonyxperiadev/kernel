/****************************************************************************/
/*     Copyright 2009, 2010 Broadcom Corporation.  All rights reserved.     */
/*     Unless you and Broadcom execute a separate written software license  */
/*	   agreement governing                                              */
/*     use of this software, this software is licensed to you under the     */
/*	   terms of the GNU General Public License version 2 (the GPL),     */
/*     available at                                                         */
/*                                                                          */
/*          http://www.broadcom.com/licenses/GPLv2.php                      */
/*                                                                          */
/*     with the following added to such license:                            */
/*                                                                          */
/*     As a special exception, the copyright holders of this software give  */
/*     you permission to link this software with independent modules, and   */
/*     to copy and distribute the resulting executable under terms of your  */
/*     choice, provided that you also meet, for each linked independent     */
/*     module, the terms and conditions of the license of that module.      */
/*     An independent module is a module which is not derived from this     */
/*     software.  The special exception does not apply to any modifications */
/*     of the software.                                                     */
/*                                                                          */
/*     Notwithstanding the above, under no circumstances may you combine    */
/*     this software in any way with any other Broadcom software provided   */
/*     under a license other than the GPL, without Broadcom's express prior */
/*     written consent.                                                     */
/*                                                                          */
/****************************************************************************/

/**
*
*  @file   csl_caph_switch.c
*
*  @brief  csl layer driver for caph SSASW driver
*
****************************************************************************/

#include "resultcode.h"
#include "mobcom_types.h"
#include "chal_caph_switch.h"
#include "csl_caph.h"
#include "csl_caph_audioh.h"
#include "csl_caph_cfifo.h"
#include "csl_caph_srcmixer.h"
#include "csl_caph_switch.h"
#include "audio_trace.h"

/***************************************************************************/
/*                       G L O B A L   S E C T I O N                       */
/***************************************************************************/

/**************************************************************************/
/* global variable definitions                                            */
/**************************************************************************/

/***************************************************************************/
/*                        L O C A L   S E C T I O N                        */
/***************************************************************************/

/***************************************************************************/
/*local macro declarations                                                 */
/***************************************************************************/

/***************************************************************************/
/*local typedef declarations                                               */
/***************************************************************************/

struct CSL_CAPH_SWITCH_DST_STATUS_t {
	CAPH_SWITCH_CHNL_e chal_chnl;
	CAPH_DST_STATUS_e dstStatus;
} CSL_CAPH_SWITCH_DST_STATUS_t;

/***************************************************************************/
/*local variable definitions                                               */
/***************************************************************************/

static CHAL_HANDLE handle = 0x0;

static struct CSL_CAPH_SWITCH_DST_STATUS_t dstStatusTable[CAPH_SWITCH_CHNL_NUM];

/***************************************************************************/
/*local function declarations                                              */
/***************************************************************************/

static void csl_caph_switch_initDSTStatus(void);
static CAPH_DST_STATUS_e csl_caph_switch_getDSTStatus(
		CAPH_SWITCH_CHNL_e	chal_chnl);
static void csl_caph_switch_setDSTStatus(CAPH_SWITCH_CHNL_e chal_chnl,
		CAPH_DST_STATUS_e dstStatus);
static CSL_CAPH_SWITCH_CHNL_e csl_caph_switch_get_cslchnl(
		CAPH_SWITCH_CHNL_e	chal_chnl);
static CAPH_SWITCH_CHNL_e csl_caph_switch_get_chalchnl(
		CSL_CAPH_SWITCH_CHNL_e	chnl);

/***************************************************************************/
/* Global function definitions                                             */
/***************************************************************************/

/****************************************************************************
*  Function Name: void csl_caph_switch_initDSTStatus(void)
*
*  Description: Initialize the status Table of DST fifo of all SWITCH channels
*
****************************************************************************/
static void csl_caph_switch_initDSTStatus(void)
{
	u8 i = 0;
	memset(&dstStatusTable, 0,
	       sizeof(CSL_CAPH_SWITCH_DST_STATUS_t) * CAPH_SWITCH_CHNL_NUM);
	for (i = 0; i < (Int8) CAPH_SWITCH_CHNL_NUM; i++) {
		dstStatusTable[i].chal_chnl =
		    (CAPH_SWITCH_CHNL_e) (0x0001 << i);
	}
	return;
}

/****************************************************************************
*  Function Name: void csl_caph_switch_setDSTStatus(
*                                         CAPH_SWITCH_CHNL_e chal_chnl,
*                                         CAPH_DST_STATUS_e dstStatus)
*
*  Description: Update the status of DST fifo of one SWITCH channel
*
****************************************************************************/
static void csl_caph_switch_setDSTStatus(CAPH_SWITCH_CHNL_e chal_chnl,
					 CAPH_DST_STATUS_e dstStatus)
{
	u8 i = 0;
	for (i = 0; i < (Int8) CAPH_SWITCH_CHNL_NUM; i++) {
		if (dstStatusTable[i].chal_chnl ==
			(CAPH_SWITCH_CHNL_e)chal_chnl) {
			dstStatusTable[i].dstStatus =
				(CAPH_DST_STATUS_e)dstStatus;
			return;
		}
	}
	/* Should not run to here.*/
	audio_xassert(0, chal_chnl);
	return;
}

/****************************************************************************
*  Function Name: CAPH_DST_STATUS_e csl_caph_switch_getDSTStatus(
*                                         CAPH_SWITCH_CHNL_e chal_chnl)
*
*  Description: Get the status of DST fifo of one SWITCH channel
*
****************************************************************************/
static CAPH_DST_STATUS_e csl_caph_switch_getDSTStatus(
			CAPH_SWITCH_CHNL_e	chal_chnl)
{
	u8 i = 0;
	for (i = 0; i < (Int8) CAPH_SWITCH_CHNL_NUM; i++) {
		if (dstStatusTable[i].chal_chnl ==
			(CAPH_SWITCH_CHNL_e)chal_chnl)
			return (CAPH_DST_STATUS_e)dstStatusTable[i].dstStatus;
	}
	/* Should not run to here.*/
	audio_xassert(0, chal_chnl);
	return CAPH_DST_OK;
}

/****************************************************************************
*  Function Name: CSL_CAPH_SWITCH_CHNL_e csl_caph_switch_get_cslchnl(
*                                         CAPH_SWITCH_CHNL_e chal_chnl)
*
*  Description: get the CSL SWITCH channel from CHAL SWITCH channel
*
****************************************************************************/
static CSL_CAPH_SWITCH_CHNL_e csl_caph_switch_get_cslchnl(
			CAPH_SWITCH_CHNL_e	chal_chnl)
{
	CSL_CAPH_SWITCH_CHNL_e chnl = CSL_CAPH_SWITCH_NONE;

	switch (chal_chnl) {
	case CAPH_SWITCH_CH_VOID:
		chnl = CSL_CAPH_SWITCH_NONE;
		break;

	case CAPH_SWITCH_CH1:
		chnl = CSL_CAPH_SWITCH_CH1;
		break;

	case CAPH_SWITCH_CH2:
		chnl = CSL_CAPH_SWITCH_CH2;
		break;

	case CAPH_SWITCH_CH3:
		chnl = CSL_CAPH_SWITCH_CH3;
		break;

	case CAPH_SWITCH_CH4:
		chnl = CSL_CAPH_SWITCH_CH4;
		break;

	case CAPH_SWITCH_CH5:
		chnl = CSL_CAPH_SWITCH_CH5;
		break;

	case CAPH_SWITCH_CH6:
		chnl = CSL_CAPH_SWITCH_CH6;
		break;

	case CAPH_SWITCH_CH7:
		chnl = CSL_CAPH_SWITCH_CH7;
		break;

	case CAPH_SWITCH_CH8:
		chnl = CSL_CAPH_SWITCH_CH8;
		break;

	case CAPH_SWITCH_CH9:
		chnl = CSL_CAPH_SWITCH_CH9;
		break;

	case CAPH_SWITCH_CH10:
		chnl = CSL_CAPH_SWITCH_CH10;
		break;

	case CAPH_SWITCH_CH11:
		chnl = CSL_CAPH_SWITCH_CH11;
		break;

	case CAPH_SWITCH_CH12:
		chnl = CSL_CAPH_SWITCH_CH12;
		break;

	case CAPH_SWITCH_CH13:
		chnl = CSL_CAPH_SWITCH_CH13;
		break;

	case CAPH_SWITCH_CH14:
		chnl = CSL_CAPH_SWITCH_CH14;
		break;

	case CAPH_SWITCH_CH15:
		chnl = CSL_CAPH_SWITCH_CH15;
		break;

	case CAPH_SWITCH_CH16:
		chnl = CSL_CAPH_SWITCH_CH16;
		break;
	default:
		audio_xassert(0, chal_chnl);
	};
	return chnl;
}

/****************************************************************************
*  Function Name: CAPH_SWITCH_CHNL_e csl_caph_switch_get_chalchnl(
*                                         CSL_CAPH_SWITCH_CHNL_e chnl)
*
*  Description: get the CHAL SWITCH channel from CSL SWITCH channel
*
****************************************************************************/
static CAPH_SWITCH_CHNL_e csl_caph_switch_get_chalchnl(
			CSL_CAPH_SWITCH_CHNL_e	chnl)
{
	CAPH_SWITCH_CHNL_e chal_chnl = CAPH_SWITCH_CH_VOID;

	switch (chnl) {
	case CSL_CAPH_SWITCH_NONE:
		chal_chnl = CAPH_SWITCH_CH_VOID;
		break;

	case CSL_CAPH_SWITCH_CH1:
		chal_chnl = CAPH_SWITCH_CH1;
		break;

	case CSL_CAPH_SWITCH_CH2:
		chal_chnl = CAPH_SWITCH_CH2;
		break;

	case CSL_CAPH_SWITCH_CH3:
		chal_chnl = CAPH_SWITCH_CH3;
		break;

	case CSL_CAPH_SWITCH_CH4:
		chal_chnl = CAPH_SWITCH_CH4;
		break;

	case CSL_CAPH_SWITCH_CH5:
		chal_chnl = CAPH_SWITCH_CH5;
		break;

	case CSL_CAPH_SWITCH_CH6:
		chal_chnl = CAPH_SWITCH_CH6;
		break;

	case CSL_CAPH_SWITCH_CH7:
		chal_chnl = CAPH_SWITCH_CH7;
		break;

	case CSL_CAPH_SWITCH_CH8:
		chal_chnl = CAPH_SWITCH_CH8;
		break;

	case CSL_CAPH_SWITCH_CH9:
		chal_chnl = CAPH_SWITCH_CH9;
		break;

	case CSL_CAPH_SWITCH_CH10:
		chal_chnl = CAPH_SWITCH_CH10;
		break;

	case CSL_CAPH_SWITCH_CH11:
		chal_chnl = CAPH_SWITCH_CH11;
		break;

	case CSL_CAPH_SWITCH_CH12:
		chal_chnl = CAPH_SWITCH_CH12;
		break;

	case CSL_CAPH_SWITCH_CH13:
		chal_chnl = CAPH_SWITCH_CH13;
		break;

	case CSL_CAPH_SWITCH_CH14:
		chal_chnl = CAPH_SWITCH_CH14;
		break;

	case CSL_CAPH_SWITCH_CH15:
		chal_chnl = CAPH_SWITCH_CH15;
		break;

	case CSL_CAPH_SWITCH_CH16:
		chal_chnl = CAPH_SWITCH_CH16;
		break;
	default:
		audio_xassert(0, chnl);
	};
	return chal_chnl;
}

/****************************************************************************
*
*  Function Name: void csl_caph_switch_init(cUInt32 baseAddress)
*
*  Description: init CAPH switch block
*
****************************************************************************/
void csl_caph_switch_init(UInt32 baseAddress)
{
	aTrace(LOG_AUDIO_CSL, "csl_caph_switch_init:: ");

	csl_caph_switch_initDSTStatus();
	handle = chal_caph_switch_init(baseAddress);

	aTrace(LOG_AUDIO_CSL,
			      "csl_caph_switch_Init:: baseAddress = 0x%lx\n",
			      baseAddress);
	return;
}

/****************************************************************************
*
*  Function Name: void csl_caph_switch_deinit(void)
*
*  Description: deinit CAPH switch block
*
****************************************************************************/
void csl_caph_switch_deinit(void)
{
	aTrace(LOG_AUDIO_CSL, "csl_caph_switch_deinit:: ");

	chal_caph_switch_deinit(handle);

	return;
}

/****************************************************************************
*
*  Function Name: CSL_CAPH_SWITCH_CHNL_e csl_caph_switch_obtain_channel(void)
*
*  Description: assign CAPH switch channel
*
****************************************************************************/
CSL_CAPH_SWITCH_CHNL_e csl_caph_switch_obtain_channel(void)
{
	CSL_CAPH_SWITCH_CHNL_e chnl = CSL_CAPH_SWITCH_NONE;
	CAPH_SWITCH_CHNL_e chal_chnl = CAPH_SWITCH_CH_VOID;

	chal_chnl = chal_caph_switch_alloc_channel(handle);
	chnl = csl_caph_switch_get_cslchnl(chal_chnl);

	aTrace(LOG_AUDIO_CSL,
			      "csl_caph_switch_obtain_channel:: chnl = 0x%x\n",
			      chnl);

	return chnl;
}

/****************************************************************************
*
*  Function Name: void csl_caph_switch_release_channel(CSL_CAPH_SWITCH_CHNL_e
*                                                                chnl)
*
*  Description: release CAPH switch channel
*
****************************************************************************/
void csl_caph_switch_release_channel(CSL_CAPH_SWITCH_CHNL_e chnl)
{
	CAPH_SWITCH_CHNL_e chal_chnl = CAPH_SWITCH_CH_VOID;
	aTrace(LOG_AUDIO_CSL,
		"csl_caph_switch_release_channel:: chnl = 0x%x\n",
		chnl);

	chal_chnl = csl_caph_switch_get_chalchnl(chnl);
	chal_caph_switch_free_channel(handle, chal_chnl);
	return;
}

/****************************************************************************
*
*  Function Name:CSL_CAPH_SWITCH_STATUS_e csl_caph_switch_config_channel(
*                         CSL_CAPH_SWITCH_CONFIG_t chnl_config)
*
*  Description: configure CAPH switch channel
*
****************************************************************************/
CSL_CAPH_SWITCH_STATUS_e csl_caph_switch_config_channel(CSL_CAPH_SWITCH_CONFIG_t
							chnl_config)
{
	CAPH_SWITCH_CHNL_e chal_chnl = CAPH_SWITCH_CH_VOID;
	CAPH_DATA_FORMAT_e chal_dataFormat = CAPH_MONO_16BIT;
	CAPH_SWITCH_TRIGGER_e chal_trig = CAPH_VOID;
	CAPH_DST_STATUS_e dstStatus = CAPH_DST_OK;
	CSL_CAPH_SWITCH_STATUS_e status = CSL_CAPH_SWITCH_OWNER;

	aTrace(LOG_AUDIO_CSL,
		"csl_caph_switch_config_channel:: chnl = 0x%x, srcAddr = "
		"0x%lx, dstcAddr = 0x%lx, dataFmt = 0x%x, trigger = 0x%x\n",
			      chnl_config.chnl,
			      chnl_config.FIFO_srcAddr,
			      chnl_config.FIFO_dstAddr,
			      chnl_config.dataFmt, chnl_config.trigger);

	/* Get cHAL Channel */
	chal_chnl = csl_caph_switch_get_chalchnl(chnl_config.chnl);
	/* Select Source for this channel */
	chal_caph_switch_select_src(handle, chal_chnl,
				    (UInt16) (chnl_config.FIFO_srcAddr));
	/* Add Dst for this channel */
	dstStatus =
	    chal_caph_switch_add_dst(handle, chal_chnl,
				     (UInt16) (chnl_config.FIFO_dstAddr));
	csl_caph_switch_setDSTStatus(chal_chnl, dstStatus);

	/* If the DST is already being used by other channels
	 * Do not set the switch channel anymore.
	 */
	if (dstStatus != CAPH_DST_OK) {
		audio_xassert(0, dstStatus);
		aWarn("%s::sw %d dst 0x%lx used by other sw\n",
				__func__, chnl_config.chnl,
				chnl_config.FIFO_dstAddr);
		chal_caph_switch_free_channel(handle, chal_chnl);
		status = CSL_CAPH_SWITCH_BORROWER;
		return status;
	}
	/* Set data format */
	switch (chnl_config.dataFmt) {
	case CSL_CAPH_16BIT_MONO:
		chal_dataFormat = CAPH_MONO_16BIT;
		break;
	case CSL_CAPH_16BIT_STEREO:
		chal_dataFormat = CAPH_STEREO_16BIT;
		break;
	case CSL_CAPH_24BIT_MONO:
		chal_dataFormat = CAPH_MONO_24BIT;
		break;
	case CSL_CAPH_24BIT_STEREO:
		chal_dataFormat = CAPH_STEREO_24BIT;
		break;
	default:
		audio_xassert(0, chnl_config.dataFmt);
	}
	chal_caph_switch_set_datafmt(handle, chal_chnl, chal_dataFormat);
	/* Select the trigger */
	chal_trig = chnl_config.trigger;
	chal_caph_switch_select_trigger(handle, chal_chnl, chal_trig);

	return status;
}

/****************************************************************************
*
*  Function Name: void csl_caph_switch_add_dst(CSL_CAPH_SWITCH_CHNL_e chnl,
*                                              UInt32 FIFO_dstAddr)
*
*  Description: Add one destination to a CAPH switch channel
*
****************************************************************************/
void csl_caph_switch_add_dst(CSL_CAPH_SWITCH_CHNL_e chnl, UInt32 FIFO_dstAddr)
{
	CAPH_SWITCH_CHNL_e chal_chnl = CAPH_SWITCH_CH_VOID;

	aTrace(LOG_AUDIO_CSL, "csl_caph_switch_add_dst:: ");
	aTrace(LOG_AUDIO_CSL,
		"csl_caph_switch_add_dst:: chnl = 0x%x, dstcAddr = 0x%lx\n",
		chnl, FIFO_dstAddr);
	/* Get cHAL Channel */
	chal_chnl = csl_caph_switch_get_chalchnl(chnl);
	/* Add one more destination for this channel */
	if (CAPH_DST_OK !=
			chal_caph_switch_add_dst(handle, chal_chnl,
				(UInt16) FIFO_dstAddr))
		aError("csl_caph_switch_add_dst:: FAIL\n");

}

/****************************************************************************
*
*  Function Name: void csl_caph_switch_remove_dst(CSL_CAPH_SWITCH_CHNL_e chnl,
*                                               UInt32 FIFO_dstAddr)
*
*  Description: Remove one destination from a CAPH switch channel
*
****************************************************************************/
void csl_caph_switch_remove_dst(CSL_CAPH_SWITCH_CHNL_e chnl,
				UInt32 FIFO_dstAddr)
{
	CAPH_SWITCH_CHNL_e chal_chnl = CAPH_SWITCH_CH_VOID;

	if (FIFO_dstAddr == 0)
		return;
	aTrace(LOG_AUDIO_CSL,
		"csl_caph_switch_remove_dst:: chnl = 0x%x, dstcAddr = 0x%lx\n",
		chnl, FIFO_dstAddr);
	/* Get cHAL Channel */
	chal_chnl = csl_caph_switch_get_chalchnl(chnl);
	/* Remove one destination for this channel */
	chal_caph_switch_remove_dst(handle, chal_chnl, (UInt16) FIFO_dstAddr);

	return;
}

/****************************************************************************
*
*  Function Name: void csl_caph_switch_start_transfer(CSL_CAPH_SWITCH_CHNL_e
*                                                               chnl)
*
*  Description: start the channel
*
****************************************************************************/
void csl_caph_switch_start_transfer(CSL_CAPH_SWITCH_CHNL_e chnl)
{
	CAPH_SWITCH_CHNL_e chal_chnl = CAPH_SWITCH_CH_VOID;

	aTrace(LOG_AUDIO_CSL,
			      "csl_caph_switch_start_transfer:: chnl = 0x%x\n",
			      chnl);
	/* Get cHAL Channel */
	chal_chnl = csl_caph_switch_get_chalchnl(chnl);
	/* Check whether DST of the switch channel is defined properly */
	if (csl_caph_switch_getDSTStatus(chal_chnl) != CAPH_DST_OK) {
		/* Clear the status table and return without enabling */
		csl_caph_switch_setDSTStatus(chal_chnl, CAPH_DST_OK);
		return;
	}
	/* Start this channel */
	chal_caph_switch_enable(handle, chal_chnl);

	return;
}

/****************************************************************************
*
*  Function Name: void csl_caph_switch_stop_transfer(CSL_CAPH_SWITCH_CHNL_e
*                                                                    chnl)
*
*  Description: stop the channel
*
****************************************************************************/
void csl_caph_switch_stop_transfer(CSL_CAPH_SWITCH_CHNL_e chnl)
{
	CAPH_SWITCH_CHNL_e chal_chnl = CAPH_SWITCH_CH_VOID;

	aTrace
	      (LOG_AUDIO_CSL, "csl_caph_switch_stop_transfer:: ");
	aTrace
	      (LOG_AUDIO_CSL, "csl_caph_switch_stop_transfer:: chnl = 0x%x",
	       chnl);
	/* Get cHAL Channel */
	chal_chnl = csl_caph_switch_get_chalchnl(chnl);
	/* Stop this channel */
	chal_caph_switch_disable(handle, chal_chnl);
	return;
}

/****************************************************************************
*
*  Function Name: csl_caph_switch_enable_clock
*
*  Description: enable/disable NOC global bit
*
****************************************************************************/
void csl_caph_switch_enable_clock(int enable)
{
	aTrace(LOG_AUDIO_CSL, "%s %d\n", __func__, enable);
	if (enable)
		chal_caph_switch_enable_clock(handle);
	else
		chal_caph_switch_disable_clock(handle);
	return;
}

/****************************************************************************
*
*  Function Name: csl_caph_switch_obtain_given_channel
*
*  Description: assign a given CAPH switch channel
*
****************************************************************************/
CSL_CAPH_SWITCH_CHNL_e csl_caph_switch_obtain_given_channel(
		CSL_CAPH_SWITCH_CHNL_e chnl)
{
	CSL_CAPH_SWITCH_CHNL_e chnl2;
	CAPH_SWITCH_CHNL_e chal_chnl;

	chal_chnl = csl_caph_switch_get_chalchnl(chnl);
	chal_chnl = chal_caph_switch_alloc_given_channel(handle, chal_chnl);
	chnl2 = csl_caph_switch_get_cslchnl(chal_chnl);

	aTrace(LOG_AUDIO_CSL, "%s ask %d get %d\n", __func__, chnl, chnl2);
	return chnl2;
}
