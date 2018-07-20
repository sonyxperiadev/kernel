/*
 * Copyright (c) 2012-2015, The Linux Foundation. All rights reserved.
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
 * Copyright (C) 2015 Sony Mobile Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 */
/* Unified somc_panel display driver implementation 
 * (C) 2016 AngeloGioacchino Del Regno <kholk11@gmail.com>
 */

#include <linux/fb.h>

#include <linux/delay.h>
#include <linux/string.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/device.h>
#include <linux/mdss_io_util.h>

#include "incell.h"
#include "../mdss_fb.h"
#include "../mdss_mdp.h"
#include "../mdss_dsi.h"
#include "somc_panels.h"
#include "panel_debugfs.h"

#define NODE_OF_HYBRID "/soc/dsi_panel_pwr_supply_hybrid_incell"
#define NODE_OF_FULL "/soc/dsi_panel_pwr_supply_full_incell"
#define MDSS_PINCTRL_STATE_TOUCH_ACTIVE  "mdss_touch_active"
#define MDSS_PINCTRL_STATE_TOUCH_SUSPEND "mdss_touch_suspend"

struct incell_ctrl *incell = NULL;
struct incell_ctrl incell_buf;
static bool sp_panel_forced;
static bool hybrid_incell;

static void incell_panel_power_worker_canceling(struct incell_ctrl *incell);

static int incell_parse_dt(struct device_node *np,
		struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	struct mdss_panel_specific_pdata *spec_pdata = NULL;
	struct device_node *panel_np;
	const char *panel_type_name;
	u32 tmp = 0;
	int rc = 0;

	spec_pdata = ctrl_pdata->spec_pdata;
	if (!spec_pdata) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	rc = of_property_read_u32(np,
			"somc,pw-wait-after-on-vddio", &tmp);
	spec_pdata->on_seq.disp_vddio = !rc ? tmp : 0;

	rc = of_property_read_u32(np,
			"somc,pw-wait-after-on-vsp", &tmp);
	spec_pdata->on_seq.disp_vsp = !rc ? tmp : 0;

	rc = of_property_read_u32(np,
			"somc,pw-wait-after-on-vsn", &tmp);
	spec_pdata->on_seq.disp_vsn = !rc ? tmp : 0;

	rc = of_property_read_u32(np,
			"somc,pw-wait-after-off-vddio", &tmp);
	spec_pdata->off_seq.disp_vddio = !rc ? tmp : 0;

	rc = of_property_read_u32(np,
			"somc,pw-wait-after-off-vsp", &tmp);
	spec_pdata->off_seq.disp_vsp = !rc ? tmp : 0;

	rc = of_property_read_u32(np,
			"somc,pw-wait-after-off-vsn", &tmp);
	spec_pdata->off_seq.disp_vsn = !rc ? tmp : 0;

	rc = of_property_read_u32(np,
			"somc,pw-wait-after-on-touch-avdd", &tmp);
	spec_pdata->on_seq.touch_avdd = !rc ? tmp : 0;

	rc = of_property_read_u32(np,
			"somc,pw-wait-after-on-touch-vddio", &tmp);
	spec_pdata->on_seq.touch_vddio = !rc ? tmp : 0;

	rc = of_property_read_u32(np,
			"somc,pw-wait-after-on-touch-reset", &tmp);
	spec_pdata->on_seq.touch_reset = !rc ? tmp : 0;

	rc = of_property_read_u32(np,
			"somc,pw-wait-after-on-touch-reset-first", &tmp);
	spec_pdata->on_seq.touch_reset_first = !rc ? tmp : 0;

	rc = of_property_read_u32(np,
			"somc,pw-wait-after-on-touch-int-n", &tmp);
	spec_pdata->on_seq.touch_intn = !rc ? tmp : 0;

	rc = of_property_read_u32(np,
			"somc,pw-wait-after-off-touch-avdd", &tmp);
	spec_pdata->off_seq.touch_avdd = !rc ? tmp : 0;

	rc = of_property_read_u32(np,
			"somc,pw-wait-after-off-touch-vddio", &tmp);
	spec_pdata->off_seq.touch_vddio = !rc ? tmp : 0;

	rc = of_property_read_u32(np,
			"somc,pw-wait-after-off-touch-reset", &tmp);
	spec_pdata->off_seq.touch_reset = !rc ? tmp : 0;

	rc = of_property_read_u32(np,
			"somc,pw-wait-after-off-touch-int-n", &tmp);
	spec_pdata->off_seq.touch_intn = !rc ? tmp : 0;

	rc = mdss_dsi_property_read_u32_var(np,
		"somc,ewu-rst-seq",
		(u32 **)&spec_pdata->ewu_seq.rst_seq,
		&spec_pdata->ewu_seq.seq_num);
	if (rc) {
		spec_pdata->ewu_seq.rst_seq = NULL;
		spec_pdata->ewu_seq.seq_num = 0;
		pr_debug("%s: Unable to read ewu sequence\n", __func__);
	}

	rc = of_property_read_u32(np,
		"somc,ewu-wait-after-touch-reset", &tmp);
	spec_pdata->ewu_seq.touch_reset = !rc ? tmp : 0;

	panel_np = of_parse_phandle(np, "qcom,panel-supply-entries", 0);

	panel_type_name = of_node_full_name(panel_np);
	if (!strcmp(panel_type_name, NODE_OF_HYBRID)) {
		spec_pdata->panel_type = HYBRID_INCELL;
		hybrid_incell = true;
	}
	if (!strcmp(panel_type_name, NODE_OF_FULL))
		spec_pdata->panel_type = FULL_INCELL;

	return 0;
}

static struct incell_ctrl *incell_get_info(void)
{
	return incell;
}

bool incell_panel_is_seq_for_ewu(void)
{
	struct incell_ctrl *incell = incell_get_info();

	if (incell &&
		(incell->seq == POWER_ON_EWU_SEQ))
		return true;

	return false;
}

static bool incell_panel_is_incell_operation(void)
{
	struct incell_ctrl *incell = incell_get_info();

	if (incell &&
		(incell->incell_intf_operation == INCELL_TOUCH_RUN))
		return true;

	return false;
}

bool incell_dsi_execute_lp11_init(void)
{
	struct incell_ctrl *incell = incell_get_info();

	return ((!incell) || (incell->seq == POWER_ON_EXECUTE));
}

static int somc_panel_calc_gpio_sleep(
		struct mdss_dsi_ctrl_pdata *ctrl_pdata,
		int gpio, bool enable)
{
	struct mdss_panel_specific_pdata *spec_pdata = ctrl_pdata->spec_pdata;
	struct mdss_panel_power_seq *pw_seq = NULL;
	int wait = 0;

	if (incell_panel_is_seq_for_ewu() &&
		(gpio == spec_pdata->touch_reset_gpio) &&
		!enable) {
		if (&spec_pdata->ewu_seq)
			pw_seq = &spec_pdata->ewu_seq;
		else
			pw_seq = &spec_pdata->on_seq;
	} else {
		if (enable)
			pw_seq = &spec_pdata->on_seq;
		else
			pw_seq = &spec_pdata->off_seq;
	}

	if (gpio == spec_pdata->touch_vddio_gpio)
		wait = pw_seq->touch_vddio;
	else if (gpio == spec_pdata->touch_reset_gpio)
		wait = pw_seq->touch_reset;
	else if (gpio == spec_pdata->touch_int_gpio)
		wait = pw_seq->touch_intn;
	else if (gpio == spec_pdata->disp_vddio_gpio)
		wait = pw_seq->disp_vddio;
	else
		wait = 0;

	wait = wait * 1000;
	return wait;
}

static void incell_panel_gpio_output(
		struct mdss_dsi_ctrl_pdata *ctrl_pdata,
		int gpio, bool enable, int value)
{
	int wait;

	wait = somc_panel_calc_gpio_sleep(ctrl_pdata, gpio, enable);

	if (gpio_is_valid(gpio))
		gpio_direction_output(gpio, value);

	if (wait)
		usleep_range(wait, wait + 100);
}

