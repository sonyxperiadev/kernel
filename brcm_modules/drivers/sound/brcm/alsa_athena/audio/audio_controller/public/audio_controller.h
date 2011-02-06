/******************************************************************************
Copyright 2009 Broadcom Corporation.  All rights reserved.

This program is the proprietary software of Broadcom Corporation and/or its 
licensors, and may only be used, duplicated, modified or distributed pursuant 
to the terms and conditions of a separate, written license agreement executed 
between you and Broadcom (an "Authorized License").

Except as set forth in an Authorized License, Broadcom grants no license
(express or implied), right to use, or waiver of any kind with respect to the 
Software, and Broadcom expressly reserves all rights in and to the Software and 
all intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, 
THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY 
NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
  
 Except as expressly set forth in the Authorized License,
1. This program, including its structure, sequence and organization, 
constitutes the valuable trade secrets of Broadcom, and you shall use all 
reasonable efforts to protect the confidentiality thereof, and to use this 
information only in connection with your use of Broadcom integrated circuit 
products.

2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" 
AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR 
WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO 
THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES 
OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, 
LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION 
OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF 
USE OR PERFORMANCE OF THE SOFTWARE.

3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS 
LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR 
EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR USE 
OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE 
POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT 
ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE 
LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF 
ANY LIMITED REMEDY.
******************************************************************************/
/**
*
* @file   audio_controller.h
* @brief  Audio Controller interface
*
*****************************************************************************/
#ifndef __AUDIO_CONTROLLER_H__
#define __AUDIO_CONTROLLER_H__

/**
 * @addtogroup Audio_Controller 
 * @{
 */

typedef enum AUDIO_HW_ID_t
{
	AUDIO_HW_NONE,
	AUDIO_HW_MEM,
	AUDIO_HW_VOICE_OUT,			
	AUDIO_HW_AUDIO_OUT,
	AUDIO_HW_PLR_OUT,			
	AUDIO_HW_MONO_BT_OUT,	
	AUDIO_HW_STEREO_BT_OUT,		
	AUDIO_HW_USB_OUT,	
	AUDIO_HW_I2S_OUT,		
	AUDIO_HW_VOICE_IN,
	AUDIO_HW_AUDIO_IN,			
	AUDIO_HW_MONO_BT_IN,		
	AUDIO_HW_STEREO_BT_IN,		
	AUDIO_HW_USB_IN,	
	AUDIO_HW_I2S_IN,
	AUDIO_HW_TAP_VOICE,
	AUDIO_HW_TAP_AUDIO,
	AUDIO_HW_DSP_VOICE,
	AUDIO_HW_DSP_TONE,
    AUDIO_HW_EARPIECE_OUT,
    AUDIO_HW_HEADSET_OUT,
    AUDIO_HW_IHF_OUT,
    AUDIO_HW_SPEECH_IN,
    AUDIO_HW_NOISE_IN,
	AUDIO_HW_TOTAL_COUNT
} AUDIO_HW_ID_t;


typedef enum AUDCTRL_SPEAKER_t
{
	AUDCTRL_SPK_HANDSET,
	AUDCTRL_SPK_HEADSET,
	AUDCTRL_SPK_HANDSFREE,
	AUDCTRL_SPK_BTM,
	AUDCTRL_SPK_LOUDSPK,
	AUDCTRL_SPK_TTY,
	AUDCTRL_SPK_HAC,	
	AUDCTRL_SPK_USB,
	AUDCTRL_SPK_BTS,
	AUDCTRL_SPK_I2S,
	AUDCTRL_SPK_VIBRA,
	AUDCTRL_SPK_UNDEFINED,
	AUDCTRL_SPK_TOTAL_COUNT
} AUDCTRL_SPEAKER_t;

