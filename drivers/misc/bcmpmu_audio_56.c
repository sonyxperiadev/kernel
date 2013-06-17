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
* * other than the GPL, without Broadcom's express prior written consent.
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

#include <linux/mfd/bcmpmu_56.h>
#include <linux/broadcom/bcmpmu_audio_56.h>

struct bcmpmu_audio {
	struct bcmpmu *bcmpmu;
	bool IHF_On;
	bool HS_On;
	struct mutex lock;
	void *debugfs_dir;
	u32 pll_use_count;
	int ihf_autoseq_dis;
};
static struct bcmpmu_audio *bcmpmu_audio;

void bcmpmu_hs_power(bool on)
{
	struct bcmpmu_rw_data reg1,reg2;
	if (!bcmpmu_audio)
		return;
	bcmpmu_audio->bcmpmu->read_dev(bcmpmu_audio->bcmpmu,PMU_REG_HSPUP1_IDDQ_PWRDWN,&reg1.val,PMU_BITMASK_ALL);
	bcmpmu_audio->bcmpmu->read_dev(bcmpmu_audio->bcmpmu,PMU_REG_HSPUP2_HS_PWRUP,&reg2.val,PMU_BITMASK_ALL);

	if (on) {
		reg1.val &= ~BCMPMU_HSPUP1_IDDQ_PWRDN;  /*HSPUP1*/
		reg2.val |= BCMPMU_HSPUP2_HS_PWRUP;     /*HSPUP2*/
		bcmpmu_audio->HS_On = true;
	} else {
		reg1.val |= BCMPMU_HSPUP1_IDDQ_PWRDN;  /*HSPUP1*/
		reg2.val &= ~BCMPMU_HSPUP2_HS_PWRUP;   /*HSPUP2*/
		bcmpmu_audio->HS_On = false;
	}
	bcmpmu_audio->bcmpmu->write_dev(bcmpmu_audio->bcmpmu,PMU_REG_HSPUP1_IDDQ_PWRDWN,reg1.val,PMU_BITMASK_ALL);
	bcmpmu_audio->bcmpmu->write_dev(bcmpmu_audio->bcmpmu,PMU_REG_HSPUP2_HS_PWRUP,reg2.val,PMU_BITMASK_ALL);
}
EXPORT_SYMBOL(bcmpmu_hs_power);



