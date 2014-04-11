/*******************************************************************************
* Copyright 2011 Broadcom Corporation.	All rights reserved.
* Copyright 2013 Sony Mobile Communications. All rights reserved.
*
* @file drivers/video/broadcom/lcd/nt35310_brooks.h
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

#ifndef __NT35310_BROOKS_H__
#define __NT35310_BROOKS_H__

#include "display_drv.h"
#include "lcd.h"

#define NT35310_BROOKS_UPDT_WIN_SEQ_LEN 13 /* (6 + 6 + 1) */

__initdata struct DSI_COUNTER nt35310_brooks_timing[] = {
	/* LP Data Symbol Rate Calc - MUST BE FIRST RECORD */
	{"ESC2LP_RATIO", DSI_C_TIME_ESC2LPDT, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0x0000003F, 1, 1, 0},
	/* SPEC:  min =  100[us] + 0[UI] */
	/* SET:   min = 1000[us] + 0[UI]                             <= */
	{"HS_INIT", DSI_C_TIME_HS, 0,
		0, 100000, 0, 0, 0, 0, 0, 0, 0x00FFFFFF, 0, 0, 0},
	/* SPEC:  min = 1[ms] + 0[UI] */
	/* SET:   min = 1[ms] + 0[UI] */
	{"HS_WAKEUP", DSI_C_TIME_HS, 0,
		0, 1000000, 0, 0, 0, 0, 0, 0, 0x00FFFFFF, 0, 0, 0},
	/* SPEC:  min = 1[ms] + 0[UI] */
	/* SET:   min = 1[ms] + 0[UI] */
	{"LP_WAKEUP", DSI_C_TIME_ESC, 0,
		0, 1000000, 0, 0, 0, 0, 0, 0, 0x00FFFFFF, 1, 1, 0},
	/* SPEC:  min = 0[ns] +  8[UI] */
	/* SET:   min = 0[ns] + 12[UI]                               <= */
	{"HS_CLK_PRE", DSI_C_TIME_HS, 0,
		0, 10, 8, 0, 0, 0, 0, 0, 0x000001FF, 0, 0, 0},
	/* SPEC:  min = 38[ns] + 0[UI]   max= 95[ns] + 0[UI] */
	/* SET:   min = 68[ns] + 0[UI]   max= 95[ns] + 0[UI]         <= */
	{"HS_CLK_PREPARE", DSI_C_TIME_HS, DSI_C_HAS_MAX,
		0, 75, 0, 0, 0, 95, 0, 0, 0x000001FF, 0, 0, 0},
	/* SPEC:  min = 262[ns] + 0[UI] */
	/* SET:   min = 314[ns] + 0[UI]                              <= */
	{"HS_CLK_ZERO", DSI_C_TIME_HS, 0,
		0, 300, 0, 0, 0, 0, 0, 0, 0x000001FF, 0, 0, 0},
	/* SPEC:  min =  60[ns] + 52[UI] */
	/* SET:   min =  72[ns] + 52[UI]                             <= */
	{"HS_CLK_POST", DSI_C_TIME_HS, 0,
		0, 70, 128, 0, 0, 0, 0, 0, 0x000001FF, 0, 0, 0},
	/* SPEC:  min =  60[ns] + 0[UI] */
	/* SET:   min =  72[ns] + 0[UI]                              <= */
	{"HS_CLK_TRAIL", DSI_C_TIME_HS, 0,
		0, 300, 0, 0, 0, 0, 0, 0, 0x000001FF, 0, 0, 0},
	/* SPEC:  min =  50[ns] + 0[UI] */
	/* SET:   min =  60[ns] + 0[UI]                              <= */
	{"HS_LPX", DSI_C_TIME_HS, 0,
		0, 70, 0, 0, 0, 75, 0, 0, 0x000001FF, 0, 0, 0},
	/* SPEC:  min = 40[ns] + 4[UI]      max= 85[ns] + 6[UI] */
	/* SET:   min = 60[ns] + 4[UI]      max= 85[ns] + 6[UI]      <= */
	{"HS_PRE", DSI_C_TIME_HS, DSI_C_HAS_MAX,
		0, 30, 4, 0, 0, 85, 6, 0, 0x000001FF, 0, 0, 0},
	/* SPEC:  min = 105[ns] + 6[UI] */
	/* SET:   min = 125[ns] + 6[UI]                              <= */
	{"HS_ZERO", DSI_C_TIME_HS, 0,
		0, 280, 6, 0, 0, 0, 0, 0, 0x000001FF, 0, 0, 0},
	/* SPEC:  min = max(0[ns]+32[UI],60[ns]+16[UI])  n=4 */
	/* SET:   min = max(0[ns]+32[UI],60[ns]+16[UI])  n=4 */
	{"HS_TRAIL", DSI_C_TIME_HS, DSI_C_MIN_MAX_OF_2,
		0, 60, 32, 100, 16, 0, 0, 0, 0x000001FF, 0, 0, 0},
	/* SPEC:  min = 100[ns] + 0[UI] */
	/* SET:   min = 120[ns] + 0[UI]                              <= */
	{"HS_EXIT", DSI_C_TIME_HS, 0,
		0, 300, 0, 0, 0, 0, 0, 0, 0x000001FF, 0, 0, 0},
	/* min = 50[ns] + 0[UI] */
	/* LP esc counters are speced in LP LPX units.
	   LP_LPX is calculated by chal_dsi_set_timing
	   and equals LP data clock */
	{"LPX", DSI_C_TIME_ESC, 0,
		1, 0, 0, 0, 0, 0, 0, 0, 0x000000FF, 1, 1, 0},
	/* min = 4*[Tlpx]  max = 4[Tlpx], set to 4 */
	{"LP-TA-GO", DSI_C_TIME_ESC, 0,
		4, 0, 0, 0, 0, 0, 0, 0, 0x000000FF, 1, 1, 0},
	/* min = 1*[Tlpx]  max = 2[Tlpx], set to 2 */
	{"LP-TA-SURE", DSI_C_TIME_ESC, 0,
		2, 0, 0, 0, 0, 0, 0, 0, 0x000000FF, 1, 1, 0},
	/* min = 5*[Tlpx]  max = 5[Tlpx], set to 5 */
	{"LP-TA-GET", DSI_C_TIME_ESC, 0,
		5, 0, 0, 0, 0, 0, 0, 0, 0x000000FF, 1, 1, 0},
};

