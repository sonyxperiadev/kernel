/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*	@file drivers/watchdog/bcm59055-wd-tapper.c
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*******************************************************************************/
/**
 * @file
 * driver/watchdog/wd-tapper.c
 *
 * Watchdog Petter - A platform driver that takes care of petting the
 * PMU Watchdog after a set interval of time. This makes sure that the Board
 * does not restart in suspend.
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/vmalloc.h>
#include <linux/platform_device.h>
#include <mach/kona_timer.h>
#include <linux/broadcom/wd-tapper.h>

#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <linux/debugfs.h>

enum {
	WD_NODE_ADD,
	WD_NODE_DEL,
	WD_NODE_UPDATE,
};

static DEFINE_SPINLOCK(tapper_lock);

/* The Driver specific data */
struct wd_tapper_data {
	struct kona_timer *kt;
	unsigned int count;
	unsigned int def_count;
	struct plist_head timeout_list;
	u32 active_timeout;
};

struct wd_tapper_data *wd_tapper_data;

unsigned int wd_tapper_get_timeout(void)
{
	if (wd_tapper_data)
		return ticks_to_sec(wd_tapper_data->count);
	return 0;
}
EXPORT_SYMBOL(wd_tapper_get_timeout);

/**
 * wd_tapper_callback - The timer expiry registered callback
 *
 * Function used to pet the watchdog after the set interval
 *
 */
int wd_tapper_callback(void *dev)
{
	/* Pet the PMU Watchdog */
	pr_info("petting the pmu wd\n");
	return 0;
}

/**
 * wd_tapper_start - Function where the timer is started on suspend
 *
 * @return 0 on successfull set of the timer or negative error value on error
 */
static int wd_tapper_start(struct platform_device *pdev, pm_message_t state)
{
	unsigned long flags;
	int ret;

	spin_lock_irqsave(&tapper_lock, flags);

	ret = kona_timer_set_match_start(wd_tapper_data->kt,
			wd_tapper_data->count);
	if (ret < 0)
		pr_err("kona_timer_set_match_start returned error \r\n");

	spin_unlock_irqrestore(&tapper_lock, flags);
	return ret;
}

/**
 * wd_tapper_stop - Function where the timer is stopped on resume
 *
 * @return 0 on successfull stop of the timer or negative error value on error
 */
static int wd_tapper_stop(struct platform_device *pdev)
{
	if (kona_timer_stop(wd_tapper_data->kt) < 0) {
		pr_err
		    ("Unable to stop the timer kona_timer_stop returned error \r\n");
		return -1;
	}
	return 0;
}

static int wd_tapper_timeout_update(struct wd_tapper_data *wd_tapper,
		struct wd_tapper_node *wd_node, int action)
{
	unsigned long flags;
	u32 new_val;
	int ret = 0;

	spin_lock_irqsave(&tapper_lock, flags);

	switch (action) {
	case WD_NODE_ADD:
		plist_node_init(&wd_node->node, wd_node->timeout);
		plist_add(&wd_node->node, &wd_tapper->timeout_list);
		break;
	case WD_NODE_DEL:
		plist_del(&wd_node->node, &wd_tapper->timeout_list);
		break;
	case WD_NODE_UPDATE:
		plist_del(&wd_node->node, &wd_tapper->timeout_list);
		plist_node_init(&wd_node->node, wd_node->timeout);
		plist_add(&wd_node->node, &wd_tapper->timeout_list);
		break;
	default:
		BUG();
		break;
	}
	new_val = plist_first(&wd_tapper->timeout_list)->prio;
	if (new_val != wd_tapper->active_timeout) {
		/*setting the timeout value */
		wd_tapper_data->count = sec_to_ticks(new_val);
		pr_info("%s: new timeout: %d ticks\n", __func__,
				wd_tapper_data->count);
		wd_tapper->active_timeout = new_val;
	}

	spin_unlock_irqrestore(&tapper_lock, flags);
	return ret;
}


