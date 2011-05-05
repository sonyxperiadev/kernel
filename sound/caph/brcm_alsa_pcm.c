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

#include "brcm_alsa.h"


#include "mobcom_types.h"
#include "resultcode.h"
#include "audio_consts.h"
#include "auddrv_def.h"

#include "brcm_audio_thread.h"

// Include BRCM AAUD driver API header files
#include "audio_controller.h"
#include "audio_ddriver.h"

#include "brcm_audio_devices.h"


#define VOICE_CALL_SUB_DEVICE 8

TIDChanOfPlaybackDev	sgTableIDChannelOfDev[] = {
	{AUDIO_DRIVER_PLAY_AUDIO, AUDIO_HW_EARPIECE_OUT, AUDCTRL_SPK_HANDSET, AUDDRV_DEV_EP},//earpiece  normal
	{AUDIO_DRIVER_PLAY_RINGER, AUDIO_HW_EARPIECE_OUT, AUDCTRL_SPK_HANDSET, AUDDRV_DEV_EP},//earpiece ringtone
	{AUDIO_DRIVER_PLAY_AUDIO, AUDIO_HW_IHF_OUT, AUDCTRL_SPK_LOUDSPK, AUDDRV_DEV_IHF},//spk normal
	{AUDIO_DRIVER_PLAY_RINGER, AUDIO_HW_IHF_OUT, AUDCTRL_SPK_LOUDSPK, AUDDRV_DEV_IHF},//spk ringtone
	{AUDIO_DRIVER_PLAY_AUDIO, AUDIO_HW_HEADSET_OUT, AUDCTRL_SPK_HEADSET, AUDDRV_DEV_HS},//headset  normal
	{AUDIO_DRIVER_PLAY_RINGER, AUDIO_HW_HEADSET_OUT, AUDCTRL_SPK_HEADSET, AUDDRV_DEV_HS},//headset ringtone
	{AUDIO_DRIVER_PLAY_BT, AUDIO_HW_AUDIO_OUT, AUDCTRL_SPK_BTM, AUDDRV_DEV_BT_SPKR},//bt headset  normal
	{AUDIO_DRIVER_PLAY_BT, AUDIO_HW_AUDIO_OUT, AUDCTRL_SPK_BTM, AUDDRV_DEV_BT_SPKR}//bt headset ringtone
};


TIDChanOfCaptureDev	sgTableIDChannelOfCaptDev[] = {
	{AUDIO_DRIVER_CAPT_HQ, AUDIO_HW_AUDIO_IN, AUDCTRL_MIC_MAIN,AUDDRV_DEV_ANALOG_MIC},
	{AUDIO_DRIVER_CAPT_HQ, AUDIO_HW_AUDIO_IN, AUDCTRL_MIC_AUX,AUDDRV_DEV_HS_MIC},
	{AUDIO_DRIVER_CAPT_HQ, AUDIO_HW_AUDIO_IN, AUDCTRL_MIC_DIGI1,AUDDRV_DEV_DIGI_MIC_L},
	{AUDIO_DRIVER_CAPT_HQ, AUDIO_HW_AUDIO_IN, AUDCTRL_MIC_DIGI2,AUDDRV_DEV_DIGI_MIC_R},
	{AUDIO_DRIVER_CAPT_VOICE, AUDIO_HW_VOICE_IN, AUDCTRL_MIC_MAIN,AUDDRV_DEV_ANALOG_MIC},//mic normal
	{AUDIO_DRIVER_CAPT_VOICE, AUDIO_HW_VOICE_IN, AUDCTRL_MIC_AUX,AUDDRV_DEV_HS_MIC},//mic aux
};

static TIDChanOfVoiceCallDev	sgTableIDChannelOfVoiceCallDev[] = {
	{AUDIO_HW_VOICE_IN, AUDCTRL_MIC_MAIN, AUDIO_HW_VOICE_OUT,AUDCTRL_SPK_LOUDSPK},//incall speaker
    {AUDIO_HW_VOICE_IN, AUDCTRL_MIC_MAIN, AUDIO_HW_VOICE_OUT,AUDCTRL_SPK_HANDSET},//incall earpiece
    {AUDIO_HW_VOICE_IN, AUDCTRL_MIC_AUX, AUDIO_HW_VOICE_OUT,AUDCTRL_SPK_HEADSET},//incall headset
    {AUDIO_HW_VOICE_IN, AUDCTRL_MIC_BTM, AUDIO_HW_VOICE_OUT,AUDCTRL_SPK_BTM},//incall bt
};


