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
#include <linux/module.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/io.h>
#include <plat/kona_memc.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <mach/rdb/brcm_rdb_csr.h>
#include <mach/rdb/brcm_rdb_chipreg.h>
#include <mach/rdb/brcm_rdb_aphy_csr.h>
#include <linux/delay.h>
#ifdef CONFIG_DEBUG_FS
#include <linux/debugfs.h>
#endif
#include <linux/uaccess.h>
#include <linux/seq_file.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#ifdef CONFIG_LPDDR_DEV_TEMP
#include <linux/interrupt.h>
#include <linux/reboot.h>
#include <linux/irq.h>
#endif
#define MEMC0_APHY_REG(kmemc, off) ((kmemc)->memc0_aphy_base + (off))
#define MEMC0_NS_REG(kmemc, off) ((kmemc)->memc0_ns_base + (off))
#define CHIPREG_REG(kmemc, off) ((kmemc)->chipreg_base + (off))
#define INSURANCE_MAX	10000
#define FREF 26000000


#define REM_NODE_OPP 0xFFFFFFFF
#define BOOT_DFS_REQ_CLIENT "BOOTUP_REQ"

enum {
	MEMC_NODE_ADD,
	MEMC_NODE_DEL,
	MEMC_NODE_UPDATE,
};

enum {
	MEMC_AP_MIN_PWR,
	MEMC_CP_MIN_PWR,
	MEMC_DSP_MIN_PWR,
};

static struct kona_memc kona_memc;

static int memc_disable_ldo(struct kona_memc *kmemc, int disable)
{
	u32 reg;
	reg = readl(CHIPREG_REG(kmemc, CHIPREG_SPARE_CONTROL0_OFFSET));
	if (disable)
		reg |= CHIPREG_SPARE_CONTROL0_DDRLDO_I_LDO_DISABLE_MASK;
	else
		reg &= ~CHIPREG_SPARE_CONTROL0_DDRLDO_I_LDO_DISABLE_MASK;
	writel(reg, CHIPREG_REG(kmemc, CHIPREG_SPARE_CONTROL0_OFFSET));
	return 0;
}

static int memc_disable_ldo_in_lpm(struct kona_memc *kmemc, int disable)
{
	u32 reg;
	reg = readl(CHIPREG_REG(kmemc, CHIPREG_SPARE_CONTROL0_OFFSET));
	if (disable)
		reg |= CHIPREG_SPARE_CONTROL0_DISABLE_LDO_ON_LOW_PWR_MASK;
	else
		reg &= ~CHIPREG_SPARE_CONTROL0_DISABLE_LDO_ON_LOW_PWR_MASK;
	writel(reg, CHIPREG_REG(kmemc, CHIPREG_SPARE_CONTROL0_OFFSET));
	return 0;
}

static int memc_set_seq_busy_criteria(struct kona_memc *kmemc, u32 val)
{
	u32 reg;
	if (val > MEMC_SEQ_BUSY_CRITERIA_MAX)
		return -EINVAL;
	reg = readl(MEMC0_NS_REG(kmemc, CSR_CORE_SPR_RW_OFFSET));
	reg &= ~CSR_CORE_SPR_RW_CONTROL_SEQ_BUSY_MASK;
	reg |= (val << CSR_CORE_SPR_RW_CONTROL_SEQ_BUSY_SHIFT) &
				CSR_CORE_SPR_RW_CONTROL_SEQ_BUSY_MASK;
	writel(reg, MEMC0_NS_REG(kmemc, CSR_CORE_SPR_RW_OFFSET));
	return 0;
}


static int memc_set_min_pwr(struct kona_memc *kmemc, u32 val, u32 core)
{
	u32 reg;
	u32 offset;
	u32 mask;
	u32 insurance = 0;
	if (val > MEMC_PWR_MAX)
		return -EINVAL;
	switch (core) {
	case MEMC_AP_MIN_PWR:
		offset = CSR_APPS_MIN_PWR_STATE_OFFSET;
		mask =
		CSR_MEMC_PWR_STATE_PENDING_APPS_MIN_PWR_STATE_PENDING_MASK;
		break;
	case MEMC_CP_MIN_PWR:
		offset = CSR_MODEM_MIN_PWR_STATE_OFFSET;
		mask =
		CSR_MEMC_PWR_STATE_PENDING_MODEM_MIN_PWR_STATE_PENDING_MASK;
		break;
	case MEMC_DSP_MIN_PWR:
		offset = CSR_DSP_MIN_PWR_STATE_OFFSET;
		mask =
		CSR_MEMC_PWR_STATE_PENDING_DSP_MIN_PWR_STATE_PENDING_MASK;
		break;
	default:
		return -EINVAL;
	}
	reg = readl(MEMC0_NS_REG(kmemc, offset));
	/*mask & shift are same for all cores*/
	reg &= ~CSR_APPS_MIN_PWR_STATE_APPS_MIN_PWR_STATE_MASK;
	reg |= (val << CSR_APPS_MIN_PWR_STATE_APPS_MIN_PWR_STATE_SHIFT) &
				CSR_APPS_MIN_PWR_STATE_APPS_MIN_PWR_STATE_MASK;
	writel(reg, MEMC0_NS_REG(kmemc, offset));
	reg = readl(MEMC0_NS_REG(kmemc, CSR_MEMC_PWR_STATE_PENDING_OFFSET));
	while (reg & mask && insurance < INSURANCE_MAX) {
		insurance++;
		udelay(1);
		reg = readl(MEMC0_NS_REG(kmemc,
					CSR_MEMC_PWR_STATE_PENDING_OFFSET));
	}
	if (insurance == INSURANCE_MAX) {
		__WARN();
		return -EINVAL;
	}
	return 0;
}

static int memc_get_min_pwr(struct kona_memc *kmemc, u32 core)
{
	u32 reg;
	u32 offset;
	switch (core) {
	case MEMC_AP_MIN_PWR:
		offset = CSR_APPS_MIN_PWR_STATE_OFFSET;
		break;
	case MEMC_CP_MIN_PWR:
		offset = CSR_MODEM_MIN_PWR_STATE_OFFSET;
		break;
	case MEMC_DSP_MIN_PWR:
		offset = CSR_DSP_MIN_PWR_STATE_OFFSET;
		break;
	default:
		return -EINVAL;
	}
	reg = readl(MEMC0_NS_REG(kmemc, offset));
	/*mask & shift are same for all cores*/
	return (reg & CSR_APPS_MIN_PWR_STATE_APPS_MIN_PWR_STATE_MASK) >>
				CSR_APPS_MIN_PWR_STATE_APPS_MIN_PWR_STATE_SHIFT;
}


static int memc_set_max_pwr(struct kona_memc *kmemc, u32 val)
{
	u32 reg;
	u32 insurance = 0;
	if (val > MEMC_MAX_PWR_MAX)
		return -EINVAL;
	reg = readl(MEMC0_NS_REG(kmemc, CSR_MEMC_MAX_PWR_STATE_OFFSET));
	reg &= ~CSR_MEMC_MAX_PWR_STATE_MEMC_MAX_PWR_STATE_MASK;
	reg |= (val << CSR_MEMC_MAX_PWR_STATE_MEMC_MAX_PWR_STATE_SHIFT) &
				CSR_MEMC_MAX_PWR_STATE_MEMC_MAX_PWR_STATE_MASK;
	writel(reg, MEMC0_NS_REG(kmemc, CSR_MEMC_MAX_PWR_STATE_OFFSET));
	reg = readl(MEMC0_NS_REG(kmemc, CSR_MEMC_PWR_STATE_PENDING_OFFSET));
	while (reg &
		CSR_MEMC_PWR_STATE_PENDING_MEMC_MAX_PWR_STATE_PENDING_MASK &&
						insurance < INSURANCE_MAX) {
		insurance++;
		udelay(1);
		reg = readl(MEMC0_NS_REG(kmemc,
					CSR_MEMC_PWR_STATE_PENDING_OFFSET));
	}
	if (insurance == INSURANCE_MAX) {
		__WARN();
		return -EINVAL;
	}
	return 0;
}

