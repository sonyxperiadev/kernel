/*
 * MSM8976-class SoC L2 Cache and CPU Clock Driver - Mainline API Impl.
 * Copyright (C) 2018, AngeloGioacchino Del Regno <kholk11@gmail.com>
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

#define pr_fmt(fmt) "%s: " fmt, __func__

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/cpu.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/pm_opp.h>
#include <linux/pm_qos.h>
#include <linux/regulator/consumer.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/regmap.h>
#include <linux/syscore_ops.h>
#include <linux/regulator/rpm-smd-regulator.h>

#include <dt-bindings/clock/qcom,cpu-8976.h>

#include "clk-debug.h"
#include "common.h"
#include "clk-pll.h"
#include "clk-hfpll.h"
#include "clk-rcg.h"
#include "clk-regmap-mux-div.h"

enum {
	APCS_C0_PLL_BASE,
	APCS_C1_PLL_BASE,
	APCS_CCI_PLL_BASE,
	APCS0_DBG_BASE,
	N_BASES,
};

enum {
	A53SS_MUX_C0,
	A53SS_MUX_C1,
	A53SS_MUX_CCI,
	A53SS_MUX_NUM,
};


enum {
	A72_CLUSTER_PLL,
	A53_CLUSTER_PLL,
	CCI_PLL,
	A72_CLUSTER_MUX,
	A53_CLUSTER_MUX,
	CCISS_MUX,
	MSM8976_REGMAP_MAX,
};

enum vdd_mx_pll_levels {
	VDD_MX_OFF,
	VDD_MX_SVS,
	VDD_MX_NOM,
	VDD_MX_TUR,
	VDD_MX_NUM,
};

#define GLB_DIAG	0x0b11101c
extern int clock_rcgwr_init(struct platform_device *pdev);

static int vdd_hf_levels[] = {
	0,		RPM_REGULATOR_LEVEL_NONE,	/* VDD_PLL_OFF */
	1800000,	RPM_REGULATOR_LEVEL_SVS,	/* VDD_PLL_SVS */
	1800000,	RPM_REGULATOR_LEVEL_NOM,	/* VDD_PLL_NOM */
	1800000,	RPM_REGULATOR_LEVEL_TURBO,	/* VDD_PLL_TUR */
};

static int vdd_sr_levels[] = {
	RPM_REGULATOR_LEVEL_NONE,		/* VDD_PLL_OFF */
	RPM_REGULATOR_LEVEL_SVS,		/* VDD_PLL_SVS */
	RPM_REGULATOR_LEVEL_NOM,		/* VDD_PLL_NOM */
	RPM_REGULATOR_LEVEL_TURBO,		/* VDD_PLL_TUR */
};

static void __iomem *virt_bases[N_BASES];
static void __iomem *mux_bases[4];
struct platform_device *cpu_clock_dev_8976;

static DEFINE_VDD_REGS_INIT(vdd_cpu_a72, 1);
static DEFINE_VDD_REGS_INIT(vdd_cpu_a53, 1);
static DEFINE_VDD_REGS_INIT(vdd_cpu_cci, 1);

static DEFINE_VDD_REGULATORS(vdd_hf, VDD_MX_NUM, 2,
				vdd_hf_levels);

static DEFINE_VDD_REGULATORS(vdd_mx_sr, VDD_MX_NUM, 1,
				vdd_sr_levels);

