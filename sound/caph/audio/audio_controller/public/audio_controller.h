/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2009 - 2011  Broadcom Corporation                                                        */
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
typedef enum {
   TelephonyUseExtSpkr,
   AudioUseExtSpkr,
} ExtSpkrUsage_en_t;


typedef enum
{
  AUDCTRL_SSP_4 = 1, //SSPI1 --- ASIC SSPI4, SSPI2 --- ASIC SSPI3
  AUDCTRL_SSP_3
} AUDCTRL_SSP_PORT_e;

typedef enum
{
  AUDCTRL_SSP_PCM,
  AUDCTRL_SSP_I2S
} AUDCTRL_SSP_BUS_e;

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
*  @brief  This function is to shut down Audio Controller
*
*  @param  none
*
*  @return none
*
****************************************************************************/
void AUDCTRL_Shutdown (void);

/**
*  @brief  Enable telephony audio path in HW and DSP
*
*  @param  mic		(in)  microphone selection
*  @param  speaker	(in)  speaker selection
*
*  @return none
*
****************************************************************************/
void AUDCTRL_EnableTelephony(
				AUDIO_SOURCE_Enum_t		mic,
				AUDIO_SINK_Enum_t		speaker
				);

/**
*  @brief  Disable telephony audio path in HW and DSP
*
*  @param  mic		(in)  microphone selection
*  @param  speaker	(in)  speaker selection
*
*  @return none
*
****************************************************************************/
void AUDCTRL_DisableTelephony(
				AUDIO_SOURCE_Enum_t		mic,
				AUDIO_SINK_Enum_t		speaker
				);

/**
*  @brief  Rate change telephony audio for DSP
*
*
*  @return none
*
****************************************************************************/
void AUDCTRL_RateChangeTelephony( UInt32 sampleRate );

/**
*  @brief  Get voice call sample rate
*
*
*  @return voice call sample rate
*
****************************************************************************/
UInt32 AUDCTRL_RateGetTelephony( void );

/**
*  @brief  Set voice call sample rate
*
*
*  @none
*
****************************************************************************/
void AUDCTRL_RateSetTelephony(UInt32 samplerate);

/**
*  @brief  Change telephony audio path in HW and DSP
*
*  @param  mic		(in)  microphone selection
*  @param  speaker	(in)  speaker selection
*
*  @return none
*
****************************************************************************/
void AUDCTRL_SetTelephonyMicSpkr(
				AUDIO_SOURCE_Enum_t		mic,
				AUDIO_SINK_Enum_t		speaker
				);

/**
*  @brief  Set telephony speaker (downlink) volume
*
*  @param  speaker	(in)  speaker selection
*  @param  volume	(in)  downlink volume
*
*  @return none
*
****************************************************************************/
void AUDCTRL_SetTelephonySpkrVolume(
				AUDIO_SINK_Enum_t		speaker,
				Int32					volume,
				AUDIO_GAIN_FORMAT_t		gain_format
				);

/**
*  @brief  Get telephony speaker (downlink) volume
*
*  @param  gain_format	(in)  gain format
*
*  @return UInt32    dB
*
****************************************************************************/
UInt32 AUDCTRL_GetTelephonySpkrVolume( AUDIO_GAIN_FORMAT_t gain_format );

/**
*  @brief  Set telephony speaker (downlink) mute / un-mute
*
*  @param  speaker	(in)  speaker selection
*  @param  mute		(in)  TRUE: mute;    FALSE: un-mute
*
*  @return none
*
****************************************************************************/
void AUDCTRL_SetTelephonySpkrMute(
				AUDIO_SINK_Enum_t		speaker,
				Boolean					mute
				);

/**
*  @brief  Set telephony microphone (uplink) gain
*
*  @param  mic		(in)  microphone selection
*  @param  gain	(in)  gain
*  @param  gain_format	(in)  gain format
*
*  @return none
*
****************************************************************************/
void AUDCTRL_SetTelephonyMicGain(
				AUDIO_SOURCE_Enum_t		mic,
				Int16					gain,
                AUDIO_GAIN_FORMAT_t     gain_format
				);