static void bcmpmu_ihf_manual_power(bool on)
{
	struct bcmpmu *bcmpmu;
	pr_debug("%s:  ######### ON = %d\n", __func__, on);
	if (!bcmpmu_audio)
		return;
	bcmpmu = bcmpmu_audio->bcmpmu;

	mutex_lock(&bcmpmu_audio->lock);
	if (on) {
		if (bcmpmu_audio->IHF_On) {
			printk(KERN_INFO "%s: IHF is already on.\n", __func__);
			mutex_unlock(&bcmpmu_audio->lock);
			return;
		}
		bcmpmu_audio->IHF_On = true;
		/* Enable IHFLDOPUP */
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFLDO_PUP,
				  bcmpmu->regmap[PMU_REG_IHFLDO_PUP].mask,
				  bcmpmu->regmap[PMU_REG_IHFLDO_PUP].mask);
		/* Enable  IHF CLK pup */
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFCLK_PUP,
				  bcmpmu->regmap[PMU_REG_IHFCLK_PUP].mask,
				  bcmpmu->regmap[PMU_REG_IHFCLK_PUP].mask);
		/* Enable  IHF BIAS */
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFBIAS_EN,
				  bcmpmu->regmap[PMU_REG_IHFBIAS_EN].mask,
				  bcmpmu->regmap[PMU_REG_IHFBIAS_EN].mask);
		/* wait for 1ms */
		usleep_range(1000, 2000);
		/* Enable  IHF RC CAL pup */
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFRCCAL_PUP,
				  bcmpmu->regmap[PMU_REG_IHFRCCAL_PUP].mask,
				  bcmpmu->regmap[PMU_REG_IHFRCCAL_PUP].mask);
		/* wait for 1.5ms */
		usleep_range(1500, 3000);
		/* Enable  IHF Forward Feed PUP */
		/*bcmpmu->write_dev(bcmpmu, PMU_REG_IHFFF_PUP,
				  bcmpmu->regmap[PMU_REG_IHFFF_PUP].mask,
				  bcmpmu->regmap[PMU_REG_IHFFF_PUP].mask);*/
		/* Enable  IHF RAMP pup */
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFRAMP_PUP,
				  bcmpmu->regmap[PMU_REG_IHFRAMP_PUP].mask,
				  bcmpmu->regmap[PMU_REG_IHFRAMP_PUP].mask);
		/* Enable  IHF LOOP FILTER pup */
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFLF_PUP,
				  bcmpmu->regmap[PMU_REG_IHFLF_PUP].mask,
				  bcmpmu->regmap[PMU_REG_IHFLF_PUP].mask);
		/* Enable  IHF Comparator and Phase Detector */
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFCMPPD_PUP,
				  bcmpmu->regmap[PMU_REG_IHFCMPPD_PUP].mask,
				  bcmpmu->regmap[PMU_REG_IHFCMPPD_PUP].mask);
		/* Enable  IHF Feedback Amp pup */
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFFB_PUP,
				  bcmpmu->regmap[PMU_REG_IHFFB_PUP].mask,
				  bcmpmu->regmap[PMU_REG_IHFFB_PUP].mask);
		/* Mute IHF gain */
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFPGA2_GAIN, 0,
				  bcmpmu->regmap[PMU_REG_IHFPGA2_GAIN].mask);
		/* Enable  IHF Power Driver pup */
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFPWRDRV_PUP,
				  bcmpmu->regmap[PMU_REG_IHFPWRDRV_PUP].mask,
				  bcmpmu->regmap[PMU_REG_IHFPWRDRV_PUP].mask);
		/* Enable  IHF Noise Gate pup */
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFNG_PUP,
				  bcmpmu->regmap[PMU_REG_IHFNG_PUP].mask,
				  bcmpmu->regmap[PMU_REG_IHFNG_PUP].mask);
		/* Enable  IHF POP */
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFPOP_EN,
				  bcmpmu->regmap[PMU_REG_IHFPOP_EN].mask,
				  bcmpmu->regmap[PMU_REG_IHFPOP_EN].mask);
		/* wait for 0.6ms */
		usleep_range(600, 1200);
		/* Enable  IHF POP pup */
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFPOP_PUP,
				  bcmpmu->regmap[PMU_REG_IHFPOP_PUP].mask,
				  bcmpmu->regmap[PMU_REG_IHFPOP_PUP].mask);
		/* wait for 2ms */
		/*usleep_range(2000, 4000);*/
		/* Disable  IHF Driver Clamp */
		/*bcmpmu->write_dev(bcmpmu, PMU_REG_IHFDRVCLAMP_DIS,
				  bcmpmu->regmap[PMU_REG_IHFDRVCLAMP_DIS].mask,
				bcmpmu->regmap[PMU_REG_IHFDRVCLAMP_DIS].mask);*/
		/* Driver controlled by calibration logic */
		/*bcmpmu->write_dev(bcmpmu, PMU_REG_IHFCAL_SEL,
				  bcmpmu->regmap[PMU_REG_IHFCAL_SEL].mask,
				  bcmpmu->regmap[PMU_REG_IHFCAL_SEL].mask);*/
		/* wait for 0.6ms */
		usleep_range(600, 1200);
		/* Enable  IHF PSR CAL pup */
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFPSRCAL_PUP,
				  bcmpmu->regmap[PMU_REG_IHFPSRCAL_PUP].mask,
				  bcmpmu->regmap[PMU_REG_IHFPSRCAL_PUP].mask);
		/* wait for 20ms */
		msleep(20);
		/* Disable  IHF POP pup */
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFPOP_PUP,
				  false,
				  bcmpmu->regmap[PMU_REG_IHFPOP_PUP].mask);
		/* wait for 0.6ms */
		/*usleep_range(600, 1200);*/
		/* PSR cal disable */
		/*bcmpmu->write_dev(bcmpmu, PMU_REG_IHFCAL_SEL,
				  false,
				  bcmpmu->regmap[PMU_REG_IHFCAL_SEL].mask);*/
		/* Enable  IHF Driver Clamp */
		/*bcmpmu->write_dev(bcmpmu, PMU_REG_IHFDRVCLAMP_DIS,
				  false,
				bcmpmu->regmap[PMU_REG_IHFDRVCLAMP_DIS].mask);*/
		/* wait for 0.6ms */
		usleep_range(600, 1200);
		/* Enable  IHF POP pup */
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFPOP_PUP,
				  bcmpmu->regmap[PMU_REG_IHFPOP_PUP].mask,
				  bcmpmu->regmap[PMU_REG_IHFPOP_PUP].mask);
	} else {
		if (!bcmpmu_audio->IHF_On) {
			printk(KERN_INFO "%s: IHF is already off.\n", __func__);
			mutex_unlock(&bcmpmu_audio->lock);
			return;
		}
		bcmpmu_audio->IHF_On = false;
		/* Disable  IHF POP pup */
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFPOP_PUP,
				  false,
				  bcmpmu->regmap[PMU_REG_IHFPOP_PUP].mask);
		/* wait for 1.5ms */
		usleep_range(1500, 3000);
		/* Disable  IHF RC CAL pup */
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFRCCAL_PUP, false,
				  bcmpmu->regmap[PMU_REG_IHFRCCAL_PUP].mask);
		/* Disable  IHF CLK pup */
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFCLK_PUP, false,
				  bcmpmu->regmap[PMU_REG_IHFCLK_PUP].mask);
		/* Disable  IHF BIAS */
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFBIAS_EN, false,
				  bcmpmu->regmap[PMU_REG_IHFBIAS_EN].mask);
		/* Disable  IHF Forward Feed PUP */
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFFF_PUP, false,
				  bcmpmu->regmap[PMU_REG_IHFFF_PUP].mask);
		/* Disable  IHF RAMP pup */
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFRAMP_PUP, false,
				  bcmpmu->regmap[PMU_REG_IHFRAMP_PUP].mask);
		/* Disable  IHF LOOP FILTER pup */
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFLF_PUP, false,
				  bcmpmu->regmap[PMU_REG_IHFLF_PUP].mask);
		/* Disable  IHF Comparator and Phase Detector */
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFCMPPD_PUP, false,
				  bcmpmu->regmap[PMU_REG_IHFCMPPD_PUP].mask);
		/* Disable  IHF Feedback Amp pup */
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFFB_PUP, false,
				  bcmpmu->regmap[PMU_REG_IHFFB_PUP].mask);
		/* Disable  IHF Power Driver pup */
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFPWRDRV_PUP, false,
				  bcmpmu->regmap[PMU_REG_IHFPWRDRV_PUP].mask);
		/* Disable  IHF Noise Gate pup */
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFNG_PUP, false,
				  bcmpmu->regmap[PMU_REG_IHFNG_PUP].mask);
		/* wait for 0.5ms */
		usleep_range(500, 1000);
		/* Disable  IHF POP */
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFPOP_EN, false,
				  bcmpmu->regmap[PMU_REG_IHFPOP_EN].mask);
		/* Disable IHFLDOPUP */
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFLDO_PUP, false,
				  bcmpmu->regmap[PMU_REG_IHFLDO_PUP].mask);
	}
	mutex_unlock(&bcmpmu_audio->lock);
}

