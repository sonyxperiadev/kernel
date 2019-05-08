/*
 * Linux mainline API clocks implementation for MSM8998
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

#include <linux/kernel.h>
#include <linux/bitops.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/regmap.h>
#include <linux/reset-controller.h>
#include <linux/clk.h>
#include <linux/clk/qcom.h>
#include <linux/mfd/syscon.h>

#include <dt-bindings/clock/qcom,gcc-msm8998.h>

#include "common.h"
#include "clk-debug.h"
#include "clk-regmap.h"
#include "clk-pll.h"
#include "clk-rcg.h"
#include "clk-branch.h"
#include "clk-alpha-pll.h"
#include "reset.h"
#include "vdd-level-8998.h"

#define F(f, s, h, m, n) { (f), (s), (2 * (h) - 1), (m), (n) }

static DEFINE_VDD_REGULATORS(vdd_dig, VDD_DIG_NUM, 1, vdd_corner);
static DEFINE_VDD_REGULATORS(vdd_dig_ao, VDD_DIG_NUM, 1, vdd_corner);

enum {
	P_CORE_BI_PLL_TEST_SE,
	P_GPLL0,
	P_GPLL0_EARLY_DIV,
	P_GPLL4,
	P_SLEEP_CLK,
	P_AUD_REF_CLK,
	P_XO,
};


static const struct parent_map gcc_parent_map_1[] = {
	{ P_XO, 0},
	{ P_GPLL0, 1 },
};

static const char * const gcc_parent_names_1[] = {
	"xo",
	"gpll0",
};

static const struct parent_map gcc_parent_map_1_ao[] = {
	{ P_XO, 0 },
	{ P_GPLL0, 1 },
	{ P_CORE_BI_PLL_TEST_SE, 7 },
};

static const char * const gcc_parent_names_1_ao[] = {
	"cxo_a",
	"gpll0_ao",
	"core_bi_pll_test_se",
};

static const struct parent_map gcc_parent_map_2[] = {
	{ P_XO, 0},
	{ P_GPLL0, 1 },
	{ P_GPLL0_EARLY_DIV, 6 },
};

static const char * const gcc_parent_names_2[] = {
	"xo",
	"gpll0",
	"gpll0_early_div",
};

static const struct parent_map gcc_parent_map_4[] = {
	{ P_XO, 0},
	{ P_GPLL0, 1 },
	{ P_GPLL4, 5 },
};

static const char * const gcc_parent_names_4[] = {
	"xo",
	"gpll0",
	"gpll4",
};

static const struct parent_map gcc_parent_map_5[] = {
	{ P_XO, 0 },
	{ P_GPLL0, 1 },
	{ P_AUD_REF_CLK, 2 },
};

static const char * const gcc_parent_names_5[] = {
	"xo",
	"gpll0",
	"aud_ref_clk",
};

static const struct parent_map gcc_parent_map_6[] = {
	{ P_XO, 0 },
	{ P_GPLL0, 1 },
	{ P_SLEEP_CLK, 5 },
	{ P_GPLL0_EARLY_DIV, 6 },
};

static const char * const gcc_parent_names_6[] = {
	"xo",
	"gpll0",
	"sleep_clk",
	"gpll0_early_div",
};

static const struct parent_map gcc_parent_map_7[] = {
	{ P_XO, 0 },
	{ P_SLEEP_CLK, 5 },
};

static const char * const gcc_parent_names_7[] = {
	"xo",
	"sleep_clk",
};

static struct clk_fixed_factor gcc_ce1_ahb_m_clk = {
	.hw.init = &(struct clk_init_data){
		.name = "gcc_ce1_ahb_m_clk",
		.ops = &clk_dummy_ops,
	},
};

static struct clk_fixed_factor gcc_ce1_axi_m_clk = {
	.hw.init = &(struct clk_init_data){
		.name = "gcc_ce1_axi_m_clk",
		.ops = &clk_dummy_ops,
	},
};

static struct clk_fixed_factor xo = {
	.mult = 1,
	.div = 1,
	.hw.init = &(struct clk_init_data){
		.name = "xo",
		.parent_names = (const char *[]){ "cxo" },
		.num_parents = 1,
		.ops = &clk_fixed_factor_ops,
	},
};

static struct clk_fixed_factor bi_tcxo = {
	.mult = 1,
	.div = 1,
	.hw.init = &(struct clk_init_data){
		.name = "bi_tcxo",
		.parent_names = (const char *[]){ "xo_board" },
		.num_parents = 1,
		.ops = &clk_fixed_factor_ops,
	},
};

static struct pll_vco fabia_vco[] = {
	{ 249600000, 2000000000, 0 },
	{ 125000000, 1000000000, 1 },
};

static unsigned int soft_vote_gpll0;

static struct clk_alpha_pll gpll0 = {
	.offset = 0x0,
	.regs = clk_alpha_pll_regs[CLK_ALPHA_PLL_TYPE_FABIA],
	.vco_table = fabia_vco,
	.num_vco = ARRAY_SIZE(fabia_vco),
	.soft_vote = &soft_vote_gpll0,
	.soft_vote_mask = PLL_SOFT_VOTE_PRIMARY,
	.flags = SUPPORTS_FSM_VOTE,
	.clkr = {
		.enable_reg = 0x52000,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "gpll0",
			.parent_names = (const char *[]){ "xo" },
			.num_parents = 1,
			.ops = &clk_alpha_pll_fabia_fixed_ops,
		}
	},
};

static struct clk_fixed_factor gpll0_early_div = {
	.mult = 1,
	.div = 2,
	.hw.init = &(struct clk_init_data){
		.name = "gpll0_early_div",
		.parent_names = (const char *[]){ "gpll0" },
		.num_parents = 1,
		.ops = &clk_fixed_factor_ops,
	},
};

static struct clk_alpha_pll gpll0_ao = {
	.offset = 0x00000,
	.regs = clk_alpha_pll_regs[CLK_ALPHA_PLL_TYPE_FABIA],
	.soft_vote = &soft_vote_gpll0,
	.soft_vote_mask = PLL_SOFT_VOTE_CPU,
	.flags = SUPPORTS_FSM_VOTE,
	.clkr = {
		.enable_reg = 0x52000,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "gpll0_ao",
			.parent_names = (const char *[]){ "cxo_a" },
			.num_parents = 1,
			.ops = &clk_alpha_pll_fabia_fixed_ops,
		},
	},
};

static struct clk_alpha_pll_postdiv gpll0_out_main = {
	.offset = 0x0,
	.regs = clk_alpha_pll_regs[CLK_ALPHA_PLL_TYPE_FABIA],
	.clkr.hw.init = &(struct clk_init_data){
		.name = "gpll0_out_main",
		.parent_names = (const char *[]){ "gpll0" },
		.num_parents = 1,
		.ops = &clk_alpha_fabia_pll_postdiv_ops,
	},
};

static struct clk_alpha_pll gpll1 = {
	.offset = 0x1000,
	.regs = clk_alpha_pll_regs[CLK_ALPHA_PLL_TYPE_FABIA],
	.vco_table = fabia_vco,
	.num_vco = ARRAY_SIZE(fabia_vco),
	.clkr = {
		.enable_reg = 0x52000,
		.enable_mask = BIT(1),
		.hw.init = &(struct clk_init_data){
			.name = "gpll1",
			.parent_names = (const char *[]){ "xo" },
			.num_parents = 1,
			.ops = &clk_alpha_pll_fabia_fixed_ops,
		}
	},
};

static struct clk_alpha_pll_postdiv gpll1_out_main = {
	.offset = 0x1000,
	.regs = clk_alpha_pll_regs[CLK_ALPHA_PLL_TYPE_FABIA],
	.clkr.hw.init = &(struct clk_init_data){
		.name = "gpll1_out_main",
		.parent_names = (const char *[]){ "gpll1" },
		.num_parents = 1,
		.ops = &clk_alpha_fabia_pll_postdiv_ops,
	},
};

static struct clk_alpha_pll gpll2 = {
	.offset = 0x2000,
	.regs = clk_alpha_pll_regs[CLK_ALPHA_PLL_TYPE_FABIA],
	.vco_table = fabia_vco,
	.num_vco = ARRAY_SIZE(fabia_vco),
	.clkr = {
		.enable_reg = 0x52000,
		.enable_mask = BIT(2),
		.hw.init = &(struct clk_init_data){
			.name = "gpll2",
			.parent_names = (const char *[]){ "xo" },
			.num_parents = 1,
			.ops = &clk_alpha_pll_fabia_fixed_ops,
		}
	},
};

static struct clk_alpha_pll_postdiv gpll2_out_main = {
	.offset = 0x2000,
	.regs = clk_alpha_pll_regs[CLK_ALPHA_PLL_TYPE_FABIA],
	.clkr.hw.init = &(struct clk_init_data){
		.name = "gpll2_out_main",
		.parent_names = (const char *[]){ "gpll2" },
		.num_parents = 1,
		.ops = &clk_alpha_fabia_pll_postdiv_ops,
	},
};

static struct clk_alpha_pll gpll3 = {
	.offset = 0x3000,
	.regs = clk_alpha_pll_regs[CLK_ALPHA_PLL_TYPE_FABIA],
	.vco_table = fabia_vco,
	.num_vco = ARRAY_SIZE(fabia_vco),
	.clkr = {
		.enable_reg = 0x52000,
		.enable_mask = BIT(3),
		.hw.init = &(struct clk_init_data){
			.name = "gpll3",
			.parent_names = (const char *[]){ "xo" },
			.num_parents = 1,
			.ops = &clk_alpha_pll_fabia_fixed_ops,
		}
	},
};

static struct clk_alpha_pll_postdiv gpll3_out_main = {
	.offset = 0x3000,
	.regs = clk_alpha_pll_regs[CLK_ALPHA_PLL_TYPE_FABIA],
	.clkr.hw.init = &(struct clk_init_data){
		.name = "gpll3_out_main",
		.parent_names = (const char *[]){ "gpll3" },
		.num_parents = 1,
		.ops = &clk_alpha_fabia_pll_postdiv_ops,
	},
};

static struct clk_alpha_pll gpll4 = {
	.offset = 0x77000,
	.regs = clk_alpha_pll_regs[CLK_ALPHA_PLL_TYPE_FABIA],
	.vco_table = fabia_vco,
	.num_vco = ARRAY_SIZE(fabia_vco),
	.clkr = {
		.enable_reg = 0x52000,
		.enable_mask = BIT(4),
		.hw.init = &(struct clk_init_data){
			.name = "gpll4",
			.parent_names = (const char *[]){ "xo" },
			.num_parents = 1,
			.ops = &clk_alpha_pll_fabia_fixed_ops,
			VDD_DIG_FMAX_MAP3(LOWER, 400000000, LOW, 800000000,
					NOMINAL, 1600000000),
		}
	},
};

static struct clk_alpha_pll_postdiv gpll4_out_main = {
	.offset = 0x77000,
	.regs = clk_alpha_pll_regs[CLK_ALPHA_PLL_TYPE_FABIA],
	.clkr.hw.init = &(struct clk_init_data){
		.name = "gpll4_out_main",
		.parent_names = (const char *[]){ "gpll4" },
		.num_parents = 1,
		.ops = &clk_alpha_fabia_pll_postdiv_ops,
	},
};

static struct clk_gate2 gcc_mmss_gpll0_clk = {
	.udelay = 500,
	.clkr = {
		.enable_reg = 0x5200c,
		.enable_mask = BIT(1),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_mmss_gpll0_clk",
			.parent_names = (const char *[]){
				"gpll0",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_gate2_ops,
		},
	},
};

static struct clk_gate2 gcc_mmss_gpll0_div_clk = {
	.udelay = 500,
	.clkr = {
		.enable_reg = 0x5200c,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_mmss_gpll0_div_clk",
			.parent_names = (const char *[]){
				"gpll0_early_div",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_gate2_ops,
		},
	},
};

static struct clk_gate2 gcc_gpu_gpll0_clk = {
	.udelay = 500,
	.clkr = {
		.enable_reg = 0x5200c,
		.enable_mask = BIT(4),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_gpu_gpll0_clk",
			.parent_names = (const char *[]){
				"gpll0",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_gate2_ops,
		},
	},
};

static struct clk_gate2 gcc_gpu_gpll0_div_clk = {
	.udelay = 500,
	.clkr = {
		.enable_reg = 0x5200c,
		.enable_mask = BIT(3),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_gpu_gpll0_div_clk",
			.parent_names = (const char *[]){
				"gpll0_early_div",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_gate2_ops,
		},
	},
};

/*
static struct pll_vote_clk gpll4 = {
	.en_reg = (void __iomem *)0x52000,
	.en_mask = BIT(4),
	.status_reg = (void __iomem *)0x77000,
	.status_mask = BIT(31),
	.clkr = {
		.rate = 384000000,
		(const char*[]) {
			"cxo_clk_src",
		},
		.num_parents = 1,
		.name = "gpll4",
		.parent_names = gcc_parent_names_,
		.num_parents = ARRAY_SIZE(gcc_parent_names_),
		.ops = &clk_ops_pll_vote,
		VDD_DIG_FMAX_MAP3(LOWER, 400000000, LOW, 800000000,
					NOMINAL, 1600000000),
	},
};
*/

static struct freq_tbl ftbl_usb30_master_clk_src[] = {
	F(  19200000,	P_XO,       1,    0,     0),
	F( 60000000,	P_GPLL0,    10,    0,     0),
	F( 120000000,	P_GPLL0,    5,    0,     0),
	F( 133333333,	P_GPLL0,  4.5,    0,     0),
	F( 150000000,	P_GPLL0,    4,    0,     0),
	{ }
};

static struct clk_rcg2 usb30_master_clk_src = {
	.cmd_rcgr = 0x0F014,
	.mnd_width = 8,
	.hid_width = 5,
	.parent_map = gcc_parent_map_1,
	.freq_tbl = ftbl_usb30_master_clk_src,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "usb30_master_clk_src",
		.parent_names = gcc_parent_names_1,
		.num_parents = ARRAY_SIZE(gcc_parent_names_1),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP4(LOWER, 66670000, LOW, 133333333,
				NOMINAL, 200000000, HIGH, 240000000),
	},
};

static struct freq_tbl ftbl_pcie_aux_clk_src[] = {
	F(  19200000,	P_XO,       1,    0,     0),
	{ }
};

