/*=============================================================================
Copyright (c) 2009 Broadcom Europe Limited.
All rights reserved.

Project  :  vcfw
Module   :  chip driver
File     :  $RCSfile: $
Revision :  $Revision: $

FILE DESCRIPTION
VideoCore OS Abstraction Layer - reentrant mutex public header file
=============================================================================*/

#ifndef VCOS_REENTRANT_MUTEX_H
#define VCOS_REENTRANT_MUTEX_H

#ifdef __cplusplus
extern "C" {
#endif

#include "interface/vcos/vcos_types.h"
#include "vcos_platform.h"

/**
 * \file
 *
 * Reentrant Mutex API. You can take one of these mutexes even if you've already
 * taken it. Just to make sure.
 *
 * A re-entrant mutex may be slower on some platforms than a regular one.
 *
 * \sa vcos_mutex.h
 *
 */

/** Create a mutex.
  *
  * @param m      Filled in with mutex on return
  * @param name   A non-null name for the mutex, used for diagnostics.
  *
  * @return VCOS_SUCCESS if mutex was created, or error code.
  */
VCOS_INLINE_DECL
VCOS_STATUS_T vcos_reentrant_mutex_create(VCOS_REENTRANT_MUTEX_T *m, const char *name);

/** Delete the mutex.
  */
VCOS_INLINE_DECL
void vcos_reentrant_mutex_delete(VCOS_REENTRANT_MUTEX_T *m);

/** Wait to claim the mutex. Must not have already been claimed by the current thread.
  */
#ifndef vcos_reentrant_mutexlock
VCOS_INLINE_DECL
void vcos_reentrant_mutex_lock(VCOS_REENTRANT_MUTEX_T *m);

/** Release the mutex.
  */
VCOS_INLINE_DECL
void vcos_reentrant_mutex_unlock(VCOS_REENTRANT_MUTEX_T *m);
#endif


#ifdef __cplusplus
}
#endif
#endif

