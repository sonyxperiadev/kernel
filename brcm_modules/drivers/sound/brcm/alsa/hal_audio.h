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
*    @file   hal_audio.h
*    @brief  This file is the HAL Audio API
**/
/**
*   @defgroup   AudioDrvGroup   Audio HAL
*   @ingroup    HALMMGroup
*   @brief      This file defines the API to control audio devices.
*				OS dependent code.
*
*   Device independent API for audio device control.
*
*	Click here to return to the HAL Multimedia overview: \ref MMHALOverview
****************************************************************************/

#ifndef _HAL_AUDIO_H__
#define _HAL_AUDIO_H__
#include "mobcom_types.h"
#include "resultcode.h"
#include "audio_consts.h"
//#include "speaker.h"
#include "mediatypes.h"
#include "hal_audio_enum.h"


//! The higher layer passes an audio ID in HAL_AUDIO_Ctrl() to identify the purpose of control action.
typedef enum
{
	AUDIO_ID_TONE,						///< DSP tone generation ID =0
	AUDIO_ID_CALL,						///< Voice call ID = 1
#ifdef FEATURE_AUDIO_VOICECALL16K
	AUDIO_ID_CALL8K = AUDIO_ID_CALL,	
#endif // FEATURE_AUDIO_VOICECALL16K
	AUDIO_ID_VPU,						///< Voice application (NB-AMR playback and recording) ID = 2
	AUDIO_ID_VPU_PLAY = AUDIO_ID_VPU,	///< Voice application (NB-AMR playback only) ID = 2
	AUDIO_ID_VPU_RECORD,				///< Voice application (NB-AMR recording only) ID = 3	
	AUDIO_ID_VPU_TELEPHONY,				///< Voice application (NB-AMR recording only) ID = 4
	AUDIO_ID_VPU_VT_LOOPBACK,           ///< Voice application for VT loopback ID = 5
	AUDIO_ID_POLY,      				///< Polyringer (MIDI) Id =6
	AUDIO_ID_IIS,						///< Play PCM audio on external device through IIS interface ID = 7
	AUDIO_ID_IIS2DAC,        			///< Connect I2S input to DAC ID = 8
#if (defined(_BCM213x1_) || defined(BCM2153_FAMILY_BB_CHIP_BOND))
	AUDIO_ID_FM_PLAY = AUDIO_ID_IIS2DAC,
#endif
	AUDIO_ID_AUDVOC,				///< Play audio through on-chip DAC and DSP codec (PCM, MP3, AAC, WB-AMR, etc) ID = 9
	AUDIO_ID_AMRWB_RECORD,			///< Record audio through on-chip DAC and DSP codec (WB-AMR) ID = 10
	AUDIO_ID_ST_BT,					///< Play MP3, AAC, PCM, etc, to stereo Bluetooth ID = 11
	AUDIO_ID_USB, 					///< Play MP3, AAC, PCM, etc, to USB audio ID = 12
	AUDIO_ID_EXT_ANLG,       		///< External device analog audio path (for FM radio listening) ID = 13
#if (defined(ROYALE_HW)  &&  defined(ZAVRAY))
	AUDIO_ID_FM_PLAY = AUDIO_ID_EXT_ANLG,
#endif
	AUDIO_ID_HQ_AUDIO_RECORD,		///< High quality Microphone recording ID = 14
	AUDIO_ID_IIS2DSP,				///< IIS2DSP ID = 15
#if (defined(_BCM2153_))
	//AUDIO_ID_FM_PLAY = AUDIO_ID_IIS2DSP, ///< FM PLAY ID = 15
#endif
	AUDIO_ID_FM_RECORD_PCM,			////< record FM raido as PCM ID = 16  
	AUDIO_ID_FM_RECORD_AAC,			///< Record FM radio as AAC ID = 17
	AUDIO_ID_PCM_READBACK,			///< Read back PCM  ID =18
	AUDIO_ID_WAVETONE_POLY,         ///<WAVE TONE POLY ID = 19
#ifdef FEATURE_AUDIO_VOICECALL16K
	AUDIO_ID_CALL16K,				///< Voice call ID for WB-AMR = 20
#endif // FEATURE_AUDIO_VOICECALL16K
	AUDIO_ID_VoIP_LOOPBACK,
	AUDIO_ID_VoIP_TELEPHONY,
	AUDIO_ID_FM_RECORD_PCM_44_1K,			////< record FM raido as PCM  
	AUDIO_ID_USER_EXT_AMP,
	AUDIO_ID_USER_RESERVED,			///< This ID is reserved for User's UI SW. (ID is expandable)
	AUDIO_ID_ALL					///< placeholder, MUST be the last one
} AudioClientID_en_t;

