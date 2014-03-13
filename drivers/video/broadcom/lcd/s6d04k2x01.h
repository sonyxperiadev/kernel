/*******************************************************************************
* Copyright 2011 Broadcom Corporation.	All rights reserved.
* Copyright 2013 Sony Mobile Communications.	All rights reserved.
*
* @file drivers/video/broadcom/lcd/s6d04k2x01.h
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

#ifndef __S6D04K2X01_H__
#define __S6D04K2X01_H__

#include "display_drv.h"
#include "lcd.h"

#define S6D04K2X01_CMD_SLPIN	0x10
#define S6D04K2X01_CMD_SLPOUT	0x11
#define S6D04K2X01_CMD_DISPOFF	0x28
#define S6D04K2X01_CMD_DISPON	0x29
#define S6D04K2X01_CMD_RDID1    0xDA
#define S6D04K2X01_CMD_RDID2    0xDB
#define S6D04K2X01_CMD_RDID3    0xDC


/* ToDo: Check this data! */
__initdata struct DSI_COUNTER s6d04k2x01_timing[] = {
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
		0, 0, 8, 0, 0, 0, 0, 0, 0x000001FF, 0, 0, 0},
	/* SPEC:  min = 38[ns] + 0[UI]   max= 95[ns] + 0[UI] */
	/* SET:   min = 48[ns] + 0[UI]   max= 95[ns] + 0[UI]         <= */
	{"HS_CLK_PREPARE", DSI_C_TIME_HS, DSI_C_HAS_MAX,
		0, 38, 0, 0, 0, 95, 0, 0, 0x000001FF, 0, 0, 0},
	/* SPEC:  min = 262[ns] + 0[UI] */
	/* SET:   min = 262[ns] + 0[UI]                              <= */
	{"HS_CLK_ZERO", DSI_C_TIME_HS, 0,
		0, 240, 0, 0, 0, 0, 0, 0, 0x000001FF, 0, 0, 0},
	/* SPEC:  min =  60[ns] + 52[UI] */
	/* SET:   min =  70[ns] + 52[UI]                             <= */
	{"HS_CLK_POST", DSI_C_TIME_HS, 0,
		0, 60, 52, 0, 0, 0, 0, 0, 0x000001FF, 0, 0, 0},
	/* SPEC:  min =  60[ns] + 0[UI] */
	/* SET:   min =  70[ns] + 0[UI]                              <= */
	{"HS_CLK_TRAIL", DSI_C_TIME_HS, 0,
		0, 60, 0, 0, 0, 0, 0, 0, 0x000001FF, 0, 0, 0},
	/* SPEC:  min =  50[ns] + 0[UI] */
	/* SET:   min =  60[ns] + 0[UI]                              <= */
	{"HS_LPX", DSI_C_TIME_HS, 0,
		0, 50, 0, 0, 0, 75, 0, 0, 0x000001FF, 0, 0, 0},
	/* SPEC:  min = 40[ns] + 4[UI]      max= 85[ns] + 6[UI] */
	/* SET:   min = 50[ns] + 4[UI]      max= 85[ns] + 6[UI]      <= */
	{"HS_PRE", DSI_C_TIME_HS, DSI_C_HAS_MAX,
		0, 40, 4, 0, 0, 85, 6, 0, 0x000001FF, 0, 0, 0},
	/* SPEC:  min = 105[ns] + 6[UI] */
	/* SET:   min = 105[ns] + 6[UI]                              <= */
	{"HS_ZERO", DSI_C_TIME_HS, 0,
		0, 145, 10, 0, 0, 0, 0, 0, 0x000001FF, 0, 0, 0},
	/* SPEC:  min = max(0[ns]+32[UI],60[ns]+16[UI])  n=4 */
	/* SET:   min = max(0[ns]+32[UI],60[ns]+16[UI])  n=4 */
	{"HS_TRAIL", DSI_C_TIME_HS, DSI_C_MIN_MAX_OF_2,
		0, 60, 42, 60, 16, 0, 0, 0, 0x000001FF, 0, 0, 0},
	/* SPEC:  min = 100[ns] + 0[UI] */
	/* SET:   min = 110[ns] + 0[UI]                              <= */
	{"HS_EXIT", DSI_C_TIME_HS, 0,
		0, 110, 0, 0, 0, 0, 0, 0, 0x000001FF, 0, 0, 0},
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

