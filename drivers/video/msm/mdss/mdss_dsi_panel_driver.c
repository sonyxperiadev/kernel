/* Copyright (c) 2012-2013, The Linux Foundation. All rights reserved.
 * Copyright (C) 2013 Sony Mobile Communications AB.
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
 * NOTE: This file has been modified by Sony Mobile Communications AB.
 * Modifications are licensed under the License.
 */

#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/of_platform.h>
#include <linux/gpio.h>
#include <linux/qpnp/pin.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/leds.h>
#include <linux/pwm.h>
#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/regulator/consumer.h>

#include "mdss_mdp.h"
#include "mdss_dsi.h"

#define DT_CMD_HDR 6

#define DEFAULT_FPS_LOG_INTERVAL 100
#define DEFAULT_FPS_ARRAY_SIZE 120

#define DSI_PCLK_MIN 3300000
#define DSI_PCLK_MAX 223000000
#define DSI_PCLK_DEFAULT 35000000

#define MS2US(a) ((a)*1000)

struct device virtdev;

static int vsn_gpio;
static int vsp_gpio;

struct fps_array {
	u32 frame_nbr;
	u32 time_delta;
};
static u16 fps_array_cnt;

static struct fps_data {
	struct mutex fps_lock;
	u32 log_interval;
	u32 interval_ms;
	struct timespec timestamp_last;
	u32 frame_counter_last;
	u32 frame_counter;
	u32 fpks;
	struct timespec fpks_ts_last;
	u16 fa_last_array_pos;
	struct fps_array fa[DEFAULT_FPS_ARRAY_SIZE];
} fpsd;

DEFINE_LED_TRIGGER(bl_led_trigger);

static int lcd_id;
static int mdss_dsi_panel_detect(struct mdss_panel_data *pdata);
static int mdss_panel_parse_dt(struct device_node *np,
				struct mdss_dsi_ctrl_pdata *ctrl_pdata,
				int driver_ic, char *id_data);

#define PANEL_SKIP_ID		0xff

static char dcs_cmd[2] = {0x54, 0x00}; /* DTYPE_DCS_READ */
static struct dsi_cmd_desc dcs_read_cmd = {
	{DTYPE_DCS_READ, 1, 0, 1, 5, sizeof(dcs_cmd)},
	dcs_cmd
};

u32 mdss_dsi_dcs_read(struct mdss_dsi_ctrl_pdata *ctrl,
			char cmd0, char cmd1)
{
	struct dcs_cmd_req cmdreq;

	dcs_cmd[0] = cmd0;
	dcs_cmd[1] = cmd1;
	memset(&cmdreq, 0, sizeof(cmdreq));
	cmdreq.cmds = &dcs_read_cmd;
	cmdreq.cmds_cnt = 1;
	cmdreq.flags = CMD_REQ_RX | CMD_REQ_COMMIT;
	cmdreq.rlen = 1;
	cmdreq.cb = NULL; /* call back */
	mdss_dsi_cmdlist_put(ctrl, &cmdreq);
	/*
	 * blocked here, until call back called
	 */

	return 0;
}

static void mdss_dsi_panel_cmds_send(struct mdss_dsi_ctrl_pdata *ctrl,
			struct dsi_panel_cmds *pcmds)
{
	struct dcs_cmd_req cmdreq;

	memset(&cmdreq, 0, sizeof(cmdreq));
	cmdreq.cmds = pcmds->cmds;
	cmdreq.cmds_cnt = pcmds->cmd_cnt;
	cmdreq.flags = CMD_REQ_COMMIT;
	cmdreq.rlen = 0;
	cmdreq.cb = NULL;

	/*Panel ON/Off commands should be sent in DSI Low Power Mode*/
	if (pcmds->link_state == DSI_LP_MODE)
		cmdreq.flags |= CMD_REQ_LP_MODE;

	mdss_dsi_cmdlist_put(ctrl, &cmdreq);
}

static char caset[] = {0x2a, 0x00, 0x00, 0x03, 0x00};	/* DTYPE_DCS_LWRITE */
static char paset[] = {0x2b, 0x00, 0x00, 0x05, 0x00};	/* DTYPE_DCS_LWRITE */

static struct dsi_cmd_desc partial_update_enable_cmd[] = {
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 1, sizeof(caset)}, caset},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 1, sizeof(paset)}, paset},
};

static int mdss_dsi_panel_partial_update(struct mdss_panel_data *pdata)
{
	struct mipi_panel_info *mipi;
	struct mdss_dsi_ctrl_pdata *ctrl = NULL;
	struct dcs_cmd_req cmdreq;
	int rc = 0;

	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	ctrl = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);
	mipi  = &pdata->panel_info.mipi;

	pr_debug("%s: ctrl=%p ndx=%d\n", __func__, ctrl, ctrl->ndx);

	caset[1] = (((pdata->panel_info.roi_x) & 0xFF00) >> 8);
	caset[2] = (((pdata->panel_info.roi_x) & 0xFF));
	caset[3] = (((pdata->panel_info.roi_x - 1 + pdata->panel_info.roi_w)
								& 0xFF00) >> 8);
	caset[4] = (((pdata->panel_info.roi_x - 1 + pdata->panel_info.roi_w)
								& 0xFF));
	partial_update_enable_cmd[0].payload = caset;

	paset[1] = (((pdata->panel_info.roi_y) & 0xFF00) >> 8);
	paset[2] = (((pdata->panel_info.roi_y) & 0xFF));
	paset[3] = (((pdata->panel_info.roi_y - 1 + pdata->panel_info.roi_h)
								& 0xFF00) >> 8);
	paset[4] = (((pdata->panel_info.roi_y - 1 + pdata->panel_info.roi_h)
								& 0xFF));
	partial_update_enable_cmd[1].payload = paset;

	pr_debug("%s: enabling partial update\n", __func__);
	memset(&cmdreq, 0, sizeof(cmdreq));
	cmdreq.cmds = partial_update_enable_cmd;
	cmdreq.cmds_cnt = 2;
	cmdreq.flags = CMD_REQ_COMMIT | CMD_CLK_CTRL;
	cmdreq.rlen = 0;
	cmdreq.cb = NULL;

	mdss_dsi_cmdlist_put(ctrl, &cmdreq);

	return rc;
}

static void mdss_dsi_panel_fps_array_clear(struct fps_data *fps)
{
	memset(fps->fa, 0, sizeof(fps->fa));
	fps_array_cnt = 0;
}

static ssize_t mdss_dsi_panel_frame_counter(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%i\n", fpsd.frame_counter);
}

static ssize_t mdss_dsi_panel_frames_per_ksecs(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%i\n", fpsd.fpks);
}

static ssize_t mdss_dsi_panel_interval_ms_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%i\n", fpsd.interval_ms);
}

static ssize_t mdss_dsi_panel_log_interval_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%i\n", fpsd.log_interval);
}

static ssize_t mdss_dsi_panel_log_interval_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int ret = count;

	if (sscanf(buf, "%4i", &fpsd.log_interval) != 1) {
		pr_err("%s: Error, buf = %s\n", __func__, buf);
		ret = -EINVAL;
	}
	return ret;
}

#define DEBUG_INTERVAL_ARRAY
static ssize_t mdss_dsi_panel_interval_array_ms(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	u16 i, len, rc = 0;
	char *tmp = buf;

	mutex_lock(&fpsd.fps_lock);
	len = fpsd.fa_last_array_pos;
	/* Get the first frames from the buffer */
	for (i = len + 1; i < DEFAULT_FPS_ARRAY_SIZE; i++) {
		if (fpsd.fa[i].time_delta) {
#ifdef DEBUG_INTERVAL_ARRAY
			/* FrameNumber, buf idx and delta time */
			rc += scnprintf(tmp + rc, PAGE_SIZE - rc ,
							"%03i[%03i]: %i,\n",
							fpsd.fa[i].frame_nbr, i,
							fpsd.fa[i].time_delta);
#else
			rc += scnprintf(tmp + rc, PAGE_SIZE - rc ,
						"%i, ", fpsd.fa[i].time_delta);
#endif
		}
	}
	/* Get the rest frames from the buffer */
	if (len) {
		for (i = 0; i <= len; i++) {
			if (fpsd.fa[i].time_delta) {
#ifdef DEBUG_INTERVAL_ARRAY
				/* FrameNumber, buf idx and delta time */
				rc += scnprintf(tmp + rc, PAGE_SIZE - rc ,
							"%03i[%03i]: %i,\n",
							fpsd.fa[i].frame_nbr, i,
							fpsd.fa[i].time_delta);
#else
				rc += scnprintf(tmp + rc, PAGE_SIZE - rc ,
						"%i, ", fpsd.fa[i].time_delta);
#endif
			}
		}
	}
	rc += scnprintf(tmp + rc, PAGE_SIZE - rc , "\n");

	/* Clear the buffer once it is read */
	mdss_dsi_panel_fps_array_clear(&fpsd);
	mutex_unlock(&fpsd.fps_lock);

	return rc;
}