int audio_init_complete = 0;
#define	NUM_PLAYBACK_SUBDEVICE	(sizeof(sgTableIDChannelOfDev)/sizeof(sgTableIDChannelOfDev[0]))  + (sizeof(sgTableIDChannelOfVoiceCallDev)/sizeof(sgTableIDChannelOfVoiceCallDev[0]))
#define	NUM_CAPTURE_SUBDEVICE	(sizeof(sgTableIDChannelOfCaptDev)/sizeof(sgTableIDChannelOfCaptDev[0]))  


// limitation for RHEA - only two blocks
#define	PCM_MAX_PLAYBACK_BUF_BYTES		(24*1024)
#define	PCM_MIN_PLAYBACK_PERIOD_BYTES		(12*1024)
#define	PCM_MAX_PLAYBACK_PERIOD_BYTES		(12*1024)

#define	PCM_MAX_CAPTURE_BUF_BYTES       (8 * 1024)
#define	PCM_MIN_CAPTURE_PERIOD_BYTES    (4 * 1024)
#define	PCM_MAX_CAPTURE_PERIOD_BYTES    PCM_MIN_CAPTURE_PERIOD_BYTES

#define	PCM_TOTAL_BUF_BYTES	(PCM_MAX_CAPTURE_BUF_BYTES+PCM_MAX_PLAYBACK_BUF_BYTES)

#define TRUE 1
#define FALSE 0

void AUDIO_DRIVER_InterruptPeriodCB(AUDIO_DRIVER_HANDLE_t drv_handle);
void AUDIO_DRIVER_CaptInterruptPeriodCB(AUDIO_DRIVER_HANDLE_t drv_handle);




/* hardware definition */
static struct snd_pcm_hardware brcm_playback_hw =
{
	.info = (SNDRV_PCM_INFO_MMAP | SNDRV_PCM_INFO_INTERLEAVED |
			SNDRV_PCM_INFO_BLOCK_TRANSFER |	SNDRV_PCM_INFO_MMAP_VALID | SNDRV_PCM_INFO_PAUSE),
	.formats = SNDRV_PCM_FMTBIT_S16_LE,
	.rates = SNDRV_PCM_RATE_8000_48000,
	.rate_min = 8000,
	.rate_max = 48000,
	.channels_min = 1,
	.channels_max = 2,
	.buffer_bytes_max = PCM_MAX_PLAYBACK_BUF_BYTES, //shared memory buffer
	.period_bytes_min = PCM_MIN_PLAYBACK_PERIOD_BYTES,
	.period_bytes_max = PCM_MAX_PLAYBACK_PERIOD_BYTES, //half shared memory buffer
	.periods_min = 2,
	.periods_max = 2,//limitation for RHEA
};


static struct snd_pcm_hardware brcm_capture_hw =
{
	.info = (SNDRV_PCM_INFO_MMAP | SNDRV_PCM_INFO_INTERLEAVED |
		SNDRV_PCM_INFO_BLOCK_TRANSFER |	SNDRV_PCM_INFO_MMAP_VALID),
	.formats = SNDRV_PCM_FMTBIT_S16_LE,
	.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_48000,
	.rate_min = 8000,
	.rate_max = 48000,
	.channels_min = 1,
	.channels_max = 2,
	.buffer_bytes_max = PCM_MAX_CAPTURE_BUF_BYTES,	// one second data
	.period_bytes_min = PCM_MIN_CAPTURE_PERIOD_BYTES, 		// one AMR brocks (each is 4 AMR frames) for pingpong, each blocks is 80 ms, 8000*0.020*2=320
	.period_bytes_max =  PCM_MAX_CAPTURE_PERIOD_BYTES, //half buffer
	.periods_min = 2,
	.periods_max = 2,
};

