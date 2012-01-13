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
* @file   audio_pmu_adapt.c
* @brief
*
******************************************************************************/

#if !defined(NO_PMU) && defined(CONFIG_BCMPMU_AUDIO)

/** Include directives */

#include "mobcom_types.h"
#include "resultcode.h"

#include <linux/kernel.h>
#include "audio_pmu_adapt.h"

/** Public Variable declarations */

/** Private Type and Constant declarations */

/*** FYI: from kernel/include/linux/broadcom/bcm59055-audio.h
enum
{
	PMU_HSGAIN_MUTE,
	PMU_HSGAIN_66DB_N,
	PMU_HSGAIN_63DB_N,
...
	PMU_HSGAIN_2P5DB_N,
	PMU_HSGAIN_2DB_N,
	PMU_HSGAIN_NUM
};

enum
{
	PMU_IHFGAIN_MUTE,
	PMU_IHFGAIN_60DB_N,
	PMU_IHFGAIN_57DB_N,
...
	PMU_IHFGAIN_3DB_P,
	PMU_IHFGAIN_3P5DB_P,
	PMU_IHFGAIN_4DB_P,
    PMU_IHFGAIN_NUM
};
***/

static PMU_AudioGainMapping_t hsPMUGainTable[PMU_HSGAIN_NUM] = {
	/* Gain in milli Bel,      HS PMU Gain enum */
	{-10000, PMU_HSGAIN_MUTE},
	{-6600, PMU_HSGAIN_66DB_N},
	{-6300, PMU_HSGAIN_63DB_N},
	{-6000, PMU_HSGAIN_60DB_N},
	{-5700, PMU_HSGAIN_57DB_N},
	{-5400, PMU_HSGAIN_54DB_N},
	{-5100, PMU_HSGAIN_51DB_N},
	{-4800, PMU_HSGAIN_48DB_N},
	{-4500, PMU_HSGAIN_45DB_N},
	{-4200, PMU_HSGAIN_42DB_N},
	{-4000, PMU_HSGAIN_40P5DB_N},	/*10*/
	{-3900, PMU_HSGAIN_39DB_N},
	{-3750, PMU_HSGAIN_37P5DB_N},
	{-3600, PMU_HSGAIN_36DB_N},
	{-3450, PMU_HSGAIN_34P5DB_N},
	{-3300, PMU_HSGAIN_33DB_N},
	{-3150, PMU_HSGAIN_31P5DB_N},
	{-3000, PMU_HSGAIN_30DB_N},
	{-2850, PMU_HSGAIN_28P5DB_N},
	{-2700, PMU_HSGAIN_27DB_N},
	{-2550, PMU_HSGAIN_25P5DB_N},	/*20*/
	{-2400, PMU_HSGAIN_24DB_N},
	{-2250, PMU_HSGAIN_22P5DB_N},
	{-2200, PMU_HSGAIN_22DB_N},
	{-2150, PMU_HSGAIN_21P5DB_N},
	{-2100, PMU_HSGAIN_21DB_N},
	{-2050, PMU_HSGAIN_20P5DB_N},
	{-2000, PMU_HSGAIN_20DB_N},
	{-1950, PMU_HSGAIN_19P5DB_N},
	{-1900, PMU_HSGAIN_19DB_N},
	{-1850, PMU_HSGAIN_18P5DB_N},	/*30*/
	{-1800, PMU_HSGAIN_18DB_N},
	{-1750, PMU_HSGAIN_17P5DB_N},
	{-1700, PMU_HSGAIN_17DB_N},
	{-1650, PMU_HSGAIN_16P5DB_N},
	{-1600, PMU_HSGAIN_16DB_N},
	{-1550, PMU_HSGAIN_15P5DB_N},
	{-1500, PMU_HSGAIN_15DB_N},
	{-1450, PMU_HSGAIN_14P5DB_N},
	{-1400, PMU_HSGAIN_14DB_N},
	{-1350, PMU_HSGAIN_13P5DB_N},	/*40*/
	{-1300, PMU_HSGAIN_13DB_N},
	{-1250, PMU_HSGAIN_12P5DB_N},
	{-1200, PMU_HSGAIN_12DB_N},
	{-1150, PMU_HSGAIN_11P5DB_N},
	{-1100, PMU_HSGAIN_11DB_N},
	{-1050, PMU_HSGAIN_10P5DB_N},
	{-1000, PMU_HSGAIN_10DB_N},
	{-950, PMU_HSGAIN_9P5DB_N},
	{-900, PMU_HSGAIN_9DB_N},
	{-850, PMU_HSGAIN_8P5DB_N},	/*50*/
	{-800, PMU_HSGAIN_8DB_N},
	{-750, PMU_HSGAIN_7P5DB_N},
	{-700, PMU_HSGAIN_7DB_N},
	{-650, PMU_HSGAIN_6P5DB_N},
	{-600, PMU_HSGAIN_6DB_N},
	{-550, PMU_HSGAIN_5P5DB_N},
	{-500, PMU_HSGAIN_5DB_N},
	{-450, PMU_HSGAIN_4P5DB_N},
	{-400, PMU_HSGAIN_4DB_N},
	{-350, PMU_HSGAIN_3P5DB_N},	/*60*/
	{-300, PMU_HSGAIN_3DB_N},
	{-250, PMU_HSGAIN_2P5DB_N},
	{-200, PMU_HSGAIN_2DB_N}	/*63*/
};

