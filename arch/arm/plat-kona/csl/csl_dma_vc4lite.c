/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*             @file     arch/arm/plat-kona/csl/csl_dma_vc4lite.c
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
*  @file   csl_dma_vc4lite.c
*
*  @brief  VC4Lite DMA CSL implementation
*
*  @note   
****************************************************************************/
#define UNDER_LINUX

#include "linux/broadcom/mobcom_types.h"
#include "plat/chal/chal_types.h"
#include "plat/osabstract/osinterrupt.h"
#include "plat/osabstract/ossemaphore.h"
#include "mach/rdb/brcm_rdb_sysmap.h"
#include "plat/csl/csl_dma_vc4lite.h"
#include "plat/chal/chal_dma_vc4lite.h"
#include "plat/osdal_os.h"
#include "linux/dma-mapping.h"

//******************************************************************************
// Local Definitions
//******************************************************************************
#define DMA_VC4LITE_BASE_ADDR                     MM_DMA_BASE_ADDR
#define DMA_VC4LITE_STACK_SIZE                    (1024)
#define DMA_VC4LITE_CHANNEL_CTRL_BLOCK_SIZE       (1600)

#ifdef UNDER_LINUX
#define DEBUG
static UInt32 *dmaCtrlBlkList = NULL;	// Virtual address of DMA memory region
static UInt32 *dmaCtrlBlkListPhys = NULL;
#else
#pragma arm section zidata="uncacheable"
__align(32)
static UInt32 dmaCtrlBlkList[DMA_VC4LITE_CHANNEL_CTRL_BLOCK_SIZE *
			     DMA_VC4LITE_TOTAL_CHANNELS] = { 0 };
#pragma arm section
#endif

typedef struct {
	Semaphore_t chanSema;
	UInt8 used;
	UInt8 irqStatus;
	UInt32 chanState;
	UInt32 *pDmaChanCtrlBlkList;	// Virtual
#ifdef UNDER_LINUX
	UInt32 *pDmaChanCtrlBlkListPHYS;	// Physical
#endif
	UInt32 dmaChanCtrlBlkMemSize;
	UInt32 dmaChanCtrlBlkItemNum;
	DMA_VC4LITE_CHANNEL_INFO_t chanInfo;
} CslDmaVc4liteChan_t;

typedef struct {
	int initialized;
	UInt32 base;
	Interrupt_t hisr;
	CHAL_HANDLE handle;
	Semaphore_t dmaSema;
	CslDmaVc4liteChan_t chan[DMA_VC4LITE_TOTAL_CHANNELS];
} CslDmaVc4lite_t;
static CslDmaVc4lite_t dmac = { 0 };

//******************************************************************************
// Local Function Definition
//******************************************************************************
#ifndef UNDER_LINUX
static void dma_vc4lite_lisr(void);
#else
static irqreturn_t bcm_vc4l_dma_interrupt(int irq, void *dev_id);
#endif
static void dma_vc4lite_hisr(void);
static UInt8 dma_vc4lite_per_map(UInt8);

#define printk(fmt, ...) do {} while (0)

//******************************************************************************
// Function Definition
//******************************************************************************

