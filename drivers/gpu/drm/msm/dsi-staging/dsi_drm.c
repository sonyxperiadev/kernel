/*
 * Copyright (c) 2016-2019, The Linux Foundation. All rights reserved.
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


#define pr_fmt(fmt)	"dsi-drm:[%s] " fmt, __func__
#include <drm/drm_atomic_helper.h>
#include <drm/drm_atomic.h>

#include "msm_kms.h"
#include "sde_connector.h"
#include "dsi_drm.h"
#include "sde_trace.h"

#define to_dsi_bridge(x)     container_of((x), struct dsi_bridge, base)
#define to_dsi_state(x)      container_of((x), struct dsi_connector_state, base)

#define DEFAULT_PANEL_JITTER_NUMERATOR		2
#define DEFAULT_PANEL_JITTER_DENOMINATOR	1
#define DEFAULT_PANEL_JITTER_ARRAY_SIZE		2
#define DEFAULT_PANEL_PREFILL_LINES	25

static struct dsi_display_mode_priv_info default_priv_info = {
	.panel_jitter_numer = DEFAULT_PANEL_JITTER_NUMERATOR,
	.panel_jitter_denom = DEFAULT_PANEL_JITTER_DENOMINATOR,
	.panel_prefill_lines = DEFAULT_PANEL_PREFILL_LINES,
	.dsc_enabled = false,
};

static void convert_to_dsi_mode(const struct drm_display_mode *drm_mode,
				struct dsi_display_mode *dsi_mode)
{
	memset(dsi_mode, 0, sizeof(*dsi_mode));

	dsi_mode->timing.h_active = drm_mode->hdisplay;
	dsi_mode->timing.h_back_porch = drm_mode->htotal - drm_mode->hsync_end;
	dsi_mode->timing.h_sync_width = drm_mode->htotal -
			(drm_mode->hsync_start + dsi_mode->timing.h_back_porch);
	dsi_mode->timing.h_front_porch = drm_mode->hsync_start -
					 drm_mode->hdisplay;
	dsi_mode->timing.h_skew = drm_mode->hskew;

	dsi_mode->timing.v_active = drm_mode->vdisplay;
	dsi_mode->timing.v_back_porch = drm_mode->vtotal - drm_mode->vsync_end;
	dsi_mode->timing.v_sync_width = drm_mode->vtotal -
		(drm_mode->vsync_start + dsi_mode->timing.v_back_porch);

	dsi_mode->timing.v_front_porch = drm_mode->vsync_start -
					 drm_mode->vdisplay;

	dsi_mode->timing.refresh_rate = drm_mode->vrefresh;

	dsi_mode->pixel_clk_khz = drm_mode->clock;

	dsi_mode->priv_info =
		(struct dsi_display_mode_priv_info *)drm_mode->private;

	if (dsi_mode->priv_info) {
		dsi_mode->timing.dsc_enabled = dsi_mode->priv_info->dsc_enabled;
		dsi_mode->timing.dsc = &dsi_mode->priv_info->dsc;
	}

	if (msm_is_mode_seamless(drm_mode))
		dsi_mode->dsi_mode_flags |= DSI_MODE_FLAG_SEAMLESS;
	if (msm_is_mode_dynamic_fps(drm_mode))
		dsi_mode->dsi_mode_flags |= DSI_MODE_FLAG_DFPS;
	if (msm_needs_vblank_pre_modeset(drm_mode))
		dsi_mode->dsi_mode_flags |= DSI_MODE_FLAG_VBLANK_PRE_MODESET;
	if (msm_is_mode_seamless_dms(drm_mode))
		dsi_mode->dsi_mode_flags |= DSI_MODE_FLAG_DMS;
	if (msm_is_mode_seamless_vrr(drm_mode))
		dsi_mode->dsi_mode_flags |= DSI_MODE_FLAG_VRR;
	if (msm_is_mode_seamless_dyn_clk(drm_mode))
		dsi_mode->dsi_mode_flags |= DSI_MODE_FLAG_DYN_CLK;

	dsi_mode->timing.h_sync_polarity =
			!!(drm_mode->flags & DRM_MODE_FLAG_PHSYNC);
	dsi_mode->timing.v_sync_polarity =
			!!(drm_mode->flags & DRM_MODE_FLAG_PVSYNC);
}

void dsi_convert_to_drm_mode(const struct dsi_display_mode *dsi_mode,
				struct drm_display_mode *drm_mode)
{
	memset(drm_mode, 0, sizeof(*drm_mode));

	drm_mode->hdisplay = dsi_mode->timing.h_active;
	drm_mode->hsync_start = drm_mode->hdisplay +
				dsi_mode->timing.h_front_porch;
	drm_mode->hsync_end = drm_mode->hsync_start +
			      dsi_mode->timing.h_sync_width;
	drm_mode->htotal = drm_mode->hsync_end + dsi_mode->timing.h_back_porch;
	drm_mode->hskew = dsi_mode->timing.h_skew;

	drm_mode->vdisplay = dsi_mode->timing.v_active;
	drm_mode->vsync_start = drm_mode->vdisplay +
				dsi_mode->timing.v_front_porch;
	drm_mode->vsync_end = drm_mode->vsync_start +
			      dsi_mode->timing.v_sync_width;
	drm_mode->vtotal = drm_mode->vsync_end + dsi_mode->timing.v_back_porch;

	drm_mode->vrefresh = dsi_mode->timing.refresh_rate;
	drm_mode->clock = dsi_mode->pixel_clk_khz;

	drm_mode->private = (int *)dsi_mode->priv_info;

	if (dsi_mode->dsi_mode_flags & DSI_MODE_FLAG_SEAMLESS)
		drm_mode->flags |= DRM_MODE_FLAG_SEAMLESS;
	if (dsi_mode->dsi_mode_flags & DSI_MODE_FLAG_DFPS)
		drm_mode->private_flags |= MSM_MODE_FLAG_SEAMLESS_DYNAMIC_FPS;
	if (dsi_mode->dsi_mode_flags & DSI_MODE_FLAG_VBLANK_PRE_MODESET)
		drm_mode->private_flags |= MSM_MODE_FLAG_VBLANK_PRE_MODESET;
	if (dsi_mode->dsi_mode_flags & DSI_MODE_FLAG_DMS)
		drm_mode->private_flags |= MSM_MODE_FLAG_SEAMLESS_DMS;
	if (dsi_mode->dsi_mode_flags & DSI_MODE_FLAG_VRR)
		drm_mode->private_flags |= MSM_MODE_FLAG_SEAMLESS_VRR;
	if (dsi_mode->dsi_mode_flags & DSI_MODE_FLAG_DYN_CLK)
		drm_mode->private_flags |= MSM_MODE_FLAG_SEAMLESS_DYN_CLK;

	if (dsi_mode->timing.h_sync_polarity)
		drm_mode->flags |= DRM_MODE_FLAG_PHSYNC;
	if (dsi_mode->timing.v_sync_polarity)
		drm_mode->flags |= DRM_MODE_FLAG_PVSYNC;

	/* set mode name */
	snprintf(drm_mode->name, DRM_DISPLAY_MODE_LEN, "%dx%dx%dx%d",
			drm_mode->hdisplay, drm_mode->vdisplay,
			drm_mode->vrefresh, drm_mode->clock);
}

