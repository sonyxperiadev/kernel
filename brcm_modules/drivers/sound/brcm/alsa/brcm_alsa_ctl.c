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

#include <linux/sysfs.h>

#include "hal_audio.h"
#include "hal_audio_access.h"

#include <linux/broadcom/hw_cfg.h>

#include "brcm_alsa.h"

ssize_t FMDirPlaySysfs_show(struct device *dev, struct device_attribute *attr, char *buf);
ssize_t FMDirPlaySysfs_store(struct device *dev, struct device_attribute *attr,const char *buf, size_t count);
static struct device_attribute FMDirectplay_attrib = __ATTR(FMDirectPlay, 0644, FMDirPlaySysfs_show, FMDirPlaySysfs_store);


void AUDIO_DRV_SetVoiceVolume( UInt8 Volume);

HAL_AUDIO_Param_Tone_t	audioToneControlParam;


//Control item name
static char gpstrCtrlName[BRCM_CTL_TOTAL][40] =
{
	"Speaker Playback Volume",
	"Speaker Playback Mute",
	"Mic Capture Volume", 
	"Mic Capture Mute",
	"Ringtone SPK Playback Volume",
	"Ringtone SPK Playback Mute", 
	"Incall SPK Playback Volume", 
	"Incall SPK Playback Mute" ,

	"Earpiece Volume",
	"Earpiece Mute",
	"Ringtone Earpiece Volume",
	"Ringtone Earpiece Mute", 
	"Incall Earpiece Volume", 
	"Incall Earpiece Mute", 

	"Headset Volume",
	"Headset Mute",
	"Ringtone Headset Volume",
	"Ringtone Headset Mute", 
	"Incall Headset Volume", 
	"Incall Headset Mute" ,
        "BTHeadset Volume",
	"BTHeadset Mute",
	"BTHeadset Ringtone  Volume",
	"BTHeadset Ringtone Mute",
	"BTHeadset Incall Volume",
	"BTHeadset Incall Mute",
	"Incall DMTF Tone",
	"FMDirectPlayCtrl",
	"PlaybackPosition"
};

static long			sgBrcmCtrlVal[BRCM_CTL_TOTAL];
#define	FMDirectPlayCount	5
static	int	sgFMDirectPlayValues[FMDirectPlayCount];
static char sgFMDirectPlayAttrName[FMDirectPlayCount][20]={
		"Enabled", "Output device", "sample rate index", "channel", "volume"
	};


snd_pcm_uframes_t GetPlaybackRenderPosition(struct snd_pcm_substream * substream);


//*****************************************************************
// Functiona Name: GetCtrlValue
//
// Description: Get control value by index
//
//*****************************************************************
long GetCtrlValue(int ctrlIndex)
{
	long val = 0;
	if(ctrlIndex>=0 && ctrlIndex<BRCM_CTL_TOTAL)
		val = sgBrcmCtrlVal[ctrlIndex];
	else
		DEBUG("GetCtrlValue: Invalid index=%d", ctrlIndex);
	return val;
}


