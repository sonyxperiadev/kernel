//*********************************************************************
//
// (c)1999-2011 Broadcom Corporation
//
// Unless you and Broadcom execute a separate written software license agreement governing use of this software,
// this software is licensed to you under the terms of the GNU General Public License version 2,
// available at http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
//
//*********************************************************************
/**
*
*   @file   csl_apcmd.c
*
*   @brief  This file contains AP command interface to DSP.
*
****************************************************************************/
#include "mobcom_types.h"
#include "shared.h"
#include "csl_dsp.h"
#include "csl_apcmd.h"

void VPRIPCMDQ_SetARM2SP(UInt16 arg0, UInt16 arg1);

void VPRIPCMDQ_SetARM2SP2(UInt16 arg0, UInt16 arg1);

extern void VPSHAREDMEM_PostCmdQ(VPCmdQ_t *cmd_msg);


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
	UInt8	 	vp_speech_mode,
	UInt8		buffer_length,
	UInt16		vp_mode_amr				//[8|7:0]=[audio_proc_enable|na]
)
{
	VPCmdQ_t msg;

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
	UInt8	vp_playbackmix_mode		 // vp_playbackmix_mode = [0:3] = [non, ear_piece, UpCh, Both]
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

void VPRIPCMDQ_StopPlayback(UInt8 flag)
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
void VPRIPCMDQ_CancelPlayback()
{
	VPCmdQ_t msg;

	msg.cmd = VP_COMMAND_CANCEL_PLAYBACK;
	msg.arg0 = 0;		// init to avoid coverity warning
	msg.arg1 = 0;
	msg.arg2 = 0;

	VPSHAREDMEM_PostCmdQ( &msg );
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
void VPRIPCMDQ_DSP_AMR_RUN(UInt16 type, Boolean amr_if2_enable, UInt16 mst_flag)
{
  VPCmdQ_t msg;

  msg.cmd = VP_COMMAND_MAIN_AMR_RUN;
  msg.arg0 = type;
  msg.arg1 = amr_if2_enable;
  msg.arg2 = mst_flag;
  VPSHAREDMEM_PostCmdQ( &msg );
}


//******************************************************************************
//
// Function Name:	VPRIPCMDQ_DigitalSound
//
// Description:
//
// Notes:
//
//******************************************************************************
void VPRIPCMDQ_DigitalSound(UInt16 arg0)
{
 VPCmdQ_t	msg;	

	msg.cmd = VP_COMMAND_DIGITAL_SOUND;
	msg.arg0 = arg0;  
	msg.arg1 = 0;
	msg.arg2 = 0;
	VPSHAREDMEM_PostCmdQ(&msg);
				
}

//******************************************************************************
//
// Function Name:	VPRIPCMDQ_SetARM2SP
//
// Description:
//
// Notes:
//
//******************************************************************************
void VPRIPCMDQ_SetARM2SP(UInt16 arg0, UInt16 arg1)
{
 VPCmdQ_t	msg;	

	msg.cmd = VP_COMMAND_SET_ARM2SP;
	msg.arg0 = arg0;  
	msg.arg1 = arg1;
	msg.arg2 = 0;
	VPSHAREDMEM_PostCmdQ(&msg);
				
}

//******************************************************************************
//
// Function Name:	VPRIPCMDQ_SetARM2SP2
//
// Description:
//
// Notes:
//
//******************************************************************************
void VPRIPCMDQ_SetARM2SP2(UInt16 arg0, UInt16 arg1)
{
 VPCmdQ_t	msg;	

	msg.cmd = VP_COMMAND_SET_ARM2SP2;
	msg.arg0 = arg0;  
	msg.arg1 = arg1;
	msg.arg2 = 0;
	VPSHAREDMEM_PostCmdQ(&msg);
				
}

//******************************************************************************
//
// Function Name:	VPRIPCMDQ_SetBTNarrowBand
//
// Description:
//
// Notes:
//
//******************************************************************************
void VPRIPCMDQ_SetBTNarrowBand(UInt16 arg0)
{
 VPCmdQ_t	msg;	
 
	msg.cmd = VP_COMMAND_SET_BT_NB;
	msg.arg0 = arg0;  
	msg.arg1 = 0;
	msg.arg2 = 0;
	VPSHAREDMEM_PostCmdQ(&msg);
	
}

//******************************************************************************
//
// Function Name:	VPRIPCMDQ_USBHeadset
//
// Description:
//
// Notes:
//
//******************************************************************************
void VPRIPCMDQ_USBHeadset(UInt16 arg0)
{
 VPCmdQ_t	msg;	

	msg.cmd = VP_COMMAND_USB_HEADSET;
	msg.arg0 = arg0;  
	msg.arg1 = 0;
	msg.arg2 = 0;
	VPSHAREDMEM_PostCmdQ(&msg);

}



//******************************************************************************
//
// Function Name:	VPRIPCMDQ_MMVPUEnable
//
// Description:
//
// Notes:
//
//******************************************************************************
void VPRIPCMDQ_MMVPUEnable(UInt16 arg0)
{
 VPCmdQ_t	msg;	

	msg.cmd = VP_COMMAND_MM_VPU_ENABLE;
	msg.arg0 = arg0;  //mode control
	msg.arg1 = 0;
	msg.arg2 = 0;
	VPSHAREDMEM_PostCmdQ(&msg);
	
}
//******************************************************************************
//
// Function Name:	VPRIPCMDQ_MMVPUEnable
//
// Description:
//
// Notes:
//
//******************************************************************************
void VPRIPCMDQ_VPUEnable(void)
{
 VPCmdQ_t	msg;	

	msg.cmd = VP_COMMAND_VPU_ENABLE;
	msg.arg0 = 0;  
	msg.arg1 = 0;
	msg.arg2 = 0;
	VPSHAREDMEM_PostCmdQ(&msg);
	
}
//******************************************************************************
//
// Function Name:	VPRIPCMDQ_MMVPUDisable
//
// Description:
//
// Notes:
//
//******************************************************************************
void VPRIPCMDQ_MMVPUDisable(void)
{
 VPCmdQ_t	msg;	
 
	msg.cmd = VP_COMMAND_MM_VPU_DISABLE;
	msg.arg0 = 0;
	msg.arg1 = 0;
	msg.arg2 = 0;
	VPSHAREDMEM_PostCmdQ(&msg);
	
}


//******************************************************************************
//
// Function Name:	VPRIPCMDQ_VOIFControl
//
// Description:
//
// Notes:
//
//******************************************************************************
void VPRIPCMDQ_VOIFControl(UInt16 arg0)
{
 VPCmdQ_t	msg;	

	msg.cmd = VP_COMMAND_VOIF_CONTROL;
	msg.arg0 = arg0;  //enable/disable
	msg.arg1 = 0;
	msg.arg2 = 0;
	VPSHAREDMEM_PostCmdQ(&msg);
	
}


//******************************************************************************
//
// Function Name:	VPRIPCMDQ_SP
//
// Description: 
// Send Speaker protection command
//
// Notes:
//
//******************************************************************************
void VPRIPCMDQ_SP(UInt16 arg0, UInt16 arg1, UInt16 arg2)
{
 VPCmdQ_t	msg;	

	msg.cmd = VP_COMMAND_SP;
	msg.arg0 = arg0;  //enable/disable
	msg.arg1 = arg1;
	msg.arg2 = arg2;
	VPSHAREDMEM_PostCmdQ(&msg);
	
}

//******************************************************************************
//
// Function Name:	VPRIPCMDQ_Clear_VoIPMode
//
// Description: 
// When finishing voip session, clear voip mode, which block audio processing for voice calls
//
// Notes:
//
//******************************************************************************
void VPRIPCMDQ_Clear_VoIPMode(UInt16 arg0)
{
 VPCmdQ_t	msg;	

	msg.cmd = VP_COMMAND_CLEAR_VOIPMODE;
	msg.arg0 = arg0;
	msg.arg1 = 0;
	msg.arg2 = 0;
	VPSHAREDMEM_PostCmdQ(&msg);

}

/** 
 * 	Function Name: VPRIPCMDQ_ENABLE_48KHZ_SPEAKER_OUTPUT
 * 	@note Function to enable the output to the speaker path at 48kHz.
 *
 *	@note This command enables the output to the speaker path at 48kHz. This command has to be 
 *     sent by the APE before sending VP_COMMAND_SP. 
 *     COMMAND_NEWAUDFIFO_START. APE should enable the Audio interrupts to
 *     come to the DSP at 16kHz using the COMMAND_AUDIO_ENABLE command, and the audio
 *     input or output should be enabled using the COMMAND_AUDIO_CONNECT command.
 *				
 * 			 @param  UInt16  	{
 *								Bit 0: Enable_48kHz_Speaker_Output: 0 = Disable, 1 = Enable
 *                                                    Bit 1: Interrupt is enabled for this //Currently not supported
 *                                                    Bit 2: = 0 Mono, = 1 Stereo // Currently only supports mono 
 *	                            }
 *
 * 			 \see VP_COMMAND_SP, NEWAUDFIFO Interface, COMMAND_NEWAUDFIFO_START  
 *	         @retval	None
 */
void VPRIPCMDQ_ENABLE_48KHZ_SPEAKER_OUTPUT(Boolean enable, Boolean interrupt_enabled, Boolean stereo_enabled)
{
  VPCmdQ_t msg;

  msg.cmd = VP_COMMAND_48KHZ_SPEAKER_OUTPUT_ENABLE;
  msg.arg0 = ( (UInt16)enable | ( ((UInt16)interrupt_enabled)<<1 ) | ( ((UInt16)stereo_enabled)<<2 ) );
  msg.arg1 = 0;
  msg.arg2 = 0;

  VPSHAREDMEM_PostCmdQ( &msg );

}

#ifdef VOLTE_SUPPORT

/** 
 * 	Function Name: VPRIPCMDQ_VoLTE_Init
 * 	@note Function initializes Jitter Buffer of VoLTE interface.
 *
 *
 *	@retval	None
 */
void VPRIPCMDQ_VoLTE_Init(void)
{
 VPCmdQ_t	msg;	

	msg.cmd = VP_COMMAND_VOLTE_INIT;
	msg.arg0 = 0;
	msg.arg1 = 0;
	msg.arg2 = 0;
	VPSHAREDMEM_PostCmdQ(&msg);

}

/** 
 * 	Function Name: VPRIPCMDQ_VoLTE_Start_Stream
 * 	@note Function flushes Jitter Buffer of VoLTE interface for new stream.
 *
 *
 *	@retval	None
 */
void VPRIPCMDQ_VoLTE_Start_Stream(void)
{
 VPCmdQ_t	msg;	

	msg.cmd = VP_COMMAND_VOLTE_START_STREAM;
	msg.arg0 = 0;
	msg.arg1 = 0;
	msg.arg2 = 0;
	VPSHAREDMEM_PostCmdQ(&msg);

}

/** 
 * 	Function Name: VPRIPCMDQ_VoLTE_Put_Frame
 * 	@note Function puts a frame into Jitter Buffer of VoLTE interface.
 *
 * 	@param  UInt16  	16-bit LSB of RTP timestamp
 * 	@param  UInt8	  	codec type
 * 	@param  UInt8  		frame type
 * 	@param  UInt8  		frame quality (0 - bad frame, 1 - good frame)
 * 	@param  UInt8  		frame index
 * 	@param  UInt8  		buffer Index
 *
 *	@retval	None
*/
void VPRIPCMDQ_VoLTE_Put_Frame(UInt16 timestamp, UInt8 codecType, UInt8 frameType, UInt8 frameQuality, UInt8 frameIndex, UInt8 bufferIndex)
{
 VPCmdQ_t	msg;	

	msg.cmd = VP_COMMAND_VOLTE_PUT_FRAME;
	msg.arg0 = timestamp;
	msg.arg1 = codecType<<8 | (frameQuality & 0x1)<<4 | (frameType & 0xF);
	msg.arg2 = bufferIndex<<8 | frameIndex;
	VPSHAREDMEM_PostCmdQ(&msg);

}

#endif // VOLTE_SUPPORT