static PMU_AudioGainMapping_t ihfPMUGainTable[PMU_IHFGAIN_NUM] = {
	/* Gain in milli Bel,          IHF PMU Gain enum */
	{-10000, PMU_IHFGAIN_MUTE},
	{-6000, PMU_IHFGAIN_60DB_N},
	{-5700, PMU_IHFGAIN_57DB_N},
	{-5400, PMU_IHFGAIN_54DB_N},
	{-5100, PMU_IHFGAIN_51DB_N},
	{-4800, PMU_IHFGAIN_48DB_N},
	{-4500, PMU_IHFGAIN_45DB_N},
	{-4200, PMU_IHFGAIN_42DB_N},
	{-3900, PMU_IHFGAIN_39DB_N},
	{-3600, PMU_IHFGAIN_36DB_N},
	{-3450, PMU_IHFGAIN_34P5DB_N},	/*10*/
	{-3300, PMU_IHFGAIN_33DB_N},
	{-3150, PMU_IHFGAIN_31P5DB_N},
	{-3000, PMU_IHFGAIN_30DB_N},
	{-2850, PMU_IHFGAIN_28P5DB_N},
	{-2700, PMU_IHFGAIN_27DB_N},
	{-2550, PMU_IHFGAIN_25P5DB_N},
	{-2400, PMU_IHFGAIN_24DB_N},
	{-2250, PMU_IHFGAIN_22P5DB_N},
	{-2100, PMU_IHFGAIN_21DB_N},
	{-1950, PMU_IHFGAIN_19P5DB_N},	/*20*/
	{-1800, PMU_IHFGAIN_18DB_N},
	{-1650, PMU_IHFGAIN_16P5DB_N},
	{-1600, PMU_IHFGAIN_16DB_N},
	{-1550, PMU_IHFGAIN_15P5DB_N},
	{-1500, PMU_IHFGAIN_15DB_N},
	{-1450, PMU_IHFGAIN_14P5DB_N},
	{-1400, PMU_IHFGAIN_14DB_N},
	{-1350, PMU_IHFGAIN_13P5DB_N},
	{-1300, PMU_IHFGAIN_13DB_N},
	{-1250, PMU_IHFGAIN_12P5DB_N},	/*30*/
	{-1200, PMU_IHFGAIN_12DB_N},
	{-1150, PMU_IHFGAIN_11P5DB_N},
	{-1100, PMU_IHFGAIN_11DB_N},
	{-1050, PMU_IHFGAIN_10P5DB_N},
	{-1000, PMU_IHFGAIN_10DB_N},
	{-950, PMU_IHFGAIN_9P5DB_N},
	{-900, PMU_IHFGAIN_9DB_N},
	{-850, PMU_IHFGAIN_8P5DB_N},
	{-800, PMU_IHFGAIN_8DB_N},
	{-750, PMU_IHFGAIN_7P5DB_N},	/*40*/
	{-700, PMU_IHFGAIN_7DB_N},
	{-650, PMU_IHFGAIN_6P5DB_N},
	{-600, PMU_IHFGAIN_6DB_N},
	{-550, PMU_IHFGAIN_5P5DB_N},
	{-500, PMU_IHFGAIN_5DB_N},
	{-450, PMU_IHFGAIN_4P5DB_N},
	{-400, PMU_IHFGAIN_4DB_N},
	{-350, PMU_IHFGAIN_3P5DB_N},
	{-300, PMU_IHFGAIN_3DB_N},
	{-250, PMU_IHFGAIN_2P5DB_N},	/*50*/
	{-200, PMU_IHFGAIN_2DB_N},
	{-150, PMU_IHFGAIN_1P5DB_N},
	{-100, PMU_IHFGAIN_1DB_N},
	{-50, PMU_IHFGAIN_P5DB_N},
	{0, PMU_IHFGAIN_0DB},	/*55*/
	{50, PMU_IHFGAIN_P5DB_P},
	{100, PMU_IHFGAIN_1DB_P},
	{150, PMU_IHFGAIN_1P5DB_P},
	{200, PMU_IHFGAIN_2DB_P},
	{250, PMU_IHFGAIN_2P5DB_P},	/*60*/
	{300, PMU_IHFGAIN_3DB_P},
	{350, PMU_IHFGAIN_3P5DB_P},
	{400, PMU_IHFGAIN_4DB_P}
};

