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

#define pr_fmt(fmt) "android-ion: " fmt

#include <linux/err.h>
#include <linux/scatterlist.h>
#include <linux/ion.h>
#include <linux/broadcom/kona_ion.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include "../ion_priv.h"
#include <linux/uaccess.h>
#include <linux/mutex.h>
#ifdef CONFIG_M4U
#include <linux/broadcom/m4u.h>
#endif

/* NISHANTH_TODO: The following two structures should ideally be defined
 * in ion_priv.h for custom ioctls to use them.
 * Duplicating it here to minimise modifications to mainline ion code.
 * These would be removed when mainline moves them to header file.
 */
/**
 * struct ion_client - a process/hw block local address space
 * @ref:		for reference counting the client
 * @node:		node in the tree of all clients
 * @dev:		backpointer to ion device
 * @handles:		an rb tree of all the handles in this client
 * @lock:		lock protecting the tree of handles
 * @heap_mask:		mask of all supported heaps
 * @name:		used for debugging
 * @task:		used for debugging
 *
 * A client represents a list of buffers this client may access.
 * The mutex stored here is used to protect both handles tree
 * as well as the handles themselves, and should be held while modifying either.
 */
struct ion_client {
	struct kref ref;
	struct rb_node node;
	struct ion_device *dev;
	struct rb_root handles;
	struct mutex lock;
	unsigned int heap_mask;
	const char *name;
	struct task_struct *task;
	pid_t pid;
	struct dentry *debug_root;
};

/**
 * ion_handle - a client local reference to a buffer
 * @ref:		reference count
 * @client:		back pointer to the client the buffer resides in
 * @buffer:		pointer to the buffer
 * @node:		node in the client's handle rbtree
 * @kmap_cnt:		count of times this client has mapped to kernel
 * @dmap_cnt:		count of times this client has mapped for dma
 * @usermap_cnt:	count of times this client has mapped for userspace
 *
 * Modifications to node, map_cnt or mapping should be protected by the
 * lock in the client.  Other fields are never changed after initialization.
 */
struct ion_handle {
	struct kref ref;
	struct ion_client *client;
	struct ion_buffer *buffer;
	struct rb_node node;
	unsigned int kmap_cnt;
	unsigned int dmap_cnt;
	unsigned int usermap_cnt;
};

static bool ion_handle_validate(struct ion_client *client, struct ion_handle *handle)
{
	struct rb_node *n = client->handles.rb_node;

	while (n) {
		struct ion_handle *handle_node = rb_entry(n, struct ion_handle,
							  node);
		if (handle < handle_node)
			n = n->rb_left;
		else if (handle > handle_node)
			n = n->rb_right;
		else
			return true;
	}
	return false;
}

struct ion_device *idev;
static int num_heaps;
static struct ion_heap **heaps;

static struct ion_buffer* kona_ion_acquire_buffer(struct ion_client *client, struct ion_handle *handle)
{
	struct ion_buffer *buffer = NULL;

	mutex_lock(&client->lock);
	if (!ion_handle_validate(client, handle)) {
		pr_err("Invalid handle passed to custom ioctl.\n");
		mutex_unlock(&client->lock);
		return NULL;
	}
	buffer = handle->buffer;
	mutex_lock(&buffer->lock);
	return buffer;
}

static void kona_ion_release_buffer(struct ion_client *client, struct ion_buffer *buffer)
{
	mutex_unlock(&buffer->lock);
	mutex_unlock(&client->lock);
}

unsigned int kona_ion_map_dma(struct ion_client *client, struct ion_handle *handle)
{
	struct ion_buffer *buffer;
	unsigned int dma_addr = 0;
	struct sg_table *sg_table;

	sg_table = ion_map_dma(client, handle);
	if (IS_ERR_OR_NULL(sg_table))
		return dma_addr;

#ifdef CONFIG_M4U
	buffer = kona_ion_acquire_buffer(client, handle);
	if (!buffer->dma_addr) {
		buffer->dma_addr = m4u_map(g_mdev, sg_table, buffer->size, buffer->align);
		if (buffer->dma_addr == INVALID_MMA) {
			buffer->dma_addr = 0;
		}
	}
	dma_addr = buffer->dma_addr;
	kona_ion_release_buffer(client, buffer);
#else
	/* Do not have IOMMU to map multiple scatterlist entries to contiguous dma address */
	if (sg_table->nents == 1)
		dma_addr = sg_phys(sg_table->sgl);
#endif

	if (!dma_addr)
		ion_unmap_dma(client, handle);
	return dma_addr;
}
EXPORT_SYMBOL(kona_ion_map_dma);

static int kona_ion_set_prop(struct ion_client *client, struct ion_custom_property *data)
{
	struct ion_buffer *buffer;

	buffer = kona_ion_acquire_buffer(client, data->handle);
	if (buffer) {
		buffer->custom_flags = (data->value & data->mask);
		kona_ion_release_buffer(client, buffer);
		return 0;
	}
	return -EINVAL;
}

