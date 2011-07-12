/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2011  Broadcom Corporation                                                        */
/*                                                                                              */
/*     Unless you and Broadcom execute a separate written software license agreement governing  */
/*     use of this software, this software is licensed to you under the terms of the GNU        */
/*     General Public License version 2 (the GPL), available at                                 */
/*                                                                                              */
/*          http://www.broadcom.com/licenses/GPLv2.php                                          */
/*                                                                                              */
/*     with the following added to such license:                                                */
/*                                                                                              */
/*     As a special exception, the copyright holders of this software give you permission to    */
/*     link this software with independent modules, and to copy and distribute the resulting    */
/*     executable under terms of your choice, provided that you also meet, for each linked      */
/*     independent module, the terms and conditions of the license of that module.              */
/*     An independent module is a module which is not derived from this software.  The special  */
/*     exception does not apply to any modifications of the software.                           */
/*                                                                                              */
/*     Notwithstanding the above, under no circumstances may you combine this software in any   */
/*     way with any other Broadcom software provided under a license other than the GPL,        */
/*     without Broadcom's express prior written consent.                                        */
/*                                                                                              */
/************************************************************************************************/

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
								UInt32						numFramesPerInterrupt,
								UInt8						audMode);

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
									UInt32						numFramesPerInterrupt,
									UInt8						audMode);


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
