/*
 * Copyright (c) 2015-2016, The Linux Foundation. All rights reserved.
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
#include <linux/init.h>
#include <linux/err.h>
#include <linux/ctype.h>
#include <linux/io.h>
#include <linux/spinlock.h>
#include <linux/clk.h>
#include <linux/regulator/consumer.h>
#include <linux/platform_device.h>
#include <linux/pm_opp.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <soc/qcom/clock-local2.h>
#include <soc/qcom/clock-pll.h>
#include <soc/qcom/clock-voter.h>
#include <soc/qcom/rpm-smd.h>
#include <soc/qcom/clock-rpm.h>

#include <linux/clk/msm-clock-generic.h>
#include <linux/regulator/rpm-smd-regulator.h>

#include <dt-bindings/clock/msm-clocks-8976.h>
#include <dt-bindings/clock/msm-clocks-hwio-8976.h>

#include "clock.h"

enum {
	GCC_BASE,
	GFX_BASE,
	MDSS_BASE,
	N_BASES,
};

static void __iomem *virt_bases[N_BASES];

#define GCC_REG_BASE(x) (void __iomem *)(virt_bases[GCC_BASE] + (x))

DEFINE_CLK_RPM_SMD_BRANCH(xo_clk_src, xo_a_clk_src,
				RPM_MISC_CLK_TYPE, XO_ID, 19200000);

DEFINE_CLK_RPM_SMD(bimc_clk, bimc_a_clk, RPM_MEM_CLK_TYPE, BIMC_ID, NULL);
DEFINE_CLK_RPM_SMD(ipa_clk, ipa_a_clk, RPM_IPA_CLK_TYPE, IPA_ID, NULL);
DEFINE_CLK_RPM_SMD(pcnoc_clk, pcnoc_a_clk, RPM_BUS_CLK_TYPE, PCNOC_ID, NULL);
DEFINE_CLK_RPM_SMD(snoc_clk, snoc_a_clk, RPM_BUS_CLK_TYPE, SNOC_ID, NULL);
DEFINE_CLK_RPM_SMD(sysmmnoc_clk, sysmmnoc_a_clk, RPM_BUS_CLK_TYPE,
							SYSMMNOC_ID, NULL);
DEFINE_CLK_RPM_SMD_QDSS(qdss_clk, qdss_a_clk, RPM_MISC_CLK_TYPE, QDSS_ID);

/* SMD_XO_BUFFER */
DEFINE_CLK_RPM_SMD_XO_BUFFER(bb_clk1, bb_clk1_a, BB_CLK1_ID);
DEFINE_CLK_RPM_SMD_XO_BUFFER(bb_clk2, bb_clk2_a, BB_CLK2_ID);
DEFINE_CLK_RPM_SMD_XO_BUFFER(rf_clk2, rf_clk2_a, RF_CLK2_ID);
DEFINE_CLK_RPM_SMD_XO_BUFFER(div_clk2, div_clk2_a, DIV_CLK2_ID);

DEFINE_CLK_RPM_SMD_XO_BUFFER_PINCTRL(bb_clk1_pin, bb_clk1_a_pin, BB_CLK1_ID);
DEFINE_CLK_RPM_SMD_XO_BUFFER_PINCTRL(bb_clk2_pin, bb_clk2_a_pin, BB_CLK2_ID);

static DEFINE_CLK_VOTER(bimc_msmbus_clk, &bimc_clk.c, LONG_MAX);
static DEFINE_CLK_VOTER(bimc_msmbus_a_clk, &bimc_a_clk.c, LONG_MAX);
static DEFINE_CLK_VOTER(bimc_usb_a_clk, &bimc_a_clk.c, LONG_MAX);
static DEFINE_CLK_VOTER(bimc_usb_clk, &bimc_clk.c, LONG_MAX);
static DEFINE_CLK_VOTER(pcnoc_keepalive_a_clk, &pcnoc_a_clk.c, LONG_MAX);
static DEFINE_CLK_VOTER(pcnoc_msmbus_clk, &pcnoc_clk.c, LONG_MAX);
static DEFINE_CLK_VOTER(pcnoc_msmbus_a_clk, &pcnoc_a_clk.c, LONG_MAX);
static DEFINE_CLK_VOTER(pcnoc_usb_a_clk, &pcnoc_a_clk.c, LONG_MAX);
static DEFINE_CLK_VOTER(pcnoc_usb_clk, &pcnoc_clk.c, LONG_MAX);
static DEFINE_CLK_VOTER(snoc_msmbus_clk, &snoc_clk.c, LONG_MAX);
static DEFINE_CLK_VOTER(snoc_msmbus_a_clk, &snoc_a_clk.c, LONG_MAX);
static DEFINE_CLK_VOTER(snoc_usb_a_clk, &snoc_a_clk.c, LONG_MAX);
static DEFINE_CLK_VOTER(snoc_usb_clk, &snoc_clk.c, LONG_MAX);
static DEFINE_CLK_VOTER(sysmmnoc_msmbus_clk,  &sysmmnoc_clk.c,  LONG_MAX);
static DEFINE_CLK_VOTER(sysmmnoc_msmbus_a_clk,  &sysmmnoc_a_clk.c,  LONG_MAX);

static DEFINE_CLK_BRANCH_VOTER(xo_lpm_clk, &xo_clk_src.c);
static DEFINE_CLK_BRANCH_VOTER(xo_otg_clk, &xo_clk_src.c);
static DEFINE_CLK_BRANCH_VOTER(xo_pil_lpass_clk, &xo_clk_src.c);
static DEFINE_CLK_BRANCH_VOTER(xo_pil_mss_clk, &xo_clk_src.c);
static DEFINE_CLK_BRANCH_VOTER(xo_pil_pronto_clk, &xo_clk_src.c);
static DEFINE_CLK_BRANCH_VOTER(xo_wlan_clk, &xo_clk_src.c);

DEFINE_CLK_DUMMY(wcnss_m_clk, 0);
DEFINE_EXT_CLK(debug_cpu_clk, NULL);

static unsigned int soft_vote_gpll0;

static struct pll_vote_clk gpll0_clk_src = {
	.en_reg = (void __iomem *)APCS_GPLL_ENA_VOTE,
	.en_mask = BIT(0),
	.status_reg = (void __iomem *)GPLL0_STATUS,
	.status_mask = BIT(17),
	.soft_vote = &soft_vote_gpll0,
	.soft_vote_mask = PLL_SOFT_VOTE_PRIMARY,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.parent = &xo_clk_src.c,
		.rate = 800000000,
		.dbg_name = "gpll0_clk_src",
		.ops = &clk_ops_pll_acpu_vote,
		CLK_INIT(gpll0_clk_src.c),
	},
};

/* Don't vote for xo if using this clock to allow xo shutdown */
static struct pll_vote_clk gpll0_ao_clk_src = {
	.en_reg = (void __iomem *)APCS_GPLL_ENA_VOTE,
	.en_mask = BIT(0),
	.status_reg = (void __iomem *)GPLL0_STATUS,
	.status_mask = BIT(17),
	.soft_vote = &soft_vote_gpll0,
	.soft_vote_mask = PLL_SOFT_VOTE_ACPU,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.parent = &xo_a_clk_src.c,
		.rate = 800000000,
		.dbg_name = "gpll0_ao_clk_src",
		.ops = &clk_ops_pll_acpu_vote,
		CLK_INIT(gpll0_ao_clk_src.c),
	},
};

static struct pll_vote_clk gpll2_clk_src = {
	.en_reg = (void __iomem *)APCS_GPLL_ENA_VOTE,
	.en_mask = BIT(2),
	.status_reg = (void __iomem *)GPLL2_STATUS,
	.status_mask = BIT(17),
	.base = &virt_bases[GCC_BASE],
	.c = {
		.rate = 932000000,
		.parent = &xo_clk_src.c,
		.dbg_name = "gpll2_clk_src",
		.ops = &clk_ops_pll_vote,
		CLK_INIT(gpll2_clk_src.c),
	},
};

static struct pll_vote_clk gpll3_clk_src = {
	.en_reg = (void __iomem *)APCS_GPLL_ENA_VOTE,
	.en_mask = BIT(4),
	.status_reg = (void __iomem *)GPLL3_STATUS,
	.status_mask = BIT(17),
	.base = &virt_bases[GCC_BASE],
	.c = {
		.rate = 1100000000,
		.parent = &xo_clk_src.c,
		.dbg_name = "gpll3_clk_src",
		.ops = &clk_ops_pll_vote,
		CLK_INIT(gpll3_clk_src.c),
	},
};

static struct pll_config_regs gpll3_regs = {
	.l_reg = (void __iomem *)GPLL3_L_VAL,
	.m_reg = (void __iomem *)GPLL3_M_VAL,
	.n_reg = (void __iomem *)GPLL3_N_VAL,
	.config_reg = (void __iomem *)GPLL3_USER_CTL,
	.mode_reg = (void __iomem *)GPLL3_MODE,
	.base = &virt_bases[GCC_BASE],
};

/* GPLL3 at 1100MHz, main output enabled. */
static struct pll_config gpll3_config = {
	.l = 57,
	.m = 7,
	.n = 24,
	.vco_val = 0x0,
	.vco_mask = BM(21, 20),
	.pre_div_val = 0x0,
	.pre_div_mask = BM(14, 12),
	.post_div_val = 0x0,
	.post_div_mask = BM(9, 8),
	.mn_ena_val = BIT(24),
	.mn_ena_mask = BIT(24),
	.main_output_val = BIT(0),
	.main_output_mask = BIT(0),
	.aux_output_val = BIT(1),
	.aux_output_mask = BIT(1),
};

static struct pll_vote_clk gpll4_clk_src = {
	.en_reg = (void __iomem *)APCS_GPLL_ENA_VOTE,
	.en_mask = BIT(5),
	.status_reg = (void __iomem *)GPLL4_STATUS,
	.status_mask = BIT(17),
	.base = &virt_bases[GCC_BASE],
	.c = {
		.rate = 1200000000,
		.parent = &xo_clk_src.c,
		.dbg_name = "gpll4_clk_src",
		.ops = &clk_ops_pll_vote,
		CLK_INIT(gpll4_clk_src.c),
	},
};

static struct pll_vote_clk gpll6_clk_src = {
	.en_reg = (void __iomem *)APCS_GPLL_ENA_VOTE,
	.en_mask = BIT(7),
	.status_reg = (void __iomem *)GPLL6_STATUS,
	.status_mask = BIT(17),
	.base = &virt_bases[GCC_BASE],
	.c = {
		.rate = 1080000000,
		.parent = &xo_clk_src.c,
		.dbg_name = "gpll6_clk_src",
		.ops = &clk_ops_pll_vote,
		CLK_INIT(gpll6_clk_src.c),
	},
};

DEFINE_EXT_CLK(gpll0_aux_clk_src, &gpll0_clk_src.c);
DEFINE_EXT_CLK(gpll0_aux_esc_clk_src, &gpll0_clk_src.c);
DEFINE_EXT_CLK(gpll0_out_clk_src, &gpll0_clk_src.c);
DEFINE_EXT_CLK(gpll0_out_m_clk_src, &gpll0_clk_src.c);
DEFINE_EXT_CLK(gpll0_out_mdp_clk_src, &gpll0_clk_src.c);
DEFINE_EXT_CLK(gpll2_aux_clk_src, &gpll2_clk_src.c);
DEFINE_EXT_CLK(gpll2_gfx3d_clk_src, &gpll2_clk_src.c);
DEFINE_EXT_CLK(gpll2_out_clk_src, &gpll2_clk_src.c);
DEFINE_EXT_CLK(gpll3_aux_clk_src, &gpll3_clk_src.c);
DEFINE_EXT_CLK(gpll4_aux_clk_src, &gpll4_clk_src.c);
DEFINE_EXT_CLK(gpll4_gfx3d_clk_src, &gpll4_clk_src.c);
DEFINE_EXT_CLK(gpll4_out_clk_src, &gpll4_clk_src.c);
DEFINE_EXT_CLK(gpll6_aux_clk_src, &gpll6_clk_src.c);
DEFINE_EXT_CLK(gpll6_gfx3d_clk_src, &gpll6_clk_src.c);
DEFINE_EXT_CLK(gpll6_out_clk_src, &gpll6_clk_src.c);
DEFINE_EXT_CLK(gpll6_out_m_clk_src, &gpll6_clk_src.c);

static struct clk_freq_tbl ftbl_aps_0_clk_src[] = {
	F(  19200000,             xo,    1,    0,     0),
	F( 300000000,      gpll4_out,    4,    0,     0),
	F( 540000000,          gpll6,    2,    0,     0),
	F_END
};

static struct rcg_clk aps_0_clk_src = {
	.cmd_rcgr_reg = APS_0_CMD_RCGR,
	.set_rate = set_rate_hid,
	.freq_tbl = ftbl_aps_0_clk_src,
	.current_freq = &rcg_dummy_freq,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "aps_0_clk_src",
		.ops = &clk_ops_rcg,
		VDD_DIG_FMAX_MAP2(LOWER, 300000000, NOMINAL, 540000000),
		CLK_INIT(aps_0_clk_src.c),
	},
};

static struct clk_freq_tbl ftbl_aps_1_clk_src[] = {
	F(  19200000,             xo,    1,    0,     0),
	F( 300000000,      gpll4_out,    4,    0,     0),
	F( 540000000,          gpll6,    2,    0,     0),
	F_END
};

static struct rcg_clk aps_1_clk_src = {
	.cmd_rcgr_reg = APS_1_CMD_RCGR,
	.set_rate = set_rate_hid,
	.freq_tbl = ftbl_aps_1_clk_src,
	.current_freq = &rcg_dummy_freq,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "aps_1_clk_src",
		.ops = &clk_ops_rcg,
		VDD_DIG_FMAX_MAP2(LOWER, 300000000, NOMINAL, 540000000),
		CLK_INIT(aps_1_clk_src.c),
	},
};

static struct clk_freq_tbl ftbl_apss_ahb_clk_src[] = {
	F(  19200000,           xo_a,    1,    0,     0),
	F(  50000000,          gpll0,   16,    0,     0),
	F(  88890000,          gpll0,    9,    0,     0),
	F( 100000000,          gpll0,    8,    0,     0),
	F_END
};

static struct rcg_clk apss_ahb_clk_src = {
	.cmd_rcgr_reg = APSS_AHB_CMD_RCGR,
	.set_rate = set_rate_hid,
	.freq_tbl = ftbl_apss_ahb_clk_src,
	.current_freq = &rcg_dummy_freq,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "apss_ahb_clk_src",
		.ops = &clk_ops_rcg,
		CLK_INIT(apss_ahb_clk_src.c),
	},
};

static struct clk_freq_tbl ftbl_blsp_i2c_apps_clk_src[] = {
	F(  19200000,             xo,    1,    0,     0),
	F(  50000000,          gpll0,   16,    0,     0),
	F_END
};

static struct rcg_clk blsp1_qup1_i2c_apps_clk_src = {
	.cmd_rcgr_reg = BLSP1_QUP1_I2C_APPS_CMD_RCGR,
	.set_rate = set_rate_hid,
	.freq_tbl = ftbl_blsp_i2c_apps_clk_src,
	.current_freq = &rcg_dummy_freq,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "blsp1_qup1_i2c_apps_clk_src",
		.ops = &clk_ops_rcg,
		VDD_DIG_FMAX_MAP1(LOWER, 50000000),
		CLK_INIT(blsp1_qup1_i2c_apps_clk_src.c),
	},
};

static struct clk_freq_tbl ftbl_blsp_spi_apps_clk_src[] = {
	F(    960000,             xo,   10,    1,     2),
	F(   4800000,             xo,    4,    0,     0),
	F(   9600000,             xo,    2,    0,     0),
	F(  16000000,          gpll0,   10,    1,     5),
	F(  19200000,             xo,    1,    0,     0),
	F(  25000000,          gpll0,   16,    1,     2),
	F(  50000000,          gpll0,   16,    0,     0),
	F_END
};

static struct rcg_clk blsp1_qup1_spi_apps_clk_src = {
	.cmd_rcgr_reg = BLSP1_QUP1_SPI_APPS_CMD_RCGR,
	.set_rate = set_rate_mnd,
	.freq_tbl = ftbl_blsp_spi_apps_clk_src,
	.current_freq = &rcg_dummy_freq,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "blsp1_qup1_spi_apps_clk_src",
		.ops = &clk_ops_rcg_mnd,
		VDD_DIG_FMAX_MAP2(LOWER, 25000000, NOMINAL, 50000000),
		CLK_INIT(blsp1_qup1_spi_apps_clk_src.c),
	},
};

static struct rcg_clk blsp1_qup2_i2c_apps_clk_src = {
	.cmd_rcgr_reg = BLSP1_QUP2_I2C_APPS_CMD_RCGR,
	.set_rate = set_rate_hid,
	.freq_tbl = ftbl_blsp_i2c_apps_clk_src,
	.current_freq = &rcg_dummy_freq,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "blsp1_qup2_i2c_apps_clk_src",
		.ops = &clk_ops_rcg,
		VDD_DIG_FMAX_MAP1(LOWER, 50000000),
		CLK_INIT(blsp1_qup2_i2c_apps_clk_src.c),
	},
};

static struct rcg_clk blsp1_qup2_spi_apps_clk_src = {
	.cmd_rcgr_reg = BLSP1_QUP2_SPI_APPS_CMD_RCGR,
	.set_rate = set_rate_mnd,
	.freq_tbl = ftbl_blsp_spi_apps_clk_src,
	.current_freq = &rcg_dummy_freq,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "blsp1_qup2_spi_apps_clk_src",
		.ops = &clk_ops_rcg_mnd,
		VDD_DIG_FMAX_MAP2(LOWER, 25000000, NOMINAL, 50000000),
		CLK_INIT(blsp1_qup2_spi_apps_clk_src.c),
	},
};

static struct rcg_clk blsp1_qup3_i2c_apps_clk_src = {
	.cmd_rcgr_reg = BLSP1_QUP3_I2C_APPS_CMD_RCGR,
	.set_rate = set_rate_hid,
	.freq_tbl = ftbl_blsp_i2c_apps_clk_src,
	.current_freq = &rcg_dummy_freq,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "blsp1_qup3_i2c_apps_clk_src",
		.ops = &clk_ops_rcg,
		VDD_DIG_FMAX_MAP1(LOWER, 50000000),
		CLK_INIT(blsp1_qup3_i2c_apps_clk_src.c),
	},
};

