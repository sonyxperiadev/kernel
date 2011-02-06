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
/**
*
*   @file   vpu.h
*
*   @brief  This file contains the Voice Processing Unit interface function prototypes and defines
*
****************************************************************************/

#ifndef _INC_VPU_H_
#define _INC_VPU_H_

#include "shared.h"
//--#include "mti_build.h"

#define MAX_TEMPLATE 32
#define MIN_VD_BLOCKS 2
#define MIN_THRESHOLD 5000

// --- VPU status return values

typedef enum {
	VPU_STATUS_OK,
	VPU_STATUS_DONE,
	VPU_STATUS_READY_TO_DIAL,
	VPU_STATUS_FAIL,
	VPU_STATUS_DB_CORRUPT,
	VPU_STATUS_FULL,
	VPU_STATUS_PAGE
} VPU_Status_t;

typedef Int16  VPU_Index_t;	// type used to  indentify  voice dialing and recording objects

// call back function to fill next playback buffer
typedef Boolean (*VPUFillFramesCB_t)(UInt8 *pBuf, UInt32 nSize);
typedef Boolean (*VPUDumpFramesCB_t)(UInt8 *pBuf, UInt32 nSize);

extern UInt8 playback_buffer_control;
//******************************************************************************
//  VPU interface function prototypes
//******************************************************************************

void VPU_Init( void );						// Initialize the voice processing unit

void VPU_StartCallRecording(				// Start call recording
	UInt8 	vp_record_mode,					// Audio path
	UInt8	buffer_length					// Buffer length
#if CHIPVERSION >= CHIP_VERSION(BCM2132,30) /* BCM2132C0 and later */
	,UInt8	vp_speech_amr_mode				// [7|6..4|3..0] = [AMR2_dtx|vp_speech_mode|vp_amr_mode]
	,UInt8 	downlink_audio_off				// 1: downlink audio off; 0: downlink audio on
#endif // #if CHIPVERSION >= CHIP_VERSION(BCM2132,30) /* BCM2132C0 and later */
	);

void VPU_StartCallRecordAndPlayback(		// starts continuous call recording and playback
	UInt8 	vp_playback_mode,				// playback to speaker/ear-piece, channel or both
	UInt8 	vp_record_mode,					// record from mic, channel or both
	UInt8	buffer_length					// Buffer length
	);

void VPU_StartMemolRecording(				// Start memo recording
	VP_Speech_Mode_t 	vp_speech_mode,
	UInt8				buffer_length
#if CHIPVERSION >= CHIP_VERSION(ML2021,00) && CHIPVERSION <= CHIP_VERSION(ML2021,99) /* BCM2121 */
	,UInt8	dtx_vp_amr_mode
#elif CHIPVERSION >= CHIP_VERSION(BCM2132,30) /* BCM2132C0 and later */
	,VP_Mode_AMR_t		vp_mode_amr
#endif // #elif CHIPVERSION >= CHIP_VERSION(BCM2132,30) /* BCM2132C0 and later */
	);


void VPU_StopRecording( void );				// Stop call/memo recording

void VPU_StartRecordingPlayback(   			// Start recording playback
	UInt8 	vp_playback_mode			   	// Audio path
#if CHIPVERSION >= CHIP_VERSION(ML2021,00) && CHIPVERSION <= CHIP_VERSION(ML2021,99) /* BCM2121 */
	,UInt8	disable_tch_vocoder				// TURE: disable TCH voice functions
	,UInt8 	downlink_audio_off				// 1: downlink audio off; 0: downlink audio on
#elif CHIPVERSION >= CHIP_VERSION(BCM2132,00) /* BCM2132 and later */
	,VP_Mode_AMR_t	vp_mode_amr				// amr codec mode
	,UInt8 	downlink_audio_off				// 1: downlink audio off; 0: downlink audio on
#if CHIPVERSION >= CHIP_VERSION(BCM2132,30) /* BCM2132C0 and later */
	,VP_PlaybackMix_Mode_t vp_playbackmix_mode		 // vp_playbackmix_mode = [0:3] = [non, ear_piece, UpCh, Both]
#endif // #if CHIPVERSION >= CHIP_VERSION(BCM2132,30) /* BCM2132C0 and later */
#endif // #elif CHIPVERSION >= CHIP_VERSION(BCM2132,00) /* BCM2132 and later */
	);

void VPU_StopPlayback( UInt8 flag );		// Stop  playback
void VPU_CancelPlayback( void );			// Cancel playback
void VPU_CancelRecording( void );			// Cancel recording
void VPU_ProcessStatus( void );				// Process  VP status queue
void VPU_Stop( void );						// Stop VPU process  for receiving page


