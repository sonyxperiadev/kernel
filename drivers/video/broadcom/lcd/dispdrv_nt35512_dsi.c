/*****************************************************************************
* Copyright 2011 Broadcom Corporation.	All rights reserved.
*
* @file drivers/video/broadcom/dispdrv_nt35512_dsi.c
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
#ifdef __KERNEL__

#include <linux/string.h>
#include <linux/broadcom/mobcom_types.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <plat/osabstract/osinterrupt.h>
#include <plat/osabstract/ostask.h>
#include <plat/csl/csl_dma_vc4lite.h>
#include <plat/dma_drv.h>
#include <plat/pi_mgr.h>
#include <video/kona_fb_boot.h>	   /* dispdrv init interface */
#include "display_drv.h"	   /* dispdrv interface	*/
#include <plat/csl/csl_lcd.h>	   /* dispdrv CSL Common Interface */
#include <plat/csl/csl_dsi.h>	   /* DSI CSL API */
#include "dispdrv_mipi_dcs.h"	   /* MIPI DCS */
#include "dispdrv_mipi_dsi.h"	   /* MIPI DSI */
#include "dispdrv_common.h"	   /* dispdrv common */
#include "lcd_clock.h"
#include <plat/csl/csl_tectl_vc4lite.h>

#else
#include <stdio.h>
#include <string.h>
#include "dbg.h"
#include "mobcom_types.h"
#include "chip_version.h"
#include "gpio.h"
#include "platform_config.h"
#include "irqctrl.h"
#include "osinterrupt.h"
#include "ostask.h"
#include "dbg.h"
#include "logapi.h"
#include "dma_drv.h"
#include "display_drv.h"	   /* dispdrv interface	*/
#include "csl_lcd.h"		   /* dispdrv CSL Common Interface */
#include "csl_dsi.h"		   /* DSI CSL API */
#include "dispdrv_mipi_dcs.h"	   /* MIPI DCS */
#include "dispdrv_mipi_dsi.h"	   /* MIPI DSI */
#endif

#include "lcd_NT35512.h"

#define NT35512_VC		(0)
#define NT35512_CMND_IS_LP TRUE /* display init comm LP or HS mode */

#if 0
#define NT35512_LOG(id, fmt, args...) printk(KERN_ERR fmt, ##args)
#else
#define NT35512_LOG(id, fmt, args...)
#endif

typedef struct {
	CSL_LCD_HANDLE clientH;
	CSL_LCD_HANDLE dsiCmVcHandle;
	DISP_DRV_STATE drvState;
	DISP_PWR_STATE pwrState;
	UInt32 busNo;
	UInt32 teIn;
	UInt32 teOut;
	Boolean isTE;
	DISPDRV_WIN_t win_dim;
	DISPDRV_WIN_t win_cur;
	struct pi_mgr_dfs_node	*dfs_node;
	Boolean boot_mode;
	UInt32 rst_bridge_pwr_down;
	UInt32 rst_bridge_reset;
	UInt32 rst_panel_reset;
	CSL_DSI_CM_VC_t	*cmnd_mode;
	CSL_DSI_CFG_t	*dsi_cfg;
	DISPDRV_INFO_T	*disp_info;
} NT35512_PANEL_t;

/* DRV INTERFACE FUNCTIONs */
static Int32 NT35512_Init(struct dispdrv_init_parms *parms,
	void **handle);

static Int32 NT35512_Exit(DISPDRV_HANDLE_T drvH);

static Int32 NT35512_Open(DISPDRV_HANDLE_T drvH);

static Int32 NT35512_Close(DISPDRV_HANDLE_T drvH);

static void NT35512_ExecCmndList(
	DISPDRV_HANDLE_T	drvH,
	pDISPCTRL_REC_T	cmnd_lst);

static Int32 NT35512_GetDispDrvFeatures(
	DISPDRV_HANDLE_T	drvH,
	const char	**driver_name,
	UInt32	*version_major,
	UInt32	*version_minor,
	DISPDRV_SUPPORT_FEATURES_T	*flags);

const DISPDRV_INFO_T *NT35512_GetDispDrvData(
	DISPDRV_HANDLE_T	drvH);

static Int32 NT35512_Start(
	DISPDRV_HANDLE_T	drvH,
	struct pi_mgr_dfs_node	*dfs_node);

static Int32 NT35512_Stop(
	DISPDRV_HANDLE_T	drvH,
	struct pi_mgr_dfs_node	*dfs_node);

static Int32 NT35512_PowerControl(
	DISPDRV_HANDLE_T	drvH,
	DISPLAY_POWER_STATE_T	state);

static Int32 NT35512_Atomic_Update(
	DISPDRV_HANDLE_T	drvH,
	void	*buff,
	DISPDRV_WIN_t	*p_win);

static Int32 NT35512_Update(
	DISPDRV_HANDLE_T	drvH,
	void	*buff,
	DISPDRV_WIN_t	*p_win,
	DISPDRV_CB_T	apiCb);

static Int32 NT35512_WinReset(DISPDRV_HANDLE_T drvH);

static DISPDRV_T NT35512_Drv = {
	&NT35512_Init,		/* init	*/
	&NT35512_Exit,		/* exit	*/
	&NT35512_GetDispDrvFeatures,/* info	*/
	&NT35512_Open,		/* open	*/
	&NT35512_Close,		/* close */
	&NT35512_GetDispDrvData,	/* get_info */
	&NT35512_Start,		/* start */
	&NT35512_Stop,		/* stop	*/
	&NT35512_PowerControl,	/* power_control */
	&NT35512_Atomic_Update,	/* update_no_os	*/
	&NT35512_Update,		/* update */
	NULL,				/* set_brightness */
	&NT35512_WinReset,		/* reset_win */
};

