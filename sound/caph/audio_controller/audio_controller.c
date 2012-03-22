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
* @file   audio_controller.c
*
* @brief  manage audio mode and audio gains over CAPH driver and external audio driver.
*
******************************************************************************/

#include "mobcom_types.h"
#include "resultcode.h"

#include "csl_arm2sp.h"
#include "csl_vpu.h"

#include "audio_consts.h"

#include "bcm_fuse_sysparm_CIB.h"

#include "csl_caph.h"
#include "csl_caph_audioh.h"
#include "csl_caph_hwctrl.h"
#include "csl_audio_render.h"
#include "csl_audio_capture.h"

#include "audio_vdriver.h"
#include "audio_controller.h"

#ifdef CONFIG_DIGI_MIC
#ifdef CONFIG_BCMPMU_AUDIO
#include "pmu.h"
#include "hal_pmu.h"
#include "hal_pmu_private.h"
#endif
#endif

#include "extern_audio.h"
#include "voif_handler.h"

#include "audctrl_policy.h"
#include "audio_trace.h"

#include <linux/err.h>
#include <linux/regulator/consumer.h>

/**There are two loopback paths available in AudioH.
One is 6.5MHz analog microphone loopback path. It does not support digital mics.
The other one is HW sidetone path. It supports all the mics. This is prefered.
Only one loopback path can be enabled at a time.*/
#define HW_SIDETONE_LOOPBACK
#undef HW_ANALOG_LOOPBACK

#define VIBRA_LDO_REGULATOR "2v9_vibra"
/** Private Type and Constant declarations */

enum ExtSpkrUsage_en_t {
	TelephonyUse,
	AudioUse,
	Audio2Use,	/* for 2nd audio playback */
	FmUse
};

struct AUDCTRL_SPKR_Mapping_t {
	AUDIO_SINK_Enum_t spkr;
	CSL_CAPH_DEVICE_e dev;
};

/* must match AUDIO_SINK_Enum_t */
static struct AUDCTRL_SPKR_Mapping_t \
SPKR_Mapping_Table[AUDIO_SINK_TOTAL_COUNT] = {
	/* sink ino                                       Device ID */
	{AUDIO_SINK_HANDSET, CSL_CAPH_DEV_EP},
	{AUDIO_SINK_HEADSET, CSL_CAPH_DEV_HS},
	{AUDIO_SINK_HANDSFREE, CSL_CAPH_DEV_IHF},
	{AUDIO_SINK_BTM, CSL_CAPH_DEV_BT_SPKR},
	{AUDIO_SINK_LOUDSPK, CSL_CAPH_DEV_IHF},
	{AUDIO_SINK_TTY, CSL_CAPH_DEV_HS},
	{AUDIO_SINK_HAC, CSL_CAPH_DEV_EP},
	{AUDIO_SINK_USB, CSL_CAPH_DEV_MEMORY},
	{AUDIO_SINK_BTS, CSL_CAPH_DEV_BT_SPKR},
	{AUDIO_SINK_I2S, CSL_CAPH_DEV_FM_TX},
	{AUDIO_SINK_VIBRA, CSL_CAPH_DEV_VIBRA},
	{AUDIO_SINK_HEADPHONE, CSL_CAPH_DEV_HS},
	{AUDIO_SINK_VALID_TOTAL, CSL_CAPH_DEV_NONE},
	{AUDIO_SINK_MEM, CSL_CAPH_DEV_MEMORY},
	{AUDIO_SINK_DSP, CSL_CAPH_DEV_DSP},
	{AUDIO_SINK_UNDEFINED, CSL_CAPH_DEV_NONE}
};

struct AUDIO_SOURCE_Mapping_t {
	AUDIO_SOURCE_Enum_t mic;
	CSL_CAPH_DEVICE_e dev;
};

/* must match AUDIO_SOURCE_Enum_t */
static struct AUDIO_SOURCE_Mapping_t \
MIC_Mapping_Table[AUDIO_SOURCE_TOTAL_COUNT] = {
	/* source info              Device ID */
	{AUDIO_SOURCE_UNDEFINED, CSL_CAPH_DEV_NONE},
	{AUDIO_SOURCE_ANALOG_MAIN, CSL_CAPH_DEV_ANALOG_MIC},
	{AUDIO_SOURCE_ANALOG_AUX, CSL_CAPH_DEV_HS_MIC},
	{AUDIO_SOURCE_DIGI1, CSL_CAPH_DEV_DIGI_MIC_L},
	{AUDIO_SOURCE_DIGI2, CSL_CAPH_DEV_DIGI_MIC_R},
	{AUDIO_SOURCE_DIGI3, CSL_CAPH_DEV_EANC_DIGI_MIC_L},
	{AUDIO_SOURCE_DIGI4, CSL_CAPH_DEV_EANC_DIGI_MIC_R},
	{AUDIO_SOURCE_MIC_ARRAY1, CSL_CAPH_DEV_DIGI_MIC},
	{AUDIO_SOURCE_MIC_ARRAY2, CSL_CAPH_DEV_EANC_DIGI_MIC},
	{AUDIO_SOURCE_BTM, CSL_CAPH_DEV_BT_MIC},
	{AUDIO_SOURCE_USB, CSL_CAPH_DEV_MEMORY},
	{AUDIO_SOURCE_I2S, CSL_CAPH_DEV_FM_RADIO},
	{AUDIO_SOURCE_RESERVED1, CSL_CAPH_DEV_NONE},
	{AUDIO_SOURCE_RESERVED2, CSL_CAPH_DEV_NONE},
	{AUDIO_SOURCE_VALID_TOTAL, CSL_CAPH_DEV_NONE},
	{AUDIO_SOURCE_SPEECH_DIGI, CSL_CAPH_DEV_DIGI_MIC},
	{AUDIO_SOURCE_MEM, CSL_CAPH_DEV_MEMORY},
	{AUDIO_SOURCE_DSP, CSL_CAPH_DEV_DSP}
};

/** Private Variables */

static AUDIO_SINK_Enum_t voiceCallSpkr = AUDIO_SINK_UNDEFINED;
static AUDIO_SOURCE_Enum_t voiceCallMic = AUDIO_SOURCE_UNDEFINED;

static int telephony_dl_gain_dB;
static int telephony_ul_gain_dB;

static unsigned int voiceCallSampleRate = AUDIO_SAMPLING_RATE_8000;
static int IsBTM_WB = FALSE;	/*Bluetooth headset is WB(16KHz voice) */
static int bInVoiceCall = FALSE;
static int bmuteVoiceCall = FALSE;
static Boolean isMFD = FALSE;
static Boolean is26MClk = FALSE;
static Boolean muteInPlay = FALSE;
static Boolean isStIHF = FALSE;

/*
static unsigned int recordGainL[ AUDIO_SOURCE_TOTAL_COUNT ] = {0};
static unsigned int recordGainR[ AUDIO_SOURCE_TOTAL_COUNT ] = {0};
*/

enum AUDPATH_en_t {
	AUDPATH_NONE,
	AUDPATH_P1,
	AUDPATH_P2,
	AUDPATH_VOIP_OUT,
	AUDPATH_AUDIO_IN,
	AUDPATH_SPEECH_IN,
	AUDPATH_VOIP_IN,
	AUDPATH_VOICECALL,
	AUDPATH_FM,
	AUDPATH_FM_CAPTURE,
	AUDPATH_FM_TX,
	AUDPATH_TOTAL_NUM
};

struct USER_SET_GAIN_t {
	int L;
	int R;
	/*AUDIO_SINK_Enum_t sink1; */
	/*AUDIO_SINK_Enum_t sink2; */
	/*int path2_L; */
	/*int path2_R; */
	int valid;
};
static struct USER_SET_GAIN_t users_gain[AUDPATH_TOTAL_NUM];
static struct USER_SET_GAIN_t
	user_vol_setting[AUDIO_APP_NUMBER][AUDIO_MODE_NUMBER];

static unsigned hw_control[AUDCTRL_HW_ACCESS_TYPE_TOTAL][4] = { {0} };

static Boolean fmPlayStarted = FALSE;
/* pathID of the playback path */
static unsigned int playbackPathID;
/* pathID of the recording path */
/*static unsigned int recordPathID;*/
static unsigned int pathIDTuning;	/* init to 0, for tuning purpose only */

static AudioApp_t currAudioApp = AUDIO_APP_VOICE_CALL;
static AudioMode_t currAudioMode = AUDIO_MODE_HANDSET;
 /* need to update this on AP and also in audioapi.c on CP. */
static AudioMode_t currAudioMode_playback = AUDIO_MODE_SPEAKERPHONE;
static AudioMode_t currAudioMode_record = AUDIO_MODE_SPEAKERPHONE;
static AudioMode_t currAudioMode_fm = AUDIO_MODE_HEADSET;

static struct regulator *vibra_reg;

static int isDigiMic(AUDIO_SOURCE_Enum_t source);
static int needDualMic(AudioMode_t mode, AudioApp_t app);
static AudioMode_t GetAudioModeFromCaptureDev(CSL_CAPH_DEVICE_e source);
static void powerOnExternalAmp(AUDIO_SINK_Enum_t speaker,
			       enum ExtSpkrUsage_en_t usage_flag,
			       int use, int force);
static void setExternAudioGain(AudioMode_t mode, AudioApp_t app);
static void fillUserVolSetting(AudioMode_t mode, AudioApp_t app);

/****************************************************************************
*
*  Function Name: AUDCTRL_GetModeBySpeaker
*
*  Description: Get the mode via speaker
*
****************************************************************************/
static AudioMode_t AUDCTRL_GetModeBySpeaker(CSL_CAPH_DEVICE_e speaker)
{
	AudioMode_t mode = AUDIO_MODE_HANDSET;

	switch (speaker) {
	case CSL_CAPH_DEV_EP:
		mode = AUDIO_MODE_HANDSET;
		break;
	case CSL_CAPH_DEV_HS:
		mode = AUDIO_MODE_HEADSET;
		break;
	case CSL_CAPH_DEV_IHF:
		mode = AUDIO_MODE_SPEAKERPHONE;
		break;
	case CSL_CAPH_DEV_BT_SPKR:
		mode = AUDIO_MODE_BLUETOOTH;
		break;
	case CSL_CAPH_DEV_FM_TX:
		mode = AUDIO_MODE_RESERVE;
		break;
	default:
		break;
	}
	return mode;
}

/****************************************************************************
*
* Function Name: AUDCTRL_Init
*
* Description:   Init function
*
****************************************************************************/
void AUDCTRL_Init(void)
{
	int i;

	AUDDRV_Init();
	csl_caph_hwctrl_init();

	/*access sysparm here will cause system panic.
	   sysparm is not initialzed when this fucniton is called. */
	/* telephony_digital_gain_dB = 12;
SYSPARM_GetAudioParamsFromFlash( cur_mode )->voice_volume_init;  dB */

	for (i = 0; i < AUDPATH_TOTAL_NUM; i++)
		users_gain[i].valid = FALSE;

	memset(&user_vol_setting, 0, sizeof(user_vol_setting));
}

/****************************************************************************
*
* Function Name: AUDCTRL_Shutdown
*
* Description:   De-Initialize audio controller
*
****************************************************************************/
void AUDCTRL_Shutdown(void)
{
	AUDDRV_Shutdown();
}

/****************************************************************************
*
* Function Name: AUDCTRL_EnableTelephony
*
* Description:   Enable telephonly path, both ul and dl
*
****************************************************************************/
void AUDCTRL_EnableTelephony(AUDIO_SOURCE_Enum_t source, AUDIO_SINK_Enum_t sink)
{
	AudioMode_t mode;
	AudioApp_t app = AUDIO_APP_VOICE_CALL;
	int bNeedDualMic = FALSE;

	aTrace(LOG_AUDIO_CNTLR, "%s sink %d, mic %d\n", __func__, sink, source);

	mode = GetAudioModeBySink(sink);

	if (AUDCTRL_Telephony_HW_16K(mode) == FALSE) {
		app = AUDCTRL_GetAudioApp();
		/*If VT app set from user,select VT app profile,
		only VT-NB supported */
		if (app != AUDIO_APP_VT_CALL)
			app = AUDIO_APP_VOICE_CALL;
	} else
		app = AUDIO_APP_VOICE_CALL_WB;

	if (user_vol_setting[app][mode].valid == FALSE)
		fillUserVolSetting(mode, app);

	bNeedDualMic = needDualMic(mode, app);

	AUDCTRL_SaveAudioApp(app);
	AUDCTRL_SaveAudioMode(mode);

	if (AUDCTRL_InVoiceCall()) {	/*already in voice call */
		if ((voiceCallSpkr != sink) || (voiceCallMic != source))
			AUDCTRL_SetTelephonyMicSpkr(source, sink);

		return;
	}

	if (isDigiMic(source)) {
		/* Enable PMU power to digital microphone */
		powerOnDigitalMic(TRUE);
	}

	bInVoiceCall = TRUE;

	/* for the next call, upper layer in Android un-mute mic before start
	   the next voice call.
	   //but in LMP (it has no Android code).
	   // in case it was muted from last voice call, need to un-mute it. */
	bmuteVoiceCall = FALSE;

	AUDDRV_Telephony_Init(source, sink, mode, app, bNeedDualMic,
			      bmuteVoiceCall);

	voiceCallSpkr = sink;
	voiceCallMic = source;

	powerOnExternalAmp(sink, TelephonyUse, TRUE, FALSE);

#ifdef CONFIG_ENABLE_VOIF
	mode = AUDCTRL_GetAudioMode();

	if (mode >= AUDIO_MODE_NUMBER)
		mode = (AudioMode_t) (mode - AUDIO_MODE_NUMBER);
	VoIF_init(mode);
#endif
	return;
}

/****************************************************************************
*
* Function Name: AUDCTRL_DisableTelephony
*
* Description:   disable telephony path, both dl and ul
*
****************************************************************************/
void AUDCTRL_DisableTelephony(void)
{
	aTrace(LOG_AUDIO_CNTLR, "AUDCTRL_DisableTelephony\n");

	/* continues speech playback when end the phone call.
	   continues speech recording when end the phone call.
	   if ( FALSE==vopath_enabled && FALSE==DspVoiceIfActive_DL()\
	   && FALSE==vipath_enabled )
	   { */

	if (AUDCTRL_InVoiceCall()) {
#ifdef CONFIG_ENABLE_VOIF
		VoIF_Deinit();
#endif

		powerOnExternalAmp(voiceCallSpkr, TelephonyUse,
				FALSE, FALSE);

		/* The following is the sequence we need to follow */
		AUDDRV_Telephony_Deinit();
		bInVoiceCall = FALSE;

		/* reset to 8KHz as default for the next call */
		voiceCallSampleRate = AUDIO_SAMPLING_RATE_8000;

		/* Disable power to digital microphone */
		if (isDigiMic(voiceCallMic))
			powerOnDigitalMic(FALSE);

		voiceCallSpkr = AUDIO_SINK_UNDEFINED;
		voiceCallMic = AUDIO_SOURCE_UNDEFINED;

		AUDCTRL_RemoveAudioApp(AUDIO_APP_VOICE_CALL);

	}
	return;
}

/****************************************************************************
*
* Function Name: AUDCTRL_RateChangeTelephony
*
* Description:   Change Nb / WB according to speech codec type used by
* mobile network
*
****************************************************************************/
void AUDCTRL_Telephony_RateChange(unsigned int sample_rate)
{
	AudioMode_t mode;
	AudioApp_t app;
	int bNeedDualMic;
	aTrace(LOG_AUDIO_CNTLR, "%s sample_rate %d", __func__, sample_rate);

	voiceCallSampleRate = sample_rate;
	/* remember the rate for current call.
	   (or for the incoming call in ring state.) */

	if (AUDCTRL_InVoiceCall()) {

		mode = AUDCTRL_GetAudioMode();

		if (AUDCTRL_Telephony_HW_16K(mode))
			app = AUDIO_APP_VOICE_CALL_WB;
		else
			app = AUDIO_APP_VOICE_CALL;

		AUDCTRL_SaveAudioApp(app);

		bNeedDualMic = needDualMic(mode, app);

		AUDDRV_Telephony_RateChange(mode, app, bNeedDualMic,
					    bmuteVoiceCall);
	}
}

