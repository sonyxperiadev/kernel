/* drivers/misc/apanic.c
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
#include <linux/mtd/mtd.h>
#include <linux/notifier.h>
#include <linux/mtd/mtd.h>
#include <linux/debugfs.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/mutex.h>
#include <linux/workqueue.h>
#include <linux/preempt.h>

#ifdef CONFIG_APANIC_ON_MMC
#include <linux/mmc-poll/mmc_poll_stack.h>

/* Enable this to debug some apanic code for MMC */
/* #define APANIC_DEBUG */
#ifdef APANIC_DEBUG
#define apanic_print(a,b...) printk(a,##b)
#else
#define apanic_print(a,b...)
#endif

#endif


extern void ram_console_enable_console(int);

#ifdef CONFIG_APANIC_ON_MMC
extern unsigned long get_apanic_start_address(void);
extern int mmc_poll_stack_init (struct mmc **mmc, int *dev_num); 
#endif

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
	struct mtd_info		*mtd;
#ifdef CONFIG_APANIC_ON_MMC
	unsigned long dev_num;	
	struct mmc *mmc;
#endif
	struct panic_header	curr;
	void			*bounce;
	struct proc_dir_entry	*apanic_console;
	struct proc_dir_entry	*apanic_threads;
};

static struct apanic_data drv_ctx;
static struct work_struct proc_removal_work;
static DEFINE_MUTEX(drv_mutex);

static unsigned int *apanic_bbt;
static unsigned int apanic_erase_blocks;
static unsigned int apanic_good_blocks;

static void set_bb(unsigned int block, unsigned int *bbt)
{
	unsigned int flag = 1;

	BUG_ON(block >= apanic_erase_blocks);

	flag = flag << (block%32);
	apanic_bbt[block/32] |= flag;
	apanic_good_blocks--;
}

static unsigned int get_bb(unsigned int block, unsigned int *bbt)
{
	unsigned int flag;

	BUG_ON(block >= apanic_erase_blocks);

	flag = 1 << (block%32);
	return apanic_bbt[block/32] & flag;
}

static void alloc_bbt(struct mtd_info *mtd, unsigned int *bbt)
{
	int bbt_size;
	apanic_erase_blocks = (mtd->size)>>(mtd->erasesize_shift);
	bbt_size = (apanic_erase_blocks+32)/32;

	apanic_bbt = kmalloc(bbt_size*4, GFP_KERNEL);
	memset(apanic_bbt, 0, bbt_size*4);
	apanic_good_blocks = apanic_erase_blocks;
}
static void scan_bbt(struct mtd_info *mtd, unsigned int *bbt)
{
	int i;

	for (i = 0; i < apanic_erase_blocks; i++) {
		if (mtd->block_isbad(mtd, i*mtd->erasesize))
			set_bb(i, apanic_bbt);
	}
}

#define APANIC_INVALID_OFFSET 0xFFFFFFFF

static unsigned int phy_offset(struct mtd_info *mtd, unsigned int offset)
{
	unsigned int logic_block = offset>>(mtd->erasesize_shift);
	unsigned int phy_block;
	unsigned good_block = 0;

	for (phy_block = 0; phy_block < apanic_erase_blocks; phy_block++) {
		if (!get_bb(phy_block, apanic_bbt))
			good_block++;
		if (good_block == (logic_block + 1))
			break;
	}

	if (good_block != (logic_block + 1))
		return APANIC_INVALID_OFFSET;

	return offset + ((phy_block-logic_block)<<mtd->erasesize_shift);
}

static void apanic_erase_callback(struct erase_info *done)
{
	wait_queue_head_t *wait_q = (wait_queue_head_t *) done->priv;
	wake_up(wait_q);
}

