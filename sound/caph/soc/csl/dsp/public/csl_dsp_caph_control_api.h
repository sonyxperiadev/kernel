/*******************************************************************************************
Copyright 2011 Broadcom Corporation.  All rights reserved.
This program is the proprietary software of Broadcom Corporation and/or its licensors, and 
may only be used, duplicated, modified or distributed pursuant to the terms and conditions 
of a separate, written license agreement executed between you and Broadcom (an "Authorized 
License").

Except as set forth in an Authorized License, Broadcom grants no license(express or 
implied), right to use, or waiver of any kind with respect to the Software, and Broadcom 
expressly reserves all rights in and to the Software and all intellectual property rights 
therein. IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN 
ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
  
 Except as expressly set forth in the Authorized License,
1. This program, including its structure, sequence and organization, constitutes the 
valuable trade secrets of Broadcom, and you shall use all reasonable efforts to protect 
the confidentiality thereof, and to use this information only in connection with your use 
of Broadcom integrated circuit products.

2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" AND WITH ALL 
FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, 
IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY 
DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, 
FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET 
ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK
ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.

3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS LICENSORS BE 
LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER
ARISING OUT OF OR IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN 
IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN 
EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER.
THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY 
LIMITED REMEDY.
*******************************************************************************************/

/*******************************************************************************************/
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

