/*******************************************************************************
* Copyright 2011 Broadcom Corporation.  All rights reserved.
*
* 	@file	 arch/arm/plat-kona/csl/csl_dsi.c
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
/**
*
*   @file   csl_dsi.c
*
*   @brief  DSI Controller Implementation 
*
*           HERA/RHEA DSI Controller Implementation
*
****************************************************************************/

#define	__CSL_DSI_USE_INT__
#define	__CSL_DSI_USE_CLK_API__

#define	UNDER_LINUX

#ifdef UNDER_LINUX
#include <linux/string.h>
#include <linux/workqueue.h>
#include <linux/time.h>
#include <linux/interrupt.h>
#include <linux/broadcom/mobcom_types.h>
#include <linux/broadcom/msconsts.h>
#include <plat/osabstract/ostypes.h>
#include <plat/osabstract/ostask.h>
#include <plat/osabstract/ossemaphore.h>
#include <plat/osabstract/osqueue.h>
#include <plat/osabstract/osinterrupt.h>
#include <mach/irqs.h>
#include <mach/memory.h>
#include <linux/kernel.h>
#include <mach/io_map.h>
#else
#include <stdio.h>
#include <string.h>
#include "mobcom_types.h"
#include "msconsts.h"
#include "ostypes.h"
#include "osheap.h"
#include "ostask.h"
#include "ossemaphore.h"
#include "osqueue.h"
#include "irqctrl.h"
#include "osinterrupt.h"
#include "sio.h"
#include "chip_irq.h"
#endif

#ifdef UNDER_LINUX
#include <plat/csl/csl_dma_vc4lite.h>
#else
#include "csl_dma_vc4lite.h"
#endif

#ifdef UNDER_LINUX
#include <plat/chal/chal_common.h>
#include <plat/chal/chal_dsi.h>
#include <plat/chal/chal_clk.h>
#include <plat/csl/csl_lcd.h>
#include <plat/csl/csl_dsi.h>

extern void *videomemory;

#else
#include "chal_common.h"
#include "chal_dsi.h"
#include "chal_clk.h"
#include "dbg.h"
#include "logapi.h"
#include "csl_lcd.h"
#include "csl_dsi.h"
#endif

#define	CORE_CLK_MAX_MHZ	125

#define	DSI_INITIALIZED		0x13579BDF
#define	DSI_INST_COUNT		(UInt32)2
#define	DSI_MAX_CLIENT		4
#define	DSI_CM_MAX_HANDLES	4

#define	DSI_PKT_RPT_MAX		0x3FFF
#define	FLUSH_Q_SIZE		1

#define	TX_PKT_ENG_1		((UInt8)0)
#define	TX_PKT_ENG_2		((UInt8)1)

#define	TASKPRI_DSI		(TPriority_t)(ABOVE_NORMAL)
#define	STACKSIZE_DSI		STACKSIZE_BASIC*5
#define	HISRSTACKSIZE_DSISTAT	(256 + RESERVED_STACK_FOR_LISR)

#ifdef UNDER_LINUX
#define	CSL_DSI0_IRQ		BCM_INT_ID_DSI0
#define	CSL_DSI1_IRQ		BCM_INT_ID_DSI1
#else
#define	CSL_DSI0_IRQ		MM_DSI0_IRQ
#define	CSL_DSI1_IRQ		MM_DSI1_IRQ
#endif

#define	CSL_DSI0_BASE_ADDR	KONA_DSI0_VA
#define	CSL_DSI1_BASE_ADDR	KONA_DSI1_VA

#define	CM_PKT_SIZE_B		768
#define	DE1_DEF_THRESHOLD_W	(CM_PKT_SIZE_B >> 2)
#define	DE1_DEF_THRESHOLD_B	(CM_PKT_SIZE_B)

// DSI Core clk tree configuration / settings
typedef struct {
	UInt32 hsPllReq_MHz;	// in:  PLL freq requested
	UInt32 escClkIn_MHz;	// in:  ESC clk in requested
	UInt32 hsPllSet_MHz;	// out: PLL freq set
	UInt32 hsBitClk_MHz;	// out: end HS bit clock
	UInt32 escClk_MHz;	// out: ESC clk after req inDIV
	UInt32 hsClkDiv;	// out: HS  CLK Div Reg value 
	UInt32 escClkDiv;	// out: ESC CLK Div Reg value
	UInt32 hsPll_P1;	// out: PLL setting
	UInt32 hsPll_N_int;	// out: PLL setting
	UInt32 hsPll_N_frac;	// out: PLL setting
	CHAL_DSI_CLK_SEL_t coreClkSel;	// out: chal core_clk_sel
} DSI_CLK_CFG_T;

// DSI Client 
typedef struct {
	CSL_LCD_HANDLE lcdH;
	Boolean open;
	Boolean hasLock;
} DSI_CLIENT_t, *DSI_CLIENT;

// DSI Command Mode 
typedef struct {
	DSI_CLIENT client;
	Boolean configured;
	UInt8 dcsCmndStart;
	UInt8 dcsCmndCont;
	CHAL_DSI_DE1_COL_MOD_t cm;
	UInt32 vc;
	cUInt32 dsiCmnd;
	Boolean isLP;
	UInt32 vmWhen;
	Boolean usesTe;
	CHAL_DSI_TE_MODE_t teMode;
	UInt32 bpp_wire;
	UInt32 bpp_dma;
	UInt32 wc_rshift;
} DSI_CM_HANDLE_t, *DSI_CM_HANDLE;

// DSI Interface 
typedef struct {
	CHAL_HANDLE chalH;
	UInt32 bus;
	UInt32 init;
	UInt32 initOnce;
	Boolean ulps;
	UInt32 dsiCoreRegAddr;
	UInt32 clients;
	DSI_CLK_CFG_T clkCfg;	// HS & ESC Clk configuration
	Task_t updReqT;
	Queue_t updReqQ;
	Semaphore_t semaDsi;
	Semaphore_t semaInt;
	Semaphore_t semaDma;
#ifdef UNDER_LINUX
	irq_handler_t lisr;
#else
	void (*lisr) (void);
#endif
	void (*hisr) (void);
	void (*task) (void);
	void (*dma_cb) (DMA_VC4LITE_CALLBACK_STATUS);
	Interrupt_t iHisr;
#ifdef UNDER_LINUX
	UInt32 interruptId;
	spinlock_t bcm_dsi_spin_Lock;
#else
	InterruptId_t interruptId;
#endif
	DSI_CLIENT_t client[DSI_MAX_CLIENT];
	DSI_CM_HANDLE_t chCm[DSI_CM_MAX_HANDLES];
} DSI_HANDLE_t, *DSI_HANDLE;

typedef struct {
	DMA_VC4LITE_CHANNEL_t dmaCh;
	DSI_HANDLE dsiH;	// CSL DSI handle
	DSI_CLIENT clientH;	// CSL DSI Client handle
	CSL_LCD_UPD_REQ_T updReq;	// update Request
} DSI_UPD_REQ_MSG_T;

typedef enum {
	DSI_LDO_HP = 1,
	DSI_LDO_LP,
	DSI_LDO_OFF,
} DSI_LDO_STATE_t;

static DSI_HANDLE_t dsiBus[DSI_INST_COUNT];

#ifndef	__CSL_DSI_USE_CLK_API__
static Boolean cslDsiClkTreeInit(DSI_HANDLE dsiH, const DSI_CLK_T *hsClk,
				 const DSI_CLK_T *escClk);
static Boolean cslDsiClkTreeEna(DSI_HANDLE dsiH);
#endif // #ifndef __CSL_DSI_USE_CLK_API__

static void cslDsiEnaIntEvent(DSI_HANDLE dsiH, UInt32 intEventMask);
static CSL_LCD_RES_T cslDsiWaitForStatAny_Poll(DSI_HANDLE dsiH, UInt32 mask,
					       UInt32 *intStat,
					       UInt32 tout_msec);
static CSL_LCD_RES_T cslDsiWaitForInt(DSI_HANDLE dsiH, UInt32 tout_msec);
static CSL_LCD_RES_T cslDsiDmaStart(DSI_UPD_REQ_MSG_T *updMsg);
static CSL_LCD_RES_T cslDsiDmaStop(DSI_UPD_REQ_MSG_T *updMsg);
static void cslDsiClearAllFifos(DSI_HANDLE dsiH);

//#define printk(format, arg...)        do {   } while (0)

//*****************************************************************************
//
// Function Name:  cslDsi0Stat_LISR
// 
// Description:    DSI Controller 0 LISR 
//
//*****************************************************************************
#ifdef __KERNEL__
static irqreturn_t cslDsi0Stat_LISR(int i, void *j)
#else
static void cslDsi0Stat_LISR(void)
#endif
{
	DSI_HANDLE dsiH = &dsiBus[0];

	chal_dsi_ena_int(dsiH->chalH, 0);
	chal_dsi_clr_int(dsiH->chalH, 0xFFFFFFFF);

	OSSEMAPHORE_Release(dsiH->semaInt);

#ifdef __KERNEL__
	return IRQ_HANDLED;
#endif
}

//*****************************************************************************
//
// Function Name:  cslDsi0Stat_HISR
// 
// Description:    DSI Controller 0 HISR 
//
//*****************************************************************************
static void cslDsi0Stat_HISR(void)
{
	DSI_HANDLE dsiH = &dsiBus[0];

	OSSEMAPHORE_Release(dsiH->semaInt);
}

//*****************************************************************************
//
// Function Name:  cslDsi1Stat_LISR
// 
// Description:    DSI Controller 1 LISR 
//
//*****************************************************************************
#ifdef __KERNEL__
static irqreturn_t cslDsi1Stat_LISR(int i, void *j)
#else
static void cslDsi1Stat_LISR(void)
#endif
{
	DSI_HANDLE dsiH = &dsiBus[1];

	chal_dsi_ena_int(dsiH->chalH, 0);
	chal_dsi_clr_int(dsiH->chalH, 0xFFFFFFFF);

	OSSEMAPHORE_Release(dsiH->semaInt);

#ifdef __KERNEL__
	return IRQ_HANDLED;
#endif
}

//*****************************************************************************
//
// Function Name:  cslDsi1Stat_HISR
// 
// Description:    DSI Controller 1 HISR 
//
//*****************************************************************************
static void cslDsi1Stat_HISR(void)
{
	DSI_HANDLE dsiH = &dsiBus[1];

	OSSEMAPHORE_Release(dsiH->semaInt);
}

//*****************************************************************************
//
// Function Name:  cslDsi0EofDma
// 
// Description:    DSI Controller 0 EOF DMA 
//
//*****************************************************************************
static void cslDsi0EofDma(DMA_VC4LITE_CALLBACK_STATUS status)
{
	DSI_HANDLE dsiH = &dsiBus[0];

	if (status != DMA_VC4LITE_CALLBACK_SUCCESS) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI] %s: ERR DMA!\n",
			__FUNCTION__);
	}

	OSSEMAPHORE_Release(dsiH->semaDma);
}

//*****************************************************************************
//
// Function Name:  cslDsi1EofDma
// 
// Description:    DSI Controller 1 EOF DMA 
//
//*****************************************************************************
static void cslDsi1EofDma(DMA_VC4LITE_CALLBACK_STATUS status)
{
	DSI_HANDLE dsiH = &dsiBus[1];

	if (status != DMA_VC4LITE_CALLBACK_SUCCESS) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI] %s: ERR DMA!\n",
			__FUNCTION__);
	}

	OSSEMAPHORE_Release(dsiH->semaDma);
}

