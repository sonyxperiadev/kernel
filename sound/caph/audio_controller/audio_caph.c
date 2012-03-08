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
		"AmpEnable"
};

static unsigned int pathID[CAPH_MAX_PCM_STREAMS];

static struct TAudioHalThreadData sgThreadData;
#define	KFIFO_SIZE		1024 /*(9*sizeof(TMsgAudioCtrl))*/
#define WAIT_TIME		3000		/* in msec */

static void AUDIO_Ctrl_Process(BRCM_AUDIO_ACTION_en_t action_code,
			void *arg_param, void *callback, int block);

#ifdef USE_HR_TIMER
static enum hrtimer_restart TimerCbStopVibrator(struct hrtimer *timer)
{
	AUDIO_Ctrl_Trigger(ACTION_AUD_DisableByPassVibra, NULL, NULL, 0);

	aTrace(LOG_AUDIO_CNTLR, "Disable Vib from HR Timer  cb\n");

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

		/* process the operation */
		AUDIO_Ctrl_Process(msgAudioCtrl.action_code,
				   &msgAudioCtrl.param,
				   msgAudioCtrl.pCallBack, msgAudioCtrl.block);
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
	default:
		break;
	}
	return size;
}

/** HAL_AUDIO_Ctrl: Client call this function to execute audio HAL functions.
 *        This function forward the message to worker thread to do actual work
 */
Result_t AUDIO_Ctrl_Trigger(BRCM_AUDIO_ACTION_en_t action_code,
			    void *arg_param, void *callback, int block)
{
	TMsgAudioCtrl msgAudioCtrl;
	Result_t status = RESULT_OK;
	unsigned int len;
	OSStatus_t osStatus;
	int params_size = AUDIO_Ctrl_Trigger_GetParamsSize(action_code);
	unsigned long to_jiff = msecs_to_jiffies(WAIT_TIME);

	aTrace(LOG_AUDIO_CNTLR, "AudioHalThread action=%d\r\n", action_code);

	msgAudioCtrl.action_code = action_code;
	if (arg_param)
		memcpy(&msgAudioCtrl.param, arg_param, params_size);
	else
		memset(&msgAudioCtrl.param, 0, params_size);
	msgAudioCtrl.pCallBack = callback;
	msgAudioCtrl.block = block;

	len =
	    kfifo_in_locked(&sgThreadData.m_pkfifo,
			    (unsigned char *)&msgAudioCtrl,
			    sizeof(TMsgAudioCtrl), &sgThreadData.m_lock);
	if (len != sizeof(TMsgAudioCtrl))
		aError
		    ("Error AUDIO_Ctrl_Trigger len=%d expected %d\n", len,
		     sizeof(TMsgAudioCtrl));

	queue_work(sgThreadData.pWorkqueue_AudioControl, &sgThreadData.mwork);
	if (block) {
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

	aTrace(LOG_AUDIO_CNTLR,
		"AUDIO_Ctrl_Process action_code=%d %s", action_code,
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

			CAPH_ASSERT(param_start->stream >=
				    (CTL_STREAM_PANEL_FIRST - 1)
				    && param_start->stream <
				    (CTL_STREAM_PANEL_LAST - 1));
			if (param_start->pdev_prop->p[0].drv_type ==
			    AUDIO_DRIVER_PLAY_AUDIO) {

				/* Enable the playback the path */
				AUDCTRL_EnablePlay(
					param_start->pdev_prop->p[0].source,
					param_start->pdev_prop->p[0].sink,
					param_start->channels,
					param_start->rate, &path);
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

			CAPH_ASSERT(param_start->stream >=
				    (CTL_STREAM_PANEL_FIRST - 1)
				    && param_start->stream <
				    (CTL_STREAM_PANEL_LAST - 1));

			/* allow FM recording in call mode */
			/*
			 * AUDCTRL_EnableRecord enables HW path, reads SYSPARM
			 * and sets HW gains as defined in SYSPARM.
			 */
			if ((param_start->callMode != 1) ||
			    (param_start->pdev_prop->c.source ==
			     AUDIO_SOURCE_I2S)) {
				AUDCTRL_EnableRecord(param_start->pdev_prop->c.
						     source,
						     param_start->pdev_prop->c.
						     sink,
						     param_start->channels,
						     param_start->rate, &path);

				pathID[param_start->stream] = path;
			}

			if (param_start->pdev_prop->c.drv_type ==
			    AUDIO_DRIVER_CAPT_HQ)
				AUDIO_DRIVER_Ctrl(param_start->drv_handle,
						  AUDIO_DRIVER_START,
						  &param_start->pdev_prop->c.
						  source);
			else
				AUDIO_DRIVER_Ctrl(param_start->drv_handle,
						  AUDIO_DRIVER_START,
						  &param_start->mixMode);

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

			if ((param_stop->callMode != 1) ||
			    (param_stop->pdev_prop->c.source ==
			     AUDIO_SOURCE_I2S)) {
				/* allow FM recording in call mode */
				AUDCTRL_DisableRecord(param_stop->pdev_prop->c.
						      source,
						      param_stop->pdev_prop->c.
						      sink,
						      pathID[param_stop->
							     stream]);
				pathID[param_stop->stream] = 0;
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
			AUDCTRL_AddPlaySpk(parm_spkr->src, parm_spkr->sink,
					   pathID[parm_spkr->stream]);
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
				ktime = ktime_set(0,
				(parm_vibra->duration*1000000));

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
			AUDCTRL_SaveAudioApp(AUDIO_APP_FM);
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

	case ACTION_AUD_SetAudioApp:
		{
			BRCM_AUDIO_Param_SetApp_t *parm_setapp =
				(BRCM_AUDIO_Param_SetApp_t *)arg_param;
			AUDCTRL_SetUserAudioApp(parm_setapp->aud_app);
		}
		break;
	case ACTION_AUD_AMPEnable:
		{
			BRCM_AUDIO_Param_AMPCTL_t *parm_setamp =
				(BRCM_AUDIO_Param_AMPCTL_t *)arg_param;
			AUDCTRL_EnableAmp(parm_setamp->amp_status);


		}
		break;
	default:
		aError
		    ("Error AUDIO_Ctrl_Process Invalid acction command\n");
		break;
	}

	if (block) {
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

}
