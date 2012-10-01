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

#include "extern_audio.h"
#include "voif_handler.h"

#include "audctrl_policy.h"
#include "audio_trace.h"

#include <linux/err.h>
#include <linux/regulator/consumer.h>

#include "csl_dsp.h"

#include "taskmsgs.h"
#include "ipcproperties.h"

#include "rpc_global.h"
#include "rpc_ipc.h"
#include "xdr_porting_layer.h"
#include "xdr.h"
#include "rpc_api.h"

/**There are two loopback paths available in AudioH.
One is 6.5MHz analog microphone loopback path. It does not support digital mics.
The other one is HW sidetone path. It supports all the mics. This is prefered.
Only one loopback path can be enabled at a time.*/
#define HW_SIDETONE_LOOPBACK
#undef HW_ANALOG_LOOPBACK

#define EANBLE_POP_CONTROL

#define VIBRA_LDO_REGULATOR "2v9_vibra"
#define DIGI_MIC_LDO_REGULATOR "hv7"

#if defined(CONFIG_MFD_BCM59039) | defined(CONFIG_MFD_BCM59042)
#define TEMP_CHECK_DELAY_2MIN (2*60*1000)
#define TEMP_CHECK_DELAY_30SEC (30*1000)
#define TEMP_CHECK_DELAY_1MS 1
#define TEMP_HIGH 95
#define TEMP_VERY_HIGH 125
#define PMU_DELTA_T_RANGE  31
#define PMU_GAIN_RANGE  64

/** Private Type and Constant declarations */
struct PMU_DeltaTOutputLimitMapping_t {
	float PMU_output_limit;
	int deltaT;
};

static struct PMU_DeltaTOutputLimitMapping_t
	hsPMU_DeltaTOutputTable[PMU_DELTA_T_RANGE] = {
	/* Gain in milli Bel,      HS PMU Gain enum */
	{1.5100, 0},
	{1.4255, 1},
	{1.3458, 2},
	{1.2705, 3},
	{1.1994, 4},
	{1.1323, 5},
	{1.0690, 6},
	{1.0092, 7},
	{0.9527, 8},
	{0.8994, 9},
	{0.8491, 10},
	{0.8016, 11},
	{0.7568, 12},
	{0.7145, 13},
	{0.6745, 14},
	{0.6368, 15},
	{0.6011, 16},
	{0.5675, 17},
	{0.5358, 18},
	{0.5058, 19},
	{0.4775, 20},
	{0.4508, 21},
	{0.4256, 22},
	{0.4018, 23},
	{0.3793, 24},
	{0.3581, 25},
	{0.3380, 26},
	{0.3191, 27},
	{0.3013, 28},
	{0.2844, 29},
	{0.2685, 30}
};

struct PMU_GAINPartialcalulation_t {
	int pmu_gain;
	float PMU_thresh_partial_val;
};

static struct PMU_GAINPartialcalulation_t
	hsPMU_GAINPartialcalulationTable[PMU_GAIN_RANGE] = {
{	-10000	,	0.000024	},
{	-6800	,	0.000955457	},
{	-6500	,	0.001349619	},
{	-6200	,	0.001906388	},
{	-5900	,	0.002692844	},
{	-5600	,	0.003803744	},
{	-5300	,	0.005372931	},
{	-5000	,	0.007589466	},
{	-4700	,	0.010720406	},
{	-4400	,	0.015142976	},
{	-4250	,	0.017997461	},
{	-4100	,	0.021390023	},
{	-3950	,	0.025422089	},
{	-3800	,	0.03021421	},
{	-3650	,	0.035909656	},
{	-3500	,	0.042678706	},
{	-3350	,	0.050723737	},
{	-3200	,	0.060285274	},
{	-3050	,	0.071649183	},
{	-2900	,	0.085155213	},
{	-2750	,	0.101207161	},
{	-2600	,	0.120284936	},
{	-2450	,	0.142958914	},
{	-2400	,	0.151429763	},
{	-2350	,	0.16040254	},
{	-2300	,	0.169906988	},
{	-2250	,	0.17997461	},
{	-2200	,	0.190638776	},
{	-2150	,	0.201934834	},
{	-2100	,	0.213900225	},
{	-2050	,	0.22657461	},
{	-2000	,	0.24},
{	-1950	,	0.254220894	},
{	-1900	,	0.269284429	},
{	-1850	,	0.285240535	},
{	-1800	,	0.302142099	},
{	-1750	,	0.320045144	},
{	-1700	,	0.339009011	},
{	-1650	,	0.359096557	},
{	-1600	,	0.380374366	},
{	-1550	,	0.402912964	},
{	-1500	,	0.426787058	},
{	-1450	,	0.452075781	},
{	-1400	,	0.478862956	},
{	-1350	,	0.50723737	},
{	-1300	,	0.537293073	},
{	-1250	,	0.569129689	},
{	-1200	,	0.602852744	},
{	-1150	,	0.638574014	},
{	-1100	,	0.676411904	},
{	-1050	,	0.716491829	},
{	-1000	,	0.758946638	},
{	-950	,	0.803917054	},
{	-900	,	0.851552134	},
{	-850	,	0.90200977	},
{	-800	,	0.955457209	},
{	-750	,	1.012071608	},
{	-700	,	1.072040621	},
{	-650	,	1.135563022	},
{	-600	,	1.202849361	},
{	-550	,	1.274122666	},
{	-500	,	1.34961918	},
{	-450	,	1.429589144	},
{	-400	,	1.514297627	}
};
#endif

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
	[AUDIO_SINK_HANDSET] = {AUDIO_SINK_HANDSET, CSL_CAPH_DEV_EP},
	[AUDIO_SINK_HEADSET] = {AUDIO_SINK_HEADSET, CSL_CAPH_DEV_HS},
	[AUDIO_SINK_HANDSFREE] = {AUDIO_SINK_HANDSFREE, CSL_CAPH_DEV_IHF},
	[AUDIO_SINK_BTM] = {AUDIO_SINK_BTM, CSL_CAPH_DEV_BT_SPKR},
	[AUDIO_SINK_LOUDSPK] = {AUDIO_SINK_LOUDSPK, CSL_CAPH_DEV_IHF},
	[AUDIO_SINK_TTY] = {AUDIO_SINK_TTY, CSL_CAPH_DEV_HS},
	[AUDIO_SINK_HAC] = {AUDIO_SINK_HAC, CSL_CAPH_DEV_EP},
	[AUDIO_SINK_USB] = {AUDIO_SINK_USB, CSL_CAPH_DEV_MEMORY},
	[AUDIO_SINK_BTS] = {AUDIO_SINK_BTS, CSL_CAPH_DEV_BT_SPKR},
	[AUDIO_SINK_I2S] = {AUDIO_SINK_I2S, CSL_CAPH_DEV_FM_TX},
	[AUDIO_SINK_VIBRA] = {AUDIO_SINK_VIBRA, CSL_CAPH_DEV_VIBRA},
	[AUDIO_SINK_HEADPHONE] = {AUDIO_SINK_HEADPHONE, CSL_CAPH_DEV_HS},
	[AUDIO_SINK_VALID_TOTAL] = {AUDIO_SINK_VALID_TOTAL, CSL_CAPH_DEV_NONE},
	[AUDIO_SINK_MEM] = {AUDIO_SINK_MEM, CSL_CAPH_DEV_MEMORY},
	[AUDIO_SINK_DSP] = {AUDIO_SINK_DSP, CSL_CAPH_DEV_DSP},
	[AUDIO_SINK_RESERVED1] = {AUDIO_SINK_RESERVED1, CSL_CAPH_DEV_NONE},
	[AUDIO_SINK_RESERVED2] = {AUDIO_SINK_RESERVED2, CSL_CAPH_DEV_NONE},
	[AUDIO_SINK_RESERVED3] = {AUDIO_SINK_RESERVED3, CSL_CAPH_DEV_NONE},
	[AUDIO_SINK_RESERVED4] = {AUDIO_SINK_RESERVED4, CSL_CAPH_DEV_NONE},
	[AUDIO_SINK_UNDEFINED] = {AUDIO_SINK_UNDEFINED, CSL_CAPH_DEV_NONE}
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
static int bNeedDualMic = FALSE;

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

static Boolean isFmMuted = FALSE;

#if defined(CONFIG_MFD_BCM59039) | defined(CONFIG_MFD_BCM59042)
Boolean gtemp = 30;
Boolean gtempGainCompEnable = 1;
#endif

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
static unsigned int pathIDTuning;	/* init to 0, for tuning purpose only */

static AudioApp_t currAudioApp = AUDIO_APP_VOICE_CALL;
static AudioApp_t currAudioAppUser = AUDIO_APP_MUSIC;
static AudioMode_t currAudioMode = AUDIO_MODE_HANDSET;
 /* need to update this on AP and also in audioapi.c on CP. */
static AudioMode_t currAudioMode_playback = AUDIO_MODE_SPEAKERPHONE;
static AudioMode_t currAudioMode_record = AUDIO_MODE_SPEAKERPHONE;
static AudioMode_t currAudioMode_fm = AUDIO_MODE_HEADSET;