//*****************************************************************
// Functiona Name: GetCtrlInfo
//
// Description: Get control item properties: range, channels, step, etc
//
//*****************************************************************
static int GetCtrlInfo(
	struct snd_kcontrol * kcontrol,
	struct snd_ctl_elem_info * uinfo
)
{
//	DEBUG("\n%lx:GetCtrlInfo ctrl->private_value=%d",jiffies, kcontrol->private_value);

	switch(kcontrol->private_value)
	{
		case BRCM_CTL_Speaker_Playback_Volume:
		case BRCM_CTL_Ringtone_SPK_Playback_Volume:
		case BRCM_CTL_Ringtone_EAR_Playback_Volume:
		case BRCM_CTL_HEADSET_Playback_Volume:
		case BRCM_CTL_Ringtone_HEADSET_Playback_Volume:
			uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
			uinfo->count			 = 2;
			uinfo->value.integer.min = 0;
			uinfo->value.integer.max = AUDIO_MAX_OUTPUT_VOLUME;
			uinfo->value.integer.step = uinfo->value.integer.max/10;
			break;

		case BRCM_CTL_EAR_Playback_Volume:
		case BRCM_CTL_Incall_SPK_Playback_Volume:
		case BRCM_CTL_Incall_EAR_Playback_Volume:
		case BRCM_CTL_Incall_HEADSET_Playback_Volume:
		case BRCM_CTL_Mic_Capture_Volume:
			
		uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
		uinfo->count			 = 1;
		uinfo->value.integer.min = 0;
		uinfo->value.integer.max = AUDIO_MAX_OUTPUT_VOLUME;
		uinfo->value.integer.step = 1;

		break;


		case BRCM_CTL_Speaker_Playback_Mute:
		case BRCM_CTL_Ringtone_SPK_Playback_Mute:
		case BRCM_CTL_Ringtone_EAR_Playback_Mute:
		case BRCM_CTL_HEADSET_Playback_Mute:
		case BRCM_CTL_Ringtone_HEADSET_Playback_Mute:
			
			uinfo->type			     = SNDRV_CTL_ELEM_TYPE_BOOLEAN;
			uinfo->count			 = 2;
			uinfo->value.integer.min = 0;
			uinfo->value.integer.max = 1;		
			break;
		case BRCM_CTL_EAR_Playback_Mute:
		case BRCM_CTL_Incall_SPK_Playback_Mute:
		case BRCM_CTL_Mic_Capture_Mute:
		case BRCM_CTL_Incall_EAR_Playback_Mute:
		case BRCM_CTL_Incall_HEADSET_Playback_Mute:
		uinfo->type			     = SNDRV_CTL_ELEM_TYPE_BOOLEAN;
		uinfo->count			 = 1;
		uinfo->value.integer.min = 0;
		uinfo->value.integer.max = 1;		
			break;

		case BRCM_CTL_BTHeadset_Volume:
		case BRCM_CTL_BTHeadset_Ringtone_Volume:
		case BRCM_CTL_BTHeadset_Incall_Volume:
			//do something for BT volume
		uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
		uinfo->count			 = 1;
		uinfo->value.integer.min = 0;
		uinfo->value.integer.max = AUDIO_MAX_OUTPUT_VOLUME;
		uinfo->value.integer.step = 1;
					
			break;
		case BRCM_CTL_BTHeadset_Mute:
		case BRCM_CTL_BTHeadset_Ringtone_Mute:
		case BRCM_CTL_BTHeadset_Incall_Mute:
			//do something for BT mute
		uinfo->type			     = SNDRV_CTL_ELEM_TYPE_BOOLEAN;
		uinfo->count			 = 1;
		uinfo->value.integer.min = 0;
		uinfo->value.integer.max = 1;		
			
			break;
			
		case BRCM_CTL_DTMF_Tone_Start:
			
		uinfo->type			     = SNDRV_CTL_ELEM_TYPE_INTEGER;
		uinfo->count			 = 2;
		uinfo->value.integer.min = 0;
		uinfo->value.integer.max = SPEAKERTONE_STAR;	
			break;
			
		case BRCM_CTL_FMDirectPlayCtrl:
		uinfo->type			     = SNDRV_CTL_ELEM_TYPE_INTEGER;
		uinfo->count			 = sizeof(sgFMDirectPlayValues)/sizeof(sgFMDirectPlayValues[0]);	// 0 - on/off, 1 - op mode, 2 - sample rate index, 3 - channel, 4 -volume, 
		uinfo->value.integer.min = 0;
		uinfo->value.integer.max = AUDIO_MAX_OUTPUT_VOLUME;	
		uinfo->value.integer.step = 1;
			break;
		case BRCM_CTL_Get_PlaybackPosition:
		uinfo->type 			 = SNDRV_CTL_ELEM_TYPE_INTEGER;
		uinfo->count			 = 1;
		uinfo->value.integer.min = 0;
		uinfo->value.integer.max = 0x7FFFFFFF;
		uinfo->value.integer.step = 1;
			break;
			
		default:
			DEBUG("\n%lx:Unknown GetCtrlInfo ctrl->private_value=%d",jiffies, (int)kcontrol->private_value);

	}

	
	return 0;

}


