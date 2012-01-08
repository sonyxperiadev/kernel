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

#include <sound/core.h>
#include <sound/control.h>
#include <sound/pcm_params.h>
#include <sound/pcm.h>
#include <sound/rawmidi.h>
#include <sound/initval.h>
#include <sound/hwdep.h>

#include "mobcom_types.h"
#include "osdal_os.h"
#include "resultcode.h"
#include "audio_consts.h"

#include "bcm_fuse_sysparm_CIB.h"
#include "csl_caph.h"
#include "audio_vdriver.h"
#include "audio_controller.h"
#include "audio_ddriver.h"
#include "audio_caph.h"
#include "caph_common.h"

#include "csl_voip.h"
#include "csl_dsp.h"
#include "osdw_dsp_drv.h"
#include "csl_audio_render.h"
#include "bcm_audio.h"

/* Local defines */

#define VOIP_FRAMES_IN_BUFFER 1

enum __VoIP_Hwdep_Status_t {
	VoIP_Hwdep_Status_None,
	VoIP_Hwdep_Status_Opened,
	VoIP_Hwdep_Status_Started,
	VoIP_Hwdep_Status_Stopped,
	VoIP_Hwdep_Status_Released,
};

struct __audio_voip_driver {
	u32 voip_data_dl_rd_index;
	u32 voip_data_dl_wr_index;
	u32 voip_data_ul_rd_index;
	u32 voip_data_ul_wr_index;
	u8 *voip_data_dl_buf_ptr;
	u8 *voip_data_ul_buf_ptr;
	AUDIO_DRIVER_HANDLE_t drv_handle;
};
#define	audio_voip_driver_t	struct __audio_voip_driver

struct __bcm_caph_hwdep_voip {
	int status;
	int frames_available_to_read;
	int frames_available_to_write;
	int writecount;
	wait_queue_head_t sleep;
	audio_voip_driver_t *buffer_handle;
	AUDIO_SOURCE_Enum_t mic;
	AUDIO_SINK_Enum_t spk;
	u32 codec_type;
	u8 voip_type;
	u32 frame_size;
	u32 buffer_size;
};
#define	bcm_caph_hwdep_voip_t struct __bcm_caph_hwdep_voip

static const u16 sVoIPFrameLen[] = { 320, 158, 36, 164, 640, 68 };

static u16 sVoIPAMRSilenceFrame[1] = { 0x000f };

static u32 voipInstCnt;
static voip_data_t voip_data;
static Boolean setdefault = FALSE;

/* local functions */
static void HWDEP_VOIP_DumpUL_CB(void *pPrivate, u8 * pSrc, u32 nSize);
static void HWDEP_VOIP_FillDL_CB(void *pPrivate, u8 * pDst, u32 nSize);
static void FillSilenceFrame(u32 codec_type, u32 frame_size, u8 *pDst);
static long hwdep_read(struct snd_hwdep *hw, char __user * buf, long count,
		       loff_t *offset);
static long hwdep_write(struct snd_hwdep *hw, const char __user * buf,
			long count, loff_t *offset);
static int hwdep_open(struct snd_hwdep *hw, struct file *file);
static int hwdep_release(struct snd_hwdep *hw, struct file *file);
static int hwdep_ioctl(struct snd_hwdep *hw, struct file *file,
		       unsigned int cmd, unsigned long arg);
static int hwdep_mmap(struct snd_hwdep *hw, struct file *file,
		      struct vm_area_struct *vma);
static int hwdep_dsp_status(struct snd_hwdep *hw,
			    struct snd_hwdep_dsp_status *status);
static void hwdep_private_free(struct snd_hwdep *hwdep);
static unsigned int hwdep_poll(struct snd_hwdep *hw, struct file *file,
			       poll_table *wait);

static void FillSilenceFrame(u32 codec_type, u32 frame_size, u8 *pDst)
{
	CSL_VOIP_Buffer_t tmpBuf;
	memset(&tmpBuf, 0, sizeof(CSL_VOIP_Buffer_t));

	if (codec_type == VoIP_Codec_AMR475)
		tmpBuf.voip_frame.frame_amr[0] = sVoIPAMRSilenceFrame[0];
	else if (codec_type == VOIP_Codec_AMR_WB_7K)
		tmpBuf.voip_frame.frame_amr_wb.frame_type = 0x7;
	else if (codec_type == VoIP_Codec_FR) {
		tmpBuf.voip_frame.frame_fr[0] = 1;
		tmpBuf.voip_frame.frame_fr[1] = 0;
		tmpBuf.voip_frame.frame_fr[2] = 0;
	} else if (codec_type == VOIP_Codec_G711_U) {
		tmpBuf.voip_frame.frame_g711[0].frame_type = 1;
		tmpBuf.voip_frame.frame_g711[1].frame_type = 1;
	}
	memcpy(pDst, &(tmpBuf.voip_frame), frame_size);
}

