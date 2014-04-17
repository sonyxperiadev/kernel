/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2011  Broadcom Corporation                                                        */
/*                                                                                              */
/*     Unless you and Broadcom execute a separate written software license agreement governing  */
/*     use of this software, this software is licensed to you under the terms of the GNU        */
/*     General Public License version 2 (the GPL), available at                                 */
/*                                                                                              */
/*          http://www.broadcom.com/licenses/GPLv2.php                                          */
/*                                                                                              */
/*     with the following added to such license:                                                */
/*                                                                                              */
/*     As a special exception, the copyright holders of this software give you permission to    */
/*     link this software with independent modules, and to copy and distribute the resulting    */
/*     executable under terms of your choice, provided that you also meet, for each linked      */
/*     independent module, the terms and conditions of the license of that module.              */
/*     An independent module is a module which is not derived from this software.  The special  */
/*     exception does not apply to any modifications of the software.                           */
/*                                                                                              */
/*     Notwithstanding the above, under no circumstances may you combine this software in any   */
/*     way with any other Broadcom software provided under a license other than the GPL,        */
/*     without Broadcom's express prior written consent.                                        */
/*                                                                                              */
/************************************************************************************************/

/**
*
*  @file   csl_smi.c
*
*  @brief  SMI/SPI(LoSSI) Combined CSL driver
*
*  @note   Platforms supported: HERA
*
****************************************************************************/
#ifndef __KERNEL__

//#if ( defined (_HERA_)  || defined(_RHEA_) || defined(_SAMOA_) )

// ENABLE TRACE (NO ERR) MESSAGEs
#undef  __SMI_SPI_ENABLE_TRACE_MSG__

#ifdef FPGA_VERSION
#undef __SMI_USE_PRM__
#undef __USE_PMUX_DRV__
#else
#define __SMI_USE_PRM__
#undef __USE_PMUX_DRV__
#endif

#include "mobcom_types.h"
#include "msconsts.h"

#include "ostypes.h"
#include "osheap.h"
#include "ostask.h"
#include "ossemaphore.h"
#include "osqueue.h"
#include "irqctrl.h"
#include "chip_irq.h"
#include "osinterrupt.h"
#include "sio.h"

#ifdef __USE_PMUX_DRV__
#include "pmux_drv.h"
#endif

#include "csl_dma_vc4lite.h"
#include "chal_common.h"
#include "chal_smi.h"
#include "chal_spi_vc4lite.h"
#include "csl_lcd.h"
#include "csl_smi.h"
#include "dbg.h"
#include "logapi.h"

#include "sysmap_types.h"
#include "chal_sysmap.h"

#if defined( __SMI_USE_PRM__ )
#include "pm_prm.h"
#endif

#else /* __KERNEL__ */

#include <linux/string.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>
#include <mach/irqs.h>
#include <mach/memory.h>
#include <linux/kernel.h>
#include <mach/io_map.h>

#include <linux/broadcom/mobcom_types.h>
#include <linux/broadcom/msconsts.h>
#include <plat/osabstract/ostypes.h>
#include <plat/osabstract/ostask.h>
#include <plat/osabstract/ossemaphore.h>
#include <plat/osabstract/osqueue.h>
#include <plat/osabstract/osinterrupt.h>

#include <plat/csl/csl_dma_vc4lite.h>
#include <plat/chal/chal_common.h>
#include <plat/chal/chal_smi.h>
#include <plat/chal/chal_spi_vc4lite.h>
#include <plat/csl/csl_lcd.h>
#include <plat/csl/csl_smi.h>

#endif /* __KERNEL__ */

#define IRQ_Clear(irq_id)	do {} while (0)
#define IRQ_Disable(irq_id) 	do {} while (0)	//disable_irq_nosync(irq_id)
#define IRQ_Enable(irq_id)	do {} while (0)	//enable_irq(irq_id)

#define  __SMI_SPI_ENABLE_TRACE_MSG__
#undef __SMI_USE_PRM__
#undef __USE_PMUX_DRV__
#undef FPGA_VERSION

//#############################################################################
// CSL SMI Specific
//#############################################################################

// Pad Muxes - TE inputs and LCD Reset - Not used here
//#define PAD_CTRL_DSI0TE HW_IO_PHYS_TO_VIRT*) (0x35004838)  //   DSI0TE  
//#define PAD_CTRL_LCDTE  HW_IO_PHYS_TO_VIRT*) (0x350048E4)  //   LCD_TE   
//#define PAD_CTRL_LCDRES HW_IO_PHYS_TO_VIRT*) (0x350048D8)  //   LCD_RES
//#define PAD_CTRL_GPIO28 HW_IO_PHYS_TO_VIRT*) (0x350048AC)  //   DSI1TE   

// Pad Muxes - SMI bus 
#define SMI_REG_RW(reg_addr)	((volatile UInt32 *)(reg_addr))
#define PAD_CTRL_GPIO18     SMI_REG_RW(HW_IO_PHYS_TO_VIRT(0x35004884))	//  LCDCS1 aka SmiCS
#define PAD_CTRL_GPIO19     SMI_REG_RW(HW_IO_PHYS_TO_VIRT(0x35004888))	//  LCDWE
#define PAD_CTRL_GPIO20     SMI_REG_RW(HW_IO_PHYS_TO_VIRT(0x3500488C))	//  LCDRE
#define PAD_CTRL_GPIO21     SMI_REG_RW(HW_IO_PHYS_TO_VIRT(0x35004890))	//  LCDD7
#define PAD_CTRL_GPIO22     SMI_REG_RW(HW_IO_PHYS_TO_VIRT(0x35004894))	//  LCDD6
#define PAD_CTRL_GPIO23     SMI_REG_RW(HW_IO_PHYS_TO_VIRT(0x35004898))	//  LCDD5
#define PAD_CTRL_GPIO24     SMI_REG_RW(HW_IO_PHYS_TO_VIRT(0x3500489C))	//  LCDD4
#define PAD_CTRL_GPIO25     SMI_REG_RW(HW_IO_PHYS_TO_VIRT(0x350048A0))	//  LCDD3
#define PAD_CTRL_GPIO26     SMI_REG_RW(HW_IO_PHYS_TO_VIRT(0x350048A4))	//  LCDD2
#define PAD_CTRL_GPIO27     SMI_REG_RW(HW_IO_PHYS_TO_VIRT(0x350048A8))	//  LCDD1

#define PAD_CTRL_GPIO00     SMI_REG_RW(HW_IO_PHYS_TO_VIRT(0x3500483C))	//  LCDD15
#define PAD_CTRL_GPIO01     SMI_REG_RW(HW_IO_PHYS_TO_VIRT(0x35004840))	//  LCDD14
#define PAD_CTRL_GPIO02     SMI_REG_RW(HW_IO_PHYS_TO_VIRT(0x35004844))	//  LCDD13
#define PAD_CTRL_GPIO03     SMI_REG_RW(HW_IO_PHYS_TO_VIRT(0x35004848))	//  LCDD12
#define PAD_CTRL_GPIO08     SMI_REG_RW(HW_IO_PHYS_TO_VIRT(0x3500485C))	//  LCDD11
#define PAD_CTRL_GPIO09     SMI_REG_RW(HW_IO_PHYS_TO_VIRT(0x35004860))	//  LCDD10
#define PAD_CTRL_GPIO10     SMI_REG_RW(HW_IO_PHYS_TO_VIRT(0x35004864))	//  LCDD9
#define PAD_CTRL_GPIO11     SMI_REG_RW(HW_IO_PHYS_TO_VIRT(0x35004868))	//  LCDD8

#define PAD_CTRL_CAMCS0     SMI_REG_RW(HW_IO_PHYS_TO_VIRT(0x35004810))	//  LCDD16 (GPIO43)
#define PAD_CTRL_CAMCS1     SMI_REG_RW(HW_IO_PHYS_TO_VIRT(0x35004814))	//  LCDD17 (GPIO44)

// Pad Muxes - SPI bus 
#define PAD_CTRL_LCDCS0     SMI_REG_RW(HW_IO_PHYS_TO_VIRT(0x350048D4))	//  LCDCS0 aka SpiCS
#define PAD_CTRL_LCDSCL     SMI_REG_RW(HW_IO_PHYS_TO_VIRT(0x350048DC))	//  LCD_CD or  SpiScl
#define PAD_CTRL_LCDSDA     SMI_REG_RW(HW_IO_PHYS_TO_VIRT(0x350048E0))	//  LCD_D0 or  SpiSda

// default STD Pad Control: P-UP MODE=3 ? ( same as reset value from RDB )
//                          MUX control bits [10..08]
#define PAD_CTRL_STD        0x00000023

// SMI CLK Registers
#define MM_CLK_WR_ACCESS    SMI_REG_RW(HW_IO_PHYS_TO_VIRT(0x3c000000))
#define SMI_AXI_CLK_ENA	    SMI_REG_RW(HW_IO_PHYS_TO_VIRT(0x3C000260))	// bit[0]
#define SMI_CLK_ENA	    SMI_REG_RW(HW_IO_PHYS_TO_VIRT(0x3C000264))	// bit[0]
#define SMI_DIV	            SMI_REG_RW(HW_IO_PHYS_TO_VIRT(0x3C000A18))	// bit[8..4] DEF=0x12(18)
#define SMI_DIV_MAX         16

#define SMI_MAX_HANDLES     1	// max 1 SMI handle
#define SPI_MAX_HANDLES     1	// max 1 SPI handle
#define SMI_SPI_MAX_HANDLES 2	// max 2 handles, 1 SPI & 1 SMI

//#define SMI_BASE_ADDR       KONA_SMI_VA // SMI Controller Base Address
#define MM_SPI_BASE_ADDR    0x3C003000	// SPI Controller Base Address

// CSL SMI Configuration
typedef struct {
	Boolean usesTE;
	Boolean usesMemWrTiming;
	Boolean isRegWrTiming;
	UInt32 smi_clk_ns;
	UInt32 buffBpp;
	UInt32 bank;
	UInt8 addr_c;
	UInt8 addr_d;
	CHAL_SMI_MODE_T smiMode;
	CHAL_SMI_BUS_WIDTH_T busWidth;
	CHAL_SMI_TIMING_T wrTiming;
	CHAL_SMI_TIMING_T wrTiming_m;
	CHAL_SMI_TIMING_T rdTiming;
} SMI_CFG_T, *pSMI_CFG;

// CSL SPI Configuration
typedef struct {
	UInt32 csBank;		// CS bank
	UInt32 clkPol;		// 0 - resting state of clock LO
	// 1 - resting state of clock HI
	UInt32 clkPhase;	// 0= 1'st CLK transitions in middle od data bit
	// 1= 1'st CLK transitions in beg    od data bit
	UInt32 csPol;		// 0=active LO  1=active HI
} SPI_CFG_t, *pSPI_CFG;

// SMI/SPI Contrller Types
#define     CTRL_SMI        1
#define     CTRL_SPI        2

// SMI/SPI config
typedef union {
	SMI_CFG_T smiCfg;
	SPI_CFG_t spiCfg;
} CSL_SMI_SPI_CFG_t, *pCSL_SMI_SPI_CFG;

// SMI/SPI handle
typedef struct {
	Boolean open;
	Boolean hasLock;
#if defined( __SMI_USE_PRM__ )
	CLIENT_ID prm_ID;
#endif
	UInt32 ctrlType;
	CSL_SMI_SPI_CFG_t cfg;
	void *chalH;
} SMI_SPI_HANDLE_t, *pSMI_SPI_HANDLE;

// SMI/SPI CSL Driver
typedef struct {
	Boolean init;
	UInt32 instance;
	pSMI_SPI_HANDLE activeHandle;
	SMI_SPI_HANDLE_t ctrl[SMI_SPI_MAX_HANDLES];
} SMI_SPI_DRV_t;

// ASYNC request message
typedef struct {
	pSMI_SPI_HANDLE smiH;	// SMI/SPI handle
	DMA_VC4LITE_CHANNEL_t dmaCh;	// DMA channel used
	UInt32 timeOutMsec;	// timeout
	CSL_LCD_CB_T cslLcdCb;	// callback
	CSL_LCD_CB_REC_T cslLcdCbRec;
} SMI_UPD_REQ_MSG_T;

// MM DMA request
typedef struct {
	UInt32 xLenBytes;
	UInt32 xStrideBytes;
	UInt32 yLen;
	Boolean isWrite;
	UInt32 *pBuff;
} SMI_DMA_REQ_t, *pSMI_DMA_REQ;

