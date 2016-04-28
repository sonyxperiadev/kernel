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
#include <mach/caph_platform.h>
#include "audio_consts.h"
#include "extern_audio.h"
#include "linux/gpio.h"
#include "audio_trace.h"

#if (!defined(CONFIG_BCMPMU_AUDIO))

#include <linux/broadcom/bcmpmu_audio.h>
#define AUDIO_PMU_INIT() NULL
#define AUDIO_PMU_HS_SET_GAIN(a, b) NULL
#define AUDIO_PMU_HS_POWER(a) NULL
#define AUDIO_PMU_IHF_SET_GAIN(a) NULL
#define AUDIO_PMU_IHF_POWER(a) NULL
#define AUDIO_PMU_DEINIT() NULL
#define AUDIO_PMU_HS_HI_GAIN_MODE_EN(a, b) NULL
#define AUDIO_PMU_IHF_HI_GAIN_MODE_EN(a) NULL
/* PMU ALC support */
#define AUDIO_PMU_IHF_ALC_EN(a) NULL
#define AUDIO_PMU_IHF_ALC_VBAT_REF(a) NULL
#define AUDIO_PMU_IHF_ALC_THLD(a) NULL
#define AUDIO_PMU_IHF_ALC_RAMP_UP_CTRL(a) NULL
#define AUDIO_PMU_IHF_ALC_RAMP_DOWN_CTRL(a) NULL

#else

#ifdef CONFIG_BCM59055_AUDIO

#include "linux/broadcom/bcm59055-audio.h"
#define AUDIO_PMU_INIT bcm59055_audio_init
#define AUDIO_PMU_HS_SET_GAIN bcm59055_hs_set_gain
#define AUDIO_PMU_HS_POWER bcm59055_hs_power
#define AUDIO_PMU_IHF_SET_GAIN bcm59055_ihf_set_gain
#define AUDIO_PMU_IHF_POWER bcm59055_ihf_power
#define AUDIO_PMU_DEINIT bcm59055_audio_deinit
#define AUDIO_PMU_HS_HI_GAIN_MODE_EN NULL
#define AUDIO_PMU_IHF_HI_GAIN_MODE_EN NULL
/* PMU ALC support */
#define AUDIO_PMU_IHF_ALC_EN NULL
#define AUDIO_PMU_IHF_ALC_VBAT_REF NULL
#define AUDIO_PMU_IHF_ALC_THLD NULL
#define AUDIO_PMU_IHF_ALC_RAMP_UP_CTRL NULL
#define AUDIO_PMU_IHF_ALC_RAMP_DOWN_CTRL NULL

#else

#include <linux/broadcom/bcmpmu_audio.h>
#define AUDIO_PMU_INIT bcmpmu_audio_init
#define AUDIO_PMU_HS_SET_GAIN bcmpmu_hs_set_gain
#define AUDIO_PMU_HS_POWER bcmpmu_hs_power
#define AUDIO_PMU_IHF_SET_GAIN bcmpmu_ihf_set_gain
#define AUDIO_PMU_IHF_POWER bcmpmu_ihf_power
#define AUDIO_PMU_DEINIT bcmpmu_audio_deinit
#define AUDIO_PMU_HS_HI_GAIN_MODE_EN bcmpmu_hi_gain_mode_hs_en
#define AUDIO_PMU_IHF_HI_GAIN_MODE_EN bcmpmu_hi_gain_mode_en
/* PMU ALC support */
#define AUDIO_PMU_IHF_ALC_EN bcmpmu_enable_alc
#define AUDIO_PMU_IHF_ALC_VBAT_REF bcmpmu_ihf_alc_vbat_ref
#define AUDIO_PMU_IHF_ALC_THLD bcmpmu_ihf_alc_thld
#define AUDIO_PMU_IHF_ALC_RAMP_UP_CTRL bcmpmu_ihf_alc_rampup_ctrl
#define AUDIO_PMU_IHF_ALC_RAMP_DOWN_CTRL bcmpmu_ihf_alc_ramp_down_ctrl
#endif
#endif

