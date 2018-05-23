/*
 * Copyright (c) 2018, The Linux Foundation. All rights reserved.
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

#include <linux/clk.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/regmap.h>
#include <linux/clk-provider.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/pm_opp.h>
#include "clk-alpha-pll.h"
#include "vdd-level-8996.h"
#include <soc/qcom/kryo-l2-accessors.h>

#include <dt-bindings/clock/qcom,cpu-8996.h>

#define VCO(a, b, c) { \
	.val = a,\
	.min_freq = b,\
	.max_freq = c,\
}

#define DIV_2_INDEX		0
#define PLL_INDEX		1
#define ACD_INDEX		2
#define ALT_INDEX		3
#define DIV_2_THRESHOLD		600000000
#define PWRCL_REG_OFFSET 0x0
#define PERFCL_REG_OFFSET 0x80000
#define CBF_REG_OFFSET	0x3620000
#define MUX_OFFSET	0x40
#define CBF_MUX_OFFSET	0x18
#define ALT_PLL_OFFSET	0x100
#define SSSCTL_OFFSET 0x160
/*
APCy_QLL_SSSCTL value:
SACDRCLEN=1
SSWEN=1
SSTRTEN=1
SSTPAPMSWEN=1
*/
#define SSSCTL_VAL 0xF

/* VREGS */
static DEFINE_VDD_REGULATORS(vdd_dig, VDD_DIG_NUM, 1,
				vdd_corner);
DEFINE_VDD_REGS_INIT(vdd_pwrcl, 1);
DEFINE_VDD_REGS_INIT(vdd_perfcl, 1);
DEFINE_VDD_REGS_INIT(vdd_cbf, 1);

enum {
	APC_BASE,
	CBF_BASE,
	EFUSE_BASE,
	NUM_BASES
};

enum {
	PWRCL_MUX,
	PERFCL_MUX,
	CBF_MUX,
	NUM_MAX_MUX
};

static void __iomem *vbases[NUM_BASES];

static const u8 prim_pll_regs[PLL_OFF_MAX_REGS] = {
       [PLL_OFF_L_VAL] = 0x04,
       [PLL_OFF_ALPHA_VAL] = 0x08,
       [PLL_OFF_USER_CTL] = 0x10,
       [PLL_OFF_CONFIG_CTL] = 0x18,
       [PLL_OFF_CONFIG_CTL_U] = 0x1C,
       [PLL_OFF_TEST_CTL] = 0x20,
       [PLL_OFF_TEST_CTL_U] = 0x24,
       [PLL_OFF_STATUS] = 0x28,
};

static const u8 alt_pll_regs[PLL_OFF_MAX_REGS] = {
       [PLL_OFF_L_VAL] = 0x04,
       [PLL_OFF_ALPHA_VAL] = 0x08,
       [PLL_OFF_ALPHA_VAL_U] = 0x0c,
       [PLL_OFF_USER_CTL] = 0x10,
       [PLL_OFF_USER_CTL_U] = 0x14,
       [PLL_OFF_CONFIG_CTL] = 0x18,
       [PLL_OFF_TEST_CTL] = 0x20,
       [PLL_OFF_TEST_CTL_U] = 0x24,
       [PLL_OFF_STATUS] = 0x28,
};

static const u8 cbf_pll_regs[PLL_OFF_MAX_REGS] = {
       [PLL_OFF_L_VAL] = 0x08,
       [PLL_OFF_ALPHA_VAL] = 0x10,
       [PLL_OFF_USER_CTL] = 0x18,
       [PLL_OFF_CONFIG_CTL] = 0x20,
       [PLL_OFF_CONFIG_CTL_U] = 0x24,
       [PLL_OFF_TEST_CTL] = 0x30,
       [PLL_OFF_TEST_CTL_U] = 0x34,
       [PLL_OFF_STATUS] = 0x28,
};

/* PLLs */

static const struct alpha_pll_config hfpll_config = {
	.l = 60,
	.config_ctl_val = 0x200D4AA8,
	.config_ctl_hi_val = 0x006,
	.pre_div_mask = BIT(12),
	.post_div_mask = 0x3 << 8,
	.post_div_val = 0x1 << 8,
	.main_output_mask = BIT(0),
	.early_output_mask = BIT(3),
};

static struct clk_alpha_pll perfcl_pll = {
	.offset = PERFCL_REG_OFFSET,
	.regs = prim_pll_regs,
	.flags = SUPPORTS_DYNAMIC_UPDATE | SUPPORTS_FSM_MODE,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "perfcl_pll",
		.parent_names = (const char *[]){ "xo" },
		.num_parents = 1,
		.ops = &clk_alpha_pll_huayra_ops,
		VDD_DIG_FMAX_MAP1(LOW, 3000000000),
	},
};

static struct clk_alpha_pll pwrcl_pll = {
	.offset = PWRCL_REG_OFFSET,
	.regs = prim_pll_regs,
	.flags = SUPPORTS_DYNAMIC_UPDATE | SUPPORTS_FSM_MODE,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "pwrcl_pll",
		.parent_names = (const char *[]){ "xo" },
		.num_parents = 1,
		.ops = &clk_alpha_pll_huayra_ops,
		VDD_DIG_FMAX_MAP1(LOW, 3000000000),
	},
};

