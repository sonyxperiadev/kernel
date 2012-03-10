/*******************************************************************************
* Copyright 2011 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license
* *other than the GPL, without Broadcom's express prior written consent.
* *****************************************************************************/

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

#include <linux/mfd/bcmpmu.h>
#include <linux/broadcom/bcmpmu_audio.h>

struct bcmpmu_audio {
	struct bcmpmu *bcmpmu;
	bool IHF_On;
	bool HS_On;
	struct mutex lock;
	void *debugfs_dir;
};
static struct bcmpmu_audio *bcmpmu_audio;

void bcmpmu_hs_power(bool on)
{
	unsigned int val;
	struct bcmpmu *bcmpmu = bcmpmu_audio->bcmpmu;
	printk(KERN_WARNING "%s:  ######### ON = %d\n", __func__, on);
	mutex_lock(&bcmpmu_audio->lock);

	if (on) {
		bcmpmu_audio->HS_On = true;
		bcmpmu->write_dev(bcmpmu, PMU_REG_HSPUP_IDDQ_PWRDWN, false,
				bcmpmu->regmap[PMU_REG_HSPUP_IDDQ_PWRDWN].mask);
		val = 1 << bcmpmu->regmap[PMU_REG_HSPUP_HS_PWRUP].shift;
		bcmpmu->write_dev(bcmpmu, PMU_REG_HSPUP_HS_PWRUP, val,
				bcmpmu->regmap[PMU_REG_HSPUP_HS_PWRUP].mask);
	} else {
		bcmpmu_audio->HS_On = false;
		bcmpmu->write_dev(bcmpmu, PMU_REG_HSPUP_HS_PWRUP, false,
				bcmpmu->regmap[PMU_REG_HSPUP_HS_PWRUP].mask);
		val = 1 << bcmpmu->regmap[PMU_REG_HSPUP_IDDQ_PWRDWN].shift;
		bcmpmu->write_dev(bcmpmu, PMU_REG_HSPUP_IDDQ_PWRDWN, val,
				bcmpmu->regmap[PMU_REG_HSPUP_IDDQ_PWRDWN].mask);
	}
	mutex_unlock(&bcmpmu_audio->lock);
}
EXPORT_SYMBOL(bcmpmu_hs_power);

void bcmpmu_hs_shortcircuit_dis(bool disable)
{
	struct bcmpmu *bcmpmu = bcmpmu_audio->bcmpmu;
	unsigned int val = disable << bcmpmu->regmap[PMU_REG_HSDRV_DISSC].shift;

	mutex_lock(&bcmpmu_audio->lock);
	bcmpmu->write_dev(bcmpmu, PMU_REG_HSDRV_DISSC,
			val, bcmpmu->regmap[PMU_REG_HSDRV_DISSC].mask);
	mutex_unlock(&bcmpmu_audio->lock);

}
EXPORT_SYMBOL(bcmpmu_hs_shortcircuit_dis);

