/*******************************************************************************
* Copyright 2011 Broadcom Corporation.	All rights reserved.
*
* @file drivers/video/broadcom/lcd/auo080100.h
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

#ifndef __AUO080100_H__
#define __AUO080100_H__

#include "mach/rdb/brcm_rdb_dsi1.h"

#include "display_drv.h"
#include "lcd.h"

#define AUO080100_CMD_SLPIN 0x10
#define AUO080100_CMD_SLPOUT 0x11
#define AUO080100_CMD_DISPOFF 0x28
#define AUO080100_CMD_DISPON 0x29

#define AUO080100_UPDT_WIN_SEQ_LEN 13 /* (6 + 6 + 1) */

__initdata struct DSI_COUNTER auo080100_timing[] = {
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

__initdata DISPCTRL_REC_T auo080100_scrn_on[] = {
	{DISPCTRL_WR_CMND, AUO080100_CMD_DISPON},
	{DISPCTRL_SLEEP_MS, 20},
	{DISPCTRL_LIST_END, 0}
};

__initdata DISPCTRL_REC_T auo080100_scrn_off[] = {
	{DISPCTRL_WR_CMND, AUO080100_CMD_DISPOFF},
	{DISPCTRL_LIST_END, 0}
};

__initdata DISPCTRL_REC_T auo080100_id[] = {
	{DISPCTRL_LIST_END, 0}
};

__initdata DISPCTRL_REC_T auo080100_slp_in[] = {
	{DISPCTRL_WR_CMND, AUO080100_CMD_DISPOFF},
	{DISPCTRL_WR_CMND, AUO080100_CMD_SLPIN},
	{DISPCTRL_SLEEP_MS, 120},
	{DISPCTRL_LIST_END, 0}
};

__initdata DISPCTRL_REC_T auo080100_slp_out[] = {
	{DISPCTRL_WR_CMND, AUO080100_CMD_SLPOUT},
	{DISPCTRL_SLEEP_MS, 10},
	{DISPCTRL_LIST_END, 0}
};

__initdata DISPCTRL_REC_T auo080100_init_panel_vid[] = {
	{DISPCTRL_WR_CMND, AUO080100_CMD_DISPON},
	{DISPCTRL_SLEEP_MS, 70},
};

void auo080100_winset(char *msgData, DISPDRV_WIN_t *p_win)
{
	return;
}

void auo080100_init_fn(void)
{
	uint32_t val;

	/* Continus clock, set after reset as spec */
	val = readl(KONA_DSI0_VA + DSI1_PHYC_OFFSET);
	val |= (0x1<<DSI1_PHYC_TX_HSCLK_CONT_SHIFT);
	writel(val, KONA_DSI0_VA + DSI1_PHYC_OFFSET);
}

__initdata struct lcd_config auo080100_cfg = {
	.name = "AUO080100",
	.mode_supp = LCD_VID_ONLY,
	.phy_timing = &auo080100_timing[0],
	.max_lanes = 4,
	.max_hs_bps = 600000000,
	.max_lp_bps = 5000000,
	.phys_width = 55,
	.phys_height = 99,
	.init_fn = auo080100_init_fn,
	.init_cmd_seq = NULL,
	.init_vid_seq = &auo080100_init_panel_vid[0],
	.slp_in_seq = &auo080100_slp_in[0],
	.slp_out_seq = &auo080100_slp_out[0],
	.scrn_on_seq = &auo080100_scrn_on[0],
	.scrn_off_seq = &auo080100_scrn_off[0],
	.id_seq = &auo080100_id[0],
	.verify_id = false,
	.updt_win_fn = auo080100_winset,
	.updt_win_seq_len = 0,
	.vid_cmnds = false,
	.vburst = false,
	.cont_clk = false,
	.sync_pulses = false,
	.hs = 140+60,
	.hbp = 140+60,
	.hfp = 140+62,
	.hbllp = 0,
	.vs = 50-1,
	.vbp = 30+1,
	.vfp = 36+1,
};

#endif