//*****************************************************************************
//
// Function Name:  cslDsi0UpdateTask
// 
// Description:    DSI Controller 0 Update Task
//
//*****************************************************************************
static void cslDsi0UpdateTask(void)
{
	DSI_UPD_REQ_MSG_T updMsg;
	OSStatus_t osStat;
	CSL_LCD_RES_T res;
	DSI_HANDLE dsiH = &dsiBus[0];

	for (;;) {
		res = CSL_LCD_OK;

		// Wait for update request
		OSQUEUE_Pend(dsiH->updReqQ, (QMsg_t *)&updMsg, TICKS_FOREVER);

		// Wait For signal from eof DMA
		osStat = OSSEMAPHORE_Obtain(dsiH->semaDma,
					    TICKS_IN_MILLISECONDS(updMsg.updReq.
								  timeOut_ms));

		if (osStat != OSSTATUS_SUCCESS) {
			if (osStat == OSSTATUS_TIMEOUT) {
				LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI] %s: "
					"TIMED OUT While waiting for "
					"EOF DMA\n", __FUNCTION__);
				res = CSL_LCD_OS_TOUT;
			} else {
				LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI] %s: "
					"OS ERR While waiting for EOF DMA\n",
					__FUNCTION__);
				res = CSL_LCD_OS_ERR;
			}

			cslDsiDmaStop(&updMsg);
		}

		if (res == CSL_LCD_OK)
			res = cslDsiWaitForInt(dsiH, updMsg.updReq.timeOut_ms);
		else
			cslDsiWaitForInt(dsiH, 1);

		chal_dsi_tx_start(dsiH->chalH, TX_PKT_ENG_1, FALSE);
		chal_dsi_tx_start(dsiH->chalH, TX_PKT_ENG_2, FALSE);
		chal_dsi_de1_enable(dsiH->chalH, FALSE);

		if (!updMsg.clientH->hasLock)
			OSSEMAPHORE_Release(dsiH->semaDsi);

		if (updMsg.updReq.cslLcdCb)
			updMsg.updReq.cslLcdCb(res, &updMsg.updReq.cslLcdCbRec);
		else
			LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI] %s: "
				"Callback EQ NULL, Skipping\n", __FUNCTION__);
	}
}				// cslDsi0UpdateTask

//*****************************************************************************
//
// Function Name:  cslDsi1UpdateTask
// 
// Description:    DSI Controller 0 Update Task
//
//*****************************************************************************
static void cslDsi1UpdateTask(void)
{
	DSI_UPD_REQ_MSG_T updMsg;
	OSStatus_t osStat;
	CSL_LCD_RES_T res;
	DSI_HANDLE dsiH = &dsiBus[1];

	for (;;) {
		res = CSL_LCD_OK;

		// Wait for update request
		OSQUEUE_Pend(dsiH->updReqQ, (QMsg_t *)&updMsg, TICKS_FOREVER);

		// Wait For signal from eof DMA
		osStat = OSSEMAPHORE_Obtain(dsiH->semaDma,
					    TICKS_IN_MILLISECONDS(updMsg.updReq.
								  timeOut_ms));

		if (osStat != OSSTATUS_SUCCESS) {
			if (osStat == OSSTATUS_TIMEOUT) {
				LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI] %s: "
					"TIMED OUT While waiting for "
					"EOF DMA\n", __FUNCTION__);
				res = CSL_LCD_OS_TOUT;
			} else {
				LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI] %s: "
					"OS ERR While waiting for EOF DMA\n",
					__FUNCTION__);
				res = CSL_LCD_OS_ERR;
			}

			cslDsiDmaStop(&updMsg);
		}

		if (res == CSL_LCD_OK) {
			res = cslDsiWaitForInt(dsiH, 100);
		} else {
			cslDsiWaitForInt(dsiH, 1);
		}

		chal_dsi_de1_enable(dsiH->chalH, FALSE);
		chal_dsi_tx_start(dsiH->chalH, TX_PKT_ENG_1, FALSE);
		chal_dsi_tx_start(dsiH->chalH, TX_PKT_ENG_2, FALSE);

		if (!updMsg.clientH->hasLock)
			OSSEMAPHORE_Release(dsiH->semaDsi);

		if (updMsg.updReq.cslLcdCb) {
			updMsg.updReq.cslLcdCb(res, &updMsg.updReq.cslLcdCbRec);
		} else {
			LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI] %s: "
				"Callback EQ NULL, Skipping\n", __FUNCTION__);
		}
	}
}				// cslDsi1UpdateTask

//*****************************************************************************
//
// Function Name:  cslDsiOsInit
// 
// Description:    DSI COntroller OS Interface Init
//
//*****************************************************************************
Boolean cslDsiOsInit(DSI_HANDLE dsiH)
{
	Boolean res = TRUE;

#ifdef UNDER_LINUX
	int ret;
#endif

	// Update Request Queue
	dsiH->updReqQ = OSQUEUE_Create(FLUSH_Q_SIZE,
				       sizeof(DSI_UPD_REQ_MSG_T),
				       OSSUSPEND_PRIORITY);
	if (!dsiH->updReqQ) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI] %s: OSQUEUE_Create failed\n",
			__FUNCTION__);
		res = FALSE;
	} else {
		OSQUEUE_ChangeName(dsiH->updReqQ,
				   dsiH->bus ? "Dsi1Q" : "Dsi0Q");
	}

	// Update Request Task
	dsiH->updReqT = OSTASK_Create(dsiH->task,
				      dsiH->
				      bus ? (TName_t) "Dsi1T" : (TName_t)
				      "Dsi0T", TASKPRI_DSI, STACKSIZE_DSI);
	if (!dsiH->updReqT) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI] %s: Create Task failure\n",
			__FUNCTION__);
		res = FALSE;
	}
	// DSI Interface Semaphore
	dsiH->semaDsi = OSSEMAPHORE_Create(1, OSSUSPEND_PRIORITY);
	if (!dsiH->semaDsi) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI] %s: ERR Sema Creation!\n",
			__FUNCTION__);
		res = FALSE;
	} else {
		OSSEMAPHORE_ChangeName(dsiH->semaDsi,
				       dsiH->bus ? "Dsi1" : "Dsi0");
	}

	// DSI Interrupt Event Semaphore
	dsiH->semaInt = OSSEMAPHORE_Create(0, OSSUSPEND_PRIORITY);
	if (!dsiH->semaInt) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI] %s: ERR Sema Creation!\n",
			__FUNCTION__);
		res = FALSE;
	} else {
		OSSEMAPHORE_ChangeName(dsiH->semaInt,
				       dsiH->bus ? "Dsi1Int" : "Dsi0Int");
	}

	// EndOfDma Semaphore
	dsiH->semaDma = OSSEMAPHORE_Create(0, OSSUSPEND_PRIORITY);
	if (!dsiH->semaDma) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI] %s: ERR Sema Creation!\n",
			__FUNCTION__);
		res = FALSE;
	} else {
		OSSEMAPHORE_ChangeName(dsiH->semaDma,
				       dsiH->bus ? "Dsi1Dma" : "Dsi0Dma");
	}

#ifndef	__KERNEL__
	// DSI Controller Interrupt
	dsiH->iHisr = OSINTERRUPT_Create((IEntry_t)dsiH->hisr,
					 dsiH->
					 bus ? (IName_t) "Dsi1" : (IName_t)
					 "Dsi0", IPRIORITY_MIDDLE,
					 HISRSTACKSIZE_DSISTAT);
#endif

#ifdef __KERNEL__
	ret = request_irq(dsiH->interruptId, dsiH->lisr, IRQF_DISABLED |
			  IRQF_NO_SUSPEND, "BRCM DSI CSL", NULL);
	if (ret < 0) {
		pr_err("%s(%s:%u)::request_irq failed IRQ %d\n",
		       __FUNCTION__, __FILE__, __LINE__, dsiH->interruptId);
		goto free_irq;
	}
#else
	IRQ_Register(dsiH->interruptId, dsiH->lisr);
#endif

	return (res);

#ifdef __KERNEL__
free_irq:
	free_irq(dsiH->interruptId, NULL);
	return (res);
#endif
}				// cslLcdcOsInit

//*****************************************************************************
//
// Function Name:   cslDsiDmaStart
//
// Description:     RHEA MM DMA 
//                  Prepare & Start DMA Transfer            
//                  FOR NOW - ONLY LINEAR MODE SUPPORTED
//
//*****************************************************************************
static CSL_LCD_RES_T cslDsiDmaStart(DSI_UPD_REQ_MSG_T *updMsg)
{
	CSL_LCD_RES_T result = CSL_LCD_OK;

	DMA_VC4LITE_CHANNEL_INFO_t dmaChInfo;
	DMA_VC4LITE_XFER_DATA_t dmaData;
	Int32 dmaCh;
	UInt32 width;
	UInt32 height;

	// Reserve Channel
	dmaCh =
	    csl_dma_vc4lite_obtain_channel(DMA_VC4LITE_CLIENT_MEMORY,
					   DMA_VC4LITE_CLIENT_MEMORY);

	if (dmaCh == -1) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI] %s: ERR Reserving "
			"DMA Ch\n", __FUNCTION__);
		return CSL_LCD_DMA_ERR;
	}
	updMsg->dmaCh = (DMA_VC4LITE_CHANNEL_t)dmaCh;

	LCD_DBG(LCD_DBG_ID, "[CSL DSI] %s: Got DmaCh[%d]\n ",
		__FUNCTION__, dmaCh);

	// Configure Channel
	dmaChInfo.autoFreeChan = 1;
	dmaChInfo.srcID = DMA_VC4LITE_CLIENT_MEMORY;

	if (updMsg->dsiH->bus)
		dmaChInfo.dstID = DMA_VC4LITE_CLIENT_DSI1;
	else
		dmaChInfo.dstID = DMA_VC4LITE_CLIENT_DSI0;

	//dmaChInfo.burstLen     = DMA_VC4LITE_BURST_LENGTH_4;
	dmaChInfo.burstLen = DMA_VC4LITE_BURST_LENGTH_8;
	dmaChInfo.xferMode = DMA_VC4LITE_XFER_MODE_LINERA;
	dmaChInfo.dstStride = 0;
	dmaChInfo.srcStride = 0;
	dmaChInfo.waitResponse = 0;
	dmaChInfo.callback = (DMA_VC4LITE_CALLBACK_t) updMsg->dsiH->dma_cb;

	if (csl_dma_vc4lite_config_channel(updMsg->dmaCh, &dmaChInfo)
	    != DMA_VC4LITE_STATUS_SUCCESS) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI] %s: "
			"ERR Configure DMA Ch\n ", __FUNCTION__);
		return CSL_LCD_DMA_ERR;
	}

	width = updMsg->updReq.lineLenP * updMsg->updReq.buffBpp;
	height = updMsg->updReq.lineCount;

	// Add the DMA transfer data
	dmaData.srcAddr = (UInt32)updMsg->updReq.buff;
	dmaData.dstAddr = chal_dsi_de1_get_dma_address(updMsg->dsiH->chalH);
	dmaData.xferLength = width * height;

	if (csl_dma_vc4lite_add_data(updMsg->dmaCh, &dmaData)
	    != DMA_VC4LITE_STATUS_SUCCESS) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI] %s: "
			"ERR add DMA transfer data\n ", __FUNCTION__);
		return CSL_LCD_DMA_ERR;
	}
	// start DMA transfer
	if (csl_dma_vc4lite_start_transfer(updMsg->dmaCh)
	    != DMA_VC4LITE_STATUS_SUCCESS) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI] %s: "
			"ERR start DMA data transfer\n ", __FUNCTION__);
		return CSL_LCD_DMA_ERR;
	}
	mb();

	return (result);
}

