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
/*****************************************************************************
Copyright 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement
governing use of this software, this software is licensed to you under the
terms of the GNU General Public License version 2, available at
http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software
in any way with any other Broadcom software provided under a license other than
the GPL, without Broadcom's express prior written consent.
*****************************************************************************/

#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/jiffies.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/wait.h>
#include <linux/delay.h>
#include <linux/moduleparam.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/module.h>

#include <linux/broadcom/bcm_major.h>
#include <sound/core.h>
#include <sound/control.h>
#include <sound/pcm.h>
#include <sound/rawmidi.h>
#include <sound/initval.h>
#include <plat/kona_reset_reason.h>

#include "mobcom_types.h"
#include "resultcode.h"
#include "audio_consts.h"
#include "bcm_fuse_sysparm_CIB.h"

#include "csl_caph.h"
#include "audio_vdriver.h"
#include "audio_controller.h"
#include "audio_ddriver.h"

#include "logapi.h"
#include "auddrv_audlog.h"
#include "audio_caph.h"
#include "caph_common.h"
#include "bcm_audio.h"
#include "audio_trace.h"
#include "bcmlog.h"
#include "csl_log.h"


/*  Module declarations. */
MODULE_AUTHOR("Broadcom MPS-Audio");
MODULE_DESCRIPTION("Broadcom CAPH sound interface");
MODULE_LICENSE("GPL");

/* global */
int gAudioDebugLevel; /* No default audio logging */
module_param(gAudioDebugLevel, int, 0);

brcm_alsa_chip_t *sgpCaph_chip;

/* AUDIO LOGGING */

#define DATA_TO_READ 4
static int logging_link[LOG_STREAM_NUMBER] = { 0, 0, 0, 0 };

static int dev_use_count;
static int process_logmsg(void *data);
static struct task_struct *audio_log_thread;
AUDLOG_CB_INFO audio_log_cbinfo[LOG_STREAM_NUMBER];
wait_queue_head_t audio_log_queue;
static AUDLOG_CB_INFO audio_log_writecb;

static int read_count;

/* wait queues */
int audio_data_arrived;
int audio_data_gone;
int logpoint_buffer_idx;

/**
 *  DriverProbe: 'probe' call back function
 * @pdev: platform device
 *
 * Returns 0 for success.
 */
static int __devinit DriverProbe(struct platform_device *pdev)
{
	struct snd_card *card;
	int err;

	aTrace(LOG_ALSA_INTERFACE, "ALSA-CAPH Driver Probe:\n");

	aTrace(LOG_ALSA_INTERFACE, "\n %lx:DriverProbe\n", jiffies);

	err = -ENODEV;

	err = -ENOMEM;
	err = snd_card_create(SNDRV_DEFAULT_IDX1, SNDRV_DEFAULT_STR1,
			      THIS_MODULE, sizeof(brcm_alsa_chip_t), &card);

	if (!card)
		goto err;

	sgpCaph_chip = (brcm_alsa_chip_t *) card->private_data;
	sgpCaph_chip->card = card;

	card->dev = &pdev->dev;
	strncpy(card->driver, pdev->dev.driver->name, sizeof(card->driver) - 1);
	/* add Null terminating character */
	card->driver[sizeof(card->driver) - 1] = '\0';
	/* PCM interface */
	err = PcmDeviceNew(card);
	if (err)
		goto err;
	/* CTRL interface */
	err = ControlDeviceNew(card);
	if (err)
		goto err;
	/* HWDEP interface */
	err = HwdepDeviceNew(card);
	if (err)
		goto err;
	/* HWDEP PTT interface */
	err = HwdepPttDeviceNew(card);
	if (err) {
		aError("cannot create PTT HWDEP device\n");
		goto err;
	}

	strcpy(card->driver, "Broadcom");
	strcpy(card->shortname, "Broadcom ALSA");
	sprintf(card->longname, "Broadcom ALSA PCM %i", 0);

	err = snd_card_register(card);
	if (err == 0) {
		int ret;
		platform_set_drvdata(pdev, card);

		ret = BrcmCreateAuddrv_testSysFs(card);
		if (ret != 0)
			aError("ALSA DriverProbe Error to create "
			"sysfs for Auddrv test ret = %d\n", ret);
#ifdef CONFIG_BCM_AUDIO_SELFTEST
		ret = BrcmCreateAuddrv_selftestSysFs(card);
		if (ret != 0)
			aError("ALSA DriverProbe Error to create sysfs"
			" for Auddrv selftest ret = %d\n", ret);
#endif

		return 0;
	}

err:
	aError("\n probe failed =%d\n", err);
	if (card)
		snd_card_free(card);

	sgpCaph_chip = NULL;
	return err;
}

