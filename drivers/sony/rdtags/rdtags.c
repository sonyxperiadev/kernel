/*
 *
 * Author: Nilsson, Stefan 2 <stefan2.nilsson@sonymobile.com>
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
#include <linux/slab.h>
#include <linux/kfifo.h>
#include <linux/delay.h>
#include <linux/vmalloc.h>
#include <linux/mutex.h>
#include "rdtags.h"

#define RDTAGS_SIG 0x47415452
#define RDTAGS_NAME_SIZE 28
#define RDTAGS_ALIGNMENT 64

#define RDTAGS_PROC_NODE_NAME	"rdtag"
#define RDTAGS_PROC_DIR_NAME	"rdtags"
#define LAST_RDTAGS_PROC_DIR_NAME "last_rdtags"

static struct device *dev;
static struct proc_dir_entry *entry;
static void *rdtags_io_base;
static void *rdtags_base;
static void *rdtags_end;
static void *last_rdtags_base;
static void *last_rdtags_end;
static struct proc_dir_entry *last_entry;
static size_t rdtags_size;
static DEFINE_MUTEX(rdlock);
static uint8_t rdtags_initialized;
static unsigned long mem_base;
static unsigned long mem_size;

struct rtag_head {
	uint32_t sig;
	uint32_t data_size;
	uint8_t name[RDTAGS_NAME_SIZE];
	uint8_t null_term; /* Shall be 0 to preserve compatibility */
	uint8_t flags;
	uint8_t data[0];
};

enum procfs_cmd {
	PROCFS_CMD_ADD,
	PROCFS_CMD_DELETE,
};

struct procfs_fifo_item {
	enum procfs_cmd cmd;
	char name[RDTAGS_NAME_SIZE];
};

/*required for platform device register*/
static struct resource rdtags_resources[] = {
        [0] = {
                .name   = "rdtags_mem",
                .flags  = IORESOURCE_MEM,
        },
};

static struct platform_device rdtags_device = {
        .name           = "rdtags",
        .id             = -1,
};

#define PROCFS_FIFO_SIZE 64
static DEFINE_KFIFO(procfs_fifo, struct procfs_fifo_item, PROCFS_FIFO_SIZE);

static void procfs_work_func(struct work_struct *work);
static DECLARE_WORK(procfs_work, procfs_work_func);

#define PROCFS_ASYNC_ADD(x) procfs_async_cmd((x), PROCFS_CMD_ADD)
#define PROCFS_ASYNC_DELETE(x) procfs_async_cmd((x), PROCFS_CMD_DELETE)

#define SLEEP_TIME_ASYNC_FINISH 10 /* ms */

#define MAX(X, Y) ((X) >= (Y) ? (X) : (Y))
#define MIN(X, Y) ((X) <= (Y) ? (X) : (Y))

#define RDTAGS_NEXT_TAG(x) ((struct rtag_head *) \
			    ALIGN((unsigned long)x + x->data_size + \
				  sizeof(*x), RDTAGS_ALIGNMENT))

#define RDTAGS_BLK_SIZE(x) ALIGN(x->data_size + sizeof(*x), RDTAGS_ALIGNMENT)
#define RDTAGS_BASE_VALID(x) ((void *)x >= rdtags_base && \
			      (void *)x < rdtags_end)
#define LAST_RDTAGS_BASE_VALID(x) ((void *)x >= last_rdtags_base && \
				   (void *)x < last_rdtags_end)

#define RDTAGS_ADDR_VALID(x) (RDTAGS_BASE_VALID(x) || LAST_RDTAGS_BASE_VALID(x))

#define CRC_SIZE 2 /* bytes */
#define RDTAG_FLAGS_CRC_PRESENT 0x1

static ssize_t tag_read(struct file *file, char __user *buf, size_t size,
		loff_t *off);

static const struct proc_ops tag_fops = {
	.proc_read = tag_read,
};

static int procfs_create_node(char *name)
{
	struct proc_dir_entry *subentry;

	if (!entry)
		return -ENXIO;

	subentry = proc_create_data(name, S_IFREG | S_IRUSR | S_IRGRP, entry, &tag_fops,
			(void *)name);
	if (!subentry)
		return -ENOMEM;


	return 0;
}