static int dsi_bridge_attach(struct drm_bridge *bridge)
{
	struct dsi_bridge *c_bridge = to_dsi_bridge(bridge);

	if (!bridge) {
		pr_err("Invalid params\n");
		return -EINVAL;
	}

	pr_debug("[%d] attached\n", c_bridge->id);

	return 0;

}

static void dsi_bridge_pre_enable(struct drm_bridge *bridge)
{
	int rc = 0;
	struct dsi_bridge *c_bridge = to_dsi_bridge(bridge);

	if (!bridge) {
		pr_err("Invalid params\n");
		return;
	}

	if (!c_bridge || !c_bridge->display || !c_bridge->display->panel) {
		pr_err("Incorrect bridge details\n");
		return;
	}

	atomic_set(&c_bridge->display->panel->esd_recovery_pending, 0);

	/* By this point mode should have been validated through mode_fixup */
	rc = dsi_display_set_mode(c_bridge->display,
			&(c_bridge->dsi_mode), 0x0);
	if (rc) {
		pr_err("[%d] failed to perform a mode set, rc=%d\n",
		       c_bridge->id, rc);
		return;
	}

	if (c_bridge->dsi_mode.dsi_mode_flags &
		(DSI_MODE_FLAG_SEAMLESS | DSI_MODE_FLAG_VRR |
		 DSI_MODE_FLAG_DYN_CLK)) {
		pr_debug("[%d] seamless pre-enable\n", c_bridge->id);
		return;
	}

	SDE_ATRACE_BEGIN("dsi_bridge_pre_enable");
	rc = dsi_display_prepare(c_bridge->display);
	if (rc) {
		pr_err("[%d] DSI display prepare failed, rc=%d\n",
		       c_bridge->id, rc);
		SDE_ATRACE_END("dsi_bridge_pre_enable");
		return;
	}

	SDE_ATRACE_BEGIN("dsi_display_enable");
	rc = dsi_display_enable(c_bridge->display);
	if (rc) {
		pr_err("[%d] DSI display enable failed, rc=%d\n",
				c_bridge->id, rc);
		(void)dsi_display_unprepare(c_bridge->display);
	}
	SDE_ATRACE_END("dsi_display_enable");
	SDE_ATRACE_END("dsi_bridge_pre_enable");

	rc = dsi_display_splash_res_cleanup(c_bridge->display);
	if (rc)
		pr_err("Continuous splash pipeline cleanup failed, rc=%d\n",
									rc);
}

