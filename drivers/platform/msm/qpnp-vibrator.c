/* Copyright (c) 2012, The Linux Foundation. All rights reserved.
 * Copyright (C) 2013 Sony Mobile Communications AB.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/errno.h>
#include <linux/hrtimer.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/slab.h>
#include <linux/spmi.h>

#include <linux/qpnp/vibrator.h>
#include "../staging/android/timed_output.h"

#define QPNP_VIB_VTG_CTL(base)		(base + 0x41)
#define QPNP_VIB_EN_CTL(base)		(base + 0x46)

#define QPNP_VIB_MAX_LEVEL		31
#define QPNP_VIB_MIN_LEVEL		12

#define QPNP_VIB_UV_PER_MV		100

#define QPNP_VIB_DEFAULT_TIMEOUT	15000
#define QPNP_VIB_DEFAULT_VTG_LVL	3100

#define QPNP_VIB_EN			BIT(7)
#define QPNP_VIB_VTG_SET_MASK		0x1F
#define QPNP_VIB_LOGIC_SHIFT		4

struct qpnp_vib {
	struct spmi_device *spmi;
	struct hrtimer vib_timer;
	struct timed_output_dev timed_dev;
	struct work_struct work;
	struct device sysfs_dev;

	u8  reg_vtg_ctl;
	u8  reg_en_ctl;
	u16 base;
	int state;
	int vtg_level;
	int timeout;
	spinlock_t lock;
};

static struct qpnp_vib *vib_dev;

static struct of_device_id spmi_match_table[];

static int qpnp_vib_read_u8(struct qpnp_vib *vib, u8 *data, u16 reg)
{
	int rc;

	rc = spmi_ext_register_readl(vib->spmi->ctrl, vib->spmi->sid,
							reg, data, 1);
	if (rc < 0)
		dev_err(&vib->spmi->dev,
			"Error reading address: %X - ret %X\n", reg, rc);

	return rc;
}

static int qpnp_vib_write_u8(struct qpnp_vib *vib, u8 *data, u16 reg)
{
	int rc;

	rc = spmi_ext_register_writel(vib->spmi->ctrl, vib->spmi->sid,
							reg, data, 1);
	if (rc < 0)
		dev_err(&vib->spmi->dev,
			"Error writing address: %X - ret %X\n", reg, rc);

	return rc;
}

int qpnp_vibrator_config(struct qpnp_vib_config *vib_cfg)
{
	u8 reg = 0;
	int rc = -EINVAL, level;

	if (!vib_dev) {
		pr_err("%s: vib_dev is NULL\n", __func__);
		rc = -ENODEV;
		goto gen_err;
	}

	level = vib_cfg->drive_mV / QPNP_VIB_UV_PER_MV;
	if (level) {
		if ((level < QPNP_VIB_MIN_LEVEL) ||
				(level > QPNP_VIB_MAX_LEVEL)) {
			dev_err(&vib_dev->spmi->dev, "Invalid voltage level\n");
			goto gen_err;
		}
	} else {
		dev_err(&vib_dev->spmi->dev, "Voltage level not specified\n");
		goto gen_err;
	}

	/* Configure the VTG CTL regiser */
	reg = vib_dev->reg_vtg_ctl;
	reg &= ~QPNP_VIB_VTG_SET_MASK;
	reg |= (level & QPNP_VIB_VTG_SET_MASK);
	rc = qpnp_vib_write_u8(vib_dev, &reg, QPNP_VIB_VTG_CTL(vib_dev->base));
	if (rc)
		goto gen_err;
	vib_dev->reg_vtg_ctl = reg;

	/* Configure the VIB ENABLE regiser */
	reg = vib_dev->reg_en_ctl;
	reg |= (!!vib_cfg->active_low) << QPNP_VIB_LOGIC_SHIFT;
	if (vib_cfg->enable_mode == QPNP_VIB_MANUAL)
		reg |= QPNP_VIB_EN;
	else
		reg |= BIT(vib_cfg->enable_mode - 1);
	rc = qpnp_vib_write_u8(vib_dev, &reg, QPNP_VIB_EN_CTL(vib_dev->base));
	if (rc < 0)
		goto gen_err;
	vib_dev->reg_en_ctl = reg;

gen_err:
	return rc;
}
EXPORT_SYMBOL(qpnp_vibrator_config);

