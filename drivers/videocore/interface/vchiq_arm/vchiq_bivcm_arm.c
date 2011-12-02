/*
 * Copyright (c) 2010-2011 Broadcom Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/pagemap.h>
#include <asm/pgtable.h>
#include <asm/io.h>
#include <asm/uaccess.h>

#include <vchiq_platform_data.h>

#include "vchiq_arm.h"
#include "vchiq_bivcm.h"
#include "vchiq_connected.h"

#include "vchiq_memdrv.h"

#include <linux/dma-mapping.h>
#include <mach/sdma.h>
#include <mach/dma_mmap.h>
#include <mach/aram_layout.h>
#include <asm/gpio.h>

#ifdef CONFIG_ARCH_KONA

#include <chal/chal_ipc.h>
#include <mach/irqs.h>
#include <chal/chal_icd.h>
#include <mach/io_map.h>
#define IPC_SHARED_MEM_BASE       KONA_INT_SRAM_BASE

#if defined( CONFIG_KONA_WFI_WORKAROUND )
#include <mach/wfi_count.h>
#endif

#else

#include <csp/chal_ipc.h>
#include <csp/chal_intid.h>
#include <csp/chal_icd.h>
#define IPC_SHARED_MEM_BASE       MM_IO_BASE_SRAM

#endif

#include <vc_mem.h>

#if defined(VCHIQ_SM_ALLOC_VCDDR)
#include "host_applications/linux/libs/debug_sym/debug_sym.h"
#endif

#if defined(VCHIQ_SM_ALLOC_VCDDR)
#include "host_applications/linux/libs/debug_sym/debug_sym.h"
#endif

#if defined(CONFIG_HAS_EARLYSUSPEND)
#include <linux/earlysuspend.h>

static void
vchiq_early_suspend(struct early_suspend *h);
static void
vchiq_late_resume(struct early_suspend *h);

static struct early_suspend g_vchiq_early_suspend =
{
   .level   = EARLY_SUSPEND_LEVEL_DISABLE_FB + 1,
   .suspend = vchiq_early_suspend,
   .resume  = vchiq_late_resume
};

#endif

#define TOTAL_SLOTS (VCHIQ_SLOT_ZERO_SLOTS + 2 * 32)

#define VCHIQ_DOORBELL_IRQ BCM_INT_ID_IPC_OPEN

#if defined(CONFIG_MAP_LITTLE_ISLAND_MODE) || defined(CONFIG_LITTLE_MODE)
#define ARM_RAM_BASE_IN_VC 0xc0000000
#else
#define ARM_RAM_BASE_IN_VC 0xe0000000
#endif
#define VIRT_TO_VC(x) PHYS_TO_VC((unsigned long)x - PAGE_OFFSET + PHYS_OFFSET)
#define PHYS_TO_VC(x) ((unsigned long)x - 0x80000000 + ARM_RAM_BASE_IN_VC)


#define VCOS_LOG_CATEGORY (&vchiq_arm_log_category)

#define IPC_SHARED_MEM_SLOTS_VIRT IPC_SHARED_MEM_SLOTS

#if ( BCMHANA_ARAM_VC_OFFSET != IPC_SHARED_MEM_VC_OFFSET )
#error BAMHANA ARAM VC OFFSET MUST be the same as IPC SHARED MEM VC OFFSET
#endif

typedef struct
{
    unsigned int                 instNum;

    const char                  *instance_name;
    const VCHIQ_PLATFORM_DATA_T *platform_data;

    VCOS_CFG_ENTRY_T             instance_cfg_dir;
    VCOS_CFG_ENTRY_T             vchiq_control_cfg_entry;
} VCHIQ_KERNEL_STATE_T;

typedef struct vchiq_bivcm_state_struct {
   int inited;
   VCHIQ_ARM_STATE_T arm_state;
} VCHIQ_BIVCM_ARM_STATE_T;

#define VCHIQ_NUM_VIDEOCORES 1



static VCHIQ_KERNEL_STATE_T    *vchiq_kernel_state[ VCHIQ_NUM_VIDEOCORES ];
static unsigned int             vchiq_num_instances = 0;

/* Global 'vc' proc entry used by all instances */
VCOS_CFG_ENTRY_T         vc_cfg_dir;

static CHAL_IPC_HANDLE   ipcHandle;
static VCOS_EVENT_T      g_pause_event;
static int               g_initialized;

static VCHIQ_STATE_T    *g_vchiq_state;
static VCHIQ_SLOT_ZERO_T *g_vchiq_slot_zero;
static unsigned int      g_wake_address;

static int          g_pagelist_count;
static FRAGMENTS_T *g_fragments_base;
static FRAGMENTS_T *g_free_fragments;
static int          g_free_fragments_count;
struct semaphore    g_free_fragments_sema;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36)
static DEFINE_SEMAPHORE(g_free_fragments_mutex);
#else
static DECLARE_MUTEX(g_free_fragments_mutex);
#endif

static int               g_use_autosuspend = 0;
#if VCOS_HAVE_TIMER
static int               g_use_suspend_timer = 1;
#endif
#ifdef CONFIG_HAS_EARLYSUSPEND
static int               g_early_susp_ctrl = 0;
static int               g_earlysusp_suspend_allowed = 0;
#endif
#if defined(VCHIQ_SM_ALLOC_VCDDR)
static VC_MEM_ACCESS_HANDLE_T g_vchiq_mem_hndl;
static void                   *g_vchiq_ipc_shared_mem;
static int                    g_vchiq_ipc_shared_mem_addr;
static int                    g_vchiq_ipc_shared_mem_size;
#endif

static irqreturn_t
vchiq_doorbell_irq(int irq, void *dev_id);

static int
create_pagelist(char __user *buf, size_t count, unsigned short type,
   struct task_struct *task, PAGELIST_T ** ppagelist);

static void
free_pagelist(PAGELIST_T *pagelist, int actual);


int __init
vchiq_platform_vcos_init(void)
{
   return 0;
}

