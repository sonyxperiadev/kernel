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

enum {
	PKEY_TIMER_T1,
	PKEY_TIMER_T2,
	PKEY_TIMER_T3,
};

static struct bcmpmu_ponkey *bcmpmu_pkey;

u32 bcmpmu_get_ponkey_state(void)
{
	if (bcmpmu_pkey)
		return bcmpmu_pkey->ponkey_state;
	return 0;
}
EXPORT_SYMBOL(bcmpmu_get_ponkey_state);

struct simulate_ponkey {
	int dummy;
};

#define __param_check_simulate_ponkey(name, p, type) \
	static inline struct type *__check_##name(void) { return (p); }

#define param_check_simulate_ponkey(name, p) \
	__param_check_simulate_ponkey(name, p, simulate_ponkey)

static int param_set_simulate_ponkey(const char *val,
			const struct kernel_param *kp);
static int param_get_simulate_ponkey(char *buffer,
			const struct kernel_param *kp);

static struct kernel_param_ops param_ops_simulate_ponkey = {
	.get = param_get_simulate_ponkey,
	.set = param_set_simulate_ponkey,
};

static struct simulate_ponkey simulate_ponkey;
module_param_named(simulate_ponkey, simulate_ponkey, simulate_ponkey,
				S_IRUGO | S_IWUSR | S_IWGRP);

static int param_set_simulate_ponkey(const char *val,
		const struct kernel_param *kp)
{
	int trig;
	int ret = -1;
	if (!val)
		return -EINVAL;
	/* coverity[secure_coding] */
	ret = sscanf(val, "%d", &trig);
	pr_info("%s, trig:%d\n", __func__, trig);

	if (bcmpmu_pkey) {
		if (trig)
			bcmpmu_pkey->ponkey_state = 1;
		else
			bcmpmu_pkey->ponkey_state = 0;
		pr_info("ponkeystate:%d", bcmpmu_pkey->ponkey_state);
		input_report_key(bcmpmu_pkey->idev,
			KEY_POWER, bcmpmu_pkey->ponkey_state);
		input_sync(bcmpmu_pkey->idev);
	} else
		pr_info("Ponkey ptr is NULL\n");

	return 0;
}

static int param_get_simulate_ponkey(char *buffer,
		const struct kernel_param *kp)
{
	if (!buffer)
		return -EINVAL;
	if (bcmpmu_pkey)
		pr_info("Curr state: %u\n", bcmpmu_pkey->ponkey_state);
	else
		pr_info("Ponkey ptr is NULL\n");
	return 0;
}

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
	pr_info("%s: ponkey_state - %d\n", __func__,
		ponkey->ponkey_state);
	input_report_key(ponkey->idev,
		KEY_POWER, ponkey->ponkey_state);
	input_sync(ponkey->idev);
}

static int __ponkey_init_timer_func(struct bcmpmu59xxx *bcmpmu, int timer,
		struct pkey_timer_act *t)
{
	u32 reg = 0;
	u8 val = 0;
	int ret = 0;
	u32 action;

	switch (timer) {
	case PKEY_TIMER_T1:
		reg = PMU_REG_PONKEYCTRL2;
		break;
	case PKEY_TIMER_T2:
		reg = PMU_REG_PONKEYCTRL3;
		break;
	case PKEY_TIMER_T3:
		reg = PMU_REG_PONKEYCTRL8;
		break;
	default:
		BUG();
	}

	ret = bcmpmu->read_dev(bcmpmu, reg, &val);
	val &= ~PONKEY_TX_ACTION_MASK;
	if (!t) {
		val |= PKEY_ACTION_NOP << PONKEY_TX_ACTION_SHIFT;
		action = PKEY_ACTION_NOP;
	} else if (t->action <= PKEY_ACTION_NOP) {
		action = t->action;
		val |= action << PONKEY_TX_ACTION_SHIFT;
		val &= ~(PONKEY_TX_DLY_MASK | PONKEY_TX_DEB_MASK);
		val |= (t->timer_dly << PONKEY_TX_DLY_SHIFT) &
				PONKEY_TX_DLY_MASK;
		val |= (t->timer_deb << PONKEY_TX_DEB_SHIFT) &
				PONKEY_TX_DEB_MASK;
	} else
		return -EINVAL;
	ret |= bcmpmu->write_dev(bcmpmu, reg, val);

