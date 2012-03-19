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
#include <linux/vmalloc.h>
#include <linux/platform_device.h>
#include <mach/kona_timer.h>
#include <linux/broadcom/wd-tapper.h>

/* The Driver specific data */
struct wd_tapper_data {
	struct kona_timer *kt;
	unsigned int count;
};

struct wd_tapper_data *wd_tapper_data;

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
	if (kona_timer_set_match_start
	    (wd_tapper_data->kt, wd_tapper_data->count) < 0) {
		pr_err("kona_timer_set_match_start returned error \r\n");
		return -1;
	}
	return 0;
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

/**
 * wd_tapper_pltfm_probe - Function where the timer is obtained and configured
 *
 * @return 0 on successful fetch and configuration of the timer or negative
 * error value on error
 */
static int __devinit wd_tapper_pltfm_probe(struct platform_device *pdev)
{
	struct wd_tapper_platform_data *pltfm_data;
	struct timer_ch_cfg cfg;
	unsigned int ch_num;

	wd_tapper_data = vmalloc(sizeof(struct wd_tapper_data));

	/* Obtain the platform data */
	pltfm_data = dev_get_platdata(&pdev->dev);

	/* Validate the data obtained */
	if (!pltfm_data) {
		dev_err(&pdev->dev, "can't get the platform data\n");
		goto out;
	}

	/* Get the time out period */
	wd_tapper_data->count = msec_to_ticks(pltfm_data->count);
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

	if (pltfm_data->name == NULL) {
		dev_err(&pdev->dev, "Timer name passed is NULL.\n");
		goto out;
	}

	/* Request the timer context */
	wd_tapper_data->kt = kona_timer_request(pltfm_data->name, ch_num);
	if (wd_tapper_data->kt == NULL) {
		dev_err(&pdev->dev, "kona_timer_request returned error \r\n");
		goto out;
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
static int __devexit wd_tapper_pltfm_remove(struct platform_device *pdev)
{
	if (kona_timer_free(wd_tapper_data->kt) < 0) {
		pr_err("Unable to free the timer \r\n");
		return -1;
	}
	return 0;
}

static struct platform_driver wd_tapper_pltfm_driver = {
	.driver = {
		   .name = "wd_tapper",
		   .owner = THIS_MODULE,
		   },
	.probe = wd_tapper_pltfm_probe,
	.remove = __devexit_p(wd_tapper_pltfm_remove),
	.suspend = wd_tapper_start,
	.resume = wd_tapper_stop,
};

static int __init wd_tapper_init(void)
{
	return platform_driver_register(&wd_tapper_pltfm_driver);
}

module_init(wd_tapper_init);

static void __exit wd_tapper_exit(void)
{
	platform_driver_unregister(&wd_tapper_pltfm_driver);
}

module_exit(wd_tapper_exit);

MODULE_AUTHOR("Broadcom Corporation");
MODULE_DESCRIPTION("Watchdog Petter");
MODULE_LICENSE("GPL");;