static const struct pll_vco alt_pll_vco_modes[] = {
	VCO(3,  250000000,  500000000),
	VCO(2,  500000000,  750000000),
	VCO(1,  750000000, 1000000000),
	VCO(0, 1000000000, 2150400000),
};

static const struct alpha_pll_config altpll_config = {
	.l = 16,
	.vco_val = 0x3 << 20,
	.vco_mask = 0x3 << 20,
	.config_ctl_val = 0x4001051b,
	.post_div_mask = 0x3 << 8,
	.post_div_val = 0x1 << 8,
	.main_output_mask = BIT(0),
	.early_output_mask = BIT(3),
};

static struct clk_alpha_pll perfcl_alt_pll = {
	.offset = PERFCL_REG_OFFSET + ALT_PLL_OFFSET,
	.regs = alt_pll_regs,
	.vco_table = alt_pll_vco_modes,
	.num_vco = ARRAY_SIZE(alt_pll_vco_modes),
	.flags = SUPPORTS_OFFLINE_REQ | SUPPORTS_FSM_MODE,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "perfcl_alt_pll",
		.parent_names = (const char *[]){ "xo" },
		.num_parents = 1,
		.ops = &clk_alpha_pll_hwfsm_ops,
	},
};

static struct clk_alpha_pll pwrcl_alt_pll = {
	.offset = PWRCL_REG_OFFSET + ALT_PLL_OFFSET,
	.regs = alt_pll_regs,
	.vco_table = alt_pll_vco_modes,
	.num_vco = ARRAY_SIZE(alt_pll_vco_modes),
	.flags = SUPPORTS_OFFLINE_REQ | SUPPORTS_FSM_MODE,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "pwrcl_alt_pll",
		.parent_names = (const char *[]){ "xo" },
		.num_parents = 1,
		.ops = &clk_alpha_pll_hwfsm_ops,
	},
};

static void qcom_cpu_clk_msm8996_acd_init(void);

/* Mux'es */

struct clk_cpu_8996_mux {
	u32	reg;
	u32	shift;
	u32	width;
	struct notifier_block nb;
	struct clk_hw	*pll;
	struct clk_hw	*pll_div_2;
	struct clk_regmap clkr;
};

#define to_clk_cpu_8996_mux_nb(_nb) \
	container_of(_nb, struct clk_cpu_8996_mux, nb)

static inline
struct clk_cpu_8996_mux *to_clk_cpu_8996_mux_hw(struct clk_hw *hw)
{
	return container_of(to_clk_regmap(hw), struct clk_cpu_8996_mux, clkr);
}

static u8 clk_cpu_8996_mux_get_parent(struct clk_hw *hw)
{
	unsigned int val;
	struct clk_regmap *clkr = to_clk_regmap(hw);
	struct clk_cpu_8996_mux *cpuclk = to_clk_cpu_8996_mux_hw(hw);
	unsigned int mask = GENMASK(cpuclk->width - 1, 0);

	regmap_read(clkr->regmap, cpuclk->reg, &val);

	val >>= cpuclk->shift;
	val &= mask;

	return val;
}

static int clk_cpu_8996_mux_set_parent(struct clk_hw *hw, u8 index)
{
	unsigned int val;
	struct clk_regmap *clkr = to_clk_regmap(hw);
	struct clk_cpu_8996_mux *cpuclk = to_clk_cpu_8996_mux_hw(hw);
	unsigned int mask = GENMASK(cpuclk->width + cpuclk->shift - 1,
				    cpuclk->shift);

	val = index;
	val <<= cpuclk->shift;

	return regmap_update_bits(clkr->regmap, cpuclk->reg, mask, val);
}

static int
clk_cpu_8996_mux_determine_rate(struct clk_hw *hw, struct clk_rate_request *req)
{
	struct clk_cpu_8996_mux *cpuclk = to_clk_cpu_8996_mux_hw(hw);
	struct clk_hw *parent = cpuclk->pll;

	if (!cpuclk->pll)
		return -EINVAL;

	if (cpuclk->pll_div_2 && req->rate < DIV_2_THRESHOLD) {
		if (req->rate < (DIV_2_THRESHOLD / 2))
			return -EINVAL;

		parent = cpuclk->pll_div_2;
	}

	req->best_parent_rate = clk_hw_round_rate(parent, req->rate);
	req->best_parent_hw = parent;

	return 0;
}

int cpu_clk_notifier_cb(struct notifier_block *nb, unsigned long event,
			void *data)
{
	int ret;
	struct clk_cpu_8996_mux *cpuclk = to_clk_cpu_8996_mux_nb(nb);
	struct clk_notifier_data *cnd = data;

	switch (event) {
	case PRE_RATE_CHANGE:
		ret = clk_cpu_8996_mux_set_parent(&cpuclk->clkr.hw, ALT_INDEX);
		qcom_cpu_clk_msm8996_acd_init();
		break;
	case POST_RATE_CHANGE:
		if (cnd->new_rate < DIV_2_THRESHOLD)
			ret = clk_cpu_8996_mux_set_parent(&cpuclk->clkr.hw,
							  DIV_2_INDEX);
		else
			ret = clk_cpu_8996_mux_set_parent(&cpuclk->clkr.hw,
							  ACD_INDEX);
		break;
	default:
		ret = 0;
		break;
	}

	return notifier_from_errno(ret);
};

