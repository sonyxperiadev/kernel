/*
 * drivers/net/wireless/somcwifictrl/dhd_custom_memprealloc.c
 *
 * Platform Dependent file for usage of Preallocted Memory
 *
 * Copyright (C) 1999-2015, Broadcom Corporation
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
 * you also meet, for each linked independent module, the terms and conditions
 * of the license of that module.  An independent module is a module which is
 * not derived from this software.  The special exception does not apply to any
 * modifications of the software.
 *
 *      Notwithstanding the above, under no circumstances may you combine this
 * software in any way with any other Broadcom software provided under a license
 * other than the GPL, without Broadcom's express prior written consent.
 */
/*
 * Copyright (C) 2015 Sony Mobile Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 */

#include <linux/device.h>
#include <linux/slab.h>
#include <linux/miscdevice.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/list.h>
#include <linux/io.h>
#include <linux/workqueue.h>
#include <linux/unistd.h>
#include <linux/bug.h>
#include <linux/skbuff.h>
#include <linux/init.h>

/* These definitions need to be aligned with bcmdhd */
#ifdef CONFIG_BCMDHD_PCIE
#define WLAN_STATIC_SKB 4
#else /* CONFIG_BCMDHD_PCIE */
#define WLAN_STATIC_SKB 14
#endif /* CONFIG_BCMDHD_PCIE */
#define WLAN_STATIC_SCAN_BUF0		5
#define WLAN_STATIC_DHD_INFO_BUF	7
#define WLAN_STATIC_DHD_WLFC_BUF	8
#define WLAN_STATIC_DHD_IF_FLOW_LKUP	9
#define WLAN_STATIC_DHD_MEMDUMP_RAM	11
#define WLAN_STATIC_DHD_WLFC_HANGER	12
#define WLAN_STATIC_DHD_PKTID_MAP	13

#define WLAN_SCAN_BUF_SIZE		(64 * 1024)

#if defined(CONFIG_64BIT)
#define WLAN_DHD_INFO_BUF_SIZE		(24 * 1024)
#define WLAN_DHD_WLFC_BUF_SIZE		(64 * 1024)
#define WLAN_DHD_IF_FLOW_LKUP_SIZE	(36 * 1024)
#else
#define WLAN_DHD_INFO_BUF_SIZE		(16 * 1024)
#define WLAN_DHD_WLFC_BUF_SIZE		(16 * 1024)
#define WLAN_DHD_IF_FLOW_LKUP_SIZE	(20 * 1024)
#endif /* CONFIG_64BIT */
#define WLAN_DHD_MEMDUMP_SIZE		(800 * 1024)
#define WLAN_DHD_WLFC_HANGER_SIZE	(128 * 1024)

#define PREALLOC_WLAN_SEC_NUM		4
#define PREALLOC_WLAN_BUF_NUM		160
#define PREALLOC_WLAN_SECTION_HEADER	24

#ifdef CONFIG_BCMDHD_PCIE
#define DHD_SKB_1PAGE_BUFSIZE	(PAGE_SIZE*1)
#define DHD_SKB_2PAGE_BUFSIZE	(PAGE_SIZE*2)
#define DHD_SKB_4PAGE_BUFSIZE	(PAGE_SIZE*4)

#define WLAN_SECTION_SIZE_0	(PREALLOC_WLAN_BUF_NUM * 128)
#define WLAN_SECTION_SIZE_1	0
#define WLAN_SECTION_SIZE_2	0
#define WLAN_SECTION_SIZE_3	(PREALLOC_WLAN_BUF_NUM * 1024)

#define DHD_SKB_1PAGE_BUF_NUM	0
#define DHD_SKB_2PAGE_BUF_NUM	64
#define DHD_SKB_4PAGE_BUF_NUM	0

#else
#define DHD_SKB_HDRSIZE		336
#define DHD_SKB_1PAGE_BUFSIZE	((PAGE_SIZE*1)-DHD_SKB_HDRSIZE)
#define DHD_SKB_2PAGE_BUFSIZE	((PAGE_SIZE*2)-DHD_SKB_HDRSIZE)
#define DHD_SKB_4PAGE_BUFSIZE	((PAGE_SIZE*4)-DHD_SKB_HDRSIZE)