//******************************************************************************
//
//  Funciton Name:      csl_dma_vc4lite_init
//  
//  Description:        Initialize VC4Lite DMA hardware and software interface
//
//******************************************************************************
DMA_VC4LITE_STATUS_t csl_dma_vc4lite_init(void)
{
	CslDmaVc4lite_t *pdma = (CslDmaVc4lite_t *)&dmac;
	int i;
	char name[16];

	if (!pdma->initialized) {
#ifdef UNDER_LINUX
		// Allocate dma memory

		dmaCtrlBlkList =
		    kmalloc(DMA_VC4LITE_CHANNEL_CTRL_BLOCK_SIZE *
			    DMA_VC4LITE_TOTAL_CHANNELS, GFP_KERNEL);
#if 0
		dmaCtrlBlkList = dma_alloc_coherent(NULL,
						    DMA_VC4LITE_CHANNEL_CTRL_BLOCK_SIZE
						    *
						    DMA_VC4LITE_TOTAL_CHANNELS,
						    (dma_addr_t *)
						    &dmaCtrlBlkListPhys,
						    GFP_KERNEL);
#endif
		if ((void *)dmaCtrlBlkList == NULL) {
			pr_info("DMA driver: failed to allocate DMA memory\n");
			return -ENOMEM;
		}
		dmaCtrlBlkListPhys = (UInt32 *)virt_to_phys(dmaCtrlBlkList);

		printk(KERN_ERR
		       "the virt addr=0x%08x phy addr=0x%08x acp addr=%08x\n",
		       dmaCtrlBlkList, virt_to_phys(dmaCtrlBlkList),
		       dmaCtrlBlkListPhys);
#endif

#ifdef UNDER_LINUX
		pdma->base = HW_IO_PHYS_TO_VIRT(DMA_VC4LITE_BASE_ADDR);
#else
		pdma->base = DMA_VC4LITE_BASE_ADDR;
#endif
		pdma->handle = (CHAL_HANDLE)chal_dma_vc4lite_init(pdma->base);
		if (pdma->handle == NULL) {
			dprintf(1, "%s: Invalid dmac handle\n", __FUNCTION__);
			return DMA_VC4LITE_STATUS_FAILURE;
		}

		pdma->dmaSema = OSSEMAPHORE_Create(1, OSSUSPEND_PRIORITY);
		if (!pdma->dmaSema) {
			dprintf(1, "%s: semaphore creation failed\n",
				__FUNCTION__);
			return DMA_VC4LITE_STATUS_FAILURE;
		}
		OSSEMAPHORE_ChangeName(pdma->dmaSema, "dmasema");

		for (i = 0; i < DMA_VC4LITE_TOTAL_CHANNELS; i++) {
			// create semaphore for each channel
			pdma->chan[i].chanSema =
			    OSSEMAPHORE_Create(1, OSSUSPEND_PRIORITY);
			if (!pdma->chan[i].chanSema) {
				dprintf(1,
					"%s: channel semaphore %d creation failed\n",
					__FUNCTION__, i);
				return DMA_VC4LITE_STATUS_FAILURE;
			}
			sprintf(name, "DmaVC4ch%02d", i);
			OSSEMAPHORE_ChangeName(pdma->chan[i].chanSema, name);

			// set the channel control block address
			pdma->chan[i].pDmaChanCtrlBlkList =
			    (UInt32 *)((UInt32)dmaCtrlBlkList +
				       i * DMA_VC4LITE_CHANNEL_CTRL_BLOCK_SIZE);
#ifdef UNDER_LINUX
			pdma->chan[i].pDmaChanCtrlBlkListPHYS =
			    (UInt32 *)((UInt32)dmaCtrlBlkListPhys +
				       i * DMA_VC4LITE_CHANNEL_CTRL_BLOCK_SIZE);
#endif
			pdma->chan[i].dmaChanCtrlBlkMemSize =
			    DMA_VC4LITE_CHANNEL_CTRL_BLOCK_SIZE;
			pdma->chan[i].dmaChanCtrlBlkItemNum = 0;
		}

		pdma->hisr = OSINTERRUPT_Create((IEntry_t)&dma_vc4lite_hisr,
						"VC4LITEDMAHISR",
						IPRIORITY_MIDDLE,
						DMA_VC4LITE_STACK_SIZE);
		if (!pdma->hisr) {
			dprintf(1, "%s: hisr creation failed\n", __FUNCTION__);
			return DMA_VC4LITE_STATUS_FAILURE;
		}
#ifndef UNDER_LINUX
		// enable all the dma channel interrupt
		IRQ_Register(MM_DMA_CHAN1_IRQ, dma_vc4lite_lisr);
		IRQ_Enable(MM_DMA_CHAN1_IRQ);

		IRQ_Register(MM_DMA_CHAN2_IRQ, dma_vc4lite_lisr);
		IRQ_Enable(MM_DMA_CHAN2_IRQ);

		IRQ_Register(MM_DMA_CHAN3_IRQ, dma_vc4lite_lisr);
		IRQ_Enable(MM_DMA_CHAN3_IRQ);
#else
		if (request_irq
		    (BCM_INT_ID_MM_DMA_CHAN1, bcm_vc4l_dma_interrupt,
		     IRQF_DISABLED | IRQF_NO_SUSPEND, "BRCM VC4L DMA1",
		     NULL) < 0) {
			pr_err("%s(%s:%u)::request_irq failed IRQ %d\n",
			       __FUNCTION__, __FILE__, __LINE__,
			       BCM_INT_ID_MM_DMA_CHAN1);
			return DMA_VC4LITE_STATUS_FAILURE;
		}
#endif
		pdma->initialized = 1;
	}

	return DMA_VC4LITE_STATUS_SUCCESS;
}

//******************************************************************************
//
//  Funciton Name:      csl_dma_vc4lite_deinit
//  
//  Description:        Deinitialize DMA hardware and software interface
//
//******************************************************************************
DMA_VC4LITE_STATUS_t csl_dma_vc4lite_deinit(void)
{
	CslDmaVc4lite_t *pdma = (CslDmaVc4lite_t *)&dmac;
	int i;

	if (!pdma->initialized) {
		dprintf(1, "%s: vc4lite dmac has not been initialized\n",
			__FUNCTION__);
		return DMA_VC4LITE_STATUS_SUCCESS;
	}

	if (pdma->hisr) {
		OSINTERRUPT_Destroy(pdma->hisr);
		pdma->hisr = NULL;
	}

	if (pdma->dmaSema) {
		OSSEMAPHORE_Destroy(pdma->dmaSema);
		pdma->dmaSema = NULL;
	}

	for (i = 0; i < DMA_VC4LITE_TOTAL_CHANNELS; i++) {
		if (pdma->chan[i].chanSema) {
			OSSEMAPHORE_Destroy(pdma->chan[i].chanSema);
			pdma->chan[i].chanSema = NULL;
		}
		memset(&pdma->chan[i], 0, sizeof(CslDmaVc4liteChan_t));
	}

	pdma->handle = NULL;
	pdma->initialized = 0;
	pdma->base = 0;

	return DMA_VC4LITE_STATUS_SUCCESS;
}

