/*
 * drivers/iommu/bcm_iovmm.c
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

#define pr_fmt(fmt) "iovmm-: " fmt

#include <linux/module.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/scatterlist.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#ifdef CONFIG_OF
#include <linux/of.h>
#endif /* CONFIG_OF */
#include <linux/sched.h>
#include <linux/iommu.h>
#include <plat/bcm_iommu.h>

struct dma_iommu_mapping {
	/* iommu specific data */
	struct iommu_domain	*domain;

	void			*bitmap;
	size_t			bits;
	unsigned int		order;
	dma_addr_t		base;

	spinlock_t		lock;
	struct kref		kref;
};


#ifndef LINUX_3_6_WAY
struct device_map {
	struct dma_iommu_mapping *mapping;
	struct device *dev;
	struct list_head list;
};

static LIST_HEAD(device_node_list);

int __iommu_add_device_node(struct device *dev,
		struct dma_iommu_mapping *mapping)
{
	struct device_map *device_node;

	/* Allocate a new region node */
	device_node = kzalloc(sizeof(struct device_map), GFP_KERNEL);
	if (!device_node) {
		pr_err("Failed allocating device <-> mapping node\n");
		return -ENOMEM;
	}
	device_node->dev = dev;
	device_node->mapping = mapping;
	list_add_tail(&device_node->list, &device_node_list);

	return 0;
}

int __iommu_remove_device_node(struct device *dev)
{
	struct device_map *device_node;
	struct list_head *elt;

	list_for_each(elt, &device_node_list) {
		device_node = list_entry(elt, struct device_map, list);
		if (device_node->dev == dev) {
			list_del(&device_node->list);
			kfree(device_node);
			return 0;
		}
	}
	pr_err("Failed removing device(%p) <-> mapping node\n", dev);
	return -EINVAL;
}

struct dma_iommu_mapping *__iommu_get_mapping(struct device *dev)
{
	struct device_map *device_node;
	struct list_head *elt;

	list_for_each(elt, &device_node_list) {
		device_node = list_entry(elt, struct device_map, list);
		if (device_node->dev == dev)
			return device_node->mapping;
	}
	pr_err("ERROR: Cannot find iommu mapping for dev(%p)\n", dev);
	return NULL;
}

#endif /* LINUX_3_6_WAY */

#ifdef CONFIG_BCM_IOVMM_DEBUG

static void print_bitmap(struct dma_iommu_mapping *mapping)
{
	static char buf[32768];
	ssize_t ret;

	ret = bitmap_scnlistprintf(buf, sizeof(buf), mapping->bitmap,
				mapping->bits);
	buf[ret++] = '\n';
	buf[ret] = '\0';
	pr_info("=== IOMMU MAPPING: 0x%p: bitmap:0x%p bits:%d ===\n",
			mapping, mapping->bitmap, mapping->bits);
	pr_info(" %s\n", buf);
	return;
}
#endif

static inline dma_addr_t __alloc_iova(struct dma_iommu_mapping *mapping,
				      size_t size)
{
	unsigned int order = get_order(size);
	unsigned int align = 0;
	unsigned int count, start;
	unsigned long flags;

	if (order > CONFIG_BCM_IOVMM_ALIGNMENT)
		order = CONFIG_BCM_IOVMM_ALIGNMENT;

	count = ((PAGE_ALIGN(size) >> PAGE_SHIFT) +
		 (1 << mapping->order) - 1) >> mapping->order;

	if (order > mapping->order)
		align = (1 << (order - mapping->order)) - 1;

	spin_lock_irqsave(&mapping->lock, flags);
	start = bitmap_find_next_zero_area(mapping->bitmap, mapping->bits, 0,
					   count, align);
	if (start > mapping->bits) {
		spin_unlock_irqrestore(&mapping->lock, flags);
		pr_err("%s failed: mapping(%p) count(%d) align(%d)\n",
				__func__, mapping, count, align);
#ifdef CONFIG_BCM_IOVMM_DEBUG
		print_bitmap(mapping);
#endif
		return DMA_ERROR_CODE;
	}

	bitmap_set(mapping->bitmap, start, count);
	pr_debug("%s: map addr(%#x)(%d)KB\n", current->group_leader->comm,
			(mapping->base +
			 (start << (mapping->order + PAGE_SHIFT))),
			size >> 10);
	spin_unlock_irqrestore(&mapping->lock, flags);

	return mapping->base + (start << (mapping->order + PAGE_SHIFT));
}

static inline void __free_iova(struct dma_iommu_mapping *mapping,
			       dma_addr_t addr, size_t size)
{
	unsigned int start = (addr - mapping->base) >>
			     (mapping->order + PAGE_SHIFT);
	unsigned int count = ((size >> PAGE_SHIFT) +
			      (1 << mapping->order) - 1) >> mapping->order;
	unsigned long flags;

	spin_lock_irqsave(&mapping->lock, flags);
	pr_debug("%s: unmap addr(%#x)(%d)KB\n", current->group_leader->comm,
			(mapping->base +
			 (start << (mapping->order + PAGE_SHIFT))),
			size >> 10);
	bitmap_clear(mapping->bitmap, start, count);
	spin_unlock_irqrestore(&mapping->lock, flags);
}

