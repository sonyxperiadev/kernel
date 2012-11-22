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
#ifdef CONFIG_M4U
#include <linux/broadcom/m4u.h>
#endif

struct ion_device *idev;
static int num_heaps;
static struct ion_heap **heaps;

unsigned int kona_ion_map_dma(struct ion_client *client,
		struct ion_handle *handle)
{
#ifdef CONFIG_M4U
	struct ion_buffer *buffer;
#endif
	unsigned int dma_addr = 0;
	struct sg_table *sg_table;

	sg_table = ion_sg_table(client, handle);
	if (IS_ERR_OR_NULL(sg_table))
		return dma_addr;

#ifdef CONFIG_M4U
	buffer = ion_lock_buffer(client, handle);
	if (!buffer)
		return dma_addr;
	dma_addr = buffer->dma_addr;
	ion_unlock_buffer(client, buffer);
#else
	/* Do not have IOMMU to map multiple scatterlist entries to
	 * contiguous dma address. With M4U disabled, this should be called
	 * only for contiguous buffer.
	 * TODO: Add check for contiguous buffer */
	dma_addr = sg_phys(sg_table->sgl);
#endif

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

static int kona_ion_probe(struct platform_device *pdev)
{
	struct ion_platform_data *pdata;
	int err;
	int i, prev_num_heaps = 0;

	if (!pdev || !pdev->dev.platform_data) {
		pr_err("Unable to probe\n");
		return -ENODEV;
	}

	pdata = pdev->dev.platform_data;
	pr_info("Probe: add (%d)heaps\n", pdata->nr);
	if (num_heaps == 0) {
		num_heaps = pdata->nr;
		heaps = kzalloc(sizeof(struct ion_heap *) * pdata->nr,
				GFP_KERNEL);
		idev = ion_device_create(kona_ion_custom_ioctl);
		if (IS_ERR_OR_NULL(idev)) {
			kfree(heaps);
			return PTR_ERR(idev);
		}
	} else {
		struct ion_heap **tmp_heaps;

		prev_num_heaps = num_heaps;
		num_heaps += pdata->nr;
		/* Reallocate ion_heap array */
		tmp_heaps = kzalloc(sizeof(struct ion_heap *) * num_heaps,
				GFP_KERNEL);
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
			&pdata->heaps[i - prev_num_heaps];

		if (heap_data->id != ION_INVALID_HEAP_ID) {
			pr_info("Heap[%d]: Name(%10s) Type(%d) Id(%d)",
					i, heap_data->name, heap_data->type,
					heap_data->id);
			pr_info("Base(0x%08x) Size (0x%08x)\n",
					(unsigned int)heap_data->base,
					heap_data->size);
			heaps[i] = ion_heap_create_full(heap_data, &pdev->dev);
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
	return err;
}

static int kona_ion_remove(struct platform_device *pdev)
{
	struct ion_device *idev = platform_get_drvdata(pdev);
	int i;

	pr_info("Kona ION device remove\n");
	ion_device_destroy(idev);
	idev = NULL;
	for (i = 0; i < num_heaps; i++)
		if (heaps[i])
			ion_heap_destroy(heaps[i]);
	kfree(heaps);
	heaps = NULL;
	num_heaps = 0;
	return 0;
}

static struct platform_driver ion_driver = {
	.probe = kona_ion_probe,
	.remove = kona_ion_remove,
	.driver = { .name = "ion-kona" }
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

