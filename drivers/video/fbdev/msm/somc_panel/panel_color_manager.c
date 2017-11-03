/*
 * Restructured unified display panel driver for Xperia Open Devices
 *                    *** Color Management ***
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

#define PA_V2_BASIC_FEAT_ENB (MDP_PP_PA_HUE_ENABLE | MDP_PP_PA_SAT_ENABLE | \
			      MDP_PP_PA_VAL_ENABLE | MDP_PP_PA_CONT_ENABLE)
#define PA_V2_BASIC_MASK_ENB (MDP_PP_PA_HUE_MASK | MDP_PP_PA_SAT_MASK | \
			      MDP_PP_PA_VAL_MASK | MDP_PP_PA_CONT_MASK)

int somc_panel_parse_dt_colormgr_config(struct device_node *np,
			struct mdss_dsi_ctrl_pdata *ctrl)
{
	struct somc_panel_color_mgr *color_mgr = ctrl->spec_pdata->color_mgr;
	struct mdss_panel_info *pinfo;
	u32 tmp, res[2];
	int rc = 0;

	pinfo = &ctrl->panel_data.panel_info;

	if (pinfo->dsi_master != pinfo->pdest)
		goto end;

	if (!of_find_property(np, "somc,mdss-dsi-pcc-table", NULL))
		goto picadj_params;

	rc = of_property_read_u32(np,
		"somc,mdss-dsi-pcc-table-size", &tmp);
	color_mgr->pcc_data.tbl_size =
		(!rc ? tmp : 0);

	if (unlikely(color_mgr->pcc_data.tbl_size <= 0))
		goto picadj_params;

	/* PCC Parameters */
	color_mgr->pcc_data.color_tbl =
		kzalloc(color_mgr->pcc_data.tbl_size *
			sizeof(struct mdss_pcc_color_tbl),
			GFP_KERNEL);
	if (!color_mgr->pcc_data.color_tbl) {
		pr_err("no mem assigned: kzalloc fail\n");
		return -ENOMEM;
	}
	rc = of_property_read_u32_array(np,
		"somc,mdss-dsi-pcc-table",
		(u32 *)color_mgr->pcc_data.color_tbl,
		color_mgr->pcc_data.tbl_size *
		sizeof(struct mdss_pcc_color_tbl) /
		sizeof(u32));
	if (rc) {
		color_mgr->pcc_data.tbl_size = 0;
		kzfree(color_mgr->pcc_data.color_tbl);
		color_mgr->pcc_data.color_tbl = NULL;
		pr_err("%s:%d, Unable to read pcc table",
			__func__, __LINE__);
		goto picadj_params;
	}
	color_mgr->pcc_data.pcc_sts |= PCC_STS_UD;

	somc_panel_parse_dcs_cmds(np, &color_mgr->pre_uv_read_cmds,
		"somc,mdss-dsi-pre-uv-command", NULL);

	somc_panel_parse_dcs_cmds(np, &color_mgr->uv_read_cmds,
		"somc,mdss-dsi-uv-command", NULL);

	rc = of_property_read_u32(np,
		"somc,mdss-dsi-uv-param-type", &tmp);
	color_mgr->pcc_data.param_type =
		(!rc ? tmp : CLR_DATA_UV_PARAM_TYPE_NONE);

	color_mgr->mdss_force_pcc = of_property_read_bool(np,
					"somc,mdss-dsi-pcc-force-cal");

	rc = of_property_read_u32_array(np,
		"somc,mdss-dsi-u-rev", res, 2);
	if (rc) {
		color_mgr->pcc_data.rev_u[0] = 0;
		color_mgr->pcc_data.rev_u[1] = 0;
	} else {
		color_mgr->pcc_data.rev_u[0] = res[0];
		color_mgr->pcc_data.rev_u[1] = res[1];
	}
	rc = of_property_read_u32_array(np,
		"somc,mdss-dsi-v-rev", res, 2);
	if (rc) {
		color_mgr->pcc_data.rev_v[0] = 0;
		color_mgr->pcc_data.rev_v[1] = 0;
	} else {
		color_mgr->pcc_data.rev_v[0] = res[0];
		color_mgr->pcc_data.rev_v[1] = res[1];
	}

	color_mgr->pcc_profile_avail = of_property_read_bool(np,
					"somc,panel-colormgr-pcc-prof-avail");

