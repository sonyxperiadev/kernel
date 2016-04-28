/*****************************************************************************
*  Copyright 2001 - 2009 Broadcom Corporation.  All rights reserved.
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2, available at
*  http://www.gnu.org/licenses/old-license/gpl-2.0.html (the "GPL").
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
*****************************************************************************/

#define UNDER_LINUX

/*
 *
 * \file   chal_dma_vc4lite.c
 * \brief  OS independent code of MM DMA hardware abstraction APIs.
 * \note
 */
#include "plat/chal/chal_types.h"
#include "plat/chal/chal_common.h"
#include "plat/chal/chal_dma_vc4lite.h"
#include "mach/rdb/brcm_rdb_dma.h"
#include "mach/rdb/brcm_rdb_sysmap.h"
#include "mach/rdb/brcm_rdb_util.h"
#ifdef UNDER_LINUX
#include "mach/memory.h"
#endif

/*
 * local macro declarations
 */

/* #define CHAL_DMAVC4LITE_DEBUG */
#define DMA_CHANNEL_ADDR_OFFSET     (0x40)
#define DMA_MAX_CHANNEL             (3)

#define CHAL_DMA_VC4LITE_ENABLE     (1)
#define CHAL_DMA_VC4LITE_DISABLE    (0)

#ifdef CHAL_DMAVC4LITE_DEBUG
#define  DMA_DEBUG(a)    a
#else
#define  DMA_DEBUG(a)
#endif

/* extended register access functions */
#define BRCM_READ_REG_IDX_FIELD_DMA(b, r, i, f) \
	((readl(((BRCM_REGTYPE(r)*)BRCM_REGADDR(b, r)) + i) & \
	BRCM_FIELDMASK(r, f)) >> BRCM_FIELDSHIFT(r, f))

#if 0
#define BRCM_READ_REG_IDX_FIELD_DMA(b, r, i, f) ((((volatile BRCM_REGTYPE(r)*) \
	BRCM_REGADDR(b, r))[i] & BRCM_FIELDMASK(r, f)) >> BRCM_FIELDSHIFT(r, f))
#endif

#define BRCM_WRITE_REG_IDX_FIELD_DMA(b, r, i, f, d) \
	(BRCM_WRITE_REG_IDX(b, r, i, ((((cUInt32)(d) \
	<< BRCM_FIELDSHIFT(r, f)) & BRCM_FIELDMASK(r, f)) | \
	(BRCM_READ_REG_IDX(b, r, i) & (~BRCM_FIELDMASK(r, f))))))

typedef struct {
	cUInt32 intEnable : 1;
	cUInt32 tdMode : 1;
	cUInt32 unused0 : 1;
	cUInt32 waitResp : 1;
	cUInt32 dstInc : 1;
	cUInt32 dstWidth : 1;
	cUInt32 dstDreq : 1;
	cUInt32 dstIgnore : 1;
	cUInt32 srcInc : 1;
	cUInt32 srcWidth : 1;
	cUInt32 srcDreq : 1;
	cUInt32 srcIgnore : 1;
	cUInt32 burstLength : 4;
	cUInt32 perMap : 5;
	cUInt32 waits : 5;
	cUInt32 noWideBurst : 1;
#ifdef CONFIG_ARCH_HAWAII
	cUInt32 burstWriteEnable32 : 1;
	cUInt32 unused1 : 4;
#else
	cUInt32 unused1 : 5;
#endif
	cUInt32 srcAddr : 32;
	cUInt32 dstAddr : 32;
	cUInt32 xferLen : 32;
	cUInt32 srcStride : 16;
	cUInt32 dstStride : 16;
	cUInt32 nextCtrlBlk : 32;
	cUInt32 unused2 : 32;
	cUInt32 unused3 : 32;
} ChalDmaVc4liteCtrlBlk_t;

typedef struct {
	cUInt32 baseAddr;
	cBool intEnableFlag[DMA_MAX_CHANNEL];
} ChalDmaVc4liteDev_t, *pChalDmaVc4liteDev_t;

/*
 * Local Variables
 */
static ChalDmaVc4liteDev_t dmaDev;

/*
 * cHAL Functions
 */

/*
 *
 *  @brief   Initialize VC4LITE DMA hardware and software interface.
 *
 *  @param   baseAddress (in) mapped address of DMA controller
 *
 *  @return  Handle of this DMA instance
 *
 *  @note
 *
 */
