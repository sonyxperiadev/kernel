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
* @file   audio_vdriver.c
* @brief  Audio VDriver API for CAPH
*
******************************************************************************/

/*=============================================================================
// Include directives
//=============================================================================
*/
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/completion.h>
#include <linux/jiffies.h>
#include "mobcom_types.h"
#include "audio_consts.h"

#include "csl_apcmd.h"

#include "bcm_fuse_sysparm_CIB.h"

#include "csl_caph.h"
#include "csl_dsp.h"
#include "csl_caph_audioh.h"
#include "resultcode.h"
#include "csl_caph_hwctrl.h"
#include "audio_vdriver.h"
#include "audio_rpc.h"
#include <mach/comms/platform_mconfig.h>
#include "io.h"
#include "csl_dsp_cneon_api.h"
#if defined(ENABLE_DMA_VOICE)
#include "csl_dsp_caph_control_api.h"
#endif

#include "audio_controller.h"
#include "audio_ddriver.h"
#include "audio_trace.h"

#include "taskmsgs.h"
#include "ipcproperties.h"

#include "rpc_global.h"
#include "rpc_ipc.h"
#include "xdr_porting_layer.h"
#include "xdr.h"
#include "rpc_api.h"

/**
*
* @addtogroup AudioDriverGroup
* @{
*/

/*=============================================================================
// Private Type and Constant declarations
//=============================================================================
*/

typedef void (*AUDDRV_User_CB) (UInt32 param1, UInt32 param2, UInt32 param3);

struct _AUDDRV_PathID_t {
	CSL_CAPH_PathID ulPathID;
	CSL_CAPH_PathID ul2PathID;
	CSL_CAPH_PathID dlPathID;
};
#define AUDDRV_PathID_t struct _AUDDRV_PathID_t

/*=============================================================================
// Private Variable declarations
//=============================================================================
*/
#if defined(CONFIG_BCM_MODEM)
#define AUDIO_MODEM(a) a
#else
#define AUDIO_MODEM(a)
#endif

/*this is needed because DSPCMD_AUDIO_ENABLE sets/clears AMCR.
AUDEN for both voiceIn and voiceOut */
static Boolean voicePlayOutpathEnabled = FALSE;

static AUDDRV_PathID_t telephonyPathID;
#if defined(CONFIG_RHEA_PANDA)
CSL_CAPH_PathID dlExtModemPathID, ulExtModemPathID;
#endif

static void *sUserCB;
static CSL_CAPH_DEVICE_e sink = CSL_CAPH_DEV_NONE;
static Boolean userEQOn = FALSE;
/* If TRUE, bypass EQ filter setting request from audio controller.*/

/* used in pcm i/f control. assume one mic, one spkr. */
static AUDIO_SOURCE_Enum_t currVoiceMic = AUDIO_SOURCE_UNDEFINED;
static AUDIO_SINK_Enum_t currVoiceSpkr = AUDIO_SINK_UNDEFINED;


static Boolean dspECEnable = TRUE;
static Boolean dspNSEnable = TRUE;
static Boolean controlFlagForCustomGain = FALSE;
static Boolean inCallRateChange = FALSE;
static int audio_tuning_flag;
static Boolean voiceRecOn = FALSE;
static AUDIO_SOURCE_Enum_t voiceInMic;
static AUDIO_SAMPLING_RATE_t voiceInSr;

struct _Audio_Driver_t {
	UInt8 isRunning;
	UInt32 taskID;
};
#define Audio_Driver_t struct _Audio_Driver_t

static Audio_Driver_t sAudDrv = { 0 };

static audio_codecId_handler_t codecId_handler;
static audio_handleCPReset_handler_t cpReset_handler;
static Int32 curCallMode = CALL_MODE_NONE;

struct completion audioEnableDone;

#if defined(CONFIG_RHEA_PANDA)
#define AUDIO_ENABLE_RESP_TIMEOUT 50  /* 50ms */
#define	timeout_jiff msecs_to_jiffies(AUDIO_ENABLE_RESP_TIMEOUT)

struct completion extModemCallDone;
#endif

/*=============================================================================
// Private function prototypes
//=============================================================================
*/
#ifndef CONFIG_BCM_MODEM
/* this array initializes the system parameters with the values
 from param_audio_rhea.txt for different modes(mic_pga - hw_sidetone_gain)*/
static AudioSysParm_t audio_parm_table[AUDIO_MODE_NUMBER_VOICE] = {
	{72, 36, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 96, 0, 0, 96, 0, 0, 0, 1,
	 0, {0} },
	{72, 36, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 96, 0, 0, 96, 0, 65520,
	 65520, 1, 0, {0} },
	{72, 36, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 96, 0, 0, 96, 0, 0, 0, 0,
	 0, {0} },
	{72, 36, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 96, 0, 0, 96, 0, 0, 0, 0,
	 0, {0} },
	{72, 36, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 96, 0, 0, 96, 0, 16, 16, 0,
	 0, {0} },
	{72, 36, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 96, 0, 0, 96, 0, 65520,
	 65520, 0, 0, {0} },
	{72, 36, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 96, 0, 0, 96, 0, 0, 0, 0,
	 0, {0} },
	{72, 36, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 96, 0, 0, 96, 0, 0, 0, 0,
	 0, {0} },
	{72, 36, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 96, 0, 0, 96, 0, 0, 0, 0,
	 0, {0} },
	{72, 36, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 96, 0, 0, 96, 0, 16, 16, 0,
	 1, {0} },
	{72, 36, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 96, 0, 0, 96, 0, 65520,
	 65520, 1, 0, {0} },
	{72, 36, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 96, 0, 0, 96, 0, 0, 0, 0,
	 0, {0} },
	{72, 36, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 96, 0, 0, 96, 0, 0, 0, 0,
	 0, {0} },
	{72, 36, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 96, 0, 0, 96, 0, 8, 8, 0,
	 0, {0} },
	{72, 36, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 96, 0, 0, 96, 0, 65520,
	 65520, 0, 0, {0} },
	{72, 36, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 96, 0, 0, 96, 0, 0, 0, 0,
	 0, {0} },
	{72, 36, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 96, 0, 0, 96, 0, 0, 0, 0,
	 0, {0} },
	{72, 36, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 96, 0, 0, 96, 0, 0, 0, 0,
	 0, {0} }
};
#endif

#ifdef CONFIG_BCM_MODEM
SysAudioParm_t *AudParmP(void)
#else
AudioSysParm_t *AudParmP(void)
#endif
{
#if defined(BSP_ONLY_BUILD)
	return NULL;
#elif defined(CONFIG_BCM_MODEM)
	return APSYSPARM_GetAudioParmAccessPtr();
#else
	return &audio_parm_table[0];
#endif
}

static UInt32 *AUDIO_GetIHF48KHzBufferBaseAddress(void);

#if defined(CONFIG_RHEA_PANDA)
static void AUDDRV_ExtModem_Telephony_Init(AUDIO_SOURCE_Enum_t mic,
		AUDIO_SINK_Enum_t speaker,
		AudioMode_t mode, AudioApp_t app,
		int bDualMic_IsNeeded,
		int bmuteVoiceCall);
static void AUDDRV_ExtModem_Telephony_Deinit(void);
static void AP_ProcessExtModemCallDone(UInt16 enabled_path);
#endif
static void AUDDRV_Telephony_InitHW(AUDIO_SOURCE_Enum_t mic,
				    AUDIO_SINK_Enum_t speaker,
				    AUDIO_SAMPLING_RATE_t sample_rate,
				    Boolean bNeedDualMic);

static void AUDDRV_Telephony_DeinitHW(void);
static CSL_CAPH_DEVICE_e AUDDRV_GetDRVDeviceFromSpkr(AUDIO_SINK_Enum_t spkr);
static CSL_CAPH_DEVICE_e AUDDRV_GetDRVDeviceFromMic(AUDIO_SOURCE_Enum_t mic);

static void AUDDRV_HW_SetFilter(AUDDRV_HWCTRL_FILTER_e filter,
				       void *coeff);
static void AUDDRV_HW_EnableSideTone(AudioMode_t audio_mode);
static void AUDDRV_HW_DisableSideTone(AudioMode_t audio_mode);
static void AP_ProcessAudioEnableDone(UInt16 enabled_path);

/*=============================================================================
// Functions
//=============================================================================
*/

/*=============================================================================
//
// Function Name: AUDDRV_Init
//
// Description:   Inititialize audio system
//
//=============================================================================
*/
void AUDDRV_Init(void)
{
	aTrace(LOG_AUDIO_DRIVER,  "AUDDRV_Init\n");

	if (sAudDrv.isRunning == TRUE)
		return;

	/* register DSP VPU status processing handlers */

	AUDIO_MODEM(CSL_RegisterVPUCaptureStatusHandler
	    ((VPUCaptureStatusCB_t) &VPU_Capture_Request);)
#if 0		/*These features are not needed in LMP now. */
	AUDIO_MODEM(CSL_RegisterVPURenderStatusHandler
		((VPURenderStatusCB_t) &VPU_Render_Request);)
	AUDIO_MODEM(CSL_RegisterUSBStatusHandler
		((USBStatusCB_t) &AUDDRV_USB_HandleDSPInt);)
#endif
	AUDIO_MODEM(CSL_RegisterVoIPStatusHandler
		((VoIPStatusCB_t) &VOIP_ProcessVOIPDLDone);)
	AUDIO_MODEM(CSL_RegisterMainAMRStatusHandler
		((MainAMRStatusCB_t) &AP_ProcessStatusMainAMRDone);)
	AUDIO_MODEM(CSL_RegisterARM2SPRenderStatusHandler
		((ARM2SPRenderStatusCB_t) &ARM2SP_Render_Request);)
	AUDIO_MODEM(CSL_RegisterARM2SP2RenderStatusHandler
		((ARM2SP2RenderStatusCB_t) &ARM2SP2_Render_Request);)
	AUDIO_MODEM(CSL_RegisterAudioLogHandler
		((AudioLogStatusCB_t) &AUDLOG_ProcessLogChannel);)
	AUDIO_MODEM(CSL_RegisterVOIFStatusHandler
		((VOIFStatusCB_t) &VOIF_Buffer_Request);)
	CSL_RegisterAudioEnableDoneHandler
		((AudioEnableDoneStatusCB_t)
		&AP_ProcessAudioEnableDone);

#if defined(CONFIG_RHEA_PANDA)
	CSL_RegisterExtModemCallDoneHandler
		((AudioEnableDoneStatusCB_t)
		&AP_ProcessExtModemCallDone);

	init_completion(&extModemCallDone);
#endif

	Audio_InitRpc();
	sAudDrv.isRunning = TRUE;
	init_completion(&audioEnableDone);
}

