/* Copyright (c) 2017-2018, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/irqflags.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/debugfs.h>
#include "cam_sync_util.h"
#include "cam_debug_util.h"

struct sync_device *sync_dev;

int cam_sync_create(int32_t *sync_obj, const char *name)
{
	int rc;
	long idx;
	bool bit;

	do {
		idx = find_first_zero_bit(sync_dev->bitmap, CAM_SYNC_MAX_OBJS);
		if (idx >= CAM_SYNC_MAX_OBJS)
			return -ENOMEM;
		CAM_DBG(CAM_SYNC, "Index location available at idx: %ld", idx);
		bit = test_and_set_bit(idx, sync_dev->bitmap);
	} while (bit);

	spin_lock_bh(&sync_dev->row_spinlocks[idx]);
	rc = cam_sync_init_object(sync_dev->sync_table, idx, name);
	if (rc) {
		CAM_ERR(CAM_SYNC, "Error: Unable to init row at idx = %ld",
			idx);
		clear_bit(idx, sync_dev->bitmap);
		spin_unlock_bh(&sync_dev->row_spinlocks[idx]);
		return -EINVAL;
	}

	*sync_obj = idx;
	CAM_DBG(CAM_SYNC, "sync_obj: %i", *sync_obj);
	spin_unlock_bh(&sync_dev->row_spinlocks[idx]);

	return rc;
}

int cam_sync_register_callback(sync_callback cb_func,
	void *userdata, int32_t sync_obj)
{
	struct sync_callback_info *sync_cb;
	struct sync_callback_info *cb_info;
	struct sync_table_row *row = NULL;

	if (sync_obj >= CAM_SYNC_MAX_OBJS || sync_obj <= 0 || !cb_func)
		return -EINVAL;

	spin_lock_bh(&sync_dev->row_spinlocks[sync_obj]);
	row = sync_dev->sync_table + sync_obj;

	if (row->state == CAM_SYNC_STATE_INVALID) {
		CAM_ERR(CAM_SYNC,
			"Error: accessing an uninitialized sync obj %d",
			sync_obj);
		spin_unlock_bh(&sync_dev->row_spinlocks[sync_obj]);
		return -EINVAL;
	}

	/* Don't register if callback was registered earlier */
	list_for_each_entry(cb_info, &row->callback_list, list) {
		if (cb_info->callback_func == cb_func &&
			cb_info->cb_data == userdata) {
			CAM_ERR(CAM_SYNC, "Duplicate register for sync_obj %d",
				sync_obj);
			spin_unlock_bh(&sync_dev->row_spinlocks[sync_obj]);
			return -EALREADY;
		}
	}

	sync_cb = kzalloc(sizeof(*sync_cb), GFP_ATOMIC);
	if (!sync_cb) {
		spin_unlock_bh(&sync_dev->row_spinlocks[sync_obj]);
		return -ENOMEM;
	}

	/* Trigger callback if sync object is already in SIGNALED state */
	if ((row->state == CAM_SYNC_STATE_SIGNALED_SUCCESS ||
		row->state == CAM_SYNC_STATE_SIGNALED_ERROR) &&
		(!row->remaining)) {
		sync_cb->callback_func = cb_func;
		sync_cb->cb_data = userdata;
		sync_cb->sync_obj = sync_obj;
		INIT_WORK(&sync_cb->cb_dispatch_work,
			cam_sync_util_cb_dispatch);
		sync_cb->status = row->state;
		CAM_DBG(CAM_SYNC, "Callback trigger for sync object:%d",
			sync_cb->sync_obj);
		queue_work(sync_dev->work_queue,
			&sync_cb->cb_dispatch_work);

		spin_unlock_bh(&sync_dev->row_spinlocks[sync_obj]);
		return 0;
	}

	sync_cb->callback_func = cb_func;
	sync_cb->cb_data = userdata;
	sync_cb->sync_obj = sync_obj;
	INIT_WORK(&sync_cb->cb_dispatch_work, cam_sync_util_cb_dispatch);
	list_add_tail(&sync_cb->list, &row->callback_list);
	spin_unlock_bh(&sync_dev->row_spinlocks[sync_obj]);

	return 0;
}

