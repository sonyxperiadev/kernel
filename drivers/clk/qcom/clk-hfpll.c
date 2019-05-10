/*
 * clk-hfpll.c
 * Copyright (c) 2013-2014, The Linux Foundation. All rights reserved.
 *
 * HF2 PLL implementation for new-gen HFPLL and SPM adaptation
 * Copyright (C) 2018 AngeloGioacchino Del Regno <kholk11@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include <linux/kernel.h>
#include <linux/export.h>
#include <linux/regmap.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/clk-provider.h>
#include <linux/spinlock.h>

#include "common.h"
#include "clk-regmap.h"
#include "clk-hfpll.h"

#define PLL_OUTCTRL	BIT(0)
#define PLL_BYPASSNL	BIT(1)
#define PLL_RESET_N	BIT(2)

/*
 *  spm_event() -- Set/Clear SPM events
 *  PLL off sequence -- enable (1)
 *    Set L2_SPM_FORCE_EVENT_EN[bit] register to 1
 *    Set L2_SPM_FORCE_EVENT[bit] register to 1
 *  PLL on sequence -- enable (0)
 *   Clear L2_SPM_FORCE_EVENT[bit] register to 0
 *   Clear L2_SPM_FORCE_EVENT_EN[bit] register to 0
 */
static void spm_event(struct clk_hw *hw, bool enable)
{
	struct clk_hfpll *hfpll = to_clk_hfpll(hw);
	struct hfpll_data const *pll = hfpll->d;
	uint32_t val;

	if (!pll->spm_iobase &&
	    (pll->spm_offset == 0) &&
	    (pll->spm_event_bit == 0))
		return;

	if (enable) {
		/* L2_SPM_FORCE_EVENT_EN */
		val = readl_relaxed(pll->spm_iobase + pll->spm_offset);
		val |= BIT(pll->spm_event_bit);
		writel_relaxed(val, (pll->spm_iobase + pll->spm_offset));
		/* Ensure that the write above goes through. */
		mb();

		/* L2_SPM_FORCE_EVENT */
		val = readl_relaxed(pll->spm_iobase + pll->spm_offset + 0x4);
		val |= BIT(pll->spm_event_bit);
		writel_relaxed(val, (pll->spm_iobase + pll->spm_offset + 0x4));
		/* Ensure that the write above goes through. */
		mb();
	} else {
		//pr_err("SPM DISABLE (PLL ON)\n");
		/* L2_SPM_FORCE_EVENT */
		val = readl_relaxed(pll->spm_iobase + pll->spm_offset + 0x4);
		val &= ~BIT(pll->spm_event_bit);
		writel_relaxed(val, (pll->spm_iobase + pll->spm_offset + 0x4));
		/* Ensure that the write above goes through. */
		mb();

		/* L2_SPM_FORCE_EVENT_EN */
		val = readl_relaxed(pll->spm_iobase + pll->spm_offset);
		val &= ~BIT(pll->spm_event_bit);
		writel_relaxed(val, (pll->spm_iobase + pll->spm_offset));
		/* Ensure that the write above goes through. */
		mb();
	}
}

/* Initialize a HFPLL at a given rate and enable it. */
static void __clk_hfpll_init_once(struct clk_hw *hw)
{
	struct clk_hfpll *h = to_clk_hfpll(hw);
	struct hfpll_data const *hd = h->d;
	struct regmap *regmap = h->clkr.regmap;

	if (likely(h->init_done))
		return;

	/* Configure PLL parameters for integer mode. */
	if (hd->config_val)
		regmap_write(regmap, hd->config_reg, hd->config_val);
	
	regmap_write(regmap, hd->m_reg, 0);
	regmap_write(regmap, hd->n_reg, 1);

	if (hd->user_reg) {
		u32 regval = hd->user_val;
		unsigned long rate;

		rate = clk_hw_get_rate(hw);

		/* Pick the right VCO. */
		if (hd->user_vco_mask && rate > hd->low_vco_max_rate)
			regval |= hd->user_vco_mask;
		regmap_write(regmap, hd->user_reg, regval);
	}

	if (hd->droop_reg)
		regmap_write(regmap, hd->droop_reg, hd->droop_val);

	h->init_done = true;
}

