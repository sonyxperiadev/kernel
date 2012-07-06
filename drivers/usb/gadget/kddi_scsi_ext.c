 /* drivers/usb/gadget/kddi_scsi_ext.c
 *
 * KDDI USB SCSI Command Extension
 *
 * Copyright (C) 2012 Sony Ericsson Mobile Communications AB.
 * Copyright (C) 2012 Sony Mobile Communications AB.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/delay.h>
#include <linux/list.h>
#include <linux/skbuff.h>
#include <linux/poll.h>
#include <linux/atomic.h>
#include <asm/byteorder.h>
#include <linux/uaccess.h>
#include <linux/pagemap.h>

#define SC_KDDI_CMD00			0xe4
#define SC_KDDI_CMD01			0xe5
#define SC_KDDI_CMD02			0xe6
#define SC_KDDI_CMD03			0xe7
#define SC_KDDI_CMD04			0xe8
#define SC_KDDI_CMD05			0xe9
#define SC_KDDI_CMD06			0xea
#define SC_KDDI_CMD07			0xeb
#define SC_KDDI_CMD08			0xec
#define SC_KDDI_CMD09			0xed
#define SC_KDDI_CMD10			0xee
#define SC_KDDI_CMD11			0xef
#define SC_KDDI_START			SC_KDDI_CMD00
#define SC_KDDI_END			SC_KDDI_CMD11
#define KDDI_CMD_NR	(SC_KDDI_END - SC_KDDI_START + 1)
#define INQUIRY_KDDI_INIT		"LISMOSC1"

/* Size of InquiryResponse VendorSpecific */
#define INQUIRY_KDDI_SPECIFIC_SIZE	20

#define KDDI_ALLOC_INI_SIZE		0x101000
#define KDDI_ALLOC_CMD_CNT		1

struct kddi_op_desc {
	struct device	dev;
	unsigned long	flags;
/* flag symbols are bit numbers */
#define FLAG_IS_READ	0
#define FLAG_IS_WRITE	1
#define FLAG_EXPORT	2 /* protected by sysfs_lock */

	char			*buffer;
	size_t			len;
	struct bin_attribute	dev_bin_attr_buffer;
	unsigned long		update;
	struct work_struct	work;
	struct sysfs_dirent	*value_sd;
};

struct kddi_data {
	struct kddi_op_desc *op_desc[KDDI_CMD_NR];
	/* KDDI Vendor specific and NUL byte */
	char inquiry_kddi_ext[INQUIRY_KDDI_SPECIFIC_SIZE + 1];
	char *reserve_buf[KDDI_CMD_NR];
};

static int check_command(struct fsg_common *common, int cmnd_size,
		enum data_direction data_dir, unsigned int mask,
		int needs_medium, const char *name);

static void op_release(struct device *dev)
{
}

static DEFINE_MUTEX(sysfs_lock);

static struct kddi_op_desc *dev_to_desc(struct device *dev)
{
	return container_of(dev, struct kddi_op_desc, dev);
}

/* setting notify change buffer */
static void buffer_notify_sysfs(struct work_struct *work)
{
	struct kddi_op_desc	*desc;

	desc = container_of(work, struct kddi_op_desc, work);
	sysfs_notify_dirent(desc->value_sd);
}

/* check KDDI extension command code */
static int kddi_ext_is_valid(unsigned cmd)
{
	return (cmd < SC_KDDI_START || cmd > SC_KDDI_END) ? 0 : 1;
}

/* read KDDI extension command buffer */
static ssize_t
kddi_ext_read_buffer(struct file *f, struct kobject *kobj,
		     struct bin_attribute *attr, char *buf,
		     loff_t off, size_t count)
{
	ssize_t	status;
	struct kddi_op_desc	*desc = attr->private;

	mutex_lock(&sysfs_lock);

	if (!test_bit(FLAG_EXPORT, &desc->flags)) {
		status = -EIO;
	} else {
		size_t srclen, n;
		void *src;
		size_t nleft = count;
		src = desc->buffer;
		srclen = desc->len;

		if (off < srclen) {
			n = min(nleft, srclen - (size_t) off);
			memcpy(buf, src + off, n);
			nleft -= n;
			buf += n;
			off = 0;
		} else {
			off -= srclen;
		}
		status = count - nleft;
	}

	mutex_unlock(&sysfs_lock);
	return status;
}

