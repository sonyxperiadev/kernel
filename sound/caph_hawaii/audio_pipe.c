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
/********************************************************************
Copyright 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement
governing use of this software, this software is licensed to you under the
terms of the GNU General Public License version 2, available at
http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software
in any way with any other Broadcom software provided under a license other than
the GPL, without Broadcom's express prior written consent.
***********************************************************************/
#include <linux/types.h>
#include "mobcom_types.h"
#include "resultcode.h"
#include "audio_consts.h"
#include "csl_caph.h"
#include "bcm_fuse_sysparm_CIB.h"
#include "audio_tuning.h"
#include "audio_vdriver.h"
#include "audio_controller.h"
#include "audio_ddriver.h"
#include "audio_caph.h"
#include "caph_common.h"
#include "audio_pipe.h"
#include "csl_caph_dma.h"

#if defined(CONFIG_BCM_KNLLOG_SUPPORT)
#include <linux/broadcom/knllog.h>
#endif
#include <mach/profile_timer.h>
#include <linux/proc_fs.h>
/*#include <linux/spinlock.h>*/

/*
* pipe info
*/
struct _AUDP_PIPE_INFO_t_ {
	Boolean used;
	int pipeID;		/* aap use */
	void *handle;		/* data driver use */
	PIPE_DIRECTION_t direction;	/* 1-DL, 2- UL */
	PIPE_DEVICE_ID_t device;
	PIPE_STATE_t state;
	unsigned long period_ms;
	AUDIO_NUM_OF_CHANNEL_t numChannels;
	AUDIO_BITS_PER_SAMPLE_t bitsPerSample;
	AUDIO_SAMPLING_RATE_t sampleRate;
	TIDChanOfDev dev_prop;
	AUDIO_DRIVER_Stats_t CBPrivate;
	UInt32 dma_phy_addr;	/* for debug only */
	void *privData;		/* from aap */
	Boolean synced;		/* updated in dma cb */
};

#define AUDP_PIPE_INFO_t struct _AUDP_PIPE_INFO_t_
static CB_PIPE aap_cb;
static AUDP_PIPE_INFO_t sAudpInfo[AUDIOP_MAX_PIPE_SUPPORT];
static struct semaphore sAudpLock;	/* critical region protection */

/* keep a record of how many pipes to send sync event to aap */
static int sRunningPipes;
static atomic_t sArrivingPipes;
unsigned long lockFlags;
static spinlock_t sHwAADmaLock;
static uint16_t sPipeSynced;

/* used to resync dma interrupts when shift happens*/
/* timestamp when the prev sync event sent to aap */
static unsigned int sPrevSyncEventMs;
static bool sPrevSynced = FALSE;
static unsigned long resync_count;
#define DMA_FRAME_PERIOD_MS  5
#define DMA_RESYNC_MARGIN_MS 3	/* time reserved for aap per frame */
#define TEST_HW_SRC 1

static void audp_set_pipe_hwparams(int pipeID, PIPE_DEVICE_ID_t id);
static void audp_debug_pipe(void);

/* brutal force resync test
extern void csl_caph_dma_sync_transfer(void); */

/*
 *  Function Name: AUDP_AADMAC_CB
 *
 *  Description: Callback funtion when DMA done, running at
 *  worker thread context (worker_audio_playback)
 *
 */
