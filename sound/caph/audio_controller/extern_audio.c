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



#ifdef THIRD_PARTY_PMU
#include "dialog_audio.h"
#endif

#if (!defined(CONFIG_BCMPMU_AUDIO))

void extern_hs_on(void) {; }
void extern_hs_off(void) {; }
void extern_ihf_on(void) {; }
void extern_ihf_off(void) {; }
void extern_stereo_speaker_on(void) {; }
void extern_stereo_speaker_off(void) {; }
int extern_hs_find_gain(int gain_mB) {return 0; }
void extern_hs_set_gain(int gain_mB, AUDIO_GAIN_LR_t lr) {; }
void extern_hs_mute(AUDIO_GAIN_LR_t lr) {; }
void extern_hs_unmute(AUDIO_GAIN_LR_t lr) {; }
int extern_ihf_find_gain(int gain_mB) {return 0; }
void extern_ihf_set_gain(int gain_mB) {; }
void extern_ihf_mute(void) {; }
void extern_ihf_unmute(void) {; }
void extern_ihf_en_hi_gain_mode(int enable) {; }


#else

#include <linux/broadcom/bcmpmu_audio.h>

struct PMU_AudioGainMapping_t {
	int gain_mB;
	unsigned int PMU_gain_enum;
};

/*
in bcmpmu_hs_set_input_mode(),
add 6 dB shift if input mode is PMU_HS_SINGLE_ENDED,
threfore the HS gain is the same for PMU_HS_SINGLE_ENDEDi
 and PMU_HS_DIFFERENTIAL.
RheaStone use PMU_HS_DIFFERENTIAL
Next pone with 59039C0 will use PMU_HS_SINGLE_ENDED.

Therefore use the register value - gain mapping for no-boost mode */

static struct PMU_AudioGainMapping_t hsPMUGainTable[PMU_HSGAIN_NUM] = {
	/* Gain in milli Bel,      HS PMU Gain enum */
#if defined(CONFIG_MFD_BCM59055)

	{-10000, 0x00},
	{-6600, 0x01},
	{-6300, 0x02},
	{-6000, 0x03},
	{-5700, 0x04},
	{-5400, 0x05},
	{-5100, 0x06},
	{-4800, 0x07},
	{-4500, 0x08},
	{-4200, 0x09},
	{-4000, 0x0A},
	{-3900, 0x0B},
	{-3750, 0x0C},
	{-3600, 0x0D},
	{-3450, 0x0E},
	{-3300, 0x0F},
	{-3150, 0x10},
	{-3000, 0x11},
	{-2850, 0x12},
	{-2700, 0x13},
	{-2550, 0x14},
	{-2400, 0x15},
	{-2250, 0x16},
	{-2200, 0x17},
	{-2150, 0x18},
	{-2100, 0x19},
	{-2050, 0x1A},
	{-2000, 0x1B},
	{-1950, 0x1C},
	{-1900, 0x1D},
	{-1850, 0x1E},
	{-1800, 0x1F},
	{-1750, 0x20},
	{-1700, 0x21},
	{-1650, 0x22},
	{-1600, 0x23},
	{-1550, 0x24},
	{-1500, 0x25},
	{-1450, 0x26},
	{-1400, 0x27},
	{-1350, 0x28},
	{-1300, 0x29},
	{-1250, 0x2A},
	{-1200, 0x2B},
	{-1150, 0x2C},
	{-1100, 0x2D},
	{-1050, 0x2E},
	{-1000, 0x2F},
	{-950, 0x30},
	{-900, 0x31},
	{-850, 0x32},
	{-800, 0x33},
	{-750, 0x34},
	{-700, 0x35},
	{-650, 0x36},
	{-600, 0x37},
	{-550, 0x38},
	{-500, 0x39},
	{-450, 0x3A},
	{-400, 0x3B},
	{-350, 0x3C},
	{-300, 0x3D},
	{-250, 0x3E},
	{-200, 0x3F}

#endif
#if defined(CONFIG_MFD_BCM59039) || defined(CONFIG_MFD_BCM_PMU59xxx)

