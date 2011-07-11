/*****************************************************************************
* Copyright 2011 Broadcom Corporation.  All rights reserved.
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

// ---- Include Files --------------------------------------------------------

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/cdev.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/pagemap.h>
#include <linux/slab.h>
#include <linux/ioctl.h>
#include <linux/semaphore.h>
#include <linux/proc_fs.h>
#include <linux/dma-mapping.h>
#include <linux/pfn.h>
#include <linux/hugetlb.h>

#include <asm/cacheflush.h>

#include <linux/videocore/vc_mem.h>

#include "interface/vcos/vcos.h"
#include "vc_vchi_sm.h"

#include <linux/videocore/vmcs_sm_ioctl.h>

// ---- Private Constants and Types ------------------------------------------

// Global unique identifier shift.  This is to ensure that the GUID information
// which is used as mmap'ing offset is properly passed back to kernel from user
// space.  The GUID maps the allocated memory resource from user space into kernel
// and into videocore shared memory.
//
// Default driver GUID shift value is set for Android integration, as Android
// mmap'ing call would return error if the 'offset' we are trying to map is not
// aligned to kernel page size, we use the offset as index into the resource map
// to know which videocore physical address we need to map to.
//
#define CONFIG_SM_VC_DEFAULT_GUID_SHIFT            (12)

// Uncomment the following line to enable debug messages
//#define ENABLE_LOG_DBG

// Logging macros (for remapping to other logging mechanisms, i.e., vcos_log)
#ifdef ENABLE_LOG_DBG
#define LOG_DBG(  fmt, arg... )  printk( KERN_INFO "[D] " fmt "\n", ##arg )
#else
#define LOG_DBG(  fmt, arg... )
#endif
#define LOG_INFO( fmt, arg... )  printk( KERN_INFO "[I] " fmt "\n", ##arg )
#define LOG_ERR(  fmt, arg... )  printk( KERN_ERR  "[E] " fmt "\n", ##arg )

#define DEVICE_NAME              "vcsm"
#define DEVICE_MINOR             0

#define PROC_DIR_ROOT_NAME       "vc-smem"
#define PROC_DIR_CFG_NAME        "cfg"
#define PROC_DIR_ALLOC_NAME      "alloc"
#define PROC_STATE               "state"
#define PROC_STATS               "statistics"
#define PROC_CFG_GUID_SHIFT      "guid-shift"
#define PROC_RESOURCES           "resources"
#define PROC_WRITE_BUF_SIZE      128

/* Statistics tracked per resource and globally.
*/
typedef enum
{
   // Attempt.
   ALLOC,
   FREE,
   LOCK,
   UNLOCK,
   MAP,
   FLUSH,
   INVALID,

   END_ATTEMPT,

   // Failure.
   ALLOC_FAIL,
   FREE_FAIL,
   LOCK_FAIL,
   UNLOCK_FAIL,
   MAP_FAIL,
   FLUSH_FAIL,
   INVALID_FAIL,

   END_ALL,

} SM_STATS_T;

static const char * sm_stats_human_read[] =
{
   "Alloc",
   "Free",
   "Lock",
   "Unlock",
   "Map",
   "Cache Flush",
   "Cache Invalidate",
};

/* Single resource allocation tracked for all devices.
*/
typedef struct sm_global_map
{
   struct sm_global_map          *next;       // Linked list next pointer
   
   unsigned int                  res_pid;     // PID owning that resource
   unsigned int                  res_vc_hdl;  // Resource handle (videocore handle)
   unsigned int                  res_usr_hdl; // Resource handle (user handle)
   unsigned int                  res_addr;    // Resource address (mapped virtual address)

} SM_GLOBAL_MAP_T;

/* Global state information.
*/
typedef struct
{
   VC_VCHI_SM_HANDLE_T sm_handle;      // Handle for videocore service.
   struct proc_dir_entry *dir_root;    // Proc entries root.
   struct proc_dir_entry *dir_cfg;     // Proc entries config sub-tree.
   struct proc_dir_entry *dir_alloc;   // Proc entries allocations sub-tree.
   SM_GLOBAL_MAP_T     *map;           // Global map.
   struct mutex        map_lock;       // Global map lock.

   SM_STATS_T          deceased[END_ALL];
   SM_STATS_T          terminated[END_ALL];

   VCOS_CFG_ENTRY_T    guid_shift_cfg_entry;
   uint32_t            guid_shift;     // Magic value for shifting GUID to make this driver
                                       // work seamlessly based on the platform using it.

   struct cdev         sm_cdev;        // Device.
   dev_t               sm_devid;
   struct class        *sm_class;
   struct device       *sm_dev;

} SM_STATE_T;

/* Single resource allocation tracked for each opened device.
*/
typedef struct sm_resource
{
   struct sm_resource        *next;          // Linked list next pointer

   uint32_t                   res_guid;      // Unique identifier for kernel/user cross reference
   uint32_t                   ref_cnt;       // Reference count
   uint8_t                    res_mapped;    // Resource has been mapped

   uint32_t                   res_handle;    // Resource allocation handle
   void                      *res_base_mem;  // Resource base memory address
   uint32_t                   res_size;      // Resource size allocated
   enum vmcs_sm_cache_e       res_cached;    // Resource cache type.

   SM_STATS_T                 res_stats[END_ALL];

} SM_RESOURCE_T;

/* Private file data associated with each opened device.
*/
typedef struct
{
   SM_RESOURCE_T *resource;

   uint32_t      guid;
   uint32_t      pid;

   struct proc_dir_entry *dir_pid;

} SM_PRIV_DATA_T;


// ---- Private Variables ----------------------------------------------------

static SM_STATE_T *sm_state;
static const char * sm_cache_map_vector[] =
{
   "(null)",
   "host",
   "videocore",
   "host+videocore",
};

// ---- Private Function Prototypes ------------------------------------------

// ---- Private Functions ----------------------------------------------------

/****************************************************************************/
/**
*   Translates a virtual address into a PFN, by following the MMU tables.
*
*   This function is needed to deal with pages which are marked as
*   VM_IO | VM_PFNMAP, which don't have a related page structure.
*
*   The pages which are remapped using remap_pfn_range (the typical function
*   used by drivers to process mmap) creates pages like these.
*
*   This function is almost a copy of follow_phys, taken from mm/memory.c
*
*   @return     0 on success, error code otherwise.
*/
/****************************************************************************/
static int get_pfn( struct vm_area_struct *vma,
		              unsigned long address,
		              unsigned long *pfnp )
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

	/* We cannot handle huge page PFN maps. Luckily they don't exist. */
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

/* Checks whether a mapped virtual address does indeed map to the
** expected physical address.
**
** Returns 0 on success, -errno on error.
*/
#define EXECUTE_FAST_PATH
static int vmcs_sm_check_mapping( unsigned int v_addr,
                                  unsigned int size,
                                  dma_addr_t expected_phys_addr )
{
   int ret = 0;
   unsigned long pfn = 0;
   dma_addr_t phys_addr = 0;
   struct vm_area_struct *vma = NULL;

   /* In practice the first method below always fails and we always end up
   ** having to lookup the address the 'backup' way.
   */
#if !defined(EXECUTE_FAST_PATH)
   struct page **pages;
   size_t first_page_offset = 0;
   size_t num_pages = 0;

   first_page_offset = (unsigned long)v_addr & (PAGE_SIZE - 1);
   num_pages = (first_page_offset + size + PAGE_SIZE - 1) / PAGE_SIZE;

	pages = kmalloc( sizeof(*pages) * num_pages, GFP_NOFS );
	if ( pages )
   {
	   down_read( &current->mm->mmap_sem );
	   ret = get_user_pages( current,
                            current->mm,
                            (unsigned long)v_addr,
                            num_pages,
                            0,
                            0,
                            pages,
                            NULL );
	   up_read( &current->mm->mmap_sem );

      if ( ret == num_pages )
      {
         phys_addr = PFN_PHYS( page_to_pfn( pages[0] ) ) + first_page_offset;
         if ( phys_addr != expected_phys_addr )
         {
            LOG_ERR( "[%s]: virtual address %x (sz: %u) pfn %x != expected %x",
                     __func__,
                     v_addr,
                     size,
                     phys_addr,
                     expected_phys_addr );
            ret = -EINVAL;
         }
         else
         {
            ret = 0;
         }

         /* Done.
         */
         goto out_free;
      }
      else
      {
         kfree( pages );
         pages = NULL;

         ret = -EINVAL;
      }
   }
#else
   ret = -EINVAL;
#endif /*!defined(EXECUTE_FAST_PATH)*/

   /* Backup way...  try to lookup the address differently.
   */
   if ( ret == -EINVAL )
   {
      down_read( &current->mm->mmap_sem );
      vma = find_vma( current->mm,
                      (unsigned long)v_addr );
      if ( vma != NULL )
      {
         ret = get_pfn( vma, (unsigned long)v_addr, &pfn );
      }
      else
      {
         ret = -EINVAL;
      }
      up_read( &current->mm->mmap_sem );

      if ( ret < 0 )
      {
         /* At this time, we faild all possible lookup methods.  We cannot
         ** make an educated decision, so we return a 'success'.
         */
         LOG_ERR( "[%s]: failed lookup virtual address %x (sz: %u), expected %x",
                  __func__,
                  v_addr,
                  size,
                  expected_phys_addr );
         ret = 0;
         goto out;
      }
      else
      {
         if ( PFN_PHYS(pfn) != expected_phys_addr )
         {
            LOG_ERR( "[%s]: virtual address %x (sz: %u) pfn %x != expected %x",
                     __func__,
                     v_addr,
                     size,
                     phys_addr,
                     expected_phys_addr );

            /* Valid error.
            */
            ret = -EINVAL;
         }

         /* Done.
         */
         goto out;
      }
   }


#if !defined(EXECUTE_FAST_PATH)
out_free:
   if ( pages )
   {
      kfree( pages );
      pages = NULL;
   }
#endif
out:
   return ret;
}

/* Carries over to the state statistics the statistics once owned by a deceased
** resource.
*/
static void vc_sm_resource_deceased( SM_RESOURCE_T *p_res, int terminated )
{
   if ( sm_state != NULL )
   {
      if ( p_res != NULL )
      {
         int ix;
         for ( ix = 0 ; ix < END_ALL ; ix++ )
         {
            if ( terminated )
            {
               sm_state->terminated[ ix ] += p_res->res_stats[ ix ];
            }
            else
            {
               sm_state->deceased[ ix ] += p_res->res_stats[ ix ];
            }
         }
      }
   }
}

