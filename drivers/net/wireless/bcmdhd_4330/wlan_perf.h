/*
 * Copyright (C) 2013 Broadcom Corporation
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#ifndef WLAN_PERF_H
#define WLAN_PERF_H

#define NUM_BUF_1PAGE  8
#define NUM_BUF_2PAGE  8
#define NUM_BUF_4PAGE  8
#define SCAN_BUFFER 65536

#define WLAN_STATIC_SCAN_BUF0		5
#define WLAN_STATIC_SCAN_BUF1		6
#define PREALLOC_WLAN_SEC_NUM		4
#define PREALLOC_WLAN_BUF_NUM		160
#define PREALLOC_WLAN_SECTION_HEADER	24

#define WLAN_SECTION_SIZE_0	(PREALLOC_WLAN_BUF_NUM * 128)
#define WLAN_SECTION_SIZE_1	(PREALLOC_WLAN_BUF_NUM * 128)
#define WLAN_SECTION_SIZE_2	(PREALLOC_WLAN_BUF_NUM * 512)
#define WLAN_SECTION_SIZE_3	(PREALLOC_WLAN_BUF_NUM * 1024)

#define DHD_SKB_HDRSIZE			336
#define DHD_SKB_1PAGE_BUFSIZE	((PAGE_SIZE*1)-DHD_SKB_HDRSIZE)
#define DHD_SKB_2PAGE_BUFSIZE	((PAGE_SIZE*2)-DHD_SKB_HDRSIZE)
#define DHD_SKB_4PAGE_BUFSIZE	((PAGE_SIZE*4)-DHD_SKB_HDRSIZE)
#define WLAN_SKB_BUF_NUM	17

struct wifi_platform_data {
	int (*set_power)(int val);
	int (*set_reset)(int val);
	int (*set_carddetect)(int val);
	void *(*mem_prealloc)(int section, unsigned long size);
	int (*get_mac_addr)(unsigned char *buf);
	void *(*get_country_code)(char *ccode);
};
struct wlan_sdio_kona_cfg {
	int		bcm_wlan_host_wake;
	int		bcm_wlan_reset;
};
int wlan_sdio_power_up(struct wlan_sdio_kona_cfg *cfg, int onoff);
int wlan_sdio_power_down(struct wlan_sdio_kona_cfg *cfg);
int wlan_sdio_card_emulate(struct wlan_sdio_kona_cfg *cfg, int insert);
#endif