static void incell_panel_set_gpio(
		struct mdss_dsi_ctrl_pdata *ctrl_pdata,
		int gpio, bool enable, int value)
{
	int wait = 0;

	wait = somc_panel_calc_gpio_sleep(ctrl_pdata, gpio, enable);

	if (gpio_is_valid(gpio))
		gpio_set_value(gpio, value);

	if (wait)
		usleep_range(wait, wait + 100);
}

int incell_get_power_status(incell_pw_status *power_status)
{
	struct incell_ctrl *incell = incell_get_info();

	if (!incell) {
		pr_err("%s: Invalid incell data\n", __func__);
		return INCELL_ERROR;
	}

	pr_debug("%s: status = %d\n", __func__, (int)(incell->state));

	switch (incell->state) {
	case INCELL_STATE_SLE000_P1:
	case INCELL_STATE_SLE001_P1:
	case INCELL_STATE_SLE010_P1:
	case INCELL_STATE_SLE011_P1:
	case INCELL_STATE_SLE100_P1:
	case INCELL_STATE_SLE101_P1:
	case INCELL_STATE_SLE110_P1:
	case INCELL_STATE_SLE111_P1:
		power_status->display_power = INCELL_POWER_ON;
		power_status->touch_power = INCELL_POWER_ON;
		break;
	case INCELL_STATE_SLE000_P0:
	case INCELL_STATE_SLE001_P0:
	case INCELL_STATE_SLE010_P0:
	case INCELL_STATE_SLE011_P0:
	case INCELL_STATE_SLE100_P0:
	case INCELL_STATE_SLE101_P0:
	case INCELL_STATE_SLE110_P0:
	case INCELL_STATE_SLE111_P0:
	default:
		power_status->display_power = INCELL_POWER_OFF;
		power_status->touch_power = INCELL_POWER_OFF;
		break;
	}

	return INCELL_OK;
}

static int incell_reset_touch(struct mdss_panel_data *pdata, int enable)
{
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct mdss_panel_info *pinfo = NULL;
	struct mdss_panel_specific_pdata *spec_pdata = NULL;
	int rc = 0;

	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

	spec_pdata = ctrl_pdata->spec_pdata;

	if (!gpio_is_valid(spec_pdata->touch_reset_gpio)) {
		pr_debug("%s:%d, touch reset line not configured\n",
			   __func__, __LINE__);
		return rc;
	}

	pr_debug("%s: enable = %d\n", __func__, enable);
	pinfo = &(ctrl_pdata->panel_data.panel_info);

	if (enable) {
		if (!spec_pdata->gpios_requested) {
			rc = mdss_dsi_request_gpios(ctrl_pdata);
			if (rc) {
				pr_err("gpio request failed\n");
				return rc;
			}
			spec_pdata->gpios_requested = true;
		}

		incell_panel_gpio_output(ctrl_pdata,
			(spec_pdata->touch_reset_gpio), true, 1);
	} else {
		incell_panel_set_gpio(ctrl_pdata,
			(spec_pdata->touch_reset_gpio), false, 0);
	}

	return rc;
}

bool incell_state_is_power_on(incell_state state)
{
	bool ret = false;

	switch (state) {
	case INCELL_STATE_SLE000_P1:
	case INCELL_STATE_SLE001_P1:
	case INCELL_STATE_SLE010_P1:
	case INCELL_STATE_SLE011_P1:
	case INCELL_STATE_SLE100_P1:
	case INCELL_STATE_SLE101_P1:
	case INCELL_STATE_SLE110_P1:
	case INCELL_STATE_SLE111_P1:
		pr_debug("%s: In-Cell on state\n", __func__);
		ret = true;
		break;
	default:
		pr_debug("%s: In-Cell off state\n", __func__);
		break;
	}
	return ret;
}

bool incell_state_is_power_locked(incell_state state)
{
	bool ret = false;

	switch (state) {
	case INCELL_STATE_SLE010_P0:
	case INCELL_STATE_SLE010_P1:
	case INCELL_STATE_SLE011_P0:
	case INCELL_STATE_SLE011_P1:
	case INCELL_STATE_SLE110_P0:
	case INCELL_STATE_SLE110_P1:
	case INCELL_STATE_SLE111_P0:
	case INCELL_STATE_SLE111_P1:
		pr_debug("%s: In-Cell I/F Lock state\n", __func__);
		ret = true;
		break;
	default:
		pr_debug("%s: In-Cell I/F Unlock state\n", __func__);
		break;
	}
	return ret;
}

bool incell_state_is_ewu(incell_state state)
{
	bool ret = false;

	switch (state) {
	case INCELL_STATE_SLE001_P0:
	case INCELL_STATE_SLE001_P1:
	case INCELL_STATE_SLE011_P0:
	case INCELL_STATE_SLE011_P1:
	case INCELL_STATE_SLE101_P0:
	case INCELL_STATE_SLE101_P1:
	case INCELL_STATE_SLE111_P0:
	case INCELL_STATE_SLE111_P1:
		pr_debug("%s: In-Cell I/F EWU state\n", __func__);
		ret = true;
		break;
	default:
		pr_debug("%s: In-Cell I/F NORMAL state\n", __func__);
		break;
	}
	return ret;
}

static void incell_power_off_ctrl(struct incell_ctrl *incell)
{
	incell_pw_seq seq = POWER_OFF_EXECUTE;
	incell_state_change change_state = INCELL_STATE_NONE;
	incell_state state = incell->state;
	incell_intf_mode intf_mode = incell->intf_mode;
	incell_worker_state worker_state = incell->worker_state;
	bool incell_intf_operation = incell->incell_intf_operation;

	if (worker_state == INCELL_WORKER_ON) {
		change_state = INCELL_STATE_P_OFF;
	} else if (incell_intf_operation == INCELL_TOUCH_RUN) {
		/* touch I/F running mode */
		if (intf_mode == INCELL_DISPLAY_HW_RESET) {
			if (!incell_state_is_power_on(state)) {
				pr_err("%s: Already power off. state:%d\n",
						__func__, (int)state);
				seq = POWER_OFF_SKIP;
			} else {
				change_state = INCELL_STATE_P_OFF;
			}
		} else {
			if (!incell_state_is_power_on(state)) {
				pr_err("%s: Power off status. state:%d\n",
						__func__, (int)state);
				seq = POWER_OFF_SKIP;
			} else if (incell_state_is_ewu(state)) {
				pr_debug("%s: Skip power off for EWU seq\n",
						__func__);
				seq = POWER_OFF_SKIP;
			} else {
				change_state = INCELL_STATE_P_OFF;
			}
		}
	} else {
		if (worker_state == INCELL_WORKER_PENDING)
			incell_panel_power_worker_canceling(incell);

		/* touch I/F idling mode */
		if (incell_state_is_power_locked(state)) {
			change_state = INCELL_STATE_S_OFF;
			seq = POWER_OFF_SKIP;
		} else if (!incell_state_is_power_on(state)) {
			change_state = INCELL_STATE_S_OFF;
			seq = POWER_OFF_SKIP;
		} else if (incell_state_is_ewu(state)) {
			change_state = INCELL_STATE_S_OFF;
			seq = POWER_OFF_SKIP;
		} else {
			change_state = INCELL_STATE_SP_OFF;
		}
	}

	pr_debug("%s: incell change state seq:%d change_state:%d\n",
				__func__, (int)seq, (int)change_state);
	incell->seq = seq;
	incell->change_state = change_state;
}