typedef enum AUDCTRL_MIC_Enum_t
{
	AUDCTRL_MIC_UNDEFINED,
	AUDCTRL_MIC_MAIN,
	AUDCTRL_MIC_AUX,
	AUDCTRL_MIC_DIGI1,
	AUDCTRL_MIC_DIGI2,
	AUDCTRL_DUAL_MIC_DIGI12,
	AUDCTRL_DUAL_MIC_DIGI21,
	AUDCTRL_DUAL_MIC_ANALOG_DIGI1,
	AUDCTRL_DUAL_MIC_ANALOG_DIGI2,
	AUDCTRL_MIC_BTM,  //Bluetooth Mono Headset Mic
	//AUDCTRL_MIC_BTS,  //not exist
	AUDCTRL_MIC_USB,  //USB headset Mic
	AUDCTRL_MIC_I2S,
    AUDCTRL_MIC_DIGI3, //Only for loopback path
	AUDCTRL_MIC_DIGI4, //Only for loopback path
	AUDCTRL_MIC_DIGI, //Digital Mic1 and Mic2(if applicable) in recording/Normal Quality Voice call.
	AUDCTRL_MIC_EANC_DIGI, //Digital Mic1/2/3/4 for Supreme Quality Voice Call.
	AUDCTRL_MIC_TOTAL_COUNT
} AUDCTRL_MIC_Enum_t;

#define AUDCTRL_MICROPHONE_t AUDCTRL_MIC_Enum_t  //need to merge with AUDDRV_MIC_Enum_t
/**
#if !defined(_ATHENA_)
//Hera??
typedef enum {
    AUDDRV_MIC_NONE,
    AUDDRV_MIC_AMIC    = 0,	    // Analog MIC/Handset   
	AUDDRV_MIC_DMIC1   = 1,	    // DMIC 1
    AUDDRV_MIC_DMIC2   = 2,	    // DMIC 2
    AUDDRV_MIC_DMIC3   = 4,	    // DMIC 3 
    AUDDRV_MIC_DMIC4   = 8,	    // DMIC 4 
    AUDDRV_MIC_AUXMIC  = 0x10	// Analog MIC/Headset   
 } AUDDRV_MIC_Enum_t;
#endif

  Athena:
  typedef enum {
    AUDDRV_MIC_NONE,
    AUDDRV_MIC_MAIN,
    AUDDRV_MIC_AUX,
	AUDDRV_MIC_PCM_IF,
    AUDDRV_MIC_DIGI1,  // DMIC1
    AUDDRV_MIC_DIGI2,  // DMIC2
	AUDDRV_DUAL_MIC_DIGI12,  // DMIC1 and DMIC2
	AUDDRV_DUAL_MIC_DIGI21,  // DMIC1 and DMIC2
	AUDDRV_DUAL_MIC_ANALOG_DIGI1,
	AUDDRV_DUAL_MIC_ANALOG_DIGI2,
    AUDDRV_MIC_USB_IF,
	AUDDRV_MIC_TOTAL_NUM
} AUDDRV_MIC_Enum_t;

  CAPH AUDIOH Mics
typedef enum 
{
    AUDDRV_MIC_ANALOG = 0,	// Analog MIC/Headset
    AUDDRV_MIC_PRI = 1,		// DMIC 1
    AUDDRV_MIC_AUX = 2,		// DMIC 2
    AUDDRV_MIC_EF = 3,		// DMIC 3 
    AUDDRV_MIC_NREF = 4,	// DMIC 4 
} AUDDRV_MIC_Enum_t;

  merge to:
    AUDDRV_MIC_NONE,
    AUDDRV_MIC_ANA_MAIN,
    AUDDRV_MIC_ANA_AUX,
	AUDDRV_MIC_PCM_IF,
    AUDDRV_MIC_DIGI1,  // DMIC1
    AUDDRV_MIC_DIGI2,  // DMIC2
	AUDDRV_DUAL_MIC_DIGI12,  // DMIC1 and DMIC2
	AUDDRV_DUAL_MIC_DIGI21,  // DMIC1 and DMIC2
	AUDDRV_DUAL_MIC_ANALOG_DIGI1,
	AUDDRV_DUAL_MIC_ANALOG_DIGI2,

	AUDDRV_MIC_EF = 3,		// DMIC 3 
    AUDDRV_MIC_NREF = 4,	// DMIC 4 

    AUDDRV_MIC_USB_IF,
	AUDDRV_MIC_TOTAL_NUM

**/

typedef enum {
   TelephonyUseExtSpkr,
   VoiceUseExtSpkr,
   AudioUseExtSpkr,
   PolyUseExtSpkr
} ExtSpkrUsage_en_t;

/**
*  @brief  This function is the Init entry point for Audio Controller 
*
*  @param  none
*
*  @return none
*
****************************************************************************/
void AUDCTRL_Init (void);

