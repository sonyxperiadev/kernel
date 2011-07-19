/*=============================================================================
Copyright (c) 2010 Broadcom Europe Ltd. All rights reserved.

Project  :  ARM
Module   :  vchiq_arm

FILE DESCRIPTION:
Header file for the VCHIQ API.

==============================================================================*/

#ifndef VCHIQ_IF_H
#define VCHIQ_IF_H

#if defined(VCHIQ_VC_SIDE) || defined(VCHIQ_LOCAL)
#define USE_MEMMGR
#include "vcfw/rtos/common/rtos_common_mem.h"
#endif

#include "interface/vchi/vchi_mh.h"

#define VCHIQ_SLOT_SIZE          4096
#define VCHIQ_MAKE_FOURCC(x0, x1, x2, x3)     (((x3) << 24) | ((x2) << 16) | ((x1) << 8) | (x0))
#define VCHIQ_GET_SERVICE_USERDATA(service)   (service->userdata)
#define VCHIQ_GET_SERVICE_FOURCC(service)     (service->fourcc)

#define VCHIQ_CHANNEL_SIZE        16384
#define VCHIQ_MAX_MSG_SIZE        VCHIQ_CHANNEL_SIZE

typedef enum {
   VCHIQ_SERVICE_OPENED,         // service, -, -
   VCHIQ_SERVICE_CLOSED,         // service, -, -
   VCHIQ_MESSAGE_AVAILABLE,      // service, header, -
   VCHIQ_BULK_TRANSMIT_DONE,     // service, -, bulk_userdata
   VCHIQ_BULK_RECEIVE_DONE,      // service, -, bulk_userdata
   VCHIQ_BULK_TRANSMIT_ABORTED,  // service, -, bulk_userdata
   VCHIQ_BULK_RECEIVE_ABORTED    // service, -, bulk_userdata
} VCHIQ_REASON_T;

typedef enum
{
   VCHIQ_ERROR   = -1,
   VCHIQ_SUCCESS = 0,
   VCHIQ_RETRY   = 1
} VCHIQ_STATUS_T;

typedef enum
{
   VCHIQ_BULK_MODE_CALLBACK,
   VCHIQ_BULK_MODE_BLOCKING,
   VCHIQ_BULK_MODE_NOCALLBACK
} VCHIQ_BULK_MODE_T;

#ifdef __HIGHC__
#pragma warning (push)
#pragma warning (disable : 4200)
#endif

typedef struct vchiq_header_struct {
   int fourcc;             /* fourcc of target service */
   unsigned int size;      /* size of message */

   char data[0];           /* message */
} VCHIQ_HEADER_T;

#ifdef __HIGHC__
#pragma warning (pop)
#endif

typedef struct {
   const void *data;
   uint32_t    size;
} VCHIQ_ELEMENT_T;

typedef const struct vchiq_service_base_struct *VCHIQ_SERVICE_HANDLE_T;

typedef VCHIQ_STATUS_T (*VCHIQ_CALLBACK_T)(VCHIQ_REASON_T, VCHIQ_HEADER_T *, VCHIQ_SERVICE_HANDLE_T, void *);

typedef struct vchiq_service_base_struct {
   int fourcc;
   VCHIQ_CALLBACK_T callback;
   void *userdata;
} VCHIQ_SERVICE_BASE_T;

typedef struct vchiq_instance_struct *VCHIQ_INSTANCE_T;

extern VCHIQ_STATUS_T vchiq_initialise(VCHIQ_INSTANCE_T *pinstance);
extern VCHIQ_STATUS_T vchiq_shutdown(VCHIQ_INSTANCE_T instance);
extern VCHIQ_STATUS_T vchiq_connect(VCHIQ_INSTANCE_T instance);
// #############################################################################
// START #######################################################################
extern int            vchiq_is_connected(VCHIQ_INSTANCE_T instance);
// END #########################################################################
// #############################################################################
extern VCHIQ_STATUS_T vchiq_add_service(VCHIQ_INSTANCE_T instance, int fourcc, VCHIQ_CALLBACK_T callback, void *userdata, VCHIQ_SERVICE_HANDLE_T *pservice);
extern VCHIQ_STATUS_T vchiq_open_service(VCHIQ_INSTANCE_T instance, int fourcc, VCHIQ_CALLBACK_T callback, void *userdata, VCHIQ_SERVICE_HANDLE_T *pservice);
extern VCHIQ_STATUS_T vchiq_close_service(VCHIQ_SERVICE_HANDLE_T service);
extern VCHIQ_STATUS_T vchiq_remove_service(VCHIQ_SERVICE_HANDLE_T service);
// #############################################################################
// START #######################################################################
extern void           vchiq_terminate_service(VCHIQ_SERVICE_HANDLE_T service);
// END #########################################################################
// #############################################################################

extern VCHIQ_STATUS_T vchiq_queue_message(VCHIQ_SERVICE_HANDLE_T service, const VCHIQ_ELEMENT_T *elements, uint32_t count);
extern void           vchiq_release_message(VCHIQ_SERVICE_HANDLE_T service, VCHIQ_HEADER_T *header);
extern VCHIQ_STATUS_T vchiq_queue_bulk_transmit(VCHIQ_SERVICE_HANDLE_T service, const void *data, int size, void *userdata);
extern VCHIQ_STATUS_T vchiq_queue_bulk_receive(VCHIQ_SERVICE_HANDLE_T service, void *data, int size, void *userdata);
extern VCHIQ_STATUS_T vchiq_queue_bulk_transmit_handle(VCHIQ_SERVICE_HANDLE_T service, VCHI_MEM_HANDLE_T handle, const void *offset, int size, void *userdata);
extern VCHIQ_STATUS_T vchiq_queue_bulk_receive_handle(VCHIQ_SERVICE_HANDLE_T service, VCHI_MEM_HANDLE_T handle, void *offset, int size, void *userdata);
extern VCHIQ_STATUS_T vchiq_bulk_transmit(VCHIQ_SERVICE_HANDLE_T service, const void *data, int size, void *userdata, VCHIQ_BULK_MODE_T mode);
extern VCHIQ_STATUS_T vchiq_bulk_receive(VCHIQ_SERVICE_HANDLE_T service, void *data, int size, void *userdata, VCHIQ_BULK_MODE_T mode);
extern VCHIQ_STATUS_T vchiq_bulk_transmit_handle(VCHIQ_SERVICE_HANDLE_T service, VCHI_MEM_HANDLE_T handle, const void *offset, int size, void *userdata, VCHIQ_BULK_MODE_T mode);
extern VCHIQ_STATUS_T vchiq_bulk_receive_handle(VCHIQ_SERVICE_HANDLE_T service, VCHI_MEM_HANDLE_T handle, void *offset, int size, void *userdata, VCHIQ_BULK_MODE_T mode);

#endif /* VCHIQ_IF_H */
