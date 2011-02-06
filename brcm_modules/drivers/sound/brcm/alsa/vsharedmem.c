
/*******************************************************************************************
Copyright 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement 
governing use of this software, this software is licensed to you under the 
terms of the GNU General Public License version 2, available at 
http://www.gnu.org/copyleft/gpl.html (the "GPL"). 

Notwithstanding the above, under no circumstances may you combine this software 
in any way with any other Broadcom software provided under a license other than 
the GPL, without Broadcom's express prior written consent.
*******************************************************************************************/

//****************** Copyright 2000 Mobilink Telecom, Inc. *********************
//
// Description:  This file contains the source code to access the shared memory
//				 for voice processing unit (VPU).
//
// $RCSfile: vsharedmem.c $
// $Revision: 1.3 $
// $Date: 2000/02/01 13:48:56 $
// $Author: awong $
//
//******************************** History *************************************
//
// $Log: vsharedmem.c $
// Revision 1.3  2000/02/01 13:48:56  awong
// Removed old shared mem stuff
// Revision 1.2  2000/01/27 14:01:26  awong
// Fixed compiler bugs
// Revision 1.1  2000/01/27 13:45:33  awong
// Initial revision
//
//******************************************************************************

//--#include "string.h"
//--#include "assert.h"
//--#include "types.h"
#include "mobcom_types.h"
#include "consts.h"
#include "shared.h"
#include "memmap.h"
//--#include "mti_trace.h"
#include "audio_ipc_consts.h"

extern int brcm_ipc_aud_control_send(const void * const, UInt32);

//VPSharedMem_t	*vp_shared_mem = (VPSharedMem_t *) (RAM_BASE + sizeof(SharedMem_t));
VPSharedMem_t	*vp_shared_mem;

#define INIT_SM( s, v )	vp_shared_mem->s = v

//******************************************************************************
//
// Function Name:	VPSHAREDMEM_Init
//
// Description: Initialize Shared Memory for UPU
//
// Notes:
//
//******************************************************************************

void VPSHAREDMEM_Init( void )
{
	vp_shared_mem = (VPSharedMem_t *) ((unsigned int)SHAREDMEM_GetSharedMemPtr() + 4*((sizeof(SharedMem_t)+3)/4) );

	INIT_SM( vp_shared_cmdq_in,			0);
	INIT_SM( vp_shared_cmdq_out,		0);
	INIT_SM( vp_shared_statusq_in,		0);
	INIT_SM( vp_shared_statusq_out,		0);

	INIT_SM( shared_FrameInt_flag,		0);
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
#if (defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_APPS_PROCESSOR) )
{
  CmdQ_t msg;

  msg.cmd  = cmd_msg->cmd;
  msg.cmd  += COMMAND_IPC_VPU_CMD_OFFSET;  //add offset to differentiate from other commands.
  msg.arg0 = cmd_msg->arg0;
  msg.arg1 = cmd_msg->arg1;
  msg.arg2 = cmd_msg->arg2;

  brcm_ipc_aud_control_send(&msg, sizeof(msg));
}
#else
{
	VPCmdQ_t 	*p;
	UInt8 	next_cmd_in;

	next_cmd_in = (UInt8)(( vp_shared_mem->vp_shared_cmdq_in + 1 ) % VP_CMDQ_SIZE);

//	snd_assert( next_cmd_in != vp_shared_mem->vp_shared_cmdq_out,return );

	p = &vp_shared_mem->vp_shared_cmdq[ vp_shared_mem->vp_shared_cmdq_in ];
	p->cmd = cmd_msg->cmd;
	p->arg0 = cmd_msg->arg0;
	p->arg1 = cmd_msg->arg1;
	p->arg2 = cmd_msg->arg2;

	vp_shared_mem->vp_shared_cmdq_in = next_cmd_in;
//LOG_PUTSTRING( "\r\n\r\n====== VPSHAREDMEM_PostCmdQ ======");
//	SDLTRACE_TraceString((UInt8 *)"next_cmd_in ", (UInt32 ) next_cmd_in);
	IRQ_TriggerRIPInt();
}
#endif // #if (defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_APPS_PROCESSOR) )

//******************************************************************************
//
// Function Name:	SHAREDMEM_ReadStatusQ
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

	if ( status_out == status_in )
	{
		return FALSE;
//        assert(0);
	}
	else
	{
		p = &vp_shared_mem->vp_shared_statusq[ status_out ];
		status_msg->status = p->status;
		status_msg->arg0 = (UInt16)p->arg0;
		status_msg->arg1 = (UInt16)p->arg1;
		status_msg->arg2 = (UInt16)p->arg2;
		status_msg->arg3 = (UInt16)p->arg3;

		vp_shared_mem->vp_shared_statusq_out = ( status_out + 1 ) % VP_STATUSQ_SIZE;

		return TRUE;

	}
}

