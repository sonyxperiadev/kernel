/*******************************************************************************
* Copyright 2011 Broadcom Corporation.	All rights reserved.
*
* @file drivers/video/broadcom/dsi/dsi.c
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

#include <linux/string.h>
#include <linux/broadcom/mobcom_types.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/kobject.h>
#include <linux/platform_device.h>
#include <plat/osabstract/osinterrupt.h>
#include <plat/osabstract/ostask.h>
#include <plat/csl/csl_dma_vc4lite.h>
#include <plat/dma_drv.h>
#include <plat/pi_mgr.h>
#include "../lcd/display_drv.h"	   /* dispdrv interface */
#include <plat/csl/csl_lcd.h>	   /* dispdrv CSL Common Interface */
#include <plat/csl/csl_dsi.h>	   /* DSI CSL API */
#include "../lcd/dispdrv_mipi_dcs.h"	   /* MIPI DCS */
#include "../lcd/dispdrv_mipi_dsi.h"	   /* MIPI DSI */

#include "../lcd/dispdrv_common.h"	   /* dispdrv common */
#include "../lcd/lcd_clock.h"
#include <plat/csl/csl_tectl_vc4lite.h>
#include <linux/regulator/consumer.h>
#include <video/kona_fb.h>

#define DSI_ERR(fmt, args...) \
	printk(KERN_ERR "%s:%d " fmt, __func__, __LINE__, ##args)
#if 1
#define DSI_INFO(fmt, args...) \
	printk(KERN_DEBUG "%s:%d " fmt, __func__, __LINE__, ##args)
#else
#define DSI_INFO(fmt, args...)
#endif
#if 0
#define DSI_DBG(fmt, args...) \
	printk(KERN_DEBUG "%s:%d " fmt, __func__, __LINE__, ##args)
#else
#define DSI_DBG(fmt, args...)
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
	struct pi_mgr_dfs_node *dfs_node;
	/* --- */
	CSL_DSI_CM_VC_t *cmnd_mode;
	CSL_DSI_CFG_t *dsi_cfg;
	DISPDRV_INFO_T		*disp_info;
	bool panel_identified;
	UInt8 maxRetPktSize;
	bool send_first_frame_event;
	struct platform_device *pdev;
} DispDrv_PANEL_t;

#if 0
/* LOCAL FUNCTIONs */
static void DispDrv_WrCmndP0(
	DISPDRV_HANDLE_T drvH,
	UInt32 reg);
#endif

/* DRV INTERFACE FUNCTIONs */
static Int32 DSI_Init(DISPDRV_INFO_T *, DISPDRV_HANDLE_T *, struct platform_device *);

static Int32 DSI_Exit(DISPDRV_HANDLE_T drvH);

static Int32 DSI_Open(DISPDRV_HANDLE_T drvH);

static Int32 DSI_Close(DISPDRV_HANDLE_T drvH);

static void DSI_ExecCmndList(DispDrv_PANEL_t *pPanel, char *buff);

static Int32 DSI_SuspendLink(DISPDRV_HANDLE_T drvH);

static Int32 DSI_ResumeLink(DISPDRV_HANDLE_T drvH);

static Int32 DSI_Start(
	DISPDRV_HANDLE_T drvH,
	struct pi_mgr_dfs_node *dfs_node);

static Int32 DSI_Stop(
	DISPDRV_HANDLE_T drvH,
	struct pi_mgr_dfs_node *dfs_node);

static Int32 DSI_PowerControl(
	DISPDRV_HANDLE_T drvH,
	DISPLAY_POWER_STATE_T state);

static Int32 DSI_Atomic_Update(
	DISPDRV_HANDLE_T drvH,
	void			*buff,
	DISPDRV_WIN_t		*p_win);

static Int32 DSI_Update(
	 DISPDRV_HANDLE_T drvH,
	 void			*buff,
	 DISPDRV_WIN_t		*p_win,
	 DISPDRV_CB_T apiCb);

static Int32 DSI_WinReset(DISPDRV_HANDLE_T drvH);

static void dsi_enable_send_first_frame_event(DISPDRV_HANDLE_T drvH);

//static int DSI_ReadReg(DISPDRV_HANDLE_T drvH, UInt8 reg, UInt8 *rxBuff);

static Int32 DSI_DCS_Read(DispDrv_PANEL_t *pPanel,
		UInt8 reg, UInt8 *rxBuff, UInt8 buffLen);

//static int DSI_ReadPanelID(DispDrv_PANEL_t *pPanel);
void panel_write(UInt8 *buff);
void panel_read(UInt8 reg, UInt8 *rxBuff, UInt8 buffLen);
extern void panel_initialize(char *init_seq);
extern void panel_read_id(void);

static DISPDRV_T disp_drv_dsi = {
	.init = &DSI_Init,
	.exit = &DSI_Exit,
	.open = &DSI_Open,
	.close = &DSI_Close,
	.suspend_link = &DSI_SuspendLink,
	.resume_link = &DSI_ResumeLink,
	.start = &DSI_Start,
	.stop = &DSI_Stop,
	.power_control = &DSI_PowerControl,
	.update_no_os = &DSI_Atomic_Update,
	.update = &DSI_Update,
	.reset_win = &DSI_WinReset,
	.enable_send_first_frame_event = &dsi_enable_send_first_frame_event,
};


/* DSI Command Mode VC Configuration */
CSL_DSI_CM_VC_t DispDrv_VCCmCfg = {
	.vc = 0,
	.dsiCmnd = DSI_DT_LG_DCS_WR,
	.dcsCmndStart = MIPI_DCS_WRITE_MEMORY_START,
	.dcsCmndCont = MIPI_DCS_WRITE_MEMORY_CONTINUE,
};

/* DSI BUS CONFIGURATION */
CSL_DSI_CFG_t DispDrv_dsiCfg = {
	.bus = 0,
	.enaRxCrc = TRUE,
	.enaRxEcc = TRUE,
	.enaHsTxEotPkt = TRUE,
	.enaLpTxEotPkt = FALSE,
	.enaLpRxEotPkt = FALSE,
};


static struct regulator *disp_reg;

static DispDrv_PANEL_t panel[1] = {
	[0] = {
		.pwrState = STATE_PWR_OFF,
	},
};

/*###########################################################################*/


/* Template for Video mode commands like Turn On Peripheral, Shutdown etc. */
static int DSI_panel_shut_down(DispDrv_PANEL_t *pPanel)
{
	CSL_DSI_CMND_t msg;
	int res = 0;

	msg.dsiCmnd = DSI_DT_SH_SHUT_DOWN;
	msg.msg	       = NULL;
	msg.msgLen     = 0;
	msg.vc	       = pPanel->cmnd_mode->vc;
	msg.isLP       = pPanel->disp_info->cmnd_LP;
	msg.isLong     = FALSE;
	msg.endWithBta = FALSE;
	msg.reply = NULL;

	res = CSL_DSI_SendPacket(pPanel->clientH, &msg, FALSE);
	return res;
}

static int DSI_panel_turn_on(DispDrv_PANEL_t *pPanel)
{
	CSL_DSI_CMND_t msg;
	int res = 0;

	msg.dsiCmnd = DSI_DT_SH_TURN_ON;
	msg.msg	       = NULL;
	msg.msgLen     = 0;
	msg.vc	       = pPanel->cmnd_mode->vc;
	msg.isLP       = pPanel->disp_info->cmnd_LP;
	msg.isLong     = FALSE;
	msg.endWithBta = FALSE;
	msg.reply = NULL;

	res = CSL_DSI_SendPacket(pPanel->clientH, &msg, FALSE);
	return res;
}

/*
 * Read panel ID from register 0xDA, 0xDB and 0xDC
 *
 * ID list must end with DISPCTRL_LIST_END
 */
static int DSI_ReadPanelIDs(DispDrv_PANEL_t *pPanel)
{
	int rc = 0;
	char *p;
	int buff_len = 1;
	UInt8 rd_buff;
	uint8_t cmd;
	uint8_t reg;
	uint8_t data;

	p = pPanel->disp_info->id_seq;
	if (!p)
		goto exit;
	cmd = *p++;

	while (cmd != DISPCTRL_LIST_END) {
		reg = *p++;
		data = *p++;
		rc = DSI_DCS_Read(pPanel, reg, &rd_buff, buff_len);
		DSI_DBG("cmd:0x%x, reg:0x%x, data=0x%x rd_buff=0x%x\n",
						cmd, reg, data, rd_buff);
		if (data != rd_buff) {
			DSI_ERR("cmd:0x%x, reg:0x%x, data=0x%x rd_buff=0x%x\n",
						cmd, reg, data, rd_buff);
			rc = -1;
			break;
		}
		cmd = *p++;
	}
exit:
	return rc;
}

//static int DSI_ReadPanelID(DispDrv_PANEL_t *pPanel)
//{
//	int ret = 0;
//	char *buff_seq;
//	UInt8 rd_buff[2];
//
//	buff_seq = pPanel->disp_info->id_seq;
//	if (!buff_seq)
//		goto done;
//
//	/* Currently we are reading 1 byte, multi-byte support is to be added */
//	while (*buff_seq) {
//		uint8_t len = *buff_seq++;
//		if (len != 2) {
//			DSI_ERR("Reading multiple bytes not supported, len=%d\n"
//				, len);
//			ret = -1;
//			break;
//		}
//
//		ret = DSI_ReadReg(pPanel, *buff_seq++, rd_buff);
//		/*
//		DSI_ERR("Reg=0x%x ret size=%d buff[0]=0x%x, buff[1]=0x%x\n",
//			DispDrv_ID[count].reg, ret, buff[0], buff[1]);
//		*/
//		if (ret < 0)
//			break;
//		if ((ret == 0) || (*buff_seq++ != rd_buff[0])) {
//			DSI_ERR("ret=%d rd_buff[0]=0x%x\n", ret, rd_buff[0]);
//			ret = -1;
//			break;
//		}
//	}
//done:
//	return ret;
//}

/*
 *
 *   Function Name: DSI_TeOn
 *
 *   Description:   Configure TE Input Pin & Route it to DSI Controller Input
 *
 */
static int DSI_TeOn(DispDrv_PANEL_t	*pPanel)
{
	Int32 res	= 0;

	TECTL_CFG_t teCfg;

	teCfg.te_mode	  = TE_VC4L_MODE_VSYNC;
	teCfg.sync_pol	  = TE_VC4L_ACT_POL_HI;
	teCfg.vsync_width = 0;
	teCfg.hsync_line  = 0;

	res = CSL_TECTL_VC4L_OpenInput(pPanel->teIn, pPanel->teOut, &teCfg);

	return res;
}

/*
 *
 *   Function Name: DSI_TeOff
 *
 *   Description:   'Release' TE Input Pin Used
 *
 */
static int DSI_TeOff(DispDrv_PANEL_t *pPanel)
{
	Int32  res = 0;

	res = CSL_TECTL_VC4L_CloseInput(pPanel->teIn);

	return res;
}


/*
 *
 *   Function Name: DSI_ReadReg
 *
 *   Description:   DSI Read Reg
 *
 */
//static int DSI_ReadReg(DISPDRV_HANDLE_T drvH, UInt8 reg, UInt8 *rxBuff)
//{
//	DispDrv_PANEL_t	 *pPanel = (DispDrv_PANEL_t *)drvH;
//	CSL_DSI_CMND_t msg;
//	CSL_DSI_REPLY_t rxMsg;      /* DSI RX message */
//	UInt8 txData[1];  /* DCS Rd Command */
//	Int32 res = 0;
//	CSL_LCD_RES_T cslRes;
//
//	msg.dsiCmnd    = DSI_DT_SH_DCS_RD_P0;
//	msg.msg	= &txData[0];
//	msg.msgLen     = 1;
//	msg.vc	 = pPanel->cmnd_mode->vc;
//	msg.isLP       = FALSE;
//	msg.isLong     = FALSE;
//	msg.endWithBta = TRUE;
//
//	rxMsg.pReadReply = (UInt8 *)rxBuff;
//	msg.reply	= (CSL_DSI_REPLY_t *)&rxMsg;
//
//	txData[0] = reg;
//	cslRes = CSL_DSI_SendPacket(pPanel->clientH, &msg, FALSE);
//	if ((cslRes != CSL_LCD_OK) ||
//		((rxMsg.type & DSI_RX_TYPE_READ_REPLY) == 0)) {
//
//		DSI_ERR("Reading From Reg[0x%08X]\n", reg);
//		res = -1;
//	} else {
//		/*
//		DSI_ERR("OK Reg[0x%08X] Val[0x%08X][0x%08X] Size=[0x%08X]\n",
//				reg, rxBuff[1], rxBuff[0], rxMsg.readReplySize);
//		*/
//		res = rxMsg.readReplySize;
//	}
//	return res;
//}


/*
 *
 *   Function Name: DSI_WinSet
 *
 *   Description:   Set Window
 *
 */
static Int32 DSI_WinSet(
	DISPDRV_HANDLE_T drvH,
	Boolean update,
	DISPDRV_WIN_t *p_win)
{
	DispDrv_PANEL_t *pPanel = (DispDrv_PANEL_t *) drvH;
	DISPDRV_INFO_T *info = pPanel->disp_info;

	if (info->vmode)
		goto done_vid;

	if ((pPanel->win_cur.l != p_win->l) ||
	    (pPanel->win_cur.r != p_win->r) ||
	    (pPanel->win_cur.t != p_win->t) ||
	    (pPanel->win_cur.b != p_win->b)) {
		pPanel->win_cur	= *p_win;

		if (update) {
			info->updt_win_fn(info->win_seq, p_win);
			DSI_ExecCmndList(pPanel, info->win_seq);
		}
	}
done_vid:
	return 0;
}

/*
 *
 *   Function Name: DSI_WinReset
 *
 *   Description:   Reset windowing to full screen size.
 *		    Typically, only used in boot code environment
 */
static Int32 DSI_WinReset(DISPDRV_HANDLE_T drvH)
{
	Int32 res;
	DispDrv_PANEL_t *pPanel = (DispDrv_PANEL_t *) drvH;

	res = DSI_WinSet(drvH, TRUE, &pPanel->win_dim);
	return res;
}

/*
 * Set flag that we need to send uevent on the next frame update.
 * Call to this function is protected in kona_fb.c by update-mutex,
 * so there is no need to protect it here.
 */
static void dsi_enable_send_first_frame_event(DISPDRV_HANDLE_T handle)
{
	DispDrv_PANEL_t *panel = (DispDrv_PANEL_t *)handle;

	panel->send_first_frame_event = true;
}

/*
 *
 *   Function Name: hw_reset
 *
 *   Description:   (De)Assert display reset
 *
 */
static void hw_reset(DISPDRV_HANDLE_T drvH, Boolean on)
{
	DispDrv_PANEL_t *pPanel = (DispDrv_PANEL_t *) drvH;
	struct hw_rst_info *rst = pPanel->disp_info->rst;

	int reset_active = rst->active ? 1 : 0;

	if (!on) {
		DSI_INFO("Resetting the panel gpio=%d\n",
			rst->gpio);
		gpio_direction_output(rst->gpio, !reset_active);
		udelay(rst->setup);
		gpio_set_value_cansleep(rst->gpio, reset_active);
		udelay(rst->pulse);
		gpio_set_value_cansleep(rst->gpio, !reset_active);
		usleep_range(rst->hold, rst->hold + 10);
	} else {
		DSI_INFO("Powering off the panel\n");
		gpio_set_value_cansleep(rst->gpio, reset_active);
	}
}

/*
 *
 *   Function Name: LCD_DRV_DispDrv_GetDrvInfo
 *
 *   Description:   Get Driver Funtion Table
 *
 */
DISPDRV_T *DISP_DRV_GetFuncTable(void)
{
	return &disp_drv_dsi;
}

/*
 *
 *   Function Name: DSI_Init
 *
 *   Description:   Setup / Verify display driver init interface
 *
 */
static Int32 DSI_Init(DISPDRV_INFO_T *info, DISPDRV_HANDLE_T *handle, struct platform_device *pdev)
{
	Int32 res = 0;
	DispDrv_PANEL_t	*pPanel;

	pPanel = &panel[0];
	pPanel->pdev = pdev;

	if (g_display_enabled)
		DSI_ERR("Skipping hardware initialisation\n");

	if (pPanel->drvState ==	DRV_STATE_OFF)	{
		pPanel->cmnd_mode = &DispDrv_VCCmCfg;
		/* Pixel transfer is always in HS mode*/
		DispDrv_VCCmCfg.isLP = FALSE;
		DispDrv_VCCmCfg.teCfg.teInType = info->vmode ? DSI_TE_NONE :
						DSI_TE_CTRLR_INPUT_0;
		switch (info->in_fmt) {
		case DISPDRV_FB_FORMAT_RGB565:
			DispDrv_VCCmCfg.cm_in = LCD_IF_CM_I_RGB565P;
			switch (info->out_fmt) {
			case DISPDRV_FB_FORMAT_RGB565:
				DispDrv_VCCmCfg.cm_out =
						LCD_IF_CM_O_RGB565;
				break;
			case DISPDRV_FB_FORMAT_RGB666P:
				DispDrv_VCCmCfg.cm_out =
						LCD_IF_CM_O_RGB666;
				break;
			case DISPDRV_FB_FORMAT_RGB666U:
				DispDrv_VCCmCfg.cm_out =
						LCD_IF_CM_O_RGB666U;
				break;
			default:
				DSI_ERR("Invalid output colour format!.");
				DispDrv_VCCmCfg.cm_out =
						LCD_IF_CM_O_RGB565;
				DSI_ERR("Changing to RGB565\n");
			}
			break;
		case DISPDRV_FB_FORMAT_xRGB8888:
			DispDrv_VCCmCfg.cm_in = LCD_IF_CM_I_xRGB8888;
			DispDrv_VCCmCfg.cm_out = LCD_IF_CM_O_xRGB8888;
			break;
		case DISPDRV_FB_FORMAT_xBGR8888:
			DispDrv_VCCmCfg.cm_in = LCD_IF_CM_I_xBGR8888;
			DispDrv_VCCmCfg.cm_out = LCD_IF_CM_O_xRGB8888;
			break;
		default:
			DispDrv_VCCmCfg.cm_in = LCD_IF_CM_I_xBGR8888;
			DispDrv_VCCmCfg.cm_out = LCD_IF_CM_O_xRGB8888;
			DSI_ERR("Unknown format %d\n", info->in_fmt);
			break;
		}

#define DSI_1MHZ (1000 * 1000)

		pPanel->dsi_cfg = &DispDrv_dsiCfg;
		DispDrv_dsiCfg.dlCount = info->lanes;
		DispDrv_dsiCfg.phy_timing = info->phy_timing;

		DispDrv_dsiCfg.escClk.clkIn_MHz = 500;
		DispDrv_dsiCfg.escClk.clkInDiv = 5;
		DispDrv_dsiCfg.hsBitClk.clkIn_MHz = info->hs_bps / DSI_1MHZ;
		WARN_ON(DispDrv_dsiCfg.hsBitClk.clkIn_MHz > 2400);
		while (DispDrv_dsiCfg.hsBitClk.clkIn_MHz < 600)
			DispDrv_dsiCfg.hsBitClk.clkIn_MHz *= 2;
		DispDrv_dsiCfg.hsBitClk.clkInDiv =
			(DispDrv_dsiCfg.hsBitClk.clkIn_MHz * DSI_1MHZ) /
			info->hs_bps;
		DispDrv_dsiCfg.lpBitRate_Mbps = info->lp_bps / DSI_1MHZ;

		DispDrv_dsiCfg.dispEngine = info->vmode ? 0 : 1;
		/* Default to AXIPV even for Command mode */
		DispDrv_dsiCfg.pixTxporter = info->vmode ? 0 : 0;
		DispDrv_dsiCfg.vmode = info->vmode;
		DispDrv_dsiCfg.vs = info->vs;
		DispDrv_dsiCfg.vbp = info->vbp;
		DispDrv_dsiCfg.vfp = info->vfp;
		DispDrv_dsiCfg.hs = info->hs;
		DispDrv_dsiCfg.hbp = info->hbp;
		DispDrv_dsiCfg.hfp = info->hfp;
		DispDrv_dsiCfg.hbllp = info->hbllp;
		DispDrv_dsiCfg.vsync_cb = info->vsync_cb;
		DispDrv_dsiCfg.enaContClock = info->cont_clk;
		DispDrv_dsiCfg.enaStEnd = info->sync_pulses;

		pPanel->disp_info = info;
		pPanel->isTE = info->vmode ? false : info->te_ctrl;
		pPanel->maxRetPktSize = 0;

		/* get TE pin configuration */
		pPanel->teIn  =	TE_VC4L_IN_1_DSI0;
		pPanel->teOut =	TE_VC4L_OUT_DSI0_TE0;

		pPanel->drvState = DRV_STATE_INIT;
		if (!g_display_enabled)
			brcm_init_lcd_clocks(pPanel->busNo);
		else
			pPanel->pwrState = STATE_SCREEN_ON;

		*handle	= (DISPDRV_HANDLE_T)pPanel;

		DSI_INFO("OK\n");
	} else {
		DSI_ERR("Not in OFF state\n");
		res = -1;
	}

	return res;
}

/*
 *
 *   Function Name: DSI_Exit
 *
 *   Description:
 *
 */
static Int32 DSI_Exit(DISPDRV_HANDLE_T drvH)
{
	DispDrv_PANEL_t *pPanel;

	pPanel = (DispDrv_PANEL_t *) drvH;
	pPanel->drvState = DRV_STATE_OFF;
	return 0;
}

/*
 *
 *  Function Name: DSI_ResumeLink
 *
 *  Description:   Start/Resume DSI link
 *
 */
static Int32 DSI_ResumeLink(DISPDRV_HANDLE_T drvH)
{
	Int32 res = 0;
	DispDrv_PANEL_t *pPanel;

	pPanel = (DispDrv_PANEL_t *) drvH;
	if (brcm_enable_dsi_pll_clocks(pPanel->busNo,
			pPanel->dsi_cfg->hsBitClk.clkIn_MHz * 1000000,
			pPanel->dsi_cfg->hsBitClk.clkInDiv,
			pPanel->disp_info->desense_offset,
			pPanel->dsi_cfg->escClk.clkIn_MHz * 1000000 /
				pPanel->dsi_cfg->escClk.clkInDiv)) {
		DSI_ERR("ERROR enabling clock\n");
	}

	if (pPanel->isTE && DSI_TeOn(pPanel) == -1) {
		DSI_ERR("Failed to Configure TE Input\n");
		goto err_te_on;
	}

	if (CSL_DSI_Init(pPanel->dsi_cfg) != CSL_LCD_OK) {
		DSI_ERR("DSI CSL Init Failed\n");
		goto err_dsi_init;
	}

	if (CSL_DSI_OpenClient(pPanel->busNo, &pPanel->clientH) != CSL_LCD_OK) {
		DSI_ERR("CSL_DSI_OpenClient Failed\n");
		goto err_dsi_open_cl;
	}

	if (CSL_DSI_Ulps(pPanel->clientH, FALSE) != CSL_LCD_OK) {
		DSI_ERR("Exit DSI Ulps Failed\n");
		goto err_dsi_ulps;
	}

	if (CSL_DSI_OpenCmVc(pPanel->clientH,
		pPanel->cmnd_mode, &pPanel->dsiCmVcHandle) != CSL_LCD_OK) {

		DSI_ERR("CSL_DSI_OpenCmVc Failed\n");
		goto err_dsi_open_cm;
	}

	if (!pPanel->dsi_cfg->dispEngine && pPanel->dsi_cfg->pixTxporter) {
		DSI_ERR("Cannot transfer pixels via MMDMA to DispEngine 0.");
		DSI_ERR("Default to Display Engine 1\n");
		pPanel->dsi_cfg->dispEngine = 1;
	}

	if (pPanel->dsi_cfg->dispEngine && pPanel->dsi_cfg->pixTxporter) {
		if (csl_dma_vc4lite_init() != DMA_VC4LITE_STATUS_SUCCESS) {
			DSI_ERR("csl_dma_vc4lite_init Failed\n");
			goto err_dma_init;
		}
	}

	return res;

err_dma_init:
	CSL_DSI_CloseCmVc(pPanel->dsiCmVcHandle);
err_dsi_open_cm:
err_dsi_ulps:
	CSL_DSI_CloseClient(pPanel->clientH);
err_dsi_open_cl:
	CSL_DSI_Close(pPanel->busNo);
err_dsi_init:
	if (pPanel->isTE)
		DSI_TeOff(pPanel);
err_te_on:
	return -1;
}

/*
 *
 *  Function Name: DSI_Open
 *
 *  Description:   disp bus ON
 *
 */
static Int32 DSI_Open(DISPDRV_HANDLE_T drvH)
{
	Int32 res = 0;
	DispDrv_PANEL_t	*pPanel;

	pPanel = (DispDrv_PANEL_t *) drvH;

	DSI_INFO("enter\n");

	if (pPanel->drvState !=	DRV_STATE_INIT)	{
		DSI_ERR("ERROR State != Init\n");
		return -1;
	}

	res = DSI_ResumeLink(drvH);
	if (res < 0)
		goto err_resume_link;

	if (!disp_reg) {
		/*CAM2 LDO */
		disp_reg = regulator_get(NULL, pPanel->disp_info->reg_name);
		if (IS_ERR_OR_NULL(disp_reg)) {
			DSI_ERR("Failed to get disp_reg\n");
			goto err_reg_init;
		}
		if (g_display_enabled) {
			/* Increase the ref_count, disp_reg power on already */
			res = regulator_enable(disp_reg);
			if (res) {
				DSI_ERR("failed to enable regulator\n");
				goto err_reg_enable;
			}
		}
	}
	res = gpio_request(pPanel->disp_info->rst->gpio, "LCD_RST");
	if (res < 0) {
		DSI_ERR("gpio_request failed %ld\n", res);
		goto err_gpio_request;
	}

	pPanel->win_dim.l = 0;
	pPanel->win_dim.r = pPanel->disp_info->width-1;
	pPanel->win_dim.t = 0;
	pPanel->win_dim.b = pPanel->disp_info->height-1;
	pPanel->win_dim.w = pPanel->disp_info->width;
	pPanel->win_dim.h = pPanel->disp_info->height;

	pPanel->drvState = DRV_STATE_OPEN;

	DSI_INFO("OK\n");

	return res;

err_gpio_request:
err_reg_enable:
err_reg_init:
err_resume_link:
	return -1;
}

/*
 *
 *  Function Name: DSI_SuspendLink
 *
 *  Description:   Suspend/Close the DSI link and shutoff clocks.
 *
 */
static Int32 DSI_SuspendLink(DISPDRV_HANDLE_T drvH)
{
	Int32 res = 0;
	DispDrv_PANEL_t *pPanel = (DispDrv_PANEL_t *)drvH;

	if (pPanel->disp_info->vmode)
		CSL_DSI_Suspend(pPanel->dsiCmVcHandle);

	if (CSL_DSI_CloseCmVc(pPanel->dsiCmVcHandle)) {
		DSI_ERR("Closing Command Mode Handle\n");
		return -1;
	}

	if (CSL_DSI_Ulps(pPanel->clientH, TRUE) != CSL_LCD_OK) {
		DSI_ERR("ERR enter DSI Ulps\n");
		return -1;
	}

	if (CSL_DSI_CloseClient(pPanel->clientH) != CSL_LCD_OK) {
		DSI_ERR("Closing DSI Client\n");
		return -1;
	}

	if (CSL_DSI_Close(pPanel->busNo) != CSL_LCD_OK) {
		DSI_ERR("ERR Closing DSI Controller\n");
		return -1;
	}

	if (pPanel->isTE)
		DSI_TeOff(pPanel);

	if (brcm_disable_dsi_pll_clocks(pPanel->busNo)) {
		DSI_ERR("ERROR disabling the pll clock\n");
		return -1;
	}
	return res;
}

/*
 *
 *  Function Name: DSI_Close
 *
 *  Description:   disp bus OFF
 *
 */
static Int32 DSI_Close(DISPDRV_HANDLE_T drvH)
{
	Int32 res = 0;
	DispDrv_PANEL_t	*pPanel	= (DispDrv_PANEL_t *)drvH;

	DSI_ExecCmndList(pPanel, pPanel->disp_info->slp_in_seq);
	pPanel->drvState = DRV_STATE_INIT;

	res = DSI_SuspendLink(drvH);
	if (res < 0)
		goto exit;

	gpio_free(pPanel->disp_info->rst->gpio);

	pPanel->drvState = DRV_STATE_INIT;
	DSI_INFO("OK\n");
exit:
	return res;
}

/*
 *
 *   Function Name:   panel_write
 *
 *   Description:  interface to other driver
 *
 */
void panel_write(UInt8 *buff)
{
	DSI_ExecCmndList(panel, (char *)buff);
}

/*
 *
 *   Function Name:   panel_read
 *
 *   Description:  interface to other driver
 *
 */
void panel_read(UInt8 reg, UInt8 *rxBuff, UInt8 buffLen)
{
	DSI_DCS_Read(panel, reg, rxBuff, buffLen);
}

DEFINE_MUTEX(cmnd_mutex);
/*
 *
 *   Function Name:   DSI_ExecCmndList
 *
 *   Description:
 *
 */
static void DSI_ExecCmndList(DispDrv_PANEL_t *pPanel, char *buff)
{
	CSL_DSI_CMND_t msg;
	int res = 0;
	Boolean generic;
	/* To avoid race condition, when multiple
	   threads try to execute send commands concurrently*/
	mutex_lock(&cmnd_mutex);
	msg.vc = pPanel->cmnd_mode->vc;
	msg.isLP = pPanel->disp_info->cmnd_LP;
	msg.endWithBta = FALSE;
	msg.reply = NULL;

	while (*buff) {
		uint8_t len = *buff++;
		if (len == DISPCTRL_TAG_SLEEP) {
			msleep(*buff++);
		} else {
			if (len == DISPCTRL_TAG_GEN_WR) {
				/* (~0 -1) was used as generic cmd tag */
				len = *buff++;
				generic = TRUE;
			} else
				generic = FALSE;
			switch (len) {
			case 0:
				res = -1;
				break;
			case 1:
				msg.dsiCmnd = generic ?
					DSI_DT_SH_GEN_WR_P1 :
					DSI_DT_SH_DCS_WR_P0;
				break;
			case 2:
				msg.dsiCmnd = generic ?
					DSI_DT_SH_GEN_WR_P2 :
					DSI_DT_SH_DCS_WR_P1;
				break;
			default:
				if (len <= CSL_DSI_GetMaxTxMsgSize())
					msg.dsiCmnd = generic ?
						DSI_DT_LG_GEN_WR :
						DSI_DT_LG_DCS_WR;
				else
					res = -1;
				break;
			}
			if (res) {
				DSI_ERR("Packet size err %d\n", res);
				goto err_size;
			}
			msg.isLong = len > 2;
			msg.msg = buff;
			msg.msgLen = len;

			if (panel[0].drvState != DRV_STATE_OPEN) {
				pr_err("driver not in OPEN state\n");
				__WARN();
				goto err_state;
			}

			res = CSL_DSI_SendPacket(pPanel->clientH, &msg, FALSE);
			buff += len;
		}
		if (res)
			DSI_ERR("Error while sending packet %d\n", res);
	}
err_state:
err_size:
	mutex_unlock(&cmnd_mutex);
	return;
}

/*
 *
 *   Function Name: DSI_SetMaxRtnPktSize
 *
 *   Description:   Set Window
 *
 */
static Int32 DSI_SetMaxRtnPktSize(DISPDRV_HANDLE_T drvH, UInt8 size)
{
	DispDrv_PANEL_t	*pPanel	= (DispDrv_PANEL_t *)drvH;
	CSL_DSI_CMND_t msg;
	UInt8 txData[2];  /* DCS Rd Command */
	Int32 res = 0;
	CSL_LCD_RES_T cslRes;

	if (size == pPanel->maxRetPktSize)
		return 0;
	DSI_INFO("%d\n", size);

	txData[0] = size;
	txData[1] = 0x0;
	msg.vc = pPanel->cmnd_mode->vc;
	msg.isLP = pPanel->disp_info->cmnd_LP;
	msg.endWithBta = FALSE;
	msg.reply = NULL;
	msg.dsiCmnd    = DSI_DT_SH_MAX_RET_PKT_SIZE;
	msg.msg        = &txData[0];
	msg.msgLen     = 2;
	msg.isLong     = FALSE;
	msg.endWithBta = FALSE;

	DSI_DBG("[DISPDRV]: disCmnd[0x%02X]\n", DSI_DT_SH_MAX_RET_PKT_SIZE);
	cslRes = CSL_DSI_SendPacket(pPanel->clientH, &msg, FALSE);
	if (cslRes != CSL_LCD_OK) {

		DSI_ERR(
			"[DISPDRV]:	ERR: Setting Max. Return Packet Size [0x%02X]\n\r"
			, DSI_DT_SH_MAX_RET_PKT_SIZE);
		res = -1;
	} else
		pPanel->maxRetPktSize = size;

	return res;
}

/*
 *
 *   Function Name: DSI_DCS_Read
 *
 *   Description:   DSI Read Reg
 *
 */
static Int32 DSI_DCS_Read(DispDrv_PANEL_t *pPanel,
		UInt8 reg, UInt8 *rxBuff, UInt8 buffLen)
{
	CSL_DSI_CMND_t msg;
	CSL_DSI_REPLY_t rxMsg;      /* DSI RX message */
	UInt8 txData[2], i;  /* DCS Rd Command */
	Int32 res = 0;
	CSL_LCD_RES_T cslRes;

	msg.dsiCmnd    = DSI_DT_SH_DCS_RD_P0;
	msg.msg        = &txData[0];
	msg.msgLen     = 2;
	msg.vc = pPanel->cmnd_mode->vc;
	msg.isLP = pPanel->disp_info->cmnd_LP;
	if (buffLen > 2)
		msg.isLong     = TRUE;
	else
		msg.isLong     = FALSE;
	msg.endWithBta = TRUE;

	rxMsg.pReadReply = (UInt8 *)rxBuff;
	msg.reply        = (CSL_DSI_REPLY_t *)&rxMsg;

	txData[0] = reg;
	txData[1] = 0x0;
	DSI_SetMaxRtnPktSize(pPanel, buffLen);
	cslRes = CSL_DSI_SendPacket(pPanel->clientH, &msg, FALSE);
	if ((cslRes != CSL_LCD_OK) ||
		((rxMsg.type & DSI_RX_TYPE_READ_REPLY) == 0)) {

		DSI_ERR(
			"[DISPDRV]:	ERR: Reading From Reg[0x%08X]\n\r"
			, reg);
		res = -1;
	} else {
		DSI_DBG("[DISPDRV]: Command: 0x%02X\n", reg);
		for (i = 0; i < buffLen; i++)
			DSI_DBG("Parameter[%d]: [0x%02X]\n", i, rxBuff[i]);
		res = rxMsg.readReplySize;
	}
	return res;
}

/*
 *
 *   Function Name: DSI_PowerControl
 *
 *   Description:	  Display Module Control
 *
 */
static Int32 DSI_PowerControl(
	DISPDRV_HANDLE_T drvH,
	DISPLAY_POWER_STATE_T state)
{
	Int32  res = 0;
	DispDrv_PANEL_t *pPanel = (DispDrv_PANEL_t *)drvH;
	DISPDRV_INFO_T *info = pPanel->disp_info;

	DSI_INFO("state %d pwrState %d\n", state, pPanel->pwrState);
	switch (state) {
	case CTRL_PWR_ON:
		switch (pPanel->pwrState) {
		case STATE_PWR_OFF:
			res = regulator_enable(disp_reg);
			if (res < 0) {
				DSI_ERR("Couldn't enable regulator, res=%ld\n",
					res);
				res = -1;
				break;
			}
			usleep_range(1000, 1010);
			hw_reset(drvH, FALSE);

			if (!pPanel->panel_identified) {
				if (DSI_ReadPanelIDs(pPanel) < 0) {
					DSI_ERR("ID read failed\n");
					res = -1;
					break;
				}
				pPanel->panel_identified = true;
			}

			if (info->init_fn)
				info->init_fn();
			DSI_ExecCmndList(pPanel, info->init_seq);
			DSI_WinSet(drvH, TRUE, &pPanel->win_dim);
			pPanel->pwrState = STATE_SCREEN_OFF;
			DSI_INFO("INIT-SEQ\n");
			break;
		default:
			DSI_ERR("POWER ON req While Not In POWER DOWN State\n");
			break;
		}
		break;

	case CTRL_PWR_OFF:
		if (pPanel->pwrState !=	STATE_PWR_OFF) {
			if (info->vmode && info->vid_cmnds)
				DSI_panel_shut_down(pPanel);
			else
				DSI_ExecCmndList(pPanel, info->scrn_off_seq);
			DSI_ExecCmndList(pPanel, info->slp_in_seq);

			res = regulator_disable(disp_reg);
			if (res < 0) {
				DSI_ERR("Couldn't disable regulator, res=%ld\n",
					res);
				res = -1;
				break;
			}
			hw_reset(drvH, TRUE);
			memset(&pPanel->win_cur, 0, sizeof(DISPDRV_WIN_t));
			pPanel->pwrState = STATE_PWR_OFF;
			DSI_INFO("PWR DOWN\n");
		}
		break;

	case CTRL_SLEEP_IN:
		switch (pPanel->pwrState) {
		case STATE_SCREEN_ON:
			if (info->vmode && info->vid_cmnds)
				DSI_panel_shut_down(pPanel);
			else
				DSI_ExecCmndList(pPanel, info->scrn_off_seq);
			/* Turn OFF and sleep in */
		case STATE_SCREEN_OFF:
			DSI_ExecCmndList(pPanel, info->slp_in_seq);
			pPanel->pwrState = STATE_SLEEP;
			DSI_INFO("SLEEP-IN\n");
			break;
		default:
			DSI_ERR("SLEEP Req, But Not In DISP ON|OFF State\n");
			break;
		}
		break;

	case CTRL_SLEEP_OUT:
		switch (pPanel->pwrState) {
		case STATE_SLEEP:
			DSI_ExecCmndList(pPanel, info->slp_out_seq);
			pPanel->pwrState = STATE_SCREEN_OFF;
			DSI_INFO("SLEEP-OUT\n");
			break;
		default:
			DSI_ERR("SLEEP-OUT Req While Not In SLEEP State\n");
			break;
		}
		break;

	case CTRL_SCREEN_ON:
		switch (pPanel->pwrState) {
		case STATE_SCREEN_OFF:
			if (info->cabc_enabled) {
				DSI_ExecCmndList(pPanel, info->cabc_init_seq);
				DSI_ExecCmndList(pPanel, info->cabc_on_seq);
			}
			if (info->vmode && info->vid_cmnds)
				DSI_panel_turn_on(pPanel);
			else
				DSI_ExecCmndList(pPanel, info->scrn_on_seq);

			pPanel->pwrState = STATE_SCREEN_ON;
			DSI_INFO("SCREEN ON\n");
			break;
		default:
			DSI_ERR("SCRN ON Req While Not In SCRN OFF State\n");
			break;
		}
		break;
	case CTRL_SCREEN_OFF:
		switch (pPanel->pwrState) {
		case STATE_SCREEN_ON:
			if (info->vmode && info->vid_cmnds)
				DSI_panel_shut_down(pPanel);
			else
				DSI_ExecCmndList(pPanel, info->scrn_off_seq);

			pPanel->pwrState = STATE_SCREEN_OFF;
			DSI_INFO("SCREEN OFF\n");
			break;
		default:
			DSI_ERR("SCRN OFF Req While Not In SCRN ON State\n");
			break;
		}
		break;
	default:
		DSI_ERR("Invalid Power State[%d] Requested\n", state);
		res = -1;
		break;
	}
	return res;
}

/*
 *
 *  Function Name: DSI_Start
 *
 *  Description:
 *
 */
static Int32 DSI_Start(
	DISPDRV_HANDLE_T drvH,
	struct pi_mgr_dfs_node *dfs_node)
{
	DispDrv_PANEL_t *pPanel = (DispDrv_PANEL_t *)drvH;

	if (brcm_enable_dsi_lcd_clocks(dfs_node, pPanel->busNo,
		DispDrv_dsiCfg.hsBitClk.clkIn_MHz * 1000000,
		DispDrv_dsiCfg.hsBitClk.clkInDiv,
		DispDrv_dsiCfg.escClk.clkIn_MHz * 1000000
		/ DispDrv_dsiCfg.escClk.clkInDiv)) {

		DSI_ERR("ERROR enabling the clock\n");
		return -1;
	}

	return 0;
}

/*
 *
 *   Function Name: DSI_Stop
 *
 *   Description:
 *
 */
static Int32 DSI_Stop(DISPDRV_HANDLE_T drvH, struct pi_mgr_dfs_node *dfs_node)
{
	DispDrv_PANEL_t *pPanel = (DispDrv_PANEL_t *)drvH;

	if (brcm_disable_dsi_lcd_clocks(dfs_node, pPanel->busNo)) {
		DSI_ERR("ERROR to enable the clock\n");
	    return -1;
	}
	return 0;
}


/*
 *
 *  Function Name: DSI_Cb
 *
 *  Description:   CSL callback
 *
 */
static void DSI_Cb(CSL_LCD_RES_T cslRes, pCSL_LCD_CB_REC pCbRec)
{
	DISPDRV_CB_RES_T apiRes;

	DSI_DBG("+\n");

	if (pCbRec->dispDrvApiCb != NULL) {
		switch (cslRes)	{
		case CSL_LCD_OK:
			apiRes = DISPDRV_CB_RES_OK;
			break;
		default:
			apiRes = DISPDRV_CB_RES_ERR;
			break;
		}

		((DISPDRV_CB_T)pCbRec->dispDrvApiCb)(apiRes);
	}

	DSI_DBG("-\n");
}


/*
 *
 *  Function Name: DSI_Update
 *
 *  Description:
 *
 */
static Int32 DSI_Update(
	DISPDRV_HANDLE_T drvH,
	void *buff,
	DISPDRV_WIN_t *p_win,
	DISPDRV_CB_T apiCb)
{
	DispDrv_PANEL_t	*pPanel	= (DispDrv_PANEL_t *)drvH;
	CSL_LCD_UPD_REQ_T req;
	Int32 res  = 0;
	uint32_t offset;
	DISPDRV_INFO_T *info = pPanel->disp_info;
	bool use_te = true;

	DSI_DBG("+\n");
	if (pPanel->pwrState ==	STATE_PWR_OFF) {
		DSI_ERR("Skip Due To Power State\n");
		return -1;
	}
	if (p_win == NULL)
		p_win =	&pPanel->win_dim;

	DSI_DBG("%d %d %d %d\n", p_win->l,
			p_win->r, p_win->t, p_win->b);

	DSI_WinSet(drvH, TRUE, p_win);

	offset = (uint32_t)buff;
	offset += (p_win->t * pPanel->disp_info->width + p_win->l)
			* pPanel->disp_info->Bpp;

	req.buff	= (uint32_t *)offset;
	req.lineLenP	= p_win->w;
	req.lineCount	= p_win->h;
	req.xStrideB	= pPanel->disp_info->width - p_win->w;
	req.buffBpp	= pPanel->disp_info->Bpp;
	req.timeOut_ms = pPanel->disp_info->vmode ? MAX_SCHEDULE_TIMEOUT : 3000;
	req.cslLcdCbRec.cslH		= pPanel->clientH;
	req.cslLcdCbRec.dispDrvApiCbRev	= DISP_DRV_CB_API_REV_1_0;
	req.cslLcdCbRec.dispDrvApiCb	= (void	*) apiCb;
	req.cslLcdCbRec.dispDrvApiCbP1	= NULL;

	DSI_DBG("buf=%08x, linelenp = %lu, linecnt =%lu\n",
		(u32)req.buff, req.lineLenP, req.lineCount);

	if (apiCb != NULL)
		req.cslLcdCb = DSI_Cb;
	else
		req.cslLcdCb = NULL;

	if ((info->no_te_in_sleep && (pPanel->pwrState == STATE_SCREEN_OFF)) ||
				!pPanel->isTE)
		use_te = false;

	if (pPanel->disp_info->vmode)
		res = CSL_DSI_UpdateVmVc(pPanel->dsiCmVcHandle, &req);
	else
		res = CSL_DSI_UpdateCmVc(pPanel->dsiCmVcHandle, &req, use_te);

	if (res != CSL_LCD_OK)	{
		DSI_ERR("ERROR ret by CSL_DSI_UpdateCmVc\n");
		res = -1;
		DSI_Cb(res, &req.cslLcdCbRec);
	}

	if (pPanel->send_first_frame_event) {
		char *envp[2] = {"FIRST FRAME", NULL};

		if (kobject_uevent_env(&pPanel->pdev->dev.kobj, KOBJ_CHANGE,
									envp))
			pr_err("%s: Failed to send FIRST FRAME\n", __func__);
		panel->send_first_frame_event = false;
	}
	DSI_DBG("-\n");

	return res;
}

/*
 *
 *  Function Name: DSI_Atomic_Update
 *
 *  Description:
 *
 */
static Int32 DSI_Atomic_Update(
	DISPDRV_HANDLE_T drvH,
	void			*buff,
	DISPDRV_WIN_t		*p_win)
{
	DispDrv_PANEL_t	*pPanel	= (DispDrv_PANEL_t *)drvH;
	CSL_LCD_UPD_REQ_T req;
	Int32 res  = 0;

	DSI_DBG("+\n");

	if (pPanel->pwrState ==	STATE_PWR_OFF) {
		DSI_ERR("Skip Due To Power State\n");
		return -1;
	}

	if (p_win == NULL)
		p_win =	&pPanel->win_dim;

	CSL_DSI_Force_Stop(pPanel->dsiCmVcHandle);

	CSL_DSI_Lock(pPanel->dsiCmVcHandle);

	csl_dma_lock();

	DSI_WinSet(drvH, TRUE, p_win);

	req.buff	= buff;
	req.lineLenP	= p_win->w;
	req.lineCount	= p_win->h;
	req.buffBpp	= pPanel->disp_info->Bpp;
	req.timeOut_ms = pPanel->disp_info->vmode ? MAX_SCHEDULE_TIMEOUT : 100;
	req.xStrideB	= 0;
	req.cslLcdCbRec.cslH		= pPanel->clientH;
	req.cslLcdCbRec.dispDrvApiCbRev	= DISP_DRV_CB_API_REV_1_0;
	req.cslLcdCbRec.dispDrvApiCb	= NULL;
	req.cslLcdCbRec.dispDrvApiCbP1	= NULL;
	req.cslLcdCb = NULL;

	DSI_DBG("buf=%08x, linelenp = %lu, linecnt =%lu\n",
		(u32)req.buff, req.lineLenP, req.lineCount);

	if (pPanel->disp_info->vmode)
		res = CSL_DSI_UpdateVmVc(pPanel->dsiCmVcHandle, &req);
	else
		res = CSL_DSI_UpdateCmVc(pPanel->dsiCmVcHandle, &req,
							pPanel->isTE);
	if (res != CSL_LCD_OK)	{
		DSI_ERR("ERROR ret by CSL_DSI_UpdateCmVc\n");
		res = -1;
	}

	DSI_DBG("-\n");

	csl_dma_unlock();

	CSL_DSI_Unlock(pPanel->dsiCmVcHandle);

	return res;
}

