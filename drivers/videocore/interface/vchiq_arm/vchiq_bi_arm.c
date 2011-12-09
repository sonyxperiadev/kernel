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

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/module.h>
#include <asm/io.h>
#include <asm/uaccess.h>

#include <vchiq_platform_data.h>

#include "vchiq_arm.h"
#include "vchiq_bi.h"
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


#define VCOS_LOG_CATEGORY (&vchiq_arm_log_category)

#define IPC_SHARED_MEM_SLOTS_VIRT IPC_SHARED_MEM_SLOTS

#if ( BCMHANA_ARAM_VC_OFFSET != IPC_SHARED_MEM_VC_OFFSET )
#error BAMHANA ARAM VC OFFSET MUST be the same as IPC SHARED MEM VC OFFSET
#endif

/*
 * On Big-Island there are restrictions about what we can DMA. Specifically, we
 * require that the data be 8-byte aligned, and be a multiple of 4 in length.
 *
 * If we're asked to deal with data which doesn't fit this criteria, then we
 * allocate a temporary buffer and copy the data into the temporary buffer.
 *
 * Normally, the memhandle is used to store the thread id of the process issuing
 * the request. We use the special value MEM_HANDLE_ALLOCATED_TEMP_BUF to indicate
 * that we allocated a temporary buffer which we need to free later.
 */

#define MEM_HANDLE_ALLOCATED_TEMP_BUF    1

typedef struct
{
    unsigned int                 instNum;

    const char                  *instance_name;
    const VCHIQ_PLATFORM_DATA_T *platform_data;

    VCOS_CFG_ENTRY_T             instance_cfg_dir;
    VCOS_CFG_ENTRY_T             vchiq_control_cfg_entry;
} VCHIQ_KERNEL_STATE_T;

typedef struct vchiq_bi_state_struct {
   int inited;
   VCHIQ_ARM_STATE_T arm_state;
} VCHIQ_BI_ARM_STATE_T;

#define VCHIQ_NUM_VIDEOCORES 1



static VCHIQ_KERNEL_STATE_T    *vchiq_kernel_state[ VCHIQ_NUM_VIDEOCORES ];
static unsigned int             vchiq_num_instances = 0;

/* Global 'vc' proc entry used by all instances */
VCOS_CFG_ENTRY_T         vc_cfg_dir;

static CHAL_IPC_HANDLE   ipcHandle;

static DMA_MMAP_CFG_T    gVchiqDmaMmap;
static SDMA_Handle_t     dmaHndl;
static struct completion gDmaDone;
static VCOS_MUTEX_T      g_dma_mutex;
static VCOS_EVENT_T      g_pause_event;
static int               g_initialized;

static VCHIQ_STATE_T    *g_vchiq_state;
static VCHIQ_SLOT_ZERO_T *g_vchiq_slot_zero;
static unsigned int      g_wake_address;

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

static int
ipc_dma( void *vcaddr, void *armaddr, int len, DMA_MMAP_PAGELIST_T *pagelist,
   enum dma_data_direction dir );

static irqreturn_t
vchiq_doorbell_irq(int irq, void *dev_id);




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

   vcos_mutex_create(&g_dma_mutex, "dma_mutex");
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
   free_irq(BCM_INT_ID_IPC_OPEN, state);
   vcos_mutex_delete(&g_dma_mutex);
}

VCHIQ_STATUS_T
vchiq_platform_init_state(VCHIQ_STATE_T *state)
{
   VCHIQ_STATUS_T status = VCHIQ_SUCCESS;
   state->platform_state = vcos_calloc(1, sizeof(VCHIQ_BI_ARM_STATE_T), "VCHIQ_BI_ARM_STATE");
   ((VCHIQ_BI_ARM_STATE_T*)state->platform_state)->inited = 1;
   status = vchiq_arm_init_state(state, &((VCHIQ_BI_ARM_STATE_T*)state->platform_state)->arm_state);
   if(status != VCHIQ_SUCCESS)
   {
      ((VCHIQ_BI_ARM_STATE_T*)state->platform_state)->inited = 0;
   }
   return status;
}