int wd_tapper_add_timeout_req(struct wd_tapper_node *wd_node,
		char *client, u32 timeout)
{
	if ((wd_node == NULL) || (client == NULL))
		return -EINVAL;

	if (unlikely(wd_node->valid))
		return -EINVAL;

	pr_info("%s: client: %s timeout: %d\n", __func__, client, timeout);
	wd_node->name = client;

	if (timeout == TAPPER_DEFAULT_TIMEOUT)
		wd_node->timeout = ticks_to_sec(wd_tapper_data->def_count);
	else
		wd_node->timeout = timeout;

	wd_node->valid = 1;

	return wd_tapper_timeout_update(wd_tapper_data, wd_node, WD_NODE_ADD);

}
EXPORT_SYMBOL(wd_tapper_add_timeout_req);

int wd_tapper_del_timeout_req(struct wd_tapper_node *wd_node)
{
	int ret;

	if ((wd_node == NULL) || (!wd_node->valid))
		return -EINVAL;

	ret = wd_tapper_timeout_update(wd_tapper_data, wd_node, WD_NODE_DEL);

	wd_node->valid = 0;
	wd_node->name = NULL;
	return ret;
}
EXPORT_SYMBOL(wd_tapper_del_timeout_req);

int wd_tapper_update_timeout_req(struct wd_tapper_node *wd_node, u32 timeout)
{
	if (unlikely(wd_node->valid == 0)) {
		BUG();
		return -EINVAL;
	}
	if (timeout == TAPPER_DEFAULT_TIMEOUT)
		timeout =  ticks_to_sec(wd_tapper_data->def_count);
	if (wd_node->timeout != timeout) {
		wd_node->timeout = timeout;
		return wd_tapper_timeout_update(wd_tapper_data,
				wd_node, WD_NODE_UPDATE);
	}
	return 0;
}
EXPORT_SYMBOL(wd_tapper_update_timeout_req);