/**
 * arm_iommu_map_sgt - map a SG table for streaming mode DMA
 * @dev: valid struct device pointer
 * @sgt: scatter gather table
 *
 * Map a set of buffers described by scatterlist in streaming mode for DMA.
 */
int arm_iommu_map_sgt(struct device *dev, struct sg_table *sgt, u32 align)
{
#ifdef LINUX_3_6_WAY
	struct dma_iommu_mapping *mapping = dev->archdata.mapping;
#else
	struct dma_iommu_mapping *mapping = __iommu_get_mapping(dev);
#endif
	dma_addr_t dma_addr, iova;
	int i, ret = DMA_ERROR_CODE;
	struct scatterlist *sg;
	unsigned int size = 0;

	if (!mapping)
		return DMA_ERROR_CODE;

	for_each_sg(sgt->sgl, sg, sgt->nents, i) {
		size += sg_dma_len(sg);
	}
	size = PAGE_ALIGN(size);

	dma_addr = __alloc_iova(mapping, size);
	if (dma_addr == DMA_ERROR_CODE)
		return dma_addr;

	iova = dma_addr;
	for_each_sg(sgt->sgl, sg, sgt->nents, i) {
		phys_addr_t phys = page_to_phys(sg_page(sg));
		unsigned int len = sg_dma_len(sg);

		ret = iommu_map(mapping->domain, iova, phys, len, 0);
		if (ret < 0)
			goto fail;
		iova += len;
	}
	return dma_addr;
fail:
	pr_err("Failed mapping dma_addr(%#x)\n", dma_addr);
	iommu_unmap(mapping->domain, dma_addr, iova-dma_addr);
	__free_iova(mapping, dma_addr, size);
	return DMA_ERROR_CODE;
}

int arm_iommu_unmap(struct device *dev, dma_addr_t iova, size_t size)
{
#ifdef LINUX_3_6_WAY
	struct dma_iommu_mapping *mapping = dev->archdata.mapping;
#else
	struct dma_iommu_mapping *mapping = __iommu_get_mapping(dev);
#endif

	if ((!mapping) || (iova == DMA_ERROR_CODE) || (size == 0)) {
		pr_err("%s FAILED: mapping(%p) iova(%#x) size(%#x)\n",
				__func__, mapping, iova, size);
		return DMA_ERROR_CODE;
	}
	/*
	 * add optional in-page offset from iova to size and align
	 * result to page size
	 */
	size = PAGE_ALIGN((iova & ~PAGE_MASK) + size);
	iova &= PAGE_MASK;

	iommu_unmap(mapping->domain, iova, size);
	__free_iova(mapping, iova, size);
	return 0;
}

/**
 * arm_iommu_create_mapping
 * @bus: pointer to the bus holding the client device (for IOMMU calls)
 * @base: start address of the valid IO address space
 * @size: size of the valid IO address space
 * @order: accuracy of the IO addresses allocations
 *
 * Creates a mapping structure which holds information about used/unused
 * IO address ranges, which is required to perform memory allocation and
 * mapping with IOMMU aware functions.
 *
 * The client device need to be attached to the mapping with
 * arm_iommu_attach_device function.
 */
struct dma_iommu_mapping *
arm_iommu_create_mapping(struct bus_type *bus, dma_addr_t base, size_t size,
			 int order)
{
	unsigned int count = size >> (PAGE_SHIFT + order);
	unsigned int bitmap_size = BITS_TO_LONGS(count) * sizeof(long);
	struct dma_iommu_mapping *mapping = NULL;
	int err = -ENOMEM;

	pr_debug("%s bus(%p) base(%x) size(%x) order(%d)\n",
			__func__, bus, base, size, order);
	if (!count)
		return ERR_PTR(-EINVAL);

	mapping = kzalloc(sizeof(struct dma_iommu_mapping), GFP_KERNEL);
	if (!mapping)
		return ERR_PTR(-ENOMEM);

	mapping->bitmap = kzalloc(bitmap_size, GFP_KERNEL);
	if (!mapping->bitmap)
		goto err;

	mapping->base = base;
	mapping->bits = BITS_PER_BYTE * bitmap_size;
	mapping->order = order;
	spin_lock_init(&mapping->lock);

	mapping->domain = iommu_domain_alloc(bus);
	if (!mapping->domain)
		goto err;

	kref_init(&mapping->kref);

	pr_info("Created mapping(%p) domain(%p) for iova(%#08x - %#08x) bitmap(%p) order(%d)\n",
			mapping, mapping->domain, base, (base + size),
			mapping->bitmap, order);
	return mapping;
err:
	kfree(mapping->bitmap);
	kfree(mapping);
	return ERR_PTR(err);
}

