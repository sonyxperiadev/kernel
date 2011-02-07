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
*   @file   dspif_voice_play.c
*
*   @brief  DSP interface common APIs for voice playback
*
****************************************************************************/

#include "mobcom_types.h"
#include "resultcode.h"
#include "msconsts.h"
#include "shared.h"
#include "audio_consts.h"
#include "auddrv_def.h"
#include "dspif_voice_play.h"
#include "log.h"
#include "sharedmem.h"
#include "dspcmd.h"
#include "vpripcmdq.h"
#include "csl_aud_drv.h"
#include "audio_vdriver.h"

/**
*
* @addtogroup AudioDriverGroup
* @{
*/

//
// local defines
//
static UInt16 ARM2SP_BuildCommandArg0 (AUDIO_SAMPLING_RATE_t samplingRate, VORENDER_PLAYBACK_MODE_t	playbackMode, VORENDER_VOICE_MIX_MODE_t   mixMode, UInt32 numFramesPerInterrupt);

//
// APIs of VPU
//


// ==========================================================================
//
// Function Name: dspif_VPU_play_start
//
// Description: Start the data transfer of VPU play
//
// =========================================================================
Result_t dspif_VPU_play_start ( VORENDER_PLAYBACK_MODE_t	playbackMode,
								VORENDER_VOICE_MIX_MODE_t   mixMode,
								AUDIO_SAMPLING_RATE_t		samplingRate,
								UInt32						speechMode, // used by AMRNB and AMRWB
								UInt32						dataRateSelection, // used by AMRNB and AMRWB
								UInt32						numFramesPerInterrupt)
{
	UInt8 i;
	
	VPlayBack_Buffer_t *pBuf;
	SharedMem_t *sh_mem = SHAREDMEM_GetDsp_SharedMemPtr();

	Log_DebugPrintf(LOGID_AUDIO, " dspif_VPU_play_start::Start VPU play, playbackMode = %d,  speechMode = %d, dataRate = %d, mixMode = %d\n", 
							playbackMode, speechMode, dataRateSelection, mixMode);

	// init the pingpong buffer 
	for (i = 0; i < 2; i++)
	{
		pBuf = &sh_mem->shared_voice_buf.vp_buf[i];
		pBuf->nb_frame = numFramesPerInterrupt;
		pBuf->vp_speech_mode = speechMode;
		memset(&pBuf->vp_frame, 0, sizeof(pBuf->vp_frame));
	}

	// Alert! Our driver playbackMode and mixMode are 1 to 1 mapping to the DSP playback and mix mode defined
	// in shared.h
	VPRIPCMDQ_StartRecordingPlayback((UInt8)playbackMode, (VP_PlaybackMix_Mode_t)mixMode);
	
	return RESULT_OK;
}

// ==========================================================================
//
// Function Name: dspif_VPU_play_stop
//
// Description: Stop immediately the data transfer of VPU voic play
//
// =========================================================================
Result_t dspif_VPU_play_stop ( void)
{
	Log_DebugPrintf(LOGID_AUDIO, "dspif_VPU_play_stop::Stop VPU voice play\n");;

	VPRIPCMDQ_CancelPlayback();

	return RESULT_OK;

}


// ==========================================================================
//
// Function Name: dspif_VPU_play_pause
//
// Description: Pause the data transfer of VPU voice play
//
// =========================================================================
Result_t dspif_VPU_play_pause (void)
{
	Log_DebugPrintf(LOGID_AUDIO, "dspif_VPU_play_pause: Pause VPU voice play\n");

	return RESULT_OK;
}


// ==========================================================================
//
// Function Name: dspif_VPU_play_resume
//
// Description: Resume the data transfer of VPU voice play
//
// =========================================================================
Result_t dspif_VPU_play_resume( void)
{
	Log_DebugPrintf(LOGID_AUDIO, "dspif_VPU_play_resume: Resume VPU voice play \n");

	return RESULT_OK;
}

// ==========================================================================
//
// Function Name: dspif_VPU_play_flush
//
// Description: Flush the shared memory of VPU voice play
//
// =========================================================================
Result_t dspif_VPU_play_flush( void)
{
	Log_DebugPrintf(LOGID_AUDIO, "dspif_VPU_play_flush: Flush VPU voice play \n");
	
	return RESULT_OK;
}

