/*
 * Contiguous Memory Allocator for DMA mapping framework
 * Copyright (c) 2010-2011 by Samsung Electronics.
 * Written by:
 *	Marek Szyprowski <m.szyprowski@samsung.com>
 *	Michal Nazarewicz <mina86@mina86.com>
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
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/rmap.h>
#include <linux/module.h>
#ifdef CONFIG_CMA_DEBUG_FS
#include <linux/debugfs.h>
#endif
#include "dma-contiguous-trace.h"

#ifndef SZ_1M
#define SZ_1M (1 << 20)
#endif

#define toKB(x) ((x) << (PAGE_SHIFT-10))

#ifdef CONFIG_CMA_STATS

struct cma_allocation {
	struct list_head clink;
	struct device *dev;
	unsigned long pfn_start;
	unsigned long count;
	unsigned int align;
};

static void update_alloc_list(struct device *dev, struct cma *cma,
			      unsigned long pfn, unsigned long count,
			      unsigned int align, int is_alloc);

#else

#define update_alloc_list(d, c, p, n, a, i)	do { } while (0)

#endif /* CONFIG_CMA_STATS */

struct cma {
#ifdef CONFIG_CMA_STATS
	struct list_head clist;
#endif
	unsigned long base_pfn;
	unsigned long count;
	unsigned long *bitmap;
#ifdef CONFIG_CMA_BEST_FIT
	unsigned long *bf_bitmap;
#endif
	struct dev_cma_info info;
};

struct cma *dma_contiguous_default_area;

static DEFINE_MUTEX(cma_mutex);

#ifdef CONFIG_CMA_SIZE_MBYTES
#define CMA_SIZE_MBYTES CONFIG_CMA_SIZE_MBYTES
#else
#define CMA_SIZE_MBYTES 0
#endif

/* Must be called with cma_mutex held */
static void recalculate_cma_region_stats(struct cma *cma)
{
	unsigned long idx;
	int start, end;
	unsigned long max_free_block = 0;

	cma->info.total_alloc = 0;

	for (start = 0; start < cma->count; start = end) {
		idx = bitmap_find_next_zero_area(cma->bitmap, cma->count,
						 start, 1, 0);
		if (unlikely(idx >= cma->count)) {
			cma->info.total_alloc += cma->count - start;
			break;
		} else {
			cma->info.total_alloc += idx - start;
		}

		end = find_next_bit(cma->bitmap, cma->count, idx);
		if ((end - idx) > max_free_block)
			max_free_block = end - idx;
	}

	if (cma->info.total_alloc > cma->info.peak_alloc)
		cma->info.peak_alloc = cma->info.total_alloc;

	cma->info.max_free_block = max_free_block;
}

/* debugfs related functions */
#ifdef CONFIG_CMA_DEBUG_FS

/* when using debugfs_create_file private data field is set on inode
 * copy data pointer to file private data */
static int cma_debugfs_open(struct inode *inode, struct file *file)
{
	file->private_data = inode->i_private;
	return 0;
}

static ssize_t cma_debugfs_read(struct file *file, char __user * user_buf,
				size_t size, loff_t *ppos)
{
	struct cma *cma = file->private_data;
	char *buf;
	unsigned int len = 0, count = 0;

	/* bitmap_scnprintf() produces one hex-digit per 4 bits of the
	 * bitmap and one comma per every 8 hex-digits meaning par every
	 * 32 bits, add 64 bytes for the additional characters
	 */
	unsigned int buffer_size = (cma->count / 4) + (cma->count / 32) + 64;

	/* Add 1K for printing in stats at the end of bitmap */
	buffer_size += SZ_1K;

	buf = kmalloc(buffer_size, GFP_KERNEL);
	if (!buf)
		return 0;

	len +=
	    snprintf(buf + len, buffer_size - len,
		     "CMA base 0x%08lx, count %lx\n", cma->base_pfn,
		     cma->count);

	len += snprintf(buf + len, buffer_size - len, "CMA bitmap : ");

	mutex_lock(&cma_mutex);
	len +=
	    bitmap_scnprintf(buf + len, buffer_size - len, cma->bitmap,
			     cma->count);

	len += snprintf(buf + len, buffer_size - len, "\n");

	len += snprintf(buf + len, buffer_size - len,
			"==================================================\n");
	len += snprintf(buf + len, buffer_size - len,
			"Total allocation     : %lu Pages, %lukB\n",
			cma->info.total_alloc, toKB(cma->info.total_alloc));
	len += snprintf(buf + len, buffer_size - len,
			"Total free           : %lu Pages, %lukB\n",
			cma->count - cma->info.total_alloc,
			toKB(cma->count - cma->info.total_alloc));
	len += snprintf(buf + len, buffer_size - len,
			"Largest Free Block   : %lu Pages, %lukB\n",
			cma->info.max_free_block,
			toKB(cma->info.max_free_block));
	len += snprintf(buf + len, buffer_size - len,
			"Peak Allocation      : %lu Pages, %lukB\n",
			cma->info.peak_alloc, toKB(cma->info.peak_alloc));

	len += snprintf(buf + len, buffer_size - len, "\n");

	mutex_unlock(&cma_mutex);

	if (len > buffer_size)
		len = buffer_size;

	count = simple_read_from_buffer(user_buf, size, ppos, buf, len);

	kfree(buf);

	return count;
}

