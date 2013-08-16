/*********************************************************************
 * *
 * *  Copyright 2010 Broadcom Corporation
 * *
 * *  Unless you and Broadcom execute a separate written software license
 * *  agreement governing use of this software, this software is licensed
 * *  to you under the terms of the GNU
 * *  General Public License version 2 (the GPL), available at
 * *  http://www.broadcom.com/licenses/GPLv2.php with the following added
 * *  to such license:
 * *  As a special exception, the copyright holders of this software give
 * *  you permission to link this software with independent modules, and
 * *  to copy and distribute the resulting executable under terms of your
 * *  choice, provided that you also meet, for each linked independent module,
 * *  the terms and conditions of the license of that module. An independent
 * *  module is a module which is not derived from this software.  The special
 * *  exception does not apply to any modifications of the software.
 * *  Notwithstanding the above, under no circumstances may you combine this
 * *  software in any way with any other Broadcom software provided under a
 * *  license other than the GPL, without Broadcom's express prior written
 * *  consent.
 * ***********************************************************************/

#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/completion.h>
#include <linux/bio.h>
#include <linux/stat.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/genhd.h>
#include <linux/completion.h>
#include <linux/bio.h>
#include <linux/module.h>

#include <linux/emmc_rpmb.h>
#include <linux/miscdevice.h>

#define MAX_KEYBOX_LEN		256
#define KEYBOX_DEBUG

/*keybox ioctl data structure*/
struct _bcm_keybox_ioc_param {
	void __user *buf;
	int len;
};

#define bcm_keybox_ioc_param struct _bcm_keybox_ioc_param

/* an 8-bit integer selected to be specific to this driver */
#define BCM_KEYBOX_IOC_MAGIC   0xF2
/**
 *
 *  ioctl commands
 *
 **/
#define BCM_KEYBOX_IOC_GET_DATA	       _IOR(BCM_KEYBOX_IOC_MAGIC, 1, bcm_keybox_ioc_param)
#define BCM_KEYBOX_IOC_WR_DATA           _IOW(BCM_KEYBOX_IOC_MAGIC, 2, bcm_keybox_ioc_param)

#define BCM_KEYBOX_IOC_MAXNR			   3

/*open state for keybox*/
static int open_state;

#ifdef KEYBOX_DEBUG

static struct kobject *keybox_kobj;

/* Utility function to dump buffer for debug purpose */
static void dump_buffer(const char *cmdbuf, unsigned int size)
{
	uint i;

	pr_info("size: %d\r\n", size);

	for (i = 0; i < size; i++) {
		if (!(i % 32))
			pr_info("\n");
		pr_info("0x%x ", *(cmdbuf + i));
	}
	pr_info("\n");
}

static ssize_t
kbox_read_key(struct device *dev, struct device_attribute *attr,
	      const char *buf, size_t n)
{
	int bytes;
	char *rd_buff;
	int ret = 0;

	if (sscanf(buf, "%d", &bytes) != 1) {
		pr_err("Usage: echo [num of bytes] > "
		       "/sys/kbox_debug/kbox_read_key\n");
		return n;
	}

	if (bytes > MAX_KEYBOX_LEN) {
		pr_err("keybox read count %d is over the limit 256\r\n", bytes);
		return -EINVAL;
	}

	rd_buff = kmalloc(bytes, GFP_KERNEL);

	ret = readkeyboxKey(rd_buff, bytes);
	if (ret < 0) {
		pr_err("readkeyboxKey Failure. Try again\n");
		kfree(rd_buff);
		return -1;
	}

	pr_info("Done Reading Data from keybox, return 0x%x\n", ret);
	dump_buffer(rd_buff, bytes);

	kfree(rd_buff);
	return n;
}

static ssize_t
kbox_read_data(struct device *dev, struct device_attribute *attr,
	       const char *buf, size_t n)
{
	int bytes;
	char *rd_buff;
	int ret = 0;

	if (sscanf(buf, "%d", &bytes) != 1) {
		pr_err("Usage: echo [num of bytes] > "
		       "/sys/kbox_debug/keybox_read_data\n");
		return n;
	}

	if (bytes > MAX_KEYBOX_LEN) {
		pr_err("keybox read count %d is over the limit 256\r\n", bytes);
		return -EINVAL;
	}

	rd_buff = kmalloc(bytes, GFP_KERNEL);

	ret = readkeyboxData(rd_buff, bytes);
	if (ret < 0) {
		pr_err("readkeyboxData Failure. Try again\n");
		kfree(rd_buff);
		return -1;
	}

	pr_info("Done Reading Data from keybox, return 0x%x\n", ret);
	dump_buffer(rd_buff, bytes);

	kfree(rd_buff);
	return n;
}