	{-10000, 0x00},
	{-6800, 0x01},
	{-6500, 0x02},
	{-6200, 0x03},
	{-5900, 0x04},
	{-5600, 0x05},
	{-5300, 0x06},
	{-5000, 0x07},
	{-4700, 0x08},
	{-4400, 0x09},
	{-4250, 0x0A},
	{-4100, 0x0B},
	{-3950, 0x0C},
	{-3800, 0x0D},
	{-3650, 0x0E},
	{-3500, 0x0F},
	{-3350, 0x10},
	{-3200, 0x11},
	{-3050, 0x12},
	{-2900, 0x13},
	{-2750, 0x14},
	{-2600, 0x15},
	{-2450, 0x16},
	{-2400, 0x17},
	{-2350, 0x18},
	{-2300, 0x19},
	{-2250, 0x1A},
	{-2200, 0x1B},
	{-2150, 0x1C},
	{-2100, 0x1D},
	{-2050, 0x1E},
	{-2000, 0x1F},
	{-1950, 0x20},
	{-1900, 0x21},
	{-1850, 0x22},
	{-1800, 0x23},
	{-1750, 0x24},
	{-1700, 0x25},
	{-1650, 0x26},
	{-1600, 0x27},
	{-1550, 0x28},
	{-1500, 0x29},
	{-1450, 0x2A},
	{-1400, 0x2B},
	{-1350, 0x2C},
	{-1300, 0x2D},
	{-1250, 0x2E},
	{-1200, 0x2F},
	{-1150, 0x30},
	{-1100, 0x31},
	{-1050, 0x32},
	{-1000, 0x33},
	{-950, 0x34},
	{-900, 0x35},
	{-850, 0x36},
	{-800, 0x37},
	{-750, 0x38},
	{-700, 0x39},
	{-650, 0x3A},
	{-600, 0x3B},
	{-550, 0x3C},
	{-500, 0x3D},
	{-450, 0x3E},
	{-400, 0x3F}

#endif

#if defined(CONFIG_MFD_BCM59042)

	{-10000, 0x00},
	{-6800, 0x01},
	{-6500, 0x02},
	{-6200, 0x03},
	{-5900, 0x04},
	{-5600, 0x05},
	{-5300, 0x06},
	{-5000, 0x07},
	{-4700, 0x08},
	{-4400, 0x09},
	{-4250, 0x0A},
	{-4100, 0x0B},
	{-3950, 0x0C},
	{-3800, 0x0D},
	{-3650, 0x0E},
	{-3500, 0x0F},
	{-3350, 0x10},
	{-3200, 0x11},
	{-3050, 0x12},
	{-2900, 0x13},
	{-2750, 0x14},
	{-2600, 0x15},
	{-2450, 0x16},
	{-2400, 0x17},
	{-2350, 0x18},
	{-2300, 0x19},
	{-2250, 0x1A},
	{-2200, 0x1B},
	{-2150, 0x1C},
	{-2100, 0x1D},
	{-2050, 0x1E},
	{-2000, 0x1F},
	{-1950, 0x20},
	{-1900, 0x21},
	{-1850, 0x22},
	{-1800, 0x23},
	{-1750, 0x24},
	{-1700, 0x25},
	{-1650, 0x26},
	{-1600, 0x27},
	{-1550, 0x28},
	{-1500, 0x29},
	{-1450, 0x2A},
	{-1400, 0x2B},
	{-1350, 0x2C},
	{-1300, 0x2D},
	{-1250, 0x2E},
	{-1200, 0x2F},
	{-1150, 0x30},
	{-1100, 0x31},
	{-1050, 0x32},
	{-1000, 0x33},
	{-950, 0x34},
	{-900, 0x35},
	{-850, 0x36},
	{-800, 0x37},
	{-750, 0x38},
	{-700, 0x39},
	{-650, 0x3A},
	{-600, 0x3B},
	{-550, 0x3C},
	{-500, 0x3D},
	{-450, 0x3E},
	{-400, 0x3F}

#endif

};