//******************************************************************************
//
//  Funciton Name:      csl_dma_vc4lite_config_channel
//  
//  Description:        Configure channel structure
//
//******************************************************************************
DMA_VC4LITE_STATUS csl_dma_vc4lite_config_channel(DMA_VC4LITE_CHANNEL_t chanID,
						  DMA_VC4LITE_CHANNEL_INFO_t *
						  pChanInfo)
{
	CslDmaVc4lite_t *pdma = (CslDmaVc4lite_t *)&dmac;

	// sanity check
	if (!pdma->initialized) {
		dprintf(1, "%s: vc4lite dmac has not been initialized\n",
			__FUNCTION__);
		return DMA_VC4LITE_STATUS_FAILURE;
	}

	if ((chanID >= DMA_VC4LITE_TOTAL_CHANNELS) || (!pChanInfo)) {
		dprintf(1, "%s: invalid input parameters\n", __FUNCTION__);
		return DMA_VC4LITE_STATUS_INVALID_INPUT;
	}

	if (!pdma->chan[chanID].used) {
		/* The channel is allocate statically for B0 workaround. */
		pdma->chan[chanID].used = 1;
	}

	if (((pChanInfo->srcID != DMA_VC4LITE_CLIENT_MEMORY) &&
	     (pChanInfo->srcID != DMA_VC4LITE_CLIENT_DSI0) &&
	     (pChanInfo->srcID != DMA_VC4LITE_CLIENT_DSI1) &&
	     (pChanInfo->srcID != DMA_VC4LITE_CLIENT_SMI) &&
	     (pChanInfo->srcID != DMA_VC4LITE_CLIENT_SPI_TX) &&
	     (pChanInfo->srcID != DMA_VC4LITE_CLIENT_SPI_TX)) ||
	    ((pChanInfo->dstID != DMA_VC4LITE_CLIENT_MEMORY) &&
	     (pChanInfo->dstID != DMA_VC4LITE_CLIENT_DSI0) &&
	     (pChanInfo->dstID != DMA_VC4LITE_CLIENT_DSI1) &&
	     (pChanInfo->dstID != DMA_VC4LITE_CLIENT_SMI) &&
	     (pChanInfo->dstID != DMA_VC4LITE_CLIENT_SPI_TX) &&
	     (pChanInfo->dstID != DMA_VC4LITE_CLIENT_SPI_TX))) {
		dprintf(1, "%s: invalid input parameters\n", __FUNCTION__);
		return DMA_VC4LITE_STATUS_INVALID_INPUT;
	}

	if (!pChanInfo->callback) {
		dprintf(1, "%s: invalid input parameters\n", __FUNCTION__);
		return DMA_VC4LITE_STATUS_INVALID_INPUT;
	}

	OSSEMAPHORE_Obtain(pdma->dmaSema, TICKS_FOREVER);
	memcpy(&pdma->chan[chanID].chanInfo, pChanInfo,
	       sizeof(DMA_VC4LITE_CHANNEL_INFO_t));
	OSSEMAPHORE_Release(pdma->dmaSema);

	return DMA_VC4LITE_STATUS_SUCCESS;
}

//******************************************************************************
//
//  Funciton Name:      csl_dma_vc4lite_obtain_channel
//  
//  Description:        Obtain a free DMA channel
//
//******************************************************************************
Int32 csl_dma_vc4lite_obtain_channel(UInt8 srcID, UInt8 dstID)
{
	CslDmaVc4lite_t *pdma = (CslDmaVc4lite_t *)&dmac;
	UInt8 i;
	int chan = -1;

	// sanity check
	if (!pdma->initialized) {
		dprintf(1, "%s: vc4lite dmac has not been initialized\n",
			__FUNCTION__);
		return -1;
	}

	if (((srcID != DMA_VC4LITE_CLIENT_MEMORY) &&
	     (srcID != DMA_VC4LITE_CLIENT_DSI0) &&
	     (srcID != DMA_VC4LITE_CLIENT_DSI1) &&
	     (srcID != DMA_VC4LITE_CLIENT_SMI) &&
	     (srcID != DMA_VC4LITE_CLIENT_SPI_TX) &&
	     (srcID != DMA_VC4LITE_CLIENT_SPI_TX)) ||
	    ((dstID != DMA_VC4LITE_CLIENT_MEMORY) &&
	     (dstID != DMA_VC4LITE_CLIENT_DSI0) &&
	     (dstID != DMA_VC4LITE_CLIENT_DSI1) &&
	     (dstID != DMA_VC4LITE_CLIENT_SMI) &&
	     (dstID != DMA_VC4LITE_CLIENT_SPI_TX) &&
	     (dstID != DMA_VC4LITE_CLIENT_SPI_TX))) {
		dprintf(1, "%s: invalid input parameters\n", __FUNCTION__);
		return -1;
	}
	// check the available channel
	OSSEMAPHORE_Obtain(pdma->dmaSema, TICKS_FOREVER);

	for (i = 0; i < DMA_VC4LITE_TOTAL_CHANNELS; i++) {
		if (!pdma->chan[i].used) {
			chan = i;
			pdma->chan[i].used = TRUE;
			pdma->chan[i].dmaChanCtrlBlkItemNum = 0;
			break;
		}
	}

	OSSEMAPHORE_Release(pdma->dmaSema);

	return chan;
}