/* callee of this API need to put 65ms delay to
 * make sure power up seq done properly by h/w if ihf_autoseq_dis is not set
*/
void bcmpmu_ihf_power(bool on)
{
	struct bcmpmu *bcmpmu;
	struct bcmpmu_rw_data reg;
	if (!bcmpmu_audio)
		return;
	bcmpmu = bcmpmu_audio->bcmpmu;
	pr_debug("%s:  ######### ON = %d\n", __func__, on);
	if (bcmpmu_audio->ihf_autoseq_dis) {
		bcmpmu_ihf_manual_power(on);
		return;
	}
	mutex_lock(&bcmpmu_audio->lock);
	if (on) {
		if (bcmpmu_audio->IHF_On) {
			printk(KERN_INFO "%s: IHF is already on.\n", __func__);
			mutex_unlock(&bcmpmu_audio->lock);
			return;
		}
		bcmpmu_audio->IHF_On = true;

		/* Enable auto sequence for IHF power up and power down */
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFAUTO_SEQ,
				  bcmpmu->regmap[PMU_REG_IHFAUTO_SEQ].mask,
				  bcmpmu->regmap[PMU_REG_IHFAUTO_SEQ].mask);

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

		/* Bypass IHF ALC/APS, so that IHF gain can be
		 * controlled manually
		*/
		/*bcmpmu->write_dev(bcmpmu, PMU_REG_IHFALC_BYPASS,
			bcmpmu->regmap[PMU_REG_IHFALC_BYPASS].mask,
			bcmpmu->regmap[PMU_REG_IHFALC_BYPASS].mask);*/

		/* Toggle i_IHFpop_pup from 1 to 0. */
		bcmpmu->read_dev(bcmpmu, PMU_REG_IHFPOP_PUP, &reg.val,
				PMU_BITMASK_ALL);
		reg.val |= (BCMPMU_IHFPOP_PUP | BCPMU_IHFPOP_BYPASS);
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
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFAUTO_SEQ, false,
				  bcmpmu->regmap[PMU_REG_IHFAUTO_SEQ].mask);

	}
	mutex_unlock(&bcmpmu_audio->lock);
}
EXPORT_SYMBOL(bcmpmu_ihf_power);


void bcmpmu_hs_set_gain(bcmpmu_hs_path_t path, bcmpmu_hs_gain_t gain)
{
	struct bcmpmu_rw_data reg1,reg2;
	if (!bcmpmu_audio)
		return;

	bcmpmu_audio->bcmpmu->read_dev(bcmpmu_audio->bcmpmu,PMU_REG_HSPGA1_GAIN,&reg1.val,PMU_BITMASK_ALL);
	bcmpmu_audio->bcmpmu->read_dev(bcmpmu_audio->bcmpmu,PMU_REG_HSPGA2_GAIN,&reg2.val,PMU_BITMASK_ALL);

	if(path == PMU_AUDIO_HS_LEFT) {
	reg1.val &= ~BCMPMU_HS_GAIN_MASK;
	reg1.val |= (gain & BCMPMU_HS_GAIN_MASK);
		bcmpmu_audio->bcmpmu->write_dev(bcmpmu_audio->bcmpmu,PMU_REG_HSPGA1_GAIN,reg1.val,PMU_BITMASK_ALL);
		return;
	}
	else if(path == PMU_AUDIO_HS_RIGHT){
		reg2.val &= ~BCMPMU_HS_GAIN_MASK;
		reg2.val |= (gain & BCMPMU_HS_GAIN_MASK);
		bcmpmu_audio->bcmpmu->write_dev(bcmpmu_audio->bcmpmu,PMU_REG_HSPGA2_GAIN,reg2.val,PMU_BITMASK_ALL);
		return;
	}
	else{  /*for PMU_AUDIO_HS_BOTH*/
		reg1.val &= ~BCMPMU_HS_GAIN_MASK;
		reg1.val |= (gain & BCMPMU_HS_GAIN_MASK);
		reg2.val &= ~BCMPMU_HS_GAIN_MASK;
		reg2.val |= (gain & BCMPMU_HS_GAIN_MASK);
		bcmpmu_audio->bcmpmu->write_dev(bcmpmu_audio->bcmpmu,PMU_REG_HSPGA1_GAIN,reg1.val,PMU_BITMASK_ALL);
		bcmpmu_audio->bcmpmu->write_dev(bcmpmu_audio->bcmpmu,PMU_REG_HSPGA2_GAIN,reg2.val,PMU_BITMASK_ALL);
	}
}
EXPORT_SYMBOL(bcmpmu_hs_set_gain);

void bcmpmu_ihf_set_gain(bcmpmu_ihf_gain_t gain)
{
	struct bcmpmu *bcmpmu;
	u8 val;
	if (!bcmpmu_audio)
		return;
	bcmpmu = bcmpmu_audio->bcmpmu;
	val = (gain & bcmpmu->regmap[PMU_REG_IHFPGA2_GAIN].mask) <<
		bcmpmu->regmap[PMU_REG_IHFPGA2_GAIN].shift;

	bcmpmu->write_dev(bcmpmu, PMU_REG_IHFPGA2_GAIN,
			val, bcmpmu->regmap[PMU_REG_IHFPGA2_GAIN].mask);
}
EXPORT_SYMBOL(bcmpmu_ihf_set_gain);