static int qpnp_vib_set(struct qpnp_vib *vib, int on)
{
	int rc;
	u8 val;

	if (on) {
		val = vib->reg_vtg_ctl;
		val &= ~QPNP_VIB_VTG_SET_MASK;
		val |= (vib->vtg_level & QPNP_VIB_VTG_SET_MASK);
		rc = qpnp_vib_write_u8(vib, &val, QPNP_VIB_VTG_CTL(vib->base));
		if (rc < 0)
			goto gen_err;
		vib->reg_vtg_ctl = val;
		val = vib->reg_en_ctl;
		val |= QPNP_VIB_EN;
		rc = qpnp_vib_write_u8(vib, &val, QPNP_VIB_EN_CTL(vib->base));
		if (rc < 0)
			goto gen_err;
		vib->reg_en_ctl = val;
	} else {
		val = vib->reg_en_ctl;
		val &= ~QPNP_VIB_EN;
		rc = qpnp_vib_write_u8(vib, &val, QPNP_VIB_EN_CTL(vib->base));
		if (rc < 0)
			goto gen_err;
		vib->reg_en_ctl = val;
	}

gen_err:
	return rc;
}

static void qpnp_vib_enable(struct timed_output_dev *dev, int value)
{
	struct qpnp_vib *vib = container_of(dev, struct qpnp_vib,
					 timed_dev);
	unsigned long flags;

	while (1) {
		spin_lock_irqsave(&vib->lock, flags);
		if (hrtimer_try_to_cancel(&vib->vib_timer) >= 0)
			break;
		spin_unlock_irqrestore(&vib->lock, flags);
		cpu_relax();
	}

	if (!value)
		vib->state = 0;
	else {
		vib->state = 1;
		hrtimer_start(&vib->vib_timer,
			      ktime_set(value / MSEC_PER_SEC,
					(value % MSEC_PER_SEC) *
					NSEC_PER_MSEC),
					HRTIMER_MODE_REL);
	}
	spin_unlock_irqrestore(&vib->lock, flags);
	schedule_work(&vib->work);
}

static void qpnp_vib_update(struct work_struct *work)
{
	struct qpnp_vib *vib = container_of(work, struct qpnp_vib,
					 work);
	qpnp_vib_set(vib, vib->state);
}

static ssize_t qpnp_vib_attrs_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct qpnp_vib *vib = container_of(dev, struct qpnp_vib, sysfs_dev);

	return snprintf(buf, PAGE_SIZE, "%d\n",
			vib->vtg_level * QPNP_VIB_UV_PER_MV);
}

static ssize_t qpnp_vib_attrs_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	int tmp_vtg_level;
	unsigned long value;
	int ret = -EINVAL;
	struct qpnp_vib *vib = container_of(dev, struct qpnp_vib, sysfs_dev);

	if (kstrtoul(buf, 0, &value)) {
		dev_err(dev, "%s: Invalid value\n", __func__);
		goto err_out;
	}

	value /= QPNP_VIB_UV_PER_MV;

	if (value) {
		if (value < QPNP_VIB_MIN_LEVEL ||
				value > QPNP_VIB_MAX_LEVEL) {
			dev_err(dev, "Invalid voltage level\n");
			goto err_out;
		}
	} else {
		dev_err(dev, "Voltage level not specified\n");
		goto err_out;
	}
	tmp_vtg_level = vib->vtg_level;
	vib->vtg_level = value;

	ret = qpnp_vib_set(vib, vib->state);
	if (ret) {
		dev_err(dev, "%s: qpnp_vib_set failed %d\n", __func__, ret);
		vib->vtg_level = tmp_vtg_level;
		goto err_out;
	}

	return count;

err_out:
	return ret;
}

static struct device_attribute qpnp_vib_attr =
	__ATTR(qpnp_vib, S_IWUSR | S_IRUGO,
	qpnp_vib_attrs_show, qpnp_vib_attrs_store);

static int qpnp_vib_get_time(struct timed_output_dev *dev)
{
	struct qpnp_vib *vib = container_of(dev, struct qpnp_vib,
							 timed_dev);

	if (hrtimer_active(&vib->vib_timer)) {
		ktime_t r = hrtimer_get_remaining(&vib->vib_timer);
		return (int)ktime_to_us(r);
	}
	return 0;
}

static enum hrtimer_restart qpnp_vib_timer_func(struct hrtimer *timer)
{
	struct qpnp_vib *vib = container_of(timer, struct qpnp_vib,
							 vib_timer);

	vib->state = 0;
	schedule_work(&vib->work);

	return HRTIMER_NORESTART;
}

#ifdef CONFIG_PM
static int qpnp_vibrator_suspend(struct device *dev)
{
	struct qpnp_vib *vib = dev_get_drvdata(dev);

	hrtimer_cancel(&vib->vib_timer);
	cancel_work_sync(&vib->work);
	/* turn-off vibrator */
	qpnp_vib_set(vib, 0);

	return 0;
}
#endif

static SIMPLE_DEV_PM_OPS(qpnp_vibrator_pm_ops, qpnp_vibrator_suspend, NULL);