picadj_params:
	/* Picture Adjustment (PicAdj) Parameters */
	if (!of_find_property(np, "somc,mdss-dsi-use-picadj", NULL))
		goto end;

	rc = of_property_read_u32(np, "somc,mdss-dsi-picadj-sat", &tmp);
	color_mgr->picadj_data.sat_adj = !rc ? tmp : -1;

	rc = of_property_read_u32(np, "somc,mdss-dsi-picadj-hue", &tmp);
	color_mgr->picadj_data.hue_adj = !rc ? tmp : -1;

	rc = of_property_read_u32(np, "somc,mdss-dsi-picadj-val", &tmp);
	color_mgr->picadj_data.val_adj = !rc ? tmp : -1;

	rc = of_property_read_u32(np, "somc,mdss-dsi-picadj-cont", &tmp);
	color_mgr->picadj_data.cont_adj = !rc ? tmp : -1;

	color_mgr->picadj_data.flags |= MDP_PP_OPS_ENABLE;

end:
	return rc;
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

static void get_uv_data(struct mdss_dsi_ctrl_pdata *ctrl_pdata,
		int *u_data, int *v_data)
{
	struct somc_panel_color_mgr *color_mgr =
			ctrl_pdata->spec_pdata->color_mgr;
	struct dsi_cmd_desc *cmds = color_mgr->uv_read_cmds.cmds;
	void *clk_handle;
	int param_type = color_mgr->pcc_data.param_type;
	char buf[MDSS_DSI_LEN];
	char *pos = buf;
	int len;
	int i;
	bool short_response;

	len = get_uv_param_len(param_type, &short_response);

	mdss_dsi_cmd_mdp_busy(ctrl_pdata);
	mdss_bus_bandwidth_ctrl(1);

	if (ctrl_pdata->panel_data.panel_info.type == MIPI_CMD_PANEL)
		clk_handle = ctrl_pdata->mdp_clk_handle;
	else
		clk_handle = ctrl_pdata->dsi_clk_handle;

	mdss_dsi_clk_ctrl(ctrl_pdata, clk_handle,
				MDSS_DSI_ALL_CLKS, MDSS_DSI_CLK_ON);
	for (i = 0; i < color_mgr->uv_read_cmds.cmd_cnt; i++) {
		if (short_response)
			mdss_dsi_cmds_rx(ctrl_pdata, cmds, 0, 0);
		else
			mdss_dsi_cmds_rx(ctrl_pdata, cmds, len, 0);
		memcpy(pos, ctrl_pdata->rx_buf.data, len);
		pos += len;
		cmds++;
	}
	mdss_dsi_clk_ctrl(ctrl_pdata, clk_handle,
				MDSS_DSI_ALL_CLKS, MDSS_DSI_CLK_OFF);
	mdss_bus_bandwidth_ctrl(0);
	conv_uv_data(buf, param_type, u_data, v_data);
}

static int find_color_area(struct mdp_pcc_cfg_data *pcc_config,
	struct mdss_pcc_data *pcc_data, unsigned short profile_override)
{
	int i;
	int ret = 0;

	for (i = 0; i < pcc_data->tbl_size; i++) {
		if (pcc_data->u_data < pcc_data->color_tbl[i].u_min)
			continue;
		if (pcc_data->u_data > pcc_data->color_tbl[i].u_max)
			continue;
		if (pcc_data->v_data < pcc_data->color_tbl[i].v_min)
			continue;
		if (pcc_data->v_data > pcc_data->color_tbl[i].v_max)
			continue;
		break;
	}
	i += profile_override;
	pcc_data->tbl_idx = i;
	if (i >= pcc_data->tbl_size) {
		ret = -EINVAL;
		goto exit;
	}

	pcc_config->r.r = pcc_data->color_tbl[i].r_data;
	pcc_config->g.g = pcc_data->color_tbl[i].g_data;
	pcc_config->b.b = pcc_data->color_tbl[i].b_data;
exit:
	return ret;
}

static int somc_panel_colormgr_pcc_select(struct mdss_dsi_ctrl_pdata *ctrl,
	int profile_number)
{
	struct somc_panel_color_mgr *color_mgr = ctrl->spec_pdata->color_mgr;
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
	somc_panel_colormgr_reset(ctrl);
	color_mgr->pcc_setup(&ctrl->panel_data);

	return ret;
}