/**
 *  DriverRemove: 'remove' call back function
 * @pdev: platform device
 *
 * Returns 0 for success.
 */
static int DriverRemove(struct platform_device *pdev)
{
	return 0;
}

static int DriverSuspend(struct platform_device *pdev, pm_message_t state)
{
	return 0;
}

/**
 *  DriverResume: 'resume' call back function
 * @pdev: platform device
 *
 * Returns 0 for success.
 */
static int DriverResume(struct platform_device *pdev)
{
	return 0;
}

/*
 * File operations for audio logging
 */

static int BCMAudLOG_open(struct inode *inode, struct file *file)
{

	aTrace(LOG_ALSA_INTERFACE, "ALSA-CAPH BCMLOG_open\n");

	dev_use_count++;
	if (dev_use_count > 1)
		return 0;

	init_waitqueue_head(&audio_log_queue);

	if (!audio_log_thread) {
		audio_log_thread =
		    kthread_run(process_logmsg, 0, "process_logmsg");
	}
	audio_data_arrived = 0;
	audio_data_gone = 0;
	return 0;
}

static int
BCMAudLOG_read(struct file *file, char __user * buf, size_t count,
	       loff_t *ppos)
{
	int ret;
	aTrace(LOG_ALSA_INTERFACE, "ALSA-CAPH BCMLOG_read\n");

	if (wait_event_interruptible(bcmlogreadq, (audio_data_arrived != 0))) {
		/*  Wait for read  ... */
		return -ERESTARTSYS;
	}

	if (audio_data_arrived == 2) {
		aTrace(LOG_ALSA_INTERFACE, "\n BCMLOG_read : dummy reading\n");
		audio_data_arrived = 0;
		return 0;
	}

	audio_data_arrived = 0;

	if (count != 4) {
		ret = count;

		if (audio_log_writecb.pPrivate != NULL) {
			if (count >= audio_log_writecb.size_to_read) {
				count = audio_log_writecb.size_to_read;
				ret =
				    copy_to_user(buf,
						 audio_log_writecb.p_LogRead,
						 count);
			} else {
				ret = count;
			}

		}

		return count - ret;

	} else {
		ret = copy_to_user(buf, "read", 4);
		return DATA_TO_READ;
	}
}

static ssize_t
BCMAudLOG_write(struct file *file, const char __user *buf,
						size_t count, loff_t *ppos)
{
	int number;
	char buffer[642];

	number = copy_from_user(buffer, buf, count);
	if (number != 0)
		aTrace(LOG_ALSA_INTERFACE,
		"\n %s : only copied %d bytes from user\n",
		__func__, count - number);

	count--;
	CSL_LOG_Write(buffer[0], logpoint_buffer_idx, &buffer[1], count);

	return count;
}

static int BCMAudLOG_release(struct inode *inode, struct file *file)
{
	aTrace(LOG_ALSA_INTERFACE, "ALSA-CAPH BCMLOG_release\n");

	dev_use_count--;
	if (dev_use_count > 0) {
		aTrace(LOG_ALSA_INTERFACE,
				"\n BCMLOG_release : 1 dev_use_count = %d\n",
				dev_use_count);
		return -1;
	}

	aTrace(LOG_ALSA_INTERFACE,
			"\n BCMLOG_release : 2 dev_use_count = %d\n",
			dev_use_count);

	if (audio_log_thread) {
		/* kthread_stop(audio_log_thread); */
		audio_log_cbinfo[0].capture_ready = 1;
		wake_up_interruptible(&audio_log_queue);
		audio_log_thread = 0;
		audio_log_cbinfo[0].capture_ready = 0;
		audio_data_arrived = 0;
	}

	aTrace(LOG_ALSA_INTERFACE,
			"\n BCMLOG_release : 3 dev_use_count = %d\n",
			dev_use_count);

	return 0;

}				/* BCMLOG_release */