#define     FLUSH_Q_SIZE                2
// \public\msconsts.h
#define     TASKNAME_SMI                (TName_t)"SmiT"
#define     TASKPRI_SMI                 (TPriority_t)(ABOVE_NORMAL)

#define     STACKSIZE_SMI               STACKSIZE_BASIC*5

#define     HISRNAME_SMISTAT            (IName_t)"SmiHisr"
#define     HISRSTACKSIZE_SMISTAT       (256 + RESERVED_STACK_FOR_LISR)

static Queue_t updReqQ;
static Semaphore_t eofDmaSema;
static Semaphore_t smiSema;
static Semaphore_t smiSemaInt;
#ifndef __KERNEL__
static Interrupt_t sSmiHisr = NULL;
#endif

static SMI_SPI_DRV_t cslSmiSpiDrv;

#define     INLINE __inline

//*****************************************************************************
// Local Functions
//*****************************************************************************
static void cslSmiSpiSwitchHandle(pSMI_SPI_HANDLE pSmi);
static CSL_LCD_RES_T cslSmiSpiDmaStart(pSMI_SPI_HANDLE smiSpiH,
				       pSMI_DMA_REQ dmaReq,
				       DMA_VC4LITE_CHANNEL_t *pDmaCh);

static CSL_LCD_RES_T cslSmiSpiDmaStop(DMA_VC4LITE_CHANNEL_t dmaCh);
INLINE static void cslSmiWaitProgDone(pSMI_SPI_HANDLE pSmi);
INLINE static void cslSmiWaitDirDone(pSMI_SPI_HANDLE pSmi);
INLINE static void cslSmiWaitTxFifoHasSpace(pSMI_SPI_HANDLE pSmi);
INLINE static void cslSmiWaitRxFifoHasData(pSMI_SPI_HANDLE pSmi);
static CSL_LCD_RES_T cslSmiBankConfig(pSMI_SPI_HANDLE pSmi);

#ifndef __KERNEL__
static void cslSmi_LISR(void);
#else
static irqreturn_t cslSmi_LISR(int irq, void *dev_id);
#endif

#ifndef __KERNEL__
static void cslSmi_HISR(void);
#endif

static void cslSmiEnablePads(void);
static int cslSmiSetSmiClk(pSMI_SPI_HANDLE pSmi, pCSL_SMI_CLK pSmiClkCfg);
extern void cslSmiSetClkEna(Boolean ena);

static void cslSmiEnaIntEvent(pSMI_SPI_HANDLE pSmi, UInt32 intEventMask);
static CSL_LCD_RES_T cslSmiWaitForInt(pSMI_SPI_HANDLE pSmi, UInt32 tout_msec);
static void cslSmiDisInt(pSMI_SPI_HANDLE pSmi);

//#############################################################################

#define CSL_SPI_POLL4DONE(chalH)                                         \
{                                                                        \
    while ( (chal_spivc4l_get_cs(chalH)& CHAL_SPIVC4L_CS_DONE) == 0 ) {} \
}
#define CSL_SPI_POLL4TXFIFO(chalH)                                       \
{                                                                        \
    while ( (chal_spivc4l_get_cs(chalH)& CHAL_SPIVC4L_CS_TXD ) == 0 ) {} \
}
#define CSL_SPI_POLL4RXFIFO(chalH)                                       \
{                                                                        \
    while ( (chal_spivc4l_get_cs(chalH)& CHAL_SPIVC4L_CS_RXD ) == 0 ) {} \
}

//#############################################################################

//*****************************************************************************
//
// Function Name:   cslSmiSetClkEna 
//
// Description:     Enable SMI ( output timing )& SMI AXI clks ( core clk )
//
//*****************************************************************************
void cslSmiSetClkEna(Boolean ena)
{
#ifndef FPGA_VERSION
#ifndef __KERNEL__
	// enable wr acess to MM_CLK CCU registers
	*MM_CLK_WR_ACCESS = 0x00A5A501;

#define CLK_HYST_ENA     0x000000100	// pup = set
#define CLK_HYST_DEL_HI  0x000000200	// pup = set
#define CLK_GATE_SW      0x000000002	// pup = set  0=HW
#define CLK_SW_ENA       0x000000001	// pup = 0    1=HW

	if (ena) {
		*SMI_AXI_CLK_ENA = CLK_GATE_SW | CLK_SW_ENA;
		*SMI_CLK_ENA = CLK_GATE_SW | CLK_SW_ENA;
	} else {
		*SMI_AXI_CLK_ENA = CLK_GATE_SW;
		*SMI_CLK_ENA = CLK_GATE_SW;
	}
#endif
#endif //#ifndef FPGA_VERSION
}

//*****************************************************************************
//
// Function Name:   cslSmiSetSmiClk
//
// Description:     Select SMI clk PLL & divider value 
//
//*****************************************************************************
static int cslSmiSetSmiClk(pSMI_SPI_HANDLE pSmi, pCSL_SMI_CLK pSmiClkCfg)
{
#if defined( __SMI_USE_PRM__ )
	// hardcode until added to DRV API 
	UInt32 smiFreqReq_Hz = 250000000;
	UInt32 smiFreqSet_Hz;
	CLOCK_STATE prmClkState;	// CLOCK_ON CLOCK_OFF
	PRM_RETURN prmRes;

	pSmi->prm_ID = PRM_client_register("CSL_SMI");
	if (pSmi->prm_ID < 0) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL SMI] %s: Failed To Register PRM "
			"Client\n\r", __FUNCTION__);
		return (-1);
	}
	prmRes = PRM_set_clock_speed(pSmi->prm_ID, RESOURCE_SMI, smiFreqReq_Hz);
	if (prmRes != PRM_OK) {
		LCD_DBG(LCD_DBG_ERR_ID,
			"[CSL SMI] %s: Failed To Set SMI Clock\n\r",
			__FUNCTION__);
		return (-1);
	}

	prmRes = PRM_set_clock_state(pSmi->prm_ID, RESOURCE_SMI, CLOCK_ON);
	if (prmRes != PRM_OK) {
		LCD_DBG(LCD_DBG_ERR_ID,
			"[CSL SMI] %s: Failed To Start SMI Clock\n\r",
			__FUNCTION__);
		return (-1);
	}

	prmClkState = PRM_get_power_clock_state(pSmi->prm_ID, RESOURCE_SMI,
						RESOURCE_CLOCK_STATE);
	if (prmClkState != CLOCK_ON) {
		LCD_DBG(LCD_DBG_ERR_ID,
			"[CSL SMI] %s: SMI CLock State Mismatch\n\r",
			__FUNCTION__);
		return (-1);
	}

	smiFreqSet_Hz = PRM_get_power_clock_state(pSmi->prm_ID, RESOURCE_SMI,
						  RESOURCE_CLOCK_SPEED);
	if (smiFreqReq_Hz != smiFreqSet_Hz) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL SMI] %s: WARNING SMI_CLK "
			"ReqHz != Set_Hz\n\r", __FUNCTION__);
	}
	//pSmi->cfg.smiCfg.smi_clk_ns = 1000000000/smiFreqReq_Hz;
	pSmi->cfg.smiCfg.smi_clk_ns = 1000000000 / smiFreqSet_Hz;

	LCD_DBG(LCD_DBG_INIT_ID, "[CSL SMI] %s: "
		"INFO SMI Clk: REQ_Hz[%d] SET_Hz[%d] SET_ns[%u]\n\r",
		__FUNCTION__, smiFreqReq_Hz, smiFreqSet_Hz,
		pSmi->cfg.smiCfg.smi_clk_ns);

	return (0);
#else

	UInt32 smiDivReg = 0;
	UInt32 smiPllMHz;

	if (pSmiClkCfg->smiClkDiv > SMI_DIV_MAX) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL SMI] cslSmiSetSmiClk: "
			"ERR SmiClkDiv Overflow [%d], Max[%d]\n\r",
			pSmiClkCfg->smiClkDiv, SMI_DIV_MAX);
		return (-1);
	}
	// it seems that fraction does not work    
	smiDivReg = (pSmiClkCfg->smiClkDiv - 1) << 5;

	if (pSmiClkCfg->pllSel == SMI_PLL_312MHz) {
		smiDivReg |= SMI_PLL_312MHz;
		smiPllMHz = 312;
	} else if (pSmiClkCfg->pllSel == SMI_PLL_500MHz) {
		smiDivReg |= SMI_PLL_500MHz;
		smiPllMHz = 500;
	} else {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL SMI] cslSmiSetSmiClk: "
			"ERR Invlaid SMI ClkIn Selection [%d]!\n\r",
			pSmiClkCfg->pllSel);
		return (-1);
	}

	pSmi->cfg.smiCfg.smi_clk_ns =
	    1000 / (smiPllMHz / pSmiClkCfg->smiClkDiv);

	LCD_DBG(LCD_DBG_INIT_ID, "[CSL SMI] cslSmiSetSmiClk: "
		"INFO SMI Clk: InMhz[%d] DivBy[%d] => Period %u[ns]!\n\r",
		smiPllMHz, pSmiClkCfg->smiClkDiv, pSmi->cfg.smiCfg.smi_clk_ns);

#ifndef FPGA_VERSION
#ifndef __KERNEL__
	// enable wr acess to MM_CLK CCU registers
	*MM_CLK_WR_ACCESS = 0x00A5A501;

	*SMI_CLK_ENA = 2;	// stop  SMI clock      
	*SMI_DIV = smiDivReg;
	*SMI_CLK_ENA = 3;	// start SMI clock      
#endif
#endif //#ifndef FPGA_VERSION
	return (0);
#endif

}

//*****************************************************************************
//
// Function Name: cslSmiEnablePads
//
// Description:   Select All SMI Interface Pins On Pad Muxes
//
//*****************************************************************************
static void cslSmiEnablePads(void)
{
#ifndef __KERNEL__
#ifndef FPGA_VERSION
#ifdef __USE_PMUX_DRV__
	PinMuxConfig_t pmuxCfg;

	// (0)GPIO[18..27]
	// (1)LCDCS1,LCDWE,LCDRE,LCDD[7..1] 
	pmuxCfg.DWord = PAD_CTRL_STD;
	pmuxCfg.PinMuxConfigBitField.mux = 1;
	PMUXDRV_Config_TVOUT(pmuxCfg);

	// (0)LCDCS0, LCDSCL , LCDSDA, LCDRES, LCDTE
	// (1)        LCDCD  , LCDD0 , LCDRES, LCDTE
	// (3)SSP2CK, SSP2SYN, SSP2DO, SSP2DI,    
	// (4)GPIO38, GPIO39 , GPIO40, GPIO41, GPIO42
	// (5)      , HA0    , DATA0 , RUN   , HAT0    DEF
	pmuxCfg.DWord = PAD_CTRL_STD;
	pmuxCfg.PinMuxConfigBitField.mux = 1;
	PMUXDRV_Config_DBI(pmuxCfg);
#else
	// SMI
	*PAD_CTRL_GPIO18 = PAD_CTRL_STD | (1 << 8);	// (0)GPIO18   (1)LCD_CS1
	*PAD_CTRL_GPIO19 = PAD_CTRL_STD | (1 << 8);	// (0)GPIO19   (1)LCD_WE
	*PAD_CTRL_GPIO20 = PAD_CTRL_STD | (1 << 8);	// (0)GPIO20   (1)LCD_RE
	*PAD_CTRL_GPIO21 = PAD_CTRL_STD | (1 << 8);	// (0)GPIO21   (1)LCD_D7
	*PAD_CTRL_GPIO22 = PAD_CTRL_STD | (1 << 8);	// (0)GPIO22   (1)LCD_D6
	*PAD_CTRL_GPIO23 = PAD_CTRL_STD | (1 << 8);	// (0)GPIO23   (1)LCD_D5
	*PAD_CTRL_GPIO24 = PAD_CTRL_STD | (1 << 8);	// (0)GPIO24   (1)LCD_D4
	*PAD_CTRL_GPIO25 = PAD_CTRL_STD | (1 << 8);	// (0)GPIO25   (1)LCD_D3
	*PAD_CTRL_GPIO26 = PAD_CTRL_STD | (1 << 8);	// (0)GPIO26   (1)LCD_D2
	*PAD_CTRL_GPIO27 = PAD_CTRL_STD | (1 << 8);	// (0)GPIO27   (1)LCD_D1

#if defined (_RHEA_)
	*PAD_CTRL_CAMCS1 = PAD_CTRL_STD | (2 << 8);	//  LCDD17
	*PAD_CTRL_CAMCS0 = PAD_CTRL_STD | (2 << 8);	//  LCDD16

	*PAD_CTRL_GPIO00 = PAD_CTRL_STD | (2 << 8);	//  LCDD15
	*PAD_CTRL_GPIO01 = PAD_CTRL_STD | (2 << 8);	//  LCDD14
	*PAD_CTRL_GPIO02 = PAD_CTRL_STD | (2 << 8);	//  LCDD13
	*PAD_CTRL_GPIO03 = PAD_CTRL_STD | (2 << 8);	//  LCDD12
	*PAD_CTRL_GPIO08 = PAD_CTRL_STD | (2 << 8);	//  LCDD11
	*PAD_CTRL_GPIO09 = PAD_CTRL_STD | (2 << 8);	//  LCDD10
	*PAD_CTRL_GPIO10 = PAD_CTRL_STD | (2 << 8);	//  LCDD9 
	*PAD_CTRL_GPIO11 = PAD_CTRL_STD | (2 << 8);	//  LCDD8
#endif

	// shared SMI/SPI
	*PAD_CTRL_LCDSCL = PAD_CTRL_STD | (1 << 8);	// (0)LCD_SCL  (1)LCD_CD  
	*PAD_CTRL_LCDSDA = PAD_CTRL_STD | (1 << 8);	// (0)LCD_SDA  (1)LCD_D0  
#endif // #ifdef __USE_PMUX_DRV__
#endif //#ifndef FPGA_VERSION
#endif
}

