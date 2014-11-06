/*
 * Copyright (c) 2012 Sony Mobile Communications AB.
 * Copyright (c) 2013 Sony Mobile Communications AB.
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
#include <linux/crc16.h>
#include <linux/kfifo.h>
#include <linux/delay.h>

#define RDTAGS_SIG 0x47415452
#define RDTAGS_NAME_SIZE 28
#define RDTAGS_ALIGNMENT 32

#define RDTAGS_PROC_NODE_NAME	"rdtag"
#define RDTAGS_PROC_DIR_NAME	"rdtags"

static struct device *dev;
static struct proc_dir_entry *entry;
static void *rdtags_base;
static void *rdtags_end;
static size_t rdtags_size;
static spinlock_t rdlock;
static uint8_t rdtags_initialized;

struct rtag_head {
	uint32_t sig;
	uint32_t data_size;
	uint8_t name[RDTAGS_NAME_SIZE];
	uint8_t null_term; /* Shall be 0 to preserve compatibility */
	uint8_t flags;
	uint16_t crc;
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
			    ALIGN((uint32_t)x + x->data_size + \
				  sizeof(*x), RDTAGS_ALIGNMENT))

#define RDTAGS_BLK_SIZE(x) ALIGN(x->data_size + sizeof(*x), RDTAGS_ALIGNMENT)
#define RDTAGS_BASE_VALID(x) ((void *)x >= rdtags_base && \
			      (void *)x < rdtags_end)

#define CRC_SIZE 2 /* bytes */
#define RDTAG_FLAGS_CRC_PRESENT 0x1

static ssize_t tag_read(char *page, char **start, off_t off, int count,
			int *eof, void *data);

static int procfs_create_node(char *name)
{
	struct proc_dir_entry *subentry;

	if (!entry)
		return -ENXIO;

	subentry = create_proc_read_entry(name, S_IFREG | S_IRUGO, entry,
					  tag_read, NULL);
	if (!subentry)
		return -ENOMEM;

	/* subentry->data will be passed to read_proc function */
	subentry->data = (void *)subentry->name;

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
				dev_err(dev, "wq: Failed to create proc subentry \"%s\": %d\n",
					name, ret);
			break;
		case PROCFS_CMD_DELETE:
			ret = procfs_delete_node(name);
			if (ret < 0)
				dev_err(dev, "wq: Failed to remove proc subentry \"%s\": %d\n",
					name, ret);
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
		dev_err(dev, "procfs FIFO buffer overflow! procfs will be out of sync!\n");
		return;
	}
	item.cmd = cmd;
	strlcpy(item.name, name, RDTAGS_NAME_SIZE);
	dev_dbg(dev, "procfs WQ request: %s %s\n",
		cmd == PROCFS_CMD_ADD ? "ADD" : "DELETE", name);
	kfifo_put(&procfs_fifo, &item);
	schedule_work(&procfs_work);
}

/*
 * Calculates the CRC for a RDTAG. Both headers and data is covered by the CRC.
 *
 * mt: Pointer to a valid (size checked) RDTAG
 *
 * Returns: The calculated CRC
 */
static uint16_t calc_crc(struct rtag_head *mt)
{
	uint16_t crc;

	/* Calculate CRC of header (minus crc field) */
	crc = crc16(0, (char *)mt, sizeof(struct rtag_head) - CRC_SIZE);

	/* Update the crc with the data part */
	crc = crc16(crc, mt->data, mt->data_size);

	return crc;
}

/*
 * Updates a RDTAG with a correct CRC
 *
 * mt: Pointer to a RDTAG
 *
 */
static void update_crc(struct rtag_head *mt)
{
	/* First do some basic sanity checking */
	void *tag_end = (void *)(((uint32_t)mt) + sizeof(*mt) + mt->data_size);
	if (!RDTAGS_BASE_VALID(mt) ||
	    !RDTAGS_BASE_VALID((tag_end)) ||
	    (mt->data_size > rdtags_size)) {
		/* Unable to calculate CRC */
		return;
	}

	/* Update the rdtag with correct CRC value */
	mt->null_term = 0;
	mt->flags = RDTAG_FLAGS_CRC_PRESENT;

	/* Calculate the crc */
	mt->crc = calc_crc(mt);
}

/*
 * Verifies the CRC of a tag
 *
 * mt: Pointer to a RDTAG
 *
 * Returns  1      on successful CRC verification
 * Returns  0      if CRC field is missing (earlier version of RDTAGS)
 * Returns -EILSEQ if CRC is present and verfication fails, or an unexpected
 *            error is encountered.
 */
