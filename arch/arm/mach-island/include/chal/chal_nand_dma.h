/*****************************************************************************
*  Copyright 2006 - 2010 Broadcom Corporation.  All rights reserved.
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2, available at
*  http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
*****************************************************************************/

/*****************************************************************************
 *    PURPOSE:       NAND DMA setup for low level standalone programs
 *
 *    FILE:          chal_nand_dma.h
 *
 *    DESCRIPTION:   NAND functions for setting up the DMA descriptor and
 *                   to perform cache management on the source / destination
 *                   buffer and DMA desctiptor.
 *
 *    WRITTEN BY:    cdoban@broadcom.com, Jun 22 2010.
**/

#ifndef __CHAL_NAND_DMA_H__
#define __CHAL_NAND_DMA_H__

#include <chal/chal_defs.h>
/*#include <csp/stdint.h>*/

/* DMA buffers and descriptor aligment */
#define CHAL_NAND_DMA_ALIGN      4

/* DMA descriptor entry */
typedef struct tag_chal_nand_prd_entry_t {
  void* 	   phys_addr;	/* SRC/DST physical address	*/
  uint32_t	desc;	      /* descriptor		*/
} chal_nand_prd_entry_t;

static inline void chal_nand_prd_addr_set (chal_nand_prd_entry_t *entry, void* phys)
{
  entry->phys_addr = phys;
}

static inline void chal_nand_prd_desc_set (chal_nand_prd_entry_t *entry,
                                           uint8_t bank,
                                           uint32_t size,
                                           uint8_t eot)
{
  entry->desc = (eot << 31) |
                ((bank & 0x7) << 28) |
                (size & ((0x1 << 22) - 1));
}

/* NAND DMA direction */
enum {
  CHAL_NAND_DMA_FROM_DEVICE,
  CHAL_NAND_DMA_TO_DEVICE
};

/*
 * Callback function that is setting up the DMA descriptor and is doing cache
 * maintenance for the source/destination buffer and the DMA descriptor.
 * The function returns the physical address of the DMA desctiptor
 */
typedef void* (*chal_nand_dma_setup_callback_t)(uint8_t direction,
                                                uint8_t bank,
                                                void* virt,
                                                uint32_t size);


void* chal_nand_dma_setup_callback (uint8_t direction,
                                    uint8_t bank,
                                    void* virt,
                                    uint32_t size);

#endif