#define VDD_MX_HF_FMAX_MAP2(l1, f1, l2, f2)		\
	.vdd_class = &vdd_hf,				\
	.rate_max = (unsigned long[VDD_MX_NUM]) {	\
		[VDD_MX_##l1] = (f1),			\
		[VDD_MX_##l2] = (f2),			\
	},						\
	.num_rate_max = VDD_MX_NUM

#define VDD_MX_SR_FMAX_MAP2(l1, f1, l2, f2)		\
	.vdd_class = &vdd_mx_sr,			\
	.rate_max = (unsigned long[VDD_MX_NUM]) {	\
		[VDD_MX_##l1] = (f1),			\
		[VDD_MX_##l2] = (f2),			\
	},						\
	.num_rate_max = VDD_MX_NUM

enum {
	I_CLUSTER_PLL_MAIN,
	I_CLUSTER_PLL,
	I_APCSAUX3,
	I_APCSAUX2,
};


static const struct parent_map cpuss_parent_map_a72[] = {
	{ I_CLUSTER_PLL_MAIN, 3 },
	{ I_CLUSTER_PLL, 5},
	{ I_APCSAUX3, 4 }, /* Safe clocks */
	{ I_APCSAUX2, 1 },
};

static const char * const cpuss_parent_names_a72[] = {
	"a72ss_hf_pll_main",
	"a72ss_hf_pll",
	"gpll0_ao_out_main",
	"gpll4_out_main",
};

static const struct parent_map cpuss_parent_map_a53[] = {
	{ I_CLUSTER_PLL_MAIN, 3 },
	{ I_CLUSTER_PLL, 5},
	{ I_APCSAUX3, 4 }, /* Safe clocks */
	{ I_APCSAUX2, 1 },
};

static const char * const cpuss_parent_names_a53[] = {
	"a53ss_sr_pll_main",
	"a53ss_sr_pll",
	"gpll0_ao_out_main",
	"gpll4_out_main",
};

static const struct parent_map cpuss_parent_map_cci[] = {
	{ I_CLUSTER_PLL_MAIN, 3 },
	{ I_CLUSTER_PLL, 5},
	{ I_APCSAUX3, 4 }, /* Safe clocks */
};

static const char * const cpuss_parent_names_cci[] = {
	"cci_sr_pll_main",
	"cci_sr_pll",
	"gpll0_ao_out_main",
};

/* Early output of PLL: 1.7GHz default */
static struct hfpll_data a72ss_hf_pll_data = {
	.mode_reg = 0x0,
	.l_reg = 0x4,
	.m_reg = 0x8,
	.n_reg = 0xC,
	.user_reg = 0x10,
	.config_reg = 0x14,
	.status_reg = 0x1C,
	.lock_bit = 16,
	.spm_offset = 0x50,
	.spm_event_bit = 0x4,
	.user_vco_mask = 0x3 << 28,
	.pre_div_mask = BIT(12),
	.pre_div_mask = 0,
	.post_div_mask = 0x300,
	.post_div_masked = 0x100,
	.early_output_mask =  0x8,
	.main_output_mask = BIT(0),
	.vco_mode_masked = 0x100000,
	.config_val = 0x04E0405D,
	.max_rate = 2016000000UL,
	.min_rate = 940800000UL,
	.l_val = 0x5B,
};

static struct clk_hfpll a72ss_hf_pll = {
	.d = &a72ss_hf_pll_data,
	.init_done = false,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "a72ss_hf_pll",
		.parent_names = (const char *[]){ "xo_a" },
		.num_parents = 1,
		.ops = &clk_ops_hf2_pll,
		/* MX level of MSM is much higher than of PLL */
		VDD_MX_HF_FMAX_MAP2(SVS, 2000000000, NOM, 2900000000UL),
	},
};

static struct clk_fixed_factor a72ss_hf_pll_main = {
	.mult = 1,
	.div = 1,
	.hw.init = &(struct clk_init_data){
		.name = "a72ss_hf_pll_main",
		.parent_names = (const char *[]){ "a72ss_hf_pll" },
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
		.ops = &clk_fixed_factor_ops,
	},
};

/* Early output of PLL: 1.4GHz default */
static struct hfpll_data a53ss_sr_pll_data = {
	.mode_reg = 0x0,
	.l_reg = 0x4,
	.m_reg = 0x8,
	.n_reg = 0xC,
	.user_reg = 0x10,
	.config_reg = 0x14,
	.status_reg = 0x1C,
	.lock_bit = 16,
	.spm_offset = 0x50,
	.spm_event_bit = 0x4,
	.user_vco_mask = 0x3 << 20,
	.pre_div_mask = 0x7 << 12,
	.post_div_mask = 0x3 << 8,
	.post_div_masked =  0x1 << 8,
	.early_output_mask =  BIT(3),
	.main_output_mask = BIT(0),
	.config_val = 0x00341600,
	.user_vco_val = 0x00141400,
	.vco_mode_masked = BIT(20),
	.min_rate = 652800000UL,
	.max_rate = 1478400000UL,
	.low_vco_max_rate = 902400000UL,
	.l_val = 0x49,
};

static struct clk_hfpll a53ss_sr_pll = {
	.d = &a53ss_sr_pll_data,
	.init_done = false,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "a53ss_sr_pll",
		.parent_names = (const char *[]){ "xo_a" },
		.num_parents = 1,
		.ops = &clk_ops_hf2_pll,
		VDD_MX_SR_FMAX_MAP2(SVS, 1000000000, NOM, 2200000000UL),
	},
};

static struct clk_fixed_factor a53ss_sr_pll_main = {
	.mult = 1,
	.div = 1,
	.hw.init = &(struct clk_init_data){
		.name = "a53ss_sr_pll_main",
		.parent_names = (const char *[]){ "a53ss_sr_pll" },
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
		.ops = &clk_fixed_factor_ops,
	},
};

/* Early output of PLL: 614.4MHz */
static struct hfpll_data cci_sr_pll_data = {
	.mode_reg = 0x0,
	.l_reg = 0x4,
	.m_reg = 0x8,
	.n_reg = 0xC,
	.user_reg = 0x10,
	.config_reg = 0x14,
	.status_reg = 0x1C,
	.lock_bit = 16,
	.spm_offset = 0x40,
	.spm_event_bit = 0x0,
	.user_vco_mask = 0x3 << 20,
	.pre_div_mask = 0x7 << 12,
	.post_div_mask = 0x3 << 8,
	.early_output_mask =  BIT(3),
	.main_output_mask = BIT(0),
	.post_div_masked = 0x1 << 8,
	.vco_mode_masked = BIT(20),
	.config_val = 0x00141400,
	.min_rate = 307200000UL,
	.max_rate = 902400000UL,
	.l_val = 0x20,
};

static struct clk_hfpll cci_sr_pll = {
	.d = &cci_sr_pll_data,
	.init_done = false,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "cci_sr_pll",
		.parent_names = (const char *[]){ "xo_a" },
		.num_parents = 1,
		.ops = &clk_ops_hf2_pll,
		VDD_MX_SR_FMAX_MAP2(SVS, 1000000000, NOM, 2200000000UL),
	},
};

static struct clk_fixed_factor cci_sr_pll_main = {
	.mult = 1,
	.div = 1,
	.hw.init = &(struct clk_init_data){
		.name = "cci_sr_pll_main",
		.parent_names = (const char *[]){ "cci_sr_pll" },
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
		.ops = &clk_fixed_factor_ops,
	},
};

static struct clk_fixed_factor sys_apcsaux_clk_2 = {
	.div = 1,
	.mult = 1,
	.hw.init = &(struct clk_init_data){
		.name = "sys_apcsaux_clk_2",
		.parent_names = (const char*[]){ "gpll4_out_main" },
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
		.ops = &clk_fixed_factor_ops,
	},
};

static struct clk_fixed_factor sys_apcsaux_clk_3 = {
	.div = 1,
	.mult = 1,
	.hw.init = &(struct clk_init_data){
		.name = "sys_apcsaux_clk_3",
		.parent_names = (const char*[]){ "gpll0_ao_out_main" },
		.num_parents = 1,
		.ops = &clk_fixed_factor_ops,
	},
};

static const struct regmap_config pll_blocks_regmap_config = {
	.reg_bits       = 32,
	.reg_stride     = 4,
	.val_bits       = 32,
	.max_register	= 0x40,
	.fast_io        = true,
};

static const struct regmap_config mux_blocks_regmap_config = {
	.reg_bits       = 32,
	.reg_stride     = 4,
	.val_bits       = 32,
	.max_register	= 0x8,
	.fast_io        = true,
};

static const char const *mux_names[] = {"c0", "c1", "cci"};

#define CPU_LATENCY_NO_L2_PC_US (280)

#define to_clk_regmap_mux_div(_hw) \
	container_of(to_clk_regmap(_hw), struct clk_regmap_mux_div, clkr)

static unsigned long cpu_clk_8976_recalc_rate(struct clk_hw *hw,
					unsigned long prate)
{
	struct clk_regmap_mux_div *cpuclk = to_clk_regmap_mux_div(hw);
	struct clk_hw *parent;
	unsigned long parent_rate;
	u32 i, div, num_parents, src = 0;
	int ret = 0;

	ret = mux_div_get_src_div(cpuclk, &src, &div);
	if (ret)
		return ret;

	num_parents = clk_hw_get_num_parents(hw);
	for (i = 0; i < num_parents; i++) {
		if (src == cpuclk->parent_map[i].cfg) {
			parent = clk_hw_get_parent_by_index(hw, i);
			parent_rate = clk_hw_get_rate(parent);

			parent_rate *= 2;
			parent_rate /= div + 1;
			return parent_rate;
		}
	}
	pr_err("%s: Can't find parent %d\n", clk_hw_get_name(hw), src);
	return ret;
}

struct cpu_desc_8976 {
	cpumask_t cpumask;
	struct pm_qos_request req;
};

enum {
	AUX_DIV2,
	AUX_FULL,
	AUX_MAX_AVAILABLE_RATES,
};

struct clk_cpu_8976_data {
	struct cpu_desc_8976 a53ssmux_desc;
	struct cpu_desc_8976 a72ssmux_desc;
	unsigned long aux3_rates[AUX_MAX_AVAILABLE_RATES];
	unsigned long aux2_rates[AUX_MAX_AVAILABLE_RATES];
};
static struct clk_cpu_8976_data cpu_data;

static void do_nothing(void *unused) { }

static int cpu_clk_8976_set_rate(struct clk_hw *hw, unsigned long rate,
			unsigned long parent_rate)
{
	struct clk_regmap_mux_div *cpuclk = to_clk_regmap_mux_div(hw);

	return __mux_div_set_src_div(cpuclk, cpuclk->src, cpuclk->div);
}

static int cpu_clk_8976_set_rate_and_parent(struct clk_hw *hw,
						unsigned long rate,
						unsigned long prate,
						u8 index)
{
	struct clk_regmap_mux_div *cpuclk = to_clk_regmap_mux_div(hw);

	return __mux_div_set_src_div(cpuclk, cpuclk->parent_map[index].cfg,
					cpuclk->div);
}

static void cpu_clk_8976_pm_qos_add_req(struct cpu_desc_8976* cluster_desc)
{
	memset(&cluster_desc->req, 0, sizeof(cluster_desc->req));
	cpumask_copy(&(cluster_desc->req.cpus_affine),
			(const struct cpumask *)&cluster_desc->cpumask);
	cluster_desc->req.type = PM_QOS_REQ_AFFINE_CORES;
	pm_qos_add_request(&cluster_desc->req, PM_QOS_CPU_DMA_LATENCY,
			CPU_LATENCY_NO_L2_PC_US);
	smp_call_function_any(&cluster_desc->cpumask, do_nothing,
				NULL, 1);
}

static int cpu_clk_8976_set_rate_little(struct clk_hw *hw, unsigned long rate,
			unsigned long parent_rate)
{
	int rc;

	cpu_clk_8976_pm_qos_add_req(&cpu_data.a53ssmux_desc);

	rc = cpu_clk_8976_set_rate(hw, rate, parent_rate);

	pm_qos_remove_request(&cpu_data.a53ssmux_desc.req);

	return rc;
}

static int cpu_clk_8976_set_rate_and_parent_little(struct clk_hw *hw,
						unsigned long rate,
						unsigned long prate,
						u8 index)
{
	int rc;

	cpu_clk_8976_pm_qos_add_req(&cpu_data.a53ssmux_desc);

	rc = cpu_clk_8976_set_rate_and_parent(hw, rate, prate, index);

	pm_qos_remove_request(&cpu_data.a53ssmux_desc.req);

	return rc;
}


static int cpu_clk_8976_set_rate_big(struct clk_hw *hw, unsigned long rate,
			unsigned long parent_rate)
{
	int rc;

	cpu_clk_8976_pm_qos_add_req(&cpu_data.a72ssmux_desc);

	rc = cpu_clk_8976_set_rate(hw, rate, parent_rate);

	pm_qos_remove_request(&cpu_data.a72ssmux_desc.req);

	return rc;
}

static int cpu_clk_8976_set_rate_and_parent_big(struct clk_hw *hw,
						unsigned long rate,
						unsigned long prate,
						u8 index)
{
	int rc;

	cpu_clk_8976_pm_qos_add_req(&cpu_data.a72ssmux_desc);

	rc = cpu_clk_8976_set_rate_and_parent(hw, rate, prate, index);

	pm_qos_remove_request(&cpu_data.a72ssmux_desc.req);

	return rc;
}

static int cpu_clk_8976_enable(struct clk_hw *hw)
{
	return clk_regmap_mux_div_ops.enable(hw);
}

static void cpu_clk_8976_disable(struct clk_hw *hw)
{
	struct clk_regmap_mux_div *cpuclk = to_clk_regmap_mux_div(hw);

	clk_regmap_mux_div_ops.disable(hw);

	cpuclk->div = cpuclk->safe_div;
	cpuclk->src = cpuclk->safe_src;
}

static u8 cpu_clk_8976_get_parent(struct clk_hw *hw)
{
	return clk_regmap_mux_div_ops.get_parent(hw);
}

static int cpu_clk_8976_set_parent(struct clk_hw *hw, u8 index)
{
	/*
	 * We have defined set_rate_and_parent and this function
	 * will never get called in this driver's whole life,
	 * however, we need to define it to make the clk API to
	 * be happy.
	 */
	return 0;
}

static int cpu_clk_8976_determine_rate(struct clk_hw *hw,
					struct clk_rate_request *req)
{
	int ret;
	u32 div = 1;
	struct clk_hw *clk_parent, *cpu_pll_hw;
	unsigned long mask, pll_rate, rate = req->rate;
	struct clk_rate_request parent_req = { };
	struct clk_regmap_mux_div *cpuclk = to_clk_regmap_mux_div(hw);
	int clk_index = I_APCSAUX3; /* Default to GPLL0_AO auxiliary */

	cpu_pll_hw = clk_hw_get_parent_by_index(hw, I_CLUSTER_PLL);
	if (!cpu_pll_hw) {
		/* Force using the APCS safe auxiliary source (GPLL0_AO) */
		pll_rate = ULONG_MAX;
	} else {
		pll_rate = cpu_data.aux3_rates[AUX_FULL];

		/* If GPLL0_AO is out of range, try to use GPLL4 */
		if (rate > cpu_data.aux3_rates[AUX_DIV2] &&
		    rate != pll_rate) {
			clk_index = I_APCSAUX2;
			clk_parent =
				clk_hw_get_parent_by_index(hw, I_APCSAUX2);
			pll_rate = cpu_data.aux2_rates[AUX_FULL];
		}
	}

	if (rate <= pll_rate) {
		/* Use one of the APCSAUX as clock source */
		clk_parent = clk_hw_get_parent_by_index(hw, clk_index);
		mask = BIT(cpuclk->hid_width) - 1;

		/*
		 * Avoid powering on the specific cluster PLL to save
		 * power whenever a low CPU frequency is requested for
		 * that cluster.
		 */
		req->best_parent_hw = clk_parent;
		req->best_parent_rate = pll_rate;

		div = DIV_ROUND_UP((2 * req->best_parent_rate), rate) - 1;
		div = min_t(unsigned long, div, mask);

		req->rate = req->best_parent_rate * 2;
		req->rate /= div + 1;

		cpuclk->src = cpuclk->parent_map[clk_index].cfg;
	} else {
		/* Use the cluster specific PLL as clock source */
		clk_index = I_CLUSTER_PLL_MAIN;
		clk_parent = clk_hw_get_parent_by_index(hw, clk_index);

		/*
		 * Originally, we would run the PLL _always_ at maximum
		 * frequency and postdivide the frequency to get where
		 * we want to be, but we can save some battery time.
		 *
		 * To save power, it's better to set the PLL to give us
		 * the clock that we want.
		 */
		parent_req.rate = rate;
		parent_req.best_parent_hw = clk_parent;

		req->best_parent_hw = clk_parent;
		ret = __clk_determine_rate(req->best_parent_hw, &parent_req);

		cpuclk->src = cpuclk->parent_map[I_CLUSTER_PLL].cfg;
		req->best_parent_rate = parent_req.rate;
	}
	cpuclk->div = div;

	return 0;
}

static int cpu_clk_8976_determine_rate_cci(struct clk_hw *hw,
					struct clk_rate_request *req)
{
	int ret;
	u32 div = 1;
	struct clk_hw *cpu_pll_main_hw;
	unsigned long rate = req->rate;
	struct clk_rate_request parent_req = { };
	struct clk_regmap_mux_div *cpuclk = to_clk_regmap_mux_div(hw);
	int pll_clk_index = I_CLUSTER_PLL;

	cpu_pll_main_hw = clk_hw_get_parent_by_index(hw, I_CLUSTER_PLL_MAIN);

	parent_req.rate = rate;
	parent_req.best_parent_hw = cpu_pll_main_hw;

	req->best_parent_hw = cpu_pll_main_hw;
	ret = __clk_determine_rate(req->best_parent_hw, &parent_req);

	cpuclk->src = cpuclk->parent_map[pll_clk_index].cfg;
	req->best_parent_rate = parent_req.rate;

	cpuclk->div = div;

	return 0;
}

static const struct clk_ops clk_ops_cpu_little = {
	.enable = cpu_clk_8976_enable,
	.disable = cpu_clk_8976_disable,
	.get_parent = cpu_clk_8976_get_parent,
	.set_rate = cpu_clk_8976_set_rate_little,
	.set_rate_and_parent = cpu_clk_8976_set_rate_and_parent_little,
	.set_parent = cpu_clk_8976_set_parent,
	.recalc_rate = cpu_clk_8976_recalc_rate,
	.determine_rate = cpu_clk_8976_determine_rate,
	.debug_init = clk_debug_measure_add,
};

static const struct clk_ops clk_ops_cpu_big = {
	.enable = cpu_clk_8976_enable,
	.disable = cpu_clk_8976_disable,
	.get_parent = cpu_clk_8976_get_parent,
	.set_rate = cpu_clk_8976_set_rate_big,
	.set_rate_and_parent = cpu_clk_8976_set_rate_and_parent_big,
	.set_parent = cpu_clk_8976_set_parent,
	.recalc_rate = cpu_clk_8976_recalc_rate,
	.determine_rate = cpu_clk_8976_determine_rate,
	.debug_init = clk_debug_measure_add,
};

static const struct clk_ops clk_ops_cci = {
	.enable = cpu_clk_8976_enable,
	.disable = cpu_clk_8976_disable,
	.get_parent = cpu_clk_8976_get_parent,
	.set_rate = cpu_clk_8976_set_rate,
	.set_rate_and_parent = cpu_clk_8976_set_rate_and_parent,
	.set_parent = cpu_clk_8976_set_parent,
	.recalc_rate = cpu_clk_8976_recalc_rate,
	.determine_rate = cpu_clk_8976_determine_rate_cci,
	.debug_init = clk_debug_measure_add,
};

/*
 * Temporarily switch to the always-on safe AUX3 clock prior changing the
 * cluster rate to avoid unstabilities during PLL reconfiguration.
 */
static int cpu_clk_8976_notifier_cb(struct notifier_block *nb,
				unsigned long evt, void *data)
{
	int ret = 0;
	struct clk_regmap_mux_div *cpuclk = container_of(nb,
					struct clk_regmap_mux_div, clk_nb);

	if (evt == PRE_RATE_CHANGE)
		ret = __mux_div_set_src_div(cpuclk, 4, 1);

	if (evt == ABORT_RATE_CHANGE)
		pr_err("Error in configuring PLL - stay at safe src only\n");

	return notifier_from_errno(ret);
}

static int cpu_clk_8976_cci_notifier_cb(struct notifier_block *nb,
				unsigned long evt, void *data)
{
	int ret = 0;
	struct clk_regmap_mux_div *cpuclk = container_of(nb,
					struct clk_regmap_mux_div, clk_nb);

	if (evt == PRE_RATE_CHANGE)
		ret = __mux_div_set_src_div(cpuclk, 4, 4);

	if (evt == ABORT_RATE_CHANGE)
		pr_err("Error in configuring PLL - stay at safe src only\n");

	return notifier_from_errno(ret);
}

static struct clk_init_data cpu_8976_clk_init_data[A53SS_MUX_NUM] = {
	[A53SS_MUX_C0] = {
		.name = "a53ssmux",
		.parent_names = cpuss_parent_names_a53,
		.num_parents = ARRAY_SIZE(cpuss_parent_map_a53),
		.ops = &clk_ops_cpu_little,
		.vdd_class = &vdd_cpu_a53,
		.flags = CLK_SET_RATE_PARENT,
	},
	[A53SS_MUX_C1] = {
		.name = "a72ssmux",
		.parent_names = cpuss_parent_names_a72,
		.num_parents = ARRAY_SIZE(cpuss_parent_names_a72),
		.ops = &clk_ops_cpu_big,
		.vdd_class = &vdd_cpu_a72,
		.flags = CLK_SET_RATE_PARENT,
	},
	[A53SS_MUX_CCI] = {
		.name = "ccissmux",
		.parent_names = cpuss_parent_names_cci,
		.num_parents = ARRAY_SIZE(cpuss_parent_names_cci),
		.ops = &clk_ops_cci,
		.vdd_class = &vdd_cpu_cci,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap_mux_div a72ssmux = {
	.reg_offset = 0x0,
	.hid_width = 5,
	.hid_shift = 0,
	.src_width = 3,
	.src_shift = 8,
	.safe_src = 4,
	.safe_div = 2,
	.safe_freq = 400000000,
	.parent_map = cpuss_parent_map_a72,
	.clk_nb.notifier_call = cpu_clk_8976_notifier_cb,
	.clkr.hw.init = &cpu_8976_clk_init_data[A53SS_MUX_C1],
};

static struct clk_regmap_mux_div a53ssmux = {
	.reg_offset = 0x0,
	.hid_width = 5,
	.hid_shift = 0,
	.src_width = 3,
	.src_shift = 8,
	.safe_src = 4,
	.safe_div = 2,
	.safe_freq = 400000000,
	.parent_map = cpuss_parent_map_a53,
	.clk_nb.notifier_call = cpu_clk_8976_notifier_cb,
	.clkr.hw.init = &cpu_8976_clk_init_data[A53SS_MUX_C0],
};

static struct clk_regmap_mux_div ccissmux = {
	.reg_offset = 0x0,
	.hid_width = 5,
	.hid_shift = 0,
	.src_width = 3,
	.src_shift = 8,
	.safe_src = 4,
	.safe_div = 4,
	.safe_freq = 200000000,
	.parent_map = cpuss_parent_map_cci,
	.clk_nb.notifier_call = cpu_clk_8976_cci_notifier_cb,
	.clkr.hw.init = &cpu_8976_clk_init_data[A53SS_MUX_CCI],
};

static struct clk_hw *clk_cpu_8976_hws[] = {
	[P_APCSAUX_2]		= &sys_apcsaux_clk_2.hw,
	[P_APCSAUX_3]		= &sys_apcsaux_clk_3.hw,
	[P_A72_HF_PLL]		= &a72ss_hf_pll.clkr.hw,
	[P_A53_SR_PLL]		= &a53ss_sr_pll.clkr.hw,
	[P_CCI_SR_PLL]		= &cci_sr_pll.clkr.hw,
	[P_A72_HF_PLL_MAIN]	= &a72ss_hf_pll_main.hw,
	[P_A53_SR_PLL_MAIN]	= &a53ss_sr_pll_main.hw,
	[P_CCI_SR_PLL_MAIN]	= &cci_sr_pll_main.hw,
	[P_A72_CLK]		= &a72ssmux.clkr.hw,
	[P_A53_CLK]		= &a53ssmux.clkr.hw,
	[P_CCI_CLK]		= &ccissmux.clkr.hw,
};

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

static void print_opp_table(int a53_cpu, int a72_cpu)
{
	struct dev_pm_opp *oppfmax, *oppfmin;
	unsigned int apc0_rate_max = a53ssmux.clkr.hw.init->num_rate_max - 1;
	unsigned int apc1_rate_max = a72ssmux.clkr.hw.init->num_rate_max - 1;
	unsigned long apc0_fmax =
		a53ssmux.clkr.hw.init->rate_max[apc0_rate_max];
	unsigned long apc1_fmax =
		a72ssmux.clkr.hw.init->rate_max[apc1_rate_max];
	unsigned long apc0_fmin = a53ssmux.clkr.hw.init->rate_max[1];
	unsigned long apc1_fmin = a72ssmux.clkr.hw.init->rate_max[1];

	rcu_read_lock();

	oppfmax = dev_pm_opp_find_freq_exact(get_cpu_device(a53_cpu), apc0_fmax,
					     true);
	oppfmin = dev_pm_opp_find_freq_exact(get_cpu_device(a53_cpu), apc0_fmin,
					     true);
	/*
	 * One time information during boot. Important to know that this looks
	 * sane since it can eventually make its way to the scheduler.
	 */
	pr_info("clock_cpu: a53: OPP voltage for %lu: %ld\n", apc0_fmin,
		dev_pm_opp_get_voltage(oppfmin));
	pr_info("clock_cpu: a53: OPP voltage for %lu: %ld\n", apc0_fmax,
		dev_pm_opp_get_voltage(oppfmax));

	oppfmax = dev_pm_opp_find_freq_exact(get_cpu_device(a72_cpu), apc1_fmax,
					     true);
	oppfmin = dev_pm_opp_find_freq_exact(get_cpu_device(a72_cpu), apc1_fmin,
					     true);
	pr_info("clock_cpu: a72: OPP voltage for %lu: %lu\n", apc1_fmin,
		dev_pm_opp_get_voltage(oppfmin));
	pr_info("clock_cpu: a72: OPP voltage for %lu: %lu\n", apc1_fmax,
		dev_pm_opp_get_voltage(oppfmax));

	rcu_read_unlock();
}

static void populate_opp_table(struct platform_device *pdev)
{
	struct platform_device *apc0_dev, *apc1_dev;
	struct device_node *apc0_node = NULL, *apc1_node = NULL;
	unsigned long apc0_fmax, apc1_fmax;
	unsigned int apc0_rate_max = 0, apc1_rate_max = 0;
	int cpu, a53_cpu = 0, a72_cpu = 0;

	apc0_node = of_parse_phandle(pdev->dev.of_node, "vdd_a53-supply", 0);
	if (!apc0_node) {
		pr_err("can't find the apc0 dt node.\n");
		return;
	}

	apc1_node = of_parse_phandle(pdev->dev.of_node, "vdd_a72-supply", 0);
	if (!apc1_node) {
		pr_err("can't find the apc1 dt node.\n");
		return;
	}

	apc0_dev = of_find_device_by_node(apc0_node);
	if (!apc0_dev) {
		pr_err("can't find the apc0 device node.\n");
		return;
	}

	apc1_dev = of_find_device_by_node(apc1_node);
	if (!apc1_dev) {
		pr_err("can't find the apc1 device node.\n");
		return;
	}

	apc0_rate_max = a53ssmux.clkr.hw.init->num_rate_max - 1;
	apc1_rate_max = a72ssmux.clkr.hw.init->num_rate_max - 1;
	apc0_fmax = a53ssmux.clkr.hw.init->rate_max[apc0_rate_max];
	apc1_fmax = a72ssmux.clkr.hw.init->rate_max[apc1_rate_max];

	for_each_possible_cpu(cpu) {
		if (cpu <= 3) {
			a53_cpu = cpu;
			WARN(add_opp(&a53ssmux.clkr.hw, get_cpu_device(cpu),
				     &apc0_dev->dev, apc0_fmax),
				     "Failed to add OPP levels for A53\n");
		} else {
			a72_cpu = cpu;
			WARN(add_opp(&a72ssmux.clkr.hw, get_cpu_device(cpu),
				     &apc1_dev->dev, apc1_fmax),
				     "Failed to add OPP levels for A72\n");
		}
	}

	/* One time print during bootup */
	pr_info("clock-cpu-8976: OPP tables populated (cpu 0-%d and %d-%d)\n",
						a53_cpu, a53_cpu+1, a72_cpu);

	print_opp_table(a53_cpu, a72_cpu);
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

	vdd = cpu_8976_clk_init_data[mux_id].vdd_class;

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

	cpu_8976_clk_init_data[mux_id].rate_max = devm_kzalloc(&pdev->dev,
					prop_len * sizeof(unsigned long),
					GFP_KERNEL);
	if (!cpu_8976_clk_init_data[mux_id].rate_max) {
		pr_err("Cannot allocate memory for rate_max\n");
		return -ENOMEM;
	}

	array = devm_kzalloc(&pdev->dev,
			prop_len * sizeof(u32) * nreg, GFP_KERNEL);
	if (!array)
		return -ENOMEM;

	of_property_read_u32_array(of, prop_name, array, prop_len * nreg);
	for (i = 0; i < prop_len; i++) {
		cpu_8976_clk_init_data[mux_id].rate_max[i] = array[nreg * i];
		for (j = 1; j < nreg; j++)
			vdd->vdd_uv[(nreg - 1) * i + (j - 1)] =
					array[nreg * i + j];
	}

	devm_kfree(&pdev->dev, array);
	vdd->num_levels = prop_len;
	vdd->cur_level = prop_len;
	vdd->use_max_uV = true;
	cpu_8976_clk_init_data[mux_id].num_rate_max = prop_len;

	return 0;
}

static void get_speed_bin(struct platform_device *pdev, int *bin,
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

	*bin = (pte_efuse >> 2) & 0x7;

	dev_info(&pdev->dev, "Speed bin: %d PVS Version: %d\n", *bin,
								*version);
}

static int cpu_8976_map_pll(struct platform_device *pdev,
				struct clk_regmap *clkr,
				int virt_id, char* res_name)
{
	struct resource *res;

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, res_name);
	if (!res) {
		dev_err(&pdev->dev, "Cannot get cluster %s PLL\n", res_name);
		return -EINVAL;
	}

	virt_bases[virt_id] = devm_ioremap(&pdev->dev, res->start,
						resource_size(res));
	if (!virt_bases[APCS_C0_PLL_BASE]) {
		dev_err(&pdev->dev, "Cannot remap cluster %s PLL\n", res_name);
		return -EINVAL;
	}

	clkr->regmap = devm_regmap_init_mmio(&pdev->dev, virt_bases[virt_id],
				&pll_blocks_regmap_config);
	if (IS_ERR(clkr->regmap)) {
		dev_err(&pdev->dev, "Cannot init regmap MMIO for %s PLL\n",
				res_name);
		return PTR_ERR(clkr->regmap);
	};

	return 0;
}

static int cpu_8976_map_spm(struct platform_device *pdev)
{
	struct resource *res = NULL;

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM,
							"spm_c0_base");
	if (!res) {
		dev_err(&pdev->dev, "Register base not defined for c1\n");
		return -ENOMEM;
	}

	a53ss_sr_pll_data.spm_iobase = devm_ioremap(&pdev->dev, res->start,
							resource_size(res));
	if (!a53ss_sr_pll_data.spm_iobase) {
		dev_err(&pdev->dev, "Failed to ioremap c0 spm registers\n");
		return -ENOMEM;
	}

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM,
							"spm_c1_base");
	if (!res) {
		dev_err(&pdev->dev, "Register base not defined for c0\n");
		return -ENOMEM;
	}

	a72ss_hf_pll_data.spm_iobase = devm_ioremap(&pdev->dev, res->start,
							resource_size(res));
	if (!a72ss_hf_pll_data.spm_iobase) {
		dev_err(&pdev->dev, "Failed to ioremap c1 spm registers\n");
		return -ENOMEM;
	}

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM,
							"spm_cci_base");
	if (!res) {
		dev_err(&pdev->dev, "Register base not defined for cci\n");
		return -ENOMEM;
	}

	cci_sr_pll_data.spm_iobase = devm_ioremap(&pdev->dev, res->start,
							resource_size(res));
	if (!cci_sr_pll_data.spm_iobase) {
		dev_err(&pdev->dev, "Failed to ioremap cci spm registers\n");
		return -ENOMEM;
	}

	return 0;
}