static int procfs_delete_node(char *name)
{
	if (!entry)
		return -ENXIO;

	remove_proc_entry(name, entry);

	return 0;
}

static void procfs_work_func(struct work_struct *work)
{
	struct procfs_fifo_item item;

	while (kfifo_get(&procfs_fifo, &item)) {
		int ret;
		char *name = (char *)&item.name;

		dev_dbg(dev, "wq: Processing request %s %s\n",
			item.cmd == PROCFS_CMD_ADD ? "ADD" : "DELETE", name);

		switch (item.cmd) {
		case PROCFS_CMD_ADD:
			ret = procfs_create_node(name);
			if (ret < 0)
				dev_err(dev, "wq: Failed to create proc" \
					"subentry \"%s\": %d\n", name, ret);
			break;
		case PROCFS_CMD_DELETE:
			ret = procfs_delete_node(name);
			if (ret < 0)
				dev_err(dev, "wq: Failed to remove proc" \
					"subentry \"%s\": %d\n", name, ret);
			break;
		default:
			dev_err(dev, "wq: Unknown request: %d for %s\n",
				item.cmd, name);
		}
	}
}

static void procfs_async_cmd(char *name, enum procfs_cmd cmd)
{
	struct procfs_fifo_item item;

	if (unlikely(kfifo_is_full(&procfs_fifo))) {
		dev_err(dev, "procfs FIFO buffer overflow! procfs will" \
			"be out of sync!\n");
		return;
	}
	item.cmd = cmd;
	strlcpy(item.name, name, RDTAGS_NAME_SIZE);
	dev_dbg(dev, "procfs WQ request: %s %s\n",
		cmd == PROCFS_CMD_ADD ? "ADD" : "DELETE", name);
	kfifo_put(&procfs_fifo, item);
	schedule_work(&procfs_work);
}

static void rdtags_flush(void){
	if (rdtags_io_base && rdtags_base){
		memcpy_toio(rdtags_io_base, rdtags_base, rdtags_size);
		pr_err("copying back to ram , rdtags is updated\n");
	}
}

static char *get_valid_name(char *name)
{
	char *temp;

	if (!name)
		return NULL;

	/* Make sure the name is not longer than RDTAGS_NAME_SIZE */
	if (RDTAGS_NAME_SIZE == strnlen(name, RDTAGS_NAME_SIZE))
		name[RDTAGS_NAME_SIZE - 1] = 0x0;

	/* Check the name for disallowed characters */
	temp = strpbrk(name, "\r\n /");
	if (temp)
		*temp = 0x0;

	/* Check that the name still actually contains something */
	if (name[0] == 0x0)
		return NULL;

	return name;
}

static struct rtag_head *get_next_free(void)
{
	struct rtag_head *mt = (struct rtag_head *)rdtags_base;

	while (mt->sig == RDTAGS_SIG) {

		/* Go to next tag */
		mt = RDTAGS_NEXT_TAG(mt);

		/* Check that we are not outside the buffer */
		if (!RDTAGS_BASE_VALID(mt))
			return NULL;
	}

	return mt;
}

static struct rtag_head *get_tag(const char *name)
{
	struct rtag_head *mt = (struct rtag_head *)rdtags_base;

	while (mt->sig == RDTAGS_SIG) {

		if (strncmp(name, mt->name, RDTAGS_NAME_SIZE) == 0)
			return mt;

		/* Go to next tag */
		mt = RDTAGS_NEXT_TAG(mt);

		/* Check that we are not outside the buffer */
		if (!RDTAGS_BASE_VALID(mt))
			return NULL;
	}

	return NULL;
}