const struct clk_ops clk_cpu_8996_mux_ops = {
	.set_parent = clk_cpu_8996_mux_set_parent,
	.get_parent = clk_cpu_8996_mux_get_parent,
	.determine_rate = clk_cpu_8996_mux_determine_rate,
};

static struct clk_cpu_8996_mux pwrcl_smux = {
	.reg = PWRCL_REG_OFFSET + MUX_OFFSET,
	.shift = 2,
	.width = 2,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "pwrcl_smux",
		.parent_names = (const char *[]){
			"xo",
			"pwrcl_pll_main",
		},
		.num_parents = 2,
		.ops = &clk_cpu_8996_mux_ops,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_cpu_8996_mux perfcl_smux = {
	.reg = PERFCL_REG_OFFSET + MUX_OFFSET,
	.shift = 2,
	.width = 2,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "perfcl_smux",
		.parent_names = (const char *[]){
			"xo",
			"perfcl_pll_main",
		},
		.num_parents = 2,
		.ops = &clk_cpu_8996_mux_ops,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_init_data cpu_8996_clk_init_data[NUM_MAX_MUX] = {
	[PWRCL_MUX] = {
		.name = "pwrcl_pmux",
		.parent_names = (const char *[]){
			"pwrcl_smux",
			"pwrcl_pll",
			"pwrcl_pll_acd",
			"pwrcl_alt_pll",
		},
		.num_parents = 4,
		.ops = &clk_cpu_8996_mux_ops,
		.flags = CLK_SET_RATE_PARENT | CLK_IS_CRITICAL,
	},
	[PERFCL_MUX] = {
		.name = "perfcl_pmux",
		.parent_names = (const char *[]){
			"perfcl_smux",
			"perfcl_pll",
			"perfcl_pll_acd",
			"perfcl_alt_pll",
		},
		.num_parents = 4,
		.ops = &clk_cpu_8996_mux_ops,
		.flags = CLK_SET_RATE_PARENT | CLK_IS_CRITICAL,
	},
	[CBF_MUX] = {
		.name = "cbf_mux",
		.parent_names = (const char *[]){
			"cbf_div_mux",
			"cbf_pll",
			"cbf_pll_acd",
		},
		.num_parents = 3,
		.ops = &clk_cpu_8996_mux_ops,
		.flags = CLK_SET_RATE_PARENT | CLK_IS_CRITICAL,
	},
};

static struct clk_cpu_8996_mux pwrcl_pmux = {
	.reg = PWRCL_REG_OFFSET + MUX_OFFSET,
	.shift = 0,
	.width = 2,
	.pll = &pwrcl_pll.clkr.hw,
	.pll_div_2 = &pwrcl_smux.clkr.hw,
	.nb.notifier_call = cpu_clk_notifier_cb,
	.clkr.hw.init = &cpu_8996_clk_init_data[PWRCL_MUX],
};

static struct clk_cpu_8996_mux perfcl_pmux = {
	.reg = PERFCL_REG_OFFSET + MUX_OFFSET,
	.shift = 0,
	.width = 2,
	.pll = &perfcl_pll.clkr.hw,
	.pll_div_2 = &perfcl_smux.clkr.hw,
	.nb.notifier_call = cpu_clk_notifier_cb,
	.clkr.hw.init = &cpu_8996_clk_init_data[PERFCL_MUX],
};


static const struct alpha_pll_config cbfpll_config = {
	.l = 72, /* 1.38GHz output */
	.config_ctl_val = 0x200D4AA8,
	.config_ctl_hi_val = 0x006,
	.pre_div_mask = BIT(12),
	.post_div_mask = 0x3 << 8,
	.post_div_val = 0x1 << 8,
	.main_output_mask = BIT(0),
	.early_output_mask = BIT(3),
};

static struct clk_alpha_pll cbf_pll = {
	.offset = CBF_REG_OFFSET,
	.regs = cbf_pll_regs,
	.flags = SUPPORTS_DYNAMIC_UPDATE | SUPPORTS_FSM_MODE,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "cbf_pll",
		.parent_names = (const char *[]){ "xo" },
		.num_parents = 1,
		.ops = &clk_alpha_pll_huayra_ops,
	},
};

static struct clk_cpu_8996_mux cbf_div_mux = {
	.reg = CBF_REG_OFFSET + CBF_MUX_OFFSET,
	.shift = 2,
	.width = 2,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "cbf_div_mux",
		.parent_names = (const char *[]){
			"xo",
			"cbf_pll_main",
		},
		.num_parents = 2,
		.ops = &clk_cpu_8996_mux_ops,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_cpu_8996_mux cbf_mux = {
	.reg = CBF_REG_OFFSET + CBF_MUX_OFFSET,
	.shift = 0,
	.width = 2,
	.pll = &cbf_pll.clkr.hw,
	.pll_div_2 = &cbf_div_mux.clkr.hw,
	.nb.notifier_call = cpu_clk_notifier_cb,
	.clkr.hw.init = &cpu_8996_clk_init_data[CBF_MUX],
};

static const struct regmap_config cpu_msm8996_regmap_config = {
	.reg_bits		= 32,
	.reg_stride		= 4,
	.val_bits		= 32,
	.max_register		= 0x80210,
	.fast_io		= true,
	.val_format_endian	= REGMAP_ENDIAN_LITTLE,
};

static const struct regmap_config cbf_msm8996_regmap_config = {
	.reg_bits		= 32,
	.reg_stride		= 4,
	.val_bits		= 32,
	.max_register		= 0x10000,
	.fast_io		= true,
	.val_format_endian	= REGMAP_ENDIAN_LITTLE,
};

static const struct of_device_id match_table[] = {
	{ .compatible = "qcom,msm8996-apcc" },
	{ .compatible = "qcom,cpu-clock-8996" },
	{}
};

struct clk_regmap *clks[] = {
	/* PLLs */
	&perfcl_pll.clkr,
	&pwrcl_pll.clkr,
	&perfcl_alt_pll.clkr,
	&pwrcl_alt_pll.clkr,
	/* MUXs */
	&perfcl_smux.clkr,
	&pwrcl_smux.clkr,
	&perfcl_pmux.clkr,
	&pwrcl_pmux.clkr,
};

struct clk_hw *cbf_clks[] = {
	/* PLLs */
	&cbf_pll.clkr.hw,
	/* MUXs */
	&cbf_div_mux.clkr.hw,
	&cbf_mux.clkr.hw,
};

struct clk_hw_clks {
	unsigned int num;
	struct clk_hw *hws[];
};

static int
qcom_cbf_clk_msm8996_register_clks(struct device *dev, struct clk_hw_clks *hws,
				   struct regmap *cbf_regmap)
{
	int i, ret;
	struct clk *clk;

