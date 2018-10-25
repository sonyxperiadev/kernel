/*
 * Copyright (c) 2016-2018, The Linux Foundation. All rights reserved.
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

#define pr_fmt(fmt) "dsi-phy-timing:" fmt

#include "dsi_phy_timing_calc.h"

static const u32 bits_per_pixel[DSI_PIXEL_FORMAT_MAX] = {
	16, 18, 18, 24, 3, 8, 12 };

static int dsi_phy_cmn_validate_and_set(struct timing_entry *t,
	char const *t_name)
{
	if (t->rec & 0xffffff00) {
		/* Output value can only be 8 bits */
		pr_err("Incorrect %s rec value - %d\n", t_name, t->rec);
		return -EINVAL;
	}
	t->reg_value = t->rec;
	return 0;
}

/**
 * calc_clk_prepare - calculates prepare timing params for clk lane.
 */
static int calc_clk_prepare(struct dsi_phy_hw *phy,
				struct phy_clk_params *clk_params,
			    struct phy_timing_desc *desc,
			    s32 *actual_frac,
			    s64 *actual_intermediate)
{
	u64 const multiplier = BIT(20);
	struct timing_entry *t = &desc->clk_prepare;
	int rc = 0;
	u64 dividend, temp, temp_multiple;
	s32 frac = 0;
	s64 intermediate;
	s64 clk_prep_actual;

	dividend = ((t->rec_max - t->rec_min) *
		clk_params->clk_prep_buf * multiplier);
	temp  = roundup(div_s64(dividend, 100), multiplier);
	temp += (t->rec_min * multiplier);
	t->rec = div_s64(temp, multiplier);

	rc = dsi_phy_cmn_validate_and_set(t, "clk_prepare");
	if (rc)
		goto error;

	/* calculate theoretical value */
	temp_multiple = 8 * t->reg_value * clk_params->tlpx_numer_ns
			 * multiplier;
	intermediate = div_s64(temp_multiple, clk_params->bitclk_mbps);
	div_s64_rem(temp_multiple, clk_params->bitclk_mbps, &frac);
	clk_prep_actual = div_s64((intermediate + frac), multiplier);

	pr_debug("CLK_PREPARE:mipi_min=%d, mipi_max=%d, rec_min=%d, rec_max=%d",
		 t->mipi_min, t->mipi_max, t->rec_min, t->rec_max);
	pr_debug(" reg_value=%d, actual=%lld\n", t->reg_value, clk_prep_actual);

	*actual_frac = frac;
	*actual_intermediate = intermediate;

error:
	return rc;
}

/**
 * calc_clk_zero - calculates zero timing params for clk lane.
 */
static int calc_clk_zero(struct dsi_phy_hw *phy,
			struct phy_clk_params *clk_params,
			struct phy_timing_desc *desc,
			s32 actual_frac, s64 actual_intermediate)
{
	u64 const multiplier = BIT(20);
	int rc = 0;
	struct timing_entry *t = &desc->clk_zero;
	s64 mipi_min, rec_temp1;
	struct phy_timing_ops *ops = phy->ops.timing_ops;

	mipi_min = ((300 * multiplier) - (actual_intermediate + actual_frac));
	t->mipi_min = div_s64(mipi_min, multiplier);

	rec_temp1 = div_s64((mipi_min * clk_params->bitclk_mbps),
			    clk_params->tlpx_numer_ns);

	if (ops->calc_clk_zero) {
		t->rec_min = ops->calc_clk_zero(rec_temp1, multiplier);
	} else {
		rc = -EINVAL;
		goto error;
	}
	t->rec_max = ((t->rec_min > 255) ? 511 : 255);

	t->rec = DIV_ROUND_UP((((t->rec_max - t->rec_min) *
		clk_params->clk_zero_buf) + (t->rec_min * 100)), 100);

	rc = dsi_phy_cmn_validate_and_set(t, "clk_zero");
	if (rc)
		goto error;


	pr_debug("CLK_ZERO:mipi_min=%d, mipi_max=%d, rec_min=%d, rec_max=%d, reg_val=%d\n",
		 t->mipi_min, t->mipi_max, t->rec_min, t->rec_max,
		 t->reg_value);
error:
	return rc;
}

