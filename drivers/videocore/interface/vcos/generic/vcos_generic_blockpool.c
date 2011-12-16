/*=============================================================================
Copyright (c) 2011 Broadcom Europe Limited.
All rights reserved.

Project  : vcfw
Module   : chip driver

FILE DESCRIPTION
VideoCore OS Abstraction Layer - generic block pool library
=============================================================================*/

#define VCOS_LOG_CATEGORY (&vcos_blockpool_log)

#include <stddef.h>
#include <string.h>
#include "interface/vcos/vcos.h"
#include "interface/vcos/generic/vcos_generic_blockpool.h"

#define VCOS_BLOCKPOOL_FOURCC(a,b,c,d) ((a) | (b << 8) | (c << 16) | (d << 24))
#define VCOS_BLOCKPOOL_MAGIC           VCOS_BLOCKPOOL_FOURCC('v', 'b', 'p', 'l')
#define VCOS_BLOCKPOOL_SUBPOOL_MAGIC   VCOS_BLOCKPOOL_FOURCC('v', 's', 'p', 'l')

#define VCOS_BLOCKPOOL_SUBPOOL_FLAG_OWNS_MEM    (1 << 0)
#define VCOS_BLOCKPOOL_SUBPOOL_FLAG_EXTENSION   (1 << 1)

/* Whether to overwrite freed blocks with 0xBD */
#ifdef NDEBUG
#define VCOS_BLOCKPOOL_OVERWRITE_ON_FREE 0
#else
#define VCOS_BLOCKPOOL_OVERWRITE_ON_FREE 1
#endif

/* Uncomment to enable really verbose debug messages */
/* #define VCOS_BLOCKPOOL_DEBUGGING */

#ifdef VCOS_BLOCKPOOL_DEBUGGING
#define VCOS_BLOCKPOOL_ASSERT vcos_demand
#define VCOS_BLOCKPOOL_TRACE_LEVEL VCOS_LOG_TRACE
#define VCOS_BLOCKPOOL_DEBUG_LOG(s, ...) vcos_log_trace("%s: " s, VCOS_FUNCTION, __VA_ARGS__)
#undef  VCOS_BLOCKPOOL_OVERWRITE_ON_FREE
#define VCOS_BLOCKPOOL_OVERWRITE_ON_FREE 1
#else
#define VCOS_BLOCKPOOL_ASSERT vcos_demand
#define VCOS_BLOCKPOOL_TRACE_LEVEL VCOS_LOG_ERROR
#define VCOS_BLOCKPOOL_DEBUG_LOG(s, ...)
#endif

#define ASSERT_POOL(p) \
   VCOS_BLOCKPOOL_ASSERT((p) && (p)->magic == VCOS_BLOCKPOOL_MAGIC);

#define ASSERT_SUBPOOL(p) \
   VCOS_BLOCKPOOL_ASSERT((p) && (p)->magic == VCOS_BLOCKPOOL_SUBPOOL_MAGIC);

static VCOS_LOG_CAT_T vcos_blockpool_log =
VCOS_LOG_INIT("vcos_blockpool", VCOS_BLOCKPOOL_TRACE_LEVEL);