static void _remove_tag(struct rtag_head *mt)
{
	struct rtag_head *mt_next, *mt_free;
	size_t size_mv, size_clr;

	if (!RDTAGS_BASE_VALID(mt))
		return;
	/*
	 * we need to get some address & size for recompacting tags:
	 *                            |- size_mv -|
	 * --------------------------------------------------------------
	 * |rdtags_base|...|    mt    |mt_next|...|mt_free|...|rdtags_end|
	 * --------------------------------------------------------------
	 *                 |-size_clr-|
	 */
	mt_next = RDTAGS_NEXT_TAG(mt);
	if (!RDTAGS_BASE_VALID(mt_next))
		mt_next = (struct rtag_head *)rdtags_end;

	mt_free = get_next_free();
	if (NULL == mt_free)
		mt_free = (struct rtag_head *)rdtags_end;
	size_mv = ((unsigned long)mt_free) - ((unsigned long)mt_next);
	size_clr = RDTAGS_BLK_SIZE(mt);

	/* Remove procfs interface asynchronously */
	PROCFS_ASYNC_DELETE(mt->name);

	/* move rear tags */
	memmove((void *)mt, (void *)mt_next, size_mv);

	/* clear vacated memory */
	if ((size_clr <= rdtags_size) &&
	    ((void *)(unsigned long)mt_free - size_clr) >= rdtags_base) {
		memset((void *)(((unsigned long)mt_free) - size_clr), 0,
				size_clr);
	}

	rdtags_flush();
}

/*
 * Removes a tag
 *
 * name: Name of tag to remove
 *
 * Returns 0 on success or a negative error code on failure
 */
int rdtags_remove_tag(const char *name)
{
	struct rtag_head *mt;
	int ret = 0;

	if (!name)
		return -EINVAL;

	if (!rdtags_initialized)
		return -ENODEV;

	dev_dbg(dev, "Removing tag \"%s\"\n", name);

	mutex_lock(&rdlock);

	/* Get the tag */
	mt = get_tag(name);
	if (!mt) {
		ret = -ENOENT;
		goto exit;
	}

	_remove_tag(mt);

exit:
	mutex_unlock(&rdlock);

	if (ret)
		dev_err(dev, "Could not remove tag \"%s\"\n", name);
	else
		dev_dbg(dev, "Removed tag \"%s\"\n", name);

	return ret;
}
EXPORT_SYMBOL(rdtags_remove_tag);

static int _add_tag(const char *name, const unsigned char *data, size_t size)
{
	struct rtag_head *mt = get_next_free();
	void *tag_end = (void *)(((unsigned long)mt) + sizeof(*mt) + size);

	if (!mt || !RDTAGS_ADDR_VALID(tag_end)) {
		/* We have run out of tag space */
		dev_err(dev, "Out of tag space! Could not add tag \"%s\" " \
			"with %zd bytes of data!\n", name, size);
		return -ENOMEM;
	}

	mt->sig = RDTAGS_SIG;
	strlcpy(mt->name, name, RDTAGS_NAME_SIZE);
	memcpy(mt->data, data, size);
	mt->data_size = size;

	/* Add procfs interface asynchronously */
	PROCFS_ASYNC_ADD(mt->name);

	rdtags_flush();

	return 0;
}

static int _update_tag(struct rtag_head *mt, const unsigned char *data,
		       size_t size)
{
	char name[RDTAGS_NAME_SIZE];
	struct rtag_head *mt_free;
	size_t free_size;

	if (!mt || mt->sig != RDTAGS_SIG) {
		dev_err(dev, "Not a valid RTAG!\n");
		return -ENOENT;
	}

	/* If the size is identical, we can update the previous tag */
	if (size == mt->data_size) {
		dev_dbg(dev, "Updating tag \"%s\"\n", mt->name);
		memcpy(mt->data, data, size);
		return 0;
	}

	/* compute available memory size if recompact tags */
	free_size = RDTAGS_BLK_SIZE(mt);
	mt_free = get_next_free();
	if (NULL == mt_free)
		mt_free = (struct rtag_head *)rdtags_end;
	free_size += ((unsigned long)rdtags_end) - ((unsigned long)mt_free);

	/* make sure the new tag fits before removing the old one */
	if (free_size < size + sizeof(*mt)) {
		dev_err(dev, "No enough memory, abort updating tag \"%s\"\n",
			mt->name);
		return -ENOMEM;
	} else {
		dev_dbg(dev, "Rewriting tag \"%s\"\n", mt->name);
		strlcpy(name, mt->name, RDTAGS_NAME_SIZE);
		_remove_tag(mt);

		return _add_tag(name, data, size);
	}
}

