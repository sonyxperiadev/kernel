/*
 * Restructured unified display panel driver for Xperia Open Devices
 *                 *** Internal Definitions ***
 *
 * This driver is based on various SoMC implementations found in
 * copyleft archives for various devices.
 *
 * Copyright (c) 2012-2014, The Linux Foundation. All rights reserved.
 * Copyright (c) Sony Mobile Communications Inc. All rights reserved.
 * Copyright (C) 2014-2016, AngeloGioacchino Del Regno <kholk11@gmail.com>
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
#ifndef SOMC_PANELS_H
#define SOMC_PANELS_H

#define ADC_RNG_MIN			0
#define ADC_RNG_MAX			1
#define ADC_PNUM			2

#define CHANGE_FPS_MIN 			36
#define CHANGE_FPS_MAX 			63

#define DEF_FPS_LOG_INTERVAL		100
#define DEF_FPS_ARRAY_SIZE		120

#define DSI_PCLK_MIN 			3300000
#define DSI_PCLK_MAX 			223000000
#define DSI_PCLK_DEFAULT 		35000000

/* PCC data infomation */
#define UNUSED				0xff
#define CLR_DATA_REG_LEN_RENE_DEFAULT	2
#define CLR_DATA_REG_LEN_NOVA_DEFAULT	1
#define CLR_DATA_REG_LEN_NOVA_AUO	3
#define CLR_DATA_REG_LEN_RENE_SR	1
#define CENTER_U_DATA			30
#define CENTER_V_DATA			30

/* Interval required for all panels to get ready after cont-splash */
#define FIRST_POLL_REG_INTERVAL 20000

/* Regulator voltages */
#define QPNP_REGULATOR_VSP_V_5P4V	5400000
#define QPNP_REGULATOR_VSN_V_M5P4V	5400000

/* Data for internal use ONLY */
struct fps_array {
	u32 frame_nbr;
	u32 time_delta;
};

struct fps_data {
	struct mutex fps_lock;
	u32 log_interval;
	u32 interval_ms;
	struct timespec timestamp_last;
	u32 frame_counter_last;
	u32 frame_counter;
	u32 fpks;
	struct timespec fpks_ts_last;
	u16 fa_last_array_pos;
	struct fps_array fa[DEF_FPS_ARRAY_SIZE];
	u16 fps_array_cnt;
	bool vps_en;
};

enum {
	CLR_DATA_UV_PARAM_TYPE_NONE,
	CLR_DATA_UV_PARAM_TYPE_RENE_DEFAULT,
	CLR_DATA_UV_PARAM_TYPE_NOVA_DEFAULT,
	CLR_DATA_UV_PARAM_TYPE_NOVA_AUO,
	CLR_DATA_UV_PARAM_TYPE_RENE_SR
};

/* Main */
void mdss_dsi_panel_cmds_send(struct mdss_dsi_ctrl_pdata *ctrl,
			struct dsi_panel_cmds *pcmds);

int  mdss_panel_parse_dt(struct device_node *np,
			 struct mdss_dsi_ctrl_pdata *ctrl_pdata);

/* Detection */
int  do_panel_detect(struct device_node **node,
		struct platform_device *pdev,
		struct mdss_dsi_ctrl_pdata *ctrl_pdata,
		bool cmd_detect_quirk, u32 cell_idx);

/* Polling */
void poll_worker_schedule(struct mdss_dsi_ctrl_pdata *ctrl_pdata);
void poll_worker_cancel(struct mdss_dsi_ctrl_pdata *ctrl_pdata);
int  panel_polling_init(struct mdss_dsi_ctrl_pdata *ctrl_pdata);

/* Regulators */
#ifdef CONFIG_SOMC_PANEL_LABIBB
int somc_panel_vregs_dt(struct device_node *np,
			struct mdss_dsi_ctrl_pdata *ctrl_pdata);
#else
#define somc_panel_vregs_dt(x, y) 0
#endif

#endif
