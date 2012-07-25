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

static struct ion_device *idev;
static int num_heaps;
static struct ion_heap **heaps;

void kona_ion_map_dma(struct ion_client *client, struct ion_custom_dma_map_data *data)
{
	struct scatterlist *sglist;

	sglist = ion_map_dma(client, data->handle);
	data->dma_addr = 0;
#if 1
	/* NISH_TODO: Move to heap implementation and use sg_dma */
	/* Do not have IOMMU to map multiple scatterlist entries to contiguous dma address */
	if (!IS_ERR_OR_NULL(sglist) && sg_is_last(sglist))
		data->dma_addr = sg_phys(sglist);
#endif
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
		kona_ion_map_dma(client, &data);

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

module_init(ion_init);
module_exit(ion_exit);

