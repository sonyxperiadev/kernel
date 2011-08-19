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

#include <linux/module.h>
#include <linux/dma-mapping.h>
#include <linux/mm.h>
#include <linux/pfn.h>
#include <linux/hugetlb.h>
#include <linux/proc_fs.h>
#include <linux/version.h>
#include <linux/mm_types.h>
#include <linux/sched.h>

#include <asm/atomic.h>
#include <asm/memory.h>
#include <mach/dma_mmap.h>

#define MAX_PROC_BUF_SIZE    256
#define PROC_PARENT_DIR      "dma_mmap"
#define PROC_ENTRY_DEBUG     "debug"
#define PROC_ENTRY_MEM_TYPE  "memType"

/* flag to turn on debug prints */
static volatile int gDbg = 0;
#define DMA_MMAP_PRINT(fmt, args...) \
   do { if (gDbg) printk("%s: " fmt, __func__,  ## args); } while (0)

static atomic_t gDmaStatMemTypeKmalloc  = ATOMIC_INIT(0);
static atomic_t gDmaStatMemTypeVmalloc  = ATOMIC_INIT(0);
static atomic_t gDmaStatMemTypeUser     = ATOMIC_INIT(0);
static atomic_t gDmaStatMemTypeCoherent = ATOMIC_INIT(0);
static struct proc_dir_entry *gProcDir;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,34)

/*
 * CacheMaintenance
 *
 * This function is used to perform cache maintenance since we can't
 * use dma_sync_single_xxx (nor dma_cache_maintenance) when working
 * with vmalloc'd memory
 *
 * Since we want to DMA to/from vmalloc'd memory, and user memory, we need
 * some cache management routines which will work for this
 *
 * dma_map_single, and dma_sync_xxx only work with direct mapped kernel
 * memory (i.e. kernel globals or kmalloc'd memory)
 *
 * dma_cache_maint uses __pa to obtain the physical address from the virtual
 * address to manage the outer cache, which is why it won't work if
 * an outer cache is enabled
 */

static void CacheMaintenance
(
   const void  *virtAddr,
   dma_addr_t   physAddr,
   size_t       numBytes,
   int          direction
)
{
   const void *end = virtAddr + numBytes;

   switch (direction)
   {
      case DMA_FROM_DEVICE: /* invalidate only */
         if ( virtAddr ) dmac_inv_range(virtAddr, end);
         outer_inv_range( physAddr, physAddr + numBytes );
         break;

      case DMA_TO_DEVICE: /* writeback only */
         if ( virtAddr ) dmac_clean_range(virtAddr, end);
         outer_clean_range( physAddr, physAddr + numBytes );
         break;

      case DMA_BIDIRECTIONAL: /* writeback and invalidate */
         if ( virtAddr ) dmac_flush_range(virtAddr, end);
         outer_flush_range( physAddr, physAddr + numBytes );
         break;

      default:
         BUG();
   }
}

static inline void SyncCpuToDev
(
    const void  *virtAddr,
    dma_addr_t   physAddr,
    size_t       numBytes,
    int          direction
)
{
    CacheMaintenance( virtAddr, physAddr, numBytes, direction );
}

static inline void SyncDevToCpu
(
    const void  *virtAddr,
    dma_addr_t   physAddr,
    size_t       numBytes,
    int          direction
)
{
    CacheMaintenance( virtAddr, physAddr, numBytes, direction );
}

#else

/*
 * SyncCpuToDev and SyncDevToCpu
 *
 * These functions are offering similar functionality to
 * dma_sync_single_cpu_to_dev and dma_sync_single_dev_to_cpu
 * however, they've been recoded to allow virtual addresses which
 * do not come out of "direct" mapped memory.
 *
 * I like to think of SyncCpuToDev as transferring ownership of the memory
 * from the cpu to the device, and SyncDevToCpu as transferring ownership
 * back from the device to the cpu. This ownership should not be confused
 * with the independant direction of transfer (which is also to/from device).
 *
 * This function is used to perform cache maintenance since we can't
 * use dma_sync_single_xxx (nor dma_cache_maintenance) when working
 * with vmalloc'd memory
 *
 * Since we want to DMA to/from vmalloc'd memory, and user memory, we need
 * some cache management routines which will work for this
 *
 * dma_map_single, and dma_sync_xxx only work with direct mapped kernel
 * memory (i.e. kernel globals or kmalloc'd memory)
 */

static void SyncCpuToDev
(
    const void  *virtAddr,
    dma_addr_t   physAddr,
    size_t       numBytes,
    int          direction
)
{
    if ( virtAddr )
    {
        dmac_map_area( virtAddr, numBytes, direction );
    }
    if ( direction == DMA_FROM_DEVICE )
    {
        outer_inv_range( physAddr, physAddr + numBytes);
    }
    else
    {
        outer_clean_range( physAddr, physAddr + numBytes);
    }
}

static void SyncDevToCpu
(
    const void  *virtAddr,
    dma_addr_t   physAddr,
    size_t       numBytes,
    int          direction
)
{
    if ( direction != DMA_TO_DEVICE )
    {
        outer_inv_range( physAddr, physAddr + numBytes );
    }
    if ( virtAddr )
    {
        dmac_unmap_area( virtAddr, numBytes, direction );
    }
}

#endif /* LINUX_VERSION */

/*
 * Translates a virtual address into a PFN, by following the MMU tables
 *
 * This function is needed to deal with pages which are marked as VM_IO |
 * VM_PFNMAP, which don't have a related page structure
 *
 * The pages which are remapped using remap_pfn_range (the typical function
 * used by drivers to process mmap) creates pages like these
 *
 * This function is almost a copy of follow_phys, taken from mm/memory.c
 *
 * @return     0 on success, error code otherwise
 */
static int get_pfn(struct vm_area_struct *vma,
		   unsigned long address, unsigned long *pfnp)
{
   struct mm_struct *mm = vma->vm_mm;
   pgd_t *pgd;
   pud_t *pud;
   pmd_t *pmd;
   pte_t *ptep, pte;
   spinlock_t *ptl;

   if (!(vma->vm_flags & (VM_IO | VM_PFNMAP)))
      goto no_page_table;

   pgd = pgd_offset(mm, address);
   if (pgd_none(*pgd) || unlikely(pgd_bad(*pgd)))
      goto no_page_table;

   pud = pud_offset(pgd, address);
   if (pud_none(*pud) || unlikely(pud_bad(*pud)))
      goto no_page_table;

   pmd = pmd_offset(pud, address);
   if (pmd_none(*pmd) || unlikely(pmd_bad(*pmd)))
      goto no_page_table;

   /* We cannot handle huge page PFN maps. Luckily they don't exist */
   if (pmd_huge(*pmd))
      goto no_page_table;

   ptep = pte_offset_map_lock(mm, pmd, address, &ptl);
   if (!ptep)
      goto no_page_table;

   pte = *ptep;
   if (!pte_present(pte))
      goto unlock;

   *pfnp = pte_pfn(pte);
   pte_unmap_unlock(ptep, ptl);
   return 0;

unlock:
   pte_unmap_unlock(ptep, ptl);

no_page_table:
   return -EINVAL;
}

/*
 * Adds a segment of memory to a memory map. Each segment is both physically
 * and virtually contiguous
 *
 * @return     0 on success, error code otherwise.
 */
static int dma_mmap_add_segment
(
   DMA_MMAP_CFG_T    *memMap,    /* Stores state information about the map */
   DMA_MMAP_REGION_T *region,    /* Region that the segment belongs to */
   void              *virtAddr,  /* Virtual address of the segment being added */
   dma_addr_t         physAddr,  /* Physical address of the segment being added */
   size_t             numBytes   /* Number of bytes of the segment being added */
)
{
   DMA_MMAP_SEGMENT_T *segment;

   DMA_MMAP_PRINT("memMap:%p va:%p pa:0x%x #:%d\n",
         memMap, virtAddr, physAddr, numBytes);

   /* Sanity check */
   if (((unsigned long)virtAddr < (unsigned long)region->virtAddr) ||
       (((unsigned long)virtAddr + numBytes)) >
       ((unsigned long)region->virtAddr + region->numBytes))
   {
      printk(KERN_ERR "%s: virtAddr %p len %d is outside region @ %p len: %d\n",
            __func__, virtAddr, numBytes, region->virtAddr, region->numBytes);
      return -EINVAL;
   }

   /* there's already at least one segment in the region */
   if (region->numSegmentsUsed > 0)
   {
      DMA_MMAP_SEGMENT_T *prev_segment;
      /*
       * Check to see if this segment is physically contiguous with the
       * previous one
       */
      prev_segment = &region->segment[region->numSegmentsUsed - 1];

      if ((prev_segment->physAddr + prev_segment->numBytes ) == physAddr)
      {
         /* It is - just add on to the end */
         DMA_MMAP_PRINT("appending %d bytes to last segment\n", numBytes);
         prev_segment->numBytes += numBytes;
         return 0;
      }
   }

   /* Reallocate to hold more segments if required */
   if (region->numSegmentsUsed >= region->numSegmentsAllocated)
   {
      DMA_MMAP_SEGMENT_T *newSegment;
      size_t oldSize = region->numSegmentsAllocated * sizeof(*newSegment);
      int newAlloc = region->numSegmentsAllocated + 4;
      size_t newSize = newAlloc * sizeof(*newSegment);

      /* reallocate segment memory */
      if ((newSegment = kmalloc(newSize, GFP_KERNEL)) == NULL)
      {
         return -ENOMEM;
      }
      memcpy(newSegment, region->segment, oldSize);
      memset(&((uint8_t *)newSegment)[oldSize], 0, newSize - oldSize);

      /* free old segment memory */
      kfree(region->segment);

      region->numSegmentsAllocated = newAlloc;
      region->segment = newSegment;
   }

   segment = &region->segment[region->numSegmentsUsed];
   region->numSegmentsUsed++;

   segment->virtAddr = virtAddr;
   segment->physAddr = physAddr;
   segment->numBytes = numBytes;

   DMA_MMAP_PRINT("returning success\n");

   return 0;
}

/*
 * Initializes a DMA_MMAP_CFG_T data structure
 */
int dma_mmap_init_map
(
   DMA_MMAP_CFG_T *memMap /* Stores state information about the map */
)
{
   memset(memMap, 0, sizeof(*memMap));
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,37)
   init_MUTEX(&memMap->lock);
#else
   sema_init(&memMap->lock, 1);
#endif
   return 0;
}
EXPORT_SYMBOL(dma_mmap_init_map);