/*
 *  append data onto tag
 *
 * Note that if the tag name already exists, the existing tag
 * will be updated (names must be unique).
 *
 * name: Name of tag to add
 * data: Pointer to the data to add
 * size: Size of the data to add
 *
 * Returns 0 on success or a negative error code on failure
 */
int rdtags_append_tagdata(const char *name, const unsigned char *data,
		   const uint32_t size)
{
	struct rtag_head *mt;
	int ret = 0;

	if (!name || !data || size == 0)
		return -EINVAL;

	if (!rdtags_initialized)
		return -ENODEV;

	dev_dbg(dev, "Appending tag \"%s\"\n", name);

	mutex_lock(&rdlock);

	/* First check if the tag exists */
	mt = get_tag(name);
	if (mt != NULL) {
		char *kbuf;
		unsigned int totalsize = 0;

		totalsize = size + mt->data_size;

		kbuf = vmalloc(totalsize);
		if (NULL == kbuf) {
			dev_err(dev, "Unable to assign memory.\n");
			ret = -ENOMEM;
			goto exit;
		}

		/* Update the tag */
		memcpy(kbuf, mt->data, mt->data_size);
		memcpy((kbuf + mt->data_size), data, size);
		ret = _update_tag(mt, kbuf, totalsize);
		vfree(kbuf);
		goto exit;
	}

	/* Add the tag */
	ret = _add_tag(name, data, size);
exit:
	mutex_unlock(&rdlock);
	if (ret)
		dev_err(dev, "Could not add/update tag \"%s\" with %d bytes of data\n\n",
			name, size);
	else
		dev_dbg(dev, "Added/updated tag \"%s\" with %d bytes of data\n\n",
			name, size);

	return ret;
}
EXPORT_SYMBOL(rdtags_append_tagdata);

/*
 * Adds a tag
 *
 * Note that if the tag name already exists, the existing tag
 * will be updated (names must be unique).
 *
 * name: Name of tag to add
 * data: Pointer to the data to add
 * size: Size of the data to add
 *
 * Returns 0 on success or a negative error code on failure
 */
int rdtags_add_tag(const char *name, const unsigned char *data,
		   const size_t size)
{
	struct rtag_head *mt;
	int ret = 0;

	if (!name || !data || size == 0)
		return -EINVAL;

	if (!rdtags_initialized)
		return -ENODEV;

	dev_dbg(dev, "Adding tag \"%s\"\n", name);

	mutex_lock(&rdlock);

	/* First check if the tag exists */
	mt = get_tag(name);

	if (mt != NULL) {
		/* Update the tag */
		ret = _update_tag(mt, data, size);
		goto exit;
	}

	/* Add the tag */
	ret = _add_tag(name, data, size);
exit:
	mutex_unlock(&rdlock);

	if (ret)
		dev_err(dev, "Could not add/update tag \"%s\" with %zd" \
				"bytes of data\n\n", name, size);
	else
		dev_dbg(dev, "Added/updated tag \"%s\" with %zd" \
				"bytes of data\n\n", name, size);

	return ret;
}
EXPORT_SYMBOL(rdtags_add_tag);

/*
 * Gets the data from a tag
 *
 * name: Name of tag to get data from
 * data: Pointer to the buffer to receive the data
 * size: Size of the buffer to receive the data. Will be updated with the
 *       actual size of the data.
 *
 * Returns a negative error code or 0 on success
 */
int rdtags_get_tag_data(const char *name, unsigned char *data, size_t *size)
{
	struct rtag_head *mt;
	int ret = 0;

	if (!name || !size)
		return -EINVAL;

	if (!rdtags_initialized)
		return -ENODEV;

	mutex_lock(&rdlock);

	/* First check if the tag exists */
	mt = get_tag(name);

	if (mt == NULL) {
		ret = -ENOENT;
		goto error;
	}

	/* Check if the buffer is valid and that it is large enough */
	if (!data || (*size < mt->data_size)) {
		/* Update "size" with the required size */
		*size = mt->data_size;
		ret = -ENOBUFS;
		goto error;
	}

	/* Copy the data and update the size */
	memcpy(data, mt->data, mt->data_size);
	*size = mt->data_size;
error:
	mutex_unlock(&rdlock);
	if (ret == -ENOBUFS)
		dev_dbg(dev, "Returning size %zd for tag \"%s\"!\n",
			*size, name);
	else if (ret)
		dev_err(dev, "Could not get data for tag \"%s\": %d!\n",
			name, ret);
	else
		dev_dbg(dev, "Read data of %zd bytes for tag \"%s\"!\n",
			*size, name);

	return ret;
}
EXPORT_SYMBOL(rdtags_get_tag_data);

