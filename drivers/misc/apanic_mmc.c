/* drivers/misc/apanic_mmc.c
 *
 * Copyright (C) 2009 Google, Inc.
 * Author: San Mehat <san@android.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/device.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/wakelock.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h>
#include <linux/notifier.h>
#include <linux/debugfs.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <linux/preempt.h>
#include <linux/genhd.h>
#include <linux/completion.h>
#include <linux/bio.h>
#include <linux/mmc-poll/mmc_poll.h>
#include <linux/mmc-poll/mmc_poll_stack.h>
#include <linux/kmsg_dump.h>
#include "apanic_mmc.h"
#ifdef CONFIG_FB_BRCM_CP_CRASH_DUMP_IMAGE_SUPPORT
#include <video/kona_fb_image_dump.h>
#endif

#define DRVNAME	 "apanic "
#define APANIC_BLK_PATH "/dev/block"

int ap_triggered;

struct panic_header {
	u32 magic;
#define PANIC_MAGIC 0xdeadf00d

	u32 version;
#define PHDR_VERSION   0x01

	u32 console_offset;
	u32 console_length;

	u32 threads_offset;
	u32 threads_length;
};

struct apanic_data {
	int    dev_num;
	int    mmc_poll_dev_num;
	char   dev_path[256];
	struct mmc *mmc;
	struct panic_header	curr;
	void			*bounce;
	struct proc_dir_entry	*apanic_trigger;
	struct proc_dir_entry	*apanic_console;
	struct proc_dir_entry	*apanic_threads;
};

struct apanic_data drv_ctx;
static struct work_struct proc_removal_work;
static struct workqueue_struct *apanic_wq;
static DEFINE_MUTEX(drv_mutex);

void mmc_panic_copy_dev_name(char *dev_name, int dev_num)
{
	struct apanic_data *ctx = &drv_ctx;

	if (strlen(APANIC_BLK_PATH) + strlen(dev_name) <
		sizeof(ctx->dev_path))
		snprintf(ctx->dev_path, sizeof(ctx->dev_path), "%s/%s", APANIC_BLK_PATH, dev_name);
	else
		printk(KERN_ERR DRVNAME "block dev name is too long\n");

	if (dev_num >= 0)
		ctx->dev_num = dev_num;
	else
		printk(KERN_ERR DRVNAME "Couldn't find block dev number:%d\n",
		       dev_num);
}
EXPORT_SYMBOL(mmc_panic_copy_dev_name);

static void mmc_bio_complete(struct bio *bio, int err)
{
	complete((struct completion *)bio->bi_private);
}

static ssize_t apanic_proc_read(struct file *file, char __user *buffer,
				size_t count, loff_t *ppos)
{
	int i, index = 0;
	int err;
	int start_sect;
	int end_sect;
	size_t file_length;
	off_t file_offset;
	struct apanic_data *ctx = &drv_ctx;
	struct block_device *bdev;
	struct bio bio;
	struct bio_vec bio_vec;
	struct completion complete;
	struct page *page;
	off_t offset = *ppos;
	void *dat = PDE_DATA(file_inode(file));

	if (!count)
		return 0;

	mutex_lock(&drv_mutex);

	switch ((int) dat) {
	case 1:
		file_length = ctx->curr.console_length;
		file_offset = ctx->curr.console_offset;
		break;
#ifndef CONFIG_CDEBUGGER
	case 2:
		file_length = ctx->curr.threads_length;
		file_offset = ctx->curr.threads_offset;
		break;
#endif
	default:
		pr_err("bad apanic source (%d)\n", (int) dat);
		mutex_unlock(&drv_mutex);
		return -EINVAL;
	}

	/*
	 * If the requested offset is greater than or is equal to the file
	 * size, we have already reached the end of file.
	 */
	if (offset >= file_length) {
		mutex_unlock(&drv_mutex);
		return 0;
	}

	/*
	 * The bytes to read request is greater than the actual file size,
	 * so trim the request.
	 */
	if ((offset + count) > file_length)
		count = file_length - offset;

	bdev = blkdev_get_by_path(ctx->dev_path,
				  FMODE_READ, apanic_proc_read);

	if (IS_ERR(bdev)) {
		printk(KERN_ERR DRVNAME
			"failed to get block device %s (%ld)\n",
			ctx->dev_path, PTR_ERR(bdev));
		mutex_unlock(&drv_mutex);
		return -1;
	}
	page = virt_to_page(ctx->bounce);

	start_sect = (file_offset +  offset) / 512;
	end_sect = (file_offset + offset + count - 1) / 512;

	for (i = start_sect; i <= end_sect; i++) {
		bio_init(&bio);
		bio.bi_io_vec = &bio_vec;
		bio_vec.bv_page = page;
		bio_vec.bv_len = 512;
		bio_vec.bv_offset = 0;
		bio.bi_vcnt = 1;
		bio.bi_idx = 0;
		bio.bi_size = 512;
		bio.bi_bdev = bdev;
		bio.bi_sector = i;
		init_completion(&complete);
		bio.bi_private = &complete;
		bio.bi_end_io = mmc_bio_complete;
		submit_bio(READ, &bio);
		wait_for_completion(&complete);
		if (!test_bit(BIO_UPTODATE, &bio.bi_flags)) {
			err = -EIO;
			goto out_blkdev;
		}

		if ((i == start_sect) && ((file_offset + offset) % 512 != 0)) {
			/* first sect, may be the only sect */
			err = copy_to_user(buffer,
				ctx->bounce + (file_offset + offset) % 512,
				min((unsigned long)count,
				(unsigned long)
				(512 - (file_offset + offset) % 512)));
			index += min((unsigned long)count, (unsigned long)
				(512 - (file_offset + offset) % 512));
		} else if ((i == end_sect) && ((file_offset + offset + count)
			% 512 != 0)) {
			/* last sect */
			err = copy_to_user(buffer + index, ctx->bounce,
				(file_offset + offset + count) % 512);
		} else {
			/* middle sect */
			err = copy_to_user(buffer + index, ctx->bounce, 512);
			index += 512;
		}
	}

	*ppos += count;

	err = count;

