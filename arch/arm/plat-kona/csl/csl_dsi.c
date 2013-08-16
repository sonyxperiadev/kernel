/*******************************************************************************
* Copyright 2011 Broadcom Corporation.  All rights reserved.
*
* @file arch/arm/plat-kona/csl/csl_dsi.c
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

#include <plat/csl/csl_dma_vc4lite.h>
#include <plat/axipv.h>
#include <plat/pv.h>
#include <mach/clock.h>

#include <plat/chal/chal_common.h>
#include <plat/chal/chal_dsi.h>
#include <plat/chal/chal_clk.h>
#include <plat/csl/csl_lcd.h>
#include <plat/csl/csl_dsi.h>

#define DSI_CORE_CLK_MAX_MHZ	125000000

#define DSI_INITIALIZED		0x13579BDF
#define DSI_INST_COUNT		(UInt32)1
#define DSI_MAX_CLIENT		4
#define DSI_CM_MAX_HANDLES	4

#define DSI_PKT_RPT_MAX		0x3FFF
#define FLUSH_Q_SIZE		1

#define TX_PKT_ENG_1		((UInt8)0)
#define TX_PKT_ENG_2		((UInt8)1)

#define TASKPRI_DSI		(TPriority_t)(ABOVE_NORMAL)
#define STACKSIZE_DSI		(STACKSIZE_BASIC * 5)
#define HISRSTACKSIZE_DSISTAT	(256 + RESERVED_STACK_FOR_LISR)

#define CSL_DSI0_IRQ		BCM_INT_ID_DSI0
#define CSL_DSI1_IRQ		BCM_INT_ID_DSI1

#define CSL_DSI0_BASE_ADDR	KONA_DSI0_VA
#define CSL_DSI1_BASE_ADDR	KONA_DSI1_VA

#if 0
#if defined(CONFIG_MACH_BCM_FPGA_E) || defined(CONFIG_MACH_BCM_FPGA)
int DE1_FIFO_SIZE_W = 256;
int CM_PKT_SIZE_B= 256 * 3;
int DE1_DEF_THRESHOLD_W = (256 * 3) >> 2;
int DE1_DEF_THRESHOLD_B = 256 * 3;
#else
#define DE1_FIFO_SIZE_W		256
#define CM_PKT_SIZE_B		(256 * 3)
#define DE1_DEF_THRESHOLD_W	DE1_FIFO_SIZE_W
#define DE1_DEF_THRESHOLD_B	(DE1_FIFO_SIZE_W * 4)
#endif
#else
#define DE1_DEF_THRESHOLD_W	(CM_PKT_SIZE_B >> 2)
#define DE1_DEF_THRESHOLD_B	(CM_PKT_SIZE_B)
#endif

#define HW_REG_WRITE(a, v) writel(v, HW_IO_PHYS_TO_VIRT(a))

#define AXIPV_MIN_BUFF_SIZE (3 * 8)


/* DSI Core clk tree configuration / settings */
typedef struct {
	UInt32 hsPllReq_MHz;	/* in:  PLL freq requested */
	UInt32 escClkIn_MHz;	/* in:  ESC clk in requested */
	UInt32 hsPllSet_MHz;	/* out: PLL freq set */
	UInt32 hsBitClk_MHz;	/* out: end HS bit clock */
	UInt32 escClk_MHz;	/* out: ESC clk after req inDIV */
	UInt32 hsClkDiv;	/* out: HS  CLK Div Reg value */
	UInt32 escClkDiv;	/* out: ESC CLK Div Reg value */
	UInt32 hsPll_P1;	/* out: PLL setting */
	UInt32 hsPll_N_int;	/* out: PLL setting */
	UInt32 hsPll_N_frac;	/* out: PLL setting */
	CHAL_DSI_CLK_SEL_t coreClkSel;	/* out: chal core_clk_sel */
} DSI_CLK_CFG_T;

/* DSI Client */
typedef struct {
	CSL_LCD_HANDLE lcdH;
	Boolean open;
	Boolean hasLock;
} DSI_CLIENT_t, *DSI_CLIENT;

/* DSI Command Mode */
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

/* DSI Interface */
typedef struct {
	CHAL_HANDLE chalH;
	UInt32 bus;
	UInt32 init;
	UInt32 initOnce;
	UInt32 initPV;
	UInt32 initAXIPV;
	Boolean ulps;
	UInt32 dsiCoreRegAddr;
	UInt32 clients;
	DSI_CLK_CFG_T clkCfg;	/* HS & ESC Clk configuration */
	Task_t updReqT;
	Queue_t updReqQ;
	Semaphore_t semaDsi;
	Semaphore_t semaInt;
	Semaphore_t semaDma;
	Semaphore_t semaAxipv;
	Semaphore_t semaPV;
	irq_handler_t lisr;
	void (*hisr) (void);
	void (*task) (void);
	void (*dma_cb) (DMA_VC4LITE_CALLBACK_STATUS);
	Interrupt_t iHisr;
	UInt32 interruptId;
	DSI_CLIENT_t client[DSI_MAX_CLIENT];
	DSI_CM_HANDLE_t chCm[DSI_CM_MAX_HANDLES];
	struct axipv_config_t *axipvCfg;
	struct pv_config_t *pvCfg;
	Boolean vmode;		/* 1 = Video Mode, 0 = Command Mode */
	UInt32 dispEngine;	/* Display Engine- 0=DE0 via Pixel Valve
						 / 1=DE1 via TXPKT_PIXD_FIFO
					:Corresponds to Display engine
					in the DSI module*/
	UInt32 pixTxporter;	/* Pixel Transporter- 0=AXIPV / 1=MMDMA
					:Corresponds to the module which
					fetches pixels and feeds DSI*/
	UInt32 dlCount;		/* No. of data lanes*/
	void (*vsync_cb)(void);	/* Function pointer for vsync events */
} DSI_HANDLE_t, *DSI_HANDLE;

typedef struct {
	DMA_VC4LITE_CHANNEL_t dmaCh;
	DSI_HANDLE dsiH;	/* CSL DSI handle */
	DSI_CLIENT clientH;	/* CSL DSI Client handle */
	CSL_LCD_UPD_REQ_T updReq;	/* update Request */
} DSI_UPD_REQ_MSG_T;

typedef enum {
	DSI_LDO_HP = 1,
	DSI_LDO_LP,
	DSI_LDO_OFF,
} DSI_LDO_STATE_t;


static DSI_HANDLE_t dsiBus[DSI_INST_COUNT];
static int videoEnabled;
static void cslDsiEnaIntEvent(DSI_HANDLE dsiH, UInt32 intEventMask);
static CSL_LCD_RES_T cslDsiWaitForStatAny_Poll(DSI_HANDLE dsiH, UInt32 mask,
		UInt32 *intStat, UInt32 tout_msec);
static CSL_LCD_RES_T cslDsiWaitForInt(DSI_HANDLE dsiH, UInt32 tout_msec);
static CSL_LCD_RES_T cslDsiDmaStart(DSI_UPD_REQ_MSG_T *updMsg);
static CSL_LCD_RES_T cslDsiDmaStop(DSI_UPD_REQ_MSG_T *updMsg);
static void cslDsiClearAllFifos(DSI_HANDLE dsiH);
static CSL_LCD_RES_T cslDsiPixTxStart(DSI_UPD_REQ_MSG_T *updMsg);
static CSL_LCD_RES_T cslDsiPixTxStop(DSI_UPD_REQ_MSG_T *updMsg);
static void cslDsiPixTxPollInt(DSI_UPD_REQ_MSG_T *updMsg);
static CSL_LCD_RES_T cslDsiAxipvStart(DSI_UPD_REQ_MSG_T *updMsg);
static CSL_LCD_RES_T cslDsiAxipvStop(DSI_UPD_REQ_MSG_T *updMsg);
static void cslDsiAxipvPollInt(DSI_UPD_REQ_MSG_T *updMsg);
static void axipv_irq_cb(int err);
static void axipv_release_cb(u32 free_buf);
static void pv_err_cb(int err);
static void pv_eof_cb(void);

#if 0
/* For debugging purposes*/
void dump_regs()
{
	int base, i;
	int reg[] = {
		0x3c200000,
		0x3c200004,
		0x3c200008,
		0x3c20000c,
		0x3c200010,
		0x3c200028,
		0x3c20002c,
		0x3c200030,
		0x3c200034,
		0x3c200038
		};
	for (i = 0; i < sizeof(reg) / 4; i++) {
		base = reg[i];
		pr_err("0x%x=0x%x\n", base, readl(HW_IO_PHYS_TO_VIRT(base)));
	}
}
#endif

static CSL_LCD_RES_T cslDsiPixTxStart(DSI_UPD_REQ_MSG_T *updMsg)
{
	if (updMsg->dsiH->pixTxporter)
		return cslDsiDmaStart(updMsg);
	else
		return cslDsiAxipvStart(updMsg);
}


static CSL_LCD_RES_T cslDsiPixTxStop(DSI_UPD_REQ_MSG_T *updMsg)
{
	if (updMsg->dsiH->pixTxporter)
		return cslDsiDmaStop(updMsg);
	else
		return cslDsiAxipvStop(updMsg);

}

void cslDsiPixTxPollInt(DSI_UPD_REQ_MSG_T *updMsg)
{
	if (updMsg->dsiH->pixTxporter)
		csl_dma_poll_int(updMsg->dmaCh);
	else
		cslDsiAxipvPollInt(updMsg);
}


static CSL_LCD_RES_T cslDsiAxipvStart(DSI_UPD_REQ_MSG_T *updMsg)
{
	struct axipv_config_t *axipvCfg = updMsg->dsiH->axipvCfg;
	struct pv_config_t *pvCfg = updMsg->dsiH->pvCfg;

	if (!axipvCfg) {
		pr_err("axipvCfg is NULL\n");
		return CSL_LCD_BAD_HANDLE;
	}
	axipvCfg->width = (updMsg->updReq.lineLenP +
				updMsg->updReq.xStrideB) * 4;
	axipvCfg->height = updMsg->updReq.lineCount;

	axipvCfg->buff.sync.addr = (u32) updMsg->updReq.buff;
	axipvCfg->buff.sync.xlen = updMsg->updReq.lineLenP * 4;
	axipvCfg->buff.sync.ylen = updMsg->updReq.lineCount;

	axipv_change_state(AXIPV_CONFIG, axipvCfg);
	if (!updMsg->dsiH->dispEngine) {
		if (!pvCfg) {
			pr_err("pvCfg is NULL\n");
			return CSL_LCD_BAD_HANDLE;
		}
		pvCfg->hact = updMsg->updReq.lineLenP;
		pvCfg->vact = updMsg->updReq.lineCount;
		pv_change_state(PV_VID_CONFIG, pvCfg);
	}
	axipv_change_state(AXIPV_START, axipvCfg);
	if (updMsg->clientH->hasLock)
		cslDsiAxipvPollInt(updMsg);
	else if (OSSTATUS_SUCCESS != OSSEMAPHORE_Obtain(updMsg->dsiH->semaAxipv,
		TICKS_IN_MILLISECONDS(100))) {
		int tx_done = axipv_check_completion(AXIPV_START, axipvCfg);
		if (tx_done < 0) {
			pr_err("Timed out waiting for PV_START_THRESH intr\n");
			axipv_change_state(AXIPV_STOP_IMM, axipvCfg);
			return CSL_LCD_ERR;
		} else {
			pr_err("csl_dsi:recovered %d\n", __LINE__);
		}
	}
	if (axipvCfg->cmd == false)
		chal_dsi_de0_enable(updMsg->dsiH->chalH, TRUE);
	if (!updMsg->dsiH->dispEngine)
		pv_change_state(PV_START, pvCfg);

	return CSL_LCD_OK;
}

