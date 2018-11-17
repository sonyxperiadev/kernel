/*
 * Restructured unified display panel driver for Xperia Open Devices
 *                    *** Color Management ***
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

#undef pr_fmt
#define pr_fmt(fmt) "somc_panel_color_manager: " fmt

#define PA_V2_BASIC_FEAT_ENB (PA_HSIC_HUE_ENABLE | PA_HSIC_SAT_ENABLE | \
			      PA_HSIC_VAL_ENABLE | PA_HSIC_CONT_ENABLE)

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

static void get_uv_data(struct dsi_display *display, int *u_data, int *v_data)
{
	struct panel_specific_pdata *spec_pdata = NULL;
	struct somc_panel_color_mgr *color_mgr = NULL;
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
	color_mgr = spec_pdata->color_mgr;
	param_type = color_mgr->standard_pcc_data.param_type;

	rbuf = kcalloc(size_rbuf, sizeof(char), GFP_KERNEL);

	len = get_uv_param_len(param_type, &short_response);
	rlen = short_response ? 1 : len;

	for (i = 0; i < color_mgr->uv_read_cmds.cmd_cnt; i++) {
		memset(&cmdreq, 0, sizeof(cmdreq));
		cmdreq.msg.type = DTYPE_DCS_READ;
		cmdreq.last_command = true;
		cmdreq.msg.channel = 0;
		cmdreq.msg.flags = 0;
		cmdreq.msg.ctrl = 0;
		cmdreq.post_wait_ms = 5;
		cmdreq.msg.tx_len = len;
		cmdreq.msg.tx_buf =
			*((color_mgr->uv_read_cmds.cmds.payload) + i);
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

int somc_panel_parse_dt_colormgr_config(struct dsi_panel *panel,
			struct device_node *np)
{
	struct somc_panel_color_mgr *color_mgr = NULL;
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
	color_mgr->dsi_pcc_applied = false;
	color_mgr->standard_pcc_enable =
		of_property_read_bool(np, "somc,mdss-dsi-pcc-enable");

	if (color_mgr->standard_pcc_enable) {
		dsi_parse_dcs_cmds(np, &color_mgr->uv_read_cmds,
			"somc,mdss-dsi-uv-command", NULL);

		rc = of_property_read_u32(np,
			"somc,mdss-dsi-uv-param-type", &tmp);
		color_mgr->standard_pcc_data.param_type =
			(!rc ? tmp : CLR_DATA_UV_PARAM_TYPE_NONE);

		rc = of_property_read_u32(np,
			"somc,mdss-dsi-pcc-table-size", &tmp);
		color_mgr->standard_pcc_data.tbl_size =
			(!rc ? tmp : 0);

		color_mgr->standard_pcc_data.color_tbl =
			kzalloc(color_mgr->standard_pcc_data.tbl_size *
				sizeof(struct dsi_pcc_color_tbl),
				GFP_KERNEL);
		if (!color_mgr->standard_pcc_data.color_tbl) {
			pr_err("no mem assigned: kzalloc fail\n");
			return -ENOMEM;
		}
		rc = of_property_read_u32_array(np,
			"somc,mdss-dsi-pcc-table",
			(u32 *)color_mgr->standard_pcc_data.color_tbl,
			color_mgr->standard_pcc_data.tbl_size *
			sizeof(struct dsi_pcc_color_tbl) /
			sizeof(u32));
		if (rc) {
			color_mgr->standard_pcc_data.tbl_size = 0;
			kzfree(color_mgr->standard_pcc_data.color_tbl);
			color_mgr->standard_pcc_data.color_tbl = NULL;
			pr_err("%s:%d, Unable to read pcc table",
				__func__, __LINE__);
		}
		//color_mgr->standard_pcc_data.pcc_sts |= PCC_STS_UD;
	} else {
		pr_err("%s:%d, Unable to read pcc table\n",
			__func__, __LINE__);
	}

	color_mgr->srgb_pcc_enable = of_property_read_bool(np,
			"somc,mdss-dsi-srgb-pcc-enable");
	if (color_mgr->srgb_pcc_enable) {
		rc = of_property_read_u32(np,
			"somc,mdss-dsi-srgb-pcc-table-size", &tmp);
		color_mgr->srgb_pcc_data.tbl_size =
			(!rc ? tmp : 0);

		color_mgr->srgb_pcc_data.color_tbl =
			kzalloc(color_mgr->srgb_pcc_data.tbl_size *
				sizeof(struct dsi_pcc_color_tbl),
				GFP_KERNEL);
		if (!color_mgr->srgb_pcc_data.color_tbl) {
			pr_err("no mem assigned: kzalloc fail\n");
			return -ENOMEM;
		}
		rc = of_property_read_u32_array(np,
			"somc,mdss-dsi-srgb-pcc-table",
			(u32 *)color_mgr->srgb_pcc_data.color_tbl,
			color_mgr->srgb_pcc_data.tbl_size *
			sizeof(struct dsi_pcc_color_tbl) /
			sizeof(u32));
		if (rc) {
			color_mgr->srgb_pcc_data.tbl_size = 0;
			kzfree(color_mgr->srgb_pcc_data.color_tbl);
			color_mgr->srgb_pcc_data.color_tbl = NULL;
			pr_err("%s:%d, Unable to read sRGB pcc table",
				__func__, __LINE__);
		}
	} else {
		pr_err("%s:%d, Unable to read srgb_pcc table",
			__func__, __LINE__);
	}

	color_mgr->vivid_pcc_enable = of_property_read_bool(np,
			"somc,mdss-dsi-vivid-pcc-enable");
	if (color_mgr->vivid_pcc_enable) {
		rc = of_property_read_u32(np,
			"somc,mdss-dsi-vivid-pcc-table-size", &tmp);
		color_mgr->vivid_pcc_data.tbl_size =
			(!rc ? tmp : 0);

		color_mgr->vivid_pcc_data.color_tbl =
			kzalloc(color_mgr->vivid_pcc_data.tbl_size *
				sizeof(struct dsi_pcc_color_tbl),
				GFP_KERNEL);
		if (!color_mgr->vivid_pcc_data.color_tbl) {
			pr_err("no mem assigned: kzalloc fail\n");
			return -ENOMEM;
		}
		rc = of_property_read_u32_array(np,
			"somc,mdss-dsi-vivid-pcc-table",
			(u32 *)color_mgr->vivid_pcc_data.color_tbl,
			color_mgr->vivid_pcc_data.tbl_size *
			sizeof(struct dsi_pcc_color_tbl) /
			sizeof(u32));
		if (rc) {
			color_mgr->vivid_pcc_data.tbl_size = 0;
			kzfree(color_mgr->vivid_pcc_data.color_tbl);
			color_mgr->vivid_pcc_data.color_tbl = NULL;
			pr_err("%s:%d, Unable to read Vivid pcc table",
				__func__, __LINE__);
		}
	} else {
		pr_err("%s:%d, Unable to read vivid_pcc table",
			__func__, __LINE__);
	}
	color_mgr->hdr_pcc_enable = of_property_read_bool(np,
			"somc,mdss-dsi-hdr-pcc-enable");
	if (color_mgr->hdr_pcc_enable) {
		rc = of_property_read_u32(np,
			"somc,mdss-dsi-hdr-pcc-table-size", &tmp);
		color_mgr->hdr_pcc_data.tbl_size =
			(!rc ? tmp : 0);

		color_mgr->hdr_pcc_data.color_tbl =
			kzalloc(color_mgr->hdr_pcc_data.tbl_size *
				sizeof(struct dsi_pcc_color_tbl),
				GFP_KERNEL);
		if (!color_mgr->hdr_pcc_data.color_tbl) {
			pr_err("no mem assigned: kzalloc fail\n");
			return -ENOMEM;
		}
		rc = of_property_read_u32_array(np,
			"somc,mdss-dsi-hdr-pcc-table",
			(u32 *)color_mgr->hdr_pcc_data.color_tbl,
			color_mgr->hdr_pcc_data.tbl_size *
			sizeof(struct dsi_pcc_color_tbl) /
			sizeof(u32));
		if (rc) {
			color_mgr->hdr_pcc_data.tbl_size = 0;
			kzfree(color_mgr->hdr_pcc_data.color_tbl);
			color_mgr->hdr_pcc_data.color_tbl = NULL;
			pr_err("%s:%d, Unable to read HDR pcc table",
				__func__, __LINE__);
		}
	} else {
		pr_err("%s:%d, Unable to read hdr_pcc table",
			__func__, __LINE__);
	}


	color_mgr->pcc_profile_avail = of_property_read_bool(np,
					"somc,panel-colormgr-pcc-prof-avail");

//picadj_params:
	/* Picture Adjustment (PicAdj) Parameters */
	if (!of_find_property(np, "somc,mdss-dsi-use-picadj", NULL))
		goto end;

	rc = of_property_read_u32(np, "somc,mdss-dsi-picadj-sat", &tmp);
	color_mgr->picadj_data.saturation = !rc ? tmp : -1;

	rc = of_property_read_u32(np, "somc,mdss-dsi-picadj-hue", &tmp);
	color_mgr->picadj_data.hue = !rc ? tmp : -1;

	rc = of_property_read_u32(np, "somc,mdss-dsi-picadj-val", &tmp);
	color_mgr->picadj_data.value = !rc ? tmp : -1;

	rc = of_property_read_u32(np, "somc,mdss-dsi-picadj-cont", &tmp);
	color_mgr->picadj_data.contrast = !rc ? tmp : -1;

	color_mgr->picadj_data.flags |= MDP_PP_OPS_ENABLE;

	rc = somc_panel_parse_dt_adaptivecolor_config(panel, np);