int __init
vchiq_platform_init(VCHIQ_STATE_T *state)
{
   g_vchiq_state = state;
   g_wake_address = 0;

#if (( defined( CONFIG_ARCH_KONA ) || defined( CONFIG_ARCH_BCMHANA )) && !(defined( CONFIG_MAP_LITTLE_ISLAND_MODE ) || defined ( CONFIG_LITTLE_MODE )))

   /*
    * On Big Island, the videocore can only access the lower 512 Mb of the ARM memory.
    * So bivcm can only work if the host is located in the lower 512 Mb of physical memory.
    */

   #define MAX_BIVCM_MEM   (512 * 1024 * 1024)

   if (( num_physpages * PAGE_SIZE ) > MAX_BIVCM_MEM )
   {
      printk( KERN_ALERT "============================================================================\n" );
      printk( KERN_ALERT "============================================================================\n" );
      printk( KERN_ALERT "=====\n" );
      printk( KERN_ALERT "===== BIVCM can't be used when the kernel has more than 512 Mb of memory.\n" );
      printk( KERN_ALERT "===== Either limit the amount of memory by using mem=512M on the kernel\n" );
      printk( KERN_ALERT "===== command line, or switch to using BI instead.\n" );
      printk( KERN_ALERT "=====\n" );
      printk( KERN_ALERT "===== num_physpages: 0x%08lx (%4ld Mb)\n", num_physpages, num_physpages >> ( 20 - PAGE_SHIFT ));
      printk( KERN_ALERT "===== max supported: 0x%08lx (%4d Mb) num_physpages can't exceed this to use BIVCM\n", 
                         MAX_BIVCM_MEM / PAGE_SIZE, MAX_BIVCM_MEM >> 20 );
      printk( KERN_ALERT "=====\n" );
      printk( KERN_ALERT "============================================================================\n" );
      printk( KERN_ALERT "============================================================================\n" );

      BUG();
      return -ENOMEM;
   }
#endif
   vcos_event_create(&g_pause_event, "pause_event");

#ifdef CONFIG_HAS_EARLYSUSPEND
   register_early_suspend(&g_vchiq_early_suspend);
#endif

   return 0;
}

void __exit
vchiq_platform_exit(VCHIQ_STATE_T *state)
{
#ifdef CONFIG_HAS_EARLYSUSPEND
   unregister_early_suspend(&g_vchiq_early_suspend);
#endif
   vcos_assert(state == g_vchiq_state);
   g_vchiq_state = NULL;
   free_irq(VCHIQ_DOORBELL_IRQ, state);
}


VCHIQ_STATUS_T
vchiq_platform_init_state(VCHIQ_STATE_T *state)
{
   VCHIQ_STATUS_T status = VCHIQ_SUCCESS;
   state->platform_state = vcos_calloc(1, sizeof(VCHIQ_BIVCM_ARM_STATE_T), "VCHIQ_BIVCM_ARM_STATE");
   ((VCHIQ_BIVCM_ARM_STATE_T*)state->platform_state)->inited = 1;
   status = vchiq_arm_init_state(state, &((VCHIQ_BIVCM_ARM_STATE_T*)state->platform_state)->arm_state);
   if(status != VCHIQ_SUCCESS)
   {
      ((VCHIQ_BIVCM_ARM_STATE_T*)state->platform_state)->inited = 0;
   }
   return status;
}

VCHIQ_ARM_STATE_T*
vchiq_platform_get_arm_state(VCHIQ_STATE_T *state)
{
   if(!((VCHIQ_BIVCM_ARM_STATE_T*)state->platform_state)->inited)
   {
      BUG();
   }
   return &((VCHIQ_BIVCM_ARM_STATE_T*)state->platform_state)->arm_state;
}

void
remote_event_signal(REMOTE_EVENT_T *event)
{
   event->fired = 1;

   /* The test on the next line also ensures the write on the previous line
      has completed */

   if (event->armed)
   {
      /* trigger vc interrupt */
      dsb();         /* data barrier operation */

      chal_ipc_int_vcset( ipcHandle, IPC_INTERRUPT_SOURCE_0 );
   }
}

int
vchiq_copy_from_user(void *dst, const void *src, int size)
{
    if ( (uint32_t)src < TASK_SIZE)
    {
        return copy_from_user(dst, src, size);
    }
    else
    {
        memcpy( dst, src, size );
        return 0;
    }
}

VCHIQ_STATUS_T
vchiq_prepare_bulk_data(VCHIQ_BULK_T *bulk, VCHI_MEM_HANDLE_T memhandle,
   void *offset, int size, int dir)
{
   PAGELIST_T *pagelist;
   int ret;

   vcos_assert(memhandle == VCHI_MEM_HANDLE_INVALID);

   ret = create_pagelist((char __user *)offset, size,
         (dir == VCHIQ_BULK_RECEIVE)
         ? PAGELIST_READ
         : PAGELIST_WRITE,
         current,
         &pagelist);
   if (ret != 0)
      return VCHIQ_ERROR;

   bulk->handle = memhandle;
   bulk->data = (void *)VIRT_TO_VC(pagelist);

   /* Store the pagelist address in remote_data, which isn't used by the
      slave. */
   bulk->remote_data = pagelist;

#if defined( CONFIG_KONA_WFI_WORKAROUND )

   /*
    * SW-7022: By incrementing wfi_count here, it prevents the 2nd core from doing
    *          a WFI while we're doing a bulk transfer. If both cores do a WFI then
    * the videocore access times to ARM memory increase by a factor of 600.
    */
   atomic_inc( &wfi_count );
#endif

   return VCHIQ_SUCCESS;
}

void
vchiq_complete_bulk(VCHIQ_BULK_T *bulk)
{
#if defined( CONFIG_KONA_WFI_WORKAROUND )
   /*
    * SW-7022: We're finished with the bulk, which means that the videocore no
    *          longer needs to access the ARM memory, so we can allow the second
    * core to do WFI calls once again.
    */
   atomic_dec( &wfi_count );
#endif
   free_pagelist((PAGELIST_T *)bulk->remote_data, bulk->actual);
}

void
vchiq_transfer_bulk(VCHIQ_BULK_T *bulk)
{
   /*
    * This should only be called on the master (VideoCore) side, but
    * provide an implementation to avoid the need for ifdefery.
    */
   vcos_assert(!"This code should not be called by the ARM on BI (VC master)");
}

void
vchiq_dump_platform_state(void *dump_context)
{
   char buf[80];
   int len;
   len = vcos_snprintf(buf, sizeof(buf),
      "  Platform: BI (VC master) pagelists=%d, free_fragments=%d",
      g_pagelist_count, g_free_fragments_count);
   vchiq_dump(dump_context, buf, len + 1);
}

VCHIQ_STATUS_T
vchiq_platform_suspend(VCHIQ_STATE_T *state)
{
   VCHIQ_STATUS_T status = VCHIQ_SUCCESS;
   /* Invalidate the wake address */
   ((volatile unsigned int *)g_vchiq_slot_zero->platform_data)[0] = ~0;

   status = vchiq_pause_internal(state);

   if (status == VCHIQ_SUCCESS)
   {
      vcos_log_info("%s - waiting for g_pause_event", __func__);
      if (vcos_event_wait(&g_pause_event) != VCOS_SUCCESS)
      {
         status = VCHIQ_RETRY;
      }
      else
      {
         vcos_log_info("%s - g_pause_event received", __func__);

         do
         {
            /*msleep(1);*/
            g_wake_address = ((volatile unsigned int *)g_vchiq_slot_zero->platform_data)[0];
         } while (g_wake_address == ~0);

         chal_ipc_sleep_vc( ipcHandle );
         msleep(1);
         ((volatile unsigned int *)g_vchiq_slot_zero->platform_data)[0] = ~0;
         vcos_wmb(g_vchiq_slot_zero->platform_data);
         msleep(1);

         if (g_wake_address == 0)
         {
            vcos_log_error("VideoCore suspend failed!");
            status = VCHIQ_ERROR;
         }
         else
         {
            vcos_log_info("VideoCore suspended - wake address %x", g_wake_address);
         }
      }
   }
   return status;
}