static struct clk_rcg2 pcie_aux_clk_src = {
	.cmd_rcgr = 0x6C000,
	.mnd_width = 8,
	.hid_width = 5,
	.parent_map = gcc_parent_map_7,
	.freq_tbl = ftbl_pcie_aux_clk_src,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "pcie_aux_clk_src",
		.parent_names = gcc_parent_names_7,
		.num_parents = ARRAY_SIZE(gcc_parent_names_7),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP2(LOWER, 9600000, LOW, 19200000),
	},
};

static struct freq_tbl ftbl_ufs_axi_clk_src[] = {
	F(  50000000,	P_GPLL0,   12,    0,     0),
	F( 100000000,	P_GPLL0,    6,    0,     0),
	F( 200000000,	P_GPLL0,    3,    0,     0),
	F( 240000000,	P_GPLL0,  2.5,    0,     0),
	{ }
};

static struct clk_rcg2 ufs_axi_clk_src = {
	.cmd_rcgr = 0x75018,
	.mnd_width = 8,
	.hid_width = 5,
	.parent_map = gcc_parent_map_2,
	.freq_tbl = ftbl_ufs_axi_clk_src,
	.flags = FORCE_ENABLE_RCG,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "ufs_axi_clk_src",
		.parent_names = gcc_parent_names_2,
		.num_parents = ARRAY_SIZE(gcc_parent_names_2),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP4(LOWER, 50000000, LOW, 100000000,
				NOMINAL, 200000000, HIGH, 240000000),
	},
};

static struct freq_tbl ftbl_blsp_i2c_apps_clk_src[] = {
	F(  19200000,	P_XO,       1,    0,     0),
	F(  50000000,	P_GPLL0,   12,    0,     0),
	{ }
};

static struct clk_rcg2 blsp1_qup1_i2c_apps_clk_src = {
	.cmd_rcgr = 0x19020,
	.hid_width = 5,
	.parent_map = gcc_parent_map_1,
	.freq_tbl = ftbl_blsp_i2c_apps_clk_src,
	.enable_safe_config = true,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "blsp1_qup1_i2c_apps_clk_src",
		.parent_names = gcc_parent_names_1,
		.num_parents = ARRAY_SIZE(gcc_parent_names_1),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP2(LOWER, 19200000, LOW, 50000000),
	},
};

static struct freq_tbl ftbl_blsp_qup_spi_apps_clk_src[] = {
	F(    960000,	P_XO,      10,    1,     2),
	F(   4800000,	P_XO,       4,    0,     0),
	F(   9600000,	P_XO,       2,    0,     0),
	F(  15000000, P_GPLL0_EARLY_DIV,   5,    1,     4),
	F(  19200000,	P_XO,       1,    0,     0),
	F(  25000000,	P_GPLL0,   12,    1,     2),
	F(  50000000,	P_GPLL0,   12,    0,     0),
	{ }
};

static struct clk_rcg2 blsp1_qup1_spi_apps_clk_src = {
	.cmd_rcgr = 0x1900C,
	.mnd_width = 8,
	.hid_width = 5,
	.parent_map = gcc_parent_map_2,
	.freq_tbl = ftbl_blsp_qup_spi_apps_clk_src,
	.enable_safe_config = true,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "blsp1_qup1_spi_apps_clk_src",
		.parent_names = gcc_parent_names_2,
		.num_parents = ARRAY_SIZE(gcc_parent_names_2),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP3(LOWER, 19200000, LOW, 25000000,
					NOMINAL, 50000000),
	},
};

static struct clk_rcg2 blsp1_qup2_i2c_apps_clk_src = {
	.cmd_rcgr = 0x1B020,
	.hid_width = 5,
	.parent_map = gcc_parent_map_1,
	.freq_tbl = ftbl_blsp_i2c_apps_clk_src,
	.enable_safe_config = true,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "blsp1_qup2_i2c_apps_clk_src",
		.parent_names = gcc_parent_names_1,
		.num_parents = ARRAY_SIZE(gcc_parent_names_1),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP2(LOWER, 19200000, LOW, 50000000),
	},
};

static struct clk_rcg2 blsp1_qup2_spi_apps_clk_src = {
	.cmd_rcgr = 0x1B00C,
	.mnd_width = 8,
	.hid_width = 5,
	.parent_map = gcc_parent_map_2,
	.freq_tbl = ftbl_blsp_qup_spi_apps_clk_src,
	.enable_safe_config = true,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "blsp1_qup2_spi_apps_clk_src",
		.parent_names = gcc_parent_names_2,
		.num_parents = ARRAY_SIZE(gcc_parent_names_2),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP3(LOWER, 19200000, LOW, 25000000,
					NOMINAL, 50000000),
	},
};

static struct clk_rcg2 blsp1_qup3_i2c_apps_clk_src = {
	.cmd_rcgr = 0x1D020,
	.hid_width = 5,
	.parent_map = gcc_parent_map_1,
	.freq_tbl = ftbl_blsp_i2c_apps_clk_src,
	.enable_safe_config = true,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "blsp1_qup3_i2c_apps_clk_src",
		.parent_names = gcc_parent_names_1,
		.num_parents = ARRAY_SIZE(gcc_parent_names_1),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP2(LOWER, 19200000, LOW, 50000000),
	},
};

static struct clk_rcg2 blsp1_qup3_spi_apps_clk_src = {
	.cmd_rcgr = 0x1D00C,
	.mnd_width = 8,
	.hid_width = 5,
	.parent_map = gcc_parent_map_2,
	.freq_tbl = ftbl_blsp_qup_spi_apps_clk_src,
	.enable_safe_config = true,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "blsp1_qup3_spi_apps_clk_src",
		.parent_names = gcc_parent_names_2,
		.num_parents = ARRAY_SIZE(gcc_parent_names_2),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP3(LOWER, 19200000, LOW, 25000000,
					NOMINAL, 50000000),
	},
};

static struct clk_rcg2 blsp1_qup4_i2c_apps_clk_src = {
	.cmd_rcgr = 0x1F020,
	.hid_width = 5,
	.parent_map = gcc_parent_map_1,
	.freq_tbl = ftbl_blsp_i2c_apps_clk_src,
	.enable_safe_config = true,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "blsp1_qup4_i2c_apps_clk_src",
		.parent_names = gcc_parent_names_1,
		.num_parents = ARRAY_SIZE(gcc_parent_names_1),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP2(LOWER, 19200000, LOW, 50000000),
	},
};

static struct clk_rcg2 blsp1_qup4_spi_apps_clk_src = {
	.cmd_rcgr = 0x1F00C,
	.mnd_width = 8,
	.hid_width = 5,
	.parent_map = gcc_parent_map_2,
	.freq_tbl = ftbl_blsp_qup_spi_apps_clk_src,
	.enable_safe_config = true,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "blsp1_qup4_spi_apps_clk_src",
		.parent_names = gcc_parent_names_2,
		.num_parents = ARRAY_SIZE(gcc_parent_names_2),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP3(LOWER, 19200000, LOW, 25000000,
					NOMINAL, 50000000),
	},
};

static struct clk_rcg2 blsp1_qup5_i2c_apps_clk_src = {
	.cmd_rcgr = 0x21020,
	.hid_width = 5,
	.parent_map = gcc_parent_map_1,
	.freq_tbl = ftbl_blsp_i2c_apps_clk_src,
	.enable_safe_config = true,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "blsp1_qup5_i2c_apps_clk_src",
		.parent_names = gcc_parent_names_1,
		.num_parents = ARRAY_SIZE(gcc_parent_names_1),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP2(LOWER, 19200000, LOW, 50000000),
	},
};

static struct clk_rcg2 blsp1_qup5_spi_apps_clk_src = {
	.cmd_rcgr = 0x2100C,
	.mnd_width = 8,
	.hid_width = 5,
	.parent_map = gcc_parent_map_2,
	.freq_tbl = ftbl_blsp_qup_spi_apps_clk_src,
	.enable_safe_config = true,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "blsp1_qup5_spi_apps_clk_src",
		.parent_names = gcc_parent_names_2,
		.num_parents = ARRAY_SIZE(gcc_parent_names_2),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP3(LOWER, 19200000, LOW, 25000000,
					NOMINAL, 50000000),
	},
};

static struct clk_rcg2 blsp1_qup6_i2c_apps_clk_src = {
	.cmd_rcgr = 0x23020,
	.hid_width = 5,
	.parent_map = gcc_parent_map_1,
	.freq_tbl = ftbl_blsp_i2c_apps_clk_src,
	.enable_safe_config = true,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "blsp1_qup6_i2c_apps_clk_src",
		.parent_names = gcc_parent_names_1,
		.num_parents = ARRAY_SIZE(gcc_parent_names_1),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP2(LOWER, 19200000, LOW, 50000000),
	},
};

static struct clk_rcg2 blsp1_qup6_spi_apps_clk_src = {
	.cmd_rcgr = 0x2300C,
	.mnd_width = 8,
	.hid_width = 5,
	.parent_map = gcc_parent_map_2,
	.freq_tbl = ftbl_blsp_qup_spi_apps_clk_src,
	.enable_safe_config = true,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "blsp1_qup6_spi_apps_clk_src",
		.parent_names = gcc_parent_names_2,
		.num_parents = ARRAY_SIZE(gcc_parent_names_2),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP3(LOWER, 19200000, LOW, 25000000,
					NOMINAL, 50000000),
	},
};

static struct freq_tbl ftbl_blsp_uart_apps_clk_src[] = {
	F(   3686400,   P_GPLL0,    1,   96, 15625),
	F(   7372800,   P_GPLL0,    1,  192, 15625),
	F(  14745600,   P_GPLL0,    1,  384, 15625),
	F(  16000000,   P_GPLL0,    5,    2,    15),
	F(  19200000,	P_XO,       1,    0,     0),
	F(  24000000,	P_GPLL0,    5,    1,     5),
	F(  32000000,	P_GPLL0,    1,    4,    75),
	F(  40000000,	P_GPLL0,   15,    0,     0),
	F(  46400000,	P_GPLL0,    1,   29,   375),
	F(  48000000,	P_GPLL0, 12.5,    0,     0),
	F(  51200000,	P_GPLL0,    1,   32,   375),
	F(  56000000,	P_GPLL0,    1,    7,    75),
	F(  58982400,	P_GPLL0,    1, 1536, 15625),
	F(  60000000,	P_GPLL0,   10,    0,     0),
	F(  63157895,	P_GPLL0,  9.5,    0,     0),
	{ }
};

static struct clk_rcg2 blsp1_uart1_apps_clk_src = {
	.cmd_rcgr = 0x1A00C,
	.mnd_width = 16,
	.hid_width = 5,
	.parent_map = gcc_parent_map_2,
	.freq_tbl = ftbl_blsp_uart_apps_clk_src,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "blsp1_uart1_apps_clk_src",
		.parent_names = gcc_parent_names_2,
		.num_parents = ARRAY_SIZE(gcc_parent_names_2),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP3(LOWER, 19200000, LOW, 31580000,
					NOMINAL, 63160000),
	},
};

static struct clk_rcg2 blsp1_uart2_apps_clk_src = {
	.cmd_rcgr = 0x1C00C,
	.mnd_width = 16,
	.hid_width = 5,
	.parent_map = gcc_parent_map_2,
	.freq_tbl = ftbl_blsp_uart_apps_clk_src,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "blsp1_uart2_apps_clk_src",
		.parent_names = gcc_parent_names_2,
		.num_parents = ARRAY_SIZE(gcc_parent_names_2),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP3(LOWER, 19200000, LOW, 31580000,
					NOMINAL, 63160000),
	},
};

static struct clk_rcg2 blsp1_uart3_apps_clk_src = {
	.cmd_rcgr = 0x1E00C,
	.mnd_width = 16,
	.hid_width = 5,
	.parent_map = gcc_parent_map_2,
	.freq_tbl = ftbl_blsp_uart_apps_clk_src,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "blsp1_uart3_apps_clk_src",
		.parent_names = gcc_parent_names_2,
		.num_parents = ARRAY_SIZE(gcc_parent_names_2),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP3(LOWER, 19200000, LOW, 31580000,
					NOMINAL, 63160000),
	},
};

static struct clk_rcg2 blsp2_qup1_i2c_apps_clk_src = {
	.cmd_rcgr = 0x26020,
	.hid_width = 5,
	.parent_map = gcc_parent_map_1,
	.freq_tbl = ftbl_blsp_i2c_apps_clk_src,
	.enable_safe_config = true,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "blsp2_qup1_i2c_apps_clk_src",
		.parent_names = gcc_parent_names_1,
		.num_parents = ARRAY_SIZE(gcc_parent_names_1),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP2(LOWER, 19200000, LOW, 50000000),
	},
};

static struct clk_rcg2 blsp2_qup1_spi_apps_clk_src = {
	.cmd_rcgr = 0x2600C,
	.mnd_width = 8,
	.hid_width = 5,
	.parent_map = gcc_parent_map_2,
	.freq_tbl = ftbl_blsp_qup_spi_apps_clk_src,
	.enable_safe_config = true,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "blsp2_qup1_spi_apps_clk_src",
		.parent_names = gcc_parent_names_2,
		.num_parents = ARRAY_SIZE(gcc_parent_names_2),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP3(LOWER, 19200000, LOW, 25000000,
					NOMINAL, 50000000),
	},
};

static struct clk_rcg2 blsp2_qup2_i2c_apps_clk_src = {
	.cmd_rcgr = 0x28020,
	.hid_width = 5,
	.parent_map = gcc_parent_map_1,
	.freq_tbl = ftbl_blsp_i2c_apps_clk_src,
	.enable_safe_config = true,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "blsp2_qup2_i2c_apps_clk_src",
		.parent_names = gcc_parent_names_1,
		.num_parents = ARRAY_SIZE(gcc_parent_names_1),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP2(LOWER, 19200000, LOW, 50000000),
	},
};

static struct clk_rcg2 blsp2_qup2_spi_apps_clk_src = {
	.cmd_rcgr = 0x2800C,
	.mnd_width = 8,
	.hid_width = 5,
	.parent_map = gcc_parent_map_2,
	.freq_tbl = ftbl_blsp_qup_spi_apps_clk_src,
	.enable_safe_config = true,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "blsp2_qup2_spi_apps_clk_src",
		.parent_names = gcc_parent_names_2,
		.num_parents = ARRAY_SIZE(gcc_parent_names_2),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP3(LOWER, 19200000, LOW, 25000000,
					NOMINAL, 50000000),
	},
};

static struct clk_rcg2 blsp2_qup3_i2c_apps_clk_src = {
	.cmd_rcgr = 0x2A020,
	.hid_width = 5,
	.parent_map = gcc_parent_map_1,
	.freq_tbl = ftbl_blsp_i2c_apps_clk_src,
	.enable_safe_config = true,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "blsp2_qup3_i2c_apps_clk_src",
		.parent_names = gcc_parent_names_1,
		.num_parents = ARRAY_SIZE(gcc_parent_names_1),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP2(LOWER, 19200000, LOW, 50000000),
	},
};