static inline void incell_state_s_off(incell_state *state)
{
	switch (*state) {
	case INCELL_STATE_SLE100_P0:
		*state = INCELL_STATE_SLE000_P0;
		break;
	case INCELL_STATE_SLE100_P1:
		*state = INCELL_STATE_SLE000_P1;
		break;
	case INCELL_STATE_SLE101_P0:
		*state = INCELL_STATE_SLE001_P0;
		break;
	case INCELL_STATE_SLE101_P1:
		*state = INCELL_STATE_SLE001_P1;
		break;
	case INCELL_STATE_SLE110_P0:
		*state = INCELL_STATE_SLE010_P0;
		break;
	case INCELL_STATE_SLE110_P1:
		*state = INCELL_STATE_SLE010_P1;
		break;
	case INCELL_STATE_SLE111_P0:
		*state = INCELL_STATE_SLE011_P0;
		break;
	case INCELL_STATE_SLE111_P1:
		*state = INCELL_STATE_SLE011_P1;
		break;
	default:
		pr_err("%s: In-Cell I/F state no update\n", __func__);
		break;
	}
}

static inline void incell_state_p_off(incell_state *state)
{
	switch (*state) {
	case INCELL_STATE_SLE000_P1:
		*state = INCELL_STATE_SLE000_P0;
		break;
	case INCELL_STATE_SLE001_P1:
		*state = INCELL_STATE_SLE001_P0;
		break;
	case INCELL_STATE_SLE010_P1:
		*state = INCELL_STATE_SLE010_P0;
		break;
	case INCELL_STATE_SLE011_P1:
		*state = INCELL_STATE_SLE011_P0;
		break;
	case INCELL_STATE_SLE100_P1:
		*state = INCELL_STATE_SLE100_P0;
		break;
	case INCELL_STATE_SLE101_P1:
		*state = INCELL_STATE_SLE101_P0;
		break;
	case INCELL_STATE_SLE110_P1:
		*state = INCELL_STATE_SLE110_P0;
		break;
	case INCELL_STATE_SLE111_P1:
		*state = INCELL_STATE_SLE111_P0;
		break;
	default:
		pr_err("%s: In-Cell I/F state no update\n", __func__);
		break;
	}
}

static inline void incell_state_sp_off(incell_state *state)
{
	switch (*state) {
	case INCELL_STATE_SLE000_P1:
	case INCELL_STATE_SLE100_P1:
	case INCELL_STATE_SLE100_P0:
		*state = INCELL_STATE_SLE000_P0;
		break;
	case INCELL_STATE_SLE101_P0:
	case INCELL_STATE_SLE101_P1:
	case INCELL_STATE_SLE001_P1:
		*state = INCELL_STATE_SLE001_P0;
		break;
	case INCELL_STATE_SLE110_P0:
	case INCELL_STATE_SLE110_P1:
	case INCELL_STATE_SLE010_P1:
		*state = INCELL_STATE_SLE010_P0;
		break;
	case INCELL_STATE_SLE111_P0:
	case INCELL_STATE_SLE111_P1:
	case INCELL_STATE_SLE011_P1:
		*state = INCELL_STATE_SLE011_P0;
		break;
	default:
		pr_err("%s: In-Cell I/F state no update\n", __func__);
		break;
	}
}

void incell_state_change_off(struct incell_ctrl *incell)
{
	incell_state_change change_state = incell->change_state;
	incell_state *state = &incell->state;

	pr_debug("%s: status:%d --->\n", __func__, ((int)(*state)));

	switch (change_state) {
	case INCELL_STATE_NONE:
		pr_debug("%s: Not change off status\n", __func__);
		break;
	case INCELL_STATE_S_OFF:
		incell_state_s_off(state);
		break;
	case INCELL_STATE_P_OFF:
		incell_state_p_off(state);
		break;
	case INCELL_STATE_SP_OFF:
		incell_state_sp_off(state);
		break;
	default:
		pr_err("%s: offmode unknown\n", __func__);
		break;
	}

	sp_panel_forced = false;
	pr_debug("%s: ---> status:%d\n", __func__, ((int)(*state)));
}

static inline void incell_state_s_on(incell_state *state)
{
	switch (*state) {
	case INCELL_STATE_SLE000_P0:
		*state = INCELL_STATE_SLE100_P0;
		break;
	case INCELL_STATE_SLE000_P1:
		*state = INCELL_STATE_SLE100_P1;
		break;
	case INCELL_STATE_SLE001_P0:
		*state = INCELL_STATE_SLE101_P0;
		break;
	case INCELL_STATE_SLE001_P1:
		*state = INCELL_STATE_SLE101_P1;
		break;
	case INCELL_STATE_SLE010_P0:
		*state = INCELL_STATE_SLE110_P0;
		break;
	case INCELL_STATE_SLE010_P1:
		*state = INCELL_STATE_SLE110_P1;
		break;
	case INCELL_STATE_SLE011_P0:
		*state = INCELL_STATE_SLE111_P0;
		break;
	case INCELL_STATE_SLE011_P1:
		*state = INCELL_STATE_SLE111_P1;
		break;
	default:
		pr_err("%s: In-Cell I/F state no update\n", __func__);
		break;
	}
}

static inline void incell_state_p_on(incell_state *state)
{
	switch (*state) {
	case INCELL_STATE_SLE000_P0:
		*state = INCELL_STATE_SLE000_P1;
		break;
	case INCELL_STATE_SLE001_P0:
		*state = INCELL_STATE_SLE001_P1;
		break;
	case INCELL_STATE_SLE010_P0:
		*state = INCELL_STATE_SLE010_P1;
		break;
	case INCELL_STATE_SLE011_P0:
		*state = INCELL_STATE_SLE011_P1;
		break;
	case INCELL_STATE_SLE100_P0:
		*state = INCELL_STATE_SLE100_P1;
		break;
	case INCELL_STATE_SLE101_P0:
		*state = INCELL_STATE_SLE101_P1;
		break;
	case INCELL_STATE_SLE110_P0:
		*state = INCELL_STATE_SLE110_P1;
		break;
	case INCELL_STATE_SLE111_P0:
		*state = INCELL_STATE_SLE111_P1;
		break;
	default:
		pr_err("%s: In-Cell I/F state no update\n", __func__);
		break;
	}
}

static inline void incell_state_sp_on(incell_state *state)
{
	switch (*state) {
	case INCELL_STATE_SLE000_P0:
	case INCELL_STATE_SLE000_P1:
	case INCELL_STATE_SLE100_P0:
		*state = INCELL_STATE_SLE100_P1;
		break;
	case INCELL_STATE_SLE001_P0:
	case INCELL_STATE_SLE001_P1:
	case INCELL_STATE_SLE101_P0:
		*state = INCELL_STATE_SLE101_P1;
		break;
	case INCELL_STATE_SLE010_P0:
	case INCELL_STATE_SLE010_P1:
	case INCELL_STATE_SLE110_P0:
		*state = INCELL_STATE_SLE110_P1;
		break;
	case INCELL_STATE_SLE011_P0:
	case INCELL_STATE_SLE011_P1:
	case INCELL_STATE_SLE111_P0:
		*state = INCELL_STATE_SLE111_P1;
		break;
	default:
		pr_err("%s: In-Cell I/F state no update\n", __func__);
		break;
	}
}

void incell_state_change_on(struct incell_ctrl *incell)
{
	incell_state_change change_state = incell->change_state;
	incell_state *state = &incell->state;

	pr_debug("%s: status:%d --->\n", __func__, ((int)(*state)));

	switch (change_state) {
	case INCELL_STATE_NONE:
		pr_debug("%s: Not change on status\n", __func__);
		break;
	case INCELL_STATE_S_ON:
		incell_state_s_on(state);
		break;
	case INCELL_STATE_P_ON:
		incell_state_p_on(state);
		break;
	case INCELL_STATE_SP_ON:
		incell_state_sp_on(state);
		break;
	default:
		pr_err("%s: onmode unknown\n", __func__);
		break;
	}

	pr_debug("%s: ---> status:%d\n", __func__, ((int)(*state)));
}