//! Microphone input selection.
typedef enum
{
	MIC_MAIN,			///< first microphone input
	MIC_AUX,			///< second microphone input
	MIC_DIGITAL_MAIN,	///< first digital microphone
	MIC_DIGITAL_AUX,	///< second digital microphone
	MIC_UNKNOWN			///< don't known mic input, also used in music playback (only speaker is used)
} MICSelection_en_t;

//! Mixer input selection,
// THIS NEED TO BE CONSOLIDATED WITH define in audvoc_if.h.
typedef enum
{
	AUDVOC_AUDIO,
	AUDVOC_POLY,
	AUDVOC_VOICE
} MixerInput_en_t;

//! 	Parameter structure for action codes:
//!   ACTION_AUD_MuteMic, ACTION_AUD_UnMuteMic, ACTION_AUD_MuteSpeaker, ACTION_AUD_UnmuteSpeaker,
typedef struct {
	AudioClientID_en_t 	audioID;	///< Audio client ID
} HAL_AUDIO_Param_AudioID_t;

//!  Parameter structure for action codes: ACTION_AUD_SetSpeakerVol
typedef struct
{
#if defined (SS_2153)
	UInt16			volumeLevel;	///< Logical speaker volume level, range is defined in audio_const.h
#else // SS_2153
	UInt8			volumeLevel;	///< Logical speaker volume level, range is defined in audio_const.h
#endif // SS_2153
} HAL_AUDIO_Param_SpkrVol_t;

//! Parameter structure for action codes: ACTION_AUD_SetMicGain
typedef struct {
	UInt8 				micGainLevel;	///< Logical gain level at microphone input
} HAL_AUDIO_Param_MicGain_t;

//! Parameter structure for action codes: ACTION_AUD_EnablePath, ACTION_AUD_DisablePath
typedef struct {
	AudioClientID_en_t  audioID;		///< Audio client ID
	UInt32				channels;		///< Number of audio channel
	AUDIO_CHANNEL_t		outputChnl;		///< Audio output channel
	MixerInput_en_t     mixerInputPath; ///< Mixer input path
	MICSelection_en_t	inputMIC;		///< Microphone input selection
#if defined (SS_2153)
	UInt16				volumeLevel;	///< Logical speaker volume level
#else // SS_2153
	UInt8				volumeLevel;	///< Logical speaker volume level
#endif // SS_2153
} HAL_AUDIO_Param_PathCfg_t;

//! Parameter structure for action codes: ACTION_AUD_PlayTone
typedef struct {
#if defined (SS_2153)
	UInt16				volumeLevel;	///< Logical speaker volume level
#else // SS_2153
	UInt8				volumeLevel;	///< Logical speaker volume level
#endif // SS_2153
	AUDIO_CHANNEL_t		outputChnl;		///< Audio output channel
	SpeakerTone_t		tone;			///< Tone ID
	UInt32				duration;		///< Tone duration
} HAL_AUDIO_Param_Tone_t;

//! Parameter structure for action codes: ACTION_AUD_PlayGenericTone
typedef struct {
#if defined (SS_2153)
	UInt16				volumeLevel;	///< Logical speaker volume level
#else // SS_2153
	UInt8				volumeLevel;	///< Logical speaker volume level
#endif // SS_2153
	AUDIO_CHANNEL_t		outputChnl;		///< Audio output channel
	Boolean	superimpose;				///< the tone superimpose voice.
	UInt16	duration;					///< tone duration in millisecond.
	UInt16	f1;							///< first frequency of tone. Mandatory value.
	UInt16	f2;							///< second frequency of tone. Optional value. 0 means not used.
	UInt16	f3;							///< third frequency of tone. Optional value. 0 means not used.
} HAL_AUDIO_Param_GenericTone_t;

//! Buffer fill callback function in streaming playback
typedef Boolean (*fBufferFill_CB_t)(UInt8* pBuffer, UInt32 nFrames);  // _t means type or defined type.

//! Buffer dump callback function in streaming playback
typedef Boolean (*fBufferDump_CB_t)(UInt8* pBuffer, UInt32 nLength);  // _t means type or defined type.


