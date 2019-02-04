/*
 * Linux mainline API clocks implementation for MSM8998
 * Graphics Processing Unit Clock Controller (GPUCC) driver
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

#include <dt-bindings/clock/qcom,gpucc-msm8998.h>

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

#define SW_COLLAPSE_MASK			BIT(0)
#define GPU_CX_GDSCR_OFFSET			0x1004
#define GPU_GX_GDSCR_OFFSET			0x1094
#define CRC_SID_FSM_OFFSET			0x10A0
#define CRC_MND_CFG_OFFSET			0x10A4
#define GPU_GX_BCR_OFFSET				0x1090

#define F(f, s, h, m, n) { (f), (s), (2 * (h) - 1), (m), (n) }
#define F_GFX(f, s, h, m, n, sf) { (f), (s), (2 * (h) - 1), (m), (n), (sf) }

static int vdd_gpucc_corner[] = {
	VDD_GFX_NONE,		/* OFF			*/
	VDD_GFX_MIN_SVS,	/* MIN:  MinSVS		*/
	VDD_GFX_LOW_SVS,	/* LOW:  LowSVS		*/
	VDD_GFX_SVS_MINUS,	/* LOW:  SVS-		*/
	VDD_GFX_SVS,		/* LOW:  SVS		*/
	VDD_GFX_SVS_PLUS,	/* LOW:  SVS+		*/
	VDD_GFX_NOMINAL,	/*       NOMINAL	*/
	VDD_GFX_TURBO,		/* HIGH: TURBO		*/
	VDD_GFX_TURBO_L1,	/* HIGH: TURBO_L1	*/
	VDD_GFX_SUPER_TURBO,	/* HIGH: SUPER_TURBO	*/
};

static DEFINE_VDD_REGULATORS(vdd_dig, VDD_DIG_NUM, 1, vdd_corner);
static DEFINE_VDD_REGULATORS(vdd_gpucc, VDD_GFX_MAX, 1, vdd_gpucc_corner);
static DEFINE_VDD_REGULATORS(vdd_gpucc_mx, VDD_MX_NUM, 1, vdd_corner);

enum {
	P_GPU_XO,
	P_GPLL0,
	P_GPU_CC_PLL0_OUT_EVEN,
	P_GPU_CC_PLL0_OUT_MAIN,
	P_GPU_CC_PLL0_OUT_ODD,
};

static const struct parent_map gpucc_parent_map_0[] = {
	{ P_GPU_XO, 0 },
	{ P_GPU_CC_PLL0_OUT_EVEN, 1 },
	{ P_GPU_CC_PLL0_OUT_ODD, 2 },
	//{ P_GPLL0, 5 },
};

static const char * const gpucc_parent_names_0[] = {
	"gpucc_xo",
	"gpucc_pll0_out_even",
	"gpucc_pll0_out_odd",
	//"gcc_gpu_gpll0_clk",
};

static const struct parent_map gpucc_parent_map_1[] = {
	{ P_GPU_XO, 0 },
	{ P_GPLL0, 5 },
};

static const char * const gpucc_parent_names_1[] = {
	"gpucc_xo",
	"gcc_gpu_gpll0_clk",
};

static struct clk_branch gpucc_xo = {
	.halt_reg = 0x01020,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "gpucc_xo",
		.parent_names = (const char*[]) {
			"cxo_a",
		},
		.num_parents = 1,
		.flags = CLK_ENABLE_HAND_OFF,
		.ops = &clk_branch2_ops,
	},
};

static struct pll_vco fabia_vco[] = {
	{ 249600000, 2000000000, 0 },
	{ 125000000, 1000000000, 1 },
};

/* Initial configuration for 360MHz */
static const struct alpha_pll_config gpu_pll0_config = {
	/*.config_ctl_val = 0x20485699 */
	.l = 0x12,
	.frac = 0xc00,
};

static struct clk_alpha_pll gpu_pll0_pll = {
	.offset = 0x0,
	.regs = clk_alpha_pll_regs[CLK_ALPHA_PLL_TYPE_FABIA],
	.vco_table = fabia_vco,
	.num_vco = ARRAY_SIZE(fabia_vco),
	.clkr.hw.init = &(struct clk_init_data) {
			.name = "gpu_cc_pll0",
			.parent_names = (const char *[]){ "gpucc_xo" },
			.num_parents = 1,
			.ops = &clk_alpha_pll_fabia_ops,
			VDD_GPU_MX_FMAX_MAP1(MIN, 1420000500),
	},
};

