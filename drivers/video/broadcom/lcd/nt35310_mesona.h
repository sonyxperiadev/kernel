/*******************************************************************************
* Copyright 2011 Broadcom Corporation.	All rights reserved.
* Copyright 2013 Sony Mobile Communications. All rights reserved.
*
* @file drivers/video/broadcom/lcd/nt35310.h
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

#ifndef __NT35310_MESONA_H__
#define __NT35310_MESONA_H__

#include "display_drv.h"
#include "lcd.h"

#define NT35310_MESONA_CMD_SLPIN	0x10
#define NT35310_MESONA_CMD_SLPOUT	0x11
#define NT35310_MESONA_CMD_DISPOFF	0x28
#define NT35310_MESONA_CMD_DISPON	0x29


#define NT35310_MESONA_UPDT_WIN_SEQ_LEN 13 /* (6 + 6 + 1) */

__initdata struct DSI_COUNTER nt35310_mesona_timing[] = {
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

__initdata DISPCTRL_REC_T nt35310_mesona_scrn_on[] = {
	{DISPCTRL_WR_CMND, NT35310_MESONA_CMD_DISPON},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_LIST_END, 0}
};

__initdata DISPCTRL_REC_T nt35310_mesona_scrn_off[] = {
	{DISPCTRL_WR_CMND, NT35310_MESONA_CMD_DISPOFF},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_LIST_END, 0}
};

__initdata DISPCTRL_REC_T nt35310_Mesona_id[] = {
	{DISPCTRL_WR_CMND, 0xDA},
	{DISPCTRL_WR_DATA, 0x0D},
/*	{DISPCTRL_WR_CMND, 0xDB},
	{DISPCTRL_WR_DATA, 0x80},
	{DISPCTRL_WR_CMND, 0xDC},
	{DISPCTRL_WR_DATA, 0x00},
*/	{DISPCTRL_LIST_END, 0}
};

__initdata DISPCTRL_REC_T nt35310_mesona_slp_in[] = {
	{DISPCTRL_WR_CMND, NT35310_MESONA_CMD_DISPOFF},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_CMND, NT35310_MESONA_CMD_SLPIN},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_SLEEP_MS, 120},
	{DISPCTRL_LIST_END, 0}
};

__initdata DISPCTRL_REC_T nt35310_mesona_slp_out[] = {
	{DISPCTRL_WR_CMND, NT35310_MESONA_CMD_SLPOUT},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_SLEEP_MS, 240},
	{DISPCTRL_LIST_END, 0}
};

__initdata DISPCTRL_REC_T nt35310_mesona_init_panel_cmd[] = {
	{DISPCTRL_WR_CMND, NT35310_MESONA_CMD_SLPOUT},
	{DISPCTRL_WR_DATA, 0x00},

	{DISPCTRL_WR_CMND, 0x35},
	{DISPCTRL_WR_DATA, 0x00},

	{DISPCTRL_WR_CMND, 0xED},
	{DISPCTRL_WR_DATA, 0x01},
	{DISPCTRL_WR_DATA, 0xFE},

	{DISPCTRL_WR_CMND, 0xB4},
	{DISPCTRL_WR_DATA, 0x15},

	{DISPCTRL_WR_CMND, 0xB7},
	{DISPCTRL_WR_DATA, 0x20},

	{DISPCTRL_WR_CMND, 0xC2},
	{DISPCTRL_WR_DATA, 0x24},
	{DISPCTRL_WR_DATA, 0x24},
	{DISPCTRL_WR_DATA, 0x24},

	{DISPCTRL_WR_CMND, 0xC6},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0xE4},
	{DISPCTRL_WR_DATA, 0xE4},
	{DISPCTRL_WR_DATA, 0xE4},

	{DISPCTRL_WR_CMND, 0xBF},
	{DISPCTRL_WR_DATA, 0xAA},

	/* ToDo: Add Gamma here... */

	{DISPCTRL_WR_CMND, 0xC1},
	{DISPCTRL_WR_DATA, 0x20},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x01},
	{DISPCTRL_WR_DATA, 0x00},

	{DISPCTRL_WR_CMND, 0x00},
	{DISPCTRL_WR_DATA, 0xAA},

	{DISPCTRL_WR_CMND, 0x29},
	{DISPCTRL_WR_DATA, 0x00},

	{DISPCTRL_WR_CMND, 0x53},
	{DISPCTRL_WR_DATA, 0x24},

	{DISPCTRL_WR_CMND, 0x51},
	{DISPCTRL_WR_DATA, 0x00},

	{DISPCTRL_LIST_END, 0}
};

void nt35310_mesona_winset(char *msgData, DISPDRV_WIN_t *p_win)
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

	if (i != NT35310_MESONA_UPDT_WIN_SEQ_LEN)
		pr_err("nt35310_winset msg len incorrect!\n");
}

__initdata struct lcd_config nt35310_mesona_cfg = {
	.name = "NT35310_Mesona",
	.mode_supp = LCD_CMD_ONLY,
	.phy_timing = &nt35310_mesona_timing[0],
	.max_lanes = 1,
	.max_hs_bps = 550000000,
	.max_lp_bps = 5000000,
	.phys_width = 55,
	.phys_height = 99,
	.init_cmd_seq = &nt35310_mesona_init_panel_cmd[0],
	.init_vid_seq = NULL,
	.slp_in_seq = &nt35310_mesona_slp_in[0],
	.slp_out_seq = &nt35310_mesona_slp_out[0],
	.scrn_on_seq = &nt35310_mesona_scrn_on[0],
	.scrn_off_seq = &nt35310_mesona_scrn_off[0],
	.id_seq = &nt35310_Mesona_id[0],
	.verify_id = true,
	.updt_win_fn = nt35310_mesona_winset,
	.updt_win_seq_len = NT35310_MESONA_UPDT_WIN_SEQ_LEN,
	.vid_cmnds = false,
	.vburst = false,
	.cont_clk = false,
	.hs = 0,
	.hbp = 0,
	.hfp = 0,
	.vs = 0,
	.vbp = 0,
	.vfp = 0,
	.special_mode_panel = false,
};

#endif
