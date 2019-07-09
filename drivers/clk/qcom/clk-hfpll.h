/*
 * Copyright (c) 2013-2014, The Linux Foundation. All rights reserved.
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
#ifndef __QCOM_CLK_HFPLL_H__
#define __QCOM_CLK_HFPLL_H__

#include <linux/clk-provider.h>
#include <linux/spinlock.h>
#include "clk-regmap.h"

struct hfpll_data {
	u32 mode_reg;
	u32 l_reg;
	u32 m_reg;
	u32 n_reg;
	u32 user_reg;
	u32 droop_reg;
	u32 config_reg;
	u32 status_reg;
	u8  lock_bit;

	void __iomem *spm_iobase;
	u8 spm_offset;
	u8 spm_event_bit;

	u32 droop_val;
	u32 config_val;
	u32 user_val;
	u32 user_vco_val;
	u32 user_vco_mask;

	/* masks */
	u32 pre_div_mask;
	u32 post_div_mask;
	u32 early_output_mask;
	u32 main_output_mask;

	/* vals */
	u32 l_val;
	u32 pre_div_masked;
	u32 post_div_masked;
	u32 vco_mode_masked;

	unsigned long low_vco_max_rate;

	unsigned long min_rate;
	unsigned long max_rate;

	u32 l_park_val;
	bool safe_parking_enabled;
};

struct clk_hfpll {
	struct hfpll_data const *d;
	int init_done;

	struct clk_regmap clkr;
	spinlock_t lock;
};

#define to_clk_hfpll(_hw) \
	container_of(to_clk_regmap(_hw), struct clk_hfpll, clkr)

extern const struct clk_ops clk_ops_hfpll;
extern const struct clk_ops clk_ops_hf2_pll;

#endif
