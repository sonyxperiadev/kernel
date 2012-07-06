/* drivers/video/msm/mipi_nt35565_panels/mipi_sharp_ls038k3sx01.c
 *
 * Copyright (C) [2011] Sony Ericsson Mobile Communications AB.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2; as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */


#include "../msm_fb.h"
#include "../mipi_dsi.h"
#include "../mipi_dsi_panel.h"


/* Initial Sequence */
static char exit_sleep[] = {
	0x11
};
static char set_horizontal_address[] = {
	0x2A, 0x00, 0x00, 0x02, 0x1B
};
static char set_vertical_address[] = {
	0x2B, 0x00, 0x00, 0x03, 0xBF
};
static char set_address_mode[] = {
	0x36, 0x00
};
static char set_pixel_format[] = {
	0x3A, 0x77
};
static char set_tear_on[] = {
	0x35, 0x00
};
static char set_tear_scanline[] = {
	0x44, 0x00, 0x01
};
static char cmd2_unlock[] = {
	0xF3, 0xAA
};
static char page0_ctrl[] = {
	0x00, 0x00
};
static char display_ctrl[] = {
	0xA2, 0x03
};
static char cmd2_p0_lock[] = {
	0xFF, 0xAA
};
/* Display ON Sequence */
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

static struct dsi_cmd_desc sharp_display_init_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 150,
		sizeof(exit_sleep), exit_sleep},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,
		sizeof(set_horizontal_address), set_horizontal_address},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,
		sizeof(set_vertical_address), set_vertical_address},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,
		sizeof(set_address_mode), set_address_mode},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,
		sizeof(set_pixel_format), set_pixel_format},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,
		sizeof(set_tear_on), set_tear_on},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,
		sizeof(set_tear_scanline), set_tear_scanline},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,
		sizeof(cmd2_unlock), cmd2_unlock},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,
		sizeof(page0_ctrl), page0_ctrl},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,
		sizeof(display_ctrl), display_ctrl},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,
		sizeof(cmd2_p0_lock), cmd2_p0_lock},
};

static struct dsi_cmd_desc sharp_display_on_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 0,
		sizeof(display_on), display_on},
};

static struct dsi_cmd_desc sharp_display_off_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 0,
		sizeof(display_off), display_off},
	{DTYPE_DCS_WRITE, 1, 0, 0, 120,
		sizeof(enter_sleep), enter_sleep}
};

static struct dsi_cmd_desc read_ddb_start_cmds[] = {
	{DTYPE_DCS_READ, 1, 0, 1, 5, sizeof(read_ddb_start), read_ddb_start},
};

static const struct mipi_dsi_phy_ctrl dsi_cmd_mode_phy_db[] = {
	/* 540*960, RGB888, 2 Lane 60 fps command mode */
	{
		/* regulator */
		{0x03, 0x0a, 0x04, 0x00, 0x20},
		/* timing */
		{0x73, 0x19, 0x11, 0x00, 0x3c, 0x46, 0x14, 0x1c,
		 0x1c, 0x03, 0x04, 0xa0},
		/* phy ctrl */
		{0x5f, 0x00, 0x00, 0x10},
		/* strength */
		{0xff, 0x00, 0x06, 0x00},
		/* pll control */
		{0x00, 0x7d, 0x31, 0xd9, 0x00, 0x50, 0x48, 0x63,
		 0x41, 0x0f, 0x03,
		 0x00, 0x14, 0x03, 0x00, 0x02, 0x00, 0x20, 0x00, 0x01 },
	},
};

static struct msm_panel_info pinfo;

static struct msm_panel_info *get_panel_info(void)
{
	/* should fix porch, pulse widht and so on */
	pinfo.xres = 540;
	pinfo.yres = 960;
	pinfo.type = MIPI_CMD_PANEL;
	pinfo.pdest = DISPLAY_1;
	pinfo.wait_cycle = 0;
	pinfo.bpp = 24;
	pinfo.lcdc.h_back_porch = 4;
	pinfo.lcdc.h_front_porch = 18;
	pinfo.lcdc.h_pulse_width = 2;
	pinfo.lcdc.v_back_porch = 11;
	pinfo.lcdc.v_front_porch = 4;
	pinfo.lcdc.v_pulse_width = 2;
	pinfo.lcdc.border_clr = 0;	/* blk */
	pinfo.lcdc.underflow_clr = 0xff;	/* blue */
	pinfo.lcdc.hsync_skew = 0;
	pinfo.bl_max = 255;
	pinfo.bl_min = 1;
	pinfo.fb_num = 2;
	pinfo.clk_rate = 397000000;
	pinfo.lcd.vsync_enable = TRUE;
	pinfo.lcd.hw_vsync_mode = TRUE;
	pinfo.lcd.refx100 = 6000; /* adjust refx100 to prevent tearing */
	pinfo.lcd.v_back_porch = 11;
	pinfo.lcd.v_front_porch = 4;
	pinfo.lcd.v_pulse_width = 2;

