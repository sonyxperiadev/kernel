/****************************************************************************
Copyright 2009 - 2011  Broadcom Corporation
 Unless you and Broadcom execute a separate written software license agreement
 governing use of this software, this software is licensed to you under the
 terms of the GNU General Public License version 2 (the GPL), available at
	http://www.broadcom.com/licenses/GPLv2.php

 with the following added to such license:
 As a special exception, the copyright holders of this software give you
 permission to link this software with independent modules, and to copy and
 distribute the resulting executable under terms of your choice, provided
 that you also meet, for each linked independent module, the terms and
 conditions of the license of that module.
 An independent module is a module which is not derived from this software.
 The special exception does not apply to any modifications of the software.
 Notwithstanding the above, under no circumstances may you combine this software
 in any way with any other Broadcom software provided under a license other than
 the GPL, without Broadcom's express prior written consent.
***************************************************************************/
/**
*
* @defgroup Audio    Audio Component
*
* @brief    This group defines the APIs for audio driver
*
* @ingroup  Audio Component
*****************************************************************************/

/**
*
*  @file  audio_vdriver.h
*
*  @brief Audio Driver API
*
*  @note
*****************************************************************************/

#ifndef	__AUDIO_VDRIVER_H__
#define	__AUDIO_VDRIVER_H__

#include "csl_caph_hwctrl.h"

/**
*
* @addtogroup Audio
* @{
*/

typedef void (*audio_codecId_handler_t) (int codecId);

/* Define the other mic which is used for Noise Cancellation.
	It is product-dependent. */
#define MIC_NOISE_CANCEL CSL_CAPH_DEV_EANC_DIGI_MIC_R

enum _AUDDRV_CP_CMD_en_t {
		/**
		AP->CP->DSP cmd to reuse the enum in dspcmd.h
		AP->CP cmd to be listed here including filter loading.
		AUDDRV_DSP_FASTCMD,
		AUDDRV_DSP_VPUCMD,
		**/

		/* CP: */
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
};
#define AUDDRV_CP_CMD_en_t enum _AUDDRV_CP_CMD_en_t


enum _AUDDRV_REQUEST_ID_t {
		AUDDRV_RATE_CHANGE_REQ,	/* 0x00 */
		AUDDRV_REQUEST_ID_TOTAL_COUNT
};
#define AUDDRV_REQUEST_ID_t enum _AUDDRV_REQUEST_ID_t


/**
* CAPH HW filters
***********************************************************************/
enum _AUDDRV_HWCTRL_FILTER_e {
		AUDDRV_EANC_FILTER1,
		AUDDRV_EANC_FILTER2,
		AUDDRV_SIDETONE_FILTER,
};
#define AUDDRV_HWCTRL_FILTER_e enum _AUDDRV_HWCTRL_FILTER_e


/* audio controller request message structure */
struct _AUDDRV_REQUEST_MSG_t {
		AUDDRV_REQUEST_ID_t reqID;
		UInt32 param1;
		UInt32 param2;
		UInt32 param3;
};
#define AUDDRV_REQUEST_MSG_t struct _ AUDDRV_REQUEST_MSG_t

enum _AudioDrvUserCtrl_t {
		AUDDRV_USER_NONE,
		AUDDRV_USER_SP_CTRL,
		AUDDRV_USER_SP_QUERY,
		AUDDRV_USER_SP_VAR,
		AUDDRV_USER_EQ_CTRL,
};
#define AudioDrvUserCtrl_t enum _AudioDrvUserCtrl_t


#ifndef MAX_NO_OF_BIQUADS
#define MAX_NO_OF_BIQUADS	12
#endif

#define EQ_COEFF_FW_NUM		3
#define EQ_COEFF_BW_NUM		2

struct _EQDlCompfilter_t {
		Int32 dl_coef_fw_8k[MAX_NO_OF_BIQUADS][EQ_COEFF_FW_NUM];
		Int32 dl_coef_bw_8k[MAX_NO_OF_BIQUADS][EQ_COEFF_BW_NUM];
		Int16 dl_comp_filter_gain_8k[MAX_NO_OF_BIQUADS];
		Int32 dl_output_bit_select_8k;
		Int32 dl_coef_fw_16k[MAX_NO_OF_BIQUADS][EQ_COEFF_FW_NUM];
		Int32 dl_coef_bw_16k[MAX_NO_OF_BIQUADS][EQ_COEFF_BW_NUM];
		Int16 dl_comp_filter_gain_16k[MAX_NO_OF_BIQUADS];
		Int32 dl_output_bit_select_16k;
		UInt16 dl_nstage_filter;
		UInt16 reserved;
};
#define EQDlCompfilter_t struct _EQDlCompfilter_t


