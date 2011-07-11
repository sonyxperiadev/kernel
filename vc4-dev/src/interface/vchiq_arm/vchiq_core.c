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

#include "vchiq_core.h"

#define VCHIQ_SLOT_HANDLER_STACK 8192

#define SLOT_INFO_FROM_INDEX(state, index) (state->slot_info + (index))
#define SLOT_DATA_FROM_INDEX(state, index) (state->slot_data + (index))
#define SLOT_INDEX_FROM_DATA(state, data) (((unsigned int)((char *)data - (char *)state->slot_data)) / VCHIQ_SLOT_SIZE)
#define SLOT_INDEX_FROM_INFO(state, info) ((unsigned int)(info - state->slot_info))
#define SLOT_QUEUE_INDEX_FROM_POS(pos) ((int)((unsigned int)(pos) / VCHIQ_SLOT_SIZE))

#define VCOS_LOG_CATEGORY (&vchiq_core_log_category)

#define BULK_INDEX(x) (x & (VCHIQ_NUM_SERVICE_BULKS - 1))

typedef struct bulk_waiter_struct
{
   VCOS_EVENT_T event;
   int actual;
} BULK_WAITER_T;

vcos_static_assert(sizeof(VCHIQ_HEADER_T) == 8);   /* we require this for consistency between endpoints */
vcos_static_assert(IS_POW2(sizeof(VCHIQ_HEADER_T)));
vcos_static_assert(IS_POW2(VCHIQ_NUM_CURRENT_BULKS));
vcos_static_assert(IS_POW2(VCHIQ_NUM_SERVICE_BULKS));

VCOS_LOG_CAT_T vchiq_core_log_category;
VCOS_LOG_CAT_T vchiq_core_msg_log_category;
VCOS_LOG_LEVEL_T vchiq_default_core_log_level = VCOS_LOG_WARN;
VCOS_LOG_LEVEL_T vchiq_default_core_msg_log_level = VCOS_LOG_WARN;

static const char *const srvstate_names[] =
{
   "FREE",
   "HIDDEN",
   "LISTENING",
   "OPENING",
   "OPEN",
   "CLOSESENT",
   "CLOSING",
   "CLOSEWAIT"
};

static const char *const reason_names[] =
{
   "SERVICE_OPENED",
   "SERVICE_CLOSED",
   "MESSAGE_AVAILABLE",
   "BULK_TRANSMIT_DONE",
   "BULK_RECEIVE_DONE",
   "BULK_TRANSMIT_ABORTED",
   "BULK_RECEIVE_ABORTED"
};

static const char *msg_type_str( unsigned int msg_type )
{
   switch (msg_type) {
   case VCHIQ_MSG_PADDING:       return "PADDING";
   case VCHIQ_MSG_CONNECT:       return "CONNECT";
   case VCHIQ_MSG_OPEN:          return "OPEN";
   case VCHIQ_MSG_OPENACK:       return "OPENACK";
   case VCHIQ_MSG_CLOSE:         return "CLOSE";
   case VCHIQ_MSG_DATA:          return "DATA";
   case VCHIQ_MSG_BULK_RX:       return "BULK_RX";
   case VCHIQ_MSG_BULK_TX:       return "BULK_TX";
   case VCHIQ_MSG_BULK_RX_DONE:  return "BULK_RX_DONE";
   case VCHIQ_MSG_BULK_TX_DONE:  return "BULK_TX_DONE";
   }
   return "???";
}

static inline void
vchiq_set_service_state(VCHIQ_SERVICE_T *service, int newstate)
{
   vcos_log_info("%d: srv:%d %s->%s", service->state->id, service->localport,
      srvstate_names[service->srvstate],
      srvstate_names[newstate]);
   service->srvstate = newstate;
}

static inline VCHIQ_STATUS_T
make_service_callback(VCHIQ_SERVICE_T *service, VCHIQ_REASON_T reason,
   VCHIQ_HEADER_T *header, void *bulk_userdata)
{
   vcos_log_trace("%d: callback:%d (%s, %x, %x)", service->state->id,
      service->localport, reason_names[reason],
      (unsigned int)header, (unsigned int)bulk_userdata);
   return service->base.callback(reason, header, &service->base, bulk_userdata);
}

static inline void
remote_event_create(REMOTE_EVENT_T *event)
{
   event->armed = 0;
   /* Don't clear the 'fired' flag because it may already have been set by the other side */
   vcos_event_create(event->event, "vchiq");
}

static inline void
remote_event_destroy(REMOTE_EVENT_T *event)
{
   vcos_event_delete(event->event);
}

static inline int
remote_event_wait(REMOTE_EVENT_T *event)
{
   if (!event->fired)
   {
      event->armed = 1;
      if (event->fired) /* Also ensures the write has completed */
         event->armed = 0;
      else if (vcos_event_wait(event->event) != VCOS_SUCCESS)
         return 0;
   }

   event->fired = 0;
   return 1;
}

static inline void
remote_event_signal_local(REMOTE_EVENT_T *event)
{
   event->armed = 0;
   vcos_event_signal(event->event);
}

static inline void
remote_event_poll(REMOTE_EVENT_T *event)
{
   if (event->armed)
      remote_event_signal_local(event);
}

void
remote_event_pollall(VCHIQ_STATE_T *state)
{
   remote_event_poll(&state->local->trigger);
   remote_event_poll(&state->local->recycle);
}

/* Round up message sizes so that any space at the end of a slot is always big
   enough for a header. This relies on header size being a power of two, which
   has been verified earlier by a static assertion. */

static inline unsigned int
calc_stride(unsigned int size)
{
   /* Allow room for the header */
   size += sizeof(VCHIQ_HEADER_T);

   /* Round up */
   return (size + sizeof(VCHIQ_HEADER_T) - 1) & ~(sizeof(VCHIQ_HEADER_T) - 1);
}

static VCHIQ_SERVICE_T *
get_listening_service(VCHIQ_STATE_T *state, int fourcc)
{
   int i;

   vcos_assert(fourcc != VCHIQ_FOURCC_INVALID);

   for (i = 0; i < state->unused_service; i++)
   {
      VCHIQ_SERVICE_T *service = &state->services[i];
      if ((service->public_fourcc == fourcc) &&
         ((service->srvstate == VCHIQ_SRVSTATE_LISTENING) ||
         ((service->srvstate == VCHIQ_SRVSTATE_OPEN) &&
         (service->remoteport == VCHIQ_PORT_FREE))))
         return service;
   }

   return NULL;
}

static VCHIQ_SERVICE_T *
get_connected_service(VCHIQ_STATE_T *state, unsigned int port)
{
   int i;
   for (i = 0; i < state->unused_service; i++) {
      VCHIQ_SERVICE_T *service = &state->services[i];
      if ((service->srvstate == VCHIQ_SRVSTATE_OPEN)
         && (service->remoteport == port)) {
         return service;
      }
   }
   return NULL;
}

static inline void
request_poll(VCHIQ_SERVICE_T *service, int poll_type)
{
   VCHIQ_STATE_T *state = service->state;

   vcos_atomic_flags_or(&service->poll_flags, (1 << poll_type));
   vcos_atomic_flags_or(&state->poll_services[service->localport>>5],
      (1 <<(service->localport & 0x1f)));

   /* ... and ensure the slot handler runs. */
   remote_event_signal_local(&state->local->trigger);
}

/* Called from queue_message, by the slot handler and application threads,
   with slot_mutex held */
static VCHIQ_HEADER_T *
reserve_space(VCHIQ_STATE_T *state, int space, int is_blocking)
{
   VCHIQ_SHARED_STATE_T *local = state->local;
   int tx_pos = state->local_tx_pos;
   int slot_space = VCHIQ_SLOT_SIZE - (tx_pos & VCHIQ_SLOT_MASK);

   if (space > slot_space) {
      VCHIQ_HEADER_T *header;
      /* Fill the remaining space with padding */
      vcos_assert(state->tx_data != NULL);
      header = (VCHIQ_HEADER_T *) (state->tx_data + (tx_pos & VCHIQ_SLOT_MASK));
      header->msgid = VCHIQ_MSGID_PADDING;
      header->size = slot_space - sizeof(VCHIQ_HEADER_T);

      tx_pos += slot_space;
   }

   /* If necessary, get the next slot. */
   if ((tx_pos & VCHIQ_SLOT_MASK) == 0)
   {
      int slot_index;

      /* If there is no free slot... */
      if (tx_pos == (state->slot_queue_available * VCHIQ_SLOT_SIZE))
      {
         /* ...wait for one. */
         VCHIQ_STATS_INC(state, slot_stalls);

         /* But first, flush through the last slot. */
         local->tx_pos = tx_pos;
         remote_event_signal(&state->remote->trigger);

         do {
            if (!is_blocking ||
               (vcos_event_wait(&state->slot_available_event) != VCOS_SUCCESS))
            {
               return NULL; /* No space available now */
            }
         }
         while (tx_pos == (state->slot_queue_available * VCHIQ_SLOT_SIZE));
      }

      slot_index = local->slot_queue[SLOT_QUEUE_INDEX_FROM_POS(tx_pos) & VCHIQ_SLOT_QUEUE_MASK];
      state->tx_data = (char *)SLOT_DATA_FROM_INDEX(state, slot_index);
   }

   state->local_tx_pos = tx_pos + space;

   return (VCHIQ_HEADER_T *)(state->tx_data + (tx_pos & VCHIQ_SLOT_MASK));
}

