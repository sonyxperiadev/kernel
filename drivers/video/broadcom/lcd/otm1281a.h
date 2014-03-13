/*******************************************************************************
* Copyright 2011 Broadcom Corporation.	All rights reserved.
*
* @file drivers/video/broadcom/lcd/otm1281a.h
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

#ifndef __OTM1281A_H__
#define __OTM1281A_H__

#include "display_drv.h"
#include "lcd.h"

#define OTM1281A_CMD_DISPOFF 0x28
#define OTM1281A_CMD_DISPON 0x29
#define OTM1281A_CMD_SLPIN 0x10
#define OTM1281A_CMD_SLPOUT 0x11
#define OTM1281A_CMD_RDID1     0xDA
#define OTM1281A_CMD_RDID2     0xDB
#define OTM1281A_CMD_RDID3     0xDC


#define OTM1281A_UPDT_WIN_SEQ_LEN 13 /* (6 + 6 + 1) */

__initdata struct DSI_COUNTER otm1281a_timing[] = {
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

__initdata DISPCTRL_REC_T otm1281a_scrn_on[] = {
	{DISPCTRL_WR_CMND, OTM1281A_CMD_DISPON},
	{DISPCTRL_LIST_END, 0}
};

__initdata DISPCTRL_REC_T otm1281a_scrn_off[] = {
	{DISPCTRL_WR_CMND, OTM1281A_CMD_DISPOFF},
	{DISPCTRL_LIST_END, 0}
};

__initdata DISPCTRL_REC_T otm1281a_id[] = {
	{DISPCTRL_WR_CMND, OTM1281A_CMD_RDID1},
	{DISPCTRL_WR_DATA, 0x40},
	{DISPCTRL_WR_CMND, OTM1281A_CMD_RDID2},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_CMND, OTM1281A_CMD_RDID3},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_LIST_END, 0}
};

__initdata DISPCTRL_REC_T otm1281a_slp_in[] = {
	{DISPCTRL_WR_CMND, OTM1281A_CMD_SLPIN},
	{DISPCTRL_LIST_END, 0}
};

