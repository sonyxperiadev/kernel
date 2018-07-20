/* Copyright (c) 2012-2015, The Linux Foundation. All rights reserved.
 *
 * MDSS DSI PLL 28HPM Mainline clock API fixes/reimplementation
 * Copyright (C) 2018 AngeloGioacchino Del Regno <kholk11@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#define pr_fmt(fmt)	"%s: " fmt, __func__

#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <linux/iopoll.h>

#include <dt-bindings/clock/mdss-pll-clk.h>

#include "mdss-pll.h"
#include "mdss-dsi-pll.h"
#include "mdss-dsi-pll-28hpm.h"


#define DSI_PLL_POLL_DELAY_US			50
#define DSI_PLL_POLL_TIMEOUT_US			500

#define VCO_DELAY_USEC		1


extern struct clk_ops clk_dummy_ops;

static struct lpfr_cfg lpfr_lut_struct[] = {
	{479500000, 8},
	{480000000, 11},
	{575500000, 8},
	{576000000, 12},
	{610500000, 8},
	{659500000, 9},
	{671500000, 10},
	{672000000, 14},
	{708500000, 10},
	{750000000, 11},
};

static struct regmap_config dsi_pll_28hpm_config = {
	.reg_bits	= 32,
	.reg_stride	= 4,
	.val_bits	= 32,
	.max_register	= 0xd4,
};

static void dsi_pll_software_reset(struct mdss_pll_resources *dsi_pll_res)
{
	/*
	 * Add HW recommended delays after toggling the software
	 * reset bit off and back on.
	 */
	MDSS_PLL_REG_W(dsi_pll_res->pll_base,
					DSI_PHY_PLL_UNIPHY_PLL_TEST_CFG, 0x01);
	udelay(1);
	MDSS_PLL_REG_W(dsi_pll_res->pll_base,
					DSI_PHY_PLL_UNIPHY_PLL_TEST_CFG, 0x00);
	udelay(1);
}

static int dsi_pll_enable(struct clk_hw *hw)
{
	int i, rc;
	struct dsi_pll_vco_clk *vco = to_vco_clk_hw(hw);
	struct mdss_pll_resources *dsi_pll_res = vco->priv;

	rc = mdss_pll_resource_enable(dsi_pll_res, true);
	if (rc) {
		pr_err("Failed to enable mdss dsi pll resources\n");
		return rc;
	}

	/* Try all enable sequences until one succeeds */
	for (i = 0; i < vco->pll_en_seq_cnt; i++) {
		rc = vco->pll_enable_seqs[i](dsi_pll_res);
		if (!rc)
			break;
	}

	if (rc) {
		mdss_pll_resource_enable(dsi_pll_res, false);
		pr_err("DSI PLL failed to lock\n");
	}
	dsi_pll_res->pll_on = true;

	return rc;
}

static void dsi_pll_disable(struct clk_hw *hw)
{
	struct dsi_pll_vco_clk *vco = to_vco_clk_hw(hw);
	struct mdss_pll_resources *dsi_pll_res = vco->priv;

	if (!dsi_pll_res->pll_on &&
		mdss_pll_resource_enable(dsi_pll_res, true)) {
		pr_err("Failed to enable mdss dsi pll resources\n");
		return;
	}

	dsi_pll_res->handoff_resources = false;

	MDSS_PLL_REG_W(dsi_pll_res->pll_base,
				DSI_PHY_PLL_UNIPHY_PLL_GLB_CFG, 0x00);

	mdss_pll_resource_enable(dsi_pll_res, false);
	dsi_pll_res->pll_on = false;
}

static int pll_28nm_vco_rate_calc(struct dsi_pll_vco_clk *vco,
		struct mdss_dsi_vco_calc *vco_calc, unsigned long vco_clk_rate)
{
	s32 rem;
	s64 frac_n_mode, ref_doubler_en_b;
	s64 ref_clk_to_pll, div_fb, frac_n_value;
	int i;

	/* Configure the Loop filter resistance */
	for (i = 0; i < vco->lpfr_lut_size; i++)
		if (vco_clk_rate <= vco->lpfr_lut[i].vco_rate)
			break;
	if (i == vco->lpfr_lut_size) {
		pr_err("unable to get loop filter resistance. vco=%ld\n",
			vco_clk_rate);
		return -EINVAL;
	}
	vco_calc->lpfr_lut_res = vco->lpfr_lut[i].r;

	div_s64_rem(vco_clk_rate, vco->ref_clk_rate, &rem);
	if (rem) {
		vco_calc->refclk_cfg = 0x1;
		frac_n_mode = 1;
		ref_doubler_en_b = 0;
	} else {
		vco_calc->refclk_cfg = 0x0;
		frac_n_mode = 0;
		ref_doubler_en_b = 1;
	}

	ref_clk_to_pll = ((vco->ref_clk_rate * 2 * (vco_calc->refclk_cfg))
			  + (ref_doubler_en_b * vco->ref_clk_rate));

	div_fb = div_s64_rem(vco_clk_rate, ref_clk_to_pll, &rem);
	frac_n_value = div_s64(((s64)rem * (1 << 16)), ref_clk_to_pll);
	vco_calc->gen_vco_clk = vco_clk_rate;

	rem = 0;
	if (frac_n_mode) {
		vco_calc->sdm_cfg0 = 0;
		vco_calc->sdm_cfg1 = (div_fb & 0x3f) - 1;
		vco_calc->sdm_cfg3 = div_s64_rem(frac_n_value, 256, &rem);
		vco_calc->sdm_cfg2 = rem;
	} else {
		vco_calc->sdm_cfg0 = (0x1 << 5);
		vco_calc->sdm_cfg0 |= (div_fb & 0x3f) - 1;
		vco_calc->sdm_cfg1 = 0;
		vco_calc->sdm_cfg2 = 0;
		vco_calc->sdm_cfg3 = 0;
	}