struct PMU_AudioGainMapping_t {
	int gain_mB;
	unsigned int PMU_gain_enum;
};

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
	PMU_IHFGAIN_3P5DB_P,
	PMU_IHFGAIN_4DB_P,
    PMU_IHFGAIN_NUM
};
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
	{-4000, PMU_HSGAIN_40P5DB_N},	/*10 */
	{-3900, PMU_HSGAIN_39DB_N},
	{-3750, PMU_HSGAIN_37P5DB_N},
	{-3600, PMU_HSGAIN_36DB_N},
	{-3450, PMU_HSGAIN_34P5DB_N},
	{-3300, PMU_HSGAIN_33DB_N},
	{-3150, PMU_HSGAIN_31P5DB_N},
	{-3000, PMU_HSGAIN_30DB_N},
	{-2850, PMU_HSGAIN_28P5DB_N},
	{-2700, PMU_HSGAIN_27DB_N},
	{-2550, PMU_HSGAIN_25P5DB_N},	/*20 */
	{-2400, PMU_HSGAIN_24DB_N},
	{-2250, PMU_HSGAIN_22P5DB_N},
	{-2200, PMU_HSGAIN_22DB_N},
	{-2150, PMU_HSGAIN_21P5DB_N},
	{-2100, PMU_HSGAIN_21DB_N},
	{-2050, PMU_HSGAIN_20P5DB_N},
	{-2000, PMU_HSGAIN_20DB_N},
	{-1950, PMU_HSGAIN_19P5DB_N},
	{-1900, PMU_HSGAIN_19DB_N},
	{-1850, PMU_HSGAIN_18P5DB_N},	/*30 */
	{-1800, PMU_HSGAIN_18DB_N},
	{-1750, PMU_HSGAIN_17P5DB_N},
	{-1700, PMU_HSGAIN_17DB_N},
	{-1650, PMU_HSGAIN_16P5DB_N},
	{-1600, PMU_HSGAIN_16DB_N},
	{-1550, PMU_HSGAIN_15P5DB_N},
	{-1500, PMU_HSGAIN_15DB_N},
	{-1450, PMU_HSGAIN_14P5DB_N},
	{-1400, PMU_HSGAIN_14DB_N},
	{-1350, PMU_HSGAIN_13P5DB_N},	/*40 */
	{-1300, PMU_HSGAIN_13DB_N},
	{-1250, PMU_HSGAIN_12P5DB_N},
	{-1200, PMU_HSGAIN_12DB_N},
	{-1150, PMU_HSGAIN_11P5DB_N},
	{-1100, PMU_HSGAIN_11DB_N},
	{-1050, PMU_HSGAIN_10P5DB_N},
	{-1000, PMU_HSGAIN_10DB_N},
	{-950, PMU_HSGAIN_9P5DB_N},
	{-900, PMU_HSGAIN_9DB_N},
	{-850, PMU_HSGAIN_8P5DB_N},	/*50 */
	{-800, PMU_HSGAIN_8DB_N},
	{-750, PMU_HSGAIN_7P5DB_N},
	{-700, PMU_HSGAIN_7DB_N},
	{-650, PMU_HSGAIN_6P5DB_N},
	{-600, PMU_HSGAIN_6DB_N},
	{-550, PMU_HSGAIN_5P5DB_N},
	{-500, PMU_HSGAIN_5DB_N},
	{-450, PMU_HSGAIN_4P5DB_N},
	{-400, PMU_HSGAIN_4DB_N},
	{-350, PMU_HSGAIN_3P5DB_N},	/*60 */
	{-300, PMU_HSGAIN_3DB_N},
	{-250, PMU_HSGAIN_2P5DB_N},
	{-200, PMU_HSGAIN_2DB_N}	/*63 */
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
	{-3450, PMU_IHFGAIN_34P5DB_N},	/*10 */
	{-3300, PMU_IHFGAIN_33DB_N},
	{-3150, PMU_IHFGAIN_31P5DB_N},
	{-3000, PMU_IHFGAIN_30DB_N},
	{-2850, PMU_IHFGAIN_28P5DB_N},
	{-2700, PMU_IHFGAIN_27DB_N},
	{-2550, PMU_IHFGAIN_25P5DB_N},
	{-2400, PMU_IHFGAIN_24DB_N},
	{-2250, PMU_IHFGAIN_22P5DB_N},
	{-2100, PMU_IHFGAIN_21DB_N},
	{-1950, PMU_IHFGAIN_19P5DB_N},	/*20 */
	{-1800, PMU_IHFGAIN_18DB_N},
	{-1650, PMU_IHFGAIN_16P5DB_N},
	{-1600, PMU_IHFGAIN_16DB_N},
	{-1550, PMU_IHFGAIN_15P5DB_N},
	{-1500, PMU_IHFGAIN_15DB_N},
	{-1450, PMU_IHFGAIN_14P5DB_N},
	{-1400, PMU_IHFGAIN_14DB_N},
	{-1350, PMU_IHFGAIN_13P5DB_N},
	{-1300, PMU_IHFGAIN_13DB_N},
	{-1250, PMU_IHFGAIN_12P5DB_N},	/*30 */
	{-1200, PMU_IHFGAIN_12DB_N},
	{-1150, PMU_IHFGAIN_11P5DB_N},
	{-1100, PMU_IHFGAIN_11DB_N},
	{-1050, PMU_IHFGAIN_10P5DB_N},
	{-1000, PMU_IHFGAIN_10DB_N},
	{-950, PMU_IHFGAIN_9P5DB_N},
	{-900, PMU_IHFGAIN_9DB_N},
	{-850, PMU_IHFGAIN_8P5DB_N},
	{-800, PMU_IHFGAIN_8DB_N},
	{-750, PMU_IHFGAIN_7P5DB_N},	/*40 */
	{-700, PMU_IHFGAIN_7DB_N},
	{-650, PMU_IHFGAIN_6P5DB_N},
	{-600, PMU_IHFGAIN_6DB_N},
	{-550, PMU_IHFGAIN_5P5DB_N},
	{-500, PMU_IHFGAIN_5DB_N},
	{-450, PMU_IHFGAIN_4P5DB_N},
	{-400, PMU_IHFGAIN_4DB_N},
	{-350, PMU_IHFGAIN_3P5DB_N},
	{-300, PMU_IHFGAIN_3DB_N},
	{-250, PMU_IHFGAIN_2P5DB_N},	/*50 */
	{-200, PMU_IHFGAIN_2DB_N},
	{-150, PMU_IHFGAIN_1P5DB_N},
	{-100, PMU_IHFGAIN_1DB_N},
	{-50, PMU_IHFGAIN_P5DB_N},
	{0, PMU_IHFGAIN_0DB},	/*55 */
	{50, PMU_IHFGAIN_P5DB_P},
	{100, PMU_IHFGAIN_1DB_P},
	{150, PMU_IHFGAIN_1P5DB_P},
	{200, PMU_IHFGAIN_2DB_P},
	{250, PMU_IHFGAIN_2P5DB_P},	/*60 */
	{300, PMU_IHFGAIN_3DB_P},
	{350, PMU_IHFGAIN_3P5DB_P},
	{400, PMU_IHFGAIN_4DB_P}
};