static struct regulator *vibra_reg;
static struct regulator *digi_mic_reg;

/*wait in us, to avoid hs/ihf pop noise*/
static int wait_bb_on;
static int wait_hspmu_on = 10*1000;
static int wait_ihfpmu_on;
static int wait_pmu_off = 2*1000;

static BRCM_AUDIO_Param_Second_Dev_t second_dev_info;

static int isDigiMic(AUDIO_SOURCE_Enum_t source);
static int needDualMic(AudioMode_t mode, AudioApp_t app);
static AudioMode_t GetAudioModeFromCaptureDev(CSL_CAPH_DEVICE_e source);
static void powerOnExternalAmp(AUDIO_SINK_Enum_t speaker,
			       enum ExtSpkrUsage_en_t usage_flag,
			       int use, int force);
static void setExternAudioGain(AudioMode_t mode, AudioApp_t app);
static void fillUserVolSetting(AudioMode_t mode, AudioApp_t app);

static void audctl_usleep_range(int min, int max)
{
	if (min)
		usleep_range(min, max);
}

static void audioh_start_hs(void)
{
#if defined(EANBLE_POP_CONTROL)
	csl_caph_ControlHWClock(TRUE);
	csl_caph_audioh_start_hs();
#endif
	audctl_usleep_range(wait_bb_on, wait_bb_on+1000);
}

