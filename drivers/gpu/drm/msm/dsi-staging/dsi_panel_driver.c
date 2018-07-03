/* Copyright (c) 2015-2017, The Linux Foundation. All rights reserved.
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
/*
 * Copyright (C) 2017 Sony Mobile Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 */

#include <linux/sysfs.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/notifier.h>
#include <linux/export.h>
#include <linux/drm_notify.h>
#include <linux/regulator/qpnp-labibb-regulator.h>
#include <linux/sde_io_util.h>
#include "dsi_panel_driver.h"
#include "dsi_display.h"
#include "dsi_panel.h"

static BLOCKING_NOTIFIER_HEAD(drm_notifier_list);

#define ADC_PNUM		2
#define ADC_RNG_MIN		0
#define ADC_RNG_MAX		1

static u32 down_period;
static unsigned long lcdid_adc = 1505000;

static char *res_buf;
static int buf_sz;
#define DSI_BUF_SIZE 1024
#define TMP_BUF_SZ 128
#define MAX_WRITE_DATA 100

#define NOT_CHARGEMON_EXIT 0
static int s_chargemon_exit = NOT_CHARGEMON_EXIT;

static int __init lcdid_adc_setup(char *str)
{
	unsigned long res;

	if (!*str)
		return 0;
	if (!kstrtoul(str, 0, &res)) {
		lcdid_adc = res;
	}

	return 1;
}
__setup("lcdid_adc=", lcdid_adc_setup);

void dsi_panel_driver_detection(
		struct platform_device *pdev,
		struct device_node **np)
{
	u32 res[ADC_PNUM];
	int rc = 0;
	struct device_node *parent;
	struct device_node *next;
	u32 dev_index = 0;
	u32 dsi_index = 0;
	u32 adc_uv = 0;

	parent = of_get_parent(*np);

	adc_uv = lcdid_adc;
	pr_notice("%s: physical:%d\n", __func__, adc_uv);

	for_each_child_of_node(parent, next) {
		rc = of_property_read_u32(next, "somc,dsi-index", &dsi_index);
		if (rc)
			dsi_index = 0;
		if (dsi_index != dev_index)
			continue;

		rc = of_property_read_u32_array(next,
				"somc,lcd-id-adc", res, ADC_PNUM);
		if (rc)
			continue;
		if (adc_uv < res[ADC_RNG_MIN] || res[ADC_RNG_MAX] < adc_uv)
			continue;

		*np = next;
		break;
	}
}

int drm_register_client(struct notifier_block *nb)
{
	return blocking_notifier_chain_register(&drm_notifier_list, nb);
}
EXPORT_SYMBOL(drm_register_client);

int drm_unregister_client(struct notifier_block *nb)
{
	return blocking_notifier_chain_unregister(&drm_notifier_list, nb);
}
EXPORT_SYMBOL(drm_unregister_client);

int drm_notifier_call_chain(unsigned long val, void *v)
{
	return blocking_notifier_call_chain(&drm_notifier_list, val, v);
}
EXPORT_SYMBOL_GPL(drm_notifier_call_chain);

static int dsi_panel_driver_vreg_name_to_config(
		struct dsi_regulator_info *regs,
		struct dsi_vreg *config,
		char *name)
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

static int dsi_panel_driver_vreg_ctrl(
		struct dsi_regulator_info *regs, char *vreg_name, bool enable)
{
	struct dsi_vreg vreg;
	int num_of_v = 0;
	int rc = 0;
	int valid = 0;

	valid = dsi_panel_driver_vreg_name_to_config(
			regs, &vreg, vreg_name);

