/*
 *  linux/arch/arm/mach-bcm2708/vchiq_core.c
 *
 *  Copyright (C) 2010 Broadcom
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

#if defined( __KERNEL__ )
#include <linux/kernel.h>
#include <linux/module.h>
#endif

#include "vchiq_core.h"

#define VCHIQ_SLOT_HANDLER_STACK 8192

/* FIXME: this should not be needed */
#ifdef _VIDEOCORE
#include "vcfw/rtos/common/rtos_common_mem.h"
#endif

VCOS_LOG_CAT_T vchiq_core_log_category;
VCOS_LOG_CAT_T vchiq_core_msg_log_category;

#define  VCOS_LOG_CATEGORY (&vchiq_core_log_category)

static const char *msg_type_str( unsigned int msg_type )
{
	switch (msg_type) {
	case VCHIQ_MSG_INVALID:		return "INVALID";
	case VCHIQ_MSG_OPEN:		return "OPEN";
	case VCHIQ_MSG_OPENACK:		return "OPENACK";
	case VCHIQ_MSG_CLOSE:		return "CLOSE";
	case VCHIQ_MSG_DATA:		return "DATA";
	case VCHIQ_MSG_CONNECT:		return "CONNECT";
	}
	return "???";
}

static inline void vchiq_set_service_state(VCHIQ_SERVICE_T *service, int newstate)
{
	vcos_log_trace("  srv %d: %d->%d", service->localport, service->srvstate, newstate);
	service->srvstate = newstate;
}

static inline void remote_event_create(REMOTE_EVENT_T *event)
{
#if 0
	event->armed = 0;
	event->fired = 0;
#endif
	vcos_event_create(event->event, "vchiq");
}

static inline void remote_event_destroy(REMOTE_EVENT_T *event)
{
	vcos_event_delete(event->event);
}

static inline void remote_event_signal(REMOTE_EVENT_T *event)
{
#ifdef VCHIQ_LOCAL
	vcos_event_signal(event->event);
#else
	event->fired = 1;
	if (event->armed) { /* Also ensures the write has completed */
		/* Notify the other side */
		vchiq_ring_doorbell();
	}
#endif
}

static inline int remote_event_wait(REMOTE_EVENT_T *event)
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

static inline void remote_event_signal_local(REMOTE_EVENT_T *event)
{
	event->armed = 0;
	vcos_event_signal(event->event);
}

#ifndef VCHIQ_LOCAL

static inline void remote_event_poll(REMOTE_EVENT_T *event)
{
	if (event->armed)
		remote_event_signal_local(event);
}

void remote_event_pollall(VCHIQ_STATE_T *state)
{
	remote_event_poll(&state->local->trigger);
	remote_event_poll(&state->local->ctrl.remove_event);
}

#endif

static inline uint32_t calc_stride(uint32_t size)
{
	return (size + (uint32_t)sizeof(VCHIQ_HEADER_T) + 7) & ~7;
}

static VCHIQ_SERVICE_T *get_listening_service(VCHIQ_CHANNEL_T *channel,
					      int fourcc)
{
	int i;

	vcos_assert(fourcc != VCHIQ_FOURCC_INVALID);

	for (i = 0; i < VCHIQ_MAX_SERVICES; i++)
		if ((channel->services[i].fourcc == fourcc)
		    && (channel->services[i].srvstate ==
			VCHIQ_SRVSTATE_LISTENING))
			return &channel->services[i];

	return NULL;
}

static VCHIQ_SERVICE_T *get_connected_service(VCHIQ_CHANNEL_T *channel,
					      unsigned int port)
{
	int i;
	for (i = 0; i < VCHIQ_MAX_SERVICES; i++) {
		VCHIQ_SERVICE_T *service = &channel->services[i];
		if ((service->srvstate == VCHIQ_SRVSTATE_OPEN)
		    && (service->remoteport == port)) {
			return service;
		}
	}
	return NULL;
}

static void *reserve_space(VCHIQ_CHANNEL_T *local, int target)
{
	if (target - local->ctrl.remove > VCHIQ_CHANNEL_SIZE) {
		while (1) {
			while (local->ctrl.remove != local->ctrl.process) {
				VCHIQ_HEADER_T *header =
				    (VCHIQ_HEADER_T *) (local->ctrl.data +
							(local->ctrl.
							 remove &
							 VCHIQ_CHANNEL_MASK));

				if (header->fourcc == VCHIQ_FOURCC_INVALID)
					local->ctrl.remove +=
					    calc_stride(header->size);
				else
					break;
			}

			if (target - local->ctrl.remove <= VCHIQ_CHANNEL_SIZE)
				break;

			if (!remote_event_wait(&local->ctrl.remove_event))
				return NULL;	/* Not now */
		}
	}
	return (void *)(local->ctrl.data +
			(local->ctrl.insert & VCHIQ_CHANNEL_MASK));
}

static VCHIQ_STATUS_T queue_message(VCHIQ_STATE_T *state, int fourcc,
				    const VCHIQ_ELEMENT_T *elements, uint32_t count,
				    uint32_t size)
{
	VCHIQ_CHANNEL_T *local;
	VCHIQ_HEADER_T *header;

	uint32_t stride;
	uint32_t pos;

	vcos_log_trace("%d: qm %d", state->id, VCHIQ_MSG_TYPE(fourcc));

	local = state->local;

	stride = calc_stride(size);

	vcos_assert(stride < VCHIQ_CHANNEL_SIZE);

	vcos_mutex_lock(local->ctrl.mutex);

	pos = local->ctrl.insert;	/* Remember for rewind */
	if ((local->ctrl.insert & VCHIQ_CHANNEL_MASK) + stride >
	    VCHIQ_CHANNEL_SIZE) {
		int target =
		    (local->ctrl.insert +
		     VCHIQ_CHANNEL_SIZE) & ~VCHIQ_CHANNEL_MASK;

		header = (VCHIQ_HEADER_T *) reserve_space(local, target);
		if (!header) {
			vcos_mutex_unlock(local->ctrl.mutex);
			return VCHIQ_RETRY;
		}
		header->fourcc = VCHIQ_FOURCC_INVALID;
		header->size = VCHIQ_CHANNEL_SIZE - sizeof(VCHIQ_HEADER_T) -
		    (local->ctrl.insert & VCHIQ_CHANNEL_MASK);

		local->ctrl.insert = target;
	}

	/*
	   write into slot
	 */

	header =
	    (VCHIQ_HEADER_T *) reserve_space(local,
					     local->ctrl.insert + stride);
	if (!header) {
		local->ctrl.insert = pos;
		vcos_mutex_unlock(local->ctrl.mutex);
		return VCHIQ_RETRY;
	}

	header->fourcc = fourcc;
	header->size = size;

	vcos_log_info("%d: qm %s(%u) s:%u d:%u - header %p",
				  state->id,
				  msg_type_str(VCHIQ_MSG_TYPE(fourcc)),
				  VCHIQ_MSG_TYPE(fourcc),
				  VCHIQ_MSG_SRCPORT(fourcc),
				  VCHIQ_MSG_DSTPORT(fourcc),
				  header);

	if (vcos_is_log_enabled( &vchiq_core_msg_log_category, VCOS_LOG_INFO))
	{
		int	svc_fourcc;

		svc_fourcc = state->local->services[VCHIQ_MSG_SRCPORT(fourcc)].base.fourcc;

		vcos_log_impl( &vchiq_core_msg_log_category,
					   VCOS_LOG_INFO,
					   "Sent Msg %s(%u) to %c%c%c%c s:%u d:%d len:%d",
					   msg_type_str(VCHIQ_MSG_TYPE(fourcc)),
					   VCHIQ_MSG_TYPE(fourcc),
					   VCHIQ_FOURCC_AS_4CHARS(svc_fourcc),
					   VCHIQ_MSG_SRCPORT(fourcc),
					   VCHIQ_MSG_DSTPORT(fourcc),
					   header->size );
	}

	if (VCHIQ_MSG_TYPE(fourcc) == VCHIQ_MSG_DATA) {
		int i;
		for (i = 0, pos = 0; i < (unsigned int)count;
		     pos += elements[i++].size)
			if (elements[i].size) {
				if (vchiq_copy_from_user
				    (header->data + pos, elements[i].data,
				     (size_t) elements[i].size) !=
				    VCHIQ_SUCCESS) {
					header->fourcc = VCHIQ_FOURCC_INVALID;
					vcos_mutex_unlock(local->ctrl.mutex);
					return VCHIQ_ERROR;
				}
				if (i == 0) {
					vcos_log_dump_mem( &vchiq_core_msg_log_category,
									   "Sent", 0, header->data + pos,
									   vcos_min( 64, elements[0].size ));
				}
			}
	} else if (size != 0) {
		vcos_assert((count == 1) && (size == elements[0].size));
		memcpy(header->data, elements[0].data, elements[0].size);
	}

	local->ctrl.insert += stride;

	vcos_mutex_unlock(local->ctrl.mutex);

	remote_event_signal(&state->remote->trigger);

	return VCHIQ_SUCCESS;
}