static void audioh_start_ihf(void)
{
#if defined(EANBLE_POP_CONTROL)
	csl_caph_ControlHWClock(TRUE);
	csl_caph_audioh_start_ihf();
#endif
	audctl_usleep_range(wait_bb_on, wait_bb_on+2000);
}

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
* Function Name: AUDCTRL_FinalizeAudioApp
*
* Description:   get audio application.
*
****************************************************************************/
static AudioApp_t AUDCTRL_FinalizeAudioApp(AudioMode_t mode)
{
	AudioApp_t app = AUDCTRL_GetAudioApp();

	if (mode == AUDIO_MODE_BLUETOOTH) {
		/* need to figure out App when use Bluetooth headset. */
		if (FALSE == AUDCTRL_IsBTMWB()
		&& app == AUDIO_APP_VOICE_CALL_WB)
			app = AUDIO_APP_VOICE_CALL;

		if (TRUE == AUDCTRL_IsBTMWB()
		&& app == AUDIO_APP_VOICE_CALL)
			app = AUDIO_APP_VOICE_CALL_WB;
	} else {
		/*when switch from BT headset to handset mode,
		need to figure out app */
		if (voiceCallSampleRate == AUDIO_SAMPLING_RATE_16000
		&& app == AUDIO_APP_VOICE_CALL)
			app = AUDIO_APP_VOICE_CALL_WB;

		if (voiceCallSampleRate == AUDIO_SAMPLING_RATE_8000
		&& app == AUDIO_APP_VOICE_CALL_WB)
			app = AUDIO_APP_VOICE_CALL;
	}
	AUDCTRL_SaveAudioApp(app);

	return currAudioApp;
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

	/*Toggle all SRCMixer/AudioH clk*/
	csl_caph_hwctrl_toggle_caphclk();
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
	AudioApp_t app = AUDCTRL_GetAudioApp();

	aTrace(LOG_AUDIO_CNTLR, "%s sink %d, mic %d\n", __func__, sink, source);

	mode = GetAudioModeBySink(sink);
	app = AUDCTRL_FinalizeAudioApp(mode);

	if (user_vol_setting[app][mode].valid == FALSE)
		fillUserVolSetting(mode, app);

	bNeedDualMic = needDualMic(mode, app);

	if (AUDCTRL_InVoiceCall()) {	/*already in voice call */
		if ((voiceCallSpkr != sink) || (voiceCallMic != source))
			AUDCTRL_SetTelephonyMicSpkr(source, sink);

		return;
	}

	if (isDigiMic(source) || bNeedDualMic) {
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
		if (isDigiMic(voiceCallMic) || bNeedDualMic)
			powerOnDigitalMic(FALSE);

		voiceCallSpkr = AUDIO_SINK_UNDEFINED;
		voiceCallMic = AUDIO_SOURCE_UNDEFINED;
	}

	bNeedDualMic = FALSE;

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
	AudioApp_t app = AUDCTRL_GetAudioApp();

	aTrace(LOG_AUDIO_CNTLR, "%s sample_rate %d-->%d",
		__func__, voiceCallSampleRate, sample_rate);

	if (voiceCallSampleRate == sample_rate)
		return;

	voiceCallSampleRate = sample_rate;
	/* remember the rate for current call.
	   (or for the incoming call in ring state.) */

	if (AUDCTRL_InVoiceCall()) {

		mode = AUDCTRL_GetAudioMode();
		app = AUDCTRL_FinalizeAudioApp(mode);
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

	aTrace(LOG_AUDIO_CNTLR, "%s sink %d, mic %d\n", __func__, sink, source);

	if (voiceCallMic == source && voiceCallSpkr == sink)
		return;

	if (source == AUDIO_SOURCE_USB || sink == AUDIO_SINK_USB)
		return;

	mode = GetAudioModeBySink(sink);
	app = AUDCTRL_FinalizeAudioApp(mode);

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
	bNeedDualMic = needDualMic(mode, app);

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


	if (voiceCallSpkr != sink)
		powerOnExternalAmp(voiceCallSpkr, TelephonyUse,
				FALSE, FALSE);
	if (voiceCallMic != source || bNeedDualMic) {
		if (isDigiMic(voiceCallMic)) {
			/* Disable power to digital microphone */
			powerOnDigitalMic(FALSE);
		}
	}

	if (voiceCallMic != source || bNeedDualMic) {
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

#ifdef CONFIG_BCM_MODEM
		telephony_dl_gain_dB = p->dsp_voice_vol_tbl[volume];
		/*values in table are in range of 0 ~ 36 dB.
		   shift to range of -36 ~ 0 dB in DSP */
		telephony_dl_gain_dB -= p->voice_volume_max;
#else
		telephony_dl_gain_dB = 0;
#endif

	}

	audio_control_generic(AUDDRV_CPCMD_SetOmegaVoiceParam,
	(mode + app * AUDIO_MODE_NUMBER), volume, 0, 0, 0);

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
	aTrace(LOG_AUDIO_CNTLR, "SetUserAudioApp new app=%d\n", app);

	currAudioAppUser = app;
}

/****************************************************************************
*
* Function Name: AUDCTRL_GetUserAudioApp
*
* Description:   Get audio application.
*
****************************************************************************/
AudioApp_t AUDCTRL_GetUserAudioApp(void)
{
	aTrace(LOG_AUDIO_CNTLR, "GetUserAudioApp currAudioAppUser=%d",
			currAudioAppUser);

	return currAudioAppUser;
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
	AUDCTRL_SaveAudioMode(mode);
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

	if (!AUDCTRL_InVoiceCall()) {
		/*for music tuning, if PCG changed audio mode,
		   need to pass audio mode to CP */
		RPC_SetProperty(RPC_PROP_AUDIO_MODE,
			(UInt32)(((int)AUDCTRL_GetAudioApp()) *
			AUDIO_MODE_NUMBER + mode));
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

	currAudioMode_fm = mode;

	app = AUDCTRL_GetAudioApp();
	sp_struct.mode = mode;
	sp_struct.app = app;
	sp_struct.pathID = arg_pathID;
	sp_struct.inHWlpbk = inHWlpbk;
	sp_struct.mixOutGain_mB = GAIN_SYSPARM;
	sp_struct.mixOutGainR_mB = GAIN_SYSPARM;
	if (isFmMuted == FALSE) {
		if (users_gain[AUDPATH_FM].valid) {
			/*do not apply FM mixer input gain to music*/
			if (path->source == CSL_CAPH_DEV_MEMORY) {
				sp_struct.mixInGain_mB = GAIN_SYSPARM;
				sp_struct.mixInGainR_mB = GAIN_SYSPARM;
			} else {
				sp_struct.mixInGain_mB =
					users_gain[AUDPATH_FM].L;
				sp_struct.mixInGainR_mB =
					users_gain[AUDPATH_FM].R;
			}
		} else {
			if (muteInPlay) {
				sp_struct.mixInGain_mB = GAIN_NA;
				sp_struct.mixInGainR_mB = GAIN_NA;
			} else {
				sp_struct.mixInGain_mB = GAIN_SYSPARM;
				sp_struct.mixInGainR_mB = GAIN_SYSPARM;
			}
		}
	} else {
		aTrace(LOG_AUDIO_CNTLR,
			"FM IS MUTED APPLY GAIN_NA\n");
		sp_struct.mixInGain_mB = GAIN_NA;
		sp_struct.mixInGainR_mB = GAIN_NA;
	}

	if (user_vol_setting[app][mode].valid == FALSE)
		fillUserVolSetting(mode, app);
	sp_struct.mixOutGain_mB = user_vol_setting[app][mode].L;
	sp_struct.mixOutGainR_mB = user_vol_setting[app][mode].R;

	AUDDRV_SetAudioMode_Speaker(sp_struct);

	if (!AUDCTRL_InVoiceCall()) {
		/*for music tuning, if PCG changed audio mode,
		   need to pass audio mode to CP in audio_vdriver_caph.c */
		RPC_SetProperty(RPC_PROP_AUDIO_MODE,
			(UInt32)(((int)app *
			AUDIO_MODE_NUMBER + mode)));
	}

	setExternAudioGain(mode, app);

	if (!bClk)
		csl_caph_ControlHWClock(FALSE);
	/*disable clock if it is enabled by this function. */
}

#ifdef CONFIG_ENABLE_SSMULTICAST
/*********************************************************************
*	Set audio mode for FM radio multicast playback
*	@param          mode            audio mode
*	@return         none
*
**********************************************************************/
void AUDCTRL_SetAudioMode_ForFM_Multicast(AudioMode_t mode,
				   unsigned int arg_pathID, Boolean inHWlpbk)
{
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

	currAudioMode_fm = AUDIO_MODE_SPEAKERPHONE;

	app = AUDCTRL_GetAudioApp();
	sp_struct.mode = AUDIO_MODE_SPEAKERPHONE;
	sp_struct.app = app;
	sp_struct.pathID = arg_pathID;
	sp_struct.inHWlpbk = inHWlpbk;
	sp_struct.mixOutGain_mB = GAIN_SYSPARM;
	sp_struct.mixOutGainR_mB = GAIN_SYSPARM;
	if (users_gain[AUDPATH_FM].valid) {
		/*do not apply FM mixer input gain to music*/
		if (path->source == CSL_CAPH_DEV_MEMORY) {
			sp_struct.mixInGain_mB = GAIN_SYSPARM;
			sp_struct.mixInGainR_mB = GAIN_SYSPARM;
		} else {
			sp_struct.mixInGain_mB = users_gain[AUDPATH_FM].L;
			sp_struct.mixInGainR_mB = users_gain[AUDPATH_FM].R;
		}
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

	/*Reload HS and IHF parametes from mode SPEAKERPHONE
	and HS ext-amp gain from mode RESERVED,app MUSIC */

	AUDDRV_SetAudioMode_Multicast(sp_struct);
	sp_struct.mode = AUDIO_MODE_HEADSET;
	AUDDRV_SetAudioMode_Multicast(sp_struct);

	setExternAudioGain(AUDIO_MODE_RESERVE, AUDIO_APP_MUSIC);
	AUDCTRL_SaveAudioMode(AUDIO_MODE_SPEAKERPHONE);

	if (!AUDCTRL_InVoiceCall()) {
		/*for music tuning, if PCG changed audio mode,
		   need to pass audio mode to CP in audio_vdriver_caph.c */
			RPC_SetProperty(RPC_PROP_AUDIO_MODE,
				(UInt32)(((int)app *
				AUDIO_MODE_NUMBER + mode)));
	}

	if (!bClk)
		csl_caph_ControlHWClock(FALSE);
	/*disable clock if it is enabled by this function. */
}

#endif

/*********************************************************************
*	Set audio mode for music record
*	@param          mode            audio mode
*	@return         none
*
**********************************************************************/
void AUDCTRL_SetAudioMode_ForMusicRecord(
	AudioMode_t mode, unsigned int arg_pathID)
{
	Boolean bClk = csl_caph_QueryHWClock();
	AudioApp_t app;

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

/*no PMU
for FM recording + voice call, need to find separate gains from sysparm
 and HW paths
also need to support audio profile (and/or mode) set from user space code
 to support multi-profile/app.
*/

	currAudioMode_record = mode;
	app = AUDCTRL_GetAudioApp();

	AUDDRV_SetAudioMode_Mic(mode, app, arg_pathID, 0);

	if (!AUDCTRL_InVoiceCall()) {
		/*for music tuning, if PCG changed audio mode,
		   need to pass audio mode to CP in audio_vdriver_caph.c */
		RPC_SetProperty(RPC_PROP_AUDIO_MODE,
			(UInt32)(((int)app *
			AUDIO_MODE_NUMBER + mode)));
	}

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
void AUDCTRL_SetAudioMode_ForMusicMulticast(AudioMode_t mode,
					unsigned int arg_pathID)
{
	Boolean bClk = csl_caph_QueryHWClock();
	SetAudioMode_Sp_t sp_struct;

	aTrace(LOG_AUDIO_CNTLR,
			"%s mode %d", __func__, mode);

	if (AUDCTRL_InVoiceCall())
		return;	/*don't affect voice call audio mode */

	memset(&sp_struct, 0, sizeof(sp_struct));
	if (!bClk)
		csl_caph_ControlHWClock(TRUE);
	/*enable clock if it is not enabled. */

	/*For SS Multicast to IHF+HS, mode will be always Speakerphone*/
	currAudioMode_playback = AUDIO_MODE_SPEAKERPHONE;

	sp_struct.app = AUDCTRL_GetAudioApp();
	sp_struct.pathID = arg_pathID;
	sp_struct.inHWlpbk = FALSE;

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
		j = AUDIO_MODE_SPEAKERPHONE;
		if (user_vol_setting[i][j].valid == FALSE)
			fillUserVolSetting(j, i);
		sp_struct.mixOutGain_mB = user_vol_setting[i][j].L;
		sp_struct.mixOutGainR_mB = user_vol_setting[i][j].R;
	}

	switch (currAudioMode) {

	case AUDIO_MODE_HEADSET:
	if (mode == AUDIO_MODE_SPEAKERPHONE) {
		/*adding IHF reload HS param*/
		sp_struct.mode = AUDIO_MODE_HEADSET;
		AUDDRV_SetAudioMode_Multicast(sp_struct);

		/*Load HS params from mode RESERVED*/
		setExternAudioGain(AUDIO_MODE_RESERVE, AUDCTRL_GetAudioApp());
		sp_struct.mode = mode;
		AUDDRV_SetAudioMode_Multicast(sp_struct);
	}
	break;

	case AUDIO_MODE_SPEAKERPHONE:
	if (mode == AUDIO_MODE_HEADSET) {
		/*Adding HS load HS param*/
		sp_struct.mode = mode;
		AUDDRV_SetAudioMode_Multicast(sp_struct);
		/*Load HS params from mode RESERVED*/
		setExternAudioGain(AUDIO_MODE_RESERVE, AUDCTRL_GetAudioApp());
	}
	break;

	/*Multicasting to BT+IHF or any other
	right now for any BT+IHF case we will not end up here*/
	default:

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
		/*must set a default, o.w. it would be used uninitialized*/
		*pMic = AUDIO_SOURCE_UNDEFINED;
		*pSpk = AUDIO_SINK_UNDEFINED;
		aWarn(
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
	enum ExtSpkrUsage_en_t usage_flag = AudioUse;
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

	if (source == AUDIO_SOURCE_I2S && AUDCTRL_InVoiceCall() == FALSE) {
		aTrace(LOG_AUDIO_CNTLR,
				"%s FM src %d, sink %d", __func__,
				source, sink);
		powerOnExternalAmp(sink, FmUse,
				TRUE, FALSE);
	} else {
		if (sr != AUDIO_PCMOUT1_RATE)
			/* goes to pcmout2 */
			usage_flag = Audio2Use;

		if (second_dev_info.sink == AUDIO_SINK_VALID_TOTAL)
			/* only power on amp in PMU for the first sink */
			powerOnExternalAmp(sink, usage_flag,
					TRUE, FALSE);
		else
		if (sink != AUDIO_SINK_LOUDSPK && sink != AUDIO_SINK_HEADSET
		  && sink != AUDIO_SINK_TTY && sink != AUDIO_SINK_HANDSFREE)
			/* only power on amp in PMU for the second sink */
			powerOnExternalAmp(second_dev_info.sink, usage_flag,
				TRUE, FALSE);
		else {
			/* power on HS amp and IHF amp in PMU */

			wait_hspmu_on = 0;

			powerOnExternalAmp(AUDIO_SINK_HEADSET, usage_flag,
					TRUE, FALSE);
			powerOnExternalAmp(AUDIO_SINK_LOUDSPK, usage_flag,
					TRUE, FALSE);
			wait_hspmu_on = 10*1000;

		}

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
	enum ExtSpkrUsage_en_t usage_flag = AudioUse;
	CSL_CAPH_HWConfig_Table_t *pPath = NULL;
	int ret = 0;

	memset(&config, 0, sizeof(CSL_CAPH_HWCTRL_CONFIG_t));
	aTrace(LOG_AUDIO_CNTLR, "%s src %d, sink %d, pathID %d",
		__func__, source, sink, pathID);

	if (pathID == 0) {
		audio_xassert(0, pathID);
		return;
	}

	if (pathID > MAX_AUDIO_PATH)
		return;

	/* get this pathID sr to determine which pcmout used*/
	pPath = csl_caph_FindPath(pathID);
	if ((pPath != NULL)
		&& (pPath->src_sampleRate != AUDIO_PCMOUT1_RATE))
		usage_flag = Audio2Use;

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

#if !defined(EANBLE_POP_CONTROL)
	if ((source != AUDIO_SOURCE_DSP && sink == AUDIO_SINK_USB)
	    || sink == AUDIO_SINK_BTS)
		;
	else {
		config.pathID = pathID;
		(void)csl_caph_hwctrl_DisablePath(config);
	}
#endif

	/* Need CSL API to obtain the pathID from the same speaker info.
	   This is to make sure that PMU is not disabled if any other
	   path is using the same speaker */

	path = csl_caph_FindPathID(sink_dev, src_dev, pathID);

	/*Disable the PMU for HS/IHF */
	/*if (path) {
		aTrace(LOG_AUDIO_CNTLR,
				"%s pathID %d use the ext amp.", __func__,
				path);
	} else */{
		if (source == AUDIO_SOURCE_I2S
		&& AUDCTRL_InVoiceCall() == FALSE) {
			aTrace(LOG_AUDIO_CNTLR,
					"%s FM src %d, sink %d",
					__func__, source, sink);

			powerOnExternalAmp(sink, FmUse, FALSE, FALSE);
			fmPlayStarted = FALSE;
		} else if ((sink == AUDIO_SINK_HEADSET)
			   || (sink == AUDIO_SINK_LOUDSPK)) {
			powerOnExternalAmp(sink, usage_flag, FALSE, FALSE);
		}
	}
#if defined(EANBLE_POP_CONTROL)
	if ((source != AUDIO_SOURCE_DSP && sink == AUDIO_SINK_USB)
	    || sink == AUDIO_SINK_BTS)
		;
	else {
		config.pathID = pathID;
		(void)csl_caph_hwctrl_DisablePath(config);
	}
#endif
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
*  Function Name: AUDCTRL_SetSecondSink
*
*  Description: Set the second sink.
*      Must call this funciton before call AUDCTRL_EnablePlay for this
*      function to take effect in AUDCTRL_EnablePlay.
*      AUDCTRL_EnablePlay will power on both HS amp and LoudSpeaker amp
*      for multicast audio.
*
****************************************************************************/
void AUDCTRL_SetSecondSink(BRCM_AUDIO_Param_Second_Dev_t info)
{
	second_dev_info = info;
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

	/*arm2sp may use HW mixer, whose gain should be set */
	AUDCTRL_SetAudioMode_ForMusicPlayback(mode, path->pathID, FALSE);
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
#ifdef CONFIG_BCM_MODEM
		users_gain[AUDPATH_FM].L = p->fm_radio_digital_vol[vol_left];
		/*users_gain[AUDPATH_FM].R =
		p->fm_radio_digital_vol[vol_right];*/
		/*vol_right is always 0. need to fix it in caph_ctl.c*/
		users_gain[AUDPATH_FM].R = p->fm_radio_digital_vol[vol_left];
#else
		users_gain[AUDPATH_FM].L = 0;
		users_gain[AUDPATH_FM].R = 0;
#endif
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

	/*if the CAPH clock is not turned on.
	do not set HW audio gain.*/
	if (FALSE == csl_caph_QueryHWClock())
		return;

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
				/*do not mute music even FM is muted*/
				if (source != AUDIO_SOURCE_I2S)
					muteInPlay = TRUE;
				else
					isFmMuted = TRUE;
			}
		}
	} else {
		if (sink == AUDIO_SINK_VIBRA)
			csl_caph_hwctrl_UnmuteSink(0 /*pathID */ , speaker);
		else {
			if (mixInCh != CSL_CAPH_SRCM_INCHNL_NONE) {
				/*set CAPH gain, Q13p2 dB */
				if (source == AUDIO_SOURCE_I2S) {
					mixInGain = users_gain[AUDPATH_FM].L;
					mixInGainR = users_gain[AUDPATH_FM].R;
				} else {
				mixInGain = (short)p->srcmixer_input_gain_l;
				mixInGain = mixInGain * 25; /* into mB */
				mixInGainR = (short)p->srcmixer_input_gain_r;
				mixInGainR = mixInGainR * 25; /* into mB */
				}

				csl_srcmixer_setMixInGain(mixInCh, mixer,
					mixInGain, mixInGainR);
				if (source != AUDIO_SOURCE_I2S)
					muteInPlay = FALSE;
				else
					isFmMuted = FALSE;
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
	enum ExtSpkrUsage_en_t usage_flag = AudioUse;
	CSL_CAPH_HWConfig_Table_t *pPath = NULL;

	aTrace(LOG_AUDIO_CNTLR,
			"%s src 0x%x, Sink 0x%x", __func__, source, sink);
	if (pathID == 0) {
		audio_xassert(0, pathID);
		return;
	}

	if (sink >= AUDIO_SINK_UNDEFINED) {
		audio_xassert(0, sink);
		return;
	}

	memset(&config, 0, sizeof(CSL_CAPH_HWCTRL_CONFIG_t));

	/* get this pathID sr to determine which pcmout used*/
	pPath = csl_caph_FindPath(pathID);
	if ((pPath != NULL)
		&& (pPath->src_sampleRate != AUDIO_PCMOUT1_RATE))
		usage_flag = Audio2Use;
	curr_spk = csl_caph_FindSinkDevice(pathID);
	aTrace(LOG_AUDIO_CNTLR,
			"curr_spk %d, new_spk %d\n", curr_spk, new_spk);

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
			powerOnExternalAmp(AUDIO_SINK_HEADSET, usage_flag,
					   FALSE, FALSE);
		else if (curr_spk == CSL_CAPH_DEV_IHF)
			powerOnExternalAmp(AUDIO_SINK_LOUDSPK, usage_flag,
					   FALSE, FALSE);
	}

#if defined(EANBLE_POP_CONTROL)
	if (source == AUDIO_SOURCE_I2S && AUDCTRL_InVoiceCall() == FALSE
		&& fmPlayStarted == TRUE) {
		if (sink == AUDIO_SINK_LOUDSPK || sink == AUDIO_SINK_HEADSET)
			powerOnExternalAmp(sink, FmUse, TRUE, FALSE);
	} else {
		if (sink == AUDIO_SINK_LOUDSPK || sink == AUDIO_SINK_HEADSET)
			powerOnExternalAmp(sink, usage_flag, TRUE, FALSE);
	}
#endif

	/* remove current spk first*/
	if (curr_spk != CSL_CAPH_DEV_NONE) {
		config.source = getDeviceFromSrc(source);
		config.sink = curr_spk;
		(void)csl_caph_hwctrl_RemovePath(pathID, config);
	}
	/* add new spk ... */
	if (getDeviceFromSink(sink) != CSL_CAPH_DEV_NONE) {
		config.source = getDeviceFromSrc(source);
		config.sink = getDeviceFromSink(sink);
		(void)csl_caph_hwctrl_AddPath(pathID, config);
	}


	if (source == AUDIO_SOURCE_I2S && AUDCTRL_InVoiceCall() == FALSE
		&& fmPlayStarted == TRUE) {
#if !defined(EANBLE_POP_CONTROL)
		if (sink == AUDIO_SINK_LOUDSPK || sink == AUDIO_SINK_HEADSET)
			powerOnExternalAmp(sink, FmUse, TRUE, FALSE);
#endif
		AUDCTRL_SetAudioMode_ForFM(
			GetAudioModeBySink(sink), pathID, FALSE);
	} else {
#if !defined(EANBLE_POP_CONTROL)
		if (sink == AUDIO_SINK_LOUDSPK || sink == AUDIO_SINK_HEADSET)
			powerOnExternalAmp(sink, usage_flag, TRUE, FALSE);
#endif
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
	enum ExtSpkrUsage_en_t usage_flag = AudioUse;
	CSL_CAPH_HWConfig_Table_t *pPath = NULL;

	aTrace(LOG_AUDIO_CNTLR, "%s src %d, sink %d,"
			"pathID %d", __func__, source, sink, pathID);

	/*if(pathID == 0)
	   {
	   audio_xassert(0,pathID);
	   return;
	   } */

	memset(&config, 0, sizeof(CSL_CAPH_HWCTRL_CONFIG_t));

	/* get this pathID sr to determine which pcmout used*/
	pPath = csl_caph_FindPath(pathID);
	if ((pPath != NULL)
		&& (pPath->src_sampleRate != AUDIO_PCMOUT1_RATE))
		usage_flag = Audio2Use;

	speaker = getDeviceFromSink(sink);
	if (speaker != CSL_CAPH_DEV_NONE) {
		/*Enable the PMU for HS/IHF. */
		if ((sink == AUDIO_SINK_LOUDSPK)
		    || (sink == AUDIO_SINK_HEADSET))

			powerOnExternalAmp(sink, usage_flag, TRUE, FALSE);

		config.source = getDeviceFromSrc(source);
		config.sink = speaker;
		(void)csl_caph_hwctrl_AddPath(pathID, config);
	}

	if (AUDIO_APP_FM == AUDCTRL_GetAudioApp()) {
		if (isStIHF &&
			currAudioMode_fm == AUDIO_MODE_SPEAKERPHONE &&
			sink == AUDIO_SINK_HANDSET)
			AUDCTRL_SetAudioMode_ForFM(
				AUDIO_MODE_SPEAKERPHONE, pathID, FALSE);
		else {
#ifndef CONFIG_ENABLE_SSMULTICAST
			AUDCTRL_SetAudioMode_ForFM(
				GetAudioModeBySink(sink), pathID, FALSE);
#else
			AUDCTRL_SetAudioMode_ForFM_Multicast(
				GetAudioModeBySink(sink), pathID, FALSE);
#endif
		}
	} else {
		if (isStIHF &&
			currAudioMode_playback == AUDIO_MODE_SPEAKERPHONE &&
			sink == AUDIO_SINK_HANDSET)
				AUDCTRL_SetAudioMode_ForMusicPlayback(
				AUDIO_MODE_SPEAKERPHONE, pathID, FALSE);
		else
#ifndef CONFIG_ENABLE_SSMULTICAST
				AUDCTRL_SetAudioMode_ForMusicPlayback(
				GetAudioModeBySink(sink), pathID, FALSE);
#else
				/*Revisit for stIHF case*/
				AUDCTRL_SetAudioMode_ForMusicMulticast(
				GetAudioModeBySink(sink), pathID);
#endif
	}
	return;

}

/****************************************************************************
*
* Function Name: AUDCTRL_AddPlaySpk_InPMU
*
* Description:   add a speaker by indicating to the PMU
*
****************************************************************************/

void AUDCTRL_AddPlaySpk_InPMU(AUDIO_SOURCE_Enum_t source,
			AUDIO_SINK_Enum_t sink, unsigned int pathID)
{
	CSL_CAPH_HWCTRL_CONFIG_t config;
	CSL_CAPH_DEVICE_e speaker = CSL_CAPH_DEV_NONE;

	aTrace(LOG_AUDIO_CNTLR, "%s src %d, sink %d,"
			"pathID %d", __func__, source, sink, pathID);

	memset(&config, 0, sizeof(CSL_CAPH_HWCTRL_CONFIG_t));

	speaker = getDeviceFromSink(sink);

	if (speaker != CSL_CAPH_DEV_NONE) {
		/*Enable the PMU for HS/IHF. */
		if (sink == AUDIO_SINK_LOUDSPK) {

			aTrace(LOG_AUDIO_CNTLR, "HS -> HS+IHF:Multicast"
				"Indication\n");

			multicastToSpkr(TRUE);

		} else if (sink == AUDIO_SINK_HEADSET) {

			aTrace(LOG_AUDIO_CNTLR, "IHF->IHF+HS: Switch"
				"and Multicast\n");

			AUDCTRL_SwitchPlaySpk(source, AUDIO_SINK_HEADSET,
					      pathID);

			multicastToSpkr(TRUE);
		}
	}

	if (AUDIO_APP_FM == AUDCTRL_GetAudioApp()) {
		if (isStIHF &&
			currAudioMode_fm == AUDIO_MODE_SPEAKERPHONE &&
			sink == AUDIO_SINK_HANDSET)
			AUDCTRL_SetAudioMode_ForFM(
				AUDIO_MODE_SPEAKERPHONE, pathID, FALSE);
		else {
#ifndef CONFIG_ENABLE_SSMULTICAST
			AUDCTRL_SetAudioMode_ForFM(
				GetAudioModeBySink(sink), pathID, FALSE);
#else


			AUDCTRL_SetAudioMode_ForFM_Multicast(
				GetAudioModeBySink(sink), pathID, FALSE);
#endif
		}
	} else {
		if (isStIHF &&
			currAudioMode_playback == AUDIO_MODE_SPEAKERPHONE &&
			sink == AUDIO_SINK_HANDSET)
				AUDCTRL_SetAudioMode_ForMusicPlayback(
				AUDIO_MODE_SPEAKERPHONE, pathID, FALSE);
		else
#ifndef CONFIG_ENABLE_SSMULTICAST
				AUDCTRL_SetAudioMode_ForMusicPlayback(
				GetAudioModeBySink(sink), pathID, FALSE);
#else
				/*Revisit for stIHF case*/

				AUDCTRL_SetAudioMode_ForMusicMulticast(
				GetAudioModeBySink(sink), pathID);
#endif
	}
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
	enum ExtSpkrUsage_en_t usage_flag = AudioUse;
	CSL_CAPH_HWConfig_Table_t *pPath = NULL;

	aTrace(LOG_AUDIO_CNTLR, "%s src 0x%x, sink 0x%x", __func__,
			source, sink);

	if (pathID == 0) {
		audio_xassert(0, pathID);
		return;
	}

	pPath = csl_caph_FindPath(pathID);
	if ((pPath != NULL)
		&& (pPath->src_sampleRate != AUDIO_PCMOUT1_RATE))
		usage_flag = Audio2Use;

	speaker = getDeviceFromSink(sink);
	if (speaker != CSL_CAPH_DEV_NONE) {
		/*Disable the PMU for HS/IHF. */
		if ((sink == AUDIO_SINK_LOUDSPK)
		    || (sink == AUDIO_SINK_HEADSET))
			powerOnExternalAmp(sink, usage_flag, FALSE, FALSE);
		config.source = getDeviceFromSrc(source);
		config.sink = speaker;
		(void)csl_caph_hwctrl_RemovePath(pathID, config);

#ifdef CONFIG_ENABLE_SSMULTICAST
		/*If IHF removed reload HS params with mode HEADSET*/
		if (sink == AUDIO_SINK_LOUDSPK &&
			(currAudioMode_playback == AUDIO_MODE_SPEAKERPHONE ||
			currAudioMode_fm == AUDIO_MODE_SPEAKERPHONE)) {
			int i, j;
			sp_struct.mode = AUDIO_MODE_HEADSET;
			sp_struct.app = AUDCTRL_GetAudioApp();
			sp_struct.pathID = pathID;
			sp_struct.inHWlpbk = FALSE;
			sp_struct.mixInGain_mB = GAIN_SYSPARM;
			sp_struct.mixInGainR_mB = GAIN_SYSPARM;

			i = AUDCTRL_GetAudioApp();
			j = AUDIO_MODE_HEADSET;
			if (user_vol_setting[i][j].valid == FALSE)
				fillUserVolSetting(j, i);
			sp_struct.mixOutGain_mB = user_vol_setting[i][j].L;
			sp_struct.mixOutGainR_mB = user_vol_setting[i][j].R;

			if (sp_struct.app == AUDIO_APP_FM) {
				AUDCTRL_SetAudioMode_ForFM(
				AUDIO_MODE_HEADSET, pathID, FALSE);
			} else {
				AUDDRV_SetAudioMode_Speaker(sp_struct);
				setExternAudioGain(AUDIO_MODE_HEADSET,
					AUDCTRL_GetAudioApp());
			}

			AUDCTRL_SaveAudioMode(AUDIO_MODE_HEADSET);
		}
#endif
	}
	return;
}


/****************************************************************************
*
* Function Name: AUDCTRL_RemovePlaySpk_InPMU
*
* Description:   remove a speaker by indicating to PMU
*
****************************************************************************/
void AUDCTRL_RemovePlaySpk_InPMU(AUDIO_SOURCE_Enum_t source,
			   AUDIO_SINK_Enum_t sink, unsigned int pathID)
{

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

		if (sink == AUDIO_SINK_LOUDSPK) {

			aTrace(LOG_AUDIO_CNTLR, "Remove IHF - Stop MC\n");
			multicastToSpkr(FALSE);

		} else if (sink == AUDIO_SINK_HEADSET) {

			aTrace(LOG_AUDIO_CNTLR, "Remove HS - Stop MC"
				"and Switch\n");
			multicastToSpkr(FALSE);

			AUDCTRL_SwitchPlaySpk(source, AUDIO_SINK_LOUDSPK,
					      pathID);

		}


#ifdef CONFIG_ENABLE_SSMULTICAST
		/*If IHF removed reload HS params with mode HEADSET*/
		if (sink == AUDIO_SINK_LOUDSPK &&
			(currAudioMode_playback == AUDIO_MODE_SPEAKERPHONE ||
			currAudioMode_fm == AUDIO_MODE_SPEAKERPHONE)) {
			int i, j;
			sp_struct.mode = AUDIO_MODE_HEADSET;
			sp_struct.app = AUDCTRL_GetAudioApp();
			sp_struct.pathID = pathID;
			sp_struct.inHWlpbk = FALSE;
			sp_struct.mixInGain_mB = GAIN_SYSPARM;
			sp_struct.mixInGainR_mB = GAIN_SYSPARM;

			i = AUDCTRL_GetAudioApp();
			j = AUDIO_MODE_HEADSET;
			if (user_vol_setting[i][j].valid == FALSE)
				fillUserVolSetting(j, i);
			sp_struct.mixOutGain_mB = user_vol_setting[i][j].L;
			sp_struct.mixOutGainR_mB = user_vol_setting[i][j].R;

			if (sp_struct.app == AUDIO_APP_FM) {
				AUDCTRL_SetAudioMode_ForFM(
				AUDIO_MODE_HEADSET, pathID, FALSE);
			} else {
				AUDDRV_SetAudioMode_Speaker(sp_struct);
				setExternAudioGain(AUDIO_MODE_HEADSET,
					AUDCTRL_GetAudioApp());
			}

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
			AUDDRV_SetAudioMode_voicerecord(
				AUDCTRL_GetAudioMode(),
				AUDCTRL_GetAudioApp(), pathID, 0);
		}

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
	AudioApp_t app = AUDCTRL_GetAudioApp();
	AudioMode_t mode = AUDCTRL_GetAudioMode();
	aTrace(LOG_AUDIO_CNTLR,
			"%s src 0x%x, sink 0x%x,sr %d",
			__func__, source, sink, sr);

	/*in call mode, return the UL path*/
	if (bInVoiceCall && source != AUDIO_SOURCE_I2S) {
		if (sr != AUDIO_SAMPLING_RATE_48000) {
			/*in call mode, return the UL path*/
			*pPathID = AUDDRV_GetULPath();
			AUDDRV_EnableDSPInput(source, sr);
			return;
		} else {
			if (sr == AUDIO_SAMPLING_RATE_48000)
				;
			/*go ahead to take a new path*/
		}
	}

	if (isDigiMic(source)) {
		/* Enable power to digital microphone */
		powerOnDigitalMic(TRUE);
	}

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

	if (!AUDCTRL_InVoiceCall()) {
		/*for music tuning, if PCG changed audio mode,
		   need to pass audio mode to CP */
		RPC_SetProperty(RPC_PROP_AUDIO_MODE,
			(UInt32)(((int)app *
			AUDIO_MODE_NUMBER + mode)));
	}

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
		AUDDRV_DisableDSPInput(1);

	/*in call mode, return*/
	if (bInVoiceCall && source != AUDIO_SOURCE_I2S)
		return;

	if (pathID)
		path = csl_caph_FindPath(pathID);
	else
		return;

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
#if 0
		AUDCTRL_SaveAudioApp(AUDIO_APP_LOOPBACK);
#endif
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
		/*For HW Loopback use MUSIC app prof?*/
		AUDCTRL_SaveAudioMode(audio_mode);
		AUDCTRL_SaveAudioApp(AUDIO_APP_MUSIC);

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
#if 0
		AUDCTRL_RemoveAudioApp(AUDIO_APP_LOOPBACK);
#endif
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

		pathID = csl_caph_FindPathID(sink_dev, src_dev, 0);

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
*alsa_amixer cset name=HW-CTL,index=21 1 0 100 0
*    (set EP_MIX_IN_GAIN to 100 mB)
*alsa_amixer cset name=HW-CTL,index=21 1 1 2400 0
*    (set EP_MIX_BITSEL_GAIN to 24 dB)
*alsa_amixer cset name=HW-CTL,index=21 1 2 -600 0
*    (set EP_MIX_FINE_GAIN to -6 dB)
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
	case AUDCTRL_HW_CFG_WAIT:
		AUDCTRL_ConfigWait(arg1, arg2);
		break;
	case AUDCTRL_HW_CFG_DSPMUTE:
		AUDCTRL_SetTelephonySpkrMute(AUDIO_SINK_RESERVED4, arg1);
		break;
	case AUDCTRL_HW_PRINT_PATH:
		csl_caph_hwctrl_PrintAllPaths();
		if (arg1 == 1) {
			Dump_AllCaph_regs();
			Dsp_Shared_memDump();
		}
		break;
#if defined(CONFIG_MFD_BCM59039) | defined(CONFIG_MFD_BCM59042)
	case AUDCTRL_HW_TEMPGAINCOMP:
		aWarn("Temp Gain Compensation Status !!!!%d\n", arg1);
		gtempGainCompEnable = arg1;
		break;
#endif

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
	int ret;

	if (powerOn == TRUE) {
		/* Enable power to digital microphone */
		if (!digi_mic_reg) {
			digi_mic_reg =
			regulator_get(NULL, DIGI_MIC_LDO_REGULATOR);

			if (IS_ERR(digi_mic_reg))
				aError("Failed to get LDO for Digital Mic\n");
		}
		if (digi_mic_reg) {
			ret = regulator_enable(digi_mic_reg);
			if (ret != 0)
				aError("Failed to enable LDO digi Mic: %d\n",
				ret);
		}

	} else {	/*powerOn == FALSE */
		/* Enable power to digital microphone */
		if (digi_mic_reg) {
			ret = regulator_disable(digi_mic_reg);
			if (ret != 0)
				aError("Failed to disable LDO digi Mic: %d\n",
				ret);
			regulator_put(digi_mic_reg);
			digi_mic_reg = NULL;
		}

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
		aWarn(
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
	static int audio2UseHS = FALSE;
	static int fmUseHS = FALSE;

	static Boolean callUseIHF = FALSE;
	static Boolean audioUseIHF = FALSE;
	static int audio2UseIHF = FALSE;
	static int fmUseIHF = FALSE;

	static Boolean IHF_IsOn = FALSE;
	static Boolean HS_IsOn = FALSE;
	static Boolean ampControl = TRUE;

	aTrace(LOG_AUDIO_CNTLR, "%s speaker %d, usage_flag %d, use %d,"
			" force %d\n",
			__func__, speaker, usage_flag, use, force);

	if (force == TRUE) {
		if (use == FALSE) {
			if (IHF_IsOn == TRUE) {
				if (isStIHF == TRUE)
					extern_stereo_speaker_off();
				else
					extern_ihf_off();
			}
			if (HS_IsOn == TRUE)
				extern_hs_off();
			ampControl = FALSE;
		}
		if (use == TRUE) {
			if (IHF_IsOn == TRUE) {
				if (isStIHF == TRUE)
					extern_stereo_speaker_on();
				else
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

		return;

	}


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
			case Audio2Use:
				audio2UseHS = use;
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
			case Audio2Use:
				audio2UseIHF = use;
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
			" callUseIHF %d,audio2UseIHF %d"
			" fmUseHS %d, audioUseHS %d,"
			"audio2UseHS %d callUseHS %d\n",
			fmUseIHF, audioUseIHF, callUseIHF, audio2UseIHF,
			fmUseHS, audioUseHS, audio2UseHS, callUseHS);

	if ((callUseHS == FALSE) && (audioUseHS == FALSE)
		&& (fmUseHS == FALSE) && (audio2UseHS == FALSE)) {
		if (HS_IsOn != FALSE) {
			aTrace(LOG_AUDIO_CNTLR,
				"power OFF pmu HS amp\n");

			extern_hs_off();
/*			audctl_usleep_range(20000,
				22000); no pop noise */
			audctl_usleep_range(wait_pmu_off,
				wait_pmu_off+2000);
		}
		HS_IsOn = FALSE;
	} else {
		if (HS_IsOn != TRUE && ampControl == TRUE) {
			aTrace(LOG_AUDIO_CNTLR,
				"powerOnExternalAmp power on HS");
			audioh_start_hs();
			extern_hs_on();
			audctl_usleep_range(wait_hspmu_on,
				wait_hspmu_on+2000);
		}

		setExternAudioGain(GetAudioModeBySink(speaker),
			AUDCTRL_GetAudioApp());

		HS_IsOn = TRUE;
	}

	if ((callUseIHF == FALSE) && (audioUseIHF == FALSE)
		&& (fmUseIHF == FALSE) && (audio2UseIHF == FALSE)) {
		if (IHF_IsOn != FALSE) {
			aTrace(LOG_AUDIO_CNTLR,
				"power OFF pmu IHF amp\n");
			if (isStIHF == TRUE)
				extern_stereo_speaker_off();
			else
				extern_ihf_off();
			audctl_usleep_range(wait_pmu_off,
				wait_pmu_off+2000);
		}
		IHF_IsOn = FALSE;
	} else {
		if (IHF_IsOn != TRUE && ampControl == TRUE) {
			aTrace(LOG_AUDIO_CNTLR,
				"powerOnExternalAmp power on IHF");
			audioh_start_ihf();
			if (isStIHF == TRUE)
				extern_stereo_speaker_on();
			else
				extern_ihf_on();
			audctl_usleep_range(wait_ihfpmu_on,
				wait_ihfpmu_on+2000);
		}

		setExternAudioGain(GetAudioModeBySink(speaker),
		AUDCTRL_GetAudioApp());

		IHF_IsOn = TRUE;
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
	/*#define can be removed after the sysparam is updated
	with these new params*/

#ifdef NEW_PARAMS_TP_PMU
	Int16 param_value;
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

		/*#define can be removed after the sysparam is updated
		   with these new params*/

#ifdef NEW_PARAMS_TP_PMU
		param_value = p->ext_speaker_preamp_pga;

		extern_set_hs_preamp_gain(param_value);

		param_value = p->ext_speaker_param1;

		extern_set_hs_noise_gate(param_value);
#endif
		break;

	case AUDIO_MODE_SPEAKERPHONE:
		pmu_gain = (short)p->ext_speaker_pga_l;	/* Q13p2 dB */

		extern_ihf_set_gain(pmu_gain * 25);
		/*#define can be removed after the sysparam is updated
		with these new params*/

#ifdef NEW_PARAMS_TP_PMU
		param_value = p->ext_speaker_preamp_pga;

		extern_set_ihf_preamp_gain(param_value);

		param_value = p->ext_speaker_param1;

		extern_set_ihf_noise_gate(param_value);

		param_value = p->ext_speaker_param2;

		extern_set_ihf_none_clip(param_value);

		param_value = p->ext_speaker_param3;

		extern_set_ihf_pwr(param_value);
#endif

#ifdef CONFIG_BCM_MODEM
		pmu_gain = (int)p->ext_speaker_high_gain_mode_enable;
		extern_ihf_en_hi_gain_mode(pmu_gain);
#endif

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

void ReloadUserVolSettingFromSysparm(void)
{
	int i, j;

	for (i = 0; i <= AUDIO_MODE_RESERVE; i++)
		for (j = 0; j <= AUDIO_APP_RESERVED15; j++)
			fillUserVolSetting(i, j);
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

/********************************************************************
*  @brief  Configure wait duration
*
*  @param  wait_id	wait id
*
*  @param  wait_length	wait duration in ms
*
*  @return none
*
****************************************************************************/
void AUDCTRL_ConfigWait(int wait_id, int wait_length)
{
	int *p_wait = NULL, old_wait = 0, new_wait = wait_length*1000;

	switch (wait_id) {
	case AUDCTRL_WAIT_BASEBAND_ON:
		p_wait = &wait_bb_on;
		break;
	case AUDCTRL_WAIT_HSPMU_ON:
		p_wait = &wait_hspmu_on;
		break;
	case AUDCTRL_WAIT_IHFPMU_ON:
		p_wait = &wait_ihfpmu_on;
		break;
	case AUDCTRL_WAIT_PMU_OFF:
		p_wait = &wait_pmu_off;
		break;
	default:
		break;
	}

	if (p_wait) {
		old_wait = *p_wait;
		*p_wait = new_wait;
	}
	aTrace(LOG_AUDIO_CNTLR, "%s wait id %d length %d to %d us\n",
		__func__, wait_id, old_wait, new_wait);
}

/****************************************************************************
*
* Function Name: AUDCTRL_EnableHS_SuplyCtrl
*
* Description:   Headset Driver Supply Indicator Control for
*				class G output
*
****************************************************************************/
void AUDCTRL_EnableHS_SuplyCtrl(UInt8 hs_ds_lag,
						UInt8 hs_ds_delay,
						Boolean hs_ds_polarity,
						UInt32 hs_ds_thres)
{
	struct classg_G_ctrl classG;

	classG.HS_DS_LAG = hs_ds_lag;
	classG.HS_DS_DELAY = hs_ds_delay;
	classG.HS_DS_POLARITY = hs_ds_polarity;
	classG.HS_DS_THRES = hs_ds_thres;

	csl_caph_classG_ctrl(&classG);

}

void AUDCTRL_SetCallMode(Int32 callMode)
{
	AUDDRV_SetCallMode(callMode);
}

void AUDCTRL_ConnectDL(void)
{
	audio_control_dsp(AUDDRV_DSPCMD_AUDIO_CONNECT_DL, TRUE, 0, 0, 0, 0);
	aTrace(LOG_AUDIO_CNTLR,  "AUDCTRL_ConnectDL PTT CONNECT_DL\n");
}

#if defined(CONFIG_MFD_BCM59039) | defined(CONFIG_MFD_BCM59042)
Boolean AUDCTRL_AllPathsDisabled(void)
{
	return csl_caph_hwctrl_allPathsDisabled();
}
#endif
void AUDCTRL_UpdateUserVolSetting(
	AUDIO_SINK_Enum_t sink,
	int vol_left,
	int vol_right,
	AudioApp_t app)
{
	AudioMode_t mode;

	mode = GetAudioModeBySink(sink);
	user_vol_setting[app][mode].L = vol_left;
	user_vol_setting[app][mode].R = vol_right;
	user_vol_setting[app][mode].valid = TRUE;
	aTrace(LOG_AUDIO_CNTLR, "%s app = %d, mode = %d\n",
		__func__, app, mode);
}

/********************************************************************
*  @brief  Set PMU Paramters
*
*  @param  param_id	param_value	channel
*
*  @param  param_id - Indicates what param has to be set
	   param_value - The value to be set
	   channel - left/right on which the param has to be set
*
*  @return none
*
****************************************************************************/

void setExternalParameter(Int16 param_id, Int16 param_value, int channel)
{
	AudioMode_t mode;
	mode = AUDCTRL_GetAudioMode();

	aTrace(LOG_AUDIO_CNTLR, "%s param_id =%d,param_value =%d\n",
		__func__, param_id, param_value);

	switch (param_id) {
	case 0:
		{
			aTrace(LOG_AUDIO_CNTLR, "IHF/HS:Gain  =%d ,"
				"mode =%d\n", param_value, mode);
			if (mode == AUDIO_MODE_HANDSET ||
					mode == AUDIO_MODE_SPEAKERPHONE)
				extern_ihf_set_gain(param_value);
			else
				if (channel ==
					PARAM_PMU_SPEAKER_PGA_LEFT_CHANNEL)
					extern_hs_set_gain(param_value,
								AUDIO_HS_LEFT);
				else if (channel ==
					PARAM_PMU_SPEAKER_PGA_LEFT_CHANNEL)
					extern_hs_set_gain(param_value,
								AUDIO_HS_RIGHT);

		}
		break;
	case 1:
		{
			aTrace(LOG_AUDIO_CNTLR, "IHF/HS:PreAmpGain  =%d,"
				"mode =%d\n", param_value, mode);
			if (mode == AUDIO_MODE_HANDSET ||
					mode == AUDIO_MODE_SPEAKERPHONE)
				extern_set_ihf_preamp_gain(param_value);
			else
				extern_set_hs_preamp_gain(param_value);
		}
		break;
	case 2:
		{
			aTrace(LOG_AUDIO_CNTLR, "IHF/HS:NoiseGate  =%d,"
				"mode =%d\n", param_value, mode);
			if (mode == AUDIO_MODE_HANDSET ||
				mode == AUDIO_MODE_SPEAKERPHONE)
				extern_set_ihf_noise_gate(param_value);
			else
				extern_set_hs_noise_gate(param_value);
		}
		break;
	case 3:
		{
			aTrace(LOG_AUDIO_CNTLR, "IHF:NoneClip  =%d\n",
						param_value);
			extern_set_ihf_none_clip(param_value);
		}
		break;
	case 4:
		{
			aTrace(LOG_AUDIO_CNTLR, "IHF:PWR  =%d\n",
						param_value);
			extern_set_ihf_pwr(param_value);
		}
		break;
	default:
		break;
	}

	return;
}

/****************************************************************************
*
* Function Name: multicastToSpkr
*
* Description:  Indicate to the PMU to add or remove IHF
*
****************************************************************************/

void multicastToSpkr(Boolean flag)
{

	aTrace(LOG_AUDIO_CNTLR, "%s flag =%d\n",
		__func__, flag);
	if (flag == TRUE)
		extern_start_stop_multicast(TRUE);
	else
		extern_start_stop_multicast(FALSE);


}
#if defined(CONFIG_MFD_BCM59039) | defined(CONFIG_MFD_BCM59042)

int AUDCTRL_TempGainComp(BrcmPmuTempGainComp *paudio)
{
	int gainL_mB = 0, gainR_mB = 0;
	int delay =  TEMP_CHECK_DELAY_2MIN;
	int deltaT = 0;
	unsigned short int hs_ds_indicator = 0;
	unsigned int hs_ds_thres;
	int pmu_gainR,  pmu_gainL;
	int hs_ds_ind_count = 0;
	int loop;
	int temp;
	if (csl_caph_IsHSActive()) {
		if (!csl_caph_QueryHWClock()) {
			aWarn("Clocks are not enabled\n");
			return delay;
		}
		csl_caph_ClockLock();
	    if (paudio->firstFlag == 0) {
			paudio->firstFlag = 1;
		/* store the gain */
			csl_srcmixer_getMixOutGain(CSL_CAPH_SRCM_STEREO_CH1,
					&gainL_mB, &gainR_mB);
			paudio->hs_orig_gain_L = gainL_mB;
			paudio->hs_orig_gain_R = gainR_mB;
			paudio->hs_curr_gain_L = gainL_mB;
			paudio->hs_curr_gain_R = gainR_mB;
			aTrace(LOG_AUDIO_PMUTEMP, "AUDCTRL_TempGainComp:"
				" original mixer"
				"out gain %d, %d\n", gainL_mB, gainR_mB);
	    }
	    temp = paudio->currTemp;
		aTrace(LOG_AUDIO_PMUTEMP, "inside AUDCTRL_TempGainComp"
			"temp %d\n", temp);
	    if (temp < TEMP_HIGH) {
				if (paudio->tempGainState == AUDCTRL_AMP_OFF) {
					/* Turn on AMP */
					extern_hs_on();
					setExternAudioGain(
						GetAudioModeBySink(
							AUDIO_SINK_HEADSET),
						AUDCTRL_GetAudioApp());
					aTrace(LOG_AUDIO_PMUTEMP,
						"Turn on HS AMP\n");
					gainL_mB = paudio->hs_orig_gain_L;
					gainR_mB = paudio->hs_orig_gain_R;
					paudio->hs_curr_gain_L = gainL_mB;
					paudio->hs_curr_gain_R = gainR_mB;
					aTrace(LOG_AUDIO_PMUTEMP, "Turn on "
						"HS AMP %d %d\n",
						paudio->hs_orig_gain_L,
						paudio->hs_orig_gain_R);
					csl_srcmixer_setMixOutGain(
						CSL_CAPH_SRCM_STEREO_CH1,
						gainL_mB,  gainR_mB);
				}
				if (paudio->tempGainState
					== AUDCTRL_HIGH_TEMP) {
					gainL_mB = paudio->hs_orig_gain_L;
					gainR_mB = paudio->hs_orig_gain_R;
					paudio->hs_curr_gain_L = gainL_mB;
					paudio->hs_curr_gain_R = gainR_mB;
					aTrace(LOG_AUDIO_PMUTEMP, "Restoring "
					"the Original BB"
						"gain %d\n", gainR_mB);
					csl_srcmixer_setMixOutGain(
						CSL_CAPH_SRCM_STEREO_CH1,
						gainL_mB,  gainR_mB);
				}
			paudio->tempGainState = AUDCTRL_NORMAL_TEMP;
			delay = TEMP_CHECK_DELAY_2MIN;
			aTrace(LOG_AUDIO_PMUTEMP, "Headset present\n");
		} else if (temp >= TEMP_HIGH
			&& temp < TEMP_VERY_HIGH) {
			if (paudio->tempGainState == AUDCTRL_AMP_OFF) {
					/* Turn on AMP */
					extern_hs_on();
						setExternAudioGain(
							GetAudioModeBySink(
							AUDIO_SINK_HEADSET),
							AUDCTRL_GetAudioApp());
				aTrace(LOG_AUDIO_PMUTEMP, "Turn on HS AMP in"
					"TEMP_HIGH State\n");
		    }
			deltaT = temp - TEMP_HIGH;
			/* compute the threshold*/
			extern_hs_get_gain(&pmu_gainL, &pmu_gainR);
			hs_ds_thres = AUDCTRL_TempGainComp_ComputeThresh(
				deltaT, pmu_gainR);
			/*Write to BB register*/
			csl_caph_audioh_hs_supply_set_hs_ds_thres(hs_ds_thres);
			aTrace(LOG_AUDIO_PMUTEMP, "pmu_gainL %d pmu_gainR  %d"
				"set hs_ds_thres = 0x%x %d\n",
				pmu_gainL, pmu_gainR, hs_ds_thres, hs_ds_thres);
			/* delay of 2 msec */
			mdelay(2);
			/* Read the headset driver supply indicator
			50 times in 50ms*/
			hs_ds_ind_count = 0;
			for (loop = 0; loop < 50; loop++) {
				/* The clocks module is waiting to turnoff
				the clock. Break the loop and return.*/
				if (csl_caph_ClockWaitStatus()) {
					aError("Clock module waiting "
					"to trun os clock !!!!");
					break;
				}
				csl_caph_audioh_hs_supply_get_hs_ds_indicator(
					(unsigned short int*)&hs_ds_indicator);
				mdelay(1);
				if (hs_ds_indicator)
					hs_ds_ind_count++;
			}
			if (hs_ds_ind_count) {
				/*covert dB to mB*/
				gainR_mB =
					paudio->hs_orig_gain_R - (50 * deltaT);
				gainL_mB =
					paudio->hs_orig_gain_L - (50 * deltaT);
				paudio->hs_curr_gain_L =
					gainL_mB;
				paudio->hs_curr_gain_R =
					gainR_mB;

				aTrace(LOG_AUDIO_PMUTEMP, "The Final"
					" hs_ds_ind_count %d,"
					"New gainR_mB %d\n",
					hs_ds_ind_count, gainR_mB);
				csl_srcmixer_setMixOutGain(
					CSL_CAPH_SRCM_STEREO_CH1,
					gainR_mB,  gainR_mB);
			} else {
				/* if one or more 1 then set the
				baseband gain = original + 0.5 deltaT; */
				gainL_mB = paudio->hs_curr_gain_L;
				gainR_mB = paudio->hs_curr_gain_R;

				aTrace(LOG_AUDIO_PMUTEMP, "Old  gainR_mB %d\n",
					gainR_mB);
				/*covert dB to mB*/
				gainR_mB = gainR_mB + (50);
				gainL_mB = gainL_mB + (50);
				paudio->hs_curr_gain_L = gainL_mB;
				paudio->hs_curr_gain_R = gainR_mB;
				csl_srcmixer_setMixOutGain(
					CSL_CAPH_SRCM_STEREO_CH1,
					gainR_mB,  gainR_mB);
				aTrace(LOG_AUDIO_PMUTEMP, "Increase"
				" gain by 0.5dB: New  gainR_mB %d"
				"paudio->hs_orig_gain_R %d\n",
				gainR_mB, paudio->hs_orig_gain_R);
				if (gainR_mB >= paudio->hs_orig_gain_R)  {
					paudio->hs_curr_gain_L =
						paudio->hs_orig_gain_L;
					paudio->hs_curr_gain_R =
						paudio->hs_orig_gain_R;
					csl_srcmixer_setMixOutGain(
						CSL_CAPH_SRCM_STEREO_CH1,
						paudio->hs_orig_gain_L,
						paudio->hs_orig_gain_R);
					aTrace(LOG_AUDIO_PMUTEMP, "Setting  "
					"gain original value  %d\n",
					paudio->hs_orig_gain_R);
				}
			}
			paudio->tempGainState = AUDCTRL_HIGH_TEMP;
			/* start the timer for 30sec.*/
			delay = TEMP_CHECK_DELAY_30SEC;
		} else if (temp >= TEMP_VERY_HIGH) {
			/* store amp turn off status.*/
			paudio->tempGainState = AUDCTRL_AMP_OFF;
			/* Turn off the amp*/
			extern_hs_off();
			aTrace(LOG_AUDIO_PMUTEMP, "Temp = %d. !!!More than 125."
			" Shutdown HS AMP\n", temp);
		}
	} else
	aTrace(LOG_AUDIO_PMUTEMP, "***Headset is not active\n");
	csl_caph_ClockUnLock();
	return delay;
}

unsigned int AUDCTRL_TempGainComp_ComputeThresh(int deltaT, int pmu_gain)
{
/* Threshold calculation
Threshold (decimal) =
floor[(PMU_OUTPUT_LIMIT/(0.5*4.8*10exp(PMU_GAIN/20)))*2to the power 23]
*/
	unsigned int thresh = 0;
	float PMU_output_limit;
	float PMU_thresh_partial_val;
	int index;
	float tempThresh;
	aTrace(LOG_AUDIO_PMUTEMP, "AUDCTRL_TempGainComp_ComputeThresh"
	" deltaT=%d, pmu_gain=%d\n", deltaT, pmu_gain);
	for (index = 0; index < PMU_GAIN_RANGE; index++) {
		if (pmu_gain ==
			hsPMU_GAINPartialcalulationTable[index].pmu_gain) {
			PMU_thresh_partial_val =
			hsPMU_GAINPartialcalulationTable[index].
			PMU_thresh_partial_val;
			PMU_output_limit =
			hsPMU_DeltaTOutputTable[deltaT].PMU_output_limit;
			tempThresh =
				(PMU_output_limit/PMU_thresh_partial_val)
				*8388608;
			thresh = (unsigned int)(tempThresh - 0.5);
			break;
		}
	}
	return thresh;
}

void AUDCTRL_TempGainCompInit(BrcmPmuTempGainComp *paudio)
{
	if (paudio->intiComplete == 0) {
		paudio->firstFlag = 0;
		paudio->tempGainState = AUDCTRL_NORMAL_TEMP;
		paudio->intiComplete = 1;
		paudio->prevTemp = 0;
		paudio->currTemp = 0;
	}
}

void AUDCTRL_TempGainCompDeInit(BrcmPmuTempGainComp *paudio)
{
	paudio->intiComplete = 0;
}

Boolean AUDCTRL_TempGainCompStatus(void)
{
	return gtempGainCompEnable;
}
#endif
