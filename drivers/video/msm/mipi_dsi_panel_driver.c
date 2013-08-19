/* drivers/video/msm/mipi_dsi_panel_driver.c
 *
 * Copyright (C) 2012-2013 Sony Mobile Communications AB.
 *
 * Author: Johan Olson <johan.olson@sonymobile.com>
 * Author: Joakim Wesslen <joakim.wesslen@sonymobile.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2; as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

/* #define DEBUG */

#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/time.h>
#include <linux/mutex.h>

#include <video/mipi_dsi_panel.h>
#include "msm_fb.h"
#include "mdp4.h"
#include "mipi_dsi.h"
#include "mipi_dsi_panel_driver.h"

#ifdef CONFIG_FB_MSM_MDP303
#define DSI_VIDEO_BASE	0xF0000 /* Taken from mdp_dma_dsi_video.c */
#else
#define DSI_VIDEO_BASE	0xE0000 /* Taken from mdp4_overlay_dsi_video.c */
#endif

#define DEFAULT_FPS_LOG_INTERVAL 100
#define DEFAULT_FPS_ARRAY_SIZE 120
#define NVRW_RETRY		10
#define NVRW_SEPARATOR_POS	2
#define NVRW_ONE_PARAM_SIZE	3
#define NVRW_DATA_SIZE		((NVRW_NUM_E6_PARAM + NVRW_NUM_E7_PARAM + \
				  NVRW_NUM_DE_PARAM) * NVRW_ONE_PARAM_SIZE)
#define NVRW_PANEL_OFF_MSLEEP	100
#define NVRW_USEFUL_DE_PARAM	4
#define NVRW_ERASE_RES_OK	0xB9
#define NVRW_STATUS_RES_NG	0x00

#define calc_coltype_num(val, numpart, maxval)\
	((val >= maxval) ? numpart - 1 : val / (maxval / numpart))

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

static const struct mipi_dsi_phy_ctrl default_dsi_phy_db[] = {
	{
		/* regulator */
		{0x03, 0x0a, 0x04, 0x00, 0x20},
		/* timing   */
		{0x78, 0x1a, 0x11, 0x00, 0x3e, 0x43, 0x16, 0x1d,
		 0x1d, 0x03, 0x04, 0xa0},
		/* phy ctrl */
		{0x5f, 0x00, 0x00, 0x10},
		/* strength */
		{0xff, 0x00, 0x06, 0x00},
		/* pll control */
		{0x00, 0x8f, 0x31, 0xd9, 0x00, 0x50, 0x48, 0x63,
		 0x41, 0x0f, 0x03,
		 0x00, 0x14, 0x03, 0x00, 0x02, 0x00, 0x20, 0x00, 0x01 },
	},
};

static struct msm_panel_info default_pinfo = {
	.type = MIPI_VIDEO_PANEL,
	.pdest = DISPLAY_1,
	.bpp = 24,

	.mipi.mode = DSI_VIDEO_MODE,
	.mipi.data_lane0 = TRUE,
	.mipi.data_lane1 = TRUE,
	.mipi.data_lane2 = TRUE,
	.mipi.data_lane3 = TRUE,
	.mipi.tx_eot_append = TRUE,
	.mipi.t_clk_post = 0x04,
	.mipi.t_clk_pre = 0x1B,
#if defined(CONFIG_FB_MSM_MIPI_R63306_JDC_MDZ50) || \
		defined(CONFIG_FB_MSM_MIPI_R63306_SHARP_LS046K3SX01)
	.mipi.esc_byte_ratio = 4,
#endif
	.mipi.stream = 0,
	.mipi.mdp_trigger = DSI_CMD_TRIGGER_SW,
	.mipi.dma_trigger = DSI_CMD_TRIGGER_SW,
	.mipi.frame_rate  = 60,
	.mipi.dsi_phy_db =
		(struct mipi_dsi_phy_ctrl *)default_dsi_phy_db,
};

static struct mdp_pcc_cfg_data *color_calib;

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

static void mipi_dsi_panel_fps_array_clear(struct fps_data *fps)
{
	memset(fps->fa, 0, sizeof(fps->fa));
	fps_array_cnt = 0;
}

static void mipi_dsi_panel_fps_data_init(struct fps_data *fps)
{
	fps->frame_counter = 0;
	fps->frame_counter_last = 0;
	fps->log_interval = DEFAULT_FPS_LOG_INTERVAL;
	fps->fpks = 0;
	fps->fa_last_array_pos = 0;
	getrawmonotonic(&fps->timestamp_last);
	mutex_init(&fps->fps_lock);
}

void mipi_dsi_panel_fps_data_update(struct msm_fb_data_type *mfd)
{
	/* Only count fps on primary display */
	if (mfd->index == 0)
		update_fps_data(&fpsd);
}

static bool is_read_cmd(int dtype)
{
	return (dtype == DTYPE_DCS_READ || dtype == DTYPE_GEN_READ ||
		dtype == DTYPE_GEN_READ1 || dtype == DTYPE_GEN_READ2);
}

static int panel_execute_cmd(struct msm_fb_data_type *mfd,
			     struct mipi_dsi_data *dsi_data,
			     const struct panel_cmd *pcmd,
			     int nbr_bytes_to_read)
{
	struct device *dev = &mfd->panel_pdev->dev;
	int n;
	int ret = 0;

	if (!pcmd) {
		dev_err(dev, "%s: no command\n", __func__);
		ret = -EINVAL;
		goto exit;
	}

	for (n = 0; ; ++n)
		switch (pcmd[n].type) {
		case CMD_END:
			dev_dbg(dev, "CMD_END\n");
			goto exit;
			break;
		case CMD_WAIT_MS:
			dev_dbg(dev, "%s: CMD_WAIT_MS = %d ms\n",
						__func__, pcmd[n].payload.data);
			msleep(pcmd[n].payload.data);
			break;
		case CMD_DSI:
			dev_dbg(dev, "%s: CMD_DSI\n", __func__);
			/* Sufficient to check if first cmd is a read, then
			 * it is impled that all are of the same type */
			if (is_read_cmd(
				pcmd[n].payload.dsi_payload.dsi[0].dtype)) {

				dev_dbg(dev, "%s: CMD_DSI READ\n", __func__);
				mipi_dsi_cmds_rx(mfd, &dsi_data->tx_buf,
					&dsi_data->rx_buf,
					pcmd[n].payload.dsi_payload.dsi,
					nbr_bytes_to_read);
			} else {
				mipi_dsi_cmds_tx(&dsi_data->tx_buf,
					pcmd[n].payload.dsi_payload.dsi,
					pcmd[n].payload.dsi_payload.cnt);
			}
			break;
		case CMD_RESET:
			dev_dbg(dev, "%s: CMD_RESET lvl=%d\n", __func__,
							pcmd[n].payload.data);
			if (dsi_data->lcd_reset)
				ret = dsi_data->lcd_reset(pcmd[n].payload.data);
			if (ret)
				goto exit;
			break;
		case CMD_PLATFORM:
			dev_dbg(dev, "%s: CMD_PLATFORM enable=%d\n", __func__,
							pcmd[n].payload.data);
			if (dsi_data->lcd_power)
				ret = dsi_data->lcd_power(pcmd[n].payload.data);
			if (ret)
				goto exit;
			break;
		default:
			dev_err(dev, "%s: Unknown command type!\n",
								__func__);
		}
exit:
	return ret;
}