// ==========================================================================
//
// Function Name: dspif_VPU_play_write_AMRNB
//
// Description: Write data to shared memory for VPU voice play.
//
// =========================================================================
UInt32 dspif_VPU_play_write_AMRNB ( UInt8 *inBuf, UInt32 inSize, UInt16 writeIndex, UInt32 speechMode, UInt32 numFramesPerInterrupt )
{
	UInt32 frameSize = sizeof(VR_Frame_AMR_t);
	
	SharedMem_t *sh_mem = SHAREDMEM_GetDsp_SharedMemPtr();
	UInt8	*buffer = NULL;
	UInt8	i, frameCount = 0;
	UInt32	totalCopied = 0;
	
	Log_DebugPrintf(LOGID_AUDIO, "dspif_VPU_play_write_AMRNB :: copy data to shared memory inBuf = 0x%x, inSize = %d\n", inBuf, inSize);
				
	frameCount = sh_mem->shared_voice_buf.vp_buf[writeIndex].nb_frame;
	// need to reset them every time?
	sh_mem->shared_voice_buf.vp_buf[writeIndex].vp_speech_mode = speechMode;
	sh_mem->shared_voice_buf.vp_buf[writeIndex].nb_frame = numFramesPerInterrupt;

	for (i = 0; i < frameCount; i++)
	{	
		buffer = (UInt8 *)(&sh_mem->shared_voice_buf.vp_buf[writeIndex].vp_frame.vectors_amr[i]);
			
		if (inSize - totalCopied >= frameSize)
		{
			// copy it to shared memeory
			memcpy(buffer, inBuf, frameSize);
			inBuf += frameSize;
			totalCopied += frameSize; 
		}
		else
		{
			Log_DebugPrintf(LOGID_AUDIO, "dspif_VPU_play_write_AMRNB :: insert silence...\n");	
			//memcpy(buffer + inSize - copied, amrnb_silence, frameSize);
		}		
	}

	return totalCopied;
}


// ==========================================================================
//
// Function Name: dspif_VPU_play_write_AMRNB
//
// Description: Write data to shared memory for VPU voice play.
//
// =========================================================================
UInt32 dspif_VPU_play_write_PCM ( UInt8 *inBuf, UInt32 inSize, UInt16 writeIndex, UInt32 speechMode, UInt32 numFramesPerInterrupt )
{
	UInt32 frameSize = LIN_PCM_FRAME_SIZE*sizeof(UInt16);
	
	SharedMem_t *sh_mem = SHAREDMEM_GetDsp_SharedMemPtr();
	UInt8	*buffer = NULL;
	UInt8	i, frameCount = 0;
	UInt32	totalCopied = 0;
	
	Log_DebugPrintf(LOGID_AUDIO, "dspif_VPU_play_write_PCM :: copy data to shared memory inBuf = 0x%x, inSize = %d\n", inBuf, inSize);
				
	frameCount = sh_mem->shared_voice_buf.vp_buf[writeIndex].nb_frame;
	// need to reset them every time?
	sh_mem->shared_voice_buf.vp_buf[writeIndex].vp_speech_mode = speechMode;
	sh_mem->shared_voice_buf.vp_buf[writeIndex].nb_frame = numFramesPerInterrupt;

	for (i = 0; i < frameCount; i++)
	{	
		buffer = (UInt8 *)(&sh_mem->shared_voice_buf.vp_buf[writeIndex].vp_frame.vectors_lin_PCM[i].param);
		if (inSize - totalCopied >= frameSize)
		{
			// copy it to shared memeory
			memcpy(buffer, inBuf, frameSize);
			inBuf += frameSize;
			totalCopied += frameSize; 
		}
		else
		{
			memcpy(buffer, inBuf, inSize - totalCopied);
			Log_DebugPrintf(LOGID_AUDIO, "dspif_VPU_play_write_PCM :: insert silence...\n");	
			memset(buffer + inSize - totalCopied, 0, frameSize - (inSize - totalCopied));
		}
	}

	return totalCopied;
}




//
// APIs of ARM2SP
//


