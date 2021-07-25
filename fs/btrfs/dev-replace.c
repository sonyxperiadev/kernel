// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) STRATO AG 2012.  All rights reserved.
 */

#include <linux/sched.h>
#include <linux/bio.h>
#include <linux/slab.h>
#include <linux/blkdev.h>
#include <linux/kthread.h>
#include <linux/math64.h>
#include "ctree.h"
#include "extent_map.h"
#include "disk-io.h"
#include "transaction.h"
#include "print-tree.h"
#include "volumes.h"
#include "async-thread.h"
#include "check-integrity.h"
#include "rcu-string.h"
#include "dev-replace.h"
#include "sysfs.h"

static int btrfs_dev_replace_finishing(struct btrfs_fs_info *fs_info,
				       int scrub_ret);
static void btrfs_dev_replace_update_device_in_mapping_tree(
						struct btrfs_fs_info *fs_info,
						struct btrfs_device *srcdev,
						struct btrfs_device *tgtdev);
static int btrfs_dev_replace_kthread(void *data);

int btrfs_init_dev_replace(struct btrfs_fs_info *fs_info)
{
	struct btrfs_key key;
	struct btrfs_root *dev_root = fs_info->dev_root;
	struct btrfs_dev_replace *dev_replace = &fs_info->dev_replace;
	struct extent_buffer *eb;
	int slot;
	int ret = 0;
	struct btrfs_path *path = NULL;
	int item_size;
	struct btrfs_dev_replace_item *ptr;
	u64 src_devid;

	path = btrfs_alloc_path();
	if (!path) {
		ret = -ENOMEM;
		goto out;
	}

	key.objectid = 0;
	key.type = BTRFS_DEV_REPLACE_KEY;
	key.offset = 0;
	ret = btrfs_search_slot(NULL, dev_root, &key, path, 0, 0);
	if (ret) {
no_valid_dev_replace_entry_found:
		ret = 0;
		dev_replace->replace_state =
			BTRFS_DEV_REPLACE_ITEM_STATE_NEVER_STARTED;
		dev_replace->cont_reading_from_srcdev_mode =
		    BTRFS_DEV_REPLACE_ITEM_CONT_READING_FROM_SRCDEV_MODE_ALWAYS;
		dev_replace->replace_state = 0;
		dev_replace->time_started = 0;
		dev_replace->time_stopped = 0;
		atomic64_set(&dev_replace->num_write_errors, 0);
		atomic64_set(&dev_replace->num_uncorrectable_read_errors, 0);
		dev_replace->cursor_left = 0;
		dev_replace->committed_cursor_left = 0;
		dev_replace->cursor_left_last_write_of_item = 0;
		dev_replace->cursor_right = 0;
		dev_replace->srcdev = NULL;
		dev_replace->tgtdev = NULL;
		dev_replace->is_valid = 0;
		dev_replace->item_needs_writeback = 0;
		goto out;
	}
	slot = path->slots[0];
	eb = path->nodes[0];
	item_size = btrfs_item_size_nr(eb, slot);
	ptr = btrfs_item_ptr(eb, slot, struct btrfs_dev_replace_item);

	if (item_size != sizeof(struct btrfs_dev_replace_item)) {
		btrfs_warn(fs_info,
			"dev_replace entry found has unexpected size, ignore entry");
		goto no_valid_dev_replace_entry_found;
	}

	src_devid = btrfs_dev_replace_src_devid(eb, ptr);
	dev_replace->cont_reading_from_srcdev_mode =
		btrfs_dev_replace_cont_reading_from_srcdev_mode(eb, ptr);
	dev_replace->replace_state = btrfs_dev_replace_replace_state(eb, ptr);
	dev_replace->time_started = btrfs_dev_replace_time_started(eb, ptr);
	dev_replace->time_stopped =
		btrfs_dev_replace_time_stopped(eb, ptr);
	atomic64_set(&dev_replace->num_write_errors,
		     btrfs_dev_replace_num_write_errors(eb, ptr));
	atomic64_set(&dev_replace->num_uncorrectable_read_errors,
		     btrfs_dev_replace_num_uncorrectable_read_errors(eb, ptr));
	dev_replace->cursor_left = btrfs_dev_replace_cursor_left(eb, ptr);
	dev_replace->committed_cursor_left = dev_replace->cursor_left;
	dev_replace->cursor_left_last_write_of_item = dev_replace->cursor_left;
	dev_replace->cursor_right = btrfs_dev_replace_cursor_right(eb, ptr);
	dev_replace->is_valid = 1;

	dev_replace->item_needs_writeback = 0;
	switch (dev_replace->replace_state) {
	case BTRFS_IOCTL_DEV_REPLACE_STATE_NEVER_STARTED:
	case BTRFS_IOCTL_DEV_REPLACE_STATE_FINISHED:
	case BTRFS_IOCTL_DEV_REPLACE_STATE_CANCELED:
		dev_replace->srcdev = NULL;
		dev_replace->tgtdev = NULL;
		break;
	case BTRFS_IOCTL_DEV_REPLACE_STATE_STARTED:
	case BTRFS_IOCTL_DEV_REPLACE_STATE_SUSPENDED:
		dev_replace->srcdev = btrfs_find_device(fs_info->fs_devices,
						src_devid, NULL, NULL, true);
		dev_replace->tgtdev = btrfs_find_device(fs_info->fs_devices,
							BTRFS_DEV_REPLACE_DEVID,
							NULL, NULL, true);
		/*
		 * allow 'btrfs dev replace_cancel' if src/tgt device is
		 * missing
		 */
		if (!dev_replace->srcdev &&
		    !btrfs_test_opt(fs_info, DEGRADED)) {
			ret = -EIO;
			btrfs_warn(fs_info,
			   "cannot mount because device replace operation is ongoing and");
			btrfs_warn(fs_info,
			   "srcdev (devid %llu) is missing, need to run 'btrfs dev scan'?",
			   src_devid);
		}
		if (!dev_replace->tgtdev &&
		    !btrfs_test_opt(fs_info, DEGRADED)) {
			ret = -EIO;
			btrfs_warn(fs_info,
			   "cannot mount because device replace operation is ongoing and");
			btrfs_warn(fs_info,
			   "tgtdev (devid %llu) is missing, need to run 'btrfs dev scan'?",
				BTRFS_DEV_REPLACE_DEVID);
		}
		if (dev_replace->tgtdev) {
			if (dev_replace->srcdev) {
				dev_replace->tgtdev->total_bytes =
					dev_replace->srcdev->total_bytes;
				dev_replace->tgtdev->disk_total_bytes =
					dev_replace->srcdev->disk_total_bytes;
				dev_replace->tgtdev->commit_total_bytes =
					dev_replace->srcdev->commit_total_bytes;
				dev_replace->tgtdev->bytes_used =
					dev_replace->srcdev->bytes_used;
				dev_replace->tgtdev->commit_bytes_used =
					dev_replace->srcdev->commit_bytes_used;
			}
			set_bit(BTRFS_DEV_STATE_REPLACE_TGT,
				&dev_replace->tgtdev->dev_state);

			WARN_ON(fs_info->fs_devices->rw_devices == 0);
			dev_replace->tgtdev->io_width = fs_info->sectorsize;
			dev_replace->tgtdev->io_align = fs_info->sectorsize;
			dev_replace->tgtdev->sector_size = fs_info->sectorsize;
			dev_replace->tgtdev->fs_info = fs_info;
			set_bit(BTRFS_DEV_STATE_IN_FS_METADATA,
				&dev_replace->tgtdev->dev_state);
		}
		break;
	}

out:
	btrfs_free_path(path);
	return ret;
}

