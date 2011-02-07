//***************************************************************************
//
//	Copyright © 2005-2008 Broadcom Corporation
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
*   @file   dsp_vpu_test.h
*
*   @brief  This file contains the Voice Processing Unit interface function prototypes and defines for DSP VPU testing
*
****************************************************************************/

#ifndef _INC_DSP_VPU_TEST_H_
#define _INC_DSP_VPU_TEST_H_

#include "sharedmem.h"
#include "mti_build.h"

//#define DSP_VPU_TEST_HARNESS

#ifdef DSP_VPU_TEST_HARNESS
extern UInt8	dspvputest_harness_flag;

//******************************************************************************
//  VPU interface function prototypes
//******************************************************************************
void DSP_VPU_StartMemolRecording(				// Start memo recording
	VP_Speech_Mode_t 	vp_speech_mode,
	UInt8				buffer_length,
	UInt16		vp_mode_amr					//[b8|b7|b6:b0] = [NS_AUD_Proc_enable|AMR_DTX_enable|AMR_mode], [b7:b0] not used
	);

void DSP_VPU_StartCallRecording(				// Start call recording 
	UInt8 	vp_record_mode,					// Audio path
	UInt8	buffer_length,					// Buffer length
	UInt16	vp_speech_amr_mode				// [8|7|6..4|3..0] = [audio_proc_enable|AMR2_dtx|vp_speech_mode|vp_amr_mode]
	);

void DSP_VPU_StartRecordingPlayback(   			// Start recording playback
	UInt8 	vp_playback_mode,			   	// Audio path
	VP_PlaybackMix_Mode_t vp_playbackmix_mode		 // vp_playbackmix_mode = [0:3] = [mix_none, mix_ear_piece, mix_UpCh, mix_Both]
	);

void DSP_VPU_StartCallRecordAndPlayback(		// starts continuous call recording and playback
	UInt8 	vp_playback_mode,				// playback to speaker/ear-piece, channel or both
	UInt8 	vp_record_mode,					// record from mic, channel or both
	UInt8	buffer_length					// Buffer length    
	);

void DSP_VPU_StopRecording( void );				// Stop call/memo recording

void DSP_VPU_StopPlayback( UInt8 flag );		// Stop  playback

void DSP_VPU_ProcessStatus( void );	

#endif //#ifdef DSP_VPU_TEST_HARNESS

#endif
