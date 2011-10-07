/*
 * Contiguous Memory Allocator for DMA mapping framework
 * Copyright (c) 2010-2011 by Samsung Electronics.
 * Written by:
 *	Marek Szyprowski <m.szyprowski@xxxxxxxxxxx>
 *	Michal Nazarewicz <mina86@xxxxxxxxxx>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License or (at your optional) any later version of the license.
 */

#define pr_fmt(fmt) "cma: " fmt

#ifdef CONFIG_CMA_DEBUG
#ifndef DEBUG
#  define DEBUG
#endif
#endif

#include <asm/page.h>
#include <asm/dma-contiguous.h>

#include <linux/memblock.h>
#include <linux/err.h>
#include <linux/mm.h>
#include <linux/mutex.h>
#include <linux/page-isolation.h>
#include <linux/slab.h>
#include <linux/swap.h>
#include <linux/mm_types.h>
#include <linux/dma-contiguous.h>

#ifndef SZ_1M
#define SZ_1M (1 << 20)
#endif

#ifdef phys_to_pfn
/* nothing to do */
#elif defined __phys_to_pfn
#  define phys_to_pfn __phys_to_pfn
#elif defined __va
#  define phys_to_pfn(x) page_to_pfn(virt_to_page(__va(x)))
#else
#  error phys_to_pfn implementation needed
#endif

struct cma {
	unsigned long	base_pfn;
	unsigned long	count;
	unsigned long	*bitmap;
};

struct cma *dma_contiguous_default_area;

#ifndef CONFIG_CMA_SIZE_ABSOLUTE
#define CONFIG_CMA_SIZE_ABSOLUTE 0
#endif

#ifndef CONFIG_CMA_SIZE_PERCENTAGE
#define CONFIG_CMA_SIZE_PERCENTAGE 0
#endif

static unsigned long size_abs = CONFIG_CMA_SIZE_ABSOLUTE * SZ_1M;
static unsigned long size_percent = CONFIG_CMA_SIZE_PERCENTAGE;
static long size_cmdline = -1;

static int __init early_cma(char *p)
{
	pr_debug("%s(%s)\n", __func__, p);
	size_cmdline = memparse(p, &p);
	return 0;
}
early_param("cma", early_cma);

static unsigned long __init __cma_early_get_total_pages(void)
{
	struct memblock_region *reg;
	unsigned long total_pages = 0;

	/*
	 * We cannot use memblock_phys_mem_size() here, because
	 * memblock_analyze() has not been called yet.
	 */
	for_each_memblock(memory, reg)
		total_pages += memblock_region_memory_end_pfn(reg) -
			       memblock_region_memory_base_pfn(reg);
	return total_pages;
}

/**
 * dma_contiguous_reserve() - reserve area for contiguous memory handling
 *
 * This funtion reserves memory from early allocator. It should be
 * called by arch specific code once the early allocator (memblock or bootmem)
 * has been activated and all other subsystems have already allocated/reserved
 * memory.
 */
void __init dma_contiguous_reserve(phys_addr_t limit)
{
	unsigned long selected_size = 0;
	unsigned long total_pages;

	pr_debug("%s(limit %08lx)\n", __func__, (unsigned long)limit);

	total_pages = __cma_early_get_total_pages();
	size_percent *= (total_pages << PAGE_SHIFT) / 100;

	pr_debug("%s: total available: %ld MiB, size absolute: %ld MiB, size percentage: %ld MiB\n",
		 __func__, (total_pages << PAGE_SHIFT) / SZ_1M,
		size_abs / SZ_1M, size_percent / SZ_1M);

#ifdef CONFIG_CMA_SIZE_SEL_ABSOLUTE
	selected_size = size_abs;
#elif defined(CONFIG_CMA_SIZE_SEL_PERCENTAGE)
	selected_size = size_percent;
#elif defined(CONFIG_CMA_SIZE_SEL_MIN)
	selected_size = min(size_abs, size_percent);
#elif defined(CONFIG_CMA_SIZE_SEL_MAX)
	selected_size = max(size_abs, size_percent);
#endif

	if (size_cmdline != -1)
		selected_size = size_cmdline;

	if (!selected_size)
		return;

	pr_debug("%s: reserving %ld MiB for global area\n", __func__,
		 selected_size / SZ_1M);

	dma_declare_contiguous(NULL, selected_size, 0, limit);
};

