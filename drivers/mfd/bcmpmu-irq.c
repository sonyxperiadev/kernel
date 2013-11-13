/*****************************************************************************
*  Copyright 2001 - 2008 Broadcom Corporation.  All rights reserved.
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2, available at
*  http://www.gnu.org/licenses/old-license/gpl-2.0.html (the "GPL").
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
*****************************************************************************/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/mutex.h>
#include <linux/moduleparam.h>
#include <mach/gpio.h>

#include <linux/mfd/bcmpmu.h>

#define BCMPMU_PRINT_ERROR (1U << 0)
#define BCMPMU_PRINT_INIT (1U << 1)
#define BCMPMU_PRINT_FLOW (1U << 2)
#define BCMPMU_PRINT_DATA (1U << 3)

static int debug_mask = BCMPMU_PRINT_ERROR | BCMPMU_PRINT_INIT;
module_param_named(debug_mask, debug_mask, int, S_IRUGO | S_IWUSR | S_IWGRP);
#define pr_irq(debug_level, args...) \
	do { \
		if (debug_mask & BCMPMU_PRINT_##debug_level) { \
			pr_info(args); \
		} \
	} while (0)

struct irq_cb {
	void (*callback) (enum bcmpmu_irq, void *);
	void *idata;
	bool int_en;
};

struct bcmpmu_irq_data {
	int irq;
	const struct bcmpmu_irq_map *irqmap;
	const struct bcmpmu_reg_map *irqregmap;
	int irqreg_size;
	unsigned int *irq_regs;
	struct bcmpmu *bcmpmu;
	struct irq_cb irq_cb[PMU_IRQ_MAX];
	struct mutex ilock;
	struct workqueue_struct *workq;
	struct work_struct work;
};

static irqreturn_t bcmpmu_isr(int irq, void *data)
{
	struct bcmpmu_irq_data *idata = data;
	if (!queue_work(idata->workq, &idata->work))
		pr_info("%s: The work is already queued\n", __func__);
	return IRQ_HANDLED;
}

static void bcmpmu_read_irq_regs(struct bcmpmu_irq_data *idata)
{
	int i;


	idata->bcmpmu->read_dev_bulk(idata->bcmpmu,
			idata->bcmpmu->regmap[PMU_REG_INT_START].map,
			idata->bcmpmu->regmap[PMU_REG_INT_START].addr,
			idata->irq_regs,
			idata->irqreg_size);
/*
	for (i = 0; i < idata->irqreg_size; i++) {
		idata->bcmpmu->read_dev_drct(idata->bcmpmu,
					     idata->bcmpmu->
					     regmap[PMU_REG_INT_START].map,
					     idata->bcmpmu->
					     regmap[PMU_REG_INT_START].addr + i,
					     &idata->irq_regs[i], 0xffffffff);
	}
*/
	if (BCMPMU_PRINT_DATA & debug_mask) {
		for (i = 0; i < idata->irqreg_size; i++) {
			pr_irq(DATA, "%s int=0x%X\n",
					__func__, idata->irq_regs[i]);
		}
	}
}

static bool bcmpmu_get_irq_index(struct bcmpmu_irq_data *idata,
				 enum bcmpmu_irq *index)
{
	static u8 reg_index = 0;
	int i;
	int j;

	for (i = reg_index; i < idata->irqreg_size; i++) {
		if (idata->irq_regs[i] != 0) {
			for (j = 0; j < PMU_IRQ_MAX; j++) {
				if ((idata->irqregmap[i].addr ==
				     idata->irqmap[j].int_addr)
				    && (idata->irqmap[j].bit_mask & idata->
					irq_regs[i])) {
					idata->irq_regs[i] &=
					    ~(idata->irqmap[j].bit_mask);
					goto int_found;
				}
			}
		}
	}
	*index = PMU_IRQ_MAX;
	reg_index = 0;
	return false;

      int_found:
	*index = j;
	reg_index = i;
	return true;
}

void bcmpmu_irq_handler(struct work_struct *work)
{
	bool ret;
	enum bcmpmu_irq index;
	struct bcmpmu_irq_data *idata;
	struct bcmpmu *bcmpmu;
	int irq, gpio_state;
	idata = container_of(work, struct bcmpmu_irq_data, work);
	bcmpmu = idata->bcmpmu;
	irq = bcmpmu->pdata->irq;
	do {
		ret = true;
		bcmpmu_read_irq_regs(idata);
		mutex_lock(&idata->ilock);
		while (ret) {
			ret = bcmpmu_get_irq_index(idata, &index);
			if (index < PMU_IRQ_MAX) {
				if ((idata->irq_cb[index].callback != NULL) &&
				    (idata->irq_cb[index].int_en != 0)) {
					idata->irq_cb[index].callback(index,
							idata->irq_cb[index].
							idata);
					pr_irq(FLOW, "%s index=%d, callback\n",
					       __func__, index);
				}
			}
		}
		mutex_unlock(&idata->ilock);
		gpio_state = gpio_get_value(irq_to_gpio(irq));
		pr_irq(FLOW, "%s: gpio_state=%d", __func__, gpio_state);
	} while (gpio_state == 0);
}