void bcmpmu_hi_gain_mode_en(bool en)
{
	struct bcmpmu *bcmpmu;
	u8 val;
	if (!bcmpmu_audio)
		return;
	bcmpmu = bcmpmu_audio->bcmpmu;
	val = en << bcmpmu->regmap[PMU_REG_HIGH_GAIN_MODE].shift;

	bcmpmu->write_dev(bcmpmu, PMU_REG_HIGH_GAIN_MODE,
			val, bcmpmu->regmap[PMU_REG_HIGH_GAIN_MODE].mask);
}
EXPORT_SYMBOL(bcmpmu_hi_gain_mode_en);

void bcmpmu_hi_gain_mode_hs_en(bcmpmu_hs_path_t path, bool en)
{
	struct bcmpmu *bcmpmu;
	unsigned int mask;
	int reg;
	u8 val;
	if (!bcmpmu_audio)
		return;
	bcmpmu = bcmpmu_audio->bcmpmu;

	if (path == PMU_AUDIO_HS_LEFT) {
		val = en << bcmpmu->regmap[PMU_REG_HIGH_GAIN_MODE_HSL].shift;
		reg = PMU_REG_HIGH_GAIN_MODE_HSL;
		mask = bcmpmu->regmap[PMU_REG_HIGH_GAIN_MODE_HSL].mask;
	} else if (path == PMU_AUDIO_HS_RIGHT) {
		val = en << bcmpmu->regmap[PMU_REG_HIGH_GAIN_MODE_HSR].shift;
		reg = PMU_REG_HIGH_GAIN_MODE_HSR;
		mask = bcmpmu->regmap[PMU_REG_HIGH_GAIN_MODE_HSR].mask;
	} else {
		val = en << bcmpmu->regmap[PMU_REG_HIGH_GAIN_MODE_HSL].shift;
		val |= en << bcmpmu->regmap[PMU_REG_HIGH_GAIN_MODE_HSR].shift;
		reg = PMU_REG_HIGH_GAIN_MODE_HSL;
		mask = bcmpmu->regmap[PMU_REG_HIGH_GAIN_MODE_HSL].mask;
		mask |= bcmpmu->regmap[PMU_REG_HIGH_GAIN_MODE_HSR].mask;
	}
	bcmpmu->write_dev(bcmpmu, reg, val, mask);

}
EXPORT_SYMBOL(bcmpmu_hi_gain_mode_hs_en);

int bcmpmu_hs_set_input_mode(int HSgain, int HSInputmode)
{
	int data1, data2, data3;
	int ret = 0;
	int HSwasEn = 0;
	if (!bcmpmu_audio)
		return -ENODEV;
	pr_debug("Inside %s, HSgain %d, HSInputmode %d\n", __func__, HSgain, HSInputmode);

	if (bcmpmu_audio->HS_On) {
		bcmpmu_audio->bcmpmu->read_dev(bcmpmu_audio->bcmpmu,
					       PMU_REG_HSPUP2_HS_PWRUP,
					       &data1,
					       PMU_BITMASK_ALL);
		data1 &= ~BCMPMU_HSPUP2_HS_PWRUP;   /*HSPUP2*/
		bcmpmu_audio->bcmpmu->write_dev(bcmpmu_audio->bcmpmu,
						PMU_REG_HSPUP2_HS_PWRUP,
						data1,
						PMU_BITMASK_ALL);
		bcmpmu_audio->HS_On = false;
		HSwasEn = 1;
	}

	bcmpmu_audio->bcmpmu->read_dev(bcmpmu_audio->bcmpmu,
				       PMU_REG_HSPGA1_GAIN,
				       &data1,
				       PMU_BITMASK_ALL);
	bcmpmu_audio->bcmpmu->read_dev(bcmpmu_audio->bcmpmu,
				       PMU_REG_HSPGA2_GAIN,
				       &data2,
				       PMU_BITMASK_ALL);
	bcmpmu_audio->bcmpmu->read_dev(bcmpmu_audio->bcmpmu,
				       PMU_REG_HSPGA3_GAIN,
				       &data3,
				       PMU_BITMASK_ALL);

	if (HSInputmode == PMU_HS_SINGLE_ENDED_AC_COUPLED) {
		data1 &= ~BCMPMU_PGA_CTL_MASK;
		data1 |= (BCMPMU_HSPGA1_PGA_GAINL | BCMPMU_HSPGA1_PGA_GAINR |
			(HSgain & BCMPMU_PGA_CTL_MASK));

		data2 &= ~BCMPMU_PGA_CTL_MASK;
		data2 |= HSgain & BCMPMU_PGA_CTL_MASK;

		data3 &= ~(BCMPMU_HSPGA3_PGA_ACINADJ | BCMPMU_HSPGA3_PGA_PULLDNSJ);
		data3 |= BCMPMU_HSPGA3_PGA_ENACCPL;
	} else if (HSInputmode == PMU_HS_DIFFERENTIAL_AC_COUPLED) {
		data1 &= ~(BCMPMU_HSPGA1_PGA_GAINL | BCMPMU_HSPGA1_PGA_GAINR |
			BCMPMU_PGA_CTL_MASK);
		data1 |= HSgain & BCMPMU_PGA_CTL_MASK;

		data2 &= ~BCMPMU_PGA_CTL_MASK;
		data2 |= HSgain & BCMPMU_PGA_CTL_MASK;

		data3 &= ~BCMPMU_HSPGA3_PGA_PULLDNSJ;
		data3 |= (BCMPMU_HSPGA3_PGA_ENACCPL | BCMPMU_HSPGA3_PGA_ACINADJ);
	} else if (HSInputmode == PMU_HS_DIFFERENTIAL_DC_COUPLED) {
		data1 &= ~(BCMPMU_HSPGA1_PGA_GAINL | BCMPMU_HSPGA1_PGA_GAINR |
			BCMPMU_PGA_CTL_MASK);
		data1 |= HSgain & BCMPMU_PGA_CTL_MASK;

		data2 &= ~BCMPMU_PGA_CTL_MASK;
		data2 |= HSgain & BCMPMU_PGA_CTL_MASK;

		data3 &= ~(BCMPMU_HSPGA3_PGA_PULLDNSJ | BCMPMU_HSPGA3_PGA_ENACCPL |
			BCMPMU_HSPGA3_PGA_ACINADJ);
	}

	ret = bcmpmu_audio->bcmpmu->write_dev(bcmpmu_audio->bcmpmu,
					      PMU_REG_HSPGA1_GAIN,
					      data1,
					      PMU_BITMASK_ALL);
	ret |= bcmpmu_audio->bcmpmu->write_dev(bcmpmu_audio->bcmpmu,
					       PMU_REG_HSPGA2_GAIN,
					       data2,
					       PMU_BITMASK_ALL);
	ret |= bcmpmu_audio->bcmpmu->write_dev(bcmpmu_audio->bcmpmu,
					       PMU_REG_HSPGA3_GAIN,
					       data3,
					       PMU_BITMASK_ALL);



	/* Power Up HS */
	if (HSwasEn) {
		bcmpmu_audio->bcmpmu->read_dev(bcmpmu_audio->bcmpmu,
					       PMU_REG_HSPUP2_HS_PWRUP,
					       &data1,
					       PMU_BITMASK_ALL);
		data1 |= BCMPMU_HSPUP2_HS_PWRUP;   /*HSPUP2*/
		bcmpmu_audio->bcmpmu->write_dev(bcmpmu_audio->bcmpmu,
						PMU_REG_HSPUP2_HS_PWRUP,
						data1,
						PMU_BITMASK_ALL);
		HSwasEn = 0;
		bcmpmu_audio->HS_On = true;
	}

	return ret;
}
EXPORT_SYMBOL(bcmpmu_hs_set_input_mode);

