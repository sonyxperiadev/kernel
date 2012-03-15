/******************************************************************************
*
* Copyright 2009 - 2012  Broadcom Corporation
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2 (the GPL),
*  available at
*
*      http://www.broadcom.com/licenses/GPLv2.php
*
*  with the following added to such license:
*
*  As a special exception, the copyright holders of this software give you
*  permission to link this software with independent modules, and to copy and
*  distribute the resulting executable under terms of your choice, provided
*  that you also meet, for each linked independent module, the terms and
*  conditions of the license of that module.
*  An independent module is a module which is not derived from this software.
*  The special exception does not apply to any modifications of the software.
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
******************************************************************************/
/**
*
*  @file   csl_dsp_caph_control_api.c
*
*  @brief  This file contains CSL DSP API functions for control of CAPH (related to DSP)
*
*  @note
*
*****************************************************************************/
#include <string.h>
#include "mobcom_types.h"
#include "shared.h"
#include "osdw_dsp_drv.h"
#include "csl_dsp.h"
#include "csl_dsp_caph_control_api.h"
#include "audio_trace.h"


/*****************************************************************************/
/**
*
* Function Name: csl_dsp_caph_control_get_aadmac_buf_base_addr
*
*   @note     This function returns the base address of the AADMAC buffer's base address
*             (pointing to the start of the Low part of the AADMAC buffer).
*
*   @param    value (DSP_AADMAC_Audio_Connections_t)  Return the base address for which
*                                                     AADMAC buffer.
*
*   @return   UInt32 *dsp_aadmac_base_addr Base address of the AADMAC buffer.
*
*****************************************************************************/
UInt32 *csl_dsp_caph_control_get_aadmac_buf_base_addr(DSP_AADMAC_Audio_Connections_t
			aadmac_audio_connection)
{
	UInt32 *base_addr;
	AP_SharedMem_t *ap_shared_mem_ptr;
	ap_shared_mem_ptr = DSPDRV_GetPhysicalSharedMemoryAddress();

	switch (aadmac_audio_connection) {
	case DSP_AADMAC_PRI_MIC_EN:
		base_addr = &(ap_shared_mem_ptr->shared_aadmac_pri_mic_low[0]);
		break;
	case DSP_AADMAC_SEC_MIC_EN:
		base_addr = &(ap_shared_mem_ptr->shared_aadmac_sec_mic_low[0]);
		break;
	case DSP_AADMAC_SPKR_EN:
		base_addr = &(ap_shared_mem_ptr->shared_aadmac_spkr_low[0]);
		break;
	default:
		/* Assert */
		assert(0);
		break;
	}

	return base_addr;

}

/*****************************************************************************/
/**
*
* Function Name: csl_dsp_caph_control_aadmac_set_samp_rate
*
*   @note     This function sets the sample rate for the AADMAC based audio for the DSP
*             (not valid for IHF)
*
*   @param    value (UInt16)  Sample rate for AADMAC based audio for the DSP (non-IHF)
*             = 8000 or 16000
*
*   @return   None
*
*****************************************************************************/
void csl_dsp_caph_control_aadmac_set_samp_rate(UInt16 value)
{
	if ((value != 8000) && (value != 16000)) {
		/* Assert */
		assert(0);
	} else {
		vp_shared_mem->shared_aadmac_audio_samp_rate = value;
	}
}

/*****************************************************************************/
/**
*
* Function Name: csl_dsp_caph_control_aadmac_enable_path
*
*   @note     This function informs the DSP about which hardware path is enabled, and
*             based on this information, DSP processes the audio interrupt.
*             (not valid for IHF)
*
*   @param    path (DSP_AADMAC_Audio_Connections_t)
*
*   @return   None
*
*****************************************************************************/
void csl_dsp_caph_control_aadmac_enable_path(UInt16 path)
{
	if (path & (UInt16) (~(DSP_AADMAC_PRI_MIC_EN | DSP_AADMAC_SEC_MIC_EN |
			       DSP_AADMAC_SPKR_EN | DSP_AADMAC_IHF_SPKR_EN))) {
		/* Assert */
		assert(0);
	} else {

		vp_shared_mem->shared_aadmac_aud_enable |= path;
		aTrace(LOG_AUDIO_DSP, "\n\r\t*"
		       "csl_dsp_caph_control_aadmac_enable_path: vp_shared_mem = %lx,"
		       "&(vp_shared_mem->shared_aadmac_aud_enable) =%lx,"
		       "shared_aadmac_aud_enable = %x*\n\r",
		       (UInt32) vp_shared_mem,
		       (UInt32) (&(vp_shared_mem->shared_aadmac_aud_enable)),
		       vp_shared_mem->shared_aadmac_aud_enable);

	}

}

/*****************************************************************************/
/**
*
* Function Name: csl_dsp_caph_control_aadmac_disable_path
*
*   @note     This function informs the DSP about which hardware path is disabled, and
*             based on this information, DSP processes the audio interrupt.
*             (not valid for IHF)
*
*   @param    path (DSP_AADMAC_Audio_Connections_t)
*
*   @return   None
*
*****************************************************************************/
void csl_dsp_caph_control_aadmac_disable_path(UInt16 path)
{
	if (path & (UInt16) (~(DSP_AADMAC_PRI_MIC_EN | DSP_AADMAC_SEC_MIC_EN |
			       DSP_AADMAC_SPKR_EN | DSP_AADMAC_IHF_SPKR_EN))) {
		/* Assert */
		assert(0);
	} else {
		vp_shared_mem->shared_aadmac_aud_enable &= ~path;
		aTrace(LOG_AUDIO_DSP, "\n\r\t*"
		       "csl_dsp_caph_control_aadmac_disable_path vp_shared_mem = %lx,"
		       "&(vp_shared_mem->shared_aadmac_aud_enable) =%lx,"
		       "shared_aadmac_aud_enable = %x*\n\r",
		       (UInt32) vp_shared_mem,
		       (UInt32) (&(vp_shared_mem->shared_aadmac_aud_enable)),
		       vp_shared_mem->shared_aadmac_aud_enable);

	}

}