/* write KDDI extension command buffer */
static ssize_t
kddi_ext_write_buffer(struct file *f, struct kobject *kobj,
		      struct bin_attribute *attr, char *buf,
		      loff_t off, size_t count)
{
	ssize_t	status;
	struct kddi_op_desc	*desc = attr->private;

	mutex_lock(&sysfs_lock);

	if (!test_bit(FLAG_EXPORT, &desc->flags)) {
		status = -EIO;
	} else {
		size_t dstlen, n;
		size_t nleft = count;
		void *dst;

		dst = desc->buffer;
		dstlen = desc->len;

		if (off < dstlen) {
			n = min(nleft, dstlen - (size_t) off);
			memcpy(dst + off, buf, n);
			nleft -= n;
			buf += n;
			off = 0;
		} else {
			off -= dstlen;
		}
		status = count - nleft;
	}

	desc->update = jiffies;
	schedule_work(&desc->work);

	mutex_unlock(&sysfs_lock);
	return status;
}

/* memory mapping KDDI extension command buffer */
static int
kddi_ext_mmap_buffer(struct file *f, struct kobject *kobj,
		     struct bin_attribute *attr, struct vm_area_struct *vma)
{
	int rc = -EINVAL;
	unsigned long pgoff, delta;
	ssize_t size = vma->vm_end - vma->vm_start;
	struct kddi_op_desc	*desc = attr->private;

	printk(KERN_INFO "%s\n", __func__);

	mutex_lock(&sysfs_lock);

	if (vma->vm_pgoff != 0) {
		printk(KERN_ERR "mmap failed: page offset %lx\n",
			vma->vm_pgoff);
		goto done;
	}

	pgoff = __pa(desc->buffer);
	delta = PAGE_ALIGN(pgoff) - pgoff;
	printk(KERN_INFO "%s size=%x delta=%lx pgoff=%lx\n", __func__, size,
		delta, pgoff);

	if (size + delta > desc->len) {
		printk(KERN_ERR "mmap failed: size %d\n", size);
		goto done;
	}

	pgoff += delta;
	vma->vm_flags |= VM_RESERVED;

	rc = io_remap_pfn_range(vma, vma->vm_start, pgoff >> PAGE_SHIFT,
		size, vma->vm_page_prot);

	if (rc < 0)
		printk(KERN_ERR "mmap failed: remap error %d\n", rc);
done:
	mutex_unlock(&sysfs_lock);
	return rc;
}


/* set 'size'file */
static ssize_t kddi_ext_size_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct kddi_op_desc	*desc = dev_to_desc(dev);
	ssize_t		status;

	mutex_lock(&sysfs_lock);

	if (!test_bit(FLAG_EXPORT, &desc->flags))
		status = -EIO;
	else
		status = snprintf(buf, PAGE_SIZE, "%d\n", desc->len);

	mutex_unlock(&sysfs_lock);
	return status;
}

/* when update 'size'file */
static ssize_t kddi_ext_size_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	long len;
	char *buffer;
	struct kddi_op_desc	*desc = dev_to_desc(dev);
	ssize_t		status;
	long cmd;
	char cmd_buf[5] = "0x";
	struct fsg_lun *curlun = fsg_lun_from_dev(dev);

	mutex_lock(&sysfs_lock);

	if (!test_bit(FLAG_EXPORT, &desc->flags)) {
		status = -EIO;
	} else {
		struct bin_attribute *dev_bin_attr_buffer =
			&desc->dev_bin_attr_buffer;
		status = strict_strtol(buf, 0, &len);
		if (status < 0) {
			status = -EINVAL;
			goto done;
		}
		if (desc->len == len) {
			status = 0;
			goto done;
		}
		status = strict_strtol(strncat(cmd_buf,
					dev_name(&desc->dev) + 7,
					2), 0, &cmd);
		if (status < 0) {
			status = -EINVAL;
			goto done;
		}
		if (cmd - SC_KDDI_START < KDDI_ALLOC_CMD_CNT &&
		    len == KDDI_ALLOC_INI_SIZE) {
			buffer = curlun->kddi_data->reserve_buf[cmd -
								SC_KDDI_START];
		} else {
			buffer = kzalloc(len, GFP_KERNEL);
			if (!buffer) {
				status = -ENOMEM;
				goto done;
			}
		}
		if (cmd - SC_KDDI_START + 1 > KDDI_ALLOC_CMD_CNT ||
		    desc->len != KDDI_ALLOC_INI_SIZE)
			kfree(desc->buffer);
		desc->len = len;
		desc->buffer = buffer;
		device_remove_bin_file(&desc->dev, dev_bin_attr_buffer);
		dev_bin_attr_buffer->size = len;
		status = device_create_bin_file(&desc->dev,
						dev_bin_attr_buffer);
	}

