/*****************************************************************************
* Copyright 2010 - 2011 Broadcom Corporation.  All rights reserved.
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

#ifndef ASM_ARM_ARCH_BCMHANA_DMA_MMAP_H
#define ASM_ARM_ARCH_BCMHANA_DMA_MMAP_H

#include <linux/kernel.h>
#include <linux/wait.h>
#include <linux/semaphore.h>
#include <linux/broadcom/dump-mem.h>
#include <linux/scatterlist.h>
#include <linux/dma-mapping.h>
#include <linux/mm.h>
#include <linux/vmalloc.h>
#include <linux/pagemap.h>

/* pages less than this size are better off not being DMA'd */
#define DMA_MMAP_MIN_SIZE    4096

typedef enum
{
   DMA_MMAP_DEV_ADDR_NOINC = 0, /* DO NOT increment the device address */
   DMA_MMAP_DEV_ADDR_INC,       /* increment the device address */
} DMA_MMAP_DEV_ADDR_MODE_T;

typedef enum
{
   DMA_MMAP_TYPE_NONE,      /* Not a valid setting */
   DMA_MMAP_TYPE_VMALLOC,   /* Memory came from vmalloc call */
   DMA_MMAP_TYPE_KMALLOC,   /* Memory came from kmalloc call */
   DMA_MMAP_TYPE_DMA,       /* Memory came from dma_alloc_xxx call */
   DMA_MMAP_TYPE_USER,      /* Memory came from user space. */
   DMA_MMAP_TYPE_IO,        /* Memory for I/O, registers */
} DMA_MMAP_TYPE_T;

typedef enum
{
    DMA_MMAP_DIRTIED,
    DMA_MMAP_CLEAN
} DMA_MMAP_DIRTIED_T;

/*
 * A segment represents a physically and virtually contiguous chunk of memory,
 * i.e. each segment can be DMA'd
 *
 * A user of the DMA code will add memory regions. Each region may need to be
 * represented by one or more segments
 */
typedef struct
{
   void          *virtAddr;   /* Virtual address used for this segment */
   dma_addr_t     physAddr;   /* Physical address this segment maps to */
   size_t         numBytes;   /* Size of the segment, in bytes */
} DMA_MMAP_SEGMENT_T;

/*
 * A region represents a virtually contiguous chunk of memory, which may be
 * made up of multiple segments
 */
typedef struct
{
    DMA_MMAP_TYPE_T         memType;
    void                   *virtAddr;
    size_t                  numBytes;

    /*
     * Each region (virtually contiguous) consists of one or more segments.
     * Each segment is virtually and physically contiguous
     */
    int                     numSegmentsUsed;
    int                     numSegmentsAllocated;
    DMA_MMAP_SEGMENT_T     *segment;

    /*
     * When a region corresponds to user memory, we need to lock all of the
     * pages down before we can figure out the physical addresses. The
     * lockedPage array contains the pages that were locked, and which
     * subsequently need to be unlocked once the memory is unmapped
     */
    unsigned                numLockedPages;
    struct page           **lockedPages;

} DMA_MMAP_REGION_T;

typedef struct
{
    int                     inUse; /* Is this mapping currently being used? */
    struct semaphore        lock;  /* Acquired when using this structure */
    enum dma_data_direction dir;   /* Direction this transfer is intended for */

    /*
     * In the event that we're mapping user memory, we need to know which task
     * the memory is for, so that we can obtain the correct mm locks.
     */

    struct task_struct     *userTask;

    int                     numRegionsUsed;
    int                     numRegionsAllocated;
    DMA_MMAP_REGION_T      *region;
} DMA_MMAP_CFG_T;

/*
 * Initializes a DMA_MMAP_CFG_T data structure
 */
extern int dma_mmap_init_map
(
   DMA_MMAP_CFG_T *memMap /* Stores state information about the map */
);

/*
 * Releases any memory currently being held by a memory mapping structure
 */
extern int dma_mmap_term_map
(
   DMA_MMAP_CFG_T *memMap /* Stores state information about the map */
);

/*
 * Dumps the contents of a memory map
 */
#define DMA_MMAP_DUMP_MAP(dumpDest, addr, memMap, maxBytes) \
   if (dumpDest) dma_mmap_dump_map(__func__, dumpDest, addr, memMap, maxBytes)
extern void dma_mmap_dump_map
(
   const char      *function,   /* Function doing the dumping       */
   DUMP_DEST        dumpDest,   /* 1 = use printk, 2 = use KNLLOG   */
   uint32_t         addr,       /* address to use for dumping       */
   DMA_MMAP_CFG_T  *memMap,     /* Memory map to dump               */
   size_t           maxBytes    /* max number of bytes to dump      */
);

/*
 * Looks at a memory address and categorizes it
 *
 * @return One of the values from the DMA_MMAP_TYPE_T enumeration
 */
extern DMA_MMAP_TYPE_T dma_mmap_mem_type
(
   void *addr
);

/*
 * Sets the process (aka userTask) associated with a mem map. This is
 * required if user-mode segments will be added to the mapping
 */