int bcmpmu_register_irq(struct bcmpmu *pmu, enum bcmpmu_irq irq,
			void (*callback) (enum bcmpmu_irq irq, void *),
			void *data)
{
	struct bcmpmu_irq_data *idata = (struct bcmpmu_irq_data *)pmu->irqinfo;
	int ret;

	if (irq >= PMU_IRQ_MAX)
		return -ENODEV;
	mutex_lock(&idata->ilock);
	if (idata->irq_cb[irq].callback == NULL) {
		idata->irq_cb[irq].callback = callback;
		idata->irq_cb[irq].idata = data;
		ret = 0;
	} else
		ret = -EINVAL;
	mutex_unlock(&idata->ilock);
	return ret;
}

int bcmpmu_unregister_irq(struct bcmpmu *pmu, enum bcmpmu_irq irq)
{
	struct bcmpmu_irq_data *idata = (struct bcmpmu_irq_data *)pmu->irqinfo;
	int ret;
	if (irq >= PMU_IRQ_MAX)
		return -ENODEV;
	mutex_lock(&idata->ilock);
	if (idata->irq_cb[irq].callback == NULL) {
		idata->irq_cb[irq].callback = NULL;
		idata->irq_cb[irq].idata = NULL;
		ret = 0;
	} else
		ret = -EINVAL;
	mutex_unlock(&idata->ilock);
	return ret;
}

int bcmpmu_mask_irq(struct bcmpmu *pmu, enum bcmpmu_irq irq)
{
	struct bcmpmu_irq_data *idata = (struct bcmpmu_irq_data *)pmu->irqinfo;
	struct bcmpmu_irq_map map;
	int ret = 0;
	if (irq >= PMU_IRQ_MAX)
		return -ENODEV;
	map = idata->irqmap[irq];
	if ((map.int_addr == 0) && (map.mask_addr == 0))
		return -ENODEV;

	ret =
	    pmu->write_dev_drct(pmu, map.map, map.mask_addr, map.bit_mask,
				map.bit_mask);
	if (ret == 0)
		idata->irq_cb[irq].int_en = 0;
	return ret;

}

int bcmpmu_unmask_irq(struct bcmpmu *pmu, enum bcmpmu_irq irq)
{
	struct bcmpmu_irq_data *idata = (struct bcmpmu_irq_data *)pmu->irqinfo;
	struct bcmpmu_irq_map map;
	int ret = 0;
	if (irq >= PMU_IRQ_MAX)
		return -ENODEV;
	map = idata->irqmap[irq];
	if ((map.int_addr == 0) && (map.mask_addr == 0))
		return -ENODEV;

	ret = pmu->write_dev_drct(pmu, map.map, map.mask_addr, 0, map.bit_mask);
	if (ret == 0)
		idata->irq_cb[irq].int_en = 1;
	return ret;
}

#ifdef CONFIG_MFD_BCMPMU_DBG
static ssize_t dbgmsk_show(struct device *dev, struct device_attribute *attr,
			   char *buf)
{
	return sprintf(buf, "debug_mask is %x\n", debug_mask);
}
static ssize_t dbgmsk_store(struct device *dev, struct device_attribute *attr,
			    const char *buf, size_t count)
{
	unsigned long val = simple_strtoul(buf, NULL, 0);
	if (val > 0xFF || val == 0)
		return -EINVAL;
	debug_mask = val;
	return count;
}
static ssize_t unmask_store(struct device *dev, struct device_attribute *attr,
			    const char *buf, size_t count)
{
	struct bcmpmu *bcmpmu = dev->platform_data;
	unsigned long val = simple_strtoul(buf, NULL, 0);
	if (val >= PMU_IRQ_MAX)
		return -EINVAL;
	bcmpmu->unmask_irq(bcmpmu, val);
	return count;
}
static ssize_t mask_store(struct device *dev, struct device_attribute *attr,
			  const char *buf, size_t count)
{
	struct bcmpmu *bcmpmu = dev->platform_data;
	unsigned long val = simple_strtoul(buf, NULL, 0);
	if (val >= PMU_IRQ_MAX)
		return -EINVAL;
	bcmpmu->mask_irq(bcmpmu, val);
	return count;
}

