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

#include <linux/mfd/bcmpmu.h>
#include "bcmpmu_audio.h"

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
	struct bcmpmu_rw_data reg1,reg2;

	bcmpmu_audio->bcmpmu->read_dev(bcmpmu_audio->bcmpmu,PMU_REG_HSPUP1_IDDQ_PWRDWN,&reg1.val,PMU_BITMASK_ALL);
	bcmpmu_audio->bcmpmu->read_dev(bcmpmu_audio->bcmpmu,PMU_REG_HSPUP2_HS_PWRUP,&reg2.val,PMU_BITMASK_ALL);

	if (on) {
		reg1.val &= ~BCMPMU_HSPUP1_IDDQ_PWRDN;  //HSPUP1
		reg2.val |= BCMPMU_HSPUP2_HS_PWRUP;     //HSPUP2
		bcmpmu_audio->HS_On = true;
	} else {
		reg1.val |= BCMPMU_HSPUP1_IDDQ_PWRDN;  //HSPUP1
		reg2.val &= ~BCMPMU_HSPUP2_HS_PWRUP;   //HSPUP2
		bcmpmu_audio->HS_On = false;
	}
	bcmpmu_audio->bcmpmu->write_dev(bcmpmu_audio->bcmpmu,PMU_REG_HSPUP1_IDDQ_PWRDWN,reg1.val,PMU_BITMASK_ALL);
	bcmpmu_audio->bcmpmu->write_dev(bcmpmu_audio->bcmpmu,PMU_REG_HSPUP2_HS_PWRUP,reg2.val,PMU_BITMASK_ALL);
}
EXPORT_SYMBOL(bcmpmu_hs_power);



void bcmpmu_ihf_power(bool on)
{
    struct bcmpmu_rw_data reg;

	if(on) {
	    if(bcmpmu_audio->IHF_On) {
			printk(KERN_INFO "%s: IHF is already on.\n", __func__);
			return -EPERM;
		}
		bcmpmu_audio->IHF_On = true;

		bcmpmu_audio->bcmpmu->read_dev(bcmpmu_audio->bcmpmu,PMU_REG_IHFTOP_IHF_IDDQ,&reg.val,PMU_BITMASK_ALL);
		reg.val &= ~BCMPMU_IHFTOP_IDDQ;  //IHFTOP
		bcmpmu_audio->bcmpmu->write_dev(bcmpmu_audio->bcmpmu,PMU_REG_IHFTOP_IHF_IDDQ,reg.val,PMU_BITMASK_ALL);

		bcmpmu_audio->bcmpmu->read_dev(bcmpmu_audio->bcmpmu,PMU_REG_IHFLDO_PUP,&reg.val,PMU_BITMASK_ALL);
		reg.val |= BCMPMU_IHFPOP_PUP;  //IHFPOP
		bcmpmu_audio->bcmpmu->write_dev(bcmpmu_audio->bcmpmu,PMU_REG_IHFLDO_PUP,reg.val,PMU_BITMASK_ALL);

		bcmpmu_audio->bcmpmu->read_dev(bcmpmu_audio->bcmpmu,PMU_REG_IHFLDO_PUP,&reg.val,PMU_BITMASK_ALL);
		reg.val |= BCMPMU_IHFLDO_PUP;  //IHFLDO
		bcmpmu_audio->bcmpmu->write_dev(bcmpmu_audio->bcmpmu,PMU_REG_IHFLDO_PUP,reg.val,PMU_BITMASK_ALL);
	}
    else{
	    if(!bcmpmu_audio->IHF_On) {
			printk(KERN_INFO "%s: IHF is already off.\n", __func__);
			return -EPERM;
		}
		bcmpmu_audio->IHF_On = false;

		bcmpmu_audio->bcmpmu->read_dev(bcmpmu_audio->bcmpmu,PMU_REG_IHFPOP_PUP,&reg.val,PMU_BITMASK_ALL);
		reg.val &= ~BCMPMU_IHFPOP_PUP;  //IHFPOP
		bcmpmu_audio->bcmpmu->write_dev(bcmpmu_audio->bcmpmu,PMU_REG_IHFPOP_PUP,reg.val,PMU_BITMASK_ALL);

		bcmpmu_audio->bcmpmu->read_dev(bcmpmu_audio->bcmpmu,PMU_REG_IHFLDO_PUP,&reg.val,PMU_BITMASK_ALL);
		reg.val &= ~BCMPMU_IHFLDO_PUP; //IHFLDO
		bcmpmu_audio->bcmpmu->write_dev(bcmpmu_audio->bcmpmu,PMU_REG_IHFLDO_PUP,reg.val,PMU_BITMASK_ALL);

		bcmpmu_audio->bcmpmu->read_dev(bcmpmu_audio->bcmpmu,PMU_REG_IHFTOP_IHF_IDDQ,&reg.val,PMU_BITMASK_ALL);
		reg.val |= BCMPMU_IHFTOP_IDDQ;  //IHFTOP
		bcmpmu_audio->bcmpmu->write_dev(bcmpmu_audio->bcmpmu,PMU_REG_IHFTOP_IHF_IDDQ,reg.val,PMU_BITMASK_ALL);

	}
}
EXPORT_SYMBOL(bcmpmu_ihf_power);