/* Called by the recycle thread */
static void
process_free_queue(VCHIQ_STATE_T *state)
{
   VCHIQ_SHARED_STATE_T *local = state->local;
   BITSET_T service_found[BITSET_SIZE(VCHIQ_MAX_SERVICES)];
   int slot_queue_available;

   /* Find slots which have been freed by the other side, and return them to
      the available queue. */
   slot_queue_available = state->slot_queue_available;

   vcos_mutex_lock(&state->slot_mutex);

   /* A read memory barrier is not necessary here because
      local->slot_queue_recycle is only read by one thread - slot_mutex is
      protecting other state. */

   while (slot_queue_available != local->slot_queue_recycle)
   {
      int pos;
      int slot_index = local->slot_queue[slot_queue_available++ & VCHIQ_SLOT_QUEUE_MASK];
      char *data = (char *)SLOT_DATA_FROM_INDEX(state, slot_index);

      vcos_log_trace("%d: pfq %d=%x %x %x", state->id, slot_index,
         (unsigned int)data, local->slot_queue_recycle,
         slot_queue_available);

      /* Initialise the bitmask for services which have used this slot */
      BITSET_ZERO(service_found);

      pos = 0;

      while (pos < VCHIQ_SLOT_SIZE)
      {
         VCHIQ_HEADER_T *header = (VCHIQ_HEADER_T *)(data + pos);
         int msgid = header->msgid;
         if (VCHIQ_MSG_TYPE(msgid) == VCHIQ_MSG_DATA)
         {
            int port = VCHIQ_MSG_SRCPORT(msgid);
            if (!BITSET_IS_SET(service_found, port))
            {
               VCHIQ_SERVICE_T *service = &state->services[port];

               /* Set the found bit for this service */
               BITSET_SET(service_found, port);

               vcos_assert(service->slot_use_count > 0);

               service->slot_use_count--;
               /* Signal the service in case it has dropped below its quota */
               vcos_event_signal(&service->quota_event);
               vcos_log_trace("%d: pfq:%d %x@%x - slot_use->%d",
                  state->id, service->localport,
                  header->size, (unsigned int)header,
                  service->slot_use_count);
            }
         }

         pos += calc_stride(header->size);
      }

      vcos_assert(pos == VCHIQ_SLOT_SIZE);
   }

   if (slot_queue_available != state->slot_queue_available)
   {
      state->slot_queue_available = slot_queue_available;
      vcos_event_signal(&state->slot_available_event);
   }

   vcos_mutex_unlock(&state->slot_mutex);
}

/* Called by the slot handler and application threads */
static VCHIQ_STATUS_T
queue_message(VCHIQ_STATE_T *state, VCHIQ_SERVICE_T *service,
   int msgid, const VCHIQ_ELEMENT_T *elements,
   int count, int size, int is_blocking)
{
   VCHIQ_SHARED_STATE_T *local;
   VCHIQ_HEADER_T *header;

   unsigned int stride;

   local = state->local;

   stride = calc_stride(size);

   vcos_assert(stride <= VCHIQ_SLOT_SIZE);

   if (vcos_mutex_lock(&state->slot_mutex) != VCOS_SUCCESS)
      return VCHIQ_RETRY;

   if (service)
   {
      int tx_end_index = SLOT_QUEUE_INDEX_FROM_POS(state->local_tx_pos + stride - 1);

      /* ...ensure it doesn't use more than its quota of slots */
      while ((tx_end_index != service->previous_tx_index) &&
         (service->slot_use_count == service->slot_quota))
      {
         vcos_log_trace("%d: qm:%d %s,%x - quota stall",
            state->id, service->localport,
            msg_type_str(VCHIQ_MSG_TYPE(msgid)), size);
         VCHIQ_SERVICE_STATS_INC(service, quota_stalls);
         vcos_mutex_unlock(&state->slot_mutex);
         if (vcos_event_wait(&service->quota_event) != VCOS_SUCCESS)
            return VCHIQ_RETRY;
         if (vcos_mutex_lock(&state->slot_mutex) != VCOS_SUCCESS)
            return VCHIQ_RETRY;
         vcos_assert(service->slot_use_count <= service->slot_quota);
         tx_end_index = SLOT_QUEUE_INDEX_FROM_POS(state->local_tx_pos + stride - 1);
      }
   }

   header = reserve_space(state, stride, is_blocking);

   if (!header) {
      if (service)
         VCHIQ_SERVICE_STATS_INC(service, slot_stalls);
      vcos_mutex_unlock(&state->slot_mutex);
      return VCHIQ_RETRY;
   }

   if (service) {
      int i, pos;
      int tx_end_index;

      vcos_log_trace("%d: qm %s@%x,%x (%d->%d)", state->id,
         msg_type_str(VCHIQ_MSG_TYPE(msgid)),
         (unsigned int)header, size,
         VCHIQ_MSG_SRCPORT(msgid),
         VCHIQ_MSG_DSTPORT(msgid));

      for (i = 0, pos = 0; i < (unsigned int)count;
         pos += elements[i++].size)
         if (elements[i].size) {
            if (vchiq_copy_from_user
               (header->data + pos, elements[i].data,
               (size_t) elements[i].size) !=
               VCHIQ_SUCCESS) {
               vcos_mutex_unlock(&state->slot_mutex);
               return VCHIQ_ERROR;
            }
            if (i == 0) {
               vcos_log_dump_mem( &vchiq_core_msg_log_category,
                              "Sent", 0, header->data + pos,
                              vcos_min( 64, elements[0].size ));
            }
         }

      /* If this transmission can't fit in the last slot used by this service... */
      tx_end_index = SLOT_QUEUE_INDEX_FROM_POS(state->local_tx_pos - 1);
      if (tx_end_index != service->previous_tx_index)
      {
         service->slot_use_count++;
         vcos_log_trace("%d: qm:%d %s,%x - slot_use->%d",
            state->id, service->localport,
            msg_type_str(VCHIQ_MSG_TYPE(msgid)), size,
            service->slot_use_count);
      }

      service->previous_tx_index = tx_end_index;
   } else {
      vcos_log_info("%d: qm %s@%x,%x (%d->%d)", state->id,
         msg_type_str(VCHIQ_MSG_TYPE(msgid)),
         (unsigned int)header, size,
         VCHIQ_MSG_SRCPORT(msgid),
         VCHIQ_MSG_DSTPORT(msgid));
      if (size != 0)
      { 
         vcos_assert((count == 1) && (size == elements[0].size));
         memcpy(header->data, elements[0].data, elements[0].size);
      }
   }

   header->msgid = msgid;
   header->size = size;

   if (vcos_is_log_enabled( &vchiq_core_msg_log_category, VCOS_LOG_INFO))
   {
      int svc_fourcc;

      svc_fourcc = state->services[VCHIQ_MSG_SRCPORT(msgid)].base.fourcc;

      vcos_log_impl( &vchiq_core_msg_log_category,
         VCOS_LOG_INFO,
         "Sent Msg %s(%u) to %c%c%c%c s:%u d:%d len:%d",
         msg_type_str(VCHIQ_MSG_TYPE(msgid)),
         VCHIQ_MSG_TYPE(msgid),
         VCHIQ_FOURCC_AS_4CHARS(svc_fourcc),
         VCHIQ_MSG_SRCPORT(msgid),
         VCHIQ_MSG_DSTPORT(msgid),
         size );
   }

   /* Make the new tx_pos visible to the peer. remote_event_signal must
      include a suitable write barrier. */
   local->tx_pos = state->local_tx_pos;

   vcos_mutex_unlock(&state->slot_mutex);

   remote_event_signal(&state->remote->trigger);

   return VCHIQ_SUCCESS;
}

static inline void
claim_slot(VCHIQ_SLOT_INFO_T *slot)
{
   slot->use_count++;
}

static void
release_slot(VCHIQ_STATE_T *state, VCHIQ_SLOT_INFO_T *slot_info)
{
   vcos_mutex_lock(&state->recycle_mutex);

   slot_info->release_count++;
   if (slot_info->release_count == slot_info->use_count)
   {
      int slot_queue_recycle;
      /* Add to the freed queue */

      /* A read barrier is necessary here to prevent speculative fetches of
         remote->slot_queue_recycle from overtaking the mutex. */
      vcos_rmb();

      slot_queue_recycle = state->remote->slot_queue_recycle;
      state->remote->slot_queue[slot_queue_recycle & VCHIQ_SLOT_QUEUE_MASK] =
         SLOT_INDEX_FROM_INFO(state, slot_info);
      state->remote->slot_queue_recycle = slot_queue_recycle + 1;
      vcos_log_info("%d: release_slot %d - recycle->%x",
         state->id, SLOT_INDEX_FROM_INFO(state, slot_info),
         state->remote->slot_queue_recycle);

      /* A write barrier is necessary, but remote_event_signal contains one. */
      remote_event_signal(&state->remote->recycle);
   }

   vcos_mutex_unlock(&state->recycle_mutex);
}

/* Called by the slot handler - don't hold the bulk mutex */
static VCHIQ_STATUS_T
notify_bulks(VCHIQ_SERVICE_T *service, VCHIQ_BULK_QUEUE_T *queue)
{
   VCHIQ_STATUS_T status = VCHIQ_SUCCESS;

   vcos_log_trace("%d: nb:%d %cx - p=%x rn=%x r=%x",
      service->state->id, service->localport,
      (queue == &service->bulk_tx) ? 't' : 'r',
      queue->process, queue->remote_notify, queue->remove);

   if (service->state->is_master)
   {
      while (queue->remote_notify != queue->process)
      {
         VCHIQ_BULK_T *bulk = &queue->bulks[BULK_INDEX(queue->remote_notify)];
         int msgtype = (bulk->dir == VCHIQ_BULK_TRANSMIT) ?
            VCHIQ_MSG_BULK_RX_DONE : VCHIQ_MSG_BULK_TX_DONE;
         int msgid = VCHIQ_MAKE_MSG(msgtype, service->localport, service->remoteport);
         VCHIQ_ELEMENT_T element = { &bulk->actual, 4 };
         /* Only reply to non-dummy bulk requests */
         if (bulk->remote_data)
         {
            status = queue_message(service->state, NULL, msgid, &element, 1, 4, 0);
            if (status != VCHIQ_SUCCESS)
               break;
         }
         queue->remote_notify++;
      }
   }
   else
   {
      queue->remote_notify = queue->process;
   }

   if (status == VCHIQ_SUCCESS)
   {
      while (queue->remove != queue->remote_notify)
      {
         VCHIQ_BULK_T *bulk = &queue->bulks[BULK_INDEX(queue->remove)];

         /* Only generate callbacks for non-dummy bulk requests */
         if (bulk->data)
         {
            if (bulk->mode == VCHIQ_BULK_MODE_BLOCKING)
            {
               BULK_WAITER_T *waiter = (BULK_WAITER_T *)bulk->userdata;
               if (waiter)
               {
                  waiter->actual = bulk->actual;
                  vcos_event_signal(&waiter->event);
               }
            }
            else if (bulk->mode == VCHIQ_BULK_MODE_CALLBACK)
            {
               VCHIQ_REASON_T reason = (bulk->dir == VCHIQ_BULK_TRANSMIT) ?
                  ((bulk->actual == VCHIQ_BULK_ACTUAL_ABORTED) ?
                     VCHIQ_BULK_TRANSMIT_ABORTED : VCHIQ_BULK_TRANSMIT_DONE) :
                  ((bulk->actual == VCHIQ_BULK_ACTUAL_ABORTED) ?
                     VCHIQ_BULK_RECEIVE_ABORTED : VCHIQ_BULK_RECEIVE_DONE);
               status = make_service_callback(service, reason,
                  NULL, bulk->userdata);
               if (status == VCHIQ_RETRY)
                  break;
            }
         }

         queue->remove++;
         vcos_event_signal(&service->bulk_remove_event);
      }
   }

   if (status != VCHIQ_SUCCESS)
      request_poll(service, (queue == &service->bulk_tx) ?
         VCHIQ_POLL_TXNOTIFY : VCHIQ_POLL_RXNOTIFY);

   return status;
}