int bcmpmu_hs_set_input_mode(int HSgain, int HSInputmode)
{
	struct bcmpmu *bcmpmu = bcmpmu_audio->bcmpmu;
	int data1, data2, data3;
	int ret = 0;
	int HSwasEn = 0;
	pr_info("Inside %s, HSgain %d, HSInputmode %d\n",
		__func__, HSgain, HSInputmode);

	mutex_lock(&bcmpmu_audio->lock);
	if (bcmpmu_audio->HS_On) {
		bcmpmu_audio->HS_On = false;
		bcmpmu->write_dev(bcmpmu, PMU_REG_HSPUP_HS_PWRUP, false,
				bcmpmu->regmap[PMU_REG_HSPUP_HS_PWRUP].mask);
		HSwasEn = 1;
	}

	bcmpmu->read_dev(bcmpmu, PMU_REG_HSPGA1, &data1,
			bcmpmu->regmap[PMU_REG_HSPGA1].mask);
	bcmpmu->read_dev(bcmpmu, PMU_REG_HSPGA1, &data2,
			bcmpmu->regmap[PMU_REG_HSPGA2].mask);
	bcmpmu->read_dev(bcmpmu, PMU_REG_HSPGA3, &data3,
			bcmpmu->regmap[PMU_REG_HSPGA3].mask);
	if (HSInputmode == PMU_HS_SINGLE_ENDED_AC_COUPLED) {

/*add 6 dB shift if input mode is PMU_HS_SINGLE_ENDED,
threfore the HS gain is the same for PMU_HS_SINGLE_ENDED and PMU_HS_DIFFERENTIAL.*/

#if defined(CONFIG_MFD_BCM59055)
		/*for 59055, i_pga_gainl==1, boost 6dB */
		data1 |= (BCMPMU_HSPGA1_PGA_GAINL | BCMPMU_HSPGA1_PGA_GAINR |
				(HSgain & BCMPMU_PGA_CTL_MASK));
#endif
#if defined(CONFIG_MFD_BCM59039)
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
#if defined(CONFIG_MFD_BCM59039)
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
#if defined(CONFIG_MFD_BCM59039)
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

	ret = bcmpmu->write_dev(bcmpmu, PMU_REG_HSPGA1, data1,
			bcmpmu->regmap[PMU_REG_HSPGA1].mask);
	ret |= bcmpmu->write_dev(bcmpmu, PMU_REG_HSPGA2, data2,
			bcmpmu->regmap[PMU_REG_HSPGA2].mask);
	ret |= bcmpmu->write_dev(bcmpmu, PMU_REG_HSPGA3, data3,
			bcmpmu->regmap[PMU_REG_HSPGA3].mask);

	/* Power Up HS */
	if (HSwasEn) {
		bcmpmu_audio->HS_On = true;
		data1 = 1 << bcmpmu->regmap[PMU_REG_HSPUP_HS_PWRUP].shift;
		bcmpmu->write_dev(bcmpmu, PMU_REG_HSPUP_HS_PWRUP, data1,
				bcmpmu->regmap[PMU_REG_HSPUP_HS_PWRUP].mask);
		HSwasEn = 1;
	}
	mutex_unlock(&bcmpmu_audio->lock);

	return ret;
}
EXPORT_SYMBOL(bcmpmu_hs_set_input_mode);

void bcmpmu_hs_set_gain(bcmpmu_hs_path_t path, bcmpmu_hs_gain_t gain)
{
	struct bcmpmu *bcmpmu = bcmpmu_audio->bcmpmu;
	int data1, data2;
	printk(KERN_WARNING "%s: ######### PATH = %d, GAIN = %d\n",
		__func__, path, gain);

	mutex_lock(&bcmpmu_audio->lock);
	bcmpmu->read_dev(bcmpmu_audio->bcmpmu, PMU_REG_HSPGA1, &data1,
			PMU_BITMASK_ALL);
	bcmpmu->read_dev(bcmpmu_audio->bcmpmu, PMU_REG_HSPGA2, &data2,
			PMU_BITMASK_ALL);

	if (path == PMU_AUDIO_HS_LEFT) {
		data1 &= ~BCMPMU_HS_GAIN_MASK;
		data1 |= (gain & BCMPMU_HS_GAIN_MASK);
		bcmpmu->write_dev(bcmpmu, PMU_REG_HSPGA1, data1,
				bcmpmu->regmap[PMU_REG_HSPGA1].mask);
	} else if (path == PMU_AUDIO_HS_RIGHT) {
		data2 &= ~BCMPMU_HS_GAIN_MASK;
		data2 |= (gain & BCMPMU_HS_GAIN_MASK);
		bcmpmu->write_dev(bcmpmu, PMU_REG_HSPGA2, data2,
				bcmpmu->regmap[PMU_REG_HSPGA2].mask);
	} else {  /*for PMU_AUDIO_HS_BOTH*/
		data1 &= ~BCMPMU_HS_GAIN_MASK;
		data1 |= (gain & BCMPMU_HS_GAIN_MASK);
		data2 &= ~BCMPMU_HS_GAIN_MASK;
		data2 |= (gain & BCMPMU_HS_GAIN_MASK);
		bcmpmu->write_dev(bcmpmu, PMU_REG_HSPGA1, data1,
				bcmpmu->regmap[PMU_REG_HSPGA1].mask);
		bcmpmu->write_dev(bcmpmu, PMU_REG_HSPGA2, data2,
				bcmpmu->regmap[PMU_REG_HSPGA2].mask);
	}
	mutex_unlock(&bcmpmu_audio->lock);
}
EXPORT_SYMBOL(bcmpmu_hs_set_gain);


void bcmpmu_ihf_power(bool on)
{
	struct bcmpmu *bcmpmu = bcmpmu_audio->bcmpmu;
	struct bcmpmu_rw_data reg;
	printk(KERN_WARNING "%s:  ######### ON = %d\n", __func__, on);
	mutex_lock(&bcmpmu_audio->lock);
	if (on) {
		if (bcmpmu_audio->IHF_On) {
			printk(KERN_INFO "%s: IHF is already on.\n", __func__);
			mutex_unlock(&bcmpmu_audio->lock);
			return;
		}
		bcmpmu_audio->IHF_On = true;

		/* Set i_IHF_IDDQ =0, */
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFTOP_IHF_IDDQ, false,
				bcmpmu->regmap[PMU_REG_IHFTOP_IHF_IDDQ].mask);

		/*toggle i_IHFLDO_pup from 0 to 1. */
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFLDO_PUP,
				  0, bcmpmu->regmap[PMU_REG_IHFLDO_PUP].mask);

		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFLDO_PUP,
				  bcmpmu->regmap[PMU_REG_IHFLDO_PUP].mask,
				  bcmpmu->regmap[PMU_REG_IHFLDO_PUP].mask);
	} else {
		if (!bcmpmu_audio->IHF_On) {
			printk(KERN_INFO "%s: IHF is already off.\n", __func__);
			mutex_unlock(&bcmpmu_audio->lock);
			return;
		}
		bcmpmu_audio->IHF_On = false;

		/* Toggle i_IHFpop_pup from 1 to 0. */
		bcmpmu->read_dev(bcmpmu, PMU_REG_IHFPOP_PUP, &reg.val,
				PMU_BITMASK_ALL);
		reg.val |= BCMPMU_IHFPOP_PUP; /* IHFPOP*/
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFPOP_PUP, reg.val,
				PMU_BITMASK_ALL);

		bcmpmu->read_dev(bcmpmu, PMU_REG_IHFPOP_PUP, &reg.val,
				PMU_BITMASK_ALL);
		reg.val &= ~BCMPMU_IHFPOP_PUP; /* IHFPOP*/
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFPOP_PUP, reg.val,
				PMU_BITMASK_ALL);

		/* Set i_IHFLDO_pup=0. */
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFLDO_PUP,
				  0, bcmpmu->regmap[PMU_REG_IHFLDO_PUP].mask);

		/* Set i_IHF_IDDQ =1. */
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFTOP_IHF_IDDQ,
				  bcmpmu->regmap[PMU_REG_IHFTOP_IHF_IDDQ].mask,
				  bcmpmu->regmap[PMU_REG_IHFTOP_IHF_IDDQ].mask);

	}
	mutex_unlock(&bcmpmu_audio->lock);
}
EXPORT_SYMBOL(bcmpmu_ihf_power);