/*
 * Releases any memory currently being held by a memory mapping structure
 */
int dma_mmap_term_map
(
   DMA_MMAP_CFG_T *memMap /* Stores state information about the map */
)
{
   int regionIdx;

   down(&memMap->lock); /* Just being paranoid */

   /* Free up any allocated memory */
   for (regionIdx = 0; regionIdx < memMap->numRegionsAllocated; regionIdx++)
   {
      kfree(memMap->region[regionIdx].segment);
   }
   kfree(memMap->region);

   up(&memMap->lock);
   memset( memMap, 0, sizeof( *memMap ));

   return 0;
}
EXPORT_SYMBOL(dma_mmap_term_map);

/*
 * Dumps the contents of a memory map
 */
void dma_mmap_dump_map
(
   const char      *function,   /* Function doing the dumping       */
   DUMP_DEST        dumpDest,   /* 1 = use printk, 2 = use KNLLOG   */
   uint32_t         addr,       /* address to use for dumping       */
   DMA_MMAP_CFG_T  *memMap,     /* Memory map to dump               */
   size_t           maxBytes    /* max number of bytes to dump      */
)
{
   int                  regionIdx;
   int                  segmentIdx;
   DMA_MMAP_REGION_T   *region;
   DMA_MMAP_SEGMENT_T  *segment;
   size_t               bytesRemaining = maxBytes;

   for (regionIdx = 0; regionIdx < memMap->numRegionsUsed; regionIdx++)
   {
      region = &memMap->region[regionIdx];

      for (segmentIdx = 0; segmentIdx < region->numSegmentsUsed; segmentIdx++)
      {
         size_t  bytesThisSegment;
         void   *ptr;

         segment = &region->segment[segmentIdx];

         bytesThisSegment = segment->numBytes;
         if (bytesThisSegment > bytesRemaining)
         {
            bytesThisSegment = bytesRemaining;
         }

         /* get a snap shot of the memory */
         if ((ptr = ioremap(segment->physAddr, bytesThisSegment)) == NULL)
         {
            printk(KERN_ERR "%s: ioremap(0x%08x, %d) failed\n", __func__,
                  segment->physAddr, bytesThisSegment);
            return;
         }
         dump_mem(function, dumpDest, addr, ptr, bytesThisSegment);
         iounmap(ptr);

         bytesRemaining -= bytesThisSegment;
         if (bytesRemaining <= 0)
         {
            return;
         }
         addr += bytesThisSegment;
      }
   }
}
EXPORT_SYMBOL(dma_mmap_dump_map);