//*****************************************************************************
//
// Function Name:   cslDsiDmaStop
//
// Description:     RHEA MM DMA - Stop DMA
//                  
//*****************************************************************************
static CSL_LCD_RES_T cslDsiDmaStop(DSI_UPD_REQ_MSG_T *updMsg)
{
	CSL_LCD_RES_T res = CSL_LCD_ERR;

	LCD_DBG(LCD_DBG_ID, "[CSL DSI] +cslDsiDmaStop\n");

	// stop DMA transfer
	if (csl_dma_vc4lite_stop_transfer(updMsg->dmaCh)
	    != DMA_VC4LITE_STATUS_SUCCESS) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI] %s: "
			"ERR DMA Stop Transfer\n ", __FUNCTION__);
		res = CSL_LCD_DMA_ERR;
	}
	// release DMA channel
	if (csl_dma_vc4lite_release_channel(updMsg->dmaCh)
	    != DMA_VC4LITE_STATUS_SUCCESS) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI] %s: "
			"ERR ERR DMA Release Channel\n ", __FUNCTION__);
		res = CSL_LCD_DMA_ERR;
	}

	cslDsiClearAllFifos(updMsg->dsiH);
	updMsg->dmaCh = DMA_VC4LITE_CHANNEL_INVALID;

	LCD_DBG(LCD_DBG_ID, "[CSL DSI] -cslDsiDmaStop\n");

	return (res);
}

//*****************************************************************************
//
// Function Name:  cslDsiAfeLdoSetState
// 
// Description:    AFE LDO Control 
//
//*****************************************************************************
static void cslDsiAfeLdoSetState(DSI_HANDLE dsiH, DSI_LDO_STATE_t state)
{
#define	DSI_LDO_HP_EN	  0x00000001
#define	DSI_LDO_LP_EN	  0x00000002
#define	DSI_LDO_CNTL_ENA  0x00000004
#define	DSI_LDO_ISO_OUT	  0x00800000

	UInt32 ldo_val = 0;

	switch (state) {
	case DSI_LDO_HP:
		ldo_val = DSI_LDO_CNTL_ENA | DSI_LDO_HP_EN;
		break;
	case DSI_LDO_LP:
		ldo_val = DSI_LDO_CNTL_ENA | DSI_LDO_LP_EN;
		break;
	case DSI_LDO_OFF:
		ldo_val = DSI_LDO_CNTL_ENA | DSI_LDO_ISO_OUT;
		break;
	default:
		ldo_val = DSI_LDO_CNTL_ENA | DSI_LDO_HP_EN;
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI] %s: "
			"ERROR Invalid LDO State[%d] !\r\n", __FUNCTION__,
			state);
		break;
	}

	if (dsiH->bus == 0)
		*(volatile cUInt32 *)HW_IO_PHYS_TO_VIRT(0x3C004030) = ldo_val;
	else
		*(volatile cUInt32 *)HW_IO_PHYS_TO_VIRT(0x3C004034) = ldo_val;

	OSTASK_Sleep(TICKS_IN_MILLISECONDS(1));
}

#ifndef	__CSL_DSI_USE_CLK_API__
//*****************************************************************************
//
// Function Name:  cslDsiClkTreeInit
// 
// Description:    Init CLK Tree For HighSpeed & Escape Clocks
//
//*****************************************************************************
static Boolean cslDsiClkTreeInit(DSI_HANDLE dsiH, const DSI_CLK_T *in_hsClk,	// HS  Input clock & Divider setting
				 const DSI_CLK_T *in_escClk	// ESC Input clock & Divider setting
    )
{
	UInt32 hsClkSet;	// Base HS Clk Set ( PLL if available )
	DSI_CLK_T hsClk = *in_hsClk;
	DSI_CLK_T escClk = *in_escClk;

	printk(KERN_ERR "hsclk->clk=0x%08x  div=0x%08x, and esc->clk = 0x%08x "
	       "and its div = 0x%08x\n",
	       hsClk.clkIn_MHz, hsClk.clkInDiv,
	       escClk.clkIn_MHz, escClk.clkInDiv);

	//--- H S  clock -------------------------------------------------------
	if ((hsClk.clkIn_MHz < 600) || (hsClk.clkIn_MHz > 2400)) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI] %s: "
			"ERR DSI PLL, Valid Input Range: 600 - 2400 MHz\n",
			__FUNCTION__);
		return (FALSE);
	}
	// RHEA, for now increments of HS by 26[Mbps]
	dsiH->clkCfg.hsPll_P1 = 1;
	dsiH->clkCfg.hsPll_N_int = (UInt32)(hsClk.clkIn_MHz / 26);
	dsiH->clkCfg.hsPll_N_frac = (UInt32)(hsClk.clkIn_MHz / 26)
	    - dsiH->clkCfg.hsPll_N_int) * (1 << 20));

	hsClkSet = 26 * (dsiH->clkCfg.hsPll_N_int
			 + dsiH->clkCfg.hsPll_N_frac / (1 << 20));

	dsiH->clkCfg.hsPllReq_MHz = hsClk.clkIn_MHz;
	dsiH->clkCfg.hsPllSet_MHz = hsClkSet;

	// check for valid HS clk divider value ( PLL CH )   
	// we can divide in the range of 1 to 256 ( 0=256 )
	if (hsClk.clkInDiv > 256) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI] %s: "
			"ERR DSI PLL CH Div Overflow[%d] , max=256\n",
			__FUNCTION__, hsClk.clkInDiv);
		return (FALSE);
	}
	dsiH->clkCfg.hsBitClk_MHz = hsClkSet / hsClk.clkInDiv;
	if (hsClk.clkInDiv == 256)
		dsiH->clkCfg.hsClkDiv = 0;
	else
		dsiH->clkCfg.hsClkDiv = hsClk.clkInDiv;

	LCD_DBG(LCD_DBG_INIT_ID, "[CSL DSI] %s: HS-CLK: "
		"InPll[%d] InPllDiv[%d], PllSetTo[%6.1f] => %6.1f[Mhz]\n",
		__FUNCTION__, hsClk.clkIn_MHz, hsClk.clkInDiv,
		hsClkSet, dsiH->clkCfg.hsBitClk_MHz);

	//--- E S C  clock -----------------------------------------------------
	if (escClk.clkIn_MHz != 500 && escClk.clkIn_MHz != 312) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI] %s: "
			"ERR ESC CLK In, Valid Input: 500 or 312[MHz]\n",
			__FUNCTION__);
		return (FALSE);
	}
	// need it later since RHEA can have multiple esc clk source
	dsiH->clkCfg.escClkIn_MHz = escClk.clkIn_MHz;

	// RHEA - not clear from RDB, for now assume 0-7 range & 0==8 
	if (escClk.clkInDiv >= 9) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI] %s: "
			"ERR ESC CLK Div Overflow[%d] , max=8\n",
			__FUNCTION__, escClk.clkInDiv);
		return (FALSE);
	}
	if (escClk.clkInDiv == 8)
		dsiH->clkCfg.escClkDiv = 0;
	else
		dsiH->clkCfg.escClkDiv = escClk.clkInDiv;

	dsiH->clkCfg.escClk_MHz = escClk.clkIn_MHz / escClk.clkInDiv;

	LCD_DBG(LCD_DBG_INIT_ID, "[CSL DSI] %s: ESC-CLK: "
		"InEsc[%d] InEscDiv[%d] => %6.1f[Mhz]\n", __FUNCTION__,
		escClk.clkIn_MHz, escClk.clkInDiv, dsiH->clkCfg.escClk_MHz);

	return (TRUE);
}

//=============================================================================
//                               RHEA Clk 
//=============================================================================
struct {
	volatile UInt32 *pAxiClkEnaR;
	volatile UInt32 *pEscClkEnaR;
	volatile UInt32 *pDsiDiv;
	volatile UInt32 *pDsiPllNint;
	volatile UInt32 *pDsiPllNfrac;
	volatile UInt32 *pDsiPllChHsClkDiv;
} dsiClkR[2] = {

	{
		((volatile UInt32 *)HW_IO_PHYS_TO_VIRT(0x3c000280)),	// DSI0_AXI
		    ((volatile UInt32 *)HW_IO_PHYS_TO_VIRT(0x3c000284)),	// DSI0_ESC
		    ((volatile UInt32 *)HW_IO_PHYS_TO_VIRT(0x3c000a1c)),	// DSI0_DIV
		    ((volatile UInt32 *)HW_IO_PHYS_TO_VIRT(0x3c000c00)),	// PLLDSIA    
		    ((volatile UInt32 *)HW_IO_PHYS_TO_VIRT(0x3c000c04)),	// PLLDSIB    
		    ((volatile UInt32 *)HW_IO_PHYS_TO_VIRT(0x3c000c08)),	// PLLDSIC  Ch0
	}, {
		((volatile UInt32 *)HW_IO_PHYS_TO_VIRT(0x3c00028C)),	// DSI1_AXI
		    ((volatile UInt32 *)HW_IO_PHYS_TO_VIRT(0x3c000290)),	// DSI1_ESC
		    ((volatile UInt32 *)HW_IO_PHYS_TO_VIRT(0x3c000a20)),	// DSI0_DIV
		    ((volatile UInt32 *)HW_IO_PHYS_TO_VIRT(0x3c000c00)),	// PLLDSIA    
		    ((volatile UInt32 *)HW_IO_PHYS_TO_VIRT(0x3c000c04)),	// PLLDSIB    
		    ((volatile UInt32 *)HW_IO_PHYS_TO_VIRT(0x3c000c0C)),	// PLLDSIE  Ch1
},};