static void notify_rx_bulks(VCHIQ_STATE_T *state)
{
	VCHIQ_CHANNEL_T *local = state->local;

	vcos_log_trace("%d: nrb", state->id);
	while (local->bulk.remove != local->bulk.process) {
		VCHIQ_BULK_T *bulk = &local->bulk.bulks[local->bulk.remove & (VCHIQ_NUM_CURRENT_BULKS - 1)];

		vcos_log_trace("%d: nrb %d %x", state->id, bulk->dstport, local->bulk.remove);

		if (bulk->dstport < VCHIQ_MAX_SERVICES) {
			VCHIQ_SERVICE_T *service =
			    &local->services[bulk->dstport];

			if (service->base.
			    callback(bulk->
				     data ? VCHIQ_BULK_RECEIVE_DONE :
				     VCHIQ_BULK_RECEIVE_ABORTED, NULL,
				     &service->base,
				     bulk->userdata) == VCHIQ_RETRY)
				break;	/* Bail out if not ready */

			vchiq_complete_bulk(bulk);
		}

		local->bulk.remove++;

		vcos_event_signal(local->bulk.remove_event);
	}
}

static void notify_tx_bulks(VCHIQ_STATE_T *state)
{
	VCHIQ_CHANNEL_T *local = state->local;
	int i;

	vcos_log_trace("%d: ntb", state->id);
	for (i = 0; i < VCHIQ_MAX_SERVICES; i++) {
		VCHIQ_SERVICE_T *service = &local->services[i];

		if (service->srvstate > VCHIQ_SRVSTATE_LISTENING)
			vcos_log_trace("%d: ntb - srv %d state %d", state->id, service->localport, service->srvstate);

		if (service->terminate) {
			if ((service->srvstate == VCHIQ_SRVSTATE_OPENING) ||
			    (service->srvstate == VCHIQ_SRVSTATE_OPEN)) {
				if (queue_message(state,
						  VCHIQ_MAKE_MSG
						  (VCHIQ_MSG_CLOSE,
						   service->localport,
						   VCHIQ_MSG_DSTPORT(service->remoteport)),
						  NULL, 0, 0) == VCHIQ_RETRY)
					continue;
			}

			service->terminate = 0;
			vchiq_set_service_state(service, VCHIQ_SRVSTATE_CLOSESENT);
		} else if (service->srvstate == VCHIQ_SRVSTATE_OPEN) {
			while (service->remove != service->process) {
				VCHIQ_BULK_T *bulk =
				    &service->bulks[service->
						    remove &
						    (VCHIQ_NUM_SERVICE_BULKS -
						     1)];

				vcos_log_trace("%d: ntb %x DONE", state->id, service->remove);

				if (service->base.
				    callback(bulk->
					     data ? VCHIQ_BULK_TRANSMIT_DONE :
					     VCHIQ_BULK_TRANSMIT_ABORTED, NULL,
					     &service->base,
					     bulk->userdata) == VCHIQ_RETRY)
					break;	/* Bail out if not ready */

				vchiq_complete_bulk(bulk);

				service->remove++;

				vcos_event_signal(service->remove_event);
			}
		}
	}
}

#if defined(VCHIQ_HANDLE_BULK_TX)

