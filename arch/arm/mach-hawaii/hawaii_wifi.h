/*
 * Copyright (C) 2011 Google, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

int hawaii_wlan_init(void);

#define RHEA_GPIO_HYS_EN		1
#define RHEA_GPIO_HYS_DIS		0
#define RHEA_GPIO_PULL_DN_EN		ON
#define RHEA_GPIO_PULL_DN_DIS		OFF
#define RHEA_GPIO_PULL_UP_EN		ON
#define RHEA_GPIO_PULL_UP_DIS		OFF
#define RHEA_GPIO_SLEW_EN		1
#define RHEA_GPIO_SLEW_DIS		0
#define RHEA_GPIO_INPUT_EN		1
#define RHEA_GPIO_INPUT_DIS		0
#define RHEA_GPIO_DRV_CURR_4		4MA
#define RHEA_GPIO_DRV_CURR_6		6MA
#define RHEA_GPIO_DRV_CURR_8		8MA
#define RHEA_GPIO_DRV_CURR_10		10MA
#define RHEA_GPIO_DRV_CURR_12		12MA
#define RHEA_GPIO_DRV_CURR_16		16MA

#define PRINT_ERR(format, args...) \
	printk(KERN_ERR "%s: " format, __func__, ## args)
#define PRINT_INFO(format, args...) \
	printk(KERN_INFO "%s: " format, __func__, ## args)



extern int sdio_dev_is_initialized(enum sdio_devtype devtype);
extern struct sdio_wifi_gpio_cfg *sdio_get_wifi_gpio(enum sdio_devtype devtype);

#ifdef CONFIG_MMC_KONA_SDIO_WIFI
extern int bcm_sdiowl_init(int onoff);
extern int bcm_sdiowl_term(void);
#endif

/* Structure below is used ONLY if
 * CONFIG_BYPASS_WIFI_DEVTREE is defined
 * otherwise take info from device tree.
 * === Add to defconfig or define it in this file ====
 */

struct board_wifi_info {
	int wl_reset_gpio;
	int host_wake_gpio;
	const char *board_nvram_file;
	const char *module_name;
};