/**
 * calc_clk_trail - calculates prepare trail params for clk lane.
 */
static int calc_clk_trail(struct dsi_phy_hw *phy,
			struct phy_clk_params *clk_params,
			struct phy_timing_desc *desc,
			s64 *teot_clk_lane)
{
	u64 const multiplier = BIT(20);
	int rc = 0;
	struct timing_entry *t = &desc->clk_trail;
	u64 temp_multiple;
	s32 frac;
	s64 mipi_max_tr, rec_temp1, mipi_max;
	s64 teot_clk_lane1;
	struct phy_timing_ops *ops = phy->ops.timing_ops;

	temp_multiple = div_s64(
			(12 * multiplier * clk_params->tlpx_numer_ns),
			clk_params->bitclk_mbps);
	div_s64_rem(temp_multiple, multiplier, &frac);

	mipi_max_tr = ((105 * multiplier) +
		       (temp_multiple + frac));
	teot_clk_lane1 = div_s64(mipi_max_tr, multiplier);

	mipi_max = (mipi_max_tr - (clk_params->treot_ns * multiplier));
	t->mipi_max = div_s64(mipi_max, multiplier);

	temp_multiple = div_s64(
			(t->mipi_min * multiplier * clk_params->bitclk_mbps),
			clk_params->tlpx_numer_ns);

	div_s64_rem(temp_multiple, multiplier, &frac);
	if (ops->calc_clk_trail_rec_min) {
		t->rec_min = ops->calc_clk_trail_rec_min(temp_multiple,
			frac, multiplier);
	} else {
		rc = -EINVAL;
		goto error;
	}

	/* recommended max */
	rec_temp1 = div_s64((mipi_max * clk_params->bitclk_mbps),
			    clk_params->tlpx_numer_ns);
	if (ops->calc_clk_trail_rec_max) {
		t->rec_max = ops->calc_clk_trail_rec_max(rec_temp1, multiplier);
	} else {
		rc = -EINVAL;
		goto error;
	}

	t->rec = DIV_ROUND_UP(
		(((t->rec_max - t->rec_min) * clk_params->clk_trail_buf) +
		 (t->rec_min * 100)), 100);

	rc = dsi_phy_cmn_validate_and_set(t, "clk_trail");
	if (rc)
		goto error;

	*teot_clk_lane = teot_clk_lane1;
	pr_debug("CLK_TRAIL:mipi_min=%d, mipi_max=%d, rec_min=%d, rec_max=%d, reg_val=%d\n",
		 t->mipi_min, t->mipi_max, t->rec_min, t->rec_max,
		 t->reg_value);

error:
	return rc;

}

/**
 * calc_hs_prepare - calculates prepare timing params for data lanes in HS.
 */
static int calc_hs_prepare(struct dsi_phy_hw *phy,
			struct phy_clk_params *clk_params,
			struct phy_timing_desc *desc,
			u64 *temp_mul)
{
	u64 const multiplier = BIT(20);
	int rc = 0;
	struct timing_entry *t = &desc->hs_prepare;
	u64 temp_multiple, dividend, temp;
	s32 frac;
	s64 rec_temp1, rec_temp2, mipi_max, mipi_min;
	u32 low_clk_multiplier = 0;

	if (clk_params->bitclk_mbps <= 120)
		low_clk_multiplier = 2;
	/* mipi min */
	temp_multiple = div_s64((4 * multiplier * clk_params->tlpx_numer_ns),
				clk_params->bitclk_mbps);
	div_s64_rem(temp_multiple, multiplier, &frac);
	mipi_min = (40 * multiplier) + (temp_multiple + frac);
	t->mipi_min = div_s64(mipi_min, multiplier);

	/* mipi_max */
	temp_multiple = div_s64(
			(6 * multiplier * clk_params->tlpx_numer_ns),
			clk_params->bitclk_mbps);
	div_s64_rem(temp_multiple, multiplier, &frac);
	mipi_max = (85 * multiplier) + temp_multiple;
	t->mipi_max = div_s64(mipi_max, multiplier);

	/* recommended min */
	temp_multiple = div_s64((mipi_min * clk_params->bitclk_mbps),
				clk_params->tlpx_numer_ns);
	temp_multiple -= (low_clk_multiplier * multiplier);
	div_s64_rem(temp_multiple, multiplier, &frac);
	rec_temp1 = roundup(((temp_multiple + frac) / 8), multiplier);
	t->rec_min = div_s64(rec_temp1, multiplier);

	/* recommended max */
	temp_multiple = div_s64((mipi_max * clk_params->bitclk_mbps),
				clk_params->tlpx_numer_ns);
	temp_multiple -= (low_clk_multiplier * multiplier);
	div_s64_rem(temp_multiple, multiplier, &frac);
	rec_temp2 = rounddown((temp_multiple / 8), multiplier);
	t->rec_max = div_s64(rec_temp2, multiplier);

	/* register value */
	dividend = ((rec_temp2 - rec_temp1) * clk_params->hs_prep_buf);
	temp = roundup(div_u64(dividend, 100), multiplier);
	t->rec = div_s64((temp + rec_temp1), multiplier);

	rc = dsi_phy_cmn_validate_and_set(t, "hs_prepare");
	if (rc)
		goto error;

	temp_multiple = div_s64(
			(8 * (temp + rec_temp1) * clk_params->tlpx_numer_ns),
			clk_params->bitclk_mbps);

	*temp_mul = temp_multiple;
	pr_debug("HS_PREP:mipi_min=%d, mipi_max=%d, rec_min=%d, rec_max=%d, reg_val=%d\n",
		 t->mipi_min, t->mipi_max, t->rec_min, t->rec_max,
		 t->reg_value);
error:
	return rc;
}

