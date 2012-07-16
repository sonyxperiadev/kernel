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
*  @brief  This file contains CSL DSP APIs for control of CAPH (related to DSP)
*
*  @note
*
**/
/*****************************************************************************/
#ifndef _CSL_DSP_CAPH_CONTROL_API_H_
#define _CSL_DSP_CAPH_CONTROL_API_H_

/* ---- Include Files -------------------------------------------------------*/
#include "mobcom_types.h"
#include "shared.h"

extern AP_SharedMem_t *vp_shared_mem;
extern AP_SharedMem_t *DSPDRV_GetPhysicalSharedMemoryAddress(void);

/**
 * \defgroup CSL_DSP_CAPH_CONTROL_API
 * @{
 */
#define NUM_8kHz_AUDIO_SAMPLES_PER_DSP_INT  (8)
#define NUM_16kHz_AUDIO_SAMPLES_PER_DSP_INT (NUM_8kHz_AUDIO_SAMPLES_PER_DSP_INT*2)
/* ---- Typedef Declarations ----------------------------------------- */
typedef enum {
	DSP_AADMAC_PRI_MIC_EN = 0x10,
	DSP_AADMAC_SEC_MIC_EN = 0x20,
	DSP_AADMAC_SPKR_EN = 0x40,
	DSP_AADMAC_IHF_SPKR_EN = 0x80,
	DSP_AADMAC_LEG_IHF_SPKR_EN = 0x100,
	DSP_AADMAC_PACKED_16BIT_IN_OUT_EN = 0x200,
	DSP_AADMAC_RETIRE_DS_CMD = 0x8000
} DSP_AADMAC_Audio_Connections_t;

typedef enum {
	DSP_AADMAC_EXT_MODEM_UL,
	DSP_AADMAC_EXT_MODEM_DL
} DSP_AADMAC_Ext_Modem_Connections_t;

/* ---- Function Declarations ----------------------------------------- */

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
					      aadmac_audio_connection);

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
void csl_dsp_caph_control_aadmac_set_samp_rate(UInt16 value);

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
void csl_dsp_caph_control_aadmac_enable_path(UInt16 path);

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
void csl_dsp_caph_control_aadmac_disable_path(UInt16 path);

/*****************************************************************************/
/**
*
* Function Name: csl_dsp_ext_modem_get_aadmac_buf_base_addr
*
*   @note     This function returns the physical base address of the AADMAC
*             buffer\'s base address (pointing to the start of the Low part
*             of the AADMAC buffer) for external modem interface. AP can use
*             this address to program the base address in AADMACs CR_1
*             register.
*
*   @param    aadmac_ext_modem_audio_connection
*             (DSP_AADMAC_Ext_Modem_Connections_t)
*             Return the base address for which AADMAC buffer (speaker or
*             microphone path) path (DSP_AADMAC_Audio_Connections_t)
*
*   @return   UInt32 *dsp_aadmac_base_addr Base address of the AADMAC buffer.
*
*****************************************************************************/
UInt32 *csl_dsp_ext_modem_get_aadmac_buf_base_addr(
	DSP_AADMAC_Ext_Modem_Connections_t aadmac_ext_modem_audio_connection);

/*****************************************************************************/
/**
*
* Function Name: csl_dsp_caph_control_aadmac_get_enable_path
*
*   @note     This function informs driver about which hardware path is disabled
*             or enabled
*
*   @param    None
*
*   @return   path (DSP_AADMAC_Audio_Connections_t)
*
*****************************************************************************/
UInt16 csl_dsp_caph_control_aadmac_get_enable_path(void);

#endif /* _CSL_DSP_CAPH_CONTROL_API_H_ */
