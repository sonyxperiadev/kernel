/*****************************************************************************
*  Copyright 2001 - 2012 Broadcom Corporation.  All rights reserved.
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
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/mutex.h>
#include <linux/moduleparam.h>
#include <linux/uaccess.h>
#include <mach/gpio.h>
#include <linux/gpio.h>
#ifdef CONFIG_DEBUG_FS
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#endif

#include <linux/mfd/bcmpmu59xxx.h>
#include <linux/mfd/bcmpmu59xxx_reg.h>

static int debug_mask = BCMPMU_PRINT_ERROR | BCMPMU_PRINT_INIT;

module_param_named(debug_mask, debug_mask, int, S_IRUGO | S_IWUSR | S_IWGRP);
#define pr_irq(debug_level, args...) \
	do { \
		if (debug_mask & BCMPMU_PRINT_##debug_level) { \
			pr_info(args); \
		} \
	} while (0)

struct irq_cb {
	void (*callback) (enum bcmpmu59xxx_irq, void *);
	void *idata;
	bool int_en;
};

struct bcmpmu59xxx_irq_data {
	int irq;
	u8 *irq_regs;
	struct bcmpmu59xxx *bcmpmu;
	struct irq_cb irq_cb[PMU_IRQ_MAX];
	struct mutex ilock;
	struct workqueue_struct *workq;
	struct work_struct work;
#ifdef CONFIG_DEBUG_FS
	struct dentry *dentry;
#endif
};


#ifdef CONFIG_DEBUG_FS

static ssize_t bcmpmu_irq_mask(struct file *file,
					   char const __user *buf,
					   size_t count, loff_t *offset)
{
	u32 len = 0;
	int ret;
	u32 irq_no = PMU_IRQ_MAX;
	char input_str[100];
	struct bcmpmu59xxx *bcmpmu =
		(struct bcmpmu59xxx *)file->private_data;
	BUG_ON(bcmpmu == NULL);

	if (count > 100)
		len = 100;
	else
		len = count;

	if (copy_from_user(input_str, buf, len))
		return -EFAULT;
	/* coverity[secure_coding] */
	sscanf(input_str, "%x", &irq_no);

	if (irq_no >= PMU_IRQ_MAX) {
		pr_info("invalid param !!\n");
		return -EFAULT;
	}

	ret = bcmpmu->mask_irq(bcmpmu, irq_no);
	if (ret)
		return ret;
	return count;
}

static const struct file_operations debug_irq_mask_fops = {
	.write = bcmpmu_irq_mask,
	.open = bcmpmu_debugfs_open,
};

static ssize_t bcmpmu_irq_unmask(struct file *file,
					   char const __user *buf,
					   size_t count, loff_t *offset)
{
	u32 len = 0;
	int ret;
	u32 irq_no = PMU_IRQ_MAX;
	char input_str[100];
	struct bcmpmu59xxx *bcmpmu =
		(struct bcmpmu59xxx *)file->private_data;
	BUG_ON(bcmpmu == NULL);

	if (count > 100)
		len = 100;
	else
		len = count;

	if (copy_from_user(input_str, buf, len))
		return -EFAULT;
	/* coverity[secure_coding] */
	sscanf(input_str, "%x", &irq_no);

	if (irq_no >= PMU_IRQ_MAX) {
		pr_info("invalid param !!\n");
		return -EFAULT;
	}

	ret = bcmpmu->unmask_irq(bcmpmu, irq_no);
	if (ret)
		return ret;
	return count;
}

static const struct file_operations debug_irq_unmask_fops = {
	.write = bcmpmu_irq_unmask,
	.open = bcmpmu_debugfs_open,
};

static void bcmpmu59xxx_irq_debug_init(struct bcmpmu59xxx_irq_data *irq_data)
{
	BUG_ON(!irq_data->bcmpmu ||
			!irq_data->bcmpmu->dent_bcmpmu);

	irq_data->dentry = debugfs_create_dir("irq",
				irq_data->bcmpmu->dent_bcmpmu);
	if (!irq_data->dentry) {
		pr_err("Failed to initialize debugfs\n");
		return;
	}
	if (!debugfs_create_u32("dbg_mask",
		S_IWUSR | S_IRUSR, irq_data->dentry, &debug_mask))
		goto err;

	if (!debugfs_create_file("mask", S_IRUSR, irq_data->dentry,
				 irq_data->bcmpmu, &debug_irq_mask_fops))
		goto err;

	if (!debugfs_create_file("unmask", S_IRUSR, irq_data->dentry,
				 irq_data->bcmpmu, &debug_irq_unmask_fops))
		goto err;
	return;
err:
	pr_err("Failed to setup bcmpmu debugfs\n");
	debugfs_remove(irq_data->dentry);
}

#endif /* CONFIG_DEBUG_FS */