/* Called by the slot handler thread */
static void
poll_services(VCHIQ_STATE_T *state)
{
   int group, i;

   for (group = 0; group < BITSET_SIZE(state->unused_service); group++)
   {
      uint32_t flags;
      flags = vcos_atomic_flags_get_and_clear(&state->poll_services[group]);
      for (i = 0; flags; i++)
      {
         if (flags & (1 << i))
         {
            VCHIQ_SERVICE_T *service = &state->services[(group<<5) + i];
            uint32_t service_flags =
               vcos_atomic_flags_get_and_clear(&service->poll_flags);
            if (service_flags & (1 << VCHIQ_POLL_TERMINATE))
            {
               vcos_log_info("%d: ps - terminate %d<->%d", state->id, service->localport, service->remoteport);
               if (vchiq_close_service_internal(service, 0/*!close_recvd*/) != VCHIQ_SUCCESS)
                  request_poll(service, VCHIQ_POLL_TERMINATE);
            }
            if (service_flags & (1 << VCHIQ_POLL_TXNOTIFY))
               notify_bulks(service, &service->bulk_tx);
            if (service_flags & (1 << VCHIQ_POLL_RXNOTIFY))
               notify_bulks(service, &service->bulk_rx);
            flags &= ~(1 << i);
         }
      }
   }
}

/* Called by the slot handler or application threads, holding the bulk mutex. */
static int
resolve_bulks(VCHIQ_SERVICE_T *service, VCHIQ_BULK_QUEUE_T *queue)
{
   VCHIQ_STATE_T *state = service->state;
   int resolved = 0;

   if ((queue->process != queue->local_insert) &&
      (queue->process != queue->remote_insert))
   {
      VCHIQ_BULK_T *bulk = &queue->bulks[BULK_INDEX(queue->process)];

      vcos_log_trace("%d: rb:%d %cx - li=%x ri=%x p=%x",
         state->id, service->localport,
         (queue == &service->bulk_tx) ? 't' : 'r',
         queue->local_insert, queue->remote_insert,
         queue->process);

      vcos_assert((int)(queue->local_insert - queue->process) > 0);
      vcos_assert((int)(queue->remote_insert - queue->process) > 0);
      vchiq_transfer_bulk(bulk);

      if (vcos_is_log_enabled( &vchiq_core_msg_log_category, VCOS_LOG_INFO))
      {
         const char *header = (queue == &service->bulk_tx) ?
            "Send Bulk to" : "Recv Bulk from";
         if (bulk->actual != VCHIQ_BULK_ACTUAL_ABORTED)
            vcos_log_impl( &vchiq_core_msg_log_category,
               VCOS_LOG_INFO,
               "%s %c%c%c%c d:%d len:%d",
               header,
               VCHIQ_FOURCC_AS_4CHARS(service->base.fourcc),
               service->remoteport,
               bulk->size );
         else
            vcos_log_impl( &vchiq_core_msg_log_category,
               VCOS_LOG_INFO,
               "%s %c%c%c%c d:%d ABORTED - tx len:%d, rx len:%d",
               header,
               VCHIQ_FOURCC_AS_4CHARS(service->base.fourcc),
               service->remoteport,
               bulk->size,
               bulk->remote_size );
      }

      vchiq_complete_bulk(bulk);
      queue->process++;
      resolved++;
   }
   return resolved;
}

/* Called with the bulk_mutex held */
static void
abort_outstanding_bulks(VCHIQ_SERVICE_T *service, VCHIQ_BULK_QUEUE_T *queue)
{
   int is_tx = (queue == &service->bulk_tx);
   vcos_log_trace("%d: aob:%d %cx - li=%x ri=%x p=%x",
      service->state->id, service->localport, is_tx ? 't' : 'r',
      queue->local_insert, queue->remote_insert, queue->process);

   vcos_assert((int)(queue->local_insert - queue->process) >= 0);
   vcos_assert((int)(queue->remote_insert - queue->process) >= 0);

   while ((queue->process != queue->local_insert) ||
      (queue->process != queue->remote_insert))
   {
      VCHIQ_BULK_T *bulk = &queue->bulks[BULK_INDEX(queue->process)];

      if (queue->process == queue->remote_insert)
      {
         /* fabricate a matching dummy bulk */
         bulk->remote_data = NULL;
         bulk->remote_size = 0;
         queue->remote_insert++;
      }

      if (queue->process != queue->local_insert)
      {
         vchiq_complete_bulk(bulk);

         if (vcos_is_log_enabled( &vchiq_core_msg_log_category, VCOS_LOG_INFO))
         {
            vcos_log_impl( &vchiq_core_msg_log_category,
               VCOS_LOG_INFO,
               "%s %c%c%c%c d:%d ABORTED - tx len:%d, rx len:%d",
               is_tx ? "Send Bulk to" : "Recv Bulk from",
               VCHIQ_FOURCC_AS_4CHARS(service->base.fourcc),
               service->remoteport,
               bulk->size,
               bulk->remote_size );
         }
      }
      else
      {
         /* fabricate a matching dummy bulk */
         bulk->data = NULL;
         bulk->size = 0;
         bulk->actual = VCHIQ_BULK_ACTUAL_ABORTED;
         bulk->dir = is_tx ? VCHIQ_BULK_TRANSMIT : VCHIQ_BULK_RECEIVE;
         queue->local_insert++;
      }

      queue->process++;
   }
}