__initdata DISPCTRL_REC_T nt35310_brooks_scrn_on[] = {
	{DISPCTRL_WR_CMND, 0x29},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_LIST_END, 0}
};

__initdata DISPCTRL_REC_T nt35310_brooks_scrn_off[] = {
	{DISPCTRL_WR_CMND, 0x28},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_LIST_END, 0}
};

__initdata DISPCTRL_REC_T nt35310_Brooks_id[] = {
	{DISPCTRL_WR_CMND, 0xDA},
	{DISPCTRL_WR_DATA, 0x26},
	{DISPCTRL_WR_CMND, 0xDB},
	{DISPCTRL_WR_DATA, 0x91},
/*	{DISPCTRL_WR_CMND, 0xDC},
	{DISPCTRL_WR_DATA, 0x00},
*/	{DISPCTRL_LIST_END, 0}
};

__initdata DISPCTRL_REC_T nt35310_brooks_slp_in[] = {
	{DISPCTRL_WR_CMND, 0x28},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_SLEEP_MS, 11},
	{DISPCTRL_WR_CMND, 0x10},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_SLEEP_MS, 101},
	{DISPCTRL_LIST_END, 0}
};

__initdata DISPCTRL_REC_T nt35310_brooks_slp_out[] = {
	{DISPCTRL_WR_CMND, 0x11},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_SLEEP_MS, 101},
	{DISPCTRL_LIST_END, 0}
};

__initdata DISPCTRL_REC_T nt35310_brooks_init_panel_cmd[] = {
	{DISPCTRL_WR_CMND, 0x11},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_SLEEP_MS, 96}, /* 16 * 6 frames (16 ms) */
	{DISPCTRL_LIST_END, 0}
};

__initdata DISPCTRL_REC_T nt35310_brooks_cabc[] = {
	{DISPCTRL_WR_CMND, 0x51}, /* Write Display Brightness */
	{DISPCTRL_WR_DATA, 0xFF},
	{DISPCTRL_WR_CMND, 0x53}, /* Write CTRL Display */
	{DISPCTRL_WR_DATA, 0x2C},
	{DISPCTRL_WR_CMND, 0x55}, /* Moving Mode */
	{DISPCTRL_WR_DATA, 0x03},
	{DISPCTRL_WR_CMND, 0xED}, /* Unlock CMD2 */
	{DISPCTRL_WR_DATA, 0x01},
	{DISPCTRL_WR_DATA, 0xFE},
	{DISPCTRL_WR_CMND, 0xDF}, /* MTP White function enable */
	{DISPCTRL_WR_DATA, 0x10},
	{DISPCTRL_WR_CMND, 0xBF}, /* CMD 2 Page 1 */
	{DISPCTRL_WR_DATA, 0xAA},
	{DISPCTRL_WR_CMND, 0xC1}, /* PWM frequency 25.3K */
	{DISPCTRL_WR_DATA, 0x20},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x01},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0xFF},
	{DISPCTRL_WR_DATA, 0x00},
	/* Start of step 5 in spec */
	{DISPCTRL_WR_CMND, 0xC4}, /* Setting Threshold */
	{DISPCTRL_WR_DATA, 0x62},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x05},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x84},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0xF0},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x18},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0xA4},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x12},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x50},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x0C},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x17},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x95},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0xF3},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0xE6},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_CMND, 0xAA}, /* Gamma Liner */
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_CMND, 0xE6}, /* CABC Gamma Offset */
	{DISPCTRL_WR_DATA, 0x22},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x22},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x62},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x44},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x44},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x64},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x67},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x76},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x77},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x54},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x25},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x56},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x35},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x77},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0xBB},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0xCC},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_CMND, 0xE7}, /* CABC Gamma Offset */
	{DISPCTRL_WR_DATA, 0x22},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x13},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x61},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x44},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x34},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x65},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x56},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x66},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x79},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x54},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x25},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x46},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x46},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x77},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0xBB},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0xCC},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_CMND, 0xE8}, /* CABC Gamma Offset */
	{DISPCTRL_WR_DATA, 0x22},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x22},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x62},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x43},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x44},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x55},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x56},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x87},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x76},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x55},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x26},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x56},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x45},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x87},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0xBB},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0xCC},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_CMND, 0xEF}, /* Back CMD1 */
	{DISPCTRL_WR_DATA, 0xAA},
	/* End of step 5 in spec */
	{DISPCTRL_LIST_END, 0}
};

