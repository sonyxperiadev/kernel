/*
 * drivers/gpu/ion/ion_carveout_heap.c
 *
 * Copyright (C) 2011 Google, Inc.
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

#define pr_fmt(fmt) "ion-carveout: " fmt

#include <linux/spinlock.h>

#include <linux/err.h>
#include <linux/genalloc.h>
#include <linux/io.h>
#include <linux/ion.h>
#include <linux/mm.h>
#include <linux/scatterlist.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#ifdef CONFIG_ION_KONA
#include <linux/dma-direction.h>
#include <asm/cacheflush.h>
#include <linux/seq_file.h>
#include <linux/debugfs.h>
#endif
#include "ion_priv.h"

#include <asm/mach/map.h>

struct ion_carveout_heap {
	struct ion_heap heap;
	struct gen_pool *pool;
	ion_phys_addr_t base;
#ifdef CONFIG_ION_KONA
	int size;
#endif
#ifdef CONFIG_ION_OOM_KILLER
	int lmk_enable;
	int lmk_min_score_adj;
	int lmk_min_free;
	struct dentry *lmk_debug_root;
#endif
};

ion_phys_addr_t ion_carveout_allocate(struct ion_heap *heap,
				      unsigned long size,
				      unsigned long align)
{
	struct ion_carveout_heap *carveout_heap =
		container_of(heap, struct ion_carveout_heap, heap);
	unsigned long offset = gen_pool_alloc(carveout_heap->pool, size);

	if (!offset)
		return ION_CARVEOUT_ALLOCATE_FAIL;

	return offset;
}

void ion_carveout_free(struct ion_heap *heap, ion_phys_addr_t addr,
		       unsigned long size)
{
	struct ion_carveout_heap *carveout_heap =
		container_of(heap, struct ion_carveout_heap, heap);

	if (addr == ION_CARVEOUT_ALLOCATE_FAIL)
		return;
	gen_pool_free(carveout_heap->pool, addr, size);
}

static int ion_carveout_heap_phys(struct ion_heap *heap,
				  struct ion_buffer *buffer,
				  ion_phys_addr_t *addr, size_t *len)
{
	*addr = buffer->priv_phys;
	*len = buffer->size;
	return 0;
}

static int ion_carveout_heap_allocate(struct ion_heap *heap,
				      struct ion_buffer *buffer,
				      unsigned long size, unsigned long align,
				      unsigned long flags)
{
	buffer->priv_phys = ion_carveout_allocate(heap, size, align);
	return buffer->priv_phys == ION_CARVEOUT_ALLOCATE_FAIL ? -ENOMEM : 0;
}

static void ion_carveout_heap_free(struct ion_buffer *buffer)
{
	struct ion_heap *heap = buffer->heap;

		ion_carveout_free(heap, buffer->priv_phys, buffer->size);
		buffer->priv_phys = ION_CARVEOUT_ALLOCATE_FAIL;
	}

struct sg_table *ion_carveout_heap_map_dma(struct ion_heap *heap,
					      struct ion_buffer *buffer)
{
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
#ifdef CONFIG_ION_KONA
	buffer->dma_addr = buffer->priv_phys;
#endif
	return table;
}

void ion_carveout_heap_unmap_dma(struct ion_heap *heap,
				 struct ion_buffer *buffer)
{
#ifdef CONFIG_ION_KONA
	buffer->dma_addr = ION_DMA_ADDR_FAIL;
#endif
	sg_free_table(buffer->sg_table);
	kfree(buffer->sg_table);
}

void *ion_carveout_heap_map_kernel(struct ion_heap *heap,
				   struct ion_buffer *buffer)
{
	int mtype = MT_MEMORY_NONCACHED;

	if (buffer->flags & ION_FLAG_CACHED)
		mtype = MT_MEMORY;

	return __arm_ioremap(buffer->priv_phys, buffer->size,
			      mtype);
}

void ion_carveout_heap_unmap_kernel(struct ion_heap *heap,
				    struct ion_buffer *buffer)
{
	__arm_iounmap(buffer->vaddr);
	buffer->vaddr = NULL;
	return;
}

int ion_carveout_heap_map_user(struct ion_heap *heap, struct ion_buffer *buffer,
			       struct vm_area_struct *vma)
{
#ifdef CONFIG_ION_KONA
	if (buffer->flags & ION_FLAG_WRITECOMBINE)
		vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);
	else if (buffer->flags & ION_FLAG_WRITETHROUGH)
		vma->vm_page_prot = pgprot_writethrough(vma->vm_page_prot);
	else if (buffer->flags & ION_FLAG_WRITEBACK)
		vma->vm_page_prot = pgprot_writeback(vma->vm_page_prot);
	else
		vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

	return remap_pfn_range(vma, vma->vm_start,
			__phys_to_pfn(buffer->priv_phys) + vma->vm_pgoff,
			vma->vm_end - vma->vm_start,
			vma->vm_page_prot);
#else
	return remap_pfn_range(vma, vma->vm_start,
			__phys_to_pfn(buffer->priv_phys) + vma->vm_pgoff,
			vma->vm_end - vma->vm_start,
			       pgprot_noncached(vma->vm_page_prot));
#endif
}

#ifdef CONFIG_ION_KONA
int ion_carveout_heap_clean_cache(struct ion_heap *heap,
		struct ion_buffer *buffer, unsigned long offset,
		unsigned long len)
{
	int mtype = MT_MEMORY;
	phys_addr_t pa;
	void *va;

	pa = buffer->priv_phys;
	va = __arm_ioremap(buffer->priv_phys, buffer->size, mtype);
	pr_debug("clean: pa(%x) va(%p) off(%ld) len(%ld)\n",
			pa, va, offset, len);
	dmac_unmap_area(va + offset, len, DMA_BIDIRECTIONAL);
	outer_clean_range(pa + offset, pa + offset + len);
	__arm_iounmap(va);

	return 0;
}

int ion_carveout_heap_invalidate_cache(struct ion_heap *heap,
		struct ion_buffer *buffer, unsigned long offset,
		unsigned long len)
{
	int mtype = MT_MEMORY;
	phys_addr_t pa;
	void *va;

	pa = buffer->priv_phys;
	va = __arm_ioremap(buffer->priv_phys, buffer->size, mtype);
	pr_debug("inv: pa(%x) va(%p) off(%ld) len(%ld)\n",
			pa, va, offset, len);
	outer_inv_range(pa + offset, pa + offset + len);
	dmac_unmap_area(va + offset, len, DMA_BIDIRECTIONAL);
	__arm_iounmap(va);

	return 0;
}
#endif

static struct ion_heap_ops carveout_heap_ops = {
	.allocate = ion_carveout_heap_allocate,
	.free = ion_carveout_heap_free,
	.phys = ion_carveout_heap_phys,
	.map_dma = ion_carveout_heap_map_dma,
	.unmap_dma = ion_carveout_heap_unmap_dma,
	.map_user = ion_carveout_heap_map_user,
	.map_kernel = ion_carveout_heap_map_kernel,
	.unmap_kernel = ion_carveout_heap_unmap_kernel,
#ifdef CONFIG_ION_KONA
	.clean_cache = ion_carveout_heap_clean_cache,
	.invalidate_cache = ion_carveout_heap_invalidate_cache,
#endif
};

#ifdef CONFIG_ION_KONA
static int ion_carveout_heap_free_size(struct ion_heap *heap)
{
	struct ion_carveout_heap *carveout_heap =
		container_of(heap, struct ion_carveout_heap, heap);

	return carveout_heap->size - heap->used;
}

#ifdef CONFIG_ION_OOM_KILLER
static int ion_carveout_lmk_shrink_info(struct ion_heap *heap, int *min_adj,
		int *min_free)
{
	struct ion_carveout_heap *carveout_heap =
		container_of(heap, struct ion_carveout_heap, heap);
	int lmk_min_free = carveout_heap->lmk_min_free;
	int free_size = ion_carveout_heap_free_size(heap);

	if (!carveout_heap->lmk_enable)
		return 0;
	if ((lmk_min_free < 0) || (lmk_min_free > 128)) {
		pr_err("lmk_min_free(%d) should be in [0-128] range\n",
				lmk_min_free);
		return 0;
	}
	*min_free = ((carveout_heap->size / 128) * lmk_min_free) & PAGE_MASK;
	*min_adj = carveout_heap->lmk_min_score_adj;
	if (free_size >= *min_free)
		return 0;

	return 1;
}

static int ion_carveout_lmk_debugfs_add(struct ion_heap *heap,
		struct dentry *debug_root)
{
	char debug_name[64];
	struct ion_carveout_heap *carveout_heap =
		container_of(heap, struct ion_carveout_heap, heap);

	snprintf(debug_name, 64, "lmk_%s", heap->name);
	carveout_heap->lmk_debug_root = debugfs_create_dir(debug_name,
			debug_root);
	debugfs_create_u32("enable", (S_IRUGO|S_IWUSR),
			carveout_heap->lmk_debug_root,
			(unsigned int *)&carveout_heap->lmk_enable);
	debugfs_create_u32("oom_score_adj", (S_IRUGO|S_IWUSR),
			carveout_heap->lmk_debug_root,
			(unsigned int *)&carveout_heap->lmk_min_score_adj);
	debugfs_create_u32("min_free", (S_IRUGO|S_IWUSR),
			carveout_heap->lmk_debug_root,
			(unsigned int *)&carveout_heap->lmk_min_free);
	return 0;
}
#endif

static int ion_carveout_heap_debug_show(struct ion_heap *heap,
		struct seq_file *s, void *unused)
{
	struct ion_carveout_heap *carveout_heap =
		container_of(heap, struct ion_carveout_heap, heap);

	seq_printf(s, "%16.s: %6s(%s) %4s(%d) %6s(%u)KB %7s(%#08lx - %#08lx)\n",
			"Carveout Heap", "Name", heap->name, "Id", heap->id,
			"Size", (carveout_heap->size>>10), "Range",
			carveout_heap->base,
			(carveout_heap->base + carveout_heap->size));
#ifdef CONFIG_ION_OOM_KILLER
	if (carveout_heap->lmk_enable) {
		int min_adj, min_free, free_size;

		ion_carveout_lmk_shrink_info(heap, &min_adj, &min_free);
		free_size = ion_carveout_heap_free_size(heap);

		seq_printf(s, "Lowmemkiller Info:\n");
		seq_printf(s, "%16.s %16.s %16.s\n%13u KB %13u KB %16u\n",
				"free mem", "threshold", "min_adj",
				free_size>>10, min_free>>10, min_adj);
	} else {
		seq_printf(s, "  Lowmemkiller disabled.\n");
	}
#endif
	return 0;
}

#endif

struct ion_heap *ion_carveout_heap_create(struct ion_platform_heap *heap_data)
{
	struct ion_carveout_heap *carveout_heap;

	carveout_heap = kzalloc(sizeof(struct ion_carveout_heap), GFP_KERNEL);
	if (!carveout_heap)
		return ERR_PTR(-ENOMEM);

	carveout_heap->pool = gen_pool_create(12, -1);
	if (!carveout_heap->pool) {
		kfree(carveout_heap);
		return ERR_PTR(-ENOMEM);
	}
	carveout_heap->base = heap_data->base;
	gen_pool_add(carveout_heap->pool, carveout_heap->base, heap_data->size,
		     -1);
	carveout_heap->heap.ops = &carveout_heap_ops;
	carveout_heap->heap.type = ION_HEAP_TYPE_CARVEOUT;
#ifdef CONFIG_ION_KONA
	carveout_heap->size = heap_data->size;
	carveout_heap->heap.debug_show = ion_carveout_heap_debug_show;
	carveout_heap->heap.free_size = ion_carveout_heap_free_size;
#endif
#ifdef CONFIG_ION_OOM_KILLER
	carveout_heap->lmk_enable = heap_data->lmk_enable;
	carveout_heap->lmk_min_score_adj = heap_data->lmk_min_score_adj;
	carveout_heap->lmk_min_free = heap_data->lmk_min_free;
	carveout_heap->heap.lmk_shrink_info = ion_carveout_lmk_shrink_info;
	carveout_heap->heap.lmk_debugfs_add = ion_carveout_lmk_debugfs_add;
#endif

	return &carveout_heap->heap;
}

void ion_carveout_heap_destroy(struct ion_heap *heap)
{
	struct ion_carveout_heap *carveout_heap =
	     container_of(heap, struct  ion_carveout_heap, heap);

	gen_pool_destroy(carveout_heap->pool);
	kfree(carveout_heap);
	carveout_heap = NULL;
}
