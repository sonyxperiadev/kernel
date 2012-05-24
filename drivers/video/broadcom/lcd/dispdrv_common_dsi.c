/*******************************************************************************
* Copyright 2012 Broadcom Corporation.	All rights reserved.
*
* @file	drivers/video/broadcom/dispdrv_hx8369a_dsi.c
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use	of this	software, this software	is licensed to you under the
* terms	of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may	you combine this
* software in any way with any other Broadcom software provided	under a	license
* other	than the GPL, without Broadcom's express prior written consent.
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

#ifdef CONFIG_LCD_HX8369_SUPPORT
#include "lcd_hx8369.h"
#endif

#define	LCD_DISPDRV_VC			(0)
#define	LCD_DISPDRV_CMND_IS_LP	FALSE /* display init comm LP or HS mode */

typedef	struct {
	CSL_LCD_HANDLE		clientH;
	CSL_LCD_HANDLE		dsiCmVcHandle;
	DISP_DRV_STATE		drvState;
	DISP_PWR_STATE		pwrState;
	UInt32			busNo;
	UInt32			teIn;
	UInt32			teOut;
	Boolean			isTE;
	DISPDRV_WIN_t		win_dim;
	DISPDRV_WIN_t		win_cur;
	struct pi_mgr_dfs_node	*dfs_node;

	Boolean			boot_mode;
	UInt32			rst;
	CSL_DSI_CM_VC_t		*cmnd_mode;
	CSL_DSI_CFG_t		*dsi_cfg;
	DISPDRV_INFO_T		*disp_info;
} LCD_DISPDRV_PANEL_t;

/* LOCAL FUNCTIONs */
static void LCD_DISPDRV_WrCmndP0(DISPDRV_HANDLE_T	drvH, UInt32	reg);

/* DRV INTERFACE FUNCTIONs */
static Int32 LCD_DISPDRV_Init(struct dispdrv_init_parms	*parms, DISPDRV_HANDLE_T *handle);

static Int32 LCD_DISPDRV_Exit(DISPDRV_HANDLE_T	drvH);

static Int32 LCD_DISPDRV_Open(DISPDRV_HANDLE_T	drvH);

static Int32 LCD_DISPDRV_Close(DISPDRV_HANDLE_T drvH);

static Int32 LCD_DISPDRV_GetDispDrvFeatures(DISPDRV_HANDLE_T drvH,
	const char			**driver_name,
	UInt32				*version_major,
	UInt32				*version_minor,
	DISPDRV_SUPPORT_FEATURES_T	*flags);

const DISPDRV_INFO_T *LCD_DISPDRV_GetDispDrvData(DISPDRV_HANDLE_T drvH);

static Int32 LCD_DISPDRV_Start(DISPDRV_HANDLE_T drvH,
	struct pi_mgr_dfs_node *dfs_node);

static Int32 LCD_DISPDRV_Stop(DISPDRV_HANDLE_T drvH,
	struct pi_mgr_dfs_node *dfs_node);

static Int32 LCD_DISPDRV_PowerControl(DISPDRV_HANDLE_T	drvH,
	DISPLAY_POWER_STATE_T	state);

static Int32 LCD_DISPDRV_Update(DISPDRV_HANDLE_T	drvH,
	 void			*buff,
	 DISPDRV_WIN_t		*p_win,
	 DISPDRV_CB_T		apiCb);

static Int32 LCD_DISPDRV_WinReset(DISPDRV_HANDLE_T drvH);

static DISPDRV_T LCD_DISPDRV_Drv = {
	&LCD_DISPDRV_Init,					/* init	*/
	&LCD_DISPDRV_Exit,					/* exit	*/
	&LCD_DISPDRV_GetDispDrvFeatures,	/* info	*/
	&LCD_DISPDRV_Open,					/* open	*/
	&LCD_DISPDRV_Close,					/* close */
	&LCD_DISPDRV_GetDispDrvData,		/* get_info */
	&LCD_DISPDRV_Start,					/* start */
	&LCD_DISPDRV_Stop,					/* stop	*/
	&LCD_DISPDRV_PowerControl,			/* power_control */
	NULL,							/* update_no_os	*/
	&LCD_DISPDRV_Update,				/* update */
	NULL,							/* set_brightness */
	&LCD_DISPDRV_WinReset,				/* reset_win */
};

/* DSI Command Mode VC Configuration */
CSL_DSI_CM_VC_t	LCD_DISPDRV_VcCmCfg = {
	LCD_DISPDRV_VC,							/* VC */
	DSI_DT_LG_DCS_WR,					/* dsiCmnd */
	MIPI_DCS_WRITE_MEMORY_START,		/* dcsCmndStart	*/
	MIPI_DCS_WRITE_MEMORY_CONTINUE,	/* dcsCmndContinue */
	FALSE,								/* isLP	*/
	LCD_IF_CM_I_RGB888U,				/* cm_in */
	LCD_IF_CM_O_RGB888,					/* cm_out */
	/* TE configuration */
	{
		DSI_TE_CTRLR_INPUT_0,			/* DSI Te Input	Type */
	},
};