static int panel_id_reg_check(struct msm_fb_data_type *mfd,
			      struct mipi_dsi_data *dsi_data,
			      const struct panel *panel)
{
	struct device *dev = &mfd->panel_pdev->dev;
	int i;
	int ret = 0;

	dev_dbg(dev, "%s\n", __func__);

	mutex_lock(&mfd->dma->ov_mutex);
	ret = panel_execute_cmd(mfd, dsi_data, panel->pctrl->read_id,
								panel->id_num);
	mutex_unlock(&mfd->dma->ov_mutex);
	if (ret) {
		dev_err(dev, "%s: read id failed\n", __func__);
		goto exit;
	}

	for (i = 0; i < panel->id_num; i++) {
		if ((i >= dsi_data->rx_buf.len) ||
			((dsi_data->rx_buf.data[i] != panel->id[i]) &&
				(panel->id[i] != PANEL_SKIP_ID))) {
			ret = -ENODEV;
			goto exit;
		}
	}
exit:
	return ret;
}

static int panel_update_config(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
	struct fb_info *fbi;
	struct msm_panel_info *pinfo;
	struct mipi_panel_info *mipi;
	uint8 lanes = 0, bpp;
	uint32 h_period, v_period, dsi_pclk_rate;

	dev_dbg(&pdev->dev, "%s: pdev = %p, pdev->name = %s\n", __func__,
							pdev, pdev->name);

	mfd = platform_get_drvdata(pdev);
	pinfo = &mfd->panel_info;
	fbi = mfd->fbi;
	fbi->var.pixclock = pinfo->clk_rate;
	fbi->var.left_margin = pinfo->lcdc.h_back_porch;
	fbi->var.right_margin = pinfo->lcdc.h_front_porch;
	fbi->var.upper_margin = pinfo->lcdc.v_back_porch;
	fbi->var.lower_margin = pinfo->lcdc.v_front_porch;
	fbi->var.hsync_len = pinfo->lcdc.h_pulse_width;
	fbi->var.vsync_len = pinfo->lcdc.v_pulse_width;

	mipi = &pinfo->mipi;

	if (mipi->data_lane3)
		lanes += 1;
	if (mipi->data_lane2)
		lanes += 1;
	if (mipi->data_lane1)
		lanes += 1;
	if (mipi->data_lane0)
		lanes += 1;

	switch (mipi->dst_format) {
	case DSI_CMD_DST_FORMAT_RGB888:
	case DSI_VIDEO_DST_FORMAT_RGB888:
	case DSI_VIDEO_DST_FORMAT_RGB666_LOOSE:
		bpp = 3;
		break;
	case DSI_CMD_DST_FORMAT_RGB565:
	case DSI_VIDEO_DST_FORMAT_RGB565:
		bpp = 2;
		break;
	default:
		bpp = 3;	/* Default format set to RGB888 */
		break;
	}

	if (pinfo->type == MIPI_VIDEO_PANEL && !pinfo->clk_rate) {
		h_period = pinfo->lcdc.h_pulse_width
				+ pinfo->lcdc.h_back_porch
				+ pinfo->xres
				+ pinfo->lcdc.h_front_porch;

		v_period = pinfo->lcdc.v_pulse_width
				+ pinfo->lcdc.v_back_porch
				+ pinfo->yres
				+ pinfo->lcdc.v_front_porch;

		if (lanes > 0) {
			pinfo->clk_rate =
			((h_period * v_period * (mipi->frame_rate) * bpp * 8)
			   / lanes);
		} else {
			dev_err(&pdev->dev,
				"%s: forcing mipi_dsi lanes to 1\n", __func__);
			pinfo->clk_rate =
				(h_period * v_period
					 * (mipi->frame_rate) * bpp * 8);
		}
	}
	pll_divider_config.clk_rate = pinfo->clk_rate;

	mipi_dsi_clk_div_config(bpp, lanes, &dsi_pclk_rate);

	if ((dsi_pclk_rate < 3300000) || (dsi_pclk_rate > 200000000))
		dsi_pclk_rate = 35000000;
	mipi->dsi_pclk_rate = dsi_pclk_rate;

	return 0;
}

static void mipi_dsi_update_lane_cfg(const struct mipi_dsi_lane_cfg *plncfg)
{
	int i, j, ln_offset;

	ln_offset = 0x300;
	for (i = 0; i < MIPI_DSI_NUM_PHY_LN; i++) {
		/* DSI1_DSIPHY_LN_CFG */
		for (j = 0; j < 3; j++)
			MIPI_OUTP(MIPI_DSI_BASE + ln_offset + j * 4,
				plncfg->ln_cfg[i][j]);
		/* DSI1_DSIPHY_LN_TEST_DATAPATH */
		MIPI_OUTP(MIPI_DSI_BASE + ln_offset + 0x0c,
			plncfg->ln_dpath[i]);
		/* DSI1_DSIPHY_LN_TEST_STR */
		for (j = 0; j < 2; j++)
			MIPI_OUTP(MIPI_DSI_BASE + ln_offset + 0x14 + j * 4,
				plncfg->ln_str[i][j]);

		ln_offset += 0x40;
	}

	/* DSI1_DSIPHY_LNCK_CFG */
	for (i = 0; i < 3; i++)
		MIPI_OUTP(MIPI_DSI_BASE + 0x0400 + i * 4,
			plncfg->lnck_cfg[i]);
	/* DSI1_DSIPHY_LNCK_TEST_DATAPATH */
	MIPI_OUTP(MIPI_DSI_BASE + 0x040c, plncfg->lnck_dpath);
	/* DSI1_DSIPHY_LNCK_TEST_STR */
	for (i = 0; i < 2; i++)
		MIPI_OUTP(MIPI_DSI_BASE + 0x0414 + i * 4,
			plncfg->lnck_str[i]);
}

static int panel_sleep_out_display_off(struct msm_fb_data_type *mfd,
				       struct mipi_dsi_data *dsi_data)
{
	struct device *dev = &mfd->panel_pdev->dev;
	int ret = 0;

	dev_dbg(dev, "%s: Execute display_init\n", __func__);
	ret = panel_execute_cmd(mfd, dsi_data,
				dsi_data->panel->pctrl->display_init, 0);
	if (ret)
		dev_err(&mfd->panel_pdev->dev, "display_init failed\n");
	return ret;
}

static int panel_display_on(struct msm_fb_data_type *mfd,
			    struct mipi_dsi_data *dsi_data)
{
	struct device *dev = &mfd->panel_pdev->dev;
	int ret = 0;

	dev_dbg(dev, "%s: Execute display on\n", __func__);
	mipi_set_tx_power_mode(0);
	ret = panel_execute_cmd(mfd, dsi_data,
					dsi_data->panel->pctrl->display_on, 0);
	if (ret) {
		dev_err(dev, "%s: display_on failed\n", __func__);
		goto exit;
	}
	dev_info(dev, "%s: DISPLAY_ON sent\n", __func__);
exit:
	return ret;
}