static void __clk_hf2_pll_init_once(struct clk_hw *hw)
{
	struct clk_hfpll *h = to_clk_hfpll(hw);
	struct hfpll_data const *hd = h->d;
	struct regmap *regmap = h->clkr.regmap;
	u32 regval = 0;

	if (likely(h->init_done)) {
		return;
	}

	if (hd->user_reg) {
		regmap_read(regmap, hd->user_reg, &regval);

		if (hd->post_div_mask) {
			regval &= ~hd->post_div_mask;
			regval |= hd->post_div_masked;
		}

		if (hd->pre_div_mask) {
			regval &= ~hd->pre_div_mask;
			regval |= hd->pre_div_masked;
		}

		if (hd->user_vco_mask) {
			regval &= ~hd->user_vco_mask;
			regval |= hd->vco_mode_masked;
		}

		if (hd->main_output_mask)
			regval |= hd->main_output_mask;

		if (hd->early_output_mask)
			regval |= hd->early_output_mask;

		regmap_write(regmap, hd->user_reg, regval);
	}

	/* Configure PLL parameters for integer mode. */
	if (hd->config_val)
		regmap_write(regmap, hd->config_reg, hd->config_val);

	regmap_write(regmap, hd->m_reg, 0);
	regmap_write(regmap, hd->n_reg, 0);

	if (hd->l_val)
		regmap_write(regmap, hd->l_reg, hd->l_val);

	h->init_done = true;
}

static void __clk_hfpll_disable(struct clk_hfpll *h)
{
	struct hfpll_data const *hd = h->d;
	struct regmap *regmap = h->clkr.regmap;

	spm_event(&h->clkr.hw, true);

	/*
	 * Disable the PLL output, disable test mode, enable the bypass mode,
	 * and assert the reset.
	 */
	regmap_update_bits(regmap, hd->mode_reg,
			PLL_BYPASSNL | PLL_RESET_N | PLL_OUTCTRL, 0);

	/* Make extra sure that the register got written */
	mb();
}

static void clk_hfpll_disable(struct clk_hw *hw)
{
	struct clk_hfpll *h = to_clk_hfpll(hw);
	unsigned long flags;

	spin_lock_irqsave(&h->lock, flags);
	__clk_hfpll_disable(h);
	spin_unlock_irqrestore(&h->lock, flags);
}

static void __clk_hfpll_enable(struct clk_hw *hw)
{
	struct clk_hfpll *h = to_clk_hfpll(hw);
	struct hfpll_data const *hd = h->d;
	struct regmap *regmap = h->clkr.regmap;
	u32 val;

	__clk_hfpll_init_once(hw);

	/* Disable PLL bypass mode. */
	regmap_update_bits(regmap, hd->mode_reg, PLL_BYPASSNL, PLL_BYPASSNL);

	/*
	 * H/W requires a 5us delay between disabling the bypass and
	 * de-asserting the reset. Delay 10us just to be safe.
	 */
	udelay(10);

	/* De-assert active-low PLL reset. */
	regmap_update_bits(regmap, hd->mode_reg, PLL_RESET_N, PLL_RESET_N);

	/* Wait for PLL to lock. */
	if (hd->status_reg) {
		do {
			regmap_read(regmap, hd->status_reg, &val);
		} while (!(val & BIT(hd->lock_bit)));
	} else {
		udelay(60);
	}

	/* Enable PLL output. */
	regmap_update_bits(regmap, hd->mode_reg, PLL_OUTCTRL, PLL_OUTCTRL);
}

