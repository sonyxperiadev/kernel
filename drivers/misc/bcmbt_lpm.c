/*
 *  linux/drivers/misc/bcmbt_lpm
 *
 *  Driver for bcm bt wake handling
 *
 *  Based on drivers/char/serial.c, by Linus Torvalds, Theodore Ts'o.
 *
 *  Copyright (C) 2001 Russell King.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * A note about mapbase / membase
 *
 *  mapbase is the physical address of the IO port.
 *  membase is an 'ioremapped' cookie.
 */

/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	drivers/serial/bcm_bt_lpm.c
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

#include <linux/module.h>
#include <linux/init.h>
#include <asm/gpio.h>
#include <mach/gpio.h>
#include <linux/broadcom/bcmbt_lpm.h>
#include <linux/platform_device.h>
#include <linux/sched.h>

#include <linux/serial_core.h>
#include <linux/tty.h>

#include <asm/uaccess.h>
#ifdef CONFIG_HAS_WAKELOCK
#include <linux/wakelock.h>
#endif
#include <linux/clk.h>

#include <asm/io.h>
#include <asm/irq.h>
#include <asm/uaccess.h>

#ifdef CONFIG_KONA_PI_MGR
#include <mach/pi_mgr.h>
#include <plat/pi_mgr.h>
#endif

#ifdef CONFIG_BCM_BT_LPM

#define BTLPM_ENABLE_CLOCK 1
#define BTLPM_DISABLE_CLOCK 0

static struct tty_ldisc_ops bcmbt_ldisc_ops;
static struct bcm_bt_lpm_data bcm_bt_lpm_data = {
	0,
};

static struct pi_mgr_qos_node btqos_node;

static int bcm_assert_bt_wake()
{
	if (bcm_bt_lpm_data.gpio_bt_wake == -1)
		return -EFAULT;

	gpio_set_value(bcm_bt_lpm_data.gpio_bt_wake, BT_WAKE_ASSERT);

#ifdef CONFIG_HAS_WAKELOCK
	wake_lock(&bcm_bt_lpm_data.bt_wake_lock);
#endif

	pr_debug("bt_wake assert: gpio: %d, %d\n", bcm_bt_lpm_data.gpio_bt_wake,
		 BT_WAKE_ASSERT);

	return 0;
}

static int bcm_deassert_bt_wake()
{
	if (bcm_bt_lpm_data.gpio_bt_wake == -1)
		return -EFAULT;

	gpio_set_value(bcm_bt_lpm_data.gpio_bt_wake, BT_WAKE_DEASSERT);

	/* release lock: enable deep sleep */
#ifdef CONFIG_HAS_WAKELOCK
	wake_unlock(&bcm_bt_lpm_data.bt_wake_lock);
#endif

	pr_debug("bt_wake DEassert: gpio: %d, %d\n",
		 bcm_bt_lpm_data.gpio_bt_wake, BT_WAKE_DEASSERT);
	return 0;
}

static int bcm_get_bt_wake_state(unsigned long __user * retinfo)
{

	unsigned long tmp;

	if (bcm_bt_lpm_data.gpio_bt_wake == -1)
		return -EFAULT;

	tmp = gpio_get_value(bcm_bt_lpm_data.gpio_bt_wake);

	pr_info("bcm_get_bt_wake_state(bt_wake:%d) \n",
		bcm_bt_lpm_data.gpio_bt_wake);

	if (copy_to_user(retinfo, &tmp, sizeof(*retinfo)))
		return -EFAULT;
	return 0;
}

static int bcmbt_init_peripheral_clock(void)
{
	int qos_btlpm = pi_mgr_qos_add_request(&btqos_node, "bcmbt_qos_node",
					       PI_MGR_PI_ID_ARM_SUB_SYSTEM,
					       PI_MGR_QOS_DEFAULT_VALUE);
	if (qos_btlpm < 0)
		return -1;

	return 0;
}

static int bcmbt_release_peripheral_clock(void)
{
	pi_mgr_qos_request_update(&btqos_node, PI_MGR_QOS_DEFAULT_VALUE);
	return 0;
}

