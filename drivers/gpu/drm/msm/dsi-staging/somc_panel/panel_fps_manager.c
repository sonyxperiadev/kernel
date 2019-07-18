/*
 * Restructured unified display panel driver for Xperia Open Devices
 *                     *** FPS Management ***
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
#include "somc_panel_exts.h"
#include "dsi_display.h"
#include "dsi_panel.h"

int somc_panel_parse_dt_chgfps_config(struct dsi_panel *panel,
			struct device_node *np)
{
	struct dsi_fps_mode *chg_fps = NULL;
	const char *panel_mode;
	const char *panel_type;
	int rc = 0;

	if (!panel) {
		pr_err("%s: Invalid input panel\n", __func__);
		return -EINVAL;
	}

	if (!panel->spec_pdata) {
		pr_err("%s: spec_pdata not initialized!!\n", __func__);
		return -EINVAL;
	}

	chg_fps = &panel->spec_pdata->fps_mode;

	chg_fps->enable = of_property_read_bool(np,
					"somc,fps-mode-enable");
	if (!chg_fps->enable) {
		pr_info("%s: Dynamic FPS not enabled. Skipping conf.\n",
			__func__);
		return 0;
	}

	panel_type = of_get_property(np,
				"somc,fps-mode-panel-type", NULL);
	if (!panel_type) {
		pr_err("%s:%d, Panel type not specified\n",
						__func__, __LINE__);
	} else {
		if (!strncmp(panel_type, "uhd_4k_type", 11)) {
			chg_fps->type = FPS_TYPE_UHD_4K;
		} else if (!strncmp(panel_type, "hybrid_incell_type", 18)) {
			chg_fps->type = FPS_TYPE_HYBRID_INCELL;
		} else if (!strncmp(panel_type, "full_incell_type", 16)) {
			chg_fps->type = FPS_TYPE_FULL_INCELL;
		} else {
			pr_err("%s: Unable to read fps panel type\n", __func__);
			goto error;
		}
	}

	panel_mode = of_get_property(np, "somc,fps-mode-panel-mode", NULL);

	if (!panel_mode) {
		pr_err("%s:%d, Panel mode not specified\n",
						__func__, __LINE__);
		return -EINVAL;
	}

	if (!strncmp(panel_mode, "susres_mode", 11)) {
		chg_fps->mode = FPS_MODE_SUSRES;
	} else if (!strncmp(panel_mode, "dynamic_mode", 12)) {
		chg_fps->mode = FPS_MODE_DYNAMIC;
	} else {
		pr_err("%s: Unable to read fps panel mode\n",
			__func__);
		return -EINVAL;
	}

	chg_fps->type = FPS_MODE_OFF_RR_OFF;

error:
	return rc;
}

void dsi_panel_fps_mode_set(struct dsi_panel *panel, int mode_type)
{
	struct dsi_fps_mode *chg_fps = &panel->spec_pdata->fps_mode;

	switch (mode_type) {
	case FPS_MODE_OFF_RR_OFF:
		dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_FPS_MODE_OFF_RR_OFF);
		chg_fps->type =  FPS_MODE_OFF_RR_OFF;
		break;
	case FPS_MODE_ON_RR_OFF:
		dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_FPS_MODE_ON_RR_OFF);
		chg_fps->type = FPS_MODE_ON_RR_OFF;
		break;
	case FPS_MODE_OFF_RR_ON:
		dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_FPS_MODE_OFF_RR_ON);
		chg_fps->type = FPS_MODE_OFF_RR_ON;
		break;
	case FPS_MODE_ON_RR_ON:
		dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_FPS_MODE_ON_RR_ON);
		chg_fps->type = FPS_MODE_ON_RR_ON;
		break;
	default:
		pr_err("%s: invalid value for fps mode type = %d\n",
			__func__, mode_type);
		break;
	}
}

void somc_panel_fps_cmd_send(struct dsi_panel *panel)
{
	struct dsi_fps_mode *chg_fps = &panel->spec_pdata->fps_mode;

	if (chg_fps->enable)
		dsi_panel_fps_mode_set(panel, chg_fps->type);
	else
		pr_debug("%s: change fps is not supported.\n", __func__);
}

int somc_panel_fps_check_state(struct dsi_display *display, int mode_type)
{
	struct dsi_fps_mode *chg_fps = &display->panel->spec_pdata->fps_mode;

	if (!display->panel->spec_pdata->display_onoff_state)
		goto disp_onoff_state_err;

	chg_fps->type = mode_type;

	if (chg_fps->mode == FPS_MODE_DYNAMIC)
		dsi_panel_fps_mode_set(display->panel,
			chg_fps->type);

	return 0;

disp_onoff_state_err:
	pr_err("%s: Disp-On is not yet completed. Please retry\n", __func__);
	return -EINVAL;
}

static ssize_t somc_panel_fps_mode_store(struct device *dev,
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

	rc = somc_panel_fps_check_state(display, mode_type);
	if (rc) {
		pr_err("%s: Error, rc = %d\n", __func__, rc);
		return rc;
	}
	return count;
}

static ssize_t somc_panel_fps_mode_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct panel_specific_pdata *spec_pdata = NULL;
	struct dsi_display *display = dev_get_drvdata(dev);

	spec_pdata = display->panel->spec_pdata;

	return scnprintf(buf, PAGE_SIZE, "%d\n", spec_pdata->fps_mode.type);
}

static struct device_attribute panel_fps_attributes[] = {
	__ATTR(fps_mode, S_IRUGO|S_IWUSR|S_IWGRP,
		somc_panel_fps_mode_show,
		somc_panel_fps_mode_store),
};

int somc_panel_fps_register_attr(struct device *dev)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(panel_fps_attributes); i++)
		if (device_create_file(dev, panel_fps_attributes + i))
			goto error;
	return 0;
error:
	dev_err(dev, "%s: Cannot create FPS Manager attributes\n", __func__);
	for (--i; i >= 0 ; i--)
		device_remove_file(dev, panel_fps_attributes + i);
	return -ENODEV;
}

int somc_panel_fps_manager_init(struct dsi_display *display)
{
	struct panel_specific_pdata *spec_pdata;

	if (display == NULL)
		return -EINVAL;
	if (display->panel == NULL)
		return -EINVAL;

	spec_pdata = display->panel->spec_pdata;
	if (!spec_pdata) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	return 0;
}
