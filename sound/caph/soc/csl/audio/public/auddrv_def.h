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
*   @file   auddrv_def.h
*
*   @brief  This file contains the auddrv definitions for driver layer
*
****************************************************************************/

#ifndef _AUDDRV_DEF_
#define _AUDDRV_DEF_


typedef enum {
	AUDDRV_MIC1 = 0x1,
	AUDDRV_MIC2 = 0x2,
	AUDDRV_SPEAKER = 0x4
} AUDDRV_DSPFILTER_DEVICE_Enum_t;

typedef enum {
    AUDDRV_MIC_NONE,

#ifdef _HERA_
    AUDDRV_MIC_AMIC    = 0,	    // Analog MIC/Handset   
	AUDDRV_MIC_DMIC1   = 1,	    // DMIC 1
    AUDDRV_MIC_DMIC2   = 2,	    // DMIC 2
    AUDDRV_MIC_DMIC3   = 4,	    // DMIC 3 
    AUDDRV_MIC_DMIC4   = 8,	    // DMIC 4 
    AUDDRV_MIC_AUXMIC  = 0x10,	// Analog MIC/Headset
#endif

    AUDDRV_MIC_ANALOG_MAIN,
    AUDDRV_MIC_ANALOG_AUX,
    AUDDRV_MIC_DIGI1,  // DMIC1
    AUDDRV_MIC_DIGI2,  // DMIC2
    AUDDRV_MIC_DIGI3,
    AUDDRV_MIC_DIGI4,

	AUDDRV_DUAL_MIC_DIGI12,  // DMIC1 and DMIC2
	AUDDRV_DUAL_MIC_DIGI21,  // DMIC1 and DMIC2
	AUDDRV_DUAL_MIC_ANALOG_DIGI1,
	AUDDRV_DUAL_MIC_DIGI1_ANALOG,

    AUDDRV_MIC_SPEECH_DIGI,
    AUDDRV_MIC_EANC_DIGI,

	AUDDRV_MIC_PCM_IF,
    AUDDRV_MIC_USB_IF,
    AUDDRV_MIC_NOISE_CANCEL,
	AUDDRV_MIC_TOTAL_NUM
} AUDDRV_MIC_Enum_t;

typedef enum {

    AUDDRV_VIBRATOR_BYPASS_MODE,
    AUDDRV_VIBRATOR_WAVE_MODE

} AUDDRV_VIBRATOR_MODE_Enum_t;

typedef enum
{
	AUDDRV_SPKR_NONE,
    AUDDRV_SPKR_EP,
    AUDDRV_SPKR_EP_STEREO,
    AUDDRV_SPKR_IHF,
    AUDDRV_SPKR_IHF_STEREO,
    AUDDRV_SPKR_HS,
	AUDDRV_SPKR_HS_LEFT,
	AUDDRV_SPKR_HS_RIGHT,
	AUDDRV_SPKR_PCM_IF,
    AUDDRV_SPKR_VIBRA,
	AUDDRV_SPKR_USB_IF,
	AUDDRV_SPKR_TOTAL_NUM
} AUDDRV_SPKR_Enum_t;


typedef enum {
	//For Athena
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
	AUDDRV_Mixer4_MPMALDCENABLE,

	//For Rhea and Samoa
	AUDDRV_Voice_Mic1_EQ,
	AUDDRV_Voice_Mic2_EQ,
	AUDDRV_Voice_Speaker_EQ,
	AUDDRV_HW_Sidetone_EQ

} AUDDRV_Filter_Enum_t;

//for athena:
//#define AUDDRV_MIXER1_SPKR_1L AUDDRV_SPKR_IHF
//#define AUDDRV_MIXER2_SPKR_1R AUDDRV_SPKR_EP
//#define AUDDRV_MIXER12_SPKR_1LR AUDDRV_SPKR_IHF_STEREO
//#define AUDDRV_MIXER3_SPKR_2L AUDDRV_SPKR_HS_LEFT
//#define AUDDRV_MIXER4_SPKR_2R AUDDRV_SPKR_HS_RIGHT
//#define AUDDRV_MIXER34_SPKR_2LR AUDDRV_SPKR_HS