static int __devinit qpnp_vibrator_probe(struct spmi_device *spmi)
{
	struct qpnp_vib *vib;
	const __be32 *temp_dt;
	struct resource *vib_resource;
	int rc = -ENOMEM;
	u8 val;

	vib = devm_kzalloc(&spmi->dev, sizeof(*vib), GFP_KERNEL);
	if (!vib)
		goto gen_err;

	vib->spmi = spmi;

	temp_dt = of_get_property(spmi->dev.of_node,
			"qcom,qpnp-vib-timeout-ms", NULL);
	if (temp_dt)
		vib->timeout = be32_to_cpu(*temp_dt);
	else
		vib->timeout = QPNP_VIB_DEFAULT_TIMEOUT;

	temp_dt = of_get_property(spmi->dev.of_node,
			"qcom,qpnp-vib-vtg-level-mV", NULL);
	if (temp_dt)
		vib->vtg_level = be32_to_cpu(*temp_dt);
	else
		vib->vtg_level = QPNP_VIB_DEFAULT_VTG_LVL;

	vib->vtg_level /= QPNP_VIB_UV_PER_MV;

	vib_resource = spmi_get_resource(spmi, 0, IORESOURCE_MEM, 0);
	if (!vib_resource) {
		dev_err(&spmi->dev, "Unable to get vibrator base address\n");
		rc = -EINVAL;
		goto gen_err;
	}
	vib->base = vib_resource->start;

	/* save the control registers values */
	rc = qpnp_vib_read_u8(vib, &val, QPNP_VIB_VTG_CTL(vib->base));
	if (rc < 0)
		goto gen_err;
	vib->reg_vtg_ctl = val;

	rc = qpnp_vib_read_u8(vib, &val, QPNP_VIB_EN_CTL(vib->base));
	if (rc < 0)
		goto gen_err;
	vib->reg_en_ctl = val;

	spin_lock_init(&vib->lock);
	INIT_WORK(&vib->work, qpnp_vib_update);

	hrtimer_init(&vib->vib_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	vib->vib_timer.function = qpnp_vib_timer_func;

	vib->timed_dev.name = "vibrator";
	vib->timed_dev.get_time = qpnp_vib_get_time;
	vib->timed_dev.enable = qpnp_vib_enable;

	rc = timed_output_dev_register(&vib->timed_dev);
	if (rc < 0)
		goto gen_err;

	vib->sysfs_dev.init_name = "qpnp_vib";

	rc = device_register(&vib->sysfs_dev);
	if (rc < 0) {
		dev_err(&spmi->dev, "%s: device_register failed %d\n",
			__func__, rc);
		goto error_dev_register;
	}
	rc = device_create_file(&vib->sysfs_dev, &qpnp_vib_attr);
	if (rc < 0) {
		dev_err(&spmi->dev, "%s: device_create_file failed %d\n",
			__func__, rc);
		goto error_device_create_file;
	}

	dev_set_drvdata(&spmi->dev, vib);

	vib_dev = vib;

	return rc;

error_device_create_file:
	device_unregister(&vib->sysfs_dev);
error_dev_register:
	timed_output_dev_unregister(&vib->timed_dev);
gen_err:
	return rc;
}

static int  __devexit qpnp_vibrator_remove(struct spmi_device *spmi)
{
	struct qpnp_vib *vib = dev_get_drvdata(&spmi->dev);

	cancel_work_sync(&vib->work);
	hrtimer_cancel(&vib->vib_timer);
	timed_output_dev_unregister(&vib->timed_dev);
	device_remove_file(&vib->sysfs_dev, &qpnp_vib_attr);
	device_unregister(&vib->sysfs_dev);

	return 0;
}

static struct of_device_id spmi_match_table[] = {
	{	.compatible = "qcom,qpnp-vibrator",
	},
	{}
};

static struct spmi_driver qpnp_vibrator_driver = {
	.driver		= {
		.name	= "qcom,qpnp-vibrator",
		.of_match_table = spmi_match_table,
		.pm	= &qpnp_vibrator_pm_ops,
	},
	.probe		= qpnp_vibrator_probe,
	.remove		= __devexit_p(qpnp_vibrator_remove),
};

static int __init qpnp_vibrator_init(void)
{
	return spmi_driver_register(&qpnp_vibrator_driver);
}
module_init(qpnp_vibrator_init);

static void __exit qpnp_vibrator_exit(void)
{
	return spmi_driver_unregister(&qpnp_vibrator_driver);
}
module_exit(qpnp_vibrator_exit);

MODULE_DESCRIPTION("qpnp vibrator driver");
MODULE_LICENSE("GPL v2");
