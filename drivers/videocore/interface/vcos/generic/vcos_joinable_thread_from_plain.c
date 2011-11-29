/*=============================================================================
Copyright (c) 2009 Broadcom Europe Limited.
All rights reserved.

Project  :  vcfw
Module   :  vcos

FILE DESCRIPTION
VideoCore OS Abstraction Layer - joinable thread from plain
=============================================================================*/


#include "interface/vcos/vcos.h"

#include "interface/vcos/vcos_thread.h"
#include "interface/vcos/vcos_mem.h"
#include "interface/vcos/generic/vcos_joinable_thread_from_plain.h"
#include "interface/vcos/vcos_lowlevel_thread.h"
#include <stddef.h>
#include <string.h>

#ifndef VCOS_DEFAULT_STACK_SIZE
#define VCOS_DEFAULT_STACK_SIZE 4096
#endif

#ifndef VCOS_TIMESLICE_DEFAULT
#define VCOS_TIMESLICE_DEFAULT   10
#endif

typedef void (*LEGACY_ENTRY_FN_T)(int, void *);

/* Not every platform needs an internal per-thread timer. Only
 * create it if needed.
 */
#ifdef _VCOS_THREAD_TIMER_UNUSED
#define CREATE_THREAD_TIMER 0
#else
#define CREATE_THREAD_TIMER 1
#endif

static void thread_timer_func(void *cxt);

/**
  * \file
  *
  * Create a joinable thread using the classic Nucleus/ThreadX style
  * non-joinable thread.
  */

/** Default attributes
  */
static VCOS_THREAD_ATTR_T default_attrs = {
   0,
   VCOS_DEFAULT_STACK_SIZE,
   VCOS_THREAD_PRI_DEFAULT,
   VCOS_AFFINITY_DEFAULT,
   VCOS_TIMESLICE_DEFAULT,
   0, /* legacy */
   1, /* autostart */

};

void vcos_thread_attr_init(VCOS_THREAD_ATTR_T *attrs)
{
   *attrs = default_attrs;
}

/** Wrapper function around the real thread function. Posts the semaphore
  * when completed.
  */
static void vcos_thread_wrapper(void *arg)
{
   int i;
   void *ret;
   VCOS_THREAD_T *thread = (VCOS_THREAD_T *)arg;

   vcos_assert(thread->magic == VCOS_THREAD_MAGIC);

#ifdef VCOS_WANT_TLS_EMULATION
   vcos_tls_thread_register(&thread->_tls);
#endif

   if (thread->legacy)
   {
      LEGACY_ENTRY_FN_T fn = (LEGACY_ENTRY_FN_T)thread->entry;
      fn(0,thread->arg);
      ret = 0;
   }
   else
   {
      ret = thread->entry(thread->arg);
   }

   /* call termination functions */
   for (i=0; i<VCOS_MAX_EXIT_HANDLERS; i++)
   {
      if (thread->at_exit[i].pfn) {
         thread->at_exit[i].pfn(thread->at_exit[i].cxt);
      }
   }

   thread->exit_data = ret;
   vcos_semaphore_post(&thread->wait);
}

static VCOS_STATUS_T create_base(VCOS_THREAD_T *thread, const char *name)
{
   VCOS_STATUS_T status;

   thread->magic  = VCOS_THREAD_MAGIC;
   thread->legacy = 0;
   thread->joined = 0;
   thread->legacy = 0;
   thread->stack = 0;
   thread->next = NULL;
#ifdef VCOS_WANT_TLS_EMULATION
   memset(&thread->_tls, 0, sizeof(thread->_tls));
#endif
   memset(thread->at_exit, 0, sizeof(thread->at_exit));


   if (!name)
   {
      vcos_assert(0);
      return VCOS_EINVAL;
   }

   strncpy(thread->name, name, sizeof(thread->name));
   thread->name[sizeof(thread->name)-1] = '\0';

   status = vcos_semaphore_create(&thread->wait, thread->name, 0);
   if (status != VCOS_SUCCESS)
      goto fail_wait_sem;

   status = vcos_semaphore_create(&thread->suspend, thread->name, 0);
   if (status != VCOS_SUCCESS)
      goto fail_suspend_sem;

   thread->_timer.pfn = NULL;
   thread->_timer.cxt = NULL;

   if (CREATE_THREAD_TIMER)
   {
      status = vcos_timer_create(&thread->_timer.timer, thread->name, thread_timer_func, thread);
      if (status != VCOS_SUCCESS)
         goto fail_timer;
   }
   else
   {
      memset(&thread->_timer, 0, sizeof(thread->_timer));
   }

   return VCOS_SUCCESS;

fail_timer:
   vcos_semaphore_delete(&thread->suspend);
fail_suspend_sem:
   vcos_semaphore_delete(&thread->wait);
fail_wait_sem:
   return status;
}

