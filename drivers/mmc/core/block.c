/*
 * Block driver for media (i.e., flash cards)
 *
 * Copyright 2002 Hewlett-Packard Company
 * Copyright 2005-2008 Pierre Ossman
 * Copyright 2013 Sony Mobile Communications Inc
 *
 * Use consistent with the GNU GPL is permitted,
 * provided that this copyright notice is
 * preserved in its entirety in all copies and derived works.
 *
 * HEWLETT-PACKARD COMPANY MAKES NO WARRANTIES, EXPRESSED OR IMPLIED,
 * AS TO THE USEFULNESS OR CORRECTNESS OF THIS CODE OR ITS
 * FITNESS FOR ANY PARTICULAR PURPOSE.
 *
 * Many thanks to Alessandro Rubini and Jonathan Corbet!
 *
 * Author:  Andrew Christian
 *          28 May 2002
 */
#include <linux/moduleparam.h>
#include <linux/module.h>
#include <linux/init.h>

#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/hdreg.h>
#include <linux/kdev_t.h>
#include <linux/blkdev.h>
#include <linux/mutex.h>
#include <linux/scatterlist.h>
#include <linux/bitops.h>
#include <linux/string_helpers.h>
#include <linux/delay.h>
#include <linux/capability.h>
#include <linux/compat.h>
#include <linux/pm_runtime.h>
#include <linux/ioprio.h>
#include <linux/idr.h>
#include <linux/debugfs.h>
#include <linux/sched/debug.h>

#include <linux/mmc/ioctl.h>
#include <linux/mmc/card.h>
#include <linux/mmc/core.h>
#include <linux/mmc/host.h>
#include <linux/mmc/mmc.h>
#include <linux/mmc/sd.h>

#include <linux/uaccess.h>

#include "queue.h"
#include "block.h"
#include "core.h"
#include "card.h"
#include "host.h"
#include "bus.h"
#include "mmc_ops.h"
#include "quirks.h"
#include "sd_ops.h"

MODULE_ALIAS("mmc:block");
#ifdef MODULE_PARAM_PREFIX
#undef MODULE_PARAM_PREFIX
#endif
#define MODULE_PARAM_PREFIX "mmcblk."

#define MMC_BLK_TIMEOUT_MS  (30 * 1000)        /* 30 secs timeout */
#define MMC_SANITIZE_REQ_TIMEOUT 240000
#define MMC_EXTRACT_INDEX_FROM_ARG(x) ((x & 0x00FF0000) >> 16)
#define MMC_EXTRACT_VALUE_FROM_ARG(x) ((x & 0x0000FF00) >> 8)

#define mmc_req_rel_wr(req)	((req->cmd_flags & REQ_FUA) && \
				  (rq_data_dir(req) == WRITE))

#define MMC_BLK_MAX_RETRIES 5 /* max # of retries before aborting a command */
#define MMC_BLK_UPDATE_STOP_REASON(stats, reason)			\
	do {								\
		if (stats->enabled)					\
			stats->pack_stop_reason[reason]++;		\
	} while (0)

#define MAX_RETRIES 5

#define EXE_ERRORS \
	(R1_OUT_OF_RANGE |   /* Command argument out of range */ \
	 R1_ADDRESS_ERROR |   /* Misaligned address */ \
	 R1_WP_VIOLATION |    /* Tried to write to protected block */ \
	 R1_CARD_ECC_FAILED | /* ECC error */ \
	 R1_ERROR)            /* General/unknown error */

static DEFINE_MUTEX(block_mutex);

/*
 * The defaults come from config options but can be overriden by module
 * or bootarg options.
 */
static int perdev_minors = CONFIG_MMC_BLOCK_MINORS;

/*
 * We've only got one major, so number of mmcblk devices is
 * limited to (1 << 20) / number of minors per device.  It is also
 * limited by the MAX_DEVICES below.
 */
static int max_devices;

#define MAX_DEVICES 256

static DEFINE_IDA(mmc_blk_ida);

/*
 * There is one mmc_blk_data per slot.
 */
struct mmc_blk_data {
	spinlock_t	lock;
	struct device	*parent;
	struct gendisk	*disk;
	struct mmc_queue queue;
	struct list_head part;

	unsigned int	flags;
#define MMC_BLK_CMD23	(1 << 0)	/* Can do SET_BLOCK_COUNT for multiblock */
#define MMC_BLK_REL_WR	(1 << 1)	/* MMC Reliable write support */
#define MMC_BLK_PACKED_CMD	(1 << 2) /* MMC packed command support */
#define MMC_BLK_CMD_QUEUE	(1 << 3) /* MMC command queue support */

	unsigned int	usage;
	unsigned int	read_only;
	unsigned int	part_type;
	unsigned int	reset_done;
#define MMC_BLK_READ		BIT(0)
#define MMC_BLK_WRITE		BIT(1)
#define MMC_BLK_DISCARD		BIT(2)
#define MMC_BLK_SECDISCARD	BIT(3)
#define MMC_BLK_FLUSH		BIT(4)
#define MMC_BLK_PARTSWITCH	BIT(5)

	/*
	 * Only set in main mmc_blk_data associated
	 * with mmc_card with dev_set_drvdata, and keeps
	 * track of the current selected device partition.
	 */
	unsigned int	part_curr;
	struct device_attribute force_ro;
	struct device_attribute power_ro_lock;
	int	area_type;

	/* debugfs files (only in main mmc_blk_data) */
	struct dentry *status_dentry;
	struct dentry *ext_csd_dentry;
};

static DEFINE_MUTEX(open_lock);

module_param(perdev_minors, int, 0444);
MODULE_PARM_DESC(perdev_minors, "Minors numbers to allocate per device");

static inline int mmc_blk_part_switch(struct mmc_card *card,
				      unsigned int part_type);
static int mmc_blk_cmdq_switch(struct mmc_card *card,
			       struct mmc_blk_data *md, bool enable);
static void mmc_blk_cmdq_err(struct mmc_queue *mq);
static enum blk_eh_timer_return mmc_blk_cmdq_req_timed_out(struct request *req);

static struct mmc_blk_data *mmc_blk_get(struct gendisk *disk)
{
	struct mmc_blk_data *md;

	mutex_lock(&open_lock);
	md = disk->private_data;
	if (md && md->usage == 0)
		md = NULL;
	if (md)
		md->usage++;
	mutex_unlock(&open_lock);

	return md;
}

static inline int mmc_get_devidx(struct gendisk *disk)
{
	int devidx = disk->first_minor / perdev_minors;
	return devidx;
}

static void mmc_blk_put(struct mmc_blk_data *md)
{
	mutex_lock(&open_lock);
	md->usage--;
	if (md->usage == 0) {
		int devidx = mmc_get_devidx(md->disk);
		ida_simple_remove(&mmc_blk_ida, devidx);
		put_disk(md->disk);
		kfree(md);
	}
	mutex_unlock(&open_lock);
}

static ssize_t power_ro_lock_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int ret;
	struct mmc_blk_data *md = mmc_blk_get(dev_to_disk(dev));
	struct mmc_card *card;
	int locked = 0;

	if (!md)
		return -EINVAL;

	card = md->queue.card;
	if (card->ext_csd.boot_ro_lock & EXT_CSD_BOOT_WP_B_PERM_WP_EN)
		locked = 2;
	else if (card->ext_csd.boot_ro_lock & EXT_CSD_BOOT_WP_B_PWR_WP_EN)
		locked = 1;

	ret = snprintf(buf, PAGE_SIZE, "%d\n", locked);

	mmc_blk_put(md);

	return ret;
}

static ssize_t power_ro_lock_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int ret;
	struct mmc_blk_data *md, *part_md;
	struct mmc_queue *mq;
	struct request *req;
	unsigned long set;

	if (kstrtoul(buf, 0, &set))
		return -EINVAL;

	if (set != 1)
		return count;

	md = mmc_blk_get(dev_to_disk(dev));
	if (!md)
		return -EINVAL;
	mq = &md->queue;

	/* Dispatch locking to the block layer */
	req = blk_get_request(mq->queue, REQ_OP_DRV_OUT, __GFP_RECLAIM);
	if (IS_ERR(req)) {
		count = PTR_ERR(req);
		goto out_put;
	}
	req_to_mmc_queue_req(req)->drv_op = MMC_DRV_OP_BOOT_WP;
	blk_execute_rq(mq->queue, NULL, req, 0);
	ret = req_to_mmc_queue_req(req)->drv_op_result;
	blk_put_request(req);

	if (!ret) {
		pr_info("%s: Locking boot partition ro until next power on\n",
			md->disk->disk_name);
		set_disk_ro(md->disk, 1);

		list_for_each_entry(part_md, &md->part, part)
			if (part_md->area_type == MMC_BLK_DATA_AREA_BOOT) {
				pr_info("%s: Locking boot partition ro until next power on\n", part_md->disk->disk_name);
				set_disk_ro(part_md->disk, 1);
			}
	}
out_put:
	mmc_blk_put(md);
	return count;
}

static ssize_t force_ro_show(struct device *dev, struct device_attribute *attr,
			     char *buf)
{
	int ret;
	struct mmc_blk_data *md = mmc_blk_get(dev_to_disk(dev));

	if (!md)
		return -EINVAL;

	ret = snprintf(buf, PAGE_SIZE, "%d\n",
		       get_disk_ro(dev_to_disk(dev)) ^
		       md->read_only);
	mmc_blk_put(md);
	return ret;
}

static ssize_t force_ro_store(struct device *dev, struct device_attribute *attr,
			      const char *buf, size_t count)
{
	int ret;
	char *end;
	struct mmc_blk_data *md = mmc_blk_get(dev_to_disk(dev));
	unsigned long set = simple_strtoul(buf, &end, 0);

	if (!md)
		return -EINVAL;

	if (end == buf) {
		ret = -EINVAL;
		goto out;
	}

	set_disk_ro(dev_to_disk(dev), set || md->read_only);
	ret = count;
out:
	mmc_blk_put(md);
	return ret;
}

#ifdef CONFIG_MMC_SIMULATE_MAX_SPEED

static int max_read_speed, max_write_speed, cache_size = 4;

module_param(max_read_speed, int, S_IRUSR | S_IRGRP);
MODULE_PARM_DESC(max_read_speed, "maximum KB/s read speed 0=off");
module_param(max_write_speed, int, S_IRUSR | S_IRGRP);
MODULE_PARM_DESC(max_write_speed, "maximum KB/s write speed 0=off");
module_param(cache_size, int, S_IRUSR | S_IRGRP);
MODULE_PARM_DESC(cache_size, "MB high speed memory or SLC cache");

/*
 * helper macros and expectations:
 *  size    - unsigned long number of bytes
 *  jiffies - unsigned long HZ timestamp difference
 *  speed   - unsigned KB/s transfer rate
 */
#define size_and_speed_to_jiffies(size, speed) \
		((size) * HZ / (speed) / 1024UL)
#define jiffies_and_speed_to_size(jiffies, speed) \
		(((speed) * (jiffies) * 1024UL) / HZ)
#define jiffies_and_size_to_speed(jiffies, size) \
		((size) * HZ / (jiffies) / 1024UL)

/* Limits to report warning */
/* jiffies_and_size_to_speed(10*HZ, queue_max_hw_sectors(q) * 512UL) ~ 25 */
#define MIN_SPEED(q) 250 /* 10 times faster than a floppy disk */
#define MAX_SPEED(q) jiffies_and_size_to_speed(1, queue_max_sectors(q) * 512UL)

#define speed_valid(speed) ((speed) > 0)

static const char off[] = "off\n";

static int max_speed_show(int speed, char *buf)
{
	if (speed)
		return scnprintf(buf, PAGE_SIZE, "%uKB/s\n", speed);
	else
		return scnprintf(buf, PAGE_SIZE, off);
}

static int max_speed_store(const char *buf, struct request_queue *q)
{
	unsigned int limit, set = 0;

	if (!strncasecmp(off, buf, sizeof(off) - 2))
		return set;
	if (kstrtouint(buf, 0, &set) || (set > INT_MAX))
		return -EINVAL;
	if (set == 0)
		return set;
	limit = MAX_SPEED(q);
	if (set > limit)
		pr_warn("max speed %u ineffective above %u\n", set, limit);
	limit = MIN_SPEED(q);
	if (set < limit)
		pr_warn("max speed %u painful below %u\n", set, limit);
	return set;
}

static ssize_t max_write_speed_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct mmc_blk_data *md = mmc_blk_get(dev_to_disk(dev));
	int ret = max_speed_show(atomic_read(&md->queue.max_write_speed), buf);

	mmc_blk_put(md);
	return ret;
}

static ssize_t max_write_speed_store(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t count)
{
	struct mmc_blk_data *md = mmc_blk_get(dev_to_disk(dev));
	int set = max_speed_store(buf, md->queue.queue);

	if (set < 0) {
		mmc_blk_put(md);
		return set;
	}

	atomic_set(&md->queue.max_write_speed, set);
	mmc_blk_put(md);
	return count;
}

static const DEVICE_ATTR(max_write_speed, S_IRUGO | S_IWUSR,
	max_write_speed_show, max_write_speed_store);

static ssize_t max_read_speed_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct mmc_blk_data *md = mmc_blk_get(dev_to_disk(dev));
	int ret = max_speed_show(atomic_read(&md->queue.max_read_speed), buf);

	mmc_blk_put(md);
	return ret;
}

static ssize_t max_read_speed_store(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t count)
{
	struct mmc_blk_data *md = mmc_blk_get(dev_to_disk(dev));
	int set = max_speed_store(buf, md->queue.queue);

	if (set < 0) {
		mmc_blk_put(md);
		return set;
	}

	atomic_set(&md->queue.max_read_speed, set);
	mmc_blk_put(md);
	return count;
}

static const DEVICE_ATTR(max_read_speed, S_IRUGO | S_IWUSR,
	max_read_speed_show, max_read_speed_store);

static ssize_t cache_size_show(struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	struct mmc_blk_data *md = mmc_blk_get(dev_to_disk(dev));
	struct mmc_queue *mq = &md->queue;
	int cache_size = atomic_read(&mq->cache_size);
	int ret;

	if (!cache_size)
		ret = scnprintf(buf, PAGE_SIZE, off);
	else {
		int speed = atomic_read(&mq->max_write_speed);

		if (!speed_valid(speed))
			ret = scnprintf(buf, PAGE_SIZE, "%uMB\n", cache_size);
		else { /* We accept race between cache_jiffies and cache_used */
			unsigned long size = jiffies_and_speed_to_size(
				jiffies - mq->cache_jiffies, speed);
			long used = atomic_long_read(&mq->cache_used);

			if (size >= used)
				size = 0;
			else
				size = (used - size) * 100 / cache_size
					/ 1024UL / 1024UL;

			ret = scnprintf(buf, PAGE_SIZE, "%uMB %lu%% used\n",
				cache_size, size);
		}
	}

	mmc_blk_put(md);
	return ret;
}

static ssize_t cache_size_store(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t count)
{
	struct mmc_blk_data *md;
	unsigned int set = 0;

	if (strncasecmp(off, buf, sizeof(off) - 2)
	 && (kstrtouint(buf, 0, &set) || (set > INT_MAX)))
		return -EINVAL;

	md = mmc_blk_get(dev_to_disk(dev));
	atomic_set(&md->queue.cache_size, set);
	mmc_blk_put(md);
	return count;
}

static const DEVICE_ATTR(cache_size, S_IRUGO | S_IWUSR,
	cache_size_show, cache_size_store);

/* correct for write-back */
static long mmc_blk_cache_used(struct mmc_queue *mq, unsigned long waitfor)
{
	long used = 0;
	int speed = atomic_read(&mq->max_write_speed);

	if (speed_valid(speed)) {
		unsigned long size = jiffies_and_speed_to_size(
					waitfor - mq->cache_jiffies, speed);
		used = atomic_long_read(&mq->cache_used);

		if (size >= used)
			used = 0;
		else
			used -= size;
	}

	atomic_long_set(&mq->cache_used, used);
	mq->cache_jiffies = waitfor;

	return used;
}

static void mmc_blk_simulate_delay(
	struct mmc_queue *mq,
	struct request *req,
	unsigned long waitfor)
{
	int max_speed;

	if (!req)
		return;

	max_speed = (rq_data_dir(req) == READ)
		? atomic_read(&mq->max_read_speed)
		: atomic_read(&mq->max_write_speed);
	if (speed_valid(max_speed)) {
		unsigned long bytes = blk_rq_bytes(req);

		if (rq_data_dir(req) != READ) {
			int cache_size = atomic_read(&mq->cache_size);

			if (cache_size) {
				unsigned long size = cache_size * 1024L * 1024L;
				long used = mmc_blk_cache_used(mq, waitfor);

				used += bytes;
				atomic_long_set(&mq->cache_used, used);
				bytes = 0;
				if (used > size)
					bytes = used - size;
			}
		}
		waitfor += size_and_speed_to_jiffies(bytes, max_speed);
		if (time_is_after_jiffies(waitfor)) {
			long msecs = jiffies_to_msecs(waitfor - jiffies);

			if (likely(msecs > 0))
				msleep(msecs);
		}
	}
}

#else

#define mmc_blk_simulate_delay(mq, req, waitfor)

#endif

static int mmc_blk_open(struct block_device *bdev, fmode_t mode)
{
	struct mmc_blk_data *md = mmc_blk_get(bdev->bd_disk);
	int ret = -ENXIO;

	mutex_lock(&block_mutex);
	if (md) {
		if (md->usage == 2)
			check_disk_change(bdev);
		ret = 0;

		if ((mode & FMODE_WRITE) && md->read_only) {
			mmc_blk_put(md);
			ret = -EROFS;
		}
	}
	mutex_unlock(&block_mutex);

	return ret;
}

static void mmc_blk_release(struct gendisk *disk, fmode_t mode)
{
	struct mmc_blk_data *md = disk->private_data;

	mutex_lock(&block_mutex);
	mmc_blk_put(md);
	mutex_unlock(&block_mutex);
}

static int
mmc_blk_getgeo(struct block_device *bdev, struct hd_geometry *geo)
{
	geo->cylinders = get_capacity(bdev->bd_disk) / (4 * 16);
	geo->heads = 4;
	geo->sectors = 16;
	return 0;
}

struct mmc_blk_ioc_data {
	struct mmc_ioc_cmd ic;
	unsigned char *buf;
	u64 buf_bytes;
};

static struct mmc_blk_ioc_data *mmc_blk_ioctl_copy_from_user(
	struct mmc_ioc_cmd __user *user)
{
	struct mmc_blk_ioc_data *idata;
	int err;

	idata = kmalloc(sizeof(*idata), GFP_KERNEL);
	if (!idata) {
		err = -ENOMEM;
		goto out;
	}

	if (copy_from_user(&idata->ic, user, sizeof(idata->ic))) {
		err = -EFAULT;
		goto idata_err;
	}

	idata->buf_bytes = (u64) idata->ic.blksz * idata->ic.blocks;
	if (idata->buf_bytes > MMC_IOC_MAX_BYTES) {
		err = -EOVERFLOW;
		goto idata_err;
	}

