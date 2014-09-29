/* Copyright (c) 2012-2013, The Linux Foundation. All rights reserved.
 * Copyright (c) 2013 Sony Mobile Communications AB.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include <asm/cacheflush.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/init.h>
#include <linux/export.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/bootmem.h>
#include <linux/memblock.h>
#include <mach/msm_iomap.h>
#include <mach/msm_memory_dump.h>


/*TODO: Needs to be set to correct value */
#define DUMP_TABLE_OFFSET	0x14
#define MSM_DUMP_TABLE_VERSION	MK_TABLE(1, 0)
#define DUMP_TABLE_OFFSET1	0x784
#define DUMP_TABLE_MAGIC	0x5D1DB1BF
#define MSM_CPU_CTXT_MAGIC	0x44434151

static struct msm_memory_dump mem_dump_data;
static void *last_regs_base;
static char *last_regs_buf;
static size_t last_regs_size;

int msm_dump_table_register(struct msm_client_dump *client_entry)
{
	struct msm_client_dump *entry;
	struct msm_dump_table *table = mem_dump_data.dump_table_ptr;

	if (!table || table->num_entries >= MAX_NUM_CLIENTS)
		return -EINVAL;
	entry = &table->client_entries[table->num_entries];
	entry->id = client_entry->id;
	entry->start_addr = client_entry->start_addr;
	entry->end_addr = client_entry->end_addr;
	table->num_entries++;
	/* flush cache */
	dmac_flush_range(table, table + sizeof(struct msm_dump_table));
	return 0;
}
EXPORT_SYMBOL(msm_dump_table_register);

static ssize_t last_regs_read(struct file *file, char __user *buf,
				    size_t len, loff_t *offset)
{
	loff_t pos = *offset;
	ssize_t count;

	if (pos >= last_regs_size)
		return 0;

	count = min(len, (size_t)(last_regs_size - pos));
	if (copy_to_user(buf, last_regs_buf + pos, count))
		return -EFAULT;

	*offset += count;
	return count;
}

static const struct file_operations last_regs_fops = {
	.owner = THIS_MODULE,
	.read = last_regs_read,
};

static int msm_export_last_regs(void)
{
	struct proc_dir_entry *proc_entry;

	writel_relaxed(mem_dump_data.dump_table_phys,
				MSM_IMEM_BASE + DUMP_TABLE_OFFSET1);
	if (!last_regs_base || !last_regs_size)
		return -EINVAL;

	last_regs_buf = kmalloc(last_regs_size, GFP_KERNEL);
	if (!last_regs_buf) {
		printk(KERN_ERR "%s: failed to allocate last_regs_buf\n",
			__func__);
		return -ENOMEM;
	}

	memcpy(last_regs_buf, last_regs_base, last_regs_size);
	memblock_free(virt_to_phys(last_regs_base), last_regs_size);
	proc_entry = create_proc_entry("last_regs",
					  S_IFREG | S_IRUGO, NULL);
	if (!proc_entry) {
		printk(KERN_ERR "%s: failed to create proc entry\n", __func__);
		kfree(last_regs_buf);
		last_regs_buf = NULL;
		return -ENOMEM;
	}
	proc_entry->proc_fops = &last_regs_fops;
	proc_entry->size = last_regs_size;

	return 0;
}

void msm_reserve_last_regs(void)
{
	struct msm_dump_table *old_table;
	struct msm_client_dump *dump_entry;
	unsigned long dump_table_phys;
	int i, ret, found = 0;

	dump_table_phys = readl_relaxed(MSM_IMEM_BASE + DUMP_TABLE_OFFSET1);
	if (!pfn_valid(__phys_to_pfn(dump_table_phys)) ||
		(__phys_to_pfn(dump_table_phys) > max_low_pfn)) {
		printk(KERN_INFO "%s: Dump table not in lowmem range: 0x%lx\n",
			__func__, dump_table_phys);
		goto err0;
	}

	ret = memblock_reserve(dump_table_phys, sizeof(struct msm_dump_table));
	if (ret) {
		printk(KERN_ERR "%s: Failed to reserve dump table area\n",
			__func__);
		goto err0;
	}

	old_table = phys_to_virt(dump_table_phys);
	if (old_table->version == MSM_DUMP_TABLE_VERSION ||
		old_table->version == DUMP_TABLE_MAGIC) {
		printk(KERN_INFO "%s: Dump table magic found!\n", __func__);
	} else {
		printk(KERN_INFO "%s: Invalid Dump table magic: 0x%x\n",
			__func__, old_table->version);
		goto err1;
	}

	dump_entry = &old_table->client_entries[0];
	for (i = 0; i < old_table->num_entries; i++) {
		if (dump_entry->id == MSM_CPU_CTXT) {
			found = 1;
			break;
		}
		dump_entry++;
	}

	if (!found) {
		printk(KERN_INFO "%s: MSM_CPU_CTXT not found\n", __func__);
		goto err1;
	}

	if (!pfn_valid(__phys_to_pfn(dump_entry->start_addr)) ||
		(__phys_to_pfn(dump_entry->start_addr) > max_low_pfn)) {
		printk(KERN_ERR "%s: last_regs not in lowmem range: 0x%lx\n",
			__func__, dump_entry->start_addr);
		goto err1;
	}

	ret = memblock_reserve(dump_entry->start_addr,
			dump_entry->end_addr - dump_entry->start_addr);
	if (ret) {
		printk(KERN_ERR "%s: Failed to reserve last_regs area\n",
			__func__);
		goto err1;
	}

	last_regs_base = phys_to_virt(dump_entry->start_addr);
	last_regs_size = dump_entry->end_addr - dump_entry->start_addr;

	printk(KERN_INFO "%s: last_regs_base=0x%x, size=%d\n",
		__func__, (unsigned int)last_regs_base, last_regs_size);

	if (readl_relaxed(last_regs_base) == MSM_CPU_CTXT_MAGIC) {
		printk(KERN_INFO "%s: HWWD context found!\n", __func__);
	} else {
		printk(KERN_INFO "%s: HWWD context not found\n", __func__);
		memblock_free(dump_entry->start_addr, last_regs_size);
		last_regs_base = NULL;
		last_regs_size = 0;
	}

err1:
	memblock_free(dump_table_phys, sizeof(struct msm_dump_table));
err0:
	return;
}
EXPORT_SYMBOL(msm_reserve_last_regs);

static int __init init_memory_dump(void)
{
	struct msm_dump_table *table;

	mem_dump_data.dump_table_ptr = kzalloc(sizeof(struct msm_dump_table),
						GFP_KERNEL);
	if (!mem_dump_data.dump_table_ptr) {
		printk(KERN_ERR "unable to allocate memory for dump table\n");
		return -ENOMEM;
	}
	table = mem_dump_data.dump_table_ptr;
	table->version = MSM_DUMP_TABLE_VERSION;
	mem_dump_data.dump_table_phys = virt_to_phys(table);
	writel_relaxed(mem_dump_data.dump_table_phys,
				MSM_IMEM_BASE + DUMP_TABLE_OFFSET);
	if (!msm_export_last_regs())
		printk(KERN_INFO "exported HWWD context as /proc/last_regs\n");
	printk(KERN_INFO "MSM Memory Dump table set up\n");
	return 0;
}

early_initcall(init_memory_dump);