extern void dump_audio_registers();

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
    int err=0;
//	DEBUG("\n %lx:hw_params %d\n",jiffies,(int)substream->stream);
	
	DEBUG("\t params_access=%d params_format=%d  params_subformat=%d params_channels=%d params_rate=%d, buffer bytes=%d\n",
		 params_access(hw_params), params_format(hw_params), params_subformat(hw_params), params_channels(hw_params), params_rate(hw_params), params_buffer_bytes(hw_params));

#if 0
    err=snd_pcm_lib_preallocate_pages(substream,SNDRV_DMA_TYPE_DEV ,          //SNDRV_DMA_TYPE_CONTINUOUS,
                                     NULL,
                                     PCM_TOTAL_BUF_BYTES, PCM_TOTAL_BUF_BYTES);

    if(err < 0)
    {
        DEBUG("Cannot pre allocate pages \n");
        return err;
    }
#endif

    err = snd_pcm_lib_malloc_pages(substream, params_buffer_bytes(hw_params));

    if(err < 0)
        DEBUG("Cannot allocate pages \n");

	return err; 
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
	DEBUG("\n %lx:hw_free - stream=%x\n",jiffies, substream);
	flush_scheduled_work(); //flush the wait queue in case pending events in the queue are processed after device close
	
	//snd_pcm_stream_lock_irq(substream);
	res = snd_pcm_lib_free_pages(substream);
    //snd_pcm_lib_preallocate_free(substream);
	//snd_pcm_stream_unlock_irq(substream);
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

    AUDIO_DRIVER_HANDLE_t  drv_handle;
	BRCM_AUDIO_Param_Open_t param_open;
	brcm_alsa_chip_t *chip = snd_pcm_substream_chip(substream);
	struct snd_pcm_runtime *runtime = substream->runtime;
	int err=0;
    int substream_number = substream->number ;

    DEBUG("\n %lx:playback_open subdevice=%d PCM_TOTAL_BUF_BYTES=%d\n",jiffies, substream->number, PCM_TOTAL_BUF_BYTES);


    if(audio_init_complete == 0)
    {
        AUDCTRL_Init ();
        audio_init_complete = 1;
    }

    // This device is for voice call so just return
    if(substream->number >= VOICE_CALL_SUB_DEVICE)
    {
//        DEBUG("\nEnable telephony \n");  
 //       AUDCTRL_EnableTelephony(AUDIO_HW_VOICE_IN,AUDIO_HW_VOICE_OUT,sgTableIDChannelOfVoiceCallDev[substream->number-VOICE_CALL_SUB_DEVICE].mic,sgTableIDChannelOfVoiceCallDev[substream->number-VOICE_CALL_SUB_DEVICE].speaker);
        
        //runtime->hw = brcm_playback_hw;	
        //chip->substream[0] = substream;
        //return err;

        substream_number = (substream->number - VOICE_CALL_SUB_DEVICE) * 2;
    }
		
	runtime->hw = brcm_playback_hw;	
	chip->substream[0] = substream;

	param_open.drv_handle = NULL;
	param_open.substream_number = substream_number;
    //open the playback device
	AUDIO_Ctrl_Trigger(ACTION_AUD_OpenPlay,&param_open,NULL,1); 
	drv_handle = param_open.drv_handle;
	if(drv_handle == NULL)
   	{
    	DEBUG("\n %lx:playback_open subdevice=%d failed\n",jiffies, substream_number);
	    return -1;
	}

    substream->runtime->private_data = drv_handle;

    DEBUG("chip-0x%x substream-0x%x drv_handle-0x%x \n",chip,substream,drv_handle);


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
	BRCM_AUDIO_Param_Close_t param_close;

	brcm_alsa_chip_t *chip = snd_pcm_substream_chip(substream);

    
	DEBUG("\n %lx:playback_close subdevice=%d\n",jiffies, substream->number);

    // This device is for voice call so stop the telephony path
    if(substream->number >= VOICE_CALL_SUB_DEVICE)
    {
        DEBUG("\nDisable telephony \n");
        AUDCTRL_DisableTelephony(AUDIO_HW_VOICE_IN,AUDIO_HW_VOICE_OUT,sgTableIDChannelOfVoiceCallDev[substream->number-VOICE_CALL_SUB_DEVICE].mic,sgTableIDChannelOfVoiceCallDev[substream->number-VOICE_CALL_SUB_DEVICE].speaker);
        
        //substream->runtime->private_data = NULL;
	    //chip->substream[0] = NULL;
        //return 0;
    }
    
    param_close.drv_handle = substream->runtime->private_data;

    //close the driver
	AUDIO_Ctrl_Trigger(ACTION_AUD_ClosePlay,&param_close,NULL,1);
   
    substream->runtime->private_data = NULL;

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
    AUDIO_DRIVER_HANDLE_t  drv_handle;
    unsigned long period_bytes;
    AUDIO_DRIVER_BUFFER_t buf_param;
    AUDIO_DRIVER_CONFIG_t drv_config;


	DEBUG("\nplayback_prepare period=%d period_size=%d bufsize=%d threshold=%d\n", runtime->periods, 
			frames_to_bytes(runtime, runtime->period_size), frames_to_bytes(runtime, runtime->buffer_size), runtime->stop_threshold);

    // This device is for voice call so open
    if(substream->number >= VOICE_CALL_SUB_DEVICE)
    {
        DEBUG("\nEnable telephony \n");  
        AUDCTRL_SaveAudioModeFlag( sgTableIDChannelOfVoiceCallDev[substream->number-VOICE_CALL_SUB_DEVICE].speaker );
        AUDCTRL_EnableTelephony(AUDIO_HW_VOICE_IN,AUDIO_HW_VOICE_OUT,sgTableIDChannelOfVoiceCallDev[substream->number-VOICE_CALL_SUB_DEVICE].mic,sgTableIDChannelOfVoiceCallDev[substream->number-VOICE_CALL_SUB_DEVICE].speaker);
        //return 0;
    }

	chip->rate[0]         = runtime->rate;
	chip->buffer_bytes[0] = runtime->dma_bytes;
	chip->period_bytes[0] = frames_to_bytes(runtime, runtime->period_size);
	chip->pcm_ptr[0] = 0;
	chip->pcm_write_ptr[0] = 0;
	chip->pcm_read_ptr[0]  = 0;
	chip->last_pcm_rdptr[0] = 0;

    drv_handle = substream->runtime->private_data;


    //set the callback
    AUDIO_DRIVER_Ctrl(drv_handle,AUDIO_DRIVER_SET_CB,(void*)AUDIO_DRIVER_InterruptPeriodCB);

    //set the interrupt period
    period_bytes = frames_to_bytes(runtime, runtime->period_size);
    AUDIO_DRIVER_Ctrl(drv_handle,AUDIO_DRIVER_SET_INT_PERIOD,(void*)&period_bytes);
    
    //set the buffer params
    buf_param.buf_size = runtime->dma_bytes;
    buf_param.pBuf = runtime->dma_area;// virtual address
    buf_param.phy_addr = (UInt32)(runtime->dma_addr);// physical address
    
    DEBUG("buf_size = %d pBuf=0x%x phy_addr=0x%x \n",runtime->dma_bytes,runtime->dma_area,runtime->dma_addr);

    AUDIO_DRIVER_Ctrl(drv_handle,AUDIO_DRIVER_SET_BUF_PARAMS,(void*)&buf_param);

    //Configure stream params  **** CAUTION:Check the mappng here
    drv_config.sample_rate = runtime->rate;
	drv_config.num_channel = runtime->channels;
    drv_config.bits_per_sample = AUDIO_16_BIT_PER_SAMPLE;
    AUDIO_DRIVER_Ctrl(drv_handle,AUDIO_DRIVER_CONFIG,(void*)&drv_config);

