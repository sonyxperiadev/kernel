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
#include <linux/dma-direction.h>
#include <asm/cacheflush.h>
#include <linux/seq_file.h>
#include <linux/debugfs.h>
#ifdef CONFIG_ION_BCM
#include <linux/broadcom/bcm_ion.h>
#endif
#ifdef CONFIG_IOMMU_API
#include <linux/iommu.h>
#endif
#ifdef CONFIG_BCM_IOVMM
#include <plat/bcm_iommu.h>
#endif

/* for ion_heap_ops structure */
#include "ion_priv.h"

struct ion_cma_heap {
	struct ion_heap heap;
	ion_phys_addr_t base;
	int size;
	struct device *dev;
#ifdef CONFIG_ION_OOM_KILLER
	int lmk_enable;
	int lmk_min_score_adj;
	int lmk_min_free;
	struct dentry *lmk_debug_root;
#endif
};

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
	struct ion_cma_heap *cma_heap =
		container_of(heap, struct ion_cma_heap, heap);
	struct device *dev = cma_heap->dev;
	struct ion_cma_buffer_info *info;
	int n_pages, i;
	struct scatterlist *sg;

	dev_dbg(dev, "Request buffer allocation len %ld\n", len);

	info = kzalloc(sizeof(struct ion_cma_buffer_info), GFP_KERNEL);
	if (!info) {
		dev_err(dev, "Can't allocate buffer info\n");
		return -ENOMEM;
	}

	info->cpu_addr = dma_alloc_coherent(dev, len, &(info->handle), 0);
	if (!info->cpu_addr) {
		goto err;
	}

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
	struct ion_cma_heap *cma_heap =
		container_of(buffer->heap, struct ion_cma_heap, heap);
	struct device *dev = cma_heap->dev;
	struct ion_cma_buffer_info *info = buffer->priv_virt;

	dev_dbg(dev, "Release buffer %p\n", buffer);
	/* release memory */
	dma_free_coherent(dev, buffer->size, info->cpu_addr, info->handle);
	/* release sg table */
	sg_free_table(info->table);
	kfree(info->table);
	kfree(info);
}

/* return physical address in addr */
static int ion_cma_phys(struct ion_heap *heap, struct ion_buffer *buffer,
			ion_phys_addr_t *addr, size_t *len)
{
	struct ion_cma_heap *cma_heap =
		container_of(heap, struct ion_cma_heap, heap);
	struct device *dev = cma_heap->dev;
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

#ifdef CONFIG_ION_BCM
	buffer->dma_addr = info->handle;
#ifdef CONFIG_IOMMU_API
#ifdef CONFIG_BCM_IOVMM
	buffer->dma_addr = arm_iommu_map_sgt(heap->device, info->table,
			buffer->align);
	if (buffer->dma_addr == DMA_ERROR_CODE) {
		pr_err("%16s: Failed iommu map buffer(%p) da(%#x) pa(%#x) size(%#x)\n",
				heap->name, buffer, buffer->dma_addr,
				info->handle, buffer->size);
		return ERR_PTR(-ENOMEM);
	}
#else
	if (iommu_map(heap->domain, buffer->dma_addr, info->handle,
				buffer->size, 0)) {
		pr_err("%16s: Failed iommu map buffer(%p) da(%#x) pa(%#x) size(%#x)\n",
				heap->name, buffer, buffer->dma_addr,
				info->handle, buffer->size);
		return ERR_PTR(-ENOMEM);
	}
#endif /* CONFIG_BCM_IOVMM */
	pr_debug("%16s: iommu map buffer(%p) da(%#x) pa(%#x) size(%#x)\n",
			heap->name, buffer, buffer->dma_addr, info->handle,
			buffer->size);
#endif /* CONFIG_IOMMU_API */
#endif /* CONFIG_ION_BCM */
	return info->table;
}

void ion_cma_heap_unmap_dma(struct ion_heap *heap,
			       struct ion_buffer *buffer)
{
#ifdef CONFIG_ION_BCM
#ifdef CONFIG_IOMMU_API
	struct ion_cma_buffer_info *info = buffer->priv_virt;
#ifdef CONFIG_BCM_IOVMM
	arm_iommu_unmap(heap->device, buffer->dma_addr, buffer->size);
#else
	iommu_unmap(heap->domain, buffer->dma_addr, buffer->size);
#endif /* CONFIG_BCM_IOVMM */
	pr_debug("%16s: iommu unmap buffer(%p) da(%#x) pa(%#x) size(%#x)\n",
			heap->name, buffer, buffer->dma_addr, info->handle,
			buffer->size);
#endif /* CONFIG_IOMMU_API */
	buffer->dma_addr = ION_DMA_ADDR_FAIL;
#endif /* CONFIG_ION_BCM */
	return;
}

static int ion_cma_mmap(struct ion_heap *mapper, struct ion_buffer *buffer,
			struct vm_area_struct *vma)
{
	struct ion_cma_heap *cma_heap =
		container_of(buffer->heap, struct ion_cma_heap, heap);
	struct device *dev = cma_heap->dev;
	struct ion_cma_buffer_info *info = buffer->priv_virt;

#ifdef CONFIG_ION_BCM
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
	return dma_mmap_coherent(dev, vma, info->cpu_addr, info->handle,
				 buffer->size);
#endif
}