end:
	return rc;
}

void somc_panel_colormgr_reset(struct dsi_panel *panel)
{
	struct somc_panel_color_mgr *color_mgr =
				panel->spec_pdata->color_mgr;

	color_mgr->standard_pcc_data.pcc_sts = PCC_STS_UD;
	color_mgr->picadj_data.flags = MDP_PP_OPS_ENABLE;
}

static int somc_panel_colormgr_pcc_select(struct dsi_display *display,
	int profile_number)
{
	struct somc_panel_color_mgr *color_mgr =
				display->panel->spec_pdata->color_mgr;
	int ret = 0;

	if (profile_number == color_mgr->pcc_profile) {
		pr_debug("%s: Not applying: requested current profile\n",
			__func__);
		return 0;
	}

	if ((profile_number < PANEL_CALIB_6000K) ||
	    (profile_number >= PANEL_CALIB_END)) {
		pr_err("%s: Not applying: requested invalid profile\n",
			__func__);
		return -EINVAL;
	}

	color_mgr->pcc_profile = profile_number;
	somc_panel_colormgr_reset(display->panel);
	somc_panel_pcc_setup(display);

	return ret;
}

int somc_panel_pcc_setup(struct dsi_display *display)
{
	int ret;
	struct dsi_panel *panel = display->panel;
	struct somc_panel_color_mgr *color_mgr = NULL;
	struct dsi_pcc_data *pcc_data = NULL;
	u8 idx = 0;

	if (panel == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	color_mgr = panel->spec_pdata->color_mgr;

	if (color_mgr->dsi_pcc_applied) {
		pr_notice("%s (%d): PCC already applied\n", __func__, __LINE__);
		goto exit;
	}

	if (display->tx_cmd_buf == NULL) {
		ret = dsi_host_alloc_cmd_tx_buffer(display);
		if (ret)
			pr_err("failed to allocate cmd tx buffer memory\n");
	}

	if (color_mgr->uv_read_cmds.cmds.send_cmd) {
		get_uv_data(display, &color_mgr->u_data, &color_mgr->v_data);
	}

	if (color_mgr->u_data == 0 && color_mgr->v_data == 0) {
		pr_err("%s (%d): u,v is flashed 0.\n", __func__, __LINE__);
		goto exit;
	}
	pr_notice("%s (%d) udata = %x vdata = %x \n", __func__, __LINE__,
		color_mgr->u_data, color_mgr->v_data);

	if (color_mgr->standard_pcc_enable) {
		pcc_data = &color_mgr->standard_pcc_data;
		if (!pcc_data->color_tbl) {
			pr_err("%s (%d): standard_color_tbl isn't found.\n",
				__func__, __LINE__);
			goto exit;
		}

		ret = find_color_area(pcc_data,
			&color_mgr->u_data, &color_mgr->v_data);
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

	if (color_mgr->srgb_pcc_enable) {
		pcc_data = &color_mgr->srgb_pcc_data;
		if (!pcc_data->color_tbl) {
			pr_err("%s (%d): srgb_color_tbl isn't found.\n",
				__func__, __LINE__);
			goto exit;
		}
		ret = find_color_area(pcc_data,
			&color_mgr->u_data, &color_mgr->v_data);
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

	if (color_mgr->vivid_pcc_enable) {
		pcc_data = &color_mgr->vivid_pcc_data;
		if (!pcc_data->color_tbl) {
			pr_err("%s (%d): vivid_color_tbl isn't found.\n",
				__func__, __LINE__);
			goto exit;
		}
		ret = find_color_area(pcc_data,
			&color_mgr->u_data, &color_mgr->v_data);
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

	if (color_mgr->hdr_pcc_enable) {
		pcc_data = &color_mgr->hdr_pcc_data;
		if (!pcc_data->color_tbl) {
			pr_err("%s (%d): hdr_color_tbl isn't found.\n",
				__func__, __LINE__);
			goto exit;
		}
		ret = find_color_area(pcc_data,
			&color_mgr->u_data, &color_mgr->v_data);
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
	color_mgr->dsi_pcc_applied = true;

exit:
	return 0;
}

static ssize_t somc_panel_pcc_show(struct dsi_pcc_data *pcc_data,
		struct somc_panel_color_mgr *color_mgr, char *buf)
{
	u32 r, g, b;
	int tbl_idx = pcc_data->tbl_idx + color_mgr->pcc_profile;

	r = g = b = 0x8000;
	if (!pcc_data->color_tbl) {
		pr_err("%s: Panel has no color table\n", __func__);
		goto exit;
	}
	if (color_mgr->u_data == 0 && color_mgr->v_data == 0) {
		pr_err("%s: u,v are 0.\n", __func__);
		goto exit;
	}
	if (tbl_idx >= pcc_data->tbl_size) {
		pr_err("%s: Invalid color area(idx=%d)\n",
			__func__, pcc_data->tbl_idx);
		goto exit;
	}
	if (pcc_data->color_tbl[tbl_idx].color_type == UNUSED) {
		pr_err("%s: Unsupported color type(idx=%d)\n",
			__func__, tbl_idx);
		goto exit;
	}
	r = pcc_data->color_tbl[tbl_idx].r_data;
	g = pcc_data->color_tbl[tbl_idx].g_data;
	b = pcc_data->color_tbl[tbl_idx].b_data;
exit:
	return scnprintf(buf, PAGE_SIZE, "0x%x 0x%x 0x%x \n", r, g, b);
}

static ssize_t somc_panel_std_pcc_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct dsi_display *display = NULL;
	struct panel_specific_pdata *spec_pdata = NULL;
	struct dsi_pcc_data *pcc_data = NULL;

	display = dev_get_drvdata(dev);
	if (display == NULL)
		return -EINVAL;

	spec_pdata = display->panel->spec_pdata;
	if (spec_pdata == NULL)
		return -EINVAL;

	pcc_data = &spec_pdata->color_mgr->standard_pcc_data;

	return somc_panel_pcc_show(pcc_data, spec_pdata->color_mgr, buf);
}

static ssize_t somc_panel_srgb_pcc_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct dsi_display *display = NULL;
	struct panel_specific_pdata *spec_pdata = NULL;
	struct dsi_pcc_data *pcc_data = NULL;

	display = dev_get_drvdata(dev);
	if (display == NULL)
		return -EINVAL;

	spec_pdata = display->panel->spec_pdata;
	if (spec_pdata == NULL)
		return -EINVAL;

	pcc_data = &spec_pdata->color_mgr->srgb_pcc_data;

	return somc_panel_pcc_show(pcc_data, spec_pdata->color_mgr, buf);
}

static ssize_t somc_panel_vivid_pcc_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct dsi_display *display = NULL;
	struct panel_specific_pdata *spec_pdata = NULL;
	struct dsi_pcc_data *pcc_data = NULL;

	display = dev_get_drvdata(dev);
	if (display == NULL)
		return -EINVAL;

	spec_pdata = display->panel->spec_pdata;
	if (spec_pdata == NULL)
		return -EINVAL;

	pcc_data = &spec_pdata->color_mgr->vivid_pcc_data;

	return somc_panel_pcc_show(pcc_data, spec_pdata->color_mgr, buf);
}

static ssize_t somc_panel_hdr_pcc_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct dsi_display *display = NULL;
	struct panel_specific_pdata *spec_pdata = NULL;
	struct dsi_pcc_data *pcc_data = NULL;

	display = dev_get_drvdata(dev);
	if (display == NULL)
		return -EINVAL;

	spec_pdata = display->panel->spec_pdata;
	if (spec_pdata == NULL)
		return -EINVAL;

	pcc_data = &spec_pdata->color_mgr->hdr_pcc_data;

	return somc_panel_pcc_show(pcc_data, spec_pdata->color_mgr, buf);
}

static ssize_t somc_panel_color_mode_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct dsi_display *display = dev_get_drvdata(dev);
	struct somc_panel_color_mgr *color_mgr =
			display->panel->spec_pdata->color_mgr;

	pr_debug("color_mode is %d \n",
			color_mgr->color_mode);

	return scnprintf(buf, PAGE_SIZE, "%d \n",
			 color_mgr->color_mode);
}

