/*=============================================================================
Copyright (c) 2009 Broadcom Europe Limited.
All rights reserved.

Project  :  vcfw
Module   :  chip driver

FILE DESCRIPTION
VideoCore OS Abstraction Layer - named semaphores
=============================================================================*/

#define VCOS_LOG_CATEGORY (&vcos_named_sem_log_cat)

#include "interface/vcos/vcos.h"
#include "interface/vcos/generic/vcos_generic_named_sem.h"
#include "interface/vcos/vcos_blockpool.h"

static VCOS_LOG_CAT_T vcos_named_sem_log_cat =
VCOS_LOG_INIT("vcos_named_sem", VCOS_LOG_ERROR);

/**
  * \file
  *
  * Named semaphores, primarily for VCFW.
  *
  * Does not actually work across processes; merely emulate the API.
  *
  * The client initialises a VCOS_NAMED_SEMAPHORE_T, but this merely
  * points at the real underlying VCIS_NAMED_SEMAPHORE_IMPL_T.
  *
  *      semaphore_t  ---\
  *                       ----- semaphore_impl_t
  *      semaphore_t  ---/
  *                     /
  *      semaphore_t  -/
  *
  */

/* Maintain a block pool of semaphore implementations */
#define NUM_SEMS  16
#define MAX_EXTENSIONS (VCOS_BLOCKPOOL_MAX_SUBPOOLS - 1)

/** Each actual real semaphore is stored in one of these. Clients just
  * get a structure with a pointer to this in it.
  *
  * It also contains a doubly linked list tracking the semaphores in-use.
  */
typedef struct VCOS_NAMED_SEMAPHORE_IMPL_T
{
   VCOS_SEMAPHORE_T sem;                     /**< Actual underlying semaphore */
   char name[VCOS_NAMED_SEMAPHORE_NAMELEN];  /**< Name of semaphore, copied */
   unsigned refs;                            /**< Reference count */
   struct VCOS_NAMED_SEMAPHORE_IMPL_T *next; /**< Next in the in-use list   */
   struct VCOS_NAMED_SEMAPHORE_IMPL_T *prev; /**< Previous in the in-use list */
} VCOS_NAMED_SEMAPHORE_IMPL_T;

static VCOS_MUTEX_T lock;
static VCOS_ONCE_T named_sem_once;
static VCOS_NAMED_SEMAPHORE_IMPL_T* sems_in_use = NULL;
static int sems_in_use_count = 0;
static int sems_total_ref_count = 0;

static VCOS_BLOCKPOOL_T sems_pool;
static char pool_mem[VCOS_BLOCKPOOL_SIZE(
      NUM_SEMS, sizeof(VCOS_NAMED_SEMAPHORE_IMPL_T))];

static VCOS_STATUS_T vcos_generic_named_semaphore_init(void)
{
   VCOS_STATUS_T status;

   status = vcos_blockpool_init(&sems_pool,
         NUM_SEMS, sizeof(VCOS_NAMED_SEMAPHORE_IMPL_T),
         pool_mem, sizeof(pool_mem), "vcos named semaphores");

   if (status != VCOS_SUCCESS)
      goto fail_blockpool;

   status = vcos_blockpool_extend(&sems_pool, MAX_EXTENSIONS, NUM_SEMS * 4);
   if (status != VCOS_SUCCESS)
      goto fail_extend;

   status = vcos_mutex_create(&lock, "vcosnmsem");
   if (status != VCOS_SUCCESS)
      goto fail_mutex;

   return status;

fail_mutex:
fail_extend:
   vcos_blockpool_delete(&sems_pool);
fail_blockpool:
   return status;
}

static void vcos_generic_named_semaphore_init_once(void)
{
   VCOS_STATUS_T status = vcos_generic_named_semaphore_init();
   vcos_assert(status == VCOS_SUCCESS);
   (void) status;
}

VCOS_STATUS_T _vcos_named_semaphore_init()
{
   vcos_once(&named_sem_once, vcos_generic_named_semaphore_init_once);
   return VCOS_SUCCESS;
}

void _vcos_named_semaphore_deinit(void)
{
   vcos_blockpool_delete(&sems_pool);
   vcos_mutex_delete(&lock);
   sems_in_use = NULL;
}