/**
 * calc_hs_zero - calculates zero timing params for data lanes in HS.
 */
static int calc_hs_zero(struct dsi_phy_hw *phy,
			struct phy_clk_params *clk_params,
			struct phy_timing_desc *desc,
			u64 temp_multiple)
{
	u64 const multiplier = BIT(20);
	int rc = 0;
	struct timing_entry *t = &desc->hs_zero;
	s64 rec_temp1, mipi_min;
	struct phy_timing_ops *ops = phy->ops.timing_ops;

	mipi_min = div_s64((10 * clk_params->tlpx_numer_ns * multiplier),
			   clk_params->bitclk_mbps);
	rec_temp1 = (145 * multiplier) + mipi_min - temp_multiple;
	t->mipi_min = div_s64(rec_temp1, multiplier);

	/* recommended min */
	rec_temp1 = div_s64((rec_temp1 * clk_params->bitclk_mbps),
			    clk_params->tlpx_numer_ns);

	if (ops->calc_hs_zero) {
		t->rec_min = ops->calc_hs_zero(rec_temp1, multiplier);
	} else {
		rc = -EINVAL;
		goto error;
	}

	t->rec_max = ((t->rec_min > 255) ? 511 : 255);
	t->rec = DIV_ROUND_UP(
			(((t->rec_max - t->rec_min) * clk_params->hs_zero_buf) +
			 (t->rec_min * 100)),
			100);

	rc = dsi_phy_cmn_validate_and_set(t, "hs_zero");
	if (rc)
		goto error;

	pr_debug("HS_ZERO:mipi_min=%d, mipi_max=%d, rec_min=%d, rec_max=%d, reg_val=%d\n",
		 t->mipi_min, t->mipi_max, t->rec_min, t->rec_max,
		 t->reg_value);

error:
	return rc;
}

/**
 * calc_hs_trail - calculates trail timing params for data lanes in HS.
 */
static int calc_hs_trail(struct dsi_phy_hw *phy,
			struct phy_clk_params *clk_params,
			struct phy_timing_desc *desc,
			u64 teot_clk_lane)
{
	int rc = 0;
	struct timing_entry *t = &desc->hs_trail;
	s64 rec_temp1;
	struct phy_timing_ops *ops = phy->ops.timing_ops;

	t->mipi_min = 60 +
			mult_frac(clk_params->tlpx_numer_ns, 4,
				  clk_params->bitclk_mbps);

	t->mipi_max = teot_clk_lane - clk_params->treot_ns;

	if (ops->calc_hs_trail) {
		ops->calc_hs_trail(clk_params, desc);
	} else {
		rc = -EINVAL;
		goto error;
	}

	rec_temp1 = DIV_ROUND_UP(
			((t->rec_max - t->rec_min) * clk_params->hs_trail_buf),
			100);
	t->rec = rec_temp1 + t->rec_min;

	rc = dsi_phy_cmn_validate_and_set(t, "hs_trail");
	if (rc)
		goto error;

	pr_debug("HS_TRAIL:mipi_min=%d, mipi_max=%d, rec_min=%d, rec_max=%d, reg_val=%d\n",
		 t->mipi_min, t->mipi_max, t->rec_min, t->rec_max,
		 t->reg_value);

error:
	return rc;
}

