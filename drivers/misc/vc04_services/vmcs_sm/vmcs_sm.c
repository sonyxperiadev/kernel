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
#include <linux/seq_file.h>
#include <linux/list.h>

#include <asm/cacheflush.h>

#include <linux/videocore/vc_mem.h>

#include "interface/vcos/vcos.h"
#include "interface/vchiq_arm/vchiq_connected.h"
#include "vc_vchi_sm.h"

#include <linux/videocore/vmcs_sm_ioctl.h>
#include "vc_sm_knl.h"

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

// Logging macros (for remapping to other logging mechanisms, i.e., vcos_log)
#define LOG_DBG( exp, fmt, arg... )  if ( exp ) printk( KERN_INFO "[D] " fmt "\n", ##arg )
#define LOG_INFO( fmt, arg... )      printk( KERN_INFO "[I] " fmt "\n", ##arg )
#define LOG_ERR( fmt, arg... )       printk( KERN_ERR  "[E] " fmt "\n", ##arg )

#define LOG_DBG_LEVEL_MIN          1
#define LOG_DBG_LEVEL_INTER_1      2
#define LOG_DBG_LEVEL_MAX          3

#define DEVICE_NAME              "vcsm"
#define DEVICE_MINOR             0

#define PROC_DIR_ROOT_NAME       "vc-smem"
#define PROC_DIR_CFG_NAME        "cfg"
#define PROC_DIR_ALLOC_NAME      "alloc"
#define PROC_STATE               "state"
#define PROC_STATS               "statistics"
#define PROC_CFG_GUID_SHIFT      "guid-shift"
#define PROC_RESOURCES           "resources"
#define PROC_DEBUG               "debug"
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

typedef int (*PROC_ENTRY_READ) ( struct seq_file *s );
typedef struct
{
   PROC_ENTRY_READ        proc_read;      // Proc read function hookup.
   struct proc_dir_entry  *dir_entry;     // Proc directory entry.
   void                   *priv_data;     // Private data associated with PDE.

} SM_PDE_T;

/* Single resource allocation tracked for all devices.
*/
typedef struct sm_global_map
{
   struct list_head              map_list;    // Linked list of maps.

   struct sm_resource            *resource;   // Pointer to the resource.

   unsigned int                  res_pid;     // PID owning that resource.
   unsigned int                  res_vc_hdl;  // Resource handle (videocore handle).
   unsigned int                  res_usr_hdl; // Resource handle (user handle).

   long unsigned int             res_addr;    // Mapped virtual address.
   struct vm_area_struct         *vma;        // VM area for this mapping.
   unsigned int                  ref_count;   // Reference count to this vma.

   struct list_head              resource_map_list; // Used to link maps associated with a resource.
} SM_GLOBAL_MAP_T;

/* Single resource allocation tracked for each opened device.
*/
typedef struct sm_resource
{
   struct list_head           resource_list;       // List of resources.

   uint32_t                   res_guid;            // Unique identifier for kernel/user cross reference.
   uint32_t                   ref_count;           // Reference / Lock count for this resource.

   uint32_t                   res_handle;          // Resource allocation handle.
   void                      *res_base_mem;        // Resource base memory address.
   uint32_t                   res_size;            // Resource size allocated.
   enum vmcs_sm_cache_e       res_cached;          // Resource cache type.

   SM_STATS_T                 res_stats[END_ALL];  // Resource statistics.

   uint8_t                    map_count;           // Counter of mappings for this resource.
   struct list_head           map_list;            // Used to link maps associated with a resource.

} SM_RESOURCE_T;

/* Private file data associated with each opened device.
*/
typedef enum
{
   INT_ACT_ALLOC,
   INT_ACT_FREE,
   INT_ACT_LOCK,
   INT_ACT_UNLOCK,
   INT_ACT_RESIZE,
   INT_ACT_WALKALLOC,

} SM_PRIV_DATA_INTERRUPTED_ACTION_T;

typedef struct
{
   struct list_head                    resource_list;    // List of resources.

   uint32_t                            guid;             // GUID (next) tracker.
   uint32_t                            pid;              // PID of creator.

   struct proc_dir_entry               *dir_pid;         // Proc entries root.
   SM_PDE_T                            dir_stats;        // Proc entries statistics sub-tree.
   SM_PDE_T                            dir_res;          // Proc entries resource sub-tree.

   VCOS_STATUS_T                       restart_sys;      // Tracks restart on interrupt.
   VC_SM_MSG_TYPE                      int_action;       // Interrupted action.
   uint32_t                            int_trans_id;     // Interrupted transaction.

} SM_PRIV_DATA_T;

/* Global state information.
*/
typedef struct
{
   VC_VCHI_SM_HANDLE_T   sm_handle;             // Handle for videocore service.
   struct proc_dir_entry *dir_root;             // Proc entries root.
   struct proc_dir_entry *dir_cfg;              // Proc entries config sub-tree.
   struct proc_dir_entry *dir_alloc;            // Proc entries allocations sub-tree.
   SM_PDE_T              dir_stats;             // Proc entries statistics sub-tree.
   SM_PDE_T              dir_state;             // Proc entries state sub-tree.
   struct proc_dir_entry *debug;                // Proc entries debug.

   struct mutex          map_lock;              // Global map lock.
   struct list_head      map_list;              // List of maps.

   SM_STATS_T            deceased[END_ALL];     // Natural termination stats collector.
   SM_STATS_T            terminated[END_ALL];   // Forcefull termination stats collector.
   uint32_t              res_deceased_cnt;      // Natural termination counter.
   uint32_t              res_terminated_cnt;    // Forcefull termination counter.

   struct proc_dir_entry *dir_guid;             // Proc entries allocations sub-tree.
   VCOS_CFG_ENTRY_T      guid_shift_cfg_entry;  // GUID shift configuration item.
   uint32_t              guid_shift;            // Magic value for shifting GUID to make this driver.
                                                // work seamlessly based on the platform using it.

   struct cdev           sm_cdev;               // Device.
   dev_t                 sm_devid;              // Device identifier.
   struct class          *sm_class;             // Class.
   struct device         *sm_dev;               // Device.

   SM_PRIV_DATA_T        *data_knl;             // Kernel internal data tracking.

} SM_STATE_T;


// ---- Private Variables ----------------------------------------------------

static SM_STATE_T *sm_state;
static unsigned int sm_debug_log = 0;
static int sm_inited = 0;

static const char * sm_cache_map_vector[] =
{
   "(null)",
   "host",
   "videocore",
   "host+videocore",
};

// ---- Private Function Prototypes ------------------------------------------

// ---- Private Functions ----------------------------------------------------

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

         if ( terminated )
         {
            sm_state->res_terminated_cnt++;
         }
         else
         {
            sm_state->res_deceased_cnt++;
         }

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
   unsigned int handle = 0;

   if ( sm_state == NULL || addr == 0 )
   {
      goto out;
   }

   mutex_lock ( &(sm_state->map_lock) );

   /* Lookup the resource.
   */
   if ( !list_empty ( &sm_state->map_list ) )
   {
      list_for_each_entry ( map, &sm_state->map_list, map_list )
      {
         if ( map->res_pid != pid || map->res_addr != addr )
            continue;

         LOG_DBG( (sm_debug_log >= LOG_DBG_LEVEL_MAX),
                  "[%s]: global map %p (pid %u, addr %lx) -> vc-hdl %x (usr-hdl %x)",
                  __func__, map, map->res_pid, map->res_addr,
                  map->res_vc_hdl, map->res_usr_hdl );

         handle = map->res_vc_hdl;
         break;
      }
   }

   mutex_unlock ( &(sm_state->map_lock) );

out:
   /* Use a debug log here as it may be a valid situation that we query
   ** for something that is not mapped, we do not want a kernel log each
   ** time around.
   **
   ** There are other error log that would pop up accordingly if someone
   ** subsequently tries to use something invalid after being told not to
   ** use it...
   */
   if ( handle == 0 )
   {
      LOG_DBG( (sm_debug_log >= LOG_DBG_LEVEL_MAX),
               "[%s]: not a valid map (pid %u, addr %x)",
               __func__, pid, addr );
   }

   return handle;
}

/* Fetch a user handle corresponding to a mapping of the pid+address
** returns 0 (ie NULL) if no such handle exists in the global map.
*/
static unsigned int vmcs_sm_usr_handle_from_pid_and_address( unsigned int pid,
                                                             unsigned int addr )
{
   SM_GLOBAL_MAP_T *map = NULL;
   unsigned int handle = 0;

   if ( sm_state == NULL || addr == 0 )
   {
      goto out;
   }

   mutex_lock ( &(sm_state->map_lock) );

   /* Lookup the resource.
   */
   if ( !list_empty ( &sm_state->map_list ) )
   {
      list_for_each_entry ( map, &sm_state->map_list, map_list )
      {
         if ( map->res_pid != pid || map->res_addr != addr )
            continue;

         LOG_DBG( (sm_debug_log >= LOG_DBG_LEVEL_MAX),
                  "[%s]: global map %p (pid %u, addr %lx) -> usr-hdl %x (vc-hdl %x)",
                  __func__, map, map->res_pid, map->res_addr,
                  map->res_usr_hdl, map->res_vc_hdl );

         handle = map->res_usr_hdl;
         break;
      }
   }

   mutex_unlock ( &(sm_state->map_lock) );

out:
   /* Use a debug log here as it may be a valid situation that we query
   ** for something that is not mapped yet (typically during start condition),
   ** we do not want a kernel log each time around.
   **
   ** There are other error log that would pop up accordingly if someone
   ** subsequently tries to use something invalid after being told not to
   ** use it...
   */
   if ( handle == 0 )
   {
      LOG_DBG( (sm_debug_log >= LOG_DBG_LEVEL_MAX),
               "[%s]: not a valid map (pid %u, addr %x)",
               __func__, pid, addr );
   }

   return handle;
}