static struct rcg_clk blsp1_qup3_spi_apps_clk_src = {
	.cmd_rcgr_reg = BLSP1_QUP3_SPI_APPS_CMD_RCGR,
	.set_rate = set_rate_mnd,
	.freq_tbl = ftbl_blsp_spi_apps_clk_src,
	.current_freq = &rcg_dummy_freq,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "blsp1_qup3_spi_apps_clk_src",
		.ops = &clk_ops_rcg_mnd,
		VDD_DIG_FMAX_MAP2(LOWER, 25000000, NOMINAL, 50000000),
		CLK_INIT(blsp1_qup3_spi_apps_clk_src.c),
	},
};

static struct rcg_clk blsp1_qup4_i2c_apps_clk_src = {
	.cmd_rcgr_reg = BLSP1_QUP4_I2C_APPS_CMD_RCGR,
	.set_rate = set_rate_hid,
	.freq_tbl = ftbl_blsp_i2c_apps_clk_src,
	.current_freq = &rcg_dummy_freq,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "blsp1_qup4_i2c_apps_clk_src",
		.ops = &clk_ops_rcg,
		VDD_DIG_FMAX_MAP1(LOWER, 50000000),
		CLK_INIT(blsp1_qup4_i2c_apps_clk_src.c),
	},
};

static struct rcg_clk blsp1_qup4_spi_apps_clk_src = {
	.cmd_rcgr_reg = BLSP1_QUP4_SPI_APPS_CMD_RCGR,
	.set_rate = set_rate_mnd,
	.freq_tbl = ftbl_blsp_spi_apps_clk_src,
	.current_freq = &rcg_dummy_freq,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "blsp1_qup4_spi_apps_clk_src",
		.ops = &clk_ops_rcg_mnd,
		VDD_DIG_FMAX_MAP2(LOWER, 25000000, NOMINAL, 50000000),
		CLK_INIT(blsp1_qup4_spi_apps_clk_src.c),
	},
};

static struct clk_freq_tbl ftbl_blsp_uart_apps_clk_src[] = {
	F(   3686400,          gpll0,    1,   72, 15625),
	F(   7372800,          gpll0,    1,  144, 15625),
	F(  14745600,          gpll0,    1,  288, 15625),
	F(  16000000,          gpll0,   10,    1,     5),
	F(  19200000,             xo,    1,    0,     0),
	F(  24000000,          gpll0,    1,    3,   100),
	F(  25000000,          gpll0,   16,    1,     2),
	F(  32000000,          gpll0,    1,    1,    25),
	F(  40000000,          gpll0,    1,    1,    20),
	F(  46400000,          gpll0,    1,   29,   500),
	F(  48000000,          gpll0,    1,    3,    50),
	F(  51200000,          gpll0,    1,    8,   125),
	F(  56000000,          gpll0,    1,    7,   100),
	F(  58982400,          gpll0,    1, 1152, 15625),
	F(  60000000,          gpll0,    1,    3,    40),
	F(  64000000,          gpll0,    1,    2,    25),
	F_END
};

static struct rcg_clk blsp1_uart1_apps_clk_src = {
	.cmd_rcgr_reg = BLSP1_UART1_APPS_CMD_RCGR,
	.set_rate = set_rate_mnd,
	.freq_tbl = ftbl_blsp_uart_apps_clk_src,
	.current_freq = &rcg_dummy_freq,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "blsp1_uart1_apps_clk_src",
		.ops = &clk_ops_rcg_mnd,
		VDD_DIG_FMAX_MAP2(LOWER, 32000000, NOMINAL, 64000000),
		CLK_INIT(blsp1_uart1_apps_clk_src.c),
	},
};

static struct rcg_clk blsp1_uart2_apps_clk_src = {
	.cmd_rcgr_reg = BLSP1_UART2_APPS_CMD_RCGR,
	.set_rate = set_rate_mnd,
	.freq_tbl = ftbl_blsp_uart_apps_clk_src,
	.current_freq = &rcg_dummy_freq,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "blsp1_uart2_apps_clk_src",
		.ops = &clk_ops_rcg_mnd,
		VDD_DIG_FMAX_MAP2(LOWER, 32000000, NOMINAL, 64000000),
		CLK_INIT(blsp1_uart2_apps_clk_src.c),
	},
};

static struct rcg_clk blsp2_qup1_i2c_apps_clk_src = {
	.cmd_rcgr_reg = BLSP2_QUP1_I2C_APPS_CMD_RCGR,
	.set_rate = set_rate_hid,
	.freq_tbl = ftbl_blsp_i2c_apps_clk_src,
	.current_freq = &rcg_dummy_freq,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "blsp2_qup1_i2c_apps_clk_src",
		.ops = &clk_ops_rcg,
		VDD_DIG_FMAX_MAP1(LOWER, 50000000),
		CLK_INIT(blsp2_qup1_i2c_apps_clk_src.c),
	},
};

static struct rcg_clk blsp2_qup1_spi_apps_clk_src = {
	.cmd_rcgr_reg = BLSP2_QUP1_SPI_APPS_CMD_RCGR,
	.set_rate = set_rate_mnd,
	.freq_tbl = ftbl_blsp_spi_apps_clk_src,
	.current_freq = &rcg_dummy_freq,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "blsp2_qup1_spi_apps_clk_src",
		.ops = &clk_ops_rcg_mnd,
		VDD_DIG_FMAX_MAP2(LOWER, 25000000, NOMINAL, 50000000),
		CLK_INIT(blsp2_qup1_spi_apps_clk_src.c),
	},
};

static struct rcg_clk blsp2_qup2_i2c_apps_clk_src = {
	.cmd_rcgr_reg = BLSP2_QUP2_I2C_APPS_CMD_RCGR,
	.set_rate = set_rate_hid,
	.freq_tbl = ftbl_blsp_i2c_apps_clk_src,
	.current_freq = &rcg_dummy_freq,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "blsp2_qup2_i2c_apps_clk_src",
		.ops = &clk_ops_rcg,
		VDD_DIG_FMAX_MAP1(LOWER, 50000000),
		CLK_INIT(blsp2_qup2_i2c_apps_clk_src.c),
	},
};

static struct rcg_clk blsp2_qup2_spi_apps_clk_src = {
	.cmd_rcgr_reg = BLSP2_QUP2_SPI_APPS_CMD_RCGR,
	.set_rate = set_rate_mnd,
	.freq_tbl = ftbl_blsp_spi_apps_clk_src,
	.current_freq = &rcg_dummy_freq,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "blsp2_qup2_spi_apps_clk_src",
		.ops = &clk_ops_rcg_mnd,
		VDD_DIG_FMAX_MAP2(LOWER, 25000000, NOMINAL, 50000000),
		CLK_INIT(blsp2_qup2_spi_apps_clk_src.c),
	},
};

static struct rcg_clk blsp2_qup3_i2c_apps_clk_src = {
	.cmd_rcgr_reg = BLSP2_QUP3_I2C_APPS_CMD_RCGR,
	.set_rate = set_rate_hid,
	.freq_tbl = ftbl_blsp_i2c_apps_clk_src,
	.current_freq = &rcg_dummy_freq,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "blsp2_qup3_i2c_apps_clk_src",
		.ops = &clk_ops_rcg,
		VDD_DIG_FMAX_MAP1(LOWER, 50000000),
		CLK_INIT(blsp2_qup3_i2c_apps_clk_src.c),
	},
};

static struct rcg_clk blsp2_qup3_spi_apps_clk_src = {
	.cmd_rcgr_reg = BLSP2_QUP3_SPI_APPS_CMD_RCGR,
	.set_rate = set_rate_mnd,
	.freq_tbl = ftbl_blsp_spi_apps_clk_src,
	.current_freq = &rcg_dummy_freq,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "blsp2_qup3_spi_apps_clk_src",
		.ops = &clk_ops_rcg_mnd,
		VDD_DIG_FMAX_MAP2(LOWER, 25000000, NOMINAL, 50000000),
		CLK_INIT(blsp2_qup3_spi_apps_clk_src.c),
	},
};

static struct rcg_clk blsp2_qup4_i2c_apps_clk_src = {
	.cmd_rcgr_reg = BLSP2_QUP4_I2C_APPS_CMD_RCGR,
	.set_rate = set_rate_hid,
	.freq_tbl = ftbl_blsp_i2c_apps_clk_src,
	.current_freq = &rcg_dummy_freq,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "blsp2_qup4_i2c_apps_clk_src",
		.ops = &clk_ops_rcg,
		VDD_DIG_FMAX_MAP1(LOWER, 50000000),
		CLK_INIT(blsp2_qup4_i2c_apps_clk_src.c),
	},
};

static struct rcg_clk blsp2_qup4_spi_apps_clk_src = {
	.cmd_rcgr_reg = BLSP2_QUP4_SPI_APPS_CMD_RCGR,
	.set_rate = set_rate_mnd,
	.freq_tbl = ftbl_blsp_spi_apps_clk_src,
	.current_freq = &rcg_dummy_freq,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "blsp2_qup4_spi_apps_clk_src",
		.ops = &clk_ops_rcg_mnd,
		VDD_DIG_FMAX_MAP2(LOWER, 25000000, NOMINAL, 50000000),
		CLK_INIT(blsp2_qup4_spi_apps_clk_src.c),
	},
};

static struct rcg_clk blsp2_uart1_apps_clk_src = {
	.cmd_rcgr_reg = BLSP2_UART1_APPS_CMD_RCGR,
	.set_rate = set_rate_mnd,
	.freq_tbl = ftbl_blsp_uart_apps_clk_src,
	.current_freq = &rcg_dummy_freq,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "blsp2_uart1_apps_clk_src",
		.ops = &clk_ops_rcg_mnd,
		VDD_DIG_FMAX_MAP2(LOWER, 32000000, NOMINAL, 64000000),
		CLK_INIT(blsp2_uart1_apps_clk_src.c),
	},
};

static struct rcg_clk blsp2_uart2_apps_clk_src = {
	.cmd_rcgr_reg = BLSP2_UART2_APPS_CMD_RCGR,
	.set_rate = set_rate_mnd,
	.freq_tbl = ftbl_blsp_uart_apps_clk_src,
	.current_freq = &rcg_dummy_freq,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "blsp2_uart2_apps_clk_src",
		.ops = &clk_ops_rcg_mnd,
		VDD_DIG_FMAX_MAP2(LOWER, 32000000, NOMINAL, 64000000),
		CLK_INIT(blsp2_uart2_apps_clk_src.c),
	},
};

static struct clk_freq_tbl ftbl_cci_clk_src[] = {
	F(  19200000,             xo,    1,    0,     0),
	F(  37500000,      gpll0_aux,    1,    3,    64),
	F_END
};

static struct rcg_clk cci_clk_src = {
	.cmd_rcgr_reg = CCI_CMD_RCGR,
	.set_rate = set_rate_mnd,
	.freq_tbl = ftbl_cci_clk_src,
	.current_freq = &rcg_dummy_freq,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "cci_clk_src",
		.ops = &clk_ops_rcg_mnd,
		VDD_DIG_FMAX_MAP2(LOWER, 19200000, NOMINAL, 37500000),
		CLK_INIT(cci_clk_src.c),
	},
};

static struct clk_freq_tbl ftbl_cpp_clk_src[] = {
	F( 160000000,          gpll0,    5,    0,     0),
	F( 240000000,      gpll4_aux,    5,    0,     0),
	F( 320000000,          gpll0,  2.5,    0,     0),
	F( 400000000,          gpll0,    2,    0,     0),
	F( 480000000,      gpll4_aux,  2.5,    0,     0),
	F_END
};

static struct rcg_clk cpp_clk_src = {
	.cmd_rcgr_reg = CPP_CMD_RCGR,
	.set_rate = set_rate_hid,
	.freq_tbl = ftbl_cpp_clk_src,
	.current_freq = &rcg_dummy_freq,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "cpp_clk_src",
		.ops = &clk_ops_rcg,
		VDD_DIG_FMAX_MAP5(LOWER, 160000000, LOW, 240000000,
		NOMINAL, 320000000, NOM_PLUS, 400000000, HIGH, 480000000),
		CLK_INIT(cpp_clk_src.c),
	},
};

static struct clk_freq_tbl ftbl_csi0_clk_src[] = {
	F( 100000000,          gpll0,    8,    0,     0),
	F( 200000000,          gpll0,    4,    0,     0),
	F( 266670000,          gpll0,    3,    0,     0),
	F_END
};

static struct rcg_clk csi0_clk_src = {
	.cmd_rcgr_reg = CSI0_CMD_RCGR,
	.set_rate = set_rate_hid,
	.freq_tbl = ftbl_csi0_clk_src,
	.current_freq = &rcg_dummy_freq,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "csi0_clk_src",
		.ops = &clk_ops_rcg,
		VDD_DIG_FMAX_MAP3(LOWER, 100000000, LOW, 200000000,
		NOM_PLUS, 266670000),
		CLK_INIT(csi0_clk_src.c),
	},
};

static struct clk_freq_tbl ftbl_csi1_clk_src[] = {
	F( 100000000,          gpll0,    8,    0,     0),
	F( 200000000,          gpll0,    4,    0,     0),
	F( 266670000,          gpll0,    3,    0,     0),
	F_END
};

static struct rcg_clk csi1_clk_src = {
	.cmd_rcgr_reg = CSI1_CMD_RCGR,
	.set_rate = set_rate_hid,
	.freq_tbl = ftbl_csi1_clk_src,
	.current_freq = &rcg_dummy_freq,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "csi1_clk_src",
		.ops = &clk_ops_rcg,
		VDD_DIG_FMAX_MAP3(LOWER, 100000000, LOW, 200000000,
		NOM_PLUS, 266670000),
		CLK_INIT(csi1_clk_src.c),
	},
};

static struct clk_freq_tbl ftbl_csi2_clk_src[] = {
	F( 100000000,          gpll0,    8,    0,     0),
	F( 200000000,          gpll0,    4,    0,     0),
	F( 266670000,          gpll0,    3,    0,     0),
	F_END
};

static struct rcg_clk csi2_clk_src = {
	.cmd_rcgr_reg = CSI2_CMD_RCGR,
	.set_rate = set_rate_hid,
	.freq_tbl = ftbl_csi2_clk_src,
	.current_freq = &rcg_dummy_freq,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "csi2_clk_src",
		.ops = &clk_ops_rcg,
		VDD_DIG_FMAX_MAP3(LOWER, 100000000, LOW, 200000000,
		NOM_PLUS, 266670000),
		CLK_INIT(csi2_clk_src.c),
	},
};

static struct clk_freq_tbl ftbl_camss_gp0_clk_src[] = {
	F( 100000000,          gpll0,    8,    0,     0),
	F( 200000000,          gpll0,    4,    0,     0),
	F( 266670000,          gpll0,    3,    0,     0),
	F_END
};

static struct rcg_clk camss_gp0_clk_src = {
	.cmd_rcgr_reg = CAMSS_GP0_CMD_RCGR,
	.set_rate = set_rate_mnd,
	.freq_tbl = ftbl_camss_gp0_clk_src,
	.current_freq = &rcg_dummy_freq,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "camss_gp0_clk_src",
		.ops = &clk_ops_rcg_mnd,
		VDD_DIG_FMAX_MAP3(LOWER, 100000000, NOMINAL, 200000000,
		HIGH, 266670000),
		CLK_INIT(camss_gp0_clk_src.c),
	},
};

static struct clk_freq_tbl ftbl_camss_gp1_clk_src[] = {
	F( 100000000,          gpll0,    8,    0,     0),
	F( 200000000,          gpll0,    4,    0,     0),
	F( 266670000,          gpll0,    3,    0,     0),
	F_END
};

static struct rcg_clk camss_gp1_clk_src = {
	.cmd_rcgr_reg = CAMSS_GP1_CMD_RCGR,
	.set_rate = set_rate_mnd,
	.freq_tbl = ftbl_camss_gp1_clk_src,
	.current_freq = &rcg_dummy_freq,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "camss_gp1_clk_src",
		.ops = &clk_ops_rcg_mnd,
		VDD_DIG_FMAX_MAP3(LOWER, 100000000, NOMINAL, 200000000,
		HIGH, 266670000),
		CLK_INIT(camss_gp1_clk_src.c),
	},
};

static struct clk_freq_tbl ftbl_jpeg0_clk_src[] = {
	F( 133330000,          gpll0,    6,    0,     0),
	F( 200000000,          gpll0,    4,    0,     0),
	F( 266666667,          gpll0,    3,    0,     0),
	F( 300000000,      gpll4_out,    4,    0,     0),
	F( 320000000,          gpll0,  2.5,    0,     0),
	F_END
};

static struct rcg_clk jpeg0_clk_src = {
	.cmd_rcgr_reg = JPEG0_CMD_RCGR,
	.set_rate = set_rate_hid,
	.freq_tbl = ftbl_jpeg0_clk_src,
	.current_freq = &rcg_dummy_freq,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "jpeg0_clk_src",
		.ops = &clk_ops_rcg,
		VDD_DIG_FMAX_MAP5(LOWER, 133330000, LOW, 200000000,
		NOMINAL, 266670000, NOM_PLUS, 300000000,
		HIGH, 320000000),
		CLK_INIT(jpeg0_clk_src.c),
	},
};

static struct clk_freq_tbl ftbl_mclk_clk_src[] = {
	F(   8000000,          gpll0,    1,    1,   100),
	F(  24000000,      gpll6_out,    1,    1,    45),
	F(  66670000,          gpll0,   12,    0,     0),
	F_END
};

static struct rcg_clk mclk0_clk_src = {
	.cmd_rcgr_reg = MCLK0_CMD_RCGR,
	.set_rate = set_rate_mnd,
	.freq_tbl = ftbl_mclk_clk_src,
	.current_freq = &rcg_dummy_freq,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "mclk0_clk_src",
		.ops = &clk_ops_rcg_mnd,
		VDD_DIG_FMAX_MAP1(LOWER, 66670000),
		CLK_INIT(mclk0_clk_src.c),
	},
};

static struct rcg_clk mclk1_clk_src = {
	.cmd_rcgr_reg = MCLK1_CMD_RCGR,
	.set_rate = set_rate_mnd,
	.freq_tbl = ftbl_mclk_clk_src,
	.current_freq = &rcg_dummy_freq,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "mclk1_clk_src",
		.ops = &clk_ops_rcg_mnd,
		VDD_DIG_FMAX_MAP1(LOWER, 66670000),
		CLK_INIT(mclk1_clk_src.c),
	},
};

