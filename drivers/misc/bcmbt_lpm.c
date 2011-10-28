/*
 *  linux/drivers/misc/bcmbt_lpm
 *
 *  Driver for brcm bt wake handling
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
* 	@file	drivers/serial/brcm_bt_lpm.c
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

#if defined(CONFIG_SERIAL_8250_CONSOLE) && defined(CONFIG_MAGIC_SYSRQ)
#define SUPPORT_SYSRQ
#endif

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/tty.h>
#include <linux/ioport.h>
#include <linux/init.h>
#include <linux/console.h>
#include <linux/sysrq.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/serial_reg.h>
#include <linux/serial_core.h>
#include <linux/serial.h>
#include <linux/serial_8250.h>
#include <linux/nmi.h>
#include <linux/mutex.h>
#ifdef CONFIG_HAS_WAKELOCK
#include <linux/wakelock.h>
#endif
#include <linux/clk.h>

#include <asm/io.h>
#include <asm/irq.h>
#include <asm/uaccess.h>

/*#include "8250.h"*/

#include <mach/gpio.h>
#include <linux/broadcom/bcmbt_lpm.h>


/* pull up/down gpio depending on functionality */
extern int bcm_gpio_pull_up(unsigned int gpio, bool up);
extern int bcm_gpio_pull_up_down_enable(unsigned int gpio, bool enable);

#ifdef CONFIG_BCM_BT_LPM

static struct bcm_bt_lpm_data bcm_bt_lpm_data = {
        0,
};


static int brcm_assert_bt_wake(struct uart_port *port)
{
    /* struct tty_port *port = &state->port; */

    /* TODO: make gpio number depending on uart number in case multiple chips are connected to multiple ports! */
    gpio_set_value(bcm_bt_lpm_data.gpio_bt_wake, BT_WAKE_ASSERT);

    /* block host from sleeping till all tx is done */
#ifdef CONFIG_HAS_WAKELOCK
    wake_lock(&bcm_bt_lpm_data.bt_wake_lock);
#endif

    pr_debug("bt_wake assert: gpio: %d, %d\n", bcm_bt_lpm_data.gpio_bt_wake, BT_WAKE_ASSERT);
    return 0;
}

static int brcm_deassert_bt_wake(struct uart_port *port)
{
    /* struct tty_port *port = &state->port; */

    /* TODO: make gpio number depending on uart number in case multiple chips are connected to multiple ports! */
    gpio_set_value(bcm_bt_lpm_data.gpio_bt_wake, BT_WAKE_DEASSERT);

    /* release lock: enable deep sleep */
#ifdef CONFIG_HAS_WAKELOCK
    wake_unlock(&bcm_bt_lpm_data.bt_wake_lock);
#endif

    pr_debug("bt_wake DEassert: gpio: %d, %d\n", bcm_bt_lpm_data.gpio_bt_wake, BT_WAKE_DEASSERT);
    return 0;
}

static int brcm_get_bt_wake_state(struct uart_port *port,
				  unsigned long __user *retinfo)
{
	/*struct tty_port *port = &state->port; */
	unsigned long tmp;

	tmp = gpio_get_value(bcm_bt_lpm_data.gpio_bt_wake);

	pr_info("brcm_get_bt_wake_state(bt_wake:%d) \n",bcm_bt_lpm_data.gpio_bt_wake);

	if (copy_to_user(retinfo, &tmp, sizeof(*retinfo)))
		return -EFAULT;
	return 0;
}

int serial8250_ioctl(struct uart_port *port, unsigned int cmd,unsigned long arg)
{
	void __user *uarg = (void __user *)arg;
	int ret = -ENOIOCTLCMD;

	pr_debug("serial8250_ioctl(cmd: x%u)\n", cmd);
	switch (cmd) {
	case TIO_ASSERT_BT_WAKE:
		ret = brcm_assert_bt_wake(port);
		break;

	case TIO_DEASSERT_BT_WAKE:
		ret = brcm_deassert_bt_wake(port);
		break;

	case TIO_GET_BT_WAKE_STATE:
		ret = brcm_get_bt_wake_state(port, uarg);
		break;
	}

	return ret;
}