static const struct file_operations cma_debugfs_fops = {
	.open = cma_debugfs_open,
	.read = cma_debugfs_read,
	.llseek = default_llseek,
	.owner = THIS_MODULE,
};

static int region_count = 0;
static char debugfs_cma_file[MAX_CMA_AREAS][6];
static void cma_debugfs_create_file(struct cma *cma)
{
	BUG_ON(unlikely(region_count >= MAX_CMA_AREAS));
	sprintf(&debugfs_cma_file[region_count][0], "cma%d", region_count);
	debugfs_create_file(&debugfs_cma_file[region_count][0],
			    S_IRUSR, NULL, cma, &cma_debugfs_fops);
	region_count++;
}

#else /* !CONFIG_CMA_DEBUG_FS */

static inline void cma_debugfs_create_file(struct cma *cma)
{
}

#endif /* COFNIG_CMA_DEBUG_FS */

/*
 * Default global CMA area size can be defined in kernel's .config.
 * This is usefull mainly for distro maintainers to create a kernel
 *that works correctly for most supported systems.
 * The size can be set in bytes or as a percentage of the total memory
 *in the system.
 *
 * Users, who want to set the size of global CMA area for their system
 *should use cma= kernel parameter.
 */
static const unsigned long size_bytes = CMA_SIZE_MBYTES * SZ_1M;
static long size_cmdline = -1;

static int __init early_cma(char *p)
{
	pr_debug("%s(%s)\n", __func__, p);
	size_cmdline = memparse(p, &p);
	return 0;
}

early_param("cma", early_cma);

#ifdef CONFIG_CMA_SIZE_PERCENTAGE

static unsigned long __init __maybe_unused cma_early_percent_memory(void)
{
	struct memblock_region *reg;
	unsigned long total_pages = 0;

	/*
	 * We cannot use memblock_phys_mem_size() here, because
	 *memblock_analyze() has not been called yet.
	 */
	for_each_memblock(memory, reg)
	    total_pages += memblock_region_memory_end_pfn(reg) -
	    memblock_region_memory_base_pfn(reg);

	return (total_pages * CONFIG_CMA_SIZE_PERCENTAGE / 100) << PAGE_SHIFT;
}

#else

static inline __maybe_unused unsigned long cma_early_percent_memory(void)
{
	return 0;
}

#endif

/**
 *dma_contiguous_reserve() - reserve area for contiguous memory handling
 *@limit: End address of the reserved memory (optional, 0 for any).
 *
 *This function reserves memory from early allocator. It should be
 *called by arch specific code once the early allocator (memblock or bootmem)
 *has been activated and all other subsystems have already allocated/reserved
 *memory.
 */
void __init dma_contiguous_reserve(phys_addr_t limit)
{
	unsigned long selected_size = 0;

	pr_debug("%s(limit %08lx)\n", __func__, (unsigned long)limit);

	if (size_cmdline != -1) {
		selected_size = size_cmdline;
	} else {
#ifdef CONFIG_CMA_SIZE_SEL_MBYTES
		selected_size = size_bytes;
#elif defined(CONFIG_CMA_SIZE_SEL_PERCENTAGE)
		selected_size = cma_early_percent_memory();
#elif defined(CONFIG_CMA_SIZE_SEL_MIN)
		selected_size = min(size_bytes, cma_early_percent_memory());
#elif defined(CONFIG_CMA_SIZE_SEL_MAX)
		selected_size = max(size_bytes, cma_early_percent_memory());
#endif
	}

	if (selected_size) {
		pr_debug("%s: reserving %ld MiB for global area\n", __func__,
			 selected_size / SZ_1M);
		dma_declare_contiguous(NULL, selected_size, 0, limit);
	}
}