// ==========================================================================
//
// Function Name: dspif_ARM2SP_play_start
//
// Description: Start the data transfer of ARM2SP play
//
// =========================================================================
Result_t dspif_ARM2SP_play_start ( UInt32 instanceID,
								VORENDER_PLAYBACK_MODE_t	playbackMode,
								VORENDER_VOICE_MIX_MODE_t   mixMode,
								AUDIO_SAMPLING_RATE_t		samplingRate,
								UInt32						numFramesPerInterrupt)
{
	UInt16 arg0;
	SharedMem_t *sh_mem = SHAREDMEM_GetDsp_SharedMemPtr();

	// restrict numFramesPerInterrupt due to the shared memory size 
	if (samplingRate == AUDIO_SAMPLING_RATE_8000 && numFramesPerInterrupt > 4)
		numFramesPerInterrupt = 4;

	if (samplingRate == AUDIO_SAMPLING_RATE_16000 && numFramesPerInterrupt > 2)
		numFramesPerInterrupt = 2;

	arg0 = ARM2SP_BuildCommandArg0 (samplingRate, playbackMode, mixMode, numFramesPerInterrupt);
    
						
	Log_DebugPrintf(LOGID_AUDIO, " dspif_ARM2SP_play_start::Start render, playbackMode = %d,  mixMode = %d, arg0 = 0x%x instanceID=0x%x\n", 
						playbackMode, mixMode, arg0, instanceID);

	if (instanceID == VORENDER_ARM2SP_INSTANCE1)
	{
	    memset(&sh_mem->shared_Arm2SP_InBuf, 0, sizeof(sh_mem->shared_Arm2SP_InBuf)); // clean buffer before starting to play

		Log_DebugPrintf(LOGID_AUDIO, " use audio_control_dsp to start ARM2SP, arg0=0x%x\n", arg0);
		audio_control_dsp(DSPCMD_TYPE_COMMAND_SET_ARM2SP, arg0, 0, 0, 0, 0);
	}
	else if (instanceID == VORENDER_ARM2SP_INSTANCE2)
	{
	    memset(&sh_mem->shared_Arm2SP2_InBuf, 0, sizeof(sh_mem->shared_Arm2SP2_InBuf)); // clean buffer before starting to play

		Log_DebugPrintf(LOGID_AUDIO, " use audio_control_dsp to start ARM2SP2, arg0=0x%x\n", arg0);
		audio_control_dsp(DSPCMD_TYPE_COMMAND_SET_ARM2SP2, arg0, 0, 0, 0, 0);
	}		
	return RESULT_OK;
}

// ==========================================================================
//
// Function Name: dspif_ARM2SP_play_stop
//
// Description: Stop immediately the data transfer of ARM2SP voic play
//
// =========================================================================
Result_t dspif_ARM2SP_play_stop ( UInt32 instanceID)
{
	Log_DebugPrintf(LOGID_AUDIO, "dspif_ARM2SP_play_stop::Stop ARM2SP voice play instanceID=0x%x\n", instanceID);
	
	if (instanceID == VORENDER_ARM2SP_INSTANCE1)
		audio_control_dsp(DSPCMD_TYPE_COMMAND_SET_ARM2SP, 0, 0, 0, 0, 0);
	else if (instanceID == VORENDER_ARM2SP_INSTANCE2)
		audio_control_dsp(DSPCMD_TYPE_COMMAND_SET_ARM2SP2, 0, 0, 0, 0, 0);

	return RESULT_OK;

}


// ==========================================================================
//
// Function Name: dspif_ARM2SP_play_pause
//
// Description: Pause the data transfer of ARM2SP voice play
//
// =========================================================================
Result_t dspif_ARM2SP_play_pause (UInt32 instanceID)
{
	Log_DebugPrintf(LOGID_AUDIO, "dspif_ARM2SP_play_pause: Pause ARM2SP voice play instanceID=0x%x \n", instanceID);

	if (instanceID == VORENDER_ARM2SP_INSTANCE1)
		audio_control_dsp(DSPCMD_TYPE_COMMAND_SET_ARM2SP, 0, 1, 0, 0, 0);
	else if (instanceID == VORENDER_ARM2SP_INSTANCE2)
		audio_control_dsp(DSPCMD_TYPE_COMMAND_SET_ARM2SP2, 0, 1, 0, 0, 0);
	
	return RESULT_OK;
}