/*
 * Initialize a new device for device replace target from a given source dev
 * and path.
 *
 * Return 0 and new device in @device_out, otherwise return < 0
 */
static int btrfs_init_dev_replace_tgtdev(struct btrfs_fs_info *fs_info,
				  const char *device_path,
				  struct btrfs_device *srcdev,
				  struct btrfs_device **device_out)
{
	struct btrfs_device *device;
	struct block_device *bdev;
	struct list_head *devices;
	struct rcu_string *name;
	u64 devid = BTRFS_DEV_REPLACE_DEVID;
	int ret = 0;

	*device_out = NULL;
	if (srcdev->fs_devices->seeding) {
		btrfs_err(fs_info, "the filesystem is a seed filesystem!");
		return -EINVAL;
	}

	bdev = blkdev_get_by_path(device_path, FMODE_WRITE | FMODE_EXCL,
				  fs_info->bdev_holder);
	if (IS_ERR(bdev)) {
		btrfs_err(fs_info, "target device %s is invalid!", device_path);
		return PTR_ERR(bdev);
	}

	filemap_write_and_wait(bdev->bd_inode->i_mapping);

	devices = &fs_info->fs_devices->devices;
	list_for_each_entry(device, devices, dev_list) {
		if (device->bdev == bdev) {
			btrfs_err(fs_info,
				  "target device is in the filesystem!");
			ret = -EEXIST;
			goto error;
		}
	}


	if (i_size_read(bdev->bd_inode) <
	    btrfs_device_get_total_bytes(srcdev)) {
		btrfs_err(fs_info,
			  "target device is smaller than source device!");
		ret = -EINVAL;
		goto error;
	}


	device = btrfs_alloc_device(NULL, &devid, NULL);
	if (IS_ERR(device)) {
		ret = PTR_ERR(device);
		goto error;
	}

	name = rcu_string_strdup(device_path, GFP_KERNEL);
	if (!name) {
		btrfs_free_device(device);
		ret = -ENOMEM;
		goto error;
	}
	rcu_assign_pointer(device->name, name);

	mutex_lock(&fs_info->fs_devices->device_list_mutex);
	set_bit(BTRFS_DEV_STATE_WRITEABLE, &device->dev_state);
	device->generation = 0;
	device->io_width = fs_info->sectorsize;
	device->io_align = fs_info->sectorsize;
	device->sector_size = fs_info->sectorsize;
	device->total_bytes = btrfs_device_get_total_bytes(srcdev);
	device->disk_total_bytes = btrfs_device_get_disk_total_bytes(srcdev);
	device->bytes_used = btrfs_device_get_bytes_used(srcdev);
	device->commit_total_bytes = srcdev->commit_total_bytes;
	device->commit_bytes_used = device->bytes_used;
	device->fs_info = fs_info;
	device->bdev = bdev;
	set_bit(BTRFS_DEV_STATE_IN_FS_METADATA, &device->dev_state);
	set_bit(BTRFS_DEV_STATE_REPLACE_TGT, &device->dev_state);
	device->mode = FMODE_EXCL;
	device->dev_stats_valid = 1;
	set_blocksize(device->bdev, BTRFS_BDEV_BLOCKSIZE);
	device->fs_devices = fs_info->fs_devices;
	list_add(&device->dev_list, &fs_info->fs_devices->devices);
	fs_info->fs_devices->num_devices++;
	fs_info->fs_devices->open_devices++;
	mutex_unlock(&fs_info->fs_devices->device_list_mutex);

