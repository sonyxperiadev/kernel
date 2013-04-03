/*
 * Copyright (c) 2012 Sony Mobile Communications AB.
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
#include <linux/rdtags.h>

#define RDTAGS_SIG 0x47415452
#define RDTAGS_NAME_SIZE 32
#define RDTAGS_ALIGNMENT 32

#define RDTAGS_PROC_NODE_NAME	"rdtag"
#define RDTAGS_PROC_DIR_NAME	"rdtags"

static struct device *dev;
static struct proc_dir_entry *entry;
static void *rdtags_base;
static void *rdtags_end;
static size_t rdtags_size;
static struct mutex mutex;

struct rtag_head {
	uint32_t sig;
	uint32_t data_size;
	uint8_t name[RDTAGS_NAME_SIZE];
	uint8_t data[0];
};

#define MAX(X, Y) ((X) >= (Y) ? (X) : (Y))
#define MIN(X, Y) ((X) <= (Y) ? (X) : (Y))

#define RDTAGS_NEXT_TAG(x) ((struct rtag_head *) \
			    ALIGN((uint32_t)x + x->data_size + \
				  sizeof(*x), RDTAGS_ALIGNMENT))

#define RDTAGS_BLK_SIZE(x) ALIGN(x->data_size + sizeof(*x), RDTAGS_ALIGNMENT)

static ssize_t tag_read(char *page, char **start, off_t off, int count,
			int *eof, void *data);

static char *get_valid_name(char *name)
{
	char *temp;

	if (!name)
		return NULL;

	if (name[0] == 0x0)
		return NULL;

	/* Make sure the name is not longer than RDTAGS_NAME_SIZE */
	if (RDTAGS_NAME_SIZE == strnlen(name, RDTAGS_NAME_SIZE))
		name[RDTAGS_NAME_SIZE - 1] = 0x0;

	/* Check the name for disallowed characters */
	temp = strpbrk(name, "\r\n ");
	if (temp)
		*temp = 0x0;

	/* Check that the name actually contains something */
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
		if ((void *)mt < rdtags_base || (void *)mt >= rdtags_end)
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
		if ((void *)mt < rdtags_base || (void *)mt >= rdtags_end)
			return NULL;
	}

	return NULL;
}

static void _remove_tag(struct rtag_head *mt)
{
	struct rtag_head *mt_next, *mt_free;
	uint32_t size_mv, size_clr;

	/*
	 * we need to get some address & size for recompacting tags:
	 *                            |- size_mv -|
	 * --------------------------------------------------------------
	 * |rdtags_base|...|    mt    |mt_next|...|mt_free|...|rdtags_end|
	 * --------------------------------------------------------------
	 *                 |-size_clr-|
	 */
	mt_next = RDTAGS_NEXT_TAG(mt);
	mt_free = get_next_free();
	if (NULL == mt_free)
		mt_free = (struct rtag_head *)rdtags_end;
	size_mv = ((uint32_t)mt_free) - ((uint32_t)mt_next);
	size_clr = RDTAGS_BLK_SIZE(mt);

	/* Remove procfs interface */
	remove_proc_entry(mt->name, entry);

	/* move rear tags */
	memmove((void *)mt, (void *)mt_next, size_mv);

	/* clear vacated memory */
	memset((void *)(((uint32_t)mt_free) - size_clr), 0, size_clr);

	return;
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

	if (!rdtags_base)
		return -ENODEV;

	mutex_lock(&mutex);

	/* Get the tag */
	mt = get_tag(name);
	if (!mt) {
		dev_err(dev, "Could not find tag \"%s\"\n", name);
		ret = -ENOENT;
		goto exit;
	}

	dev_dbg(dev, "Removing tag \"%s\"\n", name);

	_remove_tag(mt);

exit:
	mutex_unlock(&mutex);

	return ret;
}
EXPORT_SYMBOL(rdtags_remove_tag);

