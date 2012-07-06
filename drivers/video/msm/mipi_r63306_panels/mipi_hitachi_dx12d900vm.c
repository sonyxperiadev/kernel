/* drivers/video/msm/mipi_r63306_panels/mipi_hitachi_dx12d900vm.c
 *
 * Copyright (C) [2011] Sony Ericsson Mobile Communications AB.
 * Author: Macro Luo <macro.luo@sonyericsson.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2; as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */


#include "msm_fb.h"
#include "mipi_dsi.h"
#include "mipi_dsi_panel.h"

/* Display ON Sequence */
static char exit_sleep[] = {
	0x11
};
static char set_address_mode[] = {
	0x36, 0x00
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
	0xA1
};

static struct dsi_cmd_desc hitachi_display_init_cmds[] = {
};

static struct dsi_cmd_desc hitachi_display_on_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 70,
		sizeof(exit_sleep), exit_sleep},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 18,
		sizeof(set_address_mode), set_address_mode},
	{DTYPE_DCS_WRITE, 1, 0, 0, 0,
		sizeof(display_on), display_on},
};

static struct dsi_cmd_desc hitachi_display_off_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 0,
		sizeof(display_off), display_off},
	{DTYPE_DCS_WRITE, 1, 0, 0, 52,
		sizeof(enter_sleep), enter_sleep},
};

static struct dsi_cmd_desc read_ddb_start_cmds[] = {
	{DTYPE_DCS_READ, 1, 0, 1, 5, sizeof(read_ddb_start), read_ddb_start},
};

static const struct mipi_dsi_phy_ctrl dsi_video_mode_phy_db[] = {
	/* 720*1280, RGB888, 4 Lane 60 fps video mode */
	{
		/* regulator */
		{0x03, 0x0a, 0x04, 0x00, 0x20},
		/* timing */
		{0xac, 0x8b, 0x18, 0x00, 0x1b, 0x91, 0x1c, 0x8d,
		 0x1b, 0x03, 0x04, 0xa0},
		/* phy ctrl */
		{0x5f, 0x00, 0x00, 0x10},
		/* strength */
		{0xff, 0x00, 0x06, 0x00},
		/* pll control */
		{0x00, 0x76, 0x01, 0x19, 0x00, 0x40, 0x03, 0x62,
		 0x41, 0x0f, 0x03,
		 0x00, 0x1a, 0x00, 0x00, 0x02, 0x00, 0x20, 0x00, 0x01 },
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
	pinfo.lcdc.h_back_porch = 20;
	pinfo.lcdc.h_front_porch = 80;
	pinfo.lcdc.h_pulse_width = 10;
	pinfo.lcdc.v_back_porch = 10;
	pinfo.lcdc.v_front_porch = 12;
	pinfo.lcdc.v_pulse_width = 2;
	pinfo.lcdc.border_clr = 0;	/* blk */
	pinfo.lcdc.underflow_clr = 0xff;	/* blue */
	pinfo.lcdc.hsync_skew = 0;
	pinfo.bl_max = 15;
	pinfo.bl_min = 1;
	pinfo.fb_num = 2;
	pinfo.clk_rate = 390000000;

	pinfo.mipi.xres_pad = 0;
	pinfo.mipi.yres_pad = 0;
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
	pinfo.mipi.rgb_swap = DSI_RGB_SWAP_RGB;
	pinfo.mipi.r_sel = 0;
	pinfo.mipi.g_sel = 0;
	pinfo.mipi.b_sel = 0;
	pinfo.mipi.data_lane0 = TRUE;
	pinfo.mipi.data_lane1 = TRUE;
	pinfo.mipi.data_lane2 = TRUE;
	pinfo.mipi.data_lane3 = TRUE;
	pinfo.mipi.tx_eot_append = TRUE;
	pinfo.mipi.t_clk_post = 34;
	pinfo.mipi.t_clk_pre = 61;
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
	.display_init_cmds = hitachi_display_init_cmds,
	.display_on_cmds = hitachi_display_on_cmds,
	.display_off_cmds = hitachi_display_off_cmds,
	.read_id_cmds = read_ddb_start_cmds,
	.display_init_cmds_size = ARRAY_SIZE(hitachi_display_init_cmds),
	.display_on_cmds_size = ARRAY_SIZE(hitachi_display_on_cmds),
	.display_off_cmds_size = ARRAY_SIZE(hitachi_display_off_cmds),
};

static char ddb_val[] = {
	0x12, 0x52, 0x02, 0x14
};

const struct panel_id hitachi_dx12d900vm_panel_id = {
	.name = "mipi_video_hitachi_wxga_dx12d900vm",
	.pctrl = &dsi_video_controller_panel,
	.id = ddb_val,
	.id_num = ARRAY_SIZE(ddb_val),
};