/**********************************************************************
//
//       Shutdown audio driver task
//
//       @return        void
//       @note
**********************************************************************/
void AUDDRV_Shutdown(void)
{
	if (sAudDrv.isRunning == FALSE)
		return;

	sAudDrv.isRunning = FALSE;
}

/*=============================================================================
//
// Function Name: AUDDRV_Telephony_Init
//
// Description:   Initialize audio system for voice call
//
//=============================================================================
*/
/*Prepare DSP before turn on hardware audio path for voice call.
//      This is part of the control sequence for starting telephony audio.*/
void AUDDRV_Telephony_Init(AUDIO_SOURCE_Enum_t mic, AUDIO_SINK_Enum_t speaker,
		AudioMode_t mode, AudioApp_t app, int bNeedDualMic,
		int bmuteVoiceCall)
{
#if defined(ENABLE_DMA_VOICE)
	UInt16 dma_mic_spk;
#endif
	Boolean ec_enable_from_sysparm = dspECEnable;
	Boolean ns_enable_from_sysparm = dspNSEnable;

	audio_rpc_clear_flag_vc_rel_by_modem();

#if defined(CONFIG_RHEA_PANDA)
	AUDDRV_ExtModem_Telephony_Init(mic, speaker,
		mode, app, bNeedDualMic,
		bmuteVoiceCall);
#endif

	/*-/////////////////////////////////////////////////////////////////
	// Phone Setup Sequence
	// 1. Init CAPH HW
	// 2. Send DSP command AUDDRV_DSPCMD_AUDIO_ENABLE
	// 3. If requires 48KHz for example in IHF mode,
		Send VPRIPCMDQ_ENABLE_48KHZ_SPEAKER_OUTPUT
	////////////////////////////////////////////////////////////////-*/

	aTrace(LOG_AUDIO_DRIVER,
		"AUDDRV_Telephony_Init app %d, mode %d, bNeedDualMic %d\n",
		app, mode, bNeedDualMic);
	csl_caph_ControlHWClock(TRUE); /*enable clock before any DSP command*/

	currVoiceMic = mic;
	currVoiceSpkr = speaker;

	audio_control_dsp(AUDDRV_DSPCMD_MUTE_DSP_UL, 0, 0, 0, 0, 0);
	audio_control_dsp(AUDDRV_DSPCMD_EC_NS_ON, FALSE, FALSE, 0, 0, 0);
	audio_control_dsp(AUDDRV_DSPCMD_DUAL_MIC_ON, FALSE, 0, 0, 0, 0);
	audio_control_dsp(AUDDRV_DSPCMD_AUDIO_TURN_UL_COMPANDEROnOff, FALSE,
			  0, 0, 0, 0);
	audio_control_dsp(AUDDRV_DSPCMD_AUDIO_CONNECT_UL, FALSE, 0, 0, 0, 0);
	audio_control_dsp(AUDDRV_DSPCMD_AUDIO_CONNECT_DL, FALSE, 0, 0, 0, 0);

	if (app == AUDIO_APP_VOICE_CALL_WB) {
		AUDDRV_Telephony_InitHW(mic, speaker, AUDIO_SAMPLING_RATE_16000,
					bNeedDualMic);
#if defined(ENABLE_DMA_VOICE)
		csl_dsp_caph_control_aadmac_set_samp_rate
		    (AUDIO_SAMPLING_RATE_16000);
#endif
	} else {
		AUDDRV_Telephony_InitHW(mic, speaker, AUDIO_SAMPLING_RATE_8000,
					bNeedDualMic);
#if defined(ENABLE_DMA_VOICE)
		csl_dsp_caph_control_aadmac_set_samp_rate
		    (AUDIO_SAMPLING_RATE_8000);
#endif
	}

	/* Set new filter coef, sidetone filters, gains. */
	AUDDRV_SetAudioMode(mode, app,
		telephonyPathID.ulPathID,
		telephonyPathID.ul2PathID,
		telephonyPathID.dlPathID);

#if defined(ENABLE_DMA_VOICE)
	if (speaker == AUDIO_SINK_LOUDSPK) {
		csl_dsp_caph_control_aadmac_disable_path((UInt16)
			 (DSP_AADMAC_SPKR_EN) |
			 (UInt16) (DSP_AADMAC_PACKED_16BIT_IN_OUT_EN));
		/* dma_mic_spk = (UInt16) DSP_AADMAC_PRI_MIC_EN;*/
		dma_mic_spk = ((UInt16)(DSP_AADMAC_PRI_MIC_EN)) | ((UInt16)
			(DSP_AADMAC_IHF_SPKR_EN)) |
			((UInt16)DSP_AADMAC_RETIRE_DS_CMD);

		csl_dsp_caph_control_aadmac_enable_path(dma_mic_spk);
	} else {
		csl_dsp_caph_control_aadmac_disable_path((UInt16)
			 (DSP_AADMAC_IHF_SPKR_EN) |
			 (UInt16) (DSP_AADMAC_PACKED_16BIT_IN_OUT_EN));
		dma_mic_spk =
		    (UInt16) (DSP_AADMAC_PRI_MIC_EN) |
		    (UInt16) (DSP_AADMAC_SPKR_EN) |
		    (UInt16) (DSP_AADMAC_RETIRE_DS_CMD);
		if (bNeedDualMic)
			dma_mic_spk |= (UInt16) (DSP_AADMAC_SEC_MIC_EN);

		if (speaker == AUDIO_SINK_BTM)
			dma_mic_spk |= (UInt16)
			    (DSP_AADMAC_PACKED_16BIT_IN_OUT_EN);

		csl_dsp_caph_control_aadmac_enable_path(dma_mic_spk);
	}
#endif

	audio_control_dsp(AUDDRV_DSPCMD_AUDIO_ENABLE, TRUE, 0, 0, 0, 0);

	if (curCallMode != PTT_CALL)
		audio_control_dsp(
			AUDDRV_DSPCMD_AUDIO_CONNECT_DL, TRUE, 0, 0, 0, 0);
	else
		aTrace(LOG_AUDIO_DRIVER,  "AUDDRV_Telephony_Init: "
			"PTT, skip AUDIO_CONNECT_DL\n");

	audio_control_dsp(AUDDRV_DSPCMD_AUDIO_CONNECT_UL, TRUE, 0, 0, 0, 0);

/*	audio_control_dsp(AUDDRV_DSPCMD_EC_NS_ON, TRUE, TRUE, 0, 0, 0); */
	audio_control_dsp(AUDDRV_DSPCMD_EC_NS_ON, ec_enable_from_sysparm,
			  ns_enable_from_sysparm, 0, 0, 0);

	if (bNeedDualMic == TRUE)
		audio_control_dsp(AUDDRV_DSPCMD_DUAL_MIC_ON, TRUE, 0, 0, 0, 0);

	audio_control_dsp(AUDDRV_DSPCMD_AUDIO_TURN_UL_COMPANDEROnOff,
			  TRUE, 0, 0, 0, 0);

	if (bmuteVoiceCall == FALSE)
		audio_control_dsp(AUDDRV_DSPCMD_UNMUTE_DSP_UL, 0, 0, 0, 0, 0);

/* per call basis: enable the DTX by calling stack api when call connected */
	audio_control_generic(AUDDRV_CPCMD_ENABLE_DSP_DTX, TRUE, 0, 0, 0, 0);

	return;
}

#if defined(CONFIG_RHEA_PANDA)
static void AUDDRV_ExtModem_Telephony_Init(AUDIO_SOURCE_Enum_t mic,
		AUDIO_SINK_Enum_t speaker,
		AudioMode_t mode, AudioApp_t app, int bNeedDualMic,
		int bmuteVoiceCall)
{
	CSL_CAPH_HWCTRL_CONFIG_t config;
	unsigned long jiff_in = 0;

	aTrace(LOG_AUDIO_DRIVER,  "AUDDRV_ExtModem_Telephony_Init\n");
	csl_caph_ControlHWClock(TRUE); /*enable clock before any DSP command*/

	/* DL */
	config.streamID = CSL_CAPH_STREAM_NONE;
	config.pathID = 0;
	config.source = CSL_CAPH_DEV_SSP_RX;
	config.sink = CSL_CAPH_DEV_DSP_throughMEM;
	config.dmaCH = CSL_CAPH_DMA_CH15;
	config.src_sampleRate = AUDIO_SAMPLING_RATE_8000;
	config.snk_sampleRate = AUDIO_SAMPLING_RATE_8000;
	config.chnlNum = AUDIO_CHANNEL_MONO;
	config.bitPerSample = 16;

	dlExtModemPathID = csl_caph_hwctrl_extModem_SetupPath(config);

	/* UL */
	config.streamID = CSL_CAPH_STREAM_NONE;
	config.pathID = 0;
	config.source = CSL_CAPH_DEV_DSP_throughMEM;
	config.sink = CSL_CAPH_DEV_SSP_TX;
	config.dmaCH = CSL_CAPH_DMA_CH16;
	config.src_sampleRate = AUDIO_SAMPLING_RATE_8000;
	config.snk_sampleRate = AUDIO_SAMPLING_RATE_8000;
	config.chnlNum = AUDIO_CHANNEL_MONO;
	config.bitPerSample = 16;

	ulExtModemPathID = csl_caph_hwctrl_extModem_SetupPath(config);

	csl_caph_hwctrl_extModem_StartPath(ulExtModemPathID, dlExtModemPathID);

	csl_dsp_ext_modem_call(TRUE);

	jiff_in = wait_for_completion_interruptible_timeout(
		&extModemCallDone,
		timeout_jiff);
	if (!jiff_in) {
		aError("!!!Timeout on COMMAND_EXT_MODEM_CALL"
			" resp!!!\n");
		init_completion(&extModemCallDone);
	}
}
#endif

