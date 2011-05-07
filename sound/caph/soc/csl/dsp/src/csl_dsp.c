//*********************************************************************
//
//	Copyright © 2000-2010 Broadcom Corporation
//
//	This program is the proprietary software of Broadcom Corporation
//	and/or its licensors, and may only be used, duplicated, modified
//	or distributed pursuant to the terms and conditions of a separate,
//	written license agreement executed between you and Broadcom (an
//	"Authorized License").  Except as set forth in an Authorized
//	License, Broadcom grants no license (express or implied), right
//	to use, or waiver of any kind with respect to the Software, and
//	Broadcom expressly reserves all rights in and to the Software and
//	all intellectual property rights therein.  IF YOU HAVE NO
//	AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE
//	IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE
//	ALL USE OF THE SOFTWARE.
//
//	Except as expressly set forth in the Authorized License,
//
//	1.	This program, including its structure, sequence and
//		organization, constitutes the valuable trade secrets
//		of Broadcom, and you shall use all reasonable efforts
//		to protect the confidentiality thereof, and to use
//		this information only in connection with your use
//		of Broadcom integrated circuit products.
//
//	2.	TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE
//		IS PROVIDED "AS IS" AND WITH ALL FAULTS AND BROADCOM
//		MAKES NO PROMISES, REPRESENTATIONS OR WARRANTIES,
//		EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE,
//		WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
//		DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE,
//		MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
//		PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
//		COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
//		CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE
//		RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
//
//	3.	TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT
//		SHALL BROADCOM OR ITS LICENSORS BE LIABLE FOR
//		(i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
//		EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
//		WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE
//		SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE
//		POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
//		EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE
//		ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
//		LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE
//		OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
//
//***************************************************************************
/**
*
*   @file   csl_dsp.c
*
*   @brief  This file accesses the shared memory for the VPU
*
****************************************************************************/

#include <string.h>
#include "assert.h"
#include "mobcom_types.h"
#include "consts.h"
#include "ostypes.h"
#include "log.h"
#include "shared.h"
#include "csl_dsp.h"
#include "osdw_dsp_drv.h"
#include "memmap.h"


#ifdef LMP_BUILD
typedef        AP_SharedMem_t VPSharedMem_t;
#endif

VPSharedMem_t		   *vp_shared_mem;

#if (defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_APPS_PROCESSOR) )


//******************************************************************************
//
// Function Name:	VPSHAREDMEM_Init
//
// Description: Initialize Shared Memory for UPU
//
// Notes:
//
//******************************************************************************

void VPSHAREDMEM_Init(UInt32	dsp_shared_mem)
{
	AP_SHAREDMEM_Init();
	
	vp_shared_mem = (VPSharedMem_t*) dsp_shared_mem;
	
	Log_DebugPrintf(LOGID_AUDIO, " VPSHAREDMEM_Init: dsp_shared_mem=0x%lx, \n", dsp_shared_mem);

	vp_shared_mem->vp_shared_cmdq_in = 0;
	vp_shared_mem->vp_shared_cmdq_out = 0;
	vp_shared_mem->vp_shared_statusq_in = 0;
	vp_shared_mem->vp_shared_statusq_out = 0;

}

//******************************************************************************
//
// Function Name:	VPSHAREDMEM_PostCmdQ
//
// Description:   This function posts an entry to the VPU command queue
//
// Notes:
//
//******************************************************************************

void VPSHAREDMEM_PostCmdQ(			// Post an entry to the VPU command queue
	VPCmdQ_t *cmd_msg					// Entry to post
	)
{
	VPCmdQ_t 	*p;
	UInt8 	next_cmd_in;

	Log_DebugPrintf(LOGID_AUDIO, " VPSHAREDMEM_PostCmdQ: cmdq_in=0x%x, cmdq_out=0x%x \n", vp_shared_mem->vp_shared_cmdq_in, vp_shared_mem->vp_shared_cmdq_out);
	next_cmd_in = (UInt8)(( vp_shared_mem->vp_shared_cmdq_in + 1 ) % VP_CMDQ_SIZE);

	assert( next_cmd_in != vp_shared_mem->vp_shared_cmdq_out );

	p = &vp_shared_mem->vp_shared_cmdq[ vp_shared_mem->vp_shared_cmdq_in ];
	p->cmd = cmd_msg->cmd;
	p->arg0 = cmd_msg->arg0;
	p->arg1 = cmd_msg->arg1;
	p->arg2 = cmd_msg->arg2;

	vp_shared_mem->vp_shared_cmdq_in = next_cmd_in;
	Log_DebugPrintf(LOGID_AUDIO, " VPSHAREDMEM_PostCmdQ: cmd=0x%x, arg0=0x%x, arg1=%d, arg2=%d \n", cmd_msg->cmd, cmd_msg->arg0, cmd_msg->arg1, cmd_msg->arg2);

	VPSHAREDMEM_TriggerRIPInt();

}


//******************************************************************************
//
// Function Name:	VPSHAREDMEM_ReadStatusQ
//
// Description: This function read an entry from the VPU status  queue
//
// Notes:
//
//******************************************************************************

Boolean VPSHAREDMEM_ReadStatusQ(			// Read an entry from the VPU status  queue
	VPStatQ_t *status_msg					// Entry from queue
	)
{
	VPStatQ_t *p;
	UInt8	status_out = vp_shared_mem->vp_shared_statusq_out;
	UInt8	status_in = vp_shared_mem->vp_shared_statusq_in;

	Log_DebugPrintf(LOGID_AUDIO, " VPSHAREDMEM_ReadStatusQ: status_in=0x%x, status_out=0x%x \n", vp_shared_mem->vp_shared_statusq_in, vp_shared_mem->vp_shared_statusq_out);
	if ( status_out == status_in )
	{
		return FALSE;
	}
	else
	{
		p = &vp_shared_mem->vp_shared_statusq[ status_out ];
		status_msg->status = p->status;
		status_msg->arg0 = (UInt16)p->arg0;
		status_msg->arg1 = (UInt16)p->arg1;
		status_msg->arg2 = (UInt16)p->arg2;
		status_msg->arg3 = (UInt16)p->arg3;
		Log_DebugPrintf(LOGID_AUDIO, " VPSHAREDMEM_ReadStatusQ: status=%d, arg0=%d, arg1=%d, arg2=%d, arg3=%d \n", p->status, p->arg0, p->arg1, p->arg2, p->arg3);

		vp_shared_mem->vp_shared_statusq_out = ( status_out + 1 ) % VP_STATUSQ_SIZE;

		return TRUE;
	}

}


#endif // #if (defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_APPS_PROCESSOR) )