#if defined(DO_NOT_USE)
/* Fetch an address corresponding to a mapping of the pid+handle
** returns 0 (ie NULL) if no such address exists in the global map.
*/
static unsigned int vmcs_sm_usr_address_from_pid_and_vc_handle( unsigned int pid,
                                                                unsigned int hdl )
{
   SM_GLOBAL_MAP_T *map = NULL;
   unsigned int addr = 0;

   if ( sm_state == NULL || hdl == 0 )
   {
      goto out;
   }

   mutex_lock ( &(sm_state->map_lock) );

   /* Lookup the resource.
   */
   if ( !list_empty ( &sm_state->map_list ) )
   {
      list_for_each_entry ( map, &sm_state->map_list, map_list )
      {
         if ( map->res_pid != pid || map->res_vc_hdl != hdl )
            continue;

         LOG_DBG( (sm_debug_log >= LOG_DBG_LEVEL_MAX),
                  "[%s]: global map %p (pid %u, vc-hdl %x, usr-hdl %x) -> addr %lx",
                  __func__, map, map->res_pid, map->res_vc_hdl,
                  map->res_usr_hdl, map->res_addr );

         addr = map->res_addr;
         break;
      }
   }

   mutex_unlock ( &(sm_state->map_lock) );

out:
   /* Use a debug log here as it may be a valid situation that we query
   ** for something that is not mapped, we do not want a kernel log each
   ** time around.
   **
   ** There are other error log that would pop up accordingly if someone
   ** subsequently tries to use something invalid after being told not to
   ** use it...
   */
   if ( addr == 0 )
   {
      LOG_DBG( (sm_debug_log >= LOG_DBG_LEVEL_MAX),
               "[%s]: not a valid map (pid %u, hdl %x)",
               __func__, pid, hdl );
   }

   return addr;
}
#endif

/* Fetch an address corresponding to a mapping of the pid+handle
** returns 0 (ie NULL) if no such address exists in the global map.
*/
static unsigned int vmcs_sm_usr_address_from_pid_and_usr_handle( unsigned int pid,
                                                                 unsigned int hdl )
{
   SM_GLOBAL_MAP_T *map = NULL;
   unsigned int addr = 0;

   if ( sm_state == NULL || hdl == 0 )
   {
      goto out;
   }

   mutex_lock ( &(sm_state->map_lock) );

   /* Lookup the resource.
   */
   if ( !list_empty ( &sm_state->map_list ) )
   {
      list_for_each_entry ( map, &sm_state->map_list, map_list )
      {
         if ( map->res_pid != pid || map->res_usr_hdl != hdl )
            continue;

         LOG_DBG( (sm_debug_log >= LOG_DBG_LEVEL_MAX),
                  "[%s]: global map %p (pid %u, vc-hdl %x, usr-hdl %x) -> addr %lx",
                  __func__, map, map->res_pid, map->res_vc_hdl,
                  map->res_usr_hdl, map->res_addr );

         addr = map->res_addr;
         break;
      }
   }

   mutex_unlock ( &(sm_state->map_lock) );

out:
   /* Use a debug log here as it may be a valid situation that we query
   ** for something that is not mapped, we do not want a kernel log each
   ** time around.
   **
   ** There are other error log that would pop up accordingly if someone
   ** subsequently tries to use something invalid after being told not to
   ** use it...
   */
   if ( addr == 0 )
   {
      LOG_DBG( (sm_debug_log >= LOG_DBG_LEVEL_MAX),
               "[%s]: not a valid map (pid %u, hdl %x)",
               __func__, pid, hdl );
   }

   return addr;
}

/* Adds a resource mapping to the global data list which tracks all the allocated
** data mapping.
*/
static void vmcs_sm_add_map( SM_STATE_T *state,
                             SM_RESOURCE_T *resource,
                             SM_GLOBAL_MAP_T *map )
{
   mutex_lock ( &(state->map_lock) );

   /* Add to the global list of mappings
   */
   list_add( &map->map_list, &state->map_list );

   /* Add to the list of mappings for this resource
   */
   list_add( &map->resource_map_list, &resource->map_list );
   resource->map_count++;

   mutex_unlock ( &(state->map_lock) );

   LOG_DBG( (sm_debug_log >= LOG_DBG_LEVEL_INTER_1),
            "[%s]: added map %p (pid %u, vc-hdl %x, usr-hdl %x, addr %lx)",
            __func__, map, map->res_pid, map->res_vc_hdl,
            map->res_usr_hdl, map->res_addr );
}

