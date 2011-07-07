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


#ifndef __CAPH_COMMON_H__
#define __CAPH_COMMON_H__


#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/jiffies.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/wait.h>
#include <linux/delay.h>
#include <linux/moduleparam.h>
#include <linux/sched.h>
#include <linux/kernel.h>


#include <sound/core.h>
#include <sound/control.h>
#include <sound/pcm.h>
#include <sound/rawmidi.h>
#include <sound/initval.h>
#include <linux/wakelock.h>

#include "bcm_audio_devices.h"
#define BCM_AUDIO_DEBUG_ON
#if defined(BCM_AUDIO_DEBUG_ON)
#define BCM_AUDIO_DEBUG(args...)  if (gAudioDebugLevel) printk(args)
#define DEBUG(args...)  if (gAudioDebugLevel) printk(args)
#else
#define BCM_AUDIO_DEBUG(args...)
#define DEBUG(args...)
#endif

#define	MIXER_STREAM_FLAGS_CAPTURE	0x00000001
#define	MIXER_STREAM_FLAGS_CALL		0x00000002
#define	MIXER_STREAM_FLAGS_FM		0x00000004

#define	CAPH_MIXER_NAME_LENGTH		20	//Max length of a mixer name
#define	MIC_TOTAL_COUNT_FOR_USER	AUDCTRL_MIC_DIGI3
#define	CAPH_MAX_CTRL_LINES			((MIC_TOTAL_COUNT_FOR_USER>AUDCTRL_SPK_TOTAL_COUNT)?MIC_TOTAL_COUNT_FOR_USER:AUDCTRL_SPK_TOTAL_COUNT)
#define	CAPH_MAX_PCM_STREAMS		8



//Try to keep consistent with Android AudioSystem::audio_devices
typedef	enum audio_devices {
	// output devices
	DEVICE_OUT_EARPIECE = 0x1,
	DEVICE_OUT_SPEAKER = 0x2,
	DEVICE_OUT_WIRED_HEADSET = 0x4,
	DEVICE_OUT_WIRED_HEADPHONE = 0x8,
	DEVICE_OUT_BLUETOOTH_SCO = 0x10,
	DEVICE_OUT_BLUETOOTH_SCO_HEADSET = 0x20,
	DEVICE_OUT_BLUETOOTH_SCO_CARKIT = 0x40,
	DEVICE_OUT_BLUETOOTH_A2DP = 0x80,
	DEVICE_OUT_BLUETOOTH_A2DP_HEADPHONES = 0x100,
	DEVICE_OUT_BLUETOOTH_A2DP_SPEAKER = 0x200,
	DEVICE_OUT_AUX_DIGITAL = 0x400,
	DEVICE_OUT_DEFAULT = 0x8000,
	DEVICE_OUT_ALL = (DEVICE_OUT_EARPIECE | DEVICE_OUT_SPEAKER | DEVICE_OUT_WIRED_HEADSET |
			DEVICE_OUT_WIRED_HEADPHONE | DEVICE_OUT_BLUETOOTH_SCO | DEVICE_OUT_BLUETOOTH_SCO_HEADSET |
			DEVICE_OUT_BLUETOOTH_SCO_CARKIT | DEVICE_OUT_BLUETOOTH_A2DP | DEVICE_OUT_BLUETOOTH_A2DP_HEADPHONES |
			DEVICE_OUT_BLUETOOTH_A2DP_SPEAKER | DEVICE_OUT_AUX_DIGITAL | DEVICE_OUT_DEFAULT),
	DEVICE_OUT_ALL_A2DP = (DEVICE_OUT_BLUETOOTH_A2DP | DEVICE_OUT_BLUETOOTH_A2DP_HEADPHONES |
			DEVICE_OUT_BLUETOOTH_A2DP_SPEAKER),

	// input devices
	DEVICE_IN_COMMUNICATION = 0x10000,
	DEVICE_IN_AMBIENT = 0x20000,
	DEVICE_IN_BUILTIN_MIC = 0x40000,
	DEVICE_IN_BLUETOOTH_SCO_HEADSET = 0x80000,
	DEVICE_IN_WIRED_HEADSET = 0x100000,
	DEVICE_IN_AUX_DIGITAL = 0x200000,
	DEVICE_IN_VOICE_CALL = 0x400000,
	DEVICE_IN_BACK_MIC = 0x800000,
	DEVICE_IN_DEFAULT = 0x80000000,

	DEVICE_IN_ALL = (DEVICE_IN_COMMUNICATION | DEVICE_IN_AMBIENT | DEVICE_IN_BUILTIN_MIC |
			DEVICE_IN_BLUETOOTH_SCO_HEADSET | DEVICE_IN_WIRED_HEADSET | DEVICE_IN_AUX_DIGITAL |
			DEVICE_IN_VOICE_CALL | DEVICE_IN_BACK_MIC | DEVICE_IN_DEFAULT)
}AUDIO_DEVICES_t;