/* Called by the slot handler thread */
static void
parse_rx_slots(VCHIQ_STATE_T *state)
{
   VCHIQ_SHARED_STATE_T *remote = state->remote;
   int tx_pos;
   DEBUG_INITIALISE(state->local)

   tx_pos = remote->tx_pos;

   while (state->rx_pos != tx_pos) {
      VCHIQ_SERVICE_T *service = NULL;
      VCHIQ_HEADER_T *header;
      int msgid, size;
      int type;

      DEBUG_TRACE(PARSE_LINE);
      if (!state->rx_data)
      {
         int rx_index;
         vcos_assert((state->rx_pos & VCHIQ_SLOT_MASK) == 0);
         rx_index = remote->slot_queue[SLOT_QUEUE_INDEX_FROM_POS(state->rx_pos) & VCHIQ_SLOT_QUEUE_MASK];
         state->rx_data = (char *)SLOT_DATA_FROM_INDEX(state, rx_index);
         state->rx_info = SLOT_INFO_FROM_INDEX(state, rx_index);

         /* Initialise use_count to one, and increment release_count at the end
            of the slot to avoid releasing the slot prematurely. */
         state->rx_info->use_count = 1;
         state->rx_info->release_count = 0;
      }

      header = (VCHIQ_HEADER_T *)(state->rx_data + (state->rx_pos & VCHIQ_SLOT_MASK));
      DEBUG_VALUE(PARSE_HEADER, (int)header);
      msgid = header->msgid;
      DEBUG_VALUE(PARSE_MSGID, msgid);
      size = header->size;
      type = VCHIQ_MSG_TYPE(msgid);

      if ( vcos_is_log_enabled( &vchiq_core_msg_log_category, VCOS_LOG_INFO))
      {
         int svc_fourcc;

         svc_fourcc = state->services[VCHIQ_MSG_DSTPORT(msgid)].base.fourcc;
         vcos_log_impl( &vchiq_core_msg_log_category,
            VCOS_LOG_INFO,
            "Rcvd Msg %s(%u) from %c%c%c%c s:%d d:%d len:%d",
            msg_type_str(type), type,
            VCHIQ_FOURCC_AS_4CHARS(svc_fourcc),
            VCHIQ_MSG_SRCPORT(msgid),
            VCHIQ_MSG_DSTPORT(msgid),
            size );
         if (size > 0) {
            vcos_log_dump_mem( &vchiq_core_msg_log_category,
                           "Rcvd", 0, header->data,
                           vcos_min( 64, size ));
         }
      }

      if (((unsigned int)header & VCHIQ_SLOT_MASK) + calc_stride(size) > VCHIQ_SLOT_SIZE)
      {
         vcos_log_error("header %x (msgid %x) - size %x too big for slot",
            (unsigned int)header, (unsigned int)msgid, (unsigned int)size);
         vcos_assert(0);
      }

      switch (type) {
      case VCHIQ_MSG_OPEN:
         vcos_assert(VCHIQ_MSG_DSTPORT(msgid) == 0);
         if (vcos_verify(size == 4 || size == 8)) {
            unsigned short remoteport = VCHIQ_MSG_SRCPORT(msgid);
            unsigned int fourcc;

            fourcc = *(int *)header->data;
            vcos_log_info("%d: prs OPEN@%x (%d->'%c%c%c%c')",
               state->id, (unsigned int)header,
               VCHIQ_MSG_SRCPORT(msgid),
               VCHIQ_FOURCC_AS_4CHARS(fourcc));

            service = get_listening_service(state, fourcc);

            if (service)
            {
               /* A matching service exists */
               if (service->srvstate == VCHIQ_SRVSTATE_LISTENING)
               {
                  /* Acknowledge the OPEN */
                  if (queue_message(state, NULL,
                     VCHIQ_MAKE_MSG(VCHIQ_MSG_OPENACK, service->localport, remoteport),
                     NULL, 0, 0, 0) == VCHIQ_RETRY)
                     return;  /* Bail out if not ready */

                  /* The service is now open */
                  vchiq_set_service_state(service, VCHIQ_SRVSTATE_OPEN);
               }

               service->remoteport = remoteport;
               service->client_id = ((size == 8) ? ((int *)header->data)[1] : 0);
               if (make_service_callback(service, VCHIQ_SERVICE_OPENED,
                  NULL, NULL) == VCHIQ_RETRY)
               {
                  /* Bail out if not ready */
                  service->remoteport = VCHIQ_PORT_FREE;
                  return;
               }

               /* Break out, and skip the failure handling */
               break;
            }
         }

         /* No available service, or an invalid request - send a CLOSE */
         if (queue_message(state, NULL,
            VCHIQ_MAKE_MSG(VCHIQ_MSG_CLOSE, 0, VCHIQ_MSG_SRCPORT(msgid)),
            NULL, 0, 0, 0) == VCHIQ_RETRY)
            return;  /* Bail out if not ready */
         break;
      case VCHIQ_MSG_OPENACK:
         {
            unsigned int localport =
               VCHIQ_MSG_DSTPORT(msgid);
            unsigned int remoteport =
               VCHIQ_MSG_SRCPORT(msgid);
            vcos_log_info("%d: prs OPENACK@%x (%d->%d)",
               state->id, (unsigned int)header,
               remoteport, localport);
            service = &state->services[localport];
            if (vcos_verify
               (localport < VCHIQ_MAX_SERVICES)
               && (service->srvstate ==
               VCHIQ_SRVSTATE_OPENING)) {
               service->remoteport = remoteport;
               vchiq_set_service_state(service,
                        VCHIQ_SRVSTATE_OPEN);
               vcos_event_signal(&service->remove_event);
            }
         }
         break;
      case VCHIQ_MSG_CLOSE:
         {
            unsigned int localport =
               VCHIQ_MSG_DSTPORT(msgid);
            unsigned int remoteport =
               VCHIQ_MSG_SRCPORT(msgid);
            vcos_assert(size == 0); /* There should be no data */

            vcos_log_info("%d: prs CLOSE@%x (%d->%d)",
               state->id, (unsigned int)header,
               remoteport, localport);

            if (vcos_verify(localport < VCHIQ_MAX_SERVICES)) {
               service = &state->services[localport];
               if ((service->remoteport != remoteport) &&
                  VCHIQ_PORT_IS_VALID(service->remoteport)) {
                  /* This could be from a client which hadn't yet received
                     the OPENACK - look for the connected service */
                  service = get_connected_service(state, remoteport);
                  if (!service)
                     break;
               }

               if (vchiq_close_service_internal(service,
                  1/*close_recvd*/) == VCHIQ_RETRY)
                  return;  /* Bail out if not ready */

               if (vcos_is_log_enabled( &vchiq_core_msg_log_category, VCOS_LOG_INFO))
               {
                  vcos_log_impl( &vchiq_core_msg_log_category,
                              VCOS_LOG_INFO,
                              "Close Service %c%c%c%c s:%u d:%d",
                              VCHIQ_FOURCC_AS_4CHARS(service->base.fourcc),
                              service->localport,
                              service->remoteport );
               }
               break;
            }
         }
         break;
      case VCHIQ_MSG_DATA:
         {
            unsigned int localport = VCHIQ_MSG_DSTPORT(msgid);
            unsigned int remoteport = VCHIQ_MSG_SRCPORT(msgid);

            vcos_log_trace("%d: prs DATA@%x,%x (%d->%d)",
               state->id, (unsigned int)header, size,
               remoteport, localport);

            service = &state->services[localport];
            if (vcos_verify
               (localport < VCHIQ_MAX_SERVICES)
               && (service->remoteport == remoteport)
               && (service->srvstate ==
               VCHIQ_SRVSTATE_OPEN)) {
               header->msgid = msgid | VCHIQ_MSGID_CLAIMED;
               claim_slot(state->rx_info);
               DEBUG_TRACE(PARSE_LINE);
               if (make_service_callback(service,
                  VCHIQ_MESSAGE_AVAILABLE, header,
                  NULL) == VCHIQ_RETRY)
               {
                  DEBUG_TRACE(PARSE_LINE);
                  return;  /* Bail out if not ready */
               }
               DEBUG_TRACE(PARSE_LINE);
            }
         }
         break;
      case VCHIQ_MSG_CONNECT:
         vcos_log_info("%d: prs CONNECT@%x",
            state->id, (unsigned int)header);
         vcos_event_signal(&state->connect);
         break;
      case VCHIQ_MSG_BULK_RX:
      case VCHIQ_MSG_BULK_TX:
         {
            unsigned int localport = VCHIQ_MSG_DSTPORT(msgid);
            unsigned int remoteport = VCHIQ_MSG_SRCPORT(msgid);
            VCHIQ_BULK_QUEUE_T *queue;
            vcos_assert(state->is_master);
            service = &state->services[localport];
            queue = (type == VCHIQ_MSG_BULK_RX) ?
               &service->bulk_tx : &service->bulk_rx;
            if (vcos_verify
               (localport < VCHIQ_MAX_SERVICES)
               && (service->remoteport == remoteport)
               && (service->srvstate ==
               VCHIQ_SRVSTATE_OPEN))
            {
               VCHIQ_BULK_T *bulk;
               int resolved;

               vcos_assert(queue->remote_insert < queue->remove +
                  VCHIQ_NUM_SERVICE_BULKS);
               bulk = &queue->bulks[BULK_INDEX(queue->remote_insert)];
               bulk->remote_data = (void *)((int *)header->data)[0];
               bulk->remote_size = ((int *)header->data)[1];

               vcos_log_info("%d: prs %s@%x (%d->%d) %x@%x",
                  state->id, msg_type_str(type),
                  (unsigned int)header,
                  remoteport, localport,
                  bulk->remote_size,
                  (unsigned int)bulk->remote_data);

               queue->remote_insert++;

               DEBUG_TRACE(PARSE_LINE);
               if (vcos_mutex_lock(&service->bulk_mutex) != VCOS_SUCCESS)
               {
                  DEBUG_TRACE(PARSE_LINE);
                  return;
               }
               DEBUG_TRACE(PARSE_LINE);
               resolved = resolve_bulks(service, queue);
               vcos_mutex_unlock(&service->bulk_mutex);
               if (resolved)
                  notify_bulks(service, queue);
            }
         }
         break;
      case VCHIQ_MSG_BULK_RX_DONE:
      case VCHIQ_MSG_BULK_TX_DONE:
         {
            unsigned int localport = VCHIQ_MSG_DSTPORT(msgid);
            unsigned int remoteport = VCHIQ_MSG_SRCPORT(msgid);
            vcos_assert(!state->is_master);
            service = &state->services[localport];
            if (vcos_verify
               (localport < VCHIQ_MAX_SERVICES)
               && (service->remoteport == remoteport)
               && (service->srvstate !=
               VCHIQ_SRVSTATE_FREE)) {
               VCHIQ_BULK_QUEUE_T *queue;
               VCHIQ_BULK_T *bulk;

               queue = (type == VCHIQ_MSG_BULK_RX_DONE) ?
                  &service->bulk_rx : &service->bulk_tx;

               bulk = &queue->bulks[BULK_INDEX(queue->process)];
               bulk->actual = *(int *)header->data;

               vcos_log_info("%d: prs %s@%x (%d->%d) %x",
                  state->id, msg_type_str(type),
                  (unsigned int)header,
                  remoteport, localport,
                  bulk->actual);

               vcos_log_trace("%d: prs:%d %cx li=%x ri=%x p=%x",
                  state->id, localport,
                  (type == VCHIQ_MSG_BULK_RX_DONE) ? 'r' : 't',
                  queue->local_insert,
                  queue->remote_insert, queue->process);

               DEBUG_TRACE(PARSE_LINE);
               if (vcos_mutex_lock(&service->bulk_mutex) != VCOS_SUCCESS)
               {
                  DEBUG_TRACE(PARSE_LINE);
                  return;
               }
               vcos_assert(queue->process != queue->local_insert);
               vchiq_complete_bulk(bulk);
               queue->process++;
               vcos_mutex_unlock(&service->bulk_mutex);
               DEBUG_TRACE(PARSE_LINE);
               notify_bulks(service, queue);
               DEBUG_TRACE(PARSE_LINE);
            }
         }
         break;
      case VCHIQ_MSG_PADDING:
         vcos_log_trace("%d: prs PADDING@%x,%x",
            state->id, (unsigned int)header, size);
         break;
      default:
         vcos_log_error("%d: prs invalid msgid %x@%x,%x",
            state->id, msgid, (unsigned int)header, size);
         vcos_assert(0);
         break;
      }

      state->rx_pos += calc_stride(size);

      DEBUG_TRACE(PARSE_LINE);
      /* Perform some housekeeping when the end of the slot is reached. */
      if ((state->rx_pos & VCHIQ_SLOT_MASK) == 0)
      {
         /* Remove the extra reference count. */
         release_slot(state, state->rx_info);
         state->rx_data = NULL;
      }
   }
}

/* Called by the slot handler thread */
static void *
slot_handler_func(void *v)
{
   VCHIQ_STATE_T *state = (VCHIQ_STATE_T *) v;
   VCHIQ_SHARED_STATE_T *local = state->local;
   DEBUG_INITIALISE(local)

   while (1) {
      DEBUG_COUNT(SLOT_HANDLER_COUNT);
      DEBUG_TRACE(SLOT_HANDLER_LINE);
      remote_event_wait(&local->trigger);

      DEBUG_TRACE(SLOT_HANDLER_LINE);
      poll_services(state);

      DEBUG_TRACE(SLOT_HANDLER_LINE);
      parse_rx_slots(state);
   }
   return NULL;
}