	*device_out = device;
	return 0;

error:
	blkdev_put(bdev, FMODE_EXCL);
	return ret;
}

/*
 * called from commit_transaction. Writes changed device replace state to
 * disk.
 */
int btrfs_run_dev_replace(struct btrfs_trans_handle *trans,
			  struct btrfs_fs_info *fs_info)
{
	int ret;
	struct btrfs_root *dev_root = fs_info->dev_root;
	struct btrfs_path *path;
	struct btrfs_key key;
	struct extent_buffer *eb;
	struct btrfs_dev_replace_item *ptr;
	struct btrfs_dev_replace *dev_replace = &fs_info->dev_replace;

	btrfs_dev_replace_read_lock(dev_replace);
	if (!dev_replace->is_valid ||
	    !dev_replace->item_needs_writeback) {
		btrfs_dev_replace_read_unlock(dev_replace);
		return 0;
	}
	btrfs_dev_replace_read_unlock(dev_replace);

	key.objectid = 0;
	key.type = BTRFS_DEV_REPLACE_KEY;
	key.offset = 0;

	path = btrfs_alloc_path();
	if (!path) {
		ret = -ENOMEM;
		goto out;
	}
	ret = btrfs_search_slot(trans, dev_root, &key, path, -1, 1);
	if (ret < 0) {
		btrfs_warn(fs_info,
			   "error %d while searching for dev_replace item!",
			   ret);
		goto out;
	}

	if (ret == 0 &&
	    btrfs_item_size_nr(path->nodes[0], path->slots[0]) < sizeof(*ptr)) {
		/*
		 * need to delete old one and insert a new one.
		 * Since no attempt is made to recover any old state, if the
		 * dev_replace state is 'running', the data on the target
		 * drive is lost.
		 * It would be possible to recover the state: just make sure
		 * that the beginning of the item is never changed and always
		 * contains all the essential information. Then read this
		 * minimal set of information and use it as a base for the
		 * new state.
		 */
		ret = btrfs_del_item(trans, dev_root, path);
		if (ret != 0) {
			btrfs_warn(fs_info,
				   "delete too small dev_replace item failed %d!",
				   ret);
			goto out;
		}
		ret = 1;
	}

	if (ret == 1) {
		/* need to insert a new item */
		btrfs_release_path(path);
		ret = btrfs_insert_empty_item(trans, dev_root, path,
					      &key, sizeof(*ptr));
		if (ret < 0) {
			btrfs_warn(fs_info,
				   "insert dev_replace item failed %d!", ret);
			goto out;
		}
	}

	eb = path->nodes[0];
	ptr = btrfs_item_ptr(eb, path->slots[0],
			     struct btrfs_dev_replace_item);

	btrfs_dev_replace_write_lock(dev_replace);
	if (dev_replace->srcdev)
		btrfs_set_dev_replace_src_devid(eb, ptr,
			dev_replace->srcdev->devid);
	else
		btrfs_set_dev_replace_src_devid(eb, ptr, (u64)-1);
	btrfs_set_dev_replace_cont_reading_from_srcdev_mode(eb, ptr,
		dev_replace->cont_reading_from_srcdev_mode);
	btrfs_set_dev_replace_replace_state(eb, ptr,
		dev_replace->replace_state);
	btrfs_set_dev_replace_time_started(eb, ptr, dev_replace->time_started);
	btrfs_set_dev_replace_time_stopped(eb, ptr, dev_replace->time_stopped);
	btrfs_set_dev_replace_num_write_errors(eb, ptr,
		atomic64_read(&dev_replace->num_write_errors));
	btrfs_set_dev_replace_num_uncorrectable_read_errors(eb, ptr,
		atomic64_read(&dev_replace->num_uncorrectable_read_errors));
	dev_replace->cursor_left_last_write_of_item =
		dev_replace->cursor_left;
	btrfs_set_dev_replace_cursor_left(eb, ptr,
		dev_replace->cursor_left_last_write_of_item);
	btrfs_set_dev_replace_cursor_right(eb, ptr,
		dev_replace->cursor_right);
	dev_replace->item_needs_writeback = 0;
	btrfs_dev_replace_write_unlock(dev_replace);

	btrfs_mark_buffer_dirty(eb);

out:
	btrfs_free_path(path);

	return ret;
}

void btrfs_after_dev_replace_commit(struct btrfs_fs_info *fs_info)
{
	struct btrfs_dev_replace *dev_replace = &fs_info->dev_replace;

	dev_replace->committed_cursor_left =
		dev_replace->cursor_left_last_write_of_item;
}

static char* btrfs_dev_name(struct btrfs_device *device)
{
	if (!device || test_bit(BTRFS_DEV_STATE_MISSING, &device->dev_state))
		return "<missing disk>";
	else
		return rcu_str_deref(device->name);
}