/*=============================================================================
//
// Function Name: AUDDRV_Telephony_RateChange
//
// Description:   Change the sample rate for voice call
//
//=============================================================================
*/
void AUDDRV_Telephony_RateChange(AudioMode_t mode,
	AudioApp_t audio_app, int bNeedDualMic, int bmuteVoiceCall)
{
	inCallRateChange = TRUE;
	AUDDRV_Telephony_Deinit();
	AUDDRV_Telephony_Init(currVoiceMic, currVoiceSpkr, mode, audio_app,
		bNeedDualMic, bmuteVoiceCall);
	inCallRateChange = FALSE;
	return;
}

/*********************************************************************
//
//       Registers callback for rate change request
//
//      @param     callback function
//      @return         void
//       @note
**********************************************************************/

void AUDDRV_RegisterRateChangeCallback(audio_codecId_handler_t codecId_cb)
{
	aTrace(LOG_AUDIO_DRIVER,  "AUDDRV_RegisterRateChangeCallback, 0x%lx\n",
			(long unsigned int)codecId_cb);
	codecId_handler = codecId_cb;
}

/*********************************************************************
//
//       Registers callback for handling cp reset
//
//      @param     callback function
//      @return         void
//       @note
**********************************************************************/

void AUDDRV_RegisterHandleCPResetCB(
	audio_handleCPReset_handler_t cpReset_cb)
{
	aTrace(LOG_AUDIO_DRIVER,  "AUDDRV_RegisterHandleCPResetCB");
	cpReset_handler = cpReset_cb;
}

/*********************************************************************
//
// Function Name: AUDDRV_EC
//
// Description:   DSP Echo cancellation ON/OFF
//
**********************************************************************/
void AUDDRV_EC(Boolean enable, UInt32 arg)
{
	dspECEnable = enable;
}

/*********************************************************************
//
// Function Name: AUDDRV_NS
//
// Description:   DSP Noise Suppression ON/OFF
//
**********************************************************************/
void AUDDRV_NS(Boolean enable)
{
	dspNSEnable = enable;
}

/*********************************************************************
//
//       Post rate change message for telephony session
//
//      @param          codec ID
//      @return         void
//       @note
**********************************************************************/
void AUDDRV_Telephone_RequestRateChange(int codecID)
{
	if (codecId_handler != NULL)
		codecId_handler(codecID);
}

/*********************************************************************
//
//       Post cp reset message
//
//      @param          none
//      @return         void
//       @note
**********************************************************************/
void AUDDRV_HandleCPReset(Boolean cp_reset_start)
{
	if (cpReset_handler != NULL)
		cpReset_handler(cp_reset_start);
}

/*=============================================================================
//
// Function Name: AUDDRV_Telephony_Deinit
//
// Description:   DeInitialize audio system for voice call
//
//=============================================================================
*/

/*Prepare DSP before turn off hardware audio path for voice call.
// This is part of the control sequence for ending telephony audio.*/
void AUDDRV_Telephony_Deinit(void)
{
#if defined(ENABLE_DMA_VOICE)
	UInt16 dma_mic_spk;
#endif

	aTrace(LOG_AUDIO_DRIVER, "%s voiceOut %d voiceRecOn %d\n",
		__func__, voicePlayOutpathEnabled, voiceRecOn);

/* continues speech playback when end the phone call.
// continues speech recording when end the phone call.*/
/*      if ( FALSE==vopath_enabled && FALSE==DspVoiceIfActive_DL()
	&& FALSE==vipath_enabled )
	{ */

	/* a quick fix not to disable voice path for speech playbck
	   when end the phone call. */
	/* disable the DTX by calling stack api when call disconnected */
	if (voicePlayOutpathEnabled == FALSE) {
		audio_control_generic(AUDDRV_CPCMD_ENABLE_DSP_DTX, FALSE, 0, 0,
				      0, 0);

		audio_control_dsp(AUDDRV_DSPCMD_AUDIO_CONNECT_DL, FALSE,
				  0, 0, 0, 0);
		audio_control_dsp(AUDDRV_DSPCMD_AUDIO_CONNECT_UL, FALSE,
				  0, 0, 0, 0);
		audio_control_dsp(AUDDRV_DSPCMD_EC_NS_ON, FALSE, FALSE,
				0, 0, 0);
		audio_control_dsp(AUDDRV_DSPCMD_DUAL_MIC_ON, FALSE, 0, 0, 0, 0);
		audio_control_dsp(AUDDRV_DSPCMD_AUDIO_TURN_UL_COMPANDEROnOff,
				  FALSE, 0, 0, 0, 0);

#if defined(ENABLE_DMA_VOICE)
		dma_mic_spk =
			(UInt16) (DSP_AADMAC_PRI_MIC_EN) |
			(UInt16) (DSP_AADMAC_IHF_SPKR_EN) |
			(UInt16) (DSP_AADMAC_SPKR_EN) |
			(UInt16) (DSP_AADMAC_SEC_MIC_EN) |
			(UInt16) (DSP_AADMAC_PACKED_16BIT_IN_OUT_EN);
		csl_dsp_caph_control_aadmac_disable_path(dma_mic_spk);
#endif
		audio_control_dsp(AUDDRV_DSPCMD_MUTE_DSP_UL, 0, 0, 0, 0, 0);
		audio_control_dsp(AUDDRV_DSPCMD_AUDIO_ENABLE, FALSE,
					0, 0, 0, 0);
		AUDDRV_Telephony_DeinitHW();
	}

	if (!inCallRateChange) {
		currVoiceMic = AUDIO_SOURCE_UNDEFINED;
		currVoiceSpkr = AUDIO_SINK_UNDEFINED;
	}

	if (voiceRecOn)
		AUDDRV_EnableDSPInput(voiceInMic, voiceInSr);


#if defined(CONFIG_RHEA_PANDA)
	AUDDRV_ExtModem_Telephony_Deinit();
#endif

	audio_rpc_clear_flag_vc_rel_by_modem();

	return;
}

#if defined(CONFIG_RHEA_PANDA)
/*=============================================================================
//
// Function Name: AUDDRV_ExtModem_Telephony_Deinit
//
// Description:   DeInitialize audio system for voice call
//
//=============================================================================
*/
static void AUDDRV_ExtModem_Telephony_Deinit(void)
{
	unsigned long jiff_in = 0;

	csl_dsp_ext_modem_call(FALSE);

	jiff_in = wait_for_completion_interruptible_timeout(
		&extModemCallDone,
		timeout_jiff);
	if (!jiff_in) {
		aError("!!!Timeout on COMMAND_EXT_MODEM_CALL"
			" resp!!!\n");
		init_completion(&extModemCallDone);
	}

	/*keep clock on until DSP response is back.*/
	csl_caph_hwctrl_extModem_StopPath(ulExtModemPathID, dlExtModemPathID);
}
#endif

/*=============================================================================
//
// Function Name: AUDDRV_EnableDSPOutput
//
// Description:   Enable audio DSP output for voice call
//
//=============================================================================
*/
void AUDDRV_EnableDSPOutput(AUDIO_SINK_Enum_t sink,
			    AUDIO_SAMPLING_RATE_t sample_rate)
{
	aTrace(LOG_AUDIO_DRIVER,  "%s mixer %d, sample_rate %u\n",
		__func__, sink, sample_rate);

	currVoiceSpkr = sink;

#if defined(ENABLE_DMA_VOICE)
	if (sample_rate == AUDIO_SAMPLING_RATE_8000) {
		csl_dsp_caph_control_aadmac_set_samp_rate
		    (AUDIO_SAMPLING_RATE_8000);
		if (sink == AUDIO_SINK_BTM)
			csl_dsp_caph_control_aadmac_enable_path((UInt16)
				(DSP_AADMAC_SPKR_EN) |
				(UInt16) (DSP_AADMAC_PACKED_16BIT_IN_OUT_EN) |
				(UInt16) (DSP_AADMAC_RETIRE_DS_CMD));
		else
			csl_dsp_caph_control_aadmac_enable_path((UInt16)
				(DSP_AADMAC_SPKR_EN) |
				(UInt16) (DSP_AADMAC_RETIRE_DS_CMD));
	} else {
		csl_dsp_caph_control_aadmac_set_samp_rate
		    (AUDIO_SAMPLING_RATE_16000);
		if (sink == AUDIO_SINK_BTM)
			csl_dsp_caph_control_aadmac_enable_path((UInt16)
				(DSP_AADMAC_SPKR_EN) |
				(UInt16) (DSP_AADMAC_PACKED_16BIT_IN_OUT_EN) |
				(UInt16) (DSP_AADMAC_RETIRE_DS_CMD));
		else
			csl_dsp_caph_control_aadmac_enable_path((UInt16)
				(DSP_AADMAC_SPKR_EN) |
				(UInt16) (DSP_AADMAC_RETIRE_DS_CMD));
	}
#endif

	audio_control_dsp(AUDDRV_DSPCMD_AUDIO_ENABLE, 1, 0, 0, 0, 0);
	audio_control_dsp(AUDDRV_DSPCMD_AUDIO_CONNECT_DL, 1, 0, 0, 0, 0);

	voicePlayOutpathEnabled = TRUE;

	aTrace(LOG_AUDIO_DRIVER,  "%s voicePlayOutpathEnabled=%d",
		__func__, voicePlayOutpathEnabled);
}

/* ============================================================================
//
// Function Name: AUDDRV_DisableDSPOutput
//
// Description:   Disable audio DSP output for playback
//
//=============================================================================
*/
void AUDDRV_DisableDSPOutput(void)
{
	aTrace(LOG_AUDIO_DRIVER,  "%s", __func__);

	audio_control_dsp(AUDDRV_DSPCMD_AUDIO_CONNECT_DL, FALSE, 0, 0, 0,
			  0);

#if defined(ENABLE_DMA_VOICE)
	csl_dsp_caph_control_aadmac_disable_path(((UInt16)
		DSP_AADMAC_SPKR_EN) | ((UInt16)(DSP_AADMAC_IHF_SPKR_EN)) |
		((UInt16)(DSP_AADMAC_PACKED_16BIT_IN_OUT_EN)));
#endif
	audio_control_dsp(AUDDRV_DSPCMD_AUDIO_ENABLE, FALSE, 0, 0, 0, 0);

	voicePlayOutpathEnabled = FALSE;
}