static void dsi_bridge_enable(struct drm_bridge *bridge)
{
	int rc = 0;
	struct dsi_bridge *c_bridge = to_dsi_bridge(bridge);
	struct dsi_display *display;

	if (!bridge) {
		pr_err("Invalid params\n");
		return;
	}

	if (c_bridge->dsi_mode.dsi_mode_flags &
			(DSI_MODE_FLAG_SEAMLESS | DSI_MODE_FLAG_VRR |
			 DSI_MODE_FLAG_DYN_CLK)) {
		pr_debug("[%d] seamless enable\n", c_bridge->id);
		return;
	}
	display = c_bridge->display;

	rc = dsi_display_post_enable(display);
	if (rc)
		pr_err("[%d] DSI display post enabled failed, rc=%d\n",
		       c_bridge->id, rc);

	if (display && display->drm_conn)
		sde_connector_helper_bridge_enable(display->drm_conn);
}

static void dsi_bridge_disable(struct drm_bridge *bridge)
{
	int rc = 0;
	struct dsi_display *display;
	struct dsi_bridge *c_bridge = to_dsi_bridge(bridge);

	if (!bridge) {
		pr_err("Invalid params\n");
		return;
	}
	display = c_bridge->display;

	if (display && display->drm_conn)
		sde_connector_helper_bridge_disable(display->drm_conn);

	rc = dsi_display_pre_disable(c_bridge->display);
	if (rc) {
		pr_err("[%d] DSI display pre disable failed, rc=%d\n",
		       c_bridge->id, rc);
	}
}

static void dsi_bridge_post_disable(struct drm_bridge *bridge)
{
	int rc = 0;
	struct dsi_bridge *c_bridge = to_dsi_bridge(bridge);

	if (!bridge) {
		pr_err("Invalid params\n");
		return;
	}

	SDE_ATRACE_BEGIN("dsi_bridge_post_disable");
	SDE_ATRACE_BEGIN("dsi_display_disable");
	rc = dsi_display_disable(c_bridge->display);
	if (rc) {
		pr_err("[%d] DSI display disable failed, rc=%d\n",
		       c_bridge->id, rc);
		SDE_ATRACE_END("dsi_display_disable");
		return;
	}
	SDE_ATRACE_END("dsi_display_disable");

	rc = dsi_display_unprepare(c_bridge->display);
	if (rc) {
		pr_err("[%d] DSI display unprepare failed, rc=%d\n",
		       c_bridge->id, rc);
		SDE_ATRACE_END("dsi_bridge_post_disable");
		return;
	}
	SDE_ATRACE_END("dsi_bridge_post_disable");
}

static void dsi_bridge_mode_set(struct drm_bridge *bridge,
				struct drm_display_mode *mode,
				struct drm_display_mode *adjusted_mode)
{
	struct dsi_bridge *c_bridge = to_dsi_bridge(bridge);

