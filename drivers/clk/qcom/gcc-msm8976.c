/*
 * Linux mainline API clocks implementation for MSM8976
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
#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/err.h>
#include <linux/bitops.h>
#include <linux/platform_device.h>
#include <linux/pm_opp.h>
#include <linux/module.h>
#include <linux/mfd/syscon.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/regmap.h>
#include <linux/reset-controller.h>
#include <dt-bindings/clock/qcom,gcc-msm8976.h>

#include "clk-alpha-pll.h"
#include "clk-branch.h"
#include "clk-debug.h"
#include "clk-voter.h"
#include "common.h"
#include "clk-pll.h"
#include "clk-regmap.h"
#include "clk-rcg.h"
#include "reset.h"
#include "gdsc.h"
#include "vdd-level-8976.h"

/* Porting starts here */

static DEFINE_VDD_REGULATORS(vdd_dig, VDD_DIG_NUM, 1, vdd_corner);
static DEFINE_VDD_REGULATORS(vdd_dig_gfx, VDD_DIG_NUM, 1, vdd_corner);
static DEFINE_VDD_REGULATORS(vdd_gfx, VDD_GFX_MAX, 1, vdd_gfx_corner);

#define F(f, s, h, m, n) { (f), (s), (2 * (h) - 1), (m), (n) }

enum {
	P_XO,
	P_XO_A,
	P_GPLL0_OUT_MAIN,
	P_GPLL0_AUX,
	P_GPLL0_AUX_ESC,
	P_GPLL0_OUT,
	P_GPLL0_OUT_M,
	P_GPLL0_OUT_MDP,
	P_GPLL2_OUT_MAIN,
	P_GPLL2_AUX,
	P_GPLL2_OUT,
	P_GPLL3_OUT_MAIN,
	P_GPLL4_OUT_MAIN,
	P_GPLL4_AUX,
	P_GPLL4_OUT,
	P_GPLL4_GFX3D,
	P_GPLL6_OUT_MAIN,
	P_GPLL6_AUX,
	P_GPLL6_OUT,
	P_GPLL6_GFX3D,
	P_DSI0PLL,
	P_DSI1PLL,
	P_DSI0PLL_BYTE,
	P_DSI1PLL_BYTE,
	P_DSI0PLL_BYTE_MUX,
	P_DSI1PLL_BYTE_MUX,
	P_BIMC,
	P_SLEEP_CLK,
};


/* MSM8956/76 - for SDCC1 */
static const struct parent_map gcc_parent_map_1[] = {
	{ P_XO, 0},
	{ P_GPLL0_OUT_MAIN, 1 },
	{ P_GPLL4_OUT, 2 },
};

static const char * const gcc_parent_names_1[] = {
	"xo",
	"gpll0_out_main",
	"gpll4_out_main",
};

static const struct parent_map gcc_parent_map_v1_1[] = {
	{ P_XO, 0},
	{ P_GPLL0_OUT_MAIN, 1 },
	{ P_GPLL2_OUT, 4 },
};

static const char * const gcc_parent_names_v1_1[] = {
	"xo",
	"gpll0_out_main",
	"gpll2_out_main",
};

static const struct parent_map gcc_parent_map_2[] = {
	{ P_XO, 0},
	{ P_GPLL0_OUT_MAIN, 1 },
	{ P_GPLL2_AUX, 3 },
	{ P_GPLL4_OUT, 2 },
};

static const char * const gcc_parent_names_2[] = {
	"xo",
	"gpll0_out_main",
	"gpll2_out_main",
	"gpll4_out_main",
};

static const struct parent_map gcc_parent_map_3[] = {
	{ P_GPLL0_OUT_MAIN, 1 },
	{ P_GPLL2_AUX, 3 },
	{ P_GPLL6_AUX, 2 },
};

static const char * const gcc_parent_names_3[] = {
	"gpll0_out_main",
	"gpll2_out_main",
	"gpll6_out_main",
};

static const struct parent_map gcc_parent_map_4[] = {
	{ P_XO, 0},
	{ P_GPLL0_OUT_MAIN, 1 },
};

static const struct parent_map gcc_parent_map_4_fs[] = {
	{ P_XO, 0},
	{ P_GPLL0_OUT, 2 },
};

static const char * const gcc_parent_names_4[] = {
	"xo",
	"gpll0_out_main",
};

static const struct parent_map gcc_parent_map_5[] = {
	{ P_XO, 0},
	{ P_GPLL4_OUT, 2 },
	{ P_GPLL6_OUT_MAIN, 1 },
};

static const char * const gcc_parent_names_5[] = {
	"xo",
	"gpll4_out_main",
	"gpll6_out_main",
};

static const struct parent_map gcc_parent_map_6[] = {
	{ P_XO, 0},
	{ P_GPLL0_OUT_MAIN, 1 },
	{ P_GPLL4_OUT_MAIN, 5 },
};

static const char * const gcc_parent_names_6[] = {
	"xo",
	"gpll0_out_main",
	"gpll4_out_main",
};

static const struct parent_map gcc_parent_map_7_mdp[] = {
	{ P_XO, 0},
	{ P_GPLL6_OUT, 3 },
	{ P_GPLL0_OUT_MDP, 6 },
};

static const char * const gcc_parent_names_7_mdp[] = {
	"xo",
	"gpll6_out_main",
	"gpll0_out_main",
};

static const struct parent_map gcc_parent_map_7[] = {
	{ P_GPLL0_OUT_MAIN, 1 },
	{ P_GPLL6_OUT, 3 },
};

static const char * const gcc_parent_names_7[] = {
	"gpll0_out_main",
	"gpll6_out_main",
};

static const struct parent_map gcc_parent_map_8[] = {
	{ P_XO, 0},
	{ P_GPLL0_OUT_MAIN, 1 },
};

static const char * const gcc_parent_names_8[] = {
	"xo",
	"gpll0_out_main",
};

static const struct parent_map gcc_parent_map_8_a[] = {
	{ P_XO_A, 0},
	{ P_GPLL0_OUT_MAIN, 1 },
};

static const char * const gcc_parent_names_8_a[] = {
	"xo_a",
	"gpll0_out_main",
};

static const struct parent_map gcc_parent_map_8_gp[] = {
	{ P_GPLL0_OUT_MAIN, 1 },
};

static const char * const gcc_parent_names_8_gp[] = {
	"gpll0_out_main",
};

static const struct parent_map gcc_parent_map_9[] = {
	{ P_XO, 0},
	{ P_GPLL6_OUT_MAIN, 6 },
};

static const char * const gcc_parent_names_9[] = {
	"xo",
	"gpll6_out_main",
};

static const struct parent_map gcc_parent_map_10[] = {
	{ P_XO, 0 },
};

static const char * const gcc_parent_names_10[] = {
	"xo",
};

static const struct parent_map gcc_parent_map_sdcc_ice[] = {
	{ P_XO, 0},
	{ P_GPLL0_OUT_M, 3 },
};

static const char * const gcc_parent_names_sdcc_ice[] = {
	"xo",
	"gpll0_out_main",
};

static const struct parent_map gcc_parent_map_cci[] = {
	{ P_XO, 0},
	{ P_GPLL0_AUX, 2 },
};

static const char * const gcc_parent_names_cci[] = {
	"xo",
	"gpll0_out_main",
};

static const struct parent_map gcc_parent_map_cpp[] = {
	{ P_XO, 0},
	{ P_GPLL0_OUT_MAIN, 1 },
	{ P_GPLL4_AUX, 3 },
};

static const char * const gcc_parent_names_cpp[] = {
	"xo",
	"gpll0_out_main",
	"gpll4_out_main",
};

static const struct parent_map gcc_parent_map_mdss_pix0[] = {
	{ P_XO, 0 },
	{ P_DSI0PLL, 1 },
};
static const char * const gcc_parent_names_mdss_pix0[] = {
	"xo",
	"dsi_pll0_pixel_clk_src",
};

static const struct parent_map gcc_parent_map_mdss_pix1[] = {
	{ P_XO, 0 },
	{ P_DSI0PLL, 3 },
	{ P_DSI1PLL, 1 },
};

static const char * const gcc_parent_names_mdss_pix1[] = {
	"xo",
	"dsi_pll0_pixel_clk_src",
	"dsi_pll1_pixel_clk_src",
};

static const struct parent_map gcc_parent_map_mdss_byte0[] = {
	{ P_XO, 0 },
	{ P_DSI0PLL_BYTE, 1 },
};

static const char * const gcc_parent_names_mdss_byte0[] = {
	"xo",
	"dsi_pll0_byte_clk_src",
};

static const struct parent_map gcc_parent_map_mdss_byte1[] = {
	{ P_XO, 0 },
	{ P_DSI0PLL_BYTE, 3 },
	{ P_DSI1PLL_BYTE, 1 },
};

static const char * const gcc_parent_names_mdss_byte1[] = {
	"xo",
	"dsi_pll0_byte_clk_src",
	"dsi_pll1_byte_clk_src",
};

static const struct parent_map gcc_parent_map_gfx3d[] = {
	{ P_XO, 0 },
	{ P_GPLL0_OUT_MAIN, 1 },
	{ P_GPLL4_GFX3D, 5 },
	{ P_GPLL6_GFX3D, 3 },
};

static const char * const gcc_parent_names_gfx3d[] = {
	"xo",
	"gpll0_out_main",
	"gpll4_out_main",
	"gpll6_out_main",
};

static struct clk_fixed_factor wcnss_m_clk = {
	.hw.init = &(struct clk_init_data){
		.name = "wcnss_m_clk",
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

static struct clk_fixed_factor xo_a = {
	.mult = 1,
	.div = 1,
	.hw.init = &(struct clk_init_data){
		.name = "xo_a",
		.parent_names = (const char *[]){ "cxo_a" },
		.num_parents = 1,
		.ops = &clk_fixed_factor_ops,
	},
};

static struct clk_pll gpll0 = {
	.l_reg = 0x21004,
	.m_reg = 0x21008,
	.n_reg = 0x2100c,
	.config_reg = 0x21014,
	.mode_reg = 0x21000,
	.status_reg = 0x2101c,
	.status_bit = 17,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "gpll0",
		.parent_names = (const char *[]){ "xo" },
		.num_parents = 1,
		.ops = &clk_pll_ops,
	},
};

static unsigned int gpll0_voter;

static struct clk_pll_acpu_vote gpll0_out_main = {
	.soft_voter = &gpll0_voter,
	.soft_voter_mask = PLL_SOFT_VOTE_PRIMARY,
	.clkr = {
		.enable_reg = 0x45000,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "gpll0_out_main",
			.parent_names = (const char *[]){ "gpll0" },
			.num_parents = 1,
			.ops = &clk_pll_vote_ops,
		},
	},
};

static struct clk_pll_acpu_vote gpll0_ao_out_main = {
	.soft_voter = &gpll0_voter,
	.soft_voter_mask = PLL_SOFT_VOTE_CPU,
	.clkr = {
		.enable_reg = 0x45000,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "gpll0_ao_out_main",
			.parent_names = (const char *[]){ "gpll0" },
			.num_parents = 1,
			.ops = &clk_pll_vote_ops,
		},
	},
};

static struct clk_pll gpll2 = {
	.l_reg = 0x4A004,
	.m_reg = 0x4A008,
	.n_reg = 0x4A00c,
	.config_reg = 0x4A014,
	.mode_reg = 0x4A000,
	.status_reg = 0x4A01c,
	.status_bit = 17,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "gpll2",
		.parent_names = (const char *[]){ "xo" },
		.num_parents = 1,
		.ops = &clk_pll_ops,
	},
};

static struct clk_regmap gpll2_out_main = {
	.enable_reg = 0x45000,
	.enable_mask = BIT(2),
	.hw.init = &(struct clk_init_data){
		.name = "gpll2_out_main",
		.parent_names = (const char *[]){ "gpll2" },
		.num_parents = 1,
		.ops = &clk_pll_vote_ops,
	},
};

#define F_GPLL(f, l, m, n) { (f), (l), (m), (n), 0 }

static struct pll_freq_tbl gpll3_freq_tbl[] = {
	F_GPLL(1100000000, 57, 7, 24),
};

static struct clk_pll gpll3 = {
	.l_reg		= 0x22004,
	.m_reg		= 0x22008,
	.n_reg		= 0x2200c,
	.config_reg	= 0x22010,
	.mode_reg	= 0x22000,
	.status_reg	= 0x22024,
	.status_bit	= 17,
	.freq_tbl	= gpll3_freq_tbl,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "gpll3",
		.parent_names = (const char*[]) { "xo" },
		.num_parents = 1,
		.ops = &clk_pll_ops,
	},
};

static struct clk_regmap gpll3_out_main = {
	.enable_reg = 0x45000,
	.enable_mask = BIT(4),
	.hw.init = &(struct clk_init_data){
		.name = "gpll3_out_main",
		.parent_names = (const char *[]){ "gpll3" },
		.num_parents = 1,
		.ops = &clk_pll_vote_ops,
	},
};

/* GPLL3 at 1100MHz, main output enabled. */
static struct pll_config gpll3_config = {
	.l = 57,
	.m = 7,
	.n = 24,
	.vco_val = 0x0,
	.vco_mask = 0x3 << 20,
	.pre_div_val = 0x0,
	.pre_div_mask = 0x7 << 12,
	.post_div_val = 0x0,
	.post_div_mask = 0x3 << 8,
	.mn_ena_val = BIT(24),
	.mn_ena_mask = BIT(24),
	//.main_output_val = BIT(0),
	.main_output_mask = BIT(0),
	//.aux_output_val = BIT(1),
	.aux_output_mask = BIT(1),
};

static struct clk_pll gpll4 = {
	.l_reg = 0x24004,
	.m_reg = 0x24008,
	.n_reg = 0x2400c,
	.config_reg = 0x24018,
	.mode_reg = 0x24000,
	.status_reg = 0x24024,
	.status_bit = 17,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "gpll4",
		.parent_names = (const char *[]){ "xo" },
		.num_parents = 1,
		.ops = &clk_pll_ops,
	},
};

static struct clk_regmap gpll4_out_main = {
	.enable_reg = 0x45000,
	.enable_mask = BIT(5),
	.hw.init = &(struct clk_init_data){
		.name = "gpll4_out_main",
		.parent_names = (const char *[]){ "gpll4" },
		.num_parents = 1,
		.ops = &clk_pll_vote_ops,
	},
};

static struct clk_pll gpll6 = {
	.mode_reg = 0x37000,
	.l_reg = 0x37004,
	.m_reg = 0x37008,
	.n_reg = 0x3700c,
	.config_reg = 0x37014,
	.status_reg = 0x3701c,
	.status_bit = 17,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "gpll6",
		.parent_names = (const char *[]){ "xo" },
		.num_parents = 1,
		.ops = &clk_pll_ops,
	},
};

static struct clk_regmap gpll6_out_main = {
	.enable_reg = 0x45000,
	.enable_mask = BIT(7),
	.hw.init = &(struct clk_init_data){
		.name = "gpll6_out_main",
		.parent_names = (const char *[]){ "gpll6" },
		.num_parents = 1,
		.flags = CLK_ENABLE_HAND_OFF,
		.ops = &clk_pll_vote_ops,
	},
};

static const struct freq_tbl ftbl_aps_0_clk_src[] = {
	F(  19200000,            P_XO,    1,    0,     0),
	F( 300000000,     P_GPLL4_OUT,    4,    0,     0),
	F( 540000000,P_GPLL6_OUT_MAIN,    2,    0,     0),
	{ }
};

static struct clk_rcg2 aps_0_clk_src = {
	.cmd_rcgr = 0x78008,
	.hid_width = 5,
	.parent_map = gcc_parent_map_5,
	.freq_tbl = ftbl_aps_0_clk_src,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "aps_0_clk_src",
		.parent_names = gcc_parent_names_5,
		.num_parents = ARRAY_SIZE(gcc_parent_names_5),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP2(LOWER, 300000000, NOMINAL, 540000000),
	},
};

static const struct freq_tbl ftbl_aps_1_clk_src[] = {
	F(  19200000,            P_XO,    1,    0,     0),
	F( 300000000,     P_GPLL4_OUT,    4,    0,     0),
	F( 540000000,P_GPLL6_OUT_MAIN,    2,    0,     0),
	{ }
};

static struct clk_rcg2 aps_1_clk_src = {
	.cmd_rcgr = 0x79008,
	.hid_width = 5,
	.parent_map = gcc_parent_map_5,
	.freq_tbl = ftbl_aps_1_clk_src,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "aps_1_clk_src",
		.parent_names = gcc_parent_names_5,
		.num_parents = ARRAY_SIZE(gcc_parent_names_5),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP2(LOWER, 300000000, NOMINAL, 540000000),
	},
};

static const struct freq_tbl ftbl_apss_ahb_clk_src[] = {
	F(  19200000,          P_XO_A,    1,    0,     0),
	F(  50000000,P_GPLL0_OUT_MAIN,   16,    0,     0),
	F(  88890000,P_GPLL0_OUT_MAIN,    9,    0,     0),
	F( 100000000,P_GPLL0_OUT_MAIN,    8,    0,     0),
	{ }
};

static struct clk_rcg2 apss_ahb_clk_src = {
	.cmd_rcgr = 0x46000,
	.hid_width = 5,
	.parent_map = gcc_parent_map_8_a,
	.freq_tbl = ftbl_apss_ahb_clk_src,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "apss_ahb_clk_src",
		.parent_names = gcc_parent_names_8_a,
		.num_parents = ARRAY_SIZE(gcc_parent_names_8_a),
		.flags = CLK_ENABLE_HAND_OFF,
		.ops = &clk_rcg2_ops,
	},
};

static const struct freq_tbl ftbl_blsp_i2c_apps_clk_src[] = {
	F(  19200000,            P_XO,    1,    0,     0),
	F(  50000000,P_GPLL0_OUT_MAIN,   16,    0,     0),
	{ }
};

static struct clk_rcg2 blsp1_qup1_i2c_apps_clk_src = {
	.cmd_rcgr = 0x0200C,
	.hid_width = 5,
	.parent_map = gcc_parent_map_8,
	.freq_tbl = ftbl_blsp_i2c_apps_clk_src,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "blsp1_qup1_i2c_apps_clk_src",
		.parent_names = gcc_parent_names_8,
		.num_parents = ARRAY_SIZE(gcc_parent_names_8),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP1(LOWER, 50000000),
	},
};

static const struct freq_tbl ftbl_blsp_spi_apps_clk_src[] = {
	F(    960000,            P_XO,   10,    1,     2),
	F(   4800000,            P_XO,    4,    0,     0),
	F(   9600000,            P_XO,    2,    0,     0),
	F(  16000000,P_GPLL0_OUT_MAIN,   10,    1,     5),
	F(  19200000,            P_XO,    1,    0,     0),
	F(  25000000,P_GPLL0_OUT_MAIN,   16,    1,     2),
	F(  50000000,P_GPLL0_OUT_MAIN,   16,    0,     0),
	{ }
};

static struct clk_rcg2 blsp1_qup1_spi_apps_clk_src = {
	.cmd_rcgr = 0x02024,
	.mnd_width = 8,
	.hid_width = 5,
	.parent_map = gcc_parent_map_8,
	.freq_tbl = ftbl_blsp_spi_apps_clk_src,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "blsp1_qup1_spi_apps_clk_src",
		.parent_names = gcc_parent_names_8,
		.num_parents = ARRAY_SIZE(gcc_parent_names_8),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP2(LOWER, 25000000, NOMINAL, 50000000),
	},
};