static int cpu_parse_devicetree(struct platform_device *pdev)
{
	struct resource *res;
	int rc = 0;

	rc = cpu_8976_map_pll(pdev, &a53ss_sr_pll.clkr,
			APCS_C0_PLL_BASE, "c0-pll");
	if (rc)
		return rc;

	rc = cpu_8976_map_pll(pdev, &a72ss_hf_pll.clkr,
			APCS_C1_PLL_BASE, "c1-pll");
	if (rc)
		return rc;

	rc = cpu_8976_map_pll(pdev, &cci_sr_pll.clkr,
			APCS_CCI_PLL_BASE, "cci-pll");
	if (rc)
		return rc;

	rc = cpu_8976_map_spm(pdev);
	if (rc)
		return rc;

	/* HF PLL Analog Supply */
	vdd_hf.regulator[0] = devm_regulator_get(&pdev->dev,
							"vdd_hf_pll");
	if (IS_ERR(vdd_hf.regulator[0])) {
		if (PTR_ERR(vdd_hf.regulator[0]) != -EPROBE_DEFER)
			dev_err(&pdev->dev,
				"Unable to get vdd_hf_pll regulator!!!\n");
		return PTR_ERR(vdd_hf.regulator[0]);
	}

	/* HF PLL core logic */
	vdd_hf.regulator[1] = devm_regulator_get(&pdev->dev,
							"vdd_mx_hf");
	if (IS_ERR(vdd_hf.regulator[1])) {
		if (PTR_ERR(vdd_hf.regulator[1]) != -EPROBE_DEFER)
			dev_err(&pdev->dev,
				"Unable to get vdd_mx_hf regulator!!!\n");
		return PTR_ERR(vdd_hf.regulator[1]);
	}
	vdd_hf.use_max_uV = true;

	/* SR PLLs core logic */
	vdd_mx_sr.regulator[0] = devm_regulator_get(&pdev->dev,
							"vdd_mx_sr");
	if (IS_ERR(vdd_mx_sr.regulator[0])) {
		if (PTR_ERR(vdd_mx_sr.regulator[0]) != -EPROBE_DEFER)
			dev_err(&pdev->dev,
				"Unable to get vdd_mx_sr regulator!!!\n");
		return PTR_ERR(vdd_mx_sr.regulator[0]);
	}
	vdd_mx_sr.use_max_uV = true;

	vdd_cpu_a72.regulator[0] = devm_regulator_get(&pdev->dev,
							"vdd_a72");
	if (IS_ERR(vdd_cpu_a72.regulator[0])) {
		if (PTR_ERR(vdd_cpu_a72.regulator[0]) != -EPROBE_DEFER)
			dev_err(&pdev->dev,
				"Unable to get vdd_a72 regulator!!!\n");
		return PTR_ERR(vdd_cpu_a72.regulator[0]);
	}
	vdd_cpu_a72.use_max_uV = true;

	vdd_cpu_a53.regulator[0] = devm_regulator_get(&pdev->dev,
							"vdd_a53");
	if (IS_ERR(vdd_cpu_a53.regulator[0])) {
		if (PTR_ERR(vdd_cpu_a53.regulator[0]) != -EPROBE_DEFER)
			dev_err(&pdev->dev,
				"Unable to get vdd_a53 regulator!!!\n");
		return PTR_ERR(vdd_cpu_a53.regulator[0]);
	}
	vdd_cpu_a53.use_max_uV = true;

	vdd_cpu_cci.regulator[0] = devm_regulator_get(&pdev->dev,
							"vdd_cci");
	if (IS_ERR(vdd_cpu_cci.regulator[0])) {
		if (PTR_ERR(vdd_cpu_cci.regulator[0]) != -EPROBE_DEFER)
			dev_err(&pdev->dev,
				"Unable to get vdd_cci regulator!!!\n");
		return PTR_ERR(vdd_cpu_cci.regulator[0]);
	}
	vdd_cpu_cci.use_max_uV = true;

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "c0-mux");
	if (!res) {
		dev_err(&pdev->dev, "missing c0-mux\n");
		return -EINVAL;
	}

	mux_bases[A53SS_MUX_C0] = devm_ioremap(&pdev->dev, res->start,
							resource_size(res));
	if (!mux_bases[A53SS_MUX_C0]) {
		dev_err(&pdev->dev, "ioremap failed for a53 (c0) mux\n");
		return -ENOMEM;
	}

	a53ssmux.clkr.regmap = devm_regmap_init_mmio(&pdev->dev,
					mux_bases[A53SS_MUX_C0],
					&mux_blocks_regmap_config);
	if (IS_ERR(a53ssmux.clkr.regmap)) {
		dev_err(&pdev->dev, "Cannot init regmap for a53 (c0) mux\n");
		return PTR_ERR(a53ssmux.clkr.regmap);
	}

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "c1-mux");
	if (!res) {
		dev_err(&pdev->dev, "missing c1-mux\n");
		return -EINVAL;
	}

	mux_bases[A53SS_MUX_C1] = devm_ioremap(&pdev->dev, res->start,
							resource_size(res));
	if (!mux_bases[A53SS_MUX_C1]) {
		dev_err(&pdev->dev, "ioremap failed for a72 (c1) mux\n");
		return -ENOMEM;
	}

	a72ssmux.clkr.regmap = devm_regmap_init_mmio(&pdev->dev,
					mux_bases[A53SS_MUX_C1],
					&mux_blocks_regmap_config);
	if (IS_ERR(a72ssmux.clkr.regmap)) {
		dev_err(&pdev->dev, "Cannot init regmap for a72 (c1) mux\n");
		return PTR_ERR(a72ssmux.clkr.regmap);
	}

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "cci-mux");
	if (!res) {
		dev_err(&pdev->dev, "missing cci-mux\n");
		return -EINVAL;
	}

	mux_bases[A53SS_MUX_CCI] = devm_ioremap(&pdev->dev, res->start,
							resource_size(res));
	if (!mux_bases[A53SS_MUX_CCI]) {
		dev_err(&pdev->dev, "ioremap failed for cci mux\n");
		return -ENOMEM;
	}

	ccissmux.clkr.regmap = devm_regmap_init_mmio(&pdev->dev,
					mux_bases[A53SS_MUX_CCI],
					&mux_blocks_regmap_config);
	if (IS_ERR(ccissmux.clkr.regmap)) {
		dev_err(&pdev->dev, "Cannot init regmap for cci mux\n");
		return PTR_ERR(ccissmux.clkr.regmap);
	}

	return 0;
}