/**
*  @brief  the rate change request function called by CAPI message listener
*
*  @param  codecID		(in) voice call speech codec ID
*
*  @return none
*
****************************************************************************/
void AUDCTRL_Telephony_RequestRateChange(UInt8 codecID)
{
	/* 0x0A as per 3GPP 26.103 Sec 6.3 indicates AMR WB  AUDIO_ID_CALL16k
	   0x06 indicates AMR NB */
	if (codecID == 0x0A) {	/* AMR-WB */
		AUDCTRL_Telephony_RateChange(AUDIO_SAMPLING_RATE_16000);
	} else if (codecID == 0x06) {	/* AMR-NB */
		AUDCTRL_Telephony_RateChange(AUDIO_SAMPLING_RATE_8000);
	}
}

/****************************************************************************
*
* Function Name: AUDCTRL_EC
*
* Description:   DSP Echo cancellation ON/OFF
*
****************************************************************************/
void AUDCTRL_EC(Boolean enable, UInt32 arg)
{
	AUDDRV_EC(enable, arg);
}

/****************************************************************************
*
* Function Name: AUDCTRL_NS
*
* Description:   DSP Noise Suppression ON/OFF
*
****************************************************************************/
void AUDCTRL_NS(Boolean enable)
{
	AUDDRV_NS(enable);
}

/****************************************************************************
*
* Function Name: AUDCTRL_EnableAmp
*
* Description:   AMP ON/OFF
*
****************************************************************************/
void AUDCTRL_EnableAmp(Int32 ampCtl)
{
	static Int32 curAmpStatus = TRUE;
	if (curAmpStatus == FALSE && ampCtl == TRUE) {
		/* sink and usage flags are  ignored when force is set to TRUE*/
		powerOnExternalAmp(AUDIO_SINK_UNDEFINED,
				AudioUse, TRUE, TRUE);
		curAmpStatus = TRUE;
	} else if (curAmpStatus == TRUE && ampCtl == FALSE) {
		/* sink and usage flags are ignored when force is set to TRUE*/
		powerOnExternalAmp(AUDIO_SINK_UNDEFINED,
				AudioUse, FALSE, TRUE);
		curAmpStatus = FALSE;
	}
}

/****************************************************************************
*
* Function Name: AUDCTRL_SetTelephonyMicSpkr
*
* Description:   Set the micphone and speaker to telephony path, previous
*              micophone and speaker are disabled.
*      actual audio mode is determined by sink, network speech coder sample
*      rate and BT headset support of WB.
*
****************************************************************************/
void AUDCTRL_SetTelephonyMicSpkr(AUDIO_SOURCE_Enum_t source,
				 AUDIO_SINK_Enum_t sink)
{
	AudioMode_t mode;
	AudioApp_t app = AUDIO_APP_VOICE_CALL;
	int bNeedDualMic = FALSE;

	aTrace(LOG_AUDIO_CNTLR, "%s sink %d, mic %d\n", __func__, sink, source);

	if (voiceCallMic == source && voiceCallSpkr == sink)
		return;

	if (source == AUDIO_SOURCE_USB || sink == AUDIO_SINK_USB)
		return;

	mode = GetAudioModeBySink(sink);
#ifdef	CONFIG_AUDIO_FEATURE_SET_DISABLE_ECNS
	/* when turning off EC and NS, we set mode to
	  * AUDIO_MODE_HANDSFREE as customer's request, while
	  * sink is till AUDIO_SINK_BTM. To avoid mode is reset to
	  * AUDIO_MODE_BLUETOOTH base don sink, we keep using
	  * AUDIO_MODE_HANDSFREE here.
	  */
	if (AUDCTRL_GetAudioMode() == AUDIO_MODE_HANDSFREE)
		mode = AUDIO_MODE_HANDSFREE;
#endif
	if (AUDCTRL_Telephony_HW_16K(mode) == FALSE)
		app = AUDIO_APP_VOICE_CALL;
	else
		app = AUDIO_APP_VOICE_CALL_WB;

	bNeedDualMic = needDualMic(mode, app);

	AUDCTRL_SaveAudioApp(app);
	AUDCTRL_SaveAudioMode(mode);

	if (AUDCTRL_InVoiceCall() == FALSE) {
		voiceCallSpkr = sink;
		voiceCallMic = source;

		/*if PCG changed audio mode when phone is idle,
		   here need to pass audio mode to CP.
		 */

		audio_control_generic(AUDDRV_CPCMD_PassAudioMode,
				      (UInt32) mode, (UInt32) app, 0, 0, 0);
		return;
	}

	if (voiceCallMic == source && voiceCallSpkr == sink)
		return;

	if (voiceCallSpkr != sink)
		powerOnExternalAmp(voiceCallSpkr, TelephonyUse,
				FALSE, FALSE);

	if (voiceCallMic != source) {
		if (isDigiMic(voiceCallMic)) {
			/* Disable power to digital microphone */
			powerOnDigitalMic(FALSE);
		}
	}

	if (voiceCallMic != source) {
		if (isDigiMic(source)) {
			/* Enable power to digital microphone */
			powerOnDigitalMic(TRUE);
		}
	}

	AUDDRV_Telephony_Deinit();
	AUDDRV_Telephony_Init(source, sink, mode, app,
	bNeedDualMic, bmuteVoiceCall);	/* retain the mute flag */

	if (voiceCallSpkr != sink)
		powerOnExternalAmp(sink, TelephonyUse,
				TRUE, FALSE);

	voiceCallSpkr = sink;
	voiceCallMic = source;
}

/****************************************************************************
*
* Function Name: AUDCTRL_SetTelephonySpkrVolume
*
* Description:   Set dl volume of telephony path
*
****************************************************************************/
void AUDCTRL_SetTelephonySpkrVolume(AUDIO_SINK_Enum_t speaker,
				    int volume, AUDIO_GAIN_FORMAT_t gain_format)
{
	AudioApp_t app;
	AudioMode_t mode;

#ifdef CONFIG_BCM_MODEM
	SysAudioParm_t *p;
#else
	AudioSysParm_t *p;
#endif

	app = AUDCTRL_GetAudioApp();
	mode = AUDCTRL_GetAudioMode();
	p = &(AudParmP()[mode + app * AUDIO_MODE_NUMBER]);

	aTrace(LOG_AUDIO_CNTLR, "%s volume = %d", __func__, volume);

	users_gain[AUDPATH_VOICECALL].valid = TRUE;
	users_gain[AUDPATH_VOICECALL].L = volume;
	users_gain[AUDPATH_VOICECALL].R = volume;

	if (gain_format == AUDIO_GAIN_FORMAT_mB) {
		/* volume is in range of -3600 mB ~ 0 mB from the API */
		telephony_dl_gain_dB = (volume / 100);

		if (telephony_dl_gain_dB > 0)
			telephony_dl_gain_dB = 0;

		if (telephony_dl_gain_dB < -(p->voice_volume_max))
			telephony_dl_gain_dB = -(p->voice_volume_max);

		user_vol_setting[app][mode].L = volume;
		user_vol_setting[app][mode].valid = TRUE;

	} else if (gain_format == AUDIO_GAIN_FORMAT_DSP_VOICE_VOL_GAIN) {
		if (volume > 14)
			volume = 14;	/* 15 entries: 0 ~ 14. */

		telephony_dl_gain_dB = p->dsp_voice_vol_tbl[volume];
		/*values in table are in range of 0 ~ 36 dB.
		   shift to range of -36 ~ 0 dB in DSP */
		telephony_dl_gain_dB -= p->voice_volume_max;

	}

	/***
	OmegaVoice_Sysparm_t *omega_voice_parms = NULL;
	omega_voice_parms =
	AudParmP()[AUDCTRL_GetAudioMode()].omega_voice_parms;
	audio_control_generic(AUDDRV_CPCMD_SetOmegaVoiceParam,
		(UInt32)(&(omega_voice_parms[telephony_dl_gain_dB])),
		0, 0, 0, 0);
	***/

	/* if parm4 (OV_volume_step) is zero, volumectrl.c will calculate
	   OV volume step based on digital_gain_dB, VOICE_VOLUME_MAX and
	   NUM_SUPPORTED_VOLUME_LEVELS. */
	/* DSP accepts [-3600, 0] mB */
	/* pass negative number to CP:
	   at LMP int=>UInt32, then at CP UInt32=>int16 */

	audio_control_generic(AUDDRV_CPCMD_SetBasebandDownlinkGain,
			      telephony_dl_gain_dB * 100, 0, 0, 0, 0);

}

/****************************************************************************
*
* Function Name: AUDCTRL_GetTelephonySpkrVolume
*
* Description:   Set dl volume of telephony path
*
****************************************************************************/
int AUDCTRL_GetTelephonySpkrVolume(AUDIO_GAIN_FORMAT_t gain_format)
{
	if (gain_format == AUDIO_GAIN_FORMAT_mB)
		return telephony_dl_gain_dB * 100;

	return telephony_dl_gain_dB;
}

/****************************************************************************
*
* Function Name: AUDCTRL_SetTelephonySpkrMute
*
* Description:   mute/unmute the dl of telephony path
*
****************************************************************************/
void AUDCTRL_SetTelephonySpkrMute(AUDIO_SINK_Enum_t spk, Boolean mute)
{
	if (mute)
		audio_control_generic(AUDDRV_CPCMD_SetBasebandDownlinkMute, 0,
				      0, 0, 0, 0);
	else
		audio_control_generic(AUDDRV_CPCMD_SetBasebandDownlinkUnmute, 0,
				      0, 0, 0, 0);
}

/****************************************************************************
*
* Function Name: AUDCTRL_SetTelephonyMicGain
*
* Description:   Set ul gain of telephony path
*
****************************************************************************/
void AUDCTRL_SetTelephonyMicGain(AUDIO_SOURCE_Enum_t mic,
				 Int16 gain, AUDIO_GAIN_FORMAT_t gain_format)
{
	if (gain_format == AUDIO_GAIN_FORMAT_mB) {
		telephony_ul_gain_dB = gain / 100;

		aTrace(LOG_AUDIO_CNTLR, "%s gain = 0x%x\n", __func__, gain);

		audio_control_generic(AUDDRV_CPCMD_SetBasebandUplinkGain, gain,
				      0, 0, 0, 0);

		/* sysparm.c(4990):  pg1_mem->shared_echo_fast_NLP_gain[1]
   = SYSPARM_GetAudioParmAccessPtr()->audio_parm[currentAudioMode].
		   echoNlp_parms.echo_nlp_gain; */
		/* CP should load this parameter in AUDCTRL_SetAudioMode() */
	}
}

/****************************************************************************
*
* Function Name: AUDCTRL_SetTelephonyMicMute
*
* Description:   mute/unmute ul of telephony path
*
****************************************************************************/
void AUDCTRL_SetTelephonyMicMute(AUDIO_SOURCE_Enum_t mic, Boolean mute)
{
	aTrace(LOG_AUDIO_CNTLR,
			"AUDCTRL_SetTelephonyMicMute: mute = 0x%x", mute);

	if (mute) {
		bmuteVoiceCall = TRUE;
		/*pr_info(" bmuteVoiceCall = TRUE\r\n"); */
		audio_control_dsp(AUDDRV_DSPCMD_MUTE_DSP_UL, 0, 0, 0, 0, 0);
	} else {
		bmuteVoiceCall = FALSE;
		/*pr_info(" bmuteVoiceCall = FALSE\r\n"); */
		audio_control_dsp(AUDDRV_DSPCMD_UNMUTE_DSP_UL, 0, 0, 0, 0, 0);
	}
}

/****************************************************************************
*
* Function Name: AUDCTRL_GetAudioApp
*
* Description:   get audio application.
*
****************************************************************************/
AudioApp_t AUDCTRL_GetAudioApp(void)
{
	return currAudioApp;
}

/****************************************************************************
*
* Function Name: AUDCTRL_SetUserAudioApp
*
* Description:   set audio application.
*
* User space code call mixer control to use this function to set APP.
*
* Voice call APP is set by kernel auto-detection code
*
* User space code sets other APP (non voice call),
* what ever APP (except voice call APP) set from
* user space takes priority over the auto-detect and audto-set APP.
*
****************************************************************************/
void AUDCTRL_SetUserAudioApp(AudioApp_t app)
{
	aTrace(LOG_AUDIO_CNTLR, "SetUserAudioApp currAudioApp=%d new app=%d",
			currAudioApp, app);

	if (AUDCTRL_InVoiceCall())
		if (app > AUDIO_APP_VOICE_CALL_WB)
			return;

	/*AUDIO_APP_VOIP,
	   AUDIO_APP_VOIP_INCOMM,
	   and AUDIO_APP_RECORDING_GVS can only be set by user space code.
  This function allows user space to change APP away from the 3 APPs. */

	currAudioApp = app;
}

/****************************************************************************
*
* Function Name: AUDCTRL_SaveAudioApp
*
* Description:   save audio application.
*
* Kernel code can detect the start of use case and set audio APP.
* This function is used in kernel code to auto-detect and auto-set APP
*
****************************************************************************/
void AUDCTRL_SaveAudioApp(AudioApp_t app)
{
	aTrace(LOG_AUDIO_CNTLR, "%s currAudioApp=%d new app=%d", __func__,
			currAudioApp, app);
	if (AUDCTRL_InVoiceCall())
		if (app > AUDIO_APP_VOICE_CALL_WB && app != AUDIO_APP_LOOPBACK)
			return;

	/*AUDIO_APP_VOIP and AUDIO_APP_RECORDING_GVS can only be set by
	   user space code. kernel audio code can not detect them. */
	if (currAudioApp == AUDIO_APP_VOIP
	    || currAudioApp == AUDIO_APP_VOIP_INCOMM
	    || currAudioApp == AUDIO_APP_RECORDING_GVS
	    || currAudioApp == AUDIO_APP_FM)
		return;		/*keep user-set audio APP intact */

	currAudioApp = app;
}

/****************************************************************************
*
* Function Name: AUDCTRL_RemoveAudioApp
*
* Description:   indicate the audio use case ends and the app_id is not used.
*
****************************************************************************/
void AUDCTRL_RemoveAudioApp(AudioApp_t audio_app)
{

}

/*********************************************************************
*      Get current (voice call) audio mode
*      @return         mode            (voice call) audio mode
**********************************************************************/
AudioMode_t AUDCTRL_GetAudioMode(void)
{
	return currAudioMode;
}

/*********************************************************************
*      Save audio mode before call AUDCTRL_SaveAudioMode( )
*      @param          mode            (voice call) audio mode
*      @param          app             (voice call) audio app
*      @return         none
**********************************************************************/
void AUDCTRL_SaveAudioMode(AudioMode_t audio_mode)
{
	aTrace(LOG_AUDIO_CNTLR, "SaveAudioMode: mode = %d\n", audio_mode);

	currAudioMode = audio_mode;	/* update mode */
}

/*********************************************************************
*	Set (voice call) audio mode
*	@param          mode            (voice call) audio mode
*	@param          app     (voice call) audio app
*	@return         none
*
*	Actual App (profile) is also determined by network speech coder
*	sample rate and BT headset support of WB.
*	this function loads audio parameters to HW and DSP,
*	this function does not switch HW device.
**********************************************************************/
void AUDCTRL_SetAudioMode(AudioMode_t mode, AudioApp_t app)
{
	Boolean bClk = csl_caph_QueryHWClock();

	aTrace(LOG_AUDIO_CNTLR, "SetAudioMode: mode %d app %d", mode, app);

	if (AUDCTRL_InVoiceCall())
		if (app > AUDIO_APP_VOICE_CALL_WB && app != AUDIO_APP_LOOPBACK)
			return;

	AUDCTRL_SaveAudioMode(mode);

	if (app == AUDIO_APP_VOICE_CALL || app == AUDIO_APP_VOICE_CALL_WB) {
		if (AUDCTRL_Telephony_HW_16K(mode))
			app = AUDIO_APP_VOICE_CALL_WB;
		else
			app = AUDIO_APP_VOICE_CALL;
	}

	AUDCTRL_SaveAudioApp(app);

	if (!bClk)
		csl_caph_ControlHWClock(TRUE);
	/*enable clock if it is not enabled. */

	/* Here may need to consider for other apps like vt and voip etc */
	AUDDRV_SetAudioMode(mode, app,
		0,
		0,
		0); /*need pathID to find mixer input*/

	/*disable clock if it is enabled by this function */
	if (!bClk)
		csl_caph_ControlHWClock(FALSE);
}