//*****************************************************************************
//
// Function Name: cslSpiEnablePads
//
// Description:   Select All SPI Interface Pins On Pad Muxes
//
//*****************************************************************************
static void cslSpiEnablePads(void)
{
#ifndef FPGA_VERSION
#ifdef __USE_PMUX_DRV__
	PinMuxConfig_t pmuxCfg;

	// (0)LCDCS0, LCDSCL , LCDSDA, LCDRES, LCDTE
	// (1)        LCDCD  , LCDD0 , LCDRES, LCDTE
	// (3)SSP2CK, SSP2SYN, SSP2DO, SSP2DI,    
	// (4)GPIO38, GPIO39 , GPIO40, GPIO41, GPIO42
	// (5)      , HA0    , DATA0 , RUN   , HAT0    DEF
	pmuxCfg.DWord = PAD_CTRL_STD;
	pmuxCfg.PinMuxConfigBitField.mux = 0;
	PMUXDRV_Config_DBI(pmuxCfg);
#else
	*PAD_CTRL_LCDCS0 = PAD_CTRL_STD | (0 << 8);	// (0)LCD_CS0  (4)GPIO38
	*PAD_CTRL_LCDSCL = PAD_CTRL_STD | (0 << 8);	// (0)LCD_SCL  (1)LCD_CD 
	*PAD_CTRL_LCDSDA = PAD_CTRL_STD | (0 << 8);	// (0)LCD_SDA  (1)LCD_D0 
#endif //#ifdef __USE_PMUX_DRV__
#endif //#ifndef FPGA_VERSION
}

//*****************************************************************************
//
// Function Name: cslSmiMuxOn
//
// Description:   select SMI Interface Pins (shared with SPI) On Pad Muxes
//
//*****************************************************************************
static void cslSmiMuxOn(void)
{
#ifndef __KERNEL__
#ifndef FPGA_VERSION
#ifdef __USE_PMUX_DRV__
	PinMuxConfig_t pmuxCfg;

	// (0)LCDCS0, LCDSCL , LCDSDA, LCDRES, LCDTE
	// (1)        LCDCD  , LCDD0 , LCDRES, LCDTE
	// (3)SSP2CK, SSP2SYN, SSP2DO, SSP2DI,    
	// (4)GPIO38, GPIO39 , GPIO40, GPIO41, GPIO42
	// (5)      , HA0    , DATA0 , RUN   , HAT0    DEF
	pmuxCfg.DWord = PAD_CTRL_STD;
	pmuxCfg.PinMuxConfigBitField.mux = 1;
	PMUXDRV_Config_DBI(pmuxCfg);
#else
	// shared SMI/SPI
	*PAD_CTRL_LCDSCL = PAD_CTRL_STD | (1 << 8);	// 000=LCD_SCL or  001=LCD_CD  
	*PAD_CTRL_LCDSDA = PAD_CTRL_STD | (1 << 8);	// 000=LCD_SDA or  001=LCD_D0  
#endif //#ifdef __USE_PMUX_DRV__
#endif //#ifndef FPGA_VERSION
#endif
}

//*****************************************************************************
//
// Function Name: cslSpiMuxOn
//
// Description:   select SPI Interface Pins (shared with SMI) On Pad Muxes
//
//*****************************************************************************
static void cslSpiMuxOn(void)
{
#ifndef FPGA_VERSION
#ifdef __USE_PMUX_DRV__
	PinMuxConfig_t pmuxCfg;

	// (0)LCDCS0, LCDSCL , LCDSDA, LCDRES, LCDTE
	// (1)        LCDCD  , LCDD0 , LCDRES, LCDTE
	// (3)SSP2CK, SSP2SYN, SSP2DO, SSP2DI,    
	// (4)GPIO38, GPIO39 , GPIO40, GPIO41, GPIO42
	// (5)      , HA0    , DATA0 , RUN   , HAT0    DEF
	pmuxCfg.DWord = PAD_CTRL_STD;
	pmuxCfg.PinMuxConfigBitField.mux = 0;
	PMUXDRV_Config_DBI(pmuxCfg);
#else
	*PAD_CTRL_LCDSCL = PAD_CTRL_STD | (0 << 8);	// 000=LCD_SCL or  001=LCD_CD
	*PAD_CTRL_LCDSDA = PAD_CTRL_STD | (0 << 8);	// 000=LCD_SDA or  001=LCD_D0  
#endif //#ifdef __USE_PMUX_DRV__
#endif //#ifndef FPGA_VERSION
}

//*****************************************************************************
//
// Function Name: cslSmiEofDma
//
// Description:   
//
//*****************************************************************************
static void cslSmiEofDma(DMA_VC4LITE_CALLBACK_STATUS status)
{
#if defined(__SMI_SPI_ENABLE_TRACE_MSG__ )
	LCD_DBG(LCD_DBG_ID, "[CSL SMI] %s\n\r", __FUNCTION__);
#endif

	if (status != DMA_VC4LITE_CALLBACK_SUCCESS) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL SMI] %s: ERR Ret By DmaDrv\n\r",
			__FUNCTION__);
	}

	OSSEMAPHORE_Release(eofDmaSema);
}

//*****************************************************************************
//
// Function Name: cslSmiUpdateTask
//
// Description:   
//
//*****************************************************************************
static void cslSmiUpdateTask(void)
{
	SMI_UPD_REQ_MSG_T updMsg;
	OSStatus_t osStat;
	CSL_LCD_RES_T res;

	LCD_DBG(LCD_DBG_INIT_ID, "[CSL SMI/SPI] cslSmiUpdateTask(): "
		"Started ...\n\r");

	for (;;) {
		res = CSL_LCD_OK;

		OSQUEUE_Pend(updReqQ, (QMsg_t *)&updMsg, TICKS_FOREVER);

		osStat = OSSEMAPHORE_Obtain(eofDmaSema,
					    TICKS_IN_MILLISECONDS(updMsg.
								  timeOutMsec));

		if (osStat != OSSTATUS_SUCCESS) {
			if (osStat == OSSTATUS_TIMEOUT)
				res = CSL_LCD_OS_TOUT;
			else
				res = CSL_LCD_OS_ERR;

			cslSmiSpiDmaStop(updMsg.dmaCh);

			LCD_DBG(LCD_DBG_ERR_ID,
				"[CSL SMI/SPI] %s: DMA end TimeOut\n\r",
				__FUNCTION__);
		}

		if (updMsg.smiH->ctrlType == CTRL_SPI) {
			// LOSSI-DONE-NOT-SET workarround 
			CHAL_SPIVC4L_DIS_DMA(updMsg.smiH->chalH);
			CSL_SPI_POLL4DONE(updMsg.smiH->chalH);
			chal_spivc4l_set_cs(updMsg.smiH->chalH, 0);
		} else {
			// wait for end of PROG MODE, then dis PROG mode
			if (res == CSL_LCD_OK) {
				res =
				    cslSmiWaitForInt(updMsg.smiH,
						     updMsg.timeOutMsec);
			} else {
				cslSmiDisInt(updMsg.smiH);
				LCD_DBG(LCD_DBG_ERR_ID,
					"[CSL SMI/SPI] %s: DONE INT "
					"TimeOut\r\n", __FUNCTION__);
			}
		}

		if (!updMsg.smiH->hasLock)
			OSSEMAPHORE_Release(smiSema);

		if (updMsg.cslLcdCb) {
			updMsg.cslLcdCb(res, &updMsg.cslLcdCbRec);
		}
	}
}				// cslSmiUpdateTask

//*****************************************************************************
//
// Function Name:  cslSmiEnaIntEvent
// 
// Description:    event bits set to "1" will be enabled, 
//                 rest of the int events will be disabled
//                 
//*****************************************************************************
static void cslSmiEnaIntEvent(pSMI_SPI_HANDLE pSmi, UInt32 intEventMask)
{
	chal_smi_setup_int(pSmi->chalH, CHAL_SMI_CS_INT_MASK, intEventMask);
	IRQ_Enable(BCM_INT_ID_SMI);
}

//*****************************************************************************
//
// Function Name:  cslSmiDisInt
// 
// Description:    Disable SMI Interrupts
//                 
//*****************************************************************************
static void cslSmiDisInt(pSMI_SPI_HANDLE pSmi)
{
	IRQ_Disable(BCM_INT_ID_SMI);

	// clear FIFOs & DONE flag, disable Prog Mode (DMA, all INTs)
	chal_smi_fifo_done(pSmi->chalH);

	IRQ_Clear(MM_SMI_IRQ);
}

//*****************************************************************************
//
// Function Name:  cslSmiWaitForInt
// 
// Description:    
//                 
//*****************************************************************************
static CSL_LCD_RES_T cslSmiWaitForInt(pSMI_SPI_HANDLE pSmi, UInt32 tout_msec)
{
	OSStatus_t osRes;
	CSL_LCD_RES_T res = CSL_LCD_OK;

	osRes =
	    OSSEMAPHORE_Obtain(smiSemaInt, TICKS_IN_MILLISECONDS(tout_msec));

#ifndef __KERNEL__
	cslSmiDisInt(pSmi);
#endif

	if (osRes != OSSTATUS_SUCCESS) {
		if (osRes == OSSTATUS_TIMEOUT) {
			LCD_DBG(LCD_DBG_ERR_ID,
				"[CSL SMI] %s: ERR Timed Out!\n\r",
				__FUNCTION__);
			res = CSL_LCD_OS_TOUT;
		} else {
			LCD_DBG(LCD_DBG_ERR_ID,
				"[CSL SMI] %s: ERR OS Err...\n\r",
				__FUNCTION__);
			res = CSL_LCD_OS_ERR;
		}
	}

	return (res);
}

//*****************************************************************************
//
// Function Name:  cslSmi_HISR
// 
// Description:    SMI Controller HISR
//
//*****************************************************************************
#ifndef __KERNEL__
static void cslSmi_HISR(void)
{
#if defined(__SMI_SPI_ENABLE_TRACE_MSG__ )
	LCD_DBG(LCD_DBG_ID, "[CSL SMI] %s:\r\n", __FUNCTION__);
#endif
	OSSEMAPHORE_Release(smiSemaInt);
}
#endif

//*****************************************************************************
//
// Function Name:  cslSmi_LISR
// 
// Description:    SMI Controller LISR 
//
//*****************************************************************************
#ifndef __KERNEL__
static void cslSmi_LISR(void)
#else
static irqreturn_t cslSmi_LISR(int irq, void *dev_id)
#endif
{
	IRQ_Disable(BCM_INT_ID_SMI);
	IRQ_Clear(BCM_INT_ID_SMI);

	cslSmiDisInt(cslSmiSpiDrv.activeHandle);

	//OSINTERRUPT_Trigger( sSmiHisr );
	OSSEMAPHORE_Release(smiSemaInt);

#ifdef __KERNEL__
	return IRQ_HANDLED;
#endif
}

