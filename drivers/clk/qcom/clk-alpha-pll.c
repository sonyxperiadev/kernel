// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2015, 2018, The Linux Foundation. All rights reserved.
 */

#include <linux/kernel.h>
#include <linux/export.h>
#include <linux/clk-provider.h>
#include <linux/regmap.h>
#include <linux/delay.h>

#include "clk-alpha-pll.h"
#include "common.h"

#define PLL_MODE(p)		((p)->offset + 0x0)
# define PLL_OUTCTRL		BIT(0)
# define PLL_BYPASSNL		BIT(1)
# define PLL_RESET_N		BIT(2)
# define PLL_OFFLINE_REQ	BIT(7)
# define PLL_LOCK_COUNT_SHIFT	8
# define PLL_LOCK_COUNT_MASK	0x3f
# define PLL_BIAS_COUNT_SHIFT	14
# define PLL_BIAS_COUNT_MASK	0x3f
# define PLL_VOTE_FSM_ENA	BIT(20)
# define PLL_FSM_ENA		BIT(20)
# define PLL_VOTE_FSM_RESET	BIT(21)
# define PLL_UPDATE		BIT(22)
# define PLL_UPDATE_BYPASS	BIT(23)
# define PLL_OFFLINE_ACK	BIT(28)
# define ALPHA_PLL_ACK_LATCH	BIT(29)
# define PLL_ACTIVE_FLAG	BIT(30)
# define PLL_LOCK_DET		BIT(31)

#define PLL_L_VAL(p)		((p)->offset + (p)->regs[PLL_OFF_L_VAL])
#define PLL_ALPHA_VAL(p)	((p)->offset + (p)->regs[PLL_OFF_ALPHA_VAL])
#define PLL_ALPHA_VAL_U(p)	((p)->offset + (p)->regs[PLL_OFF_ALPHA_VAL_U])

#define PLL_USER_CTL(p)		((p)->offset + (p)->regs[PLL_OFF_USER_CTL])
# define PLL_POST_DIV_SHIFT	8
# define PLL_POST_DIV_ODD_SHIFT	12
# define PLL_POST_DIV_MASK(p)	GENMASK((p)->width, 0)
# define PLL_STD_POST_DIV_MASK	0xf
# define PLL_ALPHA_EN		BIT(24)
# define PLL_ALPHA_MODE		BIT(25)
# define PLL_VCO_SHIFT		20
# define PLL_VCO_MASK		0x3
# define PLL_OUT_MASK		0x7
# define PLL_REGERA_OUT_MASK	0x9

#define PLL_USER_CTL_U(p)	((p)->offset + (p)->regs[PLL_OFF_USER_CTL_U])
# define PLL_ALPHA_STATE_READ_EN BIT(4)
# define PLL_ALPHA_CAL_MASK	(0x7 << 3)
# define PLL_ALPHA_CAL_CTRL	2

#define PLL_USER_CTL_U1(p)	((p)->offset + (p)->regs[PLL_OFF_USER_CTL_U1])

#define PLL_CONFIG_CTL(p)	((p)->offset + (p)->regs[PLL_OFF_CONFIG_CTL])
#define PLL_CONFIG_CTL_U(p)	((p)->offset + (p)->regs[PLL_OFF_CONFIG_CTL_U])
#define PLL_CONFIG_CTL_U1(p)	((p)->offset + (p)->regs[PLL_OFF_CONFIG_CTL_U1])
#define PLL_TEST_CTL(p)		((p)->offset + (p)->regs[PLL_OFF_TEST_CTL])
#define PLL_TEST_CTL_U(p)	((p)->offset + (p)->regs[PLL_OFF_TEST_CTL_U])
#define PLL_TEST_CTL_U1(p)	((p)->offset + (p)->regs[PLL_OFF_TEST_CTL_U1])

#define PLL_STATUS(p)		((p)->offset + (p)->regs[PLL_OFF_STATUS])
# define PLL_PCAL_DONE		BIT(26)

#define PLL_OPMODE(p)		((p)->offset + (p)->regs[PLL_OFF_OPMODE])
# define PLL_STANDBY		0x0
# define PLL_RUN		0x1

#define PLL_FRAC(p)		((p)->offset + (p)->regs[PLL_OFF_FRAC])
# define ALPHA_16_BIT_PLL_RATE_MARGIN 500

#define PLL_CAL_L_VAL(p)	((p)->offset + (p)->regs[PLL_OFF_CAL_L_VAL])
# define PLL_CAL		0x44
# define FABIA_PLL_CAL		0x3f

#define PLL_STANDBY	0x0
#define PLL_RUN	0x1

const u8 clk_alpha_pll_regs[][PLL_OFF_MAX_REGS] = {
	[CLK_ALPHA_PLL_TYPE_DEFAULT] =  {
		[PLL_OFF_L_VAL] = 0x04,
		[PLL_OFF_ALPHA_VAL] = 0x08,
		[PLL_OFF_ALPHA_VAL_U] = 0x0c,
		[PLL_OFF_USER_CTL] = 0x10,
		[PLL_OFF_USER_CTL_U] = 0x14,
		[PLL_OFF_CONFIG_CTL] = 0x18,
		[PLL_OFF_TEST_CTL] = 0x1c,
		[PLL_OFF_TEST_CTL_U] = 0x20,
		[PLL_OFF_STATUS] = 0x24,
	},
	[CLK_ALPHA_PLL_TYPE_HUAYRA] =  {
		[PLL_OFF_L_VAL] = 0x04,
		[PLL_OFF_ALPHA_VAL] = 0x08,
		[PLL_OFF_USER_CTL] = 0x10,
		[PLL_OFF_CONFIG_CTL] = 0x14,
		[PLL_OFF_CONFIG_CTL_U] = 0x18,
		[PLL_OFF_TEST_CTL] = 0x1c,
		[PLL_OFF_TEST_CTL_U] = 0x20,
		[PLL_OFF_STATUS] = 0x24,
	},
	[CLK_ALPHA_PLL_TYPE_BRAMMO] =  {
		[PLL_OFF_L_VAL] = 0x04,
		[PLL_OFF_ALPHA_VAL] = 0x08,
		[PLL_OFF_ALPHA_VAL_U] = 0x0c,
		[PLL_OFF_USER_CTL] = 0x10,
		[PLL_OFF_CONFIG_CTL] = 0x18,
		[PLL_OFF_TEST_CTL] = 0x1c,
		[PLL_OFF_STATUS] = 0x24,
	},
	[CLK_ALPHA_PLL_TYPE_FABIA] =  {
		[PLL_OFF_L_VAL] = 0x04,
		[PLL_OFF_CAL_L_VAL] = 0x08,
		[PLL_OFF_USER_CTL] = 0x0c,
		[PLL_OFF_USER_CTL_U] = 0x10,
		[PLL_OFF_CONFIG_CTL] = 0x14,
		[PLL_OFF_CONFIG_CTL_U] = 0x18,
		[PLL_OFF_TEST_CTL] = 0x1c,
		[PLL_OFF_TEST_CTL_U] = 0x20,
		[PLL_OFF_STATUS] = 0x24,
		[PLL_OFF_OPMODE] = 0x2c,
		[PLL_OFF_FRAC] = 0x38,
	},
	[CLK_ALPHA_PLL_TYPE_TRION] =  {
		[PLL_OFF_L_VAL] = 0x04,
		[PLL_OFF_CAL_L_VAL] = 0x08,
		[PLL_OFF_USER_CTL] = 0xc,
		[PLL_OFF_USER_CTL_U] = 0x10,
		[PLL_OFF_USER_CTL_U1] = 0x14,
		[PLL_OFF_CONFIG_CTL] = 0x18,
		[PLL_OFF_CONFIG_CTL_U] = 0x1c,
		[PLL_OFF_CONFIG_CTL_U1] = 0x20,
		[PLL_OFF_TEST_CTL] = 0x24,
		[PLL_OFF_TEST_CTL_U] = 0x28,
		[PLL_OFF_TEST_CTL_U1] = 0x2c,
		[PLL_OFF_STATUS] = 0x30,
		[PLL_OFF_OPMODE] = 0x38,
		[PLL_OFF_ALPHA_VAL] = 0x40,
	},
	[CLK_ALPHA_PLL_TYPE_REGERA] =  {
		[PLL_OFF_L_VAL] = 0x04,
		[PLL_OFF_ALPHA_VAL] = 0x08,
		[PLL_OFF_USER_CTL] = 0xc,
		[PLL_OFF_CONFIG_CTL] = 0x10,
		[PLL_OFF_CONFIG_CTL_U] = 0x14,
		[PLL_OFF_CONFIG_CTL_U1] = 0x18,
		[PLL_OFF_TEST_CTL] = 0x1c,
		[PLL_OFF_TEST_CTL_U] = 0x20,
		[PLL_OFF_TEST_CTL_U1] = 0x24,
		[PLL_OFF_OPMODE] = 0x28,
	},
	[CLK_ALPHA_PLL_TYPE_AGERA] =  {
		[PLL_OFF_L_VAL] = 0x04,
		[PLL_OFF_ALPHA_VAL] = 0x08,
		[PLL_OFF_USER_CTL] = 0xc,
		[PLL_OFF_CONFIG_CTL] = 0x10,
		[PLL_OFF_CONFIG_CTL_U] = 0x14,
		[PLL_OFF_CONFIG_CTL_U1] = 0x18,
		[PLL_OFF_TEST_CTL] = 0x1c,
		[PLL_OFF_TEST_CTL_U] = 0x20,
		[PLL_OFF_TEST_CTL_U1] = 0x24,
		[PLL_OFF_OPMODE] = 0x28,
	},
};
EXPORT_SYMBOL_GPL(clk_alpha_pll_regs);

/*
 * Even though 40 bits are present, use only 32 for ease of calculation.
 */
#define ALPHA_REG_BITWIDTH	40
#define ALPHA_REG_16BIT_WIDTH	16
#define ALPHA_BITWIDTH		32U
#define ALPHA_SHIFT(w)		min(w, ALPHA_BITWIDTH)

#define PLL_HUAYRA_M_WIDTH		8
#define PLL_HUAYRA_M_SHIFT		8
#define PLL_HUAYRA_M_MASK		0xff
#define PLL_HUAYRA_N_SHIFT		0
#define PLL_HUAYRA_N_MASK		0xff
#define PLL_HUAYRA_ALPHA_WIDTH		16

#define pll_alpha_width(p)					\
		((PLL_ALPHA_VAL_U(p) - PLL_ALPHA_VAL(p) == 4) ?	\
				 ALPHA_REG_BITWIDTH : ALPHA_REG_16BIT_WIDTH)

