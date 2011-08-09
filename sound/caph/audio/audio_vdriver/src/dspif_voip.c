/*******************************************************************************************
Copyright 2009, 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement governing use 
of this software, this software is licensed to you under the terms of the GNU General Public 
License version 2, available at http://www.gnu.org/copyleft/gpl.html (the "GPL"). 

Notwithstanding the above, under no circumstances may you combine this software in any way 
with any other Broadcom software provided under a license other than the GPL, without 
Broadcom's express prior written consent.
*******************************************************************************************/

/**
*
*  @file   dspif_voip.c
*
*  @brief  VoIP
*
****************************************************************************/
#include <linux/kernel.h>
#include <linux/workqueue.h>
#include "mobcom_types.h"
#include "resultcode.h"
#include "assert.h"
#include "xassert.h"
#include "msconsts.h"
#include "shared.h"
#include "audio_consts.h"
#include "dspcmd.h"
#include "dspif_voip.h"
#include "csl_aud_queue.h"
#include "csl_apcmd.h"
#include "csl_voip.h"
#include "log.h"
//
// local defines & structures
//


#define VOIP_MAX_FRAME_LEN	(642 + AUDQUE_MARGIN) // 320 words + 1 word for codecType

static const UInt16 sVoIPDataLen[] = {0, 322, 160, 38, 166, 642, 70};

static VP_Mode_AMR_t prev_amr_mode = (VP_Mode_AMR_t)0xffff;
static Boolean				telephony_amr_if2;

static VOIPFillFramesCB_t	FillVOIPFramesCB;
static VOIPDumpFramesCB_t	DumpVOIPFramesCB;


static struct work_struct voip_work;
static struct workqueue_struct *voip_workqueue = NULL;

void VOIP_ProcessVOIPDLDone();

extern UInt32 audio_control_dsp(UInt32 param1,UInt32 param2,UInt32 param3,UInt32 param4,UInt32 param5,UInt32 param6);

//=============================================================================
// Function definitions
//=============================================================================

void VoIP_Task_Entry(struct work_struct *work)
{

	if (FillVOIPFramesCB)
		FillVOIPFramesCB(1);
}

// DSP interrupt handlers

//******************************************************************************
//
// Function Name:  VoIP_ProcessDLFrame()
//
// Description:	This function handle the VoIP DL data
//
// Notes:			
//******************************************************************************
void VOIP_ProcessVOIPDLDone()
{

    if ( voip_workqueue )
        queue_work(voip_workqueue, &voip_work); 
       
       
}

//******************************************************************************
//
// Function Name:  VoIP_StartTelephony()
//
// Description:	This function starts full duplex telephony session
//
// Notes:	The full duplex DSP interface is in sharedmem, not vsharedmem.
//		But since its function is closely related to voice processing,
//		we put it here.
//
//******************************************************************************
static Boolean VoIP_StartTelephony(
	VOIPDumpFramesCB_t telephony_dump_cb,
	VOIPFillFramesCB_t telephony_fill_cb
	)
{

    voip_workqueue = create_workqueue("voip");
    if (!voip_workqueue)
    {
        return TRUE;
    }

    INIT_WORK(&voip_work, VoIP_Task_Entry);

	DumpVOIPFramesCB = telephony_dump_cb;
	FillVOIPFramesCB = telephony_fill_cb;
	Log_DebugPrintf( LOGID_SOC_AUDIO, "=====VoIP_StartTelephony \r\n");

	VOIP_ProcessVOIPDLDone();
	return TRUE;
}


//******************************************************************************
//
// Function Name:  VoIP_StartMainAMRDecodeEncode()
//
// Description:		This function passes the AMR frame to be decoded
//					from application to DSP and starts its decoding
//					as well as encoding of the next frame.
//
// Notes:			The full duplex DSP interface is in sharedmem, not vsharedmem.
//					But since its function is closely related to voice processing,
//					we put it here.
//
//******************************************************************************
void VoIP_StartMainAMRDecodeEncode(
	VP_Mode_AMR_t		decode_amr_mode,	// AMR mode for decoding the next speech frame
	UInt8				*pBuf,		// buffer carrying the AMR speech data to be decoded
	UInt16				length,		// number of bytes of the AMR speech data to be decoded
	VP_Mode_AMR_t		encode_amr_mode,	// AMR mode for encoding the next speech frame
	Boolean				dtx_mode	// Turn DTX on (TRUE) or off (FALSE)
	)
{
	// decode the next downlink AMR speech data from application
	CSL_WriteDLVoIPData((UInt16)decode_amr_mode, (UInt16 *)pBuf);

	// signal DSP to start AMR decoding and encoding
	
	if (prev_amr_mode == 0xffff || prev_amr_mode != encode_amr_mode)
	{
	
		Log_DebugPrintf(LOGID_SOC_AUDIO, "=====VoIP_StartMainAMRDecodeEncode UL codecType=0x%x, send VP_COMMAND_MAIN_AMR_RUN to DSP", encode_amr_mode);
		prev_amr_mode = encode_amr_mode;
		VPRIPCMDQ_DSP_AMR_RUN((UInt16)encode_amr_mode, telephony_amr_if2, FALSE);
	}

}


//******************************************************************************
//
// Function Name:  AP_ProcessStatusMainAMRDone()
//
// Description:		This function handles VP_STATUS_MAIN_AMR_DONE from DSP.
//
// Notes:			
//
//******************************************************************************
void AP_ProcessStatusMainAMRDone(UInt16 codecType)
{
 	static UInt16 Buf[321]; // buffer to hold UL data and codec type

	//Log_DebugPrintf(LOGID_SOC_AUDIO,"=====AP_ProcessStatusMainAMRDone \r\n");
	
	// encoded uplink AMR speech data now ready in DSP shared memory, copy it to application
	// pBuf is to point the start of the encoded speech data buffer
	if (DumpVOIPFramesCB) 
	{
		CSL_ReadULVoIPData(codecType, Buf);
		DumpVOIPFramesCB((UInt8*)Buf, 0);
	}

}


//******************************************************************************
//
// Function Name:  VoIP_StopTelephony()
//
// Description:	This function stops full duplex telephony session
//
// Notes:	The full duplex DSP interface is in sharedmem, not vsharedmem.
//		But since its function is closely related to voice processing,
//		we put it here.
//
//******************************************************************************
static Boolean VoIP_StopTelephony(void)
{    
	Log_DebugPrintf(LOGID_SOC_AUDIO,"=====VoIP_StopTelephony \r\n");

	DumpVOIPFramesCB=NULL; 
	FillVOIPFramesCB=NULL;

    flush_workqueue(voip_workqueue);
    destroy_workqueue(voip_workqueue);

    voip_workqueue = NULL;

	prev_amr_mode = (VP_Mode_AMR_t)0xffff;
	
	return TRUE;
}

Result_t AP_VoIP_StartTelephony(
	VOIPDumpFramesCB_t telephony_dump_cb,
	VOIPFillFramesCB_t telephony_fill_cb
	)
{
	VoIP_StartTelephony(telephony_dump_cb,telephony_fill_cb);
	return RESULT_OK;
}

Result_t AP_VoIP_StopTelephony( void )
{
	// Clear voip mode, which block audio processing for voice calls
	audio_control_dsp( DSPCMD_TYPE_COMMAND_CLEAR_VOIPMODE, 0, 0, 0, 0, 0 ); // arg0 = 0 to clear VOIPmode

	VoIP_StopTelephony();	
	return RESULT_OK;
}