//*****************************************************************************
//
// Function Name:  cslDsiClkTreeEna
// 
// Description:    Configure & Enable RHEA DSI Clocks
// HERA: 
//    o Single PLL, 3 CH, CH0=DSI0  CH1=DSI1 CH2=?
//    o PLL inClk is 26MHz 
//    o (26MHz/P1)*(Nint + Nfrac)       
//    o  P1    range =  3 bits MM_CLK_PLLDSIA[26..24] 1-7    ? rdb says 0=16  ?
//    o  Nint  range = 10 bits MM_CLK_PLLDSIA[17..08] 1-1024 ? rdb says 0=512 ?
//    o  Nfrac range = 20 bits MM_CLK_PLLDSIB[19..00]
//
//*****************************************************************************
static Boolean cslDsiClkTreeEna(DSI_HANDLE dsiH) {

#define	MM_CLK_PLLDSIA_RELEASE_RST	    ((UInt32)0x00000001)
#define	MM_CLK_PLLDSIA_RELEASE_CH_RST	    ((UInt32)0x00000002)
#define	MM_CLK_PLLDSIA_PDN		    ((UInt32)0x00000008)
#define	MM_CLK_PLLDSIA_PUP		    ((UInt32)0x00000000)
#define	MM_CLK_PLLDSIA_DISABLE		    ((UInt32)0x00000010)
#define	MM_CLK_PLLDSIA_ENABLE		    ((UInt32)0x00000000)

#define	MM_CLK_PLLDSIA_LOCKED		    ((UInt32)0x10000000)
#define	MM_CLK_PLLDSIA_LOCKED_RAW	    ((UInt32)0x00000080)
#define	MM_CLK_PLLDSIA_LOCKED_SINCE_RST	    ((UInt32)0x00000040)
#define	MM_CLK_PLLDSIA_FREQ_LOCKED	    ((UInt32)0x00000020)

#define	MM_CLK_PLL_CH_LOAD_ENA		    ((UInt32)0x00000800)
#define	MM_CLK_PLL_CH_HOLD_ENA		    ((UInt32)0x00000400)
#define	MM_CLK_PLL_CH_HOLD_DIS		    ((UInt32)0x00000000)
#define	MM_CLK_PLL_CH_OUT_DIS		    ((UInt32)0x00000200)
#define	MM_CLK_PLL_CH_OUT_ENA		    ((UInt32)0x00000000)
#define	MM_CLK_PLL_CH_BYPASS		    ((UInt32)0x00000100)
#define	MM_CLK_PLL_CH_NORMAL		    ((UInt32)0x00000000)

#define	ESC_CLK_IN_500_MHZ		    ((UInt32)0)
#define	ESC_CLK_IN_312_MHZ		    ((UInt32)1)

#define	DSI_CORE_CLK_no_clock		    ((UInt32)0)
#define	DSI_CORE_CLK_ref_crystal_clk	    ((UInt32)1)
#define	DSI_CORE_CLK_testdebug_div2_clk	    ((UInt32)2)
#define	DSI_CORE_CLK_testdebug_clk	    ((UInt32)3)
#define	DSI_CORE_CLK_dsi0_txddrclk	    ((UInt32)4)
#define	DSI_CORE_CLK_dsi0_txddrclk_n	    ((UInt32)5)
#define	DSI_CORE_CLK_dsi0_txddrclk2	    ((UInt32)6)
#define	DSI_CORE_CLK_dsi0_txddrclk2_n	    ((UInt32)7)
#define	DSI_CORE_CLK_dsi0_tx0_bclkhs	    ((UInt32)8)
#define	DSI_CORE_CLK_dsi0_tx0_bclkhs_n	    ((UInt32)9)

	// pup = 1
#define	DSI_CLK_HYST_ENA		    ((UInt32)0x000000100)
	// pup = 1
#define	DSI_CLK_HYST_DEL_HI		    ((UInt32)0x000000200)
	// pup = 1  0=AUTO(HW)
#define	DSI_CLK_GATE_SW			    ((UInt32)0x000000002)
	// pup = 0  1=SW_ENA
#define	DSI_CLK_SW_ENA			    ((UInt32)0x000000001)
#define	DSI_CLK_SW_DIS			    ((UInt32)0x000000000)

#define	MM_CLK_WR_ACCESS  \
		((volatile UInt32*) HW_IO_PHYS_TO_VIRT(0x3c000000))

	static Boolean dsiPllOn = FALSE;
	static UInt32 dsiPllFreq;
	UInt32 pllReg;
	UInt32 dsiCoreClkSel;
	UInt32 escClkInSel;

	// if second core is asking for diff PLL freq, error out
	if (dsiPllOn && (dsiPllFreq != dsiH->clkCfg.hsPllSet_MHz)) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI] %s: "
			"ERR DSI PLL Already In Use %8.3f[MHz], "
			"Req %8.3f[MHz]. Second DSI must use same PLL "
			"freq, but diff PLL CH divider value!\n",
			__FUNCTION__, dsiPllFreq, dsiH->clkCfg.hsPllSet_MHz);
		return (FALSE);
	}
	// enable wr acess to MM_CLK CCU registers
	wmb();
	*MM_CLK_WR_ACCESS = 0x00A5A501;

	if (!dsiPllOn) {
		*dsiClkR[dsiH->bus].pDsiPllNfrac = dsiH->clkCfg.hsPll_N_frac;

		pllReg = MM_CLK_PLLDSIA_RELEASE_RST
		    | MM_CLK_PLLDSIA_RELEASE_CH_RST
		    | MM_CLK_PLLDSIA_PUP
		    | MM_CLK_PLLDSIA_ENABLE | (dsiH->clkCfg.hsPll_P1 << 24)
		    | (dsiH->clkCfg.hsPll_N_int << 8);

		wmb();
		*dsiClkR[dsiH->bus].pDsiPllNint = pllReg;

		// wait for PLL to LOCK
		OSTASK_Sleep(TICKS_IN_MILLISECONDS(1));

		LCD_DBG(LCD_DBG_INIT_ID, "[CSL DSI] %s: "
			"INFO DSI PLL Set to %8.3f[MHz]\n",
			__FUNCTION__, dsiH->clkCfg.hsPllSet_MHz);

		rmb();
		if (!(*dsiClkR[dsiH->bus].pDsiPllNint & MM_CLK_PLLDSIA_LOCKED)) {
			LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI] %s: ERR PLL Lock "
				"Timeout!\n", __FUNCTION__);
			return (FALSE);
		}
		dsiPllOn = TRUE;
		dsiPllFreq = dsiH->clkCfg.hsPllSet_MHz;
	}
	//=== START AXI clk
	wmb();
	*dsiClkR[dsiH->bus].pAxiClkEnaR = DSI_CLK_GATE_SW | DSI_CLK_SW_ENA;

	//--------------------------------------------------
	//=== Figure Out DSI Core HS TX clock
	//--------------------------------------------------
	// choose the highest possible clock for DSI Core 
	if ((UInt32)(dsiH->clkCfg.hsBitClk_MHz / 2) <= CORE_CLK_MAX_MHZ) {
		dsiCoreClkSel = DSI_CORE_CLK_dsi0_txddrclk;
		wmb();
		//*pCoreClkSel = CHAL_DSI_BIT_CLK_DIV_BY_2;
		dsiH->clkCfg.coreClkSel = CHAL_DSI_BIT_CLK_DIV_BY_2;
		LCD_DBG(LCD_DBG_INIT_ID, "[CSL DSI] %s: DSI CORE CLK SET "
			"TO ... BIT_CLK/2\n", __FUNCTION__);
	} else if ((UInt32)(dsiH->clkCfg.hsBitClk_MHz / 4) <= CORE_CLK_MAX_MHZ) {
		dsiCoreClkSel = DSI_CORE_CLK_dsi0_txddrclk2;
		wmb();
		//*pCoreClkSel = CHAL_DSI_BIT_CLK_DIV_BY_4;
		dsiH->clkCfg.coreClkSel = CHAL_DSI_BIT_CLK_DIV_BY_4;
		LCD_DBG(LCD_DBG_INIT_ID, "[CSL DSI] %s: DSI CORE CLK SET "
			"TO ... BIT_CLK/4\n", __FUNCTION__);
	} else {
		dsiCoreClkSel = DSI_CORE_CLK_dsi0_tx0_bclkhs;
		wmb();
		//*pCoreClkSel  = CHAL_DSI_BIT_CLK_DIV_BY_8;
		dsiH->clkCfg.coreClkSel = CHAL_DSI_BIT_CLK_DIV_BY_8;
		LCD_DBG(LCD_DBG_INIT_ID, "[CSL DSI] %s: DSI CORE CLK SET "
			"TO ... BIT_CLK/8\n", __FUNCTION__);
	}

	//-------------------------------------------------------------------
	// === set/start ESC clock & Activate DSI Core HS TX clock selection
	//-------------------------------------------------------------------
	// D I R E C T  
	if (dsiH->clkCfg.escClkIn_MHz == 312)
		escClkInSel = ESC_CLK_IN_312_MHZ;
	else
		escClkInSel = ESC_CLK_IN_500_MHZ;

	//--- STOP ESC CLK
	wmb();
	*dsiClkR[dsiH->bus].pEscClkEnaR = DSI_CLK_GATE_SW | DSI_CLK_SW_DIS;

	//--- RECONFIG ESC clk & Pixel Clk Selection
	wmb();
	*dsiClkR[dsiH->bus].pDsiDiv = (dsiCoreClkSel << 0)
	    | (escClkInSel << 4)
	    | ((dsiH->clkCfg.escClkDiv - 1) << 8);

	//--- START ESC clk
	wmb();
	*dsiClkR[dsiH->bus].pEscClkEnaR = DSI_CLK_GATE_SW | DSI_CLK_SW_ENA;

	// === START HS TX clock ( set PLL ch divider )
	// MM_CLK_PLL_CH_BYPASS  ( use 624 & o_pll_divider ? )
	// MM_CLK_PLL_CH_NORMAL  ( use PLL  ? )
	wmb();
	*dsiClkR[dsiH->bus].pDsiPllChHsClkDiv = (dsiH->clkCfg.hsClkDiv << 0)
	    | MM_CLK_PLL_CH_LOAD_ENA
	    | MM_CLK_PLL_CH_HOLD_DIS
	    | MM_CLK_PLL_CH_OUT_ENA | MM_CLK_PLL_CH_NORMAL;

	return (TRUE);
}
#endif // #ifndef __CSL_DSI_USE_CLK_API__

//*****************************************************************************
//
// Function Name:  cslDsiWaitForStatAny_Poll
// 
// Description:    
//                 
//*****************************************************************************
static CSL_LCD_RES_T cslDsiWaitForStatAny_Poll(DSI_HANDLE dsiH,
					       UInt32 statMask,
					       UInt32 *intStat, UInt32 mSec) {
	CSL_LCD_RES_T res = CSL_LCD_OK;
	UInt32 stat = 0;

	stat = chal_dsi_get_status(dsiH->chalH);

	while ((stat & statMask) == 0) {
		stat = chal_dsi_get_status(dsiH->chalH);
	}

	if (intStat != NULL)
		*intStat = stat;

	return (res);
}

//*****************************************************************************
//
// Function Name:  cslDsiClearAllFifos
// 
// Description:    Clear All DSI FIFOs
//                 
//*****************************************************************************
static void cslDsiClearAllFifos(DSI_HANDLE dsiH) {
	UInt32 fifoMask;

	fifoMask = CHAL_DSI_CTRL_CLR_LANED_FIFO
	    | CHAL_DSI_CTRL_CLR_RXPKT_FIFO
	    | CHAL_DSI_CTRL_CLR_PIX_DATA_FIFO | CHAL_DSI_CTRL_CLR_CMD_DATA_FIFO;
	chal_dsi_clr_fifo(dsiH->chalH, fifoMask);
}

//*****************************************************************************
//
// Function Name:  cslDsiEnaIntEvent
// 
// Description:    event bits set to "1" will be enabled, 
//                 rest of the events will be disabled
//                 
//*****************************************************************************
static void cslDsiEnaIntEvent(DSI_HANDLE dsiH, UInt32 intEventMask) {
	chal_dsi_ena_int(dsiH->chalH, intEventMask);
#ifdef UNDER_LINUX
//      enable_irq(dsiH->interruptId);
#else
	IRQ_Enable(dsiH->interruptId);
#endif
}

//*****************************************************************************
//
// Function Name:  cslDsiDisInt
// 
// Description:    
//                 
//*****************************************************************************
static void cslDsiDisInt(DSI_HANDLE dsiH) {
#ifdef UNDER_LINUX
	//disable_irq(dsiH->interruptId);
#else
	IRQ_Disable(dsiH->interruptId);
#endif
	chal_dsi_ena_int(dsiH->chalH, 0);
	chal_dsi_clr_int(dsiH->chalH, 0xFFFFFFFF);
#ifndef	UNDER_LINUX
	IRQ_Clear(dsiH->interruptId);
#endif
}

//*****************************************************************************
//
// Function Name:  cslDsiWaitForInt
// 
// Description:    
//                 
//*****************************************************************************
static CSL_LCD_RES_T cslDsiWaitForInt(DSI_HANDLE dsiH, UInt32 tout_msec) {
	OSStatus_t osRes;
	CSL_LCD_RES_T res = CSL_LCD_OK;

	osRes =
	    OSSEMAPHORE_Obtain(dsiH->semaInt, TICKS_IN_MILLISECONDS(tout_msec));

	if (osRes != OSSTATUS_SUCCESS) {
		cslDsiDisInt(dsiH);

		if (osRes == OSSTATUS_TIMEOUT) {
			LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI] %s: "
				"ERR Timed Out!\n", __FUNCTION__);
			res = CSL_LCD_OS_TOUT;
		} else {
			LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI] %s: "
				"ERR OS Err...!\n", __FUNCTION__);
			res = CSL_LCD_OS_ERR;
		}
	}
	return (res);
}