//	DEBUG("\n%lx:playback_prepare period bytes=%d, periods =%d, buffersize=%d\n",jiffies, g_brcm_alsa_chip->period_bytes[0], runtime->periods, runtime->dma_bytes);
	return 0;
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
    AUDIO_DRIVER_HANDLE_t  drv_handle;

    int substream_number = substream->number ;

    drv_handle = substream->runtime->private_data;



//	DEBUG("\n ALSA : PcmPlaybackTrigger \n");
	DEBUG("\n %lx:playback_trigger cmd=%d untime->boundary=%d\n",jiffies,cmd, (int)substream->runtime->boundary );

    if(substream->number >= VOICE_CALL_SUB_DEVICE)
    {
        
        substream_number = (substream->number - VOICE_CALL_SUB_DEVICE) * 2;

        DEBUG("\nVoice call - Nothing to do substream_number - %d\n",substream_number);  
    }

	switch (cmd) 
	{
		case SNDRV_PCM_TRIGGER_START:
			/* do something to start the PCM engine */
			{
                BRCM_AUDIO_Param_Start_t param_start;
                
				struct snd_pcm_runtime *runtime = substream->runtime;

                param_start.drv_handle = drv_handle;
                param_start.substream_number = substream_number;
                param_start.channels = runtime->channels;
                param_start.rate = runtime->rate;

                AUDIO_Ctrl_Trigger(ACTION_AUD_StartPlay,&param_start,NULL,0);

                

			}
			break;
		
		case SNDRV_PCM_TRIGGER_STOP:
            {
                BRCM_AUDIO_Param_Stop_t param_stop;

                param_stop.drv_handle = drv_handle;
                param_stop.substream_number = substream_number;

                AUDIO_Ctrl_Trigger(ACTION_AUD_StopPlay,&param_stop,NULL,0);

                
            }

			break;
        case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
            {
                //AUDCTRL_SaveAudioModeFlag( sgTableIDChannelOfDev[substream->number].speaker );
                BRCM_AUDIO_Param_Pause_t param_pause;

                param_pause.drv_handle = drv_handle;
                param_pause.substream_number = substream_number;

                AUDIO_Ctrl_Trigger(ACTION_AUD_PausePlay,&param_pause,NULL,0);
			    
	        }
			break;
    	
        case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
            {
                //AUDCTRL_SaveAudioModeFlag( sgTableIDChannelOfDev[substream->number].speaker );
    			BRCM_AUDIO_Param_Resume_t param_resume;
                
				struct snd_pcm_runtime *runtime = substream->runtime;

                param_resume.drv_handle = drv_handle;
                param_resume.substream_number = substream_number;
                param_resume.channels = runtime->channels;
                param_resume.rate = runtime->rate;

                AUDIO_Ctrl_Trigger(ACTION_AUD_ResumePlay,&param_resume,NULL,0);

            }
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
    snd_pcm_uframes_t pos;

	pos = g_brcm_alsa_chip->pcm_read_ptr[0] + g_brcm_alsa_chip->pcm_ptr[0];
	if(pos<0)
	{
		pos += runtime->buffer_size;
	}
	pos %= runtime->buffer_size;
//	DEBUG("PcmPlaybackPointer: hw_ptr=%d pcm_read_ptr=%d words appl_ptr=%d validPcmSize=%d buffersize=%d\n", runtime->status->hw_ptr, g_brcm_alsa_chip->pcm_read_ptr[0], (int)runtime->control->appl_ptr,
//				runtime->control->appl_ptr-g_brcm_alsa_chip->pcm_read_ptr[0],  runtime->buffer_size);

	return pos;
}