	if (!bridge || !mode || !adjusted_mode) {
		pr_err("Invalid params\n");
		return;
	}

	memset(&(c_bridge->dsi_mode), 0x0, sizeof(struct dsi_display_mode));
	convert_to_dsi_mode(adjusted_mode, &(c_bridge->dsi_mode));

	/* restore bit_clk_rate also for dynamic clk use cases */
	c_bridge->dsi_mode.timing.clk_rate_hz =
		dsi_drm_find_bit_clk_rate(c_bridge->display, adjusted_mode);

	pr_debug("clk_rate: %llu\n", c_bridge->dsi_mode.timing.clk_rate_hz);
}

static bool dsi_bridge_mode_fixup(struct drm_bridge *bridge,
				  const struct drm_display_mode *mode,
				  struct drm_display_mode *adjusted_mode)
{
	int rc = 0;
	struct dsi_bridge *c_bridge = to_dsi_bridge(bridge);
	struct dsi_display *display;
	struct dsi_display_mode dsi_mode, cur_dsi_mode, *panel_dsi_mode;
	struct drm_display_mode cur_mode;
	struct drm_crtc_state *crtc_state;

	crtc_state = container_of(mode, struct drm_crtc_state, mode);

	if (!bridge || !mode || !adjusted_mode) {
		pr_err("Invalid params\n");
		return false;
	}

	display = c_bridge->display;
	if (!display) {
		pr_err("Invalid params\n");
		return false;
	}

	/*
	 * if no timing defined in panel, it must be external mode
	 * and we'll use empty priv info to populate the mode
	 */
	if (display->panel && !display->panel->num_timing_nodes) {
		*adjusted_mode = *mode;
		adjusted_mode->private = (int *)&default_priv_info;
		adjusted_mode->private_flags = 0;
		return true;
	}

	convert_to_dsi_mode(mode, &dsi_mode);

	/*
	 * retrieve dsi mode from dsi driver's cache since not safe to take
	 * the drm mode config mutex in all paths
	 */
	rc = dsi_display_find_mode(display, &dsi_mode, &panel_dsi_mode);
	if (rc)
		return rc;

	/* propagate the private info to the adjusted_mode derived dsi mode */
	dsi_mode.priv_info = panel_dsi_mode->priv_info;
	dsi_mode.dsi_mode_flags = panel_dsi_mode->dsi_mode_flags;
	dsi_mode.timing.dsc_enabled = dsi_mode.priv_info->dsc_enabled;
	dsi_mode.timing.dsc = &dsi_mode.priv_info->dsc;

	rc = dsi_display_validate_mode(c_bridge->display, &dsi_mode,
			DSI_VALIDATE_FLAG_ALLOW_ADJUST);
	if (rc) {
		pr_err("[%d] mode is not valid, rc=%d\n", c_bridge->id, rc);
		return false;
	}

	if (bridge->encoder && bridge->encoder->crtc &&
			crtc_state->crtc) {

		convert_to_dsi_mode(&crtc_state->crtc->state->mode,
							&cur_dsi_mode);
		cur_dsi_mode.timing.dsc_enabled =
				dsi_mode.priv_info->dsc_enabled;
		cur_dsi_mode.timing.dsc = &dsi_mode.priv_info->dsc;
		rc = dsi_display_validate_mode_change(c_bridge->display,
					&cur_dsi_mode, &dsi_mode);
		if (rc) {
			pr_err("[%s] seamless mode mismatch failure rc=%d\n",
				c_bridge->display->name, rc);
			return false;
		}

		cur_mode = crtc_state->crtc->mode;

		/* No DMS/VRR when drm pipeline is changing */
		if (!drm_mode_equal(&cur_mode, adjusted_mode) &&
			(!(dsi_mode.dsi_mode_flags & DSI_MODE_FLAG_VRR)) &&
			(!(dsi_mode.dsi_mode_flags & DSI_MODE_FLAG_DYN_CLK)) &&
			(!crtc_state->active_changed ||
			 display->is_cont_splash_enabled))
			dsi_mode.dsi_mode_flags |= DSI_MODE_FLAG_DMS;
	}

