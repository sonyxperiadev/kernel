/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2010  Broadcom Corporation                                                        */
/*                                                                                              */
/*     Unless you and Broadcom execute a separate written software license agreement governing  */
/*     use of this software, this software is licensed to you under the terms of the GNU        */
/*     General Public License version 2 (the GPL), available at                                 */
/*                                                                                              */
/*          http://www.broadcom.com/licenses/GPLv2.php                                          */
/*                                                                                              */
/*     with the following added to such license:                                                */
/*                                                                                              */
/*     As a special exception, the copyright holders of this software give you permission to    */
/*     link this software with independent modules, and to copy and distribute the resulting    */
/*     executable under terms of your choice, provided that you also meet, for each linked      */
/*     independent module, the terms and conditions of the license of that module.              */
/*     An independent module is a module which is not derived from this software.  The special  */
/*     exception does not apply to any modifications of the software.                           */
/*                                                                                              */
/*     Notwithstanding the above, under no circumstances may you combine this software in any   */
/*     way with any other Broadcom software provided under a license other than the GPL,        */
/*     without Broadcom's express prior written consent.                                        */
/*                                                                                              */
/************************************************************************************************/

/*
 * Why seperate kona_mic_bias.c ?
 * There is one Mic BIAS block that supplies the required voltage for
 * AUXMIC BIAS as well as MICBIAS.
 * AUXMIC BIAS is used by headset device and its voltage is controlled by the
 * AUX_EN register (0.4V or 2.1V etc). Similar control is available for the on
 * board MIC in CAPH register block. But since One block is supplying voltage
 * to both the modules, switching this block ON and OFF needs to be
 * serialized. Also, we need to switch this block OFF when both the On board
 * MIC and the headset MIC is not being used, so as to save power.
 */

#include <linux/module.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/mutex.h>
#include <asm/io.h>

#include <mach/hardware.h>
#include <mach/rdb/brcm_rdb_aci.h>
#include <mach/rdb/brcm_rdb_auxmic.h>

#define ACI_BIAS_PWRDN_POWERUP		0
#define ACI_BIAS_PWRDN_POWERDOWN	1

#define AUDIORX_BIAS_PWRUP_POWERUP	1
#define AUDIORX_BIAS_PWRUP_POWERDOWN	0

static int mic_bias_user_count;
DEFINE_MUTEX(mic_bias_lock);

/**
 * kona_mic_bias_on - Switch ON the MIC BIAS.
 *
 * Writes to the register if the MICBIAS block is not already ON.
 * and increments the user count. In case if the block is already ON,
 * just increments the user count
 *
 * NOTE that this funciton SHOULD NOT be called from interrupt/soft irq
 * context.
 */
void kona_mic_bias_on(void)
{
	unsigned long val = 0;
	mutex_lock(&mic_bias_lock);
	pr_debug(" === mic_bias_user_count %d \r\n", mic_bias_user_count);
	if (mic_bias_user_count++ == 0) {
		pr_debug("Actually Turning ON the MIC BIAS \r\n");
		val = readl(KONA_ACI_VA + ACI_SLEEP_CTRL_OFFSET);
		val |= ACI_SLEEP_CTRL_WEAK_SLEEP_EN_MASK;
		writel(val, KONA_ACI_VA + ACI_SLEEP_CTRL_OFFSET);

		val = readl(KONA_ACI_VA + ACI_MIC_BIAS_OFFSET);
		val &= ~ACI_MIC_BIAS_MIC_AUX_BIAS_GND_MASK;
		writel(val, KONA_ACI_VA + ACI_MIC_BIAS_OFFSET);

		val = readl(KONA_ACI_VA + ACI_ACI_CTRL_OFFSET);
		val &= ~ACI_DSP_ACI_CTRL_ACI_BIAS_PWRDN_MASK;
		val |= ACI_BIAS_PWRDN_POWERUP <<
			ACI_DSP_ACI_CTRL_ACI_BIAS_PWRDN_SHIFT;
		writel(val, KONA_ACI_VA + ACI_ACI_CTRL_OFFSET);

		val = readl(KONA_ACI_VA + ACI_ADC_CTRL_OFFSET);
		val &= ~ACI_DSP_ADC_CTRL_AUDIORX_BIAS_PWRUP_MASK;
		val |= AUDIORX_BIAS_PWRUP_POWERUP <<
			ACI_DSP_ADC_CTRL_AUDIORX_BIAS_PWRUP_SHIFT;
		writel(val, KONA_ACI_VA + ACI_ADC_CTRL_OFFSET);
	}
	mutex_unlock(&mic_bias_lock);
}
EXPORT_SYMBOL(kona_mic_bias_on);


