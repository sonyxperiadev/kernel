/* drivers/video/msm/mipi_dsi_panel_r63306_tmd_mdw30.c
 *
 * Copyright (C) 2012 Sony Mobile Communications AB.
 *
 * Author: Yosuke Hatanaka <yosuke.hatanaka@sonyericsson.com>
 * Author: Yutaka Seijyou <Yutaka.X.Seijyou@sonymobile.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2; as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */


#include "msm_fb.h"
#include "mipi_dsi.h"
#include "mipi_dsi_panel_driver.h"

/* Initial Sequence */
static char set_address_mode[] = {
	0x36, 0x50
};
static char mcap[] = {
	0xB0, 0x00
};
static char acr[] = {
	0xB2, 0x00
};
static char clk_and_if[] = {
	0xB3, 0x0C
};
static char pixform[] = {
	0xB4, 0x02
};
static char pfm_pwm_ctrl[] = {
	0xB9, 0x01, 0x00, 0x75, 0x00, 0xBA, 0xBB, 0xBF,
	0xCC, 0xE3, 0x46, 0x81, 0xC2
};
static char cabc_on_off[] = {
	0xBB, 0x00
};
static char cabc_user_param[] = {
	0xBE, 0xFF, 0x0F, 0x1A, 0x18, 0x02, 0x40, 0x00,
	0x5D, 0x00, 0x00, 0x80, 0x32
};
static char panel_driving[] = {
	0xC0, 0x40, 0x02, 0x7F, 0xC8, 0x08
};
static char h_timing[] = {
	0xC1, 0x00, 0xA8, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x9C, 0x08, 0x24, 0x0B, 0x00, 0x00, 0x00, 0x00
};
static char src_out[] = {
	0xC2, 0x00, 0x00, 0x0B, 0x00, 0x00
};
static char gate_ic_ctrl[] = {
	0xC3, 0x04
};
static char ltps_if_ctrl_1[] = {
	0xC4, 0x4D, 0x83, 0x00
};
static char src_out_mode[] = {
	0xC6, 0x12, 0x00, 0x08, 0x71, 0x00, 0x00, 0x00,
	0x80, 0x00, 0x04
};
static char ltps_if_ctrl_2[] = {
	0xC7, 0x22
};
static char gamma_ctrl[] = {
	0xC8, 0x4C, 0x0C, 0x0C, 0x0C
};
static char gamma_ctrl_set_r_pos[] = {
	0xC9, 0x00, 0x63, 0x3B, 0x3A, 0x33, 0x25, 0x2A,
	0x30, 0x2B, 0x2F, 0x47, 0x79, 0x3F
};
static char gamma_ctrl_set_r_neg[] = {
	0xCA, 0x00, 0x46, 0x1A, 0x31, 0x35, 0x2F, 0x36,
	0x3A, 0x2D, 0x25, 0x28, 0x5C, 0x3F
};
static char gamma_ctrl_set_g_pos[] = {
	0xCB, 0x00, 0x63, 0x3B, 0x3A, 0x33, 0x25, 0x2A,
	0x30, 0x2B, 0x2F, 0x47, 0x79, 0x3F
};
static char gamma_ctrl_set_g_neg[] = {
	0xCC, 0x00, 0x46, 0x1A, 0x31, 0x35, 0x2F, 0x36,
	0x3A, 0x2D, 0x25, 0x28, 0x5C, 0x3F
};
static char gamma_ctrl_set_b_pos[] = {
	0xCD, 0x00, 0x63, 0x3B, 0x3A, 0x33, 0x25, 0x2A,
	0x30, 0x2B, 0x2F, 0x47, 0x79, 0x3F
};
static char gamma_ctrl_set_b_neg[] = {
	0xCE, 0x00, 0x46, 0x1A, 0x31, 0x35, 0x2F, 0x36,
	0x3A, 0x2D, 0x25, 0x28, 0x5C, 0x3F
};
static char power_setting_1[] = {
	0xD0, 0x6A, 0x64, 0x01
};
static char power_setting_2[] = {
	0xD1, 0x77, 0xD4
};
static char power_setting_internal[] = {
	0xD3, 0x33
};
static char vplvl_vnlvl_setting[] = {
	0xD5, 0x0C, 0x0C
};
static char vcom_dc_setting_1[] = {
	0xD8, 0x34, 0x64, 0x23, 0x25, 0x62, 0x32
};
static char vcom_dc_setting_2[] = {
	0xDE, 0x10, 0xA8, 0x11, 0x08, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
};
static char nvm_load_ctrl[] = {
	0xE2, 0x00
};