out_blkdev:
	blkdev_put(bdev, FMODE_READ);

	mutex_unlock(&drv_mutex);
	return err;
}

static void mmc_panic_erase(void)
{
	int i = 0;
	struct apanic_data *ctx = &drv_ctx;
	struct block_device *bdev;
	struct bio bio;
	struct bio_vec bio_vec;
	struct completion complete;
	struct page *page;

	bdev = blkdev_get_by_path(ctx->dev_path,
				  FMODE_WRITE, mmc_panic_erase);

	if (IS_ERR(bdev)) {
		printk(KERN_ERR DRVNAME
		      "failed to get block device %s (%ld)\n",
		      ctx->dev_path, PTR_ERR(bdev));
		return;
	}

	page = virt_to_page(ctx->bounce);
	memset(ctx->bounce, 0, PAGE_SIZE);

	while (i < bdev->bd_part->nr_sects) {
		bio_init(&bio);
		bio.bi_io_vec = &bio_vec;
		bio_vec.bv_offset = 0;
		bio_vec.bv_page = page;
		bio.bi_vcnt = 1;
		bio.bi_idx = 0;
		bio.bi_sector = i;
		if (bdev->bd_part->nr_sects - i >= 8) {
			bio_vec.bv_len = PAGE_SIZE;
			bio.bi_size = PAGE_SIZE;
			i += 8;
		} else {
			bio_vec.bv_len = (bdev->bd_part->nr_sects - i) * 512;
			bio.bi_size = (bdev->bd_part->nr_sects - i) * 512;
			i = bdev->bd_part->nr_sects;
		}
		bio.bi_bdev = bdev;
		init_completion(&complete);
		bio.bi_private = &complete;
		bio.bi_end_io = mmc_bio_complete;
		submit_bio(WRITE, &bio);
		wait_for_completion(&complete);
	}
	blkdev_put(bdev, FMODE_WRITE);

	return;
}

