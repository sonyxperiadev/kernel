/*
 * Linux mainline API clocks implementation for MSM8998
 * MultiMedia Clock Controller (MMCC) driver
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

#include <dt-bindings/clock/qcom,mmcc-msm8998.h>

#include "common.h"
#include "clk-debug.h"
#include "clk-regmap.h"
#include "clk-pll.h"
#include "clk-rcg.h"
#include "clk-branch.h"
#include "clk-alpha-pll.h"
#include "clk-regmap-divider.h"
#include "clk-voter.h"
#include "reset.h"
#include "vdd-level-8998.h"


#define F(f, s, h, m, n) { (f), (s), (2 * (h) - 1), (m), (n) }

#define F_SLEW(f, s, h, m, n, src_freq) { (f), (s), (2 * (h) - 1), (m), (n), \
				(src_freq) }

static DEFINE_VDD_REGULATORS(vdd_dig, VDD_DIG_NUM, 1, vdd_corner);
static DEFINE_VDD_REGULATORS(vdd_mmsscc_mx, VDD_DIG_NUM, 1, vdd_corner);


enum {
	P_XO,
	P_GPLL0,
	P_GPLL0_DIV,
	P_MMPLL0,
	P_MMPLL1,
	P_MMPLL3,
	P_MMPLL4,
	P_MMPLL5,
	P_MMPLL6,
	P_MMPLL7,
	P_MMPLL10,
	P_HDMI_PLL,
	P_DP_PHY_PLL_LINK_CLK,
	P_DP_PHY_PLL_VCO_DIV_CLK,
	P_DSI0_PHY_PLL_OUT_BYTECLK,
	P_DSI0_PHY_PLL_OUT_DSICLK,
	P_DSI1_PHY_PLL_OUT_BYTECLK,
	P_DSI1_PHY_PLL_OUT_DSICLK,
};

static const struct parent_map mmcc_parent_map_gcc_0[] = {
	{ P_XO, 0},
	{ P_GPLL0, 5 },
};

static const char * const mmcc_parent_names_gcc_0[] = {
	"cxo",
	"gcc_mmss_gpll0_clk",
};

static const struct parent_map mmcc_parent_map_gcc_1[] = {
	{ P_XO, 0},
	{ P_GPLL0, 5 },
	{ P_GPLL0_DIV, 6 },
};

static const char * const mmcc_parent_names_gcc_1[] = {
	"cxo",
	"gcc_mmss_gpll0_clk",
	"gcc_mmss_gpll0_div_clk",
};

static const struct parent_map mmcc_parent_map_1[] = {
	{ P_XO, 0},
	{ P_GPLL0, 5 },
	{ P_MMPLL0, 1 },
};

static const char * const mmcc_parent_names_1[] = {
	"cxo",
	"gcc_mmss_gpll0_clk",
	"mmpll0",
};

static const struct parent_map mmcc_parent_map_2[] = {
	{ P_XO, 0 },
	{ P_GPLL0, 5},
	{ P_MMPLL4, 2 },
	{ P_MMPLL7, 3 },
	{ P_MMPLL10, 4 },
};

static const char * const mmcc_parent_names_2[] = {
	"cxo",
	"gcc_mmss_gpll0_clk",
	"mmpll4",
	"mmpll7",
	"mmpll10",
};

static const struct parent_map mmcc_parent_map_2a[] = {
	{ P_XO, 0 },
	{ P_GPLL0, 5},
	{ P_MMPLL0, 1 },
	{ P_MMPLL4, 2 },
	{ P_MMPLL7, 3 },
	{ P_MMPLL10, 4 },
};

static const char * const mmcc_parent_names_2a[] = {
	"cxo",
	"gcc_mmss_gpll0_clk",
	"mmpll0",
	"mmpll4",
	"mmpll7",
	"mmpll10",
};


static const struct parent_map mmcc_parent_map_3[] = {
	{ P_XO, 0 },
	{ P_GPLL0, 5 },
	{ P_GPLL0_DIV, 6 },
	{ P_MMPLL5, 2 },
};

static const char * const mmcc_parent_names_3[] = {
	"cxo",
	"gcc_mmss_gpll0_clk",
	"gcc_mmss_gpll0_div_clk",
	"mmpll5",
};

static const struct parent_map mmcc_parent_map_4[] = {
	{ P_XO, 0 },
	{ P_GPLL0, 5 },
	{ P_GPLL0_DIV, 6 },
	{ P_MMPLL0, 1 },
	{ P_MMPLL1, 2 },
};

static const char * const mmcc_parent_names_4[] = {
	"cxo",
	"gcc_mmss_gpll0_clk",
	"gcc_mmss_gpll0_div_clk",
	"mmpll0",
	"mmpll1",
};

static const struct parent_map mmcc_parent_map_5[] = {
	{ P_XO, 0 },
	{ P_GPLL0, 5 },
	{ P_MMPLL5, 2 },
};

static const char * const mmcc_parent_names_5[] = {
	"cxo",
	"gcc_mmss_gpll0_clk",
	"mmpll5",
};

static const struct parent_map mmcc_parent_map_6[] = {
	{ P_XO, 0 },
	{ P_GPLL0, 5 },
	{ P_MMPLL7, 3 },
};

static const char * const mmcc_parent_names_6[] = {
	"cxo",
	"gcc_mmss_gpll0_clk",
	"mmpll7",
};

static const struct parent_map mmcc_parent_map_7[] = {
	{ P_XO, 0 },
	{ P_GPLL0, 5 },
	{ P_MMPLL3, 3 },
	{ P_MMPLL6, 4 },
};

static const char * const mmcc_parent_names_7[] = {
	"cxo",
	"gcc_mmss_gpll0_clk",
	"mmpll3",
	"mmpll6",
};

static const struct parent_map mmcc_parent_map_7a[] = {
	{ P_XO, 0 },
	{ P_GPLL0, 5 },
	{ P_MMPLL0, 1 },
	{ P_MMPLL3, 3 },
	{ P_MMPLL6, 4 },
};

static const char * const mmcc_parent_names_7a[] = {
	"cxo",
	"gcc_mmss_gpll0_clk",
	"mmpll0",
	"mmpll3",
	"mmpll6",
};


static const struct parent_map mmcc_parent_map_8[] = {
	{ P_XO, 0 },
	{ P_GPLL0, 5 },
	{ P_MMPLL0, 1 },
	{ P_MMPLL7, 3 },
	{ P_MMPLL10, 4 },
};

static const char * const mmcc_parent_names_8[] = {
	"cxo",
	"gcc_mmss_gpll0_clk",
	"mmpll0",
	"mmpll7",
	"mmpll10",
};

/* Display Clock Controller (integrated in the MMCC) */
static const struct parent_map disp_cc_parent_map_0[] = {
	{ P_XO, 0 },
	{ P_DSI0_PHY_PLL_OUT_BYTECLK, 1 },
	{ P_DSI1_PHY_PLL_OUT_BYTECLK, 2 },
};

static const char * const disp_cc_parent_names_0[] = {
	"cxo",
	"dsi0_phy_pll_out_byteclk",
	"dsi1_phy_pll_out_byteclk",
};

static const struct parent_map disp_cc_parent_map_1[] = {
	{ P_XO, 0 },
	{ P_DP_PHY_PLL_LINK_CLK, 1 },
	{ P_DP_PHY_PLL_VCO_DIV_CLK, 2 },
};

static const char * const disp_cc_parent_names_1[] = {
	"cxo",
	"dp_link_clk_divsel_ten",
	"dp_vco_divided_clk_src_mux",
};

static const struct parent_map disp_cc_parent_map_2[] = {
	{ P_XO, 0 },
	{ P_GPLL0, 5 },
	{ P_GPLL0_DIV, 6 },
};

static const char * const disp_cc_parent_names_2[] = {
	"cxo",
	"gcc_mmss_gpll0_clk",
	"gcc_mmss_gpll0_div_clk",
};
static const struct parent_map disp_cc_parent_map_3[] = {
	{ P_XO, 0 },
	{ P_MMPLL5, 2 },
	{ P_GPLL0, 5 },
	{ P_GPLL0_DIV, 6 },
};

static const char * const disp_cc_parent_names_3[] = {
	"cxo",
	"mmpll5",
	"gcc_mmss_gpll0_clk",
	"gcc_mmss_gpll0_div_clk",
};

static const struct parent_map disp_cc_parent_map_4[] = {
	{ P_XO, 0 },
	{ P_DSI0_PHY_PLL_OUT_DSICLK, 1 },
	{ P_DSI1_PHY_PLL_OUT_DSICLK, 2 },
};

static const char * const disp_cc_parent_names_4[] = {
	"cxo",
	"dsi0_phy_pll_out_dsiclk",
	"dsi1_phy_pll_out_dsiclk",
};

static const struct parent_map disp_cc_parent_map_5[] = {
	{ P_XO, 0 },
	{ P_HDMI_PLL, 1 },
};

static const char * const disp_cc_parent_names_5[] = {
	"cxo",
	"hdmipll",
};

/* Initial configuration for 808MHz rate */
static const struct alpha_pll_config mmpll0_config = {
	/*.config_ctl_val = 0x20485699,*/
	.l = 0x2a,
	.frac = 0x1556,
};

static struct clk_alpha_pll mmpll0 = {
	.offset = 0xC000,
	.regs = clk_alpha_pll_regs[CLK_ALPHA_PLL_TYPE_FABIA],
	.clkr = {
		.enable_reg = 0x1E0,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "mmpll0",
			.parent_names = (const char *[]){ "cxo" },
			.num_parents = 1,
			.ops = &clk_alpha_pll_fabia_fixed_ops,
			VDD_MM_PLL_FMAX_MAP2(LOWER, 404000000, NOMINAL, 808000195),
		},
	},
};


/* Initial configuration for 812MHz rate */
static const struct alpha_pll_config mmpll1_config = {
	/*.config_ctl_val = 0x20485699,*/
	.l = 0x2a,
	.frac = 0x4aab,
};

static struct clk_alpha_pll mmpll1 = {
	.offset = 0xC050,
	.regs = clk_alpha_pll_regs[CLK_ALPHA_PLL_TYPE_FABIA],
	.clkr = {
		.enable_reg = 0x1E0,
		.enable_mask = BIT(1),
		.hw.init = &(struct clk_init_data){
			.name = "mmpll1",
			.parent_names = (const char *[]){ "cxo" },
			.num_parents = 1,
			.ops = &clk_alpha_pll_fabia_fixed_ops,
			VDD_MM_PLL_FMAX_MAP2(LOWER, 406000000, NOMINAL, 812000097),
		},
	},
};

/* Initial configuration for 1066MHz rate */
static const struct alpha_pll_config mmpll3_config = {
	/*.config_ctl_val = 0x20485699,*/
	.l = 0x37,
	.frac = 0x8556,
};

static struct clk_alpha_pll mmpll3 = {
	.offset = 0x0,
	.regs = clk_alpha_pll_regs[CLK_ALPHA_PLL_TYPE_FABIA],
	.clkr.hw.init = &(struct clk_init_data){
		.name = "mmpll3",
		.parent_names = (const char *[]){ "cxo" },
		.num_parents = 1,
		.ops = &clk_alpha_pll_fabia_fixed_ops,
		VDD_MM_PLL_FMAX_MAP2(LOWER, 465000000, LOW, 930000000),
	},
};

/* Initial configuration for 768MHz rate */
static const struct alpha_pll_config mmpll4_config = {
	/*.config_ctl_val = 0x20485699,*/
	.l = 0x28,
};

static struct clk_alpha_pll mmpll4 = {
	.offset = 0x50,
	.regs = clk_alpha_pll_regs[CLK_ALPHA_PLL_TYPE_FABIA],
	.clkr.hw.init = &(struct clk_init_data){
		.name = "mmpll4",
		.parent_names = (const char *[]){ "cxo" },
		.num_parents = 1,
		.ops = &clk_alpha_pll_fabia_fixed_ops,
		VDD_MM_PLL_FMAX_MAP2(LOWER, 384000000, LOW, 768000000),
	},
};

/* Initial configuration for 825MHz rate */
static const struct alpha_pll_config mmpll5_config = {
	/*.config_ctl_val = 0x20485699,*/
	.l = 0x2a,
	.frac = 0xf800,
};

static struct clk_alpha_pll mmpll5 = {
	.offset = 0xA0,
	.regs = clk_alpha_pll_regs[CLK_ALPHA_PLL_TYPE_FABIA],
	.clkr.hw.init = &(struct clk_init_data){
		.name = "mmpll5",
		.parent_names = (const char *[]){ "cxo" },
		.num_parents = 1,
		.ops = &clk_alpha_pll_fabia_fixed_ops,
		VDD_MM_PLL_FMAX_MAP2(LOWER, 412500000, LOW, 825000000),
	},
};

/* Initial configuration for 888MHz rate */
static const struct alpha_pll_config mmpll6_config = {
	/*.config_ctl_val = 0x20485699,*/
	.l = 0x2e,
	.frac = 0x4000,
};

static struct clk_alpha_pll mmpll6 = {
	.offset = 0xF0,
	.regs = clk_alpha_pll_regs[CLK_ALPHA_PLL_TYPE_FABIA],
	.clkr.hw.init = &(struct clk_init_data){
		.name = "mmpll6",
		.parent_names = (const char *[]){ "cxo" },
		.num_parents = 1,
		.ops = &clk_alpha_pll_fabia_fixed_ops,
		VDD_MM_PLL_FMAX_MAP2(LOWER, 412500000, LOW, 825000000),
	},
};

/* Initial configuration for 960MHz rate */
static const struct alpha_pll_config mmpll7_config = {
	/*.config_ctl_val = 0x20485699,*/
	.l = 0x32,
};

