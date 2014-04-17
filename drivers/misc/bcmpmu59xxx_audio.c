/*******************************************************************************
* Copyright 2001 - 2012 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license
* other than the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/bug.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <linux/fs.h>
#ifdef CONFIG_DEBUG_FS
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#endif
#include <linux/mfd/bcmpmu59xxx.h>
#include <linux/mfd/bcmpmu59xxx_reg.h>
#include <linux/broadcom/bcmpmu_audio.h>

static int debug_mask = BCMPMU_PRINT_ERROR | BCMPMU_PRINT_INIT ;
module_param_named(dbgmsk, debug_mask, int, S_IRUGO | S_IWUSR | S_IWGRP);
#define pr_audio(debug_level, args...) \
	do { \
		if (debug_mask & BCMPMU_PRINT_##debug_level) { \
			pr_info(args); \
		} \
	} while (0)
/*
 * Steps to increment IHF/HS gain exponentially
 */
static const u8 hs_gain_steps[] = {
	0x00, 0x0A, 0x0F, 0x1E, 0x3C, 0x3F,
};

#define HS_GAIN_NSTEPS       ARRAY_SIZE(hs_gain_steps)
#define MAX_HS_GAIN          (0x3F)

static const u8 ihf_gain_steps[] = {
	0x00, 0x0A, 0x0F, 0x1E, 0x3C, 0x3F,
};

#define IHF_GAIN_NSTEPS      ARRAY_SIZE(ihf_gain_steps)
#define MAX_IHF_GAIN         (0x3F)

/* Debug interface to disable gain ramping from
 * command line. To disable gain ramping, set the gain
 * steps to 0.
 */
static int hs_gain_nsteps = HS_GAIN_NSTEPS;
module_param_named(hs_gain_nsteps, hs_gain_nsteps, int,
		   S_IRUGO | S_IWUSR | S_IWGRP);

static int ihf_gain_nsteps = IHF_GAIN_NSTEPS;
module_param_named(ihf_gain_nsteps, ihf_gain_nsteps, int,
		   S_IRUGO | S_IWUSR | S_IWGRP);

struct bcmpmu_audio {
	struct bcmpmu59xxx *bcmpmu;
	bool IHF_On;
	bool HS_On;
	struct mutex lock;
#ifdef CONFIG_DEBUG_FS
	void *debugfs_dir;
#endif
	u32 pll_use_count;
	int ihf_autoseq_dis;
	struct delayed_work irq_work;
	int event;
};

static struct bcmpmu_audio *pmu_audio;

enum {
	GAIN_RAMP_UP = 0,
	GAIN_RAMP_NIL = 1,
	GAIN_RAMP_DOWN = 2,
};

/* Callers of get_gain_ramp API must pass this structure to the API
 * with the 'table' and 'size' parameters initialized to the correct
 * gain table. The API returns the ramp sequence in 'start', 'end'
 * and 'dir' fields.
 */
struct gain_ramp {
	/* Table of gain values in increasing order */
	const u8 *table;
	/* Number of gain entries in the table */
	int size;
	/* Start index of the ramp sequence */
	int start;
	/* End index of the ramp sequence */
	int end;
	/* GAIN_RAMP_UP, GAIN_RAMP_NIL, GAIN_RAMP_DOWN */
	int dir;
};

static BCMPMU_Audio_HS_Param headset_param_set_from_audio_driver;

/* Description:
 *     Returns the index of the first step for a ramp-up
 *     or ramp-down sequence. For ramp-up, the start index
 *     is the first gain value above the input gain, searching
 *     from 0 to max. For ramp-down, the start index is
 *     the first gain value below the input gain, searching
 *     from max to 0.
 */
static int get_start_index(const u8 *table, int nsteps, u8 gain, int dir)
{
	int i;

	if (dir == GAIN_RAMP_UP) {
		for (i = 0; i < nsteps; i++)
			if (table[i] > gain)
				break;
	} else {
		for (i = nsteps - 1; i >= 0; i--)
			if (table[i] < gain)
				break;
	}

	return i;
}

/* Description:
 *     Returns the index of the last step for a ramp-up
 *     or ramp-down sequence. For ramp-up, the end index
 *     is the first gain value below the input gain, searching
 *     from max to 0. For ramp-down, the end index is
 *     the first gain value above the input gain, searching
 *     from 0 to max.
 */
static int get_end_index(const u8 *table, int nsteps, u32 gain, int dir)
{
	int i;

	if (dir == GAIN_RAMP_UP) {
		for (i = nsteps - 1; i >= 0; i--)
			if (table[i] < gain)
				break;
	} else {
		for (i = 0; i < nsteps; i++)
			if (table[i] > gain)
				break;
	}

	return i;
}

/* Description:
 *     Returns the ramp sequence to ramp the gain from 'old'
 *     to 'new'.
 * Assumes:
 *     'table' and 'size' fields of gain_ramp struct are initialized
 *     with data of the desired ramp table.
 * Returns:
 *     ramp direction in gain_ramp->dir: GAIN_RAMP_UP, GAIN_RAMP_DOWN
 *     or GAIN_RAMP_NIL
 */
static void get_gain_ramp(u32 old, u32 new, struct gain_ramp *ramp)
{
	if (old != new) {
		ramp->dir = (new > old) ? GAIN_RAMP_UP : GAIN_RAMP_DOWN;
		ramp->start = get_start_index(ramp->table, ramp->size, old,
					      ramp->dir);
		ramp->end = get_end_index(ramp->table, ramp->size, new,
					  ramp->dir);
	} else {
		ramp->dir = GAIN_RAMP_NIL;
	}
}

/* Description:
 *    Configures the input gain to the register.
 * Inputs:
 *    reg  - register enum
 *    data - data read from the register (callee will have to read the
 *           register data and pass it in this parameter).
 *    mask - mask for the gain bits.
 *    gain - new gain value
 */
static void _bcmpmu_set_gain(u32 reg, u8 gain)
{
	struct bcmpmu59xxx *bcmpmu = pmu_audio->bcmpmu;

	pr_audio(FLOW, "%s: reg = 0x%x, gain = 0x%x\n", __func__, reg, gain);

	bcmpmu->write_dev(bcmpmu, reg, gain);
	udelay(50);
}

