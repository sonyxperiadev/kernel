/*******************************************************************************
*
* Copyright 2012  Broadcom Corporation
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed
*  to you under the terms of the GNU General Public License version 2
*  (the GPL), available at
*       http://www.broadcom.com/licenses/GPLv2.php
*
*  with the following added to such license:
*
*  As a special exception, the copyright holders of this software give you
*  permission to link this software with independent modules, and to copy
*  and distribute the resulting executable under terms of your choice,
*  provided that you also meet, for each linked independent module, the
*  terms and conditions of the license of that module. An independent
*  module is a module which is not derived from this software. The special
*  exception does not apply to any modifications of the software.
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any  way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*******************************************************************************/

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/wait.h>
#include <linux/input.h>
#include <linux/mfd/bcmpmu59xxx.h>
#include <linux/mfd/bcmpmu59xxx_reg.h>

struct bcmpmu_ponkey {
	struct input_dev *idev;
	u32 ponkey_state;/*0: Released, 1 : Pressed */
};

static struct bcmpmu_ponkey *bcmpmu_pkey;

u32 bcmpmu_get_ponkey_state(void)
{
	if (bcmpmu_pkey)
		return bcmpmu_pkey->ponkey_state;
	return 0;
}
EXPORT_SYMBOL(bcmpmu_get_ponkey_state);

static void bcmpmu_ponkey_isr(u32 irq, void *data)
{
	struct bcmpmu_ponkey *ponkey = data;

	switch (irq) {
	case PMU_IRQ_POK_PRESSED:
		ponkey->ponkey_state = 1;
		break;

	case PMU_IRQ_POK_RELEASED:
		ponkey->ponkey_state = 0;
		break;

	default:
		pr_info("Invalid IRQ %d\n", irq);
		return;
	}

	input_report_key(ponkey->idev,
		KEY_POWER, ponkey->ponkey_state);
	input_sync(ponkey->idev);
}