static CSL_LCD_RES_T cslDsiAxipvStop(DSI_UPD_REQ_MSG_T *updMsg)
{
	struct axipv_config_t *axipvCfg = updMsg->dsiH->axipvCfg;
	struct pv_config_t *pvCfg = updMsg->dsiH->pvCfg;
	int tx_done;

	if (!axipvCfg) {
		pr_err("axipvCfg is NULL\n");
		return CSL_LCD_BAD_HANDLE;
	}
	tx_done = axipv_check_completion(AXIPV_STOP_EOF, axipvCfg);
	if (tx_done < 0) {
		if (axipv_change_state(AXIPV_STOP_IMM, axipvCfg) < 0)
			pr_err("failed to stop AXIPV\n");
		if (!updMsg->dsiH->dispEngine) {
			if (!pvCfg) {
				pr_err("csl_dsi: pvCfg is NULL %d\n", __LINE__);
				return CSL_LCD_BAD_HANDLE;
			}
			pv_change_state(PV_STOP_IMM, pvCfg);
			return CSL_LCD_OS_TOUT;
		}
	} else {
		pr_err("csl_dsi: recovered %d\n", __LINE__);
	}
	cslDsiClearAllFifos(updMsg->dsiH);
	return CSL_LCD_OK;
}

void cslDsiAxipvPollInt(DSI_UPD_REQ_MSG_T *updMsg)
{
	struct axipv_config_t *axipvCfg = updMsg->dsiH->axipvCfg;

	if (!axipvCfg) {
		pr_err("axipvCfg is NULL\n");
		return;
	}
	axipv_change_state(AXIPV_WAIT_INTR, axipvCfg);
}

static void axipv_irq_cb(int stat)
{
	static uint32_t w_lvl_2_cnt;
	DSI_HANDLE dsiH = &dsiBus[0];
	if (dsiH->vmode && (stat & AXIPV_DISABLED_INT))
		OSSEMAPHORE_Release(dsiH->semaAxipv);
	else if (stat & WATER_LVL2_INT)
		pr_err("AXIPV hit W_LVL_2 threshold %d times\n", ++w_lvl_2_cnt);
	if (stat & PV_START_THRESH_INT)
		OSSEMAPHORE_Release(dsiH->semaAxipv);
	if ((dsiH->vsync_cb != NULL) && (stat & TE_INT))
		dsiH->vsync_cb();
}

static void axipv_release_cb(u32 free_buf)
{
	DSI_HANDLE dsiH = &dsiBus[0];
	OSSEMAPHORE_Release(dsiH->semaDma);
}

static void pv_err_cb(int err)
{
	pr_err("pv_err_cb=0x%x\n", err);
}

static void pv_eof_cb()
{
	DSI_HANDLE dsiH = &dsiBus[0];
	OSSEMAPHORE_Release(dsiH->semaPV);
}

static DEFINE_SPINLOCK(lock);
static unsigned long flags;

/*
 *
 * Function Name:  cslDsi0Stat_LISR
 *
 * Description:    DSI Controller 0 LISR
 *
 */
static irqreturn_t cslDsi0Stat_LISR(int i, void *j)
{
	u32 int_status;
	DSI_HANDLE dsiH = &dsiBus[0];

	spin_lock_irqsave(&lock, flags);
	int_status = chal_dsi_get_int(dsiH->chalH);
	if (!int_status) {
		pr_err("DSI interrupt status is NULL. Hence ignoring\n");
		spin_unlock_irqrestore(&lock, flags);
		return IRQ_HANDLED;
	}
	if (int_status & (1 << 22))
		pr_info("dsi int fifo error 0x%x\n", int_status);

	chal_dsi_ena_int(dsiH->chalH, 0);
	chal_dsi_clr_int(dsiH->chalH, int_status);

	OSSEMAPHORE_Release(dsiH->semaInt);
	spin_unlock_irqrestore(&lock, flags);

	return IRQ_HANDLED;
}


/*
 *
 * Function Name:  cslDsi0EofDma
 *
 * Description:    DSI Controller 0 EOF DMA
 *
 */
static void cslDsi0EofDma(DMA_VC4LITE_CALLBACK_STATUS status)
{
	DSI_HANDLE dsiH = &dsiBus[0];

	if (status != DMA_VC4LITE_CALLBACK_SUCCESS) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI] %s: ERR DMA!\n",
			__func__);
	}

	OSSEMAPHORE_Release(dsiH->semaDma);
}


/*
 *
 * Function Name:  cslDsi0UpdateTask
 *
 * Description:    DSI Controller 0 Update Task
 *
 */
static void cslDsi0UpdateTask(void)
{
	DSI_UPD_REQ_MSG_T updMsg;
	OSStatus_t osStat;
	CSL_LCD_RES_T res;
	DSI_HANDLE dsiH = &dsiBus[0];

	for (;;) {
		res = CSL_LCD_OK;

		/* Wait for update request */
		OSQUEUE_Pend(dsiH->updReqQ, (QMsg_t *)&updMsg, TICKS_FOREVER);

		/* Wait For signal from eof DMA */
		osStat = OSSEMAPHORE_Obtain(dsiH->semaDma,
					    TICKS_IN_MILLISECONDS(updMsg.updReq.
								  timeOut_ms));

		if (osStat != OSSTATUS_SUCCESS) {
			int tx_done;
			if (osStat == OSSTATUS_TIMEOUT) {
				LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI] %s: "
					"TIMED OUT While waiting for "
					"EOF DMA\n", __func__);
				res = CSL_LCD_OS_TOUT;
			} else {
				LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI] %s: "
					"OS ERR While waiting for EOF DMA\n",
					__func__);
				res = CSL_LCD_OS_ERR;
			}

			tx_done = cslDsiPixTxStop(&updMsg);
			if (tx_done == CSL_LCD_OK) {
				osStat = OSSTATUS_SUCCESS;
				res = CSL_LCD_OK;
				pr_err("csl_dsi: recovered %d\n", __LINE__);
			} else {
				pr_err("csl_dsi: didn't recovered %d\n",
					__LINE__);
			}
		}

		if (!dsiH->vmode) {
			if (res == CSL_LCD_OK)
				res = cslDsiWaitForInt(dsiH,
						updMsg.updReq.timeOut_ms);
			else
				cslDsiWaitForInt(dsiH, 1);

			chal_dsi_tx_start(dsiH->chalH, TX_PKT_ENG_1, FALSE);
			chal_dsi_tx_start(dsiH->chalH, TX_PKT_ENG_2, FALSE);

			if (dsiH->dispEngine)
				chal_dsi_de1_enable(dsiH->chalH, FALSE);
			else
				chal_dsi_de0_enable(dsiH->chalH, FALSE);

			if (!updMsg.clientH->hasLock)
				OSSEMAPHORE_Release(dsiH->semaDsi);
		}

		if (updMsg.updReq.cslLcdCb)
			updMsg.updReq.cslLcdCb(res, &updMsg.updReq.cslLcdCbRec);
		else
			LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI] %s: "
				"Callback EQ NULL, Skipping\n", __func__);
	}
}


/*
 *
 * Function Name:  cslDsiOsInit
 *
 * Description:    DSI COntroller OS Interface Init
 *
 */
Boolean cslDsiOsInit(DSI_HANDLE dsiH)
{
	Boolean res = TRUE;
	int ret;

	/* Update Request Queue */
	dsiH->updReqQ = OSQUEUE_Create(FLUSH_Q_SIZE,
			sizeof(DSI_UPD_REQ_MSG_T),
			OSSUSPEND_PRIORITY);
	if (!dsiH->updReqQ) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI] %s: OSQUEUE_Create failed\n",
			__func__);
		res = FALSE;
	} else {
		OSQUEUE_ChangeName(dsiH->updReqQ,
				   dsiH->bus ? "Dsi1Q" : "Dsi0Q");
	}

	/* Update Request Task */
	dsiH->updReqT = OSTASK_Create(dsiH->task,
			dsiH->
			bus ? (TName_t) "Dsi1T" : (TName_t)
			"Dsi0T", TASKPRI_DSI, STACKSIZE_DSI);
	if (!dsiH->updReqT) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI] %s: Create Task failure\n",
			__func__);
		res = FALSE;
	}
	/* DSI Interface Semaphore */
	dsiH->semaDsi = OSSEMAPHORE_Create(1, OSSUSPEND_PRIORITY);
	if (!dsiH->semaDsi) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI] %s: ERR Sema Creation!\n",
			__func__);
		res = FALSE;
	} else {
		OSSEMAPHORE_ChangeName(dsiH->semaDsi,
				dsiH->bus ? "Dsi1" : "Dsi0");
	}

	/* DSI Interrupt Event Semaphore */
	dsiH->semaInt = OSSEMAPHORE_Create(0, OSSUSPEND_PRIORITY);
	if (!dsiH->semaInt) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI] %s: ERR Sema Creation!\n",
			__func__);
		res = FALSE;
	} else {
		OSSEMAPHORE_ChangeName(dsiH->semaInt,
				dsiH->bus ? "Dsi1Int" : "Dsi0Int");
	}

	/* EndOfDma Semaphore */
	dsiH->semaDma = OSSEMAPHORE_Create(0, OSSUSPEND_PRIORITY);
	if (!dsiH->semaDma) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI] %s: ERR Sema Creation!\n",
			__func__);
		res = FALSE;
	} else {
		OSSEMAPHORE_ChangeName(dsiH->semaDma,
				dsiH->bus ? "Dsi1Dma" : "Dsi0Dma");
	}

	/* Axipv Semaphore */
	dsiH->semaAxipv = OSSEMAPHORE_Create(0, OSSUSPEND_PRIORITY);
	if (!dsiH->semaAxipv) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI] semaAxipv creation error\n");
		res = FALSE;
	} else {
		OSSEMAPHORE_ChangeName(dsiH->semaAxipv,
				dsiH->bus ? "Dsi1Axipv" : "Dsi0Axipv");
	}

	/* PV Semaphore */
	dsiH->semaPV = OSSEMAPHORE_Create(0, OSSUSPEND_PRIORITY);
	if (!dsiH->semaPV) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI] semaPV creation error\n");
		res = FALSE;
	} else {
		OSSEMAPHORE_ChangeName(dsiH->semaPV,
				dsiH->bus ? "Dsi1PV" : "Dsi0PV");
	}

	ret = request_irq(dsiH->interruptId, dsiH->lisr, IRQF_DISABLED |
			  IRQF_NO_SUSPEND, "BRCM DSI CSL", NULL);
	if (ret < 0) {
		pr_err("%s(%s:%u)::request_irq failed IRQ %d\n",
		       __func__, __FILE__, __LINE__, dsiH->interruptId);
		goto free_irq;
	}
#ifdef CONFIG_SMP
	irq_set_affinity(dsiH->interruptId, cpumask_of(0));
#endif

	return res;

free_irq:
	free_irq(dsiH->interruptId, NULL);
	return res;
}

/*
 *
 * Function Name:   cslDsiDmaStart
 *
 * Description:     RHEA MM DMA
 *                  Prepare & Start DMA Transfer
 *                  FOR NOW - ONLY LINEAR MODE SUPPORTED
 *
 */
