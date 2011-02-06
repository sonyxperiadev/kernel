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
*   @file   audio_consts.h
*
*   @brief	This file contains constants used by audio
*
****************************************************************************/

#ifndef _INC_AUDIO_CONSTS_H_
#define _INC_AUDIO_CONSTS_H_

/****************************************************************
 * The following part configures the audio baseband code..
 ****************************************************************/

// This flag is used to turn on/off the codes which control the audio codec hardware
// in the audio driver code which is used to playing music.
//If this flag is defined, the control on the audio codec hardware is turned off in the
// audio driver codec which is to playing multimedia.
// If this flag is not defined, the control is turned on.
#define AUDIO_CODEC_HW_CONTROL


#ifdef BCM2153_FAMILY_BB_CHIP_BOND
#define FEATURE_AUDIO_VOICECALL16K
#else
#undef FEATURE_AUDIO_VOICECALL16K
#endif 



/****************************************************************
 * The following part shows all the constant values needed in audio baseband code.
 ****************************************************************/
/**
	Audio Volume
**/
#define	AUDIO_VOLUME_MUTE		0 						///< mute the audio
#if defined (SS_2153)
#define	AUDIO_VOLUME_MIN		0x001						///< lowest
#define	AUDIO_VOLUME_MAX 		0x1FF						///< loudest
#define	AUDIO_VOLUME_DEFAULT	0x180						///< default Music Audio volume (AUDIO_VOLUME_MAX*3/4)
#define	USE_VOLUME_CONTROL_IN_DB // 08032009 michael
#else // SS_2153
#define	AUDIO_VOLUME_MIN		1 						///< lowest
#define	AUDIO_VOLUME_MAX 		40						///< loudest
#define	AUDIO_VOLUME_DEFAULT	AUDIO_VOLUME_MAX*3/4	///< default Music Audio volume
#undef USE_VOLUME_CONTROL_IN_DB // 08032009 michael
#endif // SS_2153

/**
	Audio MICGAIN Index (3 dB/step)
**/
#define	AUDIO_MICGAIN_MIN		0 						///< lowest = 0dB
#define	AUDIO_MICGAIN_MAX 		14						///< loudest = 42dB
#define	AUDIO_MICGAIN_DEFAULT	((AUDIO_MICGAIN_MIN+AUDIO_MICGAIN_MAX)>>1)	///< default

/**
	 Sound Channels:
	 - Voice call comes out from AUDIO_CHNL_HANDSET
	 - MP3, AMR, AAC, MIDI, iMelody, eMelody can be out from AUDIO_CHNL_HEADPHONE or AUDIO_CHNL_SPEAKER
	 - Ringtone, which is usually in MIDI format, comes out from AUDIO_CHNL_SPEAKER
**/
#if 1
typedef	enum
{
	AUDIO_CHNL_HANDSET=0,				///< voice speaker
	AUDIO_CHNL_EARPIECE=0,				///< voice speaker  (this is just another name for number 0.)
	AUDIO_CHNL_HEADPHONE=1,				///< headphone
	AUDIO_CHNL_HANDSFREE=2,				///< CAR_KIT
	AUDIO_CHNL_BT_MONO=3,				///< Bluetooth mono headset
	AUDIO_CHNL_BLUETOOTH=3,				///< Bluetooth headset (this is just another name for number 3.)
	AUDIO_CHNL_SPEAKER,					///< loud speaker
	AUDIO_CHNL_TTY,						///< TTY through headset port
	AUDIO_CHNL_HAC,						///< HAC
	AUDIO_CHNL_USB,						///< USB headset
	AUDIO_CHNL_BT_STEREO,				///< Bluetooth stereo headset
	AUDIO_CHNL_RESERVE,					///< RESERVED, not specified
	//AUDIO_CHNL_UNKNOWN, 				///< replaced by AUDIO_CHNL_RESERVE
	AUDIO_CHNL_TOTAL
} AUDIO_CHANNEL_t;
#else
typedef	enum
{
	AUDIO_CHNL_HANDSET=0,				///< Handset.  voice speaker
	AUDIO_CHNL_EARPIECE=0,				///< Handset.  voice speaker
	AUDIO_CHNL_HEADPHONE=1,				///< headphone stereo
	AUDIO_CHNL_HEADPHONE_MONO,			///< headphone mono
	AUDIO_CHNL_SPEAKER,					///< loud speaker mono
	AUDIO_CHNL_SPEAKER_STEREO,			///< loud speaker stereo
	AUDIO_CHNL_SPEAKER_PHONE,			///< SPEAKER PHONE
	AUDIO_CHNL_BT,						///< Bluetooth mono
	AUDIO_CHNL_BT_STEREO,				///< Bluetooth stereo
	AUDIO_CHNL_TTY,
	AUDIO_CHNL_CAR_KIT,
	AUDIO_CHNL_TOTAL
} AUDIO_CHANNEL_t;
#endif