static ssize_t
kbox_write_data(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t n)
{
	int bytes;
	int pattern;
	char *rd_buff;
	int ret = 0;
	int i = 0;

	if (sscanf(buf, "%d %x", &bytes, &pattern) != 2) {
		pr_err("Usage: echo  [num of bytes] [byte pattern] > "
		       "/sys/kbox_debug/kbox_write_data\n");
		return n;
	}

	if (bytes > MAX_KEYBOX_LEN) {
		pr_err("keybox write count %d is over the limit 256\r\n",
		       bytes);
		return -EINVAL;
	}

	rd_buff = kmalloc(bytes, GFP_KERNEL);

	for (i = 0; i < bytes; i++)
		*(rd_buff + i) = pattern;

	ret = writekeyboxData(rd_buff, bytes);
	if (ret < 0) {
		pr_err("writekeyboxData Failure. Try again\n");
		kfree(rd_buff);
		return -1;
	}

	pr_info("Done writing Data to keybox, return 0x%x\n", ret);
	dump_buffer(rd_buff, bytes);

	kfree(rd_buff);
	return n;
}

static DEVICE_ATTR(kbox_read_key, 0644, NULL, kbox_read_key);
static DEVICE_ATTR(kbox_read_data, 0644, NULL, kbox_read_data);
static DEVICE_ATTR(kbox_write_data, 0644, NULL, kbox_write_data);

static struct attribute *keybox_attrs[] = {
	&dev_attr_kbox_read_key.attr,
	&dev_attr_kbox_read_data.attr,
	&dev_attr_kbox_write_data.attr,
	NULL,
};

static struct attribute_group keybox_attr_group = {
	.attrs = keybox_attrs,
};
#endif

/************************************************************/
int keybox_open(struct inode *inode, struct file *filp)
{
	if (open_state == 0) {
		open_state = 1;
		pr_info("keybox open!\n");
		return 0;
	}
	pr_warning("keybox has been opend!\n");
	return -1;
}

int keybox_release(struct inode *inode, struct file *filp)
{
	if (open_state == 1) {
		open_state = 0;
		pr_info("keybox release!\n");
		return 0;
	}

	pr_warning("keybox has not been opend yet!\n");
	return -1;
}

/*read keybox key*/
ssize_t keybox_read(struct file *filp, char __user *buf, size_t count,
		    loff_t *fpos)
{
	char *rd_buff;
	int ret;

	if (open_state != 1) {
		pr_err("keybox has not been opend yet\r\n");
		return -1;
	}

	if (count > MAX_KEYBOX_LEN) {
		pr_err("keybox read count %d is over the limit 256\r\n", count);
		return -EINVAL;
	}

	pr_info("keybox key read count %d\n", count);

	rd_buff = kmalloc(count, GFP_KERNEL);

	ret = readkeyboxKey(rd_buff, count);
	if (ret < 0) {
		pr_err("Read Failure. Try again\n");
		kfree(rd_buff);
		return -1;
	}

	pr_info("Done Reading Data from keybox, return 0x%x\n", ret);
#ifdef KEYBOX_DEBUG
	dump_buffer(rd_buff, count);
#endif

	if (copy_to_user(buf, rd_buff, count)) {
		kfree(rd_buff);
		pr_err("keybox_read: copy_to_user error\n");
		return -EFAULT;
	}

	kfree(rd_buff);
	return count;
}

static long handle_keybox_get_data(struct file *filp, unsigned int cmd,
				   unsigned long arg)
{
	char *rd_buff;
	int ret;
	bcm_keybox_ioc_param kb_param;
	bcm_keybox_ioc_param *usr_kb_param = NULL;

	if (open_state != 1) {
		pr_err("keybox has not been opend yet\r\n");
		return -1;
	}

	usr_kb_param = (bcm_keybox_ioc_param *) arg;
	kb_param.len = usr_kb_param->len;

	if (kb_param.len > MAX_KEYBOX_LEN) {
		pr_err("keybox read count %d is over the limit 256\r\n",
		       kb_param.len);
		return -EINVAL;
	}
	if (copy_from_user(&kb_param.buf, &usr_kb_param->buf,
			   sizeof(kb_param.buf)) != 0) {
		pr_err("handle_keybox_get_data: copy_from_user error\n");
		return -EFAULT;
	}

	pr_info("keybox data read count %d\n", kb_param.len);

	rd_buff = kmalloc(kb_param.len, GFP_KERNEL);

	ret = readkeyboxData(rd_buff, kb_param.len);
	if (ret < 0) {
		pr_err("readkeyboxData Failure. Try again\n");
		kfree(rd_buff);
		return -1;
	}
#ifdef KEYBOX_DEBUG
	dump_buffer(rd_buff, kb_param.len);
#endif

	if (copy_to_user(kb_param.buf, rd_buff, kb_param.len)) {
		kfree(rd_buff);
		pr_err("handle_keybox_get_data: copy_to_user error\n");
		return -EFAULT;
	}

	kfree(rd_buff);
	return kb_param.len;
}