static struct PMU_AudioGainMapping_t ihfPMUGainTable[PMU_IHFGAIN_NUM] = {
	/* Gain in milli Bel,          IHF PMU Gain enum */
#if defined(CONFIG_MFD_BCM59055)

	{-10000, 0x00},
	{-6000, 0x01},
	{-5700, 0x02},
	{-5400, 0x03},
	{-5100, 0x04},
	{-4800, 0x05},
	{-4500, 0x06},
	{-4200, 0x07},
	{-3900, 0x08},
	{-3600, 0x09},
	{-3450, 0x0A},
	{-3300, 0x0B},
	{-3150, 0x0C},
	{-3000, 0x0D},
	{-2850, 0x0E},
	{-2700, 0x0F},
	{-2550, 0x10},
	{-2400, 0x11},
	{-2250, 0x12},
	{-2100, 0x13},
	{-1950, 0x14},
	{-1800, 0x15},
	{-1650, 0x16},
	{-1600, 0x17},
	{-1550, 0x18},
	{-1500, 0x19},
	{-1450, 0x1A},
	{-1400, 0x1B},
	{-1350, 0x1C},
	{-1300, 0x1D},
	{-1250, 0x1E},
	{-1200, 0x1F},
	{-1150, 0x20},
	{-1100, 0x21},
	{-1050, 0x22},
	{-1000, 0x23},
	{-950, 0x24},
	{-900, 0x25},
	{-850, 0x26},
	{-800, 0x27},
	{-750, 0x28},
	{-700, 0x29},
	{-650, 0x2A},
	{-600, 0x2B},
	{-550, 0x2C},
	{-500, 0x2D},
	{-450, 0x2E},
	{-400, 0x2F},
	{-350, 0x30},
	{-300, 0x31},
	{-250, 0x32},
	{-200, 0x33},
	{-150, 0x34},
	{-100, 0x35},
	{-50, 0x36},
	{0, 0x37},
	{50, 0x38},
	{100, 0x39},
	{150, 0x3A},
	{200, 0x3B},
	{250, 0x3C},
	{300, 0x3D},
	{350, 0x3E},
	{400, 0x3F}

#endif
#if defined(CONFIG_MFD_BCM59039) || defined(CONFIG_MFD_BCM_PMU59xxx)