static void resolve_tx_bulks(VCHIQ_STATE_T *state)
{
	VCHIQ_CHANNEL_T *local = state->local;
	VCHIQ_CHANNEL_T *remote = state->remote;
	int retrigger = 0;

	vcos_log_trace("%d: rtb %x %x", state->id, remote->bulk.process, remote->bulk.insert);
	while (remote->bulk.process != remote->bulk.insert) {
		VCHIQ_BULK_T *rx_bulk = &remote->bulk.bulks[remote->bulk.process & (VCHIQ_NUM_CURRENT_BULKS - 1)];
		VCHIQ_SERVICE_T *service;
		VCHIQ_BULK_T *tx_bulk;

		if (VCHIQ_PORT_IS_VALID(rx_bulk->dstport)) {
			/* Find connected service */
			service =
			    get_connected_service(local, rx_bulk->dstport);

			if (service == NULL) {
				rx_bulk->data = NULL; /* Abort */
				vcos_log_trace("%d: rtb %x(%d) ABORTED", state->id, remote->bulk.process, rx_bulk->dstport);
			} else {
				if (service->process == service->insert)
					break; /* No matching transmit Stall */

				tx_bulk =
				    &service->bulks[service->
						    process &
						    (VCHIQ_NUM_SERVICE_BULKS -
						     1)];

				if (rx_bulk->size == tx_bulk->size) {
					const void *tx_data;

					if (vcos_is_log_enabled( &vchiq_core_msg_log_category, VCOS_LOG_INFO))
					{
						vcos_log_impl( &vchiq_core_msg_log_category,
									   VCOS_LOG_INFO,
									   "Send Bulk to %c%c%c%c d:%d len:%d",
									   VCHIQ_FOURCC_AS_4CHARS(service->base.fourcc),
									   rx_bulk->dstport,
									   tx_bulk->size );
					}

// #############################################################################
// START #######################################################################
#if 0 // we lock the memory right away, so do not have to worry about it here
					vcos_assert(rx_bulk->handle == VCHI_MEM_HANDLE_INVALID);
#endif
// END #########################################################################
// #############################################################################

// #############################################################################
// BI ##########################################################################
#if 0
					if (tx_bulk->handle ==
					    VCHI_MEM_HANDLE_INVALID)
						tx_data = tx_bulk->data;
					else {
						tx_data =
						    (const char *)
						    mem_lock(tx_bulk->handle) +
						    (int)tx_bulk->data;
						tx_bulk->data = tx_data;	/* Overwrite to avoid an abort */
					}

					vchiq_copy_bulk_to_host(rx_bulk->data,
								tx_data,
								rx_bulk->size);

					if (tx_bulk->handle != VCHI_MEM_HANDLE_INVALID)
						mem_unlock(tx_bulk->handle);
#else
               tx_data = tx_bulk->data;

#ifdef VCHIQ_ARM_SIDE
               vchiq_copy_bulk_from_host(rx_bulk->data, tx_data, rx_bulk->size,
                                         tx_bulk->handle);
#else
               vchiq_copy_bulk_from_host(rx_bulk->data, tx_data, rx_bulk->size);
#endif
#endif
// BI ##########################################################################
// #############################################################################

					vcos_log_trace("%d: rtb %x<->%x(%d)", state->id, service->process, remote->bulk.process, rx_bulk->dstport);
				} else {

					if (vcos_is_log_enabled( &vchiq_core_msg_log_category, VCOS_LOG_INFO))
					{
						vcos_log_impl( &vchiq_core_msg_log_category,
									   VCOS_LOG_INFO,
									   "Send Bulk to %c%c%c%c d:%d ABORTED tx len:%d != rx len: %d",
									   VCHIQ_FOURCC_AS_4CHARS(service->base.fourcc),
									   rx_bulk->dstport,
									   tx_bulk->size,
									   rx_bulk->size );
					}

					/* Abort these transfers */
					rx_bulk->data = NULL;
					tx_bulk->data = NULL;

					vcos_log_trace("%d: rtb %x<->%x(%d) ABORTED", state->id, service->process, remote->bulk.process, rx_bulk->dstport);
				}

				retrigger = 1;

				service->process++;
			}
		}

		remote->bulk.process++;

		remote_event_signal(&remote->trigger);
	}

	if (retrigger)
		remote_event_signal_local(&local->trigger);
}

#endif /* defined(VCHIQ_HANDLE_BULK_TX) */

#if defined(VCHIQ_HANDLE_BULK_RX)

static void resolve_rx_bulks(VCHIQ_STATE_T *state)
{
	VCHIQ_CHANNEL_T *local = state->local;
	VCHIQ_CHANNEL_T *remote = state->remote;
	int retrigger = 0;

	vcos_log_trace("%d: rrb %x %x", state->id, local->bulk.process, local->bulk.insert);
	while (local->bulk.process != local->bulk.insert) {
		VCHIQ_BULK_T *rx_bulk = &local->bulk.bulks[local->bulk.process & (VCHIQ_NUM_CURRENT_BULKS - 1)];
		VCHIQ_SERVICE_T *service;
		VCHIQ_BULK_T *tx_bulk;

		/* Ensure the service isn't being closed */
		if (rx_bulk->dstport < VCHIQ_MAX_SERVICES) {
			service = &local->services[rx_bulk->dstport];
			if (service->srvstate == VCHIQ_SRVSTATE_OPEN) {
				VCHIQ_SERVICE_T *remote_service =
				    &remote->services[service->remoteport];
				if (remote_service->process ==
				    remote_service->insert)
					break;	/* No tx bulk - stall */

				tx_bulk =
				    &remote_service->bulks[remote_service->
							   process &
							   (VCHIQ_NUM_SERVICE_BULKS
							    - 1)];

				if (rx_bulk->size == tx_bulk->size) {
					const void *tx_data;
					void *rx_data;

// #############################################################################
// BI ##########################################################################
#if 0
#ifdef VCHIQ_VC_SIDE
               vcos_assert(tx_bulk->handle == VCHI_MEM_HANDLE_INVALID);
               tx_data = tx_bulk->data;
#else
               if (tx_bulk->handle == VCHI_MEM_HANDLE_INVALID)
                  tx_data = tx_bulk->data;
               else
               {
                  tx_data = (const char *)mem_lock(tx_bulk->handle) + (int)tx_bulk->data;
                  tx_bulk->data = tx_data; /* Overwrite to avoid an abort */
               }
#endif
               if (rx_bulk->handle == VCHI_MEM_HANDLE_INVALID)
                  rx_data = rx_bulk->data;
               else
               {
                  rx_data = (char *)mem_lock(rx_bulk->handle) + (int)rx_bulk->data;
                  rx_bulk->data = rx_data; /* Overwrite to avoid an abort */
               }

               vchiq_copy_bulk_from_host(rx_data, tx_data, rx_bulk->size);

               if (rx_bulk->handle != VCHI_MEM_HANDLE_INVALID)
                  mem_unlock(rx_bulk->handle);

#ifndef VCHIQ_VC_SIDE
               if (tx_bulk->handle != VCHI_MEM_HANDLE_INVALID)
                  mem_unlock(tx_bulk->handle);
#endif
#else
               tx_data = tx_bulk->data;
               rx_data = rx_bulk->data;

#ifdef VCHIQ_ARM_SIDE
               vchiq_copy_bulk_to_host(rx_data, tx_data, rx_bulk->size,
                                       rx_bulk->handle);
#else
               vchiq_copy_bulk_to_host(rx_data, tx_data, rx_bulk->size);
#endif
#endif
// BI ##########################################################################
// #############################################################################
					if (vcos_is_log_enabled( &vchiq_core_msg_log_category, VCOS_LOG_INFO))
					{
						vcos_log_impl( &vchiq_core_msg_log_category,
									  VCOS_LOG_INFO,
									  "Rcvd Bulk to %c%c%c%c d:%d len:%d",
									  VCHIQ_FOURCC_AS_4CHARS(service->base.fourcc),
									  rx_bulk->dstport,
									  rx_bulk->size );
					}

					vcos_log_trace("%d: rrb %x<->%x(%d)", state->id, local->bulk.process, remote_service->process, service->remoteport);
				} else {

					if (vcos_is_log_enabled( &vchiq_core_msg_log_category, VCOS_LOG_INFO))
					{
						vcos_log_impl( &vchiq_core_msg_log_category,
									   VCOS_LOG_INFO,
									   "Rcvd Bulk from %c%c%c%c d:%d ABORTED tx len:%d != rx len: %d",
									   VCHIQ_FOURCC_AS_4CHARS(service->base.fourcc),
									   rx_bulk->dstport,
									   tx_bulk->size,
									   rx_bulk->size );
					}

					/* Abort these non-matching transfers */
					rx_bulk->data = NULL;
					tx_bulk->data = NULL;

					vcos_log_trace("%d: rrb %x<->%x(%d) ABORTED", state->id, local->bulk.process, remote_service->process, service->remoteport);
				}

				remote_service->process++;

				remote_event_signal(&remote->trigger);
			} else {
				rx_bulk->data = NULL; /* Aborted */
				vcos_log_trace("%d: rrb %x ABORTED", state->id, local->bulk.process);
			}
		}

		local->bulk.process++;

		retrigger = 1;
	}

	if (retrigger)
		remote_event_signal_local(&local->trigger);
}