	if (!valid) {
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

int dsi_panel_driver_vreg_get(struct dsi_panel *panel)
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

int dsi_panel_driver_vreg_put(struct dsi_panel *panel)
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

int dsi_panel_driver_pinctrl_init(struct dsi_panel *panel)
{
	panel->pinctrl.touch_state_active
		= pinctrl_lookup_state(panel->pinctrl.pinctrl,
				SDE_PINCTRL_STATE_TOUCH_ACTIVE);
	if (IS_ERR_OR_NULL(panel->pinctrl.touch_state_active))
		pr_warn("%s: can not get touch active pinstate\n", __func__);

	panel->pinctrl.touch_state_suspend
		= pinctrl_lookup_state(panel->pinctrl.pinctrl,
				SDE_PINCTRL_STATE_TOUCH_SUSPEND);
	if (IS_ERR_OR_NULL(panel->pinctrl.touch_state_suspend))
		pr_warn("%s: can not get touch suspend pinstate\n", __func__);

	dsi_panel_driver_labibb_vreg_init(panel);

	return 0;
}

static int dsi_panel_driver_touch_pinctrl_set_state(
	struct dsi_panel *panel,
	bool active)
{
	struct panel_specific_pdata *spec_pdata = NULL;
	struct pinctrl_state *pin_state;
	int rc = -EFAULT;
	int wait = 0;

	if (!panel) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	spec_pdata = panel->spec_pdata;

	pin_state = active ? panel->pinctrl.touch_state_active
				: panel->pinctrl.touch_state_suspend;

	if (!IS_ERR_OR_NULL(pin_state)) {
		rc = pinctrl_select_state(panel->pinctrl.pinctrl,
				pin_state);
		if (!rc) {
			wait = active ? spec_pdata->touch_intn
					: spec_pdata->touch_intn_off;
			if (wait)
				usleep_range(wait * 1000, wait * 1000 + 100);
		} else {
			pr_err("%s: can not set %s pins\n", __func__,
			       active ? SDE_PINCTRL_STATE_TOUCH_ACTIVE
			       : SDE_PINCTRL_STATE_TOUCH_SUSPEND);
		}
	} else {
		pr_err("%s: invalid '%s' pinstate\n", __func__,
		       active ? SDE_PINCTRL_STATE_TOUCH_ACTIVE
		       : SDE_PINCTRL_STATE_TOUCH_SUSPEND);
	}

	return rc;
}

int dsi_panel_driver_gpio_request(struct dsi_panel *panel)
{
	struct panel_specific_pdata *spec_pdata = NULL;
	int rc;

	if (!panel) {
		pr_err("%s: Invalid input panel\n", __func__);
		return -EINVAL;
	}
	spec_pdata = panel->spec_pdata;

	if (gpio_is_valid(spec_pdata->touch_vddio_en_gpio)) {
		rc = gpio_request(spec_pdata->touch_vddio_en_gpio,
							"touch_vddio_en");
		if (rc) {
			pr_err("request for touch_vddio_en failed, rc=%d\n",
									rc);
			goto error_release_touch_vddio_en;
		}
	}
	if (gpio_is_valid(spec_pdata->reset_touch_gpio)) {
		rc = gpio_request(spec_pdata->reset_touch_gpio,
							"reset_touch_gpio");
		if (rc) {
			pr_err("request for reset_gpio failed, rc=%d\n", rc);
			goto error_release_touch_reset;
		}
	}
	if (gpio_is_valid(spec_pdata->disp_dcdc_en_gpio)) {
		rc = gpio_request(spec_pdata->disp_dcdc_en_gpio,
						"disp_dcdc_en_gpio");
		if (rc) {
			pr_err("request disp_dcdc_en gpio failed, rc=%d\n", rc);
			goto error_release_disp_dcdc_en;
		}
	}

	goto error;

error_release_disp_dcdc_en:
	if (gpio_is_valid(spec_pdata->disp_dcdc_en_gpio))
		gpio_free(spec_pdata->disp_dcdc_en_gpio);
error_release_touch_reset:
	if (gpio_is_valid(spec_pdata->reset_touch_gpio))
		gpio_free(spec_pdata->reset_touch_gpio);
error_release_touch_vddio_en:
	if (gpio_is_valid(spec_pdata->touch_vddio_en_gpio))
		gpio_free(spec_pdata->touch_vddio_en_gpio);
error:
	return rc;
}

int dsi_panel_driver_gpio_release(struct dsi_panel *panel)
{
	struct panel_specific_pdata *spec_pdata = NULL;
	int rc = 0;

	if (!panel) {
		pr_err("%s: Invalid input panel\n", __func__);
		return -EINVAL;
	}
	spec_pdata = panel->spec_pdata;

	if (gpio_is_valid(spec_pdata->reset_touch_gpio))
		gpio_free(spec_pdata->reset_touch_gpio);

	if (gpio_is_valid(spec_pdata->touch_vddio_en_gpio))
		gpio_free(spec_pdata->touch_vddio_en_gpio);

	if (gpio_is_valid(spec_pdata->disp_dcdc_en_gpio))
		gpio_free(spec_pdata->disp_dcdc_en_gpio);

	return rc;
}

int dsi_panel_driver_touch_reset(struct dsi_panel *panel)
{
	struct panel_specific_pdata *spec_pdata = NULL;
	int rc, i;

	if (!panel) {
		pr_err("%s: Invalid input panel\n", __func__);
		return -EINVAL;
	}
	spec_pdata = panel->spec_pdata;

	if (spec_pdata->count_touch) {
		rc = gpio_direction_output(spec_pdata->reset_touch_gpio,
			spec_pdata->sequence_touch[0].level);
		if (rc) {
			pr_err("unable to set dir for rst touch gpio rc=%d\n", rc);
			goto exit;
		}
	}
	for (i = 0; i < spec_pdata->count_touch; i++) {
		gpio_set_value(spec_pdata->reset_touch_gpio,
			       spec_pdata->sequence_touch[i].level);

		if (spec_pdata->sequence_touch[i].sleep_ms)
			usleep_range(
				spec_pdata->sequence_touch[i].sleep_ms * 1000,
				spec_pdata->sequence_touch[i].sleep_ms * 1000);
	}

exit:
	return rc;
}

int dsi_panel_driver_touch_reset_ctrl(struct dsi_panel *panel, bool en)
{
	struct panel_specific_pdata *spec_pdata = NULL;
	int rc = 0;

	if (!panel) {
		pr_err("%s: Invalid input panel\n", __func__);
		return -EINVAL;
	}
	spec_pdata = panel->spec_pdata;

	gpio_set_value(spec_pdata->reset_touch_gpio, 0);
	usleep_range(spec_pdata->touch_reset_off * 1000,
			spec_pdata->touch_reset_off * 1000);
	gpio_set_value(spec_pdata->reset_touch_gpio, 1);

	return rc;
}

int dsi_panel_driver_reset_panel(struct dsi_panel *panel, bool en)
{
	struct dsi_panel_reset_config *r_config = &panel->reset_config;
	struct panel_specific_pdata *spec_pdata = NULL;
	struct dsi_reset_cfg *seq = NULL;
	int i;
	int rc = 0;

	if (!panel) {
		pr_err("%s: Invalid input panel\n", __func__);
		return -EINVAL;
	}
	spec_pdata = panel->spec_pdata;

	if (!gpio_is_valid(panel->reset_config.reset_gpio)) {
		pr_debug("%s:%d, panel reset line not configured\n",
			   __func__, __LINE__);
		goto exit;
	}

	pr_debug("%s: enable = %d\n", __func__, en);

	if (en)
		seq = &spec_pdata->on_seq;
	else
		seq = &spec_pdata->off_seq;

	if (seq->count) {
		pr_debug("%s: first level=%d\n",
				__func__, seq->seq[0].level);

		rc = gpio_direction_output(r_config->reset_gpio,
			seq->seq[0].level);
		if (rc) {
			pr_err("unable to set dir for rst gpio rc=%d\n", rc);
			goto exit;
		}
	}

	for (i = 0; i < seq->count; i++) {
		gpio_set_value(r_config->reset_gpio,
			       seq->seq[i].level);

		if (seq->seq[i].sleep_ms)
			usleep_range(seq->seq[i].sleep_ms * 1000,
				     seq->seq[i].sleep_ms * 1000);

		pr_debug("%s: level=%d, wait=%dms\n", __func__,
				seq->seq[i].level, seq->seq[i].sleep_ms);
	}

exit:
	return rc;
}

int dsi_panel_driver_touch_power(struct dsi_panel *panel, bool enable)
{
	struct panel_specific_pdata *spec_pdata = NULL;
	int rc = 0;

	if (!panel) {
		pr_err("%s: Invalid input panel\n", __func__);
		return -EINVAL;
	}
	spec_pdata = panel->spec_pdata;

	if (gpio_is_valid(spec_pdata->touch_vddio_en_gpio)) {
		if (enable)
			rc = gpio_direction_output(
				spec_pdata->touch_vddio_en_gpio, 0);
		else
			rc = gpio_direction_output(
				spec_pdata->touch_vddio_en_gpio, 1);
		if (rc) {
			pr_err("unable to set dir for touch_vddio_en gpio rc=%d\n", rc);
			goto exit;
		}
	}
exit:
	return rc;
}

static void dsi_panel_driver_power_off_ctrl(void)
{
	struct incell_ctrl *incell = incell_get_info();
	incell_pw_seq p_seq = POWER_EXECUTE;

	switch (incell->state) {
	case INCELL_S100:
	case INCELL_S110:
		p_seq = POWER_SKIP;
		break;
	case INCELL_S101:
		p_seq = POWER_EXECUTE;
		break;
	case INCELL_S111:
		pr_notice("%s: Power on keep in lock state\n", __func__);
		p_seq = POWER_SKIP;
		break;
	case INCELL_S001:
		pr_notice("%s: Power off by unlock\n", __func__);
		p_seq = POWER_EXECUTE;
		break;
	default:
		pr_err("%s: Already power off\n", __func__);
		p_seq = POWER_SKIP;
		break;
	}

	incell->state &= INCELL_SYSTEM_STATE_OFF;

	incell->seq = p_seq;
}

static void dsi_panel_driver_power_on_ctrl(void)
{
	struct incell_ctrl *incell = incell_get_info();
	incell_pw_seq p_seq = POWER_EXECUTE;

	switch (incell->state) {
	case INCELL_S000:
	case INCELL_S010:
		p_seq = POWER_EXECUTE;
		break;
	case INCELL_S001:
	case INCELL_S011:
		p_seq = POWER_SKIP;
		break;
	default:
		pr_err("%s: Already power on\n", __func__);
		p_seq = POWER_SKIP;
		break;
	}

	incell->state |= INCELL_SYSTEM_STATE_ON;

	incell->seq = p_seq;
}

int dsi_panel_driver_power_off(struct dsi_panel *panel)
{
	struct panel_specific_pdata *spec_pdata = NULL;
	struct incell_ctrl *incell = incell_get_info();
	int rc = 0;

	dsi_panel_driver_power_off_ctrl();
	if (incell->seq == POWER_SKIP) {
		pr_notice("%s: Power off skip\n", __func__);
		return rc;
	}

	if (!panel) {
		pr_err("%s: Invalid input panel\n", __func__);
		return -EINVAL;
	}
	spec_pdata = panel->spec_pdata;

	dsi_panel_driver_en_dcdc(panel, 0);

	if (spec_pdata->rst_b_seq) {
		rc = dsi_panel_driver_reset_panel(panel, 0);
		if (rc)
			pr_warn("%s: Panel reset failed. rc=%d\n",
					__func__, rc);
	}

	if (gpio_is_valid(spec_pdata->reset_touch_gpio)) {
		gpio_set_value(spec_pdata->reset_touch_gpio, 0);
		usleep_range(spec_pdata->touch_reset_off * 1000,
					spec_pdata->touch_reset_off * 1000 + 100);
	}

	return rc;
}

int dsi_panel_driver_post_power_off(struct dsi_panel *panel)
{
	struct panel_specific_pdata *spec_pdata = NULL;
	struct incell_ctrl *incell = incell_get_info();
	int rc = 0;
	struct drm_ext_event event;
	int blank = DRM_BLANK_POWERDOWN;
	event.data = &blank;

	if (incell->seq == POWER_SKIP) {
		pr_notice("%s: Post power off skip\n", __func__);
		drm_notifier_call_chain(DRM_EXT_EVENT_AFTER_BLANK, &event);
		return rc;
	}

	if (!panel) {
		pr_err("%s: Invalid input panel\n", __func__);
		return -EINVAL;
	}
	spec_pdata = panel->spec_pdata;

	rc = dsi_panel_driver_vreg_ctrl(&spec_pdata->vspvsn_power_info, "ibb", false);
	if (rc)
		pr_err("%s: failed to disable vsn, rc=%d\n", __func__, rc);

	rc = dsi_panel_driver_vreg_ctrl(&spec_pdata->vspvsn_power_info, "lab", false);
	if (rc)
		pr_err("%s: failed to disable vsp, rc=%d\n", __func__, rc);

	if (!spec_pdata->rst_b_seq) {
		rc = dsi_panel_driver_reset_panel(panel, 0);
		if (rc)
			pr_warn("%s: Panel reset failed. rc=%d\n", __func__, rc);
	}

	dsi_panel_driver_touch_pinctrl_set_state(panel, false);

	rc = dsi_panel_driver_touch_power(panel, false);
	if (rc)
		pr_err("%s: failed to disable touch vddion en, rc=%d\n", __func__, rc);

	rc = dsi_panel_driver_vreg_ctrl(&spec_pdata->touch_power_info, "touch-avdd", false);
	if (rc)
		pr_err("%s: failed to disable touch-avdd, rc=%d\n", __func__, rc);

	rc = dsi_panel_driver_vreg_ctrl(&panel->power_info, "vddio", false);
	if (rc)
		pr_err("%s: failed to disable vddio, rc=%d\n", __func__, rc);

	if (rc)
		pr_err("%s: failed to power off\n", __func__);
	else
		pr_notice("@@@@ panel power off @@@@\n");

	rc = dsi_panel_set_pinctrl_state(panel, false);
	if (rc)
		pr_err("%s: failed set pinctrl state, rc=%d\n", __func__, rc);

	incell->state &= INCELL_POWER_STATE_OFF;
	drm_notifier_call_chain(DRM_EXT_EVENT_AFTER_BLANK, &event);

	if (spec_pdata->down_period)
		down_period = (u32)ktime_to_ms(ktime_get());

	return rc;
}

int dsi_panel_driver_pre_power_on(struct dsi_panel *panel)
{
	struct panel_specific_pdata *spec_pdata = NULL;
	struct incell_ctrl *incell = incell_get_info();
	int rc = 0;
	struct drm_ext_event event;
	int blank = DRM_BLANK_UNBLANK;
	event.data = &blank;

	dsi_panel_driver_power_on_ctrl();
	if (incell->seq == POWER_SKIP) {
		pr_notice("%s: Pre power on skip\n", __func__);
		drm_notifier_call_chain(DRM_EXT_EVENT_BEFORE_BLANK, &event);
		return rc;
	}

	if (!panel) {
		pr_err("%s: Invalid input panel\n", __func__);
		return -EINVAL;
	}
	spec_pdata = panel->spec_pdata;

	if (spec_pdata->down_period) {
		u32 kt = (u32)ktime_to_ms(ktime_get());
		kt = (kt < down_period) ? kt + ~down_period : kt - down_period;
		if (kt < spec_pdata->down_period) {
			u32 down_time = spec_pdata->down_period - kt;
			usleep_range(down_time * 1000, down_time * 1000 + 100);
		}
	}

	drm_notifier_call_chain(DRM_EXT_EVENT_BEFORE_BLANK, &event);

	rc = dsi_panel_driver_vreg_ctrl(&panel->power_info, "vddio", true);
	if (rc) {
		pr_err("%s: failed to enable vddio, rc=%d\n", __func__, rc);
		goto exit;
	}

	rc = dsi_panel_driver_vreg_ctrl(&spec_pdata->touch_power_info, "touch-avdd", true);
	if (rc) {
		pr_err("%s: failed to enable touch-avdd, rc=%d\n", __func__, rc);
		goto exit;
	}

	rc = dsi_panel_driver_touch_power(panel, true);
	if (rc) {
		pr_err("%s: failed to enable touch vddio, rc=%d\n", __func__, rc);
		goto exit;
	}

	dsi_panel_driver_touch_pinctrl_set_state(panel, true);

exit:
	return rc;
}

int dsi_panel_driver_power_on(struct dsi_panel *panel)
{
	struct panel_specific_pdata *spec_pdata = NULL;
	struct incell_ctrl *incell = incell_get_info();
	int rc = 0;

	if (incell->seq == POWER_SKIP) {
		pr_notice("%s: Power on skip\n", __func__);
		return rc;
	}

	if (!panel) {
		pr_err("%s: Invalid input panel\n", __func__);
		return -EINVAL;
	}
	spec_pdata = panel->spec_pdata;

	if(!spec_pdata->rst_after_pon) {
		rc = dsi_panel_driver_touch_reset(panel);
		if (rc) {
			pr_err("%s: failed to reset touch panel, rc=%d\n", __func__, rc);
			goto error_disable_vregs;
		}

		rc = dsi_panel_set_pinctrl_state(panel, true);
		if (rc) {
			pr_err("%s: failed to set pinctrl, rc=%d\n", __func__, rc);
			goto error_disable_vregs;
		}

		rc = dsi_panel_reset(panel);
		if (rc) {
			pr_err("%s: failed to reset panel, rc=%d\n", __func__, rc);
			goto error_disable_gpio;
		}
	}

	rc = dsi_panel_driver_vreg_ctrl(&spec_pdata->vspvsn_power_info, "lab", true);
	if (rc) {
		pr_err("%s: failed to enable vsp, rc=%d\n", __func__, rc);
		goto exit;
	}

	rc = dsi_panel_driver_vreg_ctrl(&spec_pdata->vspvsn_power_info, "ibb", true);
	if (rc) {
		pr_err("%s: failed to enable vsn, rc=%d\n", __func__, rc);
		goto exit;
	}

	pr_notice("@@@@ panel power on @@@@\n");

	if(spec_pdata->rst_after_pon) {
		rc = dsi_panel_driver_touch_reset(panel);
		if (rc) {
			pr_err("%s: failed to reset touch panel, rc=%d\n", __func__, rc);
			goto error_disable_vregs;
		}

		rc = dsi_panel_set_pinctrl_state(panel, true);
		if (rc) {
			pr_err("%s: failed to set pinctrl, rc=%d\n", __func__, rc);
			goto error_disable_vregs;
		}

		rc = dsi_panel_reset(panel);
		if (rc) {
			pr_err("%s: failed to reset panel, rc=%d\n", __func__, rc);
			goto error_disable_gpio;
		}
	}

	dsi_panel_driver_en_dcdc(panel, 1);
	incell->state |= INCELL_POWER_STATE_ON;

	goto exit;

error_disable_gpio:
	if (gpio_is_valid(panel->reset_config.disp_en_gpio))
		gpio_set_value(panel->reset_config.disp_en_gpio, 0);

	if (gpio_is_valid(panel->bl_config.en_gpio))
		gpio_set_value(panel->bl_config.en_gpio, 0);

	(void)dsi_panel_set_pinctrl_state(panel, false);

error_disable_vregs:
	(void)dsi_pwr_enable_regulator(&panel->power_info, false);
	(void)dsi_pwr_enable_regulator(&spec_pdata->touch_power_info, false);
	(void)dsi_pwr_enable_regulator(&spec_pdata->vspvsn_power_info, false);

	pr_err("%s: failed to power on\n", __func__);
exit:
	return rc;
}

void dsi_panel_driver_labibb_vreg_init(struct dsi_panel *panel)
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
	ret = dsi_panel_driver_vreg_name_to_config(
			&spec_pdata->vspvsn_power_info, &lab_vreg, "lab");
	if (ret) {
		pr_err("%s: lab not registered\n", __func__);
		return;
	}

	ret = dsi_panel_driver_vreg_name_to_config(
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

static int dsi_panel_driver_parse_reset_touch_sequence(struct dsi_panel *panel,
					struct device_node *of_node)
{
	int rc = 0;
	int i;
	u32 length = 0;
	u32 count = 0;
	u32 size = 0;
	u32 *arr_32 = NULL;
	const u32 *arr;
	struct dsi_reset_seq *seq;
	struct panel_specific_pdata *spec_pdata = NULL;

	if (!panel) {
		pr_err("%s: Invalid input panel\n", __func__);
		return -EINVAL;
	}
	spec_pdata = panel->spec_pdata;

	arr = of_get_property(of_node,
			      "qcom,mdss-dsi-touch-reset-sequence",
			      &length);
	if (!arr) {
		pr_err("%s: dsi-reset-touch-sequence not found\n", __func__);
		rc = -EINVAL;
		goto error;
	}
	if (length & 0x1) {
		pr_err("%s: syntax error for dsi-touch-reset-sequence\n",
		       __func__);
		rc = -EINVAL;
		goto error;
	}

	pr_debug("RESET SEQ LENGTH = %d\n", length);
	length = length / sizeof(u32);

	size = length * sizeof(u32);

	arr_32 = kzalloc(size, GFP_KERNEL);
	if (!arr_32) {
		rc = -ENOMEM;
		goto error;
	}

	rc = of_property_read_u32_array(of_node,
					"qcom,mdss-dsi-touch-reset-sequence",
					arr_32, length);
	if (rc) {
		pr_err("%s: cannot read dsi-touch-reset-seqience\n", __func__);
		goto error_free_arr_32;
	}

	count = length / 2;
	size = count * sizeof(*seq);
	seq = kzalloc(size, GFP_KERNEL);
	if (!seq) {
		rc = -ENOMEM;
		goto error_free_arr_32;
	}

	spec_pdata->sequence_touch = seq;
	spec_pdata->count_touch = count;

	for (i = 0; i < length; i += 2) {
		seq->level = arr_32[i];
		seq->sleep_ms = arr_32[i + 1];
		seq++;
	}

error_free_arr_32:
	kfree(arr_32);
error:
	return rc;
}

static int dsi_panel_get_cmd_pkt_count(const char *data, u32 length, u32 *cnt)
{
	const u32 cmd_set_min_size = 7;
	u32 count = 0;
	u32 packet_length;
	u32 tmp;

	while (length >= cmd_set_min_size) {
		packet_length = cmd_set_min_size;
		tmp = ((data[5] << 8) | (data[6]));
		packet_length += tmp;
		if (packet_length > length) {
			pr_err("format error\n");
			return -EINVAL;
		}
		length -= packet_length;
		data += packet_length;
		count++;
	};

	*cnt = count;
	return 0;
}

static int dsi_panel_create_cmd_packets(const char *data,
					u32 count,
					struct cmd_data *cmd)
{
	int rc = 0;
	int i, j;
	char *payload;
	struct dsi_cmd_desc *send_cmd = cmd->send_cmd;

	for (i = 0; i < count; i++) {
		u32 size;

		send_cmd[i].msg.type = data[0];
		send_cmd[i].last_command = (data[1] == 1 ? true : false);
		send_cmd[i].msg.channel = data[2];
		send_cmd[i].msg.flags |= (data[3] == 1 ? MIPI_DSI_MSG_REQ_ACK : 0);
		send_cmd[i].msg.ctrl = 0;
		send_cmd[i].post_wait_ms = data[4];
		send_cmd[i].msg.tx_len = ((data[5] << 8) | (data[6]));

		size = send_cmd[i].msg.tx_len * sizeof(u8);

		payload = kzalloc(size, GFP_KERNEL);
		if (!payload) {
			rc = -ENOMEM;
			goto error_free_payloads;
		}

		for (j = 0; j < send_cmd[i].msg.tx_len; j++) {
			payload[j] = data[7 + j];
			pr_debug("%s (%d): data[7 + j] = %x \n",
				__func__, __LINE__, data[7 + j]);
		}

		cmd->payload[i] = payload;
		data += (7 + send_cmd[i].msg.tx_len);
	}

	return rc;
error_free_payloads:
	for (i = i - 1; i >= 0; i--)
		kfree(cmd->payload[i]);

	return rc;
}

static int dsi_parse_dcs_cmds(struct device_node *np,
		struct dsi_panel_cmds *pcmds, char *cmd_key, char *link_key)
{
	int rc = 0;
	u32 p_size = 0;
	const char *state;

	u32 length = 0;
	const char *data;
	u32 packet_count = 0;

	data = of_get_property(np, cmd_key, &length);
	if (!data) {
		pr_debug("%s :failed to get , key=%s\n", __func__, cmd_key);
		rc = -ENOTSUPP;
		goto error;
	}

	rc = dsi_panel_get_cmd_pkt_count(data, length, &packet_count);
	if (rc) {
		pr_err("commands failed, rc=%d\n", rc);
		goto error;
	}
	pr_debug("packet-count=%d, length=%d\n", packet_count, length);

	p_size = packet_count * sizeof(*pcmds->cmds.send_cmd);
	pcmds->cmds.send_cmd = kzalloc(p_size, GFP_KERNEL);
	if (!pcmds->cmds.send_cmd) {
		pr_err("failed to allocate cmd packets, rc=%d\n", rc);
		goto error_free_mem;
	}
	p_size = packet_count * sizeof(*pcmds->cmds.payload);
	pcmds->cmds.payload = kzalloc(p_size, GFP_KERNEL);
	if (!pcmds->cmds.payload) {
		pr_err("failed to allocate payload, rc=%d\n", rc);
		goto error_payload_free_mem;
	}

	rc = dsi_panel_create_cmd_packets(data, packet_count,
					  &pcmds->cmds);
	if (rc) {
		pr_err("failed to create cmd packets, rc=%d\n", rc);
		goto error_free_mem;
	}

	if (link_key) {
		state = of_get_property(np, link_key, NULL);
		if (!state || !strcmp(state, "dsi_lp_mode")) {
			pcmds->link_state = DSI_CMD_SET_STATE_LP;
		} else {
			pcmds->link_state = DSI_CMD_SET_STATE_HS;
		}
	}

	pcmds->cmd_cnt = packet_count;

	return rc;
error_free_mem:
	kfree(pcmds->cmds.send_cmd);
	pcmds->cmds.send_cmd = NULL;
error_payload_free_mem:
	kfree(pcmds->cmds.payload);
	pcmds->cmds.send_cmd = NULL;
error:
	return rc;

}

static int dsi_property_read_u32_var(struct device_node *np,
		char *name, struct dsi_reset_cfg *out)
{
	int rc = 0;
	int i;
	u32 length = 0;
	u32 count = 0;
	u32 size = 0;
	u32 *arr_32 = NULL;
	const u32 *arr;
	struct dsi_reset_seq *seq;
	struct property *prop = of_find_property(np, name, NULL);

	if (!prop) {
		pr_debug("%s:%d, unable to read %s", __func__, __LINE__, name);
		return -EINVAL;
	}

	arr = of_get_property(np, name, &length);
	if (length & 0x1) {
		pr_err("%s: syntax error for %s\n", __func__, name);
		rc = -EINVAL;
		goto error;
	}

	pr_debug("RESET SEQ LENGTH = %d\n", length);
	length = length / sizeof(u32);
	size = length * sizeof(u32);

	arr_32 = kzalloc(size, GFP_KERNEL);
	if (!arr_32) {
		rc = -ENOMEM;
		goto error;
	}

	rc = of_property_read_u32_array(np, name, arr_32, length);
	if (rc) {
		pr_err("%s: cannot read %s\n", __func__, name);
		goto error_free_arr_32;
	}

	count = length / 2;
	size = count * sizeof(*seq);
	seq = kzalloc(size, GFP_KERNEL);
	if (!seq) {
		rc = -ENOMEM;
		goto error_free_arr_32;
	}

	out->seq = seq;
	out->count = count;

	for (i = 0; i < length; i += 2) {
		seq->level = arr_32[i];
		seq->sleep_ms = arr_32[i + 1];
		seq++;
	}

error_free_arr_32:
	kfree(arr_32);
error:
	return rc;
}

int dsi_panel_driver_parse_dt_pcc(struct dsi_panel *panel, struct device_node *np)
{
	struct panel_specific_pdata *spec_pdata = NULL;
	u32 tmp = 0;
	int rc = 0;

	if (!panel) {
		pr_err("%s: Invalid input panel\n", __func__);
		return -EINVAL;
	}

	spec_pdata = panel->spec_pdata;
	spec_pdata->standard_pcc_enable = of_property_read_bool(np, "somc,mdss-dsi-pcc-enable");

	if (spec_pdata->standard_pcc_enable) {
		dsi_parse_dcs_cmds(np, &spec_pdata->uv_read_cmds,
			"somc,mdss-dsi-uv-command", NULL);

		rc = of_property_read_u32(np,
			"somc,mdss-dsi-uv-param-type", &tmp);
		spec_pdata->standard_pcc_data.param_type =
			(!rc ? tmp : CLR_DATA_UV_PARAM_TYPE_NONE);

		rc = of_property_read_u32(np,
			"somc,mdss-dsi-pcc-table-size", &tmp);
		spec_pdata->standard_pcc_data.tbl_size =
			(!rc ? tmp : 0);

		spec_pdata->standard_pcc_data.color_tbl =
			kzalloc(spec_pdata->standard_pcc_data.tbl_size *
				sizeof(struct dsi_pcc_color_tbl),
				GFP_KERNEL);
		if (!spec_pdata->standard_pcc_data.color_tbl) {
			pr_err("no mem assigned: kzalloc fail\n");
			return -ENOMEM;
		}
		rc = of_property_read_u32_array(np,
			"somc,mdss-dsi-pcc-table",
			(u32 *)spec_pdata->standard_pcc_data.color_tbl,
			spec_pdata->standard_pcc_data.tbl_size *
			sizeof(struct dsi_pcc_color_tbl) /
			sizeof(u32));
		if (rc) {
			spec_pdata->standard_pcc_data.tbl_size = 0;
			kzfree(spec_pdata->standard_pcc_data.color_tbl);
			spec_pdata->standard_pcc_data.color_tbl = NULL;
			pr_err("%s:%d, Unable to read pcc table",
				__func__, __LINE__);
		}
		spec_pdata->standard_pcc_data.pcc_sts |= PCC_STS_UD;
	} else {
		pr_err("%s:%d, Unable to read pcc table\n",
			__func__, __LINE__);
	}

	spec_pdata->srgb_pcc_enable = of_property_read_bool(np,
			"somc,mdss-dsi-srgb-pcc-enable");
	if (spec_pdata->srgb_pcc_enable) {
		rc = of_property_read_u32(np,
			"somc,mdss-dsi-srgb-pcc-table-size", &tmp);
		spec_pdata->srgb_pcc_data.tbl_size =
			(!rc ? tmp : 0);

		spec_pdata->srgb_pcc_data.color_tbl =
			kzalloc(spec_pdata->srgb_pcc_data.tbl_size *
				sizeof(struct dsi_pcc_color_tbl),
				GFP_KERNEL);
		if (!spec_pdata->srgb_pcc_data.color_tbl) {
			pr_err("no mem assigned: kzalloc fail\n");
			return -ENOMEM;
		}
		rc = of_property_read_u32_array(np,
			"somc,mdss-dsi-srgb-pcc-table",
			(u32 *)spec_pdata->srgb_pcc_data.color_tbl,
			spec_pdata->srgb_pcc_data.tbl_size *
			sizeof(struct dsi_pcc_color_tbl) /
			sizeof(u32));
		if (rc) {
			spec_pdata->srgb_pcc_data.tbl_size = 0;
			kzfree(spec_pdata->srgb_pcc_data.color_tbl);
			spec_pdata->srgb_pcc_data.color_tbl = NULL;
			pr_err("%s:%d, Unable to read sRGB pcc table",
				__func__, __LINE__);
		}
	} else {
		pr_err("%s:%d, Unable to read srgb_pcc table",
			__func__, __LINE__);
	}

	spec_pdata->vivid_pcc_enable = of_property_read_bool(np,
			"somc,mdss-dsi-vivid-pcc-enable");
	if (spec_pdata->vivid_pcc_enable) {
		rc = of_property_read_u32(np,
			"somc,mdss-dsi-vivid-pcc-table-size", &tmp);
		spec_pdata->vivid_pcc_data.tbl_size =
			(!rc ? tmp : 0);

		spec_pdata->vivid_pcc_data.color_tbl =
			kzalloc(spec_pdata->vivid_pcc_data.tbl_size *
				sizeof(struct dsi_pcc_color_tbl),
				GFP_KERNEL);
		if (!spec_pdata->vivid_pcc_data.color_tbl) {
			pr_err("no mem assigned: kzalloc fail\n");
			return -ENOMEM;
		}
		rc = of_property_read_u32_array(np,
			"somc,mdss-dsi-vivid-pcc-table",
			(u32 *)spec_pdata->vivid_pcc_data.color_tbl,
			spec_pdata->vivid_pcc_data.tbl_size *
			sizeof(struct dsi_pcc_color_tbl) /
			sizeof(u32));
		if (rc) {
			spec_pdata->vivid_pcc_data.tbl_size = 0;
			kzfree(spec_pdata->vivid_pcc_data.color_tbl);
			spec_pdata->vivid_pcc_data.color_tbl = NULL;
			pr_err("%s:%d, Unable to read Vivid pcc table",
				__func__, __LINE__);
		}
	} else {
		pr_err("%s:%d, Unable to read vivid_pcc table",
			__func__, __LINE__);
	}
	spec_pdata->hdr_pcc_enable = of_property_read_bool(np,
			"somc,mdss-dsi-hdr-pcc-enable");
	if (spec_pdata->hdr_pcc_enable) {
		rc = of_property_read_u32(np,
			"somc,mdss-dsi-hdr-pcc-table-size", &tmp);
		spec_pdata->hdr_pcc_data.tbl_size =
			(!rc ? tmp : 0);

		spec_pdata->hdr_pcc_data.color_tbl =
			kzalloc(spec_pdata->hdr_pcc_data.tbl_size *
				sizeof(struct dsi_pcc_color_tbl),
				GFP_KERNEL);
		if (!spec_pdata->hdr_pcc_data.color_tbl) {
			pr_err("no mem assigned: kzalloc fail\n");
			return -ENOMEM;
		}
		rc = of_property_read_u32_array(np,
			"somc,mdss-dsi-hdr-pcc-table",
			(u32 *)spec_pdata->hdr_pcc_data.color_tbl,
			spec_pdata->hdr_pcc_data.tbl_size *
			sizeof(struct dsi_pcc_color_tbl) /
			sizeof(u32));
		if (rc) {
			spec_pdata->hdr_pcc_data.tbl_size = 0;
			kzfree(spec_pdata->hdr_pcc_data.color_tbl);
			spec_pdata->hdr_pcc_data.color_tbl = NULL;
			pr_err("%s:%d, Unable to read HDR pcc table",
				__func__, __LINE__);
		}
	} else {
		pr_err("%s:%d, Unable to read hdr_pcc table",
			__func__, __LINE__);
	}
	return rc;
}

int dsi_panel_driver_parse_dt_fps(struct dsi_panel *panel,
				  struct device_node *np)
{
	struct panel_specific_pdata *spec_pdata = NULL;
	const char *panel_mode;
	int rc = 0;

	spec_pdata = panel->spec_pdata;

	spec_pdata->fps_mode.enable = of_property_read_bool(np,
					"somc,fps-mode-enable");
	if (spec_pdata->fps_mode.enable) {
		panel_mode = of_get_property(np,
					"somc,fps-mode-panel-mode", NULL);

		if (!panel_mode) {
			pr_err("%s:%d, Panel mode not specified\n",
							__func__, __LINE__);
			return -EINVAL;
		}

		if (!strncmp(panel_mode, "susres_mode", 11)) {
			spec_pdata->fps_mode.mode = FPS_MODE_SUSRES;
		} else if (!strncmp(panel_mode, "dynamic_mode", 12)) {
			spec_pdata->fps_mode.mode = FPS_MODE_DYNAMIC;
		} else {
			pr_err("%s: Unable to read fps panel mode\n", __func__);
			return -EINVAL;
		}

		spec_pdata->fps_mode.type = FPS_MODE_OFF_RR_OFF;
	}
	return rc;
}

void dsi_panel_driver_parse_dt_mplus(struct dsi_panel *panel,
				  struct device_node *np)
{
	struct panel_specific_pdata *spec_pdata = NULL;

	spec_pdata = panel->spec_pdata;

	spec_pdata->m_plus.enable = of_property_read_bool(np,
			"somc,mdss-dsi-mplus-enable");
	if (spec_pdata->m_plus.enable) {
		spec_pdata->m_plus.mode = M_PLUS_MODE_INIT;
	} else {
		spec_pdata->m_plus.mode = M_PLUS_MODE_INVALID;
	}
}

int dsi_panel_driver_parse_dt_labibb(struct dsi_panel *panel,
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

int dsi_panel_driver_parse_dt(struct dsi_panel *panel,
					struct device_node *np)
{
	struct panel_specific_pdata *spec_pdata = NULL;
	struct dsi_regulator_info *power_info = NULL;
	struct dsi_regulator_info *touch_power_info = NULL;
	struct dsi_regulator_info *vspvsn_power_info = NULL;
	u32 tmp = 0;
	int i = 0;
	int rc = 0;
	int valid = -EINVAL;
	const char *rst_seq;

	if (!panel) {
		pr_err("%s: Invalid input panel\n", __func__);
		return -EINVAL;
	}
	spec_pdata = panel->spec_pdata;

	rc = dsi_panel_driver_parse_dt_pcc(panel, np);

	if (of_find_property(np, "somc,pw-off-rst-b-seq", NULL)) {
		spec_pdata->rst_b_seq = true;

		(void)dsi_property_read_u32_var(np,
			"somc,pw-off-rst-b-seq",
			&spec_pdata->off_seq);
	} else {
		(void)dsi_property_read_u32_var(np,
			"somc,pw-off-rst-seq",
			&spec_pdata->off_seq);
	}

	spec_pdata->on_seq.seq = panel->reset_config.sequence;
	spec_pdata->on_seq.count = panel->reset_config.count;

	rst_seq = of_get_property(np, "somc,pw-on-rst-seq", NULL);
	if (!rst_seq) {
		pr_err("%s:failed parse pw-on-rst-seq, rst is set after pon \n", __func__);
		spec_pdata->rst_after_pon = true;
	} else if (!strcmp(rst_seq, "after_power_on")) {
		spec_pdata->rst_after_pon = true;
	} else if (!strcmp(rst_seq, "before_power_on")) {
		spec_pdata->rst_after_pon = false;
	} else {
		pr_err("%s:failed parse pw-on-rst-seq, rst is set after pon \n", __func__);
		spec_pdata->rst_after_pon = true;
	}

	power_info = &panel->power_info;
	vspvsn_power_info = &spec_pdata->vspvsn_power_info;
	touch_power_info = &spec_pdata->touch_power_info;

	for (i = 0; i < power_info->count; i++) {
		if (!strcmp("vddio", power_info->vregs[i].vreg_name)) {
			valid = 0;
			break;
		}
	}
	if (!valid) {
		rc = of_property_read_u32(np,
				"somc,pw-wait-after-on-vddio", &tmp);
		power_info->vregs[i].post_on_sleep = !rc ? tmp : 0;

		rc = of_property_read_u32(np,
				"somc,pw-wait-after-off-vddio", &tmp);
		power_info->vregs[i].post_off_sleep = !rc ? tmp : 0;
	} else {
		pr_notice("%s: vddio power info not registered\n", __func__);
	}

	valid = -EINVAL;
	for (i = 0; i < vspvsn_power_info->count; i++) {
		if (!strcmp("lab", vspvsn_power_info->vregs[i].vreg_name)) {
			valid = 0;
			break;
		}
	}
	if (!valid) {
		rc = of_property_read_u32(np,
				"somc,pw-wait-after-on-vsp", &tmp);
		vspvsn_power_info->vregs[i].post_on_sleep = !rc ? tmp : 0;

		rc = of_property_read_u32(np,
				"somc,pw-wait-after-off-vsp", &tmp);
		vspvsn_power_info->vregs[i].post_off_sleep = !rc ? tmp : 0;
	} else {
		pr_notice("%s: vsp power info not registered\n", __func__);
	}

	valid = -EINVAL;
	for (i = 0; i < vspvsn_power_info->count; i++) {
		if (!strcmp("ibb", vspvsn_power_info->vregs[i].vreg_name)) {
			valid = 0;
			break;
		}
	}
	if (!valid) {
		rc = of_property_read_u32(np,
				"somc,pw-wait-after-on-vsn", &tmp);
		vspvsn_power_info->vregs[i].post_on_sleep = !rc ? tmp : 0;

		rc = of_property_read_u32(np,
				"somc,pw-wait-after-off-vsn", &tmp);
		vspvsn_power_info->vregs[i].post_off_sleep = !rc ? tmp : 0;
	} else {
		pr_notice("%s: vsn power info not registered\n", __func__);
	}

	valid = -EINVAL;
	for (i = 0; i < touch_power_info->count; i++) {
		if (!strcmp("touch-avdd",
			touch_power_info->vregs[i].vreg_name)) {
			valid = 0;
			break;
		}
	}
	if (!valid) {
		rc = of_property_read_u32(np,
				"somc,pw-wait-after-on-touch-avdd", &tmp);
		touch_power_info->vregs[i].post_on_sleep = !rc ? tmp : 0;

		rc = of_property_read_u32(np,
				"somc,pw-wait-after-off-touch-avdd", &tmp);
		touch_power_info->vregs[i].post_off_sleep = !rc ? tmp : 0;
	} else {
		pr_notice("%s: touch avdd power info not registered\n",
								__func__);
	}

	rc = of_property_read_u32(np,
			"somc,pw-down-period", &tmp);
	spec_pdata->down_period = !rc ? tmp : 0;

	rc = of_property_read_u32(np,
			"somc,pw-wait-after-on-touch-vddio", &tmp);
	spec_pdata->touch_vddio = !rc ? tmp : 0;

	rc = of_property_read_u32(np,
			"somc,pw-wait-after-off-touch-vddio", &tmp);
	spec_pdata->touch_vddio_off = !rc ? tmp : 0;

	rc = of_property_read_u32(np,
			"somc,pw-wait-after-off-touch-reset", &tmp);
	spec_pdata->touch_reset_off = !rc ? tmp : 0;

	rc = of_property_read_u32(np,
			"somc,pw-wait-after-on-touch-int-n", &tmp);
	spec_pdata->touch_intn = !rc ? tmp : 0;

	rc = of_property_read_u32(np,
			"somc,pw-wait-after-off-touch-int-n", &tmp);
	spec_pdata->touch_intn_off = !rc ? tmp : 0;

	panel->lp11_init = of_property_read_bool(np,
			"qcom,mdss-dsi-lp11-init");

	rc = dsi_panel_driver_parse_dt_fps(panel, np);
	if (rc) {
		pr_err("failed to parse Low Fps, rc=%d\n", rc);
	}

	dsi_panel_driver_parse_dt_mplus(panel, np);

	rc = dsi_panel_driver_parse_dt_labibb(panel, np);

	return 0;
}

int dsi_panel_driver_parse_power_cfg(struct device *parent,
					struct dsi_panel *panel,
					struct device_node *of_node)
{
	int rc = 0;
	struct panel_specific_pdata *spec_pdata = NULL;

	if (!panel) {
		pr_err("%s: Invalid input panel\n", __func__);
		return -EINVAL;
	}
	spec_pdata = panel->spec_pdata;

	rc = dsi_pwr_of_get_vreg_data(of_node,
					  &spec_pdata->touch_power_info,
					  "qcom,panel-touch-supply-entries");
	if (rc)
		pr_notice("%s: Not configured touch vregs\n", __func__);

	rc = dsi_pwr_of_get_vreg_data(of_node,
					  &spec_pdata->vspvsn_power_info,
					  "qcom,panel-vspvsn-supply-entries");
	if (rc)
		pr_err("%s: Not configured vsp/vsn vregs\n", __func__);

	return rc;
}

int dsi_panel_driver_parse_gpios(struct dsi_panel *panel,
					struct device_node *of_node)
{
	int rc = 0;
	struct panel_specific_pdata *spec_pdata = NULL;

	if (!panel) {
		pr_err("%s: Invalid input panel\n", __func__);
		return -EINVAL;
	}
	spec_pdata = panel->spec_pdata;

	spec_pdata->touch_vddio_en_gpio = of_get_named_gpio(of_node,
					      "qcom,platform-touch-vddio-en-gpio",
					      0);
	if (!gpio_is_valid(spec_pdata->touch_vddio_en_gpio)) {
		pr_err("%s: failed get touch-vddio-en gpio\n", __func__);
	}

	spec_pdata->reset_touch_gpio = of_get_named_gpio(of_node,
					      "qcom,platform-touch-reset-gpio",
					      0);
	if (!gpio_is_valid(spec_pdata->reset_touch_gpio)) {
		pr_err("%s: failed get reset touch gpio\n", __func__);
	}

	rc = dsi_panel_driver_parse_reset_touch_sequence(panel, of_node);
	if (rc) {
		pr_err("%s: failed to parse reset touch sequence, rc=%d\n",
		       __func__, rc);
		goto error;
	}

	spec_pdata->disp_dcdc_en_gpio = of_get_named_gpio(of_node,
						"somc,disp-dcdc-en-gpio",
						0);
	if (!gpio_is_valid(spec_pdata->disp_dcdc_en_gpio)) {
		pr_err("%s: disp dcdc en gpio not specified\n", __func__);
	}

error:
	return rc;
}

void dsi_panel_driver_en_dcdc(struct dsi_panel *panel, int en)
{
	struct panel_specific_pdata *spec_pdata = NULL;

	if (!panel) {
		pr_err("%s: Invalid input panel\n", __func__);
		return;
	}
	spec_pdata = panel->spec_pdata;

	if (gpio_is_valid(spec_pdata->disp_dcdc_en_gpio)) {
		gpio_direction_output(spec_pdata->disp_dcdc_en_gpio, en);
		usleep_range(5000, 5000 + 100);
	}
}

void dsi_panel_fps_mode_set(struct dsi_panel *panel, int mode_type)
{
	struct panel_specific_pdata *spec_pdata;

	spec_pdata = panel->spec_pdata;

	switch (mode_type) {
	case FPS_MODE_OFF_RR_OFF:
		dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_FPS_MODE_OFF_RR_OFF);
		spec_pdata->fps_mode.type =  FPS_MODE_OFF_RR_OFF;
		break;
	case FPS_MODE_ON_RR_OFF:
		dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_FPS_MODE_ON_RR_OFF);
		spec_pdata->fps_mode.type = FPS_MODE_ON_RR_OFF;
		break;
	case FPS_MODE_OFF_RR_ON:
		dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_FPS_MODE_OFF_RR_ON);
		spec_pdata->fps_mode.type = FPS_MODE_OFF_RR_ON;
		break;
	case FPS_MODE_ON_RR_ON:
		dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_FPS_MODE_ON_RR_ON);
		spec_pdata->fps_mode.type = FPS_MODE_ON_RR_ON;
		break;
	default:
		pr_err("%s: invalid value for fps mode type = %d\n",
			__func__, mode_type);
		break;
	}
}

