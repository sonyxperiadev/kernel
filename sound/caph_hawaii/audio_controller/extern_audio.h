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

#if defined(CONFIG_IHF_TWO_EXT_AMPLIFIER)

/********************************************************************
*  @brief  power on two external IHF (loud speaker) amplifiers
*
*  @param  none
*  @return none
*
****************************************************************************/
void extern_ihf_two_external_amplifier_on(int isVoiceCall);

/********************************************************************
*  @brief  power off two external IHF (loud speaker) amplifiers
*
*  @param  none
*  @return none
*
****************************************************************************/
void extern_ihf_two_external_amplifier_off(int isVoiceCall);

#endif /* CONFIG_IHF_TWO_EXT_AMPLIFIER */

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
*  @brief  Set HS gain shift mode on external audio chip
*
*  @param  enable	 1 - enable, 0 - disable
*  @return  none
*
****************************************************************************/
void extern_hs_en_hi_gain_mode(int enable);

/********************************************************************
*  @brief  Set IHF gain shift mode on external audio chip
*
*  @param  enable	 1 - enable, 0 - disable
*  @return  none
*
****************************************************************************/
void extern_ihf_en_hi_gain_mode(int enable);

/********************************************************************
*  @brief  Set the platform configuration for external audio layer
*
*  @param  cfg - configuration structure
*  @return  none
*
****************************************************************************/
void extern_audio_platform_cfg_set(void *cfg);

/********************************************************************
*  @brief  Set the GPIO pin value to route the audio to the dock
*
*  @param  Toggle value of the GPIO pin.  1 - High, 0 - Low
*  @return  none
*
****************************************************************************/
void extern_dock_audio_route(int gpio_val);

/* PMU ALC support */
/********************************************************************
*  @brief  enable or disable PMU ALC
*
*  @param  on      1 - enable, 0 - disable
*  @return  none
*
****************************************************************************/
void extern_ihf_set_alc_enable(int on);

/********************************************************************
*  @brief  set ALC reference from VBAT or from VLDO
*
*  @param  on      1 - VBAT, 0 - VLDO
*  @return  none
*
****************************************************************************/
void extern_ihf_set_alc_vbat_ref(int on);

/********************************************************************
*  @brief  set ALC threshold at comparator input (and THD at the load)
*
*  @param  thld      0 - 1% THD
*                    1 - 3% THD
*                    2 - 5% THD
*                    3 - 10% THD
*                    0xF - 0.1% THD
*  @return  none
*
****************************************************************************/
void extern_ihf_set_alc_thld(int thld);

/********************************************************************
*  @brief  control ALC ramp up time
*
*  @param  ctrl      0 - 20 ms
*                    1 - 200 ms
*                    2 - 1 s
*                    3 - 4 s
*  @return  none
*
****************************************************************************/
void extern_ihf_set_alc_ramp_up_ctrl(int ctrl);

/********************************************************************
*  @brief  control ALC ramp down time
*
*  @param  ctrl      0 - 100 us
*                    1 - 500 us
*                    2 - 1 ms
*  @return  none
*
****************************************************************************/
void extern_ihf_set_alc_ramp_down_ctrl(int ctrl);

#endif