#define SIZE_ORDER(x)		((1 << x) * PAGE_SIZE)
#define WLAN_SECTION_SIZE_0	(SIZE_ORDER(2))  /* for PROT */
#define WLAN_SECTION_SIZE_1	(SIZE_ORDER(2))  /* for RXBUF */
#define WLAN_SECTION_SIZE_2	(SIZE_ORDER(4))  /* for DATABUF */
#define WLAN_SECTION_SIZE_3	(SIZE_ORDER(2))  /* for OSL_BUF */

#define DHD_SKB_1PAGE_BUF_NUM	8
#define DHD_SKB_2PAGE_BUF_NUM	8
#define DHD_SKB_4PAGE_BUF_NUM	1
#endif /* CONFIG_BCMDHD_PCIE */

#define WLAN_SKB_1_2PAGE_BUF_NUM	((DHD_SKB_1PAGE_BUF_NUM) + \
		(DHD_SKB_2PAGE_BUF_NUM))
#define WLAN_SKB_BUF_NUM	((WLAN_SKB_1_2PAGE_BUF_NUM) + \
		(DHD_SKB_4PAGE_BUF_NUM))

#define DHD_PKTIDMAP_FIFO_MAX		4
#define WLAN_MAX_PKTID_ITEMS		(8192)
#if defined(CONFIG_64BIT)
#define WLAN_DHD_PKTID_MAP_HDR_SIZE	(20 + 4 * (WLAN_MAX_PKTID_ITEMS + 1))
#define WLAN_DHD_PKTID_MAP_ITEM_SIZE	(48)
#else
#define WLAN_DHD_PKTID_MAP_HDR_SIZE	(36)
#define WLAN_DHD_PKTID_MAP_ITEM_SIZE	(28)
#endif /* CONFIG_64BIT */
#define WLAN_DHD_PKTID_MAP_SIZE		((WLAN_DHD_PKTID_MAP_HDR_SIZE) + \
		(DHD_PKTIDMAP_FIFO_MAX * (WLAN_MAX_PKTID_ITEMS+1) * \
		WLAN_DHD_PKTID_MAP_ITEM_SIZE))

static struct sk_buff *wlan_static_skb[WLAN_SKB_BUF_NUM];

struct wlan_mem_prealloc {
	void *mem_ptr;
	unsigned long size;
};

static struct wlan_mem_prealloc wlan_mem_array[PREALLOC_WLAN_SEC_NUM] = {
	{NULL, (WLAN_SECTION_SIZE_0 + PREALLOC_WLAN_SECTION_HEADER)},
	{NULL, (WLAN_SECTION_SIZE_1 + PREALLOC_WLAN_SECTION_HEADER)},
	{NULL, (WLAN_SECTION_SIZE_2 + PREALLOC_WLAN_SECTION_HEADER)},
	{NULL, (WLAN_SECTION_SIZE_3 + PREALLOC_WLAN_SECTION_HEADER)}
};

void *wlan_static_scan_buf0 = NULL;
void *wlan_static_dhd_info_buf = NULL;
void *wlan_static_dhd_wlfc_buf = NULL;
void *wlan_static_if_flow_lkup = NULL;
void *wlan_static_dhd_memdump_ram = NULL;
#ifdef CONFIG_BCMDHD_PCIE
void *wlan_static_dhd_pktid_map = NULL;
#else /* CONFIG_BCMDHD_PCIE */
void *wlan_static_dhd_wlfc_hanger = NULL;
#endif /* CONFIG_BCMDHD_PCIE */