/* DSI BUS CONFIGURATION */
CSL_DSI_CFG_t LCD_DISPDRV_dsiCfg = {
	0,					/* bus */
	1,					/* dlCount */
	DSI_DPHY_0_92,		/* DSI_DPHY_SPEC_T */
	/* ESC CLK Config */
	{500, 5},		/* escClk fixed 156[MHz], DIV by 2 = 78[MHz] */

	/* HS CLK Config */
	{1000, 2},	/* hsBitClk PLL	300[MHz], DIV by 1 = 300[Mbps]*/

	/* LP Speed */
	5,			/* lpBitRate_Mbps, Max 10[Mbps] */

	FALSE,				/* enaContClock	*/
	TRUE,				/* enaRxCrc */
	TRUE,				/* enaRxEcc */
	TRUE,				/* enaHsTxEotPkt */
	FALSE,				/* enaLpTxEotPkt */
	FALSE,				/* enaLpRxEotPkt */
};

static LCD_DISPDRV_PANEL_t   panel[1];

/*###########################################################################*/

/*************************************************************************************************
 *
 *   Function Name: LCD_DISPDRV_TeOn
 *
 *   Description:   Configure TE Input Pin & Route it to DSI Controller	Input
 *
 **************************************************************************************************/
static int LCD_DISPDRV_TeOn(LCD_DISPDRV_PANEL_t *pPanel)
{
	Int32 res	= 0;

	TECTL_CFG_t teCfg;

	teCfg.te_mode = TE_VC4L_MODE_VSYNC;
	teCfg.sync_pol = TE_VC4L_ACT_POL_LO;
	teCfg.vsync_width = 70000;
	teCfg.hsync_line  = 2;

	res = CSL_TECTL_VC4L_OpenInput(pPanel->teIn, pPanel->teOut, &teCfg);

	return (res);
}

/*****************************************************************************************************
 *
 *   Function Name: LCD_DISPDRV_TeOff
 *
 *   Description:   'Release' TE Input Pin Used
 *
 ******************************************************************************************************/
static int LCD_DISPDRV_TeOff(LCD_DISPDRV_PANEL_t *pPanel)
{
	Int32  res = 0;

	res = CSL_TECTL_VC4L_CloseInput(pPanel->teIn);

	return (res);
}


/******************************************************************************************************
 *
 *   Function Name: LCD_DISPDRV_WrCmndP0
 *
 *   Description:
 *
 ******************************************************************************************************/
static void LCD_DISPDRV_WrCmndP0(DISPDRV_HANDLE_T drvH, UInt32	reg)
{
	LCD_DISPDRV_PANEL_t	*pPanel	= (LCD_DISPDRV_PANEL_t *)drvH;
	CSL_DSI_CMND_t		msg;
	UInt8			msgData[4];

	msg.dsiCmnd    = DSI_DT_SH_DCS_WR_P0;
	msg.msg	         = &msgData[0];
	msg.msgLen     = 1;
	msg.vc	         = LCD_DISPDRV_VC;
	msg.isLP       = LCD_DISPDRV_CMND_IS_LP;
	msg.isLong     = FALSE;
	msg.endWithBta = FALSE;

	msgData[0] = reg;
	msgData[1] = 0;

	CSL_DSI_SendPacket(pPanel->clientH, &msg, FALSE);
}

/*****************************************************************************************************
 *
 *   Function Name: LCD_DISPDRV_WinSet
 *
 *   Description:   Set	Window
 *
 *****************************************************************************************************/
Int32 LCD_DISPDRV_WinSet(DISPDRV_HANDLE_T	drvH,
				Boolean update, DISPDRV_WIN_t *p_win)
{
	LCD_DISPDRV_PANEL_t *pPanel = (LCD_DISPDRV_PANEL_t *) drvH;
	CSL_DSI_CMND_t	    msg;
	UInt8		    msgData[5];
	CSL_DSI_REPLY_t	    rxMsg;
	UInt8		    rx_buff[8];

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
			msg.vc = LCD_DISPDRV_VC;
			msg.isLP = LCD_DISPDRV_CMND_IS_LP;
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

			LCD_DISPDRV_WrCmndP0(drvH, MIPI_DCS_WRITE_MEMORY_START);

			printk(KERN_INFO "lsq_WinSet l: %d, r: %d, t: %d, b: %d\n", p_win->l, p_win->r, p_win->t, p_win->b);

			return (0);
		}
	}
	return 0;
}

