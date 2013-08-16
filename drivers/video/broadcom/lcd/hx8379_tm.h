/*******************************************************************************
* Copyright 2011 Broadcom Corporation.	All rights reserved.
*
* @file drivers/video/broadcom/lcd/hx8379_tm.h
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

#ifndef __HX8379_TM_H__
#define __HX8379_TM_H__

#include "display_drv.h"
#include "lcd.h"

#define HX8379_TM_CMD_SLPIN 0x10
#define HX8379_TM_CMD_SLPOUT 0x11
#define HX8379_TM_CMD_DISPOFF 0x28
#define HX8379_TM_CMD_DISPON 0x29

__initdata struct DSI_COUNTER hx8379_tm_timing[] = {
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
		0, 0, 12, 0, 0, 0, 0, 0, 0x000001FF, 0, 0, 0},
	/* SPEC:  min = 38[ns] + 0[UI]   max= 95[ns] + 0[UI] */
	/* SET:   min = 48[ns] + 0[UI]   max= 95[ns] + 0[UI]         <= */
	{"HS_CLK_PREPARE", DSI_C_TIME_HS, DSI_C_HAS_MAX,
		0, 48, 0, 0, 0, 95, 0, 0, 0x000001FF, 0, 0, 0},
	/* SPEC:  min = 262[ns] + 0[UI] */
	/* SET:   min = 262[ns] + 0[UI]                              <= */
	{"HS_CLK_ZERO", DSI_C_TIME_HS, 0,
		0, 262, 0, 0, 0, 0, 0, 0, 0x000001FF, 0, 0, 0},
	/* SPEC:  min =  60[ns] + 52[UI] */
	/* SET:   min =  70[ns] + 52[UI]                             <= */
	{"HS_CLK_POST", DSI_C_TIME_HS, 0,
		0, 70, 52, 0, 0, 0, 0, 0, 0x000001FF, 0, 0, 0},
	/* SPEC:  min =  60[ns] + 0[UI] */
	/* SET:   min =  70[ns] + 0[UI]                              <= */
	{"HS_CLK_TRAIL", DSI_C_TIME_HS, 0,
		0, 70, 0, 0, 0, 0, 0, 0, 0x000001FF, 0, 0, 0},
	/* SPEC:  min =  50[ns] + 0[UI] */
	/* SET:   min =  60[ns] + 0[UI]                              <= */
	{"HS_LPX", DSI_C_TIME_HS, 0,
		0, 60, 0, 0, 0, 75, 0, 0, 0x000001FF, 0, 0, 0},
	/* SPEC:  min = 40[ns] + 4[UI]      max= 85[ns] + 6[UI] */
	/* SET:   min = 50[ns] + 4[UI]      max= 85[ns] + 6[UI]      <= */
	{"HS_PRE", DSI_C_TIME_HS, DSI_C_HAS_MAX,
		0, 50, 4, 0, 0, 85, 6, 0, 0x000001FF, 0, 0, 0},
	/* SPEC:  min = 105[ns] + 6[UI] */
	/* SET:   min = 105[ns] + 6[UI]                              <= */
	{"HS_ZERO", DSI_C_TIME_HS, 0,
		0, 105, 6, 0, 0, 0, 0, 0, 0x000001FF, 0, 0, 0},
	/* SPEC:  min = max(0[ns]+32[UI],60[ns]+16[UI])  n=4 */
	/* SET:   min = max(0[ns]+32[UI],60[ns]+16[UI])  n=4 */
	{"HS_TRAIL", DSI_C_TIME_HS, DSI_C_MIN_MAX_OF_2,
		0, 0, 32, 60, 16, 0, 0, 0, 0x000001FF, 0, 0, 0},
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

__initdata DISPCTRL_REC_T hx8379_tm_scrn_on[] = {
	{DISPCTRL_WR_CMND, HX8379_TM_CMD_DISPON},
	{DISPCTRL_LIST_END, 0}
};

__initdata DISPCTRL_REC_T hx8379_tm_scrn_off[] = {
	{DISPCTRL_WR_CMND, HX8379_TM_CMD_DISPOFF},
	{DISPCTRL_LIST_END, 0}
};

__initdata DISPCTRL_REC_T hx8379_tm_slp_in[] = {
	{DISPCTRL_WR_CMND, HX8379_TM_CMD_DISPOFF},
	{DISPCTRL_WR_CMND, HX8379_TM_CMD_SLPIN},
	{DISPCTRL_SLEEP_MS, 120},
	{DISPCTRL_LIST_END, 0}
};

__initdata DISPCTRL_REC_T hx8379_tm_slp_out[] = {
	{DISPCTRL_WR_CMND, HX8379_TM_CMD_SLPOUT},
	{DISPCTRL_SLEEP_MS, 120},
	{DISPCTRL_LIST_END, 0}
};