/*=============================================================================
//
// Function Name: AUDDRV_EnableDSPInput
//
// Description:   Enable audio DSP input for voice record
//
//=============================================================================
*/
void AUDDRV_EnableDSPInput(AUDIO_SOURCE_Enum_t source,
			   AUDIO_SAMPLING_RATE_t sample_rate)
{
	aTrace(LOG_AUDIO_DRIVER,  "%s source %d voiceRecOn %d, ulPath %d\n",
		__func__, source, voiceRecOn, telephonyPathID.ulPathID);

	voiceRecOn = TRUE;
	voiceInMic = source;
	voiceInSr = sample_rate;
	/*only in non-call mode, tell DSP to start UL*/
	if (telephonyPathID.ulPathID)
		return;

#if defined(ENABLE_DMA_VOICE)
	if (sample_rate == AUDIO_SAMPLING_RATE_8000) {
		csl_dsp_caph_control_aadmac_set_samp_rate
		    (AUDIO_SAMPLING_RATE_8000);
		if (source == AUDIO_SOURCE_BTM)
				csl_dsp_caph_control_aadmac_enable_path((UInt16)
					(DSP_AADMAC_PRI_MIC_EN) |
					(UInt16)
					(DSP_AADMAC_RETIRE_DS_CMD) |
					(UInt16)
					(DSP_AADMAC_PACKED_16BIT_IN_OUT_EN));
		else {
			if (currVoiceSpkr != AUDIO_SINK_BTM)	/* check spkr */
				csl_dsp_caph_control_aadmac_enable_path((UInt16)
					(DSP_AADMAC_PRI_MIC_EN) |
					(UInt16) (DSP_AADMAC_RETIRE_DS_CMD));
		}
	} else {
		csl_dsp_caph_control_aadmac_set_samp_rate
		    (AUDIO_SAMPLING_RATE_16000);
		if (source == AUDIO_SOURCE_BTM)
				csl_dsp_caph_control_aadmac_enable_path((UInt16)
					(DSP_AADMAC_PRI_MIC_EN) |
					(UInt16)
					(DSP_AADMAC_RETIRE_DS_CMD) |
					(UInt16)
					(DSP_AADMAC_PACKED_16BIT_IN_OUT_EN));
		else {
			if (currVoiceSpkr != AUDIO_SINK_BTM)	/* check spkr */
				csl_dsp_caph_control_aadmac_enable_path((UInt16)
					(DSP_AADMAC_PRI_MIC_EN) |
					(UInt16) (DSP_AADMAC_RETIRE_DS_CMD));
		}
	}
#endif

	audio_control_dsp(AUDDRV_DSPCMD_AUDIO_ENABLE, 1, 0, 0, 0, 0);
	audio_control_dsp(AUDDRV_DSPCMD_AUDIO_CONNECT_UL, 1, 0, 0, 0, 0);
	/* AUDDRV_DSPCMD_AUDIO_CONNECT should be called after
	AUDDRV_DSPCMD_AUDIO_ENABLE */

/*
When voice call ends, AUDDRV_DSPCMD_MUTE_DSP_UL is being sent to DSP and
this command mutes UL record gain
Not sure why this is done and there is no clarification if we really
need to send the MUTE UL command when disconnecting the voice call.
For now, when voice record is started, UMUTE UL command will be sent */

	audio_control_dsp(AUDDRV_DSPCMD_UNMUTE_DSP_UL, 0, 0, 0, 0, 0);

	currVoiceMic = source;
}

/*=============================================================================
//
// Function Name: AUDDRV_DisableDSPInput
//
// Description:   Disable audio DSP input for record
//
//=============================================================================
*/
void AUDDRV_DisableDSPInput(int stop)
{
	aTrace(LOG_AUDIO_DRIVER,  "%s stop %d, voiceRecOn %d, ulPath %d\n",
		__func__, stop, voiceRecOn, telephonyPathID.ulPathID);

	if (stop)
		voiceRecOn = FALSE;
	/*only in non-call mode, tell DSP to stop UL*/
	if (telephonyPathID.ulPathID)
		return;

	audio_control_dsp(AUDDRV_DSPCMD_AUDIO_CONNECT_UL, FALSE, 0, 0, 0, 0);
#if defined(ENABLE_DMA_VOICE)
	csl_dsp_caph_control_aadmac_disable_path((UInt16)
		(DSP_AADMAC_PRI_MIC_EN) |
		(UInt16) (DSP_AADMAC_PACKED_16BIT_IN_OUT_EN));
#endif
	audio_control_dsp(AUDDRV_DSPCMD_MUTE_DSP_UL, 0, 0, 0, 0, 0);
	audio_control_dsp(AUDDRV_DSPCMD_AUDIO_ENABLE, FALSE, 0, 0, 0, 0);

	currVoiceMic = AUDIO_SOURCE_UNDEFINED;
}


/*=============================================================================
//
// Function Name: AUDDRV_SetAudioMode_voicerecord
//
// Description:   set audio mode for voice call.
//
//=============================================================================
*/
void AUDDRV_SetAudioMode_voicerecord(AudioMode_t audio_mode,
					AudioApp_t audio_app,
					CSL_CAPH_PathID ulPathID,
					CSL_CAPH_PathID ul2PathID)
{
	aTrace(LOG_AUDIO_DRIVER,
			"%s mode==%d, app=%d\n\r", __func__,
			audio_mode, audio_app);

	RPC_SetProperty(RPC_PROP_AUDIO_MODE,
	      (UInt32) (audio_mode +
		audio_app * AUDIO_MODE_NUMBER));

	audio_control_generic(AUDDRV_CPCMD_PassAudioMode,
			      (UInt32) audio_mode, (UInt32) audio_app, 0, 0, 0);
	audio_control_generic(AUDDRV_CPCMD_SetAudioMode,
			      (UInt32) (audio_mode +
					audio_app * AUDIO_MODE_NUMBER),
			      (UInt32) audio_app, 0, 0, 0);

/*load speaker EQ filter and Mic EQ filter from sysparm to DSP*/
/* It means mic1, mic2, speaker */
	if (userEQOn == FALSE) {
		/* Use the old code, before CP function
		   audio_control_BuildDSPUlCompfilterCoef() is updated to
		   handle the mode properly.*/
		if (audio_app == AUDIO_APP_VOICE_CALL_WB)
			audio_control_generic(AUDDRV_CPCMD_SetFilter,
				audio_mode + AUDIO_MODE_NUMBER, 7, 0, 0, 0);
		else
			audio_control_generic(AUDDRV_CPCMD_SetFilter,
				audio_mode % AUDIO_MODE_NUMBER, 7, 0, 0, 0);
		/*
		audio_control_generic(AUDDRV_CPCMD_SetFilter,
				audio_mode + audio_app*AUDIO_MODE_NUMBER,
				7, 0, 0, 0);
		audio_control_generic(AUDDRV_CPCMD_SetFilter,
				audio_mode + audio_app * AUDIO_MODE_NUMBER,
				1, 0, 0, 0);
		audio_control_generic(AUDDRV_CPCMD_SetFilter,
				audio_mode + audio_app * AUDIO_MODE_NUMBER,
				2, 0, 0, 0);
		audio_control_generic(AUDDRV_CPCMD_SetFilter,
				audio_mode + audio_app * AUDIO_MODE_NUMBER,
				4, 0, 0, 0);
		*/
	}

	AUDDRV_SetAudioMode_Mic(audio_mode, audio_app, ulPathID, ul2PathID);
}


/*=============================================================================
//
// Function Name: AUDDRV_SetAudioMode
//
// Description:   set audio mode for voice call.
//
//=============================================================================
*/
void AUDDRV_SetAudioMode(AudioMode_t audio_mode, AudioApp_t audio_app,
	CSL_CAPH_PathID ulPathID,
	CSL_CAPH_PathID ul2PathID,
	CSL_CAPH_PathID dlPathID)
{
	SetAudioMode_Sp_t sp_struct;
	aTrace(LOG_AUDIO_DRIVER,
			"%s mode==%d, app=%d\n\r", __func__,
			audio_mode, audio_app);

	RPC_SetProperty(RPC_PROP_AUDIO_MODE,
	      (UInt32) (audio_mode +
		audio_app * AUDIO_MODE_NUMBER));

	audio_control_generic(AUDDRV_CPCMD_PassAudioMode,
			      (UInt32) audio_mode, (UInt32) audio_app, 0, 0, 0);
	audio_control_generic(AUDDRV_CPCMD_SetAudioMode,
			      (UInt32) (audio_mode +
					audio_app * AUDIO_MODE_NUMBER),
			      (UInt32) audio_app, 0, 0, 0);

/*load speaker EQ filter and Mic EQ filter from sysparm to DSP*/
/* It means mic1, mic2, speaker */
	if (userEQOn == FALSE) {
		/* Use the old code, before CP function
		   audio_control_BuildDSPUlCompfilterCoef() is updated to
		   handle the mode properly.*/
		if (audio_app == AUDIO_APP_VOICE_CALL_WB)
			audio_control_generic(AUDDRV_CPCMD_SetFilter,
				audio_mode + AUDIO_MODE_NUMBER, 7, 0, 0, 0);
		else
			audio_control_generic(AUDDRV_CPCMD_SetFilter,
				audio_mode % AUDIO_MODE_NUMBER, 7, 0, 0, 0);
		/*
		audio_control_generic(AUDDRV_CPCMD_SetFilter,
				audio_mode + audio_app*AUDIO_MODE_NUMBER,
				7, 0, 0, 0);
		audio_control_generic(AUDDRV_CPCMD_SetFilter,
				audio_mode + audio_app * AUDIO_MODE_NUMBER,
				1, 0, 0, 0);
		audio_control_generic(AUDDRV_CPCMD_SetFilter,
				audio_mode + audio_app * AUDIO_MODE_NUMBER,
				2, 0, 0, 0);
		audio_control_generic(AUDDRV_CPCMD_SetFilter,
				audio_mode + audio_app * AUDIO_MODE_NUMBER,
				4, 0, 0, 0);
		*/
	}
	/* else */
	/*There is no need for this function to load the ECI-headset-provided
	   speaker EQ filter and Mic EQ filter to DSP.
	   //The ECI headset enable/disable request comes with the data.
	   It means we'll get the coefficients every time if ECI headset on. */
/* audio_cmf_filter((AudioCompfilter_t *) &copy_of_AudioCompfilter ); */

	AUDDRV_SetAudioMode_Mic(audio_mode, audio_app, ulPathID, ul2PathID);

	sp_struct.mode = audio_mode;
	sp_struct.app = audio_app;
	sp_struct.pathID = dlPathID;
	sp_struct.inHWlpbk = FALSE;
	sp_struct.mixInGain_mB = GAIN_SYSPARM;
	sp_struct.mixInGainR_mB = GAIN_SYSPARM;
	sp_struct.mixOutGain_mB = GAIN_SYSPARM;
	sp_struct.mixOutGainR_mB = GAIN_SYSPARM;
	AUDDRV_SetAudioMode_Speaker(sp_struct);
}

