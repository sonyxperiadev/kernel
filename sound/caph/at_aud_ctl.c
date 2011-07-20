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



int	AtAudCtlHandler_put(Int32 cmdIndex, brcm_alsa_chip_t* pChip, Int32	ParamCount, Int32 *Params)
{
	BCM_AUDIO_DEBUG("AT-AUD-put %d %d %d %d %d %d\n", Params[0],Params[1],Params[2], Params[3],Params[4],Params[5]);
	switch(cmdIndex)
	{
		case AT_AUD_DEBUG_LEVEL:
			gAudioDebugLevel = Params[0];
			return 0;
		case AT_AUD_PCG_MODE:	//at*maudmode 1 mode
			pChip->i32PCGMode = Params[0];
			break;
		case AT_AUD_PCG_VOL:   //at*maudvol 7  vol
			pChip->i32PCGVolume = Params[0];
			break;
		case AT_AUD_PCG_PA_VOL: ////at*maudtst 121 0 vol
			pChip->i32PCGPAVol = Params[0];
			break;
		case AT_AUD_PCG_PREAMP_VOL: ////at*maudtst 121 0 vol
			pChip->i32PCGPreAmpVol = Params[0];
			break;
		case AT_AUD_AUDIO_LOGGING:
			break;
 		default:
			BCM_AUDIO_DEBUG("AtAudCtlHandler_put Unsupported AT-AUD command %d \n", cmdIndex);		
			break;
	}
	return -1;
}


int	AtAudCtlHandler_get(Int32 cmdIndex, brcm_alsa_chip_t* pChip, Int32	ParamCount, Int32 *Params)
{
	switch(cmdIndex)
	{
		case AT_AUD_DEBUG_LEVEL:
			Params[0] = gAudioDebugLevel;
			break;
		case AT_AUD_PCG_MODE:	//at*maudmode 0
			Params[0] = pChip->i32PCGMode;
			break;
		case AT_AUD_PCG_VOL:   //at*maudvol 6
			Params[0] = pChip->i32PCGVolume;
			break;
		case AT_AUD_PCG_PA_VOL:   //no corresponding at command   xxat*maudtst 121 0
			Params[0] = pChip->i32PCGPAVol;
			break;
		case AT_AUD_PCG_PREAMP_VOL:   //no corresponding at command   xxat*maudtst 121 0
			Params[0] = pChip->i32PCGPreAmpVol;
			break;
		case AT_AUD_AUDIO_LOGGING:
			break;
		default:
			BCM_AUDIO_DEBUG("Unsupported AT-AUD command %d \n", Params[0]);		
			break;
	}

	BCM_AUDIO_DEBUG("AtAudCtlHandler_get cmd=%d AT-AUD-get %d %d %d %d %d %d\n", cmdIndex, Params[0],Params[1],Params[2], Params[3],Params[4],Params[5]);
	return 0;
}