static void apanic_remove_proc_work(struct work_struct *work)
{
	struct apanic_data *ctx = &drv_ctx;

	mutex_lock(&drv_mutex);
	mmc_panic_erase();
	memset(&ctx->curr, 0, sizeof(struct panic_header));
	if (ctx->apanic_console) {
		remove_proc_entry("apanic_console", NULL);
		ctx->apanic_console = NULL;
	}
	if (ctx->apanic_threads) {
		remove_proc_entry("apanic_threads", NULL);
		ctx->apanic_threads = NULL;
	}
	mutex_unlock(&drv_mutex);
}

static ssize_t apanic_proc_write(struct file *file, const char __user *buffer,
				size_t count, loff_t *offset)
{
	queue_work(apanic_wq, &proc_removal_work);
	return count;
}

static int in_panic;

/*
 * Copy a range of characters from the log buffer.
 */
static int log_buf_copy(char *dest, struct kmsg_dumper *dumper, size_t len)
{
	static char buf[1024]; /* static to keep left-overs for next pass */
	size_t n; /* Number of bytes requested */
	static size_t r; /* Number of bytes received */
	int total = 0;

	if (!dest || !dumper || len < 0)
		return -1;

	do {
		/* Eat left-overs first */
		while (r) {
			n = min(r, len);
			memcpy(dest, buf, n);
			dest += n;
			len -= n;
			total += n;
			r -= n;
			if (!len) {
				/* Move left-overs (if any) to &buf[0] */
				if (r)
					memmove(buf, &buf[n], r);
				return total;
			}
		}
		/* Get new data into the buffer */
	} while (kmsg_dump_get_line_nolock(dumper, true, buf, sizeof(buf), &r));
	return total;
}

/*
 * Writes the contents of the console to the specified offset in flash.
 * Returns number of bytes written
 */
static int apanic_write_console_mmc(unsigned long off)
{
	struct apanic_data *ctx = &drv_ctx;
	int saved_oip;
	int idx = 0;
	size_t rc;
	int rc2;
	unsigned int last_chunk = 0;
	unsigned long start;
	unsigned long partition_end = get_apanic_end_address();
	struct kmsg_dumper dumper = { .active = true };

	start = off;
	kmsg_dump_rewind_nolock(&dumper);
	while (!last_chunk) {
		saved_oip = oops_in_progress;
		oops_in_progress = 1;

		/*
		 * bounce buffer is reserved during init time.
		 * Now copy 'writable' amount of data from __log_buf
		 * to the bounce buffer
		 */
		rc = log_buf_copy(ctx->bounce, &dumper, ctx->mmc->write_bl_len);
		if (rc < 0)
			break;

		if (rc != ctx->mmc->write_bl_len)
			last_chunk = rc;

		oops_in_progress = saved_oip;
		if (rc <= 0)
			break;
		if (rc != ctx->mmc->write_bl_len)
			memset(ctx->bounce + rc, 0,
			       ctx->mmc->write_bl_len - rc);

		if (off >= partition_end) {
			pr_err("ERROR %s: Write across the partition boundary\n"
				, __func__);
			pr_err("off = %lu end = %lu\n", off, partition_end);
			return -ENOSPC;
		}

		/* Write the bounce buffer to eMMC */
		rc2 = ctx->mmc->block_dev.block_write(ctx->mmc_poll_dev_num,
						      off, 1, ctx->bounce);
		if (rc2 <= 0) {
			printk(KERN_EMERG
			       "apanic: MMC write failed (%d)\n", rc2);
			return idx;
		}

		/* idx is a byte offset used to copy from log buf */
		if (!last_chunk)
			idx += (rc2 * ctx->mmc->write_bl_len);
		else
			idx += last_chunk;

		/*
		 * off is in terms of block count to tell the mmc driver where
		 * to start the next write from, while idx is in terms of
		 * bytes to tell the lob_buf_copy where to start the read
		 * from. Note that the block_write function of mmc also
		 * returns the number of blocks written.
		 */
		off += rc2;
	}

	pr_debug("%s: wrote %d bytes from %ld returned %ld\r\n",
		__func__, idx, start, off);

	return idx;
}