/**
* Devices
******************************************************************************/
typedef enum
{
	AUDDRV_DEV_NONE,

	AUDDRV_RENDER_DEV_AUDIO, /* Athena AUDIO path */
	AUDDRV_RENDER_DEV_POLYRINGER, /*Athena POLY path */
	AUDDRV_CAPTURE_DEV_AUDIO,
	AUDDRV_CAPTURE_DEV_BTW,

	AUDDRV_DEV_EP,  /*Earpiece*/
	AUDDRV_DEV_HS,  /*Headset speaker*/
	AUDDRV_DEV_IHF,  /*IHF speaker*/
	AUDDRV_DEV_VIBRA,  /*Vibra output*/
	AUDDRV_DEV_FM_TX,  /*FM TX broadcaster*/
	AUDDRV_DEV_BT_SPKR,  /*Bluetooth headset speaker*/
	AUDDRV_DEV_DSP,  /*DSP direct connection with SRCMixer in voice call*/
	AUDDRV_DEV_DIGI_MIC,  /*Two Digital microphones*/
//	AUDDRV_DEV_DIGI_MIC_L = AUDDRV_DEV_DIGI_MIC,  /*Digital microphone: L-channel*/
	AUDDRV_DEV_DIGI_MIC_L,  /*Digital microphone: L-channel*/
	AUDDRV_DEV_DIGI_MIC_R,  /*Digital microphone: R-channel*/
	AUDDRV_DEV_EANC_DIGI_MIC,  /*Two Noise Digital microphones for EANC control*/
	AUDDRV_DEV_EANC_DIGI_MIC_L,  /*ENAC Digital microphone: L-channel*/
	AUDDRV_DEV_EANC_DIGI_MIC_R,  /*ENAC Digital microphone: R-channel*/
	AUDDRV_DEV_SIDETONE_INPUT,  /*Sidetone path input*/
	AUDDRV_DEV_EANC_INPUT,  /*EANC Anti-noise input*/
	AUDDRV_DEV_ANALOG_MIC,  /*Phone analog mic*/
	AUDDRV_DEV_HS_MIC,  /*Headset mic*/
	AUDDRV_DEV_BT_MIC,  /*Bluetooth headset mic*/
	AUDDRV_DEV_FM_RADIO,  /*FM Radio playback*/
	AUDDRV_DEV_MEMORY,  /*DDR memory*/
	AUDDRV_DEV_SRCM,  /*SRCMixer*/
	AUDDRV_DEV_DSP_throughMEM,  /*DSP connection through Shared mem*/
}AUDDRV_DEVICE_e;



//Define the other mic which is used for Noise Cancellation.
//It is product-dependent.
#define MIC_NOISE_CANCEL AUDDRV_DEV_EANC_DIGI_MIC_R



/**
* CAPH HW gain. For tuning purpose only
******************************************************************************/
typedef enum
{
	AUDDRV_AMIC_PGA_GAIN,
	AUDDRV_AMIC_DGA_COARSE_GAIN,
	AUDDRV_AMIC_DGA_FINE_GAIN,
	AUDDRV_DMIC1_DGA_COARSE_GAIN,
	AUDDRV_DMIC1_DGA_FINE_GAIN,
	AUDDRV_DMIC2_DGA_COARSE_GAIN,
	AUDDRV_DMIC2_DGA_FINE_GAIN,
	AUDDRV_DMIC3_DGA_COARSE_GAIN,
	AUDDRV_DMIC3_DGA_FINE_GAIN,
	AUDDRV_DMIC4_DGA_COARSE_GAIN,
	AUDDRV_DMIC4_DGA_FINE_GAIN,
	AUDDRV_SRCM_INPUT_GAIN_L,
	AUDDRV_SRCM_OUTPUT_COARSE_GAIN_L,
	AUDDRV_SRCM_OUTPUT_FINE_GAIN_L,
	AUDDRV_SRCM_INPUT_GAIN_R,
	AUDDRV_SRCM_OUTPUT_COARSE_GAIN_R,
	AUDDRV_SRCM_OUTPUT_FINE_GAIN_R,
} AUDDRV_HW_GAIN_e;

#if defined (_ATHENA_)