static int apanic_proc_read(char *buffer, char **start, off_t offset,
			       int count, int *peof, void *dat)
{
	struct apanic_data *ctx = &drv_ctx;
	size_t file_length;
	off_t file_offset;
	unsigned int page_no;
	off_t page_offset;
	int rc;
	size_t len;

	if (!count)
		return 0;

	mutex_lock(&drv_mutex);

	switch ((int) dat) {
	case 1:	/* apanic_console */
		file_length = ctx->curr.console_length;
		file_offset = ctx->curr.console_offset;
		break;
	case 2:	/* apanic_threads */
		file_length = ctx->curr.threads_length;
		file_offset = ctx->curr.threads_offset;
		break;
	default:
		pr_err("Bad dat (%d)\n", (int) dat);
		mutex_unlock(&drv_mutex);
		return -EINVAL;
	}

	if ((offset + count) > file_length) {
		mutex_unlock(&drv_mutex);
		return 0;
	}

	/* We only support reading a maximum of a flash page */
	if (count > ctx->mtd->writesize)
		count = ctx->mtd->writesize;

	page_no = (file_offset + offset) / ctx->mtd->writesize;
	page_offset = (file_offset + offset) % ctx->mtd->writesize;

	if (phy_offset(ctx->mtd, (page_no * ctx->mtd->writesize))
		== APANIC_INVALID_OFFSET) {
		pr_err("apanic: reading an invalid address\n");
		mutex_unlock(&drv_mutex);
		return -EINVAL;
	}
	rc = ctx->mtd->read(ctx->mtd,
		phy_offset(ctx->mtd, (page_no * ctx->mtd->writesize)),
		ctx->mtd->writesize,
		&len, ctx->bounce);

	if (page_offset)
		count -= page_offset;
	memcpy(buffer, ctx->bounce + page_offset, count);

	*start = count;

	if ((offset + count) == file_length)
		*peof = 1;

	mutex_unlock(&drv_mutex);
	return count;
}

static void mtd_panic_erase(void)
{
	struct apanic_data *ctx = &drv_ctx;
	struct erase_info erase;
	DECLARE_WAITQUEUE(wait, current);
	wait_queue_head_t wait_q;
	int rc, i;

	init_waitqueue_head(&wait_q);
	erase.mtd = ctx->mtd;
	erase.callback = apanic_erase_callback;
	erase.len = ctx->mtd->erasesize;
	erase.priv = (u_long)&wait_q;
	for (i = 0; i < ctx->mtd->size; i += ctx->mtd->erasesize) {
		erase.addr = i;
		set_current_state(TASK_INTERRUPTIBLE);
		add_wait_queue(&wait_q, &wait);

		if (get_bb(erase.addr>>ctx->mtd->erasesize_shift, apanic_bbt)) {
			printk(KERN_WARNING
			       "apanic: Skipping erase of bad "
			       "block @%llx\n", erase.addr);
			set_current_state(TASK_RUNNING);
			remove_wait_queue(&wait_q, &wait);
			continue;
		}

		rc = ctx->mtd->erase(ctx->mtd, &erase);
		if (rc) {
			set_current_state(TASK_RUNNING);
			remove_wait_queue(&wait_q, &wait);
			printk(KERN_ERR
			       "apanic: Erase of 0x%llx, 0x%llx failed\n",
			       (unsigned long long) erase.addr,
			       (unsigned long long) erase.len);
			if (rc == -EIO) {
				if (ctx->mtd->block_markbad(ctx->mtd,
							    erase.addr)) {
					printk(KERN_ERR
					       "apanic: Err marking blk bad\n");
					goto out;
				}
				printk(KERN_INFO
				       "apanic: Marked a bad block"
				       " @%llx\n", erase.addr);
				set_bb(erase.addr>>ctx->mtd->erasesize_shift,
					apanic_bbt);
				continue;
			}
			goto out;
		}
		schedule();
		remove_wait_queue(&wait_q, &wait);
	}
	printk(KERN_DEBUG "apanic: %s partition erased\n",
	       CONFIG_APANIC_PLABEL);
out:
	return;
}