/* Fetch a videocore handle corresponding to a mapping of the pid+address
** returns 0 (ie NULL) if no such handle exists in the global map.
*/
static unsigned int vmcs_sm_vc_handle_from_pid_and_address( unsigned int pid,
                                                            unsigned int addr )
{
   SM_GLOBAL_MAP_T *map = NULL;

   if ( sm_state == NULL )
   {
      goto out;
   }

   if ( addr == 0 )
   {
      goto out;
   }

   mutex_lock ( &(sm_state->map_lock) );

   /* Lookup the resource.
   **
   ** Linear search for now, could be improved.
   */
   map = sm_state->map;
   while ( 1 )
   {
      if ( map == NULL )
      {
         break;
      }

      if ( (map->res_pid == pid) &&
           (map->res_addr == addr) )
      {
         LOG_DBG( "[%s]: global map %p (pid %u, addr %x) -> vc-hdl %x (usr-hdl %x)",
                  __func__,
                  map,
                  map->res_pid,
                  map->res_addr,
                  map->res_vc_hdl,
                  map->res_usr_hdl );

         mutex_unlock ( &(sm_state->map_lock) );
         return map->res_vc_hdl;
      }

      map = map->next;
   }

out:
   /* Use a debug log here as it may be a valid situation that we query
   ** for something that is not mapped, we do not want a kernel log each
   ** time around.
   **
   ** There are other error log that would pop up accordingly if someone
   ** subsequently tries to use something invalid after being told not to
   ** use it...
   */
   LOG_DBG( "[%s]: not a valid map (pid %u, addr %x)",
            __func__,
            pid,
            addr );

   mutex_unlock ( &(sm_state->map_lock) );
   return 0;
}

/* Retrieve a map tracking a given pid+address.
*/
static SM_GLOBAL_MAP_T *vmcs_sm_global_map_from_pid_and_address( unsigned int pid,
                                                                 unsigned int addr )
{
   SM_GLOBAL_MAP_T *map = NULL;

   if ( sm_state == NULL )
   {
      goto out;
   }

   if ( addr == 0 )
   {
      goto out;
   }

   mutex_lock ( &(sm_state->map_lock) );

   /* Lookup the resource.
   **
   ** Linear search for now, could be improved.
   */
   map = sm_state->map;
   while ( 1 )
   {
      if ( map == NULL )
      {
         break;
      }

      if ( (map->res_pid == pid) &&
           (map->res_addr == addr) )
      {
         LOG_DBG( "[%s]: global map %p (pid %u, addr %x) -> vc-hdl %x (usr-hdl %x)",
                  __func__,
                  map,
                  map->res_pid,
                  map->res_addr,
                  map->res_vc_hdl,
                  map->res_usr_hdl );

         mutex_unlock ( &(sm_state->map_lock) );
         return map;
      }

      map = map->next;
   }

out:
   LOG_ERR( "[%s]: not a valid map (pid %u, addr %x)",
            __func__,
            pid,
            addr );

   mutex_unlock ( &(sm_state->map_lock) );
   return NULL;
}

/* Fetch a user handle corresponding to a mapping of the pid+address
** returns 0 (ie NULL) if no such handle exists in the global map.
*/
static unsigned int vmcs_sm_usr_handle_from_pid_and_address( unsigned int pid,
                                                             unsigned int addr )
{
   SM_GLOBAL_MAP_T *map = NULL;

   if ( sm_state == NULL )
   {
      goto out;
   }

   if ( addr == 0 )
   {
      goto out;
   }

   mutex_lock ( &(sm_state->map_lock) );

   /* Lookup the resource.
   **
   ** Linear search for now, could be improved.
   */
   map = sm_state->map;
   while ( 1 )
   {
      if ( map == NULL )
      {
         break;
      }

      if ( (map->res_pid == pid) &&
           (map->res_addr == addr) )
      {
         LOG_DBG( "[%s]: global map %p (pid %u, addr %x) -> usr-hdl %x (vc-hdl %x)",
                  __func__,
                  map,
                  map->res_pid,
                  map->res_addr,
                  map->res_usr_hdl,
                  map->res_vc_hdl );

         mutex_unlock ( &(sm_state->map_lock) );
         return map->res_usr_hdl;
      }

      map = map->next;
   }

out:
   /* Use a debug log here as it may be a valid situation that we query
   ** for something that is not mapped yet (typically during start condition),
   ** we do not want a kernel log each time around.
   **
   ** There are other error log that would pop up accordingly if someone
   ** subsequently tries to use something invalid after being told not to
   ** use it...
   */
   LOG_DBG( "[%s]: not a valid map (pid %u, addr %x)",
            __func__,
            pid,
            addr );

   mutex_unlock ( &(sm_state->map_lock) );
   return 0;
}

#if defined(DO_NOT_USE)
/* Fetch an address corresponding to a mapping of the pid+handle
** returns 0 (ie NULL) if no such address exists in the global map.
*/
static unsigned int vmcs_sm_usr_address_from_pid_and_vc_handle( unsigned int pid,
                                                                unsigned int hdl )
{
   SM_GLOBAL_MAP_T *map = NULL;

   if ( sm_state == NULL )
   {
      goto out;
   }

   if ( hdl == 0 )
   {
      goto out;
   }

   mutex_lock ( &(sm_state->map_lock) );

   /* Lookup the resource.
   **
   ** Linear search for now, could be improved.
   */
   map = sm_state->map;
   while ( 1 )
   {
      if ( map == NULL )
      {
         break;
      }

      if ( (map->res_pid == pid) &&
           (map->res_vc_hdl == hdl) )
      {
         LOG_DBG( "[%s]: global map %p (pid %u, vc-hdl %x, usr-hdl %x) -> addr %x",
                  __func__,
                  map,
                  map->res_pid,
                  map->res_vc_hdl,
                  map->res_usr_hdl,
                  map->res_addr );

         mutex_unlock ( &(sm_state->map_lock) );
         return map->res_addr;
      }

      map = map->next;
   }

out:
   /* Use a debug log here as it may be a valid situation that we query
   ** for something that is not mapped, we do not want a kernel log each
   ** time around.
   **
   ** There are other error log that would pop up accordingly if someone
   ** subsequently tries to use something invalid after being told not to
   ** use it...
   */
   LOG_DBG( "[%s]: not a valid map (pid %u, hdl %x)",
            __func__,
            pid,
            hdl );
   mutex_unlock ( &(sm_state->map_lock) );
   return 0;
}
#endif

/* Fetch an address corresponding to a mapping of the pid+handle
** returns 0 (ie NULL) if no such address exists in the global map.
*/
static unsigned int vmcs_sm_usr_address_from_pid_and_usr_handle( unsigned int pid,
                                                                 unsigned int hdl )
{
   SM_GLOBAL_MAP_T *map = NULL;

   if ( sm_state == NULL )
   {
      goto out;
   }

   if ( hdl == 0 )
   {
      goto out;
   }

   mutex_lock ( &(sm_state->map_lock) );

   /* Lookup the resource.
   **
   ** Linear search for now, could be improved.
   */
   map = sm_state->map;
   while ( 1 )
   {
      if ( map == NULL )
      {
         break;
      }

      if ( (map->res_pid == pid) &&
           (map->res_usr_hdl == hdl) )
      {
         LOG_DBG( "[%s]: global map %p (pid %u, vc-hdl %x, usr-hdl %x) -> addr %x",
                  __func__,
                  map,
                  map->res_pid,
                  map->res_vc_hdl,
                  map->res_usr_hdl,
                  map->res_addr );

         mutex_unlock ( &(sm_state->map_lock) );
         return map->res_addr;
      }

      map = map->next;
   }

out:
   /* Use a debug log here as it may be a valid situation that we query
   ** for something that is not mapped, we do not want a kernel log each
   ** time around.
   **
   ** There are other error log that would pop up accordingly if someone
   ** subsequently tries to use something invalid after being told not to
   ** use it...
   */
   LOG_DBG( "[%s]: not a valid map (pid %u, hdl %x)",
            __func__,
            pid,
            hdl );
   mutex_unlock ( &(sm_state->map_lock) );
   return 0;
}

/* Adds a resource to the global data list which tracks all the allocated
** data mapping.
*/
static void vmcs_sm_add_global_resource( SM_STATE_T *state,
                                         SM_GLOBAL_MAP_T *map )
{
   mutex_lock ( &(state->map_lock) );

   if ( state->map == NULL )
   {
      state->map = map;
   }
   else
   {
      SM_GLOBAL_MAP_T *ptr = state->map;

      while ( ptr->next != NULL )
      {
         ptr = ptr->next;
      }

      ptr->next = map;
   }

   LOG_DBG( "[%s]: added map %p (pid %u, vc-hdl %x, usr-hdl %x, addr %x)",
            __func__,
            map,
            map->res_pid,
            map->res_vc_hdl,
            map->res_usr_hdl,
            map->res_addr );

   map->next = NULL;

   mutex_unlock ( &(state->map_lock) );
}

/* Removes a resource from the global data list which tracks all the allocated
** data mapping.
*/
static void vmcs_sm_remove_global_resource( SM_STATE_T *state,
                                            SM_GLOBAL_MAP_T **map )
{
   SM_GLOBAL_MAP_T *prev_ptr = NULL;
   SM_GLOBAL_MAP_T *curr_ptr = NULL;

   mutex_lock ( &(state->map_lock) );

   curr_ptr = state->map;
   while ( curr_ptr != NULL )
   {
      if ( curr_ptr == *map )
      {
         if ( curr_ptr == state->map )
         {
            state->map = curr_ptr->next;
         }
         else
         {
            prev_ptr->next = curr_ptr->next;
         }

         LOG_DBG( "[%s]: removed map %p (pid %d, vc-hdl %x, usr-hdl %x, addr %x)",
                  __func__,
                  curr_ptr,
                  curr_ptr->res_pid,
                  curr_ptr->res_vc_hdl,
                  curr_ptr->res_usr_hdl,
                  curr_ptr->res_addr );

         kfree( *map );
         *map = NULL;

         mutex_unlock ( &(state->map_lock) );
         return;
      }
      else
      {
         prev_ptr = curr_ptr;
         curr_ptr = curr_ptr->next;
      }
   }

   LOG_ERR( "[%s]: failed to find map (%p) in map list",
            __func__,
            *map );

   /* We should free the memory anyway to avoid a memory leak.
   */
   kfree( *map );

   mutex_unlock ( &(state->map_lock) );
}

/* Read callback for the configuration proc entry.
*/
static int vc_sm_cfg_proc_read( char *buffer,
                                char **start,
                                off_t off,
                                int count,
                                int *eof,
                                void *data )
{
   int len = 0;

   if ( (sm_state == NULL) || (off > 0) )
   {
      return 0;
   }

   len += sprintf( buffer + len,
                   "%u\n",
                   (unsigned int) sm_state->guid_shift );

   return len;
}

/* Read callback for the state proc entry.
*/
static int vc_sm_state_proc_read( char *buf,
                                  char **start,
                                  off_t offset,
                                  int count,
                                  int *eof,
                                  void *data )
{
   int len = 0;
   SM_GLOBAL_MAP_T *map = NULL;
   int map_count = 0;

   if ( sm_state )
   {
      len += sprintf( buf + len, "\nVC-ServiceHandle     0x%x\n",
                      (unsigned int)sm_state->sm_handle );

      /* Log all applicable mapping(s).
      */
      map = sm_state->map;
      while ( 1 )
      {
         if ( map == NULL )
         {
            break;
         }

         map_count++;

         len += sprintf( buf + len, "\nMapping                0x%x\n", (unsigned int)map );
         len += sprintf( buf + len, "           TGID        %u\n", map->res_pid );
         len += sprintf( buf + len, "           VC-HDL      0x%x\n", map->res_vc_hdl );
         len += sprintf( buf + len, "           USR-HDL     0x%x\n", map->res_usr_hdl );
         len += sprintf( buf + len, "           USR-ADDR    0x%x\n", map->res_addr );

         map = map->next;
      }
   }

   len += sprintf( buf + len, "\n\nTotal map count:   %d\n\n", map_count );

   *eof = 1;
   return len + 1;
}

