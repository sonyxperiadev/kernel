/*****************************************************************************
* Copyright 2003 - 2011 Broadcom Corporation.  All rights reserved.
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

#include <linux/module.h>
#include <linux/init.h>
#include <asm/gpio.h>
#include <linux/broadcom/bcmbt_lpm_ldisc.h>
#include <linux/platform_device.h>
#include <linux/sched.h>

#include <linux/serial_core.h>
#include <linux/tty.h>
#include <asm/uaccess.h>

static struct tty_ldisc_ops bcmbt_ldisc_ops;
static struct bcmbt_lpm_ldisc_data bcmbt_data = {0,};

/* this define electrical level of GPIO for assert/de-asserted stated.
   sleep logic has by default negative logic */

#ifndef BT_WAKE_ASSERT
#define BT_WAKE_ASSERT 0
#endif
#ifndef BT_WAKE_DEASSERT
#define BT_WAKE_DEASSERT (!(BT_WAKE_ASSERT))
#endif

#ifndef HOST_WAKE_ASSERT
#define HOST_WAKE_ASSERT 0
#endif
#ifndef HOST_WAKE_DEASSERT
#define HOST_WAKE_DEASSERT (!(HOST_WAKE_ASSERT))
#endif

static int bcm_assert_bt_wake(void)
{
	if (bcmbt_data.gpio_bt_wake == -1) 
		return -EFAULT;

	gpio_set_value(bcmbt_data.gpio_bt_wake, BT_WAKE_ASSERT);

	/* block host from sleeping till all tx is done */
#ifdef CONFIG_HAS_WAKELOCK
	wake_lock(&bcmbt_data.bt_wake_lock);
#endif
//	pr_debug("bcm_assert_bt_wake gpio_set_value(%d, %d)\n", bcmbt_data.gpio_bt_wake, BT_WAKE_ASSERT);
	return 0;
}

static int bcm_deassert_bt_wake(void)
{
	if (bcmbt_data.gpio_bt_wake == -1) 
		return -EFAULT;

	gpio_set_value(bcmbt_data.gpio_bt_wake, BT_WAKE_DEASSERT);

	/* release lock: enable deep sleep */
#ifdef CONFIG_HAS_WAKELOCK
	wake_unlock(&bcmbt_data.bt_wake_lock);
#endif
//	pr_debug("bcm_deassert_bt_wake gpio_set_value(%d, %d)\n", bcmbt_data.gpio_bt_wake, BT_WAKE_DEASSERT);
	return 0;
}

static int bcm_get_bt_wake_state(unsigned long arg)
{
	void __user *uarg = (void __user *)arg;
	unsigned long tmp;

	if (bcmbt_data.gpio_bt_wake == -1) 
		return -EFAULT;

	tmp = gpio_get_value(bcmbt_data.gpio_bt_wake);
	if (copy_to_user(uarg, &tmp, sizeof(*uarg)))
		return -EFAULT;

	return 0;
}

static int bcmbt_tty_ioctl(struct tty_struct *tty, struct file *file, unsigned int cmd, unsigned long arg)
{
	int rc = -1;

	switch(cmd) {
	case TIO_ASSERT_BT_WAKE:
		rc = bcm_assert_bt_wake();
		break;

	case TIO_DEASSERT_BT_WAKE:
		rc = bcm_deassert_bt_wake();
		break;

	case TIO_GET_BT_WAKE_STATE:
		rc = bcm_get_bt_wake_state(arg);
		break;
	default:
		return n_tty_ioctl_helper(tty, file, cmd, arg);

	}
	return rc;
}

static int bcmbt_tty_init(void)
{
	int err;

	/* Inherit the N_TTY's ops */
	n_tty_inherit_ops(&bcmbt_ldisc_ops);

	bcmbt_ldisc_ops.owner = THIS_MODULE;
	bcmbt_ldisc_ops.name = "bcmbt_tty";
	bcmbt_ldisc_ops.ioctl = bcmbt_tty_ioctl;

	err = tty_register_ldisc(N_BRCM_HCI, &bcmbt_ldisc_ops);
	if (err)
		pr_err("can't register N_BRCM_HCI line discipline\n");
	else
		pr_info("N_BRCM_HCI line discipline registered\n");

	return err;
}

