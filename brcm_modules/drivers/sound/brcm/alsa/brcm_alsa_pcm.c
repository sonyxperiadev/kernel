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

#include <linux/broadcom/hw_cfg.h>

#include "brcm_alsa.h"
#include "audvoc_drv.h"
#include "audvoc_consts.h"

#include "audio_ipc_consts.h"
#include "hal_audio.h"
#include "hal_audio_access.h"
#include <mach/reg_sys.h>

#include "vpu.h"


//int DbgLogRegs();
int audvoc_data_transfer(struct snd_pcm_substream * substream);
long GetCtrlValue(int ctrlIndex);
extern int	SwitchCallingRate(void);


typedef	struct	_TIDChanOfPlaybackDev
{
	AudioClientID_en_t	clientID;
	AUDIO_CHANNEL_t	channel;
	BRCM_CTRL_ENU	volctrl;
	UInt16			volume;
}TIDChanOfPlaybackDev;

#define	SPECIAL_SPK_HEADSET_VOL		0xFFFF

static TIDChanOfPlaybackDev	sgTableIDChannelOfDev[] = {
	{AUDIO_ID_FOR_PCM, AUDIO_CHNL_SPEAKER, 	BRCM_CTL_Speaker_Playback_Volume},//spk normal
	{AUDIO_ID_FOR_PCM, AUDIO_CHNL_SPEAKER, 	BRCM_CTL_Speaker_Playback_Volume},//spk ringtone
	{AUDIO_ID_CALL,	AUDIO_CHNL_SPEAKER, 	BRCM_CTL_Speaker_Playback_Volume}, //spk incall
	{AUDIO_ID_FOR_PCM, AUDIO_CHNL_EARPIECE, 	BRCM_CTL_Speaker_Playback_Volume},//earpiece  normal
	{AUDIO_ID_FOR_PCM, AUDIO_CHNL_EARPIECE, 	BRCM_CTL_Speaker_Playback_Volume},//earpiece ringtone
	{AUDIO_ID_CALL,	AUDIO_CHNL_EARPIECE, 	BRCM_CTL_Speaker_Playback_Volume}, //earpiece incall
	{AUDIO_ID_FOR_PCM, AUDIO_CHNL_HEADPHONE, BRCM_CTL_Speaker_Playback_Volume},//headset  normal
	{AUDIO_ID_FOR_PCM, AUDIO_CHNL_HEADPHONE, BRCM_CTL_Speaker_Playback_Volume},//headset ringtone
	{AUDIO_ID_CALL, AUDIO_CHNL_HEADPHONE, 	BRCM_CTL_Speaker_Playback_Volume},	 //headset incall
	{AUDIO_CHNL_BT_MONO, AUDIO_CHNL_BT_MONO, BRCM_CTL_BTHeadset_Volume},//bt headset  normal
	{AUDIO_CHNL_BT_MONO, AUDIO_CHNL_BT_MONO, BRCM_CTL_BTHeadset_Ringtone_Volume},//bt headset ringtone
	{AUDIO_ID_CALL, AUDIO_CHNL_BT_MONO, BRCM_CTL_BTHeadset_Incall_Volume},	 //bt headset incall
	{AUDIO_ID_FOR_PCM, AUDIO_CHNL_HEADPHONE, BRCM_CTL_Speaker_Playback_Volume, SPECIAL_SPK_HEADSET_VOL},//spk-headset  normal
	{AUDIO_ID_FOR_PCM, AUDIO_CHNL_HEADPHONE, BRCM_CTL_Speaker_Playback_Volume, SPECIAL_SPK_HEADSET_VOL}//spk-headset ringtone
};

#define	NUM_PLAYBACK_SUBDEVICE	(sizeof(sgTableIDChannelOfDev)/sizeof(sgTableIDChannelOfDev[0]))
#define	NUM_CAPTURE_SUBDEVICE	3


#define	PCM_MAX__PLAYBACK_BUF_BYTES		(60*1024)
#define	PCM_MIN_PERIOD_BYTES		(12*1024)

#define	VPU_PCM_DATA_BLK_LENTH			(320*2)
#define	PERIOD_CAPTURE_MAX_BYTES		(VPU_PCM_DATA_BLK_LENTH*4)
#define	PCM_MAX_CAPTURE_BUF_BYTES 		(6*1024) //(8*1024) // = (8000*2)		// one second data

#define	PCM_TOTAL_BUF_BYTES	(PCM_MAX_CAPTURE_BUF_BYTES+PCM_MAX__PLAYBACK_BUF_BYTES)
/* hardware definition */
static struct snd_pcm_hardware brcm_playback_hw =
{
	.info = (SNDRV_PCM_INFO_MMAP | SNDRV_PCM_INFO_INTERLEAVED |
			SNDRV_PCM_INFO_BLOCK_TRANSFER |	SNDRV_PCM_INFO_MMAP_VALID|SNDRV_PCM_INFO_PAUSE),
	.formats = SNDRV_PCM_FMTBIT_S16_LE,
	.rates = SNDRV_PCM_RATE_8000_48000,
	.rate_min = 8000,
	.rate_max = 48000,
	.channels_min = 1,
	.channels_max = 2,
	.buffer_bytes_max = PCM_MAX__PLAYBACK_BUF_BYTES, //shared memory buffer
	.period_bytes_min = PCM_MIN_PERIOD_BYTES,
	.period_bytes_max = PCM_MIN_PERIOD_BYTES, //half shared memory buffer
	.periods_min = 2,
	.periods_max = PCM_MAX__PLAYBACK_BUF_BYTES/PCM_MIN_PERIOD_BYTES,
};


