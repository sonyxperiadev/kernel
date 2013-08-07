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
#include <linux/dma-mapping.h>
#include <linux/irq.h>
#include <mach/cpu.h>

#include "mobcom_types.h"
#include "resultcode.h"
#include "audio_consts.h"
#include "ossemaphore.h"
#include "bcm_fuse_sysparm_CIB.h"
#include "csl_caph.h"
#include "audio_vdriver.h"
#include "audio_controller.h"
#include "audio_ddriver.h"
#include "audio_caph.h"
#include "caph_common.h"
#include "audio_trace.h"

#ifdef CONFIG_ARCH_JAVA
#include <linux/slab.h>
static struct workqueue_struct *pWorkqueue_AudioControl;
#define TIMEOUT_STOP_REQ_MS 60000
#ifdef CONFIG_SMP
#define QUEUE_WORK(x, y) queue_work(x, \
		(struct work_struct *)y);
#else
#define QUEUE_WORK(x, y) queue_work_on(0, x, \
		(struct work_struct *)y);
#endif
#endif

#define USE_HR_TIMER
/* #define USE_HUBCLK_AUTOGATE_WORKAROUND */
/* for unknown reason, aadmac autogate bit
   was set and leads to mute on Rhea. enable
   this flag to report error when it is set */
#define CHECK_AADMAC_AUTOGATE_STATUS
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
#ifdef CONFIG_ARCH_JAVA
	struct work_struct mwork;
	struct completion comp;
	bool timeout;
#endif
	BRCM_AUDIO_ACTION_en_t action_code;
	BRCM_AUDIO_Control_Params_un_t param;
	void *pCallBack;
	int block;

};
#define	TMsgAudioCtrl struct _TMsgBrcmAudioCtrl

#ifndef CONFIG_ARCH_JAVA
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
#endif

#if 0
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
		"AtCtl",
};
#endif

static unsigned int pathID[CTL_STREAM_PANEL_LAST];
static int telephonyIsEnabled;

#ifdef CONFIG_ARCH_JAVA
static void AUDIO_Ctrl_Process(TMsgAudioCtrl *msgAudioCtrl);
#else
static unsigned int n_msg_in, n_msg_out, last_action;
static struct completion complete_kfifo;
static struct TAudioHalThreadData sgThreadData;

static DEFINE_MUTEX(mutexBlock);

#define KFIFO_SIZE		2048
#define BLOCK_WAITTIME_MS	60000
#define KFIFO_TIMEOUT_MS	60000

static void AUDIO_Ctrl_Process(BRCM_AUDIO_ACTION_en_t action_code,
			       void *arg_param, void *callback, int block);
#endif

#ifdef CONFIG_AUDIO_S2
static unsigned int pcm_vibra_path_id;
static AUDIO_DRIVER_HANDLE_t pcm_vibra_drv_handle;
static spinlock_t vibra_drv_lock;
static UInt8 *s_vibra_dma_bufp;
static int s_vibra_strength;
#define VIBRA_STRENGTH_RANGE 125 /* -125 ~ 125*/
#define VIBRA_PERIOD_BYTES 480 /*5ms*/
static void PCM_Vibra_Gen_Start(Int32 strength);
static void PCM_Vibra_Gen_Stop(void);
#endif

#ifdef USE_HR_TIMER
static enum hrtimer_restart TimerCbStopVibrator(struct hrtimer *timer)
{
	Result_t status = AUDIO_Ctrl_Trigger(ACTION_AUD_DisableByPassVibra_CB,
					     NULL, NULL, 0);

	if (status != RESULT_OK) {
		/*recur after 100ms */
		hrtimer_forward_now(timer, ktime_set(0, (100 * 1000000)));
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
#ifdef CONFIG_ARCH_JAVA
	TMsgAudioCtrl *msgAudioCtrl = container_of(work, TMsgAudioCtrl, mwork);
#else
	TMsgAudioCtrl msgAudioCtrl;
	unsigned int len = 0;

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
#endif
#if defined(CONFIG_BCM_MODEM)
		if (is_dsp_timeout())
			return;
#endif
#ifdef CONFIG_ARCH_JAVA
	/* process the operation */
	AUDIO_Ctrl_Process(msgAudioCtrl);

	if ((msgAudioCtrl && msgAudioCtrl->block != 1)
					|| (msgAudioCtrl->timeout)) {
		kfree(msgAudioCtrl);
		return;
	}

	if (msgAudioCtrl->block == 1)
		complete(&msgAudioCtrl->comp);
#else
		/* process the operation */
		AUDIO_Ctrl_Process(msgAudioCtrl.action_code,
				   &msgAudioCtrl.param,
				   msgAudioCtrl.pCallBack, msgAudioCtrl.block);
		n_msg_out++;
	}
#endif
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
 * CPResetHandler :callback function that handles CP reset
 *
 */
static void CPResetHandler(Boolean cp_reset)
{
	BRCM_AUDIO_Param_cpReset_t parm_cpReset;
	aTrace(LOG_AUDIO_CNTLR, "CPResetHandler\r\n");
	parm_cpReset.cp_reset_start = cp_reset;
	AUDIO_Ctrl_Trigger(ACTION_AUD_HandleCPReset, &parm_cpReset, NULL, 0);
}

/**
 * caph_audio_init: registers callback for handling rate change and
 * if any init required.
 *
 */
void caph_audio_init(void)
{
	AUDDRV_RegisterRateChangeCallback(AudioCodecIdHander);
	AUDDRV_RegisterHandleCPResetCB(CPResetHandler);
#ifndef CONFIG_ARCH_JAVA
	init_completion(&complete_kfifo);
#endif
#if defined(CONFIG_BCM_MODEM)
	set_flag_dsp_timeout(0);
#endif
#ifdef CONFIG_AUDIO_S2
	spin_lock_init(&vibra_drv_lock);
#endif
#ifdef USE_HR_TIMER
	hrtimer_init(&hr_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	hr_timer.function = &TimerCbStopVibrator;
#endif

}

/**
 * LaunchAudioHalThread: Create Worker thread.
 *
 */
int LaunchAudioCtrlThread(void)
{
	int ret = 0;
#ifdef CONFIG_ARCH_JAVA
	pWorkqueue_AudioControl = alloc_ordered_workqueue(
			"AudioCtrlWq", WQ_MEM_RECLAIM);
	if (!pWorkqueue_AudioControl) {
#else
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
	sgThreadData.action_complete = OSSEMAPHORE_Create(0, 0);

	INIT_WORK(&sgThreadData.mwork, AudioCtrlWorkThread);

	sgThreadData.pWorkqueue_AudioControl = alloc_ordered_workqueue(
		"AudioCtrlWq", WQ_MEM_RECLAIM);

	if (!sgThreadData.pWorkqueue_AudioControl) {
#endif
		aError("\n Error : Can not create work queue:AudioCtrlWq\n");
		return -1;
	}

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
#ifdef CONFIG_ARCH_JAVA
	if (pWorkqueue_AudioControl) {
		flush_workqueue(pWorkqueue_AudioControl);
		destroy_workqueue(pWorkqueue_AudioControl);
	}
#else
	if (sgThreadData.pWorkqueue_AudioControl) {
		flush_workqueue(sgThreadData.pWorkqueue_AudioControl);
		destroy_workqueue(sgThreadData.pWorkqueue_AudioControl);
	}
	kfifo_free(&sgThreadData.m_pkfifo);
	kfifo_free(&sgThreadData.m_pkfifo_out);
#endif
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
	case ACTION_AUD_OpenVoIP:
		size = sizeof(BRCM_AUDIO_Param_Open_t);
		break;
	case ACTION_AUD_CloseRecord:
	case ACTION_AUD_ClosePlay:
	case ACTION_AUD_CloseVoIP:
		size = sizeof(BRCM_AUDIO_Param_Close_t);
		break;
	case ACTION_AUD_StartRecord:
	case ACTION_AUD_StartPlay:
	case ACTION_AUD_StartVoIP:
		size = sizeof(BRCM_AUDIO_Param_Start_t);
		break;
	case ACTION_AUD_StopRecord:
	case ACTION_AUD_StopPlay:
	case ACTION_AUD_StopVoIP:
		size = sizeof(BRCM_AUDIO_Param_Stop_t);
		break;
	case ACTION_AUD_PausePlay:
		size = sizeof(BRCM_AUDIO_Param_Pause_t);
		break;
	case ACTION_AUD_ResumePlay:
		size = sizeof(BRCM_AUDIO_Param_Resume_t);
		break;
	case ACTION_AUD_SetPrePareParameters:
	case ACTION_AUD_SET_VOIP_UL_CB:
	case ACTION_AUD_SET_VOIP_DL_CB:
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
	case ACTION_AUD_RemoveAudioApp:
		size = sizeof(BRCM_AUDIO_Param_SetApp_t);
		break;
	case ACTION_AUD_AMPEnable:
		size = sizeof(BRCM_AUDIO_Param_AMPCTL_t);
		break;
	case ACTION_AUD_SetCallMode:
		size = sizeof(BRCM_AUDIO_Param_CallMode_t);
		break;
	case ACTION_AUD_HandleCPReset:
		size = sizeof(BRCM_AUDIO_Param_cpReset_t);
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
	case ACTION_AUD_AtCtl:
		size = sizeof(BRCM_AUDIO_Param_AtCtl_t);
		break;
	default:
		break;
	}
	return size;
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
  */

#ifdef CONFIG_ARCH_JAVA
Result_t AUDIO_Ctrl_Trigger(BRCM_AUDIO_ACTION_en_t action_code,
		void *arg_param, void *callback, int block)
{
	TMsgAudioCtrl *msgAudioCtrl = NULL;
	Result_t status = RESULT_OK;
	static int record48K_in_call_is_blocked;
	int params_size = 0;
	long ret = 0;

	if (is_dsp_timeout())
		return RESULT_OK;

	if (action_code < ACTION_AUD_OpenPlay
				|| action_code >= ACTION_AUD_TOTAL) {
		aWarn("AUDIO_Ctrl_Trigger: Not a Valid action code: %d",
								action_code);
		return RESULT_ERROR;
	}
	if (action_code == ACTION_AUD_StartPlay ||
			action_code == ACTION_AUD_StopPlay ||
			action_code == ACTION_AUD_PausePlay ||
			action_code == ACTION_AUD_ResumePlay ||
			action_code == ACTION_AUD_StartRecord ||
			action_code == ACTION_AUD_StopRecord ||
			action_code == ACTION_AUD_RateChange ||
			action_code == ACTION_AUD_AddChannel ||
			action_code ==  ACTION_AUD_RemoveChannel)
		msgAudioCtrl = (TMsgAudioCtrl *)
			kzalloc(sizeof(TMsgAudioCtrl), GFP_ATOMIC);
	else
		msgAudioCtrl = (TMsgAudioCtrl *)
			kzalloc(sizeof(TMsgAudioCtrl), GFP_KERNEL);

	if (!msgAudioCtrl) {
		aError(
				"Unable to allocate memory"
				"for workqueue action_code = %d!!!\n",
				action_code);
		return RESULT_ERROR;
	}

	/** BEGIN: not support 48KHz recording during voice call */
	if (action_code == ACTION_AUD_StartRecord) {

		BRCM_AUDIO_Param_Start_t param_start;
		memcpy((void *)&param_start, arg_param,
				sizeof(BRCM_AUDIO_Param_Start_t));

		if (param_start.callMode == MODEM_CALL
				&& param_start.rate == AUDIO_SAMPLING_RATE_48000
				&& param_start.pdev_prop->c.source !=
				AUDIO_SOURCE_I2S
				&& param_start.pdev_prop->c.drv_type
				== AUDIO_DRIVER_CAPT_HQ
				/*FM recording is supported*/
				&& telephonyIsEnabled == TRUE) {

			aError("StartRecord failed.	48KHz in call");
			record48K_in_call_is_blocked = 1;
			kfree((void *)msgAudioCtrl);
			return RESULT_ERROR;
		}
	}

	/** EDN: not support 48KHz recording during voice call.*/
	if (action_code == ACTION_AUD_StopRecord) {

		BRCM_AUDIO_Param_Stop_t param_stop;
		memcpy((void *)&param_stop, arg_param,
				sizeof(BRCM_AUDIO_Param_Stop_t));

		if (param_stop.callMode == MODEM_CALL
				&& param_stop.pdev_prop->c.source !=
				AUDIO_SOURCE_I2S
				&& param_stop.pdev_prop->c.drv_type
				== AUDIO_DRIVER_CAPT_HQ
				/*FM recording is supported*/
				&& telephonyIsEnabled == TRUE
				&& record48K_in_call_is_blocked == 1
				/* if start recording from digi mic in
				 * idle mode,then start voice call,
				   when stop recording the path shall
				   be disabled.
				   */
		   ) {

			record48K_in_call_is_blocked = 0;
			kfree((void *)msgAudioCtrl);
			return RESULT_ERROR;
		}
	}

	if (action_code == ACTION_AUD_DisableByPassVibra_CB)
		action_code = ACTION_AUD_DisableByPassVibra;

	params_size = AUDIO_Ctrl_Trigger_GetParamsSize(action_code);
	msgAudioCtrl->action_code = action_code;

	if (arg_param)
		memcpy(&msgAudioCtrl->param, arg_param, params_size);
	else
		memset(&msgAudioCtrl->param, 0, params_size);

	msgAudioCtrl->pCallBack = callback;
	msgAudioCtrl->block = block;

	INIT_WORK(&msgAudioCtrl->mwork , AudioCtrlWorkThread);

	if (msgAudioCtrl->block == 1)
		init_completion(&msgAudioCtrl->comp);

	if (msgAudioCtrl && msgAudioCtrl->block == 1) {
		QUEUE_WORK(pWorkqueue_AudioControl, msgAudioCtrl);
		ret = wait_for_completion_interruptible_timeout(
				&msgAudioCtrl->comp,
				msecs_to_jiffies(TIMEOUT_STOP_REQ_MS));
		if (ret <= 0) {
			aError(
					"ERROR timeout waiting for"
					"STOP REQ.t=%d ret=%ld action_code=%d\n",
					TIMEOUT_STOP_REQ_MS, ret, action_code);

			if (work_pending(&msgAudioCtrl->mwork)) {
				aWarn("Freeing pending work, action code=%d",
						action_code);
				cancel_work_sync(&msgAudioCtrl->mwork);
				kfree(msgAudioCtrl);
			} else
				msgAudioCtrl->timeout = TRUE;

			return RESULT_ERROR;
		} else {
			if (arg_param != NULL)
				memcpy(arg_param, &msgAudioCtrl->param,
								params_size);
			else
				memset(&msgAudioCtrl->param, 0, params_size);
		}
		kfree(msgAudioCtrl);
	} else
		QUEUE_WORK(pWorkqueue_AudioControl, msgAudioCtrl);

	return status;
}
#else
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

#if defined(CONFIG_BCM_MODEM)
	if (is_dsp_timeout())
		return RESULT_OK;
#endif

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
			if (block & 1)
				mutex_unlock(&mutexBlock);
			return RESULT_ERROR;
		} else if (is_cb) {
			aError("Audio KFIFO FULL reschedule cb action %d\n",
			       action_code);
			if (block & 1)
				mutex_unlock(&mutexBlock);
			return RESULT_ERROR;
		} else {
			unsigned long ret;
			ret =
			    wait_for_completion_timeout
			    (&complete_kfifo,
			     msecs_to_jiffies(KFIFO_TIMEOUT_MS));
			if (!ret) {
				aError("ERROR Audio KFIFO timeout avail %d, "
				       "n_msg_in 0x%x, n_msg_out 0x%x, "
				       "last_action %d, action %d\n",
				       fifo_avail, n_msg_in, n_msg_out,
				       last_action, action_code);
				if (block & 1)
					mutex_unlock(&mutexBlock);
				return RESULT_ERROR;
			}
		}
	} else if (fifo_avail < KFIFO_SIZE / 2)
		aWarn("Audio KFIFO HIGH avail %d, n_msg_in 0x%x, "
		      "n_msg_out 0x%x, last_action %d, action %d\n",
		      fifo_avail, n_msg_in, n_msg_out, last_action,
		      action_code);