static ssize_t somc_panel_color_mode_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct dsi_display *display = dev_get_drvdata(dev);
	struct somc_panel_color_mgr *color_mgr =
			display->panel->spec_pdata->color_mgr;

	int mode, rc = 0;

	if (sscanf(buf, "%d", &mode) < 0) {
		pr_err("sscanf failed to set mode. keep current mode=%d \n",
			color_mgr->color_mode);
		rc = -EINVAL;
		goto exit;
	}

	switch (mode) {
	case CLR_MODE_SELECT_SRGB:
	case CLR_MODE_SELECT_DCIP3:
	case CLR_MODE_SELECT_PANELNATIVE:
		color_mgr->color_mode = mode;
		break;
	default:
		pr_err("Cannot set color mode %d. Keeping current mode %d\n",
			mode, color_mgr->color_mode);
		rc = -EINVAL;
		break;
	}

exit:
	return !rc ? count : rc;

}

static ssize_t somc_panel_colormgr_pcc_select_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct dsi_display *display = dev_get_drvdata(dev);
	struct somc_panel_color_mgr *color_mgr =
				display->panel->spec_pdata->color_mgr;

	return scnprintf(buf, PAGE_SIZE, "%hu\n", color_mgr->pcc_profile);
}

static ssize_t somc_panel_colormgr_pcc_select_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct dsi_display *display = dev_get_drvdata(dev);
	int ret = count;
	int profile = 0;

	if (!display || !display->panel) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	ret = kstrtoint(buf, 5, &profile);
	if (ret < 0) {
		pr_err("%s: Error: buf = %s\n", __func__, buf);
		return -EINVAL;
	}

	ret = somc_panel_colormgr_pcc_select(display, profile);
	if (ret < 0)
		return -EINVAL;

	return count;
}

