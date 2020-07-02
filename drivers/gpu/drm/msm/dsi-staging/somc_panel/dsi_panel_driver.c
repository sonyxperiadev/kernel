/*
 * Restructured unified display panel driver for Xperia Open Devices
 *                          *** Main ***
 *
 * This driver is based on various SoMC implementations found in
 * copyleft archives for various devices.
 *
 * Copyright (c) 2012-2014, The Linux Foundation. All rights reserved.
 * Copyright (c) 2017 Sony Mobile Communications Inc. All rights reserved.
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

#include <linux/module.h>

#include <linux/sysfs.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/notifier.h>
#include <linux/export.h>
#include <linux/drm_notify.h>
#include <linux/sde_io_util.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>
#include "somc_panel_exts.h"
#include "dsi_display.h"
#include "dsi_panel.h"

#define AOD_MODE_THRESHOLD 8

static BLOCKING_NOTIFIER_HEAD(drm_notifier_list);

static u32 down_period;
static short display_sod_mode = 0;
static short display_pre_sod_mode = 0;
static short display_aod_mode = 0;

struct device virtdev;

static char *res_buf;
static int buf_sz;
#define DSI_BUF_SIZE 1024
#define TMP_BUF_SZ 128
#define MAX_WRITE_DATA 100

#define NOT_CHARGEMON_EXIT 0
static int s_chargemon_exit = NOT_CHARGEMON_EXIT;

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

#ifdef CONFIG_SOMC_PANEL_LABIBB
	somc_panel_vregs_init(panel);
#endif

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
	if (gpio_is_valid(spec_pdata->touch_vddh_en_gpio)) {
		rc = gpio_request(spec_pdata->touch_vddh_en_gpio,
							"touch_vddh_en");
		if (rc) {
			pr_err("request for touch_vddio_en failed, rc=%d\n",
									rc);
			goto error_release_touch_vddh_en;
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
	if (gpio_is_valid(spec_pdata->disp_oled_vci_gpio)) {
		rc = gpio_request(spec_pdata->disp_oled_vci_gpio,
							"disp_oled_vci_gpio");
		if (rc) {
			pr_err("request for vci_en gpio failed, rc=%d\n", rc);
			goto error_release_disp_vci_en;
		}
	}

	if (gpio_is_valid(spec_pdata->disp_err_fg_gpio)) {
		rc = gpio_request(spec_pdata->disp_err_fg_gpio,
							"disp_err_fg_gpio");
		if (rc) {
			pr_err("request disp err fg gpio failed, rc=%d\n", rc);
			goto error_release_disp_err_fg;
		}
	}

	goto no_error;

error_release_disp_err_fg:
	if (gpio_is_valid(spec_pdata->disp_err_fg_gpio))
		gpio_free(spec_pdata->disp_err_fg_gpio);
error_release_disp_vci_en:
	if (gpio_is_valid(spec_pdata->disp_oled_vci_gpio))
		gpio_free(spec_pdata->disp_oled_vci_gpio);
error_release_disp_dcdc_en:
	if (gpio_is_valid(spec_pdata->disp_dcdc_en_gpio))
		gpio_free(spec_pdata->disp_dcdc_en_gpio);
error_release_touch_reset:
	if (gpio_is_valid(spec_pdata->reset_touch_gpio))
		gpio_free(spec_pdata->reset_touch_gpio);
error_release_touch_vddh_en:
	if (gpio_is_valid(spec_pdata->touch_vddh_en_gpio))
		gpio_free(spec_pdata->touch_vddh_en_gpio);
error_release_touch_vddio_en:
	if (gpio_is_valid(spec_pdata->touch_vddio_en_gpio))
		gpio_free(spec_pdata->touch_vddio_en_gpio);
no_error:
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

	if (gpio_is_valid(spec_pdata->touch_vddh_en_gpio))
		gpio_free(spec_pdata->touch_vddh_en_gpio);

	if (gpio_is_valid(spec_pdata->disp_oled_vci_gpio))
		gpio_free(spec_pdata->disp_oled_vci_gpio);

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

	if (!gpio_is_valid(spec_pdata->reset_touch_gpio))
		return 0;

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

	if (!gpio_is_valid(spec_pdata->reset_touch_gpio))
		return 0;

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

static int dsi_panel_driver_touch_power_on(struct dsi_panel *panel)
{
	struct panel_specific_pdata *spec_pdata = panel->spec_pdata;
	bool vddio_present = gpio_is_valid(spec_pdata->touch_vddio_en_gpio);
	int rc = 0;

	if (vddio_present) {
		rc = gpio_direction_output(spec_pdata->touch_vddio_en_gpio, 0);
		if (rc) {
			pr_err("unable to set dir for touch_vddio_en gpio rc=%d\n", rc);
			goto exit;
		}
	}

	if (gpio_is_valid(spec_pdata->touch_vddh_en_gpio)) {
		rc = gpio_direction_output(spec_pdata->touch_vddh_en_gpio, 1);
		if (rc) {
			pr_err("unable to set dir for touch_vddh_en "
			       "gpio rc=%d\n", rc);

			/* Revert the VDDIO enablement */
			if (vddio_present)
				gpio_direction_output(
					spec_pdata->touch_vddio_en_gpio, 1);
			goto exit;
		}
	}

