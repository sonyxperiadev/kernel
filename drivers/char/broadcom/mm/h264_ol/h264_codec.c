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

#define H264_CODEC_DEV_NAME "h264_codec"
#define pr_fmt(fmt) "<%s::%s::%d> " fmt "\n", H264_CODEC_DEV_NAME,\
							__func__, __LINE__

#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/mutex.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/uaccess.h>
#include <mach/rdb/brcm_rdb_h264.h>

#define H264_CODEC_BASE	0x3d000000
#define H264_CODEC_SIZE	0x602000
#define SIZE 8192
static struct miscdevice mdev;
static void __iomem *h264_codec_base;
static struct clk *h264_codec_clk;
DEFINE_MUTEX(h264_codec_mutex);

struct codec_fop_data {

	u32 *buffer;
	loff_t off;
};

static int h264_codec_open(struct inode *inode, struct file *filp)
{
	u32 *buffer = kmalloc(SIZE, GFP_KERNEL);
	if (buffer == NULL)
		return -ENOMEM;

	struct codec_fop_data *private = kmalloc(sizeof(struct codec_fop_data),
								GFP_KERNEL);
	if (private == NULL) {
		kfree(buffer);
		return -ENOMEM;
	}
	private->buffer = buffer;
	filp->private_data = private;
	mutex_lock(&h264_codec_mutex);
	clk_enable(h264_codec_clk);
	return 0;
}

static int h264_codec_release(struct inode *inode, struct file *filp)
{
	clk_disable(h264_codec_clk);
	mutex_unlock(&h264_codec_mutex);
	struct codec_fop_data *private = (struct codec_fop_data *)
							filp->private_data;
	kfree(private->buffer);
	kfree(private);
	return 0;
}

static ssize_t h264_codec_write(struct file *filp, \
		const char __user *buf, \
		size_t size, \
		loff_t *offset)
{
/* The register offset is stored by lseek call before write*/
	int i = 0;
	int n_reg = 0;
	n_reg = size/4;
	struct codec_fop_data *private = (struct codec_fop_data *)
							filp->private_data;
	u32 *buffer = private->buffer;

	BUG_ON(size > SIZE);
	if (copy_from_user(buffer, buf, size)) {
		pr_err("copy_from_user failed");
		return 0;
	}
	u32 off = (private->off) & 0xFFFFFF;
	for (i = 0 ; i < n_reg ; i++) {
		iowrite32(buffer[i], h264_codec_base + off);
		off += 4;
	}
	return size;
}

static ssize_t h264_codec_read(struct file *filp, \
		char __user *buf, \
		size_t size, \
		loff_t *offset)
{
/*The offset is passed as the content of the buffer*/

	u32 off, value;
	BUG_ON(size != 4);
	if (copy_from_user(&off, buf, 4)) {
		pr_err("copy_from_user failed");
		return 0;
	}

	off = off & 0xFFFFFF;
	value = ioread32(h264_codec_base + off);
	if (copy_to_user(buf, &value, 4)) {
		pr_err("copy_to_user failed");
		return 0;
	}
	return 4;
}

static int h264_codec_mmap(struct file *filp, struct vm_area_struct *vma)
{
	unsigned long vma_size = vma->vm_end - vma->vm_start;

	if (vma_size & (~PAGE_MASK)) {
		pr_err("mmaps must be aligned to a multiple of pages_size.\n");
		return -EINVAL;
	}

	if (!vma->vm_pgoff)
		vma->vm_pgoff = H264_CODEC_BASE >> PAGE_SHIFT;
	else
		return -EINVAL;

	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

	if (remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff, vma_size,
					vma->vm_page_prot)) {
		pr_err("remap_pfn_range() failed\n");
		return -EINVAL;
	}

	return 0;
}

static loff_t h264_codec_lseek(struct file *filp, loff_t offset, int pos)
{
	struct codec_fop_data *private = (struct codec_fop_data *)
							filp->private_data;
	private->off = offset;
	return 0;
}

static const struct file_operations h264_codec_fops = {
	.open = h264_codec_open,
	.release = h264_codec_release,
	.read = h264_codec_read,
	.write = h264_codec_write,
	.llseek = h264_codec_lseek,
};

int __init h264_codec_separate_init(void)
{
	int ret;
	pr_info("H264 Codec Driver Init\n");

	mdev.minor = MISC_DYNAMIC_MINOR;
	mdev.name = H264_CODEC_DEV_NAME;
	mdev.fops = &h264_codec_fops;
	mdev.parent = NULL;

	h264_codec_clk = clk_get(NULL, "h264_axi_clk");
	if (h264_codec_clk == NULL)
		return -1;

	/* Map the H264 registers */
	h264_codec_base = (void __iomem *)ioremap_nocache(
					H264_CODEC_BASE, H264_CODEC_SIZE);
	if (h264_codec_base == NULL) {
		pr_err("failed to map the registers.");
		return -1;
	}

	ret = misc_register(&mdev);
	if (ret) {
		pr_err("failed to register misc device.");
		iounmap(h264_codec_base);
		return ret;
	}
	return 0;
}

void __exit h264_codec_exit(void)
{
	pr_info("H264 codec Driver Exit\n");
	if (h264_codec_base) {
		iounmap(h264_codec_base);
		h264_codec_base = NULL;
	}
	misc_deregister(&mdev);
}

module_init(h264_codec_separate_init);
module_exit(h264_codec_exit);

MODULE_AUTHOR("Broadcom Corporation");
MODULE_DESCRIPTION("H624 codec device driver");
MODULE_LICENSE("GPL");
