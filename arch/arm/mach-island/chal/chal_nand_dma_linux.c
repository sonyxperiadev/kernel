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
 *    PURPOSE:       NAND DMA setup for Linux
 *
 *    FILE:          chal_nand_dma_linux.c
 *
 *    DESCRIPTION:   NAND functions for setting up the DMA descriptor and
 *                   to perform cache management on the source / destination
 *                   buffer.
 *
 *    WRITTEN BY:    cdoban@broadcom.com, Jun 22 2010.
**/

#include <chal/chal_nand_dma.h>

#include <linux/version.h>
#include <linux/dma-mapping.h>
#include <linux/init.h>
#include <linux/module.h>
#include <asm/bug.h>
#include <asm/cacheflush.h>
#include <asm/types.h>
#include <mach/dma_mmap.h>

#if 0
#define DBG_PRINT(fmt, args...)     printk(fmt, ## args)
#else
#define DBG_PRINT(fmt, args...)
#endif

#define MAX_DESC	4	/* large enough for an 8K page at least */
static chal_nand_prd_entry_t *virt_prdp;
static dma_addr_t phys_prd;

static uint32_t nand_ll_dma_setup(chal_nand_prd_entry_t * virt_entry,
				  uint8_t bank,
				  dma_addr_t phys_addr,
				  uint32_t size, uint32_t eot)
{
	DBG_PRINT("%s\n", __func__);
	DBG_PRINT("%s virt_entry=%p, bank=%d, phys_addr=%x, size=%x, eot=%x\n",
		  __func__, virt_entry, bank, phys_addr, size, eot);

	/* initialize DMA descriptor */
	chal_nand_prd_addr_set(virt_entry, (void *)phys_addr);
	chal_nand_prd_desc_set(virt_entry, bank, size, eot);

	DBG_PRINT("%s virt_entry->addr = %p\n", __func__,
		  virt_entry->phys_addr);
	DBG_PRINT("%s virt_entry->desc = %x\n", __func__, virt_entry->desc);

	DBG_PRINT("%s phys_entry=%x, virt_entry=%p\n", __func__,
		  phys_prd + ((char *)virt_entry - (char *)virt_prdp),
		  virt_entry);

	return (phys_prd + ((char *)virt_entry - (char *)virt_prdp));
}

static void SyncCpuToDev
    (const void *virtAddr,
     dma_addr_t physAddr, size_t numBytes, int direction) {
	if (virtAddr) {
		dmac_map_area(virtAddr, numBytes, direction);
	}
	if (direction == DMA_FROM_DEVICE) {
		outer_inv_range(physAddr, physAddr + numBytes);
	} else {
		outer_clean_range(physAddr, physAddr + numBytes);
	}
}

static void SyncDevToCpu
    (const void *virtAddr,
     dma_addr_t physAddr, size_t numBytes, int direction) {
	if (direction != DMA_TO_DEVICE) {
		outer_inv_range(physAddr, physAddr + numBytes);
	}
	if (virtAddr) {
		dmac_unmap_area(virtAddr, numBytes, direction);
	}
}