static struct snd_pcm_hardware brcm_capture_hw =
{
	.info = (SNDRV_PCM_INFO_MMAP | SNDRV_PCM_INFO_INTERLEAVED |
		SNDRV_PCM_INFO_BLOCK_TRANSFER |	SNDRV_PCM_INFO_MMAP_VALID),
	.formats = SNDRV_PCM_FMTBIT_S16_LE,
	.rates = SNDRV_PCM_RATE_8000,
	.rate_min = 8000,
	.rate_max = 8000,
	.channels_min = 1,
	.channels_max = 1,
	.buffer_bytes_max = PCM_MAX_CAPTURE_BUF_BYTES,	// one second data
	.period_bytes_min = VPU_PCM_DATA_BLK_LENTH, 		// one AMR brocks (each is 4 AMR frames) for pingpong, each blocks is 80 ms, 8000*0.020*2=320
	.period_bytes_max =  PERIOD_CAPTURE_MAX_BYTES, //half buffer
	.periods_min = 2,
	.periods_max = PCM_MAX_CAPTURE_BUF_BYTES/VPU_PCM_DATA_BLK_LENTH,
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++
//Function name: AudioHalInit
//Description: 
//       Init audio HAL
//
//-------------------------------------------------------------
int AudioHalInit(void)
{
	HAL_AUDIO_Init(NULL);
	return 0;
}



//++++++++++++++++++++++++++++++++++++++
//EnableVoiceCall4Dev
//	Enable voice path for the sub device
//	Return negative if error, zero otherwise
//-------------------------------------------
int EnableVoiceCall4Dev(int subdevice)
{
	HAL_AUDIO_Control_Params_un_t audioControlParam;
	if(subdevice<0 || subdevice>=NUM_PLAYBACK_SUBDEVICE)
	{
		DEBUG("Invalid subdevice # %d\n", subdevice);
		return -1;
	}
	audioControlParam.param_pathCfg.outputChnl = sgTableIDChannelOfDev[subdevice].channel;
	audioControlParam.param_pathCfg.audioID = sgTableIDChannelOfDev[subdevice].clientID;
	audioControlParam.param_pathCfg.volumeLevel = GetCtrlValue(BRCM_CTL_EAR_Playback_Volume);
	if(AUDIO_ID_CALL == audioControlParam.param_pathCfg.audioID)
	{
		HAL_AUDIO_Ctrl( ACTION_AUD_EnablePath, &audioControlParam, NULL);

		
		if(0==SwitchCallingRate())
			return 0;
		if(0==GetCtrlValue(BRCM_CTL_Mic_Capture_Mute))
			HAL_AUDIO_Ctrl(ACTION_AUD_UnmuteMic, NULL, NULL);
		else
			HAL_AUDIO_Ctrl(ACTION_AUD_MuteMic, NULL, NULL);
		return 0;
	}
	return -2;
}

//++++++++++++++++++++++++++++++++++++++
//DisableVoiceCall4Dev
//	Disable voice path for the sub device
//	Return negative if error, zero otherwise
//-------------------------------------------

int DisableVoiceCall4Dev(int subdevice)
{
	HAL_AUDIO_Control_Params_un_t audioControlParam;
	if(subdevice<0 || subdevice>=NUM_PLAYBACK_SUBDEVICE)
	{
		DEBUG("Invalid subdevice # %d\n", subdevice);
		return -1;
	}
	audioControlParam.param_pathCfg.outputChnl = sgTableIDChannelOfDev[subdevice].channel;
	audioControlParam.param_pathCfg.audioID = sgTableIDChannelOfDev[subdevice].clientID;
	if(AUDIO_ID_CALL == audioControlParam.param_pathCfg.audioID)
	{
//		DEBUG("DisableVoiceCall4Dev id %d channel %d\n", audioControlParam.param_pathCfg.audioID, audioControlParam.param_pathCfg.outputChnl);
		HAL_AUDIO_Ctrl( ACTION_AUD_DisablePath, &audioControlParam, NULL);
		return 0;
	}
	return -2;

}

//++++++++++++++++++++++++++++++++++++++
//EnableAudioPath4Dev
//	Enable audio path for the sub device
//	Return negative if error, zero otherwise
//-------------------------------------------

int EnableAudioPath4Dev(int subdevice)
{
	HAL_AUDIO_Control_Params_un_t audioControlParam;
	if(subdevice<0 || subdevice>=NUM_PLAYBACK_SUBDEVICE)
	{
		DEBUG("Invalid subdevice # %d\n", subdevice);
		return -1;
	}
	audioControlParam.param_pathCfg.outputChnl = sgTableIDChannelOfDev[subdevice].channel;
	audioControlParam.param_pathCfg.audioID = sgTableIDChannelOfDev[subdevice].clientID;
	audioControlParam.param_pathCfg.volumeLevel = GetCtrlValue(sgTableIDChannelOfDev[subdevice].volctrl);

	if(AUDIO_ID_CALL == audioControlParam.param_pathCfg.audioID)
		audioControlParam.param_pathCfg.audioID = AUDIO_ID_FOR_PCM;

	//hal needs to distinguish mono and stereo for AUDIO_ID_WAVETONE_POLY to config chan correctly		
	if(audioControlParam.param_pathCfg.audioID == AUDIO_ID_WAVETONE_POLY)
	{
		HAL_AUDIO_Param_CodecCfg_t codecCfg;

		codecCfg.audioID = AUDIO_ID_WAVETONE_POLY;
		codecCfg.monoStereo = TYPE_STEREO;
		HAL_AUDIO_Ctrl( ACTION_AUD_ConfigCodecParam, &codecCfg, NULL );
	}

	DEBUG("EnableAudioPath4Dev channel=%d id=%d vol=%d\n",  audioControlParam.param_pathCfg.outputChnl, audioControlParam.param_pathCfg.audioID, audioControlParam.param_pathCfg.volumeLevel);
		HAL_AUDIO_Ctrl( ACTION_AUD_EnablePath, &audioControlParam, NULL);

	if(SPECIAL_SPK_HEADSET_VOL == sgTableIDChannelOfDev[subdevice].volume)
	{
		audioControlParam.param_pathCfg.audioID = AUDIO_ID_USER_EXT_AMP; 
		audioControlParam.param_pathCfg.outputChnl = AUDIO_CHNL_RESERVE; 
		HAL_AUDIO_Ctrl( ACTION_AUD_SetVolumeWithPath, &audioControlParam, NULL); 
	}

	return 0;
}

//++++++++++++++++++++++++++++++++++++++
//DisableAudioPath4Dev
//	Disable audio path for the sub device
//	Return negative if error, zero otherwise
//-------------------------------------------
int DisableAudioPath4Dev(int subdevice)
{
	HAL_AUDIO_Control_Params_un_t audioControlParam;
	if(subdevice<0 || subdevice>=NUM_PLAYBACK_SUBDEVICE)
	{
		DEBUG("Invalid subdevice # %d\n", subdevice);
		return -1;
	}
	audioControlParam.param_pathCfg.outputChnl = sgTableIDChannelOfDev[subdevice].channel;
	audioControlParam.param_pathCfg.audioID = sgTableIDChannelOfDev[subdevice].clientID;
//	DEBUG("DisableAudioPath4Dev channel=%d id=%d vol=%d\n",  audioControlParam.param_pathCfg.outputChnl, audioControlParam.param_pathCfg.audioID, audioControlParam.param_pathCfg.volumeLevel);
	if(AUDIO_ID_CALL == audioControlParam.param_pathCfg.audioID)
		audioControlParam.param_pathCfg.audioID = AUDIO_ID_FOR_PCM;
	
	
		HAL_AUDIO_Ctrl( ACTION_AUD_DisablePath, &audioControlParam, NULL);
		return 0;
}



//+++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  Function Name: PcmHwParams
//
//  Description: Set hardware parameters
//
//------------------------------------------------------------
static int PcmHwParams(
	struct snd_pcm_substream * substream,
      struct snd_pcm_hw_params * hw_params
)
{
//	DEBUG("\n %lx:hw_params %d\n",jiffies,(int)substream->stream);
	
	DEBUG("\t params_access=%d params_format=%d  params_subformat=%d params_channels=%d params_rate=%d, buffer bytes=%d\n",
		 params_access(hw_params), params_format(hw_params), params_subformat(hw_params), params_channels(hw_params), params_rate(hw_params), params_buffer_bytes(hw_params));

	return snd_pcm_lib_malloc_pages(substream, params_buffer_bytes(hw_params));
}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  Function Name: PcmHwFree
//
//  Description: Release hardware resource
//
//------------------------------------------------------------
static int PcmHwFree(
	struct snd_pcm_substream * substream
)
{
	int res;
	brcm_alsa_chip_t *chip = snd_pcm_substream_chip(substream);
	
	DEBUG("\n %lx:hw_free - stream=%x\n",jiffies, substream);
	flush_workqueue(chip->pWorkqueue_PCM);
	
	snd_pcm_stream_lock_irq(substream);
	res = snd_pcm_lib_free_pages(substream);
	snd_pcm_stream_unlock_irq(substream);
	return res;
}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  Function Name: PcmPlaybackOpen
//
//  Description: Open PCM playback device
//
//------------------------------------------------------------
static int PcmPlaybackOpen(
	struct snd_pcm_substream * substream
)
{
	P_DEVICE_CHANNEL dev;
	
	brcm_alsa_chip_t *chip = snd_pcm_substream_chip(substream);
	struct snd_pcm_runtime *runtime = substream->runtime;
	int err=0;
		
	runtime->hw = brcm_playback_hw;	
	chip->substream[0] = substream;


	// open low level device
	dev = audvoc_open (0, 0, 0);
	substream->runtime->private_data = dev;

	DEBUG("\n %lx:playback_open subdevice=%d PCM_TOTAL_BUF_BYTES=%d\n",jiffies, substream->number, PCM_TOTAL_BUF_BYTES);
		
	return err;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  Function Name: PcmPlaybackClose
//
//  Description: Close PCM playback device
//
//------------------------------------------------------------
static int PcmPlaybackClose(struct snd_pcm_substream * substream)
{
	brcm_alsa_chip_t *chip = snd_pcm_substream_chip(substream);
	
	DEBUG("\n %lx:playback_close subdevice=%d\n",jiffies, substream->number);

	DisableVoiceCall4Dev(substream->number);

	// close low level device
	
	snd_pcm_stream_lock_irq(substream);
	audvoc_close(substream);
	substream->runtime->private_data = NULL;
	snd_pcm_stream_unlock_irq(substream);

	chip->substream[0] = NULL;



	return 0;
}



//+++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  Function Name: PcmPlaybackPrepare
//
//  Description: Prepare PCM playback device, next call is Trigger or Close
//
//------------------------------------------------------------
static int PcmPlaybackPrepare(
	struct snd_pcm_substream * substream
)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	brcm_alsa_chip_t *chip = snd_pcm_substream_chip(substream);
	AUDVOC_CFG_INFO p_cfg;
	P_DEVICE_CHANNEL p_dev_channel = (P_DEVICE_CHANNEL) runtime->private_data;

//	DEBUG("\n%lx:playback_prepare rate=%d, format=%d, channels=%d subdevice=%d\n",jiffies, runtime->rate, runtime->format, runtime->channels, substream->number);
	DEBUG("\nplayback_prepare period=%d period_size=%d bufsize=%d threshold=%d\n", runtime->periods, 
			frames_to_bytes(runtime, runtime->period_size), frames_to_bytes(runtime, runtime->buffer_size), runtime->stop_threshold);

	chip->rate[0]         = runtime->rate;
	chip->buffer_bytes[0] = runtime->dma_bytes;
	chip->period_bytes[0] = frames_to_bytes(runtime, runtime->period_size);
	chip->pcm_ptr[0] = 0;
	chip->pcm_write_ptr[0] = 0;
	chip->pcm_read_ptr[0]  = 0;
	chip->last_pcm_rdptr[0] = 0;

	p_cfg.channel	  = runtime->channels;
	p_cfg.sample_rate = runtime->rate;

	audvoc_configure_audio_controller(p_dev_channel,&p_cfg);
	audvoc_configure_channel(p_dev_channel);


	EnableVoiceCall4Dev(substream->number);

//	DEBUG("\n%lx:playback_prepare period bytes=%d, periods =%d, buffersize=%d\n",jiffies, g_brcm_alsa_chip->period_bytes[0], runtime->periods, runtime->dma_bytes);



	return 0;
}
#define	INIT_PCM_WORDS		(4*1024)
//+++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  Function Name: InitPlayBackSharedmem
//
//  Description: Setup shared memory address with playback channel structure
//
//------------------------------------------------------------
static int InitPlayBackSharedmem(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	P_DEVICE_CHANNEL p_dev_channel = (P_DEVICE_CHANNEL) runtime->private_data;
	
	memset(p_dev_channel->sink.dst, 0, 24*1024);
	*p_dev_channel->sink.head = INIT_PCM_WORDS; 
	g_brcm_alsa_chip->pcm_ptr[0] += bytes_to_frames(runtime,INIT_PCM_WORDS*2);
	return *p_dev_channel->sink.head;
}

static int InsertZeroPlayBackSharedmem(struct snd_pcm_substream *substream, int words)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	brcm_alsa_chip_t *pChip = snd_pcm_substream_chip(substream);
	UInt16 head;

