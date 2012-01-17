/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2007 - 2012  Broadcom Corporation                                                        */
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
*   @file   audio_consts.h
*
*   @brief  This file contains constants related to audio.
*
****************************************************************************/

#ifndef _INC_AUDIO_CONSTS_H_
#define _INC_AUDIO_CONSTS_H_


/****************************************************************
 * The following part shows all the constant values needed in audio baseband code.
 ****************************************************************/
typedef enum AUDIO_GAIN_FORMAT_t
{
    AUDIO_GAIN_FORMAT_mB,     // gain in milli Bel = 1/100 dB
    AUDIO_GAIN_FORMAT_DSP_VOICE_VOL_GAIN,      // CUSTOMER TABLE: DSP voice volume dB from sysparm
    AUDIO_GAIN_FORMAT_FM_RADIO_DIGITAL_VOLUME_TABLE,   // CUSTOMER TABLE: FM Radio audio gain table
    AUDIO_GAIN_FORMAT_INVALID
} AUDIO_GAIN_FORMAT_t;

/* enable this flag when cp image is in lmp */
#define USE_NEW_AUDIO_PARAM
//#undef USE_NEW_AUDIO_PARAM

/**
	audio application (2-D audio parameters profile)
**/
typedef enum {
    AUDIO_APP_VOICE_CALL = 0, /*AUDIO_APP_VOICE_CALL_NB*/
    AUDIO_APP_VOICE_CALL_WB,
    AUDIO_APP_MUSIC,
    AUDIO_APP_RECORDING_HQ,
    AUDIO_APP_RECORDING, /* AUDIO_APP_RECORDING_LQ */
    AUDIO_APP_RECORDING_GVS,
    AUDIO_APP_FM,/*AUDIO_APP_FM_RADIO*/
    AUDIO_APP_VOIP,
    AUDIO_APP_VOIP_INCOMM,
    AUDIO_APP_VT_CALL, /*AUDIO_APP_VT_NB*/
    AUDIO_APP_VT_CALL_WB, /*AUDIO_APP_VT_WB*/
    AUDIO_APP_LOOPBACK,
    AUDIO_APP_RESERVED12 = 12,
    AUDIO_APP_RESERVED13 = 13,
    AUDIO_APP_RESERVED14 = 14,
    AUDIO_APP_RESERVED15 = 15
} AudioApp_t; // audio profiles (Audio applications)

#define AudioProfile_t AudioApp_t

/**
	audio modes (audio parameters profile)
**/
typedef enum {
    AUDIO_MODE_HANDSET = 0,
    AUDIO_MODE_HEADSET = 1,
    AUDIO_MODE_HANDSFREE = 2,
    AUDIO_MODE_BLUETOOTH = 3,
    AUDIO_MODE_SPEAKERPHONE = 4,
    AUDIO_MODE_TTY = 5,
    AUDIO_MODE_HAC = 6,
    AUDIO_MODE_USB = 7,
    AUDIO_MODE_RESERVE = 8,
#if !defined(USE_NEW_AUDIO_PARAM)
    AUDIO_MODE_HANDSET_WB =9,
    AUDIO_MODE_HEADSET_WB =10,
    AUDIO_MODE_HANDSFREE_WB =11,
    AUDIO_MODE_BLUETOOTH_WB =12,
    AUDIO_MODE_SPEAKERPHONE_WB =13,
    AUDIO_MODE_TTY_WB =14,
    AUDIO_MODE_HAC_WB =15,
    AUDIO_MODE_USB_WB =16,
    AUDIO_MODE_RESERVE_WB =17,
#endif
    AUDIO_MODE_INVALID  // numbers of modes above this line has to be the same as the AUDIO_MODE_NUMBER
} AudioMode_t;  // Audio Profiles

//if changed, must update SPKR_Mapping_Table and BCM_CTL_SINK_LINES
typedef enum AUDIO_SINK_Enum_t
{
    AUDIO_SINK_HANDSET,  //Earpiece
    AUDIO_SINK_HEADSET,
    AUDIO_SINK_HANDSFREE,
    AUDIO_SINK_BTM,   //Bluetooth HFP (mono) (PCM Interface)
    AUDIO_SINK_LOUDSPK = 4,
    AUDIO_SINK_TTY,
    AUDIO_SINK_HAC,
    AUDIO_SINK_USB,
    AUDIO_SINK_BTS, //Bluetooth stereo (A2DP) (UART interface)
    AUDIO_SINK_I2S,
    AUDIO_SINK_VIBRA,
    AUDIO_SINK_HEADPHONE = 11, 	// A special headset which only has speakers and no microphone.
	//Any valid enums that needs to be exposed to user for playback needs to be added before AUDIO_SINK_VALID_TOTAL
    AUDIO_SINK_VALID_TOTAL,
	AUDIO_SINK_MEM,
	AUDIO_SINK_DSP,
    //these four are not used now:
    AUDIO_SINK_EP_STEREO,
    AUDIO_SINK_IHF_STEREO,
    AUDIO_SINK_HS_LEFT,
    AUDIO_SINK_HS_RIGHT,

	AUDIO_SINK_UNDEFINED,
    AUDIO_SINK_TOTAL_COUNT
} AUDIO_SINK_Enum_t;

