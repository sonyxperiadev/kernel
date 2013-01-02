
/*****************************************************************************
* Copyright 2010 - 2011 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/param.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/pm.h>
#include <linux/io.h>
#include <linux/debugfs.h>
#include <linux/workqueue.h>
#include <linux/notifier.h>
#include <linux/wakelock.h>
#include <linux/interrupt.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/clk.h>
#include <linux/irq.h>
#include <linux/delay.h>
#include <mach/kona_tmon.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <mach/rdb/brcm_rdb_tmon.h>
#include <mach/rdb/brcm_rdb_chipreg.h>

#define CLR_INT 1
#define INVALID_INX	0xFFFFFFFF
#define THOLD_VAL_MAX 0xD

struct kona_tmon {
	int irq;
	struct work_struct tmon_work;
	struct delayed_work poll_work;
	struct workqueue_struct *wqueue;
	struct clk *tmon_1m_clk;
	struct clk *tmon_apb_clk;
	struct kona_tmon_pdata *pdata;
	struct atomic_notifier_head notifiers;
	int thresh_inx;
	int poll_inx;
};

static struct kona_tmon *kona_tmon;

static int tmon_dbg_mask;
enum {
	TMON_LOG_ERR = 1 << 0,		/*b 00000001*/
	TMON_LOG_DBG = 1 << 1,		/*b 00000010*/
	TMON_LOG_DBGFS = 1 << 2,	/*b 00000100*/
	TMON_LOG_INIT = 1 << 3,		/*b 00001000*/
};

static long raw_to_celcius(unsigned long raw)
{
	return (407000 - (538 * raw)) / 1000;
}

static unsigned long celcius_to_raw(long celcius)
{
	return (407000 - celcius * 1000) / 538;
}

long tmon_get_current_temp(void)
{
	unsigned long raw_curr;
	BUG_ON(kona_tmon == NULL);

	struct kona_tmon_pdata *pdata = kona_tmon->pdata;

	raw_curr = readl(pdata->base_addr + TMON_TEMP_VAL_OFFSET) &
		TMON_TEMP_VAL_TEMP_VAL_MASK;
	return raw_to_celcius(raw_curr);
}
EXPORT_SYMBOL_GPL(tmon_get_current_temp);

static void tmon_poll_work(struct work_struct *ws)
{
	struct kona_tmon *tmon = container_of(ws, struct kona_tmon,
			poll_work);
	struct kona_tmon_pdata *pdata = tmon->pdata;
	long curr_temp;
	int poll_inx;
	int intr_en = 0;

	BUG_ON(tmon->poll_inx == INVALID_INX);
	curr_temp = tmon_get_current_temp();
	poll_inx = tmon->poll_inx;
	if (curr_temp <= (pdata->thold[poll_inx].falling +
			pdata->hysteresis)) {
		pr_info("%s: reached the polling temp %ld\n", __func__,
				pdata->thold[poll_inx].falling);
		/* updating threshold value and indexes*/
		writel(celcius_to_raw(pdata->thold[poll_inx].rising),
				pdata->base_addr + TMON_CFG_INT_THRESH_OFFSET);
		if (tmon->thresh_inx == INVALID_INX)
			intr_en = 1;
		atomic_notifier_call_chain(&tmon->notifiers,
				pdata->thold[poll_inx].falling, NULL);
		tmon->thresh_inx = poll_inx;
		tmon->poll_inx = poll_inx ? (poll_inx - 1)
			: INVALID_INX;
		writel(CLR_INT, pdata->base_addr + TMON_CFG_CLR_INT_OFFSET);
		if (intr_en)
			enable_irq(tmon->irq);
	}

	if (tmon->poll_inx != INVALID_INX)
		queue_delayed_work(tmon->wqueue, &tmon->poll_work,
				msecs_to_jiffies(pdata->poll_rate_ms));
}

static void tmon_irq_work(struct work_struct *ws)
{
	struct kona_tmon *tmon = container_of(ws, struct kona_tmon, tmon_work);
	struct kona_tmon_pdata *pdata = tmon->pdata;
	if (tmon->poll_inx != INVALID_INX)
		queue_delayed_work(tmon->wqueue, &tmon->poll_work,
				msecs_to_jiffies(pdata->poll_rate_ms));

	atomic_notifier_call_chain(&tmon->notifiers,
			pdata->thold[tmon->poll_inx].rising, NULL);
}