int cam_sync_deregister_callback(sync_callback cb_func,
	void *userdata, int32_t sync_obj)
{
	struct sync_table_row *row = NULL;
	struct sync_callback_info *sync_cb, *temp;
	bool found = false;

	if (sync_obj >= CAM_SYNC_MAX_OBJS || sync_obj <= 0)
		return -EINVAL;

	spin_lock_bh(&sync_dev->row_spinlocks[sync_obj]);
	row = sync_dev->sync_table + sync_obj;

	if (row->state == CAM_SYNC_STATE_INVALID) {
		CAM_ERR(CAM_SYNC,
			"Error: accessing an uninitialized sync obj = %d",
			sync_obj);
		spin_unlock_bh(&sync_dev->row_spinlocks[sync_obj]);
		return -EINVAL;
	}

	CAM_DBG(CAM_SYNC, "deregistered callback for sync object:%d",
		sync_obj);
	list_for_each_entry_safe(sync_cb, temp, &row->callback_list, list) {
		if (sync_cb->callback_func == cb_func &&
			sync_cb->cb_data == userdata) {
			list_del_init(&sync_cb->list);
			kfree(sync_cb);
			found = true;
		}
	}

	spin_unlock_bh(&sync_dev->row_spinlocks[sync_obj]);
	return found ? 0 : -ENOENT;
}

int cam_sync_signal(int32_t sync_obj, uint32_t status)
{
	int rc;
	struct sync_table_row *row = NULL;
	struct sync_table_row *parent_row = NULL;
	struct sync_callback_info *sync_cb;
	struct sync_user_payload *payload_info;
	struct sync_parent_info *parent_info;
	struct list_head sync_list;
	struct cam_signalable_info *list_info = NULL;
	struct cam_signalable_info *temp_list_info = NULL;

	/* Objects to be signaled will be added into this list */
	INIT_LIST_HEAD(&sync_list);

	if (sync_obj >= CAM_SYNC_MAX_OBJS || sync_obj <= 0) {
		CAM_ERR(CAM_SYNC, "Error: Out of range sync obj (0 <= %d < %d)",
			sync_obj, CAM_SYNC_MAX_OBJS);
		return -EINVAL;
	}
	row = sync_dev->sync_table + sync_obj;
	spin_lock_bh(&sync_dev->row_spinlocks[sync_obj]);
	if (row->state == CAM_SYNC_STATE_INVALID) {
		spin_unlock_bh(&sync_dev->row_spinlocks[sync_obj]);
		CAM_ERR(CAM_SYNC,
			"Error: accessing an uninitialized sync obj = %d",
			sync_obj);
		return -EINVAL;
	}

	if (row->type == CAM_SYNC_TYPE_GROUP) {
		spin_unlock_bh(&sync_dev->row_spinlocks[sync_obj]);
		CAM_ERR(CAM_SYNC,
			"Error: Signaling a GROUP sync object = %d",
			sync_obj);
		return -EINVAL;
	}

	if (row->state != CAM_SYNC_STATE_ACTIVE) {
		spin_unlock_bh(&sync_dev->row_spinlocks[sync_obj]);
		CAM_ERR(CAM_SYNC,
			"Error: Sync object already signaled sync_obj = %d",
			sync_obj);
		return -EALREADY;
	}

	if (status != CAM_SYNC_STATE_SIGNALED_SUCCESS &&
		status != CAM_SYNC_STATE_SIGNALED_ERROR) {
		spin_unlock_bh(&sync_dev->row_spinlocks[sync_obj]);
		CAM_ERR(CAM_SYNC,
			"Error: signaling with undefined status = %d",
			status);
		return -EINVAL;
	}

	row->state = status;
	rc = cam_sync_util_add_to_signalable_list(sync_obj, status, &sync_list);
	if (rc < 0) {
		spin_unlock_bh(&sync_dev->row_spinlocks[sync_obj]);
		CAM_ERR(CAM_SYNC,
			"Error: Unable to add sync object :%d to signalable list",
			sync_obj);
		return rc;
	}

	/*
	 * Now iterate over all parents of this object and if they too need to
	 * be signaled add them to the list
	 */
	list_for_each_entry(parent_info,
		&row->parents_list,
		list) {
		parent_row = sync_dev->sync_table + parent_info->sync_id;
		spin_lock_bh(&sync_dev->row_spinlocks[parent_info->sync_id]);
		parent_row->remaining--;

		parent_row->state = cam_sync_util_get_state(
			parent_row->state,
			status);

		if (!parent_row->remaining) {
			rc = cam_sync_util_add_to_signalable_list
				(parent_info->sync_id,
					parent_row->state,
					&sync_list);
			if (rc < 0) {
				spin_unlock_bh(
					&sync_dev->row_spinlocks[
						parent_info->sync_id]);
				spin_unlock_bh(
					&sync_dev->row_spinlocks[sync_obj]);
				return rc;
			}
		}
		spin_unlock_bh(&sync_dev->row_spinlocks[parent_info->sync_id]);
	}

	spin_unlock_bh(&sync_dev->row_spinlocks[sync_obj]);

	/*
	 * Now dispatch the various sync objects collected so far, in our
	 * list
	 */
	list_for_each_entry_safe(list_info,
		temp_list_info,
		&sync_list,
		list) {
		struct sync_table_row *signalable_row = NULL;
		struct sync_callback_info *temp_sync_cb;
		struct sync_user_payload *temp_payload_info;

		signalable_row = sync_dev->sync_table + list_info->sync_obj;

		spin_lock_bh(&sync_dev->row_spinlocks[list_info->sync_obj]);
		if (signalable_row->state == CAM_SYNC_STATE_INVALID) {
			spin_unlock_bh(
				&sync_dev->row_spinlocks[list_info->sync_obj]);
			continue;
		}

		/* Dispatch kernel callbacks if any were registered earlier */

		list_for_each_entry_safe(sync_cb,
			temp_sync_cb, &signalable_row->callback_list, list) {
			sync_cb->status = list_info->status;
			list_del_init(&sync_cb->list);
			queue_work(sync_dev->work_queue,
				&sync_cb->cb_dispatch_work);
		}

		/* Dispatch user payloads if any were registered earlier */
		list_for_each_entry_safe(payload_info, temp_payload_info,
			&signalable_row->user_payload_list, list) {
			spin_lock_bh(&sync_dev->cam_sync_eventq_lock);
			if (!sync_dev->cam_sync_eventq) {
				spin_unlock_bh(
				&sync_dev->cam_sync_eventq_lock);
				break;
			}
			spin_unlock_bh(&sync_dev->cam_sync_eventq_lock);
			cam_sync_util_send_v4l2_event(
				CAM_SYNC_V4L_EVENT_ID_CB_TRIG,
				list_info->sync_obj,
				list_info->status,
				payload_info->payload_data,
				CAM_SYNC_PAYLOAD_WORDS * sizeof(__u64));

			list_del_init(&payload_info->list);
			/*
			 * We can free the list node here because
			 * sending V4L event will make a deep copy
			 * anyway
			 */
			kfree(payload_info);
		}

		/*
		 * This needs to be done because we want to unblock anyone
		 * who might be blocked and waiting on this sync object
		 */
		complete_all(&signalable_row->signaled);

		spin_unlock_bh(&sync_dev->row_spinlocks[list_info->sync_obj]);

		list_del_init(&list_info->list);
		kfree(list_info);
	}

	return rc;
}