/*****************************************************************************************************
 *
 *   Function Name: LCD_DISPDRV_WinReset
 *
 *   Description:   Reset	windowing to full screen size.
 *		    Typically, only used in boot code environment
 *****************************************************************************************************/
Int32 LCD_DISPDRV_WinReset(DISPDRV_HANDLE_T drvH)
{
	Int32 res;
	LCD_DISPDRV_PANEL_t *pPanel = (LCD_DISPDRV_PANEL_t *) drvH;

	res = LCD_DISPDRV_WinSet(drvH, TRUE, &pPanel->win_dim);
	return (res);
}

/*******************************************************************************************************
 *
 * Function Name:  LCD_DISPDRV_WrSendCmnd
 *
 * Parameters:	   msg_buff   =	TX byte	buffer
 *		   msg_size   =	size of	message	to be sent [bytes]
 *
 * Description:	   Send	commands with variable no of parms
 *		   Assumption: DISPLAY's Controller Accepts DT Used
 *******************************************************************************************************/
static int LCD_DISPDRV_WrSendCmnd(DISPDRV_HANDLE_T drvH,
			UInt8 *msg_buff, UInt32 msg_size)
{
	LCD_DISPDRV_PANEL_t	*pPanel	= (LCD_DISPDRV_PANEL_t *)drvH;
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
		if (msg_size <=	CSL_DSI_GetMaxTxMsgSize())
			msg.dsiCmnd = DSI_DT_LG_DCS_WR;
		else
			res = -1;
		break;
	}

	if (res == 0) {
		msg.msg = msg_buff;
		msg.msgLen = msg_size;
		msg.vc = LCD_DISPDRV_VC;
		msg.isLP = LCD_DISPDRV_CMND_IS_LP;
		msg.isLong = msg_size > 2;
		msg.endWithBta = FALSE;

		CSL_DSI_SendPacket(pPanel->clientH, &msg, FALSE);
		LCD_DBG(LCD_DBG_INIT_ID, "[DISPDRV] %s:	DT[0x%02lX] "
			"SIZE[%lu]\n", __FUNCTION__, msg.dsiCmnd, msg_size);
	}

	return (res);
}

/******************************************************************************************************
 *
 *   Function Name:   LCD_DISPDRV_ExecCmndList
 *
 *   Description:
 *
 ******************************************************************************************************/
static void LCD_DISPDRV_ExecCmndList(DISPDRV_HANDLE_T drvH, pDISPCTRL_REC_T cmnd_lst)
{
	#define	TX_MSG_MAX	256
	UInt32 i = 0;
	UInt32 j = 0;
	UInt32 flag = 0;
	UInt8 tx_buff[TX_MSG_MAX];
	UInt32 tx_size	= 0;

	while (cmnd_lst[i].type	!= DISPCTRL_LIST_END) {
		if (cmnd_lst[i].type ==	DISPCTRL_WR_CMND) {
			tx_buff[tx_size++] = cmnd_lst[i].cmnd;

			while (cmnd_lst[i+1].type == DISPCTRL_WR_DATA) {
				tx_buff[tx_size++] = cmnd_lst[i+1].data;
				i++;
			}
			while (cmnd_lst[i+1].type == DISPCTRL_CREATE_LOOK_UP_TABLE) {
				if (flag ==1) {
					for (j=0; j<64; j++)
						tx_buff[tx_size++] = 0x04*j;
				}
				else {
					for (j=0; j<64; j++)
						tx_buff[tx_size++] = 0x08*j;
				}
				i++;
				flag++;
			}
			LCD_DISPDRV_WrSendCmnd(drvH, tx_buff, tx_size);
			tx_size	= 0;
		} else if (cmnd_lst[i].type == DISPCTRL_WR_CMND_DATA) {
			tx_buff[tx_size++] = cmnd_lst[i].cmnd;
			tx_buff[tx_size++] = cmnd_lst[i].data;
			LCD_DISPDRV_WrSendCmnd(drvH, tx_buff, tx_size);
			tx_size	= 0;

		} else if (cmnd_lst[i].type == DISPCTRL_SLEEP_MS) {
			OSTASK_Sleep(TICKS_IN_MILLISECONDS(cmnd_lst[i].data));
		}
		i++;
	}
} 

/**************************************************************************************************
 *
 *   Function Name: LCD_DISPDRV_GetDrvInfo
 *
 *   Description:   Get	Driver Funtion Table
 *
 ***************************************************************************************************/
DISPDRV_T *LCD_DISPDRV_GetFuncTable(void)
{
	return (&LCD_DISPDRV_Drv);
}