void
vchiq_platform_paused(VCHIQ_STATE_T *state)
{
   vcos_event_signal(&g_pause_event);
}

VCHIQ_STATUS_T
vchiq_platform_resume(VCHIQ_STATE_T *state)
{
   VCHIQ_STATUS_T status = VCHIQ_SUCCESS;
   if (g_wake_address == 0)
   {
      vcos_log_info("VideoCore not suspended");
      status = VCHIQ_ERROR;
   }
   else
   {
      vcos_log_info("Resuming VideoCore at address %x", g_wake_address);

      /* Write the wake address to wake up videocore */
      chal_ipc_wakeup_vc( ipcHandle, g_wake_address );

      vcos_log_info("Waiting for response from VideoCore...");

      /* Wait for VideoCore boot */
      if (vcos_event_wait(&g_pause_event) != VCOS_SUCCESS)
      {
         status = VCHIQ_RETRY;
      }
      else
      {
         g_wake_address = 0;

         vcos_log_info("VideoCore awake");
      }
   }
   return status;
}

void
vchiq_platform_resumed(VCHIQ_STATE_T *state)
{
   vcos_event_signal(&g_pause_event);
}

int
vchiq_platform_videocore_wanted(VCHIQ_STATE_T* state)
{
   vcos_unused(state);
   int early_susp_override = 0;
#ifdef CONFIG_HAS_EARLYSUSPEND
   early_susp_override = (!g_earlysusp_suspend_allowed) && g_early_susp_ctrl;
#endif

   return (early_susp_override || !g_use_autosuspend);
}

#if VCOS_HAVE_TIMER
int
vchiq_platform_use_suspend_timer(void)
{
   return g_use_suspend_timer;
}
#endif

void
vchiq_dump_platform_use_state(VCHIQ_STATE_T *state)
{
#if defined(CONFIG_HAS_EARLYSUSPEND)
   if (g_early_susp_ctrl)
   {
      vcos_log_warn("Early suspend state: suspend allowed=%d",g_earlysusp_suspend_allowed);
   }
   else
   {
      vcos_log_info("Early suspend control disabled");
   }
#endif
#if VCOS_HAVE_TIMER
   if(vchiq_platform_use_suspend_timer())
   {
      vcos_log_info("Suspend timer in use");
   }
   else
   {
      vcos_log_info("Suspend timer not in use");
   }
#endif

}


/****************************************************************************
*
*   vchiq_control_cfg_parse
*
***************************************************************************/

static void vchiq_control_cfg_parse( VCOS_CFG_BUF_T buf, void *data )
{
   VCHIQ_KERNEL_STATE_T    *kernState = data;
   char                    *command;

   command = vcos_cfg_buf_get_str( buf );

   if ( strncmp( "connect", command, strlen( "connect" )) == 0 )
   {
      if ( vchiq_memdrv_initialise() != VCHIQ_SUCCESS )
      {
         vcos_log_error( "%s: failed to initialise vchiq for '%s'",
            __func__, kernState->instance_name );
      }
      else
      {
         vcos_log_warn( "%s: initialised vchiq for '%s'", __func__,
            kernState->instance_name );
      }
   }
   /* TODO support "disconnect" */

   /* logging level change */
   else if (strncmp ("logerror", command, strlen("logerror")) == 0 )
   {
      vcos_log_set_level(&vchiq_arm_log_category, VCOS_LOG_ERROR);
      vcos_log_error("%s: vchiq_arm log level set to VCOS_LOG_ERROR", __func__);
   }
   else if (strncmp ("logwarn", command, strlen("logwarn")) == 0 )
   {
      vcos_log_set_level(&vchiq_arm_log_category, VCOS_LOG_WARN);
      vcos_log_warn("%s: vchiq_arm log level set to VCOS_LOG_WARN", __func__);
   }
   else if (strncmp ("loginfo", command, strlen("loginfo")) == 0 )
   {
      vcos_log_set_level(&vchiq_arm_log_category, VCOS_LOG_INFO);
      vcos_log_info("%s: vchiq_arm log level set to VCOS_LOG_INFO", __func__);
   }
   else if (strncmp ("logtrace", command, strlen("logtrace")) == 0 )
   {
      vcos_log_set_level(&vchiq_arm_log_category, VCOS_LOG_TRACE);
      vcos_log_trace("%s: vchiq_arm log level set to VCOS_LOG_TRACE", __func__);
   }

   /* suspend / resume related entries */
   else if ( strncmp( "suspend", command, strlen( "suspend" )) == 0 )
   { /* direct control of suspend from vchiq_control.  Only available if not autosuspending */
      if (!g_use_autosuspend)
      {
         if ( vchiq_arm_vcsuspend(g_vchiq_state) == VCHIQ_SUCCESS )
         {
            vcos_log_warn( "%s: suspended vchiq for '%s'", __func__,
                  kernState->instance_name );
         }
         else
         {
            vcos_log_error( "%s: failed to suspend vchiq '%s'",
                  __func__, kernState->instance_name );
         }
      }
      else
      {
         vcos_log_error( "%s: can't suspend vchiq '%s' - automatic suspend/resume active",
                           __func__, kernState->instance_name );
      }
   }
   else if ( strncmp( "resume", command, strlen( "resume" )) == 0 )
   { /* direct control of resume from vchiq_control.  Only available if not autosuspending */
      if (!g_use_autosuspend)
      {
         if ( vchiq_arm_vcresume(g_vchiq_state) == VCHIQ_SUCCESS )
         {
            vcos_log_warn( "%s: resumed vchiq for '%s'", __func__,
                  kernState->instance_name );
         }
         else
         {
            vcos_log_error( "%s: failed to resume vchiq for '%s'",
                  __func__, kernState->instance_name );
         }
      }
      else
      {
         vcos_log_error( "%s: can't resume vchiq '%s' - automatic suspend/resume active",
                           __func__, kernState->instance_name );
      }
   }
   else if ( strncmp( "autosuspend", command, strlen( "autosuspend" )) == 0 )
   { /* enable autosuspend, using vchi_service_use/release usage counters to decide when to suspend */
      g_use_autosuspend = 1;
      vcos_log_info("%s: Enabling autosuspend for vchiq instance '%s'", __func__, kernState->instance_name);
      vchiq_check_suspend(g_vchiq_state);
   }
   else if ( strncmp( "noautosuspend", command, strlen( "noautosuspend" )) == 0 )
   { /* disable autosuspend - allow direct control of suspend/resume through vchiq_control */
      g_use_autosuspend = 0;
      vcos_log_info("%s: Disabling autosuspend for vchiq instance '%s'", __func__, kernState->instance_name);
      vchiq_check_resume(g_vchiq_state);
   }
   else if ( strncmp( "dumpuse", command, strlen( "dumpuse" )) == 0 )
   { /* dump usage counts for all services to determine which service(s) are preventing suspend */
      if (g_use_autosuspend)
      {
         vchiq_dump_service_use_state(g_vchiq_state);
      }
      else
      {
         vcos_log_info("Autosuspend disabled.  Nothing to dump.");
      }
   }
#if VCOS_HAVE_TIMER
   else if ( strncmp( "susptimer", command, strlen( "susptimer" )) == 0 )
   { /* enable a short timeout before suspend to allow other "use" commands in */
      if (g_use_autosuspend)
      {
         g_use_suspend_timer = 1;
         vcos_log_info("%s: Using timeout before suspend", __func__);
      }
   }
   else if ( strncmp( "nosusptimer", command, strlen( "nosusptimer" )) == 0 )
   { /* disable timeout before suspend - enter suspend directly on usage count hitting 0 (from lp task) */
      if (g_use_autosuspend)
      {
         g_use_suspend_timer = 0;
         vcos_log_info("%s: Not using timeout before suspend", __func__);
      }
   }
#endif
#if defined(CONFIG_HAS_EARLYSUSPEND)
   else if ( strncmp( "earlysuspctrl", command, strlen( "earlysuspctrl" )) == 0 )
   { /* for configs with earlysuspend, allow suspend to be blocked until the earlysuspend callback is called */
      if (g_use_autosuspend)
      {
         g_early_susp_ctrl = 1;
         vcos_log_info("%s: Using Early Suspend control for suspend/resume", __func__);
         vchiq_check_resume(g_vchiq_state);
      }
   }
   else if ( strncmp( "noearlysuspctrl", command, strlen( "noearlysuspctrl" )) == 0 )
   { /* disable control of suspend from earlysuspend callback */
      if (g_use_autosuspend)
      {
         g_early_susp_ctrl = 0;
         vcos_log_info("%s: Not using Early Suspend control for suspend/resume", __func__);
         vchiq_check_suspend(g_vchiq_state);
      }
   }
#endif
   else
   {
      vcos_log_error( "%s: unknown command '%s'", __func__, command );
   }
}