void bcmpmu_ihf_bypass_en(bool enable)
{
	struct bcmpmu *bcmpmu = bcmpmu_audio->bcmpmu;
	int val;
	mutex_lock(&bcmpmu_audio->lock);

	if (enable)
		val = 1 << bcmpmu->regmap[PMU_REG_IHFTOP_BYPASS].shift;
	else
		val = 0;
	bcmpmu->write_dev(bcmpmu, PMU_REG_IHFTOP_BYPASS, val,
			bcmpmu->regmap[PMU_REG_IHFTOP_BYPASS].mask);
	mutex_unlock(&bcmpmu_audio->lock);
}
EXPORT_SYMBOL(bcmpmu_ihf_bypass_en);

void bcmpmu_ihf_set_gain(bcmpmu_ihf_gain_t gain)
{
	struct bcmpmu *bcmpmu = bcmpmu_audio->bcmpmu;
	u8 val;
	printk(KERN_WARNING "%s:  ######### GAIN = %d\n", __func__, gain);
	mutex_lock(&bcmpmu_audio->lock);
	val = (gain & bcmpmu->regmap[PMU_REG_IHFPGA2_GAIN].mask) <<
	    bcmpmu->regmap[PMU_REG_IHFPGA2_GAIN].shift;

	bcmpmu->write_dev(bcmpmu, PMU_REG_IHFPGA2_GAIN,
			  val, bcmpmu->regmap[PMU_REG_IHFPGA2_GAIN].mask);
	mutex_unlock(&bcmpmu_audio->lock);
}
EXPORT_SYMBOL(bcmpmu_ihf_set_gain);

