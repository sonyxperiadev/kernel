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
*   @file   dspif_voice_record.c
*
*   @brief  DSP interface common APIs for voice recording
*
****************************************************************************/

#include "mobcom_types.h"
#include "resultcode.h"
#include "msconsts.h"
#include "shared.h"
#include "audio_consts.h"
#include "auddrv_def.h"
#include "dspif_voice_record.h"
#include "dspcmd.h"
#include "log.h"
#include "vpripcmdq.h"
#include "csl_aud_drv.h"
#include "audio_vdriver.h"

#ifdef UNDER_LINUX
#include "ripisr_audio.h"
#endif


/**
*
* @addtogroup AudioDriverGroup
* @{
*/

static record_data_ready_cb_t record_ready = NULL;

static void dspif_VPU_record_handler(VPStatQ_t reqMsg);
//
// local defines
//


//
// APIs of VPU
//
// ==========================================================================
//
// Function Name: dspif_VPU_record_init
//
// Description: Start the data transfer of VPU record
//
// =========================================================================
void dspif_VPU_record_init ()
{
    register_capture_request_handler(dspif_VPU_record_handler);
}


// ==========================================================================
//
// Function Name: dspif_VPU_record_deinit
//
// Description: Start the data transfer of VPU record
//
// =========================================================================
void dspif_VPU_record_deinit ()
{
    register_capture_request_handler(NULL);
}
// ==========================================================================
//
// Function Name: dspif_VPU_record_start
//
// Description: Start the data transfer of VPU record
//
// =========================================================================
Result_t dspif_VPU_record_start ( VOCAPTURE_RECORD_MODE_t	recordMode,
								AUDIO_SAMPLING_RATE_t		samplingRate,
								UInt32						speechMode, // used by AMRNB and AMRWB
								UInt32						dataRate, // used by AMRNB and AMRWB
								Boolean						procEnable,
								Boolean						dtxEnable,
								UInt32						numFramesPerInterrupt)
{
	// [8|7|6..4|3..0] = [audio_proc_enable|AMR2_dtx|vp_speech_mode|vp_amr_mode]
	UInt16 encodingMode = 
			(procEnable << 8) |
			(dtxEnable << 7) |
			(speechMode << 4) |
			(dataRate);
	
	VPRIPCMDQ_StartCallRecording((UInt8)recordMode, (UInt8)numFramesPerInterrupt, (UInt16)encodingMode);
	Log_DebugPrintf(LOGID_AUDIO, " : dspif_VPU_record_start::Start capture, encodingMode = 0x%x, recordMode = 0x%x, procEnable = 0x%x, dtxEnable = 0x%x, speechMode = 0x%x, dataRate = 0x%x\n", 
							encodingMode, recordMode, procEnable, dtxEnable, speechMode, dataRate);
	
	return RESULT_OK;
}

// ==========================================================================
//
// Function Name: dspif_VPU_record_stop
//
// Description: Stop immediately the data transfer of VPU voic record
//
// =========================================================================
Result_t dspif_VPU_record_stop ( void)
{
	Log_DebugPrintf(LOGID_AUDIO, "dspif_VPU_record_stop::Stop VPU voice record\n");;

	VPRIPCMDQ_CancelRecording();

	return RESULT_OK;

}


// ==========================================================================
//
// Function Name: dspif_VPU_record_pause
//
// Description: Pause the data transfer of VPU voice record
//
// =========================================================================
Result_t dspif_VPU_record_pause (void)
{
	Log_DebugPrintf(LOGID_AUDIO, "dspif_VPU_record_pause: Pause VPU voice record\n");

	return RESULT_OK;
}


// ==========================================================================
//
// Function Name: dspif_VPU_record_resume
//
// Description: Resume the data transfer of VPU voice record
//
// =========================================================================
Result_t dspif_VPU_record_resume( void)
{
	Log_DebugPrintf(LOGID_AUDIO, "dspif_VPU_record_resume: Resume VPU voice record \n");

	return RESULT_OK;
}