void *chal_nand_dma_setup_callback(uint8_t direction,
				   uint8_t bank,
				   void *virtAddr, uint32_t numBytes)
{
	/* Convert virtual address into physical chain */
	void *prd_phys_addr;
	DMA_MMAP_TYPE_T memType = dma_mmap_mem_type(virtAddr);
	dma_addr_t physAddr;
	unsigned int offset;
	unsigned long addr = (unsigned long)virtAddr;

	DBG_PRINT("%s\n", __func__);

	/* Alocate memory for the DMA descriptor table; Only once */
	if (!virt_prdp) {
		virt_prdp =
		    dma_alloc_coherent(NULL,
				       MAX_DESC * sizeof(chal_nand_prd_entry_t),
				       &phys_prd, GFP_KERNEL);
		if (!virt_prdp) {
			printk("%s: Failed to kmalloc descriptor table\n",
			       __func__);
			return NULL;
		}
		DBG_PRINT("%s allocated virt_prdp = %p\n", __func__, virt_prdp);
	}

	offset = addr & ~PAGE_MASK;

	switch (memType) {
	case DMA_MMAP_TYPE_DMA:
		DBG_PRINT("%s: DMA_MEM_TYPE_DMA\n", __func__);
		physAddr = (vmalloc_to_pfn(virtAddr) << PAGE_SHIFT) + offset;
		DBG_PRINT("%s physAddr=%x virtAddr=%p\n", __func__, physAddr,
			  virtAddr);
		dma_sync_single_for_device(NULL, physAddr, numBytes, direction);
		dma_sync_single_for_cpu(NULL, physAddr, numBytes, direction);
		prd_phys_addr =
		    (void *)nand_ll_dma_setup(&virt_prdp[0], bank, physAddr,
					      numBytes, 1);
		break;

	case DMA_MMAP_TYPE_KMALLOC:
		DBG_PRINT("%s: DMA_MEM_TYPE_KMALLOC\n", __func__);
		physAddr = virt_to_phys(virtAddr);	/* one contiguous region for kmalloc memory */
		DBG_PRINT("%s physAddr=%x, virtAddr=%p\n", __func__, physAddr,
			  virtAddr);
		dma_sync_single_for_device(NULL, physAddr, numBytes, direction);
		dma_sync_single_for_cpu(NULL, physAddr, numBytes, direction);
		prd_phys_addr =
		    (void *)nand_ll_dma_setup(&virt_prdp[0], bank, physAddr,
					      numBytes, 1);
		break;

	case DMA_MMAP_TYPE_VMALLOC:
		{
			size_t firstPageOffset;
			size_t firstPageSize;
			size_t bytesRemaining;
			int prdidx = 0;

			DBG_PRINT("%s: DMA_MEM_TYPE_VMALLOC\n", __func__);
			/* vmalloc'd pages are not physically contiguous */

			/* Walk through the pages and figure out the physical addresses */

			firstPageOffset =
			    (unsigned long)virtAddr & (PAGE_SIZE - 1);
			firstPageSize = PAGE_SIZE - firstPageOffset;
			if (firstPageSize > numBytes) {
				firstPageSize = numBytes;
			}

			/* The first page might be partial */

			physAddr =
			    PFN_PHYS(vmalloc_to_pfn(virtAddr)) +
			    firstPageOffset;
			DBG_PRINT("%s physAddr=%x, virtAddr=%p\n", __func__,
				  physAddr, virtAddr);
			SyncCpuToDev(virtAddr, physAddr, firstPageSize,
				     direction);
			SyncDevToCpu(virtAddr, physAddr, firstPageSize,
				     direction);
			virtAddr += firstPageSize;
			bytesRemaining = numBytes - firstPageSize;

			prd_phys_addr =
			    (void *)nand_ll_dma_setup(&virt_prdp[prdidx++],
						      bank, physAddr,
						      firstPageSize,
						      bytesRemaining ? 0 : 1);

			while (bytesRemaining > 0) {
				size_t bytesThisPage =
				    (bytesRemaining >
				     PAGE_SIZE ? PAGE_SIZE : bytesRemaining);

				physAddr = PFN_PHYS(vmalloc_to_pfn(virtAddr));
				DBG_PRINT("%s physAddr=%x, virtAddr=%p\n",
					  __func__, physAddr, virtAddr);
				SyncCpuToDev(virtAddr, physAddr, firstPageSize,
					     direction);
				SyncDevToCpu(virtAddr, physAddr, firstPageSize,
					     direction);

				if (prdidx >= MAX_DESC)
					BUG();

				virtAddr += bytesThisPage;
				bytesRemaining -= bytesThisPage;

				nand_ll_dma_setup(&virt_prdp[prdidx++], bank,
						  physAddr, bytesThisPage,
						  bytesRemaining ? 0 : 1);

			}
			break;
		}

	default:
		BUG();		/* MEM_TYPE_USER not supported for this driver */
	}

	return prd_phys_addr;
}