//if changed, must update MIC_Mapping_Table and BCM_CTL_SRC_LINES
typedef enum AUDIO_SOURCE_Enum_t
{
    AUDIO_SOURCE_UNDEFINED,
    AUDIO_SOURCE_ANALOG_MAIN,
    AUDIO_SOURCE_ANALOG_AUX,
    AUDIO_SOURCE_DIGI1,  // DMIC1
    AUDIO_SOURCE_DIGI2,  // DMIC2
	AUDIO_SOURCE_DIGI3,
	AUDIO_SOURCE_DIGI4,
    AUDIO_SOURCE_MIC_ARRAY1, // reserved 
    AUDIO_SOURCE_MIC_ARRAY2, // reserved 
    AUDIO_SOURCE_BTM,    //Bluetooth Mono Headset Mic  (PCM_Interface)
    AUDIO_SOURCE_USB,  //USB headset Mic
    AUDIO_SOURCE_I2S,
	//Any valid enums that needs to be exposed to user for recording needs to be added before AUDIO_SOURCE_VALID_TOTAL
    AUDIO_SOURCE_RESERVED1, // reserved                     
    AUDIO_SOURCE_RESERVED2, // reserved 
    AUDIO_SOURCE_VALID_TOTAL,
    AUDIO_SOURCE_SPEECH_DIGI, //Digital Mic1/Mic2 in recording/Normal Quality Voice call.
    AUDIO_SOURCE_MEM,
    AUDIO_SOURCE_DSP,
    AUDIO_SOURCE_TOTAL_COUNT
} AUDIO_SOURCE_Enum_t;


#define AUDIO_SAMPLING_RATE_t unsigned int

#define AUDIO_SAMPLING_RATE_UNDEFINED ((AUDIO_SAMPLING_RATE_t)0)
#define AUDIO_SAMPLING_RATE_8000	((AUDIO_SAMPLING_RATE_t)8000)	///< 8K
#define AUDIO_SAMPLING_RATE_11030	((AUDIO_SAMPLING_RATE_t)11030)	///< 11.025K
#define AUDIO_SAMPLING_RATE_12000	((AUDIO_SAMPLING_RATE_t)12000)	///< 12K
#define AUDIO_SAMPLING_RATE_16000	((AUDIO_SAMPLING_RATE_t)16000)	///< 16K
#define AUDIO_SAMPLING_RATE_22060	((AUDIO_SAMPLING_RATE_t)22060)	///< 22.05K
#define AUDIO_SAMPLING_RATE_24000	((AUDIO_SAMPLING_RATE_t)24000)	///< 24K
#define AUDIO_SAMPLING_RATE_32000	((AUDIO_SAMPLING_RATE_t)32000)	///< 32K
#define AUDIO_SAMPLING_RATE_44100	((AUDIO_SAMPLING_RATE_t)44100)	///< 44.1K
#define AUDIO_SAMPLING_RATE_48000	((AUDIO_SAMPLING_RATE_t)48000)	///< 48K
#define AUDIO_SAMPLING_RATE_88200	((AUDIO_SAMPLING_RATE_t)88200)
#define AUDIO_SAMPLING_RATE_96000	((AUDIO_SAMPLING_RATE_t)96000)

typedef enum AUDIO_NUM_OF_CHANNEL_t
{
    AUDIO_CHANNEL_NUM_NONE = 0,
    AUDIO_CHANNEL_MONO,				//!< Mono channel (data)
    AUDIO_CHANNEL_STEREO,			//!< Stereo channel (data)
    AUDIO_CHANNEL_STEREO_LEFT,		//!< Stereo channel (data), but only make left channel into mixer.
    AUDIO_CHANNEL_STEREO_RIGHT		//!< Stereo channel (data), but only make right channel into mixer.
} AUDIO_NUM_OF_CHANNEL_t;

typedef UInt32 AUDIO_BITS_PER_SAMPLE_t;

#define AUDIO_8_BIT_PER_SAMPLE  8
#define AUDIO_16_BIT_PER_SAMPLE 16
#define AUDIO_18_BIT_PER_SAMPLE 18
#define AUDIO_24_BIT_PER_SAMPLE 24
#define AUDIO_32_BIT_PER_SAMPLE 32

#endif   //_INC_AUDIO_CONSTS_H_