static int panel_on(struct platform_device *pdev)
{
	struct mipi_dsi_data *dsi_data;
	struct msm_fb_data_type *mfd;
	struct device *dev;
	bool skip_display_on = false;
	int ret = 0;

	mfd = platform_get_drvdata(pdev);
	if (!mfd) {
		ret = -ENODEV;
		goto exit;
	}
	if (mfd->key != MFD_KEY) {
		ret = -EINVAL;
		goto exit;
	}

	dev = &mfd->panel_pdev->dev;
	dev_dbg(dev, "%s\n", __func__);

	dsi_data = platform_get_drvdata(mfd->panel_pdev);
	if (!dsi_data) {
		ret = -ENODEV;
		goto exit;
	}

	/* Clear the buffer on resume */
	mutex_lock(&fpsd.fps_lock);
	mipi_dsi_panel_fps_array_clear(&fpsd);
	mutex_unlock(&fpsd.fps_lock);

	mutex_lock(&dsi_data->lock);
	dev_dbg(dev, "%s: state = %d\n", __func__, dsi_data->panel_state);

	if (!dsi_data->panel_detected)
		goto unlock_and_exit;

	if (!dsi_data->panel)
		goto unlock_and_exit;

	if (dsi_data->panel_state == PANEL_OFF ||
	    dsi_data->panel_state == DEBUGFS_POWER_OFF) {
		if (dsi_data->pcc_config)
			mdp4_pcc_cfg(dsi_data->pcc_config);
		if (dsi_data->panel && dsi_data->panel->plncfg)
			mipi_dsi_update_lane_cfg(dsi_data->panel->plncfg);

		dev_dbg(dev, "%s: call power on\n", __func__);
		if (dsi_data->lcd_power) {
			ret = dsi_data->lcd_power(TRUE);
			if (ret)
				goto unlock_and_exit;
		}
		if (dsi_data->panel_state == DEBUGFS_POWER_OFF)
			dsi_data->panel_state = DEBUGFS_POWER_ON;
	}

	if (dsi_data->panel_state == DEBUGFS_POWER_ON) {
		dev_dbg(dev, "%s: debugfs state, don't exit sleep\n", __func__);
		goto unlock_and_exit;
	}
	if (!dsi_data->nvrw_panel_detective)
		goto unlock_and_exit;

	if (dsi_data->panel_state == PANEL_OFF) {
		ret = panel_sleep_out_display_off(mfd, dsi_data);
		if (ret)
			goto unlock_and_exit;
		if (dsi_data->panel_data.controller_on_panel_on)
			skip_display_on = true;
		dsi_data->panel_state = PANEL_SLEEP_OUT;
	}

	/* Call display on depending on if it is setup to receive video */
	/* data data before display on or not */
	if (!skip_display_on && dsi_data->panel_state == PANEL_SLEEP_OUT) {
		ret = panel_display_on(mfd, dsi_data);
		if (ret)
			goto unlock_and_exit;
		dsi_data->panel_state = PANEL_ON;
	}
unlock_and_exit:
	mutex_unlock(&dsi_data->lock);
exit:
	return ret;
}

static int panel_off(struct platform_device *pdev)
{
	struct mipi_dsi_data *dsi_data;
	struct msm_fb_data_type *mfd;
	struct device *dev;
	int ret = 0;

	mfd = platform_get_drvdata(pdev);
	if (!mfd) {
		ret = -ENODEV;
		goto exit;
	}
	if (mfd->key != MFD_KEY) {
		ret = -EINVAL;
		goto exit;
	}

	dev = &mfd->panel_pdev->dev;
	dev_dbg(dev, "%s\n", __func__);

	dsi_data = platform_get_drvdata(mfd->panel_pdev);
	if (!dsi_data) {
		ret = -ENODEV;
		goto exit;
	}

	mutex_lock(&dsi_data->lock);

	if (!dsi_data->panel_detected) {
		/* We rely on that we have detected a panel the first time
		   this function is called */
		dsi_data->panel_detected = true;
		goto unlock_and_exit;
	}

	if (dsi_data->panel_state == DEBUGFS_POWER_ON) {
		dev_dbg(dev, "%s: debugfs state, power off\n", __func__);
		goto power_off;
	}

	if (!dsi_data->panel)
		goto unlock_and_exit;

	if (dsi_data->panel->disable_dsi_timing_genarator_at_off) {
		MDP_OUTP(MDP_BASE + DSI_VIDEO_BASE, 0);
		msleep(20);
	}

	dev_dbg(dev, "%s: Execute display off\n", __func__);
	/* Set to OFF even if commands fail below */
	dsi_data->panel_state = PANEL_OFF;
	ret = panel_execute_cmd(mfd, dsi_data,
					dsi_data->panel->pctrl->display_off, 0);
	if (ret) {
		dev_err(dev, "%s: display_off failed\n", __func__);
		goto unlock_and_exit;
	}
	dev_info(dev, "%s: DISPLAY_OFF sent\n", __func__);
power_off:
	if (dsi_data->lcd_power)
		ret = dsi_data->lcd_power(FALSE);
unlock_and_exit:
	mutex_unlock(&dsi_data->lock);
exit:
	return ret;
}

static struct msm_panel_info *detect_panel(struct msm_fb_data_type *mfd)
{
	int i;
	int ret;
	struct mipi_dsi_data *dsi_data;
	struct msm_fb_panel_data *pdata = NULL;
	struct device *dev;
	const struct panel *default_panel = NULL;

	dev = &mfd->panel_pdev->dev;
	dev_dbg(dev, "%s\n", __func__);

	dsi_data = platform_get_drvdata(mfd->panel_pdev);
	if (!dsi_data || !dsi_data->panels[0]) {
		dev_err(dev, "%s: Failed to detect panel, no panel data\n",
								__func__);
		return NULL;
	}

	mutex_lock(&dsi_data->lock);
	mipi_dsi_op_mode_config(DSI_CMD_MODE);
	for (i = 0; dsi_data->panels[i]; i++) {
		ret = panel_id_reg_check(mfd, dsi_data, dsi_data->panels[i]);
		if (dsi_data->panels[i]->id[0] == PANEL_SKIP_ID
			&& dsi_data->panels[i]->id_num == 1)
			default_panel = dsi_data->panels[i];
		if (!ret && !(dsi_data->panels[i]->id[0] == PANEL_SKIP_ID
			&& dsi_data->panels[i]->id_num == 1))
			break;
	}

	if (dsi_data->panels[i]) {
		dsi_data->panel = dsi_data->panels[i];
		dsi_data->nvrw_panel_detective = true;
		dev_info(dev, "%s: Found panel: %s\n", __func__,
							dsi_data->panel->name);
	} else {
		if (default_panel)
			dsi_data->panel = default_panel;
		dev_warn(dev, "%s: Failed to detect panel!\n", __func__);
	}

	if (!dsi_data->panel) {
		mutex_unlock(&dsi_data->lock);
		return NULL;
	}