static struct DSI_COUNTER phy_timing[] = {
	/* LP Data Symbol Rate Calc - MUST BE FIRST RECORD */
	{"ESC2LP_RATIO", DSI_C_TIME_ESC2LPDT, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0x0000003F, 1, 1, 0},
	/* SPEC:  min =  100[us] + 0[UI] */
	/* SET:   min = 1000[us] + 0[UI] <= */
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
	/* SET:   min = 0[ns] + 12[UI] < = */
	{"HS_CLK_PRE", DSI_C_TIME_HS, 0,
	0, 0, 12, 0, 0, 0, 0, 0, 0x000001FF, 0, 0, 0},
	/* SPEC:  min = 38[ns] + 0[UI]   max= 95[ns] + 0[UI] */
	/* SET:   min = 48[ns] + 0[UI]   max= 95[ns] + 0[UI] <= */
	{"HS_CLK_PREPARE", DSI_C_TIME_HS, DSI_C_HAS_MAX,
	0, 48, 0, 0, 0, 95, 0, 0, 0x000001FF, 0, 0, 0},
	/* SPEC:  min = 262[ns] + 0[UI] */
	/* SET:   min = 262[ns] + 0[UI]<= */
	{"HS_CLK_ZERO", DSI_C_TIME_HS, 0,
	0, 262, 0, 0, 0, 0, 0, 0, 0x000001FF, 0, 0, 0},
	/* SPEC:  min =  60[ns] + 52[UI] */
	/* SET:   min =  70[ns] + 52[UI] <= */
	{"HS_CLK_POST", DSI_C_TIME_HS, 0,
	0, 70, 52, 0, 0, 0, 0, 0, 0x000001FF, 0, 0, 0},
	/* SPEC:  min =  60[ns] + 0[UI] */
	/* SET:   min =  70[ns] + 0[UI]<= */
	{"HS_CLK_TRAIL", DSI_C_TIME_HS, 0,
	0, 70, 0, 0, 0, 0, 0, 0, 0x000001FF, 0, 0, 0},
	/* SPEC:  min =  50[ns] + 0[UI] */
	/* SET:   min =  60[ns] + 0[UI]<= */
	{"HS_LPX", DSI_C_TIME_HS, 0,
	0, 60, 0, 0, 0, 75, 0, 0, 0x000001FF, 0, 0, 0},
	/* SPEC:  min = 40[ns] + 4[UI]      max= 85[ns] + 6[UI] */
	/* SET:   min = 50[ns] + 4[UI]      max= 85[ns] + 6[UI] <= */
	{"HS_PRE", DSI_C_TIME_HS, DSI_C_HAS_MAX,
	0, 50, 4, 0, 0, 85, 6, 0, 0x000001FF, 0, 0, 0},
	/* SPEC:  min = 105[ns] + 6[UI] */
	/* SET:   min = 105[ns] + 6[UI]<= */
	{"HS_ZERO", DSI_C_TIME_HS, 0,
	0, 105, 6, 0, 0, 0, 0, 0, 0x000001FF, 0, 0, 0},
	/* SPEC:  min = max(0[ns]+32[UI],60[ns]+16[UI])  n=4 */
	/* SET:   min = max(0[ns]+32[UI],60[ns]+16[UI])  n=4 */
	{"HS_TRAIL", DSI_C_TIME_HS, DSI_C_MIN_MAX_OF_2,
	0, 0, 32, 60, 16, 0, 0, 0, 0x000001FF, 0, 0, 0},
	/* SPEC:  min = 100[ns] + 0[UI] */
	/* SET:   min = 110[ns] + 0[UI] <= */
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

/* DISP DRV API	- Display Info */
static DISPDRV_INFO_T NT35512_Info = {
	DISPLAY_TYPE_LCD_STD,		/* type	*/
	480,				/* height */
	854,				/* width */
	DISPDRV_FB_FORMAT_RGB888_U,	/* input_format	*/
	DISPLAY_BUS_DSI,		/* bus_type */
	4,				/* Bpp */
	54,				/* phys_width */
	95,				/* phys_height */
	32150,			/* pixclock */
};

/* DSI Command Mode VC Configuration */
CSL_DSI_CM_VC_t NT35512_VCCmCfg = {
	NT35512_VC,			/* VC */
	DSI_DT_LG_DCS_WR,		/* dsiCmnd */
	MIPI_DCS_WRITE_MEMORY_START,	/* dcsCmndStart	*/
	MIPI_DCS_WRITE_MEMORY_CONTINUE,	/* dcsCmndContinue */
	FALSE,				/* isLP	*/
	LCD_IF_CM_I_RGB888U,		/* cm_in */
	LCD_IF_CM_O_RGB888,		/* cm_out */
	/* TE configuration */
	{
		DSI_TE_NONE, /* DSI Te Input Type */
	},
};

/* DSI BUS CONFIGURATION */
CSL_DSI_CFG_t NT35512_dsiCfg = {
	0,		/* bus */
	2,		/* dlCount */
	phy_timing,	/* DSI_DPHY_SPEC_T */
	/* ESC CLK Config */
	{500, 5},	/* escClk   500|312 500[MHz], DIV by 5 = 100[MHz] */

	/* HS CLK Config, RHEA VCO range 600-2400 */
	{1000, 2},	/* hsBitClk PLL 1000[MHz], DIV by 3 = 333[Mbps]	*/
	/* LP Speed */
	5,		/* lpBitRate_Mbps, Max 10[Mbps]	*/
	FALSE,		/* enaContClock	*/
	TRUE,		/* enaRxCrc */
	TRUE,		/* enaRxEcc */
	TRUE,		/* enaHsTxEotPkt */
	FALSE,		/* enaLpTxEotPkt */
	FALSE,		/* enaLpRxEotPkt */
#ifdef CONFIG_VIDEO_MODE
	0,		/* dispEngine */
	0,		/* pixTxporter */
#else
	1,		/* dispEngine */
	0,		/* pixTxporter */
#endif
};

static NT35512_PANEL_t panel[1];

/*###########################################################################*/

static void NT35512_panel_on(NT35512_PANEL_t *pPanel)
{
	DISPCTRL_REC_T cmd_list[] = {
		{DISPCTRL_WR_CMND, NT35512_CMD_DISPON, 0},
		{DISPCTRL_LIST_END, 0, 0},
	};

	NT35512_ExecCmndList(pPanel, cmd_list);
	NT35512_LOG(LCD_DBG_ERR_ID, "[DISPDRV] %s\n", __func__);
	return;
}

static void NT35512_panel_off(NT35512_PANEL_t *pPanel)
{
	DISPCTRL_REC_T cmd_list[] = {
		{DISPCTRL_WR_CMND, NT35512_CMD_DISPOFF, 0},
		{DISPCTRL_LIST_END, 0, 0},
	};

	NT35512_ExecCmndList(pPanel, cmd_list);
	NT35512_LOG(LCD_DBG_ERR_ID, "[DISPDRV] %s\n", __func__);
	return;
}

static void NT35512_panel_sleep_in(NT35512_PANEL_t *pPanel)
{
	DISPCTRL_REC_T cmd_list[] = {
		{DISPCTRL_WR_CMND, NT35512_CMD_SLPIN, 0},
		{DISPCTRL_SLEEP_MS, 0, 120},
		{DISPCTRL_LIST_END, 0, 0},
	};

	NT35512_ExecCmndList(pPanel, cmd_list);
	NT35512_LOG(LCD_DBG_ERR_ID, "[DISPDRV] %s\n", __func__);
	return;
}

static void NT35512_panel_sleep_out(NT35512_PANEL_t *pPanel)
{
	DISPCTRL_REC_T cmd_list[] = {
		{DISPCTRL_WR_CMND, NT35512_CMD_SLPOUT, 0},
		{DISPCTRL_SLEEP_MS, 0, 120},
		{DISPCTRL_LIST_END, 0, 0},
	};

	NT35512_ExecCmndList(pPanel, cmd_list);
	NT35512_LOG(LCD_DBG_ERR_ID, "[DISPDRV] %s\n", __func__);
	return;
}

static void NT35512_panel_dstb(NT35512_PANEL_t *pPanel)
{
	DISPCTRL_REC_T cmd_list[] = {
		{DISPCTRL_WR_CMND_DATA, NT35512_CMD_DSTBON, 1},
		{DISPCTRL_LIST_END, 0, 0},
	};

	NT35512_ExecCmndList(pPanel, cmd_list);
	NT35512_LOG(LCD_DBG_ERR_ID, "[DISPDRV] %s\n", __func__);
	return;
}

/* Template for Video mode commands like Turn On Peripheral, Shutdown etc. */
static int NT35512_panel_shut_down(NT35512_PANEL_t	*pPanel)
{
	CSL_DSI_CMND_t msg;
	int res = 0;
	UInt8 msg_buff[2];

	msg.dsiCmnd =	DSI_DT_SH_SHUT_DOWN;
	msg.msg =	&msg_buff[0];
	msg.msgLen =	0;
	msg.vc =	NT35512_VC;
	msg.isLP	=	NT35512_CMND_IS_LP;
	msg.isLong = FALSE;
	msg.endWithBta = FALSE;
	msg.reply = NULL;

	res = CSL_DSI_SendPacket(pPanel->clientH, &msg, FALSE);
	NT35512_LOG(LCD_DBG_ERR_ID, "[DISPDRV] %s: DT[0x%02lX] SIZE[%lu]\n",
		__func__, msg.dsiCmnd, msg.msgLen);

	return res;
}

static int NT35512_panel_turn_on(NT35512_PANEL_t *pPanel)
{
	CSL_DSI_CMND_t msg;
	int res = 0;
	UInt8 msg_buff[2];

	msg.dsiCmnd = DSI_DT_SH_TURN_ON;
	msg.msg = &msg_buff[0];
	msg.msgLen = 0;
	msg.vc = NT35512_VC;
	msg.isLP = NT35512_CMND_IS_LP;
	msg.isLong = FALSE;
	msg.endWithBta = FALSE;
	msg.reply = NULL;

	res = CSL_DSI_SendPacket(pPanel->clientH, &msg, FALSE);
	NT35512_LOG(LCD_DBG_ERR_ID, "[DISPDRV] %s: DT[0x%02lX] SIZE[%lu]\n",
		__func__, msg.dsiCmnd, msg.msgLen);

	return res;
}

static void NT35512_panel_init(NT35512_PANEL_t *pPanel)
{
	NT35512_ExecCmndList(pPanel, NT35512_dsi_init);
	NT35512_LOG(LCD_DBG_INIT_ID, "[DISPDRV] %s: ", __func__);

	return;
}
/*
 *
 * Function Name: NT35512_TeOn
 *
 * Description:   Configure TE Input Pin & Route it to DSI Controller Input
 *
 */
static int NT35512_TeOn(NT35512_PANEL_t	*pPanel)
{
	Int32 res	= 0;

	TECTL_CFG_t teCfg;

	teCfg.te_mode	  = TE_VC4L_MODE_VSYNC;
	teCfg.sync_pol	  = TE_VC4L_ACT_POL_LO;
	teCfg.vsync_width = 0;
	teCfg.hsync_line  = 0;

	res = CSL_TECTL_VC4L_OpenInput(pPanel->teIn, pPanel->teOut, &teCfg);

	return res;
}

/*
 *
 *   Function Name: NT35512_TeOff
 *
 *   Description:   'Release' TE Input Pin Used
 *
 */
static int NT35512_TeOff(NT35512_PANEL_t *pPanel)
{
	Int32  res = 0;

	res = CSL_TECTL_VC4L_CloseInput(pPanel->teIn);

	return res;
}

/*
 *
 * Function Name: NT35512_WinSet
 *
 * Description:   Set Window
 *
 */
Int32 NT35512_WinSet(DISPDRV_HANDLE_T drvH,
	Boolean update,
	DISPDRV_WIN_t	*p_win)
{
	NT35512_PANEL_t *pPanel = (NT35512_PANEL_t *) drvH;
	CSL_DSI_CMND_t msg;
	UInt8 msgData[7];
	CSL_DSI_REPLY_t rxMsg;
	UInt8 rx_buff[8];

	NT35512_LOG(LCD_DBG_ERR_ID, "NT35512_WinSet\n");

#ifdef CONFIG_VIDEO_MODE
	return 0;
#endif

	if ((pPanel->win_cur.l != p_win->l) ||
		(pPanel->win_cur.r != p_win->r) ||
		(pPanel->win_cur.t != p_win->t) ||
		(pPanel->win_cur.b != p_win->b)) {
		pPanel->win_cur	= *p_win;
		if (update) {
			rxMsg.pReadReply = rx_buff;
			msg.dsiCmnd = DSI_DT_LG_DCS_WR;
			msg.msg = &msgData[0];
			msg.msgLen = 5;
			msg.vc = NT35512_VC;
			msg.isLP = NT35512_CMND_IS_LP;
			msg.isLong = TRUE;
			msg.endWithBta = FALSE;
			msg.reply = &rxMsg;

			msgData[0] = MIPI_DCS_SET_COLUMN_ADDRESS;
			msgData[1] = (p_win->l & 0xFF00) >> 8;
			msgData[2] = (p_win->l & 0x00FF);
			msgData[3] = (p_win->r & 0xFF00) >> 8;
			msgData[4] = (p_win->r & 0x00FF);

			CSL_DSI_SendPacket(pPanel->clientH, &msg, FALSE);

			msgData[0] = MIPI_DCS_SET_PAGE_ADDRESS;
			msgData[1] = (p_win->t & 0xFF00) >> 8;
			msgData[2] = (p_win->t & 0x00FF);
			msgData[3] = (p_win->b & 0xFF00) >> 8;
			msgData[4] = (p_win->b & 0x00FF);

			CSL_DSI_SendPacket(pPanel->clientH, &msg, FALSE);
			mdelay(5);
			return 0;
		}
	}
	return 0;
}

/*
 *
 *   Function Name: OTM1281A_WinReset
 *
 *   Description:   Reset windowing to full screen size.
 *		    Typically, only used in boot code environment
 */
Int32 NT35512_WinReset(DISPDRV_HANDLE_T drvH)
{
	Int32 res;
	NT35512_PANEL_t *pPanel = (NT35512_PANEL_t *) drvH;

	printk("%s\n", __func__);
	res = NT35512_WinSet(drvH, TRUE, &pPanel->win_dim);

	return res;
}

/*
 *
 *   Function Name: NT35512_reset
 *
 *   Description:   (De)Assert display reset
 *
 */
static void NT35512_reset(DISPDRV_HANDLE_T drvH, Boolean on)
{
	NT35512_PANEL_t *pPanel = (NT35512_PANEL_t *) drvH;
	int reset_active = 0;

	gpio_request(pPanel->rst_panel_reset, "LCD_RST1");

	if (!on) {
		NT35512_LOG(LCD_DBG_ERR_ID, "Resetting the panel\n");
		gpio_direction_output(pPanel->rst_panel_reset, !reset_active);
		udelay(5);
		gpio_set_value_cansleep(pPanel->rst_panel_reset, reset_active);
		udelay(20);
		gpio_set_value_cansleep(pPanel->rst_panel_reset, !reset_active);
		msleep(20);
	} else {
		NT35512_LOG(LCD_DBG_ERR_ID,
			"Powering off the panel, gpio = %d\n",
			pPanel->rst_panel_reset);
		gpio_set_value_cansleep(pPanel->rst_panel_reset, reset_active);
		udelay(15);
		gpio_set_value_cansleep(pPanel->rst_panel_reset, reset_active);
		msleep(20);
	}
}

/*
 *
 *   Function Name: LCD_DRV_NT35512_GetDrvInfo
 *
 *   Description:   Get Driver Funtion Table
 *
 */
DISPDRV_T *DISP_DRV_NT35512_GetFuncTable(void)
{
	return &NT35512_Drv;
}

/*
 *
 *   Function Name: NT35512_Init
 *
 *   Description:   Setup / Verify display driver init interface
 *
 */
Int32 NT35512_Init(struct dispdrv_init_parms *parms,
	void **handle)
{
	Int32 res = 0;
	NT35512_PANEL_t	*pPanel;

	pPanel = &panel[0];

	if (pPanel->drvState == DRV_STATE_OFF)	{
		NT35512_LOG(LCD_DBG_INIT_ID, "[DISPDRV] %s: Bus %d\n",
			__func__, parms->w0.bits.bus_type);
		NT35512_LOG(LCD_DBG_INIT_ID, "[DISPDRV] %s: BootMode %d\n",
			__func__, parms->w0.bits.boot_mode);
		NT35512_LOG(LCD_DBG_INIT_ID, "[DISPDRV] %s: BusNo %d\n",
			__func__, parms->w0.bits.bus_no);
		NT35512_LOG(LCD_DBG_INIT_ID, "[DISPDRV] %s: col_mode_i %d\n",
			__func__, parms->w0.bits.col_mode_i);
		NT35512_LOG(LCD_DBG_INIT_ID, "[DISPDRV] %s: col_mode_o %d\n",
			__func__, parms->w0.bits.col_mode_o);
		NT35512_LOG(LCD_DBG_INIT_ID, "[DISPDRV] %s: te_input %d\n",
			__func__, parms->w0.bits.te_input);
		NT35512_LOG(LCD_DBG_INIT_ID, "[DISPDRV] %s: API Rev %d\n",
			__func__, parms->w1.bits.api_rev);
		NT35512_LOG(LCD_DBG_INIT_ID, "[DISPDRV] %s: Rst 0 %d\n",
			__func__, parms->w1.bits.lcd_rst0);
		NT35512_LOG(LCD_DBG_INIT_ID, "[DISPDRV] %s: Rst 1 %d\n",
			__func__, parms->w1.bits.lcd_rst1);
		NT35512_LOG(LCD_DBG_INIT_ID, "[DISPDRV] %s: Rst 2 %d\n",
			__func__, parms->w1.bits.lcd_rst2);

		if ((u8)parms->w1.bits.api_rev != KONA_LCD_BOOT_API_REV) {
			NT35512_LOG(LCD_DBG_ERR_ID, "[DISPDRV] %s", __func__);
			NT35512_LOG(LCD_DBG_ERR_ID,
				":Boot Init Rev Mismatch(%d.%d vs %d.%d)\n",
				(parms->w1.bits.api_rev	& 0xF0)	>> 8,
				(parms->w1.bits.api_rev	& 0x0F),
				(KONA_LCD_BOOT_API_REV	& 0xF0)	>> 8,
				(KONA_LCD_BOOT_API_REV	& 0x0F));

			return -1;
		}

		pPanel->boot_mode = parms->w0.bits.boot_mode;
		pPanel->cmnd_mode = &NT35512_VCCmCfg;
		pPanel->dsi_cfg	= &NT35512_dsiCfg;
		pPanel->disp_info = &NT35512_Info;
		pPanel->busNo =	dispdrv2busNo(parms->w0.bits.bus_no);

		/* check for valid DSI bus no */
		if (pPanel->busNo & 0xFFFFFFFE) {
			NT35512_LOG(LCD_DBG_ERR_ID, "[DISPDRV] %s", __func__);
			NT35512_LOG(LCD_DBG_ERR_ID,
				":Invalid DSI BusNo[%lu]\n", pPanel->busNo);
			return -1;
		}

		pPanel->cmnd_mode->cm_in =
			dispdrv2cmIn(parms->w0.bits.col_mode_i);
		pPanel->cmnd_mode->cm_out =
			dispdrv2cmOut(parms->w0.bits.col_mode_o);

		/* as of now, only 565 */
		switch (pPanel->cmnd_mode->cm_in) {
		case LCD_IF_CM_I_RGB565P:
			pPanel->disp_info->input_format
				= DISPDRV_FB_FORMAT_RGB565;
			pPanel->disp_info->Bpp = 2;
			break;
		case LCD_IF_CM_I_RGB888U:
			pPanel->disp_info->input_format
				= DISPDRV_FB_FORMAT_RGB888_U;
			pPanel->disp_info->Bpp = 4;
			break;
		default:
			NT35512_LOG(LCD_DBG_ERR_ID, "[DISPDRV] %s", __func__);
			NT35512_LOG(LCD_DBG_ERR_ID,
				":Unsupported Color Mode\n");
			return -1;
		}

		/* get reset pin */
		pPanel->rst_panel_reset = parms->w1.bits.lcd_rst0;
		pPanel->isTE = pPanel->cmnd_mode->teCfg.teInType != DSI_TE_NONE;
		/* get TE pin configuration */
		pPanel->teIn = dispdrv2busTE(parms->w0.bits.te_input);
		pPanel->teOut = pPanel->busNo
			? TE_VC4L_OUT_DSI1_TE0 : TE_VC4L_OUT_DSI0_TE0;
		pPanel->drvState = DRV_STATE_INIT;
		pPanel->pwrState = STATE_PWR_OFF;
		*handle	= (DISPDRV_HANDLE_T)pPanel;

		NT35512_LOG(LCD_DBG_INIT_ID, "[DISPDRV] %s: OK\n", __func__);
	} else {
		NT35512_LOG(LCD_DBG_ERR_ID, "[DISPDRV] %s: Not in OFF state\n",
			__func__);
		res = -1;
	}

	return res;
}

/*
 *
 *   Function Name: NT35512_Exit
 *
 *   Description:
 *
 */
Int32 NT35512_Exit(DISPDRV_HANDLE_T drvH)
{
	NT35512_PANEL_t *pPanel;

	pPanel = (NT35512_PANEL_t *) drvH;
	pPanel->drvState = DRV_STATE_OFF;

	return 0;
}

/*
 *
 *  Function Name: NT35512_Open
 *
 *  Description:   disp bus ON
 *
 */
Int32 NT35512_Open(DISPDRV_HANDLE_T drvH)
{
	Int32 res = 0;
	NT35512_PANEL_t	*pPanel;

	pPanel = (NT35512_PANEL_t *) drvH;

	NT35512_LOG(LCD_DBG_ERR_ID, "NT35512_Open\n");
	if (pPanel->drvState !=	DRV_STATE_INIT)	{
		NT35512_LOG(LCD_DBG_ERR_ID,
			"[DISPDRV] %s: ERROR State != Init\n", __func__);
		return -1;
	}

	if (brcm_enable_dsi_pll_clocks(pPanel->busNo,
		pPanel->dsi_cfg->hsBitClk.clkIn_MHz * 1000000,
		pPanel->dsi_cfg->hsBitClk.clkInDiv,
		pPanel->dsi_cfg->escClk.clkIn_MHz   * 1000000
		/pPanel->dsi_cfg->escClk.clkInDiv)) {

		NT35512_LOG(LCD_DBG_ERR_ID,
			"[DISPDRV] %s:ERROR to enable clock\n", __func__);
	}

	if (pPanel->isTE && NT35512_TeOn(pPanel) == -1) {
		NT35512_LOG(LCD_DBG_ERR_ID,
			"[DISPDRV] %s:Failed Config TE Input\n", __func__);
		goto err_te_on;
	}

	if (CSL_DSI_Init(pPanel->dsi_cfg) != CSL_LCD_OK) {
		NT35512_LOG(LCD_DBG_ERR_ID,
			"[DISPDRV] %s:DSI CSL Init Failed\n", __func__);
		goto err_dsi_init;
	}

	if (CSL_DSI_OpenClient(pPanel->busNo, &pPanel->clientH) != CSL_LCD_OK) {
		NT35512_LOG(LCD_DBG_ERR_ID,
			"[DISPDRV] %s:CSL_DSI_OpenClient Failed\n", __func__);
		goto err_dsi_open_cl;
	}

	if (CSL_DSI_OpenCmVc(pPanel->clientH,
		pPanel->cmnd_mode, &pPanel->dsiCmVcHandle) != CSL_LCD_OK) {
		NT35512_LOG(LCD_DBG_ERR_ID,
			"[DISPDRV] %s:CSL_DSI_OpenCmVc ""Failed\n", __func__);
		goto err_dsi_open_cm;
	}

	if (!pPanel->dsi_cfg->dispEngine && pPanel->dsi_cfg->pixTxporter) {
		NT35512_LOG(LCD_DBG_ERR_ID,
			"Cannot transfer pixels via MMDMA\n");
		NT35512_LOG(LCD_DBG_ERR_ID,
			"Display Engine 0. Default Display Engine 1\n");
		pPanel->dsi_cfg->dispEngine = 1;
	}

	if (pPanel->dsi_cfg->dispEngine && pPanel->dsi_cfg->pixTxporter) {
		if (csl_dma_vc4lite_init() != DMA_VC4LITE_STATUS_SUCCESS) {
			NT35512_LOG(LCD_DBG_ERR_ID, "[DISPDRV] %s", __func__);
			NT35512_LOG(LCD_DBG_ERR_ID,
				":csl_dma_vc4lite_init Failed\n");
			goto err_dma_init;
		}
	}
	NT35512_reset(drvH, FALSE);

	pPanel->win_dim.l = 0;
	pPanel->win_dim.r = pPanel->disp_info->width-1;
	pPanel->win_dim.t = 0;
	pPanel->win_dim.b = pPanel->disp_info->height-1;
	pPanel->win_dim.w = pPanel->disp_info->width;
	pPanel->win_dim.h = pPanel->disp_info->height;
	pPanel->drvState = DRV_STATE_OPEN;

	NT35512_LOG(LCD_DBG_INIT_ID, "[DISPDRV] %s:	OK\n\r", __func__);

	return res;

err_dma_init:
	CSL_DSI_CloseCmVc(pPanel->dsiCmVcHandle);
err_dsi_open_cm:
	CSL_DSI_CloseClient(pPanel->clientH);
err_dsi_open_cl:
	CSL_DSI_Close(pPanel->busNo);
err_dsi_init:
	if (pPanel->isTE)
		NT35512_TeOff(pPanel);
err_te_on:
	return -1;
}

/*
 *
 *  Function Name: NT35512_Close
 *
 *  Description:   disp bus OFF
 *
 */
Int32 NT35512_Close(DISPDRV_HANDLE_T drvH)
{
	Int32 res = 0;
	NT35512_PANEL_t	*pPanel	= (NT35512_PANEL_t *)drvH;

#ifdef CONFIG_VIDEO_MODE
	CSL_DSI_Suspend(pPanel->dsiCmVcHandle);
#endif

	if (CSL_DSI_CloseCmVc(pPanel->dsiCmVcHandle)) {
		NT35512_LOG(LCD_DBG_ERR_ID, "[DISPDRV] %s", __func__);
		NT35512_LOG(LCD_DBG_ERR_ID,
			":ERR Closing CMD Mode Handle\n\r");
		return -1;
	}

	if (CSL_DSI_CloseClient(pPanel->clientH) != CSL_LCD_OK) {
		NT35512_LOG(LCD_DBG_ERR_ID,
			"[DISPDRV] %s:ERROR, Closing DSI Client\n", __func__);
		return -1;
	}

	if (CSL_DSI_Close(pPanel->busNo) != CSL_LCD_OK) {
		NT35512_LOG(LCD_DBG_ERR_ID,
			"[DISPDRV] %s: ERR Closing DSI Controller\n", __func__);
		return -1;
	}

	if (pPanel->isTE)
		NT35512_TeOff(pPanel);

	if (brcm_disable_dsi_pll_clocks(pPanel->busNo)) {
		NT35512_LOG(LCD_DBG_ERR_ID, "[DISPDRV] %s", __func__);
		NT35512_LOG(LCD_DBG_ERR_ID,
			": ERROR to disable the pll clock\n");
		return -1;
	}

	pPanel->pwrState = STATE_PWR_OFF;
	pPanel->drvState = DRV_STATE_INIT;
	NT35512_LOG(LCD_DBG_INIT_ID, "[DISPDRV] %s:	OK\n\r", __func__);

	return res;
}


/*
 *
 * Function Name:  NT35512_WrSendCmnd
 *
 * Parameters:	   msg_buff   =	TX byte buffer
 *		   msg_size   =	size of message to be sent [bytes]
 *
 * Description:	   Send commands with variable no of parms
 *		   Assumption: DISPLAY's Controller Accepts DT Used
 */
static int NT35512_WrSendCmnd(
	DISPDRV_HANDLE_T	drvH,
	UInt8	*msg_buff,
	UInt32	msg_size)
{
	NT35512_PANEL_t	*pPanel	= (NT35512_PANEL_t *)drvH;
	CSL_DSI_CMND_t msg;
	int res = 0;

	switch (msg_size) {
	case 0:
		res = -1;
		break;
	case 1:
		msg.dsiCmnd = DSI_DT_SH_DCS_WR_P0;
		break;
	case 2:
		msg.dsiCmnd = DSI_DT_SH_DCS_WR_P1;
		break;
	default:
		if (msg_size <= CSL_DSI_GetMaxTxMsgSize())
			msg.dsiCmnd = DSI_DT_LG_DCS_WR;
		else
			res = -1;
		break;
	}

	if (res == 0) {
		msg.msg = msg_buff;
		msg.msgLen = msg_size;
		msg.vc = NT35512_VC;
		msg.isLP = NT35512_CMND_IS_LP;
		msg.isLong = msg_size > 2;
		msg.endWithBta = FALSE;

		CSL_DSI_SendPacket(pPanel->clientH, &msg, FALSE);
		NT35512_LOG(LCD_DBG_INIT_ID, "[DISPDRV] %s:", __func__);
		NT35512_LOG(LCD_DBG_INIT_ID,
			"DT[0x%02lX] SIZE[%lu]\n", msg.dsiCmnd, msg_size);
	}

	return res;
}


/*
 *
 *   Function Name:   NT35512_ExecCmndList
 *
 *   Description:
 *
 */
static void NT35512_ExecCmndList(
	DISPDRV_HANDLE_T	drvH,
	pDISPCTRL_REC_T	cmnd_lst)
{
#define TX_MSG_MAX 256

	UInt32 i = 0;
	UInt8 tx_buff[TX_MSG_MAX];
	UInt32 tx_size	= 0;

	while (cmnd_lst[i].type	!= DISPCTRL_LIST_END) {
		if (cmnd_lst[i].type ==	DISPCTRL_WR_CMND) {
			tx_buff[tx_size++] = cmnd_lst[i].cmnd;
			while (cmnd_lst[i+1].type == DISPCTRL_WR_DATA) {
				tx_buff[tx_size++] = cmnd_lst[i+1].data;
				i++;
			}

			NT35512_WrSendCmnd(drvH, tx_buff, tx_size);
			tx_size	= 0;
		} else if (cmnd_lst[i].type == DISPCTRL_WR_CMND_DATA) {
			tx_buff[tx_size++] = cmnd_lst[i].cmnd;
			tx_buff[tx_size++] = cmnd_lst[i].data;
			NT35512_WrSendCmnd(drvH, tx_buff, tx_size);
			tx_size	= 0;
		} else if (cmnd_lst[i].type == DISPCTRL_SLEEP_MS) {
			msleep(cmnd_lst[i].data);
		}
		i++;
	}
}

/*
 *
 *   Function Name: NT35512_PowerControl
 *
 *   Description:	  Display Module Control
 *
 */
Int32 NT35512_PowerControl(
	DISPDRV_HANDLE_T	drvH,
	DISPLAY_POWER_STATE_T	state)
{
	Int32  res = 0;
	NT35512_PANEL_t   *pPanel =	(NT35512_PANEL_t *)drvH;

	switch (state) {
	case CTRL_PWR_ON:
		switch (pPanel->pwrState) {
		case STATE_PWR_OFF:
			NT35512_LOG(LCD_DBG_ERR_ID, "[DISPDRV]	%s:", __func__);
			NT35512_LOG(LCD_DBG_ERR_ID,
				"current STATE_PWR_OFF request CTRL_PWR_ON\n\r");
			NT35512_panel_init(pPanel);
			OSTASK_Sleep(TICKS_IN_MILLISECONDS(120));
			pPanel->pwrState = STATE_SCREEN_OFF;
			break;
		default:
			NT35512_LOG(LCD_DBG_ERR_ID, "[DISPDRV] %s:", __func__);
			NT35512_LOG(LCD_DBG_ERR_ID,
				"POWER ON Requested While Not In POWER DOWN State\n\r");
			break;
		}
		break;
	case CTRL_PWR_OFF:
		if (pPanel->pwrState != STATE_PWR_OFF) {
			NT35512_LOG(LCD_DBG_ERR_ID, "[DISPDRV] %s", __func__);
			NT35512_LOG(LCD_DBG_ERR_ID,
				":current state=!STATE_PWR_OFF request CTRL_PWR_OFF\n\r ");
#ifdef CONFIG_VIDEO_MODE
			NT35512_panel_shut_down(pPanel);
#else
			NT35512_panel_off(pPanel);
#endif
			memset(&pPanel->win_cur, 0, sizeof(DISPDRV_WIN_t));
			pPanel->pwrState = STATE_PWR_OFF;
		}
		break;
	case CTRL_SLEEP_IN:
		switch (pPanel->pwrState) {
		case STATE_SCREEN_ON:
			NT35512_LOG(LCD_DBG_ERR_ID,
				"[DISPDRV]	%s: current state=STATE_SCREEN_ON  "
				"request CTRL_SLEEP_IN\n\r", __func__);
#ifdef CONFIG_VIDEO_MODE
			NT35512_panel_shut_down(pPanel);
#else
			NT35512_panel_off(pPanel);
#endif
		case STATE_SCREEN_OFF:
			NT35512_LOG(LCD_DBG_ERR_ID,
				"[DISPDRV]	%s: current state=STATE_SCREEN_OFF  "
				"request CTRL_SLEEP_IN\n\r", __func__);
			NT35512_panel_sleep_in(pPanel);
			OSTASK_Sleep(TICKS_IN_MILLISECONDS(120));
			pPanel->pwrState = STATE_SLEEP;
			break;
		default:
			NT35512_LOG(LCD_DBG_ERR_ID,
				"[DISPDRV] %s", __func__);
			NT35512_LOG(LCD_DBG_ERR_ID,
				":SLEEP Requested, But Not In DISP ON|OFF State\n\r");
			break;
		}
		break;
	case CTRL_SLEEP_OUT:
		switch (pPanel->pwrState) {
		case STATE_SLEEP:
			NT35512_panel_sleep_out(pPanel);
			OSTASK_Sleep(TICKS_IN_MILLISECONDS(120));
			pPanel->pwrState = STATE_SCREEN_OFF;
			NT35512_LOG(LCD_DBG_ERR_ID,
				"[DISPDRV]	%s: SLEEP-OUT\n\r", __func__);
			break;
		default:
			NT35512_LOG(LCD_DBG_ERR_ID, "[DISPDRV] %s", __func__);
			NT35512_LOG(LCD_DBG_ERR_ID,
				":SLEEP-OUT While Not In SLEEP\n");
			break;
		}
		break;
	case CTRL_SCREEN_ON:
		switch (pPanel->pwrState) {
		case STATE_SCREEN_OFF:
#ifdef CONFIG_VIDEO_MODE
			NT35512_panel_turn_on(pPanel);
#else
			NT35512_panel_on(pPanel);
#endif
			pPanel->pwrState = STATE_SCREEN_ON;
			NT35512_LOG(LCD_DBG_ERR_ID,
				"[DISPDRV]	%s: SCREEN ON\n", __func__);
			break;
		default:
			NT35512_LOG(LCD_DBG_ERR_ID, "[DISPDRV] %s", __func__);
			NT35512_LOG(LCD_DBG_ERR_ID,
				"SCREEN ON While Not In SCREEN OFF\n");
			break;
		}
		break;
	case CTRL_SCREEN_OFF:
		switch (pPanel->pwrState) {
		case STATE_SCREEN_ON:
#ifdef CONFIG_VIDEO_MODE
			NT35512_panel_shut_down(pPanel);
#else
			NT35512_panel_off(pPanel);
#endif
			pPanel->pwrState = STATE_SCREEN_OFF;
			NT35512_LOG(LCD_DBG_ERR_ID,
				"[DISPDRV]	%s: SCREEN OFF\n", __func__);
			break;
		default:
			NT35512_LOG(LCD_DBG_ERR_ID, "[DISPDRV] %s", __func__);
			NT35512_LOG(LCD_DBG_ERR_ID,
				"SCREEN OFF While Not In SCREEN ON\n");
			break;
		}
		break;
	default:
		NT35512_LOG(LCD_DBG_ERR_ID, "[DISPDRV] %s:" __func__);
		NT35512_LOG(LCD_DBG_ERR_ID, "Invalid Power State[%d]\n", state);
		res = -1;
		break;
	}

	return res;
}

/*
 *
 *  Function Name: NT35512_Start
 *
 *  Description:
 *
 */
Int32 NT35512_Start(
	DISPDRV_HANDLE_T	drvH,
	struct pi_mgr_dfs_node	*dfs_node)
{
	NT35512_PANEL_t *pPanel = (NT35512_PANEL_t *)drvH;

