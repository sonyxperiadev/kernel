/*
 * drivers/gpu/bcm/bcm_ion.c
 *
 * Copyright (C) 2011 Broadcom, Inc.
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

#define pr_fmt(fmt) "ion-bcm: " fmt

#include <linux/err.h>
#include <linux/scatterlist.h>
#include <linux/ion.h>
#include <linux/broadcom/bcm_ion.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include "../ion_priv.h"
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/module.h>
#include <linux/list.h>
#include <linux/of_platform.h>
#ifdef CONFIG_OF
#include <linux/of.h>
#endif /* CONFIG_OF */
#include <asm/dma-contiguous.h>
#include <linux/dma-mapping.h>
#ifdef CONFIG_IOMMU_API
#include <linux/iommu.h>
#endif
#ifdef CONFIG_BCM_IOVMM
#include <plat/bcm_iommu.h>
#endif

struct bcm_ion_heap {
	struct ion_heap *heap;
	struct platform_device *pdev;
	struct list_head node;
};

static LIST_HEAD(bcm_heap_list);
struct ion_device *idev;

/**
 * DOC: Default heap masks for broadcom heaps.
 *
 * Assumption of heap configuration is:
 * Heap0 - System heap with dma address limited to 256 MB range
 * Heap1 - System heap with unlimited dma address range
 * Heap2 - Legacy (CMA heap with 256 MB dma address limit)
 * Heap3 - Legacy (Carveout heap with 256 MB dma address limit)
 *
 * Heap4,7 - Reserved
 * Heap5 - CMA heap with 256MB address limit
 * Heap6 - CMA heap with unlimited dma address
 *
 * Heap8,11 - Reserved
 * Heap9  - Carveout heap with 256MB address limit
 * Heap10 - Carveout heap with unlimited dma address
 *
 * Heap12-14 - Secure heaps
 */
static struct ion_custom_config_data bcm_ion_config_data = {
	.version = BCM_ION_VERSION,
	.mask_secure = 0xF000,
	.mask_256M = 0x22D,
	.mask_fast = 0x608,
	.mask_hwwr = 0x608,
};

unsigned int bcm_ion_map_dma(struct ion_client *client,
		struct ion_handle *handle)
{
	struct ion_buffer *buffer;
	unsigned int dma_addr = 0;

	buffer = ion_lock_buffer(client, handle);
	if (!buffer) {
		pr_err("%s could not lock the buffer\n", __func__);
		return 0;
	}

	dma_addr = buffer->dma_addr;
	ion_unlock_buffer(client, buffer);

	if (dma_addr == ION_DMA_ADDR_FAIL) {
		pr_err("%s No valid DMA address\n", __func__);
		return 0;
	}
	return dma_addr;
}
EXPORT_SYMBOL(bcm_ion_map_dma);

static int bcm_ion_set_prop(struct ion_client *client,
		struct ion_custom_property *data)
{
	struct ion_buffer *buffer;

	buffer = ion_lock_buffer(client, data->handle);
	if (buffer) {
		buffer->custom_flags = (data->value & data->mask);
		ion_unlock_buffer(client, buffer);
		return 0;
	}
	return -EINVAL;
}

static int bcm_ion_get_prop(struct ion_client *client,
		struct ion_custom_property *data)
{
	struct ion_buffer *buffer;

	buffer = ion_lock_buffer(client, data->handle);
	if (buffer) {
		data->value = (buffer->custom_flags & data->mask);
		ion_unlock_buffer(client, buffer);
		return 0;
	}
	return -EINVAL;
}

static int bcm_ion_update_count(struct ion_client *client,
		struct ion_handle *handle)
{
	struct ion_buffer *buffer;

	buffer = ion_lock_buffer(client, handle);
	if (buffer) {
		buffer->custom_update_count++;
		ion_unlock_buffer(client, buffer);
		return 0;
	}
	return -EINVAL;
}

static int bcm_ion_get_update_count(struct ion_client *client,
		struct ion_custom_update_count *data)
{
	struct ion_buffer *buffer;

	buffer = ion_lock_buffer(client, data->handle);
	if (buffer) {
		data->count = buffer->custom_update_count;
		ion_unlock_buffer(client, buffer);
		return 0;
	}
	return -EINVAL;
}