/* #define AUDP_LOOPBACK */
#ifdef AUDP_LOOPBACK
static char gLoopbackBuffer[480];
#endif
static void AUDP_AADMAC_CB(void *pPrivate)
{
	AUDIO_DRIVER_Stats_t *p = NULL;
	unsigned int current_time_ms = 0;
	int i;

	/*
	   unsigned long lock_flags;
	   spinlock_t dma_lock;
	   spin_lock_init (&dma_lock);
	   spin_lock_irqsave (&dma_lock, lock_flags);
	 */
	if (aap_cb && pPrivate) {
		p = (AUDIO_DRIVER_Stats_t *) pPrivate;
#ifdef AUDP_LOOPBACK
		if (sAudpInfo[p->pipeID].direction == AUDIOP_PIPE_DIR_UL) {
			memcpy(&gLoopbackBuffer[0],
			       phys_to_virt(sAudpInfo[p->pipeID].dma_phy_addr +
					    p->buffer_idx * 480), 480);
		}
#endif

		/* keep sending buffer done msg to aap for each pipe */
		aap_cb(AUDIOP_PIPE_BUFFER_DONE, p->pipeID,
		       (UInt32) (p->buffer_idx), (UInt32) p->dmach,
		       (UInt32) p->intr_counter, 0,
		       (uint32_t) sAudpInfo[p->pipeID].privData);

		current_time_ms = timer_get_msec();
		/* check if enough margin left for aap,
		otherwise resync all dma transfers */
		if (sPrevSynced
		    && ((current_time_ms - sPrevSyncEventMs) <
			DMA_RESYNC_MARGIN_MS)) {
			/* uncomment to do brutal force resync test */
			csl_caph_dma_sync_transfer();
			atomic_set(&sArrivingPipes, 0);
			sPrevSynced = FALSE;
#if defined(CONFIG_BCM_KNLLOG_SUPPORT)
			KNLLOG
			    ("xxxxxx RESYNC #%d current_time_ms=%d"
			    "sPrevSyncTime=%d xxxxxx\r\n",
			     ++resync_count, current_time_ms,
			     sPrevSyncEventMs);
#endif
			return;
		}

		/* reused the logic from aap */
		atomic_inc(&sArrivingPipes);

		/* Check if all ingress operations complete */
		if (atomic_read(&sArrivingPipes) >= sRunningPipes) {
			aap_cb(AUDIOP_PIPE_SYNC_EVENT, sPipeSynced,
			       sRunningPipes, 0, 0, 0, 0);
			sPrevSyncEventMs = timer_get_msec();
			sPrevSynced = TRUE;

			for (i = 0; i < AUDIOP_MAX_PIPE_SUPPORT; i++)
				if ((sAudpInfo[i].synced == FALSE)
				    && (sAudpInfo[i].state ==
					AUDIOP_PIPE_STATE_PREPARED)) {
					/* this api will need
					rework to be called here */
					audp_start_pipe(i);
					sAudpInfo[i].synced = TRUE;
					sPipeSynced |= (1 << i);
					/* break; */
				}
			atomic_set(&sArrivingPipes, 0);
		}
#ifdef AUDP_LOOPBACK
		if (sAudpInfo[p->pipeID].direction == AUDIOP_PIPE_DIR_DL) {
			memcpy(phys_to_virt
			       (sAudpInfo[p->pipeID].dma_phy_addr +
				p->buffer_idx * 480), &gLoopbackBuffer[0], 480);
		}
#endif
	} else
		DEBUG("AUDP_AADMAC_CB fail\r\n");

	/* spin_unlock_irqrestore (&dma_lock, lock_flags); */

	return;
}

/*
 *
 * Function Name: audp_registerCB
 *
 * Description: called by AAP to register a CB
 *
 */
int audp_registerCB(CB_PIPE from_aap)
{
#if defined(CONFIG_BCM_KNLLOG_SUPPORT)
	KNLLOG("audp_registerCB\r\n");
#endif
	DEBUG("audp_registerCB\r\n");
	if (from_aap != NULL) {
		aap_cb = from_aap;
		sema_init(&sAudpLock, 1);
		spin_lock_init(&sHwAADmaLock);
		audp_debug_pipe();
	} else {
		DEBUG("audp registerCB is NULL\r\n");
		return -1;
	}
	return 0;
}

/*
 *
 * Function Name: audp_deregister
 *
 * Description: called by AAP to deregister
 *
 */
int audp_deregister()
{
#if defined(CONFIG_BCM_KNLLOG_SUPPORT)
	KNLLOG("audp_deregister\r\n");
#endif
	DEBUG("audp_deregister\r\n");
	if (aap_cb != NULL)
		aap_cb = NULL;

	return 0;
}

/*
 *
 * Function Name: audp_create_pipe
 *
 * Description: called by alsa ctrl to create pipe
 *
 */
