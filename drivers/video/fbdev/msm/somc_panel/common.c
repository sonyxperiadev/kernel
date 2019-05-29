/*
 * Restructured unified display panel driver for Xperia Open Devices
 *                *** Common generic helpers ***
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
#include <linux/interrupt.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/of_platform.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/leds.h>
#include <linux/err.h>
#include <linux/regulator/consumer.h>

#include "../mdss_mdp.h"
#include "../mdss_dsi.h"
#include "somc_panels.h"

/*
 * somc_panel_set_gpio - GPIO setting helper
 *
 * Sets GPIOs to the desired values by also
 * taking care about handling its direction.
 *
 * Note: This function assumes that GPIO is VALID!!!
 */
int somc_panel_set_gpio(int gpio, int enable)
{
	int rc = 0;

	if (enable) {
		rc = gpio_direction_output(gpio, 1);
		if (rc) {
			pr_debug("%s: Failed to set GPIO %d direction!!\n",
				 __func__, gpio);
			return rc;
		}
	} else {
		gpio_set_value(gpio, 0);
	}

	return rc;
}

int somc_panel_vreg_name_to_config(
		struct mdss_dsi_ctrl_pdata *ctrl_pdata,
		struct dss_vreg *config, char *name)
{
	struct dss_vreg *vreg_config = ctrl_pdata->panel_power_data.vreg_config;
	int num_vreg = ctrl_pdata->panel_power_data.num_vreg;
	int i = 0;
	int valid = -EINVAL;

	for (i = 0; i < num_vreg; i++) {
		if (!strcmp(name, vreg_config[i].vreg_name)) {
			*config = vreg_config[i];
			valid = 0;
			break;
		}
	}

	return valid;
}

int somc_panel_vreg_ctrl(
		struct mdss_dsi_ctrl_pdata *ctrl_pdata,
		char *vreg, bool enable)
{
	struct dss_vreg vreg_config;
	struct mdss_panel_power_seq *pw_seq = NULL;
	int valid = 0;
#ifdef CONFIG_FBDEV_SOMC_PANEL_INCELL
	int wait = 0;
#endif
	int ret = 0;

	valid = somc_panel_vreg_name_to_config(
			ctrl_pdata, &vreg_config, vreg);

	if (!valid) {
		if (enable) {
			ret = msm_dss_enable_vreg(&vreg_config, 1, 1);
			pw_seq = &ctrl_pdata->spec_pdata->on_seq;
		} else {
			ret = msm_dss_enable_vreg(&vreg_config, 1, 0);
			pw_seq = &ctrl_pdata->spec_pdata->off_seq;
		}

#ifdef CONFIG_FBDEV_SOMC_PANEL_INCELL
		if (!strcmp(vreg, "vdd"))
			wait = pw_seq->disp_vdd;
		else if (!strcmp(vreg, "vddio"))
			wait = pw_seq->disp_vddio;
		else if (!strcmp(vreg, "lab"))
			wait = pw_seq->disp_vsp;
		else if (!strcmp(vreg, "ibb"))
			wait = pw_seq->disp_vsn;
		else if (!strcmp(vreg, "touch-avdd"))
			wait = pw_seq->touch_avdd;
		else
			wait = 0;

		if (!ret && wait) {
			usleep_range(wait * 1000, wait * 1000 + 100);
		}
#endif
	}

	return ret;
}


int somc_panel_allocate(struct platform_device *pdev,
		struct mdss_dsi_ctrl_pdata *ctrl)
{
	ctrl->spec_pdata = devm_kzalloc(&pdev->dev,
		sizeof(struct mdss_panel_specific_pdata),
		GFP_KERNEL);
	if (!ctrl->spec_pdata) {
		pr_err("%s: FAILED: cannot allocate spec_pdata\n", __func__);
		goto fail_specific;
	};

	ctrl->spec_pdata->color_mgr = devm_kzalloc(&pdev->dev,
		sizeof(struct somc_panel_color_mgr), GFP_KERNEL);
	if (!ctrl->spec_pdata->color_mgr) {
		pr_err("%s: FAILED: Cannot allocate color_mgr\n", __func__);
		goto fail_color_mgr;
	};

	ctrl->spec_pdata->regulator_mgr = devm_kzalloc(&pdev->dev,
		sizeof (struct somc_panel_regulator_mgr), GFP_KERNEL);
	if (!ctrl->spec_pdata->regulator_mgr) {
		pr_err("%s: FAILED: Cannot allocate regulator_mgr\n",
			__func__);
		goto fail_regulator_mgr;
	};

	return 0;

fail_regulator_mgr:
	devm_kfree(&pdev->dev, ctrl->spec_pdata->regulator_mgr);
fail_color_mgr:
	devm_kfree(&pdev->dev, ctrl->spec_pdata->color_mgr);
fail_specific:
	devm_kfree(&pdev->dev, ctrl->spec_pdata);

	return -ENOMEM;
}