/* Removes a resource mapping from the global data list which tracks all the allocated
** data mapping.
*/
static void vmcs_sm_remove_map( SM_STATE_T *state,
                                SM_RESOURCE_T *resource,
                                SM_GLOBAL_MAP_T *map )
{
   mutex_lock ( &(state->map_lock) );

   /* Remove from the global list of mappings
   */
   list_del(&map->map_list);

   /* Remove from the list of mapping for this resource
   */
   list_del(&map->resource_map_list);
   resource->map_count--;

   mutex_unlock ( &(state->map_lock) );

   LOG_DBG( (sm_debug_log >= LOG_DBG_LEVEL_INTER_1),
            "[%s]: removed map %p (pid %d, vc-hdl %x, usr-hdl %x, addr %lx)",
            __func__, map, map->res_pid, map->res_vc_hdl,
            map->res_usr_hdl, map->res_addr );

   kfree( map );
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

/* Read callback for the debug proc entry.
*/
static int vc_sm_debug_proc_read( char *buffer,
                                  char **start,
                                  off_t off,
                                  int count,
                                  int *eof,
                                  void *data )
{
   int len = 0;

   len += sprintf( buffer + len,
                   "debug log level set to %u\n",
                   (unsigned int) sm_debug_log );
   len += sprintf( buffer + len,
                   "level is one increment in [0 (disabled), %u (highest)]\n",
                   LOG_DBG_LEVEL_MAX );

   return len;
}

/* Read callback for the global state proc entry.
*/
static int vc_sm_global_state_proc_read( struct seq_file *s )
{
   SM_GLOBAL_MAP_T *map = NULL;
   int map_count = 0;

   if ( sm_state == NULL )
   {
      return 0;
   }

   seq_printf( s, "\nVC-ServiceHandle     0x%x\n",
               (unsigned int)sm_state->sm_handle );

   /* Log all applicable mapping(s).
   */

   mutex_lock ( &(sm_state->map_lock) );

   if ( !list_empty ( &sm_state->map_list ) )
   {
      list_for_each_entry ( map, &sm_state->map_list, map_list )
      {
         map_count++;

         seq_printf( s, "\nMapping                0x%x\n", (unsigned int)map );
         seq_printf( s, "           TGID        %u\n", map->res_pid );
         seq_printf( s, "           VC-HDL      0x%x\n", map->res_vc_hdl );
         seq_printf( s, "           USR-HDL     0x%x\n", map->res_usr_hdl );
         seq_printf( s, "           USR-ADDR    0x%lx\n", map->res_addr );
      }
   }

   mutex_unlock ( &(sm_state->map_lock) );

   seq_printf( s, "\n\nTotal map count:   %d\n\n", map_count );

   return 0;
}

static int vc_sm_global_statistics_proc_read( struct seq_file *s )
{
   int ix;

   /* Global state tracked statistics.
   */
   if ( sm_state != NULL )
   {
      seq_printf( s, "\nDeceased Resources Statistics\n" );

      seq_printf( s, "\nNatural Cause (%u occurences)\n",
                      sm_state->res_deceased_cnt );
      for ( ix = 0 ; ix < END_ATTEMPT ; ix++ )
      {
         if ( sm_state->deceased[ ix ] > 0 )
         {
            seq_printf( s, "                %u\t%s\n",
                        sm_state->deceased[ ix ],
                        sm_stats_human_read[ ix ] );
         }
      }
      seq_printf( s, "\n" );
      for ( ix = 0 ; ix < END_ATTEMPT ; ix++ )
      {
         if ( sm_state->deceased[ ix + END_ATTEMPT ] > 0 )
         {
            seq_printf( s, "                %u\tFAILED %s\n",
                        sm_state->deceased[ ix + END_ATTEMPT ],
                        sm_stats_human_read[ ix ] );
         }
      }

      seq_printf( s, "\nForcefull (%u occurences)\n",
                  sm_state->res_terminated_cnt );
      for ( ix = 0 ; ix < END_ATTEMPT ; ix++ )
      {
         if ( sm_state->terminated[ ix ] > 0 )
         {
            seq_printf( s, "                %u\t%s\n",
                        sm_state->terminated[ ix ],
                        sm_stats_human_read[ ix ] );
         }
      }
      seq_printf( s, "\n" );
      for ( ix = 0 ; ix < END_ATTEMPT ; ix++ )
      {
         if ( sm_state->terminated[ ix + END_ATTEMPT ] > 0 )
         {
            seq_printf( s, "                %u\tFAILED %s\n",
                        sm_state->terminated[ ix + END_ATTEMPT ],
                        sm_stats_human_read[ ix ] );
         }
      }
   }

   return 0;
}

/* Read callback for the statistics proc entry.
*/
static int vc_sm_statistics_proc_read( struct seq_file *s )
{
   int ix;
   SM_PRIV_DATA_T *file_data;
   SM_RESOURCE_T *resource;
   int res_count = 0;
   SM_PDE_T *p_pde;

   p_pde = (SM_PDE_T *) (s->private);
   file_data = (SM_PRIV_DATA_T *) (p_pde->priv_data);

   if ( file_data == NULL )
   {
      return 0;
   }

   /* Per process statistics.
   */

   seq_printf( s, "\nStatistics for TGID %d\n", file_data->pid );

   mutex_lock ( &(sm_state->map_lock) );

   if ( !list_empty ( &file_data->resource_list ) )
   {
      list_for_each_entry ( resource, &file_data->resource_list, resource_list )
      {
         res_count++;

         seq_printf( s, "\nGUID:         0x%x\n\n", resource->res_guid );
         for ( ix = 0 ; ix < END_ATTEMPT ; ix++ )
         {
            if ( resource->res_stats[ ix ] > 0 )
            {
               seq_printf( s, "                %u\t%s\n",
                           resource->res_stats[ ix ],
                           sm_stats_human_read[ ix ] );
            }
         }
         seq_printf( s, "\n" );
         for ( ix = 0 ; ix < END_ATTEMPT ; ix++ )
         {
            if ( resource->res_stats[ ix + END_ATTEMPT ] > 0 )
            {
               seq_printf( s, "                %u\tFAILED %s\n",
                           resource->res_stats[ ix + END_ATTEMPT ],
                           sm_stats_human_read[ ix ] );
            }
         }
      }
   }

   mutex_unlock ( &(sm_state->map_lock) );

   seq_printf( s, "\nResources Count %d\n", res_count );

   return 0;
}

/* Read callback for the allocation proc entry.
*/
static int vc_sm_alloc_proc_read( struct seq_file *s )
{
   SM_PRIV_DATA_T *file_data;
   SM_RESOURCE_T *resource;
   int alloc_count = 0;
   SM_PDE_T *p_pde;

   p_pde = (SM_PDE_T *) (s->private);
   file_data = (SM_PRIV_DATA_T *) (p_pde->priv_data);

   if ( file_data == NULL )
   {
      return 0;
   }

   /* Per process statistics.
   */

   seq_printf( s, "\nAllocation for TGID %d\n", file_data->pid );

   mutex_lock ( &(sm_state->map_lock) );

   if ( !list_empty ( &file_data->resource_list ) )
   {
      list_for_each_entry ( resource, &file_data->resource_list, resource_list )
      {
            alloc_count++;

            seq_printf( s, "\nGUID:              0x%x\n", resource->res_guid );
            seq_printf( s, "Reference Count:   %u\n", resource->ref_count );
            seq_printf( s, "Mapped:            %s\n", (resource->map_count ? "yes" : "no") );
            seq_printf( s, "VC-handle:         0x%x\n", resource->res_handle );
            seq_printf( s, "VC-address:        0x%p\n", resource->res_base_mem );
            seq_printf( s, "VC-size (bytes):   %u\n", resource->res_size );
            seq_printf( s, "Cache:             %s\n", sm_cache_map_vector[ resource->res_cached ] );
      }
   }

   mutex_unlock ( &(sm_state->map_lock) );

   seq_printf( s, "\n\nTotal allocation count: %d\n\n", alloc_count );

   return 0;
}

/* Write callback for the configuration proc entry.
*/
static int vc_sm_cfg_proc_write( struct file *file,
                                 const char __user *buffer,
                                 unsigned long count,
                                 void *data )
{
   int ret;
   uint32_t guid_shift;
   unsigned char kbuf[PROC_WRITE_BUF_SIZE+1];

   memset ( kbuf, 0, PROC_WRITE_BUF_SIZE+1 );
   if ( count >= PROC_WRITE_BUF_SIZE )
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
   kbuf[ count - 1 ] = 0;

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

/* Write callback for the debug proc entry.
*/
static int vc_sm_debug_proc_write( struct file *file,
                                   const char __user *buffer,
                                   unsigned long count,
                                   void *data )
{
   int ret;
   uint32_t debug_value;
   unsigned char kbuf[PROC_WRITE_BUF_SIZE+1];

   memset ( kbuf, 0, PROC_WRITE_BUF_SIZE+1 );
   if ( count >= PROC_WRITE_BUF_SIZE )
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
   kbuf[ count - 1 ] = 0;

   /* Return read value no matter what from there on.
   */
   ret = count;

   if( sscanf( kbuf, "%u", &debug_value ) != 1 )
   {
      LOG_ERR( "[%s]: echo <value> > /proc/%s/%s",
               __func__,
               PROC_DIR_ROOT_NAME,
               PROC_DEBUG );

      /* Failed to assign the proper value.
      */
      goto out;
   }

   if ( debug_value > LOG_DBG_LEVEL_MAX )
   {
      LOG_ERR( "[%s]: echo [0,%u] > /proc/%s/%s",
               __func__,
               LOG_DBG_LEVEL_MAX,
               PROC_DIR_ROOT_NAME,
               PROC_DEBUG );

      /* Failed to assign the proper value.
      */
      goto out;
   }

   LOG_INFO( "[%s]: debug log change from level %u to level %u",
            __func__,
            sm_debug_log,
            debug_value );
   sm_debug_log = debug_value;

   /* Done.
   */
   goto out;

out:
   return ret;
}

static int vc_sm_seq_file_proc_read( struct seq_file *s, void *unused )
{
    SM_PDE_T *sm_pde;

    sm_pde = (SM_PDE_T *)(s->private);

    if ( sm_pde && sm_pde->proc_read )
    {
        sm_pde->proc_read( s );
    }

    return 0;
}

static int vc_sm_single_proc_open( struct inode *inode, struct file *file )
{
    return single_open( file, vc_sm_seq_file_proc_read, PDE(inode)->data );
}

static const struct file_operations vc_sm_proc_fops =
{
    .open       = vc_sm_single_proc_open,
    .read       = seq_read,
    .llseek     = seq_lseek,
    .release    = single_release,
};

/* Adds a resource to the private data list which tracks all the allocated
** data.
*/
static void vmcs_sm_add_resource( SM_PRIV_DATA_T *privdata,
                                  SM_RESOURCE_T *resource )
{
   mutex_lock ( &(sm_state->map_lock) );
   list_add( &resource->resource_list, &privdata->resource_list );
   mutex_unlock ( &(sm_state->map_lock) );

   LOG_DBG( (sm_debug_log >= LOG_DBG_LEVEL_INTER_1),
            "[%s]: added resource %p (base addr %p, hdl %x, size %u, cache %u)",
            __func__, resource, resource->res_base_mem, resource->res_handle,
            resource->res_size, resource->res_cached );
}

/* Removes a resource from the private data list which tracks all the allocated
** data.
*/
static void vmcs_sm_remove_resource( SM_PRIV_DATA_T *privdata,
                                     SM_RESOURCE_T *resource )
{
   mutex_lock ( &(sm_state->map_lock) );
   list_del(&resource->resource_list);
   mutex_unlock ( &(sm_state->map_lock) );

   LOG_DBG( (sm_debug_log >= LOG_DBG_LEVEL_INTER_1),
            "[%s]: located resource %p (base addr %p, hdl %x, size %u, cache %u)",
            __func__, resource, resource->res_base_mem,
            resource->res_handle, resource->res_size,
            resource->res_cached );

   vc_sm_resource_deceased( resource, 0 );
   kfree( resource );
   return;
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
   SM_RESOURCE_T *resource = NULL;

   mutex_lock ( &(sm_state->map_lock) );

   if ( !list_empty( &privdata->resource_list ) )
   {
      list_for_each_entry ( resource, &privdata->resource_list, resource_list )
      {
         if ( resource->res_guid != res_guid )
            continue;

         LOG_DBG( (sm_debug_log >= LOG_DBG_LEVEL_MAX),
                  "[%s]: located resource %p (guid: %u, base addr %p, hdl %x, size %u, cache %u)",
                  __func__, resource, resource->res_guid, resource->res_base_mem,
                  resource->res_handle, resource->res_size, resource->res_cached );
         break;
      }
   }

   mutex_unlock ( &(sm_state->map_lock) );

   return resource;
}

#if defined(NOT_USED)
/* Fetch the mapped data from the global map corresponding to the resource and
** the given virtual address
*/
static SM_GLOBAL_MAP_T *vmcs_sm_map_from_resource( SM_RESOURCE_T *resource, unsigned long addr )
{
   SM_GLOBAL_MAP_T *map = NULL;

   if ( sm_state == NULL || resource == NULL )
   {
      return NULL;
   }

   mutex_lock ( &(sm_state->map_lock) );

   if ( !list_empty ( &resource->map_list ) )
   {
      list_for_each_entry ( map, &resource->map_list, resource_map_list )
      {
         if ( map->res_addr == addr )
            break;
      }
   }
   mutex_unlock ( &(sm_state->map_lock) );

   return map;
}
#endif

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
      LOG_ERR( "[%s]: invalid device", __func__ );
      return;
   }

   mutex_lock ( &(sm_state->map_lock) );

   /* Log all applicable mapping(s).
   */
   if ( !list_empty ( &sm_state->map_list ) )
   {
      list_for_each_entry ( map, &sm_state->map_list, map_list )
      {
         if ( pid == -1 || map->res_pid == pid )
         {
            LOG_INFO( "[%s]: tgid: %u - vc-hdl: %x, usr-hdl: %x, usr-addr: %lx",
                     __func__, map->res_pid, map->res_vc_hdl,
                     map->res_usr_hdl, map->res_addr );
         }
      }
   }

   mutex_unlock ( &(sm_state->map_lock) );

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
      LOG_ERR( "[%s]: invalid device", __func__ );
      return;
   }

   mutex_lock ( &(sm_state->map_lock) );

   if ( !list_empty ( &file_data->resource_list ) )
   {
      list_for_each_entry ( resource, &file_data->resource_list, resource_list )
      {
         LOG_INFO( "[%s]: guid: %x - hdl: %x, vc-mem: %p, size: %u, cache: %u",
                   __func__, resource->res_guid, resource->res_handle,
                   resource->res_base_mem, resource->res_size,
                   resource->res_cached );
      }
   }

   mutex_unlock ( &(sm_state->map_lock) );

   return;
}

