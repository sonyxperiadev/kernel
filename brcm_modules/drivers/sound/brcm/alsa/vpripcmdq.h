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

#include "shared.h"

void VPRIPCMDQ_StartTraining(void);

void VPRIPCMDQ_CancelTraining(void);

void VPRIPCMDQ_StartRecognition(
	UInt16 nr_names
	);

void VPRIPCMDQ_CancelRecognition(void);

void VPRIPCMDQ_StartRecognitionPlayback(void);

void VPRIPCMDQ_StartCallRecording(
	UInt8 	vp_record_mode,			// Audio path
	UInt8	buffer_length			// Buffer length
#if CHIPVERSION >= CHIP_VERSION(BCM2132,00) /* BCM2132 and later */
		,
#if ((defined(_BCM2153_) && CHIP_REVISION >= 51)) //for 2153 e1 and later, bit8 was used to control NS
	UInt16	vp_speech_amr_mode		//[8] [7..4|3..0] = [ul_ns] [vp_speech_mode|vp_amr_mode]
#else
	UInt8	vp_speech_amr_mode		// [7..4|3..0] = [vp_speech_mode|vp_amr_mode]
#endif //#if ((defined(_BCM2153_) && CHIP_REVISION >= 51))
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
#if ((defined(_BCM2153_) && CHIP_REVISION >= 51)) //for 2153 e1 and later, bit8 was used to control NS
		UInt16				vp_mode_amr
#else
		VP_Mode_AMR_t		vp_mode_amr
#endif // #if ((defined(_BCM2153_) && CHIP_REVISION >= 51))
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

#if CHIPVERSION >= CHIP_VERSION(BCM2133,0) /* BCM2133 and later */
void VPRIPCMDQ_PolyringerStartPlay(UInt16 BlockSamples, UInt16 OutMode);

void VPRIPCMDQ_PolyringerStopPlay(void);

void VPRIPCMDQ_PolyringerCancelPlay(void);
#endif  /* BCM2133 and later */

#endif