static struct rcg_clk mclk2_clk_src = {
	.cmd_rcgr_reg = MCLK2_CMD_RCGR,
	.set_rate = set_rate_mnd,
	.freq_tbl = ftbl_mclk_clk_src,
	.current_freq = &rcg_dummy_freq,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "mclk2_clk_src",
		.ops = &clk_ops_rcg_mnd,
		VDD_DIG_FMAX_MAP1(LOWER, 66670000),
		CLK_INIT(mclk2_clk_src.c),
	},
};

static struct clk_freq_tbl ftbl_csi0phytimer_clk_src[] = {
	F( 100000000,          gpll0,    8,    0,     0),
	F( 200000000,          gpll0,    4,    0,     0),
	F( 266670000,          gpll0,    3,    0,     0),
	F_END
};

static struct rcg_clk csi0phytimer_clk_src = {
	.cmd_rcgr_reg = CSI0PHYTIMER_CMD_RCGR,
	.set_rate = set_rate_hid,
	.freq_tbl = ftbl_csi0phytimer_clk_src,
	.current_freq = &rcg_dummy_freq,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "csi0phytimer_clk_src",
		.ops = &clk_ops_rcg,
		VDD_DIG_FMAX_MAP3(LOWER, 100000000, LOW, 200000000,
					NOM_PLUS, 266670000),
		CLK_INIT(csi0phytimer_clk_src.c),
	},
};

static struct clk_freq_tbl ftbl_csi1phytimer_clk_src[] = {
	F( 100000000,          gpll0,    8,    0,     0),
	F( 200000000,          gpll0,    4,    0,     0),
	F( 266670000,          gpll0,    3,    0,     0),
	F_END
};

static struct rcg_clk csi1phytimer_clk_src = {
	.cmd_rcgr_reg = CSI1PHYTIMER_CMD_RCGR,
	.set_rate = set_rate_hid,
	.freq_tbl = ftbl_csi1phytimer_clk_src,
	.current_freq = &rcg_dummy_freq,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "csi1phytimer_clk_src",
		.ops = &clk_ops_rcg,
		VDD_DIG_FMAX_MAP3(LOWER, 100000000, LOW, 200000000,
		NOM_PLUS, 266670000),
		CLK_INIT(csi1phytimer_clk_src.c),
	},
};

static struct clk_freq_tbl ftbl_camss_top_ahb_clk_src[] = {
	F(  40000000,          gpll0,   10,    1,     2),
	F(  80000000,          gpll0,   10,    0,     0),
	F_END
};

static struct rcg_clk camss_top_ahb_clk_src = {
	.cmd_rcgr_reg = CAMSS_TOP_AHB_CMD_RCGR,
	.set_rate = set_rate_mnd,
	.freq_tbl = ftbl_camss_top_ahb_clk_src,
	.current_freq = &rcg_dummy_freq,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "camss_top_ahb_clk_src",
		.ops = &clk_ops_rcg_mnd,
		VDD_DIG_FMAX_MAP2(LOWER, 40000000, LOW, 80000000),
		CLK_INIT(camss_top_ahb_clk_src.c),
	},
};

static struct clk_freq_tbl ftbl_vfe0_clk_src[] = {
	F(  50000000,          gpll0,   16,    0,     0),
	F(  80000000,          gpll0,   10,    0,     0),
	F( 100000000,          gpll0,    8,    0,     0),
	F( 133333333,          gpll0,    6,    0,     0),
	F( 160000000,          gpll0,    5,    0,     0),
	F( 177777778,          gpll0,  4.5,    0,     0),
	F( 200000000,          gpll0,    4,    0,     0),
	F( 266666667,          gpll0,    3,    0,     0),
	F( 300000000,      gpll4_out,    4,    0,     0),
	F( 320000000,          gpll0,  2.5,    0,     0),
	F( 400000000,          gpll0,    2,    0,     0),
	F( 466000000,      gpll2_aux,    2,    0,     0),
	F_END
};

static struct rcg_clk vfe0_clk_src = {
	.cmd_rcgr_reg = VFE0_CMD_RCGR,
	.set_rate = set_rate_hid,
	.freq_tbl = ftbl_vfe0_clk_src,
	.current_freq = &rcg_dummy_freq,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "vfe0_clk_src",
		.ops = &clk_ops_rcg,
		VDD_DIG_FMAX_MAP5(LOWER, 160000000, LOW, 300000000,
		NOMINAL, 320000000, NOM_PLUS, 400000000,
		HIGH, 466000000),
		CLK_INIT(vfe0_clk_src.c),
	},
};

static struct clk_freq_tbl ftbl_vfe1_clk_src[] = {
	F(  50000000,          gpll0,   16,    0,     0),
	F(  80000000,          gpll0,   10,    0,     0),
	F( 100000000,          gpll0,    8,    0,     0),
	F( 133333333,          gpll0,    6,    0,     0),
	F( 160000000,          gpll0,    5,    0,     0),
	F( 177777778,          gpll0,  4.5,    0,     0),
	F( 200000000,          gpll0,    4,    0,     0),
	F( 266666667,          gpll0,    3,    0,     0),
	F( 300000000,      gpll4_out,    4,    0,     0),
	F( 320000000,          gpll0,  2.5,    0,     0),
	F( 400000000,          gpll0,    2,    0,     0),
	F( 466000000,      gpll2_aux,    2,    0,     0),
	F_END
};

static struct rcg_clk vfe1_clk_src = {
	.cmd_rcgr_reg = VFE1_CMD_RCGR,
	.set_rate = set_rate_hid,
	.freq_tbl = ftbl_vfe1_clk_src,
	.current_freq = &rcg_dummy_freq,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "vfe1_clk_src",
		.ops = &clk_ops_rcg,
		VDD_DIG_FMAX_MAP5(LOWER, 160000000, LOW, 300000000,
		NOMINAL, 320000000, NOM_PLUS, 400000000,
		HIGH, 466000000),
		CLK_INIT(vfe1_clk_src.c),
	},
};

static struct clk_freq_tbl ftbl_crypto_clk_src[] = {
	F(  50000000,          gpll0,   16,    0,     0),
	F(  80000000,          gpll0,   10,    0,     0),
	F( 100000000,          gpll0,    8,    0,     0),
	F( 160000000,          gpll0,    5,    0,     0),
	F_END
};

static struct rcg_clk crypto_clk_src = {
	.cmd_rcgr_reg = CRYPTO_CMD_RCGR,
	.set_rate = set_rate_hid,
	.freq_tbl = ftbl_crypto_clk_src,
	.current_freq = &rcg_dummy_freq,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "crypto_clk_src",
		.ops = &clk_ops_rcg,
		VDD_DIG_FMAX_MAP2(LOWER, 80000000, NOMINAL, 160000000),
		CLK_INIT(crypto_clk_src.c),
	},
};

static struct clk_freq_tbl ftbl_gp1_clk_src[] = {
	F(  19200000,             xo,    1,    0,     0),
	F_END
};

static struct rcg_clk gp1_clk_src = {
	.cmd_rcgr_reg = GP1_CMD_RCGR,
	.set_rate = set_rate_mnd,
	.freq_tbl = ftbl_gp1_clk_src,
	.current_freq = &rcg_dummy_freq,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gp1_clk_src",
		.ops = &clk_ops_rcg_mnd,
		VDD_DIG_FMAX_MAP2(LOWER, 100000000, NOMINAL, 200000000),
		CLK_INIT(gp1_clk_src.c),
	},
};

static struct clk_freq_tbl ftbl_gp2_clk_src[] = {
	F(  19200000,             xo,    1,    0,     0),
	F_END
};

static struct rcg_clk gp2_clk_src = {
	.cmd_rcgr_reg = GP2_CMD_RCGR,
	.set_rate = set_rate_mnd,
	.freq_tbl = ftbl_gp2_clk_src,
	.current_freq = &rcg_dummy_freq,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gp2_clk_src",
		.ops = &clk_ops_rcg_mnd,
		VDD_DIG_FMAX_MAP2(LOWER, 100000000, NOMINAL, 200000000),
		CLK_INIT(gp2_clk_src.c),
	},
};

static struct clk_freq_tbl ftbl_gp3_clk_src[] = {
	F(  19200000,             xo,    1,    0,     0),
	F_END
};

static struct rcg_clk gp3_clk_src = {
	.cmd_rcgr_reg = GP3_CMD_RCGR,
	.set_rate = set_rate_mnd,
	.freq_tbl = ftbl_gp3_clk_src,
	.current_freq = &rcg_dummy_freq,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gp3_clk_src",
		.ops = &clk_ops_rcg_mnd,
		VDD_DIG_FMAX_MAP2(LOWER, 100000000, NOMINAL, 200000000),
		CLK_INIT(gp3_clk_src.c),
	},
};

DEFINE_EXT_CLK(ext_byte0_clk_src, NULL);
DEFINE_EXT_CLK(ext_byte1_clk_src, NULL);
static struct clk_freq_tbl ftbl_byte0_clk_src[] = {
	{
		.div_src_val = BVAL(10, 8, dsi0_phypll_mm_source_val),
		.src_clk = &ext_byte0_clk_src.c,
		.freq_hz = 0,
	},
	F_END
};

static struct rcg_clk byte0_clk_src = {
	.cmd_rcgr_reg = BYTE0_CMD_RCGR,
	.set_rate = set_rate_hid,
	.current_freq = ftbl_byte0_clk_src,
	.freq_tbl = ftbl_byte0_clk_src,
	.base = &virt_bases[MDSS_BASE],
	.c = {
		.dbg_name = "byte0_clk_src",
		.ops = &clk_ops_byte_multiparent,
		.flags = CLKFLAG_NO_RATE_CACHE,
		VDD_DIG_FMAX_MAP3(LOWER, 125000000, LOW, 161250000,
				NOMINAL, 187500000),
		CLK_INIT(byte0_clk_src.c),
	},
};

static struct clk_freq_tbl ftbl_byte1_clk_src[] = {
	{
		.div_src_val = BVAL(10, 8, dsi1_phypll_mm_source_val)
				| BVAL(4, 0, 0),
		.src_clk = &ext_byte1_clk_src.c,
		.freq_hz = 0,
	},
	F_END
};

static struct clk_freq_tbl ftbl_byte1_v1_clk_src[] = {
	{
		.div_src_val = BVAL(10, 8, xo_source_val)
				| BVAL(4, 0, 0),
		.src_clk = &xo_clk_src.c,
		.freq_hz = 0,
	},
	{
		.div_src_val = BVAL(10, 8, dsi1_phypll_mm_source_val)
				| BVAL(4, 0, 0),
		.src_clk = &ext_byte1_clk_src.c,
		.freq_hz = 0,
	},
	{
		.div_src_val = BVAL(10, 8, dsi0_phypll_clk_mm_source_val),
		.src_clk = &ext_byte0_clk_src.c,
		.freq_hz = 0,
	},
	F_END
};

static struct rcg_clk byte1_clk_src = {
	.cmd_rcgr_reg = BYTE1_CMD_RCGR,
	.set_rate = set_rate_hid,
	.current_freq = ftbl_byte1_clk_src,
	.freq_tbl = ftbl_byte1_clk_src,
	.base = &virt_bases[MDSS_BASE],
	.c = {
		.dbg_name = "byte1_clk_src",
		.ops = &clk_ops_byte_multiparent,
		.flags = CLKFLAG_NO_RATE_CACHE,
		VDD_DIG_FMAX_MAP3(LOWER, 125000000, LOW, 161250000,
				NOMINAL, 187500000),
		CLK_INIT(byte1_clk_src.c),
	},
};

static struct clk_freq_tbl ftbl_esc0_clk_src[] = {
	F(  19200000,             xo,    1,    0,     0),
	F_END
};

static struct rcg_clk esc0_clk_src = {
	.cmd_rcgr_reg = ESC0_CMD_RCGR,
	.set_rate = set_rate_hid,
	.freq_tbl = ftbl_esc0_clk_src,
	.current_freq = &rcg_dummy_freq,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "esc0_clk_src",
		.ops = &clk_ops_rcg,
		VDD_DIG_FMAX_MAP1(LOWER, 19200000),
		CLK_INIT(esc0_clk_src.c),
	},
};

static struct clk_freq_tbl ftbl_esc1_clk_src[] = {
	F(  19200000,             xo,    1,    0,     0),
	F_END
};

static struct rcg_clk esc1_clk_src = {
	.cmd_rcgr_reg = ESC1_CMD_RCGR,
	.set_rate = set_rate_hid,
	.freq_tbl = ftbl_esc1_clk_src,
	.current_freq = &rcg_dummy_freq,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "esc1_clk_src",
		.ops = &clk_ops_rcg,
		VDD_DIG_FMAX_MAP1(LOWER, 19200000),
		CLK_INIT(esc1_clk_src.c),
	},
};

static struct clk_freq_tbl ftbl_mdp_clk_src[] = {
	F(  50000000,  gpll0_out_mdp,   16,    0,     0),
	F(  80000000,  gpll0_out_mdp,   10,    0,     0),
	F( 100000000,  gpll0_out_mdp,    8,    0,     0),
	F( 145454545,  gpll0_out_mdp,  5.5,    0,     0),
	F( 160000000,  gpll0_out_mdp,    5,    0,     0),
	F( 177777778,  gpll0_out_mdp,  4.5,    0,     0),
	F( 200000000,  gpll0_out_mdp,    4,    0,     0),
	F( 270000000,      gpll6_out,    4,    0,     0),
	F( 320000000,  gpll0_out_mdp,  2.5,    0,     0),
	F( 360000000,      gpll6_out,    3,    0,     0),
	F_END
};

static struct rcg_clk mdp_clk_src = {
	.cmd_rcgr_reg = MDP_CMD_RCGR,
	.set_rate = set_rate_hid,
	.freq_tbl = ftbl_mdp_clk_src,
	.current_freq = &rcg_dummy_freq,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "mdp_clk_src",
		.ops = &clk_ops_rcg,
		VDD_DIG_FMAX_MAP4(LOWER, 177780000, LOW, 270000000,
		NOMINAL, 320000000, HIGH, 360000000),
		CLK_INIT(mdp_clk_src.c),
	},
};

DEFINE_EXT_CLK(ext_pclk0_clk_src, NULL);
DEFINE_EXT_CLK(ext_pclk1_clk_src, NULL);
static struct clk_freq_tbl ftbl_pclk0_clk_src[] = {
	{
		.div_src_val = BVAL(10, 8, dsi0_phypll_mm_source_val)
				| BVAL(4, 0, 0),
		.src_clk = &ext_pclk0_clk_src.c,
		.freq_hz = 0,
	},
	F_END
};

static struct rcg_clk pclk0_clk_src = {
	.cmd_rcgr_reg = PCLK0_CMD_RCGR,
	.set_rate = set_rate_mnd,
	.current_freq = ftbl_pclk0_clk_src,
	.freq_tbl = ftbl_pclk0_clk_src,
	.base = &virt_bases[MDSS_BASE],
	.c = {
		.dbg_name = "pclk0_clk_src",
		.ops = &clk_ops_pixel_multiparent,
		.flags = CLKFLAG_NO_RATE_CACHE,
		VDD_DIG_FMAX_MAP3(LOWER, 166670000, LOW, 215000000,
				NOMINAL, 250000000),
		CLK_INIT(pclk0_clk_src.c),
	},
};

static struct clk_freq_tbl ftbl_pclk1_clk_src[] = {
	{
		.div_src_val = BVAL(10, 8, dsi1_phypll_mm_source_val)
				| BVAL(4, 0, 0),
		.src_clk = &ext_pclk1_clk_src.c,
		.freq_hz = 0,
	},
	F_END
};

static struct clk_freq_tbl ftbl_pclk1_v1_clk_src[] = {
	{
		.div_src_val = BVAL(10, 8, xo_source_val)
				| BVAL(4, 0, 0),
		.src_clk = &xo_clk_src.c,
		.freq_hz = 0,
	},
	{
		.div_src_val = BVAL(10, 8, dsi1_phypll_mm_source_val)
				| BVAL(4, 0, 0),
		.src_clk = &ext_pclk1_clk_src.c,
		.freq_hz = 0,
	},
	{
		.div_src_val = BVAL(10, 8, dsi0_phypll_clk_mm_source_val)
				| BVAL(4, 0, 0),
		.src_clk = &ext_pclk0_clk_src.c,
		.freq_hz = 0,
	},
	F_END
};

static struct rcg_clk pclk1_clk_src = {
	.cmd_rcgr_reg = PCLK1_CMD_RCGR,
	.set_rate = set_rate_mnd,
	.current_freq = ftbl_pclk1_clk_src,
	.freq_tbl = ftbl_pclk1_clk_src,
	.base = &virt_bases[MDSS_BASE],
	.c = {
		.dbg_name = "pclk1_clk_src",
		.ops = &clk_ops_pixel_multiparent,
		.flags = CLKFLAG_NO_RATE_CACHE,
		VDD_DIG_FMAX_MAP3(LOWER, 166670000, LOW, 215000000,
				NOMINAL, 250000000),
		CLK_INIT(pclk1_clk_src.c),
	},
};

static struct clk_freq_tbl ftbl_vsync_clk_src[] = {
	F(  19200000,             xo,    1,    0,     0),
	F_END
};

static struct rcg_clk vsync_clk_src = {
	.cmd_rcgr_reg = VSYNC_CMD_RCGR,
	.set_rate = set_rate_hid,
	.freq_tbl = ftbl_vsync_clk_src,
	.current_freq = &rcg_dummy_freq,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "vsync_clk_src",
		.ops = &clk_ops_rcg,
		VDD_DIG_FMAX_MAP1(LOWER, 19200000),
		CLK_INIT(vsync_clk_src.c),
	},
};

static struct clk_freq_tbl ftbl_gfx3d_clk_src[] = {
	F(  19200000,             xo,    1,    0,     0),
	F(  50000000,          gpll0,   16,    0,     0),
	F(  80000000,          gpll0,   10,    0,     0),
	F( 100000000,          gpll0,    8,    0,     0),
	F( 133333333,          gpll0,    6,    0,     0),
	F( 160000000,          gpll0,    5,    0,     0),
	F( 200000000,          gpll0,    4,    0,     0),
	F( 228571429,          gpll0,  3.5,    0,     0),
	F( 240000000,    gpll6_gfx3d,  4.5,    0,     0),
	F( 266666667,          gpll0,    3,    0,     0),
	F( 300000000,    gpll4_gfx3d,    4,    0,     0),
	F( 366670000,          gpll3,    3,    0,     0),
	F( 400000000,          gpll0,    2,    0,     0),
	F( 432000000,    gpll6_gfx3d,  2.5,    0,     0),
	F( 480000000,    gpll4_gfx3d,  2.5,    0,     0),
	F( 550000000,          gpll3,    2,    0,     0),
	F( 600000000,    gpll4_gfx3d,    2,    0,     0),
	F_END
};

