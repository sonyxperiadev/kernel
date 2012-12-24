/*
 * drivers/gpu/ion/ion_cma_heap.c
 *
 * Copyright (C) Linaro 2012
 * Author: <benjamin.gaignard@linaro.org> for ST-Ericsson.
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

#define pr_fmt(fmt) "ion-cma: " fmt

#include <linux/device.h>
#include <linux/ion.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/dma-mapping.h>
#ifdef CONFIG_ION_KONA
#include <linux/dma-direction.h>
#include <asm/cacheflush.h>
#endif

/* #define RHEA_CMA */
#ifdef RHEA_CMA
#include <linux/dma-contiguous.h>
#endif

/* for ion_heap_ops structure */
#include "ion_priv.h"

struct ion_cma_buffer_info {
	void *cpu_addr;
	dma_addr_t handle;
	struct sg_table *table;
};

/* ION CMA heap operations functions */
static int ion_cma_allocate(struct ion_heap *heap, struct ion_buffer *buffer,
			    unsigned long len, unsigned long align,
			    unsigned long flags)
{
	struct device *dev = heap->priv;
	struct ion_cma_buffer_info *info;
	int n_pages, i;
	struct scatterlist *sg;
#ifdef RHEA_CMA
	struct page *page;
	unsigned long nr_pages = len >> PAGE_SHIFT;
#endif

	dev_dbg(dev, "Request buffer allocation len %ld\n", len);

	info = kzalloc(sizeof(struct ion_cma_buffer_info), GFP_KERNEL);
	if (!info) {
		dev_err(dev, "Can't allocate buffer info\n");
		return -ENOMEM;
	}

#ifdef RHEA_CMA
	page = dma_alloc_from_contiguous(dev, nr_pages, 0);
	if (!page) {
		dev_err(dev, "Fail to allocate buffer\n");
		goto err;
	}
	if (current->group_leader && current->group_leader->mm)
		atomic_long_add(nr_pages, &current->group_leader->mm->cma_stat);
	info->handle = page_to_phys(page);
	info->cpu_addr = phys_to_virt(info->handle);
#else
	info->cpu_addr = dma_alloc_coherent(dev, len, &(info->handle), 0);
	if (!info->cpu_addr) {
		goto err;
	}
#endif

	info->table = kmalloc(sizeof(struct sg_table), GFP_KERNEL);
	if (!info->table) {
		dev_err(dev, "Fail to allocate sg table\n");
		dma_free_coherent(dev, len, info->cpu_addr, info->handle);
		goto err;
	}

	n_pages = PAGE_ALIGN(len) >> PAGE_SHIFT;

	/* CMA allocate one big chunk of memory
	 * so we will only have one entry in sg table */
	i = sg_alloc_table(info->table, 1, GFP_KERNEL);
	if (i) {
		dma_free_coherent(dev, len, info->cpu_addr, info->handle);
		kfree(info->table);
		goto err;
	}
	for_each_sg(info->table->sgl, sg, info->table->nents, i) {
		/*  we will this loop only one time */
		struct page *page = pfn_to_page(dma_to_pfn(dev, info->handle));
		sg_set_page(sg, page, PAGE_SIZE * n_pages, 0);
	}

	/* keep this for memory release */
	buffer->priv_virt = info;
	dev_dbg(dev, "Allocate buffer %p\n", buffer);
	return 0;

err:
	kfree(info);
	return -ENOMEM;
}

static void ion_cma_free(struct ion_buffer *buffer)
{
	struct device *dev = buffer->heap->priv;
	struct ion_cma_buffer_info *info = buffer->priv_virt;
#ifdef RHEA_CMA
	struct page *page;
	unsigned long nr_pages = buffer->size >> PAGE_SHIFT;
#endif

	dev_dbg(dev, "Release buffer %p\n", buffer);
	/* release memory */
#ifdef RHEA_CMA
	page = phys_to_page(info->handle);
	dma_release_from_contiguous(dev, page, nr_pages);
	if (current->group_leader && current->group_leader->mm)
		atomic_long_add(-nr_pages, &current->group_leader->mm->cma_stat);
#else
	dma_free_coherent(dev, buffer->size, info->cpu_addr, info->handle);
#endif
	/* release sg table */
	sg_free_table(info->table);
	kfree(info->table);
	kfree(info);
}

/* return physical address in addr */
static int ion_cma_phys(struct ion_heap *heap, struct ion_buffer *buffer,
			ion_phys_addr_t *addr, size_t *len)
{
	struct device *dev = heap->priv;
	struct ion_cma_buffer_info *info = buffer->priv_virt;

	dev_dbg(dev, "Return buffer %p physical address 0x%x\n", buffer,
		virt_to_phys(info->cpu_addr));

	*addr = virt_to_phys(info->cpu_addr);
	*len = buffer->size;

	return 0;
}

void *ion_cma_heap_map_kernel(struct ion_heap *heap,
				   struct ion_buffer *buffer)
{
	struct ion_cma_buffer_info *info = buffer->priv_virt;