	if (!idata->buf_bytes) {
		idata->buf = NULL;
		return idata;
	}

	idata->buf = kmalloc(idata->buf_bytes, GFP_KERNEL);
	if (!idata->buf) {
		err = -ENOMEM;
		goto idata_err;
	}

	if (copy_from_user(idata->buf, (void __user *)(unsigned long)
					idata->ic.data_ptr, idata->buf_bytes)) {
		err = -EFAULT;
		goto copy_err;
	}

	return idata;

copy_err:
	kfree(idata->buf);
idata_err:
	kfree(idata);
out:
	return ERR_PTR(err);
}

static int mmc_blk_ioctl_copy_to_user(struct mmc_ioc_cmd __user *ic_ptr,
				      struct mmc_blk_ioc_data *idata)
{
	struct mmc_ioc_cmd *ic = &idata->ic;

	if (copy_to_user(&(ic_ptr->response), ic->response,
			 sizeof(ic->response)))
		return -EFAULT;

	if (!idata->ic.write_flag) {
		if (copy_to_user((void __user *)(unsigned long)ic->data_ptr,
				 idata->buf, idata->buf_bytes))
			return -EFAULT;
	}

	return 0;
}

static int ioctl_rpmb_card_status_poll(struct mmc_card *card, u32 *status,
				       u32 retries_max)
{
	int err;
	u32 retry_count = 0;

	if (!status || !retries_max)
		return -EINVAL;

	do {
		err = __mmc_send_status(card, status, 5);
		if (err)
			break;

		if (!R1_STATUS(*status) &&
				(R1_CURRENT_STATE(*status) != R1_STATE_PRG))
			break; /* RPMB programming operation complete */

		/*
		 * Rechedule to give the MMC device a chance to continue
		 * processing the previous command without being polled too
		 * frequently.
		 */
		usleep_range(1000, 5000);
	} while (++retry_count < retries_max);

	if (retry_count == retries_max)
		err = -EPERM;

	return err;
}

static int ioctl_do_sanitize(struct mmc_card *card)
{
	int err;

	if (!mmc_can_sanitize(card) &&
			(card->host->caps2 & MMC_CAP2_SANITIZE)) {
		pr_warn("%s: %s - SANITIZE is not supported\n",
				mmc_hostname(card->host), __func__);
		err = -EOPNOTSUPP;
		goto out;
	}

	pr_debug("%s: %s - SANITIZE IN PROGRESS...\n",
		mmc_hostname(card->host), __func__);

	err = mmc_switch(card, EXT_CSD_CMD_SET_NORMAL,
					EXT_CSD_SANITIZE_START, 1,
					MMC_SANITIZE_REQ_TIMEOUT);

	if (err)
		pr_err("%s: %s - EXT_CSD_SANITIZE_START failed. err=%d\n",
		       mmc_hostname(card->host), __func__, err);

	pr_debug("%s: %s - SANITIZE COMPLETED\n", mmc_hostname(card->host),
					     __func__);
out:
	return err;
}

static int __mmc_blk_ioctl_cmd(struct mmc_card *card, struct mmc_blk_data *md,
			       struct mmc_blk_ioc_data *idata)
{
	struct mmc_command cmd = {};
	struct mmc_data data = {};
	struct mmc_request mrq = {};
	struct scatterlist sg;
	int err;

	if (!card || !md || !idata)
		return -EINVAL;

	cmd.opcode = idata->ic.opcode;
	cmd.arg = idata->ic.arg;
	cmd.flags = idata->ic.flags;

	if (idata->buf_bytes) {
		data.sg = &sg;
		data.sg_len = 1;
		data.blksz = idata->ic.blksz;
		data.blocks = idata->ic.blocks;

		sg_init_one(data.sg, idata->buf, idata->buf_bytes);

		if (idata->ic.write_flag)
			data.flags = MMC_DATA_WRITE;
		else
			data.flags = MMC_DATA_READ;

		/* data.flags must already be set before doing this. */
		mmc_set_data_timeout(&data, card);

		/* Allow overriding the timeout_ns for empirical tuning. */
		if (idata->ic.data_timeout_ns)
			data.timeout_ns = idata->ic.data_timeout_ns;

		if ((cmd.flags & MMC_RSP_R1B) == MMC_RSP_R1B) {
			/*
			 * Pretend this is a data transfer and rely on the
			 * host driver to compute timeout.  When all host
			 * drivers support cmd.cmd_timeout for R1B, this
			 * can be changed to:
			 *
			 *     mrq.data = NULL;
			 *     cmd.cmd_timeout = idata->ic.cmd_timeout_ms;
			 */
			data.timeout_ns = idata->ic.cmd_timeout_ms * 1000000;
		}

		mrq.data = &data;
	}

	mrq.cmd = &cmd;

	err = mmc_blk_part_switch(card, md->part_type);
	if (err)
		return err;

	if (idata->ic.is_acmd) {
		err = mmc_app_cmd(card->host, card);
		if (err)
			return err;
	}

	if ((MMC_EXTRACT_INDEX_FROM_ARG(cmd.arg) == EXT_CSD_SANITIZE_START) &&
	    (cmd.opcode == MMC_SWITCH)) {
		err = ioctl_do_sanitize(card);

		if (err)
			pr_err("%s: ioctl_do_sanitize() failed. err = %d",
			       __func__, err);

		return err;
	}

	mmc_wait_for_req(card->host, &mrq);

	if (cmd.error) {
		dev_err(mmc_dev(card->host), "%s: cmd error %d\n",
						__func__, cmd.error);
		return cmd.error;
	}
	if (data.error) {
		dev_err(mmc_dev(card->host), "%s: data error %d\n",
						__func__, data.error);
		return data.error;
	}

	/*
	 * Make sure the cache of the PARTITION_CONFIG register and
	 * PARTITION_ACCESS bits is updated in case the ioctl ext_csd write
	 * changed it successfully.
	 */
	if ((MMC_EXTRACT_INDEX_FROM_ARG(cmd.arg) == EXT_CSD_PART_CONFIG) &&
	    (cmd.opcode == MMC_SWITCH)) {
		struct mmc_blk_data *main_md = dev_get_drvdata(&card->dev);
		u8 value = MMC_EXTRACT_VALUE_FROM_ARG(cmd.arg);

		/*
		 * Update cache so the next mmc_blk_part_switch call operates
		 * on up-to-date data.
		 */
		card->ext_csd.part_config = value;
		main_md->part_curr = value & EXT_CSD_PART_CONFIG_ACC_MASK;
	}

	/*
	 * Make sure to update CACHE_CTRL in case it was changed. The cache
	 * will get turned back on if the card is re-initialized, e.g.
	 * suspend/resume or hw reset in recovery.
	 */
	if ((MMC_EXTRACT_INDEX_FROM_ARG(cmd.arg) == EXT_CSD_CACHE_CTRL) &&
	    (cmd.opcode == MMC_SWITCH)) {
		u8 value = MMC_EXTRACT_VALUE_FROM_ARG(cmd.arg) & 1;

		card->ext_csd.cache_ctrl = value;
	}

	/*
	 * According to the SD specs, some commands require a delay after
	 * issuing the command.
	 */
	if (idata->ic.postsleep_min_us)
		usleep_range(idata->ic.postsleep_min_us, idata->ic.postsleep_max_us);

	memcpy(&(idata->ic.response), cmd.resp, sizeof(cmd.resp));

	return err;
}

struct mmc_blk_ioc_rpmb_data {
	struct mmc_blk_ioc_data *data[MMC_IOC_MAX_RPMB_CMD];
};

static struct mmc_blk_ioc_rpmb_data *mmc_blk_ioctl_rpmb_copy_from_user(
	struct mmc_ioc_rpmb __user *user)
{
	struct mmc_blk_ioc_rpmb_data *idata;
	int err, i;

	idata = kzalloc(sizeof(*idata), GFP_KERNEL);
	if (!idata) {
		err = -ENOMEM;
		goto out;
	}

	for (i = 0; i < MMC_IOC_MAX_RPMB_CMD; i++) {
		idata->data[i] = mmc_blk_ioctl_copy_from_user(&(user->cmds[i]));
		if (IS_ERR(idata->data[i])) {
			err = PTR_ERR(idata->data[i]);
			goto copy_err;
		}
	}

	return idata;

copy_err:
	while (--i >= 0) {
		kfree(idata->data[i]->buf);
		kfree(idata->data[i]);
	}
	kfree(idata);
out:
	return ERR_PTR(err);
}

static int mmc_blk_ioctl_rpmb_cmd(struct block_device *bdev,
	struct mmc_ioc_rpmb __user *ic_ptr)
{
	struct mmc_blk_ioc_rpmb_data *idata;
	struct mmc_blk_data *md;
	struct mmc_card *card = NULL;
	struct mmc_command cmd = {0};
	struct mmc_data data = {0};
	struct mmc_request mrq = {NULL};
	struct scatterlist sg;
	int err = 0, i = 0;
	u32 status = 0;

	/* The caller must have CAP_SYS_RAWIO */
	if (!capable(CAP_SYS_RAWIO))
		return -EPERM;

	md = mmc_blk_get(bdev->bd_disk);
	/* make sure this is a rpmb partition */
	if ((!md) || (!(md->area_type & MMC_BLK_DATA_AREA_RPMB))) {
		err = -EINVAL;
		return err;
	}

	idata = mmc_blk_ioctl_rpmb_copy_from_user(ic_ptr);
	if (IS_ERR(idata)) {
		err = PTR_ERR(idata);
		goto cmd_done;
	}

	card = md->queue.card;
	if (IS_ERR(card)) {
		err = PTR_ERR(card);
		goto idata_free;
	}

	/*
	 * Ensure rpmb_req_pending flag is synchronized between multiple
	 * entities which may use rpmb ioclts with a lock.
	 */
	mutex_lock(&card->host->rpmb_req_mutex);
	atomic_set(&card->host->rpmb_req_pending, 1);
	mmc_get_card(card);

	if (mmc_card_doing_bkops(card)) {
		if (mmc_card_cmdq(card)) {
			err = mmc_cmdq_halt(card->host, true);
			if (err)
				goto cmd_rel_host;
		}
		err = mmc_stop_bkops(card);
		if (err) {
			dev_err(mmc_dev(card->host),
				"%s: stop_bkops failed %d\n", __func__, err);
			goto cmd_rel_host;
		}
		if (mmc_card_cmdq(card)) {
			err = mmc_cmdq_halt(card->host, false);
			if (err)
				goto cmd_rel_host;
		}
	}

	err = mmc_blk_part_switch(card, md->part_type);
	if (err)
		goto cmd_rel_host;

	for (i = 0; i < MMC_IOC_MAX_RPMB_CMD; i++) {
		struct mmc_blk_ioc_data *curr_data;
		struct mmc_ioc_cmd *curr_cmd;

		curr_data = idata->data[i];
		curr_cmd = &curr_data->ic;
		if (!curr_cmd->opcode)
			break;

		cmd.opcode = curr_cmd->opcode;
		cmd.arg = curr_cmd->arg;
		cmd.flags = curr_cmd->flags;

		if (curr_data->buf_bytes) {
			data.sg = &sg;
			data.sg_len = 1;
			data.blksz = curr_cmd->blksz;
			data.blocks = curr_cmd->blocks;

			sg_init_one(data.sg, curr_data->buf,
					curr_data->buf_bytes);

			if (curr_cmd->write_flag)
				data.flags = MMC_DATA_WRITE;
			else
				data.flags = MMC_DATA_READ;

			/* data.flags must already be set before doing this. */
			mmc_set_data_timeout(&data, card);

			/*
			 * Allow overriding the timeout_ns for empirical tuning.
			 */
			if (curr_cmd->data_timeout_ns)
				data.timeout_ns = curr_cmd->data_timeout_ns;

			mrq.data = &data;
		}

		mrq.cmd = &cmd;

		err = mmc_set_blockcount(card, data.blocks,
				curr_cmd->write_flag & (1 << 31));
		if (err)
			goto cmd_rel_host;

		mmc_wait_for_req(card->host, &mrq);

		if (cmd.error) {
			dev_err(mmc_dev(card->host), "%s: cmd error %d\n",
					__func__, cmd.error);
			err = cmd.error;
			goto cmd_rel_host;
		}
		if (data.error) {
			dev_err(mmc_dev(card->host), "%s: data error %d\n",
					__func__, data.error);
			err = data.error;
			goto cmd_rel_host;
		}

		if (copy_to_user(&(ic_ptr->cmds[i].response), cmd.resp,
					sizeof(cmd.resp))) {
			err = -EFAULT;
			goto cmd_rel_host;
		}

		if (!curr_cmd->write_flag) {
			if (copy_to_user((void __user *)(unsigned long)
						curr_cmd->data_ptr,
						curr_data->buf,
						curr_data->buf_bytes)) {
				err = -EFAULT;
				goto cmd_rel_host;
			}
		}

		/*
		 * Ensure RPMB command has completed by polling CMD13
		 * "Send Status".
		 */
		err = ioctl_rpmb_card_status_poll(card, &status, 5);
		if (err)
			dev_err(mmc_dev(card->host),
					"%s: Card Status=0x%08X, error %d\n",
					__func__, status, err);
	}

cmd_rel_host:
	mmc_put_card(card);
	atomic_set(&card->host->rpmb_req_pending, 0);
	mutex_unlock(&card->host->rpmb_req_mutex);

idata_free:
	for (i = 0; i < MMC_IOC_MAX_RPMB_CMD; i++) {
		kfree(idata->data[i]->buf);
		kfree(idata->data[i]);
	}
	kfree(idata);

cmd_done:
	mmc_blk_put(md);
	if (card && card->cmdq_init)
		wake_up(&card->host->cmdq_ctx.wait);
	return err;
}

static int mmc_blk_ioctl_cmd(struct mmc_blk_data *md,
			     struct mmc_ioc_cmd __user *ic_ptr)
{
	struct mmc_blk_ioc_data *idata;
	struct mmc_blk_ioc_data *idatas[1];
	struct mmc_queue *mq;
	struct mmc_card *card;
	int err = 0, ioc_err = 0;
	struct request *req;

	idata = mmc_blk_ioctl_copy_from_user(ic_ptr);
	if (IS_ERR_OR_NULL(idata))
		return PTR_ERR(idata);

	card = md->queue.card;
	if (IS_ERR_OR_NULL(card)) {
		err = PTR_ERR(card);
		goto cmd_done;
	}

	/*
	 * Dispatch the ioctl() into the block request queue.
	 */
	mq = &md->queue;
	req = blk_get_request(mq->queue,
		idata->ic.write_flag ? REQ_OP_DRV_OUT : REQ_OP_DRV_IN,
		__GFP_RECLAIM);
	if (IS_ERR(req)) {
		err = PTR_ERR(req);
		goto cmd_done;
	}
	idatas[0] = idata;
	req_to_mmc_queue_req(req)->drv_op = MMC_DRV_OP_IOCTL;
	req_to_mmc_queue_req(req)->drv_op_data = idatas;
	req_to_mmc_queue_req(req)->ioc_count = 1;
	blk_execute_rq(mq->queue, NULL, req, 0);
	ioc_err = req_to_mmc_queue_req(req)->drv_op_result;
	err = mmc_blk_ioctl_copy_to_user(ic_ptr, idata);
	blk_put_request(req);

cmd_done:
	kfree(idata->buf);
	kfree(idata);
	return ioc_err ? ioc_err : err;
}

static int mmc_blk_ioctl_multi_cmd(struct mmc_blk_data *md,
				   struct mmc_ioc_multi_cmd __user *user)
{
	struct mmc_blk_ioc_data **idata = NULL;
	struct mmc_ioc_cmd __user *cmds = user->cmds;
	struct mmc_card *card;
	struct mmc_queue *mq;
	int i, err = 0, ioc_err = 0;
	__u64 num_of_cmds;
	struct request *req;

	if (copy_from_user(&num_of_cmds, &user->num_of_cmds,
			   sizeof(num_of_cmds)))
		return -EFAULT;

	if (!num_of_cmds)
		return 0;

	if (num_of_cmds > MMC_IOC_MAX_CMDS)
		return -EINVAL;

	idata = kcalloc(num_of_cmds, sizeof(*idata), GFP_KERNEL);
	if (!idata)
		return -ENOMEM;

	for (i = 0; i < num_of_cmds; i++) {
		idata[i] = mmc_blk_ioctl_copy_from_user(&cmds[i]);
		if (IS_ERR(idata[i])) {
			err = PTR_ERR(idata[i]);
			num_of_cmds = i;
			goto cmd_err;
		}
	}

	card = md->queue.card;
	if (IS_ERR(card)) {
		err = PTR_ERR(card);
		goto cmd_err;
	}


	/*
	 * Dispatch the ioctl()s into the block request queue.
	 */
	mq = &md->queue;
	req = blk_get_request(mq->queue,
		idata[0]->ic.write_flag ? REQ_OP_DRV_OUT : REQ_OP_DRV_IN,
		__GFP_RECLAIM);
	if (IS_ERR(req)) {
		err = PTR_ERR(req);
		goto cmd_err;
	}
	req_to_mmc_queue_req(req)->drv_op = MMC_DRV_OP_IOCTL;
	req_to_mmc_queue_req(req)->drv_op_data = idata;
	req_to_mmc_queue_req(req)->ioc_count = num_of_cmds;
	blk_execute_rq(mq->queue, NULL, req, 0);
	ioc_err = req_to_mmc_queue_req(req)->drv_op_result;

	/* copy to user if data and response */
	for (i = 0; i < num_of_cmds && !err; i++)
		err = mmc_blk_ioctl_copy_to_user(&cmds[i], idata[i]);

	blk_put_request(req);

cmd_err:
	for (i = 0; i < num_of_cmds; i++) {
		kfree(idata[i]->buf);
		kfree(idata[i]);
	}
	kfree(idata);
	return ioc_err ? ioc_err : err;
}

static int mmc_blk_check_blkdev(struct block_device *bdev)
{
	/*
	 * The caller must have CAP_SYS_RAWIO, and must be calling this on the
	 * whole block device, not on a partition.  This prevents overspray
	 * between sibling partitions.
	 */
	if ((!capable(CAP_SYS_RAWIO)) || (bdev != bdev->bd_contains))
		return -EPERM;
	return 0;
}

static int mmc_blk_ioctl(struct block_device *bdev, fmode_t mode,
	unsigned int cmd, unsigned long arg)
{
	struct mmc_blk_data *md;
	int ret;

	switch (cmd) {
	case MMC_IOC_CMD:
		ret = mmc_blk_check_blkdev(bdev);
		if (ret)
			return ret;
		md = mmc_blk_get(bdev->bd_disk);
		if (!md)
			return -EINVAL;
		ret = mmc_blk_ioctl_cmd(md,
					(struct mmc_ioc_cmd __user *)arg);
		mmc_blk_put(md);
		return ret;
	case MMC_IOC_RPMB_CMD:
		return mmc_blk_ioctl_rpmb_cmd(bdev,
				(struct mmc_ioc_rpmb __user *)arg);
	case MMC_IOC_MULTI_CMD:
		ret = mmc_blk_check_blkdev(bdev);
		if (ret)
			return ret;
		md = mmc_blk_get(bdev->bd_disk);
		if (!md)
			return -EINVAL;
		ret = mmc_blk_ioctl_multi_cmd(md,
					(struct mmc_ioc_multi_cmd __user *)arg);
		mmc_blk_put(md);
		return ret;
	default:
		return -EINVAL;
	}
}