/* Read callback for the statistics proc entry.
*/
static int vc_sm_statistics_proc_read( char *buf,
                                       char **start,
                                       off_t offset,
                                       int count,
                                       int *eof,
                                       void *data )
{
   int len = 0;
   int ix;
   SM_PRIV_DATA_T *file_data;
   SM_RESOURCE_T *resource;

   if ( (sm_state != NULL) && (data == NULL) )
   {
      len += sprintf( buf + len, "\nDeceased Resources Statistics\n" );

      len += sprintf( buf + len, "\nNatural Cause\n" );
      for ( ix = 0 ; ix < END_ATTEMPT ; ix++ )
      {
         len += sprintf( buf + len, "                %u\t%s\n",
                         sm_state->deceased[ ix ],
                         sm_stats_human_read[ ix ] );
      }
      len += sprintf( buf + len, "\n     FAILURE\n" );
      for ( ix = 0 ; ix < END_ATTEMPT ; ix++ )
      {
         len += sprintf( buf + len, "                %u\t%s\n",
                         sm_state->deceased[ ix + END_ATTEMPT ],
                         sm_stats_human_read[ ix ] );
      }

      len += sprintf( buf + len, "\nForcefull\n" );
      for ( ix = 0 ; ix < END_ATTEMPT ; ix++ )
      {
         len += sprintf( buf + len, "                %u\t%s\n",
                         sm_state->terminated[ ix ],
                         sm_stats_human_read[ ix ] );
      }
      len += sprintf( buf + len, "\n     FAILURE\n" );
      for ( ix = 0 ; ix < END_ATTEMPT ; ix++ )
      {
         len += sprintf( buf + len, "                %u\t%s\n",
                         sm_state->terminated[ ix + END_ATTEMPT ],
                         sm_stats_human_read[ ix ] );
      }
   }
   else if ( data != NULL )
   {
      file_data = data;
      len += sprintf( buf + len, "\nStatistics for TGID %d\n", file_data->pid );

      resource = file_data->resource;
      while ( 1 )
      {
         if ( resource == NULL )
         {
            break;
         }

         len += sprintf( buf + len, "\nGUID:         0x%x\n\n", resource->res_guid );
         for ( ix = 0 ; ix < END_ATTEMPT ; ix++ )
         {
            len += sprintf( buf + len, "                %u\t%s\n",
                            resource->res_stats[ ix ],
                            sm_stats_human_read[ ix ] );
         }
         len += sprintf( buf + len, "\n     FAILURE\n" );
         for ( ix = 0 ; ix < END_ATTEMPT ; ix++ )
         {
            len += sprintf( buf + len, "                %u\t%s\n",
                            resource->res_stats[ ix + END_ATTEMPT ],
                            sm_stats_human_read[ ix ] );
         }

         resource = resource->next;
      }
   }

   *eof = 1;
   return len + 1;
}

/* Read callback for the allocation proc entry.
*/
static int vc_sm_alloc_proc_read( char *buf,
                                  char **start,
                                  off_t offset,
                                  int count,
                                  int *eof,
                                  void *data )
{
   SM_PRIV_DATA_T *file_data;
   SM_RESOURCE_T *resource;
   int len = 0;
   int alloc_count = 0;

   file_data = data;
   len += sprintf( buf + len, "\nAllocation for TGID %d\n", file_data->pid );

   resource = file_data->resource;
   while ( 1 )
   {
      if ( resource == NULL )
      {
         break;
      }

      alloc_count++;

      len += sprintf( buf + len, "\nGUID:              0x%x\n", resource->res_guid );
      len += sprintf( buf + len, "Reference Count:   %u\n", resource->ref_cnt );
      len += sprintf( buf + len, "Mapped:            %s\n", (resource->res_mapped ? "oui" : "non") );
      len += sprintf( buf + len, "VC-handle:         0x%x\n", resource->res_handle );
      len += sprintf( buf + len, "VC-address:        0x%p\n", resource->res_base_mem );
      len += sprintf( buf + len, "VC-size (bytes):   %u\n", resource->res_size );
      len += sprintf( buf + len, "Cache:             %s\n", sm_cache_map_vector[ resource->res_cached ] );

      resource = resource->next;
   }

   len += sprintf( buf + len, "\n\nTotal allocation count: %d\n\n", alloc_count );

   *eof = 1;
   return len + 1;
}

/* Write callback for the configuration proc entry.
*/
static int vc_sm_cfg_proc_write( struct file *file,
                                 const char __user *buffer,
                                 unsigned long count,
                                 void *data )
{
   int ret;
   unsigned char kbuf[PROC_WRITE_BUF_SIZE];
   uint32_t guid_shift;

   if ( count > PROC_WRITE_BUF_SIZE )
   {
      count = PROC_WRITE_BUF_SIZE;
   }

   if ( copy_from_user( kbuf,
                        buffer,
                        count ) != 0 )
   {
      LOG_ERR( "[%s]: failed to copy-from-user",
               __func__ );

      ret = -EFAULT;
      goto out;
   }

   /* Return read value no matter what from there on.
   */
   ret = count;

   if( sscanf( kbuf, "%u", &guid_shift ) != 1 )
   {
      LOG_ERR( "[%s]: echo <value> > /proc/%s/%s/%s",
               __func__,
               PROC_DIR_ROOT_NAME,
               PROC_DIR_CFG_NAME,
               PROC_CFG_GUID_SHIFT );

      /* Failed to assign the proper value.
      */
      goto out;
   }

   /* Log it because things may very well (or should 'say will for
   ** sure') stop working properly after such change.
   */
   LOG_INFO( "[%s]: guid-shift change from %u to %u",
            __func__,
            sm_state->guid_shift,
            guid_shift );
   sm_state->guid_shift = guid_shift;

   /* Done.
   */
   goto out;

out:
   return ret;
}

/* Adds a resource to the private data list which tracks all the allocated
** data.
*/
static void vmcs_sm_add_resource( SM_PRIV_DATA_T *privdata,
                                  SM_RESOURCE_T *resource )
{
   if ( privdata->resource == NULL )
   {
      privdata->resource = resource;
   }
   else
   {
      SM_RESOURCE_T *ptr = privdata->resource;

      while ( ptr->next != NULL )
      {
         ptr = ptr->next;
      }

      ptr->next = resource;
   }

   LOG_DBG( "[%s]: added resource %p (base addr %p, hdl %x, size %u, cache %u)",
            __func__,
            resource,
            resource->res_base_mem,
            resource->res_handle,
            resource->res_size,
            resource->res_cached );

   resource->next = NULL;
}

/* Removes a resource from the private data list which tracks all the allocated
** data.
*/
static void vmcs_sm_remove_resource( SM_PRIV_DATA_T *privdata,
                                     SM_RESOURCE_T **resource )
{
   SM_RESOURCE_T *prev_ptr = NULL;
   SM_RESOURCE_T *curr_ptr = privdata->resource;

   while ( curr_ptr != NULL )
   {
      if ( curr_ptr == *resource )
      {
         if ( curr_ptr == privdata->resource )
         {
            privdata->resource = curr_ptr->next;
         }
         else
         {
            prev_ptr->next = curr_ptr->next;
         }

         LOG_DBG( "[%s]: located resource %p (base addr %p, hdl %x, size %u, cache %u)",
                  __func__,
                  curr_ptr,
                  curr_ptr->res_base_mem,
                  curr_ptr->res_handle,
                  curr_ptr->res_size,
                  curr_ptr->res_cached );

         vc_sm_resource_deceased( *resource, 0 );
         kfree( *resource );
         *resource = NULL;

         return;
      }
      else
      {
         prev_ptr = curr_ptr;
         curr_ptr = curr_ptr->next;
      }
   }

   LOG_ERR( "[%s]: failed to find resource (%p) in resource list",
            __func__,
            *resource );

   /* We should free the memory anyway to avoid a memory leak.
   */
   vc_sm_resource_deceased( *resource, 1 );
   kfree( *resource );
}

/* Locates a resource from the private data list which tracks all the allocated
** data.
**
** Resource is located via the resource base address which is a globally uniquely
** handle.
*/
static SM_RESOURCE_T *vmcs_sm_get_resource_with_guid( SM_PRIV_DATA_T *privdata,
                                                      unsigned int res_guid )
{
   SM_RESOURCE_T *resource = privdata->resource;

   while ( 1 )
   {
      if ( resource == NULL )
      {
         break;
      }

      if ( resource->res_guid == res_guid )
      {
         LOG_DBG( "[%s]: located resource %p (guid: %u, base addr %p, hdl %x, size %u, cache %u)",
                  __func__,
                  resource,
                  resource->res_guid,
                  resource->res_base_mem,
                  resource->res_handle,
                  resource->res_size,
                  resource->res_cached );

         return resource;
      }

      resource = resource->next;
   }

   return NULL;
}

/* Dump the map table for the driver.  If process is -1, dumps the whole table,
** if process is a valid pid (non -1) dump only the entries associated with the
** pid of interest.
*/
static void vmcs_sm_host_walk_map_per_pid( int pid )
{
   SM_GLOBAL_MAP_T *map = NULL;

   /* Make sure the device was started properly.
   */
   if ( sm_state == NULL )
   {
      LOG_ERR( "[%s]: invalid device",
               __func__ );
      
      goto out;
   }

   /* Log all applicable mapping(s).
   */
   map = sm_state->map;
   while ( 1 )
   {
      if ( map == NULL )
      {
         break;
      }
      
      if ( (pid == -1) ||
           ((pid != -1) && (map->res_pid == pid)) )
      {
         LOG_INFO( "[%s]: tgid: %u - vc-hdl: %x, usr-hdl: %x, usr-addr: %x",
                  __func__,
                  map->res_pid,
                  map->res_vc_hdl,
                  map->res_usr_hdl,
                  map->res_addr );
      }
      map = map->next;      
   }


out:
   return;
}

/* Dump the allocation table from host side point of view.  This only dumps the
** data allocated for this process/device referenced by the file_data.
*/
static void vmcs_sm_host_walk_alloc( SM_PRIV_DATA_T *file_data )
{
   SM_RESOURCE_T *resource = NULL;

   /* Make sure the device was started properly.
   */
   if ( (sm_state == NULL) || (file_data == NULL) )
   {
      LOG_ERR( "[%s]: invalid device",
               __func__ );
      
      goto out;
   }

   resource = file_data->resource;
   while ( 1 )
   {
      if ( resource == NULL )
      {
         break;
      }

      LOG_INFO( "[%s]: guid: %x - hdl: %x, vc-mem: %p, size: %u, cache: %u",
                __func__,
                resource->res_guid,
                resource->res_handle,
                resource->res_base_mem,
                resource->res_size,
                resource->res_cached );

      resource = resource->next;
   }

out:
   return;
}

