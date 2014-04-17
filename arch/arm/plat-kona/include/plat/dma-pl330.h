/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2010  Broadcom Corporation                                                        */
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

#ifndef __PLAT_DMA_H
#define __PLAT_DMA_H

#include <asm/types.h>
#include <linux/scatterlist.h>
#define MAX_CHAN_NAME_LENGTH	32

/* DMA direction control */
enum dma_direction {
	DMA_DIRECTION_MEM_TO_MEM = 0,
	DMA_DIRECTION_MEM_TO_DEV_FLOW_CTRL_DMAC = 1,
	DMA_DIRECTION_DEV_TO_MEM_FLOW_CTRL_DMAC = 2,
	DMA_DIRECTION_MEM_TO_DEV_FLOW_CTRL_PERI = 3,
	DMA_DIRECTION_DEV_TO_MEM_FLOW_CTRL_PERI = 4,
	DMA_DIRECTION_DEV_TO_DEV = 5	/* Invalid, unsupported */
};
#define DMA_DIRECTION_MASK	0x7

/* Channel configurations definition */

#define DMA_CFG_SRC_ADDR_FIXED			(0x0 << 0)
#define DMA_CFG_SRC_ADDR_INCREMENT		(0x1 << 0)

#define DMA_CFG_DST_ADDR_FIXED			(0x0 << 1)
#define DMA_CFG_DST_ADDR_INCREMENT		(0x1 << 1)

/* src and dest burst size is assumed to be same */
#define DMA_CFG_BURST_SIZE_SHIFT        2
#define DMA_CFG_BURST_SIZE_MASK         (0x7 << DMA_CFG_BURST_SIZE_SHIFT)

#define DMA_CFG_BURST_SIZE_1            (0x0 << DMA_CFG_BURST_SIZE_SHIFT)
#define DMA_CFG_BURST_SIZE_2            (0x1 << DMA_CFG_BURST_SIZE_SHIFT)
#define DMA_CFG_BURST_SIZE_4            (0x2 << DMA_CFG_BURST_SIZE_SHIFT)
#define DMA_CFG_BURST_SIZE_8            (0x3 << DMA_CFG_BURST_SIZE_SHIFT)
#define DMA_CFG_BURST_SIZE_16           (0x4 << DMA_CFG_BURST_SIZE_SHIFT)
#define DMA_CFG_BURST_SIZE_32           (0x5 << DMA_CFG_BURST_SIZE_SHIFT)
#define DMA_CFG_BURST_SIZE_64           (0x6 << DMA_CFG_BURST_SIZE_SHIFT)
#define DMA_CFG_BURST_SIZE_128          (0x7 << DMA_CFG_BURST_SIZE_SHIFT)

/* src and dest burst length is assumed to be same */
#define DMA_CFG_BURST_LENGTH_SHIFT		5
#define DMA_CFG_BURST_LENGTH_MASK		(0xF << DMA_CFG_BURST_LENGTH_SHIFT)

#define DMA_CFG_BURST_LENGTH_1          (0x0 << DMA_CFG_BURST_LENGTH_SHIFT)
#define DMA_CFG_BURST_LENGTH_2          (0x1 << DMA_CFG_BURST_LENGTH_SHIFT)
#define DMA_CFG_BURST_LENGTH_3          (0x2 << DMA_CFG_BURST_LENGTH_SHIFT)
#define DMA_CFG_BURST_LENGTH_4          (0x3 << DMA_CFG_BURST_LENGTH_SHIFT)
#define DMA_CFG_BURST_LENGTH_5          (0x4 << DMA_CFG_BURST_LENGTH_SHIFT)
#define DMA_CFG_BURST_LENGTH_6          (0x5 << DMA_CFG_BURST_LENGTH_SHIFT)
#define DMA_CFG_BURST_LENGTH_7          (0x6 << DMA_CFG_BURST_LENGTH_SHIFT)
#define DMA_CFG_BURST_LENGTH_8          (0x7 << DMA_CFG_BURST_LENGTH_SHIFT)
#define DMA_CFG_BURST_LENGTH_9          (0x8 << DMA_CFG_BURST_LENGTH_SHIFT)
#define DMA_CFG_BURST_LENGTH_10         (0x9 << DMA_CFG_BURST_LENGTH_SHIFT)
#define DMA_CFG_BURST_LENGTH_11         (0xA << DMA_CFG_BURST_LENGTH_SHIFT)
#define DMA_CFG_BURST_LENGTH_12         (0xB << DMA_CFG_BURST_LENGTH_SHIFT)
#define DMA_CFG_BURST_LENGTH_13         (0xC << DMA_CFG_BURST_LENGTH_SHIFT)
#define DMA_CFG_BURST_LENGTH_14         (0xD << DMA_CFG_BURST_LENGTH_SHIFT)
#define DMA_CFG_BURST_LENGTH_15         (0xE << DMA_CFG_BURST_LENGTH_SHIFT)
#define DMA_CFG_BURST_LENGTH_16         (0xF << DMA_CFG_BURST_LENGTH_SHIFT)