	for (i = 0; i < ARRAY_SIZE(cbf_clks); i++) {
		clk = devm_clk_register(dev, cbf_clks[i]);
		if (IS_ERR(clk))
			return PTR_ERR(clk);
	}

	clk_alpha_pll_configure(&cbf_pll, cbf_regmap, &cbfpll_config);
	clk_prepare_enable(cbf_pll.clkr.hw.clk);
	clk_set_rate(cbf_pll.clkr.hw.clk, 614400000);

	ret = clk_notifier_register(cbf_mux.clkr.hw.clk, &cbf_mux.nb);
	if (ret)
		return ret;

	return ret;
}

static int
qcom_cpu_clk_msm8996_register_clks(struct device *dev, struct clk_hw_clks *hws,
				   struct regmap *regmap)
{
	int i, ret;

	hws->hws[0] = clk_hw_register_fixed_factor(dev, "perfcl_pll_main",
						   "perfcl_pll",
						   CLK_SET_RATE_PARENT, 1, 2);
	perfcl_smux.pll = hws->hws[0];

	hws->hws[1] = clk_hw_register_fixed_factor(dev, "pwrcl_pll_main",
						   "pwrcl_pll",
						   CLK_SET_RATE_PARENT, 1, 2);
	pwrcl_smux.pll = hws->hws[1];

	hws->hws[2] = clk_hw_register_fixed_factor(dev, "cbf_pll_main",
						   "cbf_pll",
						   CLK_SET_RATE_PARENT, 1, 2);
	cbf_mux.pll = hws->hws[2];

	hws->num = 3;

	for (i = 0; i < ARRAY_SIZE(clks); i++) {
		ret = devm_clk_register_regmap(dev, clks[i]);
		if (ret)
			return ret;
	}

	clk_alpha_pll_configure(&perfcl_pll, regmap, &hfpll_config);
	clk_alpha_pll_configure(&pwrcl_pll, regmap, &hfpll_config);
	clk_alpha_pll_configure(&perfcl_alt_pll, regmap, &altpll_config);
	clk_alpha_pll_configure(&pwrcl_alt_pll, regmap, &altpll_config);

	/* Enable all PLLs and alt PLLs */
	clk_prepare_enable(pwrcl_alt_pll.clkr.hw.clk);
	clk_prepare_enable(perfcl_alt_pll.clkr.hw.clk);
	clk_prepare_enable(pwrcl_pll.clkr.hw.clk);
	clk_prepare_enable(perfcl_pll.clkr.hw.clk);

	/* Set initial boot frequencies for power/perf PLLs */
	clk_set_rate(pwrcl_alt_pll.clkr.hw.clk, 652800000);
	clk_set_rate(perfcl_alt_pll.clkr.hw.clk, 652800000);
	clk_set_rate(pwrcl_pll.clkr.hw.clk, 652800000);
	clk_set_rate(perfcl_pll.clkr.hw.clk, 652800000);

	ret = clk_notifier_register(pwrcl_pmux.clkr.hw.clk, &pwrcl_pmux.nb);
	if (ret)
		return ret;

	ret = clk_notifier_register(perfcl_pmux.clkr.hw.clk, &perfcl_pmux.nb);
	if (ret)
		return ret;

