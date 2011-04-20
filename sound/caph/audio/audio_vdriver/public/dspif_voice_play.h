/*****************************************************************************
*
*    (c) 2009 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or
* its licensors, and may only be used, duplicated, modified or distributed
* pursuant to the terms and conditions of a separate, written license
* agreement executed between you and Broadcom (an "Authorized License").
* Except as set forth in an Authorized License, Broadcom grants no license
* (express or implied), right to use, or waiver of any kind with respect to
* the Software, and Broadcom expressly reserves all rights in and to the
* Software and all intellectual property rights therein.
* IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS
* SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE
* ALL USE OF THE SOFTWARE.  
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
*    IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS
*    FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS,
*    QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU
*    ASSUME THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
*
*****************************************************************************/
/**
*
*  @file  dspif_voice_play.h
*
*  @brief DSP common interface APIs for voice play
*
*  @note
*****************************************************************************/
/**
*
* @defgroup Audio    Audio Component
*
* @brief    This group defines the common APIs for audio virtual driver
*
* @ingroup  Audio Component
*****************************************************************************/


#ifndef	__DSPIF_VOICE_PLAY_H__
#define	__DSPIF_VOICE_PLAY_H__

/**
*
* @addtogroup Audio
* @{
*/

#ifdef __cplusplus
extern "C" {
#endif


typedef enum VORENDER_PLAYBACK_MODE_t
{
	VORENDER_PLAYBACK_NONE,
	VORENDER_PLAYBACK_DL,
	VORENDER_PLAYBACK_UL,
	VORENDER_PLAYBACK_BOTH
} VORENDER_PLAYBACK_MODE_t;
						

typedef enum VORENDER_VOICE_MIX_MODE_t
{
	VORENDER_VOICE_MIX_NONE,
	VORENDER_VOICE_MIX_DL,
	VORENDER_VOICE_MIX_UL,
	VORENDER_VOICE_MIX_BOTH
} VORENDER_VOICE_MIX_MODE_t;

typedef enum
{
	VORENDER_ARM2SP_INSTANCE_NONE,
	VORENDER_ARM2SP_INSTANCE1,
	VORENDER_ARM2SP_INSTANCE2,
	VORENDER_ARM2SP_INSTANCE_TOTAL
} VORENDER_ARM2SP_INSTANCE_e;

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
								UInt32						numFramesPerInterrupt
								);

// ==========================================================================
//
// Function Name: dspif_VPU_play_stop
//
// Description: Stop immediately the data transfer of VPU voic play
//
// =========================================================================
Result_t dspif_VPU_play_stop ( void);


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
								UInt32						numFramesPerInterrupt);

// ==========================================================================
//
// Function Name: dspif_ARM2SP_play_stop
//
// Description: Stop immediately the data transfer of ARM2SP voic play
//
// =========================================================================
Result_t dspif_ARM2SP_play_stop ( UInt32 instanceID);


// ==========================================================================
//
// Function Name: dspif_ARM2SP_play_pause
//
// Description: Pause the data transfer of ARM2SP voice play
//
// =========================================================================
Result_t dspif_ARM2SP_play_pause (UInt32 instanceID);


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
									UInt32						numFramesPerInterrupt);


// ==========================================================================
//
// Function Name: dspif_ARM2SP_play_flush
//
// Description: Flush the shared memory of ARM2SP voice play
//
// =========================================================================
Result_t dspif_ARM2SP_play_flush( UInt32 instanceID);

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
								UInt32						numFramesPerInterrupt);

// ==========================================================================
//
// Function Name: dspif_AMRWB_play_init_stop
//
// Description: Tell DSP to stop AMRWB voic play
//
// =========================================================================
Result_t dspif_AMRWB_play_init_stop ( void);

// ==========================================================================
//
// Function Name: dspif_AMRWB_play_stop
//
// Description: Stop immediately the data transfer of AMRWB voic play
//
// =========================================================================
Result_t dspif_AMRWB_play_stop ( void);

#ifdef __cplusplus
}
#endif



#endif // __DSPIF_VOICE_PLAY_H__