static int incell_touch_pinctrl_set_state(
	struct mdss_dsi_ctrl_pdata *ctrl_pdata,
	bool active)
{
	struct mdss_panel_specific_pdata *spec_pdata = NULL;
	struct pinctrl_state *pin_state;
	int rc = -EFAULT;
	int wait = 0;

	if (IS_ERR_OR_NULL(ctrl_pdata->pin_res.pinctrl))
		return PTR_ERR(ctrl_pdata->pin_res.pinctrl);

	spec_pdata = ctrl_pdata->spec_pdata;

	pin_state = active ? ctrl_pdata->pin_res.touch_state_active
				: ctrl_pdata->pin_res.touch_state_suspend;

	if (!IS_ERR_OR_NULL(pin_state)) {
		rc = pinctrl_select_state(ctrl_pdata->pin_res.pinctrl,
				pin_state);
		if (!rc) {
			wait = somc_panel_calc_gpio_sleep(ctrl_pdata,
					spec_pdata->touch_int_gpio, active);
			if (wait)
				usleep_range(wait, wait + 100);
		} else {
			pr_err("%s: can not set %s pins\n", __func__,
			       active ? MDSS_PINCTRL_STATE_TOUCH_ACTIVE
			       : MDSS_PINCTRL_STATE_TOUCH_SUSPEND);
		}
	} else {
		pr_err("%s: invalid '%s' pinstate\n", __func__,
		       active ? MDSS_PINCTRL_STATE_TOUCH_ACTIVE
		       : MDSS_PINCTRL_STATE_TOUCH_SUSPEND);
	}

	return rc;
}

static int incell_dsi_panel_power_off_ex(struct mdss_panel_data *pdata)
{
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct mdss_panel_specific_pdata *spec_pdata = NULL;
	struct mdss_panel_power_seq *pw_seq = NULL;
	struct incell_ctrl *incell = incell_get_info();
	int ret = 0;
	int rc = 0;

	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		ret = -EINVAL;
		goto end;
	}

	if (incell)
		if (incell->seq == POWER_OFF_SKIP)
			return ret;

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

	spec_pdata = ctrl_pdata->spec_pdata;
	pw_seq = &spec_pdata->off_seq;

	if (spec_pdata->off_seq.rst_b_seq) {
		rc = mdss_dsi_panel_reset(pdata, 0);
		if (rc)
			pr_warn("%s: Panel reset failed. rc=%d\n",
					__func__, rc);
	}

	ret += incell_reset_touch(pdata, 0);
	ret += somc_panel_vreg_ctrl(ctrl_pdata, "ibb", false);
	ret += somc_panel_vreg_ctrl(ctrl_pdata, "lab", false);

	if (!spec_pdata->off_seq.rst_b_seq) {
		rc = mdss_dsi_panel_reset(pdata, 0);
		if (rc)
			pr_warn("%s: Panel reset failed. rc=%d\n",
					__func__, rc);
	}

	incell_touch_pinctrl_set_state(ctrl_pdata, false);

	incell_panel_set_gpio(ctrl_pdata,
		(spec_pdata->touch_vddio_gpio), false, 1);

	incell_panel_set_gpio(ctrl_pdata,
		(spec_pdata->disp_vddio_gpio), false, 1);

	ret += somc_panel_vreg_ctrl(ctrl_pdata, "vddio", false);
	ret += somc_panel_vreg_ctrl(ctrl_pdata, "touch-avdd", false);
	ret += somc_panel_vreg_ctrl(ctrl_pdata, "vdd", false);

	if (ret)
		pr_err("%s: failed to disable vregs for %s\n",
				__func__, __mdss_dsi_pm_name(DSI_PANEL_PM));
	else
		pr_notice("@@@@ panel power off @@@@\n");

	if (mdss_dsi_pinctrl_set_state(ctrl_pdata, false))
		pr_debug("reset disable: pinctrl not enabled\n");

	if (spec_pdata->down_period)
		spec_pdata->current_period = (u32)ktime_to_ms(ktime_get());

end:
	return ret;
}

static void incell_do_power_off(void)
{
	struct incell_ctrl *incell = incell_get_info();

	if (!incell)
		return;

	incell_power_off_ctrl(incell);
	incell_state_change_off(incell);
}

static int incell_dsi_panel_off_ex(struct mdss_panel_data *pdata)
{
	incell_do_power_off();
	return 0;
}

static int incell_driver_power_off(struct fb_info *info)
{
	int ret = INCELL_ERROR;
	struct mdss_data_type *mdata = mdss_mdp_get_mdata();
	struct mdss_mdp_ctl *ctl = NULL;
	struct mdss_panel_data *pdata = NULL;
	struct msm_fb_data_type *mfd = NULL;
	struct incell_ctrl *incell = incell_get_info();

	if (!incell) {
		pr_err("%s: Invalid incell data\n", __func__);
		return ret;
	}

	if (!mdata) {
		pr_err("%s: Invalid mdata\n", __func__);
		return ret;
	}

	ctl = mdata->ctl_off;
	if (!ctl) {
		pr_err("%s: Invalid ctl data\n", __func__);
		return ret;
	}

	pdata = ctl->panel_data;
	if (!pdata) {
		pr_err("%s: Invalid panel data\n", __func__);
		return ret;
	}

	mfd = (struct msm_fb_data_type *)info->par;
	if (!mfd) {
		pr_err("%s: Invalid msm data\n", __func__);
		return ret;
	}

	if (!mdss_fb_is_power_on(mfd)) {
		incell_do_power_off();

		ret = incell_dsi_panel_power_off_ex(pdata);
		if (ret) {
			pr_err("%s: Failed to power off ret=%d\n",
					__func__, ret);
			ret = INCELL_ERROR;
			return ret;
		}
	} else {
		ret = info->fbops->fb_blank(FB_BLANK_POWERDOWN, info);
		if (ret) {
			pr_err("%s: fb_blank(blank) FAIL ret=%d\n",
					__func__, ret);
			ret = INCELL_ERROR;
			return ret;
		}

		ret = info->fbops->fb_release(info, 0);
		if (ret) {
			pr_err("%s: Cannot release fb ret=%d\n",
					__func__, ret);
			ret = INCELL_ERROR;
			return ret;
		}
	}

	ret = INCELL_OK;
	return ret;
}

static int incell_driver_send_power_on_fb(struct fb_info *info)
{
	int ret = INCELL_ERROR;

	if (!(info->fbops->fb_open) || !(info->fbops->fb_blank)
		|| !(info->fbops->fb_release)) {
		pr_err("%s: Invalid operations\n", __func__);
		goto end;
	}

	ret = info->fbops->fb_open(info, 0);
	if (ret) {
		pr_err("%s: Cannot open fb ret=%d\n", __func__, ret);
		ret = INCELL_ERROR;
		goto end;
	}

	ret = info->fbops->fb_blank(FB_BLANK_UNBLANK, info);
	if (ret) {
		pr_err("%s: fb_blank(blank) FAIL ret=%d\n",
				__func__, ret);
		info->fbops->fb_release(info, 0);
		ret = INCELL_ERROR;
		goto end;
	}
	ret = INCELL_OK;
end:
	return ret;
}

