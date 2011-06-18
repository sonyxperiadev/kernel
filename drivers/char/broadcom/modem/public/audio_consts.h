//***************************************************************************
//
//	Copyright © 2004-2010 Broadcom Corporation
//	
//	This program is the proprietary software of Broadcom Corporation 
//	and/or its licensors, and may only be used, duplicated, modified 
//	or distributed pursuant to the terms and conditions of a separate, 
//	written license agreement executed between you and Broadcom (an 
//	"Authorized License").  Except as set forth in an Authorized 
//	License, Broadcom grants no license (express or implied), right 
//	to use, or waiver of any kind with respect to the Software, and 
//	Broadcom expressly reserves all rights in and to the Software and 
//	all intellectual property rights therein.  IF YOU HAVE NO 
//	AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE 
//	IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE 
//	ALL USE OF THE SOFTWARE.  
//	
//	Except as expressly set forth in the Authorized License,
//	
//	1.	This program, including its structure, sequence and 
//		organization, constitutes the valuable trade secrets 
//		of Broadcom, and you shall use all reasonable efforts 
//		to protect the confidentiality thereof, and to use 
//		this information only in connection with your use 
//		of Broadcom integrated circuit products.
//	
//	2.	TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE 
//		IS PROVIDED "AS IS" AND WITH ALL FAULTS AND BROADCOM 
//		MAKES NO PROMISES, REPRESENTATIONS OR WARRANTIES, 
//		EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, 
//		WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY 
//		DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, 
//		MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A 
//		PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR 
//		COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR 
//		CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE 
//		RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.  
//
//	3.	TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT 
//		SHALL BROADCOM OR ITS LICENSORS BE LIABLE FOR 
//		(i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR 
//		EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY 
//		WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE 
//		SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE 
//		POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN 
//		EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE 
//		ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE 
//		LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE 
//		OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
//
//***************************************************************************
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
 * The following part configures the audio baseband code..
 ****************************************************************/


/****************************************************************
 * The following part shows all the constant values needed in audio baseband code.
 ****************************************************************/

#include "tones_def.h"

//move to sysInterface?
/*
	Audio Volume
*/
#define	AUDIO_VOLUME_MUTE		0 						///< mute the audio
#define	AUDIO_VOLUME_MIN		1 						///< lowest
#define	AUDIO_VOLUME_MAX 		40						///< loudest
#define	AUDIO_VOLUME_DEFAULT	AUDIO_VOLUME_MAX*3/4	///< default Music Audio volume

/*
	Audio MICGAIN Index (3 dB/step)
*/
#define	AUDIO_MICGAIN_MIN		0 						///< lowest = 0dB
#define	AUDIO_MICGAIN_MAX 		14						///< loudest = 42dB
#define	AUDIO_MICGAIN_DEFAULT	((AUDIO_MICGAIN_MIN+AUDIO_MICGAIN_MAX)>>1)	///< default

typedef enum AUDIO_GAIN_FORMAT_t 
{
	AUDIO_GAIN_FORMAT_VOL_LEVEL, // logic volume level
	AUDIO_GAIN_FORMAT_HW_REG,    // hardware register gain value
	AUDIO_GAIN_FORMAT_Q14_1,     // gain in Q14.1 
	AUDIO_GAIN_FORMAT_Q1_14,      // gain in Q1.14
	AUDIO_GAIN_FORMAT_DSP_VOICE_VOL_GAIN,      // DSP voice volume dB from sysparm
	AUDIO_GAIN_FORMAT_FM_RADIO_DIGITAL_VOLUME_TABLE      // FM Radio audio gain table
} AUDIO_GAIN_FORMAT_t;

