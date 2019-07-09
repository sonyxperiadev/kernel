/*
 * Restructured unified display panel driver for Xperia Open Devices
 *                  *** Special VREG handling ***
 *
 * This driver is based on various SoMC implementations found in
 * copyleft archives for various devices.
 *
 * Copyright (c) 2012-2014, The Linux Foundation. All rights reserved.
 * Copyright (c) Sony Mobile Communications Inc. All rights reserved.
 * Copyright (C) 2014-2016, AngeloGioacchino Del Regno <kholk11@gmail.com>
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

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/of_platform.h>
#include <linux/gpio.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/qpnp-labibb-regulator.h>

#include "../mdss_dsi.h"
#include "somc_panels.h"

static bool vregs_initialized = 0;

static int somc_panel_vregs_init(struct mdss_dsi_ctrl_pdata *ctrl)
{
	struct mdss_panel_specific_pdata *spec_pdata = NULL;
	struct somc_panel_regulator_mgr *regulator_mgr = NULL;
	struct dss_vreg *this_vreg = NULL;
	int rc, min_uV, max_uV = 0;

	if (vregs_initialized)
		return 0;

	spec_pdata = ctrl->spec_pdata;
	if (!spec_pdata) {
		pr_err("%s: FATAL: NULL SoMC panel data!!\n", __func__);
		return -EINVAL;
	}

	regulator_mgr = spec_pdata->regulator_mgr;

	for (rc = 0; rc < ctrl->panel_power_data.num_vreg; rc++) {
		if ((ctrl->lab != NULL) && (ctrl->ibb != NULL))
			break;

		this_vreg = &ctrl->panel_power_data.vreg_config[rc];

		if (!strcmp(this_vreg->vreg_name, "lab"))
			ctrl->lab = this_vreg->vreg;
		else if (!strcmp(this_vreg->vreg_name, "ibb"))
			ctrl->ibb = this_vreg->vreg;
	}

	if (!ctrl->lab || !ctrl->ibb) {
		pr_err("%s: LAB/IBB regulators not supported.\n", __func__);
		return -EINVAL;
	}

	/*
	 * Set lab/ibb voltage.
	 */
	min_uV = regulator_mgr->lab_output_voltage;
	max_uV = min_uV;
	rc = regulator_set_voltage(ctrl->lab, min_uV, max_uV);
	if (rc) {
		pr_err("%s: Unable to configure of lab voltage.\n", __func__);
		return rc;
	}
	min_uV = regulator_mgr->ibb_output_voltage;
	max_uV = min_uV;
	rc = regulator_set_voltage(ctrl->ibb, min_uV, max_uV);
	if (rc) {
		pr_err("%s: Unable to configure of ibb voltage.\n", __func__);
		return rc;
	}

	/**
	 * Set lab/ibb current max
	 */
	if (regulator_mgr->lab_current_max_enable) {
		rc = qpnp_lab_set_current_max(ctrl->lab,
				regulator_mgr->lab_current_max);
		if (rc) {
			pr_err("%s: Unable to configure of lab current_max.\n",
								__func__);
			return rc;
		}
	}
	if (regulator_mgr->ibb_current_max_enable) {
		rc = qpnp_ibb_set_current_max(ctrl->ibb,
				regulator_mgr->ibb_current_max);
		if (rc) {
			pr_err("%s: Unable to configure of ibb current_max.\n",
								__func__);
			return rc;
		}
	}

	/**
	 * LAB precharge time
	 */
	if (regulator_mgr->fast_prechg_enb) {
		rc = qpnp_lab_set_precharge(ctrl->lab,
			regulator_mgr->lab_fast_precharge_time,
			regulator_mgr->fast_prechg_enb);
		if (rc)
			pr_err("%s: Cannot configure lab precharge\n",
				__func__);
	}

	/**
	 * LAB soft start control
	 */
	if (regulator_mgr->lab_soft_enb) {
		rc = qpnp_lab_set_soft_start(ctrl->lab,
			regulator_mgr->lab_soft_start);
		if (rc)
			pr_err("%s: Cannot configure lab soft start\n",
				__func__);
	}
	if (regulator_mgr->ibb_soft_enb) {
		rc = qpnp_ibb_set_soft_start(ctrl->ibb,
			regulator_mgr->ibb_soft_start);
		if (rc)
			pr_err("%s: Cannot configure ibb soft start\n",
				__func__);
	}

	/**
	 * LAB/IBB pull-down control
	 */
	if (regulator_mgr->lab_pd_enb) {
		rc = qpnp_lab_set_pull_down(ctrl->lab,
			regulator_mgr->lab_pd_full);
		if (rc)
			pr_err("%s: Cannot configure lab pull down\n",
				__func__);
	}
	if (regulator_mgr->ibb_pd_enb) {
		rc = qpnp_ibb_set_pull_down(ctrl->ibb,
			regulator_mgr->ibb_pd_full);
		if (rc)
			pr_err("%s: Cannot configure ibb pull down\n",
				__func__);
	}

	vregs_initialized = true;

	return 0;
}