/* Open the device.  Creates a private state to help track all allocation
** associated with this device.
*/
static int vc_sm_open( struct inode *inode, struct file *file )
{
   int ret                   = 0;
   SM_PRIV_DATA_T *file_data = NULL;
   struct proc_dir_entry  *proc_cfg_entry;
   char alloc_name[32];

   /* Make sure the device was started properly.
   */
   if ( sm_state == NULL )
   {
      LOG_ERR( "[%s]: invalid device",
               __func__ );
      
      ret = -EPERM;
      goto out;
   }

   /* Allocate private structure.
   */
   file_data = kzalloc( sizeof( *file_data ), GFP_KERNEL );

   if ( file_data == NULL )
   {
      LOG_ERR( "[%s]: cannot allocate file data",
               __func__ );

      ret = -ENOMEM;
      goto out;
   }

   sprintf( alloc_name, "%d", current->tgid );

   file_data->resource = NULL;
   file_data->guid     = 1; /* Start at 1 - 0 means INVALID. */
   file_data->pid      = current->tgid; /* current->pid; */
   file_data->dir_pid  = proc_mkdir( alloc_name, sm_state->dir_alloc );

   file->private_data  = file_data;

   if ( file_data->dir_pid != NULL )
   {
      proc_cfg_entry = create_proc_read_entry( PROC_RESOURCES,
                                               0,
                                               file_data->dir_pid,
                                               vc_sm_alloc_proc_read,
                                               file_data );
      if ( proc_cfg_entry == NULL )
      {
         LOG_ERR( "[%s]: failed to create \'%s\' entry",
                  __func__,
                  alloc_name );

      }

      proc_cfg_entry = create_proc_read_entry( PROC_STATS,
                                               0,
                                               file_data->dir_pid,
                                               vc_sm_statistics_proc_read,
                                               file_data );
      if ( proc_cfg_entry == NULL )
      {
         LOG_ERR( "[%s]: failed to create \'%s\' entry",
                  __func__,
                  alloc_name );

      }
   }

   LOG_DBG( "[%s]: private data allocated %p",
            __func__,
            file_data );

out:
   return ret;
}

/* Close the device.  Free up all resources still associated with this device
** at the time.
*/
static int vc_sm_release( struct inode *inode, struct file *file )
{
   int ret                         = 0;
   SM_PRIV_DATA_T *file_data       = NULL;
   SM_RESOURCE_T *resource         = NULL;
   SM_RESOURCE_T *free_up          = NULL;
   SM_GLOBAL_MAP_T *global_map     = NULL;
   SM_GLOBAL_MAP_T *global_free_up = NULL;
   VC_SM_FREE_T free;
   char alloc_name[32];

   /* Make sure the device was started properly.
   */
   if ( sm_state == NULL )
   {
      LOG_ERR( "[%s]: invalid device",
               __func__ );

      ret = -EPERM;
      goto out;
   }

   file_data = (SM_PRIV_DATA_T *) file->private_data;

   LOG_DBG( "[%s]: using private data %p",
            __func__,
            file_data );

   if ( file_data != NULL )
   {
#if defined(VERIFY_TGID_ON_ALL_OPS)
      /* As extra precaution, make sure the tgid of the caller is the
      ** same as the registered owner for this data.
      */
      if ( file_data->pid != current->tgid )
      {
         LOG_ERR( "[%s]: current tgid %u != %u owner",
                  __func__,
                  current->tgid,
                  file_data->pid );

         ret = -EPERM;
         goto out;
      }
#endif

      /* Terminate any allocation still pending associated with this
      ** particular device.
      **
      ** NOTE: Do we need to care about multiple allocated/mapped?
      */
      resource = file_data->resource;
      while ( 1 )
      {
         if ( resource == NULL )
         {
            break;
         }
      
         free.res_handle = resource->res_handle;
         free.res_mem    = resource->res_base_mem;

         /* Free up the videocore allocated resource.
         */
         vc_vchi_sm_free( sm_state->sm_handle,
                          &free );

         /* Free up the local resource tracking this allocation.
         */
         free_up  = resource;
         resource = resource->next;

         vc_sm_resource_deceased( free_up, 1 );
         kfree ( free_up );
         free_up = NULL;
      }

      /* Terminate any global map which was allocated against the process
      ** which is closing the device.
      */
      global_map = sm_state->map;
      while ( 1 )
      {
         if ( global_map == NULL )
         {
            break;
         }

         /* Free up the local map tracking this allocation.
         */
         global_free_up  = global_map;
         global_map      = global_map->next;

         if ( global_free_up->res_pid == file_data->pid )
         {
            vmcs_sm_remove_global_resource( sm_state,
                                            &global_free_up );
            global_free_up = NULL;
         }
      }

      /* Remove the corresponding proc entry.
      */
      sprintf( alloc_name, "%d", file_data->pid );
      if ( file_data->dir_pid != NULL )
      {
         remove_proc_entry( PROC_RESOURCES, file_data->dir_pid );
         remove_proc_entry( PROC_STATS, file_data->dir_pid );
         remove_proc_entry( alloc_name, sm_state->dir_alloc );
      }

      /* Terminate the private data.
      */
      kfree ( file_data );
   }

out:
   return ret;
}

/* Map an allocated data into something that the user space.
*/
static int vc_sm_mmap( struct file *file, struct vm_area_struct *vma )
{
   int ret                   = 0;
   SM_PRIV_DATA_T *file_data = (SM_PRIV_DATA_T *) file->private_data;
   SM_RESOURCE_T *resource   = NULL;
   SM_GLOBAL_MAP_T *global_resource = NULL;
   uint32_t vc_addr          = 0;

   /* Make sure the device was started properly.
   */
   if ( (sm_state == NULL) || (file_data == NULL) )
   {
      LOG_ERR( "[%s]: invalid device",
               __func__ );

      ret = -EPERM;
      goto out;
   }

   LOG_DBG( "[%s]: using private data %p",
            __func__,
            file_data );
 
   /* As extra precaution, make sure the tgid of the caller is the
   ** same as the registered owner for this data.
   */
   if ( file_data->pid != current->tgid )
   {
      LOG_ERR( "[%s]: current tgid %u != %u owner",
               __func__,
               current->tgid,
               file_data->pid );

      ret = -EPERM;
      goto out;
   }

   /* We lookup to make sure that the data we are being asked to mmap is
   ** something that we allocated.
   **
   ** We use the offset information as the key to tell us which resource
   ** we are mapping.
   */
   LOG_DBG( "[%s]: resource handle from user guid %x (shift %x)",
            __func__,
            (unsigned int) vma->vm_pgoff,
            ((unsigned int) vma->vm_pgoff << sm_state->guid_shift) );

   resource =
      vmcs_sm_get_resource_with_guid (
               file_data,
               ((unsigned int) vma->vm_pgoff << sm_state->guid_shift) );

   if ( resource != NULL )
   {
      resource->res_stats[MAP]++;

      /* Convert the videocore memory address base into ARM one.
      **
      ** 1) Mask out the top two bits of the videocore address to get the offset.
      ** 2) Use the offset to calculate the physical addresse in ARM space.
      */
      vc_addr = (uint32_t)resource->res_base_mem & 0x3FFFFFFF;
      vc_addr += mm_vc_mem_phys_addr;

      /* Verify that what we are asked to mmap is proper according to the actual
      ** resource allocated.
      */
      if ( resource->res_size != (unsigned int) (vma->vm_end - vma->vm_start) )
      {
         LOG_ERR( "[%s]: size inconsistency (resource: %u - mmap: %u)",
                  __func__,
                  resource->res_size,
                  (unsigned int) (vma->vm_end - vma->vm_start) );

         resource->res_stats[MAP_FAIL]++;
         ret = -EINVAL;
         goto out;
      }

      LOG_DBG( "[%s]: resource %p (guid %x) - cnt %u, base address %p, handle %x, size %u (%u), cache %u, host %x",
               __func__,
               resource,
               resource->res_guid,
               resource->ref_cnt,
               resource->res_base_mem,
               resource->res_handle,
               resource->res_size,
               (unsigned int) (vma->vm_end - vma->vm_start),
               resource->res_cached,
               vc_addr );

      vma->vm_pgoff     = 0;                                      /* Reset GUID setting since real offset is 0. */
      if ( (resource->res_cached == VMCS_SM_CACHE_NONE) ||
           (resource->res_cached == VMCS_SM_CACHE_VC) )
      {
         /* Allocated non host cached memory, honour it.
         */
         vma->vm_page_prot = pgprot_noncached( vma->vm_page_prot );
      }
      vma->vm_flags    |= VM_IO | VM_RESERVED;

      ret = remap_pfn_range( vma,
                             vma->vm_start,
                             vc_addr >> PAGE_SHIFT,
                             resource->res_size,      /* ie. vma->vm_end - vma->vm_start */
                             vma->vm_page_prot );

      if ( ret == 0 )
      {
         /* Mapped successfully.
         */
         resource->res_mapped++;

         LOG_DBG( "[%s]: resource %p (base address %p, handle %x) - map-count %d, usr-addr %x",
                  __func__,
                  resource,
                  resource->res_base_mem,
                  resource->res_handle,
                  resource->res_mapped,
                  (unsigned int)vma->vm_start );

         /* Keep track of the tuple (pid, virtual address, videocore memory handle) in the
         ** global resource list such that one can do a mapping lookup for address/memory handle
         ** as needed.
         */
         global_resource = kzalloc( sizeof( *global_resource ), GFP_KERNEL );
         if ( global_resource == NULL )
         {
            LOG_ERR( "[%s]: failed to allocate global tracking resource",
                     __func__ );

            ret = -ENOMEM;
            goto out;
         }

         global_resource->res_pid     = file_data->pid;
         global_resource->res_vc_hdl  = resource->res_handle;
         global_resource->res_usr_hdl = resource->res_guid;
         global_resource->res_addr    = (unsigned int) vma->vm_start;

         vmcs_sm_add_global_resource( sm_state,
                                      global_resource );
      }
      else
      {
         /* Hmmm, what to do...
         */
         LOG_ERR( "[%s]: resource %p (base address %p, handle %x) - FAILED MAPPING %d",
                  __func__,
                  resource,
                  resource->res_base_mem,
                  resource->res_handle,
                  ret );
         resource->res_stats[MAP_FAIL]++;
      }
   }
   else
   {
      /* Invalid 'offset' passed?
      */
      LOG_ERR( "[%s]: failed to locate mmap-able resource",
               __func__ );
               
      ret = -ENOMEM;
   }


out:
   return ret;
}

