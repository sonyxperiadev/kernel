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
#include <linux/miscdevice.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/dma-mapping.h>
#include <linux/log2.h>
#include <linux/debugfs.h>
#include <linux/broadcom/m4u.h>

#include <mach/irqs.h>
#include <mach/rdb/brcm_rdb_sysmap.h>
#include <mach/rdb/brcm_rdb_mmmmu_open.h>

#define DEBUG_M4U

#define BCM_INT_ID_M4U_OPEN		(BCM_INT_ID_RESERVED181)
#define	M4U_PAGE_SHIFT			(12)
#define	M4U_TLB_LINE_SHIFT		(3)
#define	M4U_TLB_LINE_SIZE		(1<<M4U_TLB_LINE_SHIFT)

struct m4u_region_node {
	struct m4u_region region;
	struct list_head list;
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

static struct m4u_platform_data m4u_data = {
	.mma_start		= 0x80000000,
	.mma_end		= 0xF0000000,
	.mma_sg_start	= 0xA0000000,
	.xfifo_size		= SZ_16K,
	.nr				= ARRAY_SIZE(m4u_regions),
	.regions		= m4u_regions,
};

static struct resource m4u_resources[] = {
	[0] = {
	       .start = MMMMU_OPEN_BASE_ADDR,
	       .end = MMMMU_OPEN_BASE_ADDR + SZ_4K - 1,
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
		.platform_data = &m4u_data,
	},
	.resource = m4u_resources,
	.num_resources = ARRAY_SIZE(m4u_resources),
};
/* To be moved to board file - End */

struct m4u_device {
	struct miscdevice 		dev;
	struct mutex 			lock;
	void __iomem			*ioreg_base;
	struct page				*garbage_page;
	dma_addr_t 				pt_handle;
	u32						*pt_base;
	dma_addr_t 				xfifo_handle;
	u32						*xfifo_base;
	u32						xfifo_widx;
	struct m4u_platform_data pdata;
	struct list_head		static_region_list;
	struct dentry 			*debug_root;
};

struct m4u_device *g_mdev;
#ifdef DEBUG_M4U
#endif

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
	int i;
	struct m4u_region *r;