static struct clk_rcg2 blsp1_qup2_i2c_apps_clk_src = {
	.cmd_rcgr = 0x03000,
	.hid_width = 5,
	.parent_map = gcc_parent_map_8,
	.freq_tbl = ftbl_blsp_i2c_apps_clk_src,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "blsp1_qup2_i2c_apps_clk_src",
		.parent_names = gcc_parent_names_8,
		.num_parents = ARRAY_SIZE(gcc_parent_names_8),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP1(LOWER, 50000000),
	},
};

static struct clk_rcg2 blsp1_qup2_spi_apps_clk_src = {
	.cmd_rcgr = 0x03014,
	.mnd_width = 8,
	.hid_width = 5,
	.parent_map = gcc_parent_map_8,
	.freq_tbl = ftbl_blsp_spi_apps_clk_src,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "blsp1_qup2_spi_apps_clk_src",
		.parent_names = gcc_parent_names_8,
		.num_parents = ARRAY_SIZE(gcc_parent_names_8),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP2(LOWER, 25000000, NOMINAL, 50000000),
	},
};

static struct clk_rcg2 blsp1_qup3_i2c_apps_clk_src = {
	.cmd_rcgr = 0x04000,
	.hid_width = 5,
	.parent_map = gcc_parent_map_8,
	.freq_tbl = ftbl_blsp_i2c_apps_clk_src,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "blsp1_qup3_i2c_apps_clk_src",
		.parent_names = gcc_parent_names_8,
		.num_parents = ARRAY_SIZE(gcc_parent_names_8),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP1(LOWER, 50000000),
	},
};

static struct clk_rcg2 blsp1_qup3_spi_apps_clk_src = {
	.cmd_rcgr = 0x04024,
	.mnd_width = 8,
	.hid_width = 5,
	.parent_map = gcc_parent_map_8,
	.freq_tbl = ftbl_blsp_spi_apps_clk_src,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "blsp1_qup3_spi_apps_clk_src",
		.parent_names = gcc_parent_names_8,
		.num_parents = ARRAY_SIZE(gcc_parent_names_8),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP2(LOWER, 25000000, NOMINAL, 50000000),
	},
};

static struct clk_rcg2 blsp1_qup4_i2c_apps_clk_src = {
	.cmd_rcgr = 0x05000,
	.hid_width = 5,
	.parent_map = gcc_parent_map_8,
	.freq_tbl = ftbl_blsp_i2c_apps_clk_src,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "blsp1_qup4_i2c_apps_clk_src",
		.parent_names = gcc_parent_names_8,
		.num_parents = ARRAY_SIZE(gcc_parent_names_8),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP1(LOWER, 50000000),
	},
};

static struct clk_rcg2 blsp1_qup4_spi_apps_clk_src = {
	.cmd_rcgr = 0x05024,
	.mnd_width = 8,
	.hid_width = 5,
	.parent_map = gcc_parent_map_8,
	.freq_tbl = ftbl_blsp_spi_apps_clk_src,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "blsp1_qup4_spi_apps_clk_src",
		.parent_names = gcc_parent_names_8,
		.num_parents = ARRAY_SIZE(gcc_parent_names_8),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP2(LOWER, 25000000, NOMINAL, 50000000),
	},
};

static const struct freq_tbl ftbl_blsp_uart_apps_clk_src[] = {
	F(   3686400,P_GPLL0_OUT_MAIN,    1,   72, 15625),
	F(   7372800,P_GPLL0_OUT_MAIN,    1,  144, 15625),
	F(  14745600,P_GPLL0_OUT_MAIN,    1,  288, 15625),
	F(  16000000,P_GPLL0_OUT_MAIN,   10,    1,     5),
	F(  19200000,            P_XO,    1,    0,     0),
	F(  24000000,P_GPLL0_OUT_MAIN,    1,    3,   100),
	F(  25000000,P_GPLL0_OUT_MAIN,   16,    1,     2),
	F(  32000000,P_GPLL0_OUT_MAIN,    1,    1,    25),
	F(  40000000,P_GPLL0_OUT_MAIN,    1,    1,    20),
	F(  46400000,P_GPLL0_OUT_MAIN,    1,   29,   500),
	F(  48000000,P_GPLL0_OUT_MAIN,    1,    3,    50),
	F(  51200000,P_GPLL0_OUT_MAIN,    1,    8,   125),
	F(  56000000,P_GPLL0_OUT_MAIN,    1,    7,   100),
	F(  58982400,P_GPLL0_OUT_MAIN,    1, 1152, 15625),
	F(  60000000,P_GPLL0_OUT_MAIN,    1,    3,    40),
	F(  64000000,P_GPLL0_OUT_MAIN,    1,    2,    25),
	{ }
};

static struct clk_rcg2 blsp1_uart1_apps_clk_src = {
	.cmd_rcgr = 0x02044,
	.mnd_width = 16,
	.hid_width = 5,
	.parent_map = gcc_parent_map_8,
	.freq_tbl = ftbl_blsp_uart_apps_clk_src,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "blsp1_uart1_apps_clk_src",
		.parent_names = gcc_parent_names_8,
		.num_parents = ARRAY_SIZE(gcc_parent_names_8),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP2(LOWER, 32000000, NOMINAL, 64000000),
	},
};

static struct clk_rcg2 blsp1_uart2_apps_clk_src = {
	.cmd_rcgr = 0x03034,
	.mnd_width = 16,
	.hid_width = 5,
	.parent_map = gcc_parent_map_8,
	.freq_tbl = ftbl_blsp_uart_apps_clk_src,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "blsp1_uart2_apps_clk_src",
		.parent_names = gcc_parent_names_8,
		.num_parents = ARRAY_SIZE(gcc_parent_names_8),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP2(LOWER, 32000000, NOMINAL, 64000000),
	},
};

static struct clk_rcg2 blsp2_qup1_i2c_apps_clk_src = {
	.cmd_rcgr = 0x0C00C,
	.hid_width = 5,
	.parent_map = gcc_parent_map_8,
	.freq_tbl = ftbl_blsp_i2c_apps_clk_src,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "blsp2_qup1_i2c_apps_clk_src",
		.parent_names = gcc_parent_names_8,
		.num_parents = ARRAY_SIZE(gcc_parent_names_8),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP1(LOWER, 50000000),
	},
};

static struct clk_rcg2 blsp2_qup1_spi_apps_clk_src = {
	.cmd_rcgr = 0x0C024,
	.mnd_width = 8,
	.hid_width = 5,
	.parent_map = gcc_parent_map_8,
	.freq_tbl = ftbl_blsp_spi_apps_clk_src,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "blsp2_qup1_spi_apps_clk_src",
		.parent_names = gcc_parent_names_8,
		.num_parents = ARRAY_SIZE(gcc_parent_names_8),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP2(LOWER, 25000000, NOMINAL, 50000000),
	},
};

static struct clk_rcg2 blsp2_qup2_i2c_apps_clk_src = {
	.cmd_rcgr = 0x0D000,
	.hid_width = 5,
	.parent_map = gcc_parent_map_8,
	.freq_tbl = ftbl_blsp_i2c_apps_clk_src,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "blsp2_qup2_i2c_apps_clk_src",
		.parent_names = gcc_parent_names_8,
		.num_parents = ARRAY_SIZE(gcc_parent_names_8),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP1(LOWER, 50000000),
	},
};

static struct clk_rcg2 blsp2_qup2_spi_apps_clk_src = {
	.cmd_rcgr = 0x0D014,
	.mnd_width = 8,
	.hid_width = 5,
	.parent_map = gcc_parent_map_8,
	.freq_tbl = ftbl_blsp_spi_apps_clk_src,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "blsp2_qup2_spi_apps_clk_src",
		.parent_names = gcc_parent_names_8,
		.num_parents = ARRAY_SIZE(gcc_parent_names_8),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP2(LOWER, 25000000, NOMINAL, 50000000),
	},
};

static struct clk_rcg2 blsp2_qup3_i2c_apps_clk_src = {
	.cmd_rcgr = 0x0F000,
	.hid_width = 5,
	.parent_map = gcc_parent_map_8,
	.freq_tbl = ftbl_blsp_i2c_apps_clk_src,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "blsp2_qup3_i2c_apps_clk_src",
		.parent_names = gcc_parent_names_8,
		.num_parents = ARRAY_SIZE(gcc_parent_names_8),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP1(LOWER, 50000000),
	},
};

static struct clk_rcg2 blsp2_qup3_spi_apps_clk_src = {
	.cmd_rcgr = 0x0F024,
	.mnd_width = 8,
	.hid_width = 5,
	.parent_map = gcc_parent_map_8,
	.freq_tbl = ftbl_blsp_spi_apps_clk_src,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "blsp2_qup3_spi_apps_clk_src",
		.parent_names = gcc_parent_names_8,
		.num_parents = ARRAY_SIZE(gcc_parent_names_8),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP2(LOWER, 25000000, NOMINAL, 50000000),
	},
};

static struct clk_rcg2 blsp2_qup4_i2c_apps_clk_src = {
	.cmd_rcgr = 0x18000,
	.hid_width = 5,
	.parent_map = gcc_parent_map_8,
	.freq_tbl = ftbl_blsp_i2c_apps_clk_src,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "blsp2_qup4_i2c_apps_clk_src",
		.parent_names = gcc_parent_names_8,
		.num_parents = ARRAY_SIZE(gcc_parent_names_8),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP1(LOWER, 50000000),
	},
};

static struct clk_rcg2 blsp2_qup4_spi_apps_clk_src = {
	.cmd_rcgr = 0x18024,
	.mnd_width = 8,
	.hid_width = 5,
	.parent_map = gcc_parent_map_8,
	.freq_tbl = ftbl_blsp_spi_apps_clk_src,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "blsp2_qup4_spi_apps_clk_src",
		.parent_names = gcc_parent_names_8,
		.num_parents = ARRAY_SIZE(gcc_parent_names_8),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP2(LOWER, 25000000, NOMINAL, 50000000),
	},
};

static struct clk_rcg2 blsp2_uart1_apps_clk_src = {
	.cmd_rcgr = 0x0C044,
	.mnd_width = 16,
	.hid_width = 5,
	.parent_map = gcc_parent_map_8,
	.freq_tbl = ftbl_blsp_uart_apps_clk_src,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "blsp2_uart1_apps_clk_src",
		.parent_names = gcc_parent_names_8,
		.num_parents = ARRAY_SIZE(gcc_parent_names_8),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP2(LOWER, 32000000, NOMINAL, 64000000),
	},
};

static struct clk_rcg2 blsp2_uart2_apps_clk_src = {
	.cmd_rcgr = 0x0D034,
	.mnd_width = 16,
	.hid_width = 5,
	.parent_map = gcc_parent_map_8,
	.freq_tbl = ftbl_blsp_uart_apps_clk_src,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "blsp2_uart2_apps_clk_src",
		.parent_names = gcc_parent_names_8,
		.num_parents = ARRAY_SIZE(gcc_parent_names_8),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP2(LOWER, 32000000, NOMINAL, 64000000),
	},
};

static const struct freq_tbl ftbl_cci_clk_src[] = {
	F(  19200000,            P_XO,    1,    0,     0),
	F(  37500000,     P_GPLL0_AUX,    1,    3,    64),
	{ }
};

static struct clk_rcg2 cci_clk_src = {
	.cmd_rcgr = 0x51000,
	.mnd_width = 8,
	.hid_width = 5,
	.parent_map = gcc_parent_map_cci,
	.freq_tbl = ftbl_cci_clk_src,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "cci_clk_src",
		.parent_names = gcc_parent_names_cci,
		.num_parents = ARRAY_SIZE(gcc_parent_names_cci),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP2(LOWER, 19200000, NOMINAL, 37500000),
	},
};

static const struct freq_tbl ftbl_cpp_clk_src[] = {
	F( 160000000,P_GPLL0_OUT_MAIN,    5,    0,     0),
	F( 240000000,     P_GPLL4_AUX,    5,    0,     0),
	F( 320000000,P_GPLL0_OUT_MAIN,  2.5,    0,     0),
	F( 400000000,P_GPLL0_OUT_MAIN,    2,    0,     0),
	F( 480000000,     P_GPLL4_AUX,  2.5,    0,     0),
	{ }
};

static struct clk_rcg2 cpp_clk_src = {
	.cmd_rcgr = 0x58018,
	.hid_width = 5,
	.parent_map = gcc_parent_map_cpp,
	.freq_tbl = ftbl_cpp_clk_src,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "cpp_clk_src",
		.parent_names = gcc_parent_names_cpp,
		.num_parents = ARRAY_SIZE(gcc_parent_names_cpp),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP5(LOWER, 160000000, LOW, 240000000,
		NOMINAL, 320000000, NOM_PLUS, 400000000, HIGH, 480000000),
	},
};

static const struct freq_tbl ftbl_csi0_clk_src[] = {
	F( 100000000,P_GPLL0_OUT_MAIN,    8,    0,     0),
	F( 200000000,P_GPLL0_OUT_MAIN,    4,    0,     0),
	F( 266670000,P_GPLL0_OUT_MAIN,    3,    0,     0),
	{ }
};

static struct clk_rcg2 csi0_clk_src = {
	.cmd_rcgr = 0x4E020,
	.hid_width = 5,
	.parent_map = gcc_parent_map_8,
	.freq_tbl = ftbl_csi0_clk_src,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "csi0_clk_src",
		.parent_names = gcc_parent_names_8,
		.num_parents = ARRAY_SIZE(gcc_parent_names_8),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP3(LOWER, 100000000, LOW, 200000000,
		NOM_PLUS, 266670000),
	},
};

static const struct freq_tbl ftbl_csi1_clk_src[] = {
	F( 100000000,P_GPLL0_OUT_MAIN,    8,    0,     0),
	F( 200000000,P_GPLL0_OUT_MAIN,    4,    0,     0),
	F( 266670000,P_GPLL0_OUT_MAIN,    3,    0,     0),
	{ }
};

static struct clk_rcg2 csi1_clk_src = {
	.cmd_rcgr = 0x4F020,
	.hid_width = 5,
	.parent_map = gcc_parent_map_8,
	.freq_tbl = ftbl_csi1_clk_src,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "csi1_clk_src",
		.parent_names = gcc_parent_names_8,
		.num_parents = ARRAY_SIZE(gcc_parent_names_8),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP3(LOWER, 100000000, LOW, 200000000,
		NOM_PLUS, 266670000),
	},
};

static const struct freq_tbl ftbl_csi2_clk_src[] = {
	F( 100000000,P_GPLL0_OUT_MAIN,    8,    0,     0),
	F( 200000000,P_GPLL0_OUT_MAIN,    4,    0,     0),
	F( 266670000,P_GPLL0_OUT_MAIN,    3,    0,     0),
	{ }
};

static struct clk_rcg2 csi2_clk_src = {
	.cmd_rcgr = 0x3C020,
	.hid_width = 5,
	.parent_map = gcc_parent_map_8,
	.freq_tbl = ftbl_csi2_clk_src,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "csi2_clk_src",
		.parent_names = gcc_parent_names_8,
		.num_parents = ARRAY_SIZE(gcc_parent_names_8),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP3(LOWER, 100000000, LOW, 200000000,
		NOM_PLUS, 266670000),
	},
};

static const struct freq_tbl ftbl_camss_gp0_clk_src[] = {
	F( 100000000,P_GPLL0_OUT_MAIN,    8,    0,     0),
	F( 200000000,P_GPLL0_OUT_MAIN,    4,    0,     0),
	F( 266670000,P_GPLL0_OUT_MAIN,    3,    0,     0),
	{ }
};

static struct clk_rcg2 camss_gp0_clk_src = {
	.cmd_rcgr = 0x54000,
	.mnd_width = 8,
	.hid_width = 5,
	.parent_map = gcc_parent_map_8_gp,
	.freq_tbl = ftbl_camss_gp0_clk_src,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "camss_gp0_clk_src",
		.parent_names = gcc_parent_names_8_gp,
		.num_parents = ARRAY_SIZE(gcc_parent_names_8_gp),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP3(LOWER, 100000000, NOMINAL, 200000000,
		HIGH, 266670000),
	},
};

static const struct freq_tbl ftbl_camss_gp1_clk_src[] = {
	F( 100000000,P_GPLL0_OUT_MAIN,    8,    0,     0),
	F( 200000000,P_GPLL0_OUT_MAIN,    4,    0,     0),
	F( 266670000,P_GPLL0_OUT_MAIN,    3,    0,     0),
	{ }
};

static struct clk_rcg2 camss_gp1_clk_src = {
	.cmd_rcgr = 0x55000,
	.mnd_width = 8,
	.hid_width = 5,
	.parent_map = gcc_parent_map_8_gp,
	.freq_tbl = ftbl_camss_gp1_clk_src,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "camss_gp1_clk_src",
		.parent_names = gcc_parent_names_8_gp,
		.num_parents = ARRAY_SIZE(gcc_parent_names_8_gp),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP3(LOWER, 100000000, NOMINAL, 200000000,
		HIGH, 266670000),
	},
};

static const struct freq_tbl ftbl_jpeg0_clk_src[] = {
	/* Removed: NOM_PLUS ( 300000000,P_GPLL4_OUT_MAIN, 4, 0, 0), */
	F( 133330000,P_GPLL0_OUT_MAIN,    6,    0,     0),
	F( 200000000,P_GPLL0_OUT_MAIN,    4,    0,     0),
	F( 266666667,P_GPLL0_OUT_MAIN,    3,    0,     0),
	F( 320000000,P_GPLL0_OUT_MAIN,  2.5,    0,     0),
	{ }
};

static struct clk_rcg2 jpeg0_clk_src = {
	.cmd_rcgr = 0x57000,
	.hid_width = 5,
	.parent_map = gcc_parent_map_6,
	.freq_tbl = ftbl_jpeg0_clk_src,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "jpeg0_clk_src",
		.parent_names = gcc_parent_names_6,
		.num_parents = ARRAY_SIZE(gcc_parent_names_6),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP4(LOWER, 133330000, LOW, 200000000,
		NOMINAL, 266670000,
		HIGH, 320000000),
	},
};

static const struct freq_tbl ftbl_mclk_clk_src[] = {
	F(   8000000,P_GPLL0_OUT_MAIN,    1,    1,   100),
	F(  24000000,     P_GPLL6_OUT,    1,    1,    45),
	F(  66670000,P_GPLL0_OUT_MAIN,   12,    0,     0),
	{ }
};

static struct clk_rcg2 mclk0_clk_src = {
	.cmd_rcgr = 0x52000,
	.mnd_width = 8,
	.hid_width = 5,
	.parent_map = gcc_parent_map_7,
	.freq_tbl = ftbl_mclk_clk_src,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "mclk0_clk_src",
		.parent_names = gcc_parent_names_7,
		.num_parents = ARRAY_SIZE(gcc_parent_names_7),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP1(LOWER, 66670000),
	},
};

static struct clk_rcg2 mclk1_clk_src = {
	.cmd_rcgr = 0x53000,
	.mnd_width = 8,
	.hid_width = 5,
	.parent_map = gcc_parent_map_7,
	.freq_tbl = ftbl_mclk_clk_src,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "mclk1_clk_src",
		.parent_names = gcc_parent_names_7,
		.num_parents = ARRAY_SIZE(gcc_parent_names_7),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP1(LOWER, 66670000),
	},
};