/**
 * calc_hs_rqst - calculates rqst timing params for data lanes in HS.
 */
static int calc_hs_rqst(struct dsi_phy_hw *phy,
			struct phy_clk_params *clk_params,
			struct phy_timing_desc *desc)
{
	int rc = 0;
	struct timing_entry *t = &desc->hs_rqst;

	t->rec = DIV_ROUND_UP(
		((t->mipi_min * clk_params->bitclk_mbps) -
		 (8 * clk_params->tlpx_numer_ns)),
		(8 * clk_params->tlpx_numer_ns));

	rc = dsi_phy_cmn_validate_and_set(t, "hs_rqst");
	if (rc)
		goto error;

	pr_debug("HS_RQST:mipi_min=%d, mipi_max=%d, rec_min=%d, rec_max=%d, reg_val=%d\n",
		 t->mipi_min, t->mipi_max, t->rec_min, t->rec_max,
		 t->reg_value);

error:
	return rc;
}

/**
 * calc_hs_exit - calculates exit timing params for data lanes in HS.
 */
static int calc_hs_exit(struct dsi_phy_hw *phy,
			struct phy_clk_params *clk_params,
			struct phy_timing_desc *desc)
{
	int rc = 0;
	struct timing_entry *t = &desc->hs_exit;

	t->rec_min = (DIV_ROUND_UP(
			(t->mipi_min * clk_params->bitclk_mbps),
			(8 * clk_params->tlpx_numer_ns)) - 1);

	t->rec = DIV_ROUND_UP(
		(((t->rec_max - t->rec_min) * clk_params->hs_exit_buf) +
		 (t->rec_min * 100)), 100);

	rc = dsi_phy_cmn_validate_and_set(t, "hs_exit");
	if (rc)
		goto error;


	pr_debug("HS_EXIT:mipi_min=%d, mipi_max=%d, rec_min=%d, rec_max=%d, reg_val=%d\n",
		 t->mipi_min, t->mipi_max, t->rec_min, t->rec_max,
		 t->reg_value);

error:
	return rc;
}

/**
 * calc_hs_rqst_clk - calculates rqst timing params for clock lane..
 */
static int calc_hs_rqst_clk(struct dsi_phy_hw *phy,
			struct phy_clk_params *clk_params,
			struct phy_timing_desc *desc)
{
	int rc = 0;
	struct timing_entry *t = &desc->hs_rqst_clk;

	t->rec = DIV_ROUND_UP(
		((t->mipi_min * clk_params->bitclk_mbps) -
		 (8 * clk_params->tlpx_numer_ns)),
		(8 * clk_params->tlpx_numer_ns));

	rc = dsi_phy_cmn_validate_and_set(t, "hs_rqst_clk");
	if (rc)
		goto error;

	pr_debug("HS_RQST_CLK:mipi_min=%d, mipi_max=%d, rec_min=%d, rec_max=%d, reg_val=%d\n",
		 t->mipi_min, t->mipi_max, t->rec_min, t->rec_max,
		 t->reg_value);

error:
	return rc;
}

/**
 * dsi_phy_calc_timing_params - calculates timing paramets for a given bit clock
 */
static int dsi_phy_cmn_calc_timing_params(struct dsi_phy_hw *phy,
	struct phy_clk_params *clk_params, struct phy_timing_desc *desc)
{
	int rc = 0;
	s32 actual_frac = 0;
	s64 actual_intermediate = 0;
	u64 temp_multiple;
	s64 teot_clk_lane;

	rc = calc_clk_prepare(phy, clk_params, desc, &actual_frac,
			      &actual_intermediate);
	if (rc) {
		pr_err("clk_prepare calculations failed, rc=%d\n", rc);
		goto error;
	}