static void bcmpmu_set_gain(struct gain_ramp *ramp, u32 reg, u8 new_gain)
{
	struct bcmpmu59xxx *bcmpmu = pmu_audio->bcmpmu;
	u8 cur_gain;
	int i;

	/* Read present gain setting */
	bcmpmu->read_dev(bcmpmu, reg, &cur_gain);

	/* Compute the ramp sequence */
	get_gain_ramp(cur_gain, new_gain, ramp);

	/* Ramp gain */
	if (ramp->dir == GAIN_RAMP_UP) {
		/* Program the ramp */
		for (i = ramp->start; i <= ramp->end; i++)
			_bcmpmu_set_gain(reg, ramp->table[i]);

		/* Program the target gain */
		_bcmpmu_set_gain(reg, new_gain);

	} else if (ramp->dir == GAIN_RAMP_DOWN) {
		/* Program the ramp */
		for (i = ramp->start; i >= ramp->end; i--)
			_bcmpmu_set_gain(reg, ramp->table[i]);

		/* Program the target gain */
		_bcmpmu_set_gain(reg, new_gain);
	}
}

/* callee of this API need to put 20ms delay to
 * make sure power up seq done properly by h/w
*/
void bcmpmu_hs_power(bool on)
{
	u8 val1, val2, val3;
	struct bcmpmu59xxx *bcmpmu;
	if (!pmu_audio)
		return;
	bcmpmu = pmu_audio->bcmpmu;
	pr_audio(FLOW, "%s: ON = %d\n", __func__, on);
	mutex_lock(&pmu_audio->lock);

	bcmpmu->read_dev(bcmpmu, PMU_REG_HSPGA3, &val3);
	bcmpmu->read_dev(bcmpmu, PMU_REG_HSPUP1, &val1);
	bcmpmu->read_dev(bcmpmu, PMU_REG_HSPUP2, &val2);
	if (on) {
		val1 &= ~HSPUP1_IDDQ_PWRDN;
		val2 |= HSPUP2_HS_PWRUP;
		val3 |= HSPGA3_RAMUP_DIS;
	} else {
		val1 |= HSPUP1_IDDQ_PWRDN;
		val2 &= ~(1 << HSPUP2_HS_PWRUP_SHIFT);
		val3 &= ~HSPGA3_RAMUP_DIS;
	}
	pmu_audio->HS_On = on;
	bcmpmu->write_dev(bcmpmu, PMU_REG_HSPGA3, val3);
	bcmpmu->write_dev(bcmpmu, PMU_REG_HSPUP1, val1);
	bcmpmu->write_dev(bcmpmu, PMU_REG_HSPUP2, val2);

	bcmpmu->read_dev(bcmpmu, PMU_REG_HSPGA3, &val3);
	pr_audio(FLOW, "%s: PMU_REG_HSPGA3 %x\n",
			__func__, val3);
	mutex_unlock(&pmu_audio->lock);
}
EXPORT_SYMBOL(bcmpmu_hs_power);

void bcmpmu_hs_shortcircuit_dis(bool disable)
{
	struct bcmpmu59xxx *bcmpmu;
	u8 val;
	if (!pmu_audio)
		return;
	bcmpmu = pmu_audio->bcmpmu;
	mutex_lock(&pmu_audio->lock);
	bcmpmu->read_dev(bcmpmu, PMU_REG_HSDRV, &val);

	if (disable)
		val |= (1 << HSDRV_DISSC_SHIFT);
	else
		val &= ~(1 << HSDRV_DISSC_SHIFT);

	bcmpmu->write_dev(bcmpmu, PMU_REG_HSDRV, val);
	mutex_unlock(&pmu_audio->lock);

}
EXPORT_SYMBOL(bcmpmu_hs_shortcircuit_dis);

int bcmpmu_hs_set_input_mode(int HSgain, int HSInputmode)
{
	struct bcmpmu59xxx *bcmpmu;
	u8 temp;
	u32 data1, data2, data3;
	int ret = 0;
	int HSwasEn = 0;
	if (!pmu_audio)
		return -ENODEV;
	bcmpmu = pmu_audio->bcmpmu;
	pr_audio(FLOW, "Inside %s, HSgain %d, HSInputmode %d\n",
			__func__, HSgain, HSInputmode);

	mutex_lock(&pmu_audio->lock);
	if (pmu_audio->HS_On) {
		pmu_audio->HS_On = false;
		bcmpmu->read_dev(bcmpmu, PMU_REG_HSPUP2, &temp);
		temp &= ~(1 << HSPUP2_HS_PWRUP_SHIFT);
		bcmpmu->write_dev(bcmpmu, PMU_REG_HSPUP2, temp);
		HSwasEn = 1;
	}

	bcmpmu->read_dev(bcmpmu, PMU_REG_HSPGA1, (u8 *)&data1);
	bcmpmu->read_dev(bcmpmu, PMU_REG_HSPGA1, (u8 *)&data2);
	bcmpmu->read_dev(bcmpmu, PMU_REG_HSPGA3, (u8 *)&data3);
	if (HSInputmode == PMU_HS_SINGLE_ENDED_AC_COUPLED) {

		/*add 6 dB shift if input mode is PMU_HS_SINGLE_ENDED,
		  therefore the HS gain is the same for PMU_HS_SINGLE_ENDED
		  and PMU_HS_DIFFERENTIAL. */

#if defined(CONFIG_MFD_BCM59055)
		/*for 59055, i_pga_gainl==1, boost 6dB */
		data1 |= (BCMPMU_HSPGA1_PGA_GAINL | BCMPMU_HSPGA1_PGA_GAINR |
				(HSgain & BCMPMU_PGA_CTL_MASK));
#endif
#if defined(CONFIG_MFD_BCM59039) || defined(CONFIG_MFD_BCM_PMU59xxx)
		/* for 59039, i_pga_gainl==0, boost 6dB */
		data1 &= ~(BCMPMU_HSPGA1_PGA_GAINL | BCMPMU_HSPGA1_PGA_GAINR |
				BCMPMU_PGA_CTL_MASK);
		data1 |= HSgain & BCMPMU_PGA_CTL_MASK;
#endif

		data2 &= ~BCMPMU_PGA_CTL_MASK;
		data2 |= HSgain & BCMPMU_PGA_CTL_MASK;

		data3 &= ~(BCMPMU_HSPGA3_PGA_ACINADJ |
				BCMPMU_HSPGA3_PGA_PULLDNSJ);
		data3 |= BCMPMU_HSPGA3_PGA_ENACCPL;
	} else if (HSInputmode == PMU_HS_DIFFERENTIAL_AC_COUPLED) {

#if defined(CONFIG_MFD_BCM59055)
		/*for 59055, i_pga_gainl==0, does not boost. (lower by 6dB) */
		data1 &= ~(BCMPMU_HSPGA1_PGA_GAINL | BCMPMU_HSPGA1_PGA_GAINR |
				BCMPMU_PGA_CTL_MASK);
		data1 |= HSgain & BCMPMU_PGA_CTL_MASK;
#endif
#if defined(CONFIG_MFD_BCM59039) || defined(CONFIG_MFD_BCM_PMU59xxx)
		/* for 59039, i_pga_gainl==1, does not boost. (lower by 6dB) */
		data1 |= (BCMPMU_HSPGA1_PGA_GAINL | BCMPMU_HSPGA1_PGA_GAINR |
				(HSgain & BCMPMU_PGA_CTL_MASK));
#endif

		data2 &= ~BCMPMU_PGA_CTL_MASK;
		data2 |= HSgain & BCMPMU_PGA_CTL_MASK;

		data3 &= ~BCMPMU_HSPGA3_PGA_PULLDNSJ;
		data3 |= (BCMPMU_HSPGA3_PGA_ENACCPL |
				BCMPMU_HSPGA3_PGA_ACINADJ);
	} else if (HSInputmode == PMU_HS_DIFFERENTIAL_DC_COUPLED) {

#if defined(CONFIG_MFD_BCM59055)
		/*for 59055, i_pga_gainl==0, does not boost. (lower by 6dB) */
		data1 &= ~(BCMPMU_HSPGA1_PGA_GAINL | BCMPMU_HSPGA1_PGA_GAINR |
				BCMPMU_PGA_CTL_MASK);
		data1 |= HSgain & BCMPMU_PGA_CTL_MASK;
#endif
#if defined(CONFIG_MFD_BCM59039) || defined(CONFIG_MFD_BCM_PMU59xxx)
		/* for 59039, i_pga_gainl==1, does not boost. (lower by 6dB) */
		data1 |= (BCMPMU_HSPGA1_PGA_GAINL | BCMPMU_HSPGA1_PGA_GAINR |
				(HSgain & BCMPMU_PGA_CTL_MASK));
#endif

		data2 &= ~BCMPMU_PGA_CTL_MASK;
		data2 |= HSgain & BCMPMU_PGA_CTL_MASK;

		data3 &= ~(BCMPMU_HSPGA3_PGA_PULLDNSJ |
				BCMPMU_HSPGA3_PGA_ENACCPL |
				BCMPMU_HSPGA3_PGA_ACINADJ);
	}

	ret = bcmpmu->write_dev(bcmpmu, PMU_REG_HSPGA1, (u8)data1);
	ret |= bcmpmu->write_dev(bcmpmu, PMU_REG_HSPGA2, (u8)data2);
	ret |= bcmpmu->write_dev(bcmpmu, PMU_REG_HSPGA3, (u8)data3);

	/* Power Up HS */
	if (HSwasEn) {
		pmu_audio->HS_On = true;
		bcmpmu->read_dev(bcmpmu, PMU_REG_HSPUP2, (u8 *)&data1);
		data1 |= 1 << HSPUP2_HS_PWRUP_SHIFT;
		bcmpmu->write_dev(bcmpmu, PMU_REG_HSPUP2, (u8)data1);
	}
	mutex_unlock(&pmu_audio->lock);

	return ret;
}
EXPORT_SYMBOL(bcmpmu_hs_set_input_mode);