/**
*  @brief  Set telephony mic (uplink) mute /un-mute
*
*  @param  mic		(in)  microphone selection
*  @param  mute		(in)  TRUE: mute;    FALSE: un-mute
*
*  @return none
*
****************************************************************************/
void AUDCTRL_SetTelephonyMicMute(
				AUDIO_SOURCE_Enum_t		mic,
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

#if defined(USE_NEW_AUDIO_PARAM)
//*********************************************************************
//  Save audio mode before call AUDCTRL_SaveAudioModeFlag( )
//	@param		mode		(voice call) audio mode
//	@param		app			(voice call) audio app
//	@return		none
//**********************************************************************/
void AUDCTRL_SaveAudioModeFlag( AudioMode_t mode, AudioApp_t app );


//*********************************************************************
//   Set (voice call) audio mode
//	@param		mode		(voice call) audio mode
//	@param		app		(voice call) audio app
//	@return		none
//**********************************************************************/
void AUDCTRL_SetAudioMode( AudioMode_t mode, AudioApp_t app);

#else
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
#endif

/**
*   Get Audio Mode From Sink (speaker)
*
*   @param      sink        speaker
*	@param		mode		(voice call) audio mode
*
*	@return		none
****************************************************************************/
void AUDCTRL_GetAudioModeBySink(AUDIO_SINK_Enum_t sink, AudioMode_t *mode);

/**
*   Get src and sink from audio mode
*
*	@param		mode		(voice call) audio mode
*   @param      pMic        microphone
*   @param      pSpk        speaker
*
*	@return		none
****************************************************************************/
void AUDCTRL_GetVoiceSrcSinkByMode(AudioMode_t mode, AUDIO_SOURCE_Enum_t *pMic, AUDIO_SINK_Enum_t *pSpk);

/**
*  @brief  Enable a playback path
*
*  @param  source	(in)  playback source
*  @param  sink	(in)  playback sink
*  @param  numCh	(in)  stereo, momo
*  @param  sr	(in)  sample rate
*  @param  pPathID	(in)  to return pathID
*
*  @return none
*
****************************************************************************/
void AUDCTRL_EnablePlay(
				AUDIO_SOURCE_Enum_t 	source,
				AUDIO_SINK_Enum_t		sink,
				AUDIO_NUM_OF_CHANNEL_t	numCh,
				AUDIO_SAMPLING_RATE_t	sr,
				unsigned int			*pPathID
				);

/********************************************************************
*  @brief  Disable a playback path
*
*  @param  source	(in)  playback source
*  @param  sink	(in)  playback sink
*  @param  pathID	(in)  the pathID returned by CSL HW controller.
*
*  @return none
*
****************************************************************************/
void AUDCTRL_DisablePlay(
				AUDIO_SOURCE_Enum_t 	source,
				AUDIO_SINK_Enum_t		sink,
				unsigned int			pathID
				);

/********************************************************************
*  @brief  Set playback volume
*
*  @param  source	(in)  playback source
*  @param  sink	(in)  playback sink
*  @param  vol	(in)  volume to set
*  @param pathID (in) the pathID returned by CSL HW controller.
*
*  @return none
*
****************************************************************************/
void AUDCTRL_SetPlayVolume(
				AUDIO_SOURCE_Enum_t		source,
				AUDIO_SINK_Enum_t		sink,
				AUDIO_GAIN_FORMAT_t     gainF,
				int						vol_left,
				int						vol_right,
				unsigned int			pathID
				);

/********************************************************************
*  @brief  mute/unmute playback
*
*  @param  source	(in)  playback source
*  @param  sink	(in)  playback sink
*  @param  mute	(in)  TRUE: mute, FALSE: unmute
*  @param pathID (in) the pathID returned by CSL HW controller.
*
*  @return none
*
****************************************************************************/
void AUDCTRL_SetPlayMute(
				AUDIO_SOURCE_Enum_t 	source,
				AUDIO_SINK_Enum_t		sink,
				Boolean					mute,
				unsigned int			pathID
				);

/********************************************************************
*  @brief  switch speaker of playback
*
*  @param   source  Source
*  @param   sink	new Sink device
*  @param pathID (in) the pathID returned by CSL HW controller.
*  @return none
*
****************************************************************************/
void AUDCTRL_SwitchPlaySpk(
				AUDIO_SOURCE_Enum_t 	source,
				AUDIO_SINK_Enum_t		sink,
				unsigned int			pathID
				);

/********************************************************************
*  @brief  Add a speaker to a playback path
*
*  @param   source  Source
*  @param   sink	(in)  playback sink
*  @param pathID (in) the pathID returned by CSL HW controller.
*
*  @return none
*
****************************************************************************/
void AUDCTRL_AddPlaySpk(
				AUDIO_SOURCE_Enum_t		source,
				AUDIO_SINK_Enum_t		sink,
				unsigned int			pathID
				);

/********************************************************************
*  @brief  Remove a speaker to a playback path
*
*  @param   source  Source
*  @param  sink	(in)  playback sink
*  @param pathID (in) the pathID returned by CSL HW controller.
*
*  @return none
*
****************************************************************************/
void AUDCTRL_RemovePlaySpk(
                AUDIO_SOURCE_Enum_t     source,
				AUDIO_SINK_Enum_t		sink,
				unsigned int			pathID
				);


/********************************************************************
*  @brief  enable a record path
*
*  @param  source	(in)  record source
*  @param  sink	(in)  record sink
*  @param  numCh	(in)  stereo, mono
*  @param  sr	(in)  sample rate
*  @param  pPathID	(in)  to return pathID
*
*  @return none
*
****************************************************************************/
void AUDCTRL_EnableRecord(
				AUDIO_SOURCE_Enum_t			source,
				AUDIO_SINK_Enum_t			sink,
				AUDIO_NUM_OF_CHANNEL_t		numCh,
				AUDIO_SAMPLING_RATE_t		sr,
				unsigned int				*pPathID
				);

/********************************************************************
*  @brief  disable a record path
*
*  @param  source	(in)  record source
*  @param  sink	(in)  record sink
*  @param pathID (in) the pathID returned by CSL HW controller.
*
*  @return none
*
****************************************************************************/
void AUDCTRL_DisableRecord(
				AUDIO_SOURCE_Enum_t		source,
				AUDIO_SINK_Enum_t		sink,
				unsigned int			pathID
				);

/********************************************************************
*  @brief  Set gain of a record path
*
*  @param  source (in)  record source
*  @param  gainFormat	(in)  the gain format
*  @param  gainL	(in)  the left channel gain to set
*  @param  gainR	(in)  the right channel gain to set
*  @param pathID (in) the pathID returned by CSL HW controller.
s*
*  @return none
*
****************************************************************************/
void AUDCTRL_SetRecordGain(
				AUDIO_SOURCE_Enum_t		source,
                AUDIO_GAIN_FORMAT_t     gainFormat,
				UInt32					gainL,
				UInt32					gainR,
				unsigned int			pathID
				);

/********************************************************************
*  @brief  mute/unmute a record path
*
*  @param  source	(in)  record source
*  @param  mute	(in)  TRUE: mute, FALSE: unmute
*  @param pathID (in) the pathID returned by CSL HW controller.
*
*  @return none
*
****************************************************************************/
void AUDCTRL_SetRecordMute(
				AUDIO_SOURCE_Enum_t		source,
				Boolean					mute,
				unsigned int			pathID
				);

/********************************************************************
*  @brief  add a micophone to a record path
*
*  @param  source	(in)  record source
*
*  @return none
*
****************************************************************************/
void AUDCTRL_AddRecordMic(
				AUDIO_SOURCE_Enum_t		source
				);

/********************************************************************
*  @brief  remove a micophone from a record path
*
*  @param  source	(in)  record source
*
*  @return none
*
****************************************************************************/
void AUDCTRL_RemoveRecordMic(
				AUDIO_SOURCE_Enum_t	source
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
							AUDIO_SOURCE_Enum_t		mic,
							AUDIO_SINK_Enum_t		speaker
							);

/********************************************************************
*  @brief  Set Arm2Sp Parameter
*
*  @param  mixMode        For selection of mixing with voice DL, UL, or both
*  @param  instanceId     Instance ID: 1 for arm2sp1, 2 for arm2sp2
*
*  @return none
*
****************************************************************************/
void AUDCTRL_SetArm2spParam( UInt32 mixMode, UInt32 instanceId );

/********************************************************************
*  @brief  Configure fm/pcm port
*
*  @param  port  SSP port number
*
*  @param  bus   protocol (I2S or PCM)
*
*  @return none
*
****************************************************************************/
void AUDCTRL_ConfigSSP(AUDCTRL_SSP_PORT_e port, AUDCTRL_SSP_BUS_e bus);

/********************************************************************
*  @brief  Control ssp tdm mode
*
*  @param  status ssp tdm status
*
*  @return none
*
****************************************************************************/
void AUDCTRL_SetSspTdmMode(Boolean status);

/********************************************************************
*  @brief  Enable bypass mode for vibrator
*
*  @param  none
*
*  @return none
*
****************************************************************************/
void  AUDCTRL_EnableBypassVibra(void);


/********************************************************************
*  @brief  Disable vibrator in bypass mode
*
*  @param  none
*
*  @return none
*
****************************************************************************/
void  AUDCTRL_DisableBypassVibra(void);


/********************************************************************
*  @brief  Set the strength to vibrator in bypass mode
*
*  @param  Strength  strength value
*
*  @param  direction vibrator moving direction
*
*  @return none
*
****************************************************************************/
void  AUDCTRL_SetBypassVibraStrength(UInt32 Strength, int direction);

/********************************************************************
*  @brief  Set IHF mode
*
*  @param  IHF mode status (TRUE: stereo | FALSE: mono).
*
*  @return  none
*
****************************************************************************/
void AUDCTRL_SetIHFmode (Boolean stIHF);

/********************************************************************
*  @brief  Set BT mode
*
*  @param  BT mode status for BT production test.
*
*  @return  none
*
****************************************************************************/
void  AUDCTRL_SetBTMode(Boolean mode);

/********************************************************************
*  @brief  Enable/Disable CAPH clock
*
*  @param  enable/disable
*
*  @return  none
*
****************************************************************************/
void  AUDCTRL_ControlHWClock(Boolean enable);

/********************************************************************
*  @brief  Query CAPH clock is enabled/disabled
*
*  @param  none
*
*  @return  Boolean
*
****************************************************************************/
Boolean  AUDCTRL_QueryHWClock(void);

void SetGainOnExternalAmp_mB(AUDIO_SINK_Enum_t speaker, int gain_mB, int left_right);

/**
*  @brief  This function controls the power on/off the power supply for
*  		digital mics.
*
*  @param  powerOn	(in) TRUE: Power on, FALSE: Power off
*
*  @return void
*
****************************************************************************/
void powerOnDigitalMic( Boolean powerOn );

/**
*  @brief  This function gets the device enum mapping value from source
*
*  @param  source	(in)
*
*  @return device (out)
*
****************************************************************************/

CSL_CAPH_DEVICE_e getDeviceFromSrc(AUDIO_SOURCE_Enum_t source);

/**
*  @brief  This function gets the device enum mapping value from sink
*
*  @param  sink	(in)
*
*  @return device (out)
*
****************************************************************************/

CSL_CAPH_DEVICE_e getDeviceFromSink(AUDIO_SINK_Enum_t sink);


#endif //#define __AUDIO_CONTROLLER_H__