	vco_calc->cal_cfg11 = div_s64_rem(vco_calc->gen_vco_clk,
			256 * 1000000, &rem);
	vco_calc->cal_cfg10 = rem / 1000000;

	return 0;
}

static void pll_28nm_ssc_param_calc(struct dsi_pll_vco_clk *vco,
		struct mdss_dsi_vco_calc *vco_calc)
{
	struct mdss_pll_resources *dsi_pll_res = vco->priv;
	s64 ppm_freq, incr, spread_freq, div_rf, frac_n_value;
	s32 rem;

	if (!dsi_pll_res->ssc_en) {
		return;
	}

	vco_calc->ssc.kdiv = DIV_ROUND_CLOSEST(vco->ref_clk_rate,
			1000000) - 1;
	vco_calc->ssc.triang_steps = DIV_ROUND_CLOSEST(vco->ref_clk_rate,
			dsi_pll_res->ssc_freq * (vco_calc->ssc.kdiv + 1));
	ppm_freq = div_s64(vco_calc->gen_vco_clk * dsi_pll_res->ssc_ppm,
			1000000);
	incr = div64_s64(ppm_freq * 65536, vco->ref_clk_rate * 2 *
			vco_calc->ssc.triang_steps);

	vco_calc->ssc.triang_inc_7_0 = incr & 0xff;
	vco_calc->ssc.triang_inc_9_8 = (incr >> 8) & 0x3;

	if (!dsi_pll_res->ssc_center)
		spread_freq = vco_calc->gen_vco_clk - ppm_freq;
	else
		spread_freq = vco_calc->gen_vco_clk - (ppm_freq / 2);

	div_rf = div_s64(spread_freq, 2 * vco->ref_clk_rate);
	vco_calc->ssc.dc_offset = (div_rf - 1);

	div_s64_rem(spread_freq, 2 * vco->ref_clk_rate, &rem);
	frac_n_value = div_s64((s64)rem * 65536, 2 * vco->ref_clk_rate);

	vco_calc->ssc.freq_seed_7_0 = frac_n_value & 0xff;
	vco_calc->ssc.freq_seed_15_8 = (frac_n_value >> 8) & 0xff;
}

static void pll_28nm_vco_config(void __iomem *pll_base,
		struct mdss_dsi_vco_calc *vco_calc,
		u32 vco_delay_us, bool ssc_en)
{

	MDSS_PLL_REG_W(pll_base, DSI_PHY_PLL_UNIPHY_PLL_LPFR_CFG,
		vco_calc->lpfr_lut_res);

	/* Loop filter capacitance values : c1 and c2 */
	MDSS_PLL_REG_W(pll_base, DSI_PHY_PLL_UNIPHY_PLL_LPFC1_CFG, 0x70);
	MDSS_PLL_REG_W(pll_base, DSI_PHY_PLL_UNIPHY_PLL_LPFC2_CFG, 0x15);

	MDSS_PLL_REG_W(pll_base, DSI_PHY_PLL_UNIPHY_PLL_CHGPUMP_CFG, 0x02);
	MDSS_PLL_REG_W(pll_base, DSI_PHY_PLL_UNIPHY_PLL_CAL_CFG3, 0x2b);
	MDSS_PLL_REG_W(pll_base, DSI_PHY_PLL_UNIPHY_PLL_CAL_CFG4, 0x66);
	MDSS_PLL_REG_W(pll_base, DSI_PHY_PLL_UNIPHY_PLL_LKDET_CFG2, 0x0d);

	if (!ssc_en) {
		MDSS_PLL_REG_W(pll_base, DSI_PHY_PLL_UNIPHY_PLL_SDM_CFG1,
			(u32)(vco_calc->sdm_cfg1 & 0xff));
		MDSS_PLL_REG_W(pll_base, DSI_PHY_PLL_UNIPHY_PLL_SDM_CFG2,
			(u32)(vco_calc->sdm_cfg2 & 0xff));
		MDSS_PLL_REG_W(pll_base, DSI_PHY_PLL_UNIPHY_PLL_SDM_CFG3,
			(u32)(vco_calc->sdm_cfg3 & 0xff));
	} else {
		MDSS_PLL_REG_W(pll_base, DSI_PHY_PLL_UNIPHY_PLL_SDM_CFG1,
			(u32)vco_calc->ssc.dc_offset);
		MDSS_PLL_REG_W(pll_base, DSI_PHY_PLL_UNIPHY_PLL_SDM_CFG2,
			(u32)vco_calc->ssc.freq_seed_7_0);
		MDSS_PLL_REG_W(pll_base, DSI_PHY_PLL_UNIPHY_PLL_SDM_CFG3,
			(u32)vco_calc->ssc.freq_seed_15_8);
		MDSS_PLL_REG_W(pll_base, DSI_PHY_PLL_UNIPHY_PLL_SSC_CFG0,
			(u32)vco_calc->ssc.kdiv);
		MDSS_PLL_REG_W(pll_base, DSI_PHY_PLL_UNIPHY_PLL_SSC_CFG1,
			(u32)vco_calc->ssc.triang_inc_7_0);
		MDSS_PLL_REG_W(pll_base, DSI_PHY_PLL_UNIPHY_PLL_SSC_CFG2,
			(u32)vco_calc->ssc.triang_inc_9_8);
		MDSS_PLL_REG_W(pll_base, DSI_PHY_PLL_UNIPHY_PLL_SSC_CFG3,
			(u32)vco_calc->ssc.triang_steps);
	}
	MDSS_PLL_REG_W(pll_base, DSI_PHY_PLL_UNIPHY_PLL_SDM_CFG4, 0x00);

	/* Add hardware recommended delay for correct PLL configuration */
	if (vco_delay_us)
		udelay(vco_delay_us);

	MDSS_PLL_REG_W(pll_base, DSI_PHY_PLL_UNIPHY_PLL_REFCLK_CFG,
		(u32)vco_calc->refclk_cfg);
	MDSS_PLL_REG_W(pll_base, DSI_PHY_PLL_UNIPHY_PLL_PWRGEN_CFG, 0x00);
	MDSS_PLL_REG_W(pll_base, DSI_PHY_PLL_UNIPHY_PLL_VCOLPF_CFG, 0x71);
	MDSS_PLL_REG_W(pll_base, DSI_PHY_PLL_UNIPHY_PLL_SDM_CFG0,
		(u32)vco_calc->sdm_cfg0);
	MDSS_PLL_REG_W(pll_base, DSI_PHY_PLL_UNIPHY_PLL_CAL_CFG0, 0x12);
	MDSS_PLL_REG_W(pll_base, DSI_PHY_PLL_UNIPHY_PLL_CAL_CFG6, 0x30);
	MDSS_PLL_REG_W(pll_base, DSI_PHY_PLL_UNIPHY_PLL_CAL_CFG7, 0x00);
	MDSS_PLL_REG_W(pll_base, DSI_PHY_PLL_UNIPHY_PLL_CAL_CFG8, 0x60);
	MDSS_PLL_REG_W(pll_base, DSI_PHY_PLL_UNIPHY_PLL_CAL_CFG9, 0x00);
	MDSS_PLL_REG_W(pll_base, DSI_PHY_PLL_UNIPHY_PLL_CAL_CFG10,
		(u32)(vco_calc->cal_cfg10 & 0xff));
	MDSS_PLL_REG_W(pll_base, DSI_PHY_PLL_UNIPHY_PLL_CAL_CFG11,
		(u32)(vco_calc->cal_cfg11 & 0xff));
	MDSS_PLL_REG_W(pll_base, DSI_PHY_PLL_UNIPHY_PLL_EFUSE_CFG, 0x20);
}

