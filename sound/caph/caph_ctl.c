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
#include <sound/tlv.h>


#include "mobcom_types.h"
#include "auddrv_def.h"
#include "audio_consts.h"
#include "audio_ddriver.h"
#include "audio_controller.h"
#include "brcm_audio_devices.h"
#include "caph_common.h"


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//Volume control information
//
//-------------------------------------------------------------------------------------------
static int VolumeCtrlInfo(struct snd_kcontrol * kcontrol,	struct snd_ctl_elem_info * uinfo)
{
//	brcm_alsa_chip_t*	pChip = (brcm_alsa_chip_t*)snd_kcontrol_chip(kcontrol);
	int priv = kcontrol->private_value;
	int	stream = STREAM_OF_CTL(priv);
//	int	dev = DEV_OF_CTL(priv);

	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->value.integer.step = 1; 
	switch(stream)
	{
		case CTL_STREAM_PANEL_PCMOUT1:
		case CTL_STREAM_PANEL_PCMOUT2:
		case CTL_STREAM_PANEL_VOIPOUT:			
			uinfo->count = 2;
			uinfo->value.integer.min = -50<<2; //Q13.2
			uinfo->value.integer.max = 12<<2;//FIXME
			break;
		case CTL_STREAM_PANEL_VOICECALL:
			uinfo->count = 2;
			uinfo->value.integer.min = -50<<2; //Q13.2
			uinfo->value.integer.max = 0;//FIXME
			break;
		case CTL_STREAM_PANEL_PCMIN:
		case CTL_STREAM_PANEL_VOIPIN:
			//if Lound speaker
			uinfo->count = 1;
			uinfo->value.integer.min = 0; //Q13.2
			uinfo->value.integer.max = 170;//42.5<<2 FIXME
			//if earpiece
			break;
		default:
			break;
	}
	
	return 0;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//Get volume
//
//-------------------------------------------------------------------------------------------
static int VolumeCtrlGet(	struct snd_kcontrol * kcontrol,	struct snd_ctl_elem_value * ucontrol)
{
	brcm_alsa_chip_t*	pChip = (brcm_alsa_chip_t*)snd_kcontrol_chip(kcontrol);
	int priv = kcontrol->private_value;
	int	stream = STREAM_OF_CTL(priv);
	int	dev = DEV_OF_CTL(priv);
	Int32	*pVolume;
	CAPH_ASSERT(stream>=CTL_STREAM_PANEL_FIRST && stream<=CTL_STREAM_PANEL_LAST);
	stream--;
	pVolume = pChip->streamCtl[stream].ctlLine[dev].iVolume;

	//May need to get the value from driver
	ucontrol->value.integer.value[0] = pVolume[0];
	ucontrol->value.integer.value[1] = pVolume[1];

	return 0;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//Set volume
//    Update the mixer control only if the stream is not running.Call audio driver to apply when stream is running
//
//-------------------------------------------------------------------------------------------
static int VolumeCtrlPut(	struct snd_kcontrol * kcontrol,	struct snd_ctl_elem_value * ucontrol)
{
	brcm_alsa_chip_t*	pChip = (brcm_alsa_chip_t*)snd_kcontrol_chip(kcontrol);
	int priv = kcontrol->private_value;
	int	stream = STREAM_OF_CTL(priv);
	int	dev = DEV_OF_CTL(priv);
	Int32	*pVolume;
	CAPH_ASSERT(stream>=CTL_STREAM_PANEL_FIRST && stream<=CTL_STREAM_PANEL_LAST);
	stream--;
	pVolume = pChip->streamCtl[stream].ctlLine[dev].iVolume;
	
	pVolume[0] = ucontrol->value.integer.value[0];
	pVolume[1] = ucontrol->value.integer.value[1];
	//Apply Volume if the stream is running
	switch(stream)
	{
		case CTL_STREAM_PANEL_PCMOUT1:
		case CTL_STREAM_PANEL_PCMOUT2:
		case CTL_STREAM_PANEL_VOIPOUT:			
			AUDCTRL_SetPlayVolume (pChip->streamCtl[stream].dev_prop.u.p.hw_id,
					pChip->streamCtl[stream].dev_prop.u.p.speaker, 
					AUDIO_GAIN_FORMAT_Q13_2, 
					pVolume[0], pVolume[1]);
					
			break;
		case CTL_STREAM_PANEL_VOICECALL:
			break;
		case CTL_STREAM_PANEL_PCMIN:
		case CTL_STREAM_PANEL_VOIPIN:
			break;
		default:
			break;
	}
	

	return 0;
}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//Information of playback sink or capture source
//   Update the mixer control only if the stream is not running.Call audio driver to apply when stream is running
//
//-------------------------------------------------------------------------------------------
static int SelCtrlInfo(struct snd_kcontrol * kcontrol,	struct snd_ctl_elem_info * uinfo)
{
	brcm_alsa_chip_t*	pChip = (brcm_alsa_chip_t*)snd_kcontrol_chip(kcontrol);
	int priv = kcontrol->private_value;
	int	stream = STREAM_OF_CTL(priv);//kcontrol->id.device

	CAPH_ASSERT(stream>=CTL_STREAM_PANEL_FIRST && stream<=CTL_STREAM_PANEL_LAST);
	stream--;
	if(pChip->streamCtl[stream].iFlags & MIXER_STREAM_FLAGS_CAPTURE)
	{
		uinfo->value.integer.min = AUDCTRL_SPK_HANDSET;
		uinfo->value.integer.max = AUDCTRL_SPK_VIBRA;//FIXME
	
	}
	else
	{
		//uinfo->value.integer.min = CTL_DEV_OUT_FIRST;
		uinfo->value.integer.min = AUDCTRL_MIC_MAIN; // disabling the voice call path
		uinfo->value.integer.max = AUDCTRL_MIC_TOTAL_COUNT;//FIXME
	}
	uinfo->value.integer.step = 1;
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 2;

	return 0;
}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//Get playback sink or capture source
//
//-------------------------------------------------------------------------------------------
static int SelCtrlGet(	struct snd_kcontrol * kcontrol,	struct snd_ctl_elem_value * ucontrol)
{
	brcm_alsa_chip_t*	pChip = (brcm_alsa_chip_t*)snd_kcontrol_chip(kcontrol);
	int priv = kcontrol->private_value;
	int	stream = STREAM_OF_CTL(priv);
	Int32	*pSel;
	CAPH_ASSERT(stream>=CTL_STREAM_PANEL_FIRST && stream<=CTL_STREAM_PANEL_LAST);
	stream--;
	pSel = pChip->streamCtl[stream].iLineSelect;

	BCM_AUDIO_DEBUG("xnumid=%d xindex=%d", ucontrol->id.numid, ucontrol->id.index);

	//May need to get the value from driver
	ucontrol->value.integer.value[0] = pSel[0];
	ucontrol->value.integer.value[1] = pSel[1];

	return 0;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//Set playback sink or capture source
//   Update the mixer control only if the stream is not running.Call audio driver to apply when stream is running
//
//-------------------------------------------------------------------------------------------
static int SelCtrlPut(	struct snd_kcontrol * kcontrol,	struct snd_ctl_elem_value * ucontrol)
{
	brcm_alsa_chip_t*	pChip = (brcm_alsa_chip_t*)snd_kcontrol_chip(kcontrol);
	int priv = kcontrol->private_value;
	int	stream = STREAM_OF_CTL(priv);
	Int32	*pSel, *pCurSel;
	CAPH_ASSERT(stream>=CTL_STREAM_PANEL_FIRST && stream<=CTL_STREAM_PANEL_LAST);
//	stream--;
	pSel = pChip->streamCtl[stream-1].iLineSelect;

	pSel[0] = ucontrol->value.integer.value[0];
	pSel[1] = ucontrol->value.integer.value[1];
	pCurSel =pChip->streamCtl[stream-1].currentLineSel;

	BCM_AUDIO_DEBUG("SelCtrlPut stream =%d, pSel[0]=%d\n", (int)stream,(int)pSel[0]);
	//Apply selection if the stream is running ?
	switch(stream)
		{
		case CTL_STREAM_PANEL_PCMOUT1: //pcmout 1

			//if stream is not running
			
			   //return;
			//else
			   //call audio driver to set sink
			
		
		case CTL_STREAM_PANEL_PCMOUT2: //pcmout 2
			break;
		case CTL_STREAM_PANEL_VOICECALL://voice call
			{
			if(pSel[0]==0)
				//disable voice call
				AUDCTRL_DisableTelephony(AUDIO_HW_VOICE_IN,AUDIO_HW_VOICE_OUT,pCurSel[0],pCurSel[1]);   
			else
				{
				
				//enable voicec call with sink
				switch(pSel[0])
					{
					case AUDCTRL_MIC_MAIN:
						//route voice call to earpiece
						pCurSel[0] = AUDCTRL_MIC_MAIN;
						pCurSel[1] = AUDCTRL_SPK_HANDSET;
						AUDCTRL_EnableTelephony(AUDIO_HW_VOICE_IN,AUDIO_HW_VOICE_OUT,pCurSel[0],pCurSel[1]);
						break;
					//case CTL_DEV_OUT_IHF:
						/* check if this is the valid use case. if so, which is the mic to be set
						In case of voice call -DL, src - DSP, sink = IHF. But valid possible 
						use case handled in EnablePath is src-DSPThroughMem and snk = IHF
						*/
						
					//	break;
					case AUDCTRL_MIC_AUX:
						
						pCurSel[0] = AUDCTRL_MIC_AUX;
						pCurSel[1] = AUDCTRL_SPK_HEADSET;
						AUDCTRL_EnableTelephony(AUDIO_HW_VOICE_IN,AUDIO_HW_VOICE_OUT,pCurSel[0],pCurSel[1]);
						break;
					}
				}
			}
			break;
		default:
			break;
		}
	

	return 0;
}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//Get MUTE status of the deivce
//   
//
//-------------------------------------------------------------------------------------------
static int SwitchCtrlGet(	struct snd_kcontrol * kcontrol,	struct snd_ctl_elem_value * ucontrol)
{
	brcm_alsa_chip_t*	pChip = (brcm_alsa_chip_t*)snd_kcontrol_chip(kcontrol);
	int priv = kcontrol->private_value;
	int	stream = STREAM_OF_CTL(priv);
	int	dev = DEV_OF_CTL(priv);
	Int32	*pMute;
	CAPH_ASSERT(stream>=CTL_STREAM_PANEL_FIRST && stream<=CTL_STREAM_PANEL_LAST);
	stream--;
	pMute = pChip->streamCtl[stream].ctlLine[dev].iMute;

	//May need to get the value from driver
	pMute[0] = ucontrol->value.integer.value[0];
	pMute[1] = ucontrol->value.integer.value[1];
	
	return 0;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//Set deivce MUTE
//   Update the mixer control only if the stream is not running.Call audio driver to apply when stream is running
//
//-------------------------------------------------------------------------------------------
static int SwitchCtrlPut(	struct snd_kcontrol * kcontrol,	struct snd_ctl_elem_value * ucontrol)
{
	brcm_alsa_chip_t*	pChip = (brcm_alsa_chip_t*)snd_kcontrol_chip(kcontrol);
	int priv = kcontrol->private_value;
	int	stream = STREAM_OF_CTL(priv);
	int	dev = DEV_OF_CTL(priv);
	Int32	*pMute;
	CAPH_ASSERT(stream>=CTL_STREAM_PANEL_FIRST && stream<=CTL_STREAM_PANEL_LAST);
	stream--;
	pMute = pChip->streamCtl[stream].ctlLine[dev].iMute;
	
	ucontrol->value.integer.value[0] = pMute[0];
	ucontrol->value.integer.value[1] = pMute[1];

	//Apply mute is the stream is running

	return 0;
}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//MISC control information
//
//-------------------------------------------------------------------------------------------
static int MiscCtrlInfo(struct snd_kcontrol * kcontrol,	struct snd_ctl_elem_info * uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 3;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 5;//FIXME
	uinfo->value.integer.step = 1; 

	return 0;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//Get MISC control value
//
//-------------------------------------------------------------------------------------------
static int MiscCtrlGet(	struct snd_kcontrol * kcontrol,	struct snd_ctl_elem_value * ucontrol)
{
	brcm_alsa_chip_t*	pChip = (brcm_alsa_chip_t*)snd_kcontrol_chip(kcontrol);
	int priv = kcontrol->private_value;
	int function = priv&0xFF;

	switch(function)
	{
		case CTL_FUNCTION_LOOPBACK_TEST:
			ucontrol->value.integer.value[0] = pChip->pi32LoopBackTestParam[0];
			ucontrol->value.integer.value[1] = pChip->pi32LoopBackTestParam[1];
			ucontrol->value.integer.value[2] = pChip->pi32LoopBackTestParam[2];
			break;
		default:
			break;
			
	}

	return 0;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//Set MISC control
//
//-------------------------------------------------------------------------------------------
static int MiscCtrlPut(	struct snd_kcontrol * kcontrol,	struct snd_ctl_elem_value * ucontrol)
{
	brcm_alsa_chip_t*	pChip = (brcm_alsa_chip_t*)snd_kcontrol_chip(kcontrol);
	int priv = kcontrol->private_value;
	int function = priv&0xFF;

	switch(function)
	{
		case CTL_FUNCTION_LOOPBACK_TEST:
			pChip->pi32LoopBackTestParam[0] = ucontrol->value.integer.value[0];
			pChip->pi32LoopBackTestParam[1] = ucontrol->value.integer.value[1];
			pChip->pi32LoopBackTestParam[2] = ucontrol->value.integer.value[2];

			//Do loopback test
			AUDCTRL_SetAudioLoopback(pChip->pi32LoopBackTestParam[0],(AUDCTRL_MICROPHONE_t)pChip->pi32LoopBackTestParam[1],(AUDCTRL_SPEAKER_t)pChip->pi32LoopBackTestParam[2]);
			break;
		default:
			break;
	}


	return 0;
}






//Control value is in DB, minimium -50db, step 0.25db, minimium does not mean MUTE
static const DECLARE_TLV_DB_SCALE(caph_db_scale_volume, -5000, 25, 0);


#define BRCM_MIXER_CTRL_GENERAL(nIface, iDevice, iSubdev, sName, iIndex, iAccess, iCount, fInfo, fGet, fPut, pTlv, lPriv_val) \
	{	\
	   .iface = nIface, \
	   .device = iDevice, \
	   .subdevice = iSubdev, \
	   .name = sName, \
	   .index = iIndex, \
	   .access= iAccess,\
	   .count = iCount, \
	   .info = fInfo, \
	   .get = fGet,	\
	   .put = fPut, \
	   .tlv = { .p = pTlv }, \
	   .private_value = lPriv_val, \
	}
	


#define BRCM_MIXER_CTRL_VOLUME(dev, subdev, xname, xindex, private_val) \
	BRCM_MIXER_CTRL_GENERAL(SNDRV_CTL_ELEM_IFACE_MIXER, dev, subdev, xname, xindex, SNDRV_CTL_ELEM_ACCESS_READWRITE, 0, \
	                        VolumeCtrlInfo, VolumeCtrlGet, VolumeCtrlPut, caph_db_scale_volume,private_val)

#define BRCM_MIXER_CTRL_SWITCH(dev, subdev, xname, xindex, private_val) \
	BRCM_MIXER_CTRL_GENERAL(SNDRV_CTL_ELEM_IFACE_MIXER, dev, subdev, xname, xindex, SNDRV_CTL_ELEM_ACCESS_READWRITE, 0, \
	                        snd_ctl_boolean_stereo_info, SwitchCtrlGet, SwitchCtrlPut, 0,private_val)

#define BRCM_MIXER_CTRL_SELECTION(dev, subdev, xname, xindex, private_val) \
	BRCM_MIXER_CTRL_GENERAL(SNDRV_CTL_ELEM_IFACE_MIXER, dev, subdev, xname, xindex, SNDRV_CTL_ELEM_ACCESS_READWRITE, 0, \
	                        SelCtrlInfo, SelCtrlGet, SelCtrlPut, 0,private_val)


#define BRCM_MIXER_CTRL_MISC(dev, subdev, xname, xindex, private_val) \
	BRCM_MIXER_CTRL_GENERAL(SNDRV_CTL_ELEM_IFACE_MIXER, dev, subdev, xname, xindex, SNDRV_CTL_ELEM_ACCESS_READWRITE, 0, \
	                        MiscCtrlInfo, MiscCtrlGet, MiscCtrlPut, 0,private_val)







//
//Initial data of controls, runtime data is in 'chip' data structure
static	TPcm_Stream_Ctrls	sgCaphStreamCtls[CAPH_MAX_PCM_STREAMS] __initdata = 
	{
		//PCMOut1
		{
			.iTotalCtlLines = AUDCTRL_SPK_TOTAL_COUNT,
			.iLineSelect = {AUDCTRL_SPK_HANDSET, AUDCTRL_SPK_HANDSET},
			.strStreamName = "PCMOut 1",
			.ctlLine = {
						{.strName = "Handset",	.iVolume = {12,12},},		//AUDCTRL_SPK_HANDSET
						{.strName = "Headset",		.iVolume = {12,12},}, 	//AUDCTRL_SPK_HEADSET
						{.strName = "Handsfree",	.iVolume = {12,12},},	//AUDCTRL_SPK_HANDSFREE
						{.strName = "BT SCO",	.iVolume = {12,12},},		//AUDCTRL_SPK_BTM
						{.strName = "Loud Speaker",	.iVolume = {12,12},},	//AUDCTRL_SPK_LOUDSPK
						{.strName = "",	.iVolume = {0,0},}, 				//AUDCTRL_SPK_TTY
						{.strName = "",	.iVolume = {0,0},}, 				//AUDCTRL_SPK_HAC
						{.strName = "",	.iVolume = {0,0},},					//AUDCTRL_SPK_USB
						{.strName = "", .iVolume = {0,0},}, 				//AUDCTRL_SPK_BTS
						{.strName = "I2S", .iVolume = {13,13},}, 			//AUDCTRL_SPK_I2S
						{.strName = "Speaker Vibra", .iVolume = {14,14},},	//AUDCTRL_SPK_VIBRA
					},
		},

		//PCMOut2
		{
			.iTotalCtlLines = AUDCTRL_SPK_TOTAL_COUNT,
			.iLineSelect = {AUDCTRL_SPK_LOUDSPK, AUDCTRL_SPK_LOUDSPK},
			.strStreamName = "PCMOut 2",
			.ctlLine = {
						{.strName = "Handset",	.iVolume = {12,12},},		//AUDCTRL_SPK_HANDSET
						{.strName = "Headset",		.iVolume = {12,12},},	//AUDCTRL_SPK_HEADSET
						{.strName = "Handsfree",	.iVolume = {12,12},},	//AUDCTRL_SPK_HANDSFREE
						{.strName = "BT SCO",	.iVolume = {12,12},},		//AUDCTRL_SPK_BTM
						{.strName = "Loud Speaker", .iVolume = {12,12},},	//AUDCTRL_SPK_LOUDSPK
						{.strName = "", .iVolume = {0,0},}, 				//AUDCTRL_SPK_TTY
						{.strName = "", .iVolume = {0,0},}, 				//AUDCTRL_SPK_HAC
						{.strName = "", .iVolume = {0,0},}, 				//AUDCTRL_SPK_USB
						{.strName = "", .iVolume = {0,0},}, 				//AUDCTRL_SPK_BTS
						{.strName = "I2S", .iVolume = {13,13},},			//AUDCTRL_SPK_I2S
						{.strName = "Speaker Vibra", .iVolume = {14,14},},	//AUDCTRL_SPK_VIBRA
					},
		},

						
		//VOIP Out
		{
			.iTotalCtlLines = AUDCTRL_SPK_TOTAL_COUNT,
			.iLineSelect = {AUDCTRL_SPK_LOUDSPK, AUDCTRL_SPK_LOUDSPK},
			.strStreamName = "VOIP Out",
			.ctlLine = {
						{.strName = "Handset",	.iVolume = {12,12},},		//AUDCTRL_SPK_HANDSET
						{.strName = "Headset",		.iVolume = {12,12},},	//AUDCTRL_SPK_HEADSET
						{.strName = "Handsfree",	.iVolume = {12,12},},	//AUDCTRL_SPK_HANDSFREE
						{.strName = "BT SCO",	.iVolume = {12,12},},		//AUDCTRL_SPK_BTM
						{.strName = "Loud Speaker", .iVolume = {12,12},},	//AUDCTRL_SPK_LOUDSPK
						{.strName = "", .iVolume = {0,0},}, 				//AUDCTRL_SPK_TTY
						{.strName = "", .iVolume = {0,0},}, 				//AUDCTRL_SPK_HAC
						{.strName = "", .iVolume = {0,0},}, 				//AUDCTRL_SPK_USB
						{.strName = "", .iVolume = {0,0},}, 				//AUDCTRL_SPK_BTS
						{.strName = "I2S", .iVolume = {13,13},},			//AUDCTRL_SPK_I2S
						{.strName = "Speaker Vibra", .iVolume = {14,14},},	//AUDCTRL_SPK_VIBRA
					},
		},

		//PCM In
		{
			.iFlags = MIXER_STREAM_FLAGS_CAPTURE,
			.iTotalCtlLines = AUDCTRL_MIC_TOTAL_COUNT,
			.iLineSelect = {AUDCTRL_SPK_LOUDSPK, AUDCTRL_SPK_LOUDSPK},
			.strStreamName = "PCMIn",
			.ctlLine = {
						{.strName = "", .iVolume = {0,0},},						//AUDCTRL_MIC_UNDEFINED
						{.strName = "Main Mic",		.iVolume = {12,12},}, 		//AUDCTRL_MIC_MAIN
						{.strName = "AUX Mic",	.iVolume = {12,12},},			//AUDCTRL_MIC_AUX
						{.strName = "Digital MIC 1",	.iVolume = {12,12},},	//AUDCTRL_MIC_DIGI1
						{.strName = "Digital MIC 2",	.iVolume = {12,12},},	//AUDCTRL_MIC_DIGI2
						{.strName = "Digital Mic 12",	.iVolume = {12,12},},	//AUDCTRL_DUAL_MIC_DIGI12
						{.strName = "Digital Mic 21",	.iVolume = {12,12},},	//AUDCTRL_DUAL_MIC_DIGI21
						{.strName = "MIC_ANALOG_DIGI1",	.iVolume = {12,12},},	//AUDCTRL_DUAL_MIC_ANALOG_DIGI1
						{.strName = "MIC_DIGI1_ANALOG", .iVolume = {0,0},},		//AUDCTRL_DUAL_MIC_DIGI1_ANALOG
						{.strName = "BT SCO Mic",		.iVolume = {12,12},}, 	//AUDCTRL_MIC_BTM
						{.strName = "",	.iVolume = {0,0},},						//AUDCTRL_MIC_USB
						{.strName = "I2S",	.iVolume = {12,12},},				//AUDCTRL_MIC_I2S
						{.strName = "MIC_DIGI3",	.iVolume = {12,12},},		//AUDCTRL_MIC_DIGI3
						{.strName = "MIC_DIGI4",	.iVolume = {12,12},},		//AUDCTRL_MIC_DIGI4
						{.strName = "MIC_SPEECH_DIGI",	.iVolume = {12,12},}, 	//AUDCTRL_MIC_SPEECH_DIGI
						{.strName = "MIC_EANC_DIGI",	.iVolume = {12,12},}, 	//AUDCTRL_MIC_EANC_DIGI
					},
		},

		//VOIP In
		{
			.iFlags = MIXER_STREAM_FLAGS_CAPTURE,
			.iTotalCtlLines = AUDCTRL_MIC_TOTAL_COUNT,
			.iLineSelect = {AUDCTRL_SPK_LOUDSPK, AUDCTRL_SPK_LOUDSPK},
			.strStreamName = "VOIP In",
			.ctlLine = {
						{.strName = "", .iVolume = {0,0},}, 					//AUDCTRL_MIC_UNDEFINED
						{.strName = "Main Mic", 	.iVolume = {12,12},},		//AUDCTRL_MIC_MAIN
						{.strName = "AUX Mic",	.iVolume = {12,12},},			//AUDCTRL_MIC_AUX
						{.strName = "Digital MIC 1",	.iVolume = {12,12},},	//AUDCTRL_MIC_DIGI1
						{.strName = "Digital MIC 2",	.iVolume = {12,12},},	//AUDCTRL_MIC_DIGI2
						{.strName = "Digital Mic 12",	.iVolume = {12,12},},	//AUDCTRL_DUAL_MIC_DIGI12
						{.strName = "Digital Mic 21",	.iVolume = {12,12},},	//AUDCTRL_DUAL_MIC_DIGI21
						{.strName = "MIC_ANALOG_DIGI1",	.iVolume = {12,12},},	//AUDCTRL_DUAL_MIC_ANALOG_DIGI1
						{.strName = "MIC_DIGI1_ANALOG", .iVolume = {0,0},},		//AUDCTRL_DUAL_MIC_DIGI1_ANALOG
						{.strName = "BT SCO Mic",		.iVolume = {12,12},},	//AUDCTRL_MIC_BTM
						{.strName = "", .iVolume = {0,0},}, 					//AUDCTRL_MIC_USB
						{.strName = "I2S",	.iVolume = {12,12},},				//AUDCTRL_MIC_I2S
						{.strName = "MIC_DIGI3",	.iVolume = {12,12},},		//AUDCTRL_MIC_DIGI3
						{.strName = "MIC_DIGI4",	.iVolume = {12,12},},		//AUDCTRL_MIC_DIGI4
						{.strName = "MIC_SPEECH_DIGI",	.iVolume = {12,12},},	//AUDCTRL_MIC_SPEECH_DIGI
						{.strName = "MIC_EANC_DIGI",	.iVolume = {12,12},},	//AUDCTRL_MIC_EANC_DIGI
					},
		},
		//Voice call
		{
			.iFlags = MIXER_STREAM_FLAGS_CALL,
			.iTotalCtlLines = AUDCTRL_SPK_TOTAL_COUNT,
			.iLineSelect = {AUDCTRL_SPK_LOUDSPK, AUDCTRL_SPK_LOUDSPK},
			.strStreamName = "Voice Call",
			.ctlLine = {
						{.strName = "Handset",	.iVolume = {12,12},},		//AUDCTRL_SPK_HANDSET
						{.strName = "Headset",		.iVolume = {12,12},},	//AUDCTRL_SPK_HEADSET
						{.strName = "Handsfree",	.iVolume = {12,12},},	//AUDCTRL_SPK_HANDSFREE
						{.strName = "BT SCO",	.iVolume = {12,12},},		//AUDCTRL_SPK_BTM
						{.strName = "Loud Speaker", .iVolume = {12,12},},	//AUDCTRL_SPK_LOUDSPK
						{.strName = "", .iVolume = {0,0},}, 				//AUDCTRL_SPK_TTY
						{.strName = "", .iVolume = {0,0},}, 				//AUDCTRL_SPK_HAC
						{.strName = "", .iVolume = {0,0},}, 				//AUDCTRL_SPK_USB
						{.strName = "", .iVolume = {0,0},}, 				//AUDCTRL_SPK_BTS
						{.strName = "I2S", .iVolume = {13,13},},			//AUDCTRL_SPK_I2S
						{.strName = "Speaker Vibra", .iVolume = {14,14},},	//AUDCTRL_SPK_VIBRA
					},
		},		

	};

#define	MAX_CTL_NUMS	130
static char gStrCtlNames[MAX_CTL_NUMS][32]; 

//*****************************************************************
// Functiona Name: ControlDeviceNew
//
// Description: Create control device.
//
//*****************************************************************
int __devinit ControlDeviceNew(struct snd_card *card)
{
	unsigned int idx, j;
	int err = 0;
	brcm_alsa_chip_t*	pChip = (brcm_alsa_chip_t*)card->private_data;
	int	nIndex=0;

		
	strcpy(card->mixername, "Broadcom CAPH Mixer");
	memcpy(pChip->streamCtl, &sgCaphStreamCtls, sizeof(sgCaphStreamCtls));

	for (idx = 0; idx < ARRAY_SIZE(sgCaphStreamCtls); idx++)
	{
		//Selection
		struct snd_kcontrol_new devSelect = BRCM_MIXER_CTRL_SELECTION(0, 0, 0, 0, 0); //1234567890
		if(sgCaphStreamCtls[idx].iFlags & MIXER_STREAM_FLAGS_CAPTURE)
		{
			sprintf(gStrCtlNames[nIndex], "%s Capture Source", sgCaphStreamCtls[idx].strStreamName);
			devSelect.name = gStrCtlNames[nIndex++];
			//devSelect.name = "Capture Source";
			devSelect.device = idx+1;//idx - 3 + CTL_STREAM_PANEL_PCMIN;
			devSelect.private_value = CAPH_CTL_PRIVATE(devSelect.device, devSelect.subdevice, 0);
		}
		else if(sgCaphStreamCtls[idx].iFlags & MIXER_STREAM_FLAGS_CALL)
		{
			sprintf(gStrCtlNames[nIndex], "Call device");
			devSelect.name = gStrCtlNames[nIndex++];
		
//			devSelect.name = "Call device";
			devSelect.device = idx + 1; // CTL_STREAM_PANEL_FIRST;
			devSelect.private_value = CAPH_CTL_PRIVATE(devSelect.device, devSelect.subdevice, 0);
		}
		else
		{
			sprintf(gStrCtlNames[nIndex], "%s Playback Sink", sgCaphStreamCtls[idx].strStreamName);
			devSelect.name = gStrCtlNames[nIndex++];
		
//			devSelect.name = "Playback Sink";
			devSelect.device = idx + 1; // CTL_STREAM_PANEL_FIRST;
			devSelect.private_value = CAPH_CTL_PRIVATE(devSelect.device, devSelect.subdevice, 0);
		}
		if ((err = snd_ctl_add(card, snd_ctl_new1(&devSelect, pChip))) < 0)
		{
			BCM_AUDIO_DEBUG("Error to add devselect idx=%d\n", idx);
			return err;
		}

//		BCM_AUDIO_DEBUG("i=%d, j=%d ctl=%s", idx, j, devSelect.name);
		//volume mute
		for(j=0; j<sgCaphStreamCtls[idx].iTotalCtlLines; j++)
		{
			struct snd_kcontrol_new kctlVolume = BRCM_MIXER_CTRL_VOLUME(0, idx, 0, 0, 0);
			struct snd_kcontrol_new kctlMute = BRCM_MIXER_CTRL_SWITCH(0, idx, "Mute", 0, 0);

			if(sgCaphStreamCtls[idx].ctlLine[j].strName[0]==0) //dummy line
				continue;
			
			if(sgCaphStreamCtls[idx].iFlags & MIXER_STREAM_FLAGS_CAPTURE)
			{
			
			sprintf(gStrCtlNames[nIndex], "%s %s Gain", sgCaphStreamCtls[idx].strStreamName, sgCaphStreamCtls[idx].ctlLine[j].strName);
			kctlVolume.name = gStrCtlNames[nIndex++];
			
			//ctlVolume.name = "Gain";
			}
			else
			{
				sprintf(gStrCtlNames[nIndex], "%s %s Volume", sgCaphStreamCtls[idx].strStreamName, sgCaphStreamCtls[idx].ctlLine[j].strName);
				kctlVolume.name = gStrCtlNames[nIndex++];
			
//		kctlVolume.name = "Volume";
			}


			kctlVolume.device = idx+1;
			kctlVolume.subdevice = j;
			kctlVolume.private_value = CAPH_CTL_PRIVATE(kctlVolume.device, kctlVolume.subdevice, CTL_FUNCTION_VOL);

			kctlMute.device = idx+1;
			kctlMute.subdevice = j;
			kctlMute.private_value = CAPH_CTL_PRIVATE(kctlMute.device, kctlMute.subdevice, CTL_FUNCTION_MUTE);

			
			if ((err = snd_ctl_add(card, snd_ctl_new1(&kctlVolume, pChip))) < 0)
			{
				BCM_AUDIO_DEBUG("error to add volume for idx=%d j=%d err=%d \n", idx,j, err);
				return err;
			}


			
			sprintf(gStrCtlNames[nIndex], "%s %s Mute", sgCaphStreamCtls[idx].strStreamName, sgCaphStreamCtls[idx].ctlLine[j].strName);
			kctlMute.name = gStrCtlNames[nIndex++];
			if ((err = snd_ctl_add(card, snd_ctl_new1(&kctlMute, pChip))) < 0)
			{
				BCM_AUDIO_DEBUG("error to add mute for idx=%d j=%d err=%d\n", idx,j, err);			
				return err;
			}
//			BCM_AUDIO_DEBUG("i=%d, j=%d ctl=%s", idx, j, kctlVolume.name);
//			BCM_AUDIO_DEBUG("i=%d, j=%d ctl=%s\n", idx, j, kctlMute.name);
		
		}
	}

   //MISC 
   {
	   //Loopback Test control
	   struct snd_kcontrol_new ctlLoopTest = BRCM_MIXER_CTRL_MISC(1, 1, "PCM Loopback Test", 0, CAPH_CTL_PRIVATE(1, 1, CTL_FUNCTION_LOOPBACK_TEST) );
			   
	   if ((err = snd_ctl_add(card, snd_ctl_new1(&ctlLoopTest, pChip))) < 0)
	   {
		   BCM_AUDIO_DEBUG("error to add loopback test control err=%d\n", err); 		   
		   return err;
	   }
   }

   return err;
}


//debugging assert util
void caphassert(const char *fcn, int line, const char *expr)
{
//	if (in_interrupt())
//		panic("ASSERTION FAILED IN INTERRUPT, %s:%s:%d %s\n",
//		      __FILE__, fcn, line, expr);
//	else 
	{
		int x;
		printk(KERN_ERR "ASSERTION FAILED, %s:%s:%d %s\n",
		       __FILE__, fcn, line, expr);
		x = * (volatile int *) 0; /* force proc to exit */
	}
}


snd_pcm_state_t	PCMStreamStatus(struct snd_card *card, int device, int stream)
{
	struct snd_pcm *pcm;
	struct snd_pcm_substream *substream;
	int num=0, index=0, found;

	CAPH_ASSERT(stream>=CTL_STREAM_PANEL_FIRST && stream<=CTL_STREAM_PANEL_LAST);

	switch (stream)
	{
		case CTL_STREAM_PANEL_PCMOUT1:
		case CTL_STREAM_PANEL_PCMOUT2:
		case CTL_STREAM_PANEL_VOIPOUT:
			index = 0;	//PCM out
			num = stream - CTL_STREAM_PANEL_PCMOUT1;
			break;
		case CTL_STREAM_PANEL_PCMIN:
		case CTL_STREAM_PANEL_VOIPIN:
			index = 1; //PCM in
			num = stream - CTL_STREAM_PANEL_PCMIN;
			break;
		default:
			//invalid
			BCM_AUDIO_DEBUG("No PCM device for stream %d\n", stream);
			break;
	}

	found = 0;
	list_for_each_entry(pcm, &card->devices, list) {
		if (pcm->card == card && pcm->device == device)
		{
			found = 1;
			break;
		}
	}

	if(found)
	{
		substream = pcm->streams[index].substream;
		for (; substream; substream = substream->next)
			if(num==substream->number)
				return substream->runtime->status->state;
	}

	return SNDRV_PCM_STATE_DISCONNECTED;
		
}

