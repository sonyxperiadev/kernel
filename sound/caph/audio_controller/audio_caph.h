/*****************************************************************************
Copyright 2010 - 2011 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement
governing use of this software, this software is licensed to you under the
terms of the GNU General Public License version 2, available at
http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software
in any way with any other Broadcom software provided under a license other than
the GPL, without Broadcom's express prior written consent.
*****************************************************************************/
/**
*    @file   audio_caph.h
*    @brief  API declaration of hardware abstraction layer for Audio driver.
*   This code is OS independent and Device independent for audio device control.
*****************************************************************************/

#ifndef _AUDIO_CAPH_H__
#define _AUDIO_CAPH_H__

#define MAX_PLAYBACK_DEV 3

struct _TIDChanOfPlaybackDev {
	AUDIO_DRIVER_TYPE_t drv_type;
	AUDIO_SOURCE_Enum_t source;
	AUDIO_SINK_Enum_t sink;
};
#define	TIDChanOfPlaybackDev struct _TIDChanOfPlaybackDev

struct _TIDChanOfCaptureDev {
	AUDIO_DRIVER_TYPE_t drv_type;
	AUDIO_SOURCE_Enum_t source;
	AUDIO_SINK_Enum_t sink;
};
#define TIDChanOfCaptureDev struct _TIDChanOfCaptureDev

struct _TIDChanOfVoiceCallDev {
	AUDIO_SOURCE_Enum_t mic;
	AUDIO_SINK_Enum_t speaker;

};
#define TIDChanOfVoiceCallDev struct _TIDChanOfVoiceCallDev


struct _TIDChanOfDev {
	TIDChanOfPlaybackDev p[MAX_PLAYBACK_DEV];
	TIDChanOfCaptureDev c;
	TIDChanOfVoiceCallDev v;
};
#define TIDChanOfDev struct _TIDChanOfDev

/**
 * ! The higher layer calls this Audio hardware abstraction layer to perform
 * ! the following actions. This is expandable  if audio controller need to
 * ! handle more requests.
 */

enum __BRCM_AUDIO_ACTION_en_t {
	ACTION_AUD_OpenPlay,
	ACTION_AUD_ClosePlay,
	ACTION_AUD_StartPlay,
	ACTION_AUD_StopPlay,
	ACTION_AUD_PausePlay,
	ACTION_AUD_ResumePlay,
	ACTION_AUD_StartRecord,
	ACTION_AUD_StopRecord,
	ACTION_AUD_OpenRecord,
	ACTION_AUD_CloseRecord,
	ACTION_AUD_SetPrePareParameters,/* 10 */
	ACTION_AUD_AddChannel,
	ACTION_AUD_RemoveChannel,
	ACTION_AUD_EnableTelephony,
	ACTION_AUD_DisableTelephony,
	ACTION_AUD_EnableECNSTelephony,
	ACTION_AUD_DisableECNSTelephony,
	ACTION_AUD_SetTelephonyMicSpkr,
	ACTION_AUD_MutePlayback,
	ACTION_AUD_MuteRecord,
	ACTION_AUD_MuteTelephony,/* 20 */
	ACTION_AUD_EnableByPassVibra,
	ACTION_AUD_DisableByPassVibra,
	ACTION_AUD_SetPlaybackVolume,
	ACTION_AUD_SetRecordGain,
	ACTION_AUD_SetTelephonySpkrVolume,
	ACTION_AUD_SwitchSpkr,
	ACTION_AUD_SetHWLoopback,
	ACTION_AUD_SetAudioMode,
	ACTION_AUD_SetAudioApp, /*set audio profile*/
	ACTION_AUD_EnableFMPlay,
	ACTION_AUD_DisableFMPlay,
	ACTION_AUD_SetARM2SPInst,
	ACTION_AUD_RateChange,/* 33 */
	ACTION_AUD_AMPEnable,
	ACTION_AUD_DisableByPassVibra_CB,
	ACTION_AUD_SetCallMode,
	ACTION_AUD_ConnectDL,
	ACTION_AUD_UpdateUserVolSetting,
	ACTION_AUD_BufferReady,
	ACTION_AUD_BTTest,
	ACTION_AUD_CfgIHF,
	ACTION_AUD_CfgSSP,
	ACTION_AUD_HwCtl,
	ACTION_AUD_AtCtl,
	ACTION_AUD_TOTAL
};
#define BRCM_AUDIO_ACTION_en_t enum __BRCM_AUDIO_ACTION_en_t

