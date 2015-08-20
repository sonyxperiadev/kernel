/*
 * Copyright (C) 2015 Sony Mobile Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 */
#include <linux/gpio.h>
#include <linux/skbuff.h>
#include <linux/wlan_plat.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/pinctrl/consumer.h>
#ifdef CONFIG_BCMDHD_PCIE
#include <linux/msm_pcie.h>
#endif /* CONFIG_BCMDHD_PCIE */
#include <linux/mmc/host.h>
#include <linux/platform_device.h>
#include <linux/of_gpio.h>
#include <linux/module.h>
#include <linux/fs.h>
#include "dhd_custom_memprealloc.h"

static char *intf_macaddr = NULL;
static struct mmc_host *wlan_mmc_host;

struct bcmdhd_platform_data {
	struct platform_device *pdev;
	struct pinctrl *pinctrl;
	struct pinctrl_state *gpio_state_active;
	struct pinctrl_state *gpio_state_suspend;
	unsigned int wlan_reg_on;
#ifdef CONFIG_BCMDHD_PCIE
	unsigned int pci_number;
#endif /* CONFIG_BCMDHD_PCIE */
};

static struct bcmdhd_platform_data *bcmdhd_data;

void somc_wifi_mmc_host_register(struct mmc_host *host)
{
        wlan_mmc_host = host;
}

int somc_wifi_set_power(int on)
{
	gpio_set_value(bcmdhd_data->wlan_reg_on, on);
	return 0;
}

int somc_wifi_set_carddetect(int present)
{
#ifdef CONFIG_BCMDHD_PCIE
	int ret = 0;
	if (present)
		ret = msm_pcie_enumerate(bcmdhd_data->pci_number);
	return ret;
#else
	if (wlan_mmc_host)
		mmc_detect_change(wlan_mmc_host, 0);
	return 0;
#endif /* CONFIG_BCMDHD_PCIE */
}

static ssize_t macaddr_show(struct device *dev, struct device_attribute *attr,
				char *buf)
{
	return sprintf(buf, "%s", intf_macaddr);
}

static ssize_t macaddr_store(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count)
{
	return snprintf(intf_macaddr, count, "%s\n", buf);
}

DEVICE_ATTR(macaddr, 0644, macaddr_show, macaddr_store);

static struct attribute *wifi_attrs[] = {
	&dev_attr_macaddr.attr,
	NULL
};

static struct attribute_group wifi_attr_grp = {
	.attrs = wifi_attrs,
};

int somc_wifi_init(struct platform_device *pdev)
{
	int ret, ret_sus, gpio;
	struct pinctrl *pinctrl;
	struct pinctrl_state *lookup_state;

	bcmdhd_data = kzalloc(sizeof(*bcmdhd_data), GFP_KERNEL);
	if (!bcmdhd_data) {
		dev_err(&pdev->dev, "%s: no memory\n", __func__);
		ret = -ENOMEM;
		goto err_alloc_bcmdhd_data;
	}

	bcmdhd_data->pdev = pdev;
	pinctrl = devm_pinctrl_get(&pdev->dev);
	if (IS_ERR(pinctrl)) {
		dev_err(&pdev->dev, "%s: pinctrl not defined\n", __func__);
		ret = PTR_ERR(pinctrl);
		goto err_pinctrl;
	}
	bcmdhd_data->pinctrl = pinctrl;

	lookup_state = pinctrl_lookup_state(pinctrl, PINCTRL_STATE_DEFAULT);
	if (IS_ERR_OR_NULL(lookup_state)) {
		dev_err(&pdev->dev, "%s: pinctrl lookup failed for default\n",
			__func__);
		ret = PTR_ERR(lookup_state);
		goto err_pinctrl;
	}
	bcmdhd_data->gpio_state_active = lookup_state;

	lookup_state = pinctrl_lookup_state(pinctrl, PINCTRL_STATE_SLEEP);
	if (IS_ERR_OR_NULL(lookup_state)) {
		dev_err(&pdev->dev, "%s: pinctrl lookup failed for sleep\n",
			__func__);
		ret = PTR_ERR(lookup_state);
		goto err_pinctrl;
	}
	bcmdhd_data->gpio_state_suspend = lookup_state;

	ret = pinctrl_select_state(bcmdhd_data->pinctrl,
		bcmdhd_data->gpio_state_active);
	if (ret) {
		dev_err(&pdev->dev, "%s: failed to select active state\n",
			__func__);
		goto err_pinctrl;
	}

	gpio = of_get_gpio(pdev->dev.of_node, 0);
	if (!gpio_is_valid(gpio)) {
		dev_err(&pdev->dev, "%s: invalid gpio #%s: %d\n",
			__func__, "wlan-reg-on", gpio);
		ret = -ENXIO;
		goto err_gpio;
	}
	bcmdhd_data->wlan_reg_on = gpio;

	ret = gpio_request(bcmdhd_data->wlan_reg_on,
			"wlan-reg-on");
	if (ret) {
		dev_err(&pdev->dev, "%s: request err %s: %d\n",
			__func__, "wlan-reg-on", ret);
		goto err_gpio;
	}

#ifdef CONFIG_BCMDHD_PCIE
	ret = of_property_read_u32(pdev->dev.of_node, "wlan-pci-number",
		&bcmdhd_data->pci_number);
	if (ret < 0) {
		dev_err(&pdev->dev, "%s: failed to find PCI number %d %d \n",
			__func__, ret, bcmdhd_data->pci_number);
		goto err_gpio_request;
	}
#endif /* CONFIG_BCMDHD_PCIE */

	intf_macaddr = kzalloc(20*(sizeof(char)), GFP_KERNEL);
	if (sysfs_create_group(&pdev->dev.kobj, &wifi_attr_grp) < 0) {
		pr_err("%s: Unable to create sysfs\n", __func__);
		kfree(intf_macaddr);
	}

	return 0;

#ifdef CONFIG_BCMDHD_PCIE
err_gpio_request:
	gpio_free(bcmdhd_data->wlan_reg_on);
#endif /* CONFIG_BCMDHD_PCIE */
err_gpio:
	ret_sus = pinctrl_select_state(bcmdhd_data->pinctrl,
			bcmdhd_data->gpio_state_suspend);
	if (ret_sus)
		dev_err(&pdev->dev, "%s: failed to select suspend state\n",
			__func__);
err_pinctrl:
	kzfree(bcmdhd_data);
err_alloc_bcmdhd_data:
	return ret;
}
EXPORT_SYMBOL(somc_wifi_init);

