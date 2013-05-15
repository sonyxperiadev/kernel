/* drivers/video/msm/mipi_dsi_panel_r63306_sharp_ls043k3sx04.c
 *
 * Copyright (c) 2012 Sony Mobile Communications AB.
 *
 * Author: Johan Olson <johan.olson@sonymobile.com>
 * Author: Joakim Wesslen <joakim.wesslen@sonymobile.com>
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
static char mcap[] = {
	0xB0, 0x00
};
static char ltps_if_ctrl[] = {
	0xC4, 0xC3, 0x29
};
static char gamma_ctrl[] = {
	0xC8, 0x10, 0x00, 0x1F, 0x00
};
static char gamma_ctrl_set_r_pos[] = {
	0xC9, 0x08, 0x03, 0x01, 0x01, 0x02, 0x05, 0x11,
	0x18, 0x10, 0x0C, 0x1F, 0x10, 0x20
};
static char gamma_ctrl_set_r_neg[] = {
	0xCA, 0x26, 0x2B, 0x50, 0x4F, 0x4E, 0x49, 0x3F,
	0x36, 0x3E, 0x41, 0x34, 0x29, 0x19
};
static char gamma_ctrl_set_g_pos[] = {
	0xCB, 0x29, 0x21, 0x1F, 0x1E, 0x1A, 0x13, 0x19,
	0x1C, 0x15, 0x11, 0x24, 0x14, 0x20
};
static char gamma_ctrl_set_g_neg[] = {
	0xCC, 0x09, 0x11, 0x32, 0x33, 0x36, 0x3D, 0x36,
	0x33, 0x39, 0x3C, 0x2C, 0x21, 0x19
};
static char gamma_ctrl_set_b_pos[] = {
	0xCD, 0x3C, 0x33, 0x32, 0x2A, 0x26, 0x14, 0x1B,
	0x1F, 0x19, 0x16, 0x26, 0x10, 0x20
};
static char gamma_ctrl_set_b_neg[] = {
	0xCE, 0x00, 0x00, 0x1F, 0x26, 0x2A, 0x3B, 0x34,
	0x2E, 0x34, 0x38, 0x28, 0x22, 0x19
};
static char mcap_lock[] = {
	0xB0, 0x03
};

/* Display ON Sequence */
static char exit_sleep[] = {
	0x11
};
static char display_on[] = {
	0x29
};

/* Display OFF Sequence */
static char display_off[] = {
	0x28
};

static char enter_sleep[] = {
	0x10
};

/* Reading DDB Sequence */
static char read_ddb_start[] = {
	0xA1, 0x00
};

static struct dsi_cmd_desc display_init_cmd_seq[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 120, sizeof(exit_sleep), exit_sleep},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(mcap), mcap},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(ltps_if_ctrl), ltps_if_ctrl},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(gamma_ctrl), gamma_ctrl},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(gamma_ctrl_set_r_pos),
							gamma_ctrl_set_r_pos},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(gamma_ctrl_set_r_neg),
							gamma_ctrl_set_r_neg},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(gamma_ctrl_set_g_pos),
							gamma_ctrl_set_g_pos},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(gamma_ctrl_set_g_neg),
							gamma_ctrl_set_g_neg},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(gamma_ctrl_set_b_pos),
							gamma_ctrl_set_b_pos},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(gamma_ctrl_set_b_neg),
							gamma_ctrl_set_b_neg},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(mcap_lock), mcap_lock},
};

static struct dsi_cmd_desc display_on_cmd_seq[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(display_on), display_on},
};

static struct dsi_cmd_desc display_off_cmd_seq[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(display_off), display_off},
	{DTYPE_DCS_WRITE, 1, 0, 0, 120, sizeof(enter_sleep), enter_sleep}
};

static struct dsi_cmd_desc read_ddb_cmd_seq[] = {
	{DTYPE_DCS_READ, 1, 0, 1, 5, sizeof(read_ddb_start), read_ddb_start},
};

static const struct panel_cmd display_init_cmds[] = {
	{CMD_DSI, {.dsi_payload = {display_init_cmd_seq,
				ARRAY_SIZE(display_init_cmd_seq)} } },
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
	pinfo.lcdc.h_front_porch = 128;
	pinfo.lcdc.h_pulse_width = 3;
	pinfo.lcdc.v_back_porch = 4;
	pinfo.lcdc.v_front_porch = 5;
	pinfo.lcdc.v_pulse_width = 1;
	pinfo.lcdc.border_clr = 0;	/* blk */
	pinfo.lcdc.underflow_clr = 0;	/* black */
	pinfo.lcdc.hsync_skew = 0;
	pinfo.bl_max = 15;
	pinfo.bl_min = 1;
	pinfo.fb_num = 2;
	pinfo.clk_rate = 416000000;

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
	pinfo.mipi.rgb_swap = DSI_RGB_SWAP_BGR;
	pinfo.mipi.r_sel = 0;
	pinfo.mipi.g_sel = 0;
	pinfo.mipi.b_sel = 0;
	pinfo.mipi.data_lane0 = TRUE;
	pinfo.mipi.data_lane1 = TRUE;
	pinfo.mipi.data_lane2 = TRUE;
	pinfo.mipi.data_lane3 = TRUE;
	pinfo.mipi.tx_eot_append = TRUE;
	pinfo.mipi.t_clk_post = 0x04;
	pinfo.mipi.t_clk_pre = 0x1B;
	pinfo.mipi.stream = 0; /* dma_p */
	pinfo.mipi.mdp_trigger = DSI_CMD_TRIGGER_SW;
	pinfo.mipi.dma_trigger = DSI_CMD_TRIGGER_SW;
	pinfo.mipi.frame_rate  = 60;
	pinfo.mipi.dsi_phy_db =
		(struct mipi_dsi_phy_ctrl *)dsi_video_mode_phy_db;

	return &pinfo;
}

static struct dsi_controller dsi_video_controller_panel = {
	.get_panel_info = get_panel_info,
	.display_init = display_init_cmds,
	.display_on = display_on_cmds,
	.display_off = display_off_cmds,
	.read_id = read_ddb_cmds,
};

static char ddb_val_1a[] = {
	0x12, 0x61, 0x41, 0x46, 0x1a, 0x01, 0x00, 0xff
};

static char ddb_val[] = {
	0x12, 0x61, 0x41, 0x46, 0xff, 0x01, 0x00, 0xff
};

const struct panel sharp_ls043k3sx04_panel_id_1a = {
	.name = "mipi_video_sharp_wxga_ls043k3sx04_id_1a",
	.pctrl = &dsi_video_controller_panel,
	.id = ddb_val_1a,
	.id_num = ARRAY_SIZE(ddb_val_1a),
	.width = 53,
	.height = 95,
	.panel_id = "ls043k3sx04",
	.panel_rev = "1a",
};

const struct panel sharp_ls043k3sx04_panel_id = {
	.name = "mipi_video_sharp_wxga_ls043k3sx04",
	.pctrl = &dsi_video_controller_panel,
	.id = ddb_val,
	.id_num = ARRAY_SIZE(ddb_val),
	.width = 53,
	.height = 95,
	.panel_id = "ls043k3sx04",
	.panel_rev = "generic",
};
