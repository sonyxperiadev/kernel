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



extern struct mmc_platform_data tuna_wifi_data;


int rhea_wlan_init(void);


#define OMAP_PIN_INPUT_PULLUP		(OMAP_PULL_ENA | OMAP_INPUT_EN \
						| OMAP_PULL_UP)

#define OMAP_PIN_OUTPUT			0
#define OMAP_PIN_INPUT			OMAP_INPUT_EN
#define OMAP4_CTRL_MODULE_PAD_WKUP_CONTROL_USIMIO			0x0600
#define OMAP_PULL_ENA			(1 << 3)
#define OMAP_INPUT_EN			(1 << 8)
#define OMAP_PULL_UP			(1 << 4)

#define RHEA_GPIO_HYS_EN		1
#define RHEA_GPIO_HYS_DIS		0

#define RHEA_GPIO_PULL_DN_EN	ON
#define RHEA_GPIO_PULL_DN_DIS	OFF

#define RHEA_GPIO_PULL_UP_EN	ON
#define RHEA_GPIO_PULL_UP_DIS	OFF

#define RHEA_GPIO_SLEW_EN		1
#define RHEA_GPIO_SLEW_DIS		0

#define RHEA_GPIO_INPUT_EN		1
#define RHEA_GPIO_INPUT_DIS		0

#define RHEA_GPIO_DRV_CURR_4	4MA
#define RHEA_GPIO_DRV_CURR_6	6MA
#define RHEA_GPIO_DRV_CURR_8	8MA
#define RHEA_GPIO_DRV_CURR_10	10MA
#define RHEA_GPIO_DRV_CURR_12	12MA
#define RHEA_GPIO_DRV_CURR_16	16MA




//int WARN_ON(int x);

#define omap4_ctrl_wk_pad_writel(x, y)     0