void
*dhd_wlan_mem_prealloc(int section, unsigned long size)
{
	if (section == WLAN_STATIC_SKB)
		return wlan_static_skb;

	if (section == WLAN_STATIC_SCAN_BUF0)
		return wlan_static_scan_buf0;

	if (section == WLAN_STATIC_DHD_INFO_BUF) {
		if (size > WLAN_DHD_INFO_BUF_SIZE) {
			pr_err("request DHD_INFO size(%lu) is bigger than"
				" static size(%d).\n", size,
				WLAN_DHD_INFO_BUF_SIZE);
			return NULL;
		}
		return wlan_static_dhd_info_buf;
	}

	if (section == WLAN_STATIC_DHD_WLFC_BUF)  {
		if (size > WLAN_DHD_WLFC_BUF_SIZE) {
			pr_err("request DHD_WLFC size(%lu) is bigger than"
				" static size(%d).\n",
				size, WLAN_DHD_WLFC_BUF_SIZE);
			return NULL;
		}
		return wlan_static_dhd_wlfc_buf;
	}

	if (section == WLAN_STATIC_DHD_IF_FLOW_LKUP)  {
		if (size > WLAN_DHD_IF_FLOW_LKUP_SIZE) {
			pr_err("request DHD_IF_FLOW size(%lu) is bigger than"
				" static size(%d).\n",
				size, WLAN_DHD_IF_FLOW_LKUP_SIZE);
			return NULL;
		}
		return wlan_static_if_flow_lkup;
	}

	if (section == WLAN_STATIC_DHD_MEMDUMP_RAM) {
		if (size > WLAN_DHD_MEMDUMP_SIZE) {
			pr_err("request DHD_MEMDUMP_RAM size(%lu) is bigger"
				" than static size(%d).\n",
				size, WLAN_DHD_MEMDUMP_SIZE);
			return NULL;
		}
		return wlan_static_dhd_memdump_ram;
	}

#ifdef CONFIG_BCMDHD_PCIE
	if (section == WLAN_STATIC_DHD_PKTID_MAP)  {
		if (size > WLAN_DHD_PKTID_MAP_SIZE) {
			pr_err("request DHD_PKTID_MAP size(%lu) is bigger than"
				" static size(%d).\n",
				size, WLAN_DHD_PKTID_MAP_SIZE);
			return NULL;
		}
		return wlan_static_dhd_pktid_map;
	}
#else /* CONFIG_BCMDHD_PCIE */
	if (section == WLAN_STATIC_DHD_WLFC_HANGER) {
		if (size > WLAN_DHD_WLFC_HANGER_SIZE) {
			pr_err("request DHD_WLFC_HANGER size(%lu) is"
				" bigger than static size(%d).\n",
				size, WLAN_DHD_WLFC_HANGER_SIZE);
			return NULL;
		}
		return wlan_static_dhd_wlfc_hanger;
	}
#endif /* CONFIG_BCMDHD_PCIE */

	if ((section < 0) || (section > PREALLOC_WLAN_SEC_NUM))
		return NULL;

	if (wlan_mem_array[section].size < size)
		return NULL;

	return wlan_mem_array[section].mem_ptr;
}
EXPORT_SYMBOL(dhd_wlan_mem_prealloc);