void incell_power_on_ctrl(struct incell_ctrl *incell)
{
	incell_pw_seq seq = POWER_ON_EXECUTE;
	incell_state_change change_state = INCELL_STATE_NONE;
	incell_state state = incell->state;
	incell_intf_mode intf_mode = incell->intf_mode;
	incell_worker_state worker_state = incell->worker_state;
	bool incell_intf_operation = incell->incell_intf_operation;

	if (worker_state == INCELL_WORKER_ON) {
		change_state = INCELL_STATE_P_ON;
	} else if (incell_intf_operation == INCELL_TOUCH_RUN) {
		/* touch I/F running mode */
		if (intf_mode != INCELL_DISPLAY_HW_RESET) {
			pr_err("%s: Unknown I/F: %d\n",
					__func__, (int)intf_mode);
			seq = POWER_ON_SKIP;
		} else if (incell_state_is_power_on(state)) {
			pr_err("%s: Already power on status. state:%d\n",
					__func__, (int)state);
			seq = POWER_ON_SKIP;
		} else {
			change_state = INCELL_STATE_P_ON;
		}
	} else {
		/* touch I/F idling mode */
		if (worker_state == INCELL_WORKER_PENDING) {
			incell_panel_power_worker_canceling(incell);
			change_state = INCELL_STATE_S_ON;
			seq = POWER_ON_EWU_SEQ;
		} else if (incell_state_is_ewu(state)) {
			if (incell_state_is_power_on(state)) {
				change_state = INCELL_STATE_S_ON;
				seq = POWER_ON_EWU_SEQ;
			} else {
				change_state = INCELL_STATE_SP_ON;
				seq = POWER_ON_EXECUTE;
			}
		} else if (incell_state_is_power_on(state)) {
			change_state = INCELL_STATE_S_ON;
			seq = POWER_ON_EWU_SEQ;
		} else {
			change_state = INCELL_STATE_SP_ON;
			seq = POWER_ON_EXECUTE;
		}
	}

	pr_debug("%s: incell change state seq:%d change_state:%d\n",
				__func__, (int)seq, (int)change_state);
	incell->seq = seq;
	incell->change_state = change_state;
}

int mdss_dsi_reset_dual_display(struct mdss_dsi_ctrl_pdata *ctrl_pdata);

static int incell_dsi_panel_power_on_ex(struct mdss_panel_data *pdata)
{
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct mdss_panel_specific_pdata *spec_pdata = NULL;
	struct mdss_panel_power_seq *pw_seq = NULL;
	struct incell_ctrl *incell = incell_get_info();
	incell_state state = INCELL_STATE_NONE;
	int ret = 0;
	int wait;

	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}
	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

	if (incell) {
		incell_power_on_ctrl(incell);
		if (incell->seq != POWER_ON_EXECUTE) {
			if (incell->seq == POWER_ON_EWU_SEQ) {
				ret = 0;
				ret += incell_reset_touch(pdata, 0);
				ret += mdss_dsi_reset_dual_display(ctrl_pdata);
			}
			return ret;
		}
		state = incell->state;
	}



	spec_pdata = ctrl_pdata->spec_pdata;
	pw_seq = &spec_pdata->on_seq;

	if (!spec_pdata->gpios_requested) {
		ret = mdss_dsi_request_gpios(ctrl_pdata);
		if (ret) {
			pr_err("gpio request failed\n");
			return ret;
		}
		spec_pdata->gpios_requested = true;
	}

	somc_panel_down_period_quirk(spec_pdata);

	ret += somc_panel_vreg_ctrl(ctrl_pdata, "vdd", true);
	ret += somc_panel_vreg_ctrl(ctrl_pdata, "touch-avdd", true);
	ret += somc_panel_vreg_ctrl(ctrl_pdata, "vddio", true);

	incell_panel_gpio_output(ctrl_pdata,
		(spec_pdata->disp_vddio_gpio), true, 0);

	incell_panel_gpio_output(ctrl_pdata,
		(spec_pdata->touch_vddio_gpio), true, 0);

	incell_touch_pinctrl_set_state(ctrl_pdata, true);

	if ((spec_pdata->panel_type == HYBRID_INCELL) &&
	    (!incell_state_is_power_on(state))) {
		ret += incell_reset_touch(pdata, 1);
		wait = pw_seq->touch_reset_first;
		usleep_range(wait * 1000, wait * 1000 + 100);
	}
	ret += somc_panel_vreg_ctrl(ctrl_pdata, "lab", true);
	ret += somc_panel_vreg_ctrl(ctrl_pdata, "ibb", true);

	if (ret) {
		pr_err("%s: failed to enable vregs for %s\n",
			__func__, __mdss_dsi_pm_name(DSI_PANEL_PM));
		return ret;
	}

	pr_notice("@@@@ panel power on @@@@\n");

	/*
	 * If continuous splash screen feature is enabled, then we need to
	 * request all the GPIOs that have already been configured in the
	 * bootloader. This needs to be done irresepective of whether
	 * the lp11_init flag is set or not.
	 */
	if (!pdata->panel_info.cont_splash_enabled &&
		!pdata->panel_info.mipi.lp11_init) {
		if (mdss_dsi_pinctrl_set_state(ctrl_pdata, true))
			pr_debug("reset enable: pinctrl not enabled\n");

		ret = mdss_dsi_panel_reset(pdata, 1);
		if (ret)
			pr_err("%s: Panel reset failed. rc=%d\n",
					__func__, ret);
	}

	return ret;
}

static int incell_driver_send_power_on_seq(struct mdss_panel_data *pdata)
{
	int ret = INCELL_ERROR;
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct mdss_panel_specific_pdata *spec_pdata = NULL;

	ctrl_pdata = container_of(pdata,
			struct mdss_dsi_ctrl_pdata, panel_data);
	spec_pdata = ctrl_pdata->spec_pdata;

	if (!ctrl_pdata || !spec_pdata) {
		pr_err("%s: Invalid input data\n", __func__);
		goto end;
	}

	ret = incell_dsi_panel_power_on_ex(pdata);
	if (ret) {
		pr_err("%s: Failed to power on ret=%d\n", __func__, ret);
		ret = INCELL_ERROR;
		goto end;
	}

	if (mdss_dsi_pinctrl_set_state(ctrl_pdata, true))
		pr_debug("%s: reset enable: pinctrl not enabled\n",
								__func__);
	mdss_dsi_panel_reset(pdata, 1);

	if (incell->incell_intf_operation == INCELL_TOUCH_RUN &&
		incell->intf_mode == INCELL_DISPLAY_HW_RESET) {
		incell_reset_touch(pdata, 1);
		incell_state_change_on(incell);
	} else {
		if (!ctrl_pdata->on)
			goto end;

		ret = ctrl_pdata->on(pdata);
		if (ret) {
			pr_err("%s: Failed to send on ret=%d\n",
				__func__, ret);
			ret = INCELL_ERROR;
			goto end;
		}

		if (!ctrl_pdata->post_panel_on)
			goto end;

		ret = ctrl_pdata->post_panel_on(pdata);
		if (ret) {
			pr_err("%s: Failed to send post-on ret=%d\n",
				__func__, ret);
			ret = INCELL_ERROR;
			goto end;
		}
	}
	ret = INCELL_OK;
end:
	return ret;
}

static int incell_driver_power_on(struct fb_info *info)
{
	int ret = INCELL_ERROR;
	struct mdss_data_type *mdata = mdss_mdp_get_mdata();
	struct mdss_mdp_ctl *ctl = NULL;
	struct mdss_panel_data *pdata = NULL;
	struct msm_fb_data_type *mfd = NULL;

	if (!mdata) {
		pr_err("%s: Invalid mdata\n", __func__);
		goto end;
	}

	ctl = mdata->ctl_off;
	if (!ctl) {
		pr_err("%s: Invalid ctl data\n", __func__);
		goto end;
	}

	pdata = ctl->panel_data;
	if (!pdata) {
		pr_err("%s: Invalid panel data\n", __func__);
		goto end;
	}

	mfd = (struct msm_fb_data_type *)info->par;
	if (!mfd) {
		pr_err("%s: Invalid msm data\n", __func__);
		goto end;
	}

	if (mdss_fb_is_power_on(mfd))
		ret = incell_driver_send_power_on_seq(pdata);
	else
		ret = incell_driver_send_power_on_fb(info);

	mfd->off_sts = false;

end:
	return ret;
}

