/*
 * Restructured unified display panel driver for Xperia Open Devices
 *                      *** Legacy panels ***
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
#include <linux/qpnp/pin.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/leds.h>
#include <linux/qpnp/pwm.h>
#include <linux/err.h>
#include <linux/regulator/consumer.h>

#include "../mdss_mdp.h"
#include "../mdss_dsi.h"
#include "somc_panels.h"

static int display_power_on[DSI_CTRL_MAX];

static int legacy_panel_parse_dt(struct device_node *np,
			  struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	struct device_node *parent;
	struct mdss_panel_specific_pdata *spec_pdata = NULL;
	u32 tmp = 0;
	int rc = 0;

	spec_pdata = ctrl_pdata->spec_pdata;
	parent = of_get_parent(np);

	rc = of_property_read_u32(np, "somc,disp-dcdc-en-on-pre", &tmp);
	spec_pdata->on_seq.disp_dcdc_en_pre = !rc ? tmp : 0;
	rc = of_property_read_u32(np, "somc,disp-dcdc-en-on-post", &tmp);
	spec_pdata->on_seq.disp_dcdc_en_post = !rc ? tmp : 0;
	rc = of_property_read_u32(np, "somc,disp-dcdc-en-off-pre", &tmp);
	spec_pdata->off_seq.disp_dcdc_en_pre = !rc ? tmp : 0;
	rc = of_property_read_u32(np, "somc,disp-dcdc-en-off-post", &tmp);
	spec_pdata->off_seq.disp_dcdc_en_post = !rc ? tmp : 0;

	spec_pdata->postpwron_no_reset_quirk = of_property_read_bool(
			parent, "somc,postpwron-no-reset-quirk");

	return 0;
}

static int legacy_panel_request_gpios(struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	int rc = 0;

	if (gpio_is_valid(ctrl_pdata->spec_pdata->disp_dcdc_en_gpio)) {
		rc = gpio_request(ctrl_pdata->spec_pdata->disp_dcdc_en_gpio,
						"disp_dcdc_en_gpio");
		if (rc) {
			pr_err("request disp_dcdc_en gpio failed, rc=%d\n", rc);
			goto disp_dcdc_en_gpio_err;
		}
	}

	if (gpio_is_valid(ctrl_pdata->disp_en_gpio)) {
		rc = gpio_request(ctrl_pdata->disp_en_gpio,
						"disp_enable");
		if (rc) {
			pr_err("request disp_en gpio failed, rc=%d\n",
				       rc);
			goto disp_en_gpio_err;
		}
	}

	if (gpio_is_valid(ctrl_pdata->bklt_en_gpio)) {
		rc = gpio_request(ctrl_pdata->bklt_en_gpio,
						"bklt_enable");
		if (rc) {
			pr_err("request bklt gpio failed, rc=%d\n",
				       rc);
			goto bklt_en_gpio_err;
		}
	}
	if (gpio_is_valid(ctrl_pdata->mode_gpio)) {
		rc = gpio_request(ctrl_pdata->mode_gpio, "panel_mode");
		if (rc) {
			pr_err("request panel mode gpio failed,rc=%d\n",
								rc);
			goto mode_gpio_err;
		}
	}

mode_gpio_err:
	if (gpio_is_valid(ctrl_pdata->bklt_en_gpio))
		gpio_free(ctrl_pdata->bklt_en_gpio);
bklt_en_gpio_err:
	if (gpio_is_valid(ctrl_pdata->disp_en_gpio))
		gpio_free(ctrl_pdata->disp_en_gpio);
disp_en_gpio_err:
	if (gpio_is_valid(ctrl_pdata->spec_pdata->disp_dcdc_en_gpio))
		gpio_free(ctrl_pdata->spec_pdata->disp_dcdc_en_gpio);
disp_dcdc_en_gpio_err:
	return rc;
}

static int mdss_dsi_panel_disp_en(struct mdss_panel_data *pdata, int enable)
{
	int rc;
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct mdss_panel_power_seq *pw_seq;
	struct mdss_panel_specific_pdata *spec_pdata = NULL;

	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);
	spec_pdata = ctrl_pdata->spec_pdata;

	if (!gpio_is_valid(ctrl_pdata->disp_en_gpio))
		pr_debug("%s:%d, disp_en line not configured\n",
			   __func__, __LINE__);

	pr_debug("%s: enable = %d\n", __func__, enable);

	if (!spec_pdata->gpios_requested) {
		rc = mdss_dsi_request_gpios(ctrl_pdata);
		if (rc) {
			pr_err("gpio request failed\n");
			return rc;
		}
		spec_pdata->gpios_requested = true;
	}

	pw_seq = (enable) ? &spec_pdata->on_seq : &spec_pdata->off_seq;

	if (gpio_is_valid(spec_pdata->disp_dcdc_en_gpio)) {
		if (enable) {
			if (pw_seq->disp_dcdc_en_pre)
				usleep_range(pw_seq->disp_dcdc_en_pre * 1000,
					pw_seq->disp_dcdc_en_pre * 1000 + 100);

			somc_panel_set_gpio(spec_pdata->disp_dcdc_en_gpio,
					enable);

			if (pw_seq->disp_dcdc_en_post)
				usleep_range(pw_seq->disp_dcdc_en_post * 1000,
					pw_seq->disp_dcdc_en_post * 1000 + 100);
		}
	}

	if (pw_seq->disp_en_pre)
		usleep_range(pw_seq->disp_en_pre * 1000,
				pw_seq->disp_en_pre * 1000 + 100);
	if (gpio_is_valid(ctrl_pdata->disp_en_gpio)) {
		somc_panel_set_gpio(ctrl_pdata->disp_en_gpio, enable);
	}

	if (pw_seq->disp_en_post)
		usleep_range(pw_seq->disp_en_post * 1000,
				pw_seq->disp_en_post * 1000 + 100);

	if (gpio_is_valid(spec_pdata->disp_dcdc_en_gpio)) {
		if (!enable) {
			if (pw_seq->disp_dcdc_en_pre)
				usleep_range(pw_seq->disp_dcdc_en_pre * 1000,
					pw_seq->disp_dcdc_en_pre * 1000 + 100);

			somc_panel_set_gpio(spec_pdata->disp_dcdc_en_gpio,
					enable);

			if (pw_seq->disp_dcdc_en_post)
				usleep_range(pw_seq->disp_dcdc_en_post * 1000,
					pw_seq->disp_dcdc_en_post * 1000 + 100);
		}
	}

	return 0;
}

static int legacy_panel_power_off_ex(struct mdss_panel_data *pdata)
{
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct mdss_panel_specific_pdata *spec_pdata = NULL;
	static int skip_first_off = 1;
	int ret;

	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	if (pdata->panel_info.pdest != DISPLAY_1)
		return 0;

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

	spec_pdata = ctrl_pdata->spec_pdata;
	if (!spec_pdata) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}


	/*
	 * Continuous splash and video mode required the first
	 * pinctrl setup not to be done: set disp_on_in_boot to
	 * false in order to resume normal mdss pinctrl operation.
	 * Now we can safely shut mdss/display down and back up.
	 */
	spec_pdata->disp_on_in_boot = false;

	/* If we have to detect the panel NOW, don't power it off */
	if (skip_first_off && spec_pdata->panel_detect) {
		skip_first_off = 0;
		return 0;
	}

	if (gpio_is_valid(spec_pdata->vsn_gpio) &&
		gpio_is_valid(spec_pdata->vsp_gpio)) {
		gpio_set_value(spec_pdata->vsn_gpio, 0);
		gpio_set_value(spec_pdata->vsp_gpio, 0);
	}

	ret = mdss_dsi_panel_reset(pdata, 0);
	if (ret) {
		pr_err("%s: Failed to disable gpio.\n", __func__);
		return ret;
	}

	ret = mdss_dsi_panel_disp_en(pdata, false);
	if (ret) {
		pr_warn("%s: Disp en failed. rc=%d\n", __func__, ret);
		ret = 0;
	}

	if (!spec_pdata->off_seq.rst_b_seq) {
		ret = mdss_dsi_panel_reset(pdata, 0);
		if (ret) {
			pr_warn("%s: Panel reset failed. rc=%d\n",
							__func__, ret);
			ret = 0;
		}
	}

	if (mdss_dsi_pinctrl_set_state(ctrl_pdata, false))
		pr_info("reset disable: pinctrl not enabled\n");

	ret = msm_dss_enable_vreg(
		ctrl_pdata->panel_power_data.vreg_config,
		ctrl_pdata->panel_power_data.num_vreg, 0);
	if (ret)
		pr_err("%s: failed to disable vregs for %s\n",
			__func__, __mdss_dsi_pm_name(DSI_PANEL_PM));

	if (spec_pdata->down_period)
		spec_pdata->current_period = (u32)ktime_to_ms(ktime_get());
	display_power_on[ctrl_pdata->ndx] = 0;

	return ret;
}