static void dsi_panel_driver_fps_mode_cmds_send(struct dsi_panel *panel)
{
	struct panel_specific_pdata *spec_pdata = NULL;

	spec_pdata = panel->spec_pdata;

	if (spec_pdata->fps_mode.enable)
		dsi_panel_fps_mode_set(panel, spec_pdata->fps_mode.type);
	else
		pr_debug("%s: change fps is not supported.\n", __func__);
}

static int dsi_panel_driver_fps_mode_check_state
	(struct dsi_display *display, int mode_type)
{
	struct panel_specific_pdata *spec_pdata;

	spec_pdata = display->panel->spec_pdata;

	if (!spec_pdata->display_onoff_state)
		goto disp_onoff_state_err;

	spec_pdata->fps_mode.type = mode_type;

	if (spec_pdata->fps_mode.mode == FPS_MODE_DYNAMIC)
		dsi_panel_fps_mode_set(display->panel,
			spec_pdata->fps_mode.type);

	return 0;

disp_onoff_state_err:
	pr_err("%s: Disp-On is not yet completed. Please retry\n", __func__);
	return -EINVAL;
}

int dsi_panel_mplus_mode_set(struct dsi_panel *panel, int m_mode)
{
	int rc = 0;
	struct panel_specific_pdata *spec_pdata;

	spec_pdata = panel->spec_pdata;

	switch (m_mode) {
	case M_PLUS_MODE_1:
		dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_M_PLUS_PEAK1000);
		spec_pdata->m_plus.mode = M_PLUS_MODE_1;
		break;
	case M_PLUS_MODE_2:
		dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_M_PLUS_PEAK700);
		spec_pdata->m_plus.mode = M_PLUS_MODE_2;
		break;
	case M_PLUS_MODE_3:
		dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_M_PLUS_PEAK600);
		spec_pdata->m_plus.mode = M_PLUS_MODE_3;
		break;
	case M_PLUS_MODE_4:
		dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_M_PLUS_OFF);
		spec_pdata->m_plus.mode = M_PLUS_MODE_4;
		break;
	default:
		pr_err("%s: invalid value for m　plus mode = %d\n",
			__func__, m_mode);
		rc = -EINVAL;
		break;
	}

	return rc;
}