static void vcos_generic_blockpool_subpool_init(
      VCOS_BLOCKPOOL_T *pool, VCOS_BLOCKPOOL_SUBPOOL_T *subpool,
      void *start, size_t pool_size, VCOS_UNSIGNED num_blocks, uint32_t flags)
{
   VCOS_BLOCKPOOL_HEADER_T *block;
   VCOS_BLOCKPOOL_HEADER_T *end;

   (void)flags;

   vcos_log_trace(
         "%s: pool %p subpool %p start %p pool_size %d num_blocks %d flags %x",
         __FUNCTION__,
         pool, subpool, start, (uint32_t) pool_size, num_blocks, flags);

   subpool->magic = VCOS_BLOCKPOOL_SUBPOOL_MAGIC;
   subpool->mem = start;
   subpool->start = (void*)
      VCOS_BLOCKPOOL_ROUND_UP((unsigned long) start, sizeof(unsigned long));
   subpool->num_blocks = num_blocks;
   subpool->available_blocks = num_blocks;
   subpool->free_list = NULL;
   subpool->owner = pool;
   memset(subpool->mem, 0xBC, pool_size); /* For debugging */

   block = (VCOS_BLOCKPOOL_HEADER_T*) subpool->start;
   end = (VCOS_BLOCKPOOL_HEADER_T*)
      ((char *) subpool->start + (pool->block_size * num_blocks));

   /* Initialise the free list for this subpool */
   while (block < end)
   {
      block->owner.next = subpool->free_list;
      subpool->free_list = block;
      block = (VCOS_BLOCKPOOL_HEADER_T*)((char*) block + pool->block_size);
   }

}

VCOS_STATUS_T vcos_generic_blockpool_init(VCOS_BLOCKPOOL_T *pool,
      VCOS_UNSIGNED num_blocks, VCOS_UNSIGNED block_size,
      void *start, VCOS_UNSIGNED pool_size, const char *name)
{
   VCOS_STATUS_T status = VCOS_SUCCESS;

   (void)name;

   vcos_log_trace(
         "%s: pool %p num_blocks %d block_size %d start %p pool_size %d name %p",
         __FUNCTION__, pool, num_blocks, block_size, start, pool_size, name);

   vcos_assert(pool);
   vcos_assert((block_size > 0));
   vcos_assert(num_blocks > 0);
   vcos_assert(start);

   if (VCOS_BLOCKPOOL_SIZE(num_blocks, block_size) < pool_size)
      return VCOS_ENOMEM;

   status = vcos_mutex_create(&pool->mutex, "vcos blockpool mutex");
   if (status != VCOS_SUCCESS)
      return status;

   pool->block_data_size = block_size;
   pool->block_size = VCOS_BLOCKPOOL_ROUND_UP(block_size +
         sizeof(VCOS_BLOCKPOOL_HEADER_T), sizeof(unsigned long));
   pool->magic = VCOS_BLOCKPOOL_MAGIC;
   pool->num_subpools = 1;
   pool->num_extension_blocks = 0;
   memset(pool->subpools, 0, sizeof(pool->subpools));

   vcos_generic_blockpool_subpool_init(pool, &pool->subpools[0], start,
         pool_size, num_blocks, 0);

   return status;
}

VCOS_STATUS_T vcos_generic_blockpool_create_on_heap(VCOS_BLOCKPOOL_T *pool,
      VCOS_UNSIGNED num_blocks, VCOS_UNSIGNED block_size, const char *name)

{
   VCOS_STATUS_T status = VCOS_SUCCESS;
   size_t size = VCOS_BLOCKPOOL_SIZE(num_blocks, block_size);
   void* mem = vcos_malloc(size, name);

   vcos_log_trace("%s: num_blocks %d block_size %d name %p",
         __FUNCTION__, num_blocks, block_size, name);

   if (! mem)
      return VCOS_ENOMEM;

   status = vcos_generic_blockpool_init(pool, num_blocks,
         block_size, mem, size, name);

   if (status != VCOS_SUCCESS)
      goto fail;

   pool->subpools[0].flags |= VCOS_BLOCKPOOL_SUBPOOL_FLAG_OWNS_MEM;
   return status;

fail:
   free(mem);
   return status;
}

