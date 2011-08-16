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
/*******************************************************************************************
Copyright 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement 
governing use of this software, this software is licensed to you under the 
terms of the GNU General Public License version 2, available at 
http://www.gnu.org/copyleft/gpl.html (the "GPL"). 

Notwithstanding the above, under no circumstances may you combine this software 
in any way with any other Broadcom software provided under a license other than 
the GPL, without Broadcom's express prior written consent.
*******************************************************************************************/

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
#include <sound/hwdep.h>


#include "mobcom_types.h"
#include "resultcode.h"
#include "audio_consts.h"

#include "csl_aud_drv.h"
#include "audio_controller.h"
#include "dspif_voice_play.h"
#include "audio_ddriver.h"
#include "bcm_audio_devices.h"
#include "bcm_audio_thread.h"
#include "caph_common.h"

#include "shared.h"
#include "csl_dsp.h"
#include "osdw_dsp_drv.h"
#include "csl_audio_render.h"


enum {
	VoIP_Ioctl_GetVersion = _IOR ('H', 0x10, int),
	VoIP_Ioctl_StartUL = _IOW ('H', 0x11, int),
	VoIP_Ioctl_StopUL = _IOW ('H', 0x12, int),	
	VoIP_Ioctl_SetParms = _IOR('H', 0x13, int),
	VoIP_Ioctl_GetParms   = _IOW('H', 0x14, int)
};

typedef	enum{
	VoIP_Hwdep_Status_None,
	VoIP_Hwdep_Status_Opened,
	VoIP_Hwdep_Status_Started,
	VoIP_Hwdep_Status_Stopped,
	VoIP_Hwdep_Status_Released,
}VoIP_Hwdep_Status_t;


typedef struct __bcm_caph_hwdep
{
	struct timer_list dbg_timer;
	int	status_ul;
	int	frames_available_to_read;
	//scheduling
	wait_queue_head_t sleep;
}bcm_caph_hwdep_t;



//+++++++++++ debugging only +++++++++++++++
static unsigned char spu8SineWaveBuf[] = {
#include "1khz8kmono16bit.txt"
};
#define	TIMER_INTERVAL_MS	20
#define	VOIP_SAMPLERATE		8000
int	FillWithSineWave(char __user *pbuf, unsigned int sizeInBytes)
{
	static	unsigned int rp=0;
	
	if(!sizeInBytes)
		return 0;
	if( (sizeof(spu8SineWaveBuf)- (rp%sizeof(spu8SineWaveBuf)))>=sizeInBytes)
	{
		if(0!= copy_to_user(pbuf, &spu8SineWaveBuf[(rp%sizeof(spu8SineWaveBuf))], sizeInBytes))
			BCM_AUDIO_DEBUG("%s Error copy_to_user", __FUNCTION__);
	}
	else
	{
		unsigned int blk1, blk2;
		blk1 = (sizeof(spu8SineWaveBuf)- (rp%sizeof(spu8SineWaveBuf)));
		blk2 = sizeInBytes - blk1;
		if(0!=copy_to_user(pbuf, &spu8SineWaveBuf[(rp%sizeof(spu8SineWaveBuf))], blk1))
			BCM_AUDIO_DEBUG("%s Error copy_to_user", __FUNCTION__);;
		if(0!=copy_to_user(pbuf+blk1, spu8SineWaveBuf, blk2))
			BCM_AUDIO_DEBUG("%s Error copy_to_user", __FUNCTION__);
	}

	rp += sizeInBytes;

	return sizeInBytes;
}

//---------------------------------------------------------

long voip_read(struct snd_hwdep *hw, char __user *buf,	 long count, loff_t *offset)
{
	bcm_caph_hwdep_t	*pVoIP;
	pVoIP = (bcm_caph_hwdep_t	*)hw->private_data;
	

	if(pVoIP->status_ul == VoIP_Hwdep_Status_Started && pVoIP->frames_available_to_read>0)
	{
		int size;
		size = pVoIP->frames_available_to_read*TIMER_INTERVAL_MS*VOIP_SAMPLERATE/1000*2;
		if(size>count)
			size = count;
		pVoIP->frames_available_to_read -= size/(TIMER_INTERVAL_MS*VOIP_SAMPLERATE/1000*2);
		BCM_AUDIO_DEBUG("voip_read size=%d\n", size);
		return FillWithSineWave(buf, size);
		
	}
	else
		return 0;
}

long voip_write(struct snd_hwdep *hw, const char __user *buf, long count, loff_t *offset)
{
	return 0;
}


void TimerCbFunction(unsigned long data)
{
	struct snd_hwdep *hw = (struct snd_hwdep *)data;
	bcm_caph_hwdep_t	*pVoIP = (bcm_caph_hwdep_t	*)hw->private_data;

	if(pVoIP->status_ul == VoIP_Hwdep_Status_Started)
	{
		BCM_AUDIO_DEBUG("mod_timer TimerCbFunction frames_available_to_read=%d\n", pVoIP->frames_available_to_read);
		pVoIP->frames_available_to_read++;
		mod_timer(&pVoIP->dbg_timer, jiffies + msecs_to_jiffies(TIMER_INTERVAL_MS));
	}
	else
		BCM_AUDIO_DEBUG("TimerCbFunction frames_available_to_read=%d\n", pVoIP->frames_available_to_read);
	
	wake_up(&pVoIP->sleep);
}
	