int cam_sync_merge(int32_t *sync_obj, uint32_t num_objs, int32_t *merged_obj)
{
	int rc;
	long idx = 0;
	bool bit;

	if (!sync_obj || !merged_obj) {
		CAM_ERR(CAM_SYNC, "Invalid pointer(s)");
		return -EINVAL;
	}

	rc = cam_sync_util_validate_merge(sync_obj,
		num_objs);
	if (rc < 0) {
		CAM_ERR(CAM_SYNC, "Validation failed, Merge not allowed");
		return -EINVAL;
	}

	do {
		idx = find_first_zero_bit(sync_dev->bitmap, CAM_SYNC_MAX_OBJS);
		if (idx >= CAM_SYNC_MAX_OBJS)
			return -ENOMEM;
		bit = test_and_set_bit(idx, sync_dev->bitmap);
	} while (bit);

	spin_lock_bh(&sync_dev->row_spinlocks[idx]);

	rc = cam_sync_init_group_object(sync_dev->sync_table,
		idx, sync_obj,
		num_objs);
	if (rc < 0) {
		CAM_ERR(CAM_SYNC, "Error: Unable to init row at idx = %ld",
			idx);
		clear_bit(idx, sync_dev->bitmap);
		spin_unlock_bh(&sync_dev->row_spinlocks[idx]);
		return -EINVAL;
	}
	CAM_DBG(CAM_SYNC, "Init row at idx:%ld to merge objects", idx);
	*merged_obj = idx;
	spin_unlock_bh(&sync_dev->row_spinlocks[idx]);

	return 0;
}

int cam_sync_destroy(int32_t sync_obj)
{
	CAM_DBG(CAM_SYNC, "sync_obj: %i", sync_obj);
	return cam_sync_deinit_object(sync_dev->sync_table, sync_obj);
}