	pr_info("MMA area (0x%08x - 0x%08x) Dynamic Map start (0x%08x) \n",
			pdata->mma_start, pdata->mma_end, pdata->mma_sg_start);
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

static void m4u_map_big_page(struct m4u_device *mdev, u32 mma, u32 pa, int page_order)
{
	int i;
	int page_order_base4k;
	u32 pte, num_pte;
	u32 *pt;
	u32 mma_aligned;

	page_order_base4k = page_order - M4U_PAGE_SHIFT;
	num_pte = (1 << page_order_base4k);
	mma_aligned = (mma >> (page_order+M4U_TLB_LINE_SHIFT)) << (page_order+M4U_TLB_LINE_SHIFT);
	pt = mdev->pt_base + ((mma_aligned - mdev->pdata.mma_start) >> M4U_PAGE_SHIFT);
	pt += (mma >> page_order) & (M4U_TLB_LINE_SIZE-1);
	pte = m4u_pte(pa, page_order_base4k, 1);
	for (i=0; i<num_pte; i++) {
		*pt = pte;
		pt += M4U_TLB_LINE_SIZE;
	}
}

static int m4u_map_pages(struct m4u_device *mdev, u32 mma, u32 pa, int page_order, int n)
{
	int i;
	u32 *pt;

#ifdef DEBUG_M4U
	pr_info("(Debug) mma(0x%x) pa(0x%08x) page_order(%d) n(%d) \n", 
			mma, pa, page_order, n);
#endif
	if (page_order == M4U_PAGE_SHIFT) {
		pt = mdev->pt_base + ((mma - mdev->pdata.mma_start) >> M4U_PAGE_SHIFT);
		for (i=0; i<n; i++) {
			*pt++ = m4u_pte(pa, 0, 1);
			pa += 1 << M4U_PAGE_SHIFT;
		}
	} else {
		for (i=0; i<n; i++) {
			m4u_map_big_page(mdev, mma, pa, page_order);
			mma += 1 << page_order;
			pa += 1 << page_order;
		}
	}
	return 0;
}

static int m4u_pt_init(struct m4u_device *mdev)
{
	int i;
	int page_order, n;

	for (i=0; i<mdev->pdata.nr; i++) {
		page_order = (fls_long(mdev->pdata.regions[i].page_size) - 1);
		n = mdev->pdata.regions[i].size >> page_order;
		if ((page_order < M4U_PAGE_SHIFT) || (n & 0x7)) {
			pr_err("Invalid page configuration. page_order(%d), n(%d) \n",
					page_order, n);
			return -EINVAL;
		}
		m4u_map_pages(mdev, mdev->pdata.regions[i].mma, mdev->pdata.regions[i].pa, 
				page_order, n);
	}
	return 0;
}

static int m4u_reg_init(struct m4u_device *mdev)
{
	int pt_offset;
	u32 tbr, lr, ldr;

	pt_offset = (mdev->pdata.mma_start >> M4U_PAGE_SHIFT);
	tbr = mdev->pt_handle - (pt_offset<<2);
	lr = mdev->pdata.mma_end & ~0xFFF;
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

static void m4u_exit(struct m4u_device *mdev)
{
	int pt_size, pt_offset;

	pt_offset = (mdev->pdata.mma_start >> M4U_PAGE_SHIFT);
	pt_size = (SZ_2G >> (M4U_PAGE_SHIFT-1)) - pt_offset;

	if (mdev->xfifo_base)
		dma_free_coherent(NULL, mdev->pdata.xfifo_size, mdev->xfifo_base, mdev->xfifo_handle);
	mdev->xfifo_base = NULL;
	if (mdev->pt_base)
		dma_free_coherent(NULL, (pt_size<<2), mdev->pt_base, mdev->pt_handle);
	mdev->pt_base = NULL;
	if(mdev->garbage_page)
		__free_page(mdev->garbage_page);
	mdev->garbage_page = NULL;
}

static int m4u_init(struct m4u_device *mdev)
{
	int pt_size, pt_offset;

	/* Allocate garbage page for limit register */
	mdev->garbage_page = alloc_page(GFP_KERNEL);
	if (mdev->garbage_page == NULL) {
		pr_err("Garbage page allocation failed. \n");
		goto error;
	}
	/* Allocate page table */
	pt_offset = (mdev->pdata.mma_start >> M4U_PAGE_SHIFT);
	pt_size = (SZ_2G >> (M4U_PAGE_SHIFT-1)) - pt_offset;
	mdev->pt_base = (u32 *)dma_alloc_coherent(NULL, (pt_size<<2), 
			&mdev->pt_handle, (GFP_KERNEL | ___GFP_ZERO));
	if (mdev->pt_base == NULL) {
		pr_err("Page table allocation (0x%x) failed. \n", (pt_size<<2));
		goto error;
	}
	/* Allocate xfifo buffer */
	if (mdev->pdata.xfifo_size) {
		mdev->xfifo_base = (u32 *)dma_alloc_coherent(NULL, mdev->pdata.xfifo_size, 
				&mdev->xfifo_handle, (GFP_KERNEL | ___GFP_ZERO));
		if (mdev->xfifo_base == NULL) {
			pr_err("xfifo buffer allocation (0x%x) failed. \n", mdev->pdata.xfifo_size);
			goto error;
		}
	}

	pr_info("pt base(%p:0x%x) offset(0x%x) entries(0x%x) \n",
			mdev->pt_base, mdev->pt_handle,	pt_offset, pt_size);
	pr_info("garbage_page(0x%x) xfifo base(%p:0x%x 0x%08x) \n",
			page_to_phys(mdev->garbage_page), mdev->xfifo_base, mdev->xfifo_handle,
			mdev->pdata.xfifo_size);
	/* Initialize m4u page table */
	if (m4u_pt_init(mdev)) {
		goto error;
	}
	/* Initialize m4u registers */
	m4u_reg_init(mdev);

	return 0;

error:
	m4u_exit(mdev);
	return -ENOMEM;
}

int m4u_map(struct m4u_device *mdev, struct sg_table *sgt, u32 size, u32 align)
{
	struct m4u_region_node *region_node;
	struct m4u_region *r;
	struct list_head *reg;
	u32 pa = 0, mma = 0, pa_end;

	/* Only contiguous blocks assumed */
	if (!IS_ERR_OR_NULL(sgt) && (sgt->nents == 1))
		pa = sg_phys(sgt->sgl);
	if (pa) {
		pa_end = pa + size - 1;
		list_for_each(reg, &mdev->static_region_list) {
			region_node = list_entry(reg, struct m4u_region_node, list);
			r = &region_node->region;
			pr_debug("map region mma(0x%08x) pa(0x%08x) size(0x%08x) \n",
				   r->mma, r->pa, r->size);
			if ((pa > r->pa) && (pa_end < r->pa + r->size)) {
				mma = r->mma + (pa - r->pa);
				break;
			}
		}
	}
	pr_debug("map mma(0x%08x) pa(0x%08x) size(0x%08x) align(%d) \n",
		mma, pa, size, align);
	return mma;
}
EXPORT_SYMBOL(m4u_map);

void m4u_unmap(struct m4u_device *mdev, u32 mma, u32 size)
{
	struct m4u_region_node *region_node;
	struct m4u_region *r;
	struct list_head *reg;
	u32 pa = 0, mma_end = 0;

	list_for_each(reg, &mdev->static_region_list) {
		region_node = list_entry(reg, struct m4u_region_node, list);
		r = &region_node->region;
		pr_debug("map region mma(0x%08x) pa(0x%08x) size(0x%08x) \n",
				r->mma, r->pa, r->size);
		if ((mma > r->mma) && (mma_end < r->mma + r->size)) {
			pa = r->pa + (mma - r->mma);
			break;
		}
	}
	pr_debug("unmap mma(0x%08x) pa(0x%08x) size(0x%08x) \n",
		mma, pa, size);
}
EXPORT_SYMBOL(m4u_unmap);

int m4u_add_region(struct m4u_device *mdev, struct m4u_region *region)
{
	struct m4u_region_node *region_node;

	if (!region) {
		pr_err("Failed adding invalid region \n");
		return -EINVAL;
	}
	region_node = kmalloc(sizeof(struct m4u_region_node), GFP_KERNEL);
	if (!region_node) {
		pr_err("Failed allocating metadata for region \n");
		return -ENOMEM;
	}
	region_node->region = *region;
	list_add_tail(&region_node->list, &mdev->static_region_list);
		pr_info("Add region - mma(0x%08x) pa(0x%08x) size(0x%08x) page_size(0x%08x) \n", 
				region->mma, region->pa, region->size, region->page_size);

	return 0;
}
EXPORT_SYMBOL(m4u_add_region);

static irqreturn_t m4u_isr(int irq, void *data)
{
	struct m4u_device *mdev = (struct m4u_device *)data;
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
				if (mdev->xfifo_widx >= (mdev->pdata.xfifo_size>>2))
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

static long m4u_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct m4u_device *mdev = filp->private_data;

	pr_info("ioctl mdev(%p) \n", mdev);

	switch (cmd) {
	default:
		return -ENOTTY;
	}
	return 0;
}

static int m4u_open(struct inode *inode, struct file *filp)
{
	struct miscdevice *miscdev = filp->private_data;
	struct m4u_device *mdev = container_of(miscdev, struct m4u_device, dev);

	pr_info("open mdev(%p) \n", mdev);
#ifdef DEBUG_M4U
	m4u_print_platform_data(&mdev->pdata);
#endif
	m4u_reg_init(mdev);
	filp->private_data = mdev;

	return 0;
}

static int m4u_release(struct inode *inode, struct file *filp)
{
	struct m4u_device *mdev = filp->private_data;

	pr_info("close mdev(%p) \n", mdev);

	return 0;
}

static const struct file_operations m4u_fops = {
	.owner          = THIS_MODULE,
	.open           = m4u_open,
	.release        = m4u_release,
	.unlocked_ioctl = m4u_ioctl,
};

static int m4u_probe(struct platform_device *pdev)
{
	struct m4u_platform_data *pdata;
	struct m4u_device *mdev;
	struct resource *mem_res = NULL;
	unsigned int irq = 0;
	int ret;
	int i;

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
	INIT_LIST_HEAD(&mdev->static_region_list);
	for(i=0; i<pdata->nr; i++) {
		ret = m4u_add_region(mdev, &pdata->regions[i]);
		if (ret) {
			goto err_freemem;
		}
	}
	/* TODO: Free region buffers */
	pr_info("(Debug) mdev(%p) \n", g_mdev);

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

	/* Register IRQ */
	irq = platform_get_irq(pdev, 0);
	ret = request_irq(irq, m4u_isr,	0, pdev->name, (void *)mdev);
	if (ret) {
		pr_err("Failed to register IRQ (%d). \n", irq);
		ret = -ENODEV;
		goto err_deregister;
	}

	if (m4u_init(mdev)) {
		pr_err("Failed to initialize m4u. \n");
		ret = -ENOMEM;
		goto err_freeirq;
	}

	/* Create Debug files */
	mdev->debug_root = debugfs_create_dir(pdev->name, NULL);
	if (IS_ERR_OR_NULL(mdev->debug_root))
		pr_err("Failed to create debug root dir.\n");

	/* Set the mdev and platform device private data */
	platform_set_drvdata(pdev, mdev);
	pr_info("probe successfull - irq(%d) reg-base(%p). \n",
			irq, mdev->ioreg_base);

	return 0;

err_freeirq:
	free_irq(irq, mdev);

err_deregister:
	misc_deregister(&mdev->dev);

err_unmap:
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
		misc_deregister(&mdev->dev);
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