	if (dsi_data->panel->send_video_data_before_display_on) {
		dev_info(dev, "%s: send_video_data_before_display_on\n",
								__func__);
		dsi_data->panel_data.controller_on_panel_on = panel_on;
		dsi_data->panel_data.power_on_panel_at_pan = 0;
		/* Also need to update platform data since that is */
		/* used in msm_fb */
		pdata = mfd->pdev->dev.platform_data;
		pdata->controller_on_panel_on = panel_on;
		pdata->power_on_panel_at_pan = 0;
	}

	dsi_data->panel_data.panel_info =
				*dsi_data->panel->pctrl->get_panel_info();
	dsi_data->panel_data.panel_info.width = dsi_data->panel->width;
	dsi_data->panel_data.panel_info.height = dsi_data->panel->height;
	dsi_data->panel_data.panel_info.mipi.dsi_pclk_rate =
				mfd->panel_info.mipi.dsi_pclk_rate;

	mipi_dsi_op_mode_config(dsi_data->panel_data.panel_info.mipi.mode);

	mutex_unlock(&dsi_data->lock);
	return &dsi_data->panel_data.panel_info;
}

static int find_subdivision_area(int u_data, int v_data)
{
	int row, col;
	int num_area = 0;

	if (v_data < CLR_SUB_AREA_V_START || CLR_RESOLUTION <= v_data)
		return 0;

	row = v_data / CLR_SUB_V_BLOCK_HEIGHT - 1;
	for (col = 0; col < CLR_SUB_COL_MAX; col++) {
		if (clr_sub_tbl[row][col].sub_area == 0)
			break;
		if (clr_sub_tbl[row][col].u_min <= u_data
			&& u_data <= clr_sub_tbl[row][col].u_max) {
			num_area = clr_sub_tbl[row][col].sub_area;
			break;
		}
	}
	return num_area;
}

static int get_pcc_data(struct msm_fb_data_type *mfd)
{
	int ret;
	struct mipi_dsi_data *dsi_data;
	struct device *dev;
	enum color_type color_type = 0;
	int u_data, v_data;
	int u, v;
	int num_area = 0;
	const struct mdp_pcc_cfg_rgb *cfg_rgb = NULL;

	dev = &mfd->panel_pdev->dev;
	dev_dbg(dev, "%s\n", __func__);

	dsi_data = platform_get_drvdata(mfd->panel_pdev);
	if (!dsi_data) {
		dev_err(dev, "%s: Failed to get pcc data\n", __func__);
		goto exit;
	}
	if (!dsi_data->panel->color_correction_tbl)
		goto exit;

	mipi_dsi_op_mode_config(DSI_CMD_MODE);
	mutex_lock(&mfd->dma->ov_mutex);
	if (dsi_data->panel->id_num >= 2 &&
			(dsi_data->panel->id[0] == PANEL_SKIP_ID &&
			dsi_data->panel->id[1] == PANEL_SKIP_ID))
		ret = panel_execute_cmd(mfd, dsi_data,
			dsi_data->panel->pctrl->read_id,
			dsi_data->panel->id_num);
	else
		ret = panel_execute_cmd(mfd, dsi_data,
			dsi_data->panel->pctrl->read_color,
			CLR_REG_DATA_NUM);
	mutex_unlock(&mfd->dma->ov_mutex);
	mipi_dsi_op_mode_config(dsi_data->panel_data.panel_info.mipi.mode);
	if (ret) {
		dev_err(dev, "%s: read color type failed\n", __func__);
		goto exit;
	}

	u_data = ((dsi_data->rx_buf.data[0] & 0x0F) << 2) |
		((dsi_data->rx_buf.data[1] >> 6) & 0x03);
	v_data = (dsi_data->rx_buf.data[1] & 0x3F);
	if (u_data == 0 && v_data == 0)
		goto exit;

	if (dsi_data->panel->color_subdivision_tbl)
		num_area = find_subdivision_area(u_data, v_data);
	if (num_area > 0) {
		cfg_rgb = &dsi_data->panel->color_subdivision_tbl[num_area - 1];
	} else {
		u = calc_coltype_num(u_data, CLR_NUM_PART, CLR_RESOLUTION);
		v = calc_coltype_num(v_data, CLR_NUM_PART, CLR_RESOLUTION);
		color_type = panel_color_type[v][u];
		if (color_type != CLR15_WHT)
			cfg_rgb = &dsi_data->panel->
				color_correction_tbl[color_type];
	}

	if (cfg_rgb != NULL && color_calib == NULL) {
		color_calib = kzalloc(sizeof(struct mdp_pcc_cfg_data),
				GFP_KERNEL);
		if (color_calib == NULL)
			return -ENOMEM;

		color_calib->block	= MDP_BLOCK_DMA_P;
		color_calib->ops	= 0x05;
		color_calib->r.r	= cfg_rgb->r;
		color_calib->g.g	= cfg_rgb->g;
		color_calib->b.b	= cfg_rgb->b;
		dsi_data->pcc_config	= color_calib;
		pcc_cfg_ptr		= color_calib;

		dev_dbg(dev, "%s (%d): r=%x g=%x b=%x area=%d ct=%d ud=%d vd=%d",
			__func__, __LINE__, cfg_rgb->r, cfg_rgb->g, cfg_rgb->b,
			num_area, color_type, u_data, v_data);
	}

exit:
	return 0;
}

#ifdef CONFIG_FB_MSM_RECOVER_PANEL
static struct msm_panel_info *nvm_detect_panel(
		struct msm_fb_data_type *mfd, char *id, int id_num)
{
	struct mipi_dsi_data *dsi_data = platform_get_drvdata(mfd->panel_pdev);
	struct msm_fb_panel_data *pdata;
	struct device *dev;
	int i, n;
	int min;

	dev = &mfd->panel_pdev->dev;
	dev_dbg(dev, "%s\n", __func__);
	for (i = 0; dsi_data->panels[i]; i++) {
		if (dsi_data->panels[i]->id[0] == PANEL_SKIP_ID
			&& dsi_data->panels[i]->id_num == 1)
			continue;	/* skip default panel */

		min = MIN(dsi_data->panels[i]->id_num, id_num);
		for (n = 0; n < min; n++)
			if (dsi_data->panels[i]->id[n] != id[n] &&
				dsi_data->panels[i]->id[n] != PANEL_SKIP_ID)
				break;
		if (n >= min)
			break;
	}
	if (!dsi_data->panels[i])
		return NULL;

	dsi_data->panel = dsi_data->panels[i];
	if (dsi_data->panel->send_video_data_before_display_on) {
		dsi_data->panel_data.controller_on_panel_on = panel_on;
		dsi_data->panel_data.power_on_panel_at_pan = 0;

		pdata = mfd->pdev->dev.platform_data;
		pdata->controller_on_panel_on = panel_on;
		pdata->power_on_panel_at_pan = 0;
	}
	dev_info(dev, "%s: Found panel: %s\n", __func__, dsi_data->panel->name);

	dsi_data->panel_data.panel_info =
				*dsi_data->panel->pctrl->get_panel_info();
	dsi_data->panel_data.panel_info.width = dsi_data->panel->width;
	dsi_data->panel_data.panel_info.height = dsi_data->panel->height;
	dsi_data->panel_data.panel_info.mipi.dsi_pclk_rate =
				mfd->panel_info.mipi.dsi_pclk_rate;