static int memc_get_max_pwr(struct kona_memc *kmemc)
{
	u32 reg;

	reg = readl(MEMC0_NS_REG(kmemc, CSR_MEMC_MAX_PWR_STATE_OFFSET));
	return (reg & CSR_MEMC_MAX_PWR_STATE_MEMC_MAX_PWR_STATE_MASK) >>
			CSR_MEMC_MAX_PWR_STATE_MEMC_MAX_PWR_STATE_SHIFT;
}

static int memc_force_max_pwr(struct kona_memc *kmemc, int force)
{
	u32 reg;

	reg = readl(MEMC0_NS_REG(kmemc, CSR_MEMC_MAX_PWR_STATE_OFFSET));
	if (force)
		reg |= CSR_MEMC_MAX_PWR_STATE_FORCE_MAX_POWER_STATE_MASK;
	else
		reg &= ~CSR_MEMC_MAX_PWR_STATE_FORCE_MAX_POWER_STATE_MASK;
	writel(reg, MEMC0_NS_REG(kmemc, CSR_MEMC_MAX_PWR_STATE_OFFSET));
	return 0;
}

static int memc_get_force_max_pwr_state(struct kona_memc *kmemc)
{
	u32 reg;
	reg = readl(MEMC0_NS_REG(kmemc, CSR_MEMC_MAX_PWR_STATE_OFFSET));
	return (reg & CSR_MEMC_MAX_PWR_STATE_FORCE_MAX_POWER_STATE_MASK) >>
			CSR_MEMC_MAX_PWR_STATE_MEMC_MAX_PWR_STATE_SHIFT;
}

static u32 memc_min_pwr_get_max_req(struct kona_memc *kmemc)
{
	if (plist_head_empty(&kmemc->min_pwr_list))
		return 0;
	return plist_last(&kmemc->min_pwr_list)->prio;
}


static int memc_min_pwr_update(struct kona_memc *kmemc,
		struct kona_memc_node *memc_node, int action)
{
	u32 new_val;
	int ret = 0;
	spin_lock(&kmemc->memc_lock);
	switch (action) {
	case MEMC_NODE_ADD:
		plist_node_init(&memc_node->node, memc_node->req);
		plist_add(&memc_node->node, &kmemc->min_pwr_list);
		break;
	case MEMC_NODE_DEL:
		plist_del(&memc_node->node, &kmemc->min_pwr_list);
		break;
	case MEMC_NODE_UPDATE:
		plist_del(&memc_node->node, &kmemc->min_pwr_list);
		plist_node_init(&memc_node->node, memc_node->req);
		plist_add(&memc_node->node, &kmemc->min_pwr_list);
		break;
	default:
		BUG();
		return -EINVAL;
	}
	new_val = memc_min_pwr_get_max_req(kmemc);
	if (new_val != kmemc->active_min_pwr) {
		ret = memc_set_min_pwr(kmemc, new_val, MEMC_AP_MIN_PWR);
		if (!ret)
			kmemc->active_min_pwr = new_val;
	}
	spin_unlock(&kmemc->memc_lock);
	return ret;
}


int memc_add_min_pwr_req(struct kona_memc_node *memc_node,
		char *client_name, u32 min_pwr)
{
	if (unlikely(memc_node->valid)) {
		BUG();
		return -EINVAL;
	}

	strncpy(memc_node->name, client_name, NODE_NAME_LEN);
	memc_node->req = min_pwr;
	memc_node->valid = 1;
	return memc_min_pwr_update(&kona_memc, memc_node, MEMC_NODE_ADD);

}
EXPORT_SYMBOL(memc_add_min_pwr_req);

int memc_del_min_pwr_req(struct kona_memc_node *memc_node)
{
	int ret;
	if (unlikely(memc_node->valid == 0)) {
		BUG();
		return -EINVAL;
	}
	ret = memc_min_pwr_update(&kona_memc, memc_node, MEMC_NODE_DEL);
	memc_node->valid = 0;
	memc_node->name[0] = 0;
	return ret;
}
EXPORT_SYMBOL(memc_del_min_pwr_req);

int memc_update_min_pwr_req(struct kona_memc_node *memc_node, u32 min_pwr)
{
	if (unlikely(memc_node->valid == 0)) {
		BUG();
		return -EINVAL;
	}
	if (memc_node->req != min_pwr) {
		memc_node->req = min_pwr;
		return memc_min_pwr_update(&kona_memc,
				memc_node, MEMC_NODE_UPDATE);
	}
	return 0;
}
EXPORT_SYMBOL(memc_update_min_pwr_req);

#ifdef CONFIG_MEMC_DFS

static void memc_wait_for_state_change(struct kona_memc *kmemc)
{
	int ins = 10000;
	u32 reg;
	u32 cur_state;
	u32 tgt_state;

	/* wait till current state != target state*/
	reg = readl_relaxed(MEMC0_NS_REG(kmemc,
		CSR_CURRENT_FREQUENCY_STATE_OFFSET));
	cur_state = (reg & CSR_CURRENT_FREQUENCY_STATE_CURRENT_STATE_MASK) >>
			CSR_CURRENT_FREQUENCY_STATE_CURRENT_STATE_SHIFT;
	tgt_state = (reg & CSR_CURRENT_FREQUENCY_STATE_TARGET_STATE_MASK) >>
			CSR_CURRENT_FREQUENCY_STATE_TARGET_STATE_SHIFT;

	while (ins && cur_state != tgt_state) {
		ins--;
		udelay(1);
		reg = readl_relaxed(MEMC0_NS_REG(kmemc,
			CSR_CURRENT_FREQUENCY_STATE_OFFSET));
		cur_state = (reg &
			CSR_CURRENT_FREQUENCY_STATE_CURRENT_STATE_MASK) >>
			CSR_CURRENT_FREQUENCY_STATE_CURRENT_STATE_SHIFT;
		tgt_state = (reg &
			CSR_CURRENT_FREQUENCY_STATE_TARGET_STATE_MASK) >>
			CSR_CURRENT_FREQUENCY_STATE_TARGET_STATE_SHIFT;
	}
	BUG_ON(ins == 0);
}

static int memc_config_ddr_pll_freq(struct kona_memc *kmemc,
	struct memc_dfs_pll_freq *pll_freq)
{
	u32 reg;
	u32 max_pwr;
	int ins = 10000;