// ==========================================================================
//
// Function Name: dspif_VPU_record_read_AMRNB
//
// Description: Read data to shared memory for VPU voice record.
//
// =========================================================================
UInt32 dspif_VPU_record_read_AMRNB ( UInt8 *inBuf, UInt32 inSize, UInt16 bufIndex, UInt32 speechMode, UInt32 numFramesPerInterrupt )
{
	UInt32 frameSize = sizeof(VR_Frame_AMR_t);
	
	SharedMem_t *sh_mem = SHAREDMEM_GetDsp_SharedMemPtr();
	UInt8	*buffer = NULL;
	UInt8	i, frameCount = 0;
	UInt32	totalCopied = 0;
	
	Log_DebugPrintf(LOGID_AUDIO, "dspif_VPU_record_read_AMRNB :: copy data to shared memory inBuf = 0x%x, inSize = %d\n", inBuf, inSize);
				
	frameCount = sh_mem->shared_voice_buf.vr_buf[bufIndex].nb_frame;
	
	for (i = 0; i < frameCount; i++)
	{	
		buffer = (UInt8 *)(&sh_mem->shared_voice_buf.vr_buf[bufIndex].vr_frame.vectors_amr[i]);
			
		if (inSize - totalCopied >= frameSize)
		{
			// copy it from shared memeory 
			memcpy(inBuf, buffer, frameSize);
			inBuf += frameSize;
			totalCopied += frameSize; 
		}
		else
		{
			Log_DebugPrintf(LOGID_AUDIO, "dspif_VPU_record_read_AMRNB :: no available data in shared memory\n");	
			//memcpy(buffer + inSize - copied, amrnb_silence, frameSize);
		}		
	}

	return totalCopied;
}


// ==========================================================================
//
// Function Name: dspif_VPU_play_read_PCM
//
// Description: Read data to shared memory for VPU voice record.
//
// =========================================================================
UInt32 dspif_VPU_record_read_PCM ( UInt8 *inBuf, UInt32 inSize, UInt16 bufIndex, UInt32 speechMode, UInt32 numFramesPerInterrupt )
{
	UInt32 frameSize = LIN_PCM_FRAME_SIZE*sizeof(UInt16);
	
	SharedMem_t *sh_mem = SHAREDMEM_GetDsp_SharedMemPtr();
	UInt8	*buffer = NULL;
	UInt8	i, frameCount = 0;
	UInt32	totalCopied = 0;

	if ( speechMode == VOCAPTURE_SPEECH_MODE_LINEAR_PCM_16K)
		frameSize = WB_LIN_PCM_FRAME_SIZE*sizeof(UInt16);

	//Log_DebugPrintf(LOGID_AUDIO, "dspif_VPU_record_read_PCM :: copy data to shared memory inBuf = 0x%x, inSize = %d\n", inBuf, inSize);
				
	frameCount = sh_mem->shared_voice_buf.vr_buf[bufIndex].nb_frame;	

	for (i = 0; i < frameCount; i++)
	{	
		buffer = (UInt8 *)(&sh_mem->shared_voice_buf.vr_buf[bufIndex].vr_frame.vectors_lin_PCM[i].param);
		if (inSize - totalCopied >= frameSize)
		{
			// copy it from shared memeory 
			memcpy(inBuf, buffer, frameSize);
			inBuf += frameSize;
			totalCopied += frameSize; 
		}
		else
		{
			Log_DebugPrintf(LOGID_AUDIO, "dspif_VPU_record_read_PCM :: no available data in shared memory\n");
		}
	}

	return totalCopied;
}


//
// APIs of AMRWB
//

// ==========================================================================
//
// Function Name: dspif_AMRWB_record_start
//
// Description: Start the data transfer of AMRWB record
//
// =========================================================================
Result_t dspif_AMRWB_record_start ( VOCAPTURE_RECORD_MODE_t	recordMode,
								AUDIO_SAMPLING_RATE_t		samplingRate,
								UInt32						speechMode, // used by AMRNB and AMRWB
								UInt32						dataRate, // used by AMRNB and AMRWB
								Boolean						procEnable,
								Boolean						dtxEnable,
								UInt32						numFramesPerInterrupt)
{
	SharedMem_t* pSharedMem = SHAREDMEM_GetDsp_SharedMemPtr();

	Log_DebugPrintf(LOGID_AUDIO, "dspif_AMRWB_record_start::start AMRWB voice record\n");;
	
	pSharedMem->shared_WB_AMR_Ctrl_state = dataRate | (dtxEnable<<4) | (speechMode<<5);

	// enhancement per dsp team suggestion
	// need to init the record buffer pointer for new record
	pSharedMem->shared_encodedSamples_buffer_in[0] = 0;
	pSharedMem->shared_encodedSamples_buffer_out[0] = 0;
	//temporarily done with enable cmd. need do it separately?
	//pSharedMem->shared_16ISR_state = 1;

	audio_control_dsp(DSPCMD_TYPE_MM_VPU_ENABLE, 2, 0, 0, 0, 0 );
	
	return RESULT_OK;
}


