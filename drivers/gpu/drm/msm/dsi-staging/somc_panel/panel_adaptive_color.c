/*
 * Restructured unified display panel driver for Xperia Open Devices
 *               *** Adaptive Color Management ***
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
#include "../sde/sde_crtc.h"
#include "../sde/sde_plane.h"


int somc_panel_parse_dt_adaptivecolor_config(struct dsi_panel *panel,
			struct device_node *np)
{
	struct somc_panel_color_mgr *color_mgr = NULL;
	struct somc_panel_adaptive_color *ad_col = NULL;
	u32 tmp = 0;
	int rc = 0;

	if (!panel) {
		pr_err("%s: Invalid input panel\n", __func__);
		return -EINVAL;
	}

	if (!panel->spec_pdata) {
		pr_err("%s: spec_pdata not initialized!!\n", __func__);
		return -EINVAL;
	}

	color_mgr = panel->spec_pdata->color_mgr;
	ad_col = &color_mgr->adaptive_color;

	color_mgr->dsi_pcc_applied = false;
	ad_col->enable =
		of_property_read_bool(np, "somc,panel-adaptivecolor-enable");

	/* Picture Adjustment (PicAdj) Parameters */
	if (!of_find_property(np, "somc,mdss-dsi-use-picadj", NULL))
		goto end;

	rc = of_property_read_u32(np, "somc,panel-adacolor-sat-max", &tmp);
	ad_col->picadj_data_br_max.saturation = !rc ? tmp : -1;

	rc = of_property_read_u32(np, "somc,panel-adacolor-hue-max", &tmp);
	ad_col->picadj_data_br_max.hue = !rc ? tmp : -1;

	rc = of_property_read_u32(np, "somc,panel-adacolor-val-max", &tmp);
	ad_col->picadj_data_br_max.value = !rc ? tmp : -1;

	rc = of_property_read_u32(np, "somc,panel-adacolor-cont-max", &tmp);
	ad_col->picadj_data_br_max.contrast = !rc ? tmp : -1;

	rc = of_property_read_u32(np, "somc,panel-adacolor-br-min", &tmp);
	ad_col->picadj_br_min = !rc ? tmp : -1;

	rc = of_property_read_u32(np, "somc,panel-adacolor-br-max", &tmp);
	ad_col->picadj_br_max = !rc ? tmp : -1;

	ad_col->pa_invert =
		of_property_read_bool(np, "somc,panel-adacolor-pa-invert");

	rc = 0;

	/* AdaptiveColor is enabled: copy the default PicAdj values! */
	ad_col->picadj_data_default.hue = color_mgr->picadj_data.hue;
	ad_col->picadj_data_default.saturation =
					color_mgr->picadj_data.saturation;
	ad_col->picadj_data_default.value = color_mgr->picadj_data.value;
	ad_col->picadj_data_default.contrast = color_mgr->picadj_data.contrast;

end:
	return rc;
}

static inline void somc_panel_colormgr_set_ac_pa(
	struct somc_panel_color_mgr *color_mgr, struct drm_msm_pa_hsic *pa)
{
	color_mgr->picadj_data.hue = pa->hue;
	color_mgr->picadj_data.saturation = pa->saturation;
	color_mgr->picadj_data.value = pa->value;
	color_mgr->picadj_data.contrast = pa->contrast;
}
			

/*
 * Set a percentual of the "data_br_max" PicAdj settings based on
 * backlight percentage.
 * This should require a different way to calculate things, since the
 * algorithm to calculate backlight based color adjustment should
 * scientifically not be linear, but for our usecases this linear
 * calculation is definitely sufficient, as we don't care of very
 * small deviations (since that's how our displays are reacting for now).
 *
 * \params
 * bl_lvl - Backlight brightness level
 * invert - Invert the calculation (higher PA values on lower BL values)
 */
static void somc_panel_colormgr_adj_pa(struct dsi_panel *panel,
					u32 bl_lvl, bool invert)
{
	struct somc_panel_color_mgr *color_mgr =
			panel->spec_pdata->color_mgr;
	struct somc_panel_adaptive_color *ad_col =
			&color_mgr->adaptive_color;
	struct drm_msm_pa_hsic *data_br_max = NULL;
	struct drm_msm_pa_hsic *data_br_default = NULL;
	u8 bl_percent = 0;
	u8 pa_percent = 100;

	if (invert) {
		data_br_default = &ad_col->picadj_data_br_max;
		data_br_max = &ad_col->picadj_data_default;
	}

	/* Processing shortcuts for imposed algorithm limits */
	if (bl_lvl < ad_col->picadj_br_min) {
		somc_panel_colormgr_set_ac_pa(color_mgr, data_br_default);
		return;
	} else if (bl_lvl > ad_col->picadj_br_max) {
		somc_panel_colormgr_set_ac_pa(color_mgr, data_br_max);
		return;
	}

	bl_percent = (bl_lvl * 100) / ad_col->picadj_br_max;

	if (invert)
		pa_percent -= bl_percent;
	else
		pa_percent = bl_percent;

	color_mgr->picadj_data.hue = (u32)
		(ad_col->picadj_data_br_max.hue * pa_percent) / 100;
	color_mgr->picadj_data.saturation = (u32)
		(ad_col->picadj_data_br_max.saturation * pa_percent) / 100;
	color_mgr->picadj_data.contrast = (u32)
		(ad_col->picadj_data_br_max.contrast * pa_percent) / 100;
	color_mgr->picadj_data.value = (u32)
		(ad_col->picadj_data_br_max.value * pa_percent) / 100;

	return;
}

void somc_panel_colormgr_update_backlight(struct dsi_panel *panel, u32 bl_lvl)
{
	struct dsi_display *display = NULL;
	struct somc_panel_color_mgr *color_mgr = panel->spec_pdata->color_mgr;

	if (!color_mgr)
		return;

	if (!color_mgr->adaptive_color.enable)
		return;

	display = dsi_display_get_main_display();
	if (!display) {
		pr_err("ERROR: Cannot update for Adaptive Color\n");
		return;
	}

	/* PA adjustment */
	somc_panel_colormgr_adj_pa(panel, bl_lvl,
		color_mgr->adaptive_color.pa_invert);

	somc_panel_send_pa(display);
}
