/*******************************************************************************
* Copyright 2011 Broadcom Corporation.	All rights reserved.
*
* @file drivers/video/broadcom/lcd/hx8389b.h
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

#ifndef __HX8389B_H__
#define __HX8389B_H__

#include "display_drv.h"
#include "lcd.h"

#define HX8389B_CMD_SLPIN 0x10
#define HX8389B_CMD_SLPOUT 0x11
#define HX8389B_CMD_DISPOFF 0x28
#define HX8389B_CMD_DISPON 0x29
#define HX8389B_CMD_RDID1     0xDA
#define HX8389B_CMD_RDID2     0xDB
#define HX8389B_CMD_RDID3     0xDC

#define HX8389B_UPDT_WIN_SEQ_LEN 13 /* (6 + 6 + 1) */

__initdata struct DSI_COUNTER hx8389b_timing[] = {
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

__initdata DISPCTRL_REC_T hx8389b_scrn_on[] = {
	{DISPCTRL_WR_CMND, HX8389B_CMD_DISPON},
	{DISPCTRL_LIST_END, 0}
};

__initdata DISPCTRL_REC_T hx8389b_scrn_off[] = {
	{DISPCTRL_WR_CMND, HX8389B_CMD_DISPOFF},
	{DISPCTRL_LIST_END, 0}
};

__initdata DISPCTRL_REC_T hx8389b_id[] = {
	{DISPCTRL_WR_CMND, HX8389B_CMD_RDID1},
	{DISPCTRL_WR_DATA, 0x40},
	{DISPCTRL_WR_CMND, HX8389B_CMD_RDID2},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_CMND, HX8389B_CMD_RDID3},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_LIST_END, 0}
};

__initdata DISPCTRL_REC_T hx8389b_slp_in[] = {
	{DISPCTRL_WR_CMND, HX8389B_CMD_DISPOFF},
	{DISPCTRL_WR_CMND, HX8389B_CMD_SLPIN},
	{DISPCTRL_SLEEP_MS, 120},
	{DISPCTRL_LIST_END, 0}
};

__initdata DISPCTRL_REC_T hx8389b_slp_out[] = {
	{DISPCTRL_WR_CMND, HX8389B_CMD_SLPOUT},
	{DISPCTRL_SLEEP_MS, 120},
	{DISPCTRL_LIST_END, 0}
};

