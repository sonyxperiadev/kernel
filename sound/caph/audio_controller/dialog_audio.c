/******************************************************************************
*
* Copyright 2011, 2012  Broadcom Corporation
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2 (the GPL),
*  available at
*
*      http://www.broadcom.com/licenses/GPLv2.php
*
*  with the following added to such license:
*
*  As a special exception, the copyright holders of this software give you
*  permission to link this software with independent modules, and to copy and
*  distribute the resulting executable under terms of your choice, provided
*  that you also meet, for each linked independent module, the terms and
*  conditions of the license of that module.
*  An independent module is a module which is not derived from this software.
*  The special exception does not apply to any modifications of the software.
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
******************************************************************************/

/**
*
* @file   dialog_audio.c
* @brief
*
******************************************************************************/
#ifdef THIRD_PARTY_PMU

#include "dialog_audio.h"

int dialog_audio_hs_poweron(void){
	return 1;
}

int dialog_audio_hs_poweroff(void){
	return 1;
}

int dialog_audio_hs_set_gain(int hspath, int hsgain)
{
	return 1;
}

int dialog_audio_ihf_poweron(void)
{
	return 1;
}

int dialog_audio_ihf_poweroff(void)
{
	return 1;
}

int dialog_audio_ihf_set_gain(int ihfgain)
{
	return 1;
}

int dialog_set_hs_preamp_gain(int gain)
{
	return 1;
}

int dialog_set_ihf_preamp_gain(int gain)
{
	return 1;
}

int dialog_set_hs_noise_gate(dialog_noise_gate_t noisegate)
{
	return 1;
}

int dialog_set_ihf_noise_gate(dialog_noise_gate_t noisegate)
{
	return 1;
}

int dialog_set_ihf_none_clip(dialog_spk_nonclip_t nonclip)
{
	return 1;
}

int dialog_set_ihf_pwr(dialog_spk_pwr_t power)
{
	return 1;
}

int dialog_turn_multicast_onoff(unsigned char flag)
{
	return 1;
}

#endif