exit:
	return rc;
}

static int dsi_panel_driver_touch_power_off(struct dsi_panel *panel)
{
	struct panel_specific_pdata *spec_pdata = panel->spec_pdata;
	int rc = 0;

	if (gpio_is_valid(spec_pdata->touch_vddh_en_gpio)) {
		rc = gpio_direction_output(spec_pdata->touch_vddh_en_gpio, 0);
		if (rc) {
			pr_err("unable to set dir for touch_vddh_en gpio rc=%d\n", rc);
			goto exit;
		}
	}

	if (gpio_is_valid(spec_pdata->touch_vddio_en_gpio)) {
		rc = gpio_direction_output(spec_pdata->touch_vddio_en_gpio, 1);
		if (rc) {
			pr_err("unable to set dir for touch_vddio_en gpio rc=%d\n", rc);
			goto exit;
		}
	}
exit:
	return rc;
}

int dsi_panel_driver_touch_power(struct dsi_panel *panel, bool enable)
{
	if (!panel) {
		pr_err("%s: Invalid input panel\n", __func__);
		return -EINVAL;
	}

	if (enable)
		return dsi_panel_driver_touch_power_on(panel);
	else
		return dsi_panel_driver_touch_power_off(panel);
}

int somc_panel_external_control_touch_power(bool enable)
{
	struct dsi_display *display = dsi_display_get_main_display();

	return dsi_panel_driver_touch_power(display->panel, enable);
}
EXPORT_SYMBOL(somc_panel_external_control_touch_power);