CHAL_HANDLE chal_dma_vc4lite_init(cUInt32 baseAddress)
{
	ChalDmaVc4liteDev_t *pDmaDev;

	DMA_DEBUG(chal_dprintf(CDBG_INFO, "%s()\n", __func__));

	pDmaDev = &dmaDev;
	pDmaDev->baseAddr = baseAddress;

	return (CHAL_HANDLE) pDmaDev;
}

/*
 *
 *  @brief   Clear the VC4LITE DMA controller's interrupt status
 *
 *  @param   handle   (in) Handle returned in chal_dma_init()
 *  @param   channel  (in) DMA channel number
 *
 *  @return  DMA status
 *
 */
void chal_dma_vc4lite_clear_int_status(CHAL_HANDLE handle, cUInt32 channel)
{
	ChalDmaVc4liteDev_t *pDmaDev = &dmaDev;
	DMA_DEBUG(chal_dprintf(CDBG_INFO, "%s()\n", __func__));

	if (channel >= DMA_MAX_CHANNEL) {
		chal_dprintf(CDBG_ERRO,
			     "%s(): invalid channel parameter (%d)\n",
			     __func__, (int)channel);
		return;
	}

	pDmaDev->intEnableFlag[channel] = CHAL_DMA_VC4LITE_DISABLE;

	/* clean up the interrupt status */
	BRCM_WRITE_REG_IDX_FIELD_DMA(pDmaDev->baseAddr,
				     DMA_CS,
				     (channel * DMA_CHANNEL_ADDR_OFFSET),
				     INT, CHAL_DMA_VC4LITE_ENABLE);

	/* clear the END state bit */
	BRCM_WRITE_REG_IDX_FIELD_DMA(pDmaDev->baseAddr,
				     DMA_CS,
				     (channel * DMA_CHANNEL_ADDR_OFFSET),
				     END, CHAL_DMA_VC4LITE_ENABLE);
}

/*
 *
 *  @brief   get the VC4LITE DMA controller's interrupt status
 *
 *  @param   handle   (in) Handle returned in chal_dma_init()
 *  @param   channel  (in) DMA channel number
 *  @param   intStatus  (out) if the intStatus is set to 1, it represents
 *                            the interrupt is generated for the channel
 *
 *  @return  DMA status
 *
 */
CHAL_DMA_VC4LITE_STATUS_t chal_dma_vc4lite_get_int_status(CHAL_HANDLE handle,
							  cUInt32 channel,
							  cBool *intStatus)
{
	ChalDmaVc4liteDev_t *pDmaDev = &dmaDev;
	DMA_DEBUG(chal_dprintf(CDBG_INFO, "%s()\n", __func__));

	if (channel >= DMA_MAX_CHANNEL) {
		chal_dprintf(CDBG_ERRO,
			     "%s(): invalid channel parameter (%d)\n",
			     __func__, (int)channel);
		return CHAL_DMA_VC4LITE_STATUS_INVALID_PARAMETER;
	}
	/* get the interrupt status */
	*intStatus = BRCM_READ_REG_IDX_FIELD_DMA(pDmaDev->baseAddr,
						 DMA_CS,
						 (channel *
						  DMA_CHANNEL_ADDR_OFFSET),
						 INT);

	return CHAL_DMA_VC4LITE_STATUS_SUCCESS;
}

/*
 *
 *  @brief   enable the interrupt for the specified DMA channel
 *
 *  @param   handle   (in) Handle returned in chal_dma_init()
 *  @param   channel  (in) DMA channel number
 *
 *  @return  DMA status
 *
 */
CHAL_DMA_VC4LITE_STATUS_t chal_dma_vc4lite_enable_int(CHAL_HANDLE handle,
						      cUInt32 channel)
{
	ChalDmaVc4liteDev_t *pDmaDev = &dmaDev;
	DMA_DEBUG(chal_dprintf(CDBG_INFO, "%s()\n", __func__));

	if (channel >= DMA_MAX_CHANNEL) {
		chal_dprintf(CDBG_ERRO,
			     "%s(): invalid channel parameter (%d)\n",
			     __func__, (int)channel);
		return CHAL_DMA_VC4LITE_STATUS_INVALID_PARAMETER;
	}
	/* set the interrupt enable flag for the specified channel */
	pDmaDev->intEnableFlag[channel] = CHAL_DMA_VC4LITE_ENABLE;

	return CHAL_DMA_VC4LITE_STATUS_SUCCESS;
}