/**
 * kona_mic_bias_off - Switch OFF the MIC BIAS.
 *
 * Writes to the register if the MICBIAS block is not already OFF.
 * and decrements the user count. In case if the block is already OFF,
 * just decrements the user count
 *
 * NOTE that this funciton SHOULD NOT be called from interrupt/soft irq
 * context.
 */
void kona_mic_bias_off(void)
{
	unsigned long val = 0;
	/*
	 * Steps to Power OFF the Mic BIAS -
	 * "Besides the F_PWRDWN (0x3500E028) and AUXEN (0x3500E014),
	 * please make sure the following registers are programmed
	 * correct value (in ACI) before you program AUXMIC registers:
	 *
	 * SLEEP_CTRL WEAK_SLEEP_EN=0x0
	 * ADC_CTRL AUDIORX_VREF_PWRUP and AUDIORX_BIAS_PWRUP are 0s
	 * ACI_CTRL ACI_BIAS_PWRDN=0x1
	 */
	mutex_lock(&mic_bias_lock);
	pr_debug(" === mic_bias_user_count %d \r\n", mic_bias_user_count);
	if (mic_bias_user_count > 0)
		--mic_bias_user_count;

	if (mic_bias_user_count == 0) {
		pr_debug("Actually Turning OFF the MIC BIAS \r\n");

		writel(1, KONA_AUXMIC_VA + AUXMIC_F_PWRDWN_OFFSET);
		writel(0, KONA_AUXMIC_VA + AUXMIC_AUXEN_OFFSET);

		val = readl(KONA_ACI_VA + ACI_SLEEP_CTRL_OFFSET);
		val = val & ~ACI_SLEEP_CTRL_WEAK_SLEEP_EN_MASK;
		writel(val, KONA_ACI_VA + ACI_SLEEP_CTRL_OFFSET);

		val = readl(KONA_ACI_VA + ACI_MIC_BIAS_OFFSET);
		val = val | 1;
		writel(val, KONA_ACI_VA + ACI_MIC_BIAS_OFFSET);

		val = readl(KONA_ACI_VA + ACI_ADC_CTRL_OFFSET);
		val &= ~ACI_DSP_ADC_CTRL_AUDIORX_BIAS_PWRUP_MASK;
		val |= AUDIORX_BIAS_PWRUP_POWERDOWN <<
			ACI_DSP_ADC_CTRL_AUDIORX_BIAS_PWRUP_SHIFT;
		writel(val, KONA_ACI_VA + ACI_ADC_CTRL_OFFSET);

		val = readl(KONA_ACI_VA + ACI_ACI_CTRL_OFFSET);
		val &= ~ACI_DSP_ACI_CTRL_ACI_BIAS_PWRDN_MASK;
		val |= ACI_BIAS_PWRDN_POWERDOWN <<
			ACI_DSP_ACI_CTRL_ACI_BIAS_PWRDN_SHIFT;
		writel(val, KONA_ACI_VA + ACI_ACI_CTRL_OFFSET);
	}
	mutex_unlock(&mic_bias_lock);
}
EXPORT_SYMBOL(kona_mic_bias_off);