static CSL_LCD_RES_T cslDsiDmaStart(DSI_UPD_REQ_MSG_T *updMsg)
{
	CSL_LCD_RES_T result = CSL_LCD_OK;

	DMA_VC4LITE_CHANNEL_INFO_t dmaChInfo;
	DMA_VC4LITE_XFER_DATA_t     dmaData1D;
	DMA_VC4LITE_XFER_2DDATA_t   dmaData2D;
	Int32 dmaCh;
	UInt32 width;
	UInt32 height;
	UInt32 spare_pix;

	if (updMsg->dsiH->bus)
		dmaChInfo.dstID = DMA_VC4LITE_CLIENT_DSI1;
	else
		dmaChInfo.dstID = DMA_VC4LITE_CLIENT_DSI0;

	/* Reserve Channel */
	dmaCh =
	    csl_dma_vc4lite_obtain_channel(
		DMA_VC4LITE_CLIENT_MEMORY, dmaChInfo.dstID);

	if (dmaCh == -1) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI] %s: ERR Reserving "
			"DMA Ch\n", __func__);
		return CSL_LCD_DMA_ERR;
	}
	updMsg->dmaCh = (DMA_VC4LITE_CHANNEL_t)dmaCh;

	LCD_DBG(LCD_DBG_ID, "[CSL DSI] %s: Got DmaCh[%d]\n ",
		__func__, dmaCh);

	/* Configure Channel */
	dmaChInfo.autoFreeChan = 1;
	dmaChInfo.srcID = DMA_VC4LITE_CLIENT_MEMORY;

	dmaChInfo.burstLen = DMA_VC4LITE_BURST_LENGTH_8;
	dmaChInfo.xferMode = DMA_VC4LITE_XFER_MODE_LINERA;
	dmaChInfo.dstStride = 0;
	dmaChInfo.srcStride  = (updMsg->updReq.buffBpp *
				updMsg->updReq.xStrideB);
	dmaChInfo.waitResponse = 0;
	dmaChInfo.callback = (DMA_VC4LITE_CALLBACK_t) updMsg->dsiH->dma_cb;

	if (csl_dma_vc4lite_config_channel(updMsg->dmaCh, &dmaChInfo)
	    != DMA_VC4LITE_STATUS_SUCCESS) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI] %s: "
			"ERR Configure DMA Ch\n ", __func__);
		return CSL_LCD_DMA_ERR;
	}

	/*
	If the number of pixels is !x2 for 565 or !x4 for 888
	 1. Create a 1D configuration for remaining bytes of that row
	 2. Change the window parameters so that we can continue with
	 remaining pixels in 2D mode as before.
	*/
	spare_pix = (updMsg->updReq.lineLenP * updMsg->updReq.lineCount) &
			(updMsg->updReq.buffBpp  - 1);
	if (spare_pix) {
		uint32_t lines_to_skip;

		lines_to_skip = spare_pix / updMsg->updReq.lineLenP;
		updMsg->updReq.buff = (void *)((UInt32)updMsg->updReq.buff +
					(updMsg->updReq.buffBpp * lines_to_skip*
					(updMsg->updReq.lineLenP +
					updMsg->updReq.xStrideB)));
		updMsg->updReq.lineCount -= lines_to_skip;
		spare_pix %= updMsg->updReq.lineLenP;

		if (spare_pix > 0) {
			if (spare_pix < updMsg->updReq.lineLenP) {
				/* Add the DMA transfer data */
				dmaData1D.srcAddr  = (UInt32)updMsg->updReq.buff
					+ (spare_pix * updMsg->updReq.buffBpp);
				dmaData1D.dstAddr =
				chal_dsi_de1_get_dma_address(
				updMsg->dsiH->chalH);
				dmaData1D.xferLength  = (updMsg->updReq.lineLenP
					- spare_pix) * (updMsg->updReq.buffBpp);
				dmaData1D.burstWriteEnable32 = 0;

				if ((uint32_t)dmaData1D.xferLength & 0x3)
					pr_info("xferlength unaligned 0x%x\n",
						dmaData1D.xferLength);

				if (csl_dma_vc4lite_add_data(updMsg->dmaCh,
					&dmaData1D) !=
					DMA_VC4LITE_STATUS_SUCCESS) {
					LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI] %s:"
					"ERR add 1D DMA transfer data\n",
					__func__);
					return CSL_LCD_DMA_ERR;
				}
				updMsg->updReq.buff =
					(void *)((UInt32)updMsg->updReq.buff +
					(updMsg->updReq.buffBpp *
					(updMsg->updReq.lineLenP +
					updMsg->updReq.xStrideB)));
				updMsg->updReq.lineCount--;
			} else {
				pr_info("spare_pix=%d, linelenp=%d\n",
					spare_pix, updMsg->updReq.lineLenP);
			}
		}
	}

	width = updMsg->updReq.lineLenP * updMsg->updReq.buffBpp;
	height = updMsg->updReq.lineCount;
	dmaChInfo.xferMode     = DMA_VC4LITE_XFER_MODE_2D;

	/* Add the DMA transfer data */
	dmaData2D.srcAddr     = (UInt32)updMsg->updReq.buff;
	if ((uint32_t)dmaData2D.srcAddr & 0x3)
		pr_info("src addr unaligned %d\n", dmaData2D.srcAddr);
	dmaData2D.dstAddr  = chal_dsi_de1_get_dma_address(updMsg->dsiH->chalH);
	dmaData2D.xXferLength = width;
	dmaData2D.yXferLength = height - 1;
	dmaData2D.burstWriteEnable32 = 0;

	if ((uint32_t)dmaData2D.xXferLength & 0x3) {
		pr_info("xXferLength unaligned 0x%x stride=0x%x\n",
		dmaData2D.xXferLength, dmaChInfo.srcStride);
	}
	if (csl_dma_vc4lite_add_data_ex(updMsg->dmaCh, &dmaData2D)
	    != DMA_VC4LITE_STATUS_SUCCESS) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI] %s: "
			"ERR add DMA transfer data\n ", __func__);
		return CSL_LCD_DMA_ERR;
	}
	/* start DMA transfer */
	if (csl_dma_vc4lite_start_transfer(updMsg->dmaCh)
	    != DMA_VC4LITE_STATUS_SUCCESS) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI] %s: "
			"ERR start DMA data transfer\n ", __func__);
		return CSL_LCD_DMA_ERR;
	}

	return result;
}

/*
 *
 * Function Name:   cslDsiDmaStop
 *
 * Description:     RHEA MM DMA - Stop DMA
 *
 */
static CSL_LCD_RES_T cslDsiDmaStop(DSI_UPD_REQ_MSG_T *updMsg)
{
	CSL_LCD_RES_T res = CSL_LCD_ERR;

	LCD_DBG(LCD_DBG_ID, "[CSL DSI] +cslDsiDmaStop\n");

	/* stop DMA transfer */
	if (csl_dma_vc4lite_stop_transfer(updMsg->dmaCh)
	    != DMA_VC4LITE_STATUS_SUCCESS) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI] %s: "
			"ERR DMA Stop Transfer\n ", __func__);
		res = CSL_LCD_DMA_ERR;
	}
	/* release DMA channel */
	if (csl_dma_vc4lite_release_channel(updMsg->dmaCh)
	    != DMA_VC4LITE_STATUS_SUCCESS) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI] %s: "
			"ERR ERR DMA Release Channel\n ", __func__);
		res = CSL_LCD_DMA_ERR;
	}

	cslDsiClearAllFifos(updMsg->dsiH);
	updMsg->dmaCh = DMA_VC4LITE_CHANNEL_INVALID;

	LCD_DBG(LCD_DBG_ID, "[CSL DSI] -cslDsiDmaStop\n");

	return res;
}

/*
 *
 * Function Name:  cslDsiAfeLdoSetState
 *
 * Description:    AFE LDO Control
 *
 */
static void cslDsiAfeLdoSetState(DSI_HANDLE dsiH, DSI_LDO_STATE_t state)
{
#define DSI_LDO_HP_EN	  0x00000001
#define DSI_LDO_LP_EN	  0x00000002
#define DSI_LDO_CNTL_ENA  0x00000004
#define DSI_LDO_ISO_OUT	  0x00800000

	unsigned long ldo_val = 0;

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
			"ERROR Invalid LDO State[%d] !\r\n", __func__,
			state);
		break;
	}

	if (dsiH->bus == 0)
		HW_REG_WRITE(0x3C004030, ldo_val);
	else
		HW_REG_WRITE(0x3C004034, ldo_val);

	OSTASK_Sleep(TICKS_IN_MILLISECONDS(1));
}

/*
 *
 * Function Name:  cslDsiWaitForStatAny_Poll
 *
 * Description:
 *
 */
static CSL_LCD_RES_T cslDsiWaitForStatAny_Poll(DSI_HANDLE dsiH,
		UInt32 statMask, UInt32 *intStat, UInt32 mSec)
{
	CSL_LCD_RES_T res = CSL_LCD_OK;
	UInt32 stat = 0;
	u32 counter = 0;

	stat = chal_dsi_get_status(dsiH->chalH);

	while ((stat & statMask) == 0) {
		if (counter > 100000000) {
			WARN("%s: DSI gets stuck\n", __func__);
			break;
		}
		counter++;
		stat = chal_dsi_get_status(dsiH->chalH);
	}

	if (intStat != NULL)
		*intStat = stat;

	return res;
}

/*
 *
 * Function Name:  cslDsiClearAllFifos
 *
 * Description:    Clear All DSI FIFOs
 *
 */
static void cslDsiClearAllFifos(DSI_HANDLE dsiH)
{
	UInt32 fifoMask;

	fifoMask = CHAL_DSI_CTRL_CLR_LANED_FIFO
	    | CHAL_DSI_CTRL_CLR_RXPKT_FIFO
	    | CHAL_DSI_CTRL_CLR_PIX_DATA_FIFO | CHAL_DSI_CTRL_CLR_CMD_DATA_FIFO;
	chal_dsi_clr_fifo(dsiH->chalH, fifoMask);
}

/*
 *
 * Function Name:  cslDsiEnaIntEvent
 *
 * Description:    event bits set to "1" will be enabled,
 *                 rest of the events will be disabled
 *
 */
static void cslDsiEnaIntEvent(DSI_HANDLE dsiH, UInt32 intEventMask)
{
	chal_dsi_ena_int(dsiH->chalH, intEventMask);
}

/*
 *
 * Function Name:  cslDsiDisInt
 *
 * Description:
 *
 */
static void cslDsiDisInt(DSI_HANDLE dsiH)
{
	chal_dsi_ena_int(dsiH->chalH, 0);
	chal_dsi_clr_int(dsiH->chalH, 0xFFFFFFFF);
}


static inline int cslDsiCheckCompletion(DSI_HANDLE dsiH)
{
	int ret = -1;
	u32 int_status, enabled_int;
#if 0
	pr_err("stat = 0x%x\n", readl(HW_IO_PHYS_TO_VIRT(0x3c200028)));
	pr_err("istat= 0x%x\n", readl(HW_IO_PHYS_TO_VIRT(0x3c200030)));
	pr_err("ien= 0x%x\n", readl(HW_IO_PHYS_TO_VIRT(0x3c200034)));
	pr_err("pktc1= 0x%x\n", readl(HW_IO_PHYS_TO_VIRT(0x3c200004)));
#endif
	spin_lock_irqsave(&lock, flags);

	/* Similar to cslDsi0Stat_LISR but doesn't signal the semaphore */
	int_status = chal_dsi_get_int(dsiH->chalH);
	if (!int_status) {
		pr_info("Intr stat is NULL\n");
		ret = -1;
		goto done;
	}
	if (int_status & (1 << 22))
		pr_info("dsi int fifo error 0x%x\n", int_status);

	enabled_int = chal_dsi_get_ena_int(dsiH->chalH);
	if (int_status & enabled_int) {
		pr_err("DSI ISR was pending\n");
		chal_dsi_ena_int(dsiH->chalH, 0);
		chal_dsi_clr_int(dsiH->chalH, int_status);
		ret = 0;
		goto done;
	}
done:
	spin_unlock_irqrestore(&lock, flags);

	return ret;
}

/*
 *
 * Function Name:  cslDsiWaitForInt
 *
 * Description:
 *
 */
static CSL_LCD_RES_T cslDsiWaitForInt(DSI_HANDLE dsiH, UInt32 tout_msec)
{
	OSStatus_t osRes;
	CSL_LCD_RES_T res = CSL_LCD_OK;

	osRes =
	    OSSEMAPHORE_Obtain(dsiH->semaInt, TICKS_IN_MILLISECONDS(tout_msec));

	if (osRes != OSSTATUS_SUCCESS) {
		int tx_done;
		if (osRes == OSSTATUS_TIMEOUT) {
			LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI] %s: "
				"ERR Timed Out!\n", __func__);
			res = CSL_LCD_OS_TOUT;
		} else {
			LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI] %s: "
				"ERR OS Err...!\n", __func__);
			res = CSL_LCD_OS_ERR;
		}
		/* If DSI interrupt was blocked */
		tx_done = cslDsiCheckCompletion(dsiH);
		if (tx_done == 0) {
			pr_err("csl_dsi: DSI recovered %d\n", __LINE__);
			res = CSL_LCD_OK;
			goto done;
		}
		cslDsiDisInt(dsiH);
	}
