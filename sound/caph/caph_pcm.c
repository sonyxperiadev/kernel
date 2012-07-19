/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */
/*******************************************************************************
Copyright 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement
governing use of this software, this software is licensed to you under the
terms of the GNU General Public License version 2, available at
http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software
in any way with any other Broadcom software provided under a license other than
the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/jiffies.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/wait.h>
#include <linux/delay.h>
#include <linux/moduleparam.h>
#include <linux/sched.h>

#include <sound/core.h>
#include <sound/control.h>
#include <sound/pcm_params.h>
#include <sound/pcm.h>
#include <sound/rawmidi.h>
#include <sound/initval.h>

#include "mobcom_types.h"
#include "resultcode.h"
#include "audio_consts.h"

#include "bcm_fuse_sysparm_CIB.h"
#include "csl_caph.h"
#include "audio_vdriver.h"
#include "audio_controller.h"
#include "audio_ddriver.h"
#include "auddrv_audlog.h"
#include "audio_caph.h"
#include "caph_common.h"
#include "audio_trace.h"

#include "csl_dsp.h"
#include "osdw_dsp_drv.h"
#include "csl_audio_render.h"

#define VOICE_CALL_SUB_DEVICE 8

#define	NUM_PLAYBACK_SUBDEVICE	3
#define	NUM_CAPTURE_SUBDEVICE	2

/* limitation for RHEA - only two blocks */
#define	PCM_MAX_PLAYBACK_BUF_BYTES			(64*1024)
#define	PCM_MIN_PLAYBACK_PERIOD_BYTES		(256)
#define	PCM_MAX_PLAYBACK_PERIOD_BYTES		(PCM_MAX_PLAYBACK_BUF_BYTES/2)

#define	PCM_MAX_VOICE_PLAYBACK_BUF_BYTES		(65*1024)
#define	PCM_MIN_VOICE_PLAYBACK_PERIOD_BYTES		(2560)

#define	PCM_MAX_VOICE_PLAYBACK_PERIOD_BYTES		\
(PCM_MAX_PLAYBACK_BUF_BYTES/2)

#define	PCM_MAX_CAPTURE_BUF_BYTES       (16 * 1024)
#define	PCM_MIN_CAPTURE_PERIOD_BYTES    (4 * 1024)	/*(16 * 1024) */
#define	PCM_MAX_CAPTURE_PERIOD_BYTES    (PCM_MAX_CAPTURE_BUF_BYTES/2)

#define	PCM_MAX_VOICE_CAPTURE_BUF_BYTES       (15360)
#define	PCM_MIN_VOICE_CAPTURE_PERIOD_BYTES    (320 * 4)

#define	PCM_MAX_VOICE_CAPTURE_PERIOD_BYTES    \
(PCM_MIN_VOICE_CAPTURE_PERIOD_BYTES * 2)

#define	PCM_TOTAL_BUF_BYTES	\
(PCM_MAX_CAPTURE_BUF_BYTES+PCM_MAX_VOICE_PLAYBACK_BUF_BYTES)

static void AUDIO_DRIVER_InterruptPeriodCB(void *pPrivate);
static void AUDIO_DRIVER_CaptInterruptPeriodCB(void *pPrivate);

static unsigned int pcm_voice_capture_period_bytes[] = {
	PCM_MIN_VOICE_CAPTURE_PERIOD_BYTES, PCM_MAX_VOICE_CAPTURE_PERIOD_BYTES
};
static struct snd_pcm_hw_constraint_list
pcm_voice_capture_period_bytes_constraints_list = {
.count = ARRAY_SIZE(pcm_voice_capture_period_bytes),
.list = pcm_voice_capture_period_bytes,
.mask = 0,
};

/* hardware definition */
static struct snd_pcm_hardware brcm_playback_hw = {
	.info = (SNDRV_PCM_INFO_MMAP | SNDRV_PCM_INFO_INTERLEAVED |
		 SNDRV_PCM_INFO_BLOCK_TRANSFER | SNDRV_PCM_INFO_MMAP_VALID |
		 SNDRV_PCM_INFO_PAUSE),
	.formats = SNDRV_PCM_FMTBIT_S16_LE,
	.rates =
	    (SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_44100 |
	     SNDRV_PCM_RATE_48000),
	.rate_min = 8000,
	.rate_max = 48000,
	.channels_min = 1,
	.channels_max = 2,
	.buffer_bytes_max = PCM_MAX_PLAYBACK_BUF_BYTES,
	.period_bytes_min = PCM_MIN_PLAYBACK_PERIOD_BYTES,
	.period_bytes_max = PCM_MAX_PLAYBACK_PERIOD_BYTES,
	.periods_min = 2,
	.periods_max = 2,	/*limitation for RHEA */
};

static struct snd_pcm_hardware brcm_capture_hw = {
	.info = (SNDRV_PCM_INFO_MMAP | SNDRV_PCM_INFO_INTERLEAVED |
		 SNDRV_PCM_INFO_BLOCK_TRANSFER | SNDRV_PCM_INFO_MMAP_VALID),
	.formats = SNDRV_PCM_FMTBIT_S16_LE,
	.rates =
	    (SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_44100 |
	     SNDRV_PCM_RATE_48000),
	.rate_min = 8000,
	.rate_max = 48000,
	.channels_min = 1,
	.channels_max = 2,
	.buffer_bytes_max = PCM_MAX_CAPTURE_BUF_BYTES,
	.period_bytes_min = PCM_MIN_CAPTURE_PERIOD_BYTES,
	.period_bytes_max = PCM_MAX_CAPTURE_PERIOD_BYTES,
	.periods_min = 2,
	.periods_max = 2,
};

