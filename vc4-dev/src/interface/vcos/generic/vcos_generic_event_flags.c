/*=============================================================================
Copyright (c) 2009 Broadcom Europe Limited.
All rights reserved.

FILE DESCRIPTION
VideoCore OS Abstraction Layer - event flags implemented via mutexes
=============================================================================*/

#include <stddef.h>

#include "interface/vcos/vcos.h"
#include "interface/vcos/generic/vcos_generic_event_flags.h"

/** A structure created by a thread that waits on the event flags
  * for a particular combination of flags to arrive.
  */
typedef struct VCOS_EVENT_WAITER_T
{
   VCOS_UNSIGNED requested_events;  /**< The events wanted */
   VCOS_UNSIGNED actual_events;     /**< Actual events found */
   VCOS_UNSIGNED op;                /**< The event operation to be used */
   VCOS_STATUS_T return_status;     /**< The return status the waiter should pass back */
   VCOS_EVENT_FLAGS_T *flags;       /**< Pointer to the original 'flags' structure */
   VCOS_THREAD_T *thread;           /**< Thread waiting */
   struct VCOS_EVENT_WAITER_T *next;
   unsigned long magic;             /**< For detecting racy POSIX timers, see SW-6307 */
} VCOS_EVENT_WAITER_T;

#define WAITREQ_MAGIC   0x57414954

#ifndef NDEBUG
static int waiter_list_valid(VCOS_EVENT_FLAGS_T *flags);
#endif
static void event_flags_timer_expired(void *cxt);

VCOS_STATUS_T vcos_generic_event_flags_create(VCOS_EVENT_FLAGS_T *flags, const char *name)
{
   VCOS_STATUS_T rc;
   if ((rc=vcos_mutex_create(&flags->lock, name)) != VCOS_SUCCESS)
   {
      return rc;
   }

   flags->events = 0;
   flags->waiters.head = flags->waiters.tail = 0;
   return rc;
}

void vcos_generic_event_flags_set(VCOS_EVENT_FLAGS_T *flags,
                                  VCOS_UNSIGNED bitmask,
                                  VCOS_OPTION op)
{
   vcos_assert(flags);
   vcos_mutex_lock(&flags->lock);
   if (op == VCOS_OR)
   {
      flags->events |= bitmask;
   }
   else if (op == VCOS_AND)
   {
      flags->events &= bitmask;
   }
   else
   {
      vcos_assert(0);
   }

   /* Now wake up any threads that have now become signalled. */
   if (flags->waiters.head != NULL)
   {
      VCOS_UNSIGNED consume = 0;
      VCOS_EVENT_WAITER_T **psuspend_ptr = &flags->waiters.head;


      /* Walk the chain of tasks suspend on this event flag group to determin
       * if any of their requests can be satisfied.
       */
      while ((*psuspend_ptr) != NULL)
      {
         VCOS_EVENT_WAITER_T **pnext, *suspend_ptr = *psuspend_ptr;

         /* Determine if this request has been satisfied */

         /* First, find the event flags in common. */
         VCOS_UNSIGNED compare = flags->events & suspend_ptr->requested_events;

         /* Second, determine if all the event flags must match */
         if (suspend_ptr->op & VCOS_AND)
         {
            /* All requested events must be present */
            compare = (compare == suspend_ptr->requested_events);
         }
         /* Find next item in list before we wake up this one, as the suspend block
          * will no longer be valid once it starts running.
          */
         pnext = &(suspend_ptr->next);

         /* Wake this one up? */
         if (compare)
         {
            int at_end;

            if (suspend_ptr->op & VCOS_CONSUME)
            {
               consume |= suspend_ptr->requested_events;
            }
            /* remove this block from the list, taking care at the end */
            at_end = suspend_ptr->next == NULL;
            *psuspend_ptr = suspend_ptr->next;
            if (at_end)
               flags->waiters.tail = flags->waiters.head;

            vcos_assert(waiter_list_valid(flags));

            suspend_ptr->return_status = VCOS_SUCCESS;
            suspend_ptr->actual_events = flags->events;

            _vcos_thread_sem_post(suspend_ptr->thread);
         }
         else
         {
            /* move to next element in the list */
            psuspend_ptr = pnext;
         }
      }

      flags->events &= ~consume;

   }

   vcos_mutex_unlock(&flags->lock);
}

void vcos_generic_event_flags_delete(VCOS_EVENT_FLAGS_T *flags)
{
   vcos_mutex_delete(&flags->lock);
}

