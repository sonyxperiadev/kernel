/*******************************************************************************
Copyright 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement
governing use of this software, this software is licensed to you under the
terms of the GNU General Public License version 2, available at
http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software
in any way with any other Broadcom software provided under a license other than
the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

#define H264_OL_DEV_NAME "h264_ol"
#define pr_fmt(fmt) "<%s::%s::%d> " fmt "\n", "h264_ol", __func__, __LINE__

#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/mutex.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/uaccess.h>
#include <mach/rdb/brcm_rdb_h264.h>

#define H264_OL_BASE	0x3d00cc00
#define H264_OL_SIZE	0x1000

static struct miscdevice mdev;
static void __iomem *h264_base;
static struct clk *h264_clk;
DEFINE_MUTEX(h264_ol_mutex);

static int h264_ol_open(struct inode *inode, struct file *filp)
{
	mutex_lock(&h264_ol_mutex);
	clk_enable(h264_clk);
	return 0;
}

static int h264_ol_release(struct inode *inode, struct file *filp)
{
	clk_disable(h264_clk);
	mutex_unlock(&h264_ol_mutex);
	return 0;
}

static ssize_t h264_ol_write(struct file *filp, \
		const char __user *buf, \
		size_t size, \
		loff_t *offset)
{
	u32 value[2];
	if (copy_from_user(value, buf, 8)) {
		pr_err("copy_from_user failed");
		return 0;
	}
	iowrite32(value[1], h264_base + (value[0]&0xFF));
	return size;
}

static ssize_t h264_ol_read(struct file *filp, \
		char __user *buf, \
		size_t size, \
		loff_t *offset)
{
	u32 off, value;
	if (copy_from_user(&off, buf, 4)) {
		pr_err("copy_from_user failed");
		return 0;
	}
	value = ioread32(h264_base + (off&0xFF));
	if (copy_to_user(buf, &value, 4)) {
		pr_err("copy_to_user failed");
		return 0;
	}
	return 4;
}

static const struct file_operations h264_fops = {
	.open = h264_ol_open,
	.release = h264_ol_release,
	.read = h264_ol_read,
	.write = h264_ol_write,
};

int __init h264_ol_separate_init(void)
{
	int ret;
	pr_info("H264 OL Driver Init\n");

	mdev.minor = MISC_DYNAMIC_MINOR;
	mdev.name = H264_OL_DEV_NAME;
	mdev.fops = &h264_fops;
	mdev.parent = NULL;

	h264_clk = clk_get(NULL, "h264_axi_clk");
	if (IS_ERR(h264_clk))
		return -1;

	/* Map the H264 registers */
	h264_base = (void __iomem *)ioremap_nocache(H264_OL_BASE, H264_OL_SIZE);
	if (h264_base == NULL) {
		clk_put(h264_clk);
		pr_err("failed to map the registers.");
		return -1;
	}

	ret = misc_register(&mdev);
	if (ret) {
		clk_put(h264_clk);
		pr_err("failed to register misc device.");
		iounmap(h264_base);
		return ret;
	}
	return 0;
}

void __exit h264_ol_exit(void)
{
	pr_info("H264 Driver Exit\n");
	if (h264_base) {
		iounmap(h264_base);
		h264_base = NULL;
	}
	misc_deregister(&mdev);
}

module_init(h264_ol_separate_init);
module_exit(h264_ol_exit);

MODULE_AUTHOR("Broadcom Corporation");
MODULE_DESCRIPTION("H624 device driver");
MODULE_LICENSE("GPL");