//can be merged with AudioChannel_t.
typedef enum {
	AUDIO_MODE_HANDSET = 0,
	AUDIO_MODE_HEADSET,
	AUDIO_MODE_HANDSFREE,
	AUDIO_MODE_BLUETOOTH,
	AUDIO_MODE_SPEAKERPHONE,
	AUDIO_MODE_TTY,
	AUDIO_MODE_HAC,
	AUDIO_MODE_USB,
	AUDIO_MODE_RESERVE,
#ifdef BCM2153_FAMILY_BB_CHIP_BOND
	AUDIO_MODE_HANDSET_WB,
	AUDIO_MODE_HEADSET_WB,
	AUDIO_MODE_HANDSFREE_WB,
	AUDIO_MODE_BLUETOOTH_WB,
	AUDIO_MODE_SPEAKERPHONE_WB,
	AUDIO_MODE_TTY_WB,
	AUDIO_MODE_HAC_WB,
	AUDIO_MODE_USB_WB,
	AUDIO_MODE_RESERVE_WB,
#endif //  BCM2153_FAMILY_BB_CHIP_BOND
	AUDIO_MODE_INVALID  // numbers of modes above this line has to be the same as the AUDIO_MODE_NUMBER
} AudioMode_t;  // Audio Profiles

#define	AUDIO_CHNL_ANY	AUDIO_CHNL_TOTAL

/** \brief Audio data sampling rate in unit of Hz.
*/
typedef enum
{
	AUDIO_SAMPLERATE_8000,
	AUDIO_SAMPLERATE_11030,
	AUDIO_SAMPLERATE_12000,
	AUDIO_SAMPLERATE_16000,
	AUDIO_SAMPLERATE_22060,
	AUDIO_SAMPLERATE_24000,
	AUDIO_SAMPLERATE_32000,
	AUDIO_SAMPLERATE_48000,
	AUDIO_SAMPLERATE_44100,
	AUDIO_SAMPLERATE_96000
} AudioSampleRate_en_t;


//! Used in I2S_PlayAudio() to indicate the audio data are for single (left) channel
//! or both channels. WAVE file header defines mono (0x01) or stereo (0x02).
typedef enum
{
//Follow i2s_drv.h defination at this point
	TYPE_STEREO,						//!< Stereo channel
	TYPE_MONO							//!< Mono channel 
} AudioMonoStereo_en_t;

/** \brief Audio data format in unit of bits per sample.
*/
typedef enum
{
	EIGHT_BIT_PER_SAMPLE = 8,
	SIXTEEN_BIT_PER_SAMPLE = 16
} AudioBitPerSample_en_t;


//for VPM.i ( AMR playback and recording)
/**
  	Available Audio Modes
**/

typedef enum
{
	AUDIO_MODE_PLAYBACK,				///< Audio playing
	AUDIO_MODE_RECORD,					///< Audio recording
	AUDIO_MODE_STREAM,					///< Audio/Video streaming for media player
	AUDIO_MODE_TELEPHONY,				///< Application-based telephony (e.g., video telephony)
	AUDIO_MODE_TELEPHONY_BT_TAP,		///< Application-based telephony (e.g., video telephony)
	AUDIO_MODE_TOTAL
} AUDIO_MODE_t;

/**
	 Supported Link Modes
**/

typedef enum
{
	AUDIO_LINK_NEAR,					///< For user only
	AUDIO_LINK_FAR,						///< For caller only
	AUDIO_LINK_BOTH,					///< For both user and caller
	AUDIO_LINK_TOTAL
} AUDIO_LINK_t;

#define	AUDIO_FORMAT_AMR					MEDIA_AMR					///< Adjustable Multi-Rate

/**
	 Supported Adaptive Multi-Rate Narrow-Band (AMR-NB) Sampling Rates
**/