static int mdss_dsi_panel_read_cabc(struct device *dev)
{
	struct platform_device *pdev;
	struct mdss_dsi_ctrl_pdata *ctrl;

	pdev = container_of(dev, struct platform_device, dev);
	if (!pdev) {
		dev_err(dev, "%s(%d): no panel connected\n",
							__func__, __LINE__);
		goto exit;
	}

	ctrl = platform_get_drvdata(pdev);
	if (!ctrl) {
		dev_err(dev, "%s(%d): no panel connected\n",
							__func__, __LINE__);
		goto exit;
	}
	dev_dbg(dev, "%s: Read CABC setting %d\n",
				__func__, ctrl->spec_pdata->cabc_enabled);
	return ctrl->spec_pdata->cabc_enabled;
exit:
	return -EINVAL;
}

static void mdss_dsi_panel_write_cabc(struct device *dev, int enable)
{
	struct mdss_dsi_ctrl_pdata *ctrl;

	ctrl = dev_get_drvdata(dev);
	if (!ctrl) {
		dev_err(dev, "%s(%d): no panel connected\n",
							__func__, __LINE__);
		goto exit;
	}
	dev_dbg(dev, "%s: Set CABC setting %d\n",
							__func__, enable);
	ctrl->spec_pdata->cabc_enabled = enable;

exit:
	return;
}

static ssize_t mdss_dsi_panel_cabc_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int tmp;
	tmp = mdss_dsi_panel_read_cabc(dev);
	return scnprintf(buf, PAGE_SIZE, "%i\n", tmp);
}

static ssize_t mdss_dsi_panel_cabc_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int ret = count;
	long tmp;

	if (kstrtol(buf, 10, &tmp)) {
		dev_err(dev, "%s: Error, buf = %s\n", __func__, buf);
		ret = -EINVAL;
		goto exit;
	}
	mdss_dsi_panel_write_cabc(dev, tmp);
exit:
	return ret;
}

static ssize_t mdss_dsi_panel_id_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = dev_get_drvdata(dev);
	char const *id = ctrl_pdata->panel_data.panel_info.panel_id_name ?
		ctrl_pdata->panel_data.panel_info.panel_id_name : "default";

	return scnprintf(buf, PAGE_SIZE, "%s\n", id);
}

static struct device_attribute panel_attributes[] = {
	__ATTR(frame_counter, S_IRUGO, mdss_dsi_panel_frame_counter, NULL),
	__ATTR(frames_per_ksecs, S_IRUGO,
				mdss_dsi_panel_frames_per_ksecs, NULL),
	__ATTR(interval_ms, S_IRUGO, mdss_dsi_panel_interval_ms_show, NULL),
	__ATTR(log_interval, S_IRUGO|S_IWUSR|S_IWGRP,
					mdss_dsi_panel_log_interval_show,
					mdss_dsi_panel_log_interval_store),
	__ATTR(interval_array, S_IRUGO,
					mdss_dsi_panel_interval_array_ms, NULL),
	__ATTR(cabc, S_IRUGO|S_IWUSR|S_IWGRP, mdss_dsi_panel_cabc_show,
						mdss_dsi_panel_cabc_store),
	__ATTR(panel_id, S_IRUGO, mdss_dsi_panel_id_show, NULL),
};

static int register_attributes(struct device *dev)
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
	return -ENOMEM;
}

static inline int remove_attributes(struct device *dev)
{
	int i;
	dev_info(dev, "%s: unregistering interface\n", __func__);
	for (i = 0; i < ARRAY_SIZE(panel_attributes); i++)
		device_remove_file(dev, panel_attributes + i);
	return 0;
}

static int mdss_panel_dt_get_dst_fmt(u32 bpp, char mipi_mode, u32 pixel_packing,
				char *dst_format)
{
	int rc = 0;
	switch (bpp) {
	case 3:
		*dst_format = DSI_CMD_DST_FORMAT_RGB111;
		break;
	case 8:
		*dst_format = DSI_CMD_DST_FORMAT_RGB332;
		break;
	case 12:
		*dst_format = DSI_CMD_DST_FORMAT_RGB444;
		break;
	case 16:
		switch (mipi_mode) {
		case DSI_VIDEO_MODE:
			*dst_format = DSI_VIDEO_DST_FORMAT_RGB565;
			break;
		case DSI_CMD_MODE:
			*dst_format = DSI_CMD_DST_FORMAT_RGB565;
			break;
		default:
			*dst_format = DSI_VIDEO_DST_FORMAT_RGB565;
			break;
		}
		break;
	case 18:
		switch (mipi_mode) {
		case DSI_VIDEO_MODE:
			if (pixel_packing == 0)
				*dst_format = DSI_VIDEO_DST_FORMAT_RGB666;
			else
				*dst_format = DSI_VIDEO_DST_FORMAT_RGB666_LOOSE;
			break;
		case DSI_CMD_MODE:
			*dst_format = DSI_CMD_DST_FORMAT_RGB666;
			break;
		default:
			if (pixel_packing == 0)
				*dst_format = DSI_VIDEO_DST_FORMAT_RGB666;
			else
				*dst_format = DSI_VIDEO_DST_FORMAT_RGB666_LOOSE;
			break;
		}
		break;
	case 24:
		switch (mipi_mode) {
		case DSI_VIDEO_MODE:
			*dst_format = DSI_VIDEO_DST_FORMAT_RGB888;
			break;
		case DSI_CMD_MODE:
			*dst_format = DSI_CMD_DST_FORMAT_RGB888;
			break;
		default:
			*dst_format = DSI_VIDEO_DST_FORMAT_RGB888;
			break;
		}
		break;
	default:
		rc = -EINVAL;
		break;
	}
	return rc;
}

static int mdss_dsi_panel_detect(struct mdss_panel_data *pdata)
{
	struct device_node *np;
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct mdss_panel_specific_pdata *spec_pdata = NULL;

	int rc;

	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);
	if (!ctrl_pdata) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	spec_pdata = ctrl_pdata->spec_pdata;
	if (!spec_pdata) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	if (!spec_pdata->panel_detect)
		return 0;

	mdss_dsi_op_mode_config(DSI_CMD_MODE, pdata);
	mdss_dsi_cmds_rx(ctrl_pdata,
			 spec_pdata->id_read_cmds.cmds, 6);

	pr_debug("%s: Panel ID", __func__);
	pr_debug("0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X",
		ctrl_pdata->rx_buf.data[0], ctrl_pdata->rx_buf.data[1],
		ctrl_pdata->rx_buf.data[2], ctrl_pdata->rx_buf.data[3],
		ctrl_pdata->rx_buf.data[4], ctrl_pdata->rx_buf.data[5],
		ctrl_pdata->rx_buf.data[6], ctrl_pdata->rx_buf.data[7]);

	np = of_parse_phandle(
			pdata->panel_pdev->dev.of_node,
			"qcom,dsi-pref-prim-pan", 0);
	if (!np) {
		pr_err("%s np=NULL\n", __func__);
		return -EINVAL;
	}

	rc = mdss_panel_parse_dt(np, ctrl_pdata,
		spec_pdata->driver_ic, ctrl_pdata->rx_buf.data);

	return 0;
}

static int mdss_dsi_panel_update_panel_info(struct mdss_panel_data *pdata)
{
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct mdss_panel_specific_pdata *spec_pdata = NULL;
	struct mdss_panel_info *pinfo;
	struct mipi_panel_info *mipi;
	int rc;

	pr_debug("%s >", __func__);
	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);
	if (!ctrl_pdata) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	spec_pdata = ctrl_pdata->spec_pdata;
	if (!spec_pdata) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	if (!spec_pdata->panel_detect)
		return 0;


	pinfo = &(ctrl_pdata->panel_data.panel_info);
	mipi = &pinfo->mipi;

	pinfo->type =
		((mipi->mode == DSI_VIDEO_MODE)
			? MIPI_VIDEO_PANEL : MIPI_CMD_PANEL);

	rc = mdss_dsi_clk_div_config(pinfo, mipi->frame_rate);
	if (rc) {
		pr_err("%s: unable to initialize the clk dividers\n", __func__);
		return rc;
	}

	ctrl_pdata->pclk_rate = mipi->dsi_pclk_rate;
	ctrl_pdata->byte_clk_rate = pinfo->clk_rate / 8;

	ctrl_pdata->on_cmds = spec_pdata->on_cmds;
	ctrl_pdata->off_cmds = spec_pdata->off_cmds;
	ctrl_pdata->cabc_off_cmds = spec_pdata->cabc_off_cmds;
	ctrl_pdata->spec_pdata->disp_on_in_hs = spec_pdata->disp_on_in_hs;
	ctrl_pdata->spec_pdata->cabc_enabled = spec_pdata->cabc_enabled;

	return 0;
}