//*****************************************************************************
//
// Function Name: cslSmiOsInit
//
// Description:   
//
//*****************************************************************************
static Boolean cslSmiOsInit(void)
{
	Task_t task;
	Boolean res = TRUE;

	updReqQ = OSQUEUE_Create(FLUSH_Q_SIZE,
				 sizeof(SMI_UPD_REQ_MSG_T), OSSUSPEND_PRIORITY);

	if (!updReqQ) {
		LCD_DBG(LCD_DBG_ERR_ID,
			"[CSL SMI/SPI] cslSmiOsInit: ERR Queue creation\r\n");
		res = FALSE;
	} else {
		OSQUEUE_ChangeName(updReqQ, "SmiQ");
	}

	task = OSTASK_Create(cslSmiUpdateTask, TASKNAME_SMI,
			     TASKPRI_SMI, STACKSIZE_SMI);

	if (!task) {
		LCD_DBG(LCD_DBG_ERR_ID,
			"[CSL SMI/SPI] cslSmiOsInit: ERR Task creation!\r\n");
		res = FALSE;
	}

	eofDmaSema = OSSEMAPHORE_Create(0, OSSUSPEND_PRIORITY);
	if (!eofDmaSema) {
		LCD_DBG(LCD_DBG_ERR_ID,
			"[CSL SMI/SPI] cslSmiOsInit: ERR Sema Creation!\r\n");
		res = FALSE;
	} else {
		OSSEMAPHORE_ChangeName(eofDmaSema, "SmiSDma");
	}

	smiSema = OSSEMAPHORE_Create(1, OSSUSPEND_PRIORITY);
	if (!smiSema) {
		LCD_DBG(LCD_DBG_ERR_ID,
			"[CSL SMI/SPI] cslSmiOsInit: ERR Sema Creation!\r\n");
		res = FALSE;
	} else {
		OSSEMAPHORE_ChangeName(smiSema, "SmiS");
	}

	// SMI Interrupt Event Semaphore
	smiSemaInt = OSSEMAPHORE_Create(0, OSSUSPEND_PRIORITY);
	if (!smiSemaInt) {
		LCD_DBG(LCD_DBG_ERR_ID,
			"[CSL SMI/SPI] cslSmiOsInit: ERR Sema Creation!\r\n");
		res = FALSE;
	} else {
		OSSEMAPHORE_ChangeName(smiSema, "SmiSInt");
	}

#ifndef __KERNEL__
	// SMI Controller Interrupt
	sSmiHisr = OSINTERRUPT_Create((IEntry_t)&cslSmi_HISR, HISRNAME_SMISTAT,
				      IPRIORITY_MIDDLE, HISRSTACKSIZE_SMISTAT);
#endif

#ifndef __KERNEL__
	IRQ_Register(MM_SMI_IRQ, cslSmi_LISR);
#else
	{
		int ret =
		    request_irq(BCM_INT_ID_SMI, cslSmi_LISR, IRQF_DISABLED,
				"rhea smi interrupt", (void *)NULL);
		if (ret < 0) {
			printk(KERN_ERR
			       "Unable to register Interrupt for Rhea SMI interface\n");
			res = FALSE;
		}
	}

#endif
	return (res);
}				// cslSmiOsInit

//*****************************************************************************
//
// Function Name:   cslSmiWaitTxFifoHasSpace
//
// Description:     wait until TX FIFO can accept data [BEFORE SENDING COMMAND]
//                  
//*****************************************************************************
INLINE static void cslSmiWaitTxFifoHasSpace(pSMI_SPI_HANDLE pSmi)
{
	while ((chal_smi_rd_status_fifo(pSmi->chalH)
		& CHAL_SMI_CS_TX_FIFO_HAS_SPACE) == 0) ;
}				// cslSmiWaitTxFifoHasSpace

//*****************************************************************************
//
// Function Name:   cslSmiWaitRxFifoHasData
//
// Description:     wait until FIFO has RD data [BEFORE READING DATA FROM FIFO]
//                  
//*****************************************************************************
INLINE static void cslSmiWaitRxFifoHasData(pSMI_SPI_HANDLE pSmi)
{
	while ((chal_smi_rd_status_fifo(pSmi->chalH)
		& CHAL_SMI_CS_RX_FIFO_HAS_DATA) == 0) ;
}				// cslSmiWaitRxFifoHasData

//*****************************************************************************
//
// Function Name:   cslSmiWaitDirDone
//
// Description:     wait until DIRECT Interface is Done
//                  
//*****************************************************************************
INLINE static void cslSmiWaitDirDone(pSMI_SPI_HANDLE pSmi)
{
	while ((chal_smi_rd_status_dir(pSmi->chalH)
		& CHAL_SMI_DIR_DONE) == 0) ;
}				// cslSmiWaitDirDone

//*****************************************************************************
//
// Function Name:   cslSmiWaitProgDone
//
// Description:     wait until Programmed Interface is Done
//                  
//*****************************************************************************
INLINE static void cslSmiWaitProgDone(pSMI_SPI_HANDLE pSmi)
{
	while ((chal_smi_rd_status_fifo(pSmi->chalH)
		& CHAL_SMI_CS_TRANSFER_DONE) == 0) ;
}				// cslSmiWaitProgDone

//#############################################################################
//
//                             DMA INTERFACE
//
//#############################################################################

//*****************************************************************************
//
// Function Name:   cslSmiSpiDmaStart
//
// Description:     Prepare & Start Linear DMA Transfer
//                  
//*****************************************************************************
static CSL_LCD_RES_T cslSmiSpiDmaStart(pSMI_SPI_HANDLE smiSpiH,
				       pSMI_DMA_REQ dmaReq,
				       DMA_VC4LITE_CHANNEL_t *pDmaCh)
{
	CSL_LCD_RES_T result = CSL_LCD_OK;

	DMA_VC4LITE_CHANNEL_INFO_t dmaChInfo;
	DMA_VC4LITE_XFER_DATA_t dmaData;

	Int32 dmaCh;

	// Reserve Channel
	if (smiSpiH->ctrlType == CTRL_SMI) {
		dmaCh =
		    csl_dma_vc4lite_obtain_channel(DMA_VC4LITE_CLIENT_MEMORY,
						   DMA_VC4LITE_CLIENT_SMI);
	} else {
		dmaCh =
		    csl_dma_vc4lite_obtain_channel(DMA_VC4LITE_CLIENT_MEMORY,
						   DMA_VC4LITE_CLIENT_SPI_TX);
	}

	if (dmaCh == -1) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL SMI/SPI] %s: "
			"ERR Getting DMA Ch\r\n ", __FUNCTION__);
		return CSL_LCD_DMA_ERR;
	}
	*pDmaCh = (DMA_VC4LITE_CHANNEL_t)dmaCh;

#if defined( __SMI_SPI_ENABLE_TRACE_MSG__ )
	LCD_DBG(LCD_DBG_ID, "[CSL SMI/SPI] %s: "
		"Got DmaCh[%d]\r\n ", __FUNCTION__, dmaCh);
#endif

	// Configure Channel
	dmaChInfo.autoFreeChan = 1;
	dmaChInfo.srcID = DMA_VC4LITE_CLIENT_MEMORY;
	if (smiSpiH->ctrlType == CTRL_SMI) {
		dmaChInfo.dstID = DMA_VC4LITE_CLIENT_SMI;
		//       dmaChInfo.burstLen  = DMA_VC4LITE_BURST_LENGTH_4;  // FrameTime=20.8ms
		dmaChInfo.burstLen = DMA_VC4LITE_BURST_LENGTH_8;	// FrameTime=20.8ms
		//       dmaChInfo.burstLen  = DMA_VC4LITE_BURST_LENGTH_16; // FrameTime=46.78ms ???
	} else {
		dmaChInfo.dstID = DMA_VC4LITE_CLIENT_SPI_TX;
		dmaChInfo.burstLen = DMA_VC4LITE_BURST_LENGTH_4;
	}

	dmaChInfo.xferMode = DMA_VC4LITE_XFER_MODE_LINERA;
	dmaChInfo.dstStride = 0;
	dmaChInfo.srcStride = dmaReq->xStrideBytes;
	dmaChInfo.waitResponse = 0;
	dmaChInfo.callback = (DMA_VC4LITE_CALLBACK_t) cslSmiEofDma;

	if (csl_dma_vc4lite_config_channel(*pDmaCh, &dmaChInfo)
	    != DMA_VC4LITE_STATUS_SUCCESS) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL SMI/SPI] %s: "
			"ERR DMA Ch Config\r\n ", __FUNCTION__);
		return CSL_LCD_DMA_ERR;
	}
	// Add the DMA transfer data
	dmaData.srcAddr = (UInt32)dmaReq->pBuff;

	if (smiSpiH->ctrlType == CTRL_SMI)
		dmaData.dstAddr = chal_smi_get_dma_addr(smiSpiH->chalH);
	else
		dmaData.dstAddr = chal_spivc4l_get_dma_addr(smiSpiH->chalH);

	dmaData.xferLength = dmaReq->xLenBytes * dmaReq->yLen;
	dmaData.burstWriteEnable32 = 0;

	if (csl_dma_vc4lite_add_data(*pDmaCh, &dmaData)
	    != DMA_VC4LITE_STATUS_SUCCESS) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL SMI/SPI] %s: "
			"ERR add DMA transfer data\r\n ", __FUNCTION__);
		return CSL_LCD_DMA_ERR;
	}
	// start DMA transfer
	if (csl_dma_vc4lite_start_transfer(*pDmaCh)
	    != DMA_VC4LITE_STATUS_SUCCESS) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL SMI/SPI] %s: "
			"ERR start DMA data transfer\r\n ", __FUNCTION__);
		return CSL_LCD_DMA_ERR;
	}

	return (result);
}

//*****************************************************************************
//
// Function Name:   cslSmiSpiDmaStart_2D
//
// Description:     Prepare & Start 2D DMA Transfer
//                  
//*****************************************************************************
static CSL_LCD_RES_T cslSmiSpiDmaStart_2D(pSMI_SPI_HANDLE smiSpiH,
					  pSMI_DMA_REQ dmaReq,
					  DMA_VC4LITE_CHANNEL_t *pDmaCh)
{
	CSL_LCD_RES_T result = CSL_LCD_OK;

	DMA_VC4LITE_CHANNEL_INFO_t dmaChInfo;
	DMA_VC4LITE_XFER_2DDATA_t dmaData;

	Int32 dmaCh;

	// Reserve Channel
	if (smiSpiH->ctrlType == CTRL_SMI) {
		dmaCh =
		    csl_dma_vc4lite_obtain_channel(DMA_VC4LITE_CLIENT_MEMORY,
						   DMA_VC4LITE_CLIENT_SMI);
	} else {
		dmaCh =
		    csl_dma_vc4lite_obtain_channel(DMA_VC4LITE_CLIENT_MEMORY,
						   DMA_VC4LITE_CLIENT_SPI_TX);
	}

	if (dmaCh == -1) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL SMI/SPI] %s: "
			"ERR Getting DMA Ch\r\n ", __FUNCTION__);
		return CSL_LCD_DMA_ERR;
	}
	*pDmaCh = (DMA_VC4LITE_CHANNEL_t)dmaCh;

#if defined( __SMI_SPI_ENABLE_TRACE_MSG__ )
	LCD_DBG(LCD_DBG_ID, "[CSL SMI/SPI] %s: "
		"Got DmaCh[%d]\r\n ", __FUNCTION__, dmaCh);