__initdata DISPCTRL_REC_T otm1281a_slp_out[] = {
	{DISPCTRL_WR_CMND, OTM1281A_CMD_SLPOUT},
	{DISPCTRL_LIST_END, 0}
};
#ifdef CONFIG_MACH_HAWAII_GARNET_EDN000
__initdata DISPCTRL_REC_T otm1281a_init_panel_cmd[] = {
	{DISPCTRL_WR_CMND, 0xff},
	{DISPCTRL_WR_DATA, 0x12},
	{DISPCTRL_WR_DATA, 0x80},
	{DISPCTRL_WR_DATA, 0x01},

	{DISPCTRL_WR_CMND, 0x00},
	{DISPCTRL_WR_DATA, 0x80},
	{DISPCTRL_WR_CMND, 0xff},
	{DISPCTRL_WR_DATA, 0x12},
	{DISPCTRL_WR_DATA, 0x80},

	{DISPCTRL_WR_CMND, 0x00},
	{DISPCTRL_WR_DATA, 0x84},
	{DISPCTRL_WR_CMND, 0xff},
	{DISPCTRL_WR_DATA, 0x10},
	{DISPCTRL_WR_DATA, 0x02},

	{DISPCTRL_WR_CMND, 0x00},
	{DISPCTRL_WR_DATA, 0x90},
	{DISPCTRL_WR_CMND, 0xc5,},
	{DISPCTRL_WR_DATA, 0x20},
	{DISPCTRL_WR_DATA, 0x6F},
	{DISPCTRL_WR_DATA, 0x02},
	{DISPCTRL_WR_DATA, 0x88},
	{DISPCTRL_WR_DATA, 0x1D},
	{DISPCTRL_WR_DATA, 0x15},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x08},

	{DISPCTRL_WR_CMND, 0x00},
	{DISPCTRL_WR_DATA, 0xa0},
	{DISPCTRL_WR_CMND, 0xc5},
	{DISPCTRL_WR_DATA, 0x20},
	{DISPCTRL_WR_DATA, 0x6F},
	{DISPCTRL_WR_DATA, 0x02},
	{DISPCTRL_WR_DATA, 0x88},
	{DISPCTRL_WR_DATA, 0x1D},
	{DISPCTRL_WR_DATA, 0x15},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x08},

	{DISPCTRL_WR_CMND, 0x00},
	{DISPCTRL_WR_DATA, 0x80},
	{DISPCTRL_WR_CMND, 0xc5},
	{DISPCTRL_WR_DATA, 0x20},
	{DISPCTRL_WR_DATA, 0x01},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0xb0},
	{DISPCTRL_WR_DATA, 0xb0},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x08},
	{DISPCTRL_WR_DATA, 0x00},


	{DISPCTRL_WR_CMND, 0x00},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_CMND, 0xd8},
	{DISPCTRL_WR_DATA, 0x58},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x58},
	{DISPCTRL_WR_DATA, 0x00},

	{DISPCTRL_WR_CMND, 0x00},
	{DISPCTRL_WR_DATA, 0xb8},
	{DISPCTRL_WR_CMND, 0xf5},
	{DISPCTRL_WR_DATA, 0x0c},
	{DISPCTRL_WR_DATA, 0x12},

	{DISPCTRL_WR_CMND, 0x00},
	{DISPCTRL_WR_DATA, 0x80},
	{DISPCTRL_WR_CMND, 0xff},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},

	{DISPCTRL_WR_CMND, 0x00},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_CMND, 0xff},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},

	{DISPCTRL_WR_CMND, 0x2A},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x02},

	{DISPCTRL_WR_CMND, 0x2B},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x04},

	{DISPCTRL_WR_CMND, 0x36},
	{DISPCTRL_WR_DATA, 0xD0},

	{DISPCTRL_WR_CMND, 0x11},
	{DISPCTRL_SLEEP_MS, 200},

	{DISPCTRL_LIST_END, 0}
};
#else
__initdata DISPCTRL_REC_T otm1281a_init_panel_cmd[] = {
	{DISPCTRL_WR_CMND, 0xff},
	{DISPCTRL_WR_DATA, 0x12},
	{DISPCTRL_WR_DATA, 0x80},
	{DISPCTRL_WR_DATA, 0x01},

	{DISPCTRL_WR_CMND, 0x00},
	{DISPCTRL_WR_DATA, 0x80},

	{DISPCTRL_WR_CMND, 0xff},
	{DISPCTRL_WR_DATA, 0x12},
	{DISPCTRL_WR_DATA, 0x80},

	{DISPCTRL_WR_CMND, 0x00},
	{DISPCTRL_WR_DATA, 0x90},

	{DISPCTRL_WR_CMND, 0xc5,},
	{DISPCTRL_WR_DATA, 0x20},
	{DISPCTRL_WR_DATA, 0x6F},
	{DISPCTRL_WR_DATA, 0x02},
	{DISPCTRL_WR_DATA, 0x88},
	{DISPCTRL_WR_DATA, 0x1D},
	{DISPCTRL_WR_DATA, 0x15},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x08},

	{DISPCTRL_WR_CMND, 0x00},
	{DISPCTRL_WR_DATA, 0xA0},

	{DISPCTRL_WR_CMND, 0xc5,},
	{DISPCTRL_WR_DATA, 0x20},
	{DISPCTRL_WR_DATA, 0x6F},
	{DISPCTRL_WR_DATA, 0x02},
	{DISPCTRL_WR_DATA, 0x88},
	{DISPCTRL_WR_DATA, 0x1D},
	{DISPCTRL_WR_DATA, 0x15},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x08},

	{DISPCTRL_WR_CMND, 0x00},
	{DISPCTRL_WR_DATA, 0x80},

	{DISPCTRL_WR_CMND, 0xc5},
	{DISPCTRL_WR_DATA, 0x20},
	{DISPCTRL_WR_DATA, 0x01},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0xB0},
	{DISPCTRL_WR_DATA, 0xB0},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x04},
	{DISPCTRL_WR_DATA, 0x00},

	{DISPCTRL_WR_CMND, 0x00},/* OSCILLATOR 70HZ */
	{DISPCTRL_WR_DATA, 0x82},

	{DISPCTRL_WR_CMND, 0xC1},/*OSCILLATOR 70HZ */
	{DISPCTRL_WR_DATA, 0x09},

	{DISPCTRL_WR_CMND, 0x00},
	{DISPCTRL_WR_DATA, 0x00},

	{DISPCTRL_WR_CMND, 0xd8},
	{DISPCTRL_WR_DATA, 0x58},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x58},
	{DISPCTRL_WR_DATA, 0x00},

	{DISPCTRL_WR_CMND, 0x00},
	{DISPCTRL_WR_DATA, 0xb8},

	{DISPCTRL_WR_CMND, 0xf5},
	{DISPCTRL_WR_DATA, 0x0c},
	{DISPCTRL_WR_DATA, 0x12},

	{DISPCTRL_WR_CMND, 0x00},
	{DISPCTRL_WR_DATA, 0x80},

	{DISPCTRL_WR_CMND, 0xff},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},

	{DISPCTRL_WR_CMND, 0x00},
	{DISPCTRL_WR_DATA, 0x00},

	{DISPCTRL_WR_CMND, 0xFF},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},

	{DISPCTRL_WR_CMND, 0x11},
	{DISPCTRL_SLEEP_MS, 200},
	{DISPCTRL_LIST_END, 0}
};
#endif

void otm1281a_winset(char *msgData, DISPDRV_WIN_t *p_win)
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

	if (i != OTM1281A_UPDT_WIN_SEQ_LEN)
		pr_err("otm1281a_winset msg len incorrect!\n");
}

int otm1281a_esd_check_fn(void)
{
	int ret = -EIO;
	UInt8 id[4] = {0};

	/* Read display power mode */
	panel_read(0x0A, id, 1);
	if (0x9C == (id[0]&0xFC)) {
		ret = 0;
	} else {
		pr_err("otm1281a_esd_check_fn fail id 0x%x\n", id[0]);
		ret = -EIO;
	}

	return ret;
}

__initdata struct lcd_config otm1281a_cfg = {
	.name = "OTM1281A",
	.mode_supp = LCD_CMD_VID_BOTH,
	.phy_timing = &otm1281a_timing[0],
	.max_lanes = 4,
	.max_hs_bps = 550000000,
	.max_lp_bps = 9000000,
	.phys_width = 56,
	.phys_height = 99,
	.esd_check_fn = otm1281a_esd_check_fn,
	.init_cmd_seq = &otm1281a_init_panel_cmd[0],
	.init_vid_seq = &otm1281a_init_panel_cmd[0],
	.slp_in_seq = &otm1281a_slp_in[0],
	.slp_out_seq = &otm1281a_slp_out[0],
	.scrn_on_seq = &otm1281a_scrn_on[0],
	.scrn_off_seq = &otm1281a_scrn_off[0],
	.id_seq = &otm1281a_id[0],
	.verify_id = false,
	.updt_win_fn = otm1281a_winset,
	.updt_win_seq_len = OTM1281A_UPDT_WIN_SEQ_LEN,
	.vid_cmnds = false,
	.vburst = true,
	.cont_clk = false,
	.hs = 30,
	.hbp = 55,
	.hfp = 30,
	.vs = 20,
	.vbp = 20,
	.vfp = 20,
};

#endif
