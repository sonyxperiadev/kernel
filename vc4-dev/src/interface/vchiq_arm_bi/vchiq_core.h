/*=============================================================================
Copyright (c) 2010 Broadcom Europe Ltd. All rights reserved.

Project  :  ARM
Module   :  vchiq_arm

FILE DESCRIPTION:
Header file for core VCHIQ logic.

==============================================================================*/

#ifndef VCHIQ_CORE_H
#define VCHIQ_CORE_H

#if defined(VCHIQ_LOCAL)

/* Implicitly VideoCore */
#define VCHIQ_HANDLE_BULK_RX

#elif defined(VCHIQ_VC_SIDE)

#if !defined(__VC4_BIG_ISLAND__)
#define VCHIQ_HANDLE_BULK_RX
#define VCHIQ_HANDLE_BULK_TX
#endif

#elif defined(VCHIQ_ARM_SIDE)

#define VCHIQ_HANDLE_BULK_RX
#define VCHIQ_HANDLE_BULK_TX

#endif

#include "vchiq.h"
#include "interface/vcos/vcos_types.h"
#include "interface/vcos/vcos_logging.h"

#define VCHIQ_CHANNEL_MASK        (VCHIQ_CHANNEL_SIZE - 1)

/* Ensure that the channel size is a power of 2 */
vcos_static_assert((VCHIQ_CHANNEL_SIZE & VCHIQ_CHANNEL_MASK) == 0);

#define VCHIQ_NUM_CURRENT_BULKS        32
#define VCHIQ_NUM_SERVICE_BULKS        8

/*
 * When VCHIQ_NUM_SERVICES is 64, then 30912 bytes are required (per VCHIQ_CHANNEL_T)
 * (each service needs 216 bytes), and we currently have 0x8000 (32768)
 * bytes of space.
 */
#define VCHIQ_MAX_SERVICES             64

#define VCHIQ_MSG_INVALID              0  // -
#define VCHIQ_MSG_OPEN                 1  // + (srcport, -), fourcc
#define VCHIQ_MSG_OPENACK              2  // + (srcport, dstport)
#define VCHIQ_MSG_CLOSE                3  // + (srcport, dstport)
#define VCHIQ_MSG_DATA                 4  // + (srcport, dstport)
#define VCHIQ_MSG_CONNECT              5  // -

#define VCHIQ_PORT_MAX                 (VCHIQ_MAX_SERVICES - 1)
#define VCHIQ_PORT_FREE                0x1000
#define VCHIQ_PORT_IS_VALID(port)      (port < VCHIQ_PORT_FREE)
#define VCHIQ_MAKE_MSG(type,srcport,dstport)      ((type<<24) | (srcport<<12) | (dstport<<0))
#define VCHIQ_MSG_TYPE(fourcc)         ((unsigned int)fourcc >> 24)
#define VCHIQ_MSG_SRCPORT(fourcc)      (unsigned short)(((unsigned int)fourcc >> 12) & 0xfff)
#define VCHIQ_MSG_DSTPORT(fourcc)      ((unsigned short)fourcc & 0xfff)

#define VCHIQ_FOURCC_AS_4CHARS(fourcc)	\
	((fourcc) >> 24) & 0xff, \
	((fourcc) >> 16) & 0xff, \
	((fourcc) >>  8) & 0xff, \
	((fourcc)      ) & 0xff


/* Ensure the fields are wide enough */
vcos_static_assert(VCHIQ_MSG_SRCPORT(VCHIQ_MAKE_MSG(0,0,VCHIQ_PORT_MAX)) == 0);
vcos_static_assert(VCHIQ_MSG_TYPE(VCHIQ_MAKE_MSG(0,VCHIQ_PORT_MAX,0)) == 0);

#define VCHIQ_FOURCC_INVALID           0x00000000
#define VCHIQ_FOURCC_IS_LEGAL(fourcc)  (fourcc != VCHIQ_FOURCC_INVALID)

enum
{
   VCHIQ_SRVSTATE_FREE,
   VCHIQ_SRVSTATE_HIDDEN,
   VCHIQ_SRVSTATE_LISTENING,
   VCHIQ_SRVSTATE_OPENING,
   VCHIQ_SRVSTATE_OPEN,
   VCHIQ_SRVSTATE_CLOSESENT,
   VCHIQ_SRVSTATE_CLOSING,
   VCHIQ_SRVSTATE_CLOSEWAIT
};

enum
{
   VCHIQ_BULK_TRANSMIT,
   VCHIQ_BULK_RECEIVE
};

typedef volatile struct {
   unsigned int dstport;
   VCHI_MEM_HANDLE_T handle;
   void *data;
   int size;
   void *userdata;
} VCHIQ_BULK_T;

