/* Copyright (c) 2015-2016, The Linux Foundation. All rights reserved.
 *
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

#ifndef MDSS_DSI_PLL_28HPM_H
#define MDSS_DSI_PLL_28HPM_H

#define DSI_PHY_PLL_UNIPHY_PLL_REFCLK_CFG	(0x0)
#define DSI_PHY_PLL_UNIPHY_PLL_POSTDIV1_CFG	(0x0004)
#define DSI_PHY_PLL_UNIPHY_PLL_CHGPUMP_CFG	(0x0008)
#define DSI_PHY_PLL_UNIPHY_PLL_VCOLPF_CFG	(0x000C)
#define DSI_PHY_PLL_UNIPHY_PLL_VREG_CFG		(0x0010)
#define DSI_PHY_PLL_UNIPHY_PLL_PWRGEN_CFG	(0x0014)
#define DSI_PHY_PLL_UNIPHY_PLL_POSTDIV2_CFG	(0x0024)
#define DSI_PHY_PLL_UNIPHY_PLL_POSTDIV3_CFG	(0x0028)
#define DSI_PHY_PLL_UNIPHY_PLL_LPFR_CFG		(0x002C)
#define DSI_PHY_PLL_UNIPHY_PLL_LPFC1_CFG	(0x0030)
#define DSI_PHY_PLL_UNIPHY_PLL_LPFC2_CFG	(0x0034)
#define DSI_PHY_PLL_UNIPHY_PLL_SDM_CFG0		(0x0038)
#define DSI_PHY_PLL_UNIPHY_PLL_SDM_CFG1		(0x003C)
#define DSI_PHY_PLL_UNIPHY_PLL_SDM_CFG2		(0x0040)
#define DSI_PHY_PLL_UNIPHY_PLL_SDM_CFG3		(0x0044)
#define DSI_PHY_PLL_UNIPHY_PLL_SDM_CFG4		(0x0048)
#define DSI_PHY_PLL_UNIPHY_PLL_SSC_CFG0		(0x004C)
#define DSI_PHY_PLL_UNIPHY_PLL_SSC_CFG1		(0x0050)
#define DSI_PHY_PLL_UNIPHY_PLL_SSC_CFG2		(0x0054)
#define DSI_PHY_PLL_UNIPHY_PLL_SSC_CFG3		(0x0058)
#define DSI_PHY_PLL_UNIPHY_PLL_CAL_CFG0		(0x006C)
#define DSI_PHY_PLL_UNIPHY_PLL_CAL_CFG2		(0x0074)
#define DSI_PHY_PLL_UNIPHY_PLL_CAL_CFG3		(0x0078)
#define DSI_PHY_PLL_UNIPHY_PLL_CAL_CFG4		(0x007C)
#define DSI_PHY_PLL_UNIPHY_PLL_CAL_CFG5		(0x0080)
#define DSI_PHY_PLL_UNIPHY_PLL_CAL_CFG6		(0x0084)
#define DSI_PHY_PLL_UNIPHY_PLL_CAL_CFG7		(0x0088)
#define DSI_PHY_PLL_UNIPHY_PLL_CAL_CFG8		(0x008C)
#define DSI_PHY_PLL_UNIPHY_PLL_CAL_CFG9		(0x0090)
#define DSI_PHY_PLL_UNIPHY_PLL_CAL_CFG10	(0x0094)
#define DSI_PHY_PLL_UNIPHY_PLL_CAL_CFG11	(0x0098)
#define DSI_PHY_PLL_UNIPHY_PLL_EFUSE_CFG	(0x009C)
#define DSI_PHY_PLL_UNIPHY_PLL_STATUS		(0x00C0)

#define DSI_PHY_PLL_UNIPHY_PLL_LKDET_CFG0	(0x005c)
#define DSI_PHY_PLL_UNIPHY_PLL_LKDET_CFG1	(0x0060)

struct ssc_params {
	s32 kdiv;
	s64 triang_inc_7_0;
	s64 triang_inc_9_8;
	s64 triang_steps;
	s64 dc_offset;
	s64 freq_seed_7_0;
	s64 freq_seed_15_8;
};

struct mdss_dsi_vco_calc {
	s64 sdm_cfg0;
	s64 sdm_cfg1;
	s64 sdm_cfg2;
	s64 sdm_cfg3;
	s64 cal_cfg10;
	s64 cal_cfg11;
	s64 refclk_cfg;
	s64 gen_vco_clk;
	u32 lpfr_lut_res;
	struct ssc_params ssc;
};

#endif /* MDSS_DSI_PLL_28HPM_H */