static int somc_panel_pcc_setup(struct mdss_panel_data *pdata)
{
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct mdss_pcc_data *pcc_data = NULL;
	struct somc_panel_color_mgr *color_mgr = NULL;
	struct mdss_data_type *mdata = mdss_mdp_get_mdata();
	struct msm_fb_data_type *mfd = mdata->ctl_off->mfd;
	int ret;
	u32 copyback;
	struct mdp_pcc_cfg_data pcc_config;
	struct mdp_pcc_data_v1_7 pcc_payload;
	struct mdp_pp_feature_version pcc_version = {
		.pp_feature = PCC,
	};

	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

	color_mgr = ctrl_pdata->spec_pdata->color_mgr;
	if (!color_mgr) {
		pr_err("%s: Color Manager is not initialized!!!\n", __func__);
		goto exit;
	}

	pcc_data = &color_mgr->pcc_data;
	if (!pcc_data->color_tbl) {
		pr_err("%s: color_tbl not available: no pcc.\n", __func__);
		goto exit;
	}

	if (!of_machine_is_compatible("qcom,msm8956") ||
	    !of_machine_is_compatible("qcom,msm8939"))
		mdss_dsi_op_mode_config(DSI_CMD_MODE, pdata);

	if (color_mgr->pre_uv_read_cmds.cmds)
		mdss_dsi_panel_cmds_send(
			ctrl_pdata, &color_mgr->pre_uv_read_cmds);
	if (color_mgr->uv_read_cmds.cmds) {
		get_uv_data(ctrl_pdata, &pcc_data->u_data, &pcc_data->v_data);
		pcc_data->u_data = CENTER_U_DATA;
		pcc_data->v_data = CENTER_V_DATA;
	}
	if (pcc_data->u_data == 0 && pcc_data->v_data == 0) {
		pr_err("%s: U/V Data is invalid.\n", __func__);
		if (!color_mgr->mdss_force_pcc)
			goto exit;

		pr_info("%s: PCC force flag found. Forcing calibration.\n",
								 __func__);
	}

	if (pcc_data->rev_u[1] != 0) {
		if (pcc_data->rev_u[0] == 0)
			pcc_data->u_data = pcc_data->u_data + pcc_data->rev_u[1];
		else if (pcc_data->u_data < pcc_data->rev_u[1])
			pcc_data->u_data = 0;
		else
			pcc_data->u_data = pcc_data->u_data - pcc_data->rev_u[1];
	}
	if (pcc_data->rev_v[1] != 0) {
		if (pcc_data->rev_v[0] == 0)
			pcc_data->v_data = pcc_data->v_data + pcc_data->rev_v[1];
		else if (pcc_data->v_data < pcc_data->rev_v[1])
			pcc_data->v_data = 0;
		else
			pcc_data->v_data = pcc_data->v_data - pcc_data->rev_v[1];
	}

	memset(&pcc_config, 0, sizeof(struct mdp_pcc_cfg_data));
	ret = find_color_area(&pcc_config, pcc_data, color_mgr->pcc_profile);
	if (ret) {
		pr_err("%s: Can't find color area!!!!\n", __func__);
		goto exit;
	}

	if (pcc_data->color_tbl[pcc_data->tbl_idx].color_type != UNUSED) {
		ret = mdss_mdp_pp_get_version(&pcc_version);
		if (ret) {
			pr_err("%s: FAIL: Cannot get PP version.\n", __func__);
			goto exit;
		}
		memset(&pcc_payload, 0, sizeof(struct mdp_pcc_data_v1_7));
		pcc_config.cfg_payload = &pcc_payload;
		pcc_config.version = pcc_version.version_info;
		pcc_config.block = MDP_LOGICAL_BLOCK_DISP_0;
		pcc_config.ops = MDP_PP_OPS_ENABLE | MDP_PP_OPS_WRITE;

		pcc_payload.r.r = pcc_config.r.r;
		pcc_payload.g.g = pcc_config.g.g;
		pcc_payload.b.b = pcc_config.b.b;

		ret = mdss_mdp_pcc_config(mfd, &pcc_config, &copyback);
		if (ret != 0)
			pr_err("%s: Failed setting PCC data\n", __func__);
	}

	if (pcc_data->rev_u[1] != 0 && pcc_data->rev_v[1] != 0)
		pr_info("%s: (%d):(ru[0], ru[1])=(%d, %d), (rv[0], rv[1])=(%d, %d)",
			__func__, __LINE__,
			pcc_data->rev_u[0], pcc_data->rev_u[1],
			pcc_data->rev_v[0], pcc_data->rev_v[1]);

	pr_info("%s: (%d):ct=%d area=%d ud=%d vd=%d r=0x%08X g=0x%08X b=0x%08X",
		__func__, __LINE__,
		pcc_data->color_tbl[pcc_data->tbl_idx].color_type,
		pcc_data->color_tbl[pcc_data->tbl_idx].area_num,
		pcc_data->u_data, pcc_data->v_data,
		pcc_data->color_tbl[pcc_data->tbl_idx].r_data,
		pcc_data->color_tbl[pcc_data->tbl_idx].g_data,
		pcc_data->color_tbl[pcc_data->tbl_idx].b_data);
exit:
	return 0;
}