struct __BRCM_AUDIO_Param_Start_t {
	void *drv_handle;
	TIDChanOfDev *pdev_prop;
	UInt32 channels; /* num_of_channel */
	UInt32 rate;
	Int32 vol[2];
	Int32 mixMode;
	Int32 callMode;
	int stream;
};
#define BRCM_AUDIO_Param_Start_t struct __BRCM_AUDIO_Param_Start_t

struct __BRCM_AUDIO_Param_Stop_t {
	void *drv_handle;
	TIDChanOfDev *pdev_prop;
	Int32 callMode;
	int stream;
};
#define BRCM_AUDIO_Param_Stop_t struct __BRCM_AUDIO_Param_Stop_t

struct __BRCM_AUDIO_Param_Pause_t {
	void *drv_handle;
	TIDChanOfDev *pdev_prop;
	int stream;
};
#define BRCM_AUDIO_Param_Pause_t struct __BRCM_AUDIO_Param_Pause_t

struct __BRCM_AUDIO_Param_Resume_t {
	void *drv_handle;
	TIDChanOfDev *pdev_prop;
	UInt32 channels;
	UInt32 rate;
	int stream;
};
#define BRCM_AUDIO_Param_Resume_t struct __BRCM_AUDIO_Param_Resume_t

struct __BRCM_AUDIO_Param_Open_t {
	void *drv_handle;
	TIDChanOfDev *pdev_prop;

};
#define BRCM_AUDIO_Param_Open_t struct __BRCM_AUDIO_Param_Open_t

struct __BRCM_AUDIO_Param_Close_t {
	void *drv_handle;
	TIDChanOfDev *pdev_prop;

};
#define BRCM_AUDIO_Param_Close_t struct __BRCM_AUDIO_Param_Close_t

struct __BRCM_AUDIO_Param_Prepare_t {
	unsigned long period_bytes;
	AUDIO_DRIVER_HANDLE_t drv_handle;
	AUDIO_DRIVER_BUFFER_t buf_param;
	AUDIO_DRIVER_CONFIG_t drv_config;
	AUDIO_DRIVER_CallBackParams_t cbParams;
};
#define BRCM_AUDIO_Param_Prepare_t struct __BRCM_AUDIO_Param_Prepare_t

struct __BRCM_AUDIO_Param_BufferReady_t {
	AUDIO_DRIVER_HANDLE_t drv_handle;
	TIDChanOfDev *pdev_prop;
	int stream;
};
#define BRCM_AUDIO_Param_BufferReady_t struct __BRCM_AUDIO_Param_BufferReady_t

struct __BRCM_AUDIO_Param_Volume_t {
	Int32 source;
	Int32 sink;
	Int32 volume1;
	Int32 volume2;
	int stream;
	AudioApp_t app;
	AUDIO_GAIN_FORMAT_t gain_format;
};
#define BRCM_AUDIO_Param_Volume_t struct __BRCM_AUDIO_Param_Volume_t


struct __BRCM_AUDIO_Param_Mute_t {
	Int32 source;
	Int32 sink;
	Int32 mute1;
	Int32 mute2;
	int stream;

};
#define BRCM_AUDIO_Param_Mute_t struct __BRCM_AUDIO_Param_Mute_t

struct __BRCM_AUDIO_Param_Spkr_t {
	Int32 src;
	Int32 sink;
	int stream;
};
#define BRCM_AUDIO_Param_Spkr_t struct __BRCM_AUDIO_Param_Spkr_t

struct __BRCM_AUDIO_Param_Call_t {
	AUDIO_SINK_Enum_t cur_spkr;
	AUDIO_SINK_Enum_t new_spkr;
	AUDIO_SOURCE_Enum_t cur_mic;
	AUDIO_SOURCE_Enum_t new_mic;
};
#define BRCM_AUDIO_Param_Call_t struct __BRCM_AUDIO_Param_Call_t


struct __BRCM_AUDIO_Param_Loopback_t {
	Int32 parm;
	Int32 mic;
	Int32 spkr;
	Int32 mode;
};
#define BRCM_AUDIO_Param_Loopback_t struct __BRCM_AUDIO_Param_Loopback_t
struct __BRCM_AUDIO_Param_Vibra_t {
	Int32 strength;
	Int32 direction;
	Int32 duration;
};
#define BRCM_AUDIO_Param_Vibra_t struct __BRCM_AUDIO_Param_Vibra_t

struct __BRCM_AUDIO_Param_FM_t {
	Int32 source;
	Int32 sink;
	Int32 volume1;
	Int32 volume2;
	UInt32 fm_mix;
	int stream;
};
#define BRCM_AUDIO_Param_FM_t struct __BRCM_AUDIO_Param_FM_t