static DEFINE_MUTEX(cma_mutex);

static void __cma_activate_area(unsigned long base_pfn, unsigned long count)
{
	unsigned long pfn = base_pfn;
	unsigned i = count >> pageblock_order;
	struct zone *zone;

	VM_BUG_ON(!pfn_valid(pfn));
	zone = page_zone(pfn_to_page(pfn));

	do {
		unsigned j;
		base_pfn = pfn;
		for (j = pageblock_nr_pages; j; --j, pfn++) {
			VM_BUG_ON(!pfn_valid(pfn));
			VM_BUG_ON(page_zone(pfn_to_page(pfn)) != zone);
		}
		init_cma_reserved_pageblock(pfn_to_page(base_pfn));
	} while (--i);
}

static struct cma *__cma_create_area(unsigned long base_pfn,
				     unsigned long count)
{
	int bitmap_size = BITS_TO_LONGS(count) * sizeof(long);
	struct cma *cma;

	pr_debug("%s(base %08lx, count %lx)\n", __func__, base_pfn, count);

	cma = kmalloc(sizeof *cma, GFP_KERNEL);
	if (!cma)
		return ERR_PTR(-ENOMEM);

	cma->base_pfn = base_pfn;
	cma->count = count;
	cma->bitmap = kzalloc(bitmap_size, GFP_KERNEL);

	if (!cma->bitmap)
		goto no_mem;

	__cma_activate_area(base_pfn, count);

	pr_debug("%s: returned %p\n", __func__, (void *)cma);
	return cma;

no_mem:
	kfree(cma);
	return ERR_PTR(-ENOMEM);
}

static struct cma_reserved {
	phys_addr_t start;
	unsigned long size;
	struct device *dev;
} cma_reserved[MAX_CMA_AREAS] __initdata;
static unsigned cma_reserved_count __initdata;

static int __init __cma_init_reserved_areas(void)
{
	struct cma_reserved *r = cma_reserved;
	unsigned i = cma_reserved_count;

	pr_debug("%s()\n", __func__);

	for (; i; --i, ++r) {
		struct cma *cma;
		cma = __cma_create_area(phys_to_pfn(r->start),
					r->size >> PAGE_SHIFT);
		if (!IS_ERR(cma)) {
			if (r->dev)
				set_dev_cma_area(r->dev, cma);
			else
				dma_contiguous_default_area = cma;
		}
	}
	return 0;
}
core_initcall(__cma_init_reserved_areas);

/**
 * dma_declare_contiguous() - reserve area for contiguous memory handling
 *			      for particular device
 * @dev:   Pointer to device structure.
 * @size:  Size of the reserved memory.
 * @start: Start address of the reserved memory (optional, 0 for any).
 * @limit: End address of the reserved memory (optional, 0 for any).
 *
 * This funtion reserves memory for specified device. It should be
 * called by board specific code when early allocator (memblock or bootmem)
 * is still activate.
 */