int audp_create_pipe(PIPE_DEVICE_ID_t id)
{
	int i = -1;
	BRCM_AUDIO_Param_Open_t param_open;
	TIDChanOfDev dev_prop, *pdev_prop = NULL;
	PIPE_DIRECTION_t direction = AUDIOP_PIPE_DIR_NONE;
#if defined(CONFIG_BCM_KNLLOG_SUPPORT)
	KNLLOG("audp_create_pipe: pipe dev=0x%x\r\n", id);
#endif
	DEBUG("audp_create_pipe: pipe dev=0x%x\r\n", id);

	memset(&param_open, 0, sizeof(param_open));
	memset(&dev_prop, 0, sizeof(dev_prop));

	param_open.pdev_prop = &dev_prop;

	switch (id) {
	case AUDIOP_MIC_MAIN:
	case AUDIOP_MIC_AUX:
	case AUDIOP_MIC_DIGI1:
	case AUDIOP_MIC_DIGI2:
	case AUDIOP_MIC_DIGI3:
	case AUDIOP_MIC_DIGI4:
	case AUDIOP_MIC_BTM:
	case AUDIOP_MIC_USB:
		/* hardcoded drv type to obtain drv handle */
		param_open.pdev_prop->c.drv_type = AUDIO_DRIVER_CAPT_EPT;
		AUDIO_Ctrl_Trigger(ACTION_AUD_OpenRecord, &param_open, NULL, 1);
		direction = AUDIOP_PIPE_DIR_UL;
		break;
	case AUDIOP_SPK_HANDSET:
	case AUDIOP_SPK_HEADSET:
	case AUDIOP_SPK_LOUDSPK:
	case AUDIOP_SPK_VIBRA:
	case AUDIOP_SPK_BTM:
	case AUDIOP_SPK_USB:
		/* hardcoded drv type to obtain drv handle */
		param_open.pdev_prop->p[0].drv_type = AUDIO_DRIVER_PLAY_EPT;
		AUDIO_Ctrl_Trigger(ACTION_AUD_OpenPlay, &param_open, NULL, 1);
		direction = AUDIOP_PIPE_DIR_DL;
		break;
	default:
		DEBUG("audp_create_pipe: unsupported pipe dev=0x%x\r\n", id);
		break;
	}

	if (aap_cb && param_open.drv_handle) {
		down(&sAudpLock);
		for (i = 0; i < AUDIOP_MAX_PIPE_SUPPORT; i++)
			if (sAudpInfo[i].used == FALSE)
				break;
		up(&sAudpLock);

		if (i == AUDIOP_MAX_PIPE_SUPPORT) {
			DEBUG("audp_create_pipe failed\r\n");
			return -1;
		}

		down(&sAudpLock);
		sAudpInfo[i].used = TRUE;
		sAudpInfo[i].pipeID = i;
		sAudpInfo[i].handle = param_open.drv_handle;
		sAudpInfo[i].direction = direction;
		sAudpInfo[i].device = id;
		sAudpInfo[i].state = AUDIOP_PIPE_STATE_CREATED;
		/* hard coded pipe params */
#ifdef TEST_HW_SRC
		sAudpInfo[i].sampleRate = AUDIO_SAMPLING_RATE_16000;
#else
		sAudpInfo[i].sampleRate = AUDIO_SAMPLING_RATE_48000;
#endif
		if (id & AUDIOP_SPK_HEADSET)
			/*STEREO*/
			sAudpInfo[i].numChannels = AUDIO_CHANNEL_MONO;
		else
			sAudpInfo[i].numChannels = AUDIO_CHANNEL_MONO;

		sAudpInfo[i].bitsPerSample = 16;
		sAudpInfo[i].period_ms = DMA_FRAME_PERIOD_MS;
		sAudpInfo[i].CBPrivate.dmach = 0;
		sAudpInfo[i].CBPrivate.intr_counter = 0;
		sAudpInfo[i].CBPrivate.pipeID = sAudpInfo[i].pipeID;
		/* hard coded device related info for this pipe */
		pdev_prop = &sAudpInfo[i].dev_prop;
		if (direction & AUDIOP_PIPE_DIR_UL) {
			/* hardcoded for test, these should
			be filled when pipe is created */
			pdev_prop->c.drv_type = AUDIO_DRIVER_CAPT_EPT;
		}
		if (direction & AUDIOP_PIPE_DIR_DL)
			pdev_prop->p[0].drv_type = AUDIO_DRIVER_PLAY_EPT;
		up(&sAudpLock);

		audp_set_pipe_hwparams(sAudpInfo[i].
			pipeID,
			sAudpInfo[i].device);
		/* args: msgID, pipeID,devID, samplerate,
		stereo/mono, bitspersecond, dir */
		aap_cb(AUDIOP_PIPE_CREATED, i, sAudpInfo[i].device,
		       sAudpInfo[i].sampleRate, sAudpInfo[i].numChannels,
		       sAudpInfo[i].bitsPerSample, sAudpInfo[i].direction);
	} else
		DEBUG("audp create pipe fail\r\n");

	/* use drv handle for pipe id */
	return i;
}