static void dsi_panel_driver_notify_resume(struct dsi_panel *panel)
{
	struct drm_ext_event event;
	int blank = DRM_BLANK_UNBLANK;
	event.data = &blank;

	drm_notifier_call_chain(DRM_EXT_EVENT_AFTER_BLANK, &event);
}

static void dsi_panel_driver_notify_suspend(struct dsi_panel *panel)
{
	struct drm_ext_event event;
	int blank = DRM_BLANK_POWERDOWN;
	event.data = &blank;

	drm_notifier_call_chain(DRM_EXT_EVENT_BEFORE_BLANK, &event);

 }

void dsi_panel_driver_post_enable(struct dsi_panel *panel)
{
	panel->spec_pdata->display_onoff_state = true;

	dsi_panel_driver_fps_mode_cmds_send(panel);
	dsi_panel_driver_notify_resume(panel);
}

void dsi_panel_driver_pre_disable(struct dsi_panel  *panel)
{
	dsi_panel_driver_notify_suspend(panel);
}

void dsi_panel_driver_disable(struct dsi_panel  *panel)
{
	panel->spec_pdata->display_onoff_state = false;
}

static ssize_t dsi_panel_driver_id_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct dsi_display *display = dev_get_drvdata(dev);
	char const *id = display->panel->name ?
	display->panel->name : "default";

	return scnprintf(buf, PAGE_SIZE, "%s\n", id);
}