static int apanic(struct notifier_block *this, unsigned long event,
			void *ptr)
{
	struct apanic_data *ctx = &drv_ctx;
	struct panic_header *hdr = (struct panic_header *) ctx->bounce;
	int console_offset = 0;
	int console_len = 0;
	int console_len_bytes = 0;
#ifndef CONFIG_CDEBUGGER
	int threads_offset = 0;
	int threads_len = 0;
	int threads_len_bytes = 0;
#endif
	int rc;
	unsigned long blk;
	unsigned long partition_end = get_apanic_end_address();

	ap_triggered = 1;

	if (in_panic)
		return NOTIFY_DONE;
	in_panic = 1;
#ifdef CONFIG_PREEMPT
	/* Ensure that cond_resched() won't try to preempt anybody */
	add_preempt_count(PREEMPT_ACTIVE);
#endif
	touch_softlockup_watchdog();

	blk = get_apanic_start_address();
	if (blk == 0) {
		printk("apanic: Invalid block number \r\n");
		goto out;
	}

	if (mmc_poll_stack_init((void **)&ctx->mmc,
				ctx->dev_num,
				&ctx->mmc_poll_dev_num) < 0) {
		pr_err("apanic: Unable to init polling mode mmc stack\n");
		goto out;
	}

	pr_debug("apanic: MMC device write block size is %d \r\n",
		 ctx->mmc->write_bl_len);

	/*
	 * The first block is reserved for apanic header so copy the log_buf
	 * from start_block + 1
	 */
	console_offset = blk + 1;

	pr_debug("Log Buff is stored from block number %d \r\n",
		console_offset);
	console_len_bytes = apanic_write_console_mmc(console_offset);
	if (console_len_bytes <= 0) {
		printk(KERN_EMERG "Error writing log but to panic log! (%d)\n",
			console_len_bytes);
		console_len_bytes = 0;
	}
	console_len = (console_len_bytes+ctx->mmc->write_bl_len-1) /
		      ctx->mmc->write_bl_len;

#ifndef CONFIG_CDEBUGGER
	/*
	 * Write out all threads
	 */
	threads_offset = console_offset + console_len;

	pr_debug("Thread data is stored from block number %d \r\n",
		 threads_offset);

	/* Below two functions are removed by Linaro so commented out.
	* It will generate the compilation errors if ramdump is not enabled.
	* If ramdump is not supported but need to support APANIC thread info
	* then APANIC rework is required to use RAM.c instead of Android ram
	* console. Also need to rework log_bug_clear() */
#ifdef CONFIG_ANDROID_RAM_CONSOLE
	ram_console_enable_console(0);
	log_buf_clear();
#endif
	show_state_filter(0);

	threads_len_bytes = apanic_write_console_mmc(threads_offset);
	if (threads_len_bytes <= 0) {
		printk(KERN_EMERG "Error writing threads to panic log! (%d)\n",
		       threads_len_bytes);
		threads_len_bytes = 0;
	}
	threads_len = (threads_len_bytes+ctx->mmc->write_bl_len-1) /
		      ctx->mmc->write_bl_len;
#endif
	/*
	 * Finally write the panic header at the first block of the partition
	 */
	memset(ctx->bounce, 0, PAGE_SIZE);
	hdr->magic = PANIC_MAGIC;
	hdr->version = PHDR_VERSION;

	/*
	 * blk - Holds the starting block number of the apanic partition
	 * console_offset - Holds the block number from where console data is
	 *		     placed
	 * console_len    - Holds the size of console data stored in terms of
	 *                   number of blocks
	 *
	 * threads_offset, threads_length same as above but for threads info.
	 * This info is converted into bytes before storing the header, so
	 * when the application reads this header to go to console or to
	 * thread info it has to simply 'lseek' as many bytes.
	 */
	hdr->console_offset = (console_offset-blk)*ctx->mmc->write_bl_len;
	hdr->console_length = console_len_bytes;

#ifndef CONFIG_CDEBUGGER
	hdr->threads_offset = (threads_offset-blk)*ctx->mmc->write_bl_len;
	hdr->threads_length = (threads_len)*ctx->mmc->write_bl_len;
#endif
	pr_debug("apanic: writing the header at block %ld\n", blk);

	if (blk >= partition_end) {
		pr_err("ERROR %s: Write across the partition boundary\n",
			__func__);
		return -ENOSPC;
	}

	rc = ctx->mmc->block_dev.block_write(ctx->mmc_poll_dev_num,
					     blk, 1, ctx->bounce);
	if (rc == 0) {
		printk(KERN_EMERG "apanic: Header write failed (%d)\n", rc);
		goto out;
	}

	pr_debug("kona_mmc_poll_write: bock_read returned %d \r\n", rc);

	pr_info("apanic: Panic dump successfully written to flash \r\n");

 out:
#ifdef CONFIG_PREEMPT
	sub_preempt_count(PREEMPT_ACTIVE);
#endif
	in_panic = 0;
	return NOTIFY_DONE;
}