static struct clk_alpha_pll mmpll7 = {
	.offset = 0x140,
	.regs = clk_alpha_pll_regs[CLK_ALPHA_PLL_TYPE_FABIA],
	.clkr.hw.init = &(struct clk_init_data){
		.name = "mmpll7",
		.parent_names = (const char *[]){ "cxo" },
		.num_parents = 1,
		.ops = &clk_alpha_pll_fabia_fixed_ops,
		VDD_MM_PLL_FMAX_MAP1(LOW, 960000000),
	},
};

/* Initial configuration for 576MHz rate */
static const struct alpha_pll_config mmpll10_config = {
	/*.config_ctl_val = 0x20485699,*/
	.l = 0x1e,
};

static struct clk_alpha_pll mmpll10 = {
	.offset = 0x190,
	.regs = clk_alpha_pll_regs[CLK_ALPHA_PLL_TYPE_FABIA],
	.clkr.hw.init = &(struct clk_init_data){
		.name = "mmpll10",
		.parent_names = (const char *[]){ "cxo" },
		.num_parents = 1,
		.ops = &clk_alpha_pll_fabia_fixed_ops,
		VDD_MM_PLL_FMAX_MAP2(LOWER, 288000000, NOMINAL, 576000000),
	},
};

static struct freq_tbl ftbl_ahb_clk_src[] = {
	F(  19200000, P_XO,         1,    0,     0),
	F(  40000000, P_GPLL0,     15,    0,     0),
	F(  80800000, P_MMPLL0,    10,    0,     0),
	{ }
};

static struct clk_rcg2 ahb_clk_src = {
	.cmd_rcgr = 0x05000,
	.hid_width = 5,
	.parent_map = mmcc_parent_map_1,
	.freq_tbl = ftbl_ahb_clk_src,
	.enable_safe_config = true,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "ahb_clk_src",
		.parent_names = mmcc_parent_names_1,
		.num_parents = ARRAY_SIZE(mmcc_parent_names_1),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP3(LOWER, 19200000, LOW, 40000000,
					NOMINAL, 80800000),
	},
};

static struct freq_tbl ftbl_csi_clk_src[] = {
	F(  19200000,  P_XO,         1,    0,     0),
	F( 164571429,  P_MMPLL10,  3.5,    0,     0),
	F( 256000000,  P_MMPLL4,     3,    0,     0),
	F( 384000000,  P_MMPLL4,     2,    0,     0),
	F( 576000000,  P_MMPLL10,    1,    0,     0),
	{ }
};

static struct freq_tbl ftbl_csi_clk_src_vq[] = {
	F(  19200000,  P_XO,         1,    0,     0),
	F( 164571429,  P_MMPLL10,  3.5,    0,     0),
	F( 256000000,  P_MMPLL4,     3,    0,     0),
	F( 274290000,  P_MMPLL7,   3.5,    0,     0),
	F( 300000000,  P_GPLL0,      2,    0,     0),
	F( 384000000,  P_MMPLL4,     2,    0,     0),
	F( 576000000,  P_MMPLL10,    1,    0,     0),
	{ }
};

static struct clk_rcg2 csi0_clk_src = {
	.cmd_rcgr = 0x03090,
	.hid_width = 5,
	.parent_map = mmcc_parent_map_2,
	.freq_tbl = ftbl_csi_clk_src,
	.enable_safe_config = true,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "csi0_clk_src",
		.parent_names = mmcc_parent_names_2,
		.num_parents = ARRAY_SIZE(mmcc_parent_names_2),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP4(LOWER, 164571429, LOW, 256000000,
					NOMINAL, 384000000, HIGH, 576000000),
	},
};

static struct freq_tbl ftbl_vfe_clk_src[] = {
	F(  19200000,  P_XO,         1,    0,     0),
	F( 200000000,  P_GPLL0,      3,    0,     0),
	F( 300000000,  P_GPLL0,      2,    0,     0),
	F( 320000000,  P_MMPLL7,     3,    0,     0),
	F( 384000000,  P_MMPLL4,     2,    0,     0),
	F( 576000000,  P_MMPLL10,    1,    0,     0),
	F( 600000000,  P_GPLL0,      1,    0,     0),
	{ }
};

static struct freq_tbl ftbl_vfe_clk_src_vq[] = {
	F(  19200000,  P_XO,         1,    0,     0),
	F( 200000000,  P_GPLL0,      3,    0,     0),
	F( 300000000,  P_GPLL0,      2,    0,     0),
	F( 320000000,  P_MMPLL7,     3,    0,     0),
	F( 384000000,  P_MMPLL4,     2,    0,     0),
	F( 404000000,  P_MMPLL0,     2,    0,     0),
	F( 480000000,  P_MMPLL7,     2,    0,     0),
	F( 576000000,  P_MMPLL10,    1,    0,     0),
	F( 600000000,  P_GPLL0,      1,    0,     0),
	{ }
};

static struct clk_rcg2 vfe0_clk_src = {
	.cmd_rcgr = 0x03600,
	.hid_width = 5,
	.parent_map = mmcc_parent_map_2a,
	.freq_tbl = ftbl_vfe_clk_src,
	.enable_safe_config = true,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "vfe0_clk_src",
		.parent_names = mmcc_parent_names_2a,
		.num_parents = ARRAY_SIZE(mmcc_parent_names_2a),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP4(LOWER, 200000000, LOW, 384000000,
					NOMINAL, 576000000, HIGH, 600000000),
	},
};

static struct clk_rcg2 vfe1_clk_src = {
	.cmd_rcgr = 0x03620,
	.hid_width = 5,
	.parent_map = mmcc_parent_map_2a,
	.freq_tbl = ftbl_vfe_clk_src,
	.enable_safe_config = true,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "vfe1_clk_src",
		.parent_names = mmcc_parent_names_2a,
		.num_parents = ARRAY_SIZE(mmcc_parent_names_2a),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP4(LOWER, 200000000, LOW, 384000000,
					NOMINAL, 576000000, HIGH, 600000000),
	},
};

static struct freq_tbl ftbl_mdp_clk_src[] = {
	F(  19200000, P_XO,         1,    0,     0),
	F(  85714286, P_GPLL0,      7,    0,     0),
	F( 100000000, P_GPLL0,      6,    0,     0),
	F( 150000000, P_GPLL0,      4,    0,     0),
	F( 171428571, P_GPLL0,    3.5,    0,     0),
	F( 200000000, P_GPLL0,      3,    0,     0),
	F( 275000000, P_MMPLL5,     3,    0,     0),
	F( 300000000, P_GPLL0,      2,    0,     0),
	F( 330000000, P_MMPLL5,   2.5,    0,     0),
	F( 412500000, P_MMPLL5,     2,    0,     0),
	F( 550000000, P_MMPLL5,   1.5,    0,     0),
	{ }
};

static struct clk_rcg2 mdp_clk_src = {
	.cmd_rcgr = 0x02040,
	.hid_width = 5,
	.parent_map = mmcc_parent_map_3,
	.freq_tbl = ftbl_mdp_clk_src,
	.enable_safe_config = true,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "mdp_clk_src",
		.parent_names = mmcc_parent_names_3,
		.num_parents = ARRAY_SIZE(mmcc_parent_names_3),
		.ops = &clk_rcg2_ops,
		//.flags = CLK_GET_RATE_NOCACHE,
		VDD_DIG_FMAX_MAP4(LOWER, 171430000, LOW, 275000000,
					NOMINAL, 330000000, HIGH, 550000000),
	},
};

static struct freq_tbl ftbl_maxi_clk_src[] = {
	F(  19200000,   P_XO,         1,    0,     0),
	F(  75000000,   P_GPLL0_DIV,  4,    0,     0),
	F( 171428571,   P_GPLL0,    3.5,    0,     0),
	F( 323200000,   P_MMPLL0,   2.5,    0,     0),
	F( 406000000,   P_MMPLL1,     2,    0,     0),
	{ }
};

static struct clk_rcg2 maxi_clk_src = {
	.cmd_rcgr = 0x0F020,
	.hid_width = 5,
	.parent_map = mmcc_parent_map_4,
	.freq_tbl = ftbl_maxi_clk_src,
	.enable_safe_config = true,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "maxi_clk_src",
		.parent_names = mmcc_parent_names_4,
		.num_parents = ARRAY_SIZE(mmcc_parent_names_4),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP4(LOWER, 75000000, LOW, 171428571,
					NOMINAL, 323200000, HIGH, 406000000),
	},
};

static struct freq_tbl ftbl_cpp_clk_src[] = {
	F(  19200000,     P_XO,      1,    0,     0),
	F( 100000000,  P_GPLL0,      6,    0,     0),
	F( 200000000,  P_GPLL0,      3,    0,     0),
#if defined(CONFIG_SONY_CAM_V4L2)
	F( 384000000,  P_MMPLL4,     2,    0,     0),
#endif
	F( 576000000,  P_MMPLL10,    1,    0,     0),
	F( 600000000,  P_GPLL0,      1,    0,     0),
	{ }
};

static struct freq_tbl ftbl_cpp_clk_src_vq[] = {
	F(  19200000,     P_XO,      1,    0,     0),
	F( 100000000,  P_GPLL0,      6,    0,     0),
	F( 200000000,  P_GPLL0,      3,    0,     0),
	F( 384000000,  P_MMPLL4,     2,    0,     0),
	F( 404000000,  P_MMPLL0,     2,    0,     0),
	F( 480000000,  P_MMPLL7,     2,    0,     0),
	F( 576000000,  P_MMPLL10,    1,    0,     0),
	F( 600000000,  P_GPLL0,      1,    0,     0),
	{ }
};

static struct clk_rcg2 cpp_clk_src = {
	.cmd_rcgr = 0x03640,
	.hid_width = 5,
	.parent_map = mmcc_parent_map_2,
	.freq_tbl = ftbl_cpp_clk_src,
	.enable_safe_config = true,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "cpp_clk_src",
		.parent_names = mmcc_parent_names_2,
		.num_parents = ARRAY_SIZE(mmcc_parent_names_2),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP4(LOWER, 100000000, LOW, 200000000,
					NOMINAL, 576000000, HIGH, 600000000),
	},
};

static struct freq_tbl ftbl_jpeg0_clk_src[] = {
	F(  19200000,    P_XO,      1,    0,     0),
	F(  75000000, P_GPLL0,      8,    0,     0),
	F( 150000000, P_GPLL0,      4,    0,     0),
	F( 480000000, P_MMPLL7,     2,    0,     0),
	{ }
};

static struct freq_tbl ftbl_jpeg0_clk_src_vq[] = {
	F(  19200000,    P_XO,      1,    0,     0),
	F(  75000000, P_GPLL0,      8,    0,     0),
	F( 150000000, P_GPLL0,      4,    0,     0),
	F( 320000000, P_MMPLL7,     3,    0,     0),
	F( 480000000, P_MMPLL7,     2,    0,     0),
	{ }
};

static struct clk_rcg2 jpeg0_clk_src = {
	.cmd_rcgr = 0x03500,
	.hid_width = 5,
	.parent_map = mmcc_parent_map_6,
	.freq_tbl = ftbl_jpeg0_clk_src,
	.enable_safe_config = true,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "jpeg0_clk_src",
		.parent_names = mmcc_parent_names_6,
		.num_parents = ARRAY_SIZE(mmcc_parent_names_6),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP3(LOWER, 75000000, LOW, 150000000,
					NOMINAL, 480000000),
	},
};

static struct freq_tbl ftbl_rot_clk_src[] = {
	F( 171428571, P_GPLL0,    3.5,    0,     0),
	F( 275000000, P_MMPLL5,     3,    0,     0),
	F( 300000000, P_GPLL0,      2,    0,     0),
	F( 330000000, P_MMPLL5,   2.5,    0,     0),
	F( 412500000, P_MMPLL5,     2,    0,     0),
	F( 550000000, P_MMPLL5,   1.5,    0,     0),
	{ }
};

static struct clk_rcg2 rot_clk_src = {
	.cmd_rcgr = 0x021A0,
	.hid_width = 5,
	.parent_map = mmcc_parent_map_3,
	.freq_tbl = ftbl_rot_clk_src,
	.enable_safe_config = true,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "rot_clk_src",
		.parent_names = mmcc_parent_names_3,
		.num_parents = ARRAY_SIZE(mmcc_parent_names_3),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP5(
			LOWER, 171428571,
			LOW, 275000000,
			LOW_L1, 300000000,
			NOMINAL, 330000000,
			HIGH, 550000000),
	},
};

static struct freq_tbl ftbl_video_core_clk_src[] = {
	F( 100000000, P_GPLL0,      6,    0,     0),
	F( 186000000, P_MMPLL3,     5,    0,     0),
	F( 360000000, P_MMPLL6,     2,    0,     0),
	F( 465000000, P_MMPLL3,     2,    0,     0),
	{ }
};

static struct freq_tbl ftbl_video_core_clk_src_vq[] = {
	F( 200000000, P_GPLL0,      3,    0,     0),
	F( 269330000, P_MMPLL0,     3,    0,     0),
	F( 355200000, P_MMPLL6,   2.5,    0,     0),
	F( 444000000, P_MMPLL6,     2,    0,     0),
	F( 533000000, P_MMPLL3,     2,    0,     0),
	{ }
};

static struct clk_rcg2 video_core_clk_src = {
	.cmd_rcgr = 0x01000,
	.hid_width = 5,
	.parent_map = mmcc_parent_map_7a,
	.freq_tbl = ftbl_video_core_clk_src,
	.flags = FORCE_ENABLE_RCG,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "video_core_clk_src",
		.parent_names = mmcc_parent_names_7a,
		.num_parents = ARRAY_SIZE(mmcc_parent_names_7a),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP4(LOWER, 100000000, LOW, 186000000,
					NOMINAL, 360000000, HIGH, 465000000),
	},
};

