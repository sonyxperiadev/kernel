/*
 * drivers/gpu/m4u/m4u.c
 *
 * Copyright (C) 2012 Broadcom, Inc.
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

#define pr_fmt(fmt) "m4u: " fmt

#include <linux/file.h>
#include <linux/fs.h>
#include <linux/err.h>
#include <linux/list.h>
#include <linux/platform_device.h>
#ifdef CONFIG_DEBUG_M4U
#include <linux/miscdevice.h>
#endif
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/dma-mapping.h>
#include <linux/log2.h>
#include <linux/debugfs.h>
#include <linux/broadcom/m4u.h>

#include <mach/irqs.h>
#include <mach/rdb/brcm_rdb_sysmap.h>
#include <mach/rdb/brcm_rdb_mmmmu_open.h>

#define BCM_INT_ID_M4U_OPEN		(BCM_INT_ID_RESERVED181)
#define	M4U_PAGE_SHIFT			(12)
#define	M4U_TLB_LINE_SHIFT		(3)
#define	M4U_TLB_LINE_SIZE		(1<<M4U_TLB_LINE_SHIFT)

/**
 * m4u_mapping - m4u mapping structure
 * @ref:		reference count
 * @list:		node pointer to chain m4u mappings
 * @mdev:		m4u device
 * @region:		Region definition
 * @sgt:		scatter-gather list of the physical address space
 * @contig_flag	Contiguous buffer or sglist buffer
 * @locked		Mapping is locked in TLB
 *
 * Modifications to mapping list, or mapping should be protected by
 * external lock.
 */
struct m4u_mapping {
	struct kref ref;
	struct list_head list;
	struct m4u_device *mdev;
	struct m4u_region region;
	struct sg_table	sgt;
	u32 contig_flag;
	u32 locked;
};

/* To be moved to board file - Start */
static struct m4u_region m4u_regions[] = {
	[0] = {
		.mma			= 0x80000000,
		.pa				= 0x80000000,
		.size			= SZ_128M,
		.page_size		= SZ_1M,
	},
};

static struct m4u_platform_data m4u_pdata = {
	.mma_begin		= 0x80000000,
	.mma_size		= (SZ_2G - SZ_4M),
	.xfifo_size		= SZ_16K,
	.nr				= ARRAY_SIZE(m4u_regions),
	.regions		= m4u_regions,
};

static struct resource m4u_resources[] = {
	[0] = {
	       .start = MMMMU_OPEN_BASE_ADDR,
	       .end = MMMMU_OPEN_BASE_ADDR + SZ_4K - 1,
		   /* TODO: Size from rdb? */
	       .flags = IORESOURCE_MEM,
	       },
	[1] = {
	       .start = BCM_INT_ID_M4U_OPEN,
	       .end = BCM_INT_ID_M4U_OPEN,
	       .flags = IORESOURCE_IRQ,
	       },
};

static struct platform_device m4u_platform_device = {
	.name = "m4u",
	.id = 0,
	.dev = {
		.platform_data = &m4u_pdata,
	},
	.resource = m4u_resources,
	.num_resources = ARRAY_SIZE(m4u_resources),
};
/* To be moved to board file - End */

struct m4u_device {
#ifdef CONFIG_DEBUG_M4U
	struct miscdevice 		dev;
#endif
	struct mutex 			lock;
	void __iomem			*ioreg_base;
	struct page				*garbage_page;
	dma_addr_t 				pt_handle;
	u32						*pt_base;
	dma_addr_t 				xfifo_handle;
	u32						*xfifo_base;
	u32						xfifo_widx;
	struct m4u_platform_data pdata;
	struct list_head		map_list;
	int						page_count;
	u32						tlb_lock_count;
	struct dentry 			*debug_root;
	struct dentry 			*debug_reg_dir;
	/* Registers */
	struct dentry 			*debug_cr_fs;
	struct dentry 			*debug_isr_fs;
	struct dentry 			*debug_imr_fs;
	struct dentry 			*debug_tbr_fs;
	struct dentry 			*debug_lr_fs;
	struct dentry 			*debug_ldr_fs;
	struct dentry 			*debug_efl_fs;
	struct dentry 			*debug_elock_fs;
	struct dentry 			*debug_eunlock_fs;
	struct dentry 			*debug_xfifo_fs;
	struct dentry 			*debug_pccr_fs;
	struct dentry 			*debug_pcr1_fs;
	struct dentry 			*debug_pcr2_fs;
};

/* Global device used by kernel drivers to invoke M4U APIs */
struct m4u_device *g_mdev = NULL;

