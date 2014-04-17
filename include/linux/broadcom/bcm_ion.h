/*
 * include/linux/broadcom/bcm_ion.h
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

#ifndef _LINUX_BCM_ION_H_
#define _LINUX_BCM_ION_H_

#include <linux/types.h>
#include <linux/ion.h>
#include <linux/device.h>

/****************************************************************
 * Heap IDs defined here should match the heaps registered in the
 * board file in kernel (and heap sizes in bootargs).
 ***************************************************************/

#ifdef __KERNEL__

#define ION_INVALID_HEAP_ID		(15)

extern struct ion_device *idev;

extern unsigned int bcm_ion_map_dma(struct ion_client *client,
		struct ion_handle *handle);

/**
 * struct bcm_ion_heap_reserve_data - defines the set of parameters used by
 * platform file to search for carveout or cma nodes in DT file.
 * This is populated by the platform file. ION platform driver reads this
 * structure to get the reserved base, size, status and dummy device to which
 * CMA was attached if pdata is not provided and DT file is used.
 * @type:	type of the heap from ion_heap_type enum
 *		- to identify carveout or CMA reserve to be done.
 * @name:	tries to match with DT node-name.
 * @base:	base of the range from where reserve to be attempted
 * @limit:	limit of the range from where reserve to be attempted
 * @size:	size of the heap in bytes if applicable
 * @status:	status of carveout/cma
 * @cma_dev:	dummy device used to attach cma.
 *
 */
struct bcm_ion_heap_reserve_data {
	enum ion_heap_type type;
	const char *name;
	ion_phys_addr_t base;
	ion_phys_addr_t limit;
	size_t size;
	int status;
	struct device cma_dev;
};

/**
 * bcm_ion_get_heap_reserve_data() - Get the info about carveout/cma during
 * bootup.
 * @data:	platform data specifying carveout/cma info
 * @name:	Name of the heap/DT node of which info is required.
 *
 * To be implemented by platform file
 */
int bcm_ion_get_heap_reserve_data(struct bcm_ion_heap_reserve_data **data,
		const char *name);

/* Decode the flags and get the heap mask to be used in the platform
 * for allocation
 */
unsigned int bcm_ion_get_heapmask(unsigned int flags);

#endif

/**
 * DOC: Broadcom Ion Interface Version
 */
#define BCM_ION_VERSION		(0x00010001)

/**
 * DOC: Broadcom Ion Heap Types for user space to select at alloc time
 */
#define ION_DEFAULT_HEAP	(0xFFFF)

/**
 * DOC: Broadcom Ion custom flags
 * Except the cache flags, other flag parameters are not handled directly by
 *	ION. Instead, helper APIs are used to convert them to appropriate
 *	heap masks as per heap configuration in the platform and standard
 *	ION API is used.
 * 31    : Heap mask selection from below flags
 * 30    : Secure Buffer
 * 29    : Address limit to 256 MB range
 * 28    : Faster allocation
 * 27-24 : Mapping
 *	24 : User Map needed
 *	25 : Kernel Map needed
 * 23-21 : Usage properties
 *	0  : Others
 *	1  : GL
 *	2  : Graphics
 *	3  : Multimedia
 *	4  : Camera
 * 20    : HW may write
 * 19-16 : Custom Cache ops
 *	16 : Writecombine mode
 *	17 : Writethrough mode
 *	18 : Writeback mode
 */
#define ION_FLAG_HEAP_SELECT  (1 << 31)
#define ION_FLAG_SECURE       (1 << 30)
#define ION_FLAG_256M         (1 << 29)
#define ION_FLAG_FAST_ALLOC   (1 << 28)

#define ION_FLAG_USER_MAP     (1 << 24)
#define ION_FLAG_KMAP         (2 << 24)