void bcmpmu_hs_set_gain(bcmpmu_hs_path_t path, bcmpmu_hs_gain_t gain)
{
	struct gain_ramp ramp = {
		.table = hs_gain_steps,
		.size = hs_gain_nsteps,
	};
	if (!pmu_audio)
		return;
	pr_audio(FLOW, "%s: path = %d, gain = %d\n", __func__, path, gain);

	mutex_lock(&pmu_audio->lock);

	if (path == PMU_AUDIO_HS_LEFT || path == PMU_AUDIO_HS_BOTH)
		bcmpmu_set_gain(&ramp, PMU_REG_HSPGA1, gain);

	if (path == PMU_AUDIO_HS_RIGHT || path == PMU_AUDIO_HS_BOTH)
		bcmpmu_set_gain(&ramp, PMU_REG_HSPGA2, gain);

	mutex_unlock(&pmu_audio->lock);
}
EXPORT_SYMBOL(bcmpmu_hs_set_gain);

static void bcmpmu_ihf_manual_power(bool on)
{
	u8 temp;
	struct bcmpmu59xxx *bcmpmu;
	if (!pmu_audio)
		return;
	bcmpmu = pmu_audio->bcmpmu;
	pr_audio(FLOW, "%s: ON = %d\n", __func__, on);

	mutex_lock(&pmu_audio->lock);
	if (on) {
		if (pmu_audio->IHF_On) {
			mutex_unlock(&pmu_audio->lock);
			return;
		}
		pmu_audio->IHF_On = true;
		/* Enable IHFLDOPUP */
		bcmpmu->read_dev(bcmpmu, PMU_REG_IHFLDO, &temp);
		temp |= IHFLDO_PUP;
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFLDO, temp);

		/* Enable  IHF CLK pup BIAS */
		bcmpmu->read_dev(bcmpmu, PMU_REG_IHFCLK, &temp);
		temp |= (IHFCLK_PUP | IHFCLK_IHFBIASEN);
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFCLK, temp);

		/* wait for 1ms */
		usleep_range(1000, 2000);
		/* Enable  IHF RC CAL pup */
		bcmpmu->read_dev(bcmpmu, PMU_REG_IHFRCCAL, &temp);
		temp |= IHFRCCAL_PUP;
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFRCCAL, temp);

		/* wait for 1.5ms */
		usleep_range(1500, 3000);
		/* Enable  IHF Forward Feed PUP */
		bcmpmu->read_dev(bcmpmu, PMU_REG_IHFFF, &temp);
		temp |= IHFFF_PUP;
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFFF, temp);

		/* Enable  IHF LOOP FILTER pup */
		bcmpmu->read_dev(bcmpmu, PMU_REG_IHFLF, &temp);
		temp |= IHFLF_PUP;
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFLF, temp);

		/* Enable  IHF Comparator and Phase Detector */
		bcmpmu->read_dev(bcmpmu, PMU_REG_IHFCMPPD, &temp);
		temp |= IHFCMPPD_PUP;
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFCMPPD, temp);

		/* Enable  IHF Feedback Amp pup */
		bcmpmu->read_dev(bcmpmu, PMU_REG_IHFFB, &temp);
		temp |= IHFFB_PUP;
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFFB, temp);

		/* Mute IHF gain */
		bcmpmu->read_dev(bcmpmu, PMU_REG_IHFPGA2_GAIN, &temp);
		temp &= (~IHFPGA2_GAIN_MASK);
		temp |= IHFPGA2_MUTE;
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFPGA2_GAIN, temp);

		/* Enable  IHF Power Driver pup */
		bcmpmu->read_dev(bcmpmu, PMU_REG_IHFPWRDRV, &temp);
		temp |= IHFPWRDRV_PUP;
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFPWRDRV, temp);

		/* Enable IHF Noise Gate pup */
		bcmpmu->read_dev(bcmpmu, PMU_REG_IHF_NGMISC, &temp);
		temp |= NGMISC_IHFNG_PUP_EN_IHFNG;
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHF_NGMISC, temp);

		/* Enable  IHF POP & IHF POP pup */
		bcmpmu->read_dev(bcmpmu, PMU_REG_IHFPOP, &temp);
		temp |= (IHFPOP_EN | IHFPOP_PUP);
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFPOP, temp);

		/* wait for 2ms */
		usleep_range(2000, 4000);
		/* Enable  IHF RAMP pup Disable  IHF Driver Clamp */
		bcmpmu->read_dev(bcmpmu, PMU_REG_IHFRAMP, &temp);
		temp |= (IHFRAMP_PUP | IHFRAMP_DRVCLAMP_DIS |
			 IHFRAMP_IHFCAL_SEL);
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFRAMP, temp);

		/* wait for 0.6ms */
		usleep_range(600, 1200);
		/* Enable  IHF PSR CAL pup */
		bcmpmu->read_dev(bcmpmu, PMU_REG_IHFPSR1, &temp);
		temp |=  IHFPSR1_PSRCAL_PUP;
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFPSR1, temp);

		/* wait for 20ms */
		msleep(20);
		/* Disable  IHF POP pup */
		bcmpmu->read_dev(bcmpmu, PMU_REG_IHFPOP, &temp);
		temp &= (~IHFPOP_PUP_MASK);
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFPOP, temp);

		/* wait for 0.6ms */
		usleep_range(600, 1200);
		/* PSR cal disable */
		bcmpmu->read_dev(bcmpmu, PMU_REG_IHFRAMP, &temp);
		temp &= (~IHFRAMP_IHFCAL_SEL_MASK);
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFRAMP, temp);

		/* Enable  IHF Driver Clamp */
		bcmpmu->read_dev(bcmpmu, PMU_REG_IHFRAMP, &temp);
		temp &= (~IHFRAMP_DRVCLAMP_DIS);
		bcmpmu->read_dev(bcmpmu, PMU_REG_IHFRAMP, &temp);

		/* wait for 0.6ms */
		usleep_range(600, 1200);
		/* Enable  IHF POP pup */
		bcmpmu->read_dev(bcmpmu, PMU_REG_IHFPOP, &temp);
		temp |= (IHFPOP_PUP_MASK);
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFPOP, temp);
	} else {
		if (!pmu_audio->IHF_On) {
			mutex_unlock(&pmu_audio->lock);
			return;
		}
		pmu_audio->IHF_On = false;
		/* Disable  IHF POP pup */
		bcmpmu->read_dev(bcmpmu, PMU_REG_IHFPOP, &temp);
		temp &= (~IHFPOP_PUP_MASK);
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFPOP, temp);
		/* wait for 1.5ms */
		usleep_range(1500, 3000);
		/* Disable  IHF RC CAL pup */
		bcmpmu->read_dev(bcmpmu, PMU_REG_IHFRCCAL, &temp);
		temp &= (~IHFRCCAL_PUP_MASK);
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFRCCAL, temp);

		/* Disable  IHF CLK pup  Disable  IHF BIAS */
		bcmpmu->read_dev(bcmpmu, PMU_REG_IHFCLK, &temp);
		temp &= ~(IHFCLK_PUP_MASK | IHFCLK_IHFBIASEN);
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFCLK, temp);

		/* Disable  IHF Forward Feed PUP */
		bcmpmu->read_dev(bcmpmu, PMU_REG_IHFFF, &temp);
		temp &= ~(IHFFF_PUP_MASK);
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFFF, temp);

		/* Disable  IHF RAMP pup */
		bcmpmu->read_dev(bcmpmu, PMU_REG_IHFRAMP, &temp);
		temp &= (~IHFRAMP_PUP);
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFRAMP, temp);

		/* Disable  IHF LOOP FILTER pup */
		bcmpmu->read_dev(bcmpmu, PMU_REG_IHFLF, &temp);
		temp &= (~IHFLF_PUP_MASK);
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFLF, temp);

		/* Disable  IHF Comparator and Phase Detector */
		bcmpmu->read_dev(bcmpmu, PMU_REG_IHFCMPPD, &temp);
		temp &= (~IHFCMPPD_PUP_MASK);
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFCMPPD, temp);

		/* Disable  IHF Feedback Amp pup */
		bcmpmu->read_dev(bcmpmu, PMU_REG_IHFFB, &temp);
		temp &= (~IHFFB_PUP);
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFFB, temp);

		/* Disable  IHF Power Driver pup */
		bcmpmu->read_dev(bcmpmu, PMU_REG_IHFPWRDRV, &temp);
		temp &= (~IHFPWRDRV_PUP);
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFPWRDRV, temp);

		/* wait for 0.5ms */
		usleep_range(500, 1000);
		/* Disable  IHF POP */
		bcmpmu->read_dev(bcmpmu, PMU_REG_IHFPOP, &temp);
		temp &= (~IHFPOP_EN);
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFPOP, temp);

		/* Disable IHFLDOPUP */
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFLDO, temp);
		temp &= (~IHFLDO_PUP);
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFLDO, temp);
	}
	mutex_unlock(&pmu_audio->lock);
}