// ==========================================================================
//
// Function Name: dspif_ARM2SP_play_resume
//
// Description: Resume the data transfer of ARM2SP voice play
//
// =========================================================================
Result_t dspif_ARM2SP_play_resume( UInt32 instanceID,
									VORENDER_PLAYBACK_MODE_t	playbackMode,
									VORENDER_VOICE_MIX_MODE_t   mixMode,
									AUDIO_SAMPLING_RATE_t		samplingRate,
									UInt32						numFramesPerInterrupt)
{	
	UInt16 arg0;

	// restrict numFramesPerInterrupt due to the shared memory size 
	if (samplingRate == AUDIO_SAMPLING_RATE_8000 && numFramesPerInterrupt > 4)
		numFramesPerInterrupt = 4;

	if (samplingRate == AUDIO_SAMPLING_RATE_16000 && numFramesPerInterrupt > 2)
		numFramesPerInterrupt = 2;

	arg0 = ARM2SP_BuildCommandArg0 (samplingRate, playbackMode, mixMode, numFramesPerInterrupt);
		
	Log_DebugPrintf(LOGID_AUDIO, "dspif_ARM2SP_play_resume: Resume ARM2SP voice play instanceID=0x%x \n", instanceID);

	if (instanceID == VORENDER_ARM2SP_INSTANCE1)
		audio_control_dsp(DSPCMD_TYPE_COMMAND_SET_ARM2SP, arg0, 1, 0, 0, 0);
	else if (instanceID == VORENDER_ARM2SP_INSTANCE2)	
		audio_control_dsp(DSPCMD_TYPE_COMMAND_SET_ARM2SP2, arg0, 1, 0, 0, 0);
	
	return RESULT_OK;
}


// ==========================================================================
//
// Function Name: dspif_ARM2SP_play_flush
//
// Description: Flush the shared memory of ARM2SP voice play
//
// =========================================================================
Result_t dspif_ARM2SP_play_flush( UInt32 instanceID)
{
	Log_DebugPrintf(LOGID_AUDIO, "dspif_ARM2SP_play_flush: Flush ARM2SP voice play instanceID=0x%x\n", instanceID);

	if (instanceID == VORENDER_ARM2SP_INSTANCE1)
		audio_control_dsp(DSPCMD_TYPE_COMMAND_SET_ARM2SP, 0, 0, 0, 0, 0);
	else if (instanceID == VORENDER_ARM2SP_INSTANCE2)
		audio_control_dsp(DSPCMD_TYPE_COMMAND_SET_ARM2SP2, 0, 0, 0, 0, 0);
	return RESULT_OK;
}


// ==========================================================================
//
// Function Name: dspif_ARM2SP_play_write
//
// Description: Write data to shared memory for ARM2SP voice play.
//
//  inBuf is the starting address of data buffer supplied 
//  inSize is number of Bytes.
//  writeIndex tis the index to Ping-Pong buffer in DSP SHmem for ARM2SP
//  numFramesPerInterrupt is not used.
// =========================================================================
UInt32 dspif_ARM2SP_play_write ( UInt32 instanceID, UInt8 *inBuf, UInt32 inSize, UInt16 writeIndex, UInt32 numFramesPerInterrupt )
{
	UInt32 offset = 0;
	
	SharedMem_t *sh_mem = SHAREDMEM_GetDsp_SharedMemPtr();
	UInt8	*buffer = NULL;
	UInt32  bufSize = ARM2SP_INPUT_SIZE;
	UInt32	totalCopied_bytes;
	
	Log_DebugPrintf(LOGID_AUDIO, "dspif_ARM2SP_play_write :: copy data to shared memory inBuf = 0x%x, inSize = %d instanceID=0x%x\n", inBuf, inSize, instanceID);
	
	// offset is in 16 bits
	offset = (writeIndex == 0)? (bufSize/2) : 0;

	if (instanceID == VORENDER_ARM2SP_INSTANCE1)
	{
		buffer = (UInt8 *)(&sh_mem->shared_Arm2SP_InBuf[offset]);
	}
	else
	if (instanceID == VORENDER_ARM2SP_INSTANCE2)
	{
		buffer = (UInt8 *)(&sh_mem->shared_Arm2SP2_InBuf[offset]);
	}
	else
	{
		Log_DebugPrintf(LOGID_AUDIO, "dspif_ARM2SP_play_write :: can not find \n" );
	}
	
	totalCopied_bytes = (inSize < bufSize) ? inSize : bufSize;

	if(buffer != NULL)
	{
		memcpy(buffer, inBuf, totalCopied_bytes);
	}
	
	// return the number of bytes has been copied
	return totalCopied_bytes;
}