#ifdef CONFIG_ENABLE_SSMULTICAST
/*=============================================================================
//
// Function Name: AUDDRV_SetAudioMode_Multicast
//
// Description:   set audio mode for IHF+HS Multicast.
//		For SS multicase both HS and IHF params are from mode SPEAKER
//=============================================================================
*/
void AUDDRV_SetAudioMode_Multicast(SetAudioMode_Sp_t param)
{
	int mixInGain, mixOutGain, mixBitSel;	/* Register value. */
	int mixInGainR, mixOutGainR, mixBitSelR;	/* Register value. */
	CSL_CAPH_HWConfig_Table_t *path = NULL;
	CSL_CAPH_MIXER_e outChnl = CSL_CAPH_SRCM_CH_NONE;

#ifdef CONFIG_BCM_MODEM
	SysAudioParm_t *p;
#else
	AudioSysParm_t *p;
#endif
	/*For SS Multicast mode is always AUDIO_MODE_SPEAKERPHONE*/
	p = &(AudParmP()[AUDIO_MODE_SPEAKERPHONE + \
			param.app * AUDIO_MODE_NUMBER]);

	aTrace(LOG_AUDIO_DRIVER,  "%s mode=%d, app %d, pathID %d\n",
			__func__, param.mode, param.app, param.pathID);

	mixInGain = mixOutGain = mixBitSel = 0;
	mixInGainR = mixOutGainR = mixBitSelR = 0;
	/* Load the speaker gains form sysparm. */

	/*determine which mixer output to apply the gains to */

	switch (param.mode) {
	case AUDIO_MODE_HEADSET:
	case AUDIO_MODE_TTY:
		outChnl = CSL_CAPH_SRCM_STEREO_CH1;
		break;

	case AUDIO_MODE_SPEAKERPHONE:
		outChnl = csl_caph_FindMixer(CSL_CAPH_DEV_IHF, 0);
		break;
	default:
		break;
	}

	/*Load HW Mixer gains from sysparm */

	if (param.mixInGain_mB == GAIN_SYSPARM &&
		param.mixInGainR_mB == GAIN_SYSPARM) {
		/*GAIN_SYSPARM means use sysparm*/
		mixInGain = (short)p->srcmixer_input_gain_l; /* Q13p2 dB */
		mixInGain = mixInGain * 25;	/* into mB */
		mixInGainR = (short)p->srcmixer_input_gain_r; /* Q13p2 dB */
		mixInGainR = mixInGainR * 25;	/* into mB */
	} else if (param.mixInGain_mB == GAIN_SYSPARM) {
		mixInGain = (short)p->srcmixer_input_gain_l; /* Q13p2 dB */
		mixInGain = mixInGain * 25;	/* into mB */
		mixInGainR = param.mixInGainR_mB;
	} else if (param.mixInGainR_mB == GAIN_SYSPARM) {
		mixInGainR = (short)p->srcmixer_input_gain_r; /* Q13p2 dB */
		mixInGainR = mixInGainR * 25;	/* into mB */
		mixInGain = param.mixInGain_mB;
	} else {
		mixInGain = param.mixInGain_mB;
		mixInGainR = param.mixInGainR_mB;
	}

	/*determine which mixer input to apply the gains to */

	if (param.pathID >= 1)
		path = csl_caph_FindPath(param.pathID);

	if (path != 0) {
		int i, j;
		for (i = 0; i < MAX_SINK_NUM; i++)
			for (j = 0; j < MAX_BLOCK_NUM; j++)
				if (path->srcmRoute[i][j].outChnl !=
				CSL_CAPH_SRCM_CH_NONE) {
					/*and supposedly
					path->srcmRoute[i][j].sink
					matches this speaker*/

					outChnl = path->srcmRoute[i][j].outChnl;

					aTrace(LOG_AUDIO_DRIVER,
					"%s pathID %d outChnl 0x%x inChnl 0x%x\n",
					__func__, param.pathID, outChnl,
					path->srcmRoute[i][j].inChnl);

					aTrace(LOG_AUDIO_DRIVER,
						"mixInGain 0x%x, mixInGainR 0x%x\n",
						mixInGain, mixInGainR);
					csl_srcmixer_setMixInGain(
						  path->srcmRoute[i][j].inChnl,
						  path->srcmRoute[i][j].outChnl,
						  mixInGain, mixInGainR);
				}
	} else {
		aError(
		"AUDDRV_SetAudioMode_Multicast can not find path\n");
	}

	if (outChnl) {
		if (param.mixOutGain_mB == GAIN_SYSPARM &&
			param.mixOutGainR_mB == GAIN_SYSPARM) {
			/* Q13p2 dB */
			mixOutGain = (short)p->srcmixer_output_fine_gain_l;
			mixOutGain = mixOutGain * 25;	/*into mB */
			mixOutGainR = (short)p->srcmixer_output_fine_gain_r;
			mixOutGainR = mixOutGainR * 25;	/*into mB */
		} else if (param.mixOutGain_mB == GAIN_SYSPARM) {
			mixOutGain = (short)p->srcmixer_output_fine_gain_l;
			mixOutGain = mixOutGain * 25;	/*into mB */
			mixOutGainR = param.mixOutGainR_mB;
		} else if (param.mixOutGainR_mB == GAIN_SYSPARM) {
			mixOutGainR = (short)p->srcmixer_output_fine_gain_r;
			mixOutGainR = mixOutGainR * 25;	/*into mB */
			mixOutGain = param.mixOutGain_mB;
		} else {
			mixOutGain = param.mixOutGain_mB;
			mixOutGainR = param.mixOutGainR_mB;
		}

		/* Q13p2 dB */
		mixBitSel = (short)p->srcmixer_output_coarse_gain_l;
		mixBitSel = mixBitSel / 24;
		/* bit_shift */

		mixBitSelR = (short)p->srcmixer_output_coarse_gain_r;
		mixBitSelR = mixBitSelR / 24;
		/* bit_shift */

		aTrace(LOG_AUDIO_DRIVER,
			"%s : mixOutGain 0x%x, mixOutGainR 0x%x, "
			"mixBitSel %d, mixBitSelR %d\n",
			__func__, mixOutGain, mixOutGainR,
			mixBitSel, mixBitSelR);
		csl_srcmixer_setMixBitSel(outChnl, mixBitSel, mixBitSelR);
		csl_srcmixer_setMixOutGain(outChnl, mixOutGain, mixOutGainR);
	}


/* Do not enable/disable sidetone path based on sysparm when in HW loopback */
	if (!param.inHWlpbk) {
		/*Config sidetone */
		void *coef = NULL;
		UInt16 gain = 0;
		UInt16 enable = 0;

		enable = p->hw_sidetone_enable;
		if (!enable) {
			AUDDRV_HW_DisableSideTone(param.mode);
		} else {
			/*first step: enable sidetone */
			AUDDRV_HW_EnableSideTone(param.mode);

			/*second step: set filter and gain. */
			coef = (void *)&(p->hw_sidetone_eq[0]);
			AUDDRV_HW_SetFilter(AUDDRV_SIDETONE_FILTER, coef);

			gain = p->hw_sidetone_gain;
			csl_caph_audioh_sidetone_set_gain(gain);
		}
	}
}
#endif