#endif

	// Configure Channel
	dmaChInfo.autoFreeChan = 1;
	dmaChInfo.srcID = DMA_VC4LITE_CLIENT_MEMORY;
	if (smiSpiH->ctrlType == CTRL_SMI) {
		dmaChInfo.dstID = DMA_VC4LITE_CLIENT_SMI;
		dmaChInfo.burstLen = DMA_VC4LITE_BURST_LENGTH_8;	// FrameTime=20.8ms
	} else {
		dmaChInfo.dstID = DMA_VC4LITE_CLIENT_SPI_TX;
		dmaChInfo.burstLen = DMA_VC4LITE_BURST_LENGTH_4;
	}
	dmaChInfo.xferMode = DMA_VC4LITE_XFER_MODE_2D;
	dmaChInfo.dstStride = 0;
	dmaChInfo.srcStride = dmaReq->xStrideBytes;
	dmaChInfo.waitResponse = 0;
	dmaChInfo.callback = (DMA_VC4LITE_CALLBACK_t) cslSmiEofDma;

	if (csl_dma_vc4lite_config_channel(*pDmaCh, &dmaChInfo)
	    != DMA_VC4LITE_STATUS_SUCCESS) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL SMI/SPI] %s: "
			"ERR DMA Ch Config\r\n ", __FUNCTION__);
		return CSL_LCD_DMA_ERR;
	}
	// Add the DMA transfer data
	dmaData.srcAddr = (UInt32)dmaReq->pBuff;

	if (smiSpiH->ctrlType == CTRL_SMI)
		dmaData.dstAddr = chal_smi_get_dma_addr(smiSpiH->chalH);
	else
		dmaData.dstAddr = chal_spivc4l_get_dma_addr(smiSpiH->chalH);

	dmaData.xXferLength = dmaReq->xLenBytes;
	dmaData.yXferLength = dmaReq->yLen - 1;
	dmaData.burstWriteEnable32 = 0;

	if (csl_dma_vc4lite_add_data_ex(*pDmaCh, &dmaData)
	    != DMA_VC4LITE_STATUS_SUCCESS) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL SMI/SPI] %s: "
			"ERR add DMA transfer data\r\n ", __FUNCTION__);
		return CSL_LCD_DMA_ERR;
	}
	// start DMA transfer
	if (csl_dma_vc4lite_start_transfer(*pDmaCh)
	    != DMA_VC4LITE_STATUS_SUCCESS) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL SMI/SPI] %s: "
			"ERR start DMA data transfer\r\n ", __FUNCTION__);
		return CSL_LCD_DMA_ERR;
	}

	return (result);
}

//*****************************************************************************
//
// Function Name:   cslSmiSpiDmaStop
//
// Description:     Stops DMA
//                  
//*****************************************************************************
static CSL_LCD_RES_T cslSmiSpiDmaStop(DMA_VC4LITE_CHANNEL_t dmaCh)
{
	CSL_LCD_RES_T result = CSL_LCD_OK;

	// stop DMA transfer
	if (csl_dma_vc4lite_stop_transfer(dmaCh)
	    != DMA_VC4LITE_STATUS_SUCCESS) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL SMI/SPI] %s: "
			"ERR stop DMA data transfer\r\n ", __FUNCTION__);
		return CSL_LCD_DMA_ERR;
	}
	// release DMA channel
	if (csl_dma_vc4lite_release_channel(dmaCh)
	    != DMA_VC4LITE_STATUS_SUCCESS) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL SMI/SPI] %s: "
			"ERR stop DMA data transfer\r\n ", __FUNCTION__);
		return CSL_LCD_DMA_ERR;
	}
	return result;
}				// cslSmiSpiDmaStop

//*****************************************************************************
//                 
// Function Name:  cslSmiSetTiming
// 
// Description:    caclulate SMI Timing
//                
//*****************************************************************************
static int cslSmiCalcTiming(pSMI_SPI_HANDLE pSmi,
			    pCSL_SMI_TIMING pCslTiming,
			    pCHAL_SMI_TIMING pChalTiming,
			    pCHAL_SMI_TIMING pChalTimingMax)
{

	UInt32 smi_clk_ns;
	int res = 0;

	smi_clk_ns = pSmi->cfg.smiCfg.smi_clk_ns;

	pChalTiming->strobe = ((pCslTiming->strobe_ns + smi_clk_ns)
			       / smi_clk_ns);
	pChalTiming->pace = ((pCslTiming->pace_ns + smi_clk_ns)
			     / smi_clk_ns);
	pChalTiming->hold = ((pCslTiming->hold_ns + smi_clk_ns)
			     / smi_clk_ns);
	pChalTiming->setup = ((pCslTiming->setup_ns + smi_clk_ns)
			      / smi_clk_ns);

	if (pChalTiming->strobe > pChalTimingMax->strobe)
		res = -1;
	if (pChalTiming->pace > pChalTimingMax->pace)
		res = -1;
	if (pChalTiming->hold > pChalTimingMax->hold)
		res = -1;
	if (pChalTiming->setup > pChalTimingMax->setup)
		res = -1;
	if (res == -1) {
		LCD_DBG(LCD_DBG_ERR_ID,
			"[CSL SMI] %s: ERR Timing Reg Overflow\n\r",
			__FUNCTION__);
	}

	LCD_DBG(LCD_DBG_INIT_ID, "[CSL SMI] %s: "
		"INFO Setup  IN %d[ns] OUT %d[cnt]=>%d[ns]\n\r", __FUNCTION__,
		pCslTiming->setup_ns, pChalTiming->setup,
		pChalTiming->setup * smi_clk_ns);
	LCD_DBG(LCD_DBG_INIT_ID, "[CSL SMI] %s: "
		"INFO Strobe IN %d[ns] OUT %d[cnt]=>%d[ns]\n\r", __FUNCTION__,
		pCslTiming->strobe_ns, pChalTiming->strobe,
		pChalTiming->strobe * smi_clk_ns);
	LCD_DBG(LCD_DBG_INIT_ID, "[CSL SMI] %s: "
		"INFO Hold   IN %d[ns] OUT %d[cnt]=>%d[ns]\n\r", __FUNCTION__,
		pCslTiming->hold_ns, pChalTiming->hold,
		pChalTiming->hold * smi_clk_ns);
	LCD_DBG(LCD_DBG_INIT_ID, "[CSL SMI] %s: "
		"INFO Pace   IN %d[ns] OUT %d[cnt]=>%d[ns]\n\r", __FUNCTION__,
		pCslTiming->pace_ns, pChalTiming->pace,
		pChalTiming->pace * smi_clk_ns);

	return (res);
}

//*****************************************************************************
//                 
// Function Name:  cslSmi2cHal
// 
// Description:    Convert Interface from CSL 2 CHAL 
//                 Verify Configuration.
//
//*****************************************************************************
static CSL_LCD_RES_T cslSmi2cHal(pSMI_SPI_HANDLE pSmi, CSL_SMI_CTRL_T *smiCfg)
{
	CHAL_SMI_TIMING_T chalTimingMax;

	// MAX values are same for RD & WR cycle
	chalTimingMax.strobe = CHAL_SMI_WR_STROBE_MAX;
	chalTimingMax.pace = CHAL_SMI_WR_PACE_MAX;
	chalTimingMax.hold = CHAL_SMI_WR_HOLD_MAX;
	chalTimingMax.setup = CHAL_SMI_WR_SETUP_MAX;

	switch (smiCfg->colModeIn) {
	case LCD_IF_CM_I_RGB565P:
		pSmi->cfg.smiCfg.smiMode.inPixelMode = CHAL_SMI_CM_16B_RGB565;
		pSmi->cfg.smiCfg.buffBpp = 2;
		break;
	case LCD_IF_CM_I_xRGB8888:
		pSmi->cfg.smiCfg.smiMode.inPixelMode = CHAL_SMI_CM_32B_RGB888;
		pSmi->cfg.smiCfg.buffBpp = 4;
		break;
//      case LCD_IF_CM_I_NONE:    
//          pSmi->cfg.smiCfg.smiMode.inPixelMode = CHAL_SMI_CM_NONE;
//          pSmi->cfg.smiCfg.buffBpp = 4;
//          break;
	default:
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL SMI] %s: ERROR Invalid Input "
			"Color Mode!\n\r", __FUNCTION__);
		return (CSL_LCD_COL_MODE);
	}

	switch (smiCfg->busWidth) {
	case 8:
		if (!(((smiCfg->colModeIn == LCD_IF_CM_I_RGB565P)
		       && (smiCfg->colModeOut == LCD_IF_CM_O_RGB565))
		      || ((smiCfg->colModeIn == LCD_IF_CM_I_xRGB8888)
			  && (smiCfg->colModeOut == LCD_IF_CM_O_xRGB8888)))) {
			return (CSL_LCD_COL_MODE);
		}
		pSmi->cfg.smiCfg.busWidth = CHAL_SMI_BUSW_08B;
		pSmi->cfg.smiCfg.wrTiming.busWidth = CHAL_SMI_BUSW_08B;
		pSmi->cfg.smiCfg.wrTiming_m.busWidth = CHAL_SMI_BUSW_08B;
		pSmi->cfg.smiCfg.rdTiming.busWidth = CHAL_SMI_BUSW_08B;
		break;
#if ( defined(_RHEA_) || defined(_SAMOA_) )
	case 9:
		if (smiCfg->colModeOut != LCD_IF_CM_O_RGB666) {
			return (CSL_LCD_COL_MODE);
		}
		pSmi->cfg.smiCfg.busWidth = CHAL_SMI_BUSW_09B;
		pSmi->cfg.smiCfg.wrTiming.busWidth = CHAL_SMI_BUSW_09B;
		pSmi->cfg.smiCfg.wrTiming_m.busWidth = CHAL_SMI_BUSW_09B;
		pSmi->cfg.smiCfg.rdTiming.busWidth = CHAL_SMI_BUSW_09B;
		break;
#endif
#if ( defined(_RHEA_) )
	case 16:
		if (!(((smiCfg->colModeIn == LCD_IF_CM_I_RGB565P)
		       && (smiCfg->colModeOut == LCD_IF_CM_O_RGB565))
		      || ((smiCfg->colModeIn == LCD_IF_CM_I_xRGB8888)
			  && (smiCfg->colModeOut == LCD_IF_CM_O_xRGB8888)))) {
			return (CSL_LCD_COL_MODE);
		}
		pSmi->cfg.smiCfg.busWidth = CHAL_SMI_BUSW_16B;
		pSmi->cfg.smiCfg.wrTiming.busWidth = CHAL_SMI_BUSW_16B;
		pSmi->cfg.smiCfg.wrTiming_m.busWidth = CHAL_SMI_BUSW_16B;
		pSmi->cfg.smiCfg.rdTiming.busWidth = CHAL_SMI_BUSW_16B;
		break;
	case 18:
		if (smiCfg->colModeOut != LCD_IF_CM_O_RGB666) {
			return (CSL_LCD_COL_MODE);
		}
		pSmi->cfg.smiCfg.busWidth = CHAL_SMI_BUSW_18B;
		pSmi->cfg.smiCfg.wrTiming.busWidth = CHAL_SMI_BUSW_18B;
		pSmi->cfg.smiCfg.wrTiming_m.busWidth = CHAL_SMI_BUSW_18B;
		pSmi->cfg.smiCfg.rdTiming.busWidth = CHAL_SMI_BUSW_18B;
		break;
#endif
	default:
		LCD_DBG(LCD_DBG_ID, "[CSL SMI] cslSmi2cHal: "
			"ERR Bus Width Configuration\n\r");
		return (CSL_LCD_BUS_CFG);
	}

	// for now, not part of peripheral setup 
	pSmi->cfg.smiCfg.rdTiming.paceAll = TRUE;
	pSmi->cfg.smiCfg.wrTiming.paceAll = TRUE;
	pSmi->cfg.smiCfg.wrTiming_m.paceAll = TRUE;

	pSmi->cfg.smiCfg.addr_c = smiCfg->addr_c;
	pSmi->cfg.smiCfg.addr_d = smiCfg->addr_d;

	pSmi->cfg.smiCfg.smiMode.m68 = smiCfg->m68;
	pSmi->cfg.smiCfg.smiMode.swap = smiCfg->swap;
	pSmi->cfg.smiCfg.smiMode.setupFirstTrasferOnly =
	    smiCfg->setupFirstTrOnly;

	LCD_DBG(LCD_DBG_INIT_ID,
		"smicfg info: addr=0x%08x  pixel in mode= %d bpp=%d buswidth=%d \n",
		&pSmi->cfg.smiCfg, pSmi->cfg.smiCfg.smiMode.inPixelMode,
		pSmi->cfg.smiCfg.buffBpp, pSmi->cfg.smiCfg.busWidth);

	if (cslSmiCalcTiming
	    (pSmi, &smiCfg->rdTiming, &pSmi->cfg.smiCfg.rdTiming,
	     &chalTimingMax) != 0) {
		return (CSL_LCD_BUS_CFG);
	}

	if (cslSmiCalcTiming(pSmi, &smiCfg->wrTiming_r,
			     &pSmi->cfg.smiCfg.wrTiming, &chalTimingMax) != 0) {
		return (CSL_LCD_BUS_CFG);
	}

	if ((smiCfg->wrTiming_m.hold_ns == 0)
	    && (smiCfg->wrTiming_m.pace_ns == 0)
	    && (smiCfg->wrTiming_m.setup_ns == 0)
	    && (smiCfg->wrTiming_m.strobe_ns == 0)) {
		pSmi->cfg.smiCfg.usesMemWrTiming = FALSE;
	} else {
		pSmi->cfg.smiCfg.usesMemWrTiming = TRUE;
		if (cslSmiCalcTiming(pSmi, &smiCfg->wrTiming_m,
				     &pSmi->cfg.smiCfg.wrTiming_m,
				     &chalTimingMax) != 0) {
			return (CSL_LCD_BUS_CFG);
		}
	}

	return (CSL_LCD_OK);
}

