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
*   @file   vpripcmdq.c
*
*   @brief  This file writes to the command queue to start VPU tasks.
*
****************************************************************************/

#include "mobcom_types.h"
#include "assert.h"
#include "sharedmem.h"
#include "audio_ipc.h"


#if	defined(FUSE_APPS_PROCESSOR)

/** 
 * 	Function Name: VPRIPCMDQ_StartCallRecording
 * 	Function to Initiate Call Recording (working in both idle and speech call mode).
 *
 *	@note		Call Recording is a legacy command name. It can be used in both idle mode 
 *				(as Memo Recording) and speech call mode. In idle mode, the only source is 
 *				the microphone. While during the speech call, it is flexible to record from 
 *				a variety of sources:
 *
 *				\htmlonly <ol> 
 *				<li> Recording Near-End speech exclusively, i.e., from the microphone e.g. for voice recognition. </li>
 *				<li> Recording Far-End speech exclusively, i.e., from the channel and lastly </li>
 *				<li> Recording both Near and Far-End speech from the microphone and the channel default. </li>
 *				</ol>
 *				<br>
 *				\endhtmlonly
 *
 *				For Call Recording, the recorded format can be either PCM or AMR format. \BR
 *
 *				This is the main Speech recording command from the ARM. This command is to 
 *				be sent at the start of the memo recording. Before getting this command
 *				ARM should have enabled the Audio Interrupts using COMMAND_AUDIO_ENABLE, and 
 *				the audio inputs and outputs enabled by COMMAND_AUDIO_CONNECT. \BR
 *				
 *				After getting this
 *				command, DSP keeps getting 20ms worth of data from the microphone path and keeps
 *				encoding the data in the mode required by this command.  After every speech buffer 
 *				((shared_voice_buf.vr_buf[rec_buf_index]).vr_frame)
 *				has been filled, DSP issues a VP_STATUS_RECORDING_DATA_READY reply to the ARM and 
 *				generates an interrupt.  This prompts the ARM to read out the speech data from the 
 *				Shared memory buffer while the DSP continues to write new data into the other ping-pong 
 *				buffer.  This continues until the DSP is instructed to stop through receipt of either 
 *				the VP_COMMAND_CANCEL_RECORDING or the VP_COMMAND_STOP_RECORDING command.
 *				
 *	@param	VP_Record_Mode_t vp_record_mode; Indicates the source of recording
 *	@param	UInt16 nr_frame;  Number of 20ms frames to record per speech buffer.
 *	@param	UInt16 {	bit8:	   enables Noise Suppressor for Recording
 *					bit7:	   vp_dtx_enable; DTX enable for AMR encoder
 *					bit6-bit4: VP_Speech_Mode_t vp_speech_mode; Only support LINEAR_PCM and AMR_2.
 *					bit3:	   N/A
 *					bit2-bit0: AMR_Mode vp_amr_mode;} 
 *	@retval	None
 */
void VPRIPCMDQ_StartCallRecording(
	UInt8 	vp_record_mode,			// Audio path
	UInt8	buffer_length,			// Buffer length
	UInt16	vp_speech_amr_mode		// [8|7|6..4|3..0] = [audio_proc_enable|AMR2_dtx|vp_speech_mode|vp_amr_mode]
	)
{
	VPCmdQ_t msg;

	msg.cmd = VP_COMMAND_START_CALL_RECORDING;
	msg.arg0 = vp_record_mode;
	//RON 11/16/01 shared memory buffer lengths are variable - 20ms/100ms
	msg.arg1 = buffer_length;
	// Jie, 05/30/2003
	msg.arg2 = vp_speech_amr_mode;

	VPSHAREDMEM_PostCmdQ( &msg );
}

//******************************************************************************
//
// Function Name:	VPRIPCMDQ_StartCallRecordAndPlayback
//
// Description:     Initiate contintuous call recording and playback
//
// Notes:
//
//******************************************************************************

void VPRIPCMDQ_StartCallRecordAndPlayback(
	UInt8   vp_playback_mode,       //
	UInt8 	vp_record_mode,			// Audio path
	UInt8	buffer_length			// Buffer length
	)
{
	VPCmdQ_t msg;

	msg.cmd = VP_COMMAND_START_RECORD_AND_PLAYBACK;
	msg.arg0 = vp_playback_mode;
	msg.arg1 = vp_record_mode;
	msg.arg2 = buffer_length;

	VPSHAREDMEM_PostCmdQ( &msg );
}

//******************************************************************************
//
// Function Name:	VPRIPCMDQ_StopRecording
//
// Description:
//
// Notes:
//
//******************************************************************************

void VPRIPCMDQ_StopRecording()
{
	VPCmdQ_t msg;

	msg.cmd = VP_COMMAND_STOP_RECORDING;
	msg.arg0 = 0;		// init to avoid coverity warning
	msg.arg1 = 0;
	msg.arg2 = 0;

	VPSHAREDMEM_PostCmdQ( &msg );
}