done:
	mutex_unlock(&sysfs_lock);
	return status ? : size;
}
/* define 'size'file */
static DEVICE_ATTR(size, 0606, kddi_ext_size_show, kddi_ext_size_store);

/* set 'update'file */
static ssize_t kddi_ext_update_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct kddi_op_desc	*desc = dev_to_desc(dev);
	ssize_t		status;

	mutex_lock(&sysfs_lock);

	if (!test_bit(FLAG_EXPORT, &desc->flags))
		status = -EIO;
	else
		status = snprintf(buf, PAGE_SIZE, "%lu\n", desc->update);

	mutex_unlock(&sysfs_lock);
	return status;
}
/* define 'update'file */
static DEVICE_ATTR(update, 0404, kddi_ext_update_show, 0);

static int kddi_ext_export(struct device *dev, unsigned cmd, int init)
{
	struct fsg_lun	*curlun = fsg_lun_from_dev(dev);
	struct kddi_op_desc	*desc;
	int		status = -EINVAL;
	struct bin_attribute *dev_bin_attr_buffer;

	if (!kddi_ext_is_valid(cmd))
		goto done;

	desc = curlun->kddi_data->op_desc[cmd - SC_KDDI_START];
	if (!desc) {
		desc = kzalloc(sizeof(*desc), GFP_KERNEL);
		if (!desc) {
			status = -ENOMEM;
			goto done;
		}
		curlun->kddi_data->op_desc[cmd - SC_KDDI_START] = desc;
	}

	status = 0;
	if (test_bit(FLAG_EXPORT, &desc->flags))
		goto done;

	if (cmd - SC_KDDI_START + 1 > KDDI_ALLOC_CMD_CNT) {
		desc->buffer = kzalloc(2048, GFP_KERNEL);
		if (!desc->buffer) {
			status = -ENOMEM;
			goto done;
		}
		desc->len = 2048;
	} else {
		desc->buffer = curlun->kddi_data->reserve_buf[cmd -
							      SC_KDDI_START];
		desc->len = KDDI_ALLOC_INI_SIZE;
	}

	dev_bin_attr_buffer = &desc->dev_bin_attr_buffer;
	desc->dev.release = op_release;
	desc->dev.parent = &curlun->dev;
	dev_set_drvdata(&desc->dev, curlun);
	dev_set_name(&desc->dev, "opcode-%02x", cmd);
	status = device_register(&desc->dev);
	if (status != 0) {
		printk(KERN_INFO "failed to register opcode%d: %d\n",
			cmd, status);
		if (cmd - SC_KDDI_START + 1 > KDDI_ALLOC_CMD_CNT)
			kfree(desc->buffer);
		desc->buffer = NULL;
		desc->len = 0;
		goto done;
	}

	dev_bin_attr_buffer->attr.name = "buffer";
	if (init)
		dev_bin_attr_buffer->attr.mode = 0660;
	else
		dev_bin_attr_buffer->attr.mode = 0606;
	dev_bin_attr_buffer->read = kddi_ext_read_buffer;
	dev_bin_attr_buffer->write = kddi_ext_write_buffer;
	dev_bin_attr_buffer->mmap = kddi_ext_mmap_buffer;
	if (cmd - SC_KDDI_START + 1 > KDDI_ALLOC_CMD_CNT)
		dev_bin_attr_buffer->size = 2048;
	else
		dev_bin_attr_buffer->size = KDDI_ALLOC_INI_SIZE;
	dev_bin_attr_buffer->private = desc;
	status = device_create_bin_file(&desc->dev,
					dev_bin_attr_buffer);

	if (status != 0) {
		device_remove_bin_file(&desc->dev, dev_bin_attr_buffer);
		if (cmd - SC_KDDI_START + 1 > KDDI_ALLOC_CMD_CNT)
			kfree(desc->buffer);
		desc->buffer = NULL;
		desc->len = 0;
		device_unregister(&desc->dev);
		goto done;
	}

	if (init) {
		dev_attr_size.attr.mode = 0660;
		dev_attr_update.attr.mode = 0440;
	} else {
		dev_attr_size.attr.mode = 0606;
		dev_attr_update.attr.mode = 0404;
	}

	status = device_create_file(&desc->dev, &dev_attr_size);
	if (status == 0)
		status = device_create_file(&desc->dev,
						&dev_attr_update);
	if (status != 0) {
		printk(KERN_INFO "device_create_file failed: %d\n",
			status);
		device_remove_file(&desc->dev, &dev_attr_update);
		device_remove_file(&desc->dev, &dev_attr_size);
		device_remove_bin_file(&desc->dev, dev_bin_attr_buffer);
		if (cmd - SC_KDDI_START + 1 > KDDI_ALLOC_CMD_CNT)
			kfree(desc->buffer);
		desc->buffer = NULL;
		desc->len = 0;
		device_unregister(&desc->dev);
		goto done;
	}

	desc->value_sd = sysfs_get_dirent(desc->dev.kobj.sd, NULL, "update");
	INIT_WORK(&desc->work, buffer_notify_sysfs);

	if (status == 0)
		set_bit(FLAG_EXPORT, &desc->flags);

	desc->update = 0;
done:
	if (status)
		pr_debug("%s: opcode%d status %d\n", __func__, cmd, status);
	return status;
}