static ssize_t somc_panel_colormgr_pcc_profile_avail_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct dsi_display *display = dev_get_drvdata(dev);
	struct somc_panel_color_mgr *color_mgr =
			display->panel->spec_pdata->color_mgr;

	return scnprintf(buf, PAGE_SIZE, "%hu\n", color_mgr->pcc_profile_avail);
}

static struct device_attribute colormgr_attributes[] = {
	__ATTR(cc, S_IRUGO, somc_panel_std_pcc_show, NULL),
	__ATTR(srgb_cc, S_IRUGO, somc_panel_srgb_pcc_show, NULL),
	__ATTR(vivid_cc, S_IRUGO, somc_panel_vivid_pcc_show, NULL),
	__ATTR(hdr_cc, S_IRUGO, somc_panel_hdr_pcc_show, NULL),
	__ATTR(c_mode, S_IRUGO|S_IWUSR|S_IWGRP,
		somc_panel_color_mode_show,
		somc_panel_color_mode_store),
	__ATTR(pcc_profile, S_IRUGO|S_IWUSR|S_IWGRP,
				somc_panel_colormgr_pcc_select_show,
				somc_panel_colormgr_pcc_select_store),
	__ATTR(pcc_profile_avail, S_IRUGO,
				somc_panel_colormgr_pcc_profile_avail_show,
				NULL),
};