	return &dsi_data->panel_data.panel_info;
}

static int nvm_update_panel(struct msm_fb_data_type *mfd,
		const char *buf, size_t count)
{
	struct msm_panel_info *pinfo;
	const int num_ssv_id = NVRW_NUM_E6_PARAM * NVRW_ONE_PARAM_SIZE;
	char	ssv_id[num_ssv_id];
	char	*pos = ssv_id;
	char	id[NVRW_NUM_E6_PARAM];
	int	n, rc = -EINVAL;
	ulong	dat;

	if (count < NVRW_DATA_SIZE - 1)
		goto err_exit;

	buf += (NVRW_NUM_E7_PARAM + NVRW_NUM_DE_PARAM) * NVRW_ONE_PARAM_SIZE;
	memcpy(ssv_id, buf, num_ssv_id - 1);
	ssv_id[num_ssv_id - 1] = 0;

	for (n = 0; n < NVRW_NUM_E6_PARAM; n++, pos += NVRW_ONE_PARAM_SIZE) {
		pos[NVRW_SEPARATOR_POS] = 0;
		rc = kstrtoul(pos, 16, &dat);
		if (rc < 0)
			goto err_exit;
		id[n] = dat & 0xff;
	}
	pinfo = nvm_detect_panel(mfd, id, NVRW_NUM_E6_PARAM);
	if (!pinfo)
		goto err_exit;

	mutex_lock(&mfd->power_lock);
	rc = panel_next_off(mfd->pdev);
	if (rc)
		goto err_exit;
	mfd->panel_info = *pinfo;
	panel_update_config(mfd->pdev);
	rc = panel_next_on(mfd->pdev);
	mutex_unlock(&mfd->power_lock);

err_exit:
	return rc;
}
#endif

static int nvm_override_param(struct dsi_cmd_payload *pcmdp, char reg,
				int num_param, char *buf)
{
	ulong	dat;
	int	i, n;
	int	cnt = 0;
	int	rc;

	for (i = 0; i < pcmdp->cnt; i++) {
		if (pcmdp->dsi[i].payload[0] == reg)
			break;
	}
	if (i >= pcmdp->cnt)
		goto err_exit;
	for (n = 0; n < num_param; n++, buf += NVRW_ONE_PARAM_SIZE, cnt++) {
		buf[NVRW_SEPARATOR_POS] = 0;
		rc = kstrtoul(buf, 16, &dat);
		if (rc < 0)
			goto err_exit;
		pcmdp->dsi[i].payload[n + 1] = dat & 0xff;
	}
	return cnt;
err_exit:
	return 0;
}

static int nvm_override_data(struct msm_fb_data_type *mfd,
		const char *buf, int count)
{
	struct mipi_dsi_data	*dsi_data;
	struct dsi_cmd_payload	*pcmdp;
	char	work[NVRW_DATA_SIZE];
	char	*pos = work;
	int	cnt;

	dsi_data = platform_get_drvdata(mfd->panel_pdev);
	if (!dsi_data->panel->pnvrw_ctl)
		goto err_exit;
	if (count < NVRW_DATA_SIZE - 1)
		goto err_exit;
	memcpy(work, buf, NVRW_DATA_SIZE - 1);
	work[NVRW_DATA_SIZE - 1] = 0;
	/* override E7 register data */
	pcmdp = &dsi_data->panel->pnvrw_ctl->nvm_write_rsp->payload.dsi_payload;
	cnt = nvm_override_param(pcmdp, 0xE7, NVRW_NUM_E7_PARAM, pos);
	if (cnt == 0) {
		pr_err("%s:Override failure of the E7 parameters.\n", __func__);
		goto err_exit;
	}
	pos += cnt * NVRW_ONE_PARAM_SIZE;
	/* override DE register data */
	pcmdp = &dsi_data->panel->pnvrw_ctl->
			nvm_write_user->payload.dsi_payload;
	cnt = nvm_override_param(pcmdp, 0xDE, NVRW_NUM_DE_PARAM, pos);
	if (cnt == 0) {
		pr_err("%s:Override failure of the DE parameters.\n", __func__);
		goto err_exit;
	}
	pos += cnt * NVRW_ONE_PARAM_SIZE;
	/* override E6 register data */
	cnt = nvm_override_param(pcmdp, 0xE6, NVRW_NUM_E6_PARAM, pos);
	if (cnt == 0) {
		pr_err("%s:Override failure of the E6 parameters.\n", __func__);
		goto err_exit;
	}

	return 0;
err_exit:
	return -EINVAL;
}

static int nvm_read(struct msm_fb_data_type *mfd, char *buf)
{
	struct mipi_dsi_data		*dsi_data;
	struct dsi_nvm_rewrite_ctl	*pnvrw_ctl;
	int	n;
	int	len = 0;
	char	*pos = buf;
	int	ret;

	dsi_data = platform_get_drvdata(mfd->panel_pdev);
	pnvrw_ctl = dsi_data->panel->pnvrw_ctl;
	ret = panel_execute_cmd(mfd, dsi_data, pnvrw_ctl->nvm_mcap, 0);
	if (ret) {
		pr_err("%s: nvm_mcap failed\n", __func__);
		goto err_exit;
	}

	/* E7 register data */
	ret = panel_execute_cmd(mfd, dsi_data,
		pnvrw_ctl->nvm_read_rsp, NVRW_NUM_E7_PARAM);
	if (ret) {
		pr_err("%s: nvm_read_rsp failed\n", __func__);
		goto err_exit;
	}
	for (n = 0; n < NVRW_NUM_E7_PARAM; n++)
		len += snprintf(pos + len, PAGE_SIZE - len, "%02x ",
				dsi_data->rx_buf.data[n]);

	/* DE register data */
	ret = panel_execute_cmd(mfd, dsi_data,
		pnvrw_ctl->nvm_read_vcomdc, NVRW_NUM_DE_PARAM);
	if (ret) {
		pr_err("%s: nvm_read_vcomdc failed\n", __func__);
		goto err_exit;
	}
	for (n = 0; n < NVRW_NUM_DE_PARAM; n++) {
		if (n < NVRW_USEFUL_DE_PARAM)
			len += snprintf(pos + len, PAGE_SIZE - len, "%02x ",
					dsi_data->rx_buf.data[n]);
		else
			len += snprintf(pos + len, PAGE_SIZE - len, "00 ");
	}

	/* E6 register data */
	ret = panel_execute_cmd(mfd, dsi_data,
		pnvrw_ctl->nvm_read_ddb_write, NVRW_NUM_E6_PARAM);
	if (ret) {
		pr_err("%s: nvm_read_vcomdc failed\n", __func__);
		goto err_exit;
	}
	for (n = 0; n < NVRW_NUM_E6_PARAM; n++)
		len += snprintf(pos + len, PAGE_SIZE - len, "%02x ",
				dsi_data->rx_buf.data[n]);
	*(pos + len) = 0;

	panel_execute_cmd(mfd, dsi_data, pnvrw_ctl->nvm_mcap_lock, 0);

	return len;
err_exit:
	return 0;
}