/**
*  @brief  This function controls the power on/off of external
*          amplifier
*
*  @param  speaker	    (in)  speaker selection 
*  @param  usage_flag	(in)  external speaker usage
*  @param  use		    (in)  on/off
*
*  @return none
*
****************************************************************************/
void powerOnExternalAmp( AUDCTRL_SPEAKER_t speaker, ExtSpkrUsage_en_t usage_flag, Boolean use );

/**
*  @brief  Enable telephony audio path in HW and DSP
*
*  @param  ulSrc	(in)  uplink source 
*  @param  dlSink	(in)  downlink sink
*  @param  mic		(in)  microphone selection
*  @param  speaker	(in)  speaker selection
*
*  @return none
*
****************************************************************************/
void AUDCTRL_EnableTelephony(
				AUDIO_HW_ID_t			ulSrc,
				AUDIO_HW_ID_t			dlSink,
				AUDCTRL_MICROPHONE_t	mic,
				AUDCTRL_SPEAKER_t		speaker
				);

/**
*  @brief  Rate change telephony audio for DSP
*
*
*  @return none
*
****************************************************************************/
void AUDCTRL_RateChangeTelephony( void );

/**
*  @brief  Disable telephony audio path in HW and DSP
*
*  @param  ulSrc	(in)  uplink source 
*  @param  dlSink	(in)  downlink sink
*  @param  mic		(in)  microphone selection
*  @param  speaker	(in)  speaker selection
*
*  @return none
*
****************************************************************************/
void AUDCTRL_DisableTelephony(
				AUDIO_HW_ID_t			ulSrc,
				AUDIO_HW_ID_t			dlSink,
				AUDCTRL_MICROPHONE_t	mic,
				AUDCTRL_SPEAKER_t		speaker
				);

/**
*  @brief  Change telephony audio path in HW and DSP
*
*  @param  ulSrc	(in)  uplink source 
*  @param  dlSink	(in)  downlink sink
*  @param  mic		(in)  microphone selection
*  @param  speaker	(in)  speaker selection
*
*  @return none
*
****************************************************************************/
void AUDCTRL_SetTelephonyMicSpkr(
				AUDIO_HW_ID_t			ulSrc,
				AUDIO_HW_ID_t			dlSink,
				AUDCTRL_MICROPHONE_t	mic,
				AUDCTRL_SPEAKER_t		speaker
				);

/**
*  @brief  Set telephony speaker (downlink) volume
*
*  @param  dlSink	(in)  downlink sink
*  @param  speaker	(in)  speaker selection
*  @param  volume	(in)  downlink volume
*
*  @return none
*
****************************************************************************/
void AUDCTRL_SetTelephonySpkrVolume(
				AUDIO_HW_ID_t			dlSink,
				AUDCTRL_SPEAKER_t		speaker,
				UInt32					volume,
				AUDIO_GAIN_FORMAT_t		gain_format
				);

/**
*  @brief  Set telephony speaker (downlink) mute / un-mute
*
*  @param  dlSink	(in)  downlink sink
*  @param  speaker	(in)  speaker selection
*  @param  mute		(in)  TRUE: mute;    FALSE: un-mute
*
*  @return none
*
****************************************************************************/
void AUDCTRL_SetTelephonySpkrMute(
				AUDIO_HW_ID_t			dlSink,
				AUDCTRL_SPEAKER_t		speaker,
				Boolean					mute
				);

/**
*  @brief  Set telephony microphone (uplink) gain
*
*  @param  dlSink	(in)  downlink sink
*  @param  mic		(in)  microphone selection
*  @param  speaker	(in)  speaker selection
*
*  @return none
*
****************************************************************************/
void AUDCTRL_SetTelephonyMicGain(
				AUDIO_HW_ID_t			ulSrc,
				AUDCTRL_MICROPHONE_t	mic,
				Boolean					gain
				);

/**
*  @brief  Set telephony mic (uplink) mute / un-mute
*
*  @param  ulSrc	(in)  uplink source 
*  @param  mic		(in)  microphone selection
*  @param  mute		(in)  TRUE: mute;    FALSE: un-mute
*
*  @return none
*
****************************************************************************/
void AUDCTRL_SetTelephonyMicMute(
				AUDIO_HW_ID_t			ulSrc,
				AUDCTRL_MICROPHONE_t	mic,
				Boolean					mute
				);

/**
*  @brief  Check whether in voice call mode.
*
*  @param  none
*
*  @return TRUE or FALSE
*
****************************************************************************/
Boolean AUDCTRL_InVoiceCall( void );