//*****************************************************************
// Functiona Name: GetControlItem
//
// Description: Get control item properties: range, channels, step, etc
//
//*****************************************************************
static int GetControlItem(
	struct snd_kcontrol * kcontrol,
	struct snd_ctl_elem_value * ucontrol
)
{
	int i=0;
	//DEBUG("\n%lx:GetControlItem",jiffies);
//	DEBUG("\n%lx:GetControlItem ctrl->private_value=%d",jiffies, kcontrol->private_value);


	if(kcontrol->private_value>=0 && kcontrol->private_value<BRCM_CTL_TOTAL)
		ucontrol->value.integer.value[0] = sgBrcmCtrlVal[kcontrol->private_value];

	if(kcontrol->private_value == BRCM_CTL_FMDirectPlayCtrl)
		for(i=0; i< sizeof(sgFMDirectPlayValues)/sizeof(sgFMDirectPlayValues[0]); i++)
			ucontrol->value.integer.value[i] = sgFMDirectPlayValues[i];
	
	if(kcontrol->private_value == BRCM_CTL_Get_PlaybackPosition)
	{
		ucontrol->value.integer.value[0] = GetPlaybackRenderPosition(g_brcm_alsa_chip->substream[0]);
	}
		
	return 0;
}

//+++++++++++++++++++++++++++++++++++++++
//FMDirPlaySysfs_show (struct device *dev, struct device_attribute *attr, char *buf)
// Buffer values syntax -	 0 - on/off, 1 - output device, 2 - sample rate index, 3 - channel, 4 -volume, 
//
//---------------------------------------------------

ssize_t FMDirPlaySysfs_show(struct device *dev, struct device_attribute *attr, char *buf)
{
  	int i;
	char sbuf[256];
	for(i=0; i< sizeof(sgFMDirectPlayValues)/sizeof(sgFMDirectPlayValues[0]); i++)
	{
		snprintf(sbuf, sizeof(sbuf), "%d ",sgFMDirectPlayValues[i]); 
		strcat(buf, sbuf);
	}
	return strlen(buf);
}

//+++++++++++++++++++++++++++++++++++++++
// FMDirPlaySysfs_store (struct device *dev, struct device_attribute *attr, char *buf)
// Buffer values syntax -	 0 - on/off, 1 - output device, 2 - sample rate index, 3 - channel, 4 -volume, 
//
//---------------------------------------------------

ssize_t FMDirPlaySysfs_store(struct device *dev, struct device_attribute *attr,const char *buf, size_t count)
{
   	int i=0;
	HAL_AUDIO_Param_CodecCfg_t codecCfg;
	HAL_AUDIO_Param_PathCfg_t pathCfg;
	
	if(5!= sscanf(buf, "%d %d %d %d %d",&sgFMDirectPlayValues[0], &sgFMDirectPlayValues[1], &sgFMDirectPlayValues[2], &sgFMDirectPlayValues[3], &sgFMDirectPlayValues[4]))
	{
	
		DEBUG("\n<-FMDirectPlay SysFS Handler: On_Off=%d OpMode=%d, samplerate=%d, channel=%d, volume=%d \n",  sgFMDirectPlayValues[0],
											sgFMDirectPlayValues[1],
											sgFMDirectPlayValues[2],
											sgFMDirectPlayValues[3],
											sgFMDirectPlayValues[4]);
		DEBUG("error reading buf=%s count=%d\n", buf, count);
		return count;
	}

	if(sgFMDirectPlayValues[0]) 		// enable path
	{
		codecCfg.audioID = AUDIO_ID_IIS2DAC;
		codecCfg.sampleRate = sgFMDirectPlayValues[2];
		codecCfg.monoStereo = sgFMDirectPlayValues[3];
		HAL_AUDIO_Ctrl( ACTION_AUD_ConfigCodecParam, &codecCfg, NULL );

		pathCfg.audioID = AUDIO_ID_IIS2DAC;
		pathCfg.outputChnl = sgFMDirectPlayValues[1];
		pathCfg.volumeLevel = sgFMDirectPlayValues[4];
		HAL_AUDIO_Ctrl( ACTION_AUD_EnablePath, &pathCfg, NULL );

		pathCfg.audioID = AUDIO_ID_IIS2DAC;
		pathCfg.outputChnl = sgFMDirectPlayValues[1];
		pathCfg.volumeLevel = sgFMDirectPlayValues[4];
		HAL_AUDIO_Ctrl( ACTION_AUD_SetVolumeWithPath, &pathCfg, NULL );
	}
	else
	{
		pathCfg.audioID = AUDIO_ID_IIS2DAC;
		pathCfg.outputChnl = sgFMDirectPlayValues[1];
		HAL_AUDIO_Ctrl( ACTION_AUD_DisablePath, &pathCfg, NULL );
	}
	return count;
     
}