	BUG_ON(!kmemc->pdata);
	/*  power state change pending  ? */
	reg = readl_relaxed(MEMC0_NS_REG(kmemc,
		CSR_MEMC_PWR_STATE_PENDING_OFFSET));
	while (ins &&
		reg &
		CSR_MEMC_PWR_STATE_PENDING_MEMC_MAX_PWR_STATE_PENDING_MASK) {
		udelay(1);
		ins--;
		reg = readl(MEMC0_NS_REG(kmemc,
				CSR_MEMC_PWR_STATE_PENDING_OFFSET));
	}
	BUG_ON(ins == 0);
	/* force maxpwr to 2 - 156 from syspll */
	reg = readl_relaxed(MEMC0_NS_REG(kmemc, CSR_MEMC_MAX_PWR_STATE_OFFSET));
	max_pwr = reg;
	reg &= ~CSR_MEMC_MAX_PWR_STATE_MEMC_MAX_PWR_STATE_MASK;
	reg |= MEMC_PWR_STATE2 <<
		CSR_MEMC_MAX_PWR_STATE_MEMC_MAX_PWR_STATE_SHIFT;
	reg |= CSR_MEMC_MAX_PWR_STATE_FORCE_MAX_POWER_STATE_MASK;
	writel_relaxed(reg, MEMC0_NS_REG(kmemc, CSR_MEMC_MAX_PWR_STATE_OFFSET));

	memc_wait_for_state_change(kmemc);

	/* set ddr pll frequency */
	reg = readl_relaxed(MEMC0_APHY_REG(kmemc,
			APHY_CSR_DDR_PLL_VCO_FREQ_CNTRL0_OFFSET));
	reg &= ~(APHY_CSR_DDR_PLL_VCO_FREQ_CNTRL0_PDIV_MASK |
		APHY_CSR_DDR_PLL_VCO_FREQ_CNTRL0_NDIV_INT_MASK);
	reg |= (pll_freq->ndiv <<
		APHY_CSR_DDR_PLL_VCO_FREQ_CNTRL0_NDIV_INT_SHIFT) &
		APHY_CSR_DDR_PLL_VCO_FREQ_CNTRL0_NDIV_INT_MASK;
	reg |= (pll_freq->pdiv <<
		APHY_CSR_DDR_PLL_VCO_FREQ_CNTRL0_PDIV_SHIFT) &
		APHY_CSR_DDR_PLL_VCO_FREQ_CNTRL0_PDIV_MASK;
	writel_relaxed(reg, MEMC0_APHY_REG(kmemc,
		APHY_CSR_DDR_PLL_VCO_FREQ_CNTRL0_OFFSET));
	reg = readl_relaxed(MEMC0_APHY_REG(kmemc,
		APHY_CSR_DDR_PLL_VCO_FREQ_CNTRL1_OFFSET));
	reg &= ~APHY_CSR_DDR_PLL_VCO_FREQ_CNTRL1_NDIV_FRAC_MASK;
	reg |= (pll_freq->ndiv_frac <<
		APHY_CSR_DDR_PLL_VCO_FREQ_CNTRL1_NDIV_FRAC_SHIFT) &
		APHY_CSR_DDR_PLL_VCO_FREQ_CNTRL1_NDIV_FRAC_MASK;
	writel_relaxed(reg, MEMC0_APHY_REG(kmemc,
			APHY_CSR_DDR_PLL_VCO_FREQ_CNTRL1_OFFSET));

	reg = readl_relaxed(MEMC0_APHY_REG(kmemc,
			APHY_CSR_DDR_PLL_MDIV_VALUE_OFFSET));
	reg &= ~APHY_CSR_DDR_PLL_MDIV_VALUE_MDIV_MASK;
	reg |= (pll_freq->mdiv <<
		APHY_CSR_DDR_PLL_MDIV_VALUE_MDIV_SHIFT) &
		APHY_CSR_DDR_PLL_MDIV_VALUE_MDIV_MASK;
	writel_relaxed(reg, MEMC0_APHY_REG(kmemc,
			APHY_CSR_DDR_PLL_MDIV_VALUE_OFFSET));

	/*  power state change pending  ? */
	ins = 1000;
	reg = readl_relaxed(MEMC0_NS_REG(kmemc,
		CSR_MEMC_PWR_STATE_PENDING_OFFSET));
	while (ins &&
		reg &
		CSR_MEMC_PWR_STATE_PENDING_MEMC_MAX_PWR_STATE_PENDING_MASK) {
		udelay(1);
		ins--;
		reg = readl(MEMC0_NS_REG(kmemc,
				CSR_MEMC_PWR_STATE_PENDING_OFFSET));
	}
	BUG_ON(ins == 0);

	/*restore max power state*/
	writel_relaxed(max_pwr,
		MEMC0_NS_REG(kmemc, CSR_MEMC_MAX_PWR_STATE_OFFSET));

	memc_wait_for_state_change(kmemc);
	return 0;
}



static u32 memc_dfs_get_max_req(struct kona_memc *kmemc)
{
	if (plist_head_empty(&kmemc->dfs_list))
		return MEMC_OPP_NORMAL;
	return plist_last(&kmemc->dfs_list)->prio;
}


static int memc_dfs_update(struct kona_memc *kmemc,
		struct kona_memc_node *memc_node, int action)
{
	u32 new_val;
	int ret = 0;
	struct memc_dfs_pll_freq *pll_freq;

	spin_lock(&kmemc->memc_lock);
	switch (action) {
	case MEMC_NODE_ADD:
		plist_node_init(&memc_node->node, memc_node->req);
		plist_add(&memc_node->node, &kmemc->dfs_list);
		break;
	case MEMC_NODE_DEL:
		plist_del(&memc_node->node, &kmemc->dfs_list);
		break;
	case MEMC_NODE_UPDATE:
		plist_del(&memc_node->node, &kmemc->dfs_list);
		plist_node_init(&memc_node->node, memc_node->req);
		plist_add(&memc_node->node, &kmemc->dfs_list);
		break;
	default:
		BUG();
		return -EINVAL;
	}
	new_val = memc_dfs_get_max_req(kmemc);
	if (new_val != kmemc->active_dfs_opp) {
		switch (new_val) {
		case MEMC_OPP_ECO:
			/*Set max power to 2 - 156MHz*/
			memc_set_max_pwr(kmemc, MEMC_PWR_STATE2);
			break;

		case MEMC_OPP_NORMAL:
		case MEMC_OPP_TURBO:
			if (kmemc->active_dfs_opp == MEMC_OPP_ECO)
				memc_set_max_pwr(kmemc, MEMC_PWR_STATE3);
			if (kmemc->pll_rate != new_val) {
				pll_freq = &kmemc->pdata->pll_freq[new_val];
				memc_config_ddr_pll_freq(kmemc, pll_freq);
			}
			kmemc->pll_rate = new_val;
			break;

		default:
			BUG();
			break;
		}
		kmemc->active_dfs_opp = new_val;
	}
	spin_unlock(&kmemc->memc_lock);
	return ret;
}

int memc_add_dfs_req(struct kona_memc_node *memc_node,
		char *client_name, u32 opp)
{
	if (unlikely(memc_node->valid)) {
		BUG();
		return -EINVAL;
	}
	if (unlikely(opp >= MEMC_OPP_MAX))
		return -EINVAL;

	strncpy(memc_node->name, client_name, NODE_NAME_LEN);
	memc_node->req = opp;
	memc_node->valid = 1;
	return memc_dfs_update(&kona_memc, memc_node, MEMC_NODE_ADD);
}
EXPORT_SYMBOL(memc_add_dfs_req);

int memc_del_dfs_req(struct kona_memc_node *memc_node)
{
	int ret;
	if (unlikely(memc_node->valid == 0)) {
		BUG();
		return -EINVAL;
	}
	ret = memc_dfs_update(&kona_memc, memc_node, MEMC_NODE_DEL);
	memc_node->valid = 0;
	memc_node->name[0] = 0;
	return ret;
}
EXPORT_SYMBOL(memc_del_dfs_req);