__initdata DISPCTRL_REC_T s6d04k2x01_scrn_on[] = {
	{DISPCTRL_WR_CMND, S6D04K2X01_CMD_DISPON},
	{DISPCTRL_SLEEP_MS, 120},
	{DISPCTRL_LIST_END, 0}
};

__initdata DISPCTRL_REC_T s6d04k2x01_scrn_off[] = {
	{DISPCTRL_WR_CMND, S6D04K2X01_CMD_DISPOFF},
	{DISPCTRL_LIST_END, 0}
};

__initdata DISPCTRL_REC_T s6d04k2x01_id[] = {
	{DISPCTRL_WR_CMND, S6D04K2X01_CMD_RDID1},
	{DISPCTRL_WR_DATA, 0x26},
/*	{DISPCTRL_WR_CMND, S6D04K2X01_CMD_RDID2},
	{DISPCTRL_WR_DATA, 0x04},
	{DISPCTRL_WR_CMND, S6D04K2X01_CMD_RDID3},
	{DISPCTRL_WR_DATA, 0x04},
*/	{DISPCTRL_LIST_END, 0}
};

__initdata DISPCTRL_REC_T s6d04k2x01_slp_in[] = {
	{DISPCTRL_WR_CMND, S6D04K2X01_CMD_DISPOFF},
	{DISPCTRL_WR_CMND, S6D04K2X01_CMD_SLPIN},
	{DISPCTRL_SLEEP_MS, 120},
	{DISPCTRL_LIST_END, 0}
};

__initdata DISPCTRL_REC_T s6d04k2x01_slp_out[] = {
	{DISPCTRL_SLEEP_MS, 120},
	{DISPCTRL_WR_CMND, S6D04K2X01_CMD_SLPOUT},
	{DISPCTRL_SLEEP_MS, 18},
	{DISPCTRL_LIST_END, 0}
};

__initdata DISPCTRL_REC_T s6d04k2x01_init_panel_cmd[] = {
	{DISPCTRL_SLEEP_MS, 120},
	{DISPCTRL_WR_CMND, 0x11}, /* exit sleep */
	{DISPCTRL_SLEEP_MS, 120},

	{DISPCTRL_WR_CMND, 0xF0}, /* lvl2_enable */
	{DISPCTRL_WR_DATA, 0x5A},
	{DISPCTRL_WR_DATA, 0x5A},
	{DISPCTRL_WR_CMND, 0xF1},
	{DISPCTRL_WR_DATA, 0x5A},
	{DISPCTRL_WR_DATA, 0x5A},

	{DISPCTRL_WR_CMND, 0xF2}, /* display_ctrl */
	{DISPCTRL_WR_DATA, 0x16},
	{DISPCTRL_WR_DATA, 0xDC},
	{DISPCTRL_WR_DATA, 0x03},
	{DISPCTRL_WR_DATA, 0x28}, /* NVBP */
	{DISPCTRL_WR_DATA, 0x28}, /* NVFP */
	{DISPCTRL_WR_DATA, 0x10},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x60},
	{DISPCTRL_WR_DATA, 0xF8},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x07},
	{DISPCTRL_WR_DATA, 0x02},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0xDC},
	{DISPCTRL_WR_DATA, 0x28},
	{DISPCTRL_WR_DATA, 0x28},

	{DISPCTRL_WR_CMND, 0xF4}, /* power_ctrl */
	{DISPCTRL_WR_DATA, 0x0A},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x77},
	{DISPCTRL_WR_DATA, 0x7F},
	{DISPCTRL_WR_DATA, 0x07},
	{DISPCTRL_WR_DATA, 0x22},
	{DISPCTRL_WR_DATA, 0x2A},
	{DISPCTRL_WR_DATA, 0x43},
	{DISPCTRL_WR_DATA, 0x07},
	{DISPCTRL_WR_DATA, 0x2A},
	{DISPCTRL_WR_DATA, 0x43},
	{DISPCTRL_WR_DATA, 0x07},

	{DISPCTRL_WR_CMND, 0xF5}, /* vcom_ctrl */
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x50},
	{DISPCTRL_WR_DATA, 0x28},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x09},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x01},
	{DISPCTRL_WR_DATA, 0x01},

	{DISPCTRL_WR_CMND, 0xF6}, /* source_ctrl */
	{DISPCTRL_WR_DATA, 0x07},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x07},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x0B},
	{DISPCTRL_WR_DATA, 0x04},
	{DISPCTRL_WR_DATA, 0x04},
	{DISPCTRL_WR_DATA, 0x04},
	{DISPCTRL_WR_DATA, 0x07},

	{DISPCTRL_WR_CMND, 0xF7}, /* interface_ctrl */
	{DISPCTRL_WR_DATA, 0x02},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},

	{DISPCTRL_WR_CMND, 0xF8}, /* gate_ctrl */
	{DISPCTRL_WR_DATA, 0x44},
	{DISPCTRL_WR_DATA, 0x08},

	/* ToDo: Add gamma here! */

	{DISPCTRL_WR_CMND, 0xF0}, /* lvl2_disable */
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_CMND, 0xF1},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},