static void kddi_ext_unexport(struct device *dev, unsigned cmd)
{
	struct fsg_lun	*curlun = fsg_lun_from_dev(dev);
	struct kddi_op_desc *desc;
	int status = -EINVAL;

	if (!kddi_ext_is_valid(cmd))
		goto done;

	desc = curlun->kddi_data->op_desc[cmd - SC_KDDI_START];
	if (!desc) {
		status = -ENODEV;
		goto done;
	}

	if (test_bit(FLAG_EXPORT, &desc->flags)) {
		struct bin_attribute *dev_bin_attr_buffer =
			&desc->dev_bin_attr_buffer;
		clear_bit(FLAG_EXPORT, &desc->flags);
		cancel_work_sync(&desc->work);
		device_remove_file(&desc->dev, &dev_attr_update);
		device_remove_file(&desc->dev, &dev_attr_size);
		device_remove_bin_file(&desc->dev, dev_bin_attr_buffer);
		if (cmd - SC_KDDI_START + 1 > KDDI_ALLOC_CMD_CNT ||
		    desc->len != KDDI_ALLOC_INI_SIZE)
			kfree(desc->buffer);
		desc->buffer = NULL;
		desc->len = 0;
		status = 0;
		device_unregister(&desc->dev);
		kfree(desc);
		curlun->kddi_data->op_desc[cmd - SC_KDDI_START] = NULL;
	} else {
		status = -ENODEV;
	}

done:
	if (status)
		pr_debug("%s: opcode%d status %d\n", __func__, cmd, status);
}

/* when 'export'file update */
static ssize_t kddi_ext_export_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t len)
{
	long cmd;
	int status;

	status = strict_strtol(buf, 0, &cmd);
	if (status < 0)
		goto done;

	status = -EINVAL;

	if (!kddi_ext_is_valid(cmd))
		goto done;

	mutex_lock(&sysfs_lock);

	status = kddi_ext_export(dev, cmd, 0);
	if (status < 0)
		kddi_ext_unexport(dev, cmd);

	mutex_unlock(&sysfs_lock);
done:
	if (status)
		pr_debug(KERN_INFO"%s: status %d\n", __func__, status);
	return status ? : len;
}

/* define 'export'file */
static DEVICE_ATTR(export, 0220, 0, kddi_ext_export_store);

/* when 'unexport'file update */
static ssize_t kddi_ext_unexport_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t len)
{
	long cmd;
	int status;

	status = strict_strtol(buf, 0, &cmd);
	if (status < 0)
		goto done;

	status = -EINVAL;

	if (!kddi_ext_is_valid(cmd))
		goto done;

	mutex_lock(&sysfs_lock);

	status = 0;
	kddi_ext_unexport(dev, cmd);

	mutex_unlock(&sysfs_lock);
done:
	if (status)
		pr_debug(KERN_INFO"%s: status %d\n", __func__, status);
	return status ? : len;
}
/* define 'unexport'file */
static DEVICE_ATTR(unexport, 0220, 0, kddi_ext_unexport_store);

