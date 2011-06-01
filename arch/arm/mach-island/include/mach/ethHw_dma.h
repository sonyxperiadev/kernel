/*****************************************************************************
* Copyright 2006 - 2008 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/
#ifndef _ETHHW_DMA_H
#define _ETHHW_DMA_H

#include <linux/types.h>
#include <mach/dmacHw_reg.h>
#include <mach/ethHw_reg.h>
#include <mach/memory.h>
#include <mach/reg_utils.h>

#define ETHHW_DMA_ALIGN_BUF(bp)        ((uint32_t)(bp) & 0xfffffff8)
#define ETHHW_DMA_CFG_EOP_MASK         0x00010000
#define ETHHW_DMA_CFG_OFFSET_INIT(bp)  ((uint32_t)(bp) % 8)
#define ETHHW_DMA_CFG_OFFSET_LAST(bp, len) \
                                       ((((uint32_t)(bp) + (len)) - 1) % 8)
#define ETHHW_DMA_CFG_OFFSET(bp, len)  ((ETHHW_DMA_CFG_OFFSET_LAST((bp), (len)) << 8) | \
                                        (ETHHW_DMA_CFG_OFFSET_INIT((bp))))

#define ETHHW_DMA_MTP_TRANSACTION_SIZE(bp, len) \
                                       ((ETHHW_DMA_CFG_OFFSET_INIT((bp)) + (len) + 7) / 8)
#define ETHHW_DMA_MTP_FIFO_ADDR_CFG    HW_IO_VIRT_TO_PHYS( (uint32_t)&ethHw_regMtpFifoConfig )
#define ETHHW_DMA_MTP_FIFO_ADDR_DATA   HW_IO_VIRT_TO_PHYS( (uint32_t)&ethHw_regMtpFifoData )
#define ETHHW_DMA_MTP_CTL_LO           (dmacHw_REG_CTL_LLP_SRC_EN | \
                                        dmacHw_REG_CTL_LLP_DST_EN | \
                                        dmacHw_REG_CTL_DMS_2 | \
                                        dmacHw_REG_CTL_TTFC_MP_DMAC | \
                                        dmacHw_REG_CTL_DST_MSIZE_8 | \
                                        dmacHw_REG_CTL_SRC_TR_WIDTH_64 | \
                                        dmacHw_REG_CTL_DST_TR_WIDTH_64)
#define ETHHW_DMA_MTP_CTL_HI           (0)
#define ETHHW_DMA_MTP_CFG_LO           (dmacHw_REG_CFG_LO_CH_PRIORITY_7)
#define ETHHW_DMA_MTP_CFG_HI           (dmacHw_DST_PERI_INTF(1) | \
                                        dmacHw_REG_CFG_HI_UPDATE_DST_STAT | \
                                        dmacHw_REG_CFG_HI_AHB_HPROT_1 | \
                                        dmacHw_REG_CFG_HI_FIFO_ENOUGH)
#define ETHHW_DMA_MTP_DSTAT            HW_IO_VIRT_TO_PHYS( (uint32_t)&ethHw_regMtpDmaStatus )

#define ETHHW_DMA_PTM_FIFO_ADDR        HW_IO_VIRT_TO_PHYS( (uint32_t)&ethHw_regPtmFifoConfig )
#define ETHHW_DMA_PTM_CTL_LO           (dmacHw_REG_CTL_LLP_SRC_EN | \
                                        dmacHw_REG_CTL_LLP_DST_EN | \
                                        dmacHw_REG_CTL_SMS_2 | \
                                        dmacHw_REG_CTL_TTFC_PM_PERI | \
                                        dmacHw_REG_CTL_SRC_MSIZE_8 | \
                                        dmacHw_REG_CTL_SRC_TR_WIDTH_64 | \
                                        dmacHw_REG_CTL_DST_TR_WIDTH_64)
#define ETHHW_DMA_PTM_CTL_HI           (0)
#define ETHHW_DMA_PTM_CFG_LO           (dmacHw_REG_CFG_LO_CH_PRIORITY_7)
#define ETHHW_DMA_PTM_CFG_HI           (dmacHw_REG_CFG_HI_UPDATE_SRC_STAT | \
                                        dmacHw_REG_CFG_HI_AHB_HPROT_1 | \
                                        dmacHw_REG_CFG_HI_FIFO_ENOUGH)
#define ETHHW_DMA_PTM_SSTAT            HW_IO_VIRT_TO_PHYS( (uint32_t)&ethHw_regPtmDmaStatus )

#define ETHHW_DMA_BUF_LEN(descp)       (((descp)->stat1 >> 13) & 0x0000ffff)
#define ETHHW_DMA_CTL_INT_EN           dmacHw_REG_CTL_INT_EN
#define ETHHW_DMA_TRANSFER_DONE(descp) ((descp)->ctlHi & dmacHw_REG_CTL_DONE)

#define ETHHW_DMA_DESC_CREATE(descp, _sar, _dar, handlep, _ctlLo, _ctlHi) \
do { \
	(descp)->sar = (uint32_t)(_sar); \
	(descp)->dar = (uint32_t)(_dar); \
	(descp)->lli = (uint32_t)((handlep) + 1); \
	(descp)->ctlLo = (_ctlLo); \
	(descp)->ctlHi = (_ctlHi); \
	(descp)->stat1 = 0; \
	(descp)->stat2 = 0; \
	(descp)->rsvd = 0; \
	(descp)->rsvd_len = 0; \
} while (0);

#define ETHHW_DMA_DESC_CREATE_NEXT(descp, _sar, _dar, _lli, _ctlLo, _ctlHi) \
do { \
	(descp)->sar = (uint32_t)(_sar); \
	(descp)->dar = (uint32_t)(_dar); \
	(descp)->lli = (uint32_t)(_lli); \
	(descp)->ctlLo = (_ctlLo); \
	(descp)->ctlHi = (_ctlHi); \
	(descp)->stat1 = 0; \
	(descp)->stat2 = 0; \
	(descp)->rsvd = 0; \
	(descp)->rsvd_len = 0; \
} while (0);

#define ETHHW_DMA_DESC_RX_UPDATE(descp, _dar, _ctlHi) \
do { \
	(descp)->dar = (uint32_t)(_dar); \
	(descp)->ctlHi = (_ctlHi); \
	(descp)->stat1 = 0; \
} while (0);

#define ETHHW_DMA_DESC_WRAP(descp,_lli) \
do {\
	(descp)->lli = _lli; \
} while (0);

#define ETHHW_DMA_DESC_TX_DONE(descp) \
do {\
   (descp)->lli = 0; \
   (descp)->ctlLo |= ETHHW_DMA_CTL_INT_EN; \
} while (0);


typedef struct {
	uint32_t sar;
	uint32_t dar;
	uint32_t lli;
	uint32_t ctlLo;
	uint32_t ctlHi;
	uint32_t stat1;
	uint32_t stat2;
	uint32_t rsvd;
	uint32_t rsvd_len;
} ETHHW_DMA_DESC;


/* Force to 64-bit alignment (in this case, to a cache line) */
#define __ETHHW_DMA_ALIGN__   __attribute__ ((aligned (16)))