typedef struct remote_event_struct {
   volatile int armed;
   volatile int fired;
   VCOS_EVENT_T * event;
} REMOTE_EVENT_T;

typedef struct vchiq_state_struct VCHIQ_STATE_T;

typedef struct vchiq_service_struct {
   VCHIQ_SERVICE_BASE_T base;
   volatile int srvstate;
   unsigned int localport;
   unsigned int remoteport;
   int fourcc;
   int terminate;

   VCHIQ_STATE_T *state;
   VCHIQ_INSTANCE_T instance;

   VCHIQ_BULK_T bulks[VCHIQ_NUM_SERVICE_BULKS];

   volatile int remove;
   volatile int process;
   volatile int insert;

   VCOS_EVENT_T * remove_event;
} VCHIQ_SERVICE_T;

typedef struct vchiq_channel_struct {
   struct {
      volatile char data[VCHIQ_CHANNEL_SIZE];

      volatile int remove;
      volatile int process;
      volatile int insert;

      REMOTE_EVENT_T remove_event;

      VCOS_MUTEX_T * mutex;
   } ctrl;

   struct {
      VCHIQ_BULK_T bulks[VCHIQ_NUM_CURRENT_BULKS];

      volatile int remove;
      volatile int process;
      volatile int insert;

      VCOS_EVENT_T * remove_event;

      VCOS_MUTEX_T * mutex;
   } bulk;

   VCHIQ_SERVICE_T services[VCHIQ_MAX_SERVICES];

   REMOTE_EVENT_T trigger;

   volatile int initialised;
} VCHIQ_CHANNEL_T;

struct vchiq_state_struct {
   VCHIQ_CHANNEL_T *local;
   VCHIQ_CHANNEL_T *remote;

   int id;
   int initialised;
   int connected;
   VCOS_EVENT_T connect;      // event indicating connect message received
   VCOS_MUTEX_T mutex;        // mutex protecting services
   VCHIQ_CALLBACK_T connect_callback;
   VCHIQ_INSTANCE_T *instance;

   VCOS_THREAD_T slot_handler_thread;  // slot handler

   // moved from the channel structure
   VCOS_MUTEX_T ctrl_mutex;
   VCOS_MUTEX_T bulk_mutex;

   VCOS_EVENT_T ctrl_remove_event;
   VCOS_EVENT_T bulk_remove_event;
   VCOS_EVENT_T trigger_event;
   VCOS_EVENT_T service_remove_event[VCHIQ_MAX_SERVICES];

};

extern void vchiq_init_channel(VCHIQ_CHANNEL_T *channel);

extern void vchiq_init_state(VCHIQ_STATE_T *state, VCHIQ_CHANNEL_T *local, VCHIQ_CHANNEL_T *remote);
extern VCHIQ_STATUS_T vchiq_connect_internal(VCHIQ_STATE_T *state, VCHIQ_INSTANCE_T instance);
extern VCHIQ_SERVICE_T *vchiq_add_service_internal(VCHIQ_STATE_T *state, int fourcc, VCHIQ_CALLBACK_T callback,
                                                   void *userdata, int srvstate, VCHIQ_INSTANCE_T instance);
extern VCHIQ_STATUS_T vchiq_open_service_internal(VCHIQ_SERVICE_T *service);
extern VCHIQ_STATUS_T vchiq_close_service_internal(VCHIQ_SERVICE_T *service);
extern void vchiq_terminate_service_internal(VCHIQ_SERVICE_T *service);

extern VCHIQ_STATUS_T vchiq_copy_from_user(void *dst, const void *src, int size);
extern void vchiq_ring_doorbell(void);
// #############################################################################
// START #######################################################################
#ifdef VCHIQ_ARM_SIDE
extern void vchiq_copy_bulk_from_host(void *dst, const void *src, int size, VCHI_MEM_HANDLE_T handle);
extern void vchiq_copy_bulk_to_host(void *dst, const void *src, int size, VCHI_MEM_HANDLE_T handle);
#else
extern void vchiq_copy_bulk_from_host(void *dst, const void *src, int size);
extern void vchiq_copy_bulk_to_host(void *dst, const void *src, int size);
#endif
// END #########################################################################
// #############################################################################
extern void remote_event_pollall(VCHIQ_STATE_T *state);
extern VCHIQ_STATUS_T vchiq_prepare_bulk(VCHIQ_BULK_T *bulk, unsigned int dstport,
                                         VCHI_MEM_HANDLE_T memhandle, void *offset,
                                         int size, void *userdata, VCHIQ_BULK_MODE_T mode,
                                         int dir);
extern void vchiq_complete_bulk(VCHIQ_BULK_T *bulk);

#endif
