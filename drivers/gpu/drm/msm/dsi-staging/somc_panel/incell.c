/*
 * Copyright (c) 2016-2017, The Linux Foundation. All rights reserved.
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
/*
 * Copyright (C) 2017 Sony Mobile Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 */

#include <linux/incell.h>
#include <linux/string.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/device.h>
#include <linux/sde_io_util.h>

#include "dsi_display.h"
#include "dsi_panel.h"
#include "somc_panel_exts.h"

struct incell_ctrl *incell;
struct incell_ctrl incell_buf;

struct incell_ctrl *incell_get_info(void)
{
	return incell;
}

int incell_get_power_status(incell_pw_status *power_status)
{
	struct incell_ctrl *incell = incell_get_info();

	if (!incell) {
		pr_err("%s: Invalid incell data\n", __func__);
		return INCELL_ERROR;
	}

	pr_debug("%s: status = 0x%x\n", __func__, (incell->state));

	if (dsi_panel_driver_is_power_on(incell->state)) {
		power_status->display_power = INCELL_POWER_ON;
		power_status->touch_power = INCELL_POWER_ON;
	} else {
		power_status->display_power = INCELL_POWER_OFF;
		power_status->touch_power = INCELL_POWER_OFF;
	}

	return INCELL_OK;
}
EXPORT_SYMBOL(incell_get_power_status);

int incell_control_mode(incell_intf_mode mode, bool force)
{
	int ret = INCELL_ERROR;
	struct incell_ctrl *incell = incell_get_info();
	struct dsi_display *display = dsi_display_get_main_display();
	struct dsi_panel *panel = display->panel;

	pr_notice("%s: START - %s:%s\n", __func__,
		((mode == INCELL_TOUCH_RESET) ? "INCELL_TOUCH_RESET" : "UNKNOWN"),
		((force) ? "force" : "unforce"));

	if (!incell) {
		pr_err("%s: Invalid incell data\n", __func__);
		return ret;
	}

	if (!incell->priv) {
		pr_err("%s: Invalid drm data\n", __func__);
		return ret;
	}

	incell->intf_mode = mode;

	switch (mode) {
	case INCELL_TOUCH_RESET:
		ret = dsi_panel_driver_touch_reset_ctrl(panel, force);
		break;

	case INCELL_CONT_SPLASH_TOUCH_ENABLE:
		ret = somc_panel_cont_splash_touch_enable(panel);
		break;

	default:
		pr_err("%s: Invalid mode for touch interface %d\n",
			__func__, (int)(mode));
		break;
	}

	pr_notice("%s: FINISH - incell.status:0x%x\n", __func__, (incell->state));

	return ret;
}
EXPORT_SYMBOL(incell_control_mode);

static int incell_power_lock(unsigned char *state)
{
	int ret = INCELL_ERROR;

	switch (*state) {
	case INCELL_S000:
	case INCELL_S100:
		pr_err("%s: Lock is invalid during power off", __func__);
		ret =  INCELL_ERROR;
		break;
	case INCELL_S001:
	case INCELL_S101:
		*state |= INCELL_LOCK_STATE_ON;
		ret = INCELL_OK;
		break;
	case INCELL_S010:
	case INCELL_S011:
	case INCELL_S110:
	case INCELL_S111:
		pr_err("%s: Power state already locked", __func__);
		ret = INCELL_ALREADY_LOCKED;
		break;
	default:
		pr_err("%s: Lock is invalid, unknown state", __func__);
		ret = INCELL_ERROR;
		break;
	}

	return ret;
}