int cam_sync_wait(int32_t sync_obj, uint64_t timeout_ms)
{
	unsigned long timeleft;
	int rc = -EINVAL;
	struct sync_table_row *row = NULL;

	if (sync_obj >= CAM_SYNC_MAX_OBJS || sync_obj <= 0)
		return -EINVAL;

	row = sync_dev->sync_table + sync_obj;

	if (row->state == CAM_SYNC_STATE_INVALID) {
		CAM_ERR(CAM_SYNC,
			"Error: accessing an uninitialized sync obj = %d",
			sync_obj);
		return -EINVAL;
	}

	timeleft = wait_for_completion_timeout(&row->signaled,
		msecs_to_jiffies(timeout_ms));

	if (!timeleft) {
		CAM_ERR(CAM_SYNC,
			"Error: timed out for sync obj = %d", sync_obj);
		rc = -ETIMEDOUT;
	} else {
		switch (row->state) {
		case CAM_SYNC_STATE_INVALID:
		case CAM_SYNC_STATE_ACTIVE:
		case CAM_SYNC_STATE_SIGNALED_ERROR:
			CAM_ERR(CAM_SYNC,
				"Error: Wait on invalid state = %d, obj = %d",
				row->state, sync_obj);
			rc = -EINVAL;
			break;
		case CAM_SYNC_STATE_SIGNALED_SUCCESS:
			rc = 0;
			break;
		default:
			rc = -EINVAL;
			break;
		}
	}

	return rc;
}

static int cam_sync_handle_create(struct cam_private_ioctl_arg *k_ioctl)
{
	struct cam_sync_info sync_create;
	int result;

	if (k_ioctl->size != sizeof(struct cam_sync_info))
		return -EINVAL;

	if (!k_ioctl->ioctl_ptr)
		return -EINVAL;

	if (copy_from_user(&sync_create,
		(void *)k_ioctl->ioctl_ptr,
		k_ioctl->size))
		return -EFAULT;

	result = cam_sync_create(&sync_create.sync_obj,
		sync_create.name);

	if (!result)
		if (copy_to_user((void *)k_ioctl->ioctl_ptr,
			&sync_create,
			k_ioctl->size))
			return -EFAULT;

	return result;
}

static int cam_sync_handle_signal(struct cam_private_ioctl_arg *k_ioctl)
{
	struct cam_sync_signal sync_signal;

	if (k_ioctl->size != sizeof(struct cam_sync_signal))
		return -EINVAL;

	if (!k_ioctl->ioctl_ptr)
		return -EINVAL;

	if (copy_from_user(&sync_signal,
		(void *)k_ioctl->ioctl_ptr,
		k_ioctl->size))
		return -EFAULT;

	return cam_sync_signal(sync_signal.sync_obj,
		sync_signal.sync_state);
}

static int cam_sync_handle_merge(struct cam_private_ioctl_arg *k_ioctl)
{
	struct cam_sync_merge sync_merge;
	uint32_t *sync_objs;
	uint32_t num_objs;
	uint32_t size;
	int result;

	if (k_ioctl->size != sizeof(struct cam_sync_merge))
		return -EINVAL;

	if (!k_ioctl->ioctl_ptr)
		return -EINVAL;

	if (copy_from_user(&sync_merge,
		(void *)k_ioctl->ioctl_ptr,
		k_ioctl->size))
		return -EFAULT;

	if (sync_merge.num_objs >= CAM_SYNC_MAX_OBJS)
		return -EINVAL;

	size = sizeof(uint32_t) * sync_merge.num_objs;
	sync_objs = kzalloc(size, GFP_ATOMIC);

	if (!sync_objs)
		return -ENOMEM;

	if (copy_from_user(sync_objs,
	(void *)sync_merge.sync_objs,
	sizeof(uint32_t) * sync_merge.num_objs)) {
		kfree(sync_objs);
		return -EFAULT;
	}

	num_objs = sync_merge.num_objs;

	result = cam_sync_merge(sync_objs,
		num_objs,
		&sync_merge.merged);

	if (!result)
		if (copy_to_user((void *)k_ioctl->ioctl_ptr,
			&sync_merge,
			k_ioctl->size)) {
			kfree(sync_objs);
			return -EFAULT;
	}

	kfree(sync_objs);

	return result;
}

static int cam_sync_handle_wait(struct cam_private_ioctl_arg *k_ioctl)
{
	struct cam_sync_wait sync_wait;

	if (k_ioctl->size != sizeof(struct cam_sync_wait))
		return -EINVAL;

	if (!k_ioctl->ioctl_ptr)
		return -EINVAL;

	if (copy_from_user(&sync_wait,
		(void *)k_ioctl->ioctl_ptr,
		k_ioctl->size))
		return -EFAULT;

	k_ioctl->result = cam_sync_wait(sync_wait.sync_obj,
		sync_wait.timeout_ms);

	return 0;
}