//
// APIs of AMRWB
//

// ==========================================================================
//
// Function Name: dspif_AMRWB_play_start
//
// Description: Start the data transfer of AMRWB play
//
// =========================================================================
Result_t dspif_AMRWB_play_start ( VORENDER_PLAYBACK_MODE_t	playbackMode,
								VORENDER_VOICE_MIX_MODE_t   mixMode,
								AUDIO_SAMPLING_RATE_t		samplingRate,
								UInt32						speechMode, // used by AMRNB and AMRWB
								UInt32						dataRateSelection, // used by AMRNB and AMRWB
								UInt32						numFramesPerInterrupt)
{
	SharedMem_t* pSharedMem = SHAREDMEM_GetDsp_SharedMemPtr();
	UInt16 output_buf_select = 0; // fifo0 [0 2], dsp index depends on output buf select
	
	Log_DebugPrintf(LOGID_AUDIO, " dspif_AMRWB_play_start::Start AMRWB play, playbackMode = %d,  speechMode = %d, dataRate = %d, mixMode = %d\n", 
							playbackMode, speechMode, dataRateSelection, mixMode);

	pSharedMem->shared_WB_AMR_Ctrl_state = (output_buf_select << 11) | (speechMode << 5);

	// enhancemnet per dsp team suggestion
	// need to init the buffer pointer everytime we start the playback
	pSharedMem->shared_NEWAUD_InBuf_in[output_buf_select] = 0;
	pSharedMem->shared_NEWAUD_InBuf_out[output_buf_select] = 0;
	
	// program the threshold per dsp suggestion
	pSharedMem->shared_Inbuf_LOW_Sts_TH	= 0x200;
	pSharedMem->shared_Outbuf_LOW_Sts_TH = 0x400;	
	audio_control_dsp(DSPCMD_TYPE_MM_VPU_ENABLE, 1, 0, 0, 0, 0 );
	
	return RESULT_OK;
}

// ==========================================================================
//
// Function Name: dspif_AMRWB_play_init_stop
//
// Description: Tell DSP to stop AMRWB voic play
//
// =========================================================================
Result_t dspif_AMRWB_play_init_stop ( void)
{
	SharedMem_t* pSharedMem = SHAREDMEM_GetDsp_SharedMemPtr();

	Log_DebugPrintf(LOGID_AUDIO, "dspif_AMRWB_play_init_stop::Tell DSP to stop AMRWB voice play\n");;

	// set the flag based on the output select
	pSharedMem->shared_NEWAUD_InBuf_done_flag[0] = 1;		

	return RESULT_OK;
}

// ==========================================================================
//
// Function Name: dspif_AMRWB_play_stop
//
// Description: Stop immediately the data transfer of AMRWB voic play
//
// =========================================================================
Result_t dspif_AMRWB_play_stop ( void)
{
	Log_DebugPrintf(LOGID_AUDIO, "dspif_AMRWB_play_stop::Stop AMRWB voice play\n");;

	audio_control_dsp(DSPCMD_TYPE_MM_VPU_DISABLE, 0, 0, 0, 0, 0 );
	audio_control_dsp(DSPCMD_TYPE_SET_WAS_SPEECH_IDLE, 0, 0, 0, 0, 0 );

	return RESULT_OK;

}


// ==========================================================================
//
// Function Name: dspif_AMRWB_play_pause
//
// Description: Pause the data transfer of AMRWB voice play
//
// =========================================================================
Result_t dspif_AMRWB_play_pause (void)
{
	Log_DebugPrintf(LOGID_AUDIO, "dspif_AMRWB_play_pause: Pause AMRWB voice play\n");

	return RESULT_OK;
}


// ==========================================================================
//
// Function Name: dspif_AMRWB_play_resume
//
// Description: Resume the data transfer of AMRWB voice play
//
// =========================================================================
Result_t dspif_AMRWB_play_resume( void)
{
	Log_DebugPrintf(LOGID_AUDIO, "dspif_AMRWB_play_resume: Resume AMRWB voice play \n");

	return RESULT_OK;
}