/*=============================================================================
//
// Function Name: AUDDRV_SetAudioMode_Speaker
//
// Description:   set audio mode.
//
//=============================================================================
*/
void AUDDRV_SetAudioMode_Speaker(SetAudioMode_Sp_t param)
{
	int mixInGain, mixOutGain, mixBitSel;	/* Register value. */
	int mixInGainR, mixOutGainR, mixBitSelR;	/* Register value. */
	CSL_CAPH_HWConfig_Table_t *path = NULL;
	CSL_CAPH_MIXER_e outChnl = CSL_CAPH_SRCM_CH_NONE;

#ifdef CONFIG_BCM_MODEM
	SysAudioParm_t *p;
#else
	AudioSysParm_t *p;
#endif

	p = &(AudParmP()[param.mode + param.app * AUDIO_MODE_NUMBER]);

	aTrace(LOG_AUDIO_DRIVER,  "%s mode=%d, app %d, pathID %d\n",
			__func__, param.mode, param.app, param.pathID);

	mixInGain = mixOutGain = mixBitSel = 0;
	mixInGainR = mixOutGainR = mixBitSelR = 0;
	/* Load the speaker gains form sysparm. */

	/*determine which mixer output to apply the gains to */

	switch (param.mode) {
	case AUDIO_MODE_HANDSET:
	case AUDIO_MODE_HAC:
		outChnl = CSL_CAPH_SRCM_STEREO_CH2_L;
		break;

	case AUDIO_MODE_HEADSET:
	case AUDIO_MODE_TTY:
/*outChnl = (CSL_CAPH_SRCM_STEREO_CH1_L | CSL_CAPH_SRCM_STEREO_CH1_R);*/
		outChnl = CSL_CAPH_SRCM_STEREO_CH1;
		break;

	case AUDIO_MODE_SPEAKERPHONE:
		outChnl = csl_caph_FindMixer(CSL_CAPH_DEV_IHF, 0);
		break;

	case AUDIO_MODE_BLUETOOTH:
#ifdef CONFIG_AUDIO_FEATURE_SET_DISABLE_ECNS
	case AUDIO_MODE_HANDSFREE:
#endif
		outChnl =
		    csl_caph_hwctrl_GetMixerOutChannel(CSL_CAPH_DEV_BT_SPKR);
		break;

	default:
		break;
	}

	/*Load HW Mixer gains from sysparm */

	if (param.mixInGain_mB == GAIN_SYSPARM &&
		param.mixInGainR_mB == GAIN_SYSPARM) {
		/*GAIN_SYSPARM means use sysparm*/
		mixInGain = (short)p->srcmixer_input_gain_l; /* Q13p2 dB */
		mixInGain = mixInGain * 25;	/* into mB */
		mixInGainR = (short)p->srcmixer_input_gain_r; /* Q13p2 dB */
		mixInGainR = mixInGainR * 25;	/* into mB */
	} else if (param.mixInGain_mB == GAIN_SYSPARM) {
		mixInGain = (short)p->srcmixer_input_gain_l; /* Q13p2 dB */
		mixInGain = mixInGain * 25;	/* into mB */
		mixInGainR = param.mixInGainR_mB;
	} else if (param.mixInGainR_mB == GAIN_SYSPARM) {
		mixInGainR = (short)p->srcmixer_input_gain_r; /* Q13p2 dB */
		mixInGainR = mixInGainR * 25;	/* into mB */
		mixInGain = param.mixInGain_mB;
	} else {
		mixInGain = param.mixInGain_mB;
		mixInGainR = param.mixInGainR_mB;
	}
	if (param.mixOutGain_mB == GAIN_SYSPARM &&
		param.mixOutGainR_mB == GAIN_SYSPARM) {
		/* Q13p2 dB */
		mixOutGain = (short)p->srcmixer_output_fine_gain_l;
		mixOutGain = mixOutGain * 25;	/*into mB */
		mixOutGainR = (short)p->srcmixer_output_fine_gain_r;
		mixOutGainR = mixOutGainR * 25;	/*into mB */
	} else if (param.mixOutGain_mB == GAIN_SYSPARM) {
		mixOutGain = (short)p->srcmixer_output_fine_gain_l;
		mixOutGain = mixOutGain * 25;	/*into mB */
		mixOutGainR = param.mixOutGainR_mB;
	} else if (param.mixOutGainR_mB == GAIN_SYSPARM) {
		mixOutGainR = (short)p->srcmixer_output_fine_gain_r;
		mixOutGainR = mixOutGainR * 25;	/*into mB */
		mixOutGain = param.mixOutGain_mB;
	} else {
		mixOutGain = param.mixOutGain_mB;
		mixOutGainR = param.mixOutGainR_mB;

		/*aTrace(LOG_AUDIO_DRIVER,
			"%s copy mixOutGain %d, %d\n",
			__func__, mixOutGain, mixOutGainR);*/
	}

	/* Q13p2 dB */
	mixBitSel = (short)p->srcmixer_output_coarse_gain_l;
	mixBitSel = mixBitSel / 24;
	/* bit_shift */

	mixBitSelR = (short)p->srcmixer_output_coarse_gain_r;
	mixBitSelR = mixBitSelR / 24;
	/* bit_shift */

	aTrace(LOG_AUDIO_DRIVER,
		"%s: mixOutGain 0x%x, mixOutGainR 0x%x, "
		"mixBitSel %d, mixBitSelR %d\n",
		__func__, mixOutGain, mixOutGainR,
		mixBitSel, mixBitSelR);

	/*determine which which mixer input to apply the gains to */

	if (param.pathID >= 1)
		path = csl_caph_FindPath(param.pathID);

	if (path != 0) {
		/*if (path->sink[0] == CSL_CAPH_DEV_DSP_throughMEM)
			outChnl = path->srcmRoute[0][0].outChnl;*/

		int i, j;
		for (i = 0; i < MAX_SINK_NUM; i++)
			for (j = 0; j < MAX_BLOCK_NUM; j++)
				if (path->srcmRoute[i][j].outChnl !=
				CSL_CAPH_SRCM_CH_NONE) {
					/*and supposedly
					path->srcmRoute[i][j].sink
					matches this speaker*/

					outChnl = path->srcmRoute[i][j].outChnl;

					aTrace(LOG_AUDIO_DRIVER,
					"%s pathID %d outChnl 0x%x inChnl 0x%x\n",
					__func__, param.pathID, outChnl,
					path->srcmRoute[i][j].inChnl);

					aTrace(LOG_AUDIO_DRIVER,
						"mixInGain 0x%x, mixInGainR 0x%x\n",
						mixInGain, mixInGainR);

					if (path->srcmRoute[i][j].outChnl ==
						CSL_CAPH_SRCM_STEREO_CH2_L
					|| path->srcmRoute[i][j].outChnl ==
						CSL_CAPH_SRCM_STEREO_CH2_R
					) {
						/*mono output*/

					if (path->srcmRoute[i][j].inChnl ==
							 (int )	CAPH_SRCM_CH5
						|| path->srcmRoute[i][j].inChnl
							== (int) CAPH_SRCM_PASSCH1
						|| path->srcmRoute[i][j].inChnl
							==( int ) CAPH_SRCM_PASSCH2) {
						/*only on stereo inputs.*/
						if (mixInGain != GAIN_NA &&
						mixInGainR != GAIN_NA) {
							mixInGain =
								mixInGain-602;
							mixInGainR =
								mixInGainR-602;
							}
						}
					}

					csl_srcmixer_setMixInGain(
						  path->srcmRoute[i][j].inChnl,
						  path->srcmRoute[i][j].outChnl,
						  mixInGain, mixInGainR);
					csl_srcmixer_setMixBitSel(
						outChnl, mixBitSel, mixBitSelR);
					csl_srcmixer_setMixOutGain(
					outChnl, mixOutGain, mixOutGainR);
				}
	} else {
		aError(
		"AUDDRV_SetAudioMode_Speaker can not find path\n");
		/*do not know which mix input to apply gain on
		if (outChnl)
			csl_srcmixer_setMixAllInGain(outChnl,
				mixInGain, mixInGain);
		*/
	}

	if (path != 0) {
		if (path->sink[0] == CSL_CAPH_DEV_DSP_throughMEM)
			return;
/*no need for anything other than HW mixer gain for arm2sp */
	}

/* Do not enable/disable sidetone path based on sysparm when in HW loopback */
	if (!param.inHWlpbk) {
		/*Config sidetone */
		void *coef = NULL;
		UInt16 gain = 0;
		UInt16 enable = 0;

		enable = p->hw_sidetone_enable;
		if (!enable) {
			AUDDRV_HW_DisableSideTone(param.mode);
		} else {
			/*first step: enable sidetone */
			AUDDRV_HW_EnableSideTone(param.mode);

			/*second step: set filter and gain. */
			coef = (void *)&(p->hw_sidetone_eq[0]);
			AUDDRV_HW_SetFilter(AUDDRV_SIDETONE_FILTER, coef);

			gain = p->hw_sidetone_gain;
			csl_caph_audioh_sidetone_set_gain(gain);
		}
	}

}

/*=============================================================================
//
// Function Name: AUDDRV_SetAudioMode_Mic
//
// Description:   set audio mode.
//
//=============================================================================
*/
void AUDDRV_SetAudioMode_Mic(AudioMode_t audio_mode,
					AudioApp_t app,
					unsigned int arg_pathID,
					unsigned int arg_pathID2)
{
	/* for 48 KHz recording no DSP and 8KHz recording through DSP. */

	UInt16 gainTemp1 = 0, gainTemp2 = 0, gainTemp3 = 0, gainTemp4 = 0;

#ifdef CONFIG_BCM_MODEM
	SysAudioParm_t *p;
#else
	AudioSysParm_t *p;
#endif

	p = &(AudParmP()[audio_mode + app * AUDIO_MODE_NUMBER]);

	/* Load the mic gains from sysparm. */

	/***
	do not touch DSP UL gain in this function.
	if(isDSPNeeded == TRUE)
	{
		dspULGain = 64;
		//AudParmP()[mode].echoNlp_parms.echo_nlp_gain;
		audio_control_generic( AUDDRV_CPCMD_SetBasebandUplinkGain,
				dspULGain, 0, 0, 0, 0);
	}
	***/

	gainTemp1 = p->mic_pga; /* Q13p2 */
	csl_caph_audioh_setMicPga_by_mB(gainTemp1 * 25);

	aTrace(LOG_AUDIO_DRIVER, "%s mode=%d, app=%d mic_gain %d\n",
		__func__, audio_mode, app, gainTemp1*25);

	gainTemp1 = p->amic_dga_coarse_gain;	/* Q13p2 dB */
	gainTemp2 = p->amic_dga_fine_gain;	/* Q13p2 dB */

	gainTemp1 = p->dmic1_dga_coarse_gain;
	/* dmic1_dga_coarse_gain is the same register as amic_dga_coarse_gain */
	gainTemp2 = p->dmic1_dga_fine_gain;

	gainTemp3 = p->dmic2_dga_coarse_gain;
	gainTemp4 = p->dmic2_dga_fine_gain;

	csl_caph_audioh_vin_set_cic_scale_by_mB(((int)gainTemp1) * 25,
						((int)gainTemp2) * 25,
						((int)gainTemp3) * 25,
						((int)gainTemp4) * 25);

	gainTemp1 = p->dmic3_dga_coarse_gain;
	gainTemp2 = p->dmic3_dga_fine_gain;
	gainTemp3 = p->dmic4_dga_coarse_gain;
	gainTemp4 = p->dmic4_dga_fine_gain;

	csl_caph_audioh_nvin_set_cic_scale_by_mB(((int)gainTemp1) * 25,
						 ((int)gainTemp2) * 25,
						 ((int)gainTemp3) * 25,
						 ((int)gainTemp4) * 25);

}