static void mdss_dsi_parse_lane_swap(struct device_node *np, char *dlane_swap)
{
	const char *data;

	*dlane_swap = DSI_LANE_MAP_0123;
	data = of_get_property(np, "qcom,mdss-dsi-lane-map", NULL);
	if (data) {
		if (!strcmp(data, "lane_map_3012"))
			*dlane_swap = DSI_LANE_MAP_3012;
		else if (!strcmp(data, "lane_map_2301"))
			*dlane_swap = DSI_LANE_MAP_2301;
		else if (!strcmp(data, "lane_map_1230"))
			*dlane_swap = DSI_LANE_MAP_1230;
		else if (!strcmp(data, "lane_map_0321"))
			*dlane_swap = DSI_LANE_MAP_0321;
		else if (!strcmp(data, "lane_map_1032"))
			*dlane_swap = DSI_LANE_MAP_1032;
		else if (!strcmp(data, "lane_map_2103"))
			*dlane_swap = DSI_LANE_MAP_2103;
		else if (!strcmp(data, "lane_map_3210"))
			*dlane_swap = DSI_LANE_MAP_3210;
	}
}

static void mdss_dsi_parse_trigger(struct device_node *np, char *trigger,
		char *trigger_key)
{
	const char *data;

	*trigger = DSI_CMD_TRIGGER_SW;
	data = of_get_property(np, trigger_key, NULL);
	if (data) {
		if (!strcmp(data, "none"))
			*trigger = DSI_CMD_TRIGGER_NONE;
		else if (!strcmp(data, "trigger_te"))
			*trigger = DSI_CMD_TRIGGER_TE;
		else if (!strcmp(data, "trigger_sw_seof"))
			*trigger = DSI_CMD_TRIGGER_SW_SEOF;
		else if (!strcmp(data, "trigger_sw_te"))
			*trigger = DSI_CMD_TRIGGER_SW_TE;
	}
}

static int mdss_dsi_parse_fbc_params(struct device_node *np,
				struct mdss_panel_info *panel_info)
{
	int rc, fbc_enabled = 0;
	u32 tmp;

	fbc_enabled = of_property_read_bool(np,	"qcom,mdss-dsi-fbc-enable");
	if (fbc_enabled) {
		pr_debug("%s:%d FBC panel enabled.\n", __func__, __LINE__);
		panel_info->fbc.enabled = 1;
		rc = of_property_read_u32(np, "qcom,mdss-dsi-fbc-bpp", &tmp);
		panel_info->fbc.target_bpp =	(!rc ? tmp : panel_info->bpp);
		rc = of_property_read_u32(np, "qcom,mdss-dsi-fbc-packing",
				&tmp);
		panel_info->fbc.comp_mode = (!rc ? tmp : 0);
		panel_info->fbc.qerr_enable = of_property_read_bool(np,
			"qcom,mdss-dsi-fbc-quant-error");
		rc = of_property_read_u32(np, "qcom,mdss-dsi-fbc-bias", &tmp);
		panel_info->fbc.cd_bias = (!rc ? tmp : 0);
		panel_info->fbc.pat_enable = of_property_read_bool(np,
				"qcom,mdss-dsi-fbc-pat-mode");
		panel_info->fbc.vlc_enable = of_property_read_bool(np,
				"qcom,mdss-dsi-fbc-vlc-mode");
		panel_info->fbc.bflc_enable = of_property_read_bool(np,
				"qcom,mdss-dsi-fbc-bflc-mode");
		rc = of_property_read_u32(np, "qcom,mdss-dsi-fbc-h-line-budget",
				&tmp);
		panel_info->fbc.line_x_budget = (!rc ? tmp : 0);
		rc = of_property_read_u32(np, "qcom,mdss-dsi-fbc-budget-ctrl",
				&tmp);
		panel_info->fbc.block_x_budget = (!rc ? tmp : 0);
		rc = of_property_read_u32(np, "qcom,mdss-dsi-fbc-block-budget",
				&tmp);
		panel_info->fbc.block_budget = (!rc ? tmp : 0);
		rc = of_property_read_u32(np,
				"qcom,mdss-dsi-fbc-lossless-threshold", &tmp);
		panel_info->fbc.lossless_mode_thd = (!rc ? tmp : 0);
		rc = of_property_read_u32(np,
				"qcom,mdss-dsi-fbc-lossy-threshold", &tmp);
		panel_info->fbc.lossy_mode_thd = (!rc ? tmp : 0);
		rc = of_property_read_u32(np, "qcom,mdss-dsi-fbc-rgb-threshold",
				&tmp);
		panel_info->fbc.lossy_rgb_thd = (!rc ? tmp : 0);
		rc = of_property_read_u32(np,
				"qcom,mdss-dsi-fbc-lossy-mode-idx", &tmp);
		panel_info->fbc.lossy_mode_idx = (!rc ? tmp : 0);
	} else {
		pr_debug("%s:%d Panel does not support FBC.\n",
				__func__, __LINE__);
		panel_info->fbc.enabled = 0;
		panel_info->fbc.target_bpp =
			panel_info->bpp;
	}
	return 0;
}


static int mdss_dsi_parse_dcs_cmds(struct device_node *np,
		struct dsi_panel_cmds *pcmds, char *cmd_key, char *link_key)
{
	const char *data;
	int blen = 0, len;
	char *buf, *bp;
	struct dsi_ctrl_hdr *dchdr;
	int i, cnt;

	pcmds->cmd_cnt = 0;

	data = of_get_property(np, cmd_key, &blen);
	if (!data) {
		pr_err("%s:Unable to read key %s cmds", __func__, cmd_key);
		return -EINVAL;
	}

	if (pcmds->buf != NULL)
		kfree(pcmds->buf);

	if (pcmds->cmds != NULL)
		kfree(pcmds->cmds);

	buf = kzalloc(sizeof(char) * blen, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	memcpy(buf, data, blen);

	/* scan dcs commands */
	bp = buf;
	len = blen;
	cnt = 0;
	while (len > sizeof(*dchdr)) {
		dchdr = (struct dsi_ctrl_hdr *)bp;
		dchdr->dlen = ntohs(dchdr->dlen);
		if (dchdr->dlen > len) {
			pr_err("%s: dtsi cmd=%x error, len=%d",
				__func__, dchdr->dtype, dchdr->dlen);
			kfree(buf);
			return -ENOMEM;
		}
		bp += sizeof(*dchdr);
		len -= sizeof(*dchdr);
		bp += dchdr->dlen;
		len -= dchdr->dlen;
		cnt++;
	}

	if (len != 0) {
		pr_err("%s: dcs_cmd=%x len=%d error!",
				__func__, buf[0], blen);
		kfree(buf);
		return -ENOMEM;
	}

	pcmds->cmds = kzalloc(cnt * sizeof(struct dsi_cmd_desc),
						GFP_KERNEL);
	if (!pcmds->cmds) {
		kfree(buf);
		return -ENOMEM;
	}

	pcmds->cmd_cnt = cnt;
	pcmds->buf = buf;
	pcmds->blen = blen;

	bp = buf;
	len = blen;
	for (i = 0; i < cnt; i++) {
		dchdr = (struct dsi_ctrl_hdr *)bp;
		len -= sizeof(*dchdr);
		bp += sizeof(*dchdr);
		pcmds->cmds[i].dchdr = *dchdr;
		pcmds->cmds[i].payload = bp;
		bp += dchdr->dlen;
		len -= dchdr->dlen;
	}

	pcmds->link_state = DSI_LP_MODE; /* default */

	if (link_key != NULL) {
		data = of_get_property(np, link_key, NULL);
		if (!strncmp(data, "DSI_HS_MODE", 11))
			pcmds->link_state = DSI_HS_MODE;
	}

	pr_debug("%s: dcs_cmd=%x len=%d, cmd_cnt=%d link_state=%d\n", __func__,
		pcmds->buf[0], pcmds->blen, pcmds->cmd_cnt, pcmds->link_state);

	return 0;
}


static inline int mdss_dsi_panel_regulators(
				struct mdss_dsi_ctrl_pdata *ctrl, int enable)
{
	int ret = 0;