static struct notifier_block panic_blk = {
	.notifier_call	= apanic,
	.priority = 2,
};

static int panic_dbg_get(void *data, u64 *val)
{
	apanic(NULL, 0, NULL);
	return 0;
}

static int panic_dbg_set(void *data, u64 val)
{
	BUG();
	return -1;
}

DEFINE_SIMPLE_ATTRIBUTE(panic_dbg_fops, panic_dbg_get, panic_dbg_set, "%llu\n");

static const struct file_operations proc_apanic_fops = {
	.read		= apanic_proc_read,
	.write		= apanic_proc_write,
};

static ssize_t apanic_trigger_check(struct file *file,
				    const char __user *devpath,
				    size_t count, loff_t *ppos)
{
	struct apanic_data *ctx = &drv_ctx;
	struct panic_header *hdr = ctx->bounce;
	struct block_device *bdev;
	struct bio bio;
	struct bio_vec bio_vec;
	struct completion complete;
	struct page *page;
	char *copy_devpath;
	char *user_dev_path = NULL;
	int ret;

	/* Allocate memory to store the path name passed from user */
	/* Allocate an extra byte for storing the NULL character */
	user_dev_path = kmalloc(count+1, GFP_KERNEL);
	if (NULL == user_dev_path) {
		printk(KERN_ERR DRVNAME "Unable to allocate memory \r\n");
		ret = -1;
		goto out;
	}

	if (copy_from_user((void *)user_dev_path, devpath, count)) {
		printk(KERN_ERR DRVNAME "Unable to copy from user space \r\n");
		ret = -EFAULT;
		goto out;
	}

	*(user_dev_path + count) = 0;

	/*
	 * The logic is written in such a way that if the user passes "auto"
	 * the apanic driver should pickup the device file name from
	 * ctx->dev_path.
	 * Note that ctx->dev_path gets populated from function
	 * mmc_panic_copy_dev_name. In this fucntion to the device name
	 * "dev/block" is
	 * hardcoded. Note that for Android based systems its OK. The Android
	 * udev creates the device node for mmcblkxx under /dev/block.
	 * But for systems where mdev (reduced udev) is used the node gets
	 * created as /dev/mmcblxxx.
	 *
	 * So because of this the android init scripts can pass "auto" to get
	 * the correct file name, but if the same option is passed on systems
	 * where mdev is used, apanic driver will not be able to find the
	 * device node. For mdev based systems lets pass the device node name
	 * from the script. And in that case we'll update ctx->dev_path as
	 * well, so from else where when the device node file name is accessed
	 * we'll get the correct one.
	 */
	if (!strcmp(user_dev_path, "auto"))
		copy_devpath = ctx->dev_path;
	else {
		copy_devpath = (char *)user_dev_path;
		strncpy(ctx->dev_path, copy_devpath, sizeof(ctx->dev_path) - 1);
	}

	bdev = blkdev_get_by_path(copy_devpath,
				  FMODE_READ, apanic_trigger_check);

	if (IS_ERR(bdev)) {
		printk(KERN_ERR DRVNAME
		      "failed to get block device %s (%ld)\n",
		      copy_devpath, PTR_ERR(bdev));
		ret = -1;
		goto out;
	}

	if (!strlen(ctx->dev_path))
		strncpy(ctx->dev_path, copy_devpath, sizeof(ctx->dev_path)-1);

	page = virt_to_page(ctx->bounce);

	bio_init(&bio);
	bio.bi_io_vec = &bio_vec;
	bio_vec.bv_page = page;
	bio_vec.bv_len = PAGE_SIZE;
	bio_vec.bv_offset = 0;
	bio.bi_vcnt = 1;
	bio.bi_idx = 0;
	bio.bi_size = PAGE_SIZE;
	bio.bi_bdev = bdev;
	bio.bi_sector = 0;
	init_completion(&complete);
	bio.bi_private = &complete;
	bio.bi_end_io = mmc_bio_complete;
	submit_bio(READ, &bio);
	wait_for_completion(&complete);

	blkdev_put(bdev, FMODE_READ);
	printk(KERN_ERR DRVNAME "using block device '%s'\n", copy_devpath);

	if (hdr->magic != PANIC_MAGIC) {
		printk(KERN_INFO DRVNAME "no panic data available\n");
		ret = -1;
		goto out;
	}

	if (hdr->version != PHDR_VERSION) {
		printk(KERN_INFO DRVNAME "version mismatch (%d != %d)\n",
		       hdr->version, PHDR_VERSION);
		ret = -1;
		goto out;
	}

	memcpy(&ctx->curr, hdr, sizeof(struct panic_header));

	printk(KERN_INFO DRVNAME "c(%u, %u) t(%u, %u)\n",
	       hdr->console_offset, hdr->console_length,
	       hdr->threads_offset, hdr->threads_length);

	if (hdr->console_length) {
		ctx->apanic_console = proc_create_data("apanic_console",
						      S_IFREG | S_IRUGO, NULL,
						      &proc_apanic_fops,
						      (void *)1);
		if (!ctx->apanic_console)
			printk(KERN_ERR DRVNAME "failed creating procfile\n");
		else
			proc_set_size(ctx->apanic_console, hdr->console_length);
	}

#ifndef CONFIG_CDEBUGGER
	if (hdr->threads_length) {
		ctx->apanic_threads = proc_create_data("apanic_threads",
						       S_IFREG | S_IRUGO, NULL,
						       &proc_apanic_fops,
						       (void *)2);
		if (!ctx->apanic_threads)
			printk(KERN_ERR DRVNAME "failed creating procfile\n");
		else
			proc_set_size(ctx->apanic_threads, hdr->threads_length);
	}
#endif

	ret = count;
out:
	if (user_dev_path != NULL)
		kfree(user_dev_path);
	return ret;
}

