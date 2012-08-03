/*
 * include/linux/broadcom/kona_ion.h
 *
 * Copyright (C) 2011 Broadcom, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef _LINUX_KONA_ION_H
#define _LINUX_KONA_ION_H

#include <linux/types.h>
#include <linux/ion.h>

#define ION_INVALID_HEAP_ID	(15)

/****************************************************************
 * Heap IDs defined here should match the heaps registered in the
 * board file in kernel (and heap sizes in bootargs).
 ***************************************************************/

#ifdef __KERNEL__

#define ION_CARVEOUT0_SIZE		(16*1024*1024)

#endif

/**
 * DOC: Kona Ion Heap Types for user space to select at alloc time
 */
#define ION_DEFAULT_HEAP	(0xFFFF)


/**
 * DOC: Kona Ion Custom ioctls
 */

/**
 * struct ion_custom_dma_map_data - metadata passed to/from userspace for 
 * 		handle/dma_addr pair
 * @handle:	a handle
 * @dma_addr: dma addr for MM blocks to access buffer held by handle
 *
 * For ION_IOC_CUSTOM_DMA_MAP userspace populates the handle field with
 * the handle returned from ion alloc, and the kernel returns the dma
 * address in the dma_addr field.  It could be physical address for non-IOMMU
 * cases and IOMMU mapped addresses for M4U cases.
 *
 * Provided by userspace as an argument to the ioctl
 */
struct ion_custom_dma_map_data {
	struct ion_handle *handle;
	unsigned int dma_addr;
};

/**
 * DOC: ION_IOC_CUSTOM_DMA_MAP - get dma mapped address
 *
 * Takes an ion_custom_dma_map_data struct with the handle field populated 
 * with a valid opaque handle.  Returns the struct with the dma_addr field 
 * set to the M4U or physical address for the buffer held by the handle.
 */
#define ION_IOC_CUSTOM_DMA_MAP  	     (1)

/**
 * DOC: ION_IOC_CUSTOM_DMA_UNMAP - unmap dma mapped address
 *
 * Takes valid opaque handle as the argument. The dma mapping would be
 * unmapped. 
 */
#define ION_IOC_CUSTOM_DMA_UNMAP  	     (2)

/**
 * DOC: ION_IOC_CUSTOM_TP - Do a kernel print - to trace the code
 *
 * Pass the trace point value to be printed as argument.
 */
#define ION_IOC_CUSTOM_TP                (20)

#endif /* _LINUX_KONA_ION_H */