static long handle_keybox_wr_data(struct file *filp, unsigned int cmd,
				  unsigned long arg)
{
	char *rd_buff;
	int ret;
	bcm_keybox_ioc_param kb_param;
	bcm_keybox_ioc_param *usr_kb_param = NULL;

	if (open_state != 1) {
		pr_err("keybox has not been opend yet\r\n");
		return -1;
	}
	usr_kb_param = (bcm_keybox_ioc_param *) arg;
	kb_param.len = usr_kb_param->len;

	if (kb_param.len > MAX_KEYBOX_LEN) {
		pr_err("keybox read count %d is over the limit 256\r\n",
		       kb_param.len);
		return -EINVAL;
	}

	if (copy_from_user(&kb_param.buf, &usr_kb_param->buf,
			   sizeof(kb_param.buf)) != 0) {
		pr_err("handle_keybox_wr_data buf: copy_from_user error\n");
		return -EFAULT;
	}

	pr_info("keybox data write count %d\n", kb_param.len);

	rd_buff = kmalloc(kb_param.len, GFP_KERNEL);

	if (copy_from_user(rd_buff, kb_param.buf, kb_param.len) != 0) {
		pr_err("handle_keybox_wr_data: copy_from_user error\n");
		kfree(rd_buff);
		return -EFAULT;
	}

	ret = writekeyboxData(rd_buff, kb_param.len);
	if (ret < 0) {
		pr_err("writekeyboxData failure Try again\n");
		kfree(rd_buff);
		return -1;
	}

	kfree(rd_buff);
	return kb_param.len;
}

static long keybox_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	long ret = 0;

	if (_IOC_TYPE(cmd) != BCM_KEYBOX_IOC_MAGIC
	    || _IOC_NR(cmd) >= BCM_KEYBOX_IOC_MAXNR) {
		pr_err("keybox_ioctl ERROR cmd=0x%x\n", cmd);
		return -ENOIOCTLCMD;
	}

	switch (cmd) {
	case BCM_KEYBOX_IOC_GET_DATA:
		{
			ret = handle_keybox_get_data(filp, cmd, arg);
			break;
		}
	case BCM_KEYBOX_IOC_WR_DATA:
		{
			ret = handle_keybox_wr_data(filp, cmd, arg);
			break;
		}
	default:
		ret = -ENOIOCTLCMD;
		pr_err("keybox_ioctl ERROR unhandled cmd=0x%x\n", cmd);
		break;
	}

	return ret;
}

/***********************************************/
const struct file_operations keybox_fops = {
	.owner = THIS_MODULE,
	.open = keybox_open,
	.release = keybox_release,
	.read = keybox_read,
	.unlocked_ioctl = keybox_ioctl,
};

struct miscdevice keybox_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.fops = &keybox_fops,
	.name = "bcm_kbox",
};

static int __init keybox_init(void)
{
	int err;

	pr_info("keybox driver initialized\n");

	err = misc_register(&keybox_dev);
	if (err) {
		pr_err("keybox_init: keybox_dev register failed\n");
		return err;
	}
#ifdef KEYBOX_DEBUG
	keybox_kobj = kobject_create_and_add("kbox_debug", NULL);
	if (!keybox_kobj)
		return -ENOMEM;

	return sysfs_create_group(keybox_kobj, &keybox_attr_group);
#else
	return 0;
#endif
}

static void __exit keybox_exit(void)
{
	misc_deregister(&keybox_dev);
#ifdef KEYBOX_DEBUG
	sysfs_remove_group(keybox_kobj, &keybox_attr_group);
#endif
}

module_init(keybox_init);
module_exit(keybox_exit);