static int _add_tag(const char *name, const unsigned char* data, uint32_t size)
{
	struct rtag_head *mt = get_next_free();
	struct proc_dir_entry *subentry;
	void *tag_end = (void *)(((uint32_t)mt) + sizeof(*mt) + size);

	if (!mt || (tag_end > rdtags_end)) {
		/* We have run out of tag space */
		dev_err(dev, "Out of tag space! Could not add tag \"%s\" with "
			"%d bytes of data!\n", name, size);
		return -ENOMEM;
	}

	mt->sig = RDTAGS_SIG;
	strlcpy(mt->name, name, RDTAGS_NAME_SIZE);
	mt->name[RDTAGS_NAME_SIZE - 1] = 0x0;	/* Null terminate */
	memcpy(mt->data, data, size);
	mt->data_size = size;

	/* Add procfs interface */
	subentry = create_proc_read_entry(mt->name, S_IFREG | S_IRUGO, entry,
					  tag_read, NULL);
	if (!subentry) {
		dev_err(dev, "Failed to create proc subentry \"%s\"\n",
			mt->name);
		memset((void *)mt, 0, size + sizeof(struct rtag_head));
		return -ENOMEM;
	}

	/* subentry->data will be passed to read_proc function */
	subentry->data = (void *)subentry->name;

	return 0;
}

static int _update_tag(struct rtag_head *mt, const unsigned char *data,
		       uint32_t size)
{
	char name[RDTAGS_NAME_SIZE];
	struct rtag_head *mt_free;
	uint32_t free_size;

	if (!mt || mt->sig != RDTAGS_SIG) {
		dev_err(dev, "Not a valid RTAG!\n");
		return -ENOENT;
	}

	/* If the size is identical, we can update the previous tag */
	if (size == mt->data_size) {
		dev_dbg(dev, "Updating tag \"%s\"\n", mt->name);
		memcpy(mt->data, data, size);
		return size;
	}

	/* compute available memory size if recompact tags */
	free_size = RDTAGS_BLK_SIZE(mt);
	mt_free = get_next_free();
	if (NULL == mt_free)
		mt_free = (struct rtag_head *)rdtags_end;
	free_size += ((uint32_t)rdtags_end) - ((uint32_t)mt_free);

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
		   const uint32_t size)
{
	struct rtag_head *mt;
	int ret = 0;

	if (!name || !data || size == 0)
		return -EINVAL;

	if (!rdtags_base)
		return -ENODEV;

	mutex_lock(&mutex);

	/* First check if the tag exists */
	mt = get_tag(name);

	if (mt != NULL) {
		/* Update the tag */
		ret = _update_tag(mt, data, size);
		goto exit;
	}

	/* Add the tag */
	dev_dbg(dev, "Adding tag \"%s\"\n", name);
	ret = _add_tag(name, data, size);
exit:
	mutex_unlock(&mutex);

	return ret;
}
EXPORT_SYMBOL(rdtags_add_tag);

/*
 * Gets the data from a tag
 *
 * name: Name of tag to get data from
 * *size: Will be updated with the size of the data
 *
 * Returns a pointer to the data or NULL on failure
 */
unsigned char *rdtags_get_tag_data(const char *name, uint32_t *size)
{
	struct rtag_head *mt;

	mutex_lock(&mutex);

	if (!name || !size || !rdtags_base)
		goto error;

	/* First check if the tag exists */
	mt = get_tag(name);

	if (mt == NULL)
		goto error;

	/* Set the size and return a pointer to the data */
	*size = mt->data_size;
	mutex_unlock(&mutex);
	return mt->data;
error:
	if (size)
		*size = 0;
	mutex_unlock(&mutex);

	return NULL;
}
EXPORT_SYMBOL(rdtags_get_tag_data);

/*
 * Clears all tags and "reformats" the entire tag area
 */
