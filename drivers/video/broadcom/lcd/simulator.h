/*******************************************************************************
* Copyright 2011 Broadcom Corporation.	All rights reserved.
*
* @file drivers/video/broadcom/lcd/simulator.h
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

#ifndef __SIMULATOR_H__
#define __SIMULATOR_H__

#include "display_drv.h"
#include "lcd.h"

#define SIMULATOR_CMD_NOP		0x00
#define SIMULATOR_CMD_SWRESET	0x01
#define SIMULATOR_CMD_RDDID	0x04
#define SIMULATOR_CMD_RDNUMED	0x05
#define SIMULATOR_CMD_RDDPM	0x0A
#define SIMULATOR_CMD_RDDMADCTL	0x0B
#define SIMULATOR_CMD_RDDCOLMOD	0x0C
#define SIMULATOR_CMD_RDDIM	0x0D
#define SIMULATOR_CMD_RDDSM	0x0E
#define SIMULATOR_CMD_RDDSDR	0x0F
#define SIMULATOR_CMD_SLPIN	0x10
#define SIMULATOR_CMD_SLPOUT	0x11
#define SIMULATOR_CMD_PTLON	0x12
#define SIMULATOR_CMD_NORON	0x13
#define SIMULATOR_CMD_INVOFF	0x20
#define SIMULATOR_CMD_INVON	0x21
#define SIMULATOR_CMD_ALLPOFF	0x22
#define SIMULATOR_CMD_ALLPON	0x23
#define SIMULATOR_CMD_GAMSET	0x26
#define SIMULATOR_CMD_DISPOFF	0x28
#define SIMULATOR_CMD_DISPON	0x29
#define SIMULATOR_CMD_CASET	0x2A
#define SIMULATOR_CMD_RASET	0x2B
#define SIMULATOR_CMD_RAMWR	0x2C
#define SIMULATOR_CMD_RAMRD	0x2E
#define SIMULATOR_CMD_PTLAR	0x30
#define SIMULATOR_CMD_TEOFF	0x34
#define SIMULATOR_CMD_TEON	0x35
#define SIMULATOR_CMD_MADCTL	0x36
#define SIMULATOR_CMD_IDMOFF	0x38
#define SIMULATOR_CMD_IDMON	0x39
#define SIMULATOR_CMD_COLMOD	0x3A
#define SIMULATOR_CMD_RAMWRC	0x3C
#define SIMULATOR_CMD_RAMRDC	0x3E
#define SIMULATOR_CMD_STESL	0x44
#define SIMULATOR_CMD_GSL		0x45
#define SIMULATOR_CMD_DSTBON	0x4F
#define SIMULATOR_CMD_WRPFD	0x50
#define SIMULATOR_CMD_WRGAMMSET	0x58
#define SIMULATOR_CMD_RDFCS	0xAA
#define SIMULATOR_CMD_RDCCS	0xAF
#define SIMULATOR_CMD_DOPCTR	0xB1
#define SIMULATOR_CMD_RDID1	0xDA
#define SIMULATOR_CMD_RDID2	0xDB
#define SIMULATOR_CMD_RDID3	0xDC
#define SIMULATOR_CMD_MAUCCTR	0xF0


#define TE_SCAN_LINE 960

#define SIMULATOR_UPDT_WIN_SEQ_LEN 13 /* (6 + 6 + 1) */


__initdata struct DSI_COUNTER simulator_timing[] = {
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
		0, 70, 0, 0, 0, 75, 0, 0, 0x000001FF, 0, 0, 0},
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

__initdata DISPCTRL_REC_T simulator_scrn_on[] = {
	{DISPCTRL_LIST_END, 0}
};

__initdata DISPCTRL_REC_T simulator_scrn_off[] = {
	{DISPCTRL_LIST_END, 0}
};

__initdata DISPCTRL_REC_T simulator_slp_in[] = {
	{DISPCTRL_LIST_END, 0}
};

__initdata DISPCTRL_REC_T simulator_slp_out[] = {
	{DISPCTRL_LIST_END, 0}
};

__initdata DISPCTRL_REC_T simulator_init_panel_cmd[] = {
	{DISPCTRL_WR_CMND, SIMULATOR_CMD_COLMOD},
	{DISPCTRL_WR_DATA, 0x77},
	{DISPCTRL_WR_CMND, SIMULATOR_CMD_DISPON},
	{DISPCTRL_LIST_END, 0}
};

__initdata DISPCTRL_REC_T simulator_init_panel_vid[] = {
	{DISPCTRL_WR_CMND, SIMULATOR_CMD_COLMOD},
	{DISPCTRL_WR_DATA, 0x77},
	{DISPCTRL_WR_CMND, SIMULATOR_CMD_DISPON},
	{DISPCTRL_LIST_END, 0}
};

void simulator_winset(char *msgData, DISPDRV_WIN_t *p_win)
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

	if (i != SIMULATOR_UPDT_WIN_SEQ_LEN)
		pr_err("simulator_winset msg len incorrect!\n");
}

__initdata struct lcd_config simulator_cfg = {
	.name = "SIMULATOR",
	.mode_supp = LCD_CMD_VID_BOTH,
	.phy_timing = &simulator_timing[0],
	.max_lanes = 3,
	.max_hs_bps = 1000000000,
	.max_lp_bps = 5000000,
	.phys_width = 54,
	.phys_height = 95,
	.init_cmd_seq = &simulator_init_panel_cmd[0],
	.init_vid_seq = &simulator_init_panel_vid[0],
	.slp_in_seq = &simulator_slp_in[0],
	.slp_out_seq = &simulator_slp_out[0],
	.scrn_on_seq = &simulator_scrn_on[0],
	.scrn_off_seq = &simulator_scrn_off[0],
	.updt_win_fn = simulator_winset,
	.updt_win_seq_len = SIMULATOR_UPDT_WIN_SEQ_LEN,
	.vid_cmnds = true,
	.vburst = true,
	.hs = 46,
	.hbp = 46,
	.hfp = 46,
	.vs = 20,
	.vbp = 20,
	.vfp = 20,
};

#endif