//! Parameter structure for action codes: ACTION_AUD_ConfigCodecParam
typedef struct {
	AudioClientID_en_t  	audioID;		///< Audio client ID
	AudioSampleRate_en_t	sampleRate;		///< Audio sampling rate
	AudioMonoStereo_en_t	monoStereo;		///< Number of audio channel
	AudioBitPerSample_en_t	bitPerSample;	///< Bits per sample
	AUDIO_LINK_t			tLink;			///< Sink or source of audio stream (near end, far end, both)
	AUDIO_MODE_t			tMode;			///< Mode of operation (playbakc, record, telephony)
	Media_t                 tFormat;        ///< Media format
	UInt32					param1;			///< Generic parameter 1
	UInt32					param2;			///< Generic parameter 2
	Boolean					flag;			///< Flag to identify the meaning of generic parameter
        fBufferFill_CB_t	*bufferFillCB;	///< Address of callback routine to fill audio data
      fBufferDump_CB_t *bufferDumpCB;
} HAL_AUDIO_Param_CodecCfg_t;

//! Parameter structure for action codes: request ACTION_AUD_SetEqualizer (ACTION_AUD_SetEqualizerBandGains)
#define NUM_OF_EQU_BANDS    5
typedef struct {
	MixerInput_en_t     mixerInputPath;     ///< Mixer input path  (audio, poly)
    Int16	gain[NUM_OF_EQU_BANDS]; 		///<audio equalizer gain table.  unit is 1/256 dB.
/**
 Currently MMI passed the values from -3072:3072 (-12dB:12dB) in steps of 256 (1dB). 
 Since the H/W equalizer 1/4 dB we will use the step of 1/4 dB (64). 

a shift right by 6 is divide by 64 and u will get the value u need to set in the H/W.

For Example:
EQU_POP -4dB 5dB 3dB 0 dB 3dB

MMI sends to multimedai software -1024 1280 768 0 768 (each of this value 256 times the dB Value)

multimedai software send to the Driver -1024 1280 768 0 768 

The Driver can set the H/W register values as
@verbatim
Band1 setting = (-1024/64)&0x7f = -16
Band2 setting = (1280/64)&0x7f =  20
Band3 setting = (768/64)&0x7f =  12
Band4 setting = (0/64)&0x7f = 0
Band5 setting = (768/64)&0x7f = 12
@endverbatim
**/
} HAL_AUDIO_Param_Equalizer_t;

//! Parameter structure for action codes: request ACTION_AUD_SetEqualizerProfile
typedef struct {
	AudioEqualizer_en_t equalizerID;		///< Equalization type
	MixerInput_en_t     mixerInputPath;     ///< Mixer input path
} HAL_AUDIO_Param_EqualizerProfile_t;


//! Parameter structure for action codes: request ACTION_AUD_SetMixer
typedef struct 
{
    Boolean musicSelect;          ///< music input on/off
    Boolean polyringerSelect;     ///< polyriger input on/off
    Boolean voiceSelect;          ///< voice input on/off
    UInt8 leftLevel;              ///< Weight of left input (0 ~ 100)
    UInt8 rightLevel;             ///< Weight of right input (100 - leftLevel)
} HAL_AUDIO_Param_Mixer_t;

//! Parameter structure for action codes: request ACTION_AUD_SetSTWidening 
typedef struct {
	Boolean 	OnOff;					///< Turn on/off stereo widening
	Int8 		delay;					///< stereo widening delay
} HAL_AUDIO_Param_STW_t;


//! Parameter structure for action codes: ACTION_AUD_CtrlAudAlg
typedef struct {
	UInt32				algSwitch;		///< Bitmap for dsp algorithms: bit 0=EC, 1=NS, 2=...  
	UInt32              reserved;       ///< reserved for future usage
} HAL_AUDIO_Param_AlgCfg_t;

#ifdef FEATURE_AUDIO_VOICECALL16K
//! Parameter structure for action codes: ACTION_AUD_RateChange
typedef struct {
	AudioClientID_en_t  	audioID;		///< Audio client ID
} HAL_AUDIO_Param_RateCfg_t;
#endif //FEATURE_AUDIO_VOICECALL16K

typedef enum {
	HAL_AUDIO_TUNE_PARAM_EXTAMP_PGA,
	HAL_AUDIO_TUNE_PARAM_EXTAMP_PREPGA
} HAL_AUDIO_TUNE_PARAM_TYPE_t;

typedef struct {
	HAL_AUDIO_TUNE_PARAM_TYPE_t		paramType;
	UInt32							paramValue;
} HAL_AUDIO_Param_Tune_t;