	return ret;
}

#define CPU_AFINITY_MASK 0xFFF
#define PWRCL_CPU_REG_MASK 0x3
#define PERFCL_CPU_REG_MASK 0x103

/* ACD static settings (HMSS HPG 7.2.2) */
#define L2ACDCR_REG 0x580ULL
#define L2ACDTD_REG 0x581ULL
#define L2ACDDVMRC_REG 0x584ULL
#define L2ACDSSCR_REG 0x589ULL
#define ACDTD_VAL 0x00006A11
#define ACDCR_VAL 0x002C5FFD
#define ACDSSCR_VAL 0x00000601
#define ACDDVMRC_VAL 0x000E0F0F

static DEFINE_SPINLOCK(acd_lock);

static void qcom_cpu_clk_msm8996_acd_init(void)
{
	u64 hwid;
	unsigned long flags;

	spin_lock_irqsave(&acd_lock, flags);

	hwid = read_cpuid_mpidr() & CPU_AFINITY_MASK;

	/* Program ACD Tunable-Length Delay (TLD) */
	set_l2_indirect_reg(L2ACDTD_REG, ACDTD_VAL);
	/* Initial ACD for *this* cluster */
	set_l2_indirect_reg(L2ACDDVMRC_REG, ACDDVMRC_VAL);
	/* Program ACD soft start control bits. */
	set_l2_indirect_reg(L2ACDSSCR_REG, ACDSSCR_VAL);

	if (PWRCL_CPU_REG_MASK == (hwid | PWRCL_CPU_REG_MASK)) {
		/* Enable Soft Stop/Start */
		if (vbases[APC_BASE])
			writel_relaxed(SSSCTL_VAL, vbases[APC_BASE] +
					PWRCL_REG_OFFSET + SSSCTL_OFFSET);
		/* Ensure SSSCTL config goes through before enabling ACD. */
		mb();
		/* Program ACD control bits */
		set_l2_indirect_reg(L2ACDCR_REG, ACDCR_VAL);
	}
	if (PERFCL_CPU_REG_MASK == (hwid | PERFCL_CPU_REG_MASK)) { //else {
		/* Program ACD control bits */
		set_l2_indirect_reg(L2ACDCR_REG, ACDCR_VAL);
		/* Enable Soft Stop/Start */
		if (vbases[APC_BASE])
			writel_relaxed(SSSCTL_VAL, vbases[APC_BASE] +
					PERFCL_REG_OFFSET + SSSCTL_OFFSET);
		/* Ensure SSSCTL config goes through before enabling ACD. */
		mb();
	}

	spin_unlock_irqrestore(&acd_lock, flags);
}

static int of_get_fmax_vdd_class(struct platform_device *pdev,
				 int mux_id, char *prop_name)
{
	struct device_node *of = pdev->dev.of_node;
	int prop_len, i, j;
	struct clk_vdd_class *vdd = NULL;
	int nreg = 2;
	u32 *array;

	if (!of_find_property(of, prop_name, &prop_len)) {
		dev_err(&pdev->dev, "missing %s\n", prop_name);
		return -EINVAL;
	}

	prop_len /= sizeof(u32);
	if (prop_len % nreg) {
		dev_err(&pdev->dev, "bad length %d\n", prop_len);
		return -EINVAL;
	}

	prop_len /= nreg;

	vdd = cpu_8996_clk_init_data[mux_id].vdd_class;

	vdd->level_votes = devm_kzalloc(&pdev->dev,
				prop_len * sizeof(*vdd->level_votes),
					GFP_KERNEL);
	if (!vdd->level_votes) {
		pr_err("Cannot allocate memory for level_votes\n");
		return -ENOMEM;
	}

	vdd->vdd_uv = devm_kzalloc(&pdev->dev,
		prop_len * sizeof(int) * (nreg - 1), GFP_KERNEL);
	if (!vdd->vdd_uv) {
		pr_err("Cannot allocate memory for vdd_uv\n");
		return -ENOMEM;
	}

	cpu_8996_clk_init_data[mux_id].rate_max = devm_kzalloc(&pdev->dev,
					prop_len * sizeof(unsigned long),
					GFP_KERNEL);
	if (!cpu_8996_clk_init_data[mux_id].rate_max) {
		pr_err("Cannot allocate memory for rate_max\n");
		return -ENOMEM;
	}

	array = devm_kzalloc(&pdev->dev,
			prop_len * sizeof(u32) * nreg, GFP_KERNEL);
	if (!array)
		return -ENOMEM;

	of_property_read_u32_array(of, prop_name, array, prop_len * nreg);
	for (i = 0; i < prop_len; i++) {
		cpu_8996_clk_init_data[mux_id].rate_max[i] = array[nreg * i];
		for (j = 1; j < nreg; j++)
			vdd->vdd_uv[(nreg - 1) * i + (j - 1)] =
					array[nreg * i + j];
	}

	devm_kfree(&pdev->dev, array);
	vdd->num_levels = prop_len;
	vdd->cur_level = prop_len;
	vdd->use_max_uV = true;
	cpu_8996_clk_init_data[mux_id].num_rate_max = prop_len;

	return 0;
}

static int find_vdd_level(struct clk_init_data *clk_data, unsigned long rate)
{
	int level;

	for (level = 0; level < clk_data->num_rate_max; level++)
		if (rate <= clk_data->rate_max[level])
			break;

	if (level == clk_data->num_rate_max) {
		pr_err("Rate %lu for %s is greater than highest Fmax\n", rate,
				clk_data->name);
		return -EINVAL;
	}

	return level;
}

static int add_opp(struct clk_hw *hw, struct device *cpudev,
			struct device *vregdev, unsigned long max_rate)
{
	struct clk_init_data *clk_data = NULL;
	struct clk_vdd_class *voltspec = NULL;
	unsigned long rate = 0;
	long ret, uv;
	int level, j = 1;