static int somc_panel_vregs_ctrl(struct mdss_dsi_ctrl_pdata *ctrl, int enable)
{
	int rc;

	if (!ctrl->lab || !ctrl->ibb)
		return -EINVAL;

	pr_debug("%s: ndx=%d enable=%d\n", __func__, ctrl->ndx, enable);

	if (enable) {
		rc = regulator_enable(ctrl->lab);
		if (rc) {
			pr_err("%s: enable failed for lab regulator\n",
							__func__);
			return rc;
		}
		rc = regulator_enable(ctrl->ibb);
		if (rc) {
			pr_err("%s: enable failed for ibb regulator\n",
							__func__);
			regulator_disable(ctrl->lab);
			return rc;
		}

	} else {
		rc = regulator_disable(ctrl->lab);
		if (rc) {
			pr_err("%s: disable failed for lab regulator\n",
							__func__);
			return rc;
		}

		rc = regulator_disable(ctrl->ibb);
		if (rc) {
			pr_err("%s: disable failed for ibb regulator\n",
							__func__);
			return rc;
		}
	}

	return 0;
}

int somc_panel_vregs_dt(struct device_node *np,
			struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	struct mdss_panel_specific_pdata *spec_pdata = NULL;
	struct somc_panel_regulator_mgr *regulator_mgr = NULL;
	int rc = 0;
	u32 tmp;

	spec_pdata = ctrl_pdata->spec_pdata;
	if (!spec_pdata) {
		pr_err("%s: FATAL: NULL SoMC panel data!!\n", __func__);
		return -EINVAL;
	}

	regulator_mgr = spec_pdata->regulator_mgr;

	regulator_mgr->lab_output_voltage = QPNP_REGULATOR_VSP_V_5P4V;
	rc = of_property_read_u32(np, "somc,lab-output-voltage", &tmp);
	if (!rc)
		regulator_mgr->lab_output_voltage = tmp;

	regulator_mgr->ibb_output_voltage = QPNP_REGULATOR_VSN_V_M5P4V;
	rc = of_property_read_u32(np, "somc,ibb-output-voltage", &tmp);
	if (!rc)
		regulator_mgr->ibb_output_voltage = tmp;

	regulator_mgr->lab_current_max_enable = of_find_property(np,
			"qcom,qpnp-lab-limit-maximum-current", &tmp);

	if (regulator_mgr->lab_current_max_enable) {
		rc = of_property_read_u32(np,
			"qcom,qpnp-lab-limit-maximum-current", &tmp);
		if (!rc)
			regulator_mgr->lab_current_max = tmp;
	}

	regulator_mgr->ibb_current_max_enable = of_find_property(np,
			"qcom,qpnp-ibb-limit-maximum-current", &tmp);
	if (regulator_mgr->ibb_current_max_enable) {
		rc = of_property_read_u32(np,
			"qcom,qpnp-ibb-limit-maximum-current", &tmp);
		if (!rc)
			regulator_mgr->ibb_current_max = tmp;
	}

	regulator_mgr->fast_prechg_enb = of_find_property(np,
			"somc,qpnp-lab-max-precharge-enable", &tmp);
	if (regulator_mgr->fast_prechg_enb) {
		rc = of_property_read_u32(np,
			"somc,qpnp-lab-max-precharge-time", &tmp);
		if (!rc)
			regulator_mgr->lab_fast_precharge_time = tmp;
	}

	regulator_mgr->lab_soft_enb = of_find_property(np,
			"somc,qpnp-lab-soft-start", &tmp);
	if (regulator_mgr->lab_soft_enb) {
		rc = of_property_read_u32(np,
			"somc,qpnp-lab-soft-start", &tmp);
		if (!rc)
			regulator_mgr->lab_soft_start = tmp;
	}

	regulator_mgr->ibb_soft_enb = of_find_property(np,
			"somc,qpnp-ibb-discharge-resistor", &tmp);
	if (regulator_mgr->ibb_soft_enb) {
		rc = of_property_read_u32(np,
			"somc,qpnp-ibb-discharge-resistor", &tmp);
		if (!rc)
			regulator_mgr->ibb_soft_start = tmp;
	}

	regulator_mgr->lab_pd_enb = of_find_property(np,
			"somc,qpnp-lab-full-pull-down", &tmp);
	if (regulator_mgr->lab_pd_enb)
		regulator_mgr->lab_pd_full = of_property_read_bool(np,
			"somc,qpnp-lab-full-pull-down");

	regulator_mgr->ibb_pd_enb = of_find_property(np,
			"somc,qpnp-ibb-full-pull-down", &tmp);
	if (regulator_mgr->ibb_pd_enb)
		regulator_mgr->ibb_pd_full = of_property_read_bool(np,
			"somc,qpnp,ibb-full-pull-down");

	regulator_mgr->vreg_init = somc_panel_vregs_init;
	regulator_mgr->vreg_ctrl = somc_panel_vregs_ctrl;

	return 0;
}
