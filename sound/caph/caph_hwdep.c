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
#include "osdal_os.h"
#include "resultcode.h"
#include "audio_consts.h"

#include "csl_aud_drv.h"
#include "audio_controller.h"
#include "audio_ddriver.h"
#include "bcm_audio_devices.h"
#include "bcm_audio_thread.h"
#include "caph_common.h"

#include "shared.h"
#include "csl_dsp.h"
#include "osdw_dsp_drv.h"
#include "csl_audio_render.h"
#include "bcm_audio.h"


/* Local defines */

#define VOIP_FRAMES_IN_BUFFER 50

typedef	enum{
	VoIP_Hwdep_Status_None,
	VoIP_Hwdep_Status_Opened,
	VoIP_Hwdep_Status_Started,
	VoIP_Hwdep_Status_Stopped,
	VoIP_Hwdep_Status_Released,
}VoIP_Hwdep_Status_t;

typedef struct __audio_voip_driver
{
    UInt32 voip_data_dl_rd_index;
    UInt32 voip_data_dl_wr_index;
    UInt32 voip_data_ul_rd_index;
    UInt32 voip_data_ul_wr_index;
    UInt8* voip_data_dl_buf_ptr;
    UInt8* voip_data_ul_buf_ptr;
    AUDIO_DRIVER_HANDLE_t drv_handle;
}audio_voip_driver_t;

typedef struct __bcm_caph_hwdep
{
	int	status[VoIP_Total]; 
	int	frames_available_to_read;
	int	frames_available_to_write;
	wait_queue_head_t sleep;
	audio_voip_driver_t* buffer_handle;
	voip_data_t voip_driver_input;
	UInt8 voip_type;
	UInt32 frame_size; 
	UInt32 buffer_size; 
}bcm_caph_hwdep_t;

static const UInt16 sVoIPFrameLen[] = {320, 158, 36, 164, 640, 68};

static UInt8 sVoIPAMRSilenceFrame[1] = {0x000f}; 

/* local functions */
static void HWDEP_VOIP_DumpUL_CB(void *pPrivate, UInt8	*pSrc, UInt32 nSize);
static void HWDEP_VOIP_FillDL_CB(void *pPrivate, UInt8 *pDst, UInt32 nSize);

static void FillSilenceFrame(UInt32 codec_type, UInt32 frame_size,UInt8 *pDst)
{
	VOIP_Buffer_t tmpBuf;
	memset(&tmpBuf, 0, frame_size);
	
	if (codec_type == VoIP_Codec_AMR475)
		tmpBuf.voip_frame.frame_amr[0] = sVoIPAMRSilenceFrame[0];
	else if (codec_type == VOIP_Codec_AMR_WB_7K)
		tmpBuf.voip_frame.frame_amr_wb.frame_type = sVoIPAMRSilenceFrame[0];
	else if (codec_type == VoIP_Codec_FR)
	{
		tmpBuf.voip_frame.frame_fr[0] = 1;
		tmpBuf.voip_frame.frame_fr[1] = 0;
		tmpBuf.voip_frame.frame_fr[2] = 0;
	}
	else if (codec_type == VOIP_Codec_G711_U)
	{
		tmpBuf.voip_frame.frame_g711[0].frame_type = 1;
		tmpBuf.voip_frame.frame_g711[1].frame_type = 1; 		
	}	
	memcpy(pDst, &tmpBuf,frame_size);
}

static void HWDEP_VOIP_DumpUL_CB(void *pPrivate, UInt8	*pSrc, UInt32 nSize)
{
	bcm_caph_hwdep_t *pVoIP;
	pVoIP = (bcm_caph_hwdep_t *)pPrivate;

	BCM_AUDIO_DEBUG("HWDEP_VOIP_DumpUL_CB nSize %d pVoIP 0x%x\n", nSize,pVoIP);

	if( pVoIP->buffer_handle )
	{
		if( pVoIP->buffer_handle->voip_data_ul_buf_ptr )
		{	
			memcpy(pVoIP->buffer_handle->voip_data_ul_buf_ptr + pVoIP->buffer_handle->voip_data_ul_wr_index, pSrc, nSize);
			pVoIP->frames_available_to_read++;
			pVoIP->buffer_handle->voip_data_ul_wr_index += nSize;
			if( pVoIP->buffer_handle->voip_data_ul_wr_index >= pVoIP->buffer_size )
			{
				pVoIP->buffer_handle->voip_data_ul_wr_index -= pVoIP->buffer_size;
			}
		}
	}

	BCM_AUDIO_DEBUG("HWDEP_VOIP_DumpUL_CB  pVoIP->buffer_handle->voip_data_ul_wr_index 0x%x, pVoIP->buffer_size 0x%x \n",  pVoIP->buffer_handle->voip_data_ul_wr_index,pVoIP->buffer_size);
	
	wake_up(&pVoIP->sleep);
}