VCOS_STATUS_T vcos_generic_blockpool_extend(VCOS_BLOCKPOOL_T *pool,
      VCOS_UNSIGNED num_extensions, VCOS_UNSIGNED num_blocks)
{
   VCOS_UNSIGNED i;
   ASSERT_POOL(pool);

   vcos_log_trace("%s: pool %p num_extensions %d num_blocks %d",
         __FUNCTION__, pool, num_extensions, num_blocks);

   /* Extend may only be called once */
   if (pool->num_subpools > 1)
      return VCOS_EACCESS;

   if (num_extensions < 1 ||
         num_extensions > VCOS_BLOCKPOOL_MAX_SUBPOOLS - 1)
      return VCOS_EINVAL;

   if (num_blocks < 1)
      return VCOS_EINVAL;

   pool->num_subpools += num_extensions;
   pool->num_extension_blocks = num_blocks;

   /* Mark these subpools as valid but unallocated */
   for (i = 1; i < pool->num_subpools; ++i)
   {
      pool->subpools[i].magic = VCOS_BLOCKPOOL_SUBPOOL_MAGIC;
      pool->subpools[i].mem = NULL;
   }

   return VCOS_SUCCESS;
}

void *vcos_generic_blockpool_alloc(VCOS_BLOCKPOOL_T *pool)
{
   VCOS_UNSIGNED i;
   void* ret = NULL;
   VCOS_BLOCKPOOL_SUBPOOL_T *subpool = NULL;

   ASSERT_POOL(pool);
   vcos_mutex_lock(&pool->mutex);

   /* Starting with the main pool try and find a free block */
   for (i = 0; i < pool->num_subpools; ++i)
   {
      if (pool->subpools[i].mem && pool->subpools[i].available_blocks > 0)
      {
         subpool = &pool->subpools[i];
         break; /* Found a subpool with free blocks */
      }
   }

   if (! subpool)
   {
      /* All current subpools are full, try to allocate a new one */
      for (i = 1; i < pool->num_subpools; ++i)
      {
         if (! pool->subpools[i].mem)
         {
            VCOS_BLOCKPOOL_SUBPOOL_T *s = &pool->subpools[i];
            size_t size = VCOS_BLOCKPOOL_SIZE(pool->num_extension_blocks,
                  pool->block_data_size);
            void *mem = vcos_malloc(size, pool->name);
            if (mem)
            {
               vcos_log_trace("%s: Allocated subpool %d", __FUNCTION__, i);
               vcos_generic_blockpool_subpool_init(pool, s, mem, size,
                     pool->num_extension_blocks,
                     VCOS_BLOCKPOOL_SUBPOOL_FLAG_OWNS_MEM |
                     VCOS_BLOCKPOOL_SUBPOOL_FLAG_EXTENSION);
               subpool = s;
               break; /* Created a subpool */
            }
            else
            {
               vcos_log_warn("%s: Failed to allocate subpool", __FUNCTION__);
            }
         }
      }
   }

   if (subpool)
   {
      /* Remove from free list */
      VCOS_BLOCKPOOL_HEADER_T* nb = subpool->free_list;

      vcos_assert(subpool->free_list);
      subpool->free_list = nb->owner.next;

      /* Owner is pool so free can be called without passing pool
       * as a parameter */
      nb->owner.subpool = subpool;

      ret = nb + 1; /* Return pointer to block data */
      --(subpool->available_blocks);
   }
   vcos_mutex_unlock(&pool->mutex);

   VCOS_BLOCKPOOL_DEBUG_LOG("pool %p subpool %p ret %p", pool, subpool, ret);
   return ret;
}

void *vcos_generic_blockpool_calloc(VCOS_BLOCKPOOL_T *pool)
{
   void* ret = vcos_generic_blockpool_alloc(pool);
   if (ret)
      memset(ret, 0, pool->block_data_size);
   return ret;
}