/* Handle control from host.
*/
static long vc_sm_ioctl( struct file *file, unsigned int cmd, unsigned long arg )
{
   int ret                   = 0;
   unsigned int cmdnr        = _IOC_NR( cmd );
   SM_PRIV_DATA_T *file_data = (SM_PRIV_DATA_T *) file->private_data;
   SM_RESOURCE_T *resource   = NULL;

   /* Validate we can work with this device.
   */
   if ( (sm_state == NULL) || (file_data == NULL) )
   {
      LOG_ERR( "[%s]: invalid device",
               __func__ );

      ret = -EPERM;
      goto out;
   }

   /* As extra precaution, make sure the tgid of the caller is the
   ** same as the registered owner for this data.
   */
   if ( file_data->pid != current->tgid )
   {
      LOG_ERR( "[%s]: current tgid %u != %u owner",
               __func__,
               current->tgid,
               file_data->pid );

      ret = -EPERM;
      goto out;
   }

   /* Now process the command.
   */
   switch ( cmdnr )
   {
      /* New memory allocation.
      */
      case VMCS_SM_CMD_ALLOC:
      {
         int32_t                     success;
         VC_SM_ALLOC_T               alloc;
         VC_SM_ALLOC_RESULT_T        alloc_result;
         struct vmcs_sm_ioctl_alloc  ioparam;

         /* Get the parameter data.
         */
         if ( copy_from_user( &ioparam,
                              (void *)arg,
                              sizeof(ioparam) ) != 0 )
         {
            LOG_ERR( "[%s]: failed to copy-from-user for cmd %x",
                     __func__,
                     cmdnr );

            ret = -EFAULT;
            goto out;
         }

         /* Allocate local resource to track this allocation.
         */
         resource = kzalloc( sizeof( *resource ), GFP_KERNEL );
         if ( resource == NULL )
         {
            LOG_ERR( "[%s]: failed to allocate local tracking resource",
                     __func__ );

            ret = -ENOMEM;
            goto out;
         }
         memset ( resource, 0, sizeof( *resource ) );
         resource->res_stats[ALLOC]++;

         /* Allocate the videocore resource.
         */
         memset ( &alloc,
                  0,
                  sizeof ( alloc ) );
         alloc.type       =
            ((ioparam.cached == VMCS_SM_CACHE_VC) || (ioparam.cached == VMCS_SM_CACHE_BOTH)) ?
                                                         VC_SM_ALLOC_CACHED : VC_SM_ALLOC_NON_CACHED;
         alloc.base_unit  = ioparam.size;
         alloc.num_unit   = ioparam.num;
         if ( strlen ( ioparam.name ) )
         {
            memcpy ( alloc.name,
                     ioparam.name,
                     32 );
         }
         else
         {
            memcpy ( alloc.name,
                     VMCS_SM_RESOURCE_NAME_DEFAULT,
                     32 );
         }

         alloc.alignement = 4096;   /* Align to kernel page size such that it can be
                                    ** mmap'ed later on.
                                    */

         LOG_DBG( "[%s]: attempt to allocate \"%s\" data - type %u, base %u (%u), num %u, alignement %u",
                  __func__,
                  alloc.name,
                  alloc.type,
                  alloc.base_unit,
                  (alloc.base_unit + alloc.alignement - 1) & ~(alloc.alignement - 1),
                  alloc.num_unit,
                  alloc.alignement );

         /* Align the size asked for to the kernel page size.
         */
         alloc.base_unit =
            (alloc.base_unit + alloc.alignement - 1) & ~(alloc.alignement - 1);

         success = vc_vchi_sm_alloc( sm_state->sm_handle,
                                     &alloc,
                                     &alloc_result );
         if (( success != 0 ) || ( alloc_result.res_mem == NULL ))
         {
            LOG_ERR( "[%s]: failed to allocate memory on videocore (success=%d)",
                     __func__,
                     success );

            ret = -ENOMEM;
            resource->res_stats[ALLOC_FAIL]++;
            goto free_resource;
         }

         /* Keep track of the resource we created.
         */
         resource->res_handle   = alloc_result.res_handle;
         resource->res_base_mem = alloc_result.res_mem;
         resource->res_size     = alloc.base_unit * alloc.num_unit;
         resource->res_cached   = ioparam.cached;

         /* Kernel/user GUID.  This global identifier is used for mmap'ing the
         ** allocated region from user space later on, it is passed as the mmap'ing
         ** offset, we use it to 'hide' the videocore handle/address as well as to
         ** make sure it meets the requirements of some systems (such as Android) to
         ** not be messed up with so we can always reference the data correctly
         ** for future operations.
         */
         resource->res_guid     = file_data->guid++;
         resource->res_guid     <<= sm_state->guid_shift;

         vmcs_sm_add_resource ( file_data,
                                resource );

         LOG_DBG( "[%s]: allocated data - guid %x, hdl %x, base address %p, size %d, cache %d",
                  __func__,
                  resource->res_guid,
                  resource->res_handle,
                  resource->res_base_mem,
                  resource->res_size,
                  resource->res_cached );

         /* Copy result back to user.
         */
         ioparam.handle    = resource->res_guid;
         /* ioparam.base_addr = (unsigned int) alloc_result.res_mem; */

         if ( copy_to_user( (void *)arg,
                            &ioparam,
                            sizeof(ioparam) ) != 0 )
         {
            LOG_ERR( "[%s]: failed to copy-to-user for cmd %x",
                     __func__,
                     cmdnr );

            ret = -EFAULT;
            resource->res_stats[ALLOC_FAIL]++;
            goto free_resource;
         }

         /* Done.
         */
         goto out;
      }
      break;

      /* Lock (attempt to) *and* register a cache behavior change on existing\
      ** memory allocation.
      */
      case VMCS_SM_CMD_LOCK_CACHE:
      {
         int32_t                           success;
         VC_SM_LOCK_UNLOCK_T               lock_unlock;
         VC_SM_LOCK_RESULT_T               lock_result;
         struct vmcs_sm_ioctl_lock_cache   ioparam;

         /* Get parameter data.
         */
         if ( copy_from_user( &ioparam,
                              (void *)arg,
                              sizeof(ioparam) ) != 0 )
         {
            LOG_ERR( "[%s]: failed to copy-from-user for cmd %x",
                     __func__,
                     cmdnr );

            ret = -EFAULT;
            goto out;
         }

         /* Locate resource from GUID.
         */
         resource = vmcs_sm_get_resource_with_guid ( file_data,
                                                     ioparam.handle );
         if ( resource == NULL )
         {
            ret = -EINVAL;
            goto out;
         }
         resource->res_stats[LOCK]++;

         lock_unlock.res_handle = resource->res_handle;
         lock_unlock.res_mem    = resource->res_base_mem;

         LOG_DBG( "[%s]: attempt to lock-cache data - guid %x, hdl %x, base address %p",
                  __func__,
                  ioparam.handle,
                  lock_unlock.res_handle,
                  lock_unlock.res_mem );

         /* Lock the videocore allocated resource.
         */
         success = vc_vchi_sm_lock( sm_state->sm_handle,
                                    &lock_unlock,
                                    &lock_result );

         if ( (success != 0) || (lock_result.res_mem == NULL) )
         {
            LOG_ERR( "[%s]: failed to lock-cache memory on videocore (success=%d)",
                     __func__,
                     success );

            ret = -EPERM;
            resource->res_stats[LOCK_FAIL]++;
            goto out;
         }

         /* Successfully locked, increase the reference count for this resource.
         */
         resource->ref_cnt++;

         LOG_DBG( "[%s]: succeed to lock-cache data - hdl %x, base address %p, ref-cnt %d",
                  __func__,
                  lock_unlock.res_handle,
                  lock_unlock.res_mem,
                  resource->ref_cnt );

         /* Register the cache behavior change.  Note that all validation has taken
         ** place on the user side before that point, we also know we cannot change
         ** the cache behavior of the videocore resource.
         */
         resource->res_cached = ioparam.cached;

         /* Keep track of the new base memory allocation if it has changed.
         */
         if ( (lock_result.res_mem != NULL) &&
              (lock_result.res_old_mem != NULL) &&
              (lock_result.res_mem != lock_result.res_old_mem) )
         {
            resource->res_base_mem = lock_result.res_mem;
         }

         /* Done.
         */
         goto out;
      }
      break;

      /* Lock (attempt to) existing memory allocation.
      */
      case VMCS_SM_CMD_LOCK:
      {
         int32_t                           success;
         VC_SM_LOCK_UNLOCK_T               lock_unlock;
         VC_SM_LOCK_RESULT_T               lock_result;
         struct vmcs_sm_ioctl_lock_unlock  ioparam;

         /* Get parameter data.
         */
         if ( copy_from_user( &ioparam,
                              (void *)arg,
                              sizeof(ioparam) ) != 0 )
         {
            LOG_ERR( "[%s]: failed to copy-from-user for cmd %x",
                     __func__,
                     cmdnr );

            ret = -EFAULT;
            goto out;
         }

         /* Locate resource from GUID.
         */
         resource = vmcs_sm_get_resource_with_guid ( file_data,
                                                     ioparam.handle );
         if ( resource == NULL )
         {
            ret = -EINVAL;
            goto out;
         }
         resource->res_stats[LOCK]++;

         lock_unlock.res_handle = resource->res_handle;
         lock_unlock.res_mem    = resource->res_base_mem;

         LOG_DBG( "[%s]: attempt to lock data - guid %x, hdl %x, base address %p",
                  __func__,
                  ioparam.handle,
                  lock_unlock.res_handle,
                  lock_unlock.res_mem );

         /* Lock the videocore allocated resource.
         */
         success = vc_vchi_sm_lock( sm_state->sm_handle,
                                    &lock_unlock,
                                    &lock_result );

         if ( (success != 0) || (lock_result.res_mem == NULL) )
         {
            LOG_ERR( "[%s]: failed to lock memory on videocore (success=%d)",
                     __func__,
                     success );

            ret = -EPERM;
            resource->res_stats[LOCK_FAIL]++;
            goto out;
         }

         /* Successfully locked, increase the reference count for this resource.
         */
         resource->ref_cnt++;

         LOG_DBG( "[%s]: succeed to lock data - hdl %x, base address %p, ref-cnt %d",
                  __func__,
                  lock_unlock.res_handle,
                  lock_unlock.res_mem,
                  resource->ref_cnt );

         /* If the memory was never mapped, the call below will trigger a kernel error
         ** to pop up which is not very desirable even though it is harmless in this
         ** particular case.
         */
         if ( resource->res_mapped )
         {
            ioparam.addr = vmcs_sm_usr_address_from_pid_and_usr_handle( file_data->pid,
                                                                        ioparam.handle );
         }
         else
         {
            ioparam.addr = 0;
         }

         /* Check whether the physical address has changed since the last time a user
         ** locked the same memory map, if yes, we may have to remap.
         */
         if ( (lock_result.res_mem != NULL) &&
              (lock_result.res_old_mem != NULL) &&
              (lock_result.res_mem != lock_result.res_old_mem) &&
              resource->res_mapped )
         {
            ioparam.map_me = 1;
         }
         /* If we were never mapped, ask to do it as well.
         */
         else if ( !resource->res_mapped )
         {
            ioparam.map_me = 1;
         }
         /* We should not need to map.  Just continue using the same address we
         ** have been using before.
         */
         else
         {
            ioparam.map_me = 0;
         }

         /* Keep track of the new base memory allocation if it has changed.
         */
         if ( (lock_result.res_mem != NULL) &&
              (lock_result.res_old_mem != NULL) &&
              (lock_result.res_mem != lock_result.res_old_mem) )
         {
            resource->res_base_mem = lock_result.res_mem;
         }

         /* Copy result back to user.
         */
         if ( copy_to_user( (void *)arg,
                            &ioparam,
                            sizeof(ioparam) ) != 0 )
         {
            LOG_ERR( "[%s]: failed to copy-to-user for cmd %x",
                     __func__,
                     cmdnr );

            ret = -EFAULT;
         }

         /* Done.
         */
         goto out;
      }
      break;

      /* Unlock (attempt to) existing memory allocation.
      */
      case VMCS_SM_CMD_UNLOCK:
      {
         int32_t                           success;
         VC_SM_LOCK_UNLOCK_T               lock_unlock;
         struct vmcs_sm_ioctl_lock_unlock  ioparam;

         /* Get parameter data.
         */
         if ( copy_from_user( &ioparam,
                              (void *)arg,
                              sizeof(ioparam) ) != 0 )
         {
            LOG_ERR( "[%s]: failed to copy-from-user for cmd %x",
                     __func__,
                     cmdnr );

            ret = -EFAULT;
            goto out;
         }

         /* Locate resource from GUID.
         */
         resource = vmcs_sm_get_resource_with_guid ( file_data,
                                                     ioparam.handle );
         if ( resource == NULL )
         {
            ret = -EINVAL;
            goto out;
         }
         resource->res_stats[UNLOCK]++;

         lock_unlock.res_handle = resource->res_handle;
         lock_unlock.res_mem    = resource->res_base_mem;

         LOG_DBG( "[%s]: attempt to unlock data - guid %x, hdl %x, base address %p",
                  __func__,
                  ioparam.handle,
                  lock_unlock.res_handle,
                  lock_unlock.res_mem );

         /* Unlock the videocore allocated resource.
         */
         success = vc_vchi_sm_unlock( sm_state->sm_handle,
                                      &lock_unlock );

         if ( success != 0 )
         {
            LOG_ERR( "[%s]: failed to unlock memory on videocore (success=%d)",
                     __func__,
                     success );

            ret = -EPERM;
            resource->res_stats[UNLOCK_FAIL]++;
            goto out;
         }

         /* Successfully unlocked, decrease the reference count for this resource.
         */
         if ( resource->ref_cnt > 0 )
         {
            resource->ref_cnt--;
         }

         LOG_DBG( "[%s]: success to unlock data - hdl %x, base address %p, ref-cnt %d",
                  __func__,
                  lock_unlock.res_handle,
                  lock_unlock.res_mem,
                  resource->ref_cnt );

         /* Done.
         */
         goto out;
      }
      break;

      /* Resize (attempt to) existing memory allocation.
      */
      case VMCS_SM_CMD_RESIZE:
      {
         int32_t                           success;
         VC_SM_RESIZE_T                    resize;
         struct vmcs_sm_ioctl_resize       ioparam;

         /* Get parameter data.
         */
         if ( copy_from_user( &ioparam,
                              (void *)arg,
                              sizeof(ioparam) ) != 0 )
         {
            LOG_ERR( "[%s]: failed to copy-from-user for cmd %x",
                     __func__,
                     cmdnr );

            ret = -EFAULT;
            goto out;
         }

         /* Locate resource from GUID.
         */
         resource = vmcs_sm_get_resource_with_guid ( file_data,
                                                     ioparam.handle );

         /* If the resource is locked, its reference count will be not NULL,
         ** in which case we will not be allowed to resize it anyways, so
         ** reject the attempt here.
         */
         if ( (resource != NULL) && (resource->ref_cnt != 0) )
         {
            LOG_ERR( "[%s]: cannot resize locked resource - guid %x, ref-cnt %d",
                     __func__,
                     ioparam.handle,
                     resource->ref_cnt );

            ret = -EFAULT;
            goto out;
         }

         resize.res_handle   = (resource != NULL) ? resource->res_handle : 0;
         resize.res_mem      = (resource != NULL) ? resource->res_base_mem : NULL;
         resize.res_new_size = ioparam.new_size;

         LOG_DBG( "[%s]: attempt to resize data - guid %x, hdl %x, base address %p",
                  __func__,
                  ioparam.handle,
                  resize.res_handle,
                  resize.res_mem );

         /* Resize the videocore allocated resource.
         */
         success = vc_vchi_sm_resize( sm_state->sm_handle,
                                      &resize );

         if ( success != 0 )
         {
            LOG_ERR( "[%s]: failed to resize memory on videocore (success=%d)",
                     __func__,
                     success );

            ret = -EPERM;
            goto out;
         }

         LOG_DBG( "[%s]: success to resize data - hdl %x, size %d -> %d",
                  __func__,
                  resize.res_handle,
                  resource->res_size,
                  resize.res_new_size );

         /* Successfully resized, save the information and inform the user.
         */
         ioparam.old_size = resource->res_size;
         resource->res_size = resize.res_new_size;

         /* Copy result back to user.
         */
         if ( copy_to_user( (void *)arg,
                            &ioparam,
                            sizeof(ioparam) ) != 0 )
         {
            LOG_ERR( "[%s]: failed to copy-to-user for cmd %x",
                     __func__,
                     cmdnr );

            ret = -EFAULT;
         }

         /* Done.
         */
         goto out;
      }
      break;

      /* Terminate existing memory allocation.
      */
      case VMCS_SM_CMD_FREE:
      {
         int32_t                    success;
         VC_SM_FREE_T               free;
         struct vmcs_sm_ioctl_free  ioparam;

         /* Get parameter data.
         */
         if ( copy_from_user( &ioparam,
                              (void *)arg,
                              sizeof(ioparam) ) != 0 )
         {
            LOG_ERR( "[%s]: failed to copy-from-user for cmd %x",
                     __func__,
                     cmdnr );

            ret = -EFAULT;
            goto out;
         }

         /* Locate resource from GUID.
         */
         resource = vmcs_sm_get_resource_with_guid ( file_data,
                                                     ioparam.handle );
         if ( resource == NULL )
         {
            ret = -EINVAL;
            goto out;
         }
         resource->res_stats[FREE]++;

         free.res_handle = resource->res_handle;
         free.res_mem    = resource->res_base_mem;

         LOG_DBG( "[%s]: attempt to free data - guid %x, hdl %x, base address %p",
                  __func__,
                  ioparam.handle,
                  free.res_handle,
                  free.res_mem );

         /* Free up the videocore allocated resource.
         */
         success = vc_vchi_sm_free( sm_state->sm_handle,
                                    &free );
         if ( success != 0 )
         {
            LOG_ERR( "[%s]: failed to free memory on videocore (success=%d)",
                     __func__,
                     success );

            resource->res_stats[FREE_FAIL]++;
            ret = -EPERM;
            // goto out;
            /* Fall below anyways...
            */
         }

         LOG_DBG( "[%s]: %s to free data - hdl %x, base address %p",
                  __func__,
                  ( success == 0 ) ? "SUCCEEDED" : "FAILED",
                  free.res_handle,
                  free.res_mem );

         /* Free up the local resource tracking this allocation.
         */
         vmcs_sm_remove_resource ( file_data,
                                   &resource );

         /* Done.
         */
         goto out;
      }
      break;

      /* Walk allocation on videocore, information shows up in the
      ** videocore log.
      */
      case VMCS_SM_CMD_VC_WALK_ALLOC:
      {
         int32_t success;

         LOG_DBG( "[%s]: invoking walk alloc",
                  __func__ );

         success = vc_vchi_sm_walk_alloc( sm_state->sm_handle );
         if ( success != 0 )
         {
            LOG_ERR( "[%s]: failed to walk alloc on videocore (success=%d)",
                     __func__,
                     success );

            ret = -EPERM;
         }

         /* Done.
         */
         goto out;
      }
      break;

      /* Walk mapping table on host, information shows up in the
      ** kernel log.
      */
      case VMCS_SM_CMD_HOST_WALK_MAP:
      {
         /* Use pid of -1 to tell to walk the whole map.
         */
         vmcs_sm_host_walk_map_per_pid( -1 );

         /* Done.
         */
         goto out;
      }
      break;

      /* Walk mapping table per process on host, information shows up in the
      ** kernel log.
      */
      case VMCS_SM_CMD_HOST_WALK_PID_ALLOC:
      {
         struct vmcs_sm_ioctl_walk  ioparam;

         /* Get parameter data.
         */
         if ( copy_from_user( &ioparam,
                              (void *)arg,
                              sizeof(ioparam) ) != 0 )
         {
            LOG_ERR( "[%s]: failed to copy-from-user for cmd %x",
                     __func__,
                     cmdnr );

            ret = -EFAULT;
            goto out;
         }

         vmcs_sm_host_walk_alloc( file_data );

         /* Done.
         */
         goto out;
      }
      break;

      /* Walk allocation per process on host, information shows up in the
      ** kernel log.
      */
      case VMCS_SM_CMD_HOST_WALK_PID_MAP:
      {
         struct vmcs_sm_ioctl_walk  ioparam;

         /* Get parameter data.
         */
         if ( copy_from_user( &ioparam,
                              (void *)arg,
                              sizeof(ioparam) ) != 0 )
         {
            LOG_ERR( "[%s]: failed to copy-from-user for cmd %x",
                     __func__,
                     cmdnr );

            ret = -EFAULT;
            goto out;
         }

         vmcs_sm_host_walk_map_per_pid( ioparam.pid );

         /* Done.
         */
         goto out;
      }
      break;

      /* Gets the size of the memory associated with a user handle.
      */
      case VMCS_SM_CMD_SIZE_USR_HANDLE:
      {
         struct vmcs_sm_ioctl_size  ioparam;

         /* Get parameter data.
         */
         if ( copy_from_user( &ioparam,
                              (void *)arg,
                              sizeof(ioparam) ) != 0 )
         {
            LOG_ERR( "[%s]: failed to copy-from-user for cmd %x",
                     __func__,
                     cmdnr );

            ret = -EFAULT;
            goto out;
         }

         /* Locate resource from GUID.
         */
         resource = vmcs_sm_get_resource_with_guid ( file_data,
                                                     ioparam.handle );
         if ( resource != NULL )
         {
            ioparam.size = resource->res_size;
         }
         else
         {
            ioparam.size = 0;
         }

         if ( copy_to_user( (void *)arg,
                            &ioparam,
                            sizeof(ioparam) ) != 0 )
         {
            LOG_ERR( "[%s]: failed to copy-to-user for cmd %x",
                     __func__,
                     cmdnr );

            ret = -EFAULT;
         }

         /* Done.
         */
         goto out;
      }
      break;

      /* Verify we are dealing with a valid resource.
      */
      case VMCS_SM_CMD_CHK_USR_HANDLE:
      {
         struct vmcs_sm_ioctl_chk  ioparam;

         /* Get parameter data.
         */
         if ( copy_from_user( &ioparam,
                              (void *)arg,
                              sizeof(ioparam) ) != 0 )
         {
            LOG_ERR( "[%s]: failed to copy-from-user for cmd %x",
                     __func__,
                     cmdnr );

            ret = -EFAULT;
            goto out;
         }

         /* Locate resource from GUID.
         */
         resource = vmcs_sm_get_resource_with_guid ( file_data,
                                                     ioparam.handle );
         if ( resource == NULL )
         {
            ret = -EINVAL;
         }
         /* If the resource is cacheable, make sure we return additional
         ** information that may be needed to flush the cache.
         */
         else if ( (resource->res_cached == VMCS_SM_CACHE_HOST) ||
                   (resource->res_cached == VMCS_SM_CACHE_BOTH) )
         {
            ioparam.addr  = vmcs_sm_usr_address_from_pid_and_usr_handle( file_data->pid,
                                                                         ioparam.handle );
            ioparam.size  = resource->res_size;
            ioparam.cache = resource->res_cached;
         }
         else
         {
            ioparam.addr  = 0;
            ioparam.size  = 0;
            ioparam.cache = resource->res_cached;
         }

         if ( copy_to_user( (void *)arg,
                            &ioparam,
                            sizeof(ioparam) ) != 0 )
         {
            LOG_ERR( "[%s]: failed to copy-to-user for cmd %x",
                     __func__,
                     cmdnr );

            ret = -EFAULT;
         }

         /* Done.
         */
         goto out;
      }
      break;

      /* Maps a user handle given the process and the virtual address.
      */
      case VMCS_SM_CMD_MAPPED_USR_HANDLE:
      {
         struct vmcs_sm_ioctl_map  ioparam;

         /* Get parameter data.
         */
         if ( copy_from_user( &ioparam,
                              (void *)arg,
                              sizeof(ioparam) ) != 0 )
         {
            LOG_ERR( "[%s]: failed to copy-from-user for cmd %x",
                     __func__,
                     cmdnr );

            ret = -EFAULT;
            goto out;
         }

         /* Return the handle information from the pid+address mapping, returns
         ** 0 (ie NULL) if it cannot locate the actual mapping.
         */
         ioparam.handle = vmcs_sm_usr_handle_from_pid_and_address ( ioparam.pid,
                                                                    ioparam.addr );
         /* Additionally if the mapping is associated with a valid resource, which
         ** it should be unless unforseen problems at this time, return the size of
         ** the block mapped, this information is used for flushing the cache.
         */
         resource = vmcs_sm_get_resource_with_guid ( file_data,
                                                     ioparam.handle );
         if ( (resource != NULL) &&
              ( (resource->res_cached == VMCS_SM_CACHE_HOST) ||
                (resource->res_cached == VMCS_SM_CACHE_BOTH) ) )
         {
            ioparam.size = resource->res_size;
         }
         else
         {
            ioparam.size = 0;
         }

         if ( copy_to_user( (void *)arg,
                            &ioparam,
                            sizeof(ioparam) ) != 0 )
         {
            LOG_ERR( "[%s]: failed to copy-to-user for cmd %x",
                     __func__,
                     cmdnr );

            ret = -EFAULT;
         }

         /* Done.
         */
         goto out;
      }
      break;

      /* Maps a videocore handle given the process and the virtual address.
      */
      case VMCS_SM_CMD_MAPPED_VC_HDL_FROM_ADDR:
      {
         struct vmcs_sm_ioctl_map  ioparam;

         /* Get parameter data.
         */
         if ( copy_from_user( &ioparam,
                              (void *)arg,
                              sizeof(ioparam) ) != 0 )
         {
            LOG_ERR( "[%s]: failed to copy-from-user for cmd %x",
                     __func__,
                     cmdnr );

            ret = -EFAULT;
            goto out;
         }

         /* Return the handle information from the pid+address mapping, returns
         ** 0 (ie MEM_HANDLE_INVALID) if it cannot locate the actual mapping.
         */
         ioparam.handle = vmcs_sm_vc_handle_from_pid_and_address ( ioparam.pid,
                                                                   ioparam.addr );

         if ( copy_to_user( (void *)arg,
                            &ioparam,
                            sizeof(ioparam) ) != 0 )
         {
            LOG_ERR( "[%s]: failed to copy-to-user for cmd %x",
                     __func__,
                     cmdnr );

            ret = -EFAULT;
         }

         /* Done.
         */
         goto out;
      }
      break;

      /* Maps a videocore handle given the process and the user handle.
      */
      case VMCS_SM_CMD_MAPPED_VC_HDL_FROM_HDL:
      {
         struct vmcs_sm_ioctl_map  ioparam;

         /* Get parameter data.
         */
         if ( copy_from_user( &ioparam,
                              (void *)arg,
                              sizeof(ioparam) ) != 0 )
         {
            LOG_ERR( "[%s]: failed to copy-from-user for cmd %x",
                     __func__,
                     cmdnr );

            ret = -EFAULT;
            goto out;
         }

         /* Locate resource from GUID.
         */
         resource = vmcs_sm_get_resource_with_guid ( file_data,
                                                     ioparam.handle );
         if ( resource != NULL )
         {
            ioparam.handle = resource->res_handle;
         }
         else
         {
            ioparam.handle = 0;
         }

         if ( copy_to_user( (void *)arg,
                            &ioparam,
                            sizeof(ioparam) ) != 0 )
         {
            LOG_ERR( "[%s]: failed to copy-to-user for cmd %x",
                     __func__,
                     cmdnr );

            ret = -EFAULT;
         }

         /* Done.
         */
         goto out;
      }
      break;

      /* Maps a user address given the process and the videocore handle.
      */
      case VMCS_SM_CMD_MAPPED_USR_ADDRESS:
      {
         struct vmcs_sm_ioctl_map  ioparam;

         /* Get parameter data.
         */
         if ( copy_from_user( &ioparam,
                              (void *)arg,
                              sizeof(ioparam) ) != 0 )
         {
            LOG_ERR( "[%s]: failed to copy-from-user for cmd %x",
                     __func__,
                     cmdnr );

            ret = -EFAULT;
            goto out;
         }

         /* Return the address information from the pid+user-handle mapping, returns
         ** 0 (ie NULL) if it cannot locate the actual mapping.
         */
         ioparam.addr = vmcs_sm_usr_address_from_pid_and_usr_handle ( ioparam.pid,
                                                                      ioparam.handle );

         if ( copy_to_user( (void *)arg,
                            &ioparam,
                            sizeof(ioparam) ) != 0 )
         {
            LOG_ERR( "[%s]: failed to copy-to-user for cmd %x",
                     __func__,
                     cmdnr );

            ret = -EFAULT;
         }

         /* Done.
         */
         goto out;
      }
      break;

      /* Remove an existing mapping from the global map table.
      */
      case VMCS_SM_CMD_UNMAP:
      {
         struct vmcs_sm_ioctl_map  ioparam;
         SM_GLOBAL_MAP_T *global_free_up = NULL;

         /* Get parameter data.
         */
         if ( copy_from_user( &ioparam,
                              (void *)arg,
                              sizeof(ioparam) ) != 0 )
         {
            LOG_ERR( "[%s]: failed to copy-from-user for cmd %x",
                     __func__,
                     cmdnr );

            ret = -EFAULT;
            goto out;
         }

         /* Get the global resource, if it exists and remove it from the map
         ** table.
         */
         if ( ( global_free_up =
                     vmcs_sm_global_map_from_pid_and_address ( ioparam.pid,
                                                               ioparam.addr ) ) != NULL )
         {
            vmcs_sm_remove_global_resource( sm_state,
                                            &global_free_up );
            global_free_up = NULL;
         }

         /* Mark the resource as non longer mapped.
         */
         resource = vmcs_sm_get_resource_with_guid ( file_data,
                                                     ioparam.handle );
         if ( resource != NULL )
         {
            resource->res_mapped = 0;
         }

         /* Done.
         */
         goto out;
      }
      break;

      /* Flush the cache for a given mapping.
      */
      case VMCS_SM_CMD_FLUSH:
      {
         struct vmcs_sm_ioctl_cache  ioparam;

         /* Get parameter data.
         */
         if ( copy_from_user( &ioparam,
                              (void *)arg,
                              sizeof(ioparam) ) != 0 )
         {
            LOG_ERR( "[%s]: failed to copy-from-user for cmd %x",
                     __func__,
                     cmdnr );

            ret = -EFAULT;
            goto out;
         }

         /* Locate resource from GUID.
         */
         resource = vmcs_sm_get_resource_with_guid ( file_data,
                                                     ioparam.handle );

         if ( (resource != NULL) && resource->res_cached )
         {
            dma_addr_t phys_addr = 0;

            resource->res_stats[FLUSH]++;

            /* Convert the videocore memory address base into ARM one.
            **
            ** 1) Mask out the top two bits of the videocore address to get the offset.
            ** 2) Use the offset to calculate the physical addresse in ARM space.
            */
            phys_addr = (dma_addr_t)((uint32_t)resource->res_base_mem & 0x3FFFFFFF);
            phys_addr += (dma_addr_t)mm_vc_mem_phys_addr;

            /* Verify the mapped virtual address passed in corresponds to the
            ** expected physical address before doing any cache operation.
            */
            if ( vmcs_sm_check_mapping ( ioparam.addr, ioparam.size, phys_addr ) == 0 )
            {
               dmac_map_area( (const void *) ioparam.addr,
                              (size_t)ioparam.size,
                              DMA_TO_DEVICE );
               outer_clean_range( phys_addr,
                                  phys_addr + (size_t)ioparam.size );
               dmac_unmap_area( (const void *) ioparam.addr,
                                (size_t)ioparam.size,
                                DMA_TO_DEVICE );
            }
            else
            {
               resource->res_stats[FLUSH_FAIL]++;
               ret = -EINVAL;
               goto out;
            }
         }
         else if ( resource == NULL )
         {
            ret = -EINVAL;
            goto out;
         }

         /* Done.
         */
         goto out;
      }
      break;

      /* Invalidate the cache for a given mapping.
      */
      case VMCS_SM_CMD_INVALID:
      {
         struct vmcs_sm_ioctl_cache  ioparam;

         /* Get parameter data.
         */
         if ( copy_from_user( &ioparam,
                              (void *)arg,
                              sizeof(ioparam) ) != 0 )
         {
            LOG_ERR( "[%s]: failed to copy-from-user for cmd %x",
                     __func__,
                     cmdnr );

            ret = -EFAULT;
            goto out;
         }

         /* Locate resource from GUID.
         */
         resource = vmcs_sm_get_resource_with_guid ( file_data,
                                                     ioparam.handle );

         if ( (resource != NULL) && resource->res_cached )
         {
            dma_addr_t phys_addr = 0;

            resource->res_stats[INVALID]++;

            /* Convert the videocore memory address base into ARM one.
            **
            ** 1) Mask out the top two bits of the videocore address to get the offset.
            ** 2) Use the offset to calculate the physical addresse in ARM space.
            */
            phys_addr = (dma_addr_t)((uint32_t)resource->res_base_mem & 0x3FFFFFFF);
            phys_addr += (dma_addr_t)mm_vc_mem_phys_addr;

            /* Verify the mapped virtual address passed in corresponds to the
            ** expected physical address before doing any cache operation.
            */
            if ( vmcs_sm_check_mapping ( ioparam.addr, ioparam.size, phys_addr ) == 0 )
            {
               dmac_map_area( (const void *) ioparam.addr,
                              (size_t)ioparam.size,
                              DMA_FROM_DEVICE );
               outer_inv_range( phys_addr,
                                phys_addr + (size_t)ioparam.size );
               dmac_unmap_area( (const void *) ioparam.addr,
                                (size_t)ioparam.size,
                                DMA_FROM_DEVICE );
            }
            else
            {
               resource->res_stats[INVALID_FAIL]++;
               ret = -EINVAL;
               goto out;
            }
         }
         else if ( resource == NULL )
         {
            ret = -EINVAL;
            goto out;
         }

         /* Done.
         */
         goto out;
      }
      break;

      default:
      {
         ret = -EINVAL;
         goto out;
      }
      break;
   }


free_resource:
   if ( resource != NULL )
   {
      vc_sm_resource_deceased( resource, 1 );
      kfree ( resource );
      resource = NULL;
   }
out:
   return ret;
}

