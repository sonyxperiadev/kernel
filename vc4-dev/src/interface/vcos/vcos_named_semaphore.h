/*=============================================================================
Copyright (c) 2009 Broadcom Europe Limited.
All rights reserved.

Project  :  vcfw
Module   :  chip driver
File     :  $RCSfile: $
Revision :  $Revision: $

FILE DESCRIPTION
VideoCore OS Abstraction Layer - named semaphores
=============================================================================*/

#ifndef VCOS_NAMED_SEMAPHORE_H
#define VCOS_NAMED_SEMAPHORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "interface/vcos/vcos_types.h"
#include "vcos_platform.h"

/**
 * \file
 *
 * Create a named semaphore, or open an existing one by name.
 *
 */

/** 
  * \brief Create a named semaphore.
  *
  * Semaphores are not re-entrant.
  *
  * @param sem   Pointer to memory to be initialized
  * @param name  A name for this semaphore.
  * @param count The initial count for the semaphore.
  *
  * @return VCOS_SUCCESS if the semaphore was created.
  * 
  */
VCOS_INLINE_DECL
VCOS_STATUS_T vcos_named_semaphore_create(VCOS_NAMED_SEMAPHORE_T *sem, const char *name, VCOS_UNSIGNED count);

/**
  * \brief Wait on a named semaphore.
  *
  * There is no timeout option on a semaphore, as adding this will slow down
  * implementations on some platforms. If you need that kind of behaviour, use
  * an event group.
  *
  * This always returns VCOS_SUCCESS and so should really be a void function. However
  * too many lines of code would need to be changed in non-trivial ways, so for now
  * it is non-void.
  *
  * @param sem Semaphore to wait on
  * @return VCOS_SUCCESS - semaphore was taken.
  *
  */
VCOS_INLINE_DECL
void vcos_named_semaphore_wait(VCOS_NAMED_SEMAPHORE_T *sem);

/**
  * \brief Try to wait for a semaphore.
  *
  * Try to obtain the semaphore. If it is already taken, return VCOS_TIMEOUT.
  * @param sem Semaphore to wait on
  * @return VCOS_SUCCESS - semaphore was taken.
  *         VCOS_EAGAIN - could not take semaphore
  */
VCOS_INLINE_DECL
VCOS_STATUS_T vcos_named_semaphore_trywait(VCOS_NAMED_SEMAPHORE_T *sem);

/**
  * \brief Post a semaphore.
  *
  * @param sem Semaphore to wait on
  */
VCOS_INLINE_DECL
void vcos_named_semaphore_post(VCOS_NAMED_SEMAPHORE_T *sem);

/**
  * \brief Delete a semaphore, releasing any resources consumed by it.
  *
  * @param sem Semaphore to wait on
  */
void vcos_named_semaphore_delete(VCOS_NAMED_SEMAPHORE_T *sem);


#ifdef __cplusplus
}
#endif
#endif