void vcos_generic_blockpool_free(void *block)
{
   VCOS_BLOCKPOOL_DEBUG_LOG("block %p", block);
   if (block)
   {
      VCOS_BLOCKPOOL_HEADER_T* hdr = (VCOS_BLOCKPOOL_HEADER_T*) block - 1;
      VCOS_BLOCKPOOL_SUBPOOL_T *subpool = hdr->owner.subpool;
      VCOS_BLOCKPOOL_T *pool = NULL;

      ASSERT_SUBPOOL(subpool);
      pool = subpool->owner;
      ASSERT_POOL(pool);

      vcos_mutex_lock(&pool->mutex);
      vcos_assert((unsigned) subpool->available_blocks < subpool->num_blocks);

      /* Change ownership of block to be the free list */
      hdr->owner.next = subpool->free_list;
      subpool->free_list = hdr;
      ++(subpool->available_blocks);

      if (VCOS_BLOCKPOOL_OVERWRITE_ON_FREE)
         memset(block, 0xBD, pool->block_data_size); /* For debugging */

      if ( (subpool->flags & VCOS_BLOCKPOOL_SUBPOOL_FLAG_EXTENSION) &&
            subpool->available_blocks == subpool->num_blocks)
      {
         VCOS_BLOCKPOOL_DEBUG_LOG("%s: freeing subpool %p mem %p", __FUNCTION__,
               subpool, subpool->mem);
         /* Free the sub-pool if it was dynamically allocated */
         vcos_free(subpool->mem);
         subpool->mem = NULL;
      }
      vcos_mutex_unlock(&pool->mutex);
   }
}

VCOS_UNSIGNED vcos_generic_blockpool_available_count(VCOS_BLOCKPOOL_T *pool)
{
   VCOS_UNSIGNED ret = 0;
   VCOS_UNSIGNED i;

   ASSERT_POOL(pool);
   vcos_mutex_lock(&pool->mutex);
   for (i = 0; i < pool->num_subpools; ++i)
   {
      VCOS_BLOCKPOOL_SUBPOOL_T *subpool = &pool->subpools[i];
      ASSERT_SUBPOOL(subpool);

      /* Assume the malloc of sub pool would succeed */
      if (subpool->mem)
         ret += subpool->available_blocks;
      else
         ret += pool->num_extension_blocks;
   }
   vcos_mutex_unlock(&pool->mutex);
   return ret;
}

VCOS_UNSIGNED vcos_generic_blockpool_used_count(VCOS_BLOCKPOOL_T *pool)
{
   VCOS_UNSIGNED ret = 0;
   VCOS_UNSIGNED i;

   ASSERT_POOL(pool);
   vcos_mutex_lock(&pool->mutex);

   for (i = 0; i < pool->num_subpools; ++i)
   {
      VCOS_BLOCKPOOL_SUBPOOL_T *subpool = &pool->subpools[i];
      ASSERT_SUBPOOL(subpool);
      if (subpool->mem)
         ret += (subpool->num_blocks - subpool->available_blocks);
   }
   vcos_mutex_unlock(&pool->mutex);
   return ret;
}

void vcos_generic_blockpool_delete(VCOS_BLOCKPOOL_T *pool)
{
   vcos_log_trace("%s: pool %p", __FUNCTION__, pool);

   if (pool)
   {
      VCOS_UNSIGNED i;

      ASSERT_POOL(pool);
      for (i = 0; i < pool->num_subpools; ++i)
      {
         VCOS_BLOCKPOOL_SUBPOOL_T *subpool = &pool->subpools[i];
         ASSERT_SUBPOOL(subpool);
         if (subpool->mem)
         {
            /* For debugging */
            memset(subpool->mem,
                  0xBE,
                  VCOS_BLOCKPOOL_SIZE(subpool->num_blocks,
                     pool->block_data_size));

            if (subpool->flags & VCOS_BLOCKPOOL_SUBPOOL_FLAG_OWNS_MEM)
               vcos_free(subpool->mem);
         }
      }
      vcos_mutex_delete(&pool->mutex);
      memset(pool, 0xBE, sizeof(VCOS_BLOCKPOOL_T)); /* For debugging */
   }
}