	if (enable) {
		ret = regulator_set_optimum_mode
		  ((ctrl->shared_pdata).vdd_vreg, 100000);
		if (ret < 0) {
			pr_err("%s: vdd_vreg set regulator mode failed.\n",
							   __func__);
			goto exit;
		}

		ret = regulator_set_optimum_mode(
			(ctrl->shared_pdata).vdd_io_vreg, 100000);
		if (ret < 0) {
			pr_err("%s: vdd_io_vreg set regulator mode failed.\n",
							   __func__);
			goto exit;
		}

		ret = regulator_set_optimum_mode(
			(ctrl->shared_pdata).vdda_vreg, 100000);
		if (ret < 0) {
			pr_err("%s: vdda_vreg set regulator mode failed.\n",
							   __func__);
			goto exit;
		}

		ret = regulator_enable((ctrl->shared_pdata).vdd_io_vreg);
		if (ret) {
			pr_err("%s: Failed to enable regulator.\n", __func__);
			goto exit;
		}

		ret = regulator_enable((ctrl->shared_pdata).vdd_vreg);
		if (ret) {
			pr_err("%s: Failed to enable regulator.\n", __func__);
			goto exit;
		}
		ret = regulator_enable((ctrl->shared_pdata).vdda_vreg);
		if (ret) {
			pr_err("%s: Failed to enable regulator.\n", __func__);
			goto exit;
		}
	} else {
		ret = regulator_disable((ctrl->shared_pdata).vdd_vreg);
		if (ret) {
			pr_err("%s: Failed to disable regulator.\n", __func__);
			goto exit;
		}

		ret = regulator_disable((ctrl->shared_pdata).vdda_vreg);
		if (ret) {
			pr_err("%s: Failed to disable regulator.\n", __func__);
			goto exit;
		}

		ret = regulator_disable((ctrl->shared_pdata).vdd_io_vreg);
		if (ret) {
			pr_err("%s: Failed to disable regulator.\n", __func__);
			goto exit;
		}

		ret = regulator_set_optimum_mode
		  ((ctrl->shared_pdata).vdd_vreg, 100);
		if (ret < 0) {
			pr_err("%s: vdd_vreg set regulator mode failed.\n",
							   __func__);
			goto exit;
		}

		ret = regulator_set_optimum_mode(
			(ctrl->shared_pdata).vdd_io_vreg, 100);
		if (ret < 0) {
			pr_err("%s: vdd_io_vreg set regulator mode failed.\n",
							   __func__);
			goto exit;
		}
		ret = regulator_set_optimum_mode(
			(ctrl->shared_pdata).vdda_vreg, 100);
		if (ret < 0) {
			pr_err("%s: vdda_vreg set regulator mode failed.\n",
							   __func__);
		}
	}
exit:
	return ret;
}

#define VREG_WAIT_MS	15
#define DISP_EN_CHANGE_WAIT_MS	21
#define VREG_DISABLE_WAIT_US 6000
#define VREG_DISABLE_WAIT_MAX_US 7000

static int mdss_dsi_panel_reset_panel(
		struct mdss_panel_data *pdata, int enable)
{
	struct mdss_dsi_ctrl_pdata *ctrl = NULL;
	struct mdss_panel_info *pinfo = NULL;
	int i = 0;

	pr_debug("%s(%d):+", __func__, enable);
	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	ctrl = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);
	pinfo = &(ctrl->panel_data.panel_info);
	if (!gpio_is_valid(ctrl->disp_en_gpio))
		pr_err("%s:disp_en line not configured\n", __func__);

	if (!ctrl->rst_gpio) {
		pr_err("%s: reset line not configured\n",
			   __func__);
		return -EINVAL;
	}
	if (enable) {
		msleep(VREG_WAIT_MS);
		if (gpio_is_valid(ctrl->disp_en_gpio))
			gpio_set_value(ctrl->disp_en_gpio, 1);

		for (i = 0; i < pinfo->rst_seq_len; ++i) {
			gpio_set_value((ctrl->rst_gpio),
				pinfo->rst_seq[i]);
			if (pinfo->rst_seq[++i])
				usleep(pinfo->rst_seq[i] * 1000);
		}
	} else {
		usleep_range(VREG_DISABLE_WAIT_US, VREG_DISABLE_WAIT_MAX_US);
		gpio_set_value(ctrl->rst_gpio, 0);
		usleep_range(11000, 12000);
		if (gpio_is_valid(ctrl->disp_en_gpio)) {
			gpio_set_value(ctrl->disp_en_gpio, 0);
			msleep(DISP_EN_CHANGE_WAIT_MS);
		}
	}

	return 0;
}

static int mdss_dsi_panel_power_on_panel(
		struct mdss_panel_data *pdata, int enable)
{
	int ret;
	struct mdss_dsi_ctrl_pdata *ctrl = NULL;
	static int display_power_on;

	pr_debug("%s(%d):+", __func__, enable);
	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	ctrl = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

	if (enable && !display_power_on) {
		if (ctrl->power_data.num_vreg > 0) {
			ret = msm_dss_enable_vreg(
				ctrl->power_data.vreg_config,
				ctrl->power_data.num_vreg, 1);
			if (ret) {
				pr_err("%s:Failed to enable regulators.rc=%d\n",
					__func__, ret);
				return ret;
			}

		} else {
			mdss_dsi_panel_regulators(ctrl, enable);
		}
		/*
		 * A small delay is needed here after enabling
		 * all regulators and before issuing panel reset
		 */
		usleep_range(19000, 20000);

		gpio_set_value(vsn_gpio, 1);
		gpio_set_value(vsp_gpio, 1);
		ret = mdss_dsi_panel_reset_panel(pdata, enable);
		if (ret) {
			pr_err("%s: Failed to enable gpio.\n", __func__);
			return ret;
		}
		display_power_on = 1;
	} else {
		gpio_set_value(vsn_gpio, 0);
		gpio_set_value(vsp_gpio, 0);
		ret = mdss_dsi_panel_reset_panel(pdata, enable);
		if (ret) {
			pr_err("%s: Failed to disable gpio.\n", __func__);
			return ret;
		}
		if (ctrl->power_data.num_vreg > 0) {
			ret = msm_dss_enable_vreg(
				ctrl->power_data.vreg_config,
				ctrl->power_data.num_vreg, 0);
			if (ret) {
				pr_err("%s: Failed to disable regs.rc=%d\n",
					__func__, ret);
				return ret;
			}
		} else {
			mdss_dsi_panel_regulators(ctrl, enable);
		}
		display_power_on = 0;
	}
	return ret;
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

