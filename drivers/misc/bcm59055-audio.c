/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*       @file   drivers/audio/bcm59055-audio.c
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

/**
*
*   @file   bcm59055-audio.c
*
*   @brief  Regulator Driver for Broadcom BCM59055 PMU
*
****************************************************************************/

#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/ioport.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/ioctl.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/broadcom/bcmpmu_audio.h>
/*#include <linux/mfd/bcm590xx/bcm59055_A0.h>*/
#include <linux/mfd/bcm590xx/core.h>

struct bcm59055_audio {
	struct bcm590xx *bcm590xx;
	bool IHFenabled;
	bool HSenabled;
	bool IHFBypassEn;
	bool classAB;
	bool i2cMethod;
	int HSInputMode;
	struct mutex lock;
};
static struct bcm59055_audio *driv_data;


int bcm59055_ihf_set_gain(int IHFGain)
{
	int data;
	int ret;
	struct bcm590xx *bcm590xx = driv_data->bcm590xx;
	pr_debug("Inside %s IHFGain %d\n", __func__, IHFGain);
	mutex_lock(&driv_data->lock);
	data = bcm590xx_reg_read(bcm590xx, BCM59055_REG_IHFPGA2);
	data &= ~BCM59055_IHFPGA2_CTL_MASK;
	data |= (BCM59055_IHFPGA2_CTL_MASK & IHFGain);

	ret = bcm590xx_reg_write(bcm590xx, BCM59055_REG_IHFPGA2, data);
	mutex_unlock(&driv_data->lock);

	return ret;
}
EXPORT_SYMBOL(bcm59055_ihf_set_gain);

int bcm59055_ihf_bypass_en(bool enable)
{
	int data;
	int ret;
	struct bcm590xx *bcm590xx = driv_data->bcm590xx;
	pr_debug("Inside %s enable is %d\n", __func__, enable);
	mutex_lock(&driv_data->lock);
	data = bcm590xx_reg_read(bcm590xx, BCM59055_REG_IHFTOP);
	if (enable) {
		if (driv_data->IHFBypassEn) {
			pr_info("%s: IHF Bypass already enabled\n", __func__);
			mutex_unlock(&driv_data->lock);
			return -EPERM;
		}
		data |= (BCM59055_IHFTOP_BYPASSPUP | BCM59055_IHFTOP_BYPASS);
		driv_data->IHFBypassEn = true;
	} else {
		if (!driv_data->IHFBypassEn) {
			pr_info("%s: IHF Bypass already disabled\n", __func__);
			mutex_unlock(&driv_data->lock);
			return -EPERM;
		}
		data &= ~(BCM59055_IHFTOP_BYPASSPUP | BCM59055_IHFTOP_BYPASS);
		driv_data->IHFBypassEn = false;
	}

	ret =  bcm590xx_reg_write(bcm590xx, BCM59055_REG_IHFTOP, data);
	mutex_unlock(&driv_data->lock);
	return ret;
}
EXPORT_SYMBOL(bcm59055_ihf_bypass_en);

int bcm59055_ihf_power(bool on)
{
	int data, ret = 0;
	struct bcm590xx *bcm590xx = driv_data->bcm590xx;
	pr_debug("Inside %s, ON is %d\n", __func__, on);
	mutex_lock(&driv_data->lock);

	if (on) {
		if (on && driv_data->IHFenabled) {
			pr_info("%s: IHF is already enabled\n", __func__);
			mutex_unlock(&driv_data->lock);
			return -EPERM;
		}
		data = bcm590xx_reg_read(bcm590xx, BCM59055_REG_IHFTOP);
		data &= ~BCM59055_IHFTOP_IDDQ;
		driv_data->IHFenabled = true;
		ret = bcm590xx_reg_write(bcm590xx, BCM59055_REG_IHFTOP, data);

		data = bcm590xx_reg_read(bcm590xx, BCM59055_REG_IHFLDO);
		data |= BCM59055_IHFLDO_PUP;
		ret |= bcm590xx_reg_write(bcm590xx, BCM59055_REG_IHFLDO, data);
	} else {
		if (!on && !driv_data->IHFenabled) {
			pr_info("%s: IHF is already Off\n", __func__);
			mutex_unlock(&driv_data->lock);
			return -EPERM;
		}
		data = bcm590xx_reg_read(bcm590xx, BCM59055_REG_IHFPOP);
		data &= ~BCM59055_IHFPOP_EN;
		ret = bcm590xx_reg_write(bcm590xx, BCM59055_REG_IHFPOP, data);
		data = bcm590xx_reg_read(bcm590xx, BCM59055_REG_IHFTOP);
		data |= BCM59055_IHFTOP_IDDQ;
		ret |= bcm590xx_reg_write(bcm590xx, BCM59055_REG_IHFTOP, data);
		driv_data->IHFenabled = false;
	}
	mutex_unlock(&driv_data->lock);
	return ret;
}
EXPORT_SYMBOL(bcm59055_ihf_power);

