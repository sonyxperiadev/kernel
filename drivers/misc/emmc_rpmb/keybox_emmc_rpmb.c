/*******************************************************************************
Copyright 2010 Broadcom Corporation.  All rights reserved.
This program is the proprietary software of Broadcom Corporation and/or its
licensors, and may only be used, duplicated, modified or distributed pursuant
to the terms and conditions of a separate, written license agreement executed
between you and Broadcom (an "Authorized License").

Except as set forth in an Authorized License, Broadcom grants no
license(express or implied), right to use, or waiver of any kind with respect
to the Software, and Broadcom expressly reserves all rights in and to the
Software and all intellectual property rights therein.  IF YOU HAVE NO
AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY,
AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

Except as expressly set forth in the Authorized License,
1. This program, including its structure, sequence and organization, constitutes
the valuable trade secrets of Broadcom, and you shall use all reasonable efforts
to protect the confidentiality thereof, and to use this information only
in connection with your use of Broadcom integrated circuit products.

2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" AND
WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR WARRANTIES,
EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO THE SOFTWARE.
BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE,
MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF
VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
PERFORMANCE OF THE SOFTWARE.

3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR USE
OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE
POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY
PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED
REMEDY.
*******************************************************************************/

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

#include <linux/emmc_rpmb.h>
#include <linux/miscdevice.h>

#define MAX_KEYBOX_LEN		256
#define KEYBOX_DEBUG

static struct kobject *keybox_kobj;
static int open_state;

#ifdef KEYBOX_DEBUG
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

ssize_t keybox_read(struct file *filp, char *buf, size_t count, loff_t fpos)
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

	pr_info("keybox read count %d\n", count);

	rd_buff = kmalloc(count, GFP_KERNEL);

	ret = readkeybox(rd_buff, count);
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

static ssize_t
keybox_read_data(struct device *dev, struct device_attribute *attr,
		 const char *buf, size_t n)
{
	int bytes;
	char *rd_buff;
	int ret = 0;

	if (sscanf(buf, "%d", &bytes) != 1) {
		pr_err("Usage: echo [num of bytes] > "
		       "/sys/keybox_debug/keybox_read_data\n");
		return n;
	}

	if (bytes > MAX_KEYBOX_LEN) {
		pr_err("keybox read count %d is over the limit 256\r\n", bytes);
		return -EINVAL;
	}

	rd_buff = kmalloc(bytes, GFP_KERNEL);

	ret = readkeybox(rd_buff, bytes);
	if (ret < 0) {
		pr_err("Read Failure. Try again\n");
		kfree(rd_buff);
		return -1;
	}

	pr_info("Done Reading Data from keybox, return 0x%x\n", ret);
#ifdef KEYBOX_DEBUG
	dump_buffer(rd_buff, bytes);
#endif

	kfree(rd_buff);
	return n;
}

/***********************************************/
const struct file_operations keybox_fops = {
	.owner = THIS_MODULE,
	.open = keybox_open,
	.release = keybox_release,
	.read = keybox_read,
};

struct miscdevice keybox_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.fops = &keybox_fops,
	.name = "bcm_keybox",
	.nodename = "bcm_keybox_node"
};

static DEVICE_ATTR(keybox_read_data, 0666, NULL, keybox_read_data);

static struct attribute *keybox_attrs[] = {
	&dev_attr_keybox_read_data.attr,
	NULL,
};

static struct attribute_group keybox_attr_group = {
	.attrs = keybox_attrs,
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

	keybox_kobj = kobject_create_and_add("keybox_debug", NULL);
	if (!keybox_kobj)
		return -ENOMEM;

	return sysfs_create_group(keybox_kobj, &keybox_attr_group);
}

static void __exit keybox_exit(void)
{
	misc_deregister(&keybox_dev);
	sysfs_remove_group(keybox_kobj, &keybox_attr_group);
}

module_init(keybox_init);
module_exit(keybox_exit);