/*********************************************************************
*	Set audio mode for music playback
*	@param          mode            audio mode for music playback
*	@return         none
*
**********************************************************************/
void AUDCTRL_SetAudioMode_ForMusicPlayback(AudioMode_t mode,
				   unsigned int arg_pathID, Boolean inHWlpbk)
{
	AUDIO_SOURCE_Enum_t mic;
	AUDIO_SINK_Enum_t spk;
	Boolean bClk = csl_caph_QueryHWClock();
	CSL_CAPH_HWConfig_Table_t *path = NULL;
	SetAudioMode_Sp_t sp_struct;

	aTrace(LOG_AUDIO_CNTLR,
			"%s mode %d, pathID %d",
			__func__, mode, arg_pathID);

	path = csl_caph_FindPath(arg_pathID);

	if (AUDCTRL_InVoiceCall()) {
		if (!path)
			return;	/*don't affect voice call audio mode */
		if (!path->srcmRoute[0][0].outChnl)
			return;
		/*if arm2sp does not use HW mixer, no need to set gain */
	}

	if (!path) {
		aTrace(LOG_AUDIO_CNTLR,
			"%s mode %d, pathID %d no path",
			__func__, mode, arg_pathID);
		return; /*don't know which mixer input */
	}

	if (!bClk)
		csl_caph_ControlHWClock(TRUE);
	/*enable clock if it is not enabled. */

	AUDCTRL_GetSrcSinkByMode(mode, &mic, &spk);

/*set PMU on/off, gain,
for multicast, need to find the other mode and reconcile on mixer gains.
 like BT + IHF
*/

	currAudioMode_playback = mode;
	/*currAudioApp_playback = ; */

	sp_struct.mode = mode;
	sp_struct.app = AUDCTRL_GetAudioApp();
	sp_struct.pathID = arg_pathID;
	sp_struct.inHWlpbk = inHWlpbk;
	if (muteInPlay) {
		sp_struct.mixInGain_mB = GAIN_NA;
		sp_struct.mixInGainR_mB = GAIN_NA;
	} else {
		sp_struct.mixInGain_mB = GAIN_SYSPARM;
		sp_struct.mixInGainR_mB = GAIN_SYSPARM;
	}
	{
		int i, j;

		i = AUDCTRL_GetAudioApp();
		j = mode;
		if (user_vol_setting[i][j].valid == FALSE)
			fillUserVolSetting(j, i);
		sp_struct.mixOutGain_mB = user_vol_setting[i][j].L;
		sp_struct.mixOutGainR_mB = user_vol_setting[i][j].R;
	}

	AUDDRV_SetAudioMode_Speaker(sp_struct);

	if (!AUDCTRL_InVoiceCall() && AUDDRV_TuningFlag()) {
		/*for music tuning, if PCG changed audio mode,
		   need to pass audio mode to CP in audio_vdriver_caph.c */
		audio_control_generic(AUDDRV_CPCMD_PassAudioMode,
				      (UInt32) mode,
				      (UInt32) AUDCTRL_GetAudioApp(), 0, 0, 0);
		/*this command updates mode in audioapi.c. */
		audio_control_generic(AUDDRV_CPCMD_SetAudioMode,
				      (UInt32) (((int)AUDCTRL_GetAudioApp()) *
						AUDIO_MODE_NUMBER + mode), 0, 0,
				      0, 0);
	}

	setExternAudioGain(mode, AUDCTRL_GetAudioApp());

	if (!bClk)
		csl_caph_ControlHWClock(FALSE);
	/*disable clock if it is enabled by this function. */
}

/*********************************************************************
*	Set audio mode for FM radio playback
*	@param          mode            audio mode
*	@return         none
*
**********************************************************************/
void AUDCTRL_SetAudioMode_ForFM(AudioMode_t mode,
				   unsigned int arg_pathID, Boolean inHWlpbk)
{
	AUDIO_SOURCE_Enum_t mic;
	AUDIO_SINK_Enum_t spk;
	Boolean bClk = csl_caph_QueryHWClock();
	CSL_CAPH_HWConfig_Table_t *path = NULL;
	SetAudioMode_Sp_t sp_struct;
	AudioApp_t app;

	aTrace(LOG_AUDIO_CNTLR,
			"%s mode %d, pathID %d", __func__, mode, arg_pathID);

	path = csl_caph_FindPath(arg_pathID);

	if (AUDCTRL_InVoiceCall()) {
		if (!path)
			return;	/*don't affect voice call audio mode */
		if (!path->srcmRoute[0][0].outChnl)
			return;
		/*if arm2sp does not use HW mixer, no need to set gain */
	}

	if (!bClk)
		csl_caph_ControlHWClock(TRUE);
	/*enable clock if it is not enabled. */

	AUDCTRL_GetSrcSinkByMode(mode, &mic, &spk);
	currAudioMode_fm = mode;

	app = AUDCTRL_GetAudioApp();
	sp_struct.mode = mode;
	sp_struct.app = app;
	sp_struct.pathID = arg_pathID;
	sp_struct.inHWlpbk = inHWlpbk;
	sp_struct.mixOutGain_mB = GAIN_SYSPARM;
	sp_struct.mixOutGainR_mB = GAIN_SYSPARM;
	if (users_gain[AUDPATH_FM].valid) {
		sp_struct.mixInGain_mB = users_gain[AUDPATH_FM].L;
		sp_struct.mixInGainR_mB = users_gain[AUDPATH_FM].R;
	} else {
		if (muteInPlay) {
			sp_struct.mixInGain_mB = GAIN_NA;
			sp_struct.mixInGainR_mB = GAIN_NA;
		} else {
			sp_struct.mixInGain_mB = GAIN_SYSPARM;
			sp_struct.mixInGainR_mB = GAIN_SYSPARM;
		}
	}
	if (user_vol_setting[app][mode].valid == FALSE)
		fillUserVolSetting(mode, app);
	sp_struct.mixOutGain_mB = user_vol_setting[app][mode].L;
	sp_struct.mixOutGainR_mB = user_vol_setting[app][mode].R;

	AUDDRV_SetAudioMode_Speaker(sp_struct);

	if (!AUDCTRL_InVoiceCall() && AUDDRV_TuningFlag()) {
		/*for music tuning, if PCG changed audio mode,
		   need to pass audio mode to CP in audio_vdriver_caph.c */
		audio_control_generic(AUDDRV_CPCMD_PassAudioMode,
				      (UInt32) mode,
				      (UInt32) app, 0, 0, 0);
		/*this command updates mode in audioapi.c. */
		audio_control_generic(AUDDRV_CPCMD_SetAudioMode,
				      (UInt32) ((int)app *
						AUDIO_MODE_NUMBER + mode), 0, 0,
				      0, 0);
	}

	setExternAudioGain(mode, app);

	if (!bClk)
		csl_caph_ControlHWClock(FALSE);
	/*disable clock if it is enabled by this function. */
}


/*********************************************************************
*	Set audio mode for music record
*	@param          mode            audio mode
*	@return         none
*
**********************************************************************/
void AUDCTRL_SetAudioMode_ForMusicRecord(
	AudioMode_t mode, unsigned int arg_pathID)
{
	AUDIO_SOURCE_Enum_t mic;
	AUDIO_SINK_Enum_t spk;
	Boolean bClk = csl_caph_QueryHWClock();

	aTrace(LOG_AUDIO_CNTLR, "%s mode = %d\n", __func__, mode);

	/*if( mode==AUDCTRL_GetAudioMode() )
	   return; */

	if (AUDCTRL_InVoiceCall()) {
		/*FM radio recording can co-exist with voice call */
		return;
		/*don't affect voice call audio mode */
	}

	if (!bClk)
		csl_caph_ControlHWClock(TRUE);
	/*enable clock if it is not enabled. */

	AUDCTRL_GetSrcSinkByMode(mode, &mic, &spk);

/*no PMU
for FM recording + voice call, need to find separate gains from sysparm
 and HW paths
also need to support audio profile (and/or mode) set from user space code
 to support multi-profile/app.
*/

	currAudioMode_record = mode;

	AUDDRV_SetAudioMode_Mic(mode, AUDCTRL_GetAudioApp(),
		arg_pathID, 0);

	if (!bClk)
		csl_caph_ControlHWClock(FALSE);
	/*disable clock if it is enabled by this function. */
}

#ifdef CONFIG_ENABLE_SSMULTICAST
/*********************************************************************
*	Set audio mode for music multicast to IHF+HS
*	@param          mode            audio mode for music playback
*	@return         none
*
**********************************************************************/
void AUDCTRL_SetAudioMode_ForMusicMulticast(AudioMode_t mode)
{
	Boolean bClk = csl_caph_QueryHWClock();
	SetAudioMode_Sp_t sp_struct;

	aTrace(LOG_AUDIO_CNTLR,
			"%s mode %d", __func__, mode);

	if (AUDCTRL_InVoiceCall())
		return;	/*don't affect voice call audio mode */

	if (!bClk)
		csl_caph_ControlHWClock(TRUE);
	/*enable clock if it is not enabled. */

	/*For SS Multicast to IHF+HS, mode will be always Speakerphone*/
	currAudioMode_playback = AUDIO_MODE_SPEAKERPHONE;

	switch (currAudioMode) {

	case AUDIO_MODE_HEADSET:
	if (mode == AUDIO_MODE_SPEAKERPHONE) {
		/*adding IHF reload HS param*/
		AUDDRV_SetAudioMode_Multicast(
			AUDIO_MODE_HEADSET, AUDCTRL_GetAudioApp()
			);

		/*Load HS params from mode RESERVED*/
		setExternAudioGain(AUDIO_MODE_RESERVE, AUDCTRL_GetAudioApp());

		AUDDRV_SetAudioMode_Multicast(mode, AUDCTRL_GetAudioApp());
	}
	break;

	case AUDIO_MODE_SPEAKERPHONE:
	if (mode == AUDIO_MODE_HEADSET) {
		/*Adding HS load HS param*/
		AUDDRV_SetAudioMode_Multicast(
			mode, AUDCTRL_GetAudioApp());
		/*Load HS params from mode RESERVED*/
		setExternAudioGain(AUDIO_MODE_RESERVE, AUDCTRL_GetAudioApp());
	}
	break;

	/*Multicasting to BT+IHF or any other
	right now for any BT+IHF case we will not end up here*/
	default:

	sp_struct.mode = mode;
	sp_struct.app = AUDCTRL_GetAudioApp();
	sp_struct.pathID = 0;
	sp_struct.inHWlpbk = FALSE;
	AUDDRV_SetAudioMode_Speaker(sp_struct); /* need pathID */
	currAudioMode_playback = mode;
	break;
	} /*end of switch (currAudioMode)*/

	AUDCTRL_SaveAudioMode(AUDIO_MODE_SPEAKERPHONE);

	if (!bClk)
		csl_caph_ControlHWClock(FALSE);
	/*disable clock if it is enabled by this function. */
}
#endif

/*********************************************************************
*Description :
*	Get sink and source device by audio mode
*Parameters
*	mode -- audio mode
*	pMic -- Source device coresponding to audio mode
*	pSpk -- Sink device coresponding to audio mode
*Return   none
**********************************************************************/
void AUDCTRL_GetSrcSinkByMode(AudioMode_t mode, AUDIO_SOURCE_Enum_t *pMic,
			      AUDIO_SINK_Enum_t *pSpk)
{
	switch (mode) {
	case AUDIO_MODE_HANDSET:
	case AUDIO_MODE_HAC:
		*pMic = AUDIO_SOURCE_ANALOG_MAIN;
		*pSpk = AUDIO_SINK_HANDSET;
		break;

	case AUDIO_MODE_HEADSET:
	case AUDIO_MODE_TTY:
		*pMic = AUDIO_SOURCE_ANALOG_AUX;
		*pSpk = AUDIO_SINK_HEADSET;
		break;

	case AUDIO_MODE_BLUETOOTH:
		*pMic = AUDIO_SOURCE_BTM;
		*pSpk = AUDIO_SINK_BTM;
		break;

	case AUDIO_MODE_SPEAKERPHONE:
		*pMic = AUDIO_SOURCE_ANALOG_MAIN;
		*pSpk = AUDIO_SINK_LOUDSPK;
		break;

	default:
		aTrace(LOG_AUDIO_CNTLR,
				"AUDCTRL_GetSrcSinkByMode()"
				"mode %d is out of range\n",
				mode);
		break;
	}
}

/****************************************************************************
*
* Function Name: AUDCTRL_EnablePlay
*
* Description:   enable a playback path
*
****************************************************************************/
void AUDCTRL_EnablePlay(AUDIO_SOURCE_Enum_t source,
			AUDIO_SINK_Enum_t sink,
			AUDIO_NUM_OF_CHANNEL_t numCh,
			AUDIO_SAMPLING_RATE_t sr, unsigned int *pPathID)
{
	CSL_CAPH_HWCTRL_CONFIG_t config;
	CSL_CAPH_PathID pathID;
	AudioMode_t mode = AUDIO_MODE_HANDSET;
	int ret = 0;

	aTrace(LOG_AUDIO_CNTLR, "%s src %d, sink %d\n", __func__, source, sink);
	pathID = 0;
	memset(&config, 0, sizeof(CSL_CAPH_HWCTRL_CONFIG_t));

	/* Enable the path. And get path ID. */
	config.streamID = CSL_CAPH_STREAM_NONE;
	config.pathID = 0;
	config.source = getDeviceFromSrc(source);
	config.sink = getDeviceFromSink(sink);
	config.dmaCH = CSL_CAPH_DMA_NONE;
	config.src_sampleRate = sr;
	/* For playback, sample rate should be 48KHz. */
	config.snk_sampleRate = AUDIO_SAMPLING_RATE_48000;
	config.chnlNum = numCh;
	config.bitPerSample = 16;

	/*save audio for powerOnExternalAmp() to use. */
	mode = GetAudioModeBySink(sink);
	AUDCTRL_SaveAudioApp(AUDIO_APP_MUSIC);
	AUDCTRL_SaveAudioMode(mode);

	if (source == AUDIO_SOURCE_I2S && AUDCTRL_InVoiceCall() == FALSE) {
		aTrace(LOG_AUDIO_CNTLR,
				"%s FM src %d, sink %d", __func__,
				source, sink);

		AUDCTRL_SaveAudioApp(AUDIO_APP_FM);
		powerOnExternalAmp(sink, FmUse,
				TRUE, FALSE);
	} else {
		AUDCTRL_SaveAudioApp(AUDIO_APP_MUSIC);
		powerOnExternalAmp(sink, AudioUse,
				TRUE, FALSE);
	}

	if (user_vol_setting[AUDCTRL_GetAudioApp()][mode].valid == FALSE)
		fillUserVolSetting(mode, AUDCTRL_GetAudioApp());

	if (source == AUDIO_SOURCE_DSP && sink == AUDIO_SINK_USB) {
		/*USB call */
		config.source = CSL_CAPH_DEV_DSP;
		config.sink = CSL_CAPH_DEV_MEMORY;
	}

	if ((source == AUDIO_SOURCE_MEM || source == AUDIO_SOURCE_I2S)
	    && sink == AUDIO_SINK_DSP) {
		config.sink = CSL_CAPH_DEV_DSP_throughMEM;
	}

	if ((source != AUDIO_SOURCE_DSP && sink == AUDIO_SINK_USB)
	    || sink == AUDIO_SINK_BTS)
		;
	else
		pathID = csl_caph_hwctrl_EnablePath(config);

	/*Load the speaker gains form sysparm.
	   Can not call this following API here.
	   Because Render driver really enable the path.
	   AUDCTRL_LoadSpkrGain(pathID, spk, FALSE); */

	/* Enable DSP DL */
	if (config.source == CSL_CAPH_DEV_DSP
	    || config.sink == CSL_CAPH_DEV_DSP_throughMEM) {
		if (bInVoiceCall != TRUE)
			AUDDRV_EnableDSPOutput(sink, sr);
	}

	/*to set HW mixer gain for Music playback */
	mode = AUDCTRL_GetModeBySpeaker(config.sink);
	if (source == AUDIO_SOURCE_I2S && AUDCTRL_InVoiceCall() == FALSE) {
		AUDCTRL_SetAudioMode_ForFM(mode, pathID, FALSE);

		fmPlayStarted = TRUE;
	} else
		AUDCTRL_SetAudioMode_ForMusicPlayback(mode, pathID, FALSE);

	if (sink == AUDIO_SINK_VIBRA) {
		if (!vibra_reg) {
			vibra_reg = regulator_get(NULL, VIBRA_LDO_REGULATOR);

			if (IS_ERR(vibra_reg))
				aError("Failed to get LDO for Vibra\n");
		}
		if (vibra_reg) {
			ret = regulator_enable(vibra_reg);
			if (ret != 0)
				aError("Failed to enable LDO for Vibra: %d\n",
					ret);
		}
	}

	if (pPathID)
		*pPathID = pathID;

	playbackPathID = pathID;
	pathIDTuning = pathID;

}