int btrfs_dev_replace_start(struct btrfs_fs_info *fs_info,
		const char *tgtdev_name, u64 srcdevid, const char *srcdev_name,
		int read_src)
{
	struct btrfs_root *root = fs_info->dev_root;
	struct btrfs_trans_handle *trans;
	struct btrfs_dev_replace *dev_replace = &fs_info->dev_replace;
	int ret;
	struct btrfs_device *tgt_device = NULL;
	struct btrfs_device *src_device = NULL;

	ret = btrfs_find_device_by_devspec(fs_info, srcdevid,
					    srcdev_name, &src_device);
	if (ret)
		return ret;

	ret = btrfs_init_dev_replace_tgtdev(fs_info, tgtdev_name,
					    src_device, &tgt_device);
	if (ret)
		return ret;

	/*
	 * Here we commit the transaction to make sure commit_total_bytes
	 * of all the devices are updated.
	 */
	trans = btrfs_attach_transaction(root);
	if (!IS_ERR(trans)) {
		ret = btrfs_commit_transaction(trans);
		if (ret)
			return ret;
	} else if (PTR_ERR(trans) != -ENOENT) {
		return PTR_ERR(trans);
	}

	btrfs_dev_replace_write_lock(dev_replace);
	switch (dev_replace->replace_state) {
	case BTRFS_IOCTL_DEV_REPLACE_STATE_NEVER_STARTED:
	case BTRFS_IOCTL_DEV_REPLACE_STATE_FINISHED:
	case BTRFS_IOCTL_DEV_REPLACE_STATE_CANCELED:
		break;
	case BTRFS_IOCTL_DEV_REPLACE_STATE_STARTED:
	case BTRFS_IOCTL_DEV_REPLACE_STATE_SUSPENDED:
		ASSERT(0);
		ret = BTRFS_IOCTL_DEV_REPLACE_RESULT_ALREADY_STARTED;
		goto leave;
	}

	dev_replace->cont_reading_from_srcdev_mode = read_src;
	WARN_ON(!src_device);
	dev_replace->srcdev = src_device;
	dev_replace->tgtdev = tgt_device;

	btrfs_info_in_rcu(fs_info,
		      "dev_replace from %s (devid %llu) to %s started",
		      btrfs_dev_name(src_device),
		      src_device->devid,
		      rcu_str_deref(tgt_device->name));

	/*
	 * from now on, the writes to the srcdev are all duplicated to
	 * go to the tgtdev as well (refer to btrfs_map_block()).
	 */
	dev_replace->replace_state = BTRFS_IOCTL_DEV_REPLACE_STATE_STARTED;
	dev_replace->time_started = ktime_get_real_seconds();
	dev_replace->cursor_left = 0;
	dev_replace->committed_cursor_left = 0;
	dev_replace->cursor_left_last_write_of_item = 0;
	dev_replace->cursor_right = 0;
	dev_replace->is_valid = 1;
	dev_replace->item_needs_writeback = 1;
	atomic64_set(&dev_replace->num_write_errors, 0);
	atomic64_set(&dev_replace->num_uncorrectable_read_errors, 0);
	btrfs_dev_replace_write_unlock(dev_replace);

	ret = btrfs_sysfs_add_device_link(tgt_device->fs_devices, tgt_device);
	if (ret)
		btrfs_err(fs_info, "kobj add dev failed %d", ret);

	btrfs_wait_ordered_roots(fs_info, U64_MAX, 0, (u64)-1);

	/* force writing the updated state information to disk */
	trans = btrfs_start_transaction(root, 0);
	if (IS_ERR(trans)) {
		ret = PTR_ERR(trans);
		btrfs_dev_replace_write_lock(dev_replace);
		dev_replace->replace_state =
			BTRFS_IOCTL_DEV_REPLACE_STATE_NEVER_STARTED;
		dev_replace->srcdev = NULL;
		dev_replace->tgtdev = NULL;
		goto leave;
	}

	ret = btrfs_commit_transaction(trans);
	WARN_ON(ret);

	/* the disk copy procedure reuses the scrub code */
	ret = btrfs_scrub_dev(fs_info, src_device->devid, 0,
			      btrfs_device_get_total_bytes(src_device),
			      &dev_replace->scrub_progress, 0, 1);

	ret = btrfs_dev_replace_finishing(fs_info, ret);
	if (ret == -EINPROGRESS) {
		ret = BTRFS_IOCTL_DEV_REPLACE_RESULT_SCRUB_INPROGRESS;
	} else {
		WARN_ON(ret);
	}

	return ret;

leave:
	btrfs_dev_replace_write_unlock(dev_replace);
	btrfs_destroy_dev_replace_tgtdev(tgt_device);
	return ret;
}

int btrfs_dev_replace_by_ioctl(struct btrfs_fs_info *fs_info,
			    struct btrfs_ioctl_dev_replace_args *args)
{
	int ret;

	switch (args->start.cont_reading_from_srcdev_mode) {
	case BTRFS_IOCTL_DEV_REPLACE_CONT_READING_FROM_SRCDEV_MODE_ALWAYS:
	case BTRFS_IOCTL_DEV_REPLACE_CONT_READING_FROM_SRCDEV_MODE_AVOID:
		break;
	default:
		return -EINVAL;
	}

	if ((args->start.srcdevid == 0 && args->start.srcdev_name[0] == '\0') ||
	    args->start.tgtdev_name[0] == '\0')
		return -EINVAL;

	ret = btrfs_dev_replace_start(fs_info, args->start.tgtdev_name,
					args->start.srcdevid,
					args->start.srcdev_name,
					args->start.cont_reading_from_srcdev_mode);
	args->result = ret;
	/* don't warn if EINPROGRESS, someone else might be running scrub */
	if (ret == BTRFS_IOCTL_DEV_REPLACE_RESULT_SCRUB_INPROGRESS)
		ret = 0;

	return ret;
}

/*
 * blocked until all in-flight bios operations are finished.
 */
static void btrfs_rm_dev_replace_blocked(struct btrfs_fs_info *fs_info)
{
	set_bit(BTRFS_FS_STATE_DEV_REPLACING, &fs_info->fs_state);
	wait_event(fs_info->replace_wait, !percpu_counter_sum(
		   &fs_info->bio_counter));
}

/*
 * we have removed target device, it is safe to allow new bios request.
 */