int vco_set_rate(struct dsi_pll_vco_clk *vco, unsigned long rate)
{
	struct mdss_dsi_vco_calc vco_calc;
	struct mdss_pll_resources *dsi_pll_res = vco->priv;
	int rc = 0;

	rc = pll_28nm_vco_rate_calc(vco, &vco_calc, rate);
	if (rc) {
		pr_err("vco rate calculation failed\n");
		return rc;
	}

	pll_28nm_ssc_param_calc(vco, &vco_calc);
	pll_28nm_vco_config(dsi_pll_res->pll_base, &vco_calc,
		dsi_pll_res->vco_delay, dsi_pll_res->ssc_en);

	return 0;
}

static int vco_set_rate_hpm(struct clk_hw *hw, unsigned long rate,
		unsigned long parent_rate)
{
	int rc;
	struct dsi_pll_vco_clk *vco = to_vco_clk_hw(hw);
	struct mdss_pll_resources *dsi_pll_res = vco->priv;

	rc = mdss_pll_resource_enable(dsi_pll_res, true);
	if (rc) {
		pr_err("Failed to enable mdss dsi pll resources\n");
		return rc;
	}

	dsi_pll_software_reset(dsi_pll_res);

	MDSS_PLL_REG_W(dsi_pll_res->pll_base,
				DSI_PHY_PLL_UNIPHY_PLL_POSTDIV2_CFG, 3);

	rc = vco_set_rate(vco, rate);

	//dsi_pll_res->vco_current_rate = rate;
	//dsi_pll_res->vco_ref_clk_rate = vco->ref_clk_rate;

	mdss_pll_resource_enable(dsi_pll_res, false);
	return rc;
}

unsigned long vco_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
	u32 sdm0, doubler, sdm_byp_div;
	u64 vco_rate;
	u32 sdm_dc_off, sdm_freq_seed, sdm2, sdm3;
	struct dsi_pll_vco_clk *vco = to_vco_clk_hw(hw);
	u64 ref_clk = vco->ref_clk_rate;
	int rc;
	struct mdss_pll_resources *dsi_pll_res = vco->priv;
