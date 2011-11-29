/*=============================================================================
Copyright (c) 2009 Broadcom Europe Limited.
All rights reserved.

Project  :  vcfw
Module   :  chip driver

FILE DESCRIPTION
VideoCore OS Abstraction Layer - generic thread local storage
=============================================================================*/

#include "interface/vcos/vcos.h"
#include <stddef.h> /* For NULL */
#include <string.h>

/** Which TLS slots are in use? */
static unsigned long slot_map;

static VCOS_MUTEX_T lock;
static int inited;

VCOS_STATUS_T vcos_tls_init(void)
{
   slot_map = 0;
   vcos_assert(!inited);
   inited = 1;
   return vcos_mutex_create(&lock, "vcostls");
}

void vcos_tls_deinit(void)
{
   vcos_assert(inited);
   inited = 0;
   vcos_mutex_delete(&lock);
}

VCOS_STATUS_T vcos_generic_tls_create(VCOS_TLS_KEY_T *key)
{
   int i;
   VCOS_STATUS_T ret = VCOS_ENOSPC;

   vcos_assert(inited);

   vcos_mutex_lock(&lock);
   for (i=0; i<VCOS_TLS_MAX_SLOTS; i++)
   {
      if ((slot_map & (1<<i)) == 0)
      {
         slot_map |= (1<<i);
         ret = VCOS_SUCCESS;
         *key = i;
         break;
      }
   }
   vcos_mutex_unlock(&lock);
   return ret;
}

void vcos_generic_tls_delete(VCOS_TLS_KEY_T tls)
{
   vcos_mutex_lock(&lock);
   vcos_assert(slot_map & (1<<tls));
   slot_map &= ~(1<<tls);
   vcos_mutex_unlock(&lock);
}


void vcos_tls_thread_register(VCOS_TLS_THREAD_T *tls)
{
   memset(tls, 0, sizeof(*tls));
   _vcos_tls_thread_ptr_set(tls);
}