/**
* AUDVOC Audio Stream ID and devs
******************************************************************************/
typedef enum
{
	AUDDRV_STREAM_NONE,
	AUDDRV_STREAM1,
	AUDDRV_STREAM2,
	AUDDRV_STREAM3,
	AUDDRV_STREAM4,
	AUDDRV_STREAM_TOTAL,
} AUDDRV_STREAM_e;
#if 0
typedef enum
{
	AUDDRV_DEV_NONE,
	AUDDRV_RENDER_DEV_AUDIO, /* Athena AUDIO path */
	AUDDRV_RENDER_DEV_POLYRINGER, /*Athena POLY path */
	AUDDRV_CAPTURE_DEV_AUDIO,
	AUDDRV_CAPTURE_DEV_BTW,
}AUDDRV_DEVICE_e;
#endif
#elif defined (_RHEA_) || defined (_SAMOA_)

/**
* CAPH Audio Stream ID and devs
******************************************************************************/
typedef enum
{
	AUDDRV_STREAM_NONE,
	AUDDRV_STREAM1,
	AUDDRV_STREAM2,
	AUDDRV_STREAM3,
	AUDDRV_STREAM4,
	AUDDRV_STREAM5,
	AUDDRV_STREAM6,
	AUDDRV_STREAM7,
	AUDDRV_STREAM8,
	AUDDRV_STREAM9,
	AUDDRV_STREAM10,
	AUDDRV_STREAM11,
	AUDDRV_STREAM12,
	AUDDRV_STREAM13,
	AUDDRV_STREAM14,
	AUDDRV_STREAM15,
	AUDDRV_STREAM16,
	AUDDRV_STREAM_TOTAL,
} AUDDRV_STREAM_e;
#if 0
/**
* CAPH Devices
******************************************************************************/
typedef enum
{
	AUDDRV_DEV_NONE,
	AUDDRV_DEV_EP,  /*Earpiece*/
	AUDDRV_DEV_HS,  /*Headset speaker*/
	AUDDRV_DEV_IHF,  /*IHF speaker*/
	AUDDRV_DEV_VIBRA,  /*Vibra output*/
	AUDDRV_DEV_FM_TX,  /*FM TX broadcaster*/
	AUDDRV_DEV_BT_SPKR,  /*Bluetooth headset speaker*/
	AUDDRV_DEV_DSP,  /*DSP direct connection with SRCMixer in voice call*/
	AUDDRV_DEV_DIGI_MIC,  /*Two Digital microphones*/
	AUDDRV_DEV_DIGI_MIC_L,  /*Digital microphone: L-channel*/
	AUDDRV_DEV_DIGI_MIC_R,  /*Digital microphone: R-channel*/
	AUDDRV_DEV_EANC_DIGI_MIC,  /*Two Noise Digital microphones for EANC control*/
	AUDDRV_DEV_SIDETONE_INPUT,  /*Sidetone path input*/
	AUDDRV_DEV_EANC_INPUT,  /*EANC Anti-noise input*/
	AUDDRV_DEV_ANALOG_MIC,  /*Phone analog mic*/
	AUDDRV_DEV_HS_MIC,  /*Headset mic*/
	AUDDRV_DEV_BT_MIC,  /*Bluetooth headset mic*/
	AUDDRV_DEV_FM_RADIO,  /*FM Radio playback*/
	AUDDRV_DEV_MEMORY,  /*DDR memory*/
	AUDDRV_DEV_SRCM,  /*SRCMixer*/
}AUDDRV_DEVICE_e;
#endif
#endif

/**
* AUDVOC/CAPH Render and capture driver task states
******************************************************************************/
typedef enum {
	AUDDRV_STATE_DEINIT,
	AUDDRV_STATE_INIT,     //task and OS queue are created.
	AUDDRV_STATE_CONFIGURED,
	AUDDRV_STATE_STARTING,
	AUDDRV_STATE_STARTED,
	AUDDRV_STATE_PAUSING,
	AUDDRV_STATE_PAUSED,
	AUDDRV_STATE_STOPPING,
	AUDDRV_STATE_STOPPED
} AUDDRV_STATE_Enum_t;

#define audio_xassert(a, b) Log_DebugPrintf(LOGID_AUDIO, "%s assert line %d, %d, 0x%lx.\r\n", __FUNCTION__, __LINE__, a, (UInt32)b)

#endif // _AUDDRV_DEF_