done:
	return res;
}

#if 0
/*
 *
 * Function Name:  cslDsiBtaRecover
 *
 * Description:    Recover From Failed BTA
 *
 */
static CSL_LCD_RES_T cslDsiBtaRecover(DSI_HANDLE dsiH)
{
	CSL_LCD_RES_T res = CSL_LCD_OK;

	chal_dsi_phy_state(dsiH->chalH, PHY_TXSTOP);

	OSTASK_Sleep(TICKS_IN_MILLISECONDS(1));

	chal_dsi_clr_fifo(dsiH->chalH,
			  CHAL_DSI_CTRL_CLR_CMD_DATA_FIFO |
			  CHAL_DSI_CTRL_CLR_LANED_FIFO);

	chal_dsi_phy_state(dsiH->chalH, PHY_CORE);
	return res;
}
#endif

/*
 *
 * Function Name:  CSL_DSI_Lock
 *
 * Description:    Lock DSI Interface For Exclusive Use By a Client
 *
 */
void CSL_DSI_Lock(CSL_LCD_HANDLE vcH)
{
	DSI_HANDLE dsiH;
	DSI_CLIENT clientH;
	DSI_CM_HANDLE dsiChH;

	dsiChH = (DSI_CM_HANDLE) vcH;
	clientH = (DSI_CLIENT) dsiChH->client;
	dsiH = (DSI_HANDLE)clientH->lcdH;

	if (clientH->hasLock)
		WARN(TRUE, "[CSL DSI][%d] %s: "
		     "DSI Client Lock/Unlock Not balanced\n",
		     dsiH->bus, __func__);
	else
		clientH->hasLock = TRUE;
}

/*
 *
 * Function Name:  CSL_DSI_Unlock
 *
 * Description:    Release Client's DSI Interface Lock
 *
 */
void CSL_DSI_Unlock(CSL_LCD_HANDLE vcH)
{

	DSI_HANDLE dsiH;
	DSI_CLIENT clientH;
	DSI_CM_HANDLE dsiChH;

	dsiChH = (DSI_CM_HANDLE) vcH;
	clientH = (DSI_CLIENT) dsiChH->client;
	dsiH = (DSI_HANDLE)clientH->lcdH;

	if (!clientH->hasLock)
		WARN(TRUE, "[CSL DSI][%d] %s: "
		     "DSI Client Lock/Unlock Not balanced\n",
		     dsiH->bus, __func__);
	else
		clientH->hasLock = FALSE;
}

/*
 *
 * Function Name:  CSL_DSI_SendPacketTrigger
 *
 * Description:    Send TRIGGER Message
 *
 */
CSL_LCD_RES_T CSL_DSI_SendTrigger(CSL_LCD_HANDLE client, UInt8 trig)
{
	DSI_HANDLE dsiH;
	DSI_CLIENT clientH;
	CSL_LCD_RES_T res;

	clientH = (DSI_CLIENT) client;
	dsiH = (DSI_HANDLE)clientH->lcdH;

	if (!clientH->hasLock)
		OSSEMAPHORE_Obtain(dsiH->semaDsi, TICKS_FOREVER);

	if (dsiH->ulps) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI][%d] %s: "
			"ERR, Link Is In ULPS\n", dsiH->bus, __func__);
		res = CSL_LCD_BAD_STATE;
		goto exit_err;
	}

	chal_dsi_clr_status(dsiH->chalH, 0xFFFFFFFF);

	if (!clientH->hasLock)
		cslDsiEnaIntEvent(dsiH, (UInt32)CHAL_DSI_ISTAT_TXPKT1_DONE);

	chal_dsi_tx_trig(dsiH->chalH, TX_PKT_ENG_1, trig);


	if (!clientH->hasLock)
		res = cslDsiWaitForInt(dsiH, 100);
	else {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI][%d] %s: "
			"In Atomic wait for DSI/DMA finish...\n",
			dsiH->bus, __func__);

		res = cslDsiWaitForStatAny_Poll(dsiH,
					CHAL_DSI_STAT_TXPKT1_DONE, NULL, 100);
	}

exit_err:
	if (!clientH->hasLock)
		OSSEMAPHORE_Release(dsiH->semaDsi);

	return res;
}

#define WAIT_FOR_FIFO_FLUSH_US 10
#define WAIT_GENERAL_US 10
#define WAIT_FOR_RETRY_CNT 20
#define WAIT_FOR_RX_PKT_CNT 50
#define PKT1_STAT_MASK (CHAL_DSI_STAT_TXPKT1_END	\
			| CHAL_DSI_STAT_TXPKT1_DONE	\
			| CHAL_DSI_STAT_TXPKT1_BUSY)
#define PKT_RX_STAT_MASK (CHAL_DSI_STAT_RX1_PKT | CHAL_DSI_STAT_RX2_PKT)

/*
 *
 * Function Name:  CSL_DSI_SendPacket
 *
 * Description:    Send DSI Packet (non-pixel data) with an option to end it
 *                 with BTA.
 *                 If BTA is requested, command.reply MUST be valid
 *
 */
CSL_LCD_RES_T CSL_DSI_SendPacket(CSL_LCD_HANDLE client,
		pCSL_DSI_CMND command, Boolean isTE)
{
	DSI_HANDLE dsiH;
	DSI_CLIENT clientH;
	CSL_LCD_RES_T res = CSL_LCD_OK;
	CHAL_DSI_TX_CFG_t txPkt;
	CHAL_DSI_RES_t chalRes;
	UInt32 stat;
	UInt32 event;
	UInt32 pfifo_len = 0;
	int pkt_to_be_enabled;

	clientH = (DSI_CLIENT) client;
	dsiH = (DSI_HANDLE)clientH->lcdH;

	if (command->msgLen > CHAL_DSI_TX_MSG_MAX) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI][%d] %s: "
			"ERR, TX Packet Size To Big\n",
			dsiH->bus, __func__, command->vc);
		return CSL_LCD_MSG_SIZE;
	}
	if (command->endWithBta && (command->reply == NULL)) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI][%d] %s: "
			"ERR, VC[%d] BTA Requested But pReply Eq NULL\n",
			dsiH->bus, __func__, command->vc);
		return CSL_LCD_API_ERR;
	}

	if (!clientH->hasLock) {
		OSSEMAPHORE_Obtain(dsiH->semaDsi, TICKS_FOREVER);
	}

	if (dsiH->ulps) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI][%d] %s: "
			"ERR, VC[%d] Link Is In ULPS\n",
			dsiH->bus, __func__, command->vc);
		if (!clientH->hasLock)
			OSSEMAPHORE_Release(dsiH->semaDsi);
		return CSL_LCD_BAD_STATE;
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
	/* Don't start here if PV is enabled,
	 * instead wait for video stream to stop */
	txPkt.start = !videoEnabled;
	pkt_to_be_enabled = !txPkt.start;
	txPkt.dispEngine = 1;

	chal_dsi_clr_status(dsiH->chalH, 0xFFFFFFFF);

	if (!clientH->hasLock) {
		u32 dsi_stat, dsi_i_stat;
		if (txPkt.endWithBta) {
			event = CHAL_DSI_ISTAT_PHY_RX_TRIG
			    | CHAL_DSI_ISTAT_RX2_PKT | CHAL_DSI_ISTAT_RX1_PKT;
		} else {
			event = CHAL_DSI_ISTAT_TXPKT1_DONE;
		}
		dsi_stat = chal_dsi_get_status(dsiH->chalH);
		dsi_i_stat = chal_dsi_get_int(dsiH->chalH);
		if ((dsi_stat | dsi_i_stat) & PKT1_STAT_MASK)
			pr_err("dsi_stat=0x%x dsi_i_stat=0x%x\n",
				dsi_stat, dsi_i_stat);
		if (txPkt.start)
			cslDsiEnaIntEvent(dsiH, event);
	} else {
		if (txPkt.endWithBta) {
			event = CHAL_DSI_STAT_PHY_RX_TRIG
			    | CHAL_DSI_STAT_RX2_PKT | CHAL_DSI_STAT_RX1_PKT;
		} else {
			event = CHAL_DSI_STAT_TXPKT1_DONE;
		}
	}

	if (txPkt.msgLen <= 2) {
		LCD_DBG(LCD_DBG_ID, "[CSL DSI][%d] %s: "
			"SHORT, MSG_LEN[%d]\n",
			dsiH->bus, __func__, txPkt.msgLen);
		txPkt.msgLenCFifo = 0;	/* NA to short */
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
				dsiH->bus, __func__,
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
				dsiH->bus, __func__,
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

			axipv_release_pixdfifo_ownership(dsiH->axipvCfg);
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

	if (pkt_to_be_enabled) {
		u32 cnt2 = 0;
		u32 dsi_stat, int_status, int_en_mask;
		unsigned long pkt_flags;
		pv_change_state(PV_PAUSE_STREAM_SYNC, dsiH->pvCfg);
		local_irq_save(pkt_flags);
		udelay(WAIT_FOR_FIFO_FLUSH_US);
		dsi_stat = chal_dsi_get_status(dsiH->chalH);
		if (dsi_stat & PKT1_STAT_MASK) {
			chal_dsi_clr_status(dsiH->chalH, PKT1_STAT_MASK);
			pr_err("DSI status not cleared %d\n", __LINE__);
			dsi_stat = chal_dsi_get_status(dsiH->chalH);
			if (dsi_stat & PKT1_STAT_MASK)
				pr_err("DSI status not cleared %d\n", __LINE__);
		}
		int_status = chal_dsi_get_int(dsiH->chalH);
		int_en_mask = chal_dsi_get_ena_int(dsiH->chalH);

		if (int_status || int_en_mask) {
			pr_err("DSI Error: int_status=0x%x int_en_mask=0x%x\n",
				int_status, int_en_mask);
		}
		cslDsiDisInt(dsiH);
		chal_dsi_de0_enable(dsiH->chalH, FALSE);
		udelay(WAIT_GENERAL_US);
		chal_dsi_tx_start(dsiH->chalH, TX_PKT_ENG_1, TRUE);
start_tx:
		udelay(WAIT_GENERAL_US);
		dsi_stat = chal_dsi_get_status(dsiH->chalH) & PKT1_STAT_MASK;
		if (!dsi_stat) {
			cnt2++;
			if ((cnt2 % WAIT_FOR_RETRY_CNT) != 0)
				goto start_tx;
			/* Retry after 10*20 = 200us */
			chal_dsi_tx_start(dsiH->chalH, TX_PKT_ENG_1, FALSE);
			udelay(WAIT_GENERAL_US);
			chal_dsi_tx_start(dsiH->chalH, TX_PKT_ENG_1, TRUE);
			goto start_tx;
		}
		while (dsi_stat & CHAL_DSI_STAT_TXPKT1_BUSY) {
			udelay(WAIT_GENERAL_US);
			dsi_stat = chal_dsi_get_status(dsiH->chalH);
		}
		if (!(dsi_stat & PKT1_STAT_MASK))
			pr_err("something fishy\n");
		chal_dsi_clr_status(dsiH->chalH, PKT1_STAT_MASK);
		dsi_stat = chal_dsi_get_status(dsiH->chalH);
		if (dsi_stat & PKT1_STAT_MASK) {
			pr_err("status not cleared %d\n", __LINE__);
			chal_dsi_clr_status(dsiH->chalH, PKT1_STAT_MASK);
		}
		/* Packet transfer is complete */
		if (txPkt.endWithBta) {
			/* wait for 50 * 10 = 500us to receive the pkt */
			cnt2 = WAIT_FOR_RX_PKT_CNT;
			do {
				udelay(WAIT_GENERAL_US);
				stat = chal_dsi_get_status(dsiH->chalH);
			} while (!(stat & PKT_RX_STAT_MASK) && --cnt2);
			if (!cnt2)
				pr_err("CSL_DSI: No pkt was received!\n");
		}

		chal_dsi_de0_enable(dsiH->chalH, TRUE);
		udelay(WAIT_GENERAL_US);
		pv_change_state(PV_RESUME_STREAM, dsiH->pvCfg);
		local_irq_restore(pkt_flags);
		if (txPkt.endWithBta) {
			chalRes = chal_dsi_read_reply(dsiH->chalH, stat,
						      (pCHAL_DSI_REPLY)
						      command->reply);
			if (chalRes == CHAL_DSI_RX_NO_PKT) {
				pr_err("BTA- No Data Received\n");
				res = CSL_LCD_INT_ERR;
			}
		}
		goto exit_ok;
	}


	if (!clientH->hasLock) {
		res = cslDsiWaitForInt(dsiH, 100);
		stat = chal_dsi_get_status(dsiH->chalH);
	} else {
		res = cslDsiWaitForStatAny_Poll(dsiH, event, &stat, 100);
	}

	if (txPkt.endWithBta) {
		if (res != CSL_LCD_OK) {
			int tries = 3;
			LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI][%d] %s: "
				"WARNING, VC[%d] Probable BTA TimeOut, "
				"Recovering ...\n",
				dsiH->bus, __func__, txPkt.vc);
			/* This clears the FIFO and puts the Controller in STOP
			 * state! */
			/* cslDsiBtaRecover(dsiH); */
			while ((res == CSL_LCD_OS_TOUT) && --tries) {
				pr_err("Trying once more\n");
				if (!clientH->hasLock)
					cslDsiEnaIntEvent(dsiH, event);
				chal_dsi_tx_start(dsiH->chalH, TX_PKT_ENG_1,
						FALSE);
				if (txPkt.start)
					chal_dsi_tx_start(dsiH->chalH,
						TX_PKT_ENG_1, TRUE);
				pkt_to_be_enabled = !txPkt.start;
				if (!clientH->hasLock) {
					cslDsiEnaIntEvent(dsiH, event);
					res = cslDsiWaitForInt(dsiH, 100);
					stat = chal_dsi_get_status(dsiH->chalH);
				} else {
					res = cslDsiWaitForStatAny_Poll(dsiH,
						event, &stat, 100);
				}
			}
			if (!tries)
				pr_err("Couldn't recover!\n");
			else
				goto read_reply;
		} else {
read_reply:
			chalRes = chal_dsi_read_reply(dsiH->chalH, stat,
						      (pCHAL_DSI_REPLY)
						      command->reply);
			if (chalRes == CHAL_DSI_RX_NO_PKT) {
				LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI][%d] %s: "
					"WARNING, VC[%d] BTA "
					"No Data Received\n",
					dsiH->bus, __func__, txPkt.vc);
				res = CSL_LCD_INT_ERR;
				goto exit_ok;
			} else {
				goto exit_ok;
			}
		}
	} else {
		if (res != CSL_LCD_OK) {
			int tries = 3;
			LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI][%d] %s: "
				"WARNING, VC[%d] "
				"Timed Out Waiting For TX end\n",
				dsiH->bus, __func__, txPkt.vc);
			while ((res == CSL_LCD_OS_TOUT) && --tries) {
				printk("Trying once more\n");
				if (!clientH->hasLock)
					cslDsiEnaIntEvent(dsiH, event);
				chal_dsi_tx_start(dsiH->chalH, TX_PKT_ENG_1,
						FALSE);
				if (pkt_to_be_enabled)
					pr_err("PV hasn't trigerred!\n");
				if (txPkt.start)
					chal_dsi_tx_start(dsiH->chalH,
						TX_PKT_ENG_1, TRUE);
				pkt_to_be_enabled = !txPkt.start;
				if (!clientH->hasLock) {
					cslDsiEnaIntEvent(dsiH, event);
					res = cslDsiWaitForInt(dsiH, 100);
					stat = chal_dsi_get_status(dsiH->chalH);
				} else {
					res = cslDsiWaitForStatAny_Poll(dsiH,
						event, &stat, 100);
				}
			}
			if (!tries)
				printk("Couldn't recover!\n");
			else
				goto exit_ok;
		} else {
			goto exit_ok;
		}
	}