		fps->fa[fps_array_cnt].frame_nbr = fps->frame_counter;
		fps->fa[fps_array_cnt].time_delta = msec;
		fps->fa_last_array_pos = fps_array_cnt;
		fps_array_cnt++;
		if (fps_array_cnt >= DEFAULT_FPS_ARRAY_SIZE)
			fps_array_cnt = 0;

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

static void mdss_dsi_panel_fps_data_init(struct fps_data *fps)
{
	fps->frame_counter = 0;
	fps->frame_counter_last = 0;
	fps->log_interval = DEFAULT_FPS_LOG_INTERVAL;
	fps->fpks = 0;
	fps->fa_last_array_pos = 0;
	getrawmonotonic(&fps->timestamp_last);
	mutex_init(&fps->fps_lock);
}

int mdss_dsi_panel_fps_data_update(struct msm_fb_data_type *mfd)
{
	/* Only count fps on primary display */
	if (mfd->index == 0)
		update_fps_data(&fpsd);

	return 0;
}

void mdss_dsi_panel_pwm_cfg(struct mdss_dsi_ctrl_pdata *ctrl)
{
	int ret;

	if (!gpio_is_valid(ctrl->pwm_pmic_gpio)) {
		pr_err("%s: pwm_pmic_gpio=%d Invalid\n", __func__,
				ctrl->pwm_pmic_gpio);
		ctrl->pwm_pmic_gpio = -1;
		return;
	}

	ret = gpio_request(ctrl->pwm_pmic_gpio, "disp_pwm");
	if (ret) {
		pr_err("%s: pwm_pmic_gpio=%d request failed\n", __func__,
				ctrl->pwm_pmic_gpio);
		ctrl->pwm_pmic_gpio = -1;
		return;
	}

	ctrl->pwm_bl = pwm_request(ctrl->pwm_lpg_chan, "lcd-bklt");
	if (ctrl->pwm_bl == NULL || IS_ERR(ctrl->pwm_bl)) {
		pr_err("%s: lpg_chan=%d pwm request failed", __func__,
				ctrl->pwm_lpg_chan);
		gpio_free(ctrl->pwm_pmic_gpio);
		ctrl->pwm_pmic_gpio = -1;
	}
}

static void mdss_dsi_panel_bklt_pwm(struct mdss_dsi_ctrl_pdata *ctrl, int level)
{
	int ret;
	u32 duty;

	if (ctrl->pwm_bl == NULL) {
		pr_err("%s: no PWM\n", __func__);
		return;
	}

	duty = level * ctrl->pwm_period;
	duty /= ctrl->bklt_max;

	pr_debug("%s: bklt_ctrl=%d pwm_period=%d pwm_gpio=%d pwm_lpg_chan=%d\n",
			__func__, ctrl->bklt_ctrl, ctrl->pwm_period,
				ctrl->pwm_pmic_gpio, ctrl->pwm_lpg_chan);

	pr_debug("%s: ndx=%d level=%d duty=%d\n", __func__,
					ctrl->ndx, level, duty);

	ret = pwm_config(ctrl->pwm_bl, duty, ctrl->pwm_period);
	if (ret) {
		pr_err("%s: pwm_config() failed err=%d.\n", __func__, ret);
		return;
	}

	ret = pwm_enable(ctrl->pwm_bl);
	if (ret)
		pr_err("%s: pwm_enable() failed err=%d\n", __func__, ret);
}

static char led_pwm1[2] = {0x51, 0x0};	/* DTYPE_DCS_WRITE1 */
static struct dsi_cmd_desc backlight_cmd = {
	{DTYPE_DCS_WRITE1, 1, 0, 0, 1, sizeof(led_pwm1)},
	led_pwm1
};

static void mdss_dsi_panel_bklt_dcs(struct mdss_dsi_ctrl_pdata *ctrl, int level)
{
	struct dcs_cmd_req cmdreq;

	pr_debug("%s: level=%d\n", __func__, level);

	led_pwm1[1] = (unsigned char)level;

	memset(&cmdreq, 0, sizeof(cmdreq));
	cmdreq.cmds = &backlight_cmd;
	cmdreq.cmds_cnt = 1;
	cmdreq.flags = CMD_REQ_COMMIT | CMD_CLK_CTRL;
	cmdreq.rlen = 0;
	cmdreq.cb = NULL;

	mdss_dsi_cmdlist_put(ctrl, &cmdreq);
}

static void mdss_dsi_panel_bl_ctrl(struct mdss_panel_data *pdata,
							u32 bl_level)
{
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return;
	}

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

	switch (ctrl_pdata->bklt_ctrl) {
	case BL_WLED:
		led_trigger_event(bl_led_trigger, bl_level);
		break;
	case BL_PWM:
		mdss_dsi_panel_bklt_pwm(ctrl_pdata, bl_level);
		break;
	case BL_DCS_CMD:
		mdss_dsi_panel_bklt_dcs(ctrl_pdata, bl_level);
		break;
	default:
		pr_err("%s: Unknown bl_ctrl configuration\n",
			__func__);
		break;
	}
}

#define PANEL_DISP_ON_WAIT_MS	20
static int mdss_dsi_panel_disp_on(struct mdss_panel_data *pdata)
{
	struct mipi_panel_info *mipi;
	struct mdss_dsi_ctrl_pdata *ctrl = NULL;
	struct mdss_panel_specific_pdata *spec_pdata = NULL;


	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	ctrl = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

	spec_pdata = ctrl->spec_pdata;
	if (!spec_pdata) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}
	if (!spec_pdata->detected)
		return 0;

	mipi = &pdata->panel_info.mipi;

	if (ctrl->on_cmds.cmd_cnt) {
		msleep(PANEL_DISP_ON_WAIT_MS);
		mdss_dsi_panel_cmds_send(ctrl, &ctrl->on_cmds);
		pr_info("%s: DISPLAY_ON commands sent", __func__);
	}

	return 0;
}

static int mdss_dsi_panel_on(struct mdss_panel_data *pdata)
{
	struct mipi_panel_info *mipi;
	struct mdss_dsi_ctrl_pdata *ctrl = NULL;
	struct mdss_panel_specific_pdata *spec_pdata = NULL;

	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	ctrl = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);
	spec_pdata = ctrl->spec_pdata;
	if (!spec_pdata) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}
	if (!spec_pdata->detected)
		return 0;

	mipi = &pdata->panel_info.mipi;

	if (spec_pdata->einit_cmds.cmd_cnt) {
		pr_debug("%s: early init sequence\n", __func__);
		mdss_dsi_panel_cmds_send(ctrl, &spec_pdata->einit_cmds);
	}

	if (spec_pdata->cabc_on_cmds.cmd_cnt && spec_pdata->cabc_enabled) {
		mdss_dsi_panel_cmds_send(ctrl, &spec_pdata->cabc_on_cmds);
		pr_debug("%s: CABC Enabled", __func__);
	}

	if (spec_pdata->init_cmds.cmd_cnt) {
		mdss_dsi_panel_cmds_send(ctrl, &spec_pdata->init_cmds);
		pr_info("%s: DISPLAY_INIT commands sent", __func__);
	}

	if (ctrl->on_cmds.cmd_cnt && !spec_pdata->disp_on_in_hs) {
		mdss_dsi_panel_cmds_send(ctrl, &ctrl->on_cmds);
		pr_info("%s: DISPLAY_ON commands sent", __func__);
	}

	return 0;
}

static int mdss_dsi_parse_reset_seq(struct device_node *np,
		u32 rst_seq[MDSS_DSI_RST_SEQ_LEN], u32 *rst_len,
		const char *name)
{
	int num = 0, i;
	int rc;
	struct property *data;
	u32 tmp[MDSS_DSI_RST_SEQ_LEN];
	*rst_len = 0;
	data = of_find_property(np, name, &num);
	num /= sizeof(u32);
	if (!data || !num || num > MDSS_DSI_RST_SEQ_LEN || num % 2) {
		pr_debug("%s:%d, error reading %s, length found = %d\n",
			__func__, __LINE__, name, num);
	} else {
		rc = of_property_read_u32_array(np, name, tmp, num);
		if (rc)
			pr_debug("%s:%d, error reading %s, rc = %d\n",
				__func__, __LINE__, name, rc);
		else {
			for (i = 0; i < num; ++i)
				rst_seq[i] = tmp[i];
			*rst_len = num;
		}
	}
	return 0;
}

static int mdss_dsi_panel_off(struct mdss_panel_data *pdata)
{
	struct mipi_panel_info *mipi;
	struct mdss_dsi_ctrl_pdata *ctrl = NULL;
	struct mdss_panel_specific_pdata *spec_pdata = NULL;

	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}
	pr_debug("%s: Entering", __func__);
	ctrl = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);
	spec_pdata = ctrl->spec_pdata;
	if (!spec_pdata) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	if (!spec_pdata->detected) {
		spec_pdata->detected = true;
		return 0;
	}
	pr_debug("%s: ctrl=%p ndx=%d\n", __func__, ctrl, ctrl->ndx);

	mipi  = &pdata->panel_info.mipi;

	if (ctrl->off_cmds.cmd_cnt) {
		mdss_dsi_panel_cmds_send(ctrl, &ctrl->off_cmds);
		pr_info("%s: DISPLAY_OFF commands sent", __func__);
	}

	if (ctrl->cabc_off_cmds.cmd_cnt && ctrl->spec_pdata->cabc_enabled) {
		mdss_dsi_panel_cmds_send(ctrl, &ctrl->cabc_off_cmds);
		pr_debug("%s: CABC disabled", __func__);
	}
	return 0;
}

