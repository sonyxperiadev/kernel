/* include/video/mipi_dsi_panel.h
 *
 * Copyright (c) 2012-2013 Sony Mobile Communications AB.
 *
 * Author: Johan Olson <johan.olson@sonymobile.com>
 * Author: Joakim Wesslen <joakim.wesslen@sonymobile.com>
 * Author: Perumal Jayamani <perumal.jayamani@sonymobile.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2; as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef MIPI_DSI_PANEL_H
#define MIPI_DSI_PANEL_H

#include <linux/types.h>
#include <linux/msm_mdp.h>

#define MIPI_DSI_PANEL_NAME "mipi_dsi_panel"
#define MIPI_DSI_NUM_PHY_LN 4

enum panel_cmd_type {
	CMD_END,
	CMD_WAIT_MS,
	CMD_DSI,
	CMD_RESET,
	CMD_PLATFORM
};

struct dsi_cmd_payload {
	struct dsi_cmd_desc *dsi;
	int cnt;
};

struct panel_cmd {
	enum panel_cmd_type type;
	union {
		char data;
		struct dsi_cmd_payload dsi_payload;
	} payload;
};

struct dsi_controller {
	struct msm_panel_info *(*get_panel_info)(void);
	const struct panel_cmd *display_init;
	const struct panel_cmd *display_on;
	const struct panel_cmd *display_off;
	const struct panel_cmd *read_id;
	const struct panel_cmd *read_color;
};

struct dsi_nvm_rewrite_ctl {
	const struct panel_cmd *nvm_disp_off;
	const struct panel_cmd *nvm_mcap;
	const struct panel_cmd *nvm_mcap_lock;
	const struct panel_cmd *nvm_open;
	const struct panel_cmd *nvm_close;
	const struct panel_cmd *nvm_status;
	const struct panel_cmd *nvm_erase;
	const struct panel_cmd *nvm_erase_res;
	const struct panel_cmd *nvm_read_rsp;
	const struct panel_cmd *nvm_read_vcomdc;
	const struct panel_cmd *nvm_read_ddb_write;
	struct panel_cmd *nvm_write_rsp;
	const struct panel_cmd *nvm_flash_rsp;
	struct panel_cmd *nvm_write_user;
	const struct panel_cmd *nvm_flash_user;
	const struct panel_cmd *nvm_term_seq;
};

struct mipi_dsi_lane_cfg {
	uint32_t	ln_cfg[MIPI_DSI_NUM_PHY_LN][3];
	uint32_t	ln_dpath[MIPI_DSI_NUM_PHY_LN];
	uint32_t	ln_str[MIPI_DSI_NUM_PHY_LN][2];
	uint32_t	lnck_cfg[3];
	uint32_t	lnck_dpath;
	uint32_t	lnck_str[2];
};

struct mdp_pcc_cfg_rgb {
	uint32_t r;
	uint32_t g;
	uint32_t b;
};

struct panel {
	const char			*name;
	const char			*panel_id;
	const char			*panel_rev;
	struct dsi_controller		*pctrl;
	struct dsi_nvm_rewrite_ctl	*pnvrw_ctl;
	const u32			width;	/* in mm */
	const u32			height;	/* in mm */
	const char			*id;
	const int			id_num;
	const bool			send_video_data_before_display_on;
	const bool			disable_dsi_timing_genarator_at_off;
	const struct mipi_dsi_lane_cfg	*plncfg;
	const struct mdp_pcc_cfg_rgb	*color_correction_tbl;
	const struct mdp_pcc_cfg_rgb	*color_subdivision_tbl;
};

struct panel_platform_data {
	/* TODO: add data regarding gpio pin, level, regulators, voltages */
	int (*platform_power)(bool enable);
	int (*platform_reset)(bool high);
	const struct panel **panels;
};

/* TODO: Use get functions instead */
extern const struct panel sharp_ls043k3sx04_panel_id_1a;
extern const struct panel sharp_ls043k3sx04_panel_id;
extern const struct panel sharp_ls046k3sx01_panel_tovis_id;
extern const struct panel sharp_ls046k3sx01_panel_id_1a;
extern const struct panel sharp_ls046k3sx01_panel_id;
extern const struct panel sharp_ls050t3sx02_r63311_black_panel_id;
extern const struct panel sharp_ls050t3sx02_r63311_white_panel_id;
extern const struct panel sharp_ls050t3sx02_r63311_12624797;
extern const struct panel tmd_mdw30_panel_id_old;
extern const struct panel tmd_mdw30_panel_id_1a;
extern const struct panel tmd_mdw30_panel_id_1c;
extern const struct panel tmd_mdw30_panel_id_1e;
extern const struct panel tmd_mdw30_panel_id;
extern const struct panel jdc_mdy70_panel_id_dlogo_02;
extern const struct panel jdc_mdy70_panel_id_dlogo_2a;
extern const struct panel jdc_mdy70_panel_id_dlogo;
extern const struct panel jdc_mdy70_panel_id_1a;
extern const struct panel jdc_mdy70_panel_id_1a_02;
extern const struct panel jdc_mdy70_panel_id;
extern const struct panel jdc_mdy71_panel_id_1a;
extern const struct panel jdc_mdy71_panel_id;
extern const struct panel jdc_mdy80_black_panel_id;
extern const struct panel jdc_mdy80_white_panel_id;
extern const struct panel sharp_ls050t3sx01_panel_id_dlogo_01;
extern const struct panel sharp_ls050t3sx01_panel_id_dlogo;
extern const struct panel sharp_ls050t3sx01_panel_id_1a;
extern const struct panel sharp_ls050t3sx01_panel_id_1a_02;
extern const struct panel sharp_ls050t3sx01_panel_id;
extern const struct panel panasonic_vvx10f008b00_panel_id;
extern const struct panel jdc_mdz50_panel_id_1a;
extern const struct panel jdc_mdz50_panel_id_1b;
extern const struct panel jdc_mdz50_panel_id;
extern const struct panel jdc_mdz50_panel_id_nvm;

#endif /* MIPI_DSI_PANEL_H */