/**
	 Sound Channels:
**/
typedef	enum
{
	AUDIO_CHNL_HANDSET = 0,				///< voice speaker
	AUDIO_CHNL_HEADPHONE = 1,			///< headphone
	AUDIO_CHNL_HANDSFREE = 2,			///< CAR_KIT
	AUDIO_CHNL_BT_MONO = 3,				///< Bluetooth mono headset
	AUDIO_CHNL_BLUETOOTH = 3,			///< Bluetooth headset (this is just another name for number 3.)
	AUDIO_CHNL_SPEAKER = 4,				///< loud speaker
	AUDIO_CHNL_TTY = 5,					///< TTY through headset port
	AUDIO_CHNL_HAC = 6,					///< HAC
	AUDIO_CHNL_USB = 7,					///< USB headset
	AUDIO_CHNL_BT_STEREO = 8,			///< Bluetooth stereo headset
	AUDIO_CHNL_I2S_TX = 9,				///< 
	AUDIO_CHNL_I2S_RX = 10,
	AUDMAN_CHANNEL_HEADPHONE = 11,		///< A special headset which only has speakers and no microphone.
	AUDMAN_CHANNEL_WB_BT = 12,			///< The WideBand BT accessory which is mono and with 16KHz sample rate
	AUDIO_CHNL_VIBRA = 13,				///< Vibrator
	AUDIO_CHNL_RESERVE = 14,			///< RESERVED, not specified
	AUDIO_CHNL_TOTAL = 15
} AUDIO_CHANNEL_t;


typedef enum  AUDIO_INPUT_CHANNEL_t
{
	AUDIO_INPUT_CHNL_DEFAULT,	///< the default input mapping from output channel
	AUDIO_INPUT_CHNL_ANALOG_MIC, ///< analog mic 1
	AUDIO_INPUT_CHNL_ANALOG_AUXMIC, ///< analog mic 2
	AUDIO_INPUT_CHNL_DIGI_MIC_1, ///< digital mic 1
	AUDIO_INPUT_CHNL_DIGI_MIC_2, ///< digital mic 2
	AUDIO_INPUT_CHNL_MIC_ARRAY_1,	///< Dual-Mic for HANDSET_VOICE_CALL
	AUDIO_INPUT_CHNL_MIC_ARRAY_2,	///< Dual-Mic for HANDSET_VOICE_CALL
	AUDIO_INPUT_CHNL_TOTAL,
}AUDIO_INPUT_CHANNEL_t;

#define	AUDIO_MODE_NUMBER		9	///< Up to 9 Audio Profiles (modes) after 213x1
#define	AUDIO_APP_NUMBER		6///< number of profiles (applications), can be extended
#define NUM_OF_ENTRY_IN_DSP_VOICE_VOLUME_TABLE		15 
#define NUM_OF_ENTRY_IN_FM_RADIO_DIGITAL_VOLUME		15 

/**
	audio application (2-D audio parameters profile)
**/
typedef enum {
	AUDIO_APP_VOICE_CALL = 0,
	AUDIO_APP_VOICE_CALL_WB,
	AUDIO_APP_RESERVED1,
	AUDIO_APP_RESERVED2,
	AUDIO_APP_RESERVED3,
	AUDIO_APP_RESERVED4
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
	AUDIO_MODE_INVALID  // numbers of modes above this line has to be the same as the AUDIO_MODE_NUMBER
} AudioMode_t;  // Audio Profiles



typedef UInt32 AUDIO_SAMPLING_RATE_t;

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


typedef enum AUDIO_CHANNEL_NUM_t
{
	AUDIO_CHANNEL_NUM_NONE = 0,
	AUDIO_CHANNEL_MONO,							//!< Mono channel (data)
	AUDIO_CHANNEL_STEREO,						//!< Stereo channel (data)
	AUDIO_CHANNEL_STEREO_LEFT,					//!< Stereo channel (data), but only make left channel into mixer.
	AUDIO_CHANNEL_STEREO_RIGHT,					//!< Stereo channel (data), but only make right channel into mixer.
	AUDIO_CHANNEL_NUM_TOTAL
} AUDIO_CHANNEL_NUM_t;

typedef UInt32 AUDIO_BITS_PER_SAMPLE_t;