exit_err:
	LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI][%d] %s: ERR, CSL_LCD_RES[%d]\n",
		dsiH->bus, __func__, res);

	cslDsiDisInt(dsiH);
	cslDsiClearAllFifos(dsiH);
exit_ok:
	pkt_to_be_enabled = 0;
	chal_dsi_de1_enable(dsiH->chalH, FALSE);
	chal_dsi_tx_start(dsiH->chalH, TX_PKT_ENG_1, FALSE);

	if (pfifo_len > DE1_DEF_THRESHOLD_B)
		chal_dsi_de1_set_dma_thresh(dsiH->chalH, DE1_DEF_THRESHOLD_W);

	if (!clientH->hasLock)
		OSSEMAPHORE_Release(dsiH->semaDsi);
	return res;
}

/*
 *
 * Function Name:  CSL_DSI_OpenCmVc
 *
 * Description:    Open (configure) Command Mode VC
 *                 Returns VC Command Mode handle
 *
 */
CSL_LCD_RES_T CSL_DSI_OpenCmVc(CSL_LCD_HANDLE client,
		pCSL_DSI_CM_VC dsiCmVcCfg,
		CSL_LCD_HANDLE *dsiCmVcH)
{
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
			dsiH->bus, __func__);
		res = CSL_LCD_ERR;
		goto exit_err;
	default:
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI][%d] %s: "
			"ERR, Invalid TE Config Type [%d]\n",
			dsiH->bus, __func__, dsiCmVcCfg->teCfg.teInType);
		res = CSL_LCD_ERR;
		goto exit_err;
	}

	switch (dsiCmVcCfg->cm_in) {
		/* 1x888 pixel per 32-bit word (MSB DontCare) */
	case LCD_IF_CM_I_xBGR8888:
	case LCD_IF_CM_I_xRGB8888:
		switch (dsiCmVcCfg->cm_out) {
		case LCD_IF_CM_O_RGB666:
		case LCD_IF_CM_O_xRGB8888:
			cmVcH->bpp_dma = 4;
			cmVcH->bpp_wire = 3;
			cmVcH->wc_rshift = 0;
			cmVcH->cm = dsiH->dispEngine ?
					DE1_CM_888U : DE0_CM_888U;
			if (!dsiH->pixTxporter)
				dsiH->axipvCfg->pix_fmt = (dsiCmVcCfg->cm_in ==
							LCD_IF_CM_I_xBGR8888) ?
						AXIPV_PIXEL_FORMAT_24BPP_BGR :
						AXIPV_PIXEL_FORMAT_24BPP_RGB;
			if (!dsiH->dispEngine)
				dsiH->pvCfg->pix_fmt = DSI_VIDEO_CMD_18_24BPP;
			break;
		default:
			LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI][%d] %s: "
				"ERR, Invalid OutCol Mode[%d] for "
				"InCol %d\n", dsiH->bus, __func__,
				dsiCmVcCfg->cm_out, dsiCmVcCfg->cm_in);
			res = CSL_LCD_COL_MODE;
			goto exit_err;
			break;
		}
		break;

		/* 2x565 pixels per 32-bit word */
	case LCD_IF_CM_I_RGB565P:
		switch (dsiCmVcCfg->cm_out) {
		case LCD_IF_CM_O_RGB565:
		case LCD_IF_CM_O_RGB565_DSI_VM:
			cmVcH->bpp_dma = 2;
			cmVcH->bpp_wire = 2;
			cmVcH->wc_rshift = 1;
			if (!dsiH->pixTxporter)
				dsiH->axipvCfg->pix_fmt =
						AXIPV_PIXEL_FORMAT_16BPP_PACKED;
			if (!dsiH->dispEngine) {
				if (dsiH->pvCfg->cmd)
					dsiH->pvCfg->pix_fmt = DSI_CMD_16BPP;
				else
					dsiH->pvCfg->pix_fmt = DSI_VIDEO_16BPP;
			}
			if (dsiCmVcCfg->cm_out == LCD_IF_CM_O_RGB565)
				cmVcH->cm = dsiH->dispEngine ?
						DE1_CM_565 : DE0_CM_565P;
			else
				cmVcH->cm = dsiH->dispEngine ?
						DE1_CM_LE : DE0_CM_565P;
			break;
		default:
			LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI][%d] %s: "
				"ERR, Invalid OutCol Mode[%d] for "
				"InCol RGB565\n",
				dsiH->bus, __func__, dsiCmVcCfg->cm_out);
			res = CSL_LCD_COL_MODE;
			goto exit_err;
			break;
		}
		break;

	default:
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI][%d] %s: ERR, Invalid "
			"InCol Mode[%d]\n",
			dsiH->bus, __func__, dsiCmVcCfg->cm_in);
		res = CSL_LCD_COL_MODE;
		goto exit_err;
		break;
	}

	cmVcH->configured = TRUE;
	cmVcH->client = clientH;

	LCD_DBG(LCD_DBG_INIT_ID, "[CSL DSI][%d] %s: "
		"OK, VC[%d], TE[%s]\n",
		dsiH->bus, __func__, cmVcH->vc,
		cmVcH->usesTe ? "YES" : "NO");

	*dsiCmVcH = (CSL_LCD_HANDLE)cmVcH;
	if (!clientH->hasLock)
		OSSEMAPHORE_Release(dsiH->semaDsi);
	return res;

exit_err:
	*dsiCmVcH = NULL;
	if (!clientH->hasLock)
		OSSEMAPHORE_Release(dsiH->semaDsi);
	return res;
}

/*
 *
 * Function Name: CSL_DSI_CloseCmVc
 *
 * Description:   Close Command Mode VC handle
 *
 */
CSL_LCD_RES_T CSL_DSI_CloseCmVc(CSL_LCD_HANDLE vcH)
{
	CSL_LCD_RES_T res = CSL_LCD_OK;
	DSI_CM_HANDLE dsiChH = (DSI_CM_HANDLE) vcH;
	DSI_HANDLE dsiH = (DSI_HANDLE)dsiChH->client->lcdH;
	DSI_CLIENT clientH;

	clientH = (DSI_CLIENT) dsiChH->client;

	if (!clientH->hasLock)
		OSSEMAPHORE_Obtain(dsiH->semaDsi, TICKS_FOREVER);

	dsiChH->configured = FALSE;
	LCD_DBG(LCD_DBG_INIT_ID, "[CSL DSI][%d] %s: VC[%d] Closed\n",
		dsiH->bus, __func__, dsiChH->vc);

	if (!clientH->hasLock)
		OSSEMAPHORE_Release(dsiH->semaDsi);

	return res;
}


/*
 *
 * Function Name: CSL_DSI_Force_Stop
 *
 */
void CSL_DSI_Force_Stop(CSL_LCD_HANDLE vcH)
{
	DSI_HANDLE dsiH;
	DSI_CLIENT clientH;
	DSI_CM_HANDLE dsiChH;

	dsiChH = (DSI_CM_HANDLE) vcH;
	clientH = (DSI_CLIENT) dsiChH->client;
	dsiH = (DSI_HANDLE)clientH->lcdH;

	if (dsiH->pixTxporter) {
		/* stop DMA transfer */
		if (csl_dma_vc4lite_stop_transfer(0)
		    != DMA_VC4LITE_STATUS_SUCCESS) {
			LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI] %s: "
				"ERR DMA Stop Transfer\n ", __func__);
		}
		/* release DMA channel */
		if (csl_dma_vc4lite_release_channel(0)
		    != DMA_VC4LITE_STATUS_SUCCESS) {
			LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI] %s: "
				"ERR ERR DMA Release Channel\n ", __func__);
		}
	} else {
		/* Force stop AXIPV */
	}

	chal_dsi_tx_start(dsiH->chalH, TX_PKT_ENG_1, FALSE);
	chal_dsi_tx_start(dsiH->chalH, TX_PKT_ENG_2, FALSE);
	if (dsiH->dispEngine)
		chal_dsi_de1_enable(dsiH->chalH, FALSE);