/*
 *
 *  @brief   Prepare a DMA transfer
 *
 *  @param   handle   (in) Handle returned in chal_dma_init()
 *  @param   channel  (in) DMA channel number
 *  @param   ctrlBlk  (in) buffer to store the DMA control block
 *
 *  @return  DMA status
 *
 *  @note
 */
CHAL_DMA_VC4LITE_STATUS_t chal_dma_vc4lite_prepare_transfer(CHAL_HANDLE handle,
							    cUInt32 channel,
							    cVoid * ctrlBlkList
#ifdef UNDER_LINUX
							    ,
							    cVoid *
							    ctrlBlkListPHYS
#endif
)
{
	ChalDmaVc4liteDev_t *pDmaDev = &dmaDev;
	ChalDmaVc4liteCtrlBlk_t *pCtrlBlkList =
	    (ChalDmaVc4liteCtrlBlk_t *) ctrlBlkList;
	DMA_DEBUG(chal_dprintf(CDBG_INFO, "%s()\n", __func__));

	if (channel >= DMA_MAX_CHANNEL) {
		chal_dprintf(CDBG_ERRO,
			     "%s(): invalid channel parameter (%d)\n",
			     __func__, (int)channel);
		return CHAL_DMA_VC4LITE_STATUS_INVALID_PARAMETER;
	}

	if ((!ctrlBlkList) || ((cUInt32) (ctrlBlkList) & 0x1F)) {
		chal_dprintf(CDBG_ERRO,
			     "%s(): invalid control block address (0x%x)\n",
			     __func__, (unsigned int)(ctrlBlkList));
		return CHAL_DMA_VC4LITE_STATUS_INVALID_PARAMETER;
	}
	/* enable the interrupt bit for the last control block */
	if (pDmaDev->intEnableFlag[channel]) {
		/* search to the last control block */
		while (pCtrlBlkList->nextCtrlBlk) {
			int offset, nxtCtlBlrList;
			offset = (int)pCtrlBlkList->nextCtrlBlk;
			offset -= (int)ctrlBlkListPHYS;
			nxtCtlBlrList = (int)ctrlBlkList + offset;
			pCtrlBlkList = (ChalDmaVc4liteCtrlBlk_t *)nxtCtlBlrList;
		}
		pCtrlBlkList->intEnable = CHAL_DMA_VC4LITE_ENABLE;
	}
	mb();

	/* clean up the interrupt status */
#ifndef UNDER_LINUX
	BRCM_WRITE_REG_IDX(pDmaDev->baseAddr,
			   DMA_CONBLK_AD, (channel * DMA_CHANNEL_ADDR_OFFSET),
			   (cUInt32) (ctrlBlkList));
#else
	BRCM_WRITE_REG_IDX(pDmaDev->baseAddr,
			   DMA_CONBLK_AD, (channel * DMA_CHANNEL_ADDR_OFFSET),
			   (cUInt32) (ctrlBlkListPHYS));
#endif
	return CHAL_DMA_VC4LITE_STATUS_SUCCESS;
}

/*
 *
 *  @brief   abort a DMA channel transfer for current DMA control block
 *
 *  @param   handle   (in) Handle returned in chal_dma_init()
 *  @param   channel  (in) DMA channel number
 *
 *  @return  DMA status
 *
 *  @note
 */
CHAL_DMA_VC4LITE_STATUS_t chal_dma_vc4lite_abort_transfer(CHAL_HANDLE handle,
							  cUInt32 channel)
{
	ChalDmaVc4liteDev_t *pDmaDev = &dmaDev;
	cUInt32 chanStatus;
	DMA_DEBUG(chal_dprintf(CDBG_INFO, "%s()\n", __func__));

	if (channel >= DMA_MAX_CHANNEL) {
		chal_dprintf(CDBG_ERRO,
			     "%s(): invalid channel parameter (%d)\n",
			     __func__, (int)channel);
		return CHAL_DMA_VC4LITE_STATUS_INVALID_PARAMETER;
	}
	/* get the channel status */
	chanStatus = BRCM_READ_REG_IDX_FIELD_DMA(pDmaDev->baseAddr,
						 DMA_CS,
						 (channel *
						  DMA_CHANNEL_ADDR_OFFSET),
						 ACTIVE);

	pDmaDev->intEnableFlag[channel] = CHAL_DMA_VC4LITE_DISABLE;

	/* abort the current control block transfer */
	if (chanStatus) {
		BRCM_WRITE_REG_IDX_FIELD_DMA(pDmaDev->baseAddr,
					     DMA_CS,
					     (channel *
					      DMA_CHANNEL_ADDR_OFFSET), ABORT,
					     CHAL_DMA_VC4LITE_ENABLE);
	}

	return CHAL_DMA_VC4LITE_STATUS_SUCCESS;
}