static void HWDEP_VOIP_FillDL_CB(void *pPrivate, UInt8 *pDst, UInt32 nSize)
{
	bcm_caph_hwdep_t *pVoIP;
	pVoIP = (bcm_caph_hwdep_t *)pPrivate;
		
	BCM_AUDIO_DEBUG("HWDEP_VOIP_FillDL_CB nSize %d \n", nSize);

	if( pVoIP->buffer_handle->voip_data_dl_buf_ptr )
    {
	      BCM_AUDIO_DEBUG("HWDEP_VOIP_FillDL_CB pVoIP->frames_available_to_write %d\n", pVoIP->frames_available_to_write);
	
		  if(pVoIP->frames_available_to_write == 0)
          {
       		/* fill with silent data based on the frame type  */
              FillSilenceFrame(pVoIP->voip_driver_input.codec_type,nSize,pDst);
          }
		  else
		  {
	          memcpy(pDst,  pVoIP->buffer_handle->voip_data_dl_buf_ptr +  pVoIP->buffer_handle->voip_data_dl_rd_index, nSize);
    	      pVoIP->frames_available_to_write--;
        	  pVoIP->buffer_handle->voip_data_dl_rd_index += nSize;
	          if(pVoIP->buffer_handle->voip_data_dl_rd_index >= pVoIP->buffer_size)
    	      {
        	      pVoIP->buffer_handle->voip_data_dl_rd_index -= pVoIP->buffer_size;
          	  }
		  }
     }				
	 wake_up(&pVoIP->sleep);
}

//---------------------------------------------------------

long voip_read(struct snd_hwdep *hw, char __user *buf,	 long count, loff_t *offset)
{
	bcm_caph_hwdep_t	*pVoIP;
	long ret = 0;

	pVoIP = (bcm_caph_hwdep_t *)hw->private_data;	
	BCM_AUDIO_DEBUG("voip_read count %ld\n",count);
	
	if((pVoIP->status[VoIP_UL] == VoIP_Hwdep_Status_Started || pVoIP->status[VoIP_DL_UL] == VoIP_Hwdep_Status_Started) && (pVoIP->frames_available_to_read>0))
	{
		if( pVoIP->frames_available_to_read)
		{
			if( pVoIP->buffer_handle->voip_data_ul_buf_ptr )
			{
				ret = copy_to_user(buf, pVoIP->buffer_handle->voip_data_ul_buf_ptr + pVoIP->buffer_handle->voip_data_ul_rd_index,pVoIP->frame_size);
				pVoIP->frames_available_to_read--;
				pVoIP->buffer_handle->voip_data_ul_rd_index += pVoIP->frame_size;
				if( pVoIP->buffer_handle->voip_data_ul_rd_index >= pVoIP->buffer_size )
				{
					pVoIP->buffer_handle->voip_data_ul_rd_index -= pVoIP->buffer_size;
				}
				ret = pVoIP->frame_size;
			}
		}
	}
	else
		ret = 0;	
			
	return ret;
}

long voip_write(struct snd_hwdep *hw, const char __user *buf, long count, loff_t *offset)
{
	bcm_caph_hwdep_t	*pVoIP;
	long ret;
	pVoIP = (bcm_caph_hwdep_t	*)hw->private_data;	

	BCM_AUDIO_DEBUG("voip_write count %d pVoIP->frame_size %d\n",count,pVoIP->frame_size);

	if((pVoIP->status[VoIP_DL] == VoIP_Hwdep_Status_Started || pVoIP->status[VoIP_DL_UL] == VoIP_Hwdep_Status_Started) && (pVoIP->buffer_handle))
    {
        if(pVoIP->buffer_handle->voip_data_dl_buf_ptr)
        {
            ret = copy_from_user(pVoIP->buffer_handle->voip_data_dl_buf_ptr + pVoIP->buffer_handle->voip_data_dl_wr_index, buf, pVoIP->frame_size);
            pVoIP->frames_available_to_write++;
            pVoIP->buffer_handle->voip_data_dl_wr_index += pVoIP->frame_size;
            if( pVoIP->buffer_handle->voip_data_dl_wr_index >= pVoIP->buffer_size)
            {
                pVoIP->buffer_handle->voip_data_dl_wr_index -= pVoIP->buffer_size;
            }
        }
		ret = pVoIP->frame_size;
    }
	else
		ret =  0;
	return ret;
}

int voip_open(struct snd_hwdep *hw, struct file * file)
{
	bcm_caph_hwdep_t	*pVoIP;
	UInt8 i;
	hw->private_data = kzalloc(sizeof(bcm_caph_hwdep_t), GFP_KERNEL);
	CAPH_ASSERT(hw->private_data);
	pVoIP = (bcm_caph_hwdep_t	*)hw->private_data;

	BCM_AUDIO_DEBUG("VoIP_Ioctl_Open\n");

	init_waitqueue_head(&pVoIP->sleep);

	for(i=0;i<VoIP_Total;i++)
		pVoIP->status[i] = VoIP_Hwdep_Status_Opened; 
		
	return 0;
}