//! Audio parameter type.
typedef enum {
	GET_AUDIO_ID,			///< Get current audio client ID
	GET_AUDIO_CHNL,			///< Get curent audio channel
	GET_PLAYBACK_POSITION,	///< Get playback position
	GET_PLAYBACK_DURATION,	///< Get playback duration
	GET_ANALYZER,			///< Returns per-band measurements of intensity (for graphical display, but not supported now)
	GET_GENERIC_PARAM1,		///< Get generic parameter 1
	GET_GENERIC_PARAM2,		///< Get generic parameter 2
	GET_VOLUME_MIN,         ///<Get volume min
	GET_VOLUME_MAX,         ///<Get volume max
	GET_VOLUME_DEFAULT,     ///<Get volume default	
	GET_AUDIO_VOLUMELEVEL,  ///< Get current audio volume level
	GET_MICGAIN_MIN,        ///<Get mic gain min
	GET_MICGAIN_MAX,        ///<Get mic gain max
	GET_MICGAIN_DEFAULT,    ///<Get mic gain default
	GET_MICGAIN_INDEX,		///<Get mic gain index
	GET_AUDIO_MODE,			///< Get current audio mode
	GET_MUTE_STATUS			///< Get mic mute status
} AudioParam_en_t;

//! Parameter structure for action codes: ACTION_AUD_GetParam.
typedef struct {
	AudioClientID_en_t 	audioID;	///< Audio client ID
	AudioParam_en_t		paramType;	///< parameter type
	UInt32 				*paramPtr;	///< parameter pointer
} HAL_AUDIO_Get_Param_t;

//! Audio tone status.
typedef enum {
	TONE_STARTED,           ///< Tone generation started
	TONE_STOPPED            ///< Tone generation stopped
} HAL_AUDIO_Tone_Status_t;

#if defined(SS_2153)
//! Parameter structure for action codes: ACTION_AUD_SetCustomGain
typedef struct {
	UInt16 				spk_pga;	///< Logical PGA gain level at speaker output
	UInt16 				mic_pga;	///< Logical PGA gain level at microphone input
	UInt16 				mixer_gain;	///< Logical gain level at mixer
} HAL_AUDIO_Param_CustomGain_t;
#endif // SS_2153

//! Audio control message type.
typedef union{	
	HAL_AUDIO_Param_AudioID_t 			param_audioID;
	HAL_AUDIO_Param_Tone_t 				param_tone;
	HAL_AUDIO_Param_GenericTone_t 		param_genericTone;
	HAL_AUDIO_Param_SpkrVol_t			param_speakerVolume;
	HAL_AUDIO_Param_MicGain_t			param_micgain_index;
	HAL_AUDIO_Param_PathCfg_t 			param_pathCfg;
	HAL_AUDIO_Param_CodecCfg_t			param_codecCfg;
	HAL_AUDIO_Param_Equalizer_t 		param_equalizer;
	HAL_AUDIO_Param_EqualizerProfile_t  param_equalizerProfile;
	HAL_AUDIO_Param_STW_t 				param_3D_widening;
	HAL_AUDIO_Param_Mixer_t             param_mixer;
#if defined(SS_2153)
	HAL_AUDIO_Param_CustomGain_t		param_custom_gain;
#endif // SS_2153
	HAL_AUDIO_Param_AlgCfg_t        	param_algCfg;
#ifdef FEATURE_AUDIO_VOICECALL16K
	HAL_AUDIO_Param_RateCfg_t			param_rateCfg;
#endif // FEATURE_AUDIO_VOICECALL16K
	HAL_AUDIO_Param_Tune_t				param_tune;
} HAL_AUDIO_Control_Params_un_t; 

//! HAL audio device configuration structure, currently is not used
typedef struct
{
	//HAL_AUDIO_OperationMode_en_t operation_mode;
	Boolean device_ACTIVE;			///< Means it is initialized.
	UInt32	performance_required;	///< Percentile. Intended for clock/power management. This parameter now has no effect, so just set it to 100.
} HAL_AUDIO_CONFIG_st_t;

/**
 * @addtogroup AudioDrvGroup
 * @{
 */

//! This function initializes audio devices to default state, and initializes audio device drivers.
//! This function only needs to be called once after phone boots up.
Result_t HAL_AUDIO_Init(
	HAL_AUDIO_CONFIG_st_t *config		///< (in) Device configuration data.
	);

//! This function perform different actions corresponding to the passed parameters.
Result_t HAL_AUDIO_Ctrl(
	HAL_AUDIO_ACTION_en_t action_code,	///< (in) Action code to identify an operation.
	void	*param,						///< (io) Input/Output parameters associated with operation.
	void	*callback					///< (in) Callback function associated with the operation. E.g., call back when the operation is done.
	);

/** @} */

/* below 2 apis are used for xscripts */
Result_t HAL_AudioProcess(
	HAL_AUDIO_ACTION_en_t action_code,
	HAL_AUDIO_Control_Params_un_t *arg_param,
	void *callback
	);

//! API to get parameter. It doesn't go through the task/message layer of the audio controller
Result_t HAL_AUDIO_GetParam(
	HAL_AUDIO_Get_Param_t *paramPtr
	);

#endif	//_HAL_AUDIO_H__