/*
*
* Function Name: audp_set_pipe_hwparams
*
* Description: fill in hw related info from audp to audio controller
*
*/
static void audp_set_pipe_hwparams(int pipeID, PIPE_DEVICE_ID_t id)
{
	TIDChanOfDev *p = &sAudpInfo[pipeID].dev_prop;

	DEBUG("audp_set_pipe_hwparams pipeID=0x%x pipe_dev_id=0x%x\r\n", pipeID,
	      id);

	if (p == NULL)
		return;
	/* fill in params for this pipe */
	switch (id) {
	case AUDIOP_MIC_MAIN:
		p->c.source = AUDIO_SOURCE_ANALOG_MAIN;
		p->c.sink = AUDIO_SINK_MEM;
		break;
	case AUDIOP_MIC_AUX:
		p->c.source = AUDIO_SOURCE_ANALOG_AUX;
		p->c.sink = AUDIO_SINK_MEM;
		break;
	case AUDIOP_MIC_DIGI1:
		p->c.source = AUDIO_SOURCE_DIGI1;
		p->c.sink = AUDIO_SINK_MEM;
		break;
	case AUDIOP_MIC_DIGI2:
		p->c.source = AUDIO_SOURCE_DIGI2;
		p->c.sink = AUDIO_SINK_MEM;
		break;
	case AUDIOP_MIC_DIGI3:
		p->c.source = AUDIO_SOURCE_DIGI3;
		p->c.sink = AUDIO_SINK_MEM;
		break;
	case AUDIOP_MIC_DIGI4:
		p->c.source = AUDIO_SOURCE_DIGI4;
		p->c.sink = AUDIO_SINK_MEM;
		break;
	case AUDIOP_MIC_BTM:
		p->c.source = AUDIO_SOURCE_BTM;
		p->c.sink = AUDIO_SINK_MEM;
		break;
	case AUDIOP_MIC_USB:
		p->c.source = AUDIO_SOURCE_USB;
		p->c.sink = AUDIO_SINK_MEM;
		break;
	case AUDIOP_SPK_HANDSET:
		p->p[0].source = AUDIO_SOURCE_MEM;
		p->p[0].sink = AUDIO_SINK_HANDSET;
		break;
	case AUDIOP_SPK_HEADSET:
		p->p[0].source = AUDIO_SOURCE_MEM;
		p->p[0].sink = AUDIO_SINK_HEADSET;
		break;
	case AUDIOP_SPK_LOUDSPK:
		p->p[0].source = AUDIO_SOURCE_MEM;
		p->p[0].sink = AUDIO_SINK_LOUDSPK;
		break;
	case AUDIOP_SPK_VIBRA:
		p->p[0].source = AUDIO_SOURCE_MEM;
		p->p[0].sink = AUDIO_SINK_VIBRA;

		break;
	case AUDIOP_SPK_BTM:
		p->p[0].source = AUDIO_SOURCE_MEM;
		p->p[0].sink = AUDIO_SINK_BTM;
		break;
	case AUDIOP_SPK_USB:
		p->p[0].source = AUDIO_SOURCE_MEM;
		p->p[0].sink = AUDIO_SINK_USB;
		break;
	default:
		/* unsupported device */
		break;
	}
	return;
}