static unsigned int sgpcm_capture_period_bytes[]={PCM_MIN_CAPTURE_PERIOD_BYTES, PCM_MIN_CAPTURE_PERIOD_BYTES*2, PCM_MIN_CAPTURE_PERIOD_BYTES*3, PCM_MIN_CAPTURE_PERIOD_BYTES*4};  //bytes
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
	AUDIO_DRIVER_HANDLE_t  drv_handle;
	BRCM_AUDIO_Param_Open_t param_open;
    brcm_alsa_chip_t *chip = snd_pcm_substream_chip(substream);
	struct snd_pcm_runtime *runtime = substream->runtime;
	int err=0;

	DEBUG("\n ALSA : PcmCaptureOpen \n");

    if(audio_init_complete == 0)
    {
        AUDCTRL_Init ();
        audio_init_complete = 1;
    }
	
	runtime->hw = brcm_capture_hw;	
	chip->substream[1] = substream;

    //open the capture device
	param_open.drv_handle = NULL;
	param_open.substream_number = substream->number;
	
	AUDIO_Ctrl_Trigger(ACTION_AUD_OpenRecord,&param_open,NULL,1); 
	
	drv_handle = param_open.drv_handle;

    if(drv_handle == NULL)
    {
        DEBUG("\n %lx:capture_open subdevice=%d failed\n",jiffies, substream->number);
        return -1;
    }

    substream->runtime->private_data = drv_handle;

											
	//err = snd_pcm_hw_constraint_list(runtime,0,SNDRV_PCM_HW_PARAM_PERIOD_BYTES,
										//&sgpcm_capture_period_bytes_constraints_list);
										
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
	BRCM_AUDIO_Param_Close_t param_close;
    brcm_alsa_chip_t *chip = snd_pcm_substream_chip(substream);
	
    param_close.drv_handle = substream->runtime->private_data;
    //close the driver
	AUDIO_Ctrl_Trigger(ACTION_AUD_CloseRecord,&param_close,NULL,1);

    substream->runtime->private_data = NULL;
    
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
    brcm_alsa_chip_t *chip = snd_pcm_substream_chip(substream);
    AUDIO_DRIVER_HANDLE_t  drv_handle;
    unsigned long period_bytes;
    AUDIO_DRIVER_BUFFER_t buf_param;
    AUDIO_DRIVER_CONFIG_t drv_config;


	DEBUG("\n %lx:capture_prepare: subdevice=%d rate =%d format =%d channel=%d dma_area=0x%x dma_bytes=%d period_bytes=%d avail_min=%d periods=%d buffer_size=%d\n",
		         jiffies,	substream->number, runtime->rate, runtime->format, runtime->channels, (unsigned int)runtime->dma_area, runtime->dma_bytes,
		         frames_to_bytes(runtime, runtime->period_size), frames_to_bytes(runtime, runtime->control->avail_min), runtime->periods, runtime->buffer_size);
	
	g_brcm_alsa_chip->rate[1]         = runtime->rate;
	g_brcm_alsa_chip->buffer_bytes[1] = runtime->dma_bytes;
	g_brcm_alsa_chip->period_bytes[1] = frames_to_bytes(runtime, runtime->period_size);
	g_brcm_alsa_chip->pcm_read_ptr[1]  = 0; 
	g_brcm_alsa_chip->last_pcm_rdptr[1] = 0;


    drv_handle = substream->runtime->private_data;

    //set the callback
    AUDIO_DRIVER_Ctrl(drv_handle,AUDIO_DRIVER_SET_CB,(void*)AUDIO_DRIVER_CaptInterruptPeriodCB);

    //set the interrupt period
    period_bytes = frames_to_bytes(runtime, runtime->period_size);
    AUDIO_DRIVER_Ctrl(drv_handle,AUDIO_DRIVER_SET_INT_PERIOD,(void*)&period_bytes);
    
    //set the buffer params
    buf_param.buf_size = runtime->dma_bytes;
    buf_param.pBuf = runtime->dma_area;// virtual address
    buf_param.phy_addr = (UInt32)(runtime->dma_addr);// physical address
    
    DEBUG("buf_size = %d pBuf=0x%x phy_addr=0x%x \n",runtime->dma_bytes,runtime->dma_area,runtime->dma_addr);

    AUDIO_DRIVER_Ctrl(drv_handle,AUDIO_DRIVER_SET_BUF_PARAMS,(void*)&buf_param);

    //Configure stream params  **** CAUTION:Check the mappng here
    drv_config.sample_rate = runtime->rate;
    drv_config.num_channel = runtime->channels;
    drv_config.bits_per_sample = AUDIO_16_BIT_PER_SAMPLE;
    AUDIO_DRIVER_Ctrl(drv_handle,AUDIO_DRIVER_CONFIG,(void*)&drv_config);

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
    int ret = 0;
	brcm_alsa_chip_t *chip = snd_pcm_substream_chip(substream);
    AUDIO_DRIVER_HANDLE_t  drv_handle;

    drv_handle = substream->runtime->private_data;
	DEBUG("\n %lx:capture_trigger subdevice=%d cmd=%d\n",jiffies,substream->number, cmd);
  
	switch (cmd) 
	{
		case SNDRV_PCM_TRIGGER_START:
            {
                BRCM_AUDIO_Param_Start_t param_start;
                
				struct snd_pcm_runtime *runtime = substream->runtime;

                param_start.drv_handle = drv_handle;
                param_start.substream_number = substream->number;
                param_start.channels = runtime->channels;
                param_start.rate = runtime->rate;

                AUDIO_Ctrl_Trigger(ACTION_AUD_StartRecord,&param_start,NULL,0);
                
            }
		break;
		
		case SNDRV_PCM_TRIGGER_STOP:
            {
                BRCM_AUDIO_Param_Stop_t param_stop;

                param_stop.drv_handle = drv_handle;
                param_stop.substream_number = substream->number;

                AUDIO_Ctrl_Trigger(ACTION_AUD_StopRecord,&param_stop,NULL,0); 

		
            }
            break;
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
	//DEBUG("%lx:PcmCapturePointer pos=%d pcm_read_ptr=%d, buffer size = %d,\n",jiffies,pos,g_brcm_alsa_chip->pcm_read_ptr[1],runtime->buffer_size);
	return pos;
}