/*
 *
 *  @brief   enable the DMA channel
 *
 *  @param   handle   (in) Handle returned in chal_dma_init()
 *  @param   channel  (in) DMA channel number
 *
 *  @return  DMA status
 *
 *  @note
 */
CHAL_DMA_VC4LITE_STATUS_t chal_dma_vc4lite_enable_channel(CHAL_HANDLE handle,
							  cUInt32 channel)
{
	ChalDmaVc4liteDev_t *pDmaDev = &dmaDev;
	cUInt32 chanStatus;
	DMA_DEBUG(chal_dprintf(CDBG_INFO, "%s()\n", __func__));

	if (channel >= DMA_MAX_CHANNEL) {
		chal_dprintf(CDBG_ERRO,
			     "%s(): invalid channel parameter (%d)\n",
			     __func__, (int)channel);
		return CHAL_DMA_VC4LITE_STATUS_INVALID_PARAMETER;
	}
	/* get the channel status */
	chanStatus = BRCM_READ_REG_IDX_FIELD_DMA(pDmaDev->baseAddr,
						 DMA_CS,
						 (channel *
						  DMA_CHANNEL_ADDR_OFFSET),
						 ACTIVE);

	/* activate the specified DMA channel */
	if (!chanStatus) {
		BRCM_WRITE_REG_IDX_FIELD_DMA(pDmaDev->baseAddr,
					     DMA_CS,
					     (channel *
					      DMA_CHANNEL_ADDR_OFFSET), ACTIVE,
					     CHAL_DMA_VC4LITE_ENABLE);
	} else {
		chal_dprintf(CDBG_ERRO, "%s(): channel (%d) is busy\n",
			     __func__, (int)channel);
		return CHAL_DMA_VC4LITE_STATUS_INVALID_STATE;
	}

	return CHAL_DMA_VC4LITE_STATUS_SUCCESS;
}

/*
 *
 *  @brief   disable the DMA channel
 *
 *  @param   handle   (in) Handle returned in chal_dma_init()
 *  @param   channel  (in) DMA channel number
 *
 *  @return  DMA status
 *
 *  @note
 */
CHAL_DMA_VC4LITE_STATUS_t chal_dma_vc4lite_disable_channel(CHAL_HANDLE handle,
							   cUInt32 channel)
{
	ChalDmaVc4liteDev_t *pDmaDev = &dmaDev;
	cUInt32 chanStatus;
	DMA_DEBUG(chal_dprintf(CDBG_INFO, "%s()\n", __func__));

	if (channel >= DMA_MAX_CHANNEL) {
		chal_dprintf(CDBG_ERRO,
			     "%s(): invalid channel parameter (%d)\n",
			     __func__, (int)channel);
		return CHAL_DMA_VC4LITE_STATUS_INVALID_PARAMETER;
	}
	/* get the channel status */
	chanStatus = BRCM_READ_REG_IDX_FIELD_DMA(pDmaDev->baseAddr,
						 DMA_CS,
						 (channel *
						  DMA_CHANNEL_ADDR_OFFSET),
						 ACTIVE);

	/* de-activate the specified DMA channel */
	if (chanStatus) {
		BRCM_WRITE_REG_IDX_FIELD_DMA(pDmaDev->baseAddr,
					     DMA_CS,
					     (channel *
					      DMA_CHANNEL_ADDR_OFFSET), ACTIVE,
					     CHAL_DMA_VC4LITE_DISABLE);
	}

	return CHAL_DMA_VC4LITE_STATUS_SUCCESS;
}

/*
 *
 *  @brief   reset the DMA channel
 *
 *  @param   handle   (in) Handle returned in chal_dma_init()
 *  @param   channel  (in) DMA channel number
 *
 *  @return  DMA status
 *
 *  @note
 */