#define pll_has_64bit_config(p)	((PLL_CONFIG_CTL_U(p) - PLL_CONFIG_CTL(p)) == 4)

#define to_clk_alpha_pll(_hw) container_of(to_clk_regmap(_hw), \
					   struct clk_alpha_pll, clkr)

#define to_clk_alpha_pll_postdiv(_hw) container_of(to_clk_regmap(_hw), \
					   struct clk_alpha_pll_postdiv, clkr)

static int wait_for_pll(struct clk_alpha_pll *pll, u32 mask, bool inverse,
			const char *action)
{
	u32 val;
	int count;
	int ret;
	const char *name = clk_hw_get_name(&pll->clkr.hw);

	ret = regmap_read(pll->clkr.regmap, PLL_MODE(pll), &val);
	if (ret)
		return ret;

	for (count = 100; count > 0; count--) {
		ret = regmap_read(pll->clkr.regmap, PLL_MODE(pll), &val);
		if (ret)
			return ret;
		if (inverse && !(val & mask))
			return 0;
		else if ((val & mask) == mask)
			return 0;

		udelay(1);
	}

	WARN(1, "%s failed to %s!\n", name, action);
	return -ETIMEDOUT;
}

#define wait_for_pll_enable_active(pll) \
	wait_for_pll(pll, PLL_ACTIVE_FLAG, 0, "enable")

#define wait_for_pll_enable_lock(pll) \
	wait_for_pll(pll, PLL_LOCK_DET, 0, "enable")

#define wait_for_pll_disable(pll) \
	wait_for_pll(pll, PLL_ACTIVE_FLAG, 1, "disable")

#define wait_for_pll_offline(pll) \
	wait_for_pll(pll, PLL_OFFLINE_ACK, 0, "offline")

#define wait_for_pll_update(pll) \
	wait_for_pll(pll, PLL_UPDATE, 1, "update")

#define wait_for_pll_update_ack_set(pll) \
	wait_for_pll(pll, ALPHA_PLL_ACK_LATCH, 0, "update_ack_set")

#define wait_for_pll_update_ack_clear(pll) \
	wait_for_pll(pll, ALPHA_PLL_ACK_LATCH, 1, "update_ack_clear")

void clk_alpha_pll_configure(struct clk_alpha_pll *pll, struct regmap *regmap,
			     const struct alpha_pll_config *config)
{
	u32 val, mask;

	if (!config) {
		WARN(1, "PLL has no config!");
		return;
	}

	regmap_write(regmap, PLL_L_VAL(pll), config->l);
	regmap_write(regmap, PLL_ALPHA_VAL(pll), config->alpha);
	regmap_write(regmap, PLL_CONFIG_CTL(pll), config->config_ctl_val);

	if (pll_has_64bit_config(pll))
		regmap_write(regmap, PLL_CONFIG_CTL_U(pll),
			     config->config_ctl_hi_val);

	if (pll_alpha_width(pll) > 32)
		regmap_write(regmap, PLL_ALPHA_VAL_U(pll), config->alpha_hi);

	val = config->main_output_mask;
	val |= config->aux_output_mask;
	val |= config->aux2_output_mask;
	val |= config->early_output_mask;
	val |= config->pre_div_val;
	val |= config->post_div_val;
	val |= config->vco_val;
	val |= config->alpha_en_mask;
	val |= config->alpha_mode_mask;

	mask = config->main_output_mask;
	mask |= config->aux_output_mask;
	mask |= config->aux2_output_mask;
	mask |= config->early_output_mask;
	mask |= config->pre_div_mask;
	mask |= config->post_div_mask;
	mask |= config->vco_mask;

	regmap_update_bits(regmap, PLL_USER_CTL(pll), mask, val);

	if (pll->flags & SUPPORTS_DYNAMIC_UPDATE)
		regmap_update_bits(regmap, PLL_MODE(pll),
					PLL_UPDATE_BYPASS, PLL_UPDATE_BYPASS);

	if (pll->flags & SUPPORTS_FSM_MODE)
		qcom_pll_set_fsm_mode(regmap, PLL_MODE(pll), 6, 0);
}
EXPORT_SYMBOL_GPL(clk_alpha_pll_configure);

static int clk_alpha_pll_hwfsm_enable(struct clk_hw *hw)
{
	int ret;
	struct clk_alpha_pll *pll = to_clk_alpha_pll(hw);
	u32 val;

	ret = regmap_read(pll->clkr.regmap, PLL_MODE(pll), &val);
	if (ret)
		return ret;

	val |= PLL_FSM_ENA;

	if (pll->flags & SUPPORTS_OFFLINE_REQ)
		val &= ~PLL_OFFLINE_REQ;

	ret = regmap_write(pll->clkr.regmap, PLL_MODE(pll), val);
	if (ret)
		return ret;

	/* Make sure enable request goes through before waiting for update */
	mb();

	return wait_for_pll_enable_active(pll);
}

static void clk_alpha_pll_hwfsm_disable(struct clk_hw *hw)
{
	int ret;
	struct clk_alpha_pll *pll = to_clk_alpha_pll(hw);
	u32 val;

	ret = regmap_read(pll->clkr.regmap, PLL_MODE(pll), &val);
	if (ret)
		return;

	if (pll->flags & SUPPORTS_OFFLINE_REQ) {
		ret = regmap_update_bits(pll->clkr.regmap, PLL_MODE(pll),
					 PLL_OFFLINE_REQ, PLL_OFFLINE_REQ);
		if (ret)
			return;

		ret = wait_for_pll_offline(pll);
		if (ret)
			return;
	}

	/* Disable hwfsm */
	ret = regmap_update_bits(pll->clkr.regmap, PLL_MODE(pll),
				 PLL_FSM_ENA, 0);
	if (ret)
		return;

	wait_for_pll_disable(pll);
}

static int pll_is_enabled(struct clk_alpha_pll *pll, u32 mask)
{
	int ret;
	u32 val;

	ret = regmap_read(pll->clkr.regmap, PLL_MODE(pll), &val);
	if (ret)
		return ret;

	return !!(val & mask);
}

static int clk_alpha_pll_hwfsm_is_enabled(struct clk_hw *hw)
{
	return pll_is_enabled(to_clk_alpha_pll(hw), PLL_ACTIVE_FLAG);
}

static int clk_alpha_pll_is_enabled(struct clk_hw *hw)
{
	return pll_is_enabled(to_clk_alpha_pll(hw), PLL_LOCK_DET);
}

static int clk_alpha_pll_enable(struct clk_hw *hw)
{
	int ret;
	struct clk_alpha_pll *pll = to_clk_alpha_pll(hw);
	u32 val, mask, l_val;

	mask = PLL_OUTCTRL | PLL_RESET_N | PLL_BYPASSNL;
	ret = regmap_read(pll->clkr.regmap, PLL_MODE(pll), &val);
	if (ret)
		return ret;

	/* If in FSM mode, just vote for it */
	if (val & PLL_VOTE_FSM_ENA) {
		ret = clk_enable_regmap(hw);
		if (ret)
			return ret;
		ret = wait_for_pll_enable_active(pll);
		if (ret == 0)
			if (pll->flags & SUPPORTS_FSM_VOTE)
				*pll->soft_vote |= (pll->soft_vote_mask);
		return ret;
	}

	/* Skip if already enabled */
	if ((val & mask) == mask)
		return 0;

	ret = regmap_read(pll->clkr.regmap, PLL_L_VAL(pll), &l_val);
	if (ret)
		return ret;

	/* PLL has lost L value and needs reconfiguration */
	if (!l_val)
		clk_alpha_pll_configure(pll, pll->clkr.regmap, pll->config);

	ret = regmap_update_bits(pll->clkr.regmap, PLL_MODE(pll),
				 PLL_BYPASSNL, PLL_BYPASSNL);
	if (ret)
		return ret;

	/*
	 * H/W requires a 5us delay between disabling the bypass and
	 * de-asserting the reset.
	 */
	mb();
	udelay(5);

	ret = regmap_update_bits(pll->clkr.regmap, PLL_MODE(pll),
				 PLL_RESET_N, PLL_RESET_N);
	if (ret)
		return ret;

	ret = wait_for_pll_enable_lock(pll);
	if (ret)
		return ret;

	ret = regmap_update_bits(pll->clkr.regmap, PLL_MODE(pll),
				 PLL_OUTCTRL, PLL_OUTCTRL);

	/* Ensure that the write above goes through before returning. */
	mb();
	return ret;
}

static void clk_alpha_pll_disable(struct clk_hw *hw)
{
	int ret;
	struct clk_alpha_pll *pll = to_clk_alpha_pll(hw);
	u32 val, mask;

	ret = regmap_read(pll->clkr.regmap, PLL_MODE(pll), &val);
	if (ret)
		return;

	/* If in FSM mode, just unvote it */
	if (val & PLL_VOTE_FSM_ENA) {
		if (pll->flags & SUPPORTS_FSM_VOTE) {
			*pll->soft_vote &= ~(pll->soft_vote_mask);
			if (!*pll->soft_vote)
				clk_disable_regmap(hw);
		} else {
			clk_disable_regmap(hw);
		}
		return;
	}

	mask = PLL_OUTCTRL;
	regmap_update_bits(pll->clkr.regmap, PLL_MODE(pll), mask, 0);

	/* Delay of 2 output clock ticks required until output is disabled */
	mb();
	udelay(1);

	mask = PLL_RESET_N | PLL_BYPASSNL;
	regmap_update_bits(pll->clkr.regmap, PLL_MODE(pll), mask, 0);
}

static unsigned long
alpha_pll_calc_rate(u64 prate, u32 l, u32 a, u32 alpha_width)
{
	return (prate * l) + ((prate * a) >> ALPHA_SHIFT(alpha_width));
}

static unsigned long
alpha_pll_round_rate(unsigned long rate, unsigned long prate, u32 *l, u64 *a,
		     u32 alpha_width)
{
	u64 remainder;
	u64 quotient;

	/*
	 * The PLLs parent rate is zero probably since the parent hasn't
	 * registered yet. Return early with the requested rate.
	 */
	if (!prate) {
		pr_warn("PLLs parent rate hasn't been initialized.\n");
		return rate;
	}

	quotient = rate;
	remainder = do_div(quotient, prate);
	*l = quotient;

	if (!remainder) {
		*a = 0;
		return rate;
	}

	/* Upper ALPHA_BITWIDTH bits of Alpha */
	quotient = remainder << ALPHA_SHIFT(alpha_width);

	remainder = do_div(quotient, prate);

	if (remainder)
		quotient++;

	*a = quotient;
	return alpha_pll_calc_rate(prate, *l, *a, alpha_width);
}