	/* convert back to drm mode, propagating the private info & flags */
	dsi_convert_to_drm_mode(&dsi_mode, adjusted_mode);

	return true;
}

u64 dsi_drm_find_bit_clk_rate(void *display,
			      const struct drm_display_mode *drm_mode)
{
	int i = 0, count = 0;
	struct dsi_display *dsi_display = display;
	struct dsi_display_mode *dsi_mode;
	u64 bit_clk_rate = 0;

	if (!dsi_display || !drm_mode)
		return 0;

	dsi_display_get_mode_count(dsi_display, &count);

	for (i = 0; i < count; i++) {
		dsi_mode = &dsi_display->modes[i];
		if ((dsi_mode->timing.v_active == drm_mode->vdisplay) &&
		    (dsi_mode->timing.h_active == drm_mode->hdisplay) &&
		    (dsi_mode->pixel_clk_khz == drm_mode->clock) &&
		    (dsi_mode->timing.refresh_rate == drm_mode->vrefresh)) {
			bit_clk_rate = dsi_mode->timing.clk_rate_hz;
			break;
		}
	}

	return bit_clk_rate;
}

int dsi_conn_get_mode_info(struct drm_connector *connector,
		const struct drm_display_mode *drm_mode,
		struct msm_mode_info *mode_info,
		u32 max_mixer_width, void *display)
{
	struct dsi_display_mode dsi_mode;
	struct dsi_mode_info *timing;

	if (!drm_mode || !mode_info)
		return -EINVAL;

	convert_to_dsi_mode(drm_mode, &dsi_mode);

	if (!dsi_mode.priv_info)
		return -EINVAL;

	memset(mode_info, 0, sizeof(*mode_info));

	timing = &dsi_mode.timing;
	mode_info->frame_rate = dsi_mode.timing.refresh_rate;
	mode_info->vtotal = DSI_V_TOTAL(timing);
	mode_info->prefill_lines = dsi_mode.priv_info->panel_prefill_lines;
	mode_info->jitter_numer = dsi_mode.priv_info->panel_jitter_numer;
	mode_info->jitter_denom = dsi_mode.priv_info->panel_jitter_denom;
	mode_info->clk_rate = dsi_drm_find_bit_clk_rate(display, drm_mode);
	mode_info->mdp_transfer_time_us =
		dsi_mode.priv_info->mdp_transfer_time_us;
	mode_info->overlap_pixels = dsi_mode.priv_info->overlap_pixels;

	memcpy(&mode_info->topology, &dsi_mode.priv_info->topology,
			sizeof(struct msm_display_topology));

	mode_info->comp_info.comp_type = MSM_DISPLAY_COMPRESSION_NONE;
	if (dsi_mode.priv_info->dsc_enabled) {
		mode_info->comp_info.comp_type = MSM_DISPLAY_COMPRESSION_DSC;
		memcpy(&mode_info->comp_info.dsc_info, &dsi_mode.priv_info->dsc,
			sizeof(dsi_mode.priv_info->dsc));
		mode_info->comp_info.comp_ratio =
			MSM_DISPLAY_COMPRESSION_RATIO_3_TO_1;
	}

	if (dsi_mode.priv_info->roi_caps.enabled) {
		memcpy(&mode_info->roi_caps, &dsi_mode.priv_info->roi_caps,
			sizeof(dsi_mode.priv_info->roi_caps));
	}

	return 0;
}

static const struct drm_bridge_funcs dsi_bridge_ops = {
	.attach       = dsi_bridge_attach,
	.mode_fixup   = dsi_bridge_mode_fixup,
	.pre_enable   = dsi_bridge_pre_enable,
	.enable       = dsi_bridge_enable,
	.disable      = dsi_bridge_disable,
	.post_disable = dsi_bridge_post_disable,
	.mode_set     = dsi_bridge_mode_set,
};