int somc_panel_colormgr_register_attr(struct device *dev)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(colormgr_attributes); i++)
		if (device_create_file(dev, colormgr_attributes + i))
			goto error;
	return 0;
error:
	dev_err(dev, "%s: Cannot create Color Manager attributes\n", __func__);
	for (--i; i >= 0 ; i--)
		device_remove_file(dev, colormgr_attributes + i);
	return -ENODEV;
}

/*
 * somc_panel_pa_validate_params - Validate PicADJ HSIC parameters
 *
 * The parameters have the following ranges:
 * Saturation: 0-224 - 12000-65534  EXCL: 128
 * Hue:            0 - 1536         EXCL: NO
 * Value:          0 -  383         EXCL: 0
 * Contrast:       0 -  383         EXCL: 0
 *
 * @return Returns true if parameters are valid, otherwise false.
 */
static inline bool somc_panel_pa_validate_params(struct drm_msm_pa_hsic *hsic_blk)
{
	if ( ((hsic_blk->saturation  < 224 || hsic_blk->saturation  > 12000)
				&& hsic_blk->saturation != 128) ||
	      (hsic_blk->hue         < 0   || hsic_blk->hue         > 1536) ||
	     ((hsic_blk->value       < 0   || hsic_blk->value       > 383)
				&& hsic_blk->value  != 0) ||
	     ((hsic_blk->contrast    < 0   || hsic_blk->contrast    > 383)
				&& hsic_blk->contrast != 0) )
	{
		return true;
	}
	return false;
}