static struct rcg_clk gfx3d_clk_src = {
	.cmd_rcgr_reg = GFX3D_CMD_RCGR,
	.set_rate = set_rate_hid,
	.freq_tbl = ftbl_gfx3d_clk_src,
	.current_freq = &rcg_dummy_freq,
	.base = &virt_bases[GFX_BASE],
	.c = {
		.dbg_name = "gfx3d_clk_src",
		.ops = &clk_ops_rcg,
		.vdd_class = &vdd_gfx,
		CLK_INIT(gfx3d_clk_src.c),
	},
};

static struct clk_freq_tbl ftbl_pdm2_clk_src[] = {
	F(  64000000,          gpll0, 12.5,    0,     0),
	F_END
};

static struct rcg_clk pdm2_clk_src = {
	.cmd_rcgr_reg = PDM2_CMD_RCGR,
	.set_rate = set_rate_hid,
	.freq_tbl = ftbl_pdm2_clk_src,
	.current_freq = &rcg_dummy_freq,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "pdm2_clk_src",
		.ops = &clk_ops_rcg,
		VDD_DIG_FMAX_MAP1(LOWER, 64000000),
		CLK_INIT(pdm2_clk_src.c),
	},
};

static struct clk_freq_tbl ftbl_rbcpr_gfx_clk_src[] = {
	F(  19200000,             xo,    1,    0,     0),
	F(  50000000,          gpll0,   16,    0,     0),
	F_END
};

static struct rcg_clk rbcpr_gfx_clk_src = {
	.cmd_rcgr_reg = RBCPR_GFX_CMD_RCGR,
	.set_rate = set_rate_hid,
	.freq_tbl = ftbl_rbcpr_gfx_clk_src,
	.current_freq = &rcg_dummy_freq,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "rbcpr_gfx_clk_src",
		.ops = &clk_ops_rcg,
		VDD_DIG_FMAX_MAP2(LOWER, 50000000, NOMINAL, 100000000),
		CLK_INIT(rbcpr_gfx_clk_src.c),
	},
};

static struct clk_freq_tbl ftbl_sdcc1_apps_clk_src[] = {
	F(    144000,             xo,   16,    3,    25),
	F(    400000,             xo,   12,    1,     4),
	F(  20000000,          gpll0,   10,    1,     4),
	F(  25000000,          gpll0,   16,    1,     2),
	F(  50000000,          gpll0,   16,    0,     0),
	F( 100000000,          gpll0,    8,    0,     0),
	F( 177777778,          gpll0,  4.5,    0,     0),
	F( 200000000,          gpll0,    4,    0,     0),
	F( 342850000,      gpll4_out,  3.5,    0,     0),
	F( 400000000,      gpll4_out,    3,    0,     0),
	F_END
};

static struct clk_freq_tbl ftbl_sdcc1_v1_apps_clk_src[] = {
	F(    144000,             xo,   16,    3,    25),
	F(    400000,             xo,   12,    1,     4),
	F(  20000000,          gpll0,   10,    1,     4),
	F(  25000000,          gpll0,   16,    1,     2),
	F(  50000000,          gpll0,   16,    0,     0),
	F( 100000000,          gpll0,    8,    0,     0),
	F( 177777778,          gpll0,  4.5,    0,     0),
	F( 200000000,          gpll0,    4,    0,     0),
	F( 186400000,      gpll2_out,    5,    0,     0),
	F( 372800000,      gpll2_out,  2.5,    0,     0),
	F_END
};

static struct rcg_clk sdcc1_apps_clk_src = {
	.cmd_rcgr_reg = SDCC1_APPS_CMD_RCGR,
	.set_rate = set_rate_mnd,
	.freq_tbl = ftbl_sdcc1_apps_clk_src,
	.current_freq = &rcg_dummy_freq,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "sdcc1_apps_clk_src",
		.ops = &clk_ops_rcg_mnd,
		VDD_DIG_FMAX_MAP2(LOWER, 100000000, NOMINAL, 400000000),
		CLK_INIT(sdcc1_apps_clk_src.c),
	},
};

static struct clk_freq_tbl ftbl_sdcc1_ice_core_clk_src[] = {
	F( 100000000,    gpll0_out_m,    8,    0,     0),
	F( 200000000,    gpll0_out_m,    4,    0,     0),
	F_END
};

static struct rcg_clk sdcc1_ice_core_clk_src = {
	.cmd_rcgr_reg = SDCC1_ICE_CORE_CMD_RCGR,
	.set_rate = set_rate_mnd,
	.freq_tbl = ftbl_sdcc1_ice_core_clk_src,
	.current_freq = &rcg_dummy_freq,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "sdcc1_ice_core_clk_src",
		.ops = &clk_ops_rcg_mnd,
		VDD_DIG_FMAX_MAP2(LOWER, 100000000, NOMINAL, 200000000),
		CLK_INIT(sdcc1_ice_core_clk_src.c),
	},
};

static struct clk_freq_tbl ftbl_sdcc2_4_apps_clk_src[] = {
	F(    144000,             xo,   16,    3,    25),
	F(    400000,             xo,   12,    1,     4),
	F(  20000000,          gpll0,   10,    1,     4),
	F(  25000000,          gpll0,   16,    1,     2),
	F(  40000000,          gpll0,   10,    1,     2),
	F(  50000000,          gpll0,   16,    0,     0),
	F(  80000000,          gpll0,   10,    0,     0),
	F( 100000000,          gpll0,    8,    0,     0),
	F( 177777778,          gpll0,  4.5,    0,     0),
	F( 200000000,          gpll0,    4,    0,     0),
	F_END
};

static struct rcg_clk sdcc2_apps_clk_src = {
	.cmd_rcgr_reg = SDCC2_APPS_CMD_RCGR,
	.set_rate = set_rate_mnd,
	.freq_tbl = ftbl_sdcc2_4_apps_clk_src,
	.current_freq = &rcg_dummy_freq,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "sdcc2_apps_clk_src",
		.ops = &clk_ops_rcg_mnd,
		VDD_DIG_FMAX_MAP2(LOWER, 50000000, NOMINAL, 200000000),
		CLK_INIT(sdcc2_apps_clk_src.c),
	},
};

static struct rcg_clk sdcc3_apps_clk_src = {
	.cmd_rcgr_reg = SDCC3_APPS_CMD_RCGR,
	.set_rate = set_rate_mnd,
	.freq_tbl = ftbl_sdcc2_4_apps_clk_src,
	.current_freq = &rcg_dummy_freq,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "sdcc3_apps_clk_src",
		.ops = &clk_ops_rcg_mnd,
		VDD_DIG_FMAX_MAP2(LOWER, 50000000, NOMINAL, 200000000),
		CLK_INIT(sdcc3_apps_clk_src.c),
	},
};

static struct clk_freq_tbl ftbl_usb_fs_ic_clk_src[] = {
	F(  60000000,          gpll6,    6,    1,     3),
	F_END
};

static struct rcg_clk usb_fs_ic_clk_src = {
	.cmd_rcgr_reg = USB_FS_IC_CMD_RCGR,
	.set_rate = set_rate_mnd,
	.freq_tbl = ftbl_usb_fs_ic_clk_src,
	.current_freq = &rcg_dummy_freq,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "usb_fs_ic_clk_src",
		.ops = &clk_ops_rcg_mnd,
		VDD_DIG_FMAX_MAP1(LOWER, 60000000),
		CLK_INIT(usb_fs_ic_clk_src.c),
	},
};

static struct clk_freq_tbl ftbl_usb_fs_system_clk_src[] = {
	F(  64000000,      gpll0_out, 12.5,    0,     0),
	F_END
};

static struct rcg_clk usb_fs_system_clk_src = {
	.cmd_rcgr_reg = USB_FS_SYSTEM_CMD_RCGR,
	.set_rate = set_rate_mnd,
	.freq_tbl = ftbl_usb_fs_system_clk_src,
	.current_freq = &rcg_dummy_freq,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "usb_fs_system_clk_src",
		.ops = &clk_ops_rcg_mnd,
		VDD_DIG_FMAX_MAP1(LOWER, 64000000),
		CLK_INIT(usb_fs_system_clk_src.c),
	},
};

static struct clk_freq_tbl ftbl_usb_hs_system_clk_src[] = {
	F(  57140000,          gpll0,   14,    0,     0),
	F( 100000000,          gpll0,    8,    0,     0),
	F( 133333333,          gpll0,    6,    0,     0),
	F( 177780000,          gpll0,  4.5,    0,     0),
	F_END
};

static struct rcg_clk usb_hs_system_clk_src = {
	.cmd_rcgr_reg = USB_HS_SYSTEM_CMD_RCGR,
	.set_rate = set_rate_hid,
	.freq_tbl = ftbl_usb_hs_system_clk_src,
	.current_freq = &rcg_dummy_freq,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "usb_hs_system_clk_src",
		.ops = &clk_ops_rcg,
		VDD_DIG_FMAX_MAP3(LOWER, 57140000, NOMINAL, 133330000,
		HIGH, 177780000),
		CLK_INIT(usb_hs_system_clk_src.c),
	},
};

static struct clk_freq_tbl ftbl_vcodec0_clk_src[] = {
	F(  72727200,          gpll0,   11,    0,     0),
	F(  80000000,          gpll0,   10,    0,     0),
	F( 100000000,          gpll0,    8,    0,     0),
	F( 133333333,          gpll0,    6,    0,     0),
	F( 228570000,          gpll0,  3.5,    0,     0),
	F( 310667000,      gpll2_aux,    3,    0,     0),
	F( 360000000,      gpll6_aux,    3,    0,     0),
	F( 400000000,          gpll0,    2,    0,     0),
	F( 466000000,      gpll2_aux,    2,    0,     0),
	F_END
};

static struct rcg_clk vcodec0_clk_src = {
	.cmd_rcgr_reg = VCODEC0_CMD_RCGR,
	.set_rate = set_rate_mnd,
	.freq_tbl = ftbl_vcodec0_clk_src,
	.current_freq = &rcg_dummy_freq,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "vcodec0_clk_src",
		.ops = &clk_ops_rcg_mnd,
		VDD_DIG_FMAX_MAP5(LOWER, 228570000, LOW, 310667000,
		NOMINAL, 360000000, NOM_PLUS, 400000000,
		HIGH, 466000000),
		CLK_INIT(vcodec0_clk_src.c),
	},
};
static struct branch_clk gcc_aps_0_clk = {
	.cbcr_reg = APS_0_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_aps_0_clk",
		.parent = &aps_0_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_aps_0_clk.c),
	},
};

static struct branch_clk gcc_aps_1_clk = {
	.cbcr_reg = APS_1_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_aps_1_clk",
		.parent = &aps_1_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_aps_1_clk.c),
	},
};

static struct branch_clk gcc_blsp1_qup1_i2c_apps_clk = {
	.cbcr_reg = BLSP1_QUP1_I2C_APPS_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_blsp1_qup1_i2c_apps_clk",
		.parent = &blsp1_qup1_i2c_apps_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_blsp1_qup1_i2c_apps_clk.c),
	},
};

static struct branch_clk gcc_blsp1_qup1_spi_apps_clk = {
	.cbcr_reg = BLSP1_QUP1_SPI_APPS_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_blsp1_qup1_spi_apps_clk",
		.parent = &blsp1_qup1_spi_apps_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_blsp1_qup1_spi_apps_clk.c),
	},
};

static struct branch_clk gcc_blsp1_qup2_i2c_apps_clk = {
	.cbcr_reg = BLSP1_QUP2_I2C_APPS_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_blsp1_qup2_i2c_apps_clk",
		.parent = &blsp1_qup2_i2c_apps_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_blsp1_qup2_i2c_apps_clk.c),
	},
};

static struct branch_clk gcc_blsp1_qup2_spi_apps_clk = {
	.cbcr_reg = BLSP1_QUP2_SPI_APPS_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_blsp1_qup2_spi_apps_clk",
		.parent = &blsp1_qup2_spi_apps_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_blsp1_qup2_spi_apps_clk.c),
	},
};

static struct branch_clk gcc_blsp1_qup3_i2c_apps_clk = {
	.cbcr_reg = BLSP1_QUP3_I2C_APPS_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_blsp1_qup3_i2c_apps_clk",
		.parent = &blsp1_qup3_i2c_apps_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_blsp1_qup3_i2c_apps_clk.c),
	},
};

static struct branch_clk gcc_blsp1_qup3_spi_apps_clk = {
	.cbcr_reg = BLSP1_QUP3_SPI_APPS_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_blsp1_qup3_spi_apps_clk",
		.parent = &blsp1_qup3_spi_apps_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_blsp1_qup3_spi_apps_clk.c),
	},
};

static struct branch_clk gcc_blsp1_qup4_i2c_apps_clk = {
	.cbcr_reg = BLSP1_QUP4_I2C_APPS_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_blsp1_qup4_i2c_apps_clk",
		.parent = &blsp1_qup4_i2c_apps_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_blsp1_qup4_i2c_apps_clk.c),
	},
};

static struct branch_clk gcc_blsp1_qup4_spi_apps_clk = {
	.cbcr_reg = BLSP1_QUP4_SPI_APPS_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_blsp1_qup4_spi_apps_clk",
		.parent = &blsp1_qup4_spi_apps_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_blsp1_qup4_spi_apps_clk.c),
	},
};

static struct branch_clk gcc_blsp1_uart1_apps_clk = {
	.cbcr_reg = BLSP1_UART1_APPS_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_blsp1_uart1_apps_clk",
		.parent = &blsp1_uart1_apps_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_blsp1_uart1_apps_clk.c),
	},
};

static struct branch_clk gcc_blsp1_uart2_apps_clk = {
	.cbcr_reg = BLSP1_UART2_APPS_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_blsp1_uart2_apps_clk",
		.parent = &blsp1_uart2_apps_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_blsp1_uart2_apps_clk.c),
	},
};

static struct branch_clk gcc_blsp2_qup1_i2c_apps_clk = {
	.cbcr_reg = BLSP2_QUP1_I2C_APPS_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_blsp2_qup1_i2c_apps_clk",
		.parent = &blsp2_qup1_i2c_apps_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_blsp2_qup1_i2c_apps_clk.c),
	},
};

static struct branch_clk gcc_blsp2_qup1_spi_apps_clk = {
	.cbcr_reg = BLSP2_QUP1_SPI_APPS_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_blsp2_qup1_spi_apps_clk",
		.parent = &blsp2_qup1_spi_apps_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_blsp2_qup1_spi_apps_clk.c),
	},
};

static struct branch_clk gcc_blsp2_qup2_i2c_apps_clk = {
	.cbcr_reg = BLSP2_QUP2_I2C_APPS_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_blsp2_qup2_i2c_apps_clk",
		.parent = &blsp2_qup2_i2c_apps_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_blsp2_qup2_i2c_apps_clk.c),
	},
};

static struct branch_clk gcc_blsp2_qup2_spi_apps_clk = {
	.cbcr_reg = BLSP2_QUP2_SPI_APPS_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_blsp2_qup2_spi_apps_clk",
		.parent = &blsp2_qup2_spi_apps_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_blsp2_qup2_spi_apps_clk.c),
	},
};

static struct branch_clk gcc_blsp2_qup3_i2c_apps_clk = {
	.cbcr_reg = BLSP2_QUP3_I2C_APPS_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_blsp2_qup3_i2c_apps_clk",
		.parent = &blsp2_qup3_i2c_apps_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_blsp2_qup3_i2c_apps_clk.c),
	},
};

static struct branch_clk gcc_blsp2_qup3_spi_apps_clk = {
	.cbcr_reg = BLSP2_QUP3_SPI_APPS_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_blsp2_qup3_spi_apps_clk",
		.parent = &blsp2_qup3_spi_apps_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_blsp2_qup3_spi_apps_clk.c),
	},
};

static struct branch_clk gcc_blsp2_qup4_i2c_apps_clk = {
	.cbcr_reg = BLSP2_QUP4_I2C_APPS_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_blsp2_qup4_i2c_apps_clk",
		.parent = &blsp2_qup4_i2c_apps_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_blsp2_qup4_i2c_apps_clk.c),
	},
};

static struct branch_clk gcc_blsp2_qup4_spi_apps_clk = {
	.cbcr_reg = BLSP2_QUP4_SPI_APPS_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_blsp2_qup4_spi_apps_clk",
		.parent = &blsp2_qup4_spi_apps_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_blsp2_qup4_spi_apps_clk.c),
	},
};

static struct branch_clk gcc_blsp2_uart1_apps_clk = {
	.cbcr_reg = BLSP2_UART1_APPS_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_blsp2_uart1_apps_clk",
		.parent = &blsp2_uart1_apps_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_blsp2_uart1_apps_clk.c),
	},
};

static struct branch_clk gcc_blsp2_uart2_apps_clk = {
	.cbcr_reg = BLSP2_UART2_APPS_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_blsp2_uart2_apps_clk",
		.parent = &blsp2_uart2_apps_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_blsp2_uart2_apps_clk.c),
	},
};

static struct branch_clk gcc_camss_cci_ahb_clk = {
	.cbcr_reg = CAMSS_CCI_AHB_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_camss_cci_ahb_clk",
		.parent = &camss_top_ahb_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_camss_cci_ahb_clk.c),
	},
};

static struct branch_clk gcc_camss_cci_clk = {
	.cbcr_reg = CAMSS_CCI_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_camss_cci_clk",
		.parent = &cci_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_camss_cci_clk.c),
	},
};

static struct branch_clk gcc_camss_cpp_ahb_clk = {
	.cbcr_reg = CAMSS_CPP_AHB_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_camss_cpp_ahb_clk",
		.parent = &camss_top_ahb_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_camss_cpp_ahb_clk.c),
	},
};

static struct branch_clk gcc_camss_cpp_axi_clk = {
	.cbcr_reg = CAMSS_CPP_AXI_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_camss_cpp_axi_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_camss_cpp_axi_clk.c),
	},
};

static struct branch_clk gcc_camss_cpp_clk = {
	.cbcr_reg = CAMSS_CPP_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_camss_cpp_clk",
		.parent = &cpp_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_camss_cpp_clk.c),
	},
};

static struct branch_clk gcc_camss_csi0_ahb_clk = {
	.cbcr_reg = CAMSS_CSI0_AHB_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_camss_csi0_ahb_clk",
		.parent = &camss_top_ahb_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_camss_csi0_ahb_clk.c),
	},
};