int memc_update_dfs_req(struct kona_memc_node *memc_node, u32 opp)
{
	if (unlikely(memc_node->valid == 0)) {
		BUG();
		return -EINVAL;
	}

	if (unlikely(opp >= MEMC_OPP_MAX))
		return -EINVAL;

	if (memc_node->req != opp) {
		memc_node->req = opp;
		return memc_dfs_update(&kona_memc,
				memc_node, MEMC_NODE_UPDATE);
	}
	return 0;
}
EXPORT_SYMBOL(memc_update_dfs_req);

struct usr_dfs_mode *memc_find_usr_dfs_mode(struct kona_memc *kmemc,
	char *client_name)
{
	struct usr_dfs_mode *usr_node;
	struct usr_dfs_mode *ret = NULL;
	spin_lock(&kmemc->memc_lock);
	list_for_each_entry(usr_node, &kmemc->usr_dfs_list, node) {
		if (strncmp(usr_node->req_node.name, client_name,
			NODE_NAME_LEN) == 0) {
			ret = usr_node;
			break;
		}
	}
	spin_unlock(&kmemc->memc_lock);
	return ret;
}

static int memc_add_usr_dfs_req(struct kona_memc *kmemc, char *client_name,
		u32 opp)
{
	struct usr_dfs_mode *usr_node =
		memc_find_usr_dfs_mode(kmemc, client_name);
	if (usr_node)
		return -EINVAL;
	usr_node = kzalloc(sizeof(struct usr_dfs_mode), GFP_KERNEL);
	if (!usr_node)
		return -ENOMEM;
	spin_lock(&kmemc->memc_lock);
	list_add(&usr_node->node, &kmemc->usr_dfs_list);
	spin_unlock(&kmemc->memc_lock);
	return memc_add_dfs_req(&usr_node->req_node, client_name, opp);

}

static int memc_del_usr_dfs_req(struct kona_memc *kmemc,
	struct usr_dfs_mode *usr_node)
{
	int ret;
	ret = memc_del_dfs_req(&usr_node->req_node);
	spin_lock(&kmemc->memc_lock);
	list_del(&usr_node->node);
	spin_unlock(&kmemc->memc_lock);
	kfree(usr_node);
	return ret;
}

static int memc_update_usr_dfs_req(struct usr_dfs_mode *usr_node, u32 opp)
{
	return memc_update_dfs_req(&usr_node->req_node, opp);
}

static ssize_t dfs_usr_req_store(struct kobject *kobj,
	struct kobj_attribute *attr, const char *buf, size_t n)
{
	char name[NODE_NAME_LEN+1] = {0};
	u32 opp = MEMC_OPP_MAX;
	struct usr_dfs_mode *usr_node;

	sscanf(buf, "%s%x", name, &opp);

	if (!name[0] || opp == MEMC_OPP_MAX) {
		pr_info("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	if (opp > MEMC_OPP_MAX && opp != REM_NODE_OPP) {
			pr_info("%s: invalid opp\n", __func__);
			return -EINVAL;
	}

	usr_node = memc_find_usr_dfs_mode(&kona_memc, name);

	if (usr_node) {
		if (opp == REM_NODE_OPP)
			memc_del_usr_dfs_req(&kona_memc, usr_node);
		else
			memc_update_usr_dfs_req(usr_node, opp);
	} else if (opp < MEMC_OPP_MAX)
		memc_add_usr_dfs_req(&kona_memc, name, opp);
	else {
			pr_info("%s: invalid opp\n", __func__);
			return -EINVAL;
	}
	return n;
}

static struct kobj_attribute dfs_usr_req_attr = {
	.attr	= {
		.name = __stringify(memc_dfs),
		.mode = S_IWUSR,
	},
	.store	= dfs_usr_req_store,
};


static ssize_t dfs_init_store(struct kobject *kobj,
	struct kobj_attribute *attr, const char *buf, size_t n)

{
	struct usr_dfs_mode *usr_node;
	usr_node = memc_find_usr_dfs_mode(&kona_memc, BOOT_DFS_REQ_CLIENT);