	{-10000, 0x00},
	{-6200, 0x01},
	{-5900, 0x02},
	{-5600, 0x03},
	{-5300, 0x04},
	{-5000, 0x05},
	{-4700, 0x06},
	{-4400, 0x07},
	{-4250, 0x08},
	{-4100, 0x09},
	{-3950, 0x0A},
	{-3800, 0x0B},
	{-3650, 0x0C},
	{-3500, 0x0D},
	{-3350, 0x0E},
	{-3200, 0x0F},
	{-3050, 0x10},
	{-2900, 0x11},
	{-2750, 0x12},
	{-2600, 0x13},
	{-2450, 0x14},
	{-2300, 0x15},
	{-2150, 0x16},
	{-2000, 0x17},
	{-1850, 0x18},
	{-1700, 0x19},
	{-1550, 0x1A},
	{-1400, 0x1B},
	{-1350, 0x1C},
	{-1300, 0x1D},
	{-1250, 0x1E},
	{-1200, 0x1F},
	{-1150, 0x20},
	{-1100, 0x21},
	{-1050, 0x22},
	{-1000, 0x23},
	{-950, 0x24},
	{-900, 0x25},
	{-850, 0x26},
	{-800, 0x27},
	{-750, 0x28},
	{-700, 0x29},
	{-650, 0x2A},
	{-600, 0x2B},
	{-550, 0x2C},
	{-500, 0x2D},
	{-450, 0x2E},
	{-400, 0x2F},
	{-350, 0x30},
	{-300, 0x31},
	{-250, 0x32},
	{-200, 0x33},
	{-150, 0x34},
	{-100, 0x35},
	{-50, 0x36},
	{0, 0x37},
	{50, 0x38},
	{100, 0x39},
	{150, 0x3A},
	{200, 0x3B},
	{250, 0x3C},
	{300, 0x3D},
	{350, 0x3E},
	{400, 0x3F}

#endif

#if defined(CONFIG_MFD_BCM59042)
	{-10000, 0x00},
	{-6200, 0x01},
	{-5900, 0x02},
	{-5600, 0x03},
	{-5300, 0x04},
	{-5000, 0x05},
	{-4700, 0x06},
	{-4400, 0x07},
	{-4250, 0x08},
	{-4100, 0x09},
	{-3950, 0x0A},
	{-3800, 0x0B},
	{-3650, 0x0C},
	{-3500, 0x0D},
	{-3350, 0x0E},
	{-3200, 0x0F},
	{-3050, 0x10},
	{-2900, 0x11},
	{-2750, 0x12},
	{-2600, 0x13},
	{-2450, 0x14},
	{-2300, 0x15},
	{-2150, 0x16},
	{-2000, 0x17},
	{-1850, 0x18},
	{-1700, 0x19},
	{-1550, 0x1A},
	{-1400, 0x1B},
	{-1350, 0x1C},
	{-1300, 0x1D},
	{-1250, 0x1E},
	{-1200, 0x1F},
	{-1150, 0x20},
	{-1100, 0x21},
	{-1050, 0x22},
	{-1000, 0x23},
	{-950, 0x24},
	{-900, 0x25},
	{-850, 0x26},
	{-800, 0x27},
	{-750, 0x28},
	{-700, 0x29},
	{-650, 0x2A},
	{-600, 0x2B},
	{-550, 0x2C},
	{-500, 0x2D},
	{-450, 0x2E},
	{-400, 0x2F},
	{-350, 0x30},
	{-300, 0x31},
	{-250, 0x32},
	{-200, 0x33},
	{-150, 0x34},
	{-100, 0x35},
	{-50, 0x36},
	{0, 0x37},
	{50, 0x38},
	{100, 0x39},
	{150, 0x3A},
	{200, 0x3B},
	{250, 0x3C},
	{300, 0x3D},
	{350, 0x3E},
	{400, 0x3F}

#endif

};

#ifndef THIRD_PARTY_PMU
static char ihf_IsOn;
static char hs_IsOn;
static int pll_IsOn;
#endif

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

#ifdef THIRD_PARTY_PMU

	dialog_audio_hs_poweron();

#else
	BCMPMU_Audio_HS_Param hs_param;

	/*enable the audio PLL before power ON */
	if (pll_IsOn == 0) {
		bcmpmu_audio_init();
		pll_IsOn = 1;
	}

	bcmpmu_hs_set_gain(PMU_AUDIO_HS_BOTH,
				  PMU_HSGAIN_MUTE);

	/*
	./drivers/misc/bcm59055-audio.c:int bcm59055_hs_power(bool on)
	drivers/misc/bcm59055-audio.c
	drivers/misc/bcmpmu_audio.c
	*/

	bcmpmu_hs_power(1);

	hs_IsOn = 1;

	hs_param = bcmpmu_get_hs_param_from_audio_driver();
	hs_param.hs_power = 1;
	hs_param.hs_gain_left = PMU_HSGAIN_MUTE;
	hs_param.hs_gain_right = PMU_HSGAIN_MUTE;
	hs_param.hs_hi_gain_mode = 0;
	bcmpmu_audio_driver_set_hs_param(hs_param);
#endif

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

#ifdef THIRD_PARTY_PMU

	dialog_audio_hs_poweroff();

#else

	BCMPMU_Audio_HS_Param hs_param;

	/*BCM59039 PMU HW ramps down HS gain when powers off HS amp.
	no need for software to ramp down HS gain.
	bcmpmu_hs_set_gain(PMU_AUDIO_HS_BOTH,
				  PMU_HSGAIN_MUTE);*/
	bcmpmu_hs_power(0);

	hs_IsOn = 0;

	hs_param = bcmpmu_get_hs_param_from_audio_driver();
	hs_param.hs_power = 0;
	hs_param.hs_gain_left = PMU_HSGAIN_MUTE;
	hs_param.hs_gain_right = PMU_HSGAIN_MUTE;
	hs_param.hs_hi_gain_mode = 0;
	bcmpmu_audio_driver_set_hs_param(hs_param);

	if (ihf_IsOn == 0 && hs_IsOn == 0)
		/*disable the audio PLL after power OFF*/
		if (pll_IsOn == 1) {
			bcmpmu_audio_deinit();
			pll_IsOn = 0;
		}