static const struct pll_vco *
alpha_pll_find_vco(const struct clk_alpha_pll *pll, unsigned long rate)
{
	const struct pll_vco *v = pll->vco_table;
	const struct pll_vco *end = v + pll->num_vco;

	for (; v < end; v++)
		if (rate >= v->min_freq && rate <= v->max_freq)
			return v;

	return NULL;
}

static unsigned long
clk_alpha_pll_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
	u32 l, low, high, ctl;
	u64 a = 0, prate = parent_rate;
	struct clk_alpha_pll *pll = to_clk_alpha_pll(hw);
	u32 alpha_width = pll_alpha_width(pll);

	regmap_read(pll->clkr.regmap, PLL_L_VAL(pll), &l);

	regmap_read(pll->clkr.regmap, PLL_USER_CTL(pll), &ctl);
	if (ctl & PLL_ALPHA_EN) {
		regmap_read(pll->clkr.regmap, PLL_ALPHA_VAL(pll), &low);
		if (alpha_width > 32) {
			regmap_read(pll->clkr.regmap, PLL_ALPHA_VAL_U(pll),
				    &high);
			a = (u64)high << 32 | low;
		} else {
			a = low & GENMASK(alpha_width - 1, 0);
		}

		if (alpha_width > ALPHA_BITWIDTH)
			a >>= alpha_width - ALPHA_BITWIDTH;
	}

	return alpha_pll_calc_rate(prate, l, a, alpha_width);
}


static int __clk_alpha_pll_update_latch(struct clk_alpha_pll *pll)
{
	int ret;
	u32 mode;

	regmap_read(pll->clkr.regmap, PLL_MODE(pll), &mode);

	/* Latch the input to the PLL */
	regmap_update_bits(pll->clkr.regmap, PLL_MODE(pll), PLL_UPDATE,
			   PLL_UPDATE);

	/* Wait for 2 reference cycle before checking ACK bit */
	udelay(1);

	/*
	 * PLL will latch the new L, Alpha and freq control word.
	 * PLL will respond by raising PLL_ACK_LATCH output when new programming
	 * has been latched in and PLL is being updated. When
	 * UPDATE_LOGIC_BYPASS bit is not set, PLL_UPDATE will be cleared
	 * automatically by hardware when PLL_ACK_LATCH is asserted by PLL.
	 */
	if (mode & PLL_UPDATE_BYPASS) {
		ret = wait_for_pll_update_ack_set(pll);
		if (ret)
			return ret;

		regmap_update_bits(pll->clkr.regmap, PLL_MODE(pll), PLL_UPDATE, 0);
	} else {
		ret = wait_for_pll_update(pll);
		if (ret)
			return ret;
	}

	if (pll->flags & SUPPORTS_DYNAMIC_UPDATE)
		ret = wait_for_pll_enable_lock(pll);
	else
		ret = wait_for_pll_update_ack_clear(pll);
	if (ret)
		return ret;

	/* Wait for PLL output to stabilize */
	udelay(100);

	return 0;
}

static const struct pll_vco_data
	*find_vco_data(const struct pll_vco_data *data,
			unsigned long rate, size_t size)
{
	int i;

	if (!data)
		return NULL;

	for (i = 0; i < size; i++) {
		if (rate == data[i].freq)
			return &data[i];
	}

	return &data[i - 1];
}

static void __clk_alpha_pll_set_manual_vco(struct clk_alpha_pll *pll,
					   unsigned long rate)
{
	const struct pll_vco_data *data;

	data = find_vco_data(pll->vco_data, rate, pll->num_vco_data);
	if (data) {
		if (data->freq == rate)
			regmap_update_bits(pll->clkr.regmap, PLL_USER_CTL(pll),
				PLL_STD_POST_DIV_MASK << PLL_POST_DIV_SHIFT,
				data->post_div_val << PLL_POST_DIV_SHIFT);
		else
			regmap_update_bits(pll->clkr.regmap, PLL_USER_CTL(pll),
				PLL_STD_POST_DIV_MASK << PLL_POST_DIV_SHIFT,
				0x0 << PLL_VCO_SHIFT);
	}
}

static int __clk_alpha_pll_set_rate(struct clk_hw *hw, unsigned long rate,
				    unsigned long prate,
				    int (*is_enabled)(struct clk_hw *))
{
	struct clk_alpha_pll *pll = to_clk_alpha_pll(hw);
	const struct pll_vco *vco;
	u32 l, alpha_width = pll_alpha_width(pll);
	u64 a;
	bool pll_enabled = false;
	int rc = 0;

	rate = alpha_pll_round_rate(rate, prate, &l, &a, alpha_width);
	vco = alpha_pll_find_vco(pll, rate);
	if (pll->vco_table && !vco) {
		pr_err("alpha pll not in a valid vco range\n");
		return -EINVAL;
	}

	pll_enabled = is_enabled(&pll->clkr.hw);

	/*
	 * For PLLs that do not support dynamic programming (dynamic_update
	 * is not set), ensure PLL is off before changing rate. For
	 * optimization reasons, assume no downstream clock is actively
	 * using it.
	 */
	if (pll_enabled && !(pll->flags & SUPPORTS_DYNAMIC_UPDATE))
		hw->init->ops->disable(hw);

	regmap_write(pll->clkr.regmap, PLL_L_VAL(pll), l);

	if (alpha_width > ALPHA_BITWIDTH)
		a <<= alpha_width - ALPHA_BITWIDTH;

	regmap_write(pll->clkr.regmap, PLL_ALPHA_VAL(pll), a);

	if (alpha_width > ALPHA_BITWIDTH)
		regmap_write(pll->clkr.regmap, PLL_ALPHA_VAL_U(pll), a >> 32);

	if (vco) {
		regmap_update_bits(pll->clkr.regmap, PLL_USER_CTL(pll),
				   PLL_VCO_MASK << PLL_VCO_SHIFT,
				   vco->val << PLL_VCO_SHIFT);
	}

	__clk_alpha_pll_set_manual_vco(pll, rate);

	regmap_update_bits(pll->clkr.regmap, PLL_USER_CTL(pll),
			   PLL_ALPHA_EN, PLL_ALPHA_EN);

	if (clk_hw_is_enabled(hw)) {
		if (!(pll->flags & SUPPORTS_DYNAMIC_UPDATE))
			rc = hw->init->ops->enable(hw);
		else
			rc = __clk_alpha_pll_update_latch(pll);
	}

	return rc;
}

static int clk_alpha_pll_set_rate(struct clk_hw *hw, unsigned long rate,
				  unsigned long prate)
{
	return __clk_alpha_pll_set_rate(hw, rate, prate,
					clk_alpha_pll_is_enabled);
}

static int clk_alpha_pll_hwfsm_set_rate(struct clk_hw *hw, unsigned long rate,
					unsigned long prate)
{
	return __clk_alpha_pll_set_rate(hw, rate, prate,
					clk_alpha_pll_hwfsm_is_enabled);
}

static long clk_alpha_pll_round_rate(struct clk_hw *hw, unsigned long rate,
				     unsigned long *prate)
{
	struct clk_alpha_pll *pll = to_clk_alpha_pll(hw);
	u32 l, alpha_width = pll_alpha_width(pll);
	u64 a;
	unsigned long min_freq, max_freq;

	rate = alpha_pll_round_rate(rate, *prate, &l, &a, alpha_width);
	if (!pll->vco_table || alpha_pll_find_vco(pll, rate))
		return rate;

	min_freq = pll->vco_table[0].min_freq;
	max_freq = pll->vco_table[pll->num_vco - 1].max_freq;

	return clamp(rate, min_freq, max_freq);
}

static unsigned long
alpha_huayra_pll_calc_rate(u64 prate, u32 l, u32 a)
{
	/*
	 * a contains 16 bit alpha_val in two’s compliment number in the range
	 * of [-0.5, 0.5).
	 */
	if (a >= BIT(PLL_HUAYRA_ALPHA_WIDTH - 1))
		l -= 1;

	return (prate * l) + (prate * a >> PLL_HUAYRA_ALPHA_WIDTH);
}

static unsigned long
alpha_huayra_pll_round_rate(unsigned long rate, unsigned long prate,
			    u32 *l, u32 *a)
{
	u64 remainder;
	u64 quotient;

	quotient = rate;
	remainder = do_div(quotient, prate);
	*l = quotient;

	if (!remainder) {
		*a = 0;
		return rate;
	}

	quotient = remainder << PLL_HUAYRA_ALPHA_WIDTH;
	remainder = do_div(quotient, prate);

	if (remainder)
		quotient++;

	/*
	 * alpha_val should be in two’s compliment number in the range
	 * of [-0.5, 0.5) so if quotient >= 0.5 then increment the l value
	 * since alpha value will be subtracted in this case.
	 */
	if (quotient >= BIT(PLL_HUAYRA_ALPHA_WIDTH - 1))
		*l += 1;

	*a = quotient;
	return alpha_huayra_pll_calc_rate(prate, *l, *a);
}

static unsigned long
alpha_pll_huayra_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
	u64 rate = parent_rate, tmp;
	struct clk_alpha_pll *pll = to_clk_alpha_pll(hw);
	u32 l, alpha = 0, ctl, alpha_m, alpha_n;

	regmap_read(pll->clkr.regmap, PLL_L_VAL(pll), &l);
	regmap_read(pll->clkr.regmap, PLL_USER_CTL(pll), &ctl);

	if (ctl & PLL_ALPHA_EN) {
		regmap_read(pll->clkr.regmap, PLL_ALPHA_VAL(pll), &alpha);
		/*
		 * Depending upon alpha_mode, it can be treated as M/N value or
		 * as a two’s compliment number. When alpha_mode=1,
		 * pll_alpha_val<15:8>=M and pll_apla_val<7:0>=N
		 *
		 *		Fout=FIN*(L+(M/N))
		 *
		 * M is a signed number (-128 to 127) and N is unsigned
		 * (0 to 255). M/N has to be within +/-0.5.
		 *
		 * When alpha_mode=0, it is a two’s compliment number in the
		 * range [-0.5, 0.5).
		 *
		 *		Fout=FIN*(L+(alpha_val)/2^16)
		 *
		 * where alpha_val is two’s compliment number.
		 */
		if (!(ctl & PLL_ALPHA_MODE))
			return alpha_huayra_pll_calc_rate(rate, l, alpha);

		alpha_m = alpha >> PLL_HUAYRA_M_SHIFT & PLL_HUAYRA_M_MASK;
		alpha_n = alpha >> PLL_HUAYRA_N_SHIFT & PLL_HUAYRA_N_MASK;

		rate *= l;
		tmp = parent_rate;
		if (alpha_m >= BIT(PLL_HUAYRA_M_WIDTH - 1)) {
			alpha_m = BIT(PLL_HUAYRA_M_WIDTH) - alpha_m;
			tmp *= alpha_m;
			do_div(tmp, alpha_n);
			rate -= tmp;
		} else {
			tmp *= alpha_m;
			do_div(tmp, alpha_n);
			rate += tmp;
		}

		return rate;
	}

	return alpha_huayra_pll_calc_rate(rate, l, alpha);
}