/****************************************************************************************************
 *
 *   Function Name: LCD_DISPDRV_reset
 *
 *   Description:   (De)Assert display reset
 *
 ****************************************************************************************************/
static void LCD_DISPDRV_reset(DISPDRV_HANDLE_T drvH)
{
	LCD_DISPDRV_PANEL_t *pPanel = (LCD_DISPDRV_PANEL_t *) drvH;
	u32 gpio = pPanel->rst; //gpio=12;

	if (gpio != 0) {
		gpio_request(gpio, "LCD_RST");
		gpio_direction_output(gpio, 0);
		gpio_set_value_cansleep(gpio, 1);
		msleep(10);
		gpio_set_value_cansleep(gpio, 0);
		msleep(100);
		gpio_set_value_cansleep(gpio, 1);
		msleep(20);
	}

}

/*****************************************************************************************************
 *
 *   Function Name: LCD_DISPDRV_Init
 *
 *   Description:   Setup / Verify display driver init interface
 *
 *****************************************************************************************************/
Int32 LCD_DISPDRV_Init(struct dispdrv_init_parms *parms,
			DISPDRV_HANDLE_T		*handle)
{
	Int32 res = 0;
	LCD_DISPDRV_PANEL_t	*pPanel;

	pPanel = &panel[0];

	if (pPanel->drvState ==DRV_STATE_OFF)	{

		LCD_DBG(LCD_DBG_INIT_ID, "[DISPDRV] %s:	Bus	    %d\n",
			__FUNCTION__, parms->w0.bits.bus_type);
		LCD_DBG(LCD_DBG_INIT_ID, "[DISPDRV] %s:	BootMode   %d\n",
			__FUNCTION__, parms->w0.bits.boot_mode);
		LCD_DBG(LCD_DBG_INIT_ID, "[DISPDRV] %s:	BusNo	    %d\n",
			__FUNCTION__, parms->w0.bits.bus_no);
		LCD_DBG(LCD_DBG_INIT_ID, "[DISPDRV] %s:	col_mode_i %d\n",
			__FUNCTION__, parms->w0.bits.col_mode_i);
		LCD_DBG(LCD_DBG_INIT_ID, "[DISPDRV] %s:	col_mode_o %d\n",
			__FUNCTION__, parms->w0.bits.col_mode_o);
		LCD_DBG(LCD_DBG_INIT_ID, "[DISPDRV] %s:	te_input   %d\n",
			__FUNCTION__, parms->w0.bits.te_input);

		LCD_DBG(LCD_DBG_INIT_ID, "[DISPDRV] %s:	API Rev	   %d\n",
			__FUNCTION__, parms->w1.bits.api_rev);
		LCD_DBG(LCD_DBG_INIT_ID, "[DISPDRV] %s:	Rst 0	    %d\n",
			__FUNCTION__, parms->w1.bits.lcd_rst0);
		LCD_DBG(LCD_DBG_INIT_ID, "[DISPDRV] %s:	Rst 1	    %d\n",
			__FUNCTION__, parms->w1.bits.lcd_rst1);
		LCD_DBG(LCD_DBG_INIT_ID, "[DISPDRV] %s:	Rst 2	    %d\n",
			__FUNCTION__, parms->w1.bits.lcd_rst2);

		if ((u8)parms->w1.bits.api_rev != RHEA_LCD_BOOT_API_REV) {
			LCD_DBG(LCD_DBG_ERR_ID,	"[DISPDRV] %s:	"
				"Boot Init API Rev Mismatch(%d.%d vs %d.%d)\n",
				__FUNCTION__,
				(parms->w1.bits.api_rev	& 0xF0)	>> 8,
				(parms->w1.bits.api_rev	& 0x0F)	    ,
				(RHEA_LCD_BOOT_API_REV	& 0xF0)	>> 8,
				(RHEA_LCD_BOOT_API_REV	& 0x0F));
			return -1;
		}

		pPanel->boot_mode = parms->w0.bits.boot_mode;

		pPanel->cmnd_mode = &LCD_DISPDRV_VcCmCfg;
		pPanel->dsi_cfg	  = &LCD_DISPDRV_dsiCfg;
		pPanel->disp_info = &HX8369A_Info;

		pPanel->busNo =	dispdrv2busNo(parms->w0.bits.bus_no);

		/* check for valid DSI bus no */
		if (pPanel->busNo & 0xFFFFFFFE)	{
			LCD_DBG(LCD_DBG_ERR_ID,	"[DISPDRV] %s:	Invlid DSI "
				"BusNo[%lu]\n", __FUNCTION__, pPanel->busNo);
			return -1;
		}

		pPanel->cmnd_mode->cm_in  =
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
			LCD_DBG(LCD_DBG_ERR_ID,	"[DISPDRV] %s:	Unsupported"
				" Color	Mode\n", __FUNCTION__);
			return -1;
		}

		/* get reset pins */
		pPanel->rst = parms->w1.bits.lcd_rst0;
		
		pPanel->isTE = TRUE;//pPanel->cmnd_mode->teCfg.teInType != DSI_TE_NONE;

		/* get TE pin configuration */
		pPanel->teIn  =	dispdrv2busTE(parms->w0.bits.te_input);
		pPanel->teOut =	pPanel->busNo
			? TE_VC4L_OUT_DSI1_TE0 : TE_VC4L_OUT_DSI0_TE0;

		pPanel->drvState = DRV_STATE_INIT;
		pPanel->pwrState = STATE_PWR_OFF;

		*handle	= (DISPDRV_HANDLE_T)pPanel;

		LCD_DBG(LCD_DBG_INIT_ID, "[DISPDRV] %s:	OK\n", __FUNCTION__);
	} else {
		LCD_DBG(LCD_DBG_ERR_ID,	"[DISPDRV] %s:	Not in OFF state\n",
			__FUNCTION__);
		res = -1;
	}

	return (res);
}