static irqreturn_t tmon_isr(int irq, void *drvdata)
{
	struct kona_tmon *tmon =  drvdata;
	struct kona_tmon_pdata *pdata = tmon->pdata;
	long curr_temp;

	BUG_ON(tmon->thresh_inx == INVALID_INX);
	curr_temp = tmon_get_current_temp();
	pr_info(KERN_ALERT "SoC temperature threshold of %ld exceeded. Current temperature is %ld\n",
			pdata->thold[tmon->thresh_inx].rising, curr_temp);

	tmon->poll_inx = tmon->thresh_inx;
	/*Find next rising thold*/
	if ((tmon->thresh_inx < pdata->thold_size - 1) &&
		 (pdata->thold[tmon->thresh_inx + 1].flags & TMON_NOTIFY)) {
		tmon->thresh_inx++;
		writel(celcius_to_raw(pdata->thold[tmon->thresh_inx].rising),
				pdata->base_addr + TMON_CFG_INT_THRESH_OFFSET);
	} else {
		/*Set THOLD to max value*/
		writel(THOLD_VAL_MAX,
				pdata->base_addr + TMON_CFG_INT_THRESH_OFFSET);
		disable_irq_nosync(tmon->irq);
		tmon->thresh_inx = INVALID_INX;
	}
	/*Clear interrupt*/
	writel(CLR_INT, pdata->base_addr + TMON_CFG_CLR_INT_OFFSET);
	queue_work(tmon->wqueue, &tmon->tmon_work);
	return IRQ_HANDLED;
}

int tmon_register_notifier(struct notifier_block *notifier)
{
	return atomic_notifier_chain_register(&kona_tmon->notifiers, notifier);
}
EXPORT_SYMBOL_GPL(tmon_register_notifier);

int tmon_unregister_notifier(struct notifier_block *notifier)
{
	return atomic_notifier_chain_unregister(&kona_tmon->notifiers,
			notifier);
}
EXPORT_SYMBOL_GPL(tmon_unregister_notifier);

#ifdef CONFIG_DEBUG_FS

static int tmon_dbg_get_threshold(void *data, u64 *val)
{
	u32 reg;
	unsigned long raw;
	struct kona_tmon *tmon = (struct kona_tmon *)data;
	BUG_ON(tmon == NULL);
	struct kona_tmon_pdata *pdata = tmon->pdata;

	reg = readl(pdata->base_addr + TMON_CFG_INT_THRESH_OFFSET);
	raw = reg & TMON_CFG_INT_THRESH_INT_THRESH_MASK;
	*val = raw_to_celcius(raw);

	return 0;
}