int somc_panel_cont_splash_touch_enable(struct dsi_panel *panel)
{
	struct panel_specific_pdata *spec_pdata = NULL;
	int rc;

	if (!panel) {
		pr_err("%s: Invalid input panel\n", __func__);
		return -EINVAL;
	}

	spec_pdata = panel->spec_pdata;

	rc = somc_panel_vreg_ctrl(&spec_pdata->touch_power_info,
						"touch-avdd", true);
	if (rc)
		pr_warn("%s: failed to enable touch-avdd, rc=%d\n",
				__func__, rc);

	rc = dsi_panel_driver_touch_power(panel, true);
	if (rc)
		pr_warn("%s: failed to enable touch vddio, rc=%d\n",
				__func__, rc);

	dsi_panel_driver_touch_reset(panel);

	return 0;
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

	if (spec_pdata->lp11_off)
		usleep_range(spec_pdata->lp11_off * 1000,
				spec_pdata->lp11_off * 1000 + 100);

	if (!spec_pdata->oled_disp) {
		rc = somc_panel_vreg_ctrl(
			&spec_pdata->vspvsn_power_info, "ibb", false);
		if (rc)
			pr_err("%s: failed to disable vsn, rc=%d\n", __func__, rc);

		rc = somc_panel_vreg_ctrl(
			&spec_pdata->vspvsn_power_info, "lab", false);
		if (rc)
			pr_err("%s: failed to disable vsp, rc=%d\n", __func__, rc);
	}

	if (!spec_pdata->rst_b_seq) {
		rc = dsi_panel_driver_reset_panel(panel, 0);
		if (rc)
			pr_warn("%s: Panel reset failed. rc=%d\n", __func__, rc);
	}

	if (!spec_pdata->pre_sod_mode)
		dsi_panel_driver_touch_pinctrl_set_state(panel, false);

	/* Power off touch on OLED panels only if not in pre_sod.
	 * On LCD panels, always power off touch
	 */
	if (!spec_pdata->oled_disp ||
	    (spec_pdata->oled_disp && !spec_pdata->pre_sod_mode)) {
		rc = dsi_panel_driver_touch_power(panel, false);
		if (rc)
			pr_err("%s: touch GPIO poweroff failed, rc=%d\n",
			       __func__, rc);
	}

	if (spec_pdata->oled_disp) {
		/* The Vci is present only on some OLED displays and must be
		 * disabled only after disabling the VDDh through GPIO
		 */
		rc = somc_panel_vreg_ctrl(&panel->power_info, "vci", false);
		if (rc)
			pr_err("%s: failed to disable vci, rc=%d\n",
			       __func__, rc);

		/* Some devices have got a fixed regulator as Vci... */
		if (gpio_is_valid(spec_pdata->disp_oled_vci_gpio)) {
			rc = gpio_direction_output(spec_pdata->disp_oled_vci_gpio, 1);
			if (rc)
				pr_err("%s: unable to set dir for vci_en gpio "
				       " rc=%d\n", __func__, rc);
			gpio_set_value(spec_pdata->disp_oled_vci_gpio, 0);
		}
	}

	rc = somc_panel_vreg_ctrl(&spec_pdata->touch_power_info,
				  "touch-avdd", false);
	if (rc)
		pr_err("%s: failed to disable touch-avdd, rc=%d\n",
		       __func__, rc);

	rc = somc_panel_vreg_ctrl(&panel->power_info, "vddio", false);
	if (rc)
		pr_err("%s: failed to disable vddio, rc=%d\n", __func__, rc);

	if (gpio_is_valid(spec_pdata->disp_vddio_gpio)) {
		rc = gpio_direction_output(spec_pdata->disp_vddio_gpio, 1);
		if (rc)
			pr_err("unable to set dir for disp_vddio gpio rc=%d\n", rc);
	}

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

	spec_pdata->sod_mode = SOD_POWER_ON;
	spec_pdata->pre_sod_mode = SOD_POWER_ON;
	spec_pdata->aod_mode = SDE_MODE_DPMS_ON;

	drm_notifier_call_chain(DRM_EXT_EVENT_BEFORE_BLANK, &event);

	rc = somc_panel_vreg_ctrl(&panel->power_info, "vddio", true);
	if (rc) {
		pr_err("%s: failed to enable vddio, rc=%d\n", __func__, rc);
		goto exit;
	}

	rc = somc_panel_vreg_ctrl(&spec_pdata->touch_power_info, "touch-avdd", true);
	if (rc) {
		pr_err("%s: failed to enable touch-avdd, rc=%d\n", __func__, rc);
		goto exit;
	}

	if (gpio_is_valid(spec_pdata->disp_vddio_gpio)) {
		rc = gpio_direction_output(spec_pdata->disp_vddio_gpio, 0);
		if (rc) {
			pr_err("unable to set dir for disp_vddio gpio rc=%d\n", rc);
			goto exit;
		}
	}

	rc = dsi_panel_driver_touch_power(panel, true);
	if (rc) {
		pr_err("%s: failed to enable touch vddio, rc=%d\n", __func__, rc);
		goto exit;
	}

	if (spec_pdata->oled_disp) {
		rc = somc_panel_vreg_ctrl(&panel->power_info, "vci", true);
		if (rc) {
			pr_err("%s: failed to enable vci, rc=%d\n",
			       __func__, rc);
			goto exit;
		}

		if (gpio_is_valid(spec_pdata->disp_oled_vci_gpio)) {
			rc = gpio_direction_output(spec_pdata->disp_oled_vci_gpio, 0);
			if (rc) {
				pr_err("%s: failed to enable vci GPIO rc=%d\n",
				       __func__, rc);
			}
			gpio_set_value(spec_pdata->disp_oled_vci_gpio, 1);
		}
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

	if (!spec_pdata->oled_disp) {
		rc = somc_panel_vreg_ctrl(
			&spec_pdata->vspvsn_power_info, "lab", true);
		if (rc) {
			pr_err("%s: failed to enable vsp, rc=%d\n", __func__, rc);
			goto exit;
		}

		rc = somc_panel_vreg_ctrl(
			&spec_pdata->vspvsn_power_info, "ibb", true);
		if (rc) {
			pr_err("%s: failed to enable vsn, rc=%d\n", __func__, rc);
			goto exit;
		}
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

	if (!spec_pdata->oled_disp)
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
	const char *rst_seq, *panel_type;

	if (!panel) {
		pr_err("%s: Invalid input panel\n", __func__);
		return -EINVAL;
	}

	if (!panel->spec_pdata) {
		pr_err("%s: spec_pdata not initialized!!\n", __func__);
		return -EINVAL;
	}

	spec_pdata = panel->spec_pdata;

	if (of_property_read_u32(np, "somc,incell-touch-type",
			&panel->touch_type))
		panel->touch_type = INCELL_TOUCH_TYPE_DEFAULT;
	pr_debug("%s: In-Cell touch IC is %d\n", __func__, panel->touch_type);

	rc = somc_panel_parse_dt_colormgr_config(panel, np);

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

	panel_type = of_get_property(np, "somc,dsi-panel-type", NULL);
	if (!panel_type) {
		pr_debug("%s:failed parse dsi-panel-type \n", __func__);
		spec_pdata->oled_disp = false;
	} else if (!strcmp(panel_type, "oled")) {
		spec_pdata->oled_disp = true;
	} else {
		pr_err("%s:failed parse dsi-panel-type \n", __func__);
		spec_pdata->oled_disp = false;
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

	rc = of_property_read_u32(np, "somc,pw-wait-after-off-lp11", &tmp);
	spec_pdata->lp11_off = !rc ? tmp : 0;

	rc = of_property_read_u32(np,
			"somc,aod-threshold", &tmp);
	spec_pdata->aod_threshold = !rc ? tmp : AOD_MODE_THRESHOLD;

	panel->lp11_init = of_property_read_bool(np,
			"qcom,mdss-dsi-lp11-init");

	spec_pdata->hbm.hbm_supported = of_property_read_bool(np,
			"somc,set-hbm-usable");

	rc = somc_panel_parse_dt_chgfps_config(panel, np);
	if (rc) {
		pr_err("failed to parse Low Fps, rc=%d\n", rc);
	}

	dsi_panel_driver_parse_dt_mplus(panel, np);

	rc = somc_panel_vregs_parse_dt(panel, np);

	return 0;
}

int dsi_panel_driver_parse_power_cfg(struct dsi_panel *panel)
{
	int rc = 0;
	struct panel_specific_pdata *spec_pdata = NULL;

	if (!panel) {
		pr_err("%s: Invalid input panel\n", __func__);
		return -EINVAL;
	}
	spec_pdata = panel->spec_pdata;

	rc = dsi_pwr_of_get_vreg_data(&panel->utils,
					  &spec_pdata->touch_power_info,
					  "qcom,panel-touch-supply-entries");
	if (rc)
		pr_notice("%s: Not configured touch vregs\n", __func__);

	rc = dsi_pwr_of_get_vreg_data(&panel->utils,
					  &spec_pdata->vspvsn_power_info,
					  "qcom,panel-vspvsn-supply-entries");
	if (rc)
		pr_err("%s: Not configured vsp/vsn vregs\n", __func__);

	return 0;
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

	spec_pdata->touch_vddh_en_gpio = of_get_named_gpio(of_node,
					      "qcom,platform-touch-vddh-en-gpio",
					      0);
	if (!gpio_is_valid(spec_pdata->touch_vddh_en_gpio)) {
		pr_err("%s: touch-vddh-en gpio not present\n", __func__);
	}

	spec_pdata->touch_int_gpio = of_get_named_gpio(of_node,
					      "qcom,platform-touch-int-gpio",
					      0);
	if (!gpio_is_valid(spec_pdata->touch_int_gpio)) {
		pr_err("%s: failed get touch-int-gpio\n", __func__);
	}

	spec_pdata->reset_touch_gpio = of_get_named_gpio(of_node,
					      "qcom,platform-touch-reset-gpio",
					      0);
	if (!gpio_is_valid(spec_pdata->reset_touch_gpio)) {
		pr_err("%s: failed get reset touch gpio\n", __func__);
	} else {
		rc = dsi_panel_driver_parse_reset_touch_sequence(panel,
								 of_node);
		if (rc) {
			pr_err("%s: failed to parse reset touch sequence\n",
			       __func__);
			goto error;
		}
	}

	spec_pdata->disp_vddio_gpio = of_get_named_gpio(of_node,
					      "qcom,platform-vddio-gpio",
					      0);
	if (!gpio_is_valid(spec_pdata->disp_vddio_gpio)) {
		pr_err("%s: failed get vddio-gpio\n", __func__);
	}

	spec_pdata->disp_dcdc_en_gpio = of_get_named_gpio(of_node,
						"somc,disp-dcdc-en-gpio",
						0);
	if (!gpio_is_valid(spec_pdata->disp_dcdc_en_gpio)) {
		pr_err("%s: disp dcdc en gpio not specified\n", __func__);
	}

	spec_pdata->disp_oled_vci_gpio = of_get_named_gpio(panel->panel_of_node,
					      "somc,disp-vci-en-gpio",
					      0);
	if (!gpio_is_valid(spec_pdata->disp_oled_vci_gpio)) {
		pr_err("%s: failed get vci enable gpio\n", __func__);
	}

	spec_pdata->disp_err_fg_gpio = of_get_named_gpio(of_node,
					      "somc,disp-err-flag-gpio",
					      0);
	if (!gpio_is_valid(spec_pdata->disp_err_fg_gpio)) {
		pr_err("%s: failed get disp error flag gpio\n", __func__);
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

		/* SoMC Yoshino Maple: If powering on, 2x sleep */
		if (en)
			usleep_range(6000, 7000);
		usleep_range(7000, 7500);
	}
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

int dsi_panel_driver_toggle_light_off(struct dsi_panel *panel, bool bl_on)
{
	int rc = 0;
	struct panel_specific_pdata *spec_pdata;

	spec_pdata = panel->spec_pdata;

	if (spec_pdata->light_state != bl_on) {
		if (!bl_on) {
			rc = dsi_panel_tx_cmd_set(panel,
				DSI_CMD_SET_DISPLAY_OFF);
			if (rc) {
				pr_err("failed to set display off.\
					keep current bl_on=%d\n",
					spec_pdata->light_state);
				return -EINVAL;
			} else {
				spec_pdata->light_state = bl_on;
				pr_debug("success. bl_on=%d\n",
					spec_pdata->light_state);
			}
		} else {
			rc = dsi_panel_tx_cmd_set(panel,
				DSI_CMD_SET_DISPLAY_ON);
			if (rc) {
				pr_err("failed to set display on.\
					keep current bl_on=%d\n",
					spec_pdata->light_state);
				return -EINVAL;
			} else {
				spec_pdata->light_state = bl_on;
				pr_debug("success. bl_on=%d\n",
					spec_pdata->light_state);
			}
		}
	}
	return rc;
}


int somc_panel_set_dyn_hbm_backlight(struct dsi_panel *panel,
				     int prev_bl_lvl, int bl_lvl)
{
	struct panel_specific_pdata *spec_pdata = panel->spec_pdata;

	/*
	 * HBM is a specific command set for Samsung panels:
	 * sometimes we want to avoid enabling support for it
	 * because it may burn the OLED matrix on some specific
	 * hardware.
	 * If this display does not support HBM, either by
	 * choice or because it really doesn't support it at
	 * all, then just go on without any error.
	 */
	if (!spec_pdata->hbm.hbm_supported)
		return 0;

	/* Handle HBM disablement for lower backlight level */
	if (bl_lvl < prev_bl_lvl && spec_pdata->hbm.hbm_mode != 0) {
		spec_pdata->hbm.force_hbm_off = false;
		return dsi_panel_set_hbm_mode(panel, 0);
	}

	/*
	 * This shouldn't happen: the qcom backlight handling
	 * is not supposed to call this function in this case.
	 * Better safe than sorry, by the way.
	 */
	if (unlikely(bl_lvl == prev_bl_lvl))
		return 0;

	/*
	 * If we reach this point, then the only thing that we
	 * can possibly ever want to do is to enter HBM mode...
	 */
	if (bl_lvl == panel->bl_config.brightness_max_level)
		return dsi_panel_set_hbm_mode(panel, 1);

	return 0;
}

static void somc_panel_hbm_protect_work(struct work_struct *work)
{
	struct delayed_work *dwork = to_delayed_work(work);
	struct dsi_panel *panel =
		container_of(dwork, struct dsi_panel, hbm_protect_work);
	struct panel_specific_pdata *spec_pdata = panel->spec_pdata;
	int max_bl_lvl;

	/* Shut down HBM to protect the OLED matrix */
	if (spec_pdata->hbm.hbm_mode > 0) {
		spec_pdata->hbm.force_hbm_off = true;
		dsi_panel_set_hbm_mode(panel, -1);
		return;
	}

	/*
	 * IMPORTANT: This branch is reached only when COOLDOWN TIMER
	 *            fires.
	 * ********* force_hbm_off == true, hbm_mode == -1 *********
	 *
	 * If cooldown time reached and still want HBM, re-enable it,
	 * but do it only for X times, otherwise assume that the OS
	 * has locked up in HBM range: in this case, manual intervention
	 * is required. The OS has to request a non-HBM range brightness
	 * in order to reset this behavior.
	 */
	if (spec_pdata->hbm.force_hbm_off) {
		max_bl_lvl = panel->bl_config.brightness_max_level;

		if (panel->bl_config.bl_level != max_bl_lvl) {
			/* HBM is off by userspace intervention. Reset. */
			spec_pdata->hbm.force_hbm_off = false;
			return;
		} else if (spec_pdata->hbm.ncooldowns < HBM_MAX_COOLDOWNS) {
			/* HBM is still on after cooldown: reenable it */
			dsi_panel_set_hbm_mode(panel, 1);
			spec_pdata->hbm.ncooldowns++;
			return;
		}
	}

	/*
	 * Super Extra Mega Paranoid hardware protection kick-in:
	 * this point is reached only if something really stupid happens.
	 */
	dsi_panel_set_hbm_mode(panel, 0);
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

static void dsi_panel_driver_oled_short_det_setup(struct dsi_panel *panel,
								bool enable)
{
	if (!panel) {
		pr_err("%s: Invalid input data\n", __func__);
		return;
	}

	if (!panel->spec_pdata->oled_disp)
		return;

	if (enable)
		dsi_panel_driver_oled_short_det_enable(
				panel->spec_pdata, SHORT_WORKER_PASSIVE);
	else
		dsi_panel_driver_oled_short_det_disable(panel->spec_pdata);
}

void dsi_panel_driver_post_enable(struct dsi_panel *panel)
{
	panel->spec_pdata->display_onoff_state = true;

	dsi_panel_driver_oled_short_det_setup(panel, true);
	somc_panel_colormgr_apply_calibrations(
			panel->spec_pdata->color_mgr->pcc_profile);
	somc_panel_fps_cmd_send(panel);
	dsi_panel_driver_notify_resume(panel);

	/* Set it after resume so that the mXT640u can cancel SOD */
	display_sod_mode = SOD_POWER_ON;
}

void dsi_panel_driver_pre_disable(struct dsi_panel  *panel)
{
	dsi_panel_driver_oled_short_det_setup(panel, false);
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

int somc_panel_set_doze_mode(struct drm_connector *connector,
		int power_mode, void *disp)
{
	struct dsi_display *display = disp;
	struct dsi_panel *panel = display->panel;
	struct panel_specific_pdata *spec_pdata;
	int rc = 0;

	if (!display || !display->panel) {
		pr_err("invalid display/panel\n");
		return -EINVAL;
	}
	spec_pdata = panel->spec_pdata;

	/* AOD/SOD supported only on OLED display */
	if (!spec_pdata->oled_disp)
		return 0;

	pr_debug("somc_panel: set doze mode %d\n", power_mode);

	mutex_lock(&display->display_lock);

	switch (power_mode) {
		case SDE_MODE_DPMS_ON:
			pr_debug("Requested DPMS ON state.\n");
			if (display_aod_mode != SDE_MODE_DPMS_OFF)
				rc = dsi_panel_set_aod_off(panel);
			spec_pdata->aod_mode = power_mode;
			display_aod_mode = 0;
			dsi_panel_driver_notify_resume(panel);
			break;
		case SDE_MODE_DPMS_LP1:
			pr_info("Entering LP1 state from %d\n",
					spec_pdata->aod_mode);

			spec_pdata->sod_mode = SOD_POWER_ON;
			spec_pdata->pre_sod_mode = SOD_POWER_OFF_SKIP;
			display_sod_mode = spec_pdata->sod_mode;
			display_pre_sod_mode = spec_pdata->pre_sod_mode;

			if (spec_pdata->aod_mode != SDE_MODE_DPMS_ON)
				break;

			spec_pdata->aod_mode = power_mode;
			display_aod_mode = power_mode;

			rc = dsi_panel_set_aod_on(panel);
			dsi_panel_driver_notify_suspend(panel);
			break;
		case SDE_MODE_DPMS_LP2:
			pr_info("Entering LP2 state from %d\n",
					spec_pdata->aod_mode);

			/* If we're coming from ON state, pass through LP1 */
			if (spec_pdata->aod_mode == SDE_MODE_DPMS_ON) {
				mutex_unlock(&display->display_lock);
				somc_panel_set_doze_mode(connector,
							 SDE_MODE_DPMS_LP1,
							 disp);
				mutex_lock(&display->display_lock);
			}

			spec_pdata->sod_mode = SOD_POWER_ON;
			spec_pdata->pre_sod_mode = SOD_POWER_OFF_SKIP;
			display_sod_mode = spec_pdata->sod_mode;
			display_pre_sod_mode = spec_pdata->pre_sod_mode;

			spec_pdata->aod_mode = power_mode;
			display_aod_mode = power_mode;
			dsi_panel_driver_notify_suspend(panel);
			break;
		case SDE_MODE_DPMS_OFF:
			pr_info("Entering OFF state from %d\n",
					spec_pdata->aod_mode);

			spec_pdata->sod_mode = SOD_POWER_OFF;
			spec_pdata->pre_sod_mode = SOD_POWER_OFF;
			display_sod_mode = spec_pdata->sod_mode;
			display_pre_sod_mode = spec_pdata->pre_sod_mode;

			spec_pdata->aod_mode = power_mode;
			display_aod_mode = power_mode;
			dsi_panel_driver_notify_suspend(panel);
			rc = dsi_panel_set_aod_off(panel);
			break;
		default:
			pr_err("Requested unexpected state %d. "
			       "Disabling AOD.\n", power_mode);
			rc = dsi_panel_set_aod_off(panel);
			break;
	}

	panel->power_mode = power_mode;

	mutex_unlock(&display->display_lock);

	return rc;
}


static ssize_t dsi_panel_aod_mode_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct panel_specific_pdata *spec_pdata = NULL;
	struct dsi_display *display = dev_get_drvdata(dev);
	struct dsi_panel *panel = display->panel;
	int mode;

	spec_pdata = panel->spec_pdata;

	if (!spec_pdata->display_onoff_state) {
		pr_err("%s: Disp-On is not yet completed. Please retry\n",
			__func__);
		return -EINVAL;
	}

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

static ssize_t dsi_panel_pre_sod_mode_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int mode;
	struct panel_specific_pdata *spec_pdata = NULL;
	struct dsi_display *display = dev_get_drvdata(dev);

	spec_pdata = display->panel->spec_pdata;

	if (sscanf(buf, "%d", &mode) < 0) {
		pr_err("sscanf failed to set mode. keep current mode=%d\n",
			spec_pdata->pre_sod_mode);
		return -EINVAL;
	}

	spec_pdata->pre_sod_mode = mode;
	pr_info("%s: sod mode setting %d\n", __func__, spec_pdata->pre_sod_mode);

	return count;

}

static ssize_t dsi_panel_pre_sod_mode_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct panel_specific_pdata *spec_pdata = NULL;
	struct dsi_display *display = dev_get_drvdata(dev);

	spec_pdata = display->panel->spec_pdata;

	return scnprintf(buf, PAGE_SIZE, "pre_sod_mode = %u\n",
				spec_pdata->pre_sod_mode);
}

static ssize_t dsi_panel_sod_mode_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int mode;
	struct panel_specific_pdata *spec_pdata = NULL;
	struct dsi_display *display = dev_get_drvdata(dev);
	struct dsi_panel *panel = display->panel;

	spec_pdata = display->panel->spec_pdata;

	if (sscanf(buf, "%d", &mode) < 0) {
		pr_err("sscanf failed to set mode. keep current mode=%d\n",
			spec_pdata->sod_mode);
		return -EINVAL;
	}
	mutex_lock(&display->display_lock);

	if (spec_pdata->sod_mode != mode) {
		pr_info("%s: current %d request %d\n", __func__,
			spec_pdata->sod_mode, mode);
		if (mode == SOD_POWER_ON && spec_pdata->display_onoff_state) {
			spec_pdata->sod_mode = mode;
			display_sod_mode = spec_pdata->sod_mode;
			dsi_panel_driver_notify_resume(panel);
		} else if (mode == SOD_POWER_ON) {
			spec_pdata->sod_mode = mode;
			display_sod_mode = spec_pdata->sod_mode;
		} else if (mode == SOD_POWER_OFF &&
			   spec_pdata->display_onoff_state &&
			   spec_pdata->aod_mode) {
			pr_info("%s: power off\n", __func__);
			spec_pdata->sod_mode = mode;
			display_sod_mode = spec_pdata->sod_mode;
			dsi_panel_driver_notify_suspend(panel);
		} else if (mode == SOD_POWER_OFF &&
			   !spec_pdata->display_onoff_state) {
			pr_info("%s: power off\n", __func__);
			spec_pdata->sod_mode = mode;
			display_sod_mode = spec_pdata->sod_mode;
			dsi_panel_driver_notify_suspend(panel);
		}
	}
	mutex_unlock(&display->display_lock);
	return count;
}

static ssize_t dsi_panel_sod_mode_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct panel_specific_pdata *spec_pdata = NULL;
	struct dsi_display *display = dev_get_drvdata(dev);

	spec_pdata = display->panel->spec_pdata;

	return scnprintf(buf, PAGE_SIZE, "sod_mode = %u\n", spec_pdata->sod_mode);
}

int somc_panel_get_display_pre_sod_mode(void)
{
	pr_debug("%s: pre_sod mode setting %d\n", __func__, display_sod_mode);
	return display_pre_sod_mode;
}

int get_display_sod_mode(void)
{
	pr_debug("%s: sod mode setting %d\n", __func__, display_sod_mode);
	return display_sod_mode;
}

int somc_panel_get_display_aod_mode(void)
{
	pr_debug("%s: sod mode setting %d\n", __func__, display_sod_mode);
	return display_aod_mode;
}

static ssize_t dsi_panel_hbm_mode_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct dsi_display *display = dev_get_drvdata(dev);
	struct dsi_panel *panel = display->panel;
	int mode;

	mutex_lock(&display->display_lock);
	if (!panel->spec_pdata->display_onoff_state) {
		pr_err("%s: Display is off, can't set HBM status\n", __func__);
		goto hbm_error;
	}

	if (sscanf(buf, "%d", &mode) < 0) {
		pr_err("sscanf failed to set mode. keep current mode=%d\n",
			panel->spec_pdata->hbm.hbm_mode);
		goto hbm_error;
	}

	dsi_panel_set_hbm_mode(panel, mode);

	mutex_unlock(&display->display_lock);
	return count;

hbm_error:
	mutex_unlock(&display->display_lock);
	return -EINVAL;
}

static ssize_t dsi_panel_hbm_mode_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct dsi_display *display = dev_get_drvdata(dev);
	struct panel_specific_pdata *spec_pdata = display->panel->spec_pdata;

	return scnprintf(buf, PAGE_SIZE, "%u", spec_pdata->hbm.hbm_mode);
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
	__ATTR(panel_reg_write,  S_IRUSR|S_IRGRP|S_IWUSR|S_IWGRP,
		dsi_panel_driver_reg_write_show,
		dsi_panel_driver_reg_write_store),
	__ATTR(panel_reg_read,  S_IRUSR|S_IRGRP|S_IWUSR|S_IWGRP,
		dsi_panel_driver_reg_read_show,
		dsi_panel_driver_reg_read_store),
	__ATTR(aod_mode,  S_IRUSR|S_IRGRP|S_IWUSR|S_IWGRP,
		dsi_panel_aod_mode_show,
		dsi_panel_aod_mode_store),
	__ATTR(pre_sod_mode,  S_IRUSR|S_IRGRP|S_IWUSR|S_IWGRP,
		dsi_panel_pre_sod_mode_show,
		dsi_panel_pre_sod_mode_store),
	__ATTR(sod_mode,  S_IRUSR|S_IRGRP|S_IWUSR|S_IWGRP,
		dsi_panel_sod_mode_show,
		dsi_panel_sod_mode_store),
	__ATTR(hbm_mode, (S_IRUSR|S_IRGRP|S_IWUSR|S_IWGRP),
		dsi_panel_hbm_mode_show,
		dsi_panel_hbm_mode_store),
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
	int rc, i;

	for (i = 0; i < ARRAY_SIZE(panel_attributes); i++)
		if (device_create_file(dev, panel_attributes + i))
			goto error;

	rc = somc_panel_fps_register_attr(dev);
	if (unlikely(rc != 0))
		goto error;

	rc = somc_panel_colormgr_register_attr(dev);
	if (unlikely(rc != 0))
		goto error;

	return rc;

error:
	dev_err(dev, "%s: Unable to create interface\n", __func__);

	for (--i; i >= 0 ; i--)
		device_remove_file(dev, panel_attributes + i);
	return -ENODEV;
}