static void btrfs_rm_dev_replace_unblocked(struct btrfs_fs_info *fs_info)
{
	clear_bit(BTRFS_FS_STATE_DEV_REPLACING, &fs_info->fs_state);
	wake_up(&fs_info->replace_wait);
}

static int btrfs_dev_replace_finishing(struct btrfs_fs_info *fs_info,
				       int scrub_ret)
{
	struct btrfs_dev_replace *dev_replace = &fs_info->dev_replace;
	struct btrfs_device *tgt_device;
	struct btrfs_device *src_device;
	struct btrfs_root *root = fs_info->tree_root;
	u8 uuid_tmp[BTRFS_UUID_SIZE];
	struct btrfs_trans_handle *trans;
	int ret = 0;

	/* don't allow cancel or unmount to disturb the finishing procedure */
	mutex_lock(&dev_replace->lock_finishing_cancel_unmount);

	btrfs_dev_replace_read_lock(dev_replace);
	/* was the operation canceled, or is it finished? */
	if (dev_replace->replace_state !=
	    BTRFS_IOCTL_DEV_REPLACE_STATE_STARTED) {
		btrfs_dev_replace_read_unlock(dev_replace);
		mutex_unlock(&dev_replace->lock_finishing_cancel_unmount);
		return 0;
	}

	tgt_device = dev_replace->tgtdev;
	src_device = dev_replace->srcdev;
	btrfs_dev_replace_read_unlock(dev_replace);

	/*
	 * flush all outstanding I/O and inode extent mappings before the
	 * copy operation is declared as being finished
	 */
	ret = btrfs_start_delalloc_roots(fs_info, -1);
	if (ret) {
		mutex_unlock(&dev_replace->lock_finishing_cancel_unmount);
		return ret;
	}
	btrfs_wait_ordered_roots(fs_info, U64_MAX, 0, (u64)-1);

	while (1) {
		trans = btrfs_start_transaction(root, 0);
		if (IS_ERR(trans)) {
			mutex_unlock(&dev_replace->lock_finishing_cancel_unmount);
			return PTR_ERR(trans);
		}
		ret = btrfs_commit_transaction(trans);
		WARN_ON(ret);
		/* keep away write_all_supers() during the finishing procedure */
		mutex_lock(&fs_info->fs_devices->device_list_mutex);
		mutex_lock(&fs_info->chunk_mutex);
		if (src_device->has_pending_chunks) {
			mutex_unlock(&root->fs_info->chunk_mutex);
			mutex_unlock(&root->fs_info->fs_devices->device_list_mutex);
		} else {
			break;
		}
	}

	btrfs_dev_replace_write_lock(dev_replace);
	dev_replace->replace_state =
		scrub_ret ? BTRFS_IOCTL_DEV_REPLACE_STATE_CANCELED
			  : BTRFS_IOCTL_DEV_REPLACE_STATE_FINISHED;
	dev_replace->tgtdev = NULL;
	dev_replace->srcdev = NULL;
	dev_replace->time_stopped = ktime_get_real_seconds();
	dev_replace->item_needs_writeback = 1;

	/* replace old device with new one in mapping tree */
	if (!scrub_ret) {
		btrfs_dev_replace_update_device_in_mapping_tree(fs_info,
								src_device,
								tgt_device);
	} else {
		btrfs_err_in_rcu(fs_info,
				 "btrfs_scrub_dev(%s, %llu, %s) failed %d",
				 btrfs_dev_name(src_device),
				 src_device->devid,
				 rcu_str_deref(tgt_device->name), scrub_ret);
		btrfs_dev_replace_write_unlock(dev_replace);
		mutex_unlock(&fs_info->chunk_mutex);
		mutex_unlock(&fs_info->fs_devices->device_list_mutex);
		btrfs_rm_dev_replace_blocked(fs_info);
		if (tgt_device)
			btrfs_destroy_dev_replace_tgtdev(tgt_device);
		btrfs_rm_dev_replace_unblocked(fs_info);
		mutex_unlock(&dev_replace->lock_finishing_cancel_unmount);

		return scrub_ret;
	}

	btrfs_info_in_rcu(fs_info,
			  "dev_replace from %s (devid %llu) to %s finished",
			  btrfs_dev_name(src_device),
			  src_device->devid,
			  rcu_str_deref(tgt_device->name));
	clear_bit(BTRFS_DEV_STATE_REPLACE_TGT, &tgt_device->dev_state);
	tgt_device->devid = src_device->devid;
	src_device->devid = BTRFS_DEV_REPLACE_DEVID;
	memcpy(uuid_tmp, tgt_device->uuid, sizeof(uuid_tmp));
	memcpy(tgt_device->uuid, src_device->uuid, sizeof(tgt_device->uuid));
	memcpy(src_device->uuid, uuid_tmp, sizeof(src_device->uuid));
	btrfs_device_set_total_bytes(tgt_device, src_device->total_bytes);
	btrfs_device_set_disk_total_bytes(tgt_device,
					  src_device->disk_total_bytes);
	btrfs_device_set_bytes_used(tgt_device, src_device->bytes_used);
	ASSERT(list_empty(&src_device->resized_list));
	tgt_device->commit_total_bytes = src_device->commit_total_bytes;
	tgt_device->commit_bytes_used = src_device->bytes_used;

	btrfs_assign_next_active_device(src_device, tgt_device);

	list_add(&tgt_device->dev_alloc_list, &fs_info->fs_devices->alloc_list);
	fs_info->fs_devices->rw_devices++;

	btrfs_dev_replace_write_unlock(dev_replace);

	btrfs_rm_dev_replace_blocked(fs_info);

	btrfs_rm_dev_replace_remove_srcdev(src_device);

	btrfs_rm_dev_replace_unblocked(fs_info);

	/*
	 * Increment dev_stats_ccnt so that btrfs_run_dev_stats() will
	 * update on-disk dev stats value during commit transaction
	 */
	atomic_inc(&tgt_device->dev_stats_ccnt);

	/*
	 * this is again a consistent state where no dev_replace procedure
	 * is running, the target device is part of the filesystem, the
	 * source device is not part of the filesystem anymore and its 1st
	 * superblock is scratched out so that it is no longer marked to
	 * belong to this filesystem.
	 */
	mutex_unlock(&fs_info->chunk_mutex);
	mutex_unlock(&fs_info->fs_devices->device_list_mutex);

	/* replace the sysfs entry */
	btrfs_sysfs_rm_device_link(fs_info->fs_devices, src_device);
	btrfs_rm_dev_replace_free_srcdev(fs_info, src_device);

	/* write back the superblocks */
	trans = btrfs_start_transaction(root, 0);
	if (!IS_ERR(trans))
		btrfs_commit_transaction(trans);

	mutex_unlock(&dev_replace->lock_finishing_cancel_unmount);

	return 0;
}