int __init dma_declare_contiguous(struct device *dev, unsigned long size,
				  phys_addr_t base, phys_addr_t limit)
{
	struct cma_reserved *r = &cma_reserved[cma_reserved_count];
	unsigned long alignment;

	pr_debug("%s(size %lx, base %08lx, limit %08lx)\n", __func__,
		 (unsigned long)size, (unsigned long)base,
		 (unsigned long)limit);

	/* Sanity checks */
	if (cma_reserved_count == ARRAY_SIZE(cma_reserved))
		return -ENOSPC;

	if (!size)
		return -EINVAL;

	/* Sanitise input arguments */
	alignment = PAGE_SIZE << max(MAX_ORDER, pageblock_order);
	base = ALIGN(base, alignment);
	size = ALIGN(size, alignment);
	limit = ALIGN(limit, alignment);

	/* Reserve memory */
	if (base) {
		if (memblock_is_region_reserved(base, size) ||
		    memblock_reserve(base, size) < 0) {
			base = -EBUSY;
			goto err;
		}
	} else {
		/*
		 * Use __memblock_alloc_base() since
		 * memblock_alloc_base() panic()s.
		 */
		phys_addr_t addr = __memblock_alloc_base(size, alignment, limit);
		if (!addr) {
			base = -ENOMEM;
			goto err;
		} else if (addr + size > ~(unsigned long)0) {
			memblock_free(addr, size);
			base = -EOVERFLOW;
			goto err;
		} else {
			base = addr;
		}
	}

	/*
	 * Each reserved area must be initialised later, when more kernel
	 * subsystems (like slab allocator) are available.
	 */
	r->start = base;
	r->size = size;
	r->dev = dev;
	cma_reserved_count++;
	printk(KERN_INFO "CMA: reserved %ld MiB at %08lx\n", size / SZ_1M,
	       (unsigned long)base);

	/*
	 * Architecture specific contiguous memory fixup.
	 */
	dma_contiguous_early_fixup(base, size);
	return 0;
err:
	printk(KERN_ERR "CMA: failed to reserve %ld MiB\n", size / SZ_1M);
	return base;
}

/**
 * dma_alloc_from_contiguous() - allocate pages from contiguous area
 * @dev:   Pointer to device for which the allocation is performed.
 * @count: Requested number of pages.
 * @align: Requested alignment of pages (in PAGE_SIZE order).
 *
 * This funtion allocates memory buffer for specified device. It uses
 * device specific contiguous memory area if available or the default
 * global one. Requires architecture specific get_dev_cma_area() helper
 * function.
 */
struct page *dma_alloc_from_contiguous(struct device *dev, int count,
				       unsigned int align)
{
	struct cma *cma = get_dev_cma_area(dev);
	unsigned long pfn, pageno;
	int ret;

	if (!cma)
		return NULL;

	if (align > CONFIG_CMA_ALIGNMENT)
		align = CONFIG_CMA_ALIGNMENT;

	pr_debug("%s(cma %p, count %d, align %d)\n", __func__, (void *)cma,
		 count, align);

	if (!count)
		return NULL;

	mutex_lock(&cma_mutex);

	pageno = bitmap_find_next_zero_area(cma->bitmap, cma->count, 0, count,
					    (1 << align) - 1);
	if (pageno >= cma->count) {
		ret = -ENOMEM;
		goto error;
	}
	bitmap_set(cma->bitmap, pageno, count);

	pfn = cma->base_pfn + pageno;
	ret = alloc_contig_range(pfn, pfn + count, 0, MIGRATE_CMA);
	if (ret)
		goto free;

	mutex_unlock(&cma_mutex);

	pr_debug("%s(): returned %p\n", __func__, pfn_to_page(pfn));
	return pfn_to_page(pfn);
free:
	bitmap_clear(cma->bitmap, pageno, count);
error:
	mutex_unlock(&cma_mutex);
	return NULL;
}

/**
 * dma_release_from_contiguous() - release allocated pages
 * @dev:   Pointer to device for which the pages were allocated.
 * @pages: Allocated pages.
 * @count: Number of allocated pages.
 *
 * This funtion releases memory allocated by dma_alloc_from_contiguous().
 * It return 0 when provided pages doen't belongs to contiguous area and
 * 1 on success.
 */
int dma_release_from_contiguous(struct device *dev, struct page *pages,
				int count)
{
	struct cma *cma = get_dev_cma_area(dev);
	unsigned long pfn;

	if (!cma || !pages)
		return 0;

	pr_debug("%s(page %p)\n", __func__, (void *)pages);

	pfn = page_to_pfn(pages);

	if (pfn < cma->base_pfn || pfn >= cma->base_pfn + cma->count)
		return 0;

	mutex_lock(&cma_mutex);

	bitmap_clear(cma->bitmap, pfn - cma->base_pfn, count);
	free_contig_pages(pfn, count);

	mutex_unlock(&cma_mutex);
	return 1;
}