/* Enable an already-configured HFPLL. */
static int clk_hfpll_enable(struct clk_hw *hw)
{
	unsigned long flags;
	struct clk_hfpll *h = to_clk_hfpll(hw);
	struct hfpll_data const *hd = h->d;
	struct regmap *regmap = h->clkr.regmap;
	u32 mode;

	spin_lock_irqsave(&h->lock, flags);
	regmap_read(regmap, hd->mode_reg, &mode);
	if (!(mode & (PLL_BYPASSNL | PLL_RESET_N | PLL_OUTCTRL)))
		__clk_hfpll_enable(hw);
	spin_unlock_irqrestore(&h->lock, flags);

	return 0;
}

static int __clk_hf2_pll_enable(struct clk_hw *hw)
{
	struct clk_hfpll *h = to_clk_hfpll(hw);
	struct hfpll_data const *hd = h->d;
	struct regmap *regmap = h->clkr.regmap;
	u32 val, mode;
	int count, ret;
	bool retried = false, lock = false;

begin:
	__clk_hf2_pll_init_once(hw);

	spm_event(hw, false);

	ret = regmap_read(regmap, hd->mode_reg, &mode);
	if (ret)
		goto out;

	/* Disable PLL bypass mode. */
	ret = regmap_update_bits(regmap, hd->mode_reg, PLL_BYPASSNL,
				 PLL_BYPASSNL);
	if (ret)
		goto out;

	/*
	 * H/W requires a 5us delay between disabling the bypass and
	 * de-asserting the reset. Delay 10us just to be safe.
	 */
	mb();
	udelay(10);

	/* De-assert active-low PLL reset. */
	ret = regmap_update_bits(regmap, hd->mode_reg, PLL_RESET_N,
				 PLL_RESET_N);
	if (ret)
		goto out;

	mb();
	udelay(75);

	/*
	 * The PLL needs at least 50uS to lock, so be extremely cautious
	 * and wait for 75uS, then check and repeat every 1uS for PLL lock.
	 * Note that this PLL can suffer of "phantom" locking, meaning that
	 * it shows the lock is acquired, but then spins again and retries
	 * to get a lock (because either gets lost or bad register read),
	 * so it's mentally sane to recheck the lock status for once after
	 * getting a positive result. One more uS will not hurt anybody.
	 */
	if (hd->status_reg) {	
		for (count = 500; count > 0; count--) {
			ret = regmap_read(regmap, hd->status_reg, &val);
			if (ret)
				return ret;
			if (val & BIT(hd->lock_bit)) {
				if (lock)
					goto out;
				lock = true;
			}
			udelay(1);
		}
	} else {
		udelay(60);
	}
out:
	if (!lock) {
		pr_err("WARNING: HFPLL DID NOT LOCK!!!!\n");
		pr_err("CLK NAME: %s\n", clk_hw_get_name(hw));

		if (!retried) {
			retried = true;

			/* Restore PLL disable state */
			__clk_hfpll_disable(h);
			udelay(75);

			/* Then retry and hope for the best... */
			goto begin;
		}
	}

	/* Enable PLL output. */
	ret = regmap_update_bits(regmap, hd->mode_reg, PLL_OUTCTRL,
		PLL_OUTCTRL);

	mb();
	udelay(2);

	return ret;
}

/* Enable an already-configured HF2-PLL. */
static int clk_hf2_pll_enable(struct clk_hw *hw)
{
	unsigned long flags;
	struct clk_hfpll *h = to_clk_hfpll(hw);

	spin_lock_irqsave(&h->lock, flags);
	__clk_hf2_pll_enable(hw);
	spin_unlock_irqrestore(&h->lock, flags);

	return 0;
}

static long clk_hfpll_round_rate(struct clk_hw *hw, unsigned long rate,
				 unsigned long *parent_rate)
{
	struct clk_hfpll *h = to_clk_hfpll(hw);
	struct hfpll_data const *hd = h->d;
	unsigned long rrate;

	rate = clamp(rate, hd->min_rate, hd->max_rate);

	rrate = DIV_ROUND_UP(rate, *parent_rate) * *parent_rate;
	if (rrate > hd->max_rate)
		rrate -= *parent_rate;

	return rrate;
}