void somc_wifi_deinit(struct platform_device *pdev)
{
	if (bcmdhd_data) {
		if (gpio_is_valid(bcmdhd_data->wlan_reg_on))
			gpio_free(bcmdhd_data->wlan_reg_on);
		if (!IS_ERR_OR_NULL(bcmdhd_data->pinctrl) &&
			!IS_ERR_OR_NULL(bcmdhd_data->gpio_state_suspend)) {
			int ret = pinctrl_select_state(bcmdhd_data->pinctrl,
				bcmdhd_data->gpio_state_suspend);
			if (ret)
				dev_err(&pdev->dev, "%s: failed to select"
					" suspend state\n", __func__);
		}
		kzfree(bcmdhd_data);
	}
}
EXPORT_SYMBOL(somc_wifi_deinit);

#define ETHER_ADDR_LEN    6

static inline int xdigit (char c)
{
	unsigned d;

	d = (unsigned)(c-'0');
	if (d < 10)
		return (int)d;
	d = (unsigned)(c-'a');
	if (d < 6)
		return (int)(10+d);
	d = (unsigned)(c-'A');
	if (d < 6)
		return (int)(10+d);
	return -1;
}

struct ether_addr {
	unsigned char ether_addr_octet[ETHER_ADDR_LEN];
} __attribute__((__packed__));

struct ether_addr *
ether_aton_r (const char *asc, struct ether_addr * addr)
{
	int i, val0, val1;

	for (i = 0; i < ETHER_ADDR_LEN; ++i) {
		val0 = xdigit(*asc);
		asc++;
		if (val0 < 0)
			return NULL;

		val1 = xdigit(*asc);
		asc++;
		if (val1 < 0)
			return NULL;

		addr->ether_addr_octet[i] = (unsigned char)((val0 << 4) + val1);

		if (i < ETHER_ADDR_LEN - 1) {
			if (*asc != ':')
				return NULL;
			asc++;
		}
	}

	if (*asc != '\0')
		return NULL;

	return addr;
}

struct ether_addr * ether_aton (const char *asc)
{
	static struct ether_addr addr;
	return ether_aton_r(asc, &addr);
}

static int somc_wifi_get_mac_addr(unsigned char *buf)
{
	int ret = 0;

	char macasc[128] = {0,};
	uint rand_mac;
	static unsigned char mymac[ETHER_ADDR_LEN] = {0,};
	const unsigned char nullmac[ETHER_ADDR_LEN] = {0,};

	if (buf == NULL)
		return -EAGAIN;

	memset(buf, 0x00, ETHER_ADDR_LEN);

	if (intf_macaddr != NULL) {
		unsigned char* macbin;
		struct ether_addr* convmac = ether_aton( intf_macaddr );

		if (convmac == NULL) {
			pr_err("%s: Invalid Mac Address Format %s\n",
					__FUNCTION__, macasc );
			goto random_mac;
		}

		macbin = convmac->ether_addr_octet;

		pr_info("%s: READ MAC ADDRESS %02X:%02X:%02X:%02X:%02X:%02X\n",
				__FUNCTION__,
				macbin[0], macbin[1], macbin[2],
				macbin[3], macbin[4], macbin[5]);

		memcpy(buf, macbin, ETHER_ADDR_LEN);
	}

	return ret;

random_mac:

	pr_debug("%s: %p\n", __func__, buf);

	if (memcmp( mymac, nullmac, ETHER_ADDR_LEN) != 0) {
		/* Mac displayed from UI is never updated..
		   So, mac obtained on initial time is used */
		memcpy(buf, mymac, ETHER_ADDR_LEN);
		return 0;
	}

	prandom_seed((uint)jiffies);
	rand_mac = prandom_u32();
	buf[0] = 0x00;
	buf[1] = 0x90;
	buf[2] = 0x4c;
	buf[3] = (unsigned char)rand_mac;
	buf[4] = (unsigned char)(rand_mac >> 8);
	buf[5] = (unsigned char)(rand_mac >> 16);

	memcpy(mymac, buf, 6);

	pr_info("[%s] Exiting. MAC %02X:%02X:%02X:%02X:%02X:%02X\n",
			__FUNCTION__,
			buf[0], buf[1], buf[2], buf[3], buf[4], buf[5] );

	return 0;
}

struct wifi_platform_data somc_wifi_control = {
	.set_power	= somc_wifi_set_power,
	.set_carddetect	= somc_wifi_set_carddetect,
	.mem_prealloc	= dhd_wlan_mem_prealloc,
	.get_mac_addr	= somc_wifi_get_mac_addr,
};

EXPORT_SYMBOL(somc_wifi_control);

MODULE_LICENSE("GPL v2");