static const struct file_operations proc_apanic_trigger_fops = {
	.write		= apanic_trigger_check,
	.llseek		= noop_llseek,
};

int __init apanic_init(void)
{

	apanic_wq = alloc_workqueue("apanic_wq", WQ_FREEZABLE | WQ_UNBOUND, 1);
	if (!apanic_wq) {
		pr_err("%s: Failed to create workqueue\n", __func__);
		return -ENOMEM;
	}
	atomic_notifier_chain_register(&panic_notifier_list, &panic_blk);
	debugfs_create_file("apanic", 0644, NULL, NULL, &panic_dbg_fops);
	memset(&drv_ctx, 0, sizeof(drv_ctx));
	drv_ctx.bounce = (void *) __get_free_page(GFP_KERNEL);
	INIT_WORK(&proc_removal_work, apanic_remove_proc_work);

	drv_ctx.apanic_trigger = proc_create("apanic",
						   S_IWUSR | S_IWGRP, NULL,
						   &proc_apanic_trigger_fops);
	if (!drv_ctx.apanic_trigger)
		printk(KERN_ERR "%s: failed creating procfile\n", __func__);
	else
		proc_set_size(drv_ctx.apanic_trigger, 1);

	printk(KERN_INFO "Android kernel panic handler initialized (bind=%s)\n",
	       CONFIG_APANIC_PLABEL);

	return 0;
}

module_init(apanic_init);

