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
#include "audio_trace.h"

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
#include "auddrv_audlog.h"

/* Local defines */

#define VOIP_FRAMES_IN_BUFFER 1

enum __VoIP_Hwdep_Status_t {
	VoIP_Hwdep_Status_None,
	VoIP_Hwdep_Status_Opened,
	VoIP_Hwdep_Status_Started,
	VoIP_Hwdep_Status_Stopped,
	VoIP_Hwdep_Status_Released,
};

struct __dl_frame {
	u32 timestamp;
	char data[VOIP_MAX_FRAME_LEN];
};
struct __audio_voip_driver {
	u32 voip_data_dl_rd_index;
	u32 voip_data_dl_wr_index;
	u32 voip_data_ul_rd_index;
	u32 voip_data_ul_wr_index;
	u8 *voip_data_ul_buf_ptr;
	u8 *voip_data_dl_buf_ptr;
	u32 dl_timestamp;
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
	int dlstarted;
	int ulstarted;
};
#define	bcm_caph_hwdep_voip_t struct __bcm_caph_hwdep_voip

static const u16 sVoIPFrameLen[] = { 320, 158, 36, 164, 640, 68 };

static u16 sVoIPAMRSilenceFrame[1] = { 0x000f };

static u32 voipInstCnt;

static Boolean isVoLTE = FALSE;

bcm_caph_voip_log_t voip_log;

/* local functions */
static void HWDEP_VOIP_DumpUL_CB(void *pPrivate, u8 * pSrc, u32 nSize);
static void HWDEP_VOIP_FillDL_CB(
		void *pPrivate,
		u8 *pDst,
		u32 nSize,
		u32 *timestamp);
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

static void voip_log_capture(CAPTURE_POINT_t log_pint, unsigned char *buf,
					int size);