	if (IS_ERR_OR_NULL(cpudev)) {
		pr_err("%s: Invalid parameters\n", __func__);
		return -EINVAL;
	}

	clk_data = (struct clk_init_data *)hw->init;
	voltspec = clk_data->vdd_class;

	while (1) {
		rate = clk_data->rate_max[j++];
		level = find_vdd_level(clk_data, rate);
		if (level <= 0) {
			pr_warn("clock-cpu: no corner for %lu.\n", rate);
			return -EINVAL;
		}

		uv = voltspec->vdd_uv[level];
		if (uv < 0) {
			pr_warn("clock-cpu: no uv for %lu.\n", rate);
			return -EINVAL;
		}

		ret = dev_pm_opp_add(cpudev, rate, uv);
		if (ret) {
			pr_warn("clock-cpu: failed to add OPP for %lu\n", rate);
			return rate;
		}

		if (rate >= max_rate)
			break;
	}

	return 0;
}

static void populate_opp_table(struct platform_device *pdev)
{
	struct platform_device *apc0_dev, *apc1_dev;
	struct platform_device *cbf_dev;
	struct device_node *apc0_node = NULL, *apc1_node = NULL;
	struct device_node *cbf_node = NULL;
	unsigned long apc0_fmax, apc1_fmax;
	unsigned long cbf_fmax;
	unsigned int apc0_rate_max = 0, apc1_rate_max = 0;
	unsigned int cbf_rate_max = 0;
	int cpu, pwr_cpu = 0, perf_cpu = 0;

	apc0_node = of_parse_phandle(pdev->dev.of_node,
					"vdd-pwrcl-supply", 0);
	if (!apc0_node) {
		pr_err("can't find the apc0 vreg dt node.\n");
		return;
	}

	apc1_node = of_parse_phandle(pdev->dev.of_node,
					"vdd-perfcl-supply", 0);
	if (!apc1_node) {
		pr_err("can't find the apc1 vreg dt node.\n");
		return;
	}

	cbf_node = of_parse_phandle(pdev->dev.of_node,
					"vdd-cbf-supply", 0);
	if (!cbf_node) {
		pr_err("can't find the cbf vreg dt node.\n");
		return;
	}

	apc0_dev = of_find_device_by_node(apc0_node);
	if (!apc0_dev) {
		pr_err("can't find the apc0 vreg device node.\n");
		return;
	}

	apc1_dev = of_find_device_by_node(apc1_node);
	if (!apc1_dev) {
		pr_err("can't find the apc1 vreg device node.\n");
		return;
	}

	cbf_dev = of_find_device_by_node(apc1_node);
	if (!cbf_dev) {
		pr_err("can't find the cbf vreg device node.\n");
		return;
	}

	apc0_rate_max = pwrcl_pmux.clkr.hw.init->num_rate_max - 1;
	apc1_rate_max = perfcl_pmux.clkr.hw.init->num_rate_max - 1;
	cbf_rate_max = cbf_mux.clkr.hw.init->num_rate_max - 1;
	apc0_fmax = pwrcl_pmux.clkr.hw.init->rate_max[apc0_rate_max];
	apc1_fmax = perfcl_pmux.clkr.hw.init->rate_max[apc1_rate_max];
	cbf_fmax = cbf_mux.clkr.hw.init->rate_max[cbf_rate_max];

	for_each_possible_cpu(cpu) {
		if (cpu <= 1) {
			pwr_cpu = cpu;
			WARN(add_opp(&pwrcl_pmux.clkr.hw, get_cpu_device(cpu),
				&apc0_dev->dev, apc0_fmax),
				"Failed to add OPP levels for pwr cluster\n");
		} else {
			perf_cpu = cpu;
			WARN(add_opp(&perfcl_pmux.clkr.hw, get_cpu_device(cpu),
				&apc1_dev->dev, apc1_fmax),
				"Failed to add OPP levels for perf cluster\n");
		}
	}

	WARN(add_opp(&perfcl_pmux.clkr.hw, get_cpu_device(cpu),
		&apc1_dev->dev, apc1_fmax),
		"Failed to add OPP levels for CBF\n");

	/* One time print during bootup */
	pr_info("clock-cpu-8996: OPP tables populated (cpu 0-%d and %d-%d)\n",
						pwr_cpu, pwr_cpu+1, perf_cpu);

	//print_opp_table(pwr_cpu, perf_cpu);
}

static void get_efuse_speed_bin(struct platform_device *pdev, int *bin,
								int *version)
{
	struct resource *res;
	void __iomem *base;
	u32 pte_efuse;

	*bin = 0;
	*version = 0;

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "efuse");
	if (!res) {
		dev_info(&pdev->dev,
			 "No speed/PVS binning available. Defaulting to 0!\n");
		return;
	}

	base = devm_ioremap(&pdev->dev, res->start, resource_size(res));
	if (!base) {
		dev_warn(&pdev->dev,
			 "Unable to read efuse data. Defaulting to 0!\n");
		return;
	}

	pte_efuse = readl_relaxed(base);
	devm_iounmap(&pdev->dev, base);