static int alpha_pll_huayra_set_rate(struct clk_hw *hw, unsigned long rate,
				     unsigned long prate)
{
	struct clk_alpha_pll *pll = to_clk_alpha_pll(hw);
	u32 l, a, ctl, cur_alpha = 0;

	rate = alpha_huayra_pll_round_rate(rate, prate, &l, &a);

	regmap_read(pll->clkr.regmap, PLL_USER_CTL(pll), &ctl);

	if (ctl & PLL_ALPHA_EN)
		regmap_read(pll->clkr.regmap, PLL_ALPHA_VAL(pll), &cur_alpha);

	/*
	 * Huayra PLL supports PLL dynamic programming. User can change L_VAL,
	 * without having to go through the power on sequence.
	 */
	if (clk_alpha_pll_is_enabled(hw)) {
		if (cur_alpha != a) {
			pr_err("clock needs to be gated %s\n",
			       clk_hw_get_name(hw));
			return -EBUSY;
		}

		regmap_write(pll->clkr.regmap, PLL_L_VAL(pll), l);
		/* Ensure that the write above goes to detect L val change. */
		mb();
		return wait_for_pll_enable_lock(pll);
	}

	regmap_write(pll->clkr.regmap, PLL_L_VAL(pll), l);
	regmap_write(pll->clkr.regmap, PLL_ALPHA_VAL(pll), a);

	if (a == 0)
		regmap_update_bits(pll->clkr.regmap, PLL_USER_CTL(pll),
				   PLL_ALPHA_EN, 0x0);
	else
		regmap_update_bits(pll->clkr.regmap, PLL_USER_CTL(pll),
				   PLL_ALPHA_EN | PLL_ALPHA_MODE, PLL_ALPHA_EN);

	return 0;
}

static long alpha_pll_huayra_round_rate(struct clk_hw *hw, unsigned long rate,
					unsigned long *prate)
{
	u32 l, a;

	return alpha_huayra_pll_round_rate(rate, *prate, &l, &a);
}

const struct clk_ops clk_alpha_pll_fixed_ops = {
	.enable = clk_alpha_pll_enable,
	.disable = clk_alpha_pll_disable,
	.is_enabled = clk_alpha_pll_is_enabled,
	.recalc_rate = clk_alpha_pll_recalc_rate,
};
EXPORT_SYMBOL_GPL(clk_alpha_pll_fixed_ops);

const struct clk_ops clk_alpha_pll_ops = {
	.enable = clk_alpha_pll_enable,
	.disable = clk_alpha_pll_disable,
	.is_enabled = clk_alpha_pll_is_enabled,
	.recalc_rate = clk_alpha_pll_recalc_rate,
	.round_rate = clk_alpha_pll_round_rate,
	.set_rate = clk_alpha_pll_set_rate,
};
EXPORT_SYMBOL_GPL(clk_alpha_pll_ops);

const struct clk_ops clk_alpha_pll_huayra_ops = {
	.enable = clk_alpha_pll_enable,
	.disable = clk_alpha_pll_disable,
	.is_enabled = clk_alpha_pll_is_enabled,
	.recalc_rate = alpha_pll_huayra_recalc_rate,
	.round_rate = alpha_pll_huayra_round_rate,
	.set_rate = alpha_pll_huayra_set_rate,
};
EXPORT_SYMBOL_GPL(clk_alpha_pll_huayra_ops);

const struct clk_ops clk_alpha_pll_hwfsm_ops = {
	.enable = clk_alpha_pll_hwfsm_enable,
	.disable = clk_alpha_pll_hwfsm_disable,
	.is_enabled = clk_alpha_pll_hwfsm_is_enabled,
	.recalc_rate = clk_alpha_pll_recalc_rate,
	.round_rate = clk_alpha_pll_round_rate,
	.set_rate = clk_alpha_pll_hwfsm_set_rate,
};
EXPORT_SYMBOL_GPL(clk_alpha_pll_hwfsm_ops);


static int clk_alpha_pll_slew_update(struct clk_alpha_pll *pll)
{
	int ret = 0;
	u32 val;

	regmap_update_bits(pll->clkr.regmap, PLL_MODE(pll),
					PLL_UPDATE, PLL_UPDATE);
	regmap_read(pll->clkr.regmap, PLL_MODE(pll), &val);

	ret = wait_for_pll_update(pll);
	if (ret)
		return ret;
	/*
	 * HPG mandates a wait of at least 570ns before polling the LOCK
	 * detect bit. Have a delay of 1us just to be safe.
	 */
	mb();
	udelay(1);

	ret = wait_for_pll_enable_lock(pll);

	return ret;
}

/*
 * Slewing plls should be brought up at frequency which is in the middle of the
 * desired VCO range. So after bringing up the pll at calibration freq, set it
 * back to desired frequency(that was set by previous clk_set_rate).
 */
static int clk_alpha_pll_slew_enable(struct clk_hw *hw)
{
	unsigned long calibration_freq, freq_hz;
	struct clk_alpha_pll *pll = to_clk_alpha_pll(hw);
	const struct pll_vco *vco;
	u64 a;
	u32 l, alpha_width = pll_alpha_width(pll);
	int rc;

	vco = alpha_pll_find_vco(pll, clk_hw_get_rate(hw));
	if (!vco) {
		pr_err("alpha pll: not in a valid vco range\n");
		return -EINVAL;
	}

	/*
	 * As during slewing plls vco_sel won't be allowed to change, vco table
	 * should have only one entry table, i.e. index = 0, find the
	 * calibration frequency.
	 */
	calibration_freq = (pll->vco_table[0].min_freq +
					pll->vco_table[0].max_freq)/2;

	freq_hz = alpha_pll_round_rate(calibration_freq,
			clk_hw_get_rate(clk_hw_get_parent(hw)),
			&l, &a, alpha_width);
	if (freq_hz != calibration_freq) {
		pr_err("alpha_pll: call clk_set_rate with rounded rates!\n");
		return -EINVAL;
	}

	/* Setup PLL for calibration frequency */
	a <<= (ALPHA_REG_BITWIDTH - ALPHA_BITWIDTH);

	regmap_write(pll->clkr.regmap, PLL_L_VAL(pll), l);
	regmap_write(pll->clkr.regmap, PLL_ALPHA_VAL(pll), a);
	regmap_write(pll->clkr.regmap, PLL_ALPHA_VAL_U(pll), a >> 32);

	regmap_update_bits(pll->clkr.regmap, PLL_USER_CTL(pll),
				PLL_VCO_MASK << PLL_VCO_SHIFT,
				vco->val << PLL_VCO_SHIFT);

	regmap_update_bits(pll->clkr.regmap, PLL_USER_CTL(pll),
				PLL_ALPHA_EN, PLL_ALPHA_EN);

	/* Bringup the pll at calibration frequency */
	rc = clk_alpha_pll_enable(hw);
	if (rc) {
		pr_err("alpha pll calibration failed\n");
		return rc;
	}

	/*
	 * PLL is already running at calibration frequency.
	 * So slew pll to the previously set frequency.
	 */
	freq_hz = alpha_pll_round_rate(clk_hw_get_rate(hw),
			clk_hw_get_rate(clk_hw_get_parent(hw)),
			&l, &a, alpha_width);

	pr_debug("pll %s: setting back to required rate %lu, freq_hz %ld\n",
				hw->init->name, clk_hw_get_rate(hw), freq_hz);

	/* Setup the PLL for the new frequency */
	a <<= (ALPHA_REG_BITWIDTH - ALPHA_BITWIDTH);

	regmap_write(pll->clkr.regmap, PLL_L_VAL(pll), l);
	regmap_write(pll->clkr.regmap, PLL_ALPHA_VAL(pll), a);
	regmap_write(pll->clkr.regmap, PLL_ALPHA_VAL_U(pll), a >> 32);

	regmap_update_bits(pll->clkr.regmap, PLL_USER_CTL(pll),
				PLL_ALPHA_EN, PLL_ALPHA_EN);

	rc = clk_alpha_pll_slew_update(pll);
	if (rc)
		return rc;

	rc = clk_alpha_pll_enable(hw);

	return rc;
}

static int clk_alpha_pll_slew_set_rate(struct clk_hw *hw, unsigned long rate,
			unsigned long parent_rate)
{
	struct clk_alpha_pll *pll = to_clk_alpha_pll(hw);
	unsigned long freq_hz;
	const struct pll_vco *curr_vco, *vco;
	u32 l;
	u64 a;

	freq_hz = alpha_pll_round_rate(rate, parent_rate,
					&l, &a, pll_alpha_width(pll));
	if (freq_hz != rate) {
		pr_err("alpha_pll: Call clk_set_rate with rounded rates!\n");
		return -EINVAL;
	}

	curr_vco = alpha_pll_find_vco(pll, clk_hw_get_rate(hw));
	if (!curr_vco) {
		pr_err("alpha pll: not in a valid vco range\n");
		return -EINVAL;
	}

	vco = alpha_pll_find_vco(pll, freq_hz);
	if (!vco) {
		pr_err("alpha pll: not in a valid vco range\n");
		return -EINVAL;
	}

	/*
	 * Dynamic pll update will not support switching frequencies across
	 * vco ranges. In those cases fall back to normal alpha set rate.
	 */
	if (curr_vco->val != vco->val)
		return clk_alpha_pll_set_rate(hw, rate, parent_rate);

	a = a << (ALPHA_REG_BITWIDTH - ALPHA_BITWIDTH);

	regmap_write(pll->clkr.regmap, PLL_L_VAL(pll), l);
	regmap_write(pll->clkr.regmap, PLL_ALPHA_VAL(pll), a);
	regmap_write(pll->clkr.regmap, PLL_ALPHA_VAL_U(pll), a >> 32);

	/* Ensure that the write above goes through before proceeding. */
	mb();

	if (clk_hw_is_enabled(hw))
		clk_alpha_pll_slew_update(pll);

	return 0;
}