void bcmpmu_enable_alc(bool on)
{
	struct bcmpmu59xxx *bcmpmu;
	u8 reg;
	if (!pmu_audio)
		return;
	bcmpmu = pmu_audio->bcmpmu;
	pr_audio(FLOW, "%s: ON = %d\n", __func__, on);
	mutex_lock(&pmu_audio->lock);
	bcmpmu->read_dev(bcmpmu, PMU_REG_IHFALC1, &reg);
	if (on) {
		reg |= IHF_ALC_PUP_MASK;
		reg &= ~IHFALC1_IHFALC_BYP;
		reg |= IHF_ALC_HISPEED;
	} else {
		reg &= ~IHF_ALC_PUP_MASK;
		reg |=  IHFALC1_IHFALC_BYP;
		reg &= ~IHF_ALC_HISPEED;
	}
	bcmpmu->write_dev(bcmpmu, PMU_REG_IHFALC1, reg);
	mutex_unlock(&pmu_audio->lock);
}
EXPORT_SYMBOL(bcmpmu_enable_alc);

void bcmpmu_ihf_alc_vbat_ref(bool on)
{
	struct bcmpmu59xxx *bcmpmu;
	u8 reg;
	if (!pmu_audio)
		return;
	bcmpmu = pmu_audio->bcmpmu;
	pr_audio(FLOW, "%s: ON = %d\n", __func__, on);
	mutex_lock(&pmu_audio->lock);
	bcmpmu->read_dev(bcmpmu, PMU_REG_IHFALC1, &reg);
	if (on)
		reg |= IHF_ALC_VBAT_REF_MASK;
	else
		reg &= ~IHF_ALC_VBAT_REF_MASK;
	bcmpmu->write_dev(bcmpmu, PMU_REG_IHFALC1, reg);
	mutex_unlock(&pmu_audio->lock);
}
EXPORT_SYMBOL(bcmpmu_ihf_alc_vbat_ref);

