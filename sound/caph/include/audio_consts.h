/******************************************************************************
*
* Copyright 2009 - 2012  Broadcom Corporation
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2 (the GPL),
*  available at
*
*      http://www.broadcom.com/licenses/GPLv2.php
*
*  with the following added to such license:
*
*  As a special exception, the copyright holders of this software give you
*  permission to link this software with independent modules, and to copy and
*  distribute the resulting executable under terms of your choice, provided
*  that you also meet, for each linked independent module, the terms and
*  conditions of the license of that module.
*  An independent module is a module which is not derived from this software.
*  The special exception does not apply to any modifications of the software.
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
******************************************************************************/

/**
*
*   @file   audio_consts.h
*
*   @brief  This file contains constants related to audio.
*
****************************************************************************/

#ifndef _AUDIO_CONSTS_H_
#define _AUDIO_CONSTS_H_

/****************************************************************
 * The following part shows all the constant values needed in audio baseband code.
 ****************************************************************/
enum _AUDIO_GAIN_FORMAT_t {
		/* gain in milli Bel = 1/100 dB */
	AUDIO_GAIN_FORMAT_mB,
		/* CUSTOMER TABLE: DSP voice volume dB from sysparm */
	AUDIO_GAIN_FORMAT_DSP_VOICE_VOL_GAIN,
		/* CUSTOMER TABLE: FM Radio audio gain table */
	AUDIO_GAIN_FORMAT_FM_RADIO_DIGITAL_VOLUME_TABLE,
	AUDIO_GAIN_FORMAT_INVALID
};
#define AUDIO_GAIN_FORMAT_t enum _AUDIO_GAIN_FORMAT_t

enum _AUDIO_GAIN_LR {
	AUDIO_HS_RIGHT,
	AUDIO_HS_LEFT,
	AUDIO_HS_BOTH
};
#define AUDIO_GAIN_LR_t enum _AUDIO_GAIN_LR

/**
	audio application (2-D audio parameters profile)
**/
enum _AudioApp_t {
	AUDIO_APP_VOICE_CALL = 0,	/*AUDIO_APP_VOICE_CALL_NB */
	AUDIO_APP_VOICE_CALL_WB,
	AUDIO_APP_MUSIC,
	AUDIO_APP_RECORDING_HQ,
	AUDIO_APP_RECORDING,	/* AUDIO_APP_RECORDING_LQ */
	AUDIO_APP_RECORDING_GVS,
	AUDIO_APP_FM,		/*AUDIO_APP_FM_RADIO */
	AUDIO_APP_VOIP,
	AUDIO_APP_VOIP_INCOMM,
	AUDIO_APP_VT_CALL,	/*AUDIO_APP_VT_NB */
	AUDIO_APP_VT_CALL_WB,	/*AUDIO_APP_VT_WB */
	AUDIO_APP_LOOPBACK,
	AUDIO_APP_RESERVED12 = 12,
	AUDIO_APP_RESERVED13 = 13,
	AUDIO_APP_RESERVED14 = 14,
	AUDIO_APP_RESERVED15 = 15
};
#define AudioApp_t enum _AudioApp_t

#define AudioProfile_t AudioApp_t

/**
	audio modes (audio parameters profile)
**/
enum _AudioMode_t {
	AUDIO_MODE_HANDSET = 0,
	AUDIO_MODE_HEADSET = 1,
	AUDIO_MODE_HANDSFREE = 2,
	AUDIO_MODE_BLUETOOTH = 3,
	AUDIO_MODE_SPEAKERPHONE = 4,
	AUDIO_MODE_TTY = 5,
	AUDIO_MODE_HAC = 6,
	AUDIO_MODE_USB = 7,
	AUDIO_MODE_RESERVE = 8,
	/* numbers of modes above this line has to be the same as
	the AUDIO_MODE_NUMBER */
	AUDIO_MODE_INVALID
};
#define AudioMode_t enum _AudioMode_t

/*if changed, must update SPKR_Mapping_Table and BCM_CTL_SINK_LINES */
enum _AUDIO_SINK_Enum_t {
	AUDIO_SINK_HANDSET,	/*Earpiece */
	AUDIO_SINK_HEADSET,
	AUDIO_SINK_HANDSFREE,
	AUDIO_SINK_BTM,		/*Bluetooth HFP (mono) (PCM Interface) */
	AUDIO_SINK_LOUDSPK = 4,
	AUDIO_SINK_TTY,
	AUDIO_SINK_HAC,
	AUDIO_SINK_USB,
	AUDIO_SINK_BTS,		/*Bluetooth stereo (A2DP) (UART interface) */
	AUDIO_SINK_I2S,
	AUDIO_SINK_VIBRA,
	/* A special headset which only has speakers and no microphone.*/
	AUDIO_SINK_HEADPHONE = 11,
	/*Any valid enums that needs to be exposed to user for playback
	needs to be added before AUDIO_SINK_VALID_TOTAL*/
	AUDIO_SINK_VALID_TOTAL,
	AUDIO_SINK_MEM,
	AUDIO_SINK_DSP,
	AUDIO_SINK_RESERVED1,
	AUDIO_SINK_RESERVED2,
	AUDIO_SINK_RESERVED3,
	AUDIO_SINK_RESERVED4,