/* set 'inquiry'file */
static ssize_t kddi_ext_inquiry_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct fsg_lun *curlun = fsg_lun_from_dev(dev);
	ssize_t status;

	mutex_lock(&sysfs_lock);
	status = snprintf(buf, PAGE_SIZE,
			"\"%s\"\n", curlun->kddi_data->inquiry_kddi_ext);

	mutex_unlock(&sysfs_lock);
	return status;
}

/* get 'inquiry'file */
static ssize_t kddi_ext_inquiry_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t len)
{
	struct fsg_lun *curlun = fsg_lun_from_dev(dev);

	mutex_lock(&sysfs_lock);
	strncpy(curlun->kddi_data->inquiry_kddi_ext, buf,
		sizeof(curlun->kddi_data->inquiry_kddi_ext));
	curlun->kddi_data->inquiry_kddi_ext[sizeof(
		curlun->kddi_data->inquiry_kddi_ext) - 1] = '\0';

	mutex_unlock(&sysfs_lock);
	return 0;
}

/* define 'inquiry'file */
static DEVICE_ATTR(inquiry, 0660, kddi_ext_inquiry_show,
		kddi_ext_inquiry_store);

static int kddi_scsi_ext_do_read_buffer(struct fsg_common *common)
{
	struct fsg_lun		*curlun = common->curlun;
	struct fsg_buffhd	*bh;
	int			rc;
	u32			amount_left;
	loff_t			file_offset;
	unsigned int		amount;
	struct kddi_op_desc		*desc = NULL;

	file_offset = get_unaligned_be32(&common->cmnd[2]);

	/* Get the starting Logical Block Address and check that it's
	 * not too big */
	desc = curlun->kddi_data->op_desc[common->cmnd[0] - SC_KDDI_START];
	if (!desc->buffer) {
		printk(KERN_ERR "%s: cmd=%d not ready\n", __func__,
			common->cmnd[0]);
		curlun->sense_data =
				SS_LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE;
		curlun->sense_data_info = file_offset;
		curlun->info_valid = 1;
		return -EIO;		/* No default reply */
	}


	/* Carry out the file reads */
	amount_left = common->data_size_from_cmnd;

	if (unlikely(amount_left == 0))
		return -EIO;		/* No default reply */


	for (;;) {
		/* Figure out how much we need to read:
		 * Try to read the remaining amount.
		 * But don't read more than the buffer size.
		 * And don't try to read past the end of the file.
		 * Finally, if we're not at a page boundary, don't read past
		 *	the next page.
		 * If this means reading 0 then we were asked to read past
		 *	the end of file. */
		amount = min(amount_left, FSG_BUFLEN);
		amount = min((loff_t) amount, desc->len - file_offset);

		/* Wait for the next buffer to become available */
		bh = common->next_buffhd_to_fill;
		while (bh->state != BUF_STATE_EMPTY) {
			rc = sleep_thread(common);
			if (rc)
				return rc;
		}

		/* If we were asked to read past the end of file,
		 * end with an empty buffer. */
		if (amount == 0) {
			curlun->sense_data =
					SS_LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE;
			curlun->sense_data_info = file_offset;
			curlun->info_valid = 1;
			bh->inreq->length = 0;
			bh->state = BUF_STATE_FULL;
			break;
		}

		memcpy((char __user *) bh->buf, desc->buffer + file_offset,
			amount);

		file_offset  += amount;
		amount_left  -= amount;
		common->residue -= amount;
		bh->inreq->length = amount;
		bh->state = BUF_STATE_FULL;

		if (amount_left == 0)
			break;		/* No more left to read */

		/* Send this buffer and go read some more */
		if (!start_in_transfer(common, bh))
			return -EIO;
		common->next_buffhd_to_fill = bh->next;
	}
	return -EIO;		/* No default reply */
}