int bcm59055_hs_set_gain(int HSpath, int HSgain)
{
	int data1, data2;
	int ret;
	struct bcm590xx *bcm590xx = driv_data->bcm590xx;
	pr_debug("Inside %s, HsPath %d, HSgain %d\n", __func__, HSpath, HSgain);
	mutex_lock(&driv_data->lock);

	data1 = bcm590xx_reg_read(bcm590xx, BCM59055_REG_HSPGA1);
	data2 = bcm590xx_reg_read(bcm590xx, BCM59055_REG_HSPGA2);
	if (HSpath == PMU_AUDIO_HS_LEFT) {
		data1 &= ~BCM59055_PGA_CTL_MASK;
		data1 |= (HSgain & BCM59055_PGA_CTL_MASK);
	} else if (HSpath == PMU_AUDIO_HS_RIGHT) {
		data2 &= ~BCM59055_PGA_CTL_MASK;
		data2 |= (HSgain & BCM59055_PGA_CTL_MASK);
	} else {
		data1 &= ~BCM59055_PGA_CTL_MASK;
		data2 &= ~BCM59055_PGA_CTL_MASK;
		data1 |= (HSgain & BCM59055_PGA_CTL_MASK);
		data2 |= (HSgain & BCM59055_PGA_CTL_MASK);
	}
	ret = bcm590xx_reg_write(bcm590xx, BCM59055_REG_HSPGA1, data1);
	ret |= bcm590xx_reg_write(bcm590xx, BCM59055_REG_HSPGA2, data2);

	mutex_unlock(&driv_data->lock);
	return ret;
}
EXPORT_SYMBOL(bcm59055_hs_set_gain);

int bcm59055_hs_sc_thold(int curr)
{
	int data, ret;
	struct bcm590xx *bcm590xx = driv_data->bcm590xx;
	pr_debug("Inside %s\n", __func__);
	data = bcm590xx_reg_read(bcm590xx, BCM59055_REG_HSDRV);
	data &= ~BCM59055_HSDRV_DRV_SCCTL_MASK;

	data |= (curr & BCM59055_HSDRV_DRV_SCCTL_MASK);
	ret = bcm590xx_reg_write(bcm590xx, BCM59055_REG_HSDRV, data);
	return ret;
}
EXPORT_SYMBOL(bcm59055_hs_sc_thold);