	rc = calc_clk_zero(phy, clk_params, desc,
		actual_frac, actual_intermediate);
	if (rc) {
		pr_err("clk_zero calculations failed, rc=%d\n", rc);
		goto error;
	}

	rc = calc_clk_trail(phy, clk_params, desc, &teot_clk_lane);
	if (rc) {
		pr_err("clk_trail calculations failed, rc=%d\n", rc);
		goto error;
	}

	rc = calc_hs_prepare(phy, clk_params, desc, &temp_multiple);
	if (rc) {
		pr_err("hs_prepare calculations failed, rc=%d\n", rc);
		goto error;
	}

	rc = calc_hs_zero(phy, clk_params, desc, temp_multiple);
	if (rc) {
		pr_err("hs_zero calculations failed, rc=%d\n", rc);
		goto error;
	}

	rc = calc_hs_trail(phy, clk_params, desc, teot_clk_lane);
	if (rc) {
		pr_err("hs_trail calculations failed, rc=%d\n", rc);
		goto error;
	}

	rc = calc_hs_rqst(phy, clk_params, desc);
	if (rc) {
		pr_err("hs_rqst calculations failed, rc=%d\n", rc);
		goto error;
	}

	rc = calc_hs_exit(phy, clk_params, desc);
	if (rc) {
		pr_err("hs_exit calculations failed, rc=%d\n", rc);
		goto error;
	}

	rc = calc_hs_rqst_clk(phy, clk_params, desc);
	if (rc) {
		pr_err("hs_rqst_clk calculations failed, rc=%d\n", rc);
		goto error;
	}
error:
	return rc;
}

/**
 * calculate_timing_params() - calculates timing parameters.
 * @phy:      Pointer to DSI PHY hardware object.
 * @mode:     Mode information for which timing has to be calculated.
 * @config:   DSI host configuration for this mode.
 * @timing:   Timing parameters for each lane which will be returned.
 */
int dsi_phy_hw_calculate_timing_params(struct dsi_phy_hw *phy,
					    struct dsi_mode_info *mode,
					    struct dsi_host_common_cfg *host,
					   struct dsi_phy_per_lane_cfgs *timing)
{
	/* constants */
	u32 const esc_clk_mhz = 192; /* TODO: esc clock is hardcoded */
	u32 const esc_clk_mmss_cc_prediv = 10;
	u32 const tlpx_numer = 1000;
	u32 const tr_eot = 20;
	u32 const clk_prepare_spec_min = 38;
	u32 const clk_prepare_spec_max = 95;
	u32 const clk_trail_spec_min = 60;
	u32 const hs_exit_spec_min = 100;
	u32 const hs_exit_reco_max = 255;
	u32 const hs_rqst_spec_min = 50;

	/* local vars */
	int rc = 0;
	u32 h_total, v_total;
	u32 inter_num;
	u32 num_of_lanes = 0;
	u32 bpp;
	u64 x, y;
	struct phy_timing_desc desc;
	struct phy_clk_params clk_params = {0};
	struct phy_timing_ops *ops = phy->ops.timing_ops;

	memset(&desc, 0x0, sizeof(desc));
	h_total = DSI_H_TOTAL(mode);
	v_total = DSI_V_TOTAL(mode);

	bpp = bits_per_pixel[host->dst_format];

	inter_num = bpp * mode->refresh_rate;

	if (host->data_lanes & DSI_DATA_LANE_0)
		num_of_lanes++;
	if (host->data_lanes & DSI_DATA_LANE_1)
		num_of_lanes++;
	if (host->data_lanes & DSI_DATA_LANE_2)
		num_of_lanes++;
	if (host->data_lanes & DSI_DATA_LANE_3)
		num_of_lanes++;


	x = mult_frac(v_total * h_total, inter_num, num_of_lanes);
	y = rounddown(x, 1);

	clk_params.bitclk_mbps = rounddown(DIV_ROUND_UP_ULL(y, 1000000), 1);
	clk_params.escclk_numer = esc_clk_mhz;
	clk_params.escclk_denom = esc_clk_mmss_cc_prediv;
	clk_params.tlpx_numer_ns = tlpx_numer;
	clk_params.treot_ns = tr_eot;


