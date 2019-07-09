/*
 * Restructured unified display panel driver for Xperia Open Devices
 *                     *** FPS Management ***
 *
 * This driver is based on various SoMC implementations found in
 * copyleft archives for various devices.
 *
 * Copyright (c) 2012-2014, The Linux Foundation. All rights reserved.
 * Copyright (c) Sony Mobile Communications Inc. All rights reserved.
 * Copyright (C) 2014-2017, AngeloGioacchino Del Regno <kholk11@gmail.com>
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
#include <linux/of_platform.h>

#include "../mdss_mdp.h"
#include "../mdss_dsi.h"
#include "somc_panels.h"

#define PSEC ((u64)1000000000000)
#define KSEC ((u64)1000)

static struct fps_data fpsd, vpsd;
static struct mdss_mdp_vsync_handler vs_handle;

int somc_panel_parse_dt_chgfps_config(struct device_node *np,
			struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	struct mdss_panel_info *pinfo = &(ctrl_pdata->panel_data.panel_info);
	struct change_fps *chg_fps = &ctrl_pdata->spec_pdata->chg_fps;
	static const char *fps_mode;
	static const char *fps_type;
	int rc;
	u32 tmp;

	chg_fps->enable = of_property_read_bool(np,
					"somc,change-fps-enable");
	if (!chg_fps->enable)
		return 0;

	chg_fps->input_fpks = pinfo->mipi.frame_rate * 1000;
	somc_panel_parse_dcs_cmds(np, &ctrl_pdata->spec_pdata->fps_cmds,
				"somc,change-fps-command", NULL);

	rc = of_property_read_u32(np,
				"somc,driver-ic-vdisp", &tmp);
	if (rc) {
		pr_err("%s: Display vdisp not specified\n", __func__);
		goto error;
	}
	chg_fps->dric_vdisp = tmp;

	fps_type = of_get_property(np,
				"somc,change-fps-panel-type", NULL);
	if (!fps_type) {
		pr_err("%s:%d, Panel type not specified\n",
						__func__, __LINE__);
		goto error;
	}

	if (!strncmp(fps_type, "uhd_4k_type", 11)) {
		chg_fps->type = FPS_TYPE_UHD_4K;
	} else if (!strncmp(fps_type, "hybrid_incell_type", 18)) {
		chg_fps->type = FPS_TYPE_HYBRID_INCELL;
	} else if (!strncmp(fps_type, "full_incell_type", 16)) {
		chg_fps->type = FPS_TYPE_FULL_INCELL;
	} else {
		pr_err("%s: Unable to read fps panel type\n", __func__);
		goto error;
	}

	fps_mode = of_get_property(np,
				"somc,change-fps-panel-mode", NULL);
	if (!fps_mode) {
		pr_err("%s:%d, Panel mode not specified\n",
						__func__, __LINE__);
		goto error;
	}

	if (!strncmp(fps_mode, "susres_mode", 11)) {
		chg_fps->mode = FPS_MODE_SUSRES;
	} else if (!strncmp(fps_mode, "dynamic_mode", 12)) {
		chg_fps->mode = FPS_MODE_DYNAMIC;
	} else {
		pr_err("%s: Unable to read fps panel mode\n", __func__);
		goto error;
	}

	switch (chg_fps->type) {
	case FPS_TYPE_UHD_4K:
		(void)mdss_dsi_property_read_u32_var(np,
			"somc,change-fps-rtn-pos",
			(u32 **)&chg_fps->send_pos.pos,
			&chg_fps->send_pos.num);

			rc = of_property_read_u32(np,
				"somc,driver-ic-total-porch", &tmp);
		if (rc) {
			pr_err("%s: DrIC total_porch not specified\n",
							__func__);
			goto error;
		}
		chg_fps->dric_total_porch = tmp;

		rc = of_property_read_u32(np,
					"somc,driver-ic-rclk", &tmp);
		if (rc) {
			pr_err("%s: DrIC rclk not specified\n",
							__func__);
			goto error;
		}
		chg_fps->dric_rclk = tmp;

		chg_fps->rtn_adj = of_property_read_bool(np,
				"somc,change-fps-rtn-adj");
		break;
	case FPS_TYPE_HYBRID_INCELL:
		(void)mdss_dsi_property_read_u32_var(np,
			"somc,change-fps-send-pos",
			(u32 **)&chg_fps->send_pos.pos,
			&chg_fps->send_pos.num);

		rc = of_property_read_u32(np,
				"somc,driver-ic-rtn",  &tmp);
		if (rc) {
			pr_err("%s: DrIC rtn not specified\n",
							__func__);
			goto error;
		}
		chg_fps->dric_rtn = tmp;

		(void)mdss_dsi_property_read_u32_var(np,
			"somc,change-fps-send-pos",
			(u32 **)&chg_fps->send_pos.pos,
			&chg_fps->send_pos.num);

		rc = of_property_read_u32(np,
					"somc,driver-ic-mclk", &tmp);
		if (rc) {
			pr_err("%s: DrIC mclk not specified\n",
				__func__);
			goto error;
		}
		chg_fps->dric_mclk = tmp;

		rc = of_property_read_u32(np,
					"somc,driver-ic-vtouch", &tmp);
		if (rc) {
			pr_err("%s: DrIC vtouch not specified\n",
				__func__);
			goto error;
		}
		chg_fps->dric_vtouch = tmp;

		rc = of_property_read_u32(np,
				"somc,change-fps-send-byte", &tmp);
		if (rc) {
			pr_err("%s: fps bytes send not specified\n",
							__func__);
			goto error;
		}
		chg_fps->send_byte = tmp;

		rc = of_property_read_u32(np,
			"somc,change-fps-porch-mask-pos", &tmp);
		if (rc) {
			pr_warn("%s: fps mask position not specified\n",
							__func__);
			chg_fps->mask_pos = 0;
		} else {
			chg_fps->mask_pos = tmp;
			rc = of_property_read_u32(np,
				"somc,change-fps-porch-mask", &tmp);
			if (rc) {
				pr_warn("%s: fps mask not specified\n",
							__func__);
				chg_fps->mask = 0x0;
			} else {
				chg_fps->mask = tmp;
			}
		}
		break;
	case FPS_TYPE_FULL_INCELL:
		(void)mdss_dsi_property_read_u32_var(np,
			"somc,change-fps-rtn-pos",
			(u32 **)&chg_fps->send_pos.pos,
			&chg_fps->send_pos.num);

		rc = of_property_read_u32(np,
				"somc,driver-ic-total-porch", &tmp);
		if (rc) {
			pr_err("%s: DrIC total_porch not specified\n",
							__func__);
			goto error;
		}
		chg_fps->dric_total_porch = tmp;

		rc = of_property_read_u32(np,
					"somc,driver-ic-rclk", &tmp);
		if (rc) {
			pr_err("%s: DrIC rclk not specified\n",
				__func__);
			goto error;
		}
		chg_fps->dric_rclk = tmp;

		rc = of_property_read_u32(np,
					"somc,driver-ic-vtp", &tmp);
		if (rc) {
			pr_err("%s: DrIC vtp not specified\n",
				__func__);
			goto error;
		}
		chg_fps->dric_tp = tmp;
		break;
	default:
		pr_err("%s: Read panel mode failed.\n", __func__);
		goto error;
	}
error:
	return rc;
}

static u32 ts_diff_ms(struct timespec lhs, struct timespec rhs)
{
	struct timespec tdiff;
	s64 nsec;
	u32 msec;

	tdiff = timespec_sub(lhs, rhs);
	nsec = timespec_to_ns(&tdiff);
	msec = (u32)nsec;
	do_div(msec, NSEC_PER_MSEC);

	return msec;
}

static struct fps_data *somc_panel_get_fps_address(fps_type type)
{
	switch (type) {
	case FPSD:
		return &fpsd;
	case VPSD:
		return &vpsd;
	default:
		pr_err("%s: select Failed!\n", __func__);
		return NULL;
	}
}

static void update_fps_data(struct fps_data *fps)
{
	if (mutex_trylock(&fps->fps_lock)) {
		u32 fpks = 0;
		u32 ms_since_last = 0;
		u32 num_frames;
		struct timespec tlast = fps->timestamp_last;
		struct timespec tnow;
		u32 msec;

		getrawmonotonic(&tnow);
		msec = ts_diff_ms(tnow, tlast);
		fps->timestamp_last = tnow;

		fps->interval_ms = msec;
		fps->frame_counter++;
		num_frames = fps->frame_counter - fps->frame_counter_last;

		fps->fa[fps->fps_array_cnt].frame_nbr = fps->frame_counter;
		fps->fa[fps->fps_array_cnt].time_delta = msec;
		fps->fa_last_array_pos = fps->fps_array_cnt;
		(fps->fps_array_cnt)++;
		if (fps->fps_array_cnt >= DEF_FPS_ARRAY_SIZE)
			fps->fps_array_cnt = 0;

		ms_since_last = ts_diff_ms(tnow, fps->fpks_ts_last);

		if (num_frames > 1 && ms_since_last >= fps->log_interval) {
			fpks = (num_frames * 1000000) / ms_since_last;
			fps->fpks_ts_last = tnow;
			fps->frame_counter_last = fps->frame_counter;
			fps->fpks = fpks;
		}
		mutex_unlock(&fps->fps_lock);
	}
}

static void somc_panel_fps_data_init(fps_type type)
{
	struct fps_data *fps = somc_panel_get_fps_address(type);

	if (!fps) {
		pr_err("%s: select Failed!\n", __func__);
		return;
	}

	fps->frame_counter = 0;
	fps->frame_counter_last = 0;
	fps->log_interval = DEF_FPS_LOG_INTERVAL;
	fps->fpks = 0;
	fps->fa_last_array_pos = 0;
	fps->vps_en = false;
	getrawmonotonic(&fps->timestamp_last);
	mutex_init(&fps->fps_lock);
}

static void somc_panel_fps_data_update(
		struct msm_fb_data_type *mfd, fps_type type)
{
	struct fps_data *fps = somc_panel_get_fps_address(type);

	if (!fps) {
		pr_err("%s: select Failed!\n", __func__);
		return;
	}

	if (mfd->index == 0)
		update_fps_data(fps);
}

void somc_panel_fpsd_data_update(struct msm_fb_data_type *mfd)
{
	somc_panel_fps_data_update(mfd, FPSD);
}

struct fps_data somc_panel_get_fps_data(void)
{
	return fpsd;
}

struct fps_data somc_panel_get_vps_data(void)
{
	return vpsd;
}

static void vsync_handler(struct mdss_mdp_ctl *ctl, ktime_t t)
{
	struct msm_fb_data_type *mfd = ctl->mfd;

	somc_panel_fps_data_update(mfd, VPSD);
}

static void somc_panel_fps_cmd_send(
		struct mdss_dsi_ctrl_pdata *ctrl_pdata,
		u32 dfpks_rev, int dfpks) {
	char dfps = (char)(dfpks_rev / KSEC);
	struct mdss_panel_specific_pdata *spec_pdata = ctrl_pdata->spec_pdata;
	struct mdss_panel_info *pinfo = &ctrl_pdata->panel_data.panel_info;

	pinfo->mipi.frame_rate = dfps;

	if (spec_pdata->chg_fps.mode != FPS_MODE_SUSRES) {
		pr_debug("%s: fps change sequence\n", __func__);
		mdss_dsi_panel_cmds_send(ctrl_pdata,
				&ctrl_pdata->spec_pdata->fps_cmds);
	}
	pr_notice("%s: change fpks=%d\n", __func__, dfpks);

	pinfo->new_fps		= dfps;
	spec_pdata->chg_fps.input_fpks	= dfpks;
}

static int somc_panel_fps_calc_rtn(
		struct mdss_dsi_ctrl_pdata *ctrl_pdata, int dfpks) {
	u32 dfpks_rev;
	u32 vtotal_porch, vdisp;
	u32 vrclk, vtp;
	u32 cmds, payload;
	struct mdss_panel_specific_pdata *spec_pdata = ctrl_pdata->spec_pdata;
	u16 rtn;
	int i, j, byte_cnt;
	char send_rtn[sizeof(u16)] = {0};

	vtotal_porch = spec_pdata->chg_fps.dric_total_porch;
	vdisp = spec_pdata->chg_fps.dric_vdisp;

	vrclk = spec_pdata->chg_fps.dric_rclk;
	vtp = spec_pdata->chg_fps.dric_tp;

	if (!dfpks || !(vdisp + vtotal_porch + vtp)) {
		pr_err("%s: Invalid param dfpks=%d vdisp=%d porch=%d vtp=%d\n",
				__func__, dfpks, vdisp, vtotal_porch, vtp);
		return -EINVAL;
	}

	rtn = (u16)(
		(vrclk * KSEC) /
		(dfpks * (vdisp + vtotal_porch + vtp))
		);

	if (!rtn || !(vdisp + vtotal_porch + vtp)) {
		pr_err("%s: Invalid param rtn=%d vdisp=%d porch=%d vtp=%d\n",
				__func__, dfpks, vdisp, vtotal_porch, vtp);
		return -EINVAL;
	}

	dfpks_rev = (u32)(
		(vrclk * KSEC) /
		(rtn * (vdisp + vtotal_porch + vtp))
		);

	pr_debug("%s: porch=%d vdisp=%d vtp=%d vrclk=%d rtn=0x%x\n",
		__func__, vtotal_porch, vdisp, vtp, vrclk, rtn);

	for (i = 0; i < sizeof(send_rtn) ; i++) {
		send_rtn[i] = (char)(rtn & 0x00FF);
		pr_debug("%s: send_rtn[%d]=0x%x\n",
				__func__, i, send_rtn[i]);
		if (rtn > 0xFF) {
			rtn = (rtn >> 8);
		} else {
			byte_cnt = i;
			break;
		}
	}

	for (i = 0; i < (spec_pdata->chg_fps.send_pos.num / 2); i++) {
		cmds = spec_pdata->chg_fps.send_pos.pos[(i * 2)];
		payload = spec_pdata->chg_fps.send_pos.pos[(i * 2) + 1];
		for (j = 0; j <= byte_cnt ; j++)
			CHANGE_PAYLOAD(cmds, payload + j) =
				send_rtn[byte_cnt - j];
	}

	somc_panel_fps_cmd_send(ctrl_pdata, dfpks_rev, dfpks);

	return 0;
}

static int somc_panel_fps_calc_porch
		(struct mdss_dsi_ctrl_pdata *ctrl_pdata, int dfpks) {
	u64 vmclk;
	u64 vtouch;
	u32 dfpks_rev;
	u32 vdisp;
	u32 cmds, payload;
	int i, j;
	u16 porch_calc = 0;
	u16 send_byte;
	u16 rtn;
	u8 mask_pos;
	char mask;
	char porch[CHANGE_FPS_PORCH] = {0};
	char send[CHANGE_FPS_SEND] = {0};
	struct mdss_panel_specific_pdata *spec_pdata = ctrl_pdata->spec_pdata;
	struct dsi_panel_cmds *fps_cmds = &(spec_pdata->fps_cmds);

	rtn = spec_pdata->chg_fps.dric_rtn;
	vdisp = spec_pdata->chg_fps.dric_vdisp;
	vtouch = spec_pdata->chg_fps.dric_vtouch;
	vmclk = (u64)spec_pdata->chg_fps.dric_mclk;
	send_byte = spec_pdata->chg_fps.send_byte;
	mask_pos = spec_pdata->chg_fps.mask_pos;
	mask = spec_pdata->chg_fps.mask;

	if (!dfpks || !vmclk || !rtn) {
		pr_err("%s: Invalid param dfpks=%d vmclk=%llu rtn%d\n",
				__func__, dfpks, vmclk, rtn);
		return -EINVAL;
	}

	porch_calc = (u16)((
			(((PSEC * KSEC) - (KSEC * vtouch * (u64)dfpks)) /
			((u64)dfpks * vmclk * (u64)rtn)) - (u64)vdisp) / 2);

	if (!(vmclk * rtn * (vdisp + porch_calc) + vtouch)) {
		pr_err("%s: Invalid param \
			vmclk=%llu rtn=%d vdisp=%d porch=%d vtouch=%llu\n",
			__func__, vmclk, rtn, vdisp, porch_calc, vtouch);
		return -EINVAL;
	}

	dfpks_rev = (u32)(
		(PSEC * KSEC) /
		(vmclk * rtn * (vdisp + porch_calc) + vtouch));

	pr_debug("%s: porch=%d vdisp=%d vtouch=%llu vmclk=%llu rtn=0x%x\n",
		__func__, porch_calc, vdisp, vtouch, vmclk, rtn);

	for (i = 0; i < CHANGE_FPS_PORCH ; i++) {
		porch[i] = (char)(porch_calc & 0x00FF);
		pr_debug("%s: porch[%d]=0x%x\n", __func__, i, porch[i]);
		porch_calc = (porch_calc >> 8);
	}

	for (i = 0; i < send_byte; i = i + 2) {
		memcpy(send + i, porch, sizeof(char));
		memcpy(send + i + 1, porch + 1, sizeof(char));
	}

	for (i = 0; i < (spec_pdata->chg_fps.send_pos.num / 2); i++) {
		cmds = spec_pdata->chg_fps.send_pos.pos[(i * 2)];
		payload = spec_pdata->chg_fps.send_pos.pos[(i * 2) + 1];
		for (j = 0; j < send_byte ; j++) {
			if (j == mask_pos)
				send[j] = (mask | send[j]);
			CHANGE_PAYLOAD(cmds, payload + j) = send[j];
			pr_debug("%s: fps_cmds.cmds[%d].payload[%d]) = 0x%x\n",
				__func__,
				cmds, payload + j,
				fps_cmds->cmds[cmds].payload[payload + j]);
		}
	}

	somc_panel_fps_cmd_send(ctrl_pdata, dfpks_rev, dfpks);

	return 0;
}

static int somc_panel_fps_calc_adj
		(struct mdss_dsi_ctrl_pdata *ctrl_pdata, int dfpks) {
	u32 dfpks_rev;
	u32 vtotal_porch, vdisp, vrclk;
	u32 cmds, payload;
	struct mdss_panel_specific_pdata *spec_pdata = ctrl_pdata->spec_pdata;
	u16 rtn;
	int i, j, byte_cnt;
	char send_rtn[sizeof(u16)] = {0}, adj;

	if (!spec_pdata) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	vtotal_porch = spec_pdata->chg_fps.dric_total_porch;
	vdisp = spec_pdata->chg_fps.dric_vdisp;
	vrclk = spec_pdata->chg_fps.dric_rclk;
	adj = spec_pdata->chg_fps.rtn_adj ? 1 : 0;

	if (!dfpks || !(vdisp + vtotal_porch)) {
		pr_err("%s: Invalid param dfpks=%d vdisp=%d porch=%d\n",
				__func__, dfpks, vdisp, vtotal_porch);
		return -EINVAL;
	}

	rtn = (u16)(vrclk / (dfpks * (vdisp + vtotal_porch) / 1000)) - adj;

	if (!rtn || !(vdisp + vtotal_porch)) {
		pr_err("%s: Invalid param rtn=%d vdisp=%d　porch=%d\n",
				__func__, rtn, vdisp, vtotal_porch);
		return -EINVAL;
	}

	dfpks_rev = (u32)(vrclk / (rtn * (vdisp + vtotal_porch) / 1000));

	pr_debug("%s: porch=%d vdisp=%d vrclk=%d rtn=0x%x adj=%d\n",
		__func__, vtotal_porch, vdisp, vrclk, rtn + adj, adj);

	for (i = 0; i < sizeof(send_rtn) ; i++) {
		send_rtn[i] = (char)(rtn & 0x00FF);
		pr_debug("%s: send_rtn[%d]=0x%x\n",
				__func__, i, send_rtn[i]);
		if (rtn > 0xFF) {
			rtn = (rtn >> 8);
		} else {
			byte_cnt = i;
			break;
		}
	}

	for (i = 0; i < (spec_pdata->chg_fps.send_pos.num / 2); i++) {
		cmds = spec_pdata->chg_fps.send_pos.pos[(i * 2)];
		payload = spec_pdata->chg_fps.send_pos.pos[(i * 2) + 1];
		for (j = 0; j <= byte_cnt ; j++)
			CHANGE_PAYLOAD(cmds, payload + j) =
				send_rtn[byte_cnt - j];
	}

	somc_panel_fps_cmd_send(ctrl_pdata, dfpks_rev, dfpks);

	return 0;
}

static int somc_panel_chg_fps_calc
		(struct mdss_dsi_ctrl_pdata *ctrl_pdata, int dfpks) {
	int ret = -EINVAL;
	struct mdss_panel_specific_pdata *spec_pdata = NULL;

	spec_pdata = ctrl_pdata->spec_pdata;
	if (!spec_pdata) {
		pr_err("%s: Invalid input data\n", __func__);
		return ret;
	}

	switch (spec_pdata->chg_fps.type) {
	case FPS_TYPE_UHD_4K:
		ret = somc_panel_fps_calc_adj(ctrl_pdata, dfpks);
		break;
	case FPS_TYPE_HYBRID_INCELL:
		ret = somc_panel_fps_calc_porch(ctrl_pdata, dfpks);
		break;
	case FPS_TYPE_FULL_INCELL:
		ret = somc_panel_fps_calc_rtn(ctrl_pdata, dfpks);
		break;
	default:
		pr_err("%s: Invalid type data\n", __func__);
		break;
	}

	return ret;
}

static int mdss_dsi_panel_chg_fps_check_state
		(struct mdss_dsi_ctrl_pdata *ctrl, int dfpks) {
	struct mdss_data_type *mdata = mdss_mdp_get_mdata();
	struct msm_fb_data_type *mfd = mdata->ctl_off->mfd;
	struct mdss_overlay_private *mdp5_data = mfd_to_mdp5_data(mfd);
	struct mdss_panel_info *pinfo = &ctrl->panel_data.panel_info;
	struct mdss_dsi_ctrl_pdata *sctrl = NULL;
	struct mdss_panel_specific_pdata *specific = NULL;
	int rc = 0;

	if (!mdp5_data->ctl || !mdp5_data->ctl->power_state)
		goto error;

	if ((pinfo->mipi.mode == DSI_CMD_MODE) &&
			(!ctrl->spec_pdata->fps_cmds.cmd_cnt))
		goto cmd_cnt_err;

	specific = ctrl->spec_pdata;

	if (!specific->disp_onoff_state)
		goto disp_onoff_state_err;

	if (mdss_dsi_sync_wait_enable(ctrl)) {
		sctrl = mdss_dsi_get_other_ctrl(ctrl);
		if (sctrl) {
			if (mdss_dsi_sync_wait_trigger(ctrl)) {
				rc = somc_panel_chg_fps_calc(sctrl, dfpks);
				if (rc < 0)
					goto end;
				rc = somc_panel_chg_fps_calc(ctrl, dfpks);
			} else {
				rc = somc_panel_chg_fps_calc(ctrl, dfpks);
				if (rc < 0)
					goto end;
				rc = somc_panel_chg_fps_calc(sctrl, dfpks);
			}
		} else {
			rc = somc_panel_chg_fps_calc(ctrl, dfpks);
		}
	} else {
		rc = somc_panel_chg_fps_calc(ctrl, dfpks);
	}
end:
	return rc;
cmd_cnt_err:
	pr_err("%s: change fps isn't supported\n", __func__);
	return -EINVAL;
disp_onoff_state_err:
	pr_err("%s: Disp-On is not yet completed. Please retry\n", __func__);
	return -EINVAL;
error:
	return -EINVAL;
}

ssize_t somc_panel_change_fpks_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = dev_get_drvdata(dev);
	int dfpks, rc;

	if (!ctrl_pdata || !ctrl_pdata->spec_pdata) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	if (!ctrl_pdata->spec_pdata->chg_fps.enable) {
		pr_err("%s: change fps not enabled\n", __func__);
		return -EINVAL;
	}

	rc = kstrtoint(buf, 10, &dfpks);
	if (rc < 0) {
		pr_err("%s: Error, buf = %s\n", __func__, buf);
		return rc;
	}

	if (dfpks < 1000 * CHANGE_FPS_MIN
			|| dfpks > 1000 * CHANGE_FPS_MAX) {
		pr_err("%s: invalid value for change_fpks buf = %s\n",
				 __func__, buf);
		return -EINVAL;
	}

	if (dfpks == ctrl_pdata->spec_pdata->chg_fps.input_fpks) {
		pr_notice("%s: fpks is already %d\n", __func__, dfpks);
		return count;
	}

	rc = mdss_dsi_panel_chg_fps_check_state(ctrl_pdata, dfpks);
	if (rc) {
		pr_err("%s: Error, rc = %d\n", __func__, rc);
		return rc;
	}
	return count;
}

ssize_t somc_panel_change_fpks_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = dev_get_drvdata(dev);
	struct mdss_data_type *mdata = mdss_mdp_get_mdata();
	struct msm_fb_data_type *mfd = mdata->ctl_off->mfd;
	struct mdss_overlay_private *mdp5_data = mfd_to_mdp5_data(mfd);

	if (!mdp5_data->ctl || !mdp5_data->ctl->power_state)
		return 0;

	return scnprintf(buf, PAGE_SIZE, "%d\n",
		ctrl_pdata->spec_pdata->chg_fps.input_fpks);
}

ssize_t somc_panel_change_fps_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = dev_get_drvdata(dev);
	int dfps, dfpks, rc;

	if (!ctrl_pdata || !ctrl_pdata->spec_pdata) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	if (!ctrl_pdata->spec_pdata->chg_fps.enable) {
		pr_err("%s: change fps not enabled\n", __func__);
		return -EINVAL;
	}

	rc = kstrtoint(buf, 10, &dfps);
	if (rc < 0) {
		pr_err("%s: Error, buf = %s\n", __func__, buf);
		return rc;
	}

	if (dfps >= 1000 * CHANGE_FPS_MIN
			&& dfps <= 1000 * CHANGE_FPS_MAX) {
		dfpks = dfps;
	} else if (dfps >= CHANGE_FPS_MIN && dfps <= CHANGE_FPS_MAX) {
		dfpks = dfps * 1000;
	} else {
		pr_err("%s: invalid value for change_fps buf = %s\n",
				__func__, buf);
		return -EINVAL;
	}

	if (dfpks == ctrl_pdata->spec_pdata->chg_fps.input_fpks) {
		pr_notice("%s: fpks is already %d\n", __func__, dfpks);
		return count;
	}

	rc = mdss_dsi_panel_chg_fps_check_state(ctrl_pdata, dfpks);
	if (rc) {
		pr_err("%s: Error, rc = %d\n", __func__, rc);
		return rc;
	}
	return count;
}

ssize_t somc_panel_change_fps_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = dev_get_drvdata(dev);
	struct mdss_data_type *mdata = mdss_mdp_get_mdata();
	struct msm_fb_data_type *mfd = mdata->ctl_off->mfd;
	struct mdss_overlay_private *mdp5_data = mfd_to_mdp5_data(mfd);

	if (!mdp5_data->ctl || !mdp5_data->ctl->power_state)
		return 0;

	return scnprintf(buf, PAGE_SIZE, "%d\n",
		ctrl_pdata->spec_pdata->chg_fps.input_fpks / 1000);
}

void somc_panel_chg_fps_cmds_send(struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	struct mdss_panel_specific_pdata *spec_pdata = NULL;
	u32 fps_cmds, fps_payload;
	char rtn;

	spec_pdata = ctrl_pdata->spec_pdata;

	if (ctrl_pdata->panel_data.panel_info.mipi.mode == DSI_CMD_MODE) {
		if (spec_pdata->fps_cmds.cmd_cnt) {
			fps_cmds = spec_pdata->chg_fps.send_pos.pos[0];
			fps_payload = spec_pdata->chg_fps.send_pos.pos[1];
			rtn = CHANGE_PAYLOAD(fps_cmds, fps_payload);
			pr_debug("%s: change fps sequence --- rtn = 0x%x\n",
				__func__, rtn);
			mdss_dsi_panel_cmds_send(ctrl_pdata,
				&ctrl_pdata->spec_pdata->fps_cmds);
		}
	}
}

static ssize_t somc_panel_vsyncs_per_ksecs_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int ret = count;
	long vps_en;
	struct mdss_data_type *mdata = mdss_mdp_get_mdata();
	struct mdss_mdp_ctl *ctl = mdata->ctl_off;

	if (kstrtol(buf, 10, &vps_en)) {
		dev_err(dev, "%s: Error, buf = %s\n", __func__, buf);
		ret = -EINVAL;
		goto exit;
	}

	vs_handle.vsync_handler = (mdp_vsync_handler_t)vsync_handler;
	vs_handle.cmd_post_flush = false;

	if (vps_en) {
		vs_handle.enabled = false;
		if (!vpsd.vps_en && (ctl->ops.add_vsync_handler)) {
			ctl->ops.add_vsync_handler(ctl, &vs_handle);
			vpsd.vps_en = true;
			pr_notice("%s: vsyncs_per_ksecs is valid\n", __func__);
		}
	} else {
		vs_handle.enabled = true;
		if (vpsd.vps_en && (ctl->ops.remove_vsync_handler)) {
			ctl->ops.remove_vsync_handler(ctl, &vs_handle);
			vpsd.vps_en = false;
			fpsd.fpks = 0;
			pr_notice("%s: vsyncs_per_ksecs is invalid\n", __func__);
		}
	}
exit:
	return ret;
}

static ssize_t somc_panel_vsyncs_per_ksecs_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	if (vpsd.vps_en)
		return scnprintf(buf, PAGE_SIZE, "%i\n", vpsd.fpks);
	else
		return scnprintf(buf, PAGE_SIZE,
		"This function is invalid now.\n"
		"Please read again after writing ON.\n");
}

static ssize_t somc_panel_frame_counter(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%i\n", fpsd.frame_counter);
}

static ssize_t somc_panel_frames_per_ksecs(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%i\n", fpsd.fpks);
}

static struct device_attribute panel_fps_attributes[] = {
	__ATTR(frame_counter, S_IRUGO, somc_panel_frame_counter, NULL),
	__ATTR(frames_per_ksecs, S_IRUGO,
				somc_panel_frames_per_ksecs, NULL),
	__ATTR(vsyncs_per_ksecs, S_IRUSR|S_IRGRP|S_IWUSR|S_IWGRP,
				somc_panel_vsyncs_per_ksecs_show,
				somc_panel_vsyncs_per_ksecs_store),
	__ATTR(change_fps, S_IRUGO|S_IWUSR|S_IWGRP,
					somc_panel_change_fps_show,
					somc_panel_change_fps_store),
	__ATTR(change_fpks, S_IRUGO|S_IWUSR|S_IWGRP,
					somc_panel_change_fpks_show,
					somc_panel_change_fpks_store),
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

void somc_panel_fpsman_panel_off(void)
{
	struct mdss_data_type *mdata = mdss_mdp_get_mdata();
	struct mdss_mdp_ctl *ctl = mdata->ctl_off;

	vs_handle.vsync_handler = (mdp_vsync_handler_t)vsync_handler;
	vs_handle.cmd_post_flush = false;
	vs_handle.enabled = true;
	if (vpsd.vps_en && (ctl->ops.remove_vsync_handler)) {
		ctl->ops.remove_vsync_handler(ctl, &vs_handle);
		vpsd.vps_en = false;
		fpsd.fpks = 0;
		pr_notice("%s: vsyncs_per_ksecs is invalid\n", __func__);
	}
}

void somc_panel_fpsman_refresh(struct mdss_dsi_ctrl_pdata *ctrl,
		bool immediate_refresh)
{
	struct change_fps *chg_fps = &ctrl->spec_pdata->chg_fps;
	struct mdss_panel_info *pinfo = &ctrl->panel_data.panel_info;

	if (!chg_fps->enable) {
		pr_debug("%s: change fps not supported\n", __func__);
		return;
	}

	chg_fps->input_fpks = pinfo->mipi.frame_rate * 1000;

	if (immediate_refresh)
		somc_panel_chg_fps_calc(ctrl, chg_fps->input_fpks);
}

void somc_panel_fpsman_panel_post_on(struct mdss_dsi_ctrl_pdata *ctrl)
{
	struct change_fps *chg_fps = &ctrl->spec_pdata->chg_fps;

	if (chg_fps->enable) {
		if (chg_fps->mode == FPS_MODE_SUSRES)
			somc_panel_chg_fps_calc(ctrl, chg_fps->input_fpks);

		somc_panel_chg_fps_cmds_send(ctrl);
	} else {
		pr_notice("%s: change fps not supported.\n", __func__);
	}

	return;
}

int somc_panel_fps_manager_init(void)
{
	somc_panel_fps_data_init(FPSD);
	somc_panel_fps_data_init(VPSD);

	vs_handle.vsync_handler = NULL;

	return 0;
}