/*
 * Looks at a memory address and categorizes it
 *
 * @return One of the values from the DMA_MMAP_TYPE_T enumeration
 */
DMA_MMAP_TYPE_T dma_mmap_mem_type
(
   void *addr
)
{
   unsigned long addrVal = (unsigned long)addr;

   if (addrVal >= CONSISTENT_BASE)
   {
      /* NOTE: DMA virtual memory space starts at 0xFFxxxxxx */
      /* dma_alloc_xxx pages are physically and virtually contiguous */
      return DMA_MMAP_TYPE_DMA;
   }

   if (addrVal >= VMALLOC_END)
   {
      /*
       * Addresses between VMALLOC_END and the beginning of the DMA virtual
       * address could be considered to be I/O space. Right now, nobody cares
       * about this particular classification, so we ignore it
       */
      return DMA_MMAP_TYPE_IO;
   }

   if (is_vmalloc_addr(addr))
   {
      /*
       * Address comes from the vmalloc'd region. Pages are virtually
       * contiguous but NOT physically contiguous
       */
      return DMA_MMAP_TYPE_VMALLOC;
   }

   if (addrVal >= PAGE_OFFSET)
   {
      /*
       * PAGE_OFFSET is typically 0xC0000000
       * kmalloc'd pages are physically contiguous
       */
      return DMA_MMAP_TYPE_KMALLOC;
   }

   if (addrVal >= TASK_SIZE)
   {
      /* The memory in this range includes global memory from loadable modules.
       * This memory is allocated using the same mechanisms as vmalloc, so we
       * treat it as vmalloc'd memory for DMA purposes.
       */
      return DMA_MMAP_TYPE_VMALLOC;
   }

   return DMA_MMAP_TYPE_USER;
}
EXPORT_SYMBOL(dma_mmap_mem_type);

/*
 * Looks at a memory address and determines if we support DMA'ing to/from that
 * type of memory
 *
 * @return boolean -
 *               return value != 0 means dma supported
 *               return value == 0 means dma not supported
 */
int dma_mmap_dma_is_supported
(
   void *addr
)
{
   DMA_MMAP_TYPE_T memType = dma_mmap_mem_type(addr);

    return (memType == DMA_MMAP_TYPE_DMA)
        || (memType == DMA_MMAP_TYPE_KMALLOC)
        || (memType == DMA_MMAP_TYPE_VMALLOC)
        || (memType == DMA_MMAP_TYPE_USER);
}
EXPORT_SYMBOL(dma_mmap_dma_is_supported);

/*
 * Initializes a memory map for use
 */
int dma_mmap_start
(
   DMA_MMAP_CFG_T         *memMap, /* Stores state information about the map */
   enum dma_data_direction dir     /* Direction that the mapping will be going */
)
{
   int     rc;

   down(&memMap->lock);

   DMA_MMAP_PRINT("memMap: %p\n", memMap);

   if (memMap->inUse)
   {
      printk(KERN_ERR "%s: memory map %p is already being used\n",
            __func__, memMap);
      rc = -EBUSY;
      goto out;
   }

   memMap->inUse = 1;
   memMap->dir = dir;
   memMap->numRegionsUsed = 0;

   rc = 0;

out:
   DMA_MMAP_PRINT("returning %d\n", rc);
   up(&memMap->lock);

   return rc;
}
EXPORT_SYMBOL(dma_mmap_start);

/*
 * Determines if the indicated memory map is in use (i.e. needs unmapping)
 */
int dma_mmap_in_use( DMA_MMAP_CFG_T *memMap )
{
    return memMap->inUse;
}
EXPORT_SYMBOL( dma_mmap_in_use );

/*
 * Helper routine which is used to add user pages which don't have a page
 * struct
 *
 * @return     0 on success, error code otherwise
 */