#if defined(CONFIG_HAS_EARLYSUSPEND)
static void
vchiq_early_suspend(struct early_suspend *h)
{
   if (g_early_susp_ctrl)
   {
      vcos_log_info( "%s: allowing suspend in vchiq", __func__ );
   }
   g_earlysusp_suspend_allowed = 1;
}

static void
vchiq_late_resume(struct early_suspend *h)
{
   if (g_early_susp_ctrl)
   {
      vcos_log_info( "%s: preventing suspend in vchiq", __func__ );
   }
   g_earlysusp_suspend_allowed = 0;
}
#endif


/****************************************************************************
*
* vchiq_userdrv_create_instance
*
*   The lower level drivers (vchiq_memdrv or vchiq_busdrv) will call this
*   function for each videocore that exists. We then register a character
*   driver which is what userspace uses to talk to us.
*
***************************************************************************/

VCHIQ_STATUS_T vchiq_userdrv_create_instance( const VCHIQ_PLATFORM_DATA_T *platform_data )
{
   VCHIQ_KERNEL_STATE_T   *kernState;

   vcos_log_warn( "%s: [bivcm] vchiq_num_instances = %d, VCHIQ_NUM_VIDEOCORES = %d",
      __func__, vchiq_num_instances, VCHIQ_NUM_VIDEOCORES );

   if ( vchiq_num_instances >= VCHIQ_NUM_VIDEOCORES )
   {
      vcos_log_error( "%s: already created %d instances", __func__,
         VCHIQ_NUM_VIDEOCORES );

      return VCHIQ_ERROR;
   }

   /* Allocate some memory */
   kernState = kmalloc( sizeof( *kernState ), GFP_KERNEL );
   if ( kernState == NULL )
   {
      vcos_log_error( "%s: failed to allocate memory", __func__ );

      return VCHIQ_ERROR;
   }

   memset( kernState, 0, sizeof( *kernState ));

   vchiq_kernel_state[ vchiq_num_instances ] = kernState;

   /* Do some bookkeeping */
   kernState->instNum = vchiq_num_instances++;
   kernState->instance_name = platform_data->instance_name;
   kernState->platform_data = platform_data;

   /* Create proc entries */
   if ( !vcos_cfg_is_entry_created( vc_cfg_dir ))
   {
      if ( vcos_cfg_mkdir( &vc_cfg_dir, NULL, "vc" ) != VCOS_SUCCESS )
      {
         vcos_log_error( "%s: failed to create proc entry", __func__ );

         return VCHIQ_ERROR;
      }
   }

   if ( vcos_cfg_mkdir( &kernState->instance_cfg_dir, &vc_cfg_dir,
      kernState->instance_name ) != VCOS_SUCCESS )
   {
      vcos_log_error( "%s: failed to create proc entry", __func__ );

      return VCHIQ_ERROR;
   }

   if ( vcos_cfg_create_entry( &kernState->vchiq_control_cfg_entry,
      &kernState->instance_cfg_dir,
      "vchiq_control",
      NULL,
      vchiq_control_cfg_parse,
      kernState ) != VCOS_SUCCESS )
   {
      vcos_log_error( "%s: failed to create proc entry", __func__ );

      return VCHIQ_ERROR;
   }

   printk( KERN_INFO "%s: initialised vchiq for '%s'\n", __func__,
           kernState->instance_name );

   return VCHIQ_SUCCESS;
}

EXPORT_SYMBOL( vchiq_userdrv_create_instance );

/****************************************************************************
*
* vchiq_userdrv_suspend
*
*   The lower level drivers (vchiq_memdrv or vchiq_busdrv) will call this
*   function to suspend each videocore.
*
***************************************************************************/