/* Called by the recycle thread */
static void *
recycle_func(void *v)
{
   VCHIQ_STATE_T *state = (VCHIQ_STATE_T *) v;
   VCHIQ_SHARED_STATE_T *local = state->local;

   while (1) {
      remote_event_wait(&local->recycle);

      process_free_queue(state);
   }
   return NULL;
}

static void
init_bulk_queue(VCHIQ_BULK_QUEUE_T *queue)
{
   queue->local_insert = 0;
   queue->remote_insert = 0;
   queue->process = 0;
   queue->remote_notify = 0;
   queue->remove = 0;
}

VCHIQ_SLOT_ZERO_T *
vchiq_init_slots(void *mem_base, int mem_size)
{
   int mem_align = (VCHIQ_SLOT_SIZE - (int)mem_base) & VCHIQ_SLOT_MASK;
   VCHIQ_SLOT_ZERO_T *slot_zero = (VCHIQ_SLOT_ZERO_T *)((char *)mem_base + mem_align);
   int num_slots = (mem_size - mem_align)/VCHIQ_SLOT_SIZE;
   int first_data_slot = VCHIQ_SLOT_ZERO_SLOTS;

   /* Ensure there is enough memory to run an absolutely minimum system */
   num_slots -= first_data_slot;

   if (num_slots < 4)
   {
      vcos_log_error("vchiq_init_slots - insufficient memory %x bytes", mem_size);
      return NULL;
   }

   memset(slot_zero, 0, sizeof(VCHIQ_SLOT_ZERO_T));

   slot_zero->magic = VCHIQ_MAGIC;
   slot_zero->version = VCHIQ_VERSION;
   slot_zero->slot_zero_size = sizeof(VCHIQ_SLOT_ZERO_T);
   slot_zero->slot_size = VCHIQ_SLOT_SIZE;
   slot_zero->max_slots = VCHIQ_MAX_SLOTS;
   slot_zero->max_slots_per_side = VCHIQ_MAX_SLOTS_PER_SIDE;

   slot_zero->master.slot_first = first_data_slot;
   slot_zero->slave.slot_first = first_data_slot + (num_slots/2);
   slot_zero->master.slot_last = slot_zero->slave.slot_first - 1;
   slot_zero->slave.slot_last = first_data_slot + num_slots - 1;

   return slot_zero;
}

VCHIQ_STATUS_T
vchiq_init_state(VCHIQ_STATE_T *state, VCHIQ_SLOT_ZERO_T *slot_zero, int is_master)
{
   VCHIQ_SHARED_STATE_T *local;
   VCHIQ_SHARED_STATE_T *remote;
   VCOS_THREAD_ATTR_T attrs;
   char threadname[10];
   static int id = 0;
   int i;

#if defined( __KERNEL__ )
   printk( "%s: slot_zero = 0x%08lx, is_master = %d\n", __func__, (unsigned long)slot_zero, is_master );
#endif

   vcos_log_register("vchiq_core", &vchiq_core_log_category);
   vcos_log_register("vchiq_core_msg", &vchiq_core_msg_log_category);
   vcos_log_set_level(&vchiq_core_log_category, vchiq_default_core_log_level);
   vcos_log_set_level(&vchiq_core_msg_log_category, vchiq_default_core_msg_log_level);

   /* Check the input configuration */

   if ((slot_zero->magic != VCHIQ_MAGIC) ||
      (slot_zero->version != VCHIQ_VERSION) ||
      (slot_zero->slot_zero_size != sizeof(VCHIQ_SLOT_ZERO_T)) ||
      (slot_zero->slot_size != VCHIQ_SLOT_SIZE) ||
      (slot_zero->max_slots != VCHIQ_MAX_SLOTS) ||
      (slot_zero->max_slots_per_side != VCHIQ_MAX_SLOTS_PER_SIDE))
   {
      vcos_log_error("slot_zero=%x: magic=%x, version=%x, slot_zero_size=%x,"
         " slot_size=%d, max_slots=%d, max_slots_per_side=%d",
         (unsigned int)slot_zero, slot_zero->magic, slot_zero->version,
         slot_zero->slot_zero_size, slot_zero->slot_size, slot_zero->max_slots,
         slot_zero->max_slots_per_side);
      return VCHIQ_ERROR;
   }

   memset(state, 0, sizeof(VCHIQ_STATE_T));
   state->id = id++;
   state->is_master = is_master;

   /*
      initialize shared state pointers
    */

   if (is_master)
   {
      local = &slot_zero->master;
      remote = &slot_zero->slave;
   }
   else
   {
      local = &slot_zero->slave;
      remote = &slot_zero->master;
   }

   state->local = local;
   state->remote = remote;
   state->slot_data = (VCHIQ_SLOT_T *)slot_zero;

   /*
      initialize events and mutexes
    */

   vcos_event_create(&state->connect, "v.connect");
   vcos_mutex_create(&state->mutex, "v.mutex");
   vcos_event_create(&state->trigger_event, "v.trigger_event");
   vcos_event_create(&state->recycle_event, "v.recycle_event");

   vcos_mutex_create(&state->slot_mutex, "v.slot_mutex");
   vcos_mutex_create(&state->recycle_mutex, "v.recycle_mutex");

   vcos_event_create(&state->slot_available_event, "v.slot_available_event");
   vcos_event_create(&state->slot_remove_event, "v.slot_remove_event");

   state->slot_queue_available = 0;

   for (i = local->slot_first; i <= local->slot_last; i++)
   {
      local->slot_queue[state->slot_queue_available++] = i;
   }

   state->default_slot_quota = state->slot_queue_available/2;

   local->trigger.event = &state->trigger_event;
   remote_event_create(&local->trigger);
   local->tx_pos = 0;

   local->recycle.event = &state->recycle_event;
   remote_event_create(&local->recycle);
   local->slot_queue_recycle = state->slot_queue_available;

   local->debug[DEBUG_ENTRIES] = DEBUG_MAX;

   /*
      initialize services
    */

   for (i = 0; i < VCHIQ_MAX_SERVICES; i++) {
      VCHIQ_SERVICE_T *service = &state->services[i];
      service->srvstate = VCHIQ_SRVSTATE_FREE;
      service->localport = i;
      service->slot_use_count = 0;
      vcos_event_create(&service->remove_event, "v.remove_event");
      vcos_event_create(&service->bulk_remove_event, "v.bulk_remove_event");
      vcos_event_create(&service->quota_event, "v.quota_event");
      vcos_mutex_create(&service->bulk_mutex, "v.bulk_mutex");
   }

   /*
      bring up slot handler thread
    */

   vcos_thread_attr_init(&attrs);
   vcos_thread_attr_setstacksize(&attrs, VCHIQ_SLOT_HANDLER_STACK);
   vcos_thread_attr_setpriority(&attrs, VCOS_THREAD_PRI_REALTIME);
   vcos_snprintf(threadname, sizeof(threadname), "VCHIQ-%d", state->id);
   vcos_thread_create(&state->slot_handler_thread, threadname,
            &attrs, slot_handler_func, state);

   vcos_thread_attr_init(&attrs);
   vcos_thread_attr_setstacksize(&attrs, VCHIQ_SLOT_HANDLER_STACK);
   vcos_thread_attr_setpriority(&attrs, VCOS_THREAD_PRI_REALTIME);
   vcos_snprintf(threadname, sizeof(threadname), "VCHIQr-%d", state->id);
   vcos_thread_create(&state->recycle_thread, threadname,
            &attrs, recycle_func, state);

   /* Indicate readiness to the other side */
   local->initialised = 1;

   return VCHIQ_SUCCESS;
}

/* Called from application thread when a client or server service is created. */
VCHIQ_SERVICE_T *
vchiq_add_service_internal(VCHIQ_STATE_T *state, int fourcc,
   VCHIQ_CALLBACK_T callback, void *userdata, int srvstate,
   VCHIQ_INSTANCE_T instance)
{
   VCHIQ_SERVICE_T *service = NULL;
   int i;

   /* Prepare to use a previously unused service */
   if (state->unused_service < VCHIQ_MAX_SERVICES)
      service = &state->services[state->unused_service];

   if (srvstate == VCHIQ_SRVSTATE_OPENING) {
      for (i = 0; i < state->unused_service; i++) {
         VCHIQ_SERVICE_T *srv = &state->services[i];
         if (srv->srvstate == VCHIQ_SRVSTATE_FREE) {
            service = srv;
            break;
         }
      }
   } else {
      for (i = (state->unused_service - 1); i >= 0; i--) {
         VCHIQ_SERVICE_T *srv = &state->services[i];
         if (srv->srvstate == VCHIQ_SRVSTATE_FREE) {
            service = srv;
         } else if ((srv->public_fourcc == fourcc) &&
            ((srv->instance != instance)
            || (srv->base.callback != callback))) {
            /* There is another server using this fourcc which doesn't match */
            service = NULL;
            break;
         }
      }
   }

   if (service) {
      if (vcos_is_log_enabled( &vchiq_core_msg_log_category, VCOS_LOG_INFO)) {
         vcos_log_impl( &vchiq_core_msg_log_category,
                     VCOS_LOG_INFO,
                     "%s Service %c%c%c%c SrcPort:%d",
                     ( srvstate == VCHIQ_SRVSTATE_OPENING )
                     ? "Open" : "Add",
                     VCHIQ_FOURCC_AS_4CHARS(fourcc),
                     service->localport );
      }
      service->state = state;
      service->base.fourcc = fourcc;
      service->base.callback = callback;
      service->base.userdata = userdata;
      vchiq_set_service_state(service, srvstate);
      service->public_fourcc =
         (srvstate ==
         VCHIQ_SRVSTATE_OPENING) ? VCHIQ_FOURCC_INVALID : fourcc;
      service->instance = instance;
      service->remoteport = VCHIQ_PORT_FREE;
      service->client_id = 0;
      service->auto_close = 1;
      init_bulk_queue(&service->bulk_tx);
      init_bulk_queue(&service->bulk_rx);
      service->slot_quota = state->default_slot_quota;
      if (service->slot_use_count == 0)
         service->previous_tx_index =
            SLOT_QUEUE_INDEX_FROM_POS(state->local_tx_pos) - 1;
      memset(&service->stats, 0, sizeof(service->stats));
      vcos_atomic_flags_create(&service->poll_flags);

      /* Ensure the events are unsignalled */
      while (vcos_event_try(&service->remove_event) == VCOS_SUCCESS)
         continue;
      while (vcos_event_try(&service->quota_event) == VCOS_SUCCESS)
         continue;

      if (service == &state->services[state->unused_service])
         state->unused_service++;
   }

   return service;
}