/*
*
* Function Name: audp_set_buffer
*
* Description: called by AAP to set the pipe buffer
*
*/
int audp_set_buffer(int pipeID, dma_addr_t startAddr, uint32_t bufferSize,
		    uint32_t bufferNum, void *privData)
{
	BRCM_AUDIO_Param_Prepare_t param_prepare;

	if ((aap_cb == NULL) || (privData == NULL))
		return -1;

#if defined(CONFIG_BCM_KNLLOG_SUPPORT)
	KNLLOG
	    ("audp_set_buffer: pipeID=0x%x startAddr=0x%x"
	    "bufferSize=0x%x bufferNum=0x%x\r\n",
	     pipeID, startAddr, bufferSize, bufferNum);
#endif
	DEBUG
	    ("audp_set_buffer: pipeID=0x%x startAddr=0x%x"
	    "bufferSize=0x%x bufferNum=0x%x\r\n",
	     pipeID, startAddr, bufferSize, bufferNum);

	memset(&param_prepare, 0, sizeof(param_prepare));
	down(&sAudpLock);
	param_prepare.drv_handle = (void *)sAudpInfo[pipeID].handle;
	param_prepare.cbParams.pfCallBack = AUDP_AADMAC_CB;
	param_prepare.cbParams.pPrivateData =
	    (void *)(&sAudpInfo[pipeID].CBPrivate);

	param_prepare.buf_param.buf_size = bufferSize;
	/* not needed, do not set as NULL */
	param_prepare.buf_param.pBuf = (UInt8 *) 0xffff;
	/* physical address from aap */
	param_prepare.buf_param.phy_addr = (UInt32) (startAddr);
	sAudpInfo[pipeID].dma_phy_addr = (UInt32) (startAddr);
	param_prepare.drv_config.sample_rate = sAudpInfo[pipeID].sampleRate;
	param_prepare.drv_config.num_channel = sAudpInfo[pipeID].numChannels;
	param_prepare.drv_config.bits_per_sample =
	    sAudpInfo[pipeID].bitsPerSample;

	/* may not need this param. can be derived from others */
	param_prepare.period_bytes =
	    sAudpInfo[pipeID].period_ms *
	    (param_prepare.drv_config.sample_rate / 1000)
	    * (param_prepare.drv_config.num_channel) * 2;

	sAudpInfo[pipeID].privData = privData;

	AUDIO_Ctrl_Trigger(ACTION_AUD_SetPrePareParameters, &param_prepare,
			   NULL, 1);
	sAudpInfo[pipeID].state = AUDIOP_PIPE_STATE_PREPARED;
	up(&sAudpLock);

	/* for internal test, not needed by aap */
	aap_cb(AUDIOP_PIPE_SETBUFDONE, pipeID, (UInt32) startAddr, 0, 0, 0,
	       (uint32_t) privData);
	/* start the pilot pipe first, the later pipes will be
	started in dmacb to sync with each other */
	if (sRunningPipes == 0) {
		audp_start_pipe(pipeID);
		sAudpInfo[pipeID].synced = TRUE;
		sPipeSynced = 1;
	}

	return 0;
}

/*
*
* Function Name: audp_start_pipe
*
* Description: called by aap to start pilot pipe, or
* by dma cb to start synced pipes
*
*
*/
void audp_start_pipe(int pipeID)
{
	BRCM_AUDIO_Param_Start_t param_start;
	PIPE_DIRECTION_t direction = AUDIOP_PIPE_DIR_NONE;
	void *privData = NULL;
#if defined(CONFIG_BCM_KNLLOG_SUPPORT)
	KNLLOG("audp_start_pipe: pipe_id=0x%x\r\n", pipeID);
#endif
	memset(&param_start, 0, sizeof(param_start));

	down(&sAudpLock);
	param_start.drv_handle = sAudpInfo[pipeID].handle;
	param_start.pdev_prop = &sAudpInfo[pipeID].dev_prop;
	param_start.channels = sAudpInfo[pipeID].numChannels;
	param_start.rate = sAudpInfo[pipeID].sampleRate;
	direction = sAudpInfo[pipeID].direction;
	sAudpInfo[pipeID].state = AUDIOP_PIPE_STATE_STARTED;
	privData = sAudpInfo[pipeID].privData;
	sRunningPipes++;
	up(&sAudpLock);

	/* pump the gain for lpbk test with aap */
	/* for mic, it is hardcoded in Q13.2 format */
	if (direction == AUDIOP_PIPE_DIR_UL) {
		param_start.vol[0] = 80;
		param_start.vol[1] = 80;
	}
	if (direction == AUDIOP_PIPE_DIR_DL) {
		param_start.vol[0] = 20;
		param_start.vol[1] = 20;
	}

	if (direction == AUDIOP_PIPE_DIR_UL)
		AUDIO_Ctrl_Trigger(ACTION_AUD_StartRecord, &param_start, NULL,
				   0);
	else if (direction == AUDIOP_PIPE_DIR_DL)
		AUDIO_Ctrl_Trigger(ACTION_AUD_StartPlay, &param_start, NULL, 0);

	if (aap_cb && param_start.drv_handle)
		aap_cb(AUDIOP_PIPE_STARTED, pipeID, 0, 0, 0, 0,
		       (uint32_t) privData);
	else
		DEBUG("audp start pipe fail\r\n");

}