VCHIQ_STATUS_T vchiq_userdrv_suspend( const VCHIQ_PLATFORM_DATA_T *platform_data )
{
   VCHIQ_KERNEL_STATE_T *kernState = NULL;
   VCHIQ_STATUS_T status;
   int i;

   for (i = 0; i < vchiq_num_instances; i++)
   {
      if (vchiq_kernel_state[i]->platform_data == platform_data)
      {
         kernState = vchiq_kernel_state[i];
         break;
      }
   }

   if (!kernState)
   {
      vcos_log_error( "%s: failed to find state for instance %s", __func__,
         platform_data->instance_name );

      return VCHIQ_ERROR;
   }

   status = vchiq_arm_vcsuspend(g_vchiq_state);
   if ( status == VCHIQ_SUCCESS )
   {
      vcos_log_warn( "%s: suspended vchiq for '%s'", __func__,
         kernState->instance_name );
   }
   else
   {
      vcos_log_error( "%s: failed to suspend vchiq '%s'",
         __func__, kernState->instance_name );
   }

   return status;
}

EXPORT_SYMBOL( vchiq_userdrv_suspend );

/****************************************************************************
*
* vchiq_userdrv_resume
*
*   The lower level drivers (vchiq_memdrv or vchiq_busdrv) will call this
*   function to resume each videocore.
*
***************************************************************************/

VCHIQ_STATUS_T vchiq_userdrv_resume( const VCHIQ_PLATFORM_DATA_T *platform_data )
{
   VCHIQ_KERNEL_STATE_T *kernState = NULL;
   VCHIQ_STATUS_T status = VCHIQ_SUCCESS;
   int i;

   for (i = 0; i < vchiq_num_instances; i++)
   {
      if (vchiq_kernel_state[i]->platform_data == platform_data)
      {
         kernState = vchiq_kernel_state[i];
         break;
      }
   }

   if (!kernState)
   {
      vcos_log_error( "%s: failed to find state for instance %s", __func__,
         platform_data->instance_name );

      return VCHIQ_ERROR;
   }

   vchiq_check_resume(g_vchiq_state);

   return status;
}

EXPORT_SYMBOL( vchiq_userdrv_resume );

/*
 * Due to the limitations at the RTL level, there are some GPIO pins that
 * cannot be muxed to the videocore. As a workaround, we are using a combination
 * of the IPC doorbells and shared memory to communicate between the host and
 * videocore to control the pins that are muxed to the host.
 *
 * For the time being, we are piggybacking off of the sharedmem driver because
 * it is the fastest way. The long term solution would see the code that deals
 * with the GPIO moved to a separate service/driver.
 *
 * Operation details:
 *    - syncing:
 *         Before the videocore can request GPIO operations from the host, the
 *         videocore needs to make sure the host is up first. When the videocore
 *         boots up, it rings an IPC doorbell. If the host is already up at
 *         that moment, it will ring the same IPC doorbell back. Upon receiving
 *         the doorbell, the videocore will know the host is up and ready.
 *         Before this time, the videocore will return failure on any host GPIO
 *         requests.
 *
 *         To cover the scenario where the videocore comes up before the host,
 *         the host will automatically ring the IPC doorbell to notify the
 *         videocore.
 *
 *    - setting/clearing:
 *         1. Videocore writes GPIO pin to be set/clear into the shared memory
 *            at GPIO_MAILBOX_WRITE. If performing a set, the value is also
 *            bitwise OR'd with GPIO_MAILBOX_WRITE_SET.
 *         2. Videocore rings the IPC doorbell and waits for a reply.
 *         3. Host answers the doorbell and sets/clears the GPIO pin.
 *         4. Host rings the same IPC doorbell to notify the videocore.
 *         5. Videocore gets the doorbell and returns to the user.
 *
 *    - reading:
 *         1. Videocore writes GPIO pin to be read into the shared memory at
 *            GPIO_MAILBOX_READ.
 *         2. Videocore rings the IPC doorbell and waits for a reply.
 *         3. Host answers the doorbell and reads the value of the GPIO and
 *            stores it back into the shared memory at GPIO_MAILBOX_READ.
 *         4. Host rings the same IPC doorbell to notify the videocore.
 *         5. Videocore gets the doorbell and reads out the value of the GPIO
 *            from the shared memory and returns it to the user.
 */
static void
service_gpio( uint32_t irq_status )
{
#if defined(VCHIQ_SM_ALLOC_VCDDR)
   volatile uint32_t gpio_mailbox_write = (*(volatile uint32_t *)(g_vchiq_ipc_shared_mem + IPC_SHARED_MEM_CHANNEL_ARM_OFFSET + IPC_SHARED_MEM_GPIO_WRITE_OFFSET));
   volatile uint32_t gpio_mailbox_read  = (*(volatile uint32_t *)(g_vchiq_ipc_shared_mem + IPC_SHARED_MEM_CHANNEL_ARM_OFFSET + IPC_SHARED_MEM_GPIO_READ_OFFSET));
#else
#define GPIO_MAILBOX_WRITE            (*(volatile uint32_t *)(IPC_SHARED_MEM_BASE + IPC_SHARED_MEM_CHANNEL_ARM_OFFSET + IPC_SHARED_MEM_GPIO_WRITE_OFFSET))
#define GPIO_MAILBOX_READ             (*(volatile uint32_t *)(IPC_SHARED_MEM_BASE + IPC_SHARED_MEM_CHANNEL_ARM_OFFSET + IPC_SHARED_MEM_GPIO_READ_OFFSET))
#endif

#define GPIO_MAILBOX_WRITE_SET        (0x80000000)
#define GPIO_MAILBOX_WRITE_PIN_MASK   (0x7FFFFFFF)

   if ( irq_status & ( IPC_INTERRUPT_STATUS_ENABLED << IPC_INTERRUPT_SOURCE_2 ))
   {
#if defined(VCHIQ_SM_ALLOC_VCDDR)
      uint32_t reg = gpio_mailbox_write;
#else
      uint32_t reg = GPIO_MAILBOX_WRITE;
#endif

      if ( reg & GPIO_MAILBOX_WRITE_SET )
      {
         /* GPIO set */
         gpio_set_value( reg & GPIO_MAILBOX_WRITE_PIN_MASK , 1 );
      }
      else
      {
         /* GPIO clear */
         gpio_set_value( reg & GPIO_MAILBOX_WRITE_PIN_MASK , 0 );
      }

      /* Notify videocore that GPIO has been changed */
      chal_ipc_int_vcset( ipcHandle, IPC_INTERRUPT_SOURCE_2 );

      irq_status &= ~( IPC_INTERRUPT_STATUS_ENABLED << IPC_INTERRUPT_SOURCE_2 );
   }

   if ( irq_status & ( IPC_INTERRUPT_STATUS_ENABLED << IPC_INTERRUPT_SOURCE_3 ))
   {
#if defined(VCHIQ_SM_ALLOC_VCDDR)
      gpio_mailbox_read = gpio_get_value( gpio_mailbox_read );
#else
      GPIO_MAILBOX_READ = gpio_get_value( GPIO_MAILBOX_READ );
#endif
      /* GPIO set */

      /* Notify videocore that GPIO has been set */
      chal_ipc_int_vcset( ipcHandle, IPC_INTERRUPT_SOURCE_3 );

      irq_status &= ~( IPC_INTERRUPT_STATUS_ENABLED << IPC_INTERRUPT_SOURCE_3 );
   }

   if ( irq_status & ( IPC_INTERRUPT_STATUS_ENABLED << IPC_INTERRUPT_SOURCE_4 ))
   {
      /* Reply back to the videocore to tell them we are ready */
      chal_ipc_int_vcset( ipcHandle, IPC_INTERRUPT_SOURCE_4 );

      irq_status &= ~( IPC_INTERRUPT_STATUS_ENABLED << IPC_INTERRUPT_SOURCE_4 );
   }
}