/*
 * Clears all tags and "reformats" the entire tag area
 */
void rdtags_clear_tags(void)
{
	struct rtag_head *mt = (struct rtag_head *)rdtags_base;

	if (!rdtags_initialized) {
		dev_err(dev, "Not yet initialized, cannot clear!\n");
		return;
	}

	dev_dbg(dev, "Clearing rdtags!\n");

	mutex_lock(&rdlock);

	/* Go through all tags and remove their procfs nodes */
	while (mt->sig == RDTAGS_SIG) {

		/* Remove the procfs entry for the tag */
		PROCFS_ASYNC_DELETE(mt->name);

		mt = RDTAGS_NEXT_TAG(mt);
	}

	/* Finally reset the entire area to make it clean */
	memset(rdtags_base, 0x0, rdtags_size);
	rdtags_flush();
	mutex_unlock(&rdlock);
}
EXPORT_SYMBOL(rdtags_clear_tags);

static int rebuild_tag_tree(const unsigned char *buf, size_t size)
{
	struct rtag_head *mt = (struct rtag_head *)buf;
	int count = 0;

	dev_dbg(dev, "Building tag tree\n");

	while (mt && mt->sig == RDTAGS_SIG) {

		if (!get_valid_name(mt->name)) {
			dev_warn(dev, "Found tag with invalid name!" \
					"Skipping it!\n");
			goto loop_next;
		}

		count++;
		dev_dbg(dev, "   Found tag: \"%s\" - with %d bytes of data\n",
			mt->name, mt->data_size);

		/* Add procfs interface synchronously */
		if (procfs_create_node(mt->name) < 0)
			dev_warn(dev, "Failed to create proc subentry \"%s\"\n",
				 mt->name);
loop_next:
		/* Go to next tag */
		mt = RDTAGS_NEXT_TAG(mt);

		/* Check that we are not outside the buffer */
		if (!RDTAGS_BASE_VALID(mt) && !LAST_RDTAGS_BASE_VALID(mt))
			break;
	}

	return count;
}

static ssize_t last_tags_read(struct file *file, char __user *ubuf,
			size_t len, loff_t *offset)
{
	loff_t pos = *offset;
	ssize_t count = 0;
	struct rtag_head *mt = (struct rtag_head *)last_rdtags_base;
	char *name = file->f_path.dentry->d_iname;

	while (mt->sig == RDTAGS_SIG) {

		if (strncmp(name, mt->name, RDTAGS_NAME_SIZE) == 0) {
			if ((size_t)pos >= mt->data_size)
				return 0;
			count = min(len, (size_t)(mt->data_size - pos));
			if (copy_to_user(ubuf, mt->data + pos, count))
				return -EFAULT;
			break;
		}

		/* Go to next tag */
		mt = RDTAGS_NEXT_TAG(mt);

		/* Check that we are not outside the buffer */
		if (!LAST_RDTAGS_BASE_VALID(mt))
			return 0;
	}

	*offset += count;
	return count;
}