static int dma_mmap_add_user_region
(
   DMA_MMAP_CFG_T      *memMap,
   struct mm_struct    *userMM,
   DMA_MMAP_REGION_T   *region
)
{
   int                     rc;
   size_t                  firstPageOffset;
   size_t                  firstPageSize;
   unsigned long           pfn;
   unsigned long           virtAddr = (unsigned long)region->virtAddr;
   size_t                  bytesRemaining;
   struct vm_area_struct  *vma;

   down_read(&userMM->mmap_sem);
   if ((vma = find_vma(userMM, virtAddr)) == NULL)
   {
      printk(KERN_ERR "%s: find_vma failed for virtAddr 0x%08lx\n",
             __func__, virtAddr);
      rc = -EINVAL;
      goto out_up;
   }

   if ((virtAddr + region->numBytes) > vma->vm_end)
   {
      printk(KERN_ERR "%s: vma only covers 0x%08lx - 0x%08lx, region is "
            "0x%08lx len %d\n", __func__, vma->vm_start, vma->vm_end,
            virtAddr, region->numBytes);
      rc = -EINVAL;
      goto out_up;
   }

   /*
    * The first page may be partial
    */
   firstPageOffset = virtAddr & (PAGE_SIZE - 1);
   firstPageSize = PAGE_SIZE - firstPageOffset;
   if (firstPageSize > region->numBytes)
   {
      firstPageSize = region->numBytes;
   }

   if ((rc = get_pfn(vma, virtAddr, &pfn)) < 0)
   {
      printk(KERN_ERR "%s: get_pfn failed for virtAddr 0x%08lx\n", __func__,
            virtAddr);
      goto out_up;
   }

   rc = dma_mmap_add_segment(memMap,
                             region,
                             (void *)virtAddr,
                             PFN_PHYS(pfn) + firstPageOffset,
                             firstPageSize);
   if (rc < 0)
   {
      goto out_up;
   }

   virtAddr += firstPageSize;
   bytesRemaining = region->numBytes - firstPageSize;

   while (bytesRemaining > 0)
   {
      size_t bytesThisPage = (bytesRemaining > PAGE_SIZE ?
            PAGE_SIZE : bytesRemaining);

      if ((rc = get_pfn(vma, virtAddr, &pfn)) < 0)
      {
         printk(KERN_ERR "%s: get_pfn failed for virtAddr 0x%08lx\n",
               __func__, virtAddr);
         goto out_up;
      }

      rc = dma_mmap_add_segment(memMap,
                                region,
                                (void *)virtAddr,
                                PFN_PHYS(pfn),
                                bytesThisPage);
      if (rc < 0)
      {
         break;
      }

      virtAddr += bytesThisPage;
      bytesRemaining -= bytesThisPage;
   }

out_up:
   up_read(&userMM->mmap_sem);
   return rc;
}

/*
 * Adds a region of memory to a memory map. Each region is virtually
 * contiguous, but not necessarily physically contiguous
 *
 * @return     0 on success, error code otherwise
 */