//+++++++++++++++++++++++++++++++++++++++
//FMDirectPlayHandler(struct snd_ctl_elem_value *ucontrol)
// 	values 	 0 - on/off, 1 - output device, 2 - sample rate index, 3 - channel, 4 -volume, 
//
// 	Only when 
//
//---------------------------------------------------
int	FMDirectPlayHandler(struct snd_kcontrol * kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	int i=0;
	HAL_AUDIO_Param_CodecCfg_t codecCfg;
	HAL_AUDIO_Param_PathCfg_t		pathCfg;


	for(i=0; i< sizeof(sgFMDirectPlayValues)/sizeof(sgFMDirectPlayValues[0]); i++)
		sgFMDirectPlayValues[i] = ucontrol->value.integer.value[i];

	DEBUG("\n<-FMDirectPlayHandler: On_Off=%d OpMode=%d, samplerate=%d, channel=%d, volume=%d \n",  sgFMDirectPlayValues[0],
											sgFMDirectPlayValues[1],
											sgFMDirectPlayValues[2],
											sgFMDirectPlayValues[3],
											sgFMDirectPlayValues[4]);

	if(sgFMDirectPlayValues[0]) 		// enable path
	{
		codecCfg.audioID = AUDIO_ID_IIS2DAC;
		codecCfg.sampleRate = sgFMDirectPlayValues[2];
		codecCfg.monoStereo = sgFMDirectPlayValues[3];
		HAL_AUDIO_Ctrl( ACTION_AUD_ConfigCodecParam, &codecCfg, NULL );

		pathCfg.audioID = AUDIO_ID_IIS2DAC;
		pathCfg.outputChnl = sgFMDirectPlayValues[1];
		pathCfg.volumeLevel = sgFMDirectPlayValues[4];
		HAL_AUDIO_Ctrl( ACTION_AUD_EnablePath, &pathCfg, NULL );

		pathCfg.audioID = AUDIO_ID_IIS2DAC;
		pathCfg.outputChnl = sgFMDirectPlayValues[1];
		pathCfg.volumeLevel = sgFMDirectPlayValues[4];
		HAL_AUDIO_Ctrl( ACTION_AUD_SetVolumeWithPath, &pathCfg, NULL );
	}
	else
	{
		pathCfg.audioID = AUDIO_ID_IIS2DAC;
		pathCfg.outputChnl = sgFMDirectPlayValues[1];
		HAL_AUDIO_Ctrl( ACTION_AUD_DisablePath, &pathCfg, NULL );
	}

	return 0;
}