int brcm_init_bt_wake(struct bcm_bt_lpm_platform_data *gpio_data)
{

    if ( 0==bcm_bt_lpm_data.bt_wake_installed )
    {
        pr_info("brcm_init_bt_wake( gpio_bt_wake: %d )", gpio_data->gpio_bt_wake);

        bcm_bt_lpm_data.gpio_bt_wake = gpio_data->gpio_bt_wake;
        bcm_bt_lpm_data.gpio_host_wake = gpio_data->gpio_host_wake;

#ifdef CONFIG_HAS_WAKELOCK
        wake_lock_init(&bcm_bt_lpm_data.bt_wake_lock, WAKE_LOCK_SUSPEND, "BTWAKE");
#endif
        bcm_bt_lpm_data.bt_wake_installed = 1;
    }
    else
    {
        pr_info("brcm_init_bt_wake( gpio_bt_wake: %d )::already installed", gpio_data->gpio_bt_wake);
        return 0;
    }
	if (brcm_init_hostwake(gpio_data)) {
	    pr_info("host_wake_isr installation failed \n");
	}
	return 0;
}

static irqreturn_t host_wake_isr(int irq, void *dev)
{
	unsigned int host_wake;
	/* unsigned long irqflags; */

	/* spin_lock_irqsave(&bcm_bt_lpm_data.lock, irqflags); */

	host_wake = gpio_get_value(bcm_bt_lpm_data.gpio_host_wake);

	if (HOST_WAKE_ASSERT == host_wake)
	{
        /* hold lock: disable deep sleep */
#ifdef CONFIG_HAS_WAKELOCK
		wake_lock(&bcm_bt_lpm_data.host_wake_lock);
#endif
		pr_debug( "host_wake assert, hold lock at %lld\n",
		          ktime_to_ns(ktime_get()) );
	}
	else
	{
	    /* release lock: enable deep sleep */
#ifdef CONFIG_HAS_WAKELOCK
	    wake_unlock(&bcm_bt_lpm_data.host_wake_lock);
#endif
        pr_debug( "host_wake DEassert, release lock  at %lld\n",
                  ktime_to_ns(ktime_get()) );
	}

	/* spin_unlock_irqrestore(&bcm_bt_lpm_data.lock, irqflags); */

	return IRQ_HANDLED;
}


int brcm_init_hostwake(struct bcm_bt_lpm_platform_data *gpio_data)
{
	unsigned int irq;
	int ret;

	if (bcm_bt_lpm_data.host_wake_installed) {
		pr_info("host wake irq is already installed \n");
		return 0;
	} else
	    bcm_bt_lpm_data.host_wake_installed = 1;

#ifdef CONFIG_HAS_WAKELOCK
	wake_lock_init(&bcm_bt_lpm_data.host_wake_lock, WAKE_LOCK_SUSPEND, "HOSTWAKE");
#endif
	/* make sure host_wake is pulled into the right direction if BT chips is NOT powered to avoid
	 * wake lock being blocked accidentally! The value of HOST_WAKE_DEASSERT gives the direction
	 * to pull to. */
	irq = gpio_to_irq(gpio_data->gpio_host_wake);

	pr_info("host wake irq=%d \n", irq);

	ret = request_irq(irq, host_wake_isr,
			          ( IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING | IRQF_NO_SUSPEND ),
			          "bt_host_wake", NULL);

	pr_info("request_irq returned value=%u \n", ret);

		return ret;
}

static int bcmbt_lpm_probe(struct platform_device *pdev)
{
     struct bcm_bt_lpm_platform_data *pdata;

	pdata=pdev->dev.platform_data;
    	brcm_init_bt_wake(pdata); 
	return 0;
}

static int bcmbt_lpm_remove(struct platform_device *pdev)
{
        struct bcm_bt_lpm_platform_data *pdata = pdev->dev.platform_data;

        /* Free the GPIO resources */
        gpio_free(pdata->gpio_bt_wake);
        gpio_free(pdata->gpio_host_wake);
	//free_irq(irq,host_wake_isr);
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