/*=============================================================================
//
// Function Name: AUDDRV_User_CtrlDSP
//
// Description:   Control DSP Algo
//
//=============================================================================
*/
int AUDDRV_User_CtrlDSP(AudioDrvUserCtrl_t UserCtrlType, Boolean enable,
			Int32 size, void *param)
{
#if defined(CONFIG_BCM_MODEM)
	static Boolean ConfigSP = FALSE;
	static void *spCtrl, *spVar;

	aTrace(LOG_AUDIO_DRIVER,  "AUDDRV_User_CtrlDSP");

	switch (UserCtrlType) {
	case AUDDRV_USER_SP_QUERY:
		aTrace(LOG_AUDIO_DRIVER,  "%s AUDDRV_USER_SP_QUERY", __func__);

		if (param == NULL)
			return -EINVAL;

		csl_dsp_sp_query_msg((UInt32 *) param);

		break;
	case AUDDRV_USER_SP_CTRL:
		aTrace(LOG_AUDIO_DRIVER,  "%s AUDDRV_USER_SP_CTRL", __func__);

		if (enable) {
			if (size <= 0 || param == NULL)
				return -EINVAL;

			if (spVar != NULL)
				csl_dsp_sp_cnfg_msg((UInt16) enable, 0, 1,
						    (UInt32 *) param, spVar);
			else {
				spCtrl = kzalloc(size, GFP_KERNEL);
				if (!spCtrl)
					return -ENOMEM;

				memcpy(spCtrl, param, size);
				ConfigSP = TRUE;
			}
		} else {
			if (spVar != NULL)
				kfree(spVar);
			if (spCtrl != NULL)
				kfree(spCtrl);
			spVar = NULL;
			spCtrl = NULL;
			csl_dsp_sp_cnfg_msg((UInt16) enable, 0, 0, NULL, NULL);
		}
		break;
	case AUDDRV_USER_SP_VAR:
		aTrace(LOG_AUDIO_DRIVER,  "%s AUDDRV_USER_SET_SP_VAR",
				__func__);

		if (ConfigSP == TRUE && spCtrl != NULL) {
			if (param == NULL)
				return -EINVAL;

			csl_dsp_sp_cnfg_msg(1, 0, 1, spCtrl,
					    (UInt32 *) param);
			ConfigSP = FALSE;
		} else {
			if (size <= 0 || param == NULL)
				return -EINVAL;

			if (spVar == NULL) {
				spVar = kzalloc(size, GFP_KERNEL);
				if (!spVar)
					return -ENOMEM;
				memcpy(spVar, param, size);
			}
			csl_dsp_sp_ctrl_msg((UInt32 *) param);
		}

		break;
	case AUDDRV_USER_EQ_CTRL:
		aTrace(LOG_AUDIO_DRIVER,  "%s AUDDRV_USER_EQ_CTRL", __func__);

		if (enable == TRUE) {
			if (param == NULL)
				return -EINVAL;

			userEQOn = TRUE;
			audio_cmf_filter((AudioCompfilter_t *) param);
		} else
			userEQOn = FALSE;

		break;
	default:
		aTrace(LOG_AUDIO_DRIVER,  "%s Invalid request %d\n\r",
				__func__, UserCtrlType);
		break;
	}
#else
	aTrace(LOG_AUDIO_DRIVER,  "AUDDRV_User_CtrlDSP:dummy for AP only");
#endif
	return 0;
}

/*=============================================================================
//
// Function Name: AUDDRV_User_HandleDSPInt
//
// Description:   Handle DSP Interrupt
//
//=============================================================================
*/
void AUDDRV_User_HandleDSPInt(UInt32 param1, UInt32 param2, UInt32 param3)
{
	aTrace(LOG_AUDIO_DRIVER,  "AUDDRV_User_HandleDSPInt");
	if (sUserCB)
		((AUDDRV_User_CB) sUserCB) (param1, param2, param3);
}

/*=============================================================================
//
// Function Name: AUDDRV_ControlFlagFor_CustomGain
//
// Description:   Set a flag to allow custom gain settings.
//		If the flag is set the above three parameters are not set
//		in AUDDRV_SetAudioMode( ) .
//
//=============================================================================
*/
void AUDDRV_ControlFlagFor_CustomGain(Boolean on_off)
{
	controlFlagForCustomGain = on_off;
}

/*=============================================================================
//
// Function Name: AUDDRV_SetULSpeechRecordGain
//
// Description:   set UL speech recording gain
//
//=============================================================================
*/
void AUDDRV_SetULSpeechRecordGain(Int16 gain)
{
	audio_control_generic(AUDDRV_CPCMD_SetULSpeechRecordGain,
			      (UInt32) gain, 0, 0, 0, 0);
	return;
}

/******************************************************************************
*
* Function Name: AUDDRV_SetTuningFlag
*
* Description:   set Audio Tuning Flag
*
******************************************************************************/
void AUDDRV_SetTuningFlag(int flag)
{
	audio_tuning_flag = flag;
}

/******************************************************************************
*
* Function Name: AUDDRV_TuningFlag
*
* Description:   read Audio Tuning Flag
*
******************************************************************************/
int AUDDRV_TuningFlag(void)
{
	return audio_tuning_flag;
}

/*=============================================================================
// Private function prototypes
//=============================================================================

//=============================================================================
//
// Function Name: AUDDRV_Telephony_InitHW
//
// Description:   Enable the HW for Telephone voice call
//
//=============================================================================
*/
static void AUDDRV_Telephony_InitHW(AUDIO_SOURCE_Enum_t mic,
				    AUDIO_SINK_Enum_t speaker,
				    AUDIO_SAMPLING_RATE_t sample_rate,
				    Boolean bNeedDualMic)
{
	CSL_CAPH_HWCTRL_CONFIG_t config;
	UInt32 *memAddr = 0;
	AUDIO_BITS_PER_SAMPLE_t bits = 24;
	CSL_CAPH_PathID pathID;

#if defined(ENABLE_BT16)
	if (speaker == AUDIO_SINK_BTM)
		bits = 16;
#endif

	aTrace(LOG_AUDIO_DRIVER,  "%s mic=%d, spkr=%d sample_rate=%u*\n",
		__func__, mic, speaker, sample_rate);

	memset(&config, 0, sizeof(CSL_CAPH_HWCTRL_CONFIG_t));

	telephonyPathID.ulPathID = 0;
	telephonyPathID.ul2PathID = 0;
	telephonyPathID.dlPathID = 0;

	/* DL */
	config.streamID = CSL_CAPH_STREAM_NONE;
	config.pathID = 0;
	config.source = CSL_CAPH_DEV_DSP;
	config.sink = AUDDRV_GetDRVDeviceFromSpkr(speaker);
	config.dmaCH = CSL_CAPH_DMA_NONE;
	/*If DSP DL goes to IHF, Sample rate should be 48KHz. */
	if (speaker == AUDIO_SINK_LOUDSPK) {
		config.source = CSL_CAPH_DEV_DSP_throughMEM;
		config.src_sampleRate = AUDIO_SAMPLING_RATE_48000;
	} else {
		config.src_sampleRate = sample_rate;
	}
	config.snk_sampleRate = AUDIO_SAMPLING_RATE_48000;

	if (config.sink == CSL_CAPH_DEV_HS)
		config.chnlNum = AUDIO_CHANNEL_STEREO;
	else
		config.chnlNum = AUDIO_CHANNEL_MONO;

	config.bitPerSample = bits;

	sink = config.sink;
	if (sink == CSL_CAPH_DEV_IHF) {
		memAddr = AUDIO_GetIHF48KHzBufferBaseAddress();

		config.src_sampleRate = AUDIO_SAMPLING_RATE_48000;
		config.source = CSL_CAPH_DEV_DSP_throughMEM;
		/* csl_caph_EnablePath() handles the case DSP_MEM
		when sink is IHF */

		csl_caph_hwctrl_setDSPSharedMemForIHF((UInt32) memAddr);
	} else {
		config.source = CSL_CAPH_DEV_DSP;
	}

	telephonyPathID.dlPathID = csl_caph_hwctrl_EnablePath(config);


	/* UL */
	pathID = csl_caph_FindPathID(CSL_CAPH_DEV_DSP,
		AUDDRV_GetDRVDeviceFromMic(mic), 0);
	if (pathID) {
		/* If voice recording is ongoing, no need to set up UL path.
		 * Unable to handle this case: record and call use different
		 * mics.
		 */
		aTrace(LOG_AUDIO_DRIVER, "%s UL path %d exists\n",
			__func__, pathID);
		AUDDRV_DisableDSPInput(0);
		/*do not set ulPathID before AUDDRV_DisableDSPInput*/
		telephonyPathID.ulPathID = pathID;
	}

	/* Secondary mic */
	/* If Dual Mic is enabled. Theoretically DMIC3 or DMIC4 are used*/
	if (bNeedDualMic) {
		config.streamID = CSL_CAPH_STREAM_NONE;
		config.pathID = 0;
		config.source = DUALMICS_NOISE_REF_MIC;
		config.sink = CSL_CAPH_DEV_DSP;
		config.dmaCH = CSL_CAPH_DMA_NONE;
		config.src_sampleRate = AUDIO_SAMPLING_RATE_48000;
		config.snk_sampleRate = sample_rate;
		config.chnlNum = AUDIO_CHANNEL_MONO;
		config.bitPerSample = bits;

		telephonyPathID.ul2PathID = csl_caph_hwctrl_EnablePath(config);
	}

	/* Primary mic */
	config.streamID = CSL_CAPH_STREAM_NONE;
	config.pathID = 0;
	config.source = AUDDRV_GetDRVDeviceFromMic(mic);
	config.sink = CSL_CAPH_DEV_DSP;
	config.dmaCH = CSL_CAPH_DMA_NONE;
	config.src_sampleRate = AUDIO_SAMPLING_RATE_48000;
	config.snk_sampleRate = sample_rate;
	config.chnlNum = AUDIO_CHANNEL_MONO;
	config.bitPerSample = bits;

	if (voiceRecOn && voiceInMic != mic)
		aError("%s voice record (%d) and call (%d) different mics\n",
			__func__, voiceInMic, mic);
	if (!pathID)
		telephonyPathID.ulPathID = csl_caph_hwctrl_EnablePath(config);

	return;
}