// ==========================================================================
//
// Function Name: dspif_AMRWB_play_flush
//
// Description: Flush the shared memory of AMRWB voice play
//
// =========================================================================
Result_t dspif_AMRWB_play_flush( void)
{
	Log_DebugPrintf(LOGID_AUDIO, "dspif_AMRWB_play_flush: Flush AMRWB voice play. Please call dspif_AMRWB_play_start() to flush.\n");
	
	return RESULT_OK;
}


// ==========================================================================
//
// Function Name: dspif_AMRWB_play_write
//
// Description: Write data to shared memory for AMRWB voice play.
//
// =========================================================================
UInt32 dspif_AMRWB_play_write ( UInt8 *inBuf, UInt32 inSize, UInt16 writeIndex, UInt16 readIndex, UInt32 numFramesPerInterrupt )
{
	SharedMem_t *sh_mem = SHAREDMEM_GetDsp_SharedMemPtr();

	// writeIndex: the current writing index (open)
	// arm will never set writeIndex and readIndex same.
	// if they are same, it means buffer is empty!!

	UInt16 size_copied_bytes, size_wraparound; // bytes
	UInt16 space, totalCopied_words;  // words
	UInt32 q_load_words = inSize>>1; // words 
	UInt8 *buffer = (UInt8* ) &sh_mem->shared_decoder_InputBuffer[writeIndex&0x0fff];
	

	Log_DebugPrintf(LOGID_AUDIO_DETAIL, "dspif_AMRWB_play_write inSize = %d ##\n", inSize);

	if (writeIndex >= readIndex) //arm ahead of dsp
	{
		// shared memory available space
		space = (AUDIO_SIZE_PER_PAGE - writeIndex + readIndex - 1);
		
		// words to copy this time.
		totalCopied_words = (space <= q_load_words) ? space : q_load_words;
		
		if (totalCopied_words > 0)
		{
			if ( (writeIndex + totalCopied_words) > AUDIO_SIZE_PER_PAGE) //wrap around
			{
				// copy first part
				size_copied_bytes = (AUDIO_SIZE_PER_PAGE - writeIndex)<<1;
				memcpy(buffer, inBuf, size_copied_bytes);
				inBuf += size_copied_bytes;
				// copy second part, "-" has higher precedence than "<<"
				size_wraparound = (totalCopied_words<<1) - size_copied_bytes;
				memcpy( (UInt8* )&sh_mem->shared_decoder_InputBuffer[0], inBuf, size_wraparound );
			}
			else // no wrap around
			{
				size_copied_bytes = totalCopied_words<<1;
				size_wraparound = 0;
				memcpy(buffer, inBuf, size_copied_bytes);
			}
			
			sh_mem->shared_NEWAUD_InBuf_in[0] = (writeIndex + totalCopied_words ) % AUDIO_SIZE_PER_PAGE;
		}
		
		Log_DebugPrintf(LOGID_AUDIO_DETAIL, "dspif_AMRWB_play_write:[1] copy %d data to the shared memory\n", totalCopied_words);
	}
	else //dsp ahead of arm
	{
		// available shared memory space
		space = (readIndex - writeIndex - 1);
		// words to copy this time.
		totalCopied_words = (space <= q_load_words) ? space : q_load_words;
		
		if (totalCopied_words > 0)
		{
			size_copied_bytes = totalCopied_words<<1;
			memcpy(buffer, inBuf, size_copied_bytes);
			sh_mem->shared_NEWAUD_InBuf_in[0] = writeIndex + totalCopied_words;
		}
		
		Log_DebugPrintf(LOGID_AUDIO_DETAIL, "dspif_AMRWB_play_write:[2] copy %d data to the shared memory\n", totalCopied_words); 
	}

	// the bytes has been really copied.
	return (totalCopied_words<<1);
}

// ==========================================================================
//
// Function Name: dspif_AMRWB_play_GetWriteIndex
//
// Description: Get the current write index of shared memory for AMRWB voice play.
//
// =========================================================================
UInt16 dspif_AMRWB_play_GetWriteIndex (void)
{
	SharedMem_t *sh_mem = SHAREDMEM_GetDsp_SharedMemPtr();

	return sh_mem->shared_NEWAUD_InBuf_in[0];
}

//
// local functionss
//


