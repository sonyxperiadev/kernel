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
// Description:  This file contains  functions that write to RIP command queue
//				 to start various voice processing  tasks.
//
// $RCSfile: vpripcmdq.c $
// $Revision: 1.1 $
// $Date: 2000/01/27 13:45:31 $
// $Author: awong $
//
//******************************** History *************************************
//
// $Log: vpripcmdq.c $
// Revision 1.1  2000/01/27 13:45:31  awong
// Initial revision
//
//******************************************************************************

#include "mobcom_types.h"
//--#include "types.h"
//--#include "assert.h"
#include "shared.h"
//--#include "mti_trace.h"
#include "audio_ipc_consts.h"
#include <linux/broadcom/chip_version.h>

extern int brcm_ipc_aud_control_send(const void * const, UInt32);
extern void post_msg(UInt16 cmd, UInt16 arg0, UInt16 arg1, UInt16 arg2);
void SHAREDMEM_PostCmdQ(CmdQ_t *cmd_msg)
{
	post_msg(cmd_msg->cmd, cmd_msg->arg0, cmd_msg->arg1, cmd_msg->arg2);
}


#if CHIPVERSION >= CHIP_VERSION(ML2021,00) && CHIPVERSION <= CHIP_VERSION(ML2021,99) /* BCM2121 */
extern Boolean		AMR_memo_mode;			// TRUE: in AMR memo recording/playback mode
#endif // #if CHIPVERSION >= CHIP_VERSION(ML2021,00) && CHIPVERSION <= CHIP_VERSION(ML2021,99) /* BCM2121 */

//******************************************************************************
//
// Function Name:	VPRIPCMDQ_StartTraining
//
// Description:
//
// Notes:
//
//******************************************************************************

void VPRIPCMDQ_StartTraining()
{
	VPCmdQ_t msg;

	msg.cmd = VP_COMMAND_START_TRAINING;
	msg.arg0 = 0;		// init to avoid coverity warning
	msg.arg1 = 0;
	msg.arg2 = 0;
	VPSHAREDMEM_PostCmdQ( &msg );
}


//******************************************************************************
//
// Function Name:	VPRIPCMDQ_CancelTraining
//
// Description:
//
// Notes:
//
//******************************************************************************

void VPRIPCMDQ_CancelTraining()
{
	VPCmdQ_t msg;

	msg.cmd = VP_COMMAND_CANCEL_TRAINING;
	msg.arg0 = 0;		// init to avoid coverity warning
	msg.arg1 = 0;
	msg.arg2 = 0;

	VPSHAREDMEM_PostCmdQ( &msg );
}

//******************************************************************************
//
// Function Name:	VPRIPCMDQ_StartRecognition
//
// Description:
//
// Notes:
//
//******************************************************************************

void VPRIPCMDQ_StartRecognition(
	UInt16 nr_templates
)
{
	VPCmdQ_t msg;

	msg.cmd = VP_COMMAND_START_RECOGNITION;
	msg.arg0 = nr_templates;
	msg.arg1 = 0;		// init to avoid coverity warning
	msg.arg2 = 0;

	VPSHAREDMEM_PostCmdQ( &msg );
}

//******************************************************************************
//
// Function Name:	VPRIPCMDQ_CancelRecognition
//
// Description:
//
// Notes:
//
//******************************************************************************

void VPRIPCMDQ_CancelRecognition()
{
	VPCmdQ_t msg;

	msg.cmd = VP_COMMAND_CANCEL_RECOGNITION;
	msg.arg0 = 0;		// init to avoid coverity warning
	msg.arg1 = 0;
	msg.arg2 = 0;

	VPSHAREDMEM_PostCmdQ( &msg );
}

//******************************************************************************
//
// Function Name:	VPRIPCMDQ_StartRecognitionPlayback
//
// Description:
//
// Notes:
//
//******************************************************************************