static void release_iommu_mapping(struct kref *kref)
{
	struct dma_iommu_mapping *mapping =
		container_of(kref, struct dma_iommu_mapping, kref);

	pr_info("Release mapping(%p)\n", mapping);
	if (mapping && mapping->domain)
		iommu_domain_free(mapping->domain);
	kfree(mapping->bitmap);
	kfree(mapping);
}

void arm_iommu_release_mapping(struct dma_iommu_mapping *mapping)
{
	if (mapping)
		kref_put(&mapping->kref, release_iommu_mapping);
}

/**
 * arm_iommu_attach_device
 * @dev: valid struct device pointer
 * @mapping: io address space mapping structure (returned from
 *	arm_iommu_create_mapping)
 *
 * Attaches specified io address space mapping to the provided device,
 * this replaces the dma operations (dma_map_ops pointer) with the
 * IOMMU aware version. More than one client might be attached to
 * the same io address space mapping.
 */
int arm_iommu_attach_device(struct device *dev,
			    struct dma_iommu_mapping *mapping)
{
	int err;

	pr_info("Attach dev(%p) to mapping(%p)\n", dev, mapping);
	err = iommu_attach_device(mapping->domain, dev);
	if (err) {
		pr_info("Failed attaching dev(%p) to mapping(%p)\n",
				dev, mapping);
		return err;
	}

	kref_get(&mapping->kref);

#ifdef LINUX_3_6_WAY
	dev->archdata.mapping = mapping;
	set_dma_ops(dev, &iommu_ops);

	pr_info("Attached IOMMU controller to %s device.\n", dev_name(dev));
	return 0;
#else
	return __iommu_add_device_node(dev, mapping);
#endif /* LINUX_3_6_WAY */

}

#ifdef CONFIG_OF
#define IOVMM_OF_READ(_prop_) \
	do { \
		if (of_property_read_u32(node, #_prop_, &val)) { \
			pr_err("ERROR: Prop \"" #_prop_ "\" not found\n"); \
			goto err; \
		} \
		pr_debug(#_prop_ " = %#x\n", val); \
		_prop_ = val; \
	} while (0)

#define IOVMM_OF_READ_OPT(_prop_) \
	do { \
		if (!of_property_read_u32(node, #_prop_, &val)) { \
			pr_debug(#_prop_ " = %#x\n", val); \
			_prop_ = val; \
		} \
	} while (0)
#endif /* CONFIG_OF */

static int iovmm_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *node = dev->of_node;
	struct dma_iommu_mapping *mapping = NULL;
	u32 val, base, size, order = 0;
	int ret = -EINVAL;

	if (dev_get_platdata(dev)) {
		struct bcm_iovmm_pdata *pdata = dev_get_platdata(dev);
		pr_info("Probe: via platform_data\n");
		base = pdata->base;
		size = pdata->size;
		order = pdata->order;
#ifdef CONFIG_OF
	} else if (dev->of_node) {
		pr_info("Probe: via DT framework\n");
		IOVMM_OF_READ(base);
		IOVMM_OF_READ(size);
		IOVMM_OF_READ_OPT(order);
#endif /* CONFIG_OF */
	} else {
		pr_err("Probe Fail: No platform_data, no DT\n");
		goto err;
	}

	mapping = arm_iommu_create_mapping(&platform_bus_type, base, size,
			order);
	if (IS_ERR_OR_NULL(mapping)) {
		pr_err("Unable to create mapping\n");
		goto err;
	}

	/* Set the mdata as platform device private data */
	ret = dev_set_drvdata(dev, mapping);
	if (ret) {
		pr_err("Unable to initialize driver data\n");
		goto err;
	}

	pr_info("Created mapping(%p) domain(%p) for iova(%#08x - %#08x) order(%d)\n",
			mapping, mapping->domain, base, (base + size), order);
	return 0;

err:
	if (mapping)
		arm_iommu_release_mapping(mapping);
	pr_err("Failed to probe\n");
	return ret;
}

static int iovmm_remove(struct platform_device *pdev)
{
	struct dma_iommu_mapping *mapping = platform_get_drvdata(pdev);

	pr_info("%s\n", __func__);
	if (mapping)
		arm_iommu_release_mapping(mapping);
	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id iovmm_of_match[] = {
	{ .compatible = "bcm,iovmm", },
	{},
};
#else
#define iovmm_of_match NULL
#endif /* CONFIG_OF */


static struct platform_driver iovmm_driver = {
	.probe		= iovmm_probe,
	.remove		= iovmm_remove,
	.driver = {
		.name	= "iovmm-bcm",
		.owner	= THIS_MODULE,
		.of_match_table = iovmm_of_match,
	},
};

static int __init iovmm_init(void)
{
	pr_info("%s\n", __func__);
	return platform_driver_register(&iovmm_driver);
}

static void __exit iovmm_exit(void)
{
	pr_info("%s\n", __func__);
	platform_driver_unregister(&iovmm_driver);
}

subsys_initcall(iovmm_init);
module_exit(iovmm_exit);

MODULE_AUTHOR("Nishanth Peethambaran <nishanth@broadcom.com>");
MODULE_DESCRIPTION("IOVA management driver");
MODULE_LICENSE("GPL v2");

