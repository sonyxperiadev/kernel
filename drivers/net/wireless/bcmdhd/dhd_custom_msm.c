/*
 * Platform Dependent file for Qualcomm MSM/APQ
 *
 * Copyright (C) 1999-2016, Broadcom Corporation
 * 
 *      Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed to you
 * under the terms of the GNU General Public License version 2 (the "GPL"),
 * available at http://www.broadcom.com/licenses/GPLv2.php, with the
 * following added to such license:
 * 
 *      As a special exception, the copyright holders of this software give you
 * permission to link this software with independent modules, and to copy and
 * distribute the resulting executable under terms of your choice, provided that
 * you also meet, for each linked independent module, the terms and conditions of
 * the license of that module.  An independent module is a module which is not
 * derived from this software.  The special exception does not apply to any
 * modifications of the software.
 * 
 *      Notwithstanding the above, under no circumstances may you combine this
 * software in any way with any other Broadcom software provided under a license
 * other than the GPL, without Broadcom's express prior written consent.
 *
 * $Id: dhd_custom_msm.c 616853 2016-02-03 08:32:47Z $
 *
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/skbuff.h>
#include <linux/wlan_plat.h>
#include <linux/mmc/host.h>
#include <linux/msm_pcie.h>
#include <linux/fcntl.h>
#include <linux/fs.h>
#include <linux/of_gpio.h>

#ifdef CONFIG_BROADCOM_WIFI_RESERVED_MEM
extern int dhd_init_wlan_mem(void);
extern void *dhd_wlan_mem_prealloc(int section, unsigned long size);
#endif /* CONFIG_BROADCOM_WIFI_RESERVED_MEM */

int GPIO_WL_REG_ON = 991; /* ZERO LTE */
#if defined(CONFIG_BCMDHD_PCIE) && defined(CONFIG_BCM4358_MODULE)
extern bool brcm_dev_found;
bool gpio_init_completed = false;
EXPORT_SYMBOL(gpio_init_completed);
#endif /* CONFIG_BCMDHD_PCIE && CONFIG_BCM4358_MODULE */

extern unsigned int system_rev;
int __init
dhd_wifi_init_gpio(void)
{
	int onoff;
	char *wlan_node = "samsung,bcmdhd_wlan";
	struct device_node *root_node = NULL;

	printk("board_rev %x", system_rev);

	root_node = of_find_compatible_node(NULL, NULL, wlan_node);
	if (!root_node) {
		WARN(1, "failed to get device node of BRCM WLAN\n");
		return -ENODEV;
	}

	GPIO_WL_REG_ON = of_get_named_gpio(root_node, "wlan-en-gpio", 0);
	printk(KERN_INFO "%s: gpio_wlan_power : %d\n", __FUNCTION__, GPIO_WL_REG_ON);

	if (gpio_request_one(GPIO_WL_REG_ON, GPIOF_OUT_INIT_LOW, "WL_REG_ON")) {
		printk(KERN_ERR "%s: Faiiled to request gpio %d for WL_REG_ON\n",
			__FUNCTION__, GPIO_WL_REG_ON);
	} else {
		printk(KERN_ERR "%s: gpio_request WL_REG_ON done - WLAN_EN: GPIO %d\n",
			__FUNCTION__, GPIO_WL_REG_ON);
	}

#if defined(CONFIG_BCMDHD_PCIE) && defined(CONFIG_BCM4358_MODULE)
	if (brcm_dev_found) {
		onoff = 0;
	} else {
		onoff = 1;
	}
#else
	onoff = 1;
#endif /* CONFIG_BCMDHD_PCIE && CONFIG_BCM4358_MODULE */

	if (gpio_direction_output(GPIO_WL_REG_ON, onoff)) {
		printk(KERN_ERR "%s: WL_REG_ON failed to pull %s\n",
			__FUNCTION__, onoff ? "up" : "down");
	} else {
		printk(KERN_ERR "%s: WL_REG_ON is pulled %s\n",
			__FUNCTION__, onoff ? "up" : "down");
	}

	if (gpio_get_value(GPIO_WL_REG_ON)) {
		printk(KERN_INFO "%s: Initial WL_REG_ON: [%d]\n",
			__FUNCTION__, gpio_get_value(GPIO_WL_REG_ON));
	}

	if (onoff) {
		/* Wait for 200ms for power stability */
		msleep(200);
	}
#if defined(CONFIG_BCMDHD_PCIE) && defined(CONFIG_BCM4358_MODULE)
	gpio_init_completed = true;
#endif /* CONFIG_BCMDHD_PCIE && CONFIG_BCM4358_MODULE */

	return 0;
}