static struct clk_rcg2 blsp2_qup3_spi_apps_clk_src = {
	.cmd_rcgr = 0x2A00C,
	.mnd_width = 8,
	.hid_width = 5,
	.parent_map = gcc_parent_map_2,
	.freq_tbl = ftbl_blsp_qup_spi_apps_clk_src,
	.enable_safe_config = true,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "blsp2_qup3_spi_apps_clk_src",
		.parent_names = gcc_parent_names_2,
		.num_parents = ARRAY_SIZE(gcc_parent_names_2),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP3(LOWER, 19200000, LOW, 25000000,
					NOMINAL, 50000000),
	},
};

static struct clk_rcg2 blsp2_qup4_i2c_apps_clk_src = {
	.cmd_rcgr = 0x2C020,
	.hid_width = 5,
	.parent_map = gcc_parent_map_1,
	.freq_tbl = ftbl_blsp_i2c_apps_clk_src,
	.enable_safe_config = true,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "blsp2_qup4_i2c_apps_clk_src",
		.parent_names = gcc_parent_names_1,
		.num_parents = ARRAY_SIZE(gcc_parent_names_1),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP2(LOWER, 19200000, LOW, 50000000),
	},
};

static struct clk_rcg2 blsp2_qup4_spi_apps_clk_src = {
	.cmd_rcgr = 0x2C00C,
	.mnd_width = 8,
	.hid_width = 5,
	.parent_map = gcc_parent_map_2,
	.freq_tbl = ftbl_blsp_qup_spi_apps_clk_src,
	.enable_safe_config = true,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "blsp2_qup4_spi_apps_clk_src",
		.parent_names = gcc_parent_names_2,
		.num_parents = ARRAY_SIZE(gcc_parent_names_2),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP3(LOWER, 19200000, LOW, 25000000,
					NOMINAL, 50000000),
	},
};

static struct clk_rcg2 blsp2_qup5_i2c_apps_clk_src = {
	.cmd_rcgr = 0x2E020,
	.hid_width = 5,
	.parent_map = gcc_parent_map_1,
	.freq_tbl = ftbl_blsp_i2c_apps_clk_src,
	.enable_safe_config = true,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "blsp2_qup5_i2c_apps_clk_src",
		.parent_names = gcc_parent_names_1,
		.num_parents = ARRAY_SIZE(gcc_parent_names_1),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP2(LOWER, 19200000, LOW, 50000000),
	},
};

static struct clk_rcg2 blsp2_qup5_spi_apps_clk_src = {
	.cmd_rcgr = 0x2E00C,
	.mnd_width = 8,
	.hid_width = 5,
	.parent_map = gcc_parent_map_2,
	.freq_tbl = ftbl_blsp_qup_spi_apps_clk_src,
	.enable_safe_config = true,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "blsp2_qup5_spi_apps_clk_src",
		.parent_names = gcc_parent_names_2,
		.num_parents = ARRAY_SIZE(gcc_parent_names_2),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP3(LOWER, 19200000, LOW, 25000000,
					NOMINAL, 50000000),
	},
};

static struct clk_rcg2 blsp2_qup6_i2c_apps_clk_src = {
	.cmd_rcgr = 0x30020,
	.hid_width = 5,
	.parent_map = gcc_parent_map_1,
	.freq_tbl = ftbl_blsp_i2c_apps_clk_src,
	.enable_safe_config = true,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "blsp2_qup6_i2c_apps_clk_src",
		.parent_names = gcc_parent_names_1,
		.num_parents = ARRAY_SIZE(gcc_parent_names_1),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP2(LOWER, 19200000, LOW, 50000000),
	},
};

static struct clk_rcg2 blsp2_qup6_spi_apps_clk_src = {
	.cmd_rcgr = 0x3000C,
	.mnd_width = 8,
	.hid_width = 5,
	.parent_map = gcc_parent_map_2,
	.freq_tbl = ftbl_blsp_qup_spi_apps_clk_src,
	.enable_safe_config = true,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "blsp2_qup6_spi_apps_clk_src",
		.parent_names = gcc_parent_names_2,
		.num_parents = ARRAY_SIZE(gcc_parent_names_2),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP3(LOWER, 19200000, LOW, 25000000,
					NOMINAL, 50000000),
	},
};

static struct clk_rcg2 blsp2_uart1_apps_clk_src = {
	.cmd_rcgr = 0x2700C,
	.mnd_width = 16,
	.hid_width = 5,
	.parent_map = gcc_parent_map_2,
	.freq_tbl = ftbl_blsp_uart_apps_clk_src,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "blsp2_uart1_apps_clk_src",
		.parent_names = gcc_parent_names_2,
		.num_parents = ARRAY_SIZE(gcc_parent_names_2),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP3(LOWER, 19200000, LOW, 31580000,
					NOMINAL, 63160000),
	},
};

static struct clk_rcg2 blsp2_uart2_apps_clk_src = {
	.cmd_rcgr = 0x2900C,
	.mnd_width = 16,
	.hid_width = 5,
	.parent_map = gcc_parent_map_2,
	.freq_tbl = ftbl_blsp_uart_apps_clk_src,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "blsp2_uart2_apps_clk_src",
		.parent_names = gcc_parent_names_2,
		.num_parents = ARRAY_SIZE(gcc_parent_names_2),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP3(LOWER, 19200000, LOW, 31580000,
					NOMINAL, 63160000),
	},
};

static struct clk_rcg2 blsp2_uart3_apps_clk_src = {
	.cmd_rcgr = 0x2B00C,
	.mnd_width = 16,
	.hid_width = 5,
	.parent_map = gcc_parent_map_2,
	.freq_tbl = ftbl_blsp_uart_apps_clk_src,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "blsp2_uart3_apps_clk_src",
		.parent_names = gcc_parent_names_2,
		.num_parents = ARRAY_SIZE(gcc_parent_names_2),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP3(LOWER, 19200000,  LOW, 31580000,
					NOMINAL, 63160000),
	},
};

static struct freq_tbl ftbl_gp_clk_src[] = {
	F(  19200000,	P_XO,       1,    0,     0),
	F( 100000000,	P_GPLL0,    6,    0,     0),
	F( 200000000,	P_GPLL0,    3,    0,     0),
	{ }
};

static struct clk_rcg2 gp1_clk_src = {
	.cmd_rcgr = 0x64004,
	.mnd_width = 8,
	.hid_width = 5,
	.parent_map = gcc_parent_map_6,
	.freq_tbl = ftbl_gp_clk_src,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "gp1_clk_src",
		.parent_names = gcc_parent_names_6,
		.num_parents = ARRAY_SIZE(gcc_parent_names_6),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP3(LOWER, 50000000, LOW, 100000000,
					NOMINAL, 200000000),
	},
};

static struct clk_rcg2 gp2_clk_src = {
	.cmd_rcgr = 0x65004,
	.mnd_width = 8,
	.hid_width = 5,
	.parent_map = gcc_parent_map_6,
	.freq_tbl = ftbl_gp_clk_src,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "gp2_clk_src",
		.parent_names = gcc_parent_names_6,
		.num_parents = ARRAY_SIZE(gcc_parent_names_6),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP3(LOWER, 50000000, LOW, 100000000,
					NOMINAL, 200000000),
	},
};

static struct clk_rcg2 gp3_clk_src = {
	.cmd_rcgr = 0x66004,
	.mnd_width = 8,
	.hid_width = 5,
	.parent_map = gcc_parent_map_6,
	.freq_tbl = ftbl_gp_clk_src,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "gp3_clk_src",
		.parent_names = gcc_parent_names_6,
		.num_parents = ARRAY_SIZE(gcc_parent_names_6),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP3(LOWER, 50000000, LOW, 100000000,
					NOMINAL, 200000000),
	},
};

static struct freq_tbl ftbl_hmss_rbcpr_clk_src[] = {
	F(  19200000,     P_XO,    1,    0,     0),
	{ }
};

static struct clk_rcg2 hmss_rbcpr_clk_src = {
	.cmd_rcgr = 0x48044,
	.hid_width = 5,
	.parent_map = gcc_parent_map_1_ao,
	.freq_tbl = ftbl_hmss_rbcpr_clk_src,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "hmss_rbcpr_clk_src",
		.parent_names = gcc_parent_names_1_ao,
		.num_parents = ARRAY_SIZE(gcc_parent_names_1_ao),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP1_AO(LOWER, 19200000),
	},
};

static struct freq_tbl ftbl_pdm2_clk_src[] = {
	F(  60000000,	P_GPLL0,   10,    0,     0),
	{ }
};

static struct clk_rcg2 pdm2_clk_src = {
	.cmd_rcgr = 0x33010,
	.hid_width = 5,
	.parent_map = gcc_parent_map_1,
	.freq_tbl = ftbl_pdm2_clk_src,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "pdm2_clk_src",
		.parent_names = gcc_parent_names_1,
		.num_parents = ARRAY_SIZE(gcc_parent_names_1),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP2(LOWER, 19200000, LOW, 60000000),
	},
};

static struct freq_tbl ftbl_sdcc2_apps_clk_src[] = {
	F(    144000,	P_XO,      16,    3,    25),
	F(    400000,	P_XO,      12,    1,     4),
	F(  20000000,	P_GPLL0,   15,    1,     2),
	F(  25000000,	P_GPLL0,   12,    1,     2),
	F(  50000000,	P_GPLL0,   12,    0,     0),
	F( 100000000,	P_GPLL0,    6,    0,     0),
	F( 200000000,	P_GPLL0,    3,    0,     0),
	{ }
};

static struct clk_rcg2 sdcc2_apps_clk_src = {
	.cmd_rcgr = 0x14010,
	.mnd_width = 8,
	.hid_width = 5,
	.parent_map = gcc_parent_map_1,
	.freq_tbl = ftbl_sdcc2_apps_clk_src,
	.enable_safe_config = true,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "sdcc2_apps_clk_src",
		.parent_names = gcc_parent_names_1,
		.num_parents = ARRAY_SIZE(gcc_parent_names_1),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP3(LOWER, 19200000, LOW, 100000000,
					NOMINAL, 200000000),
	},
};

static struct freq_tbl ftbl_sdcc4_apps_clk_src[] = {
	F(    144000,	P_XO,      16,    3,    25),
	F(    400000,	P_XO,      12,    1,     4),
	F(  20000000,	P_GPLL0,   15,    1,     2),
	F(  25000000,	P_GPLL0,   12,    1,     2),
	F(  50000000,	P_GPLL0,   12,    0,     0),
	F( 100000000,	P_GPLL0,    6,    0,     0),
	{ }
};

static struct clk_rcg2 sdcc4_apps_clk_src = {
	.cmd_rcgr = 0x16010,
	.mnd_width = 8,
	.hid_width = 5,
	.parent_map = gcc_parent_map_1,
	.freq_tbl = ftbl_sdcc4_apps_clk_src,
	.enable_safe_config = true,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "sdcc4_apps_clk_src",
		.parent_names = gcc_parent_names_1,
		.num_parents = ARRAY_SIZE(gcc_parent_names_1),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP3(LOWER, 19200000, LOW, 50000000,
					NOMINAL, 100000000),
	},
};

static struct freq_tbl ftbl_tsif_ref_clk_src[] = {
	F(    105495,	P_XO,       1,    1,   182),
	{ }
};

static struct clk_rcg2 tsif_ref_clk_src = {
	.cmd_rcgr = 0x36010,
	.mnd_width = 8,
	.hid_width = 5,
	.parent_map = gcc_parent_map_5,
	.freq_tbl = ftbl_tsif_ref_clk_src,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "tsif_ref_clk_src",
		.parent_names = gcc_parent_names_5,
		.num_parents = ARRAY_SIZE(gcc_parent_names_5),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP1(LOWER, 105500),
	},
};

static struct freq_tbl ftbl_ufs_ice_core_clk_src[] = {
	F(     75000000,	P_GPLL0,    8,    0,   0),
	F(    150000000,	P_GPLL0,    4,    0,   0),
	F(    300000000,	P_GPLL0,    2,    0,   0),
	{ }
};

static struct clk_rcg2 ufs_ice_core_clk_src = {
	.cmd_rcgr = 0x76010,
	.hid_width = 5,
	.parent_map = gcc_parent_map_1,
	.freq_tbl = ftbl_ufs_ice_core_clk_src,
	.flags = FORCE_ENABLE_RCG,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "ufs_ice_core_clk_src",
		.parent_names = gcc_parent_names_1,
		.num_parents = ARRAY_SIZE(gcc_parent_names_1),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP3(LOWER, 75000000, LOW, 150000000,
					NOMINAL, 300000000),
	},
};

static struct freq_tbl ftbl_ufs_phy_aux_clk_src[] = {
	F(  19200000,	P_XO,       1,    0,     0),
	{ }
};

static struct clk_rcg2 ufs_phy_aux_clk_src = {
	.cmd_rcgr = 0x76044,
	.hid_width = 5,
	.parent_map = gcc_parent_map_2,
	.freq_tbl = ftbl_ufs_phy_aux_clk_src,
	.flags = FORCE_ENABLE_RCG,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "ufs_phy_aux_clk_src",
		.parent_names = gcc_parent_names_2,
		.num_parents = ARRAY_SIZE(gcc_parent_names_2),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP1(LOWER, 19200000),
	},
};

static struct freq_tbl ftbl_ufs_unipro_core_clk_src[] = {
	F(  37500000,	P_GPLL0,   16,    0,     0),
	F(  75000000,	P_GPLL0,    8,    0,     0),
	F( 150000000,	P_GPLL0,    4,    0,     0),
	{ }
};

static struct clk_rcg2 ufs_unipro_core_clk_src = {
	.cmd_rcgr = 0x76028,
	.hid_width = 5,
	.parent_map = gcc_parent_map_1,
	.freq_tbl = ftbl_ufs_unipro_core_clk_src,
	.flags = FORCE_ENABLE_RCG,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "ufs_unipro_core_clk_src",
		.parent_names = gcc_parent_names_1,
		.num_parents = ARRAY_SIZE(gcc_parent_names_1),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP3(LOWER, 37500000, LOW, 75000000,
					NOMINAL, 150000000),
	},
};

static struct freq_tbl ftbl_usb30_mock_utmi_clk_src[] = {
	F(  19200000,	P_XO,       1,    0,     0),
	{ }
};