/*
	if (dsi_pll_res->vco_current_rate)
		return (unsigned long)dsi_pll_res->vco_current_rate;
*/
	if (is_gdsc_disabled(dsi_pll_res))
		return 0;

	rc = mdss_pll_resource_enable(dsi_pll_res, true);
	if (rc) {
		pr_err("Failed to enable mdss dsi pll resources\n");
		return rc;
	}

	/* Check to see if the ref clk doubler is enabled */
	doubler = MDSS_PLL_REG_R(dsi_pll_res->pll_base,
				 DSI_PHY_PLL_UNIPHY_PLL_REFCLK_CFG) & BIT(0);
	ref_clk += (doubler * vco->ref_clk_rate);

	/* see if it is integer mode or sdm mode */
	sdm0 = MDSS_PLL_REG_R(dsi_pll_res->pll_base,
					DSI_PHY_PLL_UNIPHY_PLL_SDM_CFG0);
	if (sdm0 & BIT(6)) {
		/* integer mode */
		sdm_byp_div = (MDSS_PLL_REG_R(dsi_pll_res->pll_base,
			DSI_PHY_PLL_UNIPHY_PLL_SDM_CFG0) & 0x3f) + 1;
		vco_rate = ref_clk * sdm_byp_div;
	} else {
		/* sdm mode */
		sdm_dc_off = MDSS_PLL_REG_R(dsi_pll_res->pll_base,
			DSI_PHY_PLL_UNIPHY_PLL_SDM_CFG1) & 0xFF;

		sdm2 = MDSS_PLL_REG_R(dsi_pll_res->pll_base,
			DSI_PHY_PLL_UNIPHY_PLL_SDM_CFG2) & 0xFF;
		sdm3 = MDSS_PLL_REG_R(dsi_pll_res->pll_base,
			DSI_PHY_PLL_UNIPHY_PLL_SDM_CFG3) & 0xFF;
		sdm_freq_seed = (sdm3 << 8) | sdm2;

		vco_rate = (ref_clk * (sdm_dc_off + 1)) +
			mult_frac(ref_clk, sdm_freq_seed, BIT(16));
	}

	mdss_pll_resource_enable(dsi_pll_res, false);

	return (unsigned long)vco_rate;
}

long vco_round_rate(struct clk_hw *hw, unsigned long rate,
		unsigned long *parent_rate)
{
	unsigned long rrate = rate;
	struct dsi_pll_vco_clk *vco = to_vco_clk_hw(hw);

	if (rate < vco->min_rate)
		rrate = vco->min_rate;
	if (rate > vco->max_rate)
		rrate = vco->max_rate;

	return rrate;
}

int vco_prepare(struct clk_hw *hw)
{
	int rc = 0;
	struct dsi_pll_vco_clk *vco = to_vco_clk_hw(hw);
	struct mdss_pll_resources *dsi_pll_res = vco->priv;

	if (!dsi_pll_res) {
		pr_err("Dsi pll resources are not available\n");
		return -EINVAL;
	}

	if ((dsi_pll_res->vco_cached_rate != 0)
	    && (dsi_pll_res->vco_cached_rate == clk_hw_get_rate(hw))) {
		rc = hw->init->ops->set_rate(hw, dsi_pll_res->vco_cached_rate,
						dsi_pll_res->vco_cached_rate);
		if (rc) {
			pr_err("vco_set_rate failed. rc=%d\n", rc);
			goto error;
		}
		MDSS_PLL_REG_W(dsi_pll_res->pll_base,
				DSI_PHY_PLL_UNIPHY_PLL_POSTDIV1_CFG,
				dsi_pll_res->cached_cfg1);
		MDSS_PLL_REG_W(dsi_pll_res->pll_base,
				DSI_PHY_PLL_UNIPHY_PLL_POSTDIV3_CFG,
				dsi_pll_res->cached_outdiv);
	}

	rc = dsi_pll_enable(hw);

error:
	return rc;
}

void vco_unprepare(struct clk_hw *hw)
{
	struct dsi_pll_vco_clk *vco = to_vco_clk_hw(hw);
	struct mdss_pll_resources *dsi_pll_res = vco->priv;

	if (!dsi_pll_res) {
		pr_err("Dsi pll resources are not available\n");
		return;
	}

	/*
	 * The common clock framework does not reconfigure clocks
	 * that didn't change their rates, but our hardware loses
	 * configuration whenever we shut it down hence, to get it
	 * back up and running, we need to reprogram the postdiv
	 * bits everytime we want to bring it back up.
	 * Cache the current postdividers to set them back at vco
	 * prepare time.
	 */
	MDSS_PLL_REG_W(dsi_pll_res->pll_base,
				DSI_PHY_PLL_UNIPHY_PLL_POSTDIV1_CFG,
				dsi_pll_res->cached_cfg1);
	MDSS_PLL_REG_W(dsi_pll_res->pll_base,
				DSI_PHY_PLL_UNIPHY_PLL_POSTDIV3_CFG,
				dsi_pll_res->cached_outdiv);

	dsi_pll_res->vco_cached_rate = clk_hw_get_rate(hw);
	dsi_pll_disable(hw);
}

int dsi_pll_lock_status(struct mdss_pll_resources *dsi_pll_res)
{
	u32 status;
	int pll_locked;

	/* poll for PLL ready status */
	if (readl_poll_timeout_atomic((dsi_pll_res->pll_base +
			DSI_PHY_PLL_UNIPHY_PLL_STATUS),
			status,
			((status & BIT(0)) == 1),
			DSI_PLL_POLL_DELAY_US,
			DSI_PLL_POLL_TIMEOUT_US)) {
		pll_locked = 0;
	} else {
		pll_locked = 1;
	}

	return pll_locked;
}