//*****************************************************************************
//                 
// Function Name:  cslSmiBankConfig
// 
// Description:    Configure SMI bank
//                 
//*****************************************************************************
static CSL_LCD_RES_T cslSmiBankConfig(pSMI_SPI_HANDLE pSmi)
{
	CSL_LCD_RES_T res = CSL_LCD_OK;

	chal_smi_setup_mode(pSmi->chalH, pSmi->cfg.smiCfg.bank,
			    &pSmi->cfg.smiCfg.smiMode);
	chal_smi_setup_rd_timing(pSmi->chalH, pSmi->cfg.smiCfg.bank,
				 &pSmi->cfg.smiCfg.rdTiming);
	chal_smi_setup_wr_timing(pSmi->chalH, pSmi->cfg.smiCfg.bank,
				 &pSmi->cfg.smiCfg.wrTiming);
	pSmi->cfg.smiCfg.isRegWrTiming = TRUE;
	return (res);
}				// cslSmiBankConfig

//*****************************************************************************
//                 
// Function Name:  cslSmiSpiSwitchHandle
// 
// Description:    Switch Active Handle
//                 Activate Controller Configuration (pinmuxes ... )
//                 
//*****************************************************************************
static void cslSmiSpiSwitchHandle(pSMI_SPI_HANDLE pSmiSpi)
{
	if (cslSmiSpiDrv.activeHandle != pSmiSpi) {
		if (pSmiSpi->ctrlType == CTRL_SMI) {
			cslSmiMuxOn();
		} else {
			cslSpiMuxOn();
		}
		cslSmiSpiDrv.activeHandle = pSmiSpi;
	}
}

//*****************************************************************************
//
// Function Name:  CSL_SMI_Lock
// 
// Description:    
//                 
//*****************************************************************************
CSL_LCD_RES_T CSL_SMI_Lock(CSL_LCD_HANDLE smiSpiH)
{
	pSMI_SPI_HANDLE pSmiSpi = (pSMI_SPI_HANDLE) smiSpiH;

	if (!pSmiSpi->hasLock) {
		OSSEMAPHORE_Obtain(smiSema, TICKS_FOREVER);
		pSmiSpi->hasLock = TRUE;
	}
	return (CSL_LCD_OK);
}

//*****************************************************************************
//
// Function Name:  CSL_SMI_Unlock
// 
// Description:    
//                 
//*****************************************************************************
CSL_LCD_RES_T CSL_SMI_Unlock(CSL_LCD_HANDLE smiSpiH)
{
	pSMI_SPI_HANDLE pSmiSpi = (pSMI_SPI_HANDLE) smiSpiH;

	if (pSmiSpi->hasLock) {
		OSSEMAPHORE_Release(smiSema);
		pSmiSpi->hasLock = FALSE;
	}
	return (CSL_LCD_OK);
}

//*****************************************************************************
//
// Function Name:  CSL_SMI_WrDirect
// 
// Description:    Execute Write Command or Data Cycle Using Direct Interface
//                 
//*****************************************************************************
CSL_LCD_RES_T CSL_SMI_WrDirect(CSL_LCD_HANDLE smiH, Boolean cmnd,	// data or command WR cycle
			       UInt32 data)
{
	pSMI_SPI_HANDLE pSmi = (pSMI_SPI_HANDLE) smiH;

	CSL_LCD_RES_T res = CSL_LCD_OK;

	if (!pSmi->hasLock)
		OSSEMAPHORE_Obtain(smiSema, TICKS_FOREVER);

	cslSmiSpiSwitchHandle(smiH);

	if (pSmi->cfg.smiCfg.usesMemWrTiming && !pSmi->cfg.smiCfg.isRegWrTiming) {
		chal_smi_setup_wr_timing(pSmi->chalH, pSmi->cfg.smiCfg.bank,
					 &pSmi->cfg.smiCfg.wrTiming);
		pSmi->cfg.smiCfg.isRegWrTiming = TRUE;
	}

	chal_smi_dir_write_acc(pSmi->chalH, pSmi->cfg.smiCfg.bank,
			       cmnd ? pSmi->cfg.smiCfg.addr_c : pSmi->cfg.
			       smiCfg.addr_d, data);

	cslSmiWaitDirDone(pSmi);

	if (!pSmi->hasLock)
		OSSEMAPHORE_Release(smiSema);

	return (res);
}

//*****************************************************************************
//
// Function Name:  CSL_SMI_RdDirect
// 
// Description:    Execute Read Command or Data Cycle Using Direct Interface
//                 
//*****************************************************************************
CSL_LCD_RES_T CSL_SMI_RdDirect(CSL_LCD_HANDLE smiH, Boolean cmnd,	// data or command RD cycle
			       UInt32 *data)
{
	pSMI_SPI_HANDLE pSmi = (pSMI_SPI_HANDLE) smiH;
	CSL_LCD_RES_T res = CSL_LCD_OK;

	if (!pSmi->hasLock)
		OSSEMAPHORE_Obtain(smiSema, TICKS_FOREVER);

	cslSmiSpiSwitchHandle(smiH);

	chal_smi_dir_read_acc(pSmi->chalH, pSmi->cfg.smiCfg.bank,
			      cmnd ? pSmi->cfg.smiCfg.addr_c : pSmi->cfg.smiCfg.
			      addr_d);

	cslSmiWaitDirDone(pSmi);

	*data = chal_smi_dir_read_data(pSmi->chalH);

	if (!pSmi->hasLock)
		OSSEMAPHORE_Release(smiSema);

	return (res);
}

//*****************************************************************************
//
// Function Name: CSL_SMI_WrRdDataProg
// 
// Description:   Generic RD/WR Data Programmed Access (FIFO)
//                
//*****************************************************************************
CSL_LCD_RES_T CSL_SMI_WrRdDataProg(CSL_LCD_HANDLE smiH, pSMI_PROG_ACC acc)
{
	OSStatus_t status;
	CSL_LCD_RES_T res = CSL_LCD_OK;
	pSMI_SPI_HANDLE pSmi = (pSMI_SPI_HANDLE) smiH;
	SMI_UPD_REQ_MSG_T updMsg;
	UInt32 byteCount;
	UInt32 i;
	UInt32 fwc;
	CHAL_FIFO_ACC_t fifoMode;
	SMI_DMA_REQ_t dmaReq;

	if (!pSmi->hasLock)
		OSSEMAPHORE_Obtain(smiSema, TICKS_FOREVER);

	cslSmiSpiSwitchHandle(smiH);

	byteCount = acc->xLenInBytes * acc->yLen;

	fifoMode.smilL = chal_smi_calc_smil_len(byteCount,
						pSmi->cfg.smiCfg.smiMode.
						inPixelMode,
						pSmi->cfg.smiCfg.busWidth);

	fifoMode.bank = pSmi->cfg.smiCfg.bank;
	fifoMode.addr = pSmi->cfg.smiCfg.addr_d;
	if (pSmi->cfg.smiCfg.smiMode.inPixelMode == CHAL_SMI_CM_NONE)
		fifoMode.isPixelFormat = FALSE;
	else
		fifoMode.isPixelFormat = TRUE;

	fifoMode.isTE = acc->isTE;
	fifoMode.isWrite = acc->isWr;
	fifoMode.isDma = acc->isDma;

	if (pSmi->cfg.smiCfg.usesMemWrTiming && acc->isWr && acc->isWrMem) {
		chal_smi_setup_wr_timing(pSmi->chalH, pSmi->cfg.smiCfg.bank,
					 &pSmi->cfg.smiCfg.wrTiming_m);
		pSmi->cfg.smiCfg.isRegWrTiming = FALSE;
	}

	if (!fifoMode.isDma) {
#if defined( __SMI_SPI_ENABLE_TRACE_MSG__ )
		LCD_DBG(LCD_DBG_ID, "[CSL SMI] +%s: "
			"FIFO Non-DMA Mode ...\r\n", __FUNCTION__);
#endif
		// start FIFO mode
		chal_smi_fifo_acc(pSmi->chalH, &fifoMode);

		fwc = byteCount >> 2;

		if (fifoMode.isWrite) {
			for (i = 0; i < fwc; i++) {
				cslSmiWaitTxFifoHasSpace(pSmi);
				chal_smi_fifo_wr(pSmi->chalH, acc->pBuff[i]);
			}

			cslSmiWaitProgDone(pSmi);
		} else {
			for (i = 0; i < fwc; i++) {
				cslSmiWaitRxFifoHasData(pSmi);
				acc->pBuff[i] = chal_smi_fifo_rd(pSmi->chalH);
			}

			cslSmiWaitProgDone(pSmi);
		}

		// DISABLE PROG mode
		chal_smi_fifo_done(pSmi->chalH);

		if (!pSmi->hasLock)
			OSSEMAPHORE_Release(smiSema);

#if defined( __SMI_SPI_ENABLE_TRACE_MSG__ )
		LCD_DBG(LCD_DBG_ID, "[CSL SMI] -%s: "
			"FIFO Non-DMA Mode ...\r\n", __FUNCTION__);
#endif
	} else {
		// DMA request 
		dmaReq.xLenBytes = acc->xLenInBytes;
		dmaReq.xStrideBytes = acc->xStrideInBytes;
		dmaReq.yLen = acc->yLen;
		dmaReq.isWrite = acc->isWr;
		dmaReq.pBuff = acc->pBuff;

		// enable DONE int
		cslSmiEnaIntEvent(pSmi, CHAL_SMI_CS_INT_DONE);

		// start FIFO mode
		chal_smi_fifo_acc(pSmi->chalH, &fifoMode);

		// start DMA
		if (acc->xStrideInBytes == 0)
			res = cslSmiSpiDmaStart(pSmi, &dmaReq, &updMsg.dmaCh);
		else
			res =
			    cslSmiSpiDmaStart_2D(pSmi, &dmaReq, &updMsg.dmaCh);
		if (res != CSL_LCD_OK) {
			cslSmiDisInt(pSmi);
			if (!pSmi->hasLock)
				OSSEMAPHORE_Release(smiSema);
			return (res);
		}

		if (acc->cslLcdCb == NULL) {	// BLOCKING                                 

#if defined( __SMI_SPI_ENABLE_TRACE_MSG__ )
			LCD_DBG(LCD_DBG_ID, "[CSL SMI] +%s: "
				"FIFO DMA Mode Blocking\r\n", __FUNCTION__);
#endif
			status = OSSEMAPHORE_Obtain(eofDmaSema,
						    TICKS_IN_MILLISECONDS(acc->
									  timeOutMsec));

			if (status != OSSTATUS_SUCCESS) {
				//asm("bkpt");
				//panic("No VC4 DMA int back\n");
				cslSmiDisInt(pSmi);
				cslSmiSpiDmaStop(updMsg.dmaCh);

				if (status == OSSTATUS_TIMEOUT) {
					LCD_DBG(LCD_DBG_ERR_ID,
						"[CSL SMI] CSL_SMI_WrRdDataProg: "
						"ERR EOFDMA Timeout!\n\r");
					res = CSL_LCD_OS_TOUT;
				} else {
					LCD_DBG(LCD_DBG_ERR_ID,
						"[CSL SMI] CSL_SMI_WrRdDataProg: "
						"ERR EOFDMA OS Error!\n\r");
					res = CSL_LCD_OS_ERR;
				}
			} else {
				// wait for end of PROG MODE, then Disable PROG mode
				res = cslSmiWaitForInt(pSmi, acc->timeOutMsec);
				if (res != CSL_LCD_OK) {
					LCD_DBG(LCD_DBG_ERR_ID,
						"[CSL SMI/SPI] %s: DONE INT "
						"T-OUT\r\n", __FUNCTION__);
				}
			}

#if defined( __SMI_SPI_ENABLE_TRACE_MSG__ )
			LCD_DBG(LCD_DBG_ID, "[CSL SMI] -%s: "
				"FIFO DMA Mode Blocking\r\n", __FUNCTION__);
#endif
			if (!pSmi->hasLock)
				OSSEMAPHORE_Release(smiSema);
		} else {	// NON_BLOCKING

			updMsg.smiH = pSmi;
			updMsg.timeOutMsec = acc->timeOutMsec;
			updMsg.cslLcdCb = acc->cslLcdCb;
			updMsg.cslLcdCbRec = acc->cslLcdCbRec;

#if defined( __SMI_SPI_ENABLE_TRACE_MSG__ )
			LCD_DBG(LCD_DBG_ID, "[CSL SMI] +%s: "
				"FIFO DMA Mode Non-Blocking ...\r\n",
				__FUNCTION__);
#endif
			status =
			    OSQUEUE_Post(updReqQ, (QMsg_t *)&updMsg,
					 TICKS_NO_WAIT);
			if (status != OSSTATUS_SUCCESS) {
				cslSmiDisInt(pSmi);
				cslSmiSpiDmaStop(updMsg.dmaCh);
				res = CSL_LCD_OS_ERR;
				LCD_DBG(LCD_DBG_ERR_ID,
					"[CSL SMI] CSL_SMI_WrRdDataProg: "
					"ERR Posting Msg \n\r");
			}
		}
	}

	return (res);
}				// CSL_SMI_WrRdDataProg