static struct clk_alpha_pll_postdiv gpu_pll0_out_even = {
	.offset = 0x0, /*0xC?*/
	.regs = clk_alpha_pll_regs[CLK_ALPHA_PLL_TYPE_FABIA],
	.width = 4,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "gpucc_pll0_out_even",
		.parent_names = (const char *[]){ "gpu_cc_pll0" },
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT | CLK_GET_RATE_NOCACHE,
		.ops = &clk_alpha_fabia_pll_postdiv_ops,
	},
};

static struct clk_alpha_pll_postdiv gpu_pll0_out_odd = {
	.offset = 0x0, /*0xC?*/
	.regs = clk_alpha_pll_regs[CLK_ALPHA_PLL_TYPE_FABIA],
	.width = 4,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "gpucc_pll0_out_odd",
		.parent_names = (const char *[]){ "gpu_cc_pll0" },
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT | CLK_GET_RATE_NOCACHE,
		.ops = &clk_alpha_fabia_pll_postdiv_odd_ops,
	},
};

#if 0 /* Should we go that way with clk_gfx3d_src_ops? */
static struct freq_tbl ftbl_gfx3d_clk_src[] = {
	F_GFX( 180000000, 0, 1, 0, 0,  360000000),
	F_GFX( 257000000, 0, 1, 0, 0,  514000000),
	F_GFX( 342000000, 0, 1, 0, 0,  684000000),
	F_GFX( 414000000, 0, 1, 0, 0,  828000000),
	F_GFX( 515000000, 0, 1, 0, 0, 1030000000),
	F_GFX( 596000000, 0, 1, 0, 0, 1192000000),
	F_GFX( 670000000, 0, 1, 0, 0, 1340000000),
	F_GFX( 710000000, 0, 1, 0, 0, 1420000000),
/*	F_GFX( 750000000, 0, 1, 0, 0, 1500000000),*/
	{ }
};
#endif

static struct freq_tbl ftbl_gfx3d_clk_src[] = {
	F(180000000, P_GPU_CC_PLL0_OUT_EVEN,  1, 0, 0),
	F(257000000, P_GPU_CC_PLL0_OUT_EVEN,  1, 0, 0),
	F(342000000, P_GPU_CC_PLL0_OUT_EVEN,  1, 0, 0),
	F(414000000, P_GPU_CC_PLL0_OUT_EVEN,  1, 0, 0),
	F(515000000, P_GPU_CC_PLL0_OUT_EVEN,  1, 0, 0),
	F(596000000, P_GPU_CC_PLL0_OUT_EVEN,  1, 0, 0),
	F(670000000, P_GPU_CC_PLL0_OUT_EVEN,  1, 0, 0),
	F(710000000, P_GPU_CC_PLL0_OUT_EVEN,  1, 0, 0),
	{ }
};

static struct clk_init_data gfx3d_clk_data = {
	.name = "gfx3d_clk_src",
	.parent_names = gpucc_parent_names_0,
	.num_parents = ARRAY_SIZE(gpucc_parent_names_0),
	.ops = &clk_rcg2_ops,
	.flags = CLK_SET_RATE_PARENT,
	VDD_GFX_FMAX_MAP8(MIN_SVS,  180000000,
			  LOW_SVS,  257000000,
			  SVS_MINUS,342000000,
			  SVS,      414000000,
			  SVS_PLUS, 515000000,
			  NOMINAL,  596000000,
			  TURBO,    670000000,
			  TURBO_L1, 710000000),
};

static struct clk_rcg2 gfx3d_clk_src = {
	.cmd_rcgr = 0x01070,
	.hid_width = 5,
	.parent_map = gpucc_parent_map_0,
	.freq_tbl = ftbl_gfx3d_clk_src,
	.flags = FORCE_ENABLE_RCG,
	.clkr.hw.init = &gfx3d_clk_data,
};

static struct freq_tbl ftbl_rbbmtimer_clk_src[] = {
	F( 19200000,  P_GPU_XO,    1,    0,     0),
	{ }
};

static struct clk_rcg2 rbbmtimer_clk_src = {
	.cmd_rcgr = 0x010B0,
	.hid_width = 5,
	.parent_map = gpucc_parent_map_1,
	.freq_tbl = ftbl_rbbmtimer_clk_src,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "rbbmtimer_clk_src",
		.parent_names = gpucc_parent_names_1,
		.num_parents = ARRAY_SIZE(gpucc_parent_names_1),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP1(MIN, 19200000),
	},
};

