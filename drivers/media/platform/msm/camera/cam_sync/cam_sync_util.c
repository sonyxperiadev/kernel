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

#include "cam_sync_util.h"

int cam_sync_util_find_and_set_empty_row(struct sync_device *sync_dev,
	long *idx)
{
	int rc = 0;

	mutex_lock(&sync_dev->table_lock);

	*idx = find_first_zero_bit(sync_dev->bitmap, CAM_SYNC_MAX_OBJS);

	if (*idx < CAM_SYNC_MAX_OBJS)
		set_bit(*idx, sync_dev->bitmap);
	else
		rc = -1;

	mutex_unlock(&sync_dev->table_lock);

	return rc;
}

int cam_sync_init_object(struct sync_table_row *table,
	uint32_t idx,
	const char *name)
{
	struct sync_table_row *row = table + idx;

	if (!table || idx <= 0 || idx >= CAM_SYNC_MAX_OBJS)
		return -EINVAL;

	if (name)
		strlcpy(row->name, name, SYNC_DEBUG_NAME_LEN);
	INIT_LIST_HEAD(&row->parents_list);
	INIT_LIST_HEAD(&row->children_list);
	row->type = CAM_SYNC_TYPE_INDV;
	row->sync_id = idx;
	row->state = CAM_SYNC_STATE_ACTIVE;
	row->remaining = 0;
	init_completion(&row->signaled);
	INIT_LIST_HEAD(&row->callback_list);
	INIT_LIST_HEAD(&row->user_payload_list);
	CAM_DBG(CAM_SYNC,
		"row name:%s sync_id:%i [idx:%u] row_state:%u ",
		row->name, row->sync_id, idx, row->state);

	return 0;
}

uint32_t cam_sync_util_get_group_object_state(struct sync_table_row *table,
	uint32_t *sync_objs,
	uint32_t num_objs)
{
	int i;
	struct sync_table_row *child_row = NULL;
	int success_count = 0;
	int active_count = 0;

	if (!table || !sync_objs)
		return CAM_SYNC_STATE_SIGNALED_ERROR;

	/*
	 * We need to arrive at the state of the merged object based on
	 * counts of error, active and success states of all children objects
	 */
	for (i = 0; i < num_objs; i++) {
		spin_lock_bh(&sync_dev->row_spinlocks[sync_objs[i]]);
		child_row = table + sync_objs[i];
		switch (child_row->state) {
		case CAM_SYNC_STATE_SIGNALED_ERROR:
			spin_unlock_bh(&sync_dev->row_spinlocks[sync_objs[i]]);
			return CAM_SYNC_STATE_SIGNALED_ERROR;
		case CAM_SYNC_STATE_SIGNALED_SUCCESS:
			success_count++;
			break;
		case CAM_SYNC_STATE_ACTIVE:
			active_count++;
			break;
		default:
			CAM_ERR(CAM_SYNC,
				"Invalid state of child object during merge");
			spin_unlock_bh(&sync_dev->row_spinlocks[sync_objs[i]]);
			return CAM_SYNC_STATE_SIGNALED_ERROR;
		}
		spin_unlock_bh(&sync_dev->row_spinlocks[sync_objs[i]]);
	}

	if (active_count)
		return CAM_SYNC_STATE_ACTIVE;

	if (success_count == num_objs)
		return CAM_SYNC_STATE_SIGNALED_SUCCESS;

	return CAM_SYNC_STATE_SIGNALED_ERROR;
}

static int cam_sync_util_get_group_object_remaining_count(
	struct sync_table_row *table,
	uint32_t *sync_objs,
	uint32_t num_objs)
{
	int i;
	struct sync_table_row *child_row = NULL;
	int remaining_count = 0;

	if (!table || !sync_objs)
		return -EINVAL;

	for (i = 0; i < num_objs; i++) {
		child_row = table + sync_objs[i];
		if (child_row->state == CAM_SYNC_STATE_ACTIVE)
			remaining_count++;
	}

	return remaining_count;
}