void bcmpmu_ihf_alc_thld(enum ihf_alc_thld alc_thld)
{
	struct bcmpmu59xxx *bcmpmu;
	u8 reg;
	if (!pmu_audio)
		return;
	bcmpmu = pmu_audio->bcmpmu;
	pr_audio(FLOW, "%s: param %d\n", __func__, alc_thld);
	if ((alc_thld << IHFALC2_THLD_SHIFT) >
		IHFALC2_THLD_MASK) {
		pr_audio(ERROR, "%s: Invalid param %d\n",
			__func__, alc_thld);
	} else {
		mutex_lock(&pmu_audio->lock);
		bcmpmu->read_dev(bcmpmu, PMU_REG_IHFALC2, &reg);
		reg &= ~IHFALC2_THLD_MASK;
		reg |= (alc_thld << IHFALC2_THLD_SHIFT);
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFALC2, reg);
		mutex_unlock(&pmu_audio->lock);
	}
}
EXPORT_SYMBOL(bcmpmu_ihf_alc_thld);

void bcmpmu_ihf_alc_rampup_ctrl(enum ihf_alc_ramp_up_ctrl ctrl)
{
	struct bcmpmu59xxx *bcmpmu;
	u8 reg;
	if (!pmu_audio)
		return;
	bcmpmu = pmu_audio->bcmpmu;
	pr_audio(FLOW, "%s: param %d\n", __func__, ctrl);
	if ((ctrl << IHFALC2_RAMP_UP_CTRL_SHIFT) >
		IHFALC2_RAMP_UP_CTRL_MASK) {
		pr_audio(ERROR, "%s: param %d\n",
				__func__, ctrl);
	} else {
		mutex_lock(&pmu_audio->lock);
		bcmpmu->read_dev(bcmpmu, PMU_REG_IHFALC2, &reg);
		reg &= ~IHFALC2_RAMP_UP_CTRL_MASK;
		reg |= (ctrl << IHFALC2_RAMP_UP_CTRL_SHIFT);
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFALC2, reg);
		mutex_unlock(&pmu_audio->lock);
	}
}
EXPORT_SYMBOL(bcmpmu_ihf_alc_rampup_ctrl);

void bcmpmu_ihf_alc_ramp_down_ctrl(enum ihf_alc_ramp_down_ctrl ctrl)
{
	struct bcmpmu59xxx *bcmpmu;
	u8 reg;
	if (!pmu_audio)
		return;
	bcmpmu = pmu_audio->bcmpmu;
	pr_audio(FLOW, "%s: param %d\n", __func__, ctrl);
	if ((ctrl << IHFALC2_RAMP_DOWN_CTRL_SHIFT) >
		IHFALC2_RAMP_DOWN_CTRL_MASK) {
		pr_audio(ERROR, "%s: Invalid param %d\n",
			__func__, ctrl);
	} else {
		mutex_lock(&pmu_audio->lock);
		bcmpmu->read_dev(bcmpmu, PMU_REG_IHFALC2, &reg);
		reg &= ~IHFALC2_RAMP_DOWN_CTRL_MASK;
		reg |= (ctrl << IHFALC2_RAMP_DOWN_CTRL_SHIFT);
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFALC2, reg);
		mutex_unlock(&pmu_audio->lock);
	}
}
EXPORT_SYMBOL(bcmpmu_ihf_alc_ramp_down_ctrl);

/* callee of this API need to put 65ms delay to
 * make sure power up seq done properly by h/w
 * if ihf_autoseq_dis is not set
*/
void bcmpmu_ihf_power(bool on)
{
	struct bcmpmu59xxx *bcmpmu;
	u8 temp;
	if (!pmu_audio)
		return;
	bcmpmu = pmu_audio->bcmpmu;
	pr_audio(FLOW, "%s: ON = %d\n", __func__, on);
	if (pmu_audio->ihf_autoseq_dis) {
		bcmpmu_ihf_manual_power(on);
		return;
	}
	mutex_lock(&pmu_audio->lock);
	if (on) {
		if (pmu_audio->IHF_On) {
			mutex_unlock(&pmu_audio->lock);
			return;
		}
		pmu_audio->IHF_On = true;

		/* Enable auto sequence for IHF power up and power down */
		bcmpmu->read_dev(bcmpmu, PMU_REG_IHFPOP, &temp);
		temp |= IHFAUTO_SEQ;
		temp &= ~IHFPOP_BYPASS;
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFPOP, temp);

		bcmpmu->read_dev(bcmpmu, PMU_REG_IHFLDO, &temp);
		temp |= IHFLDO_PUP;
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFLDO, temp);
	} else {
		if (!pmu_audio->IHF_On) {
			mutex_unlock(&pmu_audio->lock);
			return;
		}
		pmu_audio->IHF_On = false;

		/* Bypass IHF ALC/APS, so that IHF gain can be
		 * controlled manually
		 */
		/* Do not bypass ALC by default.
		 * audio controller will call bcmpmu_enable_alc
		 * according to sysparm configuration
		 */
		/*
		bcmpmu->read_dev(bcmpmu, PMU_REG_IHFALC1, &temp);
		temp |= IHFALC1_IHFALC_BYP;
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFALC1, temp);
		*/

		/* Toggle i_IHFpop_pup from 1 to 0. */
		bcmpmu->read_dev(bcmpmu, PMU_REG_IHFPOP, &temp);
		temp |= (IHFPOP_PUP | IHFPOP_BYPASS);
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFPOP, temp);

		bcmpmu->read_dev(bcmpmu, PMU_REG_IHFPOP, &temp);
		temp &= ~IHFPOP_PUP;	/* IHFPOP */
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFPOP, temp);

		/* Set i_IHFLDO_pup=0. */
		bcmpmu->read_dev(bcmpmu, PMU_REG_IHFLDO, &temp);
		temp &= ~IHFLDO_PUP;
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFLDO, temp);

		bcmpmu->read_dev(bcmpmu, PMU_REG_IHFPOP, &temp);
		temp &= ~IHFAUTO_SEQ;
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFPOP, temp);
	}
	mutex_unlock(&pmu_audio->lock);

}
EXPORT_SYMBOL(bcmpmu_ihf_power);