	P_DEVICE_CHANNEL p_dev_channel = (P_DEVICE_CHANNEL) runtime->private_data;
	memset(p_dev_channel->sink.dst, 0, words*2);
	*p_dev_channel->sink.tail = 0;
	*p_dev_channel->sink.head = words; 

	pChip->pcm_read_ptr[0] += bytes_to_frames(runtime, words*2);
	if(pChip->pcm_read_ptr[0]>runtime->boundary)
		pChip->pcm_read_ptr[0] -= runtime->boundary;

	DEBUG("InsertZeroPlayBackSharedmem words=%d\n", words);
	return *p_dev_channel->sink.head;
}



//+++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  Function Name: PcmPlaybackTrigger
//
//  Description: Command handling function
//
//------------------------------------------------------------
static int PcmPlaybackTrigger(
	struct snd_pcm_substream * substream,
	int cmd
)
{
	int ret = 0;
	brcm_alsa_chip_t *chip = snd_pcm_substream_chip(substream);



//	DEBUG("\n ALSA : PcmPlaybackTrigger \n");
	DEBUG("\n %lx:playback_trigger cmd=%d untime->boundary=%d\n",jiffies,cmd, (int)substream->runtime->boundary );
	
	switch (cmd) 
	{
		case SNDRV_PCM_TRIGGER_START:
			/* do something to start the PCM engine */
			{
				struct snd_pcm_runtime *runtime = substream->runtime;
				P_DEVICE_CHANNEL p_dev_channel = (P_DEVICE_CHANNEL) runtime->private_data;
				int words;

				InitPlayBackSharedmem(substream);
				words = audvoc_data_transfer(substream);
				DEBUG("\n %lx:words transfered%d\n",jiffies,words );
				if(words==0)
					DEBUG("\n %lx:silence words transfered%d\n",jiffies,words );

				//do not turn on audio path until it is started. enable audio path here
				EnableAudioPath4Dev(substream->number);


				if(*p_dev_channel->sink.head==*p_dev_channel->sink.tail)
					*p_dev_channel->sink.head = (*p_dev_channel->sink.head -1)%p_dev_channel->AUDIO_BUF_SIZE;
				ret = audvoc_start_player(substream, (12*1024 - frames_to_bytes(runtime, runtime->period_size)/2));


			}
			break;
		
		case SNDRV_PCM_TRIGGER_STOP:
			/* do something to stop the PCM engine */
			ret = audvoc_stop_player(substream);
			DisableAudioPath4Dev(substream->number);
			break;

		case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
			DisableAudioPath4Dev(substream->number);
			break;
		case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
			EnableAudioPath4Dev(substream->number);
			break;
		default:
		return -EINVAL;
	}	
	
	return ret;
}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  Function Name: PcmPlaybackPointer
//
//  Description: Get playback pointer in frames
//
//------------------------------------------------------------
static snd_pcm_uframes_t PcmPlaybackPointer(struct snd_pcm_substream * substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	P_DEVICE_CHANNEL p_dev_channel = (P_DEVICE_CHANNEL) runtime->private_data;
	snd_pcm_uframes_t pos;

	int	datasize = -*p_dev_channel->sink.tail + *p_dev_channel->sink.head + 1;  
	if(datasize==1)
		DEBUG("\n %lx:Warning playback_pointer head==tail=%d\n",jiffies,(int)*p_dev_channel->sink.tail );
	if(datasize <  0)	datasize += p_dev_channel->AUDIO_BUF_SIZE;
//	pos = g_brcm_alsa_chip->pcm_read_ptr[0] + g_brcm_alsa_chip->pcm_ptr[0] - bytes_to_frames(runtime, datasize*2);
	pos = g_brcm_alsa_chip->pcm_read_ptr[0];// + g_brcm_alsa_chip->pcm_ptr[0];
//	DEBUG("\n pos=%d(%d- %d) pcm_read_ptr=%d datasize=%d\n", pos,  (pos % runtime->buffer_size), (pos-bytes_to_frames(runtime, datasize*2))%runtime->buffer_size, g_brcm_alsa_chip->pcm_read_ptr[0], bytes_to_frames(runtime, datasize*2));
	if(pos<0)
	{
		//DEBUG("\n %lx:Error playback_pointer %d\n",jiffies,(int)wordPos);
		pos += runtime->buffer_size;
	}
	pos %= runtime->buffer_size;
//	DEBUG("PcmPlaybackPointer: hw_ptr=%d pcm_read_ptr=%d words appl_ptr=%d validPcmSize=%d buffersize=%d\n", runtime->status->hw_ptr, g_brcm_alsa_chip->pcm_read_ptr[0], (int)runtime->control->appl_ptr,
//				runtime->control->appl_ptr-g_brcm_alsa_chip->pcm_read_ptr[0],  runtime->buffer_size);

	return pos;
}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  Function Name: GetPlaybackRenderPosition
//
//  Description: 
//	   Get playback position.
//		Position  = DataCopiedToSharedMem - DataRemainInSharedMem
//
//------------------------------------------------------------
snd_pcm_uframes_t GetPlaybackRenderPosition(struct snd_pcm_substream * substream)
{

	struct snd_pcm_runtime *runtime;
	P_DEVICE_CHANNEL p_dev_channel;
	long  pos =0;
	int	datasize;
	
	
	if (!substream)
		return pos;	
	
	if (!substream->runtime)
		return pos;	
	snd_pcm_stream_lock_irq(substream);
	runtime = substream->runtime;
	if(runtime->status->state == SNDRV_PCM_STATE_RUNNING)
	{
		p_dev_channel = (P_DEVICE_CHANNEL) runtime->private_data;
		if(p_dev_channel)
		{
			datasize = -*p_dev_channel->sink.tail + *p_dev_channel->sink.head + 1;  
			if(datasize <  0)	
				datasize += p_dev_channel->AUDIO_BUF_SIZE;

			pos = g_brcm_alsa_chip->pcm_read_ptr[0] - bytes_to_frames(runtime, datasize*2); //+ g_brcm_alsa_chip->pcm_ptr[0] 
			if(pos<0)
				pos = 0;
		}
	}
	
	snd_pcm_stream_unlock_irq(substream);
//	DEBUG("GetPlaybackRenderPosition %d\n", pos);
	return pos;
}