#ifdef CONFIG_COMPAT
static int mmc_blk_compat_ioctl(struct block_device *bdev, fmode_t mode,
	unsigned int cmd, unsigned long arg)
{
	return mmc_blk_ioctl(bdev, mode, cmd, (unsigned long) compat_ptr(arg));
}
#endif

static const struct block_device_operations mmc_bdops = {
	.open			= mmc_blk_open,
	.release		= mmc_blk_release,
	.getgeo			= mmc_blk_getgeo,
	.owner			= THIS_MODULE,
	.ioctl			= mmc_blk_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl		= mmc_blk_compat_ioctl,
#endif
};

static int mmc_blk_part_switch_pre(struct mmc_card *card,
				   unsigned int part_type)
{
	int ret = 0;

	if (part_type == EXT_CSD_PART_CONFIG_ACC_RPMB) {
		if (card->ext_csd.cmdq_en) {
			ret = mmc_cmdq_disable(card);
			if (ret)
				return ret;
		}
		mmc_retune_pause(card->host);
	}

	return ret;
}

static int mmc_blk_part_switch_post(struct mmc_card *card,
				    unsigned int part_type)
{
	int ret = 0;

	if (part_type == EXT_CSD_PART_CONFIG_ACC_RPMB) {
		mmc_retune_unpause(card->host);
		if (card->reenable_cmdq && !card->ext_csd.cmdq_en)
			ret = mmc_cmdq_enable(card);
	}

	return ret;
}

static inline int mmc_blk_part_switch(struct mmc_card *card,
				      unsigned int part_type)
{
	int ret = 0;
	struct mmc_blk_data *main_md = dev_get_drvdata(&card->dev);

	if ((main_md->part_curr == part_type) &&
	    (card->part_curr == part_type))
		return 0;

	if (mmc_card_mmc(card)) {
		u8 part_config = card->ext_csd.part_config;

		if (part_type) {
			/* disable CQ mode for non-user data partitions */
			ret = mmc_blk_cmdq_switch(card, main_md, false);
			if (ret)
				return ret;
		}

		ret = mmc_blk_part_switch_pre(card, part_type);
		if (ret)
			return ret;

		part_config &= ~EXT_CSD_PART_CONFIG_ACC_MASK;
		part_config |= part_type;

		ret = mmc_switch(card, EXT_CSD_CMD_SET_NORMAL,
				 EXT_CSD_PART_CONFIG, part_config,
				 card->ext_csd.part_time);
		if (ret) {
			pr_err("%s: %s: switch failure, %d -> %d\n",
				mmc_hostname(card->host), __func__,
				main_md->part_curr, part_type);
			mmc_blk_part_switch_post(card, part_type);
			return ret;
		}

		card->ext_csd.part_config = part_config;
		card->part_curr = part_type;

		ret = mmc_blk_part_switch_post(card, main_md->part_curr);
	}

	main_md->part_curr = part_type;
	return ret;
}

static int mmc_sd_num_wr_blocks(struct mmc_card *card, u32 *written_blocks)
{
	int err;
	u32 result;
	__be32 *blocks;

	struct mmc_request mrq = {};
	struct mmc_command cmd = {};
	struct mmc_data data = {};

	struct scatterlist sg;

	cmd.opcode = MMC_APP_CMD;
	cmd.arg = card->rca << 16;
	cmd.flags = MMC_RSP_SPI_R1 | MMC_RSP_R1 | MMC_CMD_AC;

	err = mmc_wait_for_cmd(card->host, &cmd, 0);
	if (err)
		return err;
	if (!mmc_host_is_spi(card->host) && !(cmd.resp[0] & R1_APP_CMD))
		return -EIO;

	memset(&cmd, 0, sizeof(struct mmc_command));

	cmd.opcode = SD_APP_SEND_NUM_WR_BLKS;
	cmd.arg = 0;
	cmd.flags = MMC_RSP_SPI_R1 | MMC_RSP_R1 | MMC_CMD_ADTC;

	data.blksz = 4;
	data.blocks = 1;
	data.flags = MMC_DATA_READ;
	data.sg = &sg;
	data.sg_len = 1;
	mmc_set_data_timeout(&data, card);

	mrq.cmd = &cmd;
	mrq.data = &data;

	blocks = kmalloc(4, GFP_KERNEL);
	if (!blocks)
		return -ENOMEM;

	sg_init_one(&sg, blocks, 4);

	mmc_wait_for_req(card->host, &mrq);

	result = ntohl(*blocks);
	kfree(blocks);

	if (cmd.error || data.error)
		return -EIO;

	*written_blocks = result;

	return 0;
}

static int get_card_status(struct mmc_card *card, u32 *status, int retries)
{
	struct mmc_command cmd = {0};
	int err;

	cmd.opcode = MMC_SEND_STATUS;
	if (!mmc_host_is_spi(card->host))
		cmd.arg = card->rca << 16;
	cmd.flags = MMC_RSP_SPI_R2 | MMC_RSP_R1 | MMC_CMD_AC;
	err = mmc_wait_for_cmd(card->host, &cmd, retries);
	if (err == 0)
		*status = cmd.resp[0];
	return err;
}

static int card_busy_detect(struct mmc_card *card, unsigned int timeout_ms,
		bool hw_busy_detect, struct request *req, bool *gen_err)
{
	unsigned long timeout = jiffies + msecs_to_jiffies(timeout_ms);
	int err = 0;
	u32 status, first_status = 0;

	do {
		err = __mmc_send_status(card, &status, 5);
		if (err) {
			pr_err("%s: error %d requesting status\n",
			       req->rq_disk->disk_name, err);
			return err;
		}

		if (!first_status)
			first_status = status;

		if (status & R1_ERROR) {
			pr_err("%s: %s: error sending status cmd, status %#x\n",
				req->rq_disk->disk_name, __func__, status);
			*gen_err = true;
		}

		/* We may rely on the host hw to handle busy detection.*/
		if ((card->host->caps & MMC_CAP_WAIT_WHILE_BUSY) &&
			hw_busy_detect)
			break;

		/*
		 * Timeout if the device never becomes ready for data and never
		 * leaves the program state.
		 */
		if (time_after(jiffies, timeout)) {
			pr_err("%s: Card stuck in programming state! %s %s\n",
				mmc_hostname(card->host),
				req->rq_disk->disk_name, __func__);
			return -ETIMEDOUT;
		}

		/*
		 * Some cards mishandle the status bits,
		 * so make sure to check both the busy
		 * indication and the card state.
		 */
	} while (!(status & R1_READY_FOR_DATA) ||
		 (R1_CURRENT_STATE(status) == R1_STATE_PRG));

	/* Check for errors during cmd execution. In this case
	 * the execution was terminated. */
	if (first_status & EXE_ERRORS) {
		pr_err("%s: error during r/w command, err status %#x, status %#x\n",
		       req->rq_disk->disk_name, first_status, status);
		return MMC_BLK_ABORT;
	}

	return err;
}

static int send_stop(struct mmc_card *card, unsigned int timeout_ms,
		struct request *req, bool *gen_err, u32 *stop_status)
{
	struct mmc_host *host = card->host;
	struct mmc_command cmd = {};
	int err;
	bool use_r1b_resp = rq_data_dir(req) == WRITE;

	/*
	 * Normally we use R1B responses for WRITE, but in cases where the host
	 * has specified a max_busy_timeout we need to validate it. A failure
	 * means we need to prevent the host from doing hw busy detection, which
	 * is done by converting to a R1 response instead.
	 */
	if (host->max_busy_timeout && (timeout_ms > host->max_busy_timeout))
		use_r1b_resp = false;

	cmd.opcode = MMC_STOP_TRANSMISSION;
	if (use_r1b_resp) {
		cmd.flags = MMC_RSP_SPI_R1B | MMC_RSP_R1B | MMC_CMD_AC;
		cmd.busy_timeout = timeout_ms;
	} else {
		cmd.flags = MMC_RSP_SPI_R1 | MMC_RSP_R1 | MMC_CMD_AC;
	}

	err = mmc_wait_for_cmd(host, &cmd, 5);
	if (err)
		return err;

	*stop_status = cmd.resp[0];

	/* No need to check card status in case of READ. */
	if (rq_data_dir(req) == READ)
		return 0;

	if (!mmc_host_is_spi(host) &&
		(*stop_status & R1_ERROR)) {
		pr_err("%s: %s: general error sending stop command, resp %#x\n",
			req->rq_disk->disk_name, __func__, *stop_status);
		*gen_err = true;
	}

	return card_busy_detect(card, timeout_ms, use_r1b_resp, req, gen_err);
}

#define ERR_NOMEDIUM	3
#define ERR_RETRY	2
#define ERR_ABORT	1
#define ERR_CONTINUE	0

static int mmc_blk_cmd_error(struct request *req, const char *name, int error,
	bool status_valid, u32 status)
{
	switch (error) {
	case -EILSEQ:
		/* response crc error, retry the r/w cmd */
		pr_err_ratelimited(
			"%s: response CRC error sending %s command, card status %#x\n",
			req->rq_disk->disk_name,
			name, status);
		return ERR_RETRY;

	case -ETIMEDOUT:
		pr_err_ratelimited(
			"%s: timed out sending %s command, card status %#x\n",
			req->rq_disk->disk_name, name, status);

		/* If the status cmd initially failed, retry the r/w cmd */
		if (!status_valid) {
			pr_err_ratelimited("%s: status not valid, retrying timeout\n",
				req->rq_disk->disk_name);
			return ERR_RETRY;
		}

		/*
		 * If it was a r/w cmd crc error, or illegal command
		 * (eg, issued in wrong state) then retry - we should
		 * have corrected the state problem above.
		 */
		if (status & (R1_COM_CRC_ERROR | R1_ILLEGAL_COMMAND)) {
			pr_err_ratelimited(
				"%s: command error, retrying timeout\n",
				req->rq_disk->disk_name);
			return ERR_RETRY;
		}

		/* Otherwise abort the command */
		pr_err_ratelimited(
			"%s: not retrying timeout\n",
			req->rq_disk->disk_name);
		return ERR_ABORT;

	default:
		/* We don't understand the error code the driver gave us */
		pr_err_ratelimited(
			"%s: unknown error %d sending read/write command, card status %#x\n",
		       req->rq_disk->disk_name, error, status);
		return ERR_ABORT;
	}
}

/*
 * Initial r/w and stop cmd error recovery.
 * We don't know whether the card received the r/w cmd or not, so try to
 * restore things back to a sane state.  Essentially, we do this as follows:
 * - Obtain card status.  If the first attempt to obtain card status fails,
 *   the status word will reflect the failed status cmd, not the failed
 *   r/w cmd.  If we fail to obtain card status, it suggests we can no
 *   longer communicate with the card.
 * - Check the card state.  If the card received the cmd but there was a
 *   transient problem with the response, it might still be in a data transfer
 *   mode.  Try to send it a stop command.  If this fails, we can't recover.
 * - If the r/w cmd failed due to a response CRC error, it was probably
 *   transient, so retry the cmd.
 * - If the r/w cmd timed out, but we didn't get the r/w cmd status, retry.
 * - If the r/w cmd timed out, and the r/w cmd failed due to CRC error or
 *   illegal cmd, retry.
 * Otherwise we don't understand what happened, so abort.
 */
static int mmc_blk_cmd_recovery(struct mmc_card *card, struct request *req,
	struct mmc_blk_request *brq, bool *ecc_err, bool *gen_err)
{
	bool prev_cmd_status_valid = true;
	u32 status, stop_status = 0;
	int err, retry;

	if (mmc_card_removed(card))
		return ERR_NOMEDIUM;

	/*
	 * Try to get card status which indicates both the card state
	 * and why there was no response.  If the first attempt fails,
	 * we can't be sure the returned status is for the r/w command.
	 */
	for (retry = 2; retry >= 0; retry--) {
		err = __mmc_send_status(card, &status, 0);
		if (!err)
			break;

		/* Re-tune if needed */
		mmc_retune_recheck(card->host);

		prev_cmd_status_valid = false;
		pr_err("%s: error %d sending status command, %sing\n",
		       req->rq_disk->disk_name, err, retry ? "retry" : "abort");
	}

	/* We couldn't get a response from the card.  Give up. */
	if (err) {
		/* Check if the card is removed */
		if (mmc_detect_card_removed(card->host))
			return ERR_NOMEDIUM;
		return ERR_ABORT;
	}

	/* Flag ECC errors */
	if ((status & R1_CARD_ECC_FAILED) ||
	    (brq->stop.resp[0] & R1_CARD_ECC_FAILED) ||
	    (brq->cmd.resp[0] & R1_CARD_ECC_FAILED))
		*ecc_err = true;

	/* Flag General errors */
	if (!mmc_host_is_spi(card->host) && rq_data_dir(req) != READ)
		if ((status & R1_ERROR) ||
			(brq->stop.resp[0] & R1_ERROR)) {
			pr_err("%s: %s: general error sending stop or status command, stop cmd response %#x, card status %#x\n",
			       req->rq_disk->disk_name, __func__,
			       brq->stop.resp[0], status);
			*gen_err = true;
		}

	/*
	 * Check the current card state.  If it is in some data transfer
	 * mode, tell it to stop (and hopefully transition back to TRAN.)
	 */
	if (R1_CURRENT_STATE(status) == R1_STATE_DATA ||
	    R1_CURRENT_STATE(status) == R1_STATE_RCV) {
		err = send_stop(card,
			DIV_ROUND_UP(brq->data.timeout_ns, 1000000),
			req, gen_err, &stop_status);
		if (err) {
			pr_err("%s: error %d sending stop command\n",
			       req->rq_disk->disk_name, err);
			/*
			 * If the stop cmd also timed out, the card is probably
			 * not present, so abort. Other errors are bad news too.
			 */
			return ERR_ABORT;
		}

		if (stop_status & R1_CARD_ECC_FAILED)
			*ecc_err = true;
	}

	/* Check for set block count errors */
	if (brq->sbc.error)
		return mmc_blk_cmd_error(req, "SET_BLOCK_COUNT", brq->sbc.error,
				prev_cmd_status_valid, status);

	/* Check for r/w command errors */
	if (brq->cmd.error)
		return mmc_blk_cmd_error(req, "r/w cmd", brq->cmd.error,
				prev_cmd_status_valid, status);

	/* Data errors */
	if (!brq->stop.error)
		return ERR_CONTINUE;

	/* Now for stop errors.  These aren't fatal to the transfer. */
	pr_info("%s: error %d sending stop command, original cmd response %#x, card status %#x\n",
	       req->rq_disk->disk_name, brq->stop.error,
	       brq->cmd.resp[0], status);

	/*
	 * Subsitute in our own stop status as this will give the error
	 * state which happened during the execution of the r/w command.
	 */
	if (stop_status) {
		brq->stop.resp[0] = stop_status;
		brq->stop.error = 0;
	}
	return ERR_CONTINUE;
}

static int mmc_blk_reset(struct mmc_blk_data *md, struct mmc_host *host,
			 int type)
{
	int err;

	if (md->reset_done & type)
		return -EEXIST;

	md->reset_done |= type;
	err = mmc_hw_reset(host);
	if (err && err != -EOPNOTSUPP) {
		/* We failed to reset so we need to abort the request */
		pr_err("%s: %s: failed to reset %d\n", mmc_hostname(host),
				__func__, err);
		return -ENODEV;
	}

	/* Ensure we switch back to the correct partition */
	if (host->card) {
		struct mmc_blk_data *main_md =
			dev_get_drvdata(&host->card->dev);
		int part_err;

		main_md->part_curr = main_md->part_type;
		part_err = mmc_blk_part_switch(host->card, md->part_type);
		if (part_err) {
			/*
			 * We have failed to get back into the correct
			 * partition, so we need to abort the whole request.
			 */
			return -ENODEV;
		}
	}
	return err;
}

static inline void mmc_blk_reset_success(struct mmc_blk_data *md, int type)
{
	md->reset_done &= ~type;
}

int mmc_access_rpmb(struct mmc_queue *mq)
{
	struct mmc_blk_data *md = mq->blkdata;
	/*
	 * If this is a RPMB partition access, return ture
	 */
	if (md && md->part_type == EXT_CSD_PART_CONFIG_ACC_RPMB)
		return true;

	return false;
}

/*
 * The non-block commands come back from the block layer after it queued it and
 * processed it with all other requests and then they get issued in this
 * function.
 */
static void mmc_blk_issue_drv_op(struct mmc_queue *mq, struct request *req)
{
	struct mmc_queue_req *mq_rq;
	struct mmc_card *card = mq->card;
	struct mmc_blk_data *md = mq->blkdata;
	struct mmc_blk_data *main_md = dev_get_drvdata(&card->dev);
	struct mmc_blk_ioc_data **idata;
	u8 **ext_csd;
	u32 status;
	int ret;
	int i;

	mq_rq = req_to_mmc_queue_req(req);

	switch (mq_rq->drv_op) {
	case MMC_DRV_OP_IOCTL:
		idata = mq_rq->drv_op_data;
		for (i = 0, ret = 0; i < mq_rq->ioc_count; i++) {
			ret = __mmc_blk_ioctl_cmd(card, md, idata[i]);
			if (ret)
				break;
		}
		/* Always switch back to main area after RPMB access */
		if (md->area_type & MMC_BLK_DATA_AREA_RPMB)
			mmc_blk_part_switch(card, main_md->part_type);
		break;
	case MMC_DRV_OP_BOOT_WP:
		ret = mmc_switch(card, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_BOOT_WP,
				 card->ext_csd.boot_ro_lock |
				 EXT_CSD_BOOT_WP_B_PWR_WP_EN,
				 card->ext_csd.part_time);
		if (ret)
			pr_err("%s: Locking boot partition ro until next power on failed: %d\n",
			       md->disk->disk_name, ret);
		else
			card->ext_csd.boot_ro_lock |=
				EXT_CSD_BOOT_WP_B_PWR_WP_EN;
		break;
	case MMC_DRV_OP_GET_CARD_STATUS:
		ret = mmc_send_status(card, &status);
		if (!ret)
			ret = status;
		break;
	case MMC_DRV_OP_GET_EXT_CSD:
		ext_csd = mq_rq->drv_op_data;
		ret = mmc_get_ext_csd(card, ext_csd);
		break;
	default:
		pr_err("%s: unknown driver specific operation\n",
		       md->disk->disk_name);
		ret = -EINVAL;
		break;
	}
	mq_rq->drv_op_result = ret;
	blk_end_request_all(req, ret ? BLK_STS_IOERR : BLK_STS_OK);
}

