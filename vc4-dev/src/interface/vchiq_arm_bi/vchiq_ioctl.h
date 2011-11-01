/*=============================================================================
Copyright (c) 2010 Broadcom Europe Ltd. All rights reserved.

Project  :  ARM
Module   :  vchiq_arm

FILE DESCRIPTION:
User-space library frontend for Linux VCHIQ driver.

==============================================================================*/

#ifndef VCHIQ_IOCTLS_H
#define VCHIQ_IOCTLS_H

#include <linux/ioctl.h>
#include "vchiq_if.h"

#define VCHIQ_IOC_MAGIC 0xc4
#define VCHIQ_INVALID_HANDLE -1

typedef struct {
   int fourcc;
   void *service_userdata;
   int handle;       /* OUT */
} VCHIQ_ADD_SERVICE_T, VCHIQ_OPEN_SERVICE_T;

typedef struct {
   int handle;
   uint32_t count;
   const VCHIQ_ELEMENT_T *elements;
} VCHIQ_QUEUE_MESSAGE_T;

typedef struct {
   int handle;
   const void *data;
   int size;
   void *userdata;
} VCHIQ_QUEUE_BULK_TRANSMIT_T;

typedef struct {
   int handle;
   void *data;
   int size;
   void *userdata;
} VCHIQ_QUEUE_BULK_RECEIVE_T;

typedef struct {
   VCHIQ_REASON_T reason;
   VCHIQ_HEADER_T *header;
   void *service_userdata;
   void *bulk_userdata;
} VCHIQ_COMPLETION_DATA_T;

typedef struct {
   int count;
   VCHIQ_COMPLETION_DATA_T *buf;
} VCHIQ_AWAIT_COMPLETION_T;

typedef struct {
   int handle;
   VCHIQ_HEADER_T *header;
} VCHIQ_RELEASE_MESSAGE_T;

#define VCHIQ_IOC_CONNECT              _IO(VCHIQ_IOC_MAGIC,   0)
#define VCHIQ_IOC_SHUTDOWN             _IO(VCHIQ_IOC_MAGIC,   1)
#define VCHIQ_IOC_ADD_SERVICE          _IOWR(VCHIQ_IOC_MAGIC, 2, VCHIQ_ADD_SERVICE_T)
#define VCHIQ_IOC_OPEN_SERVICE         _IOWR(VCHIQ_IOC_MAGIC, 3, VCHIQ_OPEN_SERVICE_T)
#define VCHIQ_IOC_REMOVE_SERVICE       _IO(VCHIQ_IOC_MAGIC,   4)
#define VCHIQ_IOC_QUEUE_MESSAGE        _IOW(VCHIQ_IOC_MAGIC,  5, VCHIQ_QUEUE_MESSAGE_T)
#define VCHIQ_IOC_QUEUE_BULK_TRANSMIT  _IOW(VCHIQ_IOC_MAGIC,  6, VCHIQ_QUEUE_BULK_TRANSMIT_T)
#define VCHIQ_IOC_QUEUE_BULK_RECEIVE   _IOW(VCHIQ_IOC_MAGIC,  7, VCHIQ_QUEUE_BULK_RECEIVE_T)
#define VCHIQ_IOC_AWAIT_COMPLETION     _IOW(VCHIQ_IOC_MAGIC,  8, VCHIQ_AWAIT_COMPLETION_T)
#define VCHIQ_IOC_RELEASE_MESSAGE      _IOW(VCHIQ_IOC_MAGIC,  9, VCHIQ_RELEASE_MESSAGE_T)
#define VCHIQ_IOC_MAX                  9

#endif