static struct freq_tbl ftbl_csiphy_clk_src[] = {
	F(  19200000,    P_XO,       1,    0,     0),
	F(  164570000,   P_MMPLL10,  3.5,  0,     0),
	F(  256000000,   P_MMPLL4,   3,    0,     0),
	F(  384000000,   P_MMPLL4,   2,    0,     0),
	{ }
};

static struct freq_tbl ftbl_csiphy_clk_src_vq[] = {
	F(  19200000,    P_XO,       1,    0,     0),
	F(  164570000,   P_MMPLL10,  3.5,  0,     0),
	F(  256000000,   P_MMPLL4,   3,    0,     0),
	F(  274290000,   P_MMPLL7,   3.5,  0,     0),
	F(  300000000,   P_GPLL0,    2,    0,     0),
	F(  384000000,   P_MMPLL4,   2,    0,     0),
	{ }
};

static struct clk_rcg2 csiphy_clk_src = {
	.cmd_rcgr = 0x03800,
	.hid_width = 5,
	.parent_map = mmcc_parent_map_2,
	.freq_tbl = ftbl_csiphy_clk_src,
	.enable_safe_config = true,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "csiphy_clk_src",
		.parent_names = mmcc_parent_names_2,
		.num_parents = ARRAY_SIZE(mmcc_parent_names_2),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP3(LOWER, 164570000, LOW, 256000000,
					NOMINAL, 384000000),
	},
};

static struct clk_rcg2 csi1_clk_src = {
	.cmd_rcgr = 0x03100,
	.hid_width = 5,
	.parent_map = mmcc_parent_map_2,
	.freq_tbl = ftbl_csi_clk_src,
	.enable_safe_config = true,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "csi1_clk_src",
		.parent_names = mmcc_parent_names_2,
		.num_parents = ARRAY_SIZE(mmcc_parent_names_2),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP4(LOWER, 164570000, LOW, 256000000,
					NOMINAL, 384000000, HIGH, 576000000),
	},
};

static struct clk_rcg2 csi2_clk_src = {
	.cmd_rcgr = 0x03160,
	.hid_width = 5,
	.parent_map = mmcc_parent_map_2,
	.freq_tbl = ftbl_csi_clk_src,
	.enable_safe_config = true,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "csi2_clk_src",
		.parent_names = mmcc_parent_names_2,
		.num_parents = ARRAY_SIZE(mmcc_parent_names_2),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP4(LOWER, 164570000, LOW, 256000000,
					NOMINAL, 384000000, HIGH, 576000000),
	},
};

static struct clk_rcg2 csi3_clk_src = {
	.cmd_rcgr = 0x031C0,
	.hid_width = 5,
	.parent_map = mmcc_parent_map_2,
	.freq_tbl = ftbl_csi_clk_src,
	.enable_safe_config = true,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "csi3_clk_src",
		.parent_names = mmcc_parent_names_2,
		.num_parents = ARRAY_SIZE(mmcc_parent_names_2),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP4(LOWER, 164570000, LOW, 256000000,
					NOMINAL, 384000000, HIGH, 576000000),
	},
};

static struct freq_tbl ftbl_fd_core_clk_src[] = {
	F( 19200000,   P_XO,         1,    0,     0),
	F( 100000000,  P_GPLL0,      6,    0,     0),
	F( 200000000,  P_GPLL0,      3,    0,     0),
	F( 576000000,  P_MMPLL10,    1,    0,     0),
	{ }
};

static struct freq_tbl ftbl_fd_core_clk_src_vq[] = {
	F( 19200000,   P_XO,         1,    0,     0),
	F( 100000000,  P_GPLL0,      6,    0,     0),
	F( 200000000,  P_GPLL0,      3,    0,     0),
	F( 404000000,  P_MMPLL0,     2,    0,     0),
	F( 480000000,  P_MMPLL7,     2,    0,     0),
	F( 576000000,  P_MMPLL10,    1,    0,     0),
	{ }
};

static struct clk_rcg2 fd_core_clk_src = {
	.cmd_rcgr = 0x03B00,
	.hid_width = 5,
	.parent_map = mmcc_parent_map_8,
	.freq_tbl = ftbl_fd_core_clk_src,
	.enable_safe_config = true,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "fd_core_clk_src",
		.parent_names = mmcc_parent_names_8,
		.num_parents = ARRAY_SIZE(mmcc_parent_names_8),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP3(LOWER, 100000000, LOW, 200000000,
					NOMINAL, 576000000),
	},
};

static struct clk_rcg2 byte0_clk_src = {
	.cmd_rcgr = 0x02120,
	.hid_width = 5,
	.parent_map = disp_cc_parent_map_0,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "byte0_clk_src",
		.parent_names = disp_cc_parent_names_0,
		.num_parents = ARRAY_SIZE(disp_cc_parent_names_0),
		.flags = CLK_SET_RATE_PARENT | CLK_GET_RATE_NOCACHE,
		.ops = &clk_byte2_ops,
		VDD_DIG_FMAX_MAP3(LOWER, 131250000, LOW, 210000000,
						NOMINAL, 312500000),
	},
};

static struct clk_rcg2 byte1_clk_src = {
	.cmd_rcgr = 0x02140,
	.hid_width = 5,
	.parent_map = disp_cc_parent_map_0,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "byte1_clk_src",
		.parent_names = disp_cc_parent_names_0,
		.num_parents = ARRAY_SIZE(disp_cc_parent_names_0),
		.flags = CLK_SET_RATE_PARENT | CLK_GET_RATE_NOCACHE,
		.ops = &clk_byte2_ops,
		VDD_DIG_FMAX_MAP3(LOWER, 131250000, LOW, 210000000,
						NOMINAL, 312500000),
	},
};

static struct clk_rcg2 pclk0_clk_src = {
	.cmd_rcgr = 0x02000,
	.mnd_width = 8,
	.hid_width = 5,
	.parent_map = disp_cc_parent_map_4,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "pclk0_clk_src",
		.parent_names = disp_cc_parent_names_4,
		.num_parents = ARRAY_SIZE(disp_cc_parent_names_4),
		.ops = &clk_pixel_ops,
		.flags = CLK_SET_RATE_PARENT | CLK_GET_RATE_NOCACHE,
		VDD_DIG_FMAX_MAP3(LOWER, 175000000, LOW, 280000000,
						NOMINAL, 416670000),
	},
};

static struct clk_rcg2 pclk1_clk_src = {
	.cmd_rcgr = 0x02020,
	.mnd_width = 8,
	.hid_width = 5,
	.parent_map = disp_cc_parent_map_4,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "pclk1_clk_src",
		.parent_names = disp_cc_parent_names_4,
		.num_parents = ARRAY_SIZE(disp_cc_parent_names_4),
		.ops = &clk_pixel_ops,
		.flags = CLK_SET_RATE_PARENT | CLK_GET_RATE_NOCACHE,
		VDD_DIG_FMAX_MAP3(LOWER, 175000000, LOW, 280000000,
						NOMINAL, 416670000),
	},
};

static struct freq_tbl ftbl_video_subcore_clk_src[] = {
	F( 100000000, P_GPLL0,      6,    0,     0),
	F( 186000000, P_MMPLL3,     5,    0,     0),
	F( 360000000, P_MMPLL6,     2,    0,     0),
	F( 465000000, P_MMPLL3,     2,    0,     0),
	{ }
};

static struct freq_tbl ftbl_video_subcore_clk_src_vq[] = {
	F( 200000000, P_GPLL0,      3,    0,     0),
	F( 269330000, P_MMPLL0,     3,    0,     0),
	F( 355200000, P_MMPLL6,   2.5,    0,     0),
	F( 444000000, P_MMPLL6,     2,    0,     0),
	F( 533000000, P_MMPLL3,     2,    0,     0),
	{ }
};

static struct clk_rcg2 video_subcore0_clk_src = {
	.cmd_rcgr = 0x01060,
	.mnd_width = 0,
	.hid_width = 5,
	.parent_map = mmcc_parent_map_7,
	.freq_tbl = ftbl_video_subcore_clk_src,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "video_subcore0_clk_src",
		.parent_names = mmcc_parent_names_7,
		.num_parents = ARRAY_SIZE(mmcc_parent_names_7),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP4(LOWER, 100000000, LOW, 186000000,
					NOMINAL, 360000000, HIGH, 465000000),
	},
};

static struct clk_rcg2 video_subcore1_clk_src = {
	.cmd_rcgr = 0x01080,
	.mnd_width = 0,
	.hid_width = 5,
	.parent_map = mmcc_parent_map_7,
	.freq_tbl = ftbl_video_subcore_clk_src,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "video_subcore1_clk_src",
		.parent_names = mmcc_parent_names_7,
		.num_parents = ARRAY_SIZE(mmcc_parent_names_7),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP4(LOWER, 100000000, LOW, 186000000,
					NOMINAL, 360000000, HIGH, 465000000),
	},
};

static struct freq_tbl ftbl_cci_clk_src[] = {
	F(  37500000,   P_GPLL0,     16,    0,     0),
	F(  50000000,   P_GPLL0,     12,    0,     0),
	F( 100000000,   P_GPLL0,      6,    0,     0),
	{ }
};

static struct clk_rcg2 cci_clk_src = {
	.cmd_rcgr = 0x03300,
	.mnd_width = 8,
	.hid_width = 5,
	.parent_map = mmcc_parent_map_gcc_0,
	.freq_tbl = ftbl_cci_clk_src,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "cci_clk_src",
		.parent_names = mmcc_parent_names_gcc_0,
		.num_parents = ARRAY_SIZE(mmcc_parent_names_gcc_0),
		.ops = &clk_rcg2_ops,
		.flags = CLK_GET_RATE_NOCACHE,
		VDD_DIG_FMAX_MAP3(LOWER, 37500000, LOW, 50000000,
					NOMINAL, 100000000),
	},
};

static struct freq_tbl ftbl_camss_gp_clk_src[] = {
	F(     10000,	P_XO,        16,    1,   120),
	F(     24000,	P_XO,        16,    1,    50),
	F(   6000000,   P_GPLL0,     10,    1,    10),
	F(  12000000,   P_GPLL0,     10,    1,     5),
	F(  13000000,   P_GPLL0,      4,   13,   150),
	F(  24000000,   P_GPLL0,      5,    1,     5),
	{ }
};

static struct clk_rcg2 camss_gp0_clk_src = {
	.cmd_rcgr = 0x03420,
	.mnd_width = 8,
	.hid_width = 5,
	.parent_map = mmcc_parent_map_gcc_0,
	.freq_tbl = ftbl_camss_gp_clk_src,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "camss_gp0_clk_src",
		.parent_names = mmcc_parent_names_gcc_0,
		.num_parents = ARRAY_SIZE(mmcc_parent_names_gcc_0),
		.ops = &clk_rcg2_ops,
		.flags = CLK_GET_RATE_NOCACHE,
		VDD_DIG_FMAX_MAP3(LOWER, 50000000, LOW, 100000000,
					NOMINAL, 200000000),
	},
};

static struct clk_rcg2 camss_gp1_clk_src = {
	.cmd_rcgr = 0x03450,
	.mnd_width = 8,
	.hid_width = 5,
	.parent_map = mmcc_parent_map_gcc_0,
	.freq_tbl = ftbl_camss_gp_clk_src,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "camss_gp1_clk_src",
		.parent_names = mmcc_parent_names_gcc_0,
		.num_parents = ARRAY_SIZE(mmcc_parent_names_gcc_0),
		.ops = &clk_rcg2_ops,
		.flags = CLK_GET_RATE_NOCACHE,
		VDD_DIG_FMAX_MAP3(LOWER, 50000000, LOW, 100000000,
					NOMINAL, 200000000),
	},
};

static struct freq_tbl ftbl_mclk_clk_src[] = {
	F(   4800000,	P_XO,         4,    0,     0),
	F(   6000000,   P_GPLL0_DIV, 10,    1,     5),
	F(   8000000,   P_GPLL0_DIV,  1,    2,    75),
	F(   9600000,	P_XO,         2,    0,     0),
	F(  16666667,   P_GPLL0_DIV,  2,    1,     9),
	F(  19200000,	P_XO,         1,    0,     0),
	F(  24000000,   P_GPLL0_DIV,  1,    2,    25),
	F(  33333333,   P_GPLL0_DIV,  1,    1,     9),
	F(  48000000,   P_GPLL0,      1,    2,    25),
	F(  66666667,   P_GPLL0,      1,    1,     9),
	{ }
};

static struct clk_rcg2 mclk0_clk_src = {
	.cmd_rcgr = 0x03360,
	.mnd_width = 8,
	.hid_width = 5,
	.parent_map = mmcc_parent_map_gcc_1,
	.enable_safe_config = true,
	.freq_tbl = ftbl_mclk_clk_src,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "mclk0_clk_src",
		.parent_names = mmcc_parent_names_gcc_1,
		.num_parents = ARRAY_SIZE(mmcc_parent_names_gcc_1),
		.ops = &clk_rcg2_ops,
		.flags = CLK_GET_RATE_NOCACHE,
		VDD_DIG_FMAX_MAP3(LOWER, 33333333, LOW, 66666667,
					NOMINAL, 68571429),
	},
};

static struct clk_rcg2 mclk1_clk_src = {
	.cmd_rcgr = 0x03390,
	.mnd_width = 8,
	.hid_width = 5,
	.parent_map = mmcc_parent_map_gcc_1,
	.enable_safe_config = true,
	.freq_tbl = ftbl_mclk_clk_src,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "mclk1_clk_src",
		.parent_names = mmcc_parent_names_gcc_1,
		.num_parents = ARRAY_SIZE(mmcc_parent_names_gcc_1),
		.ops = &clk_rcg2_ops,
		.flags = CLK_GET_RATE_NOCACHE,
		VDD_DIG_FMAX_MAP3(LOWER, 33333333, LOW, 66666667,
					NOMINAL, 68571429),
	},
};