int incell_driver_post_power_on(struct mdss_panel_data *pdata)
{
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct mdss_panel_specific_pdata *spec_pdata = NULL;
	struct incell_ctrl *incell = incell_get_info();
	incell_state state;
	int ret = 1;

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

	spec_pdata = ctrl_pdata->spec_pdata;
	if (!spec_pdata) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	if (incell && spec_pdata->panel_type == HYBRID_INCELL) {
		state = incell->state;
		if (!incell_state_is_power_on(state) &&
		    !incell_state_is_ewu(state))
			incell_reset_touch(pdata, 0);
	}
	incell_reset_touch(pdata, 1);

	if (incell)
		incell_state_change_on(incell);

	spec_pdata->disp_onoff_state = true;

	return ret;
}

void incell_force_sp_on(void)
{
	struct incell_ctrl *incell = incell_get_info();

	incell->change_state = INCELL_STATE_SP_ON;

	if (incell)
		incell_state_change_on(incell);
	return;
}

static int incell_display_hw_reset(struct incell_ctrl *incell,
				struct fb_info *info)
{
	int ret = INCELL_ERROR;
	struct mdss_data_type *mdata = mdss_mdp_get_mdata();
	struct mdss_mdp_ctl *ctl = NULL;
	struct mdss_panel_data *pdata = NULL;

	if (!incell) {
		pr_err("%s: Invalid incell data\n", __func__);
		goto end;
	}

	if (!mdata) {
		pr_err("%s: Invalid mdata\n", __func__);
		goto end;
	}

	ctl = mdata->ctl_off;
	if (!ctl) {
		pr_err("%s: Invalid ctl data\n", __func__);
		goto end;
	}

	pdata = ctl->panel_data;
	if (!pdata) {
		pr_err("%s: Invalid panel data\n", __func__);
		goto end;
	}

	/* Power off if LCD is on. */
	switch (incell->state) {
	case INCELL_STATE_SLE000_P1:
	case INCELL_STATE_SLE001_P1:
	case INCELL_STATE_SLE010_P1:
	case INCELL_STATE_SLE011_P1:
	case INCELL_STATE_SLE100_P1:
	case INCELL_STATE_SLE101_P1:
	case INCELL_STATE_SLE110_P1:
	case INCELL_STATE_SLE111_P1:
		/*
		 * It calls directly power off to DSI layer,
		 * the case of FB off.
		 */
		ret = incell_driver_power_off(info);
		break;
	default:
		pr_debug("%s: Skip LCD off %d\n", __func__,
					((int)(incell->state)));
		break;
	}

	switch (incell->state) {
	case INCELL_STATE_SLE000_P0:
	case INCELL_STATE_SLE001_P0:
	case INCELL_STATE_SLE010_P0:
	case INCELL_STATE_SLE011_P0:
		pr_debug("%s: LCD on in DSI layer. sts:%d\n", __func__,
					((int)(incell->state)));
		ret = incell_driver_send_power_on_seq(pdata);
		break;
	default:
		pr_debug("%s: LCD on in FB layer. sts:%d\n", __func__,
					((int)(incell->state)));
		ret = incell_driver_send_power_on_fb(info);
		break;
	}

	ret = INCELL_OK;
end:
	return ret;
}

static int incell_display_off(struct incell_ctrl *incell,
				struct fb_info *info)
{
	int ret = INCELL_ERROR;

	switch (incell->state) {
	case INCELL_STATE_SLE000_P0:
	case INCELL_STATE_SLE001_P0:
	case INCELL_STATE_SLE010_P0:
	case INCELL_STATE_SLE011_P0:
	case INCELL_STATE_SLE100_P0:
	case INCELL_STATE_SLE101_P0:
	case INCELL_STATE_SLE110_P0:
	case INCELL_STATE_SLE111_P0:
		pr_err("%s: LCD is already off. sts:%d\n",
			__func__, ((int)(incell->state)));
		ret = INCELL_EALREADY;
		break;
	default:
		pr_debug("%s: incell panel sts:%d\n", __func__,
						((int)(incell->state)));
		break;
	}

	if (ret == INCELL_EALREADY) {
		pr_err("%s: Already power off ret=%d\n", __func__, ret);
		return ret;
	}

	ret = incell_driver_power_off(info);
	return ret;
}

int incell_control_mode(incell_intf_mode mode, bool force)
{
	int ret = INCELL_ERROR;
	struct fb_info *info = NULL;
	struct msm_fb_data_type *mfd = NULL;
	struct mdss_data_type *mdata = mdss_mdp_get_mdata();
	struct mdss_mdp_ctl *ctl = NULL;
	struct mdss_panel_data *pdata = NULL;
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct mdss_panel_specific_pdata *spec_pdata = NULL;
	struct incell_ctrl *incell = incell_get_info();

	if (!incell) {
		pr_err("%s: Invalid incell data\n", __func__);
		return ret;
	}

	info = registered_fb[0];
	if (!info) {
		pr_err("%s: Invalid fb data\n", __func__);
		return ret;
	}

	if (!(info->fbops->fb_blank) || !(info->fbops->fb_release)
		|| !(info->fbops->fb_open)) {
		pr_err("%s: Invalid operations\n", __func__);
		return ret;
	}

	if (!mdata) {
		pr_err("%s: Invalid mdata\n", __func__);
		return ret;
	}

	ctl = mdata->ctl_off;
	if (!ctl) {
		pr_err("%s: Invalid ctl data\n", __func__);
		return ret;
	}

	pdata = ctl->panel_data;
	if (!pdata) {
		pr_err("%s: Invalid panel data\n", __func__);
		return ret;
	}

	mfd = (struct msm_fb_data_type *)info->par;
	if (!mfd) {
		pr_err("%s: Invalid msm data\n", __func__);
		return ret;
	}

	ctrl_pdata = container_of(pdata,
				struct mdss_dsi_ctrl_pdata, panel_data);
	spec_pdata = ctrl_pdata->spec_pdata;

	if (!ctrl_pdata || !spec_pdata) {
		pr_err("%s: Invalid input data\n", __func__);
		return ret;
	}

	/*
	 * It returns "INCELL_ALREADY_LOCKED"
	 * the case of not setting "INCELL_FORCE" flag.
	 */
	if (incell_state_is_power_locked(incell->state)) {
		if (force == INCELL_UNFORCE) {
			ret = INCELL_ALREADY_LOCKED;
			pr_err("%s: Already power locked ret=%d\n",
							__func__, ret);
			return ret;
		}
	}

	if (incell->worker_state != INCELL_WORKER_OFF) {
		ret = INCELL_EBUSY;
		pr_err("%s: worker scheduling ret=%d\n", __func__, ret);
		return ret;
	}

	if (incell->incell_intf_operation == INCELL_TOUCH_RUN) {
		ret = INCELL_EBUSY;
		pr_err("%s: touch I/F not finished ret=%d\n", __func__, ret);
		return ret;
	}

	incell->incell_intf_operation = INCELL_TOUCH_RUN;

	if (!mutex_trylock(&info->lock)) {
		incell->incell_intf_operation = INCELL_TOUCH_IDLE;
		ret = INCELL_EBUSY;
		return ret;
	}

	incell->intf_mode = mode;

	switch (mode) {
	case INCELL_DISPLAY_ON:
		incell->intf_mode = INCELL_DISPLAY_HW_RESET;
	case INCELL_DISPLAY_HW_RESET:
		mfd->off_sts = true;
		ret = incell_display_hw_reset(incell, info);
		mfd->off_sts = false;
		break;
	case INCELL_DISPLAY_OFF:
		mfd->off_sts = true;
		ret = incell_display_off(incell, info);
		break;
	default:
		pr_err("%s: Invalid mode for touch interface %d\n",
			__func__, (int)(mode));
		break;
	}

	mutex_unlock(&info->lock);
	incell->incell_intf_operation = INCELL_TOUCH_IDLE;
	return ret;
}