static int bcmbt_tty_ioctl(struct tty_struct *tty, struct file *file,
			   unsigned int cmd, unsigned long arg)
{
	int rc = -1;

	switch (cmd) {
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

static void bcmbt_peripheral_clocks(int enable)
{

	if (enable) {
		pi_mgr_qos_request_update(&btqos_node, 0);
#ifdef CONFIG_HAS_WAKELOCK
		wake_lock(&bcm_bt_lpm_data.host_wake_lock);
#endif

	} else {
		pi_mgr_qos_request_update(&btqos_node,
					  PI_MGR_QOS_DEFAULT_VALUE);
		/* release lock: enable deep sleep */
#ifdef CONFIG_HAS_WAKELOCK
		wake_unlock(&bcm_bt_lpm_data.host_wake_lock);
#endif

	}

}

static irqreturn_t host_wake_isr(int irq, void *dev)
{
	unsigned int host_wake;
	/* unsigned long irqflags; */

	/* spin_lock_irqsave(&bcm_bt_lpm_data.lock, irqflags); */

	host_wake = gpio_get_value(bcm_bt_lpm_data.gpio_host_wake);

	if (HOST_WAKE_ASSERT == host_wake) {
		/* hold lock: disable deep sleep */
		bcmbt_peripheral_clocks(BTLPM_ENABLE_CLOCK);
		pr_debug("host_wake assert, hold lock at %lld\n",
			 ktime_to_ns(ktime_get()));
	} else {
		/* release lock: enable deep sleep */
		bcmbt_peripheral_clocks(BTLPM_DISABLE_CLOCK);
#ifdef CONFIG_HAS_WAKELOCK
		wake_unlock(&bcm_bt_lpm_data.host_wake_lock);
#endif
		pi_mgr_qos_request_update(&btqos_node,
					  PI_MGR_QOS_DEFAULT_VALUE);
		pr_debug("host_wake DEassert, release lock  at %lld\n",
			 ktime_to_ns(ktime_get()));
	}

	return IRQ_HANDLED;
}

int bcm_init_hostwake(struct bcm_bt_lpm_platform_data *gpio_data)
{
	unsigned int irq;
	int ret;
	if (bcm_bt_lpm_data.host_wake_installed) {
		pr_info("host wake irq is already installed \n");
		return 0;
	} else
		bcm_bt_lpm_data.host_wake_installed = 1;

#ifdef CONFIG_HAS_WAKELOCK
	wake_lock_init(&bcm_bt_lpm_data.host_wake_lock, WAKE_LOCK_SUSPEND,
		       "HOSTWAKE");
#endif
	/* make sure host_wake is pulled into the right direction if BT chips is NOT powered to avoid
	 * wake lock being blocked accidentally! The value of HOST_WAKE_DEASSERT gives the direction
	 * to pull to. */
	irq = gpio_to_irq(gpio_data->gpio_host_wake);

	pr_info("host wake irq=%d \n", irq);

	ret = request_irq(irq, host_wake_isr,
			  (IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING |
			   IRQF_NO_SUSPEND), "bt_host_wake", NULL);

	pr_info("request_irq returned value=%u \n", ret);
	bcmbt_init_peripheral_clock();

	return ret;
}

int bcm_init_bt_wake(struct bcm_bt_lpm_platform_data *gpio_data)
{
	int rc;
	if (0 == bcm_bt_lpm_data.bt_wake_installed) {
		pr_info("bcm_init_bt_wake( gpio_bt_wake: %d )",
			gpio_data->gpio_bt_wake);

		bcm_bt_lpm_data.gpio_bt_wake = gpio_data->gpio_bt_wake;
		bcm_bt_lpm_data.gpio_host_wake = gpio_data->gpio_host_wake;

#ifdef CONFIG_HAS_WAKELOCK
		wake_lock_init(&bcm_bt_lpm_data.bt_wake_lock, WAKE_LOCK_SUSPEND,
			       "BTWAKE");
#endif
		/* register the tty line discipline driver */
		if (rc = bcmbt_tty_init()) {
			pr_err("%s: bcmbt_tty_init failed\n", __FUNCTION__);
#ifdef CONFIG_HAS_WAKELOCK
			wake_lock_destroy(&bcm_bt_lpm_data.host_wake_lock);
#endif
			return -1;
		}
		bcm_bt_lpm_data.bt_wake_installed = 1;
	} else {
		pr_info
		    ("bcm_init_bt_wake( gpio_bt_wake: %d )::already installed",
		     gpio_data->gpio_bt_wake);
		return 0;
	}

	if (bcm_init_hostwake(gpio_data)) {
		pr_info("host_wake_isr installation failed \n");
	}
	return 0;
}

static void clean_bt_wake(struct bcm_bt_lpm_platform_data *pdata)
{
	if (pdata->gpio_bt_wake == -1)
		return;

#ifdef CONFIG_HAS_WAKELOCK
	wake_lock_destroy(&bcm_bt_lpm_data.bt_wake_lock);
#endif
	gpio_free((unsigned)pdata->gpio_bt_wake);

	bcmbt_tty_cleanup();
}

static void clean_host_wake(struct bcm_bt_lpm_platform_data *pdata)
{
	if (pdata->gpio_host_wake == -1)
		return;

#ifdef CONFIG_HAS_WAKELOCK
	wake_lock_destroy(&bcm_bt_lpm_data.host_wake_lock);
#endif
	gpio_free((unsigned)pdata->gpio_host_wake);
}

static int bcmbt_lpm_probe(struct platform_device *pdev)
{
	struct bcm_bt_lpm_platform_data *pdata;

	pdata = pdev->dev.platform_data;
	bcm_init_bt_wake(pdata);
	return 0;
}

static int bcmbt_lpm_remove(struct platform_device *pdev)
{
	struct bcm_bt_lpm_platform_data *pdata = pdev->dev.platform_data;

	if (pdata) {
		clean_bt_wake(pdata);
		clean_host_wake(pdata);
	}
	return 0;
}

static struct platform_driver bcmbt_lpm_platform_driver = {
	.probe = bcmbt_lpm_probe,
	.remove = bcmbt_lpm_remove,
	.driver = {
		   .name = "bcmbt-lpm",
		   .owner = THIS_MODULE,
		   },
};

static int __init bcmbt_lpm_init(void)
{
	return platform_driver_register(&bcmbt_lpm_platform_driver);
}

static void __exit bcmbt_lpm_exit(void)
{
	platform_driver_unregister(&bcmbt_lpm_platform_driver);
}

module_init(bcmbt_lpm_init);
module_exit(bcmbt_lpm_exit);

MODULE_DESCRIPTION("bcmbt-lpm");
MODULE_AUTHOR("Broadcom");
MODULE_LICENSE("GPL");
#endif