int
dhd_wlan_power(int onoff)
{
	printk(KERN_INFO"------------------------------------------------");
	printk(KERN_INFO"------------------------------------------------\n");
	printk(KERN_INFO"%s Enter: power %s\n", __func__, onoff ? "on" : "off");

	if (onoff) {
		if (gpio_direction_output(GPIO_WL_REG_ON, 1)) {
			printk(KERN_ERR "%s: WL_REG_ON is failed to pull up\n", __FUNCTION__);
			return -EIO;
		}
		if (gpio_get_value(GPIO_WL_REG_ON)) {
			printk(KERN_INFO"WL_REG_ON on-step-2 : [%d]\n",
				gpio_get_value(GPIO_WL_REG_ON));
		} else {
			printk("[%s] gpio value is 0. We need reinit.\n", __func__);
			if (gpio_direction_output(GPIO_WL_REG_ON, 1)) {
				printk(KERN_ERR "%s: WL_REG_ON is "
					"failed to pull up\n", __func__);
			}
		}
	} else {
		if (gpio_direction_output(GPIO_WL_REG_ON, 0)) {
			printk(KERN_ERR "%s: WL_REG_ON is failed to pull up\n", __FUNCTION__);
			return -EIO;
		}
		if (gpio_get_value(GPIO_WL_REG_ON)) {
			printk(KERN_INFO"WL_REG_ON on-step-2 : [%d]\n",
				gpio_get_value(GPIO_WL_REG_ON));
		}
	}
	return 0;
}
EXPORT_SYMBOL(dhd_wlan_power);

static int
dhd_wlan_reset(int onoff)
{
	return 0;
}

extern int msm_pcie_status_notify(int val);

static int
dhd_wlan_set_carddetect(int val)
{
	return msm_pcie_status_notify(val);
}

struct resource dhd_wlan_resources = {
	.name	= "bcmdhd_wlan_irq",
	.start	= 0, /* Dummy */
	.end	= 0, /* Dummy */
	.flags	= IORESOURCE_IRQ | IORESOURCE_IRQ_SHAREABLE |
#ifdef CONFIG_BCMDHD_PCIE
	IORESOURCE_IRQ_HIGHEDGE,
#else
	IORESOURCE_IRQ_HIGHLEVEL,
#endif /* CONFIG_BCMDHD_PCIE */
};
EXPORT_SYMBOL(dhd_wlan_resources);

struct wifi_platform_data dhd_wlan_control = {
	.set_power	= dhd_wlan_power,
	.set_reset	= dhd_wlan_reset,
	.set_carddetect	= dhd_wlan_set_carddetect,
#ifdef CONFIG_BROADCOM_WIFI_RESERVED_MEM
	.mem_prealloc	= dhd_wlan_mem_prealloc,
#endif /* CONFIG_BROADCOM_WIFI_RESERVED_MEM */
};
EXPORT_SYMBOL(dhd_wlan_control);

int __init
dhd_wlan_init(void)
{
	int ret;

	printk(KERN_INFO"%s: START.......\n", __FUNCTION__);
	ret = dhd_wifi_init_gpio();
	if (ret < 0) {
		printk(KERN_ERR "%s: failed to initiate GPIO, ret=%d\n",
			__FUNCTION__, ret);
		goto fail;
	}

#ifdef CONFIG_BROADCOM_WIFI_RESERVED_MEM
	ret = dhd_init_wlan_mem();
	if (ret < 0) {
		printk(KERN_ERR "%s: failed to alloc reserved memory,"
			" ret=%d\n", __FUNCTION__, ret);
	}
#endif /* CONFIG_BROADCOM_WIFI_RESERVED_MEM */

fail:
	return ret;
}
#if defined(CONFIG_ARCH_MSM8996)
#if defined(CONFIG_DEFERRED_INITCALLS)
deferred_module_init(dhd_wlan_init);
#else
late_initcall(dhd_wlan_init);
#endif /* CONFIG_DEFERRED_INITCALLS */
#else
device_initcall(dhd_wlan_init);
#endif /* CONFIG_ARCH_MSM8996 */