#endif /* defined(VCHIQ_HANDLE_BULK_RX) */

static void parse_rx_slots(VCHIQ_STATE_T *state)
{
	VCHIQ_CHANNEL_T *remote = state->remote;
	VCHIQ_CHANNEL_T *local = state->local;

	while (remote->ctrl.process != remote->ctrl.insert) {
		VCHIQ_HEADER_T *header =
		    (VCHIQ_HEADER_T *) (remote->ctrl.data +
					(remote->ctrl.
					 process & VCHIQ_CHANNEL_MASK));
		VCHIQ_SERVICE_T *service = NULL;
		unsigned int stride = calc_stride(header->size);
		int type = VCHIQ_MSG_TYPE(header->fourcc);

		vcos_log_trace("%d: prs %s(%u) s:%d d:%d @%x",
					   state->id,
					   msg_type_str(type), type,
					   VCHIQ_MSG_SRCPORT(header->fourcc),
					   VCHIQ_MSG_DSTPORT(header->fourcc),
					   remote->ctrl.process & VCHIQ_CHANNEL_MASK);

		if ( vcos_is_log_enabled( &vchiq_core_msg_log_category, VCOS_LOG_INFO))
		{
			int	svc_fourcc;

			svc_fourcc = state->local->services[VCHIQ_MSG_DSTPORT(header->fourcc)].base.fourcc;
			vcos_log_impl( &vchiq_core_msg_log_category,
						   VCOS_LOG_INFO,
						   "Rcvd Msg %s(%u) from %c%c%c%c s:%d d:%d len:%d",
						   msg_type_str(type), type,
						   VCHIQ_FOURCC_AS_4CHARS(svc_fourcc),
						   VCHIQ_MSG_SRCPORT(header->fourcc),
						   VCHIQ_MSG_DSTPORT(header->fourcc),
						   header->size );
			if (header->size > 0) {
				vcos_log_dump_mem( &vchiq_core_msg_log_category,
								   "Rcvd", 0, header->data,
								   vcos_min( 64, header->size ));
			}
		}

		switch (type) {
		case VCHIQ_MSG_OPEN:
			vcos_assert(VCHIQ_MSG_DSTPORT(header->fourcc) == 0);
			if (vcos_verify(header->size == 4)) {
				VCHIQ_HEADER_T *reply;
				unsigned short remoteport =
				    VCHIQ_MSG_SRCPORT(header->fourcc);
				int target;
				service =
				    get_listening_service(local,
							  *(int *)header->data);

				vcos_mutex_lock(local->ctrl.mutex);

				target =
				    local->ctrl.insert + sizeof(VCHIQ_HEADER_T);
				reply = reserve_space(local, target);
				if (!reply) {
					vcos_mutex_unlock(local->ctrl.mutex);
					return;	/* Bail out */
				}

				if (service
				    && (service->srvstate ==
					VCHIQ_SRVSTATE_LISTENING)) {
					/* A matching, listening service exists - attempt the OPEN */
					VCHIQ_STATUS_T status;
					vchiq_set_service_state(service, VCHIQ_SRVSTATE_OPEN);	/* Proceed as if the connection will be accepted */
					status =
					    service->base.
					    callback(VCHIQ_SERVICE_OPENED, NULL,
						     &service->base, NULL);
					if (status == VCHIQ_SUCCESS) {
						/* The open was accepted - acknowledge it */
						reply->fourcc =
						    VCHIQ_MAKE_MSG
						    (VCHIQ_MSG_OPENACK,
						     service->localport,
						     remoteport);
						service->remoteport =
						    remoteport;
					} else {
						vchiq_set_service_state(service,
									VCHIQ_SRVSTATE_LISTENING);

						if (status == VCHIQ_RETRY)
							return;	/* Bail out if not ready */

						/* The open was rejected - send a close */
						reply->fourcc =
						    VCHIQ_MAKE_MSG
						    (VCHIQ_MSG_CLOSE, 0,
						     remoteport);
					}
				} else {
					/* No matching, available service - send a CLOSE */
					reply->fourcc =
					    VCHIQ_MAKE_MSG(VCHIQ_MSG_CLOSE, 0,
							   remoteport);
				}
				reply->size = 0;

				local->ctrl.insert = target;

				vcos_mutex_unlock(local->ctrl.mutex);

				remote_event_signal(&remote->trigger);
			}
			break;
		case VCHIQ_MSG_OPENACK:
			{
				unsigned int localport =
				    VCHIQ_MSG_DSTPORT(header->fourcc);
				unsigned int remoteport =
				    VCHIQ_MSG_SRCPORT(header->fourcc);
				service = &local->services[localport];
				if (vcos_verify
				    (localport < VCHIQ_MAX_SERVICES)
				    && (service->srvstate ==
					VCHIQ_SRVSTATE_OPENING)) {
					service->remoteport = remoteport;
					vchiq_set_service_state(service,
								VCHIQ_SRVSTATE_OPEN);
					vcos_event_signal(service->
							   remove_event);
				}
			}
			break;
		case VCHIQ_MSG_CLOSE:
			{
				unsigned int localport =
				    VCHIQ_MSG_DSTPORT(header->fourcc);
				unsigned int remoteport =
				    VCHIQ_MSG_SRCPORT(header->fourcc);
				vcos_assert(header->size == 0);	/* There should be no data */

				if (vcos_verify(localport < VCHIQ_MAX_SERVICES)) {
					service = &local->services[localport];
					if ((service->remoteport != remoteport) &&
					    VCHIQ_PORT_IS_VALID(service->remoteport)) {
						/* This could be from a client which hadn't yet received
						   the OPENACK - look for the connected service */
						service = get_connected_service(local, remoteport);
						if (!service)
							break;
					}
					switch (service->srvstate) {
					case VCHIQ_SRVSTATE_OPEN:
						/* Return the close */
						if (queue_message(state,
								  VCHIQ_MAKE_MSG
								  (VCHIQ_MSG_CLOSE,
								   service->localport,
								   service->remoteport),
								  NULL, 0,
								  0) ==
						    VCHIQ_RETRY)
							return;	/* Bail out if not ready */

						vchiq_set_service_state(service,
									VCHIQ_SRVSTATE_CLOSESENT);
						/* Drop through... */
					case VCHIQ_SRVSTATE_CLOSESENT:
						vchiq_set_service_state(service,
									VCHIQ_SRVSTATE_CLOSING);
						/* Drop through... */
					case VCHIQ_SRVSTATE_CLOSING:
						/* Start the close procedure */
						if (vchiq_close_service_internal(service) == VCHIQ_RETRY)
							return;	/* Bail out if not ready */
						break;
					case VCHIQ_SRVSTATE_OPENING:
						/* A client is mid-open - this is a rejection, so just fail the open */
						vchiq_set_service_state(service,
									VCHIQ_SRVSTATE_CLOSEWAIT);
						vcos_event_signal(service->
								   remove_event);
						break;
					default:
						break;
					}
				}
			}
			break;
		case VCHIQ_MSG_DATA:
			{
				unsigned int localport =
				    VCHIQ_MSG_DSTPORT(header->fourcc);
				unsigned int remoteport =
				    VCHIQ_MSG_SRCPORT(header->fourcc);
				service = &local->services[localport];
				if (vcos_verify
				    (localport < VCHIQ_MAX_SERVICES)
				    && (service->remoteport == remoteport)
				    && (service->srvstate ==
					VCHIQ_SRVSTATE_OPEN)) {
					if (service->base.
					    callback(VCHIQ_MESSAGE_AVAILABLE,
						     header, &service->base,
						     NULL) == VCHIQ_RETRY)
						return;	/* Bail out if not ready */
					header = NULL;	/* Don't invalidate this message - defer till vchiq_release */
				}
			}
			break;
		case VCHIQ_MSG_CONNECT:
			vcos_event_signal(&state->connect);
			break;
		case VCHIQ_MSG_INVALID:
		default:
			break;
		}

		remote->ctrl.process += stride;
		if (header != NULL) {
			/* Invalidate it */
			header->fourcc = VCHIQ_FOURCC_INVALID;
			/* Notify the other end there is some space */
			remote_event_signal(&remote->ctrl.remove_event);
		}
	}
}