CHAL_DMA_VC4LITE_STATUS_t chal_dma_vc4lite_reset_channel(CHAL_HANDLE handle,
							 cUInt32 channel)
{
	ChalDmaVc4liteDev_t *pDmaDev = &dmaDev;
	DMA_DEBUG(chal_dprintf(CDBG_INFO, "%s()\n", __func__));

	if (channel >= DMA_MAX_CHANNEL) {
		chal_dprintf(CDBG_ERRO,
			     "%s(): invalid channel parameter (%d)\n",
			     __func__, (int)channel);
		return CHAL_DMA_VC4LITE_STATUS_INVALID_PARAMETER;
	}

	pDmaDev->intEnableFlag[channel] = CHAL_DMA_VC4LITE_DISABLE;

	/* reset the specified DMA channel */
	BRCM_WRITE_REG_IDX_FIELD_DMA(pDmaDev->baseAddr,
				     DMA_CS,
				     (channel * DMA_CHANNEL_ADDR_OFFSET),
				     RESET, CHAL_DMA_VC4LITE_ENABLE);

	return CHAL_DMA_VC4LITE_STATUS_SUCCESS;
}

/*
 *
 *  @brief   get the state of the DMA channel
 *
 *  @param   handle   (in) Handle returned in chal_dma_init()
 *  @param   channel  (in) DMA channel number
 *
 *  @return  DMA state
 *
 *  @note
 */
cUInt32 chal_dma_vc4lite_get_channel_state(CHAL_HANDLE handle, cUInt32 channel)
{
	ChalDmaVc4liteDev_t *pDmaDev = &dmaDev;
	cUInt32 chanState = 0;
	DMA_DEBUG(chal_dprintf(CDBG_INFO, "%s()\n", __func__));

	if (channel >= DMA_MAX_CHANNEL) {
		chal_dprintf(CDBG_ERRO,
			     "%s(): invalid channel parameter (%d)\n",
			     __func__, (int)channel);
		return CHAL_DMA_VC4LITE_STATE_INVALID;
	}
	/* check the state */
	if (BRCM_READ_REG_IDX_FIELD_DMA
	    (pDmaDev->baseAddr, DMA_CS, (channel * DMA_CHANNEL_ADDR_OFFSET),
	     ERROR))
		chanState |= CHAL_DMA_VC4LITE_STATE_ERROR;

	if (BRCM_READ_REG_IDX_FIELD_DMA
	    (pDmaDev->baseAddr, DMA_CS, (channel * DMA_CHANNEL_ADDR_OFFSET),
	     DREQ_STOPS_DMA))
		chanState |= CHAL_DMA_VC4LITE_STATE_DREQ_PAUSED;

	if (BRCM_READ_REG_IDX_FIELD_DMA
	    (pDmaDev->baseAddr, DMA_CS, (channel * DMA_CHANNEL_ADDR_OFFSET),
	     PAUSED))
		chanState |= CHAL_DMA_VC4LITE_STATE_PAUSED;

	if (BRCM_READ_REG_IDX_FIELD_DMA
	    (pDmaDev->baseAddr, DMA_CS, (channel * DMA_CHANNEL_ADDR_OFFSET),
	     DREQ))
		chanState |= CHAL_DMA_VC4LITE_STATE_DREQ;

	if (BRCM_READ_REG_IDX_FIELD_DMA
	    (pDmaDev->baseAddr, DMA_CS, (channel * DMA_CHANNEL_ADDR_OFFSET),
	     END))
		chanState |= CHAL_DMA_VC4LITE_STATE_END;

	return chanState;
}

/*
 *
 *  @brief   get the control block size
 *
 *  @param   handle   (in) Handle returned in chal_dma_init()
 *
 *  @return  DMA control block size
 *
 *  @note
 */
cUInt32 chal_dma_vc4lite_get_ctrlblk_size(CHAL_HANDLE handle)
{
	return sizeof(ChalDmaVc4liteCtrlBlk_t);
}

/*
 *
 *  @brief   build the control block list
 *
 *  @param   handle   (in) Handle returned in chal_dma_init()
 *  @param   ctrlBlkList  (in) the ctrl block link list
 *  @param   ctlBlkItemNum  (in) number of control block link list items
 *  @param   ctlBlkMemSize  (in) the memory size for the current ctrl block
 *  @param   curCtrlBlkInfo  (in) the current control block information
 *
 *  @return  DMA control block size
 *
 *  @note
 */