int dma_mmap_add_region
(
   DMA_MMAP_CFG_T *memMap,  /* Stores state information about the map */
   void           *mem,     /* Virtual address that we want to get a map of */
   size_t          numBytes /* Number of bytes being mapped */
)
{
   unsigned long addr = (unsigned long)mem;
   unsigned int offset;
   int rc = 0;
   DMA_MMAP_REGION_T *region;
   dma_addr_t physAddr;

   down(&memMap->lock);

   DMA_MMAP_PRINT("memMap:%p va:%p #:%d\n", memMap, mem, numBytes);

   if (!memMap->inUse)
   {
      printk(KERN_ERR "%s: Make sure you call dma_map_start first\n",
            __func__);
      rc = -EINVAL;
      goto out;
   }

   /* Reallocate to hold more regions */
   if (memMap->numRegionsUsed >= memMap->numRegionsAllocated)
   {
      DMA_MMAP_REGION_T *newRegion;
      size_t oldSize = memMap->numRegionsAllocated * sizeof(*newRegion);
      int newAlloc = memMap->numRegionsAllocated + 4;
      size_t newSize = newAlloc * sizeof(*newRegion);

      if ((newRegion = kmalloc( newSize, GFP_KERNEL )) == NULL)
      {
         rc = -ENOMEM;
         goto out;
      }

      memcpy(newRegion, memMap->region, oldSize);
      memset(&((uint8_t *)newRegion)[oldSize], 0, newSize - oldSize);
      kfree(memMap->region);
      memMap->numRegionsAllocated = newAlloc;
      memMap->region = newRegion;
   }

   region = &memMap->region[memMap->numRegionsUsed];
   memMap->numRegionsUsed++;

   offset = addr & ~PAGE_MASK;

   region->memType = dma_mmap_mem_type(mem);
   region->virtAddr = mem;
   region->numBytes = numBytes;
   region->numSegmentsUsed = 0;
   region->pagelist = NULL;

   switch (region->memType)
   {
      case DMA_MMAP_TYPE_VMALLOC:
      {
         size_t firstPageOffset;
         size_t firstPageSize;
         uint8_t *virtAddr = region->virtAddr;
         size_t bytesRemaining;

         /* vmalloc'd pages are not physically contiguous */
         atomic_inc(&gDmaStatMemTypeVmalloc);

         firstPageOffset = (unsigned long)region->virtAddr & (PAGE_SIZE - 1);
         firstPageSize = PAGE_SIZE - firstPageOffset;
         if (firstPageSize > region->numBytes)
         {
            firstPageSize = region->numBytes;
         }

         /* The first page might be partial */
         physAddr = PFN_PHYS( vmalloc_to_pfn(virtAddr)) + firstPageOffset;
         SyncCpuToDev(virtAddr, physAddr, firstPageSize, memMap->dir);
         rc = dma_mmap_add_segment(memMap,
                                   region,
                                   virtAddr,
                                   physAddr,
                                   firstPageSize);
         if (rc != 0)
         {
            break;
         }

         virtAddr += firstPageSize;
         bytesRemaining = region->numBytes - firstPageSize;

         /* Walk through the pages and figure out the physical addresses */
         while (bytesRemaining > 0)
         {
            size_t bytesThisPage = (bytesRemaining > PAGE_SIZE ?
                                    PAGE_SIZE : bytesRemaining);

            physAddr = PFN_PHYS(vmalloc_to_pfn(virtAddr));
            SyncCpuToDev(virtAddr, physAddr, bytesThisPage, memMap->dir);
            rc = dma_mmap_add_segment(memMap,
                                      region,
                                      virtAddr,
                                      physAddr,
                                      bytesThisPage);
            if (rc < 0)
            {
               break;
            }

            virtAddr += bytesThisPage;
            bytesRemaining -= bytesThisPage;
         }
         break;
      }

      case DMA_MMAP_TYPE_KMALLOC:
      {
         atomic_inc(&gDmaStatMemTypeKmalloc);

         /*
          * kmalloc'd pages are physically contiguous, so they'll have exactly
          * one segment
          *
          * Since dma_map_single does absolutely nothing on the ARM, we
          * use the dma_sync_single_for_device/cpu instead.
          */

         physAddr = virt_to_phys(mem);
         dma_sync_single_for_device(NULL, physAddr, numBytes, memMap->dir);
         rc = dma_mmap_add_segment(memMap, region, mem, physAddr, numBytes);
         break;
      }

      case DMA_MMAP_TYPE_DMA:
      {
         /* dma_alloc_xxx pages are physically contiguous */
         atomic_inc(&gDmaStatMemTypeCoherent);
         physAddr = (vmalloc_to_pfn(mem) << PAGE_SHIFT) + offset;

         dma_sync_single_for_device(NULL, physAddr, numBytes, memMap->dir);
         rc = dma_mmap_add_segment(memMap, region, mem, physAddr, numBytes);
         break;
      }

      case DMA_MMAP_TYPE_USER:
      {
         DMA_MMAP_PAGELIST_T *pagelist;

         atomic_inc(&gDmaStatMemTypeUser);

         /*
          * If the pages are user pages, then the dma_mmap_set_pagelist
          * function must have been previously called.
          */
         pagelist = memMap->pagelist;

         if (pagelist == NULL)
         {
            printk(KERN_ERR "%s: must call dma_mmap_set_pagelist when"
               " using user-mode memory\n", __func__);
            return -EINVAL;
         }

         if (pagelist->numLockedPages == 0)
         {
            /*
             * get_user_pages will fail on pages which have no page struct.
             * Pages created by remap_pfn_range are like this, so we figure
             * out the PFN for each page
             */
            if ((rc = dma_mmap_add_user_region(memMap, pagelist->mm, region)) == 0)
            {
               break;
            }

            printk(KERN_ERR "%s: get_pfn for address %p len "
               "%d failed: %d\n", __func__, region->virtAddr,
               region->numBytes, rc);
         }
         else
         {
            size_t firstPageOffset;
            size_t firstPageSize;
            struct page **pages = pagelist->pages;
            uint8_t *virtAddr = region->virtAddr;
            size_t bytesRemaining;
            int pageIdx;

            /* User pages need to be locked */
            firstPageOffset = (unsigned long)region->virtAddr & (PAGE_SIZE - 1);
            firstPageSize   = PAGE_SIZE - firstPageOffset;
            if (firstPageSize > region->numBytes)
            {
               firstPageSize = region->numBytes;
            }

            rc = 0; /* Since get_user_pages returns +ve number */

            /*
             * We've locked the user pages. Now we need to walk them and figure
             * out the physical addresses
             */

            /*
             * The first page may be partial
             */

            /*
             * L1 and L2 cache maintenance:
             *
             * We need to make sure that we take care of both L1 and L2 caches.
             * The L1 cache is handled by calling flush_dcache_page(). The L2
             * cached is handled when we pass the ownership of the DMA buffer
             * to the DMA engine (dma_sync_single_for_device()). Depending on
             * the direction of the transfer, the DMA buffers are either
             * invalidated (read) or flushed (write). When the DMA transfer is
             * done, we need to transfer the ownership back to the CPU.
             *
             * Note: When CONFIG_HIGHMEM is enabled, we can't use
             * dma_sync_single_for_device. So we use SyncCpuToDev for the L2
             * portion .
             */
            flush_dcache_page(pages[0]);

            physAddr = PFN_PHYS(page_to_pfn(pages[0])) + firstPageOffset;

            SyncCpuToDev( NULL, physAddr, firstPageSize, memMap->dir );

            rc = dma_mmap_add_segment(memMap,
                                      region,
                                      virtAddr,
                                      physAddr,
                                      firstPageSize);
            if (rc < 0)
            {
               break;
            }

            virtAddr += firstPageSize;
            bytesRemaining = region->numBytes - firstPageSize;

            for (pageIdx = 1; pageIdx < pagelist->numLockedPages; pageIdx++)
            {
               size_t bytesThisPage = (bytesRemaining > PAGE_SIZE ?
                                       PAGE_SIZE : bytesRemaining);

               DMA_MMAP_PRINT("pageIdx:%d pages[pageIdx]=%p pfn=%lu phys=%u\n",
                              pageIdx,
                              pages[pageIdx],
                              page_to_pfn(pages[pageIdx]),
                              PFN_PHYS(page_to_pfn(pages[pageIdx])));

               flush_dcache_page(pages[pageIdx]);

               physAddr = PFN_PHYS(page_to_pfn( pages[pageIdx]));
               SyncCpuToDev( NULL, physAddr, bytesThisPage,
                             memMap->dir );

               rc = dma_mmap_add_segment(memMap,
                                         region,
                                         virtAddr,
                                         physAddr,
                                         bytesThisPage);
               if (rc < 0)
               {
                  break;
               }

               virtAddr += bytesThisPage;
               bytesRemaining -= bytesThisPage;
            }
         }
         break;
      }

      default:
      {
         printk(KERN_ERR "%s: Unsupported memory type: %d\n", __func__,
               region->memType);
         rc = -EINVAL;
         break;
      }
   }

   if (rc != 0)
   {
      memMap->numRegionsUsed--;
   }

out:
   DMA_MMAP_PRINT("returning %d\n", rc);
   up(&memMap->lock);
   return rc;
}
EXPORT_SYMBOL(dma_mmap_add_region);

/*
 * Maps in a memory region such that it can be used for performing a DMA
 *
 * @return
 */
int dma_mmap_map
(
   DMA_MMAP_CFG_T         *memMap,   /* Stores state information about the map */
   void                   *addr,     /* Virtual address that we want to get a map of */
   size_t                  numBytes, /* Number of bytes being mapped */
   enum dma_data_direction dir       /* Direction that the mapping will be going */
)
{
   int rc;

   if ((rc = dma_mmap_start(memMap, dir)) == 0)
   {
      if (( rc = dma_mmap_add_region(memMap, addr, numBytes)) < 0)
      {
         /*
          * Since the add fails, this function will fail, and the caller won't
          * call unmap, so we need to do it here
          */
         dma_mmap_unmap(memMap, DMA_MMAP_CLEAN );
      }
   }

   return rc;
}
EXPORT_SYMBOL(dma_mmap_map);

/*
 * Unmaps a memory region that has been previous mapped for performing DMA.
 *
 * @return
 */