VCHIQ_ARM_STATE_T*
vchiq_platform_get_arm_state(VCHIQ_STATE_T *state)
{
   if(!((VCHIQ_BI_ARM_STATE_T*)state->platform_state)->inited)
   {
      BUG();
   }
   return &((VCHIQ_BI_ARM_STATE_T*)state->platform_state)->arm_state;
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
vchiq_prepare_bulk_data(VCHIQ_BULK_T *bulk,
   VCHI_MEM_HANDLE_T memhandle, void *offset, int size, int dir)
{
   vcos_assert(memhandle == VCHI_MEM_HANDLE_INVALID);

   /* Check the memory is supported by dma_mmap */
   if (!dma_mmap_dma_is_supported(offset))
   {
      vcos_log_error( "%s: buffer at %lx not supported", __func__,
         (unsigned long)offset );
      return VCHIQ_ERROR;
   }

   if (dma_mmap_mem_type(offset) == DMA_MMAP_TYPE_USER)
   {
      DMA_MMAP_PAGELIST_T *pagelist;
      int ret;
      ret = dma_mmap_create_pagelist(
         (char __user *)offset,
         size,
         (dir == VCHIQ_BULK_RECEIVE)
         ? DMA_FROM_DEVICE
         : DMA_TO_DEVICE,
         current,
         &pagelist);
      if (ret <= 0)
         return VCHIQ_ERROR;
      bulk->data = offset;
      bulk->handle = (VCHI_MEM_HANDLE_T)pagelist;
   }
   else
   {
      bulk->data = offset;
      bulk->handle = 0;
   }

   return VCHIQ_SUCCESS;
}

void
vchiq_complete_bulk(VCHIQ_BULK_T *bulk)
{
   if (bulk->handle != 0)
      dma_mmap_free_pagelist((DMA_MMAP_PAGELIST_T *)bulk->handle);
}

void
vchiq_transfer_bulk(VCHIQ_BULK_T *bulk)
{
   if ((bulk->size == bulk->remote_size) &&
      (ipc_dma( bulk->remote_data, bulk->data, bulk->size, (DMA_MMAP_PAGELIST_T *)bulk->handle,
      (bulk->dir == VCHIQ_BULK_TRANSMIT) ? DMA_TO_DEVICE : DMA_FROM_DEVICE) == 0))
   {
      bulk->actual = bulk->size;
   }
   else
   {
      bulk->actual = VCHIQ_BULK_ACTUAL_ABORTED;
   }
}

void
vchiq_dump_platform_state(void *dump_context)
{
   char buf[80];
   int len;
   len = vcos_snprintf(buf, sizeof(buf),
      "  Platform: BI (ARM master)");
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
      vcos_log_error("VideoCore not suspended");
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

   vcos_log_warn( "%s: [bi] vchiq_num_instances = %d, VCHIQ_NUM_VIDEOCORES = %d",
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

/****************************************************************************
*
*   Function which translates a dma direction into a printable string.
*
***************************************************************************/

static inline const char *dma_data_direction_as_str( enum dma_data_direction dir )
{
    switch ( dir )
    {
        case DMA_BIDIRECTIONAL:     return "BIDIRECTIONAL";
        case DMA_TO_DEVICE:         return "TO_DEVICE";
        case DMA_FROM_DEVICE:       return "FROM_DEVICE";
        case DMA_NONE:              return "NONE";
    }
    return "???";
}

/****************************************************************************
*
*   dma_device_handler
*
***************************************************************************/

static void dma_device_handler( DMA_Device_t dev, int reason, void *userData )
{
    vcos_unused(dev);
    vcos_unused(userData);

    struct completion *dmaDone = userData;

    if ( reason & DMA_HANDLER_REASON_TRANSFER_COMPLETE )
    {
        complete( dmaDone );
    }

    if ( reason != DMA_HANDLER_REASON_TRANSFER_COMPLETE )
    {
        vcos_log_error( "%s: called with reason = 0x%x", __func__, reason );
    }
}


/****************************************************************************
*
*   ipc_dma
*
***************************************************************************/

static int
ipc_dma( void *vcaddr, void *armaddr, int len, DMA_MMAP_PAGELIST_T *pagelist, enum dma_data_direction dir )
{
   int rc;
   dma_addr_t vcAddrOffset;
   dma_addr_t vcPhysAddr;
   DMA_Device_t dmaDev;

   vcos_mutex_lock(&g_dma_mutex);

   vcos_log_trace( "(Bulk) dir=%s vcaddr=0x%x armaddr=0x%x len=%u",
         (dir == DMA_TO_DEVICE) ? "Tx" : "Rx", (unsigned int)vcaddr,
         (unsigned int)armaddr, len );

   /* Convert the videocore pointer to a videocore address offset */
   vcAddrOffset = (dma_addr_t)(((unsigned long)vcaddr) & 0x3FFFFFFFuL);

   /* Convert the videocore physical address into an ARM physical address */
   vcPhysAddr = mm_vc_mem_phys_addr + vcAddrOffset;

   if ((( (unsigned long)vcaddr  & 7uL ) == 0 )
   &&  (( (unsigned long)armaddr & 7uL ) == 0 )
   &&  (( len & 3 ) == 0 ))
   {
       init_completion( &gDmaDone );

       dmaDev = DMA_DEVICE_MEM_TO_MEM;
       if (( len & 7 ) != 0 )
       {
           /* If the length isn't a multiple of 8, then we need to use 32-bit transactions */
           dmaDev = DMA_DEVICE_MEM_TO_MEM_32;
       }
   }
   else
   {
      static int warned_vcaddr = 0;
      static int warned_armaddr = 0;
      static int warned_len = 0;

      if (!warned_vcaddr && (( (unsigned long)vcaddr & 7uL ) != 0 ))
      {
         vcos_log_warn( "%s: vcaddr 0x%p isn't a multiple of 8", __func__, (warned_vcaddr = 1, vcaddr) );
      }
      if (!warned_armaddr && (( (unsigned long)armaddr & 7uL ) != 0 ))
      {
         vcos_log_warn( "%s: armaddr 0x%p isn't a multiple of 8", __func__, (warned_armaddr = 1, armaddr) );
      }
      if (!warned_len && (( len & 3 ) != 0 ))
      {
         vcos_log_warn( "%s: len %d isn't a multiple of 4", __func__, (warned_len = 1, len) );
      }

      dmaDev = DMA_DEVICE_NONE;
   }

   if ( dmaDev == DMA_DEVICE_NONE )
   {
      dmaHndl = SDMA_INVALID_HANDLE;
   }
   else
   {
      dmaHndl = sdma_request_channel( dmaDev );
      if( dmaHndl < 0 )
      {
         vcos_log_error( "%s: sdma_request_channel failed", __func__ );
         rc = -1;
         goto failed_sdma_request_channel;
      }

      rc = sdma_set_device_handler( dmaDev, dma_device_handler, &gDmaDone );
      if( rc < 0 )
      {
         vcos_log_error( "%s: sdma_set_device_handler failed", __func__ );
         goto failed_sdma_set_device_handler;
      }

      INIT_COMPLETION( gDmaDone );   /* Mark as incomplete */
   }

   /* Double check the memory is supported by dma_mmap */
   rc = dma_mmap_dma_is_supported( armaddr );
   if( !rc )
   {
      vcos_log_error( "%s: Buffer not supported buf=0x%lx", __func__, (unsigned long)armaddr );
      goto failed_dma_mmap_dma_is_supported;
   }

   /* Set the pagelist (for user buffers) */
   if (pagelist)
      dma_mmap_set_pagelist( &gVchiqDmaMmap, pagelist);

   /* Map memory */
   rc = dma_mmap_map( &gVchiqDmaMmap, armaddr, len, dir );
   if( rc < 0 )
   {
      vcos_log_error( "%s: dma_mmap_map FAILED buf=0x%lx len=0x%lx",
            __func__, (unsigned long)armaddr, (unsigned long)len);
      goto failed_dma_mmap_map;
   }

   if ( dmaDev == DMA_DEVICE_NONE )
   {
      struct resource *res;
      void *vcVirtAddr;

      /* Request an I/O memory region for remapping */
      res = request_mem_region( vcPhysAddr, len, "vchiq" );
      if ( res == NULL )
      {
         vcos_log_error( "%s: failed to request I/O memory region", __func__ );
         goto failed_request_mem_region;
      }

      /* I/O remap the videocore memory */
      vcVirtAddr = ioremap_nocache( res->start, resource_size( res ));
      if ( vcVirtAddr == NULL )
      {
         vcos_log_error( "%s: failed to I/O remap videocore bulk buffer",
                         __func__ );
         release_mem_region( res->start, resource_size( res ));
         goto failed_ioremap;
      }

      dma_mmap_memcpy( &gVchiqDmaMmap, vcVirtAddr );

      /* Unmap the videocore memory */
      iounmap( vcVirtAddr );

      /* Release the I/O memory region */
      release_mem_region( res->start, resource_size( res ));
   }
   else
   {
      rc = sdma_map_create_descriptor_ring( dmaHndl, &gVchiqDmaMmap,
         vcPhysAddr, DMA_UPDATE_MODE_INC );
      if( rc < 0 )
      {
         vcos_log_error( "%s: sdma_map_create_descriptor_ring FAILED rc=%u",
            __func__, rc );
         vcos_log_error( "%s: vcaddr=0x%p armaddr=0x%p len=%d dir=%s",
            __func__, vcaddr, armaddr, len,
            dma_data_direction_as_str( dir ));
         goto failed_sdma_map_create_descriptor_ring;
      }
      if ((rc = sdma_start_transfer( dmaHndl )) != 0 )
      {
         vcos_log_error( "%s: DMA failed %d", __func__, rc );
         goto failed_sdma_start_transfer;
      }

      wait_for_completion( &gDmaDone );
   }

   rc = 0;

failed_sdma_start_transfer:
failed_sdma_map_create_descriptor_ring:
failed_ioremap:
failed_request_mem_region:
   dma_mmap_unmap( &gVchiqDmaMmap, (dir == DMA_FROM_DEVICE) ? DMA_MMAP_DIRTIED : DMA_MMAP_CLEAN );
failed_dma_mmap_map:
failed_dma_mmap_dma_is_supported:
failed_sdma_set_device_handler:
   if ( dmaHndl != SDMA_INVALID_HANDLE )
   {
      sdma_free_channel( dmaHndl );
      dmaHndl = SDMA_INVALID_HANDLE;
   }
failed_sdma_request_channel:
   vcos_mutex_unlock(&g_dma_mutex);

   return rc;
}

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
   status = vchiq_init_state(state, g_vchiq_slot_zero, 1/*master*/);

   if (status != VCHIQ_SUCCESS)
   {
      vcos_log_error("%s: vchiq_init_state failed", __func__);
      goto failed_init_state;
   }

   ipcHandle = chal_ipc_config( NULL );
   chal_icd_set_security (0, BCM_INT_ID_IPC_OPEN, eINT_STATE_SECURE );
   for ( i = 0; i < IPC_INTERRUPT_SOURCE_MAX; i++ )
   {
      chal_ipc_int_secmode( ipcHandle, i, IPC_INTERRUPT_MODE_OPEN );
   }

   /* clear all interrupts */
   for (i = 0; i < IPC_INTERRUPT_SOURCE_MAX; i++)
   {
      chal_ipc_int_clr(ipcHandle, i);
   }

   if (( err = request_irq( BCM_INT_ID_IPC_OPEN, vchiq_doorbell_irq,
      IRQF_DISABLED, "IPC driver", state )) != 0 )
   {
      vcos_log_error( "%s: failed to register irq=%d err=%d", __func__,
         BCM_INT_ID_IPC_OPEN, err );
      goto failed_request_irq;
   }
   else
   {
      /* Tell the videocore we are ready for servicing GPIO requests */
      chal_ipc_int_vcset( ipcHandle, IPC_INTERRUPT_SOURCE_4 );
   }

   /* initialize dma_mmap for use */
   dma_mmap_init_map( &gVchiqDmaMmap );

   g_initialized = 1;

   vchiq_call_connected_callbacks();

   return VCHIQ_SUCCESS;

failed_request_irq:
failed_init_state:
   return VCHIQ_ERROR;
}
EXPORT_SYMBOL( vchiq_memdrv_initialise );