static irqreturn_t bcmpmu59xxx_isr(int irq, void *data)
{
	struct bcmpmu59xxx_irq_data *idata = data;
	if (!queue_work(idata->workq, &idata->work))
		pr_info("%s: The work is already queued\n", __func__);
	return IRQ_HANDLED;
}

static void bcmpmu59xxx_read_irq_regs(struct bcmpmu59xxx_irq_data *idata)
{
	int i;
	idata->bcmpmu->read_dev_bulk(idata->bcmpmu,
			 PMU_REG_INT1,
			idata->irq_regs,
			NUM_IRQ_REG);
	if (BCMPMU_PRINT_DATA & debug_mask) {
		for (i = 0; i < NUM_IRQ_REG; i++) {
			pr_irq(DATA, "%s int=0x%X\n",
					__func__, idata->irq_regs[i]);
		}
	}
}

u8 bcmpmu59xxx_get_irq_index(struct bcmpmu59xxx_irq_data *idata,
				 enum bcmpmu59xxx_irq *irq_num)
{
	static u8 reg_index;
	u8 bit_num, i ;
	for (i = reg_index; i < NUM_IRQ_REG; i++) {
		if (idata->irq_regs[i]) {
			bit_num = fls(idata->irq_regs[i]) - 1;
			idata->irq_regs[i] &=
					    ~(1 << bit_num);
			*irq_num = (8 * i) + bit_num ;
			reg_index = i;
			return true;
			}
	}
	*irq_num = PMU_IRQ_MAX;
	reg_index = 0;
	return false;
}