__initdata DISPCTRL_REC_T hx8389b_init_panel_vid[] = {
	{DISPCTRL_WR_CMND, 0xB9},
	{DISPCTRL_WR_DATA, 0xFF},
	{DISPCTRL_WR_DATA, 0x83},
	{DISPCTRL_WR_DATA, 0x89},

	{DISPCTRL_WR_CMND, 0xB1},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x05},
	{DISPCTRL_WR_DATA, 0xE0},
	{DISPCTRL_WR_DATA, 0x90},
	{DISPCTRL_WR_DATA, 0x10},
	{DISPCTRL_WR_DATA, 0x11},
	{DISPCTRL_WR_DATA, 0x6F},
	{DISPCTRL_WR_DATA, 0xEF},
	{DISPCTRL_WR_DATA, 0x25},
	{DISPCTRL_WR_DATA, 0x2D},
	{DISPCTRL_WR_DATA, 0x22},
	{DISPCTRL_WR_DATA, 0x22},
	{DISPCTRL_WR_DATA, 0x43},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x5A},
	{DISPCTRL_WR_DATA, 0xF1},
	{DISPCTRL_WR_DATA, 0x20},
	{DISPCTRL_WR_DATA, 0x00},

	{DISPCTRL_WR_CMND, 0xB2},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x78},
	{DISPCTRL_WR_DATA, 0x08},
	{DISPCTRL_WR_DATA, 0x03},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0xF0},

	{DISPCTRL_WR_CMND, 0xB4},
	{DISPCTRL_WR_DATA, 0x90},
	{DISPCTRL_WR_DATA, 0x08},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x32},
	{DISPCTRL_WR_DATA, 0x10},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x37},
	{DISPCTRL_WR_DATA, 0x0A},
	{DISPCTRL_WR_DATA, 0x40},
	{DISPCTRL_WR_DATA, 0x04},
	{DISPCTRL_WR_DATA, 0x37},
	{DISPCTRL_WR_DATA, 0x0A},
	{DISPCTRL_WR_DATA, 0x48},
	{DISPCTRL_WR_DATA, 0x17},
	{DISPCTRL_WR_DATA, 0x59},
	{DISPCTRL_WR_DATA, 0x5E},
	{DISPCTRL_WR_DATA, 0x0A},

	{DISPCTRL_WR_CMND, 0xD5},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x4C},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x01},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x60},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x99},
	{DISPCTRL_WR_DATA, 0x88},
	{DISPCTRL_WR_DATA, 0x99},
	{DISPCTRL_WR_DATA, 0x88},
	{DISPCTRL_WR_DATA, 0x88},
	{DISPCTRL_WR_DATA, 0x32},
	{DISPCTRL_WR_DATA, 0x88},
	{DISPCTRL_WR_DATA, 0x10},
	{DISPCTRL_WR_DATA, 0x88},
	{DISPCTRL_WR_DATA, 0x76},
	{DISPCTRL_WR_DATA, 0x88},
	{DISPCTRL_WR_DATA, 0x54},
	{DISPCTRL_WR_DATA, 0x10},
	{DISPCTRL_WR_DATA, 0x32},
	{DISPCTRL_WR_DATA, 0x88},
	{DISPCTRL_WR_DATA, 0x88},
	{DISPCTRL_WR_DATA, 0x88},
	{DISPCTRL_WR_DATA, 0x88},
	{DISPCTRL_WR_DATA, 0x88},

	{DISPCTRL_WR_CMND, 0xCB},
	{DISPCTRL_WR_DATA, 0x07},
	{DISPCTRL_WR_DATA, 0x07},

	{DISPCTRL_WR_CMND, 0xBB},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0xFF},
	{DISPCTRL_WR_DATA, 0x80},

	{DISPCTRL_WR_CMND, 0xB6},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0xA1},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0xA1},


	{DISPCTRL_WR_CMND, 0xC6},
	{DISPCTRL_WR_DATA, 0x08},


	{DISPCTRL_WR_CMND, 0xCC},
	{DISPCTRL_WR_DATA, 0x02},

	{DISPCTRL_WR_CMND, 0xDE},
	{DISPCTRL_WR_DATA, 0x05},
	{DISPCTRL_WR_DATA, 0x58},
	{DISPCTRL_WR_DATA, 0x10},

	{DISPCTRL_WR_CMND, 0xE0},
	{DISPCTRL_WR_DATA, 0x05},
	{DISPCTRL_WR_DATA, 0x0E},
	{DISPCTRL_WR_DATA, 0x12},
	{DISPCTRL_WR_DATA, 0x2D},
	{DISPCTRL_WR_DATA, 0x34},
	{DISPCTRL_WR_DATA, 0x3F},
	{DISPCTRL_WR_DATA, 0x18},
	{DISPCTRL_WR_DATA, 0x45},
	{DISPCTRL_WR_DATA, 0x09},
	{DISPCTRL_WR_DATA, 0x0A},
	{DISPCTRL_WR_DATA, 0x0F},
	{DISPCTRL_WR_DATA, 0x10},
	{DISPCTRL_WR_DATA, 0x13},
	{DISPCTRL_WR_DATA, 0x10},
	{DISPCTRL_WR_DATA, 0x15},
	{DISPCTRL_WR_DATA, 0x1A},
	{DISPCTRL_WR_DATA, 0x1C},
	{DISPCTRL_WR_DATA, 0x05},
	{DISPCTRL_WR_DATA, 0x0E},
	{DISPCTRL_WR_DATA, 0x12},
	{DISPCTRL_WR_DATA, 0x2D},
	{DISPCTRL_WR_DATA, 0x34},
	{DISPCTRL_WR_DATA, 0x3F},
	{DISPCTRL_WR_DATA, 0x18},
	{DISPCTRL_WR_DATA, 0x45},
	{DISPCTRL_WR_DATA, 0x09},
	{DISPCTRL_WR_DATA, 0x0A},
	{DISPCTRL_WR_DATA, 0x0F},
	{DISPCTRL_WR_DATA, 0x10},
	{DISPCTRL_WR_DATA, 0x13},
	{DISPCTRL_WR_DATA, 0x10},
	{DISPCTRL_WR_DATA, 0x15},
	{DISPCTRL_WR_DATA, 0x1A},
	{DISPCTRL_WR_DATA, 0x1C},

	{DISPCTRL_WR_CMND, 0xC1},
	{DISPCTRL_WR_DATA, 0x01},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x07},
	{DISPCTRL_WR_DATA, 0x1C},
	{DISPCTRL_WR_DATA, 0x2A},
	{DISPCTRL_WR_DATA, 0x34},
	{DISPCTRL_WR_DATA, 0x3F},
	{DISPCTRL_WR_DATA, 0x49},
	{DISPCTRL_WR_DATA, 0x52},
	{DISPCTRL_WR_DATA, 0x5B},
	{DISPCTRL_WR_DATA, 0x63},
	{DISPCTRL_WR_DATA, 0x6A},
	{DISPCTRL_WR_DATA, 0x71},
	{DISPCTRL_WR_DATA, 0x78},
	{DISPCTRL_WR_DATA, 0x7F},
	{DISPCTRL_WR_DATA, 0x85},
	{DISPCTRL_WR_DATA, 0x8B},
	{DISPCTRL_WR_DATA, 0x91},
	{DISPCTRL_WR_DATA, 0x97},
	{DISPCTRL_WR_DATA, 0x9D},
	{DISPCTRL_WR_DATA, 0xA3},
	{DISPCTRL_WR_DATA, 0xA8},
	{DISPCTRL_WR_DATA, 0xAF},
	{DISPCTRL_WR_DATA, 0xB5},
	{DISPCTRL_WR_DATA, 0xBE},
	{DISPCTRL_WR_DATA, 0xC6},
	{DISPCTRL_WR_DATA, 0xC9},
	{DISPCTRL_WR_DATA, 0xCD},
	{DISPCTRL_WR_DATA, 0xD7},
	{DISPCTRL_WR_DATA, 0xE1},
	{DISPCTRL_WR_DATA, 0xE6},
	{DISPCTRL_WR_DATA, 0xF1},
	{DISPCTRL_WR_DATA, 0xF7},
	{DISPCTRL_WR_DATA, 0xFF},
	{DISPCTRL_WR_DATA, 0x34},
	{DISPCTRL_WR_DATA, 0xCB},
	{DISPCTRL_WR_DATA, 0x4F},
	{DISPCTRL_WR_DATA, 0xD9},
	{DISPCTRL_WR_DATA, 0x54},
	{DISPCTRL_WR_DATA, 0xC8},
	{DISPCTRL_WR_DATA, 0x3C},
	{DISPCTRL_WR_DATA, 0x4A},
	{DISPCTRL_WR_DATA, 0xC0},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x07},
	{DISPCTRL_WR_DATA, 0x1C},
	{DISPCTRL_WR_DATA, 0x2A},
	{DISPCTRL_WR_DATA, 0x34},
	{DISPCTRL_WR_DATA, 0x3F},
	{DISPCTRL_WR_DATA, 0x49},
	{DISPCTRL_WR_DATA, 0x52},
	{DISPCTRL_WR_DATA, 0x5B},
	{DISPCTRL_WR_DATA, 0x63},
	{DISPCTRL_WR_DATA, 0x6A},
	{DISPCTRL_WR_DATA, 0x71},
	{DISPCTRL_WR_DATA, 0x78},
	{DISPCTRL_WR_DATA, 0x7F},
	{DISPCTRL_WR_DATA, 0x85},
	{DISPCTRL_WR_DATA, 0x8B},
	{DISPCTRL_WR_DATA, 0x91},
	{DISPCTRL_WR_DATA, 0x97},
	{DISPCTRL_WR_DATA, 0x9D},
	{DISPCTRL_WR_DATA, 0xA3},
	{DISPCTRL_WR_DATA, 0xA8},
	{DISPCTRL_WR_DATA, 0xAF},
	{DISPCTRL_WR_DATA, 0xB5},
	{DISPCTRL_WR_DATA, 0xBE},
	{DISPCTRL_WR_DATA, 0xC6},
	{DISPCTRL_WR_DATA, 0xC9},
	{DISPCTRL_WR_DATA, 0xCD},
	{DISPCTRL_WR_DATA, 0xD7},
	{DISPCTRL_WR_DATA, 0xE1},
	{DISPCTRL_WR_DATA, 0xE6},
	{DISPCTRL_WR_DATA, 0xF1},
	{DISPCTRL_WR_DATA, 0xF7},
	{DISPCTRL_WR_DATA, 0xFF},
	{DISPCTRL_WR_DATA, 0x34},
	{DISPCTRL_WR_DATA, 0xCB},
	{DISPCTRL_WR_DATA, 0x4F},
	{DISPCTRL_WR_DATA, 0xD9},
	{DISPCTRL_WR_DATA, 0x54},
	{DISPCTRL_WR_DATA, 0xC8},
	{DISPCTRL_WR_DATA, 0x3C},
	{DISPCTRL_WR_DATA, 0x4A},
	{DISPCTRL_WR_DATA, 0xC0},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x07},
	{DISPCTRL_WR_DATA, 0x1C},
	{DISPCTRL_WR_DATA, 0x2A},
	{DISPCTRL_WR_DATA, 0x34},
	{DISPCTRL_WR_DATA, 0x3F},
	{DISPCTRL_WR_DATA, 0x49},
	{DISPCTRL_WR_DATA, 0x52},
	{DISPCTRL_WR_DATA, 0x5B},
	{DISPCTRL_WR_DATA, 0x63},
	{DISPCTRL_WR_DATA, 0x6A},
	{DISPCTRL_WR_DATA, 0x71},
	{DISPCTRL_WR_DATA, 0x78},
	{DISPCTRL_WR_DATA, 0x7F},
	{DISPCTRL_WR_DATA, 0x85},
	{DISPCTRL_WR_DATA, 0x8B},
	{DISPCTRL_WR_DATA, 0x91},
	{DISPCTRL_WR_DATA, 0x97},
	{DISPCTRL_WR_DATA, 0x9D},
	{DISPCTRL_WR_DATA, 0xA3},
	{DISPCTRL_WR_DATA, 0xA8},
	{DISPCTRL_WR_DATA, 0xAF},
	{DISPCTRL_WR_DATA, 0xB5},
	{DISPCTRL_WR_DATA, 0xBE},
	{DISPCTRL_WR_DATA, 0xC6},
	{DISPCTRL_WR_DATA, 0xC9},
	{DISPCTRL_WR_DATA, 0xCD},
	{DISPCTRL_WR_DATA, 0xD7},
	{DISPCTRL_WR_DATA, 0xE1},
	{DISPCTRL_WR_DATA, 0xE6},
	{DISPCTRL_WR_DATA, 0xF1},
	{DISPCTRL_WR_DATA, 0xF7},
	{DISPCTRL_WR_DATA, 0xFF},
	{DISPCTRL_WR_DATA, 0x34},
	{DISPCTRL_WR_DATA, 0xCB},
	{DISPCTRL_WR_DATA, 0x4F},
	{DISPCTRL_WR_DATA, 0xD9},
	{DISPCTRL_WR_DATA, 0x54},
	{DISPCTRL_WR_DATA, 0xC8},
	{DISPCTRL_WR_DATA, 0x3C},
	{DISPCTRL_WR_DATA, 0x4A},
	{DISPCTRL_WR_DATA, 0xC0},

	{DISPCTRL_WR_CMND, 0x11},
	{DISPCTRL_SLEEP_MS, 150},

	{DISPCTRL_WR_CMND, 0x29},
	{DISPCTRL_SLEEP_MS, 10},
	{DISPCTRL_LIST_END, 0}
};

