/*******************************************************************************
* Copyright 2011 Broadcom Corporation.	All rights reserved.
*
* @file drivers/video/broadcom/lcd/nt35516.h
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

#ifndef __NT35516_H__
#define __NT35516_H__

#include "display_drv.h"
#include "lcd.h"

#define NT35516_CMD_NOP		0x00
#define NT35516_CMD_SWRESET	0x01
#define NT35516_CMD_RDDID	0x04
#define NT35516_CMD_RDNUMED	0x05
#define NT35516_CMD_RDDPM	0x0A
#define NT35516_CMD_RDDMADCTL	0x0B
#define NT35516_CMD_RDDCOLMOD	0x0C
#define NT35516_CMD_RDDIM	0x0D
#define NT35516_CMD_RDDSM	0x0E
#define NT35516_CMD_RDDSDR	0x0F
#define NT35516_CMD_SLPIN	0x10
#define NT35516_CMD_SLPOUT	0x11
#define NT35516_CMD_PTLON	0x12
#define NT35516_CMD_NORON	0x13
#define NT35516_CMD_INVOFF	0x20
#define NT35516_CMD_INVON	0x21
#define NT35516_CMD_ALLPOFF	0x22
#define NT35516_CMD_ALLPON	0x23
#define NT35516_CMD_GAMSET	0x26
#define NT35516_CMD_DISPOFF	0x28
#define NT35516_CMD_DISPON	0x29
#define NT35516_CMD_CASET	0x2A
#define NT35516_CMD_RASET	0x2B
#define NT35516_CMD_RAMWR	0x2C
#define NT35516_CMD_RAMRD	0x2E
#define NT35516_CMD_PTLAR	0x30
#define NT35516_CMD_TEOFF	0x34
#define NT35516_CMD_TEON	0x35
#define NT35516_CMD_MADCTL	0x36
#define NT35516_CMD_IDMOFF	0x38
#define NT35516_CMD_IDMON	0x39
#define NT35516_CMD_COLMOD	0x3A
#define NT35516_CMD_RAMWRC	0x3C
#define NT35516_CMD_RAMRDC	0x3E
#define NT35516_CMD_STESL	0x44
#define NT35516_CMD_GSL		0x45
#define NT35516_CMD_DSTBON	0x4F
#define NT35516_CMD_WRPFD	0x50
#define NT35516_CMD_WRGAMMSET	0x58
#define NT35516_CMD_RDFCS	0xAA
#define NT35516_CMD_RDCCS	0xAF
#define NT35516_CMD_DOPCTR	0xB1
#define NT35516_CMD_RDID1	0xDA
#define NT35516_CMD_RDID2	0xDB
#define NT35516_CMD_RDID3	0xDC
#define NT35516_CMD_MAUCCTR	0xF0


#define TE_SCAN_LINE 960

#define NT35516_UPDT_WIN_SEQ_LEN 13 /* (6 + 6 + 1) */


__initdata struct DSI_COUNTER nt35516_timing[] = {
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

__initdata DISPCTRL_REC_T nt35516_scrn_on[] = {
	{DISPCTRL_WR_CMND, NT35516_CMD_DISPON},
	{DISPCTRL_LIST_END, 0}
};

__initdata DISPCTRL_REC_T nt35516_scrn_off[] = {
	{DISPCTRL_WR_CMND, NT35516_CMD_DISPOFF},
	{DISPCTRL_LIST_END, 0}
};

__initdata DISPCTRL_REC_T nt35516_id[] = {
	{DISPCTRL_WR_CMND, NT35516_CMD_RDID1},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_CMND, NT35516_CMD_RDID2},
	{DISPCTRL_WR_DATA, 0x80},
	{DISPCTRL_WR_CMND, NT35516_CMD_RDID3},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_LIST_END, 0}
};