static struct freq_tbl ftbl_gfx3d_isense_clk_src[] = {
	F(  19200000,  P_GPU_XO,    1,    0,     0),
	F(  40000000,   P_GPLL0,   15,    0,     0),
	F( 200000000,   P_GPLL0,    3,    0,     0),
	F( 300000000,   P_GPLL0,    2,    0,     0),
	{ }
};

static struct clk_rcg2 gfx3d_isense_clk_src = {
	.cmd_rcgr = 0x01100,
	.hid_width = 5,
	.parent_map = gpucc_parent_map_1,
	.freq_tbl = ftbl_gfx3d_isense_clk_src,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "gfx3d_isense_clk_src",
		.parent_names = gpucc_parent_names_1,
		.num_parents = ARRAY_SIZE(gpucc_parent_names_1),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP4(MIN, 19200000, LOWER, 40000000,
				LOW, 200000000, HIGH, 300000000),
	},
};

static struct freq_tbl ftbl_rbcpr_clk_src[] = {
	F( 19200000,  P_GPU_XO,    1,    0,     0),
	F( 50000000,   P_GPLL0,   12,    0,     0),
	{ }
};

static struct clk_rcg2 rbcpr_clk_src = {
	.cmd_rcgr = 0x01030,
	.hid_width = 5,
	.parent_map = gpucc_parent_map_1,
	.freq_tbl = ftbl_rbcpr_clk_src,
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "rbcpr_clk_src",
		.parent_names = gpucc_parent_names_1,
		.num_parents = ARRAY_SIZE(gpucc_parent_names_1),
		.ops = &clk_rcg2_ops,
		VDD_DIG_FMAX_MAP2(MIN, 19200000, NOMINAL, 50000000),
	},
};

static struct clk_branch gpucc_gfx3d_clk = {
	.halt_reg = 0x1098,
	.clkr = {
		.enable_reg = 0x1098,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gpucc_gfx3d_clk",
			.parent_names = (const char*[]) {
				"gfx3d_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
			VDD_GPU_MX_FMAX_MAP3(LOW, 414000000,
					     NOMINAL, 596000000,
					     HIGH, 710000000),
		},
	}
};