int voip_open(struct snd_hwdep *hw, struct file * file)
{
	bcm_caph_hwdep_t	*pVoIP;
	hw->private_data = kzalloc(sizeof(bcm_caph_hwdep_t), GFP_KERNEL);
	CAPH_ASSERT(hw->private_data);
	pVoIP = (bcm_caph_hwdep_t	*)hw->private_data;
	init_timer(&pVoIP->dbg_timer);
    pVoIP->dbg_timer.function = TimerCbFunction;
	pVoIP->dbg_timer.data = (unsigned long)hw;
	init_waitqueue_head(&pVoIP->sleep);

	pVoIP->status_ul = VoIP_Hwdep_Status_Opened;
	BCM_AUDIO_DEBUG("VoIP_Ioctl_Open\n");

	return 0;
}


int voip_release(struct snd_hwdep *hw, struct file * file)
{	
	bcm_caph_hwdep_t	*pVoIP;
	pVoIP = (bcm_caph_hwdep_t	*)hw->private_data;
	del_timer_sync(&pVoIP->dbg_timer);
	pVoIP->status_ul = VoIP_Hwdep_Status_Released;
	BCM_AUDIO_DEBUG("VoIP_Ioctl_Release\n");
	return 0;
}

int voip_ioctl(struct snd_hwdep *hw, struct file *file, unsigned int cmd, unsigned long arg)
{
//		void __user *argp = (void __user *)arg;
		bcm_caph_hwdep_t	*pVoIP;
		pVoIP = (bcm_caph_hwdep_t	*)hw->private_data;
		
		
		BCM_AUDIO_DEBUG("voip_ioctl cmd=%d\n",cmd);
		switch (cmd) {
//			case VoIP_Ioctl_GetVersion:
//				return put_user(BrcmAACEncVersion, (int __user *)argp);
//				break;
			case VoIP_Ioctl_StartUL:
				pVoIP->dbg_timer.expires = jiffies + msecs_to_jiffies(TIMER_INTERVAL_MS);
				add_timer(&pVoIP->dbg_timer);
				pVoIP->status_ul = VoIP_Hwdep_Status_Started;
				pVoIP->frames_available_to_read++;
				
				wake_up(&pVoIP->sleep);
				
				BCM_AUDIO_DEBUG("VoIP_Ioctl_StartUL\n");
				break;
			case VoIP_Ioctl_StopUL:
				pVoIP->status_ul = VoIP_Hwdep_Status_Stopped;
				
				wake_up(&pVoIP->sleep);
				BCM_AUDIO_DEBUG("VoIP_Ioctl_StopUL\n");
				break;
//			case VoIP_Ioctl_SetParms:
//				break;
//			case VoIP_Ioctl_GetParms:
//				break;
	
			default:
				return -ENOTTY;
	
		}
	return 0;
}

int voip_mmap(struct snd_hwdep *hw, struct file *file, struct vm_area_struct *vma)
{
	return 0;
}

int voip_dsp_status(struct snd_hwdep *hw, struct snd_hwdep_dsp_status *status)
{
	return 0;
}

void voip_private_free (struct snd_hwdep *hwdep)
{
	if(hwdep->private_data)
		kfree(hwdep->private_data);
}


static unsigned int voip_poll(struct snd_hwdep *hw, struct file *file, poll_table *wait)
{	
	unsigned int mask=0;
	bcm_caph_hwdep_t	*pVoIP;
	
	pVoIP = (bcm_caph_hwdep_t	*)hw->private_data;
	
	poll_wait(file, &pVoIP->sleep, wait);
	if(pVoIP->status_ul==VoIP_Hwdep_Status_Started)
	{
		if(pVoIP->frames_available_to_read>0)
			mask = POLLIN | POLLRDNORM;
	}
		
	return mask;
}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  Function Name: HwdepDeviceNew
//
//  Description: Create HWDEP device for VoIP
//
//------------------------------------------------------------
int __devinit HwdepDeviceNew(struct snd_card *card)
{
	int err = 0;
	struct	snd_hwdep	*pHwdep;
	
	err = snd_hwdep_new(card, "Broadcom CAPH VOIP", 0, &pHwdep);
	if (err<0)
	{
		BCM_AUDIO_DEBUG("error create hwdep device\n");
		return err;
	}
	pHwdep->iface = SNDRV_HWDEP_IFACE_OPL4;
	pHwdep->ops.read = voip_read;
	pHwdep->ops.write = voip_write;
	pHwdep->ops.open = voip_open;
	pHwdep->ops.release = voip_release;
	pHwdep->ops.ioctl = voip_ioctl;
	pHwdep->ops.mmap = voip_mmap;
	pHwdep->ops.dsp_status = voip_dsp_status;
	pHwdep->ops.poll = voip_poll;
	pHwdep->private_free = voip_private_free;
	
	return err;
}

