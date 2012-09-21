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
*	Description:  This file contains  prototypes of functions that
*	write to RIP command queue  to start various voice processing tasks.
*
******************************************************************************/

#ifndef _CSL_APCMD_H_
#define _CSL_APCMD_H_

#include "mobcom_types.h"

/**
*	Function Name: VPRIPCMDQ_StartCallRecording
*	Function to Initiate Call Recording (working in both idle
*	and speech call mode).
*
*	@note
*	Call Recording is a legacy command name. It can be used in both
*	idle mode (as Memo Recording) and speech call mode. In idle mode, the
*	only source is the microphone. While during the speech call, it is
*	flexible to record from a variety of sources:
*
*	\htmlonly <ol>
*	<li> Recording Near-End speech exclusively, i.e., from the microphone
*	e.g. for voice recognition. </li>
*	<li> Recording Far-End speech exclusively, i.e., from the channel
*	and lastly </li>
*	<li> Recording both Near and Far-End speech from the microphone
*	and the channel default. </li>
*	</ol>
*	<br>
*	\endhtmlonly
*
*	For Call Recording, the recorded format can be either
*	PCM or AMR format.\BR
*
*	This is the main Speech recording command from the ARM.
*	This command is sent at the start of the memo recording.
*	Before getting this command ARM should have enabled the
*	Audio Interrupts using COMMAND_AUDIO_ENABLE,and the audio
*	inputs and outputs enabled by COMMAND_AUDIO_CONNECT. \BR
*
*	After getting this
*	command, DSP keeps getting 20ms worth of data from the microphone path
*	and keepsencoding the data in the mode required by this command.
*	After every speech buffer has been filled, DSP issues
*	a VP_STATUS_RECORDING_DATA_READY reply to the
*	ARM and generates an interrupt.
*	This prompts the ARM to read out the speech data from
*	the Shared memory buffer while the DSP continues to write
*	new data into the other ping-pong buffer.
*	This continues until the DSP is instructed to stop through
*	receipt of either the VP_COMMAND_CANCEL_RECORDING or
*	the VP_COMMAND_STOP_RECORDING command.
*
*	@param	VP_Record_Mode_t vp_record_mode; Indicates the source of recording
*	@param	UInt16 nr_frame;  Number of 20ms frames to record per speech buffer.
*	@param	UInt16 {	bit8:	   enables Noise Suppressor for Recording
*	bit7:	   vp_dtx_enable; DTX enable for AMR encoder
*	bit6-bit4: VP_Speech_Mode_t vp_speech_mode;
*	bit3:	   N/A
*	bit2-bit0: AMR_Mode vp_amr_mode;}
*	@retval	None
*/
void VPRIPCMDQ_StartCallRecording(UInt8 vp_record_mode,	UInt8 buffer_length,
				UInt16 vp_speech_amr_mode);

/****************************************************************************/
/**

	Function Name:       VPRIPCMDQ_StartCallRecordAndPlayback

	Description:     Initiate contintuous call recording and playback

	Notes:

******************************************************************************/
void VPRIPCMDQ_StartCallRecordAndPlayback(UInt8 vp_playback_mode,
			UInt8 vp_record_mode, UInt8 buffer_length);

/****************************************************************************/
/**

	Function Name:       VPRIPCMDQ_StopRecording

	Description:

	Notes:

******************************************************************************/
void VPRIPCMDQ_StopRecording(void);

/*****************************************************************************/
/**
	Function Name:       VPRIPCMDQ_CancelRecording

	Description:

	Notes:

******************************************************************************/
void VPRIPCMDQ_CancelRecording(void);

/*****************************************************************************/
/**
	Function Name:       VPRIPCMDQ_StartMemolRecording

	Description:

	Notes:

******************************************************************************/
void VPRIPCMDQ_StartMemolRecording(UInt8 vp_speech_mode, UInt8 buffer_length,
				   UInt16 vp_mode_amr);

/*****************************************************************************/
/**
	Function Name:       VPRIPCMDQ_StartRecordingPlayback

	Description:

	Notes:

******************************************************************************/
void VPRIPCMDQ_StartRecordingPlayback(UInt8 vp_playback_mode,
			UInt8 vp_playbackmix_mode);

/*****************************************************************************/
/**
	Function Name:       VPRIPCMDQ_StopPlayback

	Description:

	Notes:

******************************************************************************/
void VPRIPCMDQ_StopPlayback(UInt8 flag);

/*****************************************************************************/
/**
	Function Name:       VPRIPCMDQ_CancelPlayback

	Description:

	Notes:

******************************************************************************/
void VPRIPCMDQ_CancelPlayback(void);