const struct clk_ops clk_alpha_pll_slew_ops = {
	.enable = clk_alpha_pll_slew_enable,
	.disable = clk_alpha_pll_disable,
	.recalc_rate = clk_alpha_pll_recalc_rate,
	.round_rate = clk_alpha_pll_round_rate,
	.set_rate = clk_alpha_pll_slew_set_rate,
};
EXPORT_SYMBOL_GPL(clk_alpha_pll_slew_ops);

static unsigned long
clk_alpha_pll_postdiv_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
	struct clk_alpha_pll_postdiv *pll = to_clk_alpha_pll_postdiv(hw);
	u32 ctl;

	regmap_read(pll->clkr.regmap, PLL_USER_CTL(pll), &ctl);

	ctl >>= PLL_POST_DIV_SHIFT;
	ctl &= PLL_POST_DIV_MASK(pll);

	return parent_rate >> fls(ctl);
}

static long
clk_alpha_pll_postdiv_round_rate(struct clk_hw *hw, unsigned long rate,
				 unsigned long *prate)
{
	struct clk_alpha_pll_postdiv *pll = to_clk_alpha_pll_postdiv(hw);
	const struct clk_div_table *table;

	if (pll->width == 2)
		table = clk_alpha_2bit_div_table;
	else
		table = clk_alpha_div_table;

	return divider_round_rate(hw, rate, prate, table,
				  pll->width, CLK_DIVIDER_POWER_OF_TWO);
}

static long
clk_alpha_pll_postdiv_round_ro_rate(struct clk_hw *hw, unsigned long rate,
				    unsigned long *prate)
{
	struct clk_alpha_pll_postdiv *pll = to_clk_alpha_pll_postdiv(hw);
	u32 ctl, div;

	regmap_read(pll->clkr.regmap, PLL_USER_CTL(pll), &ctl);

	ctl >>= PLL_POST_DIV_SHIFT;
	ctl &= BIT(pll->width) - 1;
	div = 1 << fls(ctl);

	if (clk_hw_get_flags(hw) & CLK_SET_RATE_PARENT)
		*prate = clk_hw_round_rate(clk_hw_get_parent(hw), div * rate);

	return DIV_ROUND_UP_ULL((u64)*prate, div);
}

static int clk_alpha_pll_postdiv_set_rate(struct clk_hw *hw, unsigned long rate,
					  unsigned long parent_rate)
{
	struct clk_alpha_pll_postdiv *pll = to_clk_alpha_pll_postdiv(hw);
	int div;

	/* 16 -> 0xf, 8 -> 0x7, 4 -> 0x3, 2 -> 0x1, 1 -> 0x0 */
	div = DIV_ROUND_UP_ULL((u64)parent_rate, rate) - 1;

	return regmap_update_bits(pll->clkr.regmap, PLL_USER_CTL(pll),
				  PLL_POST_DIV_MASK(pll) << PLL_POST_DIV_SHIFT,
				  div << PLL_POST_DIV_SHIFT);
}

const struct clk_ops clk_alpha_pll_postdiv_ops = {
	.recalc_rate = clk_alpha_pll_postdiv_recalc_rate,
	.round_rate = clk_alpha_pll_postdiv_round_rate,
	.set_rate = clk_alpha_pll_postdiv_set_rate,
};
EXPORT_SYMBOL_GPL(clk_alpha_pll_postdiv_ops);

const struct clk_ops clk_alpha_pll_postdiv_ro_ops = {
	.round_rate = clk_alpha_pll_postdiv_round_ro_rate,
	.recalc_rate = clk_alpha_pll_postdiv_recalc_rate,
};
EXPORT_SYMBOL_GPL(clk_alpha_pll_postdiv_ro_ops);

void clk_fabia_pll_configure(struct clk_alpha_pll *pll, struct regmap *regmap,
			     const struct alpha_pll_config *config)
{
	u32 val, mask;

	if (config->l)
		regmap_write(regmap, PLL_L_VAL(pll), config->l);

	regmap_write(regmap, PLL_CAL_L_VAL(pll), FABIA_PLL_CAL);

	if (config->alpha)
		regmap_write(regmap, PLL_FRAC(pll), config->alpha);

	if (config->config_ctl_val)
		regmap_write(regmap, PLL_CONFIG_CTL(pll),
						config->config_ctl_val);

	if (config->config_ctl_hi_val)
		regmap_write(regmap, PLL_CONFIG_CTL_U(pll),
						config->config_ctl_hi_val);

	if (config->user_ctl_val)
		regmap_write(regmap, PLL_USER_CTL(pll),
						config->user_ctl_val);

	if (config->user_ctl_hi_val)
		regmap_write(regmap, PLL_USER_CTL_U(pll),
						config->user_ctl_hi_val);

	if (config->post_div_mask) {
		mask = config->post_div_mask;
		val = config->post_div_val;
		regmap_update_bits(regmap, PLL_USER_CTL(pll), mask, val);
	}

	/*
	 * If the PLL has already been initialized, it would now be in a STANDBY
	 * state. Any new updates to the PLL frequency will require setting the
	 * PLL_UPDATE bit.
	 */
	if (pll->inited)
		__clk_alpha_pll_update_latch(pll);

	regmap_update_bits(regmap, PLL_MODE(pll), PLL_UPDATE_BYPASS,
							PLL_UPDATE_BYPASS);

	regmap_update_bits(regmap, PLL_MODE(pll), PLL_RESET_N, PLL_RESET_N);

	pll->inited = true;
}
EXPORT_SYMBOL_GPL(clk_fabia_pll_configure);

static int alpha_pll_fabia_enable(struct clk_hw *hw)
{
	int ret;
	struct clk_alpha_pll *pll = to_clk_alpha_pll(hw);
	u32 val, opmode_val, l_val, cal_val;
	struct regmap *regmap = pll->clkr.regmap;

	ret = regmap_read(regmap, PLL_MODE(pll), &val);
	if (ret)
		return ret;

	/* If in FSM mode, just vote for it */
	if (val & PLL_VOTE_FSM_ENA) {
		ret = clk_enable_regmap(hw);
		if (ret)
			return ret;
		ret = wait_for_pll_enable_active(pll);
		if (ret == 0)
			if (pll->flags & SUPPORTS_FSM_VOTE)
				*pll->soft_vote |= (pll->soft_vote_mask);
		return ret;
	}

	ret = regmap_read(regmap, PLL_OPMODE(pll), &opmode_val);
	if (ret)
		return ret;

	/* Skip If PLL is already running */
	if ((opmode_val & PLL_RUN) && (val & PLL_OUTCTRL))
		return 0;

	ret = regmap_read(regmap, PLL_L_VAL(pll), &l_val);
	if (ret)
		return ret;

	ret = regmap_read(regmap, PLL_CAL_L_VAL(pll), &cal_val);
	if (ret)
		return ret;

	/* PLL has lost its L or CAL value, needs reconfiguration */
	if (!l_val || !cal_val)
		pll->inited = false;

	if (unlikely(!pll->inited)) {
		clk_fabia_pll_configure(pll, pll->clkr.regmap, pll->config);

		pr_warn("%s: PLL configuration lost, reconfiguration of "
			"PLL done.\n", clk_hw_get_name(hw));
	}

	ret = regmap_update_bits(regmap, PLL_MODE(pll), PLL_OUTCTRL, 0);
	if (ret)
		return ret;

	ret = regmap_write(regmap, PLL_OPMODE(pll), PLL_STANDBY);
	if (ret)
		return ret;

	/* PLL should be in STANDBY mode before continuing */
	mb();

	ret = regmap_update_bits(regmap, PLL_MODE(pll), PLL_RESET_N,
				 PLL_RESET_N);
	if (ret)
		return ret;

	ret = regmap_write(regmap, PLL_OPMODE(pll), PLL_RUN);
	if (ret)
		return ret;

	ret = wait_for_pll_enable_lock(pll);
	if (ret)
		return ret;

	ret = regmap_update_bits(regmap, PLL_USER_CTL(pll),
				 PLL_OUT_MASK, PLL_OUT_MASK);
	if (ret)
		return ret;

	ret = regmap_update_bits(regmap, PLL_MODE(pll), PLL_OUTCTRL,
				 PLL_OUTCTRL);
	if (ret)
		return ret;

	/* Ensure that the write above goes through before returning. */
	mb();
	return ret;
}

static void alpha_pll_fabia_disable(struct clk_hw *hw)
{
	int ret;
	struct clk_alpha_pll *pll = to_clk_alpha_pll(hw);
	u32 val;
	struct regmap *regmap = pll->clkr.regmap;

	ret = regmap_read(regmap, PLL_MODE(pll), &val);
	if (ret)
		return;

	/* If in FSM mode, just unvote it */
	if (val & PLL_VOTE_FSM_ENA) {
		if (pll->flags & SUPPORTS_FSM_VOTE) {
			*pll->soft_vote &= ~(pll->soft_vote_mask);
			if (!*pll->soft_vote)
				clk_disable_regmap(hw);
		} else {
			clk_disable_regmap(hw);
		}
		return;
	}

	ret = regmap_update_bits(regmap, PLL_MODE(pll), PLL_OUTCTRL, 0);
	if (ret)
		return;

	/* Disable main outputs */
	ret = regmap_update_bits(regmap, PLL_USER_CTL(pll), PLL_OUT_MASK,
				 0);
	if (ret)
		return;

	/* Place the PLL in STANDBY */
	regmap_write(regmap, PLL_OPMODE(pll), PLL_STANDBY);
}

static unsigned long alpha_pll_fabia_recalc_rate(struct clk_hw *hw,
						unsigned long parent_rate)
{
	struct clk_alpha_pll *pll = to_clk_alpha_pll(hw);
	u32 l, frac, alpha_width = pll_alpha_width(pll);

	regmap_read(pll->clkr.regmap, PLL_L_VAL(pll), &l);
	regmap_read(pll->clkr.regmap, PLL_FRAC(pll), &frac);

	return alpha_pll_calc_rate(parent_rate, l, frac, alpha_width);
}

