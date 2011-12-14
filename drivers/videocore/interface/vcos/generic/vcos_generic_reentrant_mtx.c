/*=============================================================================
Copyright (c) 2009 Broadcom Europe Limited.
All rights reserved.

Project  :  vcfw
Module   :  chip driver

FILE DESCRIPTION
VideoCore OS Abstraction Layer - reentrant mutexes created from regular ones.
=============================================================================*/

#include "interface/vcos/vcos.h"
#include "interface/vcos/vcos_reentrant_mutex.h"

VCOS_STATUS_T vcos_generic_reentrant_mutex_create(VCOS_REENTRANT_MUTEX_T *m, const char *name)
{
   m->count = 0;
   m->owner = 0;
   return vcos_mutex_create(&m->mutex, name);
}

void vcos_generic_reentrant_mutex_delete(VCOS_REENTRANT_MUTEX_T *m)
{
   vcos_assert(m->count == 0);
   vcos_mutex_delete(&m->mutex);
}

void vcos_generic_reentrant_mutex_lock(VCOS_REENTRANT_MUTEX_T *m)
{
   VCOS_THREAD_T *thread = vcos_thread_current();
   vcos_assert(m);

   vcos_assert(thread != 0);

   if (m->owner != thread)
   {
      vcos_mutex_lock(&m->mutex);
      m->owner = thread;
      vcos_assert(m->count == 0);
   }
   m->count++;
}

void vcos_generic_reentrant_mutex_unlock(VCOS_REENTRANT_MUTEX_T *m)
{
   vcos_assert(m->count != 0);
   vcos_assert(m->owner == vcos_thread_current());
   m->count--;
   if (m->count == 0)
   {
      m->owner = 0;
      vcos_mutex_unlock(&m->mutex);
   }
}