/*****************************************************************************/
/**
	Function Name: VPRIPCMDQ_DSP_AMR_RUN

	Description:   This function starts main AMR codec

	Notes:

******************************************************************************/
void VPRIPCMDQ_DSP_AMR_RUN(UInt16 type, Boolean amr_if2_enable,
						   UInt16 mst_flag);

/*****************************************************************************/
/**
	Function Name:       VPRIPCMDQ_DigitalSound

	Description:

	Notes:

******************************************************************************/
void VPRIPCMDQ_DigitalSound(UInt16 arg0);

/*****************************************************************************/
/**
	Function Name:       VPRIPCMDQ_SetARM2SP

	Description:

	Notes:

******************************************************************************/
void VPRIPCMDQ_SetARM2SP(UInt16 arg0, UInt16 arg1);

/*****************************************************************************/
/**
	Function Name:       VPRIPCMDQ_SetARM2SP2

	Description:

	Notes:

******************************************************************************/
void VPRIPCMDQ_SetARM2SP2(UInt16 arg0, UInt16 arg1);

/*****************************************************************************/
/**
	Function Name:	VPRIPCMDQ_SetARM2SP_HQDL
	Function used for configuring the ARM2SP_HQ_DL interface
	for mixing/playing 48kHz or 16kHz PCM data to Downlink. \BR

	@note
	This function sends a command to the DSP for starting or
	stopping of 48kHz data transfer from the ARM to the DSP. \BR

	Before getting this command ARM should have sent
	COMMAND_AUDIO_ENABLE, and the audio inputs and outputs enabled
	by COMMAND_AUDIO_CONNECT, along with enabling of the 48kHz
	AADMAC interface in shared_aadmac_aud_enable. \BR

	To start ARM2SP_HQ_DL: ARM enables ARM2SP_HQ_DL by
				VP_COMMAND_SET_ARM2SP_HQ_DL.\BR
	To resume ARM2SP_HQ_DL: ARM enables ARM2SP_HQ_DL by
				VP_COMMAND_SET_ARM2SP_HQ_DL with arg1=1.\BR

	To control shared_Arm2SP_InBuf[]: DSP interrupts ARM
	with VP_STATUS_ARM2SP_HQ_DL_EMPTY( shared_ARM2SP_HQ_DL_done_flag,
	shared_ARM2SP_HQ_DL_InBuf_out, ARM2SP_HQ_DL_flag )
	whenever finishing every 2/4 20ms data.
	shared_ARM2SP_HQ_DL_InBuf_out indicates the next index DSP will
	read.\BR

	To stop ARM2SP_HQ_DL:\BR
	Quick stop: ARM disables ARM2SP_HQ_DL using
	VP_COMMAND_SET_ARM2SP_HQ_DL (arg0=0).\BR
	The ARM2SP_HQ_DL will be stopped right away. There may be
	un-finished PCM data in shared_Arm2SP_InBuf[].\BR

	Finishing stop: After finishing filling the last speech frames of
	PCM data, ARM set shared_ARM2SP_HQ_DL_done_flag=1
	to indicate the end of PCM data. DSP ISR finishes the last speech
	frames and disables ARM2SP_HQ_DL. DSP sends
	VP_STATUS_ARM2SP_HQ_DL_EMPTY and arg0=0. \BR

	To pause ARM2SP_HQ_DL : ARM disables ARM2SP_HQ_DL using
	VP_COMMAND_SET_ARM2SP_HQ_DL (arg0=0, arg1=1).


		@param	UInt16 ARM2SP_HQ_DL_flag \BR
			\htmlonly
			<pre>
				{
				bits14:12	-Number of frames of data
						after which the DSP is
						supposed to send the
						VP_STATUS_ARM2SP_HQ_DL_EMPTY
						reply
						(between 1 and 4 or 1 and 2)
				bit11:10	- reserved
				bit9		- DL_OVERWRITE_bit
						1= overwrite ARM2SP_HQ_DL
						buffer to DL
						(only when DL_MIX_bit=0)
				bit8		- DL_MIX_bit
						1= mixing ARM2SP_HQ_DL
						buffer with DL
						(regardless
						DL_OVERWRITE_bit
						setting)
				bit7		- 48kHz Mono/Stereo
						[1/0] = [Mono/Stereo]
				bit6-1		- reserved
				bit0		- 48kHz DL PCM enable
						[1/0] = [enable/disable]
				}
			</pre>
			\endhtmlonly
		@param	UInt16 Reset_out_ptr_flag \BR
					=0, reset output pointer -
						shared_ARM2SP_HQ_DL_InBuf_out
						- of buffer
						shared_Arm2SP_InBuf[]
						to 0. Used for new
						ARM2SP_HQ_DL session.\BR
					=1, keep output pointer -
						shared_ARM2SP_HQ_DL_InBuf_out
						- unchanged.
						Used for PAUSE/RESUME the
						same ARM2SP_HQ_DL session.

\see ARM2SP_HQ_DL_interface, shared_Arm2SP_InBuf,
	shared_ARM2SP_HQ_DL_InBuf_out, shared_ARM2SP_HQ_DL_done_flag,
	VP_STATUS_ARM2SP_HQ_DL_EMPTY,VP_COMMAND_SET_ARM2SP_HQ_DL

******************************************************************************/
void VPRIPCMDQ_SetARM2SP_HQDL(UInt16 arg0, UInt16 arg1);

