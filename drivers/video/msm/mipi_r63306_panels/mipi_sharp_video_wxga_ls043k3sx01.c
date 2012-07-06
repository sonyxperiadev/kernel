/* drivers/video/msm/mipi_r63306_panels/mipi_sharp_video_wxga_ls043k3sx01.c
 *
 * Copyright (C) [2011] Sony Ericsson Mobile Communications AB.
 * Copyright (C) 2012 Sony Mobile Communications AB.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2; as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */


#include "msm_fb.h"
#include "mipi_dsi.h"
#include "mipi_dsi_panel.h"

/* Initial Sequence */
static char mcap[] = {
	0xB0, 0x00
};
static char ltps_if_ctrl[] = {
	0xC4, 0xC3, 0x29
};
static char gamma_ctrl[] = {
	0xC8, 0x00, 0x00, 0x1F, 0x00
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

static struct dsi_cmd_desc display_init_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 120,
		sizeof(exit_sleep), exit_sleep},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,
		sizeof(mcap), mcap},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,
		sizeof(ltps_if_ctrl), ltps_if_ctrl},
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
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,
		sizeof(mcap_lock), mcap_lock},
};

static struct dsi_cmd_desc display_on_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 0,
		sizeof(display_on), display_on},
};

static struct dsi_cmd_desc display_off_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 0,
		sizeof(display_off), display_off},
	{DTYPE_DCS_WRITE, 1, 0, 0, 120,
		sizeof(enter_sleep), enter_sleep}
};

static struct dsi_cmd_desc read_ddb_start_cmds[] = {
	{DTYPE_DCS_READ, 1, 0, 1, 5, sizeof(read_ddb_start), read_ddb_start},
};

static const struct mipi_dsi_phy_ctrl dsi_video_mode_phy_db[] = {
	/* 720*1280, RGB888, 4 Lane 60 fps video mode */
	{
		{0x13, 0x01, 0x01, 0x00},	/* regulator */
		/* timing   */
		{0x78, 0x1a, 0x11, 0x00, 0x3e, 0x43, 0x16,
		 0x1d, 0x1d, 0x03, 0x04},
		{0x7f, 0x00, 0x00, 0x89},	/* phy ctrl */
		{0xdd, 0x02, 0x86, 0x00},	/* strength */
		/* pll control */
		{0x40, 0x8f, 0xb1, 0xd9, 0x00, 0x2f, 0x48, 0x63,
		0x31, 0x0f, 0x03,
		0x05, 0x14, 0x03, 0x00, 0x00, 0x54, 0x06, 0x10, 0x04, 0x00 },
	},
};

static struct msm_panel_info mipi_video_panel;

static struct msm_panel_info *get_panel_info(void)
{
	mipi_video_panel.xres = 720;
	mipi_video_panel.yres = 1280;
	mipi_video_panel.type = MIPI_VIDEO_PANEL;
	mipi_video_panel.pdest = DISPLAY_1;
	mipi_video_panel.wait_cycle = 0;
	mipi_video_panel.bpp = 24;
	mipi_video_panel.lcdc.h_back_porch = 45;
	mipi_video_panel.lcdc.h_front_porch = 128;
	mipi_video_panel.lcdc.h_pulse_width = 3;
	mipi_video_panel.lcdc.v_back_porch = 4;
	mipi_video_panel.lcdc.v_front_porch = 5;
	mipi_video_panel.lcdc.v_pulse_width = 1;
	mipi_video_panel.lcdc.border_clr = 0;	/* blk */
	mipi_video_panel.lcdc.underflow_clr = 0;	/* blk */
	mipi_video_panel.lcdc.hsync_skew = 0;
	mipi_video_panel.bl_max = 15;
	mipi_video_panel.bl_min = 1;
	mipi_video_panel.fb_num = 2;
	mipi_video_panel.clk_rate = 416000000;
	mipi_video_panel.lcd.refx100 = 6000;

	mipi_video_panel.mipi.mode = DSI_VIDEO_MODE;
	mipi_video_panel.mipi.pulse_mode_hsa_he = TRUE;
	mipi_video_panel.mipi.hfp_power_stop = FALSE;
	mipi_video_panel.mipi.hbp_power_stop = FALSE;
	mipi_video_panel.mipi.hsa_power_stop = FALSE;
	mipi_video_panel.mipi.eof_bllp_power_stop = TRUE;
	mipi_video_panel.mipi.bllp_power_stop = TRUE;
	mipi_video_panel.mipi.traffic_mode = DSI_NON_BURST_SYNCH_EVENT;
	mipi_video_panel.mipi.dst_format = DSI_VIDEO_DST_FORMAT_RGB888;
	mipi_video_panel.mipi.vc = 0;
	mipi_video_panel.mipi.dlane_swap = 0x00;
	mipi_video_panel.mipi.rgb_swap = DSI_RGB_SWAP_BGR;
	mipi_video_panel.mipi.r_sel = 0;
	mipi_video_panel.mipi.g_sel = 0;
	mipi_video_panel.mipi.b_sel = 0;
	mipi_video_panel.mipi.data_lane0 = TRUE;
	mipi_video_panel.mipi.data_lane1 = TRUE;
	mipi_video_panel.mipi.data_lane2 = TRUE;
	mipi_video_panel.mipi.data_lane3 = TRUE;
	mipi_video_panel.mipi.tx_eot_append = TRUE;
	mipi_video_panel.mipi.t_clk_post = 0x04;
	mipi_video_panel.mipi.t_clk_pre = 0x1B;
	mipi_video_panel.mipi.stream = 0; /* dma_p */
	mipi_video_panel.mipi.mdp_trigger = DSI_CMD_TRIGGER_SW;
	mipi_video_panel.mipi.dma_trigger = DSI_CMD_TRIGGER_SW;
	mipi_video_panel.mipi.frame_rate  = 60;
	mipi_video_panel.mipi.dsi_phy_db =
		(struct mipi_dsi_phy_ctrl *)dsi_video_mode_phy_db;

	return &mipi_video_panel;
}

static struct dsi_controller dsi_video_controller_panel = {
	.get_panel_info = get_panel_info,
	.display_init_cmds = display_init_cmds,
	.display_on_cmds = display_on_cmds,
	.display_off_cmds = display_off_cmds,
	.read_id_cmds = read_ddb_start_cmds,
	.display_init_cmds_size = ARRAY_SIZE(display_init_cmds),
	.display_on_cmds_size = ARRAY_SIZE(display_on_cmds),
	.display_off_cmds_size = ARRAY_SIZE(display_off_cmds),
};

static char ddb_val[] = {
	0x02, 0xff, 0xff
};

const struct panel_id sharp_ls043k3sx01_panel_id = {
	.name = "mipi_video_sharp_wxga_ls043k3sx01",
	.pctrl = &dsi_video_controller_panel,
	.width = 53,
	.height = 95,
	.id = ddb_val,
	.id_num = ARRAY_SIZE(ddb_val),
};