#ifdef CONFIG_CMA_STATS
/* Should be called with cma_mutex held */
static
void update_alloc_list(struct device *dev, struct cma *cma, unsigned long pfn,
		       unsigned long count, unsigned int align, int is_alloc)
{
	struct cma_allocation *p, *next;
	struct cma_allocation *new;

	/* First, some range checks */
	if ((pfn < cma->base_pfn) ||
	    ((cma->base_pfn + cma->count) < (pfn + count))) {
		printk(KERN_ALERT
		       "cma allocations(0x%08lx+0x%lx) is out of range ?!\n",
		       cma, pfn, count);
		goto done;
	}

	if (is_alloc) {
		if (align > CONFIG_CMA_ALIGNMENT) {
			printk(KERN_ALERT "Invalid Alignment (%u)\n", align);
			goto done;
		}

		new = kzalloc(sizeof(*new), GFP_KERNEL);
		if (!new) {
			printk(KERN_ALERT
			       "Failed to allocate memory for (cma_allocation)\n");
			goto done;
		}

		new->dev = dev;
		new->pfn_start = pfn;
		new->count = count;
		new->align = align;

		list_add_tail(&new->clink, &cma->clist);
	} else {

		BUG_ON(list_empty(&cma->clist));

		/* If this is a release, then just delete the node
		 *corresponding to (pfn, count)
		 */
		list_for_each_entry_safe(p, next, &cma->clist, clink) {
			if (p->pfn_start == pfn) {
				BUG_ON(p->count != count);
				list_del_init(&p->clink);
				kfree(p);
				goto done;
			}
		}
		BUG();
	}
done:
	return;
}

#endif /* CONFIG_CMA_STATS */

static __init int cma_activate_area(unsigned long base_pfn, unsigned long count)
{
	unsigned long pfn = base_pfn;
	unsigned i = count >> pageblock_order;
	struct zone *zone;

	WARN_ON_ONCE(!pfn_valid(pfn));
	zone = page_zone(pfn_to_page(pfn));

	do {
		unsigned j;
		base_pfn = pfn;
		for (j = pageblock_nr_pages; j; --j, pfn++) {
			WARN_ON_ONCE(!pfn_valid(pfn));
			if (page_zone(pfn_to_page(pfn)) != zone)
				return -EINVAL;
		}
		init_cma_reserved_pageblock(pfn_to_page(base_pfn));
	} while (--i);
	return 0;
}

static __init struct cma *cma_create_area(unsigned long base_pfn,
					  unsigned long count)
{
	int bitmap_size = BITS_TO_LONGS(count) * sizeof(long);
	struct cma *cma;
	int ret = -ENOMEM;

	pr_debug("%s(base %08lx, count %lx)\n", __func__, base_pfn, count);

	cma = kmalloc(sizeof *cma, GFP_KERNEL);
	if (!cma)
		return ERR_PTR(-ENOMEM);

	cma->base_pfn = base_pfn;
	cma->count = count;
	cma->bitmap = kzalloc(bitmap_size, GFP_KERNEL);
	if (!cma->bitmap)
		goto no_mem;

#ifdef CONFIG_CMA_BEST_FIT
	cma->bf_bitmap = kzalloc(bitmap_size, GFP_KERNEL);
	if (!cma->bf_bitmap)
		goto error;
#endif

	ret = cma_activate_area(base_pfn, count);
	if (ret)
		goto error;

#ifdef CONFIG_CMA_STATS
	INIT_LIST_HEAD(&cma->clist);
#endif
	cma->info.total_alloc = cma->info.peak_alloc = 0UL;
	cma->info.max_free_block = cma->info.nr_pages = cma->count;
	cma->info.start_pfn = base_pfn;

	cma_debugfs_create_file(cma);

	pr_debug("%s: returned %p\n", __func__, (void *)cma);
	return cma;

error:
#ifdef CONFIG_CMA_BEST_FIT
	kfree(cma->bf_bitmap);
#endif
	kfree(cma->bitmap);
no_mem:
	kfree(cma);
	return ERR_PTR(ret);
}