int dma_mmap_unmap
(
   DMA_MMAP_CFG_T      *memMap, /* Stores state information about the map */
   DMA_MMAP_DIRTIED_T   dirtied /* non-zero if any of the pages were modified */
)
{
   int rc = 0;
   int regionIdx;
   int segmentIdx;
   DMA_MMAP_REGION_T *region;
   DMA_MMAP_SEGMENT_T *segment;

   (void)dirtied;   /* Hmmm, not used */

   down(&memMap->lock);

   for (regionIdx = 0; regionIdx < memMap->numRegionsUsed; regionIdx++)
   {
      region = &memMap->region[regionIdx];

      switch (region->memType)
      {
         case DMA_MMAP_TYPE_VMALLOC:
         {
            for ( segmentIdx = 0; segmentIdx < region->numSegmentsUsed; segmentIdx++ )
            {
                segment = &region->segment[segmentIdx];

                SyncDevToCpu( segment->virtAddr,
                              segment->physAddr,
                              segment->numBytes,
                              memMap->dir);
            }
            break;
         }

         case DMA_MMAP_TYPE_USER:
         {
            /*
             * For user mappings, we've already calculated the physAddr
             * in dma_mmap_add_region.
             *
             * We use SyncDevToCpu since dma_sync_device_to_cpu doesn't
             * work on high memory pages when CONFIG_HIGHMEM is set.
             */

            for ( segmentIdx = 0; segmentIdx < region->numSegmentsUsed; segmentIdx++ )
            {
               segment = &region->segment[segmentIdx];

               SyncDevToCpu( NULL,
                             segment->physAddr,
                             segment->numBytes,
                             memMap->dir );
            }
            break;
         }

         case DMA_MMAP_TYPE_DMA:
         case DMA_MMAP_TYPE_KMALLOC:
         {
             BUG_ON( region->numSegmentsUsed == 1 );

            /*
             * On the ARM, dma_unmap_single does nothing, which is fine for
             * memory allocated using dma_alloc_xxx, but not for kmalloc,
             * so we use dma_sync_single_for_cpu instead, which invalidates
             * the cache
             */

            segment = &region->segment[0];

            dma_sync_single_for_cpu( NULL,
                                     segment->physAddr,
                                     segment->numBytes,
                                     memMap->dir);
            break;
         }

         default:
         {
            printk(KERN_ERR "%s: Unsupported memory type: %d\n", __func__,
                  region->memType);
            rc = -EINVAL;
            goto out;
         }
      }

      region->memType  = DMA_MMAP_TYPE_NONE;
      region->virtAddr = NULL;
      region->numBytes = 0;
      region->numSegmentsUsed = 0;
   }
   memMap->pagelist = NULL;
   memMap->numRegionsUsed = 0;
   memMap->inUse = 0;

   rc = 0;

out:
   up(&memMap->lock);
   return rc;
}
EXPORT_SYMBOL(dma_mmap_unmap);

/*
 * Copy a piece of memory, taking care to invalidate/flush as needed.
*/
static void dma_mmap_memcpy_page( struct page *page,
                                  void                     *mem_ptr,
                                  size_t                    offset,
                                  size_t                    num_bytes,
                                  enum dma_data_direction   dir )
{
   uint8_t    *kernel_addr;
   dma_addr_t  phys_addr;

   kernel_addr = kmap( page );
   kernel_addr += offset;

   phys_addr = PFN_PHYS( page_to_pfn( page )) + offset;

   if ( dir == DMA_TO_DEVICE )
   {
      /*
       * The userspace data has already been flushed out to the physical
       * memory by dma_mmap. We're now going to access the data using
       * the kernel virtual address, so we want to invalidate any cached
       * data which might be present.
       */

      SyncCpuToDev( kernel_addr, phys_addr, num_bytes, DMA_FROM_DEVICE );
      SyncDevToCpu( kernel_addr, phys_addr, num_bytes, DMA_FROM_DEVICE );

      memcpy( mem_ptr, kernel_addr, num_bytes );
   }
   else
   {
      memcpy( kernel_addr, mem_ptr, num_bytes );

      /*
       * We've copied a bunch of data into the kernel memory. We need to
       * flush it out to physical memory.
       */

      SyncCpuToDev( kernel_addr, phys_addr, num_bytes, DMA_TO_DEVICE );
      SyncDevToCpu( kernel_addr, phys_addr, num_bytes, DMA_TO_DEVICE );
   }

   kunmap( page );
}

/*
 * Walk through the regions and segments, and use the CPU to copy the data
 *
 * This is useful for working around silicon bugs in the DMA hardware when
 * the hardware needs certain alignments, etc.
 *
 * The direction of the copy is determined by the direction field stored
 * in the memory map. DMA_TO_DEVICE copies to 'mem', DMA_FROM_DEVICE copies
 * from 'mem'.
 *
 * It is assumed that 'mem' points to uncached memory.
 */

