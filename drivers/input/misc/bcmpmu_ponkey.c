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

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/wait.h>
#include <linux/input.h>

#include <linux/mfd/bcmpmu.h>
#include <linux/broadcom/bcmpmu-ponkey.h>

struct bcmpmu_ponkey {
	struct input_dev *idev;
	struct bcmpmu *bcmpmu;
	u32 ponkey_state;	/*0: Released, 1 : Pressed */
};

static struct bcmpmu_ponkey *bcmpmu_pkey;

u32 bcmpmu_get_ponkey_state(void)
{
	if (bcmpmu_pkey)
		return bcmpmu_pkey->ponkey_state;
	return 0;
}

EXPORT_SYMBOL(bcmpmu_get_ponkey_state);

static void bcmpmu_ponkey_isr(enum bcmpmu_irq irq, void *data)
{
	struct bcmpmu_ponkey *ponkey = data;

	switch (irq) {
	case PMU_IRQ_PONKEYB_F:
		ponkey->ponkey_state = 1;
		break;

	case PMU_IRQ_PONKEYB_R:
		ponkey->ponkey_state = 0;
		break;

	default:
		pr_info("Invalid IRQ %d\n", irq);
		return;
	}

	input_report_key(ponkey->idev, KEY_POWER, ponkey->ponkey_state);
	input_sync(ponkey->idev);
}