static int cam_sync_handle_destroy(struct cam_private_ioctl_arg *k_ioctl)
{
	struct cam_sync_info sync_create;

	if (k_ioctl->size != sizeof(struct cam_sync_info))
		return -EINVAL;

	if (!k_ioctl->ioctl_ptr)
		return -EINVAL;

	if (copy_from_user(&sync_create,
		(void *)k_ioctl->ioctl_ptr,
		k_ioctl->size))
		return -EFAULT;

	return cam_sync_destroy(sync_create.sync_obj);
}

static int cam_sync_handle_register_user_payload(
	struct cam_private_ioctl_arg *k_ioctl)
{
	struct cam_sync_userpayload_info userpayload_info;
	struct sync_user_payload *user_payload_kernel;
	struct sync_user_payload *user_payload_iter;
	struct sync_user_payload *temp_upayload_kernel;
	uint32_t sync_obj;
	struct sync_table_row *row = NULL;

	if (k_ioctl->size != sizeof(struct cam_sync_userpayload_info))
		return -EINVAL;

	if (!k_ioctl->ioctl_ptr)
		return -EINVAL;

	if (copy_from_user(&userpayload_info,
		(void *)k_ioctl->ioctl_ptr,
		k_ioctl->size))
		return -EFAULT;

	sync_obj = userpayload_info.sync_obj;
	if (sync_obj >= CAM_SYNC_MAX_OBJS || sync_obj <= 0)
		return -EINVAL;

	user_payload_kernel = kzalloc(sizeof(*user_payload_kernel), GFP_KERNEL);
	if (!user_payload_kernel)
		return -ENOMEM;

	memcpy(user_payload_kernel->payload_data,
		userpayload_info.payload,
		CAM_SYNC_PAYLOAD_WORDS * sizeof(__u64));

	spin_lock_bh(&sync_dev->row_spinlocks[sync_obj]);
	row =  sync_dev->sync_table + sync_obj;

	if (row->state == CAM_SYNC_STATE_INVALID) {
		CAM_ERR(CAM_SYNC,
			"Error: accessing an uninitialized sync obj = %d",
			sync_obj);
		spin_unlock_bh(&sync_dev->row_spinlocks[sync_obj]);
		kfree(user_payload_kernel);
		return -EINVAL;
	}

	if (row->state == CAM_SYNC_STATE_SIGNALED_SUCCESS ||
		row->state == CAM_SYNC_STATE_SIGNALED_ERROR) {

		cam_sync_util_send_v4l2_event(CAM_SYNC_V4L_EVENT_ID_CB_TRIG,
			sync_obj,
			row->state,
			user_payload_kernel->payload_data,
			CAM_SYNC_USER_PAYLOAD_SIZE * sizeof(__u64));

		spin_unlock_bh(&sync_dev->row_spinlocks[sync_obj]);
		kfree(user_payload_kernel);
		return 0;
	}

	list_for_each_entry_safe(user_payload_iter,
		temp_upayload_kernel,
		&row->user_payload_list,
		list) {
		if (user_payload_iter->payload_data[0] ==
				user_payload_kernel->payload_data[0] &&
			user_payload_iter->payload_data[1] ==
				user_payload_kernel->payload_data[1]) {

			spin_unlock_bh(&sync_dev->row_spinlocks[sync_obj]);
			kfree(user_payload_kernel);
			return -EALREADY;
		}
	}

	list_add_tail(&user_payload_kernel->list, &row->user_payload_list);
	spin_unlock_bh(&sync_dev->row_spinlocks[sync_obj]);
	return 0;
}

static int cam_sync_handle_deregister_user_payload(
	struct cam_private_ioctl_arg *k_ioctl)
{
	struct cam_sync_userpayload_info userpayload_info;
	struct sync_user_payload *user_payload_kernel, *temp;
	uint32_t sync_obj;
	struct sync_table_row *row = NULL;

	if (k_ioctl->size != sizeof(struct cam_sync_userpayload_info)) {
		CAM_ERR(CAM_SYNC, "Incorrect ioctl size");
		return -EINVAL;
	}

	if (!k_ioctl->ioctl_ptr) {
		CAM_ERR(CAM_SYNC, "Invalid embedded ioctl ptr");
		return -EINVAL;
	}

	if (copy_from_user(&userpayload_info,
		(void *)k_ioctl->ioctl_ptr,
		k_ioctl->size))
		return -EFAULT;

	sync_obj = userpayload_info.sync_obj;
	if (sync_obj >= CAM_SYNC_MAX_OBJS || sync_obj <= 0)
		return -EINVAL;

