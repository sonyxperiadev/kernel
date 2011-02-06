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
*    @file   hal_audio_enum.h
*    @brief  API declaration of hardware abstraction layer for Audio driver.
*   This code is OS independent and Device independent for audio device control.
****************************************************************************/


#ifndef _HAL_AUDIO_ENUM_H__
#define _HAL_AUDIO_ENUM_H__

//! The higher layer calls this Audio hardware abstraction layer to perform the following actions. This is expandable
//! if audio controller need to handle more requests.

typedef enum
{
	ACTION_AUD_MuteSpeaker,		///< Mute speaker output
	ACTION_AUD_UnmuteSpeaker,	///< Un-mute speaker output
	ACTION_AUD_MuteMic,			///< Mute microphone input
	ACTION_AUD_UnmuteMic,		///< Un-mute microphone input
	ACTION_AUD_SetMicGain,      ///<Element type {::HAL_AUDIO_Param_MicGain_t}, Set Mic Gain
	
	ACTION_AUD_PlayTone,		///<Element type {::HAL_AUDIO_Param_Tone_t}, DSP generates DTMF tone or Supervisory tone
	ACTION_AUD_PlayGenericTone, ///<Element type {::HAL_AUDIO_Param_GenericTone_t}, DSP generates generic tone
	ACTION_AUD_StopPlayTone,	///< Stop DSP tone generation

	ACTION_AUD_SetSpeakerVol,	///<Element type {::HAL_AUDIO_Param_SpkrVol_t}, Change speaker gain setting for current audio ID and output channel.
	ACTION_AUD_SetVolumeWithPath, ///<Element type {::HAL_AUDIO_Param_PathCfg_t}, Change audio path settings (volume, output channel)

	ACTION_AUD_EnablePath,		///<Element type {::HAL_AUDIO_Param_PathCfg_t}, Enable the audio path(select the audio mode and volume, turn on speaker/mic)
	ACTION_AUD_DisablePath, 	///<Element type {::HAL_AUDIO_Param_PathCfg_t}, Disable audio path(turn off speaker/mic and set volume to 0)
	
	ACTION_AUD_ConfigCodecParam,	///<Element type {::HAL_AUDIO_Param_CodecCfg_t}, Config audio codec settings (sampling rate, bits per sample)
	ACTION_AUD_SetEqualizer,	///<Element type {::HAL_AUDIO_Param_Equalizer_t}, Change equalizer's band gains
	ACTION_AUD_SetEqualizerProfile,	///<Element type {::HAL_AUDIO_Param_EqualizerProfile_t}, Change equalizer's profile (a set of band gains and filter coefficients)
	ACTION_AUD_SetSTWidening,		///<Element type {::HAL_AUDIO_Param_STW_t}, Change stereo widening settigns 
	ACTION_AUD_SetMixer,	    ///<Element type {::HAL_AUDIO_Param_Mixer_t}, Change mixer settings	

	ACTION_AUD_GetParam,		///<Element type {::HAL_AUDIO_Get_Param_t}, Get parameters of audio controller
#if defined(SS_2153)
	ACTION_AUD_SetCustomGain,	///< Set customer's gain table for SPEAKER_PGA, MIC_PGA, and MIXER_GAIN // 02062009 michael
#endif // SS_2153
	ACTION_AUD_CtrlAudAlg,      ///<Element type {::HAL_AUDIO_Param_AlgCfg_t}, Config dsp algorithms such as EC/NS for BT devices
#ifdef FEATURE_AUDIO_VOICECALL16K
	ACTION_AUD_RateChange,      ///<Element type {::HAL_AUDIO_Param_RateCfg_t},
#endif // FEATURE_AUDIO_VOICECALL16K
	ACTION_AUD_TUNE,			///< Set the tuning parameter to hardware
	ACTION_AUD_TOTAL			///< A placeholder
} HAL_AUDIO_ACTION_en_t;

#endif	//_HAL_CORE_ENUM_H_