int dsi_conn_set_info_blob(struct drm_connector *connector,
		void *info, void *display, struct msm_mode_info *mode_info)
{
	struct dsi_display *dsi_display = display;
	struct dsi_panel *panel;
	enum dsi_pixel_format fmt;
	u32 bpp;

	if (!info || !dsi_display)
		return -EINVAL;

	dsi_display->drm_conn = connector;

	sde_kms_info_add_keystr(info,
		"display type", dsi_display->display_type);

	switch (dsi_display->type) {
	case DSI_DISPLAY_SINGLE:
		sde_kms_info_add_keystr(info, "display config",
					"single display");
		break;
	case DSI_DISPLAY_EXT_BRIDGE:
		sde_kms_info_add_keystr(info, "display config", "ext bridge");
		break;
	case DSI_DISPLAY_SPLIT:
		sde_kms_info_add_keystr(info, "display config",
					"split display");
		break;
	case DSI_DISPLAY_SPLIT_EXT_BRIDGE:
		sde_kms_info_add_keystr(info, "display config",
					"split ext bridge");
		break;
	default:
		pr_debug("invalid display type:%d\n", dsi_display->type);
		break;
	}

	if (!dsi_display->panel) {
		pr_debug("invalid panel data\n");
		goto end;
	}

	panel = dsi_display->panel;
	sde_kms_info_add_keystr(info, "panel name", panel->name);

	switch (panel->panel_mode) {
	case DSI_OP_VIDEO_MODE:
		sde_kms_info_add_keystr(info, "panel mode", "video");
		sde_kms_info_add_keystr(info, "qsync support",
				panel->qsync_min_fps ? "true" : "false");
		break;
	case DSI_OP_CMD_MODE:
		sde_kms_info_add_keystr(info, "panel mode", "command");
		sde_kms_info_add_keyint(info, "mdp_transfer_time_us",
				mode_info->mdp_transfer_time_us);
		sde_kms_info_add_keystr(info, "qsync support",
				panel->qsync_min_fps ? "true" : "false");
		break;
	default:
		pr_debug("invalid panel type:%d\n", panel->panel_mode);
		break;
	}
	sde_kms_info_add_keystr(info, "dfps support",
			panel->dfps_caps.dfps_support ? "true" : "false");

	if (panel->dfps_caps.dfps_support) {
		sde_kms_info_add_keyint(info, "min_fps",
			panel->dfps_caps.min_refresh_rate);
		sde_kms_info_add_keyint(info, "max_fps",
			panel->dfps_caps.max_refresh_rate);
	}

	sde_kms_info_add_keystr(info, "dyn bitclk support",
			panel->dyn_clk_caps.dyn_clk_support ? "true" : "false");

	switch (panel->phy_props.rotation) {
	case DSI_PANEL_ROTATE_NONE:
		sde_kms_info_add_keystr(info, "panel orientation", "none");
		break;
	case DSI_PANEL_ROTATE_H_FLIP:
		sde_kms_info_add_keystr(info, "panel orientation", "horz flip");
		break;
	case DSI_PANEL_ROTATE_V_FLIP:
		sde_kms_info_add_keystr(info, "panel orientation", "vert flip");
		break;
	case DSI_PANEL_ROTATE_HV_FLIP:
		sde_kms_info_add_keystr(info, "panel orientation",
							"horz & vert flip");
		break;
	default:
		pr_debug("invalid panel rotation:%d\n",
						panel->phy_props.rotation);
		break;
	}

	switch (panel->bl_config.type) {
	case DSI_BACKLIGHT_PWM:
		sde_kms_info_add_keystr(info, "backlight type", "pwm");
		break;
	case DSI_BACKLIGHT_WLED:
		sde_kms_info_add_keystr(info, "backlight type", "wled");
		break;
	case DSI_BACKLIGHT_DCS:
		sde_kms_info_add_keystr(info, "backlight type", "dcs");
		break;
	default:
		pr_debug("invalid panel backlight type:%d\n",
						panel->bl_config.type);
		break;
	}

	if (mode_info && mode_info->roi_caps.enabled) {
		sde_kms_info_add_keyint(info, "partial_update_num_roi",
				mode_info->roi_caps.num_roi);
		sde_kms_info_add_keyint(info, "partial_update_xstart",
				mode_info->roi_caps.align.xstart_pix_align);
		sde_kms_info_add_keyint(info, "partial_update_walign",
				mode_info->roi_caps.align.width_pix_align);
		sde_kms_info_add_keyint(info, "partial_update_wmin",
				mode_info->roi_caps.align.min_width);
		sde_kms_info_add_keyint(info, "partial_update_ystart",
				mode_info->roi_caps.align.ystart_pix_align);
		sde_kms_info_add_keyint(info, "partial_update_halign",
				mode_info->roi_caps.align.height_pix_align);
		sde_kms_info_add_keyint(info, "partial_update_hmin",
				mode_info->roi_caps.align.min_height);
		sde_kms_info_add_keyint(info, "partial_update_roimerge",
				mode_info->roi_caps.merge_rois);
	}

	fmt = dsi_display->config.common_config.dst_format;
	bpp = dsi_ctrl_pixel_format_to_bpp(fmt);

	sde_kms_info_add_keyint(info, "bit_depth", bpp);

end:
	return 0;
}

