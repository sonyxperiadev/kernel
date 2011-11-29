/*=============================================================================
Copyright (c) 2010 Broadcom Europe Limited.
All rights reserved.

Project  :  vcfw
Module   :  vcos

FILE DESCRIPTION
VideoCore OS Abstraction Layer - thread reaping
=============================================================================*/

/**
  * @file vcos_thread_reaper.c
  *
  * Implements a 'thread reaper' for clients who can't just do a join.
  *
  * A thread sits on a linked list and does the vcos_thread_join() on
  * behalf of the client.
  *
  * After the thread has been joined, a callback is invoked, which the
  * client can use to do things like freeing memory used for the thread.
  *
  * The thread entry point and argument are overloaded in order to store
  * the callback.
  *
  * The worker thread has a linked list of threads. A special thread with a special
  * magic value is used to tell it to exit. This ensures everything is nice
  * and thread safe.
  *
  */

#include "interface/vcos/vcos.h"
#include "interface/vcos/generic/vcos_thread_reaper.h"

#define VCOS_THREAD_REAPER_MAGIC 'REPA'

/** Our own reaper thread */
static VCOS_THREAD_T reaper;

/** Is our reaper thread running? */
static int reaper_running;

/** Reaper thread waits on this */
static VCOS_SEMAPHORE_T reaper_sem;

/** Protect linked list */
static VCOS_MUTEX_T lock;

/** List of threads being reaped */
static VCOS_THREAD_T *head;
static VCOS_THREAD_T *tail;

static void *reaper_entry(void*);

VCOS_STATUS_T vcos_thread_reaper_init(void)
{
   VCOS_STATUS_T st;

   vcos_assert(!reaper_running);
   reaper_running = 1;

   st = vcos_semaphore_create(&reaper_sem, "reaper_sem", 0);
   if (st != VCOS_SUCCESS)
      return st;

   st = vcos_mutex_create(&lock, "reaper_lock");
   if (st != VCOS_SUCCESS)
   {
      vcos_semaphore_delete(&reaper_sem);
      return st;
   }

   st = vcos_thread_create(&reaper, "reaper",
                           NULL, reaper_entry, NULL);

   if (st != VCOS_SUCCESS)
   {
      vcos_semaphore_delete(&reaper_sem);
      vcos_mutex_delete(&lock);
      return st;
   }

   return st;
}

typedef void *(*thread_entry_t)(void*);

void vcos_thread_reap(VCOS_THREAD_T *thread, void (*on_terminated)(void*), void *arg)
{
   vcos_assert(thread->magic == VCOS_THREAD_MAGIC);
   vcos_assert(thread->next == NULL);

   vcos_assert(reaper_running);

   /* append this thread to the list */
   vcos_mutex_lock(&lock);
   thread->next = NULL;
   thread->term = (thread_entry_t)on_terminated;
   thread->term_arg = arg;
   if (!head)
   {
      head = tail = thread;
   }
   else
   {
      tail->next = thread;
      tail = thread;
   }
   vcos_mutex_unlock(&lock);

   /* signal the worker thread */
   vcos_semaphore_post(&reaper_sem);
}

static void *reaper_entry(void* arg)
{
   while (1)
   {
      VCOS_THREAD_T *t;
      vcos_semaphore_wait(&reaper_sem);

      vcos_log("reaper: woken");
      vcos_mutex_lock(&lock);

      vcos_assert(head); /* semaphore/linked list mismatch! */

      t = head;
      head = head->next;
      if (head == NULL)
      {
         tail = NULL;
      }
      vcos_mutex_unlock(&lock);
      vcos_log("reaper: got %p", t);

      if (t->magic == VCOS_THREAD_REAPER_MAGIC)
      {
         vcos_log("reaper: terminating");
         return NULL;
      }
      else if (t->magic == VCOS_THREAD_MAGIC)
      {
         vcos_log("Joining %p", t);
         vcos_thread_join(t,NULL);
         vcos_log("Done it");
         if (t->term)
         {
            t->term(t->term_arg);
         }
         vcos_log("called callback");
      }
      else
      {
         vcos_assert(0);
      }
   }
}