/*
 * Local functions
 */

static irqreturn_t
vchiq_doorbell_irq(int irq, void *dev_id)
{
   IPC_INTERRUPT_SOURCE source;
   uint32_t             status;
   VCHIQ_STATE_T       *state = dev_id;

   /* get the interrupt status value */
   chal_ipc_get_int_status( ipcHandle, &status );

   /* clear all the interrupts first */
   for ( source = IPC_INTERRUPT_SOURCE_0; source < IPC_INTERRUPT_SOURCE_MAX; source++ )
   {
      if ( status & ( IPC_INTERRUPT_STATUS_ENABLED << source ))
      {
         chal_ipc_int_clr ( ipcHandle, source );
      }
   }

   if ( status & ( IPC_INTERRUPT_STATUS_ENABLED << IPC_INTERRUPT_SOURCE_0 ))
   {
      /* signal the stack that there is something to pick up */
      remote_event_pollall(state);
   }
   else if (( status & ( IPC_INTERRUPT_STATUS_ENABLED << IPC_INTERRUPT_SOURCE_2 )) ||
            ( status & ( IPC_INTERRUPT_STATUS_ENABLED << IPC_INTERRUPT_SOURCE_3 )) ||
            ( status & ( IPC_INTERRUPT_STATUS_ENABLED << IPC_INTERRUPT_SOURCE_4 )))
   {
      /* this is a GPIO request */
      service_gpio( status );
   }

   return IRQ_HANDLED;
}

/****************************************************************************
*
*   vchiq_memdrv_initialise
*
***************************************************************************/