#endif
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

#ifdef THIRD_PARTY_PMU

	dialog_audio_ihf_poweron();

#else

#if defined(CONFIG_IHF_EXT_AMPLIFIER)
	audio_gpio_output(GPIO_IHF_EXT_AMP, 1);
#else
	/*enable the audio PLL before power ON */
	if (pll_IsOn == 0) {
		bcmpmu_audio_init();
		pll_IsOn = 1;
	}

	bcmpmu_ihf_set_gain(PMU_IHFGAIN_MUTE);
	bcmpmu_ihf_power(1);
#endif
	ihf_IsOn = 1;
#endif
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

#ifdef THIRD_PARTY_PMU

	dialog_audio_ihf_poweroff();

#else

	ihf_IsOn = 0;
#if defined(CONFIG_IHF_EXT_AMPLIFIER)
	audio_gpio_output(GPIO_IHF_EXT_AMP, 0);
#else
	bcmpmu_ihf_set_gain(PMU_IHFGAIN_MUTE);
	bcmpmu_ihf_power(0);

	if (ihf_IsOn == 0 && hs_IsOn == 0)
		/*disable the audio PLL after power OFF*/
		if (pll_IsOn == 1) {
			bcmpmu_audio_deinit();
			pll_IsOn = 0;
		}

#endif
#endif
}

/********************************************************************
*  @brief  power on external stereo speaker amplifier and switch
*
*  @param  none
*  @return none
*
****************************************************************************/
void extern_stereo_speaker_on(void)
{
#if defined(CONFIG_STEREO_SPEAKER)
	return;
#else
	extern_ihf_on();
	return;
#endif
}