static int kddi_scsi_ext_do_write_buffer(struct fsg_common *common)
{
	struct fsg_lun		*curlun = common->curlun;
	struct fsg_buffhd	*bh;
	int			get_some_more;
	u32			amount_left_to_req, amount_left_to_write;
	loff_t			file_offset;
	unsigned int		amount;
	int			rc;
	struct kddi_op_desc		*desc = NULL;

	get_some_more = 1;
	file_offset = get_unaligned_be32(&common->cmnd[2]);

	desc = curlun->kddi_data->op_desc[common->cmnd[0] - SC_KDDI_START];
	if (!desc->buffer) {
		printk(KERN_ERR "%s: cmd=%d not ready\n", __func__,
			common->cmnd[0]);
		curlun->sense_data =
				SS_LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE;
		curlun->sense_data_info = file_offset;
		curlun->info_valid = 1;
		return -EIO;		/* No default reply */
	}

	amount_left_to_req = amount_left_to_write = common->data_size_from_cmnd;
	if (file_offset + amount_left_to_write > desc->len) {
		printk(KERN_ERR "%s: vendor buffer out of range offset=0x%x"
		       "write-len=0x%x buf-len=0x%x\n", __func__,
		       (unsigned int)file_offset, amount_left_to_req,
		       desc->len);
		curlun->sense_data =
				SS_LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE;
		curlun->sense_data_info = file_offset;
		curlun->info_valid = 1;
		return -EIO;		/* No default reply */
	}

	while (amount_left_to_write > 0) {

		/* Queue a request for more data from the host */
		bh = common->next_buffhd_to_fill;
		if (bh->state == BUF_STATE_EMPTY && get_some_more) {

			/* Figure out how much we want to get:
			 * Try to get the remaining amount.
			 * But don't get more than the buffer size.
			 * And don't try to go past the end of the file.
			 * If we're not at a page boundary,
			 *	don't go past the next page.
			 * If this means getting 0, then we were asked
			 *	to write past the end of file.
			 * Finally, round down to a block boundary. */
			amount = min(amount_left_to_req, FSG_BUFLEN);

			/* Get the next buffer */
			common->usb_amount_left -= amount;
			amount_left_to_req -= amount;
			if (amount_left_to_req == 0)
				get_some_more = 0;

			/* amount is always divisible by 512, hence by
			 * the bulk-out maxpacket size */
			bh->outreq->length = bh->bulk_out_intended_length =
					amount;
			if (!start_out_transfer(common, bh))
				return -EIO;
			common->next_buffhd_to_fill = bh->next;
			continue;
		}

		/* Write the received data to the backing file */
		bh = common->next_buffhd_to_drain;
		if (bh->state == BUF_STATE_EMPTY && !get_some_more)
			break;			/* We stopped early */
		if (bh->state == BUF_STATE_FULL) {
			smp_rmb();
			common->next_buffhd_to_drain = bh->next;
			bh->state = BUF_STATE_EMPTY;

			/* Did something go wrong with the transfer? */
			if (bh->outreq->status != 0) {
				curlun->sense_data = SS_COMMUNICATION_FAILURE;
				curlun->sense_data_info = file_offset >> 9;
				curlun->info_valid = 1;
				break;
			}

			amount = bh->outreq->actual;
			if (desc->len - file_offset < amount) {
				LERROR(curlun,
					"write %u @ %llu beyond end %llu\n",
					amount,
					(unsigned long long) file_offset,
					(unsigned long long) desc->len);
				amount = desc->len - file_offset;
			}

			/* Perform the write */
			memcpy(desc->buffer + file_offset,
				(char __user *) bh->buf, amount);
			file_offset += amount;
			amount_left_to_write -= amount;
			common->residue -= amount;

#ifdef MAX_UNFLUSHED_BYTES
			curlun->unflushed_bytes += amount;
			if (curlun->unflushed_bytes >= MAX_UNFLUSHED_BYTES) {
				fsync_sub(curlun);
				curlun->unflushed_bytes = 0;
			}
#endif
			/* Did the host decide to stop early? */
			if (bh->outreq->actual != bh->outreq->length) {
				common->short_packet_received = 1;
				break;
			}
			continue;
		}

		/* Wait for something to happen */
		rc = sleep_thread(common);
		if (rc)
			return rc;
	}

	return -EIO;		/* No default reply */
}