static int dsi_pll_enable_seq(struct mdss_pll_resources *dsi_pll_res)
{
	int rc = 0;

	/*
	 * PLL power up sequence
	 * Add necessary delays recommended by hardware
	 */

	/* DSI Uniphy lock detect setting */
	MDSS_PLL_REG_W(dsi_pll_res->pll_base,
			DSI_PHY_PLL_UNIPHY_PLL_LKDET_CFG2, 0x0d);
	/* DSI Uniphy PLL Calibration setting */
	MDSS_PLL_REG_W(dsi_pll_res->pll_base,
			DSI_PHY_PLL_UNIPHY_PLL_CAL_CFG1, 0x34);
	/* DSI Uniphy PLL lock detect mcnt */
	MDSS_PLL_REG_W(dsi_pll_res->pll_base,
			DSI_PHY_PLL_UNIPHY_PLL_LKDET_CFG0, 0x10);
	/* DSI Uniphy PLL lock detect wait time */
	MDSS_PLL_REG_W(dsi_pll_res->pll_base,
			DSI_PHY_PLL_UNIPHY_PLL_LKDET_CFG1, 0x1a);
	/* make sure the above register writes happen */
	wmb();
	MDSS_PLL_REG_W(dsi_pll_res->pll_base,
		DSI_PHY_PLL_UNIPHY_PLL_GLB_CFG, 0x01);
	/* make sure the above register writes happen */
	wmb();
	MDSS_PLL_REG_W(dsi_pll_res->pll_base,
		DSI_PHY_PLL_UNIPHY_PLL_GLB_CFG, 0x05);
	udelay(30);
	MDSS_PLL_REG_W(dsi_pll_res->pll_base,
		DSI_PHY_PLL_UNIPHY_PLL_GLB_CFG, 0x07);
	udelay(50);
	MDSS_PLL_REG_W(dsi_pll_res->pll_base,
		DSI_PHY_PLL_UNIPHY_PLL_GLB_CFG, 0x0f);
	udelay(800);

	if (!dsi_pll_lock_status(dsi_pll_res)) {
		pr_err("DSI PLL lock failed\n");
		rc = -EINVAL;
	}

	return rc;
}

int fixed_4div_set_div(void *context, unsigned int reg, unsigned int div)
{
	int rc;
	struct mdss_pll_resources *dsi_pll_res = context;

	rc = mdss_pll_resource_enable(dsi_pll_res, true);
	if (rc) {
		pr_err("Failed to enable mdss dsi pll resources\n");
		return rc;
	}

	MDSS_PLL_REG_W(dsi_pll_res->pll_base,
				DSI_PHY_PLL_UNIPHY_PLL_POSTDIV2_CFG, div + 1);

	mdss_pll_resource_enable(dsi_pll_res, false);
	return rc;
}

int fixed_4div_get_div(void *context, unsigned int reg, unsigned int *div)
{
	int rc;
	struct mdss_pll_resources *dsi_pll_res = context;

	if (is_gdsc_disabled(dsi_pll_res))
		return 0;

	rc = mdss_pll_resource_enable(dsi_pll_res, true);
	if (rc) {
		pr_err("Failed to enable mdss dsi pll resources\n");
		return rc;
	}

	*div = MDSS_PLL_REG_R(dsi_pll_res->pll_base,
				DSI_PHY_PLL_UNIPHY_PLL_POSTDIV2_CFG);

	if (*div != 0)
		*div -= 1;

	mdss_pll_resource_enable(dsi_pll_res, false);

	return rc;
}

int analog_set_div(void *context, unsigned int reg, unsigned int div)
{
	int rc;
	struct mdss_pll_resources *dsi_pll_res = context;

	rc = mdss_pll_resource_enable(dsi_pll_res, true);
	if (rc) {
		pr_err("Failed to enable mdss dsi pll resources\n");
		return rc;
	}

	MDSS_PLL_REG_W(dsi_pll_res->pll_base,
				DSI_PHY_PLL_UNIPHY_PLL_POSTDIV1_CFG, div);

	mdss_pll_resource_enable(dsi_pll_res, false);

	dsi_pll_res->cached_cfg1 = div;

	return rc;
}

int analog_get_div(void *context, unsigned int reg, unsigned int *div)
{
	int rc;
	struct mdss_pll_resources *dsi_pll_res = context;

	if (is_gdsc_disabled(dsi_pll_res))
		return 0;

	rc = mdss_pll_resource_enable(dsi_pll_res, true);
	if (rc) {
		pr_err("Failed to enable mdss dsi pll resources\n");
		return rc;
	}

	*div = MDSS_PLL_REG_R(dsi_pll_res->pll_base,
		DSI_PHY_PLL_UNIPHY_PLL_POSTDIV1_CFG);

	if (*div != 0)
		*div -= 1;

	mdss_pll_resource_enable(dsi_pll_res, false);

	return rc;
}

int digital_set_div(void *context, unsigned int reg, unsigned int div)
{
	int rc;
	struct mdss_pll_resources *dsi_pll_res = context;
	unsigned int final_div = 0;

	rc = mdss_pll_resource_enable(dsi_pll_res, true);
	if (rc) {
		pr_err("Failed to enable mdss dsi pll resources\n");
		return rc;
	}

	if (likely(div >= 2))
		final_div = div - 2;

	MDSS_PLL_REG_W(dsi_pll_res->pll_base,
				DSI_PHY_PLL_UNIPHY_PLL_POSTDIV3_CFG, final_div);

	dsi_pll_res->cached_outdiv = final_div;

	mdss_pll_resource_enable(dsi_pll_res, false);
	return rc;
}

int digital_get_div(void *context, unsigned int reg, unsigned int *div)
{
	int rc;
	struct mdss_pll_resources *dsi_pll_res = context;

	if (is_gdsc_disabled(dsi_pll_res))
		return 0;

	rc = mdss_pll_resource_enable(dsi_pll_res, true);
	if (rc) {
		pr_err("Failed to enable mdss dsi pll resources\n");
		return rc;
	}

	*div = MDSS_PLL_REG_R(dsi_pll_res->pll_base,
				DSI_PHY_PLL_UNIPHY_PLL_POSTDIV3_CFG) + 1;

	mdss_pll_resource_enable(dsi_pll_res, false);

	return rc;
}

int digital_get_div_video(void *context, unsigned int reg, unsigned int *div)
{
	int rc = digital_get_div(context, reg, div);

	if (*div != 0) {
		if (*div > 1)
			*div -= 1;
		*div -= 1;
	}

 	return rc;
}