//*****************************************************************
// Functiona Name: SetControlItem
//
// Description: Set control item, will set hardware if value changed.
//
//*****************************************************************
static int SetControlItem(
	struct snd_kcontrol * kcontrol,
	struct snd_ctl_elem_value * ucontrol
)
{
	int changed = 0;

	//DEBUG("\n%lx:SetControlItem",jiffies);

	if(kcontrol->private_value>=0 && kcontrol->private_value<BRCM_CTL_TOTAL)
	{


			if(sgBrcmCtrlVal[kcontrol->private_value] != ucontrol->value.integer.value[0] )
			{
				sgBrcmCtrlVal[kcontrol->private_value] = ucontrol->value.integer.value[0] ;
//				audvoc_configure_Volume(0, 0x1FF) ;
				changed=1;
			}
			DEBUG("SetControlItem: sgBrcmCtrlVal[%d] = %d\n", kcontrol->private_value, sgBrcmCtrlVal[kcontrol->private_value] );

		if(changed || (kcontrol->private_value == BRCM_CTL_DTMF_Tone_Start) 
				  ||  (kcontrol->private_value == BRCM_CTL_FMDirectPlayCtrl) )
		{
			DEBUG("\n SET SetControlItem [%s]=%d", gpstrCtrlName[kcontrol->private_value], (int)ucontrol->value.integer.value[0]);

			switch(kcontrol->private_value)
			{
				case BRCM_CTL_Incall_EAR_Playback_Volume:
				case BRCM_CTL_EAR_Playback_Volume:
					AUDIO_DRV_SetVoiceVolume((UInt8)sgBrcmCtrlVal[kcontrol->private_value]);
					break;
				case BRCM_CTL_Mic_Capture_Volume:
				case BRCM_CTL_Incall_SPK_Playback_Volume:
				case BRCM_CTL_Incall_HEADSET_Playback_Volume:
				{
					HAL_AUDIO_Param_SpkrVol_t	spkvol;
					spkvol.volumeLevel = sgBrcmCtrlVal[kcontrol->private_value];
					HAL_AUDIO_Ctrl( ACTION_AUD_SetSpeakerVol, &spkvol, NULL);
				}		

					break;
				case BRCM_CTL_Mic_Capture_Mute:
					if(sgBrcmCtrlVal[kcontrol->private_value])
						HAL_AUDIO_Ctrl( ACTION_AUD_MuteMic, NULL, NULL);
					else
						HAL_AUDIO_Ctrl( ACTION_AUD_UnmuteMic, NULL, NULL);
					break;
      				case BRCM_CTL_BTHeadset_Volume:
				case BRCM_CTL_BTHeadset_Ringtone_Volume:
				case BRCM_CTL_BTHeadset_Incall_Volume:
					//do something for BT volume
					break;
				case BRCM_CTL_BTHeadset_Mute:
				case BRCM_CTL_BTHeadset_Ringtone_Mute:
				case BRCM_CTL_BTHeadset_Incall_Mute:
					//do something for BT mute

					break;
				case BRCM_CTL_DTMF_Tone_Start:
					{
						HAL_AUDIO_Get_Param_t getParam;
						
						//get presernt Audio channel and volume levels from HAL
						getParam.paramType = GET_AUDIO_CHNL;
						getParam.audioID = AUDIO_ID_TONE;
						HAL_AUDIO_Ctrl( ACTION_AUD_GetParam, &getParam, NULL);
						audioToneControlParam.outputChnl= (AUDIO_CHANNEL_t) *(getParam.paramPtr);
//						DEBUG("BRCM_CTL_DTMF_Tone_Start: %s tone=%0x%x", ucontrol->value.integer.value[1]?"start":"stop", sgBrcmCtrlVal[kcontrol->private_value]);
						/*generate tone only for BT case,
						rihgt now URIL generates the DTMF tone for non-BT case*/
//						if(audioToneControlParam.outputChnl == AUDIO_CHNL_BT_MONO||audioToneControlParam.outputChnl==AUDIO_CHNL_BT_STEREO)
							{
								if(ucontrol->value.integer.value[1]==0)
									{
										HAL_AUDIO_Ctrl( ACTION_AUD_StopPlayTone, &audioToneControlParam, NULL );
										//printk(KERN_ALERT "Akh:HAL_AUDIO_Ctrl - ACTION_AUD_StopPlayTone\n");
										return changed;
									}

								audioToneControlParam.volumeLevel= AUDIO_VOLUME_MAX;

								audioToneControlParam.tone = sgBrcmCtrlVal[(kcontrol->private_value)];
								//printk(KERN_ALERT "Akh:output channel = %d , tone # = %d \n",(int)audioToneControlParam.outputChnl,(int)audioToneControlParam.tone);
								//we don't  get the duration from the URIL,right now use 100 msec?
								audioToneControlParam.duration = 100;
								//send request to HAL for tone generation
								HAL_AUDIO_Ctrl( ACTION_AUD_PlayTone, &audioToneControlParam, NULL );
							}
					}
					break;
				case BRCM_CTL_FMDirectPlayCtrl:
					FMDirectPlayHandler(kcontrol, ucontrol);
					break;
				default:
					break;
	}
		}


	}


	
	return changed;
}

//Mixer control item definition
#define BRCM_MIXER_CTRL(xname, xindex, private_val) \
{	\
   .iface = SNDRV_CTL_ELEM_IFACE_MIXER, \
  .name = xname, \
  .index = xindex, \
  .access= SNDRV_CTL_ELEM_ACCESS_READWRITE,\
  .private_value = private_val, \
  .info = GetCtrlInfo, \
  .get = GetControlItem, 	\
  .put = SetControlItem, \
}