// ==============================================================================
// Function Name: ARM2SP_BuildCommandArg0
//
//	Description: Build the arg0 for ARM2SP DSP command.
// ================================================================================
static UInt16 ARM2SP_BuildCommandArg0 (AUDIO_SAMPLING_RATE_t		samplingRate,
									   VORENDER_PLAYBACK_MODE_t		playbackMode,
									   VORENDER_VOICE_MIX_MODE_t	mixMode,
									   UInt32						numFramesPerInterrupt)
{
	UInt16 arg0 = 0;

	/**
	from shared.h
		Arg0
	#define	ARM2SP_DL_ENABLE_MASK	0x0001
	#define	ARM2SP_UL_ENABLE_MASK	0x0002
	
	#define	ARM2SP_TONE_RECODED		0x0008				//bit3=1, record the tone, otherwise record UL and/or DL
	#define	ARM2SP_UL_MIX			0x0010				//should set MIX or OVERWRITE, otherwise but not both, MIX wins
	#define	ARM2SP_UL_OVERWRITE		0x0020
	#define	ARM2SP_UL_BEFORE_PROC	0x0040				//bit6=1, play PCM before UL audio processing; default bit6=0
	#define	ARM2SP_DL_MIX			0x0100
	#define	ARM2SP_DL_OVERWRITE		0x0200
	#define	ARM2SP_DL_AFTER_PROC	0x0400				//bit10=1, play PCM after DL audio processing; default bit10=0
	#define	ARM2SP_16KHZ_SAMP_RATE  0x8000				//bit15=0 -> 8kHz data, bit15 = 1 -> 16kHz data
	
	#define	ARM2SP_FRAME_NUM		0x7000				//8K:1/2/3/4, 16K:1/2; if 0 (or other): 8K:4, 16K:2
	#define	ARM2SP_FRAME_NUM_BIT_SHIFT	12				//Number of bits to shift to get the frame number
	**/

	// samplingRate
	if (samplingRate == AUDIO_SAMPLING_RATE_16000)
		arg0 |= ARM2SP_16KHZ_SAMP_RATE;

	// set number of frames per interrupt
	arg0 |= (numFramesPerInterrupt << ARM2SP_FRAME_NUM_BIT_SHIFT);

	// set ul
	switch (playbackMode)
	{
		case VORENDER_PLAYBACK_UL:
			// set UL_enable
			arg0 |= ARM2SP_UL_ENABLE_MASK;

			if (mixMode == VORENDER_VOICE_MIX_UL 
				|| mixMode == VORENDER_VOICE_MIX_BOTH)
			{
				// mixing UL
				arg0 |= ARM2SP_UL_MIX;
			}
			else
			{
				//overwrite UL
				arg0 |= ARM2SP_UL_OVERWRITE;
			}
			break;

		case VORENDER_PLAYBACK_DL:
			// set DL_enable
			arg0 |= ARM2SP_DL_ENABLE_MASK;

			if (mixMode == VORENDER_VOICE_MIX_DL 
				|| mixMode == VORENDER_VOICE_MIX_BOTH)
			{
				// mixing DL
				arg0 |= ARM2SP_DL_MIX;
			}
			else
			{
				//overwirte DL
				arg0 |= ARM2SP_DL_OVERWRITE;
			}
			break;

		case VORENDER_PLAYBACK_BOTH:
			// set UL_enable
			arg0 |= ARM2SP_UL_ENABLE_MASK;

			// set DL_enable
			arg0 |= ARM2SP_DL_ENABLE_MASK;

			if (mixMode == VORENDER_VOICE_MIX_UL 
				|| mixMode == VORENDER_VOICE_MIX_BOTH)
			{
				// mixing UL
				arg0 |= ARM2SP_UL_MIX;
			}
			else
			{
				// overwirte UL
				arg0 |= ARM2SP_UL_OVERWRITE;
			}
			
			if (mixMode == VORENDER_VOICE_MIX_DL 
				|| mixMode == VORENDER_VOICE_MIX_BOTH)
			{
				// mixing DL
				arg0 |= ARM2SP_DL_MIX;
			}
			else
			{
				// overwirte DL
				arg0 |= ARM2SP_DL_OVERWRITE;
			}
			break;

		case VORENDER_PLAYBACK_NONE:
			break;

		default:
			break;
	}

	return arg0;
}