//*****************************************************************************
//
// Function Name:  cslDsiBtaRecover
// 
// Description:    Recover From Failed BTA
//                 
//*****************************************************************************
static CSL_LCD_RES_T cslDsiBtaRecover(DSI_HANDLE dsiH) {
	CSL_LCD_RES_T res = CSL_LCD_OK;

	chal_dsi_phy_state(dsiH->chalH, PHY_TXSTOP);

	OSTASK_Sleep(TICKS_IN_MILLISECONDS(1));

	chal_dsi_clr_fifo(dsiH->chalH,
			  CHAL_DSI_CTRL_CLR_CMD_DATA_FIFO |
			  CHAL_DSI_CTRL_CLR_LANED_FIFO);

	chal_dsi_phy_state(dsiH->chalH, PHY_CORE);
	return (res);
}

//*****************************************************************************
//                 
// Function Name:  CSL_DSI_Lock
// 
// Description:    Lock DSI Interface For Exclusive Use By a Client
//                 
//*****************************************************************************
void CSL_DSI_Lock(CSL_LCD_HANDLE client) {
	DSI_CLIENT clientH = (DSI_CLIENT) client;
	DSI_HANDLE dsiH = (DSI_HANDLE)clientH->lcdH;

	if (clientH->hasLock)
		WARN(TRUE, "[CSL DSI][%d] %s: "
		     "DSI Client Lock/Unlock Not balanced\n",
		     dsiH->bus, __func__);
	else {
		OSSEMAPHORE_Obtain(dsiH->semaDsi, TICKS_FOREVER);
		clientH->hasLock = TRUE;
	}
}

//*****************************************************************************
//                 
// Function Name:  CSL_DSI_Unlock
// 
// Description:    Release Client's DSI Interface Lock
//                 
//*****************************************************************************
void CSL_DSI_Unlock(CSL_LCD_HANDLE client) {
	DSI_CLIENT clientH = (DSI_CLIENT) client;
	DSI_HANDLE dsiH = (DSI_HANDLE)clientH->lcdH;

	if (!clientH->hasLock)
		WARN(TRUE, "[CSL DSI][%d] %s: "
		     "DSI Client Lock/Unlock Not balanced\n",
		     dsiH->bus, __func__);
	else {
		OSSEMAPHORE_Release(dsiH->semaDsi);
		clientH->hasLock = FALSE;
	}
}

//*****************************************************************************
//                 
// Function Name:  CSL_DSI_SendPacketTrigger  
// 
// Description:    Send TRIGGER Message
//                 
//*****************************************************************************
CSL_LCD_RES_T CSL_DSI_SendTrigger(CSL_LCD_HANDLE client, UInt8 trig) {
	DSI_HANDLE dsiH;
	DSI_CLIENT clientH;
	CSL_LCD_RES_T res;

	clientH = (DSI_CLIENT) client;
	dsiH = (DSI_HANDLE)clientH->lcdH;

	if (!clientH->hasLock)
		OSSEMAPHORE_Obtain(dsiH->semaDsi, TICKS_FOREVER);

	if (dsiH->ulps) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI][%d] %s: "
			"ERR, Link Is In ULPS\n", dsiH->bus, __FUNCTION__);
		res = CSL_LCD_BAD_STATE;
		goto exit_err;
	}

	chal_dsi_clr_status(dsiH->chalH, 0xFFFFFFFF);

#ifdef __CSL_DSI_USE_INT__
	cslDsiEnaIntEvent(dsiH, (UInt32)CHAL_DSI_ISTAT_TXPKT1_DONE);
#endif
	chal_dsi_tx_trig(dsiH->chalH, TX_PKT_ENG_1, trig);

#ifdef __CSL_DSI_USE_INT__
	res = cslDsiWaitForInt(dsiH, 100);
#else
	res = cslDsiWaitForStatAny_Poll(dsiH,
					CHAL_DSI_STAT_TXPKT1_DONE, NULL, 100);
#endif

exit_err:
	if (!clientH->hasLock)
		OSSEMAPHORE_Release(dsiH->semaDsi);

	return (res);
}

//*****************************************************************************
//                 
// Function Name:  CSL_DSI_SendPacket
// 
// Description:    Send DSI Packet (non-pixel data) with an option to end it 
//                 with BTA. 
//                 If BTA is requested, command.reply MUST be valid
//
//*****************************************************************************
CSL_LCD_RES_T CSL_DSI_SendPacket(CSL_LCD_HANDLE client,
				 pCSL_DSI_CMND command, Boolean isTE) {
	DSI_HANDLE dsiH;
	DSI_CLIENT clientH;
	CSL_LCD_RES_T res = CSL_LCD_OK;
	CHAL_DSI_TX_CFG_t txPkt;
	CHAL_DSI_RES_t chalRes;
	UInt32 stat;
	UInt32 event;
	UInt32 pfifo_len = 0;

	clientH = (DSI_CLIENT) client;
	dsiH = (DSI_HANDLE)clientH->lcdH;

	if (command->msgLen > CHAL_DSI_TX_MSG_MAX) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI][%d] %s: "
			"ERR, TX Packet Size To Big\n",
			dsiH->bus, __FUNCTION__, command->vc);
		return (CSL_LCD_MSG_SIZE);
	}
	if (command->endWithBta && (command->reply == NULL)) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI][%d] %s: "
			"ERR, VC[%d] BTA Requested But pReply Eq NULL\n",
			dsiH->bus, __FUNCTION__, command->vc);
		return (CSL_LCD_API_ERR);
	}

	if (!clientH->hasLock)
		OSSEMAPHORE_Obtain(dsiH->semaDsi, TICKS_FOREVER);

	if (dsiH->ulps) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI][%d] %s: "
			"ERR, VC[%d] Link Is In ULPS\n",
			dsiH->bus, __FUNCTION__, command->vc);
		if (!clientH->hasLock)
			OSSEMAPHORE_Release(dsiH->semaDsi);
		return (CSL_LCD_BAD_STATE);
	}

	txPkt.dsiCmnd = command->dsiCmnd;
	txPkt.msg = command->msg;
	txPkt.msgLen = command->msgLen;
	txPkt.vc = command->vc;
	txPkt.isLP = command->isLP;
	txPkt.endWithBta = command->endWithBta;
	txPkt.isTe = isTE;
	txPkt.vmWhen = CHAL_DSI_CMND_WHEN_BEST_EFFORT;
	txPkt.repeat = 1;
	txPkt.start = 1;

	chal_dsi_clr_status(dsiH->chalH, 0xFFFFFFFF);

#ifdef __CSL_DSI_USE_INT__
	if (txPkt.endWithBta) {
		event = CHAL_DSI_ISTAT_PHY_RX_TRIG
		    | CHAL_DSI_ISTAT_RX2_PKT | CHAL_DSI_ISTAT_RX1_PKT;
	} else {
		event = CHAL_DSI_ISTAT_TXPKT1_DONE;
	}
	cslDsiEnaIntEvent(dsiH, event);
#else
	if (txPkt.endWithBta) {
		event = CHAL_DSI_STAT_PHY_RX_TRIG
		    | CHAL_DSI_STAT_RX2_PKT | CHAL_DSI_STAT_RX1_PKT;
	} else {
		event = CHAL_DSI_STAT_TXPKT1_DONE;
	}
#endif

	if (txPkt.msgLen <= 2) {
		LCD_DBG(LCD_DBG_ID, "[CSL DSI][%d] %s: "
			"SHORT, MSG_LEN[%d]\n",
			dsiH->bus, __FUNCTION__, txPkt.msgLen);
		txPkt.msgLenCFifo = 0;	// NA to short  
		chalRes = chal_dsi_tx_short(dsiH->chalH, TX_PKT_ENG_1, &txPkt);
		if (chalRes != CHAL_DSI_OK) {
			res = CSL_LCD_MSG_SIZE;
			goto exit_err;
		}
	} else {
		if (txPkt.msgLen <= CHAL_DSI_CMND_FIFO_SIZE_B) {
			txPkt.msgLenCFifo = txPkt.msgLen;

			chalRes = chal_dsi_wr_cfifo(dsiH->chalH,
						    txPkt.msg,
						    txPkt.msgLenCFifo);

			if (chalRes != CHAL_DSI_OK) {
				res = CSL_LCD_MSG_SIZE;
				goto exit_err;
			}

			LCD_DBG(LCD_DBG_ID, "[CSL DSI][%d] %s: "
				"LONG FROM CMND FIFO ONLY,"
				" CMND_FIFO[%d] PIXEL_FIFO[%d]\n",
				dsiH->bus, __FUNCTION__,
				txPkt.msgLenCFifo,
				txPkt.msgLen - txPkt.msgLenCFifo);

			chalRes = chal_dsi_tx_long(dsiH->chalH, TX_PKT_ENG_1,
						   &txPkt);
			if (chalRes != CHAL_DSI_OK) {
				res = CSL_LCD_MSG_SIZE;
				goto exit_err;
			}
		} else {
			if (txPkt.msgLen > CHAL_DSI_PIXEL_FIFO_SIZE_B) {
				txPkt.msgLenCFifo =
				    txPkt.msgLen - CHAL_DSI_PIXEL_FIFO_SIZE_B;
			} else {
				txPkt.msgLenCFifo = txPkt.msgLen % 4;
			}

			pfifo_len = txPkt.msgLen - txPkt.msgLenCFifo;

			LCD_DBG(LCD_DBG_ID, "[CSL DSI][%d] %s: "
				"LONG FROM BOTH FIFOs, "
				"CMND_FIFO[%d] PIXEL_FIFO[%d]\n",
				dsiH->bus, __FUNCTION__,
				txPkt.msgLenCFifo,
				txPkt.msgLen - txPkt.msgLenCFifo);

			chalRes = chal_dsi_wr_cfifo(dsiH->chalH, txPkt.msg,
						    txPkt.msgLenCFifo);

			if (chalRes != CHAL_DSI_OK) {
				res = CSL_LCD_MSG_SIZE;
				goto exit_err;
			}

			if (pfifo_len > DE1_DEF_THRESHOLD_B) {
				chal_dsi_de1_set_dma_thresh(dsiH->chalH,
							    pfifo_len >> 2);
			}

			chal_dsi_de1_set_cm(dsiH->chalH, DE1_CM_BE);
			chal_dsi_de1_enable(dsiH->chalH, TRUE);

			chalRes = chal_dsi_wr_pfifo_be(dsiH->chalH,
						       txPkt.msg +
						       txPkt.msgLenCFifo,
						       txPkt.msgLen -
						       txPkt.msgLenCFifo);

			if (chalRes != CHAL_DSI_OK) {
				res = CSL_LCD_MSG_SIZE;
				goto exit_err;
			}

			chalRes = chal_dsi_tx_long(dsiH->chalH, TX_PKT_ENG_1,
						   &txPkt);
			if (chalRes != CHAL_DSI_OK) {
				res = CSL_LCD_MSG_SIZE;
				goto exit_err;
			}
		}
	}

#ifdef __CSL_DSI_USE_INT__
	res = cslDsiWaitForInt(dsiH, 100);
	stat = chal_dsi_get_status(dsiH->chalH);
#else
	res = cslDsiWaitForStatAny_Poll(dsiH, event, &stat, 100);
