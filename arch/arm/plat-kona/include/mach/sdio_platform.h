/*****************************************************************************
* Copyright 2001 - 2009 Broadcom Corporation.  All rights reserved.
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

#ifndef _SDIO_PLATFORM_H
#define _SDIO_PLATFORM_H

/*
 * SDIO device type
 */
enum sdio_devtype {
	SDIO_DEV_TYPE_SDMMC = 0,
	SDIO_DEV_TYPE_WIFI,
	SDIO_DEV_TYPE_EMMC,

	/* used for internal array indexing, DO NOT modify */
	SDIO_DEV_TYPE_MAX,
};

/*
 * SDIO WiFi GPIO configuration
 */
struct sdio_wifi_gpio_cfg {
	int reset;
	int shutdown;
	int reg;
	int host_wake;
};

struct sdio_platform_cfg {
	/* specify which SDIO device */
	unsigned id;

	/*
	* For boards without the SDIO pullup registers, data_pullup needs to set
	* to 1
	*/
	unsigned int data_pullup;

	/* for devices with 8-bit lines */
	int is_8bit;

	/* card detection GPIO, required for SD/MMC */
	int cd_gpio;
	enum sdio_devtype devtype;

	/* clocks */
	char *peri_clk_name;
	char *ahb_clk_name;
	char *sleep_clk_name;
	unsigned long peri_clk_rate;

	struct sdio_wifi_gpio_cfg wifi_gpio;
};

#endif  /* SDIO_PLATFORM_H */