static struct clk_rcg2 mclk2_clk_src = {
	.cmd_rcgr = 0x5C000,
	.mnd_width = 8,
	.hid_width = 5,
	.parent_map = gcc_parent_map_7,
	.freq_tbl = ftbl_mclk_clk_src,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "mclk2_clk_src",
		.parent_names = gcc_parent_names_7,
		.num_parents = ARRAY_SIZE(gcc_parent_names_7),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP1(LOWER, 66670000),
	},
};

static const struct freq_tbl ftbl_csi0phytimer_clk_src[] = {
	F( 100000000,P_GPLL0_OUT_MAIN,    8,    0,     0),
	F( 200000000,P_GPLL0_OUT_MAIN,    4,    0,     0),
	F( 266670000,P_GPLL0_OUT_MAIN,    3,    0,     0),
	{ }
};

static struct clk_rcg2 csi0phytimer_clk_src = {
	.cmd_rcgr = 0x4E000,
	.hid_width = 5,
	.parent_map = gcc_parent_map_8,
	.freq_tbl = ftbl_csi0phytimer_clk_src,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "csi0phytimer_clk_src",
		.parent_names = gcc_parent_names_8,
		.num_parents = ARRAY_SIZE(gcc_parent_names_8),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP3(LOWER, 100000000, LOW, 200000000,
					NOM_PLUS, 266670000),
	},
};

static const struct freq_tbl ftbl_csi1phytimer_clk_src[] = {
	F( 100000000,P_GPLL0_OUT_MAIN,    8,    0,     0),
	F( 200000000,P_GPLL0_OUT_MAIN,    4,    0,     0),
	F( 266670000,P_GPLL0_OUT_MAIN,    3,    0,     0),
	{ }
};

static struct clk_rcg2 csi1phytimer_clk_src = {
	.cmd_rcgr = 0x4F000,
	.hid_width = 5,
	.parent_map = gcc_parent_map_8,
	.freq_tbl = ftbl_csi1phytimer_clk_src,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "csi1phytimer_clk_src",
		.parent_names = gcc_parent_names_8,
		.num_parents = ARRAY_SIZE(gcc_parent_names_8),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP3(LOWER, 100000000, LOW, 200000000,
		NOM_PLUS, 266670000),
	},
};

static const struct freq_tbl ftbl_camss_top_ahb_clk_src[] = {
	F(  40000000,P_GPLL0_OUT_MAIN,   10,    1,     2),
	F(  80000000,P_GPLL0_OUT_MAIN,   10,    0,     0),
	{ }
};

static struct clk_rcg2 camss_top_ahb_clk_src = {
	.cmd_rcgr = 0x5A000,
	.mnd_width = 8,
	.hid_width = 5,
	.parent_map = gcc_parent_map_8,
	.freq_tbl = ftbl_camss_top_ahb_clk_src,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "camss_top_ahb_clk_src",
		.parent_names = gcc_parent_names_8,
		.num_parents = ARRAY_SIZE(gcc_parent_names_8),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP2(LOWER, 40000000, LOW, 80000000),
	},
};

static const struct freq_tbl ftbl_vfe0_clk_src[] = {
	F(  50000000,P_GPLL0_OUT_MAIN,   16,    0,     0),
	F(  80000000,P_GPLL0_OUT_MAIN,   10,    0,     0),
	F( 100000000,P_GPLL0_OUT_MAIN,    8,    0,     0),
	F( 133333333,P_GPLL0_OUT_MAIN,    6,    0,     0),
	F( 160000000,P_GPLL0_OUT_MAIN,    5,    0,     0),
	F( 177777778,P_GPLL0_OUT_MAIN,  4.5,    0,     0),
	F( 200000000,P_GPLL0_OUT_MAIN,    4,    0,     0),
	F( 266666667,P_GPLL0_OUT_MAIN,    3,    0,     0),
	F( 300000000,     P_GPLL4_OUT,    4,    0,     0),
	F( 320000000,P_GPLL0_OUT_MAIN,  2.5,    0,     0),
	F( 400000000,P_GPLL0_OUT_MAIN,    2,    0,     0),
	F( 466000000,     P_GPLL2_AUX,    2,    0,     0),
	{ }
};

static struct clk_rcg2 vfe0_clk_src = {
	.cmd_rcgr = 0x58000,
	.hid_width = 5,
	.parent_map = gcc_parent_map_2,
	.freq_tbl = ftbl_vfe0_clk_src,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "vfe0_clk_src",
		.parent_names = gcc_parent_names_2,
		.num_parents = ARRAY_SIZE(gcc_parent_names_2),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP5(LOWER, 160000000, LOW, 300000000,
		NOMINAL, 320000000, NOM_PLUS, 400000000,
		HIGH, 466000000),
	},
};

static const struct freq_tbl ftbl_vfe1_clk_src[] = {
	F(  50000000,P_GPLL0_OUT_MAIN,   16,    0,     0),
	F(  80000000,P_GPLL0_OUT_MAIN,   10,    0,     0),
	F( 100000000,P_GPLL0_OUT_MAIN,    8,    0,     0),
	F( 133333333,P_GPLL0_OUT_MAIN,    6,    0,     0),
	F( 160000000,P_GPLL0_OUT_MAIN,    5,    0,     0),
	F( 177777778,P_GPLL0_OUT_MAIN,  4.5,    0,     0),
	F( 200000000,P_GPLL0_OUT_MAIN,    4,    0,     0),
	F( 266666667,P_GPLL0_OUT_MAIN,    3,    0,     0),
	F( 300000000,     P_GPLL4_OUT,    4,    0,     0),
	F( 320000000,P_GPLL0_OUT_MAIN,  2.5,    0,     0),
	/*F( 400000000,P_GPLL0_OUT_MAIN,    2,    0,     0),*/
	F( 466000000,     P_GPLL2_AUX,    2,    0,     0),
	{ }
};

static struct clk_rcg2 vfe1_clk_src = {
	.cmd_rcgr = 0x58054,
	.hid_width = 5,
	.parent_map = gcc_parent_map_2,
	.freq_tbl = ftbl_vfe1_clk_src,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "vfe1_clk_src",
		.parent_names = gcc_parent_names_2,
		.num_parents = ARRAY_SIZE(gcc_parent_names_2),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP5(LOWER, 160000000, LOW, 300000000,
		NOMINAL, 320000000, NOM_PLUS, 466000000,
		HIGH, 466000000),
	},
};

static const struct freq_tbl ftbl_crypto_clk_src[] = {
	F(  50000000,P_GPLL0_OUT_MAIN,   16,    0,     0),
	F(  80000000,P_GPLL0_OUT_MAIN,   10,    0,     0),
	F( 100000000,P_GPLL0_OUT_MAIN,    8,    0,     0),
	F( 160000000,P_GPLL0_OUT_MAIN,    5,    0,     0),
	{ }
};

static struct clk_rcg2 crypto_clk_src = {
	.cmd_rcgr = 0x16004,
	.hid_width = 5,
	.parent_map = gcc_parent_map_8,
	.freq_tbl = ftbl_crypto_clk_src,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "crypto_clk_src",
		.parent_names = gcc_parent_names_8,
		.num_parents = ARRAY_SIZE(gcc_parent_names_8),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP2(LOWER, 80000000, NOMINAL, 160000000),
	},
};

static const struct freq_tbl ftbl_gp1_clk_src[] = {
	F(  19200000,            P_XO,    1,    0,     0),
	{ }
};

static struct clk_rcg2 gp1_clk_src = {
	.cmd_rcgr = 0x08004,
	.mnd_width = 8,
	.hid_width = 5,
	.parent_map = gcc_parent_map_10,
	.freq_tbl = ftbl_gp1_clk_src,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "gp1_clk_src",
		.parent_names = gcc_parent_names_10,
		.num_parents = 1,
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP2(LOWER, 100000000, NOMINAL, 200000000),
	},
};

static const struct freq_tbl ftbl_gp2_clk_src[] = {
	F(  19200000,            P_XO,    1,    0,     0),
	{ }
};

static struct clk_rcg2 gp2_clk_src = {
	.cmd_rcgr = 0x09004,
	.mnd_width = 8,
	.hid_width = 5,
	.parent_map = gcc_parent_map_10,
	.freq_tbl = ftbl_gp2_clk_src,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "gp2_clk_src",
		.parent_names = gcc_parent_names_10,
		.num_parents = 1,
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP2(LOWER, 100000000, NOMINAL, 200000000),
	},
};

static const struct freq_tbl ftbl_gp3_clk_src[] = {
	F(  19200000,            P_XO,    1,    0,     0),
	{ }
};

static struct clk_rcg2 gp3_clk_src = {
	.cmd_rcgr = 0x0A004,
	.mnd_width = 8,
	.hid_width = 5,
	.parent_map = gcc_parent_map_10,
	.freq_tbl = ftbl_gp3_clk_src,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "gp3_clk_src",
		.parent_names = gcc_parent_names_10,
		.num_parents = 1,
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP2(LOWER, 100000000, NOMINAL, 200000000),
	},
};

static struct clk_rcg2 byte0_clk_src = {
	.cmd_rcgr = 0x4D044,
	.mnd_width = 0,
	.hid_width = 5,
	.parent_map = gcc_parent_map_mdss_byte0,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "byte0_clk_src",
		.parent_names = gcc_parent_names_mdss_byte0,
		.num_parents = ARRAY_SIZE(gcc_parent_names_mdss_byte0),
		.ops = &clk_byte2_ops,
		.flags = CLK_SET_RATE_PARENT | CLK_GET_RATE_NOCACHE,
		VDD_DIG_FMAX_MAP3(LOWER, 125000000, LOW, 161250000,
				NOMINAL, 187500000),
	},
};

static struct clk_rcg2 byte1_clk_src = {
	.cmd_rcgr = 0x4D0B0,
	.mnd_width = 0,
	.hid_width = 5,
	.parent_map = gcc_parent_map_mdss_byte1,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "byte1_clk_src",
		.parent_names = gcc_parent_names_mdss_byte1,
		.num_parents = ARRAY_SIZE(gcc_parent_names_mdss_byte1),
		.ops = &clk_byte2_ops,
		.flags = CLK_SET_RATE_PARENT | CLK_GET_RATE_NOCACHE,
		VDD_DIG_FMAX_MAP3(LOWER, 125000000, LOW, 161250000,
				NOMINAL, 187500000),
	},
};

static const struct freq_tbl ftbl_esc0_clk_src[] = {
	F(  19200000,            P_XO,    1,    0,     0),
	{ }
};

static struct clk_rcg2 esc0_clk_src = {
	.cmd_rcgr = 0x4D05C,
	.hid_width = 5,
	.parent_map = gcc_parent_map_10,
	.freq_tbl = ftbl_esc0_clk_src,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "esc0_clk_src",
		.parent_names = gcc_parent_names_10,
		.num_parents = 1,
		.ops = &clk_esc_ops,
		VDD_DIG_FMAX_MAP1(LOWER, 19200000),
	},
};

static const struct freq_tbl ftbl_esc1_clk_src[] = {
	F(  19200000,            P_XO,    1,    0,     0),
	{ }
};

static struct clk_rcg2 esc1_clk_src = {
	.cmd_rcgr = 0x4D0A8,
	.hid_width = 5,
	.parent_map = gcc_parent_map_10,
	.freq_tbl = ftbl_esc1_clk_src,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "esc1_clk_src",
		.parent_names = gcc_parent_names_10,
		.num_parents = 1,
		.ops = &clk_esc_ops,
		VDD_DIG_FMAX_MAP1(LOWER, 19200000),
	},
};

static const struct freq_tbl ftbl_mdp_clk_src[] = {
	F(  50000000, P_GPLL0_OUT_MDP,   16,    0,     0),
	F(  80000000, P_GPLL0_OUT_MDP,   10,    0,     0),
	F( 100000000, P_GPLL0_OUT_MDP,    8,    0,     0),
	F( 145454545, P_GPLL0_OUT_MDP,  5.5,    0,     0),
	F( 160000000, P_GPLL0_OUT_MDP,    5,    0,     0),
	F( 177777778, P_GPLL0_OUT_MDP,  4.5,    0,     0),
	F( 200000000, P_GPLL0_OUT_MDP,    4,    0,     0),
	F( 270000000,     P_GPLL6_OUT,    4,    0,     0),
	F( 320000000, P_GPLL0_OUT_MDP,  2.5,    0,     0),
	F( 360000000,     P_GPLL6_OUT,    3,    0,     0),
	{ }
};

static struct clk_rcg2 mdp_clk_src = {
	.cmd_rcgr = 0x4D014,
	.hid_width = 5,
	.parent_map = gcc_parent_map_7_mdp,
	.freq_tbl = ftbl_mdp_clk_src,
	.enable_safe_config = true,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "mdp_clk_src",
		.parent_names = gcc_parent_names_7_mdp,
		.num_parents = ARRAY_SIZE(gcc_parent_names_7_mdp),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP4(LOWER, 177780000, LOW, 270000000,
		NOMINAL, 320000000, HIGH, 360000000),
	},
};

static struct clk_rcg2 pclk0_clk_src = {
	.cmd_rcgr = 0x4D000,
	.mnd_width = 8,
	.hid_width = 5,
	.parent_map = gcc_parent_map_mdss_pix0,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "pclk0_clk_src",
		.parent_names = gcc_parent_names_mdss_pix0,
		.num_parents = ARRAY_SIZE(gcc_parent_names_mdss_pix0),
		.ops = &clk_pixel_ops,
		.flags = CLK_SET_RATE_PARENT | CLK_GET_RATE_NOCACHE,
		VDD_DIG_FMAX_MAP3(LOWER, 166670000, LOW, 215000000,
				NOMINAL, 250000000),
	},
};

static struct clk_rcg2 pclk1_clk_src = {
	.cmd_rcgr = 0x4D0B8,
	.mnd_width = 8,
	.hid_width = 5,
	.parent_map = gcc_parent_map_mdss_pix1,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "pclk1_clk_src",
		.parent_names = gcc_parent_names_mdss_pix1,
		.num_parents = ARRAY_SIZE(gcc_parent_names_mdss_pix1),
		.ops = &clk_pixel_ops,
		.flags = CLK_SET_RATE_PARENT | CLK_GET_RATE_NOCACHE,
		VDD_DIG_FMAX_MAP3(LOWER, 166670000, LOW, 215000000,
				NOMINAL, 250000000),
	},
};

static const struct freq_tbl ftbl_vsync_clk_src[] = {
	F(  19200000,            P_XO,    1,    0,     0),
	{ }
};

static struct clk_rcg2 vsync_clk_src = {
	.cmd_rcgr = 0x4D02C,
	.hid_width = 5,
	.parent_map = gcc_parent_map_10,
	.freq_tbl = ftbl_vsync_clk_src,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "vsync_clk_src",
		.parent_names = gcc_parent_names_10,
		.num_parents = 1,
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP1(LOWER, 19200000),
	},
};

static const struct freq_tbl ftbl_gfx3d_clk_src[] = {
	F(  19200000,            P_XO,    1,    0,     0),
	F(  50000000,P_GPLL0_OUT_MAIN,   16,    0,     0),
	F(  80000000,P_GPLL0_OUT_MAIN,   10,    0,     0),
	F( 100000000,P_GPLL0_OUT_MAIN,    8,    0,     0),
	F( 133333333,P_GPLL0_OUT_MAIN,    6,    0,     0),
	F( 160000000,P_GPLL0_OUT_MAIN,    5,    0,     0),
	F( 200000000,P_GPLL0_OUT_MAIN,    4,    0,     0),
	F( 228571429,P_GPLL0_OUT_MAIN,  3.5,    0,     0),
	F( 240000000,   P_GPLL6_GFX3D,  4.5,    0,     0),
	F( 266666667,P_GPLL0_OUT_MAIN,    3,    0,     0),
	F( 300000000,   P_GPLL4_GFX3D,    4,    0,     0),
	F( 360000000,   P_GPLL6_GFX3D,    3,    0,     0),
	F( 400000000,P_GPLL0_OUT_MAIN,    2,    0,     0),
	F( 432000000,   P_GPLL6_GFX3D,  2.5,    0,     0),
	F( 480000000,   P_GPLL4_GFX3D,  2.5,    0,     0),
	F( 540000000,   P_GPLL6_GFX3D,    2,    0,     0),
	F( 600000000,   P_GPLL4_GFX3D,    2,    0,     0),
	{ }
};

static struct clk_init_data gfx3d_clk_params = {
	.name = "gfx3d_clk_src",
	.parent_names = gcc_parent_names_gfx3d,
	.num_parents = ARRAY_SIZE(gcc_parent_names_gfx3d),
	.ops = &clk_rcg2_ops,
	.vdd_class = &vdd_gfx,
	//.flags = CLK_SET_RATE_PARENT,
	VDD_GFX_FMAX_MAP9(MIN_SVS,	133333333,
			LOW_SVS,	200000000,
			SVS_MINUS,	266666667,
			SVS,		300000000,
			SVS_PLUS,	360000000,
			NOMINAL,	432000000,
			TURBO,		480000000,
			TURBO_L1,	540000000,
			SUPER_TURBO,	600000000),
};

static struct clk_rcg2 gfx3d_clk_src = {
	.cmd_rcgr = 0x59000,
	.hid_width = 5,
	.parent_map = gcc_parent_map_gfx3d,
	.freq_tbl = ftbl_gfx3d_clk_src,
	.flags = FORCE_ENABLE_RCG,
	.clkr.hw.init = &gfx3d_clk_params,
};

static const struct freq_tbl ftbl_pdm2_clk_src[] = {
	F(  64000000,P_GPLL0_OUT_MAIN, 12.5,    0,     0),
	{ }
};

static struct clk_rcg2 pdm2_clk_src = {
	.cmd_rcgr = 0x44010,
	.hid_width = 5,
	.parent_map = gcc_parent_map_8,
	.freq_tbl = ftbl_pdm2_clk_src,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "pdm2_clk_src",
		.parent_names = gcc_parent_names_8,
		.num_parents = ARRAY_SIZE(gcc_parent_names_8),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP1(LOWER, 64000000),
	},
};

static const struct freq_tbl ftbl_rbcpr_gfx_clk_src[] = {
	F(  19200000,            P_XO,    1,    0,     0),
	F(  50000000,P_GPLL0_OUT_MAIN,   16,    0,     0),
	{ }
};

static struct clk_rcg2 rbcpr_gfx_clk_src = {
	.cmd_rcgr = 0x3A00C,
	.hid_width = 5,
	.parent_map = gcc_parent_map_8,
	.freq_tbl = ftbl_rbcpr_gfx_clk_src,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "rbcpr_gfx_clk_src",
		.parent_names = gcc_parent_names_8,
		.num_parents = ARRAY_SIZE(gcc_parent_names_8),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP2(LOWER, 50000000, NOMINAL, 100000000),
	},
};

static const struct freq_tbl ftbl_sdcc1_apps_clk_src[] = {
	F(    144000,            P_XO,   16,    3,    25),
	F(    400000,            P_XO,   12,    1,     4),
	F(  20000000,P_GPLL0_OUT_MAIN,   10,    1,     4),
	F(  25000000,P_GPLL0_OUT_MAIN,   16,    1,     2),
	F(  50000000,P_GPLL0_OUT_MAIN,   16,    0,     0),
	F( 100000000,P_GPLL0_OUT_MAIN,    8,    0,     0),
	F( 177777778,P_GPLL0_OUT_MAIN,  4.5,    0,     0),
	F( 200000000,P_GPLL0_OUT_MAIN,    4,    0,     0),
	F( 342850000,     P_GPLL4_OUT,  3.5,    0,     0),
	F( 400000000,     P_GPLL4_OUT,    3,    0,     0),
	{ }
};