static ssize_t dsi_panel_pcc_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct dsi_display *display = dev_get_drvdata(dev);
	struct dsi_pcc_data *pcc_data;
	u32 r, g, b;

	pcc_data = &display->panel->spec_pdata->standard_pcc_data;

	r = g = b = 0x8000;
	if (!pcc_data->color_tbl) {
		pr_err("%s: Panel has no color table\n", __func__);
		goto exit;
	}
	if (display->panel->spec_pdata->u_data == 0 && display->panel->spec_pdata->v_data == 0) {
		pr_err("%s: u,v are 0.\n", __func__);
		goto exit;
	}
	if (pcc_data->tbl_idx >= pcc_data->tbl_size) {
		pr_err("%s: Invalid color area(idx=%d)\n",
			__func__, pcc_data->tbl_idx);
		goto exit;
	}
	if (pcc_data->color_tbl[pcc_data->tbl_idx].color_type == UNUSED) {
		pr_err("%s: Unsupported color type(idx=%d)\n",
			__func__, pcc_data->tbl_idx);
		goto exit;
	}
	r = pcc_data->color_tbl[pcc_data->tbl_idx].r_data;
	g = pcc_data->color_tbl[pcc_data->tbl_idx].g_data;
	b = pcc_data->color_tbl[pcc_data->tbl_idx].b_data;
exit:
	return scnprintf(buf, PAGE_SIZE, "0x%x 0x%x 0x%x \n", r, g, b);
}