void bcmpmu_hi_gain_mode_en(bool en)
{
	struct bcmpmu *bcmpmu = bcmpmu_audio->bcmpmu;
	u8 val = en << bcmpmu->regmap[PMU_REG_HIGH_GAIN_MODE].shift;

	bcmpmu->write_dev(bcmpmu, PMU_REG_HIGH_GAIN_MODE,
			  val, bcmpmu->regmap[PMU_REG_HIGH_GAIN_MODE].mask);
}
EXPORT_SYMBOL(bcmpmu_hi_gain_mode_en);

int bcmpmu_audio_ihf_selftest_stimulus_input(int stimulus)
{
	stimulus =
	    stimulus << bcmpmu_audio->bcmpmu->regmap[PMU_REG_IHFSTIN_I_IHFSTI].
	    shift;
	return bcmpmu_audio->bcmpmu->write_dev(bcmpmu_audio->bcmpmu,
					       PMU_REG_IHFSTIN_I_IHFSTI,
					       stimulus,
					       bcmpmu_audio->bcmpmu->
					       regmap[PMU_REG_IHFSTIN_I_IHFSTI].
					       mask);
}
EXPORT_SYMBOL(bcmpmu_audio_ihf_selftest_stimulus_input);

int bcmpmu_audio_ihf_selftest_stimulus_output(int stimulus)
{
	stimulus =
	    stimulus << bcmpmu_audio->bcmpmu->regmap[PMU_REG_IHFSTIN_I_IHFSTO].
	    shift;
	return bcmpmu_audio->bcmpmu->write_dev(bcmpmu_audio->bcmpmu,
					       PMU_REG_IHFSTIN_I_IHFSTO,
					       stimulus,
					       bcmpmu_audio->bcmpmu->
					       regmap[PMU_REG_IHFSTIN_I_IHFSTO].
					       mask);
}
EXPORT_SYMBOL(bcmpmu_audio_ihf_selftest_stimulus_output);

void bcmpmu_audio_ihf_selftest_result(u8 *result)
{
	unsigned int val;

	bcmpmu_audio->bcmpmu->read_dev(bcmpmu_audio->bcmpmu,
				       PMU_REG_IHFSTO_O_IHFSTI,
				       &val,
				       bcmpmu_audio->bcmpmu->
				       regmap[PMU_REG_IHFSTO_O_IHFSTI].mask);
	val =
	    val >> bcmpmu_audio->bcmpmu->regmap[PMU_REG_IHFSTO_O_IHFSTI].shift;
	*result = val;

}
EXPORT_SYMBOL(bcmpmu_audio_ihf_selftest_result);

int bcmpmu_audio_ihf_testmode(int Mode)
{
	Mode =
	    Mode << bcmpmu_audio->bcmpmu->
	    regmap[PMU_REG_IHFSTIN_I_IHFSELFTEST_EN].shift;
	return bcmpmu_audio->bcmpmu->write_dev(bcmpmu_audio->bcmpmu,
				PMU_REG_IHFSTIN_I_IHFSELFTEST_EN,
				Mode,
				bcmpmu_audio->bcmpmu->
				regmap
				[PMU_REG_IHFSTIN_I_IHFSELFTEST_EN].
				mask);
}
EXPORT_SYMBOL(bcmpmu_audio_ihf_testmode);