/* Create support for private data tracking.
*/
static SM_PRIV_DATA_T *vc_sm_create_priv_data ( int id )
{
   char alloc_name[32];
   SM_PRIV_DATA_T *file_data = NULL;

   /* Allocate private structure.
   */
   file_data = kzalloc( sizeof( *file_data ), GFP_KERNEL );

   if ( file_data == NULL )
   {
      LOG_ERR( "[%s]: cannot allocate file data",
               __func__ );
      goto out;
   }

   sprintf( alloc_name, "%d", id );

   INIT_LIST_HEAD(&file_data->resource_list);
   file_data->guid     = 1; /* Start at 1 - 0 means INVALID. */
   file_data->pid      = id;
   file_data->dir_pid  = proc_mkdir( alloc_name, sm_state->dir_alloc );

   if ( file_data->dir_pid != NULL )
   {
      file_data->dir_res.dir_entry = create_proc_entry( PROC_RESOURCES,
                                                        0,
                                                        file_data->dir_pid );
      if ( file_data->dir_res.dir_entry == NULL )
      {
         LOG_ERR( "[%s]: failed to create \'%s\' entry",
                  __func__,
                  alloc_name );
      }
      else
      {
         file_data->dir_res.priv_data = (void *)file_data;
         file_data->dir_res.proc_read = &vc_sm_alloc_proc_read;

         file_data->dir_res.dir_entry->proc_fops = &vc_sm_proc_fops;
         file_data->dir_res.dir_entry->data = &(file_data->dir_res);
      }

      file_data->dir_stats.dir_entry = create_proc_entry( PROC_STATS,
                                                          0,
                                                          file_data->dir_pid );
      if ( file_data->dir_stats.dir_entry == NULL )
      {
         LOG_ERR( "[%s]: failed to create \'%s\' entry",
                  __func__,
                  alloc_name );
      }
      else
      {
         file_data->dir_stats.priv_data = (void *)file_data;
         file_data->dir_stats.proc_read = &vc_sm_statistics_proc_read;

         file_data->dir_stats.dir_entry->proc_fops = &vc_sm_proc_fops;
         file_data->dir_stats.dir_entry->data = &(file_data->dir_stats);
      }
   }

   LOG_DBG( (sm_debug_log >= LOG_DBG_LEVEL_INTER_1),
            "[%s]: private data allocated %p",
            __func__,
            file_data );

out:
   return file_data;
}