static void btrfs_dev_replace_update_device_in_mapping_tree(
						struct btrfs_fs_info *fs_info,
						struct btrfs_device *srcdev,
						struct btrfs_device *tgtdev)
{
	struct extent_map_tree *em_tree = &fs_info->mapping_tree.map_tree;
	struct extent_map *em;
	struct map_lookup *map;
	u64 start = 0;
	int i;

	write_lock(&em_tree->lock);
	do {
		em = lookup_extent_mapping(em_tree, start, (u64)-1);
		if (!em)
			break;
		map = em->map_lookup;
		for (i = 0; i < map->num_stripes; i++)
			if (srcdev == map->stripes[i].dev)
				map->stripes[i].dev = tgtdev;
		start = em->start + em->len;
		free_extent_map(em);
	} while (start);
	write_unlock(&em_tree->lock);
}

/*
 * Read progress of device replace status according to the state and last
 * stored position. The value format is the same as for
 * btrfs_dev_replace::progress_1000
 */
static u64 btrfs_dev_replace_progress(struct btrfs_fs_info *fs_info)
{
	struct btrfs_dev_replace *dev_replace = &fs_info->dev_replace;
	u64 ret = 0;

	switch (dev_replace->replace_state) {
	case BTRFS_IOCTL_DEV_REPLACE_STATE_NEVER_STARTED:
	case BTRFS_IOCTL_DEV_REPLACE_STATE_CANCELED:
		ret = 0;
		break;
	case BTRFS_IOCTL_DEV_REPLACE_STATE_FINISHED:
		ret = 1000;
		break;
	case BTRFS_IOCTL_DEV_REPLACE_STATE_STARTED:
	case BTRFS_IOCTL_DEV_REPLACE_STATE_SUSPENDED:
		ret = div64_u64(dev_replace->cursor_left,
				div_u64(btrfs_device_get_total_bytes(
						dev_replace->srcdev), 1000));
		break;
	}

	return ret;
}

void btrfs_dev_replace_status(struct btrfs_fs_info *fs_info,
			      struct btrfs_ioctl_dev_replace_args *args)
{
	struct btrfs_dev_replace *dev_replace = &fs_info->dev_replace;

	btrfs_dev_replace_read_lock(dev_replace);
	/* even if !dev_replace_is_valid, the values are good enough for
	 * the replace_status ioctl */
	args->result = BTRFS_IOCTL_DEV_REPLACE_RESULT_NO_ERROR;
	args->status.replace_state = dev_replace->replace_state;
	args->status.time_started = dev_replace->time_started;
	args->status.time_stopped = dev_replace->time_stopped;
	args->status.num_write_errors =
		atomic64_read(&dev_replace->num_write_errors);
	args->status.num_uncorrectable_read_errors =
		atomic64_read(&dev_replace->num_uncorrectable_read_errors);
	args->status.progress_1000 = btrfs_dev_replace_progress(fs_info);
	btrfs_dev_replace_read_unlock(dev_replace);
}