static int verify_crc(struct rtag_head *mt)
{
	uint16_t crc;
	uint16_t rdtag_crc;

	/* First do some basic sanity checking */
	void *tag_end = (void *)(((uint32_t)mt) + sizeof(*mt) + mt->data_size);
	if (!RDTAGS_BASE_VALID(mt) ||
	    !RDTAGS_BASE_VALID((tag_end)) ||
	    (mt->data_size > rdtags_size)) {
		dev_err(dev, "Tag fails basic sanity check, " \
			"clearing rest of rdtags area!\n");
		goto error;
	}

	/* Get CRC from tag */
	rdtag_crc = mt->flags & RDTAG_FLAGS_CRC_PRESENT ? mt->crc : 0;
	if (rdtag_crc == 0) {
		/* No CRC present, probably an older version of rdtags */
		return 0;
	}

	/* Calculate expected CRC and compare */
	crc = calc_crc(mt);
	if (crc == rdtag_crc)
		return 1;

	dev_err(dev, "Tag fails CRC check, suspected memory corruption, " \
		"clearing rest of rdtags area!\n");
error:
	if (RDTAGS_BASE_VALID(mt))
		memset(mt, 0x0, rdtags_end - (void *)mt);

	return -EILSEQ;
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

		/* Verify CRC of tag, if fail, it can be overwritten */
		if (verify_crc(mt) < 0)
			return mt;

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

		/* Verify CRC of tag */
		if (verify_crc(mt) < 0)
			return NULL;

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
	uint32_t size_mv, size_clr;

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
	size_mv = ((uint32_t)mt_free) - ((uint32_t)mt_next);
	size_clr = RDTAGS_BLK_SIZE(mt);

	/* Remove procfs interface asynchronously */
	PROCFS_ASYNC_DELETE(mt->name);

	/* move rear tags */
	memmove((void *)mt, (void *)mt_next, size_mv);

	/* clear vacated memory */
	if ((size_clr <= rdtags_size) &&
	    ((void *)(uint32_t)mt_free - size_clr) >= rdtags_base) {
		memset((void *)(((uint32_t)mt_free) - size_clr), 0, size_clr);
	}
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
	unsigned long flags;
	int ret = 0;

	if (!name)
		return -EINVAL;

	if (!rdtags_initialized)
		return -ENODEV;

	dev_dbg(dev, "Removing tag \"%s\"\n", name);

	spin_lock_irqsave(&rdlock, flags);

	/* Get the tag */
	mt = get_tag(name);
	if (!mt) {
		ret = -ENOENT;
		goto exit;
	}

	_remove_tag(mt);

exit:
	spin_unlock_irqrestore(&rdlock, flags);

	if (ret)
		dev_err(dev, "Could not remove tag \"%s\"\n", name);
	else
		dev_dbg(dev, "Removed tag \"%s\"\n", name);

	return ret;
}
EXPORT_SYMBOL(rdtags_remove_tag);