void dma_mmap_memcpy( DMA_MMAP_CFG_T *memMap, void *mem )
{
   DMA_MMAP_REGION_T      *region;
   int                     regionIdx;
   uint8_t                *memPtr = mem;

   /*
    * Walk through the regions and segments and copy each one individually.
    *
    * Remember, a region is virtually contigous, but not necessarily
    * physically contiguous.
    */

   for (regionIdx = 0; regionIdx < memMap->numRegionsUsed; regionIdx++)
   {
      region = &memMap->region[regionIdx];

      if ( region->memType == DMA_MMAP_TYPE_USER )
      {
         DMA_MMAP_PAGELIST_T *pagelist;
         size_t               firstPageOffset;
         size_t               firstPageSize;
         struct page        **pages;
         uint8_t             *virtAddr;
         size_t               bytesRemaining;
         int                  pageIdx;

         /*
          * Since user pages might not have corresponding kernel pages
          * (especially when CONFIG_HIGHMEM is enabled), we need to walk
          * the page list and use kmap/kunmap.
          *
          * For kernel direct memory, kmap returns the kernel direct mapping.
          * For high-memory, a new mapping is created.
          */

         if (( pagelist = memMap->pagelist ) == NULL )
         {
            printk( KERN_ERR "%s: must call dma_mmap_set_pagelist when"
                             " using user-mode memory\n", __func__ );
            return;
         }
         if ( pagelist->numLockedPages == 0 )
         {
            printk( KERN_ERR "%s: no locked pages???", __func__ );
            return;
         }

         /*
          * The first page may be partial (at either end)
          */

         pages = pagelist->pages;
         virtAddr = region->virtAddr;

         firstPageOffset = (unsigned long)virtAddr & (PAGE_SIZE - 1);
         firstPageSize   = PAGE_SIZE - firstPageOffset;
         if (firstPageSize > region->numBytes)
         {
            firstPageSize = region->numBytes;
         }

         dma_mmap_memcpy_page( pages[0], memPtr, firstPageOffset, firstPageSize, memMap->dir );

         memPtr += firstPageSize;
         virtAddr += firstPageSize;
         bytesRemaining = region->numBytes - firstPageSize;

         for (pageIdx = 1; pageIdx < pagelist->numLockedPages; pageIdx++)
         {
            size_t bytesThisPage = (bytesRemaining > PAGE_SIZE ?
                                    PAGE_SIZE : bytesRemaining);

            dma_mmap_memcpy_page( pages[pageIdx], memPtr, 0, bytesThisPage, memMap->dir );

            memPtr += bytesThisPage;
            virtAddr += bytesThisPage;
            bytesRemaining -= bytesThisPage;
         }
      }
      else
      {
         /*
          * For all of the other memory types, we can use the virtual address
          * directly. Since we can use the virtual address, we don't even
          * need to walk through the segments.
          */

         if (memMap->dir == DMA_TO_DEVICE)
         {
            memcpy( memPtr, region->virtAddr, region->numBytes );
         }
         else
         {
            memcpy( region->virtAddr, memPtr, region->numBytes );
         }
         memPtr += region->numBytes;
      }
   }
}
EXPORT_SYMBOL(dma_mmap_memcpy);

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
 *
 */
int dma_mmap_calc_desc_cnt
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
)
{
   int                     rc;
   int                     numDescriptors;
   DMA_MMAP_REGION_T      *region;
   DMA_MMAP_SEGMENT_T     *segment;
   dma_addr_t              srcPhysAddr;
   dma_addr_t              dstPhysAddr;
   int                     regionIdx;
   int                     segmentIdx;
   int                     incDevAddr;

   if (dma_calc_desc_cnt == NULL)
      return -EINVAL;

   if (addrMode == DMA_MMAP_DEV_ADDR_INC)
      incDevAddr = 1;
   else
      incDevAddr = 0;

   /*
    * Walk through the regions and segments to figure out how many total
    * descriptors we need
    */
   numDescriptors = 0;
   for (regionIdx = 0; regionIdx < memMap->numRegionsUsed; regionIdx++)
   {
      region = &memMap->region[regionIdx];

      for (segmentIdx = 0; segmentIdx < region->numSegmentsUsed; segmentIdx++)
      {
         segment = &region->segment[segmentIdx];

         if (memMap->dir == DMA_TO_DEVICE)
         {
            srcPhysAddr = segment->physAddr;
            dstPhysAddr = devPhysAddr;
         }
         else
         {
            srcPhysAddr = devPhysAddr;
            dstPhysAddr = segment->physAddr;
         }

         rc = dma_calc_desc_cnt(data1, data2, srcPhysAddr, dstPhysAddr,
                                segment->numBytes);
         if (rc < 0)
         {
            printk(KERN_ERR "%s: dma_calculate_descriptor_count failed: %d\n",
                  __func__, rc);
            return rc;
         }

         numDescriptors += rc;
         if (incDevAddr)
            devPhysAddr += segment->numBytes;
      }
   }
   return numDescriptors;
}
EXPORT_SYMBOL(dma_mmap_calc_desc_cnt);

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
 *
 */
int dma_mmap_add_desc
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
)
{
   int                     rc;
   DMA_MMAP_REGION_T      *region;
   DMA_MMAP_SEGMENT_T     *segment;
   dma_addr_t              srcPhysAddr;
   dma_addr_t              dstPhysAddr;
   int                     regionIdx;
   int                     segmentIdx;
   int                     incDevAddr;

   if (dma_add_desc == NULL)
      return -EINVAL;

   if (addrMode == DMA_MMAP_DEV_ADDR_INC)
      incDevAddr = 1;
   else
      incDevAddr = 0;

   /* Walk through the regions and segments to populate all descriptors */
   for (regionIdx = 0; regionIdx < memMap->numRegionsUsed; regionIdx++)
   {
      region = &memMap->region[regionIdx];

      for (segmentIdx = 0; segmentIdx < region->numSegmentsUsed; segmentIdx++)
      {
         segment = &region->segment[segmentIdx];

         if (memMap->dir == DMA_TO_DEVICE)
         {
            srcPhysAddr = segment->physAddr;
            dstPhysAddr = devPhysAddr;
         }
         else
         {
            srcPhysAddr = devPhysAddr;
            dstPhysAddr = segment->physAddr;
         }

         rc = dma_add_desc(data1, data2, srcPhysAddr, dstPhysAddr,
               segment->numBytes);
         if (rc < 0)
         {
            printk( KERN_ERR "%s: dma_add_descriptors failed: %d\n",
                  __func__, rc);
            return rc;
         }

         if (incDevAddr)
            devPhysAddr += segment->numBytes;
      }
   }

   return 0;
}
EXPORT_SYMBOL(dma_mmap_add_desc);

/*
 * Crate a pagelist describing the supplied user-space buffer, and return it
 * through the pagelist_out pointer. After calling, the buffer pages should
 * be locked, ready for DMA in the indicated direction.
 *
 * Returns zero on success, or a negative error code.
 */