static inline void dma_mmap_set_user_task
(
   DMA_MMAP_CFG_T *memMap,
   struct task_struct *task
)
{
   memMap->userTask = task;
}

/*
 * Looks at a memory address and determines if we support DMA'ing to/from that
 * type of memory
 *
 * @return boolean -
 *               return value != 0 means dma supported
 *               return value == 0 means dma not supported
 */
extern int dma_mmap_dma_is_supported
(
   void *addr
);

/*
 * Initializes a memory map for use. Since this function acquires a sempaphore
 * within the memory map, it is VERY important that dma_mmap_unmap be called when
 * you're finished using the map
 */
extern int dma_mmap_start
(
   DMA_MMAP_CFG_T         *memMap, /* Stores state information about the map */
   enum dma_data_direction dir     /* Direction that the mapping will be going */
);

/*
 * Determines if the indicated memory map is in use (i.e. needs unmapping)
 */
extern int dma_mmap_in_use( DMA_MMAP_CFG_T *memMap );

/*
 * Adds a region of memory to a memory map. Each region is virtually
 * contiguous, but not necessarily physically contiguous
 *
 * @return     0 on success, error code otherwise
 */
extern int dma_mmap_add_region
(
   DMA_MMAP_CFG_T *memMap,  /* Stores state information about the map */
   void           *mem,     /* Virtual address that we want to get a map of */
   size_t          numBytes /* Number of bytes being mapped */
);

/*
 * Maps in a memory region such that it can be used for performing a DMA
 *
 * @return
 */
extern int dma_mmap_map
(
   DMA_MMAP_CFG_T         *memMap, /* Stores state information about the map */
   void                   *addr,   /* Virtual address that we want to get a map of */
   size_t                  count,  /* Number of bytes being mapped */
   enum dma_data_direction dir     /* Direction that the mapping will be going */
);

/*
 * Maps in a memory region such that it can be used for performing a DMA.
 *
 * @return
 */
extern int dma_mmap_unmap
(
   DMA_MMAP_CFG_T      *memMap, /* Stores state information about the map */
   DMA_MMAP_DIRTIED_T   dirtied /* non-zero if any of the pages were modified */
);

/*
 * Uses mempcy to copy a memory map rather than using DMA.
*/
extern void dma_mmap_memcpy
(
    DMA_MMAP_CFG_T  *memMap,    /* memory map */
    void            *mem        /* contiguous virtual memory */
);


/*
 * Lock the memory map
 */
static inline void dma_mmap_lock_map
(
   DMA_MMAP_CFG_T  *memMap
)
{
    down(&memMap->lock);
}

/*
 * Unlock the memory map
 */
static inline void dma_mmap_unlock_map
(
   DMA_MMAP_CFG_T  *memMap
)
{
    up(&memMap->lock);
}

/*
 * Walk through the regions and segments and calculate the total number of DMA
 * descriptors required
 *
 * Since the DMA MMAP driver has no knowledge of the DMA device and its
 * associated DMA descriptors, the user needs to register a callback that can
 * do the calculation
 *
 * This is meant to be used with dma_mmap_add_desc and the memory map lock
 * should be acquired before calling this routine. In fact only the DMA driver
 * should call this routine
 *
 * Calling of dma_mmap_calc_desc_cnt and dma_mmap_add_desc should be atomic
 */
extern int dma_mmap_calc_desc_cnt
(
   DMA_MMAP_CFG_T  *memMap,
   dma_addr_t       devPhysAddr,
   DMA_MMAP_DEV_ADDR_MODE_T addrMode,
   void            *data1,
   void            *data2,
   int            (*dma_calc_desc_cnt)(void       *data1,
                                       void       *data2,
                                       dma_addr_t  srcAddr,
                                       dma_addr_t  dstAddr,
                                       size_t      numBytes)
);

/*
 * Walk through the regions and segments and populate all DMA descriptors
 *
 * Since the DMA MMAP driver has no knowledge of the DMA device and its
 * associated DMA descriptors, the user needs to register a callback that can
 * do the descriptor population
 *
 * This is meant to be used with dma_mmap_calc_desc_cnt and the memory map lock
 * should be acquired before calling this routine. In fact only the DMA driver
 * should call this routine
 *
 * Calling of dma_mmap_calc_desc_cnt and dma_mmap_add_desc should be atomic
 */
extern int dma_mmap_add_desc
(
   DMA_MMAP_CFG_T  *memMap,
   dma_addr_t       devPhysAddr,
   DMA_MMAP_DEV_ADDR_MODE_T addrMode,
   void            *data1,
   void            *data2,
   int            (*dma_add_desc)(void       *data1,
                                  void       *data2,
                                  dma_addr_t  srcAddr,
                                  dma_addr_t  dstAddr,
                                  size_t      numBytes)
);

/*
 * Main init function to be called once in arch
 */
extern int dma_mmap_init(void);

#endif /* ASM_ARM_ARCH_BCMHANA_DMA_MMAP_H */
