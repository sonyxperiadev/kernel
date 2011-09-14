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
 *    FILE:          chal_nand_dma_ll.c
 *
 *    DESCRIPTION:   NAND functions for setting up the DMA descriptor and
 *                   to perform cache management on the source / destination
 *                   buffer and DMA desctiptor.
 *
 *    WRITTEN BY:    cdoban@broadcom.com, Jun 22 2010.
**/

#include <chal/chal_nand.h>

/* low level standalone programs are using physical addresses */
#define PHYS_ADDR(virt)    (virt)

#ifdef CONFIG_ENABLE_MMU
#include <asm/arch/chal_utils.h>
#define CACHE_CLEAN_RANGE(virt,size)        dma_clean_range((virt),PHYS_ADDR(virt),(size));
#define CACHE_INVALIDATE_RANGE(virt,size)   dma_inv_range((virt),PHYS_ADDR(virt),(size));
#else
#define CACHE_CLEAN_RANGE(virt,size)
#define CACHE_INVALIDATE_RANGE(virt,size)
#endif

/*============================================================================*/
/* PRIVATE TYPES		*/


/*============================================================================*/
/* PRIVATE prototypes */


/*============================================================================*/
/* PRIVATE DATA		*/

/* Single entry, DMA descriptor for every bank */ 
static union {
   chal_nand_prd_entry_t prd_tbl[CHAL_NAND_BANKS_MAX];
   uint8_t byte[CHAL_NAND_BANKS_MAX*sizeof(chal_nand_prd_entry_t)];
} buf;

/*******************************************************************************
* PUBLIC FUNCTION IMPLEMENTATION
*******************************************************************************/

void* chal_nand_dma_setup_callback(uint8_t direction,
                                   uint8_t bank,
                                   void* virt,
                                   uint32_t size)
{
   /* initialize DMA descriptor */
   chal_nand_prd_addr_set (&buf.prd_tbl[bank], PHYS_ADDR(virt));
   chal_nand_prd_desc_set (&buf.prd_tbl[bank], bank, size, 0x1UL);

   /* clean cache for DMA descriptors */
   CACHE_CLEAN_RANGE (&buf, sizeof(buf));

   /* clean cache for source buffer */
   CACHE_CLEAN_RANGE (virt, size);

   if (direction == CHAL_NAND_DMA_FROM_DEVICE)
   {
      /* invalidate cache for destination buffer */
      CACHE_INVALIDATE_RANGE (virt, size);
   }

   /* return physical address of the DMA descriptor */
   return (PHYS_ADDR(&buf.prd_tbl[bank]));
}