static struct cma_reserved {
	phys_addr_t start;
	unsigned long size;
	struct device *dev;
#ifdef CONFIG_CMA_STATS
} cma_reserved[MAX_CMA_AREAS];
static unsigned cma_reserved_count;
#else
} cma_reserved[MAX_CMA_AREAS] __initdata;
static unsigned cma_reserved_count __initdata;
#endif

static int __init cma_init_reserved_areas(void)
{
	struct cma_reserved *r = cma_reserved;
	unsigned i = cma_reserved_count;

	pr_debug("%s()\n", __func__);

	for (; i; --i, ++r) {
		struct cma *cma;
		cma = cma_create_area(PFN_DOWN(r->start),
				      r->size >> PAGE_SHIFT);
		if (!IS_ERR(cma))
			dev_set_cma_area(r->dev, cma);
	}
	return 0;
}

core_initcall(cma_init_reserved_areas);

/**
 *dma_declare_contiguous() - reserve area for contiguous memory handling
 *			      for particular device
 *@dev:   Pointer to device structure.
 *@size:  Size of the reserved memory.
 *@base:  Start address of the reserved memory (optional, 0 for any).
 *@limit: End address of the reserved memory (optional, 0 for any).
 *
 * This function reserves memory for specified device. It should be
 *called by board specific code when early allocator (memblock or bootmem)
 *is still activate.
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
	if (cma_reserved_count == ARRAY_SIZE(cma_reserved)) {
		pr_err("Not enough slots for CMA reserved regions!\n");
		return -ENOSPC;
	}

	if (!size)
		return -EINVAL;

	/* Sanitise input arguments */
	alignment = PAGE_SIZE << max(MAX_ORDER, pageblock_order);
	base = ALIGN(base, alignment);
	size = ALIGN(size, alignment);
	limit &= ~(alignment - 1);

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
		 *memblock_alloc_base() panic()s.
		 */
		phys_addr_t addr =
		    __memblock_alloc_base(size, alignment, limit);
		if (!addr) {
			base = -ENOMEM;
			goto err;
		} else if (addr + size > ~(unsigned long)0) {
			memblock_free(addr, size);
			base = -EINVAL;
			goto err;
		} else {
			base = addr;
		}
	}

	/*
	 * Each reserved area must be initialised later, when more kernel
	 *subsystems (like slab allocator) are available.
	 */
	r->start = base;
	r->size = size;
	r->dev = dev;
	cma_reserved_count++;
	pr_info("CMA: reserved %ld MiB at %08lx\n", size / SZ_1M,
		(unsigned long)base);

	/* Architecture specific contiguous memory fixup. */
	dma_contiguous_early_fixup(base, size);
	return 0;
err:
	pr_err("CMA: failed to reserve %ld MiB\n", size / SZ_1M);
	return base;
}

/**
 *get_cma_area() - get start and length of cma region for device
 *@dev:   Pointer to device for which the CMA region is to be found.
 *@start: Physical start of CMA region
 *@size:  Size of device's CMA region
 *
 * This functions finds device specific CMA region (or default region
 *when there is no CMA region associated with this device), and returns
 *the start and size of that region.
 */

void get_dev_cma_info(struct device *dev, struct dev_cma_info *info)
{
	struct cma *cma;

	/* if we haven't even been initialised yet */
	if (!dma_contiguous_default_area) {
		printk(KERN_WARNING "%s: CMA not initialised !\n", __func__);
		return;
	}

	cma = dev_get_cma_area(dev);
	if (!cma)
		return;
	mutex_lock(&cma_mutex);
	memcpy(info, &cma->info, sizeof(*info));
	mutex_unlock(&cma_mutex);
}

#ifdef CONFIG_CMA_BEST_FIT

struct cma_range {
	unsigned long start;
	unsigned long end;
};