/*
*
* Function Name: audp_stop_pipe
*
* Description: called by alsa ctrl to stop pipe
*
*/
void audp_stop_pipe(int pipeID)
{
	BRCM_AUDIO_Param_Stop_t param_stop;
	TIDChanOfDev dev_prop;
	PIPE_DIRECTION_t direction = AUDIOP_PIPE_DIR_NONE;
	void *privData = NULL;
#if defined(CONFIG_BCM_KNLLOG_SUPPORT)
	KNLLOG("audp_stop_pipe: pipe_id=0x%x\r\n", pipeID);
#endif
	DEBUG("audp_stop_pipe: pipe_id=0x%x\r\n", pipeID);

	memset(&param_stop, 0, sizeof(param_stop));
	memset(&dev_prop, 0, sizeof(dev_prop));
	down(&sAudpLock);
	param_stop.drv_handle = sAudpInfo[pipeID].handle;
	/* param_stop.pdev_prop = &sAudpInfo[pipeID].dev_prop;*/
	memcpy(&dev_prop, &sAudpInfo[pipeID].dev_prop, sizeof(dev_prop));
	param_stop.pdev_prop = &dev_prop;
	direction = sAudpInfo[pipeID].direction;
	sAudpInfo[pipeID].state = AUDIOP_PIPE_STATE_STOPPED;
	privData = sAudpInfo[pipeID].privData;
	sRunningPipes--;
	up(&sAudpLock);

	if (direction == AUDIOP_PIPE_DIR_UL)
		AUDIO_Ctrl_Trigger(ACTION_AUD_StopRecord, &param_stop, NULL, 1);
	else if (direction == AUDIOP_PIPE_DIR_DL)
		AUDIO_Ctrl_Trigger(ACTION_AUD_StopPlay, &param_stop, NULL, 1);

	if (aap_cb)
		aap_cb(AUDIOP_PIPE_STOPPED, pipeID, 0, 0, 0, 0,
		       (uint32_t) privData);
	else
		DEBUG("audp stop pipe fail\r\n");
	down(&sAudpLock);
	sPipeSynced &= ~(1 << pipeID);
	up(&sAudpLock);
}

/*
*
* Function Name: audp_destroy_pipe
*
* Description: called by alsa ctrl to destroy pipe
*
*/
void audp_destroy_pipe(int pipeID)
{
	BRCM_AUDIO_Param_Close_t param_close;
	PIPE_DIRECTION_t direction = AUDIOP_PIPE_DIR_NONE;
	void *privData = NULL;
#if defined(CONFIG_BCM_KNLLOG_SUPPORT)
	KNLLOG("audp_destroy_pipe: pipe_id=0x%x\r\n", pipeID);
#endif
	DEBUG("audp_destroy_pipe: pipe_id=0x%x\r\n", pipeID);

	memset(&param_close, 0, sizeof(param_close));
	down(&sAudpLock);
	param_close.drv_handle = sAudpInfo[pipeID].handle;
	direction = sAudpInfo[pipeID].direction;
	sAudpInfo[pipeID].state = AUDIOP_PIPE_STATE_DESTROYED;
	privData = sAudpInfo[pipeID].privData;
	up(&sAudpLock);
	if (direction == AUDIOP_PIPE_DIR_UL)
		AUDIO_Ctrl_Trigger(ACTION_AUD_CloseRecord, &param_close, NULL,
				   1);
	else if (direction == AUDIOP_PIPE_DIR_DL)
		AUDIO_Ctrl_Trigger(ACTION_AUD_ClosePlay, &param_close, NULL, 1);

	if (aap_cb)
		aap_cb(AUDIOP_PIPE_DESTROYED, pipeID, 0, 0, 0, 0,
		       (uint32_t) privData);
	else
		DEBUG("audp destroy pipe fail\r\n");

	down(&sAudpLock);
	memset(&sAudpInfo[pipeID], 0, sizeof(sAudpInfo[pipeID]));
	up(&sAudpLock);

}

/*
 *
 * Function Name: audp_device_change_notify
 *
 * Description: called by alsa ctrl to notify aap
 *
 */