__initdata DISPCTRL_REC_T nt35310_brooks_idle_mode_on[] = {
	{DISPCTRL_WR_CMND, 0x12}, /* Partial Mode On */
	{DISPCTRL_WR_CMND, 0x30}, /* Partial mode area */
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x01},
	{DISPCTRL_WR_DATA, 0x3F},
	{DISPCTRL_WR_CMND, 0x39}, /* Idle Mode */
	{DISPCTRL_WR_CMND, 0xED}, /* Unlock CMD2 */
	{DISPCTRL_WR_DATA, 0x01},
	{DISPCTRL_WR_DATA, 0xFE},
	{DISPCTRL_WR_CMND, 0xB0}, /* Hi-Z */
	{DISPCTRL_WR_DATA, 0x03},
	{DISPCTRL_WR_CMND, 0xB2}, /* Idle mode 40 Hz */
	{DISPCTRL_WR_DATA, 0x8D},
	{DISPCTRL_WR_DATA, 0xD3},
	{DISPCTRL_WR_DATA, 0x8D},
	{DISPCTRL_LIST_END, 0}
};

__initdata DISPCTRL_REC_T nt35310_brooks_idle_mode_off[] = {
	{DISPCTRL_WR_CMND, 0x38}, /* Idle Mode Off */
	{DISPCTRL_WR_CMND, 0x13}, /* Partial mode area */
	{DISPCTRL_LIST_END, 0}
};

void nt35310_brooks_winset(char *msgData, DISPDRV_WIN_t *p_win)
{
	int i = 0;
	msgData[i++] = 5; /* Length of the sequence below */
	msgData[i++] = MIPI_DCS_SET_COLUMN_ADDRESS;
	msgData[i++] = (p_win->l & 0xFF00) >> 8;
	msgData[i++] = (p_win->l & 0x00FF);
	msgData[i++] = (p_win->r & 0xFF00) >> 8;
	msgData[i++] = (p_win->r & 0x00FF);

	msgData[i++] = 5; /* Length of the sequence below */
	msgData[i++] = MIPI_DCS_SET_PAGE_ADDRESS;
	msgData[i++] = (p_win->t & 0xFF00) >> 8;
	msgData[i++] = (p_win->t & 0x00FF);
	msgData[i++] = (p_win->b & 0xFF00) >> 8;
	msgData[i++] = (p_win->b & 0x00FF);
	msgData[i++] = 0;

	if (i != NT35310_BROOKS_UPDT_WIN_SEQ_LEN)
		pr_err("nt35310_winset msg len incorrect!\n");
}

__initdata struct lcd_config nt35310_brooks_cfg = {
	.name = "NT35310_Brooks",
	.mode_supp = LCD_CMD_ONLY,
	.phy_timing = &nt35310_brooks_timing[0],
	.max_lanes = 1,
	.max_hs_bps = 550000000,
	.max_lp_bps = 5000000,
	.phys_width = 55,
	.phys_height = 99,
	.init_cmd_seq = &nt35310_brooks_init_panel_cmd[0],
	.init_vid_seq = NULL,
	.slp_in_seq = &nt35310_brooks_slp_in[0],
	.slp_out_seq = &nt35310_brooks_slp_out[0],
	.scrn_on_seq = &nt35310_brooks_scrn_on[0],
	.scrn_off_seq = &nt35310_brooks_scrn_off[0],
	.cabc_seq = &nt35310_brooks_cabc[0],
	.cabc_enabled = true,
	.id_seq = &nt35310_Brooks_id[0],
	.verify_id = true,
	.updt_win_fn = nt35310_brooks_winset,
	.updt_win_seq_len = NT35310_BROOKS_UPDT_WIN_SEQ_LEN,
	.vid_cmnds = false,
	.vburst = false,
	.cont_clk = false,
	.hs = 0,
	.hbp = 0,
	.hfp = 0,
	.vs = 0,
	.vbp = 0,
	.vfp = 0,
	.special_mode_panel = true,
	.special_mode_on = false,
	.special_mode_on_cmd_seq = &nt35310_brooks_idle_mode_on[0],
	.special_mode_off_cmd_seq = &nt35310_brooks_idle_mode_off[0],
	.clear_ram_row_start = 1,
	.clear_ram_row_end = 480,
	.clear_panel_ram = true,
};

#endif
