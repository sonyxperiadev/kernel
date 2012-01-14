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
* @file   audio_pmu_adapt.h
* @brief  Audio PMU adaptation for various PMU devices
*
******************************************************************************/

#ifndef __AUDIO_PMU_ADAPT_H__
#define __AUDIO_PMU_ADAPT_H__

#if (!defined(CONFIG_BCMPMU_AUDIO))

#include <linux/broadcom/bcmpmu_audio.h>
#define AUDIO_PMU_INIT() NULL
#define AUDIO_PMU_HS_SET_GAIN(a, b) NULL
#define AUDIO_PMU_HS_POWER(a) NULL
#define AUDIO_PMU_IHF_SET_GAIN(a) NULL
#define AUDIO_PMU_IHF_POWER(a) NULL
#define AUDIO_PMU_DEINIT() NULL
#define AUDIO_PMU_HI_GAIN_MODE_EN(a) NULL

#else
#ifdef CONFIG_BCM59055_AUDIO

#include "linux/broadcom/bcm59055-audio.h"
#define AUDIO_PMU_INIT bcm59055_audio_init
#define AUDIO_PMU_HS_SET_GAIN bcm59055_hs_set_gain
#define AUDIO_PMU_HS_POWER bcm59055_hs_power
#define AUDIO_PMU_IHF_SET_GAIN bcm59055_ihf_set_gain
#define AUDIO_PMU_IHF_POWER bcm59055_ihf_power
#define AUDIO_PMU_DEINIT bcm59055_audio_deinit
#define AUDIO_PMU_HI_GAIN_MODE_EN NULL

#else

#include <linux/broadcom/bcmpmu_audio.h>
#define AUDIO_PMU_INIT bcmpmu_audio_init
#define AUDIO_PMU_HS_SET_GAIN bcmpmu_hs_set_gain
#define AUDIO_PMU_HS_POWER bcmpmu_hs_power
#define AUDIO_PMU_IHF_SET_GAIN bcmpmu_ihf_set_gain
#define AUDIO_PMU_IHF_POWER bcmpmu_ihf_power
#define AUDIO_PMU_DEINIT bcmpmu_audio_deinit
#define AUDIO_PMU_HI_GAIN_MODE_EN bcmpmu_hi_gain_mode_en

#endif

typedef struct {
	int gain_mB;
	unsigned int PMU_gain_enum;
} PMU_AudioGainMapping_t;

/********************************************************************
*  @brief  Convert Headset gain mB value to PMU gain enum
*
*  @param  Headset gain mB galue
*
*  @return PMU_HS_Gain_t PMU gain enum
*
****************************************************************************/
PMU_AudioGainMapping_t map2pmu_hs_gain(int gain_mB);

/********************************************************************
*  @brief  Convert IHF gain mB value to PMU gain enum
*
*  @param  IHF gain mB galue
*
*  @return PMU_HS_Gain_t PMU gain enum
*
****************************************************************************/
PMU_AudioGainMapping_t map2pmu_ihf_gain(int gain_mB);

/********************************************************************
*  @brief  Mute Headset gain in PMU
*
*  @param  none
*
*  @return none
*
****************************************************************************/
void pmu_hs_mute(unsigned int left_right);

/********************************************************************
*  @brief  Mute IHF gain in PMU
*
*  @param  none
*
*  @return none
*
****************************************************************************/
void pmu_ihf_mute(void);

#endif

#endif /*__AUDIO_PMU_ADAPT_H__*/