uint32_t vcos_generic_blockpool_elem_to_handle(void *block)
{
   uint32_t ret = -1;
   uint32_t index = -1;
   VCOS_BLOCKPOOL_HEADER_T *hdr = NULL;
   VCOS_BLOCKPOOL_T *pool = NULL;
   VCOS_BLOCKPOOL_SUBPOOL_T *subpool = NULL;
   uint32_t subpool_id;

   vcos_assert(block);
   hdr = (VCOS_BLOCKPOOL_HEADER_T*) block - 1;
   subpool = hdr->owner.subpool;
   ASSERT_SUBPOOL(subpool);

   pool = subpool->owner;
   ASSERT_POOL(pool);
   vcos_mutex_lock(&pool->mutex);

   /* The handle is the index into the array of blocks combined
    * with the subpool id.
    */
   index = ((size_t) hdr - (size_t) subpool->start) / pool->block_size;
   vcos_assert(index < subpool->num_blocks);

   subpool_id = ((char*) subpool - (char*) &pool->subpools[0]) /
      sizeof(VCOS_BLOCKPOOL_SUBPOOL_T);

   vcos_assert(subpool_id < VCOS_BLOCKPOOL_MAX_SUBPOOLS);
   vcos_assert(subpool_id < pool->num_subpools);
   ret = VCOS_BLOCKPOOL_HANDLE_CREATE(index, subpool_id);

   vcos_log_trace("%s: index %d subpool_id %d handle 0x%08x",
         __FUNCTION__, index, subpool_id, ret);

   vcos_mutex_unlock(&pool->mutex);
   return ret;
}

void *vcos_generic_blockpool_elem_from_handle(
      VCOS_BLOCKPOOL_T *pool, uint32_t handle)
{
   VCOS_BLOCKPOOL_SUBPOOL_T *subpool;
   uint32_t subpool_id;
   uint32_t index;
   void *ret = NULL;

   vcos_log_trace("%s: pool %p handle 0x%08x", __FUNCTION__, pool, handle);

   ASSERT_POOL(pool);
   vcos_mutex_lock(&pool->mutex);
   subpool_id = VCOS_BLOCKPOOL_HANDLE_GET_SUBPOOL(handle);

   if (subpool_id < pool->num_subpools)
   {
      index = VCOS_BLOCKPOOL_HANDLE_GET_INDEX(handle);
      subpool = &pool->subpools[subpool_id];
      if (pool->subpools[subpool_id].magic == VCOS_BLOCKPOOL_SUBPOOL_MAGIC &&
            pool->subpools[subpool_id].mem && index < subpool->num_blocks)
      {
         VCOS_BLOCKPOOL_HEADER_T *hdr = (VCOS_BLOCKPOOL_HEADER_T*)
            ((size_t) subpool->start + (index * pool->block_size));

         if (hdr->owner.subpool == subpool) /* Check block is allocated */
            ret = hdr + 1;
      }
   }
   vcos_mutex_unlock(&pool->mutex);
   return ret;
}

uint32_t vcos_generic_blockpool_is_valid_elem(
      VCOS_BLOCKPOOL_T *pool, const void *block)
{
   uint32_t ret = 0;
   const char *pool_end;
   VCOS_UNSIGNED i = 0;

   ASSERT_POOL(pool);
   if (((size_t) block) & 0x3)
      return 0;

   vcos_mutex_lock(&pool->mutex);

   for (i = 0; i < pool->num_subpools; ++i)
   {
      VCOS_BLOCKPOOL_SUBPOOL_T *subpool = &pool->subpools[i];
      ASSERT_SUBPOOL(subpool);

      if (subpool->mem)
      {
         pool_end = (const char*)subpool->mem +
            (subpool->num_blocks * pool->block_size);

         if ((const char*)block > (const char*)subpool->mem &&
               (const char*)block < pool_end)
         {
            const VCOS_BLOCKPOOL_HEADER_T *hdr = (
                  const VCOS_BLOCKPOOL_HEADER_T*) block - 1;

            /* If the block has a header where the owner points to the pool then
             * it's a valid block. */
            ret = (hdr->owner.subpool == subpool && subpool->owner == pool);
            break;
         }
      }
   }
   vcos_mutex_unlock(&pool->mutex);
   return ret;
}