int dma_mmap_create_pagelist
(
   char __user             *addr,
   size_t                   numBytes,
   enum dma_data_direction  dir,
   struct task_struct      *userTask,
   DMA_MMAP_PAGELIST_T    **pagelist_out
)
{
   int                  firstPageOffset;
   int                  numLockedPages;
   DMA_MMAP_PAGELIST_T *pagelist;
   int                  rc;

   /* User pages need to be locked */
   firstPageOffset = (unsigned long)addr & (PAGE_SIZE - 1);

   numLockedPages = (firstPageOffset + numBytes + PAGE_SIZE - 1) /
      PAGE_SIZE;
   pagelist = kmalloc(sizeof(DMA_MMAP_PAGELIST_T) +
      numLockedPages * sizeof(struct page *), GFP_KERNEL);

   if (pagelist)
   {
      pagelist->dir = dir;
      pagelist->mm = NULL;

      down_read(&userTask->mm->mmap_sem);
      rc = get_user_pages(userTask,               /* task */
                          userTask->mm,           /* mm */
                          (unsigned long)addr,    /* start */
                          numLockedPages,         /* len */
                          dir == DMA_FROM_DEVICE, /* write */
                          0,                      /* force */
                          pagelist->pages,        /* pages (array of pointers to page) */
                          NULL);                  /* vmas */
      up_read(&userTask->mm->mmap_sem);

      if (rc == numLockedPages)
      {
         pagelist->mm = NULL;
         pagelist->numLockedPages = numLockedPages;
      }
      else if (rc < 0)
      {
         /* Some pages in user space may not be lockable using get_user_pages.
          * Remember the user memory map, and try later using get_pfn */
         pagelist->mm = userTask->mm;
         pagelist->numLockedPages = 0;
         rc = 0;
      }
      else
      {
         kfree(pagelist);
         pagelist = NULL;
      }
   }
   else
      rc = -ENOMEM;

   *pagelist_out = pagelist;

   return rc;
}
EXPORT_SYMBOL(dma_mmap_create_pagelist);

/*
 * Free a pagelist previously allocated using dma_mmap_create_pagelist.
 * This will unlock any locked pages.
 */
void dma_mmap_free_pagelist
(
   DMA_MMAP_PAGELIST_T *pagelist
)
{
   int i;
   for (i = 0; i < pagelist->numLockedPages; i++) {
      if (pagelist->dir != DMA_FROM_DEVICE)
         set_page_dirty(pagelist->pages[i]);
      page_cache_release(pagelist->pages[i]);
   }

   kfree(pagelist);
}
EXPORT_SYMBOL(dma_mmap_free_pagelist);

/*
 * Specifies a pagelist up front, to get it into dma_mmap_add_region.
 */
void dma_mmap_set_pagelist
(
   DMA_MMAP_CFG_T      *memMap,
   DMA_MMAP_PAGELIST_T *pagelist
)
{
   memMap->pagelist = pagelist;
}
EXPORT_SYMBOL(dma_mmap_set_pagelist);

static int
proc_debug_write(struct file *file, const char __user *buffer,
		unsigned long count, void *data)
{
   int rc;
   unsigned int debug;
   unsigned char kbuf[MAX_PROC_BUF_SIZE];

   if (count > MAX_PROC_BUF_SIZE)
      count = MAX_PROC_BUF_SIZE;

   rc = copy_from_user(kbuf, buffer, count);
   if (rc)
   {
      printk(KERN_ERR "copy_from_user failed status=%d", rc);
      return -EFAULT;
   }

   if (sscanf(kbuf, "%u", &debug) != 1)
   {
      printk(KERN_ERR "echo <debug> > /proc/%s/%s\n",
            PROC_PARENT_DIR, PROC_ENTRY_DEBUG);
      return count;
   }

   if (debug)
      gDbg = 1;
   else
      gDbg = 0;

   return count;
}

static int
proc_debug_read(char *buffer, char **start, off_t off, int count,
		int *eof, void *data)
{
   unsigned int len = 0;

   if (off > 0)
      return 0;

   len += sprintf(buffer + len, "Debug print is %s\n", gDbg ? "enabled" : "disabled");

   return len;
}

static int proc_mem_type_read(char *buf, char **start, off_t offset,
      int count, int *eof, void *data)
{
   int len = 0;

   len += sprintf(buf + len, "dma_mmap statistics\n");
   len += sprintf(buf + len, "coherent: %d\n",
         atomic_read(&gDmaStatMemTypeCoherent));
   len += sprintf(buf + len, "kmalloc:  %d\n",
         atomic_read(&gDmaStatMemTypeKmalloc));
   len += sprintf(buf + len, "vmalloc:  %d\n",
         atomic_read(&gDmaStatMemTypeVmalloc));
   len += sprintf(buf + len, "user:     %d\n",
         atomic_read(&gDmaStatMemTypeUser));

   return len;
}

int dma_mmap_init(void)
{
   int rc;
   struct proc_dir_entry *proc_debug;
   struct proc_dir_entry *proc_mem_type;

   gProcDir = create_proc_entry(PROC_PARENT_DIR, S_IFDIR | S_IRUGO | S_IXUGO, NULL);
   if (gProcDir == NULL )
   {
      printk(KERN_ERR "Unable to create /proc/%s\n", PROC_PARENT_DIR);
      rc = -ENOMEM;
      goto err_exit;
   }

   proc_debug = create_proc_entry(PROC_ENTRY_DEBUG, 0644, gProcDir);
   if (proc_debug == NULL)
   {
      rc = -ENOMEM;
      goto err_del_parent;
   }
   proc_debug->read_proc = proc_debug_read;
   proc_debug->write_proc = proc_debug_write;
   proc_debug->data = NULL;

   proc_mem_type = create_proc_entry(PROC_ENTRY_MEM_TYPE, 0644, gProcDir);
   if (proc_mem_type == NULL)
   {
      rc = -ENOMEM;
      goto err_del_debug;
   }
   proc_mem_type->read_proc = proc_mem_type_read;
   proc_mem_type->write_proc = NULL;
   proc_mem_type->data = NULL;

   return 0;

err_del_debug:
   remove_proc_entry(PROC_ENTRY_DEBUG, gProcDir);

err_del_parent:
   remove_proc_entry(PROC_PARENT_DIR, NULL);

err_exit:
   return rc;
}