static void HWDEP_VOIP_DumpUL_CB(void *pPrivate, u8 * pSrc, u32 nSize)
{
	bcm_caph_hwdep_voip_t *pVoIP;
	pVoIP = (bcm_caph_hwdep_voip_t *) pPrivate;

	/* BCM_AUDIO_DEBUG("HWDEP_VOIP_DumpUL_CB nSize %d pVoIP 0x%x\n",
	 * nSize,pVoIP);
	 */

	if (pVoIP->buffer_handle) {
		if (pVoIP->buffer_handle->voip_data_ul_buf_ptr) {
			memcpy(pVoIP->buffer_handle->voip_data_ul_buf_ptr +
			       pVoIP->buffer_handle->voip_data_ul_wr_index,
			       pSrc, nSize);
			pVoIP->frames_available_to_read++;
#ifdef CONFIG_VOIP_BUFFER_INCREASE
			pVoIP->buffer_handle->voip_data_ul_wr_index += nSize;
			if (pVoIP->buffer_handle->voip_data_ul_wr_index >=
			    pVoIP->buffer_size) {
				pVoIP->buffer_handle->voip_data_ul_wr_index -=
				    pVoIP->buffer_size;
			}
#endif
		}
	}

	wake_up(&pVoIP->sleep);
}

static void HWDEP_VOIP_FillDL_CB(void *pPrivate, u8 * pDst, u32 nSize)
{
	bcm_caph_hwdep_voip_t *pVoIP;
	pVoIP = (bcm_caph_hwdep_voip_t *) pPrivate;

	if (pVoIP->buffer_handle->voip_data_dl_buf_ptr) {
		/*BCM_AUDIO_DEBUG("HWDEP_VOIP_FillDL_CB pVoIP->"
		 *"frames_available_to_write %d\n", pVoIP->
		 * frames_available_to_write);
		 */
		if (pVoIP->frames_available_to_write == 0) {
			/* fill with silent data based on the frame type  */
			FillSilenceFrame(pVoIP->codec_type, nSize, pDst);
		} else {
			memcpy(pDst,
			       pVoIP->buffer_handle->voip_data_dl_buf_ptr +
			       pVoIP->buffer_handle->voip_data_dl_rd_index,
			       nSize);
			pVoIP->frames_available_to_write--;
			pVoIP->writecount++;
#ifdef CONFIG_VOIP_BUFFER_INCREASE
			pVoIP->buffer_handle->voip_data_dl_rd_index += nSize;
			if (pVoIP->buffer_handle->voip_data_dl_rd_index >=
			    pVoIP->buffer_size) {
				pVoIP->buffer_handle->voip_data_dl_rd_index -=
				    pVoIP->buffer_size;
			}
#endif
		}
	}
	wake_up(&pVoIP->sleep);
}

static long hwdep_read(struct snd_hwdep *hw, char __user * buf, long count,
		       loff_t *offset)
{
	bcm_caph_hwdep_voip_t *pVoIP;
	long ret = 0;

	pVoIP = (bcm_caph_hwdep_voip_t *) hw->private_data;
	/* BCM_AUDIO_DEBUG("voip_read count %ld\n",count); */

	if ((pVoIP->status == VoIP_Hwdep_Status_Started)
	    && (pVoIP->frames_available_to_read > 0)) {
		if (pVoIP->frames_available_to_read) {
			if (pVoIP->buffer_handle->voip_data_ul_buf_ptr) {
				ret =
				    copy_to_user(buf,
						 pVoIP->buffer_handle->
						 voip_data_ul_buf_ptr +
						 pVoIP->buffer_handle->
						 voip_data_ul_rd_index,
						 pVoIP->frame_size);
				pVoIP->frames_available_to_read--;
#ifdef CONFIG_VOIP_BUFFER_INCREASE
				pVoIP->buffer_handle->voip_data_ul_rd_index +=
				    pVoIP->frame_size;
				if (pVoIP->buffer_handle->
				    voip_data_ul_rd_index >=
				    pVoIP->buffer_size) {
					pVoIP->buffer_handle->
					    voip_data_ul_rd_index -=
					    pVoIP->buffer_size;
				}
#endif
				ret = pVoIP->frame_size;
			}
		}
	} else
		ret = 0;

	return ret;
}