void audp_device_change_notify(PIPE_DEVICE_ID_t new_dev,
			       PIPE_DEVICE_ID_t current_dev)
{
	KNLLOG("audp_device_change_notify: new_dev=0x%x curr_dev=0x%x\r\n",
	       new_dev, current_dev);

	if (aap_cb)
		aap_cb(AUDIOP_PIPE_DEVICE_CHANGE, 0, (UInt32) new_dev,
		       (UInt32) current_dev, 0, 0, 0);
	else
		DEBUG("audp cb null\r\n");
}

/*
 *
 * Function Name: audp_device_change_finished
 *
 * Description: called by alsa ctrl to notify aap
 *
 */
void audp_device_change_finished(PIPE_DEVICE_ID_t current_dev,
				 PIPE_DEVICE_ID_t old_dev)
{
#if defined(CONFIG_BCM_KNLLOG_SUPPORT)
	KNLLOG("audp_device_change_finished: curr_dev=0x%x old_dev=0x%x\r\n",
	       current_dev, old_dev);
#endif
	if (aap_cb)
		aap_cb(AUDIOP_PIPE_DEVICE_CHANGED, 0, (UInt32) current_dev,
		       (UInt32) old_dev, 0, 0, 0);
	else
		DEBUG("audp cb null\r\n");
}

/*
 *
 * Function Name: audp_get_pipeID
 *
 * Description: get the pipe id from audp dev
 *
 */
int audp_get_pipeID(PIPE_DEVICE_ID_t dev)
{
	int i, pipeID = -1;
	down(&sAudpLock);
	for (i = 0; i < AUDIOP_MAX_PIPE_SUPPORT; i++)
		/* assume dev and pipe id are 1 to 1 mapped */
		if (sAudpInfo[i].device == dev) {
			pipeID = sAudpInfo[i].pipeID;
			break;
		}
	up(&sAudpLock);
	return pipeID;
}

/*
 *
 * Function Name: audctrl2audp
 *
 * Description: map audio control dev to audp
 *
 */
static PIPE_DEVICE_ID_t audctrl2audp(long param, PIPE_DIRECTION_t direction)
{
	PIPE_DEVICE_ID_t dev = AUDIOP_DEVICE_NONE;
	if (direction & AUDIOP_PIPE_DIR_UL)
		switch ((AUDIO_SOURCE_Enum_t) param) {
		case AUDIO_SOURCE_ANALOG_MAIN:
			dev = AUDIOP_MIC_MAIN;
			break;
		case AUDIO_SOURCE_ANALOG_AUX:
			dev = AUDIOP_MIC_AUX;
			break;
		case AUDIO_SOURCE_DIGI1:
			dev = AUDIOP_MIC_DIGI1;
			break;
		case AUDIO_SOURCE_DIGI2:
			dev = AUDIOP_MIC_DIGI2;
			break;
		case AUDIO_SOURCE_DIGI3:
			dev = AUDIOP_MIC_DIGI3;
			break;
		case AUDIO_SOURCE_DIGI4:
			dev = AUDIOP_MIC_DIGI4;
			break;
		case AUDIO_SOURCE_BTM:
			dev = AUDIOP_MIC_BTM;
			break;
		case AUDIO_SOURCE_USB:
			dev = AUDIOP_MIC_USB;
			break;
		default:
			break;
		};

	if (direction & AUDIOP_PIPE_DIR_DL)
		switch ((AUDIO_SINK_Enum_t) param) {
		case AUDIO_SINK_HANDSET:
			dev = AUDIOP_SPK_HANDSET;
			break;
		case AUDIO_SINK_HEADSET:
			dev = AUDIOP_SPK_HEADSET;
			break;
		case AUDIO_SINK_LOUDSPK:
			dev = AUDIOP_SPK_LOUDSPK;
			break;
		case AUDIO_SINK_VIBRA:
			dev = AUDIOP_SPK_VIBRA;
			break;
		case AUDIO_SINK_BTM:
			dev = AUDIOP_SPK_BTM;
			break;
		case AUDIO_SINK_USB:
			dev = AUDIOP_SPK_USB;
			break;
		default:
			break;
		};

	return dev;
}

/*
 *
 * Function Name: audp_remove_pipe
 *
 * Description: remove a pipe.
 * it first stops the pipe, then closes/destroys the pipe
 *
 */