static int nvm_erase(struct msm_fb_data_type *mfd)
{
	struct mipi_dsi_data		*dsi_data;
	struct dsi_nvm_rewrite_ctl	*pnvrw_ctl;
	int	i;
	int	ret;

	dsi_data = platform_get_drvdata(mfd->panel_pdev);
	pnvrw_ctl = dsi_data->panel->pnvrw_ctl;
	ret = panel_execute_cmd(mfd, dsi_data, pnvrw_ctl->nvm_disp_off, 0);
	if (ret) {
		pr_err("%s: nvm_disp_off failed\n", __func__);
		goto err_exit;
	}
	ret = panel_execute_cmd(mfd, dsi_data, pnvrw_ctl->nvm_mcap, 0);
	if (ret) {
		pr_err("%s: nvm_mcap failed\n", __func__);
		goto err_exit;
	}

	for (i = 0; i < NVRW_RETRY; i++) {
		ret = panel_execute_cmd(mfd, dsi_data, pnvrw_ctl->nvm_open, 0);
		if (ret)
			pr_err("%s: nvm_open failed\n", __func__);
		ret = panel_execute_cmd(mfd, dsi_data,
				pnvrw_ctl->nvm_write_rsp, 0);
		if (ret)
			pr_err("%s: nvm_write_rsp failed\n", __func__);
		ret = panel_execute_cmd(mfd, dsi_data,
				pnvrw_ctl->nvm_write_user, 0);
		if (ret)
			pr_err("%s: nvm_write_user failed\n", __func__);

		ret = panel_execute_cmd(mfd, dsi_data, pnvrw_ctl->nvm_erase, 0);
		if (ret)
			pr_err("%s: nvm_erase failed\n", __func__);

		ret = panel_execute_cmd(mfd, dsi_data,
				pnvrw_ctl->nvm_erase_res, 1);
		if (ret)
			pr_err("%s: nvm_erase_res failed\n", __func__);
		if (dsi_data->rx_buf.data[0] != NVRW_ERASE_RES_OK) {
			ret = panel_execute_cmd(mfd, dsi_data,
					pnvrw_ctl->nvm_disp_off, 0);
			if (ret)
				pr_err("%s: nvm_disp_off failed\n", __func__);
			pr_err("%s (%d): RETRY %d", __func__, __LINE__, i+1);
			dsi_data->nvrw_retry_cnt++;
			continue;
		}
		ret = panel_execute_cmd(mfd, dsi_data, pnvrw_ctl->nvm_close, 0);
		if (ret)
			pr_err("%s: nvm_close failed\n", __func__);
		ret = panel_execute_cmd(mfd, dsi_data,
				pnvrw_ctl->nvm_status, 1);
		if (ret)
			pr_err("%s: nvm_status failed\n", __func__);
		if (dsi_data->rx_buf.data[0] == NVRW_STATUS_RES_NG) {
			ret = panel_execute_cmd(mfd, dsi_data,
					pnvrw_ctl->nvm_disp_off, 1);
			if (ret)
				pr_err("%s: nvm_disp_off failed\n", __func__);
			pr_err("%s (%d): RETRY %d", __func__, __LINE__, i+1);
			dsi_data->nvrw_retry_cnt++;
			continue;
		}
		break;
	}
	if (i >= NVRW_RETRY)
		goto err_exit;
	ret = panel_execute_cmd(mfd, dsi_data, pnvrw_ctl->nvm_mcap_lock, 0);
	if (ret)
		pr_err("%s: nvm_mcap_lock failed\n", __func__);
	if (pnvrw_ctl->nvm_term_seq) {
		ret = panel_execute_cmd(mfd, dsi_data,
				pnvrw_ctl->nvm_term_seq, 0);
		if (ret)
			pr_err("%s: nvm_term_seq failed\n", __func__);
	}
	dsi_data->dsi_power_save(0);
	msleep(NVRW_PANEL_OFF_MSLEEP);

	return 0;
err_exit:
	return ret;
}
static int nvm_rsp_write(struct msm_fb_data_type *mfd)
{
	struct mipi_dsi_data		*dsi_data;
	struct dsi_nvm_rewrite_ctl	*pnvrw_ctl;
	int	i;
	int	ret;

	dsi_data = platform_get_drvdata(mfd->panel_pdev);
	pnvrw_ctl = dsi_data->panel->pnvrw_ctl;
	dsi_data->dsi_power_save(1);
	ret = panel_execute_cmd(mfd, dsi_data, pnvrw_ctl->nvm_mcap, 0);
	if (ret) {
		pr_err("%s: nvm_mcap failed\n", __func__);
		goto err_exit;
	}
	for (i = 0; i < NVRW_RETRY; i++) {
		ret = panel_execute_cmd(mfd, dsi_data, pnvrw_ctl->nvm_open, 0);
		if (ret)
			pr_err("%s: nvm_open failed\n", __func__);
		ret = panel_execute_cmd(mfd, dsi_data,
				pnvrw_ctl->nvm_write_rsp, 0);
		if (ret)
			pr_err("%s: nvm_write_rsp failed\n", __func__);
		ret = panel_execute_cmd(mfd, dsi_data,
				pnvrw_ctl->nvm_flash_rsp, 0);
		if (ret)
			pr_err("%s: nvm_flash_rsp failed\n", __func__);
		ret = panel_execute_cmd(mfd, dsi_data,
				pnvrw_ctl->nvm_status, 1);
		if (ret)
			pr_err("%s: nvm_status failed\n", __func__);
		if (dsi_data->rx_buf.data[0] == NVRW_STATUS_RES_NG) {
			pr_err("%s (%d): RETRY %d", __func__, __LINE__, i+1);
			dsi_data->nvrw_retry_cnt++;
			continue;
		}
		break;
	}
	if (i >= NVRW_RETRY) {
		ret = -EAGAIN;
		goto err_exit;
	}
	ret = panel_execute_cmd(mfd, dsi_data, pnvrw_ctl->nvm_mcap_lock, 0);
	if (ret)
		pr_err("%s: nvm_mcap_lock failed\n", __func__);
	dsi_data->dsi_power_save(0);
	msleep(NVRW_PANEL_OFF_MSLEEP);

	return 0;
err_exit:
	return ret;
}

static int nvm_user_write(struct msm_fb_data_type *mfd)
{
	struct mipi_dsi_data		*dsi_data;
	struct dsi_nvm_rewrite_ctl	*pnvrw_ctl;
	int ret;

	dsi_data = platform_get_drvdata(mfd->panel_pdev);
	pnvrw_ctl = dsi_data->panel->pnvrw_ctl;
	dsi_data->dsi_power_save(1);
	ret = panel_execute_cmd(mfd, dsi_data, pnvrw_ctl->nvm_mcap, 0);
	if (ret) {
		pr_err("%s: nvm_mcap failed\n", __func__);
		goto err_exit;
	}
	ret = panel_execute_cmd(mfd, dsi_data,
			pnvrw_ctl->nvm_write_user, 0);
	if (ret) {
		pr_err("%s: nvm_write_user_cmds failed\n", __func__);
		goto err_exit;
	}
	ret = panel_execute_cmd(mfd, dsi_data,
			pnvrw_ctl->nvm_flash_user, 0);
	if (ret) {
		pr_err("%s: nvm_flash_user_cmds failed\n", __func__);
		goto err_exit;
	}

	ret = panel_execute_cmd(mfd, dsi_data, pnvrw_ctl->nvm_mcap_lock, 0);
	if (ret)
		pr_err("%s: nvm_mcap_lock failed\n", __func__);
	dsi_data->dsi_power_save(0);
	msleep(NVRW_PANEL_OFF_MSLEEP);
	dsi_data->dsi_power_save(1);

	return 0;
err_exit:
	return ret;
}