static struct branch_clk gcc_camss_csi0_clk = {
	.cbcr_reg = CAMSS_CSI0_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_camss_csi0_clk",
		.parent = &csi0_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_camss_csi0_clk.c),
	},
};

static struct branch_clk gcc_camss_csi0phy_clk = {
	.cbcr_reg = CAMSS_CSI0PHY_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_camss_csi0phy_clk",
		.parent = &csi0_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_camss_csi0phy_clk.c),
	},
};

static struct branch_clk gcc_camss_csi0pix_clk = {
	.cbcr_reg = CAMSS_CSI0PIX_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_camss_csi0pix_clk",
		.parent = &csi0_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_camss_csi0pix_clk.c),
	},
};

static struct branch_clk gcc_camss_csi0rdi_clk = {
	.cbcr_reg = CAMSS_CSI0RDI_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_camss_csi0rdi_clk",
		.parent = &csi0_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_camss_csi0rdi_clk.c),
	},
};

static struct branch_clk gcc_camss_csi1_ahb_clk = {
	.cbcr_reg = CAMSS_CSI1_AHB_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_camss_csi1_ahb_clk",
		.parent = &camss_top_ahb_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_camss_csi1_ahb_clk.c),
	},
};

static struct branch_clk gcc_camss_csi1_clk = {
	.cbcr_reg = CAMSS_CSI1_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_camss_csi1_clk",
		.parent = &csi1_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_camss_csi1_clk.c),
	},
};

static struct branch_clk gcc_camss_csi1phy_clk = {
	.cbcr_reg = CAMSS_CSI1PHY_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_camss_csi1phy_clk",
		.parent = &csi1_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_camss_csi1phy_clk.c),
	},
};

static struct branch_clk gcc_camss_csi1pix_clk = {
	.cbcr_reg = CAMSS_CSI1PIX_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_camss_csi1pix_clk",
		.parent = &csi1_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_camss_csi1pix_clk.c),
	},
};

static struct branch_clk gcc_camss_csi1rdi_clk = {
	.cbcr_reg = CAMSS_CSI1RDI_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_camss_csi1rdi_clk",
		.parent = &csi1_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_camss_csi1rdi_clk.c),
	},
};

static struct branch_clk gcc_camss_csi2_ahb_clk = {
	.cbcr_reg = CAMSS_CSI2_AHB_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_camss_csi2_ahb_clk",
		.parent = &camss_top_ahb_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_camss_csi2_ahb_clk.c),
	},
};

static struct branch_clk gcc_camss_csi2_clk = {
	.cbcr_reg = CAMSS_CSI2_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_camss_csi2_clk",
		.parent = &csi2_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_camss_csi2_clk.c),
	},
};

static struct branch_clk gcc_camss_csi2phy_clk = {
	.cbcr_reg = CAMSS_CSI2PHY_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_camss_csi2phy_clk",
		.parent = &csi2_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_camss_csi2phy_clk.c),
	},
};

static struct branch_clk gcc_camss_csi2pix_clk = {
	.cbcr_reg = CAMSS_CSI2PIX_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_camss_csi2pix_clk",
		.parent = &csi2_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_camss_csi2pix_clk.c),
	},
};

static struct branch_clk gcc_camss_csi2rdi_clk = {
	.cbcr_reg = CAMSS_CSI2RDI_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_camss_csi2rdi_clk",
		.parent = &csi2_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_camss_csi2rdi_clk.c),
	},
};

static struct branch_clk gcc_camss_csi_vfe0_clk = {
	.cbcr_reg = CAMSS_CSI_VFE0_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_camss_csi_vfe0_clk",
		.parent = &vfe0_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_camss_csi_vfe0_clk.c),
	},
};

static struct branch_clk gcc_camss_csi_vfe1_clk = {
	.cbcr_reg = CAMSS_CSI_VFE1_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_camss_csi_vfe1_clk",
		.parent = &vfe1_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_camss_csi_vfe1_clk.c),
	},
};

static struct branch_clk gcc_camss_gp0_clk = {
	.cbcr_reg = CAMSS_GP0_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_camss_gp0_clk",
		.parent = &camss_gp0_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_camss_gp0_clk.c),
	},
};

static struct branch_clk gcc_camss_gp1_clk = {
	.cbcr_reg = CAMSS_GP1_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_camss_gp1_clk",
		.parent = &camss_gp1_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_camss_gp1_clk.c),
	},
};

static struct branch_clk gcc_camss_ispif_ahb_clk = {
	.cbcr_reg = CAMSS_ISPIF_AHB_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_camss_ispif_ahb_clk",
		.parent = &camss_top_ahb_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_camss_ispif_ahb_clk.c),
	},
};

static struct branch_clk gcc_camss_jpeg0_clk = {
	.cbcr_reg = CAMSS_JPEG0_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_camss_jpeg0_clk",
		.parent = &jpeg0_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_camss_jpeg0_clk.c),
	},
};

static struct branch_clk gcc_camss_jpeg_ahb_clk = {
	.cbcr_reg = CAMSS_JPEG_AHB_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_camss_jpeg_ahb_clk",
		.parent = &camss_top_ahb_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_camss_jpeg_ahb_clk.c),
	},
};

static struct branch_clk gcc_camss_jpeg_axi_clk = {
	.cbcr_reg = CAMSS_JPEG_AXI_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_camss_jpeg_axi_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_camss_jpeg_axi_clk.c),
	},
};


static struct branch_clk gcc_camss_mclk0_clk = {
	.cbcr_reg = CAMSS_MCLK0_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_camss_mclk0_clk",
		.parent = &mclk0_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_camss_mclk0_clk.c),
	},
};

static struct branch_clk gcc_camss_mclk1_clk = {
	.cbcr_reg = CAMSS_MCLK1_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_camss_mclk1_clk",
		.parent = &mclk1_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_camss_mclk1_clk.c),
	},
};

static struct branch_clk gcc_camss_mclk2_clk = {
	.cbcr_reg = CAMSS_MCLK2_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_camss_mclk2_clk",
		.parent = &mclk2_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_camss_mclk2_clk.c),
	},
};

static struct branch_clk gcc_camss_micro_ahb_clk = {
	.cbcr_reg = CAMSS_MICRO_AHB_CBCR,
	.bcr_reg = CAMSS_MICRO_BCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_camss_micro_ahb_clk",
		.parent = &camss_top_ahb_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_camss_micro_ahb_clk.c),
	},
};

static struct branch_clk gcc_camss_csi0phytimer_clk = {
	.cbcr_reg = CAMSS_CSI0PHYTIMER_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_camss_csi0phytimer_clk",
		.parent = &csi0phytimer_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_camss_csi0phytimer_clk.c),
	},
};

static struct branch_clk gcc_camss_csi1phytimer_clk = {
	.cbcr_reg = CAMSS_CSI1PHYTIMER_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_camss_csi1phytimer_clk",
		.parent = &csi1phytimer_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_camss_csi1phytimer_clk.c),
	},
};

static struct branch_clk gcc_camss_ahb_clk = {
	.cbcr_reg = CAMSS_AHB_CBCR,
	.has_sibling = 1,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_camss_ahb_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_camss_ahb_clk.c),
	},
};

static struct branch_clk gcc_camss_top_ahb_clk = {
	.cbcr_reg = CAMSS_TOP_AHB_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_camss_top_ahb_clk",
		.parent = &camss_top_ahb_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_camss_top_ahb_clk.c),
	},
};

static struct branch_clk gcc_camss_vfe0_clk = {
	.cbcr_reg = CAMSS_VFE0_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_camss_vfe0_clk",
		.parent = &vfe0_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_camss_vfe0_clk.c),
	},
};

static struct branch_clk gcc_camss_vfe_ahb_clk = {
	.cbcr_reg = CAMSS_VFE_AHB_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_camss_vfe_ahb_clk",
		.parent = &camss_top_ahb_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_camss_vfe_ahb_clk.c),
	},
};

static struct branch_clk gcc_camss_vfe_axi_clk = {
	.cbcr_reg = CAMSS_VFE_AXI_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_camss_vfe_axi_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_camss_vfe_axi_clk.c),
	},
};


static struct branch_clk gcc_camss_vfe1_ahb_clk = {
	.cbcr_reg = CAMSS_VFE1_AHB_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_camss_vfe1_ahb_clk",
		.parent = &camss_top_ahb_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_camss_vfe1_ahb_clk.c),
	},
};

static struct branch_clk gcc_camss_vfe1_axi_clk = {
	.cbcr_reg = CAMSS_VFE1_AXI_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_camss_vfe1_axi_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_camss_vfe1_axi_clk.c),
	},
};

static struct branch_clk gcc_camss_vfe1_clk = {
	.cbcr_reg = CAMSS_VFE1_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_camss_vfe1_clk",
		.parent = &vfe1_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_camss_vfe1_clk.c),
	},
};

static struct branch_clk gcc_dcc_clk = {
	.cbcr_reg = DCC_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_dcc_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_dcc_clk.c),
	},
};

static struct gate_clk gcc_oxili_gmem_clk = {
	.en_reg = OXILI_GMEM_CBCR,
	.en_mask = BIT(0),
	.delay_us = 50,
	.base = &virt_bases[GFX_BASE],
	.c = {
		.dbg_name = "gcc_oxili_gmem_clk",
		.parent = &gfx3d_clk_src.c,
		.ops = &clk_ops_gate,
		CLK_INIT(gcc_oxili_gmem_clk.c),
	},
};

static struct branch_clk gcc_gp1_clk = {
	.cbcr_reg = GP1_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_gp1_clk",
		.parent = &gp1_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_gp1_clk.c),
	},
};

static struct branch_clk gcc_gp2_clk = {
	.cbcr_reg = GP2_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_gp2_clk",
		.parent = &gp2_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_gp2_clk.c),
	},
};

static struct branch_clk gcc_gp3_clk = {
	.cbcr_reg = GP3_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_gp3_clk",
		.parent = &gp3_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_gp3_clk.c),
	},
};

static struct branch_clk gcc_mdss_ahb_clk = {
	.cbcr_reg = MDSS_AHB_CBCR,
	.has_sibling = 1,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_mdss_ahb_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_mdss_ahb_clk.c),
	},
};

static struct branch_clk gcc_mdss_axi_clk = {
	.cbcr_reg = MDSS_AXI_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_mdss_axi_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_mdss_axi_clk.c),
	},
};

static struct branch_clk gcc_mdss_byte0_clk = {
	.cbcr_reg = MDSS_BYTE0_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[MDSS_BASE],
	.c = {
		.dbg_name = "gcc_mdss_byte0_clk",
		.parent = &byte0_clk_src.c,
		.ops = &clk_ops_branch,
		.flags = CLKFLAG_NO_RATE_CACHE,
		CLK_INIT(gcc_mdss_byte0_clk.c),
	},
};

static struct branch_clk gcc_mdss_byte1_clk = {
	.cbcr_reg = MDSS_BYTE1_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[MDSS_BASE],
	.c = {
		.dbg_name = "gcc_mdss_byte1_clk",
		.parent = &byte1_clk_src.c,
		.ops = &clk_ops_branch,
		.flags = CLKFLAG_NO_RATE_CACHE,
		CLK_INIT(gcc_mdss_byte1_clk.c),
	},
};

static struct branch_clk gcc_mdss_esc0_clk = {
	.cbcr_reg = MDSS_ESC0_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_mdss_esc0_clk",
		.parent = &esc0_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_mdss_esc0_clk.c),
	},
};

static struct branch_clk gcc_mdss_esc1_clk = {
	.cbcr_reg = MDSS_ESC1_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_mdss_esc1_clk",
		.parent = &esc1_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_mdss_esc1_clk.c),
	},
};

static struct branch_clk gcc_mdss_mdp_clk = {
	.cbcr_reg = MDSS_MDP_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_mdss_mdp_clk",
		.parent = &mdp_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_mdss_mdp_clk.c),
	},
};

static DEFINE_CLK_VOTER(gcc_mdss_mdp_vote_clk, &gcc_mdss_mdp_clk.c, 0);
static DEFINE_CLK_VOTER(gcc_mdss_mdp_rotator_vote_clk, &gcc_mdss_mdp_clk.c, 0);

static struct branch_clk gcc_mdss_pclk0_clk = {
	.cbcr_reg = MDSS_PCLK0_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[MDSS_BASE],
	.c = {
		.dbg_name = "gcc_mdss_pclk0_clk",
		.parent = &pclk0_clk_src.c,
		.ops = &clk_ops_branch,
		.flags = CLKFLAG_NO_RATE_CACHE,
		CLK_INIT(gcc_mdss_pclk0_clk.c),
	},
};

static struct branch_clk gcc_mdss_pclk1_clk = {
	.cbcr_reg = MDSS_PCLK1_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[MDSS_BASE],
	.c = {
		.dbg_name = "gcc_mdss_pclk1_clk",
		.parent = &pclk1_clk_src.c,
		.ops = &clk_ops_branch,
		.flags = CLKFLAG_NO_RATE_CACHE,
		CLK_INIT(gcc_mdss_pclk1_clk.c),
	},
};

static struct branch_clk gcc_mdss_vsync_clk = {
	.cbcr_reg = MDSS_VSYNC_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_mdss_vsync_clk",
		.parent = &vsync_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_mdss_vsync_clk.c),
	},
};

static struct branch_clk gcc_mss_cfg_ahb_clk = {
	.cbcr_reg = MSS_CFG_AHB_CBCR,
	.has_sibling = 1,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_mss_cfg_ahb_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_mss_cfg_ahb_clk.c),
	},
};

static struct branch_clk gcc_mss_q6_bimc_axi_clk = {
	.cbcr_reg = MSS_Q6_BIMC_AXI_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_mss_q6_bimc_axi_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_mss_q6_bimc_axi_clk.c),
	},
};

static struct branch_clk gcc_bimc_gfx_clk = {
	.cbcr_reg = BIMC_GFX_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GFX_BASE],
	.c = {
		.dbg_name = "gcc_bimc_gfx_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_bimc_gfx_clk.c),
	},
};

static struct branch_clk gcc_oxili_ahb_clk = {
	.cbcr_reg = OXILI_AHB_CBCR,
	.has_sibling = 1,
	.base = &virt_bases[GFX_BASE],
	.c = {
		.dbg_name = "gcc_oxili_ahb_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_oxili_ahb_clk.c),
	},
};

static struct branch_clk gcc_oxili_aon_clk = {
	.cbcr_reg = OXILI_AON_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GFX_BASE],
	.c = {
		.dbg_name = "gcc_oxili_aon_clk",
		.parent = &gfx3d_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_oxili_aon_clk.c),
	},
};

static struct branch_clk gcc_oxili_gfx3d_clk = {
	.cbcr_reg = OXILI_GFX3D_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GFX_BASE],
	.c = {
		.dbg_name = "gcc_oxili_gfx3d_clk",
		.parent = &gfx3d_clk_src.c,
		VDD_DIG_FMAX_MAP5(LOWER, 300000000, LOW, 366670000,
				NOMINAL, 432000000, NOM_PLUS, 480000000,
				HIGH, 600000000),
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_oxili_gfx3d_clk.c),
	},
};

static struct branch_clk gcc_oxili_timer_clk = {
	.cbcr_reg = OXILI_TIMER_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GFX_BASE],
	.c = {
		.dbg_name = "gcc_oxili_timer_clk",
		.parent = &xo_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_oxili_timer_clk.c),
	},
};

static struct branch_clk gcc_pdm2_clk = {
	.cbcr_reg = PDM2_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_pdm2_clk",
		.parent = &pdm2_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_pdm2_clk.c),
	},
};

static struct branch_clk gcc_pdm_ahb_clk = {
	.cbcr_reg = PDM_AHB_CBCR,
	.has_sibling = 1,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_pdm_ahb_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_pdm_ahb_clk.c),
	},
};


static struct branch_clk gcc_rbcpr_gfx_ahb_clk = {
	.cbcr_reg = RBCPR_GFX_AHB_CBCR,
	.has_sibling = 1,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_rbcpr_gfx_ahb_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_rbcpr_gfx_ahb_clk.c),
	},
};

static struct branch_clk gcc_rbcpr_gfx_clk = {
	.cbcr_reg = RBCPR_GFX_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_rbcpr_gfx_clk",
		.parent = &rbcpr_gfx_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_rbcpr_gfx_clk.c),
	},
};

static struct branch_clk gcc_sdcc1_ahb_clk = {
	.cbcr_reg = SDCC1_AHB_CBCR,
	.has_sibling = 1,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_sdcc1_ahb_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_sdcc1_ahb_clk.c),
	},
};

static struct branch_clk gcc_sdcc1_apps_clk = {
	.cbcr_reg = SDCC1_APPS_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_sdcc1_apps_clk",
		.parent = &sdcc1_apps_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_sdcc1_apps_clk.c),
	},
};

static struct branch_clk gcc_sdcc1_ice_core_clk = {
	.cbcr_reg = SDCC1_ICE_CORE_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_sdcc1_ice_core_clk",
		.parent = &sdcc1_ice_core_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_sdcc1_ice_core_clk.c),
	},
};

static struct branch_clk gcc_sdcc2_ahb_clk = {
	.cbcr_reg = SDCC2_AHB_CBCR,
	.has_sibling = 1,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_sdcc2_ahb_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_sdcc2_ahb_clk.c),
	},
};

static struct branch_clk gcc_sdcc2_apps_clk = {
	.cbcr_reg = SDCC2_APPS_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_sdcc2_apps_clk",
		.parent = &sdcc2_apps_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_sdcc2_apps_clk.c),
	},
};

static struct branch_clk gcc_sdcc3_ahb_clk = {
	.cbcr_reg = SDCC3_AHB_CBCR,
	.has_sibling = 1,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_sdcc3_ahb_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_sdcc3_ahb_clk.c),
	},
};

static struct branch_clk gcc_sdcc3_apps_clk = {
	.cbcr_reg = SDCC3_APPS_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_sdcc3_apps_clk",
		.parent = &sdcc3_apps_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_sdcc3_apps_clk.c),
	},
};

static struct branch_clk gcc_usb2a_phy_sleep_clk = {
	.cbcr_reg = USB2A_PHY_SLEEP_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_usb2a_phy_sleep_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_usb2a_phy_sleep_clk.c),
	},
};

static struct branch_clk gcc_usb_hs_phy_cfg_ahb_clk = {
	.cbcr_reg = USB_HS_PHY_CFG_AHB_CBCR,
	.has_sibling = 1,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_usb_hs_phy_cfg_ahb_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_usb_hs_phy_cfg_ahb_clk.c),
	},
};