static ssize_t dsi_panel_srgb_pcc_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct dsi_display *display = dev_get_drvdata(dev);
	struct dsi_pcc_data *pcc_data;
	u32 r, g, b;

	pcc_data = &display->panel->spec_pdata->srgb_pcc_data;

	r = g = b = 0x8000;
	if (!pcc_data->color_tbl) {
		pr_err("%s: Panel has no color table\n", __func__);
		goto exit;
	}
	if (display->panel->spec_pdata->u_data == 0 && display->panel->spec_pdata->v_data == 0) {
		pr_err("%s: u,v are 0.\n", __func__);
		goto exit;
	}
	if (pcc_data->tbl_idx >= pcc_data->tbl_size) {
		pr_err("%s: Invalid color area(idx=%d)\n",
			__func__, pcc_data->tbl_idx);
		goto exit;
	}
	if (pcc_data->color_tbl[pcc_data->tbl_idx].color_type == UNUSED) {
		pr_err("%s: Unsupported color type(idx=%d)\n",
			__func__, pcc_data->tbl_idx);
		goto exit;
	}
	r = pcc_data->color_tbl[pcc_data->tbl_idx].r_data;
	g = pcc_data->color_tbl[pcc_data->tbl_idx].g_data;
	b = pcc_data->color_tbl[pcc_data->tbl_idx].b_data;
exit:
	return scnprintf(buf, PAGE_SIZE, "0x%x 0x%x 0x%x \n", r, g, b);
}

static ssize_t dsi_panel_vivid_pcc_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct dsi_display *display = dev_get_drvdata(dev);
	struct dsi_pcc_data *pcc_data;
	u32 r, g, b;

	pcc_data = &display->panel->spec_pdata->vivid_pcc_data;

	r = g = b = 0x8000;
	if (!pcc_data->color_tbl) {
		pr_err("%s: Panel has no color table\n", __func__);
		goto exit;
	}
	if (display->panel->spec_pdata->u_data == 0 && display->panel->spec_pdata->v_data == 0) {
		pr_err("%s: u,v are 0.\n", __func__);
		goto exit;
	}
	if (pcc_data->tbl_idx >= pcc_data->tbl_size) {
		pr_err("%s: Invalid color area(idx=%d)\n",
			__func__, pcc_data->tbl_idx);
		goto exit;
	}
	if (pcc_data->color_tbl[pcc_data->tbl_idx].color_type == UNUSED) {
		pr_err("%s: Unsupported color type(idx=%d)\n",
			__func__, pcc_data->tbl_idx);
		goto exit;
	}
	r = pcc_data->color_tbl[pcc_data->tbl_idx].r_data;
	g = pcc_data->color_tbl[pcc_data->tbl_idx].g_data;
	b = pcc_data->color_tbl[pcc_data->tbl_idx].b_data;
exit:
	return scnprintf(buf, PAGE_SIZE, "0x%x 0x%x 0x%x \n", r, g, b);
}

static ssize_t dsi_panel_hdr_pcc_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct dsi_display *display = dev_get_drvdata(dev);
	struct dsi_pcc_data *pcc_data;
	u32 r, g, b;

	pcc_data = &display->panel->spec_pdata->hdr_pcc_data;

	r = g = b = 0x8000;
	if (!pcc_data->color_tbl) {
		pr_err("%s: Panel has no color table\n", __func__);
		goto exit;
	}
	if (display->panel->spec_pdata->u_data == 0 && display->panel->spec_pdata->v_data == 0) {
		pr_err("%s: u,v are 0.\n", __func__);
		goto exit;
	}
	if (pcc_data->tbl_idx >= pcc_data->tbl_size) {
		pr_err("%s: Invalid color area(idx=%d)\n",
			__func__, pcc_data->tbl_idx);
		goto exit;
	}
	if (pcc_data->color_tbl[pcc_data->tbl_idx].color_type == UNUSED) {
		pr_err("%s: Unsupported color type(idx=%d)\n",
			__func__, pcc_data->tbl_idx);
		goto exit;
	}
	r = pcc_data->color_tbl[pcc_data->tbl_idx].r_data;
	g = pcc_data->color_tbl[pcc_data->tbl_idx].g_data;
	b = pcc_data->color_tbl[pcc_data->tbl_idx].b_data;
exit:
	return scnprintf(buf, PAGE_SIZE, "0x%x 0x%x 0x%x \n", r, g, b);
}

static ssize_t dsi_panel_fps_mode_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int mode_type, rc;
	struct dsi_display *display;
	struct panel_specific_pdata *spec_pdata;

	display = dev_get_drvdata(dev);
	if (!display) {
		pr_err("%s: Invalid display data\n", __func__);
		return -EINVAL;
	}

	spec_pdata = display->panel->spec_pdata;
	if (!spec_pdata) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	if (!spec_pdata->fps_mode.enable) {
		pr_err("%s: change fps not enabled\n", __func__);
		return -EINVAL;
	}

	rc = kstrtoint(buf, 10, &mode_type);
	if (rc < 0) {
		pr_err("%s: Error, buf = %s\n", __func__, buf);
		return rc;
	}

	if (mode_type == spec_pdata->fps_mode.type) {
		pr_notice("%s: fps mode is already %d\n", __func__,
			mode_type);
		return count;
	}

	rc = dsi_panel_driver_fps_mode_check_state(display, mode_type);
	if (rc) {
		pr_err("%s: Error, rc = %d\n", __func__, rc);
		return rc;
	}
	return count;
}

static ssize_t dsi_panel_fps_mode_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct panel_specific_pdata *spec_pdata = NULL;
	struct dsi_display *display = dev_get_drvdata(dev);

	spec_pdata = display->panel->spec_pdata;

	return scnprintf(buf, PAGE_SIZE, "%d\n", spec_pdata->fps_mode.type);
}

static ssize_t dsi_panel_aod_mode_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct panel_specific_pdata *spec_pdata = NULL;
	struct dsi_display *display = dev_get_drvdata(dev);
	struct dsi_panel *panel = display->panel;
	int mode;

	spec_pdata = panel->spec_pdata;

	if (sscanf(buf, "%d", &mode) < 0) {
		pr_err("sscanf failed to set mode. keep current mode=%d\n",
			spec_pdata->aod_mode);
		return -EINVAL;
	}
	if (spec_pdata->aod_mode != mode)
		spec_pdata->aod_mode = mode;

	if (mode)
		dsi_panel_set_aod_on(panel);
	else
		dsi_panel_set_aod_off(panel);

	return count;
}

static ssize_t dsi_panel_aod_mode_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct panel_specific_pdata *spec_pdata = NULL;
	struct dsi_display *display = dev_get_drvdata(dev);

	spec_pdata = display->panel->spec_pdata;

	return scnprintf(buf, PAGE_SIZE, "aod_mode = %u\n",
				spec_pdata->aod_mode);
}

static ssize_t dsi_panel_mplus_mode_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int m_mode, rc;
	struct panel_specific_pdata *spec_pdata = NULL;
	struct dsi_display *display = dev_get_drvdata(dev);

	spec_pdata = display->panel->spec_pdata;

	if (!spec_pdata->m_plus.enable) {
		pr_err("%s: m plus not enabled\n", __func__);
		return -EINVAL;
	}

	rc = kstrtoint(buf, 10, &m_mode);
	if (rc < 0) {
		pr_err("%s: Error, buf = %s\n", __func__, buf);
		return rc;
	}

	if (!spec_pdata->display_onoff_state) {
		pr_err("%s: Disp-On is not yet completed. Please retry\n",
			__func__);
		return -EINVAL;
	}

	rc = dsi_panel_mplus_mode_set(display->panel, m_mode);
	if (rc) {
		pr_err("%s: invalid value for m　plus buf = %d\n",
				__func__, m_mode);
		return -EINVAL;
	}

	return count;
}

static ssize_t dsi_panel_mplus_mode_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct panel_specific_pdata *spec_pdata = NULL;
	struct dsi_display *display = dev_get_drvdata(dev);

	spec_pdata = display->panel->spec_pdata;

	return scnprintf(buf, PAGE_SIZE, "mplus_mode = %u\n",
				spec_pdata->m_plus.mode);
}

static ssize_t dsi_panel_vr_mode_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct panel_specific_pdata *spec_pdata = NULL;
	struct dsi_display *display = dev_get_drvdata(dev);
	struct dsi_panel *panel = display->panel;
	int mode;

	spec_pdata = panel->spec_pdata;

	if (sscanf(buf, "%d", &mode) < 0) {
		pr_err("sscanf failed to set mode. keep current mode=%d\n",
			spec_pdata->vr_mode);
		return -EINVAL;
	}
	if (spec_pdata->vr_mode != mode)
		spec_pdata->vr_mode = mode;

	if (mode)
		dsi_panel_set_vr_on(panel);
	else
		dsi_panel_set_vr_off(panel);

	return count;
}

static ssize_t dsi_panel_vr_mode_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct panel_specific_pdata *spec_pdata = NULL;
	struct dsi_display *display = dev_get_drvdata(dev);

	spec_pdata = display->panel->spec_pdata;

	return scnprintf(buf, PAGE_SIZE, "vr_mode = %u\n",
				spec_pdata->vr_mode);
}

static void update_res_buf(char *string)
{
	res_buf = krealloc(res_buf, buf_sz + strnlen(string, TMP_BUF_SZ) + 1,
								GFP_KERNEL);
	if (!res_buf) {
		pr_err("%s: Failed to allocate buffer\n", __func__);
		return;
	}

	memcpy(res_buf + buf_sz, string, strnlen(string, TMP_BUF_SZ) + 1);
	buf_sz += strnlen(string, TMP_BUF_SZ); /* Exclude NULL termination */
}

static void reset_res_buf(void)
{
	kzfree(res_buf);
	res_buf = NULL;
	buf_sz = 0;
}

static int get_parameters(const char *p, u8 *par_buf, int par_buf_size,
								int *nbr_params)
{
	int ret = 0;

	while (true) {
		if (isspace(*p)) {
			p++;
		} else {
			if (sscanf(p, "%4hhx", &par_buf[*nbr_params]) == 1) {
				(*nbr_params)++;
				while (isxdigit(*p) || (*p == 'x'))
					p++;
			}
		}
		if (*nbr_params > par_buf_size) {
			update_res_buf("Too many parameters\n");
			ret = -EINVAL;
			goto exit;
		}
		if (iscntrl(*p))
			break;
	}
exit:
	return ret;
}

static int get_cmd_type(char *buf, enum dbg_cmd_type *cmd)
{
	int ret = 0;

	if (!strncmp(buf, "dcs", 3))
		*cmd = DCS;
	else if (!strncmp(buf, "gen", 3))
		*cmd = GEN;
	else
		ret = -EFAULT;
	return ret;
}

static void print_params(int dtype, u8 reg, int len, u8 *data)
{
	int i = 0;
	char tmp[TMP_BUF_SZ];

	switch (dtype) {
	case DTYPE_GEN_WRITE:
		update_res_buf("GEN_WRITE\n");
		break;
	case DTYPE_GEN_WRITE1:
		update_res_buf("GEN_WRITE1\n");
		break;
	case DTYPE_GEN_WRITE2:
		update_res_buf("GEN_WRITE2\n");
		break;
	case DTYPE_GEN_LWRITE:
		update_res_buf("GEN_LWRITE\n");
		break;
	case DTYPE_GEN_READ:
		update_res_buf("GEN_READ\n");
		break;
	case DTYPE_GEN_READ1:
		update_res_buf("GEN_READ1\n");
		break;
	case DTYPE_GEN_READ2:
		update_res_buf("GEN_READ2\n");
		break;
	case DTYPE_DCS_LWRITE:
		update_res_buf("DCS_LWRITE\n");
		break;
	case DTYPE_DCS_WRITE:
		update_res_buf("DCS_WRITE\n");
		break;
	case DTYPE_DCS_WRITE1:
		update_res_buf("DCS_WRITE1\n");
		break;
	case DTYPE_DCS_READ:
		update_res_buf("DCS_READ\n");
		break;
	default:
		snprintf(tmp, sizeof(tmp), "Unknown dtype = 0x%x\n", dtype);
		update_res_buf(tmp);
	}

	if (len > 0) {
		snprintf(tmp, sizeof(tmp), "reg=0x%.2X\n", reg);
		update_res_buf(tmp);
		snprintf(tmp, sizeof(tmp), "len=%d\n", len);
		update_res_buf(tmp);
		for (i = 0; i < len; i++) {
			snprintf(tmp, sizeof(tmp), "data[%d]=0x%.2X\n", i,
								data[i]);
			update_res_buf(tmp);
		}
	} else {
		update_res_buf("Something went wrong, length is zero.\n");
		snprintf(tmp, sizeof(tmp),
				"reg=0x%.2X, len=%d, data[0]=0x%.2X\n",
				reg, len, data[0]);
		update_res_buf(tmp);
	}
}