static int somc_panel_pa_setup(struct mdss_panel_data *pdata)
{
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct somc_panel_color_mgr *color_mgr = NULL;
	struct mdp_pa_cfg *padata = NULL;
	struct mdp_pa_cfg_data picadj;
	struct mdss_data_type *mdata = mdss_mdp_get_mdata();
	struct msm_fb_data_type *mfd = mdata->ctl_off->mfd;
	u32 copyback = 0;

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

	color_mgr = ctrl_pdata->spec_pdata->color_mgr;
	if (!color_mgr) {
		pr_err("%s: Color Manager is not initialized!!!\n", __func__);
		return -EINVAL;
	}

	padata = &color_mgr->picadj_data;
	if (!padata)
		return -EINVAL;

	memset(&picadj, 0, sizeof(struct mdp_pa_cfg_data));

	/* Check if values are in permitted range, otherwise read defaults */
	if ( ((padata->sat_adj  < 224 || padata->sat_adj  > 12000)
						&& padata->sat_adj != 128)||
	      (padata->hue_adj  < 0   || padata->hue_adj  > 1536) ||
	     ((padata->val_adj  < 0   || padata->val_adj  > 383)
						&& padata->val_adj  != 0) ||
	     ((padata->cont_adj < 0   || padata->cont_adj > 383)
						&& padata->cont_adj != 0) )
	{
		picadj.block = MDP_LOGICAL_BLOCK_DISP_0;
		picadj.pa_data.flags = MDP_PP_OPS_ENABLE | MDP_PP_OPS_READ;

		mdss_mdp_pa_config(mfd, &picadj, &copyback);
		pr_err("%s: ERROR: Values not specified or invalid. \
			Setting defaults.\n", __func__);
		pr_err("%s (%d): defaults: sat=%d hue=%d val=%d cont=%d",
			__func__, __LINE__,
			picadj.pa_data.sat_adj, picadj.pa_data.hue_adj,
			picadj.pa_data.val_adj, picadj.pa_data.cont_adj);

		padata = &picadj.pa_data;
	}

	picadj.block = MDP_LOGICAL_BLOCK_DISP_0;
	padata->flags = MDP_PP_OPS_ENABLE | MDP_PP_OPS_WRITE;
	picadj.pa_data = *padata;

	mdss_mdp_pa_config(mfd, &picadj, &copyback);

	pr_info("%s (%d):sat=%d hue=%d val=%d cont=%d",
		__func__, __LINE__, padata->sat_adj,
		padata->hue_adj, padata->val_adj, padata->cont_adj);

	return 0;
}

