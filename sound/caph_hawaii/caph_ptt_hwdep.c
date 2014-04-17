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
Copyright 2010-2011 Broadcom Corporation.  All rights reserved.

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
#include <linux/spinlock.h>

#include <sound/core.h>
#include <sound/control.h>
#include <sound/pcm_params.h>
#include <sound/pcm.h>
#include <sound/rawmidi.h>
#include <sound/initval.h>
#include <sound/hwdep.h>

#include "mobcom_types.h"
#include <plat/osdal_os.h>
#include "resultcode.h"
#include "audio_consts.h"
#include "audio_trace.h"
#include "bcm_fuse_sysparm_CIB.h"
#include "csl_caph.h"
#include "audio_vdriver.h"
#include "audio_ddriver.h"
#include "audio_caph.h"
#include "caph_common.h"
#include "csl_dsp.h"
#include "csl_ptt.h"
#include "bcm_audio.h"

/* Local defines */

#define PTT_FRAME_SIZE 320
#define PTT_WRITE_TIMEOUT 10000


enum __Ptt_Hwdep_Status_t {
	Ptt_Hwdep_Status_None,
	Ptt_Hwdep_Status_Opened,
	Ptt_Hwdep_Status_Started,
	Ptt_Hwdep_Status_Stopped,
	Ptt_Hwdep_Status_Released,
};

enum __Ptt_Hwdep_ioctl_t {
	Ptt_Ioctl_Enable,
	Ptt_Ioctl_Disable
};


struct __bcm_caph_hwdep_ptt {
	int status;
	int writecount;
	int underrun;
	wait_queue_head_t sleep;
	spinlock_t ptt_spin_lock;
	Int16 *ptt_dsp_buf_ptr;
	UInt32 ptt_dsp_buf_size;
	AUDIO_DRIVER_HANDLE_t drv_handle;
};
#define	bcm_caph_hwdep_ptt_t struct __bcm_caph_hwdep_ptt


static u32 pttInstCount;

/* local functions */

static void ptt_fill_dl(void *pPrivate, Int16 * pDst, u32 nSize);
static long hwdep_ptt_read(struct snd_hwdep *hw, char __user * buf, long count,
		       loff_t *offset);
static long hwdep_ptt_write(struct snd_hwdep *hw, const char __user * buf,
			long count, loff_t *offset);
static int hwdep_ptt_open(struct snd_hwdep *hw, struct file *file);
static int hwdep_ptt_release(struct snd_hwdep *hw, struct file *file);
static int hwdep_ptt_ioctl(struct snd_hwdep *hw, struct file *file,
		       unsigned int cmd, unsigned long arg);
static int hwdep_ptt_mmap(struct snd_hwdep *hw, struct file *file,
		      struct vm_area_struct *vma);
static int hwdep_ptt_dsp_status(struct snd_hwdep *hw,
			    struct snd_hwdep_dsp_status *status);
static void hwdep_ptt_private_free(struct snd_hwdep *hwdep);
static unsigned int hwdep_ptt_poll(struct snd_hwdep *hw, struct file *file,
			       poll_table *wait);


static void ptt_fill_dl(void *pPrivate, Int16* pDst, u32 nSize)
{
	unsigned long flags;
	bcm_caph_hwdep_ptt_t *pPtt;
	pPtt = (bcm_caph_hwdep_ptt_t *) pPrivate;
	if (pDst == NULL)
		return;
	if (nSize != PTT_FRAME_SIZE)
		return;
	if (pPtt->writecount) {
		pPtt->underrun = 1;
		memset((u8 *)pDst, 0, nSize);
	} else
		pPtt->underrun = 0;

	/* update the write pointer and wakeup write thread */
	spin_lock_irqsave(&pPtt->ptt_spin_lock, flags);
	pPtt->ptt_dsp_buf_ptr = pDst;
	pPtt->ptt_dsp_buf_size = nSize;
	pPtt->writecount = 1;
	spin_unlock_irqrestore(&pPtt->ptt_spin_lock, flags);
	wake_up(&pPtt->sleep);
}

static long hwdep_ptt_read(struct snd_hwdep *hw, char __user * buf, long count,
		       loff_t *offset)
{

	return 0;
}