	spin_lock_bh(&sync_dev->row_spinlocks[sync_obj]);
	row =  sync_dev->sync_table + sync_obj;

	if (row->state == CAM_SYNC_STATE_INVALID) {
		CAM_ERR(CAM_SYNC,
			"Error: accessing an uninitialized sync obj = %d",
			sync_obj);
		spin_unlock_bh(&sync_dev->row_spinlocks[sync_obj]);
		return -EINVAL;
	}

	list_for_each_entry_safe(user_payload_kernel, temp,
				&row->user_payload_list, list) {
		if (user_payload_kernel->payload_data[0] ==
				userpayload_info.payload[0] &&
				user_payload_kernel->payload_data[1] ==
				userpayload_info.payload[1]) {
			list_del_init(&user_payload_kernel->list);
			kfree(user_payload_kernel);
		}
	}

	spin_unlock_bh(&sync_dev->row_spinlocks[sync_obj]);
	return 0;
}

static long cam_sync_dev_ioctl(struct file *filep, void *fh,
		bool valid_prio, unsigned int cmd, void *arg)
{
	int32_t rc;
	struct sync_device *sync_dev = video_drvdata(filep);
	struct cam_private_ioctl_arg k_ioctl;

	if (!sync_dev) {
		CAM_ERR(CAM_SYNC, "sync_dev NULL");
		return -EINVAL;
	}

	if (!arg)
		return -EINVAL;

	if (cmd != CAM_PRIVATE_IOCTL_CMD)
		return -ENOIOCTLCMD;

	k_ioctl = *(struct cam_private_ioctl_arg *)arg;

	switch (k_ioctl.id) {
	case CAM_SYNC_CREATE:
		rc = cam_sync_handle_create(&k_ioctl);
		break;
	case CAM_SYNC_DESTROY:
		rc = cam_sync_handle_destroy(&k_ioctl);
		break;
	case CAM_SYNC_REGISTER_PAYLOAD:
		rc = cam_sync_handle_register_user_payload(
			&k_ioctl);
		break;
	case CAM_SYNC_DEREGISTER_PAYLOAD:
		rc = cam_sync_handle_deregister_user_payload(
			&k_ioctl);
		break;
	case CAM_SYNC_SIGNAL:
		rc = cam_sync_handle_signal(&k_ioctl);
		break;
	case CAM_SYNC_MERGE:
		rc = cam_sync_handle_merge(&k_ioctl);
		break;
	case CAM_SYNC_WAIT:
		rc = cam_sync_handle_wait(&k_ioctl);
		((struct cam_private_ioctl_arg *)arg)->result =
			k_ioctl.result;
		break;
	default:
		rc = -ENOIOCTLCMD;
	}

	return rc;
}

static unsigned int cam_sync_poll(struct file *f,
	struct poll_table_struct *pll_table)
{
	int rc = 0;
	struct v4l2_fh *eventq = f->private_data;

	if (!eventq)
		return -EINVAL;

	poll_wait(f, &eventq->wait, pll_table);

	if (v4l2_event_pending(eventq))
		rc = POLLPRI;

	return rc;
}

static int cam_sync_open(struct file *filep)
{
	int rc;
	struct sync_device *sync_dev = video_drvdata(filep);

	if (!sync_dev) {
		CAM_ERR(CAM_SYNC, "Sync device NULL");
		return -ENODEV;
	}

	mutex_lock(&sync_dev->table_lock);
	if (sync_dev->open_cnt >= 1) {
		mutex_unlock(&sync_dev->table_lock);
		return -EALREADY;
	}

	rc = v4l2_fh_open(filep);
	if (!rc) {
		sync_dev->open_cnt++;
		spin_lock_bh(&sync_dev->cam_sync_eventq_lock);
		sync_dev->cam_sync_eventq = filep->private_data;
		spin_unlock_bh(&sync_dev->cam_sync_eventq_lock);
	} else {
		CAM_ERR(CAM_SYNC, "v4l2_fh_open failed : %d", rc);
	}
	mutex_unlock(&sync_dev->table_lock);

	return rc;
}