struct _EQUlCompfilter_t {
		Int32 ul_coef_fw_8k[MAX_NO_OF_BIQUADS][EQ_COEFF_FW_NUM];
		Int32 ul_coef_bw_8k[MAX_NO_OF_BIQUADS][EQ_COEFF_BW_NUM];
		Int16 ul_comp_filter_gain_8k[MAX_NO_OF_BIQUADS];
		Int32 ul_output_bit_select_8k;
		Int32 ul_coef_fw_16k[MAX_NO_OF_BIQUADS][EQ_COEFF_FW_NUM];
		Int32 ul_coef_bw_16k[MAX_NO_OF_BIQUADS][EQ_COEFF_BW_NUM];
		Int16 ul_comp_filter_gain_16k[MAX_NO_OF_BIQUADS];
		Int32 ul_output_bit_select_16k;
		UInt16 ul_nstage_filter;
		UInt16 reserved;
};
#define EQUlCompfilter_t struct _EQUlCompfilter_t


struct _AudioCompfilter_t {
		EQDlCompfilter_t dl;
		EQUlCompfilter_t ul;
};
#define AudioCompfilter_t struct _AudioCompfilter_t

#ifndef CONFIG_BCM_MODEM
struct _AudioSysParm_t {
		UInt16 mic_pga;
		UInt16 voice_volume_max;	/* in dB. */
		UInt16 dual_mic_enable;
		UInt16 amic_dga_coarse_gain;
		UInt16 amic_dga_fine_gain;
		UInt16 dmic1_dga_coarse_gain;
		UInt16 dmic1_dga_fine_gain;
		UInt16 dmic2_dga_coarse_gain;
		UInt16 dmic2_dga_fine_gain;
		UInt16 dmic3_dga_coarse_gain;
		UInt16 dmic3_dga_fine_gain;
		UInt16 dmic4_dga_coarse_gain;
		UInt16 dmic4_dga_fine_gain;
		UInt16 srcmixer_input_gain_l;
		UInt16 srcmixer_output_coarse_gain_l;
		UInt16 srcmixer_output_fine_gain_l;
		UInt16 srcmixer_input_gain_r;
		UInt16 srcmixer_output_coarse_gain_r;
		UInt16 srcmixer_output_fine_gain_r;
		UInt16 ext_speaker_pga_l;
		UInt16 ext_speaker_pga_r;
		UInt16 hw_sidetone_enable;
		UInt16 hw_sidetone_gain;
		/* add new variables here */
		/* #define COEF_NUM_OF_EACH_GROUP_HW_SIDETONE            8 */
		/* #define NUM_OF_GROUP_HW_SIDETONE              16	*/
		Int32 hw_sidetone_eq[8 * 16];
};
#define AudioSysParm_t struct _AudioSysParm_t

#endif

UInt32 audio_control_generic(UInt32 param1,
			UInt32 param2,
			UInt32 param3,
			UInt32 param4,
			UInt32 param5, UInt32 param6);

UInt32 audio_control_dsp(UInt32 param1,
			UInt32 param2,
			UInt32 param3,
			UInt32 param4, UInt32 param5, UInt32 param6);

UInt32 audio_cmf_filter(AudioCompfilter_t *cf);

/* Description:   Inititialize audio driver */
void AUDDRV_Init(void);

/* shut down audio driver */
void AUDDRV_Shutdown(void);

/*  the control sequence for starting telephony audio. */
void AUDDRV_Telephony_Init(AUDIO_SOURCE_Enum_t mic,
		AUDIO_SINK_Enum_t speaker);

/**
*  @brief  the control sequence for rate change of voice call.
*
*  @param  sample_rate  (in) voice call sampling rate
*
*  @return none
*
****************************************************************************/
void AUDDRV_Telephony_RateChange(unsigned int sample_rate);

/**
*
*  @brief  Register callback for rate change
*
*  @param  codecId_cb
*
*  @return none
*****************************************************************************/
void AUDDRV_RegisterRateChangeCallback(audio_codecId_handler_t
			codecId_cb);

/**
*  @brief  the rate change request function called by CAPI message listener
*
*  @param  codecID		(in) voice call speech codec ID
*
*  @return none
*
****************************************************************************/
void AUDDRV_Telephone_RequestRateChange(int codecID);

/**
*  @brief  Get voice call sampling rate, stored for call session.
*
*  @param  none
*
*  @return unsigned int (voiceCallSampleRate)
*
****************************************************************************/
unsigned int AUDDRV_Telephone_GetSampleRate(void);

/**
*  @brief  Save voice call sampling rate, stored for call session.
*
*  @param  unsigned int (in) voiceCallSampleRate
*
*  @return none
*
****************************************************************************/
void AUDDRV_Telephone_SaveSampleRate(unsigned int sample_rate);