static int alpha_pll_fabia_set_rate(struct clk_hw *hw, unsigned long rate,
						unsigned long prate)
{
	struct clk_alpha_pll *pll = to_clk_alpha_pll(hw);
	u32 val, l, cal_val, alpha_width = pll_alpha_width(pll);
	u64 a;
	unsigned long rrate;
	int ret = 0;

	ret = regmap_read(pll->clkr.regmap, PLL_MODE(pll), &val);
	if (ret)
		return ret;

	ret = regmap_read(pll->clkr.regmap, PLL_CAL_L_VAL(pll),
			&cal_val);
	if (ret)
		return ret;

	/* PLL has lost it's CAL value, needs reconfiguration */
	if (!cal_val)
		pll->inited = false;

	if (unlikely(!pll->inited)) {
		clk_fabia_pll_configure(pll, pll->clkr.regmap, pll->config);
		pr_warn("%s: PLL configuration lost, reconfiguration of "
			"PLL done.\n", clk_hw_get_name(hw));
	}

	rrate = alpha_pll_round_rate(rate, prate, &l, &a, alpha_width);

	/*
	 * Due to limited number of bits for fractional rate programming, the
	 * rounded up rate could be marginally higher than the requested rate	 */
	if (rrate > (rate + ALPHA_16_BIT_PLL_RATE_MARGIN) || rrate < rate) {
		pr_err("Call set rate on the PLL with rounded rates!\n");
		return -EINVAL;
	}

	regmap_write(pll->clkr.regmap, PLL_L_VAL(pll), l);
	regmap_write(pll->clkr.regmap, PLL_FRAC(pll), a);

	return __clk_alpha_pll_update_latch(pll);
}

/*
 * Fabia PLL requires power-on self calibration which happen when the PLL comes
 * out of reset. Calibration frequency is calculated by below relation:
 *
 * calibration freq = ((pll_l_valmax + pll_l_valmin) * 0.54)
 */
static int alpha_pll_fabia_prepare(struct clk_hw *hw)
{
	unsigned long calibration_freq, freq_hz;
	struct clk_alpha_pll *pll = to_clk_alpha_pll(hw);
	const struct pll_vco *vco;
	struct clk_hw *parent;
	u64 a;
	u32 cal_l, regval;
	int ret;

	/* Check if calibration needs to be done i.e. PLL is in reset */
	ret = regmap_read(pll->clkr.regmap, PLL_MODE(pll), &regval);
	if (ret)
		return ret;

	/* Return early if calibration is not needed. */
	if (regval & PLL_RESET_N)
		return 0;

	vco = alpha_pll_find_vco(pll, clk_hw_get_rate(hw));
	if (!vco) {
		pr_err("alpha pll: not in a valid vco range\n");
		return -EINVAL;
	}

	calibration_freq = ((pll->vco_table[0].min_freq +
				pll->vco_table[0].max_freq) * 54)/100;

	parent = clk_hw_get_parent(hw);
	if (!parent)
		return -EINVAL;

	freq_hz = alpha_pll_round_rate(calibration_freq,
			clk_hw_get_rate(parent), &cal_l, &a,
			pll_alpha_width(pll));
	/*
	 * Due to a limited number of bits for fractional rate programming, the
	 * rounded up rate could be marginally higher than the requested rate.
	 */
	if (freq_hz > (calibration_freq + ALPHA_16_BIT_PLL_RATE_MARGIN) ||
						freq_hz < calibration_freq) {
		pr_err("fabia_pll: Call set rate with rounded rates!\n");
		return -EINVAL;
	}

	/* Setup PLL for calibration frequency */
	regmap_write(pll->clkr.regmap, PLL_CAL_L_VAL(pll), cal_l);

	/* Bringup the pll at calibration frequency */
	ret = alpha_pll_fabia_enable(hw);
	if (ret) {
		pr_err("alpha pll calibration failed\n");
		return ret;
	}

	alpha_pll_fabia_disable(hw);
	return 0;
}

const struct clk_ops clk_alpha_pll_fabia_ops = {
	.prepare = alpha_pll_fabia_prepare,
	.enable = alpha_pll_fabia_enable,
	.disable = alpha_pll_fabia_disable,
	.is_enabled = clk_alpha_pll_is_enabled,
	.set_rate = alpha_pll_fabia_set_rate,
	.recalc_rate = alpha_pll_fabia_recalc_rate,
	.round_rate = clk_alpha_pll_round_rate,
};
EXPORT_SYMBOL_GPL(clk_alpha_pll_fabia_ops);

const struct clk_ops clk_alpha_pll_fixed_fabia_ops = {
	.enable = alpha_pll_fabia_enable,
	.disable = alpha_pll_fabia_disable,
	.is_enabled = clk_alpha_pll_is_enabled,
	.recalc_rate = alpha_pll_fabia_recalc_rate,
	.round_rate = clk_alpha_pll_round_rate,
};
EXPORT_SYMBOL_GPL(clk_alpha_pll_fixed_fabia_ops);

static unsigned long clk_alpha_pll_postdiv_fabia_recalc_rate(struct clk_hw *hw,
					unsigned long parent_rate)
{
	struct clk_alpha_pll_postdiv *pll = to_clk_alpha_pll_postdiv(hw);
	u32 i, div = 1, val;
	int ret;

	if (!pll->post_div_table) {
		pr_err("Missing the post_div_table for the PLL\n");
		return -EINVAL;
	}

	if (pll->num_post_div == 0)
		pr_err("NUM POST DIV IS ZERO!!!!\n");

	ret = regmap_read(pll->clkr.regmap, PLL_USER_CTL(pll), &val);
	if (ret)
		return ret;

	val >>= pll->post_div_shift;
	val &= BIT(pll->width) - 1;

	for (i = 0; i < pll->num_post_div; i++) {
		if (pll->post_div_table[i].val == val) {
			div = pll->post_div_table[i].div;
			break;
		}
	}

	return (parent_rate / div);
}

static long clk_alpha_pll_postdiv_fabia_round_rate(struct clk_hw *hw,
				unsigned long rate, unsigned long *prate)
{
	struct clk_alpha_pll_postdiv *pll = to_clk_alpha_pll_postdiv(hw);

	if (!pll->post_div_table) {
		pr_err("Missing the post_div_table for the PLL\n");
		return -EINVAL;
	}

	return divider_round_rate(hw, rate, prate, pll->post_div_table,
				pll->width, CLK_DIVIDER_ROUND_KHZ);
}

static int clk_alpha_pll_postdiv_fabia_set_rate(struct clk_hw *hw,
				unsigned long rate, unsigned long parent_rate)
{
	struct clk_alpha_pll_postdiv *pll = to_clk_alpha_pll_postdiv(hw);
	int i, val = 0, div, ret;

	/*
	 * If the PLL is in FSM mode, then treat set_rate callback as a
	 * no-operation.
	 */
	ret = regmap_read(pll->clkr.regmap, PLL_MODE(pll), &val);
	if (ret)
		return ret;

	if (val & PLL_VOTE_FSM_ENA)
		return 0;

	if (!pll->post_div_table) {
		pr_err("Missing the post_div_table for the PLL\n");
		return -EINVAL;
	}

	div = DIV_ROUND_UP_ULL((u64)parent_rate, rate);
	for (i = 0; i < pll->num_post_div; i++) {
		if (pll->post_div_table[i].div == div) {
			val = pll->post_div_table[i].val;
			break;
		}
	}

	return regmap_update_bits(pll->clkr.regmap, PLL_USER_CTL(pll),
				(BIT(pll->width) - 1) << pll->post_div_shift,
				val << pll->post_div_shift);
}

const struct clk_ops clk_alpha_pll_postdiv_fabia_ops = {
	.recalc_rate = clk_alpha_pll_postdiv_fabia_recalc_rate,
	.round_rate = clk_alpha_pll_postdiv_fabia_round_rate,
	.set_rate = clk_alpha_pll_postdiv_fabia_set_rate,
};
EXPORT_SYMBOL_GPL(clk_alpha_pll_postdiv_fabia_ops);


static int alpha_trion_pll_is_enabled(struct clk_alpha_pll *pll,
					struct regmap *regmap)
{
	u32 mode_val, opmode_val;
	int ret;

	ret = regmap_read(regmap, PLL_MODE(pll), &mode_val);
	ret |= regmap_read(regmap, PLL_OPMODE(pll), &opmode_val);
	if (ret)
		return 0;

	return ((opmode_val & PLL_RUN) && (mode_val & PLL_OUTCTRL));
}

static int clk_alpha_pll_trion_is_enabled(struct clk_hw *hw)
{
	struct clk_alpha_pll *pll = to_clk_alpha_pll(hw);

	return alpha_trion_pll_is_enabled(pll, pll->clkr.regmap);
}

int clk_alpha_pll_trion_configure(struct clk_alpha_pll *pll, struct regmap *regmap,
				const struct alpha_pll_config *config)
{
	int ret = 0;

	if (pll->inited)
		return ret;

	if (alpha_trion_pll_is_enabled(pll, regmap)) {
		pr_warn("PLL is already enabled. Skipping configuration.\n");
		pll->inited = true;
		return ret;
	}

	if (config->l)
		regmap_write(regmap, PLL_L_VAL(pll), config->l);

	regmap_write(regmap, PLL_CAL_L_VAL(pll), PLL_CAL);

	if (config->frac)
		regmap_write(regmap, PLL_ALPHA_VAL(pll),
						config->frac);

	if (config->config_ctl_val)
		regmap_write(regmap, PLL_CONFIG_CTL(pll),
				config->config_ctl_val);

	if (config->config_ctl_hi_val)
		regmap_write(regmap, PLL_CONFIG_CTL_U(pll),
				config->config_ctl_hi_val);

	if (config->config_ctl_hi1_val)
		regmap_write(regmap, PLL_CONFIG_CTL_U1(pll),
				config->config_ctl_hi1_val);

	if (config->user_ctl_val)
		regmap_write(regmap, PLL_USER_CTL(pll),
				config->user_ctl_val);

	if (config->user_ctl_hi_val)
		regmap_write(regmap, PLL_USER_CTL_U(pll),
				config->user_ctl_hi_val);

	if (config->user_ctl_hi1_val)
		regmap_write(regmap, PLL_USER_CTL_U1(pll),
				config->user_ctl_hi1_val);

	if (config->test_ctl_val)
		regmap_write(regmap, PLL_TEST_CTL(pll),
				config->test_ctl_val);

	if (config->test_ctl_hi_val)
		regmap_write(regmap, PLL_TEST_CTL_U(pll),
				config->test_ctl_hi_val);

	if (config->test_ctl_hi1_val)
		regmap_write(regmap, PLL_TEST_CTL_U1(pll),
				config->test_ctl_hi1_val);

	regmap_update_bits(regmap, PLL_MODE(pll),
				 PLL_UPDATE_BYPASS,
				 PLL_UPDATE_BYPASS);

	/* Disable PLL output */
	ret = regmap_update_bits(regmap, PLL_MODE(pll), PLL_OUTCTRL, 0);
	if (ret)
		return ret;

	/* Set operation mode to OFF */
	regmap_write(regmap, PLL_OPMODE(pll), PLL_STANDBY);

	/* PLL should be in OFF mode before continuing */
	wmb();

	/* Place the PLL in STANDBY mode */
	ret = regmap_update_bits(regmap, PLL_MODE(pll),
						PLL_RESET_N, PLL_RESET_N);
	if (ret)
		return ret;

	pll->inited = true;

	return ret;
}

