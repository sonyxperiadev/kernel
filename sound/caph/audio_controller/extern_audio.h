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
* @file   extern_audio.h
* @brief  adaptation for various external devices
*
******************************************************************************/

#ifndef __EXTERN_AUDIO_H__
#define __EXTERN_AUDIO_H__

/********************************************************************
*  @brief  power on external headset amplifier
*
*  @param  none
*  @return none
*
****************************************************************************/
void extern_hs_on(void);

/********************************************************************
*  @brief  power off external headset amplifier
*
*  @param  none
*  @return none
*
****************************************************************************/
void extern_hs_off(void);

/********************************************************************
*  @brief  power on external IHF (loud speaker) amplifier
*
*  @param  none
*  @return none
*
****************************************************************************/
void extern_ihf_on(void);

/********************************************************************
*  @brief  power off external IHF (loud speaker) amplifier
*
*  @param  none
*  @return none
*
****************************************************************************/
void extern_ihf_off(void);

/********************************************************************
*  @brief  power on external stereo speaker amplifier and switches
*
*  @param  none
*  @return none
*
****************************************************************************/
void extern_stereo_speaker_on(void);

/********************************************************************
*  @brief  power off external stereo speaker and switches
*
*  @param  none
*  @return none
*
****************************************************************************/
void extern_stereo_speaker_off(void);




/********************************************************************
*  @brief  Find the actual headset gain (mB) that external audio chip can support
*
*  @param  gain_mB	requested Headset gain
*  @return  int			headset gain in external audio chip. (mB value)
*
****************************************************************************/
int extern_hs_find_gain(int gain_mB);

/********************************************************************
*  @brief  Set headset gain (mB) on external audio chip
*
*  @param  gain_mB	requested Headset gain
*  @return  none
*
****************************************************************************/
void extern_hs_set_gain(int gain_mB, AUDIO_GAIN_LR_t lr);

/********************************************************************
*  @brief  Mute Headset gain
*
*  @param  lr   left_right
*  @return none
*
****************************************************************************/
void extern_hs_mute(AUDIO_GAIN_LR_t lr);

/********************************************************************
*  @brief  un-mute Headset gain
*
*  @param  lr   left_right
*  @return none
*
****************************************************************************/
void extern_hs_unmute(AUDIO_GAIN_LR_t lr);


/********************************************************************
*  @brief  Find the actual IHF gain (mB) that external audio chip can support
*
*  @param  gain_mB	requested IHF amp gain
*  @return  int			IHF amp gain in external audio chip. (mB value)
*
****************************************************************************/
int extern_ihf_find_gain(int gain_mB);

/********************************************************************
*  @brief  Set IHF gain (mB) on external audio chip
*
*  @param  gain_mB	requested IHF amp gain
*  @return  none
*
****************************************************************************/
void extern_ihf_set_gain(int gain_mB);

/********************************************************************
*  @brief  Mute IHF gain in PMU
*
*  @param  none
*  @return none
*
****************************************************************************/
void extern_ihf_mute(void);

/********************************************************************
*  @brief  un-mute IHF gain in PMU
*
*  @param  none
*  @return none
*
****************************************************************************/
void extern_ihf_unmute(void);

/********************************************************************
*  @brief  Set IHF gain shift mode on external audio chip
*
*  @param  enable	 1 - enable, 0 - disable
*  @return  none
*
****************************************************************************/
void extern_ihf_en_hi_gain_mode(int enable);

/********************************************************************
*  @brief  Set IHF Noise gate parameter
*
*  @param  IHF Noise parameter
*  @return  none
*
****************************************************************************/


void extern_set_ihf_noise_gate(int param_value);


/********************************************************************
*  @brief  Set HS Noise gate parameter
*
*  @param  HS Noise parameter
*  @return  none
*
****************************************************************************/


void extern_set_hs_noise_gate(int param_value);

/********************************************************************
*  @brief  Set IHF None Clip parameter
*
*  @param  None Clip parameter
*  @return  none
*
****************************************************************************/

void extern_set_ihf_none_clip(int param_value);


/********************************************************************
*  @brief  Set IHF pwr parameter
*
*  @param  pwr parameter
*  @return  none
*
****************************************************************************/

void extern_set_ihf_pwr(int param_value);

/********************************************************************
*  @brief  Set IHF preamp gain
*
*  @param  gain_mB	requested IHF preamp gain
*  @return  none
*
****************************************************************************/

void extern_set_ihf_preamp_gain(int gain_mB);

/********************************************************************
*  @brief  Set HS preamp gain
*
*  @param  gain_mB	requested HS preamp gain
*  @return  none
*
****************************************************************************/

void extern_set_hs_preamp_gain(int gain_mB);

/********************************************************************
*  @brief  Set Multicast on/off on PMU
*
*  @param  flag	IHF on/off for multicasting
*  @return  none
*
****************************************************************************/
void extern_start_stop_multicast(unsigned char flag);

#if defined(CONFIG_MFD_BCM59039) | defined(CONFIG_MFD_BCM59042)
/********************************************************************
*  @brief  Get headset gain (mB)
*
*  @param  gain_mB	returned Headset gain
*  @return  none
*
****************************************************************************/
void extern_hs_get_gain(int *pGainL_mB, int *pGainR_mB);
#endif
#endif