	*bin = (pte_efuse >> 29) & 0x7;

	dev_info(&pdev->dev, "Speed bin: %d PVS Version: %d\n", *bin,
								*version);
}

static int qcom_cpu_parse_speedbin(struct platform_device *pdev)
{
	char perfclspeedbinstr[] = "qcom,perfcl-speedbinXX-vXX";
	char pwrclspeedbinstr[] = "qcom,pwrcl-speedbinXX-vXX";
	char cbfspeedbinstr[] = "qcom,cbf-speedbinXX-vXX";
	int ret, speed_bin, pvs_version;

	get_efuse_speed_bin(pdev, &speed_bin, &pvs_version);
	pvs_version = 0; // MSM8996 has only PVS 0

	snprintf(perfclspeedbinstr, ARRAY_SIZE(perfclspeedbinstr),
			"qcom,perfcl-speedbin%d-v%d", speed_bin, pvs_version);
	ret = of_get_fmax_vdd_class(pdev, PERFCL_MUX, perfclspeedbinstr);
	if (ret) {
		dev_err(&pdev->dev, "Unable to load voltage plan %s!\n", 
			perfclspeedbinstr);

		ret = of_get_fmax_vdd_class(pdev, PERFCL_MUX,
					    "qcom,perfcl-speedbin0-v0");
		if (ret) {
			dev_err(&pdev->dev,
				"Unable to load safe voltage plan\n");
			return ret;
		}
		dev_info(&pdev->dev, "Safe voltage plan loaded for perfcl.\n");
	}

	snprintf(pwrclspeedbinstr, ARRAY_SIZE(pwrclspeedbinstr),
			"qcom,pwrcl-speedbin%d-v%d", speed_bin, pvs_version);
	ret = of_get_fmax_vdd_class(pdev, PWRCL_MUX, pwrclspeedbinstr);
	if (ret) {
		dev_err(&pdev->dev, "Unable to load voltage plan %s!\n", 
			pwrclspeedbinstr);

		ret = of_get_fmax_vdd_class(pdev, PWRCL_MUX,
					    "qcom,pwrcl-speedbin0-v0");
		if (ret) {
			dev_err(&pdev->dev,
				"Unable to load safe voltage plan\n");
			return ret;
		}
		dev_info(&pdev->dev, "Safe voltage plan loaded for pwrcl.\n");
	}

	snprintf(cbfspeedbinstr, ARRAY_SIZE(cbfspeedbinstr),
			"qcom,cbf-speedbin%d-v%d", speed_bin, pvs_version);
	ret = of_get_fmax_vdd_class(pdev, CBF_MUX, cbfspeedbinstr);
	if (ret) {
		dev_err(&pdev->dev, "Unable to load voltage plan %s!\n", 
			cbfspeedbinstr);

		ret = of_get_fmax_vdd_class(pdev, CBF_MUX,
					    "qcom,cbf-speedbin0-v0");
		if (ret) {
			dev_err(&pdev->dev,
				"Unable to load safe voltage plan\n");
			return ret;
		}
		dev_info(&pdev->dev, "Safe voltage plan loaded for cbf.\n");
	}

	dev_info(&pdev->dev, "Voltage plans loaded.\n");

	return ret;
}

static int cpu_parse_devicetree(struct platform_device *pdev)
{
	int rc = 0;

	vdd_dig.regulator[0] = devm_regulator_get(&pdev->dev, "vdd-dig");
	if (IS_ERR(vdd_dig.regulator[0])) {
		if (PTR_ERR(vdd_dig.regulator[0]) != -EPROBE_DEFER)
			dev_err(&pdev->dev, "Unable to get the CX regulator");
		return PTR_ERR(vdd_dig.regulator[0]);
	}

	vdd_pwrcl.regulator[0] = devm_regulator_get(&pdev->dev, "vdd-pwrcl");
	if (IS_ERR(vdd_pwrcl.regulator[0])) {
		if (PTR_ERR(vdd_pwrcl.regulator[0]) != -EPROBE_DEFER)
			dev_err(&pdev->dev, "Unable to get the pwrcl vreg\n");
		return PTR_ERR(vdd_pwrcl.regulator[0]);
	}
	vdd_pwrcl.use_max_uV = true;
	vdd_perfcl.regulator[0] = devm_regulator_get(&pdev->dev, "vdd-perfcl");
	if (IS_ERR(vdd_perfcl.regulator[0])) {
		if (PTR_ERR(vdd_perfcl.regulator[0]) != -EPROBE_DEFER)
			dev_err(&pdev->dev, "Unable to get the perfcl vreg\n");
		return PTR_ERR(vdd_perfcl.regulator[0]);
	}
	vdd_perfcl.use_max_uV = true;

	/* Leakage constraints disallow a turbo vote during bootup */
	vdd_perfcl.skip_handoff = true;

	vdd_cbf.regulator[0] = devm_regulator_get(&pdev->dev, "vdd-cbf");
	if (IS_ERR(vdd_cbf.regulator[0])) {
		if (PTR_ERR(vdd_cbf.regulator[0]) != -EPROBE_DEFER)
			dev_err(&pdev->dev, "Unable to get the cbf vreg\n");
		return PTR_ERR(vdd_cbf.regulator[0]);
	}
	vdd_cbf.use_max_uV = true;

	rc = qcom_cpu_parse_speedbin(pdev);
	if (rc)
		return rc;

	return rc;
}

static int qcom_cpu_clk_msm8996_driver_probe(struct platform_device *pdev)
{
	int ret;
	struct resource *res, *cbf_res;
	struct regmap *regmap_cpu, *regmap_cbf;
	struct clk_hw_clks *hws;
	struct clk_hw_onecell_data *data;
	struct device *dev = &pdev->dev;
	struct device_node *node = dev->of_node;
	struct clk *tmpclk = NULL;

	/*
	 * Require the CXO_A and APCS AUX clocks to be registered
	 * prior to the CPU driver: we are using the AUX clock to
	 * get a safe rate for all of our clocks!
	 */
	tmpclk = devm_clk_get(&pdev->dev, "xo_ao");
	if (IS_ERR(tmpclk)) {
		dev_err(&pdev->dev, "The XO_AO clock cannot be found.\n");

		if (PTR_ERR(tmpclk) != -EPROBE_DEFER)
			dev_err(&pdev->dev,
				"Critical error %ld. Bailing out.\n",
				PTR_ERR(tmpclk));

		return PTR_ERR(tmpclk);
	}

	tmpclk = devm_clk_get(&pdev->dev, "aux_clk");
	if (IS_ERR(tmpclk)) {
		dev_err(&pdev->dev, "The AUX clock cannot be found.\n");

		if (PTR_ERR(tmpclk) != -EPROBE_DEFER)
			dev_err(&pdev->dev,
				"Critical error %ld. Bailing out.\n",
				PTR_ERR(tmpclk));

		return PTR_ERR(tmpclk);
	}

	/* Parent clocks are available! Initialize the APCC! */
	data = devm_kzalloc(dev, sizeof(*data) + 2 * sizeof(struct clk_hw *),
			    GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	hws = devm_kzalloc(dev, sizeof(*hws) + 4 * sizeof(struct clk_hw *),
			   GFP_KERNEL);
	if (!hws)
		return -ENOMEM;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	vbases[APC_BASE] = devm_ioremap_resource(dev, res);
	if (IS_ERR(vbases[APC_BASE]))
		return PTR_ERR(vbases[APC_BASE]);

	cbf_res = platform_get_resource_byname(pdev, IORESOURCE_MEM,
							"cbf_base");
	vbases[CBF_BASE] = devm_ioremap_resource(dev, cbf_res);
	if (IS_ERR(vbases[CBF_BASE]))
		return PTR_ERR(vbases[CBF_BASE]);

	regmap_cpu = devm_regmap_init_mmio(dev, vbases[APC_BASE],
					   &cpu_msm8996_regmap_config);
	if (IS_ERR(regmap_cpu))
		return PTR_ERR(regmap_cpu);

	regmap_cbf = devm_regmap_init_mmio(dev, vbases[CBF_BASE],
					   &cbf_msm8996_regmap_config);
	if (IS_ERR(regmap_cbf))
		return PTR_ERR(regmap_cbf);

	cbf_pll.clkr.regmap = regmap_cbf;
	cbf_div_mux.clkr.regmap = regmap_cbf;
	cbf_mux.clkr.regmap = regmap_cbf;

	ret = cpu_parse_devicetree(pdev);
	if (ret)
		return ret;

	ret = qcom_cpu_clk_msm8996_register_clks(dev, hws, regmap_cpu);
	if (ret)
		return ret;

	ret = qcom_cbf_clk_msm8996_register_clks(dev, hws, regmap_cbf);
	if (ret)
		return ret;

	qcom_cpu_clk_msm8996_acd_init();

	data->hws[P_PWRCL_MUX] = &pwrcl_pmux.clkr.hw;
	data->hws[P_PERFCL_MUX] = &perfcl_pmux.clkr.hw;
	data->hws[P_CBF_MUX] = &cbf_mux.clkr.hw;

	data->num = 3;

	platform_set_drvdata(pdev, hws);

	ret = of_clk_add_hw_provider(node, of_clk_hw_onecell_get, data);
	if (ret) {
		dev_err(dev, "CRITICAL: Cannot add clock provider!\n");
		return ret;
	}

	populate_opp_table(pdev);

	return ret;
}

static int qcom_cpu_clk_msm8996_driver_remove(struct platform_device *pdev)
{
	int i;
	struct device *dev = &pdev->dev;
	struct clk_hw_clks *hws = platform_get_drvdata(pdev);

	for (i = 0; i < hws->num; i++)
		clk_hw_unregister_fixed_rate(hws->hws[i]);

	of_clk_del_provider(dev->of_node);

	return 0;
}

static struct platform_driver qcom_cpu_clk_msm8996_driver = {
	.probe = qcom_cpu_clk_msm8996_driver_probe,
	.remove = qcom_cpu_clk_msm8996_driver_remove,
	.driver = {
		.name = "qcom-msm8996-apcc",
		.of_match_table = match_table,
	},
};

module_platform_driver(qcom_cpu_clk_msm8996_driver);

MODULE_ALIAS("platform:msm8996-apcc");
MODULE_DESCRIPTION("QCOM MSM8996 CPU clock Driver");
MODULE_LICENSE("GPL v2");