/*=============================================================================
//
// Function Name: AUDDRV_Telephony_DeinitHW
//
// Description:   Disable the HW for Telephone voice call
//
//=============================================================================
*/
static void AUDDRV_Telephony_DeinitHW(void)
{
	CSL_CAPH_HWCTRL_CONFIG_t config;
	aTrace(LOG_AUDIO_DRIVER,  "AUDDRV_Telephony_DeinitHW\n");

	memset(&config, 0, sizeof(CSL_CAPH_HWCTRL_CONFIG_t));

	AUDDRV_HW_DisableSideTone(AUDCTRL_GetAudioMode());

	config.streamID = CSL_CAPH_STREAM_NONE;
	config.pathID = telephonyPathID.ulPathID;

	if (!voiceRecOn)
		csl_caph_hwctrl_DisablePath(config);

	if (telephonyPathID.ul2PathID != 0) {
		config.streamID = CSL_CAPH_STREAM_NONE;
		config.pathID = telephonyPathID.ul2PathID;

		(void)csl_caph_hwctrl_DisablePath(config);
	}

	config.streamID = CSL_CAPH_STREAM_NONE;
	config.pathID = telephonyPathID.dlPathID;

	(void)csl_caph_hwctrl_DisablePath(config);

	sink = CSL_CAPH_DEV_NONE;

	telephonyPathID.ulPathID = 0;
	telephonyPathID.ul2PathID = 0;
	telephonyPathID.dlPathID = 0;

	return;
}

/****************************************************************************
*
*  Function Name: AUDDRV_HW_SetFilter(AUDDRV_HWCTRL_FILTER_e filter,
*                                                     void* coeff)
*
*  Description: Load filter coefficients
*
****************************************************************************/
static void AUDDRV_HW_SetFilter(AUDDRV_HWCTRL_FILTER_e filter,
				       void *coeff)
{
	if (filter == AUDDRV_SIDETONE_FILTER)
		csl_caph_audioh_sidetone_load_filter((UInt32 *) coeff);
}

/****************************************************************************
*
*  Function Name: AUDDRV_HW_EnableSideTone(AudioMode_t audio_mode)
*
*  Description: Enable Sidetone path
*
****************************************************************************/
static void AUDDRV_HW_EnableSideTone(AudioMode_t audio_mode)
{
	AUDDRV_PATH_Enum_t pathId = AUDDRV_PATH_VIBRA_OUTPUT;
	switch (audio_mode) {
	case AUDIO_MODE_HEADSET:
	case AUDIO_MODE_TTY:
		pathId = AUDDRV_PATH_HEADSET_OUTPUT;
		break;
	case AUDIO_MODE_HANDSET:
	case AUDIO_MODE_HAC:
		pathId = AUDDRV_PATH_EARPICEC_OUTPUT;
		break;
	case AUDIO_MODE_SPEAKERPHONE:
		pathId = AUDDRV_PATH_IHF_OUTPUT;
		break;

	default:
		;
	}
	csl_caph_audioh_sidetone_control((int)pathId, TRUE);
}

/****************************************************************************
*
*  Function Name: AUDDRV_HW_DisableSideTone(AudioMode_t audio_mode)
*
*  Description: Disable Sidetone path
*
****************************************************************************/
static void AUDDRV_HW_DisableSideTone(AudioMode_t audio_mode)
{
	AUDDRV_PATH_Enum_t pathId = AUDDRV_PATH_VIBRA_OUTPUT;
	switch (audio_mode) {
	case AUDIO_MODE_HEADSET:
	case AUDIO_MODE_TTY:
		pathId = AUDDRV_PATH_HEADSET_OUTPUT;
		break;
	case AUDIO_MODE_HANDSET:
	case AUDIO_MODE_HAC:
		pathId = AUDDRV_PATH_EARPICEC_OUTPUT;
		break;
	case AUDIO_MODE_SPEAKERPHONE:
		pathId = AUDDRV_PATH_IHF_OUTPUT;
		break;

	default:
		;
	}
	csl_caph_audioh_sidetone_control((int)pathId, FALSE);
}


/*==========================================================================
//
// Function Name: AUDDRV_GetDRVDeviceFromMic
//
// Description: Get the audio driver Device from the Microphone selection.
//
// =========================================================================
*/
static CSL_CAPH_DEVICE_e AUDDRV_GetDRVDeviceFromMic(AUDIO_SOURCE_Enum_t mic)
{
	CSL_CAPH_DEVICE_e dev = CSL_CAPH_DEV_NONE;

	aTrace(LOG_AUDIO_DRIVER,
			"AUDDRV_GetDRVDeviceFromMic:: mic = 0x%x\n", mic);

	switch (mic) {
	case AUDIO_SOURCE_UNDEFINED:
		dev = CSL_CAPH_DEV_NONE;
		break;

	case AUDIO_SOURCE_SPEECH_DIGI:
		/*case AUDDRV_DUAL_MIC_DIGI12:
		   //case AUDDRV_DUAL_MIC_DIGI21: */
		dev = CSL_CAPH_DEV_DIGI_MIC;
		break;

	case AUDIO_SOURCE_DIGI1:
		dev = CSL_CAPH_DEV_DIGI_MIC_L;
		break;

	case AUDIO_SOURCE_DIGI2:
		dev = CSL_CAPH_DEV_DIGI_MIC_R;
		break;

	case AUDIO_SOURCE_ANALOG_MAIN:
		dev = CSL_CAPH_DEV_ANALOG_MIC;
		break;

	case AUDIO_SOURCE_ANALOG_AUX:
		dev = CSL_CAPH_DEV_HS_MIC;
		break;

	case AUDIO_SOURCE_BTM:
		dev = CSL_CAPH_DEV_BT_MIC;
		break;

	case AUDIO_SOURCE_USB:
		dev = CSL_CAPH_DEV_MEMORY;
		break;

	default:
		break;
	};

	return dev;
}

/*==========================================================================
//
// Function Name: AUDDRV_GetDRVDeviceFromSpkr
//
// Description: Get the audio driver Device from the Speaker selection.
//
// =========================================================================
*/
static CSL_CAPH_DEVICE_e AUDDRV_GetDRVDeviceFromSpkr(AUDIO_SINK_Enum_t spkr)
{
	CSL_CAPH_DEVICE_e dev = CSL_CAPH_DEV_NONE;

	aTrace(LOG_AUDIO_DRIVER,
			"AUDDRV_GetDRVDeviceFromSpkr:: spkr = 0x%x\n", spkr);

	switch (spkr) {
	case AUDIO_SINK_UNDEFINED:
		dev = CSL_CAPH_DEV_NONE;
		break;

	case AUDIO_SINK_HANDSET:
		dev = CSL_CAPH_DEV_EP;
		break;

	case AUDIO_SINK_LOUDSPK:
		dev = CSL_CAPH_DEV_IHF;
		break;

	case AUDIO_SINK_HEADSET:
		dev = CSL_CAPH_DEV_HS;
		break;

	case AUDIO_SINK_VIBRA:
		dev = CSL_CAPH_DEV_VIBRA;
		break;

	case AUDIO_SINK_BTM:
		dev = CSL_CAPH_DEV_BT_SPKR;
		break;

	case AUDIO_SINK_USB:
		dev = CSL_CAPH_DEV_MEMORY;
		break;

	default:
		break;
	};

	return dev;
}

static UInt32 *AUDIO_GetIHF48KHzBufferBaseAddress(void)
{
/*	special path for IHF voice call
	need to use the physical address
	// Linux only change
	AP_SharedMem_t *ap_shared_mem_ptr =
	ioremap_nocache(AP_SH_BASE, AP_SH_SIZE);
	// Linux only : to get the physical address use the virtual
	address to compute offset and
	// add to the base address
	UInt32 *memAddr = (UInt32 *)(AP_SH_BASE +
	((UInt32)&(ap_shared_mem_ptr->shared_aud_out_buf_48k[0][0]) -
	(UInt32)ap_shared_mem_ptr));

	return memAddr;
*/
	return AUDIO_Return_IHF_48kHz_buffer_base_address();

}

static void AP_ProcessAudioEnableDone(UInt16 enabled_path)
{
	aTrace(LOG_AUDIO_DRIVER,
	"AP_ProcessAudioEnableDone, Got AUDIO ENABLE RESP FROM DSP 0x%x\n",
	enabled_path);

	csl_caph_dspcb(enabled_path & ~((UInt16)DSP_AADMAC_RETIRE_DS_CMD));

	complete(&audioEnableDone);
}

#if defined(CONFIG_RHEA_PANDA)
static void AP_ProcessExtModemCallDone(UInt16 enabled_path)
{
	aTrace(LOG_AUDIO_DRIVER,
			"%s, Got ExtModemCall RESP FROM DSP\n", __func__);

	if (enabled_path)
		csl_caph_hwctrl_extModem_StartPath_step2(
			ulExtModemPathID, dlExtModemPathID);
	else
		csl_caph_hwctrl_extModem_StopPath_step1(
			ulExtModemPathID, dlExtModemPathID);

	/* can proceed */
	complete(&extModemCallDone);

}
#endif

/****************************************************************************
*
* Function Name: AUDDRV_GetULPath
*
* Description:   Get UL path
*
*****************************************************************************/
int AUDDRV_GetULPath(void)
{
	int ret = (int)telephonyPathID.ulPathID;
	aTrace(LOG_AUDIO_DRIVER,  "%s %d\n", __func__, ret);
	return ret;
}

void AUDDRV_SetCallMode(Int32 callMode)
{
	curCallMode = callMode;
	aTrace(LOG_AUDIO_DRIVER,  "%s callMode = %d\n",
		__func__, (int)curCallMode);
}

void AUDDRV_CPResetCleanup(void)
{
	currVoiceMic = AUDIO_SOURCE_UNDEFINED;
	currVoiceSpkr = AUDIO_SINK_UNDEFINED;
	voiceRecOn = FALSE;
	voicePlayOutpathEnabled = FALSE;
	dspECEnable = TRUE;
	dspNSEnable = TRUE;
	inCallRateChange = FALSE;
	sink = CSL_CAPH_DEV_NONE;
	telephonyPathID.ulPathID = 0;
	telephonyPathID.ul2PathID = 0;
	telephonyPathID.dlPathID = 0;
	voiceInMic = AUDIO_SOURCE_UNDEFINED;
	voiceInSr = 0;
	curCallMode = CALL_MODE_NONE;
	init_completion(&audioEnableDone);
}