/**
 * clock_panic_callback() - panic notification callback function.
 *              This function is invoked when a kernel panic occurs.
 * @nfb:        Notifier block pointer
 * @event:      Value passed unmodified to notifier function
 * @data:       Pointer passed unmodified to notifier function
 *
 * Return: NOTIFY_OK
 */
static int clock_panic_callback(struct notifier_block *nfb,
					unsigned long event, void *data)
{
	unsigned long rate;

	rate  = clk_hw_is_enabled(&a53ssmux.clkr.hw) ?
			clk_hw_get_rate(&a53ssmux.clkr.hw) : 0;
	pr_err("%s frequency: %10lu Hz\n",
		clk_hw_get_name(&a53ssmux.clkr.hw), rate);

	rate  = clk_hw_is_enabled(&a72ssmux.clkr.hw) ?
			clk_hw_get_rate(&a72ssmux.clkr.hw) : 0;
	pr_err("%s frequency: %10lu Hz\n",
		clk_hw_get_name(&a72ssmux.clkr.hw), rate);

	return NOTIFY_OK;
}

static struct notifier_block clock_panic_notifier = {
	.notifier_call = clock_panic_callback,
	.priority = 1,
};

static const unsigned long cci_boot_rate = 614400000;
static const unsigned long a53ss_boot_rate = 1401600000;
static const unsigned long a72ss_boot_rate = 1747200000;
static int clock_cpu_probe(struct platform_device *pdev)
{
	int i, speed_bin, version, rc, cpu, mux_id, clks_sz;
	char prop_name[] = "qcom,speedX-bin-vX-XXX";
	unsigned long a72rate, a53rate, ccirate, saferate;
	struct clk *xo_req = NULL;
	struct clk *safe_req = NULL;
	struct clk *aux_clk_req = NULL;
	struct clk *clk_tmp = NULL;
	struct clk_onecell_data *clk_onecell = NULL;

	/*
	 * Require the CXO_A and APCS AUX 3 clocks to be registered
	 * prior to the CPU driver: we are using AUX3 to get a safe
	 * rate for all of our clocks!
	 */
	xo_req = devm_clk_get(&pdev->dev, "cxo_a");
	if (IS_ERR(xo_req)) {
		dev_err(&pdev->dev, "The CXO_A clock cannot be found.\n");

		if (PTR_ERR(xo_req) != -EPROBE_DEFER)
			dev_err(&pdev->dev, "Critical error %ld. Bailing out.\n", PTR_ERR(xo_req));

		return PTR_ERR(xo_req);
	}

	aux_clk_req = devm_clk_get(&pdev->dev, "aux_clk_2");
	if (IS_ERR(safe_req)) {
		dev_err(&pdev->dev, "The AUX2 clock cannot be found.\n");

		if (PTR_ERR(aux_clk_req) != -EPROBE_DEFER)
			dev_err(&pdev->dev, "Critical error. Bailing out.\n");

		return PTR_ERR(aux_clk_req);
	}
	safe_req = devm_clk_get(&pdev->dev, "aux_clk_3");
	if (IS_ERR(safe_req)) {
		dev_err(&pdev->dev, "The AUX3 clock cannot be found.\n");

		if (PTR_ERR(safe_req) != -EPROBE_DEFER)
			dev_err(&pdev->dev, "Critical error. Bailing out.\n");

		return PTR_ERR(safe_req);
	}

	/* Initialize the data to avoid more calculations in determine_date */
	cpu_data.aux3_rates[AUX_FULL] = clk_get_rate(safe_req);
	cpu_data.aux3_rates[AUX_DIV2] = cpu_data.aux3_rates[AUX_FULL] / 2;
	cpu_data.aux2_rates[AUX_FULL] = clk_get_rate(aux_clk_req);
	cpu_data.aux2_rates[AUX_DIV2] = cpu_data.aux2_rates[AUX_FULL] / 2;

	get_speed_bin(pdev, &speed_bin, &version);

	rc = cpu_parse_devicetree(pdev);
	if (rc)
		return rc;

	clk_onecell = devm_kzalloc(&pdev->dev, sizeof(struct clk_onecell_data),
					GFP_KERNEL);
	if (!clk_onecell)
		return -ENOMEM;

	clks_sz = ARRAY_SIZE(clk_cpu_8976_hws) * sizeof(struct clk *);
	clk_onecell->clks = devm_kzalloc(&pdev->dev, clks_sz, GFP_KERNEL);
	if (!clk_onecell->clks) {
		devm_kfree(&pdev->dev, clk_onecell);
		return -ENOMEM;
	}

	clk_onecell->clk_num = ARRAY_SIZE(clk_cpu_8976_hws);

	for (mux_id = 0; mux_id < A53SS_MUX_NUM; mux_id++) {
		snprintf(prop_name, ARRAY_SIZE(prop_name),
					"qcom,speed%d-bin-v%d-%s",
					speed_bin, version, mux_names[mux_id]);
		rc = of_get_fmax_vdd_class(pdev, mux_id, prop_name);
		if (rc) {
			/* Fall back to most conservative PVS table */
			dev_err(&pdev->dev, "Unable to load voltage plan %s!\n",
								prop_name);
			pr_err("Error %d\n", rc);
			snprintf(prop_name, ARRAY_SIZE(prop_name),
				"qcom,speed0-bin-v0-%s", mux_names[mux_id]);
			rc = of_get_fmax_vdd_class(pdev, mux_id, prop_name);
			if (rc) {
				dev_err(&pdev->dev,
					"Unable to load safe voltage plan\n");
				return rc;
			}
			dev_info(&pdev->dev, "Safe voltage plan loaded.\n");
		}
	}

	/* Debug Mux */
	virt_bases[APCS0_DBG_BASE] = devm_ioremap(&pdev->dev, GLB_DIAG, SZ_8);
	if (!virt_bases[APCS0_DBG_BASE]) {
		dev_err(&pdev->dev, "Failed to ioremap GLB_DIAG registers\n");
		return -ENOMEM;
	}

	for (i = 0; i < ARRAY_SIZE(clk_cpu_8976_hws); i++) {
		clk_tmp = devm_clk_register(&pdev->dev, clk_cpu_8976_hws[i]);
		if (IS_ERR(clk_tmp)) {
			dev_err(&pdev->dev,
				"Cannot register HW clock at position %d\n",i);
			return PTR_ERR(clk_tmp);
		}
		clk_onecell->clks[i] = clk_tmp;
	}

	rc = of_clk_add_provider(pdev->dev.of_node, of_clk_src_onecell_get,
				 clk_onecell);
	if (rc) {
		dev_err(&pdev->dev, "Cannot register CPU clock provider.\n");

		if (clk_onecell)
			devm_kfree(&pdev->dev, clk_onecell->clks);
		devm_kfree(&pdev->dev, clk_onecell);

		return rc;
	}

	rc = clock_rcgwr_init(pdev);
	if (rc)
		dev_err(&pdev->dev, "Failed to init RCGwR\n");

	get_online_cpus();

	/* Set safe rate to reconfigure all the RCGs */
	saferate = clk_hw_get_rate(&sys_apcsaux_clk_3.hw);
	if (!saferate)
		dev_warn(&pdev->dev, "Safe rate is bad. Should we BUG()?\n");

	a53rate = clk_hw_get_rate(&a53ssmux.clkr.hw);
	rc = clk_set_rate(a53ssmux.clkr.hw.clk, saferate);
	if (rc)
		dev_err(&pdev->dev, "Can't set safe rate\n");

	a72rate = clk_hw_get_rate(&a72ssmux.clkr.hw);
	rc = clk_set_rate(a72ssmux.clkr.hw.clk, saferate);
	if (rc)
		dev_err(&pdev->dev, "Can't set safe rate\n");

	ccirate = clk_hw_get_rate(&ccissmux.clkr.hw);
	rc = clk_set_rate(ccissmux.clkr.hw.clk, saferate/2);
	if (rc)
		dev_err(&pdev->dev, "Can't set safe rate\n");

	ccirate = clk_get_rate(ccissmux.clkr.hw.clk);

	pr_debug("                     safe       a53       a72       cci\n");
	pr_debug("Boot    frequencies: %lu %lu %lu %lu\n",
		saferate, a53rate, a72rate, ccirate);

	populate_opp_table(pdev);

	rc = of_platform_populate(pdev->dev.of_node, NULL, NULL, &pdev->dev);
	if (rc)
		return rc;

	/* Register clock notifiers for safe frequency switching */
	rc = clk_notifier_register(a72ss_hf_pll.clkr.hw.clk,
						&a72ssmux.clk_nb);
	if (rc) {
		dev_err(&pdev->dev,
			"Failed to register A72 clock notifier: %d\n", rc);
		return rc;
	}

	rc = clk_notifier_register(a53ss_sr_pll.clkr.hw.clk,
						&a53ssmux.clk_nb);
	if (rc) {
		dev_err(&pdev->dev,
			"Failed to register A53 clock notifier: %d\n", rc);
		return rc;
	}

	rc = clk_notifier_register(cci_sr_pll.clkr.hw.clk,
						&ccissmux.clk_nb);
	if (rc) {
		dev_err(&pdev->dev,
			"Failed to register CCI clock notifier: %d\n", rc);
		return rc;
	}

	/* Assign cpumask to the CPU descriptors */
	for_each_possible_cpu(cpu) {
		if (cpu <= 3)
			cpumask_set_cpu(cpu, &cpu_data.a53ssmux_desc.cpumask);
		else
			cpumask_set_cpu(cpu, &cpu_data.a72ssmux_desc.cpumask);
	}

	/* Put a proxy vote for the PLLs until initial configuration ends */
	WARN(clk_prepare_enable(cci_sr_pll.clkr.hw.clk),
				"Unable to Turn on CCI PLL");
	WARN(clk_prepare_enable(a53ss_sr_pll.clkr.hw.clk),
				"Unable to Turn on A53 PLL");
	WARN(clk_prepare_enable(a72ss_hf_pll.clkr.hw.clk),
				"Unable to Turn on A72 PLL");

	pr_debug("Setting TURBO rate for CCI and A72\n");
	rc = clk_set_rate(ccissmux.clkr.hw.clk, cci_boot_rate);
	if (rc) {
		dev_err(&pdev->dev, "Unable to set boot rate on CCI, rc=%d\n",
			rc);
	}

	rc = clk_set_rate(a72ss_hf_pll.clkr.hw.clk, a72ss_boot_rate);
	if (rc) {
		dev_err(&pdev->dev, "Unable to set boot rate on CCI, rc=%d\n",
			rc);
	}

	rc = clk_set_rate(a72ssmux.clkr.hw.clk, a72ss_boot_rate);
	if (rc) {
		dev_err(&pdev->dev, "Unable to set boot rate on CCI, rc=%d\n",
			rc);
	}

	/* Raise the refcount of CPU clocks to avoid turning them off */
	for_each_online_cpu(cpu) {
		WARN(clk_prepare_enable(ccissmux.clkr.hw.clk),
				"Unable to Turn on CCI clock");
		WARN(clk_prepare_enable(a53ssmux.clkr.hw.clk),
				"Unable to Turn on A53 clock");
		WARN(clk_prepare_enable(a72ssmux.clkr.hw.clk),
				"Unable to Turn on A72 clock");
	}

	rc = clk_set_rate(a53ssmux.clkr.hw.clk, a53ss_boot_rate);
	if (rc) {
		dev_err(&pdev->dev, "Unable to set boot rate on A53, rc=%d\n",
			rc);
	}

	atomic_notifier_chain_register(&panic_notifier_list,
					&clock_panic_notifier);

	put_online_cpus();

	/* Clocks are configured. Now we can remove the proxy vote. */
	clk_disable_unprepare(cci_sr_pll.clkr.hw.clk);
	clk_disable_unprepare(a53ss_sr_pll.clkr.hw.clk);
	clk_disable_unprepare(a72ss_hf_pll.clkr.hw.clk);

	return 0;
}