__initdata DISPCTRL_REC_T nt35516_slp_in[] = {
	{DISPCTRL_WR_CMND, NT35516_CMD_DISPOFF},
	{DISPCTRL_WR_CMND, NT35516_CMD_SLPIN},
	{DISPCTRL_SLEEP_MS, 120},
	{DISPCTRL_WR_CMND, NT35516_CMD_DSTBON},
	{DISPCTRL_WR_DATA, 1},
	{DISPCTRL_LIST_END, 0}
};

__initdata DISPCTRL_REC_T nt35516_slp_out[] = {
	{DISPCTRL_WR_CMND, NT35516_CMD_SLPOUT},
	{DISPCTRL_SLEEP_MS, 120},
	{DISPCTRL_LIST_END, 0}
};

__initdata DISPCTRL_REC_T nt35516_init_panel_cmd[] = {
	{DISPCTRL_WR_CMND, NT35516_CMD_SLPOUT},
	{DISPCTRL_SLEEP_MS, 120},
	{DISPCTRL_WR_CMND, NT35516_CMD_COLMOD},
	{DISPCTRL_WR_DATA, 0x77},
	{DISPCTRL_WR_CMND, NT35516_CMD_TEON},
	{DISPCTRL_WR_DATA, 0x0},
	{DISPCTRL_WR_CMND, NT35516_CMD_MAUCCTR},
	{DISPCTRL_WR_DATA, 0x55},
	{DISPCTRL_WR_DATA, 0xAA},
	{DISPCTRL_WR_DATA, 0x52},
	{DISPCTRL_WR_DATA, 0x08},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_CMND, NT35516_CMD_DOPCTR},
	{DISPCTRL_WR_DATA, 0x7C},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_CMND, NT35516_CMD_STESL},
	{DISPCTRL_WR_DATA, (TE_SCAN_LINE & 0xFF00) >> 8},
	{DISPCTRL_WR_DATA, (TE_SCAN_LINE & 0xFF)},
	{DISPCTRL_LIST_END, 0}
};

__initdata DISPCTRL_REC_T nt35516_init_panel_vid[] = {
	{DISPCTRL_WR_CMND, NT35516_CMD_SLPOUT},
	{DISPCTRL_SLEEP_MS, 120},
	{DISPCTRL_WR_CMND, NT35516_CMD_MAUCCTR},
	{DISPCTRL_WR_DATA, 0x55},
	{DISPCTRL_WR_DATA, 0xAA},
	{DISPCTRL_WR_DATA, 0x52},
	{DISPCTRL_WR_DATA, 0x08},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_CMND, NT35516_CMD_DOPCTR},
	{DISPCTRL_WR_DATA, 0x7C},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_LIST_END, 0}
};

void nt35516_winset(char *msgData, DISPDRV_WIN_t *p_win)
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

	if (i != NT35516_UPDT_WIN_SEQ_LEN)
		pr_err("nt35516_winset msg len incorrect!\n");
}

__initdata struct lcd_config nt35516_cfg = {
	.name = "NT35516",
	.mode_supp = LCD_CMD_VID_BOTH,
	.phy_timing = &nt35516_timing[0],
	.max_lanes = 3,
	.max_hs_bps = 550000000,
	.max_lp_bps = 5000000,
	.phys_width = 54,
	.phys_height = 95,
	.init_cmd_seq = &nt35516_init_panel_cmd[0],
	.init_vid_seq = &nt35516_init_panel_vid[0],
	.slp_in_seq = &nt35516_slp_in[0],
	.slp_out_seq = &nt35516_slp_out[0],
	.scrn_on_seq = &nt35516_scrn_on[0],
	.scrn_off_seq = &nt35516_scrn_off[0],
	.id_seq = &nt35516_id[0],
	.verify_id = true,
	.updt_win_fn = nt35516_winset,
	.updt_win_seq_len = NT35516_UPDT_WIN_SEQ_LEN,
	.vid_cmnds = true,
	.vburst = true,
	.cont_clk = false,
	.hs = 46,
	.hbp = 46,
	.hfp = 46,
	.vs = 20,
	.vbp = 20,
	.vfp = 20,
};

#endif