static int tmon_dbg_set_threshold(void *data, u64 val)
{
	u32 reg;
	unsigned long raw;
	struct kona_tmon *tmon = (struct kona_tmon *)data;
	BUG_ON(tmon == NULL);
	struct kona_tmon_pdata *pdata = tmon->pdata;

	raw = celcius_to_raw(val);
	writel(raw, pdata->base_addr + TMON_CFG_INT_THRESH_OFFSET);
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(tmon_threshold_ops, tmon_dbg_get_threshold,
		tmon_dbg_set_threshold, "%llu\n");

static int tmon_dbg_get_interval(void *data, u64 *val)
{
	u32 reg;
	unsigned long ticks;
	struct kona_tmon *tmon = (struct kona_tmon *)data;
	BUG_ON(tmon == NULL);
	struct kona_tmon_pdata *pdata = tmon->pdata;

	reg = readl(pdata->base_addr + TMON_CFG_INTERVAL_VAL_OFFSET);
	ticks = reg & TMON_CFG_INTERVAL_VAL_INTERVAL_VAL_MASK;
	*val = ticks / 32;

	return 0;
}

static int tmon_dbg_set_interval(void *data, u64 val)
{
	struct kona_tmon *tmon = (struct kona_tmon *)data;
	BUG_ON(tmon == NULL);
	struct kona_tmon_pdata *pdata = tmon->pdata;

	writel(32 * val, pdata->base_addr + TMON_CFG_INTERVAL_VAL_OFFSET);
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(tmon_interval_ops, tmon_dbg_get_interval,
		tmon_dbg_set_interval, "%llu\n");

static int kona_tmon_debugfs_init(struct kona_tmon *tmon)
{
	struct dentry *dent_tmon_root_dir;
	dent_tmon_root_dir = debugfs_create_dir("tmon", 0);
	if (!dent_tmon_root_dir)
		return -ENOMEM;

	if (!debugfs_create_u32
			("tmon_dbg_mask", S_IWUSR | S_IRUSR, dent_tmon_root_dir,
			 (int *)&tmon_dbg_mask))
		return -ENOMEM;

	if (!debugfs_create_file
			("threshold", S_IWUSR | S_IRUSR, dent_tmon_root_dir,
			 tmon, &tmon_threshold_ops))
		return -ENOMEM;

	if (!debugfs_create_file
			("interval", S_IWUSR | S_IRUSR, dent_tmon_root_dir,
			 tmon, &tmon_interval_ops))
		return -ENOMEM;

	return 0;
}
#endif

static int kona_tmon_suspend(struct platform_device *pdev)
{
	if (kona_tmon->poll_inx != INVALID_INX) {
		flush_delayed_work_sync(&kona_tmon->poll_work);
		tmon_dbg(TMON_LOG_DBG, "%s: cancelling work queue\n", __func__);
	}
	return 0;
}

static int kona_tmon_resume(struct platform_device *pdev)
{
	if (kona_tmon->poll_inx != INVALID_INX) {
		queue_delayed_work(kona_tmon->wqueue, &kona_tmon->poll_work,
				msecs_to_jiffies(0));
		tmon_dbg(TMON_LOG_DBG, "%s: starting work queue\n", __func__);
	}
	return 0;
}


static int kona_tmon_probe(struct platform_device *pdev)
{
	u32 reg, val, *addr;
	char *clk_name;
	int size, i, irq;
	int rc = 0;
	int rst_inx = INVALID_INX;
	struct resource *iomem;
	long rising, falling;
	struct kona_tmon_pdata *pdata;

	kona_tmon = kzalloc(sizeof(struct kona_tmon), GFP_KERNEL);
	if (pdev->dev.of_node) {
		pdata = kzalloc(sizeof(struct kona_tmon_pdata),	GFP_KERNEL);
		if (!pdata) {
			rc = -ENOMEM;
			goto err_free_dev_mem;
		}

		/* Get register memory resource */
		iomem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
		if (!iomem) {
			tmon_dbg(TMON_LOG_ERR, "no mem resource\n");
			rc = -ENODEV;
			kfree(pdata);
			goto err_free_dev_mem;
		}
		pdata->base_addr = ioremap(iomem->start, resource_size(iomem));
		if (!pdata->base_addr) {
			tmon_dbg(TMON_LOG_ERR, "unable to map in registers\n");
			kfree(pdata);
			rc = -ENOMEM;
			goto err_free_dev_mem;
		}

		/* Get interrupt number */
		irq = platform_get_irq(pdev, 0);
		if (irq == -ENXIO) {
			tmon_dbg(TMON_LOG_ERR, "no irq resource\n");
			kfree(pdata);
			rc = -ENODEV;
			goto err_free_dev_mem;
		}
		pdata->irq = irq;

		addr = (u32 *)of_get_property(pdev->dev.of_node, "chipreg_addr",
				&size);
		if (!addr) {
			kfree(pdata);
			rc = -EINVAL;
			goto err_free_dev_mem;
		}
		val = *(addr + 1);
		pdata->chipreg_addr = ioremap(be32_to_cpu(*addr),
				be32_to_cpu(val));

		int ret;
		ret = of_property_read_u32(pdev->dev.of_node,
				"thold_size", &val);
		if (ret != 0) {
			kfree(pdata);
			rc = -EINVAL;
			goto err_free_dev_mem;
		}
		pdata->thold_size = val;

		if (of_property_read_u32(pdev->dev.of_node,
					"poll_rate_ms", &val)) {
			rc = -EINVAL;
			kfree(pdata);
			goto err_free_dev_mem;
		}
		pdata->poll_rate_ms = val;

		if (of_property_read_u32(pdev->dev.of_node,
					"hysteresis", &val)) {
			rc = -EINVAL;
			kfree(pdata);
			goto err_free_dev_mem;
		}
		pdata->hysteresis = val;

		if (of_property_read_u32(pdev->dev.of_node, "flags", &val)) {
			rc = -EINVAL;
			kfree(pdata);
			goto err_free_dev_mem;
		}
		pdata->flags = val;

		if (of_property_read_u32(pdev->dev.of_node,
					"interval_ms", &val)) {
			rc = -EINVAL;
			kfree(pdata);
			goto err_free_dev_mem;
		}
		pdata->interval_ms = val;

		if (of_property_read_string(pdev->dev.of_node,
					"tmon_apb_clk", &clk_name)) {
			rc = -EINVAL;
			kfree(pdata);
			goto err_free_dev_mem;
		}
		pdata->tmon_apb_clk = clk_name;

		if (of_property_read_string(pdev->dev.of_node,
					"tmon_1m_clk", &clk_name)) {
			rc = -EINVAL;
			kfree(pdata);
			goto err_free_dev_mem;
		}
		pdata->tmon_1m_clk = clk_name;

		int *thold;
		thold = (int *)of_get_property(pdev->dev.of_node,
				"thold", &size);
		if (!thold) {
			rc = -EINVAL;
			kfree(pdata);
			goto err_free_dev_mem;
		}
		pdata->thold =
		kzalloc(sizeof(struct tmon_state)*pdata->thold_size,
				GFP_KERNEL);
		for (i = 0; i < pdata->thold_size; i++) {
			pdata->thold[i].rising =
				be32_to_cpu(*thold++);
			pdata->thold[i].falling =
				be32_to_cpu(*thold++);
			pdata->thold[i].flags = be32_to_cpu(*thold++);
		}
	} else if (pdev->dev.platform_data)
		pdata =	(struct kona_tmon_pdata *)pdev->dev.platform_data;

	else {
		tmon_dbg(TMON_LOG_ERR, "no platform data found\n");
		rc = -EINVAL;
		goto err_free_dev_mem;
	}

	kona_tmon->pdata = pdata;

	/*checking for input threshold temperature and reset threshold*/
	for (i = 0; i < pdata->thold_size; i++) {
		rising = pdata->thold[i].rising;
		falling = pdata->thold[i].falling;
		if (rising < falling) {
			BUG();
			rc = -EINVAL;
			goto err_free_dev_mem;
		}
		if (pdata->thold[i].flags & TMON_SHDWN) {
			rst_inx = i;
			pr_info("%s: reset temperature is %ld\n",
					__func__, pdata->thold[rst_inx].rising);
		}
	}

	/* Enable clocks */
	kona_tmon->tmon_apb_clk = clk_get(NULL, pdata->tmon_apb_clk);
	if (IS_ERR_OR_NULL(kona_tmon->tmon_apb_clk)) {
		tmon_dbg(TMON_LOG_ERR, "couldn't get tmon_apb_clk\n");
		rc = -ENODEV;
		goto err_free_dev_mem;
	}
	clk_enable(kona_tmon->tmon_apb_clk);

	kona_tmon->tmon_1m_clk = clk_get(NULL, pdata->tmon_1m_clk);
	if (IS_ERR_OR_NULL(kona_tmon->tmon_1m_clk)) {
		tmon_dbg(TMON_LOG_ERR, "couldn't get tmon_1m_clk\n");
		rc = -ENODEV;
		goto err_disable_apb_clk;
	}
	if (clk_set_rate(kona_tmon->tmon_1m_clk, 1000000)) {
		tmon_dbg(TMON_LOG_ERR, "couldn't set rate of tmon_1m_clk\n");
		rc = -ENODEV;
		goto err_put_1m_clk;
	}
	clk_enable(kona_tmon->tmon_1m_clk);

	/* Register hwmon device */
	if (!hwmon_device_register(&pdev->dev)) {
		tmon_dbg(TMON_LOG_ERR, "hwmon device register failed\n");
		rc = -ENODEV;
		goto err_free_dev_mem;
	}

	INIT_WORK(&kona_tmon->tmon_work, tmon_irq_work);
	kona_tmon->wqueue = create_workqueue("poll_workqueue");
	INIT_DELAYED_WORK(&kona_tmon->poll_work, tmon_poll_work);

	if (pdata->flags & ENBALE_VTMON) {
		reg = readl(pdata->chipreg_addr +
				CHIPREG_SPARE_CONTROL0_OFFSET);
		reg &= ~CHIPREG_SPARE_CONTROL0_VTMON_SELECT_MASK;
		writel(reg, pdata->chipreg_addr +
				CHIPREG_SPARE_CONTROL0_OFFSET);
	}

	/* Register interrupt handler */
	kona_tmon->irq = pdata->irq;
	if (0 != request_irq(kona_tmon->irq, tmon_isr,
				IRQ_LEVEL | IRQF_NO_SUSPEND | IRQF_DISABLED,
				pdev->name, kona_tmon)) {
		tmon_dbg(TMON_LOG_ERR, "unable to register isr\n");
		rc = -1;
		goto err_unregister_device;
	}

	ATOMIC_INIT_NOTIFIER_HEAD(&kona_tmon->notifiers);

	kona_tmon->thresh_inx = 0;
	kona_tmon->poll_inx = INVALID_INX;

	/*enabling tmon*/
	reg = readl(pdata->base_addr + TMON_CFG_ENBALE_OFFSET);
	reg &= ~TMON_CFG_ENBALE_ENABLE_MASK;
	writel(reg, pdata->base_addr + TMON_CFG_ENBALE_OFFSET);

	/*setting tmon interval value */
	writel(pdata->interval_ms * 32, pdata->base_addr +
			TMON_CFG_INTERVAL_VAL_OFFSET);

	/*setting the threshold value */
	writel(CLR_INT, pdata->base_addr + TMON_CFG_CLR_INT_OFFSET);
	writel(celcius_to_raw(pdata->thold[kona_tmon->thresh_inx].rising),
			pdata->base_addr + TMON_CFG_INT_THRESH_OFFSET);

	/*setting the reset theshold value */
	if (rst_inx != INVALID_INX) {
		reg = readl(pdata->base_addr + TMON_CFG_RST_EN_OFFSET);
		reg |= TMON_CFG_RST_EN_RST_ENABLE_MASK;
		writel(reg, pdata->base_addr + TMON_CFG_RST_EN_OFFSET);
		writel(celcius_to_raw(pdata->thold[rst_inx].rising),
			pdata->base_addr + TMON_CFG_RST_THRESH_OFFSET);
	} else {
		reg = readl(pdata->base_addr + TMON_CFG_RST_EN_OFFSET);
		reg &= ~TMON_CFG_RST_EN_RST_ENABLE_MASK;
		writel(reg, pdata->base_addr + TMON_CFG_RST_EN_OFFSET);
	}


#ifdef CONFIG_DEBUG_FS
	kona_tmon_debugfs_init(kona_tmon);
#endif

	return 0;
err_unregister_device:
	hwmon_device_unregister(&pdev->dev);
	clk_disable(kona_tmon->tmon_1m_clk);

err_put_1m_clk:
	clk_put(kona_tmon->tmon_1m_clk);

err_disable_apb_clk:
	clk_disable(kona_tmon->tmon_apb_clk);
	clk_put(kona_tmon->tmon_apb_clk);

err_free_dev_mem:
	kfree(kona_tmon);
	return rc;
}

static int kona_tmon_remove(struct platform_device *pdev)
{
	disable_irq(kona_tmon->irq);
	free_irq(kona_tmon->irq, kona_tmon);

	hwmon_device_unregister(&pdev->dev);

	clk_disable(kona_tmon->tmon_1m_clk);
	clk_put(kona_tmon->tmon_1m_clk);

	clk_disable(kona_tmon->tmon_apb_clk);
	clk_put(kona_tmon->tmon_apb_clk);

	kfree(kona_tmon);

	return 0;
}


static const struct of_device_id kona_tmon_dt_ids[] = {
	{ .compatible = "bcm,tmon", },
	{},
};

MODULE_DEVICE_TABLE(of, kona_tmon_dt_ids);

static struct platform_driver kona_tmon_driver = {
	.driver = {
		.name = "kona_tmon",
		.owner = THIS_MODULE,
		.of_match_table = kona_tmon_dt_ids,
	},
	.probe = kona_tmon_probe,
	.remove = kona_tmon_remove,
	.suspend = kona_tmon_suspend,
	.resume = kona_tmon_resume,
};

static int __init kona_tmon_init(void)
{
	return platform_driver_register(&kona_tmon_driver);
}

static void __exit kona_tmon_exit(void)
{
	platform_driver_unregister(&kona_tmon_driver);
}

subsys_initcall(kona_tmon_init);
module_exit(kona_tmon_exit);

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("Temperature Monitor");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");