int bcm59055_hs_set_input_mode(int HSgain, int HSInputmode)
{
	int data1, data2, data3;
	struct bcm590xx *bcm590xx = driv_data->bcm590xx;
	int ret = 0;
	int HSwasEn = 0;
	pr_debug("Inside %s, HSgain %d, HSInputmode %d\n",
		__func__, HSgain, HSInputmode);

	mutex_lock(&driv_data->lock);
	if (HSInputmode == driv_data->HSInputMode) {
		pr_info("%s: Input mode already configured\n", __func__);
		mutex_unlock(&driv_data->lock);
		return -EINVAL;
	}

	if (driv_data->HSenabled) {
		data1 = bcm590xx_reg_read(bcm590xx, BCM59055_REG_HSPUP2);
		data1 &= ~BCM59055_HSPUP2_HS_PWRUP;
		bcm590xx_reg_write(bcm590xx, BCM59055_REG_HSPUP2, data1);
		driv_data->HSenabled = false;
		HSwasEn = 1;
	}

	data1 = bcm590xx_reg_read(bcm590xx, BCM59055_REG_HSPGA1);
	data2 = bcm590xx_reg_read(bcm590xx, BCM59055_REG_HSPGA2);
	data3 = bcm590xx_reg_read(bcm590xx, BCM59055_REG_HSPGA3);

	if (HSInputmode == PMU_HS_SINGLE_ENDED_AC_COUPLED) {
		data1 &= ~BCM59055_PGA_CTL_MASK;
		data1 |= (BCM59055_HSPGA1_PGA_GAINL
			| BCM59055_HSPGA1_PGA_GAINR |
			(HSgain & BCM59055_PGA_CTL_MASK));

		data2 &= ~BCM59055_PGA_CTL_MASK;
		data2 |= HSgain & BCM59055_PGA_CTL_MASK;

		data3 &= ~(BCM59055_HSPGA3_PGA_ACINADJ
			| BCM59055_HSPGA3_PGA_PULLDNSJ);
		data3 |= BCM59055_HSPGA3_PGA_ENACCPL;
	} else if (HSInputmode == PMU_HS_DIFFERENTIAL_AC_COUPLED) {
		data1 &= ~(BCM59055_HSPGA1_PGA_GAINL
			| BCM59055_HSPGA1_PGA_GAINR |
			BCM59055_PGA_CTL_MASK);
		data1 |= HSgain & BCM59055_PGA_CTL_MASK;

		data2 &= ~BCM59055_PGA_CTL_MASK;
		data2 |= HSgain & BCM59055_PGA_CTL_MASK;

		data3 &= ~BCM59055_HSPGA3_PGA_PULLDNSJ;
		data3 |= (BCM59055_HSPGA3_PGA_ENACCPL
			| BCM59055_HSPGA3_PGA_ACINADJ);
	} else if (HSInputmode == PMU_HS_DIFFERENTIAL_DC_COUPLED) {
		data1 &=
			~(BCM59055_HSPGA1_PGA_GAINL
			| BCM59055_HSPGA1_PGA_GAINR |
			BCM59055_PGA_CTL_MASK);
		data1 |= HSgain & BCM59055_PGA_CTL_MASK;

		data2 &= ~BCM59055_PGA_CTL_MASK;
		data2 |= HSgain & BCM59055_PGA_CTL_MASK;

		data3 &=
			~(BCM59055_HSPGA3_PGA_PULLDNSJ
			| BCM59055_HSPGA3_PGA_ENACCPL |
			BCM59055_HSPGA3_PGA_ACINADJ);
	}

	ret = bcm590xx_reg_write(bcm590xx, BCM59055_REG_HSPGA1, data1);
	ret |= bcm590xx_reg_write(bcm590xx, BCM59055_REG_HSPGA2, data2);
	ret |= bcm590xx_reg_write(bcm590xx, BCM59055_REG_HSPGA3, data3);

	/* Power Up HS */
	if (HSwasEn) {
		data1 = bcm590xx_reg_read(bcm590xx, BCM59055_REG_HSPUP2);
		data1 |= BCM59055_HSPUP2_HS_PWRUP;
		ret |= bcm590xx_reg_write(bcm590xx, BCM59055_REG_HSPUP2, data1);
		HSwasEn = 0;
		driv_data->HSenabled = true;
	}

	driv_data->HSInputMode = HSInputmode;
	mutex_unlock(&driv_data->lock);

	return ret;
}
EXPORT_SYMBOL(bcm59055_hs_set_input_mode);