static int somc_panel_pa_v2_setup(struct mdss_panel_data *pdata)
{
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct somc_panel_color_mgr *color_mgr = NULL;
	struct mdp_pa_cfg *compat = NULL;
	struct mdp_pa_v2_data *padata = NULL;
	struct mdp_pa_v2_cfg_data picadj;
	struct mdss_data_type *mdata = mdss_mdp_get_mdata();
	struct msm_fb_data_type *mfd = mdata->ctl_off->mfd;
	struct mdp_pp_feature_version pa_version = {
		.pp_feature = PA,
	};
	u32 copyback = 0;
	int ret;

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

	color_mgr = ctrl_pdata->spec_pdata->color_mgr;
	if (!color_mgr) {
		pr_err("%s: Color Manager is not initialized!!!\n", __func__);
		return -EINVAL;
	}

	compat = &color_mgr->picadj_data;
	if (!compat)
		return -EINVAL;

	memset(&picadj, 0, sizeof(struct mdp_pa_v2_cfg_data));

	padata = kzalloc(sizeof(*padata), GFP_KERNEL);
	if (padata == NULL) {
		pr_err("%s: CRITICAL: Allocation failure. Bailing out.\n",
			__func__);
		return -ENOMEM;
	}

	ret = mdss_mdp_pp_get_version(&pa_version);
	if (ret) {
		pr_err("%s: Cannot get PP HW version. Bailing out.\n",
			__func__);
		return -EINVAL;
	};
	picadj.version = pa_version.version_info;

	padata->global_sat_adj = compat->sat_adj;
	padata->global_hue_adj = compat->hue_adj;
	padata->global_val_adj = compat->val_adj;
	padata->global_cont_adj = compat->cont_adj;
	padata->flags = MDP_PP_OPS_ENABLE;

	/* Check if values are in permitted range, otherwise read defaults */
	if ( ((padata->global_sat_adj  < 0  || padata->global_sat_adj  > 12000)
					    && padata->global_sat_adj != 128)||
	      (padata->global_hue_adj  < 0  || padata->global_hue_adj  > 1536) ||
	     ((padata->global_val_adj  < 0  || padata->global_val_adj  > 383)
					    && padata->global_val_adj  != 0) ||
	     ((padata->global_cont_adj < 0  || padata->global_cont_adj > 383)
					    && padata->global_cont_adj != 0) )
	{
		picadj.block = MDP_LOGICAL_BLOCK_DISP_0;
		picadj.pa_v2_data.flags = MDP_PP_OPS_ENABLE | MDP_PP_OPS_READ |
					  PA_V2_BASIC_FEAT_ENB |
					  PA_V2_BASIC_MASK_ENB;

		ret = mdss_mdp_pa_v2_config(mfd, &picadj, &copyback);
		pr_err("%s: ERROR: Values not specified or invalid. \
			Setting defaults.\n", __func__);
		pr_err("%s (%d): defaults: sat=%d hue=%d val=%d cont=%d",
			__func__, __LINE__,
			picadj.pa_v2_data.global_sat_adj,
			picadj.pa_v2_data.global_hue_adj,
			picadj.pa_v2_data.global_val_adj,
			picadj.pa_v2_data.global_cont_adj);

		padata = &picadj.pa_v2_data;
	}

	picadj.block = MDP_LOGICAL_BLOCK_DISP_0;
	picadj.flags = MDP_PP_OPS_ENABLE | MDP_PP_OPS_WRITE |
			PA_V2_BASIC_FEAT_ENB | PA_V2_BASIC_MASK_ENB;
	padata->flags = picadj.flags;
	picadj.cfg_payload = padata;

	ret = mdss_mdp_pa_v2_config(mfd, &picadj, &copyback);
	if (ret)
		pr_err("%s: Cannot configure picadj: %d\n",
			__func__, ret);

	pr_info("%s (%d):sat=%d hue=%d val=%d cont=%d",
		__func__, __LINE__, padata->global_sat_adj,
		padata->global_hue_adj, padata->global_val_adj,
		padata->global_cont_adj);

	return ret;
}

static int somc_panel_picadj_setup(struct mdss_panel_data *pdata)
{
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

	/*
	 * Note: MDSS_DSI_HW_REV_101_1 is 8974Pro, which has MDP
	 * revision 1.2.1 (102_1).
	 * New picadj is required starting from MDP rev. 1.3.0 (103)
	 * which has any DSI version >= 1.2.0 (102).
	 */
	if (ctrl_pdata->shared_data->hw_rev > MDSS_DSI_HW_REV_101_1)
		return somc_panel_pa_v2_setup(pdata);
	else
		return somc_panel_pa_setup(pdata);
}