/*****************************************************************************/
/**
	Function Name:	VPRIPCMDQ_SetARM2SP2_HQDL
	Function used for configuring the ARM2SP2_HQ_DL interface
	for mixing/playing 48kHz or 16kHz PCM data to Downlink. \BR

	@note
	This function sends a command to the DSP for starting or
	stopping of 48kHz data transfer from the ARM to the DSP. \BR

	Before getting this command ARM should have sent
	COMMAND_AUDIO_ENABLE, and the audio inputs and outputs enabled
	by COMMAND_AUDIO_CONNECT, along with enabling of the 48kHz
	AADMAC interface in shared_aadmac_aud_enable. \BR

	To start ARM2SP2_HQ_DL: ARM enables ARM2SP2_HQ_DL by
				VP_COMMAND_SET_ARM2SP2_HQ_DL.\BR
	To resume ARM2SP2_HQ_DL: ARM enables ARM2SP2_HQ_DL by
				VP_COMMAND_SET_ARM2SP2_HQ_DL with arg1=1.\BR

	To control shared_Arm2SP2_InBuf[]: DSP interrupts ARM
	with VP_STATUS_ARM2SP2_HQ_DL_EMPTY( shared_ARM2SP2_HQ_DL_done_flag,
	shared_ARM2SP2_HQ_DL_InBuf_out, ARM2SP2_HQ_DL_flag )
	whenever finishing every 2/4 20ms data.
	shared_ARM2SP2_HQ_DL_InBuf_out indicates the next index DSP will
	read.\BR

	To stop ARM2SP2_HQ_DL:\BR
	Quick stop: ARM disables ARM2SP2_HQ_DL using
	VP_COMMAND_SET_ARM2SP2_HQ_DL (arg0=0).\BR
	The ARM2SP2_HQ_DL will be stopped right away. There may be
	un-finished PCM data in shared_Arm2SP2_InBuf[].\BR

	Finishing stop: After finishing filling the last speech frames of
	PCM data, ARM set shared_ARM2SP2_HQ_DL_done_flag=1
	to indicate the end of PCM data. DSP ISR finishes the last speech
	frames and disables ARM2SP2_HQ_DL. DSP sends
	VP_STATUS_ARM2SP2_HQ_DL_EMPTY and arg0=0. \BR

	To pause ARM2SP2_HQ_DL : ARM disables ARM2SP2_HQ_DL using
	VP_COMMAND_SET_ARM2SP2_HQ_DL (arg0=0, arg1=1).\BR


		@param	UInt16 ARM2SP2_HQ_DL_flag \BR
			\htmlonly
			<pre>
				{
				bits14:12	- Number of frames of data
						after which the DSP is
						supposed to send the
						VP_STATUS_ARM2SP2_HQ_DL_EMPTY
						reply
						(between 1 and 4 or 1 and 2)
				bit11:10	- reserved
				bit9		- DL_OVERWRITE_bit
						1= overwrite ARM2SP2_HQ_DL
						buffer to DL
						(only when DL_MIX_bit=0)
				bit8		- DL_MIX_bit
						1= mixing ARM2SP2_HQ_DL
						buffer with DL
						(regardless
						DL_OVERWRITE_bit
						setting)
				bit7		- 48kHz Mono/Stereo
						[1/0] = [Mono/Stereo]
				bit6-1		- reserved
				bit0		- 48kHz DL PCM enable
						[1/0] = [enable/disable]
				}
			</pre>
			\endhtmlonly
		@param	UInt16 Reset_out_ptr_flag \BR
					=0, reset output pointer -
						shared_ARM2SP2_HQ_DL_InBuf_out
						- of buffer
						shared_Arm2SP2_InBuf[]
						to 0. Used for new
						ARM2SP2_HQ_DL session.\BR
					=1, keep output pointer -
						shared_ARM2SP2_HQ_DL_InBuf_out
						- unchanged.
						Used for PAUSE/RESUME the
						same ARM2SP2_HQ_DL session.

\see ARM2SP2_HQ_DL_interface, shared_Arm2SP2_InBuf,
	shared_ARM2SP2_HQ_DL_InBuf_out, shared_ARM2SP2_HQ_DL_done_flag,
	VP_STATUS_ARM2SP2_HQ_DL_EMPTY,VP_COMMAND_SET_ARM2SP2_HQ_DL

******************************************************************************/
void VPRIPCMDQ_SetARM2SP2_HQDL(UInt16 arg0, UInt16 arg1);