static int __devinit bcmpmu59xxx_ponkey_probe(struct platform_device *pdev)
{
	struct bcmpmu59xxx *bcmpmu = dev_get_drvdata(pdev->dev.parent);
	struct bcmpmu59xxx_pok_pdata *pkey;
	struct bcmpmu_ponkey *ponkey;
	int error;
	u8 val;

	pkey = (struct bcmpmu59xxx_pok_pdata *)pdev->dev.platform_data;

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
	bcmpmu->ponkeyinfo = (void *)ponkey;
	ponkey->idev->name = "bcmpmu_on";
	ponkey->idev->phys = "bcmpmu_on/input0";
	ponkey->idev->dev.parent = &pdev->dev;
	ponkey->idev->evbit[0] = BIT_MASK(EV_KEY);
	ponkey->idev->keybit[BIT_WORD(KEY_POWER)] = BIT_MASK(KEY_POWER);
	/* init shutdown/hard reset/restart details */
/*
 PMU_REG_GPIOCTRL2
	if (pkey->hard_reset_en == 0 || pkey->hard_reset_en == 1) {
	}
*/

	if (pkey->restart_en == 0 || pkey->restart_en == 1) {
		bcmpmu->read_dev(bcmpmu, PMU_REG_PONKEYCTRL4, &val);
		val &= ~PONKEYCTRL4_POK_RESTART_EN_MASK;
		val |= (pkey->restart_en << PONKEYCTRL4_POK_RESTART_EN_SHIFT);
		bcmpmu->write_dev(bcmpmu, PMU_REG_PONKEYCTRL4, val);
	}
	if (pkey->pok_hold_deb > 0) {
		bcmpmu->read_dev(bcmpmu, PMU_REG_PONKEYCTRL1, &val);
		val &= ~PONKEYCTRL1_PRESS_DEB_MASK;
		val |= (pkey->pok_hold_deb << PONKEYCTRL1_PRESS_DEB_SHIFT);
		bcmpmu->write_dev(bcmpmu, PMU_REG_PONKEYCTRL1, val);
	}
	if (pkey->pok_shtdwn_dly > 0) {
		bcmpmu->read_dev(bcmpmu, PMU_REG_HOSTCTRL4, &val);
		val &= ~HOSTCTRL4_BB_SHDWN_DEB_MASK;
		val |= (pkey->pok_shtdwn_dly << HOSTCTRL4_BB_SHDWN_DEB_SHIFT);
		bcmpmu->write_dev(bcmpmu, PMU_REG_HOSTCTRL4, val);
	}
	if (pkey->pok_restart_dly > 0) {
		bcmpmu->read_dev(bcmpmu, PMU_REG_PONKEYCTRL7, &val);
		val &= ~HOSTCTRL7_SW_RESTART_DLY_MASK;
		val |= (pkey->pok_restart_dly <<
			HOSTCTRL7_SW_RESTART_DLY_SHIFT);
		bcmpmu->write_dev(bcmpmu, PMU_REG_PONKEYCTRL7, val);
	}
	if (pkey->pok_restart_deb > 0) {
		bcmpmu->read_dev(bcmpmu, PMU_REG_PONKEYCTRL4, &val);
		val &= (~PONKEYCTRL4_POK_WAKUP_DEB_MASK);
		val |= (pkey->pok_restart_deb <<
				   PONKEYCTRL4_POK_WAKUP_DEB_SHIFT);
		bcmpmu->write_dev(bcmpmu, PMU_REG_PONKEYCTRL4, val);
	}
/*
	if (pkey->pok_turn_on_deb >= 0) {
		bcmpmu->read_dev(bcmpmu, PMU_REG_AUXCTRL, &val);
		val &= (~PONKEY_ONHOLD_DEB_SHIFT);
		val |= (pkey->pok_turn_on_deb << PONKEY_ONHOLD_DEB_SHIFT);
		bcmpmu->write_dev(bcmpmu, PMU_REG_AUXCTRL, val);
	}
*/
	/* set KEY_PAD_LOCK */
	if (pkey->pok_lock == 0 || pkey->pok_lock == 1) {
		bcmpmu->read_dev(bcmpmu, PMU_REG_PONKEYCTRL4, &val);
		val &= ~PONKEYCTRL4_KEY_PAD_LOCK_MASK;
		val |= (pkey->pok_lock << PONKEYCTRL4_KEY_PAD_LOCK_SHIFT);
		bcmpmu->write_dev(bcmpmu, PMU_REG_PONKEYCTRL4, val);
	}
	/* Request PRESSED and RELEASED interrupts.
	 */
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_POK_PRESSED, bcmpmu_ponkey_isr,
			     ponkey);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_POK_RELEASED, bcmpmu_ponkey_isr,
			     ponkey);

	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_POK_PRESSED);
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_POK_RELEASED);

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

static int __devexit bcmpmu59xxx_ponkey_remove(struct platform_device *pdev)
{
	struct bcmpmu59xxx *bcmpmu = dev_get_drvdata(pdev->dev.parent);
	struct bcmpmu_ponkey *ponkey = bcmpmu->ponkeyinfo;

	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_POK_PRESSED);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_POK_RELEASED);
	input_unregister_device(ponkey->idev);
	kfree(ponkey);
	return 0;
}

static struct platform_driver __refdata bcmpmu59xxx_ponkey = {
	.driver = {
		   .name = "bcmpmu59xxx-ponkey",
		   .owner = THIS_MODULE,
		   },
	.probe = bcmpmu59xxx_ponkey_probe,
	.remove =  __devexit_p(bcmpmu59xxx_ponkey_remove),
};

static int __init bcmpmu59xxx_ponkey_init(void)
{
	return platform_driver_register(&bcmpmu59xxx_ponkey);
}
module_init(bcmpmu59xxx_ponkey_init);

static void __exit bcmpmu59xxx_ponkey_exit(void)
{
	platform_driver_unregister(&bcmpmu59xxx_ponkey);
}
module_exit(bcmpmu59xxx_ponkey_exit);

MODULE_DESCRIPTION("BCMPMU PowOnKey driver");
MODULE_LICENSE("GPL");