/* Open the device.  Creates a private state to help track all allocation
** associated with this device.
*/
static int vc_sm_open( struct inode *inode, struct file *file )
{
   int ret                   = 0;

   /* Make sure the device was started properly.
   */
   if ( sm_state == NULL )
   {
      LOG_ERR( "[%s]: invalid device",
               __func__ );

      ret = -EPERM;
      goto out;
   }

   file->private_data = vc_sm_create_priv_data ( current->tgid );
   if ( file->private_data == NULL )
   {
      LOG_ERR( "[%s]: failed to create data tracker",
               __func__ );

      ret = -ENOMEM;
      goto out;
   }

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
   SM_RESOURCE_T *resource, *resource_tmp;
   VC_SM_FREE_ALL_T free_all;
   int freed_up                    = 0;
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

   LOG_DBG( (sm_debug_log >= LOG_DBG_LEVEL_INTER_1),
            "[%s]: using private data %p",
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

#if 0
      if ( file_data->restart_sys == VCOS_EINTR )
      {
         VC_SM_ACTION_CLEAN_T action_clean;

         LOG_DBG( (sm_debug_log >= LOG_DBG_LEVEL_INTER_1),
                  "[%s]: releasing following VCOS_EINTR on %u (trans_id: %u) (likely due to signal)...",
                  __func__,
                  file_data->int_action,
                  file_data->int_trans_id );


         action_clean.res_action = file_data->int_action;
         action_clean.action_trans_id = file_data->int_trans_id;

         vc_vchi_sm_clean_up( sm_state->sm_handle,
                              &action_clean );
      }
#endif

      /* Terminate any allocation still pending associated with this
      ** particular device.
      **
      ** NOTE: Do we need to care about multiple allocated/mapped?
      */
      mutex_lock ( &(sm_state->map_lock) );
      if ( !list_empty ( &file_data->resource_list ) )
      {
         list_for_each_entry_safe ( resource, resource_tmp, &file_data->resource_list, resource_list )
         {
            /* Free up the local resource tracking this allocation.
            */
            vc_sm_resource_deceased( resource, 1 );
            kfree ( resource );
            freed_up++;
         }
      }
      mutex_unlock ( &(sm_state->map_lock) );

      /* Free up the videocore allocated resource left for this allocator.
      */
      if ( freed_up )
      {
         free_all.allocator = file_data->pid;

         LOG_ERR( "[%s]: automatic clean up of left over resources for TGID %u",
                  __func__, free_all.allocator );

         if ( vc_vchi_sm_free_post_mortem( sm_state->sm_handle,
                                           &free_all ) != VCOS_SUCCESS )
         {
            LOG_ERR( "[%s]: potential resource leakage on videocore for TGID %u",
                     __func__, free_all.allocator );
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

static void vcsm_vma_open(struct vm_area_struct *vma)
{
   SM_GLOBAL_MAP_T *map = (SM_GLOBAL_MAP_T *)vma->vm_private_data;

   LOG_DBG( (sm_debug_log >= LOG_DBG_LEVEL_MAX),
            "[%s]: virt %lx-%lx, pid %i, pfn %i",
            __func__, vma->vm_start, vma->vm_end, (int)current->tgid, (int)vma->vm_pgoff);

   map->ref_count++;
}

static void vcsm_vma_close(struct vm_area_struct *vma)
{
   SM_GLOBAL_MAP_T *map = (SM_GLOBAL_MAP_T *)vma->vm_private_data;

   LOG_DBG( (sm_debug_log >= LOG_DBG_LEVEL_MAX),
            "[%s]: virt %lx-%lx, pid %i, pfn %i",
            __func__, vma->vm_start, vma->vm_end, (int)current->tgid, (int)vma->vm_pgoff);

   map->ref_count--;

   /* Remove from the map table.
   */
   if ( map->ref_count == 0 )
   {
      vmcs_sm_remove_map( sm_state, map->resource, map );
   }
}

static int vcsm_vma_fault(struct vm_area_struct *vma, struct vm_fault *vmf)
{
   SM_GLOBAL_MAP_T *map = (SM_GLOBAL_MAP_T *)vma->vm_private_data;
   SM_RESOURCE_T *resource = map->resource;
   pgoff_t page_offset;
   unsigned long pfn;
   int ret = 0;

   /* Lock the resource if necessary.
   */
   if ( !resource->ref_count )
   {
      VC_SM_LOCK_UNLOCK_T lock_unlock;
      VC_SM_LOCK_RESULT_T lock_result;
      VCOS_STATUS_T       status;

      lock_unlock.res_handle = resource->res_handle;
      lock_unlock.res_mem    = resource->res_base_mem;

      LOG_DBG( (sm_debug_log >= LOG_DBG_LEVEL_MAX),
               "[%s]: attempt to lock data - hdl %x, base address %p",
               __func__,
               lock_unlock.res_handle,
               lock_unlock.res_mem );

      /* Lock the videocore allocated resource.
      */
      status = vc_vchi_sm_lock( sm_state->sm_handle,
                                &lock_unlock,
                                &lock_result,
                                0 );
      if ( (status != VCOS_SUCCESS) ||
           ((status == VCOS_SUCCESS) && (lock_result.res_mem == NULL)) )
      {
         LOG_ERR( "[%s]: failed to lock memory on videocore (status: %u)",
                  __func__,
                  status );
         resource->res_stats[LOCK_FAIL]++;
         return VM_FAULT_SIGBUS;
      }

      resource->res_stats[LOCK]++;
      resource->ref_count++;

      /* Keep track of the new base memory allocation if it has changed.
      */
      if ( (lock_result.res_mem != NULL) &&
           (lock_result.res_old_mem != NULL) &&
           (lock_result.res_mem != lock_result.res_old_mem) )
      {
         resource->res_base_mem = lock_result.res_mem;
      }
   }

   /* We don't use vmf->pgoff since that has the fake offset */
   page_offset = ((unsigned long)vmf->virtual_address - vma->vm_start);
   pfn = (uint32_t)resource->res_base_mem & 0x3FFFFFFF;
   pfn += mm_vc_mem_phys_addr;
   pfn += page_offset;
   pfn >>= PAGE_SHIFT;

   /* Finally, remap it */
   ret = vm_insert_pfn( vma, (unsigned long)vmf->virtual_address, pfn );

   switch (ret)
   {
      case 0:
      case -ERESTARTSYS:
         return VM_FAULT_NOPAGE;
      case -ENOMEM:
      case -EAGAIN:
         return VM_FAULT_OOM;
      default:
         return VM_FAULT_SIGBUS;
   }
}

static struct vm_operations_struct vcsm_vm_ops = {
    .open =  vcsm_vma_open,
    .close = vcsm_vma_close,
    .fault = vcsm_vma_fault,
};

/* Walks a VMA and clean each valid page from the cache */
static void vcsm_vma_cache_clean_page_range(unsigned long addr, unsigned long end)
{
   pgd_t *pgd;
   pud_t *pud;
   pmd_t *pmd;
   pte_t *pte;
   unsigned long pgd_next, pud_next, pmd_next;

   if (addr >= end)
      return;

   /* Walk PGD */
   pgd = pgd_offset(current->mm, addr);
   do {
      pgd_next = pgd_addr_end(addr, end);

      if (pgd_none(*pgd) || pgd_bad(*pgd))
         continue;

      /* Walk PUD */
      pud = pud_offset(pgd, addr);
      do {
         pud_next = pud_addr_end(addr, pgd_next);
         if (pud_none(*pud) || pud_bad(*pud))
                      continue;

         /* Walk PMD */
         pmd = pmd_offset(pud, addr);
         do {
            pmd_next = pmd_addr_end(addr, pud_next);
            if (pmd_none(*pmd) || pmd_bad(*pmd))
               continue;

            /* Walk PTE */
            pte = pte_offset_map(pmd, addr);
            do {
               if (pte_none(*pte) || !pte_present(*pte))
                  continue;

               /* Clean + invalidate the cache for this page */
               //printk(KERN_ERR "pte: %p, start %lx, end %lx", pte, start, end);
               dmac_flush_range( (const void *)addr, (const void *)(addr + PAGE_SIZE) );

            } while (pte++, addr += PAGE_SIZE, addr != pmd_next);
            pte_unmap(pte);

         } while (pmd++, addr = pmd_next, addr != pud_next);

      } while (pud++, addr = pud_next, addr != pgd_next);
   } while (pgd++, addr = pgd_next, addr != end);
}

/* Map an allocated data into something that the user space.
*/
static int vc_sm_mmap( struct file *file, struct vm_area_struct *vma )
{
   int ret                   = 0;
   SM_PRIV_DATA_T *file_data = (SM_PRIV_DATA_T *) file->private_data;
   SM_RESOURCE_T *resource   = NULL;
   SM_GLOBAL_MAP_T *global_resource = NULL;

   /* Make sure the device was started properly.
   */
   if ( (sm_state == NULL) || (file_data == NULL) )
   {
      LOG_ERR( "[%s]: invalid device",
               __func__ );
      return -EPERM;
   }

   LOG_DBG( (sm_debug_log >= LOG_DBG_LEVEL_MAX),
            "[%s]: using private data %p",
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
      return -EPERM;
   }

   /* We lookup to make sure that the data we are being asked to mmap is
   ** something that we allocated.
   **
   ** We use the offset information as the key to tell us which resource
   ** we are mapping.
   */
   LOG_DBG( (sm_debug_log >= LOG_DBG_LEVEL_MAX),
            "[%s]: resource handle from user guid %x (shift %x)",
            __func__,
            (unsigned int) vma->vm_pgoff,
            ((unsigned int) vma->vm_pgoff << sm_state->guid_shift) );

   resource =
      vmcs_sm_get_resource_with_guid (
               file_data,
               ((unsigned int) vma->vm_pgoff << sm_state->guid_shift) );

   if ( resource == NULL )
   {
      /* Invalid 'offset' passed?
      */
      LOG_ERR( "[%s]: failed to locate mmap-able resource",
               __func__ );
      return -ENOMEM;
   }

   /* Verify that what we are asked to mmap is proper according to the actual
   ** resource allocated.
   */
   if ( resource->res_size != (unsigned int) (vma->vm_end - vma->vm_start) )
   {
      LOG_ERR( "[%s]: size inconsistency (resource: %u - mmap: %u)",
               __func__,
               resource->res_size,
               (unsigned int) (vma->vm_end - vma->vm_start) );

      ret = -EINVAL;
      goto error;
   }

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
      goto error;
   }

   global_resource->res_pid     = file_data->pid;
   global_resource->res_vc_hdl  = resource->res_handle;
   global_resource->res_usr_hdl = resource->res_guid;
   global_resource->res_addr    = (long unsigned int)vma->vm_start;
   global_resource->resource    = resource;
   global_resource->vma         = vma;
   vmcs_sm_add_map( sm_state, resource, global_resource );

   /* We are not actually mapping the pages, we just provide a fault
   ** handler to allow pages to be mapped when accessed
   */
   vma->vm_flags |= VM_IO | VM_RESERVED | VM_PFNMAP | VM_DONTCOPY | VM_DONTEXPAND;
   vma->vm_ops = &vcsm_vm_ops;
   vma->vm_private_data = global_resource;

   /* vm_pgoff is the first PFN of the mapped memory */
   vma->vm_pgoff = (unsigned long)resource->res_base_mem & 0x3FFFFFFF;;
   vma->vm_pgoff += mm_vc_mem_phys_addr;
   vma->vm_pgoff >>= PAGE_SHIFT;

   if ( (resource->res_cached == VMCS_SM_CACHE_NONE) ||
        (resource->res_cached == VMCS_SM_CACHE_VC) )
   {
      /* Allocated non host cached memory, honour it.
      */
      vma->vm_page_prot = pgprot_noncached( vma->vm_page_prot );
   }

   LOG_DBG( (sm_debug_log >= LOG_DBG_LEVEL_MAX),
            "[%s]: resource %p (guid %x) - cnt %u, base address %p, handle %x, size %u (%u), cache %u",
            __func__,
            resource,
            resource->res_guid,
            resource->ref_count,
            resource->res_base_mem,
            resource->res_handle,
            resource->res_size,
            (unsigned int) (vma->vm_end - vma->vm_start),
            resource->res_cached );

   LOG_DBG( (sm_debug_log >= LOG_DBG_LEVEL_MAX),
            "[%s]: resource %p (base address %p, handle %x) - map-count %d, usr-addr %x",
            __func__,
            resource,
            resource->res_base_mem,
            resource->res_handle,
            resource->map_count,
            (unsigned int)vma->vm_start );

   vcsm_vma_open(vma);
   resource->res_stats[MAP]++;
   return 0;

 error:
   resource->res_stats[MAP_FAIL]++;
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
      LOG_ERR( "[%s]: cmd %x current tgid %u != %u owner",
               __func__,
               cmdnr,
               current->tgid,
               file_data->pid );

      ret = -EPERM;
      goto out;
   }

   /* If this action is a re-post of a previously interrupted action, tell what
   ** needs to be undone...
   */
   if ( file_data->restart_sys == VCOS_EINTR )
   {
      VC_SM_ACTION_CLEAN_T action_clean;

      LOG_DBG( (sm_debug_log >= LOG_DBG_LEVEL_MIN),
               "[%s]: clean up of action %u (trans_id: %u) following VCOS_EINTR",
               __func__,
               file_data->int_action,
               file_data->int_trans_id );

      action_clean.res_action = file_data->int_action;
      action_clean.action_trans_id = file_data->int_trans_id;

      vc_vchi_sm_clean_up( sm_state->sm_handle,
                           &action_clean );

      file_data->restart_sys = VCOS_SUCCESS;
   }

   /* Now process the command.
   */
   switch ( cmdnr )
   {
      /* New memory allocation.
      */
      case VMCS_SM_CMD_ALLOC:
      {
         VCOS_STATUS_T               status;
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
         INIT_LIST_HEAD(&resource->map_list);
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
         alloc.allocator  = current->tgid;
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

         LOG_DBG( (sm_debug_log >= LOG_DBG_LEVEL_MIN),
                  "[%s]: attempt to allocate \"%s\" data - type %u, base %u (%u), num %u, alignement %u",
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

         status = vc_vchi_sm_alloc( sm_state->sm_handle,
                                    &alloc,
                                    &alloc_result,
                                    &(file_data->int_trans_id) );
         if ( status == VCOS_EINTR )
         {
            LOG_DBG( (sm_debug_log >= LOG_DBG_LEVEL_INTER_1),
                     "[%s]: requesting allocate memory action restart (trans_id: %u)",
                     __func__,
                     file_data->int_trans_id );

            ret = -ERESTARTSYS;
            /* Free up resource, it will be re-created on the next attempt.
            */
            resource->res_stats[ALLOC]--;
            file_data->restart_sys = VCOS_EINTR;
            file_data->int_action = VC_SM_MSG_TYPE_ALLOC;
            goto free_resource;
         }
         else if( (status != VCOS_SUCCESS) ||
                  ((status == VCOS_SUCCESS) && (alloc_result.res_mem == NULL)) )
         {
            LOG_ERR( "[%s]: failed to allocate memory on videocore (status: %u, trans_id: %u)",
                     __func__,
                     status,
                     file_data->int_trans_id );

            ret = -ENOMEM;
            resource->res_stats[ALLOC_FAIL]++;
            goto free_resource;
         }

         file_data->restart_sys = VCOS_SUCCESS;

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

         LOG_DBG( (sm_debug_log >= LOG_DBG_LEVEL_MIN),
                  "[%s]: allocated data - guid %x, hdl %x, base address %p, size %d, cache %d",
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
         VCOS_STATUS_T                     status;
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

         lock_unlock.res_handle = resource->res_handle;
         lock_unlock.res_mem    = resource->res_base_mem;

         LOG_DBG( (sm_debug_log >= LOG_DBG_LEVEL_INTER_1),
                  "[%s]: attempt to lock-cache data - guid %x, hdl %x, base address %p",
                  __func__,
                  ioparam.handle,
                  lock_unlock.res_handle,
                  lock_unlock.res_mem );

         /* Lock the videocore allocated resource.
         */
         status = vc_vchi_sm_lock( sm_state->sm_handle,
                                   &lock_unlock,
                                   &lock_result,
                                   &(file_data->int_trans_id) );

         if ( status == VCOS_EINTR )
         {
            LOG_DBG( (sm_debug_log >= LOG_DBG_LEVEL_INTER_1),
                     "[%s]: requesting lock-cache memory action restart (trans_id: %u)",
                     __func__,
                     file_data->int_trans_id );

            ret = -ERESTARTSYS;
            resource->res_stats[LOCK]--;
            file_data->restart_sys = VCOS_EINTR;
            file_data->int_action = VC_SM_MSG_TYPE_LOCK;
            goto out;
         }
         else if ( (status != VCOS_SUCCESS) ||
                   ((status == VCOS_SUCCESS) && (lock_result.res_mem == NULL)) )
         {
            LOG_ERR( "[%s]: failed to lock-cache memory on videocore (status: %u, trans_id: %u)",
                     __func__,
                     status,
                     file_data->int_trans_id );

            ret = -EPERM;
            resource->res_stats[LOCK_FAIL]++;
            goto out;
         }
         resource->res_stats[LOCK]++;

         file_data->restart_sys = VCOS_SUCCESS;

         /* Successfully locked, increase the reference count for this resource.
         */
         resource->ref_count++;

         LOG_DBG( (sm_debug_log >= LOG_DBG_LEVEL_INTER_1),
                  "[%s]: succeed to lock-cache data - hdl %x, base address %p, ref-cnt %d",
                  __func__,
                  lock_unlock.res_handle,
                  lock_unlock.res_mem,
                  resource->ref_count );

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
         VCOS_STATUS_T                     status;
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

         lock_unlock.res_handle = resource->res_handle;
         lock_unlock.res_mem    = resource->res_base_mem;

         LOG_DBG( (sm_debug_log >= LOG_DBG_LEVEL_INTER_1),
                  "[%s]: attempt to lock data - guid %x, hdl %x, base address %p",
                  __func__,
                  ioparam.handle,
                  lock_unlock.res_handle,
                  lock_unlock.res_mem );

         /* Lock the videocore allocated resource.
         */
         status = vc_vchi_sm_lock( sm_state->sm_handle,
                                   &lock_unlock,
                                   &lock_result,
                                   &(file_data->int_trans_id) );

         if ( status == VCOS_EINTR )
         {
            LOG_DBG( (sm_debug_log >= LOG_DBG_LEVEL_INTER_1),
                     "[%s]: requesting lock memory action restart (trans_id: %u)",
                     __func__,
                     file_data->int_trans_id );

            ret = -ERESTARTSYS;
            resource->res_stats[LOCK]--;
            file_data->restart_sys = VCOS_EINTR;
            file_data->int_action = VC_SM_MSG_TYPE_LOCK;
            goto out;
         }
         else if ( (status != VCOS_SUCCESS) ||
                   ((status == VCOS_SUCCESS) && (lock_result.res_mem == NULL)) )
         {
            LOG_ERR( "[%s]: failed to lock memory on videocore (status: %u, trans_id: %u)",
                     __func__,
                     status,
                     file_data->int_trans_id );

            ret = -EPERM;
            resource->res_stats[LOCK_FAIL]++;
            goto out;
         }
         resource->res_stats[LOCK]++;

         file_data->restart_sys = VCOS_SUCCESS;

         /* Successfully locked, increase the reference count for this resource.
         */
         resource->ref_count++;

         LOG_DBG( (sm_debug_log >= LOG_DBG_LEVEL_INTER_1),
                  "[%s]: succeed to lock data - hdl %x, base address %p, ref-cnt %d",
                  __func__,
                  lock_unlock.res_handle,
                  lock_unlock.res_mem,
                  resource->ref_count );

         /* If the memory was never mapped, the call below will trigger a kernel error
         ** to pop up which is not very desirable even though it is harmless in this
         ** particular case.
         */
         if ( resource->map_count )
         {
            ioparam.addr = vmcs_sm_usr_address_from_pid_and_usr_handle( file_data->pid,
                                                                        ioparam.handle );
         }
         else
         {
            ioparam.addr = 0;
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
         VCOS_STATUS_T                     status;
         VC_SM_LOCK_UNLOCK_T               lock_unlock;
         struct vmcs_sm_ioctl_lock_unlock  ioparam;
         SM_GLOBAL_MAP_T *map;

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

         lock_unlock.res_handle = resource->res_handle;
         lock_unlock.res_mem    = resource->res_base_mem;

         LOG_DBG( (sm_debug_log >= LOG_DBG_LEVEL_INTER_1),
                  "[%s]: attempt to unlock data - guid %x, hdl %x, base address %p",
                  __func__,
                  ioparam.handle,
                  lock_unlock.res_handle,
                  lock_unlock.res_mem );

         /* We need to zap all the vmas associated with this resource */
         if ( resource->ref_count == 1 )
         {
            down_read( &current->mm->mmap_sem );
            if ( !list_empty ( &resource->map_list ) )
            {
               list_for_each_entry ( map, &resource->map_list, resource_map_list )
               {
                  if ( map->vma )
                  {
                     zap_vma_ptes( map->vma, map->vma->vm_start, map->vma->vm_end - map->vma->vm_start );
                  }
               }
            }
            up_read( &current->mm->mmap_sem );
         }

         if ( resource->ref_count )
         {
            /* Unlock the videocore allocated resource.
            */
            status = vc_vchi_sm_unlock( sm_state->sm_handle,
                                        &lock_unlock,
                                        &(file_data->int_trans_id) );

            if ( status == VCOS_EINTR )
            {
               LOG_DBG( (sm_debug_log >= LOG_DBG_LEVEL_INTER_1),
                        "[%s]: requesting unlock memory action restart (trans_id: %u)",
                        __func__,
                        file_data->int_trans_id );

               ret = -ERESTARTSYS;
               resource->res_stats[UNLOCK]--;
               file_data->restart_sys = VCOS_EINTR;
               file_data->int_action = VC_SM_MSG_TYPE_UNLOCK;
               goto out;
            }
            else if ( status != VCOS_SUCCESS )
            {
               LOG_ERR( "[%s]: failed to unlock memory on videocore (status: %u, trans_id: %u)",
                        __func__,
                        status,
                        file_data->int_trans_id );

               ret = -EPERM;
               resource->res_stats[UNLOCK_FAIL]++;
               goto out;
            }

            resource->res_stats[UNLOCK]++;
            file_data->restart_sys = VCOS_SUCCESS;

            /* Successfully unlocked, decrease the reference count for this resource.
            */
            resource->ref_count--;
         }

         LOG_DBG( (sm_debug_log >= LOG_DBG_LEVEL_INTER_1),
                  "[%s]: success to unlock data - hdl %x, base address %p, ref-cnt %d",
                  __func__,
                  lock_unlock.res_handle,
                  lock_unlock.res_mem,
                  resource->ref_count );

         /* Done.
         */
         goto out;
      }
      break;

      /* Resize (attempt to) existing memory allocation.
      */
      case VMCS_SM_CMD_RESIZE:
      {
         VCOS_STATUS_T                     status;
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
         if ( (resource != NULL) && (resource->ref_count != 0) )
         {
            LOG_ERR( "[%s]: cannot resize locked resource - guid %x, ref-cnt %d",
                     __func__,
                     ioparam.handle,
                     resource->ref_count );

            ret = -EFAULT;
            goto out;
         }

         /* If the resource is mapped, we cannot resize it either
         */
         if ( resource->map_count != 0 )
         {
            LOG_ERR( "[%s]: cannot resize mapped resource - guid %x, ref-cnt %d",
                     __func__,
                     ioparam.handle,
                     resource->map_count );

            ret = -EFAULT;
            goto out;
         }

         resize.res_handle   = (resource != NULL) ? resource->res_handle : 0;
         resize.res_mem      = (resource != NULL) ? resource->res_base_mem : NULL;
         resize.res_new_size = ioparam.new_size;

         LOG_DBG( (sm_debug_log >= LOG_DBG_LEVEL_MAX),
                  "[%s]: attempt to resize data - guid %x, hdl %x, base address %p",
                  __func__,
                  ioparam.handle,
                  resize.res_handle,
                  resize.res_mem );

         /* Resize the videocore allocated resource.
         */
         status = vc_vchi_sm_resize( sm_state->sm_handle,
                                     &resize,
                                     &(file_data->int_trans_id) );

         if ( status == VCOS_EINTR )
         {
            LOG_DBG( (sm_debug_log >= LOG_DBG_LEVEL_MAX),
                     "[%s]: requesting resize memory action restart (trans_id: %u)",
                     __func__,
                     file_data->int_trans_id );

            ret = -ERESTARTSYS;
            file_data->restart_sys = VCOS_EINTR;
            file_data->int_action = VC_SM_MSG_TYPE_RESIZE;
            goto out;
         }
         else if ( status != VCOS_SUCCESS )
         {
            LOG_ERR( "[%s]: failed to resize memory on videocore (status: %u, trans_id: %u)",
                     __func__,
                     status,
                     file_data->int_trans_id );

            ret = -EPERM;
            goto out;
         }

         file_data->restart_sys = VCOS_SUCCESS;

         LOG_DBG( (sm_debug_log >= LOG_DBG_LEVEL_MAX),
                  "[%s]: success to resize data - hdl %x, size %d -> %d",
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
         VCOS_STATUS_T              status;
         VC_SM_FREE_T               free;
         struct vmcs_sm_ioctl_free  ioparam;
         SM_GLOBAL_MAP_T *map, *map_tmp;

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

         LOG_DBG( (sm_debug_log >= LOG_DBG_LEVEL_MIN),
                  "[%s]: attempt to free data - guid %x, hdl %x, base address %p",
                  __func__,
                  ioparam.handle,
                  free.res_handle,
                  free.res_mem );

         /* Make sure the resource we're removing is unmapped first */
         down_write( &current->mm->mmap_sem );
         if ( resource->map_count && !list_empty ( &resource->map_list ) )
         {
            list_for_each_entry_safe( map, map_tmp, &resource->map_list, resource_map_list )
            {
               ret = do_munmap( current->mm, map->res_addr, resource->res_size );
            }
         }
         up_write( &current->mm->mmap_sem );

         /* Free up the videocore allocated resource.
         */
         status = vc_vchi_sm_free( sm_state->sm_handle,
                                   &free,
                                   &(file_data->int_trans_id) );
         if ( status == VCOS_EINTR )
         {
            LOG_DBG( (sm_debug_log >= LOG_DBG_LEVEL_MIN),
                     "[%s]: requesting free memory action restart (trans_id: %u)",
                     __func__,
                     file_data->int_trans_id );

            ret = -ERESTARTSYS;
            resource->res_stats[FREE]--;
            file_data->restart_sys = VCOS_EINTR;
            file_data->int_action = VC_SM_MSG_TYPE_FREE;
            goto out;
         }
         else if ( status != VCOS_SUCCESS )
         {
            LOG_ERR( "[%s]: failed to free memory on videocore (status: %u, trans_id: %u)",
                     __func__,
                     status,
                     file_data->int_trans_id );

            resource->res_stats[FREE_FAIL]++;
            ret = -EPERM;
            // goto out;
            /* Fall below anyways...
            */
         }

         file_data->restart_sys = VCOS_SUCCESS;

         LOG_DBG( (sm_debug_log >= LOG_DBG_LEVEL_MIN),
                  "[%s]: %s to free data - hdl %x, base address %p",
                  __func__,
                  ( status == VCOS_SUCCESS ) ? "SUCCEEDED" : "FAILED",
                  free.res_handle,
                  free.res_mem );

         /* Free up the local resource tracking this allocation.
         */
         vmcs_sm_remove_resource ( file_data,
                                   resource );

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
         LOG_DBG( (sm_debug_log >= LOG_DBG_LEVEL_MAX),
                  "[%s]: invoking walk alloc",
                  __func__ );

         if ( vc_vchi_sm_walk_alloc( sm_state->sm_handle ) != VCOS_SUCCESS )
         {
            /* This is just a debug-print of the videocore allocations, so no big deal if
            ** it failed.
            */
            LOG_ERR( "[%s]: failed to walk-alloc on videocore",
                     __func__ );
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

            // L1 cache flush
            down_read( &current->mm->mmap_sem );
            vcsm_vma_cache_clean_page_range((unsigned long)ioparam.addr,
                                            (unsigned long)ioparam.addr + ioparam.size );
            up_read( &current->mm->mmap_sem );

            // L2 cache flush
            outer_clean_range( phys_addr,
                               phys_addr + (size_t)ioparam.size );
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

            // L2 cache invalidate
            outer_inv_range( phys_addr,
                             phys_addr + (size_t)ioparam.size );

            // L1 cache invalidate
            down_read( &current->mm->mmap_sem );
            vcsm_vma_cache_clean_page_range((unsigned long)ioparam.addr,
                                            (unsigned long)ioparam.addr + ioparam.size );
            up_read( &current->mm->mmap_sem );
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

/* Videocore connected.
*/
static void vc_sm_connected_init( void )
{
   int                    ret;
   VCHI_INSTANCE_T        vchi_instance;
   VCHI_CONNECTION_T      *vchi_connection;

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

   sm_state->debug = create_proc_entry( PROC_DEBUG,
                                        0,
                                        sm_state->dir_root );
   if ( sm_state->debug == NULL )
   {
      LOG_ERR( "[%s]: failed to create \'%s\' entry",
               __func__,
               PROC_DEBUG );

      ret = -EPERM;
      goto err_remove_proc_dir;
   }
   else
   {
      sm_state->debug->read_proc = &vc_sm_debug_proc_read;
      sm_state->debug->write_proc = &vc_sm_debug_proc_write;
   }

   sm_state->dir_state.dir_entry = create_proc_entry( PROC_STATE,
                                                      0,
                                                      sm_state->dir_root );
   if ( sm_state->dir_state.dir_entry == NULL )
   {
      LOG_ERR( "[%s]: failed to create \'%s\' entry",
               __func__,
               PROC_STATE );

      ret = -EPERM;
      goto err_remove_proc_debug;
   }
   else
   {
      sm_state->dir_state.priv_data = NULL;
      sm_state->dir_state.proc_read = &vc_sm_global_state_proc_read;

      sm_state->dir_state.dir_entry->proc_fops = &vc_sm_proc_fops;
      sm_state->dir_state.dir_entry->data = &(sm_state->dir_state);
   }

   sm_state->dir_stats.dir_entry = create_proc_entry( PROC_STATS,
                                                      0,
                                                      sm_state->dir_root );
   if ( sm_state->dir_stats.dir_entry == NULL )
   {
      LOG_ERR( "[%s]: failed to create \'%s\' entry",
               __func__,
               PROC_STATS );

      ret = -EPERM;
      goto err_remove_proc_state;
   }
   else
   {
      sm_state->dir_stats.priv_data = NULL;
      sm_state->dir_stats.proc_read = &vc_sm_global_statistics_proc_read;

      sm_state->dir_stats.dir_entry->proc_fops = &vc_sm_proc_fops;
      sm_state->dir_stats.dir_entry->data = &(sm_state->dir_stats);
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

   sm_state->dir_guid = create_proc_entry( PROC_CFG_GUID_SHIFT,
                                           0644,
                                           sm_state->dir_cfg );
   if ( sm_state->dir_guid == NULL )
   {
      LOG_ERR( "[%s]: failed to create \'%s\' entry",
               __func__,
               PROC_CFG_GUID_SHIFT );

      ret = -EPERM;
      goto err_remove_alloc_dir;
   }
   sm_state->dir_guid->read_proc  = &vc_sm_cfg_proc_read;
   sm_state->dir_guid->write_proc = &vc_sm_cfg_proc_write;
   sm_state->dir_guid->data       = NULL;

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

   INIT_LIST_HEAD(&sm_state->map_list);

   sm_state->data_knl = vc_sm_create_priv_data ( 0 );
   if ( sm_state->data_knl == NULL )
   {
      LOG_ERR( "[%s]: failed to create kernel private data tracker",
               __func__);
      goto err_remove_shared_memory;
   }

   /* Done!
   */
   sm_inited = 1;
   goto out;

err_remove_shared_memory:
   vc_sm_remove_sharedmemory();
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
err_remove_proc_debug:
   remove_proc_entry( PROC_DEBUG, sm_state->dir_root );
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
}

/* Driver loading.
*/
static int __init vc_sm_init( void )
{
   printk( KERN_INFO "vc-sm: Videocore shared memory driver\n" );

   vchiq_add_connected_callback( vc_sm_connected_init );
   return 0;
}


/* Driver unloading.
*/
static void __exit vc_sm_exit( void )
{
   LOG_INFO( "[%s]: start",
             __func__ );

   if ( sm_inited )
   {
      /* Remove shared memory device.
      */
      vc_sm_remove_sharedmemory();

      /* Remove all proc entries.
      */
      remove_proc_entry( PROC_CFG_GUID_SHIFT, sm_state->dir_cfg );
      remove_proc_entry( PROC_DIR_CFG_NAME,   sm_state->dir_root );
      remove_proc_entry( PROC_DIR_ALLOC_NAME, sm_state->dir_root );
      remove_proc_entry( PROC_DEBUG,          sm_state->dir_root );
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
   }

   LOG_INFO( "[%s]: end",
             __func__ );
}

#if defined(__KERNEL__)
/* Allocate a shared memory handle and block.
*/
int vc_sm_alloc( VC_SM_ALLOC_T *alloc, int *handle )
{
   int ret                   = 0;
   VCOS_STATUS_T status;
   VC_SM_ALLOC_RESULT_T alloc_result;
   SM_RESOURCE_T *resource   = NULL;
   int trans_id;

   /* Validate we can work with this device.
   */
   if ( (sm_state == NULL) || (alloc == NULL) || (handle == NULL) )
   {
      LOG_ERR( "[%s]: invalid input",
               __func__ );

      ret = -EPERM;
      goto out;
   }

   *handle = 0;

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
   INIT_LIST_HEAD(&resource->map_list);
   resource->res_stats[ALLOC]++;

   /* Allocate the videocore resource.
   */
   LOG_DBG( (sm_debug_log >= LOG_DBG_LEVEL_MIN),
            "[%s]: attempt to allocate \"%s\" data - type %u, base %u (%u), num %u, alignement %u",
            __func__,
            alloc->name,
            alloc->type,
            alloc->base_unit,
            (alloc->base_unit + alloc->alignement - 1) & ~(alloc->alignement - 1),
            alloc->num_unit,
            alloc->alignement );

   /* Align the size asked for to the kernel page size.
   */
   alloc->base_unit =
      (alloc->base_unit + alloc->alignement - 1) & ~(alloc->alignement - 1);

   status = vc_vchi_sm_alloc( sm_state->sm_handle,
                              alloc,
                              &alloc_result,
                              &trans_id );
   if ( status == VCOS_EINTR )
   {
      LOG_DBG( (sm_debug_log >= LOG_DBG_LEVEL_INTER_1),
               "[%s]: requesting allocate memory action restart (trans_id: %u)",
               __func__,
               trans_id );

      ret = -ERESTARTSYS;
      /* Free up resource, it will be re-created on the next attempt.
      */
      resource->res_stats[ALLOC]--;
      goto free_resource;
   }
   else if( (status != VCOS_SUCCESS) ||
            ((status == VCOS_SUCCESS) && (alloc_result.res_mem == NULL)) )
   {
      LOG_ERR( "[%s]: failed to allocate memory on videocore (status: %u, trans_id: %u)",
               __func__,
               status,
               trans_id );

      ret = -ENOMEM;
      resource->res_stats[ALLOC_FAIL]++;
      goto free_resource;
   }

   /* Keep track of the resource we created.
   */
   resource->res_handle   = alloc_result.res_handle;
   resource->res_base_mem = alloc_result.res_mem;
   resource->res_size     = alloc->base_unit * alloc->num_unit;
   resource->res_cached   = alloc->type;

   resource->res_guid     = sm_state->data_knl->guid++;
   *handle                = resource->res_guid;

   vmcs_sm_add_resource ( sm_state->data_knl,
                          resource );

   LOG_DBG( (sm_debug_log >= LOG_DBG_LEVEL_MIN),
            "[%s]: allocated data - guid %x, hdl %x, base address %p, size %d, cache %d",
            __func__,
            resource->res_guid,
            resource->res_handle,
            resource->res_base_mem,
            resource->res_size,
            resource->res_cached );

   /* Done.
   */
   goto out;

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
EXPORT_SYMBOL_GPL(vc_sm_alloc);

/* Get an internal resource handle mapped from the external one.
*/
int vc_sm_int_handle( int handle )
{
   int ret = 0;
   SM_RESOURCE_T *resource   = NULL;

   /* Validate we can work with this device.
   */
   if ( (sm_state == NULL) || (handle == 0) )
   {
      LOG_ERR( "[%s]: invalid input",
               __func__ );

      ret = -EPERM;
      goto out;
   }

   /* Locate resource from GUID.
   */
   resource = vmcs_sm_get_resource_with_guid ( sm_state->data_knl,
                                               handle );
   if ( resource == NULL )
   {
      goto out;
   }
   else
   {
      ret = resource->res_handle;
   }


   /* Done.
   */
   goto out;

out:
   return ret;
}
EXPORT_SYMBOL_GPL(vc_sm_int_handle);

/* Free a previously allocated shared memory handle and block.
*/
int vc_sm_free( int handle )
{
   int ret = 0;
   SM_RESOURCE_T *resource   = NULL;
   VCOS_STATUS_T status;
   VC_SM_FREE_T free;
   int trans_id;

   /* Validate we can work with this device.
   */
   if ( (sm_state == NULL) || (handle == 0) )
   {
      LOG_ERR( "[%s]: invalid input",
               __func__ );

      ret = -EPERM;
      goto out;
   }

   /* Locate resource from GUID.
   */
   resource = vmcs_sm_get_resource_with_guid ( sm_state->data_knl,
                                               handle );
   if ( resource == NULL )
   {
      ret = -EINVAL;
      goto out;
   }
   resource->res_stats[FREE]++;

   free.res_handle = resource->res_handle;
   free.res_mem    = resource->res_base_mem;

   LOG_DBG( (sm_debug_log >= LOG_DBG_LEVEL_MIN),
            "[%s]: attempt to free data - guid %x, hdl %x, base address %p",
            __func__,
            handle,
            free.res_handle,
            free.res_mem );

   /* Free up the videocore allocated resource.
   */
   status = vc_vchi_sm_free( sm_state->sm_handle,
                             &free,
                             &trans_id );
   if ( status == VCOS_EINTR )
   {
      LOG_DBG( (sm_debug_log >= LOG_DBG_LEVEL_MIN),
               "[%s]: requesting free memory action restart (trans_id: %u)",
               __func__,
               trans_id );

      ret = -ERESTARTSYS;
      resource->res_stats[FREE]--;
      goto out;
   }
   else if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "[%s]: failed to free memory on videocore (status: %u, trans_id: %u)",
               __func__,
               status,
               trans_id );

      resource->res_stats[FREE_FAIL]++;
      ret = -EPERM;
      // goto out;
      /* Fall below anyways...
      */
   }

   LOG_DBG( (sm_debug_log >= LOG_DBG_LEVEL_MIN),
            "[%s]: %s to free data - hdl %x, base address %p",
            __func__,
            ( status == VCOS_SUCCESS ) ? "SUCCEEDED" : "FAILED",
            free.res_handle,
            free.res_mem );

   /* Free up the local resource tracking this allocation.
   */
   vmcs_sm_remove_resource ( sm_state->data_knl,
                             resource );

   /* Done.
   */
   goto out;

out:
   return ret;
}
EXPORT_SYMBOL_GPL(vc_sm_free);

/* Lock a memory handle for use by kernel.
*/
int vc_sm_lock( int handle, VC_SM_LOCK_CACHE_MODE_T mode, long unsigned int *data )
{
   int ret = 0;
   VCOS_STATUS_T status;
   VC_SM_LOCK_UNLOCK_T lock_unlock;
   VC_SM_LOCK_RESULT_T lock_result;
   int trans_id;
   SM_RESOURCE_T *resource   = NULL;

   /* Validate we can work with this device.
   */
   if ( (sm_state == NULL) || (handle == 0) || (data == NULL) )
   {
      LOG_ERR( "[%s]: invalid input",
               __func__ );

      ret = -EPERM;
      goto out;
   }

   *data = 0;

   /* Locate resource from GUID.
   */
   resource = vmcs_sm_get_resource_with_guid ( sm_state->data_knl,
                                               handle );
   if ( resource == NULL )
   {
      ret = -EINVAL;
      goto out;
   }

   lock_unlock.res_handle = resource->res_handle;
   lock_unlock.res_mem    = resource->res_base_mem;

   LOG_DBG( (sm_debug_log >= LOG_DBG_LEVEL_INTER_1),
            "[%s]: attempt to lock data - guid %x, hdl %x, base address %p",
            __func__,
            handle,
            lock_unlock.res_handle,
            lock_unlock.res_mem );

   /* Lock the videocore allocated resource.
   */
   status = vc_vchi_sm_lock( sm_state->sm_handle,
                             &lock_unlock,
                             &lock_result,
                             &trans_id );

   if ( status == VCOS_EINTR )
   {
      LOG_DBG( (sm_debug_log >= LOG_DBG_LEVEL_INTER_1),
               "[%s]: requesting lock memory action restart (trans_id: %u)",
               __func__,
               trans_id );

      ret = -ERESTARTSYS;
      resource->res_stats[LOCK]--;
      goto out;
   }
   else if ( (status != VCOS_SUCCESS) ||
             ((status == VCOS_SUCCESS) && (lock_result.res_mem == NULL)) )
   {
      LOG_ERR( "[%s]: failed to lock memory on videocore (status: %u, trans_id: %u)",
               __func__,
               status,
               trans_id );

      ret = -EPERM;
      resource->res_stats[LOCK_FAIL]++;
      goto out;
   }
   resource->res_stats[LOCK]++;

   /* Successfully locked, increase the reference count for this resource.
   */
   resource->ref_count++;

   LOG_DBG( (sm_debug_log >= LOG_DBG_LEVEL_INTER_1),
            "[%s]: succeed to lock data - hdl %x, base address %p, ref-cnt %d",
            __func__,
            lock_unlock.res_handle,
            lock_unlock.res_mem,
            resource->ref_count );

   /* Keep track of the new base memory allocation if it has changed.
   */
   if ( (lock_result.res_mem != NULL) &&
        (lock_result.res_old_mem != NULL) &&
        (lock_result.res_mem != lock_result.res_old_mem) )
   {
      resource->res_base_mem = lock_result.res_mem;
   }

   /* If the memory was never mapped, the call below will trigger a kernel error
   ** to pop up which is not very desirable even though it is harmless in this
   ** particular case.
   */
   if ( resource->map_count )
   {
      *data = vmcs_sm_usr_address_from_pid_and_usr_handle( 0,
                                                           handle );
   }
   else
   {
      /* Keep track of the tuple (pid, virtual address, videocore memory handle) in the
      ** global resource list such that one can do a mapping lookup for address/memory handle
      ** as needed.
      */
      SM_GLOBAL_MAP_T *global_resource = kzalloc( sizeof( *global_resource ), GFP_KERNEL );
      long unsigned int phys_addr;

      if ( global_resource == NULL )
      {
         LOG_ERR( "[%s]: failed to allocate global tracking resource",
                  __func__ );
         ret = -ENOMEM;
         goto out;
      }

      phys_addr = (uint32_t)resource->res_base_mem & 0x3FFFFFFF;
      phys_addr += mm_vc_mem_phys_addr;
      if ( mode == VC_SM_LOCK_CACHED )
      {
         resource->res_cached = VMCS_SM_CACHE_HOST;
         *data = (long unsigned int) ioremap_cached( phys_addr,
                                                     resource->res_size );
      }
      else
      {
         resource->res_cached = VMCS_SM_CACHE_NONE;
         *data = (long unsigned int) ioremap_nocache( phys_addr,
                                                      resource->res_size );
      }

      global_resource->res_pid     = 0;
      global_resource->res_vc_hdl  = resource->res_handle;
      global_resource->res_usr_hdl = resource->res_guid;
      global_resource->res_addr    = *data;
      global_resource->resource    = resource;
      global_resource->vma         = NULL;

      vmcs_sm_add_map( sm_state, resource, global_resource );
   }

   /* Done.
   */
   goto out;

out:
   return ret;
}
EXPORT_SYMBOL_GPL(vc_sm_lock);

/* Unlock a memory handle in use by kernel.
*/
int vc_sm_unlock( int handle, int flush )
{
   int ret = 0;
   VCOS_STATUS_T status;
   VC_SM_LOCK_UNLOCK_T lock_unlock;
   SM_RESOURCE_T *resource   = NULL;
   SM_GLOBAL_MAP_T *map, *map_tmp;
   int trans_id;

   /* Validate we can work with this device.
   */
   if ( (sm_state == NULL) || (handle == 0) )
   {
      LOG_ERR( "[%s]: invalid input",
               __func__ );

      ret = -EPERM;
      goto out;
   }

   /* Locate resource from GUID.
   */
   resource = vmcs_sm_get_resource_with_guid ( sm_state->data_knl,
                                               handle );
   if ( resource == NULL )
   {
      ret = -EINVAL;
      goto out;
   }

   lock_unlock.res_handle = resource->res_handle;
   lock_unlock.res_mem    = resource->res_base_mem;

   LOG_DBG( (sm_debug_log >= LOG_DBG_LEVEL_INTER_1),
            "[%s]: attempt to unlock data - guid %x, hdl %x, base address %p",
            __func__,
            handle,
            lock_unlock.res_handle,
            lock_unlock.res_mem );

   if ( resource->ref_count == 1 )
   {
      if ( !list_empty ( &resource->map_list ) )
      {
         list_for_each_entry_safe ( map, map_tmp, &resource->map_list, resource_map_list )
         {
            if ( map->res_addr )
            {
               if ( flush && (resource->res_cached == VMCS_SM_CACHE_HOST) )
               {
                  long unsigned int phys_addr;
                  phys_addr = (uint32_t)resource->res_base_mem & 0x3FFFFFFF;
                  phys_addr += mm_vc_mem_phys_addr;

                  // L1 cache flush
                  dmac_flush_range( (const void *)map->res_addr,
                                    (const void *)(map->res_addr + resource->res_size) );

                  // L2 cache flush
                  outer_clean_range( phys_addr,
                                     phys_addr + (size_t)resource->res_size );
               }

               iounmap ( (void *)map->res_addr );
               map->res_addr = 0;

               vmcs_sm_remove_map( sm_state, map->resource, map );
               break;
            }
         }
      }
   }

   if ( resource->ref_count )
   {
      /* Unlock the videocore allocated resource.
      */
      status = vc_vchi_sm_unlock( sm_state->sm_handle,
                                  &lock_unlock,
                                  &trans_id );

      if ( status == VCOS_EINTR )
      {
         LOG_DBG( (sm_debug_log >= LOG_DBG_LEVEL_INTER_1),
                  "[%s]: requesting unlock memory action restart (trans_id: %u)",
                  __func__,
                  trans_id );

         ret = -ERESTARTSYS;
         resource->res_stats[UNLOCK]--;
         goto out;
      }
      else if ( status != VCOS_SUCCESS )
      {
         LOG_ERR( "[%s]: failed to unlock memory on videocore (status: %u, trans_id: %u)",
                  __func__,
                  status,
                  trans_id );

         ret = -EPERM;
         resource->res_stats[UNLOCK_FAIL]++;
         goto out;
      }

      resource->res_stats[UNLOCK]++;

      /* Successfully unlocked, decrease the reference count for this resource.
      */
      resource->ref_count--;
   }

   LOG_DBG( (sm_debug_log >= LOG_DBG_LEVEL_INTER_1),
            "[%s]: success to unlock data - hdl %x, base address %p, ref-cnt %d",
            __func__,
            lock_unlock.res_handle,
            lock_unlock.res_mem,
            resource->ref_count );

   /* Done.
   */
   goto out;

out:
   return ret;
}
EXPORT_SYMBOL_GPL(vc_sm_unlock);
#endif


module_init( vc_sm_init );
module_exit( vc_sm_exit );

MODULE_AUTHOR( "Broadcom" );
MODULE_DESCRIPTION( "VideoCore SharedMemory Driver" );
MODULE_LICENSE( "GPL v2" );