/* Device operations that we managed in this driver.
*/
static struct file_operations vmcs_sm_ops =
{
   .owner           = THIS_MODULE,
   .unlocked_ioctl  = vc_sm_ioctl,
   .open            = vc_sm_open,
   .release         = vc_sm_release,
   .mmap            = vc_sm_mmap,
};


/* Creation of device.
*/
static int vc_sm_create_sharedmemory( void )
{
   int ret;

   if ( sm_state == NULL )
   {
      ret = -ENOMEM;
      goto out;
   }

   /* Create a device class for creating dev nodes.
   */
   sm_state->sm_class = class_create( THIS_MODULE, "vc-sm" );
   if ( IS_ERR( sm_state->sm_class ) )
   {
      LOG_ERR( "[%s]: unable to create device class",
               __func__ );

      ret = PTR_ERR( sm_state->sm_class );
      goto out;
   }

   /* Create a character driver.
   */
   if ( (ret = alloc_chrdev_region( &sm_state->sm_devid,
                                    DEVICE_MINOR,
                                    1,
                                    DEVICE_NAME)) != 0 )
   {
      LOG_ERR( "[%s]: unable to allocate device number",
               __func__ );
      goto out_dev_class_destroy;
   }

	cdev_init( &sm_state->sm_cdev, &vmcs_sm_ops );
	if ( (ret = cdev_add( &sm_state->sm_cdev,
	                      sm_state->sm_devid,
	                      1 )) != 0 )
	{
      LOG_ERR( "[%s]: unable to register device",
               __func__ );
		goto out_chrdev_unreg;
	}

   /* Create a device node.
   */
   sm_state->sm_dev = device_create( sm_state->sm_class,
                                     NULL,
                                     MKDEV( MAJOR( sm_state->sm_devid ), DEVICE_MINOR ),
                                     NULL,
                                     DEVICE_NAME );
   if ( IS_ERR( sm_state->sm_dev ) )
   {
      LOG_ERR( "[%s]: unable to create device node",
               __func__ );
      ret = PTR_ERR( sm_state->sm_dev );
      goto out_chrdev_del;
   }

   goto out;


out_chrdev_del:
   cdev_del( &sm_state->sm_cdev );
out_chrdev_unreg:
   unregister_chrdev_region( sm_state->sm_devid, 1 );
out_dev_class_destroy:
   class_destroy( sm_state->sm_class );
   sm_state->sm_class = NULL;
out:
   return ret;
}