/**
*  @brief  Check whether in WB voice call mode.
*
*  @param  none
*
*  @return TRUE or FALSE
*
****************************************************************************/
Boolean AUDCTRL_InVoiceCallWB( void );

/**
*   Get current (voice call) audio mode 
*
*	@param		none
*
*	@return		AudioMode_t		(voice call) audio mode 
*
*   @note      
****************************************************************************/
AudioMode_t AUDCTRL_GetAudioMode( void );

/**
*  Save audio mode before call AUDCTRL_SetAudioMode( )
*	@param		mode		(voice call) audio mode 
*	@return		none
****************************************************************************/
void AUDCTRL_SaveAudioModeFlag( AudioMode_t mode );

/**
*   Set (voice call) audio mode 
*
*	@param		mode		(voice call) audio mode 
*
*	@return		none
****************************************************************************/
void AUDCTRL_SetAudioMode( AudioMode_t mode );

/**
*  @brief  Enable a playback path
*
*  @param  src	(in)  playback source 
*  @param  sink	(in)  playback sink
*  @param  spk	(in)  speaker selection
*  @param  numCh	(in)  stereo, momo
*  @param  sr	(in)  sample rate
*
*  @return none
*
****************************************************************************/
void AUDCTRL_EnablePlay(
				AUDIO_HW_ID_t			src,
				AUDIO_HW_ID_t			sink,
				AUDIO_HW_ID_t			tap,
				AUDCTRL_SPEAKER_t		spk,
				AUDIO_CHANNEL_NUM_t		numCh,
				AUDIO_SAMPLING_RATE_t	sr
				);

/********************************************************************
*  @brief  Disable a playback path
*
*  @param  src	(in)  playback source 
*  @param  sink	(in)  playback sink
*  @param  speaker	(in)  speaker selection
*
*  @return none
*
****************************************************************************/
void AUDCTRL_DisablePlay(
				AUDIO_HW_ID_t			src,
				AUDIO_HW_ID_t			sink,
				AUDCTRL_SPEAKER_t		spk
				);

/********************************************************************
*  @brief  Set playback volume
*
*  @param  sink	(in)  playback sink
*  @param  speaker	(in)  speaker selection
*  @param  vol	(in)  volume to set
*
*  @return none
*
****************************************************************************/
void AUDCTRL_SetPlayVolume(
				AUDIO_HW_ID_t			sink,
				AUDCTRL_SPEAKER_t		spk,
				AUDIO_GAIN_FORMAT_t     gainF,
				UInt32					vol_left,
				UInt32					vol_right
				);

/********************************************************************
*  @brief  mute/unmute playback 
*
*  @param  sink	(in)  playback sink 
*  @param  speaker	(in)  speaker selection
*  @param  vol	(in)  TRUE: mute, FALSE: unmute
*
*  @return none
*
****************************************************************************/
void AUDCTRL_SetPlayMute(
				AUDIO_HW_ID_t			sink,
				AUDCTRL_SPEAKER_t		spk,
				Boolean					mute
				);

/********************************************************************
*  @brief  Add a speaker to a playback path
*
*  @param  sink	(in)  playback sink
*  @param  speaker	(in)  speaker selection
*
*  @return none
*
****************************************************************************/
void AUDCTRL_AddPlaySpk(
				AUDIO_HW_ID_t			sink,
				AUDCTRL_SPEAKER_t		spk
				);

/********************************************************************
*  @brief  Remove a speaker to a playback path
*
*  @param  sink	(in)  playback sink  
*  @param  speaker	(in)  speaker selection
*
*  @return none
*
****************************************************************************/
void AUDCTRL_RemovePlaySpk(
				AUDIO_HW_ID_t			sink,
				AUDCTRL_SPEAKER_t		spk
				);


/********************************************************************
*  @brief  enable a record path
*
*  @param  src	(in)  record source
*  @param  sink	(in)  record sink
*  @param  speaker	(in)  speaker selection
*  @param  numCh	(in)  stereo, momo
*  @param  sr	(in)  sample rate
*
*  @return none
*
****************************************************************************/
void AUDCTRL_EnableRecord(
				AUDIO_HW_ID_t			src,
				AUDIO_HW_ID_t			sink,
				AUDCTRL_MICROPHONE_t	mic,
				AUDIO_CHANNEL_NUM_t		numCh,
				AUDIO_SAMPLING_RATE_t	sr
				);