static ssize_t mipi_dsi_panel_id_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct mipi_dsi_data *dsi_data = dev_get_drvdata(dev);
	char const *id =  dsi_data->panel->panel_id ?
			dsi_data->panel->panel_id : "generic";
	return scnprintf(buf, PAGE_SIZE, "%s\n", id);
}

static ssize_t mipi_dsi_panel_rev_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct mipi_dsi_data *dsi_data = dev_get_drvdata(dev);
	char const *rev =  dsi_data->panel->panel_rev ?
			dsi_data->panel->panel_rev : "generic";
	return scnprintf(buf, PAGE_SIZE, "%s\n", rev);
}

static ssize_t mipi_dsi_panel_frame_counter(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%i\n", fpsd.frame_counter);
}

static ssize_t mipi_dsi_panel_frames_per_ksecs(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%i\n", fpsd.fpks);
}

static ssize_t mipi_dsi_panel_interval_ms_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%i\n", fpsd.interval_ms);
}

static ssize_t mipi_dsi_panel_log_interval_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%i\n", fpsd.log_interval);
}

static ssize_t mipi_dsi_panel_log_interval_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int ret = count;

	if (sscanf(buf, "%4i", &fpsd.log_interval) != 1) {
		pr_err("%s: Error, buf = %s\n", __func__, buf);
		ret = -EINVAL;
	}
	return ret;
}

static ssize_t mipi_dsi_panel_interval_array_ms(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	u16 i, len, rc = 0;
	char *tmp = buf;

	mutex_lock(&fpsd.fps_lock);
	len = fpsd.fa_last_array_pos;
	/* Get the first frames from the buffer */
	for (i = len + 1; i < DEFAULT_FPS_ARRAY_SIZE; i++) {
		if (fpsd.fa[i].time_delta) {
			rc += scnprintf(tmp + rc, PAGE_SIZE - rc ,
						"%i, ", fpsd.fa[i].time_delta);
		}
	}
	/* Get the rest frames from the buffer */
	if (len) {
		for (i = 0; i <= len; i++) {
			if (fpsd.fa[i].time_delta) {
				rc += scnprintf(tmp + rc, PAGE_SIZE - rc ,
						"%i, ", fpsd.fa[i].time_delta);
			}
		}
	}
	rc += scnprintf(tmp + rc, PAGE_SIZE - rc , "\n");

	/* Clear the buffer once it is read */
	mipi_dsi_panel_fps_array_clear(&fpsd);
	mutex_unlock(&fpsd.fps_lock);

	return rc;
}

#ifdef CONFIG_FB_MSM_RECOVER_PANEL
static ssize_t mipi_dsi_panel_nvm_is_read_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct mipi_dsi_data *dsi_data = dev_get_drvdata(dev);

	if (!dsi_data->panel)
		return snprintf(buf, PAGE_SIZE, "NG");
	if (dsi_data->panel->pnvrw_ctl == NULL)
		return snprintf(buf, PAGE_SIZE, "skip");

	if (dsi_data->nvrw_panel_detective)
		return snprintf(buf, PAGE_SIZE, "OK");
	else
		return snprintf(buf, PAGE_SIZE, "NG");
}

static ssize_t mipi_dsi_panel_nvm_result_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct mipi_dsi_data *dsi_data = dev_get_drvdata(dev);

	return snprintf(buf, PAGE_SIZE, "%d", dsi_data->nvrw_result);
}

static ssize_t mipi_dsi_panel_nvm_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct mipi_dsi_data *dsi_data = dev_get_drvdata(dev);
	struct msm_fb_data_type *mfd = dsi_data->nvrw_private;
	enum power_state old_state = PANEL_OFF;
	int rc = 0;

	if (!dsi_data->nvrw_panel_detective)
		goto exit;
	if (dsi_data->panel->pnvrw_ctl == NULL)
		goto exit;

	if (prepare_for_reg_access(mfd, &old_state))
		goto exit;
	mipi_set_tx_power_mode(1);
	if (dsi_data->seq_nvm_read)
		rc = dsi_data->seq_nvm_read(mfd, buf);
	post_reg_access(mfd, old_state);
exit:
	return rc;
}

static ssize_t mipi_dsi_panel_nvm_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct mipi_dsi_data *dsi_data = dev_get_drvdata(dev);
	struct msm_fb_data_type *mfd = dsi_data->nvrw_private;
	int rc;
	enum power_state old_state = PANEL_OFF;

	dev_dbg(dev, "%s\n", __func__);
	dsi_data->nvrw_result = -1;
	dsi_data->nvrw_retry_cnt = 0;
	if (dsi_data->nvrw_panel_detective)
		goto exit;

	/* update panel information from miscTA */
	rc = nvm_update_panel(mfd, buf, count);
	if (rc)
		goto exit;
	if (!dsi_data->panel->pnvrw_ctl || !dsi_data->dsi_power_save)
		goto exit;

	mfd->nvrw_prohibit_draw = true;
	rc = prepare_for_reg_access(mfd, &old_state);
	if (rc)
		goto exit;
	mipi_set_tx_power_mode(1);

	if (dsi_data->seq_nvm_read) {
		char	nvm[NVRW_DATA_SIZE + 1];
		/* Does not NVM disappear? */
		dev_dbg(dev, "%s:seq_nvm_read\n", __func__);
		dsi_data->seq_nvm_read(mfd, nvm);
		if (0 == strncasecmp(buf, nvm, NVRW_DATA_SIZE - 1)) {
			dev_dbg(dev, "%s:skip_recover\n", __func__);
			dsi_data->nvrw_result = 0;
			goto skip_recover;
		}
	}
	if (dsi_data->override_nvm_data) {
		dev_dbg(dev, "%s:override_nvm_data\n", __func__);
		rc = dsi_data->override_nvm_data(mfd, buf, count);
		if (rc) {
			dev_err(dev, "%s : nvm data format error.<%s>\n",
				__func__, buf);
			goto release_exit;
		}
	}

	if (dsi_data->seq_nvm_erase) {
		dev_dbg(dev, "%s:seq_nvm_erase\n", __func__);
		rc = dsi_data->seq_nvm_erase(mfd);
		if (rc) {
			dev_err(dev,
				"%s : nvm data erase fail.\n", __func__);
			goto release_exit;
		}
	}
	if (dsi_data->seq_nvm_rsp_write) {
		dev_dbg(dev, "%s:seq_nvm_rsp_write\n", __func__);
		rc = dsi_data->seq_nvm_rsp_write(mfd);
		if (rc) {
			dev_err(dev,
				"%s : rsp write fail.\n", __func__);
			goto release_exit;
		}
	}

	if (dsi_data->seq_nvm_user_write) {
		dev_dbg(dev, "%s:seq_nvm_user_write\n", __func__);
		rc = dsi_data->seq_nvm_user_write(mfd);
		if (rc) {
			dev_err(dev,
				"%s : user write fail.\n", __func__);
			goto release_exit;
		}
	}
	dsi_data->nvrw_result = dsi_data->nvrw_retry_cnt + 1;