//Define all the control items
static struct snd_kcontrol_new gpSndCtrls[] __devinitdata =
{
	BRCM_MIXER_CTRL(gpstrCtrlName[BRCM_CTL_Speaker_Playback_Volume], 0, BRCM_CTL_Speaker_Playback_Volume),
	BRCM_MIXER_CTRL(gpstrCtrlName[BRCM_CTL_Speaker_Playback_Mute], 0, BRCM_CTL_Speaker_Playback_Mute),
	BRCM_MIXER_CTRL(gpstrCtrlName[BRCM_CTL_Mic_Capture_Volume], 0, BRCM_CTL_Mic_Capture_Volume),
	BRCM_MIXER_CTRL(gpstrCtrlName[BRCM_CTL_Mic_Capture_Mute], 0, BRCM_CTL_Mic_Capture_Mute),

	BRCM_MIXER_CTRL(gpstrCtrlName[BRCM_CTL_Ringtone_SPK_Playback_Volume], 0, BRCM_CTL_Ringtone_SPK_Playback_Volume),
	BRCM_MIXER_CTRL(gpstrCtrlName[BRCM_CTL_Ringtone_SPK_Playback_Mute], 0, BRCM_CTL_Ringtone_SPK_Playback_Mute),

	BRCM_MIXER_CTRL(gpstrCtrlName[BRCM_CTL_Incall_SPK_Playback_Volume], 0, BRCM_CTL_Incall_SPK_Playback_Volume),
	BRCM_MIXER_CTRL(gpstrCtrlName[BRCM_CTL_Incall_SPK_Playback_Mute], 0, BRCM_CTL_Incall_SPK_Playback_Mute),

	BRCM_MIXER_CTRL(gpstrCtrlName[BRCM_CTL_EAR_Playback_Volume], 0, BRCM_CTL_EAR_Playback_Volume),
	BRCM_MIXER_CTRL(gpstrCtrlName[BRCM_CTL_EAR_Playback_Mute], 0, BRCM_CTL_EAR_Playback_Mute),

	BRCM_MIXER_CTRL(gpstrCtrlName[BRCM_CTL_Ringtone_EAR_Playback_Volume], 0, BRCM_CTL_Ringtone_EAR_Playback_Volume),
	BRCM_MIXER_CTRL(gpstrCtrlName[BRCM_CTL_Ringtone_EAR_Playback_Mute], 0, BRCM_CTL_Ringtone_EAR_Playback_Mute),

	BRCM_MIXER_CTRL(gpstrCtrlName[BRCM_CTL_Incall_EAR_Playback_Volume], 0, BRCM_CTL_Incall_EAR_Playback_Volume),
	BRCM_MIXER_CTRL(gpstrCtrlName[BRCM_CTL_Incall_EAR_Playback_Mute], 0, BRCM_CTL_Incall_EAR_Playback_Mute),

	BRCM_MIXER_CTRL(gpstrCtrlName[BRCM_CTL_HEADSET_Playback_Volume], 0, BRCM_CTL_HEADSET_Playback_Volume),
	BRCM_MIXER_CTRL(gpstrCtrlName[BRCM_CTL_HEADSET_Playback_Mute], 0, BRCM_CTL_HEADSET_Playback_Mute),

	BRCM_MIXER_CTRL(gpstrCtrlName[BRCM_CTL_Ringtone_HEADSET_Playback_Volume], 0, BRCM_CTL_Ringtone_HEADSET_Playback_Volume),
	BRCM_MIXER_CTRL(gpstrCtrlName[BRCM_CTL_Ringtone_HEADSET_Playback_Mute], 0, BRCM_CTL_Ringtone_HEADSET_Playback_Mute),

	BRCM_MIXER_CTRL(gpstrCtrlName[BRCM_CTL_Incall_HEADSET_Playback_Volume], 0, BRCM_CTL_Incall_HEADSET_Playback_Volume),
	BRCM_MIXER_CTRL(gpstrCtrlName[BRCM_CTL_Incall_HEADSET_Playback_Mute], 0, BRCM_CTL_Incall_HEADSET_Playback_Mute),

	BRCM_MIXER_CTRL(gpstrCtrlName[BRCM_CTL_BTHeadset_Volume], 0, BRCM_CTL_BTHeadset_Volume),
	BRCM_MIXER_CTRL(gpstrCtrlName[BRCM_CTL_BTHeadset_Mute], 0, BRCM_CTL_BTHeadset_Mute),