int set_byte_mux_sel(void *context, unsigned int reg, unsigned int val)
{
	struct mdss_pll_resources *dsi_pll_res = context;

	pr_debug("byte mux set to %s mode\n", val ? "indirect" : "direct");
	MDSS_PLL_REG_W(dsi_pll_res->pll_base,
				DSI_PHY_PLL_UNIPHY_PLL_VREG_CFG, (val << 1));

	return 0;
}

int get_byte_mux_sel(void *context, unsigned int reg, unsigned int *val)
{
	int rc;
	struct mdss_pll_resources *dsi_pll_res = context;

	if (is_gdsc_disabled(dsi_pll_res))
		return 0;

	rc = mdss_pll_resource_enable(dsi_pll_res, true);
	if (rc) {
		pr_err("Failed to enable mdss dsi pll resources\n");
		return rc;
	}

	*val = !!(MDSS_PLL_REG_R(dsi_pll_res->pll_base,
				DSI_PHY_PLL_UNIPHY_PLL_VREG_CFG) & BIT(1));

	pr_debug("byte mux mode = %s", *val ? "indirect" : "direct");
	mdss_pll_resource_enable(dsi_pll_res, false);

	return 0;
}

/* Op structures - common for both DSI PLL0 and DSI PLL1 */
static struct regmap_bus fixed_4div_regmap_bus = {
	.reg_write = fixed_4div_set_div,
	.reg_read = fixed_4div_get_div,
};

static struct regmap_bus analog_postdiv_regmap_bus = {
	.reg_write = analog_set_div,
	.reg_read = analog_get_div,
};

static struct regmap_bus digital_postdiv_regmap_bus = {
	.reg_write = digital_set_div,
	.reg_read = digital_get_div,
};

static struct clk_ops clk_ops_dsi_vco = {
	.set_rate = vco_set_rate_hpm,
	.recalc_rate = vco_recalc_rate,
	.round_rate = vco_round_rate,
	.prepare = vco_prepare,
	.unprepare = vco_unprepare,
};

static struct regmap_bus byte_mux_regmap_bus = {
	.reg_write = set_byte_mux_sel,
	.reg_read = get_byte_mux_sel,
};

/* DSI PLL0 clock structures */
static struct dsi_pll_vco_clk dsi_pll0_vco_clk = {
	.ref_clk_rate = 19200000UL,
	.min_rate = 350000000UL,
	.max_rate = 750000000UL,
	.pll_en_seq_cnt = 1,
	.pll_enable_seqs[0] = dsi_pll_enable_seq,
	.lpfr_lut_size = 10,
	.lpfr_lut = lpfr_lut_struct,
	.hw.init = &(struct clk_init_data){
		.name = "dsi_pll0_vco_clk",
		.parent_names = (const char *[]){ "xo" },
		.num_parents = 1,
		.ops = &clk_ops_dsi_vco,
	},
};

static struct clk_regmap_div dsi_pll0_analog_postdiv_clk = {
	.reg = 0x20,
	.shift = 0,
	.width = 8,

	.clkr = {
		.hw.init = &(struct clk_init_data){
			.name = "dsi_pll0_analog_postdiv_clk",
			.parent_names =
				(const char *[]){ "dsi_pll0_vco_clk" },
			.num_parents = 1,
			.flags = (CLK_GET_RATE_NOCACHE | CLK_SET_RATE_PARENT),
			.ops = &clk_regmap_div_ops,
		},
	},
};

static struct clk_fixed_factor dsi_pll0_indirect_path_div2_clk = {
	.div = 2,
	.mult = 1,

	.hw.init = &(struct clk_init_data){
		.name = "dsi_pll0_indirect_path_div2_clk",
		.parent_names =
			(const char *[]){ "dsi_pll0_analog_postdiv_clk" },
		.num_parents = 1,
		.flags = (CLK_GET_RATE_NOCACHE | CLK_SET_RATE_PARENT),
		.ops = &clk_fixed_factor_ops,
	},
};

static struct clk_regmap_div dsi_pll0_pixel_clk_src = {
	.reg = 0x20,
	.shift = 0,
	.width = 8,

	.clkr = {
		.hw.init = &(struct clk_init_data){
			.name = "dsi_pll0_pixel_clk_src",
			.parent_names =
				(const char *[]){ "dsi_pll0_vco_clk" },
			.num_parents = 1,
			.flags = (CLK_GET_RATE_NOCACHE),
			.ops = &clk_regmap_div_ops,
		},
	},
};

static struct clk_regmap_mux dsi_pll0_byte_mux = {
	.reg = 0x20,
	.shift = 0,
	.width = 1,

	.clkr = {
		.hw.init = &(struct clk_init_data){
			.name = "dsi_pll0_byte_mux",
			.parent_names =
				(const char *[]){ "dsi_pll0_vco_clk",
					"dsi_pll0_indirect_path_div2_clk" },
			.num_parents = 2,
			.flags = (CLK_SET_RATE_PARENT),
			.ops = &clk_regmap_mux_closest_ops,
		},
	},
};

static struct clk_regmap_div dsi_pll0_4div_clk = {
	.shift = 0,
	.width = 1,

	.clkr.hw.init = &(struct clk_init_data){
		.name = "dsi_pll0_4div_clk",
		.parent_names =
			(const char *[]){ "dsi_pll0_byte_mux" },
		.num_parents = 1,
		.flags = (CLK_SET_RATE_PARENT),
		.ops = &clk_regmap_div_ops,
	},
};