static void mmc_blk_issue_discard_rq(struct mmc_queue *mq, struct request *req)
{
	struct mmc_blk_data *md = mq->blkdata;
	struct mmc_card *card = md->queue.card;
	unsigned int from, nr, arg;
	int err = 0, type = MMC_BLK_DISCARD;
	blk_status_t status = BLK_STS_OK;

	if (!mmc_can_erase(card)) {
		status = BLK_STS_NOTSUPP;
		goto fail;
	}

	from = blk_rq_pos(req);
	nr = blk_rq_sectors(req);

	if (mmc_can_discard(card))
		arg = MMC_DISCARD_ARG;
	else if (mmc_can_trim(card))
		arg = MMC_TRIM_ARG;
	else
		arg = MMC_ERASE_ARG;
	do {
		err = 0;
		if (card->quirks & MMC_QUIRK_INAND_CMD38) {
			err = mmc_switch(card, EXT_CSD_CMD_SET_NORMAL,
					 INAND_CMD38_ARG_EXT_CSD,
					 arg == MMC_TRIM_ARG ?
					 INAND_CMD38_ARG_TRIM :
					 INAND_CMD38_ARG_ERASE,
					 0);
		}
		if (!err)
			err = mmc_erase(card, from, nr, arg);
	} while (err == -EIO && !mmc_blk_reset(md, card->host, type));
	if (err)
		status = BLK_STS_IOERR;
	else
		mmc_blk_reset_success(md, type);
fail:
	blk_end_request(req, status, blk_rq_bytes(req));
}

static void mmc_blk_issue_secdiscard_rq(struct mmc_queue *mq,
				       struct request *req)
{
	struct mmc_blk_data *md = mq->blkdata;
	struct mmc_card *card = md->queue.card;
	unsigned int from, nr, arg;
	int err = 0, type = MMC_BLK_SECDISCARD;
	blk_status_t status = BLK_STS_OK;

	if (!(mmc_can_secure_erase_trim(card))) {
		status = BLK_STS_NOTSUPP;
		goto out;
	}

	from = blk_rq_pos(req);
	nr = blk_rq_sectors(req);

	if (mmc_can_trim(card) && !mmc_erase_group_aligned(card, from, nr))
		arg = MMC_SECURE_TRIM1_ARG;
	else
		arg = MMC_SECURE_ERASE_ARG;

retry:
	if (card->quirks & MMC_QUIRK_INAND_CMD38) {
		err = mmc_switch(card, EXT_CSD_CMD_SET_NORMAL,
				 INAND_CMD38_ARG_EXT_CSD,
				 arg == MMC_SECURE_TRIM1_ARG ?
				 INAND_CMD38_ARG_SECTRIM1 :
				 INAND_CMD38_ARG_SECERASE,
				 0);
		if (err)
			goto out_retry;
	}

	err = mmc_erase(card, from, nr, arg);
	if (err == -EIO)
		goto out_retry;
	if (err) {
		status = BLK_STS_IOERR;
		goto out;
	}

	if (arg == MMC_SECURE_TRIM1_ARG) {
		if (card->quirks & MMC_QUIRK_INAND_CMD38) {
			err = mmc_switch(card, EXT_CSD_CMD_SET_NORMAL,
					 INAND_CMD38_ARG_EXT_CSD,
					 INAND_CMD38_ARG_SECTRIM2,
					 0);
			if (err)
				goto out_retry;
		}

		err = mmc_erase(card, from, nr, MMC_SECURE_TRIM2_ARG);
		if (err == -EIO)
			goto out_retry;
		if (err) {
			status = BLK_STS_IOERR;
			goto out;
		}
	}

out_retry:
	if (err && !mmc_blk_reset(md, card->host, type))
		goto retry;
	if (!err)
		mmc_blk_reset_success(md, type);
out:
	blk_end_request(req, status, blk_rq_bytes(req));
}

static void mmc_blk_issue_flush(struct mmc_queue *mq, struct request *req)
{
	struct mmc_blk_data *md = mq->blkdata;
	struct mmc_card *card = md->queue.card;
	int ret = 0;

	ret = mmc_flush_cache(card);
	if (ret == -ENODEV) {
		pr_err("%s: %s: restart mmc card",
				req->rq_disk->disk_name, __func__);
		if (mmc_blk_reset(md, card->host, MMC_BLK_FLUSH))
			pr_err("%s: %s: fail to restart mmc",
				req->rq_disk->disk_name, __func__);
		else
			mmc_blk_reset_success(md, MMC_BLK_FLUSH);
	}

	if (ret) {
		pr_err("%s: %s: notify flush error to upper layers",
				req->rq_disk->disk_name, __func__);
		ret = -EIO;
	}
#ifdef CONFIG_MMC_SIMULATE_MAX_SPEED
	else if (atomic_read(&mq->cache_size)) {
		long used = mmc_blk_cache_used(mq, jiffies);

		if (used) {
			int speed = atomic_read(&mq->max_write_speed);

			if (speed_valid(speed)) {
				unsigned long msecs = jiffies_to_msecs(
					size_and_speed_to_jiffies(
						used, speed));
				if (msecs)
					msleep(msecs);
			}
		}
	}
#endif
	blk_end_request_all(req, ret ? BLK_STS_IOERR : BLK_STS_OK);
}

/*
 * Reformat current write as a reliable write, supporting
 * both legacy and the enhanced reliable write MMC cards.
 * In each transfer we'll handle only as much as a single
 * reliable write can handle, thus finish the request in
 * partial completions.
 */
static inline void mmc_apply_rel_rw(struct mmc_blk_request *brq,
				    struct mmc_card *card,
				    struct request *req)
{
	if (!(card->ext_csd.rel_param & EXT_CSD_WR_REL_PARAM_EN)) {
		/* Legacy mode imposes restrictions on transfers. */
		if (!IS_ALIGNED(blk_rq_pos(req), card->ext_csd.rel_sectors))
			brq->data.blocks = 1;

		if (brq->data.blocks > card->ext_csd.rel_sectors)
			brq->data.blocks = card->ext_csd.rel_sectors;
		else if (brq->data.blocks < card->ext_csd.rel_sectors)
			brq->data.blocks = 1;
	}
}

#define CMD_ERRORS							\
	(R1_OUT_OF_RANGE |	/* Command argument out of range */	\
	 R1_ADDRESS_ERROR |	/* Misaligned address */		\
	 R1_BLOCK_LEN_ERROR |	/* Transferred block length incorrect */\
	 R1_WP_VIOLATION |	/* Tried to write to protected block */	\
	 R1_CARD_ECC_FAILED |	/* Card ECC failed */			\
	 R1_CC_ERROR |		/* Card controller error */		\
	 R1_ERROR)		/* General/unknown error */

static void mmc_blk_eval_resp_error(struct mmc_blk_request *brq)
{
	u32 val;

	/*
	 * Per the SD specification(physical layer version 4.10)[1],
	 * section 4.3.3, it explicitly states that "When the last
	 * block of user area is read using CMD18, the host should
	 * ignore OUT_OF_RANGE error that may occur even the sequence
	 * is correct". And JESD84-B51 for eMMC also has a similar
	 * statement on section 6.8.3.
	 *
	 * Multiple block read/write could be done by either predefined
	 * method, namely CMD23, or open-ending mode. For open-ending mode,
	 * we should ignore the OUT_OF_RANGE error as it's normal behaviour.
	 *
	 * However the spec[1] doesn't tell us whether we should also
	 * ignore that for predefined method. But per the spec[1], section
	 * 4.15 Set Block Count Command, it says"If illegal block count
	 * is set, out of range error will be indicated during read/write
	 * operation (For example, data transfer is stopped at user area
	 * boundary)." In another word, we could expect a out of range error
	 * in the response for the following CMD18/25. And if argument of
	 * CMD23 + the argument of CMD18/25 exceed the max number of blocks,
	 * we could also expect to get a -ETIMEDOUT or any error number from
	 * the host drivers due to missing data response(for write)/data(for
	 * read), as the cards will stop the data transfer by itself per the
	 * spec. So we only need to check R1_OUT_OF_RANGE for open-ending mode.
	 */

	if (!brq->stop.error) {
		bool oor_with_open_end;
		/* If there is no error yet, check R1 response */

		val = brq->stop.resp[0] & CMD_ERRORS;
		oor_with_open_end = val & R1_OUT_OF_RANGE && !brq->mrq.sbc;

		if (val && !oor_with_open_end)
			brq->stop.error = -EIO;
	}
}

static enum mmc_blk_status mmc_blk_err_check(struct mmc_card *card,
					     struct mmc_async_req *areq)
{
	struct mmc_queue_req *mq_mrq = container_of(areq, struct mmc_queue_req,
						    areq);
	struct mmc_blk_request *brq = &mq_mrq->brq;
	struct request *req = mmc_queue_req_to_req(mq_mrq);
	int need_retune = card->host->need_retune;
	bool ecc_err = false;
	bool gen_err = false;

	/*
	 * sbc.error indicates a problem with the set block count
	 * command.  No data will have been transferred.
	 *
	 * cmd.error indicates a problem with the r/w command.  No
	 * data will have been transferred.
	 *
	 * stop.error indicates a problem with the stop command.  Data
	 * may have been transferred, or may still be transferring.
	 */

	mmc_blk_eval_resp_error(brq);

	if (brq->sbc.error || brq->cmd.error ||
	    brq->stop.error || brq->data.error) {
		switch (mmc_blk_cmd_recovery(card, req, brq, &ecc_err, &gen_err)) {
		case ERR_RETRY:
			return MMC_BLK_RETRY;
		case ERR_ABORT:
			return MMC_BLK_ABORT;
		case ERR_NOMEDIUM:
			return MMC_BLK_NOMEDIUM;
		case ERR_CONTINUE:
			break;
		}
	}

	/*
	 * Check for errors relating to the execution of the
	 * initial command - such as address errors.  No data
	 * has been transferred.
	 */
	if (brq->cmd.resp[0] & CMD_ERRORS) {
		pr_err("%s: r/w command failed, status = %#x\n",
		       req->rq_disk->disk_name, brq->cmd.resp[0]);
		return MMC_BLK_ABORT;
	}

	/* Check execution mode errors. If stop cmd was sent, these
	 * errors would be reported in response to it. In this case
	 * the execution is retried using single-block read. */
	if (brq->stop.resp[0] & EXE_ERRORS) {
		pr_err("%s: error during r/w command, stop response %#x\n",
		       req->rq_disk->disk_name, brq->stop.resp[0]);
		return MMC_BLK_RETRY_SINGLE;
	}

	/*
	 * Everything else is either success, or a data error of some
	 * kind.  If it was a write, we may have transitioned to
	 * program mode, which we have to wait for it to complete.
	 * If pre defined block count (CMD23) was used, no stop
	 * cmd was sent and we need to read status to check
	 * for errors during cmd execution.
	 */
	if (!mmc_host_is_spi(card->host) &&
	    (rq_data_dir(req) != READ || brq->sbc.opcode == MMC_SET_BLOCK_COUNT)) {
		int err;

		/* Check stop command response */
		if (brq->stop.resp[0] & R1_ERROR) {
			pr_err("%s: %s: general error sending stop command, stop cmd response %#x\n",
			       req->rq_disk->disk_name, __func__,
			       brq->stop.resp[0]);
			gen_err = true;
		}

		err = card_busy_detect(card, MMC_BLK_TIMEOUT_MS, false, req,
					&gen_err);
		if (err)
			return MMC_BLK_CMD_ERR;
	}

	/* if general error occurs, retry the write operation. */
	if (gen_err) {
		pr_warn("%s: retrying write for general error\n",
				req->rq_disk->disk_name);
		return MMC_BLK_RETRY;
	}

	/* Some errors (ECC) are flagged on the next commmand, so check stop, too */
	if (brq->data.error || brq->stop.error) {
		if (need_retune && !brq->retune_retry_done) {
			pr_debug("%s: retrying because a re-tune was needed\n",
				 req->rq_disk->disk_name);
			brq->retune_retry_done = 1;
			return MMC_BLK_RETRY;
		}
		pr_err("%s: error %d transferring data, sector %u, nr %u, cmd response %#x, card status %#x\n",
		       req->rq_disk->disk_name, brq->data.error ?: brq->stop.error,
		       (unsigned)blk_rq_pos(req),
		       (unsigned)blk_rq_sectors(req),
		       brq->cmd.resp[0], brq->stop.resp[0]);

		if (rq_data_dir(req) == READ) {
			if (ecc_err)
				return MMC_BLK_ECC_ERR;
			return MMC_BLK_DATA_ERR;
		} else {
			return MMC_BLK_CMD_ERR;
		}
	}

	if (!brq->data.bytes_xfered)
		return MMC_BLK_RETRY;

	if (blk_rq_bytes(req) != brq->data.bytes_xfered)
		return MMC_BLK_PARTIAL;

	return MMC_BLK_SUCCESS;
}

static void mmc_blk_data_prep(struct mmc_queue *mq, struct mmc_queue_req *mqrq,
			      int disable_multi, bool *do_rel_wr,
			      bool *do_data_tag)
{
	struct mmc_blk_data *md = mq->blkdata;
	struct mmc_card *card = md->queue.card;
	struct mmc_blk_request *brq = &mqrq->brq;
	struct request *req = mmc_queue_req_to_req(mqrq);

	/*
	 * Reliable writes are used to implement Forced Unit Access and
	 * are supported only on MMCs.
	 */
	*do_rel_wr = (req->cmd_flags & REQ_FUA) &&
		     rq_data_dir(req) == WRITE &&
		     (md->flags & MMC_BLK_REL_WR);

	memset(brq, 0, sizeof(struct mmc_blk_request));

	brq->mrq.data = &brq->data;

	brq->stop.opcode = MMC_STOP_TRANSMISSION;
	brq->stop.arg = 0;

	if (rq_data_dir(req) == READ) {
		brq->data.flags = MMC_DATA_READ;
		brq->stop.flags = MMC_RSP_SPI_R1 | MMC_RSP_R1 | MMC_CMD_AC;
	} else {
		brq->data.flags = MMC_DATA_WRITE;
		brq->stop.flags = MMC_RSP_SPI_R1B | MMC_RSP_R1B | MMC_CMD_AC;
	}

	brq->data.blksz = 512;
	brq->data.blocks = blk_rq_sectors(req);
	brq->data.fault_injected = false;
	/*
	 * The block layer doesn't support all sector count
	 * restrictions, so we need to be prepared for too big
	 * requests.
	 */
	if (brq->data.blocks > card->host->max_blk_count)
		brq->data.blocks = card->host->max_blk_count;

	if (brq->data.blocks > 1) {
		/*
		 * Some SD cards in SPI mode return a CRC error or even lock up
		 * completely when trying to read the last block using a
		 * multiblock read command.
		 */
		if (mmc_host_is_spi(card->host) && (rq_data_dir(req) == READ) &&
		    (blk_rq_pos(req) + blk_rq_sectors(req) ==
		     get_capacity(md->disk)))
			brq->data.blocks--;

		/*
		 * After a read error, we redo the request one sector
		 * at a time in order to accurately determine which
		 * sectors can be read successfully.
		 */
		if (disable_multi)
			brq->data.blocks = 1;

		/*
		 * Some controllers have HW issues while operating
		 * in multiple I/O mode
		 */
		if (card->host->ops->multi_io_quirk)
			brq->data.blocks = card->host->ops->multi_io_quirk(card,
						(rq_data_dir(req) == READ) ?
						MMC_DATA_READ : MMC_DATA_WRITE,
						brq->data.blocks);
	}

	if (*do_rel_wr)
		mmc_apply_rel_rw(brq, card, req);

	/*
	 * Data tag is used only during writing meta data to speed
	 * up write and any subsequent read of this meta data
	 */
	*do_data_tag = card->ext_csd.data_tag_unit_size &&
		       (req->cmd_flags & REQ_META) &&
		       (rq_data_dir(req) == WRITE) &&
		       ((brq->data.blocks * brq->data.blksz) >=
			card->ext_csd.data_tag_unit_size);

	mmc_set_data_timeout(&brq->data, card);

	brq->data.sg = mqrq->sg;
	brq->data.sg_len = mmc_queue_map_sg(mq, mqrq);

	/*
	 * Adjust the sg list so it is the same size as the
	 * request.
	 */
	if (brq->data.blocks != blk_rq_sectors(req)) {
		int i, data_size = brq->data.blocks << 9;
		struct scatterlist *sg;

		for_each_sg(brq->data.sg, sg, brq->data.sg_len, i) {
			data_size -= sg->length;
			if (data_size <= 0) {
				sg->length += data_size;
				i++;
				break;
			}
		}
		brq->data.sg_len = i;
	}

	mqrq->areq.mrq = &brq->mrq;
	mqrq->areq.mrq->req = req;
	mqrq->areq.err_check = mmc_blk_err_check;
}

static void mmc_blk_rw_rq_prep(struct mmc_queue_req *mqrq,
			       struct mmc_card *card,
			       int disable_multi,
			       struct mmc_queue *mq)
{
	u32 readcmd, writecmd;
	struct mmc_blk_request *brq = &mqrq->brq;
	struct request *req = mmc_queue_req_to_req(mqrq);
	struct mmc_blk_data *md = mq->blkdata;
	bool do_rel_wr, do_data_tag;

	mmc_blk_data_prep(mq, mqrq, disable_multi, &do_rel_wr, &do_data_tag);

	brq->mrq.cmd = &brq->cmd;

	brq->cmd.arg = blk_rq_pos(req);
	if (!mmc_card_blockaddr(card))
		brq->cmd.arg <<= 9;
	brq->cmd.flags = MMC_RSP_SPI_R1 | MMC_RSP_R1 | MMC_CMD_ADTC;

	if (brq->data.blocks > 1 || do_rel_wr) {
		/* SPI multiblock writes terminate using a special
		 * token, not a STOP_TRANSMISSION request.
		 */
		if (!mmc_host_is_spi(card->host) ||
		    rq_data_dir(req) == READ)
			brq->mrq.stop = &brq->stop;
		readcmd = MMC_READ_MULTIPLE_BLOCK;
		writecmd = MMC_WRITE_MULTIPLE_BLOCK;
	} else {
		brq->mrq.stop = NULL;
		readcmd = MMC_READ_SINGLE_BLOCK;
		writecmd = MMC_WRITE_BLOCK;
	}
	brq->cmd.opcode = rq_data_dir(req) == READ ? readcmd : writecmd;

	/*
	 * Pre-defined multi-block transfers are preferable to
	 * open ended-ones (and necessary for reliable writes).
	 * However, it is not sufficient to just send CMD23,
	 * and avoid the final CMD12, as on an error condition
	 * CMD12 (stop) needs to be sent anyway. This, coupled
	 * with Auto-CMD23 enhancements provided by some
	 * hosts, means that the complexity of dealing
	 * with this is best left to the host. If CMD23 is
	 * supported by card and host, we'll fill sbc in and let
	 * the host deal with handling it correctly. This means
	 * that for hosts that don't expose MMC_CAP_CMD23, no
	 * change of behavior will be observed.
	 *
	 * N.B: Some MMC cards experience perf degradation.
	 * We'll avoid using CMD23-bounded multiblock writes for
	 * these, while retaining features like reliable writes.
	 */
	if ((md->flags & MMC_BLK_CMD23) && mmc_op_multi(brq->cmd.opcode) &&
	    (do_rel_wr || !(card->quirks & MMC_QUIRK_BLK_NO_CMD23) ||
	     do_data_tag)) {
		brq->sbc.opcode = MMC_SET_BLOCK_COUNT;
		brq->sbc.arg = brq->data.blocks |
			(do_rel_wr ? (1 << 31) : 0) |
			(do_data_tag ? (1 << 29) : 0);
		brq->sbc.flags = MMC_RSP_R1 | MMC_CMD_AC;
		brq->mrq.sbc = &brq->sbc;
	}