	/* AUDIO_SINK_UNDEFINED value can't be changed due to customer req */
	AUDIO_SINK_UNDEFINED = 19,
	AUDIO_SINK_TOTAL_COUNT
};
#define AUDIO_SINK_Enum_t enum _AUDIO_SINK_Enum_t

/*if changed, must update MIC_Mapping_Table and BCM_CTL_SRC_LINES */
enum _AUDIO_SOURCE_Enum_t {
	AUDIO_SOURCE_UNDEFINED,
	AUDIO_SOURCE_ANALOG_MAIN,
	AUDIO_SOURCE_ANALOG_AUX,
	AUDIO_SOURCE_DIGI1,
	AUDIO_SOURCE_DIGI2,
	AUDIO_SOURCE_DIGI3,
	AUDIO_SOURCE_DIGI4,
	AUDIO_SOURCE_MIC_ARRAY1,
	AUDIO_SOURCE_MIC_ARRAY2,
		/* Bluetooth Mono Headset Mic  (PCM_Interface) */
	AUDIO_SOURCE_BTM,
		/* USB headset Mic */
	AUDIO_SOURCE_USB,
	AUDIO_SOURCE_I2S, /*11*/
	/*Any valid enums that needs to be exposed to user for recording
	needs to be added before AUDIO_SOURCE_VALID_TOTAL*/
	AUDIO_SOURCE_RESERVED1,
	AUDIO_SOURCE_RESERVED2,
	AUDIO_SOURCE_VALID_TOTAL,
	/*Digital Mic1/Mic2 in recording/Normal Quality Voice call.*/
	AUDIO_SOURCE_SPEECH_DIGI,
	AUDIO_SOURCE_MEM, /*16*/
	AUDIO_SOURCE_DSP,
	AUDIO_SOURCE_TOTAL_COUNT
};
#define AUDIO_SOURCE_Enum_t enum _AUDIO_SOURCE_Enum_t

#define AUDIO_SAMPLING_RATE_t unsigned int

#define AUDIO_SAMPLING_RATE_UNDEFINED ((AUDIO_SAMPLING_RATE_t)0)
#define AUDIO_SAMPLING_RATE_8000	((AUDIO_SAMPLING_RATE_t)8000)
#define AUDIO_SAMPLING_RATE_11030	((AUDIO_SAMPLING_RATE_t)11030)
#define AUDIO_SAMPLING_RATE_12000	((AUDIO_SAMPLING_RATE_t)12000)
#define AUDIO_SAMPLING_RATE_16000	((AUDIO_SAMPLING_RATE_t)16000)
#define AUDIO_SAMPLING_RATE_22060	((AUDIO_SAMPLING_RATE_t)22060)
#define AUDIO_SAMPLING_RATE_24000	((AUDIO_SAMPLING_RATE_t)24000)
#define AUDIO_SAMPLING_RATE_32000	((AUDIO_SAMPLING_RATE_t)32000)
#define AUDIO_SAMPLING_RATE_44100	((AUDIO_SAMPLING_RATE_t)44100)
#define AUDIO_SAMPLING_RATE_48000	((AUDIO_SAMPLING_RATE_t)48000)
#define AUDIO_SAMPLING_RATE_88200	((AUDIO_SAMPLING_RATE_t)88200)
#define AUDIO_SAMPLING_RATE_96000	((AUDIO_SAMPLING_RATE_t)96000)

#define AUDIO_PCMOUT1_RATE AUDIO_SAMPLING_RATE_48000
#define AUDIO_PCMOUT2_RATE AUDIO_SAMPLING_RATE_44100


enum _AUDIO_NUM_OF_CHANNEL_t {
	AUDIO_CHANNEL_NUM_NONE = 0,
	/* Mono channel (data) */
	AUDIO_CHANNEL_MONO,
	/* Stereo channel (data) */
	AUDIO_CHANNEL_STEREO,
	/* Stereo channel (data), but only make left channel into mixer.*/
	AUDIO_CHANNEL_STEREO_LEFT,
	/* Stereo channel (data), but only make right channel into mixer.*/
	AUDIO_CHANNEL_STEREO_RIGHT
};
#define AUDIO_NUM_OF_CHANNEL_t enum _AUDIO_NUM_OF_CHANNEL_t

#define AUDIO_BITS_PER_SAMPLE_t unsigned char

#endif