static int incell_power_unlock(unsigned char *state)
{
	struct dsi_display *display = dsi_display_get_main_display();
	struct dsi_panel *panel = display->panel;
	int ret = INCELL_ERROR;

	switch (*state) {
	case INCELL_S000:
	case INCELL_S001:
	case INCELL_S100:
	case INCELL_S101:
		pr_err("%s: Power state already unlocked", __func__);
		ret = INCELL_ALREADY_UNLOCKED;
		break;
	case INCELL_S010:
	case INCELL_S110:
	case INCELL_S111:
		*state &= INCELL_LOCK_STATE_OFF;
		ret = INCELL_OK;
		break;
	case INCELL_S011:
		*state &= INCELL_LOCK_STATE_OFF;
		pr_notice("%s: Panel power off by incell", __func__);
		dsi_panel_driver_power_off(panel);
		dsi_panel_driver_post_power_off(panel);
		ret = INCELL_OK;
		break;
	default:
		pr_err("%s: Unlock is invalid, unknown state", __func__);
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

	if (incell->incell_intf_operation == INCELL_TOUCH_RUN) {
		ret = INCELL_EBUSY;
		pr_err("%s: touch I/F not finished ret=%d\n", __func__, ret);
		return ret;
	}

	incell->incell_intf_operation = INCELL_TOUCH_RUN;

	pr_debug("%s: status:0x%x --->\n", __func__, (incell->state));

	if (lock == INCELL_DISPLAY_POWER_LOCK)
		ret = incell_power_lock(&(incell->state));
	else
		ret = incell_power_unlock(&(incell->state));

	pr_debug("%s: ---> status:0x%x\n", __func__, (incell->state));

	incell_get_power_status(power_status);

	incell->incell_intf_operation = INCELL_TOUCH_IDLE;

	return ret;
}
EXPORT_SYMBOL(incell_power_lock_ctrl);

int incell_get_panel_name(void)
{
	struct dsi_display *display = dsi_display_get_main_display();
	struct dsi_panel *panel;
	int panel_name = 0;
	int rc = 0;

	if (!display) {
		pr_err("%s: Invalid display data\n", __func__);
		return rc;
	}

	panel = display->panel;
	rc = kstrtoint(panel->name, 0, &panel_name);
	if (rc) {
		pr_err("%s: int conversion failed\n", __func__);
		panel_name = 0;
	}

	return panel_name;
}
EXPORT_SYMBOL(incell_get_panel_name);

int incell_get_display_pre_sod(void)
{
	return somc_panel_get_display_pre_sod_mode();
}
EXPORT_SYMBOL(incell_get_display_pre_sod);

int incell_get_display_sod(void)
{
	return get_display_sod_mode();
}
EXPORT_SYMBOL(incell_get_display_sod);

int incell_get_display_aod(void)
{
	return somc_panel_get_display_aod_mode();
}
EXPORT_SYMBOL(incell_get_display_aod);

bool incell_get_system_status(void)
{
	struct incell_ctrl *incell = incell_get_info();
	bool ret = false;

	if (!incell) {
		pr_err("%s: Invalid incell data\n", __func__);
		return true;
	}

	pr_debug("%s: ---> status:0x%x\n", __func__, (incell->state));
	if (incell->state & INCELL_SYSTEM_STATE_ON)
		ret = true;

	return ret;
}
EXPORT_SYMBOL(incell_get_system_status);

void incell_driver_init(struct msm_drm_private *priv)
{
	struct dsi_display *display = dsi_display_get_main_display();

	if (!display) {
		pr_err("%s: Invalid display data\n", __func__);
		return;
	}

	memset(&incell_buf, 0, sizeof(struct incell_ctrl));
	incell = &incell_buf;

	incell->incell_intf_operation = INCELL_TOUCH_IDLE;

	incell->priv = priv;

	if (display->is_cont_splash_enabled)
		incell->state = INCELL_S101;
	else
		incell->state = INCELL_S000;
}

bool incell_touch_is_compatible(incell_touch_type type) {
	struct dsi_display *display = dsi_display_get_main_display();
	struct dsi_panel *panel = display->panel;

	if (panel->touch_type == INCELL_TOUCH_TYPE_DEFAULT)
		/* Default/unset. Any driver is allowed to probe: */
		return true;

	return type == panel->touch_type;
}