/*
	else
		chal_dsi_de0_enable(dsiH->chalH, FALSE);
*/
	cslDsiClearAllFifos(dsiH);
}


/*
 *
 * Function Name: CSL_DSI_UpdateVmVc
 *
 * Description:   Video Mode
 *
 */
CSL_LCD_RES_T CSL_DSI_UpdateVmVc(CSL_LCD_HANDLE vcH,
		pCSL_LCD_UPD_REQ req)
{
	CSL_LCD_RES_T res = CSL_LCD_OK;
	DSI_HANDLE dsiH;
	DSI_CLIENT clientH;
	DSI_CM_HANDLE dsiChH;
	DSI_UPD_REQ_MSG_T updMsg;
	struct axipv_config_t *axipvCfg;
	struct pv_config_t *pvCfg;
	OSStatus_t osStat;

	dsiChH = (DSI_CM_HANDLE) vcH;
	clientH = (DSI_CLIENT) dsiChH->client;
	updMsg.clientH = clientH;
	dsiH = (DSI_HANDLE)clientH->lcdH;
	updMsg.updReq = *req;
	updMsg.dsiH = dsiH;
	updMsg.updReq.buffBpp = dsiChH->bpp_dma;
	axipvCfg = dsiH->axipvCfg;
	pvCfg = dsiH->pvCfg;

	if (!axipvCfg) {
		pr_err("axipvCfg is NULL\n");
		return CSL_LCD_BAD_STATE;
	}
	if (!pvCfg) {
		pr_err("pvCfg is NULL\n");
		return CSL_LCD_BAD_STATE;
	}
	if (dsiH->ulps) {
		pr_err("dsi is in ulps!!\n");
		return CSL_LCD_BAD_STATE;
	}
	if (!clientH->hasLock)
		OSSEMAPHORE_Obtain(dsiH->semaDsi, TICKS_FOREVER);
	if (!videoEnabled) {
		cslDsiClearAllFifos(dsiH);
		chal_dsi_clr_status(dsiH->chalH, 0xffffffff);
		chal_dsi_de0_set_cm(dsiH->chalH, dsiChH->cm);
		chal_dsi_de0_set_mode(dsiH->chalH, DE0_MODE_VID);
		/* Set pix clk divider to bits per pixel for non-burst mode */
		chal_dsi_de0_set_pix_clk_div(dsiH->chalH,
			(dsiChH->bpp_wire << 3) / dsiH->dlCount);
		res = cslDsiPixTxStart(&updMsg);
		if (res != CSL_LCD_OK) {
			LCD_DBG(LCD_DBG_ID, "[CSL DSI][%d] %s: "
			"ERR Failed To Start DMA!\n", dsiH->bus, __func__);
			goto done;
		}
		videoEnabled = 1;
	} else {
		axipvCfg->buff.async = (u32) updMsg.updReq.buff;
		axipv_post(axipvCfg);
	}

	osStat = OSQUEUE_Post(dsiH->updReqQ, (QMsg_t *)&updMsg, TICKS_NO_WAIT);
	if (osStat != OSSTATUS_SUCCESS) {
		if (osStat == OSSTATUS_TIMEOUT)
			res = CSL_LCD_OS_TOUT;
		else
			res = CSL_LCD_OS_ERR;
	}

done:
	if (!clientH->hasLock)
		OSSEMAPHORE_Release(dsiH->semaDsi);
	return res;
}


/*
 *
 * Function Name: CSL_DSI_Suspend
 *
 * Description:   Not applicable for Command mode
 *
 */
CSL_LCD_RES_T CSL_DSI_Suspend(CSL_LCD_HANDLE vcH)
{
	DSI_HANDLE dsiH;
	DSI_CLIENT clientH;
	DSI_CM_HANDLE dsiChH;
	struct axipv_config_t *axipvCfg;
	struct pv_config_t *pvCfg;
	OSStatus_t osStat;

	dsiChH = (DSI_CM_HANDLE) vcH;
	clientH = (DSI_CLIENT) dsiChH->client;
	dsiH = (DSI_HANDLE)clientH->lcdH;
	axipvCfg = dsiH->axipvCfg;
	pvCfg = dsiH->pvCfg;

	if (dsiH->pixTxporter)
		return CSL_LCD_OK; /* MMDMA => Cmd mode. No action required */
	if (!axipvCfg) {
		pr_err("axipvCfg is NULL\n");
		return CSL_LCD_BAD_HANDLE;
	}
	if (axipvCfg->cmd)
		return CSL_LCD_OK; /* AXIPV cmd mode => No action required */
	if (dsiH->dispEngine || !pvCfg) {
		pr_err("pvCfg is NULL\n");
		return CSL_LCD_BAD_HANDLE;
	}
	axipv_change_state(AXIPV_STOP_EOF, axipvCfg);
	osStat = OSSEMAPHORE_Obtain(dsiH->semaAxipv, msecs_to_jiffies(100));
	if (osStat == OSSTATUS_TIMEOUT) {
		int tx_done = axipv_check_completion(AXIPV_STOP_EOF, axipvCfg);
		if (tx_done < 0) {
			pr_err("couldn't stop AXIPV at EOF!\n");
			axipv_change_state(AXIPV_STOP_IMM, axipvCfg);
		} else {
			pr_err("csl_dsi: recovered %d\n", __LINE__);
			OSSEMAPHORE_Release(dsiH->semaDma);
		}
	}
	pv_change_state(PV_STOP_EOF_ASYNC, pvCfg);
	/*cslDsiEnaIntEvent(dsiH, (UInt32)CHAL_DSI_ISTAT_PHY_CLK_ULPS);*/
	/*wait for pv to stop*/
	osStat = OSSEMAPHORE_Obtain(dsiH->semaPV, msecs_to_jiffies(100));
	if (osStat == OSSTATUS_TIMEOUT) {
		int tx_done = check_pv_state(PV_STOP_EOF_ASYNC, pvCfg);
		if (tx_done < 0) {
			pr_err("couldn't stop PV at EOF!");
			pv_change_state(PV_STOP_IMM, pvCfg);
		} else {
			pr_err("csl_dsi: recovered %d\n", __LINE__);
		}
	}
	/*wait for DSI lanes to go to low power*/
	/*cslDsiWaitForInt(dsiH, TICKS_IN_MILLISECONDS(50));*/
	chal_dsi_de0_enable(dsiH->chalH, FALSE);
	videoEnabled = 0;
	return CSL_LCD_OK;
}

/*
 *
 * Function Name: CSL_DSI_UpdateCmVc
 *
 * Description:   Command Mode - DMA Frame Update
 *                RESTRICTIONs:  565 - XY pixel size == multiple of 2 pixels
 *                              x888 - XY pixel size == multiple of 4 pixels
 */