CHAL_DMA_VC4LITE_STATUS_t
	chal_dma_vc4lite_build_ctrlblk_list(CHAL_HANDLE
					    handle,
					    cVoid *
					    ctlBlkList,
					    cVoid *
					    ctlBlkListPhys,
					    cUInt32
					    ctlBlkItemNum,
					    cUInt32
					    ctlBlkMemSize,
					    ChalDmaCtrlBlkInfo_t
					    *curCtrlBlkInfo)
{
	ChalDmaVc4liteCtrlBlk_t *pCurCtrlBlk =
	    (ChalDmaVc4liteCtrlBlk_t *) ((cUInt32) ctlBlkList +
					 ctlBlkItemNum *
					 sizeof(ChalDmaVc4liteCtrlBlk_t));
	ChalDmaVc4liteCtrlBlk_t *pCurCtrlBlkPhys =
	    (ChalDmaVc4liteCtrlBlk_t *) ((cUInt32) ctlBlkListPhys +
					 ctlBlkItemNum *
					 sizeof(ChalDmaVc4liteCtrlBlk_t));
	ChalDmaVc4liteCtrlBlk_t *pPrvCtrlBlk;
	cUInt32 i;

	/* sanity check */
	if ((!ctlBlkList)
	    || ((ctlBlkItemNum + 1) * sizeof(ChalDmaVc4liteCtrlBlk_t) >
		ctlBlkMemSize) || (!curCtrlBlkInfo)) {
		chal_dprintf(CDBG_ERRO, "%s(): invalid DMA parameters\n",
			     __func__);
		return CHAL_DMA_VC4LITE_STATUS_INVALID_PARAMETER;
	}
	/* sanity check */
	if (((curCtrlBlkInfo->srcDreqID != CHAL_DMA_VC4LITE_DREQ_NONE)
	     && (curCtrlBlkInfo->dstDreqID != CHAL_DMA_VC4LITE_DREQ_NONE))
	    || ((curCtrlBlkInfo->srcDreqID != CHAL_DMA_VC4LITE_DREQ_NONE)
		&& (curCtrlBlkInfo->srcDreqID != CHAL_DMA_VC4LITE_DREQ_DSI0)
		&& (curCtrlBlkInfo->srcDreqID != CHAL_DMA_VC4LITE_DREQ_SMI)
		&& (curCtrlBlkInfo->srcDreqID != CHAL_DMA_VC4LITE_DREQ_SPI_TX)
		&& (curCtrlBlkInfo->srcDreqID != CHAL_DMA_VC4LITE_DREQ_SPI_RX)
		&& (curCtrlBlkInfo->srcDreqID != CHAL_DMA_VC4LITE_DREQ_DSI1))
	    || ((curCtrlBlkInfo->dstDreqID != CHAL_DMA_VC4LITE_DREQ_NONE)
		&& (curCtrlBlkInfo->dstDreqID != CHAL_DMA_VC4LITE_DREQ_DSI0)
		&& (curCtrlBlkInfo->dstDreqID != CHAL_DMA_VC4LITE_DREQ_SMI)
		&& (curCtrlBlkInfo->dstDreqID != CHAL_DMA_VC4LITE_DREQ_SPI_TX)
		&& (curCtrlBlkInfo->dstDreqID != CHAL_DMA_VC4LITE_DREQ_SPI_RX)
		&& (curCtrlBlkInfo->dstDreqID != CHAL_DMA_VC4LITE_DREQ_DSI1))) {
		chal_dprintf(CDBG_ERRO, "%s(): invalid DMA parameters\n",
			     __func__);
		return CHAL_DMA_VC4LITE_STATUS_INVALID_PARAMETER;
	}
	/* init the current DMA control block */
	for (i = 0; i < sizeof(ChalDmaVc4liteCtrlBlk_t) / 4; i++)
		*(cUInt32 *) ((cUInt32) pCurCtrlBlk + i * 4) = 0;

#ifdef CONFIG_ARCH_HAWAII
	pCurCtrlBlk->burstWriteEnable32 = curCtrlBlkInfo->burstWriteEnable32;
#endif
	pCurCtrlBlk->noWideBurst = curCtrlBlkInfo->noWideBurst;
	pCurCtrlBlk->waits = curCtrlBlkInfo->waitCycles;
	pCurCtrlBlk->burstLength = curCtrlBlkInfo->burstLength;
	pCurCtrlBlk->srcIgnore = curCtrlBlkInfo->srcIgnoreRead;
	pCurCtrlBlk->srcWidth = curCtrlBlkInfo->srcXferWidth;
	pCurCtrlBlk->srcInc = curCtrlBlkInfo->srcAddrIncrement;
	pCurCtrlBlk->dstIgnore = curCtrlBlkInfo->dstIgnoreWrite;
	pCurCtrlBlk->dstWidth = curCtrlBlkInfo->dstXferWidth;
	pCurCtrlBlk->dstInc = curCtrlBlkInfo->dstAddrIncrement;
	pCurCtrlBlk->waitResp = curCtrlBlkInfo->waitResponse;
	pCurCtrlBlk->tdMode = (cUInt8) curCtrlBlkInfo->xferMode;
#ifndef ACP_ENABLE
	pCurCtrlBlk->srcAddr = (cUInt32) curCtrlBlkInfo->srcAddr;
#ifdef UNDER_LINUX

	if (curCtrlBlkInfo->dstDreqID != CHAL_DMA_VC4LITE_DREQ_NONE)
		pCurCtrlBlk->dstAddr =
		    (cUInt32) HW_IO_VIRT_TO_PHYS(curCtrlBlkInfo->dstAddr);
	else
		pCurCtrlBlk->dstAddr = (cUInt32) curCtrlBlkInfo->dstAddr;

#else
	pCurCtrlBlk->dstAddr = (cUInt32) curCtrlBlkInfo->dstAddr;
#endif
#else
#if defined(_RHEA_)
	pCurCtrlBlk->srcAddr =
	    ((cUInt32) curCtrlBlkInfo->srcAddr) & 0x0FFFFFFF | 0x40000000;
	pCurCtrlBlk->dstAddr =
	    ((cUInt32) curCtrlBlkInfo->dstAddr) & 0x0FFFFFFF | 0x40000000;
#else
	pCurCtrlBlk->srcAddr =
	    ((cUInt32) curCtrlBlkInfo->srcAddr) & 0x0FFFFFFF | 0xC0000000;
	pCurCtrlBlk->dstAddr =
	    ((cUInt32) curCtrlBlkInfo->dstAddr) & 0x0FFFFFFF | 0xC0000000;
#endif
#endif

	if ((curCtrlBlkInfo->srcDreqID == CHAL_DMA_VC4LITE_DREQ_NONE) &&
	    (curCtrlBlkInfo->dstDreqID == CHAL_DMA_VC4LITE_DREQ_NONE)) {
		pCurCtrlBlk->perMap = CHAL_DMA_VC4LITE_DREQ_NONE;
		pCurCtrlBlk->srcDreq = 0;
		pCurCtrlBlk->dstDreq = 0;
	} else if (curCtrlBlkInfo->srcDreqID != CHAL_DMA_VC4LITE_DREQ_NONE) {
		pCurCtrlBlk->perMap = curCtrlBlkInfo->srcDreqID;
		pCurCtrlBlk->srcDreq = 1;
		pCurCtrlBlk->dstDreq = 0;
	} else if (curCtrlBlkInfo->dstDreqID != CHAL_DMA_VC4LITE_DREQ_NONE) {
		pCurCtrlBlk->perMap = curCtrlBlkInfo->dstDreqID;
		pCurCtrlBlk->srcDreq = 0;
		pCurCtrlBlk->dstDreq = 1;
	}

	if (curCtrlBlkInfo->xferMode == CHAL_DMA_VC4LITE_2D_MODE) {
		pCurCtrlBlk->srcStride = curCtrlBlkInfo->srcStride;
		pCurCtrlBlk->dstStride = curCtrlBlkInfo->dstStride;
		pCurCtrlBlk->xferLen =
		    (curCtrlBlkInfo->xferLength.XferXYLen.
		     yLen << 16) | curCtrlBlkInfo->xferLength.XferXYLen.xLen;
	} else {
		pCurCtrlBlk->xferLen = curCtrlBlkInfo->xferLength.len;
	}

	if (ctlBlkItemNum != 0) {
		pPrvCtrlBlk =
		    (ChalDmaVc4liteCtrlBlk_t *) ((cUInt32) ctlBlkList +
						 (ctlBlkItemNum -
						  1) *
						 sizeof
						 (ChalDmaVc4liteCtrlBlk_t));
		pPrvCtrlBlk->nextCtrlBlk = (cUInt32)(pCurCtrlBlkPhys);
	}

	return CHAL_DMA_VC4LITE_STATUS_SUCCESS;
}