static void bcmbt_tty_cleanup(void)
{
	int err;

	err = tty_unregister_ldisc(N_BRCM_HCI);
	if (err)
		pr_err("can't unregister N_BRCM_HCI line discipline\n");
	else
		pr_info("N_BRCM_HCI line discipline removed\n");
}

static irqreturn_t host_wake_isr(int irq, void *dev)
{
	unsigned int host_wake;
	/* unsigned long irqflags; */

	/* spin_lock_irqsave(&bcmbt_data.lock, irqflags); */

	host_wake = gpio_get_value(bcmbt_data.gpio_host_wake);

	if (HOST_WAKE_ASSERT == host_wake)
	{
		/* hold lock: disable deep sleep */
#ifdef CONFIG_HAS_WAKELOCK
		wake_lock(&bcmbt_data.host_wake_lock);
#endif
//		pr_debug( "host_wake assert, hold lock at %lld\n", ktime_to_ns(ktime_get()) );
	}
	else
	{
		/* release lock: enable deep sleep */
#ifdef CONFIG_HAS_WAKELOCK
		wake_unlock(&bcmbt_data.host_wake_lock);
#endif
//		pr_debug( "host_wake de-assert, release lock at %lld\n", ktime_to_ns(ktime_get()) );
	}

	/* spin_unlock_irqrestore(&bcmbt_data.lock, irqflags); */

	return IRQ_HANDLED;
}

static int init_bt_wake(struct bcmbt_lpm_ldisc_platform_data *pdata)
{
	int rc;

	if(pdata->gpio_bt_wake < 0)
		return 0;

	if(rc = gpio_request(pdata->gpio_bt_wake, "BT Power Mgmt"))
	{
		pr_err("%s: failed to configure BT Power Mgmt: gpio_request err=%d\n", __FUNCTION__, rc);
		return rc;
	}

	if(rc = gpio_direction_output(pdata->gpio_bt_wake, BT_WAKE_DEASSERT))
	{
		pr_err("%s: failed to configure BT Power Mgmt: gpio_direction_output err=%d\n", __FUNCTION__, rc);
		goto exit_gpio_bt_wake;
	}

#ifdef CONFIG_HAS_WAKELOCK
	wake_lock_init(&bcmbt_data.bt_wake_lock, WAKE_LOCK_SUSPEND, "BTWAKE");
#endif

	/* register the tty line discipline driver */
	if(rc = bcmbt_tty_init())
	{
		pr_err("%s: bcmbt_tty_init failed\n", __FUNCTION__);
		goto exit_lock_bt_wake;
	}

	bcmbt_data.gpio_bt_wake = pdata->gpio_bt_wake;
	return 0;

exit_lock_bt_wake:
#ifdef CONFIG_HAS_WAKELOCK
	wake_lock_destroy(&bcmbt_data.bt_wake_lock);
#endif

exit_gpio_bt_wake:
	gpio_free((unsigned) pdata->gpio_bt_wake);
	
	return rc;
}

static void clean_bt_wake(struct bcmbt_lpm_ldisc_platform_data *pdata)
{
	if(pdata->gpio_bt_wake == -1)
		return;

#ifdef CONFIG_HAS_WAKELOCK	
		wake_lock_destroy(&bcmbt_data.bt_wake_lock);
#endif
	gpio_free((unsigned) pdata->gpio_bt_wake);
	
	bcmbt_tty_cleanup();
}