/* the control sequence for ending telephony audio.
 this func let DSP to turn off voice path, if need to resume apps operation
 on voice, controller needs to reenable voice path after phonce call ends. */
void AUDDRV_Telephony_Deinit(void);

void AUDDRV_Telephony_SelectMicSpkr(AUDIO_SOURCE_Enum_t mic,
			AUDIO_SINK_Enum_t speaker);
/* Enable DSP output processing. */
void AUDDRV_EnableDSPOutput(AUDIO_SINK_Enum_t mixer_speaker_selection,
			AUDIO_SAMPLING_RATE_t sample_rate);
/* Enable DSP input processing. */
void AUDDRV_EnableDSPInput(AUDIO_SOURCE_Enum_t mic_selection,
			AUDIO_SAMPLING_RATE_t sample_rate);
void AUDDRV_DisableDSPOutput(void);
void AUDDRV_DisableDSPInput(void);
Boolean AUDDRV_IsVoiceCallWB(AudioMode_t audio_mode);
Boolean AUDDRV_IsCall16K(AudioMode_t voiceMode);
Boolean AUDDRV_InVoiceCall(void);

void AUDDRV_SetVoiceCallFlag(Boolean inVoiceCall);

#ifdef CONFIG_BCM_MODEM
SysAudioParm_t *AudParmP(void);
#else
AudioSysParm_t *AudParmP(void);
#endif

#if defined(USE_NEW_AUDIO_PARAM)
void AUDDRV_SetAudioMode(AudioMode_t audio_mode, AudioApp_t audio_app);
#else
void AUDDRV_SetAudioMode(AudioMode_t audio_mode);
#endif
void AUDDRV_SetAudioMode_ForMusicPlayback(AudioMode_t audio_mode,
			AudioApp_t audio_app,
			unsigned int arg_pathID,
			Boolean inHWlpbk);
void AUDDRV_SetAudioMode_ForMusicRecord(AudioMode_t audio_mode,
				AudioApp_t audio_app,
				unsigned int arg_pathID);

AudioMode_t AUDDRV_GetAudioModeBySink(AUDIO_SINK_Enum_t sink);

int AUDDRV_User_CtrlDSP(AudioDrvUserCtrl_t UserCtrlType,
				Boolean enable, Int32 size, void *param);

void AUDDRV_User_HandleDSPInt(UInt32 param1, UInt32 param2,
				      UInt32 param3);
void AUDDRV_SetPCMOnOff(Boolean on_off);

void AUDDRV_ControlFlagFor_CustomGain(Boolean on_off);

/**
*  @brief  Set telephony microphone (uplink) gain
*
*  @param  mic	(in)  microphone selection
*  @param  gain	(in)  gain
*  @param  gain_format	(in)  gain format
*
*  @return none
*
****************************************************************************/
void AUDDRV_SetTelephonyMicGain(AUDIO_SOURCE_Enum_t mic,
		Int16 gain,
		AUDIO_GAIN_FORMAT_t gain_format);
/**
*  @brief  Set telephony speaker (downlink) volume
*
*  @param  speaker	(in)  downlink sink, speaker selection
*  @param  volume	(in)  downlink volume
*  @param  gain_format	 (in)  gain format
*
*  @return none
*
****************************************************************************/
void AUDDRV_SetTelephonySpkrVolume(AUDIO_SINK_Enum_t speaker,
		int volume,
		AUDIO_GAIN_FORMAT_t gain_format);

void AUDDRV_Telephony_MuteMic(AUDIO_SOURCE_Enum_t mic);
void AUDDRV_Telephony_UnmuteMic(AUDIO_SOURCE_Enum_t mic);
void AUDDRV_Telephony_MuteSpkr(AUDIO_SINK_Enum_t speaker);
void AUDDRV_Telephony_UnmuteSpkr(AUDIO_SINK_Enum_t speaker);
void AUDDRV_SetULSpeechRecordGain(Int16 gain);

/*********************************************************************
*
* Get BTM headset NB or WB info
* @return	Boolean, TRUE for WB and FALSE for NB (8k)
* @note
**********************************************************************/
Boolean AUDDRV_IsBTMWB(void);

/*********************************************************************
*
* Set BTM type
* @param	Boolean isWB
* @return	none
*
* @note	isWB=TRUE for BT WB headset; =FALSE for BT NB (8k) headset.
**********************************************************************/
void AUDDRV_SetBTMTypeWB(Boolean isWB);

int AUDDRV_Get_CP_AudioMode(void);
void Audio_InitRpc(void);
void AUDLOG_ProcessLogChannel(UInt16 audio_stream_buffer_idx);
void AUDDRV_EC(Boolean enable, UInt32 arg);
void AUDDRV_NS(Boolean enable);
#endif				/* __AUDIO_VDRIVER_H__ */