static char ihf_IsOn;
static char hs_IsOn;
static int pll_IsOn;
static int hs_gain_l = -400;	/* mB */
static int hs_gain_r = -400;	/* mB */
static int ihf_gain = -400;	/* mB */

static struct extern_audio_platform_cfg ext_aud_plat_cfg = {
	.ihf_ext_amp_gpio = -1,
	.dock_aud_route_gpio = -1,
#if defined(CONFIG_GPIO_2IN1_SPK)
	.spk_2in1_gpio = -1,
#endif
};

/******************************************************************************
* Function Name: audio_gpio_output
*
* Description: Toggle gpio output pin
*
* Note: This is only required on some OEM hardware
*
******************************************************************************/
static void audio_gpio_output(char *name, int gpio_pin, int value)
{
	int rc = gpio_request(gpio_pin, name);

	if (!rc) {
		aTrace(LOG_AUDIO_CNTLR,
		       "audio_gpio_output::gpio pin %d value %d, rc=0x%x\n",
		       gpio_pin, value, rc);

		gpio_direction_output(gpio_pin, 0);
		gpio_set_value(gpio_pin, value);
		gpio_free(gpio_pin);
	} else {
		aTrace(LOG_AUDIO_CNTLR, "%s::"
		       " failed to request"
		       "%s gpio pin %d rc=%u\n", __func__, name, gpio_pin, rc);
	}
}

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

	/*Should not run to here. */
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

	/*Should not run to here. */
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
	if (pll_IsOn == 0) {
		AUDIO_PMU_INIT();
		pll_IsOn = 1;
	}

	AUDIO_PMU_HS_SET_GAIN(PMU_AUDIO_HS_BOTH, PMU_HSGAIN_MUTE);

	/*
	   ./drivers/misc/bcm59055-audio.c:int bcm59055_hs_power(bool on)
	   drivers/misc/bcm59055-audio.c
	   drivers/misc/bcmpmu_audio.c
	 */

	AUDIO_PMU_HS_POWER(1);

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
	/*BCM59056 PMU HW ramps down HS gain when powers off HS amp.
	   no need for software to ramp down HS gain.
	   AUDIO_PMU_HS_SET_GAIN(PMU_AUDIO_HS_BOTH,
	   PMU_HSGAIN_MUTE); */
	AUDIO_PMU_HS_POWER(0);

	hs_IsOn = 0;

	if (ihf_IsOn == 0 && hs_IsOn == 0)
		/*disable the audio PLL after power OFF */
		if (pll_IsOn == 1) {
			AUDIO_PMU_DEINIT();
			pll_IsOn = 0;
		}

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
#if defined(CONFIG_GPIO_2IN1_SPK)
	if (ext_aud_plat_cfg.spk_2in1_gpio > 0) {
		aTrace(LOG_AUDIO_CNTLR,
		       " caph_hawaii CONFIG_GPIO_2IN1_SPK %d\n",
		       ext_aud_plat_cfg.spk_2in1_gpio);
		audio_gpio_output("IHF_EXT_AMP",
				  ext_aud_plat_cfg.spk_2in1_gpio, 0);
	}