/*
void bcmpmu_ihf_bypass_en(bool enable)
{
	struct bcmpmu59xxx *bcmpmu;
	int val;
	if (!pmu_audio)
		return;
	bcmpmu = pmu_audio->bcmpmu;
	mutex_lock(&pmu_audio->lock);

	if (enable)
		val = 1 << bcmpmu->regmap[PMU_REG_IHFTOP_BYPASS].shift;
	else
		val = 0;
	bcmpmu->write_dev(bcmpmu, PMU_REG_IHFTOP_BYPASS, val,
			bcmpmu->regmap[PMU_REG_IHFTOP_BYPASS].mask);
	mutex_unlock(&pmu_audio->lock);
}
EXPORT_SYMBOL(bcmpmu_ihf_bypass_en);

*/
void bcmpmu_ihf_set_gain(bcmpmu_ihf_gain_t gain)
{
	struct gain_ramp ramp = {
		.table = ihf_gain_steps,
		.size = ihf_gain_nsteps,
	};
	if (!pmu_audio)
		return;
	pr_audio(FLOW, "%s: gain = %d\n", __func__, gain);

	mutex_lock(&pmu_audio->lock);
	bcmpmu_set_gain(&ramp, PMU_REG_IHFPGA2_GAIN, gain);
	mutex_unlock(&pmu_audio->lock);
}
EXPORT_SYMBOL(bcmpmu_ihf_set_gain);

void bcmpmu_hi_gain_mode_en(bool en)
{
	struct bcmpmu59xxx *bcmpmu;
	u8 val;
	if (!pmu_audio)
		return;
	bcmpmu = pmu_audio->bcmpmu;
	pr_audio(FLOW, "%s: hi gain enabled = %d\n", __func__, en);

	bcmpmu->read_dev(bcmpmu, PMU_REG_HIGH_GAIN_MODE, &val);
	if (en)
		val |= IHFPGA1_6DBGAIN_SHIFT;
	else
		val &= ~IHFPGA1_6DBGAIN_SHIFT;
	bcmpmu->write_dev(bcmpmu, PMU_REG_HIGH_GAIN_MODE, val);
}
EXPORT_SYMBOL(bcmpmu_hi_gain_mode_en);

BCMPMU_Audio_HS_Param bcmpmu_get_hs_param_from_audio_driver(void)
{
	return headset_param_set_from_audio_driver;
}
EXPORT_SYMBOL(bcmpmu_get_hs_param_from_audio_driver);

void bcmpmu_audio_driver_set_hs_param(BCMPMU_Audio_HS_Param hs_param)
{
	if (!pmu_audio)
		return;
	headset_param_set_from_audio_driver = hs_param;
}
EXPORT_SYMBOL(bcmpmu_audio_driver_set_hs_param);

#if 0
/* Backup registers used for headset selftest */
void bcmpmu_audio_ihf_selftest_backup(bool Enable)
{
	static unsigned int StoredRegValue[4];
	if (Enable) {
		/* Store PMU register Values */
		pmu_audio->bcmpmu->read_dev(pmu_audio->bcmpmu,
					    PMU_REG_IHFSTIN_I_IHFSELFTEST_EN,
					    &StoredRegValue[0],
					    PMU_BITMASK_ALL);
		pmu_audio->bcmpmu->read_dev(pmu_audio->bcmpmu,
					    PMU_REG_IHFSTIN_I_IHFSTI,
					    &StoredRegValue[1],
					    PMU_BITMASK_ALL);
		pmu_audio->bcmpmu->read_dev(pmu_audio->bcmpmu,
					    PMU_REG_IHFSTIN_I_IHFSTO,
					    &StoredRegValue[2],
					    PMU_BITMASK_ALL);
		pmu_audio->bcmpmu->read_dev(pmu_audio->bcmpmu,
					    PMU_REG_IHFSTO_O_IHFSTI,
					    &StoredRegValue[3],
					    PMU_BITMASK_ALL);
	} else {
		/* Restore PMU register values */
		pmu_audio->bcmpmu->write_dev(pmu_audio->bcmpmu,
					     PMU_REG_IHFSTIN_I_IHFSELFTEST_EN,
					     StoredRegValue[0],
					     pmu_audio->bcmpmu->regmap
					     [PMU_REG_IHFSTIN_I_IHFSELFTEST_EN].
					     mask);
		pmu_audio->bcmpmu->write_dev(pmu_audio->bcmpmu,
					     PMU_REG_IHFSTIN_I_IHFSTI,
					     StoredRegValue[1],
					     pmu_audio->bcmpmu->
					     regmap[PMU_REG_IHFSTIN_I_IHFSTI].
					     mask);
		pmu_audio->bcmpmu->write_dev(pmu_audio->bcmpmu,
					     PMU_REG_IHFSTIN_I_IHFSTO,
					     StoredRegValue[2],
					     pmu_audio->bcmpmu->
					     regmap[PMU_REG_IHFSTIN_I_IHFSTO].
					     mask);
		pmu_audio->bcmpmu->write_dev(pmu_audio->bcmpmu,
					     PMU_REG_IHFSTO_O_IHFSTI,
					     StoredRegValue[3],
					     pmu_audio->bcmpmu->
					     regmap[PMU_REG_IHFSTO_O_IHFSTI].
					     mask);
	}
}

/* Backup registers used for ihf selftest */
void bcmpmu_audio_hs_selftest_backup(bool Enable)
{
	static unsigned int StoredRegValue[7];
	if (Enable) {
		/* Store PMU register Values */
		pmu_audio->bcmpmu->read_dev(pmu_audio->bcmpmu,
					    PMU_REG_HSPGA1, &StoredRegValue[0]);
		pmu_audio->bcmpmu->read_dev(pmu_audio->bcmpmu,
					    PMU_REG_HSPGA2, &StoredRegValue[1]);
		pmu_audio->bcmpmu->read_dev(pmu_audio->bcmpmu,
					    PMU_REG_HSPGA3, &StoredRegValue[2]);
		pmu_audio->bcmpmu->read_dev(pmu_audio->bcmpmu,
					    PMU_REG_HSLDO, &StoredRegValue[3]);
		pmu_audio->bcmpmu->read_dev(pmu_audio->bcmpmu,
					    PMU_REG_HSPUP1, &StoredRegValue[4]);
		pmu_audio->bcmpmu->read_dev(pmu_audio->bcmpmu,
					    PMU_REG_HSPUP2, &StoredRegValue[5]);

	} else {
		/* Restore PMU register values */
		pmu_audio->bcmpmu->write_dev(pmu_audio->bcmpmu,
					     PMU_REG_HSPGA1, StoredRegValue[0]);
		PUP PMU_REG_HSPGA2, StoredRegValue[1]);
		pmu_audio->bcmpmu->write_dev(pmu_audio->bcmpmu,
					     PMU_REG_HSPGA3, StoredRegValue[2]);
		pmu_audio->bcmpmu->write_dev(pmu_audio->bcmpmu,
					     PMU_REG_HSLDO, StoredRegValue[3]);

		pmu_audio->bcmpmu->write_dev(pmu_audio->bcmpmu,
					     PMU_REG_HSPUP1, StoredRegValue[5]);
		pmu_audio->bcmpmu->write_dev(pmu_audio->bcmpmu,
					     PMU_REG_HSPUP2, StoredRegValue[6]);
	}
}