static struct snd_pcm_hardware brcm_voice_capture_hw = {
	.info = (SNDRV_PCM_INFO_MMAP | SNDRV_PCM_INFO_INTERLEAVED |
		 SNDRV_PCM_INFO_BLOCK_TRANSFER | SNDRV_PCM_INFO_MMAP_VALID),
	.formats = SNDRV_PCM_FMTBIT_S16_LE,
	.rates = (SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000),
	.rate_min = 8000,
	.rate_max = 16000,
	.channels_min = 1,
	.channels_max = 1,
	/* one second data */
	.buffer_bytes_max = PCM_MAX_VOICE_CAPTURE_BUF_BYTES,
	/* one AMR brocks (each is 4 AMR frames) for pingpong,
	 * each blocks is 80 ms, 8000*0.020*2=320
	 */
	.period_bytes_min = PCM_MIN_VOICE_CAPTURE_PERIOD_BYTES,
	.period_bytes_max = PCM_MAX_VOICE_CAPTURE_PERIOD_BYTES,	/*half buffer */
	.periods_min = 2,
	.periods_max =
	    PCM_MAX_VOICE_CAPTURE_BUF_BYTES /
	    PCM_MIN_VOICE_CAPTURE_PERIOD_BYTES,
};

static Int32 callMode;
#define MAX_32 0xFFFFFFFF
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 *  Function Name: PcmHwParams
 *
 *  Description: Set hardware parameters
 *
 *------------------------------------------------------------
 */
static int PcmHwParams(struct snd_pcm_substream *substream,
		       struct snd_pcm_hw_params *hw_params)
{
/*DEBUG("\n %lx:hw_params %d\n",jiffies,(int)substream->stream);
 */

	aTrace
	    (LOG_ALSA_INTERFACE, "ALSA-CAPH params_access=%d params_format=%d,"
	     "params_subformat=%d params_channels=%d,"
	     "params_rate=%d, buffer bytes=%d\n",
	     params_access(hw_params), params_format(hw_params),
	     params_subformat(hw_params), params_channels(hw_params),
	     params_rate(hw_params), params_buffer_bytes(hw_params));

