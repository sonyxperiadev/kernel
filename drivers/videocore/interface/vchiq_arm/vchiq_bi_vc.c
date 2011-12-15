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

#include "vchiq_vc.h"
#include "vchiq_bi.h"
#include "vcfw/vclib/vclib.h"
#include "vcfw/drivers/chip/ipc.h"
#include "vcfw/rtos/common/rtos_common_mem.h"
#include "helpers/vcsuspend/vcsuspend.h"
#include "helpers/dmalib/dmalib.h"

static IPC_DRIVER_T *ipc_driver;
static DRIVER_HANDLE_T ipc_handle;
static VCHIQ_SLOT_ZERO_T *g_vchiq_slot_zero;

#if defined(VCHIQ_SM_ALLOC_VCDDR)
#include "host_support/include/vc_debug_sym.h"
VC_DEBUG_DECLARE_UNCACHED_STATIC_VAR( int, vchiq_ipc_shared_mem, 0 );
#define vchiq_ipc_shared_mem VC_DEBUG_ACCESS_UNCACHED_VAR( vchiq_ipc_shared_mem )
VC_DEBUG_DECLARE_UNCACHED_STATIC_VAR( int, vchiq_ipc_shared_mem_size, 0 );
#define vchiq_ipc_shared_mem_size VC_DEBUG_ACCESS_UNCACHED_VAR( vchiq_ipc_shared_mem_size )
#endif

VCHIQ_INSTANCE_STRUCT_T vchiq_instances[1];

typedef struct vchiq_bi_state_struct {
   int inited;
   VCHIQ_VC_STATE_T vc_state;
} VCHIQ_BI_VC_STATE_T;


VCHIQ_STATUS_T
vchiq_platform_init(void)
{
   const IPC_DRIVER_T *ipc = NULL;
   IPC_OPEN_T oparams = { NULL };
   VCHIQ_STATUS_T status = VCHIQ_SUCCESS;

   if (vchiq_num_instances == 0)
   {
#if defined(VCHIQ_SM_ALLOC_VCDDR)
      int shared_memory_allocation;

      vchiq_ipc_shared_mem_size = 0;
      shared_memory_allocation = (int) rtos_malloc_priority( VCHIQ_IPC_SHARED_MEM_SIZE + VCHIQ_IPC_SHARED_MEM_EXTRA,
                                                             RTOS_ALIGN_4KBYTE,
                                                             RTOS_PRIORITY_DIRECT,
                                                             "vchiq-ipc-smem" );
      if ( shared_memory_allocation )
      {
         vchiq_ipc_shared_mem = shared_memory_allocation; 
         vchiq_ipc_shared_mem_size = VCHIQ_IPC_SHARED_MEM_SIZE + VCHIQ_IPC_SHARED_MEM_EXTRA;

         /* The videocore will initialize before the host, so memset the shared memory here. */
         memset( (void *)vchiq_ipc_shared_mem, 0, vchiq_ipc_shared_mem_size );

         /* And initialise the slots. */
         g_vchiq_slot_zero = vchiq_init_slots((void *)vchiq_ipc_shared_mem, vchiq_ipc_shared_mem_size);
      }
#else
      memset( (void *)IPC_SHARED_MEM_SLOTS, 0, IPC_SHARED_MEM_SLOTS_SIZE );

      /* And initialise the slots. */
      g_vchiq_slot_zero = vchiq_init_slots((void *)IPC_SHARED_MEM_SLOTS, IPC_SHARED_MEM_SLOTS_SIZE);
#endif

      if (g_vchiq_slot_zero)
      {
         status = vchiq_init_state(&vchiq_instances[0].state, g_vchiq_slot_zero, 0/* slave */);
         if (status == VCHIQ_SUCCESS)
         {
            vchiq_num_instances = 1;

            ipc = ipc_get_func_table();
   
            ipc_driver = (IPC_DRIVER_T *)ipc;
            ipc_driver->open( &oparams, &ipc_handle );
            ipc_driver->set_doorbell_isr(ipc_handle, 0,
               (IPC_DOORBELL_ISR)remote_event_pollall,
               &vchiq_instances[0].state);
         }
      }
      else
      {
         status = VCHIQ_ERROR;
      }
   }

   return status;
}

