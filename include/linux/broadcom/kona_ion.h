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

/****************************************************************
 * Heap IDs defined here should match the heaps registered in the
 * board file in kernel (and heap sizes in bootargs).
 ***************************************************************/

#ifdef __KERNEL__

#define ION_INVALID_HEAP_ID		(15)

#define ION_CARVEOUT0_SIZE		(16*1024*1024)

extern struct ion_device *idev;

extern unsigned int kona_ion_map_dma(struct ion_client *client,
		struct ion_handle *handle);

#endif

/**
 * DOC: Kona Ion Heap Types for user space to select at alloc time
 */
#define ION_DEFAULT_HEAP	(0xFFFF)

/**
 * DOC: Buffer property flags and masks
 */
#define ION_BUFFER_TYPE_OFFSET	(0)
#define ION_BUFFER_TYPE_MASK	(0x0F << ION_BUFFER_TYPE_OFFSET)
#define ION_BUFFER_TYPE_RASTER	(0x01 << ION_BUFFER_TYPE_OFFSET)
#define ION_BUFFER_TYPE_TILED	(0x02 << ION_BUFFER_TYPE_OFFSET)

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
 * struct ion_custom_property - metadata passed to/from userspace for
 * 		handle/property pair
 * @handle:	a handle
 * @value: value of property of the buffer held by handle
 * @mask: mask for the property being accessed
 *
 * For ION_IOC_CUSTOM_SET_PROP userspace populates the handle field with
 * the handle returned from ion alloc, data with the new property value
 * and mask with the bitmask specifying the property.
 *
 * Provided by userspace as an argument to the ioctl
 */
struct ion_custom_property {
	struct ion_handle *handle;
	unsigned int value;
	unsigned int mask;
};

/**
 * struct ion_custom_update_count - metadata passed to/from userspace for
 * 		handle/update_count pair
 * @handle:	a handle
 * @count: update counter value of the buffer held by handle
 *
 * For ION_IOC_CUSTOM_GET_UPDATE_COUNT userspace populates the handle
 * field with the handle returned from ion alloc. The count field is
 * is filled by ION with the global update counter of the buffer.
 *
 * Provided by userspace as an argument to the ioctl
 */
struct ion_custom_update_count {
	struct ion_handle *handle;
	unsigned int count;
};

/**
 * struct ion_custom_region_data - a region and handle passed to/from
 * the kernel
 * @handle:	a handle
 * @offset: offet from start of buffer
 * @len: size in bytes to be flushed
 */
struct ion_custom_region_data {
	struct ion_handle *handle;
	unsigned int offset;
	unsigned int len;
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
 * DOC: ION_IOC_CUSTOM_SET_PROP - set buffer property
 *
 * Takes an ion_custom_property struct with the handle field populated
 * with a valid opaque handle.  Sets the property of the buffer as set
 * by the property field.
 */
#define ION_IOC_CUSTOM_SET_PROP  	     (3)

/**
 * DOC: ION_IOC_CUSTOM_GET_PROP - get buffer property
 *
 * Takes an ion_custom_property struct with the handle field populated
 * with a valid opaque handle.  Returns the property of the buffer in
 * the property field.
 */
#define ION_IOC_CUSTOM_GET_PROP  	     (4)

/**
 * DOC: ION_IOC_CUSTOM_UPDATE - Notify ION that buffer is updated
 *
  * Takes valid opaque handle as the argument. The internal update counter
 * would be incremented.
 */
#define ION_IOC_CUSTOM_UPDATE            (5)

/**
 * DOC: ION_IOC_CUSTOM_GET_UPDATE_COUNT - Get the update count
 *
 * Takes an ion_custom_update_data struct with the handle field populated
 * with a valid opaque handle.  Returns the struct with the count field set
 * to the buffer internal update counter.
 */
#define ION_IOC_CUSTOM_GET_UPDATE_COUNT  (6)

/**
 * DOC: ION_IOC_CUSTOM_CACHE_FLUSH - Arm Cache flush a region of memory
 *
 * Takes an ion_custom_region_data struct with the handle field populated
 * with a valid opaque handle.  The region within the buffer also has to be
 * populated which needs to be flushed.
 */
#define ION_IOC_CUSTOM_CACHE_FLUSH		(7)

/**
 * DOC: ION_IOC_CUSTOM_CACHE_INVALIDATE - Arm Cache invalidate a region of
 * memory
 *
 * Takes an ion_custom_region_data struct with the handle field populated
 * with a valid opaque handle.  The region within the buffer also has to be
 * populated which needs to be flushed.
 */
#define ION_IOC_CUSTOM_CACHE_INVALIDATE  (8)


/**
 * DOC: ION_IOC_CUSTOM_TP - Do a kernel print - to trace the code
 *
 * Pass the trace point value to be printed as argument.
 */
#define ION_IOC_CUSTOM_TP                (20)

#endif /* _LINUX_KONA_ION_H */