int bcmpmu_audio_hs_selftest_stimulus(int stimulus)
{
	stimulus =
	    stimulus << bcmpmu_audio->bcmpmu->regmap[PMU_REG_HSIST_I_HS_IST].
	    shift;
	return bcmpmu_audio->bcmpmu->write_dev(bcmpmu_audio->bcmpmu,
					       PMU_REG_HSIST_I_HS_IST, stimulus,
					       bcmpmu_audio->bcmpmu->
					       regmap[PMU_REG_HSIST_I_HS_IST].
					       mask);
}
EXPORT_SYMBOL(bcmpmu_audio_hs_selftest_stimulus);

void bcmpmu_audio_hs_selftest_result(u8 *result)
{
	unsigned int val;
	bcmpmu_audio->bcmpmu->read_dev(bcmpmu_audio->bcmpmu,
				       PMU_REG_HSOUT1_O_HS_IST,
				       &val,
				       bcmpmu_audio->bcmpmu->
				       regmap[PMU_REG_HSOUT1_O_HS_IST].mask);
	val =
	    val >> bcmpmu_audio->bcmpmu->regmap[PMU_REG_HSOUT1_O_HS_IST].shift;
	*result = val;
}
EXPORT_SYMBOL(bcmpmu_audio_hs_selftest_result);

int bcmpmu_audio_hs_testmode(int Mode)
{
	Mode =
	    Mode << bcmpmu_audio->bcmpmu->regmap[PMU_REG_HSIST_I_HS_ENST].shift;
	return bcmpmu_audio->bcmpmu->write_dev(bcmpmu_audio->bcmpmu,
					       PMU_REG_HSIST_I_HS_ENST, Mode,
					       bcmpmu_audio->bcmpmu->
					       regmap[PMU_REG_HSIST_I_HS_ENST].
					       mask);
}
EXPORT_SYMBOL(bcmpmu_audio_hs_testmode);

/* Backup registers used for headset selftest */
void bcmpmu_audio_ihf_selftest_backup(bool Enable)
{
	static unsigned int StoredRegValue[4];
	if (Enable) {
		/* Store PMU register Values */
		bcmpmu_audio->bcmpmu->read_dev(bcmpmu_audio->bcmpmu,
					       PMU_REG_IHFSTIN_I_IHFSELFTEST_EN,
					       &StoredRegValue[0],
					       PMU_BITMASK_ALL);
		bcmpmu_audio->bcmpmu->read_dev(bcmpmu_audio->bcmpmu,
					       PMU_REG_IHFSTIN_I_IHFSTI,
					       &StoredRegValue[1],
					       PMU_BITMASK_ALL);
		bcmpmu_audio->bcmpmu->read_dev(bcmpmu_audio->bcmpmu,
					       PMU_REG_IHFSTIN_I_IHFSTO,
					       &StoredRegValue[2],
					       PMU_BITMASK_ALL);
		bcmpmu_audio->bcmpmu->read_dev(bcmpmu_audio->bcmpmu,
					       PMU_REG_IHFSTO_O_IHFSTI,
					       &StoredRegValue[3],
					       PMU_BITMASK_ALL);
	} else {
		/* Restore PMU register values */
		bcmpmu_audio->bcmpmu->write_dev(bcmpmu_audio->bcmpmu,
					PMU_REG_IHFSTIN_I_IHFSELFTEST_EN,
					StoredRegValue[0],
					bcmpmu_audio->bcmpmu->
					regmap
					[PMU_REG_IHFSTIN_I_IHFSELFTEST_EN].
					mask);
		bcmpmu_audio->bcmpmu->write_dev(bcmpmu_audio->bcmpmu,
						PMU_REG_IHFSTIN_I_IHFSTI,
						StoredRegValue[1],
						bcmpmu_audio->bcmpmu->
						regmap
						[PMU_REG_IHFSTIN_I_IHFSTI].
						mask);
		bcmpmu_audio->bcmpmu->write_dev(bcmpmu_audio->bcmpmu,
						PMU_REG_IHFSTIN_I_IHFSTO,
						StoredRegValue[2],
						bcmpmu_audio->bcmpmu->
						regmap
						[PMU_REG_IHFSTIN_I_IHFSTO].
						mask);
		bcmpmu_audio->bcmpmu->write_dev(bcmpmu_audio->bcmpmu,
						PMU_REG_IHFSTO_O_IHFSTI,
						StoredRegValue[3],
						bcmpmu_audio->bcmpmu->
						regmap[PMU_REG_IHFSTO_O_IHFSTI].
						mask);

	}
}

