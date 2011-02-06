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
*    @file   hal_audio_core.h
*    @brief  API declaration of hardware abstraction layer for Audio driver.
*   This code is OS independent and Device independent for audio device control.
****************************************************************************/


#ifndef _HAL_AUDIO_CORE_H__
#define _HAL_AUDIO_CORE_H__
#include "hal_audio_access.h"
typedef struct
{
	HAL_AUDIO_ACTION_en_t  			actionCode;
	HAL_AUDIO_Control_Params_un_t	param;
	void								*callback;
} HAL_AudioCtrlMsg_t; 

//  internal messages
typedef struct
{
	HAL_AUDIO_ACTION_en_t  			actionCode;
} AudioMsg_t; 

//! Audio hardware events. (this enumeration is not finalized.)
typedef enum
{
	HAL_AUDIO_DEVICE_READY,	///< Audio device is ready.
	HAL_AUDIO_DEVICE_PD,	///< Audio device is powered down.
	HAL_AUDIO_DSP_TONE,     ///< Audio tone timer status
} HAL_AUDIO_EVENT_en_t;

//! Function prototype of callback for audio hardware events.
typedef void (*HAL_AUDIO_CB1)(void);
typedef void (*HAL_AUDIO_CB2)(UInt32 parm1);

Result_t HAL_AUDIO_Core_Init( HAL_AUDIO_CONFIG_st_t *config);


// !! A very special usage of audio controller. Don't go through HAL API. 
Result_t AUDIO_EnablePCMReadback( void	);
Result_t AUDIO_DisablePCMReadback( void	);


//!
//! We add a message handler for HAL_Audio_Ctrl API for those who want to have a task
//! and message queue to communicate with audio HAL.
//! 

//! Init the task and message queue to communicate with HAL.
void HAL_AUDIO_Ctrl_Handler_Init( void );
//! Handler for HAL_Audio_Ctrl API when want to use a task and message queue to communicate with audio HAL.
Result_t HAL_AUDIO_Ctrl_Handler(
	HAL_AUDIO_ACTION_en_t action_code,
	void *arg_param,
	void *callback
	);

/** @cond internal*/
// Obsolete API
//! Higher layer calls this function to register callback routine for some audio hardware events.
Result_t HAL_AUDIO_RegisterEventCB(
	HAL_AUDIO_EVENT_en_t	event,		///< (in) Hardware event type.
	HAL_AUDIO_CB2			callback	///< (in) Callback routine associated with the hardware event.
	);

/** @endcond */



//! Platform related part, user can defined action code and ID according to platform setting and their own taste
#define AUDIO_ID_MUSIC_PLAY  AUDIO_ID_AUDVOC
#define ACTION_AUD_SwitchPath ACTION_AUD_SetVolumeWithPath
#define ACTION_AUD_SetEqualizerBandGains   ACTION_AUD_SetEqualizer

#endif	//_HAL_AUDIO_CORE_H__


