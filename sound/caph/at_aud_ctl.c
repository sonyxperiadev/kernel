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
#include <sound/asound.h>
#include <sound/rawmidi.h>
#include <sound/initval.h>
#include <sound/tlv.h>
#include "mobcom_types.h"
#include "resultcode.h"
#include "auddrv_def.h"
#include "audio_consts.h"
#include "dspif_voice_play.h"
#include "audio_ddriver.h"
#include "drv_caph.h"

#include "audio_controller.h"
#include "bcm_audio_devices.h"
#include "caph_common.h"


#if !defined(CONFIG_SND_BCM_AUDIO_DEBUG_OFF)
void _bcm_snd_printk(unsigned int level, const char *path, int line, const char *format, ...)
{
	va_list args;
	
	if (gAudioDebugLevel < level)
	{
//		printk("gAudioDebugLevel=%d level=%d\n", gAudioDebugLevel, level);
		return;
	}
	va_start(args, format);
	vprintk(format, args);
	va_end(args);
}
#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//Description:
//	Get sink and source device by PCG mode
//Parameters
//	mode -- PCG mode
//	pMic	 -- Source device coresponding to PCG mode
//	pSpk -- Sink device coresponding to PCG mode
//Return
//   0
//------------------------------------------------------------------------------------------
UInt32 GetVoiceSrcSinkByMode(AudioMode_t mode, AUDCTRL_MICROPHONE_t *pMic, AUDCTRL_SPEAKER_t *pSpk)
{

	switch(mode)
	{
		case	AUDIO_MODE_HANDSET:
		case	AUDIO_MODE_HANDSET_WB:
        case    AUDIO_MODE_HAC:
        case    AUDIO_MODE_HAC_WB:                
			*pMic = AUDCTRL_MIC_MAIN;
			*pSpk = AUDCTRL_SPK_HANDSET;
			break;
		case	AUDIO_MODE_HEADSET:
		case	AUDIO_MODE_HEADSET_WB:
        case    AUDIO_MODE_TTY:
        case    AUDIO_MODE_TTY_WB:
			*pMic = AUDCTRL_MIC_AUX;
			*pSpk = AUDCTRL_SPK_HEADSET;
			break;
		case	AUDIO_MODE_BLUETOOTH:
		case	AUDIO_MODE_BLUETOOTH_WB:
			*pMic = AUDCTRL_MIC_BTM;
			*pSpk = AUDCTRL_SPK_BTM;
			break;
		case	AUDIO_MODE_SPEAKERPHONE:
		case	AUDIO_MODE_SPEAKERPHONE_WB:
			*pMic = AUDCTRL_MIC_MAIN;
			*pSpk = AUDCTRL_SPK_LOUDSPK;
			break;
		default:
			BCM_AUDIO_DEBUG("GetVoiceSrcSinkByMode() mode is out of range %d\r\n", mode);		
			break;
	}
	return 0;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//Description: 
//	AT command handler, handle command AT commands at*maudmode=P1,P2,P3
//Parameters:
//	pChip -- Pointer to chip data structure
//   ParamCount -- Count of parameter array
//	Params  --- P1,P2,...,P6
//---------------------------------------------------------------------------
int	AtMaudMode(brcm_alsa_chip_t* pChip, Int32	ParamCount, Int32 *Params)
{
	AUDCTRL_MICROPHONE_t mic = AUDCTRL_MIC_MAIN;
	AUDCTRL_SPEAKER_t spk = AUDCTRL_SPK_HANDSET;


	BCM_AUDIO_DEBUG("%s P1-P6=%d %d %d %d %d %d cnt=%d\n", __FUNCTION__, Params[0], Params[1], Params[2], Params[3], Params[4], Params[5], ParamCount);	

	switch(Params[0])//P1
	{

		case 0:	//at*maudmode 0
		//Get mode from driver
		//Params[0]  = AUDCTRL_GetAudioMode();
		//Or
			Params[0] = pChip->streamCtl[CTL_STREAM_PANEL_VOICECALL].iLineSelect[0]; 
			BCM_AUDIO_DEBUG("%s mode %d \n", __FUNCTION__, Params[0]);
			return 0;

		case 1:	//at*maudmode 1 mode
			GetVoiceSrcSinkByMode(Params[1], &mic, &spk);
	//			AUDCTRL_SaveAudioModeFlag( Params[0],0 );
	//			AUDCTRL_EnableTelephony(AUDIO_HW_VOICE_IN,AUDIO_HW_VOICE_OUT,mic,spk);
	//			Update 'VC-SEL' -- 
			pChip->streamCtl[CTL_STREAM_PANEL_VOICECALL].iLineSelect[0] = spk;
			BCM_AUDIO_DEBUG("%s spk %d mode %d \n", __FUNCTION__, spk,Params[1]);
			return 0;
		default:
			BCM_AUDIO_DEBUG("%s Unsupported cmd %d \n", __FUNCTION__, Params[0]);		
			break;
	}
	
	return -1;		
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//Description: 
//	AT command handler, handle command AT commands at+maudlooptest=P1,P2,P3
//Parameters:
//	pChip -- Pointer to chip data structure
//   ParamCount -- Count of parameter array
//	Params  --- P1,P2,...,P6
//---------------------------------------------------------------------------
int	AtMaudLoopback(brcm_alsa_chip_t* pChip, Int32	ParamCount, Int32 *Params)
{
	BCM_AUDIO_DEBUG("%s P1-P6=%d %d %d %d %d %d cnt=%d\n", __FUNCTION__, Params[0], Params[1], Params[2], Params[3], Params[4], Params[5], ParamCount);	
	return 0;		
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//Description: 
//	AT command handler, handle command AT commands at*maudlog=P1,P2,P3
//Parameters:
//	pChip -- Pointer to chip data structure
//   ParamCount -- Count of parameter array
//	Params  --- P1,P2,...,P6
//---------------------------------------------------------------------------
int	AtMaudLog(brcm_alsa_chip_t* pChip, Int32	ParamCount, Int32 *Params)
{
	BCM_AUDIO_DEBUG("%s P1-P6=%d %d %d %d %d %d cnt=%d\n", __FUNCTION__, Params[0], Params[1], Params[2], Params[3], Params[4], Params[5], ParamCount);	
	return 0;		
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//Description: 
//	AT command handler, handle command AT commands at*maudtst=P1,P2,P3,P4,P5,P6
//Parameters:
//	pChip -- Pointer to chip data structure
//   ParamCount -- Count of parameter array
//	Params  --- P1,P2,...,P6
//---------------------------------------------------------------------------
int	AtMaudTst(brcm_alsa_chip_t* pChip, Int32	ParamCount, Int32 *Params)
{
	BCM_AUDIO_DEBUG("%s P1-P6=%d %d %d %d %d %d cnt=%d\n", __FUNCTION__, Params[0], Params[1], Params[2], Params[3], Params[4], Params[5], ParamCount);	
	return 0;		
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//Description: 
//	AT command handler, handle command AT commands at*maudvol=P1,P2,P3
//Parameters:
//	pChip -- Pointer to chip data structure
//   ParamCount -- Count of parameter array
//	Params  --- P1,P2,...,P6
//---------------------------------------------------------------------------
int	AtMaudVol(brcm_alsa_chip_t* pChip, Int32	ParamCount, Int32 *Params)
{
	Int32 *pVolume;
	int mode;

	BCM_AUDIO_DEBUG("%s P1-P6=%d %d %d %d %d %d cnt=%d\n", __FUNCTION__, Params[0], Params[1], Params[2], Params[3], Params[4], Params[5], ParamCount);	

	switch(Params[0])//P1
	{
		case 6:	//at*maudvol=6
			//Get volume from driver
			//Params[0] = AUDCTRL_GetTelephonySpkrVolume();
			//or
			mode = pChip->streamCtl[CTL_STREAM_PANEL_VOICECALL].iLineSelect[0]; //temp, FIXME
			pVolume = pChip->streamCtl[CTL_STREAM_PANEL_VOICECALL].ctlLine[mode].iVolume;
			Params[0] = pVolume[0];
			BCM_AUDIO_DEBUG("%s pVolume[0] %d mode=%d\n", __FUNCTION__, Params[0],mode); 

			return 0;;
		case 7: //at*maudvol=7,x
			//mode	= AUDCTRL_GetAudioMode();
			//GetVoiceSrcSinkByMode(mode, &mic, &spk);
			//or 
			mode = pChip->streamCtl[CTL_STREAM_PANEL_VOICECALL].iLineSelect[0]; //temp, FIXME
			pVolume = pChip->streamCtl[CTL_STREAM_PANEL_VOICECALL].ctlLine[mode].iVolume;
			pVolume[0] = Params[1];
		//			AUDCTRL_SetTelephonySpkrVolume (AUDIO_HW_VOICE_OUT, mode, pVolume[0], AUDIO_GAIN_FORMAT_Q13_2);//DL
		
			BCM_AUDIO_DEBUG("%s pVolume[0] %d mode=%d \n", __FUNCTION__, pVolume[0],mode); 
			return 0;
		default:
			BCM_AUDIO_DEBUG("%s Unsupported cmd %d \n", __FUNCTION__, Params[0]);		
			break;
	}	
	return -1;		
}



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//Description:
//	Kernel AT command handler and gernal purpose controls
//Parameters
//	cmdIndex -- command index coresponding to AT comand
//	ParamCount	 -- count of array Params
//	Params -- Parameter array
//Return
//   0 on success, -1 otherwise
//------------------------------------------------------------------------------------------
int	AtAudCtlHandler_put(Int32 cmdIndex, brcm_alsa_chip_t* pChip, Int32	ParamCount, Int32 *Params)
{
	BCM_AUDIO_DEBUG("AT-AUD-put ctl=%d ParamCount= %d [%d %d %d %d %d %d %d]\n", cmdIndex, ParamCount, Params[0],Params[1],Params[2], Params[3],Params[4],Params[5],Params[6]);


	switch(cmdIndex)
	{
		case AT_AUD_CTL_INDEX:
			{
				int count = sizeof(pChip->i32AtAudHandlerParms)/sizeof(pChip->i32AtAudHandlerParms[0]);
				if(count>ParamCount)
					count = ParamCount;
				memcpy(pChip->i32AtAudHandlerParms, Params, sizeof(pChip->i32AtAudHandlerParms[0])*count);
				return 0;
			}
		case AT_AUD_CTL_DBG_LEVEL:
			gAudioDebugLevel = Params[0];
			return 0;
		case AT_AUD_CTL_HANDLER:
			break;//go next ..
		default:
			return -1;//unlikely
				
	}

	switch(Params[0])
	{

		case AT_AUD_HANDLER_MODE:
			return AtMaudMode(pChip,ParamCount-1, &Params[1]);
		case AT_AUD_HANDLER_VOL:
			return AtMaudVol(pChip,ParamCount-1, &Params[1]);
		case AT_AUD_HANDLER_TST:
			return AtMaudTst(pChip,ParamCount-1, &Params[1]);
		case AT_AUD_HANDLER_LOG:
			return AtMaudLog(pChip,ParamCount-1, &Params[1]);
		case AT_AUD_HANDLER_LBTST:
			return AtMaudLoopback(pChip,ParamCount-1, &Params[1]);
		default:
			BCM_AUDIO_DEBUG("%s Unsupported handler %d \n", __FUNCTION__, Params[0]);		
			break;
	}

	return -1;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//Description:
//	Kernel AT command handler and gernal purpose controls
//Parameters
//	cmdIndex -- command index coresponding to AT comand
//	ParamCount	 -- count of array Params
//	Params -- Parameter array
//Return
//   0 on success, -1 otherwise
//------------------------------------------------------------------------------------------
int	AtAudCtlHandler_get(Int32 cmdIndex, brcm_alsa_chip_t* pChip, Int32	ParamCount, Int32 *Params)
{
	int count = sizeof(pChip->i32AtAudHandlerParms)/sizeof(pChip->i32AtAudHandlerParms[0]);

	switch(cmdIndex)
	{
	
		case AT_AUD_CTL_HANDLER:
		case AT_AUD_CTL_INDEX:
			{
				if(count>ParamCount)
					count = ParamCount;
				memcpy(Params, pChip->i32AtAudHandlerParms, sizeof(pChip->i32AtAudHandlerParms[0])*count);
				if(cmdIndex==AT_AUD_CTL_INDEX)
					return 0;
				else
					break; //continue
			}
		case AT_AUD_CTL_DBG_LEVEL:
			Params[0] = gAudioDebugLevel;
			return 0;
		default:
			return -1;//unlikely
				
	}

	switch(Params[0])
	{
		case AT_AUD_HANDLER_MODE:
			return AtMaudMode(pChip,ParamCount-1, &Params[1]);
		case AT_AUD_HANDLER_VOL:
			return AtMaudVol(pChip,ParamCount-1, &Params[1]);
		case AT_AUD_HANDLER_TST:
			return AtMaudTst(pChip,ParamCount-1, &Params[1]);
		case AT_AUD_HANDLER_LOG:
			return AtMaudLog(pChip,ParamCount-1, &Params[1]);
		case AT_AUD_HANDLER_LBTST:
			return AtMaudLoopback(pChip,ParamCount-1, &Params[1]);			
		default:
			BCM_AUDIO_DEBUG("%s Unsupported handler %d \n", __FUNCTION__, Params[0]);		
			break;
	}

	return -1;
}