	pinfo.mipi.mode = DSI_CMD_MODE;
	pinfo.mipi.dst_format = DSI_CMD_DST_FORMAT_RGB888;
	pinfo.mipi.vc = 0;
	pinfo.mipi.dlane_swap = 0x00;
	pinfo.mipi.rgb_swap = DSI_RGB_SWAP_RGB;
	pinfo.mipi.data_lane0 = TRUE;
	pinfo.mipi.data_lane1 = TRUE;
	pinfo.mipi.t_clk_post = 0x04;
	pinfo.mipi.t_clk_pre = 0x1a;
	pinfo.mipi.stream = 0; /* dma_p */
	pinfo.mipi.mdp_trigger = DSI_CMD_TRIGGER_SW;
	pinfo.mipi.dma_trigger = DSI_CMD_TRIGGER_SW;
	pinfo.mipi.te_sel = 1; /* TE from vsycn gpio */
	pinfo.mipi.interleave_max = 1;
	pinfo.mipi.insert_dcs_cmd = TRUE;
	pinfo.mipi.wr_mem_continue = 0x3c;
	pinfo.mipi.wr_mem_start = 0x2c;
	pinfo.mipi.dsi_phy_db =
		(struct mipi_dsi_phy_ctrl *)dsi_cmd_mode_phy_db;

	return &pinfo;
}

static struct dsi_controller dsi_cmd_controller_panel = {
	.get_panel_info = get_panel_info,
	.display_init_cmds = sharp_display_init_cmds,
	.display_on_cmds = sharp_display_on_cmds,
	.display_off_cmds = sharp_display_off_cmds,
	.read_id_cmds = read_ddb_start_cmds,
	.display_init_cmds_size = ARRAY_SIZE(sharp_display_init_cmds),
	.display_on_cmds_size = ARRAY_SIZE(sharp_display_on_cmds),
	.display_off_cmds_size = ARRAY_SIZE(sharp_display_off_cmds),
};

static char ddb_val_1a[] = {
	0x12, 0x57, 0x77, 0x75, 0x1a, 0x01, 0xff
};

static char ddb_val_1b[] = {
	0x12, 0x57, 0x77, 0x75, 0x1b, 0x01, 0xff
};

static char ddb_val[] = {
	0x12, 0x57, 0x77, 0x75, 0xff, 0x01, 0xff
};

static char default_ddb_val[] = {
	0x12, 0x57, 0x77, 0x75
};

const struct panel_id sharp_ls038k3sx01_panel_id_1a = {
	.name = "mipi_cmd_sharp_qhd_ls038k3sx01_id_1a",
	.pctrl = &dsi_cmd_controller_panel,
	.id = ddb_val_1a,
	.id_num = ARRAY_SIZE(ddb_val_1a),
	.width = 46,
	.height = 81,
};

const struct panel_id sharp_ls038k3sx01_panel_id_1b = {
	.name = "mipi_cmd_sharp_qhd_ls038k3sx01_id_1b",
	.pctrl = &dsi_cmd_controller_panel,
	.id = ddb_val_1b,
	.id_num = ARRAY_SIZE(ddb_val_1b),
	.width = 46,
	.height = 81,
};

const struct panel_id sharp_ls038k3sx01_panel_id = {
	.name = "mipi_cmd_sharp_qhd_ls038k3sx01",
	.pctrl = &dsi_cmd_controller_panel,
	.id = ddb_val,
	.id_num = ARRAY_SIZE(ddb_val),
	.width = 46,
	.height = 81,
};

const struct panel_id sharp_ls038k3sx01_panel_default = {
	.name = "mipi_sharp_panel",
	.pctrl = &dsi_cmd_controller_panel,
	.id = default_ddb_val,
	.id_num = ARRAY_SIZE(default_ddb_val),
	.width = 46,
	.height = 81,
};
