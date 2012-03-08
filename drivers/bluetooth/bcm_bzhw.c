/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* @file	drivers/bluetooth/brcm_bzhw.c
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
#include <linux/gpio.h>
#include <mach/gpio.h>
#include <linux/broadcom/bcm_bzhw.h>
#include <linux/platform_device.h>
#include <linux/sched.h>

#include <linux/interrupt.h>
#include <asm/irq.h>
#include <linux/uaccess.h>
#include <linux/tty.h>

#ifdef CONFIG_KONA_PI_MGR
#include <mach/pi_mgr.h>
#include <plat/pi_mgr.h>
#endif

#ifdef CONFIG_BCM_BZHW

#define BZHW_CLOCK_ENABLE 1
#define BZHW_CLOCK_DISABLE 0

#ifndef BZHW_BT_WAKE_ASSERT
#define BZHW_BT_WAKE_ASSERT 1
#endif
#ifndef BZHW_BT_WAKE_DEASSERT
#define BZHW_BT_WAKE_DEASSERT (!(BZHW_BT_WAKE_ASSERT))
#endif

#ifndef BZHW_HOST_WAKE_ASSERT
#define BZHW_HOST_WAKE_ASSERT 1
#endif
#ifndef BZHW_HOST_WAKE_DEASSERT
#define BZHW_HOST_WAKE_DEASSERT (!(BZHW_HOST_WAKE_ASSERT))
#endif

static struct bcm_bzhw_data bzhw_data = { 0, };
static struct pi_mgr_qos_node qos_node;
static struct uart_port *uport;
static int qos_bt = -1;
static int close = -1;
void serial8250_togglerts(struct uart_port *port, unsigned int flowon);

static int bcm_bzhw_init_clock(void)
{
	qos_bt =
	    pi_mgr_qos_add_request(&qos_node, "bt_qos_node",
				   PI_MGR_PI_ID_ARM_SUB_SYSTEM,
				   PI_MGR_QOS_DEFAULT_VALUE);
	if (qos_bt < 0) {
		pr_err
	("BLUETOOTH:In bcm_bzhw_init_clock bt_qosNode addition failed\n");
		return -1;
	}
	pr_err("BLUETOOTH:In bcm_bzhw_init_clock bt_qosNode SUCCESFULL\n");
	return 0;
}

void bcm_bzhw_request_clock_on(struct uart_port *port)
{
	int ret = -1;
	uport = port;
	close = -1;
	pr_debug("%s: bcm_bzhw_request_clock_on\n", __func__);
	ret = pi_mgr_qos_request_update(&qos_node, 0);
	if (ret == 0)
		pr_err
	("BLUETOOTH:In bcm_bzhw_request_clock_on UART OUT of retention\n");
	else
		pr_err
	("BLUETOOTH:In bcm_bzhw_clock_on UART OUT OF retention FAILED\n");

}
void bcm_bzhw_request_clock_off(struct uart_port *port)
{
	int ret = -1;
	uport = port;
	close = 0;
	pr_debug("%s: bcm_bzhw_request_clock_off\n", __func__);
	ret = pi_mgr_qos_request_update(&qos_node, PI_MGR_QOS_DEFAULT_VALUE);
	if (ret == 0)
		pr_err
	("BLUETOOTH:In bcm_bzhw_request_clock_off UART in retention\n");
	else
		pr_err
	("BLUETOOTH:In bcm_bzhw_request_clock_off UART NOT in retention\n");

}

int bcm_bzhw_assert_bt_wake(void)
{
	if (bzhw_data.gpio_bt_wake == -1) {
		pr_err("%s: gpio_bt_wake=%d\n", __func__,
		       bzhw_data.gpio_bt_wake);
		return -EFAULT;
	}

	gpio_set_value(bzhw_data.gpio_bt_wake, BZHW_BT_WAKE_ASSERT);
	pr_debug("BLUETOOTH:ASSERT BT_WAKE\n");
	bcm_bzhw_request_clock_on(uport);
#if 0
#ifdef CONFIG_HAS_WAKELOCK
	wake_lock(&bzhw_data.bt_wake_lock);
#endif
#endif
	return 0;
}

