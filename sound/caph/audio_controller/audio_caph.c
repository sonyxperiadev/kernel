/*****************************************************************************
Copyright 2010-2012 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement
governing use of this software, this software is licensed to you under the
terms of the GNU General Public License version 2, available at
http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software
in any way with any other Broadcom software provided under a license other than
the GPL, without Broadcom's express prior written consent.
*****************************************************************************/

/*
*
*****************************************************************************
*
*  audio_caph.c
*
*  PURPOSE:
*
*     Serialize audio  control operation
*	Eliminate waits in audio control because of atomic operation
*	requirement from ALSA
*
*  NOTES:
*
*****************************************************************************/

/* ---- Include Files ---------------------------------------------------- */

#include <linux/version.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kfifo.h>
#include <linux/wait.h>
#include <linux/jiffies.h>
#include <linux/completion.h>

#include "mobcom_types.h"
#include "resultcode.h"
#include "audio_consts.h"
#include <plat/osabstract/ossemaphore.h>
#include "bcm_fuse_sysparm_CIB.h"
#include "csl_caph.h"
#include "audio_vdriver.h"
#include "audio_controller.h"
#include "audio_ddriver.h"
#include "audio_caph.h"
#include "caph_common.h"
#include "audio_trace.h"
#include "audctrl_policy.h"

#define USE_HR_TIMER

#ifdef USE_HR_TIMER
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#define MS_TO_NS(x) (x * 1E6L)
static struct hrtimer hr_timer;
static ktime_t ktime;
#endif


/* ---- Data structure  ------------------------------------------------- */

/**
 * Wrap up HAL_AUDIO_Ctrl parameters
 * The record is passed to worker thread via KFIFO
 *
 */
struct _TMsgBrcmAudioCtrl {
	BRCM_AUDIO_ACTION_en_t action_code;
	BRCM_AUDIO_Control_Params_un_t param;
	void *pCallBack;
	int block;

};
#define	TMsgAudioCtrl struct _TMsgBrcmAudioCtrl

/**
 * The thread private data structure
 */
struct TAudioHalThreadData {
	/*
	 * KFIFO to pass control parameters from audio HAL caller  to
	 * worker thread
	 */
	struct kfifo m_pkfifo;
	/*
	 * spin lock to protect KFIFO access so that audio HAL can
	 * accept concurrent caller
	 */
	spinlock_t m_lock;
	/* worker thread data structure */
	struct work_struct mwork;
	struct workqueue_struct *pWorkqueue_AudioControl;
	Semaphore_t action_complete;
	struct kfifo m_pkfifo_out;
	spinlock_t m_lock_out;
};

static char action_names[ACTION_AUD_TOTAL][40] = {
		"OpenPlay",
		"ClosePlay",
		"StartPlay",
		"StopPlay",
		"PausePlay",
		"ResumePlay",
		"StartRecord",
		"StopRecord",
		"OpenRecord",
		"CloseRecord",
		"SetPrePareParameters",/* 10 */
		"AddChannel",
		"RemoveChannel",
		"EnableTelephony",
		"DisableTelephony",
		"EnableECNSTelephony",
		"DisableECNSTelephony",
		"SetTelephonyMicSpkr",
		"MutePlayback",
		"MuteRecord",
		"MuteTelephony",/* 20 */
		"EnableByPassVibra",
		"DisableByPassVibra",
		"SetPlaybackVolume",
		"SetRecordGain",
		"SetTelephonySpkrVolume",
		"SwitchSpkr",
		"SetHWLoopback",
		"SetAudioMode",
		"SetAudioApp", /*set audio profile*/
		"EnableFMPlay",
		"DisableFMPlay",
		"SetARM2SPInst",
		"RateChange", /*33 */
		"AmpEnable",
		"DisableByPassVibra_CB",
		"SetCallMode",
		"ConnectDL",
		"UpdateUserVolSetting",
		"BufferReady",
		"BTTest",
		"CfgIHF",
		"CfgSSP",
		"HwCtl",
		"AtCtl",
};

extern brcm_alsa_chip_t *sgpCaph_chip;

#if defined(CONFIG_MFD_BCM59039) | defined(CONFIG_MFD_BCM59042)
extern int bcmpmu_read_pmu_temp(void);
static BrcmPmuTempGainComp sgTempGainComp;
#endif

static unsigned int pathID[CAPH_MAX_PCM_STREAMS];
static unsigned int n_msg_in, n_msg_out, last_action;
static struct completion complete_kfifo;
static struct TAudioHalThreadData sgThreadData;
static int telephonyIsEnabled;
static DEFINE_MUTEX(mutexBlock);

#define KFIFO_SIZE		2048
#define BLOCK_WAITTIME_MS	60000
#define KFIFO_TIMEOUT_MS	60000

static void AUDIO_Ctrl_Process(BRCM_AUDIO_ACTION_en_t action_code,
			void *arg_param, void *callback, int block);

#if defined(CONFIG_MFD_BCM59039) | defined(CONFIG_MFD_BCM59042)
static void temp_gain_comp_work(struct work_struct *work);
#endif

#ifdef USE_HR_TIMER
static enum hrtimer_restart TimerCbStopVibrator(struct hrtimer *timer)
{
	Result_t status =
		AUDIO_Ctrl_Trigger(ACTION_AUD_DisableByPassVibra_CB,
		NULL, NULL, 0);

	if (status != RESULT_OK) {
		/*recur after 100ms*/
		hrtimer_forward_now(timer, ktime_set(0, (100*1000000)));
		return HRTIMER_RESTART;
	} else
		aTrace(LOG_AUDIO_CNTLR, "Disable Vib from HR Timer cb\n");

	return HRTIMER_NORESTART;
}

#else
static struct timer_list gTimerVib;
static struct timer_list *gpVibratorTimer;
void TimerCbStopVibrator(unsigned long priv)
{
	AUDIO_Ctrl_Trigger(ACTION_AUD_DisableByPassVibra, NULL, NULL, 0);
	/* AUDCTRL_DisableBypassVibra(); */

	aTrace(LOG_AUDIO_CNTLR, "Disable Vib from timer cb\n");
}
#endif

/**
 * AudioCtrlWorkThread: Worker thread, it query KFIFO for operation message
 * and call HAL_AudioProcess.
 *
 * @work: work structure
 */
static void AudioCtrlWorkThread(struct work_struct *work)
{
	TMsgAudioCtrl msgAudioCtrl;
	unsigned int len = 0;
	set_user_nice(current, -20);
	while (1) {
		/* get operation code from fifo */
		len = kfifo_out_locked(&sgThreadData.m_pkfifo,
				       (unsigned char *)&msgAudioCtrl,
				       sizeof(TMsgAudioCtrl),
				       &sgThreadData.m_lock);

		/* Commenting debug prints to eliminate compilation errors for
		 * kfifo member accesses
		 */

		/*
		 *  if( (len != sizeof(TMsgAudioCtrl)) && (len!=0) )
		 *      DEBUG("Error AUDIO_Ctrl len=%d expected %d
		 *      in=%d, out=%d\n", len, sizeof(TMsgAudioCtrl),
		 *      sgThreadData.m_pkfifo.in, sgThreadData.m_pkfifo.out);
		 */
		if (len == 0)	/* FIFO empty sleep */
			return;

		if (completion_done(&complete_kfifo) == 0) {
			aTrace(LOG_AUDIO_CNTLR, "Sending complete");
			complete(&complete_kfifo);
		}

		n_msg_in++;
		last_action = msgAudioCtrl.action_code;

		/* process the operation */
		/* This is message consumer of sgThreadData.m_pkfifo */
		AUDIO_Ctrl_Process(msgAudioCtrl.action_code,
				   &msgAudioCtrl.param,
				   msgAudioCtrl.pCallBack, msgAudioCtrl.block);
		n_msg_out++;
	}

	return;
}

/**
 * AudioCodecIdHander :callback function that handles the rate change
 *
 */
static void AudioCodecIdHander(int codecID)
{
	BRCM_AUDIO_Param_RateChange_t param_rate_change;
	aTrace(LOG_AUDIO_CNTLR,
			"AudioCodeCIdHander : CodecId = %d \r\n", codecID);
	param_rate_change.codecID = codecID;
	AUDIO_Ctrl_Trigger(ACTION_AUD_RateChange, &param_rate_change, NULL, 0);
}

/**
 * caph_audio_init: registers callback for handling rate change and
 * if any init required.
 *
 */