static const struct freq_tbl ftbl_sdcc1_v1_apps_clk_src[] = {
	F(    144000,            P_XO,   16,    3,    25),
	F(    400000,            P_XO,   12,    1,     4),
	F(  20000000,P_GPLL0_OUT_MAIN,   10,    1,     4),
	F(  25000000,P_GPLL0_OUT_MAIN,   16,    1,     2),
	F(  50000000,P_GPLL0_OUT_MAIN,   16,    0,     0),
	F( 100000000,P_GPLL0_OUT_MAIN,    8,    0,     0),
	F( 177777778,P_GPLL0_OUT_MAIN,  4.5,    0,     0),
	F( 200000000,P_GPLL0_OUT_MAIN,    4,    0,     0),
	F( 186400000,     P_GPLL2_OUT,    5,    0,     0),
	F( 372800000,     P_GPLL2_OUT,  2.5,    0,     0),
	{ }
};

static struct clk_rcg2 sdcc1_apps_clk_src = {
	.cmd_rcgr = 0x42004,
	.mnd_width = 8,
	.hid_width = 5,
	.parent_map = gcc_parent_map_1,
	.freq_tbl = ftbl_sdcc1_apps_clk_src,
	.enable_safe_config = true,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "sdcc1_apps_clk_src",
		.parent_names = gcc_parent_names_1,
		.num_parents = ARRAY_SIZE(gcc_parent_names_1),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP2(LOWER, 100000000, NOMINAL, 400000000),
	},
};

static struct clk_rcg2 sdcc1_apps_clk_src_v1 = {
	.cmd_rcgr = 0x42004,
	.mnd_width = 8,
	.hid_width = 5,
	.parent_map = gcc_parent_map_v1_1,
	.freq_tbl = ftbl_sdcc1_v1_apps_clk_src,
	.enable_safe_config = true,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "sdcc1_apps_clk_src",
		.parent_names = gcc_parent_names_v1_1,
		.num_parents = ARRAY_SIZE(gcc_parent_names_v1_1),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP2(LOWER, 100000000, NOMINAL, 400000000),
	},
};

static const struct freq_tbl ftbl_sdcc1_ice_core_clk_src[] = {
	F( 100000000,   P_GPLL0_OUT_M,    8,    0,     0),
	F( 200000000,   P_GPLL0_OUT_M,    4,    0,     0),
	{ }
};

static struct clk_rcg2 sdcc1_ice_core_clk_src = {
	.cmd_rcgr = 0x5D000,
	.mnd_width = 8,
	.hid_width = 5,
	.parent_map = gcc_parent_map_sdcc_ice,
	.enable_safe_config = true,
	.freq_tbl = ftbl_sdcc1_ice_core_clk_src,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "sdcc1_ice_core_clk_src",
		.parent_names = gcc_parent_names_sdcc_ice,
		.num_parents = ARRAY_SIZE(gcc_parent_names_sdcc_ice),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP2(LOWER, 100000000, NOMINAL, 200000000),
	},
};

static const struct freq_tbl ftbl_sdcc2_4_apps_clk_src[] = {
	F(    144000,            P_XO,   16,    3,    25),
	F(    400000,            P_XO,   12,    1,     4),
	F(  20000000,P_GPLL0_OUT_MAIN,   10,    1,     4),
	F(  25000000,P_GPLL0_OUT_MAIN,   16,    1,     2),
	F(  40000000,P_GPLL0_OUT_MAIN,   10,    1,     2),
	F(  50000000,P_GPLL0_OUT_MAIN,   16,    0,     0),
	F(  80000000,P_GPLL0_OUT_MAIN,   10,    0,     0),
	F( 100000000,P_GPLL0_OUT_MAIN,    8,    0,     0),
	F( 177777778,P_GPLL0_OUT_MAIN,  4.5,    0,     0),
	F( 200000000,P_GPLL0_OUT_MAIN,    4,    0,     0),
	{ }
};

static struct clk_rcg2 sdcc2_apps_clk_src = {
	.cmd_rcgr = 0x43004,
	.mnd_width = 8,
	.hid_width = 5,
	.parent_map = gcc_parent_map_4,
	.freq_tbl = ftbl_sdcc2_4_apps_clk_src,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "sdcc2_apps_clk_src",
		.parent_names = gcc_parent_names_4,
		.num_parents = ARRAY_SIZE(gcc_parent_names_4),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP2(LOWER, 50000000, NOMINAL, 200000000),
	},
};

static struct clk_rcg2 sdcc3_apps_clk_src = {
	.cmd_rcgr = 0x39004,
	.mnd_width = 8,
	.hid_width = 5,
	.parent_map = gcc_parent_map_4,
	.freq_tbl = ftbl_sdcc2_4_apps_clk_src,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "sdcc3_apps_clk_src",
		.parent_names = gcc_parent_names_4,
		.num_parents = ARRAY_SIZE(gcc_parent_names_4),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP2(LOWER, 50000000, NOMINAL, 200000000),
	},
};

static const struct freq_tbl ftbl_usb_fs_ic_clk_src[] = {
	F(  60000000,P_GPLL6_OUT_MAIN,    6,    1,     3),
	{ }
};

static struct clk_rcg2 usb_fs_ic_clk_src = {
	.cmd_rcgr = 0x3F034,
	.mnd_width = 8,
	.hid_width = 5,
	.parent_map = gcc_parent_map_9,
	.freq_tbl = ftbl_usb_fs_ic_clk_src,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "usb_fs_ic_clk_src",
		.parent_names = gcc_parent_names_9,
		.num_parents = ARRAY_SIZE(gcc_parent_names_9),
		.ops = &clk_rcg2_ops,
		.flags = CLK_ENABLE_HAND_OFF,
		VDD_DIG_FMAX_MAP1(LOWER, 60000000),
	},
};

static const struct freq_tbl ftbl_usb_fs_system_clk_src[] = {
	F(  64000000,     P_GPLL0_OUT, 12.5,    0,     0),
	{ }
};

static struct clk_rcg2 usb_fs_system_clk_src = {
	.cmd_rcgr = 0x3F010,
	.mnd_width = 8,
	.hid_width = 5,
	.parent_map = gcc_parent_map_4_fs,
	.freq_tbl = ftbl_usb_fs_system_clk_src,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "usb_fs_system_clk_src",
		.parent_names = gcc_parent_names_4,
		.num_parents = ARRAY_SIZE(gcc_parent_names_4),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP1(LOWER, 64000000),
	},
};

static const struct freq_tbl ftbl_usb_hs_system_clk_src[] = {
	F(  57140000,P_GPLL0_OUT_MAIN,   14,    0,     0),
	F( 100000000,P_GPLL0_OUT_MAIN,    8,    0,     0),
	F( 133333333,P_GPLL0_OUT_MAIN,    6,    0,     0),
	F( 177780000,P_GPLL0_OUT_MAIN,  4.5,    0,     0),
	{ }
};

static struct clk_rcg2 usb_hs_system_clk_src = {
	.cmd_rcgr = 0x41010,
	.hid_width = 5,
	.parent_map = gcc_parent_map_4,
	.freq_tbl = ftbl_usb_hs_system_clk_src,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "usb_hs_system_clk_src",
		.parent_names = gcc_parent_names_4,
		.num_parents = ARRAY_SIZE(gcc_parent_names_4),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP3(LOWER, 57140000, NOMINAL, 133330000,
		HIGH, 177780000),
	},
};

static const struct freq_tbl ftbl_vcodec0_clk_src[] = {
	F(  72727272,P_GPLL0_OUT_MAIN,   11,    0,     0),
	F(  80000000,P_GPLL0_OUT_MAIN,   10,    0,     0),
	F( 100000000,P_GPLL0_OUT_MAIN,    8,    0,     0),
	F( 133333333,P_GPLL0_OUT_MAIN,    6,    0,     0),
	F( 228571428,P_GPLL0_OUT_MAIN,  3.5,    0,     0),
	F( 310666666,     P_GPLL2_AUX,    3,    0,     0),
	F( 360000000,     P_GPLL6_AUX,    3,    0,     0),
	F( 400000000,P_GPLL0_OUT_MAIN,    2,    0,     0),
	F( 466000000,     P_GPLL2_AUX,    2,    0,     0),
	{ }
};

static struct clk_rcg2 vcodec0_clk_src = {
	.cmd_rcgr = 0x4C000,
	.mnd_width = 8,
	.hid_width = 5,
	.parent_map = gcc_parent_map_3,
	.freq_tbl = ftbl_vcodec0_clk_src,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "vcodec0_clk_src",
		.parent_names = gcc_parent_names_3,
		.num_parents = ARRAY_SIZE(gcc_parent_names_3),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP5(LOWER, 228571428, LOW, 310666666,
		NOMINAL, 360000000, NOM_PLUS, 400000000,
		HIGH, 466000000),
	},
};