static void incell_panel_power_worker(struct work_struct *work)
{
	struct incell_ctrl *incell = incell_get_info();
	struct fb_info *info = registered_fb[0];

	if (!incell) {
		pr_err("%s: Invalid incell data\n", __func__);
		return;
	}

	if (!info) {
		pr_err("%s: Invalid fb data\n", __func__);
		return;
	}

	mutex_lock(&info->lock);
	incell->worker_state = INCELL_WORKER_ON;

	if (incell->state == INCELL_STATE_SLE000_P1)
		incell_driver_power_off(info);
	else if (incell->state == INCELL_STATE_SLE100_P0 ||
			incell->state == INCELL_STATE_SLE101_P0)
		incell_driver_power_on(info);

	incell->worker_state = INCELL_WORKER_OFF;
	mutex_unlock(&info->lock);
}

static void incell_panel_power_worker_scheduling(incell_pw_lock lock,
						struct incell_ctrl *incell)
{
	incell_state state = incell->state;

	if (lock == INCELL_DISPLAY_POWER_LOCK)
		return;

	if (state != INCELL_STATE_SLE000_P1 &&
		state != INCELL_STATE_SLE100_P0 &&
		state != INCELL_STATE_SLE101_P0)
		return;

	incell->worker_state = INCELL_WORKER_PENDING;
	schedule_work(&incell->incell_work);
}

static void incell_panel_power_worker_canceling(struct incell_ctrl *incell)
{
	struct fb_info *info = registered_fb[0];
	struct msm_fb_data_type *mfd = NULL;

	cancel_work_sync(&incell->incell_work);

	if (!info) {
		pr_err("%s: Invalid fb data\n", __func__);
		goto end;
	}

	mfd = (struct msm_fb_data_type *)info->par;
	if (!mfd) {
		pr_err("%s: Invalid msm data\n", __func__);
		goto end;
	}

	mfd->off_sts = false;

end:
	incell->worker_state = INCELL_WORKER_OFF;
}

static int incell_power_lock(incell_state *state)
{
	int ret = INCELL_OK;

	switch (*state) {
	case INCELL_STATE_SLE010_P0:
	case INCELL_STATE_SLE010_P1:
	case INCELL_STATE_SLE011_P0:
	case INCELL_STATE_SLE011_P1:
	case INCELL_STATE_SLE110_P0:
	case INCELL_STATE_SLE110_P1:
	case INCELL_STATE_SLE111_P0:
	case INCELL_STATE_SLE111_P1:
		pr_err("%s: Power state already locked", __func__);
		ret = INCELL_ALREADY_LOCKED;
		break;
	case INCELL_STATE_SLE000_P0:
		*state = INCELL_STATE_SLE010_P0;
		break;
	case INCELL_STATE_SLE000_P1:
		*state = INCELL_STATE_SLE010_P1;
		break;
	case INCELL_STATE_SLE001_P0:
		*state = INCELL_STATE_SLE011_P0;
		break;
	case INCELL_STATE_SLE001_P1:
		*state = INCELL_STATE_SLE011_P1;
		break;
	case INCELL_STATE_SLE100_P0:
		*state = INCELL_STATE_SLE110_P0;
		break;
	case INCELL_STATE_SLE100_P1:
		*state = INCELL_STATE_SLE110_P1;
		break;
	case INCELL_STATE_SLE101_P0:
		*state = INCELL_STATE_SLE111_P0;
		break;
	case INCELL_STATE_SLE101_P1:
		*state = INCELL_STATE_SLE111_P1;
		break;
	default:
		pr_err("%s: Unknown power status %d\n", __func__,
			(int)(*state));
		ret = INCELL_ERROR;
		break;
	}

	return ret;
}

static int incell_power_unlock(incell_state *state)
{
	int ret = INCELL_OK;

	switch (*state) {
	case INCELL_STATE_SLE000_P0:
	case INCELL_STATE_SLE000_P1:
	case INCELL_STATE_SLE001_P0:
	case INCELL_STATE_SLE001_P1:
	case INCELL_STATE_SLE100_P0:
	case INCELL_STATE_SLE100_P1:
	case INCELL_STATE_SLE101_P0:
	case INCELL_STATE_SLE101_P1:
		pr_err("%s: Power state already unlocked", __func__);
		ret = INCELL_ALREADY_UNLOCKED;
		break;
	case INCELL_STATE_SLE010_P0:
		*state = INCELL_STATE_SLE000_P0;
		break;
	case INCELL_STATE_SLE010_P1:
		*state = INCELL_STATE_SLE000_P1;
		break;
	case INCELL_STATE_SLE011_P0:
		*state = INCELL_STATE_SLE001_P0;
		break;
	case INCELL_STATE_SLE011_P1:
		*state = INCELL_STATE_SLE001_P1;
		break;
	case INCELL_STATE_SLE110_P0:
		*state = INCELL_STATE_SLE100_P0;
		break;
	case INCELL_STATE_SLE110_P1:
		*state = INCELL_STATE_SLE100_P1;
		break;
	case INCELL_STATE_SLE111_P0:
		*state = INCELL_STATE_SLE101_P0;
		break;
	case INCELL_STATE_SLE111_P1:
		*state = INCELL_STATE_SLE101_P1;
		break;
	default:
		pr_err("%s: Unkniown power status %d\n", __func__,
			(int)(*state));
		ret = INCELL_ERROR;
		break;
	}

	return ret;
}

int incell_power_lock_ctrl(incell_pw_lock lock,
		incell_pw_status *power_status)
{
	int ret = INCELL_ERROR;
	struct incell_ctrl *incell = incell_get_info();

	if (!incell) {
		pr_err("%s: Invalid incell data\n", __func__);
		return ret;
	}

	if (incell->worker_state != INCELL_WORKER_OFF) {
		ret = INCELL_EBUSY;
		pr_err("%s: worker scheduling ret=%d\n", __func__, ret);
		return ret;
	}

	if (incell->incell_intf_operation == INCELL_TOUCH_RUN) {
		ret = INCELL_EBUSY;
		pr_err("%s: touch I/F not finished ret=%d\n", __func__, ret);
		return ret;
	}

	incell->incell_intf_operation = INCELL_TOUCH_RUN;

	pr_debug("%s: status:%d --->\n", __func__, ((int)(incell->state)));

	if (incell->state == INCELL_STATE_SLE000_P0 &&
			!sp_panel_forced) {
		incell_force_sp_on();
		sp_panel_forced = true;
	}

	if (lock == INCELL_DISPLAY_POWER_LOCK)
		ret = incell_power_lock(&(incell->state));
	else
		ret = incell_power_unlock(&(incell->state));

	pr_debug("%s: ---> status:%d\n", __func__, ((int)(incell->state)));

	incell_get_power_status(power_status);
	incell_panel_power_worker_scheduling(lock, incell);

	incell->incell_intf_operation = INCELL_TOUCH_IDLE;

	return ret;
}

static void incell_ewu_mode_state_on(incell_state *state)
{
	switch (*state) {
	case INCELL_STATE_SLE000_P0:
		*state = INCELL_STATE_SLE001_P0;
		break;
	case INCELL_STATE_SLE000_P1:
		*state = INCELL_STATE_SLE001_P1;
		break;
	case INCELL_STATE_SLE010_P0:
		*state = INCELL_STATE_SLE011_P0;
		break;
	case INCELL_STATE_SLE010_P1:
		*state = INCELL_STATE_SLE011_P1;
		break;
	case INCELL_STATE_SLE100_P0:
		*state = INCELL_STATE_SLE101_P0;
		break;
	case INCELL_STATE_SLE100_P1:
		*state = INCELL_STATE_SLE101_P1;
		break;
	case INCELL_STATE_SLE110_P0:
		*state = INCELL_STATE_SLE111_P0;
		break;
	case INCELL_STATE_SLE110_P1:
		*state = INCELL_STATE_SLE111_P1;
		break;
	default:
		pr_err("%s: Already EWU enabled\n", __func__);
		break;
	}
}