static void apanic_remove_proc_work(struct work_struct *work)
{
	struct apanic_data *ctx = &drv_ctx;

	mutex_lock(&drv_mutex);
	mtd_panic_erase();
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

static int apanic_proc_write(struct file *file, const char __user *buffer,
				unsigned long count, void *data)
{
	schedule_work(&proc_removal_work);
	return count;
}

static void mtd_panic_notify_add(struct mtd_info *mtd)
{
	struct apanic_data *ctx = &drv_ctx;
	struct panic_header *hdr = ctx->bounce;
	size_t len;
	int rc;
	int    proc_entry_created = 0;

	if (strcmp(mtd->name, CONFIG_APANIC_PLABEL))
		return;

	ctx->mtd = mtd;

	alloc_bbt(mtd, apanic_bbt);
	scan_bbt(mtd, apanic_bbt);

	if (apanic_good_blocks == 0) {
		printk(KERN_ERR "apanic: no any good blocks?!\n");
		goto out_err;
	}

	rc = mtd->read(mtd, phy_offset(mtd, 0), mtd->writesize,
			&len, ctx->bounce);
	if (rc && rc == -EBADMSG) {
		printk(KERN_WARNING
		       "apanic: Bad ECC on block 0 (ignored)\n");
	} else if (rc && rc != -EUCLEAN) {
		printk(KERN_ERR "apanic: Error reading block 0 (%d)\n", rc);
		goto out_err;
	}

	if (len != mtd->writesize) {
		printk(KERN_ERR "apanic: Bad read size (%d)\n", rc);
		goto out_err;
	}

	printk(KERN_INFO "apanic: Bound to mtd partition '%s'\n", mtd->name);

	if (hdr->magic != PANIC_MAGIC) {
		printk(KERN_INFO "apanic: No panic data available\n");
		mtd_panic_erase();
		return;
	}

	if (hdr->version != PHDR_VERSION) {
		printk(KERN_INFO "apanic: Version mismatch (%d != %d)\n",
		       hdr->version, PHDR_VERSION);
		mtd_panic_erase();
		return;
	}

	memcpy(&ctx->curr, hdr, sizeof(struct panic_header));

	printk(KERN_INFO "apanic: c(%u, %u) t(%u, %u)\n",
	       hdr->console_offset, hdr->console_length,
	       hdr->threads_offset, hdr->threads_length);

	if (hdr->console_length) {
		ctx->apanic_console = create_proc_entry("apanic_console",
						      S_IFREG | S_IRUGO, NULL);
		if (!ctx->apanic_console)
			printk(KERN_ERR "%s: failed creating procfile\n",
			       __func__);
		else {
			ctx->apanic_console->read_proc = apanic_proc_read;
			ctx->apanic_console->write_proc = apanic_proc_write;
			ctx->apanic_console->size = hdr->console_length;
			ctx->apanic_console->data = (void *) 1;
			proc_entry_created = 1;
		}
	}

	if (hdr->threads_length) {
		ctx->apanic_threads = create_proc_entry("apanic_threads",
						       S_IFREG | S_IRUGO, NULL);
		if (!ctx->apanic_threads)
			printk(KERN_ERR "%s: failed creating procfile\n",
			       __func__);
		else {
			ctx->apanic_threads->read_proc = apanic_proc_read;
			ctx->apanic_threads->write_proc = apanic_proc_write;
			ctx->apanic_threads->size = hdr->threads_length;
			ctx->apanic_threads->data = (void *) 2;
			proc_entry_created = 1;
		}
	}

	if (!proc_entry_created)
		mtd_panic_erase();

	return;
out_err:
	ctx->mtd = NULL;
}

static void mtd_panic_notify_remove(struct mtd_info *mtd)
{
	struct apanic_data *ctx = &drv_ctx;
	if (mtd == ctx->mtd) {
		ctx->mtd = NULL;
		printk(KERN_INFO "apanic: Unbound from %s\n", mtd->name);
	}
}

static struct mtd_notifier mtd_panic_notifier = {
	.add	= mtd_panic_notify_add,
	.remove	= mtd_panic_notify_remove,
};

static int in_panic = 0;

static int apanic_writeflashpage(struct mtd_info *mtd, loff_t to,
				 const u_char *buf)
{
	int rc;
	size_t wlen;
	int panic = in_interrupt() | in_atomic();

	if (panic && !mtd->panic_write) {
		printk(KERN_EMERG "%s: No panic_write available\n", __func__);
		return 0;
	} else if (!panic && !mtd->write) {
		printk(KERN_EMERG "%s: No write available\n", __func__);
		return 0;
	}

	to = phy_offset(mtd, to);
	if (to == APANIC_INVALID_OFFSET) {
		printk(KERN_EMERG "apanic: write to invalid address\n");
		return 0;
	}

	if (panic)
		rc = mtd->panic_write(mtd, to, mtd->writesize, &wlen, buf);
	else
		rc = mtd->write(mtd, to, mtd->writesize, &wlen, buf);

	if (rc) {
		printk(KERN_EMERG
		       "%s: Error writing data to flash (%d)\n",
		       __func__, rc);
		return rc;
	}

	return wlen;
}

extern int log_buf_copy(char *dest, int idx, int len);
extern void log_buf_clear(void);

/*
 * Writes the contents of the console to the specified offset in flash.
 * Returns number of bytes written
 */
static int apanic_write_console(struct mtd_info *mtd, unsigned int off)
{
	struct apanic_data *ctx = &drv_ctx;
	int saved_oip;
	int idx = 0;
	int rc, rc2;
	unsigned int last_chunk = 0;

	while (!last_chunk) {
		saved_oip = oops_in_progress;
		oops_in_progress = 1;
		rc = log_buf_copy(ctx->bounce, idx, mtd->writesize);
		if (rc < 0)
			break;

		if (rc != mtd->writesize)
			last_chunk = rc;

		oops_in_progress = saved_oip;
		if (rc <= 0)
			break;
		if (rc != mtd->writesize)
			memset(ctx->bounce + rc, 0, mtd->writesize - rc);

		rc2 = apanic_writeflashpage(mtd, off, ctx->bounce);
		if (rc2 <= 0) {
			printk(KERN_EMERG
			       "apanic: Flash write failed (%d)\n", rc2);
			return idx;
		}
		if (!last_chunk)
			idx += rc2;
		else
			idx += last_chunk;
		off += rc2;
	}
	return idx;
}

#ifdef CONFIG_APANIC_ON_MMC

static int apanic_write_console_mmc (unsigned long off)
{
	struct apanic_data *ctx = &drv_ctx;
	int saved_oip;
	int idx = 0;
	int rc, rc2;
	unsigned int last_chunk = 0;
	unsigned long num = 0;

#ifdef APANIC_DEBUG
	unsigned long start; 
	start = off; 
#endif
	while (!last_chunk) {
		saved_oip = oops_in_progress;
		oops_in_progress = 1;

		/*
		 * bounce buffer is reserved during init time.
		 * Now copy 'writable' amount of data from __log_buf
		 * to the bounce buffer
		 */
		rc = log_buf_copy(ctx->bounce, idx, ctx->mmc->write_bl_len);
		if (rc < 0)
			break;

		if (rc != ctx->mmc->write_bl_len)
			last_chunk = rc;

		oops_in_progress = saved_oip;
		if (rc <= 0)
			break;
		if (rc != ctx->mmc->write_bl_len)
			memset(ctx->bounce + rc, 0, ctx->mmc->write_bl_len - rc);

		/* Write the bounce buffer to eMMC */
		rc2 = ctx->mmc->block_dev.block_write(ctx->dev_num,off,1,ctx->bounce);
		if (rc2 <= 0) {
			printk(KERN_EMERG
			       "apanic: MMC write failed (%d)\n", rc2);
			return idx;
		}

		++num;
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

#ifdef APANIC_DEBUG
	printk ("apanic_console_write_mmc: wrote %d pages from %d returned %d\r\n", num, start, off);
#endif
	return num;
}

#endif

static int apanic(struct notifier_block *this, unsigned long event,
			void *ptr)
{
	struct apanic_data *ctx = &drv_ctx;
	struct panic_header *hdr = (struct panic_header *) ctx->bounce;
	int console_offset = 0;
	int console_len = 0;
	int threads_offset = 0;
	int threads_len = 0;
	int rc;
	unsigned long blk;

	if (in_panic)
		return NOTIFY_DONE;
	in_panic = 1;
#ifdef CONFIG_PREEMPT
	/* Ensure that cond_resched() won't try to preempt anybody */
	add_preempt_count(PREEMPT_ACTIVE);
#endif
	touch_softlockup_watchdog();

#ifdef CONFIG_APANIC_ON_MMC
	blk = get_apanic_start_address();
	if (blk == 0) {
		printk("apanic: Invalid block number \r\n");
		goto out;
	}

	if (mmc_poll_stack_init(&ctx->mmc, &ctx->dev_num) < 0) {
		printk("apanic: Unable to init polling mode mmc stack \r\n");
		goto out;
	}

	apanic_print("apanic: MMC device write block size is %d \r\n",ctx->mmc->write_bl_len);

	/* 
	 * The first block is reserved for apanic header so copy the log_buf
	 * from start_block + 1
	 */
	console_offset = blk + 1;

	apanic_print("Log Buff is stored from block number %d \r\n", console_offset);
	console_len = apanic_write_console_mmc(console_offset);
	if (console_len <= 0) {
		printk(KERN_EMERG "Error writing log but to panic log! (%d) \r\n", console_len);
		console_len = 0;
	}
	/*
	 * Write out all threads
	 */
	threads_offset = console_offset + console_len;

	apanic_print("Thread data is stored from block number %d \r\n", threads_offset);

	ram_console_enable_console(0);
	log_buf_clear();
	show_state_filter(0);

	threads_len = apanic_write_console_mmc(threads_offset);
	if (threads_len <= 0) {
		printk(KERN_EMERG "Error writing threads to panic log! (%d)\n",
		       threads_len);
		threads_len = 0;
	}

	/*
	 * Finally write the panic header at the first block of the partition
	 */
	memset(ctx->bounce, 0, PAGE_SIZE);
	hdr->magic = PANIC_MAGIC;
	hdr->version = PHDR_VERSION;

	/*
	 * blk - Holds the starting block number of the apanic partition
	 * console_offset - Holds the block number from where console data is
	 * 	            placed
	 * console_len    - Holds the size of console data stored in terms of
	 *                   number of blocks
	 *
	 * threads_offset, threads_length same as above but for threads info.
	 * This info is converted into bytes before storing the header, so
	 * when the application reads this header to go to console or to
	 * thread info it has to simply 'lseek' as many bytes.
	 */
	hdr->console_offset = (console_offset-blk)*ctx->mmc->write_bl_len;
	hdr->console_length = (console_len)*ctx->mmc->write_bl_len;

	hdr->threads_offset = (threads_offset-blk)*ctx->mmc->write_bl_len;
	hdr->threads_length = (threads_len)*ctx->mmc->write_bl_len;

	apanic_print("apanic: writing the header at block %d \r\n", blk);

	rc = ctx->mmc->block_dev.block_write(ctx->dev_num,blk,1,ctx->bounce);
	if (rc == 0) {
		printk(KERN_EMERG "apanic: Header write failed (%d)\n", rc);
		goto out;
	}

	apanic_print("kona_mmc_poll_write: bock_read returned %d \r\n", rc);

	printk(KERN_EMERG "apanic: Panic dump sucessfully written to flash\n");

#else
	if (!ctx->mtd)
		goto out;

	if (ctx->curr.magic) {
		printk(KERN_EMERG "Crash partition in use!\n");
		goto out;
	}
	console_offset = ctx->mtd->writesize;

	/*
	 * Write out the console
	 */
	console_len = apanic_write_console(ctx->mtd, console_offset);
	if (console_len < 0) {
		printk(KERN_EMERG "Error writing console to panic log! (%d)\n",
		       console_len);
		console_len = 0;
	}

	/*
	 * Write out all threads
	 */
	threads_offset = ALIGN(console_offset + console_len,
			       ctx->mtd->writesize);
	if (!threads_offset)
		threads_offset = ctx->mtd->writesize;

	ram_console_enable_console(0);

	log_buf_clear();
	show_state_filter(0);
	threads_len = apanic_write_console(ctx->mtd, threads_offset);
	if (threads_len < 0) {
		printk(KERN_EMERG "Error writing threads to panic log! (%d)\n",
		       threads_len);
		threads_len = 0;
	}

	/*
	 * Finally write the panic header
	 */
	memset(ctx->bounce, 0, PAGE_SIZE);
	hdr->magic = PANIC_MAGIC;
	hdr->version = PHDR_VERSION;

	hdr->console_offset = console_offset;
	hdr->console_length = console_len;

	hdr->threads_offset = threads_offset;
	hdr->threads_length = threads_len;

	rc = apanic_writeflashpage(ctx->mtd, 0, ctx->bounce);
	if (rc <= 0) {
		printk(KERN_EMERG "apanic: Header write failed (%d)\n",
		       rc);
		goto out;
	}

	printk(KERN_EMERG "apanic: Panic dump sucessfully written to flash\n");
#endif

 out:
#ifdef CONFIG_PREEMPT
	sub_preempt_count(PREEMPT_ACTIVE);
#endif
	in_panic = 0;
	return NOTIFY_DONE;
}

static struct notifier_block panic_blk = {
	.notifier_call	= apanic,
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

int __init apanic_init(void)
{
#ifndef CONFIG_APANIC_ON_MMC
	register_mtd_user(&mtd_panic_notifier);
#endif
	atomic_notifier_chain_register(&panic_notifier_list, &panic_blk);
	debugfs_create_file("apanic", 0644, NULL, NULL, &panic_dbg_fops);
	memset(&drv_ctx, 0, sizeof(drv_ctx));
	drv_ctx.bounce = (void *) __get_free_page(GFP_KERNEL);
	INIT_WORK(&proc_removal_work, apanic_remove_proc_work);
	printk(KERN_INFO "Android kernel panic handler initialized (bind=%s)\n",
	       CONFIG_APANIC_PLABEL);

	return 0;
}

module_init(apanic_init);