static struct clk_branch gpucc_rbbmtimer_clk = {
	.halt_reg = 0x10D0,
	.clkr = {
		.enable_reg = 0x10D0,
		.enable_mask = BIT(0),
			.hw.init = &(struct clk_init_data) {
			.name = "gpucc_rbbmtimer_clk",
			.parent_names = (const char*[]) {
				"rbbmtimer_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gpucc_gfx3d_isense_clk = {
	.halt_reg = 0x1124,
	.clkr = {
		.enable_reg = 0x1124,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gpucc_gfx3d_isense_clk",
			.parent_names = (const char*[]) {
				"gfx3d_isense_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gpucc_rbcpr_clk = {
	.halt_reg = 0x01054,
	.clkr = {
		.enable_reg = 0x01054,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data) {
			.name = "gpucc_rbcpr_clk",
			.parent_names = (const char*[]) {
				"rbcpr_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

#if 0
static struct mux_clk gpucc_gcc_dbg_clk = {
	.ops = &mux_reg_ops,
	.en_mask = BIT(16),
	.mask = 0x3FF,
	.offset = GPUCC_DEBUG_CLK_CTL,
	.en_offset = GPUCC_DEBUG_CLK_CTL,
	MUX_SRC_LIST(
		{ &gpucc_rbcpr_clk.c, 0x0003 },
		{ &gpucc_rbbmtimer_clk.c, 0x0005 },
		{ &gpucc_gfx3d_isense_clk.c, 0x000a },
	),
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "gpucc_gcc_dbg_clk",
		.ops = &clk_ops_gen_mux,
		.flags = CLKFLAG_NO_RATE_CACHE,
	},
};
#endif

static void __iomem *virt_base_gfx;
static void enable_gfx_crc(void)
{
	u32 regval;

	/* Turn on the GPU_CX GDSC */
	regval = readl_relaxed(virt_base_gfx + GPU_CX_GDSCR_OFFSET);
	regval &= ~SW_COLLAPSE_MASK;
	writel_relaxed(regval, virt_base_gfx + GPU_CX_GDSCR_OFFSET);

	/* Wait for 10usecs to let the GDSC turn ON */
	mb();
	udelay(10);

	/* Turn on the Graphics rail */
	if (regulator_enable(vdd_gpucc.regulator[0]))
		pr_warn("Failed to enable VDD_GPUCC during CRC sequence!\n");
	/* Turn on the GPU_GX GDSC */
	writel_relaxed(0x1, virt_base_gfx + GPU_GX_BCR_OFFSET);

	/*
	 * BLK_ARES should be kept asserted for 1us before being de-asserted.
	 */
	wmb();
	udelay(1);
	writel_relaxed(0x0, virt_base_gfx + GPU_GX_BCR_OFFSET);
	regval = readl_relaxed(virt_base_gfx + 0x130);
	regval |= BIT(4);
	writel_relaxed(regval, virt_base_gfx + 0x130);

	/* Keep reset asserted for at-least 1us before continuing. */
	wmb();
	udelay(1);
	regval &= ~BIT(4);
	writel_relaxed(regval, virt_base_gfx + 0x130);

	/* Make sure GMEM_RESET is de-asserted before continuing. */
	wmb();
	regval &= ~BIT(0);
	writel_relaxed(regval, virt_base_gfx + 0x130);

	/* All previous writes should be done at this point */
	wmb();
	regval = readl_relaxed(virt_base_gfx + GPU_GX_GDSCR_OFFSET);
	regval &= ~SW_COLLAPSE_MASK;
	writel_relaxed(regval, virt_base_gfx + GPU_GX_GDSCR_OFFSET);

	/* Wait for 10usecs to let the GDSC turn ON */
	mb();
	udelay(10);

	/* Enable the graphics clock */
	clk_prepare_enable(gpucc_gfx3d_clk.clkr.hw.clk);

	/* Enabling MND RC in Bypass mode */
	writel_relaxed(0x00015010, virt_base_gfx + CRC_MND_CFG_OFFSET);
	writel_relaxed(0x00800000, virt_base_gfx + CRC_SID_FSM_OFFSET);

	/* Wait for 32 cycles before continuing */
	udelay(2);
	//clk_set_rate(gpucc_gfx3d_clk.clkr.hw.clk, 670000000);

	/* Disable the graphics clock */
	clk_disable_unprepare(gpucc_gfx3d_clk.clkr.hw.clk);

	/* Turn off the gpu_cx and gpu_gx GDSCs */
	regval = readl_relaxed(virt_base_gfx + GPU_GX_GDSCR_OFFSET);
	regval |= SW_COLLAPSE_MASK;
	writel_relaxed(regval, virt_base_gfx + GPU_GX_GDSCR_OFFSET);

	/* Write to disable GX GDSC should go through before continuing */
	wmb();
	regval = readl_relaxed(virt_base_gfx + 0x130);
	regval |= BIT(0);
	writel_relaxed(regval, virt_base_gfx + 0x130);

	/* Make sure GMEM_CLAMP_IO is asserted before continuing. */
	wmb();
	regulator_disable(vdd_gpucc.regulator[0]);
	regval = readl_relaxed(virt_base_gfx + GPU_CX_GDSCR_OFFSET);
	regval |= SW_COLLAPSE_MASK;
	writel_relaxed(regval, virt_base_gfx + GPU_CX_GDSCR_OFFSET);
}

/*
static struct mux_clk gfxcc_dbg_clk = {
	.ops = &mux_reg_ops,
	.en_mask = BIT(16),
	.mask = 0x3FF,
	.offset = GPUCC_DEBUG_CLK_CTL,
	.en_offset = GPUCC_DEBUG_CLK_CTL,
	MUX_SRC_LIST(
		{ &gpucc_gfx3d_clk.c, 0x0008 },
	),
	.clkr.hw.init = &(struct clk_init_data) {
		.name = "gfxcc_dbg_clk",
		.ops = &clk_ops_gen_mux,
		.flags = CLKFLAG_NO_RATE_CACHE,
	},
};
*/

static struct clk_regmap *gpucc_msm8998_clocks[] = {
	[GPU_PLL0_PLL] = &gpu_pll0_pll.clkr,
	[GPU_PLL0_PLL_OUT_EVEN] = &gpu_pll0_out_even.clkr,
	[GPU_PLL0_PLL_OUT_ODD] = &gpu_pll0_out_odd.clkr,
	[GFX3D_CLK_SRC] = &gfx3d_clk_src.clkr,
	[RBBMTIMER_CLK_SRC] = &rbbmtimer_clk_src.clkr,
	[GFX3D_ISENSE_CLK_SRC] = &gfx3d_isense_clk_src.clkr,
	[GPUCC_RBBMTIMER_CLK] = &gpucc_rbbmtimer_clk.clkr,
	[GPUCC_GFX3D_ISENSE_CLK] = &gpucc_gfx3d_isense_clk.clkr,
	[GPUCC_GFX3D_CLK] = &gpucc_gfx3d_clk.clkr,
	//[GPUCC_DBG_CLK] = &gfxcc_dbg_clk.clkr,
	//[GPUCC_GCC_DBG_CLK] = &gpucc_gcc_dbg_clk.clkr,
};

static struct clk_regmap *gpucc_msm8998_early_clocks[] = {
	[GPUCC_XO] = &gpucc_xo.clkr,
	[RBCPR_CLK_SRC] = &rbcpr_clk_src.clkr,
	[GPUCC_RBCPR_CLK] = &gpucc_rbcpr_clk.clkr,
};

static const struct regmap_config gpucc_msm8998_regmap_config = {
	.reg_bits	= 32,
	.reg_stride	= 4,
	.val_bits	= 32,
	.max_register	= 0x9034,
	.fast_io	= true,
};

static const struct qcom_cc_desc gpucc_msm8998_desc = {
	.config = &gpucc_msm8998_regmap_config,
	.clks = gpucc_msm8998_clocks,
	.num_clks = ARRAY_SIZE(gpucc_msm8998_clocks),
};

int gpucc_msm8998_probe(struct platform_device *pdev)
{
	struct resource *res;
	struct regmap *regmap;
	struct clk *tmp;
	int rc;
	u32 regval;

	tmp = devm_clk_get(&pdev->dev, "gpll0");
	if (IS_ERR(tmp)) {
		if (PTR_ERR(tmp) != -EPROBE_DEFER)
			dev_err(&pdev->dev,
				"The GPLL0 clock cannot be found.\n");
		return PTR_ERR(tmp);
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(&pdev->dev, "Unable to retrieve register base\n");
		return -ENOMEM;
	}

	virt_base_gfx = devm_ioremap(&pdev->dev, res->start,
					resource_size(res));
	if (IS_ERR(virt_base_gfx)) {
		dev_err(&pdev->dev, "Unable to map GFX3D clock controller.\n");
		return -EINVAL;
	}

	regmap = devm_regmap_init_mmio(&pdev->dev, virt_base_gfx,
					gpucc_msm8998_desc.config);
	if (IS_ERR(regmap)) {
		dev_err(&pdev->dev, "Failed to init regmap\n");
		return PTR_ERR(regmap);
	}
/*
	regmap = qcom_cc_map(pdev, &gpu_cc_sdm845_desc);
	if (IS_ERR(regmap))
		return PTR_ERR(regmap);
*/
	vdd_dig.regulator[0] = devm_regulator_get(&pdev->dev, "vdd_dig");
	if (IS_ERR(vdd_dig.regulator[0])) {
		if (PTR_ERR(vdd_dig.regulator[0]) != -EPROBE_DEFER)
			dev_err(&pdev->dev,
				"Unable to get vdd_dig regulator\n");
		return PTR_ERR(vdd_dig.regulator[0]);
	}

	vdd_gpucc.regulator[0] = devm_regulator_get(&pdev->dev, "vdd_gpucc");
	if (IS_ERR(vdd_gpucc.regulator[0])) {
		if (PTR_ERR(vdd_gpucc.regulator[0]) != -EPROBE_DEFER)
			dev_err(&pdev->dev,
				"Unable to get vdd_gpucc regulator\n");
		return PTR_ERR(vdd_gpucc.regulator[0]);
	}

	vdd_gpucc_mx.regulator[0] = devm_regulator_get(&pdev->dev, "vdd_mx");
	if (IS_ERR(vdd_gpucc_mx.regulator[0])) {
		if (PTR_ERR(vdd_gpucc_mx.regulator[0]) != -EPROBE_DEFER)
			dev_err(&pdev->dev,
				"Unable to get vdd_gpu_mx regulator\n");
		return PTR_ERR(vdd_gpucc_mx.regulator[0]);
	}

	/* Clear the DBG_CLK_DIV bits of the GPU debug register */
	regmap_update_bits(regmap, 0x120, (3 << 17), 0);

	clk_fabia_pll_configure(&gpu_pll0_pll, regmap, &gpu_pll0_config);

	rc = qcom_cc_really_probe(pdev, &gpucc_msm8998_desc, regmap);
	if (rc) {
		dev_err(&pdev->dev, "Failed to register GPUCC clocks\n");
		return rc;
	}

	enable_gfx_crc();

	/*
	 * Force periph logic to be ON since after NAP, the value of the perf
	 * counter might be corrupted frequently.
	 */
	clk_set_flags(gpucc_gfx3d_clk.clkr.hw.clk, CLKFLAG_RETAIN_PERIPH);

	/*
	 * Program the droop detector's gfx_pdn to 1'b1 in order to reduce
	 * leakage between the graphics and CX rails.
	 */
	regval = readl_relaxed(virt_base_gfx + 0x430);
	regval |= BIT(0);
	writel_relaxed(regval, virt_base_gfx + 0x430);

	dev_info(&pdev->dev, "Registered GPUCC clocks\n");
	return 0;
}

static const struct of_device_id gpucc_msm8998_match_table[] = {
	{ .compatible = "qcom,gpucc-msm8998" },
	{},
};

static struct platform_driver gpucc_msm8998_driver = {
	.probe = gpucc_msm8998_probe,
	.driver = {
		.name = "qcom,gpucc-msm8998",
		.of_match_table = gpucc_msm8998_match_table,
		.owner = THIS_MODULE,
	},
};

int __init gpucc_msm8998_init(void)
{
	return platform_driver_register(&gpucc_msm8998_driver);
}
arch_initcall(gpucc_msm8998_init);

static const struct qcom_cc_desc gpucc_early_msm8998_desc = {
	.config = &gpucc_msm8998_regmap_config,
	.clks = gpucc_msm8998_early_clocks,
	.num_clks = ARRAY_SIZE(gpucc_msm8998_early_clocks),
};

int gpucc_early_msm8998_probe(struct platform_device *pdev)
{
	struct resource *res;
	struct regmap *regmap;
	struct clk *tmp;
	int rc;

	tmp = devm_clk_get(&pdev->dev, "gpll0");
	if (IS_ERR(tmp)) {
		if (PTR_ERR(tmp) != -EPROBE_DEFER)
			dev_err(&pdev->dev,
				"The GPLL0 clock cannot be found.\n");
		return PTR_ERR(tmp);
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(&pdev->dev, "Unable to retrieve register base\n");
		return -ENOMEM;
	}

	virt_base_gfx = devm_ioremap(&pdev->dev, res->start,
					resource_size(res));
	if (IS_ERR(virt_base_gfx)) {
		dev_err(&pdev->dev, "Unable to map GFX3D clock controller.\n");
		return -EINVAL;
	}

	regmap = devm_regmap_init_mmio(&pdev->dev, virt_base_gfx,
					gpucc_early_msm8998_desc.config);
	if (IS_ERR(regmap)) {
		dev_err(&pdev->dev, "Failed to init regmap\n");
		return PTR_ERR(regmap);
	}

	vdd_dig.regulator[0] = devm_regulator_get(&pdev->dev, "vdd_dig");
	if (IS_ERR(vdd_dig.regulator[0])) {
		if (PTR_ERR(vdd_dig.regulator[0]) != -EPROBE_DEFER)
			dev_err(&pdev->dev,
				"Unable to get vdd_dig regulator\n");
		return PTR_ERR(vdd_dig.regulator[0]);
	}

	rc = qcom_cc_really_probe(pdev, &gpucc_early_msm8998_desc, regmap);
	if (rc) {
		dev_err(&pdev->dev, "Failed to register GPUCC clocks\n");
		return rc;
	}

	/* Set the rate for GPU XO to make the clk API happy */
	clk_set_rate(gpucc_xo.clkr.hw.clk, 19200000);

	/*
	 * gpucc_xo works as the root clock for all GPUCC RCGs and GDSCs.
	 *  Keep it enabled always.
	 */
	clk_prepare_enable(gpucc_xo.clkr.hw.clk);

	dev_info(&pdev->dev, "Registered early GPUCC clocks\n");
	return 0;
}

static const struct of_device_id gpucc_early_msm8998_match_table[] = {
	{ .compatible = "qcom,gpucc-early-msm8998" },
	{},
};

static struct platform_driver gpucc_early_msm8998_driver = {
	.probe = gpucc_early_msm8998_probe,
	.driver = {
		.name = "qcom,gpucc-early-msm8998",
		.of_match_table = gpucc_early_msm8998_match_table,
		.owner = THIS_MODULE,
	},
};

int __init gpucc_early_msm8998_init(void)
{
	return platform_driver_register(&gpucc_early_msm8998_driver);
}
arch_initcall(gpucc_early_msm8998_init);
