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

#include "io_map.h"

/**
*
* @addtogroup AudioDriverGroup
* @{
*/

#ifdef __cplusplus
extern "C" {
#endif

// move from auddrv_def.h

/**
* Globale Constants
******************************************************************************/
#define AUDIOH_BASE_ADDR1            KONA_AUDIOH_BASE_VA /* brcm_rdb_cph_cfifo.h */
#define SDT_BASE_ADDR1            KONA_SDT_BASE_VA /* brcm_rdb_cph_cfifo.h */
#define SRCMIXER_BASE_ADDR1          KONA_SRCMIXER_BASE_VA /* brcm_rdb_srcmixer.h */
#define CFIFO_BASE_ADDR1             KONA_CFIFO_BASE_VA /* brcm_rdb_cph_cfifo.h */
#define AADMAC_BASE_ADDR1            KONA_AADMAC_BASE_VA /* brcm_rdb_cph_aadmac.h */
#define SSASW_BASE_ADDR1             KONA_SSASW_BASE_VA /* brcm_rdb_cph_ssasw.h */
#define AHINTC_BASE_ADDR1            KONA_AHINTC_BASE_VA /* brcm_rdb_ahintc.h */	
#define SSP4_BASE_ADDR1            KONA_SSP4_BASE_VA /* brcm_rdb_sspil.h */
#define SSP3_BASE_ADDR1            KONA_SSP3_BASE_VA /* brcm_rdb_sspil.h */


// the following 5 typedefs used to be in drv_caph_hwctrl.h	
#include "audio_consts.h"

typedef AudioMode_t (*CB_GetAudioMode_t)( void );
typedef void (*CB_SetAudioMode_t) ( AudioMode_t  audio_mode);
typedef void (*CB_SetMusicMode_t) ( AudioMode_t  audio_mode);

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
#define MIC_NOISE_CANCEL CSL_CAPH_DEV_EANC_DIGI_MIC_R



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

#define audio_xassert(a, b) (Log_DebugPrintf(LOGID_AUDIO, "%s assert line %d, %d, 0x%lx.\r\n", __FUNCTION__, __LINE__, a, (UInt32)b))

//#define audio_xassert(a, b) (Log_DebugPrintf(LOGID_AUDIO, "%s assert line %d, %d, 0x%x.\r\n", __FUNCTION__, __LINE__, a, b))

// move from drv_caph.h

#include "csl_caph.h"
/**
* CAPH Path ID
******************************************************************************/
typedef UInt8 AUDDRV_PathID;

/**
* CAPH HW path configuration parameters
******************************************************************************/
typedef struct
{
    AUDDRV_STREAM_e streamID;
    AUDDRV_PathID pathID;
    AUDDRV_DEVICE_e source;
    AUDDRV_DEVICE_e sink;
    CSL_CAPH_DMA_CHNL_e dmaCH;    
    AUDIO_SAMPLING_RATE_t src_sampleRate;
    AUDIO_SAMPLING_RATE_t snk_sampleRate;	
    AUDIO_CHANNEL_NUM_t chnlNum;
    AUDIO_BITS_PER_SAMPLE_t bitPerSample;
    CSL_CAPH_SRCM_MIX_GAIN_t mixGain;        
}AUDDRV_HWCTRL_CONFIG_t;
/**
* CAPH HW filters
******************************************************************************/
typedef enum
{
    AUDDRV_EANC_FILTER1, 
    AUDDRV_EANC_FILTER2, 
    AUDDRV_SIDETONE_FILTER, 
}AUDDRV_HWCTRL_FILTER_e;


/**
* CAPH Audio Stream Buffer done Callback function
******************************************************************************/
typedef Boolean (*AUDDRV_BufDoneCB_t)(UInt8  *pBuf,  UInt32 nSize,
                                      AUDDRV_STREAM_e streamID);

/**
* CAPH Audio Information for Render/Capture Driver
******************************************************************************/
typedef struct
{
    CSL_CAPH_CFIFO_FIFO_e fifo;
}AUDDRV_HWCTRL_INFO_t;

// end of auddrv_def.h

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

//typedef AudioMode_t (*CB_GetAudioMode_t)( void );

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
//typedef void (*CB_SetAudioMode_t) ( AudioMode_t  audio_mode);
//typedef void (*CB_SetMusicMode_t) ( AudioMode_t  audio_mode);
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