int somc_panel_send_pa(struct dsi_display *display)
{
	struct drm_msm_pa_hsic hsic_blk;
	struct msm_drm_private *priv;
	struct drm_property *prop;
	struct drm_property_blob *pblob;
	struct drm_crtc *crtc = NULL;
	struct drm_plane *prim_plane = NULL;
	struct somc_panel_color_mgr *color_mgr =
			display->panel->spec_pdata->color_mgr;
	int rc;
	uint64_t val;

	if (!display->drm_conn) {
		pr_err("The display is not connected!!\n");
		return -EINVAL;
	}

	if (!display->drm_conn->state->crtc) {
		pr_err("No CRTC on display connector!!\n");
		return -ENODEV;
	}
	crtc = display->drm_conn->state->crtc;

	if (!crtc->primary) {
		pr_err("No plane on CRTC!! Bailing out.\n");
		return -ENODEV;
	}
	prim_plane = crtc->primary;

	priv = crtc->dev->dev_private;
	prop = priv->cp_property[3]; /* SDE_CP_CRTC_DSPP_HSIC == 3 !! */
	if (prop == NULL) {
		pr_err("FAIL! HSIC is not supported!!?!?!\n");
		return -EINVAL;
	};
	pr_debug("prop->name = %s\n", prop->name);

	rc = sde_cp_crtc_get_property(crtc, prop, &val);
	if (rc) {
		pr_err("Cannot get CRTC property. Things may go wrong.\n");
	};

	memset(&hsic_blk, 0, sizeof(struct drm_msm_pa_hsic));
	hsic_blk.hue = color_mgr->picadj_data.hue;
	hsic_blk.saturation = color_mgr->picadj_data.saturation;
	hsic_blk.value = color_mgr->picadj_data.value;
	hsic_blk.contrast = color_mgr->picadj_data.contrast;

	if (!somc_panel_pa_validate_params(&hsic_blk)) {
		pr_debug("PA HSIC parameters are invalid. Bailing out.\n");
		return -EINVAL;
	}

	hsic_blk.flags = PA_V2_BASIC_FEAT_ENB;

	/* Setup CRTC global to send calibration to DSPP */
	pblob = drm_property_create_blob(crtc->dev,
			sizeof(struct drm_msm_pa_hsic), &hsic_blk);
	if (IS_ERR_OR_NULL(pblob)) {
		pr_err("Failed to create blob. Bailing out.\n");
		return -EINVAL;
	}
	pr_debug("DSPP Blob ID %d has length %zu\n",
			prop->base.id, pblob->length);

	rc = sde_cp_crtc_set_property(crtc, prop, pblob->base.id);
	if (rc) {
		pr_err("DSPP: Cannot set HSIC: %d.\n", rc);
		goto end;
	}

	pr_debug("%s (%d):sat=%d hue=%d val=%d cont=%d",
		__func__, __LINE__, hsic_blk.saturation,
		hsic_blk.hue, hsic_blk.value,
		hsic_blk.contrast);
end:
	return rc;
}