int cam_sync_init_group_object(struct sync_table_row *table,
	uint32_t idx,
	uint32_t *sync_objs,
	uint32_t num_objs)
{
	int i;
	int remaining;
	struct sync_child_info *child_info;
	struct sync_parent_info *parent_info;
	struct sync_table_row *row = table + idx;
	struct sync_table_row *child_row = NULL;

	INIT_LIST_HEAD(&row->parents_list);

	INIT_LIST_HEAD(&row->children_list);

	/*
	 * While traversing parents and children, we allocate in a loop and in
	 * case allocation fails, we call the clean up function which frees up
	 * all memory allocation thus far
	 */
	for (i = 0; i < num_objs; i++) {
		child_info = kzalloc(sizeof(*child_info), GFP_ATOMIC);

		if (!child_info) {
			cam_sync_util_cleanup_children_list(row,
				SYNC_LIST_CLEAN_ALL, 0);
			return -ENOMEM;
		}

		child_info->sync_id = sync_objs[i];
		list_add_tail(&child_info->list, &row->children_list);
	}

	for (i = 0; i < num_objs; i++) {
		/* This gets us the row corresponding to the sync object */
		child_row = table + sync_objs[i];
		spin_lock_bh(&sync_dev->row_spinlocks[sync_objs[i]]);
		parent_info = kzalloc(sizeof(*parent_info), GFP_ATOMIC);
		if (!parent_info) {
			cam_sync_util_cleanup_parents_list(child_row,
				SYNC_LIST_CLEAN_ALL, 0);
			cam_sync_util_cleanup_children_list(row,
				SYNC_LIST_CLEAN_ALL, 0);
			spin_unlock_bh(&sync_dev->row_spinlocks[sync_objs[i]]);
			return -ENOMEM;
		}
		parent_info->sync_id = idx;
		list_add_tail(&parent_info->list, &child_row->parents_list);
		spin_unlock_bh(&sync_dev->row_spinlocks[sync_objs[i]]);
	}

	row->type = CAM_SYNC_TYPE_GROUP;
	row->sync_id = idx;
	row->state = cam_sync_util_get_group_object_state(table,
		sync_objs, num_objs);
	remaining = cam_sync_util_get_group_object_remaining_count(table,
		sync_objs, num_objs);
	if (remaining < 0) {
		CAM_ERR(CAM_SYNC, "Failed getting remaining count");
		return -ENODEV;
	}

	row->remaining = remaining;

	init_completion(&row->signaled);
	INIT_LIST_HEAD(&row->callback_list);
	INIT_LIST_HEAD(&row->user_payload_list);

	if (row->state != CAM_SYNC_STATE_ACTIVE)
		complete_all(&row->signaled);

	return 0;
}

