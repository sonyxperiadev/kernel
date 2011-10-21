/*=============================================================================
Copyright (c) 2010 Broadcom Europe Ltd. All rights reserved.

Project  :  ARM
Module   :  vchiq_arm

FILE DESCRIPTION:
Header file for utility functions for VCHIQ clients.

==============================================================================*/

#ifndef VCHIQ_UTIL_H
#define VCHIQ_UTIL_H

#include "vchiq_if.h"
#include "interface/vcos/vcos.h"

typedef struct {
   uint32_t size;
   uint32_t read;
   uint32_t write;

   VCOS_EVENT_T pop;
   VCOS_EVENT_T push;

   VCHIQ_HEADER_T **storage;
} VCHIU_QUEUE_T;

extern int  vchiu_queue_init(VCHIU_QUEUE_T *queue, uint32_t size);
extern void vchiu_queue_delete(VCHIU_QUEUE_T *queue);

extern int vchiu_queue_is_empty(VCHIU_QUEUE_T *queue);

extern void vchiu_queue_push(VCHIU_QUEUE_T *queue, VCHIQ_HEADER_T *header);

extern VCHIQ_HEADER_T *vchiu_queue_peek(VCHIU_QUEUE_T *queue);
extern VCHIQ_HEADER_T *vchiu_queue_pop(VCHIU_QUEUE_T *queue);

#endif