static long hwdep_ptt_write(struct snd_hwdep *hw, const char __user * buf,
			long count, loff_t *offset)
{
	bcm_caph_hwdep_ptt_t *pPtt;
	long ret = -EBUSY;
	long total_size = count;
	long to_copy = 0;
	pPtt = (bcm_caph_hwdep_ptt_t *) hw->private_data;

	/* DEBUG("voip_write pVoIP->frame_size %d,pVoIP->"
	 * "writecount %d\n",pVoIP->frame_size,pVoIP->writecount);
	 */


	if (pPtt->status == Ptt_Hwdep_Status_Started) {
		if (count % PTT_FRAME_SIZE)
			aTrace(LOG_ALSA_INTERFACE,
					"Buffer size not multiple!\n");
		while (count > 0) {
			char  *dsp_buf;
			unsigned long flags;
			ret = wait_event_interruptible_timeout(pPtt->sleep,
					(pPtt->writecount || pPtt->status ==
					 Ptt_Hwdep_Status_Stopped),
					msecs_to_jiffies(PTT_WRITE_TIMEOUT));
			if (ret < 0)
				goto error;
			if (pPtt->status == Ptt_Hwdep_Status_Stopped) {
				ret = -EBUSY;
				goto error;
			}
			if (ret == 0) {
				ret = -ETIMEDOUT;
				goto error;
			}
			if (pPtt->underrun)
				aWarn("underrun occured\n");
			if (pPtt->ptt_dsp_buf_ptr == NULL)
				continue;
			to_copy = (count > PTT_FRAME_SIZE) ? PTT_FRAME_SIZE
					: count;
			spin_lock_irqsave(&pPtt->ptt_spin_lock, flags);
			dsp_buf = (char *) pPtt->ptt_dsp_buf_ptr;
			/* update write count */
			pPtt->writecount = 0;
			spin_unlock_irqrestore(&pPtt->ptt_spin_lock, flags);
			/* copy one frame of data */
			ret = copy_from_user(dsp_buf, buf,
					to_copy);
			buf += to_copy;
			count -= to_copy;
		}
	} else {
		ret = -EBUSY;
		goto error;
	}

	ret = total_size - count;
error:
	return ret;
}

static int hwdep_ptt_open(struct snd_hwdep *hw, struct file *file)
{
	bcm_caph_hwdep_ptt_t *pPtt;
	/* if device is already opened return */
	if (pttInstCount > 0)
		return -1;

	aTrace(LOG_ALSA_INTERFACE , "ALSA-CAPH PTT Open\n");
	hw->private_data = kzalloc(sizeof(bcm_caph_hwdep_ptt_t), GFP_KERNEL);
	CAPH_ASSERT(hw->private_data);
	pPtt = (bcm_caph_hwdep_ptt_t *) hw->private_data;
	init_waitqueue_head(&pPtt->sleep);
	pPtt->writecount = 2;
	pPtt->status = Ptt_Hwdep_Status_Opened;
	pPtt->ptt_dsp_buf_ptr = NULL;
	pPtt->ptt_dsp_buf_size = 0;
	pPtt->drv_handle = NULL;
	pPtt->underrun = 0;
	spin_lock_init(&pPtt->ptt_spin_lock);

	pttInstCount = 1;
	return 0;
}

static int hwdep_ptt_release(struct snd_hwdep *hw, struct file *file)
{
	bcm_caph_hwdep_ptt_t *pPtt;
	pPtt = (bcm_caph_hwdep_ptt_t *) hw->private_data;
	pttInstCount = 0;
	pPtt->writecount = 0;
	pPtt->status = Ptt_Hwdep_Status_Released;
	aTrace(LOG_ALSA_INTERFACE , "ALSA-CAPH PTT release\n");
	return 0;
}