VCHIQ_STATUS_T vchiq_memdrv_initialise(void)
{
   VCHIQ_STATE_T *state;
   VCHIQ_STATUS_T status;
   int err = 0;
   int i;

   if ( g_initialized )
   {
      vcos_log_warn( "%s: already initialized", __func__ );
      return VCOS_SUCCESS;
   }

#if defined(VCHIQ_SM_ALLOC_VCDDR)
   VC_MEM_ADDR_T vcMemAddr;
   size_t vcMemSize;
   uint8_t *mapAddr;
   off_t  vcMapAddr;

   g_vchiq_ipc_shared_mem_size = 0;
   
   if ( OpenVideoCoreMemory( &g_vchiq_mem_hndl ) == 0 )
   {
      if ( LookupVideoCoreSymbol( g_vchiq_mem_hndl,
                                  VCHIQ_IPC_SHARED_MEM_SIZE_SYMBOL,
                                  &vcMemAddr,
                                  &vcMemSize ) )
      {
         vcMapAddr = (off_t)vcMemAddr & VC_MEM_TO_ARM_ADDR_MASK;
         vcMapAddr += mm_vc_mem_phys_addr;
         mapAddr = ioremap_nocache( vcMapAddr, vcMemSize );
         if ( mapAddr != 0 )
         {
            memcpy ( &g_vchiq_ipc_shared_mem_size,
                     mapAddr,
                     vcMemSize );
            iounmap( mapAddr );
         }
         else
         {
            printk( KERN_ERR "!!! failed to ioremap \"%s\" @ 0x%x (phys: 0x%x, size: %u).\n",
                    VCHIQ_IPC_SHARED_MEM_SIZE_SYMBOL,
                    (unsigned int)vcMapAddr,
                    (unsigned int)vcMemAddr,
                    (unsigned int)vcMemSize );
         }
      }
      else
      {
         printk( KERN_ERR "!!! failed to lookup \"%s\".\n",
                 VCHIQ_IPC_SHARED_MEM_SIZE_SYMBOL );
      }

      if ( LookupVideoCoreSymbol( g_vchiq_mem_hndl,
                                  VCHIQ_IPC_SHARED_MEM_SYMBOL,
                                  &vcMemAddr,
                                  &vcMemSize ) )
      {
         vcMapAddr = (off_t)vcMemAddr & VC_MEM_TO_ARM_ADDR_MASK;
         vcMapAddr += mm_vc_mem_phys_addr;
         mapAddr = ioremap_nocache( vcMapAddr, vcMemSize );
         if ( mapAddr != 0 )
         {
            memcpy ( &g_vchiq_ipc_shared_mem_addr,
                     mapAddr,
                     vcMemSize );
            iounmap( mapAddr );
         }
         else
         {
            printk( KERN_ERR "!!! failed to ioremap \"%s\" @ 0x%x (phys: 0x%x, size: %u).\n",
                    VCHIQ_IPC_SHARED_MEM_SYMBOL,
                    (unsigned int)vcMapAddr,
                    (unsigned int)vcMemAddr,
                    (unsigned int)vcMemSize );
         }

         vcMapAddr = (off_t)g_vchiq_ipc_shared_mem_addr & VC_MEM_TO_ARM_ADDR_MASK;
         vcMapAddr = vcMapAddr + mm_vc_mem_phys_addr;
         mapAddr = ioremap_nocache( vcMapAddr, (size_t)g_vchiq_ipc_shared_mem_size );
         if ( mapAddr != 0 )
         {
            g_vchiq_ipc_shared_mem = mapAddr;
            /* Do not **iounmap** at this time, we can now use the shared memory mapped.
            */
         }
         else
         {
            printk( KERN_ERR "!!! failed to ioremap shared memory region for vchiq @ 0x%x (phys: 0x%x, size: %u).\n",
                    (unsigned int)vcMapAddr,
                    (unsigned int)g_vchiq_ipc_shared_mem_addr,
                    (unsigned int)g_vchiq_ipc_shared_mem_size );
         }
      }
      else
      {
         printk( KERN_ERR "!!! failed to lookup \"%s\".\n",
                 VCHIQ_IPC_SHARED_MEM_SIZE_SYMBOL );
      }

      CloseVideoCoreMemory( g_vchiq_mem_hndl );
   }
   else
   {
      printk( KERN_ERR "!!! failed to open videocore memory.\n" );
   }

   printk( KERN_INFO "Videocore allocated %u (0x%x) bytes of shared memory.\n",
           g_vchiq_ipc_shared_mem_size,
           g_vchiq_ipc_shared_mem_size );
   printk( KERN_INFO "Shared memory (0x%x) mapped @ 0x%p for kernel usage.\n",
           (unsigned int)g_vchiq_ipc_shared_mem_addr,
           g_vchiq_ipc_shared_mem );

   if ( (g_vchiq_ipc_shared_mem_size == 0) ||
        (g_vchiq_ipc_shared_mem == NULL) )
   {
      BUG();
      return VCHIQ_ERROR;
   }
#endif

#if defined(VCHIQ_SM_ALLOC_VCDDR)
   vcos_log_warn( "%s: ipc shared memory address                       = 0x%p", __func__, g_vchiq_ipc_shared_mem );
   vcos_log_warn( "%s: ipc shared memory size (vc+arm channels, extra) = 0x%x", __func__, g_vchiq_ipc_shared_mem_size );
#else
   vcos_log_warn( "%s: IPC_SHARED_MEM_SLOTS_VIRT = 0x%lx", __func__, IPC_SHARED_MEM_SLOTS_VIRT );
   vcos_log_warn( "%s: IPC_SHARED_MEM_SLOTS_SIZE = 0x%x", __func__, IPC_SHARED_MEM_SLOTS_SIZE );
#endif
   vcos_log_warn( "%s: VCHIQ_MAX_SERVICES        = %d", __func__, VCHIQ_MAX_SERVICES );

#if defined(VCHIQ_SM_ALLOC_VCDDR)
   g_vchiq_slot_zero = (VCHIQ_SLOT_ZERO_T *)g_vchiq_ipc_shared_mem;
#else
   g_vchiq_slot_zero = (VCHIQ_SLOT_ZERO_T *)IPC_SHARED_MEM_SLOTS_VIRT;
#endif
   state = g_vchiq_state;

   /* Initialize the local state. Note that vc04 has already started by now
      so the slot memory is expected to be initialised. */
   status = vchiq_init_state(state, g_vchiq_slot_zero, 0/* slave */);

   if (status != VCHIQ_SUCCESS)
   {
      vcos_log_error("%s: vchiq_init_state failed", __func__);
      goto failed_init_state;
   }

   g_pagelist_count = 0;
#if defined(VCHIQ_SM_ALLOC_VCDDR)
   g_fragments_base = (FRAGMENTS_T *)(g_vchiq_ipc_shared_mem +
      g_vchiq_slot_zero->platform_data[VCHIQ_PLATFORM_FRAGMENTS_OFFSET_IDX]);
#else
   g_fragments_base = (FRAGMENTS_T *)(IPC_SHARED_MEM_SLOTS_VIRT +
      g_vchiq_slot_zero->platform_data[VCHIQ_PLATFORM_FRAGMENTS_OFFSET_IDX]);
#endif
   g_free_fragments_count =
      g_vchiq_slot_zero->platform_data[VCHIQ_PLATFORM_FRAGMENTS_COUNT_IDX];

   g_free_fragments = g_fragments_base;
   for (i = 0; i < (g_free_fragments_count - 1); i++) {
      *(FRAGMENTS_T **) & g_fragments_base[i] =
         &g_fragments_base[i + 1];
   }
   *(FRAGMENTS_T **) & g_fragments_base[i] = NULL;
   sema_init(&g_free_fragments_sema, g_free_fragments_count);

   ipcHandle = chal_ipc_config( NULL );
   chal_icd_set_security (0, VCHIQ_DOORBELL_IRQ, eINT_STATE_SECURE );
   for ( i = 0; i < IPC_INTERRUPT_SOURCE_MAX; i++ )
   {
      chal_ipc_int_secmode( ipcHandle, i, IPC_INTERRUPT_MODE_OPEN );
   }

   /* clear all interrupts */
   for (i = 0; i < IPC_INTERRUPT_SOURCE_MAX; i++)
   {
      chal_ipc_int_clr(ipcHandle, i);
   }

   if (( err = request_irq( VCHIQ_DOORBELL_IRQ, vchiq_doorbell_irq,
      IRQF_DISABLED, "IPC driver", state )) != 0 )
   {
      vcos_log_error( "%s: failed to register irq=%d err=%d", __func__,
         VCHIQ_DOORBELL_IRQ, err );
      goto failed_request_irq;
   }
   else
   {
      /* Tell the videocore we are ready for servicing GPIO requests */
      chal_ipc_int_vcset( ipcHandle, IPC_INTERRUPT_SOURCE_4 );
   }

   g_initialized = 1;

   vchiq_call_connected_callbacks();

   return VCHIQ_SUCCESS;

failed_request_irq:
failed_init_state:
   return VCHIQ_ERROR;
}
EXPORT_SYMBOL( vchiq_memdrv_initialise );

/* There is a potential problem with partial cache lines (pages?)
   at the ends of the block when reading. If the CPU accessed anything in
   the same line (page?) then it may have pulled old data into the cache,
   obscuring the new data underneath. We can solve this by transferring the
   partial cache lines separately, and allowing the ARM to copy into the
   cached area.
 */

static int
create_pagelist(char __user *buf, size_t count, unsigned short type,
   struct task_struct *task, PAGELIST_T ** ppagelist)
{
   PAGELIST_T *pagelist;
   struct page **pages;
   unsigned long *addrs;
   unsigned int num_pages, offset, i;
   unsigned long addr, base_addr, next_addr;
   void *base_kaddr;
   size_t size;
   int run, addridx;
   int actual_pages;

   offset = (unsigned int)buf & (PAGE_SIZE - 1);
   num_pages = (count + offset + PAGE_SIZE - 1) / PAGE_SIZE;

   *ppagelist = NULL;

   /* Allocate enough storage to hold the page pointers and the page list */
   pagelist = (PAGELIST_T *) kmalloc(sizeof(PAGELIST_T) +
      (num_pages * sizeof(unsigned long)) +
      (num_pages * sizeof(pages[0])),
      GFP_KERNEL);

   vcos_log_trace("create_pagelist %x@%x - %x", (unsigned int)count, (unsigned int)buf, (unsigned int)pagelist);
   if (!pagelist)
      return -ENOMEM;

   addrs = pagelist->addrs;
   pages = (struct page **)(addrs + num_pages);

   down_read(&task->mm->mmap_sem);
   actual_pages = get_user_pages(task, task->mm,
             (unsigned long)buf & ~(PAGE_SIZE - 1), num_pages,
             (type == PAGELIST_READ) /*Write */ , 0 /*Force */ ,
             pages, NULL /*vmas */ );
   up_read(&task->mm->mmap_sem);