static long hwdep_write(struct snd_hwdep *hw, const char __user * buf,
			long count, loff_t *offset)
{
	bcm_caph_hwdep_voip_t *pVoIP;
	long ret;
	pVoIP = (bcm_caph_hwdep_voip_t *) hw->private_data;

	/* BCM_AUDIO_DEBUG("voip_write pVoIP->frame_size %d,pVoIP->"
	 * "writecount %d\n",pVoIP->frame_size,pVoIP->writecount);
	 */

	if ((pVoIP->status == VoIP_Hwdep_Status_Started)
	    && (pVoIP->buffer_handle)) {
		if (pVoIP->buffer_handle->voip_data_dl_buf_ptr) {
			ret =
			    copy_from_user(pVoIP->buffer_handle->
					   voip_data_dl_buf_ptr +
					   pVoIP->buffer_handle->
					   voip_data_dl_wr_index, buf,
					   pVoIP->frame_size);
			pVoIP->frames_available_to_write++;
			pVoIP->writecount--;
#ifdef CONFIG_VOIP_BUFFER_INCREASE
			pVoIP->buffer_handle->voip_data_dl_wr_index +=
			    pVoIP->frame_size;
			if (pVoIP->buffer_handle->voip_data_dl_wr_index >=
			    pVoIP->buffer_size) {
				pVoIP->buffer_handle->voip_data_dl_wr_index -=
				    pVoIP->buffer_size;
			}
#endif
		}
		ret = pVoIP->frame_size;
	} else
		ret = 0;
	return ret;
}

static int hwdep_open(struct snd_hwdep *hw, struct file *file)
{
	BCM_AUDIO_DEBUG("VoIP_Ioctl_Open\n");

	/*set the default parameters only once */
	if (!setdefault) {
		setdefault = TRUE;
		voip_data.mic = AUDIO_SOURCE_ANALOG_MAIN;
		voip_data.spk = AUDIO_SINK_HANDSET;
		voip_data.codec_type = 0;	/* PCM 8K */
	}
	return 0;
}

static int hwdep_release(struct snd_hwdep *hw, struct file *file)
{
	bcm_caph_hwdep_voip_t *pVoIP;
	pVoIP = (bcm_caph_hwdep_voip_t *) hw->private_data;

	BCM_AUDIO_DEBUG("VoIP_Ioctl_Release\n");
	return 0;
}