static ssize_t tag_read (struct file *file, char __user *ubuf, size_t len,
		loff_t *offset)
{
	ssize_t count;
	loff_t pos = *offset;
	size_t bufsize;
	unsigned char *buf;
	char *name = file->f_path.dentry->d_iname;
	char *dir = file->f_path.dentry->d_parent ?
			file->f_path.dentry->d_parent->d_iname : NULL;
	int ret;

	if (!name || !dir)
		return 0;

	/* check for last_rdtags list */
	if (!strncmp(dir, "last_rdtags", 11))
		return last_tags_read(file, ubuf, len, offset);


	/* Get the size of the required data buffer */
	if (rdtags_get_tag_data(name, NULL, &bufsize) != -ENOBUFS) {
		dev_err(dev, "Could not find tag \"%s\"!\n",
			name ? name : "NULL");
		return 0;
	}

	buf = kzalloc(bufsize, GFP_KERNEL);
	if (!buf) {
		dev_err(dev, "Could not allocate %zd bytes of memory!\n",
			bufsize);
		return 0;
	}

	/*
	 * Fill the buffer with data.
	 * This assumes that the tag size has not changed since the previous
	 * call to rdtags_get_tag_data. If it has, this call will fail, and
	 * the caller has to re-read the tag.
	 */
	ret = rdtags_get_tag_data(name, buf, &bufsize);
	if (ret) {
		dev_err(dev, "Could not get %zd bytes of data for" \
		       "tag \"%s\": %d!\n", bufsize, name, ret);
		kfree(buf);
		return 0;
	}

	count = MIN(len, bufsize - pos);
	if (copy_to_user(ubuf, buf + pos, count)) {
		kfree(buf);
		return -EFAULT;
	}

	kfree(buf);
	*offset += count;
	return count;
}

static ssize_t tags_read(struct file *file, char __user *ubuf, size_t len,
		loff_t *offset)
{
	/* Assume that this text always fits in count bytes */
	char *message = "Usage: <tag name/command> [tag data]\n"
		"\n"
		"command may be one of the following:\n"
		" * clear\n"
		" * append <tag name> <data>\n"
		" * delete <tag name>\n";
	loff_t pos = *offset;
	len = strlen(message);
	if (pos >= len)
		return 0;

	if (copy_to_user(ubuf, message, len))
		return -EFAULT;

	*offset += len;

	return len;
}

static ssize_t tags_write(struct file *file, const char __user *ubuf,
		size_t count, loff_t *offset)
{
	char *tag_data;
	int tag_size;
	void *kbuf;
	char *tag_append_data;

	if (0 == count)
		return 0;

	kbuf = kmalloc(count + 1, GFP_KERNEL);
	if (NULL == kbuf) {
		dev_err(dev, "Unable to assign memory.\n");
		return -ENOMEM;
	}

	/* Copy to kernel space */
	tag_size = copy_from_user(kbuf, ubuf, count);

	/* NULL terminate is needed, since we will handle strings on kbuf */
	*((char *)(kbuf + count)) = 0x00;

	if (tag_size > 0) {
		dev_err(dev, "Unable to copy %d bytes from user space!\n",
			(int)tag_size);
		goto exit;
	}

	/* Check for special single commands */
	if (strncmp(kbuf, "clear", 5) == 0) {
		rdtags_clear_tags();
		goto exit;
	}

	/* Find delimiter */
	tag_data = strnchr(kbuf, RDTAGS_NAME_SIZE, ' ');

	if (!tag_data) {
		dev_err(dev, "Incorrect format, please supply a string of " \
			"format: <tag name> <tag data>\n");
		goto exit;
	}

	/* Null terminate name at delimiter and increment tag_data pointer */
	*tag_data++ = 0x0;

	/* Do some basic sanity checking */
	if (!get_valid_name(kbuf))
		goto exit;

	/*
	 * get tag data size. note that:
	 * get_valid_name(kbuf) replaces \n\r to NULL termination.
	 * strlen cannot be used here to calculate tag_size
	 */
	tag_size = count - (tag_data - (char *)kbuf);

	/* Check for special dual commands */
	if (strncmp(kbuf, "delete", RDTAGS_NAME_SIZE) == 0) {
		/* Do some basic sanity checking */
		if (!get_valid_name(tag_data))
			goto exit;

		/* Remove the tag */
		rdtags_remove_tag(tag_data);
		goto exit;
	}

	if (strncmp(kbuf, "append", RDTAGS_NAME_SIZE) == 0) {
		tag_append_data = strnchr(tag_data, RDTAGS_NAME_SIZE, ' ');
		if (!tag_append_data) {
			dev_err(dev, "Incorrect format to append, " \
				"please supply " \
				"a string of <key> <tag name> <tag data>\n");
			goto exit;
		}

		*tag_append_data = 0x0;
		/* Do some basic sanity checking */
		if (!get_valid_name(tag_data))
			goto exit;

		tag_append_data++;
		tag_size = count - (tag_append_data - (char *)kbuf);
		/*append data on the tag */
		rdtags_append_tagdata(tag_data, tag_append_data, tag_size);
		goto exit;
	}

	/* Add the tag */
	if (rdtags_add_tag(kbuf, tag_data, tag_size))
		dev_err(dev, "rdtags_add_tag failed.\n");

exit:
	kfree(kbuf);

	/* Make sure all asynchronous work is complete before returning */
	while (!kfifo_is_empty(&procfs_fifo) && work_busy(&procfs_work)) {
		dev_dbg(dev, "Sleeping while waiting for WQ to finish\n");
		msleep(SLEEP_TIME_ASYNC_FINISH);
	}

	return count;
}