static int __devinit bcmpmu_ponkey_probe(struct platform_device *pdev)
{
	struct bcmpmu *bcmpmu = pdev->dev.platform_data;
	struct bcmpmu_ponkey *ponkey;
	int error;
	struct bcmpmu_platform_data *pdata = bcmpmu->pdata;
	printk(KERN_INFO "bcmpmu_ponkey: ponkey_probe called\n");

	ponkey = kzalloc(sizeof(struct bcmpmu_ponkey), GFP_KERNEL);
	bcmpmu_pkey = ponkey;
	if (!ponkey) {
		pr_info("bcmpmu_ponkey:failed to alloc mem.\n");
		return -ENOMEM;
	}

	ponkey->idev = input_allocate_device();
	if (!ponkey->idev) {
		pr_info("bcmpmu_ponkey:failed to allocate input dev.\n");
		error = -ENOMEM;
		goto out_input;
	}

	ponkey->ponkey_state = 0;
	ponkey->bcmpmu = bcmpmu;
	bcmpmu->ponkeyinfo = (void *)ponkey;

	ponkey->idev->name = "bcmpmu_on";
	ponkey->idev->phys = "bcmpmu_on/input0";
	ponkey->idev->dev.parent = &pdev->dev;
	ponkey->idev->evbit[0] = BIT_MASK(EV_KEY);
	ponkey->idev->keybit[BIT_WORD(KEY_POWER)] = BIT_MASK(KEY_POWER);

	/* init shutdown/hard reset/restart details */
	if (pdata->hard_reset_en == 0 || pdata->hard_reset_en == 1) {
		pdata->hard_reset_en <<=
		    bcmpmu->regmap[PMU_REG_POK_RSTPIN_ONLY].shift;
		bcmpmu->write_dev(bcmpmu, PMU_REG_POK_RSTPIN_ONLY,
				  pdata->hard_reset_en,
				  bcmpmu->regmap[PMU_REG_POK_RSTPIN_ONLY].mask);
	}
	if (pdata->restart_en == 0 || pdata->restart_en == 1) {
		pdata->restart_en <<=
		    bcmpmu->regmap[PMU_REG_PONKEY_RESTART_EN].shift;
		bcmpmu->write_dev(bcmpmu, PMU_REG_PONKEY_RESTART_EN,
				  pdata->restart_en,
				  bcmpmu->regmap[PMU_REG_PONKEY_RESTART_EN].
				  mask);
	}
	if (pdata->pok_hold_deb > 0) {
		pdata->pok_hold_deb <<=
		    bcmpmu->regmap[PMU_REG_PONKEYOFFHOLD_DEB].shift;
		pdata->pok_hold_deb &=
		    bcmpmu->regmap[PMU_REG_PONKEYOFFHOLD_DEB].mask;
		bcmpmu->write_dev(bcmpmu, PMU_REG_PONKEYOFFHOLD_DEB,
				  pdata->pok_hold_deb,
				  bcmpmu->regmap[PMU_REG_PONKEYOFFHOLD_DEB].
				  mask);
	}
	if (pdata->pok_shtdwn_dly > 0) {
		pdata->pok_shtdwn_dly <<=
		    bcmpmu->regmap[PMU_REG_PONKEY_SHUTDOWN_DLY].shift;
		pdata->pok_shtdwn_dly &=
		    bcmpmu->regmap[PMU_REG_PONKEY_SHUTDOWN_DLY].mask;
		bcmpmu->write_dev(bcmpmu, PMU_REG_PONKEY_SHUTDOWN_DLY,
				  pdata->pok_shtdwn_dly,
				  bcmpmu->regmap[PMU_REG_PONKEY_SHUTDOWN_DLY].
				  mask);
	}
	if (pdata->pok_restart_dly > 0) {
		pdata->pok_restart_dly <<=
		    bcmpmu->regmap[PMU_REG_PONKEY_RESTART_DLY].shift;
		pdata->pok_restart_dly &=
		    bcmpmu->regmap[PMU_REG_PONKEY_RESTART_DLY].mask;
		bcmpmu->write_dev(bcmpmu, PMU_REG_PONKEY_RESTART_DLY,
				  pdata->pok_restart_dly,
				  bcmpmu->regmap[PMU_REG_PONKEY_RESTART_DLY].
				  mask);
	}
	if (pdata->pok_restart_deb > 0) {
		pdata->pok_restart_deb <<=
		    bcmpmu->regmap[PMU_REG_PONKEY_RESTART_DEB].shift;
		pdata->pok_restart_deb &=
		    bcmpmu->regmap[PMU_REG_PONKEY_RESTART_DEB].mask;
		bcmpmu->write_dev(bcmpmu, PMU_REG_PONKEY_RESTART_DEB,
				  pdata->pok_restart_deb,
				  bcmpmu->regmap[PMU_REG_PONKEY_RESTART_DEB].
				  mask);
	}
	if (pdata->pok_turn_on_deb >= 0) {
		pdata->pok_turn_on_deb <<=
		    bcmpmu->regmap[PMU_REG_PONKEY_ONHOLD_DEB].shift;
		pdata->pok_turn_on_deb &=
		    bcmpmu->regmap[PMU_REG_PONKEY_ONHOLD_DEB].mask;
		bcmpmu->write_dev(bcmpmu, PMU_REG_PONKEY_ONHOLD_DEB,
				  pdata->pok_turn_on_deb,
				  bcmpmu->regmap[PMU_REG_PONKEY_ONHOLD_DEB].
				  mask);
	}
	/* set KEY_PAD_LOCK */
	if (pdata->pok_lock == 0 || pdata->pok_lock == 1) {
		pdata->pok_lock <<= bcmpmu->regmap[PMU_REG_KEY_PAD_LOCK].shift;
		bcmpmu->write_dev(bcmpmu, PMU_REG_KEY_PAD_LOCK, pdata->pok_lock,
				  bcmpmu->regmap[PMU_REG_KEY_PAD_LOCK].mask);
	}

	/* Request PRESSED and RELEASED interrupts.
	 */
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_PONKEYB_F, bcmpmu_ponkey_isr,
			     ponkey);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_PONKEYB_R, bcmpmu_ponkey_isr,
			     ponkey);

	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_PONKEYB_F);
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_PONKEYB_R);

	error = input_register_device(ponkey->idev);
	if (error) {
		dev_err(bcmpmu->dev, "Can't register input device: %d\n",
			error);
		goto out;
	}

	return 0;

out:
	input_free_device(ponkey->idev);
out_input:
	kfree(ponkey);
	return error;
}

static int __devexit bcmpmu_ponkey_remove(struct platform_device *pdev)
{
	struct bcmpmu *bcmpmu = pdev->dev.platform_data;
	struct bcmpmu_ponkey *ponkey = bcmpmu->ponkeyinfo;

	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_PONKEYB_F);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_PONKEYB_R);

	input_unregister_device(ponkey->idev);
	kfree(ponkey);

	return 0;
}

static struct platform_driver bcmpmu_ponkey_driver = {
	.driver = {
		   .name = "bcmpmu_ponkey",
		   .owner = THIS_MODULE,
		   },
	.probe = bcmpmu_ponkey_probe,
	.remove = __devexit_p(bcmpmu_ponkey_remove),
};

static int __init bcmpmu_ponkey_init(void)
{
	return platform_driver_register(&bcmpmu_ponkey_driver);
}

module_init(bcmpmu_ponkey_init);

static void __exit bcmpmu_ponkey_exit(void)
{
	platform_driver_unregister(&bcmpmu_ponkey_driver);
}

module_exit(bcmpmu_ponkey_exit);

MODULE_DESCRIPTION("BCMPMU PowOnKey driver");
MODULE_LICENSE("GPL");
