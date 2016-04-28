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
	int reserved;
};
//New change
/*
 * SDIO Platform flags:
 *
 * Bit Map
 * -------
 *
 *  -----------------------------
 * | BIT 31 | ~~~~~~~~~~ | BIT 0 |
 *  -----------------------------
 *                          |
 *                           --------> 0 - Device connected to this controller
 *                                         is removable
 *                                     1 - Device connected to this controller
 *                                         is removable
 */
enum kona_sdio_plat_flags {
	KONA_SDIO_FLAGS_DEVICE_REMOVABLE = 0,
	KONA_SDIO_FLAGS_DEVICE_NON_REMOVABLE = 1 << 0,
	KONA_SDIO_FLAGS_DEVICE_RPM_EN = 1 << 1,
	/* More flags can be added here */
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

	/* Flags describing various platform options
	 * removable or not, for example while the SD is removable eMMC is not
	 */
	enum kona_sdio_plat_flags flags;

	/* quirks and quirks2 of platform */
	unsigned int quirks;
	unsigned int quirks2;

	/* pm capability of platform */
	unsigned int pm_caps;

	/* clocks */
	char *peri_clk_name;
	char *ahb_clk_name;
	char *sleep_clk_name;
	char *vddo_regulator_name;
	char *vddsdxc_regulator_name;
	unsigned long peri_clk_rate;

	struct sdio_wifi_gpio_cfg wifi_gpio;

	/* Call back added for unified DHD support */
#ifdef CONFIG_BRCM_UNIFIED_DHD_SUPPORT
	int (*register_status_notify) (void (*callback)
				       (int card_present, void *dev_id),
				       void *dev_id);
#endif
	int (*configure_sdio_pullup) (bool pull_up);
};

#endif /* SDIO_PLATFORM_H */
