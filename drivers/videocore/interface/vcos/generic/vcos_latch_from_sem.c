/*=============================================================================
Copyright (c) 2009 Broadcom Europe Limited.
All rights reserved.

Project  :  vcfw
Module   :  vcos

FILE DESCRIPTION
VideoCore OS Abstraction Layer - Construct a mutex from a semaphore
=============================================================================*/

/** FIXME: rename to vcos_mutex_from_sem.c
  */

#include "interface/vcos/vcos_mutex.h"
#include "interface/vcos/vcos_semaphore.h"
#include "interface/vcos/vcos_thread.h"

VCOS_STATUS_T vcos_generic_mutex_create(VCOS_MUTEX_T *latch, const char *name)
{
   VCOS_STATUS_T st = vcos_semaphore_create(&latch->sem, name, 1);
   latch->owner = 0;
   return st;
}

void vcos_generic_mutex_delete(VCOS_MUTEX_T *latch)
{
   vcos_assert(latch->owner == 0);
   vcos_semaphore_delete(&latch->sem);
}

VCOS_STATUS_T vcos_generic_mutex_lock(VCOS_MUTEX_T *latch)
{
   VCOS_STATUS_T st;
   /* Non re-entrant latch - check for attempt to retake. */
   vcos_assert(latch->owner != vcos_thread_current());

   st = vcos_semaphore_wait(&latch->sem);
   if (st == VCOS_SUCCESS)
   {
      vcos_assert(latch->owner == 0);
#ifndef NDEBUG
      latch->owner = vcos_thread_current();
#endif
   }
   return st;
}

void vcos_generic_mutex_unlock(VCOS_MUTEX_T *latch)
{
   vcos_assert(latch->owner == vcos_thread_current());
#ifndef NDEBUG
   latch->owner = 0;
#endif
   vcos_semaphore_post(&latch->sem);
}