/******************************************************************************************************
 *
 *   Function Name: LCD_DISPDRV_Exit
 *
 *   Description:
 *
 ******************************************************************************************************/
Int32 LCD_DISPDRV_Exit(DISPDRV_HANDLE_T	drvH)
{
	LCD_DISPDRV_PANEL_t *pPanel;

	pPanel = (LCD_DISPDRV_PANEL_t *) drvH;
	pPanel->drvState = DRV_STATE_OFF;
	return 0;
}

/******************************************************************************************************
 *
 *  Function Name: LCD_DISPDRV_Open
 *
 *  Description:   disp	bus ON
 *
 ******************************************************************************************************/
Int32 LCD_DISPDRV_Open(DISPDRV_HANDLE_T drvH)
{
	Int32 res = 0;
	LCD_DISPDRV_PANEL_t	*pPanel;

	pPanel = (LCD_DISPDRV_PANEL_t *) drvH;

	if (pPanel->drvState != DRV_STATE_INIT)	{
		LCD_DBG(LCD_DBG_ERR_ID,	"[DISPDRV] %s: ERROR State != Init\n",
			__FUNCTION__);
		return -1;
	}

	if (brcm_enable_dsi_pll_clocks(pPanel->busNo,
			LCD_DISPDRV_dsiCfg.hsBitClk.clkIn_MHz * 1000000,
			LCD_DISPDRV_dsiCfg.hsBitClk.clkInDiv,
			LCD_DISPDRV_dsiCfg.escClk.clkIn_MHz   * 1000000/
			LCD_DISPDRV_dsiCfg.escClk.clkInDiv)) {
		LCD_DBG(LCD_DBG_ERR_ID,	"[DISPDRV] %s:	ERROR to enable the ""pll clock\n",
			__FUNCTION__);
	}

	if (pPanel->isTE && LCD_DISPDRV_TeOn(pPanel) == -1) {
		LCD_DBG(LCD_DBG_ERR_ID,	"[DISPDRV] %s:	"
			"Failed	To Configure TE	Input\n", __FUNCTION__);
		return -1;
	}

	LCD_DISPDRV_reset(drvH);

	if (CSL_DSI_Init(pPanel->dsi_cfg) != CSL_LCD_OK) {
		LCD_DBG(LCD_DBG_ERR_ID,	"[DISPDRV] %s:	ERROR, DSI CSL Init "
			"Failed\n", __FUNCTION__);
		return -1;
	}

	if (CSL_DSI_OpenClient(pPanel->busNo, &pPanel->clientH)	!= CSL_LCD_OK) {
		LCD_DBG(LCD_DBG_ERR_ID,	"[DISPDRV] %s:	ERROR, "
			"CSL_DSI_OpenClient Failed\n", __FUNCTION__);
		return -1;
	}

	if (CSL_DSI_OpenCmVc(pPanel->clientH,
			pPanel->cmnd_mode, &pPanel->dsiCmVcHandle) != CSL_LCD_OK) {
		LCD_DBG(LCD_DBG_ERR_ID,	"[DISPDRV] %s:	CSL_DSI_OpenCmVc "
			"Failed\n", __FUNCTION__);
		return -1;
	}

	if (csl_dma_vc4lite_init() != DMA_VC4LITE_STATUS_SUCCESS) {
		LCD_DBG(LCD_DBG_ERR_ID,	"[DISPDRV] %s: csl_dma_vc4lite_init "
			"Failed\n", __FUNCTION__);
		return -1;
	}

	pPanel->win_dim.l = 0;
	pPanel->win_dim.r = pPanel->disp_info->width-1;
	pPanel->win_dim.t = 0;
	pPanel->win_dim.b = pPanel->disp_info->height-1;
	pPanel->win_dim.w = pPanel->disp_info->width;
	pPanel->win_dim.h = pPanel->disp_info->height;

	pPanel->drvState   = DRV_STATE_OPEN;

	LCD_DBG(LCD_DBG_INIT_ID, "[DISPDRV] %s:	OK\n\r", __FUNCTION__);

	return (res);
}