#ifdef CONFIG_ION_BCM
int ion_cma_heap_clean_cache(struct ion_heap *heap,
		struct ion_buffer *buffer, unsigned long offset,
		unsigned long len)
{
	struct ion_cma_buffer_info *info = buffer->priv_virt;
	phys_addr_t pa;
	void *va;

	pa = info->handle;
	va = info->cpu_addr;
	pr_debug("clean: pa(%x) va(%p) off(%ld) len(%ld)\n",
			pa, va, offset, len);
	dmac_map_area(va + offset, len, DMA_BIDIRECTIONAL);
	outer_clean_range(pa + offset, pa + offset + len);

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
	dmac_unmap_area(va + offset, len, DMA_BIDIRECTIONAL);

	return 0;
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
#ifdef CONFIG_ION_BCM
	.clean_cache = ion_cma_heap_clean_cache,
	.invalidate_cache = ion_cma_heap_invalidate_cache,
#endif
};

#ifdef CONFIG_ION_BCM

static int ion_cma_heap_free_size(struct ion_heap *heap)
{
	struct ion_cma_heap *cma_heap =
		container_of(heap, struct ion_cma_heap, heap);

	return cma_heap->size - heap->used;
}

#ifdef CONFIG_ION_OOM_KILLER
static int ion_cma_lmk_shrink_info(struct ion_heap *heap, int *min_adj,
		int *min_free)
{
	struct ion_cma_heap *cma_heap =
		container_of(heap, struct ion_cma_heap, heap);
	int lmk_min_free = cma_heap->lmk_min_free;
	int free_size = ion_cma_heap_free_size(heap);

	if (!cma_heap->lmk_enable)
		return 0;
	if ((lmk_min_free < 0) || (lmk_min_free > 128)) {
		pr_err("lmk_min_free(%d) should be in [0-128] range\n",
				lmk_min_free);
		return 0;
	}
	*min_free = ((cma_heap->size / 128) * lmk_min_free) & PAGE_MASK;
	*min_adj = cma_heap->lmk_min_score_adj;
	if (free_size >= *min_free)
		return 0;

	return 1;
}

static int ion_cma_lmk_debugfs_add(struct ion_heap *heap,
		struct dentry *debug_root)
{
	char debug_name[64];
	struct ion_cma_heap *cma_heap =
		container_of(heap, struct ion_cma_heap, heap);

	snprintf(debug_name, 64, "lmk_%s", heap->name);
	cma_heap->lmk_debug_root = debugfs_create_dir(debug_name,
			debug_root);
	debugfs_create_u32("enable", (S_IRUGO|S_IWUSR),
			cma_heap->lmk_debug_root,
			(unsigned int *)&cma_heap->lmk_enable);
	debugfs_create_u32("oom_score_adj", (S_IRUGO|S_IWUSR),
			cma_heap->lmk_debug_root,
			(unsigned int *)&cma_heap->lmk_min_score_adj);
	debugfs_create_u32("min_free", (S_IRUGO|S_IWUSR),
			cma_heap->lmk_debug_root,
			(unsigned int *)&cma_heap->lmk_min_free);
	return 0;
}
#endif

static int ion_cma_heap_debug_show(struct ion_heap *heap, struct seq_file *s,
		void *unused)
{
	struct ion_cma_heap *cma_heap =
		container_of(heap, struct ion_cma_heap, heap);

	seq_printf(s, "%16.s: %6s(%s) %4s(%d) %6s(%u)KB %7s(%#08lx - %#08lx)\n",
			"CMA Heap", "Name", heap->name, "Id", heap->id,
			"Size", (cma_heap->size>>10), "Range",
			cma_heap->base,	(cma_heap->base + cma_heap->size));
#ifdef CONFIG_ION_OOM_KILLER
	if (cma_heap->lmk_enable) {
		int min_adj, min_free, free_size;

		ion_cma_lmk_shrink_info(heap, &min_adj, &min_free);
		free_size = ion_cma_heap_free_size(heap);

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


struct ion_heap *ion_cma_heap_create(struct ion_platform_heap *data)
{
	struct ion_cma_heap *cma_heap;
	struct ion_heap *heap;

	cma_heap = kzalloc(sizeof(struct ion_cma_heap), GFP_KERNEL);
	if (!cma_heap)
		return ERR_PTR(-ENOMEM);
	heap = &cma_heap->heap;

	heap->ops = &ion_cma_ops;
	/* set device as private heaps data, later it will be
	 * used to make the link with reserved CMA memory */
	heap->type = ION_HEAP_TYPE_DMA;
	cma_heap->dev = data->priv;
#ifdef CONFIG_ION_BCM
	cma_heap->base = data->base;
	cma_heap->size = data->size;
	heap->debug_show = ion_cma_heap_debug_show;
	heap->free_size = ion_cma_heap_free_size;
#endif
#ifdef CONFIG_ION_OOM_KILLER
	cma_heap->lmk_enable = data->lmk_enable;
	cma_heap->lmk_min_score_adj = data->lmk_min_score_adj;
	cma_heap->lmk_min_free = data->lmk_min_free;
	heap->lmk_shrink_info = ion_cma_lmk_shrink_info;
	heap->lmk_debugfs_add = ion_cma_lmk_debugfs_add;
#endif

	return heap;
}

void ion_cma_heap_destroy(struct ion_heap *heap)
{
	kfree(heap);
}