int bcmpmu_audio_ihf_selftest_stimulus_input(int stimulus)
{
	if (!bcmpmu_audio)
		return -ENODEV;
	stimulus = stimulus<<bcmpmu_audio->bcmpmu->regmap[PMU_REG_IHFSTIN_I_IHFSTI].shift;
	return bcmpmu_audio->bcmpmu->write_dev(bcmpmu_audio->bcmpmu,
					       PMU_REG_IHFSTIN_I_IHFSTI,
					       stimulus,
					       bcmpmu_audio->bcmpmu->regmap[PMU_REG_IHFSTIN_I_IHFSTI].mask);
}
EXPORT_SYMBOL(bcmpmu_audio_ihf_selftest_stimulus_input);

int bcmpmu_audio_ihf_selftest_stimulus_output(int stimulus)
{
	if (!bcmpmu_audio)
		return -ENODEV;
	stimulus = stimulus<<bcmpmu_audio->bcmpmu->regmap[PMU_REG_IHFSTIN_I_IHFSTO].shift;
	return bcmpmu_audio->bcmpmu->write_dev(bcmpmu_audio->bcmpmu,
					       PMU_REG_IHFSTIN_I_IHFSTO,
					       stimulus,
					       bcmpmu_audio->bcmpmu->regmap[PMU_REG_IHFSTIN_I_IHFSTO].mask);
}
EXPORT_SYMBOL(bcmpmu_audio_ihf_selftest_stimulus_output);

void bcmpmu_audio_ihf_selftest_result(u8 *result)
{
	unsigned int val;
	if (!bcmpmu_audio)
		return;

	bcmpmu_audio->bcmpmu->read_dev(bcmpmu_audio->bcmpmu,
				       PMU_REG_IHFSTO_O_IHFSTI,
				       &val,
				       bcmpmu_audio->bcmpmu->regmap[PMU_REG_IHFSTO_O_IHFSTI].mask);
        val = val>>bcmpmu_audio->bcmpmu->regmap[PMU_REG_IHFSTO_O_IHFSTI].shift;
	*result = val;

}
EXPORT_SYMBOL(bcmpmu_audio_ihf_selftest_result);

int bcmpmu_audio_ihf_testmode(int Mode)
{
	if (!bcmpmu_audio)
		return -ENODEV;
	Mode = Mode<<bcmpmu_audio->bcmpmu->regmap[PMU_REG_IHFSTIN_I_IHFSELFTEST_EN].shift;
	return bcmpmu_audio->bcmpmu->write_dev(bcmpmu_audio->bcmpmu,
					       PMU_REG_IHFSTIN_I_IHFSELFTEST_EN,
					       Mode,
					       bcmpmu_audio->bcmpmu->regmap[PMU_REG_IHFSTIN_I_IHFSELFTEST_EN].mask);
}
EXPORT_SYMBOL(bcmpmu_audio_ihf_testmode);

int bcmpmu_audio_hs_selftest_stimulus(int stimulus)
{
	if (!bcmpmu_audio)
		return -ENODEV;
	stimulus = stimulus<<bcmpmu_audio->bcmpmu->regmap[PMU_REG_HSIST_I_HS_IST].shift;
	return bcmpmu_audio->bcmpmu->write_dev(bcmpmu_audio->bcmpmu,
					       PMU_REG_HSIST_I_HS_IST,
					       stimulus,
					       bcmpmu_audio->bcmpmu->regmap[PMU_REG_HSIST_I_HS_IST].mask);
}
EXPORT_SYMBOL(bcmpmu_audio_hs_selftest_stimulus);

void bcmpmu_audio_hs_selftest_result(u8 *result)
{
	unsigned int val;
	if (!bcmpmu_audio)
		return;
	bcmpmu_audio->bcmpmu->read_dev(bcmpmu_audio->bcmpmu,
				       PMU_REG_HSOUT1_O_HS_IST,
				       &val,
				       bcmpmu_audio->bcmpmu->regmap[PMU_REG_HSOUT1_O_HS_IST].mask);
	val = val>>bcmpmu_audio->bcmpmu->regmap[PMU_REG_HSOUT1_O_HS_IST].shift;
	*result = val;
}
EXPORT_SYMBOL(bcmpmu_audio_hs_selftest_result);