static DEVICE_ATTR(dbgmsk, 0644, dbgmsk_show, dbgmsk_store);
static DEVICE_ATTR(unmask, 0644, NULL, unmask_store);
static DEVICE_ATTR(mask, 0644, NULL, mask_store);
static struct attribute *bcmpmu_irq_attrs[] = {
	&dev_attr_dbgmsk.attr,
	&dev_attr_unmask.attr,
	&dev_attr_mask.attr,
	NULL
};
static const struct attribute_group bcmpmu_irq_attr_group = {
	.attrs = bcmpmu_irq_attrs,
};
#endif

static int __devinit bcmpmu_irq_probe(struct platform_device *pdev)
{
	int ret;
	struct bcmpmu_irq_data *idata;
	struct bcmpmu *bcmpmu = pdev->dev.platform_data;
	struct bcmpmu_platform_data *pdata = bcmpmu->pdata;
	int *irqregs;

	idata = kzalloc(sizeof(struct bcmpmu_irq_data), GFP_KERNEL);
	if (idata == NULL) {
		ret = -ENOMEM;
		dev_err(bcmpmu->dev, "%s: kzalloc failed: %d\n", __func__, ret);
		return ret;
	}
	pr_irq(INIT, "%s, called.\n", __func__);

	bcmpmu->register_irq = bcmpmu_register_irq;
	bcmpmu->unregister_irq = bcmpmu_unregister_irq;
	bcmpmu->mask_irq = bcmpmu_mask_irq;
	bcmpmu->unmask_irq = bcmpmu_unmask_irq;
	idata->irqmap = bcmpmu_get_irqmap(bcmpmu);
	idata->irqregmap = bcmpmu_get_irqregmap(bcmpmu, &idata->irqreg_size);

	irqregs = kzalloc((idata->irqreg_size * sizeof(int)), GFP_KERNEL);
	if (irqregs == NULL) {
		ret = -ENOMEM;
		dev_err(bcmpmu->dev, "%s: kzalloc failed: %d\n", __func__, ret);
		goto err;
	}
	idata->irq_regs = irqregs;
	idata->bcmpmu = bcmpmu;
	mutex_init(&idata->ilock);
	idata->workq = create_workqueue("bcmpmu-irq");
	INIT_WORK(&idata->work, bcmpmu_irq_handler);
	idata->irq = pdata->irq;

	bcmpmu->irqinfo = idata;
	ret = request_irq(pdata->irq, bcmpmu_isr,
			IRQF_DISABLED | IRQF_TRIGGER_FALLING |
			IRQF_NO_SUSPEND, "bcmpmu-irq", idata);
	if (ret) {
		pr_irq(ERROR, "%s, failed request irq.\n", __func__);
		goto err0;
	}
	disable_irq(pdata->irq);
	bcmpmu_read_irq_regs(idata);
	enable_irq(pdata->irq);

#ifdef CONFIG_MFD_BCMPMU_DBG
	ret = sysfs_create_group(&pdev->dev.kobj, &bcmpmu_irq_attr_group);
#endif
	return 0;
err0:
	kfree(idata->irq_regs);
err:
	kfree(idata);
	return ret;
}

static int __devexit bcmpmu_irq_remove(struct platform_device *pdev)
{
	struct bcmpmu *bcmpmu = pdev->dev.platform_data;
	struct bcmpmu_irq_data *idata =
	    (struct bcmpmu_irq_data *)bcmpmu->irqinfo;
	if (idata->irq)
		free_irq(bcmpmu->pdata->irq, bcmpmu);
	kzfree(idata->irq_regs);
	kzfree(bcmpmu->irqinfo);
	return 0;

}

static struct platform_driver bcmpmu_irq_driver = {
	.driver = {
		   .name = "bcmpmu_irq",
		   },
	.probe = bcmpmu_irq_probe,
	.remove = __devexit_p(bcmpmu_irq_remove),
};

static int __init bcmpmu_irq_init(void)
{
	return platform_driver_register(&bcmpmu_irq_driver);
}

subsys_initcall(bcmpmu_irq_init);

static void __exit bcmpmu_irq_exit(void)
{
	platform_driver_unregister(&bcmpmu_irq_driver);
}

module_exit(bcmpmu_irq_exit);

MODULE_DESCRIPTION("BCM PMIC irq driver");
MODULE_LICENSE("GPL");