void bcmpmu_hs_set_gain(bcmpmu_hs_path_t path, bcmpmu_hs_gain_t gain)
{
	struct bcmpmu_rw_data reg1,reg2;

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
	else{  //for PMU_AUDIO_HS_BOTH
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
	struct bcmpmu_rw_data reg;

	bcmpmu_audio->bcmpmu->read_dev(bcmpmu_audio->bcmpmu,PMU_REG_IHFPGA2_GAIN,&reg.val,PMU_BITMASK_ALL);
	reg.val &= ~BCMPMU_IHF_GAIN_MASK;
	reg.val |= (gain & BCMPMU_IHF_GAIN_MASK);
	bcmpmu_audio->bcmpmu->write_dev(bcmpmu_audio->bcmpmu,PMU_REG_IHFPGA2_GAIN,reg.val,PMU_BITMASK_ALL);
}
EXPORT_SYMBOL(bcmpmu_ihf_set_gain);



void bcmpmu_audio_init(void)
{
	struct bcmpmu_rw_data reg;

	bcmpmu_audio->bcmpmu->read_dev(bcmpmu_audio->bcmpmu,PMU_REG_PLLCTRL,&reg.val,PMU_BITMASK_ALL);
	reg.val |= (BCMPMU_PLL_EN | BCMPMU_PLL_AUDIO_EN);
	bcmpmu_audio->bcmpmu->write_dev(bcmpmu_audio->bcmpmu,PMU_REG_PLLCTRL,reg.val,PMU_BITMASK_ALL);
}
EXPORT_SYMBOL(bcmpmu_audio_init);



void bcmpmu_audio_deinit(void)
{
	struct bcmpmu_rw_data reg;

	bcmpmu_audio->bcmpmu->read_dev(bcmpmu_audio->bcmpmu,PMU_REG_PLLCTRL,&reg.val,PMU_BITMASK_ALL);
	reg.val &= ~(BCMPMU_PLL_EN | BCMPMU_PLL_AUDIO_EN);
	bcmpmu_audio->bcmpmu->write_dev(bcmpmu_audio->bcmpmu,PMU_REG_PLLCTRL,reg.val,PMU_BITMASK_ALL);

	bcmpmu_audio->HS_On = false;
	bcmpmu_audio->IHF_On = false;
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
	int ret;

	struct bcmpmu *bcmpmu = pdev->dev.platform_data;
	struct bcmpmu_audio *pdata;
	struct bcmpmu_rw_data reg;
#ifdef CONFIG_DEBUG_FS
	struct dentry *audio_dir, *hs_gain, *ihf_gain, *hs_on, *ihf_on;
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

        // Enable auto sequence for IHF power up and power down
	bcmpmu_audio->bcmpmu->read_dev(bcmpmu_audio->bcmpmu,PMU_REG_IHFPOP_PUP,&reg.val,PMU_BITMASK_ALL);
	reg.val |= BCMPMU_IHFPOP_AUTOSEQ;  //IHFPOP
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
	struct bcmpmu *bcmpmu = pdev->dev.platform_data;

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

static int __exit bcmpmu_audio_drv_exit(void)
{
	platform_driver_unregister(&bcmpmu_audio_driver);
	return 0;
}
module_exit(bcmpmu_audio_drv_exit);

MODULE_DESCRIPTION("BCM PMIC Audio Driver");
MODULE_LICENSE("GPL");