	mqrq->areq.err_check = mmc_blk_err_check;
}

static bool mmc_blk_rw_cmd_err(struct mmc_blk_data *md, struct mmc_card *card,
				struct mmc_blk_request *brq,
				struct request *req,
				bool old_req_pending)
{
	bool req_pending;

	/*
	 * If this is an SD card and we're writing, we can first
	 * mark the known good sectors as ok.
	 *
	 * If the card is not SD, we can still ok written sectors
	 * as reported by the controller (which might be less than
	 * the real number of written sectors, but never more).
	 */
	if (mmc_card_sd(card)) {
		u32 blocks;
		int err;

		if (!brq->data.fault_injected) {
			err = mmc_sd_num_wr_blocks(card, &blocks);
			if (err)
				req_pending = old_req_pending;
			else
				req_pending = blk_end_request(req,
						BLK_STS_OK, blocks << 9);
		} else {
			req_pending = blk_end_request(req, BLK_STS_OK,
					brq->data.bytes_xfered);
		}
	} else {
		req_pending = blk_end_request(req, BLK_STS_OK,
					brq->data.bytes_xfered);
	}
	return req_pending;
}

static int mmc_blk_cmdq_switch(struct mmc_card *card,
			       struct mmc_blk_data *md, bool enable)
{
	int ret = 0;
	bool cmdq_mode = !!mmc_card_cmdq(card);
	struct mmc_host *host = card->host;
	struct mmc_cmdq_context_info *ctx = &host->cmdq_ctx;

	if (!(card->host->caps2 & MMC_CAP2_CMD_QUEUE) ||
	    !card->ext_csd.cmdq_support ||
	    (enable && !(md->flags & MMC_BLK_CMD_QUEUE)) ||
	    (cmdq_mode == enable))
		return 0;

	if (enable) {
		ret = mmc_set_blocklen(card, MMC_CARD_CMDQ_BLK_SIZE);
		if (ret) {
			pr_err("%s: failed (%d) to set block-size to %d\n",
			       __func__, ret, MMC_CARD_CMDQ_BLK_SIZE);
			goto out;
		}

	} else {
		if (!test_bit(CMDQ_STATE_HALT, &ctx->curr_state)) {
			ret = mmc_cmdq_halt(host, true);
			if (ret) {
				pr_err("%s: halt: failed: %d\n",
					mmc_hostname(host), ret);
				goto out;
			}
		}
	}

	ret = mmc_switch(card, EXT_CSD_CMD_SET_NORMAL,
			 EXT_CSD_CMDQ_MODE_EN, enable,
			 card->ext_csd.generic_cmd6_time);
	if (ret) {
		pr_err("%s: cmdq mode %sable failed %d\n",
		       md->disk->disk_name, enable ? "en" : "dis", ret);
		goto out;
	}

	if (enable)
		mmc_card_set_cmdq(card);
	else
		mmc_card_clr_cmdq(card);
out:
	return ret;
}

/* prepare for non-data commands */
static struct mmc_cmdq_req *mmc_cmdq_prep_dcmd(
		struct mmc_queue_req *mqrq, struct mmc_queue *mq)
{
	struct request *req = mmc_queue_req_to_req(mqrq);
	struct mmc_cmdq_req *cmdq_req = &mqrq->cmdq_req;

	memset(&mqrq->cmdq_req, 0, sizeof(struct mmc_cmdq_req));

	cmdq_req->mrq.data = NULL;
	cmdq_req->cmd_flags = req->cmd_flags;
	cmdq_req->mrq.req = req;
	req->special = mqrq;
	cmdq_req->cmdq_req_flags |= DCMD;
	cmdq_req->mrq.cmdq_req = cmdq_req;

	return &mqrq->cmdq_req;
}

static struct mmc_cmdq_req *mmc_blk_cmdq_prep_discard_req(struct mmc_queue *mq,
						struct request *req)
{
	struct mmc_blk_data *md = mq->blkdata;
	struct mmc_card *card = md->queue.card;
	struct mmc_host *host = card->host;
	struct mmc_cmdq_context_info *ctx_info = &host->cmdq_ctx;
	struct mmc_cmdq_req *cmdq_req;
	struct mmc_queue_req *active_mqrq;

	active_mqrq = req_to_mmc_queue_req(req);

	cmdq_req = mmc_cmdq_prep_dcmd(active_mqrq, mq);
	cmdq_req->cmdq_req_flags |= QBR;
	cmdq_req->mrq.cmd = &cmdq_req->cmd;
	cmdq_req->tag = req->tag;

	init_completion(&cmdq_req->mrq.completion);
	BUG_ON(req->tag > card->ext_csd.cmdq_depth);
	BUG_ON(test_and_set_bit(req->tag, &host->cmdq_ctx.active_reqs));
	set_bit(CMDQ_STATE_DCMD_ACTIVE, &ctx_info->curr_state);
	return cmdq_req;
}

static int mmc_blk_cmdq_issue_discard_rq(struct mmc_queue *mq,
					struct request *req)
{
	struct mmc_blk_data *md = mq->blkdata;
	struct mmc_card *card = md->queue.card;
	struct mmc_cmdq_req *cmdq_req = NULL;
	unsigned int from, nr, arg;
	int err = 0;

	if (!mmc_can_erase(card)) {
		err = -EOPNOTSUPP;
		blk_end_request(req, err, blk_rq_bytes(req));
		goto out;
	}

	from = blk_rq_pos(req);
	nr = blk_rq_sectors(req);

	if (mmc_can_discard(card))
		arg = MMC_DISCARD_ARG;
	else if (mmc_can_trim(card))
		arg = MMC_TRIM_ARG;
	else
		arg = MMC_ERASE_ARG;

	cmdq_req = mmc_blk_cmdq_prep_discard_req(mq, req);
	if (card->quirks & MMC_QUIRK_INAND_CMD38) {
		__mmc_switch_cmdq_mode(cmdq_req->mrq.cmd,
				EXT_CSD_CMD_SET_NORMAL,
				INAND_CMD38_ARG_EXT_CSD,
				arg == MMC_TRIM_ARG ?
				INAND_CMD38_ARG_TRIM :
				INAND_CMD38_ARG_ERASE,
				0, true, false);
		err = mmc_cmdq_wait_for_dcmd(card->host, cmdq_req);
		if (err)
			goto clear_dcmd;
	}
	err = mmc_cmdq_erase(cmdq_req, card, from, nr, arg);
clear_dcmd:
	if (err != -EAGAIN) {
		mmc_host_clk_hold(card->host);
		blk_complete_request(req);
	} else {
		pr_err("%s: err(%d) handled by cmdq-error handler\n",
			__func__, err);
	}
out:
	return err ? 1 : 0;
}

static int mmc_blk_cmdq_issue_secdiscard_rq(struct mmc_queue *mq,
				       struct request *req)
{
	struct mmc_blk_data *md = mq->blkdata;
	struct mmc_card *card = md->queue.card;
	struct mmc_cmdq_req *cmdq_req = NULL;
	unsigned int from, nr, arg;
	int err = 0;

	if (!(mmc_can_secure_erase_trim(card))) {
		err = -EOPNOTSUPP;
		blk_end_request(req, err, blk_rq_bytes(req));
		goto out;
	}

	from = blk_rq_pos(req);
	nr = blk_rq_sectors(req);

	if (mmc_can_trim(card) && !mmc_erase_group_aligned(card, from, nr))
		arg = MMC_SECURE_TRIM1_ARG;
	else
		arg = MMC_SECURE_ERASE_ARG;

	cmdq_req = mmc_blk_cmdq_prep_discard_req(mq, req);
	if (card->quirks & MMC_QUIRK_INAND_CMD38) {
		__mmc_switch_cmdq_mode(cmdq_req->mrq.cmd,
				EXT_CSD_CMD_SET_NORMAL,
				INAND_CMD38_ARG_EXT_CSD,
				arg == MMC_SECURE_TRIM1_ARG ?
				INAND_CMD38_ARG_SECTRIM1 :
				INAND_CMD38_ARG_SECERASE,
				0, true, false);
		err = mmc_cmdq_wait_for_dcmd(card->host, cmdq_req);
		if (err)
			goto clear_dcmd;
	}

	err = mmc_cmdq_erase(cmdq_req, card, from, nr, arg);
	if (err)
		goto clear_dcmd;

	if (arg == MMC_SECURE_TRIM1_ARG) {
		if (card->quirks & MMC_QUIRK_INAND_CMD38) {
			__mmc_switch_cmdq_mode(cmdq_req->mrq.cmd,
					EXT_CSD_CMD_SET_NORMAL,
					INAND_CMD38_ARG_EXT_CSD,
					INAND_CMD38_ARG_SECTRIM2,
					0, true, false);
			err = mmc_cmdq_wait_for_dcmd(card->host, cmdq_req);
			if (err)
				goto clear_dcmd;
		}

		err = mmc_cmdq_erase(cmdq_req, card, from, nr,
				MMC_SECURE_TRIM2_ARG);
	}
clear_dcmd:
	if (err != -EAGAIN) {
		mmc_host_clk_hold(card->host);
		blk_complete_request(req);
	} else {
		pr_err("%s: err(%d) handled by cmdq-error handler\n",
			__func__, err);
	}
out:
	return err ? 1 : 0;
}

static int mmc_blk_cmdq_start_req(struct mmc_host *host,
				  struct mmc_cmdq_req *cmdq_req)
{
	struct mmc_request *mrq = &cmdq_req->mrq;

	mrq->done = mmc_blk_cmdq_req_done;
	return mmc_cmdq_start_req(host, cmdq_req);
}

#define IS_RT_CLASS_REQ(x)     \
	(IOPRIO_PRIO_CLASS(req_get_ioprio(x)) == IOPRIO_CLASS_RT)

static struct mmc_cmdq_req *mmc_blk_cmdq_rw_prep(
		struct mmc_queue_req *mqrq, struct mmc_queue *mq)
{
	struct mmc_card *card = mq->card;
	struct request *req = mmc_queue_req_to_req(mqrq);
	struct mmc_blk_data *md = mq->blkdata;
	bool do_rel_wr = mmc_req_rel_wr(req) && (md->flags & MMC_BLK_REL_WR);
	bool do_data_tag;
	bool read_dir = (rq_data_dir(req) == READ);
	bool prio = IS_RT_CLASS_REQ(req);
	struct mmc_cmdq_req *cmdq_rq = &mqrq->cmdq_req;

	memset(&mqrq->cmdq_req, 0, sizeof(struct mmc_cmdq_req));

	cmdq_rq->tag = req->tag;
	if (read_dir) {
		cmdq_rq->cmdq_req_flags |= DIR;
		cmdq_rq->data.flags = MMC_DATA_READ;
	} else {
		cmdq_rq->data.flags = MMC_DATA_WRITE;
	}
	if (prio)
		cmdq_rq->cmdq_req_flags |= PRIO;

	if (do_rel_wr)
		cmdq_rq->cmdq_req_flags |= REL_WR;

	cmdq_rq->data.blocks = blk_rq_sectors(req);
	cmdq_rq->blk_addr = blk_rq_pos(req);
	cmdq_rq->data.blksz = MMC_CARD_CMDQ_BLK_SIZE;

	mmc_set_data_timeout(&cmdq_rq->data, card);

	do_data_tag = (card->ext_csd.data_tag_unit_size) &&
		(req->cmd_flags & REQ_META) &&
		(rq_data_dir(req) == WRITE) &&
		((cmdq_rq->data.blocks * cmdq_rq->data.blksz) >=
		 card->ext_csd.data_tag_unit_size);
	if (do_data_tag)
		cmdq_rq->cmdq_req_flags |= DAT_TAG;
	cmdq_rq->data.sg = mqrq->sg;
	cmdq_rq->data.sg_len = mmc_queue_map_sg(mq, mqrq);

	/*
	 * Adjust the sg list so it is the same size as the
	 * request.
	 */
	if (cmdq_rq->data.blocks > card->host->max_blk_count)
		cmdq_rq->data.blocks = card->host->max_blk_count;

	if (cmdq_rq->data.blocks != blk_rq_sectors(req)) {
		int i, data_size = cmdq_rq->data.blocks << 9;
		struct scatterlist *sg;

		for_each_sg(cmdq_rq->data.sg, sg, cmdq_rq->data.sg_len, i) {
			data_size -= sg->length;
			if (data_size <= 0) {
				sg->length += data_size;
				i++;
				break;
			}
		}
		cmdq_rq->data.sg_len = i;
	}

	mqrq->cmdq_req.cmd_flags = req->cmd_flags;
	mqrq->cmdq_req.mrq.req = req;
	mqrq->cmdq_req.mrq.cmdq_req = &mqrq->cmdq_req;
	mqrq->cmdq_req.mrq.data = &mqrq->cmdq_req.data;
	req->special = mqrq;

	pr_debug("%s: %s: mrq: 0x%p req: 0x%p mqrq: 0x%p bytes to xf: %d\n",
		mmc_hostname(card->host), __func__, &mqrq->cmdq_req.mrq,
		req, mqrq, (cmdq_rq->data.blocks * cmdq_rq->data.blksz));
	pr_debug("%s: %s: mmc_cmdq_req: 0x%p card-addr: 0x%08x dir(r-1/w-0): %d\n",
		mmc_hostname(card->host), __func__,
		cmdq_rq, cmdq_rq->blk_addr,
		(cmdq_rq->cmdq_req_flags & DIR) ? 1 : 0);

	return &mqrq->cmdq_req;
}

static void mmc_blk_cmdq_requeue_rw_rq(struct mmc_queue *mq,
				struct request *req)
{
	struct request_queue *q = req->q;

	spin_lock_irq(q->queue_lock);
	blk_requeue_request(q, req);
	spin_unlock_irq(q->queue_lock);
}

static int mmc_blk_cmdq_issue_rw_rq(struct mmc_queue *mq, struct request *req)
{
	struct mmc_queue_req *active_mqrq;
	struct mmc_card *card = mq->card;
	struct mmc_host *host = card->host;
	struct mmc_cmdq_context_info *ctx = &host->cmdq_ctx;
	struct mmc_cmdq_req *mc_rq;
	u8 active_small_sector_read = 0;
	int ret = 0;

	mmc_cmdq_up_rwsem(host);
	mmc_deferred_scaling(host);
	ret = mmc_cmdq_down_rwsem(host, req);
	if (ret)
		return ret;

	mmc_cmdq_clk_scaling_start_busy(host, true);

	BUG_ON((req->tag < 0) || (req->tag > card->ext_csd.cmdq_depth));
	BUG_ON(test_and_set_bit(req->tag, &host->cmdq_ctx.data_active_reqs));
	BUG_ON(test_and_set_bit(req->tag, &host->cmdq_ctx.active_reqs));

	active_mqrq = req_to_mmc_queue_req(req);

	mc_rq = mmc_blk_cmdq_rw_prep(active_mqrq, mq);

	if (card->quirks & MMC_QUIRK_CMDQ_EMPTY_BEFORE_DCMD) {
		unsigned int sectors = blk_rq_sectors(req);

		if (((sectors > 0) && (sectors < 8))
		    && (rq_data_dir(req) == READ))
			active_small_sector_read = 1;
	}
	ret = mmc_blk_cmdq_start_req(card->host, mc_rq);
	if (!ret && active_small_sector_read)
		host->cmdq_ctx.active_small_sector_read_reqs++;
	/*
	 * When in SVS2 on low load scenario and there are lots of requests
	 * queued for CMDQ we need to wait till the queue is empty to scale
	 * back up to Nominal even if there is a sudden increase in load.
	 * This impacts performance where lots of IO get executed in SVS2
	 * frequency since the queue is full. As SVS2 is a low load use case
	 * we can serialize the requests and not queue them in parallel
	 * without impacting other use cases. This makes sure the queue gets
	 * empty faster and we will be able to scale up to Nominal frequency
	 * when needed.
	 */

	if (!ret && (host->clk_scaling.state == MMC_LOAD_LOW)) {

		mmc_cmdq_up_rwsem(host);
		ret = wait_event_interruptible_timeout(ctx->queue_empty_wq,
			(!ctx->active_reqs &&
			!test_bit(CMDQ_STATE_ERR, &ctx->curr_state)),
			msecs_to_jiffies(MMC_CMDQ_WAIT_EVENT_TIMEOUT_MS));
		if (!ret)
			pr_err("%s: queue_empty_wq timeout case? ret = (%d)\n",
				__func__, ret);
		ret = 0;
		mmc_cmdq_down_rwsem(host, NULL);

	}

	if (ret) {
		/* clear pending request */
		WARN_ON(!test_and_clear_bit(req->tag,
				&host->cmdq_ctx.data_active_reqs));
		WARN_ON(!test_and_clear_bit(req->tag,
				&host->cmdq_ctx.active_reqs));
		mmc_cmdq_clk_scaling_stop_busy(host, true, false);
	}

	return ret;
}

/*
 * Issues a flush (dcmd) request
 */
int mmc_blk_cmdq_issue_flush_rq(struct mmc_queue *mq, struct request *req)
{
	int err;
	struct mmc_queue_req *active_mqrq;
	struct mmc_card *card = mq->card;
	struct mmc_host *host;
	struct mmc_cmdq_req *cmdq_req;
	struct mmc_cmdq_context_info *ctx_info;

	BUG_ON(!card);
	host = card->host;
	BUG_ON(!host);
	BUG_ON(req->tag > card->ext_csd.cmdq_depth);
	BUG_ON(test_and_set_bit(req->tag, &host->cmdq_ctx.active_reqs));

	ctx_info = &host->cmdq_ctx;

	set_bit(CMDQ_STATE_DCMD_ACTIVE, &ctx_info->curr_state);

	active_mqrq = req_to_mmc_queue_req(req);

	cmdq_req = mmc_cmdq_prep_dcmd(active_mqrq, mq);
	cmdq_req->cmdq_req_flags |= QBR;
	cmdq_req->mrq.cmd = &cmdq_req->cmd;
	cmdq_req->tag = req->tag;

	err = mmc_cmdq_prepare_flush(cmdq_req->mrq.cmd);
	if (err) {
		pr_err("%s: failed (%d) preparing flush req\n",
		       mmc_hostname(host), err);
		return err;
	}
	err = mmc_blk_cmdq_start_req(card->host, cmdq_req);
	return err;
}
EXPORT_SYMBOL(mmc_blk_cmdq_issue_flush_rq);