#endif

#if defined(CONFIG_IHF_EXT_AMPLIFIER)
	if (ext_aud_plat_cfg.ihf_ext_amp_gpio > 0)
		audio_gpio_output("IHF_EXT_AMP",
				  ext_aud_plat_cfg.ihf_ext_amp_gpio, 1);
#else
	/*enable the audio PLL before power ON */
	if (pll_IsOn == 0) {
		AUDIO_PMU_INIT();
		pll_IsOn = 1;
	}

	AUDIO_PMU_IHF_SET_GAIN(PMU_IHFGAIN_MUTE);
	AUDIO_PMU_IHF_POWER(1);
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
	if (ext_aud_plat_cfg.ihf_ext_amp_gpio > 0)
		audio_gpio_output("IHF_EXT_AMP",
				  ext_aud_plat_cfg.ihf_ext_amp_gpio, 0);
#else
	AUDIO_PMU_IHF_SET_GAIN(PMU_IHFGAIN_MUTE);
	AUDIO_PMU_IHF_POWER(0);

	if (ihf_IsOn == 0 && hs_IsOn == 0)
		/*disable the audio PLL after power OFF */
		if (pll_IsOn == 1) {
			AUDIO_PMU_DEINIT();
			pll_IsOn = 0;
		}
#endif
#if defined(CONFIG_GPIO_2IN1_SPK)
	if (ext_aud_plat_cfg.spk_2in1_gpio > 0) {
		aTrace(LOG_AUDIO_CNTLR,
		       " extern_ihf_off CONFIG_GPIO_2IN1_SPK %d\n",
		       ext_aud_plat_cfg.spk_2in1_gpio);
		audio_gpio_output("IHF_EXT_AMP",
				  ext_aud_plat_cfg.spk_2in1_gpio, 1);
	}
#endif

}