/* Backup registers used for ihf selftest */
void bcmpmu_audio_hs_selftest_backup(bool Enable)
{
	static unsigned int StoredRegValue[8];
	if (Enable) {
		/* Store PMU register Values */
		bcmpmu_audio->bcmpmu->read_dev(bcmpmu_audio->bcmpmu,
					       PMU_REG_HSPGA1,
					       &StoredRegValue[0],
					       PMU_BITMASK_ALL);
		bcmpmu_audio->bcmpmu->read_dev(bcmpmu_audio->bcmpmu,
					       PMU_REG_HSPGA2,
					       &StoredRegValue[1],
					       PMU_BITMASK_ALL);
		bcmpmu_audio->bcmpmu->read_dev(bcmpmu_audio->bcmpmu,
					       PMU_REG_HSPGA3,
					       &StoredRegValue[2],
					       PMU_BITMASK_ALL);
		bcmpmu_audio->bcmpmu->read_dev(bcmpmu_audio->bcmpmu,
					       PMU_REG_HSIST_I_HS_ENST,
					       &StoredRegValue[3],
					       PMU_BITMASK_ALL);
		bcmpmu_audio->bcmpmu->read_dev(bcmpmu_audio->bcmpmu,
					       PMU_REG_HSIST_I_HS_IST,
					       &StoredRegValue[4],
					       PMU_BITMASK_ALL);
		bcmpmu_audio->bcmpmu->read_dev(bcmpmu_audio->bcmpmu,
					       PMU_REG_HSPUP_IDDQ_PWRDWN,
					       &StoredRegValue[5],
					       PMU_BITMASK_ALL);
		bcmpmu_audio->bcmpmu->read_dev(bcmpmu_audio->bcmpmu,
					       PMU_REG_HSPUP_HS_PWRUP,
					       &StoredRegValue[6],
					       PMU_BITMASK_ALL);
		bcmpmu_audio->bcmpmu->read_dev(bcmpmu_audio->bcmpmu,
					       PMU_REG_PLLCTRL,
					       &StoredRegValue[7],
					       PMU_BITMASK_ALL);
	} else {
		/* Restore PMU register values */
		bcmpmu_audio->bcmpmu->write_dev(bcmpmu_audio->bcmpmu,
						PMU_REG_HSPGA1,
						StoredRegValue[0],
						bcmpmu_audio->bcmpmu->
						regmap[PMU_REG_HSPGA1].
						mask);
		bcmpmu_audio->bcmpmu->write_dev(bcmpmu_audio->bcmpmu,
						PMU_REG_HSPGA2,
						StoredRegValue[1],
						bcmpmu_audio->bcmpmu->
						regmap[PMU_REG_HSPGA2].
						mask);
		bcmpmu_audio->bcmpmu->write_dev(bcmpmu_audio->bcmpmu,
						PMU_REG_HSPGA3,
						StoredRegValue[2],
						bcmpmu_audio->bcmpmu->
						regmap[PMU_REG_HSPGA2].
						mask);
		bcmpmu_audio->bcmpmu->write_dev(bcmpmu_audio->bcmpmu,
						PMU_REG_HSIST_I_HS_ENST,
						StoredRegValue[3],
						bcmpmu_audio->bcmpmu->
						regmap[PMU_REG_HSIST_I_HS_ENST].
						mask);
		bcmpmu_audio->bcmpmu->write_dev(bcmpmu_audio->bcmpmu,
						PMU_REG_HSIST_I_HS_IST,
						StoredRegValue[4],
						bcmpmu_audio->bcmpmu->
						regmap[PMU_REG_HSIST_I_HS_IST].
						mask);
		bcmpmu_audio->bcmpmu->write_dev(bcmpmu_audio->bcmpmu,
						PMU_REG_HSPUP_IDDQ_PWRDWN,
						StoredRegValue[5],
						bcmpmu_audio->bcmpmu->
						regmap
						[PMU_REG_HSPUP_IDDQ_PWRDWN].
						mask);
		bcmpmu_audio->bcmpmu->write_dev(bcmpmu_audio->bcmpmu,
						PMU_REG_HSPUP_HS_PWRUP,
						StoredRegValue[6],
						bcmpmu_audio->bcmpmu->
						regmap[PMU_REG_HSPUP_HS_PWRUP].
						mask);
		bcmpmu_audio->bcmpmu->write_dev(bcmpmu_audio->bcmpmu,
						PMU_REG_PLLCTRL,
						StoredRegValue[7],
						bcmpmu_audio->bcmpmu->
						regmap[PMU_REG_PLLCTRL].mask);
	}
}