static unsigned int sgpcm_capture_period_bytes[]={VPU_PCM_DATA_BLK_LENTH, VPU_PCM_DATA_BLK_LENTH*2, VPU_PCM_DATA_BLK_LENTH*3, VPU_PCM_DATA_BLK_LENTH*4};  //bytes
static struct snd_pcm_hw_constraint_list sgpcm_capture_period_bytes_constraints_list = 
{
	.count = ARRAY_SIZE(sgpcm_capture_period_bytes),
	.list  = sgpcm_capture_period_bytes,
	.mask  = 0,
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  Function Name: PcmCaptureOpen
//
//  Description: Open PCM capure device
//
//------------------------------------------------------------
static int PcmCaptureOpen(struct snd_pcm_substream * substream)
{
	brcm_alsa_chip_t *chip = snd_pcm_substream_chip(substream);
	struct snd_pcm_runtime *runtime = substream->runtime;
	int err=0;

//	DEBUG("\n ALSA : PcmCaptureOpen \n");
	
	runtime->hw = brcm_capture_hw;	
	chip->substream[1] = substream;
	
											
	err = snd_pcm_hw_constraint_list(runtime,0,SNDRV_PCM_HW_PARAM_PERIOD_BYTES,
										&sgpcm_capture_period_bytes_constraints_list);
										
	if (err<0)
		return err;

	DEBUG("\n %lx:capture_open subdevice=%d\n",jiffies, substream->number);
	return 0;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  Function Name: PcmCaptureClose
//
//  Description: Close PCM capure device
//
//------------------------------------------------------------
static int PcmCaptureClose(struct snd_pcm_substream * substream)
{
	brcm_alsa_chip_t *chip = snd_pcm_substream_chip(substream);

	chip->substream[1] = NULL;
	
	DEBUG("\n %lx:capture_close subdevice=%d\n",jiffies, substream->number);

	return 0;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  Function Name: PcmCapturePrepare
//
//  Description: Prepare hardware to capture
//
//------------------------------------------------------------
static int PcmCapturePrepare(struct snd_pcm_substream * substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;

	DEBUG("\n %lx:capture_prepare: subdevice=%d rate =%d format =%d channel=%d dma_area=0x%x dma_bytes=%d period_bytes=%d avail_min=%d periods=%d buffer_size=%d\n",
		         jiffies,	substream->number, runtime->rate, runtime->format, runtime->channels, (unsigned int)runtime->dma_area, runtime->dma_bytes,
		         frames_to_bytes(runtime, runtime->period_size), frames_to_bytes(runtime, runtime->control->avail_min), runtime->periods, runtime->buffer_size);
	
	g_brcm_alsa_chip->rate[1]         = runtime->rate;
	g_brcm_alsa_chip->buffer_bytes[1] = runtime->dma_bytes;
	g_brcm_alsa_chip->period_bytes[1] = frames_to_bytes(runtime, runtime->period_size);
	g_brcm_alsa_chip->pcm_read_ptr[1]  = 0; 
	g_brcm_alsa_chip->last_pcm_rdptr[1] = 0;


	return 0;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  Function Name: CBDumpFrames
//
//  Description: Call back function to dump recrding frames
//
//------------------------------------------------------------
static Boolean CBDumpFrames(
		UInt8*			pSrc,
		UInt32			nFrames
		)
{
	int		dstIdx, elapsed;
	ssize_t 	bytes;
	unsigned char *	pDstBuf;
	struct snd_pcm_substream *substream = g_brcm_alsa_chip->substream[1];
	struct snd_pcm_runtime *runtime = substream->runtime;
	int split, bufSizeInbytes;
	int endSize = 0;


	if(substream==NULL)
	{
		DEBUG("\n %lx:Error CBDumpFrames called when sunstream is NULL\n",jiffies);
		return FALSE;
	}

	bytes = nFrames*320;

	//copy data
	dstIdx = g_brcm_alsa_chip->pcm_read_ptr[1];
	dstIdx = frames_to_bytes(runtime, dstIdx%runtime->buffer_size); //in bytes
	pDstBuf = substream->runtime->dma_area;

//	DEBUG("\n %lx:CBDumpFrames appl_ptr=%d pSrc=0x%x, nFrames=%d pre_dstIdx=%d\n",jiffies, runtime->control->appl_ptr, (unsigned int)pSrc, (int)nFrames,dstIdx);
	bufSizeInbytes =  frames_to_bytes(runtime, runtime->buffer_size);
	split = (dstIdx+bytes) - bufSizeInbytes;

	if( split >  0)
	{
//		DEBUG("\n Error:CBDumpFrames dstIdx+bytes>=buffer_size: %d+%d>=%d\n",dstIdx, bytes, frames_to_bytes(runtime, runtime->buffer_size));
//		return FALSE;
		endSize = bufSizeInbytes - dstIdx;
		memcpy(pDstBuf+dstIdx, pSrc, endSize);
		dstIdx = 0;
		pSrc += endSize;
	}

	memcpy(pDstBuf+dstIdx, pSrc, bytes-endSize);


	g_brcm_alsa_chip->pcm_read_ptr[1] += bytes_to_frames(runtime, bytes);
	if(g_brcm_alsa_chip->pcm_read_ptr[1]>runtime->boundary)
		g_brcm_alsa_chip->pcm_read_ptr[1] -= runtime->boundary;

	elapsed = g_brcm_alsa_chip->pcm_read_ptr[1] - g_brcm_alsa_chip->last_pcm_rdptr[1];
	if (elapsed <0)
	{
		elapsed += runtime->buffer_size;
		DEBUG("\n CBDumpFrames boundary met:ptr=%d, previous=%d\n",g_brcm_alsa_chip->pcm_read_ptr[1] , g_brcm_alsa_chip->last_pcm_rdptr[1]);
	}
	if( elapsed>= substream->runtime->period_size)
	{
		snd_pcm_period_elapsed(substream);
		g_brcm_alsa_chip->last_pcm_rdptr[1] = g_brcm_alsa_chip->pcm_read_ptr[1];
	}
//	else
//		DEBUG("\n less than 1 period is transfered %d < %d\n", (int)(g_brcm_alsa_chip->pcm_read_ptr[1] - g_brcm_alsa_chip->last_pcm_rdptr[1]),(int)substream->runtime->period_size);


	return TRUE;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  Function Name: VPMCaptureStart
//
//  Description: Start VPU capture
//
//-----------------------------------------------------
int 	VPMCaptureStart(struct snd_pcm_substream * substream)
{
	UInt32				ret;
	UInt8				vpuLink = 3;// 1 - Near, 2 - Far, 3 - Both
	VP_Speech_Mode_t	vpuSpeechMode = VP_SPEECH_MODE_LINEAR_PCM;
	UInt32				nParam = 0;
	brcm_alsa_chip_t *chip = snd_pcm_substream_chip(substream);


	//Enable path for recording: AUDIO_MODE_RECORD
	{
		HAL_AUDIO_Control_Params_un_t audioControlParam;
		HAL_AUDIO_Get_Param_t getParam;
						
		//get presernt Audio channel and volume levels from HAL
		getParam.paramType = GET_AUDIO_CHNL;
		HAL_AUDIO_Ctrl( ACTION_AUD_GetParam, &getParam, NULL);
		audioControlParam.param_pathCfg.outputChnl= (AUDIO_CHANNEL_t) *(getParam.paramPtr);

		getParam.paramType = GET_AUDIO_VOLUMELEVEL;
		HAL_AUDIO_Ctrl( ACTION_AUD_GetParam, &getParam, NULL);
		audioControlParam.param_pathCfg.volumeLevel = (UInt16)*(getParam.paramPtr);	

		//audioControlParam.param_pathCfg.volumeLevel = 40;
		//audioControlParam.param_pathCfg.outputChnl = AUDIO_CHNL_EARPIECE;
		
		if(substream->number == 2) //AUX, wired headset
			audioControlParam.param_pathCfg.outputChnl = AUDIO_CHNL_HEADPHONE;
		audioControlParam.param_pathCfg.audioID = AUDIO_ID_VPU_RECORD;
		HAL_AUDIO_Ctrl( ACTION_AUD_EnablePath, &audioControlParam, NULL);
		DEBUG("\n %lx:Enable path outputChnl=%d vol=%d \n",jiffies, audioControlParam.param_pathCfg.outputChnl, audioControlParam.param_pathCfg.volumeLevel );
	}


	mdelay(100);		// Sleep 100ms to make sure VPU stopped	
	ret = VPU_StartRecordingMemo(CBDumpFrames, vpuSpeechMode, vpuLink, nParam);

	DEBUG("\n %lx:VPMCaptureStart\n",jiffies);

	
	return 0;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  Function Name: VPMCaptureStop
//
//  Description: Start VPU capture
//
//------------------------------------------------------------
int 	VPMCaptureStop(struct snd_pcm_substream * substream)
{
	brcm_alsa_chip_t *chip = snd_pcm_substream_chip(substream);

	VPU_StopMemo();
	DEBUG("\n %lx:VPMCaptureStop: VPU_StopMemo\n",jiffies);
	//Disable path for recording
	{
		HAL_AUDIO_Control_Params_un_t audioControlParam;

		HAL_AUDIO_Get_Param_t getParam;
						
		//get presernt Audio channel and volume levels from HAL
		getParam.paramType = GET_AUDIO_CHNL;
		HAL_AUDIO_Ctrl( ACTION_AUD_GetParam, &getParam, NULL);
		audioControlParam.param_pathCfg.outputChnl= (AUDIO_CHANNEL_t) *(getParam.paramPtr);

		getParam.paramType = GET_AUDIO_VOLUMELEVEL;
		HAL_AUDIO_Ctrl( ACTION_AUD_GetParam, &getParam, NULL);
		audioControlParam.param_pathCfg.volumeLevel = (UInt16)*(getParam.paramPtr);	
		
		//audioControlParam.param_pathCfg.volumeLevel = 30;
		//audioControlParam.param_pathCfg.outputChnl = AUDIO_CHNL_EARPIECE;
		if(substream->number == 2) //AUX, wired headset
			audioControlParam.param_pathCfg.outputChnl = AUDIO_CHNL_HEADPHONE;

		audioControlParam.param_pathCfg.audioID = AUDIO_ID_VPU_RECORD;
		HAL_AUDIO_Ctrl( ACTION_AUD_DisablePath, &audioControlParam, NULL);
	}
	DEBUG("\n %lx:VPMCaptureStop: ACTION_AUD_DisablePath\n",jiffies);
	return 0;
}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  Function Name: PcmCaptureTrigger
//
//  Description: Command handling function
//
//------------------------------------------------------------
static int PcmCaptureTrigger(
	struct snd_pcm_substream * substream, 	//substream
	int cmd									//commands to handle
)
{
	DEBUG("\n %lx:capture_trigger subdevice=%d cmd=%d\n",jiffies,substream->number, cmd);
	
	switch (cmd) 
	{
		case SNDRV_PCM_TRIGGER_START:
			/* do something to start the PCM engine */
		VPMCaptureStart(substream);

		break;
		
		case SNDRV_PCM_TRIGGER_STOP:
			/* do something to stop the PCM engine */
		VPMCaptureStop(substream);
		
		default:
		return -EINVAL;
	}	
	return 0;
}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  Function Name: PcmCapturePointer
//
//  Description: Get capture pointer
//
//------------------------------------------------------------
static snd_pcm_uframes_t PcmCapturePointer(struct snd_pcm_substream * substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	snd_pcm_uframes_t pos;
	pos = (g_brcm_alsa_chip->pcm_read_ptr[1]% runtime->buffer_size);
//	DEBUG("\n %lx:PcmCapturePointer pos=%d\n",jiffies,pos);
	return pos;
}


int PcmPlaybackAck(struct snd_pcm_substream *substream)
{
	P_DEVICE_CHANNEL pDev;

	pDev = (P_DEVICE_CHANNEL)substream->runtime->private_data;

	if(pDev->devStatus & 2)
	{
		brcm_alsa_chip_t *chip = snd_pcm_substream_chip(substream);
		queue_work(chip->pWorkqueue_PCM, &chip->work);
		DEBUG("\nschedule additional job\n");
	}

	return 0;
}

//Playback device operator
static struct snd_pcm_ops brcm_alsa_omx_pcm_playback_ops = {
   .open =        PcmPlaybackOpen,
   .close =       PcmPlaybackClose,
   .ioctl =       snd_pcm_lib_ioctl,
   .hw_params =   PcmHwParams,
   .hw_free =     PcmHwFree,
   .prepare =     PcmPlaybackPrepare,
   .trigger =     PcmPlaybackTrigger,
   .pointer =     PcmPlaybackPointer,
   .ack = PcmPlaybackAck,
};

//Capture device operator
static struct snd_pcm_ops brcm_alsa_omx_pcm_capture_ops = {
   .open =        PcmCaptureOpen,
   .close =       PcmCaptureClose,
   .ioctl =       snd_pcm_lib_ioctl,
   .hw_params =   PcmHwParams,
   .hw_free =     PcmHwFree,
   .prepare =     PcmCapturePrepare,
   .trigger =     PcmCaptureTrigger,
   .pointer =     PcmCapturePointer,
};


//+++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  Function Name: worker_pcm
//
//  Description: Playback worker thread to copy data
//
//------------------------------------------------------------
static void worker_pcm(struct work_struct *work)
{
	brcm_alsa_chip_t *pChip =
		container_of(work, brcm_alsa_chip_t, work);
	struct snd_pcm_substream * substream;
	int words;
	P_DEVICE_CHANNEL pDev;
	int err = 0;

	if(pChip==NULL)
	{
		DEBUG("worker_pcm:unlikely to happen pChip==NULL \n");
		return ;
	}
	substream = pChip->substream[0];
	// If the DSP INT arrives when we  have excuted PcmHWFree but yet to execute PcmPlaybackClose we will not have valid dma_area pointer.
	if(substream==NULL || substream->runtime->dma_area == NULL )
		return ;
	snd_pcm_stream_lock_irq(substream);
	if (PCM_RUNTIME_CHECK(substream))
		err = -1; //error sub stream
	else
		{
        pDev = (P_DEVICE_CHANNEL)substream->runtime->private_data;
	words = audvoc_data_transfer(substream);
		}
	snd_pcm_stream_unlock_irq(substream);

	if(err)
		return;
	
//	if((words>0) && (pDev->devStatus & 2))
	if((pDev->devStatus & 2))
	{
		if(words<=0)
		{
			int wlength=256;
			InsertZeroPlayBackSharedmem(substream, wlength); //insert silence to keep it running
//			g_brcm_alsa_chip->pcm_ptr[0] += wlength;
		}
		pDev->devStatus &= ~2;
			post_msg(COMMAND_AUDIO_TASK_START_REQUEST,0,0,0);
//		DEBUG("COMMAND_AUDIO_TASK_START_REQUEST\n");
	}
	

	if( (pChip->pcm_read_ptr[0] - pChip->last_pcm_rdptr[0])>= substream->runtime->period_size)
	{
		snd_pcm_period_elapsed(substream);
		pChip->last_pcm_rdptr[0] = pChip->pcm_read_ptr[0];
		pDev->devStatus &= ~3;
	}
	else
	{
		if(pDev->devStatus & 1) //underrun
			snd_pcm_period_elapsed(substream);	
		
		pDev->devStatus |= 1;
//		DEBUG("\n less than 1 period is transfered %d < %d\n", (int)(pChip->pcm_read_ptr[0] - pChip->last_pcm_rdptr[0]),(int)substream->runtime->period_size);
	}

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  Function Name: PcmDeviceNew
//
//  Description: Create PCM playback and capture device
//
//------------------------------------------------------------
int __devinit PcmDeviceNew(struct snd_card *card)
{
	struct snd_pcm *pcm;
	int err = 0;
	brcm_alsa_chip_t	*pChip;


	err = snd_pcm_new(card, "Broadcom ALSA PCM", 0, NUM_PLAYBACK_SUBDEVICE, NUM_CAPTURE_SUBDEVICE, &pcm);
	if (err<0)
		return err;
    
    pcm->private_data = card->private_data;
	strcpy(pcm->name, "Broadcom ALSA PCM");		
    
	snd_pcm_set_ops(pcm, SNDRV_PCM_STREAM_PLAYBACK, &brcm_alsa_omx_pcm_playback_ops);
	snd_pcm_set_ops(pcm, SNDRV_PCM_STREAM_CAPTURE, &brcm_alsa_omx_pcm_capture_ops);


	pcm->info_flags = 0;
	err=snd_pcm_lib_preallocate_pages_for_all(pcm, SNDRV_DMA_TYPE_CONTINUOUS,
                                     snd_dma_continuous_data(GFP_KERNEL),
                                     PCM_TOTAL_BUF_BYTES, PCM_TOTAL_BUF_BYTES);
	pChip = (brcm_alsa_chip_t *)card->private_data;
	INIT_WORK(&pChip->work, worker_pcm);
	if(err)
		DEBUG("\n Error : PcmDeviceNew err=%d\n",err);
	pChip->pWorkqueue_PCM = create_workqueue("BrcmPcmWq");
	if(!pChip->pWorkqueue_PCM)
		DEBUG("\n Error : Can not create work queue:BrcmPcmWq\n");

	DEBUG("\n PcmDeviceNew : PcmDeviceNew err=%d\n",err);
	return err;
	
}