/********************************************************************
*  @brief  disable a record path
*
*  @param  src	(in)  record source
*  @param  sink	(in)  record sink
*  @param  mic	(in)  microphone selection
*
*  @return none
*
****************************************************************************/
void AUDCTRL_DisableRecord(
				AUDIO_HW_ID_t			src,
				AUDIO_HW_ID_t			sink,
				AUDCTRL_MICROPHONE_t	mic
				);

/********************************************************************
*  @brief  Set gain of a record path
*
*  @param  src	(in)  record source
*  @param  mic	(in)  microphone selection
*  @param  gainL	(in)  the left channel gain to set
*  @param  gainR	(in)  the right channel gain to set
*
*  @return none
*
****************************************************************************/
void AUDCTRL_SetRecordGain(
				AUDIO_HW_ID_t			src,
				AUDCTRL_MICROPHONE_t	mic,
				UInt32					gainL,
				UInt32					gainR
				);

/**
*  @brief  Ensable Tap (wideband or voice)
*
*  @param  src	(in)  
*  @param  sink	(in)  
*  @param  spk	(in)  speaker selection
*  @param  numCh	(in)  stereo, momo
*  @param  sr	(in)  sample rate
*
*  @return none
*
****************************************************************************/
void AUDCTRL_EnableTap(
				AUDIO_HW_ID_t			tap,
				AUDCTRL_SPEAKER_t		spk,
				AUDIO_SAMPLING_RATE_t	sr
				);

/**
*  @brief  Disable Tap (wideband or voice)
*
*  @param  src	(in)  
*  @param  sink	(in)  
*  @param  spr	(in)  speaker selection
*
*  @return none
*
****************************************************************************/
void AUDCTRL_DisableTap( AUDIO_HW_ID_t tap);

/**
*  @brief  Set Tap gain (only wideband)
*
*  @param  src	(in)  
*  @param  spr	(in)  speaker selection
*  @param  gain	(in)  
*
*  @return none
*
****************************************************************************/
void AUDCTRL_SetTapGain(
				AUDIO_HW_ID_t			tap,
				UInt32					gain
				);

/********************************************************************
*  @brief  mute/unmute a record path
*
*  @param  src	(in)  record source
*  @param  mic	(in)  microphone selection
*  @param  mute	(in)  TRUE: mute, FALSE: unmute
*
*  @return none
*
****************************************************************************/
void AUDCTRL_SetRecordMute(
				AUDIO_HW_ID_t			src,
				AUDCTRL_MICROPHONE_t	mic,
				Boolean					mute
				);

/********************************************************************
*  @brief  add a micophone to a record path
*
*  @param  src	(in)  record source
*  @param  mic	(in)  microphone selection
*
*  @return none
*
****************************************************************************/
void AUDCTRL_AddRecordMic(
				AUDIO_HW_ID_t			src,
				AUDCTRL_MICROPHONE_t	mic
				);

/********************************************************************
*  @brief  remove a micophone from a record path
*
*  @param  src	(in)  record source
*  @param  mic	(in)  microphone selection
*
*  @return none
*
****************************************************************************/
void AUDCTRL_RemoveRecordMic(
				AUDIO_HW_ID_t			src,
				AUDCTRL_MICROPHONE_t	mic
				);


/********************************************************************
*  @brief  enable or disable audio HW loopback
*
*  @param  enable_lpbk (in)  the audio mode
*  @param  mic         (in)  the input to loopback
*  @param  speaker     (in)  the output from loopback
*
*  @return none
*
****************************************************************************/
void AUDCTRL_SetAudioLoopback( 
							Boolean					enable_lpbk,
							AUDCTRL_MICROPHONE_t	mic,
							AUDCTRL_SPEAKER_t		speaker
							);

/********************************************************************
*  @brief  enable or disable audio HW loopback
*
*  @param  enable_lpbk (in)  the audio mode
*  @param  mic         (in)  the input to loopback
*  @param  speaker     (in)  the output from loopback
*
*  @return none
*
****************************************************************************/
void AUDCTRL_SetEQ( 
				AUDIO_HW_ID_t	audioPath,
				AudioEqualizer_en_t  equType
				);

#endif //#define __AUDIO_CONTROLLER_H__