static int bcm_is_region_ok(struct ion_buffer *buffer,
		unsigned long offset, unsigned long len)
{
	if ((offset > buffer->size) || ((offset + len) > buffer->size)
			|| (len > buffer->size))
		return 0;
	return 1;
}

static int bcm_ion_cache_clean(struct ion_client *client,
		struct ion_custom_region_data *data)
{
	struct ion_buffer *buffer;
	int ret = -EINVAL;

	buffer = ion_lock_buffer(client, data->handle);
	if (buffer && buffer->heap->ops->clean_cache) {
		if (bcm_is_region_ok(buffer, data->offset, data->len))
			ret = buffer->heap->ops->clean_cache(buffer->heap,
					buffer,	data->offset, data->len);
		ion_unlock_buffer(client, buffer);
	}
	return ret;
}

static int bcm_ion_cache_invalidate(struct ion_client *client,
		struct ion_custom_region_data *data)
{
	struct ion_buffer *buffer;
	int ret = -EINVAL;

	buffer = ion_lock_buffer(client, data->handle);
	if (buffer && buffer->heap->ops->invalidate_cache) {
		if (bcm_is_region_ok(buffer, data->offset, data->len))
			ret = buffer->heap->ops->invalidate_cache(buffer->heap,
					buffer,	data->offset, data->len);
		ion_unlock_buffer(client, buffer);
	}
	return ret;
}

unsigned int bcm_ion_get_heapmask(unsigned int flags)
{
	struct ion_custom_config_data *data = &bcm_ion_config_data;
	unsigned int heap_mask;

	if (flags & ION_FLAG_SECURE) {
		heap_mask = data->mask_secure;
	} else {
		heap_mask = ION_DEFAULT_HEAP & ~(data->mask_secure)
			& ~(data->mask_fast) & ~(data->mask_hwwr);
		if (flags & ION_FLAG_FAST_ALLOC)
			heap_mask |= data->mask_fast;
		if (flags & ION_FLAG_HWWR)
			heap_mask |= data->mask_hwwr;
		if (flags & ION_FLAG_256M)
			heap_mask &= data->mask_256M;
	}
	return heap_mask;
}
EXPORT_SYMBOL(bcm_ion_get_heapmask);

/**
 * These correspond to the flags present in memtrack.h in memtrack HAL
 */
enum {
	/* ION_MT_GRP1_ACC_E, */
	ION_MT_GRP1_UNACC_E,
	ION_MT_GRP1_MAX_E,
};

enum {
	ION_MT_GRP2_SH_E,
	ION_MT_GRP2_SHPSS_E,
	ION_MT_GRP2_PRIV_E,
	ION_MT_GRP2_MAX_E,
};

enum {
	ION_MT_GRP3_NS_SYS_E,
	ION_MT_GRP3_NS_DED_E,
	ION_MT_GRP3_S_DED_E,
	ION_MT_GRP3_MAX_E,
};

#define ION_MT_SIZES          (ION_MT_GRP1_MAX_E * ION_MT_GRP2_MAX_E \
		* ION_MT_GRP3_MAX_E)

struct bcm_ion_mt_data_t {
	pid_t pid;
	unsigned int type;
	unsigned int *sizes;
};

/**
 * Callback function which will be called for each buffer allocated/imported
 * by the ion client. Used for supporting memtrack HAL
 */