static int kddi_scsi_ext_fin(struct fsg_lun *curlun)
{
	int i;

	if (curlun == NULL)
		return -EINVAL;

	if (curlun->cdrom)
		return 0;

	if (curlun->kddi_data == NULL)
		return 0;

	for (i = SC_KDDI_START; i < SC_KDDI_END + 1; i++) {
		kddi_ext_unexport(&curlun->dev, i);
		if (i - SC_KDDI_START < KDDI_ALLOC_CMD_CNT)
			kfree(curlun->kddi_data->reserve_buf[i -
							     SC_KDDI_START]);
	}

	device_remove_file(&curlun->dev, &dev_attr_export);
	device_remove_file(&curlun->dev, &dev_attr_unexport);
	device_remove_file(&curlun->dev, &dev_attr_inquiry);
	kfree(curlun->kddi_data);
	curlun->kddi_data = NULL;

	return 0;
}

static int kddi_scsi_ext_init(struct fsg_lun *curlun)
{
	int i;
	int rc = 0;

	if (curlun == NULL)
		return -EINVAL;

	if (curlun->cdrom)
		return 0;

	curlun->kddi_data = kzalloc(sizeof(*curlun->kddi_data), GFP_KERNEL);
	if (!curlun->kddi_data)
		return -ENOMEM;

	rc = device_create_file(&curlun->dev, &dev_attr_export);
	if (rc)
		goto fin;
	rc = device_create_file(&curlun->dev, &dev_attr_unexport);
	if (rc)
		goto fin;
	rc = device_create_file(&curlun->dev, &dev_attr_inquiry);
	if (rc)
		goto fin;

	memset(curlun->kddi_data->inquiry_kddi_ext, 0,
	       sizeof(curlun->kddi_data->inquiry_kddi_ext));
	strncpy(curlun->kddi_data->inquiry_kddi_ext, INQUIRY_KDDI_INIT,
		sizeof(INQUIRY_KDDI_INIT));

	for (i = 0; i < KDDI_ALLOC_CMD_CNT; i++) {
		curlun->kddi_data->reserve_buf[i] =
		    kzalloc(KDDI_ALLOC_INI_SIZE, GFP_KERNEL);
		if (!curlun->kddi_data->reserve_buf[i]) {
			rc = -ENOMEM;
			goto fin;
		}
	}

	rc = kddi_ext_export(&curlun->dev, 0xe4, 1);
	if (rc < 0) {
		kddi_ext_unexport(&curlun->dev, 0xe4);
		goto fin;
	}

	return 0;
fin:
	kddi_scsi_ext_fin(curlun);

	return rc;
}

static int kddi_scsi_ext_do_cmd(struct fsg_common *common)
{
	int	reply = -EINVAL;
	struct kddi_op_desc	*desc;
	char		unknown[16];

	if (common->curlun->cdrom)
		goto cmd_error;

	mutex_lock(&sysfs_lock);
	if (strncmp(dev_name(&common->curlun->dev), "lun0", 4))
		goto cmd_error;

	desc = common->luns[common->lun].kddi_data->
	    op_desc[common->cmnd[0] - SC_KDDI_START];
	if (!desc)
		goto cmd_error;

	common->data_size_from_cmnd = get_unaligned_be32(&common->cmnd[6]);
	if (common->data_size_from_cmnd == 0)
		goto cmd_error;
	if (~common->cmnd[1] & 0x10) {
		reply = check_command(common, 10, DATA_DIR_FROM_HOST,
				      (1 << 1) | (0xf << 2) | (0xf << 6), 0,
				      "VENDOR WRITE BUFFER");
		if (reply == 0) {
			reply = kddi_scsi_ext_do_write_buffer(common);
			desc->update = jiffies;
			schedule_work(&desc->work);
		} else {
			goto cmd_error;
		}
		mutex_unlock(&sysfs_lock);
		return reply;
	} else {
		reply = check_command(common, 10, DATA_DIR_TO_HOST,
				      (1 << 1) | (0xf << 2) | (0xf << 6), 0,
				      "VENDOR READ BUFFER");
		if (reply == 0)
			reply = kddi_scsi_ext_do_read_buffer(common);
		else
			goto cmd_error;
		mutex_unlock(&sysfs_lock);
		return reply;
	}
cmd_error:
	mutex_unlock(&sysfs_lock);
	common->data_size_from_cmnd = 0;
	snprintf(unknown, sizeof(unknown), "Unknown x%02x", common->cmnd[0]);
	reply = check_command(common, common->cmnd_size,
			      DATA_DIR_UNKNOWN, 0x3ff, 0, unknown);
	if (reply == 0) {
		common->curlun->sense_data = SS_INVALID_COMMAND;
		reply = -EINVAL;
	}
	return reply;
}