skip_recover:
	dsi_data->nvrw_panel_detective = true;
release_exit:
	post_reg_access(mfd, old_state);
	mfd->nvrw_prohibit_draw = false;

	if (dsi_data->nvrw_panel_detective) {
		struct fb_info *fbi = mfd->fbi;
		if (fbi && fbi->fbops && fbi->fbops->fb_blank) {
			fbi->fbops->fb_blank(FB_BLANK_POWERDOWN, fbi);
			fbi->fbops->fb_blank(FB_BLANK_UNBLANK, fbi);
		}
	}
exit:
	return count;
}
#endif

static struct device_attribute panel_attributes[] = {
	__ATTR(panel_id, S_IRUGO, mipi_dsi_panel_id_show, NULL),
	__ATTR(panel_rev, S_IRUGO, mipi_dsi_panel_rev_show, NULL),
	__ATTR(frame_counter, S_IRUGO, mipi_dsi_panel_frame_counter, NULL),
	__ATTR(frames_per_ksecs, S_IRUGO,
				mipi_dsi_panel_frames_per_ksecs, NULL),
	__ATTR(interval_ms, S_IRUGO, mipi_dsi_panel_interval_ms_show, NULL),
	__ATTR(log_interval, S_IRUGO|S_IWUSR|S_IWGRP,
					mipi_dsi_panel_log_interval_show,
					mipi_dsi_panel_log_interval_store),
	__ATTR(interval_array, S_IRUGO,
					mipi_dsi_panel_interval_array_ms, NULL),
#ifdef CONFIG_FB_MSM_RECOVER_PANEL
	__ATTR(nvm_is_read, S_IRUGO, mipi_dsi_panel_nvm_is_read_show, NULL),
	__ATTR(nvm_result, S_IRUGO, mipi_dsi_panel_nvm_result_show, NULL),
	__ATTR(nvm, S_IRUSR | S_IWUSR,
			mipi_dsi_panel_nvm_show, mipi_dsi_panel_nvm_store),
#endif
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
	return -ENODEV;
}

static void remove_attributes(struct device *dev)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(panel_attributes); i++)
		device_remove_file(dev, panel_attributes + i);
}

static int __devexit mipi_dsi_panel_remove(struct platform_device *pdev)
{
	struct mipi_dsi_data *dsi_data;

	dsi_data = platform_get_drvdata(pdev);
	remove_attributes(&pdev->dev);

#ifdef CONFIG_DEBUG_FS
	mipi_dsi_panel_remove_debugfs(pdev);
#endif

	platform_set_drvdata(pdev, NULL);
	mipi_dsi_buf_release(&dsi_data->tx_buf);
	mipi_dsi_buf_release(&dsi_data->rx_buf);
	kfree(dsi_data);
	pcc_cfg_ptr = NULL;
	kfree(color_calib);
	return 0;
}

static int __devinit mipi_dsi_panel_probe(struct platform_device *pdev)
{
	int ret;
	struct panel_platform_data *platform_data;
	struct mipi_dsi_data *dsi_data;
	struct platform_device *mipi_dsi_pdev;

	pr_info("%s: pdev = %p, pdev->name = %s\n", __func__, pdev, pdev->name);

	platform_data = pdev->dev.platform_data;
	if (platform_data == NULL)
		return -EINVAL;

	dsi_data = kzalloc(sizeof(struct mipi_dsi_data), GFP_KERNEL);
	if (dsi_data == NULL)
		return -ENOMEM;

	mutex_init(&dsi_data->lock);
	dsi_data->panels = platform_data->panels;
	dsi_data->lcd_power = platform_data->platform_power;
	dsi_data->lcd_reset = platform_data->platform_reset;
	dsi_data->panel_data.panel_detect = detect_panel;
	dsi_data->panel_data.update_panel = panel_update_config;
	dsi_data->panel_detected = false;

	ret = mipi_dsi_buf_alloc(&dsi_data->tx_buf, DSI_BUF_SIZE);
	if (ret <= 0) {
		dev_err(&pdev->dev, "mipi_dsi_buf_alloc(tx) failed!\n");
		ret = -ENOMEM;
		goto out_free;
	}

	ret = mipi_dsi_buf_alloc(&dsi_data->rx_buf, DSI_BUF_SIZE);
	if (ret <= 0) {
		dev_err(&pdev->dev, "mipi_dsi_buf_alloc(rx) failed!\n");
		ret = -ENOMEM;
		goto out_rx_release;
	}

	platform_set_drvdata(pdev, dsi_data);
	dsi_data->panel_data.get_pcc_data = get_pcc_data;
	dsi_data->pcc_config = NULL;
	dsi_data->panel_data.panel_info = default_pinfo;
	dsi_data->panel_data.on = panel_on;
	dsi_data->panel_data.off = panel_off;
	dsi_data->override_nvm_data = nvm_override_data;
	dsi_data->seq_nvm_read = nvm_read;
	dsi_data->seq_nvm_erase = nvm_erase;
	dsi_data->seq_nvm_rsp_write = nvm_rsp_write;
	dsi_data->seq_nvm_user_write = nvm_user_write;
	dsi_data->nvrw_panel_detective = false;

	mipi_dsi_panel_fps_data_init(&fpsd);

	ret = platform_device_add_data(pdev, &dsi_data->panel_data,
						sizeof(dsi_data->panel_data));
	if (ret) {
		dev_err(&pdev->dev, "platform_device_add_data failed!\n");
		goto out_tx_release;
	}
	ret = register_attributes(&pdev->dev);
	if (ret)
		goto out_tx_release;
	mipi_dsi_pdev = msm_fb_add_device(pdev);
#ifdef CONFIG_DEBUG_FS
	mipi_dsi_panel_create_debugfs(mipi_dsi_pdev);
#endif
	dsi_data->nvrw_private = platform_get_drvdata(mipi_dsi_pdev);

	dev_info(&pdev->dev, "%s: Probe success\n", __func__);
	return 0;
out_tx_release:
	mipi_dsi_buf_release(&dsi_data->rx_buf);
out_rx_release:
	mipi_dsi_buf_release(&dsi_data->tx_buf);
out_free:
	platform_set_drvdata(pdev, NULL);
	kfree(dsi_data);
	dev_info(&pdev->dev, "%s: Probe fail\n", __func__);
	return ret;
}

static struct platform_driver this_driver = {
	.probe  = mipi_dsi_panel_probe,
	.remove = mipi_dsi_panel_remove,
	.driver = {
		.name = MIPI_DSI_PANEL_NAME,
	},
};

static int __init mipi_dsi_panel_init(void)
{
	return platform_driver_register(&this_driver);
}

static void __exit mipi_dsi_panel_exit(void)
{
	platform_driver_unregister(&this_driver);
}

module_init(mipi_dsi_panel_init);
module_exit(mipi_dsi_panel_exit);