#endif
void bcmpmu_audio_init(void)
{
	u8 val;
	struct bcmpmu59xxx *bcmpmu;
	if (!pmu_audio)
		return;
	bcmpmu = pmu_audio->bcmpmu;
	pr_audio(FLOW, "%s: - pll_use_count = %u\n",
			__func__, pmu_audio->pll_use_count);
	mutex_lock(&pmu_audio->lock);
	if (pmu_audio->pll_use_count++ == 0) {

		/* set PLLEN and AUDIO EN */
		bcmpmu->read_dev(bcmpmu, PMU_REG_PLLCTRL, &val);

		val |= (PLLCTRL_PLLEN | PLLCTRL_AUDIO_EN);

		bcmpmu->write_dev(bcmpmu, PMU_REG_PLLCTRL, val);

		/* HS reset enable */
		bcmpmu->read_dev(bcmpmu, PMU_REG_HSPUP1, &val);
		val &= ~HSPUP1_IDDQ_PWRDN_MASK;
		bcmpmu->write_dev(bcmpmu, PMU_REG_HSPUP1, val);

		/* enable class G HS */
		bcmpmu->read_dev(bcmpmu, PMU_REG_HSCP3, &val);
		val |= HSPUP1_CP_CG_SEL;
		bcmpmu->write_dev(bcmpmu, PMU_REG_HSCP3, val);

		/* enable HS offset correction */
		/*
		bcmpmu->read_dev(bcmpmu, PMU_REG_HSIST, &val);
		val &= ~HSIST_OC_DISOCMUX;
		bcmpmu->write_dev(bcmpmu, PMU_REG_HSIST, val);
		*/
		/* IHF reset disable */
		bcmpmu->read_dev(bcmpmu, PMU_REG_IHFTOP, &val);
		val &= ~IHFTOP_IHF_IDDQ_MASK;
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFTOP, val);

		/* Bypass IHF ALC/APS, so that IHF gain can
		 * be controlled manually
		 */
		/* Do not bypass ALC by default.
		 * audio controller will call bcmpmu_enable_alc
		 * according to sysparm configuration
		 */
		/*
		bcmpmu->read_dev(bcmpmu, PMU_REG_IHFALC1, &val);
		val |= IHFALC1_BYP_MASK;
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFALC1, val);
		*/

		/* set IHF pop ramp time */
		bcmpmu->read_dev(bcmpmu, PMU_REG_IHFPOP, &val);
		val &= ~IHFPOP_TIMECTL_MASK;
		val |= (0x2 << IHFPOP_TIMECTL_SHIFT);
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFPOP, val);

		/* Set IHF noise threshold */
		bcmpmu->read_dev(bcmpmu, PMU_REG_IHF_NGTHRESH, &val);
		val &= ~IHF_NGTHRESH_THLD_MASK;
		val |= (0x2 << IHF_NGTHRESH_THLD_SHIFT);
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHF_NGTHRESH, val);

	}
	mutex_unlock(&pmu_audio->lock);
}
EXPORT_SYMBOL(bcmpmu_audio_init);

void bcmpmu_audio_deinit(void)
{
	u8 val;
	struct bcmpmu59xxx *bcmpmu;
	if (!pmu_audio)
		return;
	bcmpmu = pmu_audio->bcmpmu;
	pr_audio(FLOW, "%s: - pll_use_count = %u\n",
			__func__, pmu_audio->pll_use_count);
	mutex_lock(&pmu_audio->lock);
	if (pmu_audio->pll_use_count && --pmu_audio->pll_use_count == 0) {
		/* HS reset enable */
		bcmpmu->read_dev(bcmpmu, PMU_REG_HSPUP1, &val);
		val |= HSPUP1_IDDQ_PWRDN;
		bcmpmu->write_dev(bcmpmu, PMU_REG_HSPUP1, val);

		/* disable HS offset correction */
		/*
		bcmpmu->read_dev(bcmpmu, PMU_REG_HSIST, &val);
		val |= HSIST_OC_DISOCMUX;
		bcmpmu->write_dev(bcmpmu, PMU_REG_HSIST, val);
		*/
		/* IHF Reset Enable */
		bcmpmu->read_dev(bcmpmu, PMU_REG_IHFTOP, &val);
		val |= IHFTOP_IHF_IDDQ;
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFTOP, val);

		/* Enable IHF ALC/APS */
		/* Do not bypass ALC by default.
		 * audio controller will call bcmpmu_enable_alc
		 * according to sysparm configuration
		 */
		/*
		bcmpmu->read_dev(bcmpmu, PMU_REG_IHFALC1, &val);
		val |= IHFALC1_IHFALC_BYP;
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFALC1, val);
		*/

		bcmpmu->read_dev(bcmpmu, PMU_REG_PLLCTRL, &val);
		val &= ~(PLLCTRL_PLLEN | PLLCTRL_AUDIO_EN | PLLCTRL_ENCLK26M);
		bcmpmu->write_dev(bcmpmu, PMU_REG_PLLCTRL, val);
		pmu_audio->HS_On = false;
		pmu_audio->IHF_On = false;
	}
	mutex_unlock(&pmu_audio->lock);
}
EXPORT_SYMBOL(bcmpmu_audio_deinit);

static void bcmpmu_audio_sc_work(struct work_struct *work)
{
	struct bcmpmu_audio *bcmpmu_audio =
		container_of(work, struct bcmpmu_audio, irq_work.work);
	struct bcmpmu59xxx *bcmpmu = bcmpmu_audio->bcmpmu;
	u8 val;
	switch (bcmpmu_audio->event) {
	case PMU_IRQ_AUD_HSAB_SHCKT:
		mutex_lock(&bcmpmu_audio->lock);
		bcmpmu->read_dev(bcmpmu, PMU_REG_IHF_HS_TST, &val);
		val |= HS_SC_EDISABLE;
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHF_HS_TST, val);
		udelay(35);
		val &= ~HS_SC_EDISABLE_MASK;
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHF_HS_TST, val);
		bcmpmu->read_dev(bcmpmu, PMU_REG_HSPUP2, &val);
		val &= ~(1 << HSPUP2_HS_PWRUP_SHIFT);
		bcmpmu->write_dev(bcmpmu, PMU_REG_HSPUP2, val);
		udelay(60);
		val |= (1 << HSPUP2_HS_PWRUP_SHIFT);
		bcmpmu->write_dev(bcmpmu, PMU_REG_HSPUP2, val);
		mutex_unlock(&bcmpmu_audio->lock);
		break;
	case PMU_IRQ_AUD_IHFD_SHCKT:
		mutex_lock(&bcmpmu_audio->lock);
		bcmpmu->read_dev(bcmpmu, PMU_REG_IHF_HS_TST, &val);
		val |= IHF_SC_EDISABLE_MASK;
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHF_HS_TST, val);
		udelay(35);
		val &= ~IHF_SC_EDISABLE_MASK;
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHF_HS_TST, val);
		mutex_unlock(&bcmpmu_audio->lock);
		bcmpmu_ihf_power(false);
		udelay(60);
		bcmpmu_ihf_power(true);
		break;
	default:
		pr_info("%s: Wrong IRQ number\n", __func__);
		break;
	}
}

