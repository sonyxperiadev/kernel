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
	"SetPrePareParameters",	/* 10 */
	"AddChannel",
	"RemoveChannel",
	"EnableTelephony",
	"DisableTelephony",
	"EnableECNSTelephony",
	"DisableECNSTelephony",
	"SetTelephonyMicSpkr",
	"MutePlayback",
	"MuteRecord",
	"MuteTelephony",	/* 20 */
	"EnableByPassVibra",
	"DisableByPassVibra",
	"SetPlaybackVolume",
	"SetRecordGain",
	"SetTelephonySpkrVolume",
	"SwitchSpkr",
	"SetHWLoopback",
	"SetAudioMode",
	"SetAudioApp",		/*set audio profile */
	"EnableFMPlay",
	"DisableFMPlay",
	"SetARM2SPInst",
	"RateChange",		/*33 */
	"AmpEnable"
};
#endif
static unsigned int pathID[CAPH_MAX_PCM_STREAMS];
static unsigned int n_msg_in, n_msg_out, last_action;
static struct completion complete_kfifo;
static struct TAudioHalThreadData sgThreadData;

#define KFIFO_SIZE		2048
#define BLOCK_WAITTIME_MS	60000
#define KFIFO_TIMEOUT_MS	60000

static void AUDIO_Ctrl_Process(BRCM_AUDIO_ACTION_en_t action_code,
			       void *arg_param, void *callback, int block);
#ifdef CONFIG_AUDIO_S2
static void PCM_Vibra_Gen_Start(void);
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

		/* process the operation */
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
	init_completion(&complete_kfifo);

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
	case ACTION_AUD_HandleCPReset:
		size = sizeof(BRCM_AUDIO_Param_cpReset_t);
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
			ret =
			    wait_for_completion_interruptible_timeout
			    (&complete_kfifo,
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
	queue_work_on(1, sgThreadData.pWorkqueue_AudioControl,
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
				return status;
			if (arg_param)
				memcpy(arg_param, &msgAudioCtrl.param,
				       params_size);
		}
	}

	return status;
}

static void AUDIO_Ctrl_Process(BRCM_AUDIO_ACTION_en_t action_code,
			       void *arg_param, void *callback, int block)
{
	TMsgAudioCtrl msgAudioCtrl;
	unsigned int len;
	int i;
	unsigned int path;

	/*aTrace(LOG_AUDIO_CNTLR,
		"AUDIO_Ctrl_Process action_code=%d %s\n", action_code,
		&action_names[action_code][0]);*/

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
				param_start->rate, &path);
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
				AUDCTRL_DisablePlay(param_stop->pdev_prop->p[0].
						    source,
						    param_stop->pdev_prop->p[0].
						    sink,
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
			AUDCTRL_EnableTelephony(parm_call->new_mic,
						parm_call->new_spkr);
		}
		break;

	case ACTION_AUD_DisableTelephony:
		AUDCTRL_DisableTelephony();
		break;

	case ACTION_AUD_SetTelephonyMicSpkr:
		{
			BRCM_AUDIO_Param_Call_t *parm_call =
			    (BRCM_AUDIO_Param_Call_t *) arg_param;
			AUDCTRL_SetTelephonyMicSpkr(parm_call->new_mic,
						    parm_call->new_spkr);
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
			PCM_Vibra_Gen_Start();
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
			AUDCTRL_SetTelephonySpkrVolume(parm_vol->sink,
						       parm_vol->volume1,
						       parm_vol->gain_format);
		}
		break;

	case ACTION_AUD_SwitchSpkr:
		{
			BRCM_AUDIO_Param_Spkr_t *parm_spkr =
			    (BRCM_AUDIO_Param_Spkr_t *) arg_param;
			aTrace(LOG_AUDIO_CNTLR,
			       "\n %lx:AUDIO_Ctrl_Process-"
			       "ACTION_AUD_SwitchSpkr. stream=%d\n",
			       jiffies, parm_spkr->stream);
			CAPH_ASSERT(parm_spkr->stream >=
				    (CTL_STREAM_PANEL_FIRST - 1)
				    && parm_spkr->stream <
				    (CTL_STREAM_PANEL_LAST - 1));
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
			CAPH_ASSERT(parm_FM->stream >=
				    (CTL_STREAM_PANEL_FIRST - 1)
				    && parm_FM->stream <
				    (CTL_STREAM_PANEL_LAST - 1));

			/*
			 * do not change voice call's audio mode. will delete
			 * the lines
			 *
			 * can set music app and mode
			 */
			/* re-enable FM; need to fill audio app */
			AUDCTRL_SaveAudioApp(AUDIO_APP_FM_RADIO);
			AUDCTRL_SaveAudioMode((AudioMode_t) parm_FM->sink);

			AUDCTRL_EnablePlay(parm_FM->source,
					   parm_FM->sink,
					   AUDIO_CHANNEL_STEREO,
					   AUDIO_SAMPLING_RATE_48000, &path);

			pathID[parm_FM->stream] = path;
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
			AUDCTRL_DisablePlay(parm_FM->source, parm_FM->sink,
					    pathID[parm_FM->stream]);
			pathID[parm_FM->stream] = 0;
			AUDCTRL_SetUserAudioApp(AUDIO_APP_MUSIC);
		}
		break;
	case ACTION_AUD_SetARM2SPInst:
		{
			BRCM_AUDIO_Param_FM_t *parm_FM =
			    (BRCM_AUDIO_Param_FM_t *) arg_param;
			AUDCTRL_SetArm2spParam(parm_FM->fm_mix, TRUE);
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
			AUDCTRL_SetTelephonyMicMute(parm_mute->source,
						    parm_mute->mute1);
		}
		break;

	case ACTION_AUD_DisableECNSTelephony:
		aTrace(LOG_AUDIO_CNTLR, "Telephony : Turning Off EC and NS\n");