void bcmpmu_audio_init(void)
{
	struct bcmpmu_rw_data reg;
	printk(KERN_WARNING "%s: ######\n", __func__);

	bcmpmu_audio->bcmpmu->read_dev(bcmpmu_audio->bcmpmu, PMU_REG_PLLCTRL,
				       &reg.val, PMU_BITMASK_ALL);
	reg.val |= (BCMPMU_PLL_EN | BCMPMU_PLL_AUDIO_EN);
	bcmpmu_audio->bcmpmu->write_dev(bcmpmu_audio->bcmpmu, PMU_REG_PLLCTRL,
					reg.val, PMU_BITMASK_ALL);
}
EXPORT_SYMBOL(bcmpmu_audio_init);

void bcmpmu_audio_deinit(void)
{
	struct bcmpmu_rw_data reg;

	bcmpmu_audio->bcmpmu->read_dev(bcmpmu_audio->bcmpmu, PMU_REG_PLLCTRL,
				       &reg.val, PMU_BITMASK_ALL);
	reg.val &= ~(BCMPMU_PLL_EN | BCMPMU_PLL_AUDIO_EN);
	bcmpmu_audio->bcmpmu->write_dev(bcmpmu_audio->bcmpmu, PMU_REG_PLLCTRL,
					reg.val, PMU_BITMASK_ALL);

	bcmpmu_audio->HS_On = false;
	bcmpmu_audio->IHF_On = false;
}
EXPORT_SYMBOL(bcmpmu_audio_deinit);

static void bcmpmu_audio_isr(enum bcmpmu_irq irq, void *data)
{
	struct bcmpmu *bcmpmu = ((struct bcmpmu_audio *)data)->bcmpmu;
	unsigned int mask;
	pr_info("%s: Interrupt for %s\n", __func__,
			(irq == PMU_IRQ_AUD_HSAB_SHCKT) ? "HS SC" : "IHF SC");
	switch (irq) {
	case PMU_IRQ_AUD_HSAB_SHCKT:
		mask = bcmpmu->regmap[PMU_REG_HS_SC_EDISABLE].mask;
		bcmpmu->write_dev(bcmpmu, PMU_REG_HS_SC_EDISABLE, mask, mask);
		udelay(35);
		bcmpmu->write_dev(bcmpmu, PMU_REG_HS_SC_EDISABLE, 0, mask);
		break;
	case PMU_IRQ_AUD_IHFD_SHCKT:
		mask = bcmpmu->regmap[PMU_REG_IHF_SC_EDISABLE].mask;
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHF_SC_EDISABLE, mask, mask);
		udelay(35);
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHF_SC_EDISABLE, 0, mask);
		break;
	default:
		pr_info("%s: Wrong IRQ number\n", __func__);
		break;
	}
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