#if defined(CONFIG_IHF_TWO_EXT_AMPLIFIER)

/********************************************************************
*  @brief  power on two external IHF (loud speaker) amplifiers
*
*  @param  none
*  @return none
*
****************************************************************************/
void extern_ihf_two_external_amplifier_on(int isVoiceCall)
{
	/* turn on one external amplifier for voice call,
	turn on both for music */
	if (ext_aud_plat_cfg.ihf_ext_amp_gpio > 0) {
		audio_gpio_output("IHF_EXT_AMP",
				  ext_aud_plat_cfg.ihf_ext_amp_gpio, 1);
		aTrace(LOG_AUDIO_CNTLR, "turn on IHF_EXT_AMP, gpio=%d",
		       ext_aud_plat_cfg.ihf_ext_amp_gpio);
	}

	if (!isVoiceCall) {
		if (ext_aud_plat_cfg.ihf_ext_amp_second_gpio > 0) {
			audio_gpio_output("IHF_EXT_AMP_2ND",
					  ext_aud_plat_cfg.
					  ihf_ext_amp_second_gpio, 1);
			aTrace(LOG_AUDIO_CNTLR,
			       "turn on IHF_EXT_AMP_2ND, gpio=%d",
			       ext_aud_plat_cfg.ihf_ext_amp_second_gpio);
		}
	}

	ihf_IsOn = 1;
}