VCHIQ_STATUS_T
vchiq_open_service_internal(VCHIQ_SERVICE_T *service, int client_id)
{
   int payload[2] = { service->base.fourcc, client_id };
   VCHIQ_ELEMENT_T body = { payload, sizeof(payload) };
   VCHIQ_STATUS_T status = queue_message(service->state, NULL,
                     VCHIQ_MAKE_MSG(VCHIQ_MSG_OPEN, service->localport, 0),
                     &body, 1, sizeof(payload), 1);
   if (status == VCHIQ_SUCCESS) {
      if (vcos_event_wait(&service->remove_event) != VCOS_SUCCESS) {
         status = VCHIQ_RETRY;
      } else if (service->srvstate != VCHIQ_SRVSTATE_OPEN) {
         vcos_log_info("%d: osi - srvstate = %d", service->state->id, service->srvstate);
         vcos_assert(service->srvstate == VCHIQ_SRVSTATE_CLOSEWAIT);
         status = VCHIQ_ERROR;
      }
   }
   return status;
}

/* Called by the slot handler */
VCHIQ_STATUS_T
vchiq_close_service_internal(VCHIQ_SERVICE_T *service, int close_recvd)
{
   VCHIQ_STATE_T *state = service->state;
   VCHIQ_STATUS_T status = VCHIQ_SUCCESS;
   int i;

   vcos_log_trace("%d: csi:%d (%s)",
      service->state->id, service->localport,
      srvstate_names[service->srvstate]);

   switch (service->srvstate)
   {
   case VCHIQ_SRVSTATE_OPENING:
      if (close_recvd)
      {
         /* The open was rejected - tell the user */
         vchiq_set_service_state(service, VCHIQ_SRVSTATE_CLOSEWAIT);
         vcos_event_signal(&service->remove_event);
      }
      else
      {
         /* Shutdown mid-open - let the other side know */
         status = queue_message(state, NULL,
            VCHIQ_MAKE_MSG
            (VCHIQ_MSG_CLOSE,
            service->localport,
            VCHIQ_MSG_DSTPORT(service->remoteport)),
            NULL, 0, 0, 0);

         if (status == VCHIQ_SUCCESS)
            vchiq_set_service_state(service, VCHIQ_SRVSTATE_CLOSESENT);
      }
      break;

   case VCHIQ_SRVSTATE_OPEN:
      if (state->is_master)
      {
         /* Abort any outstanding bulk transfers */
         vcos_mutex_lock(&service->bulk_mutex);
         abort_outstanding_bulks(service, &service->bulk_tx);
         abort_outstanding_bulks(service, &service->bulk_rx);
         status = notify_bulks(service, &service->bulk_tx);
         if (status == VCHIQ_SUCCESS)
            status = notify_bulks(service, &service->bulk_rx);
         vcos_mutex_unlock(&service->bulk_mutex);
      }

      if (status == VCHIQ_SUCCESS)
         status = queue_message(state, NULL,
            VCHIQ_MAKE_MSG
            (VCHIQ_MSG_CLOSE,
            service->localport,
            VCHIQ_MSG_DSTPORT(service->remoteport)),
            NULL, 0, 0, 0);

      if (status == VCHIQ_SUCCESS)
      {
         if (close_recvd)
            vchiq_set_service_state(service, VCHIQ_SRVSTATE_CLOSING);
         else
            vchiq_set_service_state(service, VCHIQ_SRVSTATE_CLOSESENT);
      }
      break;

   case VCHIQ_SRVSTATE_CLOSESENT:
      vcos_assert(close_recvd);

      if (!state->is_master)
      {
         /* Abort any outstanding bulk transfers */
         vcos_mutex_lock(&service->bulk_mutex);
         abort_outstanding_bulks(service, &service->bulk_tx);
         abort_outstanding_bulks(service, &service->bulk_rx);
         status = notify_bulks(service, &service->bulk_tx);
         if (status == VCHIQ_SUCCESS)
            status = notify_bulks(service, &service->bulk_rx);
         vcos_mutex_unlock(&service->bulk_mutex);
      }

      if (status == VCHIQ_SUCCESS)
         vchiq_set_service_state(service, VCHIQ_SRVSTATE_CLOSING);
      break;

   case VCHIQ_SRVSTATE_CLOSING:
      /* We may come here after a retry */
      vcos_assert(!close_recvd);
      break;

   default:
      vcos_log_error("vchiq_close_service_internal(%d) called in state %s",
         close_recvd, srvstate_names[service->srvstate]);
      vcos_assert(0);
      break;
   }

   if (service->srvstate == VCHIQ_SRVSTATE_CLOSING)
   {
      /* Complete the close process */

      if (status != VCHIQ_RETRY)
      {
         int slot_last = state->remote->slot_last;

         /* Release any claimed messages */
         for (i = state->remote->slot_first; i <= slot_last; i++)
         {
            VCHIQ_SLOT_INFO_T *slot_info = SLOT_INFO_FROM_INDEX(state, i);
            if (slot_info->release_count != slot_info->use_count)
            {
               char *data = (char *)SLOT_DATA_FROM_INDEX(state, i);
               int pos, end;

               end = VCHIQ_SLOT_SIZE;
               if (data == state->rx_data)
               {
                  /* This buffer is still being read from - stop at the current read position */
                  end = state->rx_pos & VCHIQ_SLOT_MASK;
               }

               pos = 0;

               while (pos < end)
               {
                  VCHIQ_HEADER_T *header = (VCHIQ_HEADER_T *)(data + pos);
                  int msgid = header->msgid;
                  int port = VCHIQ_MSG_DSTPORT(msgid);
                  if (port == service->localport)
                  {
                     if (msgid & VCHIQ_MSGID_CLAIMED)
                        release_slot(state, slot_info);
                  }
                  pos += calc_stride(header->size);
               }
            }
         }
      }

      service->client_id = 0;

      /* Now tell the client that the services is closed */
      if (service->instance)
      {
         int oldstate = service->srvstate;

         /* Change the service state now for the benefit of the callback */
         vchiq_set_service_state(service,
            ((service->public_fourcc == VCHIQ_FOURCC_INVALID) ||
            !service->auto_close) ?
            VCHIQ_SRVSTATE_CLOSEWAIT :
            VCHIQ_SRVSTATE_LISTENING);

         status = make_service_callback(service, VCHIQ_SERVICE_CLOSED, NULL, NULL);

         if (status == VCHIQ_RETRY)
         {
            /* Restore the old state, to be retried later */
            vchiq_set_service_state(service, oldstate);
         }
         else
         {
            if (status == VCHIQ_ERROR) {
               /* Signal an error (fatal, since the other end will probably have closed) */
               vchiq_set_service_state(service, VCHIQ_SRVSTATE_OPEN);
            }
         }
      }

      if (status != VCHIQ_RETRY)
      {
         if (service->srvstate == VCHIQ_SRVSTATE_CLOSING)
            vchiq_set_service_state(service, VCHIQ_SRVSTATE_CLOSEWAIT);
         vcos_event_signal(&service->remove_event);
      }
   }

   return status;
}

/* Called from the application process upon process death */
void
vchiq_terminate_service_internal(VCHIQ_SERVICE_T *service)
{
   VCHIQ_STATE_T *state = service->state;

   vcos_log_info("%d: tsi - (%d<->%d)", state->id, service->localport, service->remoteport);

   /* Disconnect from the instance, to prevent any callbacks */
   service->instance = NULL;

   /* Mark the service for termination by the slot handler */
   request_poll(service, VCHIQ_POLL_TERMINATE);
}

/* Called from the application process upon process death */
void
vchiq_free_service_internal(VCHIQ_SERVICE_T *service)
{
   VCHIQ_STATE_T *state = service->state;

   vcos_log_info("%d: fsi - (%d)", state->id, service->localport);

   vchiq_set_service_state(service, VCHIQ_SRVSTATE_FREE);
}

VCHIQ_STATUS_T
vchiq_connect_internal(VCHIQ_STATE_T *state, VCHIQ_INSTANCE_T instance)
{
   int i;

   /* Find all services registered to this client and enable them. */
   for (i = 0; i < state->unused_service; i++)
      if (state->services[i].instance == instance) {
         if (state->services[i].srvstate ==
            VCHIQ_SRVSTATE_HIDDEN)
            vchiq_set_service_state(&state->services[i],
               VCHIQ_SRVSTATE_LISTENING);
      }

   if (!state->connected) {
      if (queue_message(state, NULL,
         VCHIQ_MAKE_MSG(VCHIQ_MSG_CONNECT, 0, 0), NULL, 0,
         0, 1) == VCHIQ_RETRY)
         return VCHIQ_RETRY;
      vcos_event_wait(&state->connect);
      state->connected = 1;
   }

   return VCHIQ_SUCCESS;
}

VCHIQ_STATUS_T
vchiq_shutdown_internal(VCHIQ_STATE_T *state, VCHIQ_INSTANCE_T instance)
{
   VCHIQ_STATUS_T status = VCHIQ_SUCCESS;
   int i;

   /* Find all services registered to this client and close them. */
   for (i = 0; i < state->unused_service; i++)
   {
      VCHIQ_SERVICE_T *service = &state->services[i];
      if ((service->instance == instance) &&
         ((service->srvstate == VCHIQ_SRVSTATE_OPEN) ||
         (service->srvstate == VCHIQ_SRVSTATE_LISTENING)))
      {
         status = vchiq_remove_service(&service->base);
         if (status != VCHIQ_SUCCESS)
            break;
      }
   }

   return status;
}