/****************************************************************************
*
* Function Name: AUDCTRL_DisablePlay
*
* Description:   disable a playback path
*
****************************************************************************/
void AUDCTRL_DisablePlay(AUDIO_SOURCE_Enum_t source,
			 AUDIO_SINK_Enum_t sink, unsigned int pathID)
{
	CSL_CAPH_HWCTRL_CONFIG_t config;
	CSL_CAPH_PathID path = 0;
	CSL_CAPH_DEVICE_e src_dev, sink_dev;
	int ret = 0;

	memset(&config, 0, sizeof(CSL_CAPH_HWCTRL_CONFIG_t));
	aTrace(LOG_AUDIO_CNTLR, "%s src %d, sink %d, pathID %d",
		__func__, source, sink, pathID);

	if (pathID == 0) {
		audio_xassert(0, pathID);
		return;
	}

	if (source == AUDIO_SOURCE_DSP && sink == AUDIO_SINK_USB) {
		/*USB call */
		config.source = CSL_CAPH_DEV_DSP;
		config.sink = CSL_CAPH_DEV_MEMORY;
	}

	src_dev = getDeviceFromSrc(source);
	sink_dev = getDeviceFromSink(sink);

	/* Disable DSP DL */
	if (source == AUDIO_SOURCE_DSP || sink == AUDIO_SINK_DSP)
		/* if bInVoiceCall== TRUE, assume the telphony_deinit() function
		   sends DISABLE */
		if (bInVoiceCall != TRUE)
			AUDDRV_DisableDSPOutput();

	if ((source != AUDIO_SOURCE_DSP && sink == AUDIO_SINK_USB)
	    || sink == AUDIO_SINK_BTS)
		;
	else {
		config.pathID = pathID;
		(void)csl_caph_hwctrl_DisablePath(config);
	}

	/* Need CSL API to obtain the pathID from the same speaker info.
	   This is to make sure that PMU is not disabled if any other
	   path is using the same speaker */

	path = csl_caph_FindPathID(sink_dev, src_dev);

	/*Disable the PMU for HS/IHF */
	if (path) {
		aTrace(LOG_AUDIO_CNTLR,
				"%s pathID %d use the ext amp.", __func__,
				path);
	} else {
		if (source == AUDIO_SOURCE_I2S
		&& AUDCTRL_InVoiceCall() == FALSE) {
			aTrace(LOG_AUDIO_CNTLR,
					"%s FM src %d, sink %d",
					__func__, source, sink);

			powerOnExternalAmp(sink, FmUse, FALSE, FALSE);
			fmPlayStarted = FALSE;
			AUDCTRL_RemoveAudioApp(AUDIO_APP_FM);
		} else if ((sink == AUDIO_SINK_HEADSET)
			   || (sink == AUDIO_SINK_LOUDSPK)) {
			powerOnExternalAmp(sink, AudioUse, FALSE, FALSE);
			AUDCTRL_RemoveAudioApp(AUDIO_APP_MUSIC);
		}
	}

	if (sink == AUDIO_SINK_VIBRA) {
		if (vibra_reg) {
			ret = regulator_disable(vibra_reg);
			if (ret != 0)
				aError("Failed to disable LDO for Vibra: %d\n",
					ret);
			regulator_put(vibra_reg);
			vibra_reg = NULL;
		}
	}
	pathIDTuning = 0;
}

/****************************************************************************
*
*  Function Name: AUDCTRL_StartRender
*
*  Description: Start the data transfer of audio path render
*
****************************************************************************/
Result_t AUDCTRL_StartRender(unsigned int streamID)
{
	AudioMode_t mode = AUDIO_MODE_HANDSET;
	Result_t res;
	CSL_CAPH_HWConfig_Table_t *path = NULL;
	int pathID = 0;

	aTrace(LOG_AUDIO_CNTLR, "%s streamID=0x%x", __func__, streamID);

	res = csl_audio_render_start(streamID);

	path = csl_caph_FindRenderPath(streamID);
	if (path == 0)
		return RESULT_OK;

	if (path->status == PATH_OCCUPIED)
		mode = AUDCTRL_GetModeBySpeaker(path->sink[0]);

	aTrace(LOG_AUDIO_CNTLR,
		"%s path->status=%d, path->sink[0]=%d,"
		"mode=%d, mixer in 0x%x out: %d",
		__func__, path->status, path->sink[0], mode,
		path->srcmRoute[0][0].inChnl,
		path->srcmRoute[0][0].outChnl);

	if (mode == AUDIO_MODE_RESERVE)
		return RESULT_OK;	/*no need to set HW gain for FM TX. */

	AUDCTRL_SaveAudioApp(AUDIO_APP_MUSIC);

	if (AUDCTRL_InVoiceCall() && path->srcmRoute[0][0].outChnl)
		pathID = path->pathID;
	/*arm2sp may use HW mixer, whose gain should be set */
	AUDCTRL_SetAudioMode_ForMusicPlayback(mode, pathID, FALSE);

	return RESULT_OK;
}

/****************************************************************************
*
*  Function Name: AUDCTRL_StopRender
*
*  Description: Stop the data transfer of audio path render
*
****************************************************************************/
Result_t AUDCTRL_StopRender(unsigned int streamID)
{
	Result_t res;

	aTrace(LOG_AUDIO_CNTLR, "AUDCTRL_StopRender::streamID=0x%x\n",
			streamID);
	res = csl_audio_render_stop(streamID);
	AUDCTRL_RemoveAudioApp(AUDIO_APP_MUSIC);

	return res;
}

/****************************************************************************
*
* Function Name: AUDCTRL_SetPlayVolume
*
* Description:   set volume of a playback path
*
****************************************************************************/
void AUDCTRL_SetPlayVolume(AUDIO_SOURCE_Enum_t source,
			   AUDIO_SINK_Enum_t sink,
			   AUDIO_GAIN_FORMAT_t gain_format,
			   int vol_left, int vol_right, unsigned int pathID)
{
	/* left_channel in stereo, or mono: Register value. */
	static int mixInGain, mixOutGain, mixBitSel, extGain;

	/* for right channel in stereo: Register value. */
	static int mixInGain_r, mixOutGain_r, mixBitSel_r, extGain_r;

	CSL_CAPH_DEVICE_e speaker = CSL_CAPH_DEV_NONE;
	CSL_CAPH_MIXER_e mixer = CSL_CAPH_SRCM_CH_NONE;
	CSL_CAPH_SRCM_INCHNL_e mixInCh = CSL_CAPH_SRCM_INCHNL_NONE;
	AudioMode_t mode;
	AudioApp_t app;

#ifdef CONFIG_BCM_MODEM
	SysAudioParm_t *p;
#else
	AudioSysParm_t *p;
#endif

	mode = GetAudioModeBySink(sink);
	app = AUDCTRL_GetAudioApp();
	p = &(AudParmP()[mode + app * AUDIO_MODE_NUMBER]);

	if ((source != AUDIO_SOURCE_DSP && sink == AUDIO_SINK_USB)
	    || sink == AUDIO_SINK_BTS)
		return;

	/*determine hardware gains */

	/***
	MixerIn Gain:	SRC_M1D0_CH**_GAIN_CTRL : SRC_M1D0_CH**_TARGET_GAIN
	sfix<16,2>
	0x7FFF = 6 dB
	0x4000 (2 to the power of 14), =1.0, =0 dB
	0x0000 the input path is essentially switched off

	MixerOutFineGain:	SRC_SPK0_LT_GAIN_CTRL2 : SPK0_LT_FIXED_GAIN
	13-bit interger unsigned
	0x0000, = 0 dB
	0x0001, = (6.02/256) dB attenuation ~ 0.0235 dB
	0x1FFF	max attenuation

	MixerBitSelect:  SRC_SPK0_LT_GAIN_CTRL1 : SPK0_LT_BIT_SELECT
	3-bit unsigned
	***/

	if (gain_format == AUDIO_GAIN_FORMAT_FM_RADIO_DIGITAL_VOLUME_TABLE) {
		/*in idle mode, this command
		   alsa_amixer cset name=FM-VOL-LEVEL 12,12
		   passes down the pathID==0.
		   the FM's actual pathID is alloc'ed when enable FM,
		   and could be non 0. */

		users_gain[AUDPATH_FM].valid = TRUE;
		users_gain[AUDPATH_FM].L = p->fm_radio_digital_vol[vol_left];
		/*users_gain[AUDPATH_FM].R =
		p->fm_radio_digital_vol[vol_right];*/
		/*vol_right is always 0. need to fix it in caph_ctl.c*/
		users_gain[AUDPATH_FM].R = p->fm_radio_digital_vol[vol_left];

		/*if( fmPlayStarted == FALSE ) */
		/*if ( path->status != PATH_OCCUPIED ) */
		if (FALSE == csl_caph_QueryHWClock()) {
			/*the CAPH clock may be not turned on.
			   defer setting the FM radio audio gain until start
			   render.
			 */
			return;
		}

		speaker = getDeviceFromSink(sink);
		mixer = csl_caph_FindMixer(speaker, pathID);
		/*determine which mixer input to apply the gains to */
		/*is the inChnl stereo two channels? */
		mixInCh = csl_caph_FindMixInCh(speaker, pathID);

		if (mixInCh != CSL_CAPH_SRCM_INCHNL_NONE)
			csl_srcmixer_setMixInGain(mixInCh, mixer,
				users_gain[AUDPATH_FM].L,
				users_gain[AUDPATH_FM].R);
		else {
			aError("AUDCTRL_SetPlayVolume no mixer input!\n");
			audio_xassert(0, pathID);
		}

		return;

	}

	if (gain_format == AUDIO_GAIN_FORMAT_mB) {
		user_vol_setting[app][mode].L = vol_left;
		user_vol_setting[app][mode].R = vol_right;
		user_vol_setting[app][mode].valid = TRUE;

		extGain = (short)p->ext_speaker_pga_l;	/*Q13p2 dB */
		extGain = extGain * 25;	/*mB */
		extGain_r = (short)p->ext_speaker_pga_r;	/*Q13p2 dB */
		extGain_r = extGain_r * 25;	/*mB */

		switch (sink) {
		case AUDIO_SINK_HEADSET:
		case AUDIO_SINK_TTY:
			/*case AUDIO_CHNL_HEADPHONE_NO_MIC: */
#if 1
			/***** fix PMU gain, adjust CAPH gain **/
			extern_hs_set_gain(extGain, AUDIO_HS_BOTH);
#else
			/***** adjust PMU gain, adjust CAPH gain **/
			/*determine actual PMU gain in mB: */
			extGain = extern_hs_find_gain(vol_left);
#endif
			break;

		case AUDIO_SINK_LOUDSPK:
#if 1
			/***** fixed PMU gain, adjust CAPH gain **/
			extern_ihf_set_gain(extGain);
#else
			/***** adjust PMU gain, adjust CAPH gain **/
			/*determine actual PMU gain in mB: */
			extGain = extern_ihf_find_gain(vol_left);
#endif
			break;

		default:
			extGain = 0;
			extGain_r = 0;
			break;
		}

		/*set CAPH gain */
		mixInGain = (short)p->srcmixer_input_gain_l;	/*Q13p2 dB */
		mixInGain = mixInGain * 25; /* into mB */
		mixInGain_r = (short)p->srcmixer_input_gain_r;	/*Q13p2 dB */
		mixInGain_r = mixInGain_r * 25; /* into mB */

		if (vol_left >= 4214) {
			mixBitSel = 7;
			mixOutGain = 0;
		} else if (vol_left > 0) {	/*0~4213 */
			mixBitSel = vol_left / 602;
			mixBitSel += 1;
			/*since fine-gain is only an attenuation,
			   round up to the next bit shift */
			mixOutGain = vol_left - (mixBitSel * 602);
			/*put in attenuation, negative number. */
		} else if (vol_left <= 0) {
			mixBitSel = (short)p->srcmixer_output_coarse_gain_l;
			mixBitSel = mixBitSel / 24; /* bit_shift */
			mixOutGain = vol_left;
		}

		if (vol_right >= 4214) {
			mixBitSel_r = 7;
			mixOutGain_r = 0;
		} else if (vol_right > 0) {	/* 0~4213 */
			mixBitSel_r = vol_right / 602;
			mixBitSel_r += 1;
			/*since fine-gain is only an attenuation,
			   round up to the next bit shift */
			mixOutGain_r = vol_right - (mixBitSel_r * 602);
			/*put in attenuation, negative number. */
		} else if (vol_right <= 0) {
			mixBitSel_r = (short)p->srcmixer_output_coarse_gain_r;
			mixBitSel_r = mixBitSel_r / 24; /* bit_shift */
			mixOutGain_r = vol_right;
		} else {	/*vol__right < 0 */
			mixBitSel_r = 0;
			/*put in attenuation, negative number. */
		}
	} else {
		return;
	}

	/*determine which mixer output to apply the gains to */

	aTrace(LOG_AUDIO_CNTLR, "%s extGain %d\n", __func__, extGain);
	aTrace(LOG_AUDIO_CNTLR, "mixInGain %d, mixInGain_r %d,"
		" mixOutGain %d, mixOutGain_r %d,"
		" mixBitSel %d\n, mixBitSel_r %d\n",
		mixInGain, mixInGain_r,
		mixOutGain, mixOutGain_r,
		mixBitSel, mixBitSel_r);
	/*
	aTrace(LOG_AUDIO_CNTLR, "
	sink %d, gain_format %d\n",	sink, gain_format);
	aTrace(LOG_AUDIO_CNTLR, "vol_left 0x%x
	vol_right 0x%x\n", vol_left, vol_right);
	*/

	speaker = getDeviceFromSink(sink);
	mixer = csl_caph_FindMixer(speaker, pathID);
	/*determine which mixer input to apply the gains to */
	/*is the inChnl stereo two channels? */
	mixInCh = csl_caph_FindMixInCh(speaker, pathID);

	if (mixInCh != CSL_CAPH_SRCM_INCHNL_NONE)
		csl_srcmixer_setMixInGain(mixInCh, mixer,
			mixInGain, mixInGain_r);
	/*else
	do not know which mixer input channel to apply on
		csl_srcmixer_setMixAllInGain(mixer, mixInGain, mixInGain);*/

	csl_srcmixer_setMixOutGain(mixer, mixOutGain, mixOutGain_r);

	csl_srcmixer_setMixBitSel(mixer, mixBitSel, mixBitSel_r);

	return;
}