#endif

	if (txPkt.endWithBta) {
		if (res != CSL_LCD_OK) {
			LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI][%d] %s: "
				"WARNING, VC[%d] Probable BTA TimeOut, "
				"Recovering ...\n",
				dsiH->bus, __FUNCTION__, txPkt.vc);
			cslDsiBtaRecover(dsiH);
		} else {
			chalRes = chal_dsi_read_reply(dsiH->chalH, stat,
						      (pCHAL_DSI_REPLY)
						      command->reply);
			if (chalRes == CHAL_DSI_RX_NO_PKT) {
				LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI][%d] %s: "
					"WARNING, VC[%d] BTA "
					"No Data Received\n",
					dsiH->bus, __FUNCTION__, txPkt.vc);
				res = CSL_LCD_INT_ERR;
				goto exit_ok;
			} else {
				goto exit_ok;
			}
		}
	} else {
		if (res != CSL_LCD_OK) {
			LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI][%d] %s: "
				"WARNING, VC[%d] "
				"Timed Out Waiting For TX end\n",
				dsiH->bus, __FUNCTION__, txPkt.vc);
		} else {
			goto exit_ok;
		}
	}

exit_err:
	LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI][%d] %s: ERR, CSL_LCD_RES[%d]\n",
		dsiH->bus, __FUNCTION__, res);

	cslDsiDisInt(dsiH);
	cslDsiClearAllFifos(dsiH);
exit_ok:
	chal_dsi_de1_enable(dsiH->chalH, FALSE);
	chal_dsi_tx_start(dsiH->chalH, TX_PKT_ENG_1, FALSE);

	if (pfifo_len > DE1_DEF_THRESHOLD_B) {
		chal_dsi_de1_set_dma_thresh(dsiH->chalH, DE1_DEF_THRESHOLD_W);
	}

	if (!clientH->hasLock)
		OSSEMAPHORE_Release(dsiH->semaDsi);
	return (res);
}

//*****************************************************************************
//                 
// Function Name:  CSL_DSI_OpenCmVc
//                    
// Description:    Open (configure) Command Mode VC
//                 Returns VC Command Mode handle
//
//*****************************************************************************
CSL_LCD_RES_T CSL_DSI_OpenCmVc(CSL_LCD_HANDLE client,
			       pCSL_DSI_CM_VC dsiCmVcCfg,
			       CSL_LCD_HANDLE *dsiCmVcH) {
	DSI_HANDLE dsiH;
	DSI_CLIENT clientH;
	DSI_CM_HANDLE cmVcH;
	CSL_LCD_RES_T res = CSL_LCD_OK;
	UInt32 i;

	clientH = (DSI_CLIENT) client;
	dsiH = (DSI_HANDLE)clientH->lcdH;

	if (!clientH->hasLock)
		OSSEMAPHORE_Obtain(dsiH->semaDsi, TICKS_FOREVER);

	if (dsiCmVcCfg->vc > 3) {
		*dsiCmVcH = NULL;
		res = CSL_LCD_ERR;
		goto exit_err;
	}

	for (i = 0; i < DSI_CM_MAX_HANDLES; i++) {
		if (!dsiH->chCm[i].configured)
			break;
	}

	if (i >= DSI_CM_MAX_HANDLES) {
		*dsiCmVcH = NULL;
		res = CSL_LCD_INST_COUNT;
		goto exit_err;
	} else {
		cmVcH = &dsiH->chCm[i];
	}

	cmVcH->vc = dsiCmVcCfg->vc;
	cmVcH->dsiCmnd = dsiCmVcCfg->dsiCmnd;
	cmVcH->isLP = dsiCmVcCfg->isLP;
	cmVcH->vmWhen = CHAL_DSI_CMND_WHEN_BEST_EFFORT;

	cmVcH->dcsCmndStart = dsiCmVcCfg->dcsCmndStart;
	cmVcH->dcsCmndCont = dsiCmVcCfg->dcsCmndCont;

	switch (dsiCmVcCfg->teCfg.teInType) {
	case DSI_TE_NONE:
		cmVcH->usesTe = FALSE;
		break;
	case DSI_TE_CTRLR_INPUT_0:
		cmVcH->usesTe = TRUE;
		cmVcH->teMode = TE_EXT_0;
		break;
	case DSI_TE_CTRLR_INPUT_1:
		cmVcH->usesTe = TRUE;
		cmVcH->teMode = TE_EXT_1;
		break;
	case DSI_TE_CTRLR_TRIG:
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI][%d] %s: "
			"ERR, DSI TRIG Not Supported Yet\n",
			dsiH->bus, __FUNCTION__);
		res = CSL_LCD_ERR;
		goto exit_err;
	default:
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI][%d] %s: "
			"ERR, Invalid TE Config Type [%d]\n",
			dsiH->bus, __FUNCTION__, dsiCmVcCfg->teCfg.teInType);
		res = CSL_LCD_ERR;
		goto exit_err;
	}

	switch (dsiCmVcCfg->cm_in) {
		// 1x888 pixel per 32-bit word (MSB DontCare)
	case LCD_IF_CM_I_RGB888U:
		switch (dsiCmVcCfg->cm_out) {
		case LCD_IF_CM_O_RGB666:
		case LCD_IF_CM_O_RGB888:
			cmVcH->bpp_dma = 4;
			cmVcH->bpp_wire = 3;
			cmVcH->wc_rshift = 0;
			cmVcH->cm = DE1_CM_888U;
			break;
		default:
			LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI][%d] %s: "
				"ERR, Invalid OutCol Mode[%d] for "
				"InCol xRGB888\n",
				dsiH->bus, __FUNCTION__, dsiCmVcCfg->cm_out);
			res = CSL_LCD_COL_MODE;
			goto exit_err;
			break;
		}
		break;

		// 2x565 pixels per 32-bit word
	case LCD_IF_CM_I_RGB565P:
		switch (dsiCmVcCfg->cm_out) {
		case LCD_IF_CM_O_RGB565:
		case LCD_IF_CM_O_RGB565_DSI_VM:
			cmVcH->bpp_dma = 2;
			cmVcH->bpp_wire = 2;
			cmVcH->wc_rshift = 1;
			if (dsiCmVcCfg->cm_out == LCD_IF_CM_O_RGB565)
				cmVcH->cm = DE1_CM_565;
			else
				cmVcH->cm = DE1_CM_LE;
			break;
		default:
			LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI][%d] %s: "
				"ERR, Invalid OutCol Mode[%d] for "
				"InCol RGB565\n",
				dsiH->bus, __FUNCTION__, dsiCmVcCfg->cm_out);
			res = CSL_LCD_COL_MODE;
			goto exit_err;
			break;
		}
		break;

	default:
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI][%d] %s: ERR, Invalid "
			"InCol Mode[%d]\n",
			dsiH->bus, __FUNCTION__, dsiCmVcCfg->cm_in);
		res = CSL_LCD_COL_MODE;
		goto exit_err;
		break;
	}

	cmVcH->configured = TRUE;
	cmVcH->client = clientH;

	LCD_DBG(LCD_DBG_INIT_ID, "[CSL DSI][%d] %s: "
		"OK, VC[%d], TE[%s]\n",
		dsiH->bus, __FUNCTION__, cmVcH->vc,
		cmVcH->usesTe ? "YES" : "NO");

	*dsiCmVcH = (CSL_LCD_HANDLE)cmVcH;
	if (!clientH->hasLock)
		OSSEMAPHORE_Release(dsiH->semaDsi);
	return (res);

exit_err:
	*dsiCmVcH = NULL;
	if (!clientH->hasLock)
		OSSEMAPHORE_Release(dsiH->semaDsi);
	return (res);
}

//*****************************************************************************
//
// Function Name: CSL_DSI_CloseCmVc
// 
// Description:   Close Command Mode VC handle
//                 
//*****************************************************************************
CSL_LCD_RES_T CSL_DSI_CloseCmVc(CSL_LCD_HANDLE vcH) {
	CSL_LCD_RES_T res = CSL_LCD_OK;
	DSI_CM_HANDLE dsiChH = (DSI_CM_HANDLE) vcH;
	DSI_HANDLE dsiH = (DSI_HANDLE)dsiChH->client->lcdH;
	DSI_CLIENT clientH;

	clientH = (DSI_CLIENT) dsiChH->client;

	if (!clientH->hasLock)
		OSSEMAPHORE_Obtain(dsiH->semaDsi, TICKS_FOREVER);

	dsiChH->configured = FALSE;
	LCD_DBG(LCD_DBG_INIT_ID, "[CSL DSI][%d] %s: VC[%d] Closed\n",
		dsiH->bus, __FUNCTION__, dsiChH->vc);

	if (!clientH->hasLock)
		OSSEMAPHORE_Release(dsiH->semaDsi);

	return (res);
}

//*****************************************************************************
//
// Function Name: CSL_DSI_UpdateCmVc
// 
// Description:   Command Mode - DMA Frame Update
//                RESTRICTIONs:  565 - XY pixel size == multiple of 2 pixels
//                              x888 - XY pixel size == multiple of 4 pixels    
//*****************************************************************************
CSL_LCD_RES_T CSL_DSI_UpdateCmVc(CSL_LCD_HANDLE vcH,
				 pCSL_LCD_UPD_REQ req, Boolean isTE) {
	CSL_LCD_RES_T res = CSL_LCD_OK;
	DSI_HANDLE dsiH;
	DSI_CLIENT clientH;
	DSI_CM_HANDLE dsiChH;
	OSStatus_t osStat;
	DSI_UPD_REQ_MSG_T updMsgCm;

	CHAL_DSI_TX_CFG_t txPkt;

	UInt32 frame_size_p,	// XY size                    [pixels]       
	 wire_size_b,		// XY wire size               [bytes]    
	 txNo1_len,		// TX no 1 - packet size      [bytes] 
	 txNo1_cfifo_len,	// TX no 1 - from CMND  FIFO  [bytes]
	 txNo1_pfifo_len,	// TX no 1 - from PIXEL FIFO  [bytes]
	 txNo2_repeat;		// TX no 2 - packet repeat count

	dsiChH = (DSI_CM_HANDLE) vcH;
	clientH = (DSI_CLIENT) dsiChH->client;
	dsiH = (DSI_HANDLE)clientH->lcdH;

	if (dsiH->ulps)
		return CSL_LCD_BAD_STATE;

	txPkt.vc = dsiChH->vc;
	txPkt.dsiCmnd = dsiChH->dsiCmnd;
	txPkt.vmWhen = dsiChH->vmWhen;
	txPkt.isLP = dsiChH->isLP;
	txPkt.endWithBta = FALSE;
	txPkt.start = FALSE;

	updMsgCm.updReq = *req;
	updMsgCm.dsiH = dsiH;
	updMsgCm.clientH = clientH;
	// OVERRIDE REQ BPP - NOT SUPPORTED, TODO: R E M O V E  FROM  API
	updMsgCm.updReq.buffBpp = dsiChH->bpp_dma;

	frame_size_p = req->lineLenP * req->lineCount;
	wire_size_b = frame_size_p * dsiChH->bpp_wire;

	txNo2_repeat = wire_size_b / CM_PKT_SIZE_B;
	txNo1_len = wire_size_b % CM_PKT_SIZE_B;
	if ((txNo2_repeat != 0) && (txNo1_len == 0)) {
		// XY_size = n * PACKET SIZE
		txNo1_len = CM_PKT_SIZE_B;
		txNo2_repeat--;
	}

	txNo1_pfifo_len = txNo1_len;
	txNo1_cfifo_len = 0;

	if (dsiChH->bpp_dma == 4) {
		if (frame_size_p & 0x3) {
			LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI][%d] %s: "
				"ERR Pixel Buff Size!\n",
				dsiH->bus, __FUNCTION__);
			return (CSL_LCD_MSG_SIZE);
		}
	} else {
		if (frame_size_p & 0x1) {
			LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI][%d] %s: "
				"ERR Pixel Buff Size!\n",
				dsiH->bus, __FUNCTION__);
			return (CSL_LCD_MSG_SIZE);
		}
	}
	if (txNo2_repeat > DSI_PKT_RPT_MAX) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI][%d] %s: "
			"ERR Packet Repeat Count!\n", dsiH->bus, __FUNCTION__);
		return (CSL_LCD_ERR);
	}

	if (!clientH->hasLock)
		OSSEMAPHORE_Obtain(dsiH->semaDsi, TICKS_FOREVER);

	if (dsiH->ulps) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI][%d] %s: "
			"ERR, Link Is In ULPS\n", dsiH->bus, __FUNCTION__);
		if (!clientH->hasLock)
			OSSEMAPHORE_Release(dsiH->semaDsi);
		return (CSL_LCD_BAD_STATE);
	}

	chal_dsi_clr_status(dsiH->chalH, 0xFFFFFFFF);

	// set TE mode -- SHOULD BE PART OF INIT or OPEN
	chal_dsi_te_mode(dsiH->chalH, dsiChH->teMode);

	// Set DE1 Mode, Enable 
	chal_dsi_de1_set_cm(dsiH->chalH, dsiChH->cm);
	chal_dsi_de1_enable(dsiH->chalH, TRUE);

