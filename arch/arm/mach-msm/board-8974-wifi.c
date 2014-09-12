/* arch/arm/mach-msm/board-8974-wifi.c
 *
 * Copyright (C) 2013 Sony Mobile Communications AB.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include <asm/gpio.h>
#include <asm/mach/mmc.h>
#include <linux/qpnp/pin.h>
#include <linux/regulator/consumer.h>
#include <linux/wlan_plat.h>

#include "board-8974-wifi.h"

static int shinano_wifi_cd;
static void (*wifi_status_cb)(int card_present, void *dev_id);
static void *wifi_status_cb_devid;
static struct regulator *wifi_batfet;
static int batfet_ena;

#define WIFI_POWER_PMIC_GPIO 18
#define WIFI_IRQ_GPIO 67

int shinano_wifi_set_power(int on)
{
	int gpio = qpnp_pin_map("pm8941-gpio", WIFI_POWER_PMIC_GPIO);
	if (!wifi_batfet) {
		wifi_batfet = regulator_get(NULL, "batfet");
		if (IS_ERR_OR_NULL(wifi_batfet)) {
			printk(KERN_ERR "unable to get batfet reg. rc=%d\n",
				PTR_RET(wifi_batfet));
			wifi_batfet = NULL;
		}
	}
	if (on) {
		if (!batfet_ena && wifi_batfet) {
			regulator_enable(wifi_batfet);
			batfet_ena = 1;
		}
	}
	gpio_set_value(gpio, on);
	if (!on) {
		if (batfet_ena && wifi_batfet) {
			regulator_disable(wifi_batfet);
			batfet_ena = 0;
		}
	}
	return 0;
}

int shinano_wifi_set_carddetect(int val)
{
	shinano_wifi_cd = val;
	if (wifi_status_cb)
		wifi_status_cb(val, wifi_status_cb_devid);
	else
		printk(KERN_WARNING "%s: Nobody to notify\n", __func__);
	return 0;
}

static struct resource shinano_wifi_resources[] = {
	[0] = {
		.name	= "bcmdhd_wlan_irq",
		.flags	= IORESOURCE_IRQ | IORESOURCE_IRQ_HIGHLEVEL |
			  IORESOURCE_IRQ_SHAREABLE,
	},
};

struct wifi_platform_data shinano_wifi_control = {
	.set_power	= shinano_wifi_set_power,
	.set_carddetect	= shinano_wifi_set_carddetect,
};

static struct platform_device shinano_wifi = {
	.name		= "bcmdhd_wlan",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(shinano_wifi_resources),
	.resource	= shinano_wifi_resources,
	.dev		= {
		.platform_data = &shinano_wifi_control,
	},
};

void msm_init_wifi(void)
{
	shinano_wifi.resource->start = gpio_to_irq(WIFI_IRQ_GPIO);
	shinano_wifi.resource->end = gpio_to_irq(WIFI_IRQ_GPIO);
	platform_device_register(&shinano_wifi);
}

static int shinano_wifi_status_register(
			void (*callback)(int card_present, void *dev_id),
			void *dev_id)
{
	if (wifi_status_cb)
		return -EAGAIN;
	wifi_status_cb = callback;
	wifi_status_cb_devid = dev_id;
	return 0;
}

static unsigned int shinano_wifi_status(struct device *dev)
{
	return shinano_wifi_cd;
}

struct mmc_platform_data msm8974_sdc3_data = {
	.status			= shinano_wifi_status,
	.register_status_notify	= shinano_wifi_status_register,
};