static const struct proc_ops rdnode_fops = {
	.proc_read	= tags_read,
	.proc_write	= tags_write,
};

static int last_rdtags_init(void)
{
	struct rtag_head *mt;
	int nbr_old_tags = 0;
	int ret = 0;

	last_rdtags_base = (void *)__get_free_pages(GFP_KERNEL,
					get_order(rdtags_size));
	if (!last_rdtags_base) {
		dev_err(dev, "Failed to allocate last_rdtags buffer\n");
		goto exit;
	}

	memcpy_fromio(last_rdtags_base, rdtags_io_base, rdtags_size);
	last_rdtags_end = last_rdtags_base + rdtags_size;

	/* Check if the buffer contains a valid start tag */

	mt = (struct rtag_head *)last_rdtags_base;
	if (mt->sig == RDTAGS_SIG) {

		dev_dbg(dev, "Found existing tags in memory!\n");

		/* Create procfs directory to hold the last tags */
		last_entry = proc_mkdir(LAST_RDTAGS_PROC_DIR_NAME,NULL);
		if (!last_entry) {
			dev_err(dev, "Failed to create last proc entry\n");
			goto exit;
		}

		/* Temporarily entry is assigned to create procfs files while
		 * rebuilding the tags inside /proc/last_rdtags.
		 */
		entry = last_entry;

		/* Rebuilding the tags as last_rdtags */
		nbr_old_tags = rebuild_tag_tree(last_rdtags_base, rdtags_size);

		/* Initializing entry to NULL, otherwise there is a chance that
		 * new rdtags will take "last_entry or entry" as a parent
		 * directory incase failure to create /proc/rdtags.
		 */
		entry = NULL;
	}

	return nbr_old_tags;
exit:
	if (last_rdtags_base) {
		free_pages((unsigned long)last_rdtags_base,
					get_order(rdtags_size));
		last_rdtags_base = NULL;
	}

	return ret;
}

static int rdtags_driver_probe(struct platform_device *pdev)
{
	struct resource *res;
	//struct rdtags_platform_data *platform_data;
	int nbr_old_tags = 0;
	//int nbr_new_tags = 0;
	int ret = 0;

	dev = &pdev->dev;

	/* Get resources */
	res = platform_get_resource_byname(pdev, IORESOURCE_MEM,
					   "rdtags_mem");
	if (!res || !res->start) {
		dev_err(dev, "Resource invalid/absent\n");
		ret = -ENODEV;
		goto exit;
	}

	/* ioremap the static area */
	rdtags_size = res->end - res->start + 1;
	rdtags_io_base = (void *)ioremap(res->start, rdtags_size);
	if (!rdtags_io_base) {
		dev_err(dev, "Failed to map %zd bytes of memory at 0x%llx!\n",
			rdtags_size, res->start);
		ret = -EINVAL;
		goto exit;
	}

	nbr_old_tags = last_rdtags_init();

	rdtags_base = (void *)__get_free_pages(GFP_KERNEL,
					get_order(rdtags_size));
	if (!rdtags_base) {
		dev_err(dev, "Failed to allocate rdtags buffer\n");
		ret = -ENOMEM;
		goto exit;
	}

	rdtags_end = rdtags_base + rdtags_size;
	memset(rdtags_base, 0x0, rdtags_size);
	memset_io(rdtags_io_base, 0x0, rdtags_size);

	/* Add procfs interface */
	entry = proc_create_data(RDTAGS_PROC_NODE_NAME,
				  S_IFREG | S_IRUGO | S_IWUSR, NULL,
				  &rdnode_fops, NULL);
	if (!entry) {
		dev_err(dev, "Failed to create proc entry\n");
		ret = -ENOMEM;
		goto exit;
	}

	/* Create procfs directory to hold the tags */
	entry = proc_mkdir(RDTAGS_PROC_DIR_NAME, NULL);

	rdtags_initialized = 1;

	return 0;

exit:
	if (rdtags_io_base) {
		iounmap(rdtags_io_base);
		rdtags_io_base = NULL;
	}

	if (rdtags_base) {
		free_pages((unsigned long)rdtags_base, get_order(rdtags_size));
		rdtags_base = NULL;
	}

	return ret;
}