	return info->cpu_addr;
}

void ion_cma_heap_unmap_kernel(struct ion_heap *heap,
				    struct ion_buffer *buffer)
{
	return;
}

struct sg_table *ion_cma_heap_map_dma(struct ion_heap *heap,
					 struct ion_buffer *buffer)
{
	struct ion_cma_buffer_info *info = buffer->priv_virt;

	return info->table;
}

void ion_cma_heap_unmap_dma(struct ion_heap *heap,
			       struct ion_buffer *buffer)
{
	return;
}

static int ion_cma_mmap(struct ion_heap *mapper, struct ion_buffer *buffer,
			struct vm_area_struct *vma)
{
#ifdef CONFIG_ION_KONA
	struct device *dev = buffer->heap->priv;
	struct ion_cma_buffer_info *info = buffer->priv_virt;

	if (buffer->flags & ION_FLAG_WRITECOMBINE)
		vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);
	else if (buffer->flags & ION_FLAG_WRITETHROUGH)
		vma->vm_page_prot = pgprot_writethrough(vma->vm_page_prot);
	else if (buffer->flags & ION_FLAG_WRITEBACK)
		vma->vm_page_prot = pgprot_writeback(vma->vm_page_prot);
	else
		vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

	return remap_pfn_range(vma, vma->vm_start,
			dma_to_pfn(dev, info->handle) + vma->vm_pgoff,
			vma->vm_end - vma->vm_start,
			vma->vm_page_prot);
#else
	struct device *dev = buffer->heap->priv;
	struct ion_cma_buffer_info *info = buffer->priv_virt;

	return dma_mmap_coherent(dev, vma, info->cpu_addr, info->handle,
				 buffer->size);
#endif
}

#ifdef CONFIG_ION_KONA
int ion_cma_heap_flush_cache(struct ion_heap *heap,
		struct ion_buffer *buffer, unsigned long offset,
		unsigned long len)
{
	struct ion_cma_buffer_info *info = buffer->priv_virt;
	phys_addr_t pa;
	void *va;

	pa = info->handle;
	va = info->cpu_addr;
	pr_debug("flush: pa(%x) va(%p) off(%ld) len(%ld)\n",
			pa, va, offset, len);
	dmac_flush_range(va + offset, va + offset + len);
	outer_flush_range(pa + offset, pa + offset + len);

	return 0;
}

int ion_cma_heap_invalidate_cache(struct ion_heap *heap,
		struct ion_buffer *buffer, unsigned long offset,
		unsigned long len)
{
	struct ion_cma_buffer_info *info = buffer->priv_virt;
	phys_addr_t pa;
	void *va;

	pa = info->handle;
	va = info->cpu_addr;
	pr_debug("inv: pa(%x) va(%p) off(%ld) len(%ld)\n",
			pa, va, offset, len);
	outer_inv_range(pa + offset, pa + offset + len);
	dmac_unmap_area(va + offset, len, DMA_FROM_DEVICE);

	return 0;
}
#endif

#ifdef CONFIG_ION_OOM_KILLER
static int ion_cma_heap_needs_shrink(struct ion_heap *heap)
{
	/* Any local checks and disabling lowmem check
	 * can be done here
	 **/
	return heap->lmk_enable;
}
#endif

static struct ion_heap_ops ion_cma_ops = {
	.allocate = ion_cma_allocate,
	.free = ion_cma_free,
	.map_kernel = ion_cma_heap_map_kernel,
	.unmap_kernel = ion_cma_heap_unmap_kernel,
	.map_dma = ion_cma_heap_map_dma,
	.unmap_dma = ion_cma_heap_unmap_dma,
	.phys = ion_cma_phys,
	.map_user = ion_cma_mmap,
#ifdef CONFIG_ION_KONA
	.flush_cache = ion_cma_heap_flush_cache,
	.invalidate_cache = ion_cma_heap_invalidate_cache,
#endif
#ifdef CONFIG_ION_OOM_KILLER
	.needs_shrink = ion_cma_heap_needs_shrink,
#endif
};

struct ion_heap *ion_cma_heap_create(struct ion_platform_heap *data,
				     struct device *dev)
{
	struct ion_heap *heap;

	heap = kzalloc(sizeof(struct ion_heap), GFP_KERNEL);

	if (!heap)
		return ERR_PTR(-ENOMEM);

	heap->ops = &ion_cma_ops;
	/* set device as private heaps data, later it will be
	 * used to make the link with reserved CMA memory */
	heap->priv = dev;
	heap->type = ION_HEAP_TYPE_DMA;
#ifdef CONFIG_ION_KONA
	heap->size = data->size;
#endif
#ifdef CONFIG_ION_OOM_KILLER
	heap->lmk_enable = data->lmk_enable;
	heap->lmk_min_score_adj = data->lmk_min_score_adj;
	heap->lmk_min_free = data->lmk_min_free;
#endif

	return heap;
}

void ion_cma_heap_destroy(struct ion_heap *heap)
{
	kfree(heap);
}