static int cam_sync_close(struct file *filep)
{
	int rc = 0;
	int i;
	struct sync_device *sync_dev = video_drvdata(filep);

	if (!sync_dev) {
		CAM_ERR(CAM_SYNC, "Sync device NULL");
		rc = -ENODEV;
		return rc;
	}
	mutex_lock(&sync_dev->table_lock);
	sync_dev->open_cnt--;
	if (!sync_dev->open_cnt) {
		for (i = 1; i < CAM_SYNC_MAX_OBJS; i++) {
			struct sync_table_row *row =
			sync_dev->sync_table + i;

			/*
			 * Signal all ACTIVE objects as ERR, but we don't
			 * care about the return status here apart from logging
			 * it.
			 */
			if (row->state == CAM_SYNC_STATE_ACTIVE) {
				rc = cam_sync_signal(i,
					CAM_SYNC_STATE_SIGNALED_ERROR);
				if (rc < 0)
					CAM_ERR(CAM_SYNC,
					  "Cleanup signal fail idx:%d\n",
					  i);
			}
		}

		/*
		 * Flush the work queue to wait for pending signal callbacks to
		 * finish
		 */
		flush_workqueue(sync_dev->work_queue);

		/*
		 * Now that all callbacks worker threads have finished,
		 * destroy the sync objects
		 */
		for (i = 1; i < CAM_SYNC_MAX_OBJS; i++) {
			struct sync_table_row *row =
			sync_dev->sync_table + i;

			if (row->state != CAM_SYNC_STATE_INVALID) {
				rc = cam_sync_destroy(i);
				if (rc < 0)
					CAM_ERR(CAM_SYNC,
					  "Cleanup destroy fail:idx:%d\n",
					  i);
			}
		}
	}
	mutex_unlock(&sync_dev->table_lock);
	spin_lock_bh(&sync_dev->cam_sync_eventq_lock);
	sync_dev->cam_sync_eventq = NULL;
	spin_unlock_bh(&sync_dev->cam_sync_eventq_lock);
	v4l2_fh_release(filep);

	return rc;
}

int cam_sync_subscribe_event(struct v4l2_fh *fh,
		const struct v4l2_event_subscription *sub)
{
	return v4l2_event_subscribe(fh, sub, CAM_SYNC_MAX_V4L2_EVENTS, NULL);
}

int cam_sync_unsubscribe_event(struct v4l2_fh *fh,
		const struct v4l2_event_subscription *sub)
{
	return v4l2_event_unsubscribe(fh, sub);
}

static const struct v4l2_ioctl_ops g_cam_sync_ioctl_ops = {
	.vidioc_subscribe_event = cam_sync_subscribe_event,
	.vidioc_unsubscribe_event = cam_sync_unsubscribe_event,
	.vidioc_default = cam_sync_dev_ioctl,
};

static struct v4l2_file_operations cam_sync_v4l2_fops = {
	.owner = THIS_MODULE,
	.open  = cam_sync_open,
	.release = cam_sync_close,
	.poll = cam_sync_poll,
	.unlocked_ioctl   = video_ioctl2,
#ifdef CONFIG_COMPAT
	.compat_ioctl32 = video_ioctl2,
#endif
};

#if defined(CONFIG_MEDIA_CONTROLLER)
static int cam_sync_media_controller_init(struct sync_device *sync_dev,
	struct platform_device *pdev)
{
	int rc;

	sync_dev->v4l2_dev.mdev = kzalloc(sizeof(struct media_device),
		GFP_KERNEL);
	if (!sync_dev->v4l2_dev.mdev)
		return -ENOMEM;

	media_device_init(sync_dev->v4l2_dev.mdev);
	strlcpy(sync_dev->v4l2_dev.mdev->model, CAM_SYNC_DEVICE_NAME,
			sizeof(sync_dev->v4l2_dev.mdev->model));
	sync_dev->v4l2_dev.mdev->dev = &(pdev->dev);

	rc = media_device_register(sync_dev->v4l2_dev.mdev);
	if (rc < 0)
		goto register_fail;

	rc = media_entity_pads_init(&sync_dev->vdev->entity, 0, NULL);
	if (rc < 0)
		goto entity_fail;

	return 0;

entity_fail:
	media_device_unregister(sync_dev->v4l2_dev.mdev);
register_fail:
	media_device_cleanup(sync_dev->v4l2_dev.mdev);
	return rc;
}

static void cam_sync_media_controller_cleanup(struct sync_device *sync_dev)
{
	media_entity_cleanup(&sync_dev->vdev->entity);
	media_device_unregister(sync_dev->v4l2_dev.mdev);
	media_device_cleanup(sync_dev->v4l2_dev.mdev);
	kfree(sync_dev->v4l2_dev.mdev);
}

static void cam_sync_init_entity(struct sync_device *sync_dev)
{
	sync_dev->vdev->entity.function = CAM_SYNC_DEVICE_TYPE;
	sync_dev->vdev->entity.name =
				video_device_node_name(sync_dev->vdev);
}
#else
static int cam_sync_media_controller_init(struct sync_device *sync_dev,
	struct platform_device *pdev)
{
	return 0;
}

