/*******************************************************************************************
Copyright 2009, 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement governing use 
of this software, this software is licensed to you under the terms of the GNU General Public 
License version 2, available at http://www.gnu.org/copyleft/gpl.html (the "GPL"). 

Notwithstanding the above, under no circumstances may you combine this software in any way 
with any other Broadcom software provided under a license other than the GPL, without 
Broadcom's express prior written consent.
*******************************************************************************************/

/**
*
* @file   csl_aud_drv.h
* @brief  Audio HW Driver API
*
*****************************************************************************/

/**
*
* @defgroup AudioDriverGroup    Audio Driver
*
* @brief    This group defines the common APIs for audio driver
*
* @ingroup  CSLGroup
*****************************************************************************/

#ifndef	__CSL_AUD_DRV_H__
#define	__CSL_AUD_DRV_H__

#include "auddrv_def.h"
/**
*
* @addtogroup AudioDriverGroup
* @{
*/

#ifdef __cplusplus
extern "C" {
#endif
#ifndef	_DRV_CAPH_HWCTRL_
typedef enum {
    AUDDRV_TYPE_TELEPHONY, 
    AUDDRV_TYPE_AUDIO_OUTPUT,
    AUDDRV_TYPE_RINGTONE_OUTPUT,
    //   AUDDRV_TYPE_VOICE_OUTPUT,  //for ARM write
    //   AUDDRV_TYPE_VOICE_INPUT,   //for ARM read
    AUDDRV_TYPE_AUDIO_INPUT,  //HQ Audio
    AUDDRV_TYPE_MixerTap_VB, 
    AUDDRV_TYPE_MixerTap_WB, 
    AUDDRV_TYPE_AMRNB, 
    AUDDRV_TYPE_AMRWB, 
    AUDDRV_TYPE_ARM2SP_Inst1,
    AUDDRV_TYPE_ARM2SP_Inst2,
    AUDDRV_TYPE_USBHeadset
} AUDDRV_TYPE_Enum_t;

typedef enum {
    AUDDRV_ALL_OUTPUT,			//this can only be used when SelectMic( ) and SelectSpkr( ).
    AUDDRV_AUDIO_OUTPUT,
    AUDDRV_RINGTONE_OUTPUT,
    AUDDRV_VOICE_OUTPUT,
    AUDDRV_VOICE_INPUT,
    AUDDRV_AUDIO_INPUT,			//HQ Audio input
    AUDDRV_MIXERTap_VB_INPUT,	//Voice band mixer tap
    AUDDRV_MIXERTap_WB_INPUT,	//Wide band mixer tap
	AUDDRV_INOUT_NUM
} AUDDRV_InOut_Enum_t;
#endif
typedef enum {
	AUDDRV_GAIN_SPKR_IHF,  //100mW,
	AUDDRV_GAIN_SPKR_EP,  //100mW,
	AUDDRV_MIC, //used in at_phone.c, to be removed
	AUDDRV_GAIN_MIC = AUDDRV_MIC,
	AUDDRV_GAIN_VOICE_IN,
	AUDDRV_GAIN_AUDIO_IN,
	AUDDRV_GAIN_AUDIO_OUTPUT_L,
	AUDDRV_GAIN_AUDIO_OUTPUT_R,
	AUDDRV_GAIN_RINGTONE_OUTPUT_L,
	AUDDRV_GAIN_RINGTONE_OUTPUT_R,
	AUDDRV_GAIN_VOICE_OUTPUT,
	AUDDRV_GAIN_VOICE_OUTPUT_CFGR,
	AUDDRV_GAIN_MIXER1,
	AUDDRV_GAIN_MIXER2,
	AUDDRV_GAIN_MIXER3,
	AUDDRV_GAIN_MIXER4,
	AUDDRV_GAIN_MPMDGA1,
	AUDDRV_GAIN_MPMDGA2,
	AUDDRV_GAIN_MPMDGA3,
	AUDDRV_GAIN_MPMDGA4,
	AUDDRV_GAIN_MIXERTap_WB_L,
	AUDDRV_GAIN_MIXERTap_WB_R,
} AUDDRV_GAIN_Enum_t;

typedef enum {
	AUDDRV_MUTE_SPKR1_L,  //100mW
	AUDDRV_MUTE_SPKR1_R,  //100mW
	AUDDRV_MUTE_MIC,
	AUDDRV_MUTE_AUDIO_OUTPUT_L,
	AUDDRV_MUTE_AUDIO_OUTPUT_R,
	AUDDRV_MUTE_RINGTONE_OUTPUT_L,
	AUDDRV_MUTE_RINGTONE_OUTPUT_R,
	AUDDRV_MUTE_VOICE_OUTPUT,
	AUDDRV_MUTE_MIXER1,
	AUDDRV_MUTE_MIXER2,
	AUDDRV_MUTE_MIXER3,
	AUDDRV_MUTE_MIXER4,
	AUDDRV_MUTE_MPMDGA1,
	AUDDRV_MUTE_MPMDGA2,
	AUDDRV_MUTE_MPMDGA3,
	AUDDRV_MUTE_MPMDGA4,
	AUDDRV_MUTE_MIXERTap_WB_L,
	AUDDRV_MUTE_MIXERTap_WB_R,
} AUDDRV_MUTE_Enum_t;
/*
typedef enum {
	AUDDRV_VoiceDAC,
	AUDDRV_VoiceADC,
	AUDDRV_AEQ,
	AUDDRV_AEQPATHGAIN,
	AUDDRV_AEQPATHOFST,
	AUDDRV_AFIR,
	AUDDRV_PEQ,
	AUDDRV_PEQPATHGAIN,
	AUDDRV_PEQPATHOFST,
	AUDDRV_PIIR,
	AUDDRV_Mixer1_MPMIIR,
	AUDDRV_Mixer2_MPMIIR,
	AUDDRV_Mixer3_MPMIIR,
	AUDDRV_Mixer4_MPMIIR,
	AUDDRV_Mixer_BIQUAD_CFG,
	AUDDRV_Mixer1_MPMALDCENABLE,
	AUDDRV_Mixer2_MPMALDCENABLE,
	AUDDRV_Mixer3_MPMALDCENABLE,
	AUDDRV_Mixer4_MPMALDCENABLE
} AUDDRV_Filter_Enum_t;
*/

typedef enum {
	AUDDRV_REASON_HW_CTRL,
	AUDDRV_REASON_DATA_DRIVER,
	AUDDRV_REASON_HW_LOOPBACK
} AUDDRV_REASON_Enum_t;  //reason for enable/disable audio HW. (this is to avoid FIFO overflow, underflow)


typedef Boolean (*BufDoneCB_t)( UInt8  *pBuf,  UInt32 nSize );

typedef void ( *audio_HWEnabled_Cb_t )( void );

/**
*  @brief  Initialize audio system
*
*  @param  none
*
*  @return none
*
****************************************************************************/
void AUDDRV_Init( void );

/**
*  @brief  Initialize audio cHAL handle and configure speaker driver
*
*  @param  speaker			(in) speaker driver instance
*  @param  speaker_config	(in) speaker driver
*
*  @return none
*
****************************************************************************/
void AUDDRV_SPKRInit (
		   AUDDRV_SPKR_Enum_t     speaker,	
		   AUDIO_SPKR_CH_Mode_t   speaker_config
		   );

/**
*  @brief  Enable hardware audio path
*
*  @param  mixer_speaker_selection	(in) mixer identifier
*  @param  sample_rate			(in) sample rate of HW
*  @param  pData			(out) pass some needed data.
*
*  @return none
*
****************************************************************************/
void AUDDRV_Telephony_InitHW (
			  AUDDRV_MIC_Enum_t      mic,
			  AUDDRV_SPKR_Enum_t     speaker,
			  AUDIO_SAMPLING_RATE_t  sample_rate,
			  void* pData
			  );

/**
*  @brief  Disable hardware audio path
*
*  @param  none
*
*  @return none
*
****************************************************************************/
void AUDDRV_Telephony_DeinitHW( void *pData );

/**
*  @brief  Enable hardware audio output path
*
*  @param  input_path_to_mixer		(in) input path to mixer
*  @param  mixer_speaker_selection	(in) mixer identifier
*  @param  enable_speaker			(in) TRUE: enable speaker driver
*												on this mixer output
*										 FALSE: disable speaker driver
*  @param  sample_rate				(in) sample rate of HW
*
*  @return none
*
*  @note   The path includes mixer input path and/or speaker driver(s). 
*			This includes the control sequence for enabling audio output path.
*
****************************************************************************/
void AUDDRV_EnableHWOutput (
		   AUDDRV_InOut_Enum_t     input_path_to_mixer,
		   AUDDRV_SPKR_Enum_t      mixer_speaker_selection,
		   Boolean                 enable_speaker,
		   AUDIO_SAMPLING_RATE_t   sample_rate,
		   AUDIO_CHANNEL_NUM_t     input_to_mixer,
		   AUDDRV_REASON_Enum_t	   reason,
           audio_HWEnabled_Cb_t    callback
		   );

/**
*  @brief  Disable hardware audio output path
*
*  @param  speaker			(in) speaker driver instance
*  @param  speaker_config	(in) speaker driver
*
*  @return none
*
****************************************************************************/
void AUDDRV_DisableHWOutput ( 
		 AUDDRV_InOut_Enum_t  path,
		 AUDDRV_REASON_Enum_t	from
		 );

/**
*  @brief  Enable hardware audio input path
*
*  @param  input_path		(in) input ditigal path identifier
*  @param  mic_selection	(in) microphone identifier
*  @param  sample_rate		(in) sample rate of HW
*
*  @return none
*
*  @note   The path includes microphone. 
*
****************************************************************************/
void AUDDRV_EnableHWInput (
       AUDDRV_InOut_Enum_t      input_path,
       AUDDRV_MIC_Enum_t        mic_selection,
	   AUDIO_SAMPLING_RATE_t    sample_rate,
	   AUDDRV_REASON_Enum_t	from
	   );

/**
*  @brief  Disable hardware audio input path
*
*  @param  input_path		(in) input ditigal path identifier
*  @param  mic_selection	(in) microphone identifier
*  @param  sample_rate		(in) sample rate of HW
*
*  @return none
*
*  @note   The path includes microphone. 
*
****************************************************************************/
void AUDDRV_DisableHWInput ( 
		AUDDRV_InOut_Enum_t  path,
		AUDDRV_REASON_Enum_t	from
		);


/**
*  @brief  Enable or Disable the I2S MUX to audio output path
*
*  @param  on	(in) TRUE: enable
*					 FALSE: disable
*
*  @return none
*
****************************************************************************/
void AUDDRV_Set_I2sMuxToAudio ( Boolean   on );


/**
*  @brief  Select input to Mixer Tap, and enable the mixer tap.
*
*  @param  mixer_tap				(in) mixer tap instance (identifier)
*  @param  left_input_selection		(in) identify mixer input path
*  @param  right_input_selection	(in) identify mixer input path
*  @param  sample_rate				(in) sample rate of mixer tap HW
*
*  @return none
*
*  @note   sampleRate on Mxier Tap is set through funciton
*			AUDDRV_Record_SetConfig( ).
*
****************************************************************************/
void AUDDRV_Enable_MixerTap (
		   AUDDRV_InOut_Enum_t    mixer_tap,
		   AUDDRV_SPKR_Enum_t     left_input_selection,
		   AUDDRV_SPKR_Enum_t     right_input_selection,
		   AUDIO_SAMPLING_RATE_t  sample_rate,
		   AUDDRV_REASON_Enum_t	  from
		   );

/**
*  @brief  Disable the mixer tap.
*
*  @param  mixer_tap	(in) mixer instance (identifier)
*
*  @return none
*
****************************************************************************/
void AUDDRV_Disable_MixerTap ( 
		  AUDDRV_InOut_Enum_t  mixer_tap,
		  AUDDRV_REASON_Enum_t	from
		  );

/**
*  @brief  set mute at the HW mute point.
*
*  @param  mute_point	(in) mute point in HW
*  @param  mute			(in) TRUE: mute;  FALSE: un-mute
*
*  @return none
*
****************************************************************************/
void AUDDRV_SetMute (
		AUDDRV_MUTE_Enum_t  mute_point,
		Boolean				mute
		);

/**
*  @brief  Get mute status.
*
*  @param  mute_point	(in) mute point in HW
*
*  @return	Boolean		TTRUE: muted.   FALSE: not muted.
*
****************************************************************************/
Boolean AUDDRV_GetMute ( AUDDRV_MUTE_Enum_t  mute_point );

/**
*  @brief  set gain at the HW gain adjustment point.
*
*  @param  gain_adj_point	(in) gain adjustment point in HW
*  @param  gain_mB			(in) gain in mB
*
*  @return none
*
****************************************************************************/
void AUDDRV_SetGain (
		AUDDRV_GAIN_Enum_t  gain_adj_point,
		Int32				gain_mB
		);

/**
*  @brief  get gain at the HW gain adjustment point.
*
*  @param  gain_adj_point	(in) gain adjustment point in HW
*  @param  gainFomrat		(in) 
*
*  @return none
*
****************************************************************************/
UInt32 AUDDRV_GetGain ( AUDDRV_GAIN_Enum_t  gain_adj_point, Int32  gainFormat );


/**
*  @brief  set gain at the HW gain adjustment point.
*
*  @param  gain_adj_point	(in) gain adjustment point in HW
*  @param  gain_hex			(in) The gain_hex is writtent to HW register.
*
*  @return none
*
****************************************************************************/
void AUDDRV_SetGain_Hex (
		AUDDRV_GAIN_Enum_t  gain_adj_point,
		UInt32				gain_hex
		);

/**
*  @brief  Select Mic input for the HW digital path
*
*  @param  path	(in) the HW digital path
*  @param  mic	(in) Mic input 
*
*  @return none
*
*  @note   The mic selection in this function call overrides the previous
*			selection. Client calls this function to change mic input.
*
****************************************************************************/
void AUDDRV_SelectMic (
		AUDDRV_InOut_Enum_t	path,
		AUDDRV_MIC_Enum_t	mic
		);

/**
*  @brief  Select speaker for the HW digital path
*
*  @param  path		(in) the HW digital path
*  @param  speaker	(in) speaker driver 
*
*  @return none
*
*  @note   The speaker selection in this function call overrides the previous
*			selection. Client calls this function to change speaker output.
*
****************************************************************************/
void AUDDRV_SelectSpkr (
       AUDDRV_InOut_Enum_t  path,
       AUDDRV_SPKR_Enum_t   speaker,
       AUDDRV_SPKR_Enum_t   speaker_second
       );


/**
*  @brief  Load filter coefficients to HW or DSP
*
*  @param  filter	(in) filter indentifier
*  @param  coeff	(in) pointer to coefficients 
*
*  @return none
*
****************************************************************************/
void AUDDRV_SetFilter(
		AUDDRV_Filter_Enum_t	filter,
		const UInt16			*coeff
		);

/**
*  @brief  Set MPM parameters
*
*  @param  param_id	(in) parameter indentifier
*  @param  param	(in) parameter
*
*  @return none
*
****************************************************************************/
void AUDDRV_SetMPM( 
		AUDDRV_Filter_Enum_t param_id,
		const UInt16 param
		);


/**
*  @brief  Set voice call flag for HW control loic.
*
*  @param  inVoiceCall	TRUE: in voice call and needs audio HW.
*						FALSE: not in voice call.
*
*  @return none
*
****************************************************************************/
void AUDDRV_SetVCflag( Boolean inVoiceCall );

/**
*  @brief  Get voice call flag.
*
*  @param  none
*
*  @return Boolean
*
****************************************************************************/
Boolean AUDDRV_GetVCflag( void );

/**
*  @brief  Set voice path sample rate.
*
*  @param  AUDIO_SAMPLING_RATE_t      sample_rate
*
*  @return none
*
****************************************************************************/
void AUDDRV_SetVoicePathSampRate( AUDIO_SAMPLING_RATE_t  sample_rate );


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
void AUDDRV_SetAudioLoopback( 
			Boolean             enable_lpbk,
			AUDDRV_MIC_Enum_t   mic,
			AUDDRV_SPKR_Enum_t  speaker,
			int path
			);

/****************************************************************************
*
* Function Name: Result_t AUDDRV_HWControl_EnableVibrator(Boolean enable_vibrator,
*													 AUDDRV_VIBRATOR_MODE_Enum_t mode) 
* Description:   Enable/Disable vibrator with default strength
*
* Parameters:    enable_vibrator     : enable (TRUE) or disable (FALSE) operation to vibrator  
*                mode                : by_pass mode ( 0 ) or PCM playback mode (non zero)
*
* Return:       the call result
*
******************************************************************************/

void AUDDRV_HWControl_EnableVibrator(Boolean enable_vibrator, AUDDRV_VIBRATOR_MODE_Enum_t mode);


/****************************************************************************
*
* Function Name: Result_t AUDDRV_HWControl_VibratorStrength (UInt32 strength)
*
* Description:   Change the vibrator strength
*
* Parameters:    strength : strength value to vibrator  
*
* Return:       the call result
*
******************************************************************************/

void AUDDRV_HWControl_VibratorStrength(UInt32 strength);


/********************************************************************
*  @brief  set audio 5-band EQ type
*
*  @param  path        (in)  the input to mixer
*  @param  equ_id      (in)  the equalizer by ID
*
*  @return none
*
****************************************************************************/
void AUDDRV_SetEquType( 
			AUDDRV_TYPE_Enum_t   path,
			AudioEqualizer_en_t	 equ_id
			);

/********************************************************************
*  @brief  set audio 5-band EQ type
*
*  @param  path        (in)  the input to mixer
*  @param  equ_id      (in)  the equalizer by ID
*
*  @return AudioEqualizer_en_t  Equailizer type
*
****************************************************************************/
AudioEqualizer_en_t AUDDRV_GetEquType( AUDDRV_TYPE_Enum_t   path );


typedef AudioMode_t (*CB_GetAudioMode_t)( void );

#if defined(USE_NEW_AUDIO_PARAM)
typedef void (*CB_SetAudioMode_t) ( AudioMode_t  audio_mode, AudioApp_t audio_app);
typedef void (*CB_SetMusicMode_t) ( AudioMode_t  audio_mode, AudioApp_t audio_app);
typedef AudioApp_t (*CB_GetAudioApp_t)( void );

/********************************************************************
*  @brief  Register up callback for getting audio application
*
*  @param  cb  (in)  callback function pointer
*
*  @return none
*
****************************************************************************/
void AUDDRV_RegisterCB_getAudioApp( CB_GetAudioApp_t	cb );

#else
typedef void (*CB_SetAudioMode_t) ( AudioMode_t  audio_mode);
typedef void (*CB_SetMusicMode_t) ( AudioMode_t  audio_mode);
#endif

/********************************************************************
*  @brief  Register up callback for getting audio mode
*
*  @param  cb  (in)  callback function pointer
*
*  @return none
*
****************************************************************************/
void AUDDRV_RegisterCB_getAudioMode( CB_GetAudioMode_t	cb );

/********************************************************************
*  @brief  Register up callback for setting audio mode
*
*  @param  cb  (in)  callback function pointer
*
*  @return none
*
****************************************************************************/
void AUDDRV_RegisterCB_setAudioMode( CB_SetAudioMode_t	cb );

/********************************************************************
*  @brief  Register up callback for setting music audio mode
*
*  @param  cb  (in)  callback function pointer
*
*  @return none
*
****************************************************************************/
void AUDDRV_RegisterCB_setMusicMode( CB_SetMusicMode_t	cb );


#ifdef __cplusplus
}
#endif

#endif // __CSL_AUD_DRV_H__