static ssize_t dsi_panel_driver_reg_write_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct dsi_display *display = dev_get_drvdata(dev);
	const char *p;
	enum dbg_cmd_type cmd;
	u8 data[MAX_WRITE_DATA];
	int i = 0;
	int ret;
	struct dsi_cmd_desc dsi;

	if (!display->panel->spec_pdata->display_onoff_state) {
		pr_err("%s: panel is NOT on\n", __func__);
		goto fail_free_all;
	}

	reset_res_buf();

	ret = get_cmd_type((char *)buf, &cmd);
	if (ret) {
		update_res_buf("Write - unknown type\n");
		goto fail_free_all;
	}

	p = buf;
	p = p+4;

	/* Get first param, Register */
	if (sscanf(p, "%4hhx", &data[0]) != 1) {
		update_res_buf("Write - parameter error\n");
		goto fail_free_all;
	}
	i++;

	while (isxdigit(*p) || (*p == 'x'))
		p++;

	ret = get_parameters(p, data, ARRAY_SIZE(data) - 1, &i);
	if (ret)
		goto fail_free_all;

	if (cmd == DCS) {
		if (i == 1) {
			dsi.msg.type = DTYPE_DCS_WRITE;;
		} else if (i == 2) {
			dsi.msg.type = DTYPE_DCS_WRITE1;
		} else {
			dsi.msg.type = DTYPE_DCS_LWRITE;
		}
	} else {
		if (i == 1) {
			dsi.msg.type = DTYPE_GEN_WRITE1;
		} else if (i == 2) {
			dsi.msg.type = DTYPE_GEN_WRITE2;
		} else {
			dsi.msg.type = DTYPE_GEN_LWRITE;
		}
	}

	dsi.last_command = true;
	dsi.msg.channel = 0;
	dsi.msg.flags = 0;
	dsi.msg.ctrl = 0;
	dsi.post_wait_ms = 0;
	dsi.msg.tx_len = i;
	dsi.msg.tx_buf = data;

	pr_debug("%s: last = %d, vc = %d, ack = %d, wait = %d, dlen = %zd\n",
		__func__,
		dsi.last_command, dsi.msg.channel, dsi.msg.flags, dsi.post_wait_ms,
		dsi.msg.tx_len);

	dsi_panel_tx_cmd(display->panel, &dsi);

	print_params(dsi.msg.type, data[0], i, (u8 *)dsi.msg.tx_buf);

fail_free_all:
	pr_err("END\n");
	return count;
}

static ssize_t dsi_panel_driver_reg_write_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%s\n", res_buf);
}

static ssize_t dsi_panel_driver_reg_read_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct dsi_display *display = dev_get_drvdata(dev);
	struct dsi_cmd_desc dsi;
	struct dsi_display_ctrl *m_ctrl;
	u8 data[3]; /* No more than reg + two parameters is allowed */
	int i = 0;
	int j = 0;
	int ret;
	char *rbuf;
	int nbr_bytes_to_read;
	int size_rbuf = 0;
	const char *p;

	enum dbg_cmd_type cmd;

	if (!display->panel->spec_pdata->display_onoff_state) {
		pr_err("%s: panel is NOT on\n", __func__);
		goto fail_free_all;
	}

	m_ctrl = &display->ctrl[display->cmd_master_idx];

	reset_res_buf();

	ret = get_cmd_type((char *)buf, &cmd);
	if (ret) {
		update_res_buf("Read - unknown type\n");
		goto fail_free_all;
	}

	p = buf;
	p = p+4;

	/* Get nbr_bytes_to_read */
	if (sscanf(p, "%d", &nbr_bytes_to_read) != 1) {
		update_res_buf("Read - parameter error\n");
		goto fail_free_all;
	}

	while (isxdigit(*p) || (*p == 'x'))
		p++;

	ret = get_parameters(p, data, ARRAY_SIZE(data), &i);
	if (ret)
		goto fail_free_all;

	if (cmd == DCS) {
		dsi.msg.type = DTYPE_DCS_READ;
	} else {
		if (i == 1) {
			dsi.msg.type = DTYPE_GEN_READ1;
		} else {
			dsi.msg.type = DTYPE_GEN_READ2;
		}
	}

	dsi.last_command = true;
	dsi.msg.channel = 0;
	dsi.msg.flags = 0;
	dsi.msg.ctrl = 0;
	dsi.post_wait_ms = 5;
	dsi.msg.tx_len = i;
	dsi.msg.tx_buf = data;

	for (j = 0; j < i; j++)
		pr_debug("%s: tx_buf[%d] = 0x%x\n", __func__, j,
			(*((data)+(sizeof(u8)) * j)));

	if (nbr_bytes_to_read <= 2)
		size_rbuf = 4;
	else
		size_rbuf = nbr_bytes_to_read + 13;

	rbuf = kcalloc(size_rbuf, sizeof(char), GFP_KERNEL);
	if (!rbuf)
		goto fail_free_all;

	dsi_panel_rx_cmd(display, &dsi, m_ctrl, rbuf, nbr_bytes_to_read);

	print_params(dsi.msg.type, data[0], nbr_bytes_to_read, (u8 *)rbuf);

	if (rbuf)
		kzfree(rbuf);
fail_free_all:
	return count;
}

static ssize_t dsi_panel_driver_reg_read_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%s\n", res_buf);
}

static ssize_t dsi_panel_color_mode_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct dsi_display *display = dev_get_drvdata(dev);

	pr_debug("color_mode is %d \n",
			display->panel->spec_pdata->color_mode);

	return scnprintf(buf, PAGE_SIZE, "%d \n", display->panel->spec_pdata->color_mode);
}

static ssize_t dsi_panel_color_mode_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct dsi_display *display = dev_get_drvdata(dev);

	int mode, rc = 0;

	if (sscanf(buf, "%d", &mode) < 0) {
		pr_err("sscanf failed to set mode. keep current mode=%d \n",
			display->panel->spec_pdata->color_mode);
		rc = -EINVAL;
		goto exit;
	}

	switch (mode) {
	case CLR_MODE_SELECT_SRGB:
	case CLR_MODE_SELECT_DCIP3:
	case CLR_MODE_SELECT_PANELNATIVE:
		display->panel->spec_pdata->color_mode = mode;
		break;
	default:
		pr_err("failed set mode:invalid mode=%d. keep current mode"
			"=%d \n", mode, display->panel->spec_pdata->color_mode);
		rc = -EINVAL;
		break;
	}

exit:
	return !rc ? count : rc;

}

static ssize_t dsi_panel_chargemon_exit_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%d\n", s_chargemon_exit);
}

static ssize_t dsi_panel_chargemon_exit_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int exit, rc = 0;

	if (sscanf(buf, "%d", &exit) < 0) {
		pr_err("sscanf failed to set backlight.\n");
		rc = -EINVAL;
		goto exit;
	}
	s_chargemon_exit = exit;

exit:
	return !rc ? count : rc;
}

static struct device_attribute panel_attributes[] = {
	__ATTR(panel_id, S_IRUSR, dsi_panel_driver_id_show, NULL),
	__ATTR(cc, S_IRUGO, dsi_panel_pcc_show, NULL),
	__ATTR(srgb_cc, S_IRUGO, dsi_panel_srgb_pcc_show, NULL),
	__ATTR(vivid_cc, S_IRUGO, dsi_panel_vivid_pcc_show, NULL),
	__ATTR(hdr_cc, S_IRUGO, dsi_panel_hdr_pcc_show, NULL),
	__ATTR(c_mode, S_IRUGO|S_IWUSR|S_IWGRP,
		dsi_panel_color_mode_show,
		dsi_panel_color_mode_store),
	__ATTR(fps_mode, S_IRUGO|S_IWUSR|S_IWGRP,
		dsi_panel_fps_mode_show,
		dsi_panel_fps_mode_store),
	__ATTR(panel_reg_write,  S_IRUSR|S_IRGRP|S_IWUSR|S_IWGRP,
		dsi_panel_driver_reg_write_show,
		dsi_panel_driver_reg_write_store),
	__ATTR(panel_reg_read,  S_IRUSR|S_IRGRP|S_IWUSR|S_IWGRP,
		dsi_panel_driver_reg_read_show,
		dsi_panel_driver_reg_read_store),
	__ATTR(aod_mode,  S_IRUSR|S_IRGRP|S_IWUSR|S_IWGRP,
		dsi_panel_aod_mode_show,
		dsi_panel_aod_mode_store),
	__ATTR(mplus_mode,  S_IRUSR|S_IRGRP|S_IWUSR|S_IWGRP,
		dsi_panel_mplus_mode_show,
		dsi_panel_mplus_mode_store),
	__ATTR(vr_mode,  S_IRUSR|S_IRGRP|S_IWUSR|S_IWGRP,
		dsi_panel_vr_mode_show,
		dsi_panel_vr_mode_store),
	__ATTR(chargemon_exit,  S_IRUSR|S_IRGRP|S_IWUSR|S_IWGRP,
		dsi_panel_chargemon_exit_show,
		dsi_panel_chargemon_exit_store),
};

static int dsi_panel_driver_panel_register_attributes(struct device *dev)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(panel_attributes); i++)
		if (device_create_file(dev, panel_attributes + i))
			goto error;
	return 0;

error:
	dev_err(dev, "%s: Unable to create interface\n", __func__);

	for (--i; i >= 0 ; i--)
		device_remove_file(dev, panel_attributes + i);
	return -ENODEV;
}

int dsi_panel_driver_create_fs(const struct dsi_display *display)
{
	int rc = 0;
	char *path_name = "dsi_panel_driver";
	struct device virtdev;

	dev_set_name(&virtdev, "%s", path_name);
	rc = device_register(&virtdev);
	if (rc) {
		pr_err("%s: device_register rc = %d\n", __func__, rc);
		goto err;
	}

	rc = dsi_panel_driver_panel_register_attributes(&virtdev);
	if (rc) {
		device_unregister(&virtdev);
		goto err;
	}
	dev_set_drvdata(&virtdev, (void*)display);
err:
	return rc;
}