int bcmpmu_audio_hs_testmode(int Mode)
{
	if (!bcmpmu_audio)
		return -ENODEV;
	Mode = Mode<<bcmpmu_audio->bcmpmu->regmap[PMU_REG_HSIST_I_HS_ENST].shift;
	return bcmpmu_audio->bcmpmu->write_dev(bcmpmu_audio->bcmpmu,
					       PMU_REG_HSIST_I_HS_ENST,
					       Mode,
					       bcmpmu_audio->bcmpmu->regmap[PMU_REG_HSIST_I_HS_ENST].mask);
}
EXPORT_SYMBOL(bcmpmu_audio_hs_testmode);

/* Backup registers used for headset selftest */
void bcmpmu_audio_hs_selftest_backup(bool Enable)
{
	static unsigned int StoredRegValue[4];
	if (!bcmpmu_audio)
		return;
	if(Enable) {
		/* Store PMU register Values */
		bcmpmu_audio->bcmpmu->read_dev(bcmpmu_audio->bcmpmu, PMU_REG_IHFSTIN_I_IHFSELFTEST_EN, &StoredRegValue[0], PMU_BITMASK_ALL );
		bcmpmu_audio->bcmpmu->read_dev(bcmpmu_audio->bcmpmu, PMU_REG_IHFSTIN_I_IHFSTI, &StoredRegValue[1], PMU_BITMASK_ALL );
		bcmpmu_audio->bcmpmu->read_dev(bcmpmu_audio->bcmpmu, PMU_REG_IHFSTIN_I_IHFSTO, &StoredRegValue[2], PMU_BITMASK_ALL );
		bcmpmu_audio->bcmpmu->read_dev(bcmpmu_audio->bcmpmu, PMU_REG_IHFSTO_O_IHFSTI, &StoredRegValue[3], PMU_BITMASK_ALL );
	}else{
		/* Restore PMU register values */
		bcmpmu_audio->bcmpmu->write_dev(bcmpmu_audio->bcmpmu, PMU_REG_IHFSTIN_I_IHFSELFTEST_EN, StoredRegValue[0], bcmpmu_audio->bcmpmu->regmap[PMU_REG_IHFSTIN_I_IHFSELFTEST_EN].mask );
		bcmpmu_audio->bcmpmu->write_dev(bcmpmu_audio->bcmpmu, PMU_REG_IHFSTIN_I_IHFSTI, StoredRegValue[1], bcmpmu_audio->bcmpmu->regmap[PMU_REG_IHFSTIN_I_IHFSTI].mask );
		bcmpmu_audio->bcmpmu->write_dev(bcmpmu_audio->bcmpmu, PMU_REG_IHFSTIN_I_IHFSTO, StoredRegValue[2], bcmpmu_audio->bcmpmu->regmap[PMU_REG_IHFSTIN_I_IHFSTO].mask );
		bcmpmu_audio->bcmpmu->write_dev(bcmpmu_audio->bcmpmu, PMU_REG_IHFSTO_O_IHFSTI, StoredRegValue[3], bcmpmu_audio->bcmpmu->regmap[PMU_REG_IHFSTO_O_IHFSTI].mask );

	}
}
/* Backup registers used for ihf selftest */
void bcmpmu_audio_ihf_selftest_backup(bool Enable)
{
	static unsigned int  StoredRegValue[8];
	if (!bcmpmu_audio)
		return;
	if(Enable) {
		/* Store PMU register Values */
		bcmpmu_audio->bcmpmu->read_dev(bcmpmu_audio->bcmpmu, PMU_REG_HSPGA1_GAIN, &StoredRegValue[0], PMU_BITMASK_ALL );
		bcmpmu_audio->bcmpmu->read_dev(bcmpmu_audio->bcmpmu, PMU_REG_HSPGA2_GAIN, &StoredRegValue[1], PMU_BITMASK_ALL );
		bcmpmu_audio->bcmpmu->read_dev(bcmpmu_audio->bcmpmu, PMU_REG_HSPGA2_GAIN, &StoredRegValue[2], PMU_BITMASK_ALL );
		bcmpmu_audio->bcmpmu->read_dev(bcmpmu_audio->bcmpmu, PMU_REG_HSIST_I_HS_ENST, &StoredRegValue[3], PMU_BITMASK_ALL );
		bcmpmu_audio->bcmpmu->read_dev(bcmpmu_audio->bcmpmu, PMU_REG_HSIST_I_HS_IST, &StoredRegValue[4], PMU_BITMASK_ALL );
		bcmpmu_audio->bcmpmu->read_dev(bcmpmu_audio->bcmpmu, PMU_REG_HSPUP1_IDDQ_PWRDWN, &StoredRegValue[5], PMU_BITMASK_ALL );
		bcmpmu_audio->bcmpmu->read_dev(bcmpmu_audio->bcmpmu, PMU_REG_HSPUP2_HS_PWRUP, &StoredRegValue[6], PMU_BITMASK_ALL );
		bcmpmu_audio->bcmpmu->read_dev(bcmpmu_audio->bcmpmu, PMU_REG_PLLCTRL, &StoredRegValue[7], PMU_BITMASK_ALL );
	}else{
		/* Restore PMU register values */
		bcmpmu_audio->bcmpmu->write_dev(bcmpmu_audio->bcmpmu, PMU_REG_HSPGA1_GAIN, StoredRegValue[0], bcmpmu_audio->bcmpmu->regmap[PMU_REG_HSPGA1_GAIN].mask );
		bcmpmu_audio->bcmpmu->write_dev(bcmpmu_audio->bcmpmu, PMU_REG_HSPGA2_GAIN, StoredRegValue[1], bcmpmu_audio->bcmpmu->regmap[PMU_REG_HSPGA2_GAIN].mask );
		bcmpmu_audio->bcmpmu->write_dev(bcmpmu_audio->bcmpmu, PMU_REG_HSPGA2_GAIN, StoredRegValue[2], bcmpmu_audio->bcmpmu->regmap[PMU_REG_HSPGA2_GAIN].mask );
		bcmpmu_audio->bcmpmu->write_dev(bcmpmu_audio->bcmpmu, PMU_REG_HSIST_I_HS_ENST, StoredRegValue[3], bcmpmu_audio->bcmpmu->regmap[PMU_REG_HSIST_I_HS_ENST].mask );
		bcmpmu_audio->bcmpmu->write_dev(bcmpmu_audio->bcmpmu, PMU_REG_HSIST_I_HS_IST, StoredRegValue[4], bcmpmu_audio->bcmpmu->regmap[PMU_REG_HSIST_I_HS_IST].mask );
		bcmpmu_audio->bcmpmu->write_dev(bcmpmu_audio->bcmpmu, PMU_REG_HSPUP1_IDDQ_PWRDWN, StoredRegValue[5], bcmpmu_audio->bcmpmu->regmap[PMU_REG_HSPUP1_IDDQ_PWRDWN].mask );
		bcmpmu_audio->bcmpmu->write_dev(bcmpmu_audio->bcmpmu, PMU_REG_HSPUP2_HS_PWRUP, StoredRegValue[6], bcmpmu_audio->bcmpmu->regmap[PMU_REG_HSPUP2_HS_PWRUP].mask );
		bcmpmu_audio->bcmpmu->write_dev(bcmpmu_audio->bcmpmu, PMU_REG_PLLCTRL, StoredRegValue[7], bcmpmu_audio->bcmpmu->regmap[PMU_REG_PLLCTRL].mask );
	}
}