static int alpha_trion_pll_calibrate(struct clk_hw *hw)
{
	int ret = 0;
	struct clk_alpha_pll *pll = to_clk_alpha_pll(hw);
	u32 l_val, cal_val;

	ret = regmap_read(pll->clkr.regmap, PLL_L_VAL(pll), &l_val);
	if (ret)
		return ret;

	ret = regmap_read(pll->clkr.regmap, PLL_CAL_L_VAL(pll),
				&cal_val);
	if (ret)
		return ret;

	/* PLL has lost it's L or CAL value, needs reconfiguration */
	if (!l_val || !cal_val)
		pll->inited = false;

	if (unlikely(!pll->inited)) {
		ret = clk_alpha_pll_trion_configure(pll, pll->clkr.regmap,
						pll->config);
		if (ret) {
			pr_err("Failed to configure %s\n", clk_hw_get_name(hw));
			return ret;
		}
		pr_warn("PLL configuration lost, reconfiguration of PLL done.\n");
	}

	return ret;
}

static int alpha_trion_pll_enable(struct clk_hw *hw)
{
	int ret = 0;
	struct clk_alpha_pll *pll = to_clk_alpha_pll(hw);
	u32 val;

	ret = regmap_read(pll->clkr.regmap, PLL_MODE(pll), &val);
	if (ret)
		return ret;

	/* If in FSM mode, just vote for it */
	if (val & PLL_VOTE_FSM_ENA) {
		ret = clk_enable_regmap(hw);
		if (ret)
			return ret;
		return wait_for_pll_enable_active(pll);
	}

	/* Check if PLL has lost calibration and eventually reconfigure it */
	ret = alpha_trion_pll_calibrate(hw);
	if (ret)
		return ret;

	/* Set operation mode to RUN */
	regmap_write(pll->clkr.regmap, PLL_OPMODE(pll), PLL_RUN);

	ret = wait_for_pll_enable_lock(pll);
	if (ret)
		return ret;

	/* Enable PLL main output */
	ret = regmap_update_bits(pll->clkr.regmap, PLL_USER_CTL(pll),
				 PLL_OUT_MASK, PLL_OUT_MASK);
	if (ret)
		return ret;

	/* Enable Global PLL outputs */
	ret = regmap_update_bits(pll->clkr.regmap, PLL_MODE(pll),
				 PLL_OUTCTRL, PLL_OUTCTRL);
	if (ret)
		return ret;

	/* Ensure that the write above goes through before returning. */
	mb();
	return ret;
}

static void alpha_trion_pll_disable(struct clk_hw *hw)
{
	int ret;
	struct clk_alpha_pll *pll = to_clk_alpha_pll(hw);
	u32 val;

	ret = regmap_read(pll->clkr.regmap, PLL_MODE(pll), &val);
	if (ret)
		return;

	/* If in FSM mode, just unvote it */
	if (val & PLL_VOTE_FSM_ENA) {
		clk_disable_regmap(hw);
		return;
	}

	/* Disable Global PLL outputs */
	ret = regmap_update_bits(pll->clkr.regmap, PLL_MODE(pll),
							PLL_OUTCTRL, 0);
	if (ret)
		return;

	/* Disable the main PLL output */
	ret = regmap_update_bits(pll->clkr.regmap, PLL_USER_CTL(pll),
			PLL_OUT_MASK, 0);
	if (ret)
		return;

	/* Place the PLL into STANDBY mode */
	regmap_write(pll->clkr.regmap, PLL_OPMODE(pll), PLL_STANDBY);

	regmap_update_bits(pll->clkr.regmap, PLL_MODE(pll),
				 PLL_RESET_N, PLL_RESET_N);
}

/*
 * The Trion PLL requires a power-on self-calibration which happens when the
 * PLL comes out of reset. The calibration is performed at an output frequency
 * of ~1300 MHz which means that SW will have to vote on a voltage that's
 * equal to or greater than SVS_L1 on the corresponding rail. Since this is not
 * feasable to do in the atomic enable path, temporarily bring up the PLL here,
 * let it calibrate, and place it in standby before returning.
 */
#define XO_RATE			19200000
static int clk_trion_pll_prepare(struct clk_hw *hw)
{
	struct clk_alpha_pll *pll = to_clk_alpha_pll(hw);
	u32 regval;
	int ret = 0;

	/* Return early if calibration is not needed. */
	regmap_read(pll->clkr.regmap, PLL_STATUS(pll), &regval);
	if (regval & PLL_PCAL_DONE)
		return ret;

	ret = clk_vote_rate_vdd(hw->core, PLL_CAL * XO_RATE);
	if (ret)
		return ret;

	ret = alpha_trion_pll_enable(hw);
	if (ret)
		goto ret_path;

	alpha_trion_pll_disable(hw);
ret_path:
	clk_unvote_rate_vdd(hw->core, PLL_CAL * XO_RATE);
	return ret;
}

static unsigned long alpha_trion_pll_recalc_rate(struct clk_hw *hw,
						unsigned long parent_rate)
{
	struct clk_alpha_pll *pll = to_clk_alpha_pll(hw);
	u32 l, frac, alpha_width = ALPHA_REG_16BIT_WIDTH;

	regmap_read(pll->clkr.regmap, PLL_L_VAL(pll), &l);
	regmap_read(pll->clkr.regmap, PLL_ALPHA_VAL(pll), &frac);

	return alpha_pll_calc_rate(parent_rate, l, frac, alpha_width);
}

static int alpha_trion_pll_set_rate(struct clk_hw *hw, unsigned long rate,
				  unsigned long prate)
{
	struct clk_alpha_pll *pll = to_clk_alpha_pll(hw);
	unsigned long rrate;
	int ret;
	u32 l = 0;
	u64 a = 0;

	/* Check if PLL has lost calibration and eventually reconfigure it */
	ret = alpha_trion_pll_calibrate(hw);
	if (ret)
		return ret;

	rrate = alpha_pll_round_rate(rate, prate, &l, &a,
						ALPHA_REG_16BIT_WIDTH);
	/*
	 * Due to limited number of bits for fractional rate programming, the
	 * rounded up rate could be marginally higher than the requested rate.
	 */
	if (rrate > (rate + ALPHA_16_BIT_PLL_RATE_MARGIN) || rrate < rate) {
		pr_err("Trion_pll: Call clk_set_rate with rounded rates!\n");
		return -EINVAL;
	}
	regmap_write(pll->clkr.regmap, PLL_L_VAL(pll), l);
	regmap_write(pll->clkr.regmap, PLL_ALPHA_VAL(pll), a);

	/* Latch the input to the PLL */
	ret = regmap_update_bits(pll->clkr.regmap, PLL_MODE(pll),
			PLL_UPDATE, PLL_UPDATE);
	if (ret)
		return ret;

	/* Wait for 2 reference cycle before checking ACK bit */
	udelay(1);

	ret = wait_for_pll_update_ack_set(pll);
	if (ret) {
		WARN(1, "PLL latch failed. Output may be unstable!\n");
		return ret;
	}

	/* Return latch input to 0 */
	ret = regmap_update_bits(pll->clkr.regmap, PLL_MODE(pll),
		PLL_UPDATE, 0);
	if (ret)
		return ret;

	if (clk_hw_is_enabled(hw)) {
		ret = wait_for_pll_enable_lock(pll);
		if (ret)
			return ret;
	}

	/* Wait for PLL output to stabilize */
	udelay(100);

	return ret;
}

const struct clk_ops clk_alpha_pll_trion_ops = {
	.prepare = clk_trion_pll_prepare,
	.enable = alpha_trion_pll_enable,
	.disable = alpha_trion_pll_disable,
	.is_enabled = clk_alpha_pll_trion_is_enabled,
	.recalc_rate = alpha_trion_pll_recalc_rate,
	.round_rate = clk_alpha_pll_round_rate,
	.set_rate = alpha_trion_pll_set_rate,
};
EXPORT_SYMBOL_GPL(clk_alpha_pll_trion_ops);

const struct clk_ops clk_alpha_pll_trion_fixed_ops = {
	.enable = alpha_trion_pll_enable,
	.disable = alpha_trion_pll_disable,
	.is_enabled = clk_alpha_pll_trion_is_enabled,
	.recalc_rate = alpha_trion_pll_recalc_rate,
	.round_rate = clk_alpha_pll_round_rate,
};
EXPORT_SYMBOL_GPL(clk_alpha_pll_trion_fixed_ops);


static long clk_alpha_pll_postdiv_trion_round_rate(struct clk_hw *hw,
				unsigned long rate, unsigned long *prate)
{
	struct clk_alpha_pll_postdiv *pll = to_clk_alpha_pll_postdiv(hw);

	if (!pll->post_div_table) {
		pr_err("Missing the post_div_table for the PLL\n");
		return -EINVAL;
	}

	return divider_round_rate(hw, rate, prate, pll->post_div_table,
				pll->width, CLK_DIVIDER_ROUND_CLOSEST);
}

const struct clk_ops clk_trion_pll_postdiv_ops = {
	.recalc_rate = clk_alpha_pll_postdiv_fabia_recalc_rate,
	.round_rate = clk_alpha_pll_postdiv_trion_round_rate,
	.set_rate = clk_alpha_pll_postdiv_fabia_set_rate,
};
EXPORT_SYMBOL_GPL(clk_trion_pll_postdiv_ops);


int clk_alpha_pll_regera_configure(struct clk_alpha_pll *pll, struct regmap *regmap,
				const struct alpha_pll_config *config)
{
	u32 mode_regval;
	int ret = 0;

	if (!config) {
		pr_err("PLL configuration missing.\n");
		return -EINVAL;
	}

	if (pll->inited)
		return ret;

	ret = regmap_read(regmap, PLL_MODE(pll), &mode_regval);
	if (ret)
		return ret;

	if (mode_regval & PLL_LOCK_DET) {
		pr_warn("PLL is already enabled. Skipping configuration.\n");
		pll->inited = true;
		return 0;
	}

	if (config->alpha)
		regmap_write(regmap, PLL_ALPHA_VAL(pll),
						config->alpha);

	if (config->l)
		regmap_write(regmap, PLL_L_VAL(pll), config->l);

	if (config->config_ctl_val)
		regmap_write(regmap, PLL_CONFIG_CTL(pll),
				config->config_ctl_val);

	if (config->config_ctl_hi_val)
		regmap_write(regmap, PLL_CONFIG_CTL_U(pll),
				config->config_ctl_hi_val);

	if (config->config_ctl_hi1_val)
		regmap_write(regmap, PLL_CONFIG_CTL_U1(pll),
				config->config_ctl_hi1_val);

	if (config->user_ctl_val)
		regmap_write(regmap, PLL_USER_CTL(pll),
				config->user_ctl_val);

	if (config->test_ctl_val)
		regmap_write(regmap, PLL_TEST_CTL(pll),
				config->test_ctl_val);

	if (config->test_ctl_hi_val)
		regmap_write(regmap, PLL_TEST_CTL_U(pll),
				config->test_ctl_hi_val);

	if (config->test_ctl_hi1_val)
		regmap_write(regmap, PLL_TEST_CTL_U1(pll),
				config->test_ctl_hi1_val);

	/* Set operation mode to OFF */
	regmap_write(regmap, PLL_OPMODE(pll), PLL_STANDBY);

	/* PLL should be in OFF mode before continuing */
	wmb();

	pll->inited = true;
	return 0;
}