/* Display ON Sequence */
static char exit_sleep[] = {
	0x11
};
static char display_on[] = {
	0x29
};

/* Display OFF Sequence */
static char enter_sleep[] = {
	0x10
};

/* Reading DDB Sequence */
static char read_ddb_start[] = {
	0xA1, 0x00
};

static struct dsi_cmd_desc display_init_cmd_seq_id_old[] = {
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,
		sizeof(mcap), mcap},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,
		sizeof(acr), acr},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,
		sizeof(clk_and_if), clk_and_if},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,
		sizeof(pixform), pixform},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,
		sizeof(pfm_pwm_ctrl), pfm_pwm_ctrl},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,
		sizeof(cabc_on_off), cabc_on_off},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,
		sizeof(cabc_user_param), cabc_user_param},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,
		sizeof(panel_driving), panel_driving},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,
		sizeof(h_timing), h_timing},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,
		sizeof(src_out), src_out},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,
		sizeof(gate_ic_ctrl), gate_ic_ctrl},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,
		sizeof(ltps_if_ctrl_1), ltps_if_ctrl_1},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,
		sizeof(src_out_mode), src_out_mode},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,
		sizeof(ltps_if_ctrl_2), ltps_if_ctrl_2},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,
		sizeof(gamma_ctrl), gamma_ctrl},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,
		sizeof(gamma_ctrl_set_r_pos), gamma_ctrl_set_r_pos},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,
		sizeof(gamma_ctrl_set_r_neg), gamma_ctrl_set_r_neg},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,
		sizeof(gamma_ctrl_set_g_pos), gamma_ctrl_set_g_pos},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,
		sizeof(gamma_ctrl_set_g_neg), gamma_ctrl_set_g_neg},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,
		sizeof(gamma_ctrl_set_b_pos), gamma_ctrl_set_b_pos},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,
		sizeof(gamma_ctrl_set_b_neg), gamma_ctrl_set_b_neg},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,
		sizeof(power_setting_1), power_setting_1},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,
		sizeof(power_setting_2), power_setting_2},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,
		sizeof(power_setting_internal), power_setting_internal},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,
		sizeof(vplvl_vnlvl_setting), vplvl_vnlvl_setting},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,
		sizeof(vcom_dc_setting_1), vcom_dc_setting_1},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,
		sizeof(vcom_dc_setting_2), vcom_dc_setting_2},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,
		sizeof(nvm_load_ctrl), nvm_load_ctrl},
};

static struct dsi_cmd_desc display_init_cmd_seq[] = {
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,
		sizeof(set_address_mode), set_address_mode},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,
		sizeof(mcap), mcap},
};

static struct dsi_cmd_desc display_on_cmd_seq[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 120,
		sizeof(exit_sleep), exit_sleep},
	{DTYPE_DCS_WRITE, 1, 0, 0, 0,
		sizeof(display_on), display_on},
};

static struct dsi_cmd_desc display_off_cmd_seq[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 120,
		sizeof(enter_sleep), enter_sleep}
};

static struct dsi_cmd_desc read_ddb_cmd_seq[] = {
	{DTYPE_DCS_READ, 1, 0, 1, 5, sizeof(read_ddb_start), read_ddb_start},
};

static const struct panel_cmd display_init_cmds[] = {
	{CMD_DSI, {.dsi_payload = {display_init_cmd_seq,
				ARRAY_SIZE(display_init_cmd_seq)} } },
	{CMD_END, {} },
};