/****************************************************************************
*
* Function Name: AUDCTRL_SetPlayMute
*
* Description:   mute/unmute a playback path
*
****************************************************************************/
void AUDCTRL_SetPlayMute(AUDIO_SOURCE_Enum_t source,
			 AUDIO_SINK_Enum_t sink,
			 Boolean mute, unsigned int pathID)
{
	CSL_CAPH_DEVICE_e speaker = CSL_CAPH_DEV_NONE;
	CSL_CAPH_MIXER_e mixer = CSL_CAPH_SRCM_CH_NONE;
	CSL_CAPH_SRCM_INCHNL_e mixInCh = CSL_CAPH_SRCM_INCHNL_NONE;
	AudioMode_t mode;
	AudioApp_t app;
	int mixInGain, mixInGainR;
#ifdef CONFIG_BCM_MODEM
	SysAudioParm_t *p;
#else
	AudioSysParm_t *p;
#endif

	mode = GetAudioModeBySink(sink);
	app = AUDCTRL_GetAudioApp();
	p = &(AudParmP()[mode + app * AUDIO_MODE_NUMBER]);

	aTrace(LOG_AUDIO_CNTLR, "%s sink 0x%x, source 0x%x, mute 0x%x",
			__func__, sink, source, mute);

	if ((source != AUDIO_SOURCE_DSP && sink == AUDIO_SINK_USB)
	    || sink == AUDIO_SINK_BTS)
		return;

	speaker = getDeviceFromSink(sink);
	mixer = csl_caph_FindMixer(speaker, pathID);
	/*determine which mixer input to apply the gains to */
	/*is the inChnl stereo two channels? */
	mixInCh = csl_caph_FindMixInCh(speaker, pathID);

	/*if(pathID == 0)
	   {
	   audio_xassert(0,pathID);
	   return;
	   } */

	if (mute == TRUE) {
		if (sink == AUDIO_SINK_VIBRA)
			csl_caph_hwctrl_MuteSink(0 /*pathID */ , speaker);
		else {
			if (mixInCh != CSL_CAPH_SRCM_INCHNL_NONE) {
				csl_srcmixer_setMixInGain(mixInCh, mixer,
					GAIN_NA, GAIN_NA);
				muteInPlay = TRUE;
			}
		}
	} else {
		if (sink == AUDIO_SINK_VIBRA)
			csl_caph_hwctrl_UnmuteSink(0 /*pathID */ , speaker);
		else {
			if (mixInCh != CSL_CAPH_SRCM_INCHNL_NONE) {
				/*set CAPH gain, Q13p2 dB */
				mixInGain = (short)p->srcmixer_input_gain_l;
				mixInGain = mixInGain * 25; /* into mB */
				mixInGainR = (short)p->srcmixer_input_gain_r;
				mixInGainR = mixInGainR * 25; /* into mB */
				csl_srcmixer_setMixInGain(mixInCh, mixer,
					mixInGain, mixInGainR);
				muteInPlay = FALSE;
			}
		}
	}

	return;
}

/****************************************************************************
*
* Function Name: AUDCTRL_SwitchPlaySpk
*
* Description: switch a speaker to a playback path
* suggested to use when a single device is in associated with single path.
* For multicasting use cases, use Add/Remove Spk API
*
****************************************************************************/
void AUDCTRL_SwitchPlaySpk(AUDIO_SOURCE_Enum_t source,
			   AUDIO_SINK_Enum_t sink, unsigned int pathID)
{
	CSL_CAPH_HWCTRL_CONFIG_t config;
	CSL_CAPH_DEVICE_e curr_spk = CSL_CAPH_DEV_NONE;
	CSL_CAPH_DEVICE_e new_spk = getDeviceFromSink(sink);

	aTrace(LOG_AUDIO_CNTLR,
			"%s src 0x%x, Sink 0x%x", __func__, source, sink);
	if (pathID == 0) {
		audio_xassert(0, pathID);
		return;
	}

	curr_spk = csl_caph_FindSinkDevice(pathID);

	if (curr_spk == new_spk) {
		aTrace(LOG_AUDIO_CNTLR,
				"%s same speaker %d. ignored",
				__func__, curr_spk);
		return;
	}

	if (source == AUDIO_SOURCE_I2S && AUDCTRL_InVoiceCall() == FALSE
		&& fmPlayStarted == TRUE) {
		if (curr_spk == CSL_CAPH_DEV_HS)
			powerOnExternalAmp(AUDIO_SINK_HEADSET, FmUse,
					   FALSE, FALSE);
		else if (curr_spk == CSL_CAPH_DEV_IHF)
			powerOnExternalAmp(AUDIO_SINK_LOUDSPK, FmUse,
					   FALSE, FALSE);
	} else {
		if (curr_spk == CSL_CAPH_DEV_HS)
			powerOnExternalAmp(AUDIO_SINK_HEADSET, AudioUse,
					   FALSE, FALSE);
		else if (curr_spk == CSL_CAPH_DEV_IHF)
			powerOnExternalAmp(AUDIO_SINK_LOUDSPK, AudioUse,
					   FALSE, FALSE);
	}

	/* add new spk first... */
	if (getDeviceFromSink(sink) != CSL_CAPH_DEV_NONE) {
		config.source = getDeviceFromSrc(source);
		config.sink = getDeviceFromSink(sink);
		(void)csl_caph_hwctrl_AddPath(pathID, config);
	}
	/* remove current spk */
	if (curr_spk != CSL_CAPH_DEV_NONE) {
		config.source = getDeviceFromSrc(source);
		config.sink = curr_spk;
		(void)csl_caph_hwctrl_RemovePath(pathID, config);
	}

	if (source == AUDIO_SOURCE_I2S && AUDCTRL_InVoiceCall() == FALSE
		&& fmPlayStarted == TRUE) {
		if (sink == AUDIO_SINK_LOUDSPK || sink == AUDIO_SINK_HEADSET)
			powerOnExternalAmp(sink, FmUse, TRUE, FALSE);

		AUDCTRL_SetAudioMode_ForFM(
			GetAudioModeBySink(sink), pathID, FALSE);
	} else {
		if (sink == AUDIO_SINK_LOUDSPK || sink == AUDIO_SINK_HEADSET)
			powerOnExternalAmp(sink, AudioUse, TRUE, FALSE);

		AUDCTRL_SetAudioMode_ForMusicPlayback(
			GetAudioModeBySink(sink), pathID, FALSE);

	}

	return;
}

/****************************************************************************
*
* Function Name: AUDCTRL_SwitchPlaySpk_forTuning
*
* Description: switch a speaker to a playback path
* suggested to use when a single device is in associated with single path.
* For multicasting use cases, use Add/Remove Spk API
*
****************************************************************************/
void AUDCTRL_SwitchPlaySpk_forTuning(AudioMode_t mode)
{
	AUDIO_SOURCE_Enum_t mic;
	AUDIO_SINK_Enum_t spk;

	if (AUDCTRL_InVoiceCall() == TRUE)
		return;		/* do not affect voice call */

	if (!csl_caph_QueryHWClock())
		return;		/* no clock, not playing. */

	if (pathIDTuning == 0) {
		/*no playback path */
		return;
	}

	AUDCTRL_GetSrcSinkByMode(mode, &mic, &spk);

	if (AUDCTRL_GetAudioApp() == AUDIO_APP_MUSIC)
		AUDCTRL_SwitchPlaySpk(mic, spk, pathIDTuning);

	return;
}

/****************************************************************************
*
* Function Name: AUDCTRL_AddPlaySpk
*
* Description:   add a speaker to a playback path
*
****************************************************************************/
void AUDCTRL_AddPlaySpk(AUDIO_SOURCE_Enum_t source,
			AUDIO_SINK_Enum_t sink, unsigned int pathID)
{
	CSL_CAPH_HWCTRL_CONFIG_t config;
	CSL_CAPH_DEVICE_e speaker = CSL_CAPH_DEV_NONE;

	aTrace(LOG_AUDIO_CNTLR, "%s src %d, sink %d,"
			"pathID %d", __func__, source, sink, pathID);

	/*if(pathID == 0)
	   {
	   audio_xassert(0,pathID);
	   return;
	   } */

	speaker = getDeviceFromSink(sink);
	if (speaker != CSL_CAPH_DEV_NONE) {
		/*Enable the PMU for HS/IHF. */
		if ((sink == AUDIO_SINK_LOUDSPK)
		    || (sink == AUDIO_SINK_HEADSET))
			powerOnExternalAmp(sink, AudioUse, TRUE, FALSE);

		config.source = getDeviceFromSrc(source);
		config.sink = speaker;
		(void)csl_caph_hwctrl_AddPath(pathID, config);
	}
#ifndef CONFIG_ENABLE_SSMULTICAST
	if (currAudioApp == AUDIO_APP_FM) {
		if (isStIHF &&
			currAudioMode_fm == AUDIO_MODE_SPEAKERPHONE &&
			sink == AUDIO_SINK_HANDSET)
			AUDCTRL_SetAudioMode_ForFM(
				AUDIO_MODE_SPEAKERPHONE, pathID, FALSE);
		else
			AUDCTRL_SetAudioMode_ForFM(
				GetAudioModeBySink(sink), pathID, FALSE);
	} else {
		if (isStIHF &&
			currAudioMode_playback == AUDIO_MODE_SPEAKERPHONE &&
			sink == AUDIO_SINK_HANDSET)
				AUDCTRL_SetAudioMode_ForMusicPlayback(
				AUDIO_MODE_SPEAKERPHONE, pathID, FALSE);
		else
			AUDCTRL_SetAudioMode_ForMusicPlayback(
				GetAudioModeBySink(sink), pathID, FALSE);
	}
#else
	AUDCTRL_SetAudioMode_ForMusicMulticast(
		GetAudioModeBySink(sink));
#endif
	return;

}

/****************************************************************************
*
* Function Name: AUDCTRL_RemovePlaySpk
*
* Description:   remove a speaker to a playback path
*
****************************************************************************/
void AUDCTRL_RemovePlaySpk(AUDIO_SOURCE_Enum_t source,
			   AUDIO_SINK_Enum_t sink, unsigned int pathID)
{
	CSL_CAPH_HWCTRL_CONFIG_t config;
	CSL_CAPH_DEVICE_e speaker = CSL_CAPH_DEV_NONE;
#ifdef CONFIG_ENABLE_SSMULTICAST
	SetAudioMode_Sp_t sp_struct;
#endif

	aTrace(LOG_AUDIO_CNTLR, "%s src 0x%x, sink 0x%x", __func__,
			source, sink);

	if (pathID == 0) {
		audio_xassert(0, pathID);
		return;
	}
	speaker = getDeviceFromSink(sink);
	if (speaker != CSL_CAPH_DEV_NONE) {
		/*Disable the PMU for HS/IHF. */
		if ((sink == AUDIO_SINK_LOUDSPK)
		    || (sink == AUDIO_SINK_HEADSET))
			powerOnExternalAmp(sink, AudioUse, FALSE, FALSE);

		config.source = getDeviceFromSrc(source);
		config.sink = speaker;
		(void)csl_caph_hwctrl_RemovePath(pathID, config);

#ifdef CONFIG_ENABLE_SSMULTICAST
	/*If IHF removed reload HS params with mode HEADSET*/
	if (sink == AUDIO_SINK_LOUDSPK &&
		currAudioMode_playback == AUDIO_MODE_SPEAKERPHONE) {

		sp_struct.mode = AUDIO_MODE_HEADSET;
		sp_struct.app = AUDCTRL_GetAudioApp();
		sp_struct.pathID = 0;
		sp_struct.inHWlpbk = FALSE;
		AUDDRV_SetAudioMode_Speaker(sp_struct);
		setExternAudioGain(AUDIO_MODE_HEADSET, AUDCTRL_GetAudioApp());
		AUDCTRL_SaveAudioMode(AUDIO_MODE_HEADSET);
	}
#endif
	}
	return;
}

/****************************************************************************
*
* Function Name: AUDCTRL_EnableRecordMono
*
* Description:   enable a record path for single mic
*
****************************************************************************/
static void AUDCTRL_EnableRecordMono(AUDIO_SOURCE_Enum_t source,
				     AUDIO_SINK_Enum_t sink,
				     AUDIO_NUM_OF_CHANNEL_t numCh,
				     AUDIO_SAMPLING_RATE_t sr,
				     unsigned int *pPathID)
{
	CSL_CAPH_HWCTRL_CONFIG_t config;
	CSL_CAPH_PathID pathID;

	pathID = 0;
	memset(&config, 0, sizeof(CSL_CAPH_HWCTRL_CONFIG_t));

	/* Enable the path. And get path ID. */
	config.streamID = CSL_CAPH_STREAM_NONE;
	config.pathID = 0;
	config.source = getDeviceFromSrc(source);
	config.sink = getDeviceFromSink(sink);
	config.dmaCH = CSL_CAPH_DMA_NONE;
	config.snk_sampleRate = sr;
	/* For playback, sample rate should be 48KHz. */
	config.src_sampleRate = AUDIO_SAMPLING_RATE_48000;
	config.chnlNum = numCh;
	config.bitPerSample = 16;

	if (source == AUDIO_SOURCE_USB && sink == AUDIO_SINK_DSP) {
		/* in this case, the entire data pass is
			USB Mic(48K mono) --> DDR --> (via AADMAC,
			Caph switch)HW srcMixer input CH2
		   --> HW srcMixer tapout CH2 --> DSP input --> DSP
		   sharedmem --> DDR

		   for HW control, need to setup the caph path DDR -->
		   (via AADMAC, Caph switch)HW srcMixer input CH2 -->
		   HW srcMixer tapout CH2 --> DSP.
		   the caph path source is MEMORY, the capth path sink is DSP.
		   Also need to set the input sampling rate as 48K,
		   and output sampling
		   rate as 8K or 16 (depending on the passed in parameter sr),
		   so we
		   know we need to use the HW srcMixer.
		 */
		config.source = CSL_CAPH_DEV_MEMORY;
		config.sink = CSL_CAPH_DEV_DSP;
	}
	if (config.sink == CSL_CAPH_DEV_DSP) {
#if defined(ENABLE_BT16)
		if (source == AUDIO_SOURCE_BTM)
			config.bitPerSample = 16;
		else
#endif
			config.bitPerSample = 24;
	}
	pathID = csl_caph_hwctrl_EnablePath(config);
	*pPathID = pathID;
	/*Load the mic gains from sysparm.
	   Can not call the following API here.
	   Because Capture driver really enables the path.
	   AUDCTRL_LoadMicGain(pathID, mic, FALSE);

	   also need to have a table to list the used
	   Mic / Mic's (AUDIO_SOURCE_Enum_t)
	   for each audio mode (audio device).
	   use gains from sysparm as baseline, adjust gains to achieve user-set
	   volume/gain before call AUDDRV_SetAudioMode( ).
	   AUDDRV_SetAudioMode( ) reads sysparm and reconcile them with user-set
	   volume/gain, then set to HW, DSP.
	 */

	aTrace(LOG_AUDIO_CNTLR,
			"%s path configuration, source = %d, sink = %d,"
			"pathID %d.\r\n",
			__func__, config.source, config.sink, pathID);

#if 0
	/* in case it was muted from last record */
	AUDCTRL_SetRecordMute(src, mic, FALSE);
#endif
	/* Enable DSP UL for Voice Call. */
	if (config.sink == CSL_CAPH_DEV_DSP) {
		if (!AUDCTRL_InVoiceCall()) {
			/* without setting mic gains (such as PGA),
			   idle mode recording gives low volume */
			AUDDRV_SetAudioMode_Mic(
				AUDCTRL_GetAudioMode(),
				AUDCTRL_GetAudioApp(), pathID, 0);
		}

		/* if bInVoiceCall== TRUE, assume the telphony_init() function
		   sends ENABLE and CONNECT_UL */
		if (bInVoiceCall != TRUE)
			AUDDRV_EnableDSPInput(source, sr);
	}

}

/****************************************************************************
*
* Function Name: AUDCTRL_EnableRecord
*
* Description:   enable a record path
*
****************************************************************************/
void AUDCTRL_EnableRecord(AUDIO_SOURCE_Enum_t source,
			  AUDIO_SINK_Enum_t sink,
			  AUDIO_NUM_OF_CHANNEL_t numCh,
			  AUDIO_SAMPLING_RATE_t sr, unsigned int *pPathID)
{
	unsigned int pathID;
	aTrace(LOG_AUDIO_CNTLR,
			"%s src 0x%x, sink 0x%x,sr %d",
			__func__, source, sink, sr);

	if (isDigiMic(source)) {
		/* Enable power to digital microphone */
		powerOnDigitalMic(TRUE);
	}

	if (sr == AUDIO_SAMPLING_RATE_48000)
		AUDCTRL_SaveAudioApp(AUDIO_APP_RECORDING_HQ);
	else
		AUDCTRL_SaveAudioApp(AUDIO_APP_RECORDING);

	if (source == AUDIO_SOURCE_SPEECH_DIGI) {
		/* Not supported - One stream - two paths use case for record.
		   Will be supported with one path itself */
		AUDCTRL_EnableRecordMono(AUDIO_SOURCE_DIGI1, sink,
					 AUDIO_CHANNEL_MONO, sr, &pathID);
		AUDCTRL_EnableRecordMono(AUDIO_SOURCE_DIGI2, sink,
					 AUDIO_CHANNEL_MONO, sr, NULL);
	} else if (source == AUDIO_SOURCE_MIC_ARRAY1) {

		aTrace(LOG_AUDIO_CNTLR,
				"%s src = 0x%x, sink = 0x%x,"
				"numCh = 0x%x, sr = 0x%x\n",
				__func__, source, sink, numCh, sr);
		numCh = 2;	/* stereo format */
		AUDCTRL_EnableRecordMono(source, sink, numCh, sr, &pathID);
	} else {
		AUDCTRL_EnableRecordMono(source, sink, numCh, sr, &pathID);
	}
	*pPathID = pathID;
}