	BRCM_MIXER_CTRL(gpstrCtrlName[BRCM_CTL_BTHeadset_Ringtone_Volume], 0, BRCM_CTL_BTHeadset_Ringtone_Volume),
	BRCM_MIXER_CTRL(gpstrCtrlName[BRCM_CTL_BTHeadset_Ringtone_Mute], 0, BRCM_CTL_BTHeadset_Ringtone_Mute),

	BRCM_MIXER_CTRL(gpstrCtrlName[BRCM_CTL_BTHeadset_Incall_Volume], 0, BRCM_CTL_BTHeadset_Incall_Volume),
	BRCM_MIXER_CTRL(gpstrCtrlName[BRCM_CTL_BTHeadset_Incall_Mute], 0, BRCM_CTL_BTHeadset_Incall_Mute),

	BRCM_MIXER_CTRL(gpstrCtrlName[BRCM_CTL_DTMF_Tone_Start], 0, BRCM_CTL_DTMF_Tone_Start),
	BRCM_MIXER_CTRL(gpstrCtrlName[BRCM_CTL_FMDirectPlayCtrl], 0, BRCM_CTL_FMDirectPlayCtrl),
	BRCM_MIXER_CTRL(gpstrCtrlName[BRCM_CTL_Get_PlaybackPosition], 0, BRCM_CTL_Get_PlaybackPosition),
};

//*****************************************************************
// Functiona Name: ControlDeviceNew
//
// Description: Create control device.
//
//*****************************************************************
int __devinit ControlDeviceNew(struct snd_card *card)
{
	unsigned int idx;
	int err;

	strcpy(card->mixername, "Broadcom ALSA Mixer");

	for (idx = 0; idx < ARRAY_SIZE(gpSndCtrls); idx++)
	{
		if ((err = snd_ctl_add(card, snd_ctl_new1(&gpSndCtrls[idx], g_brcm_alsa_chip))) < 0)
			return err;
	}

	memset(sgBrcmCtrlVal, 0, sizeof(sgBrcmCtrlVal));
   
   return 0;
}

int BrcmCreateControlSysFs(struct snd_card *card)
{
	int ret;
	//create sysfs file for FM directplay control
	ret = snd_add_device_sysfs_file(SNDRV_DEVICE_TYPE_CONTROL,card,-1,&FMDirectplay_attrib);
	//	DEBUG("BrcmCreateControlSysFs ret=%d", ret);	
	return ret;
}

//FIXEDME: need to get exact mode
//-----------------------------------------------------------
// AUDIO_GetAudioMode(): return audio mode (Handset, BT, ...)
//-----------------------------------------------------------
AudioMode_t	AUDIO_GetAudioMode()
{
    HAL_AUDIO_Get_Param_t getParam;
	AudioMode_t curmode = AUDIO_MODE_HANDSET;
	
	getParam.paramType = GET_AUDIO_MODE;
	if( (HAL_AUDIO_Ctrl( ACTION_AUD_GetParam, &getParam, NULL))!=0)
	{
		DEBUG("AUDIO_GetAudioMode:Failed to call HAL_AUDIO_Ctrl\n");
	}
	else
	{
		curmode = *getParam.paramPtr;
		DEBUG("AUDIO_GetAudioMode: curmode=%x  \n",curmode);
	}

	return curmode;
}

int	SwitchCallRate(UInt8 codecId)
{
	int ret = 0;
#if defined(FUSE_APPS_PROCESSOR)	
	HAL_AUDIO_Param_RateCfg_t rateCfg;		
	rateCfg.audioID = (AudioClientID_en_t)((codecId == 0x0A)? AUDIO_ID_CALL16K : AUDIO_ID_CALL);
	HAL_AUDIO_Ctrl( ACTION_AUD_RateChange, &rateCfg, NULL);
	DEBUG("SwitchCallRate: Rate change to # %d\n", rateCfg.audioID);
	/* Unmute speaker and mic */
	HAL_AUDIO_Ctrl( ACTION_AUD_UnmuteSpeaker, NULL, NULL);
	if(0==GetCtrlValue(BRCM_CTL_Mic_Capture_Mute))
		HAL_AUDIO_Ctrl(ACTION_AUD_UnmuteMic, NULL, NULL);
	ret = rateCfg.audioID;
#endif
	return ret;
}