void caph_audio_init(void)
{
	AUDDRV_RegisterRateChangeCallback(AudioCodecIdHander);
	init_completion(&complete_kfifo);

#ifdef USE_HR_TIMER
	hrtimer_init(&hr_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	hr_timer.function = &TimerCbStopVibrator;
#endif

#if defined(CONFIG_MFD_BCM59039) | defined(CONFIG_MFD_BCM59042)
	sgTempGainComp.intiComplete = 0;
	INIT_DELAYED_WORK(&sgTempGainComp.temp_gain_comp, temp_gain_comp_work);
#endif
}

#if defined(CONFIG_MFD_BCM59039) | defined(CONFIG_MFD_BCM59042)
static void temp_gain_comp_work(struct work_struct *work)
{
	int temp = 0;
	int next_delay;
	int result;
	BrcmPmuTempGainComp *paudio =
		container_of(work, BrcmPmuTempGainComp, temp_gain_comp.work);
	if (!AUDCTRL_TempGainCompStatus())
		return;
	/* read temperature from PMU */
	temp = bcmpmu_read_pmu_temp();
	paudio->prevTemp = paudio->currTemp;
	paudio->currTemp = temp;
	next_delay = AUDCTRL_TempGainComp(paudio);
	aTrace(LOG_AUDIO_PMUTEMP, "temp_gain_comp_work:  temp = %d"
		"next_delay = %d\n", temp, next_delay);
	if (!AUDCTRL_AllPathsDisabled()) {
		result = schedule_delayed_work(&paudio->temp_gain_comp,
		msecs_to_jiffies(next_delay));
	} else {
		AUDCTRL_TempGainCompDeInit(&sgTempGainComp);
		aTrace(LOG_AUDIO_PMUTEMP, "Stopped Scheduling"
			"temp_gain_comp_work\n");
	}
}
#endif


/**
 * LaunchAudioHalThread: Create Worker thread.
 *
 */
int LaunchAudioCtrlThread(void)
{
	int ret;
	spin_lock_init(&sgThreadData.m_lock);
	spin_lock_init(&sgThreadData.m_lock_out);

	ret = kfifo_alloc(&sgThreadData.m_pkfifo, KFIFO_SIZE, GFP_KERNEL);
	/*
	 * Commenting debug prints to eliminate compilation errors for kfifo
	 * member access
	 */
	/*
	 * DEBUG("LaunchAudioCtrlThread KFIFO_SIZE= %d actual =%d\n",
	 * KFIFO_SIZE,sgThreadData.m_pkfifo.size);
	 */
	ret = kfifo_alloc(&sgThreadData.m_pkfifo_out, KFIFO_SIZE, GFP_KERNEL);
	/*
	 * Commenting debug prints to eliminate compilation errors for kfifo
	 * member access
	 */
	/*
	 * DEBUG("LaunchAudioCtrlThread KFIFO_SIZE= %d actual =%d\n",
	 * KFIFO_SIZE,sgThreadData.m_pkfifo_out.size);
	 */
	INIT_WORK(&sgThreadData.mwork, AudioCtrlWorkThread);

	sgThreadData.pWorkqueue_AudioControl = create_workqueue("AudioCtrlWq");
	if (!sgThreadData.pWorkqueue_AudioControl)
		aError("\n Error : Can not create work queue:AudioCtrlWq\n");
	sgThreadData.action_complete = OSSEMAPHORE_Create(0, 0);
#ifndef USE_HR_TIMER
	gpVibratorTimer = NULL;
#endif
	return ret;
}

/**
 * LaunchAudioHalThread: Clean up, free KFIFO
 *
 */
int TerminateAudioHalThread(void)
{

	if (sgThreadData.pWorkqueue_AudioControl) {
		flush_workqueue(sgThreadData.pWorkqueue_AudioControl);
		destroy_workqueue(sgThreadData.pWorkqueue_AudioControl);
	}
	kfifo_free(&sgThreadData.m_pkfifo);
	kfifo_free(&sgThreadData.m_pkfifo_out);
	return 0;
}

/*
 * this is to avoid coverity error: CID 17571:
 * Out-of-bounds access (OVERRUN_STATIC)Overrunning struct type
 * BRCM_AUDIO_Param_Close_t (and other structures) of size 16 bytes by passing
 * it as an argument to a function which indexes it at byte position 103.
 */
static int AUDIO_Ctrl_Trigger_GetParamsSize(BRCM_AUDIO_ACTION_en_t action_code)
{
	int size = 0;

	switch (action_code) {
	case ACTION_AUD_OpenRecord:
	case ACTION_AUD_OpenPlay:
		size = sizeof(BRCM_AUDIO_Param_Open_t);
		break;
	case ACTION_AUD_CloseRecord:
	case ACTION_AUD_ClosePlay:
		size = sizeof(BRCM_AUDIO_Param_Close_t);
		break;
	case ACTION_AUD_StartRecord:
	case ACTION_AUD_StartPlay:
		size = sizeof(BRCM_AUDIO_Param_Start_t);
		break;
	case ACTION_AUD_StopRecord:
	case ACTION_AUD_StopPlay:
		size = sizeof(BRCM_AUDIO_Param_Stop_t);
		break;
	case ACTION_AUD_PausePlay:
		size = sizeof(BRCM_AUDIO_Param_Pause_t);
		break;
	case ACTION_AUD_ResumePlay:
		size = sizeof(BRCM_AUDIO_Param_Resume_t);
		break;
	case ACTION_AUD_SetPrePareParameters:
		size = sizeof(BRCM_AUDIO_Param_Prepare_t);
		break;
	case ACTION_AUD_BufferReady:
		size = sizeof(BRCM_AUDIO_Param_BufferReady_t);
		break;
	case ACTION_AUD_AddChannel:
	case ACTION_AUD_RemoveChannel:
	case ACTION_AUD_SwitchSpkr:
		size = sizeof(BRCM_AUDIO_Param_Spkr_t);
		break;
	case ACTION_AUD_EnableTelephony:
	case ACTION_AUD_DisableTelephony:
	case ACTION_AUD_SetTelephonyMicSpkr:
		size = sizeof(BRCM_AUDIO_Param_Call_t);
		break;
	case ACTION_AUD_MutePlayback:
	case ACTION_AUD_MuteRecord:
	case ACTION_AUD_MuteTelephony:
		size = sizeof(BRCM_AUDIO_Param_Mute_t);
		break;
	case ACTION_AUD_EnableECNSTelephony:
	case ACTION_AUD_DisableECNSTelephony:
		size = sizeof(BRCM_AUDIO_Param_ECNS_t);
		break;
	case ACTION_AUD_EnableByPassVibra:
		size = sizeof(BRCM_AUDIO_Param_Vibra_t);
		break;
	case ACTION_AUD_DisableByPassVibra:
		size = 0;
		break;
	case ACTION_AUD_SetPlaybackVolume:
	case ACTION_AUD_SetRecordGain:
	case ACTION_AUD_SetTelephonySpkrVolume:
	case ACTION_AUD_UpdateUserVolSetting:
		size = sizeof(BRCM_AUDIO_Param_Volume_t);
		break;
	case ACTION_AUD_SetHWLoopback:
		size = sizeof(BRCM_AUDIO_Param_Loopback_t);
		break;
		/*
		 * case ACTION_AUD_SetAudioMode:
		 *      break;
		 */
	case ACTION_AUD_EnableFMPlay:
	case ACTION_AUD_DisableFMPlay:
	case ACTION_AUD_SetARM2SPInst:
		size = sizeof(BRCM_AUDIO_Param_FM_t);
		break;
	case ACTION_AUD_RateChange:
		size = sizeof(BRCM_AUDIO_Param_RateChange_t);
		break;
	case ACTION_AUD_SetAudioApp:
		size = sizeof(BRCM_AUDIO_Param_SetApp_t);
		break;
	case ACTION_AUD_AMPEnable:
		size = sizeof(BRCM_AUDIO_Param_AMPCTL_t);
		break;
	case ACTION_AUD_SetCallMode:
		size = sizeof(BRCM_AUDIO_Param_CallMode_t);
		break;
	case ACTION_AUD_AtCtl:
		size = sizeof(BRCM_AUDIO_Param_AtCtl_t);
		break;
	case ACTION_AUD_BTTest:
		size = sizeof(BRCM_AUDIO_Param_BT_Test_t);
		break;
	case ACTION_AUD_CfgIHF:
		size = sizeof(BRCM_AUDIO_Param_Cfg_IHF_t);
		break;
	case ACTION_AUD_CfgSSP:
		size = sizeof(BRCM_AUDIO_Param_Cfg_SSP_t);
		break;
	case ACTION_AUD_HwCtl:
		size = sizeof(BRCM_AUDIO_Param_HwCtl_t);
		break;
	default:
		break;
	}
	return size;
}

/*
Sets the user set audio app based on the App policy
*/

void AUDIO_Ctrl_SetUserAudioApp(AudioApp_t app)
{
	int state;
	AudioApp_t userapp;
	AudioMode_t mode;

	if (app != AUDIO_APP_MUSIC) {

		AUDCTRL_SetUserAudioApp(app);

		/*Get current state If we are already in state incall,record,FM
		we need to apply the new app prof here based on policy,For app
		prof set from user side with setparams like GVS,VoIP we may get
		the set app profile call after we start the record path.
		we are reloading the correct App here.
		*/

		state =  AUDIO_Policy_GetState();

		if (state == BRCM_STATE_INCALL || state == BRCM_STATE_FM ||
			state == BRCM_STATE_RECORD) {
			userapp = AUDIO_Policy_Get_Profile(app);
			aTrace(LOG_AUDIO_CNTLR, "AudioPolicy state=%d, App=%d\n"
					, state, userapp);
			if (userapp != AUDCTRL_GetAudioApp()) {
				/*Save user app,set audio mode methods below
				will query the current audioapp*/
				AUDCTRL_SaveAudioApp(userapp);
				mode = AUDCTRL_GetAudioMode();

				switch (userapp) {
				case AUDIO_APP_VT_CALL:
				case AUDIO_APP_VT_CALL_WB:
					AUDCTRL_SetAudioMode(mode, userapp);
				break;
				/*Assuming Both HQ and RECORDING is
				at 48khz for SS*/
				case AUDIO_APP_RECORDING_HQ:
				case AUDIO_APP_RECORDING:
				case AUDIO_APP_RECORDING_GVS:
					AUDCTRL_SetAudioMode_ForMusicRecord(
					mode,
					pathID[CTL_STREAM_PANEL_PCMIN-1]);
				break;
				case AUDIO_APP_VOIP:
				case AUDIO_APP_VOIP_INCOMM:
					AUDCTRL_SetAudioMode_ForMusicRecord(
					mode,
					pathID[CTL_STREAM_PANEL_PCMIN-1]);
					AUDCTRL_SetAudioMode_ForMusicPlayback(
					mode,
					pathID[CTL_STREAM_PANEL_PCMOUT1-1],
					FALSE);
				break;
				case AUDIO_APP_LOOPBACK:
					AUDCTRL_SetAudioMode(mode, userapp);
				break;
				/* For FM play case we alawys set FM mode first
				and then start FM,So we are always in FM app
				when we start.we will not end up here for
				present FM design
				*/
				case AUDIO_APP_FM:
					AUDCTRL_SetAudioMode_ForFM(mode,
						pathID[CTL_STREAM_PANEL_FM-1],
								FALSE);
				aWarn("WARNING: %s new app requested = %d",
							__func__, app);
				break;
				/*For Music,we will query the app policy and
				apply the correct app. Voice call app is not
				set from user space
				*/
				case AUDIO_APP_MUSIC:
				case AUDIO_APP_VOICE_CALL:
				case AUDIO_APP_VOICE_CALL_WB:
				case AUDIO_APP_RESERVED12:
				case AUDIO_APP_RESERVED13:
				case AUDIO_APP_RESERVED14:
				case AUDIO_APP_RESERVED15:
				/*To keep compiler happy*/
				aWarn("In %s new app requested=%d",
					__func__, app);
				break;
				default:
				/*We should not be here*/
				aError("!!Error in %s new app requested=%d",
				__func__, app);
				break;
				}
			}
		}

	}
}


/**
  * AUDIO_Ctrl_Trigger
  *      Client call this function to execute audio HAL functions.
  *      This function forward the message to worker thread to do actual work
  * @action_code: action request enum
  * @arg_param: argument for the action request
  * @callback: callback function pointer,
  *     see typedef void (*PFuncAudioCtrlCB)(int)
  * @block: bit 0 to indicate blocking call or not
  *     It is also used as private data for the call back function if callback
  *     is specified.
  *     The caller will get called as callback(block)
  *     When used as private data for the call back function, please pass
  *     a pointer which is 4 bytes alignment (block&0x03 ==0 ), because we
  *     we reserve bit 0 to indicate block mode.
  *
  * Return 0 for success, non-zero for error code
  *
  * This is message producer to sgThreadData.m_pkfifo
  */
Result_t AUDIO_Ctrl_Trigger(BRCM_AUDIO_ACTION_en_t action_code,
			    void *arg_param, void *callback, int block)
{
	TMsgAudioCtrl msgAudioCtrl;
	Result_t status = RESULT_OK;
	unsigned int len;
	OSStatus_t osStatus;
	int params_size;
	unsigned long to_jiff = msecs_to_jiffies(BLOCK_WAITTIME_MS);
	int fifo_avail;
	unsigned long t_flag;
	int is_atomic;
	int is_cb = 0;



	/** BEGIN: not support 48KHz recording during voice call */
	static int record48K_in_call_is_blocked;
	if (action_code == ACTION_AUD_StartRecord) {

		BRCM_AUDIO_Param_Start_t param_start;
		memcpy((void *)&param_start, arg_param,
			sizeof(BRCM_AUDIO_Param_Start_t));

		if (param_start.callMode == MODEM_CALL
			&& param_start.rate == AUDIO_SAMPLING_RATE_48000
			&& param_start.pdev_prop->c.source != AUDIO_SOURCE_I2S
			&& param_start.pdev_prop->c.drv_type
				== AUDIO_DRIVER_CAPT_HQ
			/*FM recording is supported*/
			&& telephonyIsEnabled == TRUE) {

			aError("StartRecord failed.	48KHz in call");
			record48K_in_call_is_blocked = 1;
			return RESULT_ERROR;
		}
	}

	if (action_code == ACTION_AUD_StopRecord) {

		BRCM_AUDIO_Param_Stop_t param_stop;
		memcpy((void *)&param_stop, arg_param,
			sizeof(BRCM_AUDIO_Param_Stop_t));

		if (param_stop.callMode == MODEM_CALL
			&& param_stop.pdev_prop->c.source != AUDIO_SOURCE_I2S
			&& param_stop.pdev_prop->c.drv_type
				== AUDIO_DRIVER_CAPT_HQ
			/*FM recording is supported*/
			&& telephonyIsEnabled == TRUE
			&& record48K_in_call_is_blocked == 1
			/* if start recording from digi mic in idle mode,
			then start voice call,
			when stop recording the path shall be disabled.
			*/
			) {

			record48K_in_call_is_blocked = 0;
			return RESULT_ERROR;
		}
	}
	/** EDN: not support 48KHz recording during voice call.*/

	/*When multi blocking triggers (such as OpenPlay and ClosePlay) arrive
	  at the same time, each would compete for action_complete semaphore,
	  which may be intended for other trigger. As a result, incorrect
	  parameters are returned.
	*/
	if (block & 1)
		mutex_lock(&mutexBlock);

	if (action_code == ACTION_AUD_DisableByPassVibra_CB) {
		action_code = ACTION_AUD_DisableByPassVibra;
		is_cb = 1;
	}
	params_size = AUDIO_Ctrl_Trigger_GetParamsSize(action_code);
	msgAudioCtrl.action_code = action_code;
	if (arg_param)
		memcpy(&msgAudioCtrl.param, arg_param, params_size);
	else
		memset(&msgAudioCtrl.param, 0, params_size);
	msgAudioCtrl.pCallBack = callback;
	msgAudioCtrl.block = block;

	is_atomic = 0;
	if (action_code == ACTION_AUD_StartPlay ||
	    action_code == ACTION_AUD_StopPlay ||
	    action_code == ACTION_AUD_PausePlay ||
	    action_code == ACTION_AUD_ResumePlay ||
	    action_code == ACTION_AUD_StartRecord ||
	    action_code == ACTION_AUD_StopRecord ||
	    action_code == ACTION_AUD_RateChange ||
	    action_code == ACTION_AUD_AddChannel ||
	    action_code ==  ACTION_AUD_RemoveChannel)

		is_atomic = 1;

/* Triggers come to audio KFIFO from many threads: HAL, hwdep,
   interrupt callback etc.
   Audio KFIFO overflow may happen during stress test, due to that
   either audio driver is stuck, or audio thread is blocked by other
   threads.
   - When KFIFO is half full, give warning.
   - When KFIFO is full:
    * For timer callback, ask to reshedule.
    * For other non-atomic triggers, wait for completion.
    * For atomic triggers, throw away. This is not ideal, but atomic
      triggers require min delay and does not allow sleep or waiting.
*/
AUDIO_Ctrl_Trigger_Wait:
	spin_lock_irqsave(&sgThreadData.m_lock, t_flag);
	fifo_avail = kfifo_avail(&sgThreadData.m_pkfifo);
	spin_unlock_irqrestore(&sgThreadData.m_lock, t_flag);

	if (fifo_avail < sizeof(TMsgAudioCtrl)) {
		aError("Audio KFIFO FULL avail %d, n_msg_in 0x%x, "
			"n_msg_out 0x%x, last_action %d, action %d\n",
			fifo_avail, n_msg_in, n_msg_out, last_action,
			action_code);

		if (is_atomic) {
			aError("ERROR Audio KFIFO FULL throw atomic "
				"action %d\n", action_code);
			return RESULT_ERROR;
		} else if (is_cb) {
			aError("Audio KFIFO FULL reschedule cb action %d\n",
				action_code);
			return RESULT_ERROR;
		} else {
			unsigned long ret;
			ret = wait_for_completion_interruptible_timeout(
				&complete_kfifo,
				msecs_to_jiffies(KFIFO_TIMEOUT_MS));
			if (!ret) {
				aError("ERROR Audio KFIFO timeout avail %d, "
					"n_msg_in 0x%x, n_msg_out 0x%x, "
					"last_action %d, action %d\n",
					fifo_avail, n_msg_in, n_msg_out,
					last_action, action_code);
				BUG();
				return RESULT_ERROR;
			}
		}
	} else if (fifo_avail < KFIFO_SIZE/2)
		aWarn("Audio KFIFO HIGH avail %d, n_msg_in 0x%x, "
			"n_msg_out 0x%x, last_action %d, action %d\n",
			fifo_avail, n_msg_in, n_msg_out, last_action,
			action_code);

	spin_lock_irqsave(&sgThreadData.m_lock, t_flag);
	fifo_avail = kfifo_avail(&sgThreadData.m_pkfifo);

	if (fifo_avail < sizeof(TMsgAudioCtrl)) {
		/*this means trigger from other thead has taken the spot*/
		spin_unlock_irqrestore(&sgThreadData.m_lock, t_flag);
		/*aError("Audio KFIFO FULL loop action %d, "
			"cb %d, atomic %d\n",
			action_code, is_cb, is_atomic);*/
		/*return RESULT_ERROR;*/
		goto AUDIO_Ctrl_Trigger_Wait;
	}

	len = kfifo_in(&sgThreadData.m_pkfifo,
		(unsigned char *)&msgAudioCtrl,
		sizeof(TMsgAudioCtrl));

	spin_unlock_irqrestore(&sgThreadData.m_lock, t_flag);

	aTrace(LOG_AUDIO_CNTLR, "AUDIO_Ctrl_Trigger action %d, avail %d, "
		"n_msg_in 0x%x, n_msg_out 0x%x, last_action %d\n",
		action_code, fifo_avail, n_msg_in, n_msg_out, last_action);

	if (len != sizeof(TMsgAudioCtrl))
		aError
		    ("Error AUDIO_Ctrl_Trigger len=%d expected %d\n", len,
		     sizeof(TMsgAudioCtrl));

	queue_work(sgThreadData.pWorkqueue_AudioControl, &sgThreadData.mwork);
	if (block & 1) {
		osStatus =
		    OSSEMAPHORE_Obtain(sgThreadData.action_complete, to_jiff);
		if (osStatus != OSSTATUS_SUCCESS) {
			aWarn("AUDIO_Ctrl_Trigger Timeout=%d\r\n",
					osStatus);
			status = RESULT_ERROR;
			return status;
		}

		while (1) {
			/* wait for output from output fifo */
			len =
			    kfifo_out_locked(&sgThreadData.m_pkfifo_out,
					     (unsigned char *)&msgAudioCtrl,
					     sizeof(TMsgAudioCtrl),
					     &sgThreadData.m_lock_out);
			/*
			 * Commenting debug prints to eliminate compilation
			 * errors for kfifo member access
			 */
			/*
			 *   if( (len != sizeof(TMsgAudioCtrl)) && (len!=0) )
			 *      DEBUG("Error AUDIO_Ctrl_Trigger
			 *      len=%d expected %d in=%d, out=%d\n", len,
			 *      sizeof(TMsgAudioCtrl),
			 *      sgThreadData.m_pkfifo_out.in,
			 *      sgThreadData.m_pkfifo_out.out);
			 */
			if (len == 0)	/* FIFO empty sleep */
				break;
			if (arg_param)
				memcpy(arg_param, &msgAudioCtrl.param,
				       params_size);
		}
		mutex_unlock(&mutexBlock);
	}

	return status;
}

/* This is message consumer of sgThreadData.m_pkfifo */
static void AUDIO_Ctrl_Process(BRCM_AUDIO_ACTION_en_t action_code,
			void *arg_param, void *callback, int block)
{
	TMsgAudioCtrl msgAudioCtrl;
	unsigned int len;
	int i;
	unsigned int path;
	int app_profile;

	aTrace(LOG_AUDIO_CNTLR,
		"AUDIO_Ctrl_Process action_code=%d %s\n", action_code,
		&action_names[action_code][0]);

	switch (action_code) {
	case ACTION_AUD_OpenPlay:
		{
			BRCM_AUDIO_Param_Open_t *param_open =
			    (BRCM_AUDIO_Param_Open_t *) arg_param;

			param_open->drv_handle =
			    AUDIO_DRIVER_Open(param_open->pdev_prop->p[0].
					      drv_type);
			if (param_open->drv_handle == NULL)
				aTrace(LOG_AUDIO_CNTLR,
						"\n %lx:AUDIO_Ctrl_Process-"
						"AUDIO_DRIVER_Open  failed\n",
						jiffies);
		}
		break;

	case ACTION_AUD_ClosePlay:
		{
			BRCM_AUDIO_Param_Close_t *param_close =
			    (BRCM_AUDIO_Param_Close_t *) arg_param;

			AUDIO_DRIVER_Close(param_close->drv_handle);
		}
		break;

	case ACTION_AUD_StartPlay:
		{
		BRCM_AUDIO_Param_Start_t *param_start =
		    (BRCM_AUDIO_Param_Start_t *) arg_param;
		AudioMode_t  newmode = GetAudioModeBySink(
				param_start->pdev_prop->p[0].sink);

		CAPH_ASSERT(param_start->stream >=
			    (CTL_STREAM_PANEL_FIRST - 1)
			    && param_start->stream <
			    (CTL_STREAM_PANEL_LAST - 1));

		sgpCaph_chip->streamCtl[param_start->stream].playback_stop = 0;

		sgpCaph_chip->streamCtl[param_start->stream].playback_prev_time
			= 0;

		newmode = AUDIO_Policy_Get_Mode( \
			param_start->pdev_prop->p[0].sink);
		app_profile = AUDIO_Policy_Get_Profile(AUDIO_APP_MUSIC);
		AUDCTRL_SaveAudioMode((AudioMode_t)newmode);
		AUDCTRL_SaveAudioApp((AudioApp_t)app_profile);

		if (param_start->pdev_prop->p[0].drv_type ==
		    AUDIO_DRIVER_PLAY_AUDIO) {

			/* Enable the playback path */
			AUDCTRL_EnablePlay(
				param_start->pdev_prop->p[0].source,
				param_start->pdev_prop->p[0].sink,
				param_start->channels,
				param_start->rate, &path);

			/*if pathID is out of bound*/
			if (param_start->stream >=
					 CAPH_MAX_PCM_STREAMS)
				break;
			pathID[param_start->stream] = path;

			/*
			 * AUDCTRL_EnablePlay enables HW path, reads
			 * SYSPARM and sets HW gains as defined in
			 * SYSPARM.
			 */

			AUDIO_DRIVER_Ctrl(param_start->drv_handle,
					  AUDIO_DRIVER_START,
					  &param_start->pdev_prop->p[0].
					  sink);
		} else if (param_start->pdev_prop->p[0].drv_type ==
			   AUDIO_DRIVER_PLAY_VOICE) {
			AUDIO_DRIVER_Ctrl(param_start->drv_handle,
					  AUDIO_DRIVER_START, NULL);
		}
#if defined(CONFIG_MFD_BCM59039) | defined(CONFIG_MFD_BCM59042)
			AUDCTRL_TempGainCompInit(&sgTempGainComp);
			schedule_delayed_work(&sgTempGainComp.temp_gain_comp,
				0);
#endif

		}
		break;
	case ACTION_AUD_StopPlay:
		{
			BRCM_AUDIO_Param_Stop_t *param_stop =
			    (BRCM_AUDIO_Param_Stop_t *) arg_param;

			CAPH_ASSERT(param_stop->stream >=
				    (CTL_STREAM_PANEL_FIRST - 1)
				    && param_stop->stream <
				    (CTL_STREAM_PANEL_LAST - 1));

			sgpCaph_chip->
				streamCtl[param_stop->stream].playback_stop = 1;

			AUDIO_DRIVER_Ctrl(param_stop->drv_handle,
					  AUDIO_DRIVER_STOP, NULL);

			/* Remove secondary playback path if it's in use */
			for (i = (MAX_PLAYBACK_DEV - 1); i > 0; i--) {
				if (param_stop->pdev_prop->p[i].sink !=
				    AUDIO_SINK_UNDEFINED) {

					/*if pathID is out of bound*/
					if (param_stop->stream >=
						 CAPH_MAX_PCM_STREAMS)
						break;
#ifdef THIRD_PARTY_PMU
					AUDCTRL_RemovePlaySpk_InPMU(param_stop->
							      pdev_prop->p[0]
							      .source,
							      param_stop->
							      pdev_prop->p[i].
							      sink,
							      pathID
							      [param_stop->
							       stream]);
#else
					AUDCTRL_RemovePlaySpk(param_stop->
							      pdev_prop->p[0]
							      .source,
							      param_stop->
							      pdev_prop->p[i].
							      sink,
							      pathID
							      [param_stop->
							       stream]);
#endif
				}
			}

			if (param_stop->pdev_prop->p[0].drv_type ==
			    AUDIO_DRIVER_PLAY_AUDIO) {
				/* disable the playback path */
				AUDCTRL_DisablePlay(param_stop->pdev_prop->p[0].
					    source,
					    param_stop->pdev_prop->p[0].
					    sink,
					    pathID[param_stop->stream]);

				pathID[param_stop->stream] = 0;
			}
			aTrace(LOG_AUDIO_CNTLR,
					"AUDIO_Ctrl_Process Stop Playback"
					" completed\n");
		}
		break;
	case ACTION_AUD_BufferReady:
		{
			BRCM_AUDIO_Param_BufferReady_t *param_bufferready =
			    (BRCM_AUDIO_Param_BufferReady_t *) arg_param;

			/*aTrace(LOG_AUDIO_CNTLR,
			       "\n %lx:AUDIO_Ctrl_Process-"
			       "ACTION_AUD_BufferReady. stream=%d\n",
			       jiffies, param_bufferready->stream);*/
			CAPH_ASSERT(param_bufferready->stream >=
				    (CTL_STREAM_PANEL_FIRST - 1)
				    && param_bufferready->stream <
				    (CTL_STREAM_PANEL_LAST - 1));

			AUDIO_DRIVER_Ctrl(param_bufferready->drv_handle,
					  AUDIO_DRIVER_BUFFER_READY, NULL);

			/*aTrace(LOG_AUDIO_CNTLR,
			"AUDIO_Ctrl_Process ACTION_AUD_BufferReady"
			" completed. stream=%d\n", param_bufferready->stream);*/
		}
		break;
	case ACTION_AUD_PausePlay:
		{
			BRCM_AUDIO_Param_Pause_t *param_pause =
			    (BRCM_AUDIO_Param_Pause_t *) arg_param;
			CAPH_ASSERT(param_pause->stream >=
				    (CTL_STREAM_PANEL_FIRST - 1)
				    && param_pause->stream <
				    (CTL_STREAM_PANEL_LAST - 1));

			if (param_pause->pdev_prop->p[0].drv_type ==
			    AUDIO_DRIVER_PLAY_AUDIO) {
				/* disable the playback path */
				/* coverity[overrun-local] */
				AUDCTRL_DisablePlay(param_pause->pdev_prop->p[0]
						    .source,
						    param_pause->pdev_prop->
						    p[0].sink,
						    pathID[param_pause->
							   stream]);

				pathID[param_pause->stream] = 0;
			}
			AUDIO_DRIVER_Ctrl(param_pause->drv_handle,
					  AUDIO_DRIVER_PAUSE, NULL);
		}
		break;

	case ACTION_AUD_ResumePlay:
		{
		BRCM_AUDIO_Param_Resume_t *param_resume =
		    (BRCM_AUDIO_Param_Resume_t *) arg_param;

		CAPH_ASSERT(param_resume->stream >=
			    (CTL_STREAM_PANEL_FIRST - 1)
			    && param_resume->stream <
			    (CTL_STREAM_PANEL_LAST - 1));

		AUDIO_DRIVER_Ctrl(param_resume->drv_handle,
				  AUDIO_DRIVER_RESUME, NULL);

		if (param_resume->pdev_prop->p[0].drv_type ==
		    AUDIO_DRIVER_PLAY_AUDIO) {
			/*  Enable the playback the path */
			/* coverity[overrun-local] */
			AUDCTRL_EnablePlay(param_resume->pdev_prop->p[0]
					   .source,
					   param_resume->pdev_prop->
					   p[0].sink,
					   param_resume->channels,
					   param_resume->rate, &path);
			pathID[param_resume->stream] = path;
		}

		}
		break;
	case ACTION_AUD_StartRecord:
		{
			BRCM_AUDIO_Param_Start_t *param_start =
			    (BRCM_AUDIO_Param_Start_t *) arg_param;

			AudioApp_t app_prof = AUDIO_APP_RECORDING;
			AudioMode_t  new_mode = AUDIO_MODE_SPEAKERPHONE;
			AudioMode_t cur_mode;

			cur_mode = AUDCTRL_GetAudioMode();

			/*Audio app for Voip,GVS will be set from user side*/
			app_prof = AUDCTRL_GetUserAudioApp();

			/*use current mode based on mode earpiece or speaker*/
			if (app_prof == AUDIO_APP_VOIP ||
				app_prof == AUDIO_APP_VOIP_INCOMM)
				new_mode = cur_mode;

			/*use mode headset for aux mic*/
			if (param_start->pdev_prop->c.source ==
				AUDIO_SOURCE_ANALOG_AUX)
				new_mode = AUDIO_MODE_HEADSET;

			/*For FM Rec request app policy with FM APP*/
			if (param_start->pdev_prop->c.source ==
				AUDIO_SOURCE_I2S) {
				app_prof = AUDIO_APP_FM;
				/*keep the mode*/
				new_mode = cur_mode;
			}
			app_profile = AUDIO_Policy_Get_Profile(app_prof);
			new_mode = AUDIO_Policy_Get_Mode(new_mode);

			AUDCTRL_SaveAudioMode(new_mode);
			AUDCTRL_SaveAudioApp(app_profile);

			CAPH_ASSERT(param_start->stream >=
				    (CTL_STREAM_PANEL_FIRST - 1)
				    && param_start->stream <
				    (CTL_STREAM_PANEL_LAST - 1));

			/* allow FM recording in call mode */
			/*
			 * AUDCTRL_EnableRecord enables HW path, reads SYSPARM
			 * and sets HW gains as defined in SYSPARM.
			 */
			AUDCTRL_EnableRecord(param_start->pdev_prop->c.
					     source,
					     param_start->pdev_prop->c.
					     sink,
					     param_start->channels,
					     param_start->rate, &path);

			/* coverity[overrun-local] */
			pathID[param_start->stream] = path;

			if (param_start->pdev_prop->c.drv_type ==
			    AUDIO_DRIVER_CAPT_HQ) {
				AUDIO_DRIVER_Ctrl(param_start->drv_handle,
						  AUDIO_DRIVER_START,
						  &param_start->pdev_prop->c.
						  source);
			} else {
				voice_rec_t voiceRecStr;
				memset(&voiceRecStr, 0, sizeof(voice_rec_t));
				voiceRecStr.recordMode = param_start->mixMode;
				voiceRecStr.callMode = param_start->callMode;
				AUDIO_DRIVER_Ctrl(param_start->drv_handle,
						  AUDIO_DRIVER_START,
						  &voiceRecStr);
			}

			AUDIO_Policy_SetState(BRCM_STATE_RECORD);
		}
		break;
	case ACTION_AUD_StopRecord:
		{
			BRCM_AUDIO_Param_Stop_t *param_stop =
			    (BRCM_AUDIO_Param_Stop_t *) arg_param;

			CAPH_ASSERT(param_stop->stream >=
				    (CTL_STREAM_PANEL_FIRST - 1)
				    && param_stop->stream <
				    (CTL_STREAM_PANEL_LAST - 1));

			AUDIO_DRIVER_Ctrl(param_stop->drv_handle,
					  AUDIO_DRIVER_STOP, NULL);

			/* coverity[overrun-local] */
			AUDCTRL_DisableRecord(param_stop->pdev_prop->c.source,
					param_stop->pdev_prop->c.sink,
					pathID[param_stop->stream]);
			pathID[param_stop->stream] = 0;
			AUDIO_Policy_RestoreState();

			/* If we do start play before we stop rec,we do not
			allow the app profile change to MUSIC,we should allow
			App profile change to MUSIC for the playback path
			once we stop record */

			app_profile = AUDIO_Policy_Get_Profile(AUDIO_APP_MUSIC);
			if (app_profile == AUDIO_APP_MUSIC) {
				AUDCTRL_SaveAudioApp(app_profile);
				AUDCTRL_SetAudioMode_ForMusicPlayback(
					AUDCTRL_GetAudioMode(),
					pathID[CTL_STREAM_PANEL_PCMOUT1-1],
					FALSE);
			}
		}
		break;
	case ACTION_AUD_OpenRecord:
		{
			BRCM_AUDIO_Param_Open_t *param_open =
			    (BRCM_AUDIO_Param_Open_t *) arg_param;

			param_open->drv_handle =
			    AUDIO_DRIVER_Open(param_open->pdev_prop->c.
					      drv_type);

			aTrace(LOG_AUDIO_CNTLR,
					"param_open->drv_handle -  0x%lx\n",
					(UInt32) param_open->drv_handle);

		}
		break;
	case ACTION_AUD_CloseRecord:
		{
			BRCM_AUDIO_Param_Close_t *param_close =
			    (BRCM_AUDIO_Param_Close_t *) arg_param;

			AUDIO_DRIVER_Close(param_close->drv_handle);
		}
		break;
	case ACTION_AUD_AddChannel:
		{
			BRCM_AUDIO_Param_Spkr_t *parm_spkr =
			    (BRCM_AUDIO_Param_Spkr_t *) arg_param;
			CAPH_ASSERT(parm_spkr->stream >=
				    (CTL_STREAM_PANEL_FIRST - 1)
				    && parm_spkr->stream <
				    (CTL_STREAM_PANEL_LAST - 1));

			/*Query for MUSIC profile,in case we are not in
			state normal like FM,Voice,Recording policy will
			decide,else we will set MUSIC profile*/

			app_profile = AUDIO_Policy_Get_Profile(AUDIO_APP_MUSIC);
			AUDCTRL_SaveAudioApp(app_profile);
#ifdef THIRD_PARTY_PMU
			/* coverity[overrun-local] */
			AUDCTRL_AddPlaySpk_InPMU(parm_spkr->src,
				parm_spkr->sink, pathID[parm_spkr->stream]);
#else
			/* coverity[overrun-local] */
			AUDCTRL_AddPlaySpk(parm_spkr->src, parm_spkr->sink,
					   pathID[parm_spkr->stream]);
#endif
		}
		break;
	case ACTION_AUD_RemoveChannel:
		{
			BRCM_AUDIO_Param_Spkr_t *parm_spkr =
			    (BRCM_AUDIO_Param_Spkr_t *) arg_param;
			CAPH_ASSERT(parm_spkr->stream >=
				    (CTL_STREAM_PANEL_FIRST - 1)
				    && parm_spkr->stream <
				    (CTL_STREAM_PANEL_LAST - 1));
#ifdef THIRD_PARTY_PMU

			/* coverity[overrun-local] */
			AUDCTRL_RemovePlaySpk_InPMU(parm_spkr->src,
				parm_spkr->sink, pathID[parm_spkr->stream]);
#else
			/* coverity[overrun-local] */
			AUDCTRL_RemovePlaySpk(parm_spkr->src, parm_spkr->sink,
					      pathID[parm_spkr->stream]);
#endif
		}
		break;

	case ACTION_AUD_EnableTelephony:
		{
			BRCM_AUDIO_Param_Call_t *parm_call =
			    (BRCM_AUDIO_Param_Call_t *) arg_param;
			AudioMode_t  audio_mode = AUDIO_MODE_SPEAKERPHONE;
			audio_mode = GetAudioModeBySink(parm_call->new_spkr);
			app_profile = AUDIO_APP_VOICE_CALL;

			telephonyIsEnabled = TRUE; /*DSP is using mic path */

			/*Consider VT-NB case*/
			if (AUDIO_APP_VT_CALL == AUDCTRL_GetUserAudioApp())
				app_profile = AUDIO_APP_VT_CALL;

			if (AUDCTRL_Telephony_HW_16K(audio_mode)) {
				if (app_profile == AUDIO_APP_VT_CALL)
					app_profile = AUDIO_APP_VT_CALL_WB;
				else
					app_profile = AUDIO_APP_VOICE_CALL_WB;
			}
			app_profile = AUDIO_Policy_Get_Profile(app_profile);
			AUDCTRL_SaveAudioApp(app_profile);
			AUDCTRL_SaveAudioMode(audio_mode);

			AUDCTRL_EnableTelephony(parm_call->new_mic,
						parm_call->new_spkr);

			AUDIO_Policy_SetState(BRCM_STATE_INCALL);
#if defined(CONFIG_MFD_BCM59039) | defined(CONFIG_MFD_BCM59042)
			AUDCTRL_TempGainCompInit(&sgTempGainComp);
			schedule_delayed_work(&sgTempGainComp.temp_gain_comp,
				0);
#endif
		}
		break;

	case ACTION_AUD_DisableTelephony:
		AUDCTRL_DisableTelephony();
		AUDIO_Policy_RestoreState();
		/*VT app profile is set from User space
		when VT call disable set user-app prof to default MUSIC */
		if (AUDIO_APP_VT_CALL == AUDCTRL_GetUserAudioApp())
			AUDCTRL_SetUserAudioApp(AUDIO_APP_MUSIC);
		telephonyIsEnabled = FALSE;
		break;

	case ACTION_AUD_SetTelephonyMicSpkr:
		{
			BRCM_AUDIO_Param_Call_t *parm_call =
			    (BRCM_AUDIO_Param_Call_t *) arg_param;
			AudioMode_t  audio_mode = GetAudioModeBySink(
							parm_call->new_spkr);
			AUDCTRL_SaveAudioMode(audio_mode);

			AUDCTRL_SetTelephonyMicSpkr(parm_call->new_mic,
						    parm_call->new_spkr);
		}
		break;

	case ACTION_AUD_MutePlayback:
		{
		BRCM_AUDIO_Param_Mute_t *parm_mute =
		    (BRCM_AUDIO_Param_Mute_t *) arg_param;
		CAPH_ASSERT(parm_mute->stream >=
			    (CTL_STREAM_PANEL_FIRST - 1)
			    && parm_mute->stream <
			    (CTL_STREAM_PANEL_LAST - 1));
		/*
		 * currently driver doesnt handle Mute for left/right
		 * channels
		 */

		if (parm_mute->stream >= CAPH_MAX_PCM_STREAMS)
			break;
		AUDCTRL_SetPlayMute(parm_mute->source,
				    parm_mute->sink,
				    parm_mute->mute1,
				    pathID[parm_mute->stream]);
		}
		break;
	case ACTION_AUD_MuteRecord:
		{
			BRCM_AUDIO_Param_Mute_t *parm_mute =
			    (BRCM_AUDIO_Param_Mute_t *) arg_param;
			CAPH_ASSERT(parm_mute->stream >=
				    (CTL_STREAM_PANEL_FIRST - 1)
				    && parm_mute->stream <
				    (CTL_STREAM_PANEL_LAST - 1));
			/* coverity[overrun-local] */
			AUDCTRL_SetRecordMute(parm_mute->source,
					      parm_mute->mute1,
					      pathID[parm_mute->stream]);
		}
		break;
	case ACTION_AUD_EnableByPassVibra:
		{
			BRCM_AUDIO_Param_Vibra_t *parm_vibra =
				(BRCM_AUDIO_Param_Vibra_t *)arg_param;
#ifdef USE_HR_TIMER
			int isHRTimActive = 0;
#endif

			aTrace(LOG_AUDIO_CNTLR,
					"ACTION_AUD_EnableVibra"
					"and SetVibraStrength\n");

			AUDCTRL_EnableBypassVibra(parm_vibra->strength,
				parm_vibra->direction);

#ifdef USE_HR_TIMER
			isHRTimActive = hrtimer_active(&hr_timer);

			if (isHRTimActive) {
				aTrace(LOG_AUDIO_CNTLR,
					"Hrtimer cancel is going to be called\n");

				hrtimer_cancel(&hr_timer);
			}

			if (parm_vibra->duration != 0) {
				ktime = ktime_set(parm_vibra->duration/1000,
				(parm_vibra->duration%1000)*1000000);

				hrtimer_start(&hr_timer, ktime,
					HRTIMER_MODE_REL);
			}
#else
			if (gpVibratorTimer) {
				del_timer_sync(gpVibratorTimer);
				gpVibratorTimer = NULL;
			}
			if (parm_vibra->duration != 0) {
				gpVibratorTimer = &gTimerVib;
				init_timer(gpVibratorTimer);
				gpVibratorTimer->function =
					TimerCbStopVibrator;
				gpVibratorTimer->data = 0;
				gpVibratorTimer->expires = jiffies +
					msecs_to_jiffies(parm_vibra->duration);
				add_timer(gpVibratorTimer);
			}
#endif
		}
		break;
	case ACTION_AUD_DisableByPassVibra:
		{
#ifdef USE_HR_TIMER
			int isHRTimActive = 0;
			isHRTimActive = hrtimer_active(&hr_timer);
			if (isHRTimActive) {
				aTrace(LOG_AUDIO_CNTLR,
					"Hrtimer cancel is going to be called"
					"from Disable Vibra\n");

				hrtimer_cancel(&hr_timer);
			}
#else
			if (gpVibratorTimer) {
				del_timer_sync(gpVibratorTimer);
				gpVibratorTimer = NULL;
			}
#endif
			/* stop it */
			aTrace(LOG_AUDIO_CNTLR,
				"ACTION_AUD_DisableByPassVibra\n");

			AUDCTRL_DisableBypassVibra();
		}
		break;
	case ACTION_AUD_SetPlaybackVolume:
		{
			BRCM_AUDIO_Param_Volume_t *parm_vol =
			    (BRCM_AUDIO_Param_Volume_t *) arg_param;
			CAPH_ASSERT(parm_vol->stream >=
				    (CTL_STREAM_PANEL_FIRST - 1)
				    && parm_vol->stream <
				    (CTL_STREAM_PANEL_LAST - 1));
			/* coverity[overrun-local] */
			AUDCTRL_SetPlayVolume(parm_vol->source, parm_vol->sink,
					      parm_vol->gain_format,
					      parm_vol->volume1,
					      parm_vol->volume2,
					      pathID[parm_vol->stream]);
		}
		break;
	case ACTION_AUD_SetRecordGain:
		{
			BRCM_AUDIO_Param_Volume_t *parm_vol =
			    (BRCM_AUDIO_Param_Volume_t *) arg_param;
			CAPH_ASSERT(parm_vol->stream >=
				    (CTL_STREAM_PANEL_FIRST - 1)
				    && parm_vol->stream <
				    (CTL_STREAM_PANEL_LAST - 1));
			/* coverity[overrun-local] */
			AUDCTRL_SetRecordGain(parm_vol->source,
					      AUDIO_GAIN_FORMAT_mB,
					      parm_vol->volume1,
					      parm_vol->volume2,
					      pathID[parm_vol->stream]);
		}
		break;

	case ACTION_AUD_SetTelephonySpkrVolume:
		{
			BRCM_AUDIO_Param_Volume_t *parm_vol =
			    (BRCM_AUDIO_Param_Volume_t *) arg_param;
			AUDCTRL_SetTelephonySpkrVolume(parm_vol->sink,
						       parm_vol->volume1,
						       parm_vol->gain_format);
		}
		break;

	case ACTION_AUD_SwitchSpkr:
		{
			BRCM_AUDIO_Param_Spkr_t *parm_spkr =
			    (BRCM_AUDIO_Param_Spkr_t *) arg_param;
			AudioMode_t  audio_mode;
			CAPH_ASSERT(parm_spkr->stream >=
				    (CTL_STREAM_PANEL_FIRST - 1)
				    && parm_spkr->stream <
				    (CTL_STREAM_PANEL_LAST - 1));
			audio_mode = GetAudioModeBySink(parm_spkr->sink);
			audio_mode = AUDIO_Policy_Get_Mode(audio_mode);
			AUDCTRL_SaveAudioMode(audio_mode);

			/* coverity[overrun-local] */
			AUDCTRL_SwitchPlaySpk(parm_spkr->src, parm_spkr->sink,
					      pathID[parm_spkr->stream]);
		}
		break;

	case ACTION_AUD_SetAudioMode:
		{
			BRCM_AUDIO_Param_Call_t *parm_call =
			    (BRCM_AUDIO_Param_Call_t *) arg_param;
			AudioMode_t tempMode =
			    (AudioMode_t) parm_call->new_spkr;
			AUDCTRL_SetAudioMode(tempMode, AUDCTRL_GetAudioApp());
		}
		break;

	case ACTION_AUD_SetHWLoopback:
		{
			BRCM_AUDIO_Param_Loopback_t *parm_loop =
			    (BRCM_AUDIO_Param_Loopback_t *) arg_param;
			AUDCTRL_SetAudioLoopback(parm_loop->parm,
						 (AUDIO_SOURCE_Enum_t)
						 parm_loop->mic,
						 (AUDIO_SINK_Enum_t) parm_loop->
						 spkr, (int)parm_loop->mode);
		}
		break;
	case ACTION_AUD_EnableFMPlay:
		{
			BRCM_AUDIO_Param_FM_t *parm_FM =
			    (BRCM_AUDIO_Param_FM_t *) arg_param;
			AudioMode_t fm_mode;
			CAPH_ASSERT(parm_FM->stream >=
				    (CTL_STREAM_PANEL_FIRST - 1)
				    && parm_FM->stream <
				    (CTL_STREAM_PANEL_LAST - 1));
			/*Query FM mode from Policy*/
			app_profile = AUDIO_Policy_Get_Profile(AUDIO_APP_FM);
			AUDCTRL_SaveAudioApp(app_profile);
			fm_mode = GetAudioModeBySink(parm_FM->sink);
			AUDCTRL_SaveAudioMode(fm_mode);

			AUDCTRL_EnablePlay(parm_FM->source,
					   parm_FM->sink,
					   AUDIO_CHANNEL_STEREO,
					   AUDIO_SAMPLING_RATE_48000, &path);

			/* coverity[overrun-local] */
			pathID[parm_FM->stream] = path;
			AUDIO_Policy_SetState(BRCM_STATE_FM);
#if defined(CONFIG_MFD_BCM59039) | defined(CONFIG_MFD_BCM59042)
			AUDCTRL_TempGainCompInit(&sgTempGainComp);
			schedule_delayed_work(&sgTempGainComp.temp_gain_comp,
				0);
#endif
		}
		break;
	case ACTION_AUD_DisableFMPlay:
		{
			BRCM_AUDIO_Param_FM_t *parm_FM =
			    (BRCM_AUDIO_Param_FM_t *) arg_param;
			CAPH_ASSERT(parm_FM->stream >=
				    (CTL_STREAM_PANEL_FIRST - 1)
				    && parm_FM->stream <
				    (CTL_STREAM_PANEL_LAST - 1));
			/* coverity[overrun-local] */
			AUDCTRL_DisablePlay(parm_FM->source, parm_FM->sink,
					    pathID[parm_FM->stream]);

			AUDIO_Policy_RestoreState();
			/*if we are playing Music+FM play and FM Stop,
			or music start happend first and then FM stop
			restore app prof to Music*/
			if (BRCM_STATE_NORMAL == AUDIO_Policy_GetState()) {
				AudioApp_t app_prof;
				app_prof = AUDIO_Policy_Get_Profile(
						AUDIO_APP_MUSIC);
				AUDCTRL_SaveAudioApp(app_prof);
				AUDCTRL_SetAudioMode_ForMusicPlayback(
				AUDCTRL_GetAudioMode(), pathID[parm_FM->stream],
								FALSE);
			}
			pathID[parm_FM->stream] = 0;
		}
		break;
	case ACTION_AUD_SetARM2SPInst:
		{
			BRCM_AUDIO_Param_FM_t *parm_FM =
			    (BRCM_AUDIO_Param_FM_t *) arg_param;
			AUDCTRL_SetArm2spParam(parm_FM->fm_mix,
					       TRUE);
		}
		break;
	case ACTION_AUD_SetPrePareParameters:
		{
			BRCM_AUDIO_Param_Prepare_t *parm_prepare =
			    (BRCM_AUDIO_Param_Prepare_t *) arg_param;
			/* set the callback */
			AUDIO_DRIVER_Ctrl(parm_prepare->drv_handle,
					  AUDIO_DRIVER_SET_CB,
					  (void *)&parm_prepare->cbParams);
			/* set the interrupt period */
			AUDIO_DRIVER_Ctrl(parm_prepare->drv_handle,
					  AUDIO_DRIVER_SET_INT_PERIOD,
					  (void *)&parm_prepare->period_bytes);
			/* set the buffer params */
			AUDIO_DRIVER_Ctrl(parm_prepare->drv_handle,
					  AUDIO_DRIVER_SET_BUF_PARAMS,
					  (void *)&parm_prepare->buf_param);
			/* Configure stream params */
			AUDIO_DRIVER_Ctrl(parm_prepare->drv_handle,
					  AUDIO_DRIVER_CONFIG,
					  (void *)&parm_prepare->drv_config);
		}
		break;

	case ACTION_AUD_MuteTelephony:
		{
			BRCM_AUDIO_Param_Mute_t *parm_mute =
			    (BRCM_AUDIO_Param_Mute_t *) arg_param;
			AUDCTRL_SetTelephonyMicMute(parm_mute->source,
						    parm_mute->mute1);
		}
		break;

	case ACTION_AUD_DisableECNSTelephony:
		aTrace(LOG_AUDIO_CNTLR, "Telephony : Turning Off EC and NS\n");
#ifdef CONFIG_AUDIO_FEATURE_SET_DISABLE_ECNS

		/* when turning off EC and NS, using
		  * AUDIO_MODE_HANDSFREE as customer's request
		  */
		AUDCTRL_SetAudioMode(AUDIO_MODE_HANDSFREE,
		AUDCTRL_GetAudioApp());
#endif
		AUDCTRL_EC(FALSE, 0);
		AUDCTRL_NS(FALSE);
		break;
	case ACTION_AUD_EnableECNSTelephony:
		aTrace(LOG_AUDIO_CNTLR, "Telephony : Turning On EC and NS\n");
		AUDCTRL_EC(TRUE, 0);
		AUDCTRL_NS(TRUE);
		break;
	case ACTION_AUD_RateChange:
		{
			BRCM_AUDIO_Param_RateChange_t *param_rate_change =
			    (BRCM_AUDIO_Param_RateChange_t *) arg_param;

			/* 0x0A as per 3GPP 26.103 Sec 6.3 indicates AMR WB
			 * AUDIO_ID_CALL16k
			 * 0x06 indicates AMR NB
			 */

			/*Consider VT-NB/WB case*/
			if (AUDCTRL_InVoiceCall()) {
				if (AUDCTRL_IsBTMWB()) {
					if (AUDIO_APP_VT_CALL ==
						AUDCTRL_GetUserAudioApp())
						app_profile =
							AUDIO_APP_VT_CALL_WB;
					else
						app_profile =
							AUDIO_APP_VOICE_CALL_WB;
				}
				else if (param_rate_change->codecID == 0x0A) {
					if (AUDIO_APP_VT_CALL ==
						AUDCTRL_GetUserAudioApp())
						app_profile =
							AUDIO_APP_VT_CALL_WB;
					else
						app_profile =
							AUDIO_APP_VOICE_CALL_WB;
				} else if (param_rate_change->codecID == 0x06) {
					if (AUDIO_APP_VT_CALL ==
						AUDCTRL_GetUserAudioApp())
						app_profile =
							AUDIO_APP_VT_CALL;
					else
						app_profile =
							AUDIO_APP_VOICE_CALL;
				} else {
					aError("Invalid Telephony CodecID %d\n",
						param_rate_change->codecID);
						break;
				}
			app_profile = AUDIO_Policy_Get_Profile(app_profile);
			AUDCTRL_SaveAudioMode(AUDCTRL_GetAudioMode());
			AUDCTRL_SaveAudioApp(app_profile);
			}

			AUDCTRL_Telephony_RequestRateChange(param_rate_change->
							    codecID);
		}
		break;

	case ACTION_AUD_SetAudioApp:
		{
			BRCM_AUDIO_Param_SetApp_t *parm_setapp =
				(BRCM_AUDIO_Param_SetApp_t *)arg_param;
			AUDIO_Ctrl_SetUserAudioApp(parm_setapp->aud_app);
		}
		break;
	case ACTION_AUD_AMPEnable:
		{
			BRCM_AUDIO_Param_AMPCTL_t *parm_setamp =
				(BRCM_AUDIO_Param_AMPCTL_t *)arg_param;
			AUDCTRL_EnableAmp(parm_setamp->amp_status);


		}
		break;
	case ACTION_AUD_SetCallMode:
		{
			BRCM_AUDIO_Param_CallMode_t *parm_callmode =
				(BRCM_AUDIO_Param_CallMode_t *)arg_param;
			AUDCTRL_SetCallMode(parm_callmode->callMode);
		}
		break;
	case ACTION_AUD_ConnectDL: /* PTT call */
		AUDCTRL_ConnectDL();
		break;
	case ACTION_AUD_UpdateUserVolSetting:
		{
			BRCM_AUDIO_Param_Volume_t *parm_vol =
				(BRCM_AUDIO_Param_Volume_t *) arg_param;
			AUDCTRL_UpdateUserVolSetting(
				parm_vol->sink,
				parm_vol->volume1,
				parm_vol->volume2,
				parm_vol->app);
		}
		break;
	case ACTION_AUD_AtCtl:
		{
			BRCM_AUDIO_Param_AtCtl_t parm_atctl;

			memcpy((void *)&parm_atctl, arg_param,
				sizeof(BRCM_AUDIO_Param_AtCtl_t));

			if (parm_atctl.isGet)
				AtAudCtlHandler_get(parm_atctl.cmdIndex,
				parm_atctl.pChip,
				parm_atctl.ParamCount,
				parm_atctl.Params);
			else
				AtAudCtlHandler_put(parm_atctl.cmdIndex,
				parm_atctl.pChip,
				parm_atctl.ParamCount,
				parm_atctl.Params);

			/*copy values back to arg_param */
			memcpy((void *)arg_param, (void *)&parm_atctl,
				sizeof(BRCM_AUDIO_Param_AtCtl_t));
		}
		break;
	case ACTION_AUD_BTTest:
		{
			BRCM_AUDIO_Param_BT_Test_t *param_bt =
				(BRCM_AUDIO_Param_BT_Test_t *) arg_param;
			aTrace(LOG_AUDIO_CNTLR,
				"\n %lx:AUDIO_Ctrl_Process-"
				"ACTION_AUD_BTTest.\n", jiffies);
			AUDCTRL_SetBTMode(param_bt->mode);
		}
		break;
	case ACTION_AUD_CfgIHF:
		{
			BRCM_AUDIO_Param_Cfg_IHF_t *param_ihf =
				(BRCM_AUDIO_Param_Cfg_IHF_t *) arg_param;
			aTrace(LOG_AUDIO_CNTLR,
				"\n %lx:AUDIO_Ctrl_Process-"
				"ACTION_AUD_CfgIHF.\n", jiffies);
			AUDCTRL_SetIHFmode(param_ihf->stIHF);
		}
		break;
	case ACTION_AUD_CfgSSP:
		{
			BRCM_AUDIO_Param_Cfg_SSP_t *param_ssp =
				(BRCM_AUDIO_Param_Cfg_SSP_t *) arg_param;
			aTrace(LOG_AUDIO_CNTLR,
				"\n %lx:AUDIO_Ctrl_Process-"
				"ACTION_AUD_CfgSSP.\n", jiffies);
			AUDCTRL_ConfigSSP(param_ssp->mode,
				param_ssp->bus,
				param_ssp->en_lpbk);
		}
		break;
	case ACTION_AUD_HwCtl:
		{
			BRCM_AUDIO_Param_HwCtl_t *param_hwCtl =
				(BRCM_AUDIO_Param_HwCtl_t *) arg_param;
			aTrace(LOG_AUDIO_CNTLR,
				"\n %lx:AUDIO_Ctrl_Process-"
				"ACTION_AUD_HwCtl.\n", jiffies);
			AUDCTRL_HardwareControl(param_hwCtl->access_type,
				param_hwCtl->arg1,
				param_hwCtl->arg2,
				param_hwCtl->arg3,
				param_hwCtl->arg4);
		}
		break;
	default:
		aError("Error AUDIO_Ctrl_Process Invalid action %d\n",
			action_code);
		break;
	}

	if (block & 1) {
		/* put the message in output fifo if waiting */
		msgAudioCtrl.action_code = action_code;
		if (arg_param)
			memcpy(&msgAudioCtrl.param, arg_param,
			       sizeof(BRCM_AUDIO_Control_Params_un_t));
		else
			memset(&msgAudioCtrl.param, 0,
			       sizeof(BRCM_AUDIO_Control_Params_un_t));
		msgAudioCtrl.pCallBack = callback;
		msgAudioCtrl.block = block;

		len = kfifo_in_locked(&sgThreadData.m_pkfifo_out,
				      (unsigned char *)&msgAudioCtrl,
				      sizeof(TMsgAudioCtrl),
				      &sgThreadData.m_lock_out);
		if (len != sizeof(TMsgAudioCtrl))
			aError("Error AUDIO_Ctrl_Process "
					"len=%d expected %d\n", len,
					sizeof(TMsgAudioCtrl));
		/* release the semaphore */
		OSSEMAPHORE_Release(sgThreadData.action_complete);
	}

	if (callback) {
		PFuncAudioCtrlCB pCB = (PFuncAudioCtrlCB)callback;
		pCB(block);
	}


}