static const struct panel_cmd display_init_cmds_id_old[] = {
	{CMD_DSI, {.dsi_payload = {display_init_cmd_seq_id_old,
				ARRAY_SIZE(display_init_cmd_seq_id_old)} } },
	{CMD_END, {} },
};

static const struct panel_cmd display_on_cmds[] = {
	{CMD_DSI, {.dsi_payload = {display_on_cmd_seq,
				ARRAY_SIZE(display_on_cmd_seq)} } },
	{CMD_END, {} },
};

static const struct panel_cmd display_off_cmds[] = {
	{CMD_DSI, {.dsi_payload = {display_off_cmd_seq,
				ARRAY_SIZE(display_off_cmd_seq)} } },
	{CMD_END, {} },
};

static const struct panel_cmd read_ddb_cmds[] = {
	{CMD_DSI, {.dsi_payload = {read_ddb_cmd_seq,
				ARRAY_SIZE(read_ddb_cmd_seq)} } },
	{CMD_END, {} },
};

static const struct mipi_dsi_phy_ctrl dsi_video_mode_phy_db[] = {
	/* 720*1280, RGB888, 4 Lane 60 fps video mode */
	{
		/* regulator */
		{0x03, 0x0a, 0x04, 0x00, 0x20},
		/* timing */
		{0x7b, 0x1b, 0x12, 0x00, 0x40, 0x49, 0x17, 0x1e,
		 0x1e, 0x03, 0x04, 0xa0},
		/* phy ctrl */
		{0x5f, 0x00, 0x00, 0x10},
		/* strength */
		{0xff, 0x00, 0x06, 0x00},
		/* pll control */
		{0x00, 0x9e, 0x31, 0xd9, 0x00, 0x50, 0x48, 0x63,
		 0x41, 0x0f, 0x03,
		 0x00, 0x14, 0x03, 0x00, 0x02, 0x00, 0x20, 0x00, 0x01 },
	},
};

static struct msm_panel_info pinfo;

static struct msm_panel_info *get_panel_info(void)
{
	pinfo.xres = 720;
	pinfo.yres = 1280;
	pinfo.type = MIPI_VIDEO_PANEL;
	pinfo.pdest = DISPLAY_1;
	pinfo.wait_cycle = 0;
	pinfo.bpp = 24;
	pinfo.lcdc.h_back_porch = 45;
	pinfo.lcdc.h_front_porch = 156;
	pinfo.lcdc.h_pulse_width = 3;
	pinfo.lcdc.v_back_porch = 3;
	pinfo.lcdc.v_front_porch = 9;
	pinfo.lcdc.v_pulse_width = 4;
	pinfo.lcdc.border_clr = 0;	/* blk */
	pinfo.lcdc.underflow_clr = 0xff;	/* blue */
	pinfo.lcdc.hsync_skew = 0;
	pinfo.bl_max = 15;
	pinfo.bl_min = 1;
	pinfo.fb_num = 2;
	pinfo.clk_rate = 431000000;

	pinfo.mipi.mode = DSI_VIDEO_MODE;
	pinfo.mipi.pulse_mode_hsa_he = TRUE;
	pinfo.mipi.hfp_power_stop = FALSE;
	pinfo.mipi.hbp_power_stop = FALSE;
	pinfo.mipi.hsa_power_stop = FALSE;
	pinfo.mipi.eof_bllp_power_stop = TRUE;
	pinfo.mipi.bllp_power_stop = TRUE;
	pinfo.mipi.traffic_mode = DSI_NON_BURST_SYNCH_EVENT;
	pinfo.mipi.dst_format = DSI_VIDEO_DST_FORMAT_RGB888;
	pinfo.mipi.vc = 0;
	pinfo.mipi.dlane_swap = 0x00;
	pinfo.mipi.rgb_swap = DSI_RGB_SWAP_RGB;
	pinfo.mipi.r_sel = 0;
	pinfo.mipi.g_sel = 0;
	pinfo.mipi.b_sel = 0;
	pinfo.mipi.data_lane0 = TRUE;
	pinfo.mipi.data_lane1 = TRUE;
	pinfo.mipi.data_lane2 = TRUE;
	pinfo.mipi.data_lane3 = TRUE;
	pinfo.mipi.tx_eot_append = TRUE;
	pinfo.mipi.t_clk_post = 0x04;
	pinfo.mipi.t_clk_pre = 0x1b;
	pinfo.mipi.stream = 0; /* dma_p */
	pinfo.mipi.mdp_trigger = DSI_CMD_TRIGGER_SW;
	pinfo.mipi.dma_trigger = DSI_CMD_TRIGGER_SW;
	pinfo.mipi.frame_rate  = 60;
	pinfo.mipi.dsi_phy_db =
		(struct mipi_dsi_phy_ctrl *)dsi_video_mode_phy_db;

