//*************** Copyright (c) 2000 - 2004 Broadcom Corp. *********************
//
// Description:  
//
// File:	polyripcmd.c
//
//
//******************************************************************************
//#define ENABLE_LOGGING
 
#include "mobcom_types.h"
#include "chip_version.h"
#include "shared.h"
#include "sharedmem_comms.h"
#include "sharedmem.h"
#include "polyripcmd.h"



//******************************************************************************
//
// Function Name:	RIPCMDQ_PolyringerStartPlay
//
// Description:		Start poly-ringer 
//
// Notes:
//
//******************************************************************************
void RIPCMDQ_PolyringerStartPlay(UInt16 BlockSamples, UInt16 OutMode)
{
	CmdQ_t 	msg;

	msg.cmd = COMMAND_POLYRINGER_STARTPLAY;	
    	msg.arg0 = BlockSamples;
    	msg.arg1 = OutMode;
		msg.arg2 = 0;
	if(OutMode==1) //For PCM back mode
	{
		msg.arg1 = 0;
		msg.arg2 = 0;
	}
	SHAREDMEM_PostCmdQ(&msg);


}


//******************************************************************************
//
// Function Name:	RIPCMDQ_PolyringerCancelPlay
//
// Description:		Cancel poly-ringer and trash buffered data
//
// Notes:
//
//******************************************************************************
void RIPCMDQ_PolyringerCancelPlay(void)
{

	CmdQ_t msg;

	msg.cmd = COMMAND_POLYRINGER_CANCELPLAY;
	msg.arg0 = 0;	
	msg.arg1 = 0;
	msg.arg2 = 0;
	SHAREDMEM_PostCmdQ( &msg );
}