static struct branch_clk gcc_usb_fs_ahb_clk = {
	.cbcr_reg = USB_FS_AHB_CBCR,
	.has_sibling = 1,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_usb_fs_ahb_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_usb_fs_ahb_clk.c),
	},
};

static struct branch_clk gcc_usb_fs_ic_clk = {
	.cbcr_reg = USB_FS_IC_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_usb_fs_ic_clk",
		.parent = &usb_fs_ic_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_usb_fs_ic_clk.c),
	},
};

static struct branch_clk gcc_usb_fs_system_clk = {
	.cbcr_reg = USB_FS_SYSTEM_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_usb_fs_system_clk",
		.parent = &usb_fs_system_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_usb_fs_system_clk.c),
	},
};

static struct branch_clk gcc_usb_hs_ahb_clk = {
	.cbcr_reg = USB_HS_AHB_CBCR,
	.has_sibling = 1,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_usb_hs_ahb_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_usb_hs_ahb_clk.c),
	},
};

static struct branch_clk gcc_usb_hs_system_clk = {
	.cbcr_reg = USB_HS_SYSTEM_CBCR,
	.bcr_reg = USB_HS_BCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_usb_hs_system_clk",
		.parent = &usb_hs_system_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_usb_hs_system_clk.c),
	},
};

static struct branch_clk gcc_venus0_ahb_clk = {
	.cbcr_reg = VENUS0_AHB_CBCR,
	.has_sibling = 1,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_venus0_ahb_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_venus0_ahb_clk.c),
	},
};

static struct branch_clk gcc_venus0_axi_clk = {
	.cbcr_reg = VENUS0_AXI_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_venus0_axi_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_venus0_axi_clk.c),
	},
};

static struct branch_clk gcc_venus0_core0_vcodec0_clk = {
	.cbcr_reg = VENUS0_CORE0_VCODEC0_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_venus0_core0_vcodec0_clk",
		.parent = &vcodec0_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_venus0_core0_vcodec0_clk.c),
	},
};

static struct branch_clk gcc_venus0_vcodec0_clk = {
	.cbcr_reg = VENUS0_VCODEC0_CBCR,
	.has_sibling = 0,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_venus0_vcodec0_clk",
		.parent = &vcodec0_clk_src.c,
		.ops = &clk_ops_branch,
		CLK_INIT(gcc_venus0_vcodec0_clk.c),
	},
};

static struct local_vote_clk gcc_apss_ahb_clk = {
	.cbcr_reg = APSS_AHB_CBCR,
	.vote_reg = APCS_CLOCK_BRANCH_ENA_VOTE,
	.en_mask = BIT(14),
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_apss_ahb_clk",
		.ops = &clk_ops_vote,
		CLK_INIT(gcc_apss_ahb_clk.c),
	},
};

static struct local_vote_clk gcc_apss_axi_clk = {
	.cbcr_reg = APSS_AXI_CBCR,
	.vote_reg = APCS_CLOCK_BRANCH_ENA_VOTE,
	.en_mask = BIT(13),
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_apss_axi_clk",
		.ops = &clk_ops_vote,
		CLK_INIT(gcc_apss_axi_clk.c),
	},
};

static struct local_vote_clk gcc_blsp1_ahb_clk = {
	.cbcr_reg = BLSP1_AHB_CBCR,
	.vote_reg = APCS_CLOCK_BRANCH_ENA_VOTE,
	.en_mask = BIT(10),
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_blsp1_ahb_clk",
		.ops = &clk_ops_vote,
		CLK_INIT(gcc_blsp1_ahb_clk.c),
	},
};


static struct local_vote_clk gcc_blsp2_ahb_clk = {
	.cbcr_reg = BLSP2_AHB_CBCR,
	.vote_reg = APCS_CLOCK_BRANCH_ENA_VOTE,
	.en_mask = BIT(20),
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_blsp2_ahb_clk",
		.ops = &clk_ops_vote,
		CLK_INIT(gcc_blsp2_ahb_clk.c),
	},
};

static struct local_vote_clk gcc_prng_ahb_clk = {
	.cbcr_reg = PRNG_AHB_CBCR,
	.vote_reg = APCS_CLOCK_BRANCH_ENA_VOTE,
	.en_mask = BIT(8),
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_prng_ahb_clk",
		.ops = &clk_ops_vote,
		CLK_INIT(gcc_prng_ahb_clk.c),
	},
};

static struct local_vote_clk gcc_boot_rom_ahb_clk = {
	.cbcr_reg = BOOT_ROM_AHB_CBCR,
	.vote_reg = APCS_CLOCK_BRANCH_ENA_VOTE,
	.en_mask = BIT(7),
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_boot_rom_ahb_clk",
		.ops = &clk_ops_vote,
		CLK_INIT(gcc_boot_rom_ahb_clk.c),
	},
};

static struct local_vote_clk gcc_crypto_ahb_clk = {
	.cbcr_reg = CRYPTO_AHB_CBCR,
	.vote_reg = APCS_CLOCK_BRANCH_ENA_VOTE,
	.en_mask = BIT(0),
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_crypto_ahb_clk",
		.ops = &clk_ops_vote,
		CLK_INIT(gcc_crypto_ahb_clk.c),
	},
};

static struct local_vote_clk gcc_crypto_axi_clk = {
	.cbcr_reg = CRYPTO_AXI_CBCR,
	.vote_reg = APCS_CLOCK_BRANCH_ENA_VOTE,
	.en_mask = BIT(1),
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_crypto_axi_clk",
		.ops = &clk_ops_vote,
		CLK_INIT(gcc_crypto_axi_clk.c),
	},
};

static struct local_vote_clk gcc_crypto_clk = {
	.cbcr_reg = CRYPTO_CBCR,
	.vote_reg = APCS_CLOCK_BRANCH_ENA_VOTE,
	.en_mask = BIT(2),
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_crypto_clk",
		.parent = &crypto_clk_src.c,
		.ops = &clk_ops_vote,
		CLK_INIT(gcc_crypto_clk.c),
	},
};

static struct local_vote_clk gcc_cpp_tbu_clk = {
	.cbcr_reg = CPP_TBU_CBCR,
	.vote_reg = APCS_SMMU_CLOCK_BRANCH_ENA_VOTE,
	.en_mask = BIT(14),
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_cpp_tbu_clk",
		.ops = &clk_ops_vote,
		CLK_INIT(gcc_cpp_tbu_clk.c),
	},
};

static struct local_vote_clk gcc_gfx_1_tbu_clk = {
	.cbcr_reg = GFX_1_TBU_CBCR,
	.vote_reg = APCS_SMMU_CLOCK_BRANCH_ENA_VOTE,
	.en_mask = BIT(19),
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_gfx_1_tbu_clk",
		.ops = &clk_ops_vote,
		CLK_INIT(gcc_gfx_1_tbu_clk.c),
	},
};

static struct local_vote_clk gcc_gfx_tbu_clk = {
	.cbcr_reg = GFX_TBU_CBCR,
	.vote_reg = APCS_SMMU_CLOCK_BRANCH_ENA_VOTE,
	.en_mask = BIT(3),
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_gfx_tbu_clk",
		.ops = &clk_ops_vote,
		CLK_INIT(gcc_gfx_tbu_clk.c),
	},
};

static struct local_vote_clk gcc_gfx_tcu_clk = {
	.cbcr_reg = GFX_TCU_CBCR,
	.vote_reg = APCS_SMMU_CLOCK_BRANCH_ENA_VOTE,
	.en_mask = BIT(2),
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_gfx_tcu_clk",
		.ops = &clk_ops_vote,
		CLK_INIT(gcc_gfx_tcu_clk.c),
	},
};

static struct local_vote_clk gcc_apss_tcu_clk = {
	.cbcr_reg = APSS_TCU_CBCR,
	.vote_reg = APCS_SMMU_CLOCK_BRANCH_ENA_VOTE,
	.en_mask = BIT(1),
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_apss_tcu_clk",
		.ops = &clk_ops_vote,
		CLK_INIT(gcc_apss_tcu_clk.c),
	},
};

static struct local_vote_clk gcc_gtcu_ahb_clk = {
	.cbcr_reg = GTCU_AHB_CBCR,
	.vote_reg = APCS_SMMU_CLOCK_BRANCH_ENA_VOTE,
	.en_mask = BIT(13),
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_gtcu_ahb_clk",
		.ops = &clk_ops_vote,
		CLK_INIT(gcc_gtcu_ahb_clk.c),
	},
};

static struct local_vote_clk gcc_jpeg_tbu_clk = {
	.cbcr_reg = JPEG_TBU_CBCR,
	.vote_reg = APCS_SMMU_CLOCK_BRANCH_ENA_VOTE,
	.en_mask = BIT(10),
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_jpeg_tbu_clk",
		.ops = &clk_ops_vote,
		CLK_INIT(gcc_jpeg_tbu_clk.c),
	},
};

static struct local_vote_clk gcc_mdp_rt_tbu_clk = {
	.cbcr_reg = MDP_RT_TBU_CBCR,
	.vote_reg = APCS_SMMU_CLOCK_BRANCH_ENA_VOTE,
	.en_mask = BIT(15),
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_mdp_rt_tbu_clk",
		.ops = &clk_ops_vote,
		CLK_INIT(gcc_mdp_rt_tbu_clk.c),
	},
};

static struct local_vote_clk gcc_mdp_tbu_clk = {
	.cbcr_reg = MDP_TBU_CBCR,
	.vote_reg = APCS_SMMU_CLOCK_BRANCH_ENA_VOTE,
	.en_mask = BIT(4),
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_mdp_tbu_clk",
		.ops = &clk_ops_vote,
		CLK_INIT(gcc_mdp_tbu_clk.c),
	},
};

static struct local_vote_clk gcc_smmu_cfg_clk = {
	.cbcr_reg = SMMU_CFG_CBCR,
	.vote_reg = APCS_SMMU_CLOCK_BRANCH_ENA_VOTE,
	.en_mask = BIT(12),
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_smmu_cfg_clk",
		.ops = &clk_ops_vote,
		CLK_INIT(gcc_smmu_cfg_clk.c),
	},
};

static struct local_vote_clk gcc_venus_1_tbu_clk = {
	.cbcr_reg = VENUS_1_TBU_CBCR,
	.vote_reg = APCS_SMMU_CLOCK_BRANCH_ENA_VOTE,
	.en_mask = BIT(20),
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_venus_1_tbu_clk",
		.ops = &clk_ops_vote,
		CLK_INIT(gcc_venus_1_tbu_clk.c),
	},
};

static struct local_vote_clk gcc_venus_tbu_clk = {
	.cbcr_reg = VENUS_TBU_CBCR,
	.vote_reg = APCS_SMMU_CLOCK_BRANCH_ENA_VOTE,
	.en_mask = BIT(5),
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_venus_tbu_clk",
		.ops = &clk_ops_vote,
		CLK_INIT(gcc_venus_tbu_clk.c),
	},
};

static struct local_vote_clk gcc_vfe1_tbu_clk = {
	.cbcr_reg = VFE1_TBU_CBCR,
	.vote_reg = APCS_SMMU_CLOCK_BRANCH_ENA_VOTE,
	.en_mask = BIT(17),
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_vfe1_tbu_clk",
		.ops = &clk_ops_vote,
		CLK_INIT(gcc_vfe1_tbu_clk.c),
	},
};

static struct local_vote_clk gcc_vfe_tbu_clk = {
	.cbcr_reg = VFE_TBU_CBCR,
	.vote_reg = APCS_SMMU_CLOCK_BRANCH_ENA_VOTE,
	.en_mask = BIT(9),
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_vfe_tbu_clk",
		.ops = &clk_ops_vote,
		CLK_INIT(gcc_vfe_tbu_clk.c),
	},
};

static struct reset_clk gcc_qusb2_phy_clk = {
	.reset_reg = QUSB2_PHY_BCR,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_qusb2_phy_clk",
		.ops = &clk_ops_rst,
		CLK_INIT(gcc_qusb2_phy_clk.c),
	},
};

static struct reset_clk gcc_usb2_hs_phy_only_clk = {
	.reset_reg = USB2_HS_PHY_ONLY_BCR,
	.base = &virt_bases[GCC_BASE],
	.c = {
		.dbg_name = "gcc_usb2_hs_phy_only_clk",
		.ops = &clk_ops_rst,
		CLK_INIT(gcc_usb2_hs_phy_only_clk.c),
	},
};

static struct clk_ops clk_ops_debug_mux;

static struct measure_clk_data debug_mux_priv = {
	.cxo = &xo_clk_src.c,
	.plltest_reg = PLLTEST_PAD_CFG,
	.plltest_val = 0x51A00,
	.xo_div4_cbcr = GCC_XO_DIV4_CBCR,
	.ctl_reg = CLOCK_FRQ_MEASURE_CTL,
	.status_reg = CLOCK_FRQ_MEASURE_STATUS,
	.base = &virt_bases[GCC_BASE],
};

static struct mux_clk gcc_debug_mux = {
	.priv = &debug_mux_priv,
	.ops = &mux_reg_ops,
	.offset = GCC_DEBUG_CLK_CTL,
	.mask = 0x3FF,
	.en_offset = GCC_DEBUG_CLK_CTL,
	.en_mask = BIT(16),
	.base = &virt_bases[GCC_BASE],
	MUX_REC_SRC_LIST(
		&debug_cpu_clk.c,
	),
	MUX_SRC_LIST(
		{ &debug_cpu_clk.c, 0x016A },
		{ &snoc_clk.c,  0x0000 },
		{ &pcnoc_clk.c, 0x0008 },
		{ &bimc_clk.c,  0x0154 },
		{ &sysmmnoc_clk.c , 0x0001},
		{ &ipa_clk.c , 0x0200},
		{ &gcc_gp1_clk.c, 0x0010 },
		{ &gcc_gp2_clk.c, 0x0011 },
		{ &gcc_gp3_clk.c, 0x0012 },
		{ &gcc_bimc_gfx_clk.c, 0x002d },
		{ &gcc_mss_cfg_ahb_clk.c, 0x0030 },
		{ &gcc_mss_q6_bimc_axi_clk.c, 0x0031 },
		{ &gcc_apss_tcu_clk.c, 0x0050 },
		{ &gcc_mdp_tbu_clk.c, 0x0051 },
		{ &gcc_gfx_tbu_clk.c, 0x0052 },
		{ &gcc_gfx_tcu_clk.c, 0x0053 },
		{ &gcc_venus_tbu_clk.c, 0x0054 },
		{ &gcc_gtcu_ahb_clk.c, 0x0058 },
		{ &gcc_vfe_tbu_clk.c, 0x005a },
		{ &gcc_smmu_cfg_clk.c, 0x005b },
		{ &gcc_jpeg_tbu_clk.c, 0x005c },
		{ &gcc_usb_hs_system_clk.c, 0x0060 },
		{ &gcc_usb_hs_ahb_clk.c, 0x0061 },
		{ &gcc_usb2a_phy_sleep_clk.c, 0x0063 },
		{ &gcc_usb_hs_phy_cfg_ahb_clk.c, 0x0064 },
		{ &gcc_sdcc1_apps_clk.c, 0x0068 },
		{ &gcc_sdcc1_ahb_clk.c, 0x0069 },
		{ &gcc_sdcc1_ice_core_clk.c, 0x006a },
		{ &gcc_sdcc2_apps_clk.c, 0x0070 },
		{ &gcc_sdcc2_ahb_clk.c, 0x0071 },
		{ &gcc_blsp1_ahb_clk.c, 0x0088 },
		{ &gcc_blsp1_qup1_spi_apps_clk.c, 0x008a },
		{ &gcc_blsp1_qup1_i2c_apps_clk.c, 0x008b },
		{ &gcc_blsp1_uart1_apps_clk.c, 0x008c },
		{ &gcc_blsp1_qup2_spi_apps_clk.c, 0x008e },
		{ &gcc_blsp1_qup2_i2c_apps_clk.c, 0x0090 },
		{ &gcc_blsp1_uart2_apps_clk.c, 0x0091 },
		{ &gcc_blsp1_qup3_spi_apps_clk.c, 0x0093 },
		{ &gcc_blsp1_qup3_i2c_apps_clk.c, 0x0094 },
		{ &gcc_blsp1_qup4_spi_apps_clk.c, 0x0098 },
		{ &gcc_blsp1_qup4_i2c_apps_clk.c, 0x0099 },
		{ &gcc_camss_ahb_clk.c, 0x00a8 },
		{ &gcc_camss_top_ahb_clk.c, 0x00a9 },
		{ &gcc_camss_micro_ahb_clk.c, 0x00aa },
		{ &gcc_camss_gp0_clk.c, 0x00ab },
		{ &gcc_camss_gp1_clk.c, 0x00ac },
		{ &gcc_camss_mclk0_clk.c, 0x00ad },
		{ &gcc_camss_mclk1_clk.c, 0x00ae },
		{ &gcc_camss_cci_clk.c, 0x00af },
		{ &gcc_camss_cci_ahb_clk.c, 0x00b0 },
		{ &gcc_camss_csi0phytimer_clk.c, 0x00b1 },
		{ &gcc_camss_csi1phytimer_clk.c, 0x00b2 },
		{ &gcc_camss_jpeg0_clk.c, 0x00b3 },
		{ &gcc_camss_jpeg_ahb_clk.c, 0x00b4 },
		{ &gcc_camss_jpeg_axi_clk.c, 0x00b5 },
		{ &gcc_camss_vfe0_clk.c, 0x00b8 },
		{ &gcc_camss_cpp_clk.c, 0x00b9 },
		{ &gcc_camss_cpp_ahb_clk.c, 0x00ba },
		{ &gcc_camss_vfe_ahb_clk.c, 0x00bb },
		{ &gcc_camss_vfe_axi_clk.c, 0x00bc },
		{ &gcc_camss_csi_vfe0_clk.c, 0x00bf },
		{ &gcc_camss_csi0_clk.c, 0x00c0 },
		{ &gcc_camss_csi0_ahb_clk.c, 0x00c1 },
		{ &gcc_camss_csi0phy_clk.c, 0x00c2 },
		{ &gcc_camss_csi0rdi_clk.c, 0x00c3 },
		{ &gcc_camss_csi0pix_clk.c, 0x00c4 },
		{ &gcc_camss_csi1_clk.c, 0x00c5 },
		{ &gcc_camss_csi1_ahb_clk.c, 0x00c6 },
		{ &gcc_camss_csi1phy_clk.c, 0x00c7 },
		{ &gcc_pdm_ahb_clk.c, 0x00d0 },
		{ &gcc_pdm2_clk.c, 0x00d2 },
		{ &gcc_prng_ahb_clk.c, 0x00d8 },
		{ &gcc_camss_csi1rdi_clk.c, 0x00e0 },
		{ &gcc_camss_csi1pix_clk.c, 0x00e1 },
		{ &gcc_camss_ispif_ahb_clk.c, 0x00e2 },
		{ &gcc_camss_csi2_clk.c, 0x00e3 },
		{ &gcc_camss_csi2_ahb_clk.c, 0x00e4 },
		{ &gcc_camss_csi2phy_clk.c, 0x00e5 },
		{ &gcc_camss_csi2rdi_clk.c, 0x00e6 },
		{ &gcc_camss_csi2pix_clk.c, 0x00e7 },
		{ &gcc_cpp_tbu_clk.c, 0x00e9 },
		{ &gcc_camss_vfe1_axi_clk.c, 0x00ea },
		{ &gcc_camss_vfe1_ahb_clk.c, 0x00eb },
		{ &gcc_camss_vfe1_clk.c, 0x00ec },
		{ &gcc_mdp_rt_tbu_clk.c, 0x00ee },
		{ &gcc_usb_fs_system_clk.c, 0x00f0 },
		{ &gcc_usb_fs_ahb_clk.c, 0x00f1 },
		{ &gcc_usb_fs_ic_clk.c, 0x00f4 },
		{ &gcc_boot_rom_ahb_clk.c, 0x00f8 },
		{ &gcc_crypto_clk.c, 0x0138 },
		{ &gcc_crypto_axi_clk.c, 0x0139 },
		{ &gcc_crypto_ahb_clk.c, 0x013a },
		{ &gcc_camss_csi_vfe1_clk.c, 0x01b4 },
		{ &gcc_camss_cpp_axi_clk.c, 0x01b5 },
		{ &gcc_venus0_core0_vcodec0_clk.c, 0x01b8 },
		{ &gcc_mdss_pclk1_clk.c, 0x01ba },
		{ &gcc_mdss_byte1_clk.c, 0x01bb },
		{ &gcc_mdss_esc1_clk.c, 0x01bc },
		{ &gcc_camss_mclk2_clk.c, 0x01bd },
		{ &gcc_oxili_timer_clk.c, 0x01e9 },
		{ &gcc_oxili_gfx3d_clk.c, 0x01ea },
		{ &gcc_oxili_ahb_clk.c, 0x01eb },
		{ &gcc_oxili_aon_clk.c, 0x01ee },
		{ &gcc_oxili_gmem_clk.c, 0x01f0 },
		{ &gcc_venus0_vcodec0_clk.c, 0x01f1 },
		{ &gcc_venus0_axi_clk.c, 0x01f2 },
		{ &gcc_venus0_ahb_clk.c, 0x01f3 },
		{ &gcc_mdss_ahb_clk.c, 0x01f6 },
		{ &gcc_mdss_axi_clk.c, 0x01f7 },
		{ &gcc_mdss_pclk0_clk.c, 0x01f8 },
		{ &gcc_mdss_mdp_clk.c, 0x01f9 },
		{ &gcc_mdss_vsync_clk.c, 0x01fb },
		{ &gcc_mdss_byte0_clk.c, 0x01fc },
		{ &gcc_mdss_esc0_clk.c, 0x01fd },
		{ &gcc_vfe1_tbu_clk.c, 0x0209 },
		{ &gcc_gfx_1_tbu_clk.c, 0x020b },
		{ &gcc_venus_1_tbu_clk.c, 0x020c },
		{ &gcc_rbcpr_gfx_clk.c, 0x0218 },
		{ &gcc_rbcpr_gfx_ahb_clk.c, 0x0219 },
		{ &gcc_sdcc3_apps_clk.c, 0x0220 },
		{ &gcc_sdcc3_ahb_clk.c, 0x0221 },
		{ &gcc_blsp2_ahb_clk.c, 0x0228 },
		{ &gcc_blsp2_qup1_spi_apps_clk.c, 0x022a },
		{ &gcc_blsp2_qup1_i2c_apps_clk.c, 0x022b },
		{ &gcc_blsp2_uart1_apps_clk.c, 0x022c },
		{ &gcc_blsp2_qup2_spi_apps_clk.c, 0x022e },
		{ &gcc_blsp2_qup2_i2c_apps_clk.c, 0x0230 },
		{ &gcc_blsp2_uart2_apps_clk.c, 0x0231 },
		{ &gcc_blsp2_qup3_spi_apps_clk.c, 0x0233 },
		{ &gcc_blsp2_qup3_i2c_apps_clk.c, 0x0234 },
		{ &gcc_blsp2_qup4_spi_apps_clk.c, 0x0238 },
		{ &gcc_blsp2_qup4_i2c_apps_clk.c, 0x0239 },
		{ &gcc_dcc_clk.c, 0x0268 },
		{ &gcc_aps_0_clk.c, 0x0280 },
		{ &gcc_aps_1_clk.c, 0x0288 },
		{ &wcnss_m_clk.c,   0x0198 },
	),
	.c = {
		.dbg_name = "gcc_debug_mux",
		.ops = &clk_ops_debug_mux,
		.flags = CLKFLAG_NO_RATE_CACHE | CLKFLAG_MEASURE,
		CLK_INIT(gcc_debug_mux.c),
	},
};

