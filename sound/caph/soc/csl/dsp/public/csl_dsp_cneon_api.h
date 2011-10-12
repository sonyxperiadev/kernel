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
*  @file   csl_dsp_cneon_api.h
*
*  @brief  This file contains CSL DSP APIs for customer IPs
*
*  @note   
*
**/
/*******************************************************************************************/
#ifndef _CSL_DSP_CNEON_API_H_
#define _CSL_DSP_CNEON_API_H_

// ---- Include Files -------------------------------------------------------
#include "mobcom_types.h"


/**
 * \defgroup CSL_DSP_CNEON_Interface
 * @{
 */

// ---- Function Declarations -----------------------------------------

/*****************************************************************************************/
/**
* 
* Function Name: CSL_DSP_CNEON_AUDIO_CNC_Set_Emergency_Flag
*
*   @note     This function sets the Emergency flag for Click Noise Cancellation.
*                                                                                         
*   @param    value (UInt16)  Value to be set to emergency flag for CNC.
*
*   @return   None
*
**/
/*******************************************************************************************/
void CSL_DSP_CNEON_AUDIO_CNC_Set_Emergency_Flag(UInt16 value);

/*****************************************************************************************/
/**
* 
* Function Name: CSL_DSP_CNEON_AUDIO_CNC_Set_Init_Flag
*
*   @note     This function sets the Init flag for Click Noise Cancellation.
*                                                                                         
*   @param    value (UInt16)  Value to be set to init flag for CNC.
*
*   @return   None
*
**/
/*******************************************************************************************/
void CSL_DSP_CNEON_AUDIO_CNC_Set_Init_Flag(UInt16 value);

/*****************************************************************************************/
/**
* 
* Function Name: CSL_DSP_CNEON_AUDIO_CNC_Enable
*
*   @note     This function enables Click Noise Cancellation.
*                                                                                         
*   @param    None
*
*   @return   None
*
**/
/*******************************************************************************************/
void CSL_DSP_CNEON_AUDIO_CNC_Enable(void);

/*****************************************************************************************/
/**
* 
* Function Name: CSL_DSP_CNEON_AUDIO_CNC_Disable
*
*   @note     This function disables Click Noise Cancellation.
*                                                                                         
*   @param    None
*
*   @return   None
*
**/
/*******************************************************************************************/
void CSL_DSP_CNEON_AUDIO_CNC_Disable(void);

/*****************************************************************************************/
/**
* 
* Function Name: csl_dsp_sp_cnfg_msg
*
*   @note     This function configures speaker protection.
*                                                                                         
*   @param    control	0/1 - disable/enable 
*   @param    mode		0	- mono (stereo is not supported)  
*   @param    Init		0/1 - no initialization/initializes the internal parameters and take new configuration
*   @param    sp_config_struct	configuration structure 
*   @param    sp_config_struct	variables structure  
*
*   @return	  0/1 - pass/failed 
*
**/
/*******************************************************************************************/
UInt16 csl_dsp_sp_cnfg_msg(UInt16 control, UInt16 mode, UInt16 Init, UInt32 *sp_config_struct, UInt32 *sp_vars_struct);

/*****************************************************************************************/
/**
* 
* Function Name: csl_dsp_sp_ctrl_msg
*
*   @note     This function controls speaker protection.
*                                                                                         
*   @param    sp_config_struct	variables structure  
*
*
**/
/*******************************************************************************************/
void csl_dsp_sp_ctrl_msg(UInt32 *sp_vars_struct);

/*****************************************************************************************/
/**
* 
* Function Name: csl_dsp_sp_query_msg
*
*   @note     This function returs speaker protection status.
*                                                                                         
*   @param    query		status structure  
*
**/
/*******************************************************************************************/
void csl_dsp_sp_query_msg(UInt32 *query);

#endif //_CSL_DSP_CNEON_API_H_