int PcmPlaybackAck(struct snd_pcm_substream *substream)
{

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
//  Function Name: worker_pcm_play
//
//  Description: Playback worker thread to copy data
//
//------------------------------------------------------------
static void worker_pcm_play(struct work_struct *work)
{
	//brcm_alsa_chip_t *pChip =
	//	container_of(work, brcm_alsa_chip_t, work_play);
	brcm_alsa_chip_t *pChip = g_brcm_alsa_chip;

    struct snd_pcm_substream * substream;
    struct snd_pcm_runtime *runtime;
	int words;
    AUDIO_DRIVER_HANDLE_t  drv_handle;
    AUDIO_DRIVER_TYPE_t    drv_type;


	if(pChip==NULL)
	{
		DEBUG("worker_pcm:unlikely to happen pChip==NULL \n");
		return ;
	}

    
	substream = pChip->substream[0];
	if(substream==NULL)
		return ;

    runtime = substream->runtime;

    drv_handle = substream->runtime->private_data;

    //DEBUG("\n %lx:worker_pcm_play \n",jiffies);

    AUDIO_DRIVER_Ctrl(drv_handle,AUDIO_DRIVER_GET_DRV_TYPE,(void*)&drv_type);

      
    switch (drv_type)
    {
        case AUDIO_DRIVER_PLAY_VOICE:
        case AUDIO_DRIVER_PLAY_AUDIO:
        case AUDIO_DRIVER_PLAY_RINGER:
            {
                //update the PCM read pointer by period size
                pChip->pcm_read_ptr[0] += runtime->period_size;
                if(pChip->pcm_read_ptr[0]>runtime->boundary)
                    pChip->pcm_read_ptr[0] -= runtime->boundary;
                // send the period elapsed
	            snd_pcm_period_elapsed(substream);
		        pChip->last_pcm_rdptr[0] = pChip->pcm_read_ptr[0];
            }
            break;
        default:
            DEBUG("Invalid driver type\n");
            break;
    }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  Function Name: worker_pcm_capt
//
//  Description: Playback worker thread to copy data
//
//------------------------------------------------------------
static void worker_pcm_capt(struct work_struct *work)
{
	//brcm_alsa_chip_t *pChip =
	//	container_of(work, brcm_alsa_chip_t, work_play);
	brcm_alsa_chip_t *pChip = g_brcm_alsa_chip;

    struct snd_pcm_substream * substream;
    struct snd_pcm_runtime *runtime;
	int words;
    AUDIO_DRIVER_HANDLE_t  drv_handle;
    AUDIO_DRIVER_TYPE_t    drv_type;


	if(pChip==NULL)
	{
		DEBUG("worker_pcm_capt:unlikely to happen pChip==NULL \n");
		return ;
	}

    
	substream = pChip->substream[1];
	if(substream==NULL)
		return ;

    runtime = substream->runtime;

    drv_handle = substream->runtime->private_data;

    //DEBUG("pChip-0x%x substream-0x%x drv_handle-0x%x \n",pChip,substream,drv_handle);

    DEBUG("\n %lx:worker_pcm_capt \n",jiffies);

    AUDIO_DRIVER_Ctrl(drv_handle,AUDIO_DRIVER_GET_DRV_TYPE,(void*)&drv_type);

    switch (drv_type)
    {
        case AUDIO_DRIVER_CAPT_HQ:
        case AUDIO_DRIVER_CAPT_VOICE:
            {
                //update the PCM read pointer by period size
                pChip->pcm_read_ptr[1] += runtime->period_size;
                if(pChip->pcm_read_ptr[1]>runtime->boundary)
                    pChip->pcm_read_ptr[1] -= runtime->boundary;
                // send the period elapsed
	            snd_pcm_period_elapsed(substream);
		        pChip->last_pcm_rdptr[1] = pChip->pcm_read_ptr[1];
            }
            break;
        default:
            DEBUG("Invalid driver type\n");
            break;
    }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  Function Name: AUDIO_DRIVER_InterruptPeriodCB
//
//  Description: Playback inteerupt
//
//------------------------------------------------------------
void AUDIO_DRIVER_InterruptPeriodCB(AUDIO_DRIVER_HANDLE_t drv_handle)
{
	struct snd_pcm_substream * substream = g_brcm_alsa_chip->substream[0];
    AUDIO_DRIVER_HANDLE_t  drv_handle_local;
    AUDIO_DRIVER_TYPE_t    drv_type;

    drv_handle_local = substream->runtime->private_data;
	
    if(!substream)
		return;

    if(drv_handle_local != drv_handle)
    {
        DEBUG("Invalid Handle\n");
        return;
    }
    // schedule the PCM playback worker thread
    schedule_work(&g_brcm_alsa_chip->work_play);
    return;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  Function Name: AUDIO_DRIVER_CaptInterruptPeriodCB
//
//  Description: Capture inteerupt
//
//------------------------------------------------------------
void AUDIO_DRIVER_CaptInterruptPeriodCB(AUDIO_DRIVER_HANDLE_t drv_handle)
{
	struct snd_pcm_substream * substream = g_brcm_alsa_chip->substream[1];
    AUDIO_DRIVER_HANDLE_t  drv_handle_local;
    AUDIO_DRIVER_TYPE_t    drv_type;

    drv_handle_local = substream->runtime->private_data;
	
    if(!substream)
		return;

    if(drv_handle_local != drv_handle)
    {
        DEBUG("Invalid Handle\n");
        return;
    }
    // schedule the PCM playback worker thread
    schedule_work(&g_brcm_alsa_chip->work_capt);
    return;
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

#if 0
	err=snd_pcm_lib_preallocate_pages_for_all(pcm,SNDRV_DMA_TYPE_CONTINUOUS ,          //SNDRV_DMA_TYPE_DEV,
                                     snd_dma_continuous_data(GFP_KERNEL),
                                     PCM_TOTAL_BUF_BYTES, PCM_TOTAL_BUF_BYTES);
#endif
	err=snd_pcm_lib_preallocate_pages_for_all(pcm,SNDRV_DMA_TYPE_DEV  ,          //SNDRV_DMA_TYPE_CONTINUOUS,
                                     NULL,
                                     PCM_TOTAL_BUF_BYTES, PCM_TOTAL_BUF_BYTES);

	pChip = (brcm_alsa_chip_t *)card->private_data;
	INIT_WORK(&pChip->work_play, worker_pcm_play);
    INIT_WORK(&pChip->work_capt, worker_pcm_capt);
    
	if(err)
		DEBUG("\n Error : PcmDeviceNew err=%d\n",err);

    // Initialize the audio controller
    if(audio_init_complete == 0)
    {
		DEBUG("\n PcmDeviceNew : call AUDCTRL_Init\n");
        AUDCTRL_Init ();
        audio_init_complete = 1;
    }

	DEBUG("\n PcmDeviceNew : PcmDeviceNew err=%d\n",err);
	return err;
	
}

