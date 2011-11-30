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
#include "vchiq_2835.h"
#include "vchiq_2835_vc.h"
#include "vchiq_vc_dma.h"
#include "vcfw/rtos/common/rtos_common_mem.h"
#include "helpers/dmalib/dmalib.h"

static const ARM_DRIVER_T *arm_driver;
static DRIVER_HANDLE_T arm_handle;
static VCHIQ_SLOT_ZERO_T *g_vchiq_slot_zero;

VCHIQ_INSTANCE_STRUCT_T vchiq_instances[1];

VCHIQ_STATUS_T
vchiq_vc_initialise(const ARM_DRIVER_T *arm, DRIVER_HANDLE_T handle,
   uint32_t slotbase)
{
   VCHIQ_STATUS_T status;

   if (!vcos_verify((vchiq_num_instances == 0) &&
                    ((slotbase % CACHE_LINE_SIZE) == 0)))
      return VCHIQ_ERROR;

   g_vchiq_slot_zero = (VCHIQ_SLOT_ZERO_T *)slotbase;

   status = init_dma_xfers(
      g_vchiq_slot_zero->platform_data[VCHIQ_PLATFORM_FRAGMENTS_COUNT_IDX],
      (FRAGMENTS_T *)(g_vchiq_slot_zero->platform_data[VCHIQ_PLATFORM_FRAGMENTS_OFFSET_IDX]));

   if (status == VCHIQ_SUCCESS)
   {
      status = vchiq_init_state(&vchiq_instances[0].state, g_vchiq_slot_zero, 1/*master*/);

      if (status == VCHIQ_SUCCESS)
      {
         arm_driver = arm;
         arm_handle = handle;

         arm_driver->set_doorbell_isr(arm_handle, 2,
            (ARM_DOORBELL_ISR)remote_event_pollall,
            &vchiq_instances[0].state);
	  
         vchiq_num_instances = 1;
      }
   }

   if (status != VCHIQ_SUCCESS)
      uninit_dma_xfers();

   return status;
}

VCHIQ_STATUS_T
vchiq_platform_init(void)
{
   /* Nothing to do in this platform */
   return VCHIQ_SUCCESS;
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
      arm_driver->doorbell_ring(arm_handle, 0, 1);
   }
}

void
vchiq_platform_paused(VCHIQ_STATE_T *state)
{
   vcos_unused(state);
   vcos_assert_msg(0, "Suspend/resume not supported");
}

void
vchiq_platform_resumed(VCHIQ_STATE_T *state)
{
   vcos_unused(state);
}

VCHIQ_STATUS_T
vchiq_platform_init_state(VCHIQ_STATE_T *state)
{
   vcos_unused(state);
   return VCHIQ_SUCCESS;
}

VCHIQ_VC_STATE_T*
vchiq_platform_get_vc_state(VCHIQ_STATE_T *state)
{
   vcos_unused(state);
   return NULL;
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
