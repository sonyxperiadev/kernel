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
#include "vchiq_bivcm.h"
#include "vchiq_vc_dma.h"
#include "vcfw/vclib/vclib.h"
#include "vcfw/drivers/chip/ipc.h"
#include "vcfw/rtos/common/rtos_common_mem.h"
#include "helpers/vcsuspend/vcsuspend.h"
#include "helpers/dmalib/dmalib.h"

#define MAX_FRAGMENTS 64

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

typedef struct vchiq_bivcm_state_struct {
   int inited;
   VCHIQ_VC_STATE_T vc_state;
} VCHIQ_BIVCM_VC_STATE_T;


VCHIQ_STATUS_T
vchiq_platform_init(void)
{
   const IPC_DRIVER_T *ipc = NULL;
   IPC_OPEN_T oparams = { NULL };
   VCHIQ_STATUS_T status = VCHIQ_SUCCESS;

   if (vchiq_num_instances == 0)
   {
      int slot_mem_size = 0;

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

         /* Reserve some space for the fragments */
         slot_mem_size = VCHIQ_IPC_SHARED_MEM_SIZE & ~(CACHE_LINE_SIZE - 1);
         slot_mem_size -= MAX_FRAGMENTS * sizeof(FRAGMENTS_T);

         status = init_dma_xfers(MAX_FRAGMENTS, (FRAGMENTS_T *)(shared_memory_allocation + slot_mem_size));
      }
#else
      /* Reserve some space for the fragments */
      slot_mem_size = IPC_SHARED_MEM_SLOTS_SIZE & ~(CACHE_LINE_SIZE - 1);
      slot_mem_size -= MAX_FRAGMENTS * sizeof(FRAGMENTS_T);

      status = init_dma_xfers(MAX_FRAGMENTS, (FRAGMENTS_T *)(IPC_SHARED_MEM_SLOTS + slot_mem_size));
#endif


      if (status == VCHIQ_SUCCESS)
      {
         /* And initialise the slots. */
#if defined(VCHIQ_SM_ALLOC_VCDDR)
         g_vchiq_slot_zero = vchiq_init_slots((void *)vchiq_ipc_shared_mem, slot_mem_size );
#else
         g_vchiq_slot_zero = vchiq_init_slots((void *)IPC_SHARED_MEM_SLOTS, slot_mem_size );
#endif
         if (!g_vchiq_slot_zero)
            status = VCHIQ_ERROR;
      }
      if (status == VCHIQ_SUCCESS)
      {
         status = vchiq_init_state(&vchiq_instances[0].state, g_vchiq_slot_zero, 1/* master */);
         if (status == VCHIQ_SUCCESS)
         {
            ipc = ipc_get_func_table();

            g_vchiq_slot_zero->platform_data[VCHIQ_PLATFORM_FRAGMENTS_OFFSET_IDX] = slot_mem_size;
            g_vchiq_slot_zero->platform_data[VCHIQ_PLATFORM_FRAGMENTS_COUNT_IDX]  = MAX_FRAGMENTS;

            ipc_driver = (IPC_DRIVER_T *)ipc;
            ipc_driver->open( &oparams, &ipc_handle );
            ipc_driver->set_doorbell_isr(ipc_handle, 0,
               (IPC_DOORBELL_ISR)remote_event_pollall,
               &vchiq_instances[0].state);

            vchiq_num_instances = 1;
         }
      }
   }

   if (status != VCHIQ_SUCCESS)
      uninit_dma_xfers();

   return status;
}

VCHIQ_STATUS_T
vchiq_platform_init_state(VCHIQ_STATE_T *state)
{
   VCHIQ_STATUS_T status = VCHIQ_SUCCESS;
   state->platform_state = vcos_calloc(1, sizeof(VCHIQ_BIVCM_VC_STATE_T), "VCHIQ_BIVCM_VC_STATE");
   status = vchiq_vc_init_state(&((VCHIQ_BIVCM_VC_STATE_T*)state->platform_state)->vc_state);
   if(status == VCHIQ_SUCCESS)
   {
      ((VCHIQ_BIVCM_VC_STATE_T*)state->platform_state)->inited = 1;
   }
   return status;
}

VCHIQ_VC_STATE_T*
vchiq_platform_get_vc_state(VCHIQ_STATE_T *state)
{
   if(!((VCHIQ_BIVCM_VC_STATE_T*)state->platform_state)->inited)
   {
      vcos_demand(0);
   }
   return &((VCHIQ_BIVCM_VC_STATE_T*)state->platform_state)->vc_state;
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
   const PAGELIST_T *pagelist;
   int len;
   
   len = bulk->size;

   if (len != bulk->remote_size)
   {
      bulk->actual = VCHIQ_BULK_ACTUAL_ABORTED;
      return;
   }

   pagelist = (PAGELIST_T *)bulk->remote_data;

   if ((len == pagelist->length) &&
       (((bulk->dir == VCHIQ_BULK_RECEIVE) &&
         (pagelist->type == PAGELIST_WRITE)) ||
        ((bulk->dir == VCHIQ_BULK_TRANSMIT) &&
         (pagelist->type != PAGELIST_WRITE))))
   {
      bulk->actual = dma_memcpy_pagelist(bulk->data, pagelist);
   }
   else
   {
      vcos_assert_msg(0, "vchiq: invalid pagelist at %x - length %x, type %x (bulk %s)",
         (unsigned int)pagelist, pagelist->length, pagelist->type,
         (bulk->dir == VCHIQ_BULK_TRANSMIT) ? "TX" : "RX");
      bulk->actual = VCHIQ_BULK_ACTUAL_ABORTED;
   }
}

VCHIQ_STATUS_T
vchiq_copy_from_user(void *dst, const void *src, int size)
{
#if defined(VCHIQ_SM_ALLOC_VCDDR)
   VC_MEMCPY(dst, src, size);
#else
   memcpy(dst, src, size);
#endif
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