static struct clk_rcg2 usb30_mock_utmi_clk_src = {
	.cmd_rcgr = 0x0F028,
	.hid_width = 5,
	.parent_map = gcc_parent_map_2,
	.freq_tbl = ftbl_usb30_mock_utmi_clk_src,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "usb30_mock_utmi_clk_src",
		.parent_names = gcc_parent_names_2,
		.num_parents = ARRAY_SIZE(gcc_parent_names_2),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP2(LOWER, 40000000, LOW, 60000000),
	},
};

static struct freq_tbl ftbl_usb3_phy_aux_clk_src[] = {
	F(   1200000,	P_XO,      16,    0,     0),
	{ }
};

static struct clk_rcg2 usb3_phy_aux_clk_src = {
	.cmd_rcgr = 0x5000C,
	.hid_width = 5,
	.parent_map = gcc_parent_map_7,
	.freq_tbl = ftbl_usb3_phy_aux_clk_src,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "usb3_phy_aux_clk_src",
		.parent_names = gcc_parent_names_7,
		.num_parents = ARRAY_SIZE(gcc_parent_names_7),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP1(LOWER, 19200000),
	},
};

static struct freq_tbl ftbl_hmss_gpll0_clk_src[] = {
	F( 300000000,   P_GPLL0,    2,    0,     0),
	F( 600000000,   P_GPLL0,    1,    0,     0),
	{ }
};

static struct clk_rcg2 hmss_gpll0_clk_src = {
	.cmd_rcgr = 0x4805C,
	.hid_width = 5,
	.parent_map = gcc_parent_map_1_ao,
	.freq_tbl = ftbl_hmss_gpll0_clk_src,
	/* .enable_safe_config = true, -- Do we need this here? */
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "hmss_gpll0_clk_src",
		.parent_names = gcc_parent_names_1_ao,
		.num_parents = ARRAY_SIZE(gcc_parent_names_1_ao),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP1_AO(LOWER, 600000000),
	},
};