static int __devinit bcmpmu_audio_probe(struct platform_device *pdev)
{
	struct bcmpmu *bcmpmu = pdev->dev.platform_data;
	struct bcmpmu_audio *pdata;
	struct bcmpmu_rw_data reg;
#ifdef CONFIG_DEBUG_FS
	struct dentry *audio_dir = NULL, *hs_gain = NULL, *ihf_gain =
	    NULL, *hs_on = NULL, *ihf_on = NULL;
#endif

	printk(KERN_INFO "%s: called.\n", __func__);

	pdata = kzalloc(sizeof(struct bcmpmu_audio), GFP_KERNEL);
	if (pdata == NULL) {
		printk(KERN_ERR "%s: failed to alloc mem.\n", __func__);
		return -ENOMEM;
	}
	mutex_init(&pdata->lock);
	pdata->bcmpmu = bcmpmu;
	bcmpmu_audio = pdata;
	bcmpmu_audio->HS_On = false;
	bcmpmu_audio->IHF_On = false;

	/* Enable auto sequence for IHF power up and power down */
	bcmpmu->read_dev(bcmpmu_audio->bcmpmu, PMU_REG_IHFPOP_PUP, &reg.val,
			PMU_BITMASK_ALL);
	reg.val |= BCMPMU_IHFPOP_AUTOSEQ;  /*IHFPOP*/
	bcmpmu->write_dev(bcmpmu_audio->bcmpmu, PMU_REG_IHFPOP_PUP, reg.val,
			PMU_BITMASK_ALL);
	/* Bypass IHF ALC/APS, so that IHF gain can be controlled manually */
	bcmpmu->write_dev(bcmpmu, PMU_REG_IHFALC_BYPASS,
			bcmpmu->regmap[PMU_REG_IHFALC_BYPASS].mask,
			bcmpmu->regmap[PMU_REG_IHFALC_BYPASS].mask);
	/* temporarily disable HS short-circuitry as HS SC interrupt
	 * is getting generated continuously on 59039 variants.
	 * Remove this once fix has been found
	*/
#ifdef CONFIG_MFD_BCM59039
	bcmpmu_hs_shortcircuit_dis(true);
#endif

	/* register for HS and IHF Shortcircuit INT */
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_AUD_HSAB_SHCKT, bcmpmu_audio_isr,
			bcmpmu_audio);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_AUD_IHFD_SHCKT, bcmpmu_audio_isr,
			bcmpmu_audio);
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_AUD_HSAB_SHCKT);
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_AUD_IHFD_SHCKT);
#ifdef CONFIG_DEBUG_FS
	if (bcmpmu->debugfs_root_dir == NULL)
		goto debugfs_err;
	audio_dir = debugfs_create_dir("audio", bcmpmu->debugfs_root_dir);
	if (!audio_dir)
		goto debugfs_err;
	hs_gain =
	    debugfs_create_file("hs_gain", 0644, audio_dir, pdata,
				&dbg_hs_gain);
	if (!hs_gain)
		goto debugfs_err;
	ihf_gain =
	    debugfs_create_file("ihf_gain", 0644, audio_dir, pdata,
				&dbg_ihf_gain);
	if (!ihf_gain)
		goto debugfs_err;
	hs_on =
	    debugfs_create_file("hs_on", 0644, audio_dir, pdata, &dbg_hs_on);
	if (!hs_on)
		goto debugfs_err;
	ihf_on =
	    debugfs_create_file("ihf_on", 0644, audio_dir, pdata, &dbg_ihf_on);
	if (!ihf_on)
		goto debugfs_err;
	return 0;

debugfs_err:
	debugfs_remove(hs_on);
	debugfs_remove(ihf_gain);
	debugfs_remove(hs_gain);
	debugfs_remove(audio_dir);
#endif
	return 0;
}

static int __devexit bcmpmu_audio_remove(struct platform_device *pdev)
{
	kfree(bcmpmu_audio);
	return 0;
}

static struct platform_driver bcmpmu_audio_driver = {
	.driver = {
		   .name = "bcmpmu_audio",
		   },
	.probe = bcmpmu_audio_probe,
	.remove = __devexit_p(bcmpmu_audio_remove),
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