static int somc_panel_crtc_send_pcc(struct dsi_display *display,
			       u32 r_data, u32 g_data, u32 b_data)
{
	struct drm_msm_pcc pcc_blk;
	struct msm_drm_private *priv;
	struct drm_property *prop;
	struct drm_property_blob *pblob;
	struct drm_crtc *crtc = NULL;
	int rc;
	uint64_t val;

	if (!display->drm_conn) {
		pr_err("The display is not connected!!\n");
		return -EINVAL;
	};

	if (!display->drm_conn->state->crtc) {
		pr_err("No CRTC on display connector!!\n");
		return -ENODEV;
	}

	crtc = display->drm_conn->state->crtc;

	priv = crtc->dev->dev_private;
	prop = priv->cp_property[1]; /* SDE_CP_CRTC_DSPP_PCC == 1 !! */
	if (prop == NULL) {
		pr_err("FAIL! PCC is not supported!!?!?!\n");
		return -EINVAL;
	};
	pr_debug("prop->name = %s\n", prop->name);

	rc = sde_cp_crtc_get_property(crtc, prop, &val);
	if (rc) {
		pr_err("Cannot get CRTC property. Things may go wrong.\n");
	};

	memset(&pcc_blk, 0, sizeof(struct drm_msm_pcc));
	pcc_blk.r.r = r_data;
	pcc_blk.g.g = g_data;
	pcc_blk.b.b = b_data;

	pblob = drm_property_create_blob(crtc->dev,
			sizeof(struct drm_msm_pcc), &pcc_blk);
	if (IS_ERR_OR_NULL(pblob)) {
		pr_err("Failed to create blob. Bailing out.\n");
		return -EINVAL;
	}
	pr_debug("DSPP Blob ID %d has length %zu\n",
			prop->base.id, pblob->length);

	rc = sde_cp_crtc_set_property(crtc, prop, pblob->base.id);
	if (rc) {
		pr_err("DSPP: Cannot set PCC: %d.\n", rc);
	}

	return rc;
}

static int somc_panel_send_pcc(struct dsi_display *display,
			       int color_table_offset)
{
	struct dsi_pcc_data *pcc_data = NULL;
	struct somc_panel_color_mgr *color_mgr =
			display->panel->spec_pdata->color_mgr;
	int table_idx;
	u32 r_data, g_data, b_data;

	pcc_data = &color_mgr->standard_pcc_data;
	table_idx = pcc_data->tbl_idx + color_table_offset;

	r_data = pcc_data->color_tbl[table_idx].r_data;
	g_data = pcc_data->color_tbl[table_idx].g_data;
	b_data = pcc_data->color_tbl[table_idx].b_data;

	return somc_panel_crtc_send_pcc(display, r_data, g_data, b_data);
}

int somc_panel_colormgr_apply_calibrations(void)
{
	struct dsi_display *display = dsi_display_get_main_display();
	struct somc_panel_color_mgr *color_mgr = NULL;
	int rc = 0;

	if (!display)
		return -EINVAL;

	color_mgr = display->panel->spec_pdata->color_mgr;

	rc = somc_panel_pcc_setup(display);
	if (rc) {
		pr_err("%s: Couldn't apply PCC calibration\n", __func__);
	}

	rc = somc_panel_send_pcc(display, color_mgr->pcc_profile);
	if (rc) {
		pr_err("%s: Cannot send PCC calibration\n", __func__);
	}

	rc += somc_panel_send_pa(display);
	if (rc) {
		pr_err("%s: Cannot send HSIC calibration\n", __func__);
	}

	return rc;
}

int somc_panel_color_manager_init(struct dsi_display *display)
{
	struct somc_panel_color_mgr *color_mgr =
		display->panel->spec_pdata->color_mgr;

	if (!color_mgr) {
		pr_err("%s: Color Manager is NULL!!!\n", __func__);
		return -EINVAL;
	}

	/* Be sure of initialization to default profile */
	color_mgr->pcc_profile = 0;

	return 0;
}