	return &pinfo;
}

static struct dsi_controller dsi_video_controller_panel_id_old = {
	.get_panel_info = get_panel_info,
	.display_init = display_init_cmds_id_old,
	.display_on = display_on_cmds,
	.display_off = display_off_cmds,
	.read_id = read_ddb_cmds,
};

static struct dsi_controller dsi_video_controller_panel = {
	.get_panel_info = get_panel_info,
	.display_init = display_init_cmds,
	.display_on = display_on_cmds,
	.display_off = display_off_cmds,
	.read_id = read_ddb_cmds,
};

static char ddb_val_id_old[] = {
	0x01, 0xFF, 0xFF
};

static char ddb_val_1a[] = {
	0x12, 0x56, 0x81, 0x72, 0x1a, 0x01, 0x00, 0xff
};

static char ddb_val_1c[] = {
	0x12, 0x56, 0x81, 0x72, 0x1c, 0x01, 0x00, 0xff
};

static char ddb_val_1e[] = {
	0x12, 0x56, 0x81, 0x72, 0x1e, 0x01, 0x00, 0xff
};

static char ddb_val[] = {
	0x12, 0x56, 0x81, 0x72, 0xff, 0x01, 0x00, 0xff
};

const struct panel tmd_mdw30_panel_id_old = {
	.name = "mipi_video_tmd_wxga_mdw30_id_old_struct",
	.pctrl = &dsi_video_controller_panel_id_old,
	.id = ddb_val_id_old,
	.id_num = ARRAY_SIZE(ddb_val_id_old),
	.width = 57,
	.height = 101,
	.panel_id = "mdw30",
	.panel_rev = "old",
};

const struct panel tmd_mdw30_panel_id_1a = {
	.name = "mipi_video_tmd_wxga_mdw30_id_1a",
	.pctrl = &dsi_video_controller_panel,
	.id = ddb_val_1a,
	.id_num = ARRAY_SIZE(ddb_val_1a),
	.width = 57,
	.height = 101,
	.panel_id = "mdw30",
	.panel_rev = "1a",
};

const struct panel tmd_mdw30_panel_id_1c = {
	.name = "mipi_video_tmd_wxga_mdw30_id_1c",
	.pctrl = &dsi_video_controller_panel,
	.id = ddb_val_1c,
	.id_num = ARRAY_SIZE(ddb_val_1c),
	.width = 57,
	.height = 101,
	.panel_id = "mdw30",
	.panel_rev = "1c",
};

const struct panel tmd_mdw30_panel_id_1e = {
	.name = "mipi_video_tmd_wxga_mdw30_id_1e",
	.pctrl = &dsi_video_controller_panel,
	.id = ddb_val_1e,
	.id_num = ARRAY_SIZE(ddb_val_1e),
	.width = 57,
	.height = 101,
	.panel_id = "mdw30",
	.panel_rev = "1e",
};

const struct panel tmd_mdw30_panel_id = {
	.name = "mipi_video_tmd_wxga_mdw30",
	.pctrl = &dsi_video_controller_panel,
	.width = 57,
	.height = 101,
	.id = ddb_val,
	.id_num = ARRAY_SIZE(ddb_val),
	.width = 57,
	.height = 101,
	.panel_id = "mdw30",
	.panel_rev = "generic",
};
