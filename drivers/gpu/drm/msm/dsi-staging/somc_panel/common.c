/*
 * Restructured unified display panel driver for Xperia Open Devices
 *                *** Common generic helpers ***
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

int vreg_name_to_config(struct dsi_regulator_info *regs,
			struct dsi_vreg *config, char *name)
{
	int num_vreg = regs->count;
	int i = 0;
	int valid = -EINVAL;

	for (i = 0; i < num_vreg; i++) {
		if (!strcmp(name, regs->vregs[i].vreg_name)) {
			*config = regs->vregs[i];
			valid = 0;
			break;
		}
	}

	return valid;
}

int somc_panel_vreg_ctrl(struct dsi_regulator_info *regs,
				char *vreg_name, bool enable)
{
	struct dsi_vreg vreg;
	int num_of_v = 0;
	int rc = 0;
	int is_invalid = 0;

	is_invalid = vreg_name_to_config(regs, &vreg, vreg_name);

	if (!is_invalid) {
		if (enable) {
			pr_debug("%s: vreg on, name:%s\n", __func__,
							vreg.vreg_name);

			if (vreg.pre_on_sleep)
				usleep_range(vreg.pre_on_sleep * 1000,
						vreg.pre_on_sleep * 1000 + 100);

			rc = regulator_set_load(vreg.vreg,
						vreg.enable_load);
			if (rc < 0) {
				pr_err("Setting optimum mode failed for %s\n",
				       vreg.vreg_name);
				goto error;
			}
			num_of_v = regulator_count_voltages(vreg.vreg);
			if (num_of_v > 0) {
				rc = regulator_set_voltage(vreg.vreg,
							   vreg.min_voltage,
							   vreg.max_voltage);
				if (rc) {
					pr_err("Set voltage(%s) fail, rc=%d\n",
						 vreg.vreg_name, rc);
					goto error;
				}
			}

			rc = regulator_enable(vreg.vreg);
			if (rc) {
				pr_err("enable failed for %s, rc=%d\n",
				       vreg.vreg_name, rc);
				goto error;
			}

			if (vreg.post_on_sleep)
				usleep_range(vreg.post_on_sleep * 1000,
						vreg.post_on_sleep * 1000 + 100);
		} else {
			pr_debug("%s: vreg off, name:%s\n", __func__,
							vreg.vreg_name);

			if (vreg.pre_off_sleep)
				usleep_range(vreg.pre_off_sleep * 1000,
						vreg.pre_off_sleep * 1000 + 100);

			(void)regulator_set_load(vreg.vreg,
						vreg.disable_load);
			(void)regulator_disable(vreg.vreg);

			if (vreg.post_off_sleep)
				usleep_range(vreg.post_off_sleep * 1000,
						vreg.post_off_sleep * 1000 + 100);
		}
	}

	return 0;
error:
	return rc;
}


int somc_panel_regulators_get(struct dsi_panel *panel)
{
	struct regulator *vreg = NULL;
	struct panel_specific_pdata *spec_pdata = NULL;
	int rc = 0;
	int j, k;

	if (!panel) {
		pr_err("%s: Invalid input panel\n", __func__);
		return -EINVAL;
	}
	spec_pdata = panel->spec_pdata;

	for (j = 0; j < spec_pdata->touch_power_info.count; j++) {
		vreg = devm_regulator_get(panel->parent,
			spec_pdata->touch_power_info.vregs[j].vreg_name);
		rc = PTR_RET(vreg);
		if (rc) {
			pr_err("failed to get %s regulator\n",
			       spec_pdata->touch_power_info.vregs[j].vreg_name);
			goto error_touch_put;
		}
		spec_pdata->touch_power_info.vregs[j].vreg = vreg;
	}
	for (k = 0; k < spec_pdata->vspvsn_power_info.count; k++) {
		vreg = devm_regulator_get(panel->parent,
			spec_pdata->vspvsn_power_info.vregs[k].vreg_name);
		rc = PTR_RET(vreg);
		if (rc) {
			pr_err("failed to get %s regulator\n",
			     spec_pdata->vspvsn_power_info.vregs[k].vreg_name);
			goto error_vspvsn_put;
		}
		spec_pdata->vspvsn_power_info.vregs[k].vreg = vreg;
	}

	return rc;
error_vspvsn_put:
	for (k = k - 1; k >= 0; k--) {
		devm_regulator_put(
			spec_pdata->vspvsn_power_info.vregs[k].vreg);
		spec_pdata->vspvsn_power_info.vregs[k].vreg = NULL;
	}
error_touch_put:
	for (j = j - 1; j >= 0; j--) {
		devm_regulator_put(
			spec_pdata->touch_power_info.vregs[j].vreg);
		spec_pdata->touch_power_info.vregs[j].vreg = NULL;
	}
	return rc;
}

int somc_panel_regulators_put(struct dsi_panel *panel)
{
	struct panel_specific_pdata *spec_pdata = NULL;
	int rc = 0;
	int i;

	if (!panel) {
		pr_err("%s: Invalid input panel\n", __func__);
		return -EINVAL;
	}
	spec_pdata = panel->spec_pdata;

	for (i = spec_pdata->touch_power_info.count - 1; i >= 0; i--)
		devm_regulator_put(spec_pdata->touch_power_info.vregs[i].vreg);
	for (i = spec_pdata->vspvsn_power_info.count - 1; i >= 0; i--)
		devm_regulator_put(spec_pdata->vspvsn_power_info.vregs[i].vreg);

	return rc;
}

int somc_panel_allocate(struct device *dev, struct dsi_panel *panel)
{
	panel->spec_pdata = devm_kzalloc(dev,
		sizeof(struct panel_specific_pdata),
		GFP_KERNEL);
	if (!panel->spec_pdata) {
		pr_err("%s: FAILED: cannot allocate spec_pdata\n", __func__);
		goto fail_specific;
	};

	panel->spec_pdata->color_mgr = devm_kzalloc(dev,
		sizeof(struct somc_panel_color_mgr), GFP_KERNEL);
	if (!panel->spec_pdata->color_mgr) {
		pr_err("%s: FAILED: Cannot allocate color_mgr\n", __func__);
		goto fail_color_mgr;
	};

/*
	panel->spec_pdata->regulator_mgr = devm_kzalloc(dev,
		sizeof (struct somc_panel_regulator_mgr), GFP_KERNEL);
	if (!panel->spec_pdata->regulator_mgr) {
		pr_err("%s: FAILED: Cannot allocate regulator_mgr\n",
			__func__);
		goto fail_regulator_mgr;
	};
*/
	return 0;
/*
fail_regulator_mgr:
	devm_kfree(dev, panel->spec_pdata->regulator_mgr);
*/
fail_color_mgr:
	devm_kfree(dev, panel->spec_pdata->color_mgr);
fail_specific:
	devm_kfree(dev, panel->spec_pdata);

	return -ENOMEM;
}