int btrfs_dev_replace_cancel(struct btrfs_fs_info *fs_info)
{
	struct btrfs_dev_replace *dev_replace = &fs_info->dev_replace;
	struct btrfs_device *tgt_device = NULL;
	struct btrfs_device *src_device = NULL;
	struct btrfs_trans_handle *trans;
	struct btrfs_root *root = fs_info->tree_root;
	int result;
	int ret;

	if (sb_rdonly(fs_info->sb))
		return -EROFS;

	mutex_lock(&dev_replace->lock_finishing_cancel_unmount);
	btrfs_dev_replace_write_lock(dev_replace);
	switch (dev_replace->replace_state) {
	case BTRFS_IOCTL_DEV_REPLACE_STATE_NEVER_STARTED:
	case BTRFS_IOCTL_DEV_REPLACE_STATE_FINISHED:
	case BTRFS_IOCTL_DEV_REPLACE_STATE_CANCELED:
		result = BTRFS_IOCTL_DEV_REPLACE_RESULT_NOT_STARTED;
		btrfs_dev_replace_write_unlock(dev_replace);
		break;
	case BTRFS_IOCTL_DEV_REPLACE_STATE_STARTED:
		tgt_device = dev_replace->tgtdev;
		src_device = dev_replace->srcdev;
		btrfs_dev_replace_write_unlock(dev_replace);
		ret = btrfs_scrub_cancel(fs_info);
		if (ret < 0) {
			result = BTRFS_IOCTL_DEV_REPLACE_RESULT_NOT_STARTED;
		} else {
			result = BTRFS_IOCTL_DEV_REPLACE_RESULT_NO_ERROR;
			/*
			 * btrfs_dev_replace_finishing() will handle the
			 * cleanup part
			 */
			btrfs_info_in_rcu(fs_info,
				"dev_replace from %s (devid %llu) to %s canceled",
				btrfs_dev_name(src_device), src_device->devid,
				btrfs_dev_name(tgt_device));
		}
		break;
	case BTRFS_IOCTL_DEV_REPLACE_STATE_SUSPENDED:
		/*
		 * Scrub doing the replace isn't running so we need to do the
		 * cleanup step of btrfs_dev_replace_finishing() here
		 */
		result = BTRFS_IOCTL_DEV_REPLACE_RESULT_NO_ERROR;
		tgt_device = dev_replace->tgtdev;
		src_device = dev_replace->srcdev;
		dev_replace->tgtdev = NULL;
		dev_replace->srcdev = NULL;
		dev_replace->replace_state =
				BTRFS_IOCTL_DEV_REPLACE_STATE_CANCELED;
		dev_replace->time_stopped = ktime_get_real_seconds();
		dev_replace->item_needs_writeback = 1;

		btrfs_dev_replace_write_unlock(dev_replace);

		btrfs_scrub_cancel(fs_info);

		trans = btrfs_start_transaction(root, 0);
		if (IS_ERR(trans)) {
			mutex_unlock(&dev_replace->lock_finishing_cancel_unmount);
			return PTR_ERR(trans);
		}
		ret = btrfs_commit_transaction(trans);
		WARN_ON(ret);

		btrfs_info_in_rcu(fs_info,
		"suspended dev_replace from %s (devid %llu) to %s canceled",
			btrfs_dev_name(src_device), src_device->devid,
			btrfs_dev_name(tgt_device));

		if (tgt_device)
			btrfs_destroy_dev_replace_tgtdev(tgt_device);
		break;
	default:
		result = -EINVAL;
	}

	mutex_unlock(&dev_replace->lock_finishing_cancel_unmount);
	return result;
}

void btrfs_dev_replace_suspend_for_unmount(struct btrfs_fs_info *fs_info)
{
	struct btrfs_dev_replace *dev_replace = &fs_info->dev_replace;

	mutex_lock(&dev_replace->lock_finishing_cancel_unmount);
	btrfs_dev_replace_write_lock(dev_replace);
	switch (dev_replace->replace_state) {
	case BTRFS_IOCTL_DEV_REPLACE_STATE_NEVER_STARTED:
	case BTRFS_IOCTL_DEV_REPLACE_STATE_FINISHED:
	case BTRFS_IOCTL_DEV_REPLACE_STATE_CANCELED:
	case BTRFS_IOCTL_DEV_REPLACE_STATE_SUSPENDED:
		break;
	case BTRFS_IOCTL_DEV_REPLACE_STATE_STARTED:
		dev_replace->replace_state =
			BTRFS_IOCTL_DEV_REPLACE_STATE_SUSPENDED;
		dev_replace->time_stopped = ktime_get_real_seconds();
		dev_replace->item_needs_writeback = 1;
		btrfs_info(fs_info, "suspending dev_replace for unmount");
		break;
	}

	btrfs_dev_replace_write_unlock(dev_replace);
	mutex_unlock(&dev_replace->lock_finishing_cancel_unmount);
}

/* resume dev_replace procedure that was interrupted by unmount */
int btrfs_resume_dev_replace_async(struct btrfs_fs_info *fs_info)
{
	struct task_struct *task;
	struct btrfs_dev_replace *dev_replace = &fs_info->dev_replace;

	btrfs_dev_replace_write_lock(dev_replace);
	switch (dev_replace->replace_state) {
	case BTRFS_IOCTL_DEV_REPLACE_STATE_NEVER_STARTED:
	case BTRFS_IOCTL_DEV_REPLACE_STATE_FINISHED:
	case BTRFS_IOCTL_DEV_REPLACE_STATE_CANCELED:
		btrfs_dev_replace_write_unlock(dev_replace);
		return 0;
	case BTRFS_IOCTL_DEV_REPLACE_STATE_STARTED:
		break;
	case BTRFS_IOCTL_DEV_REPLACE_STATE_SUSPENDED:
		dev_replace->replace_state =
			BTRFS_IOCTL_DEV_REPLACE_STATE_STARTED;
		break;
	}
	if (!dev_replace->tgtdev || !dev_replace->tgtdev->bdev) {
		btrfs_info(fs_info,
			   "cannot continue dev_replace, tgtdev is missing");
		btrfs_info(fs_info,
			   "you may cancel the operation after 'mount -o degraded'");
		dev_replace->replace_state =
					BTRFS_IOCTL_DEV_REPLACE_STATE_SUSPENDED;
		btrfs_dev_replace_write_unlock(dev_replace);
		return 0;
	}
	btrfs_dev_replace_write_unlock(dev_replace);

	/*
	 * This could collide with a paused balance, but the exclusive op logic
	 * should never allow both to start and pause. We don't want to allow
	 * dev-replace to start anyway.
	 */
	if (test_and_set_bit(BTRFS_FS_EXCL_OP, &fs_info->flags)) {
		btrfs_dev_replace_write_lock(dev_replace);
		dev_replace->replace_state =
					BTRFS_IOCTL_DEV_REPLACE_STATE_SUSPENDED;
		btrfs_dev_replace_write_unlock(dev_replace);
		btrfs_info(fs_info,
		"cannot resume dev-replace, other exclusive operation running");
		return 0;
	}

	task = kthread_run(btrfs_dev_replace_kthread, fs_info, "btrfs-devrepl");
	return PTR_ERR_OR_ZERO(task);
}