	switch (action) {
	case PKEY_ACTION_SHUTDOWN:
		if (bcmpmu->read_dev(bcmpmu, PMU_REG_PONKEYCTRL4, &val))
			return -EINVAL;
		val &= ~PONKEYCTRL4_KEY_PAD_LOCK_MASK;
		if (bcmpmu->write_dev(bcmpmu, PMU_REG_PONKEYCTRL4, val))
			return -EINVAL;
		break;

	case PKEY_ACTION_RESTART:
		if (bcmpmu->read_dev(bcmpmu, PMU_REG_PONKEYCTRL4, &val))
			return -EINVAL;
		val |= PONKEYCTRL4_POK_RESTART_EN_MASK;
		if (bcmpmu->write_dev(bcmpmu, PMU_REG_PONKEYCTRL4, val))
			return -EINVAL;
		break;

	case PKEY_ACTION_SMART_RESET:
		if (bcmpmu->read_dev(bcmpmu, PMU_REG_PONKEYCTRL6, &val))
			return -EINVAL;
		val |= PONKEY_SMART_RST_EN_MASK;
		if (t->flags & PKEY_SMART_RST_PWR_EN)
			val |= PONKEY_SMART_RST_PWR_EN_MASK;
		val &= ~PONKEY_SMART_RST_DLY_MASK;
		val |= (t->ctrl_params << PONKEY_SMART_RST_DLY_SHIFT) &
				PONKEY_SMART_RST_DLY_MASK;
		if (bcmpmu->write_dev(bcmpmu, PMU_REG_PONKEYCTRL6, val))
			return -EINVAL;
		break;

	default:
		break;
	}
	return ret;
}

static int __devinit bcmpmu59xxx_ponkey_probe(struct platform_device *pdev)
{
	struct bcmpmu59xxx *bcmpmu = dev_get_drvdata(pdev->dev.parent);
	struct bcmpmu59xxx_pkey_pdata *pkey;
	struct bcmpmu_ponkey *ponkey;
	int error;
	u8 val;

	pkey = (struct bcmpmu59xxx_pkey_pdata *)pdev->dev.platform_data;

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
	/*Disable all smart timer features by default
	__ponkey_init_timer_func function will enable it as needed*/
	if (bcmpmu->read_dev(bcmpmu, PMU_REG_PONKEYCTRL4, &val))
		return -EINVAL;
	val &= ~(PONKEYCTRL4_POK_RESTART_EN_MASK |
				PONKEYCTRL4_POK_WAKUP_DEB_MASK);
	val |= PONKEYCTRL4_KEY_PAD_LOCK_MASK;
	val |= (pkey->wakeup_deb << PONKEYCTRL4_POK_WAKUP_DEB_SHIFT) &
			PONKEYCTRL4_POK_WAKUP_DEB_MASK;
	if (bcmpmu->write_dev(bcmpmu, PMU_REG_PONKEYCTRL4, val))
		return -EINVAL;
	/*Clear smart reset feature bits in PMU_REG_PONKEYCTRL6*/
	pr_info("%s: smart rest status: %x\n", __func__, val);
	if (bcmpmu->write_dev(bcmpmu, PMU_REG_PONKEYCTRL6, 0))
		return -EINVAL;

	val = (pkey->press_deb << PONKEYCTRL1_PRESS_DEB_SHIFT) &
			PONKEYCTRL1_PRESS_DEB_MASK;
	val |= (pkey->release_deb << PONKEYCTRL1_RELEASE_DEB_SHIFT) &
			PONKEYCTRL1_RELEASE_DEB_MASK;
	if (bcmpmu->write_dev(bcmpmu, PMU_REG_PONKEYCTRL1, val))
		return -EINVAL;

	if (__ponkey_init_timer_func(bcmpmu, PKEY_TIMER_T1, pkey->t1))
		return -EINVAL;
	if (__ponkey_init_timer_func(bcmpmu, PKEY_TIMER_T2, pkey->t2))
		return -EINVAL;
	if (__ponkey_init_timer_func(bcmpmu, PKEY_TIMER_T3, pkey->t3))
		return -EINVAL;


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