static void mmc_blk_cmdq_reset(struct mmc_host *host, bool clear_all)
{
	int err = 0;

		if (mmc_cmdq_halt(host, true)) {
			pr_err("%s: halt failed\n", mmc_hostname(host));
			goto reset;
		}

	if (clear_all)
		mmc_cmdq_discard_queue(host, 0);
reset:
	mmc_host_clk_hold(host);
	host->cmdq_ops->disable(host, true);
	mmc_host_clk_release(host);
	err = mmc_cmdq_hw_reset(host);
	if (err && err != -EOPNOTSUPP) {
		pr_err("%s: failed to cmdq_hw_reset err = %d\n",
				mmc_hostname(host), err);
		mmc_host_clk_hold(host);
		host->cmdq_ops->enable(host);
		mmc_host_clk_release(host);
		mmc_cmdq_halt(host, false);
		goto out;
	}
	/*
	 * CMDQ HW reset would have already made CQE
	 * in unhalted state, but reflect the same
	 * in software state of cmdq_ctx.
	 */
	mmc_host_clr_halt(host);
out:
	return;
}

/**
 * get_cmdq_req_by_tag - returns cmdq_rq based on tag.
 * @q:		request_queue pointer.
 * @tag:	tag number of request to check.
 *
 */
static struct mmc_cmdq_req *get_cmdq_req_by_tag(struct request_queue *q,
						int tag)
{
	struct request *req;
	struct mmc_queue_req *mq_rq;
	struct mmc_cmdq_req *cmdq_req = NULL;

	req = blk_queue_find_tag(q, tag);
	if (WARN_ON(!req))
		goto out;
	mq_rq = req->special;
	if (WARN_ON(!mq_rq))
		goto out;
	cmdq_req = &(mq_rq->cmdq_req);
out:
	return cmdq_req;
}

/**
 * mmc_blk_cmdq_reset_all - Reset everything for CMDQ block request.
 * @host:	mmc_host pointer.
 * @err:	error for which reset is performed.
 *
 * This function implements reset_all functionality for
 * cmdq. It resets the controller, power cycle the card,
 * and invalidate all busy tags(requeue all request back to
 * elevator).
 */
static void mmc_blk_cmdq_reset_all(struct mmc_host *host, int err)
{
	struct mmc_request *mrq = host->err_mrq;
	struct mmc_card *card = host->card;
	struct mmc_cmdq_context_info *ctx_info = &host->cmdq_ctx;
	struct request_queue *q;
	int itag = 0;
	struct mmc_cmdq_req *cmdq_req = NULL;
	struct mmc_request *dcmd_mrq;
	bool is_err_mrq_dcmd = false;

	if (WARN_ON(!mrq))
		return;

	q = mrq->req->q;
	WARN_ON(!test_bit(CMDQ_STATE_ERR, &ctx_info->curr_state));

	#ifdef CONFIG_MMC_CLKGATE
	pr_debug("%s: %s: active_reqs = %lu, clk_requests = %d\n",
			mmc_hostname(host), __func__,
			ctx_info->active_reqs, host->clk_requests);
	#endif

	mmc_blk_cmdq_reset(host, false);

	if (mrq->cmdq_req->cmdq_req_flags & DCMD)
		is_err_mrq_dcmd = true;

	for_each_set_bit(itag, &ctx_info->active_reqs,
			host->num_cq_slots) {
		cmdq_req = get_cmdq_req_by_tag(q, itag);
		if (WARN_ON(!cmdq_req))
			continue;
		if (!(cmdq_req->cmdq_req_flags & DCMD)) {
			WARN_ON(!test_and_clear_bit(itag,
				 &ctx_info->data_active_reqs));
			mmc_cmdq_post_req(host, itag, err);
		} else {
			dcmd_mrq = &cmdq_req->mrq;
			WARN_ON(!test_and_clear_bit(CMDQ_STATE_DCMD_ACTIVE,
					&ctx_info->curr_state));
			pr_debug("%s: cmd(%u), req_op(%u)\n", __func__,
				 dcmd_mrq->cmd->opcode, req_op(dcmd_mrq->req));
			if (!is_err_mrq_dcmd && !dcmd_mrq->cmd->error &&
				(req_op(dcmd_mrq->req) == REQ_OP_SECURE_ERASE ||
				 req_op(dcmd_mrq->req) == REQ_OP_DISCARD)) {
				dcmd_mrq->cmd->error = -EAGAIN;
				complete(&dcmd_mrq->completion);
			}

		}
		WARN_ON(!test_and_clear_bit(itag,
					&ctx_info->active_reqs));
	mmc_host_clk_release(host);
		mmc_put_card(card);
	}

	spin_lock_irq(q->queue_lock);
	blk_queue_invalidate_tags(q);
	spin_unlock_irq(q->queue_lock);
}

static void mmc_blk_cmdq_shutdown(struct mmc_queue *mq)
{
	int err;
	struct mmc_card *card = mq->card;
	struct mmc_host *host = card->host;

	mmc_get_card(card);
	mmc_host_clk_hold(host);
	err = mmc_cmdq_halt(host, true);
	if (err) {
		pr_err("%s: halt: failed: %d\n", __func__, err);
		goto out;
	}

	/* disable CQ mode in card */
	if (mmc_card_cmdq(card)) {
		err = mmc_switch(card, EXT_CSD_CMD_SET_NORMAL,
			 EXT_CSD_CMDQ_MODE_EN, 0,
			 card->ext_csd.generic_cmd6_time);
	if (err) {
		pr_err("%s: failed to switch card to legacy mode: %d\n",
		       __func__, err);
		goto out;
		}
		mmc_card_clr_cmdq(card);
	}
		host->cmdq_ops->disable(host, false);
		host->card->cmdq_init = false;
out:
	mmc_host_clk_release(host);
	mmc_put_card(card);
}

static enum blk_eh_timer_return mmc_blk_cmdq_req_timed_out(struct request *req)
{
	struct mmc_queue *mq = req->q->queuedata;
	struct mmc_host *host = mq->card->host;
	struct mmc_queue_req *mq_rq = req->special;
	struct mmc_request *mrq;
	struct mmc_cmdq_req *cmdq_req;
	struct mmc_cmdq_context_info *ctx_info = &host->cmdq_ctx;

	BUG_ON(!host);

	/*
	 * The mmc_queue_req will be present only if the request
	 * is issued to the LLD. The request could be fetched from
	 * block layer queue but could be waiting to be issued
	 * (for e.g. clock scaling is waiting for an empty cmdq queue)
	 * Reset the timer in such cases to give LLD more time
	 */
	if (!mq_rq) {
		pr_warn("%s: restart timer for tag: %d\n", __func__, req->tag);
		return BLK_EH_RESET_TIMER;
	}

	mrq = &mq_rq->cmdq_req.mrq;
	cmdq_req = &mq_rq->cmdq_req;

	BUG_ON(!mrq || !cmdq_req);

	if (cmdq_req->cmdq_req_flags & DCMD)
		mrq->cmd->error = -ETIMEDOUT;
	else
		mrq->data->error = -ETIMEDOUT;

	host->err_stats[MMC_ERR_CMDQ_REQ_TIMEOUT]++;

	if (mrq->cmd && mrq->cmd->error) {
		if (!(mrq->req->cmd_flags & REQ_PREFLUSH)) {
			/*
			 * Notify completion for non flush commands like
			 * discard that wait for DCMD finish.
			 */
			set_bit(CMDQ_STATE_REQ_TIMED_OUT,
					&ctx_info->curr_state);
			complete(&mrq->completion);
			return BLK_EH_NOT_HANDLED;
		}
	}

	if (test_bit(CMDQ_STATE_REQ_TIMED_OUT, &ctx_info->curr_state) ||
		test_bit(CMDQ_STATE_ERR, &ctx_info->curr_state))
		return BLK_EH_NOT_HANDLED;

	set_bit(CMDQ_STATE_REQ_TIMED_OUT, &ctx_info->curr_state);
	return BLK_EH_HANDLED;
}

/*
 * mmc_blk_cmdq_err: error handling of cmdq error requests.
 * Function should be called in context of error out request
 * which has claim_host and rpm acquired.
 * This may be called with CQ engine halted. Make sure to
 * unhalt it after error recovery.
 *
 * TODO: Currently cmdq error handler does reset_all in case
 * of any erorr. Need to optimize error handling.
 */
static void mmc_blk_cmdq_err(struct mmc_queue *mq)
{
	struct mmc_host *host = mq->card->host;
	struct mmc_request *mrq = host->err_mrq;
	struct mmc_cmdq_context_info *ctx_info = &host->cmdq_ctx;
	struct request_queue *q;
	int err, ret;
	u32 status = 0;

	mmc_host_clk_hold(host);
	host->cmdq_ops->dumpstate(host);
	mmc_host_clk_release(host);

	if (WARN_ON(!mrq))
		return;

	down_write(&ctx_info->err_rwsem);
	pr_err("%s: %s Starting cmdq Error handler\n",
		mmc_hostname(host), __func__);
	q = mrq->req->q;
	err = mmc_cmdq_halt(host, true);
	if (err) {
		pr_err("halt: failed: %d\n", err);
		goto reset;
	}

	/* RED error - Fatal: requires reset */
	if (mrq->cmdq_req->resp_err) {
		err = mrq->cmdq_req->resp_err;
		goto reset;
	}

	/*
	 * TIMEOUT errrors can happen because of execution error
	 * in the last command. So send cmd 13 to get device status
	 */
	if ((mrq->cmd && (mrq->cmd->error == -ETIMEDOUT)) ||
			(mrq->data && (mrq->data->error == -ETIMEDOUT))) {
		if (mmc_host_halt(host) || mmc_host_cq_disable(host)) {
			ret = get_card_status(host->card, &status, 0);
			if (ret)
				pr_err("%s: CMD13 failed with err %d\n",
						mmc_hostname(host), ret);
		}
		pr_err("%s: Timeout error detected with device status 0x%08x\n",
			mmc_hostname(host), status);
	}

	/*
	 * In case of software request time-out, we schedule err work only for
	 * the first error out request and handles all other request in flight
	 * here.
	 */
	if (test_bit(CMDQ_STATE_REQ_TIMED_OUT, &ctx_info->curr_state)) {
		err = -ETIMEDOUT;
	} else if (mrq->data && mrq->data->error) {
		err = mrq->data->error;
	} else if (mrq->cmd && mrq->cmd->error) {
		/* DCMD commands */
		err = mrq->cmd->error;
	}

reset:
	mmc_blk_cmdq_reset_all(host, err);
	if (mrq->cmdq_req->resp_err)
		mrq->cmdq_req->resp_err = false;
	mmc_cmdq_halt(host, false);

	host->err_mrq = NULL;
	clear_bit(CMDQ_STATE_REQ_TIMED_OUT, &ctx_info->curr_state);
	WARN_ON(!test_and_clear_bit(CMDQ_STATE_ERR, &ctx_info->curr_state));

#ifdef CONFIG_MMC_CLKGATE
	pr_err("%s: %s Exiting CMDQ Error handler clk-rqs(%d), claim-cnt(%d), claimed(%d), claimer(%s)\n",
		mmc_hostname(host), __func__, host->clk_requests,
		host->claim_cnt, host->claimed, host->claimer->comm);
#else
	pr_err("%s: %s Exiting CMDQ Error handler, claim-cnt(%d), claimed(%d), claimer(%s)\n",
		mmc_hostname(host), __func__, host->claim_cnt, host->claimed,
		host->claimer->comm);
#endif
	sched_show_task(mq->thread);
	if (host->claimed && host->claimer)
		sched_show_task(host->claimer);
#ifdef CONFIG_MMC_CLKGATE
	WARN_ON(host->clk_requests < 0);
#endif
	WARN_ON(host->claim_cnt < 0);

	up_write(&ctx_info->err_rwsem);
	wake_up(&ctx_info->wait);
	wake_up_interruptible(&host->cmdq_ctx.queue_empty_wq);
}

/* invoked by block layer in softirq context */
void mmc_blk_cmdq_complete_rq(struct request *rq)
{
	struct mmc_queue_req *mq_rq = rq->special;
	struct mmc_request *mrq = &mq_rq->cmdq_req.mrq;
	struct mmc_host *host = mrq->host;
	struct mmc_cmdq_context_info *ctx_info = &host->cmdq_ctx;
	struct mmc_cmdq_req *cmdq_req = &mq_rq->cmdq_req;
	struct mmc_queue *mq = (struct mmc_queue *)rq->q->queuedata;
	int err = 0;
	int err_resp = 0;
	bool is_dcmd = false;
	bool err_rwsem = false;

	if (down_read_trylock(&ctx_info->err_rwsem)) {
		err_rwsem = true;
	} else {
		pr_err("%s: err_rwsem lock failed to acquire => err handler active\n",
			__func__);
		WARN_ON_ONCE(!test_bit(CMDQ_STATE_ERR, &ctx_info->curr_state));
		goto out;
	}

	if (mrq->cmd && mrq->cmd->error)
		err = mrq->cmd->error;
	else if (mrq->data && mrq->data->error)
		err = mrq->data->error;
	if (cmdq_req->resp_err)
		err_resp = cmdq_req->resp_err;

	if ((err || err_resp || work_busy(&mq->cmdq_err_work)) &&
	    !cmdq_req->skip_err_handling) {
		pr_err("%s: %s: txfr error(%d)/resp_err(%d)\n",
				mmc_hostname(mrq->host), __func__, err,
				err_resp);
		if (test_bit(CMDQ_STATE_ERR, &ctx_info->curr_state)) {
			pr_err("%s: CQ in error state, ending current req: %d\n",
				__func__, err);
		} else {
			set_bit(CMDQ_STATE_ERR, &ctx_info->curr_state);
			BUG_ON(host->err_mrq != NULL);
			host->err_mrq = mrq;
			schedule_work(&mq->cmdq_err_work);
		}
		goto out;
	}

	/* clear pending request */
	BUG_ON(!test_and_clear_bit(cmdq_req->tag,
				   &ctx_info->active_reqs));
	if (cmdq_req->cmdq_req_flags & DCMD)
		is_dcmd = true;
	else
		BUG_ON(!test_and_clear_bit(cmdq_req->tag,
					 &ctx_info->data_active_reqs));
	if (!is_dcmd)
		mmc_cmdq_post_req(host, cmdq_req->tag, err);
	if (cmdq_req->cmdq_req_flags & DCMD) {
		clear_bit(CMDQ_STATE_DCMD_ACTIVE, &ctx_info->curr_state);
		blk_end_request_all(rq, err);
		goto out;
	}
	/*
	 * In case of error, cmdq_req->data.bytes_xfered is set to 0.
	 * If we call blk_end_request() with nr_bytes as 0 then the request
	 * never gets completed. So in case of error, to complete a request
	 * with error we should use blk_end_request_all().
	 */
	if (err && cmdq_req->skip_err_handling) {
		cmdq_req->skip_err_handling = false;
		blk_end_request_all(rq, err);
		goto out;
	}

	blk_end_request(rq, err, cmdq_req->data.bytes_xfered);

out:

	mmc_cmdq_clk_scaling_stop_busy(host, true, is_dcmd);
	if (err_rwsem && !(err || err_resp)) {
		mmc_host_clk_release(host);
		wake_up(&ctx_info->wait);
		host->last_completed_rq_time = ktime_get();
		mmc_put_card(host->card);
	}

	if (!ctx_info->active_reqs)
		wake_up_interruptible(&host->cmdq_ctx.queue_empty_wq);

	if (blk_queue_stopped(mq->queue) && !ctx_info->active_reqs)
		complete(&mq->cmdq_shutdown_complete);

	if (err_rwsem)
		up_read(&ctx_info->err_rwsem);
	return;
}

/*
 * Complete reqs from block layer softirq context
 * Invoked in irq context
 */
void mmc_blk_cmdq_req_done(struct mmc_request *mrq)
{
	struct request *req = mrq->req;

	blk_complete_request(req);
}
EXPORT_SYMBOL(mmc_blk_cmdq_req_done);

static void mmc_blk_rw_cmd_abort(struct mmc_queue *mq, struct mmc_card *card,
				 struct request *req,
				 struct mmc_queue_req *mqrq)
{
	if (mmc_card_removed(card))
		req->rq_flags |= RQF_QUIET;
	while (blk_end_request(req, BLK_STS_IOERR, blk_rq_cur_bytes(req)));
	mq->qcnt--;
}

/**
 * mmc_blk_rw_try_restart() - tries to restart the current async request
 * @mq: the queue with the card and host to restart
 * @req: a new request that want to be started after the current one
 */
static void mmc_blk_rw_try_restart(struct mmc_queue *mq, struct request *req,
				   struct mmc_queue_req *mqrq)
{
	if (!req)
		return;

	/*
	 * If the card was removed, just cancel everything and return.
	 */
	if (mmc_card_removed(mq->card)) {
		req->rq_flags |= RQF_QUIET;
		blk_end_request_all(req, BLK_STS_IOERR);
		mq->qcnt--; /* FIXME: just set to 0? */
		return;
	}
	/* Else proceed and try to restart the current async request */
	mmc_blk_rw_rq_prep(mqrq, mq->card, 0, mq);
	mmc_start_areq(mq->card->host, &mqrq->areq, NULL);
}