//******************************************************************************
//
//  Funciton Name:      csl_dma_vc4lite_release_channel
//  
//  Description:        Release the previously allocated channel
//
//******************************************************************************
DMA_VC4LITE_STATUS csl_dma_vc4lite_release_channel(DMA_VC4LITE_CHANNEL_t chanID)
{
	CslDmaVc4lite_t *pdma = (CslDmaVc4lite_t *)&dmac;

	// sanity check
	if (!pdma->initialized) {
		dprintf(1, "%s: vc4lite dmac has not been initialized\n",
			__FUNCTION__);
		return DMA_VC4LITE_STATUS_FAILURE;
	}

	if (chanID >= DMA_VC4LITE_TOTAL_CHANNELS) {
		dprintf(1, "%s: invalid input parameters\n", __FUNCTION__);
		return DMA_VC4LITE_STATUS_INVALID_INPUT;
	}

	OSSEMAPHORE_Obtain(pdma->dmaSema, TICKS_FOREVER);

	if (pdma->chan[chanID].used) {
		if (chal_dma_vc4lite_reset_channel(pdma->handle, chanID) !=
		    CHAL_DMA_VC4LITE_STATUS_SUCCESS) {
			dprintf(1, "%s: shutdown DMA channel error\n",
				__FUNCTION__);
			return DMA_VC4LITE_STATUS_FAILURE;
		}

		pdma->chan[chanID].used = FALSE;
		pdma->chan[chanID].dmaChanCtrlBlkItemNum = 0;
		memset(&pdma->chan[chanID].chanInfo, 0,
		       sizeof(DMA_VC4LITE_CHANNEL_INFO_t));
	}

	OSSEMAPHORE_Release(pdma->dmaSema);

	return DMA_VC4LITE_STATUS_SUCCESS;
}

void csl_dma_poll_int(int chanID)
{
	CslDmaVc4lite_t *pdma = (CslDmaVc4lite_t *)&dmac;

	int chanNum = chanID;

	do {
		printk(KERN_ERR "chan %d CS reg is 0x%08x\n", chanNum,
		       readl(HW_IO_PHYS_TO_VIRT(DMA_VC4LITE_BASE_ADDR) +
			     chanNum * 0x100));
		chal_dma_vc4lite_get_int_status(pdma->handle, chanNum,
						&pdma->chan[chanNum].irqStatus);
	} while (pdma->chan[chanNum].irqStatus == 0);
	printk(KERN_ERR "chan %d CS reg is 0x%08x\n", chanNum,
	       readl(HW_IO_PHYS_TO_VIRT(DMA_VC4LITE_BASE_ADDR) +
		     chanNum * 0x100));

	chal_dma_vc4lite_clear_int_status(pdma->handle, chanNum);
	pdma->chan[chanNum].dmaChanCtrlBlkItemNum = 0;
	dma_unmap_single(NULL,
			 (dma_addr_t)pdma->chan[chanNum].
			 pDmaChanCtrlBlkListPHYS,
			 DMA_VC4LITE_CHANNEL_CTRL_BLOCK_SIZE, DMA_TO_DEVICE);

	if (pdma->chan[chanNum].chanInfo.autoFreeChan) {
		if (chal_dma_vc4lite_reset_channel(pdma->handle, chanNum) !=
		    CHAL_DMA_VC4LITE_STATUS_SUCCESS) {
			dprintf(1, "%s: reset DMA channel error\n",
				__FUNCTION__);
		}

		pdma->chan[chanNum].used = FALSE;
		pdma->chan[chanNum].dmaChanCtrlBlkItemNum = 0;
		memset(&pdma->chan[chanNum].chanInfo, 0,
		       sizeof(DMA_VC4LITE_CHANNEL_INFO_t));
	}

}