/******************************************************************************
* Function Name: map2pmu_hs_gain
*
* Description:   convert Headset gain mB value to PMU gain enum
*
* Note: input gain is in mB.
*
******************************************************************************/
PMU_AudioGainMapping_t map2pmu_hs_gain(int arg_gain_mB)
{
	UInt8 i = 0;

	if (arg_gain_mB < hsPMUGainTable[1].gain_mB)
		return hsPMUGainTable[0];
	else if (arg_gain_mB >= hsPMUGainTable[PMU_HSGAIN_NUM - 1].gain_mB)
		return hsPMUGainTable[PMU_HSGAIN_NUM - 1];

	for (i = 1; i < PMU_HSGAIN_NUM; i++) {
		if (arg_gain_mB <= hsPMUGainTable[i].gain_mB)
			return hsPMUGainTable[i];
	}

	/*Should not run to here.*/
	return hsPMUGainTable[PMU_HSGAIN_NUM - 1];

}

/******************************************************************************
* Function Name: map2pmu_ihf_gain
*
* Description:   convert IHF gain mB value to PMU gain enum
*
* Note: input gain is in mB.
*
******************************************************************************/
PMU_AudioGainMapping_t map2pmu_ihf_gain(int arg_gain_mB)
{
	UInt8 i = 0;

	if (arg_gain_mB < ihfPMUGainTable[1].gain_mB)
		return ihfPMUGainTable[0];
	else if (arg_gain_mB >= ihfPMUGainTable[PMU_IHFGAIN_NUM - 1].gain_mB)
		return ihfPMUGainTable[PMU_IHFGAIN_NUM - 1];

	for (i = 1; i < PMU_IHFGAIN_NUM; i++) {
		if (arg_gain_mB <= ihfPMUGainTable[i].gain_mB)
			return ihfPMUGainTable[i];
	}

	/*Should not run to here.*/
	return ihfPMUGainTable[PMU_IHFGAIN_NUM - 1];

}

/********************************************************************
*  @brief  Mute Headset gain in PMU
*
*  @param  none
*
*  @return none
*
****************************************************************************/
void pmu_hs_mute(unsigned int left_right)
{
	AUDIO_PMU_HS_SET_GAIN(left_right, PMU_HSGAIN_MUTE);
}

/********************************************************************
*  @brief  Mute IHF gain in PMU
*
*  @param  none
*
*  @return none
*
****************************************************************************/
void pmu_ihf_mute(void)
{
	AUDIO_PMU_IHF_SET_GAIN(PMU_IHFGAIN_MUTE);
}

#endif