static void delete_base(VCOS_THREAD_T *thread)
{
   if (CREATE_THREAD_TIMER)
      vcos_timer_delete(&thread->_timer.timer);
   vcos_semaphore_delete(&thread->suspend);
   vcos_semaphore_delete(&thread->wait);
}

VCOS_STATUS_T _vcos_thread_create_attach(VCOS_THREAD_T *thread,
                                         const char *name)
{
   VCOS_STATUS_T status;

   status = create_base(thread, name);
   if (status != VCOS_SUCCESS)
      goto fail_base;

   status = _vcos_llthread_create_attach(&thread->thread);
   if (status != VCOS_SUCCESS)
      goto fail_llthread;

#ifdef VCOS_WANT_TLS_EMULATION
   vcos_tls_thread_register(&thread->_tls);
#endif

   return VCOS_SUCCESS;

fail_llthread:
   delete_base(thread);
fail_base:
   return status;
}

VCOS_STATUS_T vcos_thread_create(VCOS_THREAD_T *thread,
                                 const char *name,
                                 VCOS_THREAD_ATTR_T *attrs,
                                 void *(*entry)(void *arg),
                                 void *arg)
{
   VCOS_STATUS_T status;

   void *stack, *allocstack = 0;

   /* If no attributes, use the defaults */
   if (!attrs)
      attrs = &default_attrs;

   status = create_base(thread, name);
   if (status != VCOS_SUCCESS)
      goto fail_base;

   /* Do we need to create a stack? */
   stack = attrs->ta_stackaddr;

   if (stack && !VCOS_CAN_SET_STACK_ADDR)
   {
      status = VCOS_EINVAL;
      goto fail_stack;
   }

   if (!stack && VCOS_CAN_SET_STACK_ADDR)
   {
      allocstack = vcos_malloc(attrs->ta_stacksz,thread->name);
      if (!allocstack)
      {
         status = VCOS_ENOMEM;
         goto fail_stack;
      }
      stack = allocstack;
      thread->stack = stack;
   }

   thread->legacy = attrs->legacy;

   thread->entry = entry;
   thread->arg = arg;

   status = vcos_llthread_create(&thread->thread, thread->name, vcos_thread_wrapper, thread,
                                 stack, attrs->ta_stacksz,
                                 attrs->ta_priority, attrs->ta_affinity,
                                 attrs->ta_timeslice, attrs->ta_autostart);
   if (status != VCOS_SUCCESS)
      goto fail_thread;

   return VCOS_SUCCESS;

fail_thread:
   if (allocstack)
      vcos_free(allocstack);
fail_stack:
   delete_base(thread);
fail_base:
   return status;
}

#ifndef NDEBUG
/* Compile time check that thread structure offsets are as expected */
int vcos_thread_structure_wrong[offsetof(VCOS_THREAD_T,thread) == 0 ? 1 : -1];
#endif

/** Exit the thread. Resources must still be cleaned up via a call to thread_join().
  */
void vcos_thread_exit(void *data)
{

   /* Get the current task pointer, and cast to the joinable thread
    * structure - this works with Nucleus and ThreadX style threads
    * which is what this is designed for, but may not work with
    * other thread types.
    */

   VCOS_THREAD_T *thread = vcos_thread_current();

   vcos_assert(thread);
   vcos_assert(thread->magic == VCOS_THREAD_MAGIC);

   thread->exit_data = data;

   /* Wake up our parent */
   vcos_semaphore_post(&thread->wait);

   vcos_llthread_exit();
}

void _vcos_thread_delete(VCOS_THREAD_T *thread)
{
   vcos_assert(thread);
   vcos_assert(thread->magic == VCOS_THREAD_MAGIC);

   vcos_llthread_delete(&thread->thread);

   if (thread->stack)
      vcos_free(thread->stack);

   delete_base(thread);
}

/** Wait for a thread to terminate and then clean up its resources.
  */