static struct clk_fixed_factor dsi_pll0_byte_clk_src = {
	.div = 4,
	.mult = 1,

	.hw.init = &(struct clk_init_data){
		.name = "dsi_pll0_byte_clk_src",
		.parent_names =
			(const char *[]){ "dsi_pll0_4div_clk" },
		.num_parents = 1,
		.flags = (CLK_SET_RATE_PARENT),
		.ops = &clk_fixed_factor_ops,
	},
};

/* DSI PLL1 clock structures */
static struct dsi_pll_vco_clk dsi_pll1_vco_clk = {
	.ref_clk_rate = 19200000UL,
	.min_rate = 350000000UL,
	.max_rate = 750000000UL,
	.pll_en_seq_cnt = 1,
	.pll_enable_seqs[0] = dsi_pll_enable_seq,
	.lpfr_lut_size = 10,
	.lpfr_lut = lpfr_lut_struct,
	.hw.init = &(struct clk_init_data){
		.name = "dsi_pll1_vco_clk",
		.parent_names = (const char *[]){ "xo" },
		.num_parents = 1,
		.ops = &clk_ops_dsi_vco,
	},
};

static struct clk_regmap_div dsi_pll1_analog_postdiv_clk = {
	.reg = 0x20,
	.shift = 0,
	.width = 8,

	.clkr = {
		.hw.init = &(struct clk_init_data){
			.name = "dsi_pll1_analog_postdiv_clk",
			.parent_names =
				(const char *[]){ "dsi_pll1_vco_clk" },
			.num_parents = 1,
			.flags = (CLK_GET_RATE_NOCACHE | CLK_SET_RATE_PARENT),
			.ops = &clk_regmap_div_ops,
		},
	},
};

static struct clk_fixed_factor dsi_pll1_indirect_path_div2_clk = {
	.div = 2,
	.mult = 1,

	.hw.init = &(struct clk_init_data){
		.name = "dsi_pll1_indirect_path_div2_clk",
		.parent_names =
			(const char *[]){ "dsi_pll1_analog_postdiv_clk" },
		.num_parents = 1,
		.flags = (CLK_GET_RATE_NOCACHE | CLK_SET_RATE_PARENT),
		.ops = &clk_fixed_factor_ops,
	},
};

static struct clk_regmap_div dsi_pll1_pixel_clk_src = {
	.reg = 0x20,
	.shift = 0,
	.width = 8,

	.clkr = {
		.hw.init = &(struct clk_init_data){
			.name = "dsi_pll1_pixel_clk_src",
			.parent_names =
				(const char *[]){ "dsi_pll1_vco_clk" },
			.num_parents = 1,
			.flags = (CLK_GET_RATE_NOCACHE),
			.ops = &clk_regmap_div_ops,
		},
	},
};

static struct clk_regmap_mux dsi_pll1_byte_mux = {
	.reg = 0x20,
	.shift = 0,
	.width = 1,

	.clkr = {
		.hw.init = &(struct clk_init_data){
			.name = "dsi_pll1_byte_mux",
			.parent_names =
				(const char *[]){ "dsi_pll1_vco_clk",
					"dsi_pll1_indirect_path_div2_clk" },
			.num_parents = 2,
			.flags = (CLK_SET_RATE_PARENT),
			.ops = &clk_regmap_mux_closest_ops,
		},
	},
};

static struct clk_regmap_div dsi_pll1_4div_clk = {
	.shift = 0,
	.width = 1,

	.clkr.hw.init = &(struct clk_init_data){
		.name = "dsi_pll1_4div_clk",
		.parent_names =
			(const char *[]){ "dsi_pll1_byte_mux" },
		.num_parents = 1,
		.flags = (CLK_SET_RATE_PARENT),
		.ops = &clk_regmap_div_ops,
	},
};

static struct clk_fixed_factor dsi_pll1_byte_clk_src = {
	.div = 4,
	.mult = 1,

	.hw.init = &(struct clk_init_data){
		.name = "dsi_pll1_byte_clk_src",
		.parent_names =
			(const char *[]){ "dsi_pll1_4div_clk" },
		.num_parents = 1,
		.flags = (CLK_SET_RATE_PARENT),
		.ops = &clk_fixed_factor_ops,
	},
};

static struct clk_hw *mdss_dsi_pllcc_28hpm[] = {
	[HPM_BYTE0_MUX_CLK]		= &dsi_pll0_byte_mux.clkr.hw,
	[HPM_BYTE0_SRC_CLK]		= &dsi_pll0_byte_clk_src.hw,
	[HPM_PIX0_SRC_CLK]		= &dsi_pll0_pixel_clk_src.clkr.hw,
	[HPM_ANALOG_POSTDIV_0_CLK]	= &dsi_pll0_analog_postdiv_clk.clkr.hw,
	[HPM_INDIR_PATH_DIV2_0_CLK]	= &dsi_pll0_indirect_path_div2_clk.hw,
	[HPM_4DIV_0_CLK]		= &dsi_pll0_4div_clk.clkr.hw,
	[HPM_VCO_CLK_0_CLK]		= &dsi_pll0_vco_clk.hw,

	[HPM_BYTE1_MUX_CLK]		= &dsi_pll1_byte_mux.clkr.hw,
	[HPM_BYTE1_SRC_CLK]		= &dsi_pll1_byte_clk_src.hw,
	[HPM_PIX1_SRC_CLK]		= &dsi_pll1_pixel_clk_src.clkr.hw,
	[HPM_ANALOG_POSTDIV_1_CLK]	= &dsi_pll1_analog_postdiv_clk.clkr.hw,
	[HPM_INDIR_PATH_DIV2_1_CLK]	= &dsi_pll1_indirect_path_div2_clk.hw,
	[HPM_4DIV_1_CLK]		= &dsi_pll1_4div_clk.clkr.hw,
	[HPM_VCO_CLK_1_CLK]		= &dsi_pll1_vco_clk.hw,
};

