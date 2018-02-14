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
#include "clk-alpha-pll.h"
#include <soc/qcom/kryo-l2-accessors.h>

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
#define MUX_OFFSET	0x40
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

enum {
	APC_BASE,
	EFUSE_BASE,
	NUM_BASES
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

static struct clk_cpu_8996_mux pwrcl_pmux = {
	.reg = PWRCL_REG_OFFSET + MUX_OFFSET,
	.shift = 0,
	.width = 2,
	.pll = &pwrcl_pll.clkr.hw,
	.pll_div_2 = &pwrcl_smux.clkr.hw,
	.nb.notifier_call = cpu_clk_notifier_cb,
	.clkr.hw.init = &(struct clk_init_data) {
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
};

static struct clk_cpu_8996_mux perfcl_pmux = {
	.reg = PERFCL_REG_OFFSET + MUX_OFFSET,
	.shift = 0,
	.width = 2,
	.pll = &perfcl_pll.clkr.hw,
	.pll_div_2 = &perfcl_smux.clkr.hw,
	.nb.notifier_call = cpu_clk_notifier_cb,
	.clkr.hw.init = &(struct clk_init_data) {
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
};

static const struct regmap_config cpu_msm8996_regmap_config = {
	.reg_bits		= 32,
	.reg_stride		= 4,
	.val_bits		= 32,
	.max_register		= 0x80210,
	.fast_io		= true,
	.val_format_endian	= REGMAP_ENDIAN_LITTLE,
};

static const struct of_device_id match_table[] = {
	{ .compatible = "qcom-msm8996-apcc" },
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

struct clk_hw_clks {
	unsigned int num;
	struct clk_hw *hws[];
};

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

	hws->num = 2;

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
static int qcom_cpu_clk_msm8996_driver_probe(struct platform_device *pdev)
{
	int ret;
	struct resource *res;
	struct regmap *regmap_cpu;
	struct clk_hw_clks *hws;
	struct clk_hw_onecell_data *data;
	struct device *dev = &pdev->dev;
	struct device_node *node = dev->of_node;

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

	regmap_cpu = devm_regmap_init_mmio(dev, vbases[APC_BASE],
					   &cpu_msm8996_regmap_config);
	if (IS_ERR(regmap_cpu))
		return PTR_ERR(regmap_cpu);

	ret = qcom_cpu_clk_msm8996_register_clks(dev, hws, regmap_cpu);
	if (ret)
		return ret;
	qcom_cpu_clk_msm8996_acd_init();

	data->hws[0] = &pwrcl_pmux.clkr.hw;
	data->hws[1] = &perfcl_pmux.clkr.hw;

	data->num = 2;

	platform_set_drvdata(pdev, hws);

	return of_clk_add_hw_provider(node, of_clk_hw_onecell_get, data);
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