VCOS_STATUS_T
vcos_generic_named_semaphore_create(VCOS_NAMED_SEMAPHORE_T *sem,
      const char *name, VCOS_UNSIGNED count)
{
   VCOS_STATUS_T status = VCOS_ENOSPC;
   int name_len, cmp = -1;
   VCOS_NAMED_SEMAPHORE_IMPL_T *impl;
   VCOS_NAMED_SEMAPHORE_IMPL_T *new_impl;

   vcos_log_trace("%s: sem %p name %s count %d", __FUNCTION__,
         sem, (name ? name : "null"), count);

   vcos_assert(name);
   vcos_once(&named_sem_once, vcos_generic_named_semaphore_init_once);

   vcos_mutex_lock(&lock);
   name_len = vcos_strlen(name);
   if (name_len >= VCOS_NAMED_SEMAPHORE_NAMELEN)
   {
      vcos_assert(0);
      status = VCOS_EINVAL;
      goto end;
   }

   /* do we already have this semaphore? */
   impl = sems_in_use;
   while (impl && (cmp = vcos_strcmp(name, impl->name)) < 0)
      impl = impl->next;

   if (impl && cmp == 0)
   {
      /* Semaphore is already in use so just increase the ref count */
      impl->refs++;
      sems_total_ref_count++;
      sem->actual = impl;
      sem->sem = &impl->sem;
      status = VCOS_SUCCESS;
      vcos_log_trace(
            "%s: ref count %d name %s total refs %d num sems %d",
            __FUNCTION__, impl->refs, impl->name,
            sems_total_ref_count, sems_in_use_count);
      goto end;
   }

   /* search for unused semaphore */
   new_impl = vcos_blockpool_calloc(&sems_pool);
   if (new_impl)
   {
      status = vcos_semaphore_create(&new_impl->sem, name, count);
      if (status == VCOS_SUCCESS)
      {
         new_impl->refs = 1;
         sems_total_ref_count++;
         sems_in_use_count++;
         vcos_strcpy(new_impl->name, name); /* already checked length! */
         sem->actual = new_impl;
         sem->sem = &new_impl->sem;

         /* Insert into the sorted list
          * impl is either NULL or the first element where
          * name > impl->name.
          */
         if (impl)
         {
            new_impl->prev = impl->prev;
            impl->prev = new_impl;
            new_impl->next = impl;

            if (new_impl->prev)
               new_impl->prev->next = new_impl;
         }
         else
         {
            /* Appending to the tail of the list / empty list */
            VCOS_NAMED_SEMAPHORE_IMPL_T *tail = sems_in_use;
            while(tail && tail->next)
               tail = tail->next;

            if (tail)
            {
               tail->next = new_impl;
               new_impl->prev = tail;
            }
         }

         if (sems_in_use == impl)
         {
            /* Inserted at head or list was empty */
            sems_in_use = new_impl;
         }

      vcos_log_trace(
            "%s: new ref actual %p prev %p next %p count %d name %s " \
            "total refs %d num sems %d",
            __FUNCTION__,
            new_impl, new_impl->prev, new_impl->next,
            new_impl->refs, new_impl->name,
            sems_total_ref_count, sems_in_use_count);
      }
   }

end:
   vcos_mutex_unlock(&lock);
   if (status != VCOS_SUCCESS)
   {
      vcos_log_error("%s: failed to create named semaphore name %s status %d " \
            "total refs %d num sems %d",
            __FUNCTION__, (name ? name : "NULL"), status,
            sems_total_ref_count, sems_in_use_count);
   }
   return status;
}

void vcos_named_semaphore_delete(VCOS_NAMED_SEMAPHORE_T *sem)
{
   VCOS_NAMED_SEMAPHORE_IMPL_T *actual = sem->actual;
   vcos_mutex_lock(&lock);

   /* if this fires, the semaphore has already been deleted */
   vcos_assert(actual->refs);

   vcos_log_trace(
         "%s: actual %p ref count %d name %s prev %p next %p total refs %d num sems %d",
         __FUNCTION__, actual, actual->refs, actual->name,
         actual->prev, actual->next,
         sems_total_ref_count, sems_in_use_count);

   sems_total_ref_count--;
   if (--actual->refs == 0)
   {
      sems_in_use_count--;
      if (actual->prev)
         actual->prev->next = actual->next;

      if (actual->next)
         actual->next->prev = actual->prev;

      if (sems_in_use == actual)
         sems_in_use = actual->next;

      vcos_semaphore_delete(&actual->sem);
      sem->actual = NULL;
      sem->sem = NULL;
      vcos_blockpool_free(actual);
   }
   vcos_mutex_unlock(&lock);
}