#define AUDIO_8_BIT_PER_SAMPLE  8
#define AUDIO_16_BIT_PER_SAMPLE 16
#define AUDIO_18_BIT_PER_SAMPLE 18
#define AUDIO_24_BIT_PER_SAMPLE 24
#define AUDIO_32_BIT_PER_SAMPLE 32


//for VPM.i ( AMR playback and recording)
/**
  	Available Audio Modes (this should be rename.)
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

typedef enum AUDIO_SPEECH_LINK_t
{
	AUDIO_SPEECH_LINK_NONE,
	AUDIO_SPEECH_LINK_NEAR,
	AUDIO_SPEECH_LINK_FAR,										
	AUDIO_SPEECH_LINK_BOTH					
} AUDIO_SPEECH_LINK_t;

#define	AUDIO_FORMAT_AMR		MEDIA_AMR					///< Adjustable Multi-Rate

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
	 Supported Adaptive Multi-Rate Wide-Band (AMR-WB) Bitstream Mode
**/

typedef enum
{
	AUDIO_AMRWB_ITU_0,					///< ITU-0 format
	AUDIO_AMRWB_ITU_1,					///< ITU-1 format
	AUDIO_AMRWB_MIME					///< MIME format
} AUDIO_AMRWB_BITSTREAM_MODE_t;