void bcmpmu_audio_init(void)
{
	struct bcmpmu_rw_data reg;
	struct bcmpmu *bcmpmu;
	if (!bcmpmu_audio)
		return;
	bcmpmu = bcmpmu_audio->bcmpmu;
	pr_debug("%s: ###### - pll_use_count = %u\n",
		__func__, bcmpmu_audio->pll_use_count);
	mutex_lock(&bcmpmu_audio->lock);
	if (bcmpmu_audio->pll_use_count++ == 0)	{
		/* set PLLEN and AUDIO EN */
		bcmpmu->read_dev(bcmpmu,
				 PMU_REG_PLLCTRL, &reg.val, PMU_BITMASK_ALL);
		reg.val |= (BCMPMU_PLL_EN | BCMPMU_PLL_AUDIO_EN);
		bcmpmu->write_dev(bcmpmu,
				  PMU_REG_PLLCTRL, reg.val, PMU_BITMASK_ALL);

		/* HS reset disable */
		bcmpmu->write_dev(bcmpmu,
				  PMU_REG_HSPUP1_IDDQ_PWRDWN,
				  false,
			      bcmpmu->regmap[PMU_REG_HSPUP1_IDDQ_PWRDWN].mask);

		/* enable class G HS */
		bcmpmu->write_dev(bcmpmu,
				  PMU_REG_HSCP3_CP_CG_SEL,
				  bcmpmu->regmap[PMU_REG_HSCP3_CP_CG_SEL].mask,
				  bcmpmu->regmap[PMU_REG_HSCP3_CP_CG_SEL].mask);
		/* enable HS offset correction */
		/*bcmpmu->write_dev(bcmpmu,
				  PMU_REG_HSIST_OC_DISOCMUX, false,
			     bcmpmu->regmap[PMU_REG_HSIST_OC_DISOCMUX].mask);*/

		/* IHF reset disable */
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFTOP_IHF_IDDQ,
			  false,
			  bcmpmu->regmap[PMU_REG_IHFTOP_IHF_IDDQ].mask);
		/* Bypass IHF ALC/APS, so that IHF gain can
		 * be controlled manually
		*/
		/*bcmpmu->write_dev(bcmpmu,
				  PMU_REG_IHFALC_BYPASS,
				  bcmpmu->regmap[PMU_REG_IHFALC_BYPASS].mask,
				  bcmpmu->regmap[PMU_REG_IHFALC_BYPASS].mask);*/

		/* set IHF pop ramp time */
		bcmpmu->write_dev(bcmpmu,
			       PMU_REG_IHFPOP_POPTIME_CTL, 0x2,
			       bcmpmu->regmap[PMU_REG_IHFPOP_POPTIME_CTL].mask);
	}
	mutex_unlock(&bcmpmu_audio->lock);
}
EXPORT_SYMBOL(bcmpmu_audio_init);