VCHIQ_STATUS_T
vchiq_close_service(VCHIQ_SERVICE_HANDLE_T handle)
{
   /* Unregister the service */
   VCHIQ_SERVICE_T *service = (VCHIQ_SERVICE_T *) handle;
   VCHIQ_STATUS_T status = VCHIQ_ERROR;

   if (service == NULL)
      return VCHIQ_ERROR;

   vcos_log_info("%d: close_service:%d", service->state->id, service->localport);

   if (service->public_fourcc != VCHIQ_FOURCC_INVALID)
   {
      if (service->srvstate == VCHIQ_SRVSTATE_CLOSEWAIT)
      {
         vchiq_set_service_state(service, VCHIQ_SRVSTATE_LISTENING);
         status = VCHIQ_SUCCESS;
      }
   }
   else
   {
      /* For clients, make it an alias of vchiq_remove_service */
      status = vchiq_remove_service(handle);
   }

   return status;
}

VCHIQ_STATUS_T
vchiq_remove_service(VCHIQ_SERVICE_HANDLE_T handle)
{
   /* Unregister the service */
   VCHIQ_SERVICE_T *service = (VCHIQ_SERVICE_T *) handle;
   VCHIQ_STATUS_T status = VCHIQ_SUCCESS;

   if (service == NULL)
      return VCHIQ_ERROR;

   vcos_log_info("%d: remove_service:%d", service->state->id, service->localport);

   switch (service->srvstate)
   {
   case VCHIQ_SRVSTATE_OPENING:
   case VCHIQ_SRVSTATE_OPEN:
      /* Mark the service for termination by the slot handler */
      request_poll(service, VCHIQ_POLL_TERMINATE);

      /* Drop through... */
   case VCHIQ_SRVSTATE_CLOSESENT:
   case VCHIQ_SRVSTATE_CLOSING:
      while ((service->srvstate != VCHIQ_SRVSTATE_CLOSEWAIT) &&
         (service->srvstate != VCHIQ_SRVSTATE_LISTENING))
      {
         if (vcos_event_wait(&service->remove_event) != VCOS_SUCCESS) {
            status = VCHIQ_RETRY;
            break;
         }
      }
      break;

   default:
      break;
   }

   if (status == VCHIQ_SUCCESS) {
      if (service->srvstate == VCHIQ_SRVSTATE_OPEN)
         status = VCHIQ_ERROR;
      else
      {
         service->instance = NULL;
         vchiq_set_service_state(service, VCHIQ_SRVSTATE_FREE);
      }
   }

   return status;
}


VCHIQ_STATUS_T
vchiq_bulk_transfer(VCHIQ_SERVICE_T *service,
   VCHI_MEM_HANDLE_T memhandle, void *offset, int size, void *userdata,
   VCHIQ_BULK_MODE_T mode, VCHIQ_BULK_DIR_T dir)
{
   VCHIQ_BULK_QUEUE_T *queue = (dir == VCHIQ_BULK_TRANSMIT) ?
      &service->bulk_tx : &service->bulk_rx;
   VCHIQ_BULK_T *bulk;
   VCHIQ_STATE_T *state;
   BULK_WAITER_T bulk_waiter;
   const char dir_char = (dir == VCHIQ_BULK_TRANSMIT) ? 't' : 'r';
   const int dir_msgtype = (dir == VCHIQ_BULK_TRANSMIT) ? VCHIQ_MSG_BULK_TX : VCHIQ_MSG_BULK_RX;
   VCHIQ_STATUS_T status = VCHIQ_ERROR;

   if ((service == NULL) ||
       ((memhandle == VCHI_MEM_HANDLE_INVALID) && (offset == NULL)))
      return VCHIQ_ERROR;

   state = service->state;

   if (service->srvstate != VCHIQ_SRVSTATE_OPEN)
      return VCHIQ_ERROR;  /* Must be connected */

   if (vcos_mutex_lock(&service->bulk_mutex) != VCOS_SUCCESS)
      return VCHIQ_RETRY;

   if (queue->local_insert == queue->remove + VCHIQ_NUM_SERVICE_BULKS)
   {
      VCHIQ_SERVICE_STATS_INC(service, bulk_stalls);
      do {
         vcos_mutex_unlock(&service->bulk_mutex);
         if (vcos_event_wait(&service->bulk_remove_event) != VCOS_SUCCESS)
            return VCHIQ_RETRY;
         if (vcos_mutex_lock(&service->bulk_mutex) != VCOS_SUCCESS)
            return VCHIQ_RETRY;
      } while (queue->local_insert == queue->remove + VCHIQ_NUM_SERVICE_BULKS);
   }

   bulk = &queue->bulks[BULK_INDEX(queue->local_insert)];

   if (mode == VCHIQ_BULK_MODE_BLOCKING)
   {
      vcos_event_create(&bulk_waiter.event, "bulk_waiter");
      bulk_waiter.actual = 0;
      userdata = &bulk_waiter;
   }

   bulk->mode = mode;
   bulk->dir = dir;
   bulk->userdata = userdata;
   bulk->size = size;
   bulk->actual = VCHIQ_BULK_ACTUAL_ABORTED;

   if (vchiq_prepare_bulk_data(bulk, memhandle, offset, size, dir) != VCHIQ_SUCCESS)
   {
      if (mode == VCHIQ_BULK_MODE_BLOCKING)
         vcos_event_delete(&bulk_waiter.event);
      goto error_exit;
   }

   vcos_log_info("%d: bt (%d->%d) %cx %x@%x %x", state->id,
      service->localport, service->remoteport, dir_char,
      size, (unsigned int)bulk->data, (unsigned int)userdata);

   if (state->is_master)
   {
      queue->local_insert++;
      if (resolve_bulks(service, queue))
         request_poll(service, (dir == VCHIQ_BULK_TRANSMIT) ?
            VCHIQ_POLL_TXNOTIFY : VCHIQ_POLL_RXNOTIFY);
   }
   else
   {
      int payload[2] = { (int)bulk->data, bulk->size };
      VCHIQ_ELEMENT_T element = { payload, sizeof(payload) };

      if (queue_message(state, NULL,
         VCHIQ_MAKE_MSG(dir_msgtype,
            service->localport, service->remoteport),
         &element, 1, sizeof(payload), 1) != VCHIQ_SUCCESS)
      {
         vchiq_complete_bulk(bulk);
         if (mode == VCHIQ_BULK_MODE_BLOCKING)
            vcos_event_delete(&bulk_waiter.event);
         goto error_exit;
      }
      queue->local_insert++;
      queue->remote_insert++;
   }

   vcos_mutex_unlock(&service->bulk_mutex);
 
   vcos_log_trace("%d: bt:%d %cx li=%x ri=%x p=%x", state->id,
      service->localport, dir_char,
      queue->local_insert, queue->remote_insert, queue->process);

   status = VCHIQ_SUCCESS;

   if (mode == VCHIQ_BULK_MODE_BLOCKING)
   {
      if (vcos_event_wait(&bulk_waiter.event) != VCOS_SUCCESS)
      {
         vcos_log_info("bulk wait interrupted");
         /* Stop notify_bulks signalling a non-existent waiter */
         bulk->userdata = NULL;
         status = VCHIQ_ERROR;
      }
      else if (bulk_waiter.actual == VCHIQ_BULK_ACTUAL_ABORTED)
         status = VCHIQ_ERROR;

      vcos_event_delete(&bulk_waiter.event);
   }

error_exit:
   return status;
}

VCHIQ_STATUS_T
vchiq_queue_bulk_transmit(VCHIQ_SERVICE_HANDLE_T handle,
   const void *data, int size, void *userdata)
{
   return vchiq_bulk_transfer((VCHIQ_SERVICE_T *)handle,
      VCHI_MEM_HANDLE_INVALID, (void *)data, size, userdata,
      VCHIQ_BULK_MODE_CALLBACK, VCHIQ_BULK_TRANSMIT);
}

VCHIQ_STATUS_T
vchiq_queue_bulk_receive(VCHIQ_SERVICE_HANDLE_T handle, void *data, int size,
   void *userdata)
{
   return vchiq_bulk_transfer((VCHIQ_SERVICE_T *)handle,
      VCHI_MEM_HANDLE_INVALID, data, size, userdata,
      VCHIQ_BULK_MODE_CALLBACK, VCHIQ_BULK_RECEIVE);
}

VCHIQ_STATUS_T
vchiq_queue_bulk_transmit_handle(VCHIQ_SERVICE_HANDLE_T handle,
   VCHI_MEM_HANDLE_T memhandle, const void *offset, int size, void *userdata)
{
   return vchiq_bulk_transfer((VCHIQ_SERVICE_T *)handle,
      memhandle, (void *)offset, size, userdata,
      VCHIQ_BULK_MODE_CALLBACK, VCHIQ_BULK_TRANSMIT);
}

VCHIQ_STATUS_T
vchiq_queue_bulk_receive_handle(VCHIQ_SERVICE_HANDLE_T handle,
   VCHI_MEM_HANDLE_T memhandle, void *offset, int size, void *userdata)
{
   return vchiq_bulk_transfer((VCHIQ_SERVICE_T *)handle,
      memhandle, offset, size, userdata,
      VCHIQ_BULK_MODE_CALLBACK, VCHIQ_BULK_RECEIVE);
}

VCHIQ_STATUS_T
vchiq_bulk_transmit(VCHIQ_SERVICE_HANDLE_T handle, const void *data, int size,
   void *userdata, VCHIQ_BULK_MODE_T mode)
{
   return vchiq_bulk_transfer((VCHIQ_SERVICE_T *)handle,
      VCHI_MEM_HANDLE_INVALID, (void *)data, size, userdata,
      mode, VCHIQ_BULK_TRANSMIT);
}