/**
	 Supported Adaptive Multi-Rate Wide-Band (AMR-WB) Bit Rates
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

//! Audio equalization type.
typedef enum AudioEqualizer_en_t {
	EQU_NORMAL,		///< Normal
	EQU_Bass,		///< Bass
	EQU_Live,		///< Live
	EQU_Classic,	///< Classic
	EQU_Rock,		///< Rock
	EQU_Jazz,		///< Jazz
	EQU_Invalid
} AudioEqualizer_en_t;

#define EQU_ID_t AudioEqualizer_en_t

// Consts and types moved from audioapi_asic.h

#define		AUDIO_TONE_DURATION_FOREVER 0xFFFF
#define		AUDIO_ERRORTONE_DURATION	1000

#define PR_DAC_IIR_SIZE				25

typedef enum {
    PARAM_AUDIO_AGC_DL_DECAY,							// 0  delete
    PARAM_AUDIO_AGC_DL_ENABLE,							// 1
    PARAM_AUDIO_AGC_DL_HI_THRESH,						// 2
    PARAM_AUDIO_AGC_DL_LOW_THRESH,						// 3
    PARAM_AUDIO_AGC_DL_MAX_IDX,							// 4
    PARAM_AUDIO_AGC_DL_MAX_STEP_DOWN,					// 5
    PARAM_AUDIO_AGC_DL_MAX_THRESH,						// 6
    PARAM_AUDIO_AGC_DL_MIN_IDX,							// 7
    PARAM_AUDIO_AGC_DL_STEP_DOWN,						// 8
    PARAM_AUDIO_AGC_DL_STEP_UP,							// 9
    PARAM_AUDIO_AGC_UL_DECAY,							// 10
    PARAM_AUDIO_AGC_UL_ENABLE,							// 11
    PARAM_AUDIO_AGC_UL_HI_THRESH,						// 12
    PARAM_AUDIO_AGC_UL_LOW_THRESH,						// 13
    PARAM_AUDIO_AGC_UL_MAX_IDX,							// 14
    PARAM_AUDIO_AGC_UL_MAX_STEP_DOWN,					// 15
    PARAM_AUDIO_AGC_UL_MAX_THRESH,						// 16
    PARAM_AUDIO_AGC_UL_MIN_IDX,							// 17
    PARAM_AUDIO_AGC_UL_STEP_DOWN,						// 18
    PARAM_AUDIO_AGC_UL_STEP_UP,							// 19
    PARAM_ECHO_ADAPT_NORM_FACTOR,						// 20
    PARAM_ECHO_CANCEL_DTD_HANG,							// 21
    PARAM_ECHO_CANCEL_DTD_THRESH,						// 22
    PARAM_ECHO_CANCEL_FRAME_SAMPLES,					// 23
    PARAM_ECHO_CANCEL_HSEC_LOOP,						// 24
    PARAM_ECHO_CANCEL_HSEC_MFACT,						// 25
    PARAM_ECHO_CANCEL_HSEC_STEP,						// 26
    PARAM_ECHO_CANCEL_INPUT_GAIN,						// 27
    PARAM_ECHO_CANCEL_MAX_HSEC,							// 28
    PARAM_ECHO_CANCEL_OUTPUT_GAIN,						// 29
    PARAM_ECHO_CANCEL_UPDATE_DELAY,						// 30
    PARAM_ECHO_CANCELLING_ENABLE,						// 31
    PARAM_ECHO_CNG_BIAS,								// 32
    PARAM_ECHO_CNG_ENABLE,								// 33
    PARAM_ECHO_COUPLING_DELAY,							// 34
    PARAM_ECHO_DIGITAL_INPUT_CLIP_LEVEL,				// 35
    PARAM_ECHO_DUAL_FILTER_MODE,						// 36
    PARAM_ECHO_EN_FAR_SCALE_FACTOR,						// 37
    PARAM_ECHO_EN_NEAR_SCALE_FACTOR,					// 38
    PARAM_ECHO_FAR_IN_FILTER,							// 39
    PARAM_ECHO_FEED_FORWARD_GAIN,						// 40
    PARAM_ECHO_NLP_CNG_FILTER,							// 41
    PARAM_ECHO_NLP_DL_ENERGY_WINDOW,					// 42
    PARAM_ECHO_NLP_DL_GAIN_TABLE,						// 43
    PARAM_ECHO_NLP_DL_IDLE_UL_GAIN,						// 44
    PARAM_ECHO_NLP_DOWNLINK_VOLUME_CTRL,				// 45
    PARAM_ECHO_NLP_DTALK_DL_GAIN,						// 46
    PARAM_ECHO_NLP_DTALK_UL_GAIN,						// 47
    PARAM_ECHO_NLP_ENABLE,								// 48
    PARAM_ECHO_NLP_GAIN,								// 49
    PARAM_ECHO_NLP_MAX_SUPP,							// 50
    PARAM_ECHO_NLP_MIN_DL_PWR,							// 51
    PARAM_ECHO_NLP_MIN_UL_PWR,							// 52
    PARAM_ECHO_NLP_RELATIVE_DL_ENERGY_DECAY,			// 53
    PARAM_ECHO_NLP_RELATIVE_DL_ENERGY_WINDOW,			// 54
    PARAM_ECHO_NLP_RELATIVE_DL_W_THRESH,				// 55
    PARAM_ECHO_NLP_RELATIVE_OFFSET_DL_ACTIVE,			// 56
    PARAM_ECHO_NLP_RELATIVE_OFFSET_DTALK,				// 57
    PARAM_ECHO_NLP_TIMEOUT_VAL,							// 58
    PARAM_ECHO_NLP_UL_ACTIVE_DL_GAIN,					// 59
    PARAM_ECHO_NLP_UL_BRK_IN_THRESH,					// 60
    PARAM_ECHO_NLP_UL_ENERGY_WINDOW,					// 61
    PARAM_ECHO_NLP_UL_GAIN_TABLE,						// 62
    PARAM_ECHO_NLP_UL_IDLE_DL_GAIN,						// 63
    PARAM_ECHO_NLP_DTALK_HANG_COUNT,					// 64
    PARAM_ECHO_NLP_UL_ACTIVE_HANG_COUNT,				// 65
    PARAM_ECHO_NLP_RELATIVE_OFFSET_UL_ACTIVE,			// 66
    PARAM_ECHO_SPKR_PHONE_INPUT_GAIN,					// 67
    PARAM_ECHO_STABLE_COEF_THRESH,						// 68
    PARAM_MIC_PGA,										// 69				
    PARAM_SPEAKER_PGA,									// 70
    PARAM_NOISE_SUPP_INPUT_GAIN,						// 71
    PARAM_NOISE_SUPP_OUTPUT_GAIN,						// 72
    PARAM_NOISE_SUPPRESSION_ENABLE,						// 73
    PARAM_SIDETONE,										// 74
    PARAM_VOICE_ADC,									// 75
    PARAM_VOICE_DAC,									// 76
	//PARAM_AUDIO_CHANNEL,								// 77
	//PARAM_AUDIO_DEVICE_TYPE,							// 78
	PARAM_NOISE_SUPP_MIN=79,								// 79
	PARAM_NOISE_SUPP_MAX,								// 80
	PARAM_VOLUME_STEP_SIZE,								// 81
	PARAM_NUM_SUPPORTED_VOLUME_LEVELS,					// 82
	//PARAM_DAC_FILTER_SCALE_FACTOR,						// 83
	
	PARAM_ECHO_DUAL_EC_ECLEN = 84,							// 84
	PARAM_ECHO_DUAL_EC_RinLpcBuffSize,					// 85
	PARAM_ECHO_DUAL_E__RinCirBuffSizeModij,				// 86
	PARAM_ECHO_DUAL_EC_DT_TH_ERL_dB,					// 87
	PARAM_ECHO_DUAL_EC_echo_step_size_gain,				// 88 
	PARAM_ECHO_DUAL_EC_HANGOVER_CNT,					// 89
	PARAM_ECHO_DUAL_EC_VAD_TH_dB,						// 90
	PARAM_ECHO_DUAL_EC_DT_HANGOVER_TIME,				// 91

	PARAM_COMPANDER_FLAG,								// 92
	PARAM_EXPANDER_ALPHA,								// 93
	PARAM_EXPANDER_BETA,								// 94
	PARAM_EXPANDER_UPPER_LIMIT,							// 95
	PARAM_EXPANDER_C,									// 96
	PARAM_COMPRESSOR_ALPHA,								// 97
	PARAM_COMPRESSOR_BETA,								// 98
	PARAM_COMPRESSOR_OUTPUT_GAIN,						// 99
	PARAM_COMPRESSOR_GAIN,								// 100	 	  

	PARAM_COMPANDER_FLAG_UL,							// 101
	PARAM_EXPANDER_ALPHA_UL,							// 102
	PARAM_EXPANDER_BETA_UL,								// 103
	PARAM_EXPANDER_UPPER_LIMIT_UL,						// 104
	PARAM_EXPANDER_C_UL,								// 105
	PARAM_COMPRESSOR_ALPHA_UL,							// 106
	PARAM_COMPRESSOR_BETA_UL,							// 107
	PARAM_COMPRESSOR_OUTPUT_GAIN_UL,					// 108
	PARAM_COMPRESSOR_GAIN_UL,							// 109	 	  
	
	PARAM_AUDIO_DSP_SIDETONE,							// 110
	PARAM_AUDIO_DL_IDLE_PGA_ADJ,						// 111
	PARAM_AUDIO_NS_UL_IDLE_ADJ,							// 112

	PARAM_SIDETONE_OUTPUT_GAIN,							// 113
	PARAM_SIDETONE_BIQUAD_SCALE_FACTOR,					// 114
	PARAM_SIDETONE_BIQUAD_SYS_GAIN,       				// 115
	
	PARAM_AUDIO_EC_DE_EMP_FILT_COEF,					// 116
	PARAM_AUDIO_EC_PRE_EMP_FILT_COEF,					// 117

	PARAM_AUDIO_HPF_ENABLE,	   							// 118
	PARAM_AUDIO_UL_HPF_COEF_B, 							// 119
	PARAM_AUDIO_UL_HPF_COEF_A, 							// 120
	PARAM_AUDIO_DL_HPF_COEF_B, 							// 121
	PARAM_AUDIO_DL_HPF_COEF_A, 							// 122

	PARAM_AUDVOC_VCFGR,									// 123

	PARAM_PCM_FILTER_ENABLE,							// 124
	PARAM_PCM_UL_BIQUAD_GAIN,							// 125
	PARAM_PCM_DL_BIQUAD_GAIN,							// 126
	PARAM_PCM_DL_FILTER,								// 127
	PARAM_PCM_UL_FILTER,								// 128
	
	PARAM_COMP_BIQUAD_GAIN,								// 129
	PARAM_COMP_BIQUAD_FILTER,							// 130
	PARAM_ECHO_SUBBAND_NLP_DISTORTION_THRESH,			// 131
	PARAM_ECHO_SUBBAND_NLP_UL_MARGIN,					// 132
	PARAM_ECHO_SUBBAND_NLP_NOISE_MARGIN,				// 133
	
	PARAM_DL_SUBBAND_COMPANDER_FLAG,					// 134

	PARAM_COMPRESS_DL_FIR1_T2LIN,						// 135
	PARAM_COMPRESS_DL_FIR1_G2T2,						// 136
	PARAM_COMPRESS_DL_FIR1_G3T3,						// 137
	PARAM_COMPRESS_DL_FIR1_G4T4,						// 138
	PARAM_COMPRESS_DL_FIR1_ALPHA,						// 139
	PARAM_COMPRESS_DL_FIR1_BETA,						// 140
	PARAM_COMPRESS_DL_FIR1_G1LIN,						// 141
	PARAM_COMPRESS_DL_FIR1_STEP2,						// 142
	PARAM_COMPRESS_DL_FIR1_STEP3,						// 143
	PARAM_COMPRESS_DL_FIR1_STEP4,						// 144
														
	PARAM_COMPRESS_DL_FIR2_T2LIN,						// 145
	PARAM_COMPRESS_DL_FIR2_G2T2,						// 146
	PARAM_COMPRESS_DL_FIR2_G3T3,						// 147
	PARAM_COMPRESS_DL_FIR2_G4T4,						// 148
	PARAM_COMPRESS_DL_FIR2_ALPHA,						// 149
	PARAM_COMPRESS_DL_FIR2_BETA,						// 150
	PARAM_COMPRESS_DL_FIR2_G1LIN,						// 151
	PARAM_COMPRESS_DL_FIR2_STEP2,						// 152
	PARAM_COMPRESS_DL_FIR2_STEP3,						// 153
	PARAM_COMPRESS_DL_FIR2_STEP4,						// 154
														
	PARAM_COMPRESS_DL_STREAM3_T2LIN,					// 155
	PARAM_COMPRESS_DL_STREAM3_G2T2,						// 156
	PARAM_COMPRESS_DL_STREAM3_G3T3,						// 157
	PARAM_COMPRESS_DL_STREAM3_G4T4,						// 158
	PARAM_COMPRESS_DL_STREAM3_ALPHA,					// 159
	PARAM_COMPRESS_DL_STREAM3_BETA,						// 160
	PARAM_COMPRESS_DL_STREAM3_G1LIN,					// 161
	PARAM_COMPRESS_DL_STREAM3_STEP2,					// 162
	PARAM_COMPRESS_DL_STREAM3_STEP3,					// 163
	PARAM_COMPRESS_DL_STREAM3_STEP4,					// 164
														
	PARAM_COMPRESS_DL_T2LIN,							// 165
	PARAM_COMPRESS_DL_G2T2,								// 166
	PARAM_COMPRESS_DL_G3T3,								// 167
	PARAM_COMPRESS_DL_G4T4,								// 168
	PARAM_COMPRESS_DL_ALPHA,							// 169
	PARAM_COMPRESS_DL_BETA,								// 170
	PARAM_COMPRESS_DL_G1LIN,							// 171
	PARAM_COMPRESS_DL_STEP2,							// 172
	PARAM_COMPRESS_DL_STEP3,							// 173
	PARAM_COMPRESS_DL_STEP4,							// 174
	
    PARAM_ECHO_SUBBAND_NLP_ENABLE,						// 175	
    
	PARAM_VOICE_VOLUME_MAX,								// 176
	PARAM_VOICE_VOLUME_INIT,							// 177
	PARAM_AUDVOC_ADAC_IIR,								// 178  (5 Biquads, total 25 coefficients.)
	PARAM_AUDVOC_AEQMODE,								// 179
	PARAM_AUDVOC_AEQPATHGAIN,							// 180
	PARAM_AUDVOC_AEQCOF,								// 181
	PARAM_AUDVOC_MIXER_IIR,								// 182

	PARAM_MPM_NIIR_COEFFICIENT,							// 183
	PARAM_MPM_GAIN_ATTACK_STEP,							// 184
	PARAM_MPM_GAIN_ATTACK_SLOPE,						// 185
	PARAM_MPM_GAIN_DECAY_SLOPE,							// 186
	PARAM_MPM_GAIN_ATTACK_THRESHOLD,					// 187
	PARAM_IHF_PROTECTION_ENABLE,						// 188
	PARAM_GE_CENTER_FREQ_1,							// 189
	PARAM_GE_CENTER_FREQ_2,							// 190
	PARAM_GE_CENTER_FREQ_3,							// 191
	PARAM_GE_CENTER_FREQ_4,							// 192
	PARAM_GE_CENTER_FREQ_5,							// 193
	PARAM_GE_BANDWIDTH_1,							// 194
	PARAM_GE_BANDWIDTH_2,							// 195
	PARAM_GE_BANDWIDTH_3,							// 196
	PARAM_GE_BANDWIDTH_4,							// 197
	PARAM_GE_BANDWIDTH_5,							// 198
	PARAM_GE_CROSSOVER_FREQ_1,						// 199
	PARAM_GE_CROSSOVER_FREQ_2,						// 200
	PARAM_GE_CROSSOVER_FREQ_3,						// 201
	PARAM_GE_CROSSOVER_FREQ_4,						// 202
   
	PARAM_SIDETONE_FILTER,							// 203	
	//parm used for Omega Voice tuning
	PARAM_OMEGA_VOICE_ENABLE				=204,	// 204	//Start at idx 204 to align ids with all chips
	PARAM_OMEGA_VOICE_THRES_DB = 205,				// 205
	PARAM_OMEGA_VOICE_MAX_GAIN_DB,					// 206
	PARAM_OMEGA_VOICE_GAIN_STEP_UP_DB256,			// 207
	PARAM_OMEGA_VOICE_GAIN_STEP_DN_DB256,			// 208
	PARAM_OMEGA_VOICE_MAX_GAIN_DB_SCALE_FACTOR,		// 209

	PARAM_VOICE_DAC_HPF_ENABLE=210,					// 210
	PARAM_VOICE_DAC_HPF_CUTOFF_FREQ,				// 211
/***
	PARAM_AUDVOC_ADAC_IIR_HPF_ENABLE=212,			// 212
	PARAM_AUDVOC_ADAC_IIR_HPF_CUTOFF_FREQ,			// 213
***/
	PARAM_OMEGA_VOICE_MAX_ALLOWED_GAIN_SPREAD_DB=214,		// 214

	PARAM_AUDVOC_MIXER_IIR_HPF_ENABLE = 215,		// 215
	PARAM_AUDVOC_MIXER_IIR_HPF_CUTOFF_FREQ = 216,	// 216
	PARAM_MPMBIQUAD_CFG = 217,						// 217

	PARAM_EXT_SPEAKER_PGA, 						// 218
	PARAM_EXT_SPEAKER_PREAMP_PGA, 				// 219

	PARAM_PCM_DL_BIQUAD_OUTPUT_GAIN = 220,				// 220
	PARAM_PCM_UL_BIQUAD_OUTPUT_GAIN,					// 221

	PARAM_ALPHA_VOICE_ENABLE =222, 				// 222 Start at idx 222 to align ids with all chips
	PARAM_ALPHA_VOICE_AVC_TARGET_DB, 			// 223
	PARAM_ALPHA_VOICE_AVC_MAX_GAIN_DB, 			// 224
	PARAM_ALPHA_VOICE_AVC_STEP_UP_DB256, 		// 225
	PARAM_ALPHA_VOICE_AVC_STEP_DOWN_DB256, 		// 226

	// KAPPA_VOICE
	PARAM_KAPPA_VOICE_ENABLE, 					// 227
	PARAM_KAPPA_VOICE_HIGH_PITCHED_TONE_TARGET_DB, 		// 228
	PARAM_KAPPA_VOICE_FAST_DETECTOR_LOUDNESS_TARGET_DB, // 229
	PARAM_KAPPA_VOICE_FAST_DETECTOR_MAX_SUPP_DB, 		// 230
	PARAM_KAPPA_VOICE_SLOW_DETECTOR_LOUDNESS_TARGET_DB, // 231
	PARAM_KAPPA_VOICE_SLOW_DETECTOR_MAX_SUPP_DB, 		// 232 

	PARAM_EXT_SPEAKER_ENABLE = 233,				// 233  (not used in Athena platform)
	PARAM_ECHO_SUBBAND_NLP_ERL_ERLE_ADJ_WB = 234,		// 234  (not used in Athena platform)
	PARAM_ECHO_SUBBAND_NLP_UL_MARGIN_WB,				// 235  (not used in Athena platform)
	PARAM_ECHO_SUBBAND_NLP_NOISE_MARGIN_WB,				// 236  (not used in Athena platform)

	PARAM_VOICE_DUALMIC_ENABLE = 237,					// 237
	PARAM_VOICE_ADC_D1,									// 238
	PARAM_VOICE_ADC_D2,									// 239
	PARAM_VOICE_DIGMIC_SAMPLE_SEL,						// 240

	PARAM_ECHO_SUBBAND_NLP_DT_FINE_CONTROL,             // 241

	PARAM_VOICE_DUALMIC_ANC_ENABLE=242,					// 242
	PARAM_VOICE_DUALMIC_NLP_ENABLE,						// 243
	PARAM_VOICE_DUALMIC_PRE_WNR_ENABLE,					// 244
	PARAM_VOICE_DUALMIC_POST_WNR_ENABLE,				// 245
	PARAM_VOICE_DUALMIC_NS_LEVEL,						// 246

	PARAM_ECHO_CANCEL_MIC2_INPUT_GAIN=247,			   	// 247
	PARAM_ECHO_CANCEL_MIC2_OUTPUT_GAIN,					// 248
	PARAM_ECHO_MIC2_FEED_FORWARD_GAIN,					// 249

	PARAM_NLP_DISTORTION_COUPLING = 250,				// 250

	PARAM_DSP_VOICE_VOLUME_TABLE = 291,					// 291
	PARAM_FM_RADIO_DIGITAL_VOLUME = 292,					// 292
	PARAM_REVERB_TIME_CONSTANT 	  = 293,				// 293
	PARAM_REVERB_LEVEL 			  = 294,				// 294
	PARAM_ECHO_PATH_DELAY 		  = 295,				// 295
	PARAM_ECHO_PATH_SPAN 		  = 296,				// 296
	PARAM_DETECTION_THRESHOLD 	  = 297,				// 297

	AUDIO_PARM_NUMBER   								// 298
 	
} AudioParam_t;


#define AUDIO_FIFO_LENGTH_MS	480		//in ms, the ring buffers in Audio Router and under use this size.

#define AUDIO_SPKR_CH_Mode_t_DEFINED
typedef enum
{
	AUDIO_SPKR_CHANNEL_DIFF,	
	AUDIO_SPKR_CHANNEL_3WIRE_1,
	AUDIO_SPKR_CHANNEL_3WIRE_2,
	AUDIO_SPKR_CHANNEL_3WIRE_3,
	AUDIO_SPKR_CHANNEL_3WIRE_4,
	AUDIO_SPKR_CHANNEL_INVALID
} AUDIO_SPKR_CH_Mode_t;

#endif

