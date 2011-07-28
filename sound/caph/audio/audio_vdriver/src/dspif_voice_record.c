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
#ifdef CONFIG_AUDIO_BUILD
#include "sysparm.h"
#endif
#include "audio_consts.h"
#include "auddrv_def.h"
#include "drv_caph.h"
#include "dspif_voice_record.h"
#include "sharedmem.h"
#include "dspcmd.h"
#include "log.h"
#include "csl_apcmd.h"
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
static capture_data_cb_t capture_cb = NULL;

//
// APIs of VPU
//


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
	
	// restrict numFramesPerInterrupt due to the shared memory size 
	if (numFramesPerInterrupt > 4)
			numFramesPerInterrupt = 4;

	Log_DebugPrintf(LOGID_AUDIO, " dspif_VPU_record_start::Start capture, encodingMode = 0x%x, recordMode = 0x%x, procEnable = 0x%x, dtxEnable = 0x%x, speechMode = 0x%lx, dataRate = 0x%lx\n", 
							encodingMode, recordMode, procEnable, dtxEnable, speechMode, dataRate);
	
	VPRIPCMDQ_StartCallRecording((UInt8)recordMode, (UInt8)numFramesPerInterrupt, (UInt16)encodingMode);

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
// Function Name: dspif_VPU_record_set_cb
//
// Description: set the callback for voice recording
//
// =========================================================================

void dspif_VPU_record_set_cb (capture_data_cb_t capture_data_cb)
{
	Log_DebugPrintf(LOGID_AUDIO, "dspif_VPU_record_set_cb\n");

	capture_cb = capture_data_cb;
}

// ===================================================================
//
// Function Name: VPU_Capture_Request
//
// Description: Send a VPU capture request for voice capture driver to copy
// data from DSP shared memory.
//
// ====================================================================
void VPU_Capture_Request(UInt16 bufferIndex)
{
	//Log_DebugPrintf(LOGID_AUDIO, " VPU_Capture_Request:: capture interrupt callback.bufferIndex %d\n",bufferIndex);
	if(capture_cb != NULL)
		capture_cb(bufferIndex);
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