static struct clk_rcg2 mclk2_clk_src = {
	.cmd_rcgr = 0x033C0,
	.mnd_width = 8,
	.hid_width = 5,
	.parent_map = mmcc_parent_map_gcc_1,
	.enable_safe_config = true,
	.freq_tbl = ftbl_mclk_clk_src,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "mclk2_clk_src",
		.parent_names = mmcc_parent_names_gcc_1,
		.num_parents = ARRAY_SIZE(mmcc_parent_names_gcc_1),
		.ops = &clk_rcg2_ops,
		.flags = CLK_GET_RATE_NOCACHE,
		VDD_DIG_FMAX_MAP3(LOWER, 33333333, LOW, 66666667,
					NOMINAL, 68571429),
	},
};

static struct clk_rcg2 mclk3_clk_src = {
	.cmd_rcgr = 0x033F0,
	.mnd_width = 8,
	.hid_width = 5,
	.parent_map = mmcc_parent_map_gcc_1,
	.enable_safe_config = true,
	.freq_tbl = ftbl_mclk_clk_src,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "mclk3_clk_src",
		.parent_names = mmcc_parent_names_gcc_1,
		.num_parents = ARRAY_SIZE(mmcc_parent_names_gcc_1),
		.ops = &clk_rcg2_ops,
		.flags = CLK_GET_RATE_NOCACHE,
		VDD_DIG_FMAX_MAP3(LOWER, 33333333, LOW, 66666667,
					NOMINAL, 68571429),
	},
};

static struct freq_tbl ftbl_csiphytimer_clk_src[] = {
	F(  19200000, P_XO,         1,    0,     0),
	F( 200000000, P_GPLL0,      3,    0,     0),
	F( 269333333, P_MMPLL0,     3,    0,     0),
	{ }
};

static struct clk_rcg2 csi0phytimer_clk_src = {
	.cmd_rcgr = 0x03000,
	.hid_width = 5,
	.parent_map = mmcc_parent_map_1,
	.freq_tbl = ftbl_csiphytimer_clk_src,
	.enable_safe_config = true,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "csi0phytimer_clk_src",
		.parent_names = mmcc_parent_names_1,
		.num_parents = ARRAY_SIZE(mmcc_parent_names_1),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP3(LOWER, 100000000, LOW, 200000000,
					NOMINAL, 269333333),
	},
};

static struct clk_rcg2 csi1phytimer_clk_src = {
	.cmd_rcgr = 0x03030,
	.hid_width = 5,
	.parent_map = mmcc_parent_map_1,
	.freq_tbl = ftbl_csiphytimer_clk_src,
	.enable_safe_config = true,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "csi1phytimer_clk_src",
		.parent_names = mmcc_parent_names_1,
		.num_parents = ARRAY_SIZE(mmcc_parent_names_1),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP3(LOWER, 100000000, LOW, 200000000,
					NOMINAL, 269333333),
	},
};

static struct clk_rcg2 csi2phytimer_clk_src = {
	.cmd_rcgr = 0x03060,
	.hid_width = 5,
	.parent_map = mmcc_parent_map_1,
	.freq_tbl = ftbl_csiphytimer_clk_src,
	.enable_safe_config = true,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "csi2phytimer_clk_src",
		.parent_names = mmcc_parent_names_1,
		.num_parents = ARRAY_SIZE(mmcc_parent_names_1),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP3(LOWER, 100000000, LOW, 200000000,
					NOMINAL, 269333333),
	},
};

static struct freq_tbl ftbl_dp_gtc_clk_src[] = {
	F( 300000000, P_GPLL0,      2,    0,     0),
	{ }
};

static struct clk_rcg2 dp_gtc_clk_src = {
	.cmd_rcgr = 0x02280,
	.hid_width = 5,
	.parent_map = mmcc_parent_map_gcc_0,
	.freq_tbl = ftbl_dp_gtc_clk_src,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "dp_gtc_clk_src",
		.parent_names = mmcc_parent_names_gcc_0,
		.num_parents = ARRAY_SIZE(mmcc_parent_names_gcc_0),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP2(LOWER, 40000000, LOW, 300000000),
	},
};

static struct clk_rcg2 esc0_clk_src = {
	.cmd_rcgr = 0x02160,
	.hid_width = 5,
	.parent_map = disp_cc_parent_map_0,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "esc0_clk_src",
		.parent_names = disp_cc_parent_names_0,
		.num_parents = ARRAY_SIZE(disp_cc_parent_names_0),
		.ops = &clk_esc_ops,
		VDD_DIG_FMAX_MAP1(LOWER, 19200000),
	},
};

static struct clk_rcg2 esc1_clk_src = {
	.cmd_rcgr = 0x02180,
	.hid_width = 5,
	.parent_map = disp_cc_parent_map_0,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "esc1_clk_src",
		.parent_names = disp_cc_parent_names_0,
		.num_parents = ARRAY_SIZE(disp_cc_parent_names_0),
		.ops = &clk_esc_ops,
		VDD_DIG_FMAX_MAP1(LOWER, 19200000),
	},
};

static struct freq_tbl extpclk_freq_tbl[] = {
	{ .src = P_HDMI_PLL },
	{ }
};

static struct clk_rcg2 extpclk_clk_src = {
	.cmd_rcgr = 0x02060,
	.hid_width = 5,
	.parent_map = disp_cc_parent_map_5,
	.freq_tbl = extpclk_freq_tbl,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "extpclk_clk_src",
		.parent_names = disp_cc_parent_names_5,
		.num_parents = ARRAY_SIZE(disp_cc_parent_names_5),
		.ops = &clk_byte_ops,
		.flags = CLK_SET_RATE_PARENT,
		VDD_DIG_FMAX_MAP3(LOWER, 150000000, LOW, 300000000,
					NOMINAL, 600000000),
	},
};

static struct freq_tbl ftbl_hdmi_clk_src[] = {
	F(  19200000,      P_XO,         1,    0,     0),
	{ }
};

static struct clk_rcg2 hdmi_clk_src = {
	.cmd_rcgr = 0x02100,
	.hid_width = 5,
	.parent_map = mmcc_parent_map_gcc_1,
	.freq_tbl = ftbl_hdmi_clk_src,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "hdmi_clk_src",
		.parent_names = mmcc_parent_names_gcc_1,
		.num_parents = ARRAY_SIZE(mmcc_parent_names_gcc_1),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP2(LOWER, 19200000, NOMINAL, 19200000),
	},
};

static struct freq_tbl ftbl_vsync_clk_src[] = {
	F(  19200000,      P_XO,         1,    0,     0),
	{ }
};

static struct clk_rcg2 vsync_clk_src = {
	.cmd_rcgr = 0x02080,
	.hid_width = 5,
	.parent_map = disp_cc_parent_map_2,
	.freq_tbl = ftbl_vsync_clk_src,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "vsync_clk_src",
		.parent_names = disp_cc_parent_names_2,
		.num_parents = ARRAY_SIZE(disp_cc_parent_names_2),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP2(LOWER, 19200000, NOMINAL, 19200000),
	},
};

static struct freq_tbl ftbl_dp_aux_clk_src[] = {
	F(  19200000,      P_XO,         1,    0,     0),
	{ }
};

static struct clk_rcg2 dp_aux_clk_src = {
	.cmd_rcgr = 0x02260,
	.hid_width = 5,
	.parent_map = disp_cc_parent_map_2,
	.freq_tbl = ftbl_dp_aux_clk_src,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "dp_aux_clk_src",
		.parent_names = disp_cc_parent_names_2,
		.num_parents = ARRAY_SIZE(disp_cc_parent_names_2),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP2(LOWER, 19200000, NOMINAL, 19200000),
	},
};

static struct clk_rcg2 dp_pixel_clk_src = {
	.cmd_rcgr = 0x02240,
	.mnd_width = 8,
	.hid_width = 5,
	.parent_map = disp_cc_parent_map_1,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "dp_pixel_clk_src",
		.parent_names = disp_cc_parent_names_1,
		.num_parents = ARRAY_SIZE(disp_cc_parent_names_1),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP3(LOWER, 154000000, LOW, 337500000,
					NOMINAL, 675000000),
	},
};

static struct freq_tbl ftbl_dp_link_clk_src[] = {
	F_SLEW( 162000, P_DP_PHY_PLL_LINK_CLK,   2,   0,   0,  324000),
	F_SLEW( 270000, P_DP_PHY_PLL_LINK_CLK,   2,   0,   0,  540000),
	F_SLEW( 540000, P_DP_PHY_PLL_LINK_CLK,   2,   0,   0, 1080000),
	{ }
};

static struct clk_rcg2 dp_link_clk_src = {
	.cmd_rcgr = 0x02200,
	.hid_width = 5,
	.parent_map = disp_cc_parent_map_1,
	.freq_tbl = ftbl_dp_link_clk_src,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "dp_link_clk_src",
		.parent_names = disp_cc_parent_names_1,
		.num_parents = ARRAY_SIZE(disp_cc_parent_names_1),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP3(LOWER, 162000, LOW, 270000,
					NOMINAL, 540000),
	},
};

/*
 * Current understanding is that the DP PLL is going to be configured by using
 * the set_rate ops for the dp_link_clk_src and dp_pixel_clk_src. When set_rate
 * is called on this RCG, the rate call never makes it to the external DP
 * clocks.
 */
static struct freq_tbl ftbl_dp_crypto_clk_src[] = {
	F( 101250, P_DP_PHY_PLL_LINK_CLK,   1,   5,   16),
	F( 168750, P_DP_PHY_PLL_LINK_CLK,   1,   5,   16),
	F( 337500, P_DP_PHY_PLL_LINK_CLK,   1,   5,   16),
	{ }
};

static struct clk_rcg2 dp_crypto_clk_src = {
	.cmd_rcgr = 0x02220,
	.mnd_width = 8,
	.hid_width = 5,
	.parent_map = disp_cc_parent_map_1,
	.freq_tbl = ftbl_dp_crypto_clk_src,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "dp_crypto_clk_src",
		.parent_names = disp_cc_parent_names_1,
		.num_parents = ARRAY_SIZE(disp_cc_parent_names_1),
		.ops = &clk_rcg2_ops,
		.flags = CLK_GET_RATE_NOCACHE,
		VDD_DIG_FMAX_MAP3(LOWER, 101250, LOW, 168750,
					NOMINAL, 337500),
	},
};