static int find_best_area(struct cma *cma, int count,
			  unsigned long mask, struct cma_range *best_fit)
{
	unsigned long index;
	int start, end;

	best_fit->start = 0UL;
	best_fit->end = ULONG_MAX;

	for (start = 0; start < cma->count; start = end) {
		index = bitmap_find_next_zero_area(cma->bitmap, cma->count,
						   start, count, mask);
		if (index >= cma->count)
			break;

		end = find_next_bit(cma->bitmap, cma->count, index);

		/* check if we've already tried this */
		if (!test_bit(index, cma->bf_bitmap)) {
			/* Best case */
			if ((end - index) == count) {
				best_fit->start = index;
				best_fit->end = end;
				break;
			}

			/* This is the smallest possible area that is bigger
			 * than the allocation request so far. We just remember
			 * this in "best_fit", but dont set the bitmap bit yet,
			 * we may find a better area ahead
			 */
			if ((end - index) < (best_fit->end - best_fit->start)) {
				best_fit->start = index;
				best_fit->end = end;
			}
		}
	}

	if (unlikely(best_fit->end == ULONG_MAX)) {
		printk(KERN_ERR "%s: Failed to find free pages\n", __func__);
		return -ENOMEM;
	}

	pr_debug("%s: found area at start(%lu), end(%lu)\n",
		 __func__, best_fit->start, best_fit->end);

	__set_bit(best_fit->start, cma->bf_bitmap);

	return 0;
}
#endif /* CONFIG_CMA_BEST_FIT */

/**
 *dma_alloc_from_contiguous() - allocate pages from contiguous area
 *@dev:   Pointer to device for which the allocation is performed.
 *@count: Requested number of pages.
 *@align: Requested alignment of pages (in PAGE_SIZE order).
 *
 *This function allocates memory buffer for specified device. It uses
 *device specific contiguous memory area if available or the default
 *global one. Requires architecture specific get_dev_cma_area() helper
 *function.
 */