void rdtags_clear_tags(void)
{
	struct rtag_head *mt = (struct rtag_head *)rdtags_base;

	if (!rdtags_base)
		return;

	mutex_lock(&mutex);
	dev_dbg(dev, "Clearing rdtags!\n");

	/* Go through all tags and remove them */
	while (mt->sig == RDTAGS_SIG) {
		/* Remove the tag */
		_remove_tag(mt);
	}

	/* Finally reset the entire area to make it clean */
	memset(rdtags_base, 0x0, rdtags_size);
	mutex_unlock(&mutex);
}
EXPORT_SYMBOL(rdtags_clear_tags);

static int rebuild_tag_tree(const unsigned char *buf, uint32_t size)
{
	struct rtag_head *mt = (struct rtag_head *)buf;
	int count = 0;

	dev_dbg(dev, "Building tag tree\n");

	while (mt && mt->sig == RDTAGS_SIG) {
		struct proc_dir_entry *subentry;
		void *tag_end =
			(void *)(((uint32_t)mt) + sizeof(*mt) + mt->data_size);
		/* Do some basic sanity checking */
		if (!get_valid_name(mt->name) || (tag_end > rdtags_end)) {
			dev_err(dev, "Found broken tag, clearing rest of "
				"rdtags area!\n");
			memset(mt, 0x0, rdtags_end - (void *)mt);
			break;
		}

		dev_dbg(dev, "   Found tag: \"%s\" - with %d bytes of data\n",
			mt->name, mt->data_size);

		/* Add procfs interface */
		subentry = create_proc_read_entry(mt->name, S_IFREG | S_IRUGO,
						  entry, tag_read, NULL);
		if (!subentry) {
			dev_err(dev, "Failed to create proc subentry \"%s\"\n",
				mt->name);
			goto loop_next;
		}

		/* reassign subentry->data because of compact memory */
		subentry->data = (void *)subentry->name;

		count++;

loop_next:

		/* Go to next tag */
		mt = RDTAGS_NEXT_TAG(mt);

		/* Check that we are not outside the buffer */
		if ((void *)mt < rdtags_base || (void *)mt >= rdtags_end)
			break;
	}

	return count;
}

static ssize_t tag_read(char *page, char **start, off_t off, int count,
			int *eof, void *data)
{
	struct rtag_head *mt = get_tag((char *)data);
	ssize_t len;

	if (!mt || mt->sig != RDTAGS_SIG) {
		dev_err(dev, "Not a valid RTAG!\n");
		return 0;
	}

	len = MIN(count, mt->data_size - off);
	memcpy(page, (void *)(mt->data + off), len);
	*start = page;

	if (off + len == mt->data_size)
		*eof = 1;

	return len;
}

static ssize_t tags_read(char *page, char **start, off_t off,
			   int count, int *eof, void *data)
{
	/* Assume that this text always fits in count bytes */
	char *message = "Usage: <tag name/command> [tag data]\n"
		"\n"
		"command may be one of the following:\n"
		" * clear\n"
		" * delete <tag name>\n";

	*start = page;
	*eof = 1;
	if (off == 0)
		return snprintf(page, count, "%s", message);

	return 0;
}

