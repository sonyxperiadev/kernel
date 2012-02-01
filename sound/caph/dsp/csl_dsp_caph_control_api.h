//*********************************************************************
//
// (c)1999-2011 Broadcom Corporation
//
// Unless you and Broadcom execute a separate written software license agreement governing use of this software,
// this software is licensed to you under the terms of the GNU General Public License version 2,
// available at http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
//
//*********************************************************************
/**
*
*  @file   csl_dsp_caph_control_api.c
*
*  @brief  This file contains CSL DSP APIs for control of CAPH (related to DSP)
*
*  @note   
*
**/
/*******************************************************************************************/
#ifndef _CSL_DSP_CAPH_CONTROL_API_H_
#define _CSL_DSP_CAPH_CONTROL_API_H_

// ---- Include Files -------------------------------------------------------
#include "mobcom_types.h"


/**
 * \defgroup CSL_DSP_CAPH_CONTROL_API
 * @{
 */

#define NUM_8kHz_AUDIO_SAMPLES_PER_DSP_INT  (8)
#define NUM_16kHz_AUDIO_SAMPLES_PER_DSP_INT (NUM_8kHz_AUDIO_SAMPLES_PER_DSP_INT*2)

// ---- Typedef Declarations -----------------------------------------
typedef enum
{
    DSP_AADMAC_PRI_MIC_EN = 0x10,
    DSP_AADMAC_SEC_MIC_EN = 0x20,
    DSP_AADMAC_SPKR_EN = 0x40,
    DSP_AADMAC_IHF_SPKR_EN = 0x80,
    DSP_AADMAC_LEG_IHF_SPKR_EN = 0x100
} DSP_AADMAC_Audio_Connections_t;

// ---- Function Declarations -----------------------------------------

/*****************************************************************************************/
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
**/
/*******************************************************************************************/
extern UInt32 * csl_dsp_caph_control_get_aadmac_buf_base_addr(DSP_AADMAC_Audio_Connections_t aadmac_audio_connection);

/*****************************************************************************************/
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
**/
/*******************************************************************************************/
extern void csl_dsp_caph_control_aadmac_set_samp_rate(UInt16 value);

/*****************************************************************************************/
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
**/
/*******************************************************************************************/
extern void csl_dsp_caph_control_aadmac_enable_path(UInt16 path);

/*****************************************************************************************/
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
**/
/*******************************************************************************************/
extern void csl_dsp_caph_control_aadmac_disable_path(UInt16 path);

#endif //_CSL_DSP_CAPH_CONTROL_API_H_