struct page *dma_alloc_from_contiguous(struct device *dev, int count,
				       unsigned int align)
{
	unsigned long mask, pfn, pageno;
	struct cma *cma = dev_get_cma_area(dev);
	struct page *page;
	int ret;
#ifdef CONFIG_CMA_BEST_FIT
	struct cma_range best_fit;
#else
	unsigned long start = 0;
#endif

	if (!cma || !cma->count)
		return NULL;

	if (align > CONFIG_CMA_ALIGNMENT)
		align = CONFIG_CMA_ALIGNMENT;

	pr_debug("%s(cma %p, count %d, align %d)\n", __func__, (void *)cma,
		 count, align);

	if (!count || count > cma->count)
		return NULL;

	mask = (1 << align) - 1;

	trace_cma_alloc_start(cma, count, align);

	mutex_lock(&cma_mutex);

#ifndef CONFIG_CMA_BEST_FIT
	for (;;) {
		pageno = bitmap_find_next_zero_area(cma->bitmap, cma->count,
						    start, count, mask);
		if (pageno >= cma->count) {
			pr_debug("%s:%d ## CMA ALLOCATION FAILED ##\n",
			       __func__, __LINE__);
			pr_debug("%s:%d # Couldn't find %d pages in cma\n",
			       __func__, __LINE__, count);
			pr_debug("%s:%d # Total allocation(%lukB, %ld pages)"
			       " Largest free block(%lukB, %ld pages)\n",
			       __func__, __LINE__,
			       toKB(cma->info.total_alloc),
			       cma->info.total_alloc,
			       toKB(cma->info.max_free_block),
			       cma->info.max_free_block);
			ret = -ENOMEM;
			goto error;
		}

		pfn = cma->base_pfn + pageno;
		ret = alloc_contig_range(pfn, pfn + count, MIGRATE_CMA);
		if (ret == 0) {
			bitmap_set(cma->bitmap, pageno, count);
			break;
		} else if (ret != -EBUSY) {
			printk(KERN_ERR
			       "%s:%d ## CMA alloc_contig_range failed ##"
			       " ret = %d for range(%08x-%08x)\n",
			       __func__, __LINE__, ret, __pfn_to_phys(pfn),
			       __pfn_to_phys(pfn + count));
			goto error;
		}

		pr_debug
		    ("%s(): memory range at %p is busy !\n",
		     __func__, pfn_to_page(pfn));

		/* try again with a bit different memory target */
		start = pageno + mask + 1;
	}

#else /* !CONFIG_CMA_BEST_FIT */
	for (;;) {
		ret = find_best_area(cma, count, mask, &best_fit);
		if (ret) {
			pr_debug("%s:%d #### CMA ALLOCATION FAILED ####\n",
			       __func__, __LINE__);
			pr_debug("%s:%d # Couldn't find %d pages in cma\n",
			       __func__, __LINE__, count);

			pr_debug("%s:%d # Total alloc(%lukB, %ld pages)"
			       " Largest block(%lukB, %ld pages)\n",
			       __func__, __LINE__,
			       toKB(cma->info.total_alloc),
			       cma->info.total_alloc,
			       toKB(cma->info.max_free_block),
			       cma->info.max_free_block);
			ret = -ENOMEM;
			show_mem(SHOW_MEM_FILTER_NODES);
			break;
		}

		/* Now try and allocate from the *best* cma area for this size.
		 * Note that area can be bigger than the requested allocation.
		 * So, if migration fails, we keep retrying untill we reach end
		 * of this area. If we have failed allocation here, then we
		 * continue the outer loop and find the next-best area for
		 * this allocation
		 **/

		pr_debug("Alloc (%d) pages from (%lu count:%lu, size:%lu)\n",
			 count, best_fit.start, best_fit.end,
			 best_fit.end - best_fit.start);

		for (pageno = best_fit.start, ret = -EBUSY;
		     ((pageno + count) <= best_fit.end) && ret == -EBUSY;
		     pageno = __ALIGN_MASK(pageno + 1, mask)) {

			pfn = cma->base_pfn + pageno;

			pr_debug("alloc range:%ld-%ld\n", pageno,
				 pageno + count);
			ret = alloc_contig_range(pfn, pfn + count, MIGRATE_CMA);
			if (ret == 0) {
				bitmap_set(cma->bitmap, pageno, count);
				pr_debug("allocation successful\n");
				break;
			}
		}

		if (likely(ret == 0))
			break;

		if (unlikely(ret != -EBUSY)) {
			printk(KERN_ERR
			       "%s:%d CMA alloc_contig_range failed"
			       " ret = %d for range(%08x-%08x)\n",
			       __func__, __LINE__, ret,
			       __pfn_to_phys(cma->base_pfn + pageno),
			       __pfn_to_phys(cma->base_pfn + pageno + count));
			break;
		}
	}

	/* Clear the best_fit bitmap here */
	memset(cma->bf_bitmap, 0, BITS_TO_LONGS(cma->count) * sizeof(long));

	/* if we are here, and ret is true, we have failed */
	if (ret)
		goto error;

#endif /* !CONFIG_CMA_BEST_FIT */

	page = pfn_to_page(pfn);
	__mod_zone_page_state(page_zone(page), NR_CONTIG_PAGES, count);
	recalculate_cma_region_stats(cma);

	update_alloc_list(dev, cma, pfn, count, align, 1);

	mutex_unlock(&cma_mutex);

	pr_debug("%s(): returned %p\n", __func__, page);

	trace_cma_alloc_end_success(cma, pfn, count);

	return page;
error:
	trace_cma_alloc_end_failed(cma, count);
	mutex_unlock(&cma_mutex);
	return NULL;
}

/**
 *dma_release_from_contiguous() - release allocated pages
 *@dev:   Pointer to device for which the pages were allocated.
 *@pages: Allocated pages.
 *@count: Number of allocated pages.
 *
 * This function releases memory allocated by dma_alloc_from_contiguous().
 * It returns false when provided pages do not belong to contiguous area and
 * true otherwise.
 */
bool dma_release_from_contiguous(struct device *dev, struct page *pages,
				int count)
{
	struct cma *cma = dev_get_cma_area(dev);
	unsigned long pfn;

	if (!cma || !pages)
		return false;

	pr_debug("%s(page %p)\n", __func__, (void *)pages);

	pfn = page_to_pfn(pages);

	if (pfn < cma->base_pfn || pfn >= cma->base_pfn + cma->count)
		return false;

	trace_cma_release_start(cma, pfn, count);

	VM_BUG_ON(pfn + count > cma->base_pfn + cma->count);

	mutex_lock(&cma_mutex);
	bitmap_clear(cma->bitmap, pfn - cma->base_pfn, count);
	free_contig_range(pfn, count);
	update_alloc_list(dev, cma, pfn, count, 0, 0);
	__mod_zone_page_state(page_zone(pages), NR_CONTIG_PAGES, -count);
	recalculate_cma_region_stats(cma);
	mutex_unlock(&cma_mutex);

	trace_cma_release_end(cma, pfn, count);

	return true;
}