struct __BRCM_AUDIO_Param_RateChange_t {
	UInt8 codecID;

};
#define BRCM_AUDIO_Param_RateChange_t struct __BRCM_AUDIO_Param_RateChange_t

struct __BRCM_AUDIO_Param_SetApp_t {
	int aud_app;
	int aud_mode;

};
#define BRCM_AUDIO_Param_SetApp_t struct __BRCM_AUDIO_Param_SetApp_t

struct __BRCM_AUDIO_Param_CallMode_t {
	UInt32 callMode;

};
#define BRCM_AUDIO_Param_CallMode_t struct __BRCM_AUDIO_Param_CallMode_t

struct __BRCM_AUDIO_Param_ECNS_t {
	Int32 ec_ns;
};
#define BRCM_AUDIO_Param_ECNS_t struct __BRCM_AUDIO_Param_ECNS_t

struct __BRCM_AUDIO_Param_AMPCTL_t {
	Int32 amp_status;
};
#define BRCM_AUDIO_Param_AMPCTL_t struct __BRCM_AUDIO_Param_AMPCTL_t

struct __BRCM_AUDIO_Param_BT_Test_t {
	int mode;
};
#define BRCM_AUDIO_Param_BT_Test_t struct __BRCM_AUDIO_Param_BT_Test_t

struct __BRCM_AUDIO_Param_Cfg_IHF_t {
	Boolean stIHF;
};
#define BRCM_AUDIO_Param_Cfg_IHF_t struct __BRCM_AUDIO_Param_Cfg_IHF_t

struct __BRCM_AUDIO_Param_Cfg_SSP_t {
	int mode;
	int bus;
	int en_lpbk;
};
#define BRCM_AUDIO_Param_Cfg_SSP_t struct __BRCM_AUDIO_Param_Cfg_SSP_t

struct __BRCM_AUDIO_Param_HwCtl_t {
	int access_type;
	int arg1;
	int arg2;
	int arg3;
	int arg4;
};
#define BRCM_AUDIO_Param_HwCtl_t struct __BRCM_AUDIO_Param_HwCtl_t

struct __BRCM_AUDIO_Param_AtCtl_t {
	Int32 cmdIndex;
	void *pChip;
	Int32 ParamCount;
	Int32 isGet;
	Int32 Params[7];
};
#define BRCM_AUDIO_Param_AtCtl_t struct __BRCM_AUDIO_Param_AtCtl_t

union __BRCM_AUDIO_Control_Params_un_t {
	BRCM_AUDIO_Param_Start_t param_start;
	BRCM_AUDIO_Param_Stop_t param_stop;
	BRCM_AUDIO_Param_Pause_t param_pause;
	BRCM_AUDIO_Param_Resume_t param_resume;
	BRCM_AUDIO_Param_Open_t parm_open;
	BRCM_AUDIO_Param_Close_t parm_close;
	BRCM_AUDIO_Param_Volume_t parm_vol;
	BRCM_AUDIO_Param_Mute_t parm_mute;
	BRCM_AUDIO_Param_Spkr_t parm_spkr;
	BRCM_AUDIO_Param_Call_t parm_call;
	BRCM_AUDIO_Param_RateChange_t parm_ratechange;
	BRCM_AUDIO_Param_Loopback_t parm_loop;
	BRCM_AUDIO_Param_Vibra_t parm_vibra;
	BRCM_AUDIO_Param_FM_t parm_FM;
	BRCM_AUDIO_Param_Prepare_t parm_prepare;
	BRCM_AUDIO_Param_SetApp_t parm_setapp;
	BRCM_AUDIO_Param_CallMode_t parm_callmode;
	BRCM_AUDIO_Param_ECNS_t parm_ecns;
	BRCM_AUDIO_Param_AMPCTL_t parm_ampctl;
	BRCM_AUDIO_Param_BT_Test_t parm_bt_test;
	BRCM_AUDIO_Param_Cfg_IHF_t parm_cfg_ihf;
	BRCM_AUDIO_Param_Cfg_SSP_t parm_cfg_ssp;
	BRCM_AUDIO_Param_HwCtl_t parm_hwCtl;
	BRCM_AUDIO_Param_AtCtl_t parm_atctl;
};
#define BRCM_AUDIO_Control_Params_un_t union __BRCM_AUDIO_Control_Params_un_t

void caph_audio_init(void);

int LaunchAudioCtrlThread(void);

int TerminateAudioHalThread(void);

void AUDIO_Ctrl_SetUserAudioApp(AudioApp_t app);

Result_t AUDIO_Ctrl_Trigger(BRCM_AUDIO_ACTION_en_t action_code,
			    void *arg_param, void *callback, int block);

#endif /* _CAPH_AUDIO_H__ */