/******************************************************************************************************
 *
 *  Function Name: LCD_DISPDRV_Close
 *
 *  Description:   disp	bus OFF
 *
 ******************************************************************************************************/
Int32 LCD_DISPDRV_Close(DISPDRV_HANDLE_T drvH)
{
	Int32			res = 0;
	LCD_DISPDRV_PANEL_t	*pPanel	= (LCD_DISPDRV_PANEL_t *)drvH;

	if (CSL_DSI_CloseCmVc(pPanel->dsiCmVcHandle)) {
		LCD_DBG(LCD_DBG_ERR_ID,	"[DISPDRV] %s: ERROR,	"
			"Closing Command Mode Handle\n\r", __FUNCTION__);
		return -1;
	}

	if (CSL_DSI_CloseClient(pPanel->clientH) != CSL_LCD_OK)	{
		LCD_DBG(LCD_DBG_ERR_ID,	"[DISPDRV] %s:	ERROR, Closing "
			"DSI Client\n", __FUNCTION__);
		return -1;
	}

	if (CSL_DSI_Close(pPanel->busNo) != CSL_LCD_OK)	{
		LCD_DBG(LCD_DBG_ERR_ID,	"[DISPDRV] %s: ERR Closing DSI "
			"Controller\n",	__FUNCTION__);
		return -1;
	}

	if (pPanel->isTE)
		LCD_DISPDRV_TeOff(pPanel);

	if (brcm_disable_dsi_pll_clocks(pPanel->busNo))	{
		LCD_DBG(LCD_DBG_ERR_ID,	"[DISPDRV] %s: ERROR to	disable	"
			"the pll clock\n", __FUNCTION__);
		return -1;
	}

	pPanel->pwrState = STATE_PWR_OFF;
	pPanel->drvState = DRV_STATE_INIT;
	LCD_DBG(LCD_DBG_INIT_ID, "[DISPDRV] %s:	OK\n\r", __FUNCTION__);

	return (res);
}

/******************************************************************************************************
 *
 *   Function Name: LCD_DISPDRV_PowerControl
 *
 *   Description:	  Display Module Control
 *
 ******************************************************************************************************/