static void mmc_blk_issue_rw_rq(struct mmc_queue *mq, struct request *new_req)
{
	struct mmc_blk_data *md = mq->blkdata;
	struct mmc_card *card = md->queue.card;
	struct mmc_blk_request *brq;
	int disable_multi = 0, retry = 0, type, retune_retry_done = 0;
	enum mmc_blk_status status;
	struct mmc_queue_req *mqrq_cur = NULL;
	struct mmc_queue_req *mq_rq;
	struct request *old_req;
	struct mmc_async_req *new_areq;
	struct mmc_async_req *old_areq;
	bool req_pending = true;
#ifdef CONFIG_MMC_SIMULATE_MAX_SPEED
	unsigned long waitfor = jiffies;
#endif

	if (new_req) {
		mqrq_cur = req_to_mmc_queue_req(new_req);
		mq->qcnt++;
	}

	if (!mq->qcnt)
		return;

	do {
		if (new_req) {
			/*
			 * When 4KB native sector is enabled, only 8 blocks
			 * multiple read or write is allowed
			 */
			if (mmc_large_sector(card) &&
				!IS_ALIGNED(blk_rq_sectors(new_req), 8)) {
				pr_err("%s: Transfer size is not 4KB sector size aligned\n",
					new_req->rq_disk->disk_name);
				mmc_blk_rw_cmd_abort(mq, card, new_req, mqrq_cur);
				return;
			}

			mmc_blk_rw_rq_prep(mqrq_cur, card, 0, mq);
			new_areq = &mqrq_cur->areq;
		} else
			new_areq = NULL;

		old_areq = mmc_start_areq(card->host, new_areq, &status);
		if (!old_areq) {
			/*
			 * We have just put the first request into the pipeline
			 * and there is nothing more to do until it is
			 * complete.
			 */
			return;
		}

		/*
		 * An asynchronous request has been completed and we proceed
		 * to handle the result of it.
		 */
		mq_rq =	container_of(old_areq, struct mmc_queue_req, areq);
		brq = &mq_rq->brq;
		old_req = mmc_queue_req_to_req(mq_rq);
		type = rq_data_dir(old_req) == READ ? MMC_BLK_READ : MMC_BLK_WRITE;

		switch (status) {
		case MMC_BLK_SUCCESS:
		case MMC_BLK_PARTIAL:
			/*
			 * A block was successfully transferred.
			 */
			mmc_blk_reset_success(md, type);

			mmc_blk_simulate_delay(mq, new_req, waitfor);

			req_pending = blk_end_request(old_req, BLK_STS_OK,
						brq->data.bytes_xfered);
			/*
			 * If the blk_end_request function returns non-zero even
			 * though all data has been transferred and no errors
			 * were returned by the host controller, it's a bug.
			 */
			if (status == MMC_BLK_SUCCESS && req_pending) {
				pr_err("%s BUG rq_tot %d d_xfer %d\n",
				       __func__, blk_rq_bytes(old_req),
				       brq->data.bytes_xfered);
				mmc_blk_rw_cmd_abort(mq, card, old_req, mq_rq);
				return;
			}
			break;
		case MMC_BLK_CMD_ERR:
			req_pending = mmc_blk_rw_cmd_err(md, card, brq, old_req, req_pending);
			if (mmc_blk_reset(md, card->host, type)) {
				if (req_pending)
					mmc_blk_rw_cmd_abort(mq, card, old_req, mq_rq);
				else
					mq->qcnt--;
				mmc_blk_rw_try_restart(mq, new_req, mqrq_cur);
				return;
			}
			if (!req_pending) {
				mq->qcnt--;
				mmc_blk_rw_try_restart(mq, new_req, mqrq_cur);
				return;
			}
			break;
		case MMC_BLK_RETRY:
			retune_retry_done = brq->retune_retry_done;
			if (retry++ < MMC_BLK_MAX_RETRIES)
				break;
			/* Fall through */
		case MMC_BLK_ABORT:
			if (!mmc_blk_reset(md, card->host, type) &&
				(retry++ < (MMC_BLK_MAX_RETRIES + 1)))
				break;
			mmc_blk_rw_cmd_abort(mq, card, old_req, mq_rq);
			mmc_blk_rw_try_restart(mq, new_req, mqrq_cur);
			return;
		case MMC_BLK_DATA_ERR: {
			int err;

			err = mmc_blk_reset(md, card->host, type);
			if (!err)
				break;
			if (err == -ENODEV) {
				mmc_blk_rw_cmd_abort(mq, card, old_req, mq_rq);
				mmc_blk_rw_try_restart(mq, new_req, mqrq_cur);
				return;
			}
			/* Fall through */
		}
		case MMC_BLK_RETRY_SINGLE:
		case MMC_BLK_ECC_ERR:
			if (brq->data.blocks > 1) {
				/* Redo read one sector at a time */
				pr_warn("%s: retrying using single block read\n",
					old_req->rq_disk->disk_name);
				disable_multi = 1;
				break;
			}
			/*
			 * After an error, we redo I/O one sector at a
			 * time, so we only reach here after trying to
			 * read a single sector.
			 */
			req_pending = blk_end_request(old_req, BLK_STS_IOERR,
						      brq->data.blksz);
			if (!req_pending) {
				mq->qcnt--;
				mmc_blk_rw_try_restart(mq, new_req, mqrq_cur);
				return;
			}
			break;
		case MMC_BLK_NOMEDIUM:
			mmc_blk_rw_cmd_abort(mq, card, old_req, mq_rq);
			mmc_blk_rw_try_restart(mq, new_req, mqrq_cur);
			return;
		default:
			pr_err("%s: Unhandled return value (%d)",
					old_req->rq_disk->disk_name, status);
			mmc_blk_rw_cmd_abort(mq, card, old_req, mq_rq);
			mmc_blk_rw_try_restart(mq, new_req, mqrq_cur);
			return;
		}

		if (req_pending) {
			/*
			 * In case of a incomplete request
			 * prepare it again and resend.
			 */
			mmc_blk_rw_rq_prep(mq_rq, card,
					disable_multi, mq);
			mmc_start_areq(card->host,
					&mq_rq->areq, NULL);
			mq_rq->brq.retune_retry_done = retune_retry_done;
		}
	} while (req_pending);

	mq->qcnt--;
}

static inline int mmc_blk_cmdq_part_switch(struct mmc_card *card,
				      struct mmc_blk_data *md)
{
	struct mmc_blk_data *main_md = mmc_get_drvdata(card);
	struct mmc_host *host = card->host;
	struct mmc_cmdq_context_info *ctx = &host->cmdq_ctx;
	u8 part_config = card->ext_csd.part_config;
	int ret = 0, err = 0;

	if ((main_md->part_curr == md->part_type) &&
	    (card->part_curr == md->part_type))
		return 0;

	WARN_ON(!((card->host->caps2 & MMC_CAP2_CMD_QUEUE) &&
		 card->ext_csd.cmdq_support &&
		 (md->flags & MMC_BLK_CMD_QUEUE)));

	if (!test_bit(CMDQ_STATE_HALT, &ctx->curr_state)) {
		ret = mmc_cmdq_halt(host, true);
		if (ret) {
			pr_err("%s: %s: halt: failed: %d\n",
				mmc_hostname(host), __func__,  ret);
			goto out;
		}
	}

	/* disable CQ mode in card */
	if (mmc_card_cmdq(card)) {
		ret = mmc_switch(card, EXT_CSD_CMD_SET_NORMAL,
				 EXT_CSD_CMDQ_MODE_EN, 0,
				 card->ext_csd.generic_cmd6_time);
		if (ret) {
			pr_err("%s: %s: cmdq mode disable failed %d\n",
				mmc_hostname(host), __func__, ret);
			goto cmdq_unhalt;
		}
		mmc_card_clr_cmdq(card);
	}

	part_config &= ~EXT_CSD_PART_CONFIG_ACC_MASK;
	part_config |= md->part_type;

	ret = mmc_switch(card, EXT_CSD_CMD_SET_NORMAL,
			 EXT_CSD_PART_CONFIG, part_config,
			 card->ext_csd.part_time);
	if (ret) {
		pr_err("%s: %s: mmc_switch failure, %d -> %d , err = %d\n",
			mmc_hostname(host), __func__, main_md->part_curr,
			md->part_type, ret);
		goto cmdq_switch;
	}

	card->ext_csd.part_config = part_config;
	card->part_curr = md->part_type;

	main_md->part_curr = md->part_type;

cmdq_switch:
	err = mmc_blk_cmdq_switch(card, md, true);
	if (err) {
		pr_err("%s: %s: mmc_blk_cmdq_switch failed: %d\n",
			mmc_hostname(host), __func__,  err);
		ret = err;
		goto out;
	}
cmdq_unhalt:
	err = mmc_cmdq_halt(host, false);
	if (err) {
		pr_err("%s: %s: unhalt: failed: %d\n",
			mmc_hostname(host), __func__,  err);
		ret = err;
	}
out:
	return ret;
}

static int  mmc_cmdq_wait_for_small_sector_read(struct mmc_card *card,
						struct request *req)
{
	struct mmc_host *host = card->host;
	struct mmc_cmdq_context_info *ctx = &host->cmdq_ctx;
	int ret = 0;

	if ((card->quirks & MMC_QUIRK_CMDQ_EMPTY_BEFORE_DCMD) &&
		ctx->active_small_sector_read_reqs) {
		mmc_cmdq_up_rwsem(host);
		ret = wait_event_interruptible(ctx->queue_empty_wq,
					      !ctx->active_reqs);
		if (ret) {
			pr_err("%s: failed while waiting for the CMDQ to be empty %s err (%d)\n",
				mmc_hostname(host), __func__, ret);
			BUG_ON(1);
		}
		/* clear the counter now */
		ctx->active_small_sector_read_reqs = 0;
		ret = mmc_cmdq_down_rwsem(host, req);
		if (ret)
			return ret;
		/*
		 * If there were small sector (less than 8 sectors) read
		 * operations in progress then we have to wait for the
		 * outstanding requests to finish and should also have
		 * atleast 6 microseconds delay before queuing the DCMD
		 * request.
		 */
		udelay(MMC_QUIRK_CMDQ_DELAY_BEFORE_DCMD);
	}
	return ret;
}

static int mmc_blk_cmdq_issue_drv_op(struct mmc_card *card,
				struct mmc_queue *mq, struct request *req)
{
	int ret;

	ret = mmc_cmdq_halt_on_empty_queue(card->host);
	if (ret) {
		pr_err("%s: failed to halt on empty queue\n",
						mmc_hostname(card->host));
		blk_end_request_all(req, ret);
		mmc_put_card(card);
		return ret;
	}

	mmc_blk_issue_drv_op(mq, req);

	ret = mmc_cmdq_halt(card->host, false);
	if (ret)
		pr_err("%s: %s: failed to unhalt\n",
				mmc_hostname(card->host), __func__);
	mmc_put_card(card);

	return ret;
}

static int mmc_blk_cmdq_issue_rq(struct mmc_queue *mq, struct request *req)
{
	int ret, err = 0;
	struct mmc_blk_data *md = mq->blkdata;
	struct mmc_card *card = md->queue.card;
	struct mmc_host *host = card->host;

	mmc_get_card(card);

	if (!card->host->cmdq_ctx.active_reqs && mmc_card_doing_bkops(card)) {
		ret = mmc_cmdq_halt(card->host, true);
		if (ret)
			goto out;
		ret = mmc_stop_bkops(card);
		if (ret) {
			pr_err("%s: %s: mmc_stop_bkops failed %d\n",
					md->disk->disk_name, __func__, ret);
			goto out;
		}
		ret = mmc_cmdq_halt(card->host, false);
		if (ret)
			goto out;
	}

	ret = mmc_blk_cmdq_part_switch(card, md);
	if (ret) {
		pr_err("%s: %s: partition switch failed %d, resetting cmdq\n",
				md->disk->disk_name, __func__, ret);

		mmc_blk_cmdq_reset(host, false);
		err = mmc_blk_cmdq_part_switch(card, md);
		if (!err) {
			pr_err("%s: %s: partition switch success err = %d\n",
				md->disk->disk_name, __func__, err);
		} else {
			pr_err("%s: %s: partition switch failed err = %d\n",
				md->disk->disk_name, __func__, err);
			ret = err;
			goto out;
		}
	}

	if (req) {
		switch (req_op(req)) {
		case REQ_OP_DISCARD:
			ret = mmc_cmdq_wait_for_small_sector_read(card, req);
			if (!ret)
				ret = mmc_blk_cmdq_issue_discard_rq(mq, req);
			break;
		case REQ_OP_SECURE_ERASE:
			ret = mmc_cmdq_wait_for_small_sector_read(card, req);
			if (ret)
				break;
			if (!(card->quirks & MMC_QUIRK_SEC_ERASE_TRIM_BROKEN))
				ret = mmc_blk_cmdq_issue_secdiscard_rq(mq, req);
			else
				ret = mmc_blk_cmdq_issue_discard_rq(mq, req);
			break;
		case REQ_OP_FLUSH:
			ret = mmc_cmdq_wait_for_small_sector_read(card, req);
			if (!ret)
				ret = mmc_blk_cmdq_issue_flush_rq(mq, req);
			break;
		case REQ_OP_DRV_IN:
		case REQ_OP_DRV_OUT:
			ret = mmc_blk_cmdq_issue_drv_op(card, mq, req);
			break;
		default:
			ret = mmc_blk_cmdq_issue_rw_rq(mq, req);
			/*
			 * If issuing of the request fails with eitehr EBUSY or
			 * EAGAIN error, re-queue the request.
			 * This case would occur with ICE calls.
			 * For request which gets completed successfully or
			 * errored out, we release host lock in completion or
			 * error handling softirq context. But here the request
			 * is neither completed nor erred-out, so release the
			 * host lock explicitly.
			 */
			if (ret == -EBUSY || ret == -EAGAIN) {
				mmc_blk_cmdq_requeue_rw_rq(mq, req);
				mmc_put_card(host->card);
			} else if (ret == -ENOMEM) {
				/*
				 * Elaborate error handling is not needed for
				 * system errors. Let the higher layer decide
				 * on the next steps.
				 */
				goto out;
			}
			break;
		}
	}

	return ret;

out:
	if (req)
		blk_end_request_all(req, ret);
	mmc_put_card(card);

	return ret;
}

void mmc_blk_issue_rq(struct mmc_queue *mq, struct request *req)
{
	int ret;
	struct mmc_blk_data *md = mq->blkdata;
	struct mmc_card *card = md->queue.card;
	int err;

	if (req && !mq->qcnt) {
		/* claim host only for the first request */
		mmc_get_card(card);
		if (mmc_card_doing_bkops(card)) {
			ret = mmc_stop_bkops(card);
			if (ret)
				goto out;
		}
	}

	ret = mmc_blk_part_switch(card, md->part_type);
	if (ret) {
		err = mmc_blk_reset(md, card->host, MMC_BLK_PARTSWITCH);
		if (!err) {
			pr_err("%s: mmc_blk_reset(MMC_BLK_PARTSWITCH) succeeded.\n",
					mmc_hostname(card->host));
			mmc_blk_reset_success(md, MMC_BLK_PARTSWITCH);
		} else
			pr_err("%s: mmc_blk_reset(MMC_BLK_PARTSWITCH) failed.\n",
					mmc_hostname(card->host));

		if (req) {
			blk_end_request_all(req, BLK_STS_IOERR);
		}
		goto out;
	}

	if (req) {
		switch (req_op(req)) {
		case REQ_OP_DRV_IN:
		case REQ_OP_DRV_OUT:
			/*
			 * Complete ongoing async transfer before issuing
			 * ioctl()s
			 */
			if (mq->qcnt)
				mmc_blk_issue_rw_rq(mq, NULL);
			mmc_blk_issue_drv_op(mq, req);
			break;
		case REQ_OP_DISCARD:
			/*
			 * Complete ongoing async transfer before issuing
			 * discard.
			 */
			if (mq->qcnt)
				mmc_blk_issue_rw_rq(mq, NULL);
			mmc_blk_issue_discard_rq(mq, req);
			break;
		case REQ_OP_SECURE_ERASE:
			/*
			 * Complete ongoing async transfer before issuing
			 * secure erase.
			 */
			if (mq->qcnt)
				mmc_blk_issue_rw_rq(mq, NULL);
			if (!(card->quirks & MMC_QUIRK_SEC_ERASE_TRIM_BROKEN))
				mmc_blk_issue_secdiscard_rq(mq, req);
			else
				mmc_blk_issue_discard_rq(mq, req);
			break;
		case REQ_OP_FLUSH:
			/*
			 * Complete ongoing async transfer before issuing
			 * flush.
			 */
			if (mq->qcnt)
				mmc_blk_issue_rw_rq(mq, NULL);
			mmc_blk_issue_flush(mq, req);
			break;
		default:
			/* Normal request, just issue it */
			mmc_blk_issue_rw_rq(mq, req);
			card->host->context_info.is_waiting_last_req = false;
			break;
		}
	} else {
		/* No request, flushing the pipeline with NULL */
		mmc_blk_issue_rw_rq(mq, NULL);
		card->host->context_info.is_waiting_last_req = false;
	}

out:
	if (!mq->qcnt)
		mmc_put_card(card);
}

static inline int mmc_blk_readonly(struct mmc_card *card)
{
	return mmc_card_readonly(card) ||
	       !(card->csd.cmdclass & CCC_BLOCK_WRITE);
}

static struct mmc_blk_data *mmc_blk_alloc_req(struct mmc_card *card,
					      struct device *parent,
					      sector_t size,
					      bool default_ro,
					      const char *subname,
					      int area_type)
{
	struct mmc_blk_data *md;
	int devidx, ret;

	devidx = ida_simple_get(&mmc_blk_ida, 0, max_devices, GFP_KERNEL);
	if (devidx < 0) {
		/*
		 * We get -ENOSPC because there are no more any available
		 * devidx. The reason may be that, either userspace haven't yet
		 * unmounted the partitions, which postpones mmc_blk_release()
		 * from being called, or the device has more partitions than
		 * what we support.
		 */
		if (devidx == -ENOSPC)
			dev_err(mmc_dev(card->host),
				"no more device IDs available\n");

		return ERR_PTR(devidx);
	}

	md = kzalloc(sizeof(struct mmc_blk_data), GFP_KERNEL);
	if (!md) {
		ret = -ENOMEM;
		goto out;
	}

	md->area_type = area_type;

	/*
	 * Set the read-only status based on the supported commands
	 * and the write protect switch.
	 */
	md->read_only = mmc_blk_readonly(card);

	md->disk = alloc_disk(perdev_minors);
	if (md->disk == NULL) {
		ret = -ENOMEM;
		goto err_kfree;
	}

	spin_lock_init(&md->lock);
	INIT_LIST_HEAD(&md->part);
	md->usage = 1;

	ret = mmc_init_queue(&md->queue, card, &md->lock, subname, area_type);
	if (ret)
		goto err_putdisk;

	md->queue.blkdata = md;

	md->disk->major	= MMC_BLOCK_MAJOR;
	md->disk->first_minor = devidx * perdev_minors;
	md->disk->fops = &mmc_bdops;
	md->disk->private_data = md;
	md->disk->queue = md->queue.queue;
	md->parent = parent;
	set_disk_ro(md->disk, md->read_only || default_ro);
	md->disk->flags = GENHD_FL_EXT_DEVT;
	if (area_type & (MMC_BLK_DATA_AREA_RPMB | MMC_BLK_DATA_AREA_BOOT))
		md->disk->flags |= GENHD_FL_NO_PART_SCAN;

	/*
	 * As discussed on lkml, GENHD_FL_REMOVABLE should:
	 *
	 * - be set for removable media with permanent block devices
	 * - be unset for removable block devices with permanent media
	 *
	 * Since MMC block devices clearly fall under the second
	 * case, we do not set GENHD_FL_REMOVABLE.  Userspace
	 * should use the block device creation/destruction hotplug
	 * messages to tell when the card is present.
	 */

	snprintf(md->disk->disk_name, sizeof(md->disk->disk_name),
		 "mmcblk%u%s", card->host->index, subname ? subname : "");

	if (mmc_card_mmc(card))
		blk_queue_logical_block_size(md->queue.queue,
					     card->ext_csd.data_sector_size);
	else
		blk_queue_logical_block_size(md->queue.queue, 512);

	set_capacity(md->disk, size);

	if (mmc_host_cmd23(card->host)) {
		if ((mmc_card_mmc(card) &&
		     card->csd.mmca_vsn >= CSD_SPEC_VER_3) ||
		    (mmc_card_sd(card) &&
		     card->scr.cmds & SD_SCR_CMD23_SUPPORT))
			md->flags |= MMC_BLK_CMD23;
	}

	if (mmc_card_mmc(card) &&
	    md->flags & MMC_BLK_CMD23 &&
	    ((card->ext_csd.rel_param & EXT_CSD_WR_REL_PARAM_EN) ||
	     card->ext_csd.rel_sectors)) {
		md->flags |= MMC_BLK_REL_WR;
		blk_queue_write_cache(md->queue.queue, true, true);
	}

	if (card->cmdq_init) {
		md->flags |= MMC_BLK_CMD_QUEUE;
		md->queue.cmdq_complete_fn = mmc_blk_cmdq_complete_rq;
		md->queue.cmdq_issue_fn = mmc_blk_cmdq_issue_rq;
		md->queue.cmdq_error_fn = mmc_blk_cmdq_err;
		md->queue.cmdq_req_timed_out = mmc_blk_cmdq_req_timed_out;
		md->queue.cmdq_shutdown = mmc_blk_cmdq_shutdown;
	}

	return md;

 err_putdisk:
	put_disk(md->disk);
 err_kfree:
	kfree(md);
 out:
	ida_simple_remove(&mmc_blk_ida, devidx);
	return ERR_PTR(ret);
}