static struct of_device_id clock_cpu_match_table[] = {
	{.compatible = "qcom,cpu-clock-8976"},
	{}
};

static struct platform_driver clock_cpu_driver = {
	.probe = clock_cpu_probe,
	.driver = {
		.name = "cpu-clock-8976",
		.of_match_table = clock_cpu_match_table,
		.owner = THIS_MODULE,
	},
};

static int __init clock_cpu_init(void)
{
	int ret = 0;

	ret = platform_driver_register(&clock_cpu_driver);

	return ret;
}
arch_initcall(clock_cpu_init);

#define APCS_ALIAS0_CMD_RCGR		0xb111050
#define APCS_ALIAS1_CMD_RCGR		0xb011050
#define APCS_ALIAS1_CFG_OFF		0x4
#define APCS_ALIAS1_CORE_CBCR_OFF	0x8
#define SRC_SEL				0x4
#define SRC_DIV				0x1
static void __init cpu_clock_early_sel(u32 alias)
{
	void __iomem  *base;
	int regval = 0, count;
	base = ioremap_nocache(alias, SZ_8);
	regval = readl_relaxed(base);

	/* Source GPLL0 and at the rate of GPLL0 */
	regval = (SRC_SEL << 8) | SRC_DIV;
	writel_relaxed(regval, base + APCS_ALIAS1_CFG_OFF);
	/* Make sure src sel and src div is set before update bit */
	mb();

	/* update bit */
	regval = readl_relaxed(base);
	regval |= BIT(0);
	writel_relaxed(regval, base);

	/* Wait for update to take effect */
	for (count = 500; count > 0; count--) {
		if (!(readl_relaxed(base)) & BIT(0))
			break;
		udelay(1);
	}
	if (!(readl_relaxed(base)) & BIT(0))
		panic("RCG configuration didn't update!\n");

	/* Enable the branch */
	regval = readl_relaxed(base + APCS_ALIAS1_CORE_CBCR_OFF);
	regval |= BIT(0);
	writel_relaxed(regval, base + APCS_ALIAS1_CORE_CBCR_OFF);
	/* Branch enable should be complete */
	mb();
	iounmap(base);
}

static int __init cpu_clock_early_init(void)
{
	struct device_node *ofnode = of_find_compatible_node(NULL, NULL,
							"qcom,cpu-clock-8976");

	if (!ofnode)
		return 0;

	cpu_clock_early_sel(APCS_ALIAS1_CMD_RCGR);
	pr_info("A72 Perf clocks configured with GPLL0 source\n");

	cpu_clock_early_sel(APCS_ALIAS0_CMD_RCGR);
	pr_info("A53 Power clocks configured with GPLL0 source\n");

	return 0;
}
early_initcall(cpu_clock_early_init);