#ifdef CONFIG_CMA_STATS
#ifdef CONFIG_PROC_FS

static void *cma_start(struct seq_file *m, loff_t * pos)
{
	struct cma_reserved *region = (struct cma_reserved *)m->private;

	if (!cma_reserved_count || *pos >= cma_reserved_count)
		return NULL;

	return region + *pos;
}

static void *cma_next(struct seq_file *m, void *arg, loff_t * pos)
{
	struct cma_reserved *region = (struct cma_reserved *)m->private;

	*pos = *pos + 1;

	if (*pos == cma_reserved_count)
		return NULL;

	seq_putc(m, '\n');
	seq_putc(m, '\n');

	return region + *pos;
}

static void cma_stop(struct seq_file *m, void *arg)
{
}

static int cmastat_show(struct seq_file *m, void *arg)
{
	struct cma_reserved *region = (struct cma_reserved *)arg;
	struct cma *cma;
	struct cma_allocation *p;
	const char *region_name;
	const char *device_name;

	if (region->dev) {
		cma = dev_get_cma_area(region->dev);
		region_name = dev_name(region->dev);
	} else {
		cma = dma_contiguous_default_area;
		region_name = "Default Region";
	}

	seq_printf(m, "%-20s : (%08lx + %lx)", region_name, cma->base_pfn,
		   cma->count);
	seq_putc(m, '\n');
	seq_printf(m, "%-20s :   %-20s %-15s %-20s %-10s", "Device Name",
		   "Number of Pages", "Alignment", "Address Range", "Size");
	seq_putc(m, '\n');

	mutex_lock(&cma_mutex);
	list_for_each_entry(p, &cma->clist, clink) {
		if (p->dev) {
			device_name = dev_name(p->dev);
		} else {
			device_name = "Unknown";
		}
		seq_printf(m, "%-20s :", device_name);
		seq_printf(m, "%15ld", p->count);
		seq_printf(m, "%14u       %10lx-%08lx %7ldkB",
			   p->align,
			   ((p->pfn_start) << PAGE_SHIFT),
			   ((p->pfn_start + p->count) << PAGE_SHIFT),
			   toKB(p->count));
		seq_putc(m, '\n');

		/* Check of this line was successfully copied by seq_printf()
		 *if not, break out and return. We well get another _start
		 *call with larger buffer
		 */
		if (m->count >= m->size)
			break;
	}

	if (m->count < m->size) {
		seq_putc(m, '\n');
		seq_printf(m, "Total allocation     : %lu Pages, %lukB\n",
			   cma->info.total_alloc,
			   (cma->info.total_alloc * PAGE_SIZE) / SZ_1K);
		seq_printf(m, "Total free           : %lu Pages, %lukB\n",
			   cma->count - cma->info.total_alloc,
			   toKB(cma->count - cma->info.total_alloc));
		seq_printf(m, "Largest Free Block   : %lu Pages, %lukB\n",
			   cma->info.max_free_block,
			   toKB(cma->info.max_free_block));
		seq_printf(m, "Peak Allocation      : %lu Pages, %lukB\n",
			   cma->info.peak_alloc, toKB(cma->info.peak_alloc));
		seq_printf(m,
			   "=================================================");
		seq_putc(m, '\n');
		seq_putc(m, '\n');
	}

	mutex_unlock(&cma_mutex);

	return 0;
}

static const struct seq_operations cmastat_op = {
	.start = cma_start,
	.next = cma_next,
	.stop = cma_stop,
	.show = cmastat_show,
};

static int cmastat_open(struct inode *inode, struct file *file)
{
	int ret;
	ret = seq_open(file, &cmastat_op);
	if (!ret) {
		struct seq_file *m = file->private_data;
		m->private = &cma_reserved;
	}

	return ret;
}

static const struct file_operations cmastat_file_ops = {
	.open = cmastat_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
};

static int __init cma_stats_init(void)
{
	proc_create("cmastat", S_IRUGO, NULL, &cmastat_file_ops);
	return 0;
}

late_initcall(cma_stats_init);

#endif
#endif /* CONFIG_CMA_STATS */
