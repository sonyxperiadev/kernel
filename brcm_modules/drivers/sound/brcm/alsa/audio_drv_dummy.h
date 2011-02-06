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

/*
*
*   brief  This file contains dummy audio driver. This can be used for bare baseband build
*
****************************************************************************/

#include "audio_drv.h"

#ifndef _AUDIO_DRV_DUMMY__H
#define _AUDIO_DRV_DUMMY__H


#if defined(GOLDFINGER_B1) || defined(VOYAGER) || defined(MARTINI_HW) || defined(MARTINI_HW_EP3)
#include "platform_config.h"
#endif

void AudioExtDev_WriteReg( UInt8 register_add, UInt16 value)
{
	return;
}

void AudioExtDev_Init( void)
{
#if defined(GOLDFINGER_B1) || defined(VOYAGER) || defined(MARTINI_HW) || defined(MARTINI_HW_EP3)
	GPIO_ConfigOutput( GET_GPIO_PIN_MASK( HAL_HACEN ) );
#endif
	return;
}

void AudioExtDev_SetInputFormat(UInt32 format)
{

}

void AudioExtDev_SetBypass(UInt32 bypass)
{

}

#if !defined(BCM2153_FAMILY_BB_CHIP_BOND) || defined(BUILD_BONDRAY)

void AudioExtDev_PowerOn(AudioMode_t amode)
{
#if defined(GOLDFINGER_B1) || defined(VOYAGER) || defined(MARTINI_HW) || defined(MARTINI_HW_EP3)
	if (pathCfgPtr->outputChnl == AUDIO_CHNL_HAC)
	{
		GPIO_SET_HI( HAL_HACEN );
	}
	else
	{
		GPIO_SET_LO( HAL_HACEN );
	}
#endif
	return;
}


void AudioExtDev_PowerOff( )
{
#if defined(GOLDFINGER_B1) || defined(VOYAGER) || defined(MARTINI_HW) || defined(MARTINI_HW_EP3)
	GPIO_SET_LO( HAL_HACEN );
#endif
	return;
}
#endif

#if !defined(BCM2153_FAMILY_BB_CHIP_BOND) || defined(BUILD_BONDRAY)
void AudioExtDev_SetVolume(UInt8 extVol)
{
	return;
}
#endif

Boolean AudioExtDev_IsReady( void )
{
	return 1;
}

void AudioExtDev_SetSampleRate( AudioSampleRate_en_t sample_rate )
{
	return;
}

void AudioExtDev_SetBiasOnOff(AudioMode_t audioMode, Boolean OnOff)
{
	switch(audioMode)
	{
		case AUDIO_MODE_HEADSET:
			break;

		default :
			break;
	}

}

#if defined(BUILD_BONDRAY)
void AudioExtDev_SetPreGain(AudioExtPreGainMAX9877_t pregain)
{
	/*Dummy function to take care of bondray build*/
}
#endif

#endif //#ifndef _AUDIO_DRV_DUMMY__H
