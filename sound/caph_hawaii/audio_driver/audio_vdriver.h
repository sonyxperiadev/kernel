/****************************************************************************
Copyright 2009 - 2012  Broadcom Corporation
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
typedef void (*audio_handleCPReset_handler_t) (Boolean cpReset);

/* Define the other mic which is used for Noise Cancellation.
	It is product-dependent. */
#define MIC_NOISE_CANCEL CSL_CAPH_DEV_EANC_DIGI_MIC_R

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

#if (!defined(CONFIG_BCM_MODEM) || defined(JAVA_ZEBU_TEST))
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
	UInt16 ext_speaker_high_gain_mode_enable;
	/* add new variables here */
	/* #define COEF_NUM_OF_EACH_GROUP_HW_SIDETONE            8 */
	/* #define NUM_OF_GROUP_HW_SIDETONE              16	*/
	Int32 hw_sidetone_eq[8 * 16];
	Int32 dsp_voice_vol_tbl[NUM_OF_ENTRY_IN_DSP_VOICE_VOLUME_TABLE];
	Int32 fm_radio_digital_vol[NUM_OF_ENTRY_IN_FM_RADIO_DIGITAL_VOLUME];
};
#define AudioSysParm_t struct _AudioSysParm_t

#endif
struct _SetAudioMode_Sp_t {
	AudioMode_t mode;
	AudioApp_t app;
	unsigned int pathID;
	Boolean inHWlpbk;
	int mixInGain_mB;
	int mixInGainR_mB;
	int mixOutGain_mB;
	int mixOutGainR_mB;
};
#define SetAudioMode_Sp_t struct _SetAudioMode_Sp_t

extern struct completion audioEnableDone;

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
		AUDIO_SINK_Enum_t speaker,
		AudioMode_t mode, AudioApp_t app,
		int bDualMic_IsNeeded,
		int bmuteVoiceCall);

/**
*  @brief  the control sequence for rate change of voice call.
*
*  @param  mode
*  @param  audio_app
*  @param  bDualMic_IsNeeded
*
*  @return none
*
****************************************************************************/
void AUDDRV_Telephony_RateChange(AudioMode_t mode,
	AudioApp_t audio_app, int bNeedDualMic, int bmuteVoiceCall);

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
*
*  @brief  Register callback for handling cp reset
*
*  @param  none
*
*  @return none
*****************************************************************************/
void AUDDRV_RegisterHandleCPResetCB(audio_handleCPReset_handler_t cpReset_cb);

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
*  @brief  the rate change request function called by CAPI message listener
*
*  @param  codecID		(in) voice call speech codec ID
*
*  @return none
*
****************************************************************************/
void AUDDRV_HandleCPReset(Boolean cp_reset_start);

/* the control sequence for ending telephony audio.
 this func let DSP to turn off voice path, if need to resume apps operation
 on voice, controller needs to reenable voice path after phonce call ends. */
void AUDDRV_Telephony_Deinit(void);

/* Enable DSP output processing. */
void AUDDRV_EnableDSPOutput(AUDIO_SINK_Enum_t mixer_speaker_selection,
			AUDIO_SAMPLING_RATE_t sample_rate);
/* Enable DSP input processing. */
void AUDDRV_EnableDSPInput(AUDIO_SOURCE_Enum_t mic_selection,
			AUDIO_SAMPLING_RATE_t sample_rate);
void AUDDRV_DisableDSPOutput(void);
void AUDDRV_DisableDSPInput(int stop);

SysMultimediaAudioParm_t *MMAudParmP(void);

#if defined(CONFIG_BCM_MODEM) && (!defined(JAVA_ZEBU_TEST))
SysAudioParm_t *AudParmP(void);
#else
AudioSysParm_t *AudParmP(void);
#endif

void AUDDRV_SetAudioMode(AudioMode_t audio_mode, AudioApp_t audio_app,
	CSL_CAPH_PathID ulPathID,
	CSL_CAPH_PathID ul2PathID,
	CSL_CAPH_PathID dlPathID);

void AUDDRV_SetAudioMode_Speaker(SetAudioMode_Sp_t param);

#ifdef CONFIG_ENABLE_SSMULTICAST
void AUDDRV_SetAudioMode_Multicast(SetAudioMode_Sp_t param);
#endif

void AUDDRV_SetAudioMode_Mic(AudioMode_t audio_mode,
				AudioApp_t audio_app,
				unsigned int arg_pathID,
				unsigned int arg_pathID2);

int AUDDRV_User_CtrlDSP(AudioDrvUserCtrl_t UserCtrlType,
				Boolean enable, Int32 size, void *param);

void AUDDRV_User_HandleDSPInt(UInt32 param1, UInt32 param2,
				      UInt32 param3);

void AUDDRV_ControlFlagFor_CustomGain(Boolean on_off);

void AUDDRV_SetULSpeechRecordGain(Int16 gain);

void Audio_InitRpc(void);
void AUDLOG_ProcessLogChannel(UInt16 audio_stream_buffer_idx);
void AUDDRV_EC(Boolean enable, UInt32 arg);
void AUDDRV_NS(Boolean enable);
void AUDDRV_ECreset_NLPoff(Boolean ECenable);

void AUDDRV_SetTuningFlag(int flag);
int AUDDRV_TuningFlag(void);
int AUDDRV_GetULPath(void);
void AUDDRV_SetCallMode(Int32);
void AUDDRV_ConnectDL(void);
void AUDDRV_CPResetCleanup(void);
void AUDDRV_Telephony_DeinitHW(void);
CSL_CAPH_DEVICE_e AUDDRV_GetDRVDeviceFromSpkr(AUDIO_SINK_Enum_t spkr);
CSL_CAPH_DEVICE_e AUDDRV_GetDRVDeviceFromMic(AUDIO_SOURCE_Enum_t mic);
void AUDDRV_SetPrimaryMicFromSpkr(AUDIO_SINK_Enum_t spkr,
		AUDIO_SOURCE_Enum_t mic);
AUDIO_SOURCE_Enum_t AUDDRV_GetPrimaryMicFromSpkr(AUDIO_SINK_Enum_t spkr);
void AUDDRV_SetSecMicFromSpkr(AUDIO_SINK_Enum_t spkr,
		AUDIO_SOURCE_Enum_t mic);
AUDIO_SOURCE_Enum_t AUDDRV_GetSecMicFromSpkr(AUDIO_SINK_Enum_t spkr);
void AUDDRV_PrintAllMics(void);
void AUDDRV_SetCallSampleRate(UInt32 voiceSampleRate);
void AUDDRV_SetIHFDLSampleRate(int mode);
int AUDDRV_Get_TrEqParm(void *param, unsigned int size, AudioApp_t app,
	unsigned int sample_rate);
void AUDDRV_SetEchoRefMic(int arg1);
int AUDDRV_GetEchoRefMic(void);
void set_flag_dsp_timeout(int flag_val);
int is_dsp_timeout(void);
int AUDDRV_Get_FDMBCParm(void *param, int size);
#endif				/* __AUDIO_VDRIVER_H__ */
