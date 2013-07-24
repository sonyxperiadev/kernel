/*******************************************************************************
* Copyright 2011 Broadcom Corporation.	All rights reserved.
*
* @file drivers/video/broadcom/lcd/ILI9806c.h
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

#ifndef __ILI9806C_H__
#define __ILI9806C_H__

#include "display_drv.h"
#include "lcd.h"

#define ILI9806C_CMD_SLPIN		0x10
#define ILI9806C_CMD_SLPOUT		0x11
#define ILI9806C_CMD_DISPOFF		0x28
#define ILI9806C_CMD_DISPON		0x29

#define ILI9806C_UPDT_WIN_SEQ_LEN 13

__initdata struct DSI_COUNTER ili9806c_timing[] = {
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

__initdata DISPCTRL_REC_T ili9806c_scrn_on[] = {
	{DISPCTRL_WR_CMND, ILI9806C_CMD_DISPON},
	{DISPCTRL_LIST_END, 0}
};

__initdata DISPCTRL_REC_T ili9806c_scrn_off[] = {
	{DISPCTRL_WR_CMND, ILI9806C_CMD_DISPOFF},
	{DISPCTRL_LIST_END, 0}
};

__initdata DISPCTRL_REC_T ili9806c_slp_in[] = {
	{DISPCTRL_WR_CMND, ILI9806C_CMD_DISPOFF},
	{DISPCTRL_WR_CMND, ILI9806C_CMD_SLPIN},
	{DISPCTRL_SLEEP_MS, 120},
	{DISPCTRL_LIST_END, 0}
};

__initdata DISPCTRL_REC_T ili9806c_slp_out[] = {
	{DISPCTRL_WR_CMND, ILI9806C_CMD_SLPOUT},
	{DISPCTRL_SLEEP_MS, 120},
	{DISPCTRL_LIST_END, 0}
};

__initdata DISPCTRL_REC_T ili9806c_init_panel_vid[] = {
	{DISPCTRL_WR_CMND, 0xFF},
	{DISPCTRL_WR_DATA, 0xFF},
	{DISPCTRL_WR_DATA, 0x98},
	{DISPCTRL_WR_DATA, 0x16},

	{DISPCTRL_WR_CMND, 0xBA},
	{DISPCTRL_WR_DATA, 0x60},

	{DISPCTRL_WR_CMND, 0xB0},
	{DISPCTRL_WR_DATA, 0x01},

	{DISPCTRL_WR_CMND, 0xBC},
	{DISPCTRL_WR_DATA, 0x03},
	{DISPCTRL_WR_DATA, 0x0D},
	{DISPCTRL_WR_DATA, 0x63},
	{DISPCTRL_WR_DATA, 0x31},
	{DISPCTRL_WR_DATA, 0x01},
	{DISPCTRL_WR_DATA, 0x01},
	{DISPCTRL_WR_DATA, 0x1B},
	{DISPCTRL_WR_DATA, 0x10},
	{DISPCTRL_WR_DATA, 0x37},
	{DISPCTRL_WR_DATA, 0x13},
	{DISPCTRL_WR_DATA, 0x10},
	{DISPCTRL_WR_DATA, 0x37},
	{DISPCTRL_WR_DATA, 0x32},
	{DISPCTRL_WR_DATA, 0x32},
	{DISPCTRL_WR_DATA, 0x19},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0xFF},
	{DISPCTRL_WR_DATA, 0xE2},
	{DISPCTRL_WR_DATA, 0x01},
	{DISPCTRL_WR_DATA, 0x10},
	{DISPCTRL_WR_DATA, 0x10},
	{DISPCTRL_WR_DATA, 0x43},
	{DISPCTRL_WR_DATA, 0x0B},

	{DISPCTRL_WR_CMND, 0xBD},
	{DISPCTRL_WR_DATA, 0x01},
	{DISPCTRL_WR_DATA, 0x23},
	{DISPCTRL_WR_DATA, 0x45},
	{DISPCTRL_WR_DATA, 0x67},
	{DISPCTRL_WR_DATA, 0x01},
	{DISPCTRL_WR_DATA, 0x23},
	{DISPCTRL_WR_DATA, 0x45},
	{DISPCTRL_WR_DATA, 0x67},

	{DISPCTRL_WR_CMND, 0xBE},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x22},
	{DISPCTRL_WR_DATA, 0x22},
	{DISPCTRL_WR_DATA, 0x22},
	{DISPCTRL_WR_DATA, 0x97},
	{DISPCTRL_WR_DATA, 0x86},
	{DISPCTRL_WR_DATA, 0xCA},
	{DISPCTRL_WR_DATA, 0xDB},
	{DISPCTRL_WR_DATA, 0xAC},
	{DISPCTRL_WR_DATA, 0xBD},
	{DISPCTRL_WR_DATA, 0x68},
	{DISPCTRL_WR_DATA, 0x79},
	{DISPCTRL_WR_DATA, 0x22},
	{DISPCTRL_WR_DATA, 0x22},
	{DISPCTRL_WR_DATA, 0x22},
	{DISPCTRL_WR_DATA, 0x22},
	{DISPCTRL_WR_DATA, 0x22},

	{DISPCTRL_WR_CMND, 0xED},
	{DISPCTRL_WR_DATA, 0x7F},
	{DISPCTRL_WR_DATA, 0x0F},

	{DISPCTRL_WR_CMND, 0xF3},
	{DISPCTRL_WR_DATA, 0x70},

	{DISPCTRL_WR_CMND, 0xB4},
	{DISPCTRL_WR_DATA, 0x02},

	{DISPCTRL_WR_CMND, 0xC0},
	{DISPCTRL_WR_DATA, 0x9F},
	{DISPCTRL_WR_DATA, 0x0B},
	{DISPCTRL_WR_DATA, 0x08},

	{DISPCTRL_WR_CMND, 0xC1},
	{DISPCTRL_WR_DATA, 0x17},
	{DISPCTRL_WR_DATA, 0x98},
	{DISPCTRL_WR_DATA, 0x92},
	{DISPCTRL_WR_DATA, 0x20},

	{DISPCTRL_WR_CMND, 0xD8},
	{DISPCTRL_WR_DATA, 0x50},

	{DISPCTRL_WR_CMND, 0xFC},
	{DISPCTRL_WR_DATA, 0x08},

	{DISPCTRL_WR_CMND, 0xE0},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x0C},
	{DISPCTRL_WR_DATA, 0x23},
	{DISPCTRL_WR_DATA, 0x13},
	{DISPCTRL_WR_DATA, 0x15},
	{DISPCTRL_WR_DATA, 0x1F},
	{DISPCTRL_WR_DATA, 0xCB},
	{DISPCTRL_WR_DATA, 0x08},
	{DISPCTRL_WR_DATA, 0x04},
	{DISPCTRL_WR_DATA, 0x08},
	{DISPCTRL_WR_DATA, 0x03},
	{DISPCTRL_WR_DATA, 0x0F},
	{DISPCTRL_WR_DATA, 0x13},
	{DISPCTRL_WR_DATA, 0x26},
	{DISPCTRL_WR_DATA, 0x24},
	{DISPCTRL_WR_DATA, 0x00},

	{DISPCTRL_WR_CMND, 0xE1},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x09},
	{DISPCTRL_WR_DATA, 0x15},
	{DISPCTRL_WR_DATA, 0x11},
	{DISPCTRL_WR_DATA, 0x13},
	{DISPCTRL_WR_DATA, 0x17},
	{DISPCTRL_WR_DATA, 0x79},
	{DISPCTRL_WR_DATA, 0x08},
	{DISPCTRL_WR_DATA, 0x03},
	{DISPCTRL_WR_DATA, 0x08},
	{DISPCTRL_WR_DATA, 0x06},
	{DISPCTRL_WR_DATA, 0x0B},
	{DISPCTRL_WR_DATA, 0x08},
	{DISPCTRL_WR_DATA, 0x27},
	{DISPCTRL_WR_DATA, 0x21},
	{DISPCTRL_WR_DATA, 0x00},

	{DISPCTRL_WR_CMND, 0xD5},
	{DISPCTRL_WR_DATA, 0x0D},
	{DISPCTRL_WR_DATA, 0x0A},
	{DISPCTRL_WR_DATA, 0x0A},
	{DISPCTRL_WR_DATA, 0x0A},
	{DISPCTRL_WR_DATA, 0xCB},
	{DISPCTRL_WR_DATA, 0xA5},
	{DISPCTRL_WR_DATA, 0x01},
	{DISPCTRL_WR_DATA, 0x04},

	{DISPCTRL_WR_CMND, 0xF7},
	{DISPCTRL_WR_DATA, 0x8A},

	{DISPCTRL_WR_CMND, 0xC7},
	{DISPCTRL_WR_DATA, 0x87},

	{DISPCTRL_WR_CMND, 0x36},
	{DISPCTRL_WR_DATA, 0x00},

	{DISPCTRL_WR_CMND, 0x11},
	{DISPCTRL_SLEEP_MS, 200},
	{DISPCTRL_WR_CMND, 0x29},
	{DISPCTRL_SLEEP_MS, 30},
	{DISPCTRL_LIST_END, 0}
};

void ILI9806C_winset(char *msgData, DISPDRV_WIN_t *p_win)
{
	return;
}

__initdata struct lcd_config ili9806c_cfg = {
	.name = "ILI9806C",
	.mode_supp = LCD_VID_ONLY,
	.phy_timing = &ili9806c_timing[0],
	.max_lanes = 2,
	.max_hs_bps = 500000000,
	.max_lp_bps = 5000000,
	.phys_width = 55,
	.phys_height = 99,
	.init_cmd_seq = NULL,
	.init_vid_seq = &ili9806c_init_panel_vid[0],
	.slp_in_seq = &ili9806c_slp_in[0],
	.slp_out_seq = &ili9806c_slp_out[0],
	.scrn_on_seq = &ili9806c_scrn_on[0],
	.scrn_off_seq = &ili9806c_scrn_off[0],
	.id_seq = NULL,
	.verify_id = false,
	.updt_win_fn = ILI9806C_winset,
	.updt_win_seq_len = 0,
	.vid_cmnds = false,
	.vburst = true,
	.cont_clk = false,
	.hs = 10,
	.hbp = 50,
	.hfp = 200,
	.vs = 2,
	.vbp = 18,
	.vfp = 20,
};

#endif