static int hwdep_ioctl(struct snd_hwdep *hw, struct file *file,
		       unsigned int cmd, unsigned long arg)
{
	bcm_caph_hwdep_voip_t *pVoIP;
	AUDIO_DRIVER_CallBackParams_t cbParams;
	int ret = 0;
	Boolean enable = FALSE;
	Int32 size = 0;
	int data;
	static UserCtrl_data_t *dataptr;

	pVoIP = (bcm_caph_hwdep_voip_t *) hw->private_data;

	BCM_AUDIO_DEBUG("hwdep_ioctl cmd=%d\n", cmd);

	switch (cmd) {
	case VoIP_Ioctl_GetVersion:
		/* ret = put_user(BrcmAACEncVersion, (int __user *)arg); */
		break;
	case VoIP_Ioctl_Start:
		BCM_AUDIO_DEBUG("VoIP_Ioctl_Start\n");
		if (voipInstCnt == 0) {	/* start VoIP only once */
			voipInstCnt++;
			hw->private_data =
			    kzalloc(sizeof(bcm_caph_hwdep_voip_t), GFP_KERNEL);
			CAPH_ASSERT(hw->private_data);
			pVoIP = (bcm_caph_hwdep_voip_t *) hw->private_data;
			init_waitqueue_head(&pVoIP->sleep);

			pVoIP->mic = voip_data.mic;
			pVoIP->spk = voip_data.spk;
			pVoIP->codec_type = voip_data.codec_type;

			pVoIP->buffer_handle =
			    (audio_voip_driver_t *)
			    kzalloc(sizeof(audio_voip_driver_t), GFP_KERNEL);

			if (pVoIP->buffer_handle)
				memset((u8 *) pVoIP->buffer_handle, 0,
				       sizeof(audio_voip_driver_t));
			else
				return -ENOMEM;

			pVoIP->frame_size = sVoIPFrameLen[pVoIP->codec_type];

			pVoIP->buffer_size =
			    pVoIP->frame_size * VOIP_FRAMES_IN_BUFFER;

			pVoIP->buffer_handle->voip_data_dl_buf_ptr =
				kzalloc(pVoIP->buffer_size, GFP_KERNEL);
			if (pVoIP->buffer_handle->voip_data_dl_buf_ptr) {
				memset(pVoIP->buffer_handle->
				       voip_data_dl_buf_ptr, 0,
				       pVoIP->buffer_size);
			} else {
				kfree(pVoIP->buffer_handle);
				return -ENOMEM;
			}

			pVoIP->buffer_handle->voip_data_ul_buf_ptr =
				kzalloc(pVoIP->buffer_size, GFP_KERNEL);
			if (pVoIP->buffer_handle->voip_data_ul_buf_ptr)
				memset(pVoIP->buffer_handle->
				       voip_data_ul_buf_ptr, 0,
				       pVoIP->buffer_size);
			else {
				kfree(pVoIP->buffer_handle->
				      voip_data_ul_buf_ptr);
				kfree(pVoIP->buffer_handle);
				return -ENOMEM;
			}

			pVoIP->buffer_handle->drv_handle =
			    AUDIO_DRIVER_Open(AUDIO_DRIVER_VOIP);
			if (!pVoIP->buffer_handle->drv_handle) {
				kfree(pVoIP->buffer_handle->
				      voip_data_dl_buf_ptr);
				kfree(pVoIP->buffer_handle->
				      voip_data_ul_buf_ptr);
				kfree(pVoIP->buffer_handle);
				return -ENOMEM;
			}

			/* set UL callback */

			cbParams.voipULCallback = HWDEP_VOIP_DumpUL_CB;
			cbParams.pPrivateData = (void *)pVoIP;
			AUDIO_DRIVER_Ctrl(pVoIP->buffer_handle->drv_handle,
					  AUDIO_DRIVER_SET_VOIP_UL_CB,
					  (void *)&cbParams);

			/* set DL callback */
			cbParams.voipDLCallback = HWDEP_VOIP_FillDL_CB;
			cbParams.pPrivateData = (void *)pVoIP;
			AUDIO_DRIVER_Ctrl(pVoIP->buffer_handle->drv_handle,
					  AUDIO_DRIVER_SET_VOIP_DL_CB,
					  (void *)&cbParams);

			if ((pVoIP->codec_type == 4) ||
			    (pVoIP->codec_type == 5)) {
				/* VOIP_PCM_16K or VOIP_AMR_WB_MODE_7k */
				AUDCTRL_Telephony_RateChange(16000);
			}

			AUDCTRL_EnableTelephony(pVoIP->mic, pVoIP->spk);
			AUDCTRL_SetTelephonySpkrVolume(pVoIP->spk, 0,
						       AUDIO_GAIN_FORMAT_mB);
			AUDIO_DRIVER_Ctrl(pVoIP->buffer_handle->drv_handle,
					  AUDIO_DRIVER_START, &voip_data);

			pVoIP->writecount = 1;
			pVoIP->status = VoIP_Hwdep_Status_Started;
		} else {
			voipInstCnt++;
			BCM_AUDIO_DEBUG("VoIP_Ioctl_Start -> just increment "
				"the count, voip already started\n");
		}

		break;
	case VoIP_Ioctl_Stop:
		BCM_AUDIO_DEBUG("VoIP_Ioctl_Stop\n");

		if (voipInstCnt == 2)
			voipInstCnt--;
		else if (voipInstCnt == 1) {
			AUDIO_DRIVER_Ctrl(pVoIP->buffer_handle->drv_handle,
					  AUDIO_DRIVER_STOP, NULL);
			AUDCTRL_DisableTelephony();

			AUDIO_DRIVER_Close(pVoIP->buffer_handle->drv_handle);
			kfree(pVoIP->buffer_handle->voip_data_dl_buf_ptr);
			kfree(pVoIP->buffer_handle->voip_data_ul_buf_ptr);
			kfree(pVoIP->buffer_handle);
			pVoIP->status = VoIP_Hwdep_Status_Stopped;
			wake_up(&pVoIP->sleep);
			voipInstCnt = 0;
		}

		break;
	case VoIP_Ioctl_SetSource:
		get_user(data, __user(int *)arg);
		voip_data.mic = (AUDIO_SOURCE_Enum_t) data;
		BCM_AUDIO_DEBUG(" VoIP_Ioctl_SetSource mic %ld,\n",
				voip_data.mic);
		break;

	case VoIP_Ioctl_SetSink:
		get_user(data, __user(int *)arg);
		voip_data.spk = (AUDIO_SINK_Enum_t) data;
		BCM_AUDIO_DEBUG(" VoIP_Ioctl_SetSink spk %ld,\n",
				voip_data.spk);
		break;

	case VoIP_Ioctl_SetCodecType:
		get_user(data, __user(int *)arg);
		voip_data.codec_type = (u32) data;
		BCM_AUDIO_DEBUG(" VoIP_Ioctl_SetCodecType codec_type %ld,\n",
				voip_data.codec_type);
		break;
	case VoIP_Ioctl_SetBitrate:
		get_user(data, __user(int *)arg);
		voip_data.bitrate_index = (u32) data;
		BCM_AUDIO_DEBUG(" VoIP_Ioctl_SetBitrate bitrate_index %ld,\n",
				voip_data.bitrate_index);
		break;
	case VoIP_Ioctl_GetSource:
		data = (int)voip_data.mic;
		put_user(data, __user(int *)arg);
		break;
	case VoIP_Ioctl_GetSink:
		data = (int)voip_data.spk;
		put_user(data, __user(int *)arg);
		break;
	case VoIP_Ioctl_GetCodecType:
		data = (int)voip_data.codec_type;
		put_user(data, __user(int *)arg);
		break;
	case VoIP_Ioctl_GetBitrate:
		data = (int)voip_data.bitrate_index;
		put_user(data, __user(int *)arg);
		break;
	case VoIP_Ioctl_GetMode:
		{
			AudioMode_t mode = AUDCTRL_GetAudioMode();
			put_user((int)mode, __user(int *)arg);
		}
		break;
	case VoIP_Ioctl_SetMode:
		{
			int mode;
			AUDIO_SOURCE_Enum_t new_mic;
			AUDIO_SINK_Enum_t new_spk;
			get_user(mode, __user(int *)arg);
			AUDCTRL_GetSrcSinkByMode((AudioMode_t) (mode), &new_mic,
						 &new_spk);

			voip_data.mic = new_mic;
			voip_data.spk = new_spk;

			if ((pVoIP->codec_type == 4) ||
			    (pVoIP->codec_type == 5)) {
				/* VOIP_PCM_16K or VOIP_AMR_WB_MODE_7k */
				AUDCTRL_Telephony_RateChange(16000);
			}
#if !defined(USE_NEW_AUDIO_PARAM)
			AUDCTRL_SetAudioMode(mode);
#else
			AUDCTRL_SetAudioMode(mode, AUDCTRL_GetAudioApp());
#endif
			BCM_AUDIO_DEBUG(" VoIP_Ioctl_SetMode mode %d,\n",
					mode);
		}
		break;
	case VoIP_Ioctl_SetVoLTEFlag:
		get_user(data, __user(int *)arg);
		voip_data.isVoLTE = (u8) data;
		BCM_AUDIO_DEBUG(" VoIP_Ioctl_SetFlag isVoLTE %d,\n",
				voip_data.isVoLTE);
		break;
	case VoIP_Ioctl_GetVoLTEFlag:
		data = (int)voip_data.isVoLTE;
		put_user(data, __user(int *)arg);
		break;
	case DSPCtrl_Ioctl_SPCtrl:
		if (dataptr == NULL)
			dataptr = kzalloc(sizeof(UserCtrl_data_t), GFP_KERNEL);
		else
			memset(dataptr, 0, sizeof(UserCtrl_data_t));

		if (!dataptr)
			return -ENOMEM;

		ret =
		    copy_from_user(dataptr, (int __user *)arg,
				   sizeof(UserCtrl_data_t));

		enable = (Boolean) dataptr->data[0];
		size = dataptr->data[1];
		ret =
		    AUDDRV_User_CtrlDSP(AUDDRV_USER_SP_CTRL, enable, size,
					(void *)&(dataptr->data[2]));
		if (!enable) {
			kfree(dataptr);
			dataptr = NULL;
		}
		break;
	case DSPCtrl_Ioctl_SPSetVar:
		/*
		 * Will move this part later after we separate the voip and
		 * dspctrl connections.
		 */
		if (dataptr == NULL)
			dataptr = kzalloc(sizeof(UserCtrl_data_t), GFP_KERNEL);
		else
			memset(dataptr, 0, sizeof(UserCtrl_data_t));

		if (!dataptr)
			return -ENOMEM;

		ret =
		    copy_from_user(dataptr, (int __user *)arg,
				   sizeof(UserCtrl_data_t));

		size = dataptr->data[0];
		ret =
		    AUDDRV_User_CtrlDSP(AUDDRV_USER_SP_VAR, enable, size,
					(void *)&(dataptr->data[2]));
		break;
	case DSPCtrl_Ioctl_SPQuery:
		if (dataptr == NULL)
			dataptr = kzalloc(sizeof(UserCtrl_data_t), GFP_KERNEL);
		else
			memset(dataptr, 0, sizeof(UserCtrl_data_t));

		if (!dataptr)
			return -ENOMEM;
		ret =
		    AUDDRV_User_CtrlDSP(AUDDRV_USER_SP_QUERY, enable, size,
					(void *)dataptr);
		if (ret < 0)
			return ret;

		if (copy_to_user
		    ((int __user *)arg, dataptr, sizeof(UserCtrl_data_t)))
			return -EFAULT;
		break;
	case DSPCtrl_Ioctl_EQCtrl:
		if (dataptr == NULL)
			dataptr = kzalloc(sizeof(UserCtrl_data_t), GFP_KERNEL);
		else
			memset(dataptr, 0, sizeof(UserCtrl_data_t));

		if (!dataptr)
			return -ENOMEM;

		if (copy_from_user
		    (dataptr, (int __user *)arg, sizeof(UserCtrl_data_t)))
			return -EFAULT;

		enable = (Boolean) dataptr->data[0];
		ret =
		    AUDDRV_User_CtrlDSP(AUDDRV_USER_EQ_CTRL, enable, size,
					(void *)&(dataptr->data[2]));
		if (!enable) {
			kfree(dataptr);
			dataptr = NULL;
		}
		break;
	default:
		ret = -ENOTTY;
		break;

	}
	return ret;
}