   if (actual_pages != num_pages)
   {
      /* This is probably due to the process being killed */
      while (actual_pages > 0)
      {
         actual_pages--;
         page_cache_release(pages[actual_pages]);
      }
      kfree(pagelist);
      if (actual_pages == 0)
         actual_pages = -ENOMEM;
      return actual_pages;
   }

   pagelist->length = count;
   pagelist->type = type;
   pagelist->offset = offset;

   /* Group the pages into runs of contiguous pages */

   base_addr = PFN_PHYS(page_to_pfn(pages[0]));
   next_addr = base_addr + PAGE_SIZE;
   base_addr += offset;
   base_kaddr = (char *)page_address(pages[0]) + offset;
   size = vcos_min(PAGE_SIZE - offset, count);
   addridx = 0;
   run = 0;

   for (i = 1; i < num_pages; i++) {
      addr = PFN_PHYS(page_to_pfn(pages[i]));
      if ((addr == next_addr) && (run < (PAGE_SIZE - 1))) {
         next_addr += PAGE_SIZE;
         size = vcos_min(size + PAGE_SIZE, count);
         run++;
      } else {
         addrs[addridx] = PHYS_TO_VC((base_addr & ~(PAGE_SIZE - 1)) + run);
         addridx++;
         if (type == PAGELIST_READ)
         {
            dmac_map_area(base_kaddr, size, DMA_FROM_DEVICE);
            outer_inv_range(base_addr, base_addr + size);
         }
         else
         {
            dmac_map_area(base_kaddr, size, DMA_TO_DEVICE);
            outer_clean_range(base_addr, base_addr + size);
         }

         base_addr = addr;
         base_kaddr = page_address(pages[i]);
         next_addr = addr + PAGE_SIZE;
         count -= size;
         size = vcos_min(PAGE_SIZE, count);
         offset = 0;
         run = 0;
      }
   }

   addrs[addridx] = PHYS_TO_VC((base_addr & ~(PAGE_SIZE - 1)) + run);
   addridx++;

   if (type == PAGELIST_READ)
   {
      dmac_map_area(base_kaddr, size, DMA_FROM_DEVICE);
      outer_inv_range(base_addr, base_addr + size);
   }
   else
   {
      dmac_map_area(base_kaddr, size, DMA_TO_DEVICE);
      outer_clean_range(base_addr, base_addr + size);
   }

   /* Partial cache lines (fragments) require special measures */
   if ((type == PAGELIST_READ) &&
      ((pagelist->offset & (CACHE_LINE_SIZE - 1)) ||
      ((pagelist->offset + pagelist->length) & (CACHE_LINE_SIZE - 1)))) {
      FRAGMENTS_T *fragments;

      if (down_interruptible(&g_free_fragments_sema) != 0) {
         kfree(pagelist);
         return -EINTR;
      }

      vcos_assert(g_free_fragments != NULL);

      down(&g_free_fragments_mutex);
      fragments = (FRAGMENTS_T *) g_free_fragments;
      vcos_assert(fragments != NULL);
      g_free_fragments = *(FRAGMENTS_T **) g_free_fragments;
      g_free_fragments_count--;
      up(&g_free_fragments_mutex);
      pagelist->type =
         PAGELIST_READ_WITH_FRAGMENTS + (fragments - g_fragments_base);
   }

   dmac_map_area(pagelist, (int)(addrs + addridx) - (int)pagelist, DMA_TO_DEVICE);
   outer_clean_range(__virt_to_phys((unsigned long)pagelist), __virt_to_phys((unsigned long)(addrs + addridx)));

   *ppagelist = pagelist;

   down(&g_free_fragments_mutex);
   g_pagelist_count++;
   up(&g_free_fragments_mutex);

   return 0;
}

static void
free_pagelist(PAGELIST_T *pagelist, int actual)
{
   struct page **pages;
   unsigned int num_pages, i;
   unsigned long *addrs;
   int len, offset, page_count;

   vcos_log_trace("free_pagelist - %x, %d", (unsigned int)pagelist, actual);

   num_pages =
       (pagelist->length + pagelist->offset + PAGE_SIZE - 1) / PAGE_SIZE;

   pages = (struct page **)(pagelist->addrs + num_pages);

   len = pagelist->length;
   offset = pagelist->offset;

   /* Deal with any partial cache lines (fragments) */
   if (pagelist->type >= PAGELIST_READ_WITH_FRAGMENTS) {
      FRAGMENTS_T *fragments =
          g_fragments_base + (pagelist->type -
               PAGELIST_READ_WITH_FRAGMENTS);
      int head_bytes, tail_bytes;

      if (actual >= 0)
      {
         if ((head_bytes = (CACHE_LINE_SIZE - pagelist->offset) & (CACHE_LINE_SIZE - 1)) != 0) {
            if (head_bytes > actual)
               head_bytes = actual;

            memcpy((char *)page_address(pages[0]) +
                   pagelist->offset, fragments->headbuf,
                   head_bytes);
            offset += head_bytes;
            len -= head_bytes;
         }
         if ((head_bytes < actual) &&
            (tail_bytes =
            (pagelist->offset + actual) & (CACHE_LINE_SIZE -
                              1)) != 0) {
            memcpy((char *)page_address(pages[num_pages - 1]) +
                   ((pagelist->offset + actual) & (PAGE_SIZE -
                           1) & ~(CACHE_LINE_SIZE - 1)),
                   fragments->tailbuf, tail_bytes);
            len -= tail_bytes;
         }
      }

      down(&g_free_fragments_mutex);
      *(FRAGMENTS_T **) fragments = g_free_fragments;
      g_free_fragments = fragments;
      g_free_fragments_count++;
      up(&g_free_fragments_mutex);
      up(&g_free_fragments_sema);
   }

   page_count = 0;
   addrs = pagelist->addrs;

   for (i = 0; i < num_pages; i++) {
      if (pagelist->type != PAGELIST_WRITE)
      {
         if (!page_count)
         {
            int block_bytes;
            page_count = (*(addrs++) & (PAGE_SIZE - 1)) + 1;
            block_bytes = PAGE_SIZE * page_count - offset;
            if (block_bytes > len)
               block_bytes = len;
            if (block_bytes)
            {
               void *base_kaddr = page_address(pages[i]) + offset;
               unsigned long base_addr = PFN_PHYS(page_to_pfn(pages[i])) + offset;
               dmac_unmap_area(base_kaddr, block_bytes, DMA_FROM_DEVICE);
               outer_inv_range(base_addr, base_addr + block_bytes);
            }
            offset = 0;
            len -= block_bytes;
         }
         page_count--;
         set_page_dirty(pages[i]);
      }
      page_cache_release(pages[i]);
   }

   kfree(pagelist);

   down(&g_free_fragments_mutex);
   g_pagelist_count--;
   up(&g_free_fragments_mutex);
}