void bcmpmu_audio_deinit(void)
{
	struct bcmpmu_rw_data reg;
	struct bcmpmu *bcmpmu;
	if (!bcmpmu_audio)
		return;
	bcmpmu = bcmpmu_audio->bcmpmu;
	pr_debug("%s: ###### - pll_use_count = %u\n",
		__func__, bcmpmu_audio->pll_use_count);
	mutex_lock(&bcmpmu_audio->lock);
	if (bcmpmu_audio->pll_use_count &&
			--bcmpmu_audio->pll_use_count == 0) {
		/* HS reset enable */
		bcmpmu->write_dev(bcmpmu,
			PMU_REG_HSPUP1_IDDQ_PWRDWN,
		 bcmpmu->regmap[PMU_REG_HSPUP1_IDDQ_PWRDWN].mask,
		 bcmpmu->regmap[PMU_REG_HSPUP1_IDDQ_PWRDWN].mask);
		/* disable HS offset correction */
		/*bcmpmu->write_dev(bcmpmu,
			PMU_REG_HSIST_OC_DISOCMUX,
		 bcmpmu->regmap[PMU_REG_HSIST_OC_DISOCMUX].mask,
		 bcmpmu->regmap[PMU_REG_HSIST_OC_DISOCMUX].mask);*/

		/* IHF Reset Enable */
		bcmpmu->write_dev(bcmpmu, PMU_REG_IHFTOP_IHF_IDDQ,
			  bcmpmu->regmap[PMU_REG_IHFTOP_IHF_IDDQ].mask,
			  bcmpmu->regmap[PMU_REG_IHFTOP_IHF_IDDQ].mask);
		/* Enable IHF ALC/APS */
		/*bcmpmu->write_dev(bcmpmu, PMU_REG_IHFALC_BYPASS, false,
			  bcmpmu->regmap[PMU_REG_IHFALC_BYPASS].mask);*/

		bcmpmu->read_dev(bcmpmu,
			PMU_REG_PLLCTRL, &reg.val, PMU_BITMASK_ALL);
		reg.val &= ~(BCMPMU_PLL_EN | BCMPMU_PLL_AUDIO_EN);
		bcmpmu->write_dev(bcmpmu,
			PMU_REG_PLLCTRL, reg.val, PMU_BITMASK_ALL);
		bcmpmu_audio->HS_On = false;
		bcmpmu_audio->IHF_On = false;
	}
	mutex_unlock(&bcmpmu_audio->lock);
}
EXPORT_SYMBOL(bcmpmu_audio_deinit);



#ifdef CONFIG_DEBUG_FS
static int audio_hs_setgain(void *data, u64 val)
{
	bcmpmu_hs_set_gain(PMU_AUDIO_HS_BOTH, val);
	return 0;
}
DEFINE_SIMPLE_ATTRIBUTE(dbg_hs_gain, NULL,
	audio_hs_setgain, "%llu\n");

static int audio_ihf_setgain(void *data, u64 val)
{
	bcmpmu_ihf_set_gain(val);
	return 0;
}
DEFINE_SIMPLE_ATTRIBUTE(dbg_ihf_gain, NULL,
	audio_ihf_setgain, "%llu\n");

static int audio_hs_on(void *data, u64 val)
{
	bcmpmu_hs_power((bool)val);
	return 0;
}
DEFINE_SIMPLE_ATTRIBUTE(dbg_hs_on, NULL,
	audio_hs_on, "%llu\n");

static int audio_ihf_on(void *data, u64 val)
{
	if (val)
		bcmpmu_ihf_power(true);
	else
		bcmpmu_ihf_power(false);

	return 0;
}
DEFINE_SIMPLE_ATTRIBUTE(dbg_ihf_on, NULL,
	audio_ihf_on, "%llu\n");
#endif

static int __devinit bcmpmu_audio_probe(struct platform_device *pdev)
{
	struct bcmpmu *bcmpmu = pdev->dev.platform_data;
	struct bcmpmu_audio *pdata;
	struct bcmpmu_rw_data reg;
#ifdef CONFIG_DEBUG_FS
	struct dentry *audio_dir = NULL, *hs_gain = NULL, *ihf_gain = NULL, *hs_on = NULL, *ihf_on = NULL;
#endif

	printk(KERN_INFO "%s: called.\n", __func__);
	if (!bcmpmu->pmu_rev)
		return -ENODEV;

	pdata = kzalloc(sizeof(struct bcmpmu_audio), GFP_KERNEL);
	if (pdata == NULL) {
		printk(KERN_ERR "%s: failed to alloc mem.\n", __func__);
		return -ENOMEM;
	}
	mutex_init(&pdata->lock);
	pdata->pll_use_count = 0;
	pdata->bcmpmu = bcmpmu;
	bcmpmu_audio = pdata;
	bcmpmu_audio->HS_On = false;
	bcmpmu_audio->IHF_On = false;
	bcmpmu_audio->ihf_autoseq_dis = bcmpmu->pdata->ihf_autoseq_dis;

	/* Enable auto sequence for IHF power up and power down */
	bcmpmu_audio->bcmpmu->read_dev(bcmpmu_audio->bcmpmu,PMU_REG_IHFPOP_PUP,&reg.val,PMU_BITMASK_ALL);
	reg.val |= BCMPMU_IHFPOP_AUTOSEQ;  /*IHFPOP*/
	bcmpmu_audio->bcmpmu->write_dev(bcmpmu_audio->bcmpmu,PMU_REG_IHFPOP_PUP,reg.val,PMU_BITMASK_ALL);

#ifdef CONFIG_DEBUG_FS
	if (bcmpmu->debugfs_root_dir == NULL)
		goto debugfs_err;
	audio_dir = debugfs_create_dir("audio", bcmpmu->debugfs_root_dir);
	if (!audio_dir)
		goto debugfs_err;
	hs_gain = debugfs_create_file("hs_gain", 0644, audio_dir, pdata, &dbg_hs_gain);
	if (!hs_gain)
		goto debugfs_err;
	ihf_gain = debugfs_create_file("ihf_gain", 0644, audio_dir, pdata, &dbg_ihf_gain);
	if (!ihf_gain)
		goto debugfs_err;
	hs_on = debugfs_create_file("hs_on", 0644, audio_dir, pdata, &dbg_hs_on);
	if (!hs_on)
		goto debugfs_err;
	ihf_on = debugfs_create_file("ihf_on", 0644, audio_dir, pdata, &dbg_ihf_on);
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