void hx8389b_winset(char *msgData, DISPDRV_WIN_t *p_win)
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

	if (i != HX8389B_UPDT_WIN_SEQ_LEN)
		pr_err("hx8389b_winset msg len incorrect!\n");

}

__initdata struct lcd_config hx8389b_cfg = {
	.name = "HX8389B",
	.mode_supp = LCD_VID_ONLY,
	.phy_timing = &hx8389b_timing[0],
	.max_lanes = 2,
	.max_hs_bps = 500000000,
	.max_lp_bps = 5000000,
	.phys_width = 54,
	.phys_height = 95,
	.init_cmd_seq = NULL,
	.init_vid_seq = &hx8389b_init_panel_vid[0],
	.slp_in_seq = &hx8389b_slp_in[0],
	.slp_out_seq = &hx8389b_slp_out[0],
	.scrn_on_seq = &hx8389b_scrn_on[0],
	.scrn_off_seq = &hx8389b_scrn_off[0],
	.id_seq = &hx8389b_id[0],
	.verify_id = false,
	.updt_win_fn = hx8389b_winset,
	.updt_win_seq_len = 0,
	.vid_cmnds = true,
	.vburst = true,
	.cont_clk = false,
	.hs = 8,
	.hbp = 60,
	.hfp = 22,
	.vs = 2,
	.vbp = 10,
	.vfp = 2,
};

#endif