/*****************************************************************************/
/**
	Function Name:       VPRIPCMDQ_SetBTNarrowBand

	Description:

	Notes:

******************************************************************************/
void VPRIPCMDQ_SetBTNarrowBand(UInt16 arg0);

/*****************************************************************************/
/**
	Function Name:       VPRIPCMDQ_USBHeadset

	Description:

	Notes:

******************************************************************************/
void VPRIPCMDQ_USBHeadset(UInt16 arg0);

/*****************************************************************************/
/**
	Function Name:       VPRIPCMDQ_MMVPUEnable

	Description:

	Notes:

******************************************************************************/
void VPRIPCMDQ_MMVPUEnable(UInt16 arg0);

/*****************************************************************************/
/**
	Function Name:       VPRIPCMDQ_MMVPUEnable

	Description:

	Notes:

******************************************************************************/
void VPRIPCMDQ_VPUEnable(void);

/*****************************************************************************/
/**
	Function Name:       VPRIPCMDQ_MMVPUDisable

	Description:

	Notes:

******************************************************************************/
void VPRIPCMDQ_MMVPUDisable(void);

/*****************************************************************************/
/**
	Function Name:       VPRIPCMDQ_VOIFControl

	Description:

	Notes:

******************************************************************************/
void VPRIPCMDQ_VOIFControl(UInt16 arg0);

/*****************************************************************************/
/**
	Function Name:       VPRIPCMDQ_SP

	Description: Send Speaker protection command

	Notes:

******************************************************************************/
void VPRIPCMDQ_SP(UInt16 arg0, UInt16 arg1, UInt16 arg2);

/*****************************************************************************/
/**
	Function Name:       VPRIPCMDQ_Clear_VoIPMode

	Description: When finishing voip session, clear voip mode, which block
	audio processing for voice calls

	Notes:

******************************************************************************/
void VPRIPCMDQ_Clear_VoIPMode(UInt16 arg0);

/**
*	Function Name: VPRIPCMDQ_VoLTE_Init
*	@note Function initializes Jitter Buffer of VoLTE interface.
*
*
*	@retval	None
*/
void VPRIPCMDQ_VoLTE_Init(void);

/**
*	Function Name: VPRIPCMDQ_VoLTE_Start_Stream
*	@note Function flushes Jitter Buffer of VoLTE interface for new stream.
*
*
*	@retval	None
*/
void VPRIPCMDQ_VoLTE_Start_Stream(void);

/**
*	Function Name: VPRIPCMDQ_VoLTE_Put_Frame
*	@note Function puts a frame into Jitter Buffer of VoLTE interface.
*
*	@param  UInt16	16-bit LSB of RTP timestamp
*	@param  UInt8	codec type
*	@param  UInt8	frame type
*	@param  UInt8	frame quality (0 - bad frame, 1 - good frame)
*	@param  UInt8	frame index
*	@param  UInt8	buffer Index
*
*	@retval	None
*/
void VPRIPCMDQ_VoLTE_Put_Frame(UInt16 timestamp, UInt8 codecType,
			       UInt8 frameType, UInt8 frameQuality,
			       UInt8 frameIndex, UInt8 bufferIndex);

/*****************************************************************************/
/**
	Function Name:       VPRIPCMDQ_PTTEnable

	Description:

	Notes:

******************************************************************************/
void VPRIPCMDQ_PTTEnable(UInt16 arg0);

/*****************************************************************************/
/**
*
* Function Name: csl_dsp_ext_modem_call
*
*   @note   This is the function to call before going into a call with an
*           external modem or ending of a call using an external modem.\BR
*           This should be sent before the enabling of any audio at the start
*           of a call and after the disabling of any audio at the end of a call.
*           The Audio clocks should not be shut-off before this function is
*           called, and the response to it has been received from the DSP by
*           the AP.\BR
*          This function sends the VP_COMMAND_EXT_MODEM_CALL command to the DSP.
*           For this command, the DSP enables the external modem interface, it
*           configures the AADMAC channels for speaker and mic external modem
*           paths and then sends a reply VP_STATUS_EXT_MODEM_CALL_DONE to the
*           AP with the same argument as the one passed in the command.
*
*   @param  enable = 0 = stop a call with external modem\BR
*                  = 1 = start a call with external modem
*   @return None
*
*****************************************************************************/
void csl_dsp_ext_modem_call(UInt16 enable);

#endif /* _CSL_APCMD_H_ */