__initdata DISPCTRL_REC_T hx8379_tm_init_panel_vid[] = {
	{DISPCTRL_WR_CMND, 0xB9},
	{DISPCTRL_WR_DATA, 0xFF},
	{DISPCTRL_WR_DATA, 0x83},
	{DISPCTRL_WR_DATA, 0x79},

	{DISPCTRL_WR_CMND, 0xBA},
	{DISPCTRL_WR_DATA, 0x51},
	{DISPCTRL_WR_DATA, 0x93},

	{DISPCTRL_WR_CMND, 0xB1},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x50},
	{DISPCTRL_WR_DATA, 0x14},
	{DISPCTRL_WR_DATA, 0xEA},
	{DISPCTRL_WR_DATA, 0x90},
	{DISPCTRL_WR_DATA, 0x08},
	{DISPCTRL_WR_DATA, 0x11},
	{DISPCTRL_WR_DATA, 0x11},
	{DISPCTRL_WR_DATA, 0x11},
	{DISPCTRL_WR_DATA, 0x28},
	{DISPCTRL_WR_DATA, 0x30},
	{DISPCTRL_WR_DATA, 0xBF},
	{DISPCTRL_WR_DATA, 0x3F},
	{DISPCTRL_WR_DATA, 0x42},
	{DISPCTRL_WR_DATA, 0x0B},
	{DISPCTRL_WR_DATA, 0x76},
	{DISPCTRL_WR_DATA, 0xF1},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0xE6},

	{DISPCTRL_WR_CMND, 0xB2},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0xFE},
	{DISPCTRL_WR_DATA, 0x0B},
	{DISPCTRL_WR_DATA, 0x11},
	{DISPCTRL_WR_DATA, 0x19},
	{DISPCTRL_WR_DATA, 0x22},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0xFF},
	{DISPCTRL_WR_DATA, 0x0B},
	{DISPCTRL_WR_DATA, 0x11},
	{DISPCTRL_WR_DATA, 0x19},
	{DISPCTRL_WR_DATA, 0x20},

	{DISPCTRL_WR_CMND, 0xB4},
	{DISPCTRL_WR_DATA, 0x80},
	{DISPCTRL_WR_DATA, 0x08},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x32},
	{DISPCTRL_WR_DATA, 0x10},
	{DISPCTRL_WR_DATA, 0x03},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x33},
	{DISPCTRL_WR_DATA, 0x0A},
	{DISPCTRL_WR_DATA, 0x40},
	{DISPCTRL_WR_DATA, 0x07},
	{DISPCTRL_WR_DATA, 0x37},
	{DISPCTRL_WR_DATA, 0x04},
	{DISPCTRL_WR_DATA, 0x20},
	{DISPCTRL_WR_DATA, 0x08},
	{DISPCTRL_WR_DATA, 0x20},
	{DISPCTRL_WR_DATA, 0x20},
	{DISPCTRL_WR_DATA, 0x02},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x40},
	{DISPCTRL_WR_DATA, 0x0A},
	{DISPCTRL_WR_DATA, 0x30},
	{DISPCTRL_WR_DATA, 0x14},
	{DISPCTRL_WR_DATA, 0x40},
	{DISPCTRL_WR_DATA, 0x55},
	{DISPCTRL_WR_DATA, 0x0A},

	{DISPCTRL_WR_CMND, 0xD5},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x03},
	{DISPCTRL_WR_DATA, 0x0A},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x01},
	{DISPCTRL_WR_DATA, 0x05},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x03},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x99},
	{DISPCTRL_WR_DATA, 0x88},
	{DISPCTRL_WR_DATA, 0x88},
	{DISPCTRL_WR_DATA, 0x88},
	{DISPCTRL_WR_DATA, 0x23},
	{DISPCTRL_WR_DATA, 0x01},
	{DISPCTRL_WR_DATA, 0x67},
	{DISPCTRL_WR_DATA, 0x45},
	{DISPCTRL_WR_DATA, 0x01},
	{DISPCTRL_WR_DATA, 0x23},
	{DISPCTRL_WR_DATA, 0x88},
	{DISPCTRL_WR_DATA, 0x88},
	{DISPCTRL_WR_DATA, 0x88},
	{DISPCTRL_WR_DATA, 0x88},
	{DISPCTRL_WR_DATA, 0x88},
	{DISPCTRL_WR_DATA, 0x88},
	{DISPCTRL_WR_DATA, 0x88},
	{DISPCTRL_WR_DATA, 0x99},
	{DISPCTRL_WR_DATA, 0x88},
	{DISPCTRL_WR_DATA, 0x88},
	{DISPCTRL_WR_DATA, 0x32},
	{DISPCTRL_WR_DATA, 0x54},
	{DISPCTRL_WR_DATA, 0x76},
	{DISPCTRL_WR_DATA, 0x10},
	{DISPCTRL_WR_DATA, 0x10},
	{DISPCTRL_WR_DATA, 0x32},
	{DISPCTRL_WR_DATA, 0x88},
	{DISPCTRL_WR_DATA, 0x88},
	{DISPCTRL_WR_DATA, 0x88},
	{DISPCTRL_WR_DATA, 0x88},
	{DISPCTRL_WR_DATA, 0x88},
	{DISPCTRL_WR_DATA, 0x88},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},

	{DISPCTRL_WR_CMND, 0xE0},
	{DISPCTRL_WR_DATA, 0x79},
	{DISPCTRL_WR_DATA, 0x01},
	{DISPCTRL_WR_DATA, 0x21},
	{DISPCTRL_WR_DATA, 0x25},
	{DISPCTRL_WR_DATA, 0x34},
	{DISPCTRL_WR_DATA, 0x32},
	{DISPCTRL_WR_DATA, 0x3F},
	{DISPCTRL_WR_DATA, 0x32},
	{DISPCTRL_WR_DATA, 0x4A},
	{DISPCTRL_WR_DATA, 0x09},
	{DISPCTRL_WR_DATA, 0x10},
	{DISPCTRL_WR_DATA, 0x12},
	{DISPCTRL_WR_DATA, 0x14},
	{DISPCTRL_WR_DATA, 0x16},
	{DISPCTRL_WR_DATA, 0x14},
	{DISPCTRL_WR_DATA, 0x17},
	{DISPCTRL_WR_DATA, 0x15},
	{DISPCTRL_WR_DATA, 0x19},
	{DISPCTRL_WR_DATA, 0x01},
	{DISPCTRL_WR_DATA, 0x21},
	{DISPCTRL_WR_DATA, 0x25},
	{DISPCTRL_WR_DATA, 0x34},
	{DISPCTRL_WR_DATA, 0x32},
	{DISPCTRL_WR_DATA, 0x3F},
	{DISPCTRL_WR_DATA, 0x32},
	{DISPCTRL_WR_DATA, 0x4A},
	{DISPCTRL_WR_DATA, 0x09},
	{DISPCTRL_WR_DATA, 0x10},
	{DISPCTRL_WR_DATA, 0x12},
	{DISPCTRL_WR_DATA, 0x14},
	{DISPCTRL_WR_DATA, 0x16},
	{DISPCTRL_WR_DATA, 0x14},
	{DISPCTRL_WR_DATA, 0x17},
	{DISPCTRL_WR_DATA, 0x15},
	{DISPCTRL_WR_DATA, 0x19},

	{DISPCTRL_WR_CMND, 0xCC},
	{DISPCTRL_WR_DATA, 0x0A},

	{DISPCTRL_WR_CMND, 0xB6},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x8E},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x8E},

	/* flip */
	{DISPCTRL_WR_CMND, 0x36},
	{DISPCTRL_WR_DATA, 0x02},

	{DISPCTRL_WR_CMND, 0x11},
	{DISPCTRL_SLEEP_MS, 150},

	{DISPCTRL_WR_CMND, 0x29},
	{DISPCTRL_SLEEP_MS, 10},

	{DISPCTRL_LIST_END, 0}
};

void hx8379_tm_winset(char *msgData, DISPDRV_WIN_t *p_win)
{
	return;
}

__initdata struct lcd_config hx8379_tm_cfg = {
	.name = "HX8379_TM",
	.mode_supp = LCD_VID_ONLY,
	.phy_timing = &hx8379_tm_timing[0],
	.max_lanes = 2,
	.max_hs_bps = 500000000,
	.max_lp_bps = 5000000,
	.phys_width = 54,
	.phys_height = 95,
	.init_cmd_seq = NULL,
	.init_vid_seq = &hx8379_tm_init_panel_vid[0],
	.slp_in_seq = &hx8379_tm_slp_in[0],
	.slp_out_seq = &hx8379_tm_slp_out[0],
	.scrn_on_seq = &hx8379_tm_scrn_on[0],
	.scrn_off_seq = &hx8379_tm_scrn_off[0],
	.id_seq = NULL,
	.verify_id = false,
	.updt_win_fn = hx8379_tm_winset,
	.updt_win_seq_len = 0,
	.vid_cmnds = false,
	.vburst = true,
	.cont_clk = false,
	.hs = 20,
	.hbp = 46,
	.hfp = 21,
	.vs = 5,
	.vbp = 5,
	.vfp = 5,
};

#endif