static void *slot_handler_func(void *v)
{
	VCHIQ_STATE_T *state = (VCHIQ_STATE_T *) v;
	VCHIQ_CHANNEL_T *local = state->local;

	while (1) {
		remote_event_wait(&local->trigger);

		parse_rx_slots(state);

#if defined(VCHIQ_HANDLE_BULK_RX)
		resolve_rx_bulks(state);
#endif

#if defined(VCHIQ_HANDLE_BULK_TX)
		resolve_tx_bulks(state);
#endif

		notify_rx_bulks(state);
		notify_tx_bulks(state);
	}
	return NULL;
}

void vchiq_init_channel(VCHIQ_CHANNEL_T *channel)
{
	int i;

	channel->ctrl.remove = 0;
	channel->ctrl.process = 0;
	channel->ctrl.insert = 0;

	remote_event_create(&channel->ctrl.remove_event);
	vcos_mutex_create(channel->ctrl.mutex, "vchiq-ctrl");

	channel->bulk.remove = 0;
	channel->bulk.process = 0;
	channel->bulk.insert = 0;

	vcos_event_create(channel->bulk.remove_event, "vchiq");
	vcos_mutex_create(channel->bulk.mutex, "vchiq-bulk");

	remote_event_create(&channel->trigger);

	/*
	   initialize services
	 */

	for (i = 0; i < VCHIQ_MAX_SERVICES; i++) {
		VCHIQ_SERVICE_T *service = &channel->services[i];
		service->srvstate = VCHIQ_SRVSTATE_FREE;
		service->localport = i;
		vcos_event_create(service->remove_event, "vchiq-service");
	}
}

static inline int is_pow2(int i)
{
	return i && !(i & (i - 1));
}

void vchiq_init_state(VCHIQ_STATE_T *state, VCHIQ_CHANNEL_T * local,
		      VCHIQ_CHANNEL_T *remote)
{
	VCOS_THREAD_ATTR_T attrs;
	char threadname[8];
	static int id = 0;
	int i;

	vcos_log_register("vchiq_core", &vchiq_core_log_category);
	vcos_log_register("vchiq_core_msg", &vchiq_core_msg_log_category);

	vcos_assert(is_pow2(VCHIQ_CHANNEL_SIZE));

	vcos_assert(is_pow2(VCHIQ_NUM_CURRENT_BULKS));
	vcos_assert(is_pow2(VCHIQ_NUM_SERVICE_BULKS));

	vcos_assert(sizeof(VCHIQ_HEADER_T) == 8);	/* we require this for consistency between endpoints */

	memset(state, 0, sizeof(VCHIQ_STATE_T));
	state->id = id++;

	/*
	   initialize events and mutex
	 */

	vcos_event_create(&state->connect, "vchiq");
	vcos_mutex_create(&state->mutex, "vchiq");

	/*
	   initialize channel pointers
	 */

	state->local = local;
	state->remote = remote;

#if defined( __KERNEL__ )
    printk( "%s: state         = 0x%p\n", __func__, state );
    printk( "%s: state->local  = 0x%p\n", __func__, state->local );
    printk( "%s: state->remote = 0x%p\n", __func__, state->remote );
#endif

// #############################################################################
// START #######################################################################
   /* events and mutexs within the CHANNEL memory are actually declared within the
    * local state memory */
   for (i = 0; i < VCHIQ_MAX_SERVICES; i++)
   {
      //vcos_event_create(&state->service_remove_event[i], "vchiq_s");
      local->services[i].remove_event = &state->service_remove_event[i];
   }

   local->ctrl.mutex = &state->ctrl_mutex;
   local->bulk.mutex = &state->bulk_mutex;

#ifdef VCHIQ_LOCAL
   //local->ctrl.remove_event = &state->ctrl_remove_event;
   //local->trigger = &state->trigger_event;
#else
   local->ctrl.remove_event.event = &state->ctrl_remove_event;
   local->trigger.event = &state->trigger_event;
#endif
   local->bulk.remove_event = &state->bulk_remove_event;
// END #########################################################################
// #############################################################################

	vchiq_init_channel(local);

	/*
	   bring up slot handler thread
	 */

	vcos_thread_attr_init(&attrs);
	vcos_thread_attr_setstacksize(&attrs, VCHIQ_SLOT_HANDLER_STACK);
	vcos_thread_attr_setpriority(&attrs, 5);	/* FIXME: should not be hardcoded */
	vcos_thread_attr_settimeslice(&attrs, 20);	/* FIXME: should not be hardcoded */
	strcpy(threadname, "VCHIQ-0");
	threadname[6] += state->id % 10;
	vcos_thread_create(&state->slot_handler_thread, threadname,
			   &attrs, slot_handler_func, state);

	/* Indicate readiness to the other side */
	local->initialised = 1;
}