void bcm_ion_buffer_process(struct ion_buffer *buffer, void *arg)
{
	struct bcm_ion_mt_data_t *ion_mt_data = (struct bcm_ion_mt_data_t *)arg;
	unsigned int *sizes = ion_mt_data->sizes;
	unsigned int size, tmp, index = 0;

	size = buffer->size;

	/**
	 * Hierarchy is GRP3->GRP2->GRP1
	 * index = GRP3 * (GRP2_MAX * GRP1_MAX)
	 *       + GRP2 * (GRP1_MAX)
	 **/

	/* Check the buffer type - OTH, GL, GFX, ..) */
	tmp = (buffer->flags & ION_FLAG_USAGE_MASK) >> ION_FLAG_USAGE_OFFSET;
	if (tmp != ion_mt_data->type)
		return;

	/* Modify the index based on GRP3 - system/dedicated/secure */
	switch (buffer->heap->type) {

	case ION_HEAP_TYPE_CARVEOUT:
	case ION_HEAP_TYPE_CHUNK:
		tmp = ION_MT_GRP3_NS_DED_E;
		break;
	case ION_HEAP_TYPE_SECURE:
		tmp = ION_MT_GRP3_S_DED_E;
		break;
	default:
		tmp = ION_MT_GRP3_NS_SYS_E;
		break;
	}
	index += tmp * ION_MT_GRP2_MAX_E * ION_MT_GRP1_MAX_E;

	/* Modify the index based on GRP2 - shared/private */
	if (buffer->handle_count > 1)
		tmp = ION_MT_GRP2_SH_E;
	else
		tmp = ION_MT_GRP2_PRIV_E;
	index += tmp * ION_MT_GRP1_MAX_E;

	/* Skip index adjustment for GRP1 as ION tracks only unaccounted mem */

	/* Increment the size for the type.flags */
	sizes[index] += size;
}

int bcm_ion_mt_get_mem(struct ion_client *client,
		struct ion_custom_mt_get_mem *data, unsigned int *sizes)
{
	struct ion_client *client_remote;
	struct bcm_ion_mt_data_t ion_mt_data;
	int i;

	ion_mt_data.pid   = data->pid;
	ion_mt_data.type  = data->type;
	ion_mt_data.sizes = sizes;

	/* Sanity check the version, sizes and num_sizes */
	if ((data->version != 0) || (data->sizes == NULL)
			|| (data->num_sizes != ION_MT_SIZES)) {
		pr_err("ION_IOC_CUSTOM_MT_GET_MEM param mismatch\n");
		return -EINVAL;
	}

	/* Get the client corresponding to the pid
	 * If client matching the pid is not present, return all sizes
	 * as zero, the call should not fail */
	client_remote = ion_client_get_from_pid(client, data->pid);
	if (client_remote == NULL) {
		pr_debug("ION_IOC_CUSTOM_MT_GET_MEM pid(%d) not used\n",
				data->pid);
		return 0;
	}

	/* Loop for all buffers which are allocated/imported by the
	 * requested pid(client) */
	ion_client_foreach_buffer(client_remote, bcm_ion_buffer_process,
			(void *)&ion_mt_data);

	/* Release the client reference */
	ion_client_put(client_remote);

	return 0;
}