static int _add_tag(const char *name, const unsigned char *data, uint32_t size)
{
	struct rtag_head *mt = get_next_free();
	void *tag_end = (void *)(((uint32_t)mt) + sizeof(*mt) + size);

	if (!mt || (tag_end > rdtags_end)) {
		/* We have run out of tag space */
		dev_err(dev, "Out of tag space! Could not add tag \"%s\" " \
			"with %d bytes of data!\n", name, size);
		return -ENOMEM;
	}

	mt->sig = RDTAGS_SIG;
	strlcpy(mt->name, name, RDTAGS_NAME_SIZE);
	memcpy(mt->data, data, size);
	mt->data_size = size;
	update_crc(mt);

	/* Add procfs interface asynchronously */
	PROCFS_ASYNC_ADD(mt->name);

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
		update_crc(mt);
		return 0;
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
	unsigned long flags;
	int ret = 0;

	if (!name || !data || size == 0)
		return -EINVAL;

	if (!rdtags_initialized)
		return -ENODEV;

	dev_dbg(dev, "Adding tag \"%s\"\n", name);

	spin_lock_irqsave(&rdlock, flags);

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
	spin_unlock_irqrestore(&rdlock, flags);

	if (ret)
		dev_err(dev, "Could not add/update tag \"%s\" with %d bytes of data\n\n",
			name, size);
	else
		dev_dbg(dev, "Added/updated tag \"%s\" with %d bytes of data\n\n",
			name, size);

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
int rdtags_get_tag_data(const char *name, unsigned char *data, uint32_t *size)
{
	struct rtag_head *mt;
	unsigned long flags;
	int ret = 0;

	if (!name || !size)
		return -EINVAL;

	if (!rdtags_initialized)
		return -ENODEV;

	spin_lock_irqsave(&rdlock, flags);

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
	spin_unlock_irqrestore(&rdlock, flags);
	if (ret == -ENOBUFS)
		dev_dbg(dev, "Returning size %d for tag \"%s\"!\n",
			*size, name);
	else if (ret)
		dev_err(dev, "Could not get data for tag \"%s\": %d!\n",
			name, ret);
	else
		dev_dbg(dev, "Read data of %d bytes for tag \"%s\"!\n",
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
	unsigned long flags;

	if (!rdtags_initialized) {
		dev_err(dev, "Not yet initialized, cannot clear!\n");
		return;
	}

	dev_dbg(dev, "Clearing rdtags!\n");

	spin_lock_irqsave(&rdlock, flags);

	/* Go through all tags and remove their procfs nodes */
	while (mt->sig == RDTAGS_SIG) {
		/* Verify tag integrity */
		if (verify_crc(mt) < 0)
			break;

		/* Remove the procfs entry for the tag */
		PROCFS_ASYNC_DELETE(mt->name);

		mt = RDTAGS_NEXT_TAG(mt);
	}

	/* Finally reset the entire area to make it clean */
	memset(rdtags_base, 0x0, rdtags_size);
	spin_unlock_irqrestore(&rdlock, flags);
}
EXPORT_SYMBOL(rdtags_clear_tags);

static int rebuild_tag_tree(const unsigned char *buf, uint32_t size)
{
	struct rtag_head *mt = (struct rtag_head *)buf;
	int count = 0;

	dev_dbg(dev, "Building tag tree\n");

	while (mt && mt->sig == RDTAGS_SIG) {
		/* Verify tag integrity */
		if (verify_crc(mt) < 0)
			break;

		if (!get_valid_name(mt->name)) {
			dev_warn(dev, "Found tag with invalid name! Skipping it!\n");
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
		if (!RDTAGS_BASE_VALID(mt))
			break;
	}

	return count;
}

static ssize_t tag_read(char *page, char **start, off_t off, int count,
			int *eof, void *data)
{
	ssize_t len;
	int bufsize;
	unsigned char *buf;
	char *name = (char *)data;
	int ret;

	/* Get the size of the required data buffer */
	if (rdtags_get_tag_data(name, NULL, &bufsize) != -ENOBUFS) {
		dev_err(dev, "Could not find tag \"%s\"!\n",
			name ? name : "NULL");
		return 0;
	}

	buf = kmalloc(bufsize, GFP_KERNEL);
	if (!buf) {
		dev_err(dev, "Could not allocate %d bytes of memory!\n",
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
		dev_err(dev, "Could not get %d bytes of data for tag \"%s\": %d!\n",
			bufsize, name, ret);
		kfree(buf);
		return 0;
	}

	len = MIN(count, bufsize - off);
	memcpy(page, (void *)(buf + off), len);
	kfree(buf);
	*start = page;

	if (off + len == bufsize)
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

	/* Make sure all asynchronous work is complete before returning */
	while (!kfifo_is_empty(&procfs_fifo) && work_busy(&procfs_work)) {
		dev_dbg(dev, "Sleeping while waiting for WQ to finish\n");
		msleep(SLEEP_TIME_ASYNC_FINISH);
	}

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
	spin_lock_init(&rdlock);

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
		dev_dbg(dev, "Found existing tags in memory!\n");
		nbr_old_tags = rebuild_tag_tree(rdtags_base, rdtags_size);
	} else {
		dev_dbg(dev, "No existing tags found in memory!\n");
		/* Clear the area to be sure */
		memset(rdtags_base, 0x0, rdtags_size);
	}

	rdtags_initialized = 1;

	/* Check if the platform has specified a platform specific callback */
	platform_data = dev->platform_data;
	if (platform_data && platform_data->platform_init)
		nbr_new_tags = platform_data->platform_init();

	dev_info(dev, "Loaded with %d existing and " \
		 "%d new tags. Size: %d@0x%.8X\n",
			nbr_old_tags, nbr_new_tags, rdtags_size, res->start);

	return 0;

exit:
	if (rdtags_base) {
		iounmap(rdtags_base);
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

static int __init rdtags_core_init(void)
{
	int err;
	err = platform_driver_register(&rdtags_driver);
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