typedef enum
{
	AUDIO_AMR_475,						///< 4.75 kbps
	AUDIO_AMR_515,						///< 5.15 kbps
	AUDIO_AMR_590,						///< 5.90 kbps
	AUDIO_AMR_670,						///< 6.70 kbps
	AUDIO_AMR_740,						///< 7.40 kbps
	AUDIO_AMR_795,						///< 7.95 kbps
	AUDIO_AMR_102,						///< 10.2 kbps
	AUDIO_AMR_122						///< 12.1 kbps
} AUDIO_AMR_t;

/**
	 Supported Adaptive Multi-Rate Wide-Band (AMR-WB) Sampling Rates
**/

typedef enum
{
	AUDIO_AMRWB_660,					///< 6.60 kbps
	AUDIO_AMRWB_885,					///< 8.85 kbps
	AUDIO_AMRWB_1265,					///< 12.65 kbps
	AUDIO_AMRWB_1425,					///< 14.25 kbps
	AUDIO_AMRWB_1585,					///< 15.85 kbps
	AUDIO_AMRWB_1825,					///< 18.25 kbps
	AUDIO_AMRWB_1985,					///< 19.85 kbps
	AUDIO_AMRWB_2305,					///< 23.05 kbps
	AUDIO_AMRWB_2385					///< 23.85 kbps
} AUDIO_AMRWB_t;

#define AUDIO_EQU_DEFINED 
//! Audio equalization type.
typedef enum {
	EQU_NORMAL,		///< Normal
	EQU_Bass,		///< Bass
	EQU_Live,		///< Live
	EQU_Classic,	///< Classic
	EQU_Rock,		///< Rock
	EQU_Jazz,		///< Jazz
	EQU_Invalid
} AudioEqualizer_en_t;

//! Audio tone type.
typedef enum {
	TONE_TYPE_DTMF = 2,
	TONE_TYPE_GENERIC,
	TONE_TYPE_ALL
} AudioToneType_en_t;

typedef enum
{
// DTMF Tones
	SPEAKERTONE_ZERO = 0,
	SPEAKERTONE_ONE,
	SPEAKERTONE_TWO,
	SPEAKERTONE_THREE,
	SPEAKERTONE_FOUR,
	SPEAKERTONE_FIVE,
	SPEAKERTONE_SIX,
	SPEAKERTONE_SEVEN,
	SPEAKERTONE_EIGHT,
	SPEAKERTONE_NINE,
	SPEAKERTONE_POUND,
	SPEAKERTONE_STAR,

// Supervisory Tones
	SPEAKERTONE_DIALING,
	SPEAKERTONE_BUSY,
	SPEAKERTONE_RINGING,
	SPEAKERTONE_CONGESTION,
	SPEAKERTONE_RADIO_ACKN,
	SPEAKERTONE_NO_RADIO_PATH,
	SPEAKERTONE_CALL_WAITING,
	SPEAKERTONE_ERROR
} SpeakerTone_t;

typedef enum
{
	READ_AUDVOC_AEQMODE = 1,
	WRITE_AUDVOC_AEQMODE = 2,
	GET_CP_AUDIO_MODE = 3,
	UPDATE_AUDIO_MODE = 4,
	FLUSH_POLY_PIFIFODATA = 5,
	FLUSH_STEREO_AIFIFODATA = 6,
	AP_CP_MSG_TOTAL
}AP_CONTROL_CP_MSGS;

typedef enum
{
	CONTROL_ECHO_CANCELLATION = 1,
	ENABLE_DSP_AUDIO = 2,
	DISABLE_DSP_AUDIO = 3,
	TURN_UL_COMPANDER_ON = 4,
	TURN_UL_COMPANDER_OFF = 5,
	SET_DSP_TONE_VOLUME = 6,
	SET_DSP_AMR_VOLUME = 7,
	SET_DSP_UL_GAIN = 8,
	GET_DSP_UL_GAIN =9,
	MUTE_DSP_UL = 10,
	UNMUTE_DSP_UL = 11,
	CHECK_DSP_AUDIO = 12,
	ENABLE_DSP_DTX = 13,
#ifdef FEATURE_AUDIO_VOICECALL16K
	MUTE_DSP_DL = 14,
	UNMUTE_DSP_DL = 15,
#endif // FEATURE_AUDIO_VOICECALL16K
	AP_DSP_MSG_TOTAL
}AP_CONTROL_DSP_MSGS;

#endif

