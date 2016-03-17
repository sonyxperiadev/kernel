/*
 * drivers/gpu/ion/ion_secure_heap.c
 *
 * Copyright (C) 2013 Broadcom, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#define pr_fmt(fmt) "ion-secure: " fmt

#include <linux/spinlock.h>

#include <linux/err.h>
#include <linux/genalloc.h>
#include <linux/io.h>
#include <linux/ion.h>
#include <linux/mm.h>
#include <linux/scatterlist.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/seq_file.h>
#include <linux/debugfs.h>
#include <linux/broadcom/bcm_ion.h>
#include "ion_priv.h"
#include <asm/mach/map.h>


#define SECURE_HEAP_IOVA_BASE   (0xFBF00000)
#define SECURE_HEAP_IOVA_END    (0xFFF00000)


struct ion_secure_heap {
	struct ion_heap heap;
	struct gen_pool *pool;
	ion_phys_addr_t base;
	int size;
};

static ion_phys_addr_t ion_secure_allocate(struct ion_heap *heap,
				      unsigned long size,
				      unsigned long align)
{
	struct ion_secure_heap *secure_heap =
		container_of(heap, struct ion_secure_heap, heap);
	unsigned long offset = gen_pool_alloc(secure_heap->pool, size);

	if (!offset)
		return ION_SECURE_ALLOCATE_FAIL;

	return offset;
}

static void ion_secure_free(struct ion_heap *heap, ion_phys_addr_t addr,
		       unsigned long size)
{
	struct ion_secure_heap *secure_heap =
		container_of(heap, struct ion_secure_heap, heap);

	if (addr == ION_SECURE_ALLOCATE_FAIL)
		return;
	gen_pool_free(secure_heap->pool, addr, size);
}

static int ion_secure_heap_phys(struct ion_heap *heap,
				  struct ion_buffer *buffer,
				  ion_phys_addr_t *addr, size_t *len)
{
	*addr = buffer->priv_phys;
	*len = buffer->size;
	return 0;
}

static int ion_secure_heap_allocate(struct ion_heap *heap,
				      struct ion_buffer *buffer,
				      unsigned long size, unsigned long align,
				      unsigned long flags)
{
	buffer->priv_phys = ion_secure_allocate(heap, size, align);
	return buffer->priv_phys == ION_SECURE_ALLOCATE_FAIL ? -ENOMEM : 0;
}

static void ion_secure_heap_free(struct ion_buffer *buffer)
{
	struct ion_heap *heap = buffer->heap;

	ion_secure_free(heap, buffer->priv_phys, buffer->size);
	buffer->priv_phys = ION_SECURE_ALLOCATE_FAIL;
}

struct sg_table *ion_secure_heap_map_dma(struct ion_heap *heap,
					      struct ion_buffer *buffer)
{
	struct ion_secure_heap *secure_heap =
		container_of(heap, struct ion_secure_heap, heap);
	struct sg_table *table;
	int ret;

	table = kzalloc(sizeof(struct sg_table), GFP_KERNEL);
	if (!table)
		return ERR_PTR(-ENOMEM);
	ret = sg_alloc_table(table, 1, GFP_KERNEL);
	if (ret) {
		kfree(table);
		return ERR_PTR(ret);
	}
	sg_set_page(table->sgl, phys_to_page(buffer->priv_phys), buffer->size,
		    0);
	buffer->dma_addr = SECURE_HEAP_IOVA_BASE
		+ (buffer->priv_phys - secure_heap->base);
	pr_debug("%16s: map buffer(%p) da(%#x) pa(%#lx) size(%#x)\n",
			heap->name, buffer, buffer->dma_addr, buffer->priv_phys,
			buffer->size);
	return table;
}

void ion_secure_heap_unmap_dma(struct ion_heap *heap,
				 struct ion_buffer *buffer)
{
	pr_debug("%16s: unmap buffer(%p) da(%#x) pa(%#lx) size(%#x)\n",
			heap->name, buffer, buffer->dma_addr, buffer->priv_phys,
			buffer->size);
	buffer->dma_addr = ION_DMA_ADDR_FAIL;
	sg_free_table(buffer->sg_table);
	kfree(buffer->sg_table);
}

static struct ion_heap_ops secure_heap_ops = {
	.allocate = ion_secure_heap_allocate,
	.free = ion_secure_heap_free,
	.phys = ion_secure_heap_phys,
	.map_dma = ion_secure_heap_map_dma,
	.unmap_dma = ion_secure_heap_unmap_dma,
};

struct ion_heap *ion_secure_heap_create(struct ion_platform_heap *heap_data)
{
	struct ion_secure_heap *secure_heap;

	secure_heap = kzalloc(sizeof(struct ion_secure_heap), GFP_KERNEL);
	if (!secure_heap)
		return ERR_PTR(-ENOMEM);

	secure_heap->pool = gen_pool_create(12, -1);
	if (!secure_heap->pool) {
		kfree(secure_heap);
		return ERR_PTR(-ENOMEM);
	}
	secure_heap->base = heap_data->base;
	gen_pool_add(secure_heap->pool, secure_heap->base, heap_data->size,
		     -1);
	secure_heap->heap.ops = &secure_heap_ops;
	secure_heap->heap.type = ION_HEAP_TYPE_SECURE;
	secure_heap->size = heap_data->size;

	return &secure_heap->heap;
}

void ion_secure_heap_destroy(struct ion_heap *heap)
{
	struct ion_secure_heap *secure_heap =
	     container_of(heap, struct  ion_secure_heap, heap);

	gen_pool_destroy(secure_heap->pool);
	kfree(secure_heap);
	secure_heap = NULL;
}