/* Macros for debugfs creation */
#define _M4U_GET_REG(REG_NAME, REG_KEY)		\
static int get_ ## REG_NAME ##  _val(void *data, u64 *val) \
{ \
	*val = m4u_read_reg(data, MMMMU_OPEN_ ## REG_KEY ## _OFFSET); \
	return 0; \
}

#define _M4U_SET_REG(REG_NAME, REG_KEY)		\
static int set_ ## REG_NAME ##  _val(void *data, u64 val) \
{ \
	m4u_write_reg(data, MMMMU_OPEN_ ## REG_KEY ## _OFFSET, val); \
	return 0; \
}

#define M4U_GET_REG(REG_NAME, REG_KEY)	\
	_M4U_GET_REG(REG_NAME, REG_KEY) 		\
	DEFINE_SIMPLE_ATTRIBUTE(m4u_debug_reg_ ## REG_NAME ## _fops, \
		get_ ## REG_NAME ## _val, NULL, "0x%08llx\n");

#define M4U_SET_REG(REG_NAME, REG_KEY)	\
	_M4U_SET_REG(REG_NAME, REG_KEY) 		\
	DEFINE_SIMPLE_ATTRIBUTE(m4u_debug_reg_ ## REG_NAME ## _fops, \
		NULL, set_ ## REG_NAME ## _val, "0x%08llx\n");

#define M4U_GET_SET_REG(REG_NAME, REG_KEY)	\
	_M4U_GET_REG(REG_NAME, REG_KEY) 		\
	_M4U_SET_REG(REG_NAME, REG_KEY) 		\
	DEFINE_SIMPLE_ATTRIBUTE(m4u_debug_reg_ ## REG_NAME ## _fops, \
		get_ ## REG_NAME ## _val, set_ ## REG_NAME ## _val, "0x%08llx\n");

#define M4U_DEBUGFS_CREATE_REG_FILE(REG_NAME)	\
		mdev->debug_ ## REG_NAME ## _fs = debugfs_create_file( 		\
				#REG_NAME, (S_IRUGO|S_IWUSR), mdev->debug_reg_dir, 	\
				mdev, &m4u_debug_reg_ ## REG_NAME ## _fops);

static inline void m4u_write_reg(struct m4u_device *mdev, u32 offset, u32 value)
{
	iowrite32(value, (mdev->ioreg_base + offset));
}

static inline u32 m4u_read_reg(struct m4u_device *mdev, u32 offset)
{
	return ioread32(mdev->ioreg_base + offset);
}

static void m4u_print_platform_data(struct m4u_platform_data *pdata)
{
	struct m4u_region *r;
	int i;

	pr_info("MMA area (0x%08x - 0x%08x) \n",
			pdata->mma_begin, (pdata->mma_begin + pdata->mma_size));
	pr_info("(%d) Fixed mapping MMA regions: \n", pdata->nr);
	for(i=0; i<pdata->nr; i++) {
		r = &(pdata->regions[i]);
		pr_info("\t(%d) - mma(0x%08x) pa(0x%08x) size(0x%08x) page_size(0x%08x) \n",
				i, r->mma, r->pa, r->size, r->page_size);
	}
	pr_info("xfifo size(0x%08x) \n", pdata->xfifo_size);
}

static inline u32 m4u_pte(u32 pa, u32 order, u32 valid)
{
	u32 page_order = order & 0xf;
	u32 page_size = (SZ_4K << page_order);
	u32 pte;

	if (pa & (page_size-1))
		pr_err("Invalid pa(0x%x) for page size(0x%x) \n",
				pa, page_size);
	pte = (pa & ~(page_size-1)) | (page_order << 8) | (valid & 1);
	return pte;
}

#if 0
static inline void m4u_tlb_lock(struct m4u_device *mdev, u32 mma)
{
	u32 mma_aligned;

	/* TODO: Threshold for locked TLB check */
	mma_aligned = mma & MMMMU_OPEN_ELOCK_ADDRESS_MASK;
	m4u_write_reg(mdev, MMMMU_OPEN_ELOCK_OFFSET, mma_aligned);
	mdev->tlb_lock_count++;
}

static inline void m4u_tlb_unlock(struct m4u_device *mdev, u32 mma)
{
	u32 mma_aligned;

	mma_aligned = mma & MMMMU_OPEN_EUNLOCK_ADDRESS_MASK;
	m4u_write_reg(mdev, MMMMU_OPEN_EUNLOCK_OFFSET, mma_aligned);
	mdev->tlb_lock_count--;
}
#endif

static inline void m4u_tlb_invalidate(struct m4u_device *mdev, u32 mma, int page_order, int n)
{
	u32 mma_aligned;
	u32 flush_order;

	/* TODO: Invalidation on non-aligned boundaries */
	mma_aligned = mma & MMMMU_OPEN_EFL_ADDRESS_MASK;
	flush_order = (page_order - M4U_PAGE_SHIFT) + 
		(n - M4U_TLB_LINE_SHIFT);
	m4u_write_reg(mdev, MMMMU_OPEN_EFL_OFFSET, (mma_aligned | flush_order));
}

static int m4u_reg_init(struct m4u_device *mdev)
{
	struct m4u_platform_data *pdata = &mdev->pdata;
	int pt_offset;
	u32 tbr, lr, ldr;

	pt_offset = (pdata->mma_begin >> M4U_PAGE_SHIFT);
	tbr = mdev->pt_handle - (pt_offset<<2);
	lr = (pdata->mma_begin + pdata->mma_size) & ~0xFFF;
	ldr = m4u_pte(page_to_phys(mdev->garbage_page),0,1);
	m4u_write_reg(mdev, MMMMU_OPEN_CR_OFFSET, 0);
	m4u_write_reg(mdev, MMMMU_OPEN_IMR_OFFSET,
			(MMMMU_OPEN_IMR_PERFCOUNT1_OVERFLOW_MASK |
			 MMMMU_OPEN_IMR_PERFCOUNT2_OVERFLOW_MASK));
	m4u_write_reg(mdev, MMMMU_OPEN_TBR_OFFSET, tbr);
	m4u_write_reg(mdev, MMMMU_OPEN_LR_OFFSET, lr);
	m4u_write_reg(mdev, MMMMU_OPEN_LDR_OFFSET, ldr);
	m4u_write_reg(mdev, MMMMU_OPEN_EFL_OFFSET, 0x11);

	return 0;
}

/* Invalidation of page will have to be done external */
static void m4u_map_single_page(struct m4u_device *mdev, u32 mma, u32 pa, int page_order, u32 valid)
{
	struct m4u_platform_data *pdata = &mdev->pdata;
	int page_order_base4k;
	u32 pte, num_pte;
	u32 *pt;
	u32 mma_aligned;
	int i;

	page_order_base4k = page_order - M4U_PAGE_SHIFT;
	num_pte = (1 << page_order_base4k);
	mma_aligned = (mma >> (page_order+M4U_TLB_LINE_SHIFT)) << (page_order+M4U_TLB_LINE_SHIFT);
	pt = mdev->pt_base + ((mma_aligned - pdata->mma_begin) >> M4U_PAGE_SHIFT);
	pt += (mma >> page_order) & (M4U_TLB_LINE_SIZE-1);
	pte = m4u_pte(pa, page_order_base4k, valid);
	for (i=0; i<num_pte; i++) {
		*pt = pte;
		pt += M4U_TLB_LINE_SIZE;
	}
}

static int m4u_map_pages(struct m4u_device *mdev, u32 mma, u32 pa, int page_order, int n, u32 valid)
{
	struct m4u_platform_data *pdata = &mdev->pdata;
	u32 *pt;
	int i;

	pr_debug("mma(0x%x) pa(0x%08x) page_order(%d) n(%d) \n",
			mma, pa, page_order, n);
	if (page_order == M4U_PAGE_SHIFT) {
		pt = mdev->pt_base + ((mma - pdata->mma_begin) >> M4U_PAGE_SHIFT);
		for (i=0; i<n; i++) {
			*pt++ = m4u_pte(pa, 0, valid);
			pa += valid << M4U_PAGE_SHIFT;
		}
	} else {
		for (i=0; i<n; i++) {
			m4u_map_single_page(mdev, mma, pa, page_order, valid);
			mma += 1 << page_order;
			pa += valid << page_order;
		}
	}
	if (valid)
		mdev->page_count += n;
	else
		mdev->page_count -= n;
	return 0;
}

/**
 * Create a new mapping
 */
static int m4u_mapping_create(struct m4u_device *mdev, struct m4u_region *region, struct sg_table *sgt)
{
	struct m4u_mapping *mapping;
	int page_order, n;
	int ret = 0;

	if (!region) {
		pr_err("Failed adding invalid region \n");
		return -EINVAL;
	}
	/* Allocate a new region node */
	mapping = kzalloc(sizeof(struct m4u_mapping), GFP_KERNEL);
	if (!mapping) {
		pr_err("Failed allocating region node \n");
		return -ENOMEM;
	}
	mapping->mdev = mdev;
	kref_init(&mapping->ref);
	mapping->region = *region;
	if(sgt)
		mapping->sgt = *sgt;
	else
		mapping->contig_flag = 1;
	pr_info("Add region - mma(0x%08x) pa(0x%08x) size(0x%08x) page_size(0x%08x) \n",
			region->mma, region->pa, region->size, region->page_size);

	mutex_lock(&mdev->lock);
	/* Add it to static list */
	list_add_tail(&mapping->list, &mdev->map_list);

	/* Validate the page size is multiple of 4K and number of pages is multiple of 8 */
	if (sgt) {
		pr_info("SG table map \n");
		/* TODO: If not contiguous, map pages in sgt to page table */
	} else {
		page_order = ilog2(region->page_size);
		n = region->size >> page_order;
		if ((page_order < M4U_PAGE_SHIFT) || (!n)) {
			pr_err("Map Invalid page configuration. page_order(%d), n(%d) \n",
					page_order, n);
			ret = -EINVAL;
			goto err;
		}
		/* Create page table entries in the page table */
		m4u_map_pages(mdev, region->mma, region->pa, page_order, n, 1);
		m4u_tlb_invalidate(mdev, region->mma, page_order, n);
		/* TODO: TLB locking based on page_size and threshold */
	}

	mutex_unlock(&mdev->lock);
	return 0;

err:
	mutex_unlock(&mdev->lock);
	kfree(mapping);
	return ret;
}

static void _m4u_mapping_destroy(struct kref *kref)
{
	struct m4u_mapping *mapping = container_of(kref, struct m4u_mapping, ref);
	struct m4u_device *mdev = mapping->mdev;
	struct m4u_region *r = &mapping->region;
	int page_order, n;

	/* Unmap the region in page table, unlock and flush the TLB */
	page_order = ilog2(r->page_size);
	n = r->size >> page_order;
	if ((page_order < M4U_PAGE_SHIFT) || (!n)) {
		pr_err("Unmap Invalid page configuration. page_order(%d), n(%d) \n",
				page_order, n);
	}
	m4u_map_pages(mdev, r->mma, r->pa, page_order, n, 0);
	/* TODO: Unlock TLB if region was TLB locked */
	m4u_tlb_invalidate(mdev, r->mma, page_order, n);

	/* Remove the node from list and free the node */
	list_del(&mapping->list);
	kfree(mapping);
}

static void m4u_mapping_get(struct m4u_mapping *mapping)
{
	kref_get(&mapping->ref);
}

static int m4u_mapping_put(struct m4u_mapping *mapping)
{
	return kref_put(&mapping->ref, _m4u_mapping_destroy);
}

static u32 m4u_mapping_lookup(struct m4u_device *mdev, u32 pa, u32 size, u32 align)
{
	struct list_head *elt;
	struct m4u_mapping *mapping = NULL;
	struct m4u_region *r;
	u32 mma = INVALID_MMA;
	u32 pa_end = pa + size;

	mutex_lock(&mdev->lock);
	list_for_each(elt, &mdev->map_list) {
		mapping = list_entry(elt, struct m4u_mapping, list);
		r = &mapping->region;
		pr_debug("Search region mma(0x%08x) pa(0x%08x) size(0x%08x) \n",
				r->mma, r->pa, r->size);
		if ((pa >= r->pa) && (pa_end <= r->pa + r->size)) {
			/* TODO: Alignment check */
			mma = r->mma + (pa - r->pa);
			m4u_mapping_get(mapping);
			break;
		}
	}
	mutex_unlock(&mdev->lock);
	return mma;
}

u32 m4u_map_contiguous(struct m4u_device *mdev, u32 pa, u32 size, u32 align)
{
	struct m4u_region region;
	u32 mma = INVALID_MMA;
	int ret;

	/* Search for existing mapping matching pa, size, align */
	mma = m4u_mapping_lookup(mdev, pa, size, align);
	if (mma == INVALID_MMA) {
		/* TODO: Use page_size = size for expected sizes and keep track of
		 * free areas in a separate list */
		size = roundup_pow_of_two(size);
		if (size < (1 << (M4U_PAGE_SHIFT + M4U_TLB_LINE_SHIFT))) {
			pr_debug("Invalid size (%d) for creating mapping \n", size);
			size = 1 << (M4U_PAGE_SHIFT + M4U_TLB_LINE_SHIFT);
		}
		/* TODO: Get mma from pool satisfying the alignment request */
		if (mma != INVALID_MMA) {
			region.mma = mma;
			region.pa = pa;
			region.size = size;
			region.page_size = size >> M4U_TLB_LINE_SHIFT;
			ret = m4u_mapping_create(mdev, &region, NULL);
			if (ret)
				mma = INVALID_MMA;
		}
	}
	pr_debug("map_contiguous mma(0x%08x) pa(0x%08x) size(0x%08x) align(%d) \n",
			mma, pa, size, align);
	return mma;
}
EXPORT_SYMBOL(m4u_map_contiguous);

u32 m4u_map(struct m4u_device *mdev, struct sg_table *sgt, u32 size, u32 align)
{
	struct m4u_region region;
	u32 pa, mma = INVALID_MMA;
	int ret;

	if (!IS_ERR_OR_NULL(sgt) && (sgt->nents == 1))
	{
		pa = sg_phys(sgt->sgl);
		mma = m4u_map_contiguous(mdev, pa, size, align);
	} else {
		size = roundup_pow_of_two(size);
		if (size < (1 << (M4U_PAGE_SHIFT + M4U_TLB_LINE_SHIFT))) {
			pr_debug("Invalid size (%d) for creating mapping \n", size);
			size = 1 << (M4U_PAGE_SHIFT + M4U_TLB_LINE_SHIFT);
		}
		/* TODO: Get mma from pool satisfying the alignment request */
		if (mma != INVALID_MMA) {
			region.mma = mma;
			region.pa = 0;
			region.size = size;
			/* TODO: Utilize bigger pagesizes */
			region.page_size = (1 << M4U_PAGE_SHIFT);
			ret = m4u_mapping_create(mdev, &region, sgt);
			if (ret)
				mma = INVALID_MMA;
		}
		pr_debug("map_sgt mma(0x%08x) sgt(%p) size(0x%08x) align(%d) \n",
				mma, sgt, size, align);
	}
	return mma;
}
EXPORT_SYMBOL(m4u_map);

void m4u_unmap(struct m4u_device *mdev, u32 mma)
{
	struct list_head *elt;
	struct m4u_mapping *mapping = NULL;
	struct m4u_region *r;

	mutex_lock(&mdev->lock);
	list_for_each(elt, &mdev->map_list) {
		mapping = list_entry(elt, struct m4u_mapping, list);
		r = &mapping->region;
		pr_debug("Search region mma(0x%08x) pa(0x%08x) size(0x%08x) \n",
				r->mma, r->pa, r->size);
		if ((mma >= r->mma) && (mma < r->mma + r->size)) {
			pr_debug("Unmap mma(0x%08x) mapping - mma(0x%08x) pa(0x%08x) size(0x%08x) \n",
					mma, r->mma, r->pa, r->size);
			m4u_mapping_put(mapping);
			break;
		}
	}
	mutex_unlock(&mdev->lock);
}
EXPORT_SYMBOL(m4u_unmap);

static void m4u_exit(struct m4u_device *mdev)
{
	struct m4u_platform_data *pdata = &mdev->pdata;
	int pt_size;

	pt_size = (pdata->mma_size >> (M4U_PAGE_SHIFT-2));

	if (mdev->xfifo_base)
		dma_free_coherent(NULL, pdata->xfifo_size, mdev->xfifo_base, mdev->xfifo_handle);
	mdev->xfifo_base = NULL;
	if (mdev->pt_base)
		dma_free_coherent(NULL, pt_size, mdev->pt_base, mdev->pt_handle);
	mdev->pt_base = NULL;
	if(mdev->garbage_page)
		__free_page(mdev->garbage_page);
	mdev->garbage_page = NULL;
}

static int m4u_init(struct m4u_device *mdev)
{
	struct m4u_platform_data *pdata = &mdev->pdata;
	int pt_size;
	int ret;
	int i;

	/* Allocate garbage page for limit register */
	mdev->garbage_page = alloc_page(GFP_KERNEL);
	if (mdev->garbage_page == NULL) {
		pr_err("Garbage page allocation failed. \n");
		goto error;
	}
	/* Allocate page table */
	pt_size = (pdata->mma_size >> (M4U_PAGE_SHIFT-2));
	mdev->pt_base = (u32 *)dma_alloc_coherent(NULL, pt_size,
			&mdev->pt_handle, (GFP_KERNEL | ___GFP_ZERO));
	if (mdev->pt_base == NULL) {
		pr_err("Page table allocation (0x%x) failed. \n", pt_size);
		goto error;
	}
	pr_info("pt base(%p:0x%x 0x%08x) garbage_page(0x%08x) \n", mdev->pt_base, 
			mdev->pt_handle, pt_size, page_to_phys(mdev->garbage_page));

	/* Allocate xfifo buffer */
	if (pdata->xfifo_size) {
		mdev->xfifo_base = (u32 *)dma_alloc_coherent(NULL, pdata->xfifo_size,
				&mdev->xfifo_handle, (GFP_KERNEL | ___GFP_ZERO));
		if (mdev->xfifo_base == NULL) {
			pr_err("xfifo buffer allocation (0x%x) failed. \n", pdata->xfifo_size);
			goto error;
		}
	}
	pr_info("xfifo base(%p:0x%x 0x%08x) \n",
			mdev->xfifo_base, mdev->xfifo_handle, pdata->xfifo_size);

	/* TODO: Allocate bitmap for mma pool */

	/* Initialize m4u registers */
	m4u_reg_init(mdev);

	/* Initialize m4u page table */
	for (i=0; i<pdata->nr; i++) {
		/* TODO: Validate mma space is a valid area in pool */
		ret = m4u_mapping_create(mdev, &pdata->regions[i], NULL);
		if (ret) {
			goto error;
		}
	}

	return 0;

error:
	m4u_exit(mdev);
	return -ENOMEM;
}

static irqreturn_t m4u_isr(int irq, void *data)
{
	struct m4u_device *mdev = (struct m4u_device *)data;
	struct m4u_platform_data *pdata = &mdev->pdata;
	u32 status = 0;
	u32 val;

	/* Read the status bits */
	status = m4u_read_reg(mdev, MMMMU_OPEN_ISR_OFFSET);
	pr_debug("Interrupt status [0x%08x] \n", status);
	if (status & MMMMU_OPEN_IMR_EXFIFO_NOT_EMPTY_MASK) {
		while(1) {
			val = m4u_read_reg(mdev, MMMMU_OPEN_XFIFO_OFFSET);
			if (val & MMMMU_OPEN_XFIFO_VALID_MASK) {
				pr_debug("xfifo(0x%x) \n", val);
				mdev->xfifo_base[mdev->xfifo_widx++] = val;
				if (mdev->xfifo_widx >= (pdata->xfifo_size>>2))
					mdev->xfifo_widx = 0;
			} else {
				break;
			}
		}
		return IRQ_HANDLED;
	}
	if (status & MMMMU_OPEN_IMR_PERFCOUNT1_OVERFLOW_MASK) {
		val = m4u_read_reg(mdev, MMMMU_OPEN_PCR1_OFFSET);
		pr_debug("pcr1(0x%x) \n", val);
		/* Mask the iterrupt */
		m4u_write_reg(mdev, MMMMU_OPEN_IMR_OFFSET, MMMMU_OPEN_IMR_PERFCOUNT1_OVERFLOW_MASK);
		return IRQ_HANDLED;
	}
	if (status & MMMMU_OPEN_IMR_PERFCOUNT2_OVERFLOW_MASK) {
		val = m4u_read_reg(mdev, MMMMU_OPEN_PCR2_OFFSET);
		pr_debug("pcr2(0x%x) \n", val);
		/* Mask the iterrupt */
		m4u_write_reg(mdev, MMMMU_OPEN_IMR_OFFSET, MMMMU_OPEN_IMR_PERFCOUNT2_OVERFLOW_MASK);
		return IRQ_HANDLED;
	}

	return IRQ_NONE;
}

M4U_GET_REG(cr, CR);
M4U_GET_REG(isr, ISR);
M4U_GET_SET_REG(imr, IMR);
M4U_GET_SET_REG(tbr, TBR);	/* Convert to only get after bringup */
M4U_GET_SET_REG(lr, LR);	/* Convert to only get after bringup */
M4U_GET_REG(ldr, LDR);
M4U_SET_REG(efl, EFL);
M4U_SET_REG(elock, ELOCK);
M4U_SET_REG(eunlock, EUNLOCK);
M4U_GET_REG(xfifo, XFIFO);
M4U_GET_SET_REG(pccr, PCCR);
M4U_GET_SET_REG(pcr1, PCR1);
M4U_GET_SET_REG(pcr2, PCR2);

void m4u_debugfs_init(struct m4u_device *mdev, struct platform_device *pdev)
{
	char debug_name[64];

	/* Create root directory */
	mdev->debug_root = debugfs_create_dir(pdev->name, NULL);
	if (IS_ERR_OR_NULL(mdev->debug_root))
		pr_err("Failed to create debug root dir.\n");

	/* Create register files */
	snprintf(debug_name, 64, "registers");
	mdev->debug_reg_dir = debugfs_create_dir(debug_name, mdev->debug_root);
	if (IS_ERR_OR_NULL(mdev->debug_reg_dir)) {
		pr_err("Failed to create dir(%s).\n", debug_name);
	} else {
		M4U_DEBUGFS_CREATE_REG_FILE(cr);
		M4U_DEBUGFS_CREATE_REG_FILE(isr);
		M4U_DEBUGFS_CREATE_REG_FILE(imr);
		M4U_DEBUGFS_CREATE_REG_FILE(tbr);
		M4U_DEBUGFS_CREATE_REG_FILE(lr);
		M4U_DEBUGFS_CREATE_REG_FILE(ldr);
		M4U_DEBUGFS_CREATE_REG_FILE(efl);
		M4U_DEBUGFS_CREATE_REG_FILE(elock);
		M4U_DEBUGFS_CREATE_REG_FILE(eunlock);
		M4U_DEBUGFS_CREATE_REG_FILE(xfifo);
		M4U_DEBUGFS_CREATE_REG_FILE(pccr);
		M4U_DEBUGFS_CREATE_REG_FILE(pcr1);
		M4U_DEBUGFS_CREATE_REG_FILE(pcr2);
	}
}

#ifdef CONFIG_DEBUG_M4U
enum m4u_ioctls {
	M4U_MAPPING_ADD = 0,
	M4U_MAPPING_REMOVE,
	M4U_MAPPING_UPDATE,
	M4U_REG_GET,
	M4U_REG_SET,
	M4U_REG_SET_GET,
};

struct m4u_reg_data {
	u32 offset;
	u32 val;
};

static int m4u_mapping_update(struct m4u_device *mdev, struct m4u_region *region)
{
	struct list_head *elt;
	struct m4u_mapping *mapping = NULL;
	struct m4u_region *r;
	int page_order, n;
	u32 mma, mma_end;

	if (!region) {
		pr_err("Failed updating invalid region \n");
		return -EINVAL;
	}
	mma = region->mma;
	mma_end = mma + region->size;
	mutex_lock(&mdev->lock);
	list_for_each(elt, &mdev->map_list) {
		mapping = list_entry(elt, struct m4u_mapping, list);
		r = &mapping->region;
		pr_debug("Search region mma(0x%08x) pa(0x%08x) size(0x%08x) \n",
				r->mma, r->pa, r->size);
		if ((mapping->contig_flag) && (mma == r->mma) && (mma < r->mma + r->size)) {
			*r = *region;
			if (region->page_size) {
				page_order = ilog2(region->page_size);
				n = region->size >> page_order;
				if (!((page_order < M4U_PAGE_SHIFT) || (n & 0x7))) {
					pr_debug("Update mapping - mma(0x%08x) pa(0x%08x) size(0x%08x) \n",
							r->mma, r->pa, r->size);
					/* TODO: Unlock TLB if region was TLB locked */
					m4u_tlb_invalidate(mdev, region->mma, page_order, n);
					m4u_map_pages(mdev, region->mma, region->pa, page_order, n, 1);
					break;
				}
			}
		}
	}
	mutex_unlock(&mdev->lock);
	return 0;
}

static long m4u_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct m4u_device *mdev = filp->private_data;

	pr_debug("ioctl mdev(%p) cmd(%d) \n", mdev, cmd);

	switch (cmd) {
	case M4U_MAPPING_ADD:
	{
		struct m4u_region data;
		if (copy_from_user(&data, (void __user *)arg, sizeof(data)))
			return -EFAULT;
		pr_debug("M4U_MAPPING_ADD - mma(0x%08x) pa(0x%08x) size(0x%08x) page_size(0x%08x) \n",
				data.mma, data.pa, data.size, data.page_size);
		m4u_mapping_create(mdev, &data, NULL);
		break;
	}
	case M4U_MAPPING_REMOVE:
	{
		pr_debug("M4U_MAPPING_REMOVE - mma(0x%08x) \n", (u32)arg);
		m4u_unmap(mdev, (u32)arg);
		break;
	}
	case M4U_MAPPING_UPDATE:
	{
		struct m4u_region data;
		if (copy_from_user(&data, (void __user *)arg, sizeof(data)))
			return -EFAULT;
		pr_debug("M4U_MAPPING_UPDATE - mma(0x%08x) pa(0x%08x) size(0x%08x) page_size(0x%08x) \n",
				data.mma, data.pa, data.size, data.page_size);
		m4u_mapping_update(mdev, &data);
		break;
	}
	case M4U_REG_GET:
	{
		struct m4u_reg_data data;
		if (copy_from_user(&data, (void __user *)arg, sizeof(data)))
			return -EFAULT;
		data.val = m4u_read_reg(mdev, data.offset);
		pr_debug("M4U_REG_GET - offset(0x%08x) data(0x%08x) \n",
				data.offset, data.val);
		if (copy_to_user((void __user *)arg, &data, sizeof(data)))
			return -EFAULT;
		break;
	}
	case M4U_REG_SET:
	{
		struct m4u_reg_data data;
		if (copy_from_user(&data, (void __user *)arg, sizeof(data)))
			return -EFAULT;
		pr_debug("M4U_REG_SET - offset(0x%08x) data(0x%08x) \n",
				data.offset, data.val);
		m4u_write_reg(mdev, data.offset, data.val);
		break;
	}
	case M4U_REG_SET_GET:
	{
		struct m4u_reg_data data;
		if (copy_from_user(&data, (void __user *)arg, sizeof(data)))
			return -EFAULT;
		pr_debug("M4U_REG_SET_GET - offset(0x%08x) data(0x%08x) \n",
				data.offset, data.val);
		m4u_write_reg(mdev, data.offset, data.val);
		data.val = m4u_read_reg(mdev, data.offset);
		pr_debug("M4U_REG_SET_GET - offset(0x%08x) data(0x%08x) \n",
				data.offset, data.val);
		if (copy_to_user((void __user *)arg, &data, sizeof(data)))
			return -EFAULT;
		break;
	}
	default:
		return -ENOTTY;
	}
	return 0;
}

static int m4u_open(struct inode *inode, struct file *filp)
{
	struct miscdevice *miscdev = filp->private_data;
	struct m4u_device *mdev = container_of(miscdev, struct m4u_device, dev);

	pr_debug("open mdev(%p) \n", mdev);
	filp->private_data = mdev;

	return 0;
}

static int m4u_release(struct inode *inode, struct file *filp)
{
	struct m4u_device *mdev = filp->private_data;

	pr_debug("close mdev(%p) \n", mdev);

	return 0;
}

static const struct file_operations m4u_fops = {
	.owner          = THIS_MODULE,
	.open           = m4u_open,
	.release        = m4u_release,
	.unlocked_ioctl = m4u_ioctl,
};
#endif

static int m4u_probe(struct platform_device *pdev)
{
	struct m4u_platform_data *pdata;
	struct m4u_device *mdev;
	struct resource *mem_res = NULL;
	unsigned int irq = 0;
	int ret;

	pr_debug("M4U device probe \n");
	if (!pdev || !pdev->dev.platform_data) {
		pr_err("Not able to probe \n");
		return -ENODEV;
	}
	pdata = pdev->dev.platform_data;
	m4u_print_platform_data(pdata);

	/* Alloc device specific context */
	mdev = kzalloc(sizeof(struct m4u_device), GFP_KERNEL);
	if (!mdev) {
		pr_err("Allocation of context failed. \n");
		return -ENOMEM;
	}
	g_mdev = mdev;
	mutex_init(&mdev->lock);
	memcpy(&mdev->pdata, pdata, sizeof(*pdata));
	INIT_LIST_HEAD(&mdev->map_list);

	/* Map the register space */
	mem_res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!mem_res) {
		pr_err("Register base not specified. \n");
		ret = -ENODEV;
		goto err_freemem;
	}
	mem_res = request_mem_region(mem_res->start, resource_size(mem_res),
					dev_name(&pdev->dev));
	if (!mem_res) {
		pr_err("Register space already reserved. \n");
		ret = -EBUSY;
		goto err_freemem;
	}
	mdev->ioreg_base = ioremap_nocache(mem_res->start, resource_size(mem_res));
	if (!mdev->ioreg_base) {
		pr_err("Failed to map the registers. \n");
		ret = -ENOMEM;
		goto err_freereg;
	}

#ifdef CONFIG_DEBUG_M4U
	/* Register misc device */
#if defined (CONFIG_MACH_HAWAII_FPGA_E) || defined (CONFIG_MACH_HAWAII_FPGA)
	mdev->dev.minor = 5;
#else
	mdev->dev.minor = MISC_DYNAMIC_MINOR;
#endif
	mdev->dev.name = pdev->name;
	mdev->dev.fops = &m4u_fops;
	mdev->dev.parent = NULL;
	ret = misc_register(&mdev->dev);
	if (ret) {
		pr_err("Failed to register misc device minor(%d).\n",
				mdev->dev.minor);
		ret = -ENODEV;
		goto err_unmap;
	}
#endif

	/* Register IRQ */
	irq = platform_get_irq(pdev, 0);
	ret = request_irq(irq, m4u_isr,	0, pdev->name, (void *)mdev);
	if (ret) {
		pr_err("Failed to register IRQ (%d). \n", irq);
		ret = -ENODEV;
		goto err_deregister;
	}

	/* Initialize m4u page table and registers as per platform data */
	if (m4u_init(mdev)) {
		pr_err("Failed to initialize m4u. \n");
		ret = -ENOMEM;
		goto err_freeirq;
	}

	/* Create Debug files */
	m4u_debugfs_init(mdev, pdev);

	/* Set the mdev and platform device private data */
	platform_set_drvdata(pdev, mdev);
	pr_info("probe successfull - mdev(%p) irq(%d) reg-base(%p). \n",
			mdev, irq, mdev->ioreg_base);

	return 0;

err_freeirq:
	free_irq(irq, mdev);

err_deregister:
#ifdef CONFIG_DEBUG_M4U
	misc_deregister(&mdev->dev);

err_unmap:
#endif
	iounmap(mdev->ioreg_base);

err_freereg:
	if (mem_res)
		release_mem_region(mem_res->start, resource_size(mem_res));

err_freemem:
	kfree(mdev);

	return ret;
}

static int m4u_remove(struct platform_device *pdev)
{
	struct m4u_device *mdev = platform_get_drvdata(pdev);
	struct resource *mem_res;
	unsigned int irq;

	pr_info("M4U device remove \n");
	if (mdev) {
		debugfs_remove_recursive(mdev->debug_root);
		m4u_exit(mdev);
		irq = platform_get_irq(pdev, 0);
		free_irq(irq, mdev);
#ifdef CONFIG_DEBUG_M4U
		misc_deregister(&mdev->dev);
#endif
		iounmap(mdev->ioreg_base);
		mem_res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
		if (mem_res)
			release_mem_region(mem_res->start, resource_size(mem_res));
		kfree(mdev);
	}
	return 0;
}

#ifdef CONFIG_PM
static int m4u_suspend(struct platform_device *pdev, pm_message_t message)
{
	return 0;
}

static int m4u_resume(struct platform_device *pdev)
{
	return 0;
}

#else

#define	m4u_suspend	NULL
#define	m4u_resume	NULL

#endif

static struct platform_driver m4u_platform_driver = {
	.probe = m4u_probe,
	.remove = m4u_remove,
#ifdef CONFIG_PM
	.suspend	= m4u_suspend,
	.resume		= m4u_resume,
#endif
	.driver = { .name = "m4u" }
};

static int __init m4u_driver_init(void)
{
	pr_debug("M4U driver init \n");
	platform_device_register(&m4u_platform_device);
	return platform_driver_register(&m4u_platform_driver);
}

static void __exit m4u_driver_exit(void)
{
	pr_info("M4U driver exit \n");
	platform_driver_unregister(&m4u_platform_driver);
}

subsys_initcall(m4u_driver_init);
module_exit(m4u_driver_exit);

MODULE_AUTHOR("Broadcom Corporation");
MODULE_DESCRIPTION("M4U device driver");
MODULE_LICENSE("GPL");