/****************************************************************************
*
* Function Name: AUDCTRL_DisableRecord
*
* Description:   disable a record path
*
****************************************************************************/
void AUDCTRL_DisableRecord(AUDIO_SOURCE_Enum_t source,
			   AUDIO_SINK_Enum_t sink, unsigned int pathID)
{
	CSL_CAPH_HWConfig_Table_t *path = NULL;
	CSL_CAPH_HWCTRL_CONFIG_t config;
	aTrace(LOG_AUDIO_CNTLR, "%s src 0x%x, sink 0x%x\n",
			__func__, source, sink);

	/* Disable DSP UL */
	if (sink == AUDIO_SINK_DSP)
		/* assume the telephony_deinit() function sends DISABLE */
		if (bInVoiceCall != TRUE)
			AUDDRV_DisableDSPInput();

	if (pathID)
		path = csl_caph_FindPath(pathID);
	else
		return;

	if (path->snk_sampleRate == AUDIO_SAMPLING_RATE_48000)
		AUDCTRL_RemoveAudioApp(AUDIO_APP_RECORDING_HQ);
	else
		AUDCTRL_RemoveAudioApp(AUDIO_APP_RECORDING);

	if (source == AUDIO_SOURCE_SPEECH_DIGI) {
		/* Not supported - One stream - two paths use case for record.
		   Will be supported with one path itself */
		memset(&config, 0, sizeof(CSL_CAPH_HWCTRL_CONFIG_t));
		if (pathID == 0) {
			audio_xassert(0, pathID);
			return;
		}

		config.pathID = pathID;
		aTrace(LOG_AUDIO_CNTLR, "AUDCTRL_DisableRecord: pathID %d.\r\n",
				pathID);
		(void)csl_caph_hwctrl_DisablePath(config);

		if (pathID == 0) {
			audio_xassert(0, pathID);
			return;
		}

		config.pathID = pathID;

		aTrace(LOG_AUDIO_CNTLR, "AUDCTRL_DisableRecord: pathID %d.\r\n",
				pathID);
		(void)csl_caph_hwctrl_DisablePath(config);
	} else {
		memset(&config, 0, sizeof(CSL_CAPH_HWCTRL_CONFIG_t));
		if (pathID == 0) {
			audio_xassert(0, pathID);
			return;
		}
		config.pathID = pathID;
		aTrace(LOG_AUDIO_CNTLR, "AUDCTRL_DisableRecord: pathID %d.\r\n",
				pathID);

		if (source == AUDIO_SOURCE_USB && sink == AUDIO_SINK_DSP) {
			/* in this case, the entire data pass is
			   USB Mic(48K mono) --> DDR -->
			   (via AADMAC, Caph switch)HW srcMixer input CH2
			   --> HW srcMixer tapout CH2 --> DSP input
			   --> DSP sharedmem --> DDR

			   for HW control, need to setup the caph path DDR -->
			   (via AADMAC, Caph switch)HW srcMixer input CH2 -->
			   HW srcMixer tapout CH2 --> DSP.

			   the caph path source is MEMORY, the capth path
			   sink is DSP.
			   Also need to set the input sampling rate as 48K,
			   and output
			   sampling rate as 8K or 16 (depending on the passed
			   in parameter
			   sr), so we know we need to use the HW srcMixer.
			 */
			config.source = CSL_CAPH_DEV_MEMORY;
			config.sink = CSL_CAPH_DEV_DSP;
		}

		(void)csl_caph_hwctrl_DisablePath(config);
	}
	if (isDigiMic(source)) {
		/* Disable power to digital microphone */
		powerOnDigitalMic(FALSE);
	}
}

/****************************************************************************
*
*  Function Name: AUDCTRL_StartCapturet
*
*  Description: Start the data transfer of audio path capture
*
****************************************************************************/
Result_t AUDCTRL_StartCapture(unsigned int streamID)
{
	AudioMode_t mode = AUDIO_MODE_HANDSET;
	CSL_CAPH_HWConfig_Table_t *path = NULL;

	aTrace(LOG_AUDIO_CNTLR, "%s streamID=0x%x\n", __func__, streamID);

	path = csl_caph_FindCapturePath(streamID);
	mode = GetAudioModeFromCaptureDev(path->source);
	if (path->snk_sampleRate == AUDIO_SAMPLING_RATE_48000)
		AUDCTRL_SaveAudioApp(AUDIO_APP_RECORDING_HQ);
	else
		AUDCTRL_SaveAudioApp(AUDIO_APP_RECORDING);

	AUDCTRL_SetAudioMode_ForMusicRecord(mode, 0);
	/* start capture after gain setting to reduce glitch */
	csl_audio_capture_start(streamID);

	return RESULT_OK;
}

/****************************************************************************
*
*  Function Name: csl_audio_capture_stop
*
*  Description: Stop the data transfer of audio path capture
*
****************************************************************************/
Result_t AUDCTRL_StopCapture(unsigned int streamID)
{
	CSL_CAPH_HWConfig_Table_t *path = NULL;

	aTrace(LOG_AUDIO_CNTLR, "%s streamID=0x%x\n", __func__, streamID);

	path = csl_caph_FindCapturePath(streamID);
	if (path->snk_sampleRate == AUDIO_SAMPLING_RATE_48000)
		AUDCTRL_RemoveAudioApp(AUDIO_APP_RECORDING_HQ);
	else
		AUDCTRL_RemoveAudioApp(AUDIO_APP_RECORDING);

	csl_audio_capture_stop(streamID);

	return RESULT_OK;
}

/****************************************************************************
*
* Function Name: AUDCTRL_SetRecordGain
*
* Description:   set gain of a record path
*
****************************************************************************/
void AUDCTRL_SetRecordGain(AUDIO_SOURCE_Enum_t source,
			   AUDIO_GAIN_FORMAT_t gainFormat,
			   UInt32 gainL, UInt32 gainR, unsigned int pathID)
{
	CSL_CAPH_HWConfig_Table_t *path;
	csl_caph_Mic_Gain_t outGain;

	aTrace(LOG_AUDIO_CNTLR, "%s src = 0x%x, gainL = 0x%lx, gainR = 0x%lx\n",
			__func__, source, gainL, gainR);

	if (!pathID)
		return;
	path = csl_caph_FindPath(pathID);

	if (gainFormat == AUDIO_GAIN_FORMAT_mB) {
		/*switch( mic )  why not this. simply see mic.
		   does audio_caph.c pass down correct mic param? */
		switch (path->source) {
		case AUDIO_SOURCE_ANALOG_MAIN:
		case AUDIO_SOURCE_ANALOG_AUX:
			outGain =
			    csl_caph_map_mB_gain_to_registerVal
			    (MIC_ANALOG_HEADSET, (int)gainL);
			if (path->source == CSL_CAPH_DEV_ANALOG_MIC)
				csl_caph_audioh_setgain_register
				    (AUDDRV_PATH_ANALOGMIC_INPUT,
				     outGain.micPGA, 0);
			else
				csl_caph_audioh_setgain_register
				    (AUDDRV_PATH_HEADSET_INPUT, outGain.micPGA,
				     0);

			csl_caph_audioh_setgain_register
			    (AUDDRV_PATH_VIN_INPUT_L, outGain.micCICBitSelect,
			     outGain.micCICFineScale);

			break;

		case AUDIO_SOURCE_DIGI1:
			outGain =
			    csl_caph_map_mB_gain_to_registerVal(MIC_DIGITAL,
								(int)gainL);
			csl_caph_audioh_setgain_register
			    (AUDDRV_PATH_VIN_INPUT_L, outGain.micCICBitSelect,
			     outGain.micCICFineScale);
			break;

		case AUDIO_SOURCE_DIGI2:
			outGain =
			    csl_caph_map_mB_gain_to_registerVal(MIC_DIGITAL,
								(int)gainL);
			csl_caph_audioh_setgain_register
			    (AUDDRV_PATH_VIN_INPUT_R, outGain.micCICBitSelect,
			     outGain.micCICFineScale);

			break;

		case AUDIO_SOURCE_SPEECH_DIGI:
	/*Digital Mic1/Mic2 in recording/Normal Quality Voice call. */
			outGain =
			    csl_caph_map_mB_gain_to_registerVal(MIC_DIGITAL,
								(int)gainL);
			csl_caph_audioh_setgain_register(AUDDRV_PATH_VIN_INPUT,
							 outGain.
							 micCICBitSelect,
							 outGain.
							 micCICFineScale);
			break;

		case AUDIO_SOURCE_BTM:	/*Bluetooth Mono Headset Mic */
		case AUDIO_SOURCE_USB:	/*USB headset Mic */
		case AUDIO_SOURCE_I2S:
			break;

		case AUDIO_SOURCE_DIGI3:	/*Only for loopback path */
			outGain =
			    csl_caph_map_mB_gain_to_registerVal(MIC_DIGITAL,
								(int)gainL);
			csl_caph_audioh_setgain_register
			    (AUDDRV_PATH_NVIN_INPUT_L, outGain.micCICBitSelect,
			     outGain.micCICFineScale);
			break;

		case AUDIO_SOURCE_DIGI4:	/*Only for loopback path */
			outGain =
			    csl_caph_map_mB_gain_to_registerVal(MIC_DIGITAL,
								(int)gainL);
			csl_caph_audioh_setgain_register
			    (AUDDRV_PATH_NVIN_INPUT_R, outGain.micCICBitSelect,
			     outGain.micCICFineScale);
			break;

		default:
			break;
		}

	}

	return;
}

/****************************************************************************
*
* Function Name: AUDCTRL_SetRecordMuteMono
*
* Description:   mute/unmute a record path for a single mic
*
****************************************************************************/
static void AUDCTRL_SetRecordMuteMono(AUDIO_SOURCE_Enum_t source,
				      Boolean mute, unsigned int pathID)
{
	aTrace(LOG_AUDIO_CNTLR, "%s src 0x%x, mute 0x%x\n", __func__,
			source, mute);

	if (pathID == 0) {
		audio_xassert(0, pathID);
		return;
	}

	if (mute == TRUE)
		csl_caph_hwctrl_MuteSource(pathID);
	else
		csl_caph_hwctrl_UnmuteSource(pathID);

	return;
}

/****************************************************************************
*
* Function Name: AUDCTRL_SetRecordMuteMono
*
* Description:   mute/unmute a record path
*
****************************************************************************/
void AUDCTRL_SetRecordMute(AUDIO_SOURCE_Enum_t source,
			   Boolean mute, unsigned int pathID)
{
	aTrace(LOG_AUDIO_CNTLR, "%s src 0x%x, mute 0x%x\n", __func__,
			source, mute);
	if (pathID == 0) {
		audio_xassert(0, pathID);
		return;
	}

	if (source == AUDIO_SOURCE_USB)
		return;

	if (source == AUDIO_SOURCE_SPEECH_DIGI) {
		/* Not supported - One stream - two paths use case for record.
		   Will be supported with one path itself */
		AUDCTRL_SetRecordMuteMono(AUDIO_SOURCE_DIGI1, mute, pathID);
		AUDCTRL_SetRecordMuteMono(AUDIO_SOURCE_DIGI2, mute, pathID);
	} else {
		AUDCTRL_SetRecordMuteMono(source, mute, pathID);
	}

	return;
}

/****************************************************************************
*
* Function Name: AUDCTRL_AddRecordMic
*
* Description:   add a microphone to a record path
*
****************************************************************************/
void AUDCTRL_AddRecordMic(AUDIO_SOURCE_Enum_t mic)
{
}

/****************************************************************************
*
* Function Name: AUDCTRL_RemoveRecordMic
*
* Description:   remove a microphone from a record path
*
****************************************************************************/
void AUDCTRL_RemoveRecordMic(AUDIO_SOURCE_Enum_t mic)
{
	/* Nothing to do. */
}