/* Peripheral request flush control */

#define FLUSHP_CTRL_START_SHIFT         9
#define PERIPHERAL_FLUSHP_START         (0x1 << FLUSHP_CTRL_START_SHIFT)
#define FLUSHP_CTRL_END_SHIFT           10
#define PERIPHERAL_FLUSHP_END           (0x1 << FLUSHP_CTRL_END_SHIFT)
#define PERIPHERAL_FLUSHP_DEFAULT       PERIPHERAL_FLUSHP_END

/* Unaligned transfer Handling for Peripheral requirements */

/* Peripheral generate single xfer requests for last burst-unaligned data */
#define DMA_PERI_END_SINGLE_REQ_MASK    11
#define DMA_PERI_END_SINGLE_REQ         (0x1 << DMA_PERI_END_SINGLE_REQ_MASK)
/* Peripheral always generate burst requests
 * Support modified burst transfers for size lesser than a burst.
 */
#define DMA_PERI_REQ_ALWAYS_BURST_MASK 12
#define DMA_PERI_REQ_ALWAYS_BURST      (0x1 << DMA_PERI_REQ_ALWAYS_BURST_MASK)


enum pl330_xfer_status {
	DMA_PL330_XFER_OK,
	DMA_PL330_XFER_ERR,
	DMA_PL330_XFER_ABORT,
};

struct dma_transfer_list {
	dma_addr_t srcaddr;	/* src address */
	dma_addr_t dstaddr;	/* dst address */
	unsigned int xfer_size;	/* In bytes */
	struct list_head next;	/* Next item */
};

/* Multiple sg list  */
struct dma_transfer_list_sg	{
	struct scatterlist *sgl;
	unsigned long sg_len;
	struct list_head next;
};

typedef void (*pl330_xfer_callback_t) (void *private_data,
				       enum pl330_xfer_status status);

int dma_request_chan(unsigned int *chan, const char *name);
int dma_free_chan(unsigned int chan);
int dma_map_peripheral(unsigned int chan, const char *peri_name);
int dma_unmap_peripheral(unsigned int chan);
int dma_setup_transfer(unsigned int chan, dma_addr_t s, dma_addr_t d,
		       unsigned int xfer_size, int ctrl, int cfg);
int dma_setup_transfer_sg(unsigned int chan,
			  struct scatterlist *sgl,
			  unsigned int sg_len,
			  dma_addr_t hw_addr, int control, int cfg);
int dma_setup_transfer_list(unsigned int chan, struct list_head *head,
			    int ctrl, int cfg);
int dma_setup_transfer_list_multi_sg(unsigned int chan,
			struct list_head *head, dma_addr_t hw_addr,
			int control, int cfg);
int dma_start_transfer(unsigned int chan);
int dma_stop_transfer(unsigned int chan);
int dma_shutdown_all_chan(void);
int dma_register_callback(unsigned int chan,
			  pl330_xfer_callback_t cb, void *pri);
int dma_free_callback(unsigned int chan);

#endif /* __PLAT_DMA_H */