static int BCMAudLOG_mmap(struct file *filp, struct vm_area_struct *vma)
{
	int ret;
	long length = vma->vm_end - vma->vm_start;

	aTrace(LOG_ALSA_INTERFACE, "ALSA-CAPH BCMLOG_mmap\n");

	/* check length - do not allow larger mappings than the number of
	 * pages allocated
	 */
	if (length > (PAGE_SIZE + (sizeof(LOG_FRAME_t) * 4))) {
		aError("\n Failed at page boundary\n\r");
		return -EIO;
	}

	/* map the whole physically contiguous area in one piece */
	ret = remap_pfn_range(vma, vma->vm_start,
			      virt_to_phys((void *)bcmlog_stream_area) >>
			      PAGE_SHIFT, length, vma->vm_page_prot);

	if (ret != 0) {
		aError("\n BCMLOG_mmap_kmem -EAGAIN \r\n");
		return -EAGAIN;
	}

	return 0;
}

/**
 *  BCMLOG_ioctl - IOCTL to set switch to kernel mode for
 *               1. Audio logging setup
 *               2. Dump Audio data to MTT &  Copy audio data to user space
 *               3. Stop and reset audio logging
 *
 */
static long BCMAudLOG_ioctl(struct file *file, unsigned int cmd,
			    unsigned long arg)
{
	AUDDRV_CFG_LOG_INFO *p_log_info = (AUDDRV_CFG_LOG_INFO *) (arg);
	int index;
	int rtn = 0;
	aTrace(LOG_ALSA_INTERFACE, "ALSA-CAPH BCMLOG_ioctl cmd=0x%x\n", cmd);

	switch (cmd) {
	case BCM_LOG_IOCTL_CONFIG_CHANNEL:

		if ((p_log_info->log_link >= AUDIO_LOG_PATH_1)
		    && (p_log_info->log_link <= AUDIO_LOG_PATH_4)) {
			index = p_log_info->log_link - AUDIO_LOG_PATH_1;

			if ((index >= 0) && (index <= 3)) {
				audio_log_cbinfo[index].capture_ready = FALSE;
				audio_log_cbinfo[index].stream_index = index;
				audio_log_cbinfo[index].capture_point =
				    p_log_info->log_capture_point;
				audio_log_cbinfo[index].consumer =
				    p_log_info->log_consumer;
				spin_lock_init(&audio_log_cbinfo[index].
					       audio_log_lock);

				read_count = 0;
				audio_log_cbinfo[index].dma_count = 0;
			}
		}
		break;

	case BCM_LOG_IOCTL_FLUSH_CHANNEL:

		audio_data_arrived = 2;
		wake_up_interruptible(&bcmlogreadq);
		break;

	case BCM_LOG_IOCTL_STOPPLAY_CHANNEL:
		{
			audio_data_gone = 2;
			wake_up_interruptible(&bcmlogwriteq);
		}
		break;
	case BCM_LOG_IOCTL_GETMSG_CHANNEL:
		{
			STREAM_INFO *p = (STREAM_INFO *) arg;
			STREAM_INFO info;

			aTrace
			    (LOG_ALSA_INTERFACE,
			     "BCMLOG_ioctl : BCM_LOG_IOCTL_GETMSG_CHANNEL\n");
			aTrace(LOG_ALSA_INTERFACE,
					"\n p->stream_index = %d\n",
					p->stream_index);

			index = p->stream_index - AUDIO_LOG_PATH_1;
			if ((index >= 0) && (index <= 3)) {
				info.stream_index = p->stream_index;
				info.wav_info.NumOfChan =
				    audio_log_cbinfo[index].audlog_header.
				    stereoType;
				info.wav_info.bitsPerSample =
				    audio_log_cbinfo[index].audlog_header.
				    bitsPerSample;
				info.wav_info.SamplesPerSec =
				    audio_log_cbinfo[index].audlog_header.
				    samplingRate;

				if (copy_to_user(p, &info, sizeof(info)))
					return -EFAULT;
			} else {
				rtn = -1;
			}
		}
		break;

	case BCM_LOG_IOCTL_WAITFOR:
		{
			if (wait_event_interruptible(bcmlogwriteq,
				(audio_data_gone != 0)))
				audio_data_gone = 0;
		}
		break;

	case BCM_LOG_IOCTL_START_CHANNEL:
		{
			index = p_log_info->log_link - AUDIO_LOG_PATH_1;
			logging_link[index] = 1;
			rtn =
			    AUDDRV_AudLog_Start(p_log_info->log_link,
						p_log_info->log_capture_point,
						p_log_info->log_consumer,
						(char *)NULL);
			if (rtn < 0) {
				aError("\n Couldnt setup channel\n");
				rtn = -1;
			}
		}
		break;

	case BCM_LOG_IOCTL_STOP:
		{
			if ((p_log_info->log_link >= AUDIO_LOG_PATH_1)
			    && (p_log_info->log_link <= AUDIO_LOG_PATH_4)) {
				index = p_log_info->log_link - AUDIO_LOG_PATH_1;

				if ((index >= 0) && (index <= 3)) {
					audio_log_cbinfo[index].capture_ready =
					    0;
					audio_log_cbinfo[index].stream_index =
					    index;
					audio_log_cbinfo[index].capture_point =
					    AUD_LOG_NONE;
					audio_log_cbinfo[index].pPrivate = NULL;
				}
			} else if ((p_log_info->log_link > 0) &&
						(p_log_info->log_link <= 4)) {
				logging_link[p_log_info->log_link - 1] = 0;
				rtn = AUDDRV_AudLog_Stop(p_log_info->log_link);
				if (rtn < 0)
					rtn = -1;
			}
		}
		break;
	default:
		{
			aError("\n Wrong IOCTL cmd\n");
			rtn = -1;
		}
		break;
	}
	return rtn;
}