int cam_sync_deinit_object(struct sync_table_row *table, uint32_t idx)
{
	struct sync_table_row      *row = table + idx;
	struct sync_child_info     *child_info, *temp_child;
	struct sync_callback_info  *sync_cb, *temp_cb;
	struct sync_parent_info    *parent_info, *temp_parent;
	struct sync_user_payload   *upayload_info, *temp_upayload;
	struct sync_table_row      *child_row = NULL, *parent_row = NULL;
	struct list_head            temp_child_list, temp_parent_list;

	if (!table || idx <= 0 || idx >= CAM_SYNC_MAX_OBJS)
		return -EINVAL;

	CAM_DBG(CAM_SYNC,
		"row name:%s sync_id:%i [idx:%u] row_state:%u",
		row->name, row->sync_id, idx, row->state);

	spin_lock_bh(&sync_dev->row_spinlocks[idx]);
	if (row->state == CAM_SYNC_STATE_INVALID) {
		spin_unlock_bh(&sync_dev->row_spinlocks[idx]);
		CAM_ERR(CAM_SYNC,
			"Error: accessing an uninitialized sync obj: idx = %d",
			idx);
		return -EINVAL;
	}
	row->state = CAM_SYNC_STATE_INVALID;

	/* Object's child and parent objects will be added into this list */
	INIT_LIST_HEAD(&temp_child_list);
	INIT_LIST_HEAD(&temp_parent_list);

	list_for_each_entry_safe(child_info, temp_child, &row->children_list,
		list) {
		if (child_info->sync_id <= 0)
			continue;

		list_del_init(&child_info->list);
		list_add_tail(&child_info->list, &temp_child_list);
	}

	list_for_each_entry_safe(parent_info, temp_parent, &row->parents_list,
		list) {
		if (parent_info->sync_id <= 0)
			continue;

		list_del_init(&parent_info->list);
		list_add_tail(&parent_info->list, &temp_parent_list);
	}

	spin_unlock_bh(&sync_dev->row_spinlocks[idx]);

	/* Cleanup the child to parent link from child list */
	while (!list_empty(&temp_child_list)) {
		child_info = list_first_entry(&temp_child_list,
			struct sync_child_info, list);
		child_row = sync_dev->sync_table + child_info->sync_id;

		spin_lock_bh(&sync_dev->row_spinlocks[child_info->sync_id]);

		if (child_row->state == CAM_SYNC_STATE_INVALID) {
			list_del_init(&child_info->list);
			spin_unlock_bh(&sync_dev->row_spinlocks[
				child_info->sync_id]);
			kfree(child_info);
			continue;
		}

		cam_sync_util_cleanup_parents_list(child_row,
			SYNC_LIST_CLEAN_ONE, idx);

		list_del_init(&child_info->list);
		spin_unlock_bh(&sync_dev->row_spinlocks[child_info->sync_id]);
		kfree(child_info);
	}

	/* Cleanup the parent to child link */
	while (!list_empty(&temp_parent_list)) {
		parent_info = list_first_entry(&temp_parent_list,
			struct sync_parent_info, list);
		parent_row = sync_dev->sync_table + parent_info->sync_id;

		spin_lock_bh(&sync_dev->row_spinlocks[parent_info->sync_id]);

		if (parent_row->state == CAM_SYNC_STATE_INVALID) {
			list_del_init(&parent_info->list);
			spin_unlock_bh(&sync_dev->row_spinlocks[
				parent_info->sync_id]);
			kfree(parent_info);
			continue;
		}

		cam_sync_util_cleanup_children_list(parent_row,
			SYNC_LIST_CLEAN_ONE, idx);

		list_del_init(&parent_info->list);
		spin_unlock_bh(&sync_dev->row_spinlocks[parent_info->sync_id]);
		kfree(parent_info);
	}

	spin_lock_bh(&sync_dev->row_spinlocks[idx]);
	list_for_each_entry_safe(upayload_info, temp_upayload,
			&row->user_payload_list, list) {
		list_del_init(&upayload_info->list);
		kfree(upayload_info);
	}

	list_for_each_entry_safe(sync_cb, temp_cb,
			&row->callback_list, list) {
		list_del_init(&sync_cb->list);
		kfree(sync_cb);
	}

	memset(row, 0, sizeof(*row));
	clear_bit(idx, sync_dev->bitmap);
	INIT_LIST_HEAD(&row->callback_list);
	INIT_LIST_HEAD(&row->parents_list);
	INIT_LIST_HEAD(&row->children_list);
	INIT_LIST_HEAD(&row->user_payload_list);
	spin_unlock_bh(&sync_dev->row_spinlocks[idx]);

	CAM_DBG(CAM_SYNC, "Destroying sync obj:%d successful", idx);
	return 0;
}

void cam_sync_util_cb_dispatch(struct work_struct *cb_dispatch_work)
{
	struct sync_callback_info *cb_info = container_of(cb_dispatch_work,
		struct sync_callback_info,
		cb_dispatch_work);

	cb_info->callback_func(cb_info->sync_obj,
		cb_info->status,
		cb_info->cb_data);

	kfree(cb_info);
}

void cam_sync_util_send_v4l2_event(uint32_t id,
	uint32_t sync_obj,
	int status,
	void *payload,
	int len)
{
	struct v4l2_event event;
	__u64 *payload_data = NULL;
	struct cam_sync_ev_header *ev_header = NULL;

	event.id = id;
	event.type = CAM_SYNC_V4L_EVENT;

	ev_header = CAM_SYNC_GET_HEADER_PTR(event);
	ev_header->sync_obj = sync_obj;
	ev_header->status = status;

	payload_data = CAM_SYNC_GET_PAYLOAD_PTR(event, __u64);
	memcpy(payload_data, payload, len);

	v4l2_event_queue(sync_dev->vdev, &event);
	CAM_DBG(CAM_SYNC, "send v4l2 event for sync_obj :%d",
		sync_obj);
}