void VPRIPCMDQ_StartRecognitionPlayback()
{
	VPCmdQ_t msg;

	msg.cmd = VP_COMMAND_START_RECOGNITION_PLAYBACK;
	msg.arg0 = 0;		// init to avoid coverity warning
	msg.arg1 = 0;
	msg.arg2 = 0;

	VPSHAREDMEM_PostCmdQ( &msg );
}

//******************************************************************************
//
// Function Name:	VPRIPCMDQ_StartCallRecording
//
// Description:
//
// Notes:
//
//******************************************************************************

void VPRIPCMDQ_StartCallRecording(
	UInt8 	vp_record_mode,			// Audio path
	UInt8	buffer_length			// Buffer length
#if CHIPVERSION >= CHIP_VERSION(BCM2132,00) /* BCM2132 and later */
	,
#if ((defined(_BCM2153_) && CHIP_REVISION >= 51)) //for 2153 e1 and later, bit8 was used to control VPU NS
	UInt16	vp_speech_amr_mode		//[8] [7..4|3..0] = [ul_ns] [vp_speech_mode|vp_amr_mode]
#else
	UInt8	vp_speech_amr_mode		// [7..4|3..0] = [vp_speech_mode|vp_amr_mode]
#endif //#if ((defined(_BCM2153_) && CHIP_REVISION >= 51))
#endif // #if CHIPVERSION >= CHIP_VERSION(BCM2132,00) /* BCM2132 and later */
	)
{
	VPCmdQ_t msg;

	msg.cmd = VP_COMMAND_START_CALL_RECORDING;
    msg.arg0 = vp_record_mode;
	//RON 11/16/01 shared memory buffer lengths are variable - 20ms/100ms
    msg.arg1 = buffer_length;
#if CHIPVERSION >= CHIP_VERSION(BCM2132,00) /* BCM2132 and later */
	// Jie, 05/30/2003
	msg.arg2 = vp_speech_amr_mode;
#endif // #if CHIPVERSION >= CHIP_VERSION(BCM2132,00) /* BCM2132 and later */

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

#if CHIPVERSION >= CHIP_VERSION(ML2021,00) && CHIPVERSION <= CHIP_VERSION(ML2021,99) /* BCM2121 */
	AMR_memo_mode = FALSE;		// if not AMR memo recording, it is OK
#endif // #if CHIPVERSION >= CHIP_VERSION(ML2021,00) && CHIPVERSION <= CHIP_VERSION(ML2021,99) /* BCM2121 */

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

#if CHIPVERSION >= CHIP_VERSION(ML2021,00) && CHIPVERSION <= CHIP_VERSION(ML2021,99) /* BCM2121 */
	AMR_memo_mode = FALSE;		// if not AMR memo recording, it is OK
#endif // #if CHIPVERSION >= CHIP_VERSION(ML2021,00) && CHIPVERSION <= CHIP_VERSION(ML2021,99) /* BCM2121 */

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
   	UInt8				buffer_length
#if CHIPVERSION >= CHIP_VERSION(ML2021,00) && CHIPVERSION <= CHIP_VERSION(ML2021,99) /* BCM2121 */
	,UInt8				dtx_vp_amr_mode
#elif CHIPVERSION >= CHIP_VERSION(BCM2132,00) /* BCM2132 and later */
	,
#if ((defined(_BCM2153_) && CHIP_REVISION >= 51)) //for 2153 e1 and later, bit8 was used to control VPU NS
	UInt16 				vp_mode_amr
#else
	VP_Mode_AMR_t		vp_mode_amr
#endif // #if ((defined(_BCM2153_) && CHIP_REVISION >= 51))
#endif // #elif CHIPVERSION >= CHIP_VERSION(BCM2132,00) /* BCM2132 and later */
)
{
	VPCmdQ_t msg;

//LOG_PUTSTRING( "\r\n\r\n====== Sending Start recording CMD ======\r\n" );

	msg.cmd = VP_COMMAND_START_MEMO_RECORDING;
	msg.arg0 = vp_speech_mode;					//vp_speech_mode;
	//RON 11/16/01 shared memory buffer lengths are variable - 20ms/100ms
    msg.arg1 = buffer_length;
#if CHIPVERSION >= CHIP_VERSION(ML2021,00) && CHIPVERSION <= CHIP_VERSION(ML2021,99) /* BCM2121 */
	msg.arg2 = dtx_vp_amr_mode;
	if( vp_speech_mode == VP_SPEECH_MODE_AMR ) AMR_memo_mode = TRUE;	
#elif CHIPVERSION >= CHIP_VERSION(BCM2132,00) /* BCM2132 and later */
	msg.arg2 = vp_mode_amr;
#endif // #elif CHIPVERSION >= CHIP_VERSION(BCM2132,00) /* BCM2132 and later */

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
	UInt8 	vp_playback_mode		// Audio path
#if CHIPVERSION >= CHIP_VERSION(ML2021,00) && CHIPVERSION <= CHIP_VERSION(ML2021,99) /* BCM2121 */
	,UInt8	disable_tch_vocoder				// TURE: disable TCH voice functions
#elif CHIPVERSION >= CHIP_VERSION(BCM2132,30) /* BCM2132C0 and later */
	,VP_PlaybackMix_Mode_t vp_playbackmix_mode		 // vp_playbackmix_mode = [0:3] = [non, ear_piece, UpCh, Both]
#endif // #elif CHIPVERSION >= CHIP_VERSION(BCM2132,30) /* BCM2132C0 and later */
	)
{
	VPCmdQ_t msg;

	msg.cmd = VP_COMMAND_START_RECORDING_PLAYBACK;
    msg.arg0 = vp_playback_mode;	//RON 11/16/01
#if CHIPVERSION >= CHIP_VERSION(ML2021,00) && CHIPVERSION <= CHIP_VERSION(ML2021,99) /* BCM2121 */
	msg.arg1 = disable_tch_vocoder;				// TURE: disable TCH voice functions
	// for BCM2121, disable_tch_vocoder=1 only used for AMR memo playback
	if( disable_tch_vocoder ) AMR_memo_mode = TRUE;	
#elif CHIPVERSION >= CHIP_VERSION(BCM2132,30) /* BCM2132C0 and later */
	msg.arg1 = vp_playbackmix_mode;		 // vp_playbackmix_mode = [0:3] = [non, ear_piece, UpCh, Both]
#endif // #elif CHIPVERSION >= CHIP_VERSION(BCM2132,30) /* BCM2132C0 and later */
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

#if CHIPVERSION >= CHIP_VERSION(ML2021,00) && CHIPVERSION <= CHIP_VERSION(ML2021,99) /* BCM2121 */
	AMR_memo_mode = FALSE;		// if not AMR memo playback, it is OK
#endif // #if CHIPVERSION >= CHIP_VERSION(ML2021,00) && CHIPVERSION <= CHIP_VERSION(ML2021,99) /* BCM2121 */

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

void VPRIPCMDQ_CancelPlayback()
{
	VPCmdQ_t msg;

	msg.cmd = VP_COMMAND_CANCEL_PLAYBACK;
	msg.arg0 = 0;		// init to avoid coverity warning
	msg.arg1 = 0;
	msg.arg2 = 0;

#if CHIPVERSION >= CHIP_VERSION(ML2021,00) && CHIPVERSION <= CHIP_VERSION(ML2021,99) /* BCM2121 */
	AMR_memo_mode = FALSE;		// if not AMR memo playback, it is OK
#endif // #if CHIPVERSION >= CHIP_VERSION(ML2021,00) && CHIPVERSION <= CHIP_VERSION(ML2021,99) /* BCM2121 */

	VPSHAREDMEM_PostCmdQ( &msg );
}


#if CHIPVERSION >= CHIP_VERSION(BCM2133,0) /* BCM2133 and later */
//******************************************************************************
//
// Function Name:	VPRIPCMDQ_PolyringerStartPlay
//
// Description:		Start poly-ringer play with EventBufferA
//
// Notes:
//
//******************************************************************************
void VPRIPCMDQ_PolyringerStartPlay(UInt16 BlockSamples, UInt16 OutMode)
{
	CmdQ_t 	msg;

	msg.cmd = COMMAND_POLYRINGER_STARTPLAY;
    msg.arg0 = BlockSamples;
    msg.arg1 = OutMode;
	msg.arg2 = 0;
	SHAREDMEM_PostCmdQ(&msg);
}

//******************************************************************************
//
// Function Name:	VPRIPCMDQ_PolyringerStopPlay
//
// Description:		Stop poly-ringer and finish the last EventBuffer
//
// Notes:
//
//******************************************************************************
void VPRIPCMDQ_PolyringerStopPlay(void)
{
	CmdQ_t msg;

	msg.cmd = COMMAND_POLYRINGER_STARTPLAY;
	msg.arg0 = 0;		// init to avoid coverity warning
	msg.arg1 = 0;
	msg.arg2 = 0;
	SHAREDMEM_PostCmdQ( &msg );
}

//******************************************************************************
//
// Function Name:	VPRIPCMDQ_PolyringerCancelPlay
//
// Description:		Cancel poly-ringer without any EventBuffer processing
//
// Notes:
//
//******************************************************************************
void VPRIPCMDQ_PolyringerCancelPlay(void)
{
	CmdQ_t msg;

	msg.cmd = COMMAND_POLYRINGER_CANCELPLAY;
	msg.arg0 = 0;		// init to avoid coverity warning
	msg.arg1 = 0;
	msg.arg2 = 0;
	SHAREDMEM_PostCmdQ( &msg );
}

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
  CmdQ_t msg;

  msg.cmd = COMMAND_VOCODER_INIT;
  msg.arg0 = val;
  msg.arg1 = 0;
  msg.arg2 = 0;
  SHAREDMEM_PostCmdQ( &msg );
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
  CmdQ_t msg;

  msg.cmd = COMMAND_MAIN_AMR_RUN;
  msg.arg0 = type;
  msg.arg1 = amr_if2_enable;
  msg.arg2 = mst_flag;
  SHAREDMEM_PostCmdQ( &msg );
}