/* Clock lookup */
static struct clk_lookup msm_clocks_lookup[] = {
	 CLK_LIST(gpll0_clk_src),
	 CLK_LIST(gpll0_ao_clk_src),
	 CLK_LIST(gpll2_clk_src),
	 CLK_LIST(gpll3_clk_src),
	 CLK_LIST(gpll4_clk_src),
	 CLK_LIST(gpll6_clk_src),
	 CLK_LIST(xo_clk_src),
	 CLK_LIST(xo_a_clk_src),
	 CLK_LIST(bimc_clk),
	 CLK_LIST(bimc_a_clk),
	 CLK_LIST(ipa_clk),
	 CLK_LIST(ipa_a_clk),
	 CLK_LIST(pcnoc_clk),
	 CLK_LIST(pcnoc_a_clk),
	 CLK_LIST(qdss_clk),
	 CLK_LIST(qdss_a_clk),
	 CLK_LIST(snoc_clk),
	 CLK_LIST(snoc_a_clk),
	 CLK_LIST(sysmmnoc_clk),
	 CLK_LIST(sysmmnoc_a_clk),
	 CLK_LIST(sysmmnoc_msmbus_clk),
	 CLK_LIST(sysmmnoc_msmbus_a_clk),
	 CLK_LIST(bimc_msmbus_clk),
	 CLK_LIST(bimc_msmbus_a_clk),
	 CLK_LIST(bimc_usb_clk),
	 CLK_LIST(bimc_usb_a_clk),
	 CLK_LIST(pcnoc_keepalive_a_clk),
	 CLK_LIST(pcnoc_msmbus_clk),
	 CLK_LIST(pcnoc_msmbus_a_clk),
	 CLK_LIST(pcnoc_usb_clk),
	 CLK_LIST(pcnoc_usb_a_clk),
	 CLK_LIST(snoc_msmbus_clk),
	 CLK_LIST(snoc_msmbus_a_clk),
	 CLK_LIST(snoc_usb_clk),
	 CLK_LIST(snoc_usb_a_clk),
	 CLK_LIST(xo_lpm_clk),
	 CLK_LIST(xo_otg_clk),
	 CLK_LIST(xo_pil_lpass_clk),
	 CLK_LIST(xo_pil_mss_clk),
	 CLK_LIST(xo_pil_pronto_clk),
	 CLK_LIST(xo_wlan_clk),

	 /* XO buffers */
	 CLK_LIST(bb_clk1),
	 CLK_LIST(bb_clk2),
	 CLK_LIST(rf_clk2),
	 CLK_LIST(div_clk2),
	 CLK_LIST(bb_clk1_pin),
	 CLK_LIST(bb_clk2_pin),

	 CLK_LIST(gcc_blsp1_qup1_spi_apps_clk),
	 CLK_LIST(gcc_blsp1_qup1_i2c_apps_clk),
	 CLK_LIST(gcc_blsp1_qup2_i2c_apps_clk),
	 CLK_LIST(gcc_blsp1_qup2_spi_apps_clk),
	 CLK_LIST(gcc_blsp1_qup3_i2c_apps_clk),
	 CLK_LIST(gcc_blsp1_qup3_spi_apps_clk),
	 CLK_LIST(gcc_blsp1_qup4_i2c_apps_clk),
	 CLK_LIST(gcc_blsp1_qup4_spi_apps_clk),
	 CLK_LIST(gcc_blsp1_uart1_apps_clk),
	 CLK_LIST(gcc_blsp1_uart2_apps_clk),
	 CLK_LIST(gcc_blsp2_qup1_i2c_apps_clk),
	 CLK_LIST(gcc_blsp2_qup1_spi_apps_clk),
	 CLK_LIST(gcc_blsp2_qup2_i2c_apps_clk),
	 CLK_LIST(gcc_blsp2_qup2_spi_apps_clk),
	 CLK_LIST(gcc_blsp2_qup3_i2c_apps_clk),
	 CLK_LIST(gcc_blsp2_qup3_spi_apps_clk),
	 CLK_LIST(gcc_blsp2_qup4_i2c_apps_clk),
	 CLK_LIST(gcc_blsp2_qup4_spi_apps_clk),
	 CLK_LIST(gcc_blsp2_uart1_apps_clk),
	 CLK_LIST(gcc_blsp2_uart2_apps_clk),
	 CLK_LIST(gcc_camss_cci_ahb_clk),
	 CLK_LIST(gcc_camss_cci_clk),
	 CLK_LIST(gcc_camss_cpp_ahb_clk),
	 CLK_LIST(gcc_camss_cpp_axi_clk),
	 CLK_LIST(gcc_camss_cpp_clk),
	 CLK_LIST(gcc_camss_csi0_ahb_clk),
	 CLK_LIST(gcc_camss_csi0_clk),
	 CLK_LIST(gcc_camss_csi0phy_clk),
	 CLK_LIST(gcc_camss_csi0pix_clk),
	 CLK_LIST(gcc_camss_csi0rdi_clk),
	 CLK_LIST(gcc_camss_csi1_ahb_clk),
	 CLK_LIST(gcc_camss_csi1_clk),
	 CLK_LIST(gcc_camss_csi1phy_clk),
	 CLK_LIST(gcc_camss_csi1pix_clk),
	 CLK_LIST(gcc_camss_csi1rdi_clk),
	 CLK_LIST(gcc_camss_csi2_ahb_clk),
	 CLK_LIST(gcc_camss_csi2_clk),
	 CLK_LIST(gcc_camss_csi2phy_clk),
	 CLK_LIST(gcc_camss_csi2pix_clk),
	 CLK_LIST(gcc_camss_csi2rdi_clk),
	 CLK_LIST(gcc_camss_csi_vfe0_clk),
	 CLK_LIST(gcc_camss_csi_vfe1_clk),
	 CLK_LIST(gcc_camss_gp0_clk),
	 CLK_LIST(gcc_camss_gp1_clk),
	 CLK_LIST(gcc_camss_ispif_ahb_clk),
	 CLK_LIST(gcc_camss_jpeg0_clk),
	 CLK_LIST(gcc_camss_jpeg_ahb_clk),
	 CLK_LIST(gcc_camss_jpeg_axi_clk),
	 CLK_LIST(gcc_camss_mclk0_clk),
	 CLK_LIST(gcc_camss_mclk1_clk),
	 CLK_LIST(gcc_camss_mclk2_clk),
	 CLK_LIST(gcc_camss_micro_ahb_clk),
	 CLK_LIST(gcc_camss_csi0phytimer_clk),
	 CLK_LIST(gcc_camss_csi1phytimer_clk),
	 CLK_LIST(gcc_camss_ahb_clk),
	 CLK_LIST(gcc_camss_top_ahb_clk),
	 CLK_LIST(gcc_camss_vfe0_clk),
	 CLK_LIST(gcc_camss_vfe_ahb_clk),
	 CLK_LIST(gcc_camss_vfe_axi_clk),
	 CLK_LIST(gcc_camss_vfe1_ahb_clk),
	 CLK_LIST(gcc_camss_vfe1_axi_clk),
	 CLK_LIST(gcc_camss_vfe1_clk),
	 CLK_LIST(gcc_dcc_clk),
	 CLK_LIST(gcc_gp1_clk),
	 CLK_LIST(gcc_gp2_clk),
	 CLK_LIST(gcc_gp3_clk),
	 CLK_LIST(gcc_mdss_ahb_clk),
	 CLK_LIST(gcc_mdss_axi_clk),
	 CLK_LIST(gcc_mdss_esc0_clk),
	 CLK_LIST(gcc_mdss_esc1_clk),
	 CLK_LIST(gcc_mdss_mdp_clk),
	 CLK_LIST(gcc_mdss_vsync_clk),
	 CLK_LIST(gcc_mss_cfg_ahb_clk),
	 CLK_LIST(gcc_mss_q6_bimc_axi_clk),
	 CLK_LIST(gcc_pdm2_clk),
	 CLK_LIST(gcc_prng_ahb_clk),
	 CLK_LIST(gcc_pdm_ahb_clk),
	 CLK_LIST(gcc_rbcpr_gfx_ahb_clk),
	 CLK_LIST(gcc_rbcpr_gfx_clk),
	 CLK_LIST(gcc_sdcc1_ahb_clk),
	 CLK_LIST(gcc_sdcc1_apps_clk),
	 CLK_LIST(gcc_sdcc1_ice_core_clk),
	 CLK_LIST(gcc_sdcc2_ahb_clk),
	 CLK_LIST(gcc_sdcc2_apps_clk),
	 CLK_LIST(gcc_sdcc3_ahb_clk),
	 CLK_LIST(gcc_sdcc3_apps_clk),
	 CLK_LIST(gcc_usb2a_phy_sleep_clk),
	 CLK_LIST(gcc_usb_hs_phy_cfg_ahb_clk),
	 CLK_LIST(gcc_usb_fs_ahb_clk),
	 CLK_LIST(gcc_usb_fs_ic_clk),
	 CLK_LIST(gcc_usb_fs_system_clk),
	 CLK_LIST(gcc_usb_hs_ahb_clk),
	 CLK_LIST(gcc_usb_hs_system_clk),
	 CLK_LIST(gcc_venus0_ahb_clk),
	 CLK_LIST(gcc_venus0_axi_clk),
	 CLK_LIST(gcc_venus0_core0_vcodec0_clk),
	 CLK_LIST(gcc_venus0_vcodec0_clk),
	 CLK_LIST(gcc_apss_ahb_clk),
	 CLK_LIST(gcc_apss_axi_clk),
	 CLK_LIST(gcc_blsp1_ahb_clk),
	 CLK_LIST(gcc_blsp2_ahb_clk),
	 CLK_LIST(gcc_boot_rom_ahb_clk),
	 CLK_LIST(gcc_crypto_ahb_clk),
	 CLK_LIST(gcc_crypto_axi_clk),
	 CLK_LIST(gcc_crypto_clk),
	 CLK_LIST(gcc_cpp_tbu_clk),
	 CLK_LIST(gcc_apss_tcu_clk),
	 CLK_LIST(gcc_jpeg_tbu_clk),
	 CLK_LIST(gcc_mdp_rt_tbu_clk),
	 CLK_LIST(gcc_mdp_tbu_clk),
	 CLK_LIST(gcc_smmu_cfg_clk),
	 CLK_LIST(gcc_venus_1_tbu_clk),
	 CLK_LIST(gcc_venus_tbu_clk),
	 CLK_LIST(gcc_vfe1_tbu_clk),
	 CLK_LIST(gcc_vfe_tbu_clk),
	 CLK_LIST(gcc_aps_0_clk),
	 CLK_LIST(gcc_aps_1_clk),
	 CLK_LIST(aps_0_clk_src),
	 CLK_LIST(aps_1_clk_src),
	 CLK_LIST(apss_ahb_clk_src),
	 CLK_LIST(blsp1_qup1_i2c_apps_clk_src),
	 CLK_LIST(blsp1_qup1_spi_apps_clk_src),
	 CLK_LIST(blsp1_qup2_i2c_apps_clk_src),
	 CLK_LIST(blsp1_qup2_spi_apps_clk_src),
	 CLK_LIST(blsp1_qup3_i2c_apps_clk_src),
	 CLK_LIST(blsp1_qup3_spi_apps_clk_src),
	 CLK_LIST(blsp1_qup4_i2c_apps_clk_src),
	 CLK_LIST(blsp1_qup4_spi_apps_clk_src),
	 CLK_LIST(blsp1_uart1_apps_clk_src),
	 CLK_LIST(blsp1_uart2_apps_clk_src),
	 CLK_LIST(blsp2_qup1_i2c_apps_clk_src),
	 CLK_LIST(blsp2_qup1_spi_apps_clk_src),
	 CLK_LIST(blsp2_qup2_i2c_apps_clk_src),
	 CLK_LIST(blsp2_qup2_spi_apps_clk_src),
	 CLK_LIST(blsp2_qup3_i2c_apps_clk_src),
	 CLK_LIST(blsp2_qup3_spi_apps_clk_src),
	 CLK_LIST(blsp2_qup4_i2c_apps_clk_src),
	 CLK_LIST(blsp2_qup4_spi_apps_clk_src),
	 CLK_LIST(blsp2_uart1_apps_clk_src),
	 CLK_LIST(blsp2_uart2_apps_clk_src),
	 CLK_LIST(cci_clk_src),
	 CLK_LIST(cpp_clk_src),
	 CLK_LIST(csi0_clk_src),
	 CLK_LIST(csi1_clk_src),
	 CLK_LIST(csi2_clk_src),
	 CLK_LIST(camss_gp0_clk_src),
	 CLK_LIST(camss_gp1_clk_src),
	 CLK_LIST(jpeg0_clk_src),
	 CLK_LIST(mclk0_clk_src),
	 CLK_LIST(mclk1_clk_src),
	 CLK_LIST(mclk2_clk_src),
	 CLK_LIST(csi0phytimer_clk_src),
	 CLK_LIST(csi1phytimer_clk_src),
	 CLK_LIST(camss_top_ahb_clk_src),
	 CLK_LIST(vfe0_clk_src),
	 CLK_LIST(vfe1_clk_src),
	 CLK_LIST(crypto_clk_src),
	 CLK_LIST(gp1_clk_src),
	 CLK_LIST(gp2_clk_src),
	 CLK_LIST(gp3_clk_src),
	 CLK_LIST(esc0_clk_src),
	 CLK_LIST(esc1_clk_src),
	 CLK_LIST(mdp_clk_src),
	 CLK_LIST(vsync_clk_src),
	 CLK_LIST(pdm2_clk_src),
	 CLK_LIST(rbcpr_gfx_clk_src),
	 CLK_LIST(sdcc1_apps_clk_src),
	 CLK_LIST(sdcc1_ice_core_clk_src),
	 CLK_LIST(sdcc2_apps_clk_src),
	 CLK_LIST(sdcc3_apps_clk_src),
	 CLK_LIST(usb_fs_ic_clk_src),
	 CLK_LIST(usb_fs_system_clk_src),
	 CLK_LIST(usb_hs_system_clk_src),
	 CLK_LIST(vcodec0_clk_src),
	 CLK_LIST(gcc_qusb2_phy_clk),
	 CLK_LIST(gcc_usb2_hs_phy_only_clk),
	 CLK_LIST(wcnss_m_clk),
};