CSL_LCD_RES_T CSL_DSI_UpdateCmVc(CSL_LCD_HANDLE vcH,
		pCSL_LCD_UPD_REQ req, Boolean isTE)
{
	CSL_LCD_RES_T res = CSL_LCD_OK;
	DSI_HANDLE dsiH;
	DSI_CLIENT clientH;
	DSI_CM_HANDLE dsiChH;
	OSStatus_t osStat;
	DSI_UPD_REQ_MSG_T updMsgCm;

	CHAL_DSI_TX_CFG_t txPkt;

	UInt32 frame_size_p,	/* XY size                    [pixels] */
	spare_pix = 0,       /* When pixel count is (! x2 in 565 mode)
				or (! x4 in 888 mode) */
	 wire_size_b,		/* XY wire size               [bytes] */
	 txNo1_len,		/* TX no 1 - packet size      [bytes] */
	 txNo1_cfifo_len,	/* TX no 1 - from CMND  FIFO  [bytes] */
	 txNo1_pfifo_len,	/* TX no 1 - from PIXEL FIFO  [bytes] */
	 txNo2_repeat;		/* TX no 2 - packet repeat count */

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
	/* OVERRIDE REQ BPP - NOT SUPPORTED, TODO: REMOVE FROM API */
	updMsgCm.updReq.buffBpp = dsiChH->bpp_dma;

	frame_size_p = req->lineLenP * req->lineCount;
	/* linelength is odd => MMDMA 2D config fails */
	if (1 == dsiH->pixTxporter) {
		if ((dsiChH->bpp_dma == 2) && (req->lineLenP & 1)) {
			pr_info("ERR Pixel Buff Size!");
			return CSL_LCD_MSG_SIZE;
		}
		/* Include the MM-DMA requirement on payload size once the RDB
		 * is updated */
	} else {
		/* Todo: Verify that the update request meet minimum criteria
		 * for DSI, PV and AXIPV */
		 if ((0 != ((req->lineLenP * dsiChH->bpp_dma) & 7))
			|| (0 != (((req->xStrideB + req->lineLenP)
			* dsiChH->bpp_dma) & 7))
			|| (((req->lineLenP * req->lineCount)
			* dsiChH->bpp_dma) < AXIPV_MIN_BUFF_SIZE)
			) {
			pr_info("ERR Pixel Buff Size!");
			return CSL_LCD_MSG_SIZE;
		 }
	}

	wire_size_b = frame_size_p * dsiChH->bpp_wire;

	txNo2_repeat = wire_size_b / CM_PKT_SIZE_B;
	txNo1_len = wire_size_b % CM_PKT_SIZE_B;
	if ((txNo2_repeat != 0) && (txNo1_len == 0)) {
		/* XY_size = n * PACKET SIZE */
		txNo1_len = CM_PKT_SIZE_B;
		txNo2_repeat--;
	}

	txNo1_pfifo_len = txNo1_len;
	txNo1_cfifo_len = 0;

	if (txNo2_repeat > DSI_PKT_RPT_MAX) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI][%d] %s: "
			"ERR Packet Repeat Count!\n", dsiH->bus, __func__);
		return CSL_LCD_ERR;
	}

	if (!clientH->hasLock)
		OSSEMAPHORE_Obtain(dsiH->semaDsi, TICKS_FOREVER);

	if (dsiH->ulps) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI][%d] %s: "
			"ERR, Link Is In ULPS\n", dsiH->bus, __func__);
		if (!clientH->hasLock)
			OSSEMAPHORE_Release(dsiH->semaDsi);
		return CSL_LCD_BAD_STATE;
	}

	cslDsiClearAllFifos(dsiH);
	chal_dsi_clr_status(dsiH->chalH, 0xFFFFFFFF);

	/* set TE mode -- SHOULD BE PART OF INIT or OPEN */
	chal_dsi_te_mode(dsiH->chalH, dsiChH->teMode);

	txPkt.dispEngine = dsiH->dispEngine;
	if (1 == dsiH->dispEngine) {
		/* Set DE1 Mode, Enable */
		chal_dsi_de1_set_cm(dsiH->chalH, dsiChH->cm);
		if (dsiH->pixTxporter)
			chal_dsi_de1_set_dma_thresh(dsiH->chalH, (frame_size_p
			>> 2) > DE1_DEF_THRESHOLD_W ? DE1_DEF_THRESHOLD_W :
			(frame_size_p >> 2));
		else
			chal_dsi_de1_set_dma_thresh(dsiH->chalH, 8);
		chal_dsi_de1_enable(dsiH->chalH, TRUE);
	} else {
		/* Set DE0 Mode, Enable */
		chal_dsi_de0_set_cm(dsiH->chalH, dsiChH->cm);
		chal_dsi_de0_set_mode(dsiH->chalH, DE0_MODE_CMD);
		chal_dsi_de0_enable(dsiH->chalH, TRUE);
	}

	/* BOF TX PKT ENG(s) Set-Up */
	/* TX No1 - first packet */
	chal_dsi_wr_cfifo(dsiH->chalH, &dsiChH->dcsCmndStart, 1);
	txPkt.msgLen = 1 + txNo1_len;

	if (1 == dsiH->pixTxporter)
		spare_pix = frame_size_p & (dsiChH->bpp_dma - 1);
	else
		spare_pix = 0; /* AXIPV FIFO width is 64bits*/

	if (spare_pix) {
			int offset;
			/*
			1. Transfer 1/2/3 to cmd data fifo
			2. Update the TXPKT1_H.BC_CMDFIFO with no. of bytes to
				be taken from cmd data fifo
			3. Do not update the window parameters as they will be
				needed to create DMA linked lists: 1D + 2D
			*/
			txNo1_cfifo_len += spare_pix*dsiChH->bpp_wire;
			offset = 0;
			switch (spare_pix) {
			case 3:
				chal_dsi_wr_cfifo(dsiH->chalH,
				(char *)phys_to_virt((uint32_t)req->buff),
				dsiChH->bpp_wire);
				if (req->lineLenP >= spare_pix)
					offset = dsiChH->bpp_dma;
				else if (req->lineLenP == (spare_pix - 1))
					offset = dsiChH->bpp_dma;
				else
					offset = (req->lineLenP +
					req->xStrideB) * dsiChH->bpp_dma;
			case 2:
				chal_dsi_wr_cfifo(dsiH->chalH,
				(char *)phys_to_virt((uint32_t)req->buff
				+ offset), dsiChH->bpp_wire);
				if (req->lineLenP >= spare_pix)
					offset += dsiChH->bpp_dma;
				else {
					offset += (req->lineLenP +
					req->xStrideB) * dsiChH->bpp_dma;
					if ((req->lineLenP == (spare_pix - 1)
						&& (spare_pix == 3))) {
						offset -= dsiChH->bpp_dma;
					}
				}
			case 1:
				chal_dsi_wr_cfifo(dsiH->chalH,
				(char *)phys_to_virt((uint32_t)req->buff +
				offset), dsiChH->bpp_wire);
				break;
			default:
				pr_info("ERROR: spare_pix =%d\n", spare_pix);
			}
	}
	txPkt.msgLenCFifo = 1 + txNo1_cfifo_len;
	txPkt.repeat = 1;
	txPkt.isTe = dsiChH->usesTe && isTE;

	if (txNo2_repeat == 0)
		chal_dsi_tx_long(dsiH->chalH, TX_PKT_ENG_1, &txPkt);
	else
		chal_dsi_tx_long(dsiH->chalH, TX_PKT_ENG_2, &txPkt);

	/* TX No2 - if any, rest of the frame */
	if (txNo2_repeat != 0) {
		chal_dsi_wr_cfifo(dsiH->chalH, &dsiChH->dcsCmndCont, 1);
		txPkt.repeat = txNo2_repeat;
		txPkt.msgLen = 1 + CM_PKT_SIZE_B;
		txPkt.msgLenCFifo = 1;
		txPkt.isTe = FALSE;

		chal_dsi_tx_long(dsiH->chalH, TX_PKT_ENG_1, &txPkt);
	}
	/* EOF TX PKT ENG(s) Set-Up */

	/*--- Wait for TX PKT ENG 1 DONE */
	cslDsiEnaIntEvent(dsiH, (UInt32)CHAL_DSI_ISTAT_TXPKT1_DONE);

	if (1 == dsiH->dispEngine) {
		/*--- Start TX PKT Engine(s) */
		if (txNo2_repeat != 0) {
			chal_dsi_tx_start(dsiH->chalH, TX_PKT_ENG_2, TRUE);
		}
		chal_dsi_tx_start(dsiH->chalH, TX_PKT_ENG_1, TRUE);
	}
	/* send rest of the frame */

	/*--- Start DMA */
	res = cslDsiPixTxStart(&updMsgCm);
	if (res != CSL_LCD_OK) {
		LCD_DBG(LCD_DBG_ID, "[CSL DSI][%d] %s: "
			"ERR Failed To Start DMA!\n", dsiH->bus, __func__);

		if (!clientH->hasLock)
			OSSEMAPHORE_Release(dsiH->semaDsi);
		return res;
	}
	if (0 == dsiH->dispEngine) {
		/*--- Start TX PKT Engine(s) */
		if (txNo2_repeat != 0) {
			chal_dsi_tx_start(dsiH->chalH, TX_PKT_ENG_2, TRUE);
		}
		chal_dsi_tx_start(dsiH->chalH, TX_PKT_ENG_1, TRUE);
	}
	if (req->cslLcdCb == NULL) {
		if (!clientH->hasLock) {
			osStat = OSSEMAPHORE_Obtain(dsiH->semaDma,
					    TICKS_IN_MILLISECONDS(req->
								  timeOut_ms));

			if (osStat != OSSTATUS_SUCCESS) {
				int tx_done;
				if (osStat == OSSTATUS_TIMEOUT) {
					LCD_DBG(LCD_DBG_ERR_ID,
					"[CSL DSI][%d] %s: "
					"ERR Timed Out Waiting For EOF DMA!\n",
					dsiH->bus, __func__);
					res = CSL_LCD_OS_TOUT;
				} else {
					LCD_DBG(LCD_DBG_ERR_ID,
					"[CSL DSI][%d] %s: "
					"ERR OS Err...\n",
					dsiH->bus, __func__);
					res = CSL_LCD_OS_ERR;
				}
				tx_done = cslDsiPixTxStop(&updMsgCm);
				if (tx_done == CSL_LCD_OK) {
					osStat = OSSTATUS_SUCCESS;
					res = CSL_LCD_OK;
				}
			}

			/*wait for interface to drain */
			if (res == CSL_LCD_OK)
				res = cslDsiWaitForInt(dsiH, req->timeOut_ms);
			else
				cslDsiWaitForInt(dsiH, 1);
		} else {
			LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI][%d] %s: "
				"In Atomic wait for DSI/DMA/AXIPV finish...\n",
				dsiH->bus, __func__);

			cslDsiPixTxPollInt(&updMsgCm);
			res = cslDsiWaitForStatAny_Poll(dsiH,
					CHAL_DSI_STAT_TXPKT1_DONE, NULL, 100);
			LCD_DBG(LCD_DBG_ERR_ID, "Frame data transfer done\n");
		}

		chal_dsi_tx_start(dsiH->chalH, TX_PKT_ENG_1, FALSE);
		chal_dsi_tx_start(dsiH->chalH, TX_PKT_ENG_2, FALSE);
		if (dsiH->dispEngine)
			chal_dsi_de1_enable(dsiH->chalH, FALSE);
		else
			chal_dsi_de0_enable(dsiH->chalH, FALSE);

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
	return res;
}

/*
 *
 * Function Name:  CSL_DSI_CloseClient
 *
 * Description:    Close Client Interface
 *
 */
CSL_LCD_RES_T CSL_DSI_CloseClient(CSL_LCD_HANDLE client)
{
	DSI_HANDLE dsiH;
	DSI_CLIENT clientH;
	CSL_LCD_RES_T res;

	clientH = (DSI_CLIENT) client;
	dsiH = (DSI_HANDLE)clientH->lcdH;

	if (clientH->hasLock) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI][%d] %s: ERR Client LOCK "
			"Active!\n", dsiH->bus, __func__);
		res = CSL_LCD_ERR;
	} else {
		OSSEMAPHORE_Obtain(dsiH->semaDsi, TICKS_FOREVER);

		clientH->open = FALSE;
		dsiH->clients--;
		res = CSL_LCD_OK;
		LCD_DBG(LCD_DBG_INIT_ID, "[CSL DSI][%d] %s: "
			"OK, Clients Left[%d]\n",
			dsiH->bus, __func__, dsiH->clients);

		OSSEMAPHORE_Release(dsiH->semaDsi);
	}
	return res;
}

/*
 *
 * Function Name: CSL_DSI_GetMaxTxMsgSize
 *
 * Description:   Return Maximum size of Command Packet we can send [BYTEs]
 *                (non pixel data)
 *
 */
UInt32 CSL_DSI_GetMaxTxMsgSize(void)
{
	return CHAL_DSI_CMND_FIFO_SIZE_B + CHAL_DSI_PIXEL_FIFO_SIZE_B;
}

/*
 *
 * Function Name: CSL_DSI_GetMaxRxMsgSize
 *
 * Description:   Return Maximum size of Command Packet we can receive [BYTEs]
 *
 */
UInt32 CSL_DSI_GetMaxRxMsgSize(void)
{
	return CHAL_DSI_RX_MSG_MAX;
}

/*
 *
 * Function Name:  CSL_DSI_OpenClient
 *
 * Description:    Register Client Of DSI interface
 *
 */
CSL_LCD_RES_T CSL_DSI_OpenClient(UInt32 bus, CSL_LCD_HANDLE *clientH)
{
	DSI_HANDLE dsiH;
	CSL_LCD_RES_T res;
	UInt32 i;

	if (bus >= DSI_INST_COUNT) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI] %s: ERR Invalid "
			"Bus Id[%d]!\n", __func__, bus);
		*clientH = (CSL_LCD_HANDLE)NULL;
		return CSL_LCD_BUS_ID;
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
				dsiH->client[i].hasLock = FALSE;
				*clientH = (CSL_LCD_HANDLE)&dsiH->client[i];
				break;
			}
		}
		if (i >= DSI_MAX_CLIENT) {
			LCD_DBG(LCD_DBG_ID, "[CSL DSI][%d] %s: "
				"ERR, Max Client Count Reached[%d]\n",
				dsiH->bus, __func__, DSI_MAX_CLIENT);
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
			dsiH->bus, __func__, dsiH->clients);

	OSSEMAPHORE_Release(dsiH->semaDsi);

	return res;
}

/*
 *
 * Function Name: CSL_DSI_Ulps
 *
 * Description:   Enter / Exit ULPS on Clk & Data Line(s)
 *
 */
CSL_LCD_RES_T CSL_DSI_Ulps(CSL_LCD_HANDLE client, Boolean on)
{
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

	return res;
}

/*
 *
 * Function Name:  CSL_DSI_Close
 *
 * Description:    Close DSI Controller
 *
 */
CSL_LCD_RES_T CSL_DSI_Close(UInt32 bus)
{
	DSI_HANDLE dsiH;
	CSL_LCD_RES_T res = CSL_LCD_OK;

	dsiH = (DSI_HANDLE)&dsiBus[bus];

	OSSEMAPHORE_Obtain(dsiH->semaDsi, TICKS_FOREVER);

	if (dsiH->init != DSI_INITIALIZED) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI][%d] %s: "
			"DSI Interface Not Init\n", bus, __func__);
		res = CSL_LCD_ERR;
		goto CSL_DSI_CloseRet;
	}

	if (dsiH->clients != 0) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI][%d] %s: "
			"DSI Interface Client Count[%d] != 0\n",
			bus, __func__, dsiH->clients);
		res = CSL_LCD_ERR;
		goto CSL_DSI_CloseRet;
	}

	chal_dsi_off(dsiH->chalH);
	chal_dsi_phy_afe_off(dsiH->chalH);
	dsiH->init = ~DSI_INITIALIZED;

	cslDsiAfeLdoSetState(dsiH, DSI_LDO_OFF);

	LCD_DBG(LCD_DBG_INIT_ID, "[CSL DSI][%d] %s: OK\n", bus, __func__);

CSL_DSI_CloseRet:
	OSSEMAPHORE_Release(dsiH->semaDsi);

	return res;
}