/****************************************************************************
*
* Function Name: AUDCTRL_SetAudioLoopback
*
* Description:   Set the loopback path
*
****************************************************************************/
void AUDCTRL_SetAudioLoopback(Boolean enable_lpbk,
			      AUDIO_SOURCE_Enum_t mic,
			      AUDIO_SINK_Enum_t speaker, int sidetone_mode)
{
	/*Sidetone FIR filter coeffs. */
	static UInt32 sidetoneCoeff[128] = {
		0x7FFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};
	UInt32 *coeff = &sidetoneCoeff[0];
	CSL_CAPH_DEVICE_e src_dev = CSL_CAPH_DEV_ANALOG_MIC;
	CSL_CAPH_DEVICE_e sink_dev = CSL_CAPH_DEV_EP;
	static CSL_CAPH_DEVICE_e hw_lpbk_audSpkr;
	CSL_CAPH_PathID pathID = 0;
	CSL_CAPH_HWCTRL_CONFIG_t hwCtrlConfig;
	AudioMode_t audio_mode = AUDIO_MODE_HANDSET;

	aTrace(LOG_AUDIO_CNTLR, "AUDCTRL_SetAudioLoopback:"
			"mic = %d, speaker = %d, mode = %d\n",
			mic, speaker, sidetone_mode);

	memset(&hwCtrlConfig, 0, sizeof(CSL_CAPH_HWCTRL_CONFIG_t));

	src_dev = getDeviceFromSrc(mic);

	switch (speaker) {
	case AUDIO_SINK_HANDSET:
		hw_lpbk_audSpkr = CSL_CAPH_DEV_EP;
		break;
	case AUDIO_SINK_HEADSET:
		hw_lpbk_audSpkr = CSL_CAPH_DEV_HS;
		break;
	case AUDIO_SINK_LOUDSPK:
		hw_lpbk_audSpkr = CSL_CAPH_DEV_IHF;
		break;
	default:
		hw_lpbk_audSpkr = CSL_CAPH_DEV_EP;
		break;
	}

	sink_dev = getDeviceFromSink(speaker);
	audio_mode = GetAudioModeBySink(speaker);

	audio_control_generic(AUDDRV_CPCMD_PassAudioMode, (UInt32) audio_mode,
			      0, 0, 0, 0);

	if (enable_lpbk) {
		aTrace(LOG_AUDIO_CNTLR, "%s Enable loopback with"
				"sidetone mode = %d\n",
				__func__, sidetone_mode);

		AUDCTRL_SaveAudioApp(AUDIO_APP_LOOPBACK);

		/* For I2S/PCM loopback */
		if (((src_dev == CSL_CAPH_DEV_FM_RADIO)
		     && (sink_dev == CSL_CAPH_DEV_FM_TX))
		    || ((src_dev == CSL_CAPH_DEV_BT_MIC)
			&& (sink_dev == CSL_CAPH_DEV_BT_SPKR))) {
			/* I2S hard coded to use ssp3, BT PCM to use ssp4.
			   This could be changed later */
			AUDIO_SOURCE_Enum_t srcTemp = AUDIO_SOURCE_I2S;
			AUDIO_SINK_Enum_t sinkTemp = AUDIO_SINK_I2S;
			if (src_dev == CSL_CAPH_DEV_BT_MIC)
				srcTemp = AUDIO_SOURCE_BTM;
			if (sink_dev == CSL_CAPH_DEV_BT_SPKR)
				sinkTemp = AUDIO_SINK_BTM;

			AUDCTRL_EnablePlay(srcTemp, speaker, AUDIO_CHANNEL_MONO,
					   48000, NULL);
			AUDCTRL_EnableRecord(mic, sinkTemp, AUDIO_CHANNEL_MONO,
					     48000, NULL);
			return;
		}
#if 0
/*removed this to make fm radio work using xpft script */
		if (src_dev == CSL_CAPH_DEV_FM_RADIO) {
			AUDCTRL_EnableRecord(audRecHw, audPlayHw, mic,
					     AUDIO_CHANNEL_STEREO, 48000);
			if ((speaker == AUDIO_SINK_LOUDSPK)
			    || (speaker == AUDIO_SINK_HEADSET))
				powerOnExternalAmp(speaker, AudioUse,
						   TRUE);
			return;
		}
#endif
		/*  Microphone pat */
		if (isDigiMic(mic)) {
			/* Enable power to digital microphone */
			powerOnDigitalMic(TRUE);
		}
		/* enable HW path */
		hwCtrlConfig.streamID = CSL_CAPH_STREAM_NONE;
		hwCtrlConfig.source = src_dev;
		hwCtrlConfig.sink = sink_dev;
		hwCtrlConfig.src_sampleRate = AUDIO_SAMPLING_RATE_48000;
		hwCtrlConfig.snk_sampleRate = AUDIO_SAMPLING_RATE_48000;
		if (src_dev == CSL_CAPH_DEV_BT_MIC)
			hwCtrlConfig.src_sampleRate =
			AUDIO_SAMPLING_RATE_8000;	/*how about WB? */
		if (sink_dev == CSL_CAPH_DEV_BT_SPKR)
			hwCtrlConfig.snk_sampleRate =
			AUDIO_SAMPLING_RATE_8000;	/*how about WB? */

		hwCtrlConfig.chnlNum =
		    (speaker ==
		     AUDIO_SINK_HEADSET) ? AUDIO_CHANNEL_STEREO :
		    AUDIO_CHANNEL_MONO;
		hwCtrlConfig.bitPerSample = 16;
		hwCtrlConfig.sidetone_mode = sidetone_mode;

		pathID = csl_caph_hwctrl_EnablePath(hwCtrlConfig);

/*#ifdef HW_SIDETONE_LOOPBACK*/
		if (sidetone_mode) {
			/*sidetone_mode = 1 to use sidetone path */
			/*Enable the sidetone path. */
			/*first step: enable sidetone */
			csl_caph_hwctrl_EnableSidetone(sink_dev);
			/*second step: set filter and gain */
			csl_caph_hwctrl_ConfigSidetoneFilter(coeff);
			csl_caph_hwctrl_SetSidetoneGain(0);
			/* Set sidetone gain to 0dB. */
		} else {
			/*loopback does not use sidetone path*/
			aTrace(LOG_AUDIO_CNTLR,
					"%s sidetone path disabled\n",
					__func__);
		}
/*#endif*/

		/*Assume HW lpt does not involve DSP.
		o.w. use AUDCTRL_SetAudioMode */
		AUDCTRL_SaveAudioMode(audio_mode);
		AUDCTRL_SaveAudioApp(AUDIO_APP_LOOPBACK);
		AUDCTRL_SetAudioMode_ForMusicPlayback(audio_mode, pathID, TRUE);
		AUDCTRL_SetAudioMode_ForMusicRecord(audio_mode, pathID);

		/* Enable Loopback ctrl */
		/* Enable PMU for headset/IHF */
		if ((speaker == AUDIO_SINK_LOUDSPK)
		    || (speaker == AUDIO_SINK_HEADSET))
			powerOnExternalAmp(speaker, AudioUse, TRUE, FALSE);

		if (((src_dev == CSL_CAPH_DEV_ANALOG_MIC)
		     || (src_dev == CSL_CAPH_DEV_HS_MIC))
		    && ((sink_dev == CSL_CAPH_DEV_EP)
			|| (sink_dev == CSL_CAPH_DEV_IHF)
			|| (sink_dev == CSL_CAPH_DEV_HS))) {
#ifdef HW_ANALOG_LOOPBACK
			csl_caph_audio_loopback_control(hw_lpbk_audSpkr, 0,
							enable_lpbk);
#endif
		}

	} else {
		/* Disable Analog Mic path */
		aTrace(LOG_AUDIO_CNTLR,
				"AUDCTRL_SetAudioLoopback: Disable loopback\n");

		AUDCTRL_RemoveAudioApp(AUDIO_APP_LOOPBACK);

		/* Disable I2S/PCM loopback */
		if (((src_dev == CSL_CAPH_DEV_FM_RADIO)
		     && (sink_dev == CSL_CAPH_DEV_FM_TX))
		    || ((src_dev == CSL_CAPH_DEV_BT_MIC)
			&& (sink_dev == CSL_CAPH_DEV_BT_SPKR))) {
			/* I2S configured to use ssp3, BT PCM to use ssp4. */
			AUDIO_SOURCE_Enum_t srcTemp = AUDIO_SOURCE_I2S;
			AUDIO_SINK_Enum_t sinkTemp = AUDIO_SINK_I2S;
			if (src_dev == CSL_CAPH_DEV_BT_MIC)
				srcTemp = AUDIO_SOURCE_BTM;
			if (sink_dev == CSL_CAPH_DEV_BT_SPKR)
				sinkTemp = AUDIO_SINK_BTM;

			AUDCTRL_DisablePlay(srcTemp, speaker, 0);
			AUDCTRL_DisableRecord(mic, sinkTemp, 0);
			return;
		}
#if 0
/*removed this to make fm radio work using xpft script*/
		if (src_dev == CSL_CAPH_DEV_FM_RADIO) {
			AUDCTRL_DisableRecord(audRecHw, audPlayHw, mic);
			if ((speaker == AUDIO_SINK_LOUDSPK)
			    || (speaker == AUDIO_SINK_HEADSET))
				powerOnExternalAmp(speaker, AudioUse,
						   FALSE);
			return;
		}
#endif
		if (isDigiMic(mic)) {
			/* Enable power to digital microphone */
			powerOnDigitalMic(FALSE);
		}

		memset(&hwCtrlConfig, 0, sizeof(CSL_CAPH_HWCTRL_CONFIG_t));

		src_dev = getDeviceFromSrc(mic);
		sink_dev = getDeviceFromSink(speaker);

		pathID = csl_caph_FindPathID(sink_dev, src_dev);

		if (pathID == 0) {
			audio_xassert(0, pathID);
			return;
		}

		hwCtrlConfig.pathID = pathID;
		if (((src_dev == CSL_CAPH_DEV_ANALOG_MIC)
		     || (src_dev == CSL_CAPH_DEV_HS_MIC))
		    && ((sink_dev == CSL_CAPH_DEV_EP)
			|| (sink_dev == CSL_CAPH_DEV_IHF)
			|| (sink_dev == CSL_CAPH_DEV_HS))) {
#ifdef HW_ANALOG_LOOPBACK
			csl_caph_audio_loopback_control(hw_lpbk_audSpkr, 0,
							enable_lpbk);
#endif
		}
/*#ifdef HW_SIDETONE_LOOPBACK*/
		/*Disable Sidetone path. */
		if (sidetone_mode)
			csl_caph_hwctrl_DisableSidetone(sink_dev);
/*#endif*/

		/*clocks are disabled here, so no register access after this. */
		(void)csl_caph_hwctrl_DisablePath(hwCtrlConfig);

		/*Enable PMU for headset/IHF */
		if ((speaker == AUDIO_SINK_LOUDSPK)
		    || (speaker == AUDIO_SINK_HEADSET)) {
			powerOnExternalAmp(speaker, AudioUse, FALSE, FALSE);
		}

	}
}

/****************************************************************************
*
* Function Name: AUDCTRL_SetArm2spParam
*
* Description:   Set Arm2Sp Parameter
*
****************************************************************************/
void AUDCTRL_SetArm2spParam(UInt32 mixMode, Boolean is_fm)
{
	if (is_fm)
		csl_caph_arm2sp_set_fm_mixmode((int)mixMode);
}

/****************************************************************************
*
* Function Name: AUDCTRL_ConfigSSP
*
* Description:   Set FM/PCM SSP protocol, and port number
*                If loopback is enabled, port is ignored
*
****************************************************************************/
void AUDCTRL_ConfigSSP(AUDCTRL_SSP_PORT_e port, AUDCTRL_SSP_BUS_e bus,
		       int en_lpbk)
{
	CSL_SSP_PORT_e csl_port = CSL_SSP_3;
	CSL_SSP_BUS_e csl_bus = CSL_SSP_PCM;

	if (port == AUDCTRL_SSP_4)
		csl_port = CSL_SSP_4;
	else if (port == AUDCTRL_SSP_3)
		csl_port = CSL_SSP_3;
	else if (!en_lpbk)
		return;

	if (bus == AUDCTRL_SSP_I2S)
		csl_bus = CSL_SSP_I2S;
	else if (bus == AUDCTRL_SSP_PCM)
		csl_bus = CSL_SSP_PCM;
	else if (bus == AUDCTRL_SSP_TDM)
		csl_bus = CSL_SSP_TDM;
	else if (!en_lpbk)
		return;

	csl_caph_hwctrl_ConfigSSP(csl_port, csl_bus, en_lpbk);
}


/****************************************************************************
*
* Function Name: AUDCTRL_EnableBypassVibra
*
* Description:   Enable the Vibrator bypass and set the strength to the vibrator
*
****************************************************************************/
void AUDCTRL_EnableBypassVibra(UInt32 Strength, int direction)
{
	UInt32 vib_power;
	int ret = 0;

	aTrace(LOG_AUDIO_CNTLR, "AUDCTRL_EnableBypassVibra");

	if (!vibra_reg) {
		vibra_reg = regulator_get(NULL, VIBRA_LDO_REGULATOR);

		if (IS_ERR(vibra_reg))
			aError("Failed to get LDO for Vibra\n");
	}
	if (vibra_reg) {
		ret = regulator_enable(vibra_reg);
		if (ret != 0)
			aError("Failed to enable LDO for Vibra: %d\n", ret);
	}

	csl_caph_hwctrl_vibrator(AUDDRV_VIBRATOR_BYPASS_MODE, TRUE);

	Strength = ((Strength > 100) ? 100 : Strength);
	vib_power = (0x7fff / 100) * Strength;
	vib_power = ((direction == 0) ? vib_power : (0xffff - vib_power + 1));

	csl_caph_hwctrl_vibrator_strength(vib_power);
}

/****************************************************************************
*
* Function Name: AUDCTRL_DisableBypassVibra
*
* Description:   Disable the Vibrator bypass
*
****************************************************************************/
void AUDCTRL_DisableBypassVibra(void)
{
	int ret;
	aTrace(LOG_AUDIO_CNTLR, "AUDCTRL_DisableBypassVibra");
	csl_caph_hwctrl_vibrator(AUDDRV_VIBRATOR_BYPASS_MODE, FALSE);

	if (vibra_reg) {
		ret = regulator_disable(vibra_reg);
		if (ret != 0)
			aError("Failed to disable LDO for Vibra: %d\n", ret);
		regulator_put(vibra_reg);
		vibra_reg = NULL;
	}
}

/****************************************************************************
*
* Function Name: AUDCTRL_Telephony_HW_16K
*
* Description:   find HW sample rate
*
****************************************************************************/
int AUDCTRL_Telephony_HW_16K(AudioMode_t voiceMode)
{
	int is_call16k = FALSE;

	if (voiceCallSampleRate == AUDIO_SAMPLING_RATE_16000)
		is_call16k = TRUE;

	/* BT headset needs to consider NB or WB too */
#ifdef	CONFIG_AUDIO_FEATURE_SET_DISABLE_ECNS

	if ((voiceMode == AUDIO_MODE_BLUETOOTH) ||
		(voiceMode == AUDIO_MODE_HANDSFREE))
#else
	if (voiceMode == AUDIO_MODE_BLUETOOTH)
#endif
		is_call16k = AUDCTRL_IsBTMWB();

	return is_call16k;
}

/****************************************************************************
*
* Function Name: AUDCTRL_InVoiceCall
*
* Description:   Return Voice Call status
*
****************************************************************************/
int AUDCTRL_InVoiceCall(void)
{
	return bInVoiceCall;
}

/*********************************************************************
*
*	Get BTM headset NB or WB info
*	@return		Boolean, TRUE for WB and FALSE for NB (8k)
*	@note
**********************************************************************/
int AUDCTRL_IsBTMWB(void)
{
	return IsBTM_WB;
}

/*********************************************************************
*
*	Set BTM type
*	@param		Boolean isWB
*	@return		none
*
*	@note	isWB=TRUE for BT WB headset; =FALSE for BT NB (8k) headset.
**********************************************************************/
void AUDCTRL_SetBTMTypeWB(Boolean isWB)
{
	IsBTM_WB = isWB;
	/* AUDDRV_SetPCMRate(IsBTM_WB); */
}

/********************************************************************
*  @brief  Set IHF mode
*
*  @param  IHF mode status (TRUE: stereo | FALSE: mono).
*
*  @return  none
*
****************************************************************************/
void AUDCTRL_SetIHFmode(Boolean stIHF)
{
	csl_caph_hwctrl_SetIHFmode(stIHF);
	isStIHF = stIHF;
}

/****************************************************************************
*
* Function Name: AUDCTRL_SetBtMode
*
* Description:   Set BT mode
*
****************************************************************************/
void AUDCTRL_SetBTMode(int mode)
{
	if ((mode == BT_MODE_WB) || (mode == BT_MODE_WB_TEST))
		AUDCTRL_SetBTMTypeWB(TRUE);
	else
		AUDCTRL_SetBTMTypeWB(FALSE);
	csl_caph_hwctrl_SetBTMode(mode);
}

/****************************************************************************
*
* Function Name: AUDCTRL_GetHardwareControl
*
* Description:   Get hardware control parameters
*
****************************************************************************/
void AUDCTRL_GetHardwareControl(AUDCTRL_HW_ACCESS_TYPE_en_t access_type,
				unsigned *buf)
{
	if (access_type >= AUDCTRL_HW_ACCESS_TYPE_TOTAL || !buf)
		return;

	memcpy(buf, hw_control[access_type], sizeof(hw_control[access_type]));
}