int dsi_pll_clock_register_28hpm(struct platform_device *pdev,
				struct mdss_pll_resources *pll_res)
{
	int i, rc = 0;
	struct clk_onecell_data *clk_data;
	struct clk *clk;
	struct regmap *regmap;
	int num_clks = ARRAY_SIZE(mdss_dsi_pllcc_28hpm);
	bool video_fixup;

	if (!pdev || !pdev->dev.of_node) {
		pr_err("Invalid input parameters\n");
		return -EINVAL;
	}

	if (!pll_res || !pll_res->pll_base) {
		pr_err("Invalid PLL resources\n");
		return -EPROBE_DEFER;
	}

	clk_data = devm_kzalloc(&pdev->dev, sizeof(struct clk_onecell_data),
					GFP_KERNEL);
	if (!clk_data)
		return -ENOMEM;

	clk_data->clks = devm_kzalloc(&pdev->dev, (num_clks *
				sizeof(struct clk *)), GFP_KERNEL);
	if (!clk_data->clks) {
		devm_kfree(&pdev->dev, clk_data);
		return -ENOMEM;
	}

	clk_data->clk_num = num_clks;

	video_fixup = of_property_read_bool(pdev->dev.of_node,
					"qcom,dsi-pll-video-mode-fixup");
	if (video_fixup) {
		dev_info(&pdev->dev,
			"Applying pixel clock fixup for video mode\n");
		digital_postdiv_regmap_bus.reg_read = digital_get_div_video;
	}

	if (!pll_res->index) {
		regmap = devm_regmap_init(&pdev->dev, &fixed_4div_regmap_bus,
					pll_res, &dsi_pll_28hpm_config);
		dsi_pll0_4div_clk.clkr.regmap = regmap;

		regmap = devm_regmap_init(&pdev->dev, &digital_postdiv_regmap_bus,
					pll_res, &dsi_pll_28hpm_config);
		dsi_pll0_pixel_clk_src.clkr.regmap = regmap;

		regmap = devm_regmap_init(&pdev->dev, &analog_postdiv_regmap_bus,
					pll_res, &dsi_pll_28hpm_config);
		dsi_pll0_analog_postdiv_clk.clkr.regmap = regmap;

		regmap = devm_regmap_init(&pdev->dev, &byte_mux_regmap_bus,
					pll_res, &dsi_pll_28hpm_config);
		dsi_pll0_byte_mux.clkr.regmap = regmap;

		pll_res->vco_delay = VCO_DELAY_USEC;

		dsi_pll0_vco_clk.priv = pll_res;

		for (i = HPM_BYTE0_MUX_CLK; i <= HPM_VCO_CLK_0_CLK; i++) {
			pr_debug("register clk: %d index: %d\n",
							i, pll_res->index);
			clk = devm_clk_register(&pdev->dev,
					mdss_dsi_pllcc_28hpm[i]);
			if (IS_ERR(clk)) {
				pr_err("clk registration failed for DSI: %d\n",
						pll_res->index);
				rc = -EINVAL;
				goto clk_reg_fail;
			}
			clk_data->clks[i] = clk;
		}

		rc = of_clk_add_provider(pdev->dev.of_node,
				of_clk_src_onecell_get, clk_data);
	} else {
		regmap = devm_regmap_init(&pdev->dev, &fixed_4div_regmap_bus,
					pll_res, &dsi_pll_28hpm_config);
		dsi_pll1_4div_clk.clkr.regmap = regmap;

		regmap = devm_regmap_init(&pdev->dev, &digital_postdiv_regmap_bus,
					pll_res, &dsi_pll_28hpm_config);
		dsi_pll1_pixel_clk_src.clkr.regmap = regmap;

		regmap = devm_regmap_init(&pdev->dev, &analog_postdiv_regmap_bus,
					pll_res, &dsi_pll_28hpm_config);
		dsi_pll1_analog_postdiv_clk.clkr.regmap = regmap;

		regmap = devm_regmap_init(&pdev->dev, &byte_mux_regmap_bus,
					pll_res, &dsi_pll_28hpm_config);
		dsi_pll1_byte_mux.clkr.regmap = regmap;

		dsi_pll1_vco_clk.priv = pll_res;

		pll_res->vco_delay = VCO_DELAY_USEC;

		for (i = HPM_BYTE1_MUX_CLK; i <= HPM_VCO_CLK_1_CLK; i++) {
			pr_debug("register clk: %d index: %d\n",
							i, pll_res->index);
			clk = devm_clk_register(&pdev->dev,
					mdss_dsi_pllcc_28hpm[i]);
			if (IS_ERR(clk)) {
				pr_err("clk registration failed for DSI: %d\n",
						pll_res->index);
				rc = -EINVAL;
				goto clk_reg_fail;
			}
			clk_data->clks[i] = clk;
		}

		rc = of_clk_add_provider(pdev->dev.of_node,
				of_clk_src_onecell_get, clk_data);
	}

	if (!rc)
		pr_info("Registered DSI PLL:%d clocks successfully\n",
			pll_res->index);

	return rc;

clk_reg_fail:
	devm_kfree(&pdev->dev, clk_data->clks);
	devm_kfree(&pdev->dev, clk_data);
	return rc;
}