static int mdss_panel_parse_dt(struct device_node *np,
			struct mdss_dsi_ctrl_pdata *ctrl_pdata,
			int driver_ic, char *id_data)
{
	u32 res[MDSS_DSI_RST_SEQ_LEN], tmp;
	int rc, i, len;
	const char *data;
	const char *panel_name;
	static const char *pdest;
	struct device_node *parent;
	struct device_node *next;
	struct mdss_panel_info *pinfo = &(ctrl_pdata->panel_data.panel_info);
	struct mdss_panel_specific_pdata *spec_pdata = NULL;

	spec_pdata = ctrl_pdata->spec_pdata;
	if (!spec_pdata) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	parent = of_get_parent(np);
	for_each_child_of_node(parent, next) {
		rc = of_property_read_u32(next, "somc,driver-ic", &tmp);
		if (rc)
			continue;
		if (driver_ic != tmp)
			continue;

		data = of_get_property(next, "somc,panel-id", &len);
		if (!data) {
			pr_err("%s:%d, panel not read\n",
				__func__, __LINE__);
			goto error;
		}
		if (data && id_data) {
			if ((len == 1) && (data[0] == PANEL_SKIP_ID))
				continue;
			for (i = 0; i < len; i++) {
				pr_debug("read data:0x%02X dtsi data:0x%02X",
						id_data[i], data[i]);
				if ((id_data[i] != data[i]) &&
					(data[i] != PANEL_SKIP_ID)) {
					rc = -ENODEV;
					break;
				}
				rc = 0;
			}
			if (rc)
				continue;
		} else if (data && !id_data) {
			if ((len != 1) || (data[0] != PANEL_SKIP_ID))
				continue;
			else {
				rc = of_property_read_u32(next
					, "somc,panel-detect", &tmp);
				spec_pdata->panel_detect = !rc ? tmp : 0;
			}
		}

		panel_name = of_get_property(next,
			"qcom,mdss-dsi-panel-name", NULL);
		if (!panel_name) {
			pr_info("%s:%d, panel name not specified\n",
							__func__, __LINE__);
		} else {
			pinfo->panel_id_name = panel_name;
			pr_info("%s: Panel Name = %s\n", __func__, panel_name);
		}

		rc = of_property_read_u32(next,
			"qcom,mdss-dsi-panel-width", &tmp);
		if (rc) {
			pr_err("%s:%d, panel resolution not specified\n",
							__func__, __LINE__);
		}
		pinfo->xres = (!rc ? tmp : 720);
		rc = of_property_read_u32(next,
			"qcom,mdss-dsi-panel-height", &tmp);
		if (rc) {
			pr_err("%s:%d, panel resolution not specified\n",
							__func__, __LINE__);
		}
		pinfo->yres  = (!rc ? tmp : 1280);

		rc = of_property_read_u32(next,
			"qcom,mdss-pan-physical-width-dimension", &tmp);
		if (rc)
			pr_err("%s:%d, panel physical size not specified\n",
							__func__, __LINE__);

		pinfo->physical_width = (!rc ? tmp : 0);
		rc = of_property_read_u32(next,
			"qcom,mdss-pan-physical-height-dimension", &tmp);
		pinfo->physical_height = (!rc ? tmp : 0);

		rc = of_property_read_u32(next,
			"qcom,mdss-dsi-h-left-border", &tmp);
		pinfo->lcdc.xres_pad = (!rc ? tmp : 0);
		rc = of_property_read_u32(next,
			"qcom,mdss-dsi-h-right-border", &tmp);
		if (!rc)
			pinfo->lcdc.xres_pad += tmp;
		rc = of_property_read_u32(next,
			"qcom,mdss-dsi-v-top-border", &tmp);
		pinfo->lcdc.yres_pad = (!rc ? tmp : 0);
		rc = of_property_read_u32(next,
			"qcom,mdss-dsi-v-bottom-border", &tmp);
		if (!rc)
			pinfo->lcdc.yres_pad += tmp;

		rc = of_property_read_u32(next, "qcom,mdss-dsi-bpp", &tmp);
		if (rc) {
			pr_err("%s:%d, panel bpp not specified\n",
				__func__, __LINE__);
			goto error;
		}
		pinfo->bpp = tmp;
		pinfo->mipi.mode = DSI_VIDEO_MODE;
		data = of_get_property(next, "qcom,mdss-dsi-panel-type", NULL);
		if (data && !strncmp(data, "dsi_cmd_mode", 12))
			pinfo->mipi.mode = DSI_CMD_MODE;

		pinfo->type = MIPI_VIDEO_PANEL;
		if (pinfo->mipi.mode == DSI_CMD_MODE)
			pinfo->type = MIPI_CMD_PANEL;

		tmp = 0;
		data = of_get_property(next,
			"qcom,mdss-dsi-pixel-packing", NULL);
		if (data && !strcmp(data, "loose"))
			tmp = 1;
		rc = mdss_panel_dt_get_dst_fmt(pinfo->bpp,
			pinfo->mipi.mode, tmp,
			&(pinfo->mipi.dst_format));
		if (rc) {
			pr_debug("%s: problem dst format. Set Default\n",
				__func__);
			pinfo->mipi.dst_format =
				DSI_VIDEO_DST_FORMAT_RGB888;
		}
		pdest = of_get_property(next,
			"qcom,mdss-dsi-panel-destination", NULL);

		if (strlen(pdest) != 9) {
			pr_err("%s: Unknown pdest specified\n", __func__);
			goto error;
		}
		if (!strncmp(pdest, "display_1", 9))
			pinfo->pdest = DISPLAY_1;
		else if (!strncmp(pdest, "display_2", 9))
			pinfo->pdest = DISPLAY_2;
		else {
			pr_err("%s: pdest not specified. Set Default\n",
								__func__);
			pinfo->pdest = DISPLAY_1;
		}
		rc = of_property_read_u32(next,
			"qcom,mdss-dsi-h-front-porch", &tmp);
		pinfo->lcdc.h_front_porch = (!rc ? tmp : 6);
		rc = of_property_read_u32(next,
			"qcom,mdss-dsi-h-back-porch", &tmp);
		pinfo->lcdc.h_back_porch = (!rc ? tmp : 6);
		rc = of_property_read_u32(next,
			"qcom,mdss-dsi-h-pulse-width", &tmp);
		pinfo->lcdc.h_pulse_width = (!rc ? tmp : 2);
		rc = of_property_read_u32(next,
			"qcom,mdss-dsi-h-sync-skew", &tmp);
		pinfo->lcdc.hsync_skew = (!rc ? tmp : 0);
		rc = of_property_read_u32(next,
			"qcom,mdss-dsi-v-back-porch", &tmp);
		pinfo->lcdc.v_back_porch = (!rc ? tmp : 6);
		rc = of_property_read_u32(next,
			"qcom,mdss-dsi-v-front-porch", &tmp);
		pinfo->lcdc.v_front_porch = (!rc ? tmp : 6);
		rc = of_property_read_u32(next,
			"qcom,mdss-dsi-v-pulse-width", &tmp);
		pinfo->lcdc.v_pulse_width = (!rc ? tmp : 2);
		rc = of_property_read_u32(next,
			"qcom,mdss-dsi-underflow-color", &tmp);
		pinfo->lcdc.underflow_clr = (!rc ? tmp : 0xff);
		rc = of_property_read_u32(next,
			"qcom,mdss-dsi-border-color", &tmp);
		pinfo->lcdc.border_clr = (!rc ? tmp : 0);

		ctrl_pdata->bklt_ctrl = UNKNOWN_CTRL;
		data = of_get_property(next,
			"qcom,mdss-dsi-bl-pmic-control-type", NULL);
		if (data) {
			if (!strncmp(data, "bl_ctrl_wled", 12)) {
				led_trigger_register_simple("bkl-trigger",
					&bl_led_trigger);
				pr_debug("%s: SUCCESS WLED TRIGGER register\n",
					__func__);
				ctrl_pdata->bklt_ctrl = BL_WLED;
			} else if (!strncmp(data, "bl_ctrl_pwm", 11)) {
				ctrl_pdata->bklt_ctrl = BL_PWM;
				rc = of_property_read_u32(next,
					"qcom,mdss-dsi-bl-pmic-pwm-frequency",
					&tmp);
				if (rc) {
					pr_err("%s:%d, Error, pwm_period\n",
							__func__, __LINE__);
				goto error;
				}
				ctrl_pdata->pwm_period = tmp;
				rc = of_property_read_u32(next,
					"qcom,mdss-dsi-bl-pmic-bank-select",
					&tmp);
				if (rc) {
					pr_err("%s:%d, Error, lpg channel\n",
							__func__, __LINE__);
					goto error;
				}
				ctrl_pdata->pwm_lpg_chan = tmp;
				tmp = of_get_named_gpio(next,
					"qcom,mdss-dsi-pwm-gpio", 0);
				ctrl_pdata->pwm_pmic_gpio = tmp;
			} else if (!strncmp(data, "bl_ctrl_dcs", 11)) {
				ctrl_pdata->bklt_ctrl = BL_DCS_CMD;
			}
		}
		rc = of_property_read_u32(next,
			"qcom,mdss-dsi-bl-min-level", &tmp);
		pinfo->bl_min = (!rc ? tmp : 0);
		rc = of_property_read_u32(next,
			"qcom,mdss-dsi-bl-max-level", &tmp);
		pinfo->bl_max = (!rc ? tmp : 255);

		rc = of_property_read_u32(next,
			"qcom,mdss-dsi-interleave-mode", &tmp);
		pinfo->mipi.interleave_mode = (!rc ? tmp : 0);

		pinfo->mipi.vsync_enable = of_property_read_bool(next,
			"qcom,mdss-dsi-te-check-enable");
		pinfo->mipi.hw_vsync_mode = of_property_read_bool(next,
			"qcom,mdss-dsi-te-using-te-pin");

		rc = of_property_read_u32(next,
			"qcom,mdss-dsi-h-sync-pulse", &tmp);
		pinfo->mipi.pulse_mode_hsa_he = (!rc ? tmp : false);

		pinfo->mipi.tx_eot_append = of_property_read_bool(next,
			"somc,mdss-dsi-eot-append");
		pinfo->mipi.force_clk_lane_hs = of_property_read_bool(next,
			"somc,mdss-dsi-clklane-in-hs");

		pinfo->mipi.no_max_pkt_size = of_property_read_bool(next,
			"somc,mdss-dsi-no-max-packet-size");

		pinfo->mipi.hfp_power_stop = of_property_read_bool(next,
			"qcom,mdss-dsi-hfp-power-mode");
		pinfo->mipi.hsa_power_stop = of_property_read_bool(next,
			"qcom,mdss-dsi-hsa-power-mode");
		pinfo->mipi.hbp_power_stop = of_property_read_bool(next,
			"qcom,mdss-dsi-hbp-power-mode");
		pinfo->mipi.bllp_power_stop = of_property_read_bool(next,
			"qcom,mdss-dsi-bllp-power-mode");
		pinfo->mipi.eof_bllp_power_stop = of_property_read_bool(
			next, "qcom,mdss-dsi-bllp-eof-power-mode");

		pinfo->mipi.traffic_mode = DSI_NON_BURST_SYNCH_PULSE;
		data = of_get_property(next,
			"qcom,mdss-dsi-traffic-mode", NULL);
		if (data) {
			if (!strcmp(data, "non_burst_sync_event"))
				pinfo->mipi.traffic_mode
					 = DSI_NON_BURST_SYNCH_EVENT;
			else if (!strcmp(data, "burst_mode"))
				pinfo->mipi.traffic_mode = DSI_BURST_MODE;
		}

		rc = of_property_read_u32(next,
			"qcom,mdss-dsi-te-dcs-command", &tmp);
		pinfo->mipi.insert_dcs_cmd = (!rc ? tmp : 1);
		rc = of_property_read_u32(next,
			"qcom,mdss-dsi-te-v-sync-continue-lines", &tmp);
		pinfo->mipi.wr_mem_continue = (!rc ? tmp : 0x3c);
		rc = of_property_read_u32(next,
			"qcom,mdss-dsi-te-v-sync-rd-ptr-irq-line", &tmp);
		pinfo->mipi.wr_mem_start = (!rc ? tmp : 0x2c);
		rc = of_property_read_u32(next,
			"qcom,mdss-dsi-te-pin-select", &tmp);
		pinfo->mipi.te_sel = (!rc ? tmp : 1);
		rc = of_property_read_u32(next,
			"qcom,mdss-dsi-virtual-channel-id", &tmp);
		pinfo->mipi.vc = (!rc ? tmp : 0);

		pinfo->mipi.rgb_swap = DSI_RGB_SWAP_RGB;
		data = of_get_property(next,
			"qcom,mdss-dsi-color-order", NULL);
		if (data) {
			if (!strcmp(data, "rgb_swap_rbg"))
				pinfo->mipi.rgb_swap = DSI_RGB_SWAP_RBG;
			else if (!strcmp(data, "rgb_swap_bgr"))
				pinfo->mipi.rgb_swap = DSI_RGB_SWAP_BGR;
			else if (!strcmp(data, "rgb_swap_brg"))
				pinfo->mipi.rgb_swap = DSI_RGB_SWAP_BRG;
			else if (!strcmp(data, "rgb_swap_grb"))
				pinfo->mipi.rgb_swap = DSI_RGB_SWAP_GRB;
			else if (!strcmp(data, "rgb_swap_gbr"))
				pinfo->mipi.rgb_swap = DSI_RGB_SWAP_GBR;
		}

		pinfo->mipi.data_lane0 = of_property_read_bool(next,
			"qcom,mdss-dsi-lane-0-state");
		pinfo->mipi.data_lane1 = of_property_read_bool(next,
			"qcom,mdss-dsi-lane-1-state");
		pinfo->mipi.data_lane2 = of_property_read_bool(next,
			"qcom,mdss-dsi-lane-2-state");
		pinfo->mipi.data_lane3 = of_property_read_bool(next,
			"qcom,mdss-dsi-lane-3-state");

		rc = of_property_read_u32(next,
			"qcom,mdss-dsi-t-clk-pre", &tmp);
		pinfo->mipi.t_clk_pre = (!rc ? tmp : 0x24);
		rc = of_property_read_u32(next,
			"qcom,mdss-dsi-t-clk-post", &tmp);
		pinfo->mipi.t_clk_post = (!rc ? tmp : 0x03);

		rc = of_property_read_u32(next, "qcom,mdss-dsi-stream", &tmp);
		pinfo->mipi.stream = (!rc ? tmp : 0);

		data = of_get_property(next,
			"qcom,mdss-dsi-panel-mode-gpio-state", &tmp);
		if (data) {
			if (!strcmp(data, "high"))
				pinfo->mode_gpio_state = MODE_GPIO_HIGH;
			else if (!strcmp(data, "low"))
				pinfo->mode_gpio_state = MODE_GPIO_LOW;
		} else {
			pinfo->mode_gpio_state = MODE_GPIO_NOT_VALID;
		}

		rc = of_property_read_u32(next,
			"qcom,mdss-dsi-panel-framerate", &tmp);
		pinfo->mipi.frame_rate = (!rc ? tmp : 60);
		rc = of_property_read_u32(next,
			"qcom,mdss-dsi-panel-clock-rate", &tmp);
		pinfo->clk_rate = (!rc ? tmp : 0);

		data = of_get_property(next,
			"qcom,panel-phy-regulatorSettings", &len);
		if (data && (len == 7)) {
			for (i = 0; i < len; i++) {
				pinfo->mipi.dsi_phy_db.regulator[i] =
						data[i];
			}
		}

		data = of_get_property(next,
			"qcom,mdss-dsi-panel-timings", &len);
		if ((!data) || (len != 12)) {
			pr_err("%s:%d, Unable to read Phy timing settings",
				   __func__, __LINE__);
			goto error;
		}
		for (i = 0; i < len; i++)
			pinfo->mipi.dsi_phy_db.timing[i] = data[i];

		data = of_get_property(next,
			"qcom,panel-phy-strengthCtrl", &len);
		if ((data) && (len == 2)) {
			pinfo->mipi.dsi_phy_db.strength[0] = data[0];
			pinfo->mipi.dsi_phy_db.strength[1] = data[1];
		}

		data = of_get_property(next,
			"somc,mdss-dsi-lane-config", &len);
		if ((data) && (len == 45)) {
			for (i = 0; i < len; i++) {
				pinfo->mipi.dsi_phy_db.lanecfg[i] =
						data[i];
			}
		}

		mdss_dsi_parse_fbc_params(next, pinfo);

		mdss_dsi_parse_trigger(next, &(pinfo->mipi.mdp_trigger),
			"qcom,mdss-dsi-mdp-trigger");

		mdss_dsi_parse_trigger(next, &(pinfo->mipi.dma_trigger),
			"qcom,mdss-dsi-dma-trigger");

		mdss_dsi_parse_lane_swap(next, &(pinfo->mipi.dlane_swap));

		mdss_dsi_parse_reset_seq(next, pinfo->rst_seq,
			&(pinfo->rst_seq_len),
			"qcom,mdss-dsi-reset-sequence");

		mdss_dsi_parse_dcs_cmds(next, &spec_pdata->einit_cmds,
			"somc,mdss-dsi-early-init-command", NULL);

		mdss_dsi_parse_dcs_cmds(next, &spec_pdata->init_cmds,
			"somc,mdss-dsi-init-command", NULL);

		mdss_dsi_parse_dcs_cmds(next, &spec_pdata->on_cmds,
			"qcom,mdss-dsi-on-command",
			"qcom,mdss-dsi-on-command-state");

		mdss_dsi_parse_dcs_cmds(next, &spec_pdata->off_cmds,
		"qcom,mdss-dsi-off-command", "qcom,mdss-dsi-off-command-state");

		mdss_dsi_parse_dcs_cmds(next, &spec_pdata->id_read_cmds,
			"somc,mdss-dsi-id-read-command", NULL);

		rc = of_property_read_u32_array(next,
			"somc,mdss-phy-size-mm", res, 2);
		if (rc)
			pr_err("%s:%d, panel physical size not specified\n",
							__func__, __LINE__);
		pinfo->width = (!rc ? res[0] : 0);
		pinfo->height = (!rc ? res[1] : 0);

		rc = of_property_read_u32(next,
				"somc,mdss-dsi-disp-on-in-hs", &tmp);
		spec_pdata->disp_on_in_hs = (!rc ? tmp : 0);

		mdss_dsi_parse_dcs_cmds(next,
				&spec_pdata->cabc_on_cmds,
				"somc,mdss-dsi-cabc-on-command", NULL);

		mdss_dsi_parse_dcs_cmds(next,
				&spec_pdata->cabc_off_cmds,
				"somc,mdss-dsi-cabc-off-command", NULL);

		rc = of_property_read_u32(next,
				"somc,mdss-dsi-cabc-enabled", &tmp);
		spec_pdata->cabc_enabled = !rc ? tmp : 0;


		break;
	}

	return 0;
error:
	return -EINVAL;
}