Int32 LCD_DISPDRV_PowerControl(DISPDRV_HANDLE_T drvH,
				DISPLAY_POWER_STATE_T state)
{
	Int32  res = 0;
	LCD_DISPDRV_PANEL_t *pPanel = (LCD_DISPDRV_PANEL_t *)drvH;

	switch (state) {
	case CTRL_PWR_ON:
		switch (pPanel->pwrState) {
		case STATE_PWR_OFF:
			LCD_DISPDRV_ExecCmndList(drvH, hx8369a_dsi_init);

			LCD_DISPDRV_WinSet(drvH, TRUE, &pPanel->win_dim);

			pPanel->pwrState = STATE_SCREEN_OFF;
			LCD_DBG(LCD_DBG_INIT_ID, "[DISPDRV]	%s: INIT-SEQ\n",
				__FUNCTION__);
			break;
		default:
			LCD_DBG(LCD_DBG_ERR_ID,	"[DISPDRV] %s:	POWER ON "
				"Requested While Not In	POWER DOWN State\n",
				__FUNCTION__);
			break;
		}
		break;

	case CTRL_PWR_OFF:
		if (pPanel->pwrState != STATE_PWR_OFF) {
			LCD_DISPDRV_WrCmndP0(drvH, MIPI_DCS_SET_DISPLAY_OFF);
			LCD_DISPDRV_WrCmndP0(drvH, MIPI_DCS_ENTER_SLEEP_MODE);
			OSTASK_Sleep(120);
			LCD_DISPDRV_reset(drvH);

			pPanel->pwrState = STATE_PWR_OFF;

			LCD_DBG(LCD_DBG_INIT_ID, "[DISPDRV]	%s: PWR	DOWN\n\r",
				__FUNCTION__);
		}
		break;

	case CTRL_SLEEP_IN:
		switch (pPanel->pwrState) {
		case STATE_SCREEN_ON:
			LCD_DISPDRV_WrCmndP0(drvH, MIPI_DCS_SET_DISPLAY_OFF);
		case STATE_SCREEN_OFF:
			LCD_DISPDRV_WrCmndP0(drvH, MIPI_DCS_ENTER_SLEEP_MODE);
			OSTASK_Sleep(120);

			pPanel->pwrState = STATE_SLEEP;
			LCD_DBG(LCD_DBG_INIT_ID, "[DISPDRV]	%s: SLEEP-IN\n\r",
			    __FUNCTION__);
			break;
		default:
			LCD_DBG(LCD_DBG_ERR_ID,	"[DISPDRV] %s:	"
				"SLEEP Requested, But Not In "
				"DISP ON|OFF State\n\r", __FUNCTION__);
			break;
		}
		break;

	case CTRL_SLEEP_OUT:
		switch (pPanel->pwrState) {
		case STATE_SLEEP:
			LCD_DISPDRV_WrCmndP0(drvH, MIPI_DCS_EXIT_SLEEP_MODE);
			OSTASK_Sleep(120);
			
			pPanel->pwrState = STATE_SCREEN_OFF;

			LCD_DBG(LCD_DBG_INIT_ID, "[DISPDRV]	%s: SLEEP-OUT\n\r",
				__FUNCTION__);
			break;
		default:
			LCD_DBG(LCD_DBG_ERR_ID,	"[DISPDRV] %s:	"
				"SLEEP-OUT Req While Not In SLEEP State\n",
				__FUNCTION__);
			break;
		}
		break;

	case CTRL_SCREEN_ON:
		switch (pPanel->pwrState) {
		case STATE_SCREEN_OFF:
			LCD_DISPDRV_WrCmndP0(drvH, MIPI_DCS_SET_DISPLAY_ON);
			pPanel->pwrState = STATE_SCREEN_ON;
			break;
		default:
			LCD_DBG(LCD_DBG_ERR_ID,	"[DISPDRV] %s:	"
				"SCREEN	ON Req While Not In SCREEN OFF State\n",
				__FUNCTION__);
			break;
		}
		break;
	case CTRL_SCREEN_OFF:
		switch (pPanel->pwrState) {
		case STATE_SCREEN_ON:
			LCD_DISPDRV_WrCmndP0(drvH, MIPI_DCS_SET_DISPLAY_OFF);
			pPanel->pwrState = STATE_SCREEN_OFF;
			break;
		default:
			LCD_DBG(LCD_DBG_ERR_ID,	"[DISPDRV] %s:	"
				"SCREEN	OFF Req	While Not In SCREEN ON State\n",
				__FUNCTION__);
			break;
		}
		break;

	default:
		LCD_DBG(LCD_DBG_ERR_ID,	"[DISPDRV] %s: Invalid Power "
			"State[%d] Requested\n", __FUNCTION__, state);
		res = -1;
		break;
	}
	return (res);
}

/******************************************************************************************************
 *
 *  Function Name: LCD_DISPDRV_Start
 *
 *  Description:
 *
 ******************************************************************************************************/
Int32 LCD_DISPDRV_Start(DISPDRV_HANDLE_T drvH, struct pi_mgr_dfs_node *dfs_node)
{
	LCD_DISPDRV_PANEL_t *pPanel = (LCD_DISPDRV_PANEL_t *)drvH;

	if (brcm_enable_dsi_lcd_clocks(dfs_node, pPanel->busNo,
					LCD_DISPDRV_dsiCfg.hsBitClk.clkIn_MHz * 
					1000000,
					LCD_DISPDRV_dsiCfg.hsBitClk.clkInDiv,
					LCD_DISPDRV_dsiCfg.escClk.clkIn_MHz *
					1000000 /
					LCD_DISPDRV_dsiCfg.escClk.clkInDiv)) {

		LCD_DBG(LCD_DBG_ERR_ID,
			"[DISPDRV] %s: ERROR to enable the clock\n",
			__FUNCTION__);
		return (-1);
	}

	return (0);
}

/******************************************************************************************************
 *
 *   Function Name: LCD_DISPDRV_Stop
 *
 *   Description:
 *
 ******************************************************************************************************/
Int32 LCD_DISPDRV_Stop(DISPDRV_HANDLE_T	drvH, struct pi_mgr_dfs_node *dfs_node)
{
	LCD_DISPDRV_PANEL_t *pPanel = (LCD_DISPDRV_PANEL_t *)drvH;

	if (brcm_disable_dsi_lcd_clocks(dfs_node, pPanel->busNo)) {
		LCD_DBG(LCD_DBG_ERR_ID, "[DISPDRV] %s: ERROR to enable "
			"the clock\n", __FUNCTION__);
	    return (-1);
	}
	return	(0);
}

/******************************************************************************************************
 *
 *   Function Name: LCD_DISPDRV_GetInfo
 *
 *   Description:
 *
 *******************************************************************************************************/