	spin_lock_irqsave(&sgThreadData.m_lock, t_flag);
	fifo_avail = kfifo_avail(&sgThreadData.m_pkfifo);

	if (fifo_avail < sizeof(TMsgAudioCtrl)) {
		/*this means trigger from other thead has taken the spot */
		spin_unlock_irqrestore(&sgThreadData.m_lock, t_flag);
		/*aError("Audio KFIFO FULL loop action %d, "
		   "cb %d, atomic %d\n",
		   action_code, is_cb, is_atomic); */
		/*return RESULT_ERROR; */
		goto AUDIO_Ctrl_Trigger_Wait;
	}

	len = kfifo_in(&sgThreadData.m_pkfifo,
		       (unsigned char *)&msgAudioCtrl, sizeof(TMsgAudioCtrl));

	spin_unlock_irqrestore(&sgThreadData.m_lock, t_flag);

	/*aTrace(LOG_AUDIO_CNTLR, "AUDIO_Ctrl_Trigger action %d, avail %d, "
	       "n_msg_in 0x%x, n_msg_out 0x%x, last_action %d\n",
	       action_code, fifo_avail, n_msg_in, n_msg_out, last_action);*/

	if (len != sizeof(TMsgAudioCtrl))
		aError
		    ("Error AUDIO_Ctrl_Trigger len=%d expected %d\n", len,
		     sizeof(TMsgAudioCtrl));

#ifdef CONFIG_SMP
	queue_work(sgThreadData.pWorkqueue_AudioControl,
		      &sgThreadData.mwork);
#else
	queue_work_on(0, sgThreadData.pWorkqueue_AudioControl,
		      &sgThreadData.mwork);
#endif