enum drm_connector_status dsi_conn_detect(struct drm_connector *conn,
		bool force,
		void *display)
{
	enum drm_connector_status status = connector_status_unknown;
	struct msm_display_info info;
	int rc;

	if (!conn || !display)
		return status;

	/* get display dsi_info */
	memset(&info, 0x0, sizeof(info));
	rc = dsi_display_get_info(conn, &info, display);
	if (rc) {
		pr_err("failed to get display info, rc=%d\n", rc);
		return connector_status_disconnected;
	}

	if (info.capabilities & MSM_DISPLAY_CAP_HOT_PLUG)
		status = (info.is_connected ? connector_status_connected :
					      connector_status_disconnected);
	else
		status = connector_status_connected;

	conn->display_info.width_mm = info.width_mm;
	conn->display_info.height_mm = info.height_mm;

	return status;
}

void dsi_connector_put_modes(struct drm_connector *connector,
	void *display)
{
	struct drm_display_mode *drm_mode;
	struct dsi_display_mode dsi_mode;
	struct dsi_display *dsi_display;

	if (!connector || !display)
		return;

	list_for_each_entry(drm_mode, &connector->modes, head) {
		convert_to_dsi_mode(drm_mode, &dsi_mode);
		dsi_display_put_mode(display, &dsi_mode);
	}

	/* free the display structure modes also */
	dsi_display = display;
	kfree(dsi_display->modes);
	dsi_display->modes = NULL;
}

int dsi_connector_get_modes(struct drm_connector *connector,
		void *display)
{
	u32 count = 0;
	struct dsi_display_mode *modes = NULL;
	struct drm_display_mode drm_mode;
	int rc, i;

	if (sde_connector_get_panel(connector)) {
		/*
		 * TODO: If drm_panel is attached, query modes from the panel.
		 * This is complicated in split dsi cases because panel is not
		 * attached to both connectors.
		 */
		goto end;
	}
	rc = dsi_display_get_mode_count(display, &count);
	if (rc) {
		pr_err("failed to get num of modes, rc=%d\n", rc);
		goto end;
	}

	rc = dsi_display_get_modes(display, &modes);
	if (rc) {
		pr_err("failed to get modes, rc=%d\n", rc);
		count = 0;
		goto end;
	}

	for (i = 0; i < count; i++) {
		struct drm_display_mode *m;

		memset(&drm_mode, 0x0, sizeof(drm_mode));
		dsi_convert_to_drm_mode(&modes[i], &drm_mode);
		m = drm_mode_duplicate(connector->dev, &drm_mode);
		if (!m) {
			pr_err("failed to add mode %ux%u\n",
			       drm_mode.hdisplay,
			       drm_mode.vdisplay);
			count = -ENOMEM;
			goto end;
		}
		m->width_mm = connector->display_info.width_mm;
		m->height_mm = connector->display_info.height_mm;
		/* set the first mode in list as preferred */
		if (i == 0)
			m->type |= DRM_MODE_TYPE_PREFERRED;
		drm_mode_probed_add(connector, m);
#ifdef CONFIG_DRM_SDE_SPECIFIC_PANEL
		if (modes[i].isDefault)
			drm_set_preferred_mode(
				connector, m->hdisplay, m->vdisplay);
#endif /* CONFIG_DRM_SDE_SPECIFIC_PANEL */
	}
end:
	pr_debug("MODE COUNT =%d\n\n", count);
	return count;
}