/* Termination of the device.
*/
static int vc_sm_remove_sharedmemory( void )
{
   int ret;

   if ( sm_state == NULL )
   {
      /* Nothing to do.
      */
      ret = 0;
      goto out;
   }

   /* Remove the sharedmemory character driver.
   */
	cdev_del( &sm_state->sm_cdev );

   /* Unregister region.
   */
	unregister_chrdev_region( sm_state->sm_devid, 1 );

   ret = 0;
   goto out;


out:
   return ret;
}

/* Driver loading.
*/
static int __init vc_sm_init( void )
{
   int                    ret;
   VCHI_INSTANCE_T        vchi_instance;
   VCHI_CONNECTION_T      *vchi_connection;
   struct proc_dir_entry  *proc_cfg_entry;

   LOG_INFO( "[%s]: start",
             __func__ );

   /* Allocate memory for the state structure.
   */
   sm_state = vcos_kcalloc( 1, sizeof( SM_STATE_T ), __func__ );
   if ( sm_state == NULL )
   {
      LOG_ERR( "[%s]: failed to allocate memory", __func__ );

      ret = -ENOMEM;
      goto out;
   }

   mutex_init( &(sm_state->map_lock) );

   /* Initialize and create a VCHI connection for the shared memory service
   ** running on videocore.
   */
   ret = vchi_initialise( &vchi_instance );
   if ( ret != 0 )
   {
      LOG_ERR( "[%s]: failed to initialise VCHI instance (ret=%d)",
               __func__,
               ret );

      ret = -EIO;
      goto err_free_mem;
   }

   ret = vchi_connect( NULL, 0, vchi_instance );
   if ( ret != 0 )
   {
      LOG_ERR( "[%s]: failed to connect VCHI instance (ret=%d)",
               __func__,
               ret );

      ret = -EIO;
      goto err_free_mem;
   }

   /* Initialize an instance of the shared memory service.
   */
   sm_state->sm_handle = vc_vchi_sm_init( vchi_instance, &vchi_connection, 1 );
   if ( sm_state->sm_handle == NULL )
   {
      LOG_ERR( "[%s]: failed to initialize shared memory service",
               __func__ );

      ret = -EPERM;
      goto err_free_mem;
   }

   /* Create a proc directory entry (root).
   */
   sm_state->dir_root = proc_mkdir( PROC_DIR_ROOT_NAME, NULL );
   if ( sm_state->dir_root == NULL )
   {
      LOG_ERR( "[%s]: failed to create \'%s\' directory entry",
               __func__,
               PROC_DIR_ROOT_NAME );

      ret = -EPERM;
      goto err_stop_sm_service;
   }

   proc_cfg_entry = create_proc_read_entry( PROC_STATE,
                                            0,
                                            sm_state->dir_root,
                                            vc_sm_state_proc_read,
                                            NULL );
   if ( proc_cfg_entry == NULL )
   {
      LOG_ERR( "[%s]: failed to create \'%s\' entry",
               __func__,
               PROC_STATE );

      ret = -EPERM;
      goto err_remove_proc_dir;
   }

   proc_cfg_entry = create_proc_read_entry( PROC_STATS,
                                            0,
                                            sm_state->dir_root,
                                            vc_sm_statistics_proc_read,
                                            NULL );
   if ( proc_cfg_entry == NULL )
   {
      LOG_ERR( "[%s]: failed to create \'%s\' entry",
               __func__,
               PROC_STATS );

      ret = -EPERM;
      goto err_remove_proc_state;
   }

   /* Create the proc entry children.
   */
   sm_state->dir_cfg = proc_mkdir( PROC_DIR_CFG_NAME, sm_state->dir_root );
   if ( sm_state->dir_cfg == NULL )
   {
      LOG_ERR( "[%s]: failed to create \'%s\' directory entry",
               __func__,
               PROC_DIR_CFG_NAME );

      ret = -EPERM;
      goto err_remove_proc_statistics;
   }

   sm_state->dir_alloc = proc_mkdir( PROC_DIR_ALLOC_NAME, sm_state->dir_root );
   if ( sm_state->dir_alloc == NULL )
   {
      LOG_ERR( "[%s]: failed to create \'%s\' directory entry",
               __func__,
               PROC_DIR_ALLOC_NAME );

      ret = -EPERM;
      goto err_remove_cfg_dir;
   }

   proc_cfg_entry = create_proc_entry( PROC_CFG_GUID_SHIFT,
                                       0644,
                                       sm_state->dir_cfg );
   if ( proc_cfg_entry == NULL )
   {
      LOG_ERR( "[%s]: failed to create \'%s\' entry",
               __func__,
               PROC_CFG_GUID_SHIFT );

      ret = -EPERM;
      goto err_remove_alloc_dir;
   }
   proc_cfg_entry->read_proc  = vc_sm_cfg_proc_read;
   proc_cfg_entry->write_proc = vc_sm_cfg_proc_write;
   proc_cfg_entry->data       = NULL;

   /* Set the default values for the modifiable parameters created
   ** above.
   */
   sm_state->guid_shift = CONFIG_SM_VC_DEFAULT_GUID_SHIFT;

   /* Create a shared memory device.
   */
   ret = vc_sm_create_sharedmemory();
   if ( ret != 0 )
   {
      LOG_ERR( "[%s]: failed to create shared memory device",
               __func__);
      goto err_remove_cfg_entry;
   }

   /* Done!
   */
   goto out;


err_remove_cfg_entry:
   remove_proc_entry( PROC_CFG_GUID_SHIFT, sm_state->dir_cfg );
err_remove_alloc_dir:
   remove_proc_entry( PROC_DIR_ALLOC_NAME, sm_state->dir_root );
err_remove_cfg_dir:
   remove_proc_entry( PROC_DIR_CFG_NAME, sm_state->dir_root );
err_remove_proc_statistics:
   remove_proc_entry( PROC_STATS, sm_state->dir_root );
err_remove_proc_state:
   remove_proc_entry( PROC_STATE, sm_state->dir_root );
err_remove_proc_dir:
   remove_proc_entry( PROC_DIR_ROOT_NAME, NULL );
err_stop_sm_service:
   vc_vchi_sm_stop( &sm_state->sm_handle );
err_free_mem:
   vcos_kfree( sm_state );
out:
   LOG_INFO( "[%s]: end - returning %d",
             __func__,
             ret );
   return ret;
}