static int legacy_panel_power_on_ex(struct mdss_panel_data *pdata)
{
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct mdss_panel_specific_pdata *spec_pdata = NULL;
	struct somc_panel_regulator_mgr *regulator_mgr = NULL;
	int ret;

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

	spec_pdata = ctrl_pdata->spec_pdata;
	if (!spec_pdata) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	/* 
	 * If display is already on or a dynamic mode switch is
	 * pending, the regulators shall not be turned off or on.
	 */
	if (display_power_on[ctrl_pdata->ndx] ||
	    pdata->panel_info.dynamic_switch_pending)
		return 0;

	if (pdata->panel_info.pdest != DISPLAY_1)
		return 0;

	regulator_mgr = spec_pdata->regulator_mgr;

	if (regulator_mgr->vreg_init)
		regulator_mgr->vreg_init(ctrl_pdata);

	somc_panel_down_period_quirk(spec_pdata);

	ret = msm_dss_enable_vreg(
		ctrl_pdata->panel_power_data.vreg_config,
		ctrl_pdata->panel_power_data.num_vreg, 1);
	if (ret) {
		pr_err("%s: failed to enable vregs for %s\n",
			__func__, __mdss_dsi_pm_name(DSI_PANEL_PM));
		goto vreg_error;
	}

	if (mdss_dsi_pinctrl_set_state(ctrl_pdata, true))
		pr_info("reset enable: pinctrl not enabled\n");

	if (gpio_is_valid(spec_pdata->vsn_gpio) &&
		gpio_is_valid(spec_pdata->vsp_gpio)) {
		usleep_range(19000, 20000);

		somc_panel_set_gpio(spec_pdata->vsn_gpio, 1);
		somc_panel_set_gpio(spec_pdata->vsp_gpio, 1);
	}

	if (spec_pdata->pwron_reset) {
		ret = mdss_dsi_panel_reset(pdata, 1);
		if (ret)
			pr_err("%s: Failed to enable gpio.\n",
						__func__);
	}

	ret = mdss_dsi_panel_disp_en(pdata, true);
	if (ret)
		pr_err("%s: Disp en failed. rc=%d\n",
				__func__, ret);

	/*
	 * If continuous splash screen feature is enabled, then we need to
	 * request all the GPIOs that have already been configured in the
	 * bootloader. This needs to be done irresepective of whether
	 * the lp11_init flag is set or not.
	 */
	if ((pdata->panel_info.cont_splash_enabled ||
		!pdata->panel_info.mipi.lp11_init) &&
		!spec_pdata->postpwron_no_reset_quirk) {
		ret = mdss_dsi_panel_reset(pdata, 1);
		if (ret)
			pr_err("%s: Panel reset failed. rc=%d\n",
					__func__, ret);
	}

	display_power_on[ctrl_pdata->ndx] = 1;

vreg_error:
	if (ret) {
		msm_dss_enable_vreg(
			ctrl_pdata->panel_power_data.vreg_config,
			ctrl_pdata->panel_power_data.num_vreg, 0);
	}

	return ret;
}

int legacy_panel_driver_init(struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	struct mdss_panel_specific_pdata *spec_pdata = NULL;

	spec_pdata = ctrl_pdata->spec_pdata;
	if (!spec_pdata) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	spec_pdata->panel_power_off = legacy_panel_power_off_ex;
	spec_pdata->panel_power_on = legacy_panel_power_on_ex;
	spec_pdata->dsi_request_gpios = legacy_panel_request_gpios;
	spec_pdata->parse_specific_dt = legacy_panel_parse_dt;

	return 0;
}