static struct mmc_blk_data *mmc_blk_alloc(struct mmc_card *card)
{
	sector_t size;

	if (!mmc_card_sd(card) && mmc_card_blockaddr(card)) {
		/*
		 * The EXT_CSD sector count is in number or 512 byte
		 * sectors.
		 */
		size = card->ext_csd.sectors;
	} else {
		/*
		 * The CSD capacity field is in units of read_blkbits.
		 * set_capacity takes units of 512 bytes.
		 */
		size = (typeof(sector_t))card->csd.capacity
			<< (card->csd.read_blkbits - 9);
	}

	return mmc_blk_alloc_req(card, &card->dev, size, false, NULL,
					MMC_BLK_DATA_AREA_MAIN);
}

static int mmc_blk_alloc_part(struct mmc_card *card,
			      struct mmc_blk_data *md,
			      unsigned int part_type,
			      sector_t size,
			      bool default_ro,
			      const char *subname,
			      int area_type)
{
	char cap_str[10];
	struct mmc_blk_data *part_md;

	part_md = mmc_blk_alloc_req(card, disk_to_dev(md->disk), size, default_ro,
				    subname, area_type);
	if (IS_ERR(part_md))
		return PTR_ERR(part_md);
	part_md->part_type = part_type;
	list_add(&part_md->part, &md->part);

	string_get_size((u64)get_capacity(part_md->disk), 512, STRING_UNITS_2,
			cap_str, sizeof(cap_str));
	pr_info("%s: %s %s partition %u %s\n",
	       part_md->disk->disk_name, mmc_card_id(card),
	       mmc_card_name(card), part_md->part_type, cap_str);
	return 0;
}

/* MMC Physical partitions consist of two boot partitions and
 * up to four general purpose partitions.
 * For each partition enabled in EXT_CSD a block device will be allocatedi
 * to provide access to the partition.
 */

static int mmc_blk_alloc_parts(struct mmc_card *card, struct mmc_blk_data *md)
{
	int idx, ret = 0;

	if (!mmc_card_mmc(card))
		return 0;

	for (idx = 0; idx < card->nr_parts; idx++) {
		if (card->part[idx].size) {
			ret = mmc_blk_alloc_part(card, md,
				card->part[idx].part_cfg,
				card->part[idx].size >> 9,
				card->part[idx].force_ro,
				card->part[idx].name,
				card->part[idx].area_type);
			if (ret)
				return ret;
		}
	}

	return ret;
}

static void mmc_blk_remove_req(struct mmc_blk_data *md)
{
	struct mmc_card *card;

	if (md) {
		/*
		 * Flush remaining requests and free queues. It
		 * is freeing the queue that stops new requests
		 * from being accepted.
		 */
		card = md->queue.card;
		spin_lock_irq(md->queue.queue->queue_lock);
		queue_flag_set(QUEUE_FLAG_BYPASS, md->queue.queue);
		spin_unlock_irq(md->queue.queue->queue_lock);
		blk_set_queue_dying(md->queue.queue);
		mmc_cleanup_queue(&md->queue);

		if (md->flags & MMC_BLK_CMD_QUEUE)
			mmc_cmdq_clean(&md->queue, card);
		if (md->disk->flags & GENHD_FL_UP) {
			device_remove_file(disk_to_dev(md->disk), &md->force_ro);
			if ((md->area_type & MMC_BLK_DATA_AREA_BOOT) &&
					card->ext_csd.boot_ro_lockable)
				device_remove_file(disk_to_dev(md->disk),
					&md->power_ro_lock);
#ifdef CONFIG_MMC_SIMULATE_MAX_SPEED
			device_remove_file(disk_to_dev(md->disk),
						&dev_attr_max_write_speed);
			device_remove_file(disk_to_dev(md->disk),
						&dev_attr_max_read_speed);
			device_remove_file(disk_to_dev(md->disk),
						&dev_attr_cache_size);
#endif

			del_gendisk(md->disk);
		}
		mmc_blk_put(md);
	}
}

static void mmc_blk_remove_parts(struct mmc_card *card,
				 struct mmc_blk_data *md)
{
	struct list_head *pos, *q;
	struct mmc_blk_data *part_md;

	list_for_each_safe(pos, q, &md->part) {
		part_md = list_entry(pos, struct mmc_blk_data, part);
		list_del(pos);
		mmc_blk_remove_req(part_md);
	}
}

static int mmc_add_disk(struct mmc_blk_data *md)
{
	int ret;
	struct mmc_card *card = md->queue.card;

	device_add_disk(md->parent, md->disk);
	md->force_ro.show = force_ro_show;
	md->force_ro.store = force_ro_store;
	sysfs_attr_init(&md->force_ro.attr);
	md->force_ro.attr.name = "force_ro";
	md->force_ro.attr.mode = S_IRUGO | S_IWUSR;
	ret = device_create_file(disk_to_dev(md->disk), &md->force_ro);
	if (ret)
		goto force_ro_fail;
#ifdef CONFIG_MMC_SIMULATE_MAX_SPEED
	atomic_set(&md->queue.max_write_speed, max_write_speed);
	ret = device_create_file(disk_to_dev(md->disk),
			&dev_attr_max_write_speed);
	if (ret)
		goto max_write_speed_fail;
	atomic_set(&md->queue.max_read_speed, max_read_speed);
	ret = device_create_file(disk_to_dev(md->disk),
			&dev_attr_max_read_speed);
	if (ret)
		goto max_read_speed_fail;
	atomic_set(&md->queue.cache_size, cache_size);
	atomic_long_set(&md->queue.cache_used, 0);
	md->queue.cache_jiffies = jiffies;
	ret = device_create_file(disk_to_dev(md->disk), &dev_attr_cache_size);
	if (ret)
		goto cache_size_fail;
#endif

	if ((md->area_type & MMC_BLK_DATA_AREA_BOOT) &&
	     card->ext_csd.boot_ro_lockable) {
		umode_t mode;

		if (card->ext_csd.boot_ro_lock & EXT_CSD_BOOT_WP_B_PWR_WP_DIS)
			mode = S_IRUGO;
		else
			mode = S_IRUGO | S_IWUSR;

		md->power_ro_lock.show = power_ro_lock_show;
		md->power_ro_lock.store = power_ro_lock_store;
		sysfs_attr_init(&md->power_ro_lock.attr);
		md->power_ro_lock.attr.mode = mode;
		md->power_ro_lock.attr.name =
					"ro_lock_until_next_power_on";
		ret = device_create_file(disk_to_dev(md->disk),
				&md->power_ro_lock);
		if (ret)
			goto power_ro_lock_fail;
	}
	return ret;

power_ro_lock_fail:
#ifdef CONFIG_MMC_SIMULATE_MAX_SPEED
	device_remove_file(disk_to_dev(md->disk), &dev_attr_cache_size);
cache_size_fail:
	device_remove_file(disk_to_dev(md->disk), &dev_attr_max_read_speed);
max_read_speed_fail:
	device_remove_file(disk_to_dev(md->disk), &dev_attr_max_write_speed);
max_write_speed_fail:
#endif
	device_remove_file(disk_to_dev(md->disk), &md->force_ro);
force_ro_fail:
	del_gendisk(md->disk);

	return ret;
}

#ifdef CONFIG_DEBUG_FS

static int mmc_dbg_card_status_get(void *data, u64 *val)
{
	struct mmc_card *card = data;
	struct mmc_blk_data *md = dev_get_drvdata(&card->dev);
	struct mmc_queue *mq = &md->queue;
	struct request *req;
	int ret;

	/* Ask the block layer about the card status */
	req = blk_get_request(mq->queue, REQ_OP_DRV_IN, __GFP_RECLAIM);
	if (IS_ERR(req))
		return PTR_ERR(req);
	req_to_mmc_queue_req(req)->drv_op = MMC_DRV_OP_GET_CARD_STATUS;
	blk_execute_rq(mq->queue, NULL, req, 0);
	ret = req_to_mmc_queue_req(req)->drv_op_result;
	if (ret >= 0) {
		*val = ret;
		ret = 0;
	}
	blk_put_request(req);

	return ret;
}
DEFINE_SIMPLE_ATTRIBUTE(mmc_dbg_card_status_fops, mmc_dbg_card_status_get,
		NULL, "%08llx\n");

/* That is two digits * 512 + 1 for newline */
#define EXT_CSD_STR_LEN 1025

static int mmc_ext_csd_open(struct inode *inode, struct file *filp)
{
	struct mmc_card *card = inode->i_private;
	struct mmc_blk_data *md = dev_get_drvdata(&card->dev);
	struct mmc_queue *mq = &md->queue;
	struct request *req;
	char *buf;
	ssize_t n = 0;
	u8 *ext_csd;
	int err, i;

	buf = kmalloc(EXT_CSD_STR_LEN + 1, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	/* Ask the block layer for the EXT CSD */
	req = blk_get_request(mq->queue, REQ_OP_DRV_IN, __GFP_RECLAIM);
	if (IS_ERR(req)) {
		err = PTR_ERR(req);
		goto out_free;
	}
	req_to_mmc_queue_req(req)->drv_op = MMC_DRV_OP_GET_EXT_CSD;
	req_to_mmc_queue_req(req)->drv_op_data = &ext_csd;
	blk_execute_rq(mq->queue, NULL, req, 0);
	err = req_to_mmc_queue_req(req)->drv_op_result;
	blk_put_request(req);
	if (err) {
		pr_err("FAILED %d\n", err);
		goto out_free;
	}

	for (i = 0; i < 512; i++)
		n += sprintf(buf + n, "%02x", ext_csd[i]);
	n += sprintf(buf + n, "\n");

	if (n != EXT_CSD_STR_LEN) {
		err = -EINVAL;
		kfree(ext_csd);
		goto out_free;
	}

	filp->private_data = buf;

	kfree(ext_csd);
	return 0;

out_free:
	kfree(buf);
	return err;
}

static ssize_t mmc_ext_csd_read(struct file *filp, char __user *ubuf,
				size_t cnt, loff_t *ppos)
{
	char *buf = filp->private_data;

	return simple_read_from_buffer(ubuf, cnt, ppos,
				       buf, EXT_CSD_STR_LEN);
}

static int mmc_ext_csd_release(struct inode *inode, struct file *file)
{
	kfree(file->private_data);
	return 0;
}

static const struct file_operations mmc_dbg_ext_csd_fops = {
	.open		= mmc_ext_csd_open,
	.read		= mmc_ext_csd_read,
	.release	= mmc_ext_csd_release,
	.llseek		= default_llseek,
};

static int mmc_blk_add_debugfs(struct mmc_card *card, struct mmc_blk_data *md)
{
	struct dentry *root;

	if (!card->debugfs_root)
		return 0;

	root = card->debugfs_root;

	if (mmc_card_mmc(card) || mmc_card_sd(card)) {
		md->status_dentry =
			debugfs_create_file("status", S_IRUSR, root, card,
					    &mmc_dbg_card_status_fops);
		if (!md->status_dentry)
			return -EIO;
	}

	if (mmc_card_mmc(card)) {
		md->ext_csd_dentry =
			debugfs_create_file("ext_csd", S_IRUSR, root, card,
					    &mmc_dbg_ext_csd_fops);
		if (!md->ext_csd_dentry)
			return -EIO;
	}

	return 0;
}

static void mmc_blk_remove_debugfs(struct mmc_card *card,
				   struct mmc_blk_data *md)
{
	if (!card->debugfs_root)
		return;

	if (!IS_ERR_OR_NULL(md->status_dentry)) {
		debugfs_remove(md->status_dentry);
		md->status_dentry = NULL;
	}

	if (!IS_ERR_OR_NULL(md->ext_csd_dentry)) {
		debugfs_remove(md->ext_csd_dentry);
		md->ext_csd_dentry = NULL;
	}
}

#else

static int mmc_blk_add_debugfs(struct mmc_card *card, struct mmc_blk_data *md)
{
	return 0;
}

static void mmc_blk_remove_debugfs(struct mmc_card *card,
				   struct mmc_blk_data *md)
{
}

#endif /* CONFIG_DEBUG_FS */

static int mmc_blk_probe(struct mmc_card *card)
{
	struct mmc_blk_data *md, *part_md;
	char cap_str[10];

	/*
	 * Check that the card supports the command class(es) we need.
	 */
	if (!(card->csd.cmdclass & CCC_BLOCK_READ))
		return -ENODEV;

	mmc_fixup_device(card, mmc_blk_fixups);

	md = mmc_blk_alloc(card);
	if (IS_ERR(md))
		return PTR_ERR(md);

	string_get_size((u64)get_capacity(md->disk), 512, STRING_UNITS_2,
			cap_str, sizeof(cap_str));
	pr_info("%s: %s %s %s %s\n",
		md->disk->disk_name, mmc_card_id(card), mmc_card_name(card),
		cap_str, md->read_only ? "(ro)" : "");

	if (mmc_blk_alloc_parts(card, md))
		goto out;

	dev_set_drvdata(&card->dev, md);

#ifdef CONFIG_MMC_BLOCK_DEFERRED_RESUME
	mmc_set_bus_resume_policy(card->host, 1);
#endif

	if (mmc_add_disk(md))
		goto out;

	list_for_each_entry(part_md, &md->part, part) {
		if (mmc_add_disk(part_md))
			goto out;
	}

	/* Add two debugfs entries */
	mmc_blk_add_debugfs(card, md);

	if (mmc_card_sd(card))
		pm_runtime_set_autosuspend_delay(&card->dev,
			MMC_SDCARD_AUTOSUSPEND_DELAY_MS);
	else
		pm_runtime_set_autosuspend_delay(&card->dev,
			MMC_AUTOSUSPEND_DELAY_MS);

	pm_runtime_use_autosuspend(&card->dev);

	/*
	 * Don't enable runtime PM for SD-combo cards here. Leave that
	 * decision to be taken during the SDIO init sequence instead.
	 */
	if (card->type != MMC_TYPE_SD_COMBO) {
		pm_runtime_set_active(&card->dev);
		pm_runtime_enable(&card->dev);
	}

	return 0;

 out:
	mmc_blk_remove_parts(card, md);
	mmc_blk_remove_req(md);
	return 0;
}

static void mmc_blk_remove(struct mmc_card *card)
{
	struct mmc_blk_data *md = dev_get_drvdata(&card->dev);

	mmc_blk_remove_debugfs(card, md);
	mmc_blk_remove_parts(card, md);
	pm_runtime_get_sync(&card->dev);
	mmc_claim_host(card->host);
	mmc_blk_part_switch(card, md->part_type);
	mmc_release_host(card->host);
	if (card->type != MMC_TYPE_SD_COMBO)
		pm_runtime_disable(&card->dev);
	pm_runtime_put_noidle(&card->dev);
	mmc_blk_remove_req(md);
	dev_set_drvdata(&card->dev, NULL);
#ifdef CONFIG_MMC_BLOCK_DEFERRED_RESUME
	mmc_set_bus_resume_policy(card->host, 0);
#endif
}

static int _mmc_blk_suspend(struct mmc_card *card, bool wait)
{
	struct mmc_blk_data *part_md;
	struct mmc_blk_data *md = dev_get_drvdata(&card->dev);
	int rc = 0;

	if (md) {
		rc = mmc_queue_suspend(&md->queue, wait);
		if (rc)
			goto out;
		list_for_each_entry(part_md, &md->part, part) {
			rc = mmc_queue_suspend(&part_md->queue, wait);
			if (rc)
				goto out_resume;
		}
	}
	goto out;

out_resume:
	mmc_queue_resume(&md->queue);
	list_for_each_entry(part_md, &md->part, part) {
		mmc_queue_resume(&part_md->queue);
	}
out:
	return rc;
}

static void mmc_blk_shutdown(struct mmc_card *card)
{
	int rc = 0;

	_mmc_blk_suspend(card, 1);

	mmc_claim_host(card->host);
	/* send cache off control */
	rc = mmc_cache_ctrl(card->host, 0);
	mmc_release_host(card->host);
	if (rc)
		pr_err("%s: mmc_flush_cache returned error = %d",
			mmc_hostname(card->host), rc);
}

#ifdef CONFIG_PM_SLEEP
static int mmc_blk_suspend(struct device *dev)
{
	struct mmc_card *card = mmc_dev_to_card(dev);

	return _mmc_blk_suspend(card, 0);
}

static int mmc_blk_resume(struct device *dev)
{
	struct mmc_blk_data *part_md;
	struct mmc_blk_data *md = dev_get_drvdata(dev);

	if (md) {
		/*
		 * Resume involves the card going into idle state,
		 * so current partition is always the main one.
		 */
		md->part_curr = md->part_type;
		mmc_queue_resume(&md->queue);
		list_for_each_entry(part_md, &md->part, part) {
			mmc_queue_resume(&part_md->queue);
		}
	}
	return 0;
}
#endif

static SIMPLE_DEV_PM_OPS(mmc_blk_pm_ops, mmc_blk_suspend, mmc_blk_resume);

static struct mmc_driver mmc_driver = {
	.drv		= {
		.name	= "mmcblk",
		.pm	= &mmc_blk_pm_ops,
	},
	.probe		= mmc_blk_probe,
	.remove		= mmc_blk_remove,
	.shutdown	= mmc_blk_shutdown,
};

static int __init mmc_blk_init(void)
{
	int res;

	if (perdev_minors != CONFIG_MMC_BLOCK_MINORS)
		pr_info("mmcblk: using %d minors per device\n", perdev_minors);

	max_devices = min(MAX_DEVICES, (1 << MINORBITS) / perdev_minors);

	res = register_blkdev(MMC_BLOCK_MAJOR, "mmc");
	if (res)
		goto out;

	res = mmc_register_driver(&mmc_driver);
	if (res)
		goto out2;

	return 0;
 out2:
	unregister_blkdev(MMC_BLOCK_MAJOR, "mmc");
 out:
	return res;
}

static void __exit mmc_blk_exit(void)
{
	mmc_unregister_driver(&mmc_driver);
	unregister_blkdev(MMC_BLOCK_MAJOR, "mmc");
}

module_init(mmc_blk_init);
module_exit(mmc_blk_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Multimedia Card (MMC) block device driver");