int dsi_panel_driver_create_fs(struct dsi_display *display)
{
	int rc = 0;
	char *path_name = "dsi_panel_driver";

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
	dev_set_drvdata(&virtdev, display);
err:
	return rc;
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

static irqreturn_t dsi_panel_driver_oled_short_det_handler(int irq, void *dev)
{
	struct dsi_display *display = dev;
	struct short_detection_ctrl *short_det =
				&display->panel->spec_pdata->short_det;

	if (display == NULL || short_det == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		goto exit;
	}

	pr_err("%s: VREG_NG interrupt!\n", __func__);

	if ((gpio_get_value(display->panel->spec_pdata->disp_err_fg_gpio)) == 1)
		pr_err("%s: VREG NG!!!\n", __func__);
	else
		goto exit;

	if (short_det->short_check_working) {
		pr_debug("%s already being check work.\n", __func__);
		goto exit;
	}

	short_det->current_chatter_cnt = SHORT_CHATTER_CNT_START;

	schedule_delayed_work(&short_det->check_work,
		msecs_to_jiffies(short_det->target_chatter_check_interval));

	goto exit;

exit:
	return IRQ_HANDLED;
}

void dsi_panel_driver_oled_short_det_enable(
		struct panel_specific_pdata *spec_pdata, bool inWork)
{
	struct short_detection_ctrl *short_det = &spec_pdata->short_det;

	if (short_det == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return;
	}

	if (short_det->short_check_working && !inWork) {
		pr_debug("%s: short_check_worker is already being processed.\n",
								__func__);
		return;
	}

	if (short_det->irq_enable)
		return;

	short_det->irq_enable = true;
	enable_irq(short_det->irq_num);

	return;
}

void dsi_panel_driver_oled_short_det_disable(
		struct panel_specific_pdata *spec_pdata)
{
	struct short_detection_ctrl *short_det =
				&spec_pdata->short_det;

	if (spec_pdata == NULL || short_det == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return;
	}

	disable_irq(short_det->irq_num);
	short_det->irq_enable = false;

	return;
}

int dsi_panel_driver_oled_short_det_init_works(struct dsi_display *display)
{
	struct panel_specific_pdata *spec_pdata = display->panel->spec_pdata;
	struct short_detection_ctrl *short_det = &spec_pdata->short_det;
	int rc = 0;

	if (display == NULL || short_det == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	INIT_DELAYED_WORK(&short_det->check_work,
				dsi_panel_driver_oled_short_check_worker);

	short_det->current_chatter_cnt = 0;
	short_det->short_check_working = false;
	short_det->target_chatter_check_interval =
				SHORT_DEFAULT_TARGET_CHATTER_INTERVAL;

	if (!gpio_is_valid(spec_pdata->disp_err_fg_gpio)) {
		pr_err("%s: disp error flag gpio is invalid\n", __func__);
		return -EINVAL;
	}
	short_det->irq_num = gpio_to_irq(spec_pdata->disp_err_fg_gpio);

	rc = request_irq(short_det->irq_num,
			dsi_panel_driver_oled_short_det_handler,
			SHORT_IRQF_FLAGS, "disp_err_fg_gpio", display);
	if (rc) {
		pr_err("Failed to irq request rc=%d\n",	rc);
		return rc;
	}

	dsi_panel_driver_oled_short_det_disable(display->panel->spec_pdata);
	if (display->is_cont_splash_enabled)
		dsi_panel_driver_oled_short_det_enable(
			display->panel->spec_pdata, SHORT_WORKER_PASSIVE);

	return 0;
}

void dsi_panel_driver_oled_short_check_worker(struct work_struct *work)
{
	int rc = 0;
	struct delayed_work *dwork = to_delayed_work(work);
	struct short_detection_ctrl *short_det =
		container_of(dwork, struct short_detection_ctrl, check_work);
	struct panel_specific_pdata *spec_pdata =
		container_of(short_det, struct panel_specific_pdata, short_det);

	if (spec_pdata == NULL || short_det == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return;
	}

	if (!spec_pdata->display_onoff_state) {
		pr_err("%s: power status failed\n", __func__);
		return;
	}

	if (short_det->short_check_working) {
		pr_debug("%s: already status check\n", __func__);
		return;
	}
	short_det->short_check_working = true;

	dsi_panel_driver_oled_short_det_disable(spec_pdata);

	/* status check */
	rc = gpio_get_value(spec_pdata->disp_err_fg_gpio);
	if (rc)
		goto status_error;

	dsi_panel_driver_oled_short_det_enable(spec_pdata, SHORT_WORKER_ACTIVE);

	pr_debug("%s: short_check_worker done.\n", __func__);
	goto status_passed;

status_error:
	short_det->current_chatter_cnt++;
	pr_err("%s: Short Detection [%d]\n",
			__func__, short_det->current_chatter_cnt);
	if (short_det->current_chatter_cnt >=
			SHORT_DEFAULT_TARGET_CHATTER_CNT) {
		pr_err("%s: execute shutdown.\n", __func__);

		/* shutdown */
		do {
			pm_power_off();
			msleep(SHORT_POWER_OFF_RETRY_INTERVAL);
		} while (1);

		/* initialize */
		short_det->current_chatter_cnt = 0;
		short_det->short_check_working = false;
		return;
	}

status_passed:
	short_det->short_check_working = false;
	schedule_delayed_work(&short_det->check_work,
		msecs_to_jiffies(short_det->target_chatter_check_interval));
	return;
}

int somc_panel_init(struct dsi_display *display)
{
	struct panel_specific_pdata *spec_pdata = NULL;
	int rc = 0;

	if (display == NULL)
		return -EINVAL;
	if (display->panel == NULL)
		return -EINVAL;
	if (display->panel->spec_pdata == NULL)
		return -EINVAL;

	spec_pdata = display->panel->spec_pdata;

	rc = somc_panel_color_manager_init(display);
	if (rc)
		goto end;

	rc = somc_panel_fps_manager_init(display);
	if (rc)
		goto end;

	INIT_DELAYED_WORK(&display->panel->hbm_protect_work,
			  somc_panel_hbm_protect_work);
end:
	return rc;
}
