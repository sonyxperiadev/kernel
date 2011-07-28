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
*  @file  dspif_voice_record.h
*
*  @brief DSP common interface APIs for voice record
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


#ifndef	__DSPIF_VOICE_RECORD_H__
#define	__DSPIF_VOICE_RECORD_H__

/**
*
* @addtogroup Audio
* @{
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef enum VOCAPTURE_RECORD_MODE_t
{
	VOCAPTURE_RECORD_NONE,
	VOCAPTURE_RECORD_UL,		
	VOCAPTURE_RECORD_DL,
	VOCAPTURE_RECORD_BOTH
} VOCAPTURE_RECORD_MODE_t;
						

typedef enum VOCAPTURE_VOICE_MIX_MODE_t
{
	VOCAPTURE_VOICE_MIX_NONE,
	VOCAPTURE_VOICE_MIX_DL,
	VOCAPTURE_VOICE_MIX_UL,
	VOCAPTURE_VOICE_MIX_BOTH
} VOCAPTURE_VOICE_MIX_MODE_t;

typedef void (*capture_data_cb_t)(UInt16 buf_index);



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
								Boolean						dexEnable,
								UInt32						numFramesPerInterrupt);

// ==========================================================================
//
// Function Name: dspif_VPU_record_stop
//
// Description: Stop immediately the data transfer of VPU voic record
//
// =========================================================================
Result_t dspif_VPU_record_stop ( void);


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
								Boolean						dexEnable,
								UInt32						numFramesPerInterrupt);


// ==========================================================================
//
// Function Name: dspif_AMRWB_record_stop
//
// Description: Stop immediately the data transfer of AMRWB voic record
//
// =========================================================================
Result_t dspif_AMRWB_record_stop ( void);


// ==========================================================================
//
// Function Name: dspif_VPU_record_pause
//
// Description: Pause the data transfer of VPU voice record
//
// =========================================================================
Result_t dspif_VPU_record_pause (void);

// ==========================================================================
//
// Function Name: dspif_VPU_record_resume
//
// Description: Resume the data transfer of VPU voice record
//
// =========================================================================
Result_t dspif_VPU_record_resume( void);

// ==========================================================================
//
// Function Name: dspif_VPU_record_set_cb
//
// Description: set the callback for voice recording
//
// =========================================================================
void dspif_VPU_record_set_cb (capture_data_cb_t capture_data_cb);


// ===================================================================
//
// Function Name: VPU_Capture_Request
//
// Description: Send a VPU capture request for voice capture driver to copy
// data from DSP shared memory.
//
// ====================================================================
void VPU_Capture_Request(UInt16 bufferIndex);


#ifdef __cplusplus
}
#endif



#endif // __DSPIF_VOICE_PLAY_H__