static struct platform_driver rdtags_driver = {
	.probe = rdtags_driver_probe,
	.driver		= {
		.name	= "rdtags",
	},
};


static const struct of_device_id dt_match[] = {
        { .compatible = "rdtag_store_region" },
        {}
};

static int __init ramdump_rdtag_store_init(void)
{
        struct device_node *node;
        uint32_t *regs = NULL;
        size_t cells;

        node = of_find_matching_node(NULL, dt_match);
        if (!node) {
                pr_err("rdtag_store_driver node not found\n");
                return -EINVAL;
        }

        cells = of_n_addr_cells(node) + of_n_size_cells(node);
        regs = kcalloc(cells, sizeof(uint32_t), GFP_KERNEL);
        if (!regs) {
                pr_err("Failed to allocate memory for cells\n");
                of_node_put(node);
                return -ENOMEM;
        }

        if (of_property_read_u32_array(node, "reg", regs, cells)) {
                pr_err("unable to find base address of node in dtb\n");
                goto failed;
        }
        if (cells == 4) {
                mem_base = (unsigned long)regs[0] << 32 | regs[1];
                mem_size = (unsigned long)regs[2] << 32 | regs[3];
        } else if (cells == 2) {
                mem_base = regs[0];
                        mem_size = regs[1];
        } else {
                pr_err("bad number of cells in the regs property\n");
                goto failed;
        }

        pr_err("board-ramdump: Initialized rdtag stored memory at %lx-%lx\n",
                mem_base, mem_base + mem_size - 1);
        of_node_put(node);
        kfree(regs);

        return 0;
failed:
        of_node_put(node);
        kfree(regs);
        return -EINVAL;
}


static int __init rdtags_core_init(void)
{
	int err;
	ramdump_rdtag_store_init();
	rdtags_resources[0].start = mem_base;
	rdtags_resources[0].end = mem_base + (mem_size -1);
	rdtags_device.num_resources = ARRAY_SIZE(rdtags_resources);
	rdtags_device.resource = rdtags_resources;
	err = platform_driver_register(&rdtags_driver);
 	err = platform_device_register(&rdtags_device);
	return err;
}

static void __exit rdtags_module_exit(void)
{
	rdtags_initialized = 0;

	if (entry) {
		remove_proc_entry(RDTAGS_PROC_NODE_NAME, NULL);
		remove_proc_entry(RDTAGS_PROC_DIR_NAME, NULL);
		entry = NULL;
	}

	if (last_entry) {
		remove_proc_entry(LAST_RDTAGS_PROC_DIR_NAME, NULL);
		last_entry = NULL;
	}

	/* Unmap everything, but do not clear the area */
	if (rdtags_io_base) {
		iounmap(rdtags_io_base);
		rdtags_io_base = NULL;
	}

	if (last_rdtags_base) {
		free_pages((unsigned long)last_rdtags_base,
					get_order(rdtags_size));
		last_rdtags_base = NULL;
	}

	if (rdtags_base) {
		free_pages((unsigned long)rdtags_base, get_order(rdtags_size));
		rdtags_base = NULL;
	}

	platform_driver_unregister(&rdtags_driver);
}

MODULE_AUTHOR("Sony Mobile Communications");
MODULE_DESCRIPTION("ramdump tags");
MODULE_LICENSE("GPL v2");

module_init(rdtags_core_init);
module_exit(rdtags_module_exit);