VCHIQ_SERVICE_T *vchiq_add_service_internal(VCHIQ_STATE_T *state, int fourcc,
					    VCHIQ_CALLBACK_T callback,
					    void *userdata, int srvstate,
					    VCHIQ_INSTANCE_T instance)
{
	VCHIQ_CHANNEL_T *local = state->local;
	VCHIQ_SERVICE_T *service = NULL;
	int i;

	if (srvstate == VCHIQ_SRVSTATE_OPENING) {
		for (i = 0; i < VCHIQ_MAX_SERVICES; i++) {
			VCHIQ_SERVICE_T *srv = &local->services[i];
			if (srv->srvstate == VCHIQ_SRVSTATE_FREE) {
				service = srv;
				break;
			}
		}
	} else {
		for (i = (VCHIQ_MAX_SERVICES - 1); i >= 0; i--) {
			VCHIQ_SERVICE_T *srv = &local->services[i];
			if (srv->srvstate == VCHIQ_SRVSTATE_FREE) {
				service = srv;
			} else if ((srv->fourcc == fourcc) &&
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

		service->base.fourcc = fourcc;
		service->base.callback = callback;
		service->base.userdata = userdata;
		vchiq_set_service_state(service, srvstate);
		service->fourcc =
		    ( srvstate == VCHIQ_SRVSTATE_OPENING )
			? VCHIQ_FOURCC_INVALID : fourcc;
		service->state = state;
		service->instance = instance;
		service->remoteport = VCHIQ_PORT_FREE;
		service->remove = 0;
		service->process = 0;
		service->insert = 0;

		/* Ensure the events are unsignalled */
		while (vcos_event_try(service->remove_event) == VCOS_SUCCESS)
			continue;
	}

	return service;
}

VCHIQ_STATUS_T vchiq_open_service_internal(VCHIQ_SERVICE_T *service)
{
	VCHIQ_ELEMENT_T body = { &service->base.fourcc, sizeof(service->base.fourcc) };
	VCHIQ_STATUS_T status = queue_message(service->state,
					      VCHIQ_MAKE_MSG(VCHIQ_MSG_OPEN, service->localport, 0),
					      &body, 1, sizeof(service->base.fourcc));
	if (status == VCHIQ_SUCCESS) {
		if (vcos_event_wait(service->remove_event) != VCOS_SUCCESS) {
			status = VCHIQ_RETRY;
		} else if (service->srvstate != VCHIQ_SRVSTATE_OPEN) {
			vcos_log_trace("%d: osi - srvstate = %d", service->state->id, service->srvstate);
			vcos_assert(service->srvstate == VCHIQ_SRVSTATE_CLOSEWAIT);
			status = VCHIQ_ERROR;
		}
	}
	return status;
}

VCHIQ_STATUS_T vchiq_close_service_internal(VCHIQ_SERVICE_T *service)
{
	/* This is the first half of the close process, the remainder handled by
	   notify_tx_bulks */
	VCHIQ_CHANNEL_T *local = service->state->local;
	int pos;
	VCHIQ_STATUS_T status = VCHIQ_SUCCESS;
	int reason;

	vcos_log_trace("%d: csi %d - i %x, p %x, r %x/i %x, p %x, r %x", service->state->id, service->localport, service->insert, service->process, service->remove, local->bulk.insert, local->bulk.process, local->bulk.remove);

	/* Complete any outstanding bulk receives */

	reason = VCHIQ_BULK_RECEIVE_DONE;

	for (pos = local->bulk.remove; pos != local->bulk.insert; pos++) {
		VCHIQ_BULK_T *bulk =
		    &local->bulk.bulks[pos & (VCHIQ_NUM_CURRENT_BULKS - 1)];

		if (pos == local->bulk.process)
			reason = VCHIQ_BULK_RECEIVE_ABORTED;

		vcos_log_trace("%d: csi %d - %x %d", service->state->id, service->localport, pos, bulk->dstport);

		if (bulk->dstport == service->localport) {
			VCHIQ_SERVICE_T *service =
			    &local->services[bulk->dstport];

			status = service->base.callback(bulk->data ? reason : VCHIQ_BULK_RECEIVE_ABORTED, NULL, &service->base, bulk->userdata);
			vcos_log_trace("%d: csi %d - %x callback(%d)->%d", service->state->id, service->localport, pos, reason, status);
			if (status == VCHIQ_RETRY)
				break; /* Bail out if not ready */

			vchiq_complete_bulk(bulk);

			bulk->dstport = VCHIQ_PORT_FREE;	/* Avoid a second callback */
		}
	}

	if (status == VCHIQ_SUCCESS) {
		/* Complete any outstanding bulk transmits */

		reason = VCHIQ_BULK_TRANSMIT_DONE;

		while (service->remove != service->insert) {
			VCHIQ_BULK_T *bulk =
			    &service->bulks[service->
					    remove & (VCHIQ_NUM_SERVICE_BULKS -
						      1)];

			if (service->remove == service->process)
				reason = VCHIQ_BULK_TRANSMIT_ABORTED;

			status = service->base.callback(reason, NULL, &service->base, bulk->userdata);
			vcos_log_trace("%d: csi %d - %x callback(%d)->%d", service->state->id, service->localport, service->remove, reason, status);
			if (status == VCHIQ_RETRY)
				break;

			vchiq_complete_bulk(bulk);

			if (service->remove == service->process)
				service->process++;
			service->remove++;
		}

		if (service->remove == service->insert) {
			int oldstate = service->srvstate;
			vcos_assert(service->process == service->insert);

			if (service->instance) {
				vchiq_set_service_state(service,
							(service->fourcc ==
							 VCHIQ_FOURCC_INVALID) ?
							VCHIQ_SRVSTATE_CLOSEWAIT :
							VCHIQ_SRVSTATE_LISTENING);

				status = service->base.callback(VCHIQ_SERVICE_CLOSED, NULL, &service->base, NULL);
				vcos_log_trace("%d: csi %d - callback(SERVICE_CLOSED)->%d", service->state->id, service->localport, status);

				if (vcos_is_log_enabled( &vchiq_core_msg_log_category, VCOS_LOG_INFO))
				{
					int	svc_fourcc;

					svc_fourcc = service->base.fourcc;

					vcos_log_impl( &vchiq_core_msg_log_category,
								   VCOS_LOG_INFO,
								   "Close Service %c%c%c%c s:%u d:%d",
								   VCHIQ_FOURCC_AS_4CHARS(svc_fourcc),
								   service->localport,
								   service->remoteport );
				}

				if (status == VCHIQ_RETRY)
					vchiq_set_service_state(service, oldstate);
				else {
					if (status == VCHIQ_ERROR) {
						/* Signal an error (fatal, since the other end will probably have closed) */
						vchiq_set_service_state(service, VCHIQ_SRVSTATE_OPEN);
					}
					vcos_event_signal(service->remove_event);
				}
			} else {
				/* The client has died - transition to free */
				vchiq_set_service_state(service, VCHIQ_SRVSTATE_FREE);
			}
		}
	}

	vcos_log_trace("%d: csi - i %x, p %x, r %x -> %d", service->state->id, service->insert, service->process, service->remove, status);

	return status;
}

void vchiq_terminate_service_internal(VCHIQ_SERVICE_T *service)
{
	VCHIQ_CHANNEL_T *remote = service->state->remote;
	int remove, fourcc;

	/* Release any unreleased messages */
	remove = remote->ctrl.remove;

	vcos_log_trace("%d: tsi - (%d<->%d) i %x, p %x, r %x", service->state->id, service->localport, service->remoteport, remote->ctrl.insert, remote->ctrl.process, remove);

	fourcc =
	    VCHIQ_MAKE_MSG(VCHIQ_MSG_DATA, service->remoteport,
			   service->localport);
	while (remove != remote->ctrl.insert) {
		VCHIQ_HEADER_T *header =
		    (VCHIQ_HEADER_T *) (remote->ctrl.data +
					(remove & VCHIQ_CHANNEL_MASK));

		remove += calc_stride(header->size);

		if (header->fourcc == fourcc)
			header->fourcc = VCHIQ_FOURCC_INVALID;
	}

	/* Mark the service for termination by the slot handler... */
	service->terminate = 1;

	/* Disconnect from the instance */
	service->instance = NULL;

	/* ... and ensure the slot handler runs. */
	remote_event_signal_local(&service->state->local->trigger);
}

VCHIQ_STATUS_T vchiq_connect_internal(VCHIQ_STATE_T *state,
				      VCHIQ_INSTANCE_T instance)
{
	VCHIQ_CHANNEL_T *local = state->local;
	int i;

	/* Find all services registered to this client and enable them. */
	for (i = 0; i < VCHIQ_MAX_SERVICES; i++)
		if (local->services[i].instance == instance) {
			if (local->services[i].srvstate ==
			    VCHIQ_SRVSTATE_HIDDEN)
				vchiq_set_service_state(&local->services[i],
							VCHIQ_SRVSTATE_LISTENING);
		}

	if (!state->connected) {
		vcos_log_trace( "%s: Sending CONNECT message", __func__ );
		if (queue_message
		    (state, VCHIQ_MAKE_MSG(VCHIQ_MSG_CONNECT, 0, 0), NULL, 0,
		     0) == VCHIQ_RETRY)
			return VCHIQ_RETRY;

		vcos_event_wait(&state->connect);
		state->connected = 1;
	}
	else
	{
		vcos_log_trace( "%s: already connected", __func__ );
	}

	return VCHIQ_SUCCESS;
}

VCHIQ_STATUS_T vchiq_remove_service(VCHIQ_SERVICE_HANDLE_T handle)
{
	/* Unregister the service */
	VCHIQ_SERVICE_T *service = (VCHIQ_SERVICE_T *) handle;
	VCHIQ_STATE_T *state = service->state;
	VCHIQ_STATUS_T status = VCHIQ_SUCCESS;

	switch (service->srvstate) {
	case VCHIQ_SRVSTATE_OPENING:
	case VCHIQ_SRVSTATE_OPEN:
		{
			int oldstate = service->srvstate;

			/* Start the CLOSE procedure */
			vchiq_set_service_state(service,
						VCHIQ_SRVSTATE_CLOSESENT);
			status =
			    queue_message(state,
					  VCHIQ_MAKE_MSG(VCHIQ_MSG_CLOSE,
							 service->localport,
							 VCHIQ_MSG_DSTPORT(service->remoteport)),
					  NULL, 0, 0);

			if (status != VCHIQ_SUCCESS)
				vchiq_set_service_state(service, oldstate);
		}
		break;

	case VCHIQ_SRVSTATE_HIDDEN:
	case VCHIQ_SRVSTATE_LISTENING:
	case VCHIQ_SRVSTATE_CLOSING:
	case VCHIQ_SRVSTATE_CLOSEWAIT:
		break;

	default:
		status = VCHIQ_ERROR;
		break;
	}

	while ((service->srvstate == VCHIQ_SRVSTATE_CLOSING) ||
	       (service->srvstate == VCHIQ_SRVSTATE_CLOSESENT)) {
		if (vcos_event_wait(service->remove_event) != VCOS_SUCCESS) {
			status = VCHIQ_RETRY;
			break;
		}
	}

	if (status == VCHIQ_SUCCESS) {
		if (service->srvstate == VCHIQ_SRVSTATE_OPEN)
			status = VCHIQ_ERROR;
		else
			vchiq_set_service_state(service, VCHIQ_SRVSTATE_FREE);
	}

	return status;
}

VCHIQ_STATUS_T vchiq_queue_bulk_transmit(VCHIQ_SERVICE_HANDLE_T handle,
					 const void *data, int size,
					 void *userdata)
{
	return vchiq_bulk_transmit(handle, data, size, userdata, VCHIQ_BULK_MODE_CALLBACK);
}

VCHIQ_STATUS_T vchiq_queue_bulk_receive(VCHIQ_SERVICE_HANDLE_T handle,
					void *data, int size, void *userdata)
{
	return vchiq_bulk_receive(handle, data, size, userdata, VCHIQ_BULK_MODE_CALLBACK);
}

VCHIQ_STATUS_T vchiq_queue_bulk_transmit_handle(VCHIQ_SERVICE_HANDLE_T handle,
						VCHI_MEM_HANDLE_T memhandle,
						const void *offset, int size,
						void *userdata)
{
	return vchiq_bulk_transmit_handle(handle, memhandle, offset, size, userdata, VCHIQ_BULK_MODE_CALLBACK);
}

VCHIQ_STATUS_T vchiq_queue_bulk_receive_handle(VCHIQ_SERVICE_HANDLE_T handle,
					       VCHI_MEM_HANDLE_T memhandle,
					       void *offset, int size,
					       void *userdata)
{
	return vchiq_bulk_receive_handle(handle, memhandle, offset, size, userdata, VCHIQ_BULK_MODE_CALLBACK);
}

VCHIQ_STATUS_T vchiq_bulk_transmit(VCHIQ_SERVICE_HANDLE_T handle,
					 const void *data, int size,
					 void *userdata,
				     VCHIQ_BULK_MODE_T mode)
{
	VCHIQ_SERVICE_T *service = (VCHIQ_SERVICE_T *) handle;
	VCHIQ_BULK_T *bulk;
	VCHIQ_STATE_T *state;

	vcos_assert((service != NULL) && (data != NULL));

	state = service->state;

	vcos_log_trace("%d: qbt %d", state->id, service->localport);

	if (service->srvstate != VCHIQ_SRVSTATE_OPEN)
		return VCHIQ_ERROR;	/* Must be connected */

	while (service->insert == service->remove + VCHIQ_NUM_SERVICE_BULKS)
		if (vcos_event_wait(service->remove_event) != VCOS_SUCCESS)
			return VCHIQ_RETRY;

	bulk = &service->bulks[service->insert & (VCHIQ_NUM_SERVICE_BULKS - 1)];

	if (vchiq_prepare_bulk(bulk, service->remoteport, VCHI_MEM_HANDLE_INVALID,
		(void *)data, size, userdata, mode, VCHIQ_BULK_TRANSMIT) != VCHIQ_SUCCESS)
		return VCHIQ_ERROR;

	vcos_log_trace("%d: qbt %d %x", state->id, service->localport, service->insert);

	service->insert++;

#ifdef VCHIQ_HANDLE_BULK_TX
	remote_event_signal_local(&state->local->trigger);
#else
	remote_event_signal(&state->remote->trigger);
#endif

	return VCHIQ_SUCCESS;
}

VCHIQ_STATUS_T vchiq_bulk_receive(VCHIQ_SERVICE_HANDLE_T handle,
					void *data, int size, void *userdata,
					VCHIQ_BULK_MODE_T mode)
{
	VCHIQ_SERVICE_T *service = (VCHIQ_SERVICE_T *) handle;
	VCHIQ_STATE_T *state;
	VCHIQ_CHANNEL_T *local;
	VCHIQ_BULK_T *bulk;

	vcos_assert((service != NULL) && (data != NULL));

	state = service->state;
	local = state->local;

	vcos_log_trace("%d: qbr %d", state->id, service->localport);

	if (service->srvstate != VCHIQ_SRVSTATE_OPEN)
		return VCHIQ_ERROR;	/* Must be connected (receives are processed in order of submission) */

	vcos_mutex_lock(local->bulk.mutex);

	while (local->bulk.insert ==
	       local->bulk.remove + VCHIQ_NUM_CURRENT_BULKS)
		vcos_event_wait(local->bulk.remove_event);

	bulk =
	    &local->bulk.bulks[local->bulk.
			       insert & (VCHIQ_NUM_CURRENT_BULKS - 1)];

	if (vchiq_prepare_bulk(bulk, service->localport, VCHI_MEM_HANDLE_INVALID,
		data, size, userdata, mode, VCHIQ_BULK_RECEIVE) != VCHIQ_SUCCESS)
		return VCHIQ_ERROR;

	vcos_log_trace("%d: qbr %d %x", state->id, service->localport, local->bulk.insert);

	local->bulk.insert++;

	vcos_mutex_unlock(local->bulk.mutex);

#ifdef VCHIQ_HANDLE_BULK_RX
	remote_event_signal_local(&local->trigger);
#else
	remote_event_signal(&state->remote->trigger);
#endif

	return VCHIQ_SUCCESS;
}

VCHIQ_STATUS_T vchiq_bulk_transmit_handle(VCHIQ_SERVICE_HANDLE_T handle,
						VCHI_MEM_HANDLE_T memhandle,
						const void *offset, int size,
						void *userdata,
						VCHIQ_BULK_MODE_T mode)
{
	VCHIQ_SERVICE_T *service = (VCHIQ_SERVICE_T *) handle;
	VCHIQ_BULK_T *bulk;
	VCHIQ_STATE_T *state;

	vcos_assert(service != NULL);

	state = service->state;

	vcos_log_trace("%d: qbth %d", state->id, service->localport);

	if (service->srvstate != VCHIQ_SRVSTATE_OPEN)
		return VCHIQ_ERROR;	/* Must be connected */

	while (service->insert == service->remove + VCHIQ_NUM_SERVICE_BULKS)
		if (vcos_event_wait(service->remove_event) != VCOS_SUCCESS)
			return VCHIQ_RETRY;

	bulk = &service->bulks[service->insert & (VCHIQ_NUM_SERVICE_BULKS - 1)];

	if (vchiq_prepare_bulk(bulk, service->remoteport, memhandle,
		(void *)offset, size, userdata, mode, VCHIQ_BULK_TRANSMIT) != VCHIQ_SUCCESS)
		return VCHIQ_ERROR;

	vcos_log_trace("%d: qbth %d %x", state->id, service->localport, service->insert);

	service->insert++;

#ifdef VCHIQ_HANDLE_BULK_TX
	remote_event_signal_local(&state->local->trigger);
#else
	remote_event_signal(&state->remote->trigger);
#endif

	return VCHIQ_SUCCESS;
}

VCHIQ_STATUS_T vchiq_bulk_receive_handle(VCHIQ_SERVICE_HANDLE_T handle,
					       VCHI_MEM_HANDLE_T memhandle,
					       void *offset, int size,
					       void *userdata,
						   VCHIQ_BULK_MODE_T mode)
{
	VCHIQ_SERVICE_T *service = (VCHIQ_SERVICE_T *) handle;
	VCHIQ_STATE_T *state;
	VCHIQ_CHANNEL_T *local;
	VCHIQ_BULK_T *bulk;

	vcos_assert(service != NULL);

	state = service->state;
	local = state->local;

	vcos_log_trace("%d: qbrh %d", state->id, service->localport);

	if (service->srvstate != VCHIQ_SRVSTATE_OPEN)
		return VCHIQ_ERROR;	/* Must be connected (receives are processed in order of submission) */

	vcos_mutex_lock(local->bulk.mutex);

	while (local->bulk.insert ==
	       local->bulk.remove + VCHIQ_NUM_CURRENT_BULKS)
		vcos_event_wait(local->bulk.remove_event);

	bulk =
	    &local->bulk.bulks[local->bulk.
			       insert & (VCHIQ_NUM_CURRENT_BULKS - 1)];

	if (vchiq_prepare_bulk(bulk, service->localport, memhandle,
						   offset, size, userdata, mode, VCHIQ_BULK_RECEIVE) != VCHIQ_SUCCESS )
		return VCHIQ_ERROR;

	vcos_log_trace("%d: qbrh %d %x", state->id, service->localport, local->bulk.insert);

	local->bulk.insert++;

	vcos_mutex_unlock(local->bulk.mutex);

#ifdef VCHIQ_HANDLE_BULK_RX
	remote_event_signal_local(&local->trigger);
#else
	remote_event_signal(&state->remote->trigger);
#endif

	return VCHIQ_SUCCESS;
}

VCHIQ_STATUS_T vchiq_queue_message(VCHIQ_SERVICE_HANDLE_T handle,
				   const VCHIQ_ELEMENT_T *elements, uint32_t count)
{
	VCHIQ_SERVICE_T *service = (VCHIQ_SERVICE_T *) handle;

	uint32_t size = 0;
	uint32_t i;

	if (service->srvstate != VCHIQ_SRVSTATE_OPEN)
		return VCHIQ_ERROR;

	for (i = 0; i < count; i++)
		size += elements[i].size;

	if (calc_stride(size) > VCHIQ_CHANNEL_SIZE)
		return VCHIQ_ERROR;

	return queue_message(service->state,
			     VCHIQ_MAKE_MSG(VCHIQ_MSG_DATA, service->localport,
					    service->remoteport), elements,
			     count, size);
}

void vchiq_release_message(VCHIQ_SERVICE_HANDLE_T handle,
			   VCHIQ_HEADER_T *header)
{
	VCHIQ_SERVICE_T *service = (VCHIQ_SERVICE_T *) handle;

	vcos_assert(header->fourcc != VCHIQ_FOURCC_INVALID);

	header->fourcc = VCHIQ_FOURCC_INVALID;
	remote_event_signal(&service->state->remote->ctrl.remove_event);
}

#if defined(__KERNEL__)
EXPORT_SYMBOL(vchiq_queue_bulk_transmit);
EXPORT_SYMBOL(vchiq_queue_bulk_receive);
#endif