static void cam_sync_media_controller_cleanup(struct sync_device *sync_dev)
{
}

static void cam_sync_init_entity(struct sync_device *sync_dev)
{
}
#endif

static int cam_sync_probe(struct platform_device *pdev)
{
	int rc;
	int idx;

	sync_dev = kzalloc(sizeof(*sync_dev), GFP_KERNEL);
	if (!sync_dev)
		return -ENOMEM;

	mutex_init(&sync_dev->table_lock);
	spin_lock_init(&sync_dev->cam_sync_eventq_lock);

	for (idx = 0; idx < CAM_SYNC_MAX_OBJS; idx++)
		spin_lock_init(&sync_dev->row_spinlocks[idx]);

	sync_dev->vdev = video_device_alloc();
	if (!sync_dev->vdev) {
		rc = -ENOMEM;
		goto vdev_fail;
	}

	rc = cam_sync_media_controller_init(sync_dev, pdev);
	if (rc < 0)
		goto mcinit_fail;

	sync_dev->vdev->v4l2_dev = &sync_dev->v4l2_dev;

	rc = v4l2_device_register(&(pdev->dev), sync_dev->vdev->v4l2_dev);
	if (rc < 0)
		goto register_fail;

	strlcpy(sync_dev->vdev->name, CAM_SYNC_NAME,
				sizeof(sync_dev->vdev->name));
	sync_dev->vdev->release  = video_device_release;
	sync_dev->vdev->fops     = &cam_sync_v4l2_fops;
	sync_dev->vdev->ioctl_ops = &g_cam_sync_ioctl_ops;
	sync_dev->vdev->minor     = -1;
	sync_dev->vdev->vfl_type  = VFL_TYPE_GRABBER;
	rc = video_register_device(sync_dev->vdev,
		VFL_TYPE_GRABBER, -1);
	if (rc < 0)
		goto v4l2_fail;

	cam_sync_init_entity(sync_dev);
	video_set_drvdata(sync_dev->vdev, sync_dev);
	memset(&sync_dev->sync_table, 0, sizeof(sync_dev->sync_table));
	memset(&sync_dev->bitmap, 0, sizeof(sync_dev->bitmap));
	bitmap_zero(sync_dev->bitmap, CAM_SYNC_MAX_OBJS);

	/*
	 * We treat zero as invalid handle, so we will keep the 0th bit set
	 * always
	 */
	set_bit(0, sync_dev->bitmap);

	sync_dev->work_queue = alloc_workqueue(CAM_SYNC_WORKQUEUE_NAME,
		WQ_HIGHPRI | WQ_UNBOUND, 1);

	if (!sync_dev->work_queue) {
		CAM_ERR(CAM_SYNC,
			"Error: high priority work queue creation failed");
		rc = -ENOMEM;
		goto v4l2_fail;
	}

	return rc;

v4l2_fail:
	v4l2_device_unregister(sync_dev->vdev->v4l2_dev);
register_fail:
	cam_sync_media_controller_cleanup(sync_dev);
mcinit_fail:
	video_device_release(sync_dev->vdev);
vdev_fail:
	mutex_destroy(&sync_dev->table_lock);
	kfree(sync_dev);
	return rc;
}

static int cam_sync_remove(struct platform_device *pdev)
{
	v4l2_device_unregister(sync_dev->vdev->v4l2_dev);
	cam_sync_media_controller_cleanup(sync_dev);
	video_device_release(sync_dev->vdev);
	kfree(sync_dev);
	sync_dev = NULL;

	return 0;
}

static struct platform_device cam_sync_device = {
	.name = "cam_sync",
	.id = -1,
};

static struct platform_driver cam_sync_driver = {
	.probe = cam_sync_probe,
	.remove = cam_sync_remove,
	.driver = {
		.name = "cam_sync",
		.owner = THIS_MODULE,
	},
};

static int __init cam_sync_init(void)
{
	int rc;

	rc = platform_device_register(&cam_sync_device);
	if (rc)
		return -ENODEV;

	return platform_driver_register(&cam_sync_driver);
}

static void __exit cam_sync_exit(void)
{
	int idx;

	for (idx = 0; idx < CAM_SYNC_MAX_OBJS; idx++)
		spin_lock_init(&sync_dev->row_spinlocks[idx]);
	platform_driver_unregister(&cam_sync_driver);
	platform_device_unregister(&cam_sync_device);
	kfree(sync_dev);
}

module_init(cam_sync_init);
module_exit(cam_sync_exit);
MODULE_DESCRIPTION("Camera sync driver");
MODULE_LICENSE("GPL v2");
