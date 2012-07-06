/* drivers/misc/ramdumplogs.c
 *
 * Copyright (C) 2011 Sony Ericsson Mobile Communications Japan.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include "../../arch/arm/mach-msm/smd_private.h"

struct ramdump_log {
	uint32_t    sig;
	uint32_t    size;
	uint8_t     data[0];
};

#define RAMDUMPINFO_SIG 0x42972468
#define AMSSLOG_SIG 0x12095015

static char *ramdumpinfo;
static size_t ramdumpinfo_size;

static ssize_t ramdumpinfo_read(struct file *file, char __user *buf,
				    size_t len, loff_t *offset)
{
	loff_t pos = *offset;
	ssize_t count;

	if (pos >= ramdumpinfo_size)
		return 0;

	count = min(len, (size_t)(ramdumpinfo_size - pos));
	if (copy_to_user(buf, ramdumpinfo + pos, count))
		return -EFAULT;

	*offset += count;
	return count;
}

static const struct file_operations ramdumpinfo_file_ops = {
	.owner = THIS_MODULE,
	.read = ramdumpinfo_read,
};

static char *amsslog;
static size_t amsslog_size;
static void *amsslog_backup_addr;
static size_t amsslog_backup_size;

static ssize_t amsslog_read(struct file *file, char __user *buf,
				    size_t len, loff_t *offset)
{
	loff_t pos = *offset;
	ssize_t count;

	if (pos >= amsslog_size)
		return 0;

	count = min(len, (size_t)(amsslog_size - pos));
	if (copy_to_user(buf, amsslog + pos, count))
		return -EFAULT;

	*offset += count;
	return count;
}

static const struct file_operations amsslog_file_ops = {
	.owner = THIS_MODULE,
	.read = amsslog_read,
};

static int ramdumplog_init(struct resource *res)
{
	struct proc_dir_entry *entry;
	struct ramdump_log *buffer;
	size_t size;
	int ret = 0;

	if (res == NULL)
		return -ENODEV;

	size = res->end - res->start + 1;
	buffer = (struct ramdump_log *)ioremap(res->start, size);
	if (buffer == NULL) {
		printk(KERN_ERR "ramdumplog: failed to map memory\n");
		return -ENOMEM;
	}
	if (buffer->sig == RAMDUMPINFO_SIG) {
		printk(KERN_INFO
		       "ramdumpinfo:found at 0x%x\n", (unsigned int)buffer);
		ramdumpinfo = kmalloc(size, GFP_KERNEL);
		if (ramdumpinfo == NULL) {
			printk(KERN_ERR
			       "ramdumpinfo: failed to allocate buffer\n");
			ret = -ENOMEM;
			goto error;
		}
		ramdumpinfo_size = min(buffer->size, size);
		memcpy(ramdumpinfo, buffer->data,
		       size - (buffer->data - (uint8_t *)buffer));
		entry = create_proc_entry("ramdumpinfo",
					   S_IFREG | S_IRUGO, NULL);
		if (!entry) {
			printk(KERN_ERR
			       "ramdumpinfo: failed to create proc entry\n");
			kfree(ramdumpinfo);
			ramdumpinfo = NULL;
			ret = -ENOMEM;
			goto error;
		}
		entry->proc_fops = &ramdumpinfo_file_ops;
		entry->size = ramdumpinfo_size;
	} else if (buffer->sig == AMSSLOG_SIG) {
		printk(KERN_INFO
		       "amsslog:found at 0x%x\n", (unsigned int)buffer);
		amsslog = kmalloc(size, GFP_KERNEL);
		if (amsslog == NULL) {
			printk(KERN_ERR
			       "amsslog: failed to allocate buffer\n");
			ret = -ENOMEM;
			goto error;
		}
		amsslog_size = min(buffer->size, size);
		memcpy(amsslog, buffer->data,
		       size - (buffer->data - (uint8_t *)buffer));
		entry = create_proc_entry("last_amsslog",
					   S_IFREG | S_IRUGO, NULL);
		if (!entry) {
			printk(KERN_ERR
			       "amsslog: failed to create proc entry\n");
			kfree(amsslog);
			amsslog = NULL;
			ret = -ENOMEM;
			goto error;
		}
		entry->proc_fops = &amsslog_file_ops;
		entry->size = amsslog_size;
	}
error:
	memset(buffer, 0, size);
	iounmap(buffer);
	return ret;
}

static int amsslog_backup_handler(struct notifier_block *this,
				unsigned long event, void *ptr)
{
	struct ramdump_log *buffer = amsslog_backup_addr;
	char *smem_errlog = NULL;
	size_t size;

	smem_errlog = smem_get_entry(SMEM_ERR_CRASH_LOG, &size);
	if (smem_errlog && !memcmp(smem_errlog, "ERR", 3)) {
		buffer->sig = AMSSLOG_SIG;
		buffer->size = size - 2 * sizeof(unsigned int);
		if (buffer->size > amsslog_backup_size)
			buffer->size = amsslog_backup_size;

		memcpy(buffer->data, smem_errlog, buffer->size);
	}

	return NOTIFY_DONE;
}


static struct notifier_block panic_amsslog_backup = {
	.notifier_call  = amsslog_backup_handler,
};

static int ramdumplog_driver_probe(struct platform_device *pdev)
{
	struct resource *res;

	res = platform_get_resource_byname(pdev,
					IORESOURCE_MEM, "ramdumpinfo");
	if (!res || !res->start) {
		printk(KERN_ERR "%s: ramdumpinfo resource invalid/absent\n",
				__func__);
		return -ENODEV;
	}
	ramdumplog_init(res);

	res = platform_get_resource_byname(pdev,
					IORESOURCE_MEM, "amsslog");
	if (!res || !res->start) {
		printk(KERN_ERR "%s: amsslog resource invalid/absent\n",
				__func__);
		return -ENODEV;
	}
	ramdumplog_init(res);

	amsslog_backup_size = res->end - res->start + 1;
	amsslog_backup_addr = ioremap_nocache(res->start, amsslog_backup_size);
	if (!amsslog_backup_addr) {
		printk(KERN_ERR "failed to map amsslog_addr\n");
		return -ENOMEM;
	}

	atomic_notifier_chain_register(&panic_notifier_list,
				&panic_amsslog_backup);
	return 0;
}

static struct platform_driver ramdumplog_driver = {
	.probe = ramdumplog_driver_probe,
	.driver		= {
		.name	= "ramdumplog",
	},
};

static int __init ramdumplog_module_init(void)
{
	int err;
	err = platform_driver_register(&ramdumplog_driver);
	return err;
}

MODULE_AUTHOR("Sony Ericsson Mobile Communications Japan");
MODULE_DESCRIPTION("ramdump crash logs");
MODULE_LICENSE("GPL V2");

module_init(ramdumplog_module_init);