static struct clk_branch gcc_aps_0_clk = {
	.halt_reg = 0x78004,
	.clkr = {
		.enable_reg = 0x78004,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_aps_0_clk",
			.parent_names = (const char*[]) {
				"aps_0_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_aps_1_clk = {
	.halt_reg = 0x79004,
	.clkr = {
		.enable_reg = 0x79004,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_aps_1_clk",
			.parent_names = (const char*[]) {
				"aps_1_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_blsp1_qup1_i2c_apps_clk = {
	.halt_reg = 0x02008,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x02008,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_blsp1_qup1_i2c_apps_clk",
			.parent_names = (const char*[]) {
				"blsp1_qup1_i2c_apps_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_blsp1_qup1_spi_apps_clk = {
	.halt_reg = 0x02004,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x02004,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_blsp1_qup1_spi_apps_clk",
			.parent_names = (const char*[]) {
				"blsp1_qup1_spi_apps_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_blsp1_qup2_i2c_apps_clk = {
	.halt_reg = 0x03010,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x03010,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_blsp1_qup2_i2c_apps_clk",
			.parent_names = (const char*[]) {
				"blsp1_qup2_i2c_apps_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_blsp1_qup2_spi_apps_clk = {
	.halt_reg = 0x0300C,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x0300C,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_blsp1_qup2_spi_apps_clk",
			.parent_names = (const char*[]) {
				"blsp1_qup2_spi_apps_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_blsp1_qup3_i2c_apps_clk = {
	.halt_reg = 0x04020,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x04020,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_blsp1_qup3_i2c_apps_clk",
			.parent_names = (const char*[]) {
				"blsp1_qup3_i2c_apps_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_blsp1_qup3_spi_apps_clk = {
	.halt_reg = 0x0401C,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x0401C,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_blsp1_qup3_spi_apps_clk",
			.parent_names = (const char*[]) {
				"blsp1_qup3_spi_apps_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_blsp1_qup4_i2c_apps_clk = {
	.halt_reg = 0x05020,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x05020,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_blsp1_qup4_i2c_apps_clk",
			.parent_names = (const char*[]) {
				"blsp1_qup4_i2c_apps_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_blsp1_qup4_spi_apps_clk = {
	.halt_reg = 0x0501C,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x0501C,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_blsp1_qup4_spi_apps_clk",
			.parent_names = (const char*[]) {
				"blsp1_qup4_spi_apps_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_blsp1_uart1_apps_clk = {
	.halt_reg = 0x0203C,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x0203C,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_blsp1_uart1_apps_clk",
			.parent_names = (const char*[]) {
				"blsp1_uart1_apps_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_blsp1_uart2_apps_clk = {
	.halt_reg = 0x0302C,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x0302C,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_blsp1_uart2_apps_clk",
			.parent_names = (const char*[]) {
				"blsp1_uart2_apps_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT | CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_blsp2_qup1_i2c_apps_clk = {
	.halt_reg = 0x0C008,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x0C008,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_blsp2_qup1_i2c_apps_clk",
			.parent_names = (const char*[]) {
				"blsp2_qup1_i2c_apps_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_blsp2_qup1_spi_apps_clk = {
	.halt_reg = 0x0C004,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x0C004,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_blsp2_qup1_spi_apps_clk",
			.parent_names = (const char*[]) {
				"blsp2_qup1_spi_apps_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_blsp2_qup2_i2c_apps_clk = {
	.halt_reg = 0x0D010,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x0D010,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_blsp2_qup2_i2c_apps_clk",
			.parent_names = (const char*[]) {
				"blsp2_qup2_i2c_apps_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_blsp2_qup2_spi_apps_clk = {
	.halt_reg = 0x0D00C,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x0D00C,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_blsp2_qup2_spi_apps_clk",
			.parent_names = (const char*[]) {
				"blsp2_qup2_spi_apps_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_blsp2_qup3_i2c_apps_clk = {
	.halt_reg = 0x0F020,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x0F020,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_blsp2_qup3_i2c_apps_clk",
			.parent_names = (const char*[]) {
				"blsp2_qup3_i2c_apps_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_blsp2_qup3_spi_apps_clk = {
	.halt_reg = 0x0F01C,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x0F01C,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_blsp2_qup3_spi_apps_clk",
			.parent_names = (const char*[]) {
				"blsp2_qup3_spi_apps_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_blsp2_qup4_i2c_apps_clk = {
	.halt_reg = 0x18020,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x18020,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_blsp2_qup4_i2c_apps_clk",
			.parent_names = (const char*[]) {
				"blsp2_qup4_i2c_apps_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_blsp2_qup4_spi_apps_clk = {
	.halt_reg = 0x1801C,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x1801C,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_blsp2_qup4_spi_apps_clk",
			.parent_names = (const char*[]) {
				"blsp2_qup4_spi_apps_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_blsp2_uart1_apps_clk = {
	.halt_reg = 0x0C03C,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x0C03C,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_blsp2_uart1_apps_clk",
			.parent_names = (const char*[]) {
				"blsp2_uart1_apps_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_blsp2_uart2_apps_clk = {
	.halt_reg = 0x0D02C,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x0D02C,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_blsp2_uart2_apps_clk",
			.parent_names = (const char*[]) {
				"blsp2_uart2_apps_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_camss_cci_ahb_clk = {
	.halt_reg = 0x5101C,
	.clkr = {
		.enable_reg = 0x5101C,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_camss_cci_ahb_clk",
			.parent_names = (const char*[]) {
				"camss_top_ahb_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_camss_cci_clk = {
	.halt_reg = 0x51018,
	.clkr = {
		.enable_reg = 0x51018,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_camss_cci_clk",
			.parent_names = (const char*[]) {
				"cci_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_camss_cpp_ahb_clk = {
	.halt_reg = 0x58040,
	//.halt_check = BRANCH_VOTED,
	.clkr = {
		.enable_reg = 0x58040,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_camss_cpp_ahb_clk",
			.parent_names = (const char*[]) {
				"camss_top_ahb_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_camss_cpp_axi_clk = {
	.halt_reg = 0x58064,
	//.halt_check = BRANCH_VOTED,
	.clkr = {
		.enable_reg = 0x58064,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_camss_cpp_axi_clk",
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_camss_cpp_clk = {
	.halt_reg = 0x5803C,
	.clkr = {
		.enable_reg = 0x5803C,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_camss_cpp_clk",
			.parent_names = (const char*[]) {
				"cpp_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_camss_csi0_ahb_clk = {
	.halt_reg = 0x4E040,
	.clkr = {
		.enable_reg = 0x4E040,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_camss_csi0_ahb_clk",
			.parent_names = (const char*[]) {
				"camss_top_ahb_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_camss_csi0_clk = {
	.halt_reg = 0x4E03C,
	.clkr = {
		.enable_reg = 0x4E03C,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_camss_csi0_clk",
			.parent_names = (const char*[]) {
				"csi0_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_camss_csi0phy_clk = {
	.halt_reg = 0x4E048,
	.clkr = {
		.enable_reg = 0x4E048,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_camss_csi0phy_clk",
			.parent_names = (const char*[]) {
				"csi0_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_camss_csi0pix_clk = {
	.halt_reg = 0x4E058,
	.clkr = {
		.enable_reg = 0x4E058,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_camss_csi0pix_clk",
			.parent_names = (const char*[]) {
				"csi0_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_camss_csi0rdi_clk = {
	.halt_reg = 0x4E050,
	.clkr = {
		.enable_reg = 0x4E050,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_camss_csi0rdi_clk",
			.parent_names = (const char*[]) {
				"csi0_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_camss_csi1_ahb_clk = {
	.halt_reg = 0x4F040,
	.clkr = {
		.enable_reg = 0x4F040,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_camss_csi1_ahb_clk",
			.parent_names = (const char*[]) {
				"camss_top_ahb_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_camss_csi1_clk = {
	.halt_reg = 0x4F03C,
	.clkr = {
		.enable_reg = 0x4F03C,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_camss_csi1_clk",
			.parent_names = (const char*[]) {
				"csi1_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_camss_csi1phy_clk = {
	.halt_reg = 0x4F048,
	.clkr = {
		.enable_reg = 0x4F048,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_camss_csi1phy_clk",
			.parent_names = (const char*[]) {
				"csi1_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_camss_csi1pix_clk = {
	.halt_reg = 0x4F058,
	.clkr = {
		.enable_reg = 0x4F058,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_camss_csi1pix_clk",
			.parent_names = (const char*[]) {
				"csi1_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_camss_csi1rdi_clk = {
	.halt_reg = 0x4F050,
	.clkr = {
		.enable_reg = 0x4F050,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_camss_csi1rdi_clk",
			.parent_names = (const char*[]) {
				"csi1_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_camss_csi2_ahb_clk = {
	.halt_reg = 0x3C040,
	.clkr = {
		.enable_reg = 0x3C040,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_camss_csi2_ahb_clk",
			.parent_names = (const char*[]) {
				"camss_top_ahb_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_camss_csi2_clk = {
	.halt_reg = 0x3C03C,
	.clkr = {
		.enable_reg = 0x3C03C,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_camss_csi2_clk",
			.parent_names = (const char*[]) {
				"csi2_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_camss_csi2phy_clk = {
	.halt_reg = 0x3C048,
	.clkr = {
		.enable_reg = 0x3C048,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_camss_csi2phy_clk",
			.parent_names = (const char*[]) {
				"csi2_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_camss_csi2pix_clk = {
	.halt_reg = 0x3C058,
	.clkr = {
		.enable_reg = 0x3C058,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_camss_csi2pix_clk",
			.parent_names = (const char*[]) {
				"csi2_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_camss_csi2rdi_clk = {
	.halt_reg = 0x3C050,
	.clkr = {
		.enable_reg = 0x3C050,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_camss_csi2rdi_clk",
			.parent_names = (const char*[]) {
				"csi2_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_camss_csi_vfe0_clk = {
	.halt_reg = 0x58050,
	.clkr = {
		.enable_reg = 0x58050,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_camss_csi_vfe0_clk",
			.parent_names = (const char*[]) {
				"vfe0_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_camss_csi_vfe1_clk = {
	.halt_reg = 0x58074,
	.clkr = {
		.enable_reg = 0x58074,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_camss_csi_vfe1_clk",
			.parent_names = (const char*[]) {
				"vfe1_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_camss_gp0_clk = {
	.halt_reg = 0x54018,
	.clkr = {
		.enable_reg = 0x54018,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_camss_gp0_clk",
			.parent_names = (const char*[]) {
				"camss_gp0_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_camss_gp1_clk = {
	.halt_reg = 0x55018,
	.clkr = {
		.enable_reg = 0x55018,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_camss_gp1_clk",
			.parent_names = (const char*[]) {
				"camss_gp1_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_camss_ispif_ahb_clk = {
	.halt_reg = 0x50004,
	.clkr = {
		.enable_reg = 0x50004,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_camss_ispif_ahb_clk",
			.parent_names = (const char*[]) {
				"camss_top_ahb_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_camss_jpeg0_clk = {
	.halt_reg = 0x57020,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x57020,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_camss_jpeg0_clk",
			.parent_names = (const char*[]) {
				"jpeg0_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_camss_jpeg_ahb_clk = {
	.halt_reg = 0x57024,
	//.halt_check = BRANCH_VOTED,
	.clkr = {
		.enable_reg = 0x57024,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_camss_jpeg_ahb_clk",
			.parent_names = (const char*[]) {
				"camss_top_ahb_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_camss_jpeg_axi_clk = {
	.halt_reg = 0x57028,
	//.halt_check = BRANCH_VOTED,
	.clkr = {
		.enable_reg = 0x57028,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_camss_jpeg_axi_clk",
			.ops = &clk_branch2_ops,
		},
	},
};


static struct clk_branch gcc_camss_mclk0_clk = {
	.halt_reg = 0x52018,
	.clkr = {
		.enable_reg = 0x52018,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_camss_mclk0_clk",
			.parent_names = (const char*[]) {
				"mclk0_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_camss_mclk1_clk = {
	.halt_reg = 0x53018,
	.clkr = {
		.enable_reg = 0x53018,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_camss_mclk1_clk",
			.parent_names = (const char*[]) {
				"mclk1_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_camss_mclk2_clk = {
	.halt_reg = 0x5C018,
	.clkr = {
		.enable_reg = 0x5C018,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_camss_mclk2_clk",
			.parent_names = (const char*[]) {
				"mclk2_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_camss_micro_ahb_clk = {
	.halt_reg = 0x5600C,
	.clkr = {
		.enable_reg = 0x5600C,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_camss_micro_ahb_clk",
			.parent_names = (const char*[]) {
				"camss_top_ahb_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_camss_csi0phytimer_clk = {
	.halt_reg = 0x4E01C,
	.clkr = {
		.enable_reg = 0x4E01C,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_camss_csi0phytimer_clk",
			.parent_names = (const char*[]) {
				"csi0phytimer_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_camss_csi1phytimer_clk = {
	.halt_reg = 0x4F01C,
	.clkr = {
		.enable_reg = 0x4F01C,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_camss_csi1phytimer_clk",
			.parent_names = (const char*[]) {
				"csi1phytimer_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_camss_ahb_clk = {
	.halt_reg = 0x56004,
	.clkr = {
		.enable_reg = 0x56004,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_camss_ahb_clk",
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_camss_top_ahb_clk = {
	.halt_reg = 0x5A014,
	.clkr = {
		.enable_reg = 0x5A014,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_camss_top_ahb_clk",
			.parent_names = (const char*[]) {
				"camss_top_ahb_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_camss_vfe0_clk = {
	.halt_reg = 0x58038,
	.clkr = {
		.enable_reg = 0x58038,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_camss_vfe0_clk",
			.parent_names = (const char*[]) {
				"vfe0_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_camss_vfe_ahb_clk = {
	.halt_reg = 0x58044,
	//.halt_check = BRANCH_VOTED,
	.clkr = {
		.enable_reg = 0x58044,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_camss_vfe_ahb_clk",
			.parent_names = (const char*[]) {
				"camss_top_ahb_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_camss_vfe_axi_clk = {
	.halt_reg = 0x58048,
	//.halt_check = BRANCH_VOTED,
	.clkr = {
		.enable_reg = 0x58048,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_camss_vfe_axi_clk",
			.ops = &clk_branch2_ops,
		},
	},
};


static struct clk_branch gcc_camss_vfe1_ahb_clk = {
	.halt_reg = 0x58060,
	//.halt_check = BRANCH_VOTED,
	.clkr = {
		.enable_reg = 0x58060,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_camss_vfe1_ahb_clk",
			.parent_names = (const char*[]) {
				"camss_top_ahb_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_camss_vfe1_axi_clk = {
	.halt_reg = 0x58068,
	//.halt_check = BRANCH_VOTED,
	.clkr = {
		.enable_reg = 0x58068,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_camss_vfe1_axi_clk",
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_camss_vfe1_clk = {
	.halt_reg = 0x5805C,
	//.halt_check = BRANCH_VOTED,
	.clkr = {
		.enable_reg = 0x5805C,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_camss_vfe1_clk",
			.parent_names = (const char*[]) {
				"vfe1_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_dcc_clk = {
	.halt_reg = 0x77004,
	.clkr = {
		.enable_reg = 0x77004,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_dcc_clk",
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_gate2 gcc_oxili_gmem_clk = {
	.udelay = 50,
	.clkr = {
		.enable_reg = 0x59024,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_oxili_gmem_clk",
			.parent_names = (const char*[]) {
				"gfx3d_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_gate2_ops,
		},
	},
};

static struct clk_branch gcc_gp1_clk = {
	.halt_reg = 0x08000,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x08000,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_gp1_clk",
			.parent_names = (const char*[]) {
				"gp1_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_gp2_clk = {
	.halt_reg = 0x09000,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x09000,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_gp2_clk",
			.parent_names = (const char*[]) {
				"gp2_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_gp3_clk = {
	.halt_reg = 0x0A000,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x0A000,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_gp3_clk",
			.parent_names = (const char*[]) {
				"gp3_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_mdss_ahb_clk = {
	.halt_reg = 0x4D07C,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x4D07C,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_mdss_ahb_clk",
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_mdss_axi_clk = {
	.halt_reg = 0x4D080,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x4D080,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_mdss_axi_clk",
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_mdss_byte0_clk = {
	.halt_reg = 0x4D094,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x4D094,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_mdss_byte0_clk",
			.parent_names = (const char*[]) {
				"byte0_clk_src",
			},
			.num_parents = 1,
			.flags = (CLK_GET_RATE_NOCACHE |
				  CLK_SET_RATE_PARENT),
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_mdss_byte1_clk = {
	.halt_reg = 0x4D0A0,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x4D0A0,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_mdss_byte1_clk",
			.parent_names = (const char*[]) {
				"byte1_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_GET_RATE_NOCACHE | CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_mdss_esc0_clk = {
	.halt_reg = 0x4D098,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x4D098,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_mdss_esc0_clk",
			.parent_names = (const char*[]) {
				"esc0_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_mdss_esc1_clk = {
	.halt_reg = 0x4D09C,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x4D09C,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_mdss_esc1_clk",
			.parent_names = (const char*[]) {
				"esc1_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_mdss_mdp_clk = {
	.halt_reg = 0x4D088,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x4D088,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_mdss_mdp_clk",
			.parent_names = (const char*[]) {
				"mdp_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static DEFINE_CLK_VOTER(gcc_mdss_mdp_vote_clk, gcc_mdss_mdp_clk, 0);
static DEFINE_CLK_VOTER(gcc_mdss_mdp_rotator_vote_clk, gcc_mdss_mdp_clk, 0);

static struct clk_branch gcc_mdss_pclk0_clk = {
	.halt_reg = 0x4D084,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x4D084,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_mdss_pclk0_clk",
			.parent_names = (const char*[]) {
				"pclk0_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_GET_RATE_NOCACHE | CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_mdss_pclk1_clk = {
	.halt_reg = 0x4D0A4,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x4D0A4,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_mdss_pclk1_clk",
			.parent_names = (const char*[]) {
				"pclk1_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_GET_RATE_NOCACHE | CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_mdss_vsync_clk = {
	.halt_reg = 0x4D090,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x4D090,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_mdss_vsync_clk",
			.parent_names = (const char*[]) {
				"vsync_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_mss_cfg_ahb_clk = {
	.halt_reg = 0x49000,
	.clkr = {
		.enable_reg = 0x49000,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_mss_cfg_ahb_clk",
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_mss_q6_bimc_axi_clk = {
	.halt_reg = 0x49004,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x49004,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_mss_q6_bimc_axi_clk",
			.flags = CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_bimc_gfx_clk = {
	.halt_reg = 0x59048,
	//.halt_check = BRANCH_VOTED, //TODO: CHECKME
	.clkr = {
		.enable_reg = 0x59048,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_bimc_gfx_clk",
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_oxili_ahb_clk = {
	.halt_reg = 0x59028,
	.clkr = {
		.enable_reg = 0x59028,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_oxili_ahb_clk",
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_oxili_aon_clk = {
	.halt_reg = 0x59044,
	.clkr = {
		.enable_reg = 0x59044,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_oxili_aon_clk",
			.parent_names = (const char*[]) {
				"gfx3d_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_oxili_gfx3d_clk = {
	.halt_reg = 0x59020,
	.clkr = {
		.enable_reg = 0x59020,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_oxili_gfx3d_clk",
			.parent_names = (const char*[]) {
				"gfx3d_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			VDD_DIG_GFX_FMAX_MAP5(LOWER, 300000000, LOW, 360000000,
				NOMINAL, 432000000, NOM_PLUS, 480000000,
				HIGH, 600000000),
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_oxili_timer_clk = {
	.halt_reg = 0x59040,
	.clkr = {
		.enable_reg = 0x59040,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_oxili_timer_clk",
			.parent_names = (const char*[]) {
				"xo",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_pdm2_clk = {
	.halt_reg = 0x4400C,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x4400C,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_pdm2_clk",
			.parent_names = (const char*[]) {
				"pdm2_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_pdm_ahb_clk = {
	.halt_reg = 0x44004,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x44004,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_pdm_ahb_clk",
			.ops = &clk_branch2_ops,
		},
	},
};


static struct clk_branch gcc_rbcpr_gfx_ahb_clk = {
	.halt_reg = 0x3A008,
	.clkr = {
		.enable_reg = 0x3A008,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_rbcpr_gfx_ahb_clk",
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_rbcpr_gfx_clk = {
	.halt_reg = 0x3A004,
	.clkr = {
		.enable_reg = 0x3A004,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_rbcpr_gfx_clk",
			.parent_names = (const char*[]) {
				"rbcpr_gfx_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_sdcc1_ahb_clk = {
	.halt_reg = 0x4201C,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x4201C,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_sdcc1_ahb_clk",
			.flags = CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_sdcc1_apps_clk = {
	.halt_reg = 0x42018,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x42018,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_sdcc1_apps_clk",
			.parent_names = (const char*[]) {
				"sdcc1_apps_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_sdcc1_ice_core_clk = {
	.halt_reg = 0x5D014,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x5D014,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_sdcc1_ice_core_clk",
			.parent_names = (const char*[]) {
				"sdcc1_ice_core_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_sdcc2_ahb_clk = {
	.halt_reg = 0x4301C,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x4301C,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_sdcc2_ahb_clk",
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_sdcc2_apps_clk = {
	.halt_reg = 0x43018,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x43018,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_sdcc2_apps_clk",
			.parent_names = (const char*[]) {
				"sdcc2_apps_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_sdcc3_ahb_clk = {
	.halt_reg = 0x3901C,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x3901C,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_sdcc3_ahb_clk",
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_sdcc3_apps_clk = {
	.halt_reg = 0x39018,
	.halt_check = BRANCH_HALT,
	.clkr = {
		.enable_reg = 0x39018,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_sdcc3_apps_clk",
			.parent_names = (const char*[]) {
				"sdcc3_apps_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_usb2a_phy_sleep_clk = {
	.halt_reg = 0x4102C,
	.clkr = {
		.enable_reg = 0x4102C,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_usb2a_phy_sleep_clk",
			.flags = CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_usb_hs_phy_cfg_ahb_clk = {
	.halt_reg = 0x41030,
	.clkr = {
		.enable_reg = 0x41030,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_usb_hs_phy_cfg_ahb_clk",
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_usb_fs_ahb_clk = {
	.halt_reg = 0x3F008,
	.clkr = {
		.enable_reg = 0x3F008,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_usb_fs_ahb_clk",
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_usb_fs_ic_clk = {
	.halt_reg = 0x3F030,
	.clkr = {
		.enable_reg = 0x3F030,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_usb_fs_ic_clk",
			.parent_names = (const char*[]) {
				"usb_fs_ic_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_usb_fs_system_clk = {
	.halt_reg = 0x3F004,
	.clkr = {
		.enable_reg = 0x3F004,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_usb_fs_system_clk",
			.parent_names = (const char*[]) {
				"usb_fs_system_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_usb_hs_ahb_clk = {
	.halt_reg = 0x41008,
	.clkr = {
		.enable_reg = 0x41008,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_usb_hs_ahb_clk",
			.flags = CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_usb_hs_system_clk = {
	.halt_reg = 0x41004,
	.clkr = {
		.enable_reg = 0x41004,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_usb_hs_system_clk",
			.parent_names = (const char*[]) {
				"usb_hs_system_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_venus0_ahb_clk = {
	.halt_reg = 0x4C020,
	.clkr = {
		.enable_reg = 0x4C020,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_venus0_ahb_clk",
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_venus0_axi_clk = {
	.halt_reg = 0x4C024,
	.clkr = {
		.enable_reg = 0x4C024,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_venus0_axi_clk",
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_venus0_core0_vcodec0_clk = {
	.halt_reg = 0x4C02C,
	.clkr = {
		.enable_reg = 0x4C02C,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_venus0_core0_vcodec0_clk",
			.parent_names = (const char*[]) {
				"vcodec0_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_venus0_core1_vcodec0_clk = {
	.halt_reg = 0x4C034,
	.clkr = {
		.enable_reg = 0x4C034,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_venus0_core1_vcodec0_clk",
			.parent_names = (const char*[]) {
				"vcodec0_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};


static struct clk_branch gcc_venus0_vcodec0_clk = {
	.halt_reg = 0x4C01C,
	.clkr = {
		.enable_reg = 0x4C01C,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name ="gcc_venus0_vcodec0_clk",
			.parent_names = (const char*[]) {
				"vcodec0_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

/* Vote clocks */
static struct clk_branch gcc_apss_ahb_clk = {
	.halt_reg = 0x4601C,
	.halt_check = BRANCH_HALT_VOTED,
	.clkr = {
		.enable_reg = 0x45004,
		.enable_mask = BIT(14),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_apss_ahb_clk",
			.flags = CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_apss_axi_clk = {
	.halt_reg = 0x46020,
	.halt_check = BRANCH_HALT_VOTED,
	.clkr = {
		.enable_reg = 0x45004,
		.enable_mask = BIT(13),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_apss_axi_clk",
			.flags = CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_blsp1_ahb_clk = {
	.halt_reg = 0x01008,
	.halt_check = BRANCH_HALT_VOTED,
	.clkr = {
		.enable_reg = 0x45004,
		.enable_mask = BIT(10),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_blsp1_ahb_clk",
			.flags = CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};


static struct clk_branch gcc_blsp2_ahb_clk = {
	.halt_reg = 0x0B008,
	.halt_check = BRANCH_HALT_VOTED,
	.clkr = {
		.enable_reg = 0x45004,
		.enable_mask = BIT(20),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_blsp2_ahb_clk",
			.flags = CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_prng_ahb_clk = {
	.halt_reg = 0x13004,
	.halt_check = BRANCH_HALT_VOTED,
	.clkr = {
		.enable_reg = 0x45004,
		.enable_mask = BIT(8),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_prng_ahb_clk",
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_boot_rom_ahb_clk = {
	.halt_reg = 0x1300C,
	.halt_check = BRANCH_HALT_VOTED,
	.clkr = {
		.enable_reg = 0x45004,
		.enable_mask = BIT(7),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_boot_rom_ahb_clk",
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_crypto_ahb_clk = {
	.halt_reg = 0x16024,
	.halt_check = BRANCH_HALT_VOTED,
	.clkr = {
		.enable_reg = 0x45004,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_crypto_ahb_clk",
			.flags = CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_crypto_axi_clk = {
	.halt_reg = 0x16020,
	.halt_check = BRANCH_HALT_VOTED,
	.clkr = {
		.enable_reg = 0x45004,
		.enable_mask = BIT(1),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_crypto_axi_clk",
			.flags = CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_crypto_clk = {
	.halt_reg = 0x1601C,
	.halt_check = BRANCH_HALT_VOTED,
	.clkr = {
		.enable_reg = 0x45004,
		.enable_mask = BIT(2),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_crypto_clk",
			.parent_names = (const char *[]){
				"crypto_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_cpp_tbu_clk = {
	.halt_reg = 0x12040,
	.halt_check = BRANCH_HALT_VOTED,
	.clkr = {
		.enable_reg = 0x4500C,
		.enable_mask = BIT(14),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_cpp_tbu_clk",
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_gfx_1_tbu_clk = {
	.halt_reg = 0x12098,
	.halt_check = BRANCH_HALT_VOTED,
	.clkr = {
		.enable_reg = 0x4500C,
		.enable_mask = BIT(19),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_gfx_1_tbu_clk",
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_gfx_tbu_clk = {
	.halt_reg = 0x12010,
	.halt_check = BRANCH_HALT_VOTED,
	.clkr = {
		.enable_reg = 0x4500C,
		.enable_mask = BIT(3),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_gfx_tbu_clk",
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_gfx_tcu_clk = {
	.halt_reg = 0x12020,
	.halt_check = BRANCH_HALT_VOTED,
	.clkr = {
		.enable_reg = 0x4500C,
		.enable_mask = BIT(2),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_gfx_tcu_clk",
			.flags = CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_apss_tcu_clk = {
	.halt_reg = 0x12018,
	.halt_check = BRANCH_HALT_VOTED,
	.clkr = {
		.enable_reg = 0x4500C,
		.enable_mask = BIT(1),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_apss_tcu_clk",
			.flags = CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_gtcu_ahb_clk = {
	.halt_reg = 0x12044,
	.halt_check = BRANCH_HALT_VOTED,
	.clkr = {
		.enable_reg = 0x4500C,
		.enable_mask = BIT(13),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_gtcu_ahb_clk",
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_jpeg_tbu_clk = {
	.halt_reg = 0x12034,
	.halt_check = BRANCH_HALT_VOTED,
	.clkr = {
		.enable_reg = 0x4500C,
		.enable_mask = BIT(10),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_jpeg_tbu_clk",
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_mdp_rt_tbu_clk = {
	.halt_reg = 0x1204C,
	.halt_check = BRANCH_HALT_VOTED,
	.clkr = {
		.enable_reg = 0x4500C,
		.enable_mask = BIT(15),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_mdp_rt_tbu_clk",
			.flags = CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_mdp_tbu_clk = {
	.halt_reg = 0x1201C,
	.halt_check = BRANCH_HALT_VOTED,
	.clkr = {
		.enable_reg = 0x4500C,
		.enable_mask = BIT(4),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_mdp_tbu_clk",
			.flags = CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_smmu_cfg_clk = {
	.halt_reg = 0x12038,
	.halt_check = BRANCH_HALT_VOTED,
	.clkr = {
		.enable_reg = 0x4500C,
		.enable_mask = BIT(12),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_smmu_cfg_clk",
			.flags = CLK_ENABLE_HAND_OFF,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_venus_1_tbu_clk = {
	.halt_reg = 0x1209C,
	.halt_check = BRANCH_HALT_VOTED,
	.clkr = {
		.enable_reg = 0x4500C,
		.enable_mask = BIT(20),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_venus_1_tbu_clk",
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_venus_tbu_clk = {
	.halt_reg = 0x12014,
	.halt_check = BRANCH_HALT_VOTED,
	.clkr = {
		.enable_reg = 0x4500C,
		.enable_mask = BIT(5),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_venus_tbu_clk",
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_vfe1_tbu_clk = {
	.halt_reg = 0x12090,
	.halt_check = BRANCH_HALT_VOTED,
	.clkr = {
		.enable_reg = 0x4500C,
		.enable_mask = BIT(17),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_vfe1_tbu_clk",
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_vfe_tbu_clk = {
	.halt_reg = 0x1203C,
	.halt_check = BRANCH_HALT_VOTED,
	.clkr = {
		.enable_reg = 0x4500C,
		.enable_mask = BIT(9),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_vfe_tbu_clk",
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_regmap *gcc_msm8976_clocks[] = {
	[GPLL0] = &gpll0.clkr,
	[GPLL2] = &gpll2.clkr,
	[GPLL3] = &gpll3.clkr,
	[GPLL4] = &gpll4.clkr,
	[GPLL6] = &gpll6.clkr,
	[GPLL0_CLK_SRC] = &gpll0_out_main.clkr,
	[GPLL0_AO_CLK_SRC] = &gpll0_ao_out_main.clkr,
	[GPLL2_CLK_SRC] = &gpll2_out_main,
	[GPLL3_CLK_SRC] = &gpll3_out_main,
	[GPLL4_CLK_SRC] = &gpll4_out_main,
	[GPLL6_CLK_SRC] = &gpll6_out_main,

	[GCC_BLSP1_QUP1_SPI_APPS_CLK] = &gcc_blsp1_qup1_spi_apps_clk.clkr,
	[GCC_BLSP1_QUP1_I2C_APPS_CLK] = &gcc_blsp1_qup1_i2c_apps_clk.clkr,
	[GCC_BLSP1_QUP2_I2C_APPS_CLK] = &gcc_blsp1_qup2_i2c_apps_clk.clkr,
	[GCC_BLSP1_QUP2_SPI_APPS_CLK] = &gcc_blsp1_qup2_spi_apps_clk.clkr,
	[GCC_BLSP1_QUP3_I2C_APPS_CLK] = &gcc_blsp1_qup3_i2c_apps_clk.clkr,
	[GCC_BLSP1_QUP3_SPI_APPS_CLK] = &gcc_blsp1_qup3_spi_apps_clk.clkr,
	[GCC_BLSP1_QUP4_I2C_APPS_CLK] = &gcc_blsp1_qup4_i2c_apps_clk.clkr,
	[GCC_BLSP1_QUP4_SPI_APPS_CLK] = &gcc_blsp1_qup4_spi_apps_clk.clkr,
	[GCC_BLSP1_UART1_APPS_CLK] = &gcc_blsp1_uart1_apps_clk.clkr,
	[GCC_BLSP1_UART2_APPS_CLK] = &gcc_blsp1_uart2_apps_clk.clkr,
	[GCC_BLSP2_QUP1_I2C_APPS_CLK] = &gcc_blsp2_qup1_i2c_apps_clk.clkr,
	[GCC_BLSP2_QUP1_SPI_APPS_CLK] = &gcc_blsp2_qup1_spi_apps_clk.clkr,
	[GCC_BLSP2_QUP2_I2C_APPS_CLK] = &gcc_blsp2_qup2_i2c_apps_clk.clkr,
	[GCC_BLSP2_QUP2_SPI_APPS_CLK] = &gcc_blsp2_qup2_spi_apps_clk.clkr,
	[GCC_BLSP2_QUP3_I2C_APPS_CLK] = &gcc_blsp2_qup3_i2c_apps_clk.clkr,
	[GCC_BLSP2_QUP3_SPI_APPS_CLK] = &gcc_blsp2_qup3_spi_apps_clk.clkr,
	[GCC_BLSP2_QUP4_I2C_APPS_CLK] = &gcc_blsp2_qup4_i2c_apps_clk.clkr,
	[GCC_BLSP2_QUP4_SPI_APPS_CLK] = &gcc_blsp2_qup4_spi_apps_clk.clkr,
	[GCC_BLSP2_UART1_APPS_CLK] = &gcc_blsp2_uart1_apps_clk.clkr,
	[GCC_BLSP2_UART2_APPS_CLK] = &gcc_blsp2_uart2_apps_clk.clkr,
	[GCC_CAMSS_CCI_AHB_CLK] = &gcc_camss_cci_ahb_clk.clkr,
	[GCC_CAMSS_CCI_CLK] = &gcc_camss_cci_clk.clkr,
	[GCC_CAMSS_CPP_AHB_CLK] = &gcc_camss_cpp_ahb_clk.clkr,
	[GCC_CAMSS_CPP_AXI_CLK] = &gcc_camss_cpp_axi_clk.clkr,
	[GCC_CAMSS_CPP_CLK] = &gcc_camss_cpp_clk.clkr,
	[GCC_CAMSS_CSI0_AHB_CLK] = &gcc_camss_csi0_ahb_clk.clkr,
	[GCC_CAMSS_CSI0_CLK] = &gcc_camss_csi0_clk.clkr,
	[GCC_CAMSS_CSI0PHY_CLK] = &gcc_camss_csi0phy_clk.clkr,
	[GCC_CAMSS_CSI0PIX_CLK] = &gcc_camss_csi0pix_clk.clkr,
	[GCC_CAMSS_CSI0RDI_CLK] = &gcc_camss_csi0rdi_clk.clkr,
	[GCC_CAMSS_CSI1_AHB_CLK] = &gcc_camss_csi1_ahb_clk.clkr,
	[GCC_CAMSS_CSI1_CLK] = &gcc_camss_csi1_clk.clkr,
	[GCC_CAMSS_CSI1PHY_CLK] = &gcc_camss_csi1phy_clk.clkr,
	[GCC_CAMSS_CSI1PIX_CLK] = &gcc_camss_csi1pix_clk.clkr,
	[GCC_CAMSS_CSI1RDI_CLK] = &gcc_camss_csi1rdi_clk.clkr,
	[GCC_CAMSS_CSI2_AHB_CLK] = &gcc_camss_csi2_ahb_clk.clkr,
	[GCC_CAMSS_CSI2_CLK] = &gcc_camss_csi2_clk.clkr,
	[GCC_CAMSS_CSI2PHY_CLK] = &gcc_camss_csi2phy_clk.clkr,
	[GCC_CAMSS_CSI2PIX_CLK] = &gcc_camss_csi2pix_clk.clkr,
	[GCC_CAMSS_CSI2RDI_CLK] = &gcc_camss_csi2rdi_clk.clkr,
	[GCC_CAMSS_CSI_VFE0_CLK] = &gcc_camss_csi_vfe0_clk.clkr,
	[GCC_CAMSS_CSI_VFE1_CLK] = &gcc_camss_csi_vfe1_clk.clkr,
	[GCC_CAMSS_GP0_CLK] = &gcc_camss_gp0_clk.clkr,
	[GCC_CAMSS_GP1_CLK] = &gcc_camss_gp1_clk.clkr,
	[GCC_CAMSS_ISPIF_AHB_CLK] = &gcc_camss_ispif_ahb_clk.clkr,
	[GCC_CAMSS_JPEG0_CLK] = &gcc_camss_jpeg0_clk.clkr,
	[GCC_CAMSS_JPEG_AHB_CLK] = &gcc_camss_jpeg_ahb_clk.clkr,
	[GCC_CAMSS_JPEG_AXI_CLK] = &gcc_camss_jpeg_axi_clk.clkr,
	[GCC_CAMSS_MCLK0_CLK] = &gcc_camss_mclk0_clk.clkr,
	[GCC_CAMSS_MCLK1_CLK] = &gcc_camss_mclk1_clk.clkr,
	[GCC_CAMSS_MCLK2_CLK] = &gcc_camss_mclk2_clk.clkr,
	[GCC_CAMSS_MICRO_AHB_CLK] = &gcc_camss_micro_ahb_clk.clkr,
	[GCC_CAMSS_CSI0PHYTIMER_CLK] = &gcc_camss_csi0phytimer_clk.clkr,
	[GCC_CAMSS_CSI1PHYTIMER_CLK] = &gcc_camss_csi1phytimer_clk.clkr,
	[GCC_CAMSS_AHB_CLK] = &gcc_camss_ahb_clk.clkr,
	[GCC_CAMSS_TOP_AHB_CLK] = &gcc_camss_top_ahb_clk.clkr,
	[GCC_CAMSS_VFE0_CLK] = &gcc_camss_vfe0_clk.clkr,
	[GCC_CAMSS_VFE_AHB_CLK] = &gcc_camss_vfe_ahb_clk.clkr,
	[GCC_CAMSS_VFE_AXI_CLK] = &gcc_camss_vfe_axi_clk.clkr,
	[GCC_CAMSS_VFE1_AHB_CLK] = &gcc_camss_vfe1_ahb_clk.clkr,
	[GCC_CAMSS_VFE1_AXI_CLK] = &gcc_camss_vfe1_axi_clk.clkr,
	[GCC_CAMSS_VFE1_CLK] = &gcc_camss_vfe1_clk.clkr,
	[GCC_DCC_CLK] = &gcc_dcc_clk.clkr,
	[GCC_GP1_CLK] = &gcc_gp1_clk.clkr,
	[GCC_GP2_CLK] = &gcc_gp2_clk.clkr,
	[GCC_GP3_CLK] = &gcc_gp3_clk.clkr,
	[GCC_MDSS_AHB_CLK] = &gcc_mdss_ahb_clk.clkr,
	[GCC_MDSS_AXI_CLK] = &gcc_mdss_axi_clk.clkr,
	[GCC_MDSS_ESC0_CLK] = &gcc_mdss_esc0_clk.clkr,
	[GCC_MDSS_ESC1_CLK] = &gcc_mdss_esc1_clk.clkr,
	[GCC_MDSS_MDP_CLK] = &gcc_mdss_mdp_clk.clkr,
	[GCC_MDSS_VSYNC_CLK] = &gcc_mdss_vsync_clk.clkr,
	[GCC_MSS_CFG_AHB_CLK] = &gcc_mss_cfg_ahb_clk.clkr,
	[GCC_MSS_Q6_BIMC_AXI_CLK] = &gcc_mss_q6_bimc_axi_clk.clkr,
	[GCC_PDM2_CLK] = &gcc_pdm2_clk.clkr,
	[GCC_PRNG_AHB_CLK] = &gcc_prng_ahb_clk.clkr,
	[GCC_PDM_AHB_CLK] = &gcc_pdm_ahb_clk.clkr,
	[GCC_RBCPR_GFX_AHB_CLK] = &gcc_rbcpr_gfx_ahb_clk.clkr,
	[GCC_RBCPR_GFX_CLK] = &gcc_rbcpr_gfx_clk.clkr,
	[GCC_SDCC1_AHB_CLK] = &gcc_sdcc1_ahb_clk.clkr,
	[GCC_SDCC1_APPS_CLK] = &gcc_sdcc1_apps_clk.clkr,
	[GCC_SDCC1_ICE_CORE_CLK] = &gcc_sdcc1_ice_core_clk.clkr,
	[GCC_SDCC2_AHB_CLK] = &gcc_sdcc2_ahb_clk.clkr,
	[GCC_SDCC2_APPS_CLK] = &gcc_sdcc2_apps_clk.clkr,
	[GCC_SDCC3_AHB_CLK] = &gcc_sdcc3_ahb_clk.clkr,
	[GCC_SDCC3_APPS_CLK] = &gcc_sdcc3_apps_clk.clkr,
	[GCC_USB2A_PHY_SLEEP_CLK] = &gcc_usb2a_phy_sleep_clk.clkr,
	[GCC_USB_HS_PHY_CFG_AHB_CLK] = &gcc_usb_hs_phy_cfg_ahb_clk.clkr,
	[GCC_USB_FS_AHB_CLK] = &gcc_usb_fs_ahb_clk.clkr,
	[GCC_USB_FS_IC_CLK] = &gcc_usb_fs_ic_clk.clkr,
	[GCC_USB_FS_SYSTEM_CLK] = &gcc_usb_fs_system_clk.clkr,
	[GCC_USB_HS_AHB_CLK] = &gcc_usb_hs_ahb_clk.clkr,
	[GCC_USB_HS_SYSTEM_CLK] = &gcc_usb_hs_system_clk.clkr,
	[GCC_VENUS0_AHB_CLK] = &gcc_venus0_ahb_clk.clkr,
	[GCC_VENUS0_AXI_CLK] = &gcc_venus0_axi_clk.clkr,
	[GCC_VENUS0_CORE0_VCODEC0_CLK] = &gcc_venus0_core0_vcodec0_clk.clkr,
	[GCC_VENUS0_CORE1_VCODEC0_CLK] = &gcc_venus0_core1_vcodec0_clk.clkr,
	[GCC_VENUS0_VCODEC0_CLK] = &gcc_venus0_vcodec0_clk.clkr,
	[GCC_APSS_AHB_CLK] = &gcc_apss_ahb_clk.clkr,
	[GCC_APSS_AXI_CLK] = &gcc_apss_axi_clk.clkr,
	[GCC_BLSP1_AHB_CLK] = &gcc_blsp1_ahb_clk.clkr,
	[GCC_BLSP2_AHB_CLK] = &gcc_blsp2_ahb_clk.clkr,
	[GCC_BOOT_ROM_AHB_CLK] = &gcc_boot_rom_ahb_clk.clkr,
	[GCC_CRYPTO_AHB_CLK] = &gcc_crypto_ahb_clk.clkr,
	[GCC_CRYPTO_AXI_CLK] = &gcc_crypto_axi_clk.clkr,
	[GCC_CRYPTO_CLK] = &gcc_crypto_clk.clkr,
	[GCC_CPP_TBU_CLK] = &gcc_cpp_tbu_clk.clkr,
	[GCC_APSS_TCU_CLK] = &gcc_apss_tcu_clk.clkr,
	[GCC_JPEG_TBU_CLK] = &gcc_jpeg_tbu_clk.clkr,
	[GCC_MDP_RT_TBU_CLK] = &gcc_mdp_rt_tbu_clk.clkr,
	[GCC_MDP_TBU_CLK] = &gcc_mdp_tbu_clk.clkr,
	[GCC_SMMU_CFG_CLK] = &gcc_smmu_cfg_clk.clkr,
	[GCC_VENUS_1_TBU_CLK] = &gcc_venus_1_tbu_clk.clkr,
	[GCC_VENUS_TBU_CLK] = &gcc_venus_tbu_clk.clkr,
	[GCC_VFE1_TBU_CLK] = &gcc_vfe1_tbu_clk.clkr,
	[GCC_VFE_TBU_CLK] = &gcc_vfe_tbu_clk.clkr,
	[GCC_APS_0_CLK] = &gcc_aps_0_clk.clkr,
	[GCC_APS_1_CLK] = &gcc_aps_1_clk.clkr,
	[APS_0_CLK_SRC] = &aps_0_clk_src.clkr,
	[APS_1_CLK_SRC] = &aps_1_clk_src.clkr,
	[APSS_AHB_CLK_SRC] = &apss_ahb_clk_src.clkr,
	[BLSP1_QUP1_I2C_APPS_CLK_SRC] = &blsp1_qup1_i2c_apps_clk_src.clkr,
	[BLSP1_QUP1_SPI_APPS_CLK_SRC] = &blsp1_qup1_spi_apps_clk_src.clkr,
	[BLSP1_QUP2_I2C_APPS_CLK_SRC] = &blsp1_qup2_i2c_apps_clk_src.clkr,
	[BLSP1_QUP2_SPI_APPS_CLK_SRC] = &blsp1_qup2_spi_apps_clk_src.clkr,
	[BLSP1_QUP3_I2C_APPS_CLK_SRC] = &blsp1_qup3_i2c_apps_clk_src.clkr,
	[BLSP1_QUP3_SPI_APPS_CLK_SRC] = &blsp1_qup3_spi_apps_clk_src.clkr,
	[BLSP1_QUP4_I2C_APPS_CLK_SRC] = &blsp1_qup4_i2c_apps_clk_src.clkr,
	[BLSP1_QUP4_SPI_APPS_CLK_SRC] = &blsp1_qup4_spi_apps_clk_src.clkr,
	[BLSP1_UART1_APPS_CLK_SRC] = &blsp1_uart1_apps_clk_src.clkr,
	[BLSP1_UART2_APPS_CLK_SRC] = &blsp1_uart2_apps_clk_src.clkr,
	[BLSP2_QUP1_I2C_APPS_CLK_SRC] = &blsp2_qup1_i2c_apps_clk_src.clkr,
	[BLSP2_QUP1_SPI_APPS_CLK_SRC] = &blsp2_qup1_spi_apps_clk_src.clkr,
	[BLSP2_QUP2_I2C_APPS_CLK_SRC] = &blsp2_qup2_i2c_apps_clk_src.clkr,
	[BLSP2_QUP2_SPI_APPS_CLK_SRC] = &blsp2_qup2_spi_apps_clk_src.clkr,
	[BLSP2_QUP3_I2C_APPS_CLK_SRC] = &blsp2_qup3_i2c_apps_clk_src.clkr,
	[BLSP2_QUP3_SPI_APPS_CLK_SRC] = &blsp2_qup3_spi_apps_clk_src.clkr,
	[BLSP2_QUP4_I2C_APPS_CLK_SRC] = &blsp2_qup4_i2c_apps_clk_src.clkr,
	[BLSP2_QUP4_SPI_APPS_CLK_SRC] = &blsp2_qup4_spi_apps_clk_src.clkr,
	[BLSP2_UART1_APPS_CLK_SRC] = &blsp2_uart1_apps_clk_src.clkr,
	[BLSP2_UART2_APPS_CLK_SRC] = &blsp2_uart2_apps_clk_src.clkr,
	[CCI_CLK_SRC] = &cci_clk_src.clkr,
	[CPP_CLK_SRC] = &cpp_clk_src.clkr,
	[CSI0_CLK_SRC] = &csi0_clk_src.clkr,
	[CSI1_CLK_SRC] = &csi1_clk_src.clkr,
	[CSI2_CLK_SRC] = &csi2_clk_src.clkr,
	[CAMSS_GP0_CLK_SRC] = &camss_gp0_clk_src.clkr,
	[CAMSS_GP1_CLK_SRC] = &camss_gp1_clk_src.clkr,
	[JPEG0_CLK_SRC] = &jpeg0_clk_src.clkr,
	[MCLK0_CLK_SRC] = &mclk0_clk_src.clkr,
	[MCLK1_CLK_SRC] = &mclk1_clk_src.clkr,
	[MCLK2_CLK_SRC] = &mclk2_clk_src.clkr,
	[CSI0PHYTIMER_CLK_SRC] = &csi0phytimer_clk_src.clkr,
	[CSI1PHYTIMER_CLK_SRC] = &csi1phytimer_clk_src.clkr,
	[CAMSS_TOP_AHB_CLK_SRC] = &camss_top_ahb_clk_src.clkr,
	[VFE0_CLK_SRC] = &vfe0_clk_src.clkr,
	[VFE1_CLK_SRC] = &vfe1_clk_src.clkr,
	[CRYPTO_CLK_SRC] = &crypto_clk_src.clkr,
	[GP1_CLK_SRC] = &gp1_clk_src.clkr,
	[GP2_CLK_SRC] = &gp2_clk_src.clkr,
	[GP3_CLK_SRC] = &gp3_clk_src.clkr,
	[ESC0_CLK_SRC] = &esc0_clk_src.clkr,
	[ESC1_CLK_SRC] = &esc1_clk_src.clkr,
	[MDP_CLK_SRC] = &mdp_clk_src.clkr,
	[VSYNC_CLK_SRC] = &vsync_clk_src.clkr,
	[PDM2_CLK_SRC] = &pdm2_clk_src.clkr,
	[RBCPR_GFX_CLK_SRC] = &rbcpr_gfx_clk_src.clkr,
	[SDCC1_APPS_CLK_SRC] = &sdcc1_apps_clk_src.clkr,
	[SDCC1_ICE_CORE_CLK_SRC] = &sdcc1_ice_core_clk_src.clkr,
	[SDCC2_APPS_CLK_SRC] = &sdcc2_apps_clk_src.clkr,
	[SDCC3_APPS_CLK_SRC] = &sdcc3_apps_clk_src.clkr,
	[USB_FS_IC_CLK_SRC] = &usb_fs_ic_clk_src.clkr,
	[USB_FS_SYSTEM_CLK_SRC] = &usb_fs_system_clk_src.clkr,
	[USB_HS_SYSTEM_CLK_SRC] = &usb_hs_system_clk_src.clkr,
	[VCODEC0_CLK_SRC] = &vcodec0_clk_src.clkr,
	//[GCC_QUSB2_PHY_CLK] = &gcc_qusb2_phy_clk.clkr,
	//[GCC_USB2_HS_PHY_ONLY_CLK] = &gcc_usb2_hs_phy_only_clk.clkr,
	//[WCNSS_M_CLK] = &wcnss_m_clk.clkr,
};

static struct clk_fixed_factor gcc_ce1_ahb_m_clk = {
	.hw.init = &(struct clk_init_data){
		.name = "gcc_ce1_ahb_m_clk",
		.ops = &clk_dummy_ops,
		.flags = CLK_IGNORE_UNUSED,
	},
};

static struct clk_fixed_factor gcc_ce1_axi_m_clk = {
	.hw.init = &(struct clk_init_data){
		.name = "gcc_ce1_axi_m_clk",
		.ops = &clk_dummy_ops,
		.flags = CLK_IGNORE_UNUSED,
	},
};

static struct clk_hw *gcc_msm8976_hws[] = {
	[GCC_XO]		= &xo.hw,
	[GCC_XO_AO]		= &xo_a.hw,
	[GCC_CE1_AHB_M_CLK]	= &gcc_ce1_ahb_m_clk.hw,
	[GCC_CE1_AXI_M_CLK]	= &gcc_ce1_axi_m_clk.hw,
	[GCC_WCNSS_M_CLK]	= &wcnss_m_clk.hw,
};

static const struct qcom_reset_map gcc_msm8976_resets[] = {
	[RST_CAMSS_MICRO_BCR]		= { 0x56008 },
	[RST_USB_HS_BCR]		= { 0x41000 },
	[RST_QUSB2_PHY_BCR]		= { 0x4103C }, /* gcc_qusb2_phy_clk */
	[RST_USB2_HS_PHY_ONLY_BCR]	= { 0x41034 }, /*gcc_usb2_hs_phy_only_clk*/
	[RST_USB_HS_PHY_CFG_AHB_BCR]	= { 0x41038 },
	[RST_USB_FS_BCR]		= { 0x3F000 },
	[RST_CAMSS_CSI1PIX_BCR]		= { 0x4F054 },
	[RST_CAMSS_CSI_VFE1_BCR]	= { 0x58070 },
	[RST_CAMSS_VFE1_BCR]		= { 0x5807C },
	[RST_CAMSS_CPP_BCR]		= { 0x58080 },
};

static const struct regmap_config gcc_msm8976_regmap_config = {
	.reg_bits	= 32,
	.reg_stride	= 4,
	.val_bits	= 32,
	.max_register	= 0x7fffc,
	.fast_io	= true,
};

static const struct qcom_cc_desc gcc_msm8976_desc = {
	.config		= &gcc_msm8976_regmap_config,
	.clks		= gcc_msm8976_clocks,
	.num_clks	= ARRAY_SIZE(gcc_msm8976_clocks),
//	.hwclks		= gcc_msm8976_hws,
//	.num_hwclks	= ARRAY_SIZE(gcc_msm8976_hws),
	.resets		= gcc_msm8976_resets,
	.num_resets	= ARRAY_SIZE(gcc_msm8976_resets),
};

static struct of_device_id msm_clock_gcc_match_table[] = {
	{ .compatible = "qcom,gcc-8976" },
	{ .compatible = "qcom,gcc-8976-v1" },
	{},
};

static struct platform_driver gcc_voters_8976_driver;
static struct platform_driver gcc_mdss_8976_driver;
static struct platform_driver gcc_gfx_8976_driver;

#define GCC_REG_BASE 0x1800000
static int gcc_8976_probe(struct platform_device *pdev)
{
	struct regmap *regmap;
	void __iomem *base;
	int i, ret;
	u32 val;
	bool compat_bin = false;

	regmap = qcom_cc_map(pdev, &gcc_msm8976_desc);
	if (IS_ERR(regmap))
		return -EPROBE_DEFER; //PTR_ERR(regmap);

	compat_bin = of_device_is_compatible(pdev->dev.of_node,
						"qcom,gcc-8976-v1");

	vdd_dig.regulator[0] = devm_regulator_get(&pdev->dev, "vdd_dig");
	if (IS_ERR(vdd_dig.regulator[0])) {
		if (!(PTR_ERR(vdd_dig.regulator[0]) == -EPROBE_DEFER))
			dev_err(&pdev->dev,
					"Unable to get vdd_dig regulator!!!\n");
		return -EPROBE_DEFER; //PTR_ERR(vdd_dig.regulator[0]);
	}

	/* Switch SDCC1 clocks if 8976v1.0 */
	if (compat_bin) {
		gcc_msm8976_clocks[SDCC1_APPS_CLK_SRC] = &sdcc1_apps_clk_src_v1.clkr;
	}

	/* Vote for GPLL0 to turn on. Needed by acpuclock. */
	regmap_update_bits(regmap, 0x45000, BIT(0), BIT(0));

	/* Register the hws */
	for (i = 0; i < ARRAY_SIZE(gcc_msm8976_hws); i++) {
		ret = devm_clk_hw_register(&pdev->dev, gcc_msm8976_hws[i]);
		if (ret)
			return ret;
	}

	ret = qcom_cc_really_probe(pdev, &gcc_msm8976_desc, regmap);
	if (ret) {
		dev_err(&pdev->dev, "Failed to register GCC clocks\n");
		return ret;
	}

	clk_set_rate(apss_ahb_clk_src.clkr.hw.clk, 19200000);
	clk_prepare_enable(apss_ahb_clk_src.clkr.hw.clk);

	//clk_prepare_enable(gpll0_ao_out_main.hw.clk);
	//clk_prepare_enable(gpll6_out_main.hw.clk);

	/* Configure Sleep and Wakeup cycles for GMEM clock */
	//regmap_update_bits(regmap, 0x59024, mask, WTF?);
/*
	regmap_read(regmap, 0x59024, &val);
	val ^= 0xFF0;
	val |= (0 << 8);
	val |= (0 << 4);
	regmap_write(regmap, 0x59024, val);
*/

	base = ioremap_nocache(GCC_REG_BASE, 0x80000);
	val = readl_relaxed((void __iomem*)(base + 0x59024));
	val ^= 0xFF0;
	val |= (0 << 8);
	val |= (0 << 4);
	writel_relaxed(val, (void __iomem*)(base + 0x59024));

	clk_pll_configure_sr_hpm_lp(&gpll3, regmap,
					&gpll3_config, true);

	clk_set_rate(gpll3.clkr.hw.clk, 1100000000);

	/* Enable AUX2 clock for APSS */
	regmap_update_bits(regmap, 0x60000, BIT(2), BIT(2));

	dev_info(&pdev->dev, "Registered GCC clocks\n");

	ret = platform_driver_register(&gcc_mdss_8976_driver);
	ret = platform_driver_register(&gcc_voters_8976_driver);
	ret = platform_driver_register(&gcc_gfx_8976_driver);

	return 0;
}

static struct platform_driver gcc_8976_driver = {
	.probe = gcc_8976_probe,
	.driver = {
		.name = "qcom,gcc-8976",
		.of_match_table = msm_clock_gcc_match_table,
	},
};

/* Voters */
static struct clk_hw *gcc_voters_8976_hws[] = {
	[GCC_MDSS_MDP_VOTE_CLK]	= &gcc_mdss_mdp_vote_clk.hw,
	[GCC_MDSS_MDP_ROTATOR_VOTE_CLK] = &gcc_mdss_mdp_rotator_vote_clk.hw,
};

static struct of_device_id gcc_voters_8976_match_table[] = {
	{ .compatible = "qcom,gcc-voters-8976" },
	{ }
};

static int msm_gcc_8976_voters_probe(struct platform_device *pdev)
{
	int rc, i, num_clks;
	struct clk *clk;
	struct clk_onecell_data *onecell;

	num_clks = ARRAY_SIZE(gcc_voters_8976_hws);

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
		if (!gcc_voters_8976_hws[i])
			continue;

		clk = devm_clk_register(&pdev->dev, gcc_voters_8976_hws[i]);
		if (IS_ERR(clk)) {
			dev_err(&pdev->dev, "Cannot register clock no %d\n",i);
			return PTR_ERR(clk);
		}
		onecell->clks[i] = clk;
	}

	rc = of_clk_add_provider(pdev->dev.of_node,
			of_clk_src_onecell_get, onecell);
	if (rc == 0)
		dev_info(&pdev->dev, "Registered GCC Software Voters\n");

	return rc;
}

static struct platform_driver gcc_voters_8976_driver = {
	.probe = msm_gcc_8976_voters_probe,
	.driver = {
		.name = "gcc-voters-8976",
		.of_match_table = gcc_voters_8976_match_table,
	},
};

static int __init gcc_8976_init(void)
{
	return platform_driver_register(&gcc_8976_driver);
}
core_initcall_sync(gcc_8976_init);

static void __exit gcc_8976_exit(void)
{
	platform_driver_unregister(&gcc_8976_driver);
}
module_exit(gcc_8976_exit);




static struct measure_clk_data debug_mux_priv = {
	.xo_div4_cbcr = 0x30034,
	.ctl_reg = 0x74004,
	.status_reg = 0x74008,
/* Kept for reference:
	.plltest_reg = PLLTEST_PAD_CFG,
	.plltest_val = 0x51A00,
*/
};

static const char *const debug_mux_parent_names[] = {
	"debug_cpu_clk",
	"snoc_clk",
	"pcnoc_clk",
	"bimc_clk",
	"sysmmnoc_clk",
	"ipa_clk",
	"gcc_gp1_clk",
	"gcc_gp2_clk",
	"gcc_gp3_clk",
	"gcc_bimc_gfx_clk",
	"gcc_mss_cfg_ahb_clk",
	"gcc_mss_q6_bimc_axi_clk",
	"gcc_apss_tcu_clk",
	"gcc_mdp_tbu_clk",
	"gcc_gfx_tbu_clk",
	"gcc_gfx_tcu_clk",
	"gcc_venus_tbu_clk",
	"gcc_gtcu_ahb_clk",
	"gcc_vfe_tbu_clk",
	"gcc_smmu_cfg_clk",
	"gcc_jpeg_tbu_clk",
	"gcc_usb_hs_system_clk",
	"gcc_usb_hs_ahb_clk",
	"gcc_usb2a_phy_sleep_clk",
	"gcc_usb_hs_phy_cfg_ahb_clk",
	"gcc_sdcc1_apps_clk",
	"gcc_sdcc1_ahb_clk",
	"gcc_sdcc1_ice_core_clk",
	"gcc_sdcc2_apps_clk",
	"gcc_sdcc2_ahb_clk",
	"gcc_blsp1_ahb_clk",
	"gcc_blsp1_qup1_spi_apps_clk",
	"gcc_blsp1_qup1_i2c_apps_clk",
	"gcc_blsp1_uart1_apps_clk",
	"gcc_blsp1_qup2_spi_apps_clk",
	"gcc_blsp1_qup2_i2c_apps_clk",
	"gcc_blsp1_uart2_apps_clk",
	"gcc_blsp1_qup3_spi_apps_clk",
	"gcc_blsp1_qup3_i2c_apps_clk",
	"gcc_blsp1_qup4_spi_apps_clk",
	"gcc_blsp1_qup4_i2c_apps_clk",
	"gcc_camss_ahb_clk",
	"gcc_camss_top_ahb_clk",
	"gcc_camss_micro_ahb_clk",
	"gcc_camss_gp0_clk",
	"gcc_camss_gp1_clk",
	"gcc_camss_mclk0_clk",
	"gcc_camss_mclk1_clk",
	"gcc_camss_cci_clk",
	"gcc_camss_cci_ahb_clk",
	"gcc_camss_csi0phytimer_clk",
	"gcc_camss_csi1phytimer_clk",
	"gcc_camss_jpeg0_clk",
	"gcc_camss_jpeg_ahb_clk",
	"gcc_camss_jpeg_axi_clk",
	"gcc_camss_vfe0_clk",
	"gcc_camss_cpp_clk",
	"gcc_camss_cpp_ahb_clk",
	"gcc_camss_vfe_ahb_clk",
	"gcc_camss_vfe_axi_clk",
	"gcc_camss_csi_vfe0_clk",
	"gcc_camss_csi0_clk",
	"gcc_camss_csi0_ahb_clk",
	"gcc_camss_csi0phy_clk",
	"gcc_camss_csi0rdi_clk",
	"gcc_camss_csi0pix_clk",
	"gcc_camss_csi1_clk",
	"gcc_camss_csi1_ahb_clk",
	"gcc_camss_csi1phy_clk",
	"gcc_pdm_ahb_clk",
	"gcc_pdm2_clk",
	"gcc_prng_ahb_clk",
	"gcc_camss_csi1rdi_clk",
	"gcc_camss_csi1pix_clk",
	"gcc_camss_ispif_ahb_clk",
	"gcc_camss_csi2_clk",
	"gcc_camss_csi2_ahb_clk",
	"gcc_camss_csi2phy_clk",
	"gcc_camss_csi2rdi_clk",
	"gcc_camss_csi2pix_clk",
	"gcc_cpp_tbu_clk",
	"gcc_camss_vfe1_axi_clk",
	"gcc_camss_vfe1_ahb_clk",
	"gcc_camss_vfe1_clk",
	"gcc_mdp_rt_tbu_clk",
	"gcc_usb_fs_system_clk",
	"gcc_usb_fs_ahb_clk",
	"gcc_usb_fs_ic_clk",
	"gcc_boot_rom_ahb_clk",
	"gcc_crypto_clk",
	"gcc_crypto_axi_clk",
	"gcc_crypto_ahb_clk",
	"gcc_camss_csi_vfe1_clk",
	"gcc_camss_cpp_axi_clk",
	"gcc_venus0_core0_vcodec0_clk",
	"gcc_mdss_pclk1_clk",
	"gcc_mdss_byte1_clk",
	"gcc_mdss_esc1_clk",
	"gcc_camss_mclk2_clk",
	"gcc_oxili_timer_clk",
	"gcc_oxili_gfx3d_clk",
	"gcc_oxili_ahb_clk",
	"gcc_oxili_aon_clk",
	"gcc_oxili_gmem_clk",
	"gcc_venus0_vcodec0_clk",
	"gcc_venus0_axi_clk",
	"gcc_venus0_ahb_clk",
	"gcc_mdss_ahb_clk",
	"gcc_mdss_axi_clk",
	"gcc_mdss_pclk0_clk",
	"gcc_mdss_mdp_clk",
	"gcc_mdss_vsync_clk",
	"gcc_mdss_byte0_clk",
	"gcc_mdss_esc0_clk",
	"gcc_vfe1_tbu_clk",
	"gcc_gfx_1_tbu_clk",
	"gcc_venus_1_tbu_clk",
	"gcc_rbcpr_gfx_clk",
	"gcc_rbcpr_gfx_ahb_clk",
	"gcc_sdcc3_apps_clk",
	"gcc_sdcc3_ahb_clk",
	"gcc_blsp2_ahb_clk",
	"gcc_blsp2_qup1_spi_apps_clk",
	"gcc_blsp2_qup1_i2c_apps_clk",
	"gcc_blsp2_uart1_apps_clk",
	"gcc_blsp2_qup2_spi_apps_clk",
	"gcc_blsp2_qup2_i2c_apps_clk",
	"gcc_blsp2_uart2_apps_clk",
	"gcc_blsp2_qup3_spi_apps_clk",
	"gcc_blsp2_qup3_i2c_apps_clk",
	"gcc_blsp2_qup4_spi_apps_clk",
	"gcc_blsp2_qup4_i2c_apps_clk",
	"gcc_dcc_clk",
	"gcc_aps_0_clk",
	"gcc_aps_1_clk",
	"wcnss_m_clk",
};

static struct clk_debug_mux gcc_debug_mux = {
	.priv = &debug_mux_priv,
	//.en_mask = BIT(16), // Disappeared!
	.src_sel_mask = 0x3FF,
	/* TODO: CHECKME */
	.src_sel_shift = 0,
	.post_div_mask = 0xF,
	.post_div_shift = 0,
	/*               */
	MUX_SRC_LIST(
		{ "debug_cpu_clk",			0x016A },
		{ "snoc_clk",				0x0000 },
		{ "pcnoc_clk",				0x0008 },
		{ "bimc_clk",				0x0154 },
		{ "sysmmnoc_clk",			0x0001 },
		{ "ipa_clk",				0x0200 },
		{ "gcc_gp1_clk",			0x0010 },
		{ "gcc_gp2_clk",			0x0011 },
		{ "gcc_gp3_clk",			0x0012 },
		{ "gcc_bimc_gfx_clk",			0x002d },
		{ "gcc_mss_cfg_ahb_clk",		0x0030 },
		{ "gcc_mss_q6_bimc_axi_clk",		0x0031 },
		{ "gcc_apss_tcu_clk",			0x0050 },
		{ "gcc_mdp_tbu_clk",			0x0051 },
		{ "gcc_gfx_tbu_clk",			0x0052 },
		{ "gcc_gfx_tcu_clk",			0x0053 },
		{ "gcc_venus_tbu_clk",			0x0054 },
		{ "gcc_gtcu_ahb_clk",			0x0058 },
		{ "gcc_vfe_tbu_clk",			0x005a },
		{ "gcc_smmu_cfg_clk",			0x005b },
		{ "gcc_jpeg_tbu_clk",			0x005c },
		{ "gcc_usb_hs_system_clk",		0x0060 },
		{ "gcc_usb_hs_ahb_clk",			0x0061 },
		{ "gcc_usb2a_phy_sleep_clk",		0x0063 },
		{ "gcc_usb_hs_phy_cfg_ahb_clk",		0x0064 },
		{ "gcc_sdcc1_apps_clk",			0x0068 },
		{ "gcc_sdcc1_ahb_clk",			0x0069 },
		{ "gcc_sdcc1_ice_core_clk",		0x006a },
		{ "gcc_sdcc2_apps_clk",			0x0070 },
		{ "gcc_sdcc2_ahb_clk",			0x0071 },
		{ "gcc_blsp1_ahb_clk",			0x0088 },
		{ "gcc_blsp1_qup1_spi_apps_clk",	0x008a },
		{ "gcc_blsp1_qup1_i2c_apps_clk",	0x008b },
		{ "gcc_blsp1_uart1_apps_clk",		0x008c },
		{ "gcc_blsp1_qup2_spi_apps_clk",	0x008e },
		{ "gcc_blsp1_qup2_i2c_apps_clk",	0x0090 },
		{ "gcc_blsp1_uart2_apps_clk",		0x0091 },
		{ "gcc_blsp1_qup3_spi_apps_clk",	0x0093 },
		{ "gcc_blsp1_qup3_i2c_apps_clk",	0x0094 },
		{ "gcc_blsp1_qup4_spi_apps_clk",	0x0098 },
		{ "gcc_blsp1_qup4_i2c_apps_clk",	0x0099 },
		{ "gcc_camss_ahb_clk",			0x00a8 },
		{ "gcc_camss_top_ahb_clk",		0x00a9 },
		{ "gcc_camss_micro_ahb_clk",		0x00aa },
		{ "gcc_camss_gp0_clk",			0x00ab },
		{ "gcc_camss_gp1_clk",			0x00ac },
		{ "gcc_camss_mclk0_clk",		0x00ad },
		{ "gcc_camss_mclk1_clk",		0x00ae },
		{ "gcc_camss_cci_clk",			0x00af },
		{ "gcc_camss_cci_ahb_clk",		0x00b0 },
		{ "gcc_camss_csi0phytimer_clk",		0x00b1 },
		{ "gcc_camss_csi1phytimer_clk",		0x00b2 },
		{ "gcc_camss_jpeg0_clk",		0x00b3 },
		{ "gcc_camss_jpeg_ahb_clk",		0x00b4 },
		{ "gcc_camss_jpeg_axi_clk",		0x00b5 },
		{ "gcc_camss_vfe0_clk",			0x00b8 },
		{ "gcc_camss_cpp_clk",			0x00b9 },
		{ "gcc_camss_cpp_ahb_clk",		0x00ba },
		{ "gcc_camss_vfe_ahb_clk",		0x00bb },
		{ "gcc_camss_vfe_axi_clk",		0x00bc },
		{ "gcc_camss_csi_vfe0_clk",		0x00bf },
		{ "gcc_camss_csi0_clk",			0x00c0 },
		{ "gcc_camss_csi0_ahb_clk",		0x00c1 },
		{ "gcc_camss_csi0phy_clk",		0x00c2 },
		{ "gcc_camss_csi0rdi_clk",		0x00c3 },
		{ "gcc_camss_csi0pix_clk",		0x00c4 },
		{ "gcc_camss_csi1_clk",			0x00c5 },
		{ "gcc_camss_csi1_ahb_clk",		0x00c6 },
		{ "gcc_camss_csi1phy_clk",		0x00c7 },
		{ "gcc_pdm_ahb_clk",			0x00d0 },
		{ "gcc_pdm2_clk",			0x00d2 },
		{ "gcc_prng_ahb_clk",			0x00d8 },
		{ "gcc_camss_csi1rdi_clk",		0x00e0 },
		{ "gcc_camss_csi1pix_clk",		0x00e1 },
		{ "gcc_camss_ispif_ahb_clk",		0x00e2 },
		{ "gcc_camss_csi2_clk",			0x00e3 },
		{ "gcc_camss_csi2_ahb_clk",		0x00e4 },
		{ "gcc_camss_csi2phy_clk",		0x00e5 },
		{ "gcc_camss_csi2rdi_clk",		0x00e6 },
		{ "gcc_camss_csi2pix_clk",		0x00e7 },
		{ "gcc_cpp_tbu_clk",			0x00e9 },
		{ "gcc_camss_vfe1_axi_clk",		0x00ea },
		{ "gcc_camss_vfe1_ahb_clk",		0x00eb },
		{ "gcc_camss_vfe1_clk",			0x00ec },
		{ "gcc_mdp_rt_tbu_clk",			0x00ee },
		{ "gcc_usb_fs_system_clk",		0x00f0 },
		{ "gcc_usb_fs_ahb_clk",			0x00f1 },
		{ "gcc_usb_fs_ic_clk",			0x00f4 },
		{ "gcc_boot_rom_ahb_clk",		0x00f8 },
		{ "gcc_crypto_clk",			0x0138 },
		{ "gcc_crypto_axi_clk",			0x0139 },
		{ "gcc_crypto_ahb_clk",			0x013a },
		{ "gcc_camss_csi_vfe1_clk",		0x01b4 },
		{ "gcc_camss_cpp_axi_clk",		0x01b5 },
		{ "gcc_venus0_core0_vcodec0_clk",	0x01b8 },
		{ "gcc_mdss_pclk1_clk",			0x01ba },
		{ "gcc_mdss_byte1_clk",			0x01bb },
		{ "gcc_mdss_esc1_clk",			0x01bc },
		{ "gcc_camss_mclk2_clk",		0x01bd },
		{ "gcc_oxili_timer_clk",		0x01e9 },
		{ "gcc_oxili_gfx3d_clk",		0x01ea },
		{ "gcc_oxili_ahb_clk",			0x01eb },
		{ "gcc_oxili_aon_clk",			0x01ee },
		{ "gcc_oxili_gmem_clk",			0x01f0 },
		{ "gcc_venus0_vcodec0_clk",		0x01f1 },
		{ "gcc_venus0_axi_clk",			0x01f2 },
		{ "gcc_venus0_ahb_clk",			0x01f3 },
		{ "gcc_mdss_ahb_clk",			0x01f6 },
		{ "gcc_mdss_axi_clk",			0x01f7 },
		{ "gcc_mdss_pclk0_clk",			0x01f8 },
		{ "gcc_mdss_mdp_clk",			0x01f9 },
		{ "gcc_mdss_vsync_clk",			0x01fb },
		{ "gcc_mdss_byte0_clk",			0x01fc },
		{ "gcc_mdss_esc0_clk",			0x01fd },
		{ "gcc_vfe1_tbu_clk",			0x0209 },
		{ "gcc_gfx_1_tbu_clk",			0x020b },
		{ "gcc_venus_1_tbu_clk",		0x020c },
		{ "gcc_rbcpr_gfx_clk",			0x0218 },
		{ "gcc_rbcpr_gfx_ahb_clk",		0x0219 },
		{ "gcc_sdcc3_apps_clk",			0x0220 },
		{ "gcc_sdcc3_ahb_clk",			0x0221 },
		{ "gcc_blsp2_ahb_clk",			0x0228 },
		{ "gcc_blsp2_qup1_spi_apps_clk",	0x022a },
		{ "gcc_blsp2_qup1_i2c_apps_clk",	0x022b },
		{ "gcc_blsp2_uart1_apps_clk",		0x022c },
		{ "gcc_blsp2_qup2_spi_apps_clk",	0x022e },
		{ "gcc_blsp2_qup2_i2c_apps_clk",	0x0230 },
		{ "gcc_blsp2_uart2_apps_clk",		0x0231 },
		{ "gcc_blsp2_qup3_spi_apps_clk",	0x0233 },
		{ "gcc_blsp2_qup3_i2c_apps_clk",	0x0234 },
		{ "gcc_blsp2_qup4_spi_apps_clk",	0x0238 },
		{ "gcc_blsp2_qup4_i2c_apps_clk",	0x0239 },
		{ "gcc_dcc_clk",			0x0268 },
		{ "gcc_aps_0_clk",			0x0280 },
		{ "gcc_aps_1_clk",			0x0288 },
		{ "wcnss_m_clk",			0x0198 },
	),
	.hw.init = &(struct clk_init_data){
		.name = "gcc_debug_mux",
		.ops = &clk_debug_mux_ops,
		.parent_names = debug_mux_parent_names,
		.num_parents = ARRAY_SIZE(debug_mux_parent_names),
		.flags = CLK_IS_MEASURE,
	},
};

/*
static struct clk_lookup msm_clocks_measure[] = {
	CLK_LOOKUP_OF("measure", gcc_debug_mux, "debug"),
	CLK_LIST(debug_cpu_clk),
};
*/
static int msm_clock_debug_probe(struct platform_device *pdev)
{
	struct clk *clk = NULL;
	int ret = 0, count = 0;
/*
	clk_ops_debug_mux = clk_ops_gen_mux;
	clk_ops_debug_mux.get_rate = measure_get_rate;
*/

	clk = devm_clk_get(&pdev->dev, "xo_clk_src");
	if (IS_ERR(clk)) {
		if (PTR_ERR(clk) != -EPROBE_DEFER)
			dev_err(&pdev->dev, "Unable to get xo clock\n");
		return PTR_ERR(clk);
	}

	debug_mux_priv.cxo = clk;

/*
	debug_cpu_clk.c.parent = devm_clk_get(&pdev->dev, "debug_cpu_clk");
	if (IS_ERR(debug_cpu_clk.c.parent)) {
		dev_err(&pdev->dev, "Failed to get CPU debug Mux\n");
		return PTR_ERR(debug_cpu_clk.c.parent);
	}
*/

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

	//gcc_debug_mux.num_parent_regmap = count;
	gcc_debug_mux.regmap = devm_kzalloc(&pdev->dev,
				sizeof(struct regmap *) * count, GFP_KERNEL);
	if (!gcc_debug_mux.regmap)
		return -ENOMEM;

	if (of_get_property(pdev->dev.of_node, "qcom,gcc", NULL)) {
		gcc_debug_mux.regmap[GCC] =
			syscon_regmap_lookup_by_phandle(pdev->dev.of_node,
					"qcom,gcc");
		if (IS_ERR(gcc_debug_mux.regmap[GCC]))
			return PTR_ERR(gcc_debug_mux.regmap[GCC]);
	}

	if (of_get_property(pdev->dev.of_node, "qcom,cpu", NULL)) {
		gcc_debug_mux.regmap[CPU] =
			syscon_regmap_lookup_by_phandle(pdev->dev.of_node,
					"qcom,cpu");
		if (IS_ERR(gcc_debug_mux.regmap[CPU]))
			return PTR_ERR(gcc_debug_mux.regmap[CPU]);
	}

	if (of_get_property(pdev->dev.of_node, "qcom,gpu", NULL)) {
		gcc_debug_mux.regmap[GPU_CC] =
			syscon_regmap_lookup_by_phandle(pdev->dev.of_node,
					"qcom,gpu");
		if (IS_ERR(gcc_debug_mux.regmap[GPU_CC]))
			return PTR_ERR(gcc_debug_mux.regmap[GPU_CC]);
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

static struct of_device_id msm_clock_debug_match_table[] = {
	{ .compatible = "qcom,cc-debug-8976" },
	{}
};

static struct platform_driver msm_clock_debug_driver = {
	.probe = msm_clock_debug_probe,
	.driver = {
		.name = "qcom,cc-debug-8976",
		.of_match_table = msm_clock_debug_match_table,
		.owner = THIS_MODULE,
	},
};

static int __init msm_clock_debug_init(void)
{
	return platform_driver_register(&msm_clock_debug_driver);
}
late_initcall(msm_clock_debug_init);

/* MDSS Clocks */
static struct clk_regmap *gcc_mdss_msm8976_clocks[] = {
	[GCC_MDSS_BYTE0_CLK_SRC]	= &byte0_clk_src.clkr,
	[GCC_MDSS_BYTE1_CLK_SRC]	= &byte1_clk_src.clkr,
	[GCC_MDSS_BYTE0_CLK]		= &gcc_mdss_byte0_clk.clkr,
	[GCC_MDSS_BYTE1_CLK]		= &gcc_mdss_byte1_clk.clkr,
	[GCC_MDSS_PCLK0_CLK_SRC]	= &pclk0_clk_src.clkr,
	[GCC_MDSS_PCLK1_CLK_SRC]	= &pclk1_clk_src.clkr,
	[GCC_MDSS_PCLK0_CLK]		= &gcc_mdss_pclk0_clk.clkr,
	[GCC_MDSS_PCLK1_CLK]		= &gcc_mdss_pclk1_clk.clkr,
};

static const struct qcom_cc_desc gcc_msm8976_mdss_desc = {
	.config		= &gcc_msm8976_regmap_config,
	.clks		= gcc_mdss_msm8976_clocks,
	.num_clks	= ARRAY_SIZE(gcc_mdss_msm8976_clocks),
};

static struct of_device_id gcc_mdss_8976_match_table[] = {
	{ .compatible = "qcom,gcc-mdss-8976" },
	{ .compatible = "qcom,gcc-mdss-8976-v1" },
	{}
};

static int msm_gcc_8976_mdss_probe(struct platform_device *pdev)
{
	void __iomem *base;
	struct resource *res;
	struct device *dev = &pdev->dev;
	struct regmap *regmap;
//	struct clk *xo_req = NULL;
	int ret;
/*
	xo_req = devm_clk_get(&pdev->dev, "cxo_a");
	if (IS_ERR(xo_req)) {
		dev_err(&pdev->dev, "The CXO_A clock cannot be found.\n");

		if (PTR_ERR(xo_req) != -EPROBE_DEFER)
			dev_err(&pdev->dev, "Critical error %ld. Bailing out.\n", PTR_ERR(xo_req));

		return PTR_ERR(xo_req);
	}
*/
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	base = devm_ioremap(dev, res->start, resource_size(res));
	if (IS_ERR(base)) {
		dev_err(&pdev->dev, "Unable to map MDSS clock controller.\n");
		return -EINVAL;
	}

	regmap = devm_regmap_init_mmio(dev, base, &gcc_msm8976_regmap_config);
	if (IS_ERR(regmap)) {
		dev_err(&pdev->dev, "Unable to map MDSS MMIO.\n");
		return PTR_ERR(regmap);
	}

	ret = qcom_cc_really_probe(pdev, &gcc_msm8976_mdss_desc, regmap);
	if (ret) {
		dev_err(&pdev->dev, "Failed to register MDSS clocks\n");
		return ret;
	}

	dev_err(&pdev->dev, "Registered GCC MDSS clocks.\n");

	return ret;
}

static struct platform_driver gcc_mdss_8976_driver = {
	.probe = msm_gcc_8976_mdss_probe,
	.driver = {
		.name = "gcc-mdss-8976",
		.of_match_table = gcc_mdss_8976_match_table,
		.owner = THIS_MODULE,
	},
};

/*
static int __init gcc_mdss_8976_init(void)
{
	return platform_driver_register(&gcc_mdss_8976_driver);
}
//core_initcall_sync(gcc_mdss_8976_init);
arch_initcall_sync(gcc_mdss_8976_init);

static void gcc_mdss_8976_exit(void)
{
	return platform_driver_unregister(&gcc_mdss_8976_driver);
}
module_exit(gcc_mdss_8976_exit)
*/

/* GFX Clocks */
static struct clk_regmap *gcc_gfx3d_msm8976_clocks[] = {
	[GCC_GFX3D_CLK_SRC]		= &gfx3d_clk_src.clkr,
	[GCC_GFX3D_OXILI_CLK]		= &gcc_oxili_gfx3d_clk.clkr,
	[GCC_GFX3D_BIMC_CLK]		= &gcc_bimc_gfx_clk.clkr,
	[GCC_GFX3D_OXILI_AHB_CLK]	= &gcc_oxili_ahb_clk.clkr,
	[GCC_GFX3D_OXILI_AON_CLK]	= &gcc_oxili_aon_clk.clkr,
	[GCC_GFX3D_OXILI_GMEM_CLK]	= &gcc_oxili_gmem_clk.clkr,
	[GCC_GFX3D_OXILI_TIMER_CLK]	= &gcc_oxili_timer_clk.clkr,
	[GCC_GFX3D_TBU0_CLK]		= &gcc_gfx_tbu_clk.clkr,
	[GCC_GFX3D_TBU1_CLK]		= &gcc_gfx_1_tbu_clk.clkr,
	[GCC_GFX3D_TCU_CLK]		= &gcc_gfx_tcu_clk.clkr,
	[GCC_GFX3D_GTCU_AHB_CLK]	= &gcc_gtcu_ahb_clk.clkr,
};

static const struct qcom_cc_desc gcc_msm8976_gfx3d_desc = {
	.config		= &gcc_msm8976_regmap_config,
	.clks		= gcc_gfx3d_msm8976_clocks,
	.num_clks	= ARRAY_SIZE(gcc_gfx3d_msm8976_clocks),
};

static struct of_device_id gcc_gfx_8976_match_table[] = {
	{ .compatible = "qcom,gcc-gfx-8976" },
	{}
};

static int msm_gcc_8976_gfx_probe(struct platform_device *pdev)
{
	void __iomem *base;
	struct resource *res;
	struct device *dev = &pdev->dev;
	struct regmap *regmap;
	int ret;
	u32 val;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	base = devm_ioremap(dev, res->start, resource_size(res));
	if (IS_ERR(base)) {
		dev_err(&pdev->dev, "Unable to map GFX3D clock controller.\n");
		return -EINVAL;
	}

	regmap = devm_regmap_init_mmio(dev, base, &gcc_msm8976_regmap_config);
	if (IS_ERR(regmap)) {
		dev_err(&pdev->dev, "Unable to map GFX3D MMIO.\n");
		return PTR_ERR(regmap);
	}

	vdd_gfx.regulator[0] = devm_regulator_get(&pdev->dev, "vdd_gfx");
	if (IS_ERR(vdd_gfx.regulator[0])) {
		if (PTR_ERR(vdd_gfx.regulator[0]) != -EPROBE_DEFER)
			dev_err(&pdev->dev, "Unable to get vdd_gfx regulator!");
		return PTR_ERR(vdd_gfx.regulator[0]);
	}

	vdd_dig_gfx.regulator[0] = devm_regulator_get(&pdev->dev, "vdd_dig_gfx");
	if (IS_ERR(vdd_gfx.regulator[0])) {
		if (PTR_ERR(vdd_dig_gfx.regulator[0]) != -EPROBE_DEFER)
			dev_err(&pdev->dev, "Unable to get vdd_dig_gfx regulator!");
		return PTR_ERR(vdd_dig_gfx.regulator[0]);
	}

	ret = qcom_cc_really_probe(pdev, &gcc_msm8976_gfx3d_desc, regmap);
	if (ret) {
		dev_err(&pdev->dev, "Failed to register GPUCC clocks\n");
		return ret;
	}

	/* Oxili Ocmem in GX rail: OXILI_GMEM_CLAMP_IO */
	regmap_update_bits(regmap, 0x5B00C, BIT(0), 0);

	/* Configure Sleep and Wakeup cycles for OXILI clock */
//	regmap_update_bits(regmap, 0x59020, 0xF0, (0 << 4));
	val = regmap_read(regmap, 0x59020, &val);
	val &= ~0xF0;
	val |= (0 << 4);
	regmap_write(regmap, 0x59020, val);

/*
	regval = readl_relaxed(GCC_REG_BASE(OXILI_GFX3D_CBCR));
	regval &= ~0xF0;
	regval |= CLKFLAG_SLEEP_CYCLES << 4;
	writel_relaxed(regval, GCC_REG_BASE(OXILI_GFX3D_CBCR));
*/
	dev_info(&pdev->dev, "Registered GCC GFX clocks.\n");

	return ret;
}

static struct platform_driver gcc_gfx_8976_driver = {
	.probe = msm_gcc_8976_gfx_probe,
	.driver = {
		.name = "gcc-gfx-8976",
		.of_match_table = gcc_gfx_8976_match_table,
		.owner = THIS_MODULE,
	},
};
/*
static int __init gcc_gfx_8976_init(void)
{
	return platform_driver_register(&gcc_gfx_8976_driver);
}
arch_initcall_sync(gcc_gfx_8976_init);

static void gcc_gfx_8976_exit(void)
{
	return platform_driver_unregister(&gcc_gfx_8976_driver);
}
module_exit(gcc_gfx_8976_exit)
*/