static int tags_write(struct file *file, const char *buffer,
			unsigned long count, void *data)
{
	char *tag_data;
	int tag_size;
	void *kbuf;

	if (0 == count)
		return 0;

	kbuf = kmalloc(count + 1, GFP_KERNEL);
	if (NULL == kbuf) {
		dev_err(dev, "Unable to assign memory.\n");
		return -ENOMEM;
	}

	/* Copy to kernel space */
	tag_size = copy_from_user(kbuf, buffer, count);

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
	tag_data = strnchr(kbuf, ' ', RDTAGS_NAME_SIZE);

	if (!tag_data) {
		dev_err(dev, "Incorrect format, please supply a string of "
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
	 * get_valid_name(kbuf) has guaranteed that
	 * strlen(kbuf) < RDTAGS_NAME_SIZE
	 */
	tag_size = count - (strnlen(kbuf, RDTAGS_NAME_SIZE - 1) + 1);

	/* Check for special dual commands */
	if (strncmp(kbuf, "delete", RDTAGS_NAME_SIZE) == 0) {
		/* Do some basic sanity checking */
		if (!get_valid_name(tag_data))
			goto exit;

		/* Remove the tag */
		rdtags_remove_tag(tag_data);
		goto exit;
	}

	/* Add the tag */
	rdtags_add_tag(kbuf, tag_data, tag_size);
exit:
	kfree(kbuf);

	return count;
}

static int rdtags_driver_probe(struct platform_device *pdev)
{
	struct resource *res;
	struct rtag_head *mt;
	struct rdtags_platform_data *platform_data;
	int nbr_old_tags = 0;
	int nbr_new_tags = 0;
	int ret = 0;

	dev = &pdev->dev;
	mutex_init(&mutex);
	mutex_lock(&mutex);

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
	rdtags_base = (void *)ioremap(res->start, rdtags_size);
	if (!rdtags_base) {
		dev_err(dev, "Failed to map %d bytes of memory at 0x%.8X!\n",
			rdtags_size, res->start);
		ret = -EINVAL;
		goto exit;
	}
	rdtags_end = rdtags_base + rdtags_size;

	/* Add procfs interface */
	entry = create_proc_entry(RDTAGS_PROC_NODE_NAME,
				  S_IFREG | S_IRUGO | S_IWUSR,
				  NULL);
	if (!entry) {
		dev_err(dev, "Failed to create proc entry\n");
		ret = -ENOMEM;
		goto exit;
	}
	entry->read_proc = tags_read;
	entry->write_proc = tags_write;

	/* Create procfs directory to hold the tags */
	entry = proc_mkdir(RDTAGS_PROC_DIR_NAME, NULL);

	/* Check if the buffer contains a valid start tag */
	mt = (struct rtag_head *)rdtags_base;
	if (mt->sig == RDTAGS_SIG) {
		dev_dbg(dev, "Found existing valid tags in memory!\n");
		nbr_old_tags = rebuild_tag_tree(rdtags_base, rdtags_size);
	} else {
		dev_dbg(dev, "No existing tags found in memory!\n");
		/* Clear the area to be sure */
		memset(rdtags_base, 0x0, rdtags_size);
	}

	mutex_unlock(&mutex);

	/* Check if the platform has specified a platform specific callback */
	platform_data = dev->platform_data;
	if (platform_data && platform_data->platform_init)
		nbr_new_tags = platform_data->platform_init();

	dev_info(dev, "Loaded with %d existing and " \
		 "%d new tags. Size: %d\n",
			nbr_old_tags, nbr_new_tags, rdtags_size);

	return 0;

exit:
	if (rdtags_base) {
		iounmap(rdtags_base);
		rdtags_base = NULL;
	}
	mutex_unlock(&mutex);

	return ret;
}

static struct platform_driver rdtags_driver = {
	.probe = rdtags_driver_probe,
	.driver		= {
		.name	= "rdtags",
	},
};

static int __init rdtags_core_init(void)
{
	int err;
	err = platform_driver_register(&rdtags_driver);
	return err;
}

static void __exit rdtags_module_exit(void)
{
	if (entry) {
		remove_proc_entry(RDTAGS_PROC_NODE_NAME, NULL);
		remove_proc_entry(RDTAGS_PROC_DIR_NAME, NULL);
		entry = NULL;
	}

	/* Unmap everything, but do not clear the area */
	if (rdtags_base) {
		iounmap(rdtags_base);
		rdtags_base = NULL;
	}

	platform_driver_unregister(&rdtags_driver);
}

MODULE_AUTHOR("Sony Mobile Communications");
MODULE_DESCRIPTION("ramdump tags");
MODULE_LICENSE("GPL V2");

core_initcall(rdtags_core_init);
module_exit(rdtags_module_exit);