	/* Setup default parameters */
	desc.clk_prepare.mipi_min = clk_prepare_spec_min;
	desc.clk_prepare.mipi_max = clk_prepare_spec_max;
	desc.clk_trail.mipi_min = clk_trail_spec_min;
	desc.hs_exit.mipi_min = hs_exit_spec_min;
	desc.hs_exit.rec_max = hs_exit_reco_max;
	desc.hs_rqst.mipi_min = hs_rqst_spec_min;
	desc.hs_rqst_clk.mipi_min = hs_rqst_spec_min;

	if (ops->get_default_phy_params) {
		ops->get_default_phy_params(&clk_params);
	} else {
		rc = -EINVAL;
		goto error;
	}

	desc.clk_prepare.rec_min = DIV_ROUND_UP(
			(desc.clk_prepare.mipi_min * clk_params.bitclk_mbps),
			(8 * clk_params.tlpx_numer_ns)
			);

	desc.clk_prepare.rec_max = rounddown(
		mult_frac((desc.clk_prepare.mipi_max * clk_params.bitclk_mbps),
			  1, (8 * clk_params.tlpx_numer_ns)),
		1);

	pr_debug("BIT CLOCK = %d, tlpx_numer_ns=%d, treot_ns=%d\n",
	       clk_params.bitclk_mbps, clk_params.tlpx_numer_ns,
	       clk_params.treot_ns);
	rc = dsi_phy_cmn_calc_timing_params(phy, &clk_params, &desc);
	if (rc) {
		pr_err("Timing calc failed, rc=%d\n", rc);
		goto error;
	}

	if (ops->update_timing_params) {
		ops->update_timing_params(timing, &desc);
	} else {
		rc = -EINVAL;
		goto error;
	}

error:
	return rc;
}

int dsi_phy_timing_calc_init(struct dsi_phy_hw *phy,
			enum dsi_phy_version version)
{
	struct phy_timing_ops *ops = NULL;

	if (version == DSI_PHY_VERSION_UNKNOWN ||
	    version >= DSI_PHY_VERSION_MAX || !phy) {
		pr_err("Unsupported version: %d\n", version);
		return -ENOTSUPP;
	}

	ops = kzalloc(sizeof(struct phy_timing_ops), GFP_KERNEL);
	if (!ops)
		return -EINVAL;
	phy->ops.timing_ops = ops;

	switch (version) {
	case DSI_PHY_VERSION_2_0:
		ops->get_default_phy_params =
			dsi_phy_hw_v2_0_get_default_phy_params;
		ops->calc_clk_zero =
			dsi_phy_hw_v2_0_calc_clk_zero;
		ops->calc_clk_trail_rec_min =
			dsi_phy_hw_v2_0_calc_clk_trail_rec_min;
		ops->calc_clk_trail_rec_max =
			dsi_phy_hw_v2_0_calc_clk_trail_rec_max;
		ops->calc_hs_zero =
			dsi_phy_hw_v2_0_calc_hs_zero;
		ops->calc_hs_trail =
			dsi_phy_hw_v2_0_calc_hs_trail;
		ops->update_timing_params =
			dsi_phy_hw_v2_0_update_timing_params;
		break;
	case DSI_PHY_VERSION_3_0:
		ops->get_default_phy_params =
			dsi_phy_hw_v3_0_get_default_phy_params;
		ops->calc_clk_zero =
			dsi_phy_hw_v3_0_calc_clk_zero;
		ops->calc_clk_trail_rec_min =
			dsi_phy_hw_v3_0_calc_clk_trail_rec_min;
		ops->calc_clk_trail_rec_max =
			dsi_phy_hw_v3_0_calc_clk_trail_rec_max;
		ops->calc_hs_zero =
			dsi_phy_hw_v3_0_calc_hs_zero;
		ops->calc_hs_trail =
			dsi_phy_hw_v3_0_calc_hs_trail;
		ops->update_timing_params =
			dsi_phy_hw_v3_0_update_timing_params;
		break;
	case DSI_PHY_VERSION_0_0_HPM:
	case DSI_PHY_VERSION_0_0_LPM:
	case DSI_PHY_VERSION_1_0:
	default:
		kfree(ops);
		return -ENOTSUPP;
	}

	return 0;
}