static void incell_ewu_mode_state_off(incell_state *state)
{
	switch (*state) {
	case INCELL_STATE_SLE001_P0:
		*state = INCELL_STATE_SLE000_P0;
		break;
	case INCELL_STATE_SLE001_P1:
		*state = INCELL_STATE_SLE000_P1;
		break;
	case INCELL_STATE_SLE011_P0:
		*state = INCELL_STATE_SLE010_P0;
		break;
	case INCELL_STATE_SLE011_P1:
		*state = INCELL_STATE_SLE010_P1;
		break;
	case INCELL_STATE_SLE101_P0:
		*state = INCELL_STATE_SLE100_P0;
		break;
	case INCELL_STATE_SLE101_P1:
		*state = INCELL_STATE_SLE100_P1;
		break;
	case INCELL_STATE_SLE111_P0:
		*state = INCELL_STATE_SLE110_P0;
		break;
	case INCELL_STATE_SLE111_P1:
		*state = INCELL_STATE_SLE110_P1;
		break;
	default:
		pr_err("%s: Already EWU disabled\n", __func__);
		break;
	}
}

void incell_ewu_mode_ctrl(incell_ewu_mode ewu)
{
	struct incell_ctrl *incell = incell_get_info();

	if (!incell) {
		pr_err("%s: Invalid incell data\n", __func__);
		return;
	}

	pr_debug("%s: EWU mode is %s\n", __func__,
			ewu == INCELL_DISPLAY_EWU_ENABLE ? "on":"off");

	pr_debug("%s: status:%d --->\n", __func__, ((int)(incell->state)));

	if ((ewu == INCELL_DISPLAY_EWU_ENABLE))
		incell_ewu_mode_state_on(&(incell->state));
	else
		incell_ewu_mode_state_off(&(incell->state));

	pr_debug("%s: ---> status:%d\n", __func__, ((int)(incell->state)));
}

void incell_panel_fb_notifier_call_chain(
		struct msm_fb_data_type *mfd, int blank, bool type)
{
	struct fb_event event;

	if ((mfd->panel_info->type == MIPI_VIDEO_PANEL) ||
	    (mfd->panel_info->type == MIPI_CMD_PANEL)) {
		if (!incell_panel_is_incell_operation()) {
			event.info = mfd->fbi;
			event.data = &blank;

			if (type == FB_NOTIFIER_PRE) {
				fb_notifier_call_chain(FB_EXT_EARLY_EVENT_BLANK, &event);
			} else {
				fb_notifier_call_chain(FB_EXT_EVENT_BLANK, &event);
			}
		}
	}
}

static int incell_panel_request_gpios(struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	struct mdss_panel_specific_pdata *spec_pdata = NULL;
	int rc = 0;

	if (ctrl_pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	spec_pdata = ctrl_pdata->spec_pdata;

	if (gpio_is_valid(spec_pdata->disp_vddio_gpio)) {
		rc = gpio_request(spec_pdata->disp_vddio_gpio,
						"disp_vddio");
		if (rc) {
			pr_err("request disp vddio gpio failed, rc=%d\n",
				       rc);
			goto disp_vddio_gpio_err;
		}
	}

	if (gpio_is_valid(spec_pdata->touch_vddio_gpio)) {
		rc = gpio_request(spec_pdata->touch_vddio_gpio,
						"touch_vddio");
		if (rc) {
			pr_err("request touch vddio gpio failed, rc=%d\n",
				       rc);
			goto touch_vddio_gpio_err;
		}
	}

	if (gpio_is_valid(spec_pdata->touch_reset_gpio)) {
		rc = gpio_request(spec_pdata->touch_reset_gpio,
						"touch_reset");
		if (rc) {
			pr_err("request touch reset gpio failed,rc=%d\n",
								rc);
			goto touch_reset_gpio_err;
		}
	}

	if (gpio_is_valid(spec_pdata->touch_int_gpio)) {
		rc = gpio_request(spec_pdata->touch_int_gpio,
						"touch_int");
		if (rc) {
			pr_err("request touch int gpio failed,rc=%d\n",
								rc);
			goto touch_int_gpio_err;
		}
	}

	return rc;

touch_int_gpio_err:
	if (gpio_is_valid(spec_pdata->touch_reset_gpio))
		gpio_free(spec_pdata->touch_reset_gpio);
touch_reset_gpio_err:
	if (gpio_is_valid(spec_pdata->touch_vddio_gpio))
		gpio_free(spec_pdata->touch_vddio_gpio);
touch_vddio_gpio_err:
	if (gpio_is_valid(spec_pdata->disp_vddio_gpio))
		gpio_free(spec_pdata->disp_vddio_gpio);
disp_vddio_gpio_err:
	return rc;
}

void incell_panel_free_gpios(struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	struct mdss_panel_specific_pdata *spec_pdata = NULL;

	if (ctrl_pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return;
	}

	spec_pdata = ctrl_pdata->spec_pdata;

	if (gpio_is_valid(spec_pdata->touch_int_gpio))
		gpio_free(spec_pdata->touch_int_gpio);

	if (gpio_is_valid(spec_pdata->touch_reset_gpio))
		gpio_free(spec_pdata->touch_reset_gpio);

	if (gpio_is_valid(spec_pdata->touch_vddio_gpio))
		gpio_free(spec_pdata->touch_vddio_gpio);

	if (gpio_is_valid(spec_pdata->disp_vddio_gpio))
		gpio_free(spec_pdata->disp_vddio_gpio);
}

int incell_pinctrl_init(struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	ctrl_pdata->pin_res.touch_state_active
		= pinctrl_lookup_state(ctrl_pdata->pin_res.pinctrl,
				MDSS_PINCTRL_STATE_TOUCH_ACTIVE);
	if (IS_ERR_OR_NULL(ctrl_pdata->pin_res.touch_state_active))
		pr_warn("%s: can not get touch active pinstate\n", __func__);

	ctrl_pdata->pin_res.touch_state_suspend
		= pinctrl_lookup_state(ctrl_pdata->pin_res.pinctrl,
				MDSS_PINCTRL_STATE_TOUCH_SUSPEND);
	if (IS_ERR_OR_NULL(ctrl_pdata->pin_res.touch_state_suspend))
		pr_warn("%s: can not get touch suspend pinstate\n", __func__);

	return 0;
}

void incell_driver_init(bool cont_splash_enabled)
{
	memset(&incell_buf, 0, sizeof(struct incell_ctrl));
	incell = &incell_buf;

	if (cont_splash_enabled)
		incell->state = INCELL_STATE_SLE100_P1;
	else
		incell->state = INCELL_STATE_SLE000_P0;

	incell->incell_intf_operation = INCELL_TOUCH_IDLE;

	pr_debug("%s: state:%d touch operation:%d\n", __func__,
		(int)(incell->state), (int)(incell->incell_intf_operation));

	incell->worker_state = INCELL_WORKER_OFF;
	INIT_WORK(&incell->incell_work, incell_panel_power_worker);
}

int incell_panel_driver_init(struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	struct mdss_panel_specific_pdata *spec_pdata = NULL;
	struct mdss_panel_info *pinfo = NULL;

	pinfo = &ctrl_pdata->panel_data.panel_info;

	spec_pdata = ctrl_pdata->spec_pdata;

	spec_pdata->panel_power_off = incell_dsi_panel_power_off_ex;
	spec_pdata->panel_power_on = incell_dsi_panel_power_on_ex;
	spec_pdata->panel_post_on = incell_driver_post_power_on;
	spec_pdata->dsi_panel_off_ex = incell_dsi_panel_off_ex;
	spec_pdata->dsi_request_gpios = incell_panel_request_gpios;
	spec_pdata->parse_specific_dt = incell_parse_dt;

	incell_driver_init(pinfo->cont_splash_enabled);

	return 0;
}