int logmsg_ready(struct snd_pcm_substream *substream, int log_point)
{

	int i;
	unsigned char *p_read;
	struct snd_pcm_runtime *runtime;

	for (i = 0; i < LOG_STREAM_NUMBER; i++) {

		/* DEBUG("capture_point = %d\n",
		* audio_log_cbinfo[i].capture_point);
		* DEBUG("capture_ready = %d\n",
		* audio_log_cbinfo[i].capture_ready);
		*/

		if (audio_log_cbinfo[i].capture_point == log_point) {
			if (substream) {
				audio_log_cbinfo[i].pPrivate =
				    (struct snd_pcm_substream *)substream;
			}

			spin_lock(&audio_log_cbinfo[i].audio_log_lock);
			audio_log_cbinfo[i].capture_ready = 1;
			spin_unlock(&audio_log_cbinfo[i].audio_log_lock);

			if (audio_log_cbinfo[i].consumer == LOG_TO_PC) {
				aTrace
				    (LOG_ALSA_INTERFACE,
				     "\n Trigger MTT writeing now\n");
				wake_up_interruptible(&audio_log_queue);
			} else {
				aTrace
				    (LOG_ALSA_INTERFACE,
				     "\n Trigger file writeing now\n");

				if (substream) {
					runtime = substream->runtime;
					audio_data_arrived = 1;

					audio_log_cbinfo[i].audlog_header.
					    magicID = 0xA0D10106;
					audio_log_cbinfo[i].audlog_header.
					    logPointID =
					    audio_log_cbinfo[i].stream_index;
					audio_log_cbinfo[i].audlog_header.
					    audioFormat = PCM_TYPE;
					audio_log_cbinfo[i].audlog_header.
					    samplingRate = runtime->rate;
					audio_log_cbinfo[i].audlog_header.
					    stereoType = runtime->channels;
					audio_log_cbinfo[i].audlog_header.
					    bitsPerSample =
					    runtime->sample_bits;

					p_read =
					    (void *)audio_log_cbinfo[i].
					    p_LogRead;
					if (p_read != runtime->dma_area) {
						p_read = runtime->dma_area;
					} else {
						p_read =
						    runtime->dma_area +
						    runtime->dma_bytes / 2;
					}

					audio_log_cbinfo[i].p_LogRead =
					    (void *)p_read;
					audio_log_cbinfo[i].size_to_read =
					    runtime->dma_bytes / 2;
					audio_log_writecb = audio_log_cbinfo[i];
					wake_up_interruptible(&bcmlogreadq);
				}
			}

		}
	}

	return 0;
}