int cam_sync_util_validate_merge(uint32_t *sync_obj, uint32_t num_objs)
{
	int i;
	struct sync_table_row *row = NULL;

	if (num_objs <= 1) {
		CAM_ERR(CAM_SYNC, "Single object merge is not allowed");
		return -EINVAL;
	}

	for (i = 0; i < num_objs; i++) {
		row = sync_dev->sync_table + sync_obj[i];
		spin_lock_bh(&sync_dev->row_spinlocks[sync_obj[i]]);
		if (row->type == CAM_SYNC_TYPE_GROUP ||
			row->state == CAM_SYNC_STATE_INVALID) {
			CAM_ERR(CAM_SYNC,
				"Group obj %d can't be merged or obj UNINIT",
				sync_obj[i]);
			spin_unlock_bh(&sync_dev->row_spinlocks[sync_obj[i]]);
			return -EINVAL;
		}
		spin_unlock_bh(&sync_dev->row_spinlocks[sync_obj[i]]);
	}
	return 0;
}

int cam_sync_util_add_to_signalable_list(int32_t sync_obj,
	uint32_t status,
	struct list_head *sync_list)
{
	struct cam_signalable_info *signalable_info = NULL;

	signalable_info = kzalloc(sizeof(*signalable_info), GFP_ATOMIC);
	if (!signalable_info)
		return -ENOMEM;

	signalable_info->sync_obj = sync_obj;
	signalable_info->status = status;

	list_add_tail(&signalable_info->list, sync_list);
	CAM_DBG(CAM_SYNC, "Add sync_obj :%d with status :%d to signalable list",
		sync_obj, status);

	return 0;
}

int cam_sync_util_get_state(int current_state,
	int new_state)
{
	int result = CAM_SYNC_STATE_SIGNALED_ERROR;

	if (new_state != CAM_SYNC_STATE_SIGNALED_SUCCESS &&
		new_state != CAM_SYNC_STATE_SIGNALED_ERROR)
		return CAM_SYNC_STATE_SIGNALED_ERROR;

	switch (current_state) {
	case CAM_SYNC_STATE_INVALID:
		result =  CAM_SYNC_STATE_SIGNALED_ERROR;
		break;

	case CAM_SYNC_STATE_ACTIVE:
	case CAM_SYNC_STATE_SIGNALED_SUCCESS:
		if (new_state == CAM_SYNC_STATE_SIGNALED_ERROR)
			result = CAM_SYNC_STATE_SIGNALED_ERROR;
		else if (new_state == CAM_SYNC_STATE_SIGNALED_SUCCESS)
			result = CAM_SYNC_STATE_SIGNALED_SUCCESS;
		break;

	case CAM_SYNC_STATE_SIGNALED_ERROR:
		result = CAM_SYNC_STATE_SIGNALED_ERROR;
		break;
	}

	return result;
}

void cam_sync_util_cleanup_children_list(struct sync_table_row *row,
	uint32_t list_clean_type, uint32_t sync_obj)
{
	struct sync_child_info *child_info = NULL;
	struct sync_child_info *temp_child_info = NULL;
	uint32_t                curr_sync_obj;

	list_for_each_entry_safe(child_info,
			temp_child_info, &row->children_list, list) {
		if ((list_clean_type == SYNC_LIST_CLEAN_ONE) &&
			(child_info->sync_id != sync_obj))
			continue;

		curr_sync_obj = child_info->sync_id;
		list_del_init(&child_info->list);
		kfree(child_info);

		if ((list_clean_type == SYNC_LIST_CLEAN_ONE) &&
			(curr_sync_obj == sync_obj))
			break;
	}
}

void cam_sync_util_cleanup_parents_list(struct sync_table_row *row,
	uint32_t list_clean_type, uint32_t sync_obj)
{
	struct sync_parent_info *parent_info = NULL;
	struct sync_parent_info *temp_parent_info = NULL;
	uint32_t                 curr_sync_obj;

	list_for_each_entry_safe(parent_info,
			temp_parent_info, &row->parents_list, list) {
		if ((list_clean_type == SYNC_LIST_CLEAN_ONE) &&
			(parent_info->sync_id != sync_obj))
			continue;

		curr_sync_obj = parent_info->sync_id;
		list_del_init(&parent_info->list);
		kfree(parent_info);

		if ((list_clean_type == SYNC_LIST_CLEAN_ONE) &&
			(curr_sync_obj == sync_obj))
			break;
	}
}