int bcm_bzhw_deassert_bt_wake(void)
{
	if (bzhw_data.gpio_bt_wake == -1) {
		pr_err("%s: gpio_bt_wake=%d\n", __func__,
		       bzhw_data.gpio_bt_wake);
		return -EFAULT;
	}

	gpio_set_value(bzhw_data.gpio_bt_wake, BZHW_BT_WAKE_DEASSERT);
	pr_debug("BLUETOOTH:DEASSERT BT_WAKE\n");
	if ((gpio_get_value(bzhw_data.gpio_host_wake)
	     == BZHW_HOST_WAKE_DEASSERT)) {
		pr_err
	("BLUETOOTH:DEASSERT BT_WAKEshut UART clock since no activity\n");
		pi_mgr_qos_request_update(&qos_node, PI_MGR_QOS_DEFAULT_VALUE);

	}
#if 0
#ifdef CONFIG_HAS_WAKELOCK
	wake_unlock(&bzhw_data.bt_wake_lock);
#endif
#endif
	return 0;
}

static int bcm_bzhw_init_bt_wake(struct bcm_bzhw_platform_data *pdata)
{
	int rc;

	if (pdata->gpio_bt_wake < 0) {
		pr_err("%s: gpio_bt_wake=%d\n", __func__, pdata->gpio_bt_wake);
		return 0;
	}

	rc = gpio_request(pdata->gpio_bt_wake, "BT Power Mgmt");
	if (rc) {
		pr_err
	("%s: failed to configure BT Power Mgmt: gpio_request err%d\n",
		     __func__, rc);
		return rc;
	}

	rc = gpio_direction_output(pdata->gpio_bt_wake, BZHW_BT_WAKE_DEASSERT);

#ifdef CONFIG_HAS_WAKELOCK
	wake_lock_init(&bzhw_data.bt_wake_lock, WAKE_LOCK_SUSPEND, "BTWAKE");
#endif

	bzhw_data.gpio_bt_wake = pdata->gpio_bt_wake;
	return 0;

}

static void bcm_bzhw_clean_bt_wake(struct bcm_bzhw_platform_data *pdata)
{
	if (pdata->gpio_bt_wake < 0)
		return;

#ifdef CONFIG_HAS_WAKELOCK
	wake_lock_destroy(&bzhw_data.bt_wake_lock);
#endif
	gpio_free((unsigned)pdata->gpio_bt_wake);

}

static irqreturn_t bcm_bzhw_host_wake_isr(int irq, void *dev)
{
	unsigned int host_wake;
	int ret = -1;
	host_wake = gpio_get_value(bzhw_data.gpio_host_wake);

	if (close == 0) {
		pr_err("BLUETOOTH: chip in reset ignore isr\n");
	} else {
		if (BZHW_HOST_WAKE_ASSERT == host_wake) {
			/* wake up peripheral clock */
			if (uport != NULL)
				serial8250_togglerts(uport, 1);
#ifdef CONFIG_HAS_WAKELOCK
			wake_lock(&bzhw_data.host_wake_lock);
#endif
			ret = pi_mgr_qos_request_update(&qos_node, 0);
		} else {
			/* release lock: enable deep sleep */
			if (uport != NULL)
				serial8250_togglerts(uport, 0);
#ifdef CONFIG_HAS_WAKELOCK
			wake_unlock(&bzhw_data.host_wake_lock);
#endif
			ret =
			    pi_mgr_qos_request_update(&qos_node,
						      PI_MGR_QOS_DEFAULT_VALUE);
		}
	}
	return IRQ_HANDLED;
}