static int init_host_wake(struct bcmbt_lpm_ldisc_platform_data *pdata)
{
	int rc;

	if(pdata->gpio_host_wake < 0)
		return 0;

	if(rc = gpio_request(pdata->gpio_host_wake, "BT Host Power Mgmt"))
	{
		pr_err("%s: failed to configure BT Host Power Mgmt: gpio_request err=%d\n", __FUNCTION__, rc);
		return rc;
	}

#ifdef CONFIG_HAS_WAKELOCK
	wake_lock_init(&bcmbt_data.host_wake_lock, WAKE_LOCK_SUSPEND, "HOSTWAKE");
#endif

	/* make sure host_wake is pulled into the right direction if BT chips is NOT powered to avoid
	 * wake lock being blocked accidentally! The value of HOST_WAKE_DEASSERT gives the direction
	 * to pull to. */
	if((rc = bcmbt_data.host_irq = gpio_to_irq(pdata->gpio_host_wake)) < 0)
	{
		pr_err("%s: failed to configure BT Host Power Mgmt: gpio_to_irq err=%d\n", __FUNCTION__, rc);
		goto exit_lock_host_wake;
	}
	pr_info("%s: host wake irq=%d \n",  __FUNCTION__, bcmbt_data.host_irq);

	if(rc = request_irq(bcmbt_data.host_irq, host_wake_isr,
							( IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING | IRQF_NO_SUSPEND ),
							"bt_host_wake", NULL))
	{
		pr_err("%s: failed to configure BT Host Power Mgmt: request_irq err=%d\n", __FUNCTION__, rc);
		goto exit_gpio_to_irq;
	}
	bcmbt_data.gpio_host_wake = pdata->gpio_host_wake;
	return 0;

exit_gpio_to_irq:
	free_irq(bcmbt_data.host_irq, host_wake_isr);
	
exit_lock_host_wake:
#ifdef CONFIG_HAS_WAKELOCK
	wake_lock_destroy(&bcmbt_data.host_wake_lock);
#endif
	gpio_free((unsigned) pdata->gpio_host_wake);
	return rc;
}

static void clean_host_wake(struct bcmbt_lpm_ldisc_platform_data *pdata)
{
	if(pdata->gpio_host_wake == -1)
		return;

	if(bcmbt_data.host_irq >= 0)
		free_irq(bcmbt_data.host_irq, host_wake_isr);
#ifdef CONFIG_HAS_WAKELOCK	
		wake_lock_destroy(&bcmbt_data.host_wake_lock);
#endif
	gpio_free((unsigned) pdata->gpio_host_wake);
}

static int bcmbt_lpm_probe(struct platform_device *pdev)
{
	int rc = 0;
	struct bcmbt_lpm_ldisc_platform_data *pdata = pdev->dev.platform_data;

	bcmbt_data.gpio_bt_wake = -1;
	bcmbt_data.gpio_host_wake = -1;
	bcmbt_data.host_irq = -1;

	if(!pdata)
	{
		pr_err("%s: platform data is not set\n", __FUNCTION__);
		return -ENODEV;
	}

	pr_info("%s: gpio_bt_wake=%d, gpio_host_wake=%d\n", __FUNCTION__, pdata->gpio_bt_wake, pdata->gpio_host_wake);
	
	if(rc = init_bt_wake(pdata))
		return rc;

	if(rc = init_host_wake(pdata))
	{
		clean_bt_wake(pdata);
		return rc;
	}
	return 0;
}

static int bcmbt_lpm_remove(struct platform_device *pdev)
{
	struct bcmbt_lpm_ldisc_platform_data *pdata = pdev->dev.platform_data;
	
	if(pdata)
	{
		clean_bt_wake(pdata);
		clean_host_wake(pdata);
	}
	return 0;
}

static struct platform_driver bcmbt_lpm_platform_driver = {
	.probe = bcmbt_lpm_probe,
	.remove = bcmbt_lpm_remove,
	.driver = {
		.name = "bcmbt-lpm-ldisc",
		.owner = THIS_MODULE,
	},
};

static int __init bcmbt_lpm_init(void)
{
	int rc = platform_driver_register(&bcmbt_lpm_platform_driver);
	if(rc)
		pr_err("bcmbt_lpm_platform_driver_register failed err=%d\n", rc);
	else
		pr_info("bcmbt_lpm_init success\n");

	return rc;
}

static void __exit bcmbt_lpm_exit(void)
{
	platform_driver_unregister(&bcmbt_lpm_platform_driver);
}

module_init(bcmbt_lpm_init);
module_exit(bcmbt_lpm_exit);

MODULE_ALIAS_LDISC(N_BRCM_HCI);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Broadcom");
