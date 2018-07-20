/*
 * Restructured unified display panel driver for Xperia Open Devices
 *                  *** Special VREG handling ***
 *
 * This driver is based on various SoMC implementations found in
 * copyleft archives for various devices.
 *
 * Copyright (c) 2012-2014, The Linux Foundation. All rights reserved.
 * Copyright (c) Sony Mobile Communications Inc. All rights reserved.
 * Copyright (C) 2014-2018, AngeloGioacchino Del Regno <kholk11@gmail.com>
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

#include <linux/sysfs.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/notifier.h>
#include <linux/export.h>
#include <linux/regulator/qpnp-labibb-regulator.h>
#include "somc_panel_exts.h"
#include "dsi_display.h"
#include "dsi_panel.h"

int somc_panel_vregs_parse_dt(struct dsi_panel *panel,
					struct device_node *np)
{
	struct panel_specific_pdata *spec_pdata = NULL;
	struct dsi_panel_labibb_data *labibb = NULL;
	u32 tmp = 0;
	int rc = 0;

	if (!panel) {
		pr_err("%s: Invalid input panel\n", __func__);
		return -EINVAL;
	}

	spec_pdata = panel->spec_pdata;

	/* Parsing lab/ibb register settings */
	labibb = &spec_pdata->labibb;

	memset(labibb, 0, sizeof(*labibb));
	if (of_find_property(np, "somc,lab-output-voltage", &tmp))
		labibb->labibb_ctrl_state |= OVR_LAB_VOLTAGE;

	if (of_find_property(np, "somc,ibb-output-voltage", &tmp))
		labibb->labibb_ctrl_state |= OVR_IBB_VOLTAGE;

	if (of_find_property(np, "somc,qpnp-lab-limit-maximum-current", &tmp))
		labibb->labibb_ctrl_state |= OVR_LAB_CURRENT_MAX;

	if (of_find_property(np, "somc,qpnp-ibb-limit-maximum-current", &tmp))
		labibb->labibb_ctrl_state |= OVR_IBB_CURRENT_MAX;

	if (of_find_property(np, "somc,qpnp-lab-max-precharge-time", &tmp))
		labibb->labibb_ctrl_state |= OVR_LAB_PRECHARGE_CTL;

	if (of_find_property(np, "somc,qpnp-lab-soft-start", &tmp))
		labibb->labibb_ctrl_state |= OVR_LAB_SOFT_START_CTL;

	if (of_find_property(np, "somc,qpnp-ibb-discharge-resistor", &tmp))
		labibb->labibb_ctrl_state |= OVR_IBB_SOFT_START_CTL;

	if (of_find_property(np, "somc,qpnp-lab-pull-down-enable", &tmp))
		labibb->labibb_ctrl_state |= OVR_LAB_PD_CTL;

	if (of_find_property(np, "somc,qpnp-ibb-pull-down-enable", &tmp))
		labibb->labibb_ctrl_state |= OVR_IBB_PD_CTL;

	labibb->lab_output_voltage = QPNP_REGULATOR_VSP_V_5P4V;
	if (((labibb->labibb_ctrl_state) & OVR_LAB_VOLTAGE)) {
		rc = of_property_read_u32(np, "somc,lab-output-voltage", &tmp);
		if (!rc)
			labibb->lab_output_voltage = tmp;
	}

	labibb->ibb_output_voltage = QPNP_REGULATOR_VSN_V_M5P4V;
	if (((labibb->labibb_ctrl_state) & OVR_IBB_VOLTAGE)) {
		rc = of_property_read_u32(np, "somc,ibb-output-voltage", &tmp);
		if (!rc)
			labibb->ibb_output_voltage = tmp;
	}

	labibb->lab_current_max = LAB_CURRENT_MAX;
	if (((labibb->labibb_ctrl_state) & OVR_LAB_CURRENT_MAX)) {
		rc = of_property_read_u32(np,
			"somc,qpnp-lab-limit-maximum-current", &tmp);
		if (!rc)
			labibb->lab_current_max = tmp;
	}

	labibb->ibb_current_max = IBB_CURRENT_MAX;
	if (((labibb->labibb_ctrl_state) & OVR_IBB_CURRENT_MAX)) {
		rc = of_property_read_u32(np,
			"somc,qpnp-ibb-limit-maximum-current", &tmp);
		if (!rc)
			labibb->ibb_current_max = tmp;
	}

	labibb->lab_fast_precharge_time = LAB_FAST_PRECHARGE_TIME;
	labibb->lab_fast_precharge_en = false;
	if (((labibb->labibb_ctrl_state) & OVR_LAB_PRECHARGE_CTL)) {
		rc = of_property_read_u32(np,
			"somc,qpnp-lab-max-precharge-time", &tmp);
		if (!rc)
			labibb->lab_fast_precharge_time = tmp;

		labibb->lab_fast_precharge_en = of_property_read_bool(np,
					"somc,qpnp-lab-max-precharge-enable");
	}

	labibb->lab_soft_start = LAB_SOFT_START_TIME;
	if (((labibb->labibb_ctrl_state) & OVR_LAB_SOFT_START_CTL)) {
		rc = of_property_read_u32(np,
			"somc,qpnp-lab-soft-start", &tmp);
		if (!rc)
			labibb->lab_soft_start = tmp;
	}

	labibb->ibb_soft_start = IBB_SOFT_START_RESISTOR;
	if (((labibb->labibb_ctrl_state) & OVR_IBB_SOFT_START_CTL)) {
		rc = of_property_read_u32(np,
			"somc,qpnp-ibb-discharge-resistor", &tmp);
		if (!rc)
			labibb->ibb_soft_start = tmp;
	}

	labibb->lab_pd_full = false;
	if (((labibb->labibb_ctrl_state) & OVR_LAB_PD_CTL))
		labibb->lab_pd_full = of_property_read_bool(np,
					"somc,qpnp-lab-full-pull-down");

	labibb->ibb_pd_full = false;
	if (((labibb->labibb_ctrl_state) & OVR_IBB_PD_CTL))
		labibb->ibb_pd_full = of_property_read_bool(np,
					"somc,qpnp-ibb-full-pull-down");

	return rc;
}