static int bcm_bzhw_init_hostwake(struct bcm_bzhw_platform_data *gpio_data)
{
	int rc;
	int ret;

	if (gpio_data->gpio_host_wake < 0)
		return 0;
	rc = gpio_request(gpio_data->gpio_host_wake, "BT Host Power Mgmt");
	if (rc) {
		pr_err
	    ("%s: failed to configure BT Host Mgmt: gpio_request err=%d\n",
		     __func__, rc);
		return rc;
	}
#ifdef CONFIG_HAS_WAKELOCK
	wake_lock_init(&bzhw_data.host_wake_lock, WAKE_LOCK_SUSPEND,
		       "HOSTWAKE");
#endif
	bcm_bzhw_init_clock();

	rc = bzhw_data.host_irq = gpio_to_irq(gpio_data->gpio_host_wake);
	if (rc < 0) {
		pr_err
	    ("%s: failed to configure BT Host Mgmt: gpio_to_irq err=%d\n",
		     __func__, rc);
		goto exit_lock_host_wake;
	}

	pr_debug("host wake irq=%d\n", bzhw_data.host_irq);

	rc = request_irq(bzhw_data.host_irq, bcm_bzhw_host_wake_isr,
			 (IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING |
			  IRQF_NO_SUSPEND), "bt_host_wake", NULL);
	if (rc) {
		pr_err
	    ("%s: failed to configure BT Host Mgmt: request_irq err=%d\n",
		     __func__, rc);
		goto exit_gpio_to_irq;
	}

	pr_debug("BLUETOOTH:brcm_bthw request_irq returned value=%u\n", ret);

	bzhw_data.gpio_host_wake = gpio_data->gpio_host_wake;

	return 0;

exit_gpio_to_irq:
	free_irq(bzhw_data.host_irq, bcm_bzhw_host_wake_isr);

exit_lock_host_wake:
#ifdef CONFIG_HAS_WAKELOCK
	wake_lock_destroy(&bzhw_data.host_wake_lock);
#endif
	gpio_free((unsigned)gpio_data->gpio_host_wake);
	return rc;

}

static void bcm_bzhw_clean_host_wake(struct bcm_bzhw_platform_data *pdata)
{
	if (pdata->gpio_host_wake == -1)
		return;
	if (bzhw_data.host_irq >= 0)
		free_irq(bzhw_data.host_irq, bcm_bzhw_host_wake_isr);

#ifdef CONFIG_HAS_WAKELOCK
	wake_lock_destroy(&bzhw_data.host_wake_lock);
#endif
	gpio_free((unsigned)pdata->gpio_host_wake);
#ifdef CONFIG_HAS_WAKELOCK
	wake_lock_destroy(&bzhw_data.host_wake_lock);
#endif
	gpio_free((unsigned)pdata->gpio_host_wake);
}

static int bcm_bzhw_probe(struct platform_device *pdev)
{
	struct bcm_bzhw_platform_data *pdata;
	int rc = 0;

	pdata = pdev->dev.platform_data;
	bzhw_data.gpio_bt_wake = -1;
	bzhw_data.gpio_host_wake = -1;
	bzhw_data.host_irq = -1;
	if (!pdata) {
		pr_err("%s: platform data is not set\n", __func__);
		return -ENODEV;
	}
	pr_info("%s: gpio_bt_wake=%d, gpio_host_wake=%d\n", __func__,
		pdata->gpio_bt_wake, pdata->gpio_host_wake);

	uport = NULL;
	bcm_bzhw_init_bt_wake(pdata);
	rc = bcm_bzhw_init_hostwake(pdata);
	if (rc)
		return rc;

	return 0;
}

static int bcm_bzhw_remove(struct platform_device *pdev)
{
	struct bcm_bzhw_platform_data *pdata = pdev->dev.platform_data;

	if (pdata) {
		bcm_bzhw_clean_bt_wake(pdata);
		bcm_bzhw_clean_host_wake(pdata);
		if (pi_mgr_qos_request_remove(&qos_node))
			pr_info("failed to unregister qos client\n");
	}
	return 0;
}

static struct platform_driver bcm_bzhw_platform_driver = {
	.probe = bcm_bzhw_probe,
	.remove = bcm_bzhw_remove,
	.driver = {
		   .name = "bcm_bzhw",
		   .owner = THIS_MODULE,
		   },
};

static int __init bcm_bzhw_init(void)
{
	int rc = platform_driver_register(&bcm_bzhw_platform_driver);
	if (rc)
		pr_err("bcm_bzhw_platform_driver_register failed err=%d\n", rc);
	else
		pr_info("bcm_bzhw_init success\n");

	return rc;
}

static void __exit bcm_bzhw_exit(void)
{
	platform_driver_unregister(&bcm_bzhw_platform_driver);
}

module_init(bcm_bzhw_init);
module_exit(bcm_bzhw_exit);

MODULE_DESCRIPTION("bcm_bzhw");
MODULE_AUTHOR("Broadcom");
MODULE_LICENSE("GPL");
#endif