const DISPDRV_INFO_T *LCD_DISPDRV_GetDispDrvData(DISPDRV_HANDLE_T drvH)
{
	return (&HX8369A_Info);
}

/******************************************************************************************************
 *
 *  Function Name: LCD_DISPDRV_GetDispDrvFeatures
 *
 *  Description:
 *
 ******************************************************************************************************/
Int32 LCD_DISPDRV_GetDispDrvFeatures(DISPDRV_HANDLE_T drvH,
					const char **driver_name,
					UInt32 *version_major,
					UInt32 *version_minor,
					DISPDRV_SUPPORT_FEATURES_T	*flags)
{
	Int32 res = -1;

	if ((NULL != driver_name)   && (NULL !=	version_major) &&
	    (NULL != version_minor) && (NULL !=	flags))	{

		*driver_name   = *LCD_DISPDRV_panel_name;
		*version_major = 0;
		*version_minor = 15;
		*flags = DISPDRV_SUPPORT_NONE;
		res = 0;
	}
	return (res);
}

/******************************************************************************************************
 *
 *  Function Name: LCD_DISPDRV_Cb
 *
 *  Description:   CSL callback
 *
 ******************************************************************************************************/
static void LCD_DISPDRV_Cb(CSL_LCD_RES_T cslRes, pCSL_LCD_CB_REC pCbRec)
{
	DISPDRV_CB_RES_T apiRes;

	LCD_DBG(LCD_DBG_ID, "[DISPDRV]	+%s\r\n", __FUNCTION__);

	if (pCbRec->dispDrvApiCb != NULL) {
		switch (cslRes)	{
		case CSL_LCD_OK:
			apiRes = DISPDRV_CB_RES_OK;
			break;
		default:
			apiRes = DISPDRV_CB_RES_ERR;
			break;
		}
		((DISPDRV_CB_T)pCbRec->dispDrvApiCb) (apiRes);
	}

	LCD_DBG(LCD_DBG_ID, "[DISPDRV] -%s\r\n", __FUNCTION__);
}


/*******************************************************************************************************
 *
 *  Function Name: LCD_DISPDRV_Update
 *
 *  Description:
 *
 *******************************************************************************************************/
Int32 LCD_DISPDRV_Update(DISPDRV_HANDLE_T drvH,
			void *buff,
			DISPDRV_WIN_t *p_win, DISPDRV_CB_T apiCb)
{
	LCD_DISPDRV_PANEL_t	*pPanel	= (LCD_DISPDRV_PANEL_t *)drvH;
	CSL_LCD_UPD_REQ_T req;
	Int32 res  = 0;
	Boolean use_te;

	LCD_DBG(LCD_DBG_ID, "[DISPDRV]	+%s\r\n", __FUNCTION__);

	if (pPanel->pwrState ==	STATE_PWR_OFF) {
		LCD_DBG(LCD_DBG_ERR_ID,	"[DISPDRV] %s: Skip Due	To "
			"Power State\n", __FUNCTION__);
		return -1;
	}

	if (p_win == NULL)
		use_te = FALSE;
	else
		use_te = TRUE;
	p_win =	&pPanel->win_dim;

	LCD_DISPDRV_WinSet(drvH, TRUE, p_win);

	req.buff = buff;
	req.lineLenP = p_win->w;
	req.lineCount = p_win->h;
	req.buffBpp = pPanel->disp_info->Bpp;
	req.timeOut_ms = 100;

	LCD_DBG(LCD_DBG_ID, "%s: buf=%08x, linelenp = %lu, linecnt =%lu\n",
		__FUNCTION__, (u32)req.buff, req.lineLenP, req.lineCount);

	req.cslLcdCbRec.cslH = pPanel->clientH;
	req.cslLcdCbRec.dispDrvApiCbRev	= DISP_DRV_CB_API_REV_1_0;
	req.cslLcdCbRec.dispDrvApiCb = (void	*) apiCb;
	req.cslLcdCbRec.dispDrvApiCbP1= NULL;

	if (apiCb != NULL)
		req.cslLcdCb = LCD_DISPDRV_Cb;
	else
		req.cslLcdCb = NULL;

	if (CSL_DSI_UpdateCmVc(pPanel->dsiCmVcHandle, &req, use_te)
		!= CSL_LCD_OK)	{
		LCD_DBG(LCD_DBG_ERR_ID,	"[DISPDRV] %s:	ERROR ret by "
			"CSL_DSI_UpdateCmVc\n\r", __FUNCTION__);
		res = -1;
	}

	LCD_DBG(LCD_DBG_ID, "[DISPDRV]	-%s\r\n", __FUNCTION__);

	return (res);
}