void audp_remove_pipe(PIPE_DEVICE_ID_t id)
{
	int pipeID;
	/* need api to map from dev id to pipeID */
	pipeID = audp_get_pipeID(id);
#if defined(CONFIG_BCM_KNLLOG_SUPPORT)
	KNLLOG("audp_remove_pipe dev id=0x%x pipeID=0x%x \r\n", id, pipeID);
#endif
	DEBUG("audp_remove_pipe: pipe_id=0x%x\r\n", pipeID);

	audp_stop_pipe(pipeID);
	audp_destroy_pipe(pipeID);
}

/*
 *
 * Function Name: audp_ctrl_handler
 *
 * Description: entry point of audp. handles audp control from amixer control
 *                   1. support 2 kinds of control: create(add) and remove
 *                   2. switch will be done in 2 steps
 *                       a. remove the current pipe b. create or add new pipe
 *
 */
int audp_ctrl_handler(long action, long param, PIPE_DIRECTION_t direction)
{
	PIPE_DEVICE_ID_t dev = AUDIOP_DEVICE_NONE;
#if defined(CONFIG_BCM_KNLLOG_SUPPORT)
	KNLLOG("audp_ctrl_handler action=0x%lx param=0x%lx direction=0x%x\r\n",
	       action, param, direction);
#endif
	DEBUG("audp_ctrl_handler action=0x%lx param=0x%lx direction=0x%x\r\n",
	      action, param, direction);

	dev = audctrl2audp(param, direction);

	if (action == 1) {	/* create pipe */
		if (audp_get_pipeID(dev) < 0)
			audp_create_pipe(dev);
		else {
			DEBUG
			    ("audp_ctrl_handler: pipe already"
			    "created for 0x%x\r\n",
			     dev);
			return -1;
		}
	} else if (action == 2) {	/* remove pipe */
		if (audp_get_pipeID(dev) < 0) {
			DEBUG
			    ("audp_ctrl_handler: pipe did "
			    "not exist for 0x%x\r\n",
			     dev);
			return -1;
		} else
			audp_remove_pipe(dev);
	} else {
		DEBUG("audp_ctrl_handler: unsupported action 0x%lx\r\n",
		      action);
		return -1;
	}
	return 0;
}

/*
 *
 * Function Name: audp_proc_read_pipes
 *
 * Description: read all pipe info in sAudpInfo
 *
 */
static int audp_proc_read_pipes(char *buf, char **start, off_t offset,
				int count, int *eof, void *data)
{
	int limit = count - 200;
	int len = 0;
	int i = 0;

	down(&sAudpLock);

	for (i = 0; i < AUDIOP_MAX_PIPE_SUPPORT; i++) {
		if (len >= limit)
			break;

		len += sprintf(buf + len, "Pipe: %d ", sAudpInfo[i].pipeID);

		len += sprintf(buf + len, "Used: %d ", sAudpInfo[i].used);
		len += sprintf(buf + len, "State: %d ", sAudpInfo[i].state);
		len += sprintf(buf + len, "Dev: %d ", sAudpInfo[i].device);
		len +=
		    sprintf(buf + len, "Int#: %ld ",
			    sAudpInfo[i].CBPrivate.intr_counter);
		len +=
		    sprintf(buf + len, "DmaCh: %d ",
			    sAudpInfo[i].CBPrivate.dmach);
		len +=
		    sprintf(buf + len, "buf_idx: %d ",
			    sAudpInfo[i].CBPrivate.buffer_idx);
		len += sprintf(buf + len, "resync: %ld ", resync_count);
		len += sprintf(buf + len, "\n");
	}
	up(&sAudpLock);
	*eof = 1;

	return len;

}

/*
 *
 * Function Name: audp_debug_pipe
 *
 * Description: a simple debug api
 *
 */
static void audp_debug_pipe()
{
	static struct proc_dir_entry *gAudpDir;

	/* Create /proc/audp/pipes and /proc/audp/devices */
	gAudpDir = create_proc_entry("audp", S_IFDIR | S_IRUGO | S_IXUGO, NULL);

	if (gAudpDir == NULL) {
		printk(KERN_ERR "Unable to create /proc/audp\n");
	} else {
		create_proc_read_entry("pipes", 0, gAudpDir,
				       audp_proc_read_pipes, NULL);
	}
}