	if (block & 1) {
		osStatus =
		    OSSEMAPHORE_Obtain(sgThreadData.action_complete, to_jiff);
		if (osStatus != OSSTATUS_SUCCESS) {
			aWarn("AUDIO_Ctrl_Trigger Timeout=%d\r\n", osStatus);
			status = RESULT_ERROR;
			mutex_unlock(&mutexBlock);
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
#endif

#ifdef CONFIG_ARCH_JAVA
static void AUDIO_Ctrl_Process(TMsgAudioCtrl *msgAudioCtrl)
{
	BRCM_AUDIO_ACTION_en_t action_code = msgAudioCtrl->action_code;
	void *arg_param = &msgAudioCtrl->param;
	int block = msgAudioCtrl->block;
#else
static void AUDIO_Ctrl_Process(BRCM_AUDIO_ACTION_en_t action_code,
			       void *arg_param, void *callback, int block)
{
	TMsgAudioCtrl msgAudioCtrl;
	unsigned int len;
#endif
	int i = 0;
	unsigned int path = 0;

#ifdef CHECK_AADMAC_AUTOGATE_STATUS
	if (get_chip_id() < KONA_CHIP_ID_JAVA_A0) {
		if (AUDCTRL_AadmacAutoGateStatus())
			/* this should never happen */
			aError("\n %lx:AUDIO_Ctrl_Process-"
			"!!! AADMAC_ENABLE_AUTO_GATE was set !!!\n",
			jiffies);
	}
#endif
	switch (action_code) {
	case ACTION_AUD_OpenPlay:
		{
			BRCM_AUDIO_Param_Open_t *param_open =
			    (BRCM_AUDIO_Param_Open_t *) arg_param;
			aTrace(LOG_AUDIO_CNTLR,
			       "\n %lx:AUDIO_Ctrl_Process-"
			       "ACTION_AUD_OpenPlay. stream=%d\n",
			       jiffies, param_open->stream);

			param_open->drv_handle =
			    AUDIO_DRIVER_Open(param_open->pdev_prop->p[0].
					      drv_type);
			if (param_open->drv_handle == NULL)
				aError("\n %lx:AUDIO_Ctrl_Process-"
				       "AUDIO_DRIVER_Open failed. stream=%d\n",
				       jiffies, param_open->stream);
		}
		break;

	case ACTION_AUD_ClosePlay:
		{
			BRCM_AUDIO_Param_Close_t *param_close =
			    (BRCM_AUDIO_Param_Close_t *) arg_param;
			aTrace(LOG_AUDIO_CNTLR,
			       "\n %lx:AUDIO_Ctrl_Process-"
			       "ACTION_AUD_ClosePlay. stream=%d\n",
			       jiffies, param_close->stream);

			AUDIO_DRIVER_Close(param_close->drv_handle);
		}
		break;

	case ACTION_AUD_StartPlay:
		{
		BRCM_AUDIO_Param_Start_t *param_start =
			 (BRCM_AUDIO_Param_Start_t *) arg_param;

		aTrace(LOG_AUDIO_CNTLR,
			 "\n %lx:AUDIO_Ctrl_Process-"
			 "ACTION_AUD_StartPlay. stream=%d\n",
			 jiffies, param_start->stream);
		CAPH_ASSERT(param_start->stream >=
			(CTL_STREAM_PANEL_FIRST - 1)
			&& param_start->stream <
			(CTL_STREAM_PANEL_LAST - 1));
		if ((param_start->pdev_prop->p[0].drv_type ==
			AUDIO_DRIVER_PLAY_AUDIO) ||
			(param_start->pdev_prop->p[0].drv_type ==
			AUDIO_DRIVER_PLAY_EPT)) {

			/* Enable the playback the path */
			AUDCTRL_EnablePlay(param_start->pdev_prop->p[0].
				source,
				param_start->pdev_prop->p[0].
				sink, param_start->channels,
				param_start->rate,
				param_start->bitsPerSample, &path);
				pathID[param_start->stream] = path;

			AUDIO_DRIVER_Ctrl(param_start->drv_handle,
					  AUDIO_DRIVER_START,
					  &param_start->pdev_prop->p[0].
					  sink);
		} else if (param_start->pdev_prop->p[0].drv_type ==
			   AUDIO_DRIVER_PLAY_VOICE) {
			AUDIO_DRIVER_Ctrl(param_start->drv_handle,
					  AUDIO_DRIVER_START, NULL);
		}

		}
		break;
	case ACTION_AUD_StopPlay:
		{
			BRCM_AUDIO_Param_Stop_t *param_stop =
			    (BRCM_AUDIO_Param_Stop_t *) arg_param;

			aTrace(LOG_AUDIO_CNTLR,
			       "\n %lx:AUDIO_Ctrl_Process-"
			       "ACTION_AUD_StopPlay. stream=%d\n",
			       jiffies, param_stop->stream);
			CAPH_ASSERT(param_stop->stream >=
				    (CTL_STREAM_PANEL_FIRST - 1)
				    && param_stop->stream <
				    (CTL_STREAM_PANEL_LAST - 1));

			AUDIO_DRIVER_Ctrl(param_stop->drv_handle,
					  AUDIO_DRIVER_STOP, NULL);

			/* Remove secondary playback path if it's in use */
			for (i = (MAX_PLAYBACK_DEV - 1); i > 0; i--) {
				if (param_stop->pdev_prop->p[i].sink !=
				    AUDIO_SINK_UNDEFINED) {
					AUDCTRL_RemovePlaySpk(param_stop->
							      pdev_prop->p[0]
							      .source,
							      param_stop->
							      pdev_prop->p[i].
							      sink,
							      pathID
							      [param_stop->
							       stream]);
				}
			}

			if ((param_stop->pdev_prop->p[0].drv_type ==
			     AUDIO_DRIVER_PLAY_AUDIO)
			    || (param_stop->pdev_prop->p[0].drv_type ==
				AUDIO_DRIVER_PLAY_EPT)) {
				/* disable the playback path */
				AUDCTRL_DisablePlay(param_stop->source,
						    param_stop->sink,
						    pathID[param_stop->stream]);

				pathID[param_stop->stream] = 0;
			}
			aTrace(LOG_AUDIO_CNTLR,
			       "AUDIO_Ctrl_Process ACTION_AUD_StopPlay"
			       " completed. stream=%d\n", param_stop->stream);
		}
		break;
	case ACTION_AUD_BufferReady:
		{
			BRCM_AUDIO_Param_BufferReady_t *param_bufferready =
			    (BRCM_AUDIO_Param_BufferReady_t *) arg_param;

			/* don't trace here, too frequent
			aTrace(LOG_AUDIO_CNTLR,
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

			aTrace(LOG_AUDIO_CNTLR,
			       "\n %lx:AUDIO_Ctrl_Process-"
			       "ACTION_AUD_PausePlay. stream=%d\n",
			       jiffies, param_pause->stream);
			CAPH_ASSERT(param_pause->stream >=
				    (CTL_STREAM_PANEL_FIRST - 1)
				    && param_pause->stream <
				    (CTL_STREAM_PANEL_LAST - 1));

			if ((param_pause->pdev_prop->p[0].drv_type ==
			     AUDIO_DRIVER_PLAY_AUDIO)
			    || (param_pause->pdev_prop->p[0].drv_type ==
				AUDIO_DRIVER_PLAY_EPT)) {
				/* disable the playback path */
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

		aTrace(LOG_AUDIO_CNTLR,
			     "\n %lx:AUDIO_Ctrl_Process-"
			     "ACTION_AUD_ResumePlay. stream=%d\n",
			       jiffies, param_resume->stream);
		CAPH_ASSERT(param_resume->stream >=
			    (CTL_STREAM_PANEL_FIRST - 1)
			    && param_resume->stream <
			    (CTL_STREAM_PANEL_LAST - 1));

		AUDIO_DRIVER_Ctrl(param_resume->drv_handle,
				  AUDIO_DRIVER_RESUME, NULL);

		if ((param_resume->pdev_prop->p[0].drv_type ==
			 AUDIO_DRIVER_PLAY_AUDIO)
			 || (param_resume->pdev_prop->p[0].drv_type ==
			AUDIO_DRIVER_PLAY_EPT)) {
			/*  Enable the playback the path */
			AUDCTRL_EnablePlay(param_resume->pdev_prop->p[0]
						.source,
						param_resume->pdev_prop->
						p[0].sink,
						param_resume->channels,
						param_resume->bits_per_sample,
						param_resume->rate, &path);
			pathID[param_resume->stream] = path;
		}

		}
		break;
	case ACTION_AUD_StartRecord:
		{
			BRCM_AUDIO_Param_Start_t *param_start =
			    (BRCM_AUDIO_Param_Start_t *) arg_param;

			aTrace(LOG_AUDIO_CNTLR,
			       "\n %lx:AUDIO_Ctrl_Process-"
			       "ACTION_AUD_StartRecord. stream=%d\n",
			       jiffies, param_start->stream);
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

			pathID[param_start->stream] = path;
			if ((param_start->pdev_prop->c.drv_type ==
			     AUDIO_DRIVER_CAPT_HQ)
			    || (param_start->pdev_prop->c.drv_type ==
				AUDIO_DRIVER_CAPT_EPT)) {

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

		}
		break;
	case ACTION_AUD_StopRecord:
		{
			BRCM_AUDIO_Param_Stop_t *param_stop =
			    (BRCM_AUDIO_Param_Stop_t *) arg_param;

			aTrace(LOG_AUDIO_CNTLR,
			       "\n %lx:AUDIO_Ctrl_Process-"
			       "ACTION_AUD_StopRecord. stream=%d\n",
			       jiffies, param_stop->stream);
			CAPH_ASSERT(param_stop->stream >=
				    (CTL_STREAM_PANEL_FIRST - 1)
				    && param_stop->stream <
				    (CTL_STREAM_PANEL_LAST - 1));

			AUDIO_DRIVER_Ctrl(param_stop->drv_handle,
					  AUDIO_DRIVER_STOP, NULL);

			AUDCTRL_DisableRecord(param_stop->pdev_prop->c.source,
					param_stop->pdev_prop->c.sink,
					pathID[param_stop->stream]);
			pathID[param_stop->stream] = 0;
		}
		break;
	case ACTION_AUD_OpenRecord:
		{
			BRCM_AUDIO_Param_Open_t *param_open =
			    (BRCM_AUDIO_Param_Open_t *) arg_param;

			aTrace(LOG_AUDIO_CNTLR,
			       "\n %lx:AUDIO_Ctrl_Process-"
			       "ACTION_AUD_OpenRecord. stream=%d\n",
			       jiffies, param_open->stream);
			param_open->drv_handle =
			    AUDIO_DRIVER_Open(param_open->pdev_prop->c.
					      drv_type);

			if (param_open->drv_handle == NULL)
				aError("param_open->drv_handle -  0x%lx\n",
				       (UInt32) param_open->drv_handle);

		}
		break;
	case ACTION_AUD_CloseRecord:
		{
			BRCM_AUDIO_Param_Close_t *param_close =
			    (BRCM_AUDIO_Param_Close_t *) arg_param;

			aTrace(LOG_AUDIO_CNTLR,
			       "\n %lx:AUDIO_Ctrl_Process-"
			       "ACTION_AUD_CloseRecord. stream=%d\n",
			       jiffies, param_close->stream);
			AUDIO_DRIVER_Close(param_close->drv_handle);
		}
		break;
	case ACTION_AUD_AddChannel:
		{
			BRCM_AUDIO_Param_Spkr_t *parm_spkr =
			    (BRCM_AUDIO_Param_Spkr_t *) arg_param;
			aTrace(LOG_AUDIO_CNTLR,
			       "\n %lx:AUDIO_Ctrl_Process-"
			       "ACTION_AUD_AddChannel. stream=%d\n",
			       jiffies, parm_spkr->stream);
			CAPH_ASSERT(parm_spkr->stream >=
				    (CTL_STREAM_PANEL_FIRST - 1)
				    && parm_spkr->stream <
				    (CTL_STREAM_PANEL_LAST - 1));
			AUDCTRL_AddPlaySpk(parm_spkr->src, parm_spkr->sink,
					   pathID[parm_spkr->stream]);
		}
		break;
	case ACTION_AUD_RemoveChannel:
		{
			BRCM_AUDIO_Param_Spkr_t *parm_spkr =
			    (BRCM_AUDIO_Param_Spkr_t *) arg_param;
			aTrace(LOG_AUDIO_CNTLR,
			       "\n %lx:AUDIO_Ctrl_Process-"
			       "ACTION_AUD_RemoveChannel. stream=%d\n",
			       jiffies, parm_spkr->stream);
			CAPH_ASSERT(parm_spkr->stream >=
				    (CTL_STREAM_PANEL_FIRST - 1)
				    && parm_spkr->stream <
				    (CTL_STREAM_PANEL_LAST - 1));
			AUDCTRL_RemovePlaySpk(parm_spkr->src, parm_spkr->sink,
					      pathID[parm_spkr->stream]);
		}
		break;

	case ACTION_AUD_EnableTelephony:
		{
			BRCM_AUDIO_Param_Call_t *parm_call =
			    (BRCM_AUDIO_Param_Call_t *) arg_param;

			aTrace(LOG_AUDIO_CNTLR,
			    "AUDIO_Ctrl_Process ACTION_AUD_EnableTelephony:mic=%d, spkr=%d\n",
			    parm_call->new_mic, parm_call->new_spkr);

			telephonyIsEnabled = TRUE; /*DSP is using mic path */
			AUDCTRL_EnableTelephony(parm_call->new_mic,
						parm_call->new_spkr);
		}
		break;

	case ACTION_AUD_DisableTelephony:
		aTrace(LOG_AUDIO_CNTLR,
		    "AUDIO_Ctrl_Process ACTION_AUD_DisableTelephony:\n");

		AUDCTRL_DisableTelephony();
		telephonyIsEnabled = FALSE;
		break;

	case ACTION_AUD_SetTelephonyMicSpkr:
		{
			BRCM_AUDIO_Param_Call_t *parm_call =
			    (BRCM_AUDIO_Param_Call_t *) arg_param;

			aTrace(LOG_AUDIO_CNTLR,
			    "AUDIO_Ctrl_Process ACTION_AUD_SetTelephonyMicSpkr:mic=%d, spkr=%d\n",
			    parm_call->new_mic, parm_call->new_spkr);

			AUDCTRL_SetTelephonyMicSpkr(parm_call->new_mic,
						    parm_call->new_spkr,
						    false);
		}
		break;

	case ACTION_AUD_MutePlayback:
		{
			BRCM_AUDIO_Param_Mute_t *parm_mute =
			    (BRCM_AUDIO_Param_Mute_t *) arg_param;
			aTrace(LOG_AUDIO_CNTLR,
			       "\n %lx:AUDIO_Ctrl_Process-"
			       "ACTION_AUD_MutePlayback. stream=%d\n",
			       jiffies, parm_mute->stream);
			CAPH_ASSERT(parm_mute->stream >=
				    (CTL_STREAM_PANEL_FIRST - 1)
				    && parm_mute->stream <
				    (CTL_STREAM_PANEL_LAST - 1));
			/*
			 * currently driver doesnt handle Mute for left/right
			 * channels
			 */
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
			aTrace(LOG_AUDIO_CNTLR,
			       "\n %lx:AUDIO_Ctrl_Process-"
			       "ACTION_AUD_MuteRecord. stream=%d\n",
			       jiffies, parm_mute->stream);
			CAPH_ASSERT(parm_mute->stream >=
				    (CTL_STREAM_PANEL_FIRST - 1)
				    && parm_mute->stream <
				    (CTL_STREAM_PANEL_LAST - 1));
			AUDCTRL_SetRecordMute(parm_mute->source,
					      parm_mute->mute1,
					      pathID[parm_mute->stream]);
		}
		break;
	case ACTION_AUD_EnableByPassVibra:
		{
			BRCM_AUDIO_Param_Vibra_t *parm_vibra =
			    (BRCM_AUDIO_Param_Vibra_t *) arg_param;
#ifdef USE_HR_TIMER
			int isHRTimActive = 0;
#endif

			aTrace(LOG_AUDIO_CNTLR,
			       "ACTION_AUD_EnableVibra"
			       "and SetVibraStrength\n");
#ifdef CONFIG_AUDIO_S2
			PCM_Vibra_Gen_Start(parm_vibra->strength);
#else
			AUDCTRL_EnableBypassVibra(parm_vibra->strength,
						  parm_vibra->direction);
#endif

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
				gpVibratorTimer->function = TimerCbStopVibrator;
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
#ifdef CONFIG_AUDIO_S2
			PCM_Vibra_Gen_Stop();
#else
			AUDCTRL_DisableBypassVibra();
#endif
		}
		break;
	case ACTION_AUD_SetPlaybackVolume:
		{
			BRCM_AUDIO_Param_Volume_t *parm_vol =
			    (BRCM_AUDIO_Param_Volume_t *) arg_param;
			aTrace(LOG_AUDIO_CNTLR,
			       "\n %lx:AUDIO_Ctrl_Process-"
			       "ACTION_AUD_SetPlaybackVolume. stream=%d\n",
			       jiffies, parm_vol->stream);
			CAPH_ASSERT(parm_vol->stream >=
				    (CTL_STREAM_PANEL_FIRST - 1)
				    && parm_vol->stream <
				    (CTL_STREAM_PANEL_LAST - 1));
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
			aTrace(LOG_AUDIO_CNTLR,
			       "\n %lx:AUDIO_Ctrl_Process-"
			       "ACTION_AUD_SetRecordVolume. stream=%d\n",
			       jiffies, parm_vol->stream);
			CAPH_ASSERT(parm_vol->stream >=
				    (CTL_STREAM_PANEL_FIRST - 1)
				    && parm_vol->stream <
				    (CTL_STREAM_PANEL_LAST - 1));
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

			aTrace(LOG_AUDIO_CNTLR,
			    "AUDIO_Ctrl_Process ACTION_AUD_SetTelephonySpkrVolume:sink=%d, vol=%d\n",
			    (int)parm_vol->sink, (int)parm_vol->volume1);

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
			aTrace(LOG_AUDIO_CNTLR,
			       "\n %lx:AUDIO_Ctrl_Process-"
			       "ACTION_AUD_SwitchSpkr. stream=%d\n",
			       jiffies, parm_spkr->stream);
			CAPH_ASSERT(parm_spkr->stream >=
				    (CTL_STREAM_PANEL_FIRST - 1)
				    && parm_spkr->stream <
				    (CTL_STREAM_PANEL_LAST - 1));
			audio_mode = GetAudioModeBySink(parm_spkr->sink);
			AUDCTRL_SaveAudioMode(audio_mode);
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

			aTrace(LOG_AUDIO_CNTLR,
			    "AUDIO_Ctrl_Process ACTION_AUD_SetAudioMode:new_spkr=%d\n",
			    parm_call->new_spkr);

			AUDCTRL_SetAudioMode(tempMode, AUDCTRL_GetAudioApp());
		}
		break;

	case ACTION_AUD_CommitAudioProfile:
		{
			aTrace(LOG_AUDIO_CNTLR,
				"AUDIO_Ctrl_Process ACTION_AUD_CommitAudioProfile:Mode=%d, app=%d\n",
				AUDCTRL_GetAudioMode(), AUDCTRL_GetAudioApp());

			AUDCTRL_SetAudioMode(AUDCTRL_GetAudioMode(),
					AUDCTRL_GetAudioApp());
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
			CAPH_ASSERT(parm_FM->stream >=
				    (CTL_STREAM_PANEL_FIRST - 1)
				    && parm_FM->stream <
				    (CTL_STREAM_PANEL_LAST - 1));

			aTrace(LOG_AUDIO_CNTLR,
			    "AUDIO_Ctrl_Process ACTION_AUD_EnableFMPlay\n");

			/*
			 * do not change voice call's audio mode. will delete
			 * the lines
			 *
			 * can set music app and mode
			 */
			/* re-enable FM; need to fill audio app */
			AUDCTRL_SaveAudioMode((AudioMode_t) parm_FM->sink);

			AUDCTRL_EnablePlay(parm_FM->source,
					   parm_FM->sink,
					   AUDIO_CHANNEL_STEREO,
						AUDIO_SAMPLING_RATE_48000,
						16, &path);
			pathID[parm_FM->stream] = path;
		}
		break;
	case ACTION_AUD_DisableFMPlay:
		{
			BRCM_AUDIO_Param_FM_t *parm_FM =
			    (BRCM_AUDIO_Param_FM_t *) arg_param;

			aTrace(LOG_AUDIO_CNTLR,
			    "AUDIO_Ctrl_Process ACTION_AUD_DisableFMPlay\n");

			CAPH_ASSERT(parm_FM->stream >=
				    (CTL_STREAM_PANEL_FIRST - 1)
				    && parm_FM->stream <
				    (CTL_STREAM_PANEL_LAST - 1));
			AUDCTRL_DisablePlay(parm_FM->source, parm_FM->sink,
					    pathID[parm_FM->stream]);
			pathID[parm_FM->stream] = 0;
		}
		break;
	case ACTION_AUD_SetARM2SPInst:
		{
			BRCM_AUDIO_Param_FM_t *parm_FM =
			    (BRCM_AUDIO_Param_FM_t *) arg_param;

			aTrace(LOG_AUDIO_CNTLR,
			    "AUDIO_Ctrl_Process ACTION_AUD_SetARM2SPInst\n");

			AUDCTRL_SetArm2spParam(CSL_CAPH_CFG_ARM2SP_FM,
				parm_FM->fm_mix);
		}
		break;
	case ACTION_AUD_SetPrePareParameters:
		{
			BRCM_AUDIO_Param_Prepare_t *parm_prepare =
			    (BRCM_AUDIO_Param_Prepare_t *) arg_param;
			aTrace(LOG_AUDIO_CNTLR,
			       "\n %lx:AUDIO_Ctrl_Process-"
			       "ACTION_AUD_SetPrePareParameters. stream=%d\n",
			       jiffies, parm_prepare->stream);
			/* set the callback */
			AUDIO_DRIVER_Ctrl(parm_prepare->drv_handle,
					  AUDIO_DRIVER_SET_CB,
					  (void *)&parm_prepare->cbParams);
			/* set the count of periods */
			AUDIO_DRIVER_Ctrl(parm_prepare->drv_handle,
					  AUDIO_DRIVER_SET_PERIOD_COUNT,
					  (void *)&parm_prepare->period_count);
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

			aTrace(LOG_AUDIO_CNTLR,
			    "AUDIO_Ctrl_Process ACTION_AUD_MuteTelephony: source=%d,mute1=%d\n",
			    (int)parm_mute->source, (int)parm_mute->mute1);

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

			aTrace(LOG_AUDIO_CNTLR,
			    "AUDIO_Ctrl_Process ACTION_AUD_RateChange: codecID=%d\n",
			    param_rate_change->codecID);


			/* 0x0A as per 3GPP 26.103 Sec 6.3 indicates AMR WB
			 * AUDIO_ID_CALL16k
			 * 0x06 indicates AMR NB
			 */
			AUDCTRL_Telephony_RequestRateChange(param_rate_change->
							    codecID);
		}
		break;

	case ACTION_AUD_HandleCPReset:
		{
			BRCM_AUDIO_Param_cpReset_t *param_cpReset =
			    (BRCM_AUDIO_Param_cpReset_t *) arg_param;
			aTrace(LOG_AUDIO_CNTLR, "ACTION_AUD_HandleCPReset\n");

			AUDCTRL_HandleCPReset(param_cpReset->cp_reset_start);
		}
		break;

	case ACTION_AUD_SetAudioApp:
		{
			BRCM_AUDIO_Param_SetApp_t *parm_setapp =
				(BRCM_AUDIO_Param_SetApp_t *) arg_param;

			aTrace(LOG_AUDIO_CNTLR,
				"AUDIO_Ctrl_Process ACTION_AUD_SetAudioApp: aud_app=%d\n",
				parm_setapp->aud_app);

			AUDCTRL_SaveAudioApp(parm_setapp->aud_app);
		}
		break;
	case ACTION_AUD_RemoveAudioApp:
		{
			BRCM_AUDIO_Param_SetApp_t *parm_rmapp =
				(BRCM_AUDIO_Param_SetApp_t *) arg_param;

			aTrace(LOG_AUDIO_CNTLR,
				"AUDIO_Ctrl_Process ACTION_AUD_RemvoeAudioApp: aud_app=%d\n",
				parm_rmapp->aud_app);

			AUDCTRL_RemoveAudioApp(parm_rmapp->aud_app);
		}
		break;
	case ACTION_AUD_AMPEnable:
		{
			BRCM_AUDIO_Param_AMPCTL_t *parm_setamp =
			    (BRCM_AUDIO_Param_AMPCTL_t *) arg_param;

			aTrace(LOG_AUDIO_CNTLR,
			    "AUDIO_Ctrl_Process ACTION_AUD_AMPEnable: amp_status=%d\n",
			    (int)parm_setamp->amp_status);

			AUDCTRL_EnableAmp(parm_setamp->amp_status);

		}
		break;
	case ACTION_AUD_SetCallMode:
		{
			BRCM_AUDIO_Param_CallMode_t *parm_callmode =
				(BRCM_AUDIO_Param_CallMode_t *)arg_param;

			aTrace(LOG_AUDIO_CNTLR,
			    "AUDIO_Ctrl_Process ACTION_AUD_SetCallMode: callMode=%d\n",
			    (int)parm_callmode->callMode);

			AUDCTRL_SetCallMode(parm_callmode->callMode);
		}
		break;
	case ACTION_AUD_ConnectDL: /* PTT call */

		aTrace(LOG_AUDIO_CNTLR,
		    "AUDIO_Ctrl_Process ACTION_AUD_ConnectDL:\n");

		AUDCTRL_ConnectDL();
		break;
	case ACTION_AUD_UpdateUserVolSetting:
		{
			BRCM_AUDIO_Param_Volume_t *parm_vol =
				(BRCM_AUDIO_Param_Volume_t *) arg_param;

			aTrace(LOG_AUDIO_CNTLR,
			    "AUDIO_Ctrl_Process ACTION_AUD_UpdateUserVolSetting:\n");

			AUDCTRL_UpdateUserVolSetting(
				parm_vol->sink,
				parm_vol->volume1,
				parm_vol->volume2,
				parm_vol->app);
		}
		break;
	case ACTION_AUD_OpenVoIP:
		{
			BRCM_AUDIO_Param_Open_t *param_open =
			    (BRCM_AUDIO_Param_Open_t *) arg_param;
			aTrace(LOG_AUDIO_CNTLR,
				"\n %lx:AUDIO_Ctrl_Process-"
				"ACTION_AUD_OpenVoIP.\n", jiffies);

			param_open->drv_handle =
			    AUDIO_DRIVER_Open(param_open->drv_type);
			if (param_open->drv_handle == NULL)
				aError("\n %lx:AUDIO_Ctrl_Process-"
					"ACTION_AUD_OpenVoIP failed.\n",
					jiffies);
		}
		break;
	case ACTION_AUD_CloseVoIP:
		{
			BRCM_AUDIO_Param_Close_t *param_close =
			    (BRCM_AUDIO_Param_Close_t *) arg_param;
			aTrace(LOG_AUDIO_CNTLR,
				"\n %lx:AUDIO_Ctrl_Process-"
				"ACTION_AUD_CloseVoIP.\n", jiffies);

			AUDIO_DRIVER_Close(param_close->drv_handle);
		}
		break;
	case ACTION_AUD_SET_VOIP_UL_CB:
		{
			BRCM_AUDIO_Param_Prepare_t *parm_prepare =
			    (BRCM_AUDIO_Param_Prepare_t *) arg_param;
			aTrace(LOG_AUDIO_CNTLR,
				"\n %lx:AUDIO_Ctrl_Process-"
				"ACTION_AUD_SET_VOIP_UL_CB.\n", jiffies);
			AUDIO_DRIVER_Ctrl(parm_prepare->drv_handle,
				AUDIO_DRIVER_SET_VOIP_UL_CB,
				(void *)&parm_prepare->cbParams);
		}
		break;
	case ACTION_AUD_SET_VOIP_DL_CB:
		{
			BRCM_AUDIO_Param_Prepare_t *parm_prepare =
			    (BRCM_AUDIO_Param_Prepare_t *) arg_param;
			aTrace(LOG_AUDIO_CNTLR,
				"\n %lx:AUDIO_Ctrl_Process-"
				"ACTION_AUD_SET_VOIP_DL_CB.\n", jiffies);
			AUDIO_DRIVER_Ctrl(parm_prepare->drv_handle,
				AUDIO_DRIVER_SET_VOIP_DL_CB,
				(void *)&parm_prepare->cbParams);
		}
		break;
	case ACTION_AUD_StartVoIP:
		{
			BRCM_AUDIO_Param_Start_t *param_start =
				(BRCM_AUDIO_Param_Start_t *) arg_param;

			aTrace(LOG_AUDIO_CNTLR,
				"\n %lx:AUDIO_Ctrl_Process-"
				"ACTION_AUD_StartVoIPl.\n", jiffies);
			AUDIO_DRIVER_Ctrl(param_start->drv_handle,
				AUDIO_DRIVER_START, param_start->data);
		}
		break;
	case ACTION_AUD_StopVoIP:
		{
			BRCM_AUDIO_Param_Stop_t *param_stop =
				(BRCM_AUDIO_Param_Stop_t *) arg_param;

			aTrace(LOG_AUDIO_CNTLR,
				"\n %lx:AUDIO_Ctrl_Process-"
				"ACTION_AUD_StopVoIP.\n", jiffies);
			AUDIO_DRIVER_Ctrl(param_stop->drv_handle,
				AUDIO_DRIVER_STOP, NULL);
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
	default:
		aError("Error AUDIO_Ctrl_Process Invalid action %d\n",
		       action_code);
		break;
	}

#ifdef CONFIG_ARCH_JAVA
	if ((msgAudioCtrl->pCallBack != NULL) && (!msgAudioCtrl->timeout)) {
		PFuncAudioCtrlCB pCB = (PFuncAudioCtrlCB)msgAudioCtrl->
								pCallBack;
		pCB(block);
	}
#else
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
#endif

}


#ifdef CONFIG_AUDIO_S2

/*store 2 periods*/
static const UInt16 sample_tone190_48m[VIBRA_PERIOD_BYTES] = {
	0xFFFF, 0x032B, 0x065A, 0x098A, 0x0CB3, 0x0FDD, 0x1308, 0x1627,
	0x194B, 0x1C69, 0x1F7E, 0x2293, 0x25A3, 0x28A6, 0x2BAB, 0x2EA7,
	0x3196, 0x3483, 0x3767, 0x3A3E, 0x3D11, 0x3FDA, 0x4294, 0x4549,
	0x47ED, 0x4A8B, 0x4D1D, 0x4F9E, 0x5217, 0x5484, 0x56DD, 0x592F,
	0x5B72, 0x5DA1, 0x5FC7, 0x61DE, 0x63E0, 0x65D8, 0x67BE, 0x6994,
	0x6B56, 0x6D0B, 0x6EAE, 0x703D, 0x71BC, 0x732A, 0x7482, 0x75CB,
	0x7701, 0x7824, 0x7933, 0x7A30, 0x7B18, 0x7BEF, 0x7CB1, 0x7D5E,
	0x7DF9, 0x7E80, 0x7EF2, 0x7F50, 0x7F9B, 0x7FD0, 0x7FF2, 0x7FFF,
	0x7FF8, 0x7FDD, 0x7FAD, 0x7F6A, 0x7F12, 0x7EA6, 0x7E26, 0x7D92,
	0x7CEA, 0x7C30, 0x7B5F, 0x7A7B, 0x7987, 0x787D, 0x7760, 0x7632,
	0x74EF, 0x739C, 0x7235, 0x70BA, 0x6F32, 0x6D94, 0x6BE5, 0x6A29,
	0x6858, 0x6676, 0x6488, 0x6287, 0x6075, 0x5E59, 0x5C2A, 0x59EB,
	0x57A3, 0x5548, 0x52E1, 0x5071, 0x4DEF, 0x4B60, 0x48CC, 0x4626,
	0x4374, 0x40BF, 0x3DF8, 0x3B28, 0x3855, 0x3572, 0x3288, 0x2F9C,
	0x2CA2, 0x29A1, 0x269F, 0x2390, 0x207D, 0x1D6A, 0x1A4D, 0x172B,
	0x140C, 0x10E5, 0x0DB9, 0x0A92, 0x0762, 0x0432, 0x0107, 0xFDD6,
	0xFAA6, 0xF77D, 0xF44E, 0xF122, 0xEDFD, 0xEAD6, 0xE7B3, 0xE497,
	0xE17C, 0xDE65, 0xDB59, 0xD84D, 0xD548, 0xD24F, 0xCF58, 0xCC68,
	0xC986, 0xC6A7, 0xC3D2, 0xC10B, 0xBE49, 0xBB91, 0xB8E9, 0xB647,
	0xB3B0, 0xB12C, 0xAEAF, 0xAC3F, 0xA9DC, 0xA78A, 0xA543, 0xA30A,
	0xA0E4, 0x9EC9, 0x9CBD, 0x9AC5, 0x98D9, 0x96FE, 0x9537, 0x937D,
	0x91D4, 0x9040, 0x8EBB, 0x8D47, 0x8BE8, 0x8A9A, 0x895E, 0x8836,
	0x8720, 0x861D, 0x852F, 0x8453, 0x838A, 0x82D7, 0x8237, 0x81A8,
	0x8131, 0x80CC, 0x807B, 0x803F, 0x8017, 0x8003, 0x8003, 0x8019,
	0x8042, 0x807F, 0x80D0, 0x8137, 0x81AF, 0x823D, 0x82DF, 0x8393,
	0x845D, 0x853A, 0x8628, 0x872C, 0x8845, 0x896C, 0x8AAA, 0x8BF9,
	0x8D58, 0x8ECC, 0x9052, 0x91E8, 0x9391, 0x954C, 0x9714, 0x98F1,
	0x9ADD, 0x9CD6, 0x9EE2, 0xA0FE, 0xA325, 0xA55F, 0xA7A6, 0xA9F8,
	0xAC5C, 0xAECC, 0xB145, 0xB3D0, 0xB666, 0xB903, 0xBBB1, 0xBE6A,
	0xC126, 0xC3F4, 0xC6C9, 0xC9A3, 0xCC8B, 0xCF7B, 0xD26E, 0xD56C,
	0xD872, 0xDB79, 0xDE8A, 0xE1A1, 0xE4B7, 0xE7D7, 0xEAFC, 0xEE1D,
	0xF148, 0xF475, 0xF79D, 0xFACC, 0xFDFD, 0x0128, 0x0459, 0x0789,
	0x0AB1, 0x0DDF, 0x110A, 0x142C, 0x1752, 0x1A73, 0x1D8A, 0x20A2,
	0x23B6, 0x26BE, 0x29C5, 0x2CC5, 0x2FB9, 0x32AC, 0x3596, 0x3872,
	0x3B4B, 0x3E1B, 0x40DA, 0x4395, 0x4646, 0x48E5, 0x4B80, 0x4E0D,
	0x508A, 0x52FE, 0x5566, 0x57BA, 0x5A07, 0x5C44, 0x5E6F, 0x608E,
	0x62A0, 0x649B, 0x668E, 0x686E, 0x6A3B, 0x6BFA, 0x6DA8, 0x6F41,
	0x70CC, 0x7246, 0x73A9, 0x74FF, 0x7640, 0x776D, 0x788A, 0x7992,
	0x7A87, 0x7B69, 0x7C38, 0x7CF2, 0x7D9A, 0x7E2D, 0x7EAB, 0x7F16,
	0x7F6D, 0x7FAF, 0x7FDE, 0x7FF9, 0x7FFE, 0x7FF0, 0x7FCE, 0x7F97,
	0x7F4C, 0x7EEE, 0x7E7A, 0x7DF2, 0x7D57, 0x7CA8, 0x7BE5, 0x7B10,
	0x7A25, 0x7926, 0x7818, 0x76F3, 0x75BC, 0x7475, 0x7319, 0x71AB,
	0x702D, 0x6E9A, 0x6CF7, 0x6B45, 0x697F, 0x67A8, 0x65C4, 0x63CC,
	0x61C5, 0x5FB2, 0x5D8C, 0x5B57, 0x5917, 0x56C6, 0x5467, 0x51FF,
	0x4F84, 0x4CFE, 0x4A71, 0x47D2, 0x4529, 0x4278, 0x3FB9, 0x3CF0,
	0x3A22, 0x3745, 0x3460, 0x3178, 0x2E83, 0x2B86, 0x2888, 0x257E,
	0x226E, 0x1F5E, 0x1C43, 0x1925, 0x1608, 0x12E2, 0x0FB7, 0x0C93,
	0x0964, 0x0634, 0x030A, 0xFFDA, 0xFCA8, 0xF97E, 0xF64F, 0xF322,
	0xEFFC, 0xECD3, 0xE9AC, 0xE690, 0xE371, 0xE058, 0xDD48, 0xDA38,
	0xD72F, 0xD432, 0xD136, 0xCE41, 0xCB59, 0xC876, 0xC59A, 0xC2CC,
	0xC005, 0xBD46, 0xBA97, 0xB7EE, 0xB551, 0xB2C5, 0xB03F, 0xADC6,
	0xAB61, 0xA901, 0xA6B1, 0xA473, 0xA241, 0xA01B, 0x9E09, 0x9C03,
	0x9A0E, 0x982B, 0x9656, 0x9492, 0x92E1, 0x913F, 0x8FAE, 0x8E32,
	0x8CC4, 0x8B6B, 0x8A25, 0x88F0, 0x87CD, 0x86C0, 0x85C4, 0x84DB,
	0x8408, 0x8347, 0x8299, 0x8200, 0x817A, 0x810A, 0x80AD, 0x8063,
	0x802E, 0x800E, 0x8001, 0x8009, 0x8026, 0x8055, 0x809A, 0x80F3,
	0x8160, 0x81E2, 0x8275, 0x831E, 0x83DB, 0x84AB, 0x858F, 0x8687,
	0x8790, 0x88AE, 0x89DF, 0x8B21, 0x8C77, 0x8DDF, 0x8F58, 0x90E4,
	0x9283, 0x942F, 0x95F0, 0x97C2, 0x99A1, 0x9B93, 0x9D96, 0x9FA4,
	0xA1C6, 0xA3F5, 0xA630, 0xA87E, 0xAAD9, 0xAD3D, 0xAFB2, 0xB235
};


static const UInt16 sample_tone195_48m[VIBRA_PERIOD_BYTES] = {
	0xFFFF, 0x0344, 0x0687, 0x09CC, 0x0D0C, 0x104A, 0x1387, 0x16C0,
	0x19F4, 0x1D25, 0x2051, 0x2378, 0x2699, 0x29B3, 0x2CC6, 0x2FD1,
	0x32D5, 0x35D1, 0x38C3, 0x3BAB, 0x3E8B, 0x415F, 0x4429, 0x46E7,
	0x4999, 0x4C3F, 0x4ED8, 0x5164, 0x53E3, 0x5654, 0x58B6, 0x5B0A,
	0x5D4D, 0x5F83, 0x61A8, 0x63BB, 0x65C1, 0x67B3, 0x6994, 0x6B64,
	0x6D22, 0x6ECF, 0x7067, 0x71EE, 0x7362, 0x74C2, 0x760F, 0x7748,
	0x786E, 0x797F, 0x7A7C, 0x7B65, 0x7C38, 0x7CF8, 0x7DA2, 0x7E37,
	0x7EB8, 0x7F23, 0x7F79, 0x7FBA, 0x7FE5, 0x7FFC, 0x7FFD, 0x7FE8,
	0x7FBE, 0x7F7F, 0x7F2B, 0x7EC1, 0x7E42, 0x7DAE, 0x7D06, 0x7C48,
	0x7B76, 0x7A8F, 0x7993, 0x7883, 0x7760, 0x7628, 0x74DD, 0x737E,
	0x720C, 0x7086, 0x6EEE, 0x6D44, 0x6B88, 0x69B9, 0x67D9, 0x65E6,
	0x63E5, 0x61D1, 0x5FAD, 0x5D7A, 0x5B37, 0x58E5, 0x5683, 0x5414,
	0x5196, 0x4F0C, 0x4C73, 0x49CE, 0x471C, 0x4460, 0x4197, 0x3EC3,
	0x3BE4, 0x38FD, 0x360C, 0x3311, 0x300E, 0x2D02, 0x29F0, 0x26D6,
	0x23B6, 0x2090, 0x1D64, 0x1A34, 0x16FF, 0x13C6, 0x108B, 0x0D4C,
	0x0A0B, 0x06C8, 0x0385, 0x0040, 0xFCFC, 0xF9B8, 0xF675, 0xF334,
	0xEFF5, 0xECB9, 0xE980, 0xE64B, 0xE31A, 0xDFED, 0xDCC6, 0xD9A4,
	0xD68A, 0xD376, 0xD06A, 0xCD66, 0xCA6A, 0xC776, 0xC48D, 0xC1AE,
	0xBED8, 0xBC0D, 0xB94F, 0xB69C, 0xB3F4, 0xB159, 0xAECE, 0xAC4D,
	0xA9DB, 0xA778, 0xA523, 0xA2DE, 0xA0A8, 0x9E82, 0x9C6C, 0x9A67,
	0x9873, 0x9690, 0x94BE, 0x92FF, 0x9152, 0x8FB7, 0x8E2F, 0x8CBB,
	0x8B57, 0x8A0A, 0x88CF, 0x87A8, 0x8695, 0x8596, 0x84AD, 0x83D7,
	0x8317, 0x826A, 0x81D3, 0x8152, 0x80E4, 0x808C, 0x804A, 0x801D,
	0x8004, 0x8002, 0x8015, 0x803E, 0x807C, 0x80CE, 0x8136, 0x81B3,
	0x8246, 0x82EC, 0x83A9, 0x847A, 0x855E, 0x8658, 0x8766, 0x8889,
	0x89BF, 0x8B0A, 0x8C67, 0x8DD8, 0x8F5B, 0x90F1, 0x929A, 0x9456,
	0x9624, 0x9802, 0x99F2, 0x9BF4, 0x9E05, 0xA027, 0xA25A, 0xA49C,
	0xA6ED, 0xA94D, 0xABBB, 0xAE38, 0xB0C3, 0xB35A, 0xB5FE, 0xB8AF,
	0xBB6A, 0xBE32, 0xC105, 0xC3E2, 0xC6CA, 0xC9BA, 0xCCB4, 0xCFB7,
	0xD2C2, 0xD5D4, 0xD8ED, 0xDC0D, 0xDF32, 0xE25E, 0xE58D, 0xE8C2,
	0xEBFA, 0xEF36, 0xF274, 0xF5B5, 0xF8F8, 0xFC3B, 0xFF7F, 0x02C3,
	0x0607, 0x094B, 0x0C8C, 0x0FCA, 0x1308, 0x1641, 0x1977, 0x1CA9,
	0x1FD5, 0x22FD, 0x261D, 0x2938, 0x2C4D, 0x2F5A, 0x325F, 0x355C,
	0x3850, 0x3B3A, 0x3E1A, 0x40F1, 0x43BB, 0x467C, 0x4930, 0x4BD8,
	0x4E73, 0x5101, 0x5382, 0x55F5, 0x585A, 0x5AAF, 0x5CF6, 0x5F2D,
	0x6154, 0x636B, 0x6572, 0x6767, 0x694C, 0x6B1E, 0x6CDF, 0x6E8D,
	0x702A, 0x71B4, 0x732A, 0x748D, 0x75DD, 0x7719, 0x7841, 0x7957,
	0x7A56, 0x7B42, 0x7C19, 0x7CDC, 0x7D8A, 0x7E22, 0x7EA6, 0x7F14,
	0x7F6E, 0x7FB2, 0x7FE0, 0x7FFA, 0x7FFD, 0x7FED, 0x7FC7, 0x7F8A,
	0x7F39, 0x7ED3, 0x7E57, 0x7DC7, 0x7D20, 0x7C67, 0x7B97, 0x7AB3,
	0x79BB, 0x78AE, 0x778E, 0x7659, 0x7511, 0x73B4, 0x7245, 0x70C3,
	0x6F2E, 0x6D87, 0x6BCD, 0x6A02, 0x6823, 0x6634, 0x6435, 0x6224,
	0x6003, 0x5DD2, 0x5B90, 0x5941, 0x56E2, 0x5476, 0x51F9, 0x4F70,
	0x4CDA, 0x4A37, 0x4787, 0x44CC, 0x4205, 0x3F33, 0x3C56, 0x3970,
	0x3680, 0x3386, 0x3084, 0x2D7B, 0x2A69, 0x2750, 0x2431, 0x210C,
	0x1DE1, 0x1AB2, 0x177E, 0x1445, 0x110A, 0x0DCB, 0x0A8B, 0x0749,
	0x0405, 0x00C1, 0xFD7D, 0xFA38, 0xF6F5, 0xF3B4, 0xF074, 0xED37,
	0xE9FF, 0xE6C9, 0xE396, 0xE06A, 0xDD42, 0xDA21, 0xD704, 0xD3EF,
	0xD0E1, 0xCDDD, 0xCADF, 0xC7EA, 0xC4FE, 0xC21E, 0xBF47, 0xBC7B,
	0xB9BA, 0xB705, 0xB45C, 0xB1C0, 0xAF31, 0xACAF, 0xAA3B, 0xA7D6,
	0xA57F, 0xA337, 0xA0FE, 0x9ED6, 0x9CBE, 0x9AB5, 0x98BF, 0x96D9,
	0x9505, 0x9343, 0x9193, 0x8FF5, 0x8E6A, 0x8CF3, 0x8B8D, 0x8A3C,
	0x88FF, 0x87D3, 0x86BD, 0x85BC, 0x84D0, 0x83F6, 0x8333, 0x8283,
	0x81E9, 0x8164, 0x80F3, 0x8098, 0x8053, 0x8022, 0x8007, 0x8001,
	0x8011, 0x8036, 0x8071, 0x80C0, 0x8125, 0x819F, 0x822E, 0x82D2,
	0x838A, 0x8458, 0x853A, 0x8630, 0x873D, 0x885B, 0x898F, 0x8AD5,
	0x8C30, 0x8D9E, 0x8F1D, 0x90B1, 0x9258, 0x9410, 0x95DB, 0x97B7,
	0x99A4, 0x9BA3, 0x9DB3, 0x9FD3, 0xA203, 0xA441, 0xA691, 0xA8EF,
	0xAB5B, 0xADD6, 0xB05E, 0xB2F3, 0xB594, 0xB844, 0xBAFE, 0xBDC4,
	0xC095, 0xC372, 0xC656, 0xC947, 0xCC3F, 0xCF40, 0xD249, 0xD55B
};


static const UInt16 sample_tone200_48m[VIBRA_PERIOD_BYTES] = {
	0x0000, 0x035A, 0x06B3, 0x0A0B, 0x0D62, 0x10B6, 0x1406, 0x1753,
	0x1A9C, 0x1DE1, 0x2121, 0x245B, 0x278D, 0x2ABA, 0x2DDE, 0x30FC,
	0x3410, 0x371B, 0x3A1C, 0x3D12, 0x3FFF, 0x42E1, 0x45B6, 0x487F,
	0x4B3D, 0x4DEC, 0x508D, 0x5320, 0x55A5, 0x581B, 0x5A82, 0x5CD8,
	0x5F1F, 0x6154, 0x6379, 0x658C, 0x678D, 0x697D, 0x6B58, 0x6D23,
	0x6ED9, 0x707D, 0x720C, 0x7387, 0x74EE, 0x7641, 0x777F, 0x78A8,
	0x79BC, 0x7ABA, 0x7BA2, 0x7C76, 0x7D33, 0x7DDA, 0x7E6C, 0x7EE7,
	0x7F4B, 0x7F99, 0x7FD2, 0x7FF4, 0x7FFF, 0x7FF4, 0x7FD3, 0x7F99,
	0x7F4C, 0x7EE6, 0x7E6C, 0x7DDB, 0x7D33, 0x7C76, 0x7BA3, 0x7ABB,
	0x79BB, 0x78A7, 0x777F, 0x7641, 0x74EE, 0x7387, 0x720B, 0x707C,
	0x6ED9, 0x6D23, 0x6B58, 0x697C, 0x678D, 0x658C, 0x6379, 0x6154,
	0x5F1F, 0x5CD8, 0x5A82, 0x581B, 0x55A5, 0x5320, 0x508E, 0x4DEA,
	0x4B3D, 0x487F, 0x45B7, 0x42E1, 0x4000, 0x3D14, 0x3A1C, 0x371B,
	0x3410, 0x30FB, 0x2DDF, 0x2ABA, 0x278E, 0x245B, 0x2121, 0x1DE1,
	0x1A9D, 0x1753, 0x1406, 0x10B5, 0x0D61, 0x0A0B, 0x06B3, 0x0359,
	0x0000, 0xFCA5, 0xF94D, 0xF5F5, 0xF29F, 0xFE4B, 0xEBFA, 0xE8AC,
	0xE564, 0xE21E, 0xDEDF, 0xDBA5, 0xD872, 0xD546, 0xD222, 0xCF05,
	0xCBF1, 0xC8E5, 0xC5E4, 0xC2ED, 0xC001, 0xBD20, 0xBA4A, 0xB781,
	0xB4C4, 0xB214, 0xAF73, 0xACE0, 0xAA5B, 0xA7E5, 0xA57F, 0xA328,
	0xA0E1, 0x9EAC, 0x9C86, 0x9A74, 0x9873, 0x9685, 0x94A7, 0x92DD,
	0x9127, 0x8F84, 0x8DF5, 0x8C79, 0x8B12, 0x89BF, 0x8882, 0x8759,
	0x8645, 0x8546, 0x845D, 0x838A, 0x82CD, 0x8225, 0x8195, 0x8118,
	0x80B5, 0x8066, 0x802E, 0x800C, 0x8001, 0x800D, 0x802E, 0x8066,
	0x80B4, 0x8119, 0x8194, 0x8226, 0x82CD, 0x838A, 0x845D, 0x8546,
	0x8645, 0x8759, 0x8881, 0x89BF, 0x8B12, 0x8C78, 0x8DF4, 0x8F84,
	0x9127, 0x92DE, 0x94A8, 0x9684, 0x9873, 0x9A74, 0x9C87, 0x9EAC,
	0xA0E1, 0xA328, 0xA57E, 0xA7E5, 0xAA5A, 0xACE0, 0xAF73, 0xB215,
	0xB4C5, 0xB781, 0xBA49, 0xBD1F, 0xC000, 0xC2ED, 0xC5E4, 0xC8E6,
	0xCBF0, 0xCF04, 0xD221, 0xD547, 0xD873, 0xDBA6, 0xDEDF, 0xE21F,
	0xE563, 0xE8AD, 0xEBFA, 0xEf4B, 0xF29F, 0xF5F6, 0xF94D, 0xFCA7,
	0x0000, 0x035A, 0x06B3, 0x0A0B, 0x0D62, 0x10B6, 0x1406, 0x1753,
	0x1A9C, 0x1DE1, 0x2121, 0x245B, 0x278D, 0x2ABA, 0x2DDE, 0x30FC,
	0x3410, 0x371B, 0x3A1C, 0x3D12, 0x3FFF, 0x42E1, 0x45B6, 0x487F,
	0x4B3D, 0x4DEC, 0x508D, 0x5320, 0x55A5, 0x581B, 0x5A82, 0x5CD8,
	0x5F1F, 0x6154, 0x6379, 0x658C, 0x678D, 0x697D, 0x6B58, 0x6D23,
	0x6ED9, 0x707D, 0x720C, 0x7387, 0x74EE, 0x7641, 0x777F, 0x78A8,
	0x79BC, 0x7ABA, 0x7BA2, 0x7C76, 0x7D33, 0x7DDA, 0x7E6C, 0x7EE7,
	0x7F4B, 0x7F99, 0x7FD2, 0x7FF4, 0x7FFF, 0x7FF4, 0x7FD3, 0x7F99,
	0x7F4C, 0x7EE6, 0x7E6C, 0x7DDB, 0x7D33, 0x7C76, 0x7BA3, 0x7ABB,
	0x79BB, 0x78A7, 0x777F, 0x7641, 0x74EE, 0x7387, 0x720B, 0x707C,
	0x6ED9, 0x6D23, 0x6B58, 0x697C, 0x678D, 0x658C, 0x6379, 0x6154,
	0x5F1F, 0x5CD8, 0x5A82, 0x581B, 0x55A5, 0x5320, 0x508E, 0x4DEA,
	0x4B3D, 0x487F, 0x45B7, 0x42E1, 0x4000, 0x3D14, 0x3A1C, 0x371B,
	0x3410, 0x30FB, 0x2DDF, 0x2ABA, 0x278E, 0x245B, 0x2121, 0x1DE1,
	0x1A9D, 0x1753, 0x1406, 0x10B5, 0x0D61, 0x0A0B, 0x06B3, 0x0359,
	0x0000, 0xFCA5, 0xF94D, 0xF5F5, 0xF29F, 0xFE4B, 0xEBFA, 0xE8AC,
	0xE564, 0xE21E, 0xDEDF, 0xDBA5, 0xD872, 0xD546, 0xD222, 0xCF05,
	0xCBF1, 0xC8E5, 0xC5E4, 0xC2ED, 0xC001, 0xBD20, 0xBA4A, 0xB781,
	0xB4C4, 0xB214, 0xAF73, 0xACE0, 0xAA5B, 0xA7E5, 0xA57F, 0xA328,
	0xA0E1, 0x9EAC, 0x9C86, 0x9A74, 0x9873, 0x9685, 0x94A7, 0x92DD,
	0x9127, 0x8F84, 0x8DF5, 0x8C79, 0x8B12, 0x89BF, 0x8882, 0x8759,
	0x8645, 0x8546, 0x845D, 0x838A, 0x82CD, 0x8225, 0x8195, 0x8118,
	0x80B5, 0x8066, 0x802E, 0x800C, 0x8001, 0x800D, 0x802E, 0x8066,
	0x80B4, 0x8119, 0x8194, 0x8226, 0x82CD, 0x838A, 0x845D, 0x8546,
	0x8645, 0x8759, 0x8881, 0x89BF, 0x8B12, 0x8C78, 0x8DF4, 0x8F84,
	0x9127, 0x92DE, 0x94A8, 0x9684, 0x9873, 0x9A74, 0x9C87, 0x9EAC,
	0xA0E1, 0xA328, 0xA57E, 0xA7E5, 0xAA5A, 0xACE0, 0xAF73, 0xB215,
	0xB4C5, 0xB781, 0xBA49, 0xBD1F, 0xC000, 0xC2ED, 0xC5E4, 0xC8E6,
	0xCBF0, 0xCF04, 0xD221, 0xD547, 0xD873, 0xDBA6, 0xDEDF, 0xE21F,
	0xE563, 0xE8AD, 0xEBFA, 0xEf4B, 0xF29F, 0xF5F6, 0xF94D, 0xFCA7
};

static const UInt16 sample_tone205_48m[VIBRA_PERIOD_BYTES] = {
	0x0001, 0x036B, 0x06DD, 0x0A4B, 0x0DB2, 0x111D, 0x147E, 0x17E3,
	0x1B42, 0x1E9D, 0x21EC, 0x253A, 0x2882, 0x2BBC, 0x2EF4, 0x3224,
	0x3544, 0x3861, 0x3B73, 0x3E74, 0x4170, 0x4459, 0x473D, 0x4A12,
	0x4CD5, 0x4F8F, 0x523A, 0x54D1, 0x575D, 0x59D9, 0x5C3F, 0x5E9B,
	0x60E3, 0x6316, 0x653B, 0x674D, 0x694B, 0x6B34, 0x6D0B, 0x6ECE,
	0x707B, 0x7214, 0x7399, 0x7507, 0x7661, 0x77A5, 0x78D1, 0x79E9,
	0x7AEA, 0x7BD3, 0x7CA7, 0x7D62, 0x7E07, 0x7E96, 0x7F0C, 0x7F6B,
	0x7FB3, 0x7FE3, 0x7FFB, 0x7FFD, 0x7FE5, 0x7FB8, 0x7F73, 0x7F14,
	0x7E9F, 0x7E13, 0x7D70, 0x7CB7, 0x7BE5, 0x7AFC, 0x79FF, 0x78E8,
	0x77BB, 0x767C, 0x7523, 0x73B8, 0x7235, 0x709C, 0x6EF2, 0x6D30,
	0x6B5B, 0x6974, 0x6776, 0x6566, 0x6347, 0x6111, 0x5ECA, 0x5C75,
	0x5A0B, 0x5791, 0x550A, 0x5270, 0x4FC7, 0x4D13, 0x4A4C, 0x4777,
	0x449B, 0x41AC, 0x3EB2, 0x3BB1, 0x38A1, 0x3584, 0x3265, 0x2F36,
	0x2BFE, 0x28C5, 0x257E, 0x2230, 0x1EE1, 0x1B88, 0x1828, 0x14CB,
	0x1163, 0x0DF9, 0x0A92, 0x0722, 0x03B3, 0x0047, 0xFCD6, 0xF964,
	0xF5FB, 0xF28E, 0xEF23, 0xEBC1, 0xE85C, 0xE4FD, 0xE1A8, 0xDE53,
	0xDB03, 0xD7C1, 0xD481, 0xD147, 0xCE1E, 0xCAF6, 0xC7D9, 0xC4CC,
	0xC1C4, 0xBEC8, 0xBBDC, 0xB8F9, 0xB622, 0xB35F, 0xB0A4, 0xADF7,
	0xAB5B, 0xA8D2, 0xA655, 0xA3E8, 0xA191, 0x9F47, 0x9D0F, 0x9AED,
	0x98D9, 0x96D9, 0x94F1, 0x9317, 0x9152, 0x8FA5, 0x8E09, 0x8C82,
	0x8B13, 0x89B7, 0x8871, 0x8744, 0x862C, 0x8528, 0x843E, 0x8368,
	0x82A9, 0x8204, 0x8174, 0x80FB, 0x809C, 0x8052, 0x8020, 0x8006,
	0x8002, 0x8018, 0x8044, 0x8088, 0x80E4, 0x8156, 0x81E1, 0x8283,
	0x833B, 0x840B, 0x84F3, 0x85EF, 0x8703, 0x882E, 0x896C, 0x8AC3,
	0x8C2F, 0x8DAE, 0x8F46, 0x90F1, 0x92AE, 0x9484, 0x966B, 0x9863,
	0x9A73, 0x9C95, 0x9EC5, 0xA10B, 0xA363, 0xA5C6, 0xA840, 0xAACB,
	0xAD5F, 0xB007, 0xB2C0, 0xB581, 0xB854, 0xBB34, 0xBE1C, 0xC116,
	0xC41C, 0xC727, 0xCA41, 0xCD66, 0xD08E, 0xD3C5, 0xD704, 0xDA44,
	0xDD93, 0xE0E6, 0xE43A, 0xE799, 0xEAFC, 0xEE5D, 0xF1C8, 0xF534,
	0xF89D, 0xFC0E, 0xFF7F, 0x02EA, 0x065B, 0x09CC, 0x0D32, 0x109E,
	0x1406, 0x1764, 0x1AC5, 0x1E20, 0x216F, 0x24BF, 0x2808, 0x2B42,
	0x2E7C, 0x31AD, 0x34CF, 0x37ED, 0x3B01, 0x3E04, 0x4102, 0x43F2,
	0x46D2, 0x49A9, 0x4C72, 0x4F2A, 0x51D7, 0x5475, 0x56FE, 0x597D,
	0x5BEA, 0x5E43, 0x608F, 0x62C9, 0x64EC, 0x6701, 0x6902, 0x6AED,
	0x6CC8, 0x6E8E, 0x703C, 0x71D9, 0x7362, 0x74D2, 0x762F, 0x7777,
	0x78A6, 0x79C1, 0x7AC5, 0x7BB2, 0x7C8A, 0x7D49, 0x7DF1, 0x7E82,
	0x7EFC, 0x7F5F, 0x7FAA, 0x7FDD, 0x7FFA, 0x7FFD, 0x7FEB, 0x7FC0,
	0x7F7E, 0x7F24, 0x7EB3, 0x7E2A, 0x7D8B, 0x7CD4, 0x7C04, 0x7B21,
	0x7A25, 0x7912, 0x77EC, 0x76AC, 0x7556, 0x73EE, 0x726E, 0x70D9,
	0x6F32, 0x6D73, 0x6B9F, 0x69BD, 0x67C2, 0x65B4, 0x6398, 0x6165,
	0x5F20, 0x5CCE, 0x5A67, 0x57EE, 0x556B, 0x52D2, 0x502A, 0x4D79,
	0x4AB4, 0x47E2, 0x4507, 0x421A, 0x3F23, 0x3C23, 0x3914, 0x35F9,
	0x32DB, 0x2FAE, 0x2C77, 0x293F, 0x25F9, 0x22AC, 0x1F5E, 0x1C05,
	0x18A7, 0x154A, 0x11E3, 0x0E78, 0x0B12, 0x07A3, 0x0433, 0x00C7,
	0xFD56, 0xF9E5, 0xF67C, 0xF30F, 0xEFA2, 0xEC40, 0xE8DB, 0xE57A,
	0xE225, 0xDECE, 0xDB7F, 0xD83B, 0xD4F9, 0xD1BF, 0xCE94, 0xCB6C,
	0xC84D, 0xC53E, 0xC235, 0xBF36, 0xBC4A, 0xB965, 0xB68B, 0xB3C6,
	0xB109, 0xAE5B, 0xABC0, 0xA930, 0xA6B1, 0xA446, 0xA1E8, 0x9F9B,
	0x9D65, 0x9B3D, 0x9925, 0x9727, 0x9537, 0x935A, 0x9196, 0x8FE3,
	0x8E43, 0x8CBD, 0x8B48, 0x89E9, 0x88A2, 0x8770, 0x8653, 0x854F,
	0x845E, 0x8386, 0x82C5, 0x821A, 0x8187, 0x810C, 0x80A8, 0x805B,
	0x8026, 0x8008, 0x8001, 0x8013, 0x803C, 0x807D, 0x80D5, 0x8144,
	0x81CC, 0x8269, 0x831E, 0x83EC, 0x84CD, 0x85C8, 0x86D9, 0x87FE,
	0x893C, 0x8A8F, 0x8BF6, 0x8D75, 0x8F09, 0x90AE, 0x926C, 0x943E,
	0x9620, 0x9818, 0x9A25, 0x9C40, 0x9E71, 0xA0B6, 0xA306, 0xA56D,
	0xA7E4, 0xAA65, 0xACFC, 0xAFA3, 0xB254, 0xB517, 0xB7E8, 0xBAC3,
	0xBDAF, 0xC0A7, 0xC3A4, 0xC6B3, 0xC9CC, 0xCCEA, 0xD016, 0xD34D,
	0xD684, 0xD9CA, 0xDD16, 0xE063, 0xE3BC, 0xE71A, 0xEA77, 0xEDDE,
	0xF148, 0xF4AE, 0xF81D, 0xFB8D, 0xFEF8, 0x0269, 0x05DB, 0x0944,
	0x0CB2, 0x101F, 0x1380, 0x16E5, 0x1A47, 0x1D9C, 0x20F3, 0x2444
};


void S2_vibtonz_en(bool on_off)
{
	BRCM_AUDIO_Param_Vibra_t parm_vibra = {0};

	aTrace(LOG_AUDIO_CNTLR,
		"S2_vibtonz_en: on_off= %d\n", on_off);

	if (on_off) {
		parm_vibra.strength = s_vibra_strength;
		AUDIO_Ctrl_Trigger(ACTION_AUD_EnableByPassVibra,
			&parm_vibra, NULL, 0);
	} else {
		AUDIO_Ctrl_Trigger(ACTION_AUD_DisableByPassVibra, NULL,
			NULL, 0);
	}
}

void S2_vibtonz_pwm(int nForce)
{
	UInt16 *sample_tone_p;
	int scale_gain = 0;
	int sample_i = 0;
	Int32 sample = 0;

	aTrace(LOG_AUDIO_CNTLR,
	"S2_vibtonz_pwm: strength %d => %d\n", s_vibra_strength, nForce);

	if (s_vibra_strength == nForce)
		return;

	s_vibra_strength = nForce;

	if (s_vibra_dma_bufp == NULL)
		return;

	/* map strength to scale gain and apply to PCM wave table */
	scale_gain = s_vibra_strength;
	aTrace(LOG_AUDIO_CNTLR, "S2_vibtonz_pwm: scale gain = %d\n",
					scale_gain);
	sample_tone_p = (UInt16 *)(s_vibra_dma_bufp);
	while (sample_i < VIBRA_PERIOD_BYTES) {
		sample = *((Int16 *)(sample_tone_p + sample_i));
		*(sample_tone_p + sample_i)
			= sample*scale_gain/VIBRA_STRENGTH_RANGE;
		sample_i++;
	}


#if 0
	BRCM_AUDIO_Param_Vibra_t parm_vibra = {0};
	/* restart the vibration with the new strength*/
	AUDIO_Ctrl_Trigger(ACTION_AUD_DisableByPassVibra, NULL,
		NULL, 0);
	parm_vibra.strength = s_vibra_strength;
	AUDIO_Ctrl_Trigger(ACTION_AUD_EnableByPassVibra,
		&parm_vibra, NULL, 0);
#endif
}

static void AUDIO_DRIVER_PCM_Vibra_InterruptPeriodCB(void *pPrivate)
{
	unsigned long flags;
	/*static int count;

	if (count%10 == 0)
		aTrace(LOG_AUDIO_CNTLR,
		"PCM_Vibra_InterruptPeriodCB: count = %d\n", count);
	count++;*/

	spin_lock_irqsave(&vibra_drv_lock, flags);
	if (pcm_vibra_drv_handle)
		AUDIO_DRIVER_Ctrl(pcm_vibra_drv_handle,
			AUDIO_DRIVER_BUFFER_READY, NULL);
	spin_unlock_irqrestore(&vibra_drv_lock, flags);
}

static void PCM_Vibra_Gen_Start(Int32 strength)
{
	unsigned long period_bytes;
	unsigned long num_blocks;
	unsigned long period_ms;
	unsigned long flags;
	UInt16 *sample_tone_p;

	static AUDIO_DRIVER_CONFIG_t drv_config;
	static AUDIO_DRIVER_BUFFER_t pcm_vibra_buf_param;
	static dma_addr_t dma_addr;
	static AUDIO_SINK_Enum_t spkr;
	CSL_CAPH_DEVICE_e aud_dev;
	AUDIO_DRIVER_CallBackParams_t cbParams;

	sample_tone_p = &sample_tone200_48m[0];

	/* open the plyabck device */

	aTrace(LOG_AUDIO_CNTLR, "Vibra Audio DDRIVER Open\n");

	spin_lock_irqsave(&vibra_drv_lock, flags);
	pcm_vibra_drv_handle = AUDIO_DRIVER_Open(AUDIO_DRIVER_PLAY_AUDIO);
	spin_unlock_irqrestore(&vibra_drv_lock, flags);

	aTrace(LOG_AUDIO_CNTLR, "Vibra Audio DDRIVER Config\n");

	/* set the callback */
	cbParams.pfCallBack =
	    AUDIO_DRIVER_PCM_Vibra_InterruptPeriodCB;
	cbParams.pPrivateData = (void *)pcm_vibra_drv_handle;
	AUDIO_DRIVER_Ctrl(pcm_vibra_drv_handle, AUDIO_DRIVER_SET_CB,
			  (void *)&cbParams);

	/* configure defaults */

	drv_config.sample_rate =
		    AUDIO_SAMPLING_RATE_48000;

	drv_config.num_channel = AUDIO_CHANNEL_MONO;
	drv_config.bits_per_sample = 16;

	aTrace(LOG_AUDIO_CNTLR, "Config:sr=%u nc=%d bs=%d\n",
			drv_config.sample_rate,
			drv_config.num_channel,
			drv_config.bits_per_sample);

	AUDIO_DRIVER_Ctrl(pcm_vibra_drv_handle, AUDIO_DRIVER_CONFIG,
			  (void *)&drv_config);

	/* set the interrupt period */
	period_bytes = VIBRA_PERIOD_BYTES;
	num_blocks = 2;
	period_ms = (period_bytes * 1000) /
			(drv_config.num_channel * 2 * drv_config.sample_rate);
	aTrace(LOG_AUDIO_CNTLR,
			"Period: ms=%ld bytes=%ld blocks:%ld\n",
			period_ms, period_bytes, num_blocks);
	AUDIO_DRIVER_Ctrl(pcm_vibra_drv_handle,
			  AUDIO_DRIVER_SET_INT_PERIOD,
			  (void *)&period_bytes);
	pcm_vibra_buf_param.buf_size = period_bytes * num_blocks;
	if (pcm_vibra_buf_param.pBuf == NULL) {
		s_vibra_dma_bufp = pcm_vibra_buf_param.pBuf =
			dma_alloc_coherent(NULL, pcm_vibra_buf_param.buf_size,
			       &dma_addr, GFP_KERNEL);
	}
	if (pcm_vibra_buf_param.pBuf == NULL) {
		aError("Cannot allocate Buffer for Vibra\n");
		return;
	}
	pcm_vibra_buf_param.phy_addr = (UInt32) dma_addr;

	aTrace(LOG_AUDIO_CNTLR,
			"virt_addr = 0x%x phy_addr=0x%x\n",
			(unsigned int)pcm_vibra_buf_param.pBuf,
			(unsigned int)dma_addr);

	memcpy(pcm_vibra_buf_param.pBuf,
		(char *)(sample_tone_p),
			pcm_vibra_buf_param.buf_size);

	/* set the buffer params */
	AUDIO_DRIVER_Ctrl(pcm_vibra_drv_handle,
			  AUDIO_DRIVER_SET_BUF_PARAMS,
			  (void *)&pcm_vibra_buf_param);

	/* Start the playback */
	spkr = /*AUDIO_SINK_LOUDSPK;*/ AUDIO_SINK_VIBRA;
	aud_dev = /*CSL_CAPH_DEV_IHF;*/ CSL_CAPH_DEV_VIBRA;
	aTrace(LOG_AUDIO_CNTLR, " Start Vibra Playback\n");

	AUDCTRL_EnablePlay(AUDIO_SOURCE_MEM,
			   spkr,
			   drv_config.num_channel,
			   drv_config.sample_rate, &pcm_vibra_path_id);

	AUDIO_DRIVER_Ctrl(pcm_vibra_drv_handle, AUDIO_DRIVER_START,
			  &aud_dev);
	aTrace(LOG_AUDIO_CNTLR, "Vibra Playback started\n");

}



static void PCM_Vibra_Gen_Stop(void)
{
	unsigned long flags;

	aTrace(LOG_AUDIO_CNTLR, " Stop Vibra playback\n");

	AUDIO_DRIVER_Ctrl(pcm_vibra_drv_handle, AUDIO_DRIVER_STOP, NULL);

	/* disable the playback path */
	AUDCTRL_DisablePlay(AUDIO_SOURCE_MEM,
				AUDIO_SINK_VIBRA,
				pcm_vibra_path_id);

	spin_lock_irqsave(&vibra_drv_lock, flags);
	AUDIO_DRIVER_Close(pcm_vibra_drv_handle);
	pcm_vibra_drv_handle = NULL;
	spin_unlock_irqrestore(&vibra_drv_lock, flags);
}

#endif