static long clk_hf2_pll_round_rate(struct clk_hw *hw, unsigned long rate,
				 unsigned long *parent_rate)
{
	struct clk_hfpll *h = to_clk_hfpll(hw);
	struct hfpll_data const *hd = h->d;
	unsigned long rrate;

	if (!parent_rate)
		return 0;

	if (rate < hd->min_rate)
		rate = hd->min_rate;
	if (rate > hd->max_rate)
		rate = hd->max_rate;

	rrate = min(hd->max_rate,
		DIV_ROUND_UP(rate, (*parent_rate)) * (*parent_rate));

	return rrate;
}

/*
 * For optimization reasons, assumes no downstream clocks are actively using
 * it.
 */
static int clk_hfpll_set_rate(struct clk_hw *hw, unsigned long rate,
			      unsigned long parent_rate)
{
	struct clk_hfpll *h = to_clk_hfpll(hw);
	struct hfpll_data const *hd = h->d;
	struct regmap *regmap = h->clkr.regmap;
	unsigned long flags;
	u32 l_val, val;
	bool enabled;

	l_val = rate / parent_rate;

	spin_lock_irqsave(&h->lock, flags);

	enabled = __clk_is_enabled(hw->clk);
	if (enabled)
		__clk_hfpll_disable(h);

	/* Pick the right VCO. */
	if (hd->user_reg && hd->user_vco_mask) {
		regmap_read(regmap, hd->user_reg, &val);
		if (rate <= hd->low_vco_max_rate)
			val &= ~hd->user_vco_mask;
		else
			val |= hd->user_vco_mask;
		regmap_write(regmap, hd->user_reg, val);
	}

	regmap_write(regmap, hd->l_reg, l_val);

	if (enabled)
		__clk_hfpll_enable(hw);

	spin_unlock_irqrestore(&h->lock, flags);

	return 0;
}

static int clk_hf2_pll_set_rate(struct clk_hw *hw, unsigned long rate,
			      unsigned long parent_rate)
{
	struct clk_hfpll *h = to_clk_hfpll(hw);
	struct hfpll_data const *hd = h->d;
	struct regmap *regmap = h->clkr.regmap;
	unsigned long flags;
	u32 l_val, val, mode;
	bool enabled;
	u32 enable_mask = PLL_OUTCTRL | PLL_BYPASSNL | PLL_RESET_N;

	if (rate < hd->min_rate)
		rate = hd->min_rate;
	if (rate > hd->max_rate)
		rate = hd->max_rate;

	l_val = rate / parent_rate;

	spin_lock_irqsave(&h->lock, flags);

	regmap_read(regmap, hd->mode_reg, &mode);
	enabled = (mode & enable_mask) == enable_mask;
	if (enabled)
		__clk_hfpll_disable(h);

	/* Pick the right VCO. */
	if (hd->user_reg && hd->user_vco_val) {
		regmap_read(regmap, hd->user_reg, &val);
		if ( (rate <= hd->low_vco_max_rate) &&
		     (rate >= hd->min_rate) ) {
			val |= hd->vco_mode_masked;
			regmap_write(regmap, hd->config_reg,
				hd->user_vco_val);
		} else {
			val &= ~hd->vco_mode_masked;
			regmap_write(regmap, hd->config_reg,
				hd->config_val);
		}
		regmap_write(regmap, hd->user_reg, val);
	}

	regmap_write(regmap, hd->l_reg, l_val);

	if (enabled)
		__clk_hf2_pll_enable(hw);

	spin_unlock_irqrestore(&h->lock, flags);

	return 0;
}