/*
    	LCD_DBG(LCD_DBG_ID, "[CSL DSI] %s: In X Pixels          [%08d]\n",
    	    __FUNCTION__, req->lineLenP ); 
    	LCD_DBG(LCD_DBG_ID, "[CSL DSI] %s: In Y                 [%08d]\n",
    	    __FUNCTION__, req->lineCount ); 
    	LCD_DBG(LCD_DBG_ID, "[CSL DSI] %s: In bpp               [%08d]\n",
    	    __FUNCTION__, req->buffBpp ); 
    	LCD_DBG(LCD_DBG_ID, "[CSL DSI] %s: In buff              [%08X]\n",
    	    __FUNCTION__, req->buff ); 
    	LCD_DBG(LCD_DBG_ID, "[CSL DSI] %s: Packet Size          [%08d]\n",
    	    __FUNCTION__, CM_PKT_SIZE_B ); 
    	LCD_DBG(LCD_DBG_ID, "[CSL DSI] %s: Packet Repeat        [%08d]\n",
    	    __FUNCTION__, txNo2_repeat ); 

    	LCD_DBG(LCD_DBG_ID, "[CSL DSI] %s: First Line CMND FIFO [%08d]\n",
    	    __FUNCTION__, txNo1_cfifo_len ); 
    	LCD_DBG(LCD_DBG_ID, "[CSL DSI] %s: First Line PIX  FIFO [%08d]\n",
    	    __FUNCTION__, txNo1_pfifo_len ); 
*/
	//======================================== BOF TX PKT ENG(s) Set-Up ===
	// TX No1 - first packet 
	chal_dsi_wr_cfifo(dsiH->chalH, &dsiChH->dcsCmndStart, 1);
	txPkt.msgLen = 1 + txNo1_len;
	txPkt.msgLenCFifo = 1 + txNo1_cfifo_len;
	txPkt.repeat = 1;
	txPkt.isTe = dsiChH->usesTe && isTE;

	if (txNo2_repeat == 0)
		chal_dsi_tx_long(dsiH->chalH, TX_PKT_ENG_1, &txPkt);
	else
		chal_dsi_tx_long(dsiH->chalH, TX_PKT_ENG_2, &txPkt);

	// TX No2 - if any, rest of the frame 
	if (txNo2_repeat != 0) {
		chal_dsi_wr_cfifo(dsiH->chalH, &dsiChH->dcsCmndCont, 1);
		txPkt.repeat = txNo2_repeat;
		txPkt.msgLen = 1 + CM_PKT_SIZE_B;
		txPkt.msgLenCFifo = 1;
		txPkt.isTe = FALSE;

		chal_dsi_tx_long(dsiH->chalH, TX_PKT_ENG_1, &txPkt);
	}
	mb();
	//======================================== EOF TX PKT ENG(s) Set-Up ===

	//--- Wait for TX PKT ENG 1 DONE
	cslDsiEnaIntEvent(dsiH, (UInt32)CHAL_DSI_ISTAT_TXPKT1_DONE);

	//--- Start TX PKT Engine(s)
	if (txNo2_repeat != 0) {
		chal_dsi_tx_start(dsiH->chalH, TX_PKT_ENG_2, TRUE);
		mb();
	}
	chal_dsi_tx_start(dsiH->chalH, TX_PKT_ENG_1, TRUE);
	mb();
	// send rest of the frame 

//      cslDsiEnaIntEvent( dsiH, (UInt32)( CHAL_DSI_ISTAT_TXPKT1_DONE  | 
//                                         CHAL_DSI_ISTAT_ERR_CONT_LP1 | 
//                                         CHAL_DSI_ISTAT_ERR_CONT_LP0 |
//                                         CHAL_DSI_ISTAT_ERR_CONTROL  | 
//                                         CHAL_DSI_ISTAT_ERR_SYNC_ESC) );

	//--- Start DMA
	if ((res = cslDsiDmaStart(&updMsgCm)) != CSL_LCD_OK) {
		LCD_DBG(LCD_DBG_ID, "[CSL DSI][%d] %s: "
			"ERR Failed To Start DMA!\n", dsiH->bus, __FUNCTION__);

		if (!clientH->hasLock)
			OSSEMAPHORE_Release(dsiH->semaDsi);
		return (res);
	}

	if (req->cslLcdCb == NULL) {
		osStat = OSSEMAPHORE_Obtain(dsiH->semaDma,
					    TICKS_IN_MILLISECONDS(req->
								  timeOut_ms));

		if (osStat != OSSTATUS_SUCCESS) {
			cslDsiDmaStop(&updMsgCm);
			if (osStat == OSSTATUS_TIMEOUT) {
				LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI][%d] %s: "
					"ERR Timed Out Waiting For EOF DMA!\n",
					dsiH->bus, __FUNCTION__);
				res = CSL_LCD_OS_TOUT;
			} else {
				LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI][%d] %s: "
					"ERR OS Err...\n",
					dsiH->bus, __FUNCTION__);
				res = CSL_LCD_OS_ERR;
			}
		}
		//wait for interface to drain
		if (res == CSL_LCD_OK)
			res = cslDsiWaitForInt(dsiH, req->timeOut_ms);
		else
			cslDsiWaitForInt(dsiH, 1);

		chal_dsi_tx_start(dsiH->chalH, TX_PKT_ENG_1, FALSE);
		chal_dsi_tx_start(dsiH->chalH, TX_PKT_ENG_2, FALSE);
		chal_dsi_de1_enable(dsiH->chalH, FALSE);

		// cslDsiClearAllFifos ( dsiH );

		if (!clientH->hasLock)
			OSSEMAPHORE_Release(dsiH->semaDsi);
	} else {
		osStat = OSQUEUE_Post(dsiH->updReqQ,
				      (QMsg_t *)&updMsgCm, TICKS_NO_WAIT);

		if (osStat != OSSTATUS_SUCCESS) {
			if (osStat == OSSTATUS_TIMEOUT)
				res = CSL_LCD_OS_TOUT;
			else
				res = CSL_LCD_OS_ERR;
		}
	}
	return (res);
}				// CSL_DSI_UpdateCmVc

//*****************************************************************************
//                 
// Function Name:  CSL_DSI_CloseClient
// 
// Description:    Close Client Interface 
//
//*****************************************************************************
CSL_LCD_RES_T CSL_DSI_CloseClient(CSL_LCD_HANDLE client) {

	DSI_HANDLE dsiH;
	DSI_CLIENT clientH;
	CSL_LCD_RES_T res;

	clientH = (DSI_CLIENT) client;
	dsiH = (DSI_HANDLE)clientH->lcdH;

	if (clientH->hasLock) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI][%d] %s: ERR Client LOCK "
			"Active!\n", dsiH->bus, __FUNCTION__);
		res = CSL_LCD_ERR;
	} else {
		OSSEMAPHORE_Obtain(dsiH->semaDsi, TICKS_FOREVER);

		clientH->open = FALSE;
		dsiH->clients--;
		res = CSL_LCD_OK;
		LCD_DBG(LCD_DBG_INIT_ID, "[CSL DSI][%d] %s: "
			"OK, Clients Left[%d]\n",
			dsiH->bus, __FUNCTION__, dsiH->clients);

		OSSEMAPHORE_Release(dsiH->semaDsi);
	}
	return (res);
}

//*****************************************************************************
//
// Function Name: CSL_DSI_GetMaxTxMsgSize
// 
// Description:   Return Maximum size of Command Packet we can send [BYTEs]
//                (non pixel data)
//                
//*****************************************************************************
UInt32 CSL_DSI_GetMaxTxMsgSize(void) {
	return (CHAL_DSI_CMND_FIFO_SIZE_B + CHAL_DSI_PIXEL_FIFO_SIZE_B);
}

//*****************************************************************************
//
// Function Name: CSL_DSI_GetMaxRxMsgSize
// 
// Description:   Return Maximum size of Command Packet we can receive [BYTEs]
//                
//*****************************************************************************
UInt32 CSL_DSI_GetMaxRxMsgSize(void) {
	return (CHAL_DSI_RX_MSG_MAX);
}

//*****************************************************************************
//                 
// Function Name:  CSL_DSI_OpenClient
// 
// Description:    Register Client Of DSI interface 
//
//*****************************************************************************
CSL_LCD_RES_T CSL_DSI_OpenClient(UInt32 bus, CSL_LCD_HANDLE *clientH) {
	DSI_HANDLE dsiH;
	CSL_LCD_RES_T res;
	UInt32 i;

	if (bus >= DSI_INST_COUNT) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI] %s: ERR Invalid "
			"Bus Id[%d]!\n", __FUNCTION__, bus);
		*clientH = (CSL_LCD_HANDLE)NULL;
		return (CSL_LCD_BUS_ID);
	}

	dsiH = (DSI_HANDLE)&dsiBus[bus];

	OSSEMAPHORE_Obtain(dsiH->semaDsi, TICKS_FOREVER);

	if (dsiH->init != DSI_INITIALIZED) {
		res = CSL_LCD_NOT_INIT;
	} else {
		for (i = 0; i < DSI_MAX_CLIENT; i++) {
			if (!dsiH->client[i].open) {
				dsiH->client[i].lcdH = &dsiBus[bus];
				dsiH->client[i].open = TRUE;
				*clientH = (CSL_LCD_HANDLE)&dsiH->client[i];
				break;
			}
		}
		if (i >= DSI_MAX_CLIENT) {
			LCD_DBG(LCD_DBG_ID, "[CSL DSI][%d] %s: "
				"ERR, Max Client Count Reached[%d]\n",
				dsiH->bus, __FUNCTION__, DSI_MAX_CLIENT);
			res = CSL_LCD_INST_COUNT;
		} else {
			dsiH->clients++;
			res = CSL_LCD_OK;
		}
	}

	if (res != CSL_LCD_OK)
		*clientH = (CSL_LCD_HANDLE)NULL;
	else
		LCD_DBG(LCD_DBG_INIT_ID, "[CSL DSI][%d] %s: "
			"OK, Client Count[%d]\n",
			dsiH->bus, __FUNCTION__, dsiH->clients);

	OSSEMAPHORE_Release(dsiH->semaDsi);

	return (res);
}

