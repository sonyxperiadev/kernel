/******************************************************************************/
/*                                                                            */
/*  Copyright 2012  Broadcom Corporation                                      */
/*                                                                            */
/*     Unless you and Broadcom execute a separate written software license    */
/*     agreement governing use of this software, this software is licensed    */
/*     to you under the terms of the GNU General Public License version 2     */
/*     (the GPL), available at                                                */
/*                                                                            */
/*          http://www.broadcom.com/licenses/GPLv2.php                        */
/*                                                                            */
/*     with the following added to such license:                              */
/*                                                                            */
/*     As a special exception, the copyright holders of this software give    */
/*     you permission to link this software with independent modules, and     */
/*     to copy and distribute the resulting executable under terms of your    */
/*     choice, provided that you also meet, for each linked independent       */
/*     module, the terms and conditions of the license of that module. An     */
/*     independent module is a module which is not derived from this          */
/*     software.  The special exception does not apply to any modifications   */
/*     of the software.                                                       */
/*                                                                            */
/*     Notwithstanding the above, under no circumstances may you combine      */
/*     this software in any way with any other Broadcom software provided     */
/*     under a license other than the GPL, without Broadcom's express prior   */
/*     written consent.                                                       */
/*                                                                            */
/******************************************************************************/
#ifndef __KONA_MEMC_H__
#define __KONA_MEMC_H__

#include <linux/plist.h>
#include <linux/spinlock.h>

enum {
	KONA_MEMC_ENABLE_SELFREFRESH = 1,
	KONA_MEMC_DISABLE_DDRLDO = (1 << 1),
	KONA_MEMC_DISABLE_DDRLDO_IN_LPM = (1 << 2),
	KONA_MEMC_SET_SEQ_BUSY_CRITERIA = (1 << 3),
	KONA_MEMC_HW_FREQ_CHANGE_EN = (1 << 4),
	KONA_MEMC_DDR_PLL_PWRDN_EN = (1 << 5)
};

#define MEMC_SEQ_BUSY_CRITERIA_MAX	3
#define MEMC_MAX_PWR_MAX			3

struct kona_memc_pdata {
	u32 flags;
	u32 seq_busy_val;
	u32 max_pwr;
	void __iomem *memc0_ns_base;
	void __iomem *chipreg_base;
	void __iomem *memc0_aphy_base;
};

enum {
	APPS_MIN_PWR_STATE0,
	APPS_MIN_PWR_STATE1,
	APPS_MIN_PWR_STATE2,
	APPS_MIN_PWR_STATE3,
	APPS_MIN_PWR_MAX = APPS_MIN_PWR_STATE3
};

struct kona_memc_node {
	char *name;
	struct plist_node node;
	u32 min_pwr;
	bool valid;
};

struct kona_memc {
	struct plist_head min_pwr_list;
	spinlock_t memc_lock;
	u32 memc0_ns_base;
	u32 chipreg_base;
	u32 active_min_pwr;
	u32 memc0_aphy_base;
	struct kona_memc_pdata *pdata;
};

int memc_add_min_pwr_req(struct kona_memc_node *memc_node,
		char *client_name, u32 min_pwr);
int memc_del_min_pwr_req(struct kona_memc_node *memc_node);
int memc_update_min_pwr_req(struct kona_memc_node *memc_node, u32 min_pwr);
int memc_enable_selfrefresh(struct kona_memc *kmemc, int enable);
u32 kona_memc_get_ddr_clk_freq(void);

#endif /*__KONA_MEMC_H__*/