extern VCOS_STATUS_T vcos_generic_event_flags_get(VCOS_EVENT_FLAGS_T *flags,
                                                  VCOS_UNSIGNED bitmask,
                                                  VCOS_OPTION op,
                                                  VCOS_UNSIGNED suspend,
                                                  VCOS_UNSIGNED *retrieved_bits)
{
   VCOS_EVENT_WAITER_T waitreq;
   VCOS_STATUS_T rc = VCOS_EAGAIN;
   int satisfied = 0;

   vcos_assert(flags);

   /* default retrieved bits to 0 */
   *retrieved_bits = 0;

   vcos_mutex_lock(&flags->lock);
   switch (op & VCOS_EVENT_FLAG_OP_MASK)
   {
   case VCOS_AND:
      if ((flags->events & bitmask) == bitmask)
      {
         *retrieved_bits = flags->events;
         rc = VCOS_SUCCESS;
         satisfied = 1;
         if (op & VCOS_CONSUME)
            flags->events &= ~bitmask;
      }
      break;

   case VCOS_OR:
      if (flags->events & bitmask)
      {
         *retrieved_bits = flags->events;
         rc = VCOS_SUCCESS;
         satisfied = 1;
         if (op & VCOS_CONSUME)
            flags->events &= ~bitmask;
      }
      break;

   default:
      vcos_assert(0);
      rc = VCOS_EINVAL;
      break;
   }

   if (!satisfied && suspend)
   {
      /* Have to go to sleep.
       *
       * Append to tail so we get FIFO ordering.
       */
      waitreq.requested_events = bitmask;
      waitreq.op = op;
      waitreq.return_status = VCOS_EAGAIN;
      waitreq.flags = flags;
      waitreq.actual_events = 0;
      waitreq.thread = vcos_thread_current();
      waitreq.next = 0;
      waitreq.magic = WAITREQ_MAGIC;
      vcos_assert(waitreq.thread != (VCOS_THREAD_T*)-1);
      VCOS_QUEUE_APPEND_TAIL(&flags->waiters, &waitreq);

      if (suspend != (VCOS_UNSIGNED)-1)
         _vcos_task_timer_set(event_flags_timer_expired, &waitreq, suspend);

      vcos_mutex_unlock(&flags->lock);
      /* go to sleep and wait to be signalled or timeout */

      _vcos_thread_sem_wait();

      *retrieved_bits = waitreq.actual_events;
      rc = waitreq.return_status;

      /* cancel the timer - do not do this while holding the mutex as it
       * might be waiting for the timeout function to complete, which will
       * try to take the mutex.
       */
      if (suspend != (VCOS_UNSIGNED)-1)
      {
#ifdef VCOS_TIMERS_UNSAFE
         waitreq.magic = ~waitreq.magic;
#endif
         _vcos_task_timer_cancel();
      }

   }
   else
   {
      vcos_mutex_unlock(&flags->lock);
   }

   return rc;
}


/** Called when a get call times out. Remove this thread's
  * entry from the waiting queue, then resume the thread.
  */
static void event_flags_timer_expired(void *cxt)
{
   VCOS_EVENT_WAITER_T *waitreq = (VCOS_EVENT_WAITER_T *)cxt;
   VCOS_EVENT_FLAGS_T *flags = waitreq->flags;
   VCOS_EVENT_WAITER_T **plist;
   VCOS_THREAD_T *thread = 0;

   vcos_assert(waitreq);

#ifdef VCOS_TIMERS_UNSAFE
   if (waitreq->magic != WAITREQ_MAGIC)
   {
      /* Timer was cancelled, but after the expiry function was called
       * by pthreads. There doesn't seem to be any way to detect this
       * race condition other than a somewhat clunky test here.
       */
      return;
   }
#endif

   vcos_assert(flags);
   vcos_mutex_lock(&flags->lock);

   /* walk the list of waiting threads on this event group, and remove
    * the one that has expired.
    *
    * FIXME: could use doubly-linked list if lots of threads are found
    * to be waiting on a single event flag instance.
    */
   plist = &flags->waiters.head;
   while (*plist != NULL)
   {
      if (*plist == waitreq)
      {
         int at_end;
         /* found it */
         thread = (*plist)->thread;
         at_end = ((*plist)->next == NULL);

         /* link past */
         *plist = (*plist)->next;
         if (at_end)
            flags->waiters.tail = flags->waiters.head;

         break;
      }
      plist = &(*plist)->next;
   }
   vcos_assert(waiter_list_valid(flags));

   vcos_mutex_unlock(&flags->lock);

   if (thread)
   {
      _vcos_thread_sem_post(thread);
   }
}

#ifndef NDEBUG

static int waiter_list_valid(VCOS_EVENT_FLAGS_T *flags)
{
   int valid;
   /* Either both head and tail are NULL, or neither are NULL */
   if (flags->waiters.head == NULL)
   {
      valid = (flags->waiters.tail == NULL);
   }
   else
   {
      valid = (flags->waiters.tail != NULL);
   }

   /* If head and tail point at the same non-NULL element, then there
    * is only one element in the list.
    */
   if (flags->waiters.head && (flags->waiters.head == flags->waiters.tail))
   {
      valid = (flags->waiters.head->next == NULL);
   }
   return valid;
}

#endif