static long bcm_ion_custom_ioctl(struct ion_client *client,
				      unsigned int cmd,
				      unsigned long arg)
{
	switch (cmd) {
	case ION_IOC_CUSTOM_DMA_MAP:
	{
		struct ion_custom_dma_map_data data;

		if (copy_from_user(&data, (void __user *)arg, sizeof(data)))
			return -EFAULT;

		pr_debug("ION_IOC_CUSTOM_DMA_MAP client(%p) handle(%p)\n",
				client, data.handle);
		data.dma_addr = bcm_ion_map_dma(client, data.handle);

		if (copy_to_user((void __user *)arg, &data, sizeof(data)))
			return -EFAULT;
		break;
	}
	case ION_IOC_CUSTOM_SET_PROP:
	{
		struct ion_custom_property data;

		if (copy_from_user(&data, (void __user *)arg, sizeof(data)))
			return -EFAULT;

		pr_debug("ION_IOC_CUSTOM_SET_PROP client(%p) handle(%p) value(%x) mask(%x)\n",
				client, data.handle, data.value, data.mask);
		if (bcm_ion_set_prop(client, &data))
			return -EINVAL;

		break;
	}
	case ION_IOC_CUSTOM_GET_PROP:
	{
		struct ion_custom_property data;

		if (copy_from_user(&data, (void __user *)arg, sizeof(data)))
			return -EFAULT;

		pr_debug("ION_IOC_CUSTOM_GET_PROP client(%p) handle(%p) mask(%x)\n",
				client, data.handle, data.mask);
		if (bcm_ion_get_prop(client, &data))
			return -EINVAL;

		if (copy_to_user((void __user *)arg, &data, sizeof(data)))
			return -EFAULT;
		break;
	}
	case ION_IOC_CUSTOM_UPDATE:
	{
		struct ion_handle *handle = (struct ion_handle *)arg;

		pr_debug("ION_IOC_CUSTOM_UPDATE client(%p) handle(%p)\n",
				client, handle);
		if (bcm_ion_update_count(client, handle))
			return -EINVAL;

		break;
	}
	case ION_IOC_CUSTOM_GET_UPDATE_COUNT:
	{
		struct ion_custom_update_count data;

		if (copy_from_user(&data, (void __user *)arg, sizeof(data)))
			return -EFAULT;

		pr_debug("ION_IOC_CUSTOM_GET_UPDATE_COUNT client(%p) handle(%p)\n",
				client, data.handle);
		if (bcm_ion_get_update_count(client, &data))
			return -EINVAL;

		if (copy_to_user((void __user *)arg, &data, sizeof(data)))
			return -EFAULT;
		break;
	}
	case ION_IOC_CUSTOM_CACHE_CLEAN:
	{
		struct ion_custom_region_data data;

		if (copy_from_user(&data, (void __user *)arg, sizeof(data)))
			return -EFAULT;

		pr_debug("ION_IOC_CUSTOM_CACHE_CLEAN client(%p) handle(%p)\n",
				client, data.handle);
		if (bcm_ion_cache_clean(client, &data))
			return -EINVAL;

		if (copy_to_user((void __user *)arg, &data, sizeof(data)))
			return -EFAULT;
		break;
	}
	case ION_IOC_CUSTOM_CACHE_INVALIDATE:
	{
		struct ion_custom_region_data data;

		if (copy_from_user(&data, (void __user *)arg, sizeof(data)))
			return -EFAULT;

		pr_debug("ION_IOC_CUSTOM_CACHE_INVALIDATE client(%p) handle(%p)\n",
				client, data.handle);
		if (bcm_ion_cache_invalidate(client, &data))
			return -EINVAL;

		if (copy_to_user((void __user *)arg, &data, sizeof(data)))
			return -EFAULT;
		break;
	}
	case ION_IOC_CUSTOM_GET_CONFIG:
	{
		pr_debug("ION_IOC_CUSTOM_GET_CONFIG client(%p)\n", client);

		if (copy_to_user((void __user *)arg, &bcm_ion_config_data,
					sizeof(bcm_ion_config_data)))
			return -EFAULT;
		break;
	}
	case ION_IOC_CUSTOM_MT_GET_MEM:
	{
		struct ion_custom_mt_get_mem data;
		unsigned int *sizes;

		if (copy_from_user(&data, (void __user *)arg, sizeof(data)))
			return -EFAULT;

		pr_debug("ION_IOC_CUSTOM_MT_GET_MEM pid(%d) type(%d) n(%d)\n",
				data.pid, data.type, data.num_sizes);

		sizes = kzalloc(ION_MT_SIZES * sizeof(unsigned int),
				GFP_KERNEL);
		if (!sizes)
			return -ENOMEM;

		if (bcm_ion_mt_get_mem(client, &data, sizes)) {
			kfree(sizes);
			return -EINVAL;
		}

		if (copy_to_user((void __user *)data.sizes, sizes,
					ION_MT_SIZES * sizeof(unsigned int))) {
			kfree(sizes);
			return -EFAULT;
		}
		kfree(sizes);
		break;
	}
	case ION_IOC_CUSTOM_TP:
	{
		pr_debug(" TP(%ld)\n", arg);
		break;
	}
	default:
		pr_err(" unsupported custom ioctl(%d)\n", cmd);
		return -ENOTTY;
	}
	return 0;
}