static struct clk_branch gcc_hmss_rbcpr_clk = {
	.halt_reg = 0x48008,
	.clkr = {
		.enable_reg = 0x48008,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_hmss_rbcpr_clk",
			.parent_names = (const char*[]) {
				"hmss_rbcpr_clk_src"
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};


static struct freq_tbl ftbl_qspi_ref_clk_src[] = {
	F(  75000000,	P_GPLL0,    8,    0,     0),
	F( 150000000,	P_GPLL0,    4,    0,     0),
	F( 256000000,	P_GPLL4,  1.5,    0,     0),
	F( 300000000,	P_GPLL0,    2,    0,     0),
	{ }
};

static struct clk_rcg2 qspi_ref_clk_src = {
	.cmd_rcgr = 0x9000C,
	.hid_width = 5,
	.parent_map = gcc_parent_map_2,
	.freq_tbl = ftbl_qspi_ref_clk_src,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "qspi_ref_clk_src",
		.parent_names = gcc_parent_names_2,
		.num_parents = ARRAY_SIZE(gcc_parent_names_2),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP3(LOWER, 40000000, LOW, 160400000,
							NOMINAL, 320800000),
	},
};

static struct clk_branch gcc_hdmi_clkref_clk = {
	.halt_reg = 0x88000,
	.clkr = {
		.enable_reg = 0x88000,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_hdmi_clkref_clk",
			.flags = CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_pcie_clkref_clk = {
	.halt_reg = 0x8800C,
	.clkr = {
		.enable_reg = 0x8800C,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_pcie_clkref_clk",
			.flags = CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_rx1_usb2_clkref_clk = {
	.halt_reg = 0x88014,
	.clkr = {
		.enable_reg = 0x88014,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_rx1_usb2_clkref_clk",
			.flags = CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_ufs_clkref_clk = {
	.halt_reg = 0x88004,
	.clkr = {
		.enable_reg = 0x88004,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_ufs_clkref_clk",
			.flags = CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_usb3_clkref_clk = {
	.halt_reg = 0x88008,
	.clkr = {
		.enable_reg = 0x88008,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_usb3_clkref_clk",
			.flags = CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_gate2 gpll0_out_msscc = {
	.udelay = 1,
	.clkr = {
		.enable_reg = 0x5200C,
		.enable_mask = BIT(2),
		.hw.init = &(struct clk_init_data) {
			.name = "gpll0_out_msscc",
			.parent_names = (const char *[]){
				"gpll0",
			},
			.num_parents = 1,
			.flags = CLK_ENABLE_HAND_OFF, // | CLK_SET_RATE_PARENT,
			.ops = &clk_gate2_ops,
		},
	},
};


static struct clk_branch gcc_aggre1_ufs_axi_clk = {
	.halt_reg = 0x82028,
	.halt_check = BRANCH_HALT,
	.hwcg_reg = 0x82028,
	.hwcg_bit = 1,
	.clkr = {
		.enable_reg = 0x82028,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_aggre1_ufs_axi_clk",
			.parent_names = (const char*[]) {
				"ufs_axi_clk_src"
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT | CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_aggre1_ufs_axi_hw_ctl_clk = {
	.halt_reg = 0x82028,
	.clkr = {
		.enable_reg = 0x82028,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_aggre1_ufs_axi_hw_ctl_clk",
			.parent_names = (const char*[]) {
				"gcc_aggre1_ufs_axi_clk"
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_hw_ctl_ops,
		},
	},
};

static struct clk_branch gcc_aggre1_usb3_axi_clk = {
	.halt_reg = 0x82024,
	.clkr = {
		.enable_reg = 0x82024,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_aggre1_usb3_axi_clk",
			.parent_names = (const char*[]) {
				"usb30_master_clk_src"
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT | CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_bimc_mss_q6_axi_clk = {
	.halt_reg = 0x4401C,
	.clkr = {
		.enable_reg = 0x4401C,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_bimc_mss_q6_axi_clk",
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_blsp1_ahb_clk = {
	.halt_reg = 0x17004,
	.halt_check = BRANCH_HALT_VOTED,
	.clkr = {
		.enable_reg = 0x52004,
		.enable_mask = BIT(17),
		.hw.init = &(struct clk_init_data){
				.name = "gcc_blsp1_ahb_clk",
				.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_blsp1_qup1_i2c_apps_clk = {
	.halt_reg = 0x19008,
	.clkr = {
		.enable_reg = 0x19008,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_blsp1_qup1_i2c_apps_clk",
			.parent_names = (const char*[]) {
				"blsp1_qup1_i2c_apps_clk_src"
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT | CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_blsp1_qup1_spi_apps_clk = {
	.halt_reg = 0x19004,
	.clkr = {
		.enable_reg = 0x19004,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_blsp1_qup1_spi_apps_clk",
			.parent_names = (const char*[]) {
				"blsp1_qup1_spi_apps_clk_src"
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT | CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_blsp1_qup2_i2c_apps_clk = {
	.halt_reg = 0x1B008,
	.clkr = {
		.enable_reg = 0x1B008,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_blsp1_qup2_i2c_apps_clk",
			.parent_names = (const char*[]) {
				"blsp1_qup2_i2c_apps_clk_src"
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT | CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_blsp1_qup2_spi_apps_clk = {
	.halt_reg = 0x1B004,
	.clkr = {
		.enable_reg = 0x1B004,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_blsp1_qup2_spi_apps_clk",
			.parent_names = (const char*[]) {
				"blsp1_qup2_spi_apps_clk_src"
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT | CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_blsp1_qup3_i2c_apps_clk = {
	.halt_reg = 0x1D008,
	.clkr = {
		.enable_reg = 0x1D008,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_blsp1_qup3_i2c_apps_clk",
			.parent_names = (const char*[]) {
				"blsp1_qup3_i2c_apps_clk_src"
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT | CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_blsp1_qup3_spi_apps_clk = {
	.halt_reg = 0x1D004,
	.clkr = {
		.enable_reg = 0x1D004,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_blsp1_qup3_spi_apps_clk",
			.parent_names = (const char*[]) {
				"blsp1_qup3_spi_apps_clk_src"
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT | CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_blsp1_qup4_i2c_apps_clk = {
	.halt_reg = 0x1F008,
	.clkr = {
		.enable_reg = 0x1F008,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_blsp1_qup4_i2c_apps_clk",
			.parent_names = (const char*[]) {
				"blsp1_qup4_i2c_apps_clk_src"
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT | CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_blsp1_qup4_spi_apps_clk = {
	.halt_reg = 0x1F004,
	.clkr = {
		.enable_reg = 0x1F004,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_blsp1_qup4_spi_apps_clk",
			.parent_names = (const char*[]) {
				"blsp1_qup4_spi_apps_clk_src"
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT | CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_blsp1_qup5_i2c_apps_clk = {
	.halt_reg = 0x21008,
	.clkr = {
		.enable_reg = 0x21008,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_blsp1_qup5_i2c_apps_clk",
			.parent_names = (const char*[]) {
				"blsp1_qup5_i2c_apps_clk_src"
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT | CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_blsp1_qup5_spi_apps_clk = {
	.halt_reg = 0x21004,
	.clkr = {
		.enable_reg = 0x21004,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_blsp1_qup5_spi_apps_clk",
			.parent_names = (const char*[]) {
				"blsp1_qup5_spi_apps_clk_src"
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT | CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_blsp1_qup6_i2c_apps_clk = {
	.halt_reg = 0x23008,
	.clkr = {
		.enable_reg = 0x23008,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_blsp1_qup6_i2c_apps_clk",
			.parent_names = (const char*[]) {
				"blsp1_qup6_i2c_apps_clk_src"
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT | CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_blsp1_qup6_spi_apps_clk = {
	.halt_reg = 0x23004,
	.clkr = {
		.enable_reg = 0x23004,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_blsp1_qup6_spi_apps_clk",
			.parent_names = (const char*[]) {
				"blsp1_qup6_spi_apps_clk_src"
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT | CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_blsp1_uart1_apps_clk = {
	.halt_reg = 0x1A004,
	.clkr = {
		.enable_reg = 0x1A004,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_blsp1_uart1_apps_clk",
			.parent_names = (const char*[]) {
				"blsp1_uart1_apps_clk_src"
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT | CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_blsp1_uart2_apps_clk = {
	.halt_reg = 0x1C004,
	.clkr = {
		.enable_reg = 0x1C004,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_blsp1_uart2_apps_clk",
			.parent_names = (const char*[]) {
				"blsp1_uart2_apps_clk_src"
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT | CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_blsp1_uart3_apps_clk = {
	.halt_reg = 0x1E004,
	.clkr = {
		.enable_reg = 0x1E004,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_blsp1_uart3_apps_clk",
			.parent_names = (const char*[]) {
				"blsp1_uart3_apps_clk_src"
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT | CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_blsp2_ahb_clk = {
	.halt_reg = 0x25004,
	.halt_check = BRANCH_HALT_VOTED,
	.clkr = {
		.enable_reg = 0x52004,
		.enable_mask = BIT(15),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_blsp2_ahb_clk",
			.flags = CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_blsp2_qup1_i2c_apps_clk = {
	.halt_reg = 0x26008,
	.clkr = {
		.enable_reg = 0x26008,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_blsp2_qup1_i2c_apps_clk",
			.parent_names = (const char*[]) {
				"blsp2_qup1_i2c_apps_clk_src"
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT | CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_blsp2_qup1_spi_apps_clk = {
	.halt_reg = 0x26004,
	.clkr = {
		.enable_reg = 0x26004,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_blsp2_qup1_spi_apps_clk",
			.parent_names = (const char*[]) {
				"blsp2_qup1_spi_apps_clk_src"
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT | CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_blsp2_qup2_i2c_apps_clk = {
	.halt_reg = 0x28008,
	.clkr = {
		.enable_reg = 0x28008,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_blsp2_qup2_i2c_apps_clk",
			.parent_names = (const char*[]) {
				"blsp2_qup2_i2c_apps_clk_src"
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT | CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_blsp2_qup2_spi_apps_clk = {
	.halt_reg = 0x28004,
	.clkr = {
		.enable_reg = 0x28004,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_blsp2_qup2_spi_apps_clk",
			.parent_names = (const char*[]) {
				"blsp2_qup2_spi_apps_clk_src"
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT | CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_blsp2_qup3_i2c_apps_clk = {
	.halt_reg = 0x2A008,
	.clkr = {
		.enable_reg = 0x2A008,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_blsp2_qup3_i2c_apps_clk",
			.parent_names = (const char*[]) {
				"blsp2_qup3_i2c_apps_clk_src"
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT | CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_blsp2_qup3_spi_apps_clk = {
	.halt_reg = 0x2A004,
	.clkr = {
		.enable_reg = 0x2A004,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_blsp2_qup3_spi_apps_clk",
			.parent_names = (const char*[]) {
				"blsp2_qup3_spi_apps_clk_src"
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT | CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_blsp2_qup4_i2c_apps_clk = {
	.halt_reg = 0x2C008,
	.clkr = {
		.enable_reg = 0x2C008,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_blsp2_qup4_i2c_apps_clk",
			.parent_names = (const char*[]) {
				"blsp2_qup4_i2c_apps_clk_src"
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT | CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_blsp2_qup4_spi_apps_clk = {
	.halt_reg = 0x2C004,
	.clkr = {
		.enable_reg = 0x2C004,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_blsp2_qup4_spi_apps_clk",
			.parent_names = (const char*[]) {
				"blsp2_qup4_spi_apps_clk_src"
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT | CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_blsp2_qup5_i2c_apps_clk = {
	.halt_reg = 0x2E008,
	.clkr = {
		.enable_reg = 0x2E008,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_blsp2_qup5_i2c_apps_clk",
			.parent_names = (const char*[]) {
				"blsp2_qup5_i2c_apps_clk_src"
			},
			.num_parents = 1,
			.flags = CLK_ENABLE_HAND_OFF | CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_blsp2_qup5_spi_apps_clk = {
	.halt_reg = 0x2E004,
	.clkr = {
		.enable_reg = 0x2E004,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_blsp2_qup5_spi_apps_clk",
			.parent_names = (const char*[]) {
				"blsp2_qup5_spi_apps_clk_src"
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT | CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_blsp2_qup6_i2c_apps_clk = {
	.halt_reg = 0x30008,
	.clkr = {
		.enable_reg = 0x30008,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_blsp2_qup6_i2c_apps_clk",
			.parent_names = (const char*[]) {
				"blsp2_qup6_i2c_apps_clk_src"
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT | CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_blsp2_qup6_spi_apps_clk = {
	.halt_reg = 0x30004,
	.clkr = {
		.enable_reg = 0x30004,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_blsp2_qup6_spi_apps_clk",
			.parent_names = (const char*[]) {
				"blsp2_qup6_spi_apps_clk_src"
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT | CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_blsp2_uart1_apps_clk = {
	.halt_reg = 0x27004,
	.clkr = {
		.enable_reg = 0x27004,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_blsp2_uart1_apps_clk",
			.parent_names = (const char*[]) {
				"blsp2_uart1_apps_clk_src"
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT | CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_blsp2_uart2_apps_clk = {
	.halt_reg = 0x29004,
	.clkr = {
		.enable_reg = 0x29004,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_blsp2_uart2_apps_clk",
			.parent_names = (const char*[]) {
				"blsp2_uart2_apps_clk_src"
			},
			.num_parents = 1,
			.flags = CLK_ENABLE_HAND_OFF | CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_blsp2_uart3_apps_clk = {
	.halt_reg = 0x2B004,
	.clkr = {
		.enable_reg = 0x2B004,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_blsp2_uart3_apps_clk",
			.parent_names = (const char*[]) {
				"blsp2_uart3_apps_clk_src"
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT | CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_boot_rom_ahb_clk = {
	.halt_reg = 0x38004,
	.halt_check = BRANCH_HALT_VOTED,
	.clkr = {
		.enable_reg = 0x52004,
		.enable_mask = BIT(10),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_boot_rom_ahb_clk",
			.flags = CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_cfg_noc_usb3_axi_clk = {
	.halt_reg = 0x05018,
	.clkr = {
		.enable_reg = 0x05018,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_cfg_noc_usb3_axi_clk",
			.parent_names = (const char*[]) {
				"usb30_master_clk_src"
			},
			.num_parents = 1,
			.flags = CLK_ENABLE_HAND_OFF,
			//.flags = CLK_SET_RATE_PARENT, //HANDOFF?
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_bimc_gfx_clk = {
	.halt_reg = 0x46040,
	.clkr = {
		.enable_reg = 0x46040,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_bimc_gfx_clk",
			.flags = CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_gp1_clk = {
	.halt_reg = 0x64000,
	.clkr = {
		.enable_reg = 0x64000,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_gp1_clk",
			.parent_names = (const char*[]) {
				"gp1_clk_src"
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT | CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_gp2_clk = {
	.halt_reg = 0x65000,
	.clkr = {
		.enable_reg = 0x65000,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_gp2_clk",
			.parent_names = (const char*[]) {
				"gp2_clk_src"
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT | CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_gp3_clk = {
	.halt_reg = 0x66000,
	.clkr = {
		.enable_reg = 0x66000,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_gp3_clk",
			.parent_names = (const char*[]) {
				"gp3_clk_src"
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT | CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_gpu_bimc_gfx_clk = {
	.halt_reg = 0x71010,
	.halt_check = BRANCH_VOTED,
	.clkr = {
		.enable_reg = 0x71010,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_gpu_bimc_gfx_clk",
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_gpu_cfg_ahb_clk = {
	.halt_reg = 0x71004,
	.halt_check = BRANCH_VOTED,
	.hwcg_reg = 0x71004,
	.hwcg_bit = 1,
	.clkr = {
		.enable_reg = 0x71004,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_gpu_cfg_ahb_clk",
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_gpu_iref_clk = {
	.halt_reg = 0x88010,
	.clkr = {
		.enable_reg = 0x88010,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_gpu_iref_clk",
			.flags = CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_hmss_dvm_bus_clk = {
	.halt_reg = 0x4808C,
	.clkr = {
		.enable_reg = 0x4808C,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_hmss_dvm_bus_clk",
			.flags = CLK_IGNORE_UNUSED,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_mmss_noc_cfg_ahb_clk = {
	.halt_reg = 0x09004,
	.clkr = {
		.enable_reg = 0x09004,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_mmss_noc_cfg_ahb_clk",
			.flags = CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_mmss_sys_noc_axi_clk = {
	.halt_reg = 0x09000,
	.clkr = {
		.enable_reg = 0x09000,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_mmss_sys_noc_axi_clk",
			.flags = CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_pcie_0_aux_clk = {
	.halt_reg = 0x6B014,
	.clkr = {
		.enable_reg = 0x6B014,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_pcie_0_aux_clk",
			.parent_names = (const char*[]) {
				"pcie_aux_clk_src"
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_pcie_0_cfg_ahb_clk = {
	.halt_reg = 0x6B010,
	.clkr = {
		.enable_reg = 0x6B010,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_pcie_0_cfg_ahb_clk",
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_pcie_0_mstr_axi_clk = {
	.halt_reg = 0x6B00C,
	.clkr = {
		.enable_reg = 0x6B00C,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_pcie_0_mstr_axi_clk",
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_pcie_0_pipe_clk = {
	.halt_reg = 0x6B018,
	.halt_check = BRANCH_HALT_DELAY,
	.clkr = {
		.enable_reg = 0x6B018,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_pcie_0_pipe_clk",
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_pcie_0_slv_axi_clk = {
	.halt_reg = 0x6B008,
	.clkr = {
		.enable_reg = 0x6B008,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_pcie_0_slv_axi_clk",
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_pcie_phy_aux_clk = {
	.halt_reg = 0x6F004,
	.clkr = {
		.enable_reg = 0x6F004,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_pcie_phy_aux_clk",
			.parent_names = (const char*[]) {
				"pcie_aux_clk_src"
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_pdm2_clk = {
	.halt_reg = 0x3300C,
	.clkr = {
		.enable_reg = 0x3300C,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_pdm2_clk",
			.parent_names = (const char*[]) {
				"pdm2_clk_src"
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT | CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_pdm_ahb_clk = {
	.halt_reg = 0x33004,
	.halt_check = BRANCH_HALT,
	.hwcg_reg = 0x33004,
	.hwcg_bit = 1,
	.clkr = {
		.enable_reg = 0x33004,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_pdm_ahb_clk",
			.flags = CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_prng_ahb_clk = {
	.halt_reg = 0x34004,
	.halt_check = BRANCH_HALT_VOTED,
	.hwcg_reg = 0x34004,
	.hwcg_bit = 1,
	.clkr = {
		.enable_reg = 0x52004,
		.enable_mask = BIT(13),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_prng_ahb_clk",
			.flags = CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_sdcc2_ahb_clk = {
	.halt_reg = 0x14008,
	.clkr = {
		.enable_reg = 0x14008,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_sdcc2_ahb_clk",
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_sdcc2_apps_clk = {
	.halt_reg = 0x14004,
	.clkr = {
		.enable_reg = 0x14004,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_sdcc2_apps_clk",
			.parent_names = (const char*[]) {
				"sdcc2_apps_clk_src"
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT | CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_sdcc4_ahb_clk = {
	.halt_reg = 0x16008,
	.clkr = {
		.enable_reg = 0x16008,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_sdcc4_ahb_clk",
			.flags = CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_sdcc4_apps_clk = {
	.halt_reg = 0x16004,
	.clkr = {
		.enable_reg = 0x16004,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_sdcc4_apps_clk",
			.parent_names = (const char*[]) {
				"sdcc4_apps_clk_src"
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT | CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_tsif_ahb_clk = {
	.halt_reg = 0x36004,
	.clkr = {
		.enable_reg = 0x36004,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_tsif_ahb_clk",
			.flags = CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_tsif_ref_clk = {
	.halt_reg = 0x36008,
	.clkr = {
		.enable_reg = 0x36008,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_tsif_ref_clk",
			.parent_names = (const char*[]) {
				"tsif_ref_clk_src"
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT | CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_ufs_ahb_clk = {
	.halt_reg = 0x7500C,
	.halt_check = BRANCH_HALT,
	.hwcg_reg = 0x7500C,
	.hwcg_bit = 1,
	.clkr = {
		.enable_reg = 0x7500C,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_ufs_ahb_clk",
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_ufs_axi_clk = {
	.halt_reg = 0x75008,
	.halt_check = BRANCH_HALT,
	.hwcg_reg = 0x75008,
	.hwcg_bit = 1,
	.clkr = {
		.enable_reg = 0x75008,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_ufs_axi_clk",
			.parent_names = (const char*[]) {
				"ufs_axi_clk_src"
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_ufs_axi_hw_ctl_clk = {
	.halt_reg = 0x75008,
	.clkr = {
		.enable_reg = 0x75008,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_ufs_axi_hw_ctl_clk",
			.parent_names = (const char*[]) {
				"gcc_ufs_axi_clk"
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_hw_ctl_ops,
		},
	},
};

static struct clk_branch gcc_ufs_ice_core_clk = {
	.halt_reg = 0x7600C,
	.halt_check = BRANCH_HALT,
	.hwcg_reg = 0x7600C,
	.hwcg_bit = 1,
	.clkr = {
		.enable_reg = 0x7600C,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_ufs_ice_core_clk",
			.parent_names = (const char*[]) {
				"ufs_ice_core_clk_src"
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_ufs_ice_core_hw_ctl_clk = {
	.halt_reg = 0x7600C,
	.clkr = {
		.enable_reg = 0x7600C,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_ufs_ice_core_hw_ctl_clk",
			.parent_names = (const char*[]) {
				"gcc_ufs_ice_core_clk"
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_hw_ctl_ops,
		},
	},
};

static struct clk_branch gcc_ufs_phy_aux_clk = {
	.halt_reg = 0x76040,
	.halt_check = BRANCH_HALT,
	.hwcg_reg = 0x76040,
	.hwcg_bit = 1,
	.clkr = {
		.enable_reg = 0x76040,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_ufs_phy_aux_clk",
			.parent_names = (const char*[]) {
				"ufs_phy_aux_clk_src"
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_ufs_phy_aux_hw_ctl_clk = {
	.halt_reg = 0x76040,
	.clkr = {
		.enable_reg = 0x76040,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_ufs_phy_aux_hw_ctl_clk",
			.parent_names = (const char*[]) {
				"gcc_ufs_phy_aux_clk"
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_hw_ctl_ops,
		},
	},
};

static struct clk_gate2 gcc_ufs_rx_symbol_0_clk = {
	.udelay = 500,
	.clkr = {
		.enable_reg = 0x75014,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_ufs_rx_symbol_0_clk",
			.ops = &clk_gate2_ops,
		},
	},
};

static struct clk_gate2 gcc_ufs_rx_symbol_1_clk = {
	.udelay = 500,
	.clkr = {
		.enable_reg = 0x7605C,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_ufs_rx_symbol_1_clk",
			.ops = &clk_gate2_ops,
		},
	},
};

static struct clk_gate2 gcc_ufs_tx_symbol_0_clk = {
	.udelay = 500,
	.clkr = {
		.enable_reg = 0x75010,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_ufs_tx_symbol_0_clk",
			.ops = &clk_gate2_ops,
		},
	},
};

static struct clk_branch gcc_ufs_unipro_core_clk = {
	.halt_reg = 0x76008,
	.halt_check = BRANCH_HALT,
	.hwcg_reg = 0x76008,
	.hwcg_bit = 1,
	.clkr = {
		.enable_reg = 0x76008,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_ufs_unipro_core_clk",
			.parent_names = (const char*[]) {
				"ufs_unipro_core_clk_src"
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_ufs_unipro_core_hw_ctl_clk = {
	.halt_reg = 0x76008,
	.clkr = {
		.enable_reg = 0x76008,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_ufs_unipro_core_hw_ctl_clk",
			.parent_names = (const char*[]) {
				"gcc_ufs_unipro_core_clk"
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_hw_ctl_ops,
		},
	},
};

static struct clk_branch gcc_usb30_master_clk = {
	.halt_reg = 0x0F008,
	.clkr = {
		.enable_reg = 0x0F008,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_usb30_master_clk",
			/*TODO: Should we depend on gcc_cfg_noc_usb3_axi_clk?*/
			.parent_names = (const char*[]) {
				"usb30_master_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_ENABLE_HAND_OFF | CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
		// wtf? .depends = &gcc_cfg_noc_usb3_axi_clk.c,
	},
};

static struct clk_branch gcc_usb30_mock_utmi_clk = {
	.halt_reg = 0x0F010,
	.clkr = {
		.enable_reg = 0x0F010,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_usb30_mock_utmi_clk",
			.parent_names = (const char*[]) {
				"usb30_mock_utmi_clk_src"
			},
			.num_parents = 1,
			.flags = CLK_ENABLE_HAND_OFF | CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_usb30_sleep_clk = {
	.halt_reg = 0x0F00C,
	.clkr = {
		.enable_reg = 0x0F00C,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_usb30_sleep_clk",
			.flags = CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_usb3_phy_aux_clk = {
	.halt_reg = 0x50000,
	.clkr = {
		.enable_reg = 0x50000,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_usb3_phy_aux_clk",
			.parent_names = (const char*[]) {
				"usb3_phy_aux_clk_src"
			},
			.num_parents = 1,
			.flags = CLK_ENABLE_HAND_OFF | CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

/*
static struct clk_gate2 gcc_usb3_phy_pipe_clk = {
	.udelay = 50,
	.clkr = {
		.enable_reg = 0x50004,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
				.name = "gcc_usb3_phy_pipe_clk",
				.ops = &clk_gate2_ops,
		},
	},
};
*/

static struct clk_branch gcc_usb3_phy_pipe_clk = {
	.halt_reg = 0x50004,
	.halt_check = BRANCH_HALT_DELAY,
	.clkr = {
		.enable_reg = 0x50004,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_usb3_phy_pipe_clk",
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_usb_phy_cfg_ahb2phy_clk = {
	.halt_reg = 0x6a004,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x6a004,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_usb_phy_cfg_ahb2phy_clk",
			.ops = &clk_branch2_ops,
		},
	},
};


static struct clk_branch gcc_mss_cfg_ahb_clk = {
	.halt_reg = 0x8A000,
	.halt_check = BRANCH_HALT,
	.hwcg_reg = 0x8a000,
	.hwcg_bit = 1,
	.clkr = {
		.enable_reg = 0x8A000,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_mss_cfg_ahb_clk",
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_mss_q6_bimc_axi_clk = {
	.halt_reg = 0x8A040,
	.clkr = {
		.enable_reg = 0x8A040,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_mss_q6_bimc_axi_clk",
			//.flags = CLK_IS_CRITICAL, //.always_on = true,
			.flags = CLK_ENABLE_HAND_OFF | CLK_IGNORE_UNUSED,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_mss_mnoc_bimc_axi_clk = {
	.halt_reg = 0x8A004,
	.halt_check = BRANCH_HALT,
	.hwcg_reg = 0x8A004,
	.hwcg_bit = 1,
	.clkr = {
		.enable_reg = 0x8A004,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_mss_mnoc_bimc_axi_clk",
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_mss_snoc_axi_clk = {
	.halt_reg = 0x8A03C,
	.clkr = {
		.enable_reg = 0x8A03C,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_mss_snoc_axi_clk",
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_dcc_ahb_clk = {
	.halt_reg = 0x84004,
	.clkr = {
		.enable_reg = 0x84004,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_dcc_ahb_clk",
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch hlos1_vote_lpass_core_smmu_clk = {
	.halt_reg = 0x7D010,
	.clkr = {
		.enable_reg = 0x7D010,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "hlos1_vote_lpass_core_smmu_clk",
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch hlos1_vote_lpass_adsp_smmu_clk = {
	.halt_reg = 0x7D014,
	.clkr = {
		.enable_reg = 0x7D014,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "hlos1_vote_lpass_adsp_smmu_clk",
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_qspi_ahb_clk = {
	.halt_reg = 0x90004,
	.clkr = {
		.enable_reg = 0x90004,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_qspi_ahb_clk",
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_qspi_ref_clk = {
	.halt_reg = 0x90008,
	.clkr = {
		.enable_reg = 0x90008,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gcc_qspi_ref_clk",
			.parent_names = (const char*[]) {
				"qspi_ref_clk_src"
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct measure_clk_data debug_mux_priv = {
	.xo_div4_cbcr = 0x43008,
	.ctl_reg = 0x62004,
	.status_reg = 0x62008,
};
/*
	MUX_REC_SRC_LIST(
		&gpu_gcc_debug_clk.c,
		&gfx_gcc_debug_clk.c,
		&debug_mmss_clk.c,
		&debug_cpu_clk.c,
	),
*/

static const char *const debug_mux_parent_names[] = {
	"gpu_gcc_debug_clk",
	"gfx_gcc_debug_clk",
	"debug_mmss_clk",
	"debug_cpu_clk",
	"snoc_clk",
	"cnoc_clk",
	"bimc_clk",
	"gcc_mmss_sys_noc_axi_clk",
	"gcc_mmss_noc_cfg_ahb_clk",
	"gcc_usb30_master_clk",
	"gcc_usb30_sleep_clk",
	"gcc_usb30_mock_utmi_clk",
	"gcc_usb3_phy_aux_clk",
	"gcc_usb3_phy_pipe_clk",
	"gcc_sdcc2_apps_clk",
	"gcc_sdcc2_ahb_clk",
	"gcc_sdcc4_apps_clk",
	"gcc_sdcc4_ahb_clk",
	"gcc_blsp1_ahb_clk",
	"gcc_blsp1_qup1_spi_apps_clk",
	"gcc_blsp1_qup1_i2c_apps_clk",
	"gcc_blsp1_uart1_apps_clk",
	"gcc_blsp1_qup2_spi_apps_clk",
	"gcc_blsp1_qup2_i2c_apps_clk",
	"gcc_blsp1_uart2_apps_clk",
	"gcc_blsp1_qup3_spi_apps_clk",
	"gcc_blsp1_qup3_i2c_apps_clk",
	"gcc_blsp1_uart3_apps_clk",
	"gcc_blsp1_qup4_spi_apps_clk",
	"gcc_blsp1_qup4_i2c_apps_clk",
	"gcc_blsp1_qup5_spi_apps_clk",
	"gcc_blsp1_qup5_i2c_apps_clk",
	"gcc_blsp1_qup6_spi_apps_clk",
	"gcc_blsp1_qup6_i2c_apps_clk",
	"gcc_blsp2_ahb_clk",
	"gcc_blsp2_qup1_spi_apps_clk",
	"gcc_blsp2_qup1_i2c_apps_clk",
	"gcc_blsp2_uart1_apps_clk",
	"gcc_blsp2_qup2_spi_apps_clk",
	"gcc_blsp2_qup2_i2c_apps_clk",
	"gcc_blsp2_uart2_apps_clk",
	"gcc_blsp2_qup3_spi_apps_clk",
	"gcc_blsp2_qup3_i2c_apps_clk",
	"gcc_blsp2_uart3_apps_clk",
	"gcc_blsp2_qup4_spi_apps_clk",
	"gcc_blsp2_qup4_i2c_apps_clk",
	"gcc_blsp2_qup5_spi_apps_clk",
	"gcc_blsp2_qup5_i2c_apps_clk",
	"gcc_blsp2_qup6_spi_apps_clk",
	"gcc_blsp2_qup6_i2c_apps_clk",
	"gcc_pdm_ahb_clk",
	"gcc_pdm2_clk",
	"gcc_prng_ahb_clk",
	"gcc_tsif_ahb_clk",
	"gcc_tsif_ref_clk",
	"gcc_boot_rom_ahb_clk",
	"ce1_clk",
	"gcc_ce1_axi_m_clk",
	"gcc_ce1_ahb_m_clk",
	"measure_only_bimc_hmss_axi_clk",
	"gcc_bimc_gfx_clk",
	"gcc_hmss_rbcpr_clk",
	"gcc_gp1_clk",
	"gcc_gp2_clk",
	"gcc_gp3_clk",
	"gcc_pcie_0_slv_axi_clk",
	"gcc_pcie_0_mstr_axi_clk",
	"gcc_pcie_0_cfg_ahb_clk",
	"gcc_pcie_0_aux_clk",
	"gcc_pcie_0_pipe_clk",
	"gcc_pcie_phy_aux_clk",
	"gcc_ufs_axi_clk",
	"gcc_ufs_ahb_clk",
	"gcc_ufs_tx_symbol_0_clk",
	"gcc_ufs_rx_symbol_0_clk",
	"gcc_ufs_rx_symbol_1_clk",
	"gcc_ufs_unipro_core_clk",
	"gcc_ufs_ice_core_clk",
	"gcc_dcc_ahb_clk",
	"ipa_clk",
	"gcc_mss_cfg_ahb_clk",
	"gcc_mss_q6_bimc_axi_clk",
	"gcc_mss_mnoc_bimc_axi_clk",
	"gcc_mss_snoc_axi_clk",
	"gcc_gpu_cfg_ahb_clk",
	"gcc_gpu_bimc_gfx_clk",
	"gcc_qspi_ahb_clk",
	"gcc_qspi_ref_clk",
};

static struct clk_debug_mux gcc_debug_mux = {
	.priv = &debug_mux_priv,
	.debug_offset = 0x62008,
	.post_div_offset = 0x62000,
	.cbcr_offset = 0x62004,
	.src_sel_mask = 0x3FF,
	.src_sel_shift = 0,
	.post_div_mask = 0xF,
	.post_div_shift = 0,
	MUX_SRC_LIST(
		{ "gpu_gcc_debug_clk",				0x013d },
		{ "gfx_gcc_debug_clk",				0x013d },
		{ "debug_mmss_clk",				0x0022 },
		{ "debug_cpu_clk",				0x00c0 },
		{ "snoc_clk",					0x0000 },
		{ "cnoc_clk",					0x000e },
		{ "bimc_clk",					0x014e },
		{ "gcc_mmss_sys_noc_axi_clk",			0x001f },
		{ "gcc_mmss_noc_cfg_ahb_clk",			0x0020 },
		{ "gcc_usb30_master_clk",			0x003e },
		{ "gcc_usb30_sleep_clk",			0x003f },
		{ "gcc_usb30_mock_utmi_clk",			0x0040 },
		{ "gcc_usb3_phy_aux_clk",			0x0041 },
		{ "gcc_usb3_phy_pipe_clk",			0x0042 },
		{ "gcc_sdcc2_apps_clk",				0x0046 },
		{ "gcc_sdcc2_ahb_clk",				0x0047 },
		{ "gcc_sdcc4_apps_clk",				0x0048 },
		{ "gcc_sdcc4_ahb_clk",				0x0049 },
		{ "gcc_blsp1_ahb_clk",				0x004a },
		{ "gcc_blsp1_qup1_spi_apps_clk",		0x004c },
		{ "gcc_blsp1_qup1_i2c_apps_clk",		0x004d },
		{ "gcc_blsp1_uart1_apps_clk",			0x004e },
		{ "gcc_blsp1_qup2_spi_apps_clk",		0x0050 },
		{ "gcc_blsp1_qup2_i2c_apps_clk",		0x0051 },
		{ "gcc_blsp1_uart2_apps_clk",			0x0052 },
		{ "gcc_blsp1_qup3_spi_apps_clk",		0x0054 },
		{ "gcc_blsp1_qup3_i2c_apps_clk",		0x0055 },
		{ "gcc_blsp1_uart3_apps_clk",			0x0056 },
		{ "gcc_blsp1_qup4_spi_apps_clk",		0x0058 },
		{ "gcc_blsp1_qup4_i2c_apps_clk",		0x0059 },
		{ "gcc_blsp1_qup5_spi_apps_clk",		0x005a },
		{ "gcc_blsp1_qup5_i2c_apps_clk",		0x005b },
		{ "gcc_blsp1_qup6_spi_apps_clk",		0x005c },
		{ "gcc_blsp1_qup6_i2c_apps_clk",		0x005d },
		{ "gcc_blsp2_ahb_clk",				0x005e },
		{ "gcc_blsp2_qup1_spi_apps_clk",		0x0060 },
		{ "gcc_blsp2_qup1_i2c_apps_clk",		0x0061 },
		{ "gcc_blsp2_uart1_apps_clk",			0x0062 },
		{ "gcc_blsp2_qup2_spi_apps_clk",		0x0064 },
		{ "gcc_blsp2_qup2_i2c_apps_clk",		0x0065 },
		{ "gcc_blsp2_uart2_apps_clk",			0x0066 },
		{ "gcc_blsp2_qup3_spi_apps_clk",		0x0068 },
		{ "gcc_blsp2_qup3_i2c_apps_clk",		0x0069 },
		{ "gcc_blsp2_uart3_apps_clk",			0x006a },
		{ "gcc_blsp2_qup4_spi_apps_clk",		0x006c },
		{ "gcc_blsp2_qup4_i2c_apps_clk",		0x006d },
		{ "gcc_blsp2_qup5_spi_apps_clk",		0x006e },
		{ "gcc_blsp2_qup5_i2c_apps_clk",		0x006f },
		{ "gcc_blsp2_qup6_spi_apps_clk",		0x0070 },
		{ "gcc_blsp2_qup6_i2c_apps_clk",		0x0071 },
		{ "gcc_pdm_ahb_clk",				0x0072 },
		{ "gcc_pdm2_clk",				0x0074 },
		{ "gcc_prng_ahb_clk",				0x0075 },
		{ "gcc_tsif_ahb_clk",				0x0076 },
		{ "gcc_tsif_ref_clk",				0x0077 },
		{ "gcc_boot_rom_ahb_clk",			0x007a },
		{ "ce1_clk",					0x0097 },
		{ "gcc_ce1_axi_m_clk",				0x0098 },
		{ "gcc_ce1_ahb_m_clk",				0x0099 },
		{ "measure_only_bimc_hmss_axi_clk",		0x00bb },
		{ "gcc_bimc_gfx_clk",				0x00ac },
		{ "gcc_hmss_rbcpr_clk",				0x00bc },
		{ "gcc_gp1_clk",				0x00df },
		{ "gcc_gp2_clk",				0x00e0 },
		{ "gcc_gp3_clk",				0x00e1 },
		{ "gcc_pcie_0_slv_axi_clk",			0x00e2 },
		{ "gcc_pcie_0_mstr_axi_clk",			0x00e3 },
		{ "gcc_pcie_0_cfg_ahb_clk",			0x00e4 },
		{ "gcc_pcie_0_aux_clk",				0x00e5 },
		{ "gcc_pcie_0_pipe_clk",			0x00e6 },
		{ "gcc_pcie_phy_aux_clk",			0x00e8 },
		{ "gcc_ufs_axi_clk",				0x00ea },
		{ "gcc_ufs_ahb_clk",				0x00eb },
		{ "gcc_ufs_tx_symbol_0_clk",			0x00ec },
		{ "gcc_ufs_rx_symbol_0_clk",			0x00ed },
		{ "gcc_ufs_rx_symbol_1_clk",			0x0162 },
		{ "gcc_ufs_unipro_core_clk",			0x00f0 },
		{ "gcc_ufs_ice_core_clk",			0x00f1 },
		{ "gcc_dcc_ahb_clk",				0x0119 },
		{ "ipa_clk",					0x011b },
		{ "gcc_mss_cfg_ahb_clk",			0x011f },
		{ "gcc_mss_q6_bimc_axi_clk",			0x0124 },
		{ "gcc_mss_mnoc_bimc_axi_clk",			0x0120 },
		{ "gcc_mss_snoc_axi_clk",			0x0123 },
		{ "gcc_gpu_cfg_ahb_clk",			0x013b },
		{ "gcc_gpu_bimc_gfx_clk",			0x013f },
		{ "gcc_qspi_ahb_clk",				0x0156 },
		{ "gcc_qspi_ref_clk",				0x0157 },
	),
	.hw.init = &(struct clk_init_data){
		.name = "gcc_debug_mux",
		.ops = &clk_debug_mux_ops,
		.parent_names = debug_mux_parent_names,
		.num_parents = ARRAY_SIZE(debug_mux_parent_names),
		.flags = CLK_IS_MEASURE,
	},
};

static struct clk_hw *gcc_msm8998_hws[] = {
	[GCC_XO] = &xo.hw,
	[GCC_BI_TCXO] = &bi_tcxo.hw,
	[GCC_CE1_AHB_M_CLK] = &gcc_ce1_ahb_m_clk.hw,
	[GCC_CE1_AXI_M_CLK] = &gcc_ce1_axi_m_clk.hw,
	[GCC_GPLL0_EARLY_DIV] = &gpll0_early_div.hw,
};

static struct clk_regmap *gcc_msm8998_clocks[] = {
	[GPLL0] = &gpll0.clkr,
	[GPLL0_AO] = &gpll0_ao.clkr,
	[GPLL0_OUT_MAIN] = &gpll0_out_main.clkr,
	[GPLL1] = &gpll1.clkr,
	[GPLL1_OUT_MAIN] = &gpll1_out_main.clkr,
	[GPLL2] = &gpll2.clkr,
	[GPLL2_OUT_MAIN] = &gpll2_out_main.clkr,
	[GPLL3] = &gpll3.clkr,
	[GPLL3_OUT_MAIN] = &gpll3_out_main.clkr,
	[GPLL4] = &gpll4.clkr,
	[GPLL4_OUT_MAIN] = &gpll4_out_main.clkr,
	[USB30_MASTER_CLK_SRC] = &usb30_master_clk_src.clkr,
	[PCIE_AUX_CLK_SRC] = &pcie_aux_clk_src.clkr,
	[UFS_AXI_CLK_SRC] = &ufs_axi_clk_src.clkr,
	[BLSP1_QUP1_I2C_APPS_CLK_SRC] = &blsp1_qup1_i2c_apps_clk_src.clkr,
	[BLSP1_QUP1_SPI_APPS_CLK_SRC] = &blsp1_qup1_spi_apps_clk_src.clkr,
	[BLSP1_QUP2_I2C_APPS_CLK_SRC] = &blsp1_qup2_i2c_apps_clk_src.clkr,
	[BLSP1_QUP2_SPI_APPS_CLK_SRC] = &blsp1_qup2_spi_apps_clk_src.clkr,
	[BLSP1_QUP3_I2C_APPS_CLK_SRC] = &blsp1_qup3_i2c_apps_clk_src.clkr,
	[BLSP1_QUP3_SPI_APPS_CLK_SRC] = &blsp1_qup3_spi_apps_clk_src.clkr,
	[BLSP1_QUP4_I2C_APPS_CLK_SRC] = &blsp1_qup4_i2c_apps_clk_src.clkr,
	[BLSP1_QUP4_SPI_APPS_CLK_SRC] = &blsp1_qup4_spi_apps_clk_src.clkr,
	[BLSP1_QUP5_I2C_APPS_CLK_SRC] = &blsp1_qup5_i2c_apps_clk_src.clkr,
	[BLSP1_QUP5_SPI_APPS_CLK_SRC] = &blsp1_qup5_spi_apps_clk_src.clkr,
	[BLSP1_QUP6_I2C_APPS_CLK_SRC] = &blsp1_qup6_i2c_apps_clk_src.clkr,
	[BLSP1_QUP6_SPI_APPS_CLK_SRC] = &blsp1_qup6_spi_apps_clk_src.clkr,
	[BLSP1_UART1_APPS_CLK_SRC] = &blsp1_uart1_apps_clk_src.clkr,
	[BLSP1_UART2_APPS_CLK_SRC] = &blsp1_uart2_apps_clk_src.clkr,
	[BLSP1_UART3_APPS_CLK_SRC] = &blsp1_uart3_apps_clk_src.clkr,
	[BLSP2_QUP1_I2C_APPS_CLK_SRC] = &blsp2_qup1_i2c_apps_clk_src.clkr,
	[BLSP2_QUP1_SPI_APPS_CLK_SRC] = &blsp2_qup1_spi_apps_clk_src.clkr,
	[BLSP2_QUP2_I2C_APPS_CLK_SRC] = &blsp2_qup2_i2c_apps_clk_src.clkr,
	[BLSP2_QUP2_SPI_APPS_CLK_SRC] = &blsp2_qup2_spi_apps_clk_src.clkr,
	[BLSP2_QUP3_I2C_APPS_CLK_SRC] = &blsp2_qup3_i2c_apps_clk_src.clkr,
	[BLSP2_QUP3_SPI_APPS_CLK_SRC] = &blsp2_qup3_spi_apps_clk_src.clkr,
	[BLSP2_QUP4_I2C_APPS_CLK_SRC] = &blsp2_qup4_i2c_apps_clk_src.clkr,
	[BLSP2_QUP4_SPI_APPS_CLK_SRC] = &blsp2_qup4_spi_apps_clk_src.clkr,
	[BLSP2_QUP5_I2C_APPS_CLK_SRC] = &blsp2_qup5_i2c_apps_clk_src.clkr,
	[BLSP2_QUP5_SPI_APPS_CLK_SRC] = &blsp2_qup5_spi_apps_clk_src.clkr,
	[BLSP2_QUP6_I2C_APPS_CLK_SRC] = &blsp2_qup6_i2c_apps_clk_src.clkr,
	[BLSP2_QUP6_SPI_APPS_CLK_SRC] = &blsp2_qup6_spi_apps_clk_src.clkr,
	[BLSP2_UART1_APPS_CLK_SRC] = &blsp2_uart1_apps_clk_src.clkr,
	[BLSP2_UART2_APPS_CLK_SRC] = &blsp2_uart2_apps_clk_src.clkr,
	[BLSP2_UART3_APPS_CLK_SRC] = &blsp2_uart3_apps_clk_src.clkr,
	[GP1_CLK_SRC] = &gp1_clk_src.clkr,
	[GP2_CLK_SRC] = &gp2_clk_src.clkr,
	[GP3_CLK_SRC] = &gp3_clk_src.clkr,
	[HMSS_RBCPR_CLK_SRC] = &hmss_rbcpr_clk_src.clkr,
	[PDM2_CLK_SRC] = &pdm2_clk_src.clkr,
	[SDCC2_APPS_CLK_SRC] = &sdcc2_apps_clk_src.clkr,
	[SDCC4_APPS_CLK_SRC] = &sdcc4_apps_clk_src.clkr,
	[TSIF_REF_CLK_SRC] = &tsif_ref_clk_src.clkr,
	[UFS_ICE_CORE_CLK_SRC] = &ufs_ice_core_clk_src.clkr,
	[UFS_PHY_AUX_CLK_SRC] = &ufs_phy_aux_clk_src.clkr,
	[UFS_UNIPRO_CORE_CLK_SRC] = &ufs_unipro_core_clk_src.clkr,
	[USB30_MOCK_UTMI_CLK_SRC] = &usb30_mock_utmi_clk_src.clkr,
	[USB3_PHY_AUX_CLK_SRC] = &usb3_phy_aux_clk_src.clkr,
	[HMSS_GPLL0_CLK_SRC] = &hmss_gpll0_clk_src.clkr,
	[QSPI_REF_CLK_SRC] = &qspi_ref_clk_src.clkr,
	[GPLL0_OUT_MSSCC] = &gpll0_out_msscc.clkr,
	[GCC_AGGRE1_UFS_AXI_CLK] = &gcc_aggre1_ufs_axi_clk.clkr,
	[GCC_AGGRE1_UFS_AXI_HW_CTL_CLK] = &gcc_aggre1_ufs_axi_hw_ctl_clk.clkr,
	[GCC_AGGRE1_USB3_AXI_CLK] = &gcc_aggre1_usb3_axi_clk.clkr,
	[GCC_BIMC_MSS_Q6_AXI_CLK] = &gcc_bimc_mss_q6_axi_clk.clkr,
	[GCC_BLSP1_AHB_CLK] = &gcc_blsp1_ahb_clk.clkr,
	[GCC_BLSP1_QUP1_I2C_APPS_CLK] = &gcc_blsp1_qup1_i2c_apps_clk.clkr,
	[GCC_BLSP1_QUP1_SPI_APPS_CLK] = &gcc_blsp1_qup1_spi_apps_clk.clkr,
	[GCC_BLSP1_QUP2_I2C_APPS_CLK] = &gcc_blsp1_qup2_i2c_apps_clk.clkr,
	[GCC_BLSP1_QUP2_SPI_APPS_CLK] = &gcc_blsp1_qup2_spi_apps_clk.clkr,
	[GCC_BLSP1_QUP3_I2C_APPS_CLK] = &gcc_blsp1_qup3_i2c_apps_clk.clkr,
	[GCC_BLSP1_QUP3_SPI_APPS_CLK] = &gcc_blsp1_qup3_spi_apps_clk.clkr,
	[GCC_BLSP1_QUP4_I2C_APPS_CLK] = &gcc_blsp1_qup4_i2c_apps_clk.clkr,
	[GCC_BLSP1_QUP4_SPI_APPS_CLK] = &gcc_blsp1_qup4_spi_apps_clk.clkr,
	[GCC_BLSP1_QUP5_I2C_APPS_CLK] = &gcc_blsp1_qup5_i2c_apps_clk.clkr,
	[GCC_BLSP1_QUP5_SPI_APPS_CLK] = &gcc_blsp1_qup5_spi_apps_clk.clkr,
	[GCC_BLSP1_QUP6_I2C_APPS_CLK] = &gcc_blsp1_qup6_i2c_apps_clk.clkr,
	[GCC_BLSP1_QUP6_SPI_APPS_CLK] = &gcc_blsp1_qup6_spi_apps_clk.clkr,
	[GCC_BLSP1_UART1_APPS_CLK] = &gcc_blsp1_uart1_apps_clk.clkr,
	[GCC_BLSP1_UART2_APPS_CLK] = &gcc_blsp1_uart2_apps_clk.clkr,
	[GCC_BLSP1_UART3_APPS_CLK] = &gcc_blsp1_uart3_apps_clk.clkr,
	[GCC_BLSP2_AHB_CLK] = &gcc_blsp2_ahb_clk.clkr,
	[GCC_BLSP2_QUP1_I2C_APPS_CLK] = &gcc_blsp2_qup1_i2c_apps_clk.clkr,
	[GCC_BLSP2_QUP1_SPI_APPS_CLK] = &gcc_blsp2_qup1_spi_apps_clk.clkr,
	[GCC_BLSP2_QUP2_I2C_APPS_CLK] = &gcc_blsp2_qup2_i2c_apps_clk.clkr,
	[GCC_BLSP2_QUP2_SPI_APPS_CLK] = &gcc_blsp2_qup2_spi_apps_clk.clkr,
	[GCC_BLSP2_QUP3_I2C_APPS_CLK] = &gcc_blsp2_qup3_i2c_apps_clk.clkr,
	[GCC_BLSP2_QUP3_SPI_APPS_CLK] = &gcc_blsp2_qup3_spi_apps_clk.clkr,
	[GCC_BLSP2_QUP4_I2C_APPS_CLK] = &gcc_blsp2_qup4_i2c_apps_clk.clkr,
	[GCC_BLSP2_QUP4_SPI_APPS_CLK] = &gcc_blsp2_qup4_spi_apps_clk.clkr,
	[GCC_BLSP2_QUP5_I2C_APPS_CLK] = &gcc_blsp2_qup5_i2c_apps_clk.clkr,
	[GCC_BLSP2_QUP5_SPI_APPS_CLK] = &gcc_blsp2_qup5_spi_apps_clk.clkr,
	[GCC_BLSP2_QUP6_I2C_APPS_CLK] = &gcc_blsp2_qup6_i2c_apps_clk.clkr,
	[GCC_BLSP2_QUP6_SPI_APPS_CLK] = &gcc_blsp2_qup6_spi_apps_clk.clkr,
	[GCC_BLSP2_UART1_APPS_CLK] = &gcc_blsp2_uart1_apps_clk.clkr,
	[GCC_BLSP2_UART2_APPS_CLK] = &gcc_blsp2_uart2_apps_clk.clkr,
	[GCC_BLSP2_UART3_APPS_CLK] = &gcc_blsp2_uart3_apps_clk.clkr,
	[GCC_CFG_NOC_USB3_AXI_CLK] = &gcc_cfg_noc_usb3_axi_clk.clkr,
	[GCC_BIMC_GFX_CLK] = &gcc_bimc_gfx_clk.clkr,
	[GCC_GP1_CLK] = &gcc_gp1_clk.clkr,
	[GCC_GP2_CLK] = &gcc_gp2_clk.clkr,
	[GCC_GP3_CLK] = &gcc_gp3_clk.clkr,
	[GCC_GPU_BIMC_GFX_CLK] = &gcc_gpu_bimc_gfx_clk.clkr,
	[GCC_GPU_CFG_AHB_CLK] = &gcc_gpu_cfg_ahb_clk.clkr,
	[GCC_GPU_IREF_CLK] = &gcc_gpu_iref_clk.clkr,
	[GCC_HMSS_DVM_BUS_CLK] = &gcc_hmss_dvm_bus_clk.clkr,
	[GCC_HMSS_RBCPR_CLK] = &gcc_hmss_rbcpr_clk.clkr,
	[GCC_MMSS_NOC_CFG_AHB_CLK] = &gcc_mmss_noc_cfg_ahb_clk.clkr,
	[GCC_MMSS_SYS_NOC_AXI_CLK] = &gcc_mmss_sys_noc_axi_clk.clkr,
	[GCC_PCIE_0_AUX_CLK] = &gcc_pcie_0_aux_clk.clkr,
	[GCC_PCIE_0_CFG_AHB_CLK] = &gcc_pcie_0_cfg_ahb_clk.clkr,
	[GCC_PCIE_0_MSTR_AXI_CLK] = &gcc_pcie_0_mstr_axi_clk.clkr,
	[GCC_PCIE_0_PIPE_CLK] = &gcc_pcie_0_pipe_clk.clkr,
	[GCC_PCIE_0_SLV_AXI_CLK] = &gcc_pcie_0_slv_axi_clk.clkr,
	[GCC_PCIE_PHY_AUX_CLK] = &gcc_pcie_phy_aux_clk.clkr,
	[GCC_PDM2_CLK] = &gcc_pdm2_clk.clkr,
	[GCC_PDM_AHB_CLK] = &gcc_pdm_ahb_clk.clkr,
	[GCC_SDCC2_AHB_CLK] = &gcc_sdcc2_ahb_clk.clkr,
	[GCC_SDCC2_APPS_CLK] = &gcc_sdcc2_apps_clk.clkr,
	[GCC_SDCC4_AHB_CLK] = &gcc_sdcc4_ahb_clk.clkr,
	[GCC_SDCC4_APPS_CLK] = &gcc_sdcc4_apps_clk.clkr,
	[GCC_TSIF_AHB_CLK] = &gcc_tsif_ahb_clk.clkr,
	[GCC_TSIF_REF_CLK] = &gcc_tsif_ref_clk.clkr,
	[GCC_UFS_AHB_CLK] = &gcc_ufs_ahb_clk.clkr,
	[GCC_UFS_AXI_CLK] = &gcc_ufs_axi_clk.clkr,
	[GCC_UFS_AXI_HW_CTL_CLK] = &gcc_ufs_axi_hw_ctl_clk.clkr,
	[GCC_UFS_ICE_CORE_CLK] = &gcc_ufs_ice_core_clk.clkr,
	[GCC_UFS_ICE_CORE_HW_CTL_CLK] = &gcc_ufs_ice_core_hw_ctl_clk.clkr,
	[GCC_UFS_PHY_AUX_CLK] = &gcc_ufs_phy_aux_clk.clkr,
	[GCC_UFS_PHY_AUX_HW_CTL_CLK] = &gcc_ufs_phy_aux_hw_ctl_clk.clkr,
	[GCC_UFS_RX_SYMBOL_0_CLK] = &gcc_ufs_rx_symbol_0_clk.clkr,
	[GCC_UFS_RX_SYMBOL_1_CLK] = &gcc_ufs_rx_symbol_1_clk.clkr,
	[GCC_UFS_TX_SYMBOL_0_CLK] = &gcc_ufs_tx_symbol_0_clk.clkr,
	[GCC_UFS_UNIPRO_CORE_CLK] = &gcc_ufs_unipro_core_clk.clkr,
	[GCC_UFS_UNIPRO_CORE_HW_CTL_CLK] = &gcc_ufs_unipro_core_hw_ctl_clk.clkr,
	[GCC_USB30_MASTER_CLK] = &gcc_usb30_master_clk.clkr,
	[GCC_USB30_MOCK_UTMI_CLK] = &gcc_usb30_mock_utmi_clk.clkr,
	[GCC_USB30_SLEEP_CLK] = &gcc_usb30_sleep_clk.clkr,
	[GCC_USB3_PHY_AUX_CLK] = &gcc_usb3_phy_aux_clk.clkr,
	[GCC_USB3_PHY_PIPE_CLK] = &gcc_usb3_phy_pipe_clk.clkr,
	[GCC_PRNG_AHB_CLK] = &gcc_prng_ahb_clk.clkr,
	[GCC_BOOT_ROM_AHB_CLK] = &gcc_boot_rom_ahb_clk.clkr,
	[GCC_MSS_CFG_AHB_CLK] = &gcc_mss_cfg_ahb_clk.clkr,
	[GCC_MSS_Q6_BIMC_AXI_CLK] = &gcc_mss_q6_bimc_axi_clk.clkr,
	[GCC_MSS_MNOC_BIMC_AXI_CLK] = &gcc_mss_mnoc_bimc_axi_clk.clkr,
	[GCC_MSS_SNOC_AXI_CLK] = &gcc_mss_snoc_axi_clk.clkr,
	[GCC_HDMI_CLKREF_CLK] = &gcc_hdmi_clkref_clk.clkr,
	[GCC_PCIE_CLKREF_CLK] = &gcc_pcie_clkref_clk.clkr,
	[GCC_RX1_USB2_CLKREF_CLK] = &gcc_rx1_usb2_clkref_clk.clkr,
	[GCC_UFS_CLKREF_CLK] = &gcc_ufs_clkref_clk.clkr,
	[GCC_USB3_CLKREF_CLK] = &gcc_usb3_clkref_clk.clkr,
	[GCC_DCC_AHB_CLK] = &gcc_dcc_ahb_clk.clkr,
	[HLOS1_VOTE_LPASS_CORE_SMMU_CLK] = &hlos1_vote_lpass_core_smmu_clk.clkr,
	[HLOS1_VOTE_LPASS_ADSP_SMMU_CLK] = &hlos1_vote_lpass_adsp_smmu_clk.clkr,
	[GCC_QSPI_AHB_CLK] = &gcc_qspi_ahb_clk.clkr,
	[GCC_QSPI_REF_CLK] = &gcc_qspi_ref_clk.clkr,
	[GCC_GPU_GPLL0_CLK] = &gcc_gpu_gpll0_clk.clkr,
	[GCC_GPU_GPLL0_DIV_CLK] = &gcc_gpu_gpll0_div_clk.clkr,
	[GCC_MMSS_GPLL0_CLK] = &gcc_mmss_gpll0_clk.clkr,
	[GCC_MMSS_GPLL0_DIV_CLK] = &gcc_mmss_gpll0_div_clk.clkr,
	[GCC_USB_PHY_CFG_AHB2PHY_CLK] = &gcc_usb_phy_cfg_ahb2phy_clk.clkr,
};

static const struct qcom_reset_map gcc_msm8998_resets[] = {
	[QUSB2PHY_PRIM_BCR] = { 0x12000 },
	[QUSB2PHY_SEC_BCR] = { 0x12004 },
	[BLSP1_BCR] = { 0x17000 },
	[BLSP2_BCR] = { 0x25000 },
	[BOOT_ROM_BCR] = { 0x38000 },
	[PRNG_BCR] = { 0x34000 },
	[UFS_BCR] = { 0x75000 },
	[USB_30_BCR] = { 0x0f000 },
	[USB3_PHY_BCR] = { 0x50020 },
	[USB3PHY_PHY_BCR] = { 0x50024 },
	[PCIE_0_PHY_BCR] = { 0x6c01c },
	[PCIE_PHY_BCR] = { 0x6f000 },
	[PCIE_PHY_NOCSR_COM_PHY_BCR] = { 0x6f00C },
	[PCIE_PHY_COM_BCR] = { 0x6f014 },
};

static const struct regmap_config gcc_msm8998_regmap_config = {
	.reg_bits	= 32,
	.reg_stride	= 4,
	.val_bits	= 32,
	.max_register	= 0xb0000,
	.fast_io	= true,
};

static const struct qcom_cc_desc gcc_msm8998_desc = {
	.config = &gcc_msm8998_regmap_config,
	.clks = gcc_msm8998_clocks,
	.num_clks = ARRAY_SIZE(gcc_msm8998_clocks),
	.resets = gcc_msm8998_resets,
	.num_resets = ARRAY_SIZE(gcc_msm8998_resets),
};

static struct of_device_id gcc_msm8998_match_table[] = {
	{
		.compatible = "qcom,gcc-msm8998",
		.data = (void *)(uintptr_t)1,
	},
	{
		.compatible = "qcom,gcc-msm8998-v2",
		.data = (void *)(uintptr_t)2,
	},
	{
		.compatible = "qcom,gcc-msmhamster",
		.data = (void *)(uintptr_t)0,
	},
	{}
};

static void gcc_msm8998_clocks_fixup(int socrev)
{
	if (socrev < 1)
		return;

	/* 8998v1 only */
	if (socrev == 1)
		gcc_msm8998_clocks[GCC_UFS_RX_SYMBOL_1_CLK] = NULL;

	/* 8998v1 and v2 */
	if (socrev >= 1) {
		gcc_msm8998_clocks[QSPI_REF_CLK_SRC] = NULL;
		gcc_msm8998_clocks[GCC_QSPI_REF_CLK] = NULL;
		gcc_msm8998_clocks[GCC_QSPI_AHB_CLK] = NULL;	
	}
}

static int gcc_msm8998_probe(struct platform_device *pdev)
{
	const struct of_device_id *match;
	struct regmap *regmap;
	int i, socrev, ret = 0;

	match = of_match_node(gcc_msm8998_match_table, pdev->dev.of_node);
	if (match) {
		socrev = (uintptr_t)match->data;
		gcc_msm8998_clocks_fixup(socrev);
	} else {
		pr_err("Cannot find compatible string match!\n");
	}

	regmap = qcom_cc_map(pdev, &gcc_msm8998_desc);
	if (IS_ERR(regmap))
		return PTR_ERR(regmap);

	/*
	 * Clear the HMSS_AHB_CLK_ENA bit to allow the gcc_hmss_ahb_clk clock
	 * to be gated by RPM during VDD_MIN.
	 */
	ret = regmap_update_bits(regmap, 0x52008, BIT(21), 0); //BIT(21));
	if (ret)
		return ret;

	vdd_dig.regulator[0] = devm_regulator_get(&pdev->dev, "vdd_dig");
	if (IS_ERR(vdd_dig.regulator[0])) {
		if (!(PTR_ERR(vdd_dig.regulator[0]) == -EPROBE_DEFER))
			dev_err(&pdev->dev,
					"Unable to get vdd_dig regulator\n");
		return PTR_ERR(vdd_dig.regulator[0]);
	}

	vdd_dig_ao.regulator[0] = devm_regulator_get(&pdev->dev, "vdd_dig_ao");
	if (IS_ERR(vdd_dig_ao.regulator[0])) {
		if (!(PTR_ERR(vdd_dig_ao.regulator[0]) == -EPROBE_DEFER))
			dev_err(&pdev->dev,
					"Unable to get vdd_dig_ao regulator\n");
		return PTR_ERR(vdd_dig_ao.regulator[0]);
	}

	/* Register the hws */
	for (i = 0; i < ARRAY_SIZE(gcc_msm8998_hws); i++) {
		ret = devm_clk_hw_register(&pdev->dev, gcc_msm8998_hws[i]);
		if (ret)
			return ret;
	}

	ret = qcom_cc_really_probe(pdev, &gcc_msm8998_desc, regmap);
	if (ret) {
		dev_err(&pdev->dev, "Failed to register GCC clocks\n");
		return ret;
	}

	//clk_set_rate(gpll0_early_div.clkr.hw.clk, 300000000);

	/* 
	 * GCC_MMSS_MISC - GCC_GPU_MISC:
	 * 1. Disable the GPLL0 active input to MMSS and GPU
	 * 2. Select clk division 1 (CLK/2)
	 */
	regmap_write(regmap, 0x0902C, 0x10003); /* MMSS*/
	regmap_write(regmap, 0x71028, 0x10003); /* GPU */

	/* This clock is used for all MMSSCC register access */
	clk_prepare_enable(gcc_mmss_noc_cfg_ahb_clk.clkr.hw.clk);

	/* Keep bimc gfx clock port on all the time */
	clk_prepare_enable(gcc_bimc_gfx_clk.clkr.hw.clk);

	/* This clock is used for all GPUCC register access */
	clk_prepare_enable(gcc_gpu_cfg_ahb_clk.clkr.hw.clk);

	/* Set the HMSS_GPLL0_SRC for 300MHz to CPU subsystem */
	clk_set_rate(hmss_gpll0_clk_src.clkr.hw.clk, 300000000);

	clk_set_flags(gcc_gpu_bimc_gfx_clk.clkr.hw.clk, CLKFLAG_RETAIN_MEM);

	dev_info(&pdev->dev, "Registered GCC clocks\n");
	return 0;
}

static struct platform_driver gcc_msm8998_driver = {
	.probe = gcc_msm8998_probe,
	.driver = {
		.name = "qcom,gcc-msm8998",
		.of_match_table = gcc_msm8998_match_table,
		.owner = THIS_MODULE,
	},
};

int __init gcc_msm8998_init(void)
{
	return platform_driver_register(&gcc_msm8998_driver);
}
arch_initcall(gcc_msm8998_init);

/* ======== Clock Debug Controller ======== */
/*
static struct clk_lookup msm_clocks_measure_8998[] = {
	CLK_LIST(gpu_gcc_debug_clk),
	CLK_LIST(gfx_gcc_debug_clk),
	CLK_LIST(debug_mmss_clk),
	CLK_LIST(debug_cpu_clk),
	CLK_LOOKUP_OF("measure", gcc_debug_mux, "debug"),
};
*/
static struct of_device_id clk_debug_match_table[] = {
	{ .compatible = "qcom,debugcc-msm8998" },
	{}
};

static int msm_clock_debug_8998_probe(struct platform_device *pdev)
{
	struct clk *clk;
	int ret = 0, count;

	clk = devm_clk_get(&pdev->dev, "xo_clk_src");
	if (IS_ERR(clk)) {
		if (PTR_ERR(clk) != -EPROBE_DEFER)
			dev_err(&pdev->dev, "Unable to get xo clock\n");
		return PTR_ERR(clk);
	}

	debug_mux_priv.cxo = clk;

	//clk_ops_debug_mux = clk_ops_gen_mux;
	//clk_ops_debug_mux.get_rate = measure_get_rate;
	ret = of_property_read_u32(pdev->dev.of_node, "qcom,cc-count",
								&count);
	if (ret < 0) {
		dev_err(&pdev->dev, "Num of debug clock controller not specified\n");
		return ret;
	}

	if (!count) {
		dev_err(&pdev->dev, "Count of CC cannot be zero\n");
		return -EINVAL;
	}

	gcc_debug_mux.regmap = devm_kzalloc(&pdev->dev,
				sizeof(struct regmap *) * count, GFP_KERNEL);
	if (!gcc_debug_mux.regmap)
		return -ENOMEM;

	if (of_get_property(pdev->dev.of_node, "qcom,gcc", NULL)) {
		gcc_debug_mux.regmap[GCC] =
			syscon_regmap_lookup_by_phandle(pdev->dev.of_node,
					"qcom,gcc");
		if (IS_ERR(gcc_debug_mux.regmap[GCC])) {
			pr_err("Failed to map qcom,gcc\n");
			return PTR_ERR(gcc_debug_mux.regmap[GCC]);
		}
	}

	if (of_get_property(pdev->dev.of_node, "qcom,dispcc", NULL)) {
		gcc_debug_mux.regmap[DISP_CC] =
			syscon_regmap_lookup_by_phandle(pdev->dev.of_node,
					"qcom,dispcc");
		if (IS_ERR(gcc_debug_mux.regmap[DISP_CC])) {
			pr_err("Failed to map qcom,dispcc\n");
			return PTR_ERR(gcc_debug_mux.regmap[DISP_CC]);
		}
	}

	if (of_get_property(pdev->dev.of_node, "qcom,gpucc", NULL)) {
		gcc_debug_mux.regmap[GPU_CC] =
			syscon_regmap_lookup_by_phandle(pdev->dev.of_node,
					"qcom,gpucc");
		if (IS_ERR(gcc_debug_mux.regmap[GPU_CC])) {
			pr_err("Failed to map qcom,gpucc\n");
			return PTR_ERR(gcc_debug_mux.regmap[GPU_CC]);
		}
	}

	if (of_get_property(pdev->dev.of_node, "qcom,cpucc", NULL)) {
		gcc_debug_mux.regmap[CPU] =
			syscon_regmap_lookup_by_phandle(pdev->dev.of_node,
					"qcom,cpucc");
		if (IS_ERR(gcc_debug_mux.regmap[CPU])) {
			pr_err("Failed to map qcom,cpucc\n");
			return PTR_ERR(gcc_debug_mux.regmap[CPU]);
		}
	}

	clk = devm_clk_register(&pdev->dev, &gcc_debug_mux.hw);
	if (IS_ERR(clk)) {
		dev_err(&pdev->dev, "Unable to register GCC debug mux\n");
		return PTR_ERR(clk);
	}

	ret = clk_debug_measure_register(&gcc_debug_mux.hw);
	if (ret)
		dev_err(&pdev->dev, "Could not register Measure clock\n");
	else
		dev_info(&pdev->dev, "Registered debug mux successfully\n");

	return ret;
}

static struct platform_driver clk_debug_msm8998_driver = {
	.probe = msm_clock_debug_8998_probe,
	.driver = {
		.name = "qcom,debugcc-msm8998",
		.of_match_table = clk_debug_match_table,
		.owner = THIS_MODULE,
	},
};

int __init clk_debug_msm8998_init(void)
{
	return platform_driver_register(&clk_debug_msm8998_driver);
}
late_initcall(clk_debug_msm8998_init);