static int msm_gcc_probe(struct platform_device *pdev)
{
	struct resource *res;
	int ret;
	u32 regval;
	bool compat_bin = false;

	ret = vote_bimc(&bimc_clk, INT_MAX);
	if (ret < 0)
		return ret;

	compat_bin = of_device_is_compatible(pdev->dev.of_node,
						"qcom,gcc-8976-v1");

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "cc_base");
	if (!res) {
		dev_err(&pdev->dev, "Register base not defined\n");
		return -ENOMEM;
	}

	virt_bases[GCC_BASE] = devm_ioremap(&pdev->dev, res->start,
							resource_size(res));
	if (!virt_bases[GCC_BASE]) {
		dev_err(&pdev->dev, "Failed to ioremap CC registers\n");
		return -ENOMEM;
	}

	vdd_dig.regulator[0] = devm_regulator_get(&pdev->dev, "vdd_dig");
	if (IS_ERR(vdd_dig.regulator[0])) {
		if (!(PTR_ERR(vdd_dig.regulator[0]) == -EPROBE_DEFER))
			dev_err(&pdev->dev,
					"Unable to get vdd_dig regulator!!!\n");
		return PTR_ERR(vdd_dig.regulator[0]);
	}

	 /*Vote for GPLL0 to turn on. Needed by acpuclock. */
	regval = readl_relaxed(GCC_REG_BASE(APCS_GPLL_ENA_VOTE));
	regval |= BIT(0);
	writel_relaxed(regval, GCC_REG_BASE(APCS_GPLL_ENA_VOTE));

	if (compat_bin)
		sdcc1_apps_clk_src.freq_tbl = ftbl_sdcc1_v1_apps_clk_src;

	ret = of_msm_clock_register(pdev->dev.of_node,
				msm_clocks_lookup,
				ARRAY_SIZE(msm_clocks_lookup));
	if (ret)
		return ret;

	ret = enable_rpm_scaling();
	if (ret < 0) {
		dev_err(&pdev->dev, "rpm scaling failed to enable %d\n", ret);
		return ret;
	}

	clk_set_rate(&apss_ahb_clk_src.c, 19200000);
	clk_prepare_enable(&apss_ahb_clk_src.c);

	/*
	 * Hold an active set vote for PCNOC AHB source. Sleep set
	 * vote is 0.
	 */
	clk_set_rate(&pcnoc_keepalive_a_clk.c, 19200000);
	clk_prepare_enable(&pcnoc_keepalive_a_clk.c);

	clk_prepare_enable(&xo_a_clk_src.c);

	/* Configure Sleep and Wakeup cycles for GMEM clock */
	regval = readl_relaxed(GCC_REG_BASE(OXILI_GMEM_CBCR));
	regval ^= 0xFF0;
	regval |= CLKFLAG_WAKEUP_CYCLES << 8;
	regval |= CLKFLAG_SLEEP_CYCLES << 4;
	writel_relaxed(regval, GCC_REG_BASE(OXILI_GMEM_CBCR));

	configure_sr_hpm_lp_pll(&gpll3_config, &gpll3_regs, 1);

	/* Enable AUX2 clock for APSS */
	regval = readl_relaxed(GCC_REG_BASE(APSS_MISC));
	regval |= BIT(2);
	writel_relaxed(regval, GCC_REG_BASE(APSS_MISC));

	dev_info(&pdev->dev, "Registered GCC clocks\n");

	return 0;
}

static struct of_device_id msm_clock_gcc_match_table[] = {
	{ .compatible = "qcom,gcc-8976" },
	{ .compatible = "qcom,gcc-8976-v1" },
	{},
};

static struct platform_driver msm_clock_gcc_driver = {
	.probe = msm_gcc_probe,
	.driver = {
		.name = "qcom,gcc-8976",
		.of_match_table = msm_clock_gcc_match_table,
		.owner = THIS_MODULE,
	},
};

static int __init msm_gcc_init(void)
{
	return platform_driver_register(&msm_clock_gcc_driver);
}
arch_initcall(msm_gcc_init);

static struct clk_lookup msm_clocks_measure[] = {
	CLK_LOOKUP_OF("measure", gcc_debug_mux, "debug"),
	CLK_LIST(debug_cpu_clk),
};

static int msm_clock_debug_probe(struct platform_device *pdev)
{
	int ret;

	clk_ops_debug_mux = clk_ops_gen_mux;
	clk_ops_debug_mux.get_rate = measure_get_rate;

	debug_cpu_clk.c.parent = devm_clk_get(&pdev->dev, "debug_cpu_clk");
	if (IS_ERR(debug_cpu_clk.c.parent)) {
		dev_err(&pdev->dev, "Failed to get CPU debug Mux\n");
		return PTR_ERR(debug_cpu_clk.c.parent);
	}

	ret =  of_msm_clock_register(pdev->dev.of_node, msm_clocks_measure,
					ARRAY_SIZE(msm_clocks_measure));
	if (ret) {
		dev_err(&pdev->dev, "Failed to register debug Mux\n");
		return ret;
	}

	dev_info(&pdev->dev, "Registered Debug Mux successfully\n");
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

/* MDSS DSI_PHY_PLL */
static struct clk_lookup msm_clocks_gcc_mdss[] = {
	CLK_LIST(ext_byte0_clk_src),
	CLK_LIST(ext_byte1_clk_src),
	CLK_LIST(ext_pclk0_clk_src),
	CLK_LIST(ext_pclk1_clk_src),
	CLK_LIST(byte0_clk_src),
	CLK_LIST(byte1_clk_src),
	CLK_LIST(pclk0_clk_src),
	CLK_LIST(pclk1_clk_src),
	CLK_LIST(gcc_mdss_pclk0_clk),
	CLK_LIST(gcc_mdss_pclk1_clk),
	CLK_LIST(gcc_mdss_byte0_clk),
	CLK_LIST(gcc_mdss_byte1_clk),

	/* voters for new rotator implementation */
	CLK_LIST(gcc_mdss_mdp_vote_clk),
	CLK_LIST(gcc_mdss_mdp_rotator_vote_clk),
};

static int msm_gcc_mdss_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct clk *curr_p;
	struct resource *res;
	bool compat_bin = false;

	compat_bin = of_device_is_compatible(pdev->dev.of_node,
						"qcom,gcc-mdss-8976-v1");

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "cc_base");
	if (!res) {
		dev_err(&pdev->dev, "Register base not defined\n");
		return -ENOMEM;
	}

	virt_bases[MDSS_BASE] = devm_ioremap(&pdev->dev, res->start,
						resource_size(res));
	if (!virt_bases[MDSS_BASE]) {
		dev_err(&pdev->dev, "Failed to ioremap CC registers\n");
		return -ENOMEM;
	}

	if (compat_bin) {
		pclk1_clk_src.freq_tbl = ftbl_pclk1_v1_clk_src;
		byte1_clk_src.freq_tbl = ftbl_byte1_v1_clk_src;
	}

	curr_p = ext_pclk0_clk_src.c.parent = devm_clk_get(&pdev->dev,
								"pclk0_src");
	if (IS_ERR(curr_p)) {
		dev_err(&pdev->dev, "Failed to get pclk0 source.\n");
		return PTR_ERR(curr_p);
	}

	curr_p = ext_pclk1_clk_src.c.parent = devm_clk_get(&pdev->dev,
								"pclk1_src");
	if (IS_ERR(curr_p)) {
		dev_err(&pdev->dev, "Failed to get pclk1 source.\n");
		ret = PTR_ERR(curr_p);
		goto pclk1_fail;
	}

	curr_p = ext_byte0_clk_src.c.parent = devm_clk_get(&pdev->dev,
								"byte0_src");
	if (IS_ERR(curr_p)) {
		dev_err(&pdev->dev, "Failed to get byte0 source.\n");
		ret = PTR_ERR(curr_p);
		goto byte0_fail;
	}

	curr_p = ext_byte1_clk_src.c.parent = devm_clk_get(&pdev->dev,
								"byte1_src");
	if (IS_ERR(curr_p)) {
		dev_err(&pdev->dev, "Failed to get byte1 source.\n");
		ret = PTR_ERR(curr_p);
		goto byte1_fail;
	}

	ext_pclk0_clk_src.c.flags = CLKFLAG_NO_RATE_CACHE;
	ext_pclk1_clk_src.c.flags = CLKFLAG_NO_RATE_CACHE;
	ext_byte0_clk_src.c.flags = CLKFLAG_NO_RATE_CACHE;
	ext_byte1_clk_src.c.flags = CLKFLAG_NO_RATE_CACHE;

	ret = of_msm_clock_register(pdev->dev.of_node, msm_clocks_gcc_mdss,
					ARRAY_SIZE(msm_clocks_gcc_mdss));
	if (ret)
		goto fail;

	dev_info(&pdev->dev, "Registered GCC MDSS clocks.\n");

	return ret;
fail:
	devm_clk_put(&pdev->dev, ext_byte1_clk_src.c.parent);
byte1_fail:
	devm_clk_put(&pdev->dev, ext_byte0_clk_src.c.parent);
byte0_fail:
	devm_clk_put(&pdev->dev, ext_pclk1_clk_src.c.parent);
pclk1_fail:
	devm_clk_put(&pdev->dev, ext_pclk0_clk_src.c.parent);
	return ret;
}

static struct of_device_id msm_clock_mdss_match_table[] = {
	{ .compatible = "qcom,gcc-mdss-8976" },
	{ .compatible = "qcom,gcc-mdss-8976-v1" },
	{}
};

static struct platform_driver msm_clock_gcc_mdss_driver = {
	.probe = msm_gcc_mdss_probe,
	.driver = {
		.name = "gcc-mdss-8976",
		.of_match_table = msm_clock_mdss_match_table,
		.owner = THIS_MODULE,
	},
};

static int __init msm_gcc_mdss_init(void)
{
	return platform_driver_register(&msm_clock_gcc_mdss_driver);
}
fs_initcall_sync(msm_gcc_mdss_init);

/* GFX Clocks */
static struct clk_lookup msm_clocks_gcc_gfx[] = {
	CLK_LIST(gfx3d_clk_src),
	CLK_LIST(gcc_oxili_gfx3d_clk),
	CLK_LIST(gcc_bimc_gfx_clk),
	CLK_LIST(gcc_oxili_ahb_clk),
	CLK_LIST(gcc_oxili_aon_clk),
	CLK_LIST(gcc_oxili_gmem_clk),
	CLK_LIST(gcc_oxili_timer_clk),
	CLK_LIST(gcc_gfx_1_tbu_clk),
	CLK_LIST(gcc_gfx_tbu_clk),
	CLK_LIST(gcc_gfx_tcu_clk),
	CLK_LIST(gcc_gtcu_ahb_clk),
};

static int of_get_fmax_vdd_class(struct platform_device *pdev, struct clk *c,
								char *prop_name)
{
	struct device_node *of = pdev->dev.of_node;
	int prop_len, i;
	struct clk_vdd_class *vdd = c->vdd_class;
	u32 *array;

	if (!of_find_property(of, prop_name, &prop_len)) {
		dev_err(&pdev->dev, "missing %s\n", prop_name);
		return -EINVAL;
	}

	prop_len /= sizeof(u32);
	if (prop_len % 2) {
		dev_err(&pdev->dev, "bad length %d\n", prop_len);
		return -EINVAL;
	}

	prop_len /= 2;
	vdd->level_votes = devm_kzalloc(&pdev->dev,
				prop_len * sizeof(*vdd->level_votes),
					GFP_KERNEL);
	if (!vdd->level_votes)
		return -ENOMEM;

	vdd->vdd_uv = devm_kzalloc(&pdev->dev, prop_len * sizeof(int),
					GFP_KERNEL);
	if (!vdd->vdd_uv)
		return -ENOMEM;

	c->fmax = devm_kzalloc(&pdev->dev, prop_len * sizeof(unsigned long),
					GFP_KERNEL);
	if (!c->fmax)
		return -ENOMEM;

	array = devm_kzalloc(&pdev->dev,
			prop_len * sizeof(u32) * 2, GFP_KERNEL);
	if (!array)
		return -ENOMEM;

	of_property_read_u32_array(of, prop_name, array, prop_len * 2);
	for (i = 0; i < prop_len; i++) {
		c->fmax[i] = array[2 * i];
		vdd->vdd_uv[i] = array[2 * i + 1];
	}

	devm_kfree(&pdev->dev, array);
	vdd->num_levels = prop_len;
	vdd->cur_level = prop_len;
	c->num_fmax = prop_len;

	return 0;
}

static void print_opp_table(struct device *dev)
{
	struct clk *gpu_clk = &gfx3d_clk_src.c;
	struct dev_pm_opp *opp;
	int i;

	pr_debug("OPP table for GPU core clock:\n");
	for (i = 1; i < gpu_clk->num_fmax; i++) {
		if (!gpu_clk->fmax[i])
			continue;
		opp = dev_pm_opp_find_freq_exact(dev, gpu_clk->fmax[i], true);
		pr_info("clock-gpu: OPP voltage for %lu Hz: %ld uV\n",
			gpu_clk->fmax[i], dev_pm_opp_get_voltage(opp));
	}
}

static void populate_gpu_opp_table(struct platform_device *pdev)
{
	struct device_node *of = pdev->dev.of_node;
	struct platform_device *gpu_dev;
	struct device_node *gpu_node;
	struct clk *gpu_clk = &gfx3d_clk_src.c;
	int i, ret, level;
	unsigned long rate = 0;

	gpu_node = of_parse_phandle(of, "gpu_handle", 0);
	if (!gpu_node) {
		pr_err("clock-gpu: %s: Unable to get device_node pointer for GPU\n",
							__func__);
		return;
	}

	gpu_dev = of_find_device_by_node(gpu_node);
	if (!gpu_dev) {
		pr_err("clock-gpu: %s: Unable to find platform_device node for GPU\n",
							__func__);
		return;
	}

	for (i = 0; i < gpu_clk->num_fmax; i++) {
		if (!gpu_clk->fmax[i])
			continue;

		ret = clk_round_rate(gpu_clk, gpu_clk->fmax[i]);
		if (ret < 0) {
			pr_warn("clock-gpu: %s: round_rate failed at %lu - err: %d\n",
							__func__, rate, ret);
			return;
		}
		rate = ret;

		level = find_vdd_level(gpu_clk, rate);
		if (level <= 0) {
			pr_warn("no uv for %lu.\n", rate);
			return;
		}

		ret = dev_pm_opp_add(&gpu_dev->dev, rate,
				gpu_clk->vdd_class->vdd_uv[level]);
		if (ret) {
			pr_warn("clock-gpu: %s: couldn't add OPP for %lu - err: %d\n",
							__func__, rate, ret);
			return;
		}
	}

	print_opp_table(&gpu_dev->dev);
}

static int msm_gcc_gfx_probe(struct platform_device *pdev)
{
	struct resource *res;
	int ret;
	u32 regval;

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "cc_base");
	if (!res) {
		dev_err(&pdev->dev, "Register base not defined\n");
		return -ENOMEM;
	}

	virt_bases[GFX_BASE] = devm_ioremap(&pdev->dev, res->start,
							resource_size(res));
	if (!virt_bases[GFX_BASE]) {
		dev_err(&pdev->dev, "Failed to ioremap CC registers\n");
		return -ENOMEM;
	}

	vdd_gfx.regulator[0] = devm_regulator_get(&pdev->dev, "vdd_gfx");
	if (IS_ERR(vdd_gfx.regulator[0])) {
		if (PTR_ERR(vdd_gfx.regulator[0]) != -EPROBE_DEFER)
			dev_err(&pdev->dev, "Unable to get vdd_gfx regulator!");
		return PTR_ERR(vdd_gfx.regulator[0]);
	}

	ret = of_get_fmax_vdd_class(pdev, &gfx3d_clk_src.c,
					"qcom,gfxfreq-corner");
	if (ret) {
		dev_err(&pdev->dev, "Unable to get gfx freq-corner mapping info\n");
		return ret;
	}

	ret = of_msm_clock_register(pdev->dev.of_node, msm_clocks_gcc_gfx,
				ARRAY_SIZE(msm_clocks_gcc_gfx));

	/* Oxili Ocmem in GX rail: OXILI_GMEM_CLAMP_IO */
	regval = readl_relaxed(GCC_REG_BASE(GX_DOMAIN_MISC));
	regval &= ~BIT(0);
	writel_relaxed(regval, GCC_REG_BASE(GX_DOMAIN_MISC));

	/* Configure Sleep and Wakeup cycles for OXILI clock */
	regval = readl_relaxed(GCC_REG_BASE(OXILI_GFX3D_CBCR));
	regval &= ~0xF0;
	regval |= CLKFLAG_SLEEP_CYCLES << 4;
	writel_relaxed(regval, GCC_REG_BASE(OXILI_GFX3D_CBCR));

	dev_info(&pdev->dev, "Registered GCC GFX clocks.\n");

	populate_gpu_opp_table(pdev);

	return ret;
}

static struct of_device_id msm_clock_gfx_match_table[] = {
	{ .compatible = "qcom,gcc-gfx-8976" },
	{}
};

static struct platform_driver msm_clock_gcc_gfx_driver = {
	.probe = msm_gcc_gfx_probe,
	.driver = {
		.name = "gcc-gfx-8976",
		.of_match_table = msm_clock_gfx_match_table,
		.owner = THIS_MODULE,
	},
};

static int __init msm_gcc_gfx_init(void)
{
	return platform_driver_register(&msm_clock_gcc_gfx_driver);
}
arch_initcall_sync(msm_gcc_gfx_init);