#define ION_FLAG_USAGE_OFFSET (21)
#define ION_FLAG_USAGE_MASK   (7 << ION_FLAG_USAGE_OFFSET)
#define ION_FLAG_OTHERS       (0 << ION_FLAG_USAGE_OFFSET)
#define ION_FLAG_GL           (1 << ION_FLAG_USAGE_OFFSET)
#define ION_FLAG_GRAPHICS     (2 << ION_FLAG_USAGE_OFFSET)
#define ION_FLAG_MULTIMEDIA   (3 << ION_FLAG_USAGE_OFFSET)
#define ION_FLAG_CAMERA       (4 << ION_FLAG_USAGE_OFFSET)

#define ION_FLAG_HWWR         (1 << 20) /* For buffers which may over-run */

#define ION_FLAG_ACP          (1 << 19) /* ACP allocation(WB) requested */
#define ION_FLAG_WRITEBACK    (1 << 18) /* Needs explicit cache flushes */
#define ION_FLAG_WRITETHROUGH (1 << 17) /* Needs explicit cache invalidates */
#define ION_FLAG_WRITECOMBINE (1 << 16)

/**
 * DOC: Buffer property flags and masks
 */
#define ION_BUFFER_TYPE_OFFSET	(0)
#define ION_BUFFER_TYPE_MASK	(0x0F << ION_BUFFER_TYPE_OFFSET)
#define ION_BUFFER_TYPE_RASTER	(0x01 << ION_BUFFER_TYPE_OFFSET)
#define ION_BUFFER_TYPE_TILED	(0x02 << ION_BUFFER_TYPE_OFFSET)

/**
 * DOC: Broadcom Ion Custom ioctls
 */

/**
 * struct ion_custom_dma_map_data - metadata passed to/from userspace for
 *         handle/dma_addr pair
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
 *         handle/property pair
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
 *         handle/update_count pair
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
 * struct ion_custom_config_data - metadata to be filled by kernel and
 *	passed to userspace for getting interface version and heap
 *	masks to be used for different requests
 *
 * @version: Version info of custom ioctls
 * @mask_secure: Heap mask to be used for secure buffers. All other
 *	heaps are ignored if secure buffer is requested.
 * @mask_256M: Heap mask which limits the iova/pa address to 256M for
 *	hardwares which has limitation of 28-bit addressing. It will
 *	not fallback to another heap outside this heap mask for such
 *	requests.
 * @mask_fast: Heap mask which will be OR-ed only for fast allocation
 *	requests. CMA heap allocation time is not predictable. For
 *	critical use-cases, it may have a separate carveout area. If
 *	allocation fails, it will fallback to slower heaps.
 * @mask_hwwr: Heap mask which will be OR-ed only for buffers which
 *	MM hardware may write (use carveout for non-iommu solution
 *	if memory corruption may happen). This would be needed only
 *	for non-iommu config to reduce possibility of  critical memory
 *	getting corrupted. If allocation fails, it will fallback to
 *	other heaps.
 * @reserved1-3: Reserved for future expansion
 *
 * Carveouts, CMAs, System heaps are normal order as faster allocation
 *	requests may want to try carveout first and dont want the
 *	carveout heap to be used up by normal buffers. It would be
 *	better to fill up carveout and cma before attempting system
 *	heap. No 256MB limit heaps come before 256MB limited heaps
 *	as requests with no limit can fallback to 256MB limited heap.
 *
 * Provided by userspace as an argument to the ioctl.
 */
struct ion_custom_config_data {
	unsigned int version;
	unsigned int mask_secure;
	unsigned int mask_256M;
	unsigned int mask_fast;
	unsigned int mask_hwwr;
	unsigned int reserved1;
	unsigned int reserved2;
	unsigned int reserved3;
};

/**
 * struct ion_custom_mt_get_mem - Used to get memory usage records for
 * the memtrack HAL
 *
 * @version:   Version of the API (driver will fill it)
 * @pid:       PID for which info is needed
 * @type:      Type of allocation
 * @sizes:     Pointer to array of sizes for type.flags
 * @num_sizes: Size of the above array
 */
