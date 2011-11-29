/*=============================================================================
Copyright (c) 2009 Broadcom Europe Limited.
All rights reserved.

Project  :  vcfw
Module   :  chip driver

FILE DESCRIPTION
VideoCore OS Abstraction Layer - IRQ support
=============================================================================*/

#ifndef VCOS_ISR_H
#define VCOS_ISR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "interface/vcos/vcos_types.h"
#include "vcos_platform.h"

/**
  * \file vcos_isr.h
  *
  * \section isr ISR support
  *
  * API for dispatching interrupts.
  */

/**
  *
  * Register an interrupt handler. The old handler (if any) is returned in
  * old_handler. The old handler should be called if the interrupt was not
  * for you.
  *
  * The handler function will be called in a context with interrupts disabled,
  * so should be written to be as short as possible. If significant processing
  * is needed, the handler should delegate to a thread.
  *
  * The handler function can call any OS primitive that does not block (e.g.
  * post a semaphore or set an event flag). Blocking operations (including memory
  * allocation from the system heap) are not permitted.
  *
  * To deregister an ISR, pass in NULL.
  *
  * @param vec  Vector to register for
  * @param handler Handler to be called
  * @param old_handler Updated with the old handler, or NULL.
  */

VCOS_INLINE_DECL
void vcos_register_isr(VCOS_UNSIGNED vec,
                       VCOS_ISR_HANDLER_T handler,
                       VCOS_ISR_HANDLER_T *old_handler);

/** Disable interrupts, returning the old value (enabled/disabled) to the caller.
  */
VCOS_INLINE_DECL
VCOS_UNSIGNED vcos_int_disable(void);

/** Restore the previous interrupt enable/disable state.
  */
VCOS_INLINE_DECL
void vcos_int_restore(VCOS_UNSIGNED previous);

#ifdef __cplusplus
}
#endif
#endif