//*****************************************************************************
//
// Function Name: CSL_SMI_UpdateDma
// 
// Description:   Data ( data LCD bus cycles ) Block Transfer
//                
//*****************************************************************************
CSL_LCD_RES_T CSL_SMI_Update(CSL_LCD_HANDLE smiH, CSL_LCD_UPD_REQ_T * req)
{
	SMI_PROG_ACC_t acc;
	pSMI_SPI_HANDLE pSmi = (pSMI_SPI_HANDLE) smiH;

	acc.xLenInBytes = req->lineLenP * req->buffBpp;
	acc.xStrideInBytes = req->xStrideB;
	acc.yLen = req->lineCount;
	acc.pBuff = (UInt32 *)req->buff;
	acc.timeOutMsec = req->timeOut_ms;
	acc.isTE = pSmi->cfg.smiCfg.usesTE;
	acc.isWr = TRUE;
	acc.isDma = TRUE;
	acc.cslLcdCb = req->cslLcdCb;
	acc.cslLcdCbRec = req->cslLcdCbRec;

	if (pSmi->cfg.smiCfg.usesMemWrTiming)
		acc.isWrMem = TRUE;
	else
		acc.isWrMem = FALSE;

	return (CSL_SMI_WrRdDataProg(smiH, &acc));
}				// CSL_SMI_Update

//*****************************************************************************
//                 
// Function Name:  CSL_SMI_Close
// 
// Description:    Close SMI Controller Instance
//
//*****************************************************************************
CSL_LCD_RES_T CSL_SMI_Close(CSL_LCD_HANDLE smiH)
{
	CSL_LCD_RES_T res = CSL_LCD_OK;
	pSMI_SPI_HANDLE pSmi = (pSMI_SPI_HANDLE) smiH;

	if (pSmi->open) {
		pSmi->open = FALSE;
		LCD_DBG(LCD_DBG_INIT_ID, "[CSL SMI] CSL_SMI_Close: "
			"Closing SMI Controller Handle\n\r");
		cslSmiSpiDrv.instance--;

#if defined( __SMI_USE_PRM__ )
		if (pSmi->prm_ID >= 0) {
			PRM_set_clock_state(pSmi->prm_ID, RESOURCE_SMI,
					    CLOCK_OFF);
			PRM_client_deregister(pSmi->prm_ID);
			pSmi->prm_ID = -1;
		}
#endif

		if (cslSmiSpiDrv.activeHandle == pSmi)
			cslSmiSpiDrv.activeHandle = NULL;

	} else {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL SMI] CSL_SMI_Close: "
			"WARNING SMI Handle Not Open!\n\r");
	}
	return (res);
}

//*****************************************************************************
//                 
// Function Name:  CSL_SMI_Open
// 
// Description:    Inits & Configures SMI Controller
//
//*****************************************************************************
CSL_LCD_RES_T CSL_SMI_Open(CSL_SMI_CTRL_T *busCfg, CSL_LCD_HANDLE *smiH)
{
	pSMI_SPI_HANDLE pSmi = NULL;
	CSL_LCD_RES_T res;
	UInt32 i;
	Boolean smiFound = FALSE;

	if (!cslSmiSpiDrv.init) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL SMI] CSL_SMI_Open: "
			"ERR SMI/SPI CSL Not Init!\n\r");
		*smiH = NULL;
		return (CSL_LCD_NOT_INIT);
	}

	if (cslSmiSpiDrv.instance >= SMI_SPI_MAX_HANDLES) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL SMI] CSL_SMI_Open: "
			"ERR Out Of Hanldes!\n\r");
		*smiH = NULL;
		return (CSL_LCD_INST_COUNT);
	}

	for (i = 0; i < SMI_SPI_MAX_HANDLES; i++) {
		if (cslSmiSpiDrv.ctrl[i].open &&
		    (cslSmiSpiDrv.ctrl[i].ctrlType == CTRL_SMI)) {
			LCD_DBG(LCD_DBG_ERR_ID, "[CSL SMI] CSL_SMI_Open: "
				"Only Single SMI Instance Allowed!\n\r");
			smiFound = TRUE;
			break;
		}
	}

	if (!smiFound) {
		for (i = 0; i < SMI_SPI_MAX_HANDLES; i++) {
			if (!cslSmiSpiDrv.ctrl[i].open) {
				pSmi = &cslSmiSpiDrv.ctrl[i];
				break;
			}
		}
	}

	if (pSmi == NULL) {
		*smiH = NULL;
		return (CSL_LCD_INST_COUNT);
	}

	pSmi->chalH = chal_smi_init(KONA_SMI_VA);
	//pSmi->chalH = chal_smi_init ( chal_sysmap_get_base_addr( SMI ) );

	if (cslSmiSetSmiClk(pSmi, &busCfg->smiClk) == -1) {
		*smiH = NULL;
		return (CSL_LCD_BUS_CFG);
	}

	if ((res = cslSmi2cHal(pSmi, busCfg)) != CSL_LCD_OK) {
		*smiH = NULL;
		return (res);
	}

	if (busCfg->usesTE)
		pSmi->cfg.smiCfg.usesTE = TRUE;
	else
		pSmi->cfg.smiCfg.usesTE = FALSE;

	pSmi->open = TRUE;
	pSmi->ctrlType = CTRL_SMI;
	pSmi->cfg.smiCfg.bank = 0;

	cslSmiSpiDrv.instance++;
	cslSmiSpiDrv.activeHandle = pSmi;

	// we cannot access any SMI reg before we enable AXI clk
	cslSmiSetClkEna(TRUE);	// enable SMI clocks (AXI, SMI)
	cslSmiEnablePads();	// enable SMI pads
	cslSmiBankConfig(pSmi);	// configure SMI bank

	*smiH = (CSL_LCD_HANDLE)pSmi;
	LCD_DBG(LCD_DBG_INIT_ID, "[CSL SMI] %s: OK\n\r", __FUNCTION__);

	return (CSL_LCD_OK);
}				// CSL_SMI_Open

//*****************************************************************************
//                 
// Function Name:  CSL_SMI_SPI_Init
// 
// Description:    Init CSL SMI Driver
//
//*****************************************************************************
CSL_LCD_RES_T CSL_SMI_SPI_Init(void)
{
	CSL_LCD_RES_T res = CSL_LCD_OK;

#ifdef FPGA_VERSION
	// 0x35001F00 ROOT_RST: Hera Root Reset Manager
	*(UInt32 *)0x35001F00 = 0x00a5a501;	// needed since FPGA MM rev 6.0
	*(UInt32 *)0x35001F08 |= 0x00000030;	// take MM subsys out of reset ?
#endif // #ifdef FPGA_VERSION

	if (!cslSmiSpiDrv.init) {
		memset((void *)&cslSmiSpiDrv, 0, sizeof(SMI_SPI_DRV_t));

		if (cslSmiOsInit()) {
			cslSmiSpiDrv.init = TRUE;
		} else {
			res = CSL_LCD_ERR;
		}
	}
	return (res);
}

//*****************************************************************************
//                 
// Function Name:  CSL_SPI_VC4L_Close
// 
// Description:    Close SPI Controller Instance
//
//*****************************************************************************
CSL_LCD_RES_T CSL_SPI_VC4L_Close(CSL_LCD_HANDLE smiH)
{
	CSL_LCD_RES_T res = CSL_LCD_OK;
	pSMI_SPI_HANDLE pSpi = (pSMI_SPI_HANDLE) smiH;

	if (pSpi->open) {
		pSpi->open = FALSE;
		LCD_DBG(LCD_DBG_INIT_ID, "[CSL SPI VC4L] %s: "
			"Closing SPI Controller Handle.\n\r", __FUNCTION__);
		cslSmiSpiDrv.instance--;

		if (cslSmiSpiDrv.activeHandle == pSpi)
			cslSmiSpiDrv.activeHandle = NULL;
	} else {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL SPI VC4L] %s: "
			"WARNING SPI Handle Not Open!\n\r", __FUNCTION__);
	}
	return (res);
}

//*****************************************************************************
//                 
// Function Name:  CSL_SPI_VC4L_Open
// 
// Description:    Init CSL SPI Driver
//
//*****************************************************************************
CSL_LCD_RES_T CSL_SPI_VC4L_Open(pCSL_SPI_CFG ctrlInit, CSL_LCD_HANDLE *spiH)
{

	pSMI_SPI_HANDLE pSpi = NULL;
	CSL_LCD_RES_T res = CSL_LCD_OK;
	UInt32 i;
	Boolean spiFound = FALSE;

	if (!cslSmiSpiDrv.init) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL SPI VC4L] %s: "
			"ERR SMI/SPI CSL Not Init!\n\r", __FUNCTION__);
		*spiH = NULL;
		return (CSL_LCD_NOT_INIT);
	}

	if (ctrlInit->mode != MM_SPI_MODE_LOSSI) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL SPI VC4L] %s: "
			"ERR Only LOSSI Mode Is Supported!\n\r", __FUNCTION__);
		*spiH = NULL;
		return (CSL_LCD_BUS_CFG);
	}

	if (ctrlInit->csBank != 0) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL SPI VC4L] %s: "
			"ERR csBank != 0 !\n\r", __FUNCTION__);
		*spiH = NULL;
		return (CSL_LCD_BUS_CFG);
	}

	if (cslSmiSpiDrv.instance >= SMI_SPI_MAX_HANDLES) {
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL SPI VC4L] %s: "
			"ERR Instance Count!\n\r", __FUNCTION__);
		*spiH = NULL;
		return (CSL_LCD_INST_COUNT);
	}

	for (i = 0; i < SMI_SPI_MAX_HANDLES; i++) {
		if (cslSmiSpiDrv.ctrl[i].open &&
		    (cslSmiSpiDrv.ctrl[i].ctrlType == CTRL_SPI)) {
			LCD_DBG(LCD_DBG_ERR_ID, "[CSL SPI VC4L] %s: "
				"Only Single SPI Instance Allowed!\n\r",
				__FUNCTION__);
			spiFound = TRUE;
			break;
		}
	}

	if (!spiFound) {
		for (i = 0; i < SMI_SPI_MAX_HANDLES; i++) {
			if (!cslSmiSpiDrv.ctrl[i].open) {
				pSpi = &cslSmiSpiDrv.ctrl[i];
				break;
			}
		}
	}

	if (pSpi == NULL) {
		*spiH = NULL;
		return (CSL_LCD_INST_COUNT);
	}

	pSpi->chalH = chal_spivc4l_init(MM_SPI_BASE_ADDR);
//    pSpi->chalH = chal_spivc4l_init ( chal_sysmap_get_base_addr( MM_SPI ) );

	pSpi->ctrlType = CTRL_SPI;
	pSpi->open = TRUE;

	cslSpiEnablePads();

	cslSmiSpiDrv.instance++;
	cslSmiSpiDrv.activeHandle = pSpi;

	*spiH = (CSL_LCD_HANDLE)pSpi;

	LCD_DBG(LCD_DBG_INIT_ID, "[CSL SPI VC4L] %s: OK\n\r", __FUNCTION__);

	return (res);
}