/* Driver unloading.
*/
static void __exit vc_sm_exit( void )
{
   LOG_INFO( "[%s]: start",
             __func__ );

   /* Remove shared memory device.
   */
   vc_sm_remove_sharedmemory();

   /* Remove all proc entries.
   */
   remove_proc_entry( PROC_CFG_GUID_SHIFT, sm_state->dir_cfg );
   remove_proc_entry( PROC_DIR_CFG_NAME,   sm_state->dir_root );
   remove_proc_entry( PROC_DIR_ALLOC_NAME, sm_state->dir_root );
   remove_proc_entry( PROC_STATE,          sm_state->dir_root );
   remove_proc_entry( PROC_STATS,          sm_state->dir_root );
   remove_proc_entry( PROC_DIR_ROOT_NAME,  NULL );

   /* Stop the videocore shared memory service.
   */
   vc_vchi_sm_stop( &sm_state->sm_handle );

   /* Free the memory for the state structure.
   */
   mutex_destroy( &(sm_state->map_lock) );
   vcos_kfree( sm_state );

   LOG_INFO( "[%s]: end",
             __func__ );
}

module_init( vc_sm_init );
module_exit( vc_sm_exit );

MODULE_AUTHOR( "Broadcom" );
MODULE_DESCRIPTION( "VideoCore SharedMemory Driver" );
MODULE_LICENSE( "GPL v2" );