int bcm59055_hs_shortcircuit_en(bool enable)
{
	int data;
	int ret;
	struct bcm590xx *bcm590xx = driv_data->bcm590xx;
	pr_debug("Inside %s, enable %d\n", __func__, enable);
	data = bcm590xx_reg_read(bcm590xx, BCM59055_REG_HSDRV);

	if (enable)
		data |= BCM59055_HSDRV_DRV_DISSC;
	else
		data &= ~BCM59055_HSDRV_DRV_DISSC;

	ret =  bcm590xx_reg_write(bcm590xx, BCM59055_REG_HSDRV, data);
	return ret;
}
EXPORT_SYMBOL(bcm59055_hs_shortcircuit_en);

int bcm59055_hs_power(bool on)
{
	int data1, data2;
	struct bcm590xx *bcm590xx = driv_data->bcm590xx;
	int ret = 0;
	pr_debug("Inside %s, ON %d\n", __func__, on);
	mutex_lock(&driv_data->lock);
	data1 = bcm590xx_reg_read(bcm590xx, BCM59055_REG_HSPUP1);
	data2 = bcm590xx_reg_read(bcm590xx, BCM59055_REG_HSPUP2);
	if (on) {
		data1 &= ~BCM59055_HSPUP1_IDDQ_PWRDN;
		data2 |= BCM59055_HSPUP2_HS_PWRUP;
		driv_data->HSenabled = true;
		ret = bcm590xx_reg_write(bcm590xx, BCM59055_REG_HSPUP1, data1);
		ret |= bcm590xx_reg_write(bcm590xx, BCM59055_REG_HSPUP2, data2);

		bcm59055_hs_shortcircuit_en(true);
	} else {
		data2 &= ~BCM59055_HSPUP2_HS_PWRUP;
		data1 |= BCM59055_HSPUP1_IDDQ_PWRDN;
		ret = bcm590xx_reg_write(bcm590xx, BCM59055_REG_HSPUP2, data2);
		ret |= bcm590xx_reg_write(bcm590xx, BCM59055_REG_HSPUP1, data1);
		driv_data->HSenabled = false;
		bcm59055_hs_shortcircuit_en(false);
	}
	mutex_unlock(&driv_data->lock);
	return ret;
}
EXPORT_SYMBOL(bcm59055_hs_power);

int bcm59055_hs_class_sel(bool i2cmethod, bool classAB)
{
	int data;
	int ret;
	struct bcm590xx *bcm590xx = driv_data->bcm590xx;
	pr_debug("Inside %s\n", __func__);
	mutex_lock(&driv_data->lock);
	data = bcm590xx_reg_read(bcm590xx, BCM59055_REG_HSCP3);
	/*data |= BCM59055_IHFPOP_AUTOSEQ;*/

	if (driv_data->i2cMethod) {
		data &= ~BCM59055_HSCP3_CG_SEL;
		if (driv_data->classAB)
			data &= ~BCM59055_HSCP3_CG_I2C;
		else
			data |= BCM59055_HSCP3_CG_I2C;
		driv_data->i2cMethod = 1;
	} else {
		data |= BCM59055_HSCP3_CG_SEL;
		driv_data->i2cMethod = 0;
	}
	ret = bcm590xx_reg_write(bcm590xx, BCM59055_REG_HSCP3, data);
	ret = bcm590xx_reg_write(bcm590xx, BCM59055_REG_PLLCTRL, data);
	mutex_unlock(&driv_data->lock);

	return ret;
}
EXPORT_SYMBOL(bcm59055_hs_class_sel);

int bcm59055_audio_init(void)
{
	int data, ret = 0;
	struct bcm590xx *bcm590xx = driv_data->bcm590xx;
	pr_debug("Inside %s\n", __func__);

	mutex_lock(&driv_data->lock);
	data = bcm590xx_reg_read(bcm590xx, BCM59055_REG_PLLCTRL);
	data |= (BCM59055_PLLCTRL_AUDIO_EN | BCM59055_PLLCTRL_PLLEN);
	ret = bcm590xx_reg_write(bcm590xx, BCM59055_REG_PLLCTRL, data);
	mutex_unlock(&driv_data->lock);
	return ret;
}
EXPORT_SYMBOL(bcm59055_audio_init);