static inline int ethHw_dmaInit(int controller)
{
	dmacHw_DMA_ENABLE( controller );

	return 1;
}


static inline int ethHw_dmaConfig(int controller, int ch,
                                  uint32_t ctlHi, uint32_t ctlLo,
                                  uint32_t cfgHi, uint32_t cfgLo,
                                  uint32_t sstat, uint32_t dstat)
{
	/* Clear control and config registers first.  The dmacHw_SET_ APIs are not
	 *  consistent.  In some cases, the set value is or'ed, in others, it will
	 *  overwrite the register.  Use dmacHw_RESET_ to clear these or'ed
	 *  registers first.
	 */
	dmacHw_RESET_CONTROL_LO( controller, ch );
	dmacHw_RESET_CONTROL_HI( controller, ch );
	dmacHw_RESET_CONFIG_LO( controller, ch );
	dmacHw_RESET_CONFIG_HI( controller, ch );

	dmacHw_SET_CONTROL_LO( controller, ch, ctlLo );
	dmacHw_SET_CONTROL_HI( controller, ch, ctlHi );
	dmacHw_SET_CONFIG_LO( controller, ch, cfgLo );
	dmacHw_SET_CONFIG_HI( controller, ch, cfgHi );

	dmacHw_SET_SSTATAR( controller, ch, sstat );
	dmacHw_SET_DSTATAR( controller, ch, dstat );

	/* Clear interrupt status and disable interrupts */
	dmacHw_TRAN_INT_CLEAR( controller, ch );
	dmacHw_BLOCK_INT_CLEAR( controller, ch );
	dmacHw_STRAN_INT_CLEAR( controller, ch );
	dmacHw_DTRAN_INT_CLEAR( controller, ch );
	dmacHw_ERROR_INT_CLEAR( controller, ch );

	dmacHw_TRAN_INT_DISABLE( controller, ch );
	dmacHw_BLOCK_INT_DISABLE( controller, ch );
	dmacHw_STRAN_INT_DISABLE( controller, ch );
	dmacHw_DTRAN_INT_DISABLE( controller, ch );
	dmacHw_ERROR_INT_DISABLE( controller, ch );

	dmacHw_SET_AMBA_BURST_LEN( controller, ch, 8 );

	return 1;
}


static inline int ethHw_dmaTxConfig(int burst)
{
	ETHHW_REG_WRITE_MASKED( ethHw_regImpEnable, ETHHW_REG_IMP_ENABLE, ETHHW_REG_IMP_ENABLE );

	reg32_write( &ethHw_regMtpConfig,
			ETHHW_REG_MTP_CONFIG_ENABLE |
			ETHHW_REG_MTP_CONFIG_BURST( burst ) );

	return 1;
}


static inline int ethHw_dmaRxConfig(int burst, int block)
{
	reg32_write( &ethHw_regPtmConfig,
			ETHHW_REG_PTM_CONFIG_ENABLE |
			ETHHW_REG_PTM_CONFIG_BURST( burst ) |
			ETHHW_REG_PTM_CONFIG_BLOCK( block ) );

	return 1;
}


static inline int ethHw_dmaEnable(int controller, int ch, void *descp)
{
	dmacHw_SET_LLP( controller, ch, descp );
	dmacHw_DMA_START( controller, ch );

	return 1;
}


static inline int ethHw_dmaDisable(int controller, int ch)
{
	dmacHw_DMA_STOP( controller, ch );

	return 1;
}


static inline int ethHw_dmaIrqEnable(int controller, int ch)
{
	dmacHw_BLOCK_INT_ENABLE( controller, ch );

	return 1;
}


static inline int ethHw_dmaIrqDisable(int controller, int ch)
{
	dmacHw_BLOCK_INT_DISABLE( controller, ch );

	return 1;
}


static inline int ethHw_dmaIrqClear(int controller, int ch)
{
	dmacHw_ERROR_INT_CLEAR( controller, ch );
	dmacHw_BLOCK_INT_CLEAR( controller, ch );

	return 1;
}


static inline int ethHw_dmaIsBusy(int controller, int ch)
{
	return CHANNEL_BUSY( controller, ch );
}

#endif