int process_logmsg(void *data)
{
	int i;
	struct snd_pcm_substream *substream;
	struct snd_pcm_runtime *runtime;
	AUDIOLOG_HEADER_t log_header;
	unsigned char *p_dma_area;
	unsigned int sig_code;
	struct BCMLOG_LogLinkList_t link_list[2];
	unsigned short state;
	unsigned short sender;

	while (1) {
		wait_event_interruptible(audio_log_queue,
					 audio_log_cbinfo[0].
					 capture_ready | audio_log_cbinfo[1].
					 capture_ready | audio_log_cbinfo[2].
					 capture_ready | audio_log_cbinfo[3].
					 capture_ready);

		/* DEBUG("\n Capture thread running now\n"); */

		if (dev_use_count == 0) {
			aError("\n Stop process_logmsg thread\n");
			audio_log_writecb.pPrivate = NULL;
			break;
		}

		for (i = 0; i < LOG_STREAM_NUMBER; i++) {
			if (audio_log_cbinfo[i].capture_ready) {
				/* DEBUG("\n Capture stream at
				* capture point %d, stream memory at 0x%x \n",
				* audio_log_cbinfo[i].capture_point,
				* audio_log_cbinfo[i].p_LogRead);
				*/

				/* Preapre audio log messssage header */

				if ((audio_log_cbinfo[i].capture_point ==
				     AUD_LOG_PCMOUT)
				    || (audio_log_cbinfo[i].capture_point ==
					AUD_LOG_PCMIN)) {
					substream =
					    (struct snd_pcm_substream *)
					    audio_log_cbinfo[i].pPrivate;
					if (substream == NULL)
						goto this_path_done;
					runtime = substream->runtime;
					if (runtime == NULL)
						goto this_path_done;

					log_header.magicID = 0xA0D10106;
					log_header.logPointID =
					    audio_log_cbinfo[i].stream_index;
					log_header.audioFormat = PCM_TYPE;
					log_header.samplingRate = runtime->rate;
					log_header.stereoType =
					    runtime->channels;
					log_header.bitsPerSample =
					    runtime->sample_bits;
					log_header.frame_size =
					    runtime->dma_bytes / 2;
				} else {
					/* Prepare other format header */
				}

				link_list[0].byte_array = (void *)&log_header;
				link_list[0].size = sizeof(log_header);

				/* Preapre audio messssage stream */

				if ((audio_log_cbinfo[i].capture_point ==
				     AUD_LOG_PCMOUT)
				    || (audio_log_cbinfo[i].capture_point ==
					AUD_LOG_PCMIN)) {
					substream =
					    (struct snd_pcm_substream *)
					    audio_log_cbinfo[i].pPrivate;
					if (substream == NULL)
						goto this_path_done;
					runtime = substream->runtime;
					if (runtime == NULL)
						goto this_path_done;

					p_dma_area =
					    (void *)audio_log_cbinfo[i].
					    p_LogRead;

					if (p_dma_area == runtime->dma_area) {
						p_dma_area =
						    runtime->dma_area +
						    runtime->dma_bytes / 2;

					} else {
						p_dma_area = runtime->dma_area;
					}

					audio_log_cbinfo[i].p_LogRead =
					    (void *)p_dma_area;


					link_list[1].byte_array = p_dma_area;
					link_list[1].size =
						runtime->dma_bytes / 2;

				} else {
					/* Prepare other path data chunk */
				}

				sig_code = AUDIO_DATA;
				state = 0;
				sender = P_log_general;

#ifdef CONFIG_BRCM_FUSE_LOG
				BCMLOG_LogLinkList(sig_code,
					link_list, 2, state, sender);
#endif

this_path_done:
				spin_lock(&audio_log_cbinfo[i].audio_log_lock);
				audio_log_cbinfo[i].capture_ready = 0;
				spin_unlock(&audio_log_cbinfo[i].
					    audio_log_lock);

			}
		}

	}

	return 0;
}