static void clk_hf2_pll_list_registers(struct seq_file *f, struct clk_hw *hw)
{
	struct clk_hfpll *h = to_clk_hfpll(hw);
	struct hfpll_data const *hd = h->d;
	struct regmap *regmap = h->clkr.regmap;
	u32 val;

	regmap_read(regmap, hd->l_reg, &val);
	pr_info("L_REG: 0x%.8x\n", val);

	regmap_read(regmap, hd->m_reg, &val);
	pr_info("M_REG: 0x%.8x\n", val);

	regmap_read(regmap, hd->n_reg, &val);
	pr_info("N_REG: 0x%.8x\n", val);

	regmap_read(regmap, hd->user_reg, &val);
	pr_info("USER_REG: 0x%.8x\n", val);

	regmap_read(regmap, hd->config_reg, &val);
	pr_info("CONFIG_REG: 0x%.8x\n", val);

	regmap_read(regmap, hd->status_reg, &val);
	pr_info("STATUS_REG: 0x%.8x\n", val);
}

static unsigned long clk_hfpll_recalc_rate(struct clk_hw *hw,
					   unsigned long parent_rate)
{
	struct clk_hfpll *h = to_clk_hfpll(hw);
	struct hfpll_data const *hd = h->d;
	struct regmap *regmap = h->clkr.regmap;
	u32 l_val;

	regmap_read(regmap, hd->l_reg, &l_val);

	return l_val * parent_rate;
}

static void clk_hfpll_init(struct clk_hw *hw)
{
	struct clk_hfpll *h = to_clk_hfpll(hw);
	struct hfpll_data const *hd = h->d;
	struct regmap *regmap = h->clkr.regmap;
	u32 mode, status;

	regmap_read(regmap, hd->mode_reg, &mode);
	if (mode != (PLL_BYPASSNL | PLL_RESET_N | PLL_OUTCTRL)) {
		__clk_hfpll_init_once(hw);
		return;
	}

	if (hd->status_reg) {
		regmap_read(regmap, hd->status_reg, &status);
		if (!(status & BIT(hd->lock_bit))) {
			WARN(1, "HFPLL %s is ON, but not locked!\n",
					__clk_get_name(hw->clk));
			clk_hfpll_disable(hw);
			__clk_hfpll_init_once(hw);
		}
	}
}

static void clk_hf2_pll_init(struct clk_hw *hw)
{
	struct clk_hfpll *h = to_clk_hfpll(hw);
	spin_lock_init(&h->lock);
}

static int hfpll_is_enabled(struct clk_hw *hw)
{
	struct clk_hfpll *h = to_clk_hfpll(hw);
	struct hfpll_data const *hd = h->d;
	struct regmap *regmap = h->clkr.regmap;
	u32 mode;

	regmap_read(regmap, hd->mode_reg, &mode);
	mode &= 0x7;
	return mode == (PLL_BYPASSNL | PLL_RESET_N | PLL_OUTCTRL);
}

const struct clk_ops clk_ops_hfpll = {
	.enable = clk_hfpll_enable,
	.disable = clk_hfpll_disable,
	.is_enabled = hfpll_is_enabled,
	.round_rate = clk_hfpll_round_rate,
	.set_rate = clk_hfpll_set_rate,
	.recalc_rate = clk_hfpll_recalc_rate,
	.init = clk_hfpll_init,
};
EXPORT_SYMBOL_GPL(clk_ops_hfpll);

const struct clk_ops clk_ops_hf2_pll = {
	.enable = clk_hf2_pll_enable,
	.disable = clk_hfpll_disable,
	.is_enabled = hfpll_is_enabled,
	.round_rate = clk_hf2_pll_round_rate,
	.set_rate = clk_hf2_pll_set_rate,
	.recalc_rate = clk_hfpll_recalc_rate,
	.list_registers = clk_hf2_pll_list_registers,
	.init = clk_hf2_pll_init,
};
EXPORT_SYMBOL_GPL(clk_ops_hf2_pll);