/********************************************************************
*  @brief  power off two external IHF (loud speaker) amplifiers
*
*  @param  none
*  @return none
*
****************************************************************************/
void extern_ihf_two_external_amplifier_off(int isVoiceCall)
{
	ihf_IsOn = 0;
	if (ext_aud_plat_cfg.ihf_ext_amp_gpio > 0) {
		audio_gpio_output("IHF_EXT_AMP",
				  ext_aud_plat_cfg.ihf_ext_amp_gpio, 0);
		aTrace(LOG_AUDIO_CNTLR, "turn off IHF_EXT_AMP, gpio=%d",
		       ext_aud_plat_cfg.ihf_ext_amp_gpio);
	}
	if (!isVoiceCall) {
		if (ext_aud_plat_cfg.ihf_ext_amp_second_gpio > 0) {
			audio_gpio_output("IHF_EXT_AMP_2ND",
					  ext_aud_plat_cfg.
					  ihf_ext_amp_second_gpio, 0);
			aTrace(LOG_AUDIO_CNTLR,
			       "turn off IHF_EXT_AMP_2ND, gpio=%d",
			       ext_aud_plat_cfg.ihf_ext_amp_second_gpio);
		}
	}
}
#endif /* CONFIG_IHF_TWO_EXT_AMPLIFIER */

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
		AUDIO_PMU_HS_SET_GAIN(PMU_AUDIO_HS_BOTH,
				      gain_map.PMU_gain_enum);
		hs_gain_l = gain_map.gain_mB;
		hs_gain_r = gain_map.gain_mB;
	} else if (lr == AUDIO_HS_LEFT) {
		AUDIO_PMU_HS_SET_GAIN(PMU_AUDIO_HS_LEFT,
				      gain_map.PMU_gain_enum);
		hs_gain_l = gain_map.gain_mB;
	} else if (lr == AUDIO_HS_RIGHT) {
		AUDIO_PMU_HS_SET_GAIN(PMU_AUDIO_HS_RIGHT,
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
		AUDIO_PMU_HS_SET_GAIN(PMU_AUDIO_HS_BOTH, PMU_HSGAIN_MUTE);
	else
	if (lr == AUDIO_HS_LEFT)
		AUDIO_PMU_HS_SET_GAIN(PMU_AUDIO_HS_LEFT, PMU_HSGAIN_MUTE);
	else
	if (lr == AUDIO_HS_RIGHT)
		AUDIO_PMU_HS_SET_GAIN(PMU_AUDIO_HS_RIGHT, PMU_HSGAIN_MUTE);
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

	AUDIO_PMU_IHF_SET_GAIN(gain_map.PMU_gain_enum);

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
	AUDIO_PMU_IHF_SET_GAIN(PMU_IHFGAIN_MUTE);
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
*  @brief  Set HS gain shift mode on external audio chip
*
*  @param  enable	 1 - enable, 0 - disable
*  @return  none
*
****************************************************************************/
void extern_hs_en_hi_gain_mode(int enable)
{
	aTrace(LOG_AUDIO_CNTLR, "%s enable=%d\n", __func__, enable);
#if 0				/* Kishore PMU does not support */
	if (0 == enable)
		AUDIO_PMU_HS_HI_GAIN_MODE_EN(PMU_AUDIO_HS_BOTH, 0);
	else
		AUDIO_PMU_HS_HI_GAIN_MODE_EN(PMU_AUDIO_HS_BOTH, 1);
#endif
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
	aTrace(LOG_AUDIO_CNTLR, "%s enable=%d\n", __func__, enable);

	if (0 == enable)
		AUDIO_PMU_IHF_HI_GAIN_MODE_EN(0);
	else
		AUDIO_PMU_IHF_HI_GAIN_MODE_EN(1);
}

/********************************************************************
*  @brief  Set the platform configuration for external audio layer
*
*  @param  cfg - configuration structure
*  @return  none
*
****************************************************************************/
void extern_audio_platform_cfg_set(void *cfg)
{
	struct extern_audio_platform_cfg *extaud_cfg =
	    (struct extern_audio_platform_cfg *)cfg;
	memcpy(&ext_aud_plat_cfg, extaud_cfg, sizeof(ext_aud_plat_cfg));
}

/********************************************************************
*  @brief  Set the GPIO pin value to route the audio to the dock
*
*  @param  Toggle value of the GPIO pin.  1 - High, 0 - Low
*  @return  none
*
****************************************************************************/
void extern_dock_audio_route(int gpio_val)
{
	if (ext_aud_plat_cfg.dock_aud_route_gpio > 0)
		audio_gpio_output("AUDIO_DOCK_ROUTE",
			ext_aud_plat_cfg.dock_aud_route_gpio, gpio_val);
}

/* PMU ALC support */
/********************************************************************
*  @brief  enable or disable PMU ALC
*
*  @param  on      1 - enable, 0 - disable
*  @return  none
*
****************************************************************************/
void extern_ihf_set_alc_enable(int on)
{
	aTrace(LOG_AUDIO_CNTLR, "%s on=%d\n", __func__, on);

	AUDIO_PMU_IHF_ALC_EN((bool)on);
}

/********************************************************************
*  @brief  set ALC reference from VBAT or from VLDO
*
*  @param  on      1 - VBAT, 0 - VLDO
*  @return  none
*
****************************************************************************/
void extern_ihf_set_alc_vbat_ref(int on)
{
	aTrace(LOG_AUDIO_CNTLR, "%s on=%d\n", __func__, on);

	AUDIO_PMU_IHF_ALC_VBAT_REF((bool)on);
}

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
void extern_ihf_set_alc_thld(int thld)
{
	aTrace(LOG_AUDIO_CNTLR, "%s thld=%d\n", __func__, thld);

	AUDIO_PMU_IHF_ALC_THLD((enum ihf_alc_thld)thld);
}

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
void extern_ihf_set_alc_ramp_up_ctrl(int ctrl)
{
	aTrace(LOG_AUDIO_CNTLR, "%s ctrl=%d\n", __func__, ctrl);

	AUDIO_PMU_IHF_ALC_RAMP_UP_CTRL((enum ihf_alc_ramp_up_ctrl)ctrl);
}

/********************************************************************
*  @brief  control ALC ramp down time
*
*  @param  ctrl      0 - 100 us
*                    1 - 500 us
*                    2 - 1 ms
*  @return  none
*
****************************************************************************/
void extern_ihf_set_alc_ramp_down_ctrl(int ctrl)
{
	aTrace(LOG_AUDIO_CNTLR, "%s ctrl=%d\n", __func__, ctrl);

	AUDIO_PMU_IHF_ALC_RAMP_DOWN_CTRL((enum ihf_alc_ramp_down_ctrl)ctrl);
}