// ==========================================================================
//
// Function Name: dspif_AMRWB_record_stop
//
// Description: Stop immediately the data transfer of AMRWB voic record
//
// =========================================================================
Result_t dspif_AMRWB_record_stop ( void)
{
	Log_DebugPrintf(LOGID_AUDIO, "dspif_AMRWB_record_stop::Stop AMRWB voice record\n");

	audio_control_dsp(DSPCMD_TYPE_MM_VPU_DISABLE, 0, 0, 0, 0, 0 );
	audio_control_dsp(DSPCMD_TYPE_SET_WAS_SPEECH_IDLE, 0, 0, 0, 0, 0 );

	return RESULT_OK;

}


// ==========================================================================
//
// Function Name: dspif_AMRWB_record_pause
//
// Description: Pause the data transfer of AMRWB voice record
//
// =========================================================================
Result_t dspif_AMRWB_record_pause (void)
{
	Log_DebugPrintf(LOGID_AUDIO, "dspif_AMRWB_play_pause: Pause AMRWB voice record\n");

	return RESULT_OK;
}


// ==========================================================================
//
// Function Name: dspif_AMRWB_record_resume
//
// Description: Resume the data transfer of AMRWB voice record
//
// =========================================================================
Result_t dspif_AMRWB_record_resume( void)
{
	Log_DebugPrintf(LOGID_AUDIO, "dspif_AMRWB_record_resume: Resume AMRWB voice record \n");

	return RESULT_OK;
}


// ==========================================================================
//
// Function Name: dspif_AMRWB_record_read
//
// Description: Read data to shared memory for AMRWB voice record.
//
// =========================================================================
UInt32 dspif_AMRWB_record_read ( UInt8 *inBuf, UInt32 inSize, UInt16 bufIndex, UInt32 numFramesPerInterrupt )
{
	SharedMem_t *sh_mem = SHAREDMEM_GetDsp_SharedMemPtr();

	// writeIndex: the current writing index (open)
	// arm will never set writeIndex and readIndex same.
	// if they are same, it means buffer is empty!!

	UInt16 size_copied, size_wraparound, totalCopied; 
	UInt32 frameSize = inSize;
	UInt8 *buffer = (UInt8* )&sh_mem->shared_encoder_OutputBuffer[bufIndex&0x0fff];
	
	Log_DebugPrintf(LOGID_AUDIO_DETAIL, "dspif_AMRWB_record_read: read request size = %d ##\n", inSize);

	totalCopied = frameSize;
	
	if (bufIndex + totalCopied/2 >= AUDIO_SIZE_PER_PAGE)//wrap around
	{
		// copy first part
		size_copied = (AUDIO_SIZE_PER_PAGE - bufIndex)<<1;
		memcpy (inBuf, buffer, size_copied);
		inBuf += size_copied;
		// copy second part
		size_wraparound = (totalCopied/2 + bufIndex - AUDIO_SIZE_PER_PAGE)<<1;
		memcpy (inBuf, buffer, size_wraparound);

		sh_mem->shared_encodedSamples_buffer_out[0] = totalCopied/2 + bufIndex - AUDIO_SIZE_PER_PAGE;
	}
	else // no wrap around
	{
		// just copy it from shared memeory
		size_copied = totalCopied;
		memcpy (inBuf, buffer, size_copied);

		sh_mem->shared_encodedSamples_buffer_out[0] += totalCopied/2;
	}

	// the bytes has been really copied.
    return totalCopied;
}

// ==========================================================================
//
// Function Name: dspif_AMRWB_record_GetReadIndex
//
// Description: Get the current read index of shared memory for AMRWB voice record.
//
// =========================================================================
UInt16 dspif_AMRWB_record_GetReadIndex (void)
{
    SharedMem_t *sh_mem = SHAREDMEM_GetDsp_SharedMemPtr();

    return sh_mem->shared_encodedSamples_buffer_out[0];
}

// ==========================================================================
//
// Function Name: dspif_VPU_record_set_cb
//
// Description: Start the data transfer of VPU record
//
// =========================================================================
Result_t dspif_VPU_record_set_cb (record_data_ready_cb_t* record_data_ready)

{
   record_ready = record_data_ready;
}

// ==========================================================================
//
// Function Name: dspif_VPU_record_handler
//
// Description: Start the data transfer of VPU record
//
// =========================================================================

static void dspif_VPU_record_handler(VPStatQ_t reqMsg)
{
    //call the record ready callback
    if(record_ready != NULL)
        record_ready(reqMsg.arg0);
}