VCHIQ_STATUS_T
vchiq_bulk_receive(VCHIQ_SERVICE_HANDLE_T handle, void *data, int size,
   void *userdata, VCHIQ_BULK_MODE_T mode)
{
   return vchiq_bulk_transfer((VCHIQ_SERVICE_T *)handle,
      VCHI_MEM_HANDLE_INVALID, data, size, userdata,
      mode, VCHIQ_BULK_RECEIVE);
}

VCHIQ_STATUS_T
vchiq_bulk_transmit_handle(VCHIQ_SERVICE_HANDLE_T handle,
   VCHI_MEM_HANDLE_T memhandle, const void *offset, int size, void *userdata,
   VCHIQ_BULK_MODE_T mode)
{
   return vchiq_bulk_transfer((VCHIQ_SERVICE_T *)handle,
      memhandle, (void *)offset, size, userdata,
      mode, VCHIQ_BULK_TRANSMIT);
}

VCHIQ_STATUS_T
vchiq_bulk_receive_handle(VCHIQ_SERVICE_HANDLE_T handle,
   VCHI_MEM_HANDLE_T memhandle, void *offset, int size, void *userdata,
   VCHIQ_BULK_MODE_T mode)
{
   return vchiq_bulk_transfer((VCHIQ_SERVICE_T *)handle,
      memhandle, offset, size, userdata,
      mode, VCHIQ_BULK_RECEIVE);
}

VCHIQ_STATUS_T
vchiq_queue_message(VCHIQ_SERVICE_HANDLE_T handle,
   const VCHIQ_ELEMENT_T *elements, int count)
{
   VCHIQ_SERVICE_T *service = (VCHIQ_SERVICE_T *) handle;

   unsigned int size = 0;
   unsigned int i;

   if ((service == NULL) ||
      (service->srvstate != VCHIQ_SRVSTATE_OPEN))
      return VCHIQ_ERROR;

   for (i = 0; i < (unsigned int)count; i++)
   {
      if (elements[i].size)
      {
         if (elements[i].data == NULL)
            return VCHIQ_ERROR;
         size += elements[i].size;
      }
   }

   if (size > VCHIQ_MAX_MSG_SIZE)
      return VCHIQ_ERROR;

   return queue_message(service->state, service,
            VCHIQ_MAKE_MSG(VCHIQ_MSG_DATA, service->localport,
               service->remoteport), elements, count, size, 1);
}

void
vchiq_release_message(VCHIQ_SERVICE_HANDLE_T handle, VCHIQ_HEADER_T *header)
{
   VCHIQ_SERVICE_T *service = (VCHIQ_SERVICE_T *)handle;
   VCHIQ_STATE_T *state;
   int slot_index;
   int msgid;

   if (service == NULL)
      return;

   state = service->state;

   slot_index = SLOT_INDEX_FROM_DATA(state, (void *)header);

   if ((slot_index >= state->remote->slot_first) &&
      (slot_index <= state->remote->slot_last) &&
      ((msgid = header->msgid) & VCHIQ_MSGID_CLAIMED))
   {
      VCHIQ_SLOT_INFO_T *slot_info = SLOT_INFO_FROM_INDEX(state, slot_index);

      /* Rewrite the message header to prevent a double release */
      header->msgid = msgid & ~VCHIQ_MSGID_CLAIMED;

      release_slot(state, slot_info);
   }
}

int
vchiq_get_client_id(VCHIQ_SERVICE_HANDLE_T handle)
{
   VCHIQ_SERVICE_T *service = (VCHIQ_SERVICE_T *)handle;
   return service ? service->client_id : 0;
}

VCHIQ_STATUS_T
vchiq_get_config(VCHIQ_INSTANCE_T instance,
   int config_size, VCHIQ_CONFIG_T *pconfig)
{
   VCHIQ_CONFIG_T config;

   vcos_unused(instance);

   config.max_msg_size = VCHIQ_MAX_MSG_SIZE;
   config.bulk_threshold = VCHIQ_MAX_MSG_SIZE;
   config.max_outstanding_bulks = VCHIQ_NUM_SERVICE_BULKS;
   config.max_services = VCHIQ_MAX_SERVICES;

   if (config_size > sizeof(VCHIQ_CONFIG_T))
      return VCHIQ_ERROR;

   memcpy(pconfig, &config, vcos_min(config_size, sizeof(VCHIQ_CONFIG_T)));

   return VCHIQ_SUCCESS;
}

VCHIQ_STATUS_T
vchiq_set_service_option(VCHIQ_SERVICE_HANDLE_T handle,
   VCHIQ_SERVICE_OPTION_T option, int value)
{
   VCHIQ_SERVICE_T *service = (VCHIQ_SERVICE_T *)handle;
   VCHIQ_STATUS_T status = VCHIQ_ERROR;

   if (service)
   {
      switch (option)
      {
      case VCHIQ_SERVICE_OPTION_AUTOCLOSE:
         service->auto_close = value;
         status = VCHIQ_SUCCESS;
         break;

      default:
         break;
      }
   }

   return status;
}

void
vchiq_dump_shared_state(void *dump_context, VCHIQ_STATE_T *state,
   VCHIQ_SHARED_STATE_T *shared, const char *label)
{
   static const char *const debug_names[] =
   {
      "<entries>",
      "SLOT_HANDLER_COUNT",
      "SLOT_HANDLER_LINE",
      "PARSE_LINE",
      "PARSE_HEADER",
      "PARSE_MSGID",
      "AWAIT_COMPLETION_LINE",
      "DEQUEUE_MESSAGE_LINE",
      "SERVICE_CALLBACK_LINE",
      "MSG_QUEUE_FULL_COUNT",
      "COMPLETION_QUEUE_FULL_COUNT"
   };
   int i;

   char buf[80];
   int len;
   len = vcos_snprintf(buf, sizeof(buf),
      "  %s: slots %d-%d tx_pos=%x recycle=%x",
      label, shared->slot_first, shared->slot_last,
      shared->tx_pos, shared->slot_queue_recycle);
   vchiq_dump(dump_context, buf, len + 1);

   len = vcos_snprintf(buf, sizeof(buf),
      "    Slots claimed:"); 
   vchiq_dump(dump_context, buf, len + 1);

   for (i = shared->slot_first; i <= shared->slot_last; i++)
   {
      VCHIQ_SLOT_INFO_T slot_info = *SLOT_INFO_FROM_INDEX(state, i);
      if (slot_info.use_count != slot_info.release_count)
      {
         len = vcos_snprintf(buf, sizeof(buf),
            "      %d: %d/%d", i, slot_info.use_count, slot_info.release_count);
         vchiq_dump(dump_context, buf, len + 1);
      }
   }

   for (i = 1; i < shared->debug[DEBUG_ENTRIES]; i++)
   {
      len = vcos_snprintf(buf, sizeof(buf), "    DEBUG: %s = %d(%x)",
         debug_names[i], shared->debug[i], shared->debug[i]);
      vchiq_dump(dump_context, buf, len + 1);
   }
}

void
vchiq_dump_state(void *dump_context, VCHIQ_STATE_T *state)
{
   char buf[80];
   int len;
   int i;

   len = vcos_snprintf(buf, sizeof(buf),
      "State %d: tx_pos=%x(@%x), rx_pos=%x(@%x)",
      state->id, state->local->tx_pos,
      (uint32_t)state->tx_data + (state->local_tx_pos & VCHIQ_SLOT_MASK),
      state->rx_pos,
      (uint32_t)state->rx_data + (state->rx_pos & VCHIQ_SLOT_MASK));
   vchiq_dump(dump_context, buf, len + 1);

   len = vcos_snprintf(buf, sizeof(buf),
      "  Slots: %d available, %d recyclable, %d stalls",
      state->slot_queue_available - SLOT_QUEUE_INDEX_FROM_POS(state->local_tx_pos),
      state->local->slot_queue_recycle - state->slot_queue_available,
      state->stats.slot_stalls);
   vchiq_dump(dump_context, buf, len + 1);

   vchiq_dump_platform_state(dump_context);

   vchiq_dump_shared_state(dump_context, state, state->local, "Local");
   vchiq_dump_shared_state(dump_context, state, state->remote, "Remote");

   vchiq_dump_platform_instances(dump_context);

   for (i = 0; i < state->unused_service; i++) {
      VCHIQ_SERVICE_T *service = &state->services[i];

      if (service->srvstate != VCHIQ_SRVSTATE_FREE)
         vchiq_dump_service_state(dump_context, service);
   }
}

void
vchiq_dump_service_state(void *dump_context, VCHIQ_SERVICE_T *service)
{
   char buf[80];
   int len;

   len = vcos_snprintf(buf, sizeof(buf), "Service %d: %s",
      service->localport, srvstate_names[service->srvstate]);

   if (service->srvstate != VCHIQ_SRVSTATE_FREE)
   {
      char remoteport[30];
      int fourcc = service->base.fourcc;
      if (service->remoteport != VCHIQ_PORT_FREE)
      {
         int len2 = vcos_snprintf(remoteport, sizeof(remoteport), "%d",
            service->remoteport);
         if (service->public_fourcc != VCHIQ_FOURCC_INVALID)
            vcos_snprintf(remoteport + len2, sizeof(remoteport) - len2,
               " (client %x)", service->client_id);
      }
      else
         vcos_strcpy(remoteport, "n/a");

      len += vcos_snprintf(buf + len, sizeof(buf) - len,
         " '%c%c%c%c' remote %s (slot use %d/%d)",
         VCHIQ_FOURCC_AS_4CHARS(fourcc),
         remoteport,
         service->slot_use_count, service->slot_quota);
      vchiq_dump(dump_context, buf, len + 1);

      len = vcos_snprintf(buf, sizeof(buf),
         "  %d quota stalls, %d slot stalls, %d bulk stalls",
         service->stats.quota_stalls, service->stats.slot_stalls,
         service->stats.bulk_stalls);
   }

   vchiq_dump(dump_context, buf, len + 1);

   vchiq_dump_platform_service_state(dump_context, service);
}