//******************************************************************************
//
// Function Name:	VPRIPCMDQ_CancelRecording
//
// Description:
//
// Notes:
//
//******************************************************************************

void VPRIPCMDQ_CancelRecording()
{
	VPCmdQ_t msg;

	msg.cmd = VP_COMMAND_CANCEL_RECORDING;
	msg.arg0 = 0;		// init to avoid coverity warning
	msg.arg1 = 0;
	msg.arg2 = 0;

	VPSHAREDMEM_PostCmdQ( &msg );
}


//******************************************************************************
//
// Function Name:	VPRIPCMDQ_StartMemolRecording
//
// Description:
//
// Notes:
//
//******************************************************************************

void VPRIPCMDQ_StartMemolRecording(
	VP_Speech_Mode_t 	vp_speech_mode,
	UInt8				buffer_length,
	UInt16		vp_mode_amr				//[8|7:0]=[audio_proc_enable|na]
)
{
	VPCmdQ_t msg;
//LOG_PUTSTRING( "\r\n\r\n====== Sending Start recording CMD ======\r\n" );

	msg.cmd = VP_COMMAND_START_MEMO_RECORDING;
	msg.arg0 = vp_speech_mode;					//vp_speech_mode;
	//RON 11/16/01 shared memory buffer lengths are variable - 20ms/100ms
	msg.arg1 = buffer_length;
	msg.arg2 = vp_mode_amr;

	VPSHAREDMEM_PostCmdQ( &msg );
}

//******************************************************************************
//
// Function Name:	VPRIPCMDQ_StartRecordingPlayback
//
// Description:
//
// Notes:
//
//******************************************************************************

void VPRIPCMDQ_StartRecordingPlayback(
	UInt8 	vp_playback_mode,		// Audio path
	VP_PlaybackMix_Mode_t vp_playbackmix_mode		 // vp_playbackmix_mode = [0:3] = [non, ear_piece, UpCh, Both]
	)
{
	VPCmdQ_t msg;

	msg.cmd = VP_COMMAND_START_RECORDING_PLAYBACK;
	msg.arg0 = vp_playback_mode;	//RON 11/16/01
	msg.arg1 = vp_playbackmix_mode;		 // vp_playbackmix_mode = [0:3] = [non, ear_piece, UpCh, Both]
	msg.arg2 = 0;

	VPSHAREDMEM_PostCmdQ( &msg );
}


//******************************************************************************
//
// Function Name:	VPRIPCMDQ_StopPlayback
//
// Description:
//
// Notes:
//
//******************************************************************************

void VPRIPCMDQ_StopPlayback(
UInt8	flag
)
{
	VPCmdQ_t msg;

	msg.cmd = VP_COMMAND_STOP_PLAYBACK;
	// Set arg0 to TRUE if DSP should disable the Audio path. This should only
	// happen if we are in idle mode
	msg.arg0 = flag;
	msg.arg1 = 0;
	msg.arg2 = 0;

	VPSHAREDMEM_PostCmdQ( &msg );
}

//******************************************************************************
//
// Function Name:	VPRIPCMDQ_CancelPlayback
//
// Description:
//
// Notes:
//
//******************************************************************************
#if (defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_APPS_PROCESSOR) )

void VPRIPCMDQ_CancelPlayback()
{
	VPCmdQ_t msg;

	msg.cmd = VP_COMMAND_CANCEL_PLAYBACK;
	msg.arg0 = 0;		// init to avoid coverity warning
	msg.arg1 = 0;
	msg.arg2 = 0;

	VPSHAREDMEM_PostCmdQ( &msg );
}
#endif

//******************************************************************************
//
// Function Name: VPRIPCMDQ_DSP_AMR_RUN
//
// Description:   This function starts main AMR codec
//
// Notes:
//
//******************************************************************************
void
VPRIPCMDQ_DSP_AMR_RUN(UInt16 type, Boolean amr_if2_enable, UInt16 mst_flag)
{
  VPCmdQ_t msg;

  msg.cmd = VP_COMMAND_MAIN_AMR_RUN;
  msg.arg0 = type;
  msg.arg1 = amr_if2_enable;
  msg.arg2 = mst_flag;
  VPSHAREDMEM_PostCmdQ( &msg );
}

#endif   //#if	defined(FUSE_APPS_PROCESSOR)

//******************************************************************************
//
// Function Name:  VPRIPCMDQ_VocoderInit
//
// Description:  This function initializes the voice codec
//
// Notes:
//
//******************************************************************************
void
VPRIPCMDQ_VocoderInit(UInt16 val)
{
}


//******************************************************************************
//
// Function Name:  VPRIPCMDQ_SetVoiceDtx
//
// Description:  This function initializes the voice codec
//
// Notes:
//
//******************************************************************************
void
VPRIPCMDQ_SetVoiceDtx(Boolean enable)
{
  CmdQ_t  msg;

  msg.cmd = COMMAND_DTX_ENABLE;
  msg.arg0 = enable;
  msg.arg1 = 0;
  msg.arg2 = 0;

  SHAREDMEM_PostCmdQ(&msg);
}