static int hwdep_ptt_ioctl(struct snd_hwdep *hw, struct file *file,
		       unsigned int cmd, unsigned long arg)
{
	bcm_caph_hwdep_ptt_t *pPtt;
	AUDIO_DRIVER_CallBackParams_t cbParams;
	int ret = 0;
	int count;

	pPtt = (bcm_caph_hwdep_ptt_t *) hw->private_data;

	aTrace(LOG_ALSA_INTERFACE, "ALSA-CAPH hwdep_ptt_ioctl cmd=%d\n", cmd);

	switch (cmd) {

	case Ptt_Ioctl_Enable:
		{
		UInt32 buf_index = 0;
		Int16  *buf_ptr;
		aTrace(LOG_ALSA_INTERFACE, "Ptt_Ioctl_Enable\n");

		pPtt->drv_handle = AUDIO_DRIVER_Open(AUDIO_DRIVER_PTT);
		if (!pPtt->drv_handle)
			return -ENOMEM;

		/* set DL callback */
		cbParams.pttDLCallback = ptt_fill_dl;
		cbParams.pPrivateData = (void *)pPtt;
		AUDIO_DRIVER_Ctrl(pPtt->drv_handle, AUDIO_DRIVER_SET_PTT_CB ,
				(void *)&cbParams);

		/* clear the buffers before starting the PTT */
		for (count = 0; count < 2; count++) {
			buf_index = count;
			AUDIO_DRIVER_Ctrl(pPtt->drv_handle,
					AUDIO_DRIVER_GET_PTT_BUFFER,
					(void *)(&buf_index));
			buf_ptr = (Int16 *)buf_index;
			memset(buf_ptr, 0, PTT_FRAME_SIZE);
			pPtt->writecount--;
		}
		AUDIO_DRIVER_Ctrl(pPtt->drv_handle,
				  AUDIO_DRIVER_START, NULL);
		pPtt->status = Ptt_Hwdep_Status_Started;
		}

		break;
	case Ptt_Ioctl_Disable:
		{
		aTrace(LOG_ALSA_INTERFACE, "Ptt_Ioctl_Disable\n");

		AUDIO_DRIVER_Ctrl(pPtt->drv_handle,
					  AUDIO_DRIVER_STOP, NULL);
		AUDIO_DRIVER_Close(pPtt->drv_handle);
		pPtt->status = Ptt_Hwdep_Status_Stopped;
		wake_up(&pPtt->sleep);
		}
		break;
	default:
		ret = -ENOTTY;
		break;

	}
	return ret;
}

static int hwdep_ptt_mmap(struct snd_hwdep *hw, struct file *file,
		      struct vm_area_struct *vma)
{
	return 0;
}

static int hwdep_ptt_dsp_status(struct snd_hwdep *hw,
			    struct snd_hwdep_dsp_status *status)
{
	return 0;
}

static void hwdep_ptt_private_free(struct snd_hwdep *hwdep)
{
	kfree(hwdep->private_data);
}

static unsigned int hwdep_ptt_poll(struct snd_hwdep *hw, struct file *file,
			       poll_table *wait)
{
	unsigned int mask = 0;
	bcm_caph_hwdep_ptt_t *pPtt;

	pPtt = (bcm_caph_hwdep_ptt_t *) hw->private_data;
	poll_wait(file, &pPtt->sleep, wait);
	if (pPtt->status == Ptt_Hwdep_Status_Started) {

		if (pPtt->writecount > 0)	/* buffer available to write */
			mask |= POLLOUT | POLLWRNORM;
	}

	/* DEBUG("voip_poll mask %ld\n",mask); */

	return mask;
}

/**
 * HwdepPttDeviceNew: Create HWDEP device for PTT
 *
 * Return 0 for success
 */
int HwdepPttDeviceNew(struct snd_card *card)
{
	int err = 0;
	struct snd_hwdep *pHwdep;

	err = snd_hwdep_new(card, "Broadcom CAPH PTT", 1, &pHwdep);
	if (err < 0) {
		aError("error create ptt hwdep device\n");
		return err;
	}
	pHwdep->iface = SNDRV_HWDEP_IFACE_OPL4;
	pHwdep->ops.read = hwdep_ptt_read;
	pHwdep->ops.write = hwdep_ptt_write;
	pHwdep->ops.open = hwdep_ptt_open;
	pHwdep->ops.release = hwdep_ptt_release;
	pHwdep->ops.ioctl = hwdep_ptt_ioctl;
	pHwdep->ops.mmap = hwdep_ptt_mmap;
	pHwdep->ops.dsp_status = hwdep_ptt_dsp_status;
	pHwdep->ops.poll = hwdep_ptt_poll;
	pHwdep->private_free = hwdep_ptt_private_free;

	return err;
}