struct ion_custom_mt_get_mem {
	unsigned int version;
	pid_t pid;
	unsigned int type;
	size_t *sizes;
	unsigned int num_sizes;
};

/**
 * DOC: ION_IOC_CUSTOM_DMA_MAP - get dma mapped address
 *
 * Takes an ion_custom_dma_map_data struct with the handle field populated
 * with a valid opaque handle.  Returns the struct with the dma_addr field
 * set to the M4U or physical address for the buffer held by the handle.
 */
#define ION_IOC_CUSTOM_DMA_MAP		(1)

/**
 * DOC: ION_IOC_CUSTOM_SET_PROP - set buffer property
 *
 * Takes an ion_custom_property struct with the handle field populated
 * with a valid opaque handle.  Sets the property of the buffer as set
 * by the property field.
 */
#define ION_IOC_CUSTOM_SET_PROP		(3)

/**
 * DOC: ION_IOC_CUSTOM_GET_PROP - get buffer property
 *
 * Takes an ion_custom_property struct with the handle field populated
 * with a valid opaque handle.  Returns the property of the buffer in
 * the property field.
 */
#define ION_IOC_CUSTOM_GET_PROP		(4)

/**
 * DOC: ION_IOC_CUSTOM_UPDATE - Notify ION that buffer is updated
 *
  * Takes valid opaque handle as the argument. The internal update counter
 * would be incremented.
 */
#define ION_IOC_CUSTOM_UPDATE		(5)

/**
 * DOC: ION_IOC_CUSTOM_GET_UPDATE_COUNT - Get the update count
 *
 * Takes an ion_custom_update_data struct with the handle field populated
 * with a valid opaque handle.  Returns the struct with the count field set
 * to the buffer internal update counter.
 */
#define ION_IOC_CUSTOM_GET_UPDATE_COUNT	(6)

/**
 * DOC: ION_IOC_CUSTOM_CACHE_CLEAN - Arm Cache clean a region of memory
 *
 * Takes an ion_custom_region_data struct with the handle field populated
 * with a valid opaque handle.  The region within the buffer also has to be
 * populated which needs to be cleaned.
 */
#define ION_IOC_CUSTOM_CACHE_CLEAN	(7)

/**
 * DOC: ION_IOC_CUSTOM_CACHE_INVALIDATE - Arm Cache invalidate a region of
 * memory
 *
 * Takes an ion_custom_region_data struct with the handle field populated
 * with a valid opaque handle.  The region within the buffer also has to be
 * populated which needs to be flushed.
 */
#define ION_IOC_CUSTOM_CACHE_INVALIDATE	(8)

/**
 * DOC: ION_IOC_CUSTOM_GET_CONFIG - Get platform configuration information
 *
 * Get interface version info and the heap ids configured in the platform.
 * The heap info can be used to convert the buffer request to appropriate
 * heap mask and passed to standard ION ioctl.
 *
 * Takes an ion_custom_config_data struct. The version and heap mask fields
 * has to be populated.
 */
#define ION_IOC_CUSTOM_GET_CONFIG	(9)

/**
 * DOC: ION_IOC_CUSTOM_MT_GET_MEM - Get the memory usage information for
 * the pid that is passed from memtrack HAL.
 *
 * Fill the ion_custom_mt_get_mem struct with records for each type, flag.
 *
 * User to fill the version, pid, sizes, num_sizes fields
 * Driver will cross-check version and num_sizes fields and fill up the
 * "sizes" array with sizes of each usage type/flag.
 * User needs to allocate enough memory for "sizes" field as specified in
 * the num_sizes.
 */
#define ION_IOC_CUSTOM_MT_GET_MEM	(10)

/**
 * DOC: ION_IOC_CUSTOM_TP - Do a kernel print - to trace the code
 *
 * Pass the trace point value to be printed as argument.
 */
#define ION_IOC_CUSTOM_TP		(20)

#endif /* _LINUX_BCM_ION_H_ */

