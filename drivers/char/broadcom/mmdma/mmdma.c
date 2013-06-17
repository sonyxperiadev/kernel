/*
 * drivers/char/broadcom/mmdma/mmdma.c
 *
 * Copyright (C) 2012 Broadcom, Inc.
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

#define pr_fmt(fmt) "mmdma: " fmt

#include <linux/fs.h>
#include <linux/init.h>
#include <linux/mutex.h>
#include <asm/uaccess.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/broadcom/mmdma.h>

#define MMDMA_DEV_NAME "mmdma"

extern int mmdma_execute(unsigned int srcAddr, unsigned int dstAddr, unsigned int size);
static int mmdmamajor = 0;
struct class *mmdma_class;

static long mmdma_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	mmdma_params params;

	pr_debug("ioctl \n");
	switch(cmd)	{
	case MMDMA_IOCTL_XFER:
	{
		if (copy_from_user(&params, (void __user *)arg, sizeof(params)))
			return -EFAULT;
		ret = mmdma_execute(params.srcAdd, params.dstAdd, params.size);
		break;
	}
	default:
		return -ENOTTY;
	}
	return ret;
}

static int mmdma_open(struct inode *inode, struct file *filp)
{
	pr_debug("open \n");
	return 0;
}

static int mmdma_release(struct inode *inode, struct file *filp)
{
	pr_debug("release \n");
	return 0;
}

static struct file_operations mmdma_fops = {
	.open = mmdma_open,
	.release = mmdma_release,
	.unlocked_ioctl = mmdma_ioctl,
};

int __init mmdma_init(void)
{
	int ret;

	pr_debug("init \n");
#if defined (CONFIG_MACH_BCM_FPGA_E) || defined (CONFIG_MACH_BCM_FPGA)
	mmdmamajor = 208;
	ret = register_chrdev(mmdmamajor, MMDMA_DEV_NAME, &mmdma_fops);
	if (ret != 0)
		return -EINVAL;
#else
	ret = register_chrdev(mmdmamajor, MMDMA_DEV_NAME, &mmdma_fops);
	if (ret < 0)
		return -EINVAL;
	mmdmamajor = ret;
	ret = 0;
#endif
	if (ret) {
		pr_err("Registering mmdma device failed \n");
	} else {
		pr_info ("major(%d) \n", mmdmamajor);
		mmdma_class = class_create(THIS_MODULE, MMDMA_DEV_NAME);
		device_create(mmdma_class, NULL, MKDEV(mmdmamajor, 0), NULL, MMDMA_DEV_NAME);
	}

	return ret;
}

void __exit mmdma_exit(void)
{
	pr_info("exit \n");
	device_destroy(mmdma_class, MKDEV(mmdmamajor, 0));
	class_destroy(mmdma_class);
	unregister_chrdev(mmdmamajor, MMDMA_DEV_NAME);
}

module_init(mmdma_init);
module_exit(mmdma_exit);

MODULE_AUTHOR("Broadcom Corporation");
MODULE_DESCRIPTION("MMDMA device driver");
MODULE_LICENSE("GPL");

