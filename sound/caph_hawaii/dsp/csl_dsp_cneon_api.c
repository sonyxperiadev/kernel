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
*  @file   csl_dsp_cneon_api.c
*
*  @brief  This file contains CSL DSP APIs for customer IPs
*
*  @note
*
*****************************************************************************/
#include <string.h>
#include "mobcom_types.h"
#include "shared.h"
#include "csl_dsp.h"
#include "csl_apcmd.h"
#include "csl_dsp_cneon_api.h"

static dummy3_queries_t spStatus;

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
void CSL_DSP_CNEON_AUDIO_CNC_Set_Emergency_Flag(UInt16 value)
{
	vp_shared_mem->shared_cnc_emergency_flag = value;
}

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
void CSL_DSP_CNEON_AUDIO_CNC_Set_Init_Flag(UInt16 value)
{
	vp_shared_mem->shared_cnc_init_flag = value;
}

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
void CSL_DSP_CNEON_AUDIO_CNC_Enable(void)
{
	VPCmdQ_t msg;

	msg.cmd = VP_COMMAND_CNC_ENABLE;
	msg.arg0 = 1;
	msg.arg1 = 0;
	msg.arg2 = 0;
	VPSHAREDMEM_PostCmdQ(&msg);
}

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
void CSL_DSP_CNEON_AUDIO_CNC_Disable(void)
{
	VPCmdQ_t msg;

	msg.cmd = VP_COMMAND_CNC_ENABLE;
	msg.arg0 = 0;
	msg.arg1 = 0;
	msg.arg2 = 0;
	VPSHAREDMEM_PostCmdQ(&msg);
}

void sp_StatusUpdate(void)
{
	memcpy(&spStatus, &vp_shared_mem->shared_SP_params_left,
	       sizeof(dummy3_queries_t));

}

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
			   UInt32 *sp_config_struct, UInt32 *sp_vars_struct)
{
	if (Init) {
		memcpy(&vp_shared_mem->shared_SP_left_config, sp_config_struct,
		       sizeof(dummy2_config_t));

		memcpy(&vp_shared_mem->shared_SP_input, sp_vars_struct,
		       sizeof(dummy4_input_t));

	}

	VPRIPCMDQ_SP(control, mode, Init);

	return 0;

}

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
void csl_dsp_sp_ctrl_msg(UInt32 *sp_vars_struct)
{

	memcpy(&vp_shared_mem->shared_SP_input, sp_vars_struct,
	       sizeof(dummy4_input_t));

}

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
void csl_dsp_sp_query_msg(UInt32 *query)
{
	memcpy(query, &spStatus, sizeof(dummy3_queries_t));

}
