/* drivers/video/msm/mipi_dsi_panel_r63306_sharp_ls043k3sx04.c
 *
 * Copyright (c) 2012 Sony Mobile Communications AB.
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


#include "msm_fb.h"
#include "mipi_dsi.h"
#include "mipi_dsi_panel_driver.h"

static char exit_sleep[] = {
	0x11
};

static char display_on[] = {
	0x29
};

static char display_off[] = {
	0x28
};

static char enter_sleep[] = {
	0x10
};

static char read_ddb_start[] = {
	0xA1, 0x00
};

static char adaptive_brightness[] = {
	0x55, 0x00
};

static struct dsi_cmd_desc display_init_cmd_seq[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(exit_sleep), exit_sleep},
};

static struct dsi_cmd_desc display_on_cmd_seq[] = {
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,
			sizeof(adaptive_brightness), adaptive_brightness},
	{DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(display_on), display_on},
};

static struct dsi_cmd_desc display_off_cmd_seq[] = {
	/* Spec says delay > 20 ms */
	{DTYPE_DCS_WRITE, 1, 0, 0, 21, sizeof(display_off), display_off},
	/* Spec says delay > 101 ms */
	{DTYPE_DCS_WRITE, 1, 0, 0, 101, sizeof(enter_sleep), enter_sleep}
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
	{CMD_WAIT_MS, {.data = 151} },  /* Spec says > 150 ms */
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
	{
		/* regulator */
		{0x03, 0x0a, 0x04, 0x00, 0x20},
		/* timing   */
		{0xDE, 0x36, 0x24, 0x00, 0x66, 0x6E, 0x2A, 0x39,
		 0x3E, 0x03, 0x04, 0xA0},
		/* phy ctrl */
		{0x5f, 0x00, 0x00, 0x10},
		/* strength */
		{0xff, 0x00, 0x06, 0x00},
		/* pll control */
		{0x00, 0x9D, 0x31, 0xd9, 0x00, 0x50, 0x48, 0x63,
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
	pinfo.lcdc.h_back_porch = 48;
	pinfo.lcdc.h_front_porch = 100;
	pinfo.lcdc.h_pulse_width = 12;
	pinfo.lcdc.v_back_porch = 4;
	pinfo.lcdc.v_front_porch = 4;
	pinfo.lcdc.v_pulse_width = 2;
	pinfo.lcdc.border_clr = 0;	/* black */
	pinfo.lcdc.underflow_clr = 0x0;
	pinfo.lcdc.hsync_skew = 0;
	pinfo.bl_max = 15;
	pinfo.bl_min = 1;
	pinfo.fb_num = 2;
	pinfo.clk_rate = 859846200;

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
	pinfo.mipi.t_clk_pre = 0x2c;
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

static char ddb_val_12624797[] = {
	0x12, 0x62, 0x47, 0x97
};

const struct panel sharp_ls050t3sx02_r63311_12624797 = {
	.name = "Sharp LS050T3SX02 R63311, 1262-4797 black",
	.pctrl = &dsi_video_controller_panel,
	.id = ddb_val_12624797,
	.id_num = ARRAY_SIZE(ddb_val_12624797),
	.width = 62,
	.height = 110,
	.send_video_data_before_display_on = true,
	.panel_id = "1262-4791",
	.panel_rev = "generic",
};

static char ddb_val_black[] = {
	0x12, 0x62, 0x47, 0x91
};

const struct panel sharp_ls050t3sx02_r63311_black_panel_id = {
	.name = "Sharp LS050T3SX02 R63311 black",
	.pctrl = &dsi_video_controller_panel,
	.id = ddb_val_black,
	.id_num = ARRAY_SIZE(ddb_val_black),
	.width = 62,
	.height = 110,
	.send_video_data_before_display_on = true,
	.panel_id = "1262-4791",
	.panel_rev = "generic",
};

static char ddb_val_white[] = {
	0x12, 0x69, 0x65, 0x61
};

const struct panel sharp_ls050t3sx02_r63311_white_panel_id = {
	.name = "Sharp LS050T3SX02 R63311 white",
	.pctrl = &dsi_video_controller_panel,
	.id = ddb_val_white,
	.id_num = ARRAY_SIZE(ddb_val_white),
	.width = 62,
	.height = 110,
	.send_video_data_before_display_on = true,
	.panel_id = "1269-6561",
	.panel_rev = "generic",
};
