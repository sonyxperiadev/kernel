//****************** Copyright 2000 Mobilink Telecom, Inc. *********************
//
// Description:  This file contains  prototypes of functions that write to RIP 
//				command queue  to start various voice processing  tasks.
//
// $RCSfile: vpripcmdq.h $
// $Revision: 1.1 $
// $Date: 2000/01/27 13:45:26 $
// $Author: awong $
//
//******************************** History *************************************
//
// $Log: vpripcmdq.h $
// Revision 1.1  2000/01/27 13:45:26  awong
// Initial revision
//
//******************************************************************************

#ifndef _INC_VPRIPCMDQ_H_
#define _INC_VPRIPCMDQ_H_

#include "sharedmem.h"




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
	UInt8	buffer_length			// Buffer length
#if CHIPVERSION >= CHIP_VERSION(BCM2132,00) /* BCM2132 and later */
	,
	UInt16	vp_speech_amr_mode		// [8|7|6..4|3..0] = [audio_proc_enable|AMR2_dtx|vp_speech_mode|vp_amr_mode]
#endif // #if CHIPVERSION >= CHIP_VERSION(BCM2132,00) /* BCM2132 and later */
	);

void VPRIPCMDQ_StartCallRecordAndPlayback(
	UInt8	vp_playback_mode,		// playback destination
	UInt8 	vp_record_mode,			// Audio path
	UInt8	buffer_length			// Buffer length
	);

void VPRIPCMDQ_StopRecording(void);

void VPRIPCMDQ_StartMemolRecording(
	VP_Speech_Mode_t 	vp_speech_mode,
  	UInt8				buffer_length
#if CHIPVERSION >= CHIP_VERSION(ML2021,00) && CHIPVERSION <= CHIP_VERSION(ML2021,99) /* BCM2121 */
	,UInt8				dtx_vp_amr_mode			
#elif CHIPVERSION >= CHIP_VERSION(BCM2132,00) /* BCM2132 and later */
	,
	UInt16		vp_mode_amr				//[8|7:0]=[audio_proc_enable|na]
#endif // #elif CHIPVERSION >= CHIP_VERSION(BCM2132,00) /* BCM2132 and later */
	);

void VPRIPCMDQ_StartRecordingPlayback(
	UInt8 	vp_playback_mode		// Audio path
#if CHIPVERSION >= CHIP_VERSION(ML2021,00) && CHIPVERSION <= CHIP_VERSION(ML2021,99) /* BCM2121 */
	,UInt8	disable_tch_vocoder		// TURE: disable TCH voice functions	
#elif CHIPVERSION >= CHIP_VERSION(BCM2132,30) /* BCM2132C0 and later */
	,VP_PlaybackMix_Mode_t vp_playbackmix_mode		 // vp_playbackmix_mode = [0:3] = [non, ear_piece, UpCh, Both]                   
#endif // #elif CHIPVERSION >= CHIP_VERSION(BCM2132,30) /* BCM2132C0 and later */
	);

void VPRIPCMDQ_StopPlayback(
	UInt8	flag
	);

void VPRIPCMDQ_CancelPlayback(void);

void VPRIPCMDQ_CancelRecording(void);


void VPRIPCMDQ_VocoderInit(UInt16);
void VPRIPCMDQ_SetVoiceDtx(Boolean );
void VPRIPCMDQ_DSP_AMR_RUN(UInt16 , Boolean , UInt16 );
#if (defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_APPS_PROCESSOR))
void VPRIPCMDQ_SetCPVTStartStopFlag(UInt16 );
void VPRIPCMDQ_DSPAudioAlign(UInt16 );
#endif

#endif
