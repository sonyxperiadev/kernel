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
*  @file   csl_dsp_cneon_api.h
*
*  @brief  This file contains CSL DSP APIs for customer IPs
*
*  @note
*
*****************************************************************************/
#ifndef _CSL_DSP_CNEON_API_H_
#define _CSL_DSP_CNEON_API_H_

/* ---- Include Files ------------------------------------------------------- */
#include "mobcom_types.h"

extern AP_SharedMem_t *vp_shared_mem;
extern void VPSHAREDMEM_PostCmdQ(VPCmdQ_t *cmd_msg);

/**
 * \defgroup CSL_DSP_CNEON_Interface
 * @{
 */

/* ---- Function Declarations ----------------------------------------- */

/*****************************************************************************/
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
*****************************************************************************/
void CSL_DSP_CNEON_AUDIO_CNC_Set_Emergency_Flag(UInt16 value);

/*****************************************************************************/
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
*****************************************************************************/
void CSL_DSP_CNEON_AUDIO_CNC_Set_Init_Flag(UInt16 value);

/*****************************************************************************/
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
*****************************************************************************/
void CSL_DSP_CNEON_AUDIO_CNC_Enable(void);

/*****************************************************************************/
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
*****************************************************************************/
void CSL_DSP_CNEON_AUDIO_CNC_Disable(void);

/*****************************************************************************/
/**
*
* Function Name: csl_dsp_sp_cnfg_msg
*
*   @note     This function configures speaker protection.
*
*   @param    control	0/1 - disable/enable
*   @param    mode		0	- mono (stereo is not supported)
*   @param    Init		0/1 - no initialization/initializes the internal
*	parameters and take new configuration
*   @param    sp_config_struct	configuration structure
*   @param    sp_config_struct	variables structure
*
*   @return	  0/1 - pass/failed
*
*****************************************************************************/
UInt16 csl_dsp_sp_cnfg_msg(UInt16 control, UInt16 mode, UInt16 Init,
			   UInt32 *sp_config_struct, UInt32 *sp_vars_struct);

/*****************************************************************************/
/**
*
* Function Name: csl_dsp_sp_ctrl_msg
*
*   @note     This function controls speaker protection.
*
*   @param    sp_config_struct	variables structure
*
*
*****************************************************************************/
void csl_dsp_sp_ctrl_msg(UInt32 *sp_vars_struct);

/*****************************************************************************/
/**
*
* Function Name: csl_dsp_sp_query_msg
*
*   @note     This function returs speaker protection status.
*
*   @param    query		status structure
*
*****************************************************************************/
void csl_dsp_sp_query_msg(UInt32 *query);

#endif /* _CSL_DSP_CNEON_API_H_ */