/* Platform device structure */
static struct platform_device sgPlatformDevice = {
	.name = "brcm_caph_device",
	.id = -1,
};

/* Platfoorm driver structure */
static struct platform_driver __refdata sgPlatformDriver = {
	/*
	 * probe is assigned in ALSAModuleInit
	 * because of "section mismatch" warning.
	 */
	/*	.probe = DriverProbe, */
	.remove = __devexit_p(DriverRemove),
	.suspend = DriverSuspend,
	.resume = DriverResume,
	.driver = {
		   .name = "brcm_caph_device",
		   .owner = THIS_MODULE,
		   },
};

static struct class *audlog_class;
/* File operations for audio logging */
static const struct file_operations bcmlog_fops = {
	.owner = THIS_MODULE,
	.open = BCMAudLOG_open,
	.read = BCMAudLOG_read,
	.write = BCMAudLOG_write,
	.unlocked_ioctl = BCMAudLOG_ioctl,
	.mmap = BCMAudLOG_mmap,
	.release = BCMAudLOG_release,
};

/**
 * ModuleInit: Module initialization
 *
*/
static int __devinit ALSAModuleInit(void)
{
	int err = 0;

	aTrace(LOG_ALSA_INTERFACE, "ALSA Module init called:\n");
	if (is_ap_only_boot()) {
		/* don't register audio driver for AP only boot mode */
		aTrace(LOG_ALSA_INTERFACE, "AP Only Boot\n");
		return 0;
	}

	err = platform_device_register(&sgPlatformDevice);
	aTrace(LOG_ALSA_INTERFACE, "\n %lx:device register done %d\n"
			, jiffies, err);
	if (err)
		return err;

	sgPlatformDriver.probe = DriverProbe;
	err = platform_driver_register(&sgPlatformDriver);
	aTrace(LOG_ALSA_INTERFACE, "\n %lx:driver register done %d\n"
			, jiffies, err);
	if (err)
		return err;

	LaunchAudioCtrlThread();

	/* Device for audio logging */

	err = register_chrdev(BCM_ALSA_LOG_MAJOR, "bcm_audio_log",
		&bcmlog_fops);
	if (err < 0)
		return err;
	audlog_class = class_create(THIS_MODULE, "bcm_audio_log");
	if (IS_ERR(audlog_class))
		return PTR_ERR(audlog_class);

	device_create(audlog_class, NULL, MKDEV(BCM_ALSA_LOG_MAJOR, 0), NULL,
		      "bcm_audio_log");
	init_waitqueue_head(&bcmlogreadq);
	init_waitqueue_head(&bcmlogwriteq);
	memset(audio_log_cbinfo, 0, sizeof(audio_log_cbinfo));

	return err;
}

/**
 * ModuleExit: Module de-initialization
*
*/
static void __devexit ALSAModuleExit(void)
{

	aTrace(LOG_ALSA_INTERFACE, "\n %lx:ModuleExit\n", jiffies);
	if (is_ap_only_boot()) {
		/* AP only boot mode - no need to de-register */
		aTrace(LOG_ALSA_INTERFACE, "AP Only Boot\n");
		return;
	}

	snd_card_free(sgpCaph_chip->card);

	platform_driver_unregister(&sgPlatformDriver);

	platform_device_unregister(&sgPlatformDevice);
	TerminateAudioHalThread();

	aTrace(LOG_ALSA_INTERFACE, "\n %lx:exit done\n", jiffies);
}

/* lower down the CAPH module init priority, so it can be done after RPC init*/
module_init(ALSAModuleInit);
/* late_initcall(ALSAModuleInit); */
module_exit(ALSAModuleExit);
