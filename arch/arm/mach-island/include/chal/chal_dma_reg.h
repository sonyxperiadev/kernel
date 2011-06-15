/*****************************************************************************
*  Copyright 2001 - 2008 Broadcom Corporation.  All rights reserved.
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
/**
*
*  @file  chal_dma_reg.h
*
*  @brief cHAL DMA register definitions
*
* \note
*****************************************************************************/
#ifndef _CHAL_DMA_REG_H_
#define _CHAL_DMA_REG_H_

//#include <mach/csp/rdb/brcm_rdb_sysmap.h>

#ifdef __cplusplus
extern "C" {
#endif


#define	CHAL_DMA_REG_MAX_LOOP_COUNT                                        256
#define CHAL_DMA_DMAGO_SECURE                                              0x00000000
#define CHAL_DMA_DMAGO_OPEN                                                0x00000002
#define CHAL_DMA_DMAGO_CHANNEL_NUM_SHIFT                                   8


#define CHAL_DMA_REG_CHAN_STATUS_STOPPED                                   0x00000000
#define CHAL_DMA_REG_CHAN_STATUS_EXECUTING                                 0x00000001
#define CHAL_DMA_REG_CHAN_STATUS_CACHE_MISS                                0x00000002
#define CHAL_DMA_REG_CHAN_STATUS_UPDATE_PC                                 0x00000003
#define CHAL_DMA_REG_CHAN_STATUS_WAIT_FOR_EVENT                            0x00000004
#define CHAL_DMA_REG_CHAN_STATUS_AT_BARRIER                                0x00000005
#define CHAL_DMA_REG_CHAN_STATUS_Q_BUSY                                    0x00000006
#define CHAL_DMA_REG_CHAN_STATUS_WAIT_FOR_PERIPHERAL                       0x00000007
#define CHAL_DMA_REG_CHAN_STATUS_KILLING                                   0x00000008
#define CHAL_DMA_REG_CHAN_STATUS_COMPLETING                                0x00000009
#define CHAL_DMA_REG_CHAN_STATUS_FAULT_COMPLETING                          0x0000000E
#define CHAL_DMA_REG_CHAN_STATUS_FAULT                                     0x0000000F



#define CHAL_DMA_REG_CCR_SRC_OPEN                                          0x00000200
#define CHAL_DMA_REG_CCR_DST_OPEN                                          0x00800000

#ifdef __cplusplus
}
#endif

#endif /* _CHAL_DMA_REG_H_ */