static int find_color_area(struct dsi_pcc_data *pcc_data, int *u_data, int *v_data)
{
	int i;
	int ret = 0;

	for (i = 0; i < pcc_data->tbl_size; i++) {
		if (*u_data < pcc_data->color_tbl[i].u_min)
			continue;
		if (*u_data > pcc_data->color_tbl[i].u_max)
			continue;
		if (*v_data < pcc_data->color_tbl[i].v_min)
			continue;
		if (*v_data > pcc_data->color_tbl[i].v_max)
			continue;
		break;
	}
	pcc_data->tbl_idx = i;

	pr_debug("%s:%d, pcc_color_index = %d \n",
			__func__, __LINE__,i);

	if (i >= pcc_data->tbl_size) {
		ret = -EINVAL;
		goto exit;
	}
exit:
	return ret;
}

static void conv_uv_data(char *data, int param_type, int *u_data, int *v_data)
{
	switch (param_type) {
	case CLR_DATA_UV_PARAM_TYPE_RENE_DEFAULT:
		*u_data = ((data[0] & 0x0F) << 2) |
			/* 4bit of data[0] higher data. */
			((data[1] >> 6) & 0x03);
			/* 2bit of data[1] lower data. */
		*v_data = (data[1] & 0x3F);
			/* Remainder 6bit of data[1] is effective as v_data. */
		break;
	case CLR_DATA_UV_PARAM_TYPE_NOVA_DEFAULT:
	case CLR_DATA_UV_PARAM_TYPE_RENE_SR:
		/* 6bit is effective as u_data */
		*u_data = data[0] & 0x3F;
		/* 6bit is effective as v_data */
		*v_data = data[1] & 0x3F;
		break;
	case CLR_DATA_UV_PARAM_TYPE_NOVA_AUO:
		/* 6bit is effective as u_data */
		*u_data = data[0] & 0x3F;
		/* 6bit is effective as v_data */
		*v_data = data[2] & 0x3F;
		break;
	default:
		pr_err("%s: Failed to conv type:%d\n", __func__, param_type);
		break;
	}
}

static int get_uv_param_len(int param_type, bool *short_response)
{
	int ret = 0;

	*short_response = false;
	switch (param_type) {
	case CLR_DATA_UV_PARAM_TYPE_RENE_DEFAULT:
		ret = CLR_DATA_REG_LEN_RENE_DEFAULT;
		break;
	case CLR_DATA_UV_PARAM_TYPE_NOVA_DEFAULT:
		ret = CLR_DATA_REG_LEN_NOVA_DEFAULT;
		break;
	case CLR_DATA_UV_PARAM_TYPE_NOVA_AUO:
		ret = CLR_DATA_REG_LEN_NOVA_AUO;
		break;
	case CLR_DATA_UV_PARAM_TYPE_RENE_SR:
		ret = CLR_DATA_REG_LEN_RENE_SR;
		*short_response = true;
		break;
	default:
		pr_err("%s: Failed to get param len\n", __func__);
		break;
	}

	return ret;
}

static void get_uv_data(struct dsi_display *display, int *u_data, int *v_data)
{
	struct panel_specific_pdata *spec_pdata = NULL;
	int param_type = 0;
	char buf[DSI_LEN];
	char *tempbuf = buf;
	int len = 0;
	int i = 0;
	bool short_response = 0;
	int reslen = 0;
	char *rbuf;
	int size_rbuf = 4;
	int rlen = 0;
	struct dsi_panel *panel;
	struct dsi_cmd_desc cmdreq;
	struct dsi_display_ctrl *m_ctrl;

	pr_debug("%s (%d): Entered get_uv_data\n", __func__, __LINE__);
	panel = display->panel;
	spec_pdata = panel->spec_pdata;
	m_ctrl = &(display->ctrl[display->cmd_master_idx]);
	param_type = panel->spec_pdata->standard_pcc_data.param_type;

	rbuf = kcalloc(size_rbuf, sizeof(char), GFP_KERNEL);

	len = get_uv_param_len(param_type, &short_response);
	rlen = short_response ? 1 : len;

	for (i = 0; i < spec_pdata->uv_read_cmds.cmd_cnt; i++) {
		memset(&cmdreq, 0, sizeof(cmdreq));
		cmdreq.msg.type = DTYPE_DCS_READ;
		cmdreq.last_command = true;
		cmdreq.msg.channel = 0;
		cmdreq.msg.flags = 0;
		cmdreq.msg.ctrl = 0;
		cmdreq.post_wait_ms = 5;
		cmdreq.msg.tx_len = len;
		cmdreq.msg.tx_buf =
			*((spec_pdata->uv_read_cmds.cmds.payload) + i);
		reslen = dsi_panel_rx_cmd(display, &cmdreq,
				m_ctrl, rbuf, rlen);
		if (reslen < 0) {
			pr_err("%s (%d): rx_cmd failed.\n",
				__func__, __LINE__);
			return;
		}
		pr_debug("%s (%d): received data = %x \n",
			__func__, __LINE__, *rbuf);
		memcpy(tempbuf, rbuf, len);
		pr_debug("%s (%d): tempbuf = %x len = %d \n",
			__func__, __LINE__, *tempbuf, len);
		tempbuf += len;
	}
	conv_uv_data(buf, param_type, u_data, v_data);

	pr_debug("%s (%d): u = %x v = %x \n",
		__func__, __LINE__, *u_data, *v_data);
}

int dsi_panel_pcc_setup(struct dsi_display *display)
{
	int ret;
	struct dsi_panel *panel;
	struct panel_specific_pdata *spec_pdata = NULL;
	struct dsi_pcc_data *pcc_data = NULL;
	u8 idx = 0;
	panel = display->panel;

	if (panel == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	spec_pdata = panel->spec_pdata;

	if (spec_pdata->uv_read_cmds.cmds.send_cmd) {
		get_uv_data(display, &spec_pdata->u_data, &spec_pdata->v_data);
	}

	if (spec_pdata->u_data == 0 && spec_pdata->v_data == 0) {
		pr_err("%s (%d): u,v is flashed 0.\n", __func__, __LINE__);
		goto exit;
	}
	pr_notice("%s (%d) udata = %x vdata = %x \n", __func__, __LINE__,
		spec_pdata->u_data, spec_pdata->v_data);

	if (spec_pdata->standard_pcc_enable) {
		pcc_data = &spec_pdata->standard_pcc_data;
		if (!pcc_data->color_tbl) {
			pr_err("%s (%d): standard_color_tbl isn't found.\n",
				__func__, __LINE__);
			goto exit;
		}

		ret = find_color_area(pcc_data,
			&spec_pdata->u_data, &spec_pdata->v_data);
		if (ret) {
			pr_err("%s (%d)failed to find standard color area.\n",
				__func__, __LINE__);
			goto exit;
		}
		idx = pcc_data->tbl_idx;
		pr_notice("Standard : %s (%d): ct=%d area=%d r=0x%08X g=0x%08X b=0x%08X\n",
			__func__, __LINE__,
			pcc_data->color_tbl[idx].color_type,
			pcc_data->color_tbl[idx].area_num,
			pcc_data->color_tbl[idx].r_data,
			pcc_data->color_tbl[idx].g_data,
			pcc_data->color_tbl[idx].b_data);
	} else {
		pr_notice("%s (%d): standard_pcc isn't enabled.\n",
			__func__, __LINE__);
		goto exit;
	}

	if (spec_pdata->srgb_pcc_enable) {
		pcc_data = &spec_pdata->srgb_pcc_data;
		if (!pcc_data->color_tbl) {
			pr_err("%s (%d): srgb_color_tbl isn't found.\n",
				__func__, __LINE__);
			goto exit;
		}
		ret = find_color_area(pcc_data,
			&spec_pdata->u_data, &spec_pdata->v_data);
		if (ret) {
			pr_err("%s (%d)failed to find srgb color area.\n",
				__func__, __LINE__);
			goto exit;
		}
		idx = pcc_data->tbl_idx;
		pr_notice("SRGB : %s (%d): ct=%d area=%d r=0x%08X g=0x%08X b=0x%08X\n",
			__func__, __LINE__,
			pcc_data->color_tbl[idx].color_type,
			pcc_data->color_tbl[idx].area_num,
			pcc_data->color_tbl[idx].r_data,
			pcc_data->color_tbl[idx].g_data,
			pcc_data->color_tbl[idx].b_data);
	} else {
		pr_notice("%s (%d): srgb_pcc isn't enabled.\n",
			__func__, __LINE__);
		goto exit;
	}

	if (spec_pdata->vivid_pcc_enable) {
		pcc_data = &spec_pdata->vivid_pcc_data;
		if (!pcc_data->color_tbl) {
			pr_err("%s (%d): vivid_color_tbl isn't found.\n",
				__func__, __LINE__);
			goto exit;
		}
		ret = find_color_area(pcc_data,
			&spec_pdata->u_data, &spec_pdata->v_data);
		if (ret) {
			pr_err("%s (%d)failed to find vivid color area.\n",
				__func__, __LINE__);
			goto exit;
		}
		idx = pcc_data->tbl_idx;
		pr_notice("Vivid : %s (%d): ct=%d area=%d r=0x%08X g=0x%08X b=0x%08X\n",
			__func__, __LINE__,
			pcc_data->color_tbl[idx].color_type,
			pcc_data->color_tbl[idx].area_num,
			pcc_data->color_tbl[idx].r_data,
			pcc_data->color_tbl[idx].g_data,
			pcc_data->color_tbl[idx].b_data);
	} else {
		pr_notice("%s (%d): vivid_pcc isn't enabled.\n",
			__func__, __LINE__);
		goto exit;
	}

	if (spec_pdata->hdr_pcc_enable) {
		pcc_data = &spec_pdata->hdr_pcc_data;
		if (!pcc_data->color_tbl) {
			pr_err("%s (%d): hdr_color_tbl isn't found.\n",
				__func__, __LINE__);
			goto exit;
		}
		ret = find_color_area(pcc_data,
			&spec_pdata->u_data, &spec_pdata->v_data);
		if (ret) {
			pr_err("%s (%d)failed to find hdr color area.\n",
				__func__, __LINE__);
			goto exit;
		}
		idx = pcc_data->tbl_idx;
		pr_notice("HDR : %s (%d): ct=%d area=%d r=0x%08X g=0x%08X b=0x%08X\n",
			__func__, __LINE__,
			pcc_data->color_tbl[idx].color_type,
			pcc_data->color_tbl[idx].area_num,
			pcc_data->color_tbl[idx].r_data,
			pcc_data->color_tbl[idx].g_data,
			pcc_data->color_tbl[idx].b_data);
	} else {
		pr_notice("%s (%d): hdr_pcc isn't enabled.\n",
			__func__, __LINE__);
		goto exit;
	}

exit:
	return 0;
}

int dsi_panel_driver_enable(struct dsi_panel *panel)
{
	int rc = 0;

	if (panel == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	panel->spec_pdata->m_plus.mode = M_PLUS_MODE_INIT;

	return rc;
}

bool dsi_panel_driver_is_power_on(unsigned char state)
{
	bool ret = false;

	if (state & INCELL_POWER_STATE_ON)
		ret = true;

	pr_debug("%s: In-Cell %s state\n", __func__, (ret ? "on" : "off"));

	return ret;
}

bool dsi_panel_driver_is_power_lock(unsigned char state)
{
	bool ret = false;

	if (state & INCELL_LOCK_STATE_ON)
		ret = true;

	pr_debug("%s: In-Cell I/F %s state\n", __func__,
		(ret ? "Lock" : "Unlock"));

	return ret;
}

int dsi_panel_driver_get_chargemon_exit(void)
{
	return s_chargemon_exit;
}

void dsi_panel_driver_reset_chargemon_exit(void)
{
	s_chargemon_exit = 0;
}
