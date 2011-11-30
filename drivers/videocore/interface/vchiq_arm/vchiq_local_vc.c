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
#include "vcfw/rtos/common/rtos_common_mem.h"
#include "helpers/dmalib/dmalib.h"

#define NUM_SLOTS 32

VCHIQ_INSTANCE_STRUCT_T vchiq_instances[2];

static VCHIQ_SLOT_ZERO_T *vchiq_slot_zero;


void
remote_event_signal(REMOTE_EVENT_T *event)
{
	vcos_event_signal(event->event);
}

VCHIQ_STATUS_T
vchiq_platform_init(void)
{
   if (vchiq_num_instances < 2)
   {
      if (vchiq_num_instances == 0)
      {
         void *slot_mem;
         int num_slots;

         num_slots = VCHIQ_SLOT_ZERO_SLOTS + 2 * NUM_SLOTS;
         slot_mem = vcos_malloc_aligned(num_slots * VCHIQ_SLOT_SIZE, VCHIQ_SLOT_SIZE, "vchiq-slots");
         if (!slot_mem)
            return VCHIQ_ERROR;

         vchiq_slot_zero = vchiq_init_slots(slot_mem, num_slots * VCHIQ_SLOT_SIZE);

         if (!vchiq_slot_zero)
         {
            vcos_free(slot_mem);
            return VCHIQ_ERROR;
         }
      }

      vchiq_init_state(&vchiq_instances[vchiq_num_instances].state,
                       vchiq_slot_zero,
                       vchiq_num_instances == 0);

      if (vchiq_num_instances == 1)
      {
         /* This state initialisation may have erased a signal - signal anyway
            to be sure. This is a bit of a hack, caused by the desire for the
            server threads to be started on the same core as the calling thread. */
         vcos_event_signal(vchiq_slot_zero->slave.trigger.event);
      }
      vchiq_num_instances++;
   }

   return VCHIQ_SUCCESS;
}

VCHIQ_STATUS_T
vchiq_platform_init_state(VCHIQ_STATE_T *state)
{ // not required on this platform
   return VCHIQ_SUCCESS;
}


VCHIQ_STATUS_T
vchiq_copy_from_user(void *dst, const void *src, int size)
{
   VC_MEMCPY(dst, src, size);
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
      if (!data)
         return VCHIQ_ERROR;
      data += (unsigned int)offset;
   }
   else
      data = offset;

   if (!VCHIQ_IS_SAFE_DATA(data))
      return VCHIQ_ERROR;

   bulk->data = data;
   bulk->handle = memhandle;

   return VCHIQ_SUCCESS;
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
   int size = bulk->size;

   if (size != bulk->remote_size)
   {
      bulk->actual = VCHIQ_BULK_ACTUAL_ABORTED;
      return;
   }

   if (bulk->dir == VCHIQ_BULK_TRANSMIT)
   {
      VC_MEMCPY(bulk->remote_data, bulk->data, size);
   }
   else
   {
      VC_MEMCPY(bulk->data, bulk->remote_data, size);
   }

   bulk->actual = size;
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
   vcos_assert_msg(0, "Suspend/resume not supported");
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

VCHIQ_VC_STATE_T*
vchiq_platform_get_vc_state(VCHIQ_STATE_T *state)
{
   vcos_unused(state);
   return NULL;
}