#ifdef CONFIG_OF
#define ION_OF_READ(_prop_) \
	do { \
		if (of_property_read_u32(node, #_prop_, &val)) { \
			pr_err("ERROR: Prop \"" #_prop_ "\" not found\n"); \
			goto of_err; \
		} \
		pr_debug(#_prop_ " = %#x\n", val); \
		heap_data->_prop_ = val; \
	} while (0)

#define ION_OF_READ_OPT(_prop_) \
	do { \
		if (!of_property_read_u32(node, #_prop_, &val)) { \
			pr_debug(#_prop_ " = %#x\n", val); \
			heap_data->_prop_ = val; \
		} \
	} while (0)

#ifdef CONFIG_CMA
static u64 ion_dmamask = DMA_BIT_MASK(32);
#endif

static void bcm_ion_free_data(struct device *dev)
{
	if (dev->of_node) {
		struct ion_platform_heap *heap_data = dev->platform_data;
		if (heap_data) {
			kfree(heap_data->name);
			kfree(heap_data);
			dev->platform_data = NULL;
		}
	}
}

static struct ion_platform_heap *bcm_ion_parse_dt(struct device *dev)
{
	struct device_node *node = dev->of_node;
	struct bcm_ion_heap_reserve_data *heap_init_data;
	struct ion_platform_heap *heap_data = NULL;
#ifdef CONFIG_IOMMU_API
	struct device_node *tmp_node;
	struct platform_device *pdev_iommu;
#endif
#ifdef CONFIG_BCM_IOVMM
	struct platform_device *pdev_iovmm;
	struct dma_iommu_mapping *mapping;
#endif /* CONFIG_BCM_IOVMM */
	const char *name;
	u32 val;
	int ret = -EINVAL;

	heap_data = kzalloc(sizeof(struct ion_platform_heap),
			GFP_KERNEL);
	if (!heap_data) {
		pr_err("ERROR: Platform heap allocation failed\n");
		return ERR_PTR(-ENOMEM);
	}
	if (of_property_read_string(node, "name", &name)) {
		pr_err("ERROR: Property \"name\" not found\n");
		goto err;
	}
	heap_data->name = kzalloc((strlen(name) + 1), GFP_KERNEL);
	if (!heap_data->name) {
		pr_err("ERROR: Couldn't allocate memory for ion name\n");
		ret = -ENOMEM;
		goto err;
	}
	strncpy((char *)heap_data->name, name, strlen(name) + 1);

	ION_OF_READ(type);
	ION_OF_READ(id);
	if ((heap_data->type == ION_HEAP_TYPE_CARVEOUT) ||
			(heap_data->type == ION_HEAP_TYPE_DMA) ||
			(heap_data->type == ION_HEAP_TYPE_SECURE)) {
		if (bcm_ion_get_heap_reserve_data(&heap_init_data,
					heap_data->name)) {
			pr_err("%16s: Memory was not reserved\n",
					heap_data->name);
			goto of_err;
		}
#ifdef CONFIG_CMA
		if (heap_data->type == ION_HEAP_TYPE_DMA) {
			struct cma *cma;
			dev->dma_mask = &ion_dmamask;
			dev->coherent_dma_mask = DMA_BIT_MASK(32);
			cma = dev_get_cma_area(&heap_init_data->cma_dev);
			dev_set_cma_area(dev, cma);
			heap_data->priv = dev;
		}
#endif
		heap_data->base = heap_init_data->base;
		heap_data->size = heap_init_data->size;
		ION_OF_READ_OPT(lmk_enable);
		ION_OF_READ_OPT(lmk_min_score_adj);
		ION_OF_READ_OPT(lmk_min_free);
	}
#ifndef CONFIG_BCM_IOVMM
	if (heap_data->type == ION_HEAP_TYPE_SYSTEM) {
		pr_err("%16s: Not supported without iovmm\n",
				heap_data->name);
		goto of_err;
	}
#endif
#ifdef CONFIG_IOMMU_API
	/* Get the iommu device and link ion dev to iommu dev */
	tmp_node = of_parse_phandle(dev->of_node, "iommu", 0);
	if (tmp_node  == NULL) {
		pr_err("%16s: Get iommu node failed\n", heap_data->name);
		goto of_err;
	}
	pdev_iommu = of_find_device_by_node(tmp_node);
	if (pdev_iommu == NULL) {
		pr_err("%16s: Get iommu device failed\n", heap_data->name);
		goto of_err;
	}
	dev->archdata.iommu = &pdev_iommu->dev;
	heap_data->device = dev;
#endif /* CONFIG_IOMMU_API */

#ifdef CONFIG_BCM_IOVMM
	/* Get the iommu mapping and attach ion dev to mapping */
	tmp_node = of_parse_phandle(dev->of_node, "iovmm", 0);
	if (tmp_node  == NULL) {
		pr_err("%16s: Get iovmm node failed\n",
				heap_data->name);
		goto of_err;
	}
	pdev_iovmm = of_find_device_by_node(tmp_node);
	if (pdev_iovmm == NULL) {
		pr_err("%16s: Get iovmm device failed\n",
				heap_data->name);
		goto of_err;
	}
	mapping = platform_get_drvdata(pdev_iovmm);
	if (arm_iommu_attach_device(dev, mapping)) {
		pr_err("%s Attaching dev(%p) to mapping(%p) failed\n",
				heap_data->name, dev, mapping);
		goto of_err;
	}
	pr_info("%16s: Linked to iommu-mapping(%p)\n", heap_data->name,
			mapping);
#endif /* CONFIG_BCM_IOVMM */

	dev->platform_data = heap_data;
	return heap_data;
of_err:
	kfree(heap_data->name);
err:
	kfree(heap_data);
	return ERR_PTR(ret);
}
#else

static void bcm_ion_free_data(struct device *dev) { }

#endif /* CONFIG_OF */

static struct ion_platform_heap *bcm_ion_parse_pdata(struct device *dev)
{
	struct bcm_ion_heap_reserve_data *heap_init_data;
	struct ion_platform_data *pdata = dev->platform_data;
	struct ion_platform_heap *heap_data;
	int i;
#ifdef CONFIG_IOMMU_API
	struct platform_device *pdev_iommu;
#endif
#ifdef CONFIG_BCM_IOVMM
	struct platform_device *pdev_iovmm;
	struct dma_iommu_mapping *mapping;
#endif

	if (pdata->nr <= 0)
		return NULL;
	for (i = 0; i < pdata->nr; i++) {
		heap_data = &pdata->heaps[i];

		if ((heap_data->type == ION_HEAP_TYPE_CARVEOUT) ||
				(heap_data->type == ION_HEAP_TYPE_DMA)) {
			if (bcm_ion_get_heap_reserve_data(&heap_init_data,
						heap_data->name)) {
				pr_err("%16s: Memory was not reserved\n",
						heap_data->name);
				heap_data->id = ION_INVALID_HEAP_ID;
				continue;
			}
#ifdef CONFIG_CMA
			if (heap_data->type == ION_HEAP_TYPE_DMA) {
				struct cma *cma;
				cma = dev_get_cma_area(
						&heap_init_data->cma_dev);
				dev_set_cma_area(dev, cma);
				heap_data->priv = dev;
			}
#endif
			heap_data->base = heap_init_data->base;
			heap_data->size = heap_init_data->size;
		}
#ifdef CONFIG_IOMMU_API
		heap_data->device = dev;
#endif
#ifndef CONFIG_BCM_IOVMM
		if (heap_data->type == ION_HEAP_TYPE_SYSTEM) {
			pr_err("%16s: Not supported without iovmm\n",
					heap_data->name);
			heap_data->id = ION_INVALID_HEAP_ID;
		}
#endif
	}
#ifdef CONFIG_IOMMU_API
	heap_data = &pdata->heaps[0];
	pdev_iommu = pdata->pdev_iommu;
	/* Get the iommu device and link ion dev to iommu dev */
	if (pdev_iommu == NULL) {
		pr_err("%16s: Get iommu device failed\n",
				heap_data->name);
		return ERR_PTR(-EINVAL);
	}
	dev->archdata.iommu = &pdev_iommu->dev;
	heap_data->device = dev;
#endif /* CONFIG_IOMMU_API */

#ifdef CONFIG_BCM_IOVMM
	/* Get the iommu mapping and attach ion dev to mapping */
	pdev_iovmm = pdata->pdev_iovmm;
	if (pdev_iovmm == NULL) {
		pr_err("%16s: Get iovmm device failed\n",
				heap_data->name);
		return ERR_PTR(-EINVAL);
	}
	mapping = platform_get_drvdata(pdev_iovmm);
	if (arm_iommu_attach_device(dev, mapping)) {
		pr_err("%s Attaching dev(%p) to mapping(%p) failed\n",
				heap_data->name, dev, mapping);
		return ERR_PTR(-EINVAL);
	}
	pr_info("%16s: Linked to iommu-mapping(%p)\n", heap_data->name,
			mapping);
#endif /* CONFIG_BCM_IOVMM */

	return pdata->heaps;
}

static int bcm_ion_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct ion_platform_heap *heap_datas;
	int i, num_heaps;

	if (!pdev) {
		pr_err("Unable to probe\n");
		return -ENODEV;
	}

	/* Create ion misc device */
	if (!idev) {
		idev = ion_device_create(bcm_ion_custom_ioctl);
		if (IS_ERR_OR_NULL(idev)) {
			pr_err("Probe Fail: ION device creation failed\n");
			return PTR_ERR(idev);
		}
	}

	/* Parse dtb or pdata to get number of heaps and their properies */
	if (dev_get_platdata(dev)) {
		struct ion_platform_data *pdata = dev->platform_data;
		pr_info("Probe: Add (%d)heaps via platform_data\n", pdata->nr);
		num_heaps = pdata->nr;
		heap_datas = bcm_ion_parse_pdata(dev);
		if (IS_ERR_OR_NULL(heap_datas)) {
			pr_err("Probe Fail: pdata parsing failed\n");
			return PTR_ERR(heap_datas);
		}
#ifdef CONFIG_OF
	} else if (dev->of_node) {
		pr_info("Probe: via DT framework\n");
		num_heaps = 1;
		heap_datas = bcm_ion_parse_dt(dev);
		if (IS_ERR_OR_NULL(heap_datas)) {
			pr_err("Probe Fail: DT parsing failed\n");
			return PTR_ERR(heap_datas);
		}
#endif /* CONFIG_OF */
	} else {
		pr_err("Probe Fail: No platform_data, no DT\n");
		return -EINVAL;
	}

	for (i = 0; i < num_heaps; i++) {
		struct ion_platform_heap *heap_data = &heap_datas[i];
		struct ion_heap *heap;
		struct bcm_ion_heap *bcm_heap;

		pr_info("Heap(%10s) Type(%d) Id(%d)\n",
				heap_data->name, heap_data->type,
				heap_data->id);
		/* Ignore heaps with duplicate heap ids. */
		list_for_each_entry(bcm_heap, &bcm_heap_list, node) {
			if (heap_data->id == bcm_heap->heap->id) {
				heap_data->id = ION_INVALID_HEAP_ID;
				break;
			}
		}
		/* Ignore heaps with invalid heap ids. */
		if (heap_data->id >= ION_INVALID_HEAP_ID) {
			bcm_ion_free_data(dev);
			continue;
		}
		/* Create and add the heap. */
		heap = ion_heap_create(heap_data);
		if (IS_ERR_OR_NULL(heap)) {
			bcm_ion_free_data(dev);
			continue;
		}
		ion_device_add_heap(idev, heap);
#ifdef CONFIG_IOMMU_API
#ifndef CONFIG_BCM_IOVMM
		heap->domain = iommu_domain_alloc(&platform_bus_type);
		if (iommu_attach_device(heap->domain, dev)) {
			pr_err("%s Attaching dev(%p) to iommu failed\n",
					heap_data->name, dev);
			bcm_ion_free_data(dev);
			ion_heap_destroy(heap);
			continue;
		}
#endif /* CONFIG_BCM_IOVMM */
#endif /* CONFIG_IOMMU_API */

		/* Add to heap list */
		bcm_heap = kzalloc(sizeof(struct bcm_ion_heap), GFP_KERNEL);
		if (!bcm_heap) {
			pr_err("Allocation of bcm_heap failed\n");
			bcm_ion_free_data(dev);
			ion_heap_destroy(heap);
			continue;
		}
		bcm_heap->pdev = pdev;
		bcm_heap->heap = heap;
		list_add_tail(&bcm_heap->node, &bcm_heap_list);
		pr_info("Added Heap(%10s) Type(%d) Id(%d) Base(0x%08x) Size (0x%08x)\n",
				heap_data->name, heap_data->type,
				heap_data->id, (u32)heap_data->base,
				heap_data->size);
	}
	platform_set_drvdata(pdev, idev);
	return 0;
}

static int bcm_ion_remove(struct platform_device *pdev)
{
	struct ion_device *idev = platform_get_drvdata(pdev);
	struct bcm_ion_heap *bcm_heap, *tmp_heap;
	struct device *dev = &pdev->dev;

	pr_info("Broadcom ION device remove\n");

	/* Ignore heaps with duplicate heap ids. */
	list_for_each_entry_safe(bcm_heap, tmp_heap, &bcm_heap_list, node) {
		if (pdev == bcm_heap->pdev) {
			pr_info("Remove heap id(%d)\n", bcm_heap->heap->id);
			bcm_ion_free_data(dev);
			ion_heap_destroy(bcm_heap->heap);
			list_del(&bcm_heap->node);
			kfree(bcm_heap);
		}
	}
	if (list_empty(&bcm_heap_list)) {
		pr_info("Destroy ion device\n");
		ion_device_destroy(idev);
		idev = NULL;
	}
	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id ion_of_match[] = {
	{ .compatible = "bcm,ion", },
	{},
};
#else
#define ion_of_match NULL
#endif /* CONFIG_OF */

static struct platform_driver ion_driver = {
	.probe = bcm_ion_probe,
	.remove = bcm_ion_remove,
	.driver = {
		.name = "ion-bcm",
		.owner = THIS_MODULE,
		.of_match_table = ion_of_match,
	},
};

static int __init ion_init(void)
{
	pr_info("Broadcom ION driver init\n");
	return platform_driver_register(&ion_driver);
}

static void __exit ion_exit(void)
{
	pr_info("Broadcom ION driver exit\n");
	platform_driver_unregister(&ion_driver);
}

module_init(ion_init);
module_exit(ion_exit);

/***************************************************
 * Get the heap info to be shared with userspace for
 * selecting heap mask for allocation.
 **************************************************/
#ifdef CONFIG_OF
#define ION_CONFIG_OF_READ_OPT(_prop_, _var_) \
	do { \
		if (!of_property_read_u32(node, #_prop_, &_var_)) \
			pr_debug(#_prop_ " = %#x\n", _var_); \
	} while (0)
#endif

static int bcm_ion_config_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct ion_custom_config_data *data = &bcm_ion_config_data;

	if (!pdev) {
		pr_err("config: Unable to probe\n");
		return -ENODEV;
	}

	/* Parse dtb or pdata to get ion config info */
	if (dev_get_platdata(dev)) {
		struct ion_custom_config_data *pdata = dev->platform_data;

		pr_info("config: Probe: via pdata\n");
		memcpy(data, pdata, sizeof(bcm_ion_config_data));
		data->version = BCM_ION_VERSION;
#ifdef CONFIG_OF
	} else if (dev->of_node) {
		struct device_node *node = dev->of_node;

		pr_info("config: Probe: via DT framework\n");
		ION_CONFIG_OF_READ_OPT(mask_secure, data->mask_secure);
		ION_CONFIG_OF_READ_OPT(mask_256M, data->mask_256M);
		ION_CONFIG_OF_READ_OPT(mask_fast, data->mask_fast);
		ION_CONFIG_OF_READ_OPT(mask_hwwr, data->mask_hwwr);
		data->version = BCM_ION_VERSION;
#endif /* CONFIG_OF */
	} else {
		pr_err("config: Probe Fail: No platform_data, no DT\n");
		return -EINVAL;
	}
	pr_info("config: version(%x) secure(%x) 256M(%x) fast(%x) hwwr(%x)\n",
	       data->version, data->mask_secure, data->mask_256M,
	       data->mask_fast, data->mask_hwwr);
	return 0;
}

static int bcm_ion_config_remove(struct platform_device *pdev)
{
	pr_info("Broadcom ION config device remove\n");
	return 0;
}


#ifdef CONFIG_OF
static const struct of_device_id ion_config_of_match[] = {
	{ .compatible = "bcm,ion-config", },
	{},
};
#else
#define ion_config_of_match NULL
#endif /* CONFIG_OF */

static struct platform_driver ion_config_driver = {
	.probe = bcm_ion_config_probe,
	.remove = bcm_ion_config_remove,
	.driver = {
		.name = "ion-config-bcm",
		.owner = THIS_MODULE,
		.of_match_table = ion_config_of_match,
	},
};

static int __init ion_config_init(void)
{
	pr_info("Broadcom ION config driver init\n");
	return platform_driver_register(&ion_config_driver);
}

static void __exit ion_config_exit(void)
{
	pr_info("Broadcom ION config driver exit\n");
	platform_driver_unregister(&ion_config_driver);
}

module_init(ion_config_init);
module_exit(ion_config_exit);