/********************************************************************
*  @brief  power off external stereo speaker amplifier and switch
*
*  @param  none
*  @return none
*
****************************************************************************/
void extern_stereo_speaker_off(void)
{
#if defined(CONFIG_STEREO_SPEAKER)
	return;
#else
	extern_ihf_off();
	return;
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

#ifdef THIRD_PARTY_PMU

      /*There is no gain mapping as below..is that ok??*/
	if (lr == AUDIO_HS_LEFT)
		dialog_audio_hs_set_gain(AUDIO_HS_LEFT, gain_mB);
	else if (lr == AUDIO_HS_RIGHT)
		dialog_audio_hs_set_gain(AUDIO_HS_RIGHT, gain_mB);
	else if (lr == AUDIO_HS_BOTH)
		dialog_audio_hs_set_gain(AUDIO_HS_BOTH, gain_mB);

#else

	BCMPMU_Audio_HS_Param hs_param;
	struct PMU_AudioGainMapping_t gain_map;

	gain_map = map2pmu_hs_gain(gain_mB);
	aTrace(LOG_AUDIO_CNTLR, "%s need %d, pmu_gain_enum=0x%x\n",
			__func__, gain_mB, gain_map.PMU_gain_enum);

	hs_param = bcmpmu_get_hs_param_from_audio_driver();

	if (lr == AUDIO_HS_BOTH) {
		bcmpmu_hs_set_gain(PMU_AUDIO_HS_BOTH,
			gain_map.PMU_gain_enum);
		hs_gain_l = gain_map.gain_mB;
		hs_gain_r = gain_map.gain_mB;

		hs_param.hs_gain_left = gain_map.PMU_gain_enum;
		hs_param.hs_gain_right = gain_map.PMU_gain_enum;
	} else if (lr == AUDIO_HS_LEFT) {
		bcmpmu_hs_set_gain(PMU_AUDIO_HS_LEFT,
			gain_map.PMU_gain_enum);
		hs_gain_l = gain_map.gain_mB;

		hs_param.hs_gain_left = gain_map.PMU_gain_enum;
	} else if (lr == AUDIO_HS_RIGHT) {
		bcmpmu_hs_set_gain(PMU_AUDIO_HS_RIGHT,
			gain_map.PMU_gain_enum);
		hs_gain_r = gain_map.gain_mB;

		hs_param.hs_gain_right = gain_map.PMU_gain_enum;
	}

	bcmpmu_audio_driver_set_hs_param(hs_param);
#endif
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
	BCMPMU_Audio_HS_Param hs_param;

	hs_param = bcmpmu_get_hs_param_from_audio_driver();

	if (lr == AUDIO_HS_BOTH) {
		bcmpmu_hs_set_gain(PMU_AUDIO_HS_BOTH, PMU_HSGAIN_MUTE);

		hs_param.hs_gain_left = PMU_HSGAIN_MUTE;
		hs_param.hs_gain_right = PMU_HSGAIN_MUTE;
	} else if (lr == AUDIO_HS_LEFT) {
		bcmpmu_hs_set_gain(PMU_AUDIO_HS_LEFT, PMU_HSGAIN_MUTE);

		hs_param.hs_gain_left = PMU_HSGAIN_MUTE;
	} else if (lr == AUDIO_HS_RIGHT) {
		bcmpmu_hs_set_gain(PMU_AUDIO_HS_RIGHT, PMU_HSGAIN_MUTE);

		hs_param.hs_gain_right = PMU_HSGAIN_MUTE;
	}

	bcmpmu_audio_driver_set_hs_param(hs_param);
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

#ifdef THIRD_PARTY_PMU

	aTrace(LOG_AUDIO_CNTLR, "%s gain_mB=%d\n",
			__func__, gain_mB);

	dialog_audio_ihf_set_gain(gain_mB);

#else

	struct PMU_AudioGainMapping_t gain_map;

	gain_map = map2pmu_ihf_gain(gain_mB);
	aTrace(LOG_AUDIO_CNTLR, "%s gain_mB=%d, pmu_gain_enum=0x%x\n",
			__func__, gain_mB, gain_map.PMU_gain_enum);

	bcmpmu_ihf_set_gain(gain_map.PMU_gain_enum);

	ihf_gain = gain_map.gain_mB;

#endif

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


/********************************************************************
*  @brief  Set IHF Noise gate parameter
*
*  @param  IHF Noise parameter
*  @return  none
*
****************************************************************************/

void extern_set_ihf_noise_gate(int param_value)
{
#ifdef THIRD_PARTY_PMU

	dialog_noise_gate_t noise_gate;

	noise_gate.cfg = (param_value & 0xC0) >> 6;
	noise_gate.atk = (param_value & 0x0038) >> 3;
	noise_gate.deb = (param_value & 0x06) >> 1;
	noise_gate.en  = (param_value & 0x01);
	noise_gate.rms = (param_value & 0x1800) >> 11;
	noise_gate.rel = (param_value & 0x700) >> 8;

	aTrace(LOG_AUDIO_CNTLR, "%s cfg=%d, atk=%d, deb =%d\n",
		__func__, noise_gate.cfg, noise_gate.atk, noise_gate.deb);

	aTrace(LOG_AUDIO_CNTLR, "en =%d, rms =%d, rel =%d\n" ,
		noise_gate.en, noise_gate.rms, noise_gate.rel);

	dialog_set_ihf_noise_gate(noise_gate);

#endif
}


/********************************************************************
*  @brief  Set HS Noise gate parameter
*
*  @param  HS Noise parameter
*  @return  none
*
****************************************************************************/

void extern_set_hs_noise_gate(int param_value)
{
#ifdef THIRD_PARTY_PMU
	dialog_noise_gate_t noise_gate;

	noise_gate.cfg = (param_value & 0xC0) >> 6;
	noise_gate.atk = (param_value & 0x0038) >> 3;
	noise_gate.deb = (param_value & 0x06) >> 1;
	noise_gate.en  = (param_value & 0x01);
	noise_gate.rms = (param_value & 0x1800) >> 11;
	noise_gate.rel = (param_value & 0x700) >> 8;


	aTrace(LOG_AUDIO_CNTLR, "%s cfg=%d, atk=%d, deb =%d\n",
		__func__, noise_gate.cfg, noise_gate.atk, noise_gate.deb);

	aTrace(LOG_AUDIO_CNTLR, "en =%d, rms =%d, rel =%d\n" ,
		noise_gate.en, noise_gate.rms, noise_gate.rel);


	dialog_set_hs_noise_gate(noise_gate);
#endif

}

/********************************************************************
*  @brief  Set IHF None Clip parameter
*
*  @param  None Clip parameter
*  @return  none
*
****************************************************************************/

void extern_set_ihf_none_clip(int param_value)
{

#ifdef THIRD_PARTY_PMU

	dialog_spk_nonclip_t non_clip;

	non_clip.zc_en = (param_value & 0x80) >> 7;
	non_clip.rel = (param_value & 0x70) >> 4;
	non_clip.atk = (param_value & 0xe) >> 1;
	non_clip.en = (param_value & 0x01);
	non_clip.hld = (param_value & 0xC000) >> 14;
	non_clip.thd = (param_value & 0x3f00) >> 8;

	aTrace(LOG_AUDIO_CNTLR, "%s zc_en=%d, rel=%d, atk =%d\n",
		__func__, non_clip.zc_en, non_clip.rel, non_clip.atk);

	aTrace(LOG_AUDIO_CNTLR, "en =%d, hld =%d, thd=%d\n",
		non_clip.en, non_clip.hld, non_clip.thd);

	dialog_set_ihf_none_clip(non_clip);
#endif
}


/********************************************************************
*  @brief  Set IHF pwr parameter
*
*  @param  pwr parameter
*  @return  none
*
****************************************************************************/


void extern_set_ihf_pwr(int param_value)
{

#ifdef THIRD_PARTY_PMU
	dialog_spk_pwr_t spk_pwr;

	spk_pwr.pwr = (param_value & 0x7e) >> 1;
	spk_pwr.limit_en = (param_value & 0x01);

	aTrace(LOG_AUDIO_CNTLR, "%s pwr=%d , limit_en = %d\n",
		__func__, spk_pwr.pwr, spk_pwr.limit_en);

	dialog_set_ihf_pwr(spk_pwr);

#endif

}

/********************************************************************
*  @brief  Set IHF preamp gain
*
*  @param  gain_mB	requested IHF preamp gain
*  @return  none
*
****************************************************************************/

void extern_set_ihf_preamp_gain(int gain_mB)
{
#ifdef THIRD_PARTY_PMU
	dialog_set_ihf_preamp_gain(gain_mB);

	aTrace(LOG_AUDIO_CNTLR, "%s gain_mB=%d",
		__func__, gain_mB);

#endif

}

/********************************************************************
*  @brief  Set HS preamp gain
*
*  @param  gain_mB	requested HS preamp gain
*  @return  none
*
****************************************************************************/

void extern_set_hs_preamp_gain(int gain_mB)
{
#ifdef THIRD_PARTY_PMU
	dialog_set_hs_preamp_gain(gain_mB);

	aTrace(LOG_AUDIO_CNTLR, "%s gain_mBr=%d",
		__func__, gain_mB);

#endif

}

/********************************************************************
*  @brief  Set Multicast on/off on PMU
*
*  @param  flag	IHF on/off for multicasting
*  @return  none
*
****************************************************************************/

void extern_start_stop_multicast(unsigned char flag)
{

#ifdef THIRD_PARTY_PMU


	aTrace(LOG_AUDIO_CNTLR, "%s flag=%d",
					__func__, flag);

	dialog_turn_multicast_onoff(flag);

#endif


}

#if defined(CONFIG_MFD_BCM59039) | defined(CONFIG_MFD_BCM59042)
/********************************************************************
*  @brief  Get headset gain (mB)
*
*  @param  gain_mB	returned Headset gain
*  @return  none
*
****************************************************************************/
void extern_hs_get_gain(int *pGainL_mB, int *pGainR_mB)
{

		*pGainL_mB = hs_gain_l;
		*pGainR_mB = hs_gain_r;
}
#endif
#endif
