/*
 * Copyright (c) 2010-2011 Broadcom Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "vchiq_vc.h"
#include "vchiq_pagelist.h"
#include "helpers/dmalib/dmalib.h"
#include "vcfw/drivers/chip/dma.h"
#include "vcfw/drivers/chip/cache.h"

// we'll always set the burst mode. It only works on channel 0, but should be harmless on other channels. Channel 0 is used for bulk transfers.
#define DMA0_TI_BURST_N(x) (((x)<<DMA0_TI_BURST_LENGTH_LSB)&DMA0_TI_BURST_LENGTH_SET)

#ifdef __VIDEOCORE4__
#define DMA0_TI_DEFAULT_BURST DMA0_TI_BURST_N(15)
#define IC  IC0_C
#else
#define DMA0_TI_DEFAULT_BURST DMA0_TI_BURST_N(5)
#endif

#define MAX_XFER_COUNT 4
#define MAX_SCB_COUNT 512 /* Enough for a 2MB block of discontiguous pages */

static const CACHE_DRIVER_T *cache_driver = NULL;
static DRIVER_HANDLE_T cache_handle = NULL;

static dma_transfer_t *volatile free_dma_xfers = NULL;
static VCOS_MUTEX_T free_dma_xfers_mutex;
static VCOS_EVENT_T free_dma_xfers_avail;
static int vchiq_max_fragments;
static FRAGMENTS_T *vchiq_fragments;

VCHIQ_STATUS_T
init_dma_xfers(int max_fragments, FRAGMENTS_T *fragments)
{
   int i;
   int32_t success;

   vcos_assert(free_dma_xfers == NULL);

   vchiq_max_fragments = max_fragments;
   vchiq_fragments = fragments;

   //get a handle to the cache driver
   cache_driver = cache_get_func_table();
   vcos_demand( NULL != cache_driver );
   success = cache_driver->open( NULL, &cache_handle );
   vcos_demand( success >= 0 );
   vcos_mutex_create(&free_dma_xfers_mutex, "free_dma_xfers_mutex");
   vcos_event_create(&free_dma_xfers_avail, "free_dma_xfers_avail");

   for (i = 0; i < MAX_XFER_COUNT; i++)
   {
      /* There is already room for (at least) 3 SCBs in dma_transfer_t */
      dma_transfer_t *xfer = rtos_malloc_priority( sizeof(dma_transfer_t) + (MAX_SCB_COUNT - 3)*sizeof(DMA_SCB), RTOS_ALIGN_256BIT, RTOS_PRIORITY_L1_NONALLOCATING, "vchiq dma_xfer");
      if (!xfer)
         return VCHIQ_ERROR;
      xfer->scb      = (DMA_SCB *)ALIGN_UP(xfer->buf, 32);
      xfer->temp_pre = xfer->temp_post = NULL;
      xfer->temp_pre_dest = xfer->temp_post_dest = NULL;
      xfer->temp_pre_size = xfer->temp_post_size = 0;
      xfer->finished = latch_event_present();
      xfer->callback = 0;
      xfer->next = free_dma_xfers;
      free_dma_xfers = xfer;
   }

   return VCHIQ_SUCCESS;
}

void
uninit_dma_xfers(void)
{
   while (free_dma_xfers != NULL)
   {
      dma_transfer_t *xfer = free_dma_xfers;
      free_dma_xfers = xfer->next;
      rtos_priorityfree(xfer);
   }
   vcos_mutex_delete(&free_dma_xfers_mutex);
   vcos_event_delete(&free_dma_xfers_avail);
}

dma_transfer_t *
alloc_dma_xfer(void)
{
   dma_transfer_t *xfer;
   vcos_mutex_lock(&free_dma_xfers_mutex);
   while ((xfer = free_dma_xfers) == NULL)
   {
      vcos_mutex_unlock(&free_dma_xfers_mutex);
      vcos_event_wait(&free_dma_xfers_avail);
      vcos_mutex_lock(&free_dma_xfers_mutex);
   }
   free_dma_xfers = xfer->next;
   vcos_mutex_unlock(&free_dma_xfers_mutex);
   xfer->next = NULL;
   return xfer;
}

