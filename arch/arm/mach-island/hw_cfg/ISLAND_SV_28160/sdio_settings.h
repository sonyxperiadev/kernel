/*****************************************************************************
* Copyright 2010 - 2011 Broadcom Corporation.  All rights reserved.
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

#ifndef SDIO_SETTINGS_H
#define SDIO_SETTINGS_H

/*
 * Refer to include/linux/broadcom/sdio_platform.h for details
 */

#define HW_WLAN_GPIO_RESET_PIN 157
/*
 * #define HW_WLAN_GPIO_SHUTDOWN_PIN         GPIO48_KEY_IN5_ETM3
 * #define HW_WLAN_GPIO_REG_PIN			GPIO57_KEY_OUT5_SPIS_SSN_ETM12_EXTPHY_MDC
 */


/*
 * HW_SDIO_PARAM defines the array of the struct sdio_platform_cfg data
 * structure, with each element in the array representing a SDIO device setting
 */
#define HW_SDIO_PARAM \
{ \
   { /* SDIO0 */ \
      .id = 0, \
      .data_pullup = 0, \
      .peri_clk_name = "sdio1_clk", \
      .ahb_clk_name = "sdio1_ahb_clk", \
      .sleep_clk_name = "sdio1_sleep_clk", \
      .peri_clk_rate = 52000000, \
      .devtype = SDIO_DEV_TYPE_WIFI, \
   }, \
   { /* SDIO1 */ \
      .id = 1, \
      .data_pullup = 0, \
	.is_8bit =1, \
      .peri_clk_name = "sdio2_clk", \
      .ahb_clk_name = "sdio2_ahb_clk", \
      .sleep_clk_name = "sdio2_sleep_clk", \
      .peri_clk_rate = 52000000, \
      .devtype = SDIO_DEV_TYPE_EMMC, \
   }, \
   { /* SDIO3 */ \
      .id = 2, \
      .data_pullup = 0, \
      .peri_clk_name = "sdio3_clk", \
      .ahb_clk_name = "sdio3_ahb_clk", \
      .sleep_clk_name = "sdio3_sleep_clk", \
      .peri_clk_rate = 48000000, \
      .cd_gpio = 106, \
      .devtype = SDIO_DEV_TYPE_SDMMC, \
   }, \
}

#endif /* SDIO_SETTINGS_H */
