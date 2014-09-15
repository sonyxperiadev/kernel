/* Copyright (c) 2013, Sony Mobile Communications AB. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/seq_file.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/math64.h>
#include <linux/errno.h>
#include <mach/msm_mpmctr.h>

#define MPM_SCLK_HZ   (32768)

static s64 get_sclk_time(void)
{
	u32 ticks = msm_mpm_get_count();
	if (!ticks)
		return 0;

	return div_s64((s64)ticks * NSEC_PER_SEC, MPM_SCLK_HZ);
}

static int show(struct seq_file *f, void *data)
{
	s64 secs;
	s32 nanos;
	s64 mpm_nanos = get_sclk_time();

	if (mpm_nanos <= 0) {
		pr_err("mpmtime: get failed (%lld)\n", (long long)mpm_nanos);
		return -EIO;
	}

	secs = div_s64_rem(mpm_nanos, NSEC_PER_SEC, &nanos);
	seq_printf(f, "%lld.%02ld\n",
			(long long) secs,
			(nanos / (NSEC_PER_SEC/100)));
	return 0;
}

static int open(struct inode *inode, struct file *f)
{
	return single_open(f, show, NULL);
}

static const struct file_operations fops = {
	.open		= open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int __init init(void)
{
	proc_create("mpmtime", 0, NULL, &fops);
	return 0;
}

module_init(init);
MODULE_LICENSE("GPL");