int mdss_dsi_panel_init(struct device_node *node,
	struct mdss_dsi_ctrl_pdata *ctrl_pdata,
	bool cmd_cfg_cont_splash)
{
	int rc = 0;
	char *path_name = "mdss_dsi_panel_driver";
	struct device_node *dsi_ctrl_np = NULL;
	struct platform_device *ctrl_pdev = NULL;
	bool cont_splash_enabled;
	bool partial_update_enabled;
	struct mdss_panel_specific_pdata *spec_pdata = NULL;

	dev_set_name(&virtdev, "%s", path_name);
	rc = device_register(&virtdev);
	if (rc) {
		pr_err("%s: device_register rc = %d\n", __func__, rc);
		return rc;
	}

	rc = register_attributes(&virtdev);
	if (rc) {
		pr_err("%s: register_attributes rc = %d\n", __func__, rc);
		goto error_attr;
	}

	pr_debug("%s:%d, debug info", __func__, __LINE__);

	spec_pdata = ctrl_pdata->spec_pdata;
	if (!spec_pdata) {
		pr_err("%s: Invalid input data\n", __func__);
		rc = -EINVAL;
		goto error;
	}

	dsi_ctrl_np = of_parse_phandle(node,
				       "qcom,mdss-dsi-panel-controller", 0);
	if (!dsi_ctrl_np) {
		dev_err(&virtdev, "%s: Dsi controller node not initialized\n",
								__func__);
		rc = -EPROBE_DEFER;
		goto error;
	}

	ctrl_pdev = of_find_device_by_node(dsi_ctrl_np);

	rc = dev_set_drvdata(&virtdev, ctrl_pdata);

	vsn_gpio = of_get_named_gpio(node, "somc,vsn-gpio", 0);
	vsp_gpio = of_get_named_gpio(node, "somc,vsp-gpio", 0);
	if (!gpio_is_valid(vsn_gpio)) {
		dev_err(&virtdev, "%s: vsn_gpio=%d Invalid\n",
				__func__, vsn_gpio);
		rc = -EINVAL;
		goto error;
	}
	if (!gpio_is_valid(vsp_gpio)) {
		dev_err(&virtdev, "%s: vsp_gpio=%d Invalid\n",
				__func__, vsp_gpio);
		rc = -EINVAL;
		goto error;
	}

	rc = gpio_request(vsn_gpio, "lcd_vsn");
	if (rc) {
		dev_info(&virtdev, "%s: Error requesting VSN GPIO %d\n",
				__func__, rc);
		goto error;
	}
	rc = gpio_request(vsp_gpio, "lcd_vsp");
	if (rc) {
		dev_info(&virtdev, "%s: Error requesting VSP GPIO %d\n",
				__func__, rc);
		gpio_free(vsn_gpio);
		goto error;
	}
	gpio_direction_output(vsn_gpio, 0);
	gpio_direction_output(vsp_gpio, 0);

	lcd_id = of_get_named_gpio(node, "somc,dric-gpio", 0);
	if (!gpio_is_valid(lcd_id)) {
		dev_err(&virtdev, "%s:%d, lcd_id gpio not specified\n",
						__func__, __LINE__);
		goto error_lcd_power;
	} else {
		pr_debug("%s: lcd_id gpio is %d\n",
						__func__, lcd_id);
	}

	if (!spec_pdata->driver_ic) {
		rc = gpio_request(lcd_id, "lcd_id");
		if (rc) {
			dev_err(&virtdev, "request lcd id gpio failed, rc=%d\n",
				rc);
			spec_pdata->driver_ic = PANEL_DRIVER_IC_NONE;
			goto error_lcd_power;
		}
	}

	rc = gpio_direction_input(lcd_id);
	if (rc) {
		dev_err(&virtdev, "set_direction for lcd_id failed, rc=%d",
			rc);
		gpio_free(lcd_id);
		spec_pdata->driver_ic = PANEL_DRIVER_IC_NONE;
		goto error_lcd_power;
	}

	mdss_dsi_panel_power_detect(ctrl_pdev, 1);
	spec_pdata->driver_ic = gpio_get_value(lcd_id);
	dev_info(&virtdev, "LCD id=%d\n", spec_pdata->driver_ic);
	mdss_dsi_panel_power_detect(ctrl_pdev, 0);

	rc = mdss_panel_parse_dt(node, ctrl_pdata,
		spec_pdata->driver_ic, NULL);
	if (rc)
		goto error_parse_dt;

	if (cmd_cfg_cont_splash)
		cont_splash_enabled = of_property_read_bool(node,
				"qcom,cont-splash-enabled");
	else
		cont_splash_enabled = false;
	if (!cont_splash_enabled) {
		pr_info("%s:%d Continuous splash flag not found.\n",
				__func__, __LINE__);
		ctrl_pdata->panel_data.panel_info.cont_splash_enabled = 0;
	} else {
		pr_info("%s:%d Continuous splash flag enabled.\n",
				__func__, __LINE__);

		ctrl_pdata->panel_data.panel_info.cont_splash_enabled = 1;
	}

	partial_update_enabled = of_property_read_bool(node,
						"qcom,partial-update-enabled");
	if (partial_update_enabled) {
		pr_info("%s:%d Partial update enabled.\n", __func__, __LINE__);
		ctrl_pdata->panel_data.panel_info.partial_update_enabled = 1;
		ctrl_pdata->partial_update_fnc = mdss_dsi_panel_partial_update;
	} else {
		pr_info("%s:%d Partial update disabled.\n", __func__, __LINE__);
		ctrl_pdata->panel_data.panel_info.partial_update_enabled = 0;
		ctrl_pdata->partial_update_fnc = NULL;
	}

	spec_pdata->detect = mdss_dsi_panel_detect;
	spec_pdata->update_panel = mdss_dsi_panel_update_panel_info;

	spec_pdata->panel_power_on = mdss_dsi_panel_power_on_panel;
	spec_pdata->reset = mdss_dsi_panel_reset_panel;
	spec_pdata->disp_on = mdss_dsi_panel_disp_on;
	spec_pdata->update_fps = mdss_dsi_panel_fps_data_update;

	ctrl_pdata->on = mdss_dsi_panel_on;
	ctrl_pdata->off = mdss_dsi_panel_off;
	ctrl_pdata->panel_data.set_backlight = mdss_dsi_panel_bl_ctrl;

	mdss_dsi_panel_fps_data_init(&fpsd);

	return 0;
error_parse_dt:
	gpio_free(lcd_id);
error_lcd_power:
	gpio_free(vsp_gpio);
	gpio_free(vsn_gpio);
error:
	remove_attributes(&virtdev);
error_attr:
	device_unregister(&virtdev);
	return rc;
}

void mdss_dsi_panel_reset(struct mdss_panel_data *pdata, int enable)
{
	mdss_dsi_panel_reset_panel(pdata, enable);
}