int voip_release(struct snd_hwdep *hw, struct file * file)
{	
	bcm_caph_hwdep_t	*pVoIP;
	UInt8 i;
	pVoIP = (bcm_caph_hwdep_t	*)hw->private_data;
	
	BCM_AUDIO_DEBUG("VoIP_Ioctl_Release\n");
		
	for(i=0;i<VoIP_Total;i++)
		pVoIP->status[i] = VoIP_Hwdep_Status_Released; //Release all the types
	return 0;
}

int voip_ioctl(struct snd_hwdep *hw, struct file *file, unsigned int cmd, unsigned long arg)
{
		bcm_caph_hwdep_t	*pVoIP;
		AUDIO_DRIVER_CallBackParams_t	cbParams;
		voip_start_stop_type_t *voip_type;
		int ret = 0;
		pVoIP = (bcm_caph_hwdep_t	*)hw->private_data;

		BCM_AUDIO_DEBUG("voip_ioctl cmd=%d\n",cmd);
		
		switch (cmd) {
			case VoIP_Ioctl_GetVersion:
			{
				//ret = put_user(BrcmAACEncVersion, (int __user *)arg);
			}
			break;
			case VoIP_Ioctl_Start:
			{
				voip_type = (voip_start_stop_type_t *)arg;	
				BCM_AUDIO_DEBUG("VoIP_Ioctl_Start pVoIP->voip_type %d\n",pVoIP->voip_type);

				/* Retain this testing code until set parameters functionality is working*/
				pVoIP->voip_driver_input.codec_type = 0; //PCM 8K default
				
				pVoIP->buffer_handle = (audio_voip_driver_t*) OSDAL_ALLOCHEAPMEM(sizeof(audio_voip_driver_t));				
				if ( pVoIP->buffer_handle )
					memset((UInt8*) pVoIP->buffer_handle, 0, sizeof(audio_voip_driver_t));
				else
					return -ENOMEM;

				pVoIP->frame_size = sVoIPFrameLen[pVoIP->voip_driver_input.codec_type];

				BCM_AUDIO_DEBUG("VoIP_Ioctl_Start pVoIP->frame_size %d\n",pVoIP->frame_size);

				pVoIP->buffer_size = pVoIP->frame_size * VOIP_FRAMES_IN_BUFFER;
				
				pVoIP->buffer_handle->voip_data_dl_buf_ptr = (UInt8*) OSDAL_ALLOCHEAPMEM(pVoIP->buffer_size);				
				if( pVoIP->buffer_handle->voip_data_dl_buf_ptr )
				{
					memset(pVoIP->buffer_handle->voip_data_dl_buf_ptr, 0, pVoIP->buffer_size);
				}
				else
				{
					OSDAL_FREEHEAPMEM( pVoIP->buffer_handle );
					return -ENOMEM;
				}
				
				pVoIP->buffer_handle->voip_data_ul_buf_ptr = (UInt8*) OSDAL_ALLOCHEAPMEM(pVoIP->buffer_size);
				if( pVoIP->buffer_handle->voip_data_ul_buf_ptr )
					memset(pVoIP->buffer_handle->voip_data_ul_buf_ptr, 0, pVoIP->buffer_size);
				else
				{
					OSDAL_FREEHEAPMEM( pVoIP->buffer_handle->voip_data_ul_buf_ptr );
					OSDAL_FREEHEAPMEM( pVoIP->buffer_handle );
					return -ENOMEM;
				}
				
				pVoIP->buffer_handle->drv_handle = AUDIO_DRIVER_Open(AUDIO_DRIVER_VOIP);
				if ( !pVoIP->buffer_handle->drv_handle )
				{
					OSDAL_FREEHEAPMEM( pVoIP->buffer_handle->voip_data_dl_buf_ptr );
					OSDAL_FREEHEAPMEM( pVoIP->buffer_handle->voip_data_ul_buf_ptr );
					OSDAL_FREEHEAPMEM( pVoIP->buffer_handle);
					return -ENOMEM;
				}
				
				//set UL callback

				cbParams.voipULCallback = HWDEP_VOIP_DumpUL_CB;
				cbParams.pPrivateData = (void *)pVoIP;
				AUDIO_DRIVER_Ctrl(pVoIP->buffer_handle->drv_handle,AUDIO_DRIVER_SET_VOIP_UL_CB,(void*)&cbParams);
				
				//set DL callback
				cbParams.voipDLCallback = HWDEP_VOIP_FillDL_CB;
				cbParams.pPrivateData = (void *)pVoIP;
				AUDIO_DRIVER_Ctrl(pVoIP->buffer_handle->drv_handle,AUDIO_DRIVER_SET_VOIP_DL_CB,(void*)&cbParams);

				AUDCTRL_EnableTelephony(AUDIO_HW_VOICE_IN,AUDIO_HW_VOICE_OUT,AUDCTRL_MIC_MAIN,AUDCTRL_SPK_HANDSET);
				//AUDCTRL_EnableTelephony(AUDIO_HW_VOICE_IN,AUDIO_HW_VOICE_OUT,pVoIP->voip_driver_input.mic,pVoIP->voip_driver_input.spk);
				AUDCTRL_SetTelephonySpkrVolume(AUDIO_HW_VOICE_OUT, AUDCTRL_SPK_HANDSET, AUDIO_VOLUME_DEFAULT, AUDIO_GAIN_FORMAT_VOL_LEVEL);
				//AUDCTRL_SetTelephonySpkrVolume(AUDIO_HW_VOICE_OUT, pVoIP->voip_driver_input.spk, AUDIO_VOLUME_DEFAULT, AUDIO_GAIN_FORMAT_VOL_LEVEL);
			
				AUDIO_DRIVER_Ctrl(pVoIP->buffer_handle->drv_handle,AUDIO_DRIVER_START,&pVoIP->voip_driver_input.codec_type);
				pVoIP->voip_type = *voip_type;
				pVoIP->status[pVoIP->voip_type] = VoIP_Hwdep_Status_Started;				
			}
				
			break;
			case VoIP_Ioctl_Stop:
			{
				BCM_AUDIO_DEBUG("VoIP_Ioctl_StopUL\n");

				AUDIO_DRIVER_Ctrl(pVoIP->buffer_handle->drv_handle,AUDIO_DRIVER_STOP,NULL);				
				AUDCTRL_DisableTelephony (AUDIO_HW_VOICE_IN, AUDIO_HW_VOICE_OUT, AUDCTRL_MIC_MAIN,AUDCTRL_SPK_HANDSET);
				//AUDCTRL_DisableTelephony(AUDIO_HW_VOICE_IN,AUDIO_HW_VOICE_OUT,pVoIP->voip_driver_input.mic,pVoIP->voip_driver_input.spk);

				AUDIO_DRIVER_Close(pVoIP->buffer_handle->drv_handle);			
				OSDAL_FREEHEAPMEM( pVoIP->buffer_handle->voip_data_dl_buf_ptr );
				OSDAL_FREEHEAPMEM( pVoIP->buffer_handle->voip_data_ul_buf_ptr );
				OSDAL_FREEHEAPMEM( pVoIP->buffer_handle);				
				pVoIP->status[pVoIP->voip_type] = VoIP_Hwdep_Status_Stopped;				
				wake_up(&pVoIP->sleep);
		
			}
				break;
			case VoIP_Ioctl_SetParms:
			{
				voip_data_t *data;
				data = (voip_data_t *)arg;
				memcpy(&pVoIP->voip_driver_input,data,sizeof(voip_data_t));

				BCM_AUDIO_DEBUG(" VoIP_Ioctl_SetParms codec type %d, mic %d, spk %d\n",pVoIP->voip_driver_input.codec_type, pVoIP->voip_driver_input.mic,pVoIP->voip_driver_input.spk);
			}
			break;
			case VoIP_Ioctl_GetParms:
			{
			}
			break;
			default:
			{
				ret =  -ENOTTY;
			}
			break;
	
		}
	return ret;
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

	BCM_AUDIO_DEBUG("voip_poll \n");
	
	poll_wait(file, &pVoIP->sleep, wait);
	
	if(pVoIP->status[VoIP_UL]==VoIP_Hwdep_Status_Started)
	{
		if(pVoIP->frames_available_to_read > 0)
			mask = POLLIN | POLLRDNORM;
	}
	else if(pVoIP->status[VoIP_DL]==VoIP_Hwdep_Status_Started)
	{
		if(pVoIP->frames_available_to_write > 0)
			mask = POLLOUT | POLLWRNORM;
	}
	else if(pVoIP->status[VoIP_DL_UL]==VoIP_Hwdep_Status_Started)
	{
		if((pVoIP->frames_available_to_read > 0) && (pVoIP->frames_available_to_write > 0))
			mask = POLLIN | POLLRDNORM | POLLOUT | POLLWRNORM;
		else if(pVoIP->frames_available_to_read > 0)
			mask = POLLIN | POLLRDNORM; 
		else if(pVoIP->frames_available_to_write > 0)
			mask = POLLOUT | POLLWRNORM;
	}

	BCM_AUDIO_DEBUG("voip_poll mask %ld\n",mask);
		
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

