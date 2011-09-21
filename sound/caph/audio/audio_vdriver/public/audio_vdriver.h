/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2011  Broadcom Corporation                                                        */
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
*   @file   audio_vdriver.h
*
*   @brief  common APIs for audio
*
****************************************************************************/
/**
*
* @defgroup Audio    Audio Component
*
* @brief    This group defines the common APIs for audio virtual driver
*
* @ingroup  Audio Component
*****************************************************************************/

/**
*
*  @file  audio_vdriver.h
*
*  @brief Audio Virtual Driver API
*
*  @note
*****************************************************************************/

#ifndef	__AUDIO_VDRIVER_H__
#define	__AUDIO_VDRIVER_H__


//richlu
#include "csl_caph_hwctrl.h"

/**
*
* @addtogroup Audio
* @{
*/

#ifdef __cplusplus
extern "C" {
#endif

//Define the other mic which is used for Noise Cancellation.
//It is product-dependent.
#define MIC_NOISE_CANCEL CSL_CAPH_DEV_EANC_DIGI_MIC_R

typedef enum {
	//AP->CP->DSP cmd to reuse the enum in dspcmd.h
	//AP->CP cmd to be listed here including filter loading.

	/**
	AUDDRV_DSP_FASTCMD,
	AUDDRV_DSP_VPUCMD,
	**/

	//CP:
	AUDDRV_CPCMD_SetOmegaVoiceParam,
	AUDDRV_CPCMD_SetFilter,
	AUDDRV_CPCMD_SetBasebandVolume,
	AUDDRV_CPCMD_SetBasebandDownlinkMute,
	AUDDRV_CPCMD_SetBasebandDownlinkUnmute,
	AUDDRV_CPCMD_SetBasebandUplinkGain,
	AUDDRV_CPCMD_SetBasebandDownlinkGain,
	AUDDRV_CPCMD_SET_DSP_AMR_VOLUME_UL,
	AUDDRV_CPCMD_SET_DSP_AMR_VOLUME_DL,
	AUDDRV_CPCMD_SetAudioMode,
	AUDDRV_CPCMD_READ_AUDVOC_AEQMODE,
	AUDDRV_CPCMD_WRITE_AUDVOC_AEQMODE,
	AUDDRV_CPCMD_GET_CP_AUDIO_MODE,
	AUDDRV_CPCMD_UPDATE_AUDIO_MODE,
	AUDDRV_CPCMD_ENABLE_DSP_DTX,
	AUDDRV_CPCMD_SetULSpeechRecordGain,
	AUDDRV_CPCMD_PassAudioMode
} AUDDRV_CP_CMD_en_t;

typedef enum AUDDRV_REQUEST_ID_t
{
    AUDDRV_RATE_CHANGE_REQ,     //0x00
	AUDDRV_REQUEST_ID_TOTAL_COUNT
} AUDDRV_REQUEST_ID_t;

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

/**
* CAPH HW filters
******************************************************************************/
typedef enum
{
    AUDDRV_EANC_FILTER1, 
    AUDDRV_EANC_FILTER2, 
    AUDDRV_SIDETONE_FILTER, 
}AUDDRV_HWCTRL_FILTER_e;


typedef enum {
	AUDDRV_MIC1 = 0x1,
	AUDDRV_MIC2 = 0x2,
	AUDDRV_SPEAKER = 0x4
} AUDDRV_DSPFILTER_DEVICE_Enum_t;

// audio controller request message structure
typedef struct AUDDRV_REQUEST_MSG_t
{
    AUDDRV_REQUEST_ID_t     reqID;
    UInt32                  param1;
    UInt32                  param2;
    UInt32                  param3;
} AUDDRV_REQUEST_MSG_t;

typedef enum {
	AUDDRV_USER_GET_SPKPROT,
	AUDDRV_USER_ENA_SPKPROT,
	AUDDRV_USER_SET_EQ,
} AudioDrvUserParam_t;

typedef struct AUDDRV_PathID_t{
	CSL_CAPH_PathID ulPathID;
	CSL_CAPH_PathID ul2PathID;
	CSL_CAPH_PathID dlPathID;
} AUDDRV_PathID_t;

#ifndef MAX_NO_OF_BIQUADS
#define MAX_NO_OF_BIQUADS	12
#endif

#define EQ_COEFF_FW_NUM		3
#define EQ_COEFF_BW_NUM		2

typedef struct 
{
	Int32  dl_coef_fw_8k[MAX_NO_OF_BIQUADS][EQ_COEFF_FW_NUM];			
	Int32  dl_coef_bw_8k[MAX_NO_OF_BIQUADS][EQ_COEFF_BW_NUM];			
	Int16  dl_comp_filter_gain_8k[MAX_NO_OF_BIQUADS];		
	Int32  dl_output_bit_select_8k;
	Int32  dl_coef_fw_16k[MAX_NO_OF_BIQUADS][EQ_COEFF_FW_NUM];			
	Int32  dl_coef_bw_16k[MAX_NO_OF_BIQUADS][EQ_COEFF_BW_NUM];			
	Int16  dl_comp_filter_gain_16k[MAX_NO_OF_BIQUADS];	
	Int32  dl_output_bit_select_16k;		
	UInt16 dl_nstage_filter;
} EQDlCompfilter_t;

typedef struct 
{
	Int32  ul_coef_fw_8k[MAX_NO_OF_BIQUADS][EQ_COEFF_FW_NUM];		
	Int32  ul_coef_bw_8k[MAX_NO_OF_BIQUADS][EQ_COEFF_BW_NUM];		
	Int16  ul_comp_filter_gain_8k[MAX_NO_OF_BIQUADS];
	Int32  ul_output_bit_select_8k;	
	Int32  ul_coef_fw_16k[MAX_NO_OF_BIQUADS][EQ_COEFF_FW_NUM];				
	Int32  ul_coef_bw_16k[MAX_NO_OF_BIQUADS][EQ_COEFF_BW_NUM];				
	Int16  ul_comp_filter_gain_16k[MAX_NO_OF_BIQUADS];		
	Int32  ul_output_bit_select_16k;			
	UInt16 ul_nstage_filter;	
} EQUlCompfilter_t;

typedef struct 
{
	EQDlCompfilter_t dl;
	EQUlCompfilter_t ul;
} AudioCompfilter_t;

UInt32 audio_control_generic(
				UInt32 param1,
				UInt32 param2,
				UInt32 param3,
				UInt32 param4,
				UInt32 param5,
				UInt32 param6);

UInt32 audio_control_dsp(
				UInt32 param1,
				UInt32 param2,
				UInt32 param3,
				UInt32 param4,
				UInt32 param5,
				UInt32 param6);


UInt32 audio_cmf_filter(AudioCompfilter_t* cf);

// Description:   Inititialize audio driver
//
void AUDDRV_Init( void );

// shut down audio driver
void AUDDRV_Shutdown( void );

//  the control sequence for starting telephony audio.
void AUDDRV_Telephony_Init (AUDDRV_MIC_Enum_t   mic,
				AUDDRV_SPKR_Enum_t  speaker,
				void * pData
				);

// the control sequence for ratechange of voice call.
void AUDDRV_Telephony_RateChange ( UInt32 sampleRate );

UInt32 AUDDRV_Telephone_GetSampleRate( void );

// rate change request function
void AUDDRV_RequestRateChange(UInt8 codecID);

// the control sequence for ending telephony audio.
//this func let DSP to turn off voice path, if need to resume apps operation on voice, controller needs to reenable voice path after phonce call ends.
void AUDDRV_Telephony_Deinit (void *pData);


void AUDDRV_Telephony_SelectMicSpkr  (
				AUDDRV_MIC_Enum_t   mic,
				AUDDRV_SPKR_Enum_t  speaker,
				void *pData);
// Enable DSP output processing.
void AUDDRV_EnableDSPOutput (
				AUDDRV_SPKR_Enum_t      mixer_speaker_selection,
				AUDIO_SAMPLING_RATE_t   sample_rate
				);
// Enable DSP input processing.
void AUDDRV_EnableDSPInput (
				AUDDRV_MIC_Enum_t		 mic_selection,
				AUDIO_SAMPLING_RATE_t	 sample_rate
				);

Boolean AUDDRV_IsVoiceCallWB(AudioMode_t audio_mode);
Boolean AUDDRV_IsCall16K(AudioMode_t voiceMode);
Boolean AUDDRV_InVoiceCall( void );

#if defined(USE_NEW_AUDIO_PARAM)
AudioApp_t AUDDRV_GetAudioApp( void );
//void AUDDRV_SaveAudioApp( AudioApp_t audio_app );
void AUDDRV_SaveAudioMode( AudioMode_t audio_mode, AudioApp_t audio_app);
void AUDDRV_SetAudioMode( AudioMode_t audio_mode, AudioApp_t audio_app);
void AUDDRV_SetMusicMode ( AudioMode_t  audio_mode, AudioApp_t audio_app);
#else
void AUDDRV_SaveAudioMode( AudioMode_t audio_mode );
void AUDDRV_SetAudioMode ( AudioMode_t  audio_mode, UInt32 dev);
void AUDDRV_SetMusicMode ( AudioMode_t  audio_mode);
#endif

AudioMode_t AUDDRV_GetAudioMode( void );
UInt32 AUDDRV_GetAudioDev( void );

void AUDDRV_SetVCflag( Boolean inVoiceCall );
Boolean AUDDRV_GetVCflag( void );

void AUDDRV_User_CtrlDSP (
				AudioDrvUserParam_t	audioDrvUserParam,
				void			*user_CB,
				UInt32			param1,
				UInt32			param2
				);

void AUDDRV_User_HandleDSPInt ( UInt32 param1, UInt32 param2, UInt32 param3 );
void AUDDRV_SetPCMOnOff(Boolean	on_off);

void AUDDRV_Telephony_InitHW (AUDDRV_MIC_Enum_t mic,
			  AUDDRV_SPKR_Enum_t speaker,
			  AUDIO_SAMPLING_RATE_t	sample_rate,
			  void * pData);

void AUDDRV_Telephony_DeinitHW(void *pData);

void AUDDRV_ControlFlagFor_CustomGain( Boolean on_off );
#ifdef CONFIG_DEPENDENCY_READY_SYSPARM 
void AUDDRV_SetDSPFilter( AudioMode_t audio_mode, 
		UInt32 dev, 
		SysAudioParm_t* pAudioParm);
void AUDDRV_SetHWSidetoneFilter(AudioMode_t audio_mode, 
		SysAudioParm_t* pAudioParm);
#endif		
void AUDDRV_SetHWGain(CSL_CAPH_HW_GAIN_e hw, 
		UInt32 gain); 
void AUDDRV_Telephony_UnmuteMic (AUDDRV_MIC_Enum_t mic,
					void *pData);
void AUDDRV_Telephony_MuteMic (AUDDRV_MIC_Enum_t mic,
					void *pData);
void AUDDRV_Telephony_MuteSpkr (AUDDRV_SPKR_Enum_t speaker,
					void *pData);
void AUDDRV_Telephony_UnmuteSpkr (AUDDRV_SPKR_Enum_t speaker,
					void *pData);
void AUDDRV_SetULSpeechRecordGain(Int16 gain);
Boolean AUDDRV_IsDualMicEnabled(void);



// move from drv_audio_commom.h
/**
*
*  @brief  Get the audio csl Device from the Driver device 
*
*  @param  AUDDRV_DEVICE_e (in) Driver layer device
*
*  @return CSL_CAPH_DEVICE_e CSL layer device
*****************************************************************************/
CSL_AUDIO_DEVICE_e AUDDRV_GetCSLDevice (AUDDRV_DEVICE_e dev);



/**
*
*  @brief  Get the audio driver device from the microphone selection
*
*  @param  AUDDRV_MIC_Enum_t (in) Driver microphone selection
*
*  @return CSL_CAPH_DEVICE_e Driver device
*****************************************************************************/
CSL_CAPH_DEVICE_e AUDDRV_GetDRVDeviceFromMic (AUDDRV_MIC_Enum_t mic);

/**
*
*  @brief  Get the audio driver device from the speaker selection
*
*  @param  AUDDRV_SPKR_Enum_t (in) Driver speaker selection
*
*  @return CSL_CAPH_DEVICE_e Driver device
*****************************************************************************/
CSL_CAPH_DEVICE_e AUDDRV_GetDRVDeviceFromSpkr (AUDDRV_SPKR_Enum_t spkr);


/**
*
*  @brief  Get the DSP UL gain
*
*  @param  CSL_CAPH_DEVICE_e (in) Driver microphone selection
*
*  @return DSP gain in Q1.14
*****************************************************************************/
Int16 AUDDRV_GetDSPULGain(CSL_CAPH_DEVICE_e mic, Int16 gain);


/**
*
*  @brief  Get the DSP DL gain
*
*  @param  CSL_CAPH_DEVICE_e (in) Driver speaker selection
*
*  @return DSP gain in mdB in Q15
*****************************************************************************/
Int16 AUDDRV_GetDSPDLGain(CSL_CAPH_DEVICE_e spkr, Int16 gain);



/**
*
*  @brief  Get the Mixer input gain
*
*  @param  gain (in) Gain in Q13.2 
*
*  @return HW gain (out)  mixer input gain -- register value.
*****************************************************************************/
UInt16 AUDDRV_GetMixerInputGain(Int16 gain);




/**
*
*  @brief  Get the Mixer output fine gain
*
*  @param  gain (in) Gain in Q13.2 
*
*  @return HW gain (out)  mixer output fine gain -- register value.
*****************************************************************************/
UInt16 AUDDRV_GetMixerOutputFineGain(Int16 gain);


/**
*
*  @brief  Get the Mixer output coarse gain
*
*  @param  gain (in) Gain in Q13.2 
*
*  @return HW gain (out)  mixer output coarse gain -- register value.
*****************************************************************************/
UInt16 AUDDRV_GetMixerOutputCoarseGain(Int16 gain);


/**
*
*  @brief  Get the HW DL gain
*
*  @param  spkr (in) Driver speaker selection
*  @param  gain (in) Gain in Q1.14 
*
*  @return HW gain
*****************************************************************************/
Int16 AUDDRV_GetHWDLGain(CSL_CAPH_DEVICE_e spkr, Int16 gain);


/**
*
*  @brief  Get the PMU gain
*
*  @param  CSL_CAPH_DEVICE_e (in) Driver speaker selection
*
*  @return PMU gain
*****************************************************************************/
UInt16 AUDDRV_GetPMUGain(CSL_CAPH_DEVICE_e spkr, Int16 gain);



/**
*
*  @brief  Get the PMU gain
*
*  @param  CSL_CAPH_DEVICE_e (in) Driver speaker selection
*
*  @return PMU gain
*****************************************************************************/
UInt16 AUDDRV_GetPMUGain_Q1_14(CSL_CAPH_DEVICE_e spkr, Int16 gain);



#ifdef __cplusplus
}
#endif

#endif // __AUDIO_VDRIVER_H__