void vcos_thread_join(VCOS_THREAD_T *thread,
                      void **pData)
{
   vcos_assert(thread);
   vcos_assert(thread->magic == VCOS_THREAD_MAGIC);
   vcos_assert(!thread->joined); /* can't join more than once! */

   vcos_semaphore_wait(&thread->wait);

   thread->joined = 1;
   if (pData)
   {
      *pData = thread->exit_data;
   }

   /* Clean up */
   _vcos_thread_delete(thread);
}

VCOS_STATUS_T vcos_thread_create_classic(VCOS_THREAD_T *thread,
                                         const char *name,
                                         void *(*entry)(void* arg),
                                         void *arg,
                                         void *stack,
                                         VCOS_UNSIGNED stacksz,
                                         VCOS_UNSIGNED priaff,
                                         VCOS_UNSIGNED timeslice,
                                         VCOS_UNSIGNED autostart)
{
   VCOS_STATUS_T status;
   VCOS_THREAD_ATTR_T attrs;
   VCOS_UNSIGNED priority = priaff & ~VCOS_AFFINITY_MASK;
   VCOS_UNSIGNED affinity = priaff & VCOS_AFFINITY_MASK;

   vcos_thread_attr_init(&attrs);
#if VCOS_CAN_SET_STACK_ADDR
   vcos_thread_attr_setstack(&attrs, stack, stacksz);
#else
   vcos_assert(stack == 0);
   (void)stack;
#endif

   (void)stacksz;

   vcos_assert((autostart == VCOS_START) || (autostart == VCOS_NO_START));

   vcos_thread_attr_setpriority(&attrs, priority);
   vcos_thread_attr_setaffinity(&attrs, affinity);
   vcos_thread_attr_settimeslice(&attrs, timeslice);
   vcos_thread_attr_setautostart(&attrs, autostart);

   status = vcos_thread_create(thread, name, &attrs, entry, arg);

   return status;
}

void _vcos_task_timer_set(void (*pfn)(void*), void *cxt, VCOS_UNSIGNED ms)
{
   VCOS_THREAD_T *self = vcos_thread_current();
   vcos_assert(self);
   vcos_assert(self->_timer.pfn == NULL);
   self->_timer.pfn = pfn;
   self->_timer.cxt = cxt;
   vcos_timer_set(&self->_timer.timer, ms);
}

void _vcos_task_timer_cancel(void)
{
   VCOS_THREAD_T *self = vcos_thread_current();
   if (self->_timer.pfn)
   {
      vcos_timer_cancel(&self->_timer.timer);
      self->_timer.pfn = NULL;
   }
}

static void thread_timer_func(void *cxt)
{
   VCOS_THREAD_T *self = (VCOS_THREAD_T *)cxt;
   void (*pfn)(void*) = self->_timer.pfn;

   vcos_assert(self);
   vcos_assert(self->magic == VCOS_THREAD_MAGIC);
   vcos_assert(self->_timer.pfn);
   self->_timer.pfn = NULL;
   pfn(self->_timer.cxt);
}

const char *vcos_thread_get_name(const VCOS_THREAD_T *thread)
{
   vcos_assert(thread);
   return thread->name;
}

extern VCOS_STATUS_T vcos_thread_at_exit(void (*pfn)(void*), void *cxt)
{
   int i;
   VCOS_THREAD_T *self = vcos_thread_current();
   if (!self)
   {
      vcos_assert(0);
      return VCOS_EINVAL;
   }
   vcos_assert(self->joined == 0);
   for (i=0; i<VCOS_MAX_EXIT_HANDLERS; i++)
   {
      if (self->at_exit[i].pfn == NULL)
      {
         self->at_exit[i].pfn = pfn;
         self->at_exit[i].cxt = cxt;
         return VCOS_SUCCESS;
      }
   }
   return VCOS_ENOSPC;
}

extern void vcos_thread_deregister_at_exit(void (*pfn)(void*), void *cxt)
{
   int i;
   VCOS_THREAD_T *self = vcos_thread_current();
   vcos_assert(self);
   vcos_assert(!self->joined);
   for (i=0; i<VCOS_MAX_EXIT_HANDLERS; i++)
   {
      if ((self->at_exit[i].pfn == pfn) && (self->at_exit[i].cxt == cxt)) {
         self->at_exit[i].pfn = NULL;
         return;
      }
   }
   vcos_assert(0); /* pfn/cxt not found */
}
