/*****************************************************************************
 * Copyright 2010 Broadcom Corporation.  All rights reserved.
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
/*
 * Description: Broadcom Bluetooth rfkill power control via GPIO
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/broadcom/bcmblt-rfkill.h>
#include <linux/platform_device.h>
#include <linux/rfkill.h>
#include <linux/sched.h>

static int bcmblt_rfkill_set_block(void *data, bool blocked)
{
	struct bcmblt_rfkill_platform_data *pdata = 
		(struct bcmblt_rfkill_platform_data *) data;
	int gpio = pdata->gpio;

	/* BT power block is requested */
	if (blocked) {
		/* turn off BT */
		gpio_set_value(gpio, 0);
		rfkill_set_sw_state(pdata->rfkill, true);
		printk(KERN_INFO "%s: BT power block is requested. "
		       "The new BT power gpio value is: %d\n",
		       __FUNCTION__, gpio_get_value(gpio));
	}
	else {
		/* turn on BT */
		gpio_set_value(gpio, 1);
		rfkill_set_sw_state(pdata->rfkill, false);
		printk(KERN_INFO "%s: BT power unblock is requested. "
		       "The new BT power gpio value is: %d\n",
		       __FUNCTION__, gpio_get_value(gpio));
	}
	return 0;
}

/* defines RFKILL operations implemented by this driver */
static const struct rfkill_ops bcmblt_rfkill_ops = {
	.set_block = bcmblt_rfkill_set_block,
};

static int bcmblt_rfkill_probe(struct platform_device *pdev)
{
	int rc = 0;    
	struct bcmblt_rfkill_platform_data *pdata = pdev->dev.platform_data;

	if(!pdata) {
		printk(KERN_ERR "%s: platform_data is not set\n", __FUNCTION__);
		return -ENOENT;
	}

	/* request BT Power GPIO and set it to LOW effectively turning off the
	 * power  */
	rc = gpio_request_one(pdata->gpio, GPIOF_OUT_INIT_LOW, "BT Power");
	if (rc)
		return rc;

	/* allocate RFKILL struct for the driver */
	pdata->rfkill = rfkill_alloc("bcmblt", &pdev->dev, 
				     RFKILL_TYPE_BLUETOOTH,
				     &bcmblt_rfkill_ops,
				     (void *)pdata);

	/* if allocation failed */
	if (!pdata->rfkill) {
		printk(KERN_ERR "%s: rfkill_alloc failed\n", __FUNCTION__);
		gpio_free(pdata->gpio);
		return -ENOMEM;
	}

	/* rfkill drivers that preserve their software block state over power
	 * off use this function to notify the rfkill core (and through that
	 * also userspace) of their initial state.  It should only be used
	 * before registration. */
	rfkill_init_sw_state(pdata->rfkill, true);

	/* register RFKILL struct */
	rc = rfkill_register(pdata->rfkill);
	if (rc)
	{
		printk(KERN_ERR "%s: rfkill_register failed\n", __FUNCTION__);
		gpio_free(pdata->gpio);
		rfkill_destroy(pdata->rfkill);
		return rc;
	}
	return rc;
}

static int bcmblt_rfkill_remove(struct platform_device *pdev)
{
	struct bcmblt_rfkill_platform_data *pdata = pdev->dev.platform_data;

	if (pdata->rfkill) {
		rfkill_unregister(pdata->rfkill);
		rfkill_destroy(pdata->rfkill);
		gpio_free(pdata->gpio);
		pdata->rfkill = NULL;
	}
	pdata->gpio = -1;

	return 0;
}

static int bcmblt_rfkill_suspend(struct platform_device *pdev)
{
	struct bcmblt_rfkill_platform_data *pdata = pdev->dev.platform_data;
	
	/* block power */
	bcmblt_rfkill_set_block(pdata, true);
	return 0;
}

static int bcmblt_rfkill_resume(struct platform_device *pdev)
{
	struct bcmblt_rfkill_platform_data *pdata = pdev->dev.platform_data;
	/* for now do nothing. When testing see if the BT power will be in sync
	   with BT on/off state */
	return 0;
}

static struct platform_driver bcmblt_rfkill_platform_driver = {
	.driver = {
		.name = "bcmblt-rfkill",
		.owner = THIS_MODULE,
	},
	.probe = bcmblt_rfkill_probe,
	.remove = bcmblt_rfkill_remove,
	.suspend = bcmblt_rfkill_suspend,
	.resume = bcmblt_rfkill_resume,
};

static int __init bcmblt_rfkill_init(void)
{
	return platform_driver_register(&bcmblt_rfkill_platform_driver);
}

static void __exit bcmblt_rfkill_exit(void)
{
	platform_driver_unregister(&bcmblt_rfkill_platform_driver);
}

module_init(bcmblt_rfkill_init);
module_exit(bcmblt_rfkill_exit);

MODULE_DESCRIPTION("bcmblt-rfkill");
MODULE_AUTHOR("Broadcom");
MODULE_LICENSE("GPL");