static void bcmpmu_audio_isr(enum bcmpmu59xxx_irq irq, void *data)
{
	struct bcmpmu_audio *paudio = data;
	pr_audio(INIT, "%s: Interrupt for === %s ===\n", __func__,
			(irq == PMU_IRQ_AUD_HSAB_SHCKT) ? "HS SC" : "IHF SC");

	paudio->event = irq;
	schedule_delayed_work(&paudio->irq_work, 0);
}


#ifdef CONFIG_DEBUG_FS
static int audio_hs_setgain(void *data, u64 val)
{
	bcmpmu_hs_set_gain(PMU_AUDIO_HS_BOTH, val);
	return 0;
}
DEFINE_SIMPLE_ATTRIBUTE(dbg_hs_gain, NULL, audio_hs_setgain, "%llu\n");

static int audio_ihf_setgain(void *data, u64 val)
{
	bcmpmu_ihf_set_gain(val);
	return 0;
}
DEFINE_SIMPLE_ATTRIBUTE(dbg_ihf_gain, NULL, audio_ihf_setgain, "%llu\n");

static int audio_hs_on(void *data, u64 val)
{
	bcmpmu_hs_power((bool) val);
	return 0;
}
DEFINE_SIMPLE_ATTRIBUTE(dbg_hs_on, NULL, audio_hs_on, "%llu\n");

static int audio_ihf_on(void *data, u64 val)
{
	if (val)
		bcmpmu_ihf_power(true);
	else
		bcmpmu_ihf_power(false);

	return 0;
}
DEFINE_SIMPLE_ATTRIBUTE(dbg_ihf_on, NULL, audio_ihf_on, "%llu\n");
#endif

static int bcmpmu_audio_probe(struct platform_device *pdev)
{
	struct bcmpmu59xxx *bcmpmu = dev_get_drvdata(pdev->dev.parent);
	struct bcmpmu_audio *audio_data;
	struct bcmpmu59xxx_audio_pdata *pdata =
		(struct bcmpmu59xxx_audio_pdata *)pdev->dev.platform_data;
#ifdef CONFIG_DEBUG_FS
	struct dentry *audio_dir = NULL, *hs_gain = NULL, *ihf_gain =
		NULL, *hs_on = NULL, *ihf_on = NULL;
#endif

	BUG_ON(!bcmpmu);
	pr_audio(INIT, "%s: called.\n", __func__);
	audio_data = kzalloc(sizeof(struct bcmpmu_audio), GFP_KERNEL);
	if (audio_data == NULL) {
		printk(KERN_ERR "%s: failed to alloc mem.\n", __func__);
		return -ENOMEM;
	}
	mutex_init(&audio_data->lock);
	audio_data->pll_use_count = 0;
	audio_data->bcmpmu = bcmpmu;
	pmu_audio = audio_data;
	pmu_audio->HS_On = false;
	pmu_audio->IHF_On = false;
	pmu_audio->ihf_autoseq_dis = pdata->ihf_autoseq_dis;

	INIT_DELAYED_WORK(&audio_data->irq_work, bcmpmu_audio_sc_work);
	bcmpmu_hs_shortcircuit_dis(true);

	/* register for HS and IHF Shortcircuit INT */
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_AUD_HSAB_SHCKT, bcmpmu_audio_isr,
			pmu_audio);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_AUD_IHFD_SHCKT, bcmpmu_audio_isr,
			pmu_audio);
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_AUD_HSAB_SHCKT);
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_AUD_IHFD_SHCKT);

	bcmpmu_hs_shortcircuit_dis(false);

#ifdef CONFIG_DEBUG_FS
	if (!bcmpmu->dent_bcmpmu)
		goto err;
	audio_dir = debugfs_create_dir("audio", bcmpmu->dent_bcmpmu);
	if (!audio_dir)
		goto err;
	audio_data->debugfs_dir = audio_dir;
	if (!debugfs_create_u32("dbg_mask",
		S_IWUSR | S_IRUSR, audio_dir, &debug_mask))
		goto debugfs_err;

	hs_gain =
		debugfs_create_file("hs_gain", 0644, audio_dir, audio_data,
				&dbg_hs_gain);
	if (!hs_gain)
		goto debugfs_err;

	ihf_gain =
		debugfs_create_file("ihf_gain", 0644, audio_dir, audio_data,
				&dbg_ihf_gain);
	if (!ihf_gain)
		goto debugfs_err;

	hs_on =
		debugfs_create_file("hs_on",
				0644, audio_dir, audio_data, &dbg_hs_on);
	if (!hs_on)
		goto debugfs_err;

	ihf_on =
		debugfs_create_file("ihf_on", 0644, audio_dir, pdata,
					&dbg_ihf_on);
	if (!ihf_on)
		goto debugfs_err;

	return 0;
debugfs_err:
	if (hs_on)
		debugfs_remove(hs_on);
	if (ihf_gain)
		debugfs_remove(ihf_gain);
	if (hs_gain)
		debugfs_remove(hs_gain);
	debugfs_remove(audio_dir);
err:
	pr_err("Failed to setup audio debugfs\n");
#endif
	return 0;
}

static int bcmpmu_audio_remove(struct platform_device *pdev)
{
#ifdef CONFIG_DEBUG_FS
	debugfs_remove(pmu_audio->debugfs_dir);
#endif
	kfree(pmu_audio);
	return 0;
}

static struct platform_driver bcmpmu_audio_driver = {
	.driver = {
		.name = "bcmpmu_audio",
	},
	.probe = bcmpmu_audio_probe,
	.remove = bcmpmu_audio_remove,
};

static int __init bcmpmu_audio_drv_init(void)
{
	return platform_driver_register(&bcmpmu_audio_driver);
}
module_init(bcmpmu_audio_drv_init);

static void __exit bcmpmu_audio_drv_exit(void)
{
	platform_driver_unregister(&bcmpmu_audio_driver);
}
module_exit(bcmpmu_audio_drv_exit);

MODULE_DESCRIPTION("BCM PMIC Audio Driver");
MODULE_LICENSE("GPL");