#ifdef CONFIG_DEBUG_FS
static int wd_tapper_dbg_get_timeout(void *data, u64 *val)
{
	struct wd_tapper_data *di = data;
	*val = ticks_to_sec(di->count);
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(wd_tapper_timeout_fops,
		wd_tapper_dbg_get_timeout,
		NULL, "%llu\n");

static void wd_tapper_debugfs_init(struct wd_tapper_data *di)
{
	struct dentry *dir;
	struct dentry *file;

	dir = debugfs_create_dir("wd_tapper", NULL);

	if (dir) {
		file = debugfs_create_file("timeout", S_IRUSR, dir, di,
				&wd_tapper_timeout_fops);
		if (!file)
			debugfs_remove_recursive(dir);
	}
}
#endif

/**
 * wd_tapper_pltfm_probe - Function where the timer is obtained and configured
 *
 * @return 0 on successful fetch and configuration of the timer or negative
 * error value on error
 */
static int wd_tapper_pltfm_probe(struct platform_device *pdev)
{
	struct wd_tapper_platform_data *pltfm_data;
	struct timer_ch_cfg cfg;
	unsigned int ch_num = 0xFFFF; /* Invalid channel number */
	struct device_node *np = pdev->dev.of_node;
	u32 val;
	char *prop;

	wd_tapper_data = vmalloc(sizeof(struct wd_tapper_data));
	if (wd_tapper_data == NULL) {
		dev_err(&pdev->dev,"Unable to allocate mem for wd_tapper_data\n");
		return -1;
	}

	/* Obtain the platform data */
	pltfm_data = dev_get_platdata(&pdev->dev);
#if 0
	/* Validate the data obtained */
	if (!pltfm_data) {
		dev_err(&pdev->dev, "can't get the platform data\n");
		goto out;
	}
#endif

	if (pltfm_data) {
		/* Get the time out period */
		wd_tapper_data->count = sec_to_ticks(pltfm_data->count);
		wd_tapper_data->def_count = wd_tapper_data->count;

		if (wd_tapper_data->count == 0) {
			dev_err(&pdev->dev, "count value set is 0 - INVALID\n");
			goto out;
		}

		/* Get the channel number */
		ch_num = pltfm_data->ch_num;
		if (ch_num > 3) {
			dev_err(&pdev->dev,
			"Wrong choice of channel number to match\n");
			goto out;
		}

		/* Request the timer context */
		wd_tapper_data->kt = kona_timer_request(pltfm_data->name, ch_num);
		if (wd_tapper_data->kt == NULL) {
			dev_err(&pdev->dev, "kona_timer_request returned error \r\n");
			goto out;
		}
	}

	if (pltfm_data == NULL) {
		/* Get the platform data form dt-blob */
		if (!of_property_read_u32(np, "count", &val)) {
			wd_tapper_data->count = sec_to_ticks(val);
			wd_tapper_data->def_count = wd_tapper_data->count;
		}

		if (wd_tapper_data->count == 0) {
			dev_err(&pdev->dev, "count value set is 0 - INVALID\n");
			goto out;
		}

		if (!of_property_read_u32(np, "ch-num", &val))
			ch_num = val;

		if (ch_num > 3) {
			dev_err(&pdev->dev,
				"Wrong choice of channel number to match\n");
			goto out;
		}

		val = of_property_read_string(np, "timer-name",
				(const char **)&prop);
		if (val != 0)
			goto out;

		wd_tapper_data->kt = kona_timer_request(prop, ch_num);
		if (wd_tapper_data->kt == NULL) {
			dev_err(&pdev->dev,
				"kona_timer_request returned error \r\n");
			goto out;
		}
	}

	/* Populate the timer config */
	cfg.mode = MODE_ONESHOT;
	cfg.arg = wd_tapper_data->kt;
	cfg.cb = (intr_callback) wd_tapper_callback;
	cfg.reload = wd_tapper_data->count;

	/* Configure the timer */
	if (kona_timer_config(wd_tapper_data->kt, &cfg) < 0) {
		dev_err(&pdev->dev, "kona_timer_config returned error \r\n");
		goto out;
	}

	plist_head_init(&wd_tapper_data->timeout_list);

#ifdef CONFIG_DEBUG_FS
	wd_tapper_debugfs_init(wd_tapper_data);
#endif

	dev_info(&pdev->dev, "Probe Success\n");
	return 0;
out:
	dev_err(&pdev->dev, "Probe failed\n");
	vfree(wd_tapper_data);
	return -1;
}

/**
 * wd_tapper_pltfm_remove - Function where the timer is freed
 *
 * @return 0 on successful free of the timer or negative error value on error
 */
static int wd_tapper_pltfm_remove(struct platform_device *pdev)
{
	if (kona_timer_free(wd_tapper_data->kt) < 0) {
		pr_err("Unable to free the timer \r\n");
		return -1;
	}
	return 0;
}

static const struct of_device_id wd_tapper_match[] = {
	{ .compatible = "bcm,wd-tapper" },
	{ /* Sentinel */ }
};

static struct platform_driver wd_tapper_pltfm_driver = {
	.driver = {
		   .name = "wd_tapper",
		   .owner = THIS_MODULE,
		   .of_match_table = wd_tapper_match,
		   },
	.probe = wd_tapper_pltfm_probe,
	.remove = wd_tapper_pltfm_remove,
	.suspend = wd_tapper_start,
	.resume = wd_tapper_stop,
};

static int __init wd_tapper_init(void)
{
	return platform_driver_register(&wd_tapper_pltfm_driver);
}

arch_initcall_sync(wd_tapper_init);

static void __exit wd_tapper_exit(void)
{
	platform_driver_unregister(&wd_tapper_pltfm_driver);
}

module_exit(wd_tapper_exit);

MODULE_AUTHOR("Broadcom Corporation");
MODULE_DESCRIPTION("Watchdog Petter");
MODULE_LICENSE("GPL");;
