/*******************************************************************************
Copyright 2012 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement
governing use of this software, this software is licensed to you under the
terms of the GNU General Public License version 2, available at
http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software
in any way with any other Broadcom software provided under a license other than
the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <mach/cache-l2x0.h>

#define PROC_DIR              "bcm_mmutil"
#define PROC_L2X0_PREFETCH    "l2x0_prefetch"

struct bcm_mmutil {
	struct miscdevice dev;
	struct proc_dir_entry *procdir;
	struct proc_dir_entry *procentry;
};

static struct bcm_mmutil mmutil;

static int bcm_mmutil_write_proc(struct file *file,
		const char __user *buffer,
		unsigned long count, void *data)
{
	int e;
	unsigned long n;

	(void)file;
	(void)data;

	e = kstrtoul_from_user(buffer, count, 0, &n);
	if (e)
		return e;
	if (n)
		rhea_l2x0_prefetch(1);
	else
		rhea_l2x0_prefetch(0);

	return count;
}

static int bcm_mmutil_open(struct inode *inode, struct file *file)
{
	(void)inode;
	(void)file;

	rhea_l2x0_prefetch(1);
	return 0;
}

static int bcm_mmutil_release(struct inode *inode, struct file *file)
{
	(void)inode;
	(void)file;

	rhea_l2x0_prefetch(0);
	return 0;
}

static long bcm_mmutil_ioctl(struct file *file, unsigned cmd, unsigned long arg)
{
	int ret = 0;

	switch (cmd) {
	default:
		pr_info("%s: unknown ioctl: cmd = 0x%08x", __func__, cmd);
		ret = -ENOIOCTLCMD;
	}

	return ret;
}

static const struct file_operations bcm_mmutil_fops = {
	.owner          = THIS_MODULE,
	.open           = bcm_mmutil_open,
	.release        = bcm_mmutil_release,
	.unlocked_ioctl = bcm_mmutil_ioctl,
};

int __init bcm_mmutil_module_init(void)
{
	int ret;

	pr_debug("bcm_mmutil driver init\n");

	mmutil.dev.name = "bcm_mmutil";
	mmutil.dev.minor = MISC_DYNAMIC_MINOR;
	mmutil.dev.fops = &bcm_mmutil_fops;

	ret = misc_register(&mmutil.dev);
	if (ret) {
		pr_err("failed to register driver %d", ret);
		goto err_register;
	}

	mmutil.procdir = proc_mkdir(PROC_DIR, NULL);
	if (mmutil.procdir == NULL) {
		pr_err("failed to create proc dir\n");
		ret = -ENOENT;
		goto err_proc_mkdir;
	}

	mmutil.procentry = create_proc_entry(PROC_L2X0_PREFETCH,
			(S_IRUSR | S_IRGRP),
			mmutil.procdir);
	if (mmutil.procentry == NULL) {
		pr_err("failed to create proc entry\n");
		ret = -ENOENT;
		goto err_proc_create_entry;
	}

	mmutil.procentry->write_proc = bcm_mmutil_write_proc;

	return 0;

err_proc_create_entry:
	remove_proc_entry(PROC_DIR, NULL);
err_proc_mkdir:
	misc_deregister(&mmutil.dev);
err_register:
	return ret;
}

void __exit bcm_mmutil_module_term(void)
{
	pr_debug("bcm_mmutil driver exit\n");

	remove_proc_entry(PROC_L2X0_PREFETCH, mmutil.procdir);
	remove_proc_entry(PROC_DIR, NULL);

	misc_deregister(&mmutil.dev);
}

module_init(bcm_mmutil_module_init);
module_exit(bcm_mmutil_module_term);

MODULE_AUTHOR("Broadcom Corporation");
MODULE_DESCRIPTION("bcm_mmutil driver");
MODULE_LICENSE("GPL");