uint32_t guULCount = 0, guDLCount = 0;
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

	/* DEBUG("HWDEP_VOIP_DumpUL_CB nSize %d pVoIP 0x%x\n",
	 * nSize,pVoIP);
	 */
	guULCount++;
	if (0 == (guULCount%500))
		aTrace(LOG_ALSA_INTERFACE, "DumpUL_CB ulstarted=%d dlstarted="
		"%d, voipInstCnt=%d frames_available_to_read=%d, "
		"frames_available_to_write=%d",
		pVoIP->ulstarted, pVoIP->dlstarted, voipInstCnt,
		pVoIP->frames_available_to_read,
		pVoIP->frames_available_to_write);

	if (pVoIP->ulstarted == 0)
		return;

	if (pVoIP->buffer_handle) {
		if (pVoIP->buffer_handle->voip_data_ul_buf_ptr) {
			memcpy(pVoIP->buffer_handle->voip_data_ul_buf_ptr +
			       pVoIP->buffer_handle->voip_data_ul_wr_index,
			       pSrc, nSize);
			pVoIP->frames_available_to_read++;
			if (pVoIP->frames_available_to_read > 1)
				aTrace(LOG_ALSA_INTERFACE, "more than 1 frame"
				" available frame_size %d, readcount %d\n",
				pVoIP->frame_size,
				pVoIP->frames_available_to_read);
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

static void HWDEP_VOIP_FillDL_CB(
		void *pPrivate,
		u8 *pDst,
		u32 nSize,
		u32 *timestamp)
{
	bcm_caph_hwdep_voip_t *pVoIP;
	pVoIP = (bcm_caph_hwdep_voip_t *) pPrivate;

	guDLCount++;
	if (0 == (guDLCount%500))
		aTrace(LOG_ALSA_INTERFACE, "FillDL_CB ulstarted=%d dlstarted="
		"%d, voipInstCnt=%d frames_available_to_read=%d, "
		"frames_available_to_write=%d",
		pVoIP->ulstarted, pVoIP->dlstarted, voipInstCnt,
		pVoIP->frames_available_to_read,
		pVoIP->frames_available_to_write);

	if (pVoIP->dlstarted == 0) {
		FillSilenceFrame(pVoIP->codec_type, nSize, pDst);
		return;
	}

	if (pVoIP->buffer_handle->voip_data_dl_buf_ptr) {
		/*DEBUG("HWDEP_VOIP_FillDL_CB pVoIP->"
		 *"frames_available_to_write %d\n", pVoIP->
		 * frames_available_to_write);
		 */
		if (pVoIP->frames_available_to_write == 0) {
			/* fill with silent data based on the frame type  */
			FillSilenceFrame(pVoIP->codec_type, nSize, pDst);
			aTrace(LOG_ALSA_INTERFACE, "under run frame_size %d,"
				"writecount %d\n",
				pVoIP->frame_size, pVoIP->writecount);

		} else {
		if (isVoLTE)
			*timestamp = pVoIP->buffer_handle->dl_timestamp;
		else
			*timestamp = 0;
			memcpy(pDst,
			       pVoIP->buffer_handle->voip_data_dl_buf_ptr +
			       pVoIP->buffer_handle->voip_data_dl_rd_index,
			       nSize);
			pVoIP->frames_available_to_write--;
			if (!isVoLTE)
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
	if (!pVoIP)
		return 0;
	/* DEBUG("voip_read count %ld\n",count); */

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

				voip_log_capture(AUD_LOG_VOCODER_UL,
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
	struct __dl_frame *frame_dl_data = (struct __dl_frame *)buf;
	pVoIP = (bcm_caph_hwdep_voip_t *) hw->private_data;
	if (!pVoIP)
		return count;

	/* DEBUG("voip_write pVoIP->frame_size %d,pVoIP->"
	 * "writecount %d\n",pVoIP->frame_size,pVoIP->writecount);
	 */

	if ((pVoIP->status == VoIP_Hwdep_Status_Started)
	    && (pVoIP->buffer_handle)) {
		if (pVoIP->buffer_handle->voip_data_dl_buf_ptr) {
			if (isVoLTE) {
				pVoIP->buffer_handle->dl_timestamp =
						frame_dl_data->timestamp;

				ret =
			    copy_from_user(
					pVoIP->buffer_handle->
					voip_data_dl_buf_ptr +
					pVoIP->buffer_handle->
					voip_data_dl_wr_index,
					(struct __dl_frame *)
					frame_dl_data->data,
					pVoIP->frame_size);

				voip_log_capture(AUD_LOG_VOCODER_DL,
					pVoIP->buffer_handle->
					voip_data_dl_buf_ptr +
					pVoIP->buffer_handle->
					voip_data_dl_wr_index,
					pVoIP->frame_size);

				/* send the DL frame to DSP . In case of VoLTE,
				whenever the application sends the data,
				need to send it to DSP.No need to wait
				for DSP callback. */

				VOLTE_ProcessDLData();
			} else {
				ret =
					copy_from_user(pVoIP->buffer_handle->
					voip_data_dl_buf_ptr +
					pVoIP->buffer_handle->
					voip_data_dl_wr_index,
					buf,
					pVoIP->frame_size);

				voip_log_capture(AUD_LOG_VOCODER_DL,
					pVoIP->buffer_handle->
					voip_data_dl_buf_ptr +
					pVoIP->buffer_handle->
					voip_data_dl_wr_index,
					pVoIP->frame_size);

			}
			pVoIP->frames_available_to_write++;
			if (!isVoLTE)
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
	aTrace(LOG_ALSA_INTERFACE , "ALSA-CAPH VoIP_Ioctl_Open\n");

	return 0;
}

static int hwdep_release(struct snd_hwdep *hw, struct file *file)
{
	aTrace(LOG_ALSA_INTERFACE , "ALSA-CAPH VoIP_Ioctl_Release\n");
	return 0;
}

static int hwdep_ioctl(struct snd_hwdep *hw, struct file *file,
		       unsigned int cmd, unsigned long arg)
{
	bcm_caph_hwdep_voip_t *pVoIP;
	int ret = 0;
	Boolean enable = FALSE;
	Int32 size = 0;
	int data;
	static UserCtrl_data_t *dataptr;
	brcm_alsa_chip_t *pChip = NULL;
	struct treq_sysparm_t *eq;

	pChip = (brcm_alsa_chip_t *)hw->card->private_data;

	pVoIP = (bcm_caph_hwdep_voip_t *) hw->private_data;

	aTrace(LOG_ALSA_INTERFACE, "ALSA-CAPH hwdep_ioctl cmd=%08X\n", cmd);

	switch (cmd) {
	case VoIP_Ioctl_GetVersion:
		/* ret = put_user(BrcmAACEncVersion, (int __user *)arg); */
		break;
	case VoIP_Ioctl_Start:
		get_user(data, (int __user *)arg);
		aTrace(LOG_ALSA_INTERFACE, "VoIP_Ioctl_Start type=%d (2==UL)"
			"voipInstCnt=%u\n", data, voipInstCnt);
		if (voipInstCnt == 0) {	/* start VoIP only once */
			BRCM_AUDIO_Param_RateChange_t param_rate_change;
			BRCM_AUDIO_Param_Open_t param_open;
			BRCM_AUDIO_Param_Prepare_t parm_prepare;
			BRCM_AUDIO_Param_Start_t param_start;

			voipInstCnt++;

			hw->private_data =
			    kzalloc(sizeof(bcm_caph_hwdep_voip_t), GFP_KERNEL);
			CAPH_ASSERT(hw->private_data);
			pVoIP = (bcm_caph_hwdep_voip_t *) hw->private_data;

			init_waitqueue_head(&pVoIP->sleep);

			pVoIP->codec_type = pChip->voip_data.codec_type;

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
				pVoIP->buffer_handle = NULL;
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
				pVoIP->buffer_handle->
				      voip_data_ul_buf_ptr = NULL;
				kfree(pVoIP->buffer_handle);
				pVoIP->buffer_handle = NULL;
				return -ENOMEM;
			}

			param_open.drv_handle = NULL;
			param_open.drv_type = AUDIO_DRIVER_VOIP;
			AUDIO_Ctrl_Trigger(ACTION_AUD_OpenVoIP,
				&param_open, NULL, 1);
			pVoIP->buffer_handle->drv_handle =
				param_open.drv_handle;

			if (!pVoIP->buffer_handle->drv_handle) {
				kfree(pVoIP->buffer_handle->
				      voip_data_dl_buf_ptr);
				pVoIP->buffer_handle->
				      voip_data_dl_buf_ptr = NULL;
				kfree(pVoIP->buffer_handle->
				      voip_data_ul_buf_ptr);
				pVoIP->buffer_handle->
				      voip_data_ul_buf_ptr = NULL;
				kfree(pVoIP->buffer_handle);
				pVoIP->buffer_handle = NULL;
				return -ENOMEM;
			}

			/* set UL callback */

			parm_prepare.drv_handle =
				pVoIP->buffer_handle->drv_handle;
			parm_prepare.cbParams.voipULCallback =
				HWDEP_VOIP_DumpUL_CB;
			parm_prepare.cbParams.pPrivateData = (void *)pVoIP;
			AUDIO_Ctrl_Trigger(
				ACTION_AUD_SET_VOIP_UL_CB,
				&parm_prepare, NULL, 0);

			/* set DL callback */
			parm_prepare.drv_handle =
				pVoIP->buffer_handle->drv_handle;
			parm_prepare.cbParams.voipDLCallback =
				HWDEP_VOIP_FillDL_CB;
			parm_prepare.cbParams.pPrivateData = (void *)pVoIP;
			AUDIO_Ctrl_Trigger(
				ACTION_AUD_SET_VOIP_DL_CB,
				&parm_prepare, NULL, 0);
			/* VoIP is always 16K.
			No need to set the codec type here*/
			if (isVoLTE) {
				if ((pVoIP->codec_type == 4)
				   || (pVoIP->codec_type == 5)) {
					/* VOIP_PCM_16K or
					VOIP_AMR_WB_MODE_7k */
				param_rate_change.codecID = 0x0A;
			} else
				param_rate_change.codecID = 0x06;
			AUDIO_Ctrl_Trigger(ACTION_AUD_RateChange,
					&param_rate_change, NULL, 0);
			}
			param_start.drv_handle =
				pVoIP->buffer_handle->drv_handle;
			param_start.data = (void *)&pChip->voip_data;
			AUDIO_Ctrl_Trigger(ACTION_AUD_StartVoIP,
				&param_start, NULL, 0);

			pVoIP->writecount = VOIP_FRAMES_IN_BUFFER;
			pVoIP->status = VoIP_Hwdep_Status_Started;
		} else {
			voipInstCnt++;
			aTrace(LOG_ALSA_INTERFACE,
					"VoIP_Ioctl_Start -> just increment "
					"the count, voip already started\n");
		}
		if (pVoIP != NULL) {
		if (data == VoIP_UL)
			pVoIP->ulstarted = 1;
		else
			pVoIP->dlstarted = 1;
		}

		break;
	case VoIP_Ioctl_Stop:
		get_user(data, (int __user *)arg);
		aTrace(LOG_ALSA_INTERFACE, "VoIP_Ioctl_Stop type=%d (2==UL) "
			"voipInstCnt=%u\n", data, voipInstCnt);

		if (voipInstCnt == 2)
			voipInstCnt--;
		else if (voipInstCnt == 1) {
			BRCM_AUDIO_Param_Stop_t param_stop;
			BRCM_AUDIO_Param_Close_t param_close;

			param_stop.drv_handle =
				pVoIP->buffer_handle->drv_handle;
			AUDIO_Ctrl_Trigger(ACTION_AUD_StopVoIP,
				&param_stop, NULL, 0);

			param_close.drv_handle =
				pVoIP->buffer_handle->drv_handle;
			AUDIO_Ctrl_Trigger(ACTION_AUD_CloseVoIP,
				&param_close, NULL, 1);
			kfree(pVoIP->buffer_handle->voip_data_dl_buf_ptr);
			pVoIP->buffer_handle->voip_data_dl_buf_ptr = NULL;
			kfree(pVoIP->buffer_handle->voip_data_ul_buf_ptr);
			pVoIP->buffer_handle->voip_data_ul_buf_ptr = NULL;
			kfree(pVoIP->buffer_handle);
			pVoIP->buffer_handle = NULL;
			pVoIP->status = VoIP_Hwdep_Status_Stopped;
			wake_up(&pVoIP->sleep);
			voipInstCnt = 0;
		}

		if (data == VoIP_UL)
			pVoIP->ulstarted = 0;
		else
			pVoIP->dlstarted = 0;

		break;
	case VoIP_Ioctl_SetSource:
		aTrace(LOG_ALSA_INTERFACE ,
				" Warning: VoIP_Ioctl_SetSource"
				"is depreciated , please"
				"use mixer control VC-SEL instead\n");
		break;

	case VoIP_Ioctl_SetSink:
		aTrace(LOG_ALSA_INTERFACE ,
				" Warning: VoIP_Ioctl_SetSink"
				"is depreciated, please"
				"use mixer control VC-SEL instead\n");
		break;

	case VoIP_Ioctl_SetCodecType:
		get_user(data, (int __user *)arg);
		pChip->voip_data.codec_type = (u32) data;
		aTrace(LOG_ALSA_INTERFACE,
				" VoIP_Ioctl_SetCodecType codec_type %ld,\n",
				pChip->voip_data.codec_type);

		pVoIP = (bcm_caph_hwdep_voip_t *) hw->private_data;
		if (!pVoIP)
			break;

		pVoIP->codec_type = pChip->voip_data.codec_type;

		/*Check whether in a VoLTE call*/
		/*If no, do nothing.*/
		/*If yes, do the NB<->WB switching*/
		if (isVoLTE) {
			BRCM_AUDIO_Param_RateChange_t param_rate_change;
			if (pVoIP->ulstarted == 0 && pVoIP->dlstarted == 0)
				break;

			if (!(pVoIP->buffer_handle))
				break;

			if (!(pVoIP->buffer_handle->drv_handle))
				break;

			AUDIO_DRIVER_Ctrl(pVoIP->buffer_handle->drv_handle,
				AUDIO_DRIVER_SET_AMR, &pChip->voip_data);

			if ((pVoIP->codec_type == 4) ||
			    (pVoIP->codec_type == 5))
				/* VOIP_PCM_16K or VOIP_AMR_WB_MODE_7k */
				param_rate_change.codecID = 0x0A;
			else
				param_rate_change.codecID = 0x06;
			AUDIO_Ctrl_Trigger(ACTION_AUD_RateChange,
					&param_rate_change, NULL, 0);
		}
		break;
	case VoIP_Ioctl_SetBitrate:
		get_user(data, (int __user *)arg);
		pChip->voip_data.bitrate_index = (u32) data;
		aTrace(LOG_ALSA_INTERFACE,
				" VoIP_Ioctl_SetBitrate bitrate_index %ld,\n",
				pChip->voip_data.bitrate_index);
		pVoIP = (bcm_caph_hwdep_voip_t *) hw->private_data;
		if (!pVoIP)
			break;

		if (isVoLTE) {
			if (pVoIP->ulstarted == 0 && pVoIP->dlstarted == 0)
				break;

			if (!(pVoIP->buffer_handle))
				break;

			if (!(pVoIP->buffer_handle->drv_handle))
				break;


			AUDIO_DRIVER_Ctrl(pVoIP->buffer_handle->drv_handle,
				AUDIO_DRIVER_SET_AMR, &pChip->voip_data);
		}


		break;
	case VoIP_Ioctl_GetSource:
		{
		s32 *pSel;
		pSel = pChip->streamCtl[CTL_STREAM_PANEL_VOICECALL - 1]
			.iLineSelect;
		data = (int)pSel[0];
		put_user(data, (int __user *)arg);
		}
		break;
	case VoIP_Ioctl_GetSink:
		{
		s32 *pSel;
		pSel = pChip->streamCtl[CTL_STREAM_PANEL_VOICECALL - 1]
			.iLineSelect;
		data = (int)pSel[1];
		put_user(data, (int __user *)arg);
		}
		break;
	case VoIP_Ioctl_GetCodecType:
		data = (int)pChip->voip_data.codec_type;
		put_user(data, (int __user *)arg);
		break;
	case VoIP_Ioctl_GetBitrate:
		data = (int)pChip->voip_data.bitrate_index;
		put_user(data, (int __user *)arg);
		break;
	case VoIP_Ioctl_GetMode:
		{
			AudioMode_t mode = AUDCTRL_GetAudioMode();
			put_user((int)mode, (int __user *)arg);
			aTrace(LOG_ALSA_INTERFACE,
					" VoIP_Ioctl_GetMode mode %d,\n",
					mode);
		}
		break;
	case VoIP_Ioctl_SetMode:
		aTrace(LOG_ALSA_INTERFACE ,
				" Warning: VoIP_Ioctl_SetMode"
				"is depreciated, please "
				"use mixer control VC-SEL instead\n");
		break;
	case VoIP_Ioctl_SetVoLTEDTX:
		get_user(data, (int __user *)arg);
		pChip->voip_data.isDTXEnabled = (u8) data;
		aTrace(LOG_ALSA_INTERFACE, " VoIP_Ioctl_SetVoLTEDTX %d,\n",
				pChip->voip_data.isDTXEnabled);

		pVoIP = (bcm_caph_hwdep_voip_t *) hw->private_data;
		if (!pVoIP)
			break;
		if (!(pVoIP->buffer_handle))
			break;
		if (!(pVoIP->buffer_handle->drv_handle))
			break;

		AUDIO_DRIVER_Ctrl(pVoIP->buffer_handle->drv_handle,
				AUDIO_DRIVER_SET_DTX, &pChip->voip_data);

		break;
	
	case VoIP_Ioctl_SetVoLTEFlag:
		get_user(data, (int __user *)arg);
		pChip->voip_data.isVoLTE = (u8) data;
		aTrace(LOG_ALSA_INTERFACE, " VoIP_Ioctl_SetFlag isVoLTE %d,\n",
				pChip->voip_data.isVoLTE);
		isVoLTE = pChip->voip_data.isVoLTE;
		break;
	case VoIP_Ioctl_GetVoLTEFlag:
		data = (int)pChip->voip_data.isVoLTE;
		put_user(data, (int __user *)arg);
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
	case Ctrl_Ioctl_SWEQParm:
		aTrace(LOG_ALSA_INTERFACE,
			"ALSA-CAPH hwdep_ioctl Ctrl_Ioctl_SWEQParm");
		eq = kzalloc(sizeof(*eq), GFP_KERNEL);
		if (eq == NULL) {
			aError("treq_sysparm_t mem alloc failed");
			return -ENOMEM;
		}

		/* get the sysparm from driver
		 SW EQ is only for music playback for now*/
		if (copy_from_user(eq, (int __user *)arg,
			sizeof(struct treq_sysparm_t))) {
			if (eq != NULL) {
				kfree(eq);
				eq = NULL;
			}
			return -EFAULT;
		}

		ret = AUDDRV_Get_TrEqParm((void *)eq,
			sizeof(*eq), AUDIO_APP_MUSIC,
			(eq->data)[TREQ_DATA_SIZE-1]);

		if (!ret) {
			if (copy_to_user((void __user *)arg, eq,
			sizeof(*eq))) {
				if (eq != NULL) {
					kfree(eq);
					eq = NULL;
				}
				return -EFAULT;
			}
		}

		if (eq != NULL) {
			kfree(eq);
			eq = NULL;
		}
		break;

	case Ctrl_Ioctl_FDMBCParm:
		aTrace(LOG_ALSA_INTERFACE,
			"ALSA-CAPH hwdep_ioctl Ctrl_Ioctl_FDMBCParm");
		if (dataptr == NULL)
			dataptr = kzalloc(sizeof(UserCtrl_data_t), GFP_KERNEL);
		else
			memset(dataptr, 0, sizeof(UserCtrl_data_t));

		if (dataptr == NULL) {
			aError("UserCtrl_data_t mem alloc failed");
			return -ENOMEM;
		}

		/* get the sysparm from driver */
		if (copy_from_user
		    (dataptr, (int __user *)arg, sizeof(UserCtrl_data_t)))
			return -EFAULT;

		size = dataptr->data[0];
		ret = AUDDRV_Get_FDMBCParm((void *)(&dataptr->data[1]), size);

		if (!ret) {
			if (copy_to_user((void __user *)arg, dataptr,
			sizeof(UserCtrl_data_t))) {
				if (dataptr != NULL) {
					kfree(dataptr);
					dataptr = NULL;
				}
				return -EFAULT;
			}
		}

		if (dataptr != NULL) {
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
	hwdep->private_data = NULL;
}

static unsigned int hwdep_poll(struct snd_hwdep *hw, struct file *file,
			       poll_table *wait)
{
	unsigned int mask = 0;
	bcm_caph_hwdep_voip_t *pVoIP;

	pVoIP = (bcm_caph_hwdep_voip_t *) hw->private_data;
	if (!pVoIP)
		return 0;
	poll_wait(file, &pVoIP->sleep, wait);
	if (pVoIP->status == VoIP_Hwdep_Status_Started) {

		if (pVoIP->frames_available_to_read > 0)
			mask = POLLIN | POLLRDNORM;
		if (pVoIP->writecount > 0)	/* buffer available to write */
			mask |= POLLOUT | POLLWRNORM;
	}

	/* DEBUG("voip_poll mask %ld\n",mask); */

	return mask;
}

/**
 * HwdepDeviceNew: Create HWDEP device
 *
 * Return 0 for success
 */
int HwdepDeviceNew(struct snd_card *card)
{
	int err = 0;
	struct snd_hwdep *pHwdep;

	err = snd_hwdep_new(card, "Broadcom CAPH VOIP", 0, &pHwdep);
	if (err < 0) {
		aError("error create hwdep device\n");
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


void voip_log_capture(CAPTURE_POINT_t log_point, unsigned char *buf,
					int size) {

	struct snd_pcm_substream *substream;
	struct snd_pcm_runtime *runtime;

	if (voip_log.voip_log_on <= 0)
		return;
	if (log_point == AUD_LOG_VOCODER_UL) {
		substream = &voip_log.substream_ul;
		runtime = &voip_log.runtime_ul;
		if (mutex_lock_interruptible(&voip_log.voip_ul_mutex))
			return;

	} else {
		substream = &voip_log.substream_dl;
		runtime = &voip_log.runtime_dl;
		if (mutex_lock_interruptible(&voip_log.voip_dl_mutex))
			return;
	}


	runtime->rate = AUDIO_SAMPLING_RATE_16000;
	runtime->sample_bits = 16;
	runtime->channels = AUDIO_CHANNEL_MONO;
	runtime->dma_area = buf;
	runtime->dma_bytes = 2*size;
	substream->runtime = runtime;


	if (log_point == AUD_LOG_VOCODER_UL)
		mutex_unlock(&voip_log.voip_ul_mutex);
	else
		mutex_unlock(&voip_log.voip_dl_mutex);

	logmsg_ready(substream, log_point);

}
