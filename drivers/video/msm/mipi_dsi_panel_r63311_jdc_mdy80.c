/* drivers/video/msm/mipi_dsi_panel_r63311_jdc_mdy80.c
 *
 * Copyright (C) 2012 Sony Mobile Communications AB.
 *
 * Author: Perumal Jayamani<Perumal.Jayamani@sonymobile.com>
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
static char soft_reset[] = {
	0x01
};
static char mcap[] = {
	0xB0, 0x00
};

static char set_addr_mode[] = {
	0x36, 0xC0
};

static char exit_sleep[] = {
	0x11
};

static char interface_setting[] = {
	0xB3, 0x14, 0x00, 0x00, 0x20, 0x00, 0x00
};
static char interface_id_setting[] = {
	0xB4, 0x0C, 0x00
};
static char dsi_control[] = {
	0xB6, 0x3A, 0xD3
};

static char adaptive_brightness[] = {
	0x55, 0x00
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

static struct dsi_cmd_desc display_init_cmd_seq[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 5,
		sizeof(soft_reset), soft_reset},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,
		sizeof(mcap), mcap},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,
		sizeof(set_addr_mode), set_addr_mode},
	{DTYPE_DCS_WRITE, 1, 0, 0, 5,
		sizeof(exit_sleep), exit_sleep},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,
		sizeof(interface_setting), interface_setting},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,
		sizeof(interface_id_setting), interface_id_setting},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,
		sizeof(dsi_control), dsi_control},
};

static struct dsi_cmd_desc display_on_cmd_seq[] = {
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,
		sizeof(adaptive_brightness), adaptive_brightness},
	{DTYPE_DCS_WRITE, 1, 0, 0, 0,
		sizeof(display_on), display_on},
};

static struct dsi_cmd_desc display_off_cmd_seq[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 20,
		sizeof(display_off), display_off},
	{DTYPE_DCS_WRITE, 1, 0, 0, 80,
		sizeof(enter_sleep), enter_sleep},
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
	{CMD_WAIT_MS, {.data = 120} },
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
	/* 1080*1920, RGB888, 4 Lane 60 fps video mode */
	{
		/* regulator */
		{0x03, 0x0a, 0x04, 0x00, 0x20},
		/* timing */
		{0xe7, 0x39, 0x27, 0x00, 0x6a, 0x71, 0x2c, 0x3c,
		 0x41, 0x03, 0x04, 0xa0},
		/* phy ctrl */
		{0x5f, 0x00, 0x00, 0x10},
		/* strength */
		{0xff, 0x00, 0x06, 0x00},
		/* pll control */
		{0x00, 0xb1, 0x31, 0xd9, 0x00, 0x50, 0x48, 0x63,
		 0x41, 0x0f, 0x03,
		 0x00, 0x14, 0x03, 0x00, 0x02, 0x00, 0x20, 0x00, 0x01 },
	},
};

static struct msm_panel_info pinfo;

static struct msm_panel_info *get_panel_info(void)
{
	pinfo.xres = 1080;
	pinfo.yres = 1920;
	pinfo.type = MIPI_VIDEO_PANEL;
	pinfo.pdest = DISPLAY_1;
	pinfo.wait_cycle = 0;
	pinfo.bpp = 24;
	pinfo.lcdc.h_back_porch = 80;
	pinfo.lcdc.h_front_porch = 125;
	pinfo.lcdc.h_pulse_width = 10;
	pinfo.lcdc.v_back_porch = 4;
	pinfo.lcdc.v_front_porch = 8;
	pinfo.lcdc.v_pulse_width = 4;
	pinfo.lcdc.border_clr = 0;	/* blk */
	pinfo.lcdc.underflow_clr = 0xff;	/* blue */
	pinfo.lcdc.hsync_skew = 0;
	pinfo.bl_max = 15;
	pinfo.bl_min = 1;
	pinfo.fb_num = 2;
	pinfo.clk_rate = 901384616;

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
	pinfo.mipi.t_clk_post = 0x02;
	pinfo.mipi.t_clk_pre = 0x2d;
	pinfo.mipi.esc_byte_ratio = 9;
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

static char ddb_val_black[] = {
	0x12, 0x62, 0x47, 0x90
};

static char ddb_val_white[] = {
	0x12, 0x69, 0x45, 0x63
};

const struct panel jdc_mdy80_black_panel_id = {
	.name = "mipi_video_jdc_Full_HD_mdy80 black",
	.pctrl = &dsi_video_controller_panel,
	.id = ddb_val_black,
	.id_num = ARRAY_SIZE(ddb_val_black),
	.width = 62,
	.height = 110,
	.send_video_data_before_display_on = true,
	.panel_id = "1262-4790",
	.panel_rev = "generic",
};

const struct panel jdc_mdy80_white_panel_id = {
	.name = "mipi_video_jdc_Full_HD_mdy80 white",
	.pctrl = &dsi_video_controller_panel,
	.id = ddb_val_white,
	.id_num = ARRAY_SIZE(ddb_val_white),
	.width = 62,
	.height = 110,
	.send_video_data_before_display_on = true,
	.panel_id = "1269-4563",
	.panel_rev = "generic",
};