#ifdef AUDIO_FEATURE_SET_DISABLE_ECNS

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

			AUDCTRL_SetUserAudioApp(parm_setapp->aud_app);
		}
		break;
	case ACTION_AUD_AMPEnable:
		{
			BRCM_AUDIO_Param_AMPCTL_t *parm_setamp =
			    (BRCM_AUDIO_Param_AMPCTL_t *) arg_param;
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


#ifdef CONFIG_AUDIO_S2

static unsigned int pcm_vibra_path_id;
static AUDIO_DRIVER_HANDLE_t pcm_vibra_drv_handle;
/* 10ms*/
#define PERIOD_BYTES 480
static UInt16 sample_tone200_48m[PERIOD_BYTES] = {
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



static void AUDIO_DRIVER_PCM_Vibra_InterruptPeriodCB(void *pPrivate)
{
	static int count;
	count++;
	/*aTrace(LOG_AUDIO_DRIVER,
		"PCM_Vibra_InterruptPeriodCB: count = %d\n", count);*/
}

static void PCM_Vibra_Gen_Start(void)
{
	unsigned long period_bytes;
	unsigned long num_blocks;
	unsigned long period_ms;

	static AUDIO_DRIVER_CONFIG_t drv_config;
	static AUDIO_DRIVER_BUFFER_t pcm_vibra_buf_param;
	static dma_addr_t dma_addr;
	static AUDIO_SINK_Enum_t spkr;
	CSL_CAPH_DEVICE_e aud_dev;
	AUDIO_DRIVER_CallBackParams_t cbParams;

	/* open the plyabck device */

	aTrace(LOG_AUDIO_DRIVER, " Audio DDRIVER Open\n");
	pcm_vibra_drv_handle = AUDIO_DRIVER_Open(AUDIO_DRIVER_PLAY_AUDIO);

	aTrace(LOG_AUDIO_DRIVER, " Audio DDRIVER Config\n");

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

	aTrace(LOG_AUDIO_DRIVER, "Config:sr=%u nc=%d bs=%d\n",
			drv_config.sample_rate,
			drv_config.num_channel,
			drv_config.bits_per_sample);

	AUDIO_DRIVER_Ctrl(pcm_vibra_drv_handle, AUDIO_DRIVER_CONFIG,
			  (void *)&drv_config);
			  
			 
	/* set the interrupt period */
	period_bytes = PERIOD_BYTES;
	num_blocks = 2;
	period_ms = (period_bytes * 1000) /
			(drv_config.num_channel * 2 * drv_config.sample_rate);
	aTrace(LOG_AUDIO_DRIVER,
			"Period: ms=%ld bytes=%ld blocks:%ld\n",
			period_ms, period_bytes, num_blocks);
	AUDIO_DRIVER_Ctrl(pcm_vibra_drv_handle,
			  AUDIO_DRIVER_SET_INT_PERIOD,
			  (void *)&period_bytes);
	pcm_vibra_buf_param.buf_size = period_bytes * num_blocks;
	if (pcm_vibra_buf_param.pBuf == NULL) {
		pcm_vibra_buf_param.pBuf =
			dma_alloc_coherent(NULL, pcm_vibra_buf_param.buf_size,
			       &dma_addr, GFP_KERNEL);
	}
	if (pcm_vibra_buf_param.pBuf == NULL) {
		aTrace(LOG_AUDIO_DRIVER,
				"Cannot allocate Buffer\n");
		return;
	}
	pcm_vibra_buf_param.phy_addr = (UInt32) dma_addr;

	aTrace(LOG_AUDIO_DRIVER,
			"virt_addr = 0x%x phy_addr=0x%x\n",
			(unsigned int)pcm_vibra_buf_param.pBuf,
			(unsigned int)dma_addr);

	memcpy(pcm_vibra_buf_param.pBuf,
		(char *)(&sample_tone200_48m[0]),
			pcm_vibra_buf_param.buf_size);

	/* set the buffer params */
	AUDIO_DRIVER_Ctrl(pcm_vibra_drv_handle,
			  AUDIO_DRIVER_SET_BUF_PARAMS,
			  (void *)&pcm_vibra_buf_param);

	/* Start the playback */
	spkr = AUDIO_SINK_VIBRA;
	aud_dev = CSL_CAPH_DEV_VIBRA;
	aTrace(LOG_AUDIO_DRIVER, " Start Playback\n");

	AUDCTRL_EnablePlay(AUDIO_SOURCE_MEM,
			   spkr,
			   drv_config.num_channel,
			   drv_config.sample_rate, &pcm_vibra_path_id);

	AUDCTRL_SetPlayVolume(AUDIO_SOURCE_MEM, spkr,
			      AUDIO_GAIN_FORMAT_mB, 0x00, 0x00,
			      0);

	AUDIO_DRIVER_Ctrl(pcm_vibra_drv_handle, AUDIO_DRIVER_START,
			  &aud_dev);
	aTrace(LOG_AUDIO_DRIVER, "Playback started\n");

}



static void PCM_Vibra_Gen_Stop(void)
{
	aTrace(LOG_AUDIO_DRIVER, " Stop playback\n");

	AUDIO_DRIVER_Ctrl(pcm_vibra_drv_handle, AUDIO_DRIVER_STOP, NULL);

	/* disable the playback path */
	AUDCTRL_DisablePlay(AUDIO_SOURCE_MEM,
				AUDIO_SINK_VIBRA,
				pcm_vibra_path_id);

	AUDIO_DRIVER_Close(pcm_vibra_drv_handle);
}

#endif