//*****************************************************************************
//                 
// Function Name:  CSL_SPI_VC4L_LossiRead
// 
// Description:    Sends LoSSI read command and collect read reply.
//                 Controller has LoSSI Rd Commands hardcoded and will 
//                 auto-issue number of required RD clocks to do a READ. 
//
//                 Make sure RxBuff is large enough for the RD command !
//
//                 08-bit(1B) <- 0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0xda,0xdb,0xdc
//                 24-bit(3B) <- 0x04
//                 32-bit(4B) <- 0x09
//*****************************************************************************
CSL_LCD_RES_T CSL_SPI_VC4L_LossiRead(CSL_LCD_HANDLE spiH, pSPI_RD_ACC pSpiRdAcc)
{
	pSMI_SPI_HANDLE pSpi = (pSMI_SPI_HANDLE) spiH;
	UInt32 i = 0;
	UInt32 spiCsReg;
	UInt32 rdLenBytes;

#if defined( __SMI_SPI_ENABLE_TRACE_MSG__ )
	LCD_DBG(LCD_DBG_ID, "[CSL SPI VC4L] %s: "
		"RD Command [0x%02x]!\n\r", __FUNCTION__, pSpiRdAcc->rdCmnd);
#endif

	switch (pSpiRdAcc->rdCmnd) {
	case 0x0a:
	case 0x0b:
	case 0x0c:
	case 0x0d:
	case 0x0e:
	case 0x0f:
	case 0xda:
	case 0xdb:
	case 0xdc:
		rdLenBytes = 1;
		break;
	case 0x04:
		rdLenBytes = 3;
		break;
	case 0x09:
		rdLenBytes = 4;
		break;
	default:
		LCD_DBG(LCD_DBG_ERR_ID, "[CSL SPI VC4L] %s: "
			"Unknown LoSSI RD Command [0x%02x]!\n\r",
			__FUNCTION__, pSpiRdAcc->rdCmnd);
		return (CSL_LCD_ERR);
	}

	if (!pSpi->hasLock)
		OSSEMAPHORE_Obtain(smiSema, TICKS_FOREVER);

	if (pSpiRdAcc->spiDiv != 0) {
		if (chal_spivc4l_set_clk_div(pSpi->chalH, pSpiRdAcc->spiDiv) ==
		    -1) {
			if (!pSpi->hasLock)
				OSSEMAPHORE_Release(smiSema);
			LCD_DBG(LCD_DBG_ERR_ID, "[CSL SPI VC4L] %s: "
				"ERR Invaild SPIDIV Value[%d]!\n\r",
				__FUNCTION__, pSpiRdAcc->spiDiv);
			return CSL_LCD_BUS_CFG;
		}
	}

	cslSmiSpiSwitchHandle(spiH);

	spiCsReg = CHAL_SPIVC4L_CS_LEN	// LoSSI enable
	    | CHAL_SPIVC4L_CS_TA	// Transfer Active
	    | CHAL_SPIVC4L_CS_CPOL	// Clock Polarity
	    | CHAL_SPIVC4L_CS_CPHA	// Clock Phase
	    | CHAL_SPIVC4L_CS_CLEAR;	// Clear TX&RX FIFOs

	chal_spivc4l_set_cs(pSpi->chalH, spiCsReg);

	chal_spivc4l_wr_fifo(pSpi->chalH, pSpiRdAcc->rdCmnd);

	CSL_SPI_POLL4DONE(pSpi->chalH);

	while (i < rdLenBytes) {
		CSL_SPI_POLL4RXFIFO(pSpi->chalH);
		pSpiRdAcc->pRxBuff[i++] = chal_spivc4l_rd_fifo(pSpi->chalH);
	}

	CHAL_SPIVC4L_DIS_TA(pSpi->chalH);

	if (!pSpi->hasLock)
		OSSEMAPHORE_Release(smiSema);

	return (CSL_LCD_OK);
}

//*****************************************************************************
//                 
// Function Name:  CSL_SPI_VC4L_LossiWrite
// 
// Description:    Send SPI Lossi Data Block using DMA or nonDMA mode
//
//                 SPI TX Buffer Content:
//                 - LSB of 32-bit DATA WORD is data byte while 9-th bit 
//                   controls interface data type CMND(0) or DATA(1)
//
// o 16 WORD (32bit) FIFOs ( TX & RX )
// o works in 'manual' DMA_LEN reg mode in order to avoid 
//   configuration of the interface from first word of DMA buff
//
// CURRENT ISSUES/FACTs
//   o JIRA bug : Lossi DONE is not set in DMA mode ( fixed in RHEA )
//     Work-arround: 
//       - upon DMA end, clear DMA enable control bit, than poll for DONE
//   o HERA DMA works only in waitResponse == 1 mode, TE sync issues 
//       - RHEA has fix for the both issues
//       - HERA TE sw workarround in place ( by using 'manual' DMA_LEN )
//       - Even with waitResponse == 1 DMA mode HERA functionality is ?
//
//*****************************************************************************

CSL_LCD_RES_T CSL_SPI_VC4L_LossiWrite(CSL_LCD_HANDLE spiH,
				      pSPI_WR_ACC pSpiWrAcc)
{
	UInt32 spiCsReg;
	SMI_DMA_REQ_t dmaReq;
	DMA_VC4LITE_CHANNEL_t dmaCh;

	OSStatus_t status;
	CSL_LCD_RES_T res = CSL_LCD_OK;
	pSMI_SPI_HANDLE pSpi = (pSMI_SPI_HANDLE) spiH;
	SMI_UPD_REQ_MSG_T updMsg;
	UInt32 i = 0;
	UInt32 accLen;
	UInt32 transferSize;

	if (!pSpi->hasLock)
		OSSEMAPHORE_Obtain(smiSema, TICKS_FOREVER);

	if (pSpiWrAcc->spiDiv != 0) {
		if (chal_spivc4l_set_clk_div(pSpi->chalH, pSpiWrAcc->spiDiv) ==
		    -1) {
			if (!pSpi->hasLock)
				OSSEMAPHORE_Release(smiSema);

			LCD_DBG(LCD_DBG_ERR_ID, "[CSL SPI VC4L] %s: "
				"ERR Invaild SPIDIV Value[%d]!\n\r",
				__FUNCTION__, pSpiWrAcc->spiDiv);
			return CSL_LCD_BUS_CFG;
		}
	}

	cslSmiSpiSwitchHandle(spiH);

	if (!pSpiWrAcc->isDma) {
#if defined( __SMI_SPI_ENABLE_TRACE_MSG__ )
		LCD_DBG(LCD_DBG_ID, "[CSL SPI VC4L] %s: "
			"NON-DMA Blocking ...\r\n", __FUNCTION__);
#endif
		spiCsReg = CHAL_SPIVC4L_CS_LEN	// LoSSI enable
		    | CHAL_SPIVC4L_CS_TA	// Transfer Active
		    | CHAL_SPIVC4L_CS_CPOL	// Clock Polarity
		    | CHAL_SPIVC4L_CS_CPHA	// Clock Phase
		    | CHAL_SPIVC4L_CS_CLEAR;	// Clear TX&RX FIFOs

		if (pSpiWrAcc->isTE)
			spiCsReg |= CHAL_SPIVC4L_CS_TE;

		chal_spivc4l_set_cs(pSpi->chalH, spiCsReg);

		accLen = (pSpiWrAcc->xLenInBytes >> 2) * pSpiWrAcc->yLen;
		while (i < accLen) {
			CSL_SPI_POLL4TXFIFO(pSpi->chalH);
			chal_spivc4l_wr_fifo(pSpi->chalH,
					     pSpiWrAcc->pBuff[i++]);
		}

		CSL_SPI_POLL4DONE(pSpi->chalH);

		CHAL_SPIVC4L_DIS_TA(pSpi->chalH);

		if (!pSpi->hasLock)
			OSSEMAPHORE_Release(smiSema);
	} else {
		// DMA request 
		dmaReq.xLenBytes = pSpiWrAcc->xLenInBytes;
		dmaReq.xStrideBytes = 0;
		dmaReq.yLen = pSpiWrAcc->yLen;
		dmaReq.isWrite = TRUE;
		dmaReq.pBuff = pSpiWrAcc->pBuff;

		transferSize = (pSpiWrAcc->xLenInBytes >> 2) * pSpiWrAcc->yLen;

		// start DMA
		if ((res = cslSmiSpiDmaStart(pSpi, &dmaReq, &dmaCh))
		    != CSL_LCD_OK) {
			if (!pSpi->hasLock)
				OSSEMAPHORE_Release(smiSema);
			return (res);
		}
		// Clear TX&RX FIFOs
		chal_spivc4l_set_cs(pSpi->chalH, CHAL_SPIVC4L_CS_CLEAR);
		while (chal_spivc4l_get_cs(pSpi->chalH) &
		       CHAL_SPIVC4L_CS_CLEAR) ;

		spiCsReg = CHAL_SPIVC4L_CS_LEN	// LoSSI enable
		    | CHAL_SPIVC4L_CS_ADCS	// Auto DMA End CS deassert
		    | CHAL_SPIVC4L_CS_TA	// Transfer Active
		    | CHAL_SPIVC4L_CS_CPOL	// Clock Polarity
		    | CHAL_SPIVC4L_CS_CPHA;	// Clock Phase

		chal_spivc4l_set_dma_len(pSpi->chalH, transferSize);

		if (pSpiWrAcc->isTE)
			spiCsReg |= CHAL_SPIVC4L_CS_TE;

		chal_spivc4l_set_cs(pSpi->chalH, spiCsReg);
		CHAL_SPIVC4L_ENA_DMA(pSpi->chalH);

		if (pSpiWrAcc->cslLcdCb == NULL) {
#if defined( __SMI_SPI_ENABLE_TRACE_MSG__ )
			LCD_DBG(LCD_DBG_ID, "[CSL SPI VC4L] %s: "
				"DMA Blocking ...\r\n", __FUNCTION__);
#endif
			status = OSSEMAPHORE_Obtain(eofDmaSema,
						    TICKS_IN_MILLISECONDS
						    (pSpiWrAcc->timeOutMsec));

			if (status != OSSTATUS_SUCCESS) {
				if (status == OSSTATUS_TIMEOUT) {
					LCD_DBG(LCD_DBG_ERR_ID,
						"[CSL SPI VC4L] %s: "
						"ERR EOFDMA Timeout!\n\r",
						__FUNCTION__);
					res = CSL_LCD_OS_TOUT;
				} else {
					LCD_DBG(LCD_DBG_ERR_ID,
						"[CSL SPI VC4L] %s: "
						"ERR EOFDMA OS Error!\n\r",
						__FUNCTION__);
					res = CSL_LCD_OS_ERR;
				}
			}

			if (res == CSL_LCD_OK) {
#if ( !defined(_RHEA_) )
				// LOSSI-DONE-NOT-SET workarround 
				CHAL_SPIVC4L_DIS_DMA(pSpi->chalH);
#endif
				CSL_SPI_POLL4DONE(pSpi->chalH);

#if defined( __SMI_SPI_ENABLE_TRACE_MSG__ )
				LCD_DBG(LCD_DBG_ID, "[CSL SPI VC4L] %s: "
					"DONE ... OK \n\r", __FUNCTION__);
#endif
			} else {
				LCD_DBG(LCD_DBG_INIT_ID, "[CSL SPI VC4L] %s: "
					"Stopping DMA Ch[%d]\n\r", __FUNCTION__,
					dmaCh);
				cslSmiSpiDmaStop(dmaCh);
			}
			chal_spivc4l_set_cs(pSpi->chalH, 0);

			if (!pSpi->hasLock)
				OSSEMAPHORE_Release(smiSema);
		} else {
			updMsg.smiH = pSpi;
			updMsg.dmaCh = dmaCh;
			updMsg.timeOutMsec = pSpiWrAcc->timeOutMsec;
			updMsg.cslLcdCb = pSpiWrAcc->cslLcdCb;
			updMsg.cslLcdCbRec = pSpiWrAcc->cslLcdCbRec;

#if defined( __SMI_SPI_ENABLE_TRACE_MSG__ )
			LCD_DBG(LCD_DBG_ID, "[CSL SPI VC4L] %s: "
				"DMA Non-Blocking ...\n\r", __FUNCTION__);
#endif
			status =
			    OSQUEUE_Post(updReqQ, (QMsg_t *)&updMsg,
					 TICKS_NO_WAIT);
			if (status != OSSTATUS_SUCCESS) {
				res = CSL_LCD_OS_ERR;
				LCD_DBG(LCD_DBG_ERR_ID, "[CSL SPI VC4L] %s: "
					"ERR Posting Msg \n\r", __FUNCTION__);
			}
		}
	}

	return (res);
}				// CSL_SPI_VC4L_LossiWrite