Boolean VPU_StartRecordingMemo(				// start memo recording
	VPUDumpFramesCB_t	DumpFramesCB,		// callback function to dump recorded frame
	VP_Speech_Mode_t 	vp_speech_mode,		// speech mode
	UInt8				audioPath,			// audio path
	UInt32				nParam				// speech mode-specific parameters
											// for AMR this is VP_Mode_AMR_t
	);

Boolean VPU_StartPlaybackMemo(   			// Start memo playback
	VPUFillFramesCB_t	FillFramesCB,		// callback function to fill playback frame
	VP_Speech_Mode_t 	vp_speech_mode,		// speech mode
	UInt8				audioPath			// audio path
	);

Boolean VPU_StopMemo(void);

#if CHIPVERSION >= CHIP_VERSION(BCM2133,0) /* BCM2133 and later */
void VPU_AudioTurnOn(Boolean Uplink, Boolean Downlink);
void VPU_AudioTurnOff(void);
void VPU_PolyringerStartPlay_Test(UInt16 SynRate, UInt16 OutMode);   
void VPU_PolyringerCancelPlay_Test(UInt8 audioPath);   
#endif

#if CHIPVERSION >= CHIP_VERSION(BCM2132,32) /* BCM2132C2 (with patch) and later */
Boolean VPU_StartTelephony(
	VPUDumpFramesCB_t	telephony_dump_cb,	// callback function to pass AMR frame speech data encoded by DSP to application
	VPUFillFramesCB_t	telephony_fill_cb,	// callback function to pass AMR frame speech data encoded by DSP to application
	VP_Mode_AMR_t		encode_amr_mode,	// AMR mode for encoding the next speech frame
	Boolean				dtx_mode,			// Turn DTX on (TRUE) or off (FALSE)
	Boolean				amr_if2_enable		// Select AMR IF1 (FALSE) or IF2 (TRUE) format
	);
void VPU_StartMainAMRDecodeEncode(
	VP_Mode_AMR_t		decode_amr_mode,	// AMR mode for decoding the next speech frame
	UInt8				*pBuf,		// buffer carrying the AMR speech data to be decoded
	UInt8				length,		// number of bytes of the AMR speech data to be decoded
	VP_Mode_AMR_t		encode_amr_mode,	// AMR mode for encoding the next speech frame
	Boolean				dtx_mode	// Turn DTX on (TRUE) or off (FALSE)
	);
void VPU_ProcessStatusMainAMRDone(StatQ_t status_msg);
Boolean VPU_StopTelelphony(void);
#endif // #if CHIPVERSION >= CHIP_VERSION(BCM2132,32) /* BCM2132C2 (with patch) and later */

Boolean VoIP_StopTelephony(void);

Boolean VoIP_StartTelephony(
	VPUDumpFramesCB_t telephony_dump_cb,
	VPUDumpFramesCB_t telephony_fill_cb,
	UInt16	 voip_codec_type,  // AMR mode for encoding the next speech frame
	Boolean	       dtx_mode,	// Turn DTX on (TRUE) or off (FALSE): this is obsolete. contained in voip_codec_type
	Boolean	     amr_if2_enable	 // Select AMR IF1 (FALSE) or IF2 (TRUE) format: obsolete
	);


#if CHIPVERSION >= CHIP_VERSION(BCM2132,33) /* BCM2132C3 and later */
Boolean VPU_VT_Activ(void);
void VPU_VT_Clear(void);
UInt8 VPU_VT_enc_mode(void);
UInt8 VPU_VT_dec_mode(void);
UInt8 VPU_VT_len(void);
UInt32 *VPU_VT_ptr(void);
#endif

#if ((defined(_BCM2153_) && CHIP_REVISION >= 51))
void VPU_NS_Set(Boolean vpu_ns_status);
void VPU_NS_ParmTuning(Boolean vpu_ns_tuning); // 12192008 michael
#endif

typedef enum
{
  VPU_LINK_NEAR=1,
  VPU_LINK_FAR,
  VPU_LINK_BOTH
} VPU_Link_t;


Boolean VPU_EnableI2S_From_ADC(VPU_Link_t vpu_link);
Boolean VPU_DisableI2S_From_ADC(void);

typedef void (*GP_CB_FUNC)(void);

void GPTimerRegister(GP_CB_FUNC gpt_cb);
void GPTimerUnRegister(void);

#endif