	if (brcm_enable_dsi_lcd_clocks(dfs_node, pPanel->busNo,
		NT35512_dsiCfg.hsBitClk.clkIn_MHz * 1000000,
		NT35512_dsiCfg.hsBitClk.clkInDiv,
		NT35512_dsiCfg.escClk.clkIn_MHz * 1000000
		/ NT35512_dsiCfg.escClk.clkInDiv)) {

		NT35512_LOG(LCD_DBG_ERR_ID,
			"[DISPDRV] %s: ERROR to enable the clock\n", __func__);
		return -1;
	}

	return 0;
}

/*
 *
 *   Function Name: NT35512_Stop
 *
 *   Description:
 *
 */
Int32 NT35512_Stop(DISPDRV_HANDLE_T drvH,
	struct pi_mgr_dfs_node *dfs_node)
{
	NT35512_PANEL_t *pPanel = (NT35512_PANEL_t *)drvH;

	if (brcm_disable_dsi_lcd_clocks(dfs_node, pPanel->busNo)) {
		NT35512_LOG(LCD_DBG_ERR_ID,
			"[DISPDRV] %s: ERROR to enable the clock\n", __func__);
		return -1;
	}
	return 0;
}

/*
 *
 *   Function Name: NT35512_GetInfo
 *
 *   Description:
 *
 */
const DISPDRV_INFO_T *NT35512_GetDispDrvData(
	DISPDRV_HANDLE_T drvH)
{
	return &NT35512_Info;
}

/*
 *
 *  Function Name: NT35512_GetDispDrvFeatures
 *
 *  Description:
 *
 */
Int32 NT35512_GetDispDrvFeatures(
	DISPDRV_HANDLE_T	drvH,
	const char	**driver_name,
	UInt32	*version_major,
	UInt32	*version_minor,
	DISPDRV_SUPPORT_FEATURES_T	*flags)
{
	Int32 res = -1;

	if ((NULL != driver_name)   && (NULL !=	version_major) &&
		(NULL != version_minor) && (NULL !=	flags))	{
		*driver_name = "NT35512 (IN:ARG888 OUT:RGB888)";
		*version_major = 0;
		*version_minor = 15;
		*flags = DISPDRV_SUPPORT_NONE;
		res = 0;
	}

	return res;
}


/*
 *
 *  Function Name: NT35512_Cb
 *
 *  Description:   CSL callback
 *
 */
static void NT35512_Cb(CSL_LCD_RES_T cslRes,
	pCSL_LCD_CB_REC pCbRec)
{
	DISPDRV_CB_RES_T apiRes;

	NT35512_LOG(LCD_DBG_ID, "[DISPDRV]	+%s\r\n", __func__);

	if (pCbRec->dispDrvApiCb != NULL) {
		switch (cslRes) {
		case CSL_LCD_OK:
			apiRes = DISPDRV_CB_RES_OK;
			break;
		default:
			apiRes = DISPDRV_CB_RES_ERR;
		break;
		}
		((DISPDRV_CB_T)pCbRec->dispDrvApiCb)(apiRes);
	}
	NT35512_LOG(LCD_DBG_ID, "[DISPDRV] -%s\r\n", __func__);
}


/*
 *
 *  Function Name: NT35512_Update
 *
 *  Description:
 *
 */
Int32 NT35512_Update(
	DISPDRV_HANDLE_T	drvH,
	void	*buff,
	DISPDRV_WIN_t	*p_win,
	DISPDRV_CB_T	apiCb)
{
	NT35512_PANEL_t	*pPanel	= (NT35512_PANEL_t *)drvH;
	CSL_LCD_UPD_REQ_T req;
	Int32 res  = 0;
	uint32_t offset;

	NT35512_LOG(LCD_DBG_ID, "[DISPDRV] +%s\r\n", __func__);
	if (pPanel->pwrState ==	STATE_PWR_OFF) {
		NT35512_LOG(LCD_DBG_ERR_ID,
			"[DISPDRV] %s: Skip Due To Power State\n", __func__);
		return -1;
	}

	if (p_win == NULL)
		p_win =	&pPanel->win_dim;

	NT35512_LOG(LCD_DBG_ID, "%s %d %d %d %d\n", __func__,
		p_win->l, p_win->r, p_win->t, p_win->b);
	NT35512_WinSet(drvH, TRUE, p_win);

	offset = (uint32_t)buff;
	offset += (p_win->t * pPanel->disp_info->width + p_win->l)
		* pPanel->disp_info->Bpp;
	req.buff = (uint32_t *)offset;
	req.lineLenP = p_win->w;
	req.lineCount = p_win->h;
	req.xStrideB = pPanel->disp_info->width - p_win->w;
	req.buffBpp = pPanel->disp_info->Bpp;

#ifdef CONFIG_VIDEO_MODE
	req.timeOut_ms = MAX_SCHEDULE_TIMEOUT;
#else
	req.timeOut_ms = 200;
#endif

	NT35512_LOG(LCD_DBG_ID, "%s: buf=%08x, linelenp = %lu, linecnt =%lu\n",
		__func__, (u32)req.buff, req.lineLenP, req.lineCount);

	req.cslLcdCbRec.cslH = pPanel->clientH;
	req.cslLcdCbRec.dispDrvApiCbRev = DISP_DRV_CB_API_REV_1_0;
	req.cslLcdCbRec.dispDrvApiCb = (void	*) apiCb;
	req.cslLcdCbRec.dispDrvApiCbP1 = NULL;

	if (apiCb != NULL)
		req.cslLcdCb = NT35512_Cb;
	else
		req.cslLcdCb = NULL;
#ifndef CONFIG_VIDEO_MODE
	if (CSL_DSI_UpdateCmVc(pPanel->dsiCmVcHandle, &req, pPanel->isTE)
		!= CSL_LCD_OK) {
		NT35512_LOG(LCD_DBG_ERR_ID,
			"[DISPDRV] %s:	ERROR ret by "
			"CSL_DSI_UpdateCmVc\n\r", __func__);
		res = -1;
		NT35512_Cb(res, &req.cslLcdCbRec);
	}
#else
	if (CSL_DSI_UpdateVmVc(pPanel->dsiCmVcHandle, &req)
		!= CSL_LCD_OK) {
		NT35512_LOG(LCD_DBG_ERR_ID, "[DISPDRV] %s", __func__);
		NT35512_LOG(LCD_DBG_ERR_ID,
			":ERROR ret by CSL_DSI_UpdateCmVc\n\r");
		res = -1;
		NT35512_Cb(res, &req.cslLcdCbRec);
	}
#endif
	NT35512_LOG(LCD_DBG_ID, "[DISPDRV]	-%s\r\n", __func__);

	return res;
}

/*
 *
 *  Function Name: NT35512_Atomic_Update
 *
 *  Description:
 *
 */
Int32 NT35512_Atomic_Update(
	DISPDRV_HANDLE_T drvH,
	void	*buff,
	DISPDRV_WIN_t	*p_win)
{
	NT35512_PANEL_t	*pPanel	= (NT35512_PANEL_t *)drvH;
	CSL_LCD_UPD_REQ_T req;
	Int32 res  = 0;

	NT35512_LOG(LCD_DBG_ID, "[DISPDRV]	+%s\r\n", __func__);

	if (pPanel->pwrState ==	STATE_PWR_OFF) {
		NT35512_LOG(LCD_DBG_ERR_ID,
			"[DISPDRV] %s: Skip Due To Power State\n", __func__);
		return -1;
	}
	if (p_win == NULL)
		p_win =	&pPanel->win_dim;

	CSL_DSI_Force_Stop(pPanel->dsiCmVcHandle);
	CSL_DSI_Lock(pPanel->dsiCmVcHandle);
	csl_dma_lock();

	NT35512_WinSet(drvH, TRUE, p_win);

	req.buff	= buff;
	req.lineLenP	= p_win->w;
	req.lineCount	= p_win->h;
	req.buffBpp	= pPanel->disp_info->Bpp;
#ifdef CONFIG_VIDEO_MODE
	req.timeOut_ms = MAX_SCHEDULE_TIMEOUT;
#else
	req.timeOut_ms	= 100;
#endif
	req.xStrideB	= 0;

	NT35512_LOG(LCD_DBG_ID, "%s: buf=%08x, linelenp = %lu, linecnt =%lu\n",
		__func__, (u32)req.buff, req.lineLenP, req.lineCount);

	req.cslLcdCbRec.cslH	= pPanel->clientH;
	req.cslLcdCbRec.dispDrvApiCbRev	= DISP_DRV_CB_API_REV_1_0;
	req.cslLcdCbRec.dispDrvApiCb	= NULL;
	req.cslLcdCbRec.dispDrvApiCbP1	= NULL;
	req.cslLcdCb = NULL;

#ifndef CONFIG_VIDEO_MODE
	if (CSL_DSI_UpdateCmVc(pPanel->dsiCmVcHandle, &req, pPanel->isTE)
		!= CSL_LCD_OK)	{
		NT35512_LOG(LCD_DBG_ERR_ID, "[DISPDRV] %s", __func__);
		NT35512_LOG(LCD_DBG_ERR_ID,
			":ERR ret by CSL_DSI_UpdateCmVc\n\r");
		res = -1;
	}
#else
	if (CSL_DSI_UpdateVmVc(pPanel->dsiCmVcHandle, &req)
		!= CSL_LCD_OK)	{
		NT35512_LOG(LCD_DBG_ERR_ID, "[DISPDRV] %s", __func__);
		NT35512_LOG(LCD_DBG_ERR_ID,
			":ERR ret by CSL_DSI_UpdateCmVc\n\r");
		res = -1;
	}
#endif
	NT35512_LOG(LCD_DBG_ID, "[DISPDRV]	-%s\r\n", __func__);

	csl_dma_unlock();
	CSL_DSI_Unlock(pPanel->dsiCmVcHandle);

	return res;
}