VCHIQ_STATUS_T
vchiq_platform_init_state(VCHIQ_STATE_T *state)
{
   VCHIQ_STATUS_T status = VCHIQ_SUCCESS;
   state->platform_state = vcos_calloc(1, sizeof(VCHIQ_BI_VC_STATE_T), "VCHIQ_BI_VC_STATE");
   status = vchiq_vc_init_state(&((VCHIQ_BI_VC_STATE_T*)state->platform_state)->vc_state);
   if(status == VCHIQ_SUCCESS)
   {
      ((VCHIQ_BI_VC_STATE_T*)state->platform_state)->inited = 1;
   }
   return status;
}

VCHIQ_VC_STATE_T*
vchiq_platform_get_vc_state(VCHIQ_STATE_T *state)
{
   if(!((VCHIQ_BI_VC_STATE_T*)state->platform_state)->inited)
   {
      vcos_demand(0);
   }
   return &((VCHIQ_BI_VC_STATE_T*)state->platform_state)->vc_state;
}


VCHIQ_STATUS_T
vchiq_prepare_bulk_data(VCHIQ_BULK_T *bulk,
   VCHI_MEM_HANDLE_T memhandle, void *offset, int size, int dir)
{
   char *data;
   if (memhandle != VCHI_MEM_HANDLE_INVALID)
   {
      data = (char *)mem_lock(memhandle);
      if (data)
         data += (unsigned int)offset;
   }
   else
      data = offset;

   if (!VCHIQ_IS_SAFE_DATA(data))
      return VCHIQ_ERROR;

   if (!RTOS_IS_ALIAS_DIRECT(data))
   {
      if (dir == VCHIQ_BULK_RECEIVE)
         vclib_dcache_invalidate_range(data, size);
      else
         vclib_dcache_flush_range(data, size);
   }

   bulk->data = data;
   bulk->handle = memhandle;

   return data ? VCHIQ_SUCCESS : VCHIQ_ERROR;
}

void
vchiq_complete_bulk(VCHIQ_BULK_T *bulk)
{
   if (bulk->handle != VCHI_MEM_HANDLE_INVALID)
      mem_unlock(bulk->handle);
}

void
vchiq_transfer_bulk(VCHIQ_BULK_T *bulk)
{
	/*
	 * This should only be called on the master (ARM) side, but
	 * provide an implementation to avoid the need for ifdefery.
	 */
	vcos_assert(!"This code should not be called by the VC on BigIsland");
}

VCHIQ_STATUS_T
vchiq_copy_from_user(void *dst, const void *src, int size)
{
   VC_MEMCPY(dst, src, size);
   return VCHIQ_SUCCESS;
}

void
remote_event_signal(REMOTE_EVENT_T *event)
{
   event->fired = 1;

   /* The test on the next line also ensures the write on the previous line
      has completed */

   if (event->armed) {
      /* Notify the other side */
      ipc_driver->doorbell_ring(ipc_handle, 0, 1);
   }
}

static void
vchiq_set_resume_address(uint32_t resume_address)
{
   g_vchiq_slot_zero->platform_data[0] = resume_address;

   while (((volatile uint32_t *)g_vchiq_slot_zero->platform_data)[0] == resume_address)
      continue;
}

void
vchiq_platform_paused(VCHIQ_STATE_T *state)
{
   vcsuspend_register_comms_callback(&vchiq_set_resume_address);
   vcsuspend_suspend();
   vchiq_resume_internal(state);
}

void
vchiq_platform_resumed(VCHIQ_STATE_T *state)
{
   vcos_unused(state);
}

VCHIQ_STATUS_T
vchiq_use_service_internal(VCHIQ_SERVICE_T *service)
{
   vcos_unused(service);
   return VCHIQ_SUCCESS;
}

VCHIQ_STATUS_T
vchiq_release_service_internal(VCHIQ_SERVICE_T *service)
{
   vcos_unused(service);
   return VCHIQ_SUCCESS;
}