static ssize_t mdss_dsi_panel_pcc_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = dev_get_drvdata(dev);
	struct mdss_pcc_data *pcc_data =
		&ctrl_pdata->spec_pdata->color_mgr->pcc_data;
	u32 r, g, b;

	r = g = b = 0;
	if (!pcc_data->color_tbl)
		goto exit;
	if (pcc_data->u_data == 0 && pcc_data->v_data == 0)
		goto exit;
	if (pcc_data->tbl_idx >= pcc_data->tbl_size)
		goto exit;
	if (pcc_data->color_tbl[pcc_data->tbl_idx].color_type == UNUSED)
		goto exit;
	r = pcc_data->color_tbl[pcc_data->tbl_idx].r_data;
	g = pcc_data->color_tbl[pcc_data->tbl_idx].g_data;
	b = pcc_data->color_tbl[pcc_data->tbl_idx].b_data;
exit:
	return scnprintf(buf, PAGE_SIZE, "0x%x 0x%x 0x%x ", r, g, b);
}

static ssize_t somc_panel_colormgr_pcc_select_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct mdss_dsi_ctrl_pdata *ctrl = dev_get_drvdata(dev);
	struct somc_panel_color_mgr *color_mgr = ctrl->spec_pdata->color_mgr;

	return scnprintf(buf, PAGE_SIZE, "%hu\n", color_mgr->pcc_profile);
}

static ssize_t somc_panel_colormgr_pcc_select_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct mdss_dsi_ctrl_pdata *ctrl = dev_get_drvdata(dev);
	int ret = count;
	int profile = 0;

	if (!ctrl || !ctrl->spec_pdata) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	ret = kstrtoint(buf, 5, &profile);
	if (ret < 0) {
		pr_err("%s: Error: buf = %s\n", __func__, buf);
		return -EINVAL;
	}

	ret = somc_panel_colormgr_pcc_select(ctrl, profile);
	if (ret < 0)
		return -EINVAL;

	return count;
}

static ssize_t somc_panel_colormgr_pcc_profile_avail_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct mdss_dsi_ctrl_pdata *ctrl = dev_get_drvdata(dev);
	struct somc_panel_color_mgr *color_mgr = ctrl->spec_pdata->color_mgr;

	return scnprintf(buf, PAGE_SIZE, "%hu\n", color_mgr->pcc_profile_avail);
}

static struct device_attribute colormgr_attributes[] = {
	__ATTR(cc, S_IRUGO, mdss_dsi_panel_pcc_show, NULL),
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

static int somc_panel_colormgr_unblank_handler(struct mdss_dsi_ctrl_pdata *ctrl)
{
	struct somc_panel_color_mgr *color_mgr = ctrl->spec_pdata->color_mgr;

	if (color_mgr->pcc_data.pcc_sts & PCC_STS_UD) {
		color_mgr->pcc_setup(&ctrl->panel_data);
		color_mgr->pcc_data.pcc_sts &= ~PCC_STS_UD;
	}

	if (color_mgr->picadj_data.flags & MDP_PP_OPS_ENABLE) {
		color_mgr->picadj_setup(&ctrl->panel_data);
		color_mgr->picadj_data.flags &= ~MDP_PP_OPS_ENABLE;
	}

	return 0;
}

void somc_panel_colormgr_reset(struct mdss_dsi_ctrl_pdata *ctrl)
{
	struct somc_panel_color_mgr *color_mgr = ctrl->spec_pdata->color_mgr;

	color_mgr->pcc_data.pcc_sts = PCC_STS_UD;
	color_mgr->picadj_data.flags = MDP_PP_OPS_ENABLE;
}

int somc_panel_color_manager_init(struct mdss_dsi_ctrl_pdata *ctrl)
{
	struct somc_panel_color_mgr *color_mgr = NULL;

	color_mgr = ctrl->spec_pdata->color_mgr;
	if (!color_mgr) {
		pr_err("%s: Color Manager is NULL!!!\n", __func__);
		return -EINVAL;
	}

	/* Be sure of initialization to default profile */
	color_mgr->pcc_profile = 0;

	color_mgr->pcc_setup = somc_panel_pcc_setup;
	color_mgr->picadj_setup = somc_panel_picadj_setup;
	color_mgr->unblank_hndl = somc_panel_colormgr_unblank_handler;

	return 0;
}