enum drm_mode_status dsi_conn_mode_valid(struct drm_connector *connector,
		struct drm_display_mode *mode,
		void *display)
{
	struct dsi_display_mode dsi_mode;
	int rc;

	if (!connector || !mode) {
		pr_err("Invalid params\n");
		return MODE_ERROR;
	}

	convert_to_dsi_mode(mode, &dsi_mode);

	rc = dsi_display_validate_mode(display, &dsi_mode,
			DSI_VALIDATE_FLAG_ALLOW_ADJUST);
	if (rc) {
		pr_err("mode not supported, rc=%d\n", rc);
		return MODE_BAD;
	}

	return MODE_OK;
}

int dsi_conn_pre_kickoff(struct drm_connector *connector,
		void *display,
		struct msm_display_kickoff_params *params)
{
	if (!connector || !display || !params) {
		pr_err("Invalid params\n");
		return -EINVAL;
	}

	return dsi_display_pre_kickoff(connector, display, params);
}

void dsi_conn_enable_event(struct drm_connector *connector,
		uint32_t event_idx, bool enable, void *display)
{
	struct dsi_event_cb_info event_info;

	memset(&event_info, 0, sizeof(event_info));

	event_info.event_cb = sde_connector_trigger_event;
	event_info.event_usr_ptr = connector;

	dsi_display_enable_event(connector, display,
			event_idx, &event_info, enable);
}

int dsi_conn_post_kickoff(struct drm_connector *connector)
{
	struct drm_encoder *encoder;
	struct dsi_bridge *c_bridge;
	struct dsi_display_mode adj_mode;
	struct dsi_display *display;
	struct dsi_display_ctrl *m_ctrl, *ctrl;
	int i, rc = 0;

	if (!connector || !connector->state) {
		pr_err("invalid connector or connector state");
		return -EINVAL;
	}

	encoder = connector->state->best_encoder;
	if (!encoder) {
		pr_debug("best encoder is not available");
		return 0;
	}

	c_bridge = to_dsi_bridge(encoder->bridge);
	adj_mode = c_bridge->dsi_mode;
	display = c_bridge->display;

	if (adj_mode.dsi_mode_flags & DSI_MODE_FLAG_VRR) {
		m_ctrl = &display->ctrl[display->clk_master_idx];
		rc = dsi_ctrl_timing_db_update(m_ctrl->ctrl, false);
		if (rc) {
			pr_err("[%s] failed to dfps update  rc=%d\n",
				display->name, rc);
			return -EINVAL;
		}

		/* Update the rest of the controllers */
		display_for_each_ctrl(i, display) {
			ctrl = &display->ctrl[i];
			if (!ctrl->ctrl || (ctrl == m_ctrl))
				continue;

			rc = dsi_ctrl_timing_db_update(ctrl->ctrl, false);
			if (rc) {
				pr_err("[%s] failed to dfps update rc=%d\n",
					display->name,  rc);
				return -EINVAL;
			}
		}

		c_bridge->dsi_mode.dsi_mode_flags &= ~DSI_MODE_FLAG_VRR;
	}

	/* ensure dynamic clk switch flag is reset */
	c_bridge->dsi_mode.dsi_mode_flags &= ~DSI_MODE_FLAG_DYN_CLK;

	return 0;
}

struct dsi_bridge *dsi_drm_bridge_init(struct dsi_display *display,
				       struct drm_device *dev,
				       struct drm_encoder *encoder)
{
	int rc = 0;
	struct dsi_bridge *bridge;

	bridge = kzalloc(sizeof(*bridge), GFP_KERNEL);
	if (!bridge) {
		rc = -ENOMEM;
		goto error;
	}

	bridge->display = display;
	bridge->base.funcs = &dsi_bridge_ops;
	bridge->base.encoder = encoder;

	rc = drm_bridge_attach(encoder, &bridge->base, NULL);
	if (rc) {
		pr_err("failed to attach bridge, rc=%d\n", rc);
		goto error_free_bridge;
	}

	encoder->bridge = &bridge->base;
	return bridge;
error_free_bridge:
	kfree(bridge);
error:
	return ERR_PTR(rc);
}

void dsi_drm_bridge_cleanup(struct dsi_bridge *bridge)
{
	if (bridge && bridge->base.encoder)
		bridge->base.encoder->bridge = NULL;

	kfree(bridge);
}
