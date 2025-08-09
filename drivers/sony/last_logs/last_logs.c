/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */
/*
 * NOTE: This file has been modified by Sony Mobile Communications Inc.
 * Modifications are Copyright (c) 2017 Sony Mobile Communications Inc,
 * and licensed under the license of the file.
 */

#include <linux/proc_fs.h>
#include <linux/errno.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/last_logs.h>

static void __iomem *last_logs_virt_iobase;
static unsigned long last_logs_size;

struct proc_dir_entry *procfs_entry;

static ssize_t last_logs_read(struct file *file, char __user *buf,
	size_t len, loff_t *offp)
{
	struct last_logs_data *priv_data =
			(struct last_logs_data *)file->private_data;

	return simple_read_from_buffer(buf, len, offp,
			priv_data->addr, priv_data->size);
}

static int last_logs_open(struct inode *inode, struct file *file)
{
	file->private_data = PDE_DATA(inode);
	return 0;
}

static const struct proc_ops last_logs_ops = {
	.proc_read = last_logs_read,
	.proc_open = last_logs_open,
};

static int last_logs_init_resource(struct platform_device *pdev,
				last_logs_region *region)
{
	struct last_logs_data *priv_data;
	size_t debug_resource_size;
	int ret = -1;
	void __iomem *last_virt_iobase;
	void *last_logs_addr;

	debug_resource_size = region->size;

	dev_info(&pdev->dev, "last_log driver initialized %u@%x\n",
			(unsigned int)debug_resource_size, region->size);

	last_virt_iobase = last_logs_virt_iobase + region->offset;
	last_logs_addr = kzalloc(debug_resource_size, GFP_KERNEL);
	if (!last_logs_addr) {
		dev_err(&pdev->dev, "ERROR: %s could not allocate memory",
			__func__);
		iounmap(last_virt_iobase);
		return -ENOMEM;
	}

	memcpy_fromio(last_logs_addr, last_virt_iobase, debug_resource_size);
	/* clear & unmap last_logs debug memory */
	memset_io(last_virt_iobase, 0, debug_resource_size);

	priv_data = kzalloc(sizeof(struct last_logs_data), GFP_KERNEL);
	if (!priv_data) {
		dev_err(&pdev->dev, "ERROR: %s could not allocate memory",
			__func__);
		kfree(last_logs_addr);
		return -ENOMEM;
	}

	priv_data->addr = last_logs_addr;
	priv_data->size = debug_resource_size;

	if (procfs_entry) {
		priv_data->procfs_file = proc_create_data(region->name,
			S_IFREG | S_IRUGO, procfs_entry, &last_logs_ops,
				priv_data);
		if (!priv_data->procfs_file) {
			dev_err(&pdev->dev,
				"%s: Failed to create debug file entry %s\n",
				__func__, region->name);
			ret = -EINVAL;
			goto exit;
		}
	}

	return 0;

exit:
	kfree(last_logs_addr);
	kfree(priv_data);
	return ret;
}

static int last_logs_probe(struct platform_device *pdev)
{
	int i;
	last_logs_header last_logs_hdr;
	last_logs_region region;
	struct device_node *node;
	struct resource r;
	int ret;

	node = of_parse_phandle(pdev->dev.of_node, "memory-region", 0);
	if (!node) {
		dev_err(&pdev->dev, "no memory-region specified\n");
		return -EINVAL;
	}

	ret = of_address_to_resource(node, 0, &r);
	if (ret)
		return ret;

	last_logs_size = resource_size(&r);
	last_logs_virt_iobase = ioremap(r.start, last_logs_size);
	if (!last_logs_virt_iobase) {
		dev_err(&pdev->dev, "Failed to ioremap\n");
		return -EINVAL;
	}

	memcpy_fromio((void *)&last_logs_hdr, last_logs_virt_iobase,
			LAST_LOGS_HEADER_SIZE);
	if (last_logs_hdr.version != LAST_LOGS_VERSION ||
		last_logs_hdr.magic != LAST_LOGS_MAGIC) {
		dev_err(&pdev->dev, "ERROR: %s: magic = %x, version = 0x%x\n",
			__func__, last_logs_hdr.magic,
			last_logs_hdr.version);
		goto exit;
	}

	dev_info(&pdev->dev, "%s: magic = %x, version = 0x%x\n",
		__func__, last_logs_hdr.magic,
		last_logs_hdr.version);

	/* Copy & Erase the header for next use */
	memset_io(last_logs_virt_iobase, 0, LAST_LOGS_HEADER_SIZE);

	if (last_logs_hdr.num_regions > MAX_LAST_LOGS_REGIONS)
		goto exit;

	if (!procfs_entry) {
		procfs_entry = proc_mkdir("last_logs", NULL);
		if (!procfs_entry) {
			dev_err(&pdev->dev,
				"%s: Failed to create last_logs dir\n",
				__func__);
			goto exit;
		}
	}

	for (i = 0; i < last_logs_hdr.num_regions; i++) {
		memcpy_fromio((void *)&region, &last_logs_hdr.regions[i],
					sizeof(last_logs_region));
		if (!region.name[0]) {
			dev_err(&pdev->dev, "ERROR device name is invalid");
			continue;
		}

		if (last_logs_init_resource(pdev, &region) != 0)
			dev_err(&pdev->dev, "ERROR: %s last_logs_init",
				region.name);
	}

exit:
	iounmap(last_logs_virt_iobase);
	return 0;
}

static const struct of_device_id last_log_match_table[] = {
	{.compatible = "rd_last_logs", },
	{},
};

static struct platform_driver last_logs_driver = {
	.probe		= last_logs_probe,
	.driver		= {
		.name = "last_logs",
		.owner = THIS_MODULE,
		.of_match_table = last_log_match_table,
	},
};

static int __init last_logs_init(void)
{
	return platform_driver_register(&last_logs_driver);
}

static void last_logs_exit(void)
{
	platform_driver_unregister(&last_logs_driver);
	return;
}

late_initcall(last_logs_init);
module_exit(last_logs_exit);
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("lmk_last_logs driver");
