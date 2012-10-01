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


#include <linux/module.h>

#include <linux/bootmem.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/fcntl.h>
#include <linux/init.h>
#include <linux/mutex.h>
#include <linux/uaccess.h>

#define LOGRAM_VERSION "1"

static DEFINE_MUTEX(logram_mutex);
static long unsigned int logram_dump_address;

static loff_t logram_llseek(struct file *file, loff_t offset, int origin)
{
	return -1;
}

static ssize_t logram_read(struct file *file, char __user *buf,
			  size_t count, loff_t *ppos)
{
	printk(KERN_ERR "Not implemented");
	return 0;
}

static ssize_t logram_write(struct file *file, const char __user *buf,
			   size_t count, loff_t *ppos)
{

	int status = kstrtoul_from_user(buf, count, 16, &logram_dump_address);
	if (status) {
		printk(KERN_ERR "Error copying data from user");
		return status;
	}

	printk(KERN_ERR "Saving dump address %08lx to %08x",
			logram_dump_address, (unsigned int)&logram_dump_address);

	return count;
}

static int logram_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	return -EINVAL;
}

static long logram_unlocked_ioctl(struct file *file, unsigned int cmd,
		unsigned long arg)
{
	int ret;

	mutex_lock(&logram_mutex);
	ret = logram_ioctl(file, cmd, arg);
	mutex_unlock(&logram_mutex);

	return ret;
}

const struct file_operations logram_fops = {
	.owner		= THIS_MODULE,
	.llseek		= logram_llseek,
	.read		= logram_read,
	.write		= logram_write,
	.unlocked_ioctl	= logram_unlocked_ioctl,
};

static struct miscdevice logram_dev = {
	MISC_DYNAMIC_MINOR,
	"logram",
	&logram_fops
};

int __init logram_init(void)
{
	int ret = 0;

	printk(KERN_INFO "Broadcom logram helper v%s\n",
		LOGRAM_VERSION);
	ret = misc_register(&logram_dev);
	return ret;
}

void __exit logram_cleanup(void)
{
	misc_deregister(&logram_dev);
}

module_init(logram_init);
module_exit(logram_cleanup);
MODULE_LICENSE("GPL");
