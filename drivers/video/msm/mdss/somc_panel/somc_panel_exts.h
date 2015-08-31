/*
 * Restructured unified display panel driver for Xperia Open Devices
 *                  *** Extensions for MDSS ***
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
#ifndef SOMC_PANEL_EXTS_H
#define SOMC_PANEL_EXTS_H

#include <linux/mdss_io_util.h>
#include "../mdss_panel.h"
#include "../mdss_dsi_cmd.h"
#include "../mdss_fb.h"

#define PCC_STS_UD	0x01	/* update request */
#define DRIVER_IC_FIH	4

/* panel DriverIC type */
enum {
	PANEL_DRIVER_IC_R63311,
	PANEL_DRIVER_IC_NT35596,
	PANEL_DRIVER_IC_SY35590,
	PANEL_DRIVER_IC_NT71397,
	PANEL_DRIVER_IC_NONE,
};

struct mdss_pcc_color_tbl {
	u32 color_type;
	u32 area_num;
	u32 u_min;
	u32 u_max;
	u32 v_min;
	u32 v_max;
	u32 r_data;
	u32 g_data;
	u32 b_data;
} __packed;

struct mdss_pcc_data {
	struct mdss_pcc_color_tbl *color_tbl;
	u32 tbl_size;
	u8 tbl_idx;
	u8 pcc_sts;
	u32 u_data;
	u32 v_data;
	int param_type;
};

struct mdss_panel_power_seq {
	int disp_dcdc_en_pre;
	int disp_dcdc_en_post;
	int disp_en_pre;
	int disp_en_post;
	int seq_num;
	int *rst_seq;
	int seq_b_num;
	int *rst_b_seq;
};

struct esd_reg_status_ctrl {
	u8 reg;
	u8 correct_val;
	int nbr_bytes_to_read;
	struct dsi_cmd_desc dsi;
};

struct poll_ctrl {
	bool enable;
	int intervals;
	struct delayed_work poll_working;
	struct mdss_dsi_ctrl_pdata *ctrl_pdata;

	struct esd_reg_status_ctrl esd;

	void (*schedule_work) (struct mdss_dsi_ctrl_pdata *ctrl_pdata);
	void (*cancel_work) (struct mdss_dsi_ctrl_pdata *ctrl_pdata);
};

struct mdss_panel_specific_pdata {
	int (*pcc_setup)(struct mdss_panel_data *pdata);
	int (*panel_power_ctrl) (struct mdss_panel_data *pdata, int enable);
	int (*disp_on) (struct mdss_panel_data *pdata);
	int (*detect) (struct mdss_panel_data *pdata);
	int (*update_panel) (struct mdss_panel_data *pdata);
	int (*update_fps) (struct msm_fb_data_type *mfd);
	int (*reset) (struct mdss_panel_data *pdata, int enable);

	bool disp_on_in_boot;
	bool detected;
	int driver_ic;
	int32_t lcd_id;
	int32_t adc_uv;
	int panel_detect;
	int init_from_begin;
	int cabc_enabled;
	int cabc_active;
	int lcm_bl_gpio;
	int disp_dcdc_en_gpio;
	int disp_p5;
	int disp_n5;
	int vsn_gpio;
	int vsp_gpio;
	bool pwron_reset;
	bool dsi_seq_hack;
	bool postpwron_no_reset_quirk;

	struct dsi_panel_cmds cabc_early_on_cmds;
	struct dsi_panel_cmds cabc_on_cmds;

	struct dsi_panel_cmds cabc_off_cmds;
	struct dsi_panel_cmds cabc_late_off_cmds;
	struct dsi_panel_cmds fps_cmds;
	struct dsi_panel_cmds lock_cmds;
	struct dsi_panel_cmds unlock_cmds;

	struct dsi_panel_cmds einit_cmds;
	struct dsi_panel_cmds init_cmds;
	struct dsi_panel_cmds id_read_cmds;

	bool pcc_enable;
	struct dsi_panel_cmds pre_uv_read_cmds;
	struct dsi_panel_cmds uv_read_cmds;
	struct mdss_pcc_data pcc_data;
	struct mdp_pa_cfg picadj_data;

	struct mdss_panel_power_seq on_seq;
	struct mdss_panel_power_seq off_seq;
	u32 down_period;
	u32 new_vfp;

	/* LAB/IBB SoMC regulator params */
	u32 lab_output_voltage;
	u32 ibb_output_voltage;
	u32 lab_current_max;
	u32 ibb_current_max;
	bool lab_current_max_enable;
	bool ibb_current_max_enable;
	int (*vreg_init) (struct mdss_dsi_ctrl_pdata *ctrl);
	int (*vreg_ctrl) (struct mdss_dsi_ctrl_pdata *ctrl, int enable);

	struct poll_ctrl polling;
};

int mdss_dsi_panel_power_detect(struct platform_device *pdev, int enable);
int mdss_dsi_panel_fps_data_update(struct msm_fb_data_type *mfd);
int mdss_dsi_pinctrl_set_state(struct mdss_dsi_ctrl_pdata *ctrl_pdata,
					bool active);
int mdss_dsi_panel_disp_en(struct mdss_panel_data *pdata, int enable);

static inline struct mdss_dsi_ctrl_pdata *mdss_dsi_get_master_ctrl(
					struct mdss_panel_data *pdata)
{
	int dsi_master = DSI_CTRL_0;

	if (pdata->panel_info.dsi_master == DISPLAY_2)
		dsi_master = DSI_CTRL_1;
	else
		dsi_master = DSI_CTRL_0;

	return mdss_dsi_get_ctrl_by_index(dsi_master);
}

#endif