static struct clk_branch mmss_bimc_smmu_ahb_clk = {
	.halt_reg = 0x0E004,
	.halt_check = BRANCH_HALT,
	.hwcg_reg = 0x0E004,
	.hwcg_bit = 1,
	.clkr = {
		.enable_reg = 0x0E004,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_bimc_smmu_ahb_clk",
			.parent_names = (const char *[]){
				"ahb_clk_src",
			},
			.flags = CLK_ENABLE_HAND_OFF,
			.num_parents = 1,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_bimc_smmu_axi_clk = {
	.halt_reg = 0x0E008,
	.halt_check = BRANCH_HALT,
	.hwcg_reg = 0x0E008,
	.hwcg_bit = 1,
	.clkr = {
		.enable_reg = 0x0E008,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_bimc_smmu_axi_clk",
			.flags = CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_snoc_dvm_axi_clk = {
	.halt_reg = 0x0E040,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x0E040,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_snoc_dvm_axi_clk",
			.flags = CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_camss_ahb_clk = {
	.halt_reg = 0x0348C,
	.halt_check = BRANCH_HALT,
	.hwcg_reg = 0x0348C,
	.hwcg_bit = 1,
	.clkr = {
		.enable_reg = 0x0348C,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_camss_ahb_clk",
			.parent_names = (const char *[]){
				"ahb_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_camss_cci_ahb_clk = {
	.halt_reg = 0x03348,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x03348,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_camss_cci_ahb_clk",
			.parent_names = (const char *[]){
				"ahb_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_camss_cci_clk = {
	.halt_reg = 0x03344,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x03344,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_camss_cci_clk",
			.parent_names = (const char*[]) {
				"cci_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_camss_cpp_ahb_clk = {
	.halt_reg = 0x036B4,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x036B4,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_camss_cpp_ahb_clk",
			.parent_names = (const char *[]){
				"ahb_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_camss_cpp_clk = {
	.halt_reg = 0x036B0,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x036B0,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_camss_cpp_clk",
			.parent_names = (const char*[]) {
				"cpp_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_camss_cpp_axi_clk = {
	.halt_reg = 0x036C4,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x036C4,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_camss_cpp_axi_clk",
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_camss_cpp_vbif_ahb_clk = {
	.halt_reg = 0x036C8,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x036C8,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_camss_cpp_vbif_ahb_clk",
			.parent_names = (const char *[]){
				"ahb_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_camss_cphy_csid0_clk = {
	.halt_reg = 0x03730,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x03730,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_camss_cphy_csid0_clk",
			.parent_names = (const char*[]) {
				"csiphy_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_camss_csi0_ahb_clk = {
	.halt_reg = 0x030BC,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x030BC,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_camss_csi0_ahb_clk",
			.parent_names = (const char *[]){
				"ahb_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_camss_csi0_clk = {
	.halt_reg = 0x030B4,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x030B4,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_camss_csi0_clk",
			.parent_names = (const char*[]) {
				"csi0_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_camss_csi0pix_clk = {
	.halt_reg = 0x030E4,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x030E4,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_camss_csi0pix_clk",
			.parent_names = (const char*[]) {
				"csi0_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_camss_csi0rdi_clk = {
	.halt_reg = 0x030D4,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x030D4,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_camss_csi0rdi_clk",
			.parent_names = (const char*[]) {
				"csi0_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_camss_cphy_csid1_clk = {
	.halt_reg = 0x03734,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x03734,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_camss_cphy_csid1_clk",
			.parent_names = (const char*[]) {
				"csiphy_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_camss_csi1_ahb_clk = {
	.halt_reg = 0x03128,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x03128,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_camss_csi1_ahb_clk",
			.parent_names = (const char *[]){
				"ahb_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_camss_csi1_clk = {
	.halt_reg = 0x03124,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x03124,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_camss_csi1_clk",
			.parent_names = (const char*[]) {
				"csi1_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_camss_csi1pix_clk = {
	.halt_reg = 0x03154,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x03154,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_camss_csi1pix_clk",
			.parent_names = (const char*[]) {
				"csi1_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_camss_csi1rdi_clk = {
	.halt_reg = 0x03144,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x03144,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_camss_csi1rdi_clk",
			.parent_names = (const char*[]) {
				"csi1_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_camss_cphy_csid2_clk = {
	.halt_reg = 0x03738,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x03738,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_camss_cphy_csid2_clk",
			.parent_names = (const char*[]) {
				"csiphy_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_camss_csi2_ahb_clk = {
	.halt_reg = 0x03188,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x03188,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_camss_csi2_ahb_clk",
			.parent_names = (const char *[]){
				"ahb_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_camss_csi2_clk = {
	.halt_reg = 0x03184,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x03184,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_camss_csi2_clk",
			.parent_names = (const char*[]) {
				"csi2_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_camss_csi2pix_clk = {
	.halt_reg = 0x031B4,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x031B4,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_camss_csi2pix_clk",
			.parent_names = (const char*[]) {
				"csi2_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_camss_csi2rdi_clk = {
	.halt_reg = 0x031A4,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x031A4,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_camss_csi2rdi_clk",
			.parent_names = (const char*[]) {
				"csi2_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_camss_cphy_csid3_clk = {
	.halt_reg = 0x0373C,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x0373C,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_camss_cphy_csid3_clk",
			.parent_names = (const char*[]) {
				"csiphy_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_camss_csi3_ahb_clk = {
	.halt_reg = 0x031E8,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x031E8,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_camss_csi3_ahb_clk",
			.parent_names = (const char *[]){
				"ahb_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_camss_csi3_clk = {
	.halt_reg = 0x031E4,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x031E4,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_camss_csi3_clk",
			.parent_names = (const char*[]) {
				"csi3_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_camss_csi3pix_clk = {
	.halt_reg = 0x03214,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x03214,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_camss_csi3pix_clk",
			.parent_names = (const char*[]) {
				"csi3_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_camss_csi3rdi_clk = {
	.halt_reg = 0x03204,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x03204,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_camss_csi3rdi_clk",
			.parent_names = (const char*[]) {
				"csi3_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_camss_csi_vfe0_clk = {
	.halt_reg = 0x03704,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x03704,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_camss_csi_vfe0_clk",
			.parent_names = (const char*[]) {
				"vfe0_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_camss_csi_vfe1_clk = {
	.halt_reg = 0x03714,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x03714,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_camss_csi_vfe1_clk",
			.parent_names = (const char*[]) {
				"vfe1_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_camss_csiphy0_clk = {
	.halt_reg = 0x03740,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x03740,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_camss_csiphy0_clk",
			.parent_names = (const char*[]) {
				"csiphy_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_camss_csiphy1_clk = {
	.halt_reg = 0x03744,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x03744,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_camss_csiphy1_clk",
			.parent_names = (const char*[]) {
				"csiphy_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_camss_csiphy2_clk = {
	.halt_reg = 0x03748,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x03748,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_camss_csiphy2_clk",
			.parent_names = (const char*[]) {
				"csiphy_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_fd_ahb_clk = {
	.halt_reg = 0x03B74,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x03B74,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_fd_ahb_clk",
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_fd_core_clk = {
	.halt_reg = 0x03B68,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x03B68,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_fd_core_clk",
			.parent_names = (const char*[]) {
				"fd_core_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_fd_core_uar_clk = {
	.halt_reg = 0x03B6C,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x03B6C,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_fd_core_uar_clk",
			.parent_names = (const char*[]) {
				"fd_core_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_camss_gp0_clk = {
	.halt_reg = 0x03444,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x03444,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_camss_gp0_clk",
			.parent_names = (const char*[]) {
				"camss_gp0_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_camss_gp1_clk = {
	.halt_reg = 0x03474,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x03474,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_camss_gp1_clk",
			.parent_names = (const char*[]) {
				"camss_gp1_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_camss_ispif_ahb_clk = {
	.halt_reg = 0x03224,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x03224,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_camss_ispif_ahb_clk",
			.parent_names = (const char *[]){
				"ahb_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_camss_jpeg0_clk = {
	.halt_reg = 0x035A8,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x035A8,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_camss_jpeg0_clk",
			.parent_names = (const char*[]) {
				"jpeg0_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static DEFINE_CLK_VOTER(mmss_camss_jpeg0_vote_clk, mmss_camss_jpeg0_clk, 0);
static DEFINE_CLK_VOTER(mmss_camss_jpeg0_dma_vote_clk,
					mmss_camss_jpeg0_clk, 0);

static struct clk_branch mmss_camss_jpeg_ahb_clk = {
	.halt_reg = 0x035B4,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x035B4,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_camss_jpeg_ahb_clk",
			.parent_names = (const char *[]){
				"ahb_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_camss_jpeg_axi_clk = {
	.halt_reg = 0x035B8,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x035B8,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_camss_jpeg_axi_clk",
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_camss_mclk0_clk = {
	.halt_reg = 0x03384,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x03384,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_camss_mclk0_clk",
			.parent_names = (const char*[]) {
				"mclk0_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_camss_mclk1_clk = {
	.halt_reg = 0x033B4,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x033B4,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_camss_mclk1_clk",
			.parent_names = (const char*[]) {
				"mclk1_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_camss_mclk2_clk = {
	.halt_reg = 0x033E4,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x033E4,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_camss_mclk2_clk",
			.parent_names = (const char*[]) {
				"mclk2_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_camss_mclk3_clk = {
	.halt_reg = 0x03414,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x03414,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_camss_mclk3_clk",
			.parent_names = (const char*[]) {
				"mclk3_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_camss_micro_ahb_clk = {
	.halt_reg = 0x03494,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x03494,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_camss_micro_ahb_clk",
			.parent_names = (const char *[]){
				"ahb_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_camss_csi0phytimer_clk = {
	.halt_reg = 0x03024,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x03024,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_camss_csi0phytimer_clk",
			.parent_names = (const char*[]) {
				"csi0phytimer_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_camss_csi1phytimer_clk = {
	.halt_reg = 0x03054,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x03054,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_camss_csi1phytimer_clk",
			.parent_names = (const char*[]) {
				"csi1phytimer_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_camss_csi2phytimer_clk = {
	.halt_reg = 0x03084,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x03084,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_camss_csi2phytimer_clk",
			.parent_names = (const char*[]) {
				"csi2phytimer_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_camss_top_ahb_clk = {
	.halt_reg = 0x03484,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x03484,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_camss_top_ahb_clk",
			.parent_names = (const char *[]){
				"ahb_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_camss_vfe0_ahb_clk = {
	.halt_reg = 0x03668,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x03668,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_camss_vfe0_ahb_clk",
			.parent_names = (const char *[]){
				"ahb_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_camss_vfe0_clk = {
	.halt_reg = 0x036A8,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x036A8,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_camss_vfe0_clk",
			.parent_names = (const char*[]) {
				"vfe0_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_camss_vfe0_stream_clk = {
	.halt_reg = 0x03720,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x03720,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_camss_vfe0_stream_clk",
			.parent_names = (const char*[]) {
				"vfe0_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_camss_vfe1_ahb_clk = {
	.halt_reg = 0x03678,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x03678,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_camss_vfe1_ahb_clk",
			.parent_names = (const char *[]){
				"ahb_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_camss_vfe1_clk = {
	.halt_reg = 0x036AC,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x036AC,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_camss_vfe1_clk",
			.parent_names = (const char*[]) {
				"vfe1_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_camss_vfe1_stream_clk = {
	.halt_reg = 0x03724,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x03724,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_camss_vfe1_stream_clk",
			.parent_names = (const char*[]) {
				"vfe1_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_camss_vfe_vbif_ahb_clk = {
	.halt_reg = 0x036B8,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x036B8,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_camss_vfe_vbif_ahb_clk",
			.parent_names = (const char *[]){
				"ahb_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_camss_vfe_vbif_axi_clk = {
	.halt_reg = 0x036BC,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x036BC,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_camss_vfe_vbif_axi_clk",
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_mdss_ahb_clk = {
	.halt_reg = 0x02308,
	.halt_check = BRANCH_HALT,
	.hwcg_reg = 0x02308,
	.hwcg_bit = 1,
	.clkr = {
		.enable_reg = 0x02308,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_mdss_ahb_clk",
			.parent_names = (const char *[]){
				"ahb_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_mdss_axi_clk = {
	.halt_reg = 0x02310,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x02310,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_mdss_axi_clk",
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_mdss_byte0_clk = {
	.halt_reg = 0x0233C,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x0233C,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_mdss_byte0_clk",
			.parent_names = (const char*[]) {
				"byte0_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT | CLK_GET_RATE_NOCACHE,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_regmap_div mmss_mdss_byte0_intf_div_clk = {
	.reg = 0x237c,
	.shift = 0,
	.width = 2,
	/*
	 * NOTE: Op does not work for div-3. Current assumption is that div-3
	 * is not a recommended setting for this divider.
	 */
	.clkr = {
		.hw.init = &(struct clk_init_data){
			.name = "mmss_mdss_byte0_intf_div_clk",
			.parent_names = (const char *[]){
					"byte0_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_regmap_div_ops,
			.flags = CLK_GET_RATE_NOCACHE,
		},
	},
};

static struct clk_branch mmss_mdss_byte0_intf_clk = {
	.halt_reg = 0x02374,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x02374,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_mdss_byte0_intf_clk",
			.parent_names = (const char *[]){
				"mmss_mdss_byte0_intf_div_clk",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT | CLK_GET_RATE_NOCACHE,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_mdss_byte1_clk = {
	.halt_reg = 0x02340,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x02340,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_mdss_byte1_clk",
			.parent_names = (const char*[]) {
				"byte1_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT | CLK_GET_RATE_NOCACHE,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_regmap_div mmss_mdss_byte1_intf_div_clk = {
	.reg = 0x2380,
	.shift = 0,
	.width = 2,
	/*
	 * NOTE: Op does not work for div-3. Current assumption is that div-3
	 * is not a recommended setting for this divider.
	 */
	.clkr = {
		.hw.init = &(struct clk_init_data){
			.name = "mmss_mdss_byte1_intf_div_clk",
			.parent_names = (const char *[]){
					"byte1_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_regmap_div_ops,
			.flags = CLK_GET_RATE_NOCACHE,
		},
	},
};

static struct clk_branch mmss_mdss_byte1_intf_clk = {
	.halt_reg = 0x2378,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x2378,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "mmss_mdss_byte1_intf_clk",
			.parent_names = (const char *[]){
				"mmss_mdss_byte1_intf_div_clk",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT | CLK_GET_RATE_NOCACHE,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_mdss_dp_aux_clk = {
	.halt_reg = 0x02364,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x02364,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_mdss_dp_aux_clk",
			.parent_names = (const char*[]) {
				"dp_aux_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_mdss_dp_pixel_clk = {
	.halt_reg = 0x02360,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x02360,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_mdss_dp_pixel_clk",
			.parent_names = (const char*[]) {
				"dp_pixel_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_mdss_dp_link_clk = {
	.halt_reg = 0x02354,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x02354,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_mdss_dp_link_clk",
			.parent_names = (const char*[]) {
				"dp_link_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

/* Reset state of MMSS_MDSS_DP_LINK_INTF_DIV is 0x3 (div-4) */
static struct clk_branch mmss_mdss_dp_link_intf_clk = {
	.halt_reg = 0x02358,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x02358,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_mdss_dp_link_intf_clk",
			.parent_names = (const char*[]) {
				"dp_link_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_mdss_dp_crypto_clk = {
	.halt_reg = 0x0235C,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x0235C,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_mdss_dp_crypto_clk",
			.parent_names = (const char*[]) {
				"dp_crypto_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_mdss_dp_gtc_clk = {
	.halt_reg = 0x02368,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x02368,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_mdss_dp_gtc_clk",
			.parent_names = (const char*[]) {
				"dp_gtc_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_mdss_esc0_clk = {
	.halt_reg = 0x02344,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x02344,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_mdss_esc0_clk",
			.parent_names = (const char*[]) {
				"esc0_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_mdss_esc1_clk = {
	.halt_reg = 0x02348,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x02348,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_mdss_esc1_clk",
			.parent_names = (const char*[]) {
				"esc1_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_mdss_extpclk_clk = {
	.halt_reg = 0x02324,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x02324,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_mdss_extpclk_clk",
			.parent_names = (const char*[]) {
				"extpclk_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_mdss_hdmi_clk = {
	.halt_reg = 0x02338,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x02338,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_mdss_hdmi_clk",
			.parent_names = (const char*[]) {
				"hdmi_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_mdss_hdmi_dp_ahb_clk = {
	.halt_reg = 0x0230C,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x0230C,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_mdss_hdmi_dp_ahb_clk",
			.parent_names = (const char *[]){
				"ahb_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_mdss_mdp_clk = {
	.halt_reg = 0x0231C,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x0231C,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_mdss_mdp_clk",
			.parent_names = (const char*[]) {
				"mmss_mdss_mdp_lut_clk",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT | CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_mdss_mdp_lut_clk = {
	.halt_reg = 0x02320,
	.halt_check = BRANCH_HALT_DELAY,
	.clkr = {
		.enable_reg = 0x02320,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_mdss_mdp_lut_clk",
			.parent_names = (const char*[]) {
				"mdp_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_mdss_pclk0_clk = {
	.halt_reg = 0x02314,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x02314,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_mdss_pclk0_clk",
			.parent_names = (const char*[]) {
				"pclk0_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT | CLK_GET_RATE_NOCACHE,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_mdss_pclk1_clk = {
	.halt_reg = 0x02318,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x02318,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_mdss_pclk1_clk",
			.parent_names = (const char*[]) {
				"pclk1_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT | CLK_GET_RATE_NOCACHE,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_mdss_rot_clk = {
	.halt_reg = 0x02350,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x02350,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_mdss_rot_clk",
			.parent_names = (const char*[]) {
				"rot_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_mdss_vsync_clk = {
	.halt_reg = 0x02328,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x02328,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_mdss_vsync_clk",
			.parent_names = (const char*[]) {
				"vsync_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_mnoc_ahb_clk = {
	.halt_reg = 0x05024,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x05024,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_mnoc_ahb_clk",
			.parent_names = (const char*[]) {
				"ahb_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_ENABLE_HAND_OFF | CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_misc_ahb_clk = {
	.halt_reg = 0x328,
	.halt_check = BRANCH_HALT,
	.hwcg_reg = 0x328,
	.hwcg_bit = 1,
	.clkr = {
		.enable_reg = 0x328,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "mmss_misc_ahb_clk",
			/*
			 * Dependency to be enabled before the branch is
			 * enabled.
			 */
			.parent_names = (const char *[]){
				"mmss_mnoc_ahb_clk",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_misc_cxo_clk = {
	.halt_reg = 0x00324,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x00324,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_misc_cxo_clk",
			.parent_names = (const char *[]){ "cxo" },
			.num_parents = 1,
			.flags = CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_mnoc_maxi_clk = {
	.halt_reg = 0x0F004,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x0F004,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_mnoc_maxi_clk",
			.parent_names = (const char*[]) {
				"maxi_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_ENABLE_HAND_OFF | CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_video_subcore0_clk = {
	.halt_reg = 0x01048,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x01048,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_video_subcore0_clk",
			.parent_names = (const char*[]) {
				"video_subcore0_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_video_subcore1_clk = {
	.halt_reg = 0x0104C,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x0104C,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_video_subcore1_clk",
			.parent_names = (const char*[]) {
				"video_subcore1_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_video_ahb_clk = {
	.halt_reg = 0x01030,
	.halt_check = BRANCH_HALT,
	.hwcg_reg = 0x01030,
	.hwcg_bit = 1,
	.clkr = {
		.enable_reg = 0x01030,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_video_ahb_clk",
			.parent_names = (const char *[]){
				"ahb_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_video_axi_clk = {
	.halt_reg = 0x01034,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x01034,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_video_axi_clk",
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_video_core_clk = {
	.halt_reg = 0x01028,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x01028,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_video_core_clk",
			.parent_names = (const char*[]) {
				"video_core_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_video_maxi_clk = {
	.halt_reg = 0x01038,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x01038,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_video_maxi_clk",
			.parent_names = (const char*[]) {
				"maxi_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_vmem_ahb_clk = {
	.halt_reg = 0x0F068,
	.halt_check = BRANCH_HALT,
	.hwcg_reg = 0x0F068,
	.hwcg_bit = 1,
	.clkr = {
		.enable_reg = 0x0F068,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_vmem_ahb_clk",
			.parent_names = (const char *[]){
				"ahb_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_vmem_maxi_clk = {
	.halt_reg = 0x0F064,
	.halt_check = BRANCH_HALT,
	.hwcg_reg = 0x0F064,
	.hwcg_bit = 1,
	.clkr = {
		.enable_reg = 0x0F064,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "mmss_vmem_maxi_clk",
			.parent_names = (const char*[]) {
				"maxi_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT | CLK_IGNORE_UNUSED,
			.ops = &clk_branch2_ops,
		},
	},
};

#if 0 /* Unsupported for now */
static struct mux_clk mmss_debug_mux = {
	.ops = &mux_reg_ops,
	.en_mask = BIT(16),
	.mask = 0x3FF,
	.offset = 0x00900,
	.en_offset = 0x00900,
	MUX_SRC_LIST(
		{ &mmss_mnoc_ahb_clk.c, 0x0001 },
		{ &mmss_misc_ahb_clk.c, 0x0003 },
		{ &mmss_vmem_maxi_clk.c, 0x0009 },
		{ &mmss_vmem_ahb_clk.c, 0x000a },
		{ &mmss_bimc_smmu_ahb_clk.c, 0x000c },
		{ &mmss_bimc_smmu_axi_clk.c, 0x000d },
		{ &mmss_video_core_clk.c, 0x000e },
		{ &mmss_video_axi_clk.c, 0x000f },
		{ &mmss_video_maxi_clk.c, 0x0010 },
		{ &mmss_video_ahb_clk.c, 0x0011 },
		{ &mmss_mdss_rot_clk.c, 0x0012 },
		{ &mmss_snoc_dvm_axi_clk.c, 0x0013 },
		{ &mmss_mdss_mdp_clk.c, 0x0014 },
		{ &mmss_mdss_mdp_lut_clk.c, 0x0015 },
		{ &mmss_mdss_pclk0_clk.c, 0x0016 },
		{ &mmss_mdss_pclk1_clk.c, 0x0017 },
		{ &mmss_mdss_extpclk_clk.c, 0x0018 },
		{ &mmss_video_subcore0_clk.c, 0x001a },
		{ &mmss_video_subcore1_clk.c, 0x001b },
		{ &mmss_mdss_vsync_clk.c, 0x001c },
		{ &mmss_mdss_hdmi_clk.c, 0x001d },
		{ &mmss_mdss_byte0_clk.c, 0x001e },
		{ &mmss_mdss_byte1_clk.c, 0x001f },
		{ &mmss_mdss_esc0_clk.c, 0x0020 },
		{ &mmss_mdss_esc1_clk.c, 0x0021 },
		{ &mmss_mdss_ahb_clk.c, 0x0022 },
		{ &mmss_mdss_hdmi_dp_ahb_clk.c, 0x0023 },
		{ &mmss_mdss_axi_clk.c, 0x0024 },
		{ &mmss_camss_top_ahb_clk.c, 0x0025 },
		{ &mmss_camss_micro_ahb_clk.c, 0x0026 },
		{ &mmss_camss_gp0_clk.c, 0x0027 },
		{ &mmss_camss_gp1_clk.c, 0x0028 },
		{ &mmss_camss_mclk0_clk.c, 0x0029 },
		{ &mmss_camss_mclk1_clk.c, 0x002a },
		{ &mmss_camss_mclk2_clk.c, 0x002b },
		{ &mmss_camss_mclk3_clk.c, 0x002c },
		{ &mmss_camss_cci_clk.c, 0x002d },
		{ &mmss_camss_cci_ahb_clk.c, 0x002e },
		{ &mmss_camss_csi0phytimer_clk.c, 0x002f },
		{ &mmss_camss_csi1phytimer_clk.c, 0x0030 },
		{ &mmss_camss_csi2phytimer_clk.c, 0x0031 },
		{ &mmss_camss_jpeg0_clk.c, 0x0032 },
		{ &mmss_camss_ispif_ahb_clk.c, 0x0033 },
		{ &mmss_camss_jpeg_ahb_clk.c, 0x0035 },
		{ &mmss_camss_jpeg_axi_clk.c, 0x0036 },
		{ &mmss_camss_ahb_clk.c, 0x0037 },
		{ &mmss_camss_vfe0_clk.c, 0x0038 },
		{ &mmss_camss_vfe1_clk.c, 0x0039 },
		{ &mmss_camss_cpp_clk.c, 0x003a },
		{ &mmss_camss_cpp_ahb_clk.c, 0x003b },
		{ &mmss_camss_csi_vfe0_clk.c, 0x003f },
		{ &mmss_camss_csi_vfe1_clk.c, 0x0040 },
		{ &mmss_camss_csi0_clk.c, 0x0041 },
		{ &mmss_camss_csi0_ahb_clk.c, 0x0042 },
		{ &mmss_camss_csiphy0_clk.c, 0x0043 },
		{ &mmss_camss_csi0rdi_clk.c, 0x0044 },
		{ &mmss_camss_csi0pix_clk.c, 0x0045 },
		{ &mmss_camss_csi1_clk.c, 0x0046 },
		{ &mmss_camss_csi1_ahb_clk.c, 0x0047 },
		{ &mmss_camss_csi1rdi_clk.c, 0x0049 },
		{ &mmss_camss_csi1pix_clk.c, 0x004a },
		{ &mmss_camss_csi2_clk.c, 0x004b },
		{ &mmss_camss_csi2_ahb_clk.c, 0x004c },
		{ &mmss_camss_csi2rdi_clk.c, 0x004e },
		{ &mmss_camss_csi2pix_clk.c, 0x004f },
		{ &mmss_camss_csi3_clk.c, 0x0050 },
		{ &mmss_camss_csi3_ahb_clk.c, 0x0051 },
		{ &mmss_camss_csi3rdi_clk.c, 0x0053 },
		{ &mmss_camss_csi3pix_clk.c, 0x0054 },
		{ &mmss_mnoc_maxi_clk.c, 0x0070 },
		{ &mmss_camss_vfe0_stream_clk.c, 0x0071 },
		{ &mmss_camss_vfe1_stream_clk.c, 0x0072 },
		{ &mmss_camss_cpp_vbif_ahb_clk.c, 0x0073 },
		{ &mmss_misc_cxo_clk.c, 0x0077 },
		{ &mmss_camss_cpp_axi_clk.c, 0x007a },
		{ &mmss_camss_csiphy1_clk.c, 0x0085 },
		{ &mmss_camss_vfe0_ahb_clk.c, 0x0086 },
		{ &mmss_camss_vfe1_ahb_clk.c, 0x0087 },
		{ &mmss_camss_csiphy2_clk.c, 0x0088 },
		{ &mmss_fd_core_clk.c, 0x0089 },
		{ &mmss_fd_core_uar_clk.c, 0x008a },
		{ &mmss_fd_ahb_clk.c, 0x008c },
		{ &mmss_camss_cphy_csid0_clk.c, 0x008d },
		{ &mmss_camss_cphy_csid1_clk.c, 0x008e },
		{ &mmss_camss_cphy_csid2_clk.c, 0x008f },
		{ &mmss_camss_cphy_csid3_clk.c, 0x0090 },
		{ &mmss_mdss_dp_link_clk.c, 0x0098 },
		{ &mmss_mdss_dp_link_intf_clk.c, 0x0099 },
		{ &mmss_mdss_dp_crypto_clk.c, 0x009a },
		{ &mmss_mdss_dp_pixel_clk.c, 0x009b },
		{ &mmss_mdss_dp_aux_clk.c, 0x009c },
		{ &mmss_mdss_dp_gtc_clk.c, 0x009d },
		{ &mmss_mdss_byte0_intf_clk.c, 0x00ad },
		{ &mmss_mdss_byte1_intf_clk.c, 0x00ae },
	),
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "mmss_debug_mux",
		.ops = &clk_ops_gen_mux,
		.flags = CLK_GET_RATE_NOCACHE,
	},
};
#endif

static struct clk_regmap *mmcc_msm8998_clocks[] = {
	[MMPLL0_PLL] = &mmpll0.clkr,
	[MMPLL1_PLL] = &mmpll1.clkr,
	[MMPLL3_PLL] = &mmpll3.clkr,
	[MMPLL4_PLL] = &mmpll4.clkr,
	[MMPLL5_PLL] = &mmpll5.clkr,
	[MMPLL6_PLL] = &mmpll6.clkr,
	[MMPLL7_PLL] = &mmpll7.clkr,
	[MMPLL10_PLL] = &mmpll10.clkr,
	[AHB_CLK_SRC] = &ahb_clk_src.clkr,
	[CSI0_CLK_SRC] = &csi0_clk_src.clkr,
	[VFE0_CLK_SRC] = &vfe0_clk_src.clkr,
	[VFE1_CLK_SRC] = &vfe1_clk_src.clkr,
	[MDP_CLK_SRC] = &mdp_clk_src.clkr,
	[MAXI_CLK_SRC] = &maxi_clk_src.clkr,
	[CPP_CLK_SRC] = &cpp_clk_src.clkr,
	[JPEG0_CLK_SRC] = &jpeg0_clk_src.clkr,
	[ROT_CLK_SRC] = &rot_clk_src.clkr,
	[VIDEO_CORE_CLK_SRC] = &video_core_clk_src.clkr,
	[CSI1_CLK_SRC] = &csi1_clk_src.clkr,
	[CSI2_CLK_SRC] = &csi2_clk_src.clkr,
	[CSI3_CLK_SRC] = &csi3_clk_src.clkr,
	[FD_CORE_CLK_SRC] = &fd_core_clk_src.clkr,
	[VIDEO_SUBCORE0_CLK_SRC] = &video_subcore0_clk_src.clkr,
	[VIDEO_SUBCORE1_CLK_SRC] = &video_subcore1_clk_src.clkr,
	[CCI_CLK_SRC] = &cci_clk_src.clkr,
	[CSIPHY_CLK_SRC] = &csiphy_clk_src.clkr,
	[CAMSS_GP0_CLK_SRC] = &camss_gp0_clk_src.clkr,
	[CAMSS_GP1_CLK_SRC] = &camss_gp1_clk_src.clkr,
	[MCLK0_CLK_SRC] = &mclk0_clk_src.clkr,
	[MCLK1_CLK_SRC] = &mclk1_clk_src.clkr,
	[MCLK2_CLK_SRC] = &mclk2_clk_src.clkr,
	[MCLK3_CLK_SRC] = &mclk3_clk_src.clkr,
	[BYTE0_CLK_SRC] = &byte0_clk_src.clkr,
	[BYTE1_CLK_SRC] = &byte1_clk_src.clkr,
	[PCLK0_CLK_SRC] = &pclk0_clk_src.clkr,
	[PCLK1_CLK_SRC] = &pclk1_clk_src.clkr,
	[EXTPCLK_CLK_SRC] = &extpclk_clk_src.clkr,
	[DP_PIXEL_CLK_SRC] = &dp_pixel_clk_src.clkr,
	[DP_LINK_CLK_SRC] = &dp_link_clk_src.clkr,
	[DP_CRYPTO_CLK_SRC] = &dp_crypto_clk_src.clkr,
	[CSI0PHYTIMER_CLK_SRC] = &csi0phytimer_clk_src.clkr,
	[CSI1PHYTIMER_CLK_SRC] = &csi1phytimer_clk_src.clkr,
	[CSI2PHYTIMER_CLK_SRC] = &csi2phytimer_clk_src.clkr,
	[DP_AUX_CLK_SRC] = &dp_aux_clk_src.clkr,
	[DP_GTC_CLK_SRC] = &dp_gtc_clk_src.clkr,
	[ESC0_CLK_SRC] = &esc0_clk_src.clkr,
	[ESC1_CLK_SRC] = &esc1_clk_src.clkr,
	[HDMI_CLK_SRC] = &hdmi_clk_src.clkr,
	[VSYNC_CLK_SRC] = &vsync_clk_src.clkr,
	[MMSS_BIMC_SMMU_AHB_CLK] = &mmss_bimc_smmu_ahb_clk.clkr,
	[MMSS_BIMC_SMMU_AXI_CLK] = &mmss_bimc_smmu_axi_clk.clkr,
	[MMSS_SNOC_DVM_AXI_CLK] = &mmss_snoc_dvm_axi_clk.clkr,
	[MMSS_CAMSS_AHB_CLK] = &mmss_camss_ahb_clk.clkr,
	[MMSS_CAMSS_CCI_AHB_CLK] = &mmss_camss_cci_ahb_clk.clkr,
	[MMSS_CAMSS_CCI_CLK] = &mmss_camss_cci_clk.clkr,
	[MMSS_CAMSS_CPP_AHB_CLK] = &mmss_camss_cpp_ahb_clk.clkr,
	[MMSS_CAMSS_CPP_CLK] = &mmss_camss_cpp_clk.clkr,
	[MMSS_CAMSS_CPP_AXI_CLK] = &mmss_camss_cpp_axi_clk.clkr,
	[MMSS_CAMSS_CPP_VBIF_AHB_CLK] = &mmss_camss_cpp_vbif_ahb_clk.clkr,
	[MMSS_CAMSS_CPHY_CSID0_CLK] = &mmss_camss_cphy_csid0_clk.clkr,
	[MMSS_CAMSS_CSI0_AHB_CLK] = &mmss_camss_csi0_ahb_clk.clkr,
	[MMSS_CAMSS_CSI0_CLK] = &mmss_camss_csi0_clk.clkr,
	[MMSS_CAMSS_CSI0PIX_CLK] = &mmss_camss_csi0pix_clk.clkr,
	[MMSS_CAMSS_CSI0RDI_CLK] = &mmss_camss_csi0rdi_clk.clkr,
	[MMSS_CAMSS_CPHY_CSID1_CLK] = &mmss_camss_cphy_csid1_clk.clkr,
	[MMSS_CAMSS_CSI1_AHB_CLK] = &mmss_camss_csi1_ahb_clk.clkr,
	[MMSS_CAMSS_CSI1_CLK] = &mmss_camss_csi1_clk.clkr,
	[MMSS_CAMSS_CSI1PIX_CLK] = &mmss_camss_csi1pix_clk.clkr,
	[MMSS_CAMSS_CSI1RDI_CLK] = &mmss_camss_csi1rdi_clk.clkr,
	[MMSS_CAMSS_CPHY_CSID2_CLK] = &mmss_camss_cphy_csid2_clk.clkr,
	[MMSS_CAMSS_CSI2_AHB_CLK] = &mmss_camss_csi2_ahb_clk.clkr,
	[MMSS_CAMSS_CSI2_CLK] = &mmss_camss_csi2_clk.clkr,
	[MMSS_CAMSS_CSI2PIX_CLK] = &mmss_camss_csi2pix_clk.clkr,
	[MMSS_CAMSS_CSI2RDI_CLK] = &mmss_camss_csi2rdi_clk.clkr,
	[MMSS_CAMSS_CPHY_CSID3_CLK] = &mmss_camss_cphy_csid3_clk.clkr,
	[MMSS_CAMSS_CSI3_AHB_CLK] = &mmss_camss_csi3_ahb_clk.clkr,
	[MMSS_CAMSS_CSI3_CLK] = &mmss_camss_csi3_clk.clkr,
	[MMSS_CAMSS_CSI3PIX_CLK] = &mmss_camss_csi3pix_clk.clkr,
	[MMSS_CAMSS_CSI3RDI_CLK] = &mmss_camss_csi3rdi_clk.clkr,
	[MMSS_CAMSS_CSI_VFE0_CLK] = &mmss_camss_csi_vfe0_clk.clkr,
	[MMSS_CAMSS_CSI_VFE1_CLK] = &mmss_camss_csi_vfe1_clk.clkr,
	[MMSS_CAMSS_CSIPHY0_CLK] = &mmss_camss_csiphy0_clk.clkr,
	[MMSS_CAMSS_CSIPHY1_CLK] = &mmss_camss_csiphy1_clk.clkr,
	[MMSS_CAMSS_CSIPHY2_CLK] = &mmss_camss_csiphy2_clk.clkr,
	[MMSS_FD_AHB_CLK] = &mmss_fd_ahb_clk.clkr,
	[MMSS_FD_CORE_CLK] = &mmss_fd_core_clk.clkr,
	[MMSS_FD_CORE_UAR_CLK] = &mmss_fd_core_uar_clk.clkr,
	[MMSS_CAMSS_GP0_CLK] = &mmss_camss_gp0_clk.clkr,
	[MMSS_CAMSS_GP1_CLK] = &mmss_camss_gp1_clk.clkr,
	[MMSS_CAMSS_ISPIF_AHB_CLK] = &mmss_camss_ispif_ahb_clk.clkr,
	[MMSS_CAMSS_JPEG0_CLK] = &mmss_camss_jpeg0_clk.clkr,
	[MMSS_CAMSS_JPEG_AHB_CLK] = &mmss_camss_jpeg_ahb_clk.clkr,
	[MMSS_CAMSS_JPEG_AXI_CLK] = &mmss_camss_jpeg_axi_clk.clkr,
	[MMSS_CAMSS_MCLK0_CLK] = &mmss_camss_mclk0_clk.clkr,
	[MMSS_CAMSS_MCLK1_CLK] = &mmss_camss_mclk1_clk.clkr,
	[MMSS_CAMSS_MCLK2_CLK] = &mmss_camss_mclk2_clk.clkr,
	[MMSS_CAMSS_MCLK3_CLK] = &mmss_camss_mclk3_clk.clkr,
	[MMSS_CAMSS_MICRO_AHB_CLK] = &mmss_camss_micro_ahb_clk.clkr,
	[MMSS_CAMSS_CSI0PHYTIMER_CLK] = &mmss_camss_csi0phytimer_clk.clkr,
	[MMSS_CAMSS_CSI1PHYTIMER_CLK] = &mmss_camss_csi1phytimer_clk.clkr,
	[MMSS_CAMSS_CSI2PHYTIMER_CLK] = &mmss_camss_csi2phytimer_clk.clkr,
	[MMSS_CAMSS_TOP_AHB_CLK] = &mmss_camss_top_ahb_clk.clkr,
	[MMSS_CAMSS_VFE0_AHB_CLK] = &mmss_camss_vfe0_ahb_clk.clkr,
	[MMSS_CAMSS_VFE0_CLK] = &mmss_camss_vfe0_clk.clkr,
	[MMSS_CAMSS_VFE0_STREAM_CLK] = &mmss_camss_vfe0_stream_clk.clkr,
	[MMSS_CAMSS_VFE1_AHB_CLK] = &mmss_camss_vfe1_ahb_clk.clkr,
	[MMSS_CAMSS_VFE1_CLK] = &mmss_camss_vfe1_clk.clkr,
	[MMSS_CAMSS_VFE1_STREAM_CLK] = &mmss_camss_vfe1_stream_clk.clkr,
	[MMSS_CAMSS_VFE_VBIF_AHB_CLK] = &mmss_camss_vfe_vbif_ahb_clk.clkr,
	[MMSS_CAMSS_VFE_VBIF_AXI_CLK] = &mmss_camss_vfe_vbif_axi_clk.clkr,
	[MMSS_MDSS_AHB_CLK] = &mmss_mdss_ahb_clk.clkr,
	[MMSS_MDSS_AXI_CLK] = &mmss_mdss_axi_clk.clkr,
	[MMSS_MDSS_BYTE0_CLK] = &mmss_mdss_byte0_clk.clkr,
	[MMSS_MDSS_BYTE0_INTF_DIV_CLK] = &mmss_mdss_byte0_intf_div_clk.clkr,
	[MMSS_MDSS_BYTE0_INTF_CLK] = &mmss_mdss_byte0_intf_clk.clkr,
	[MMSS_MDSS_BYTE1_CLK] = &mmss_mdss_byte1_clk.clkr,
	[MMSS_MDSS_BYTE1_INTF_DIV_CLK] = &mmss_mdss_byte1_intf_div_clk.clkr,
	[MMSS_MDSS_BYTE1_INTF_CLK] = &mmss_mdss_byte1_intf_clk.clkr,
	[MMSS_MDSS_DP_AUX_CLK] = &mmss_mdss_dp_aux_clk.clkr,
	[MMSS_MDSS_DP_CRYPTO_CLK] = &mmss_mdss_dp_crypto_clk.clkr,
	[MMSS_MDSS_DP_PIXEL_CLK] = &mmss_mdss_dp_pixel_clk.clkr,
	[MMSS_MDSS_DP_LINK_CLK] = &mmss_mdss_dp_link_clk.clkr,
	[MMSS_MDSS_DP_LINK_INTF_CLK] = &mmss_mdss_dp_link_intf_clk.clkr,
	[MMSS_MDSS_DP_GTC_CLK] = &mmss_mdss_dp_gtc_clk.clkr,
	[MMSS_MDSS_ESC0_CLK] = &mmss_mdss_esc0_clk.clkr,
	[MMSS_MDSS_ESC1_CLK] = &mmss_mdss_esc1_clk.clkr,
	[MMSS_MDSS_EXTPCLK_CLK] = &mmss_mdss_extpclk_clk.clkr,
	[MMSS_MDSS_HDMI_CLK] = &mmss_mdss_hdmi_clk.clkr,
	[MMSS_MDSS_HDMI_DP_AHB_CLK] = &mmss_mdss_hdmi_dp_ahb_clk.clkr,
	[MMSS_MDSS_MDP_CLK] = &mmss_mdss_mdp_clk.clkr,
	[MMSS_MDSS_MDP_LUT_CLK] = &mmss_mdss_mdp_lut_clk.clkr,
	[MMSS_MDSS_PCLK0_CLK] = &mmss_mdss_pclk0_clk.clkr,
	[MMSS_MDSS_PCLK1_CLK] = &mmss_mdss_pclk1_clk.clkr,
	[MMSS_MDSS_ROT_CLK] = &mmss_mdss_rot_clk.clkr,
	[MMSS_MDSS_VSYNC_CLK] = &mmss_mdss_vsync_clk.clkr,
	[MMSS_MISC_AHB_CLK] = &mmss_misc_ahb_clk.clkr,
	[MMSS_MISC_CXO_CLK] = &mmss_misc_cxo_clk.clkr,
	[MMSS_MNOC_AHB_CLK] = &mmss_mnoc_ahb_clk.clkr,
	[MMSS_VIDEO_SUBCORE0_CLK] = &mmss_video_subcore0_clk.clkr,
	[MMSS_VIDEO_SUBCORE1_CLK] = &mmss_video_subcore1_clk.clkr,
	[MMSS_VIDEO_AHB_CLK] = &mmss_video_ahb_clk.clkr,
	[MMSS_VIDEO_AXI_CLK] = &mmss_video_axi_clk.clkr,
	[MMSS_VIDEO_CORE_CLK] = &mmss_video_core_clk.clkr,
	[MMSS_VIDEO_MAXI_CLK] = &mmss_video_maxi_clk.clkr,
	[MMSS_VMEM_AHB_CLK] = &mmss_vmem_ahb_clk.clkr,
	[MMSS_VMEM_MAXI_CLK] = &mmss_vmem_maxi_clk.clkr,
	[MMSS_MNOC_MAXI_CLK] = &mmss_mnoc_maxi_clk.clkr,

};

static const struct qcom_reset_map mmcc_msm8998_resets[] = {
	[CAMSS_MICRO_BCR] = { 0x3490 },
};

static const struct regmap_config mmcc_msm8998_regmap_config = {
	.reg_bits	= 32,
	.reg_stride	= 4,
	.val_bits	= 32,
	.max_register	= 0x40004,
	.fast_io	= true,
};

static const struct qcom_cc_desc mmcc_msm8998_desc = {
	.config = &mmcc_msm8998_regmap_config,
	.clks = mmcc_msm8998_clocks,
	.num_clks = ARRAY_SIZE(mmcc_msm8998_clocks),
	.resets = mmcc_msm8998_resets,
	.num_resets = ARRAY_SIZE(mmcc_msm8998_resets),
};

static const struct of_device_id mmcc_msm8998_match_table[] = {
	{ .compatible = "qcom,mmcc-msm8998" },
	{ .compatible = "qcom,mmcc-msm8998-v2" },
	{ }
};
MODULE_DEVICE_TABLE(of, mmcc_msm8998_match_table);

/* Voters */
struct clk_hw *mmcc_voters_msm8998_hws[] = {
	[MMSS_CAMSS_JPEG0_VOTE_CLK] = &mmss_camss_jpeg0_vote_clk.hw,
	[MMSS_CAMSS_JPEG0_DMA_VOTE_CLK] = &mmss_camss_jpeg0_dma_vote_clk.hw,
};

static struct of_device_id mmcc_voters_msm8998_match_table[] = {
	{ .compatible = "qcom,mmsscc-voters-msm8998" },
	{ }
};

static int mmcc_msm8998_voters_probe(struct platform_device *pdev)
{
	int rc, i, num_clks;
	struct clk *clk;
	struct clk_onecell_data *onecell;

	num_clks = ARRAY_SIZE(mmcc_voters_msm8998_hws);

	onecell = devm_kzalloc(&pdev->dev,
			sizeof(struct clk_onecell_data), GFP_KERNEL);
	if (!onecell)
		return -ENOMEM;

	onecell->clks = devm_kzalloc(&pdev->dev,
			(num_clks * sizeof(struct clk*)), GFP_KERNEL);
	if (!onecell->clks)
		return -ENOMEM;

	onecell->clk_num = num_clks;

	for (i = 0; i < num_clks; i++) {
		if (!mmcc_voters_msm8998_hws[i])
			continue;

		clk = devm_clk_register(&pdev->dev, mmcc_voters_msm8998_hws[i]);
		if (IS_ERR(clk)) {
			dev_err(&pdev->dev, "Cannot register clock no %d\n",i);
			return PTR_ERR(clk);
		}
		onecell->clks[i] = clk;
	}

	rc = of_clk_add_provider(pdev->dev.of_node,
			of_clk_src_onecell_get, onecell);
	if (rc == 0)
		dev_info(&pdev->dev, "Registered MMCC Software Voters\n");

	return rc;
}

static struct platform_driver mmcc_voters_msm8998_driver = {
	.probe = mmcc_msm8998_voters_probe,
	.driver = {
		.name = "mmsscc-voters-msm8998",
		.of_match_table = mmcc_voters_msm8998_match_table,
	},
};

static void msm_mmsscc_hamster_fixup(void)
{
	mmpll3.clkr.hw.init->rate_max[VDD_DIG_LOWER] = 533000000;
	mmpll3.clkr.hw.init->rate_max[VDD_DIG_LOW] = 533000000;
	mmpll3.clkr.hw.init->rate_max[VDD_DIG_LOW_L1] = 533000000;
	mmpll3.clkr.hw.init->rate_max[VDD_DIG_NOMINAL] = 1066000000;
	mmpll3.clkr.hw.init->rate_max[VDD_DIG_HIGH] = 1066000000;

	mmpll4.clkr.hw.init->rate_max[VDD_DIG_LOW] = 384000000;
	mmpll4.clkr.hw.init->rate_max[VDD_DIG_LOW_L1] = 384000000;
	mmpll4.clkr.hw.init->rate_max[VDD_DIG_NOMINAL] = 768000000;

	mmpll5.clkr.hw.init->rate_max[VDD_DIG_LOW] = 412500000;
	mmpll5.clkr.hw.init->rate_max[VDD_DIG_LOW_L1] = 412500000;
	mmpll5.clkr.hw.init->rate_max[VDD_DIG_NOMINAL] = 825000000;

	mmpll6.clkr.hw.init->rate_max[VDD_DIG_LOWER] = 444000000;
	mmpll6.clkr.hw.init->rate_max[VDD_DIG_LOW] = 444000000;
	mmpll6.clkr.hw.init->rate_max[VDD_DIG_LOW_L1] = 444000000;
	mmpll6.clkr.hw.init->rate_max[VDD_DIG_NOMINAL] = 888000000;
	mmpll6.clkr.hw.init->rate_max[VDD_DIG_HIGH] = 888000000;

	vfe0_clk_src.freq_tbl = ftbl_vfe_clk_src_vq;
	vfe0_clk_src.clkr.hw.init->rate_max[VDD_DIG_LOW] = 404000000;
	vfe0_clk_src.clkr.hw.init->rate_max[VDD_DIG_LOW_L1] = 480000000;
	vfe1_clk_src.freq_tbl = ftbl_vfe_clk_src_vq;
	vfe1_clk_src.clkr.hw.init->rate_max[VDD_DIG_LOW] = 404000000;
	vfe1_clk_src.clkr.hw.init->rate_max[VDD_DIG_LOW_L1] = 480000000;

	csi0_clk_src.freq_tbl = ftbl_csi_clk_src_vq;
	csi0_clk_src.clkr.hw.init->rate_max[VDD_DIG_LOW] = 274290000;
	csi0_clk_src.clkr.hw.init->rate_max[VDD_DIG_LOW_L1] = 320000000;
	csi1_clk_src.freq_tbl = ftbl_csi_clk_src_vq;
	csi1_clk_src.clkr.hw.init->rate_max[VDD_DIG_LOW] = 274290000;
	csi1_clk_src.clkr.hw.init->rate_max[VDD_DIG_LOW_L1] = 320000000;
	csi2_clk_src.freq_tbl = ftbl_csi_clk_src_vq;
	csi2_clk_src.clkr.hw.init->rate_max[VDD_DIG_LOW] = 274290000;
	csi2_clk_src.clkr.hw.init->rate_max[VDD_DIG_LOW_L1] = 320000000;
	csi3_clk_src.freq_tbl = ftbl_csi_clk_src_vq;
	csi3_clk_src.clkr.hw.init->rate_max[VDD_DIG_LOW] = 274290000;
	csi3_clk_src.clkr.hw.init->rate_max[VDD_DIG_LOW_L1] = 320000000;

	cpp_clk_src.freq_tbl = ftbl_cpp_clk_src_vq;
	cpp_clk_src.clkr.hw.init->rate_max[VDD_DIG_LOW] = 384000000;
	cpp_clk_src.clkr.hw.init->rate_max[VDD_DIG_LOW_L1] = 404000000;
	jpeg0_clk_src.freq_tbl = ftbl_jpeg0_clk_src_vq;
	jpeg0_clk_src.clkr.hw.init->rate_max[VDD_DIG_LOW_L1] = 320000000;
	csiphy_clk_src.freq_tbl = ftbl_csiphy_clk_src_vq;
	csiphy_clk_src.clkr.hw.init->rate_max[VDD_DIG_LOW] = 274290000;
	csiphy_clk_src.clkr.hw.init->rate_max[VDD_DIG_LOW_L1] = 300000000;
	fd_core_clk_src.freq_tbl = ftbl_fd_core_clk_src_vq;
	fd_core_clk_src.clkr.hw.init->rate_max[VDD_DIG_LOW] = 404000000;
	fd_core_clk_src.clkr.hw.init->rate_max[VDD_DIG_LOW_L1] = 480000000;

	csi0phytimer_clk_src.clkr.hw.init->rate_max[VDD_DIG_LOW_L1] = 269333333;
	csi1phytimer_clk_src.clkr.hw.init->rate_max[VDD_DIG_LOW_L1] = 269333333;
	csi2phytimer_clk_src.clkr.hw.init->rate_max[VDD_DIG_LOW_L1] = 269333333;

	mdp_clk_src.clkr.hw.init->rate_max[VDD_DIG_LOW_L1] = 330000000;
	extpclk_clk_src.clkr.hw.init->rate_max[VDD_DIG_LOW] = 312500000;
	extpclk_clk_src.clkr.hw.init->rate_max[VDD_DIG_LOW_L1] = 375000000;
	rot_clk_src.clkr.hw.init->rate_max[VDD_DIG_LOW_L1] = 330000000;

	video_core_clk_src.freq_tbl = ftbl_video_core_clk_src_vq;
	video_core_clk_src.clkr.hw.init->rate_max[VDD_DIG_LOWER] = 200000000;
	video_core_clk_src.clkr.hw.init->rate_max[VDD_DIG_LOW] = 269330000;
	video_core_clk_src.clkr.hw.init->rate_max[VDD_DIG_LOW_L1] = 355200000;
	video_core_clk_src.clkr.hw.init->rate_max[VDD_DIG_NOMINAL] = 444000000;
	video_core_clk_src.clkr.hw.init->rate_max[VDD_DIG_HIGH] = 533000000;

	video_subcore0_clk_src.freq_tbl = ftbl_video_subcore_clk_src_vq;
	video_subcore0_clk_src.clkr.hw.init->rate_max[VDD_DIG_LOWER] = 200000000;
	video_subcore0_clk_src.clkr.hw.init->rate_max[VDD_DIG_LOW] = 269330000;
	video_subcore0_clk_src.clkr.hw.init->rate_max[VDD_DIG_LOW_L1] = 355200000;
	video_subcore0_clk_src.clkr.hw.init->rate_max[VDD_DIG_NOMINAL] = 444000000;
	video_subcore0_clk_src.clkr.hw.init->rate_max[VDD_DIG_HIGH] = 533000000;

	video_subcore1_clk_src.freq_tbl = ftbl_video_subcore_clk_src_vq;
	video_subcore1_clk_src.clkr.hw.init->rate_max[VDD_DIG_LOWER] = 200000000;
	video_subcore1_clk_src.clkr.hw.init->rate_max[VDD_DIG_LOW] = 269330000;
	video_subcore1_clk_src.clkr.hw.init->rate_max[VDD_DIG_LOW_L1] = 355200000;
	video_subcore1_clk_src.clkr.hw.init->rate_max[VDD_DIG_NOMINAL] = 444000000;
	video_subcore1_clk_src.clkr.hw.init->rate_max[VDD_DIG_HIGH] = 533000000;
};

static void msm_mmsscc_v2_fixup(void)
{
	csi0_clk_src.clkr.hw.init->rate_max[VDD_DIG_NOMINAL] = 480000000;
	csi1_clk_src.clkr.hw.init->rate_max[VDD_DIG_NOMINAL] = 480000000;
	csi2_clk_src.clkr.hw.init->rate_max[VDD_DIG_NOMINAL] = 480000000;
	csi3_clk_src.clkr.hw.init->rate_max[VDD_DIG_NOMINAL] = 480000000;
}

static int mmcc_msm8998_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct regmap *regmap;
	bool is_v2 = 0;

	regmap = qcom_cc_map(pdev, &mmcc_msm8998_desc);
	if (IS_ERR(regmap))
		return PTR_ERR(regmap);

	/* Clear the DBG_CLK_DIV bits of the MMSS debug register */
	regmap_update_bits(regmap, 0x900, (3 << 17), 0);

	is_v2 = of_device_is_compatible(pdev->dev.of_node,
						"qcom,mmcc-msm8998-v2");

	vdd_dig.regulator[0] = devm_regulator_get(&pdev->dev, "vdd_dig");
	if (IS_ERR(vdd_dig.regulator[0])) {
		if (PTR_ERR(vdd_dig.regulator[0]) != -EPROBE_DEFER)
			dev_err(&pdev->dev, "Unable to get vdd_dig regulator!");
		return PTR_ERR(vdd_dig.regulator[0]);
	}

	vdd_mmsscc_mx.regulator[0] = devm_regulator_get(&pdev->dev,
							"vdd_mmsscc_mx");
	if (IS_ERR(vdd_mmsscc_mx.regulator[0])) {
		if (PTR_ERR(vdd_mmsscc_mx.regulator[0]) != -EPROBE_DEFER)
			dev_err(&pdev->dev, "Unable to get vdd_mmsscc_mx regulator!");
		return PTR_ERR(vdd_mmsscc_mx.regulator[0]);
	}

	if (is_v2) {
		msm_mmsscc_hamster_fixup();
		msm_mmsscc_v2_fixup();
	}

	clk_fabia_pll_configure(&mmpll0, regmap, &mmpll0_config);
	clk_fabia_pll_configure(&mmpll1, regmap, &mmpll1_config);
	clk_fabia_pll_configure(&mmpll3, regmap, &mmpll3_config);
	clk_fabia_pll_configure(&mmpll4, regmap, &mmpll4_config);
	clk_fabia_pll_configure(&mmpll5, regmap, &mmpll5_config);
	clk_fabia_pll_configure(&mmpll6, regmap, &mmpll6_config);
	clk_fabia_pll_configure(&mmpll7, regmap, &mmpll7_config);
	clk_fabia_pll_configure(&mmpll10, regmap, &mmpll10_config);

	ret = qcom_cc_really_probe(pdev, &mmcc_msm8998_desc, regmap);
	if (ret) {
		dev_err(&pdev->dev, "Failed to register MMSS clocks\n");
		return ret;
	}

	dev_info(&pdev->dev, "Registered MMSS clocks\n");

	return platform_driver_register(&mmcc_voters_msm8998_driver);
}

static struct platform_driver msm_clock_mmss_driver = {
	.probe = mmcc_msm8998_probe,
	.driver = {
		.name = "qcom,mmsscc-8998",
		.of_match_table = mmcc_msm8998_match_table,
		.owner = THIS_MODULE,
	},
};

int __init mmcc_msm8998_init(void)
{
	return platform_driver_register(&msm_clock_mmss_driver);
}
arch_initcall(mmcc_msm8998_init);