void somc_panel_vregs_init(struct dsi_panel *panel)
{
	int ret;
	int min_uV, max_uV = 0;
	struct panel_specific_pdata *spec_pdata = NULL;
	struct dsi_panel_labibb_data *labibb = NULL;
	struct dsi_vreg lab_vreg, ibb_vreg;

	spec_pdata = panel->spec_pdata;
	if (!spec_pdata) {
		pr_err("%s: Invalid specific panel data\n", __func__);
		return;
	}

	labibb = &(spec_pdata->labibb);
	if (!labibb) {
		pr_err("%s: Invalid regulator settings\n", __func__);
		return;
	}

	/*
	 * Get lab/ibb info.
	 */
	ret = vreg_name_to_config(
			&spec_pdata->vspvsn_power_info, &lab_vreg, "lab");
	if (ret) {
		pr_err("%s: lab not registered\n", __func__);
		return;
	}

	ret = vreg_name_to_config(
			&spec_pdata->vspvsn_power_info, &ibb_vreg, "ibb");
	if (ret) {
		pr_err("%s: ibb not registered\n", __func__);
		return;
	}

	/*
	 * Set lab/ibb voltage.
	 */

	if (((labibb->labibb_ctrl_state) & OVR_LAB_VOLTAGE)) {
		min_uV = labibb->lab_output_voltage;
		max_uV = min_uV;
		ret = regulator_set_voltage(lab_vreg.vreg,
							min_uV, max_uV);
		if (ret)
			pr_err("%s: Unable to configure of lab voltage.\n",
								__func__);
	}

	if (((labibb->labibb_ctrl_state) & OVR_IBB_VOLTAGE)) {
		min_uV = labibb->ibb_output_voltage;
		max_uV = min_uV;
		ret = regulator_set_voltage(ibb_vreg.vreg, min_uV, max_uV);
		if (ret)
			pr_err("%s: Unable to configure of ibb voltage.\n",
								__func__);
	}

	/*
	 * Set lab/ibb current max
	 */
	if (((labibb->labibb_ctrl_state) & OVR_LAB_CURRENT_MAX)) {
		ret = qpnp_lab_set_current_max(lab_vreg.vreg,
						labibb->lab_current_max);
		if (ret)
			pr_err("%s: Unable to configure of lab current_max.\n",
								__func__);
	}

	if (((labibb->labibb_ctrl_state) & OVR_IBB_CURRENT_MAX)) {
		ret = qpnp_ibb_set_current_max(ibb_vreg.vreg,
						labibb->ibb_current_max);
		if (ret)
			pr_err("%s: Unable to configure of ibb current_max.\n",
								__func__);
	}

	/*
	 * Set lab precharge
	 */
	if (((labibb->labibb_ctrl_state) & OVR_LAB_PRECHARGE_CTL)) {
		ret = qpnp_lab_set_precharge(lab_vreg.vreg,
					labibb->lab_fast_precharge_time,
					labibb->lab_fast_precharge_en);
		if (ret)
			pr_err("%s: Unable to configure of lab precharge.\n",
								__func__);
	}

	/*
	 * Set lab/ibb soft-start control
	 */
	if (((labibb->labibb_ctrl_state) & OVR_LAB_SOFT_START_CTL)) {
		ret = qpnp_lab_set_soft_start(lab_vreg.vreg,
						labibb->lab_soft_start);
		if (ret)
			pr_err("%s: Unable to configure of lab soft-start.\n",
								__func__);
	}

	if (((labibb->labibb_ctrl_state) & OVR_IBB_SOFT_START_CTL)) {
		ret = qpnp_ibb_set_soft_start(ibb_vreg.vreg,
						labibb->ibb_soft_start);
		if (ret)
			pr_err("%s: Unable to configure of ibb soft-start.\n",
							__func__);
	}

	/*
	 * Set lab/ibb pull-down control
	 */
	if (((labibb->labibb_ctrl_state) & OVR_LAB_PD_CTL)) {
		ret = qpnp_lab_set_pull_down(lab_vreg.vreg,
						labibb->lab_pd_full);
		if (ret)
			pr_err("%s: Unable to configure of lab pull-down.\n",
								__func__);
	}

	if (((labibb->labibb_ctrl_state) & OVR_IBB_PD_CTL)) {
		ret = qpnp_ibb_set_pull_down(ibb_vreg.vreg,
						labibb->ibb_pd_full);
		if (ret)
			pr_err("%s: Unable to configure of ibb pull-down.\n",
								__func__);
	}
}

