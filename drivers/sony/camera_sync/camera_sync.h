/*
 * Sync File validation framework and debug infomation
 *
 * Copyright (C) 2012 Google, Inc.
 * Copyright 2021 Sony Corporation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef CAMERA_SYNC_H
#define CAMERA_SYNC_H

#include <linux/list.h>
#include <linux/rbtree.h>
#include <linux/spinlock.h>
#include <linux/dma-fence.h>

#include <linux/sync_file.h>
#include <uapi/linux/sync_file.h>

/**
 * struct camera_sync_timeline - sync object
 * @kref:		reference count on fence.
 * @name:		name of the camera_sync_timeline. Useful for debugging
 * @lock:		lock protecting @pt_list and @value
 * @pt_tree:		rbtree of active (unsignaled/errored) camera_sync_pts
 * @pt_list:		list of active (unsignaled/errored) camera_sync_pts
 * @camera_sync_timeline_list:	membership in global camera_sync_timeline_list
 */
struct camera_sync_timeline {
	struct kref		kref;
	char			name[32];

	/* protected by lock */
	u64			context;
	int			value;

	struct rb_root		pt_tree;
	struct list_head	pt_list;
	spinlock_t		lock;

	struct list_head	camera_sync_timeline_list;
};

static inline struct camera_sync_timeline *camera_sync_fence_parent(struct dma_fence *fence)
{
	return container_of(fence->lock, struct camera_sync_timeline, lock);
}

/**
 * struct camera_sync_pt - camera_sync_pt object
 * @base: base fence object
 * @link: link on the sync timeline's list
 * @node: node in the sync timeline's tree
 */
struct camera_sync_pt {
	struct dma_fence base;
	struct list_head link;
	struct rb_node node;
};

#endif /* CAMERA_SYNC_H */