	if (!usr_node)
		return -EINVAL;

#ifdef CONFIG_MEMC_FORCE_156M_IN_SUSPEND
	pr_info("MEMC DFS Del req:%s\n", BOOT_DFS_REQ_CLIENT);
	memc_del_usr_dfs_req(&kona_memc, usr_node);
#else
	pr_info("MEMC DFS update req:%s\n", BOOT_DFS_REQ_CLIENT);
	memc_update_usr_dfs_req(usr_node, MEMC_OPP_NORMAL);
#endif
	return n;
}

static struct kobj_attribute dfs_init_attr = {
	.attr	= {
		.name = __stringify(memc_dfs_init),
		.mode = S_IWUSR,
	},
	.store	= dfs_init_store,
};


static struct attribute *kmemc_attr[] = {
	&dfs_usr_req_attr.attr,
	&dfs_init_attr.attr,
	NULL,
};

static struct attribute_group kmemc_attr_group = {
	.attrs = kmemc_attr,
};



#endif /*CONFIG_MEMC_DFS*/

int memc_enable_selfrefresh(struct kona_memc *kmemc, int enable)
{
	u32 reg;
	spin_lock(&kmemc->memc_lock);
	reg = readl(MEMC0_NS_REG(kmemc, CSR_HW_FREQ_CHANGE_CNTRL_OFFSET));
	if (enable)
		reg |= CSR_HW_FREQ_CHANGE_CNTRL_HW_AUTO_PWR_TRANSITION_MASK;
	else
		reg &= ~CSR_HW_FREQ_CHANGE_CNTRL_HW_AUTO_PWR_TRANSITION_MASK;
	writel(reg, MEMC0_NS_REG(kmemc, CSR_HW_FREQ_CHANGE_CNTRL_OFFSET));
	spin_unlock(&kmemc->memc_lock);
	return 0;
}
EXPORT_SYMBOL(memc_enable_selfrefresh);

#ifdef CONFIG_LPDDR_DEV_TEMP
static int memc_update_temp_period(struct kona_memc *kmemc, int period)
{
	u32 reg;

	reg = readl(MEMC0_NS_REG(kmemc, CSR_LPDDR2_DEV_TEMP_PERIOD_OFFSET));
	reg &= ~CSR_LPDDR2_DEV_TEMP_PERIOD_DEV_TEMP_PERIOD_MASK;
	reg |= (period <<
		CSR_LPDDR2_DEV_TEMP_PERIOD_DEV_TEMP_PERIOD_SHIFT) &
		CSR_LPDDR2_DEV_TEMP_PERIOD_DEV_TEMP_PERIOD_MASK;
	writel(reg, MEMC0_NS_REG(kmemc, CSR_LPDDR2_DEV_TEMP_PERIOD_OFFSET));
	return 0;
}

static int memc_dev_temp_intr_handle(struct kona_memc *kmemc)
{
	int i;
	struct kona_memc_pdata *pdata = kmemc->pdata;

	/*checking if value of device temp reg has changed*/
	if (kmemc->temp_intr == 0)
		goto exit;

	/* check device selected cs0/cs1 */
	if ((!(pdata->flags & KONA_MEMC_CS0_DEV_TEMP) && !(kmemc->dev_sel)) ||
		(!(pdata->flags & KONA_MEMC_CS1_DEV_TEMP) && kmemc->dev_sel)) {
		pr_info("%s: wrong device selection\n", __func__);
		goto exit;
	}

	/* check temperature staus*/
	for (i = 0; i < pdata->num_thold; i++) {
		if (kmemc->temp_sts == pdata->temp_tholds[i].mr4_sts) {
			pr_info("%s:LPDDR2 temp lmt reached %d\n",
					__func__, kmemc->temp_sts);
			if (pdata->temp_tholds[i].action & SHDWN)
				kernel_halt();
		}
	}
exit:
	return 0;
}

static void kmemc_irq_work(struct work_struct *ws)
{
	struct kona_memc *kmemc = container_of(ws, struct kona_memc, memc_work);

	if (kmemc->dev_temp_en)
		memc_dev_temp_intr_handle(kmemc);
}

static irqreturn_t kmemc_isr(int irq, void *drvdata)
{
	u32 reg;
	struct kona_memc *kmemc =  drvdata;

	if (kmemc->dev_temp_en) {
		/* check if temperature interrupt */
		reg = readl(MEMC0_NS_REG(kmemc,
					CSR_LPDDR2_DEV_TEMP_STATUS_OFFSET));
		kmemc->temp_intr = (reg &
			CSR_LPDDR2_DEV_TEMP_STATUS_DEV_TEMP_INTR_MASK) >>
			CSR_LPDDR2_DEV_TEMP_STATUS_DEV_TEMP_INTR_SHIFT;
		kmemc->temp_sts = (reg &
			CSR_LPDDR2_DEV_TEMP_STATUS_DEV_TEMP_STATUS_MASK) >>
			CSR_LPDDR2_DEV_TEMP_STATUS_DEV_TEMP_STATUS_SHIFT;
		kmemc->dev_sel =  (reg &
			CSR_LPDDR2_DEV_TEMP_STATUS_DEVICE_SELECTED_MASK) >>
			CSR_LPDDR2_DEV_TEMP_STATUS_DEVICE_SELECTED_SHIFT;
	}
	schedule_work(&kmemc->memc_work);
	return IRQ_HANDLED;
}

int memc_enable_thermal_mgmt(struct kona_memc *kmemc)
{
	u32 reg;
	struct kona_memc_pdata *pdata = kmemc->pdata;

	kmemc->dev_temp_en = 1;
	if (pdata->flags & KONA_MEMC_CS0_DEV_TEMP) {
		reg = readl(MEMC0_NS_REG(kmemc,
					CSR_LPDDR2_DEV_TEMP_PERIOD_OFFSET));
		reg |= CSR_LPDDR2_DEV_TEMP_PERIOD_ENABLE_DDR_CS0_DEV_TEMP_MASK;
		writel(reg, MEMC0_NS_REG(kmemc,
					CSR_LPDDR2_DEV_TEMP_PERIOD_OFFSET));
	}
	if (pdata->flags & KONA_MEMC_CS1_DEV_TEMP) {
		reg = readl(MEMC0_NS_REG(kmemc,
					CSR_LPDDR2_DEV_TEMP_PERIOD_OFFSET));
		reg |= CSR_LPDDR2_DEV_TEMP_PERIOD_ENABLE_DDR_CS1_DEV_TEMP_MASK;
		writel(reg, MEMC0_NS_REG(kmemc,
					CSR_LPDDR2_DEV_TEMP_PERIOD_OFFSET));
	}

	memc_update_temp_period(kmemc, pdata->temp_period);

	/*checking if the threshold level is already passed*/
	reg = readl(MEMC0_NS_REG(kmemc,
				CSR_LPDDR2_DEV_TEMP_STATUS_OFFSET));
	kmemc->temp_intr = (reg &
			CSR_LPDDR2_DEV_TEMP_STATUS_DEV_TEMP_INTR_MASK) >>
		CSR_LPDDR2_DEV_TEMP_STATUS_DEV_TEMP_INTR_SHIFT;
	kmemc->temp_sts = (reg &
			CSR_LPDDR2_DEV_TEMP_STATUS_DEV_TEMP_STATUS_MASK) >>
		CSR_LPDDR2_DEV_TEMP_STATUS_DEV_TEMP_STATUS_SHIFT;
	kmemc->dev_sel =  (reg &
			CSR_LPDDR2_DEV_TEMP_STATUS_DEVICE_SELECTED_MASK) >>
		CSR_LPDDR2_DEV_TEMP_STATUS_DEVICE_SELECTED_SHIFT;

	schedule_work(&kmemc->memc_work);
	return 0;
}
#endif

unsigned long compute_ddr_clk_freq(struct kona_memc *kmemc)
{
	u32 reg_val = 0;
	u32 mdiv, ndiv_int, ndiv_frac, pdiv, phy_div, frac_div;
	u64 temp;

	frac_div = 0x100000;
	reg_val = readl(MEMC0_NS_REG(kmemc,
				CSR_MEMC_FREQ_STATE_MAPPING_OFFSET));
	phy_div = (reg_val &
		CSR_MEMC_FREQ_STATE_MAPPING_DDR_FREQ_DIVIDE_VAL_MASK)  >>
		CSR_MEMC_FREQ_STATE_MAPPING_DDR_FREQ_DIVIDE_VAL_SHIFT;
	phy_div = 2 * (phy_div + 1);

	reg_val = readl(MEMC0_APHY_REG(kmemc,
				APHY_CSR_DDR_PLL_VCO_FREQ_CNTRL0_OFFSET));
	ndiv_int = (reg_val & APHY_CSR_DDR_PLL_VCO_FREQ_CNTRL0_NDIV_INT_MASK) >>
		APHY_CSR_DDR_PLL_VCO_FREQ_CNTRL0_NDIV_INT_SHIFT;
	if (ndiv_int == 0)
		ndiv_int = 1024;

	reg_val = readl(MEMC0_APHY_REG(kmemc,
				APHY_CSR_DDR_PLL_VCO_FREQ_CNTRL1_OFFSET));
	ndiv_frac = (reg_val &
			APHY_CSR_DDR_PLL_VCO_FREQ_CNTRL1_NDIV_FRAC_MASK) >>
		APHY_CSR_DDR_PLL_VCO_FREQ_CNTRL1_NDIV_FRAC_SHIFT;

	reg_val = readl(MEMC0_APHY_REG(kmemc,
				APHY_CSR_DDR_PLL_VCO_FREQ_CNTRL0_OFFSET));
	pdiv = (reg_val & APHY_CSR_DDR_PLL_VCO_FREQ_CNTRL0_PDIV_MASK) >>
		APHY_CSR_DDR_PLL_VCO_FREQ_CNTRL0_PDIV_SHIFT;
	if (pdiv == 0)
		pdiv = 8;

	reg_val = readl(MEMC0_APHY_REG(kmemc,
				APHY_CSR_DDR_PLL_MDIV_VALUE_OFFSET));
	mdiv = (reg_val & APHY_CSR_DDR_PLL_MDIV_VALUE_MDIV_MASK) >>
		APHY_CSR_DDR_PLL_MDIV_VALUE_MDIV_SHIFT;
	mdiv = 2 << mdiv;

	/*fddr = ddr_pll_fref / phy_div
	  = (fref * (ndiv_int * 2^20 + ndiv_frac)) /
	  (pdiv * 2^20 *mdiv * phy_div) */
	temp = ((u64)(ndiv_int * frac_div + ndiv_frac) * FREF);

	do_div(temp, pdiv * mdiv * phy_div * frac_div);

	return (unsigned long)temp;
}

static int memc_init(struct kona_memc *kmemc)
{
	u32 reg;
	struct kona_memc_pdata *pdata = kmemc->pdata;

	memc_enable_selfrefresh(&kona_memc,
			!!(pdata->flags & KONA_MEMC_ENABLE_SELFREFRESH));
	memc_disable_ldo(&kona_memc,
			!!(pdata->flags & KONA_MEMC_DISABLE_DDRLDO));
	memc_disable_ldo_in_lpm(&kona_memc,
			!!(pdata->flags & KONA_MEMC_DISABLE_DDRLDO_IN_LPM));
	if (pdata->flags & KONA_MEMC_SET_SEQ_BUSY_CRITERIA)
		memc_set_seq_busy_criteria(&kona_memc, pdata->seq_busy_val);

	reg = readl(MEMC0_NS_REG(kmemc, CSR_HW_FREQ_CHANGE_CNTRL_OFFSET));
	if (pdata->flags & KONA_MEMC_HW_FREQ_CHANGE_EN)
		reg |= CSR_HW_FREQ_CHANGE_CNTRL_HW_FREQ_CHANGE_EN_MASK;
	else
		reg &= ~CSR_HW_FREQ_CHANGE_CNTRL_HW_FREQ_CHANGE_EN_MASK;

	if (pdata->flags & KONA_MEMC_DDR_PLL_PWRDN_EN)
		reg |= CSR_HW_FREQ_CHANGE_CNTRL_DDR_PLL_PWRDN_ENABLE_MASK;
	else
		reg &= ~CSR_HW_FREQ_CHANGE_CNTRL_DDR_PLL_PWRDN_ENABLE_MASK;
	writel(reg, MEMC0_NS_REG(kmemc, CSR_HW_FREQ_CHANGE_CNTRL_OFFSET));
	/*Set AP min power state to 0 by default*/
	memc_set_min_pwr(kmemc, 0, MEMC_AP_MIN_PWR);

#ifdef CONFIG_LPDDR_DEV_TEMP
	INIT_WORK(&kmemc->memc_work, kmemc_irq_work);
	memc_enable_thermal_mgmt(kmemc);
#endif
	return 0;
}

static int kona_memc_probe(struct platform_device *pdev)
{
	u32 val, *addr;
	int size, ret;
	struct resource *iomem;
	struct kona_memc_pdata *pdata;
#ifdef CONFIG_LPDDR_DEV_TEMP
	int irq;
	int *temp_tholds;
#endif

#ifdef CONFIG_MEMC_DFS
	int i;
	u32 *dfs;
#endif
	spin_lock_init(&kona_memc.memc_lock);
	plist_head_init(&kona_memc.min_pwr_list);
	kona_memc.active_min_pwr = 0;
#ifdef CONFIG_MEMC_DFS
	kona_memc.active_dfs_opp = MEMC_OPP_NORMAL;
	kona_memc.pll_rate = MEMC_OPP_ECO; /*init to min*/
	 INIT_LIST_HEAD(&kona_memc.usr_dfs_list);
	 plist_head_init(&kona_memc.dfs_list);
	ret = sysfs_create_group(power_kobj, &kmemc_attr_group);
	if (ret) {
		pr_info("%s:sysfs_create_group failed\n", __func__);
		return ret;
	}
#endif

	if (pdev->dev.platform_data)
		pdata =	(struct kona_memc_pdata *)pdev->dev.platform_data;
	else if (pdev->dev.of_node) {
		pdata = kzalloc(sizeof(struct kona_memc_pdata),	GFP_KERNEL);

		if (!pdata)
			return -ENOMEM;

		/* Get register memory resource */
		iomem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
		if (!iomem) {
			pr_info("no mem resource\n");
			kfree(pdata);
			return -ENODEV;
		}
		pdata->memc0_ns_base = ioremap(iomem->start,
				resource_size(iomem));
		if (!pdata->memc0_ns_base) {
			pr_info("unable to map in registers\n");
			kfree(pdata);
			return -ENOMEM;
		}

		addr = (u32 *)of_get_property(pdev->dev.of_node, "chipreg_base",
				&size);
		if (!addr) {
			kfree(pdata);
			return -EINVAL;
		}
		val = *(addr + 1);
		pdata->chipreg_base = ioremap(be32_to_cpu(*addr),
				be32_to_cpu(val));

		addr = (u32 *)of_get_property(pdev->dev.of_node,
				"memc0_aphy_base", &size);
		if (!addr) {
			kfree(pdata);
			return -EINVAL;
		}
		val = *(addr + 1);
		pdata->memc0_aphy_base = ioremap(be32_to_cpu(*addr),
				be32_to_cpu(val));


		ret = of_property_read_u32(pdev->dev.of_node,
				"seq_busy_val", &val);
		if (ret != 0) {
			kfree(pdata);
			return -EINVAL;
		}
		pdata->seq_busy_val = val;

		if (of_property_read_u32(pdev->dev.of_node,
					"flags", &val)) {
			kfree(pdata);
			return -EINVAL;
		}
		pdata->flags = val;

		if (of_property_read_u32(pdev->dev.of_node,
					"max_pwr", &val)) {
			kfree(pdata);
			return -EINVAL;
		}
		pdata->max_pwr = val;

#ifdef CONFIG_LPDDR_DEV_TEMP
		/* Get interrupt number */
		irq = platform_get_irq(pdev, 0);
		if (irq == -ENXIO)
			pr_info("%s:no irq resource\n", __func__);
		pdata->irq = irq;


		if (of_property_read_u32(pdev->dev.of_node,
					"temp_period", &val)) {
			kfree(pdata);
			return -EINVAL;
		}
		pdata->temp_period = val;

		if (of_property_read_u32(pdev->dev.of_node,
					"num_thold", &val)) {
			kfree(pdata);
			return -EINVAL;
		}
		pdata->num_thold = val;

		temp_tholds = (int *)of_get_property(pdev->dev.of_node,
				"temp_tholds", &size);
		if (!temp_tholds) {
			kfree(pdata);
			return -EINVAL;
		}
		pdata->temp_tholds =
			kzalloc(sizeof(struct temp_thold)*pdata->num_thold,
					GFP_KERNEL);
		if (!pdata->temp_tholds) {
			kfree(pdata);
			return -EINVAL;
		}

		for (i = 0; i < pdata->num_thold; i++) {
			pdata->temp_tholds[i].mr4_sts =
				be32_to_cpu(*temp_tholds++);
			pdata->temp_tholds[i].action =
				be32_to_cpu(*temp_tholds++);
		}
		pr_info("%s:temp_period: %x, num_thold: %d\n", __func__,
				pdata->temp_period, pdata->num_thold);
#endif

#ifdef CONFIG_MEMC_DFS
		dfs = (u32 *)of_get_property(pdev->dev.of_node,
			"pll_freq", &size);
		for (i = 0; i < MEMC_OPP_MAX; i++) {
			pdata->pll_freq[i].ndiv = be32_to_cpu(*dfs++);
			pdata->pll_freq[i].ndiv_frac =
					be32_to_cpu(*dfs++);
			pdata->pll_freq[i].pdiv = be32_to_cpu(*dfs++);
			pdata->pll_freq[i].mdiv = be32_to_cpu(*dfs++);
		}
#endif

	} else {
		pr_info("%s: no platform data found\n", __func__);
		return -EINVAL;
	}

	kona_memc.pdata = pdata;
	kona_memc.memc0_ns_base = pdata->memc0_ns_base;
	kona_memc.chipreg_base = pdata->chipreg_base;
	kona_memc.memc0_aphy_base = pdata->memc0_aphy_base;
	memc_init(&kona_memc);
	pr_info("%s: ddr freq = %lu\n", __func__,
			compute_ddr_clk_freq(&kona_memc));
#ifdef CONFIG_LPDDR_DEV_TEMP
	/* Register interrupt handler */
	kona_memc.irq = pdata->irq;
	if (kona_memc.irq == -ENXIO)
		return 0;

	if (0 != request_irq(kona_memc.irq, kmemc_isr,
				IRQ_LEVEL | IRQF_DISABLED | IRQF_NO_SUSPEND,
				pdev->name, &kona_memc)) {
		pr_info("unable to register isr\n");
		return -EINVAL;
	}
#endif

#ifdef CONFIG_MEMC_DFS
	/*Add turbo request to boot MEMC @ turbo freq*/
	memc_add_usr_dfs_req(&kona_memc, BOOT_DFS_REQ_CLIENT, MEMC_OPP_TURBO);
#endif
	return 0;
}

static int kona_memc_remove(struct platform_device *pdev)
{
	return 0;
}

static const struct of_device_id kona_memc_dt_ids[] = {
	{ .compatible = "bcm,memc", },
	{},
};
MODULE_DEVICE_TABLE(of, kona_memc_dt_ids);

static struct platform_driver kona_memc_driver = {
	.probe = kona_memc_probe,
	.remove = kona_memc_remove,
	.driver = {
		.name = "kona_memc",
		.of_match_table = kona_memc_dt_ids,
	},
};

#ifdef CONFIG_DEBUG_FS

static int memc_dbg_get_memc_selfrefresh_state(void *data, u64 *val)
{
	u32 reg;
	struct kona_memc *kmemc = (struct kona_memc *)data;
	BUG_ON(kmemc == NULL);
	reg = readl(MEMC0_NS_REG(kmemc, CSR_HW_FREQ_CHANGE_CNTRL_OFFSET));
	*val = (reg & CSR_HW_FREQ_CHANGE_CNTRL_HW_AUTO_PWR_TRANSITION_MASK) ?
									1 : 0;
	return 0;
}

static int memc_dbg_selfrefresh_en(void *data, u64 val)
{
	struct kona_memc *kmemc = (struct kona_memc *)data;
	BUG_ON(kmemc == NULL);
	return memc_enable_selfrefresh(kmemc, (int)val);
}

DEFINE_SIMPLE_ATTRIBUTE(memc_self_refresh_en_ops,
	memc_dbg_get_memc_selfrefresh_state,
	memc_dbg_selfrefresh_en, "%llu\n");

static int memc_dbg_get_ap_min_pwr_state(void *data, u64 *val)
{
	int ret;
	struct kona_memc *kmemc = (struct kona_memc *)data;
	BUG_ON(kmemc == NULL);
	ret = memc_get_min_pwr(kmemc, MEMC_AP_MIN_PWR);
	if (IS_ERR(&ret))
		return ret;
	*val = (u64)ret;
	return 0;
}

static int memc_dbg_set_ap_min_pwr_state(void *data, u64 val)
{
	struct kona_memc *kmemc = (struct kona_memc *)data;
	BUG_ON(kmemc == NULL);
	return memc_set_min_pwr(kmemc, (u32)val, MEMC_AP_MIN_PWR);
}

DEFINE_SIMPLE_ATTRIBUTE(memc_ap_min_pwr_ops, memc_dbg_get_ap_min_pwr_state,
		memc_dbg_set_ap_min_pwr_state, "%llu\n");

static int memc_dbg_get_mdm_min_pwr_state(void *data, u64 *val)
{
	int ret;
	struct kona_memc *kmemc = (struct kona_memc *)data;
	BUG_ON(kmemc == NULL);
	ret = memc_get_min_pwr(kmemc, MEMC_CP_MIN_PWR);
	if (IS_ERR(&ret))
		return ret;
	*val = (u64)ret;
	return 0;
}

static int memc_dbg_set_mdm_min_pwr_state(void *data, u64 val)
{
	struct kona_memc *kmemc = (struct kona_memc *)data;
	BUG_ON(kmemc == NULL);
	return memc_set_min_pwr(kmemc, (u32)val, MEMC_CP_MIN_PWR);
}

DEFINE_SIMPLE_ATTRIBUTE(memc_mdm_min_pwr_ops, memc_dbg_get_mdm_min_pwr_state,
		memc_dbg_set_mdm_min_pwr_state, "%llu\n");

static int memc_dbg_get_dsp_min_pwr_state(void *data, u64 *val)
{
	int ret;
	struct kona_memc *kmemc = (struct kona_memc *)data;
	BUG_ON(kmemc == NULL);
	ret = memc_get_min_pwr(kmemc, MEMC_DSP_MIN_PWR);
	if (IS_ERR(&ret))
		return ret;
	*val = (u64)ret;
	return 0;
}

static int memc_dbg_set_dsp_min_pwr_state(void *data, u64 val)
{
	struct kona_memc *kmemc = (struct kona_memc *)data;
	BUG_ON(kmemc == NULL);
	return memc_set_min_pwr(kmemc, (u32)val, MEMC_DSP_MIN_PWR);
}

DEFINE_SIMPLE_ATTRIBUTE(memc_dsp_min_pwr_ops, memc_dbg_get_dsp_min_pwr_state,
		memc_dbg_set_dsp_min_pwr_state, "%llu\n");

static int memc_dbg_get_max_pwr_state(void *data, u64 *val)
{
	int ret;
	struct kona_memc *kmemc = (struct kona_memc *)data;
	BUG_ON(kmemc == NULL);
	ret = memc_get_max_pwr(kmemc);
	if (IS_ERR(&ret))
		return ret;
	*val = (u64)ret;
	return 0;
}

static int memc_dbg_set_max_pwr_state(void *data, u64 val)
{
	struct kona_memc *kmemc = (struct kona_memc *)data;
	BUG_ON(kmemc == NULL);
	return memc_set_max_pwr(kmemc, (u32)val);
}

DEFINE_SIMPLE_ATTRIBUTE(memc_max_pwr_ops, memc_dbg_get_max_pwr_state,
		memc_dbg_set_max_pwr_state, "%llu\n");

static int memc_dbg_get_force_max_pwr_state(void *data, u64 *val)
{
	int ret;
	struct kona_memc *kmemc = (struct kona_memc *)data;
	BUG_ON(kmemc == NULL);
	ret = memc_get_force_max_pwr_state(kmemc);
	if (IS_ERR(&ret))
		return ret;
	*val = (u64)ret;
	return 0;
}

static int memc_dbg_force_max_pwr_state(void *data, u64 val)
{
	struct kona_memc *kmemc = (struct kona_memc *)data;
	BUG_ON(kmemc == NULL);
	return memc_force_max_pwr(kmemc, (u32)val);
}

DEFINE_SIMPLE_ATTRIBUTE(memc_force_max_pwr_ops,
		memc_dbg_get_force_max_pwr_state,
		memc_dbg_force_max_pwr_state, "%llu\n");


static int memc_dbg_get_ddr_clk_freq(void *data, u64 *val)
{
	struct kona_memc *kmemc = (struct kona_memc *)data;
	*val = compute_ddr_clk_freq(kmemc);
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(memc_get_ddr_clk_freq_ops,
		memc_dbg_get_ddr_clk_freq,
		NULL, "%llu\n");

u32 kona_memc_get_ddr_clk_freq(void)
{
	return compute_ddr_clk_freq(&kona_memc);
}

#ifdef CONFIG_LPDDR_DEV_TEMP
static int memc_dbg_get_dev_temp_state(void *data, u64 *val)
{
	struct kona_memc *kmemc = (struct kona_memc *)data;
	BUG_ON(kmemc == NULL);

	*val = kmemc->dev_temp_en;
	return 0;
}
static int memc_dbg_dev_temp_en(void *data, u64 val)
{
	u32 mask0, mask1, reg;
	struct kona_memc *kmemc = (struct kona_memc *)data;
	struct kona_memc_pdata *pdata = kona_memc.pdata;

	BUG_ON(kmemc == NULL);

	if (kmemc->dev_temp_en == val)
		return 0;
	reg = readl(MEMC0_NS_REG(kmemc,
				CSR_LPDDR2_DEV_TEMP_PERIOD_OFFSET));
	if (pdata->flags & KONA_MEMC_CS0_DEV_TEMP)
		mask0 = CSR_LPDDR2_DEV_TEMP_PERIOD_ENABLE_DDR_CS0_DEV_TEMP_MASK;
	if (pdata->flags & KONA_MEMC_CS1_DEV_TEMP)
		mask1 = CSR_LPDDR2_DEV_TEMP_PERIOD_ENABLE_DDR_CS1_DEV_TEMP_MASK;
	if (val) {
		if (mask0)
			reg |= mask0;
		if (mask1)
			reg |= mask1;
	} else {
		if (mask0)
			reg &= ~mask0;
		if (mask1)
			reg &= ~mask1;
	}
	writel(reg, MEMC0_NS_REG(kmemc,
				CSR_LPDDR2_DEV_TEMP_PERIOD_OFFSET));
	kmemc->dev_temp_en = val;
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(memc_dev_temp_en_ops,
	memc_dbg_get_dev_temp_state,
	memc_dbg_dev_temp_en, "%llu\n");

static int memc_dbg_get_temp_period(void *data, u64 *val)
{
	int temp_period;
	u32 reg;
	struct kona_memc *kmemc = (struct kona_memc *)data;
	BUG_ON(kmemc == NULL);

	reg = readl(MEMC0_NS_REG(kmemc, CSR_LPDDR2_DEV_TEMP_PERIOD_OFFSET));
	reg &= CSR_LPDDR2_DEV_TEMP_PERIOD_DEV_TEMP_PERIOD_MASK;

	temp_period = (reg * 38) / 1000000; /*prints temp_period in ms*/
	pr_info("in ms");
	*val = temp_period;
	return 0;
}
static int memc_dbg_set_temp_period(void *data, u64 val)
{
	struct kona_memc *kmemc = (struct kona_memc *)data;
	BUG_ON(kmemc == NULL);

	memc_update_temp_period(kmemc, val);
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(memc_temp_period_ops,
	memc_dbg_get_temp_period,
	memc_dbg_set_temp_period, "%llu\n");
#endif

#ifdef CONFIG_MEMC_DFS
static char dbg_fs_buf[2048];
static int kona_memc_debugfs_open(struct inode *inode, struct file *file)
{
	file->private_data = inode->i_private;
	return 0;
}

static ssize_t kona_memc_dbg_get_dfs_req_list(struct file *file,
					   char __user *user_buf, size_t count,
					   loff_t *ppos)
{
	u32 len = 0;
	struct kona_memc_node *this;
	struct kona_memc *kmemc = (struct kona_memc *)file->private_data;

	plist_for_each_entry(this, &kmemc->dfs_list, node) {
		len += snprintf(dbg_fs_buf + len, sizeof(dbg_fs_buf) - len,
			"%s:%u\n",
			this->name, this->req);
	}
	return simple_read_from_buffer(user_buf, count, ppos, dbg_fs_buf,
				       len);
}

static const struct file_operations dfs_req_fops = {
	.open = kona_memc_debugfs_open,
	.read = kona_memc_dbg_get_dfs_req_list,
};

#endif /*CONFIG_MEMC_DFS*/

static struct dentry *dent_kona_memc_dir;

static int kona_menc_init_debugfs(void)
{
	dent_kona_memc_dir = debugfs_create_dir("kona_memc", 0);
	if (!dent_kona_memc_dir)
		return -ENOMEM;

	if (!debugfs_create_file("self_refresh_en", S_IRUGO | S_IWUSR,
				dent_kona_memc_dir, &kona_memc ,
				&memc_self_refresh_en_ops))
		return -ENOMEM;

	if (!debugfs_create_file("ap_min_pwr", S_IRUGO | S_IWUSR,
				dent_kona_memc_dir, &kona_memc ,
				&memc_ap_min_pwr_ops))
		return -ENOMEM;
	if (!debugfs_create_file("mdm_min_pwr", S_IRUGO | S_IWUSR,
				dent_kona_memc_dir, &kona_memc ,
				&memc_mdm_min_pwr_ops))
		return -ENOMEM;
	if (!debugfs_create_file("dsp_min_pwr", S_IRUGO | S_IWUSR,
				dent_kona_memc_dir, &kona_memc ,
				&memc_dsp_min_pwr_ops))
		return -ENOMEM;
	if (!debugfs_create_file("max_pwr_state", S_IRUGO | S_IWUSR,
				dent_kona_memc_dir, &kona_memc ,
				&memc_max_pwr_ops))
		return -ENOMEM;

	if (!debugfs_create_file("force_max_power", S_IRUSR | S_IWUSR,
				 dent_kona_memc_dir, &kona_memc,
				 &memc_force_max_pwr_ops))
		return -ENOMEM;

	if (!debugfs_create_file("ddr_pll_rate", S_IRUGO,
				 dent_kona_memc_dir, &kona_memc,
				 &memc_get_ddr_clk_freq_ops))
		return -ENOMEM;

#ifdef CONFIG_LPDDR_DEV_TEMP
	if (!debugfs_create_file("dev_temp_en", S_IRUGO | S_IWUSR,
				dent_kona_memc_dir, &kona_memc ,
				&memc_dev_temp_en_ops))
		return -ENOMEM;
	if (!debugfs_create_file("temp_period", S_IRUGO | S_IWUSR,
				dent_kona_memc_dir, &kona_memc ,
				&memc_temp_period_ops))
		return -ENOMEM;
#endif

#ifdef CONFIG_MEMC_DFS
	if (!debugfs_create_file("dfs_req", S_IRUGO,
				 dent_kona_memc_dir, &kona_memc,
				 &dfs_req_fops))
		return -ENOMEM;

#endif

	return 0;
}
#endif /*CONFIG_DEBUG_FS*/

static int __init kona_memc_init(void)
{
	int ret;
	pr_info("kona_memc : inside %s\n", __func__);
	ret = platform_driver_register(&kona_memc_driver);
#ifdef CONFIG_DEBUG_FS
	if (!ret)
		kona_menc_init_debugfs();
#endif
	return ret;
}

static void __exit kona_memc_exit(void)
{
	platform_driver_unregister(&kona_memc_driver);
}

arch_initcall(kona_memc_init);
module_exit(kona_memc_exit);

MODULE_DESCRIPTION("KONA MEMC DRIVER");