//*****************************************************************************
//
// Function Name: CSL_DSI_Ulps
// 
// Description:   Enter / Exit ULPS on Clk & Data Line(s)
//                
//*****************************************************************************
CSL_LCD_RES_T CSL_DSI_Ulps(CSL_LCD_HANDLE client, Boolean on) {
	CSL_LCD_RES_T res = CSL_LCD_OK;
	DSI_HANDLE dsiH;
	DSI_CLIENT clientH;

	clientH = (DSI_CLIENT) client;
	dsiH = (DSI_HANDLE)clientH->lcdH;

	if (!clientH->hasLock)
		OSSEMAPHORE_Obtain(dsiH->semaDsi, TICKS_FOREVER);

	if (on && !dsiH->ulps) {
		chal_dsi_phy_state(dsiH->chalH, PHY_ULPS);
		dsiH->ulps = TRUE;
	} else {
		if (dsiH->ulps) {
			chal_dsi_phy_state(dsiH->chalH, PHY_CORE);
			cslDsiWaitForStatAny_Poll(dsiH,
						  CHAL_DSI_STAT_PHY_D0_STOP,
						  NULL, 10);
			dsiH->ulps = FALSE;
		}
	}

	if (!clientH->hasLock)
		OSSEMAPHORE_Release(dsiH->semaDsi);

	return (res);
}				// CSL_DSI_Ulps 

//*****************************************************************************
//                 
// Function Name:  CSL_DSI_Close
// 
// Description:    Close DSI Controller
//
//*****************************************************************************
CSL_LCD_RES_T CSL_DSI_Close(UInt32 bus) {
	DSI_HANDLE dsiH;
	CSL_LCD_RES_T res = CSL_LCD_OK;

	dsiH = (DSI_HANDLE)&dsiBus[bus];

	OSSEMAPHORE_Obtain(dsiH->semaDsi, TICKS_FOREVER);

	if (dsiH->init != DSI_INITIALIZED) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI][%d] %s: "
			"DSI Interface Not Init\n", bus, __FUNCTION__);
		res = CSL_LCD_ERR;
		goto CSL_DSI_CloseRet;
	}

	if (dsiH->clients != 0) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI][%d] %s: "
			"DSI Interface Client Count[%d] != 0\n",
			bus, __FUNCTION__, dsiH->clients);
		res = CSL_LCD_ERR;
		goto CSL_DSI_CloseRet;
	}

	chal_dsi_off(dsiH->chalH);
	chal_dsi_phy_afe_off(dsiH->chalH);
	dsiH->init = ~DSI_INITIALIZED;

	cslDsiAfeLdoSetState(dsiH, DSI_LDO_OFF);

	LCD_DBG(LCD_DBG_INIT_ID, "[CSL DSI][%d] %s: OK\n", bus, __FUNCTION__);

CSL_DSI_CloseRet:
	OSSEMAPHORE_Release(dsiH->semaDsi);

	return (res);
}

#ifdef __CSL_DSI_USE_CLK_API__
static void csl_dsi_set_chal_api_clks(DSI_HANDLE dsiH,
				      const pCSL_DSI_CFG dsiCfg) {

	dsiH->clkCfg.escClk_MHz = dsiCfg->escClk.clkIn_MHz
	    / dsiCfg->escClk.clkInDiv;
	dsiH->clkCfg.hsBitClk_MHz = dsiCfg->hsBitClk.clkIn_MHz
	    / dsiCfg->hsBitClk.clkInDiv;

	if ((UInt32)(dsiH->clkCfg.hsBitClk_MHz / 2) <= CORE_CLK_MAX_MHZ) {
		dsiH->clkCfg.coreClkSel = CHAL_DSI_BIT_CLK_DIV_BY_2;
		LCD_DBG(LCD_DBG_ID, "[CSL DSI][%d] %s: "
			"DSI CORE CLK SET TO BIT_CLK/2\n", dsiH->bus, __func__);
	} else if ((UInt32)(dsiH->clkCfg.hsBitClk_MHz / 4) <= CORE_CLK_MAX_MHZ) {
		dsiH->clkCfg.coreClkSel = CHAL_DSI_BIT_CLK_DIV_BY_4;
		LCD_DBG(LCD_DBG_ID, "[CSL DSI][%d] %s: "
			"DSI CORE CLK SET TO BIT_CLK/4\n", dsiH->bus, __func__);
	} else {
		dsiH->clkCfg.coreClkSel = CHAL_DSI_BIT_CLK_DIV_BY_8;
		LCD_DBG(LCD_DBG_ID, "[CSL DSI][%d] %s: "
			"DSI CORE CLK SET TO BIT_CLK/8\n", dsiH->bus, __func__);
	}
}
#endif // #ifdef 2__CSL_DSI_USE_CLK_API__

//*****************************************************************************
//                 
// Function Name:  CSL_DSI_Init
// 
// Description:    Init DSI Controller
//
//*****************************************************************************
CSL_LCD_RES_T CSL_DSI_Init(const pCSL_DSI_CFG dsiCfg) {
	CSL_LCD_RES_T res = CSL_LCD_OK;
	DSI_HANDLE dsiH;

	CHAL_DSI_MODE_t chalMode;
	CHAL_DSI_INIT_t chalInit;
	CHAL_DSI_AFE_CFG_t chalAfeCfg;

	if (dsiCfg->bus >= DSI_INST_COUNT) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI][%d] %s: "
			"ERR Invalid Bus Id!\n", dsiCfg->bus, __FUNCTION__);
		return (CSL_LCD_BUS_ID);
	}

	dsiH = (DSI_HANDLE)&dsiBus[dsiCfg->bus];

	if (dsiH->init == DSI_INITIALIZED) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI][%d] %s: "
			"DSI Interface Already Init\n",
			dsiCfg->bus, __FUNCTION__);
		return (CSL_LCD_IS_OPEN);
	}

	if (dsiH->initOnce != DSI_INITIALIZED) {
		memset(dsiH, 0, sizeof(DSI_HANDLE_t));

		dsiH->bus = dsiCfg->bus;

#ifdef UNDER_LINUX
		spin_lock_init(&(dsiH->bcm_dsi_spin_Lock));
#endif

		if (dsiH->bus == 0) {
			dsiH->dsiCoreRegAddr = CSL_DSI0_BASE_ADDR;
			dsiH->interruptId = CSL_DSI0_IRQ;
			dsiH->lisr = cslDsi0Stat_LISR;
			dsiH->hisr = cslDsi0Stat_HISR;
			dsiH->task = cslDsi0UpdateTask;
			dsiH->dma_cb = cslDsi0EofDma;
		} else {
			dsiH->dsiCoreRegAddr = CSL_DSI1_BASE_ADDR;
			dsiH->interruptId = CSL_DSI1_IRQ;
			dsiH->lisr = cslDsi1Stat_LISR;
			dsiH->hisr = cslDsi1Stat_HISR;
			dsiH->task = cslDsi1UpdateTask;
			dsiH->dma_cb = cslDsi1EofDma;
		}

		if (!cslDsiOsInit(dsiH)) {
			LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI][%d] %s: ERROR OS "
				"Init!\n", dsiCfg->bus, __FUNCTION__);
			return (CSL_LCD_OS_ERR);
		} else {
			dsiH->initOnce = DSI_INITIALIZED;
		}
	}
	// Init User Controlled Values
	chalInit.dlCount = dsiCfg->dlCount;
	chalInit.clkContinuous = dsiCfg->enaContClock;

	chalMode.enaContClock = dsiCfg->enaContClock;	// 2'nd time ?
	chalMode.enaRxCrc = dsiCfg->enaRxCrc;
	chalMode.enaRxEcc = dsiCfg->enaRxEcc;
	chalMode.enaHsTxEotPkt = dsiCfg->enaHsTxEotPkt;
	chalMode.enaLpTxEotPkt = dsiCfg->enaLpTxEotPkt;
	chalMode.enaLpRxEotPkt = dsiCfg->enaLpRxEotPkt;

	// Init HARD-CODED Settings
	chalAfeCfg.afeCtaAdj = 7;	// 0 - 15
	chalAfeCfg.afePtaAdj = 7;	// 0 - 15
	chalAfeCfg.afeBandGapOn = TRUE;
	chalAfeCfg.afeDs2xClkEna = FALSE;

	chalAfeCfg.afeClkIdr = 6;	// 0 - 7  DEF 6
	chalAfeCfg.afeDlIdr = 6;	// 0 - 7  DEF 6

#ifdef __CSL_DSI_USE_CLK_API__
	csl_dsi_set_chal_api_clks(dsiH, dsiCfg);
#else
	// Init Values for HS/ESC Clock Trees 
	if (!cslDsiClkTreeInit(dsiH, dsiCfg->hsBitClk, dsiCfg->escClk)) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI][%d] %s: ERROR HS/ESC "
			"Clk Init!\n", dsiCfg->bus, __FUNCTION__);
		return (CSL_LCD_ERR);
	}
#endif

	cslDsiAfeLdoSetState(dsiH, DSI_LDO_HP);

#ifndef	__CSL_DSI_USE_CLK_API__
	if (cslDsiClkTreeEna(dsiH))
#endif
	{
		dsiH->chalH = chal_dsi_init(dsiH->dsiCoreRegAddr, &chalInit);

		if (dsiH->chalH == NULL) {
			LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI][%d] %s: ERROR in "
				"cHal Init!\n", dsiCfg->bus, __FUNCTION__);
			res = CSL_LCD_ERR;
		} else {
			chal_dsi_phy_afe_on(dsiH->chalH, &chalAfeCfg);
			// as per rdb clksel must be set before ANY timing isSet
			// 0=byte clock 1=bitclk2 2=bitclk
			chalMode.clkSel = dsiH->clkCfg.coreClkSel;
			chal_dsi_on(dsiH->chalH, &chalMode);

			if (!chal_dsi_set_timing(dsiH->chalH,
						 dsiCfg->dPhySpecRev,
						 dsiH->clkCfg.coreClkSel,
						 dsiH->clkCfg.escClk_MHz,
						 dsiH->clkCfg.hsBitClk_MHz,
						 dsiCfg->lpBitRate_Mbps)) {
				LCD_DBG(LCD_DBG_ERR_ID,
					"[CSL DSI][%d] %s: ERROR In Timing "
					"Calculation!\n",
					dsiCfg->bus, __FUNCTION__);
				res = CSL_LCD_ERR;
			} else {
				chal_dsi_de1_set_dma_thresh(dsiH->chalH,
							    DE1_DEF_THRESHOLD_W);
				cslDsiClearAllFifos(dsiH);
				// wait for STOP state
				OSTASK_Sleep(TICKS_IN_MILLISECONDS(1));
			}
		}
	}
#ifndef	__CSL_DSI_USE_CLK_API__
	else {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI][%d] %s: ERROR Enabling DSI "
			"Clocks\n", dsiCfg->bus, __FUNCTION__);
		res = CSL_LCD_ERR;
	}
#endif

	if (res == CSL_LCD_OK) {
		LCD_DBG(LCD_DBG_INIT_ID, "[CSL DSI][%d] %s: OK\n",
			dsiCfg->bus, __FUNCTION__);
		dsiH->init = DSI_INITIALIZED;
		dsiH->bus = dsiCfg->bus;
	} else {
		dsiH->init = 0;
	}

	return (res);
}				// CSL_DSI_Init

#ifdef UNDER_LINUX
int Log_DebugPrintf(UInt16 logID, char *fmt, ...) {

	char p[255];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(p, 255, fmt, ap);
	va_end(ap);

	printk(p);

	return 1;
}
#endif