/********************************************************************
*  @brief  Hardware register access fucntion
*
*  @param
*
*  @return  int
* note
* this is for test purpose
*alsa_amixer cset name=HW-CTL 1 0 100 0  (set EP_MIX_IN_GAIN to 100 mB, 1 dB)
*alsa_amixer cset name=HW-CTL 1 1 2400 0  (set EP_MIX_BITSEL_GAIN to 24 dB)
*alsa_amixer cset name=HW-CTL 1 2 -600 0  (set EP_MIX_FINE_GAIN to -6 dB)
*
****************************************************************************/
int AUDCTRL_HardwareControl(AUDCTRL_HW_ACCESS_TYPE_en_t access_type,
			    int arg1, int arg2, int arg3, int arg4)
{
	CSL_CAPH_MIXER_e outChnl = CSL_CAPH_SRCM_CH_NONE;

	aTrace(LOG_AUDIO_CNTLR,
			"AUDCTRL_HardwareControl::type %d,"
			"arg 0x%x %x %x %x.\n",
			access_type, arg1, arg2, arg3, arg4);

	if (access_type >= AUDCTRL_HW_ACCESS_TYPE_TOTAL)
		return -1;

	hw_control[access_type][0] = arg1;
	hw_control[access_type][1] = arg2;
	hw_control[access_type][2] = arg3;
	hw_control[access_type][3] = arg4;

	/* Need to set SRC clock mode before enable clock */
	if (access_type == AUDCTRL_HW_CFG_CLK) {
		is26MClk = arg1 ? TRUE : FALSE;
		csl_caph_SetSRC26MClk(is26MClk);
	}
	csl_caph_ControlHWClock(TRUE);

	switch (access_type) {
	case AUDCTRL_HW_CFG_HEADSET:
		csl_caph_hwctrl_SetHeadsetMode(arg1);
		break;
	case AUDCTRL_HW_CFG_SSP:
		AUDCTRL_ConfigSSP(arg1, arg2, arg3);
		break;
	case AUDCTRL_HW_CFG_MFD:
		isMFD = arg1 ? TRUE : FALSE;
		break;

	case AUDCTRL_HW_WRITE_GAIN:

		/*arg2 is gain in milli Bel */
		switch (arg1) {
		case AUDCTRL_EP_MIX_IN_GAIN:
			outChnl = CSL_CAPH_SRCM_STEREO_CH2_L;
			/*csl_srcmixer_setMixInGain(
	   path->srcmRoute[0][0].inChnl, outChnl, arg2, arg2); */
			csl_srcmixer_setMixAllInGain(outChnl, arg2, arg2);
			break;

		case AUDCTRL_EP_MIX_BITSEL_GAIN:
			outChnl = CSL_CAPH_SRCM_STEREO_CH2_L;
			csl_srcmixer_setMixBitSel(outChnl,
				arg2 / 600, arg2 / 600);
			break;

		case AUDCTRL_EP_MIX_FINE_GAIN:
			outChnl = CSL_CAPH_SRCM_STEREO_CH2_L;
			csl_srcmixer_setMixOutGain(outChnl, arg2, arg2);
			break;

		case AUDCTRL_IHF_MIX_IN_GAIN:
			/*outChnl = (CSL_CAPH_SRCM_STEREO_CH2_R |
			   CSL_CAPH_SRCM_STEREO_CH2_L); */
			outChnl = CSL_CAPH_SRCM_STEREO_CH2_R;
			/*csl_srcmixer_setMixInGain(
				path->srcmRoute[0][0].inChnl,
				outChnl, arg2, arg2); */
			csl_srcmixer_setMixAllInGain(outChnl, arg2, arg2);
			break;

		case AUDCTRL_IHF_MIX_BITSEL_GAIN:
			/*outChnl = (CSL_CAPH_SRCM_STEREO_CH2_R |
			   CSL_CAPH_SRCM_STEREO_CH2_L); */
			outChnl = CSL_CAPH_SRCM_STEREO_CH2_R;
			csl_srcmixer_setMixBitSel(outChnl,
				arg2 / 600, arg2 / 600);
			break;

		case AUDCTRL_IHF_MIX_FINE_GAIN:
			/*outChnl = (CSL_CAPH_SRCM_STEREO_CH2_R |
			   CSL_CAPH_SRCM_STEREO_CH2_L); */
			outChnl = CSL_CAPH_SRCM_STEREO_CH2_R;
			csl_srcmixer_setMixOutGain(outChnl, arg2, arg2);
			break;

		case AUDCTRL_HS_LEFT_MIX_IN_GAIN:
			outChnl = CSL_CAPH_SRCM_STEREO_CH1_L;
			/*csl_srcmixer_setMixInGain(
				path->srcmRoute[0][0].inChnl,
				outChnl, arg2, arg2); */
			csl_srcmixer_setMixAllInGain(outChnl, arg2, arg2);
			break;

		case AUDCTRL_HS_LEFT_MIX_BITSEL_GAIN:
			outChnl = CSL_CAPH_SRCM_STEREO_CH1_L;
			csl_srcmixer_setMixBitSel(outChnl,
				arg2 / 600, arg2 / 600);
			break;

		case AUDCTRL_HS_LEFT_MIX_FINE_GAIN:
			outChnl = CSL_CAPH_SRCM_STEREO_CH1_L;
			csl_srcmixer_setMixOutGain(outChnl, arg2, arg2);
			break;

		case AUDCTRL_HS_RIGHT_MIX_IN_GAIN:
			outChnl = CSL_CAPH_SRCM_STEREO_CH1_R;
			/*csl_srcmixer_setMixInGain(
				path->srcmRoute[0][0].inChnl,
				outChnl, arg2, arg2); */
			csl_srcmixer_setMixAllInGain(outChnl, arg2, arg2);
			break;

		case AUDCTRL_HS_RIGHT_MIX_BITSEL_GAIN:
			outChnl = CSL_CAPH_SRCM_STEREO_CH1_R;
			csl_srcmixer_setMixBitSel(outChnl,
				arg2 / 600, arg2 / 600);
			break;

		case AUDCTRL_HS_RIGHT_MIX_FINE_GAIN:
			outChnl = CSL_CAPH_SRCM_STEREO_CH1_R;
			csl_srcmixer_setMixOutGain(outChnl, arg2, arg2);
			break;

		case AUDCTRL_PMU_HS_RIGHT_GAIN:
			extern_hs_set_gain(arg2, AUDIO_HS_RIGHT);
			break;

		case AUDCTRL_PMU_HS_LEFT_GAIN:
			extern_hs_set_gain(arg2, AUDIO_HS_LEFT);
			break;

		case AUDCTRL_PMU_IHF_GAIN:
			extern_ihf_set_gain(arg2);
			break;

		case AUDCTRL_PMU_HIGH_GAIN_MODE:
			extern_ihf_en_hi_gain_mode(arg2);
			/*arg2: TRUE or FALSE */
			break;

		default:
			break;
		}
		break;

	default:
		break;
	}

	return 0;
}

/****************************************************************************
*
* Function Name: powerOnDigitalMic
*
* Description:   power on/off the Digital Mic
*
****************************************************************************/
void powerOnDigitalMic(Boolean powerOn)
{
#ifdef CONFIG_BCMPMU_AUDIO

	if (powerOn == TRUE) {
#ifdef CONFIG_DIGI_MIC
		/* Enable power to digital microphone */
		PMU_SetLDOMode(PMU_HVLDO7CTRL, 0);
#endif
	} else {		/*powerOn == FALSE */
#ifdef CONFIG_DIGI_MIC
		/* Enable power to digital microphone */
		PMU_SetLDOMode(PMU_HVLDO7CTRL, 1);
#endif
	}
#endif
}

/****************************************************************************
*
* Function Name: getDeviceFromSrc
*
* Description:   convert audio controller Mic enum to auddrv device enum
*
****************************************************************************/
CSL_CAPH_DEVICE_e getDeviceFromSrc(AUDIO_SOURCE_Enum_t source)
{
	return MIC_Mapping_Table[source].dev;
}

/****************************************************************************
*
* Function Name: getDeviceFromSink
*
* Description:   convert audio controller Spkr enum to auddrv device enum
*
****************************************************************************/
CSL_CAPH_DEVICE_e getDeviceFromSink(AUDIO_SINK_Enum_t sink)
{
	return SPKR_Mapping_Table[sink].dev;
}

/*********************************************************************
//Description:
//      Get audio mode from sink
//Parameters
//      mode -- audio mode
//      sink -- Sink device coresponding to audio mode
//Return        none
**********************************************************************/
AudioMode_t GetAudioModeBySink(AUDIO_SINK_Enum_t sink)
{
	switch (sink) {
	case AUDIO_SINK_HANDSET:
		return AUDIO_MODE_HANDSET;

	case AUDIO_SINK_HEADSET:
	case AUDIO_SINK_HEADPHONE:
		return AUDIO_MODE_HEADSET;

	case AUDIO_SINK_HANDSFREE:
		return AUDIO_MODE_HANDSFREE;

	case AUDIO_SINK_BTM:
	case AUDIO_SINK_BTS:
		return AUDIO_MODE_BLUETOOTH;

	case AUDIO_SINK_LOUDSPK:
		return AUDIO_MODE_SPEAKERPHONE;

	case AUDIO_SINK_TTY:
		return AUDIO_MODE_TTY;

	case AUDIO_SINK_HAC:
		return AUDIO_MODE_HAC;

	case AUDIO_SINK_USB:
		return AUDIO_MODE_USB;

	case AUDIO_SINK_DSP:
		return AUDCTRL_GetAudioMode();	/*play throu DSP */

	case AUDIO_SINK_I2S:
	case AUDIO_SINK_VIBRA:
	default:
		aTrace(LOG_AUDIO_CNTLR,
				"%s can not find mode %d\n", __func__, sink);
		return AUDIO_MODE_INVALID;
	}
}

static AudioMode_t GetAudioModeFromCaptureDev(CSL_CAPH_DEVICE_e source)
{
	if (source == CSL_CAPH_DEV_ANALOG_MIC) {
		return AUDIO_MODE_HANDSET;
		/* AUDIO_MODE_SPEAKERPHONE? use which mode? */
	} else if (source == CSL_CAPH_DEV_HS_MIC)
		return AUDIO_MODE_HEADSET;
	else if (source == CSL_CAPH_DEV_DIGI_MIC
		 || source == CSL_CAPH_DEV_DIGI_MIC_L
		 || source == CSL_CAPH_DEV_DIGI_MIC_R) {
		return AUDIO_MODE_SPEAKERPHONE;
		/* AUDIO_MODE_HANDSET?  use which mode? */
	} else if (source == CSL_CAPH_DEV_BT_MIC)
		return AUDIO_MODE_BLUETOOTH;

	return AUDIO_MODE_INVALID;
}

/****************************************************************************
*
* Function Name: isDigiMic
*
* Description:   return a flag
*
****************************************************************************/
static int isDigiMic(AUDIO_SOURCE_Enum_t source)
{
	if ((source == AUDIO_SOURCE_DIGI1)
	    || (source == AUDIO_SOURCE_DIGI2)
	    || (source == AUDIO_SOURCE_DIGI3)
	    || (source == AUDIO_SOURCE_DIGI4)
	    || (source == AUDIO_SOURCE_SPEECH_DIGI))
		return 1;
	else
		return 0;
}

/****************************************************************************
*
* Function Name: powerOnExternalAmp
*
* Description:   return a flag
*
****************************************************************************/
static int needDualMic(AudioMode_t mode, AudioApp_t app)
{
	/* VOICE_DUALMIC_ENABLE in parm_audio.txt */
	if (AudParmP()[mode + app * AUDIO_MODE_NUMBER].dual_mic_enable)
		return 1;
	else
		return 0;
}

/****************************************************************************
*
* Function Name: powerOnExternalAmp
*
* Description:   call external amplifier driver
*
****************************************************************************/
static void powerOnExternalAmp(AUDIO_SINK_Enum_t speaker,
			       enum ExtSpkrUsage_en_t usage_flag,
			       int use, int force)
{

	static Boolean callUseHS = FALSE;
	static Boolean audioUseHS = FALSE;
	/*static int audio2UseHS = FALSE; */
	static int fmUseHS = FALSE;

	static Boolean callUseIHF = FALSE;
	static Boolean audioUseIHF = FALSE;
	/*static int audio2UseIHF = FALSE; */
	static int fmUseIHF = FALSE;

	static Boolean IHF_IsOn = FALSE;
	static Boolean HS_IsOn = FALSE;
	static Boolean ampControl = TRUE;

	aTrace(LOG_AUDIO_CNTLR, "%s speaker %d, usage_flag %d, use %d,"
			" force %d\n",
			__func__, speaker, usage_flag, use, force);
	if (force == TRUE) {
		if (use == FALSE) {
			if (IHF_IsOn == TRUE)
				extern_ihf_off();
			if (HS_IsOn == TRUE)
				extern_hs_off();
			ampControl = FALSE;
		}
		if (use == TRUE) {
			if (IHF_IsOn == TRUE) {
				extern_ihf_on();
				setExternAudioGain(
					GetAudioModeBySink(AUDIO_SINK_LOUDSPK),
					AUDCTRL_GetAudioApp());
			}
			if (HS_IsOn == TRUE) {
				extern_hs_on();
				setExternAudioGain(
					GetAudioModeBySink(AUDIO_SINK_HEADSET),
					AUDCTRL_GetAudioApp());
			}
			ampControl = TRUE;
		}

	} else {
		switch (speaker) {
		case AUDIO_SINK_HEADSET:
		case AUDIO_SINK_TTY:
			switch (usage_flag) {
			case TelephonyUse:
				callUseHS = use;
				if (use)
					/*only one output channel
					for voice call */
					callUseIHF = FALSE;
				break;

			case AudioUse:
				audioUseHS = use;
				break;

			case FmUse:
				fmUseHS = use;
				break;

			default:
				break;
		}
		break;

		case AUDIO_SINK_LOUDSPK:
			switch (usage_flag) {
			case TelephonyUse:
				callUseIHF = use;
				/*only one output channel for voice call */
				if (use)
					callUseHS = FALSE;

				break;

			case AudioUse:
				audioUseIHF = use;
				break;

			case FmUse:
				fmUseIHF = use;
				break;

			default:
				break;
			}
			break;

		default:
			return;
		}

		aTrace(LOG_AUDIO_CNTLR, "fmUseIHF %d, audioUseIHF %d,"
				" callUseIHF %d,"
				" fmUseHS %d, audioUseHS %d, callUseHS %d\n",
				fmUseIHF, audioUseIHF, callUseIHF,
				fmUseHS, audioUseHS, callUseHS);

		if ((callUseHS == FALSE) && (audioUseHS == FALSE)
		    && (fmUseHS == FALSE)) {
			if (HS_IsOn != FALSE) {
				aTrace(LOG_AUDIO_CNTLR,
					"power OFF pmu HS amp\n");

				extern_hs_off();
			}
			HS_IsOn = FALSE;
		} else {
			if (HS_IsOn != TRUE && ampControl == TRUE) {
				aTrace(LOG_AUDIO_CNTLR,
					"powerOnExternalAmp power on HS");
				extern_hs_on();
			}

			setExternAudioGain(GetAudioModeBySink(speaker),
				AUDCTRL_GetAudioApp());

			HS_IsOn = TRUE;
		}

		if ((callUseIHF == FALSE) && (audioUseIHF == FALSE)
		    && (fmUseIHF == FALSE)) {
			if (IHF_IsOn != FALSE) {
				aTrace(LOG_AUDIO_CNTLR,
					"power OFF pmu IHF amp\n");
				extern_ihf_off();
			}
			IHF_IsOn = FALSE;
		} else {
			if (IHF_IsOn != TRUE && ampControl == TRUE) {
				aTrace(LOG_AUDIO_CNTLR,
					"powerOnExternalAmp power on IHF");
				extern_ihf_on();
		}

		setExternAudioGain(GetAudioModeBySink(speaker),
			AUDCTRL_GetAudioApp());

		IHF_IsOn = TRUE;
	}
	}
}

/****************************************************************************
*
* Function Name: Set Extern Audio Gains
*
* Description:   Set external audio chip's gain
*
****************************************************************************/
static void setExternAudioGain(AudioMode_t mode, AudioApp_t app)
{
	int pmu_gain = 0;

#ifdef CONFIG_BCM_MODEM
	SysAudioParm_t *p;
#else
	AudioSysParm_t *p;
#endif

	p = &(AudParmP()[mode + app * AUDIO_MODE_NUMBER]);

	aTrace(LOG_AUDIO_CNTLR, "%s mode %d, app %d\n", __func__, mode, app);

	/*Load PMU gain from sysparm.For SS multicast case Headset gain
	 will be loaded from AUDIO_MODE_RESERVE
	*/
	switch (mode) {
	case AUDIO_MODE_HEADSET:
	case AUDIO_MODE_TTY:
#ifdef CONFIG_ENABLE_SSMULTICAST
	case AUDIO_MODE_RESERVE:
#endif
		pmu_gain = (short)p->ext_speaker_pga_l;	/* Q13p2 dB */
		extern_hs_set_gain(pmu_gain * 25, AUDIO_HS_LEFT);

		pmu_gain = (short)p->ext_speaker_pga_r;	/* Q13p2 dB */
		extern_hs_set_gain(pmu_gain * 25, AUDIO_HS_RIGHT);

		break;

	case AUDIO_MODE_SPEAKERPHONE:
		pmu_gain = (short)p->ext_speaker_pga_l;	/* Q13p2 dB */
		extern_ihf_set_gain(pmu_gain * 25);

		pmu_gain = (int)p->ext_speaker_high_gain_mode_enable;
		extern_ihf_en_hi_gain_mode(pmu_gain);

		break;

	default:
		break;
	}

}

/********************************************************************
*  @brief  Get MFD mode
*
*  @param  none
*
*  @return  Boolean
*
****************************************************************************/
Boolean AUDCTRL_GetMFDMode(void)
{
	return isMFD;
}

/********************************************************************
*  @brief  Get SRCMixer Clock rate
*
*  @param  none
*
*  @return  TRUE to use 26M clock; FALSE not (78M)
*
****************************************************************************/
Boolean AUDCTRL_GetSRCClock(void)
{
	return is26MClk;
}

static void fillUserVolSetting(AudioMode_t mode, AudioApp_t app)
{
#ifdef CONFIG_BCM_MODEM
	SysAudioParm_t *p;
#else
	AudioSysParm_t *p;
#endif

	p = &(AudParmP()[mode + app * AUDIO_MODE_NUMBER]);

	user_vol_setting[app][mode].L = (short)p->srcmixer_output_fine_gain_l;
	user_vol_setting[app][mode].L *= 25;
	user_vol_setting[app][mode].R = (short)p->srcmixer_output_fine_gain_r;
	user_vol_setting[app][mode].R *= 25;
	user_vol_setting[app][mode].valid = TRUE;
}
