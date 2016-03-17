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
	KONA_MEMC_DDR_PLL_PWRDN_EN = (1 << 5),
#ifdef CONFIG_LPDDR_DEV_TEMP
	KONA_MEMC_CS0_DEV_TEMP = (1 << 6),
	KONA_MEMC_CS1_DEV_TEMP = (1 << 7),
#endif
};

#define MEMC_SEQ_BUSY_CRITERIA_MAX	3
#define MEMC_MAX_PWR_MAX		3
#define NODE_NAME_LEN			10

#ifdef CONFIG_LPDDR_DEV_TEMP
enum {
	SHDWN = 1,
};

struct temp_thold {
	int mr4_sts;
	int action;
};
#endif

struct kona_memc_node {
	char name[NODE_NAME_LEN+1];
	struct plist_node node;
	u32 req;
	bool valid;
};


#ifdef CONFIG_MEMC_DFS
enum {
	MEMC_OPP_ECO,
	MEMC_OPP_NORMAL,
	MEMC_OPP_TURBO,
	MEMC_OPP_MAX,
};

struct usr_dfs_mode {
	struct list_head node;
	struct kona_memc_node req_node;
};


struct memc_dfs_pll_freq {
	u32 ndiv;
	u32 ndiv_frac;
	u32 pdiv;
	u32 mdiv;
};

#endif /*CONFIG_MEMC_DFS*/

struct kona_memc_pdata {
	u32 flags;
	u32 seq_busy_val;
	u32 max_pwr;
	void __iomem *memc0_ns_base;
	void __iomem *chipreg_base;
	void __iomem *memc0_aphy_base;
#ifdef CONFIG_LPDDR_DEV_TEMP
	int irq;
	int temp_period; /*number of cycles on XTAL clk*/
	struct temp_thold *temp_tholds;
	int num_thold; /*number of temperature limits*/
#endif
#ifdef CONFIG_MEMC_DFS
	struct memc_dfs_pll_freq pll_freq[MEMC_OPP_MAX];
#endif

};

enum {
	MEMC_PWR_STATE0,
	MEMC_PWR_STATE1,
	MEMC_PWR_STATE2,
	MEMC_PWR_STATE3,
	MEMC_PWR_MAX = MEMC_PWR_STATE3
};

struct kona_memc {
	struct plist_head min_pwr_list;
#ifdef CONFIG_MEMC_DFS
	struct plist_head dfs_list;
	struct list_head usr_dfs_list;
	u32 active_dfs_opp;
	u32 pll_rate;
#endif
	spinlock_t memc_lock;
	void __iomem *memc0_ns_base;
	void __iomem *chipreg_base;
	u32 active_min_pwr;
	void __iomem *memc0_aphy_base;
	struct kona_memc_pdata *pdata;
#ifdef CONFIG_LPDDR_DEV_TEMP
	int irq;
	struct work_struct memc_work;
	int temp_intr;
	int temp_sts;
	int dev_sel;
	int dev_temp_en;
#endif
};

int memc_add_min_pwr_req(struct kona_memc_node *memc_node,
		char *client_name, u32 min_pwr);
int memc_del_min_pwr_req(struct kona_memc_node *memc_node);
int memc_update_min_pwr_req(struct kona_memc_node *memc_node, u32 min_pwr);
int memc_enable_selfrefresh(struct kona_memc *kmemc, int enable);
u32 kona_memc_get_ddr_clk_freq(void);

#ifdef CONFIG_MEMC_DFS
int memc_add_dfs_req(struct kona_memc_node *memc_node,
		char *client_name, u32 opp);
int memc_del_dfs_req(struct kona_memc_node *memc_node);
int memc_update_dfs_req(struct kona_memc_node *memc_node, u32 opp);
#else
static inline int memc_add_dfs_req(struct kona_memc_node *memc_node,
		char *client_name, u32 opp)
{
	return -EINVAL;
}
static inline int memc_del_dfs_req(struct kona_memc_node *memc_node)
{
	return -EINVAL;
}

static inline int memc_update_dfs_req(struct kona_memc_node *memc_node, u32 opp)
{
	return -EINVAL;
}
#endif /*CONFIG_MEMC_DFS*/


#endif /*__KONA_MEMC_H__*/



