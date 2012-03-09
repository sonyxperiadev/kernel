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
* @file   extern_audio.c
* @brief
*
******************************************************************************/

#include <linux/kernel.h>
#include "audio_consts.h"
#include "extern_audio.h"
#include "linux/gpio.h"
#include "audio_trace.h"

#if (!defined(CONFIG_BCMPMU_AUDIO))

#define bcmpmu_audio_init() NULL
#define bcmpmu_hs_set_gain(a, b) NULL
#define bcmpmu_hs_power(a) NULL
#define bcmpmu_ihf_set_gain(a) NULL
#define bcmpmu_ihf_power(a) NULL
#define bcmpmu_audio_deinit() NULL
#define bcmpmu_hi_gain_mode_en(a) NULL

#else

#include <linux/broadcom/bcmpmu_audio.h>

struct PMU_AudioGainMapping_t {
	int gain_mB;
	unsigned int PMU_gain_enum;
};

/*** FYI: from kernel/include/linux/broadcom/bcm59055-audio.h
enum from include/linux/broadcom/bcmpmu_audio.h
***/

static struct PMU_AudioGainMapping_t hsPMUGainTable[PMU_HSGAIN_NUM] = {
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

static struct PMU_AudioGainMapping_t ihfPMUGainTable[PMU_IHFGAIN_NUM] = {
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

static char ihf_IsOn;
static char hs_IsOn;
static int hs_gain_l = -400; /* mB */
static int hs_gain_r = -400; /* mB */
static int ihf_gain = -400; /* mB */

#if defined(CONFIG_IHF_EXT_AMPLIFIER)
#define GPIO_IHF_EXT_AMP 28

/******************************************************************************
* Function Name: audio_gpio_output
*
* Description: Toggle gpio output pin
*
* Note: This is only required on some OEM hardware
*
******************************************************************************/
static void audio_gpio_output(int gpio_pin, int value)
{
	int rc = gpio_request(gpio_pin, "IHF_EXT_AMP");

	aTrace(LOG_AUDIO_CNTLR,
		"audio_gpio_output::gpio pin %d value %d, rc=0x%x\n",
		gpio_pin, value, rc);
	gpio_direction_output(gpio_pin, 0);
	gpio_set_value(gpio_pin, value);
	gpio_free(gpio_pin);
}
#else
#define audio_gpio_output(a, b)
#endif

/******************************************************************************
* Function Name: map2pmu_hs_gain
*
* Description:   convert Headset gain mB value to PMU gain enum
*
* Note: input gain is in mB.
*
******************************************************************************/
static struct PMU_AudioGainMapping_t map2pmu_hs_gain(int arg_gain_mB)
{
	int i = 0;

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
static struct PMU_AudioGainMapping_t map2pmu_ihf_gain(int arg_gain_mB)
{
	int i = 0;

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
*  @brief  power on external headset amplifier
*
*  @param  none
*  @return none
*
****************************************************************************/
void extern_hs_on(void)
{
	/*enable the audio PLL before power ON */
	bcmpmu_audio_init();

	bcmpmu_hs_set_gain(PMU_AUDIO_HS_BOTH,
				  PMU_HSGAIN_MUTE),

	/*
	./drivers/misc/bcm59055-audio.c:int bcm59055_hs_power(bool on)
	drivers/misc/bcm59055-audio.c
	drivers/misc/bcmpmu_audio.c
	*/

	bcmpmu_hs_power(1);

	hs_IsOn = 1;
}

/********************************************************************
*  @brief  power off external headset amplifier
*
*  @param  none
*  @return none
*
****************************************************************************/
void extern_hs_off(void)
{
	bcmpmu_hs_set_gain(PMU_AUDIO_HS_BOTH,
				  PMU_HSGAIN_MUTE),
	bcmpmu_hs_power(0);

	hs_IsOn = 0;

	if (ihf_IsOn == 0 && hs_IsOn == 0)
		/*disable the audio PLL after power OFF*/
		bcmpmu_audio_deinit();
}

/********************************************************************
*  @brief  power on external IHF (loud speaker) amplifier
*
*  @param  none
*  @return none
*
****************************************************************************/
void extern_ihf_on(void)
{
#if defined(CONFIG_IHF_EXT_AMPLIFIER)
	audio_gpio_output(GPIO_IHF_EXT_AMP, 1);
#else
	/*enable the audio PLL before power ON */
	bcmpmu_audio_init();

	bcmpmu_ihf_set_gain(PMU_IHFGAIN_MUTE),
	bcmpmu_ihf_power(1);
#endif
	ihf_IsOn = 1;
}

/********************************************************************
*  @brief  power off external IHF (loud speaker) amplifier
*
*  @param  none
*  @return none
*
****************************************************************************/
void extern_ihf_off(void)
{
	ihf_IsOn = 0;
#if defined(CONFIG_IHF_EXT_AMPLIFIER)
	audio_gpio_output(GPIO_IHF_EXT_AMP, 0);
#else
	bcmpmu_ihf_set_gain(PMU_IHFGAIN_MUTE),
	bcmpmu_ihf_power(0);

	if (ihf_IsOn == 0 && hs_IsOn == 0)
		/*disable the audio PLL after power OFF*/
		bcmpmu_audio_deinit();
#endif
}



/********************************************************************
*  @brief  Find the actual headset gain (mB) that external audio chip can support
*
*  @param  gain_mB	requested Headset gain
*  @return  int			headset gain in external audio chip. (mB value)
*
****************************************************************************/
int extern_hs_find_gain(int gain_mB)
{
	struct PMU_AudioGainMapping_t gain_map;

	gain_map = map2pmu_hs_gain(gain_mB);
	aTrace(LOG_AUDIO_CNTLR, "%s need %d, find %d\n", __func__,
			gain_mB, gain_map.gain_mB);

	return gain_map.gain_mB;
}

/********************************************************************
*  @brief  Set headset gain (mB) on left-channel on external audio chip
*
*  @param  gain_mB	requested Headset gain
*  @return  none
*
****************************************************************************/
void extern_hs_set_gain(int gain_mB, AUDIO_GAIN_LR_t lr)
{
	struct PMU_AudioGainMapping_t gain_map;

	gain_map = map2pmu_hs_gain(gain_mB);
	aTrace(LOG_AUDIO_CNTLR, "%s need %d, pmu_gain_enum=%d\n",
			__func__, gain_mB, gain_map.PMU_gain_enum);

	if (lr == AUDIO_HS_BOTH) {
		bcmpmu_hs_set_gain(PMU_AUDIO_HS_BOTH,
			gain_map.PMU_gain_enum);
		hs_gain_l = gain_map.gain_mB;
		hs_gain_r = gain_map.gain_mB;
	} else if (lr == AUDIO_HS_LEFT) {
		bcmpmu_hs_set_gain(PMU_AUDIO_HS_LEFT,
			gain_map.PMU_gain_enum);
		hs_gain_l = gain_map.gain_mB;
	} else if (lr == AUDIO_HS_RIGHT) {
		bcmpmu_hs_set_gain(PMU_AUDIO_HS_RIGHT,
			gain_map.PMU_gain_enum);
		hs_gain_r = gain_map.gain_mB;
	}
}

/********************************************************************
*  @brief  Mute Headset left-channel gain
*
*  @param  none
*  @return none
*
****************************************************************************/
void extern_hs_mute(AUDIO_GAIN_LR_t lr)
{
	if (lr == AUDIO_HS_BOTH)
		bcmpmu_hs_set_gain(PMU_AUDIO_HS_BOTH, PMU_HSGAIN_MUTE);
	else
	if (lr == AUDIO_HS_LEFT)
		bcmpmu_hs_set_gain(PMU_AUDIO_HS_LEFT, PMU_HSGAIN_MUTE);
	else
	if (lr == AUDIO_HS_RIGHT)
		bcmpmu_hs_set_gain(PMU_AUDIO_HS_RIGHT, PMU_HSGAIN_MUTE);
}

/********************************************************************
*  @brief  un-mute Headset left-channel gain
*
*  @param  none
*  @return none
*
****************************************************************************/
void extern_hs_unmute(AUDIO_GAIN_LR_t lr)
{
	if (lr == AUDIO_HS_BOTH)
		extern_hs_set_gain(hs_gain_l, AUDIO_HS_BOTH);
	else
	if (lr == AUDIO_HS_LEFT)
		extern_hs_set_gain(hs_gain_l, AUDIO_HS_LEFT);
	else
	if (lr == AUDIO_HS_RIGHT)
		extern_hs_set_gain(hs_gain_r, AUDIO_HS_RIGHT);
}


/********************************************************************
*  @brief  Find the actual IHF gain (mB) that external audio chip can support
*
*  @param  gain_mB	requested IHF amp gain
*  @return  int			IHF amp gain in external audio chip. (mB value)
*
****************************************************************************/
int extern_ihf_find_gain(int gain_mB)
{
	struct PMU_AudioGainMapping_t gain_map;

	gain_map = map2pmu_ihf_gain(gain_mB);
	aTrace(LOG_AUDIO_CNTLR, "%s need %d, find %d\n",
			__func__, gain_mB, gain_map.gain_mB);

	return gain_map.gain_mB;
}

/********************************************************************
*  @brief  Set IHF gain (mB) on external audio chip
*
*  @param  gain_mB	requested IHF amp gain
*  @return  none
*
****************************************************************************/
void extern_ihf_set_gain(int gain_mB)
{
	struct PMU_AudioGainMapping_t gain_map;

	gain_map = map2pmu_ihf_gain(gain_mB);
	aTrace(LOG_AUDIO_CNTLR, "%s gain_mB=%d, pmu_gain_enum=%d\n",
			__func__, gain_mB, gain_map.PMU_gain_enum);

	bcmpmu_ihf_set_gain(gain_map.PMU_gain_enum);

	ihf_gain = gain_map.gain_mB;
}


/********************************************************************
*  @brief  Mute IHF gain in PMU
*
*  @param  none
*  @return none
*
****************************************************************************/
void extern_ihf_mute(void)
{
	bcmpmu_ihf_set_gain(PMU_IHFGAIN_MUTE);
}

/********************************************************************
*  @brief  un-mute IHF gain in PMU
*
*  @param  none
*  @return none
*
****************************************************************************/
void extern_ihf_unmute(void)
{
	extern_ihf_set_gain(ihf_gain);
}

/********************************************************************
*  @brief  Set IHF gain shift mode on external audio chip
*
*  @param  enable	 1 - enable, 0 - disable
*  @return  none
*
****************************************************************************/
void extern_ihf_en_hi_gain_mode(int enable)
{
	if (0 == enable)
		bcmpmu_hi_gain_mode_en(0);
	else
		bcmpmu_hi_gain_mode_en(1);
}

#endif