	return snd_pcm_lib_malloc_pages(substream,
					params_buffer_bytes(hw_params));
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 *  Function Name: PcmHwFree
 *
 *  Description: Release hardware resource
 *
 *------------------------------------------------------------
 */
static int PcmHwFree(struct snd_pcm_substream *substream)
{
	int res;
	brcm_alsa_chip_t *chip = snd_pcm_substream_chip(substream);
	int substream_number = substream->number;
	struct completion *compl_ptr;

	if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
		substream_number += CTL_STREAM_PANEL_PCMIN - 1;

	/*DEBUG("\n %lx:hw_free - stream=%lx\n",
	 * jiffies, (UInt32) substream);
	 */
	/*flush the wait queue in case pending events in the queue
	 * are processed after device close
	 * Below function is not required as audio uses its own queue
	 */
	/* flush_scheduled_work(); */
	aTrace(LOG_ALSA_INTERFACE, "PcmHwFree:completion %p stream #%d\n",
		chip->streamCtl[substream_number].pStopCompletion,
		substream->number);
	compl_ptr = chip->streamCtl[substream_number].pStopCompletion;
	if (compl_ptr) {
		long ret;
		#define	TIMEOUT_STOP_REQ_MS	30000
		ret = wait_for_completion_interruptible_timeout(compl_ptr,
			msecs_to_jiffies(TIMEOUT_STOP_REQ_MS));
		/* The return value is -ERESTARTSYS if interrupted,      */
		/* 0 if timed out, positive (at least 1, or number of jiffies*/
		/* left till timeout) if completed.                         */
		WARN(ret <= 0, "ERROR timeout waiting for STOP REQ."
				"t=%d ret=%ld\n", TIMEOUT_STOP_REQ_MS, ret);
		if (ret > 0) {
			chip->streamCtl[substream_number].pStopCompletion
				= NULL;
			aTrace(LOG_ALSA_INTERFACE, "Release completion\n");
		} else
			return 0;/*return without releasing DMA buffer*/
	}

	res = snd_pcm_lib_free_pages(substream);
	return res;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 *  Function Name: PcmPlaybackOpen
 *
 *  Description: Open PCM playback device
 *
 *------------------------------------------------------------
 */
static int PcmPlaybackOpen(struct snd_pcm_substream *substream)
{

	AUDIO_DRIVER_HANDLE_t drv_handle;
	BRCM_AUDIO_Param_Open_t param_open;
	brcm_alsa_chip_t *chip = snd_pcm_substream_chip(substream);
	struct snd_pcm_runtime *runtime = substream->runtime;
	int err = 0;
	int substream_number = substream->number;

	aTrace
	    (LOG_ALSA_INTERFACE, "ALSA-CAPH %lx:playback_open subdevice=%d,"
	     "PCM_TOTAL_BUF_BYTES=%d chip->iEnablePhoneCall=%d,"
	     "speaker=%d\n",
	     jiffies, substream->number, PCM_TOTAL_BUF_BYTES,
	     (unsigned int)chip->iEnablePhoneCall,
	     chip->streamCtl[substream_number].iLineSelect[0]);

	param_open.drv_handle = NULL;
	param_open.pdev_prop = &chip->streamCtl[substream_number].dev_prop;

	aTrace(LOG_ALSA_INTERFACE,
			"\n %lx:playback_open route the playback to CAPH\n",
			jiffies);
	/*route the playback to CAPH */
	runtime->hw = brcm_playback_hw;
	chip->streamCtl[substream_number].dev_prop.p[0].drv_type =
	    AUDIO_DRIVER_PLAY_AUDIO;
	chip->streamCtl[substream_number].pSubStream = substream;
	/*open the playback device */
	AUDIO_Ctrl_Trigger(ACTION_AUD_OpenPlay, &param_open, NULL, 1);
	drv_handle = param_open.drv_handle;
	if (drv_handle == NULL) {
		aError("\n %lx:playback_open subdevice=%d failed\n",
				jiffies, substream_number);
		return -1;
	}

	substream->runtime->private_data = drv_handle;
	aTrace(LOG_ALSA_INTERFACE,
			"chip-0x%lx substream-0x%lx drv_handle-0x%lx\n",
			(UInt32) chip, (UInt32) substream, (UInt32) drv_handle);
	return err;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 *  Function Name: PcmPlaybackClose
 *
 *  Description: Close PCM playback device
 *
 *------------------------------------------------------------
 */
static int PcmPlaybackClose(struct snd_pcm_substream *substream)
{
	BRCM_AUDIO_Param_Close_t param_close;
	brcm_alsa_chip_t *chip = snd_pcm_substream_chip(substream);

	aTrace(LOG_ALSA_INTERFACE,
		"ALSA-CAPH %lx:playback_close subdevice=%d\n", jiffies,
		substream->number);

	param_close.drv_handle = substream->runtime->private_data;

	/*close the driver */
	AUDIO_Ctrl_Trigger(ACTION_AUD_ClosePlay, &param_close, NULL, 1);

	substream->runtime->private_data = NULL;
	chip->streamCtl[substream->number].pSubStream = NULL;

	return 0;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 *  Function Name: PcmPlaybackPrepare
 *
 *  Description: Prepare PCM playback device, next call is Trigger or Close
 *
 *------------------------------------------------------------
 */
static int PcmPlaybackPrepare(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	brcm_alsa_chip_t *chip = snd_pcm_substream_chip(substream);
	BRCM_AUDIO_Param_Prepare_t parm_prepare;

	aTrace
	    (LOG_ALSA_INTERFACE, "ALSA-CAPH playback_prepare "
	     "period=%d period_size=%d,"
	     "bufsize=%d threshold=%ld frame_bits %d rate=%d ch=%d\n",
	     (int)runtime->periods, (int)runtime->period_size,
	     (int)runtime->buffer_size, runtime->stop_threshold,
	     runtime->frame_bits, runtime->rate, runtime->channels);

	chip->streamCtl[substream->number].stream_hw_ptr = 0;
	chip->streamCtl[substream->number].xrun_occured = 0;
	chip->streamCtl[substream->number].playback_prev_time = 0;
	parm_prepare.drv_handle = substream->runtime->private_data;
	parm_prepare.cbParams.pfCallBack = AUDIO_DRIVER_InterruptPeriodCB;
	parm_prepare.cbParams.pPrivateData = (void *)substream;
	parm_prepare.period_bytes =
	    frames_to_bytes(runtime, runtime->period_size);
	parm_prepare.buf_param.buf_size = runtime->dma_bytes;
	/*virtual address */
	parm_prepare.buf_param.pBuf = runtime->dma_area;
	/* physical address */
	parm_prepare.buf_param.phy_addr = (UInt32) (runtime->dma_addr);

	aTrace(LOG_ALSA_INTERFACE, "buf_size = %d pBuf=0x%lx phy_addr=0x%x\n",
			runtime->dma_bytes, (UInt32) runtime->dma_area,
			runtime->dma_addr);

	parm_prepare.drv_config.sample_rate = runtime->rate;
	parm_prepare.drv_config.num_channel = runtime->channels;
	parm_prepare.drv_config.bits_per_sample = 16;
	parm_prepare.drv_config.instanceId = substream->number;
	parm_prepare.drv_config.arm2sp_mixMode =
	    chip->pi32SpeechMixOption[substream->number];

	AUDIO_Ctrl_Trigger(ACTION_AUD_SetPrePareParameters, &parm_prepare, NULL,
			   0);
	/* calculate period in ms */
	if (runtime->rate)
		chip->streamCtl[substream->number].period_ms  =
			(runtime->period_size * 1000)/runtime->rate;

	/*
	   DEBUG("\n%lx:playback_prepare period bytes=%d,
	   periods =%d, buffersize=%d\n",jiffies,
	   g_brcm_alsa_chip->period_bytes[0], runtime->periods,
	   runtime->dma_bytes);
	 */
	return 0;
}

static void CtrlStopCB(int priv)
{
	struct completion *compl_ptr = (struct completion *)priv;
	aTrace(LOG_ALSA_INTERFACE, "CtrlStopCB:%p\n", compl_ptr);
	complete(compl_ptr);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 *  Function Name: PcmPlaybackTrigger
 *
 *  Description: Command handling function
 *
 *------------------------------------------------------------
 */
static int PcmPlaybackTrigger(struct snd_pcm_substream *substream, int cmd)
{
	int ret = 0;
	brcm_alsa_chip_t *chip = snd_pcm_substream_chip(substream);
	AUDIO_DRIVER_HANDLE_t drv_handle;
	int substream_number = substream->number;
	s32 *pSel;
	int i, count = 0;

	aTrace(LOG_ALSA_INTERFACE,
		"ALSA-CAPH PcmPlaybackTrigger substream_number=%d, cmd %d\n",
		substream_number, cmd);

	callMode = chip->iCallMode;
	drv_handle = substream->runtime->private_data;
	pSel = chip->streamCtl[substream_number].iLineSelect;

	for (i = 0; i < MAX_PLAYBACK_DEV; i++) {
		chip->streamCtl[substream_number].dev_prop.p[i].source =
		    AUDIO_SOURCE_MEM;
		chip->streamCtl[substream_number].dev_prop.p[i].sink =
		    AUDIO_SINK_UNDEFINED;
	}

	if (AUDCTRL_GetMFDMode() &&
		(chip->streamCtl[substream_number].iLineSelect[0] ==
			AUDIO_SINK_VIBRA)) {
		/*With MFD, vibra plays to IHF, and it does not mix with
		  voice even in call mode*/
		chip->streamCtl[substream_number].dev_prop.p[0].sink =
			AUDIO_SINK_LOUDSPK;

	} else if (
		(callMode == MODEM_CALL
			/*&& (audio_rpc_read_flag_vc_rel_by_modem() == FALSE)*/)
	    && (chip->streamCtl[substream_number].iLineSelect[0]
		!= AUDIO_SINK_I2S)) {
		/*call mode & not FM Tx playback */
		/*
		during voice call, audio is played throught DSP to CAPH so that
		DSP has the audio data for echo cancellation reference.

		but, for the short time window between the time voice call
		is already released by modem and the time user app tells
		kernel driver to end voice call audio path,
		audio playback is directly to CAPH. (not through DSP)
		This avoids the 1 frame (20ms) audio glitch in speaker output.
		*/

		chip->streamCtl[substream_number].dev_prop.p[0].source =
		    AUDIO_SOURCE_MEM;
		chip->streamCtl[substream_number].dev_prop.p[0].sink =
		    AUDIO_SINK_DSP;

		aTrace(LOG_ALSA_INTERFACE,
			"ALSA-CAPH PcmPlaybackTrigger to DSP MEM\n");

	} else {
		for (i = 0; i < MAX_PLAYBACK_DEV; i++) {
			if (pSel[i] >= AUDIO_SINK_HANDSET
			    && pSel[i] < AUDIO_SINK_VALID_TOTAL) {
				chip->streamCtl[substream_number].dev_prop.p[i].
				    sink = pSel[i];

				aTrace(LOG_ALSA_INTERFACE,
					"ALSA-CAPH PcmPlaybackTrigger to CAPH\n");

			} else {
				/* No valid device in the list to do a playback,
				 * return error
				 */
				if (++count == MAX_PLAYBACK_DEV) {
					aError("No device selected by"
						" the user ?\n");
					return -EINVAL;
				} else
					chip->streamCtl[substream_number].
					    dev_prop.p[i].sink =
					    AUDIO_SINK_UNDEFINED;
			}

		}
	}

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
		{
		BRCM_AUDIO_Param_Start_t param_start;
		BRCM_AUDIO_Param_Spkr_t param_spkr;
		BRCM_AUDIO_Param_Second_Dev_t param_second_spkr;

		struct snd_pcm_runtime *runtime = substream->runtime;

		chip->streamCtl[substream->number].playback_prev_time = 0;

		param_start.drv_handle = drv_handle;
		param_start.pdev_prop =
		    &chip->streamCtl[substream_number].dev_prop;
		param_start.channels = runtime->channels;
		param_start.rate = runtime->rate;
		param_start.vol[0] =
		    chip->streamCtl[substream_number].ctlLine[pSel[0]].
		    iVolume[0];
		param_start.vol[1] =
		    chip->streamCtl[substream_number].ctlLine[pSel[0]].
		    iVolume[1];
		param_start.stream = substream_number;

		param_second_spkr.source = AUDIO_SOURCE_UNDEFINED;
		param_second_spkr.sink = AUDIO_SINK_VALID_TOTAL;
		param_second_spkr.pathID = 0;
		param_second_spkr.substream_number = substream_number;

		/*the for loop starts with p[1], the second channel. */

		for (i = 1; i < MAX_PLAYBACK_DEV; i++) {
			AUDIO_SINK_Enum_t sink =
			chip->streamCtl[substream_number].dev_prop.p[i].sink;
			/*to support short tone to stereo IHF + headset,
			  only care these sinks*/
			if (sink == AUDIO_SINK_HEADSET ||
			    sink == AUDIO_SINK_HEADPHONE ||
			    sink == AUDIO_SINK_LOUDSPK) {
				param_second_spkr.source =
					chip->streamCtl[substream_number].
					dev_prop.p[i].source;

				param_second_spkr.sink =
					chip->streamCtl[substream_number].
					dev_prop.p[i].sink;
			}
		}
		AUDCTRL_SetSecondSink(param_second_spkr);
		AUDIO_Ctrl_Trigger(ACTION_AUD_StartPlay, &param_start, NULL, 0);

		for (i = 1; i < MAX_PLAYBACK_DEV; i++) {
			if (chip->streamCtl[substream_number].dev_prop.
			    p[i].sink != AUDIO_SINK_UNDEFINED) {

				param_spkr.src =
				    chip->streamCtl[substream_number].
				    dev_prop.p[i].source;
				param_spkr.sink =
				    chip->streamCtl[substream_number].
				    dev_prop.p[i].sink;
				param_spkr.stream = substream_number;
				AUDIO_Ctrl_Trigger
				    (ACTION_AUD_AddChannel, &param_spkr,
				     NULL, 0);
			}
		}
		}
		break;

	case SNDRV_PCM_TRIGGER_STOP:
		{
			BRCM_AUDIO_Param_Stop_t param_stop;
			struct completion *compl_ptr;

			param_stop.drv_handle = drv_handle;
			param_stop.pdev_prop =
			    &chip->streamCtl[substream_number].dev_prop;
			param_stop.stream = substream_number;
			aTrace
			    (LOG_ALSA_INTERFACE,
			     "ACTION_AUD_StopPlay param_stop.stream %d\n",
			     param_stop.stream);

			compl_ptr = &chip->streamCtl[substream_number]
				.stopCompletion;
			init_completion(compl_ptr);
			chip->streamCtl[substream_number].pStopCompletion
				= compl_ptr;

			AUDIO_Ctrl_Trigger(ACTION_AUD_StopPlay, &param_stop,
					   CtrlStopCB, (int)compl_ptr);

		}
		break;

	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		{
			BRCM_AUDIO_Param_Pause_t param_pause;

			param_pause.drv_handle = drv_handle;
			param_pause.pdev_prop =
			    &chip->streamCtl[substream_number].dev_prop;
			param_pause.stream = substream_number;
			AUDIO_Ctrl_Trigger(ACTION_AUD_PausePlay, &param_pause,
					   NULL, 0);

		}
		break;

	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		{
			BRCM_AUDIO_Param_Resume_t param_resume;

			struct snd_pcm_runtime *runtime = substream->runtime;

			param_resume.drv_handle = drv_handle;
			param_resume.pdev_prop =
			    &chip->streamCtl[substream_number].dev_prop;
			param_resume.channels = runtime->channels;
			param_resume.rate = runtime->rate;
			param_resume.stream = substream_number;
			AUDIO_Ctrl_Trigger(ACTION_AUD_ResumePlay, &param_resume,
					   NULL, 0);

		}
		break;

	default:
		return -EINVAL;
	}

	return ret;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 *  Function Name: PcmPlaybackPointer
 *
 *  Description: Get playback pointer in frames
 *
 *------------------------------------------------------------
 */
static snd_pcm_uframes_t PcmPlaybackPointer(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	snd_pcm_uframes_t pos = 0;
	int whichbuffer = 1;
	brcm_alsa_chip_t *chip = snd_pcm_substream_chip(substream);
	UInt16 dmaPointer = 0;

	if (chip->streamCtl[substream->number].xrun_occured)
		return SNDRV_PCM_POS_XRUN;

	if ((callMode == CALL_MODE_NONE)
	    || (chip->streamCtl[substream->number].iLineSelect[0] ==
		AUDIO_SINK_VIBRA && AUDCTRL_GetMFDMode())
	    || (chip->streamCtl[substream->number].iLineSelect[0] ==
		AUDIO_SINK_I2S)) {
		dmaPointer =
		    csl_audio_render_get_current_position(StreamIdOfDriver
							  (runtime->
							   private_data));
		if (bytes_to_frames(runtime, dmaPointer) >=
		    runtime->period_size)
			aError("Error unexpected: PcmPlaybackPointer"
				" hw_ptr = %d,dmaptr= %d, pos = %d\n",
				(int)chip->streamCtl[substream->number].
				stream_hw_ptr, dmaPointer, (int)pos);
#if !defined(DYNAMIC_DMA_PLAYBACK)
		whichbuffer = csl_audio_render_get_current_buffer(
			StreamIdOfDriver(runtime->private_data));
		if (whichbuffer != 1 && whichbuffer != 2) {
			/* aError("Error unexpected: PcmPlaybackPointer"
					"whichbuffer=%d state=%d\n",
					whichbuffer, runtime->status->state); */
			whichbuffer = 1;
		}

		if (runtime->status->state == SNDRV_PCM_STATE_RUNNING)
			pos = (whichbuffer - 1)*runtime->period_size;
		else
			pos = chip->streamCtl[substream->number].stream_hw_ptr;
	} else
#else
		dmaPointer = 0;
	}
#endif

	pos =
	    chip->streamCtl[substream->number].stream_hw_ptr +
	    bytes_to_frames(runtime, dmaPointer);
	pos %= runtime->buffer_size;
	/*
	aTrace(LOG_ALSA_INTERFACE,
		"PcmPlaybackPointer substream->number = %d pos=%lu"
		"state=%d whichbuffer=%d, int_pos=%lu\n",
		substream->number, pos, runtime->status->state,
		whichbuffer, chip->streamCtl[substream->number].stream_hw_ptr);
	*/
	return pos;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 *  Function Name: PcmCaptureOpen
 *
 *  Description: Open PCM capure device
 *
 *------------------------------------------------------------
 */
static int PcmCaptureOpen(struct snd_pcm_substream *substream)
{
	AUDIO_DRIVER_HANDLE_t drv_handle;
	BRCM_AUDIO_Param_Open_t param_open;
	brcm_alsa_chip_t *chip = snd_pcm_substream_chip(substream);
	struct snd_pcm_runtime *runtime = substream->runtime;
	int err = 0;
	/* for indexing */
	int substream_number = substream->number + CTL_STREAM_PANEL_PCMIN - 1;

	aTrace(LOG_ALSA_INTERFACE,
		"ALSA-CAPH PcmCaptureOpen substream->number = %d\n",
		substream->number);

	callMode = chip->iCallMode;

	if ((substream_number + 1) == CTL_STREAM_PANEL_PCMIN) {
		chip->streamCtl[substream_number].dev_prop.c.drv_type =
		    AUDIO_DRIVER_CAPT_HQ;
		runtime->hw = brcm_capture_hw;
	} else if ((substream_number + 1) == CTL_STREAM_PANEL_SPEECHIN) {
		chip->streamCtl[substream_number].dev_prop.c.drv_type =
		    AUDIO_DRIVER_CAPT_VOICE;
		runtime->hw = brcm_voice_capture_hw;
		/* should be multiple of SM size (16K support) */
		err = snd_pcm_hw_constraint_step(runtime, 0,
					 SNDRV_PCM_HW_PARAM_BUFFER_BYTES,
					 2560);
		err = snd_pcm_hw_constraint_list(runtime, 0,
		       SNDRV_PCM_HW_PARAM_PERIOD_BYTES,
		       &pcm_voice_capture_period_bytes_constraints_list);
		if (err < 0)
			return err;
	}
	/*open the capture device */
	param_open.drv_handle = NULL;
	param_open.pdev_prop = &chip->streamCtl[substream_number].dev_prop;

	/*for capture */
	chip->streamCtl[substream_number].pSubStream = substream;

	AUDIO_Ctrl_Trigger(ACTION_AUD_OpenRecord, &param_open, NULL, 1);

	drv_handle = param_open.drv_handle;

	if (drv_handle == NULL) {
		aError("\n %lx:capture_open subdevice=%d failed\n",
				jiffies, substream_number);
		return -1;
	}

	substream->runtime->private_data = drv_handle;

	aTrace(LOG_ALSA_INTERFACE,
			"\n %lx:capture_open subdevice=%d\n", jiffies,
			substream_number);

	return 0;

}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 *  Function Name: PcmCaptureClose
 *
 *  Description: Close PCM capure device
 *
 *------------------------------------------------------------
 */
static int PcmCaptureClose(struct snd_pcm_substream *substream)
{
	BRCM_AUDIO_Param_Close_t param_close;
	brcm_alsa_chip_t *chip = snd_pcm_substream_chip(substream);
	int substream_number = substream->number + CTL_STREAM_PANEL_PCMIN - 1;

	param_close.drv_handle = substream->runtime->private_data;
	/*close the driver */
	AUDIO_Ctrl_Trigger(ACTION_AUD_CloseRecord, &param_close, NULL, 1);

	substream->runtime->private_data = NULL;
	chip->streamCtl[substream_number].pSubStream = NULL;

	aTrace(LOG_ALSA_INTERFACE,
		"ALSA-CAPH %lx:capture_close subdevice=%d\n"
		, jiffies, substream_number);

	return 0;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 *  Function Name: PcmCapturePrepare
 *
 *  Description: Prepare hardware to capture
 *
 *------------------------------------------------------------
 */
static int PcmCapturePrepare(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	brcm_alsa_chip_t *chip = snd_pcm_substream_chip(substream);
	int substream_number = substream->number + CTL_STREAM_PANEL_PCMIN - 1;
	BRCM_AUDIO_Param_Prepare_t parm_prepare;

	aTrace(LOG_ALSA_INTERFACE,
		 "ALSA-CAPH %lx:capture_prepare: subdevice=%d rate =%d,"
		 "format =%d channel=%d dma_area=0x%x dma_bytes=%d,"
		 "period_bytes=%d avail_min=%d periods=%d buffer_size=%d\n",
		 jiffies, substream->number, runtime->rate, runtime->format,
		 runtime->channels, (unsigned int)runtime->dma_area,
		 runtime->dma_bytes, frames_to_bytes(runtime,
			 runtime->period_size),
		 frames_to_bytes(runtime, runtime->control->avail_min),
		 runtime->periods, (int)runtime->buffer_size);

	chip->streamCtl[substream_number].stream_hw_ptr = 0;

	parm_prepare.drv_handle = substream->runtime->private_data;

	parm_prepare.cbParams.pfCallBack = AUDIO_DRIVER_CaptInterruptPeriodCB;
	parm_prepare.cbParams.pPrivateData = (void *)substream;

	parm_prepare.period_bytes =
	    frames_to_bytes(runtime, runtime->period_size);

	parm_prepare.buf_param.buf_size = runtime->dma_bytes;
	/* virtual address */
	parm_prepare.buf_param.pBuf = runtime->dma_area;
	/* physical address */
	parm_prepare.buf_param.phy_addr = (UInt32) (runtime->dma_addr);

	aTrace(LOG_ALSA_INTERFACE, "buf_size = %d pBuf=0x%lx phy_addr=0x%x\n",
			runtime->dma_bytes, (UInt32) runtime->dma_area,
			runtime->dma_addr);

	parm_prepare.drv_config.sample_rate = runtime->rate;
	parm_prepare.drv_config.num_channel = runtime->channels;
	parm_prepare.drv_config.bits_per_sample = 16;

	AUDIO_Ctrl_Trigger(ACTION_AUD_SetPrePareParameters, &parm_prepare, NULL,
			   0);

	return 0;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 *  Function Name: PcmCaptureTrigger
 *
 *  Description: Command handling function
 *
 *------------------------------------------------------------
 */
static int PcmCaptureTrigger(struct snd_pcm_substream *substream,
		int cmd /*commands to handle */)
{
	brcm_alsa_chip_t *chip = snd_pcm_substream_chip(substream);
	AUDIO_DRIVER_HANDLE_t drv_handle;
	s32 *pSel;
	int substream_number = substream->number + CTL_STREAM_PANEL_PCMIN - 1;
	int ret;

	aTrace(LOG_ALSA_INTERFACE,
		"ALSA-CAPH %lx:capture_trigger subdevice=%d cmd=%d\n", jiffies,
		substream_number, cmd);

	callMode = chip->iCallMode;
	drv_handle = substream->runtime->private_data;
	pSel = chip->streamCtl[substream_number].iLineSelect;

	if (pSel[0] >= AUDIO_SOURCE_ANALOG_MAIN
	    && pSel[0] < AUDIO_SOURCE_VALID_TOTAL) {
		chip->streamCtl[substream_number].dev_prop.c.source = pSel[0];
	} else {
		aError
		    ("Error!! No valid device selected by the user,"
		     " pSel[0]=%d\n", pSel[0]);
		return -EINVAL;
	}
	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
		{
		BRCM_AUDIO_Param_Start_t param_start;

		struct snd_pcm_runtime *runtime = substream->runtime;

		param_start.drv_handle = drv_handle;
		param_start.pdev_prop =
		    &chip->streamCtl[substream_number].dev_prop;
		param_start.channels = runtime->channels;
		param_start.stream = substream_number;

		if (callMode == MODEM_CALL)
			/*record Mode */
			param_start.mixMode =
			    chip->pi32SpeechMixOption[substream_number];
		else	/* In Idle mode */
			param_start.mixMode = 0;

		aTrace(LOG_ALSA_INTERFACE, "param_start.mixMode %ld\n",
				param_start.mixMode);

		param_start.rate = runtime->rate;
		param_start.callMode = callMode;

		if ((substream_number + 1) == CTL_STREAM_PANEL_PCMIN)
			chip->streamCtl[substream_number].dev_prop.c.
			    sink = AUDIO_SINK_MEM;
		else if ((substream_number + 1) ==
			 CTL_STREAM_PANEL_SPEECHIN)
			chip->streamCtl[substream_number].dev_prop.c.
			    sink = AUDIO_SINK_DSP;

		param_start.vol[0] =
		    chip->streamCtl[substream_number].ctlLine[pSel[0]].
		    iVolume[0];
		param_start.vol[1] =
		    chip->streamCtl[substream_number].ctlLine[pSel[0]].
		    iVolume[1];

		ret = AUDIO_Ctrl_Trigger(ACTION_AUD_StartRecord, &param_start,
				   NULL, 0);

		if (ret == RESULT_ERROR) {

			if (chip->streamCtl[substream_number].
			pStopCompletion)
				complete(
				chip->streamCtl[substream_number].
				pStopCompletion);

			chip->streamCtl[substream_number].pStopCompletion
			= NULL;

			return -1;
		}

		}
		break;

	case SNDRV_PCM_TRIGGER_STOP:
		{
		BRCM_AUDIO_Param_Stop_t param_stop;
		struct completion *compl_ptr;

		param_stop.drv_handle = drv_handle;
		param_stop.pdev_prop =
		    &chip->streamCtl[substream_number].dev_prop;
		param_stop.stream = substream_number;

		param_stop.callMode = callMode;

		compl_ptr = &chip->streamCtl[substream_number]
			.stopCompletion;
		init_completion(compl_ptr);
		chip->streamCtl[substream_number].pStopCompletion
			= compl_ptr;
		ret = AUDIO_Ctrl_Trigger(ACTION_AUD_StopRecord, &param_stop,
				   CtrlStopCB, (int)compl_ptr);

		if (ret == RESULT_ERROR) {
			complete(compl_ptr);
			chip->streamCtl[substream_number].pStopCompletion
			= NULL;
		}

		}
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 *  Function Name: PcmCapturePointer
 *
 *  Description: Get capture pointer
 *
 *------------------------------------------------------------
 */
static snd_pcm_uframes_t PcmCapturePointer(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	snd_pcm_uframes_t pos;
	brcm_alsa_chip_t *chip = snd_pcm_substream_chip(substream);
	int substream_number = substream->number + CTL_STREAM_PANEL_PCMIN - 1;

	pos =
	    chip->streamCtl[substream_number].stream_hw_ptr %
	    runtime->buffer_size;
	/*
	aTrace(LOG_ALSA_INTERFACE,
	"%lx:PcmCapturePointer pos=%d pcm_read_ptr=%d, buffer size = %d,\n",
	jiffies, (int)pos, (int)chip->streamCtl[substream_number].stream_hw_ptr,
	(int)runtime->buffer_size);
	*/
	return pos;
}

int PcmPlaybackCopy(struct snd_pcm_substream *substream, int channel,
	    snd_pcm_uframes_t pos,
	    void __user *buf, snd_pcm_uframes_t count)
{
	int periods_copied;
	struct snd_pcm_runtime *runtime = substream->runtime;
	AUDIO_DRIVER_HANDLE_t drv_handle = substream->runtime->private_data;
	char *buf_start = runtime->dma_area;
	int period_bytes = frames_to_bytes(runtime, runtime->period_size);
	char *hwbuf = runtime->dma_area + frames_to_bytes(runtime, pos);
	int bytes_to_copy = frames_to_bytes(runtime, count);
	int not_copied = copy_from_user(hwbuf, buf, bytes_to_copy);
	char *new_hwbuf = hwbuf + (bytes_to_copy - not_copied);

	if (bytes_to_copy != period_bytes)
		aError("%s: stream = %d,"
		" buf_start = %p, hwbuf = %p, "
		"new_hwbuf = %p,period_bytes = %d,"
		" bytes_to_copy = %d, not_copied = %d, count %d\n",
		__func__, substream->number, buf_start, hwbuf,
		new_hwbuf, period_bytes, bytes_to_copy, not_copied,
		(int)count);

	if (not_copied) {
		aError("%s: why didn't copy all the bytes?"
			" not_copied = %d, to_copy = %d\n",
			__func__, not_copied, bytes_to_copy);
		aTrace(LOG_ALSA_INTERFACE, "%s: stream = %d,"
		" buf_start = %p, hwbuf = %p, "
		"new_hwbuf = %p,period_bytes = %d,"
		" bytes_to_copy = %d, not_copied = %d\n",
		__func__, substream->number, buf_start, hwbuf,
		new_hwbuf, period_bytes, bytes_to_copy, not_copied);
	}

	/**
	 * Set DMA engine ready bit according to pos and count
	*/
	periods_copied = bytes_to_copy/period_bytes;
	while (periods_copied--) {
		BRCM_AUDIO_Param_BufferReady_t param_bufferready;

		param_bufferready.drv_handle = drv_handle;
		param_bufferready.stream = substream->number;

		/*aTrace(LOG_ALSA_INTERFACE, "%s: stream = %d, "
			"ACTION_AUD_BufferReady\n",
			__func__, param_bufferready.stream);*/

		AUDIO_Ctrl_Trigger(ACTION_AUD_BufferReady, &param_bufferready,
			NULL, 0);
	}

	return 0;

}
int PcmPlaybackSilence(struct snd_pcm_substream *substream, int channel,
	       snd_pcm_uframes_t pos, snd_pcm_uframes_t count)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	char *hwbuf = runtime->dma_area + frames_to_bytes(runtime, pos);
	snd_pcm_format_set_silence(runtime->format, hwbuf,
		count * runtime->channels);
	/**
	 * Set DMA engine ready bit according to pos and count
	 */

	aTrace(LOG_ALSA_INTERFACE, "%s: stream = %d, Called\n",
			__func__, substream->number);

	return 0;
}


/* Playback device operator */
static struct snd_pcm_ops brcm_alsa_omx_pcm_playback_ops = {
	.open = PcmPlaybackOpen,
	.close = PcmPlaybackClose,
	.ioctl = snd_pcm_lib_ioctl,
	.hw_params = PcmHwParams,
	.hw_free = PcmHwFree,
	.prepare = PcmPlaybackPrepare,
	.trigger = PcmPlaybackTrigger,
	.pointer = PcmPlaybackPointer,
#if defined(DYNAMIC_DMA_PLAYBACK)
	.copy = PcmPlaybackCopy,
	/*.silence = PcmPlaybackSilence,*/
#endif
};

/* Capture device operator*/
static struct snd_pcm_ops brcm_alsa_omx_pcm_capture_ops = {
	.open = PcmCaptureOpen,
	.close = PcmCaptureClose,
	.ioctl = snd_pcm_lib_ioctl,
	.hw_params = PcmHwParams,
	.hw_free = PcmHwFree,
	.prepare = PcmCapturePrepare,
	.trigger = PcmCaptureTrigger,
	.pointer = PcmCapturePointer,
};

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 *  Function Name: AUDIO_DRIVER_CaptInterruptPeriodCB
 *
 *  Description: Playback inteerupt
 *
 *------------------------------------------------------------
 */
static void AUDIO_DRIVER_CaptInterruptPeriodCB(void *pPrivate)
{
	struct snd_pcm_substream *substream =
	    (struct snd_pcm_substream *)pPrivate;
	AUDIO_DRIVER_HANDLE_t drv_handle;
	AUDIO_DRIVER_TYPE_t drv_type;
	struct snd_pcm_runtime *runtime;
	brcm_alsa_chip_t *pChip = NULL;
	int substream_number;

	if (!substream) {
		aError("Invalid substream 0x%p\n", substream);
		return;
	}
	substream_number = substream->number + CTL_STREAM_PANEL_PCMIN - 1;
	pChip = snd_pcm_substream_chip(substream);
	runtime = substream->runtime;
	if (!runtime) {
		aError("Invalid runtime 0x%p\n", runtime);
		return;
	}
	drv_handle = substream->runtime->private_data;

	AUDIO_DRIVER_Ctrl(drv_handle, AUDIO_DRIVER_GET_DRV_TYPE,
			  (void *)&drv_type);
/*
	aTrace(LOG_ALSA_INTERFACE,
		"\n %lx:CaptInterruptPeriodCB drv_type=%d,\n",
		jiffies, (int)drv_type);
*/

	switch (drv_type) {
	case AUDIO_DRIVER_CAPT_HQ:
	case AUDIO_DRIVER_CAPT_VOICE:
		{
			/*update the PCM read pointer by period size */
			pChip->streamCtl[substream_number].stream_hw_ptr +=
			    runtime->period_size;
			if (pChip->streamCtl[substream_number].stream_hw_ptr >
			    runtime->boundary)
				pChip->streamCtl[substream_number].
				    stream_hw_ptr -= runtime->boundary;
			/*send the period elapsed */
			snd_pcm_period_elapsed(substream);

			logmsg_ready(substream, AUD_LOG_PCMIN);

		}
		break;
	default:
		aWarn("Invalid driver type\n");
		break;
	}
	return;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 *  Function Name: AUDIO_DRIVER_InterruptPeriodCB
 *
 *  Description: Callback funtion when DMA done, running at
 *  worker thread context (worker_audio_playback)
 *
 *------------------------------------------------------------
 */
static void AUDIO_DRIVER_InterruptPeriodCB(void *pPrivate)
{
	struct snd_pcm_substream *substream =
	    (struct snd_pcm_substream *)pPrivate;
	AUDIO_DRIVER_HANDLE_t drv_handle;
	AUDIO_DRIVER_TYPE_t drv_type;
	struct snd_pcm_runtime *runtime;
	brcm_alsa_chip_t *pChip = NULL;
	UInt32 num_periods = 1;
	UInt32 int_time;
	Int32 int_period;
	int whichbuffer = 0;


	/* find in ms */
	int_time = jiffies_to_msecs(jiffies);

	if (!substream) {
		aError("Invalid substream 0x%p\n", substream);
		return;
	}
	pChip = snd_pcm_substream_chip(substream);

	runtime = substream->runtime;
	if (!runtime) {
		aError("Invalid runtime 0x%p\n", runtime);
		return;
	}
	drv_handle = substream->runtime->private_data;

	if (pChip->streamCtl[substream->number].playback_stop) {
		aWarn("Invalid DMA Callback\n");
		return;
	}

	if (pChip->streamCtl[substream->number].playback_prev_time != 0) {
		int_period = int_time -
			pChip->streamCtl[substream->number].playback_prev_time;
		/* wrap around */
		if (int_period < 0)
			int_period += (MAX_32);

		/* get the number of periods */
		if (pChip->streamCtl[substream->number].period_ms)
			num_periods =
			int_period/pChip->streamCtl[substream->number].
			period_ms;

		if (num_periods > 1) {
			pChip->streamCtl[substream->number].xrun_occured = 1;
			aWarn("dI-%ld %ld %ld\n",
				int_period, int_time,
				pChip->streamCtl[substream->number].
				playback_prev_time);
		}
	}
	pChip->streamCtl[substream->number].playback_prev_time = int_time;

	whichbuffer = csl_audio_render_get_current_buffer(
		StreamIdOfDriver(runtime->private_data));
	if (whichbuffer == 0) {
#if !defined(DYNAMIC_DMA_PLAYBACK)
		pChip->streamCtl[substream->number].xrun_occured = 1;
		aWarn("whichbuffer-%d\n", whichbuffer);
#endif
	}

	AUDIO_DRIVER_Ctrl(drv_handle, AUDIO_DRIVER_GET_DRV_TYPE,
			  (void *)&drv_type);

	switch (drv_type) {
	case AUDIO_DRIVER_PLAY_VOICE:
	case AUDIO_DRIVER_PLAY_AUDIO:
	case AUDIO_DRIVER_PLAY_RINGER:
		{
			/*update the PCM read pointer by period size */

			pChip->streamCtl[substream->number].stream_hw_ptr +=
			    runtime->period_size;
			if (pChip->streamCtl[substream->number].stream_hw_ptr >
			    runtime->boundary)
				pChip->streamCtl[substream->number].
				    stream_hw_ptr -= runtime->boundary;
			/* send the period elapsed */
			snd_pcm_period_elapsed(substream);

			logmsg_ready(substream, AUD_LOG_PCMOUT);

		}
		break;
	default:
		aWarn("Invalid driver type\n");
		break;
	}

	return;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 *  Function Name: PcmDeviceNew
 *
 *  Description: Create PCM playback and capture device
 *
 *------------------------------------------------------------
 */
int __devinit PcmDeviceNew(struct snd_card *card)
{
	struct snd_pcm *pcm;
	int err = 0;
	brcm_alsa_chip_t *pChip;
	struct snd_pcm_substream *substream;

	callMode = CALL_MODE_NONE;
	err =
	    snd_pcm_new(card, "Broadcom CAPH", 0, NUM_PLAYBACK_SUBDEVICE,
			NUM_CAPTURE_SUBDEVICE, &pcm);
	if (err < 0)
		return err;

	pcm->private_data = card->private_data;
	strcpy(pcm->name, "Broadcom CAPH PCM");

	snd_pcm_set_ops(pcm, SNDRV_PCM_STREAM_PLAYBACK,
			&brcm_alsa_omx_pcm_playback_ops);
	snd_pcm_set_ops(pcm, SNDRV_PCM_STREAM_CAPTURE,
			&brcm_alsa_omx_pcm_capture_ops);

	pcm->info_flags = 0;

	/*pre-allocate memory for playback device */
	substream = pcm->streams[0].substream;
	for (; substream; substream = substream->next) {

		err =
		    snd_pcm_lib_preallocate_pages(substream, SNDRV_DMA_TYPE_DEV,
						  0,
						  (IS_PCM_MEM_PREALLOCATED) ?
						  PCM_MAX_PLAYBACK_BUF_BYTES :
						  0,
						  PCM_MAX_PLAYBACK_BUF_BYTES);
		if (err)
			aError
			    ("\n Error : Error when allocate memory for"
			     " playback device err=%d\n", err);
	}

	/*pre-allocate memory for capture device */
	substream = pcm->streams[1].substream;
	for (; substream; substream = substream->next) {
		err =
		    snd_pcm_lib_preallocate_pages(substream, SNDRV_DMA_TYPE_DEV,
						  0,
						  (IS_PCM_MEM_PREALLOCATED) ?
						  PCM_MAX_CAPTURE_BUF_BYTES : 0,
						  PCM_MAX_CAPTURE_BUF_BYTES);
		if (err)
			aError
			    ("\n Error : Error when allocate memory for"
			     " capture device err=%d\n", err);
	}

	pChip = (brcm_alsa_chip_t *) card->private_data;

	/* Initialize the audio controller */
	aTrace(LOG_ALSA_INTERFACE, "ALSA-CAPH PcmDeviceNew:call AUDIO_Init\n");
	caph_audio_init();
	AUDCTRL_Init();
#if defined(CONFIG_BCM_MODEM)
	DSPDRV_Init();
#endif

	aTrace(LOG_ALSA_INTERFACE,
			"\n PcmDeviceNew : PcmDeviceNew err=%d\n", err);
	return err;

}