static int __init
dhd_init_wlan_mem(void)
{
	int i;
	int j;

	for (i = 0; i < DHD_SKB_1PAGE_BUF_NUM; i++) {
		wlan_static_skb[i] = dev_alloc_skb(DHD_SKB_1PAGE_BUFSIZE);
		if (!wlan_static_skb[i])
			goto err_skb_alloc;
	}

	for (i = DHD_SKB_1PAGE_BUF_NUM; i < WLAN_SKB_1_2PAGE_BUF_NUM; i++) {
		wlan_static_skb[i] = dev_alloc_skb(DHD_SKB_2PAGE_BUFSIZE);
		if (!wlan_static_skb[i])
			goto err_skb_alloc;
	}

#if !defined(CONFIG_BCMDHD_PCIE)
	wlan_static_skb[i] = dev_alloc_skb(DHD_SKB_4PAGE_BUFSIZE);
	if (!wlan_static_skb[i])
		goto err_skb_alloc;
#endif /* !CONFIG_BCMDHD_PCIE */

	for (i = 0; i < PREALLOC_WLAN_SEC_NUM; i++) {
		if (wlan_mem_array[i].size > 0) {
			wlan_mem_array[i].mem_ptr =
				kmalloc(wlan_mem_array[i].size, GFP_KERNEL);

			if (!wlan_mem_array[i].mem_ptr)
				goto err_mem_alloc;
		}
	}

	wlan_static_scan_buf0 = kmalloc(WLAN_SCAN_BUF_SIZE, GFP_KERNEL);
	if (!wlan_static_scan_buf0) {
		pr_err("Failed to alloc wlan_static_scan_buf0\n");
		goto err_mem_alloc;
	}

	wlan_static_dhd_info_buf = kmalloc(WLAN_DHD_INFO_BUF_SIZE, GFP_KERNEL);
	if (!wlan_static_dhd_info_buf) {
		pr_err("Failed to alloc wlan_static_dhd_info_buf\n");
		goto err_mem_alloc;
	}

#ifdef CONFIG_BCMDHD_PCIE
	wlan_static_if_flow_lkup = kmalloc(WLAN_DHD_IF_FLOW_LKUP_SIZE,
		GFP_KERNEL);
	if (!wlan_static_if_flow_lkup) {
		pr_err("Failed to alloc wlan_static_if_flow_lkup\n");
		goto err_mem_alloc;
	}

#ifdef CONFIG_BCMDHD_PREALLOC_PKTIDMAP
	wlan_static_dhd_pktid_map = kmalloc(WLAN_DHD_PKTID_MAP_SIZE,
		GFP_KERNEL);
	if (!wlan_static_dhd_pktid_map) {
		pr_err("Failed to alloc wlan_static_dhd_pktid_map\n");
		goto err_mem_alloc;
	}

#endif /* CONFIG_BCMDHD_PREALLOC_PKTIDMAP */
#else
	wlan_static_dhd_wlfc_buf = kmalloc(WLAN_DHD_WLFC_BUF_SIZE,
		GFP_KERNEL);
	if (!wlan_static_dhd_wlfc_buf) {
		pr_err("Failed to alloc wlan_static_dhd_wlfc_buf\n");
		goto err_mem_alloc;
	}

	wlan_static_dhd_wlfc_hanger = kmalloc(WLAN_DHD_WLFC_HANGER_SIZE,
		GFP_KERNEL);
	if (!wlan_static_dhd_wlfc_hanger) {
		pr_err("Failed to alloc wlan_static_dhd_wlfc_hanger\n");
		goto err_mem_alloc;
	}
#endif /* CONFIG_BCMDHD_PCIE */

#ifdef CONFIG_BCMDHD_DEBUG_PAGEALLOC
	wlan_static_dhd_memdump_ram = kmalloc(WLAN_DHD_MEMDUMP_SIZE,
		GFP_KERNEL);
	if (!wlan_static_dhd_memdump_ram) {
		pr_err("Failed to alloc wlan_static_dhd_memdump_ram\n");
		goto err_mem_alloc;
	}
#endif /* CONFIG_BCMDHD_DEBUG_PAGEALLOC */

	pr_err("%s: WIFI MEM Allocated\n", __func__);
	return 0;

err_mem_alloc:
#ifdef CONFIG_BCMDHD_DEBUG_PAGEALLOC
	kfree(wlan_static_dhd_memdump_ram);
#endif /* CONFIG_BCMDHD_DEBUG_PAGEALLOC */

#ifdef CONFIG_BCMDHD_PCIE
	kfree(wlan_static_dhd_pktid_map);
	kfree(wlan_static_if_flow_lkup);
#else
	kfree(wlan_static_dhd_wlfc_hanger);
	kfree(wlan_static_dhd_wlfc_buf);
#endif /* CONFIG_BCMDHD_PCIE */
	kfree(wlan_static_dhd_info_buf);
	kfree(wlan_static_scan_buf0);
	pr_err("Failed to mem_alloc for WLAN\n");

	for (j = 0; j < i; j++) {
		kfree(wlan_mem_array[j].mem_ptr);
	}

	i = WLAN_SKB_BUF_NUM;

err_skb_alloc:
	pr_err("Failed to skb_alloc for WLAN\n");
	for (j = 0; j < i; j++) {
		dev_kfree_skb(wlan_static_skb[j]);
	}

	return -ENOMEM;
}

device_initcall(dhd_init_wlan_mem);
