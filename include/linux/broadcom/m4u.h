/*
 * include/linux/broadcom/m4u.h
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

#ifndef _LINUX_M4U_H_
#define _LINUX_M4U_H_

#include <linux/scatterlist.h>
#include <linux/uaccess.h>

#define INVALID_MMA		(0xFFFFFFFF)

struct m4u_device;

/**
 * struct m4u_region - defines a m4u map region in the given platform
 * @pa:		physical address start of the region to be mapped
 * @mma:	m4u virtual address which maps to physical address start
 *			of the region
 * @size:	Size of the region in bytes
 * @page_size: The page size to be used for mapping this region size has to
 * be an integer multiple of 8 times the page_size used for mapping.
 *
 * Provided by the board file.
 */
struct m4u_region {
	u32 pa;
	u32 mma;
	u32 size;
	u32 page_size;
};

/**
 * struct m4u_platform_data - m4u configuration passed from the board file
 * @mma_begin:	lower limit of mma address possible. The page table will have
 * 				entries only from this address.
 * @mma_size:	Size of mma address space. This decides	the page table size
 * 				allocated.
 * @xfifo_size: Size of memory in bytes used to buffer xfifo values of m4u on
 * 				error.
 * @nr:			Number of m4u map regions to be initialized as part of m4u init
 * @regions:	Array of m4u map regions.
 *
 * Provided by the board file.
 * TODO: Add secure overlay window
 */
struct m4u_platform_data {
	u32 mma_begin;
	u32 mma_size;
	u32 xfifo_size;
	int nr;
	struct m4u_region *regions;
};

/**
 * g_mdev - m4u device context
 * This would be initialized with m4u context pointer if m4u initialization
 * was successful.
 *
 * Parameter to m4u exposed APIs.
 */
extern struct m4u_device *g_mdev;

/**
 * m4u_map_contiguous() - Create a m4u mapping for the requested physcially
 * contiguous buffer and return the mma address. Heaps which can allocate
 * physically contiguous memory should use this API to define static mappings
 * at boot time to avoid page table update overheads and size should be power
 * of two for lesser TLB thrashing.
 *
 * @mdev:	m4u device
 * @pa:		physical address start of the region to be mapped
 * @size:	Size in bytes of tbe buffer described by the sg_table
 * @align:	Alignment required for the mma start address in bytes
 *
 * Returns mma address of physical address base of the buffer if successfully
 * mapped. Returns INVALID_MMA if buffer region could not be mapped.
 */
u32 m4u_map_contiguous(struct m4u_device *mdev, u32 pa, u32 size, u32 align);

/**
 * m4u_map() - Create a m4u mapping for the requested scatter-gather table if
 * needed and return the mma address. If static mapping available for the
 * requested buffer, new mapping is not created.
 * Refcounting of mapped address is not done by the driver.
 *
 * @mdev:	m4u device
 * @sgt:	scatter-gather table describing the physical pages to be mapped
 * @size:	Size in bytes of tbe buffer described by the sg_table
 * @align:	Alignment required for the mma start address in bytes
 *
 * Returns mma address of physical address base of the buffer if successfully
 * mapped. Returns INVALID_MMA if buffer region could not be mapped.
 */
u32 m4u_map(struct m4u_device *mdev, struct sg_table *sgt, u32 size, u32 align);

/**
 * m4u_unmap() - Remove a previously added m4u map region
 *
 * @mdev:	m4u device
 * @mma:	mma address start of the buffer to be unmapped
 *
 * Returns 0 if successfully removed.
 * TODO: Do we need to validate the size as well?
 */
void m4u_unmap(struct m4u_device *mdev, u32 mma);

/* TODO: Add secure map and unmap APIs */

#endif /* _LINUX_M4U_H_ */