int bcm59055_audio_deinit(void)
{
	int data;
	int ret;
	struct bcm590xx *bcm590xx = driv_data->bcm590xx;
	pr_debug("Inside %s\n", __func__);
	mutex_lock(&driv_data->lock);
	data = bcm590xx_reg_read(bcm590xx, BCM59055_REG_PLLCTRL);
	data &= ~(BCM59055_PLLCTRL_AUDIO_EN | BCM59055_PLLCTRL_PLLEN);
	driv_data->IHFenabled = 0;
	driv_data->HSenabled = 0;
	driv_data->IHFBypassEn = 0;
	ret = bcm590xx_reg_write(bcm590xx, BCM59055_REG_PLLCTRL, data);
	mutex_unlock(&driv_data->lock);
	return ret;
}
EXPORT_SYMBOL(bcm59055_audio_deinit);

/* Functions for selftest */
int bcm59055_audio_ihf_selftest_stimulus_input(int stimulus)
{
	int data;
	struct bcm590xx *bcm590xx = driv_data->bcm590xx;

	data  =  bcm590xx_reg_read(bcm590xx,
				      BCM59055_REG_IHFSTIN);
	return bcm590xx_reg_write(bcm590xx,
			BCM59055_REG_IHFSTIN,
			(data & ~PMU_IHFSTIN_MASK_I_IHFSTI)
			|(stimulus << PMU_IHFSTIN_OFFSET_I_IHFSTI));
}
EXPORT_SYMBOL(bcm59055_audio_ihf_selftest_stimulus_input);

int bcm59055_audio_ihf_selftest_stimulus_output(int stimulus)
{
	int data;
	struct bcm590xx *bcm590xx = driv_data->bcm590xx;
	data  =  bcm590xx_reg_read(bcm590xx,
				      BCM59055_REG_IHFSTIN);
	return bcm590xx_reg_write(bcm590xx,
			BCM59055_REG_IHFSTIN,
			(data & ~PMU_IHFSTIN_MASK_I_IHFSTO)
			| (stimulus << PMU_IHFSTIN_OFFSET_I_IHFSTO));


}
EXPORT_SYMBOL(bcm59055_audio_ihf_selftest_stimulus_output);

void bcm59055_audio_ihf_selftest_result(u8 *result)
{
	struct bcm590xx *bcm590xx = driv_data->bcm590xx;

	*result = bcm590xx_reg_read(bcm590xx, BCM59055_REG_IHFSTO);

	*result =
		(*result&PMU_IHFSTO_MASK_O_IHFSTI)>>PMU_IHFSTO_OFFSET_O_IHFSTI;

}
EXPORT_SYMBOL(bcm59055_audio_ihf_selftest_result);

int bcm59055_audio_ihf_testmode(int Mode)
{
	int data;
	struct bcm590xx *bcm590xx = driv_data->bcm590xx;

	data  =  bcm590xx_reg_read(bcm590xx,
				      BCM59055_REG_IHFSTIN);
	return bcm590xx_reg_write(bcm590xx, BCM59055_REG_IHFSTIN,
			   (data & ~PMU_IHFSTIN_MASK_I_IHFSELFTEST_EN)
			   |(Mode << PMU_IHFSTIN_OFFSET_I_IHFSELFTEST_EN));
}
EXPORT_SYMBOL(bcm59055_audio_ihf_testmode);


int bcm59055_audio_hs_selftest_stimulus(int stimulus)
{
	int data;
	struct bcm590xx *bcm590xx = driv_data->bcm590xx;

	data  =  (u8)bcm590xx_reg_read(bcm590xx,
					   BCM59055_REG_HSIST);
	return bcm590xx_reg_write(bcm590xx, BCM59055_REG_HSIST,
			(data & ~PMU_HSIST_MASK_I_HS_IST)
			| (stimulus << PMU_HSIST_OFFSET_I_HS_IST));

}
EXPORT_SYMBOL(bcm59055_audio_hs_selftest_stimulus);

void bcm59055_audio_hs_selftest_result(u8 *result)
{
	struct bcm590xx *bcm590xx = driv_data->bcm590xx;

	*result = bcm590xx_reg_read(bcm590xx, BCM59055_REG_HSOUT1);
	*result = *result>>PMU_HSOUT1_OFFSET_O_HS_IST;
}
EXPORT_SYMBOL(bcm59055_audio_hs_selftest_result);