#if 1
	{DISPCTRL_WR_CMND, 0x36}, /* mad_ctrl */
	{DISPCTRL_WR_DATA, 0x48},

	{DISPCTRL_WR_CMND, 0x2A}, /*  Col */
	{DISPCTRL_WR_DATA, 0x00}, /* colm start */
	{DISPCTRL_WR_DATA, 0x32}, /* old val 0x32 */
	{DISPCTRL_WR_DATA, 0x01}, /* colm end */
	{DISPCTRL_WR_DATA, 0x0D},

	{DISPCTRL_WR_CMND, 0x2B}, /* Page */
	{DISPCTRL_WR_DATA, 0x00}, /* 0 */
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00}, /* 176 */
	{DISPCTRL_WR_DATA, 0xAF},
#endif
#if 0
	{DISPCTRL_WR_CMND, 0x0c},
	{DISPCTRL_WR_DATA, 0x0},
	{DISPCTRL_WR_DATA, 0x5},
#endif
	{DISPCTRL_WR_CMND, 0x35}, /* Enable tearing */
	{DISPCTRL_WR_DATA, 0x0},

	{DISPCTRL_SLEEP_MS, 120},
	{DISPCTRL_LIST_END, 0}
};


void s6d04k2x01_winset(char *msgData, DISPDRV_WIN_t *p_win)
{
#define S6D04K2X01_UPDT_WIN_SEQ_LEN 13 /* (6 + 6 + 1) */
	int i = 0;
	msgData[i++] = 5; /* Length of the sequence below */
	msgData[i++] = MIPI_DCS_SET_COLUMN_ADDRESS;
	msgData[i++] = (p_win->l  & 0xFF00) >> 8;
	/* + 0x32 is for an offset of 50 */
	msgData[i++] = ((p_win->l + 0x32) & 0x00FF);
	msgData[i++] = ((p_win->r + 0x32) & 0xFF00) >> 8;
	msgData[i++] = ((p_win->r + 0x32) & 0x00FF);

	msgData[i++] = 5; /* Length of the sequence below */
	msgData[i++] = MIPI_DCS_SET_PAGE_ADDRESS;
	msgData[i++] = (p_win->t & 0xFF00) >> 8;
	msgData[i++] = (p_win->t & 0x00FF);
	msgData[i++] = (p_win->b & 0xFF00) >> 8;
	msgData[i++] = (p_win->b & 0x00FF);
	msgData[i++] = 0;

	if (i != S6D04K2X01_UPDT_WIN_SEQ_LEN)
		pr_err("s6d04k2x01_winset msg len incorrect!\n");
}

__initdata struct lcd_config s6d04k2x01_cfg = {
	.name = "S6D04K2X01",
	.mode_supp = LCD_CMD_ONLY,
	.phy_timing = &s6d04k2x01_timing[0],
	.max_lanes = 1,
	.max_hs_bps = 300000000,
	.max_lp_bps = 100000000,
	.phys_width = 32,
	.phys_height = 25,
	.init_vid_seq = NULL,
	.init_cmd_seq = &s6d04k2x01_init_panel_cmd[0],
	.slp_in_seq = &s6d04k2x01_slp_in[0],
	.slp_out_seq = &s6d04k2x01_slp_out[0],
	.scrn_on_seq = &s6d04k2x01_scrn_on[0],
	.scrn_off_seq = &s6d04k2x01_scrn_off[0],
	.id_seq = &s6d04k2x01_id[0],
	.verify_id = true,
	.updt_win_fn = s6d04k2x01_winset,
	.updt_win_seq_len = S6D04K2X01_UPDT_WIN_SEQ_LEN,
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
