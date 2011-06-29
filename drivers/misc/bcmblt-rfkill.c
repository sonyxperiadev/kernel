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
#include <asm/gpio.h>
#include <linux/broadcom/bcmblt-rfkill.h>
#include <linux/platform_device.h>
#include <linux/rfkill.h>
#include <linux/sched.h>

#ifdef BT_WAKE_ASSERT
#define BT_WAKE BT_WAKE_ASSERT
#else
#define BT_WAKE 1
#endif

#define BT_SLEEP !(BT_WAKE)

static int bcmblt_rfkill_set_power(void *data, int state)
{
	int gpio = (int) data;

	switch (state) {
	case RFKILL_USER_STATE_UNBLOCKED: /* off */
		gpio_set_value(gpio, BT_SLEEP);
		pr_info("bcm_blt_rfkill_setpower: unblocked(%d) %s\n", gpio,
					 gpio_get_value(gpio)?"High [POWER ON]":"Low [POWER_OFF]");
		printk(KERN_ERR "bcm_blt_rfkill_setpower: unblocked %s\n",
					 gpio_get_value(gpio)?"High [POWER ON]":"Low [POWER_OFF]");
		break;

	case RFKILL_USER_STATE_SOFT_BLOCKED: /* on */
		gpio_set_value(gpio, BT_WAKE);
		  pr_info("bcm_blt_rfkill_setpower: blocked(%d) %s\n", gpio,
					 gpio_get_value(gpio)?"High [POWER ON]":"Low [POWER_OFF]");
		printk(KERN_ERR "bcm_blt_rfkill_setpower: blocked %s\n",
					 gpio_get_value(gpio)?"High [POWER ON]":"Low [POWER_OFF]");
		break;

	default:
		pr_err("invalid bluetooth rfkill state %d\n", state);
	}
	return 0;
}

static const struct rfkill_ops bcmblt_rfkill_ops = {
	.set_block = bcmblt_rfkill_set_power,
};

static int bcmblt_rfkill_probe(struct platform_device *pdev)
{
	int rc = 0;    

	struct bcmblt_rfkill_platform_data *pdata = pdev->dev.platform_data;
	enum rfkill_user_states default_state = RFKILL_USER_STATE_UNBLOCKED;  /* on */

	if(!pdata) {
		printk(KERN_ERR "%s: platform_data is not set\n", __FUNCTION__);
		return ENOENT;
	}

	rc = gpio_request(pdata->gpio, "BT Power");
	if (rc < 0) {
		pr_err("bcmblt_rfkill_probe: Failed to request GPIO %d, error %d\n", pdata->gpio, rc);
		return rc;
	}
	
	rc = gpio_direction_output(pdata->gpio, 1);
	if (rc < 0) {
		pr_err("bcmblt_rfkill_probe: Failed to configure input direction for GPIO %d, error %d\n", pdata->gpio, rc);
		goto exit_gpio;
	}
	pr_err("bcmblt_rfkill_probe:  Set gpio: %d, level: %s\n",
				pdata->gpio, gpio_get_value(pdata->gpio)?"High":"Low" );

	/* Default to off */
	bcmblt_rfkill_set_power((void *)(pdata->gpio), default_state);

	pdata->rfkill = rfkill_alloc("bcmblt", &pdev->dev, 
				RFKILL_TYPE_BLUETOOTH,
				&bcmblt_rfkill_ops,
				(void *)(pdata->gpio));
	if (unlikely(!pdata->rfkill)) {
		pr_err("bcmblt_rfkill_probe: Failed to allocate memory\n");
		rc = -ENOMEM;
		goto exit_gpio;
	}

	schedule_timeout_interruptible( 2*HZ );
	printk("bcmblt_rfkill sleep for 2 seconds ...\n");
	/* persisted as rfkill_set_default() in older kernels */
	rfkill_set_sw_state(pdata->rfkill, default_state);

	rc = rfkill_register(pdata->rfkill);
	if (unlikely(rc))
	{
		pr_err("bcmblt_rfkill_probe: Failed to register rfkill\n");
		goto exit_destroy;
	}

	return 0;

exit_destroy:
	rfkill_destroy(pdata->rfkill);
exit_gpio:
	gpio_free(pdata->gpio);

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

static struct platform_driver bcmblt_rfkill_platform_driver = {
	.probe = bcmblt_rfkill_probe,
	.remove = bcmblt_rfkill_remove,
	.driver = {
			.name = "bcmblt-rfkill",
			.owner = THIS_MODULE,
			},
};

static int __init bcmblt_rfkill_init(void)
{
	 pr_err("__init bcmblt_rfkill_init()" );
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