void bcmpmu59xxx_irq_handler(struct work_struct *work)
{
	bool ret;
	enum bcmpmu59xxx_irq index;
	struct bcmpmu59xxx_irq_data *idata;
	struct bcmpmu59xxx *bcmpmu;
	int irq, gpio_state;

	idata = container_of(work, struct bcmpmu59xxx_irq_data, work);
	bcmpmu = idata->bcmpmu;
	irq = idata->irq;

	do {
		ret = true;
		bcmpmu59xxx_read_irq_regs(idata);
		mutex_lock(&idata->ilock);
		while (ret) {
			ret = bcmpmu59xxx_get_irq_index(idata, &index);
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
int bcmpmu59xxx_set_irq_mask(struct bcmpmu59xxx *pmu, int irq, bool mask)
{
	struct bcmpmu59xxx_irq_data *idata =
			(struct bcmpmu59xxx_irq_data *)pmu->irqinfo;
	int ret = 0;
	u32 irq_reg, irq_num ;
	u32 addr;
	u8 bit, temp ;

	pr_info("%s: irq %d mask %d\n", __func__, irq, mask);
	irq_reg = irq / IRQ_REG_WIDTH;
	irq_num = irq & (IRQ_REG_WIDTH - 1);

	addr = irq_reg + PMU_REG_INT1MSK;

	bit = 1 << irq_num ;
	pr_info("%s: addr %x bit %d\n", __func__, addr, bit);
	ret =
	    pmu->read_dev(pmu, addr, &temp);
	if (ret < 0)
		return ret;
	if (mask)
		temp &= (~bit);
	else
		temp |= (bit);

	ret = pmu->write_dev(pmu, addr, temp);
	if (ret == 0)
		idata->irq_cb[irq].int_en = mask;

	return ret;
}

static void bcmpmu59xxx_mask_irqs(struct bcmpmu59xxx_irq_data *idata)
{
	int i;
	for (i = 0; i < NUM_IRQ_REG; i++)
		idata->bcmpmu->write_dev(idata->bcmpmu,
				PMU_REG_INT1MSK + i,
				0xFF);
}

int bcmpmu59xxx_unmask_irq(struct bcmpmu59xxx *pmu, enum bcmpmu59xxx_irq irq)
{
	struct bcmpmu59xxx_irq_data *idata =
			(struct bcmpmu59xxx_irq_data *)pmu->irqinfo;
	if (irq >= PMU_IRQ_MAX)
		return -ENODEV;
	if (idata->irq_cb[irq].int_en)
		return 0;
	return bcmpmu59xxx_set_irq_mask(pmu, irq, true);
}

int bcmpmu59xxx_mask_irq(struct bcmpmu59xxx *pmu, enum bcmpmu59xxx_irq irq)
{
	struct bcmpmu59xxx_irq_data *idata =
			(struct bcmpmu59xxx_irq_data *)pmu->irqinfo;

	if (irq >= PMU_IRQ_MAX)
		return -ENODEV;
	if (!idata->irq_cb[irq].int_en)
		return 0;
	return bcmpmu59xxx_set_irq_mask(pmu, irq, false);
}

int bcmpmu59xxx_register_irq(struct bcmpmu59xxx *pmu, enum bcmpmu59xxx_irq irq,
			void (*callback) (enum bcmpmu59xxx_irq irq, void *),
			void *data)
{
	struct bcmpmu59xxx_irq_data *idata =
			(struct bcmpmu59xxx_irq_data *)pmu->irqinfo;
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

int bcmpmu59xxx_unregister_irq(struct bcmpmu59xxx *pmu,
				enum bcmpmu59xxx_irq irq)
{
	struct bcmpmu59xxx_irq_data *idata =
			(struct bcmpmu59xxx_irq_data *)pmu->irqinfo;
	int ret;
	if (irq >= PMU_IRQ_MAX)
		return -ENODEV;
	mutex_lock(&idata->ilock);
	if (idata->irq_cb[irq].callback != NULL) {
		idata->irq_cb[irq].callback = NULL;
		idata->irq_cb[irq].idata = NULL;
		ret = 0;
	} else
		ret = -EINVAL;
	bcmpmu59xxx_mask_irq(pmu, irq);
	mutex_unlock(&idata->ilock);
	return ret;
}

static int bcmpmu59xxx_irq_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct bcmpmu59xxx_irq_data *idata;
	struct bcmpmu59xxx *bcmpmu =  dev_get_drvdata(pdev->dev.parent);
	struct bcmpmu59xxx_platform_data *pdata;
	u8 *irqregs;
	pdata = bcmpmu->pdata;
	idata = kzalloc(sizeof(struct bcmpmu59xxx_irq_data), GFP_KERNEL);
	if (idata == NULL) {
		ret = -ENOMEM;
		dev_err(bcmpmu->dev, "%s: kzalloc failed: %d\n", __func__, ret);
		return ret;
	}
	pr_irq(INIT, "%s, called.\n", __func__);

	bcmpmu->register_irq = bcmpmu59xxx_register_irq;
	bcmpmu->unregister_irq = bcmpmu59xxx_unregister_irq;
	bcmpmu->mask_irq = bcmpmu59xxx_mask_irq;
	bcmpmu->unmask_irq = bcmpmu59xxx_unmask_irq;
	irqregs = kzalloc((NUM_IRQ_REG * sizeof(char)), GFP_KERNEL);
	if (irqregs == NULL) {
		ret = -ENOMEM;
		dev_err(bcmpmu->dev, "%s: kzalloc failed: %d\n", __func__, ret);
		goto err;
	}
	idata->irq_regs = irqregs;
	idata->bcmpmu = bcmpmu;
	mutex_init(&idata->ilock);
	idata->workq = create_workqueue("bcmpmu59xxx-irq");
	INIT_WORK(&idata->work, bcmpmu59xxx_irq_handler);
	idata->irq = pdata->irq;

	bcmpmu->irqinfo = idata;
	ret = request_irq(pdata->irq, bcmpmu59xxx_isr,
			IRQF_DISABLED | IRQF_TRIGGER_FALLING |
			IRQF_NO_SUSPEND, "bcmpmu59xxx-irq", idata);
	if (ret) {
		pr_irq(ERROR, "%s, failed request irq.\n", __func__);
		goto err0;
	}
#ifdef CONFIG_DEBUG_FS
	bcmpmu59xxx_irq_debug_init(idata);
#endif
	disable_irq(pdata->irq);
	bcmpmu59xxx_mask_irqs(idata);
	bcmpmu59xxx_read_irq_regs(idata);
	enable_irq(pdata->irq);
	return 0;
err0:
	kfree(idata->irq_regs);
err:
	kfree(idata);
	return ret;
}

static int bcmpmu59xxx_irq_remove(struct platform_device *pdev)
{
	struct bcmpmu59xxx *bcmpmu =  dev_get_drvdata(pdev->dev.parent);
	struct bcmpmu59xxx_platform_data *pdata ;
	struct bcmpmu59xxx_irq_data *idata =
	    (struct bcmpmu59xxx_irq_data *)bcmpmu->irqinfo;

	pdata = bcmpmu->pdata;
	if (idata->irq)
		free_irq(pdata->irq, bcmpmu);
#ifdef CONFIG_DEBUG_FS
	if (idata->dentry)
		debugfs_remove(idata->dentry);
#endif /* CONFIG_DEBUG_FS */

	kzfree(idata->irq_regs);
	kzfree(bcmpmu->irqinfo);
	return 0;
}

static struct platform_driver bcmpmu59xxx_irq_driver = {
	.driver = {
		   .name = "bcmpmu59xxx_irq",
		   },
	.probe = bcmpmu59xxx_irq_probe,
	.remove = bcmpmu59xxx_irq_remove,
};

static int __init bcmpmu59xxx_irq_init(void)
{
	return platform_driver_register(&bcmpmu59xxx_irq_driver);
}
arch_initcall(bcmpmu59xxx_irq_init);

static void __exit bcmpmu59xxx_irq_exit(void)
{
	platform_driver_unregister(&bcmpmu59xxx_irq_driver);
}
module_exit(bcmpmu59xxx_irq_exit);

MODULE_DESCRIPTION("BCM PMIC irq driver");
MODULE_LICENSE("GPL");