static int clk_alpha_regera_pll_calibrate(struct clk_hw *hw)
{
	int ret = 0;
	struct clk_alpha_pll *pll = to_clk_alpha_pll(hw);
	u32 l_val;

	ret = regmap_read(pll->clkr.regmap, PLL_L_VAL(pll), &l_val);
	if (ret)
		return ret;

	/* PLL has lost it's L value, needs reconfiguration */
	if (!l_val)
		pll->inited = false;

	if (unlikely(!pll->inited)) {
		ret = clk_alpha_pll_regera_configure(pll, pll->clkr.regmap,
						pll->config);
		if (ret) {
			pr_err("Failed to configure %s\n", clk_hw_get_name(hw));
			return ret;
		}
		pr_warn("PLL configuration lost, reconfiguration of PLL done.\n");
	}

	return ret;
}


static int clk_alpha_pll_regera_enable(struct clk_hw *hw)
{
	int ret;
	struct clk_alpha_pll *pll = to_clk_alpha_pll(hw);
	u32 val;

	ret = regmap_read(pll->clkr.regmap, PLL_MODE(pll), &val);
	if (ret)
		return ret;

	/* If in FSM mode, just vote for it */
	if (val & PLL_VOTE_FSM_ENA) {
		ret = clk_enable_regmap(hw);
		if (ret)
			return ret;
		return wait_for_pll_enable_active(pll);
	}

	/* Check if PLL needs reconfiguration */
	ret = clk_alpha_regera_pll_calibrate(hw);
	if (ret)
		return ret;

	/* Get the PLL out of bypass mode */
	ret = regmap_update_bits(pll->clkr.regmap, PLL_MODE(pll),
						PLL_BYPASSNL, PLL_BYPASSNL);
	if (ret)
		return ret;

	/*
	 * H/W requires a 1us delay between disabling the bypass and
	 * de-asserting the reset.
	 */
	mb();
	udelay(1);

	ret = regmap_update_bits(pll->clkr.regmap, PLL_MODE(pll),
					PLL_RESET_N, PLL_RESET_N);
	if (ret)
		return ret;

	/* Set operation mode to RUN */
	regmap_write(pll->clkr.regmap, PLL_OPMODE(pll), PLL_RUN);

	ret = wait_for_pll_enable_lock(pll);
	if (ret)
		return ret;

	/* Enable the PLL outputs */
	ret = regmap_update_bits(pll->clkr.regmap, PLL_USER_CTL(pll),
				PLL_REGERA_OUT_MASK, PLL_REGERA_OUT_MASK);
	if (ret)
		return ret;

	/* Enable the global PLL outputs */
	ret = regmap_update_bits(pll->clkr.regmap, PLL_MODE(pll),
				 PLL_OUTCTRL, PLL_OUTCTRL);
	if (ret)
		return ret;

	/* Ensure that the write above goes through before returning. */
	mb();
	return ret;
}

static void clk_alpha_pll_regera_disable(struct clk_hw *hw)
{
	int ret;
	struct clk_alpha_pll *pll = to_clk_alpha_pll(hw);
	u32 val, mask;

	ret = regmap_read(pll->clkr.regmap, PLL_MODE(pll), &val);
	if (ret)
		return;

	/* If in FSM mode, just unvote it */
	if (val & PLL_VOTE_FSM_ENA) {
		clk_disable_regmap(hw);
		return;
	}

	/* Disable the global PLL output */
	ret = regmap_update_bits(pll->clkr.regmap, PLL_MODE(pll),
							PLL_OUTCTRL, 0);
	if (ret)
		return;

	/* Disable the PLL outputs */
	ret = regmap_update_bits(pll->clkr.regmap, PLL_USER_CTL(pll),
					PLL_REGERA_OUT_MASK, 0);

	/* Put the PLL in bypass and reset */
	mask = PLL_RESET_N | PLL_BYPASSNL;
	ret = regmap_update_bits(pll->clkr.regmap, PLL_MODE(pll), mask, 0);
	if (ret)
		return;

	/* Place the PLL mode in OFF state */
	regmap_write(pll->clkr.regmap, PLL_OPMODE(pll), PLL_STANDBY);
}

static int clk_alpha_pll_regera_set_rate(struct clk_hw *hw, unsigned long rate,
				  unsigned long prate)
{
	struct clk_alpha_pll *pll = to_clk_alpha_pll(hw);
	unsigned long rrate;
	u32 l, regval;
	u64 a;
	int ret;

	/* Check if PLL needs reconfiguration */
	ret = clk_alpha_regera_pll_calibrate(hw);
	if (ret)
		return ret;

	rrate = alpha_pll_round_rate(rate, prate, &l, &a,
						ALPHA_REG_16BIT_WIDTH);
	/*
	 * Due to a limited number of bits for fractional rate programming, the
	 * rounded up rate could be marginally higher than the requested rate.
	 */
	if (rrate > (rate + ALPHA_16_BIT_PLL_RATE_MARGIN) || rrate < rate) {
		pr_err("Requested rate (%lu) not matching the PLL's "
			"supported frequency (%lu)\n", rate, rrate);
		return -EINVAL;
	}

	regmap_write(pll->clkr.regmap, PLL_ALPHA_VAL(pll), a);
	regmap_write(pll->clkr.regmap, PLL_L_VAL(pll), l);

	/* Return early if the PLL is disabled */
	ret = regmap_read(pll->clkr.regmap, PLL_OPMODE(pll), &regval);
	if (ret)
		return ret;
	else if (regval == PLL_STANDBY)
		return 0;

	/* Wait before polling for the frequency latch */
	udelay(5);

	ret = wait_for_pll_enable_lock(pll);
	if (ret)
		return ret;

	/* Wait for PLL output to stabilize */
	udelay(100);
	return 0;
}

const struct clk_ops clk_alpha_pll_regera_ops = {
	.enable = clk_alpha_pll_regera_enable,
	.disable = clk_alpha_pll_regera_disable,
	.is_enabled = clk_alpha_pll_is_enabled,
	.recalc_rate = alpha_trion_pll_recalc_rate,
	.round_rate = clk_alpha_pll_round_rate,
	.set_rate = clk_alpha_pll_regera_set_rate,
};
EXPORT_SYMBOL_GPL(clk_alpha_pll_regera_ops);


void clk_alpha_pll_agera_configure(struct clk_alpha_pll *pll, struct regmap *regmap,
				const struct alpha_pll_config *config)
{
	u32 val, mask;

	if (config->l)
		regmap_write(regmap, PLL_L_VAL(pll), config->l);

	if (config->alpha)
		regmap_write(regmap, PLL_ALPHA_VAL(pll), config->alpha);

	if (config->post_div_mask) {
		mask = config->post_div_mask;
		val = config->post_div_val;
		regmap_update_bits(regmap, PLL_USER_CTL(pll), mask, val);
	}

	if (config->main_output_mask || config->aux_output_mask ||
		config->aux2_output_mask || config->early_output_mask) {

		val = config->main_output_mask;
		val |= config->aux_output_mask;
		val |= config->aux2_output_mask;
		val |= config->early_output_mask;

		mask = config->main_output_mask;
		mask |= config->aux_output_mask;
		mask |= config->aux2_output_mask;
		mask |= config->early_output_mask;

		regmap_update_bits(regmap, PLL_USER_CTL(pll), mask, val);
	}

	if (config->config_ctl_val)
		regmap_write(regmap, PLL_CONFIG_CTL(pll),
				config->config_ctl_val);

	if (config->config_ctl_hi_val)
		regmap_write(regmap, PLL_CONFIG_CTL_U(pll),
				config->config_ctl_hi_val);

	if (config->test_ctl_val)
		regmap_write(regmap, PLL_TEST_CTL(pll),
					config->test_ctl_val);

	if (config->test_ctl_hi_val)
		regmap_write(regmap, PLL_TEST_CTL_U(pll),
					config->test_ctl_hi_val);
}

static int clk_alpha_pll_agera_set_rate(struct clk_hw *hw, unsigned long rate,
				  unsigned long prate)
{
	struct clk_alpha_pll *pll = to_clk_alpha_pll(hw);
	unsigned long rrate;
	int ret;
	u32 l;
	u64 a;

	rrate = alpha_pll_round_rate(rate, prate, &l, &a,
						ALPHA_REG_16BIT_WIDTH);
	/*
	 * Due to limited number of bits for fractional rate programming, the
	 * rounded up rate could be marginally higher than the requested rate.
	 */
	if (rrate > (rate + ALPHA_16_BIT_PLL_RATE_MARGIN) || rrate < rate) {
		pr_err("Call set rate on the PLL with rounded rates!\n");
		return -EINVAL;
	}

	/* change L_VAL without having to go through the power on sequence */
	regmap_write(pll->clkr.regmap, PLL_L_VAL(pll), l);
	regmap_write(pll->clkr.regmap, PLL_ALPHA_VAL(pll), a);

	/* Ensure that the write above goes through before proceeding. */
	mb();

	if (clk_hw_is_enabled(hw)) {
		ret = wait_for_pll_enable_lock(pll);
		if (ret) {
			pr_err("Agera: Failed to lock after L_VAL update\n");
			return ret;
		}
	}

	return 0;
}

const struct clk_ops clk_alpha_pll_agera_ops = {
	.enable = clk_alpha_pll_enable,
	.disable = clk_alpha_pll_disable,
	.is_enabled = clk_alpha_pll_is_enabled,
	.recalc_rate = alpha_trion_pll_recalc_rate,
	.round_rate = clk_alpha_pll_round_rate,
	.set_rate = clk_alpha_pll_agera_set_rate,
};
EXPORT_SYMBOL(clk_alpha_pll_agera_ops);