void
free_dma_xfer(dma_transfer_t *xfer)
{
   vcos_assert(xfer != NULL);
   vcos_mutex_lock(&free_dma_xfers_mutex);
   xfer->next = free_dma_xfers;
   free_dma_xfers = xfer;
   vcos_event_signal(&free_dma_xfers_avail);
   vcos_mutex_unlock(&free_dma_xfers_mutex);
   cache_driver->close( cache_handle );
}


int
dma_memcpy_pagelist(char *vcptr, const PAGELIST_T *pagelist)
{
   int len, offset, i;
   dma_transfer_queue_t *q;
   dma_transfer_t *xfer;
   DMA_SCB *scb;
   const uint32_t xfer_info  = DMA0_TI_DEST_INC_SET|DMA0_TI_SRC_INC_SET
                        | DMA0_TI_DEFAULT_BURST
                        | DMA0_TI_DEST_WIDTH_SET
                        | DMA0_TI_SRC_WIDTH_SET;
   unsigned long addr;
   int pre_size = 0, post_size = 0;
   char *block_start;
   char *block_end;
   int block_bytes;
   int xfer_len;
   int type;

   len = xfer_len = pagelist->length;
   offset = pagelist->offset;
   type = pagelist->type;
   i = 0;

   if (type != PAGELIST_WRITE)
   {
      // the source is read only - so only need to do a cache flush before reading it. Alignment doesn't matter
      if (!IS_ALIAS_NOT_L1(vcptr)) {
         cache_driver->flush_range( cache_handle, CACHE_DEST_L1, vcptr, len );
         vcptr = ALIAS_L1_NONALLOCATING(vcptr);
      }

      if (pagelist->type >= PAGELIST_READ_WITH_FRAGMENTS)
      {
         int fragment_num = (pagelist->type - PAGELIST_READ_WITH_FRAGMENTS);
         FRAGMENTS_T *fragments;
         int head_bytes, tail_bytes;
         if (fragment_num >= vchiq_max_fragments)
            return VCHIQ_BULK_ACTUAL_ABORTED;
         fragments = vchiq_fragments + fragment_num;
         head_bytes = (-offset) & (CACHE_LINE_SIZE - 1);
         if (head_bytes)
         {
            if (head_bytes > len)
               head_bytes = len;
            memcpy(fragments->headbuf, vcptr, head_bytes);
            vcptr += head_bytes;
            len -= head_bytes;
            if (len == 0)
               return xfer_len;
            offset += head_bytes;
            if (offset == PAGE_SIZE)
            {
               offset = 0;
               i = 1;
            }
         }
         tail_bytes = (offset + len) & (CACHE_LINE_SIZE - 1);
         if (tail_bytes)
         {
            len -= tail_bytes;
            memcpy(fragments->tailbuf, vcptr + len, tail_bytes);
            if (len == 0)
               return xfer_len;
         }
      }
   }

   addr = pagelist->addrs[i];
   block_start = (char *)(addr & ~(PAGE_SIZE - 1)) + offset;
   block_bytes = PAGE_SIZE * ((addr & (PAGE_SIZE - 1)) + 1) - offset;
   block_end = block_start + block_bytes;
   if (block_bytes > len)
      block_bytes = len;

   if (block_bytes < 64)
   {
      if (type == PAGELIST_WRITE)
         memcpy(vcptr, block_start, block_bytes);
      else
         memcpy(block_start, vcptr, block_bytes);
      vcptr += block_bytes;
      len -= block_bytes;
      i += 1;
      if (len == 0)
         return xfer_len;
      addr = pagelist->addrs[i];
      block_start = (char *)(addr & ~(PAGE_SIZE - 1));
      block_bytes = PAGE_SIZE * ((addr & (PAGE_SIZE - 1)) + 1);
      block_end = block_start + block_bytes;
      if (block_bytes > len)
      {
         block_bytes = len;
         if (block_bytes < 64)
         {
            if (type == PAGELIST_WRITE)
               memcpy(vcptr, block_start, block_bytes);
            else
               memcpy(block_start, vcptr, block_bytes);
            return xfer_len;
         }
      }
   }

   if (type == PAGELIST_WRITE)
   {
      // VC memory is written to. There is a danger if the start or end are not
      // cache-line aligned, that another user of the cache line will cause the data
      // to be corrupted.
      // We only care when L1 aliased - skip pre and post otherwise
      if (!IS_ALIAS_NOT_L1(vcptr)) {
         post_size = ((int)vcptr + len) & (CACHE_LINE_SIZE - 1);
         len -= post_size;
         if (len < block_bytes)
            block_bytes = len;

         pre_size =  (-(int)vcptr) & (CACHE_LINE_SIZE - 1);
         if (pre_size >= block_bytes)
         {
            memcpy(vcptr, block_start, block_bytes);
            vcptr += block_bytes;
            len -= block_bytes;
            pre_size -= block_bytes;
            i += 1;
            addr = pagelist->addrs[i];
            block_start = (char *)(addr & ~(PAGE_SIZE - 1));
            block_bytes = PAGE_SIZE * ((addr & (PAGE_SIZE - 1)) + 1);
            block_end = block_start + block_bytes;
            if (block_bytes > len)
               block_bytes = len;
         }
         if (pre_size)
         {
            memcpy(vcptr, block_start, pre_size);
            vcptr += pre_size;
            block_start += pre_size;
            block_bytes -= pre_size;
            len -= pre_size;
         }
      }
   }

   if (len && !IS_ALIAS_NOT_L1(vcptr)) {
      cache_driver->invalidate_range( cache_handle, CACHE_DEST_L1, vcptr, len );
      vcptr = ALIAS_L1_NONALLOCATING(vcptr);
   }

   xfer = alloc_dma_xfer();
   if (!xfer)
      return VCHIQ_BULK_ACTUAL_ABORTED;

   q = dma_get_transfer_queue(dma_prio_transfer_bulk);
   if (!q)
   {
      free_dma_xfer(xfer);
      return VCHIQ_BULK_ACTUAL_ABORTED;
   }

   xfer->callback = 0;

   scb = xfer->scb;

   while (len)
   {
      len -= block_bytes;
      if (type == PAGELIST_WRITE)
      {
         scb->dst_addr = vcptr;
         scb->src_addr = block_start;
      }
      else
      {
         scb->dst_addr = block_start;
         scb->src_addr = vcptr;
      }

      scb->next_scb = len ? (scb+1) : NULL;
      scb->length = block_bytes;
      scb->xfer_info = xfer_info | (len ? 0 : DMA0_TI_INTEN_SET);
      scb->blk_status = 0;
      vcptr += block_bytes;
      if (len)
      {
         scb++;
         if (scb == (xfer->scb + MAX_SCB_COUNT))
         {
            /* Split this large transfer */
            scb[-1].next_scb = NULL;
            scb[-1].xfer_info = xfer_info | DMA0_TI_INTEN_SET;
            dma_transfer_queue_post(q,xfer);
            dma_transfer_wait(xfer);
            xfer->next = NULL;
            scb = xfer->scb;
         }
         i += 1;
         addr = pagelist->addrs[i];
         block_start = (char *)(addr & ~(PAGE_SIZE - 1));
         block_bytes = PAGE_SIZE * ((addr & (PAGE_SIZE - 1)) + 1);
         block_end = block_start + block_bytes;
         if (block_bytes > len)
            block_bytes = len;
      }
   }

   dma_transfer_queue_post(q,xfer);

   if ( post_size ) {
      block_start += block_bytes;
      block_bytes = (-(int)block_start) & (PAGE_SIZE - 1); // space left on this page
      // memcpy these remaining bytes to populate the L1
      if ((block_start + post_size) > block_end)
      {
         if (block_bytes)
         {
            memcpy(vcptr, block_start, block_bytes);
            vcptr += block_bytes;
            post_size -= block_bytes;
         }
         i += 1;
         addr = pagelist->addrs[i];
         vcos_assert((addr & (PAGE_SIZE - 1)) == 0);
         block_start = (char *)addr;
      }
      memcpy(vcptr, block_start, post_size);
   }

   dma_transfer_wait(xfer);
   dma_transfer_queue_release(q);
   free_dma_xfer(xfer);

   return xfer_len;
}