static void csl_dsi_set_chal_api_clks(DSI_HANDLE dsiH,
				      const pCSL_DSI_CFG dsiCfg) {

	dsiH->clkCfg.escClk_MHz = dsiCfg->escClk.clkIn_MHz
	    / dsiCfg->escClk.clkInDiv;
	dsiH->clkCfg.hsBitClk_MHz = dsiCfg->hsBitClk.clkIn_MHz
	    / dsiCfg->hsBitClk.clkInDiv;

#if 0 /* RDB doesn't recommend using DDR2 clock */
	if ((dsiH->clkCfg.hsBitClk_MHz * 1000000 / 2) <= DSI_CORE_CLK_MAX_MHZ) {
		dsiH->clkCfg.coreClkSel = CHAL_DSI_BIT_CLK_DIV_BY_2;
		LCD_DBG(LCD_DBG_ID, "[CSL DSI][%d] %s: "
			"DSI CORE CLK SET TO BIT_CLK/2\n", dsiH->bus, __func__);
	} else if ((dsiH->clkCfg.hsBitClk_MHz * 1000000 / 4) <=
			DSI_CORE_CLK_MAX_MHZ) {
		dsiH->clkCfg.coreClkSel = CHAL_DSI_BIT_CLK_DIV_BY_4;
		LCD_DBG(LCD_DBG_ID, "[CSL DSI][%d] %s: "
			"DSI CORE CLK SET TO BIT_CLK/4\n", dsiH->bus, __func__);
	} else {
		dsiH->clkCfg.coreClkSel = CHAL_DSI_BIT_CLK_DIV_BY_8;
		LCD_DBG(LCD_DBG_ID, "[CSL DSI][%d] %s: "
			"DSI CORE CLK SET TO BIT_CLK/8\n", dsiH->bus, __func__);
	}
#else
	if (dsiH->clkCfg.hsBitClk_MHz > 200) {
		dsiH->clkCfg.coreClkSel = CHAL_DSI_BIT_CLK_DIV_BY_8;
		LCD_DBG(LCD_DBG_ID, "%d DSI CORE CLK SET TO BIT_CLK/8\n",
			dsiH->bus);
	} else {
		dsiH->clkCfg.coreClkSel = CHAL_DSI_BIT_CLK_DIV_BY_2;
		LCD_DBG(LCD_DBG_ID, "%d DSI CORE CLK SET TO BIT_CLK/2\n",
			dsiH->bus);
	}
#endif
}

/*
 *
 * Function Name:  CSL_DSI_Init
 *
 * Description:    Init DSI Controller
 *
 */
CSL_LCD_RES_T CSL_DSI_Init(const pCSL_DSI_CFG dsiCfg)
{
	int ret;
	CSL_LCD_RES_T res = CSL_LCD_OK;
	DSI_HANDLE dsiH;

	CHAL_DSI_MODE_t chalMode;
	CHAL_DSI_INIT_t chalInit;
	CHAL_DSI_AFE_CFG_t chalAfeCfg;

	struct axipv_init_t axipv_init_data = {
		.irq = BCM_INT_ID_AXIPV,
		.base_addr = KONA_AXIPV_VA,
#ifdef AXIPV_HAS_CLK
		.clk_name = AXIPV_AXI_BUS_CLK_NAME_STR,
#endif
		.irq_cb = axipv_irq_cb,
		.release_cb = axipv_release_cb,
	};
	struct pv_init_t pv_init_data = {
		.id = 0,
		.irq = BCM_INT_ID_PV,
		.base_addr = KONA_PIXELVALVE_VA,
#ifdef PV_HAS_CLK
		.apb_clk_name = PIXELV_APB_BUS_CLK_NAME_STR,
		.pix_clk_name = PIXELV_PERI_CLK_NAME_STR,
#endif
		.err_cb = pv_err_cb,
		.eof_cb = pv_eof_cb,
	};


	if (dsiCfg->bus >= DSI_INST_COUNT) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI][%d] %s: "
			"ERR Invalid Bus Id!\n", dsiCfg->bus, __func__);
		return CSL_LCD_BUS_ID;
	}

	dsiH = (DSI_HANDLE)&dsiBus[dsiCfg->bus];

	if (dsiH->init == DSI_INITIALIZED) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI][%d] %s: "
			"DSI Interface Already Init\n",
			dsiCfg->bus, __func__);
		return CSL_LCD_IS_OPEN;
	}

	if (dsiH->initOnce != DSI_INITIALIZED) {

		memset(dsiH, 0, sizeof(DSI_HANDLE_t));

		dsiH->bus = dsiCfg->bus;

		if (dsiH->bus == 0) {
			dsiH->dsiCoreRegAddr = CSL_DSI0_BASE_ADDR;
			dsiH->interruptId = CSL_DSI0_IRQ;
			dsiH->lisr = cslDsi0Stat_LISR;
			dsiH->hisr = NULL;
			dsiH->task = cslDsi0UpdateTask;
			dsiH->dma_cb = cslDsi0EofDma;
		}

		if (!cslDsiOsInit(dsiH)) {
			LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI][%d] %s: ERROR OS "
				"Init!\n", dsiCfg->bus, __func__);
			return CSL_LCD_OS_ERR;
		} else {
			dsiH->initOnce = DSI_INITIALIZED;
		}
	}

	/* Let vsync events be signalled in ISR context by AXIPV */
	/* dsiH->vsync_cb = dsiCfg->vsync_cb; */
	dsiH->vsync_cb = NULL;
	axipv_init_data.vsync_cb = dsiCfg->vsync_cb,

	dsiH->dlCount = dsiCfg->dlCount;
	dsiH->dispEngine = dsiCfg->dispEngine;
	dsiH->pixTxporter = dsiCfg->pixTxporter;
	dsiH->vmode = dsiCfg->vmode;
	if (!dsiH->dispEngine && dsiH->pixTxporter) {
		pr_err("Error:MMDMA cannot feed DE0! Default to DE1\n");
		dsiH->dispEngine = 1;
	}

	if (0 == dsiH->pixTxporter) {
		if (0 == dsiH->dispEngine) {
			if (dsiH->initPV != DSI_INITIALIZED) {
				printk(KERN_INFO"Initialising PV\n");
				ret = pv_init(&pv_init_data, &dsiH->pvCfg);
				if ((ret < 0) || !dsiH->pvCfg) {
					pr_err("pv_init failed with ret = %d\n",
						ret);
					return CSL_LCD_ERR;
				}
				dsiH->initPV = DSI_INITIALIZED;
			}

			dsiH->pvCfg->pclk_sel = DISP_CTRL_DSI;
			if (dsiCfg->vmode) {
				dsiH->pvCfg->cmd = false;
				dsiH->pvCfg->cont = true;
			} else {
				dsiH->pvCfg->cmd = true;
				dsiH->pvCfg->cont = false;
			}
			dsiH->pvCfg->vs = dsiCfg->vs;
			dsiH->pvCfg->vbp = dsiCfg->vbp;
			dsiH->pvCfg->vfp = dsiCfg->vfp;
			dsiH->pvCfg->hs = dsiCfg->hs;
			dsiH->pvCfg->hbp = dsiCfg->hbp;
			dsiH->pvCfg->hfp = dsiCfg->hfp;
			dsiH->pvCfg->interlaced = false;
			dsiH->pvCfg->vsyncd = 0;
			dsiH->pvCfg->pix_stretch = 0;
		}

		if (dsiH->initAXIPV != DSI_INITIALIZED) {
			printk(KERN_INFO"Initialising AXIPV\n");
			ret = axipv_init(&axipv_init_data, &dsiH->axipvCfg);
			if ((ret < 0) || !dsiH->axipvCfg) {
				pr_err("axipv_init failed with ret=%d\n", ret);
				return CSL_LCD_ERR;
			}
			dsiH->initAXIPV = DSI_INITIALIZED;
		}
		dsiH->axipvCfg->test = false;
		if (dsiCfg->vmode) {
			dsiH->axipvCfg->cmd = false;
			dsiH->axipvCfg->async = true;
			dsiH->axipvCfg->bypassPV = 0;
		} else {
			dsiH->axipvCfg->async = false;
			dsiH->axipvCfg->cmd = true;
			dsiH->axipvCfg->bypassPV = 1;
		}
	}

	/* Init User Controlled Values */
	chalInit.dlCount = dsiCfg->dlCount;
	chalInit.clkContinuous = dsiCfg->enaContClock;

	chalMode.enaContClock = dsiCfg->enaContClock;	/* 2'nd time ? */
	chalMode.enaRxCrc = dsiCfg->enaRxCrc;
	chalMode.enaRxEcc = dsiCfg->enaRxEcc;
	chalMode.enaHsTxEotPkt = dsiCfg->enaHsTxEotPkt;
	chalMode.enaLpTxEotPkt = dsiCfg->enaLpTxEotPkt;
	chalMode.enaLpRxEotPkt = dsiCfg->enaLpRxEotPkt;

	/* Init HARD-CODED Settings */
	chalAfeCfg.afeCtaAdj = 7;	/* 0 - 15 */
	chalAfeCfg.afePtaAdj = 7;	/* 0 - 15 */
	chalAfeCfg.afeBandGapOn = TRUE;
	chalAfeCfg.afeDs2xClkEna = FALSE;

	chalAfeCfg.afeClkIdr = 6;	/* 0 - 7  DEF 6 */
	chalAfeCfg.afeDlIdr = 6;	/* 0 - 7  DEF 6 */

	csl_dsi_set_chal_api_clks(dsiH, dsiCfg);
	videoEnabled = g_display_enabled;
	if (!g_display_enabled)
		cslDsiAfeLdoSetState(dsiH, DSI_LDO_HP);

	dsiH->chalH = chal_dsi_init(dsiH->dsiCoreRegAddr, &chalInit);

	if (dsiH->chalH == NULL) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL DSI][%d] %s: ERROR in "
				"cHal Init!\n", dsiCfg->bus, __func__);
		res = CSL_LCD_ERR;
	} else {
		/* as per rdb clksel must be set before ANY timing
		   is set, 0=byte clock 1=bitclk2 2=bitclk */
		chalMode.clkSel = dsiH->clkCfg.coreClkSel;
		if (!g_display_enabled) {
			chal_dsi_phy_afe_on(dsiH->chalH, &chalAfeCfg);
			chal_dsi_on(dsiH->chalH, &chalMode);

			if (!chal_dsi_set_timing(dsiH->chalH,
					dsiCfg->phy_timing,
					dsiH->clkCfg.coreClkSel,
					dsiH->clkCfg.escClk_MHz,
					dsiH->clkCfg.hsBitClk_MHz,
					dsiCfg->lpBitRate_Mbps)) {
				LCD_DBG(LCD_DBG_ERR_ID,
						"[%d]%s: ERROR In Timing Calc\n"
						, dsiCfg->bus);
				res = CSL_LCD_ERR;
			} else {
				chal_dsi_de1_set_dma_thresh(dsiH->chalH,
						DE1_DEF_THRESHOLD_W);
				cslDsiClearAllFifos(dsiH);
				/* wait for STOP state */
				OSTASK_Sleep(TICKS_IN_MILLISECONDS(1));
			}
		}
	}

	if (res == CSL_LCD_OK) {
		LCD_DBG(LCD_DBG_INIT_ID, "[CSL DSI][%d] %s: OK\n",
			dsiCfg->bus, __func__);
		dsiH->init = DSI_INITIALIZED;
		dsiH->bus = dsiCfg->bus;
#define MM_SWITCH_QOS_ENABLE_REG HW_IO_PHYS_TO_VIRT(0x3C001444)
#define MM_QOS_EN_REG HW_IO_PHYS_TO_VIRT(0x3C004060)
#define ENABLE_AXIPV_QOS (1 << 7)
		writel(readl(MM_SWITCH_QOS_ENABLE_REG) | ENABLE_AXIPV_QOS,
			MM_SWITCH_QOS_ENABLE_REG);
		writel(readl(MM_QOS_EN_REG) | ENABLE_AXIPV_QOS, MM_QOS_EN_REG);
	} else {
		dsiH->init = 0;
	}

	return res;
}

int Log_DebugPrintf(UInt16 logID, char *fmt, ...)
{
	char p[255];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(p, 255, fmt, ap);
	va_end(ap);

	printk(p);

	return 1;
}

