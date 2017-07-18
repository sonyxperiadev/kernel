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

#define CHANGE_FPS_MIN 			22
#define CHANGE_FPS_MAX 			120

#define CHANGE_FPS_PORCH		2
#define CHANGE_FPS_SEND			10

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

#define CHANGE_PAYLOAD(a, b) (spec_pdata->fps_cmds.cmds[a].payload[b])

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

/* Common */
int  somc_panel_set_gpio(int gpio, int enable);

int  somc_panel_vreg_name_to_config(
		struct mdss_dsi_ctrl_pdata *ctrl_pdata,
		struct dss_vreg *config, char *name);

int  somc_panel_vreg_ctrl(
		struct mdss_dsi_ctrl_pdata *ctrl_pdata,
		char *vreg, bool enable);

/* Main */
void somc_panel_down_period_quirk(
			struct mdss_panel_specific_pdata *spec_pdata);

void mdss_dsi_panel_cmds_send(struct mdss_dsi_ctrl_pdata *ctrl,
			struct dsi_panel_cmds *pcmds);

int  mdss_panel_parse_dt(struct device_node *np,
			 struct mdss_dsi_ctrl_pdata *ctrl_pdata);

int  mdss_dsi_request_gpios(struct mdss_dsi_ctrl_pdata *ctrl_pdata);

int  mdss_dsi_property_read_u32_var(struct device_node *np,
		char *name, u32 **out_data, int *num);

int  somc_panel_parse_dcs_cmds(struct device_node *np,
		struct dsi_panel_cmds *pcmds, char *cmd_key, char *link_key);

/* Detection */
int  do_panel_detect(struct device_node **node,
		struct platform_device *pdev,
		struct mdss_dsi_ctrl_pdata *ctrl_pdata,
		bool cmd_detect_quirk, u32 cell_idx);

/* Polling */
void poll_worker_schedule(struct mdss_dsi_ctrl_pdata *ctrl_pdata);
void poll_worker_cancel(struct mdss_dsi_ctrl_pdata *ctrl_pdata);
int  panel_polling_init(struct mdss_dsi_ctrl_pdata *ctrl_pdata);

/* FPS Manager */
int  somc_panel_parse_dt_chgfps_config(struct device_node *pan_node,
			struct mdss_dsi_ctrl_pdata *ctrl_pdata);
void somc_panel_chg_fps_cmds_send(struct mdss_dsi_ctrl_pdata *ctrl_pdata);
void somc_panel_fpsman_panel_post_on(struct mdss_dsi_ctrl_pdata *ctrl);
void somc_panel_fpsman_panel_off(void);
void somc_panel_fpsman_refresh(struct mdss_dsi_ctrl_pdata *ctrl,
		bool immediate_refresh);
int  somc_panel_fps_register_attr(struct device *dev);
int  somc_panel_fps_manager_init(void);

/* Color Manager */
int somc_panel_parse_dt_colormgr_config(struct device_node *np,
			struct mdss_dsi_ctrl_pdata *ctrl);
int  somc_panel_color_manager_init(struct mdss_dsi_ctrl_pdata *ctrl);
int  somc_panel_colormgr_register_attr(struct device *dev);
void somc_panel_colormgr_reset(struct mdss_dsi_ctrl_pdata *ctrl);

/* Regulators */
#ifdef CONFIG_SOMC_PANEL_LABIBB
int somc_panel_vregs_dt(struct device_node *np,
			struct mdss_dsi_ctrl_pdata *ctrl_pdata);
#else
#define somc_panel_vregs_dt(x, y) 0
#endif

#ifdef CONFIG_SOMC_PANEL_LEGACY
/* Legacy */
int legacy_panel_driver_init(struct mdss_dsi_ctrl_pdata *ctrl_pdata);
#endif

#ifdef CONFIG_SOMC_PANEL_INCELL
/* Incell */
int  incell_panel_driver_init(struct mdss_dsi_ctrl_pdata *ctrl_pdata);

bool incell_panel_is_seq_for_ewu(void);
void incell_panel_free_gpios(struct mdss_dsi_ctrl_pdata *ctrl_pdata);
int incell_driver_post_power_on(struct mdss_panel_data *pdata);

void incell_panel_fb_notifier_call_chain(
		struct msm_fb_data_type *mfd, int blank, bool type);
#endif

#endif