static int btrfs_dev_replace_kthread(void *data)
{
	struct btrfs_fs_info *fs_info = data;
	struct btrfs_dev_replace *dev_replace = &fs_info->dev_replace;
	u64 progress;
	int ret;

	progress = btrfs_dev_replace_progress(fs_info);
	progress = div_u64(progress, 10);
	btrfs_info_in_rcu(fs_info,
		"continuing dev_replace from %s (devid %llu) to target %s @%u%%",
		btrfs_dev_name(dev_replace->srcdev),
		dev_replace->srcdev->devid,
		btrfs_dev_name(dev_replace->tgtdev),
		(unsigned int)progress);

	ret = btrfs_scrub_dev(fs_info, dev_replace->srcdev->devid,
			      dev_replace->committed_cursor_left,
			      btrfs_device_get_total_bytes(dev_replace->srcdev),
			      &dev_replace->scrub_progress, 0, 1);
	ret = btrfs_dev_replace_finishing(fs_info, ret);
	WARN_ON(ret);

	clear_bit(BTRFS_FS_EXCL_OP, &fs_info->flags);
	return 0;
}

int btrfs_dev_replace_is_ongoing(struct btrfs_dev_replace *dev_replace)
{
	if (!dev_replace->is_valid)
		return 0;

	switch (dev_replace->replace_state) {
	case BTRFS_IOCTL_DEV_REPLACE_STATE_NEVER_STARTED:
	case BTRFS_IOCTL_DEV_REPLACE_STATE_FINISHED:
	case BTRFS_IOCTL_DEV_REPLACE_STATE_CANCELED:
		return 0;
	case BTRFS_IOCTL_DEV_REPLACE_STATE_STARTED:
	case BTRFS_IOCTL_DEV_REPLACE_STATE_SUSPENDED:
		/*
		 * return true even if tgtdev is missing (this is
		 * something that can happen if the dev_replace
		 * procedure is suspended by an umount and then
		 * the tgtdev is missing (or "btrfs dev scan") was
		 * not called and the the filesystem is remounted
		 * in degraded state. This does not stop the
		 * dev_replace procedure. It needs to be canceled
		 * manually if the cancellation is wanted.
		 */
		break;
	}
	return 1;
}

void btrfs_dev_replace_read_lock(struct btrfs_dev_replace *dev_replace)
{
	read_lock(&dev_replace->lock);
	atomic_inc(&dev_replace->read_locks);
}

void btrfs_dev_replace_read_unlock(struct btrfs_dev_replace *dev_replace)
{
	ASSERT(atomic_read(&dev_replace->read_locks) > 0);
	atomic_dec(&dev_replace->read_locks);
	read_unlock(&dev_replace->lock);
}

void btrfs_dev_replace_write_lock(struct btrfs_dev_replace *dev_replace)
{
again:
	wait_event(dev_replace->read_lock_wq,
		   atomic_read(&dev_replace->blocking_readers) == 0);
	write_lock(&dev_replace->lock);
	if (atomic_read(&dev_replace->blocking_readers)) {
		write_unlock(&dev_replace->lock);
		goto again;
	}
}

void btrfs_dev_replace_write_unlock(struct btrfs_dev_replace *dev_replace)
{
	ASSERT(atomic_read(&dev_replace->blocking_readers) == 0);
	write_unlock(&dev_replace->lock);
}

/* inc blocking cnt and release read lock */
void btrfs_dev_replace_set_lock_blocking(
					struct btrfs_dev_replace *dev_replace)
{
	/* only set blocking for read lock */
	ASSERT(atomic_read(&dev_replace->read_locks) > 0);
	atomic_inc(&dev_replace->blocking_readers);
	read_unlock(&dev_replace->lock);
}

/* acquire read lock and dec blocking cnt */
void btrfs_dev_replace_clear_lock_blocking(
					struct btrfs_dev_replace *dev_replace)
{
	/* only set blocking for read lock */
	ASSERT(atomic_read(&dev_replace->read_locks) > 0);
	ASSERT(atomic_read(&dev_replace->blocking_readers) > 0);
	read_lock(&dev_replace->lock);
	/* Barrier implied by atomic_dec_and_test */
	if (atomic_dec_and_test(&dev_replace->blocking_readers))
		cond_wake_up_nomb(&dev_replace->read_lock_wq);
}

void btrfs_bio_counter_inc_noblocked(struct btrfs_fs_info *fs_info)
{
	percpu_counter_inc(&fs_info->bio_counter);
}

void btrfs_bio_counter_sub(struct btrfs_fs_info *fs_info, s64 amount)
{
	percpu_counter_sub(&fs_info->bio_counter, amount);
	cond_wake_up_nomb(&fs_info->replace_wait);
}

void btrfs_bio_counter_inc_blocked(struct btrfs_fs_info *fs_info)
{
	while (1) {
		percpu_counter_inc(&fs_info->bio_counter);
		if (likely(!test_bit(BTRFS_FS_STATE_DEV_REPLACING,
				     &fs_info->fs_state)))
			break;

		btrfs_bio_counter_dec(fs_info);
		wait_event(fs_info->replace_wait,
			   !test_bit(BTRFS_FS_STATE_DEV_REPLACING,
				     &fs_info->fs_state));
	}
}