static int kona_ion_get_prop(struct ion_client *client, struct ion_custom_property *data)
{
	struct ion_buffer *buffer;

	buffer = kona_ion_acquire_buffer(client, data->handle);
	if (buffer) {
		data->value = (buffer->custom_flags & data->mask);
		kona_ion_release_buffer(client, buffer);
		return 0;
	}
	return -EINVAL;
}

static int kona_ion_update_count(struct ion_client *client, struct ion_handle *handle)
{
	struct ion_buffer *buffer;

	buffer = kona_ion_acquire_buffer(client, handle);
	if (buffer) {
		buffer->custom_update_count++;
		kona_ion_release_buffer(client, buffer);
		return 0;
	}
	return -EINVAL;
}

static int kona_ion_get_update_count(struct ion_client *client, struct ion_custom_update_count *data)
{
	struct ion_buffer *buffer;

	buffer = kona_ion_acquire_buffer(client, data->handle);
	if (buffer) {
		data->count = buffer->custom_update_count;
		kona_ion_release_buffer(client, buffer);
		return 0;
	}
	return -EINVAL;
}

static long kona_ion_custom_ioctl (struct ion_client *client,
				      unsigned int cmd,
				      unsigned long arg)
{
	switch (cmd) {
	case ION_IOC_CUSTOM_DMA_MAP:
	{
		struct ion_custom_dma_map_data data;

		if (copy_from_user(&data, (void __user *)arg, sizeof(data)))
			return -EFAULT;

		pr_debug("ION_IOC_CUSTOM_DMA_MAP client(%p) handle(%p) \n",
				client, data.handle);
		data.dma_addr = kona_ion_map_dma(client, data.handle);

		if (copy_to_user((void __user *)arg, &data, sizeof(data)))
			return -EFAULT;
		break;
	}
	case ION_IOC_CUSTOM_DMA_UNMAP:
	{
		struct ion_handle *handle = (struct ion_handle *)arg;

		pr_debug("ION_IOC_CUSTOM_DMA_UNMAP client(%p) handle(%p) \n",
				client, handle);
		ion_unmap_dma(client, handle);

		break;
	}
	case ION_IOC_CUSTOM_SET_PROP:
	{
		struct ion_custom_property data;

		if (copy_from_user(&data, (void __user *)arg, sizeof(data)))
			return -EFAULT;

		pr_debug("ION_IOC_CUSTOM_SET_PROP client(%p) handle(%p) value(%x) mask(%x)\n",
				client, data.handle, data.value, data.mask);
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
		struct ion_handle *handle = (struct ion_handle*)arg;

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
	case ION_IOC_CUSTOM_TP:
	{
		pr_debug(" TP(%ld) \n", arg);
		break;
	}
	default:
		pr_err(" unsupported custom ioctl(%d) \n", cmd);
		return -ENOTTY;
	}
	return 0;
}

static int kona_ion_probe(struct platform_device *pdev)
{
	struct ion_platform_data *pdata;
	int err;
	int i;

	if (!pdev || !pdev->dev.platform_data) {
		pr_err("Unable to probe \n");
		return -ENODEV;
	}

	pdata = pdev->dev.platform_data;
	pr_info("Max heaps(%d) \n", pdata->nr);
	num_heaps = pdata->nr;

	heaps = kzalloc(sizeof(struct ion_heap *) * pdata->nr, GFP_KERNEL);

	idev = ion_device_create(kona_ion_custom_ioctl);
	if (IS_ERR_OR_NULL(idev)) {
		kfree(heaps);
		return PTR_ERR(idev);
	}

	/* create the heaps as specified in the board file */
	for (i = 0; i < num_heaps; i++) {
		struct ion_platform_heap *heap_data = &pdata->heaps[i];

		if (heap_data->id != ION_INVALID_HEAP_ID) {
			pr_info("Heap[%d]: Name(%10s) Type(%d) Id(%d) Base(0x%08x) Size (0x%08x)\n",
					i, heap_data->name, heap_data->type, heap_data->id,
					(unsigned int)heap_data->base, heap_data->size);
			heaps[i] = ion_heap_create(heap_data);
			if (IS_ERR_OR_NULL(heaps[i])) {
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

	pr_info("Kona ION device remove \n");
	ion_device_destroy(idev);
	for (i = 0; i < num_heaps; i++)
		if (heaps[i])
			ion_heap_destroy(heaps[i]);
	kfree(heaps);
	return 0;
}

static struct platform_driver ion_driver = {
	.probe = kona_ion_probe,
	.remove = kona_ion_remove,
	.driver = { .name = "ion-kona" }
};

static int __init ion_init(void)
{
	pr_info("Kona ION driver init \n");
	return platform_driver_register(&ion_driver);
}

static void __exit ion_exit(void)
{
	pr_info("Kona ION driver exit \n");
	platform_driver_unregister(&ion_driver);
}

subsys_initcall(ion_init);
module_exit(ion_exit);