static int hwdep_mmap(struct snd_hwdep *hw, struct file *file,
		      struct vm_area_struct *vma)
{
	return 0;
}

static int hwdep_dsp_status(struct snd_hwdep *hw,
			    struct snd_hwdep_dsp_status *status)
{
	return 0;
}

static void hwdep_private_free(struct snd_hwdep *hwdep)
{
	kfree(hwdep->private_data);
}

static unsigned int hwdep_poll(struct snd_hwdep *hw, struct file *file,
			       poll_table *wait)
{
	unsigned int mask = 0;
	bcm_caph_hwdep_voip_t *pVoIP;

	pVoIP = (bcm_caph_hwdep_voip_t *) hw->private_data;
	poll_wait(file, &pVoIP->sleep, wait);
	if (pVoIP->status == VoIP_Hwdep_Status_Started) {

		if (pVoIP->frames_available_to_read > 0)
			mask = POLLIN | POLLRDNORM;
		if (pVoIP->writecount > 0)	/* buffer available to write */
			mask |= POLLOUT | POLLWRNORM;
	}

	/* BCM_AUDIO_DEBUG("voip_poll mask %ld\n",mask); */

	return mask;
}

/**
 * HwdepDeviceNew: Create HWDEP device
 *
 * Return 0 for success
 */
int __devinit HwdepDeviceNew(struct snd_card *card)
{
	int err = 0;
	struct snd_hwdep *pHwdep;

	err = snd_hwdep_new(card, "Broadcom CAPH VOIP", 0, &pHwdep);
	if (err < 0) {
		BCM_AUDIO_DEBUG("error create hwdep device\n");
		return err;
	}
	pHwdep->iface = SNDRV_HWDEP_IFACE_OPL4;
	pHwdep->ops.read = hwdep_read;
	pHwdep->ops.write = hwdep_write;
	pHwdep->ops.open = hwdep_open;
	pHwdep->ops.release = hwdep_release;
	pHwdep->ops.ioctl = hwdep_ioctl;
	pHwdep->ops.mmap = hwdep_mmap;
	pHwdep->ops.dsp_status = hwdep_dsp_status;
	pHwdep->ops.poll = hwdep_poll;
	pHwdep->private_free = hwdep_private_free;

	return err;
}