typedef	struct _TCtrl_Line
{
	Int8 strName[CAPH_MIXER_NAME_LENGTH];
	Int32 iVolume[2];
	Int32 iMute[2];
}TCtrl_Line, *PTCtrl_Line;



typedef	struct _TPcm_Stream_Ctrls
{
	Int32	 iFlags;
	Int32	 iTotalCtlLines;
	Int32	 iLineSelect[2];	//Multiple selection, For playback sink, one bit represent one sink; for capture source, 
	char strStreamName[CAPH_MIXER_NAME_LENGTH];
	TCtrl_Line	ctlLine[CAPH_MAX_CTRL_LINES];
	snd_pcm_uframes_t	 stream_hw_ptr;
	TIDChanOfDev	dev_prop;
	void   *pSubStream;	
	//Int32    drvHandle;
	
}TPcm_Stream_Ctrls, *PTPcm_Stream_Ctrls;


typedef struct brcm_alsa_chip
{
	struct snd_card *card;
	TPcm_Stream_Ctrls	streamCtl[CAPH_MAX_PCM_STREAMS];

	/* workqueue */
	struct work_struct work_play;
    struct work_struct work_capt;

	Int32	pi32LoopBackTestParam[3];	//loopback test
	Int32	iEnablePhoneCall;			//Eanble/disable audio path for phone call
	Int32	iMutePhoneCall[2];	//UL mute and DL mute			//Mute MIC for phone call
	Int32	pi32SpeechMixOption[2];//Sppech mixing option, 0x00 - none, 0x01 - Downlink, 0x02 - uplink, 0x03 - both
} brcm_alsa_chip_t;


void caphassert(const char *fcn, int line, const char *expr);
#define CAPH_ASSERT(e)      ((e) ? (void) 0 : caphassert(__func__, __LINE__, #e))


enum	CTL_STREAM_PANEL_t
{
	CTL_STREAM_PANEL_PCMOUT1=1,
	CTL_STREAM_PANEL_FIRST=CTL_STREAM_PANEL_PCMOUT1,
	CTL_STREAM_PANEL_PCMOUT2,	
	CTL_STREAM_PANEL_VOIPOUT,
	CTL_STREAM_PANEL_PCMIN,
	CTL_STREAM_PANEL_SPEECHIN,
	CTL_STREAM_PANEL_VOIPIN,
	CTL_STREAM_PANEL_VOICECALL,
	CTL_STREAM_PANEL_FM,
	CTL_STREAM_PANEL_LAST
};


#if 0
enum	CTL_DEV_LINE_t
{
	CTL_DEV_OUT_EARPIECE = 1,
	CTL_DEV_OUT_FIRST=CTL_DEV_OUT_EARPIECE,
	CTL_DEV_OUT_IHF,		
	CTL_DEV_OUT_HEADSET,
	CTL_DEV_OUT_I2STX,
	CTL_DEV_OUT_BTHFP,
	CTL_DEV_OUT_LAST=CTL_DEV_OUT_BTHFP,

	CTL_DEV_IN_HANDSET,
	CTL_DEV_IN_FIRST = 	CTL_DEV_IN_HANDSET,
	CTL_DEV_IN_HEADSET,
	CTL_DEV_IN_DIGITAL1,
	CTL_DEV_IN_DIGITAL2,
	CTL_DEV_IN_BTHFP,
	CTL_DEV_IN_LAST = CTL_DEV_IN_BTHFP
};
#endif

enum	CTL_FUNCTION_t
{
	CTL_FUNCTION_VOL = 1,
	CTL_FUNCTION_MUTE,
	CTL_FUNCTION_LOOPBACK_TEST,
	CTL_FUNCTION_PHONE_ENABLE,
	CTL_FUNCTION_PHONE_CALL_MIC_MUTE,
	CTL_FUNCTION_SPEECH_MIXING_OPTION,
	CTL_FUNCTION_FM_ENABLE,
	CTL_FUNCTION_FM_FORMAT,
};

#define	CAPH_CTL_PRIVATE(dev, line, function) ((dev)<<16|(line)<<8|(function))
#define	STREAM_OF_CTL(private)		(((private)>>16)&0xFF)
#define	DEV_OF_CTL(private)			(((private)>>8)&0xFF)
#define	FUNC_OF_CTL(private)		((private)&0xFF)



//variables
extern int gAudioDebugLevel;

//functions
extern int __devinit PcmDeviceNew(struct snd_card *card);
extern int __devinit ControlDeviceNew(struct snd_card *card);


#endif //__CAPH_COMMON_H__