int bcm59055_audio_hs_testmode(int Mode)
{
	int data;
	struct bcm590xx *bcm590xx = driv_data->bcm590xx;


	/* 1. Enable test mode (driving buffer enabled)
	(i_hs_enst[1:0]  =  '11') on PMU
	*/
	data  =  (u8)bcm590xx_reg_read(bcm590xx,
					   BCM59055_REG_HSIST);
	return bcm590xx_reg_write(bcm590xx, BCM59055_REG_HSIST,
			    (data & ~PMU_HSIST_MASK_I_HS_ENST)
				|(Mode << PMU_HSIST_OFFSET_I_HS_ENST));

}
EXPORT_SYMBOL(bcm59055_audio_hs_testmode);



static int __devinit bcm59055_audio_probe(struct platform_device *pdev)
{
	struct bcm590xx *bcm590xx = dev_get_drvdata(pdev->dev.parent);
	struct bcm59055_audio *audio_data;
	int data;
	pr_info("Inside %s\n", __func__);

	if (!bcm590xx)
		return -EINVAL;
	audio_data = kzalloc((sizeof(struct bcm59055_audio)), GFP_KERNEL);
	if (!audio_data) {
		pr_err("%s : Can not allocate memory\n", __func__);
		return -ENOMEM;
	}
	mutex_init(&audio_data->lock);
	audio_data->HSenabled = false;
	audio_data->IHFenabled = false;
	audio_data->IHFBypassEn = false;
	audio_data->bcm590xx = bcm590xx;
	driv_data = audio_data;
	platform_set_drvdata(pdev, audio_data);
	/* Enable auto sequence for IHF power up and power down */
	data = bcm590xx_reg_read(bcm590xx, BCM59055_REG_IHFPOP);
	data |= BCM59055_IHFPOP_AUTOSEQ;
	bcm590xx_reg_write(bcm590xx, BCM59055_REG_IHFPOP, data);
	/*set default miser input, mode and per-amp gain settings*/
	if (bcm590xx->pdata && bcm590xx->pdata->audio_pdata) {
		struct bcm590xx_audio_pdata *audio_pdata =
			bcm590xx->pdata->audio_pdata;

		if (!bcm59055_hs_class_sel(audio_pdata->i2cmethod,
				audio_pdata->classAB))
			pr_info("%s: Unable to set class\n", __func__);

		bcm59055_hs_set_input_mode(audio_pdata->hsgain,
			audio_pdata->hsinputmode);
		bcm59055_hs_sc_thold(audio_pdata->sc_thold);
		bcm59055_ihf_set_gain(audio_pdata->ihf_gain);
		bcm59055_ihf_bypass_en(audio_pdata->ihf_bypass_en);
	}
	/* turn off the shortcircuit logic, once HS is ON
	 * it would be turned on again */
	bcm59055_hs_shortcircuit_en(false);

	return 0;
}

static int __devexit bcm59055_audio_remove(struct platform_device *pdev)
{
	struct bcm59055_audio *audio_data = platform_get_drvdata(pdev);
	kfree(audio_data);
	driv_data = NULL;
	return 0;
}

static struct platform_driver bcm59055_audio_driver = {
	.driver = {
		.name	= "bcm590xx-audio",
		.owner	= THIS_MODULE,
	},
	.remove		= __devexit_p(bcm59055_audio_remove),
	.probe		= bcm59055_audio_probe,
};

static int __init bcm59055_audio_drv_init(void)
{
	return platform_driver_register(&bcm59055_audio_driver);
}
late_initcall(bcm59055_audio_drv_init);

static void __exit bcm59055_audio_drv_exit(void)
{
	platform_driver_unregister(&bcm59055_audio_driver);
}
module_exit(bcm59055_audio_drv_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Audio Interface Driver for BCM59055 PMU");
MODULE_ALIAS("platform:bcm59055-audio");