//******************************************************************************
//
//  Funciton Name:      csl_dma_vc4lite_start_transfer
//  
//  Description:        Start DMA channel transfer
//
//******************************************************************************
DMA_VC4LITE_STATUS csl_dma_vc4lite_start_transfer(DMA_VC4LITE_CHANNEL_t chanID)
{
	CslDmaVc4lite_t *pdma = (CslDmaVc4lite_t *)&dmac;
	dma_addr_t temp;

	// sanity check
	if (!pdma->initialized) {
		dprintf(1, "%s: vc4lite dmac has not been initialized\n",
			__FUNCTION__);
		return DMA_VC4LITE_STATUS_FAILURE;
	}

	if (!pdma->chan[chanID].used) {
		dprintf(1, "%s: obtain channel needs to be called first\n",
			__FUNCTION__);
		return DMA_VC4LITE_STATUS_FAILURE;
	}

	if (chal_dma_vc4lite_enable_int(pdma->handle, chanID) !=
	    CHAL_DMA_VC4LITE_STATUS_SUCCESS) {
		dprintf(1, "%s: enable channel interrupt failure\n",
			__FUNCTION__);
		return DMA_VC4LITE_STATUS_FAILURE;
	}

#ifdef UNDER_LINUX
	if (chal_dma_vc4lite_prepare_transfer
	    (pdma->handle, chanID,
	     (cVoid *)pdma->chan[chanID].pDmaChanCtrlBlkList,
	     (cVoid *)pdma->chan[chanID].pDmaChanCtrlBlkListPHYS)
#else
	if (chal_dma_vc4lite_prepare_transfer
	    (pdma->handle, chanID,
	     (cVoid *)pdma->chan[chanID].pDmaChanCtrlBlkList)
#endif
	    != CHAL_DMA_VC4LITE_STATUS_SUCCESS) {
		dprintf(1, "%s: prepare data transfer failure\n", __FUNCTION__);
		return DMA_VC4LITE_STATUS_FAILURE;
	}

	temp = dma_map_single(NULL,
			      pdma->chan[chanID].pDmaChanCtrlBlkList,
			      DMA_VC4LITE_CHANNEL_CTRL_BLOCK_SIZE,
			      DMA_TO_DEVICE);
	printk(KERN_ERR "the temp dma addr=0x%08x and the phys addr=0x%08x\n",
	       temp, pdma->chan[chanID].pDmaChanCtrlBlkListPHYS);

	if (chal_dma_vc4lite_enable_channel(pdma->handle, chanID) !=
	    CHAL_DMA_VC4LITE_STATUS_SUCCESS) {
		dprintf(1, "%s: start channel failure\n", __FUNCTION__);
		return DMA_VC4LITE_STATUS_FAILURE;
	}

	return DMA_VC4LITE_STATUS_SUCCESS;
}

//******************************************************************************
//
//  Funciton Name:      csl_dma_vc4lite_stop_transfer
//  
//  Description:        Stop DMA channel transfer
//
//******************************************************************************
DMA_VC4LITE_STATUS csl_dma_vc4lite_stop_transfer(DMA_VC4LITE_CHANNEL_t chanID)
{
	CslDmaVc4lite_t *pdma = (CslDmaVc4lite_t *)&dmac;

	// sanity check
	if (!pdma->initialized) {
		dprintf(1, "%s: vc4lite dmac has not been initialized\n",
			__FUNCTION__);
		return DMA_VC4LITE_STATUS_FAILURE;
	}

	if (chal_dma_vc4lite_abort_transfer(pdma->handle, chanID) !=
	    CHAL_DMA_VC4LITE_STATUS_SUCCESS) {
		dprintf(1, "%s: abort the current transfer failure\n",
			__FUNCTION__);
		return DMA_VC4LITE_STATUS_FAILURE;
	}

	if (chal_dma_vc4lite_reset_channel(pdma->handle, chanID) !=
	    CHAL_DMA_VC4LITE_STATUS_SUCCESS) {
		dprintf(1, "%s: reset channel failure\n", __FUNCTION__);
		return DMA_VC4LITE_STATUS_FAILURE;
	}

	return DMA_VC4LITE_STATUS_SUCCESS;
}

//******************************************************************************
//
//  Funciton Name:      csl_dma_vc4lite_add_data
//  
//  Description:        Build up link list for DMA transfer.
//
//******************************************************************************
DMA_VC4LITE_STATUS csl_dma_vc4lite_add_data(DMA_VC4LITE_CHANNEL_t chanID,
					    DMA_VC4LITE_XFER_DATA_t *pData)
{
	CslDmaVc4lite_t *pdma = (CslDmaVc4lite_t *)&dmac;
	ChalDmaCtrlBlkInfo_t dmaCtrlBlkInfo;

	// sanity check
	if (!pdma->initialized) {
		dprintf(1, "%s: vc4lite dmac has not been initialized\n",
			__FUNCTION__);
		return DMA_VC4LITE_STATUS_FAILURE;
	}

	if (chanID >= DMA_VC4LITE_TOTAL_CHANNELS) {
		dprintf(1, "%s: invalid input parameters\n", __FUNCTION__);
		return DMA_VC4LITE_STATUS_INVALID_INPUT;
	}

	if (!pdma->chan[chanID].used) {
		dprintf(1, "%s: obtain channel needs to be called first\n",
			__FUNCTION__);
		return DMA_VC4LITE_STATUS_FAILURE;
	}

	if (pdma->chan[chanID].chanInfo.xferMode == DMA_VC4LITE_XFER_MODE_2D) {
		dprintf(1, "%s: invalid input parameters\n", __FUNCTION__);
		return DMA_VC4LITE_STATUS_INVALID_INPUT;
	}
	// set the ctrl block information
	dmaCtrlBlkInfo.noWideBurst = 0;
	dmaCtrlBlkInfo.waitCycles = 0;
	dmaCtrlBlkInfo.srcIgnoreRead = 0;
	dmaCtrlBlkInfo.dstIgnoreWrite = 0;
	if (chanID == 1) {
		if ((pData->dstAddr & 0xff000000) == 0x34000000)
			dmaCtrlBlkInfo.dstIgnoreWrite = 1;
		if ((pData->srcAddr & 0xff000000) == 0x34000000)
			dmaCtrlBlkInfo.srcIgnoreRead = 0;
	}

	dmaCtrlBlkInfo.waitResponse = pdma->chan[chanID].chanInfo.waitResponse;

	dmaCtrlBlkInfo.srcDreqID =
	    dma_vc4lite_per_map(pdma->chan[chanID].chanInfo.srcID);
	dmaCtrlBlkInfo.dstDreqID =
	    dma_vc4lite_per_map(pdma->chan[chanID].chanInfo.dstID);
	dmaCtrlBlkInfo.burstLength = pdma->chan[chanID].chanInfo.burstLen;

	if (pdma->chan[chanID].chanInfo.srcID == DMA_VC4LITE_CLIENT_MEMORY)
		dmaCtrlBlkInfo.srcAddrIncrement = 1;
	else
		dmaCtrlBlkInfo.srcAddrIncrement = 0;

	if (pdma->chan[chanID].chanInfo.dstID == DMA_VC4LITE_CLIENT_MEMORY)
		dmaCtrlBlkInfo.dstAddrIncrement = 1;
	else
		dmaCtrlBlkInfo.dstAddrIncrement = 0;

	// only 32bit width supported for both source and destination 
	if (chanID == 0) {
		dmaCtrlBlkInfo.srcXferWidth = 0;
		dmaCtrlBlkInfo.dstXferWidth = 0;
	} else {
		if (pData->dstAddr & 0xc0000000) {
			dmaCtrlBlkInfo.srcXferWidth = 0;
			dmaCtrlBlkInfo.dstXferWidth = 1;
		} else {
			dmaCtrlBlkInfo.srcXferWidth = 0;
			dmaCtrlBlkInfo.dstXferWidth = 0;
		}
	}

	dmaCtrlBlkInfo.srcAddr = pData->srcAddr;
	dmaCtrlBlkInfo.dstAddr = pData->dstAddr;

	/* This shall fix the lock up issue. */
	dmaCtrlBlkInfo.xferMode = 0;

	dmaCtrlBlkInfo.srcStride = 0;
	dmaCtrlBlkInfo.dstStride = 0;
	dmaCtrlBlkInfo.xferLength.len = pData->xferLength;

	if (chal_dma_vc4lite_build_ctrlblk_list(pdma->handle,
						pdma->chan[chanID].
						pDmaChanCtrlBlkList,
						pdma->chan[chanID].
						dmaChanCtrlBlkItemNum,
						pdma->chan[chanID].
						dmaChanCtrlBlkMemSize,
						&dmaCtrlBlkInfo) !=
	    CHAL_DMA_VC4LITE_STATUS_SUCCESS) {
		dprintf(1, "%s: get control block list failure\n",
			__FUNCTION__);
		return DMA_VC4LITE_STATUS_FAILURE;
	}

	printk(KERN_ERR "ctl blk item num = %d, ctl blk info"
	       "0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n",
	       pdma->chan[chanID].dmaChanCtrlBlkItemNum,
	       ((unsigned int *)pdma->chan[chanID].pDmaChanCtrlBlkList)[0],
	       ((unsigned int *)pdma->chan[chanID].pDmaChanCtrlBlkList)[1],
	       ((unsigned int *)pdma->chan[chanID].pDmaChanCtrlBlkList)[2],
	       ((unsigned int *)pdma->chan[chanID].pDmaChanCtrlBlkList)[3],
	       ((unsigned int *)pdma->chan[chanID].pDmaChanCtrlBlkList)[4],
	       ((unsigned int *)pdma->chan[chanID].pDmaChanCtrlBlkList)[5]
	    );

	pdma->chan[chanID].dmaChanCtrlBlkItemNum++;
	return DMA_VC4LITE_STATUS_SUCCESS;
}

/**
*
*  This function add 2D data buffer for the DMA channel
*  @param		chanID (in) channel number
*  @param       pData     (in) pointer to dma channel data buffer 
*
*  @return	    status    (out) 0 or 1
*
*****************************************************************************/
DMA_VC4LITE_STATUS csl_dma_vc4lite_add_data_ex(DMA_VC4LITE_CHANNEL_t chanID,
					       DMA_VC4LITE_XFER_2DDATA_t *
					       pData)
{
	CslDmaVc4lite_t *pdma = (CslDmaVc4lite_t *)&dmac;
	ChalDmaCtrlBlkInfo_t dmaCtrlBlkInfo;

	// sanity check
	if (!pdma->initialized) {
		dprintf(1, "%s: vc4lite dmac has not been initialized\n",
			__FUNCTION__);
		return DMA_VC4LITE_STATUS_FAILURE;
	}

	if (chanID >= DMA_VC4LITE_TOTAL_CHANNELS) {
		dprintf(1, "%s: invalid input parameters\n", __FUNCTION__);
		return DMA_VC4LITE_STATUS_INVALID_INPUT;
	}

	if (!pdma->chan[chanID].used) {
		dprintf(1, "%s: obtain channel needs to be called first\n",
			__FUNCTION__);
		return DMA_VC4LITE_STATUS_FAILURE;
	}

	if (pdma->chan[chanID].chanInfo.xferMode ==
	    DMA_VC4LITE_XFER_MODE_LINERA) {
		dprintf(1, "%s: invalid input parameters\n", __FUNCTION__);
		return DMA_VC4LITE_STATUS_INVALID_INPUT;
	}
	// set the ctrl block information
	dmaCtrlBlkInfo.noWideBurst = 0;
	dmaCtrlBlkInfo.waitCycles = 0;
	dmaCtrlBlkInfo.srcIgnoreRead = 0;
	dmaCtrlBlkInfo.dstIgnoreWrite = 0;
	dmaCtrlBlkInfo.waitResponse = pdma->chan[chanID].chanInfo.waitResponse;

	dmaCtrlBlkInfo.srcDreqID =
	    dma_vc4lite_per_map(pdma->chan[chanID].chanInfo.srcID);
	dmaCtrlBlkInfo.dstDreqID =
	    dma_vc4lite_per_map(pdma->chan[chanID].chanInfo.dstID);

	dmaCtrlBlkInfo.burstLength = pdma->chan[chanID].chanInfo.burstLen;

	if (pdma->chan[chanID].chanInfo.srcID == DMA_VC4LITE_CLIENT_MEMORY)
		dmaCtrlBlkInfo.srcAddrIncrement = 1;
	else
		dmaCtrlBlkInfo.srcAddrIncrement = 0;

	if (pdma->chan[chanID].chanInfo.dstID == DMA_VC4LITE_CLIENT_MEMORY)
		dmaCtrlBlkInfo.dstAddrIncrement = 1;
	else
		dmaCtrlBlkInfo.dstAddrIncrement = 0;

	// only 32bit width supported for both source and destination
	dmaCtrlBlkInfo.srcXferWidth = 0;
	dmaCtrlBlkInfo.dstXferWidth = 0;

	dmaCtrlBlkInfo.srcAddr = (UInt32)pData->srcAddr;
	dmaCtrlBlkInfo.dstAddr = (UInt32)pData->dstAddr;

	dmaCtrlBlkInfo.xferMode = CHAL_DMA_VC4LITE_2D_MODE;

	dmaCtrlBlkInfo.srcStride = pdma->chan[chanID].chanInfo.srcStride;
	dmaCtrlBlkInfo.dstStride = pdma->chan[chanID].chanInfo.dstStride;
	dmaCtrlBlkInfo.xferLength.XferXYLen.xLen = pData->xXferLength;
	dmaCtrlBlkInfo.xferLength.XferXYLen.yLen = pData->yXferLength;

	if (chal_dma_vc4lite_build_ctrlblk_list(pdma->handle,
						pdma->chan[chanID].
						pDmaChanCtrlBlkList,
						pdma->chan[chanID].
						dmaChanCtrlBlkItemNum,
						pdma->chan[chanID].
						dmaChanCtrlBlkMemSize,
						&dmaCtrlBlkInfo) !=
	    CHAL_DMA_VC4LITE_STATUS_SUCCESS) {
		dprintf(1, "%s: get control block list failure\n",
			__FUNCTION__);
		return DMA_VC4LITE_STATUS_FAILURE;
	}

	printk(KERN_ERR "DMA_Ex ctl blk item num = %d, ctl blk info"
	       "0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n",
	       pdma->chan[chanID].dmaChanCtrlBlkItemNum,
	       ((unsigned int *)pdma->chan[chanID].pDmaChanCtrlBlkList)[0],
	       ((unsigned int *)pdma->chan[chanID].pDmaChanCtrlBlkList)[1],
	       ((unsigned int *)pdma->chan[chanID].pDmaChanCtrlBlkList)[2],
	       ((unsigned int *)pdma->chan[chanID].pDmaChanCtrlBlkList)[3],
	       ((unsigned int *)pdma->chan[chanID].pDmaChanCtrlBlkList)[4],
	       ((unsigned int *)pdma->chan[chanID].pDmaChanCtrlBlkList)[5]
	    );

	pdma->chan[chanID].dmaChanCtrlBlkItemNum++;
	return DMA_VC4LITE_STATUS_SUCCESS;
}

//******************************************************************************
//
//  Funciton Name:      dma_vc4lite_lisr
//  
//  Description:        Low level VC4LITE DMA interrupt service routine
//
//******************************************************************************
#ifndef UNDER_LINUX
static void dma_vc4lite_lisr(void)
#else
static irqreturn_t bcm_vc4l_dma_interrupt(int irq, void *dev_id)
#endif
{
	CslDmaVc4lite_t *pdma = (CslDmaVc4lite_t *)&dmac;
	UInt8 chanNum;

	printk(KERN_ERR "DMA int hapened for irq =%d\n", irq);
#ifndef UNDER_LINUX
	// disable the dma channel 1 interrupt
	IRQ_Disable(MM_DMA_CHAN1_IRQ);
	IRQ_Clear(MM_DMA_CHAN1_IRQ);

	// disable the dma channel 2 interrupt
	IRQ_Disable(MM_DMA_CHAN2_IRQ);
	IRQ_Clear(MM_DMA_CHAN2_IRQ);

	// disable the dma channel 3 interrupt
	IRQ_Disable(MM_DMA_CHAN3_IRQ);
	IRQ_Clear(MM_DMA_CHAN3_IRQ);
#else
	//disable_irq_nosync(BCM_INT_ID_MM_DMA_CHAN1);
	//disable_irq_nosync(BCM_INT_ID_MM_DMA_CHAN2);
	//disable_irq_nosync(BCM_INT_ID_MM_DMA_CHAN3);
#endif

	// sanity check
	if (!pdma->initialized) {
#ifndef UNDER_LINUX
		return;
#else
		return IRQ_HANDLED;
#endif
	}
	// check the interrupt status
	for (chanNum = 0; chanNum < 1; chanNum++) {
		pdma->chan[chanNum].chanState =
		    chal_dma_vc4lite_get_channel_state(pdma->handle, chanNum);
		printk(KERN_ERR "chan %d CS reg is 0x%08x\n", chanNum,
		       readl(HW_IO_PHYS_TO_VIRT(DMA_VC4LITE_BASE_ADDR) +
			     chanNum * 0x100));
		if (chal_dma_vc4lite_get_int_status
		    (pdma->handle, chanNum,
		     &pdma->chan[chanNum].irqStatus) ==
		    CHAL_DMA_VC4LITE_STATUS_SUCCESS) {
			if (pdma->chan[chanNum].irqStatus != 0) {
				printk(KERN_ERR
				       "ISR is happening for channel = %d",
				       chanNum);

				chal_dma_vc4lite_clear_int_status(pdma->handle,
								  chanNum);
				pdma->chan[chanNum].dmaChanCtrlBlkItemNum = 0;
				dma_unmap_single(NULL,
						 (dma_addr_t)pdma->
						 chan[chanNum].
						 pDmaChanCtrlBlkListPHYS,
						 DMA_VC4LITE_CHANNEL_CTRL_BLOCK_SIZE,
						 DMA_TO_DEVICE);
			} else
				printk(KERN_ERR
				       "channel = %d has no irq status",
				       chanNum);

		} else
			printk(KERN_ERR
			       "failed to read irq status for channel = %d",
			       chanNum);

	}

	OSINTERRUPT_Trigger(pdma->hisr);

#ifdef UNDER_LINUX
	return IRQ_HANDLED;
#endif
}

//******************************************************************************
//
//  Funciton Name:      dma_vc4lite_hisr
//  
//  Description:        High level VC4LITE DMA interrupt service routine
//
//******************************************************************************
static void dma_vc4lite_hisr(void)
{
	CslDmaVc4lite_t *pdma = (CslDmaVc4lite_t *)&dmac;
	UInt8 chanNum;

	// process the callback function
	for (chanNum = 0; chanNum < 1; chanNum++) {
		if (pdma->chan[chanNum].irqStatus != 0) {
			printk(KERN_ERR "callback for channel = %d", chanNum);
			if (pdma->chan[chanNum].chanInfo.callback) {
				if ((pdma->chan[chanNum].chanState !=
				     CHAL_DMA_VC4LITE_STATE_INVALID)
				    && (pdma->chan[chanNum].
					chanState | CHAL_DMA_VC4LITE_STATE_END))
					pdma->chan[chanNum].chanInfo.
					    callback
					    (DMA_VC4LITE_CALLBACK_SUCCESS);
				else
					pdma->chan[chanNum].chanInfo.
					    callback(DMA_VC4LITE_CALLBACK_FAIL);
			}
			// free the channel is the auto flag is set
			if (pdma->chan[chanNum].chanInfo.autoFreeChan) {
				if (chal_dma_vc4lite_reset_channel
				    (pdma->handle,
				     chanNum) !=
				    CHAL_DMA_VC4LITE_STATUS_SUCCESS) {
					dprintf(1,
						"%s: reset DMA channel error\n",
						__FUNCTION__);
				}

				pdma->chan[chanNum].used = FALSE;
				pdma->chan[chanNum].dmaChanCtrlBlkItemNum = 0;
				memset(&pdma->chan[chanNum].chanInfo, 0,
				       sizeof(DMA_VC4LITE_CHANNEL_INFO_t));
			}

			pdma->chan[chanNum].irqStatus = 0;
		}
	}

#ifndef UNDER_LINUX
	IRQ_Enable(MM_DMA_CHAN1_IRQ);
	IRQ_Enable(MM_DMA_CHAN2_IRQ);
	IRQ_Enable(MM_DMA_CHAN3_IRQ);
#else
	//enable_irq(BCM_INT_ID_MM_DMA_CHAN1);
	//enable_irq(BCM_INT_ID_MM_DMA_CHAN2);
	//enable_irq(BCM_INT_ID_MM_DMA_CHAN3);
#endif
}

//******************************************************************************
//
//  Funciton Name:      dma_vc4lite_per_map
//  
//  Description:        peripheral mapping for the peripheral ID number
//
//******************************************************************************
static UInt8 dma_vc4lite_per_map(UInt8 ClientID)
{
	UInt8 perID;

	switch (ClientID) {
	case DMA_VC4LITE_CLIENT_MEMORY:
		perID = CHAL_DMA_VC4LITE_DREQ_NONE;
		break;

	case DMA_VC4LITE_CLIENT_DSI0:
		perID = CHAL_DMA_VC4LITE_DREQ_DSI0;
		break;

	case DMA_VC4LITE_CLIENT_DSI1:
		perID = CHAL_DMA_VC4LITE_DREQ_DSI1;
		break;

	case DMA_VC4LITE_CLIENT_SMI:
		perID = CHAL_DMA_VC4LITE_DREQ_SMI;
		break;

	case DMA_VC4LITE_CLIENT_SPI_TX:
		perID = CHAL_DMA_VC4LITE_DREQ_SPI_TX;
		break;

	case DMA_VC4LITE_CLIENT_SPI_RX:
		perID = CHAL_DMA_VC4LITE_DREQ_SPI_RX;
		break;

	default:
		perID = 0xff;
		break;
	}

	return perID;
}