//******************************************************************************
//
// Function Name: VPRIPCMDQ_DSP_CLEAR_VOIPMODE
//
// Description:   This function sends cmd to dsp to clear voip mode
//
// Notes:
//
//******************************************************************************
void 
VPRIPCMDQ_DSP_CLEAR_VOIPMODE(void)
{
  CmdQ_t msg = {0}; 
  msg.cmd = COMMAND_CLEAR_VOIPMODE;
  msg.arg0 = 0;
  msg.arg1 = 0;
  msg.arg2 = 0;
  SHAREDMEM_PostCmdQ( &msg );
}

#if (defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_APPS_PROCESSOR))

void
VPRIPCMDQ_SetCPVTStartStopFlag(UInt16 arg0)
{
  CmdQ_t msg;

  msg.cmd = AUDIO_IPC_CMD_VT_AMR_START_STOP;
  msg.cmd += COMMAND_IPC_FUNC_CMD_OFFSET;
  msg.arg0 = arg0;
  msg.arg1 = 0;
  msg.arg2 = 0;

  brcm_ipc_aud_control_send(&msg, sizeof(msg));
}

void
VPRIPCMDQ_DSPAudioAlign(UInt16 arg0)
{
  CmdQ_t msg;

  msg.cmd = AUDIO_IPC_CMD_DSP_AUDIO_ALIGN;
  msg.cmd += COMMAND_IPC_FUNC_CMD_OFFSET;
  msg.arg0 = arg0;

  brcm_ipc_aud_control_send(&msg, sizeof(msg));
}

#endif

#endif  /* BCM2133 and later */



