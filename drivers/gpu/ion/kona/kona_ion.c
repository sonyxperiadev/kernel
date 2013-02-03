/*
 * drivers/gpu/kona/kona_ion.c
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

#define pr_fmt(fmt) "ion-kona: " fmt

#include <linux/err.h>
#include <linux/scatterlist.h>
#include <linux/ion.h>
#include <linux/broadcom/kona_ion.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include "../ion_priv.h"
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/module.h>
#ifdef CONFIG_OF
#include <linux/of.h>
#include <asm/dma-contiguous.h>
#include <linux/dma-mapping.h>
#endif /* CONFIG_OF */

struct ion_device *idev;
static int num_heaps;
static struct ion_heap **heaps;

unsigned int kona_ion_map_dma(struct ion_client *client,
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
EXPORT_SYMBOL(kona_ion_map_dma);

static int kona_ion_set_prop(struct ion_client *client,
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

static int kona_ion_get_prop(struct ion_client *client,
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

static int kona_ion_update_count(struct ion_client *client,
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

static int kona_ion_get_update_count(struct ion_client *client,
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

static int kona_is_region_ok(struct ion_buffer *buffer,
		unsigned long offset, unsigned long len)
{
	if ((offset > buffer->size) || ((offset + len) > buffer->size)
			|| (len > buffer->size))
		return 0;
	return 1;
}

static int kona_ion_cache_flush(struct ion_client *client,
		struct ion_custom_region_data *data)
{
	struct ion_buffer *buffer;
	int ret = -EINVAL;

	buffer = ion_lock_buffer(client, data->handle);
	if (buffer && buffer->heap->ops->flush_cache) {
		if (kona_is_region_ok(buffer, data->offset, data->len))
			ret = buffer->heap->ops->flush_cache(buffer->heap,
					buffer,	data->offset, data->len);
		ion_unlock_buffer(client, buffer);
	}
	return ret;
}

static int kona_ion_cache_invalidate(struct ion_client *client,
		struct ion_custom_region_data *data)
{
	struct ion_buffer *buffer;
	int ret = -EINVAL;

	buffer = ion_lock_buffer(client, data->handle);
	if (buffer && buffer->heap->ops->invalidate_cache) {
		if (kona_is_region_ok(buffer, data->offset, data->len))
			ret = buffer->heap->ops->invalidate_cache(buffer->heap,
					buffer,	data->offset, data->len);
		ion_unlock_buffer(client, buffer);
	}
	return ret;
}

static long kona_ion_custom_ioctl(struct ion_client *client,
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
		data.dma_addr = kona_ion_map_dma(client, data.handle);

		if (copy_to_user((void __user *)arg, &data, sizeof(data)))
			return -EFAULT;
		break;
	}
	case ION_IOC_CUSTOM_SET_PROP:
	{
		struct ion_custom_property data;

		if (copy_from_user(&data, (void __user *)arg, sizeof(data)))
			return -EFAULT;

		pr_debug("ION_IOC_CUSTOM_SET_PROP client(%p) handle(%p)"
			   " value(%x) mask(%x)\n", client, data.handle,
			   data.value, data.mask);
		if (kona_ion_set_prop(client, &data))
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
		if (kona_ion_get_prop(client, &data))
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
		if (kona_ion_update_count(client, handle))
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
		if (kona_ion_get_update_count(client, &data))
			return -EINVAL;

		if (copy_to_user((void __user *)arg, &data, sizeof(data)))
			return -EFAULT;
		break;
	}
	case ION_IOC_CUSTOM_CACHE_FLUSH:
	{
		struct ion_custom_region_data data;

		if (copy_from_user(&data, (void __user *)arg, sizeof(data)))
			return -EFAULT;

		pr_debug("ION_IOC_CUSTOM_CACHE_FLUSH client(%p) handle(%p)\n",
				client, data.handle);
		if (kona_ion_cache_flush(client, &data))
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
		if (kona_ion_cache_invalidate(client, &data))
			return -EINVAL;

		if (copy_to_user((void __user *)arg, &data, sizeof(data)))
			return -EFAULT;
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

static u64 ion_dmamask = DMA_BIT_MASK(32);

struct ion_platform_heap *kona_ion_parse_dt(struct device *dev)
{
	struct device_node *node = dev->of_node;
	struct kona_ion_dt_heap_data *heap_init_data;
	struct ion_platform_heap *heap_data = NULL;
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
	heap_data->name = kzalloc(strlen(name), GFP_KERNEL);
	if (!heap_data->name) {
		pr_err("ERROR: Couldn't allocate memory for ion name\n");
		ret = -ENOMEM;
		goto err;
	}
	strcpy((char *)heap_data->name, name);

	ION_OF_READ(type);
	ION_OF_READ(id);
	if ((heap_data->type == ION_HEAP_TYPE_CARVEOUT) ||
			(heap_data->type == ION_HEAP_TYPE_DMA)) {
		if (kona_ion_get_dt_heap_data(&heap_init_data,
					heap_data->name)) {
			pr_err("%16s: Memory was not reserved\n",
					heap_data->name);
			goto of_err;
		}
		if (heap_data->type == ION_HEAP_TYPE_DMA) {
			struct cma *cma;
			dev->dma_mask = &ion_dmamask;
			dev->coherent_dma_mask = DMA_BIT_MASK(32);
			cma = dev_get_cma_area(&heap_init_data->cma_dev);
			dev_set_cma_area(dev, cma);
		}
		heap_data->base = heap_init_data->base;
		heap_data->size = heap_init_data->size;
		ION_OF_READ_OPT(lmk_enable);
		ION_OF_READ_OPT(lmk_min_score_adj);
		ION_OF_READ_OPT(lmk_min_free);
	}
	dev->platform_data = heap_data;
	return heap_data;
of_err:
	kfree(heap_data->name);
err:
	kfree(heap_data);
	return ERR_PTR(ret);
}
#endif /* CONFIG_OF */

static int kona_ion_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	int err;
	int i, j, prev_num_heaps = 0, new_num_heaps;
	struct ion_platform_heap *heap_datas;

	if (!pdev) {
		pr_err("Unable to probe\n");
		return -ENODEV;
	}

	if (dev_get_platdata(dev)) {
		struct ion_platform_data *pdata = dev->platform_data;
		pr_info("Probe: Add (%d)heaps via platform_data\n", pdata->nr);
		new_num_heaps = pdata->nr;
		heap_datas = pdata->heaps;
#ifdef CONFIG_OF
	} else if (dev->of_node) {
		pr_info("Probe: via DT framework\n");
		new_num_heaps = 1;
		heap_datas = kona_ion_parse_dt(dev);
		if (IS_ERR_OR_NULL(heap_datas)) {
			pr_err("Probe Fail: DT parsing failed\n");
			return PTR_ERR(heap_datas);
		}
#endif /* CONFIG_OF */
	} else {
		pr_err("Probe Fail: No platform_data, no DT\n");
		return -EINVAL;
	}

	/* Check for duplicate heap->ids. ION cannot handle it. */
	for (i = 0; i < new_num_heaps; i++) {
		int new_id = heap_datas[i].id;
		for (j = 0; j < num_heaps; j++) {
			int old_id;
			if (!heaps[j])
				continue;
			old_id = heaps[j]->id;
			if (new_id == old_id) {
				pr_err("Probe fail: %16s duplicate id(%d)\n",
						heap_datas[i].name, new_id);
				err = -EINVAL;
				goto err2;
			}
		}
	}

	if (num_heaps == 0) {
		num_heaps = new_num_heaps;
		heaps = kzalloc(sizeof(struct ion_heap *) * new_num_heaps,
				GFP_KERNEL);
		if (!heaps) {
			err = -ENOMEM;
			goto err2;
		}
		idev = ion_device_create(kona_ion_custom_ioctl);
		if (IS_ERR_OR_NULL(idev)) {
			pr_err("Probe Fail: ION device creation failed\n");
			kfree(heaps);
			err = PTR_ERR(idev);
			goto err2;
		}
	} else {
		struct ion_heap **tmp_heaps;

		prev_num_heaps = num_heaps;
		num_heaps += new_num_heaps;
		/* Reallocate ion_heap array */
		tmp_heaps = kzalloc(sizeof(struct ion_heap *) * num_heaps,
				GFP_KERNEL);
		if (!tmp_heaps) {
			err = -ENOMEM;
			goto err2;
		}
		/* Copy old heap array into reallocated buffer and
		 * free old one */
		for (i = 0; i < prev_num_heaps; i++)
			tmp_heaps[i] = heaps[i];
		kfree(heaps);
		/* Set the reallocated buffer as new heap array */
		heaps = tmp_heaps;
	}

	/* create the heaps as specified in the board file */
	for (i = prev_num_heaps; i < num_heaps; i++) {
		struct ion_platform_heap *heap_data =
			&heap_datas[i - prev_num_heaps];

		if (heap_data->id != ION_INVALID_HEAP_ID) {
			pr_info("Heap[%d]: Name(%10s) Type(%d) Id(%d)",
					i, heap_data->name, heap_data->type,
					heap_data->id);
			pr_info("Base(0x%08x) Size (0x%08x)\n",
					(unsigned int)heap_data->base,
					heap_data->size);
			heaps[i] = ion_heap_create_full(heap_data, dev);
			if (IS_ERR_OR_NULL(heaps[i])) {
				heaps[i] = NULL;
				err = PTR_ERR(heaps[i]);
				goto err;
			}
			ion_device_add_heap(idev, heaps[i]);
		}
	}
	platform_set_drvdata(pdev, idev);
	return 0;
err:
	for (i = 0; i < num_heaps; i++) {
		if (heaps[i])
			ion_heap_destroy(heaps[i]);
	}
	kfree(heaps);
err2:
#ifdef CONFIG_OF
	if (dev->of_node) {
		struct ion_platform_heap *heap_data = dev->platform_data;
		if (heap_data) {
			kfree(heap_data->name);
			kfree(heap_data);
			dev->platform_data = NULL;
		}
	}
#endif /* CONFIG_OF */
	return err;
}

static int kona_ion_remove(struct platform_device *pdev)
{
	struct ion_device *idev = platform_get_drvdata(pdev);
	int i;
#ifdef CONFIG_OF
	struct device *dev = &pdev->dev;
	struct ion_platform_heap *heap_data = dev->platform_data;
#endif /* CONFIG_OF */

	pr_info("Kona ION device remove\n");
	ion_device_destroy(idev);
	idev = NULL;
	for (i = 0; i < num_heaps; i++)
		if (heaps[i])
			ion_heap_destroy(heaps[i]);
	kfree(heaps);
#ifdef CONFIG_OF
	if (dev->of_node)
		if (heap_data) {
			kfree(heap_data->name);
			kfree(heap_data);
			dev->platform_data = NULL;
		}
#endif /* CONFIG_OF */
	heaps = NULL;
	num_heaps = 0;
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
	.probe = kona_ion_probe,
	.remove = __devexit_p(kona_ion_remove),
	.driver = {
		.name = "ion-kona",
		.owner = THIS_MODULE,
		.of_match_table = ion_of_match,
	},
};

static int __init ion_init(void)
{
	pr_info("Kona ION driver init\n");
	return platform_driver_register(&ion_driver);
}

static void __exit ion_exit(void)
{
	pr_info("Kona ION driver exit\n");
	platform_driver_unregister(&ion_driver);
}

subsys_initcall(ion_init);
module_exit(ion_exit);